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
