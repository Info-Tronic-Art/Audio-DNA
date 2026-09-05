# Lane 2 — render-effects — re-normalize findings (2026-09-05)

Scope: `src/render`, `src/effects`, `shaders/`. Baseline: `9139dd4` (2026-07-16 normalize) → `HEAD`.

## Scope of drift

```
git log --oneline 9139dd4..HEAD -- src/render src/effects shaders/ | wc -l
```
**19 commits** touch these paths (counted by hand from the log, listed below oldest→newest is reversed in `git log` output; I list newest-first as returned):

```
c792ecc fix(compositor): L2 — Layer Solo now actually solos, at all THREE sites
123555e fix(milkdrop): L0-MD — restore preset autoload, dead on every launch since 22fcedc
57aa436 fix(presets): dual-key effect/param targeting — stop silent retargeting
c51aff7 feat(mapping): message-thread mapping tick @120Hz (OW arc C3: W5+A4+A6)
88af683 refactor(gl): per-renderer EffectChainGLState + per-call snapshot + output detach law (OW arc C1: W1+W2+W3)
cb4d5fa refactor(featurebus): seqlock conversion — single-writer value-copy protocol (S2: R1-R5+R7+R9)
ca0b425 fix(output-window): remove duplicate MappingEngine::processFrame call, correct stale thread-safety comments
7d39d25 Add REST status/toggle for Syphon output
c316a1c State the adjudicated truth in the isAttached() guard's comment
ff19094 Add isAttached() defense-in-depth guard; correct test comment neutrality
cbeb287 Mark the effectChain_ per-field sync boundary explicitly (review advisory)
05114eb Revert EffectChain move semantics; fix the test helper instead
22fcedc Harden activeSources_: confine mutation to the GL thread
f0916d1 Fix EffectChain build break: restore move semantics after mutex addition
10b68cb Fix EffectChain cross-thread race: mutex + idempotent population
76594fd Fix MilkDrop UAF: don't destroy activeSources_ on GL context close
7921572 feat(undo): Undo v1 step 2 — SetClipCmd + single-cell sites, replace-undo media fix
71f2387 feat(output): Wave 1-A — wire Syphon output publishing
368d621 refactor(cleanup): Wave 0 Group 1 — purge dead code (13 items)
```

`git diff --stat 9139dd4..HEAD -- src/render src/effects shaders/`:
```
 shaders/hue_shift.frag          |  36 -----   (deleted, dead loose file — see below)
 shaders/passthrough.vert        |  12 --      (deleted)
 shaders/rgb_split.frag          |  26 ----    (deleted)
 shaders/ripple.frag             |  32 -----   (deleted)
 shaders/vignette.frag           |  29 ----    (deleted)
 src/effects/EffectChain.cpp     | 177 ++++++++++++++---------
 src/effects/EffectChain.h       | 135 ++++++++++++++----
 src/effects/EffectLibrary.cpp   |  26 ++++
 src/effects/EffectLibrary.h     |  10 +-
 src/effects/ISFShaderLoader.cpp |  35 -----
 src/effects/ISFShaderLoader.h   |   6 -
 src/effects/UniformBridge.cpp   |  57 --------  (deleted, already correctly noted in doc)
 src/effects/UniformBridge.h     |  33 -----      (deleted, already correctly noted in doc)
 src/render/CompositorEngine.cpp |  30 +++-
 src/render/CompositorEngine.h   |  10 +-
 src/render/EmbeddedShaders.h    |  30 ----      (net: 1 dead embedded shader removed)
 src/render/Renderer.cpp         | 305 ++++++++++++++++++++++++++++++++++------
 src/render/Renderer.h           | 106 +++++++++++++-
 18 files changed, 645 insertions(+), 450 deletions(-)
```
The 5 deleted `shaders/*.frag`/`.vert` loose files are Wave-0 dead-code purge — the real shader source of truth is `src/render/EmbeddedShaders.h` (inline strings); these loose files were orphaned duplicates, not a second shader system. Not documented in FEATURES.md, doesn't need to be (they were dead).

