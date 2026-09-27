# s-rta-0927 — running work log (secondary, MINIMAL, ultracode/workflows)

Boot 2026-09-27 09:12 (from `date`). HEAD 4b0c39a (code = 6fe8abb), unpushed 0. build/ binary 06:05:50 (= 6fe8abb,
current). Disk 373 GB free. 10 cores. No app running, no live lock, no worktrees at boot.
Dirty at boot (not ours, leave): .harmony/.harmony-version, AGENTS.md (untracked).

## START HERE items (birth prompt s-rta-0926b)
1. plan5 outputs C1 -> C2 -> C3 (long; lane O). Boris Q1-Q7 relayed once in chat 09:1x; defaults ship.
2. Routine timing flake since canvas merge (lane T): discriminate b766720 vs main x5, then windows fix.
3. Routines display slice A (lane R): Fable build plan -> build -> review + critic panel.
4. Boris answers pending (defaults in force).
5. Loose ends 2-8 (spare-ring hitch -> lane T measure+options; tests/visual Tier-1 red -> later lane; F4 capture cost).

## Plan — wave 1 (one workflow, 3 worktree lanes, live app serialized by /tmp/audiodna-live.lock)
- W1 .claude/worktrees/rta0927-w1 — lane O outputs-c1 (opus xhigh): drift check of plan5 vs main (50 commits since
  its base 5285662) -> BLOCKED only on a broken structural assumption (-> Fable delta ruling) -> build C1 per plan5 §11.
  Reviewers: plan-conformance + GL/threading/screen-safety (sonnet high, pinned); one critic (visual+logic hats) on the
  output-probe PNGs. <=1 fix round.
- W2 rta0927-w2 — lane T routines-timing (opus high): x5 probe-routines on b766720 vs x5 on main app + capture
  durations; fix probe windows; spare-ring hitch = measure + options only (Fable later).
- W3 rta0927-w3 — lane R routine-display (Fable plan -> opus high builder) -> reviewer + 5 critics
  (visual/UX/graphic/logic/interaction-logic). <=1 fix round.
- Safety: NO lane opens the Output window; tests/visual/test_output_window_level.py is FORBIDDEN (it opens it); Tier-1 =
  the 5 named files only, never `pytest tests/visual/` as a directory.
- 09:17 wave 1 launched (wvp723n5k / wf_7b640deb-676): laneO outputs-c1 (opus xhigh W1; BLOCKED-on-drift -> Fable delta ->
  continue), laneT routines-timing (opus high W2), laneR routine-display (Fable plan -> opus high W3). Boris outputs Q1-Q8
  (no Q4) relayed in chat 09:17 — defaults ship.

## My own errors this session (no gate would surface them)
1. 09:17 launched the wave-1 workflow BEFORE syntax-checking it (rig rule: check before launch). Checked after: SYNTAX OK.
   Habit: run the node new-AsyncFunction check on the script text as its own step, then launch.
- 09:40 lane R plan (Fable) ADOPTED by Harmony: .harmony/.reports/s-rta-0927/plan-routine-display-A.md (7 commits; pads row
  in DeckView, bands in LayerStrip, V fader follows model + cyan cue, pad menu incl. Start Ease/Jump, Record tab pad row
  removed, UniversalParamControl ROUTINE cue last/deferrable). MERGE NOTE: lanes O and R BOTH claim pitfall 40 +
  CLAUDE.md index line -> the second to merge renumbers to 41. Lane O committed C1a 521a16c (drift check passed; section
  still being written).
- 10:48 LOCK STARVATION observed: lane O re-acquires /tmp/audiodna-live.lock right after each release (per-probe
  withlock.sh), so lanes T and R (20 s poll) have waited ~40-60 min. Rule for next packets: after releasing, a lane
  waits >= 45 s before re-acquiring (lets a 20 s poller win); or hand the lock over in FIFO via a queue file.
- 11:05 lane O builder DONE @ e14027c (7 commits): drift D1-D17 no structural break; ctest 690/690; 14 existing probes 0 FAIL
  on lane build; Tier-1 8 failed/5 passed on BOTH base and lane (identical sets); o_tap_cost +0.25-0.43 ms CPU @1080p;
  output_window_opened=false. Harmony LOOKED at probe_portrait_1080x1920 (letterboxed, upright) and
  probe_1920x1080_of_720p vs f720_render_frame (same picture, scaled) — correct. MERGE NOTES: CLAUDE.md now 24,992 B of
  the 25,000 cap (lane R adds lines -> must move content out at merge); lane build needs -DAUDIODNA_BUILD_TEST_SERVER=ON
  (check main build/ cache). found_not_fixed: currentImageFile_ write-only; EmbeddedShaders.h:3 stale comment;
  probe-canvas.py pgrep -x clang++ invalid regex; EffectChain.h deferred-boundary note stale.
