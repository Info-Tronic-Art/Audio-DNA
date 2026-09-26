# cleanup lane — s-rta-0926 (four small items)

Labels: VERIFIED = ran/measured in this lane. INFERRED = reasoned from code, not run.
Builds: MAIN = `/Users/boriskarpman/projects/RealTimeAudio/build/…/Audio-DNA.app` (unmodified,
ctest 573/573 per the rig brief). LANE = worktree `build-lane` (deleted before returning; a fresh
`cmake -S . -B build-lane …` from this worktree's HEAD reproduces it).

## 1. EffectChain.cpp mid-chain dry/wet GL feedback loop (class-sweep item #27)

**Confirmed the hazard** by reading `src/effects/EffectChain.cpp`'s pre-fix `render()`: for a
non-first effect with `dryWet < 0.99`, the composite pass wrote into
`texMgr.getFBO(compositeFBO)` where `compositeFBO = 1 - writeFBO`. Tracing the ping-pong
invariant (writeFBO alternates 0/1 every iteration, currentInput always equals the previous
iteration's write target once past the first pass) shows `compositeFBO`'s texture is *exactly*
`preEffectTexture` — the same texture the composite pass samples as `u_original` — for any
effect after the first. Same shape as the xfade lane's CompositorEngine fix #6 (dry/wet on a
non-first effect), just in the sibling GLOBAL chain (`src/effects/EffectChain.cpp` /
`Renderer::effectChain_`, applied to the fully-composited output — see CLAUDE.md's "Global
effects (via `effectChain_` in Renderer)").

**Fix**: same technique the compositor actually used for *this specific bug shape* (not
ScratchPool/pickEffectTarget, which the xfade report's own section 3 says was used for the
*other* five call sites — dry/wet was fixed with **in-place fixed-function blending**): the
effect's "wet" result is already sitting in the FBO the render just bound
(`texMgr.getFBO(writeFBO)`); draw the pre-effect ("dry") texture over it with
`GL_ONE_MINUS_CONSTANT_ALPHA`/`GL_CONSTANT_ALPHA` blending (`glBlendColor` alpha = dryWet). That
reads the destination through the ROP, not a sampler, so it needs no third texture and samples
nothing it writes — `mix(dry, wet, dryWet)`, matching `effect_drywet`'s shader formula
(`EmbeddedShaders.h`: `mix(orig, eff, u_drywet)`). The screen-composite branch (last effect,
dry/wet, no temporal) was already safe (writes to `defaultFBO`, distinct from either sampled
texture) and is untouched.

**Live evidence — what I could and couldn't run** (`.harmony/probe-effects-parity.{sh,py}`, new
G1 row):
- **Confirmed by reading code** (not inference): neither `/api/set_effect_chain` nor
  `/api/set_effect` (`ApiServer.cpp`), nor either's TestServer.cpp mirror, nor OSC's
  `/audiodna/effect/{name}/{param}` → `MainComponent.cpp`'s `onSetEffectParam`, expose
  `Effect::dryWet_` — all three only match a named `EffectParam` by name; `dryWet_` is a
  separate field with its own `getDryWet()`/`setDryWet()`, set only in C++ (`Effect.h`, default
  1.0) and never from any REST/OSC/preset surface. `PresetManager` doesn't serialize it either.
  There is genuinely no reachable live path to drive `dryWet < 1` on the global chain without
  editing `ApiServer.cpp`/`TestServer.cpp`/`MainComponent.cpp`'s OSC wiring — all out of this
  lane's fence.
- So the literally-requested assertion ("second effect at dryWet 0.5 → frame = 0.5·DRY +
  0.5·WET") **cannot be run live in this lane**. G1's `PARITY-DOCUMENTS-GAP` row sends the
  request shape that fix would need (`"dryWet": 0.5` in the effect object) and reads
  `/api/state` back, asserting `dry_wet` is *still* 1.0 — a live, re-checkable proof of the gap
  itself, not just a comment.
- G1's `non-blank` rows are the closest reachable regression: `/api/load_image` +
  `/api/set_effect_chain` with two enabled effects (Hue Shift, then Invert — confirmed
  registration order from `EffectLibrary.cpp`, so Invert is the non-first effect), run BEFORE
  any `/api/load_composition` call in the file (once a composition loads, its active deck stays
  composited and shadows the loaded image — `Renderer::renderOpenGL`'s `deckActive` branch runs
  first). This is the **first live coverage of `EffectChain.cpp` in this probe file** — V1/V5/V6
  above all exercise `CompositorEngine`'s per-clip/layer effects instead.
- **RED-on-main / GREEN-on-lane, honestly**: since dryWet is unreachable, G1 cannot discriminate
  the fix — I ran it on both builds to confirm this rather than assume it. **MAIN**: `46 PASS /
  0 FAIL` including G1 (VERIFIED, `/tmp/parity.DOtEAF`). **LANE**: `46 PASS / 0 FAIL`, identical
  numbers (VERIFIED, `/tmp/parity.3cfX60`). Both runs, plus the pre-existing V1/V5/V6/REF*
  rows, are unaffected (I never touched `CompositorEngine.cpp`). G1 is real, executable, and
  proves the API gap and the regression path both hold — it is not a RED/GREEN discriminator
  for the numeric mid-chain claim itself.
- **Recommendation for a properly-fenced follow-up**: add `dryWet` parsing to
  `ApiServer::handleSetEffectChain`/`handleSetEffect` (mirroring how the Composition-side
  `Clip::EffectSlot.dryWet` is already parsed in `TestServer.cpp:1092`), which would let a
  future probe run the literal numeric assertion this task asked for.

ctest: unaffected (no ctest target touches `EffectChain::render()`'s GL path — it needs a live
context; the mid-chain branch has no unit-test harness today, same infeasibility documented in
`test_compositor_opacity_alias.cpp`/`test_compositor_effects_parity.cpp`).

## 2. test_compositor_opacity_alias.cpp / test_compositor_effects_parity.cpp rewrite

Both files mirrored the REMOVED `writeFBO`/hardcoded-A/B mechanism (pre-xfade). Read the current
`CompositorEngine::applyClipTransform`/`applyClipOpacity`/`applyClipEffects` — all three now
route every target through `pickEffectTarget()` (render/ScratchPool.h's `pick()`), and
`applyClipEffects` no longer tracks any "starting write index" at all: every pass, in every
call, calls `pick(currentInput, holdTex)` fresh. That generalizes the fix so it no longer cares
about effect-count parity, which is exactly what the old tests' index-parity formula assumed.

**Not retired** — `test_scratch_pool.cpp` (mapping shown): it exhaustively tests `pick()` itself
(every readTex/holdTex combo, the historic ping-pong, the 3rd-member/hold interaction, the
2-member insufficiency) and *explicitly disclaims* scenario-level wiring ("What it does NOT
cover: that every compositor call site actually routes its target through the picker"). The two
files rewritten here cover exactly that disclaimed scenario level: the transform→opacity
`forceCopy` business logic (when does `pick()` even run) and the two-chained-`applyClipEffects`-
call sequence (clip effects → layer effects) across odd/even/zero per-clip effect counts —
neither is exercised by `test_scratch_pool.cpp`'s pure single/double-call tests.

Both rewrites call the REAL `ScratchPool::pick()` for target selection (never a re-derived index
formula), mirroring the actual current call sequence from `CompositorEngine.cpp` (cited by line
range in each file's header comment), same "mirror the mechanism, not the subsystem" precedent
as `test_layer_transport_reverse.cpp` etc. (driving the real GL-issuing methods headlessly
remains infeasible, unchanged from before).

**Teeth (VERIFIED)**: compiled both rewritten files + the pre-existing `test_scratch_pool.cpp`
against a **mutated copy** of `ScratchPool.h` (scratchpad only — the real, fenced file was never
touched; `sha256sum` of the real file: `fa03416d932927b7cba90fb58cf77f4a05b61cff30c2995320f76e2cfa0bfab4`,
matching the xfade report's own citation, i.e. still byte-identical to what the xfade lane
shipped) with the `readTex` exclusion dropped (`if (pool[i] != holdTex)` instead of
`if (pool[i] != readTex && pool[i] != holdTex)`). Result: **8 of 10 test cases failed** (70
assertions, 60 passed / 10 failed) — including 3 of my 3 alias-checking cases in each rewritten
file and all 4 of `test_scratch_pool.cpp`'s own cases; the 2 that stayed green are the
mutation-invariant no-op paths (no transform + default opacity; zero per-clip effects), correctly
unaffected since they never call `pick()` at all. Recompiling the same three files against the
real (unmutated) header — resolved via the worktree's own `src/render/ScratchPool.h`, not the
mutant — passed all 211 assertions across the 10 cases, matching the full-suite ctest result
below exactly.

## 3. Routines Save bar-range validation + takeBeatOfBar refusal

Confirmed the finding against `.harmony/.reports/s-rta-0926/review-routine-strip-r1.md`'s own
"Non-blocking observations" (verbatim: "does not client-side-validate `fromBar >= 1` or
`fromBar <= toBar`... a blank field reads as `0` via `getIntValue()`, which the pre-existing
`takeBeatOfBar` helper... was not written to expect for bar 0 and would produce a negative beat
offset rather than a refusal... the same server-side call is already reachable unmodified via
`/api/routine/save`'s `useBars` path").

- **`RoutineBankModel.h`** (pure, `juce_core`-only, same posture the file already documents):
  added `clampRoutineBarRange(fromBar, toBar)` — clamps both to `>= 1` and `toBar` to
  `>= fromBar`. `RecordPanel.cpp`'s `saveRoutineBtn_.onClick` now clamps the raw editor values
  through it before ever calling `onSaveRoutine`.
- **`RoutineSlice.cpp`**'s `takeBeatOfBar(take, bar)` now refuses `bar < 1`: logs a whole-word
  `std::cerr` diagnostic (matching this file's own house style — no exceptions anywhere in
  `src/`, confirmed by grep) and substitutes bar 1, instead of computing
  `(bar - 1) * kBeatsPerBar` negative. This protects the REST path
  (`/api/routine/save`'s `useBars` → `MainComponent.cpp:5576-5577`, out of fence, untouched)
  identically to the UI path, since both funnel through this one function.
- Tests added: 5 `clampRoutineBarRange` cases (`tests/test_routine_bank_model.cpp`) and 1
  `takeBeatOfBar` refusal case (`tests/test_routine.cpp`, the one case the fence allows).

## 4. Deck Save button sizing

Confirmed against `.harmony/.reports/s-rta-0926/review-polish-r1.md`'s nit (verbatim:
"`deckSaveButton_`... stays on the old fixed 60px while `deckLoadButton_`... is now measured...
if `AudioDNALookAndFeel`'s button font ever grows, 'Deck Save' could develop the exact same
clipping bug this diff just fixed for 'Deck Load'"). Extracted one `measuredButtonWidth` lambda
(same measurement convention: `GlyphArrangement::getStringWidthInt` at the 14pt button font,
+16px padding, `jmax` against the old 60px floor) and applied it to both buttons. No other
layout change.

## Build / test evidence

- **Configure + build-lane**: clean configure, build exits 0 (26 pre-existing warnings total in
  the touched-file set, all confirmed pre-existing by `git diff` — the 3
  `unused parameter 'targetFBO'/'width'/'height'` warnings at `EffectChain.cpp:259` are inside
  `applyDryWet()`, a function I never touched; no new warnings at any line I edited).
- **ctest (serial, build-lane)**: **577/577 passed** (`Total Test time (real) = 9.74 sec`),
  including all new cases (`test #20-#25` compositor rewrites, `#389` takeBeatOfBar refusal,
  `#521-#525` clampRoutineBarRange).
- **Live probe** (`.harmony/probe-effects-parity.sh`, screen-safe `open -g`, live lock
  acquired/released around both runs, app confirmed not running before/after each): **MAIN
  46/0**, **LANE 46/0** — see section 1.

## Files changed (all within fence)

`src/effects/EffectChain.cpp`, `.harmony/probe-effects-parity.{sh,py}`,
`tests/test_compositor_opacity_alias.cpp`, `tests/test_compositor_effects_parity.cpp`,
`tests/CMakeLists.txt` (comment blocks for those two targets only), `src/ui/RoutineBankModel.h`,
`src/ui/RecordPanel.cpp` (bar editors only), `src/recording/RoutineSlice.cpp` (bar < 1 refusal
only), `tests/test_routine_bank_model.cpp`, `tests/test_routine.cpp` (one case),
`src/MainComponent.cpp` (Deck Save/Load sizing only). `src/render/ScratchPool.h` was read, not
edited — no shared helper was needed. `src/effects/EffectChain.h` was not touched (no signature
change).

## Learnings

- The xfade lane's own "fix rule" is not monolithic: 5 of its 6 fixed call sites route through
  `ScratchPool::pick()`/`pickEffectTarget()`, but the dry/wet-specific hazard (class-sweep #6,
  and this lane's #27) was fixed with in-place fixed-function blending instead, because the
  destination already holds the value the blend needs and reading it through the ROP avoids
  needing a third texture at all. "Same technique the compositor used" needed reading which of
  the two techniques applied to *this* bug shape, not assuming the more prominent one. (Learning
  returned per the rig brief -- notebook.md is fenced off in this lane, not edited.)
- `Effect::dryWet_` has no REST/OSC/preset write path anywhere in the app today — confirmed by
  reading, not assumed. A future lane adding live dry/wet testing for the global chain needs
  `ApiServer.cpp`/`TestServer.cpp` in its fence.
