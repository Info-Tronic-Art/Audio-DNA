# Reviewer Verdict — bf2 merge-in (M0) r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
PINNED: lane/bf2 head c45b579, merge 8a5831d (parents 4a1f240 lane + 34179a2 main), base af96de2
FILES: every file changed on both sides (21 non-report files) + probe scripts + docs; read through git objects only.
ISSUES: 0 blocking / 1 SHOULD / 3 NIT

## Verified (read / grepped by me)
- Both sides kept in all 21 shared files: for each, the lane's +/- lines vs the merge-base equal the lane's lines in (34179a2..8a5831d), and main's lines equal (4a1f240..8a5831d). Only probe-tsan-unit.sh, CLAUDE.md, pitfalls.md differ, as designed (renumber, [tsan] count, pitfall order).
- No src file conflicted. Lane header changes are additive only (no signature the main side calls changed). RecorderHost.cpp (lane) uses FeatureSnapshot only; main's Program/PerfState/RoutineEngine changes do not touch it. Lane Renderer hunk sits after main's rewritten frame-snap read; `now` (Renderer.cpp:359) is in scope. MainComponent.cpp: syncOffset_ built at :1831 before analysisThread_.startThread :1842; #if/#endif balanced (depth 0). Lane test_recorder_host additions use no Deck/Layer API.
- ONE definition of each route: ApiServer.cpp has no duplicate registration; /api/sync x6, /api/debug/sync_witness, /api/debug/sync_persist once each; /api/debug/undo once (:345), onDebugUndo once. No /api/sync* outside ApiServer.cpp.
- Pitfall NN: Harmony's exact command (ruling H-1) prints nothing at c45b579 (rc 1). Order 63..67 then 68 in CLAUDE.md index and pitfalls.md. CLAUDE.md = 24,370 bytes (<= 25,000). No main CLAUDE.md line lost except main's Analysis-Thread paragraph, which carries the lane's own earlier edit (delay line sentence; the trimmed O(1) clause still lives in docs/claude/analysis.md:82). Key-capabilities line == main's.
- [tsan] count: test_layer_runtime_race 4 + test_manual_scalar_race 1 + test_analysis_sync_thread 1 + test_sync_witness 4 + test_sync_offset_controller 1 = 11 = EXPECTED_TSAN_CASES; TARGETS lists all 5. ADNA_TSAN_TEST_PROPERTIES defined (tests/CMakeLists.txt:3282) before the lane block (:3931+). No duplicate add_executable. tests/CMakeLists.txt and CMakeLists.txt: head vs main = additions only.
- 1321 = main 1252 + lane 1183 - base 1114: arithmetic shown; main 1252 independently in bf9b-fix.md:1196. (ctest count itself INFERRED; not run.)
- BY-NAME-QUIT SWEEP: lane scripts probe-sync.sh / probe-sync-venues.sh / .py quit through quit_ours only (probe-sync.sh:115, venues.py App.quit). The four edited main probes (beatclock, downbeat-level, onset-render, tempo-start): 0 osascript, refuse_foreign_start + record_ourpid present. Whole head: every code-line osascript/pkill/kill hit is by pid (gate-s165, probe-deck-path, probe-tempo-silence use "first process whose unix id is $PID"), a self-forked watcher pid, or inside probe-quit-ours.sh; 16 archived .harmony/.reports scripts that quit by name each carry the "ARCHIVED RECORD (R-N1" refuse line or the REFUSE-quit-unless-ours guard.
- Nothing stray: no .venv, no symlink, no .orig/.rej, no mutant/instrumentation in src/tests/CMake. (Mutant/TEETH files under .harmony/.reports are dated evidence from main.)

## Findings
SHOULD docs/claude/testing-eyes.md:72 (identical in both parents) tells the reader `pkill -f "Audio-DNA.*--test-mode"`: a by-name kill recipe in a doc agents copy. Not a script and not a merge defect; also .harmony/gotchas.md:122,136 advise pkill. Fix: reword to "kill the pid you launched". Escalates to MUST only if Harmony counts doc recipes in the sweep.
NIT .harmony/APP-INVENTORY.md: top line says 16 OSC patterns / 16/16 wired, section heading says 17 patterns (pre-existing off-by-one, kept in step).
NIT .harmony/probe-quit-ours-selftest.sh part 3 static sweep matches only *.sh launch lines; probe-sync-venues.py launches the app from Python and is covered only by the quit-pattern grep, not by a refuse/record check (its own adna_pids() guard does refuse).
NIT G1 is not closed: 1320/1321, the [timing] case failed under a recorded >20 % process (ruling H-8 covers it; Harmony's quiet x3 run decides). Also the builder ran a live batch (bf2-delta.md live lock 20:46-20:48) although plan section 4 says Harmony runs live rows; the packet is not visible to me, so not judged.

METADATA: reviewer=claude-sonnet-5-5, builder_packet=bf2-M0, date=2026-10-03, confidence: VERIFIED for merge resolution, routes, counts, sweep; INFERRED for ctest totals and the [timing] cause.
