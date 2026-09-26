# Reviewer Verdict — cleanup-r1
STATUS: DONE
VERDICT: APPROVE

Reviewed source ONLY at pinned worktree `.claude/worktrees/wf_09b440d3-9b7-2` commit `4ae64b9`
(via `git -C <wt> show 4ae64b9:<file>`), diffed against `main`. Read-only; no build/app/debugger
used, per fence.

## FILE: src/effects/EffectChain.cpp (commit 3ec7b4c)
  [OK] Correctness: manually re-derived the aliasing claim by tracing the ping-pong invariant
       (writeFBO alternates 0/1; for any non-first effect, `compositeFBO = 1 - writeFBO` under
       the OLD code equals the FBO index holding `preEffectTexture`) — the bug is real: the old
       composite pass sampled `u_original` from the same FBO it was rendering into (undefined GL
       feedback loop).
  [OK] Fix math: worked the blend algebra by hand. `glBlendFunc(GL_ONE_MINUS_CONSTANT_ALPHA,
       GL_CONSTANT_ALPHA)` + `glBlendColor(_,_,_,dryWet)` drawing `preEffectTexture` (dry) over
       the framebuffer already holding the "wet" result gives
       `result = dry*(1-dryWet) + wet*dryWet` = `mix(dry, wet, dryWet)` — bit-for-bit the same
       formula as `effect_drywet`'s GLSL (`EmbeddedShaders.h`: `mix(orig, eff, u_drywet)`,
       verified at line ~86). Reads the destination through the ROP, not a sampler, so no
       feedback loop. The screen-composite branch (last effect + dryWet, no temporal) was
       already correct and is untouched — confirmed by diff.
  [OK] Coverage of "every position": traced isLast/anyTemporal/needsDryWet branch matrix — the
       mid-chain-or-temporal-last case is the only one that changed, and it's the only one that
       had the bug.
  [ISSUE→non-blocking] Spec/claim fidelity (dimension 7) verified independently, not just
       trusted: grepped `src/api/ApiServer.cpp`, `src/test/TestServer.cpp`, `src/MainComponent.cpp`
       (OSC) and `src/model/Clip.cpp`/`Layer.cpp` (preset serialization) myself. Confirmed
       `Effect::setDryWet()` has exactly ONE caller in all of `src/` — `TestServer.cpp:646`,
       which only resets to `1.0f` on `/api/reset`. No REST/OSC/preset/UI path (checked
       `EffectsRackPanel.cpp` too — no dry/wet control exists there for the global chain) can
       ever set a global `Effect`'s `dryWet_` below the 1.0 default. So the fixed branch is
       currently **dead in the shipped app** — a real, correctly-fixed latent bug, but with zero
       reachable trigger today. This matches the builder's own disclosure in
       `.harmony/.reports/s-rta-0926/cleanup.md` §1 exactly (no overclaiming found — the report
       explicitly says G1 "cannot discriminate the fix" and ran MAIN vs LANE to prove it:
       identical 46/0 both builds). Non-blocking because it is honestly reported, not hidden,
       and the fix is independently verifiable by code-reading (done above) even without a live
       trigger. Suggestion for a future lane: a pure-logic unit test mirroring the
       writeFBO/compositeFBO index arithmetic (same style as the `ScratchPool::pick()` mirrors in
       item 2) would give this fix regression protection independent of the live-probe gap — the
       math is pure integers, no GL context needed, unlike the GL calls themselves.