- 11:09 ANOMALY (Harmony looked): lane O evidence f720_render_frame.png (canvas 1280x720, render_frame on the LANE build)
  shows the picture FOUR times in a 2x2 grid with (15,15,15) grey gutters; at 1920x1080 it is one full-bleed picture.
  The outputs critic flagged only the outer grey border (MUST). Unknown yet: pre-existing on main (canvas lane, runtime
  resolution change) or C1. Cheapest discriminating test: same probe step (set_composition_params 1280x720 ->
  render_frame) on build/ main app vs lane build. Will run it at the gate if the fix round does not.
- 11:13 lane R builder DONE @ f15f318 (8 commits C1-C7 + report): ctest 689/689; probe-routine-display new; 22 shots;
  output_window_opened=false. Reviews + 5 critics running. Lane O r1: conformance PASS_WITH_NITS, critic FAIL (grey
  border at 720p), GL review pending. canvas720-diag investigator dispatched (opus, main app, diagnosis only).
- 11:15 ANOMALY CLOSED (canvas720-diag, .harmony/.reports/s-rta-0927/canvas720-diag.md): NOT a bug — fixture B
  (media/P16_02_Screen_Split_2x2.png) IS a 2x2 grid on a #0F0F0F frame; the probe's resolution row captured B, not A.
  Deciding run: A at 1280x720 on main = one picture (d 0.013). Main correct at 1024x768/1280x720/4K, runtime + fresh;
  preview + snapshot identical. The outputs critic MUST is a false alarm -> steer the lane O fix round (probe row should
  compare like with like: trigger A first, or reference B at 720p). My own miss: I read "4 copies" as a render bug
  without first checking what the input fixture looks like — habit: before calling a picture wrong, look at the input.
