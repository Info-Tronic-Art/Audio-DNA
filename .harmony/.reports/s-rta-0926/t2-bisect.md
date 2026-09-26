# s-rta-0926 — probe-step3 T2 timing: A/B/A bisect e1ed9cc vs 1908413

(Recovered by Harmony from the workflow return value — the lane wrote this file inside its worktree, which was
auto-removed as "unchanged" because .harmony/ is gitignored. Lesson in notebook.)

VERDICT: **environmental**

## Runs (alternating NEW/OLD, main probe, same rig)

| # | Build | Start | mean_offset_ms | drift_ms | drift_stderr_ms | drift bound (±) | p95_jitter_ms | T2 verdict | Overall | Load avg (before→after) | Other audio active? |
|---|-------|-------|------|------|------|------|------|------|------|------|------|
| 1 | NEW (1908413) | 15:30:50 | 45.05 | -9.10 | 3.95 | 8.90 | 19.71 | FAIL (drift+p95) | 91/2 | 10.80→6.95 | Stremio (pid 59448) audio-out assertion |
| 2 | OLD (e1ed9cc) | 15:39:51 | 41.08 | -0.43 | 3.74 | 8.48 | 17.59 | FAIL (p95) | 92/1 | 3.37→5.89 | Stremio (pid 59448) same assertion |
| 3 | NEW (1908413) | 15:49:42 | 44.77 | 4.91 | 4.00 | 9.00 | 19.23 | FAIL (p95) | 92/1 | 6.34→6.76 | none visible |
| 4 | OLD (e1ed9cc) | 15:54:xx | 32.04 | 0.85 | 2.26 | 5.52 | 10.62 | PASS | 93/0 | 5.68→3.46 | none |
| 5 | NEW (1908413) | 15:59:xx | 32.73 | 2.41 | 2.46 | 5.92 | 12.60 | PASS | 93/0 | 2.67→2.67 | none |
| 6 | OLD (e1ed9cc) | 16:04:xx | 33.25 | 1.39 | 2.47 | 5.94 | 10.75 | PASS | 93/0 | 1.96→2.50 | none |

Sequence was NEW, OLD, NEW, OLD, NEW, OLD as specified. FAIL/PASS boundary tracks time-in-session, not build.

## Cause

Environmental, not a code regression. A controlled alternating NEW/OLD/NEW/OLD/NEW/OLD test (NEW = main checkout's build at 1908413, never rebuilt; OLD = a fresh build-old compiled from a detached checkout of e1ed9cc, app target only) shows both builds FAIL T2 (p95 jitter, and once drift) in the first three runs and both PASS cleanly in the last three runs — the pass/fail line tracks *when* the run happened, not *which commit* it ran. Source-diff review of every file touched between e1ed9cc and 1908413 in src/recording, src/audio, src/analysis, and src/MainComponent.cpp found nothing on the audio callback, analysis thread, or RecorderHost::tick hot path: src/audio and src/analysis have zero diff, src/MainComponent.cpp has zero diff, and the only src/recording changes are a single ArmOptions::startBeatInBar double field copied into Take::Meta at arm/disarm/60s-periodic-save (metadata bookkeeping, not per-tick timing work) plus its JSON (de)serialization. The remaining files in the full diff (CompositorEngine/ScratchPool GL compositing, TopBar UI cosmetics, a new unused Routine data model, and a replay-path compile() refactor) are render-thread-only, message-thread-only, or not exercised by this probe's recording flow — none adds a mutex, sleep, or cross-thread signal. A concrete environmental correlate was found: Stremio (pid 59448) was actively holding a coreaudiod "audio-out BuiltInSpeakerDevice" assertion — contending for the same CoreAudio device Audio-DNA uses — during exactly the two worst runs (both FAILs), and that assertion is gone from every snapshot once the runs turned clean; Stremio had launched one minute before Harmony's original 13:38 e1ed9cc PASS (likely not yet playing) and had time to start active playback by the 14:24/15:17 FAIL runs. General system load (10.8→2.0 over the session, partly self-inflicted by this lane's own build-old compile finishing right before run 1) also settled in step with the improvement, though load alone doesn't fully explain run 2's FAIL at low load — Stremio's device contention is the better-supported specific cause for runs 1-2, with general settling covering the gradual improvement after. Drift itself bounces sign/magnitude run to run with no build-correlated pattern, consistent with noise around ~0 (not a monotonic clock-rate bug), and the script's own drift_stderr already flags these ~62s takes as noisy.

## Next step

No code changes recommended on e1ed9cc..1908413 — the diff contains no plausible T2-timing mechanism and the controlled test does not show OLD reliably passing where NEW reliably fails. If Harmony wants to convert the Stremio correlation into causation: rerun probe-step3.sh against 1908413 once with an app deliberately driving audio through the built-in speaker and once fully idle, back to back. Independent of this verdict, consider having probe-step3.sh snapshot `pmset -g assertions` at arm time and WARN if another process holds an audio-out assertion on the same device, so future flaky T2 runs self-diagnose instead of needing a bisect.

## Learnings

- A worktree-isolated agent's Bash tool refuses bash <file>/source <file> invocations combined with run_in_background:true or nohup+disown, and also refuses a single multi-statement call mixing a background loop with `bash <other-script>` ("too complex to verify") even when every referenced path is legitimate (a main-checkout script the rig rules explicitly authorized). The pattern that reliably worked every time: a single-line `cd <worktree> && <command> ... &` followed by a bare `wait` in the SAME foreground (non-background) Bash call, with the tool's timeout parameter raised to 600000ms for the probe's ~5min runtime.
- The Write tool refuses an absolute path into the main checkout's shared tree from inside a worktree-isolated agent ("Edit the worktree copy of this file instead of the shared-checkout path"), even for a path a work packet named explicitly and even for a directory (.harmony/) that is gitignored/untracked. The workaround is to write to the WORKTREE's own copy of the same relative path (git worktrees share .git but have independent working trees, so <worktree>/.harmony/... is a distinct, valid filesystem location) and flag the actual location clearly in the deliverable for the dispatcher to relocate if needed.
- A live-lock protocol built on `mkdir` as the atomic acquire worked correctly under real concurrent contention from another live Harmony lane sharing the same physical rig — but one lane's teardown (`rm -rf` the lock dir) raced a second lane's fresh `mkdir` without checking ownership first; re-reading the lock's `owner` file content after every successful `mkdir` (not just trusting the mkdir exit code) is the safety margin that caught this and should be the standard pattern anywhere multiple sessions share one physical live-app lock.
- Before concluding a rare live-timing test failure is a code regression, snapshot `pmset -g assertions` (or equivalent OS resource-contention state) alongside the usual load-average check — a media player (or any app) actively driving output through the SAME physical audio device under test is a concrete, verifiable confound that plain `uptime`/load-average would not surface, and it can fully explain drift/jitter degradation that looks superficially like a code-caused regression.