## FILE: tests/test_compositor_opacity_alias.cpp, tests/test_compositor_effects_parity.cpp (fce2fa5)
  [OK] Both rewrites drive the REAL `ScratchPool::pick()` (production header, not a
       re-derived index formula) through the actual current call sequence documented in
       `CompositorEngine.cpp` comments (transform→opacity forceCopy logic; two chained
       `applyClipEffects` calls across odd/even/zero enabled-effect counts).
  [OK] Confirmed NOT subsumed by `test_scratch_pool.cpp` — read that file: its 4 `TEST_CASE`s
       (`pick never returns sampled/held`, `A/B ping-pong`, `third member while held`,
       `two-member insufficiency`) exercise `pick()` in isolation with no call-sequence/wiring
       coverage, matching the disclaimer text quoted in both rewritten files' headers.
  [OK] Mutation-test claim in cleanup.md (dropping the `readTex` exclusion from `pick()`) is a
       credible teeth-check for exactly this kind of scenario-mirroring test; I did not
       personally re-run the mutation (no build per fence) but the described mechanism (8/10
       cases red under the mutant, all green under the real header) is consistent with the test
       bodies I read — they assert `CHECK(firstLayerPassTarget != clipTexAfterPerClip)` etc.,
       which is exactly what a "never exclude readTex" mutant would violate.
  [OK] `tests/CMakeLists.txt` diff is comment-only for these two targets (no new source files,
       ScratchPool.h is header-only and was already included) — confirmed via `git diff`.

## FILE: src/ui/RoutineBankModel.h, src/recording/RoutineSlice.cpp, src/ui/RecordPanel.cpp (7ca3634)
  [OK] `clampRoutineBarRange` is pure, `juce_core`-only, matches the file's existing posture.
       `RecordPanel.cpp` clamps at the UI boundary before `onSaveRoutine`.
  [OK] `takeBeatOfBar` now refuses `bar < 1` with a whole-word `std::cerr` diagnostic (confirmed
       no exceptions convention elsewhere in `src/` is consistent with this style) and
       substitutes bar 1 — protects the REST path (`/api/routine/save`'s `useBars`, unmodified,
       out of fence) through the same funnel.
  [OK] Tests: 5 `clampRoutineBarRange` cases (boundary, blank/0, negative, inverted-range) +
       1 `takeBeatOfBar` refusal case — all read as genuine assertions on real return values, not
       vacuous.

## FILE: src/MainComponent.cpp (4ae64b9)
  [OK] Deck Save now sized with the same measured-width lambda as Deck Load (font
       `drawButtonText` actually uses, 16px total padding, 60px floor unchanged). No other
       layout change — confirmed by diff, single hunk.

## SLIM
  No new dead/vestigial/duplicate/speculative code found. Item 1's fix is a correction to
  EXISTING logic (same function, same signature) — not new code — so it is not itself a SLIM
  finding, even though the branch it lives in is currently unreachable via any live surface
  (documented above under Spec/claim fidelity, not SLIM, since nothing new was added).

## Scope / fence
  `git diff main...worktree-wf_09b440d3-9b7-2 --stat` confirms all touched files match the
  claimed fence: `src/effects/EffectChain.cpp`, `.harmony/probe-effects-parity.{sh,py}`,
  `src/MainComponent.cpp`, `src/recording/RoutineSlice.cpp`, `src/ui/RecordPanel.cpp`,
  `src/ui/RoutineBankModel.h`, `tests/CMakeLists.txt`, `tests/test_compositor_effects_parity.cpp`,
  `tests/test_compositor_opacity_alias.cpp`, `tests/test_routine.cpp`,
  `tests/test_routine_bank_model.cpp`, plus the report itself. `src/effects/EffectChain.h` and
  `src/render/ScratchPool.h` untouched, as claimed. Worktree is clean; no build-lane/.venv
  artifacts present.

SUMMARY: 4 commits reviewed across 13 changed files, 0 blocking issues, 1 non-blocking
suggestion (item 1: no pure-logic regression test for the fixed branch, mirroring item 2's
`ScratchPool::pick()`-mirror pattern, would be cheap insurance against the live-probe's honestly
disclosed inability to discriminate the fix). Fix correctness for item 1 verified by hand-tracing
the GL blend algebra and the aliasing invariant, not by trusting the report. Reachability claim
for item 1 independently re-verified by grep across ApiServer.cpp/TestServer.cpp/
MainComponent.cpp/Clip.cpp/Layer.cpp/EffectsRackPanel.cpp. Items 2-4 verified by reading full
diffs and confirming test assertions are non-vacuous and target real production code.

METADATA: reviewer=reviewer-cleanup, builder_packet=cleanup, date=2026-09-26