## Doc sections read (by heading)
- `## 4. Visual Effects System [R]` (line 194) + `### 4a. FX Browser` (line ~500)
- `## 5. Render Pipeline [R]` (line 326), `### 5a. Clip-to-Clip Transitions`, `### 5b. Keying & Masking System`, `### 5c. Master Opacity & Composition Transform`
- Cross-checked `## 22. Ghost Features — Control Domain [C]` → `22b. Crossfader` (touches `Renderer.cpp:508`, in my lane)

---

## RECOUNT TABLE (every number re-derived from source)

| Claim | Doc says | Source says | Method |
|---|---|---|---|
| Total effects | 135 | **135** (confirmed) | Parsed `src/effects/EffectLibrary.cpp` for balanced `registerEffect({...})` call blocks (script below). 135 calls, 0 unmatched, 0 duplicate names. |
| Categories | 11 | **11** (confirmed) | Same script, distinct `category` field values: warp 27, color 31, pattern 19, glitch 15, animation 6, blur 10, 3d 9, time 6, composite 3, audio 4, blend 5. All 11 counts match the doc table exactly, cell for cell. |
| Total params | 333 | **333** (confirmed) | Same script, counted `{"paramName", "u_...", value}` triples inside each call block. |
| Clip-to-clip transition shaders | 15 | **15** (confirmed) | `getTransitionShaderName()` in `CompositorEngine.cpp` has 14 explicit `case` labels + `default:` (Dissolve) = 15 distinct shader-mapped `MixMode` values. Cross-checked against `Renderer.cpp`'s "Phase 14: Transition Shaders" compile block — exactly 15 `compile("transition_*", ...)` calls, same 15 names. |
| MixMode transition-subset enum entries | 30 | **30** (confirmed) | Counted `Layer::MixMode` enum entries from `Cut` through `Displace` in `src/model/Layer.h`. |
| **"+1 deck" transition** | not itemized as a number in doc | **1 additional, undocumented, real** | See MAJOR finding below — a 16th, deck-level transition shader (`deck_transition` / `EmbeddedShaders::deckTransition`) exists and runs in production, entirely separate from the 15-shader clip-level set and from the `MixMode` enum. |
| Embedded shaders (`EmbeddedShaders.h`) | 244 | **243** | `grep -c '^inline const char\*' src/render/EmbeddedShaders.h` → 243. Cross-checked `grep -c '^inline '` → also 243 (no other inline types exist). A second pattern, raw `R"(` count, gives 268 — that's *not* a shader count, it's inflated by definitions that concatenate multiple raw-string literals internally (utility-block + main-body). 243 is the correct unit count. |
| Explains the −1 | — | `git diff 9139dd4..HEAD -- src/render/EmbeddedShaders.h` shows exactly one shader deleted: `sourceFluidDisplay` (36 lines, part of Wave-0 dead-code purge, `368d621`), no shader added. 244 − 1 = 243. So the doc's 244 was correct as of 9139dd4 and is now stale by exactly the one shader Wave 0 removed. |
| Live health endpoint `effects_count: 135` | given as "today's live value" | **Explained, not a discrepancy** — see below | Traced `/api/health` (`ApiServer.cpp:233`, mirrored in `TestServer.cpp:197`) → `effectChain_.getNumEffects()`. `effectChain_` is `previewPanel_.getRenderer().getEffectChain()` — the single **global** `EffectChain` instance, wired in `MainComponent.cpp:1642`. |