- 11:24 r1 verdicts: lane O conformance PASS_WITH_NITS, GL PASS (SHOULD: never-key jassert is Release no-op), critic FAIL
  (false alarm, closed). Lane R review PASS; critics visual PASS, logic PASS, UX PASS (SHOULDs), graphic FAIL (cue cyan =
  macro/link cyan), interaction-logic FAIL (Waiting pad-menu edits don't reach the pending run). Fix rounds started;
  Harmony steered both via SendMessage: O = probe-side like-for-like row only, no app change; R = cyan DOWNGRADED (adopted
  kAccentCyan design, -> Boris check), fix the Waiting-edit MUST, restartPending display, tooltips OK, keep Boris-default
  behaviours, Pitfall 6 before switching x -> U+00D7.
- 11:30 HARNESS GOTCHA (my error #2): SendMessage to a RUNNING workflow agent does not deliver into it — it RESUMES a second,
  concurrent executor from the same transcript (same agentId). Lane O: two fix executors committed on one branch
  (5e1b7a1 by the workflow copy, 79b4410.. by the resumed copy); the workflow's r2 reviews pinned an intermediate head.
  Lane R: I TaskStopped the id -> it killed the WORKFLOW copy (no result -> laneR fixRoundFailed); the resumed copy (with my
  rulings) lives on as a standalone background agent and is the only writer in W3. Habit: never SendMessage a workflow
  agent mid-run; steer via the NEXT stage's prompt, or stop the workflow and resume from a script edit.
- 11:34 lane O gate: Harmony RED probe-outputs on pre-merge main app = PY 1 PASS / 9 FAIL, PROBE-OUTPUTS RED (11:31). Merged
  lane/outputs-c1 -> main e00b69a (local, unpushed). cmake + build OK; ctest 691/691. Full battery + probe-outputs on
  merged main running (b2m2tl0sj). W1 worktree + branch removed (fixdelta reviewer redirected to the main repo — a
  standalone agent QUEUES a SendMessage; only workflow agents fork). Pending: fixdelta review, conformance r2.
- 11:55 battery on merged main (C1): outputs 10/0, render-state 31/0, crossfade 35/0, effects-parity 46/0, manual-bpm 22/0,
  resync 16/0, downbeat 14/0, routines 98/0, mastersignal 22/0, decktabs 6/0, canvas 15/0, fitmode 10/0; deckclock 9/1:
  d_return_hitch "/api/state: RemoteDisconnected". CHASED: the app did not crash or get quit (orderly shutdown at 11:50:12
  = the .sh's quit after python exited). Mechanism (constants VERIFIED, race INFERRED): the row sleeps away=5.0 s on an
  idle requests.Session keep-alive connection; cpp-httplib CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND = 5
  (build/_deps/httplib-src/httplib.h:26) -> the server closes the idle socket as the client reuses it. Probe flake,
  independent of C1. Fix (probe side, later lane): retry idempotent GETs once on connection errors, or away != 5.0.
  Discriminating check: re-run deck-clock after the battery.
- 11:56 MY ERROR #3: the lane R fix executor that SURVIVED my TaskStop is the one that NEVER saw my rulings (its transcript
  subagents/agent-ae72...jsonl has 0 "DOWNGRADED"; the workflow copy's transcript has it — and that is the one I killed).
  Result: b28e554 recoloured the routine cue to chartreuse kRoutineCue #b4ff2e (hue survey: the only empty band 60-120
  deg) against my "keep cyan" ruling. Habit: after a SendMessage/TaskStop on a shared id, VERIFY which transcript holds the
  message before assuming who is alive.
  DECISION (Harmony, on the merits): KEEP chartreuse as the shipping default — the critic's collision is functional (cyan
  = the app-wide mapped-knob accent on every screen), the design's own intent was a DISTINCT cue, and it is one constant to
  revert. It is a taste call that is Boris's: he gets a side-by-side (cyan vs chartreuse) artifact after the r2 critic
  panel; his word overrides.
- 12:06 deck-clock re-run on merged main: 9/1 AGAIN (same row d_return_hitch, RemoteDisconnected); step3 re-run 94/0 (first
  failure = compiler load). KEEP-ALIVE THEORY REFUTED by a direct test: 7 idle gaps 4.0-6.0 s incl. three at 5.0 s all OK
  on BOTH main and the pre-C1 app. deck-clock on the pre-C1 app (W2 build-lane = 4b0c39a code) 10/0. => C1 REGRESSION
  (2/2 fail on main vs pass pre-C1; the lane's own run passed once). NOT pushed. Diagnosis+fix lane dispatched (W4).
  My error #4: I wrote the keep-alive mechanism into the log as the likely cause before running the one test that could
  refute it. Habit: run the cheapest discriminating test before writing a cause down (the constants matched; the
  behaviour did not).
- 12:32 Harmony_Main e2e battery (another session, pid 61324) held the drain-dispatch gate 12:0x-12:30 (write-capable
  dispatch blocked). Then: c1-state-fix lane dispatched (opus, W4 fix/c1-state-0927 from main e00b69a). Lane R fix round
  DONE (b6e49fa: F1 Waiting edits reach the pending start, F2 chartreuse cue, F3 red Delete, F4 restart mark, F5
  tooltips, F6 probe rows). Harmony LOOKED at crop-after-03-playing-deck vs crop-fixround-before (cyan) — both render as
  designed; chartreuse clearly distinct. Round-2 review + 5 critics launched (w26nwxiy0 / wf_da205ee1-46d).
- 12:42 lane R round 2: review PASS; critics visual/logic/UX/graphic PASS; interaction-logic FAIL (MUST: pad-menu edits during
  a RESTART wait do not reach the in-flight restart — same class as r1, disclosed by the builder). Fix round 2 dispatched
  (opus, W3 from b6e49fa): restart-wait resync + ROUTINE hint contrast >= 7:1. Boris questions carried: pad row vs column
  row look-alike (Q1), band x / layer X no confirm (Q2/Q3), restart mark salience, cyan vs chartreuse cue.
- 13:31 CORRECTION (my error #5): c1-state-fix PROVED the keep-alive race (server [KAI] log: keep-alive timeout fired 0.66 ms
  after the client's send; the 160-byte GET drained). It fails ~1 in 3 runs on the pre-C1 app too (pre1 RED). My
  "REFUTED" rested on 3 samples at a 2-6% per-attempt edge = no power; my "C1 REGRESSION" rested on n=1 pre-C1 pass.
  Habit: before calling a flake theory refuted or a regression proven, size the sample to the failure rate (a 1-in-3
  run flake needs >= 5 runs per arm). Fix = probe-deck-clock Connection: close (8654b74); merged. Latent: other
  Session-based probes (canvas/render-state/fitmode/outputs/...) — same one-liner in a probe-hardening lane.
- 13:31 lane R gate: Harmony RED probe-routine-display on pre-merge main = 4 PASS / 13 FAIL. Merged lane/routine-display ->
  main 20cede2 (doc conflicts resolved by Harmony: lane R pitfall renumbered 40 -> 41 in pitfalls.md + CLAUDE.md index +
  UI Patterns ref; CLAUDE.md "Kick off phase N" protocol moved verbatim to docs/claude/phase-protocol.md + trigger row,
  CLAUDE.md 26,437 -> 22,452 B). Rebuild OK; ctest 720/720. fix/c1-state merged on top.
- 13:32 battery on main f4507e8 (C1 + routine display + probe fix) running (b7a8yqhed). W3/W4 removed. C2 lane launched
  (w0jnoeofi / wf_53a3c17a-58f, W5 from f4507e8, opus xhigh; reviewer + 4 critics incl. interaction-logic; fix round
  prompt tells the builder to VERIFY each finding before fixing). Lane T has a commit (9cbfbd5).
- 14:01 GATE GREEN on main f4507e8: outputs 10/0, routine-display 16/0, render-state 31/0, crossfade 35/0, effects-parity
  46/0, manual-bpm 22/0, resync 16/0, downbeat 14/0, routines 98/0, mastersignal 22/0, decktabs 6/0, canvas 15/0,
  deckclock 10/0, fitmode 10/0, step3 94/0, tempo witness GREEN; ctest 720/720. No Audio-DNA running; Output window
  never opened (o_no_window_opened). Pushed.
- 14:03 probe-hardening lane dispatched (sonnet high, W6 from 8573006): Connection: close in canvas/fitmode/outputs/
  render-state probes, probe-canvas pgrep clang++ regex, probe-manual-bpm chmod, stale comments EmbeddedShaders.h:3 +
  EffectChain.h. Boris page .harmony/.reports/s-rta-0927/boris-checks.html written + OPENED (cue colour side-by-side,
  pad row look-alike, no-confirm stops, restart mark, outputs C1 checklist, older defaults).
- 14:31 probe-hardening merged -> main a302dcb (inline gate: probes GREEN on main app per lane; src diff verified
  comment-only by Harmony), W6 removed, pushed (unpushed 0). In flight: lane T (wave-1 workflow, 1 commit), C2 (W5).
- 14:46 lane T merged -> main 1636785 (review PASS_WITH_NITS; notebook conflict concatenated). Harmony gate: probe-routines
  x3 on current main app running (b1yoxmr7e). Wave 3 launched (wfnmf1wzk / wf_369fe37a-523): beatclock (Fable draft ->
  3 blind seats rt-safety/musical-timing/minimal-change -> Fable final -> opus builder W7) + renderperf (Fable plan: spare
  ring lazy alloc + capture memcpy -> opus builder W8); pinned reviewer each, <=1 fix round. C2 still in W5.
  Remaining queue: C3 (after C2), restore-at-routine-start 38-86 ms message-thread hold (diagnose), tests/visual Tier-1
  red (8 failing ids, investigate), DeckView redundant setLookAndFeel, tests/CMakeLists stale comment.
- 14:52 Harmony gate lane T: probe-routines on current main app x3 = 98/0, 98/0, 98/0 (quiet, clang=0). W2 removed. Pushed.
- 15:44 C2 lane: builder DONE 383f6bf (ctest 729; 15 probes GREEN on lane; never opened a window); r1 review PASS_WITH_NITS,
  critics visual/UX/logic/interaction-logic all PASS (SHOULDs -> C3: mode-change reconcile + displays refresh,
  classifyOutputKey Shift-up; native menu shortcut text = JUCE limit; Identify Displays = slice 2). Harmony looked at the
  menu render (ticked Display 2, main flagged, All Outputs Off + chord) and the TopBar "Outputs: Off" crop. Harmony RED
  on pre-merge main app: probe-outputs PY 10 PASS / 3 FAIL (o_state_displays). Merged -> dc7adf9 (probe-outputs comment
  conflict: kept HEAD's; one Connection: close line). Rebuild OK; ctest 729/729. Battery running (bg9clidtz); first 10
  probes GREEN incl. outputs 13/0. C3 launched early (wuptg6kdn / wf_782b31de-a3f, W9 from dc7adf9) with settings.json
  safety (backup/restore sha256 or test-only path override).
- 16:04 C2 battery: probe-canvas sat 26 min in wait_no_compiler (3 build lanes compiling) while holding the live lock -> Harmony killed its python (TERM; the .sh quits the app); canvas (+ step3 if load-flaked) re-run at a quiet moment. Rig learning: a perf-waiting probe must not hold the live lock while waiting for idle CPU.
- 16:14 same for probe-fitmode f_perf (wait_no_compiler limit 1800 s) -> killed; canvas + fitmode (+ step3 if load-flaked) queued for a quiet re-run. Rig fix to file: perf rows must not wait while holding the live lock (skip-with-note under load, or release/re-acquire around the wait).
- 16:20 C2 gate GREEN on main dc7adf9: outputs 13/0, routine-display 16/0, render-state 31/0, crossfade 35/0, effects-parity
  46/0, manual-bpm 22/0, resync 16/0, downbeat 14/0, routines 98/0, mastersignal 22/0, decktabs 6/0, deckclock 10/0,
  step3 94/0, tempo GREEN; canvas 12/0 + fitmode 10/0 on the correctness rows (row filter; perf rows c_perf_1080/4k +
  f_perf DEFERRED to a quiet window — 3 lanes compiling). ctest 729/729. Pushed.
