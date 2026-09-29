# asyncload -- lane report (s-rta-0929)

## BUILDER REPORT

STATUS: DONE_WITH_CONCERNS
RESULT: Composition / Load Deck / Duplicate Deck loads are STAGED: videos open on a 2 x low MediaOpener pool, the UI stays live (16 x 4K: 9-16 ms message-thread stall, was 244-257 ms), the old show plays until a clean cut, POST /api/load_composition answers after the swap; branch lane/asyncload, 8 commits on main adf9b8a, probe-async-load GREEN x2, ctest 942/942.
FACTS: commit-1 gate `.harmony/probe-async-load.py` a2 (i): opens_ms / total_ms 0.982 and 0.977 (a2b 0.945 / 0.943) -> PROCEED; staged flow `src/MainComponent.cpp` (beginStagedOpen / onStagedLanded / finishStagedLoad / cancelStagedOpen / pumpLoadQueue / setFileLabel); pool `src/core/MediaOpener.cpp`; pure bookkeeping `src/core/StagedLoad.h`; ticket `src/core/LoadTicket.h` + the REST wait and stop() release in `src/api/ApiServer.cpp`; timing `src/core/LoadTiming.h`; TEST-ONLY audio witnesses `src/audio/CombinedCallback.h` / `src/audio/AudioCallback.cpp`; ctests `tests/test_load_ticket.cpp`, `tests/test_staged_load.cpp`, `tests/test_media_opener.cpp`; live probe `.harmony/probe-async-load.sh`; docs `docs/claude/rendering.md` ("Asynchronous loads"), `docs/claude/pitfalls.md` (NN), `CLAUDE.md` 24,363 B.
METHOD: plan-asyncload + HARMONY ADOPTION AL1-AL12, one commit per item; RED on the main app (adf9b8a) and on the commit-1 app for every probe row, RED by absence for every ctest; teeth on scratch copies (ctest) and one TEMPORARY env-switched live build reverted by blob hash; perf only under acquire_quiet_lock with load average printed; GREEN = two consecutive full runs of the final app (sha256 2c877262c6a80efd).
CONFIDENCE+VERIFY: high for the behaviour rows and the REST ordering (GREEN x2, RED on both baselines, teeth T1 T3 T5 T7 bite live); medium for perf margins (two unexplained environmental episodes below). Verify: `ctest --test-dir build-lane -j1` -> 942/942; under the live lock `ASYNCLOAD_APP=<build-lane app> bash .harmony/probe-async-load.sh <out>` -> `PROBE-ASYNC-LOAD GREEN` (phase 1 `PY 83 PASS / 0 FAIL`, phase 2 `PY 3 PASS / 0 FAIL`); `strings <app> | grep -c AUDIODNA_ASYNCLOAD_` -> 0; the AL9 (a) TEST_SERVER=OFF build: strings audio_xruns / audio_callbacks / audio_callback_gap_max_ms / audio_callback_period_ms / analysis_ring_overruns / ui_text / cancel_load all 0 (opens_pending 1), dir deleted.
UNKNOWNS/NOT-DONE: (1) an intermittent ~110 ms message-thread stall ~0.13 s after a 16-cell cut in two batches (10:33, 10:40) -- 10/10 loads then, 0 of 30+ since; cause unidentified. (2) one audio outlier (CoreAudio overloads +2, 160.9 ms callback gap) in 1 of 37 a2 windows; 10 more a2 + 10 no-load controls clean. (3) T6 (AL2) not run live (screen safety) -- forked-child ctest shows the kill path does not crash here, so AL2's row is a GUARD. (4) probe-video w6b (a) fails on main 4/5 and final 3/5 -- pre-existing. (5) the 1-thread pool arm (AL1) was not needed (late +0 / hold +0 in 5/5).
NUANCE: the plan's fixture (flat-colour ultrafast H.264) opens a 4K file in 15-17 ms, not the 68 ms the plan's numbers assumed: RED is 244-257 ms (not 1000-1200) and staged windows are 0.05-0.3 s, so rows read state right after a post instead of sleeping; the AL7 refusal / source-gone label notes are HELD like every other label write while a load is staged (visible only after a cancel); a failed newer Composition load (parse / validate) leaves a staged load staged (end state = the synchronous app's); an explicit cancel / New Composition / quit also drops the AL7 queue. R9 corrected (AL9 b): a callback-duration timer is not "only our memcpy" -- memory-bandwidth contention can move it; the OS overload count is used because it is the OS's own verdict and costs the callback nothing. R3 corrected (AL1): each open also runs FFmpeg's 2 frame threads, so 2 pool threads = up to ~6 runnable threads.
HANDOFF-NEEDS: none (Harmony: assign pitfall NN (58); merge; decide whether to chase finding 1 with the d1_grid_16_images row).

INBOX-RECHECK: none

### SUMMARY
Composition loads, Load Deck and Duplicate Deck now open their videos off the message thread (a 2 x low "MediaOpen" pool) against a STAGED model: the UI stays live (16 x 4K load: message-thread stall 9-16 ms on the final app, was 244-257 ms), the old show keeps playing and answering triggers until the cut, the cut shows no black / no pending frame, and POST /api/load_composition still answers only once the new composition is on screen. The commit-1 attribution gate passed (opens = 94-98 % of the load). probe-async-load is GREEN twice in a row on the final app; RED on main and on the commit-1 app recorded; teeth T1 T3 T5 T7 bite live, T2 / T5b / T7* / opener teeth bite in ctest, T6 is a GUARD (see AL2).

### ITEMS (plan commits + adoption rulings)
| item | commit | result |
|---|---|---|
| c1 measurement + gate | a51b2d7 | GATE PASS: a2 opens_ms / total_ms = 0.982 (256.1 / 260.9 ms, open_max 17.3) and 0.977 (241.9 / 247.6, open_max 15.5); a2b 0.945 / 0.943 -> PROCEED |
| c2 LoadTicket + StagedLoad | 48ad148 | 15 ctest cases; RED by absence; teeth on scratch copies |
| c3 MediaOpener | a303c52 | 6 ctest cases (20/20 runs green); RED by absence; teeth (no Stale check) FAILs (c1) |
| c4 staged flow + REST wait | f8181be | probe rows a1-a7 + a5b + ends GREEN x2 |
| AL2 hung-open ctest | cbaf62d | GUARD (T6 passes too, 5.81 s vs 5.27 s) |
| c5 docs | 68af318 | CLAUDE.md 24,980 -> 24,363 B; routine paragraph moved verbatim |
| probe d2 control row | 68bb175 | diagnostic (audio outlier control arm) |
| c6 report | (this commit) | |
| AL1 pool size | -- | a2 (h) late +0 and hold +0 in 5/5 (and 17/17 final-app a2 runs): the 1-thread re-run is NOT triggered; pool stays 2 x low |
| AL3 deterministic cancel ctests | a303c52 | (c1) start latch -> stale 2; (c2) workers held -> dropped 2, no landing |
| AL4 VideoPlayer.h comment | f8181be | text-only; Pitfall 56 clause in 68af318 |
| AL5 label hold | f8181be | a4 (f1) "Loading ...V32..." through a trigger; (f2) cancel after a trigger shows "top.png" |
| AL6 quit order | f8181be | servers stop first, then cancelStagedOpen, then detach (comment in ~MainComponent) |
| AL7 FIFO queue | 48ad148 + f8181be | a5b back-to-back: load.queued 1 at the barrier, two copies "A copy", "A copy"; T7 (newest wins) -> one copy |
| AL8 a/b/c + staged_players | a303c52 / a51b2d7 / f8181be | thumbnail pixel identity ctest; static_assert lock-free; removeAllJobs cited; /api/state load.staged_players |
| AL9 a/b/c + NIT | see below | (a) OFF build: see VERIFY; (b) R9 text corrected here; (c) a4 precondition row; NIT: rendering.md says open() logs from pool threads |
| AL10 CLAUDE.md payment | 68af318 | moved verbatim (1,172 B, 8/8 sentences present in recording.md) |
| AL11 member-order comment | f8181be | at the mediaOpener_ declaration |
| AL12 row set + battery | -- | see "Existing probes" |

### PROBE -- probe-async-load (final app, quiet lock, raw verdict lines)
GREEN run 1 (12:00:48, load 7.43): `PY 83 PASS / 0 FAIL` ... `PASS  phase1: app terminated 1 s after the quit` / `PASS  phase1: no new Audio-DNA crash report` / `PASS  phase1: 0 UserNotificationCenter windows on screen` / phase 2 `PY 3 PASS / 0 FAIL` / `PASS  phase2_hung_open: app terminated 7 s after the quit` / `PASS  phase2_hung_open: no new Audio-DNA crash report` / `PASS  phase2_hung_open: 0 UserNotificationCenter windows on screen` / `PROBE-ASYNC-LOAD GREEN`
GREEN run 2 (12:03:41, load 4.76): identical verdict lines (`PY 83 PASS / 0 FAIL`, phase 2 `PY 3 PASS / 0 FAIL`, quit 1 s / 7 s, `PROBE-ASYNC-LOAD GREEN`).

Per-row numbers (GREEN run 1 / run 2):
| row | numbers |
|---|---|
| a2 16 x 4K | stall max 12.1 / 9.4 ms (bar 50); answer (= cut) 0.146 / 0.151 s (bar 2.5); old clip uploads +3 in the window, playhead 0.473 -> 0.540 / 0.469 -> 0.538; fence black +0, hold +0; overloads +0, ring overruns +0, callback gap max 10.84 / 10.82 ms (period 10.67); late +0; timing: msg_ms 6.7, opens_ms (16 landings) 1.1, open_max (one landing) 0.23, swap 2.7, ui 2.2, total 144.0 ms |
| a2b 16 x 1080p | stall 15.9 / 13.3 ms; answer 0.064 / 0.054 s ((d) window < 0.1 s: not measurable, printed); black +0; audio clean |
| a3 cancel by newer | V32 "superseded by a newer load" 0.002 s after W3's post; W3 ok after 0.014 / 0.010 s; label "Loaded: ..._W3"; open_batches +2, stale +2, dropped +14; video_players 0; stall 19.8 ms; (g) V16b all 16 clips 1920 + thumbnail, video_players 16 |
| a3b / a3c explicit cancel | superseded 0.002-0.003 s after the cancel; W2 kept; label restored to the pre-staging text "base.png"; video_players 0; seq_open unchanged (a3c) |
| a4 trigger in window | V32 answered after 0.279 / 0.304 s; the trigger answered at 0.107 / 0.105 s and W2's col 1 was active at 0.110 s (the OLD show answered); V32 cut with every activeClipColumn -1; label "Loading a4_..._V32..." right after the trigger; (f2) cancel after a trigger -> "top.png" |
| a5 duplicate | "Loading A copy..." then "Duplicated deck: A copy"; 2 decks after 0.059 / 0.066 s, activeDeck 1; video_pending +0, videos_pending max 0; black +0, hold <= 3; dbox <= 3 vs the 4K reference; stall 4.5 / 3.1 ms; video_players 8 |
| a5b duplicate twice | 0.05 s apart and back to back: ["A", "A copy", "A copy"], activeDeck 2, video_players 12; back to back: load.queued 1 at the ui_text barrier |
| a6 append | "Loading deck16..." then "Loaded deck: deck16"; 2 decks after 0.151 / 0.167 s; 16 x 3840; stall 7.7 / 11.9 ms; audio clean |
| a7 failure mid batch | ok after 0.122 / 0.130 s; cell 7 (header-only H.264) 1920 / thumbnail 0, cell 9 mediaMissing, 14 x 3840; opens_failed +1; 4K on screen; 0 dialogs |
| end_hold_witness | fence hold +4 over the run, black +0 |
| end_quit_mid_load | V32 in flight (answered False = cancelled at quit); app terminated 1 s after the quit |
| end_quit_hung_open | FIFO load "superseded by a newer load"; W3 ok after 0.015 s; label "Loaded: end_quit_hung_open_W3" while a pool thread sits in open(2); quit 7 s (5 s wait + the AL2 leak line `[MediaOpener] an open did not return within 5000 ms at shutdown: its pool is left running (intentional leak)`), no .ips, 0 dialogs |

### RED -- probe-async-load on MAIN (adf9b8a app, 11:46:33, load 5.03): `PY 30 PASS / 49 FAIL`, phase 2 `PY 1 PASS / 2 FAIL`, `PROBE-ASYNC-LOAD RED`. FAIL lines verbatim:
```
FAIL  a1_witness_fields: /api/state has no 'load' object (got None)
FAIL  a1_witness_fields: GET /api/debug/ui_text answers the file label (got None / 404)
FAIL  a2_load_16x4k: (a) longest message-thread stall 254.8 ms <= 50.0
FAIL  a2_load_16x4k: (b) ok:true within 2.5 s (0.0034279823303222656) AND the composition read right after the answer is V16 (layer id 811 == 812, 1 clips)
FAIL  a2_load_16x4k: (g) CoreAudio overloads +None == 0
FAIL  a2_load_16x4k: (g) analysis ring overruns +None == 0
FAIL  a2_load_16x4k: (g) audio callback gap max None ms <= 2.5 x period (None)
FAIL  a2b_load_16x1080: (a) longest message-thread stall 77.4 ms <= 50.0
FAIL  a2b_load_16x1080: (b) ok:true within 2.5 s (0.0045528411865234375) AND the composition read right after the answer is V16 (layer id 821 == 822, 1 clips)
FAIL  a2b_load_16x1080: (g) CoreAudio overloads +None == 0
FAIL  a2b_load_16x1080: (g) analysis ring overruns +None == 0
FAIL  a2b_load_16x1080: (g) audio callback gap max None ms <= 2.5 x period (None)
FAIL  a3_cancel_by_newer_load: (a) V32 answers 'superseded by a newer load' within 0.5 s of W3's post (got True None, -0.1481328010559082)
FAIL  a3_cancel_by_newer_load: (b) W3 ok:true within 2.5 s (0.0018742084503173828) and on screen when it answered (False)
FAIL  a3_cancel_by_newer_load: (c) the label reads 'Loaded: a3_cancel_by_newer_load_W3' (got None / 404)
FAIL  a3_cancel_by_newer_load: (d) two batches began (open_batches +None == 2)
FAIL  a3_cancel_by_newer_load: (f) longest message-thread stall 532.7 ms <= 50.0
FAIL  a3b_explicit_cancel: (a) V32 answers 'superseded by a newer load' within 0.5 s of the cancel (got True None, -0.15121984481811523)
FAIL  a3b_explicit_cancel: (b) nothing swapped: W2 still on screen (layer id 842 == 841)
FAIL  a3b_explicit_cancel: (c) the label is restored to the text before staging None (got None / 404)
FAIL  a3b_explicit_cancel: (d) the cancelled batch's players are retired (video_players 32 == 0)
FAIL  a3c_cancel_with_sequences: (a) V32 answers 'superseded by a newer load' within 0.5 s of the cancel (got True None, -0.14477205276489258)
FAIL  a3c_cancel_with_sequences: (b) nothing swapped: W2 still on screen (layer id 852 == 851)
FAIL  a3c_cancel_with_sequences: (c) the label is restored to the text before staging None (got None / 404)
FAIL  a3c_cancel_with_sequences: (d) the cancelled batch's players are retired (video_players 32 == 0)
FAIL  a3c_cancel_with_sequences: (f) no sequence left open by the cancelled batch (seq_open 0 -> 2)
FAIL  a4_load_then_trigger: window too short to test (the load answered after 0.0029799938201904297 s <= triggerDelayS 0.1)
FAIL  a4_load_then_trigger: (c) V32 on screen with no active clip (the trigger did not leak into it; activeClipColumn [1])
FAIL  a4_load_then_trigger: (f1) the label keeps 'Loading a4_load_then_trigger_V32...' through the trigger (got None)
FAIL  a4_load_then_trigger: (f2) after a trigger + cancel the label shows the trigger's text 'top.png' (mid None, end None)
FAIL  a5_duplicate_deck: (a) the label reads 'Loading A copy...' then 'Duplicated deck: A copy' (got None -> None)
FAIL  a5_duplicate_deck: (b) the copy is the active deck within 2.5 s (activeDeck 0, None s)
FAIL  a5_duplicate_deck: (f) the copy has its own players (video_players 4 == 8)
FAIL  a5b_duplicate_twice (0.05 s apart): two Duplicate clicks = two copies, in order (names ['A'])
FAIL  a5b_duplicate_twice (0.05 s apart): the second copy is active (0 == 2)
FAIL  a5b_duplicate_twice (0.05 s apart): each copy has its own players (video_players 4 == 12)
FAIL  a5b_duplicate_twice (0.05 s apart): the label reads 'Duplicated deck: A copy' (got None)
FAIL  a5b_duplicate_twice (back to back): two Duplicate clicks = two copies, in order (names ['A'])
FAIL  a5b_duplicate_twice (back to back): the second copy is active (0 == 2)
FAIL  a5b_duplicate_twice (back to back): each copy has its own players (video_players 4 == 12)
FAIL  a5b_duplicate_twice (back to back): the label reads 'Duplicated deck: A copy' (got None)
FAIL  a5b_duplicate_twice (back to back): the second click QUEUED behind the staged first (load.queued None == 1 at the barrier)
FAIL  a6_append_deck: (b) 2 decks within 2.5 s (None), activeDeck 1 (0), label 'Loaded deck: deck16' (None)
FAIL  a6_append_deck: (b) the label reads 'Loading deck16...' while the deck is staged (got None)
FAIL  a6_append_deck: (c) the appended deck's 16 clips have clipWidth 3840 (0 clips)
FAIL  a6_append_deck: (g) CoreAudio overloads +None == 0
FAIL  a6_append_deck: (g) analysis ring overruns +None == 0
FAIL  a6_append_deck: (g) audio callback gap max None ms <= 2.5 x period (None)
FAIL  a7_failure_mid_batch: (c) one failed open counted (opens_failed +None == 1)
FAIL  end_quit_hung_open: the hung load answers 'superseded by a newer load' (got True None)
FAIL  end_quit_hung_open: the UI answers while an open hangs (label None == 'Loaded: end_quit_hung_open_W3')
FAIL  phase2_hung_open: the app is still running 31 s after the quit (killed)
```
### RED -- probe-async-load on the COMMIT-1 app (11:48:23, load 5.02): `PY 53 PASS / 27 FAIL`, phase 2 `PY 1 PASS / 2 FAIL`, `PROBE-ASYNC-LOAD RED`. FAIL lines verbatim:
```
FAIL  a2_load_16x4k: (a) longest message-thread stall 243.8 ms <= 50.0
FAIL  a2_load_16x4k: (b) ok:true within 2.5 s (0.002953052520751953) AND the composition read right after the answer is V16 (layer id 811 == 812, 1 clips)
FAIL  a2b_load_16x1080: (a) longest message-thread stall 74.1 ms <= 50.0
FAIL  a2b_load_16x1080: (b) ok:true within 2.5 s (0.0016090869903564453) AND the composition read right after the answer is V16 (layer id 821 == 822, 1 clips)
FAIL  a3_cancel_by_newer_load: (a) V32 answers 'superseded by a newer load' within 0.5 s of W3's post (got True None, -0.14603972434997559)
FAIL  a3_cancel_by_newer_load: (b) W3 ok:true within 2.5 s (0.0015721321105957031) and on screen when it answered (False)
FAIL  a3_cancel_by_newer_load: (d) two batches began (open_batches +0 == 2)
FAIL  a3_cancel_by_newer_load: (f) longest message-thread stall 495.1 ms <= 50.0
FAIL  a3b_explicit_cancel: (a) V32 answers 'superseded by a newer load' within 0.5 s of the cancel (got True None, -0.14915204048156738)
FAIL  a3b_explicit_cancel: (b) nothing swapped: W2 still on screen (layer id 842 == 841)
FAIL  a3b_explicit_cancel: (c) the label is restored to the text before staging 'base.png' (got 'Loaded: a3b_explicit_cancel_V32' / 200)
FAIL  a3b_explicit_cancel: (d) the cancelled batch's players are retired (video_players 32 == 0)
FAIL  a3c_cancel_with_sequences: (a) V32 answers 'superseded by a newer load' within 0.5 s of the cancel (got True None, -0.14045166969299316)
FAIL  a3c_cancel_with_sequences: (b) nothing swapped: W2 still on screen (layer id 852 == 851)
FAIL  a3c_cancel_with_sequences: (c) the label is restored to the text before staging 'base.png' (got 'Loaded: a3c_cancel_with_sequences_V32' / 200)
FAIL  a3c_cancel_with_sequences: (d) the cancelled batch's players are retired (video_players 32 == 0)
FAIL  a3c_cancel_with_sequences: (f) no sequence left open by the cancelled batch (seq_open 0 -> 2)
FAIL  a4_load_then_trigger: window too short to test (the load answered after 0.007875919342041016 s <= triggerDelayS 0.1)
FAIL  a4_load_then_trigger: (c) V32 on screen with no active clip (the trigger did not leak into it; activeClipColumn [1])
FAIL  a4_load_then_trigger: (f1) the label keeps 'Loading a4_load_then_trigger_V32...' through the trigger (got 'v4k_01.mp4')
FAIL  a4_load_then_trigger: (f2) after a trigger + cancel the label shows the trigger's text 'top.png' (mid 'v4k_01.mp4', end 'v4k_01.mp4')
FAIL  a5_duplicate_deck: (a) the label reads 'Loading A copy...' then 'Duplicated deck: A copy' (got 'Duplicated deck: A copy' -> 'Duplicated deck: A copy')
FAIL  a5_duplicate_deck: (e) longest message-thread stall 69.3 ms <= 50.0
FAIL  a5b_duplicate_twice (back to back): the second click QUEUED behind the staged first (load.queued 0 == 1 at the barrier)
FAIL  a6_append_deck: (a) longest message-thread stall 238.4 ms <= 50.0
FAIL  a6_append_deck: (b) the label reads 'Loading deck16...' while the deck is staged (got 'Loaded deck: deck16')
FAIL  a7_failure_mid_batch: (c) one failed open counted (opens_failed +0 == 1)
FAIL  end_quit_hung_open: the hung load answers 'superseded by a newer load' (got True None)
FAIL  end_quit_hung_open: the UI answers while an open hangs (label None == 'Loaded: end_quit_hung_open_W3')
FAIL  phase2_hung_open: the app is still running 31 s after the quit (killed)
```
(main's `phase2_hung_open` is the message thread blocked in open(2) of the FIFO: the Apple-event quit never lands; the .sh kills it at 30 s -- no .ips, 0 dialogs.)

### TEETH (FAIL lines verbatim; restore evidence)
Live, one TEMPORARY env-switched build (`AUDIODNA_ASYNCLOAD_T1/T3/T5/T7/POOL/T6`, applied on top of f8181be, reverted by blob hash: StagedLoad.h 647d9c34, ApiServer.cpp 763d83c0, MediaOpener.cpp 4f433923 == HEAD after; rebuilt; `strings Audio-DNA | grep -c AUDIODNA_ASYNCLOAD_` = 0 on build-lane and on the final app):
- T1 (Ledger::land ignores gen): `FAIL  a3_cancel_by_newer_load: (g) a video load superseding a staged one lands all 16 of its own clips (dims + thumbnail) and players (video_players 14 == 16)`
- T3 (REST answers before the wait): `FAIL  a2_load_16x4k: (b) ok:true within 2.5 s (0.002318143844604492) AND the composition read right after the answer is V16 (layer id 811 == 812, 1 clips)` and `FAIL  a4_load_then_trigger: window too short to test (the load answered after 0.0028111934661865234 s <= triggerDelayS 0.1)`
- T5 (labelAfterCancel returns current): `FAIL  a3b_explicit_cancel: (c) the label is restored to the text before staging 'base.png' (got 'Loading a3b_explicit_cancel_V32...' / 200)`
- T7 (AL7 off: newest wins): `FAIL  a5b_duplicate_twice (back to back): two Duplicate clicks = two copies, in order (names ['A', 'A copy'])` (+ activeDeck 1, video_players 8, queued 0)
ctest teeth on scratch copies (deliverable sha256 unchanged before/after: StagedLoad.h 0de9524b..., MediaOpener.cpp f069b541...):
- T1: `CHECK( l.land(g1) == Landing::Stale )` FAILED; T2 (takeAll drops the sequence id): `REQUIRE( ids.size() == 2 )` FAILED; T5: `CHECK( labelAfterCancel("Loading X...", "Loading X...", "Loaded: W") == "Loaded: W" )` FAILED; T5b (returns before): `CHECK( labelAfterCancel("clip.mp4", "Loading X...", "Loaded: W") == "clip.mp4" )` FAILED; T7 (admit newest-wins): `CHECK( admit(Kind::DeckDuplicate, true, 0) == Admit::Enqueue )` FAILED; T7b (unbounded): `CHECK_FALSE( q.push({ Kind::DeckAppend, "/x.json", 0, 0, "x" }) )` FAILED; T7c (no epoch): `CHECK( resolveDuplicateSource(2, 3, ids, 7) == -1 )` FAILED; opener (landed skips the Stale check): `CHECK( o.stale() == 2 )` FAILED.
- T6 (AL2: the pool's destructor path restored, no leak): the forked-child ctest still PASSES (`All tests passed (7 assertions in 1 test case)`, 5.81 s vs 5.27 s shipped): ~ThreadPool's stopThread(500) kill of a thread blocked in open(2) of a FIFO did not crash here -> the AL2 case and end_quit_hung_open are GUARDs. Not run live: a crash at quit would put a "quit unexpectedly" dialog on Boris's screen (screen-safety law outranks a teeth run; the CLI child shows the same kill path without UI).

### T4 DATA (not a gate) -- a2 x5, 16 x 4K
| pool | stall max ms | answer (cut) s | landings on the message thread: opens_ms / max one | msg_ms | overloads / gap max | late | hold |
|---|---|---|---|---|---|---|---|
| shipped 2 x low (final, 12:05) | 11.6 10.6 11.6 15.9 13.6 | 0.146-0.157 | 0.72-0.86 / 0.08-0.13 | 3.8-15.5 | +0 +2 +0 +0 +0 / 11.5, 160.9, 10.8, 10.8, 10.8 | +0 x5 | +0 x5 |
| shipped 2 x low (final, 12:18, 10 more interleaved with 10 no-load controls) | 8.3-15.1 | total_ms 141-210 | 0.73-1.51 / 0.07-0.71 | 4.4-16.2 | +0 x10 / <= 10.87 | +0 | +0 |
| no-load control (d2, 10 x) | -- | -- | -- | -- | +0 x10 / <= 10.92 | -- | -- |
| teeth 8 x highest (11:25) | 10.4 13.8 7.0 12.8 13.3 | 0.075-0.086 | 16.7-30.9 / 3.3-9.3 | 22.0-41.6 | +0 x5 / <= 10.81 | +0 x5 | +0 x5 |
Reading: 8 x highest cuts ~2x sooner but makes every LANDING 20-40x dearer on the message thread (the message thread competes with 8 highest threads); on this fixture neither pool touches the old clip's frames or the audio. AL1's condition (a2 (h) or hold failing in >= 1 of 5) did not occur -> the 1-thread arm was not run; R3 corrected: each open also starts FFmpeg's 2 frame threads (VideoPlayer.cpp:108), so the 2-thread pool puts up to ~6 runnable threads on the CPU during a batch -- the old show's rows, not the arithmetic, are the verdict (and they held).

### EXISTING PROBES on the final app (never re-thresholded)
probe-media-open `PY 38 PASS / 0 FAIL PROBE-MEDIA-OPEN GREEN`; probe-seq-vram `PY 66 PASS / 0 FAIL PROBE-SEQ-VRAM GREEN`; probe-image-load `PY 37 PASS / 0 FAIL PROBE-IMAGE-LOAD GREEN`; probe-crossfade `PY 35 PASS / 0 FAIL PROBE-CROSSFADE GREEN`; probe-deck-tabs `6 PASS / 0 FAIL`; probe-capture `PY 9 PASS / 0 FAIL PROBE-CAPTURE GREEN`; probe-routines (ROUTINES_BUILD_DIR=build-lane, pause 1.8) `109 PASS / 0 FAIL`; probe-video `PY 55 PASS / 1 FAIL PROBE-VIDEO RED` -- the one FAIL `w6b_retrigger_mid_fade: (a) every peak_callback_ms poll <= 16.7 (1 over)` (28.2 ms) is PRE-EXISTING / load-sensitive: 5 runs per arm, interleaved, same holds: main FAILs 4/5 (max callback 16.94, 19.22, 19.65, 15.68 pass, 20.25), final FAILs 3/5 (19.28, 14.75 pass, 23.75, 18.30, 15.49 pass) -- not this lane's (it loads nothing during the row). probe-idle-paint: first attempt 12:07 TAINTED by another lane's compiler (5 SKIP, not a verdict); re-run 13:36 (rows c0_preflight, i1_idle_card, i2_idle_many16, g2_strip_playhead, g4_routine, v2b_fallback_frames; 5 launches) `13 PASS / 0 FAIL / 0 SKIP (13:52:02, load 9.48 8.75 8.87) PROBE-IDLE-PAINT GREEN`. Tier-1 (test mode, 13:09, load 9.09): test_sources `4 passed`, test_effects `3 passed`, test_audio_reactivity `4 passed`, test_time_sweep `1 passed`, test_performance `2 passed`.

### CTEST
`ctest --test-dir build-lane -j1` (13:52, final tree): `100% tests passed, 0 tests failed out of 942` (main 920 + 22: test_load_ticket 3, test_staged_load 12, test_media_opener 7). Per commit: c1 920/920, c2 935/935, c3 941/941, c4 941/941, AL2 942/942.

### FINDINGS NOT FIXED (named, not improvised)
1. An intermittent ~110 ms message-thread stall ~0.12-0.14 s AFTER the cut of a 16-cell load (a2 117.3 / 115.6 / 113.7 / 108.7 / 114.0 / 117.6, a2b 120.2 / 118.9, a6 110.4 / 107.0 ms) in two batches at 10:33 and 10:40 (load 5.8-7.9) -- 10 of 10 16-cell loads then (the 1- and 4-cell loads of the same runs stayed <= 20 ms); in every later batch (c4a x4, A/B of both binaries, GREEN x2, a2 x5, a2 x10: 30+ loads) <= 16 ms. The load's own message-thread work in those runs was msg_ms 5-9 ms, so the stall sat outside the load code; the A/B at 11:20 showed the same binary at 10.6-15.2 ms. Cause not identified (no profiler is allowed; the window-pass counter was added after the episode). If it recurs, the d1_grid_16_images row + main_component_paints around the cut are the next data.
2. One audio outlier: in the 5-run a2 batch at 12:05 one load window showed CoreAudio overloads +2 and a 160.9 ms callback gap; 10 more a2 windows and 10 no-load control windows (12:18) were all clean. Not attributed (the load never runs on the RT callback); 1 of 37 a2 load windows on the async builds (every a2b / a6 window clean too).
3. The plan's RED magnitudes (1000-1200 ms for 16 x 4K) assumed ~68 ms per 4K open; the plan's own fixture recipe (flat-colour ultrafast H.264) opens in 15-17 ms, so RED is 244-257 ms and the staged windows are 0.15-0.3 s. Rows were made state-driven (label read at once after a post, a ui_text barrier) instead of fixed sleeps so they cannot miss a short window.
4. probe-video w6b (a) is load-sensitive on main (4/5 FAIL) -- pre-existing, filed for the video owner.
5. SLIM: `LoadTiming::active()` and `stagedload::LabelHold::loadingText()` are unused (EXCESS_DEAD, one inline accessor each) -- left in place so the GREEN evidence stays on binary sha256 2c877262c6a80efd; cut them in a follow-up.

### BORIS CHECKS (in the app)
- Load a composition with many videos (library row or File > Open): the window never freezes; the file label reads "Loading <name>..." and the output keeps playing the current show until the new one cuts in (no black flash); keys / pads during that second still act on the show you see.
- Right-click a deck tab > Duplicate twice quickly: two copies appear ("<name> copy" twice), the second one active.
- Load Deck... while a composition is still loading: the deck is added after the composition arrives.

### NOTEBOOK (for Harmony to append to .harmony/notebook.md)
- 2026-09-29 asyncload | every file-label write goes through MainComponent::setFileLabel (AL5: while a load is staged the text is HELD for a cancel); a new label writer that calls fileLabel_.setText directly breaks the "Loading..." hold | discovered: src/MainComponent.cpp setFileLabel
- 2026-09-29 asyncload | the async-load probe fixture (flat-colour ultrafast H.264) opens a 4K file in ~15 ms, so a staged window is 0.05-0.3 s: a row that must land INSIDE a window reads right after its POST (the ui_text message queues behind the load's), never after a fixed sleep | discovered: .harmony/probe-async-load.py a5 / a5b
- 2026-09-29 asyncload | `osascript 'tell application "Audio-DNA" to quit'` against a HUNG app blocks ~120 s (Apple-event timeout) before any 30 s kill clock starts: background it | discovered: .harmony/probe-async-load.sh quit_check
- 2026-09-29 asyncload | .harmony/probe-*.{sh,py,json} are gitignored: `git add -f` or the probe silently misses the commit | discovered: .gitignore
- 2026-09-29 asyncload | juce::ThreadPool::removeAllJobs(false, 0) deletes QUEUED jobs and never waits for a running one; count dropped jobs in the ThreadPoolJob destructor (ran_ false) -- getNumJobs() races a finishing job | discovered: src/core/MediaOpener.cpp OpenJob


### FILES CHANGED
- `src/core/LoadTiming.h` (new) -- the last load's cost split for /api/state load.timing.
- `src/core/LoadTicket.h` (new) -- waitable load outcome (first finish wins).
- `src/core/StagedLoad.h` (new, pure) -- Ledger, labels, LabelHold (AL5), Adopted, admit / LoadQueue / resolveDuplicateSource (AL7).
- `src/core/MediaOpener.h/.cpp` (new) -- the 2 x low pool, WeakReference landings, non-blocking cancel, dropped counting, AL2 leak.
- `src/MainComponent.h/.cpp` -- staged flow, setFileLabel (46 label writers routed), queue, quit order, /api/state load{}, TEST-ONLY route targets, xrun sampling; openMediaForDeck removed.
- `src/api/ApiServer.h/.cpp` -- the ticket wait + registry + stop() release; TEST-ONLY ui_text / load_deck / duplicate_deck / cancel_load; state "load".
- `src/test/TestServer.h/.cpp` -- state "load".
- `src/audio/CombinedCallback.h`, `AudioCallback.h/.cpp`, `AudioEngine.h` -- TEST-ONLY audio witnesses (compiled out otherwise; AL9 (a) verified).
- `src/media/VideoPlayer.h` -- the open() contract comment only (AL4).
- `CMakeLists.txt` (+MediaOpener.cpp, headers), `tests/CMakeLists.txt` (+3 targets), `tests/test_load_ticket.cpp`, `tests/test_staged_load.cpp`, `tests/test_media_opener.cpp`.
- `.harmony/probe-async-load.{sh,py,json}` (new, git add -f).
- `CLAUDE.md`, `docs/claude/{rendering,integration,pitfalls,recording}.md`.

### SLIM CHECK
EXCESS_DEAD: `LoadTiming::active()` and `LabelHold::loadingText()` (unused inline accessors; left so the evidence stays on the tested binary). Nothing else: every new counter is read by /api/state or a ctest; the diagnostic probe rows d1 / d2 are opt-in (not in the default order).

### RISKS
- The REST load now blocks one httplib worker per in-flight load (bounded 60 s; a newer load releases the older at once; stop() releases all first).
- A staged load holds its adopted (parked) players beside the live ones until the cut -- the same memory the synchronous load held.
- finding 1 (intermittent post-cut stall) would, if real and recurring, still breach the 50 ms bar at a cut even though the load code itself costs 5-9 ms.

### PACKET QUALITY
- Clarity: CLEAR (plan + adoption rulings precise; AL7 / AL5 needed design detail I filled within the rulings -- queue drop on cancel, epoch for queued Duplicates, held notes).
- Missing context: the probe fixture's real open cost (the plan's 68 ms/open model did not match its own recipe); that `.harmony/probe-*` files are gitignored; that the lock is shared with g4cpu too (long waits).
- Unused context: none.
- Self-brief files: plan-asyncload.md (read in full, adoption applied), lock.sh, probe-media-open.{sh,py,json} (copied helpers), s-rta-0928b final.sh (battery shape) -- all useful.
- Deviations: T6 run as a forked-child ctest, not live (screen-safety law); a3 (g) added so T1 can bite live; a3b / a3c (c) compare with the pre-staging label (the row triggers base.png first); a5b adds a back-to-back pair (the ruled 0.05 s pair lands after a ~35 ms window on this fixture); a2 (d) measures from 0.1 s windows (0.3 s was the c1-only guard); probe .sh backgrounds the quit Apple event.
