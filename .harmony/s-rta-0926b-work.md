# s-rta-0926b — running work log (secondary, MINIMAL, ultracode/workflows)

Boot 2026-09-26 ~18:37. HEAD bc69fd0 (code = 6e8f120), unpushed 0. build/ binary 18:12:35 (= 6e8f120). Disk 378 GB free.
10 cores. No app running, no live lock, no worktrees. Dirty at boot (not ours, leave): .harmony/.harmony-version, AGENTS.md (untracked).

## START HERE items
1. DONE — first-call token number 54,790 (ctx-now.sh first call; <= 75,000 PASS, was 101,568). Up-channel:
   idea-ledger idea-2026-09-26-RealTimeAudio-1790462247885395697; inbox status-note updated.
2. Routine feel questions (9, incl. same-BPM realign product call) relayed to Boris in chat 18:4x. Awaiting answers.
3. Render follow-ups (xfade-report §5) — diagnose LIVE.
4. BPM thread safety (setManualBPM / Tap from message thread; Link per-tick realign).
5. Recorder provisional save empty tempo map; pre-ebbff22/e5ceb98 takes (no migration).

## Plan — wave 1 (one workflow, 3 worktree lanes, live app serialized by /tmp/audiodna-live.lock)
- R render (opus builder, worktree): apply parity-trace.diff in a scratch build; live-diagnose R1 outgoing-clip temporal
  key during crossfade, R2 persistent-layer keys across decks, R3 applyTransition ignores outgoing transform, R4
  compositePersistentLayers skips stages, R5 temporal FBO created mid-pass. Report BEFORE fix. Fix only the clear ones
  (with RED-first reproducers); R1 (per-clip temporal history) stops at options -> Fable ruling -> wave 2.
- B bpm (opus builder, worktree): TSan/unit reproducer of the message-thread writes; route setManualBPM/Tap through an
  analysis-thread request like requestResync; Link per-tick same-tempo must not realign; explicit same-BPM set_bpm
  semantics UNCHANGED (Boris question 9 pending).
- C recorder (sonnet builder, worktree): provisional save carries a tempo map; count old takes on disk; migration = options only.
Each lane: builder -> pinned reviewer -> <=1 fix round. Harmony: RED new probes on build/ (6e8f120) BEFORE merge,
merge, cmake -S . -B build + rebuild, ctest, probes GREEN, look at frames.
- 18:5x launched wave 1 w4aozsb4x (wf_41d6317f-a47): render (opus xhigh, worktree) / bpm (opus xhigh, worktree) /
  recorder (sonnet high, worktree); each -> pinned reviewer (sonnet) -> <=1 fix round; render forks -> 3 blind seats +
  Fable ruling (REPORT .harmony/.reports/s-rta-0926b/ruling-render-forks.md). Reports: .harmony/.reports/s-rta-0926b/.
