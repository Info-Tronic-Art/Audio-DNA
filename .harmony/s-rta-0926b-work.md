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
- 20:1x wave 1 DONE (10 agents): render R1 VERIFIED (fork) + R2-R5 VERIFIED+fixed, reviewer PASS; bpm race TSan-proven
  + fixed, Link 30 Hz phase reset fixed, reviewer PASS_WITH_NITS; recorder early tempo save fixed, reviewer PASS_WITH_NITS.
  Fable ruling (ruling-render-forks.md): R1 = B' (per-layer outgoing slot: copy buffer, swap ring); R4-opaque (1) blend
  over + honour opacity; R4-router (1) never publish; R4-types FX Only + media-less yes, Mask/3D no + toggle disabled.
  HARMONY DECIDES: adopt all four. C-migration: none. LINK-RAMP: (b) a Link tempo change never realigns (wave 2).
- RECEIVER-VERIFY caught: recorder lane census WRONG (read key tempo/a, disk is tempoMap): real 158 takes, 139 empty,
  0 empty after ebbff22; the "7 anomalous takes" do not exist. Notebook + report corrected (30143db).
- Harmony RED on build/ 6e8f120: probe-render-state "PY 1 PASS / 18 FAIL" RED; probe-crossfade k,l "PY 4 PASS / 4 FAIL"
  RED; tempo witness (take.json 1.5 s after Record) tempoMap [] RED.
- merged 7713aa4 render, efa40ab bpm, c9823ef recorder; build/ reconfigured + rebuilt 20:23; ctest 588/588 (serial).
- MY ERROR: `cd .harmony && ...` moved the main loop cwd (same as s-rta-0926 error #5). Restored. Habit: no `cd` in the
  main loop at all — absolute paths, git -C, script files.
- Harmony GREEN on merged build/ (5f84899 code = c9823ef): render-state "PY 19 PASS / 0 FAIL"; crossfade "PY 35 PASS / 0 FAIL";
  effects-parity "PY 46 PASS / 0 FAIL"; manual-bpm 18/0; downbeat 14/0; routines 74/0 (pause 1.8); mastersignal 22/0;
  tempo witness GREEN (tempoMap start anchor bpm 120 at 1.5 s). Frame looked at: k_outgoing_transform refA/mid02/05/08/refB
  strip — real blend of the scaled outgoing clip into the incoming grid.
- resync 15/1 once (V2 opacity 0.3987 at 64 ms) -> A/B/C x2 (merged / bpm-only W2 / render-only W1): 6/6 16/0 (V2 0.003-0.009
  at 62-108 ms). NOT a regression. INFERRED cause: load avg ~13 right after build+ctest delayed state publish past 64 ms.
- step3 T2 p95 FAIL on ALL three builds (merged 19.57, bpm-only 17.29, render-only 16.17; all other 93 rows PASS). Render-only
  touches no analysis/capture code -> ENVIRONMENTAL (Stremio running ~25% CPU, same as s-rta-0926 bisect). Cheapest
  discriminating test: re-run probe-step3 on build/ with Stremio closed (Boris's app — ask).
- 20:5x wave 2 launched (wf_b6cc814e-3f6): render2 (W1, opus: ruling steps 1-4 + empty-active-deck persistence + docs),
  bpm2 (W2, opus: LINK-RAMP (b) + default-build Link toggle disabled/never feeds 120 BPM + nits/docs), probehygiene
  (isolated, sonnet: probe-effects-parity fresh dir + render_frame check). Paused ~10 min for a quiet step3 run.
- Boris closed Stremio 20:5x -> probe-step3 on merged build/ x2: 94 PASS / 0 FAIL both (p95 10.11 / 10.75 ms). T2 miss was
  ENVIRONMENTAL (Stremio) — CLOSED. Wave 2 resumed (wu2yiazvi).