### Why 135 == 135 is not a coincidence, but also not "user has 135 effects active"
`Renderer::initEffectChain()` (`src/render/Renderer.cpp`, ~line 1533) loads **every** effect from `EffectLibrary::registerDefaults()` into the global chain at GL-context creation, one `addEffect()` call per effect, **all disabled by default**:
```cpp
for (const auto& cat : categoryOrder)
{
    auto names = effectLibrary_.getEffectsByCategory(cat);
    for (const auto& name : names)
    {
        auto effect = effectLibrary_.createEffect(name);
        if (effect) { effect->setEnabled(false); effectChain_.addEffect(std::move(effect)); }
    }
}
```
So the global `EffectChain` is **always** a full 1:1 mirror of the library (135 slots, most disabled) — never a sparse, user-built chain. `getNumEffects()` is therefore always 135 regardless of how many the user has actually turned on. The live endpoint's 135 is real and correctly derived — it's just measuring "library size mirrored into the global chain," not "effects in use." This is a materially different fact than what a reader of the endpoint name (`effects_count`) would assume, and it is **nowhere explained in FEATURES.md**.

This also means FEATURES.md §4's Implementation-chain step 2 — *"User drags effect from FX Browser → `EffectSlot` added to clip/layer/global chain"* — is **factually wrong for the global level**: nothing is ever `addEffect()`'d to the global chain at runtime; the FX Browser drag at the global level can only be *enabling* an already-present (disabled) slot, not adding a new one. Confirmed no second call site: `grep -rn '\.addEffect(' src/` returns exactly one hit, the startup-only one above.

---

## MAJOR / CRITICAL findings

### 1. [MAJOR] Clip/Layer effects do NOT go through the `EffectChain` class at all — doc implies one unified path
FEATURES.md §4's "Implementation chain" (steps 2–4) and its Data-flow diagram present a single mechanism — `EffectSlot` → `EffectChain::render()` ping-pong FBOs — serving clip, layer, and global levels alike.

Source shows two genuinely different mechanisms:
- **Global level**: real `EffectChain` C++ object (`src/effects/EffectChain.h/.cpp`), preloaded with all 135 effects at startup (see above), rendered via `EffectChain::render()`'s own ping-pong FBOs (`effectFBO_A_`/`effectFBO_B_` owned by `EffectChainGLState`).
- **Clip/Layer level**: `Clip::EffectSlot` (`src/model/Clip.h:48`, a plain data struct) stored in `std::vector<EffectSlot> effects` on `Clip` and `std::vector<Clip::EffectSlot> layerEffects` on `Layer` (`src/model/Layer.h:140`). These are applied by `CompositorEngine::applyClipEffects()` (`CompositorEngine.cpp:232`), which resolves shaders directly via `EffectLibrary::getEffectDef()` and does its **own independent** ping-pong through `CompositorEngine`'s own `effectFBO_A_`/`effectFBO_B_` — it never touches the `EffectChain` class or its instance data at all.

Evidence: `grep -rn "EffectSlot\|effectChain\|EffectChain" src/model/Clip.h src/model/Layer.h` shows `EffectSlot`/`effects`/`layerEffects` only — no `EffectChain` reference anywhere in the model. `grep -n "applyClipEffects" src/render/CompositorEngine.cpp` shows 6 call sites, all independent of `EffectChain::render()`.

This split is *partially* self-acknowledged in §4's own Gotchas ("Temporal effects need BOTH render paths (EffectChain + CompositorEngine). Fixing only one path = silent failure in the other.") — so the fact of two paths isn't a total secret, but the numbered Implementation-chain steps at the top of the section flatly contradict it by describing one path. A reader who stops at "Implementation chain" (the normal reading order) gets a wrong mental model that the Gotchas buried six paragraphs later has to silently correct.

### 2. [MAJOR] A 16th, deck-level transition shader exists and is completely absent from the transitions doc
§5a ("Clip-to-Clip Transitions") documents exactly 15 shader-backed transitions plus 15 enum-only fallbacks — all scoped to `Layer::MixMode` / per-clip crossfades. It does not mention deck-level transitions at all.

