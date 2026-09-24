# Reviewer Verdict — s-rta-0924b onset render fix (RE-REVIEW)
STATUS: DONE
VERDICT: PASS

Prior review (review-onset.md) was PASS with 2 MINOR. Builder made no code changes this
round (HEAD unchanged at 51d614d) and instead addressed both MINORs by disclosure/argument.
I independently re-verified both, re-verified the plan's critic-flagged MAJOR (test-mode
inject race) is actually fixed in the shipped diff, re-ran the full test suite, and
re-read every changed production file end to end.

## Verdict: PASS

## Findings

None BLOCKER, none MAJOR.

MINOR 1 (from round 1, builder disagreed) — RESOLVED, no code change needed.
`src/test/TestServer.h:122` `uint32_t lastInjectedOnsetCount_ = 0;` is plain, not atomic,
contra the plan's 2.6 comment example. Verified both read and write sites
(`src/test/TestServer.cpp:60-61`, inside `injectSnapshot()`) execute after
`std::lock_guard<std::mutex> lock(injectMutex_)` at `:50`, in the same function, same lock
scope — no other function touches the member (`grep -n lastInjectedOnsetCount_`, 2 hits,
both in `injectSnapshot`). The mutex is sufficient synchronization; an atomic would be
redundant. Builder's disagreement is correct — no fix required.

MINOR 2 (downbeatDetected, `TopBar.cpp:232`) — correctly left out of scope. Plan section 6
and critic's "no scope creep" pass both explicitly defer it; CLAUDE.md pitfall #30
(`CLAUDE.md:1101`) states "`downbeatDetected` has the same one-hop class and is NOT yet
covered", so the gap is disclosed rather than silently dropped. Fixing it would be
undiscussed scope creep on this lane.

## Independently re-verified (not just re-read the report)

1. **Critic's MAJOR (plan-onset-render.md, TestServer/ApiServer inject race) is actually
   fixed in code**, not just claimed. Diffed `src/api/ApiServer.cpp/.h`,
   `src/test/TestServer.cpp/.h`, `src/MainComponent.cpp`: both inject entry points
   (`ApiServer::handleInjectFeatures` and `TestServer::handleInjectFeatures`) now build an
   `InjectedOnsetCount` (request INTENT only — explicit count or bump flag) and pass it
   through to the single `TestServer::injectSnapshot(snap, onsetIntent)`, which resolves
   `onsetIntent.resolve(lastInjectedOnsetCount_)` and writes the result back, all under
   `injectMutex_` (`TestServer.cpp:44-62`). Two concurrent injects now serialize on the
   lock and land on baseline+2, not both on baseline+1 — the exact race the critic
   constructed is closed. `InjectedOnsetCount::resolve`/`::zero()` in
   `src/features/OnsetPulse.h` match the plan's amendment intent.

2. **RT/thread-ownership rules**: `OnsetPulse` (`src/features/OnsetPulse.h`) is a plain
   integer-arithmetic class, no allocation, no locks, no atomics — correct for a
   GL-thread-owned member. `Renderer::onsetPulse_`/`frameSnap_` are GL-thread-confined
   (`Renderer.h:305-315`); `onsetPulseFrames_` is the one cross-thread field and is
   correctly `std::atomic<uint32_t>` with relaxed ordering (write on GL thread, read from
   HTTP threads via `getOnsetPulseFrames()`). `OutputRenderer` and `AudioReadoutPanel` each
   get their own `OnsetPulse` instance on their own thread — no sharing, matches the plan's
   explicitly-rejected "per-uploader" alternative being avoided.

3. **Frame-loop reorder** (`Renderer.cpp:222-237`): the bus read + pulse derivation now
   happens before the nothing-to-render early return (`:240-247`), exactly per plan 2.2;
   `renderSource()` (`:1120-1126`) now reads `frameSnap_` by reference instead of a second
   `featureBus_.read()`, closing the per-source-clip race the plan's tradeoffs section
   identifies.

4. **Tests are genuinely fail-first, not tautological**: `tests/test_integration_pipeline.cpp`'s
   new case documents the pre-fix numbers in a comment ("25 == 57 at 60 fps... 83 at
   120 fps") and asserts both the fix (`framesPulse == detectedOnsets`) and the defect
   direction that would fail without it (`framesRawTrue < detectedOnsets` at 60fps,
   `> detectedOnsets` at 120fps) — this is a real before/after oracle, not a test written to
   match the implementation. `tests/test_onset_pulse.cpp`'s wrap/reset/backwards-jump cases
   and the arithmetic 60/120 Hz sampling-model case are consistent with `OnsetPulse.h`'s
   documented contract. `tests/visual/test_onset_pulse.py` (Eyes) asserts on the
   `onset_pulse_frames` counter, not PSNR, matching the plan's stated rationale (a
   converging stateful-source PSNR diff is not a usable one-frame oracle).

5. **Ran the suite myself** (did not trust the builder's number): `cd build-lane && ctest
   -j1` at unchanged HEAD `51d614d` → **417/417 passed, 0 failed, 7.14s** (my own run,
   independent of the builder's report).

6. **Scope/commit hygiene**: `git status` in the lane worktree shows only untracked build
   dirs (`build-lane/`, `build-lane-ts/`) — no uncommitted source changes, matches the
   builder's "no new commits" claim. Two commits on the branch (`4532779` fix,
   `51d614d` docs) — no unrelated edits found in the diff; every changed file
   (`OnsetPulse.h`, `Renderer.*`, `OutputWindow.*`, `AudioReadoutPanel.*`, `ApiServer.*`,
   `TestServer.*`, `MainComponent.cpp`, tests, CLAUDE.md, APP-INVENTORY.md, notebook.md,
   TESTING.md, probe script) is accounted for by the plan.

## Not independently re-run this round (Harmony's live gate, per builder's own disclosure)
Eyes `tests/visual/test_onset_pulse.py` and `.harmony/probe-onset-render.sh` require a
running `--test-mode` app with a visible GL context; I did not launch the app. This
matches the builder's own HANDOFF-NEEDS and does not block a source-level PASS — the
source-level fail-first coverage (item 4 above) is sufficient discriminating evidence
that the fix works; the live gate is a separate, still-open verification step for Harmony.

## Recommendation
PASS. No code changes required. Remaining work before full close: Harmony's live gate
(Eyes test C + probe D), and `downbeatDetected`'s identical defect class needs its own
lane if it is ever fixed (explicitly out of scope here, disclosed in CLAUDE.md pitfall #30).

METADATA: reviewer=review-agent, builder_packet=s-rta-0924b, date=2026-09-24
