# s-rta-0926 — running work log (secondary, MINIMAL, ultracode/workflows)

Boot 2026-09-26 ~12:55. HEAD 51a34a1, unpushed 0. build/ binary 12:27:30 (includes 4fca2c5 ms-white2). Disk 255 GB free.
Dirty at boot (not ours, leave): .harmony/.harmony-version, AGENTS.md (untracked).

## Plan — wave 1 (one workflow, 4 lanes; live app serialized by lock dir /tmp/audiodna-live.lock)
- A parity (worktree, opus builder): live fixture matrix on main build -> instrumented scratch build -> diagnosis report
  BEFORE fix -> fix -> probe RED(main)/GREEN(scratch) with a parity-to-reference oracle -> reviewer.
- B step3 row (main tree, probe only, no commit): poll inputSource after /api/perf/play -> timing vs real bug.
- C step-7 docs (main tree, no commit): plan-roadmap §4 rows, every count derived by command.
- D polish (worktree): Deck Loa clip + No-clip-selected overlap -> window-only shots (startup state) -> critic panel
  (visual/UX/logic) incl. the Master Signal fader.
- Wave 2 (after merge + my gates): L-R Routines slice 1 (plan-roadmap §1).
Harmony gates after merge: rebuild build/, ctest, probe-effects-parity (new oracle), probe-mastersignal, probe-step3,
look at one decoded frame per probe.
- 13:1x launched wave1 wp3pdyzz7 (wf_c26c7de7-792): A parity opus builder worktree -> reviewer (+1 fix round); B step3row sonnet (main tree, no commit); C docs7 sonnet (main tree, no commit); D polish sonnet worktree -> 3 critics (+1 fix round).
- launched routines plan w7q00o6xb (wf_5c800da5-0a8): Fable plan -> 3 blind critics -> Fable ruling -> plan-routines-s1-final.md. Build waits for wave-1 merge.
- wave1 DONE: parity = NOT reproducible on 4fca2c5 (0/104; misread "2 PASS / 3 FAIL"), crossfade clobber found (merged df52585);
  step3row = probe timing, fixed 8e1906d, Harmony gate 93/0; docs7 0bf0ad3 (counts re-derived by Harmony: 35 routes, 7 perf,
  OSC 13); polish merged e1ed9cc (reviewer PASS_WITH_NITS; critics PASS on both fixes, FAIL on Master Signal fader label).
  build/ rebuilt at e1ed9cc: ctest 539/539.
- wave2 wde415p6u: xfade (opus, worktree) + fader (sonnet, worktree, critics, reviewer).
- routines plan DONE (plan-routines-s1-final.md: 7 ACCEPT, 0 REJECT; lanes 1a -> Harmony gate -> 1b -> optional bank strip).
  Lane 1a launched w9c64dgcm (opus). 2 non-blocking Boris confirmations in plan section 10.
- 14:0x Harmony behavioral gate polish on build/ e1ed9cc (window-only shot, looked at): 'Deck Load' full word; 'No clip selected' below the 8 Dashboard knobs, no overlap. PASS.
- GATE 1a (build/ f43b72a, cmake -S . -B build + rebuild): ctest 546/546; probe-mastersignal 22/0; probe-step3 92/1 --
  the FAIL is "T2 alignment: p95 jitter 18.37 ms > 15" (mean offset 42.97, was 10.84/33.16 at 13:38 on e1ed9cc).
  Load average 24 on 10 cores during the run (3 clang at 99% from wave-2 lane builds). INFERRED: CPU contention
  (lane 1a touched compile/replay + take meta only; T2 is the capture/onset path). OPEN until the DISCRIMINATOR runs:
  probe-step3 on the SAME binary with no builds running; p95 <= 15 -> load; > 15 -> real, bisect f43b72a vs e1ed9cc.
  All 92 other rows PASS incl. every replay/snap-back row (the refactor's surface). 1b launched on this basis.
- wave2 merged: xfade 0ecd7a7 (reviewer PASS_WITH_NITS), fader 1908413 (critics: 2 FAIL verdicts VOID - they read main /
  the xfade worktree; visual PASS; reviewer FAIL on 0-4 px label margin at 1728 pt). Harmony RED first on f43b72a:
  probe-crossfade 9 FAIL. build/ 1908413 (cmake -S . -B build): ctest 556/556; test_master_signal_link 14/14.
  Harmony gates: probe-crossfade 27/0 GREEN (mid frames looked at: real blend); probe-effects-parity (hardened) 39/0;
  probe-mastersignal 22/0; window-only shot: "Master Signal:" full + 1.00 readouts + magenta vs cyan at 1728 pt
  (reviewer's margin concern closed at the default size; narrower windows = follow-up).
- inbox routed item (primary s234): split CLAUDE.md <= 25 KB -> workflow wjx9xqbu6 (no-loss proof + fresh-reader test).