Source (`src/render/Renderer.cpp` ~lines 508–592, `src/render/Renderer.h:286-294`, `src/render/EmbeddedShaders.h:111-113`) implements a **separate, real, user-visible** cross-deck crossfade:
- `Renderer` tracks `deckTransitionProgress_` / `deckTransitionSpeed_` and `prevDeckTexture_`.
- Detects `composition_->activeDeckIndex` changing, snapshots the outgoing deck's framebuffer, then blends old→new deck over `globalTransitionSpeed` seconds using a dedicated shader program `"deck_transition"` (`EmbeddedShaders::deckTransition`).
- Reads `composition_->crossfaderBlendMode` as the blend-mode uniform for this blend.

This is a real, currently-firing render feature (switching decks visibly crossfades, not cuts) with its own embedded shader, entirely outside the 15/30 transition inventory in §5a. §22b ("Crossfader — Deck Blend Control") mentions `Renderer.cpp:508` only in the context of debunking the *live crossfader* ghost feature — it never describes the deck-switch transition shader as a feature in its own right. Net: this render-pipeline feature has zero dedicated documentation anywhere in FEATURES.md.

### 3. [MAJOR] Internal doc self-contradiction: §4 Test-coverage line says "112 effects", §4's own inventory table (same section, few paragraphs up) says 135
Quote: *"`tests/visual/test_effects.py` — auto-discovers all 112 effects, verifies param changes output"*.

Traced `tests/visual/test_effects.py`: its `all_effects` fixture is just `app.state()["effects"]`, and `/api/state` (`ApiServer.cpp` handler) iterates `effectChain_.getNumEffects()` with no filtering — i.e., it discovers however many effects are in the (always-135) global chain. There is no skip/filter logic in the test (`grep -n "skip\|continue" tests/visual/test_effects.py` shows only an unrelated `pytest.skip` for a missing fixture image and one unrelated `continue`). So the test discovers **135**, not 112.

I checked whether 112 was ever correct: at `9139dd4` itself, `EffectLibrary.cpp` already had 135 `registerEffect({...})` calls (`git show 9139dd4:src/effects/EffectLibrary.cpp | grep -c 'registerEffect({'` → 135). So **112 was already wrong at the last normalize** — this is not new drift from the 127 commits, it's a pre-existing stale number that survived the last pass and should be corrected to 135 now.

---

## MINOR findings

- **Line-number drift** on class-declaration citations in §4/§5 entry points:
  - `EffectChain class (src/effects/EffectChain.h:23)` → now line **59** (class moved down by the new `EffectChainGLState` struct added in `88af683`).
  - `Renderer class (src/render/Renderer.h:37)` → now line **41**.
  - `EffectLibrary class (src/effects/EffectLibrary.h:12)` → still correct, line 12.
  - `CompositorEngine class (src/render/CompositorEngine.h:27)` → still correct, line 27.
  - `ShaderManager class (src/render/ShaderManager.h:11)` → still correct, line 11.
- **Line-number drift** on `Renderer.cpp:1324-1338` cited in §5a as the transition-shader compile block. Current location is `Renderer.cpp:1468-1483` (confirmed by reading the `// === Phase 14: Transition Shaders (15 clip-to-clip transitions) ===` block directly). The comment text and the 15 `compile()` calls themselves are otherwise verbatim-accurate.
- **Embedded-shader count off by exactly 1** (244 → 243), fully explained above by the Wave-0 removal of `sourceFluidDisplay` — cosmetic/count-only, no behavior change (it had no remaining callers per the same commit's dead-code purge).
- Stale source comment (not a FEATURES.md bug, flagging for awareness only): `EffectLibrary.h` still says *"All 17 effects from the architecture are registered via registerDefaults()"* and `registerDefaults()`'s own doc-comment says *"Register all 17 built-in effects"* — both wildly stale (actual: 135), but this is a source-code comment issue, not a FEATURES.md drift, so not scored.

---

## Dead surfaces traced

1. **Layer Solo (fixed TODAY, `c792ecc`, was dead before)** — worth recording for the historical record even though it's no longer a current bug. FEATURES.md never asserted Solo was wired (§26, out of my lane, just describes the UI button), so this isn't a doc-drift *finding* against my sections, but it's exactly the class of defect this whole audit exists to catch, and it lived in my lane's files (`CompositorEngine.cpp`) for an unknown period before today. Before `c792ecc`: `layer.solo` was set/toggled/undone by UI and model code, but **zero** of the three `deck.layers` iteration sites in `CompositorEngine.cpp` (`compositeDeck()`'s active-check loop, `compositeDeck()`'s composite loop, `compositePersistentLayers()`) ever read `layer.solo` — confirmed by the commit's own diff (three separate `+ (anySolo && !layer.solo)` insertions) and its message stating a reviewer proved these were the only three `deck.layers` sites in `src/render`. As of HEAD this is fixed and consumed correctly at all three sites — no further action needed, but the next normalize's §8/§26 pass should probably note Solo is now real, since neither section currently says one way or the other.
2. **Global `EffectChain` "addEffect" surface** — see MAJOR #1: the FX-Browser-drag-adds-to-global-chain claim in §4 has no live call site. The actual consumer of a global-level drag is `Effect::setEnabled(true)` on an already-present slot (I did not chase the exact UI drop-handler for the global rack inside `EffectsRackPanel` — that file is outside my lane paths — but I can positively confirm from `src/render`/`src/effects` alone that `addEffect()` is never called again after startup, so whatever the UI does, it cannot be "adding" a slot).

---

## Could not determine (honest gaps)

- I did not verify the exact UI code path for how `EffectsRackPanel`/FX Browser actually mutates the global chain's per-slot enabled state (file is `src/ui/EffectsRackPanel.*`, outside my lane's paths). I can only confirm from `src/effects`/`src/render` that it cannot be via `addEffect()`.
- I did not verify whether `Clip::EffectSlot`'s param-value application for temporal effects (`u_prev_frame`) at the clip/layer level uses the same `EffectChainGLState`-style per-GL-context safety as the global chain's post-`88af683` refactor, or an older/different temporal-buffer mechanism (`layerTemporalBuffers_` per §5, implementation chain step 6). I read the call sites but not the full temporal-buffer allocation/lifetime code; a mismatch here (given the OutputWindow's unshared-GL-context work landing this cycle) is plausible but unconfirmed.
- I did not chase every one of the 135 effects to confirm 0 dead/unregistered shader names (i.e., that every `shaderName` in `EffectLibrary.cpp` has a matching `compile(...)` call in `Renderer.cpp`) — that's a much larger cross-check than my lane's five-number remit and I did not have budget to do it exhaustively with two independent patterns as the task's evidentiary bar requires. Flagging as a plausible follow-up for whoever owns full effect-shader integrity, not asserting a defect exists.
- I did not check whether the `deck_transition` shader (`EmbeddedShaders::deckTransition`) has any test coverage at all (my best guess from the "no test coverage for transitions" note in §5a is no, since it's not even in the visible transition inventory to begin with, but I did not grep test files specifically for `deck_transition` or `deckTransitionProgress_`).

---

## Method note (reproducibility)

The 135/11/333 recount was done with this exact script against `src/effects/EffectLibrary.cpp`:
```python
import re
src = open('src/effects/EffectLibrary.cpp').read()
calls, n, key = [], len(src), 'registerEffect({'
idx = src.find(key)
while idx != -1:
    start = idx + len('registerEffect(')
    depth, j = 1, start
    while j < n:
        if src[j] == '(': depth += 1
        elif src[j] == ')':
            depth -= 1
            if depth == 0: break
        j += 1
    calls.append(src[start:j]); idx = src.find(key, j)
# 135 calls; category Counter() sums to 135; param-triple regex sums to 333
```
This balances parens rather than relying on line-based grep, which is what caught and avoided an off-by-2 trap (`grep -c 'registerEffect('` alone returns 137, which double-counts the function *definition* and the one `registerDynamic()` internal call — neither is a registered effect).
