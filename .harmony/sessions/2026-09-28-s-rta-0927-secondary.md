# Session log — s-rta-0927 (2026-09-27 09:12 → 2026-09-28 ~00:50, SECONDARY, RealTimeAudio)

Profile: MINIMAL foreign-repo secondary. Harmony_Main SYSTEM files touched: NONE. Harmony_Main writes: NONE. Ultracode on
(workflows). Running log: .harmony/s-rta-0927-work.md. Plans/reports/evidence: .harmony/.reports/s-rta-0927/. Every merge:
Harmony ran the new probe RED on the pre-merge main app herself, merged, rebuilt build/, ran ctest serially and the full
live battery; the Output window was never opened by any gate (o_no_window_opened).

| type | ref | msg |
|---|---|---|
| shipped | e00b69a | plan5 C1: canvas -> IOSurface frame slots (kSlots 4) -> OutputPresenter; OutputWindow rewritten (normal level, never key, checked in Release); legacy OutputRenderer + loadImage fan-out retired; output_probe / set_output_tap / state.outputs |
| shipped | 20cede2 | routines display slice A: 8 pads above the column numbers, name bands on every layer, strip faders follow the model, routine cue in its own hue (chartreuse), pad menu incl. Start Ease/Jump, edits while a start/restart waits reach it, Record-tab pad row retired |
| shipped | f4507e8 a302dcb | probes use a fresh connection per request (cpp-httplib 5 s keep-alive drop); probe-canvas sees clang++; manual-bpm probe executable; stale OutputRenderer comments |
| shipped | 1636785 | probe-routines windows re-based on the routine's recorded moves (the canvas-merge flake was the probe) |
| shipped | dc7adf9 | plan5 C2: any number of displays (OutputManager), ticked Output menu + All Outputs Off, TopBar "Outputs: N", Cmd+Shift+Esc / Cmd+backtick / Cmd+F, plain Esc no longer closes outputs, deck files never open outputs, currentImageFile_ removed |
| shipped | 2ee1013 | beat clock: FeatureSnapshot::totalBeatCount; RecorderClock integrates count+phase (a stall no longer loses beats); a loop folds whole cycles with one restore; stall hook + probe-beatclock |
| shipped | 5267a0c | render perf: frame-ring cells lazy (first crossfade 33-72 -> 9-14 ms cold, 2-5 warm), PixelConvert rows, capture GL share 94-110 -> 2-5 ms (4K 372 -> 8), per-capture read handoff |
| shipped | 8c4c1a1 | plan5 C3: hot-plug reconcile (hook + 30 Hz poll), replug returns, AppSettings read-modify-write, Restore Last Outputs, Shift-up keys, test mode never touches the real settings.json |
| shipped | f630336 | source defects: julia/burning/newton/sierpinski/crystal_cavern/Dot Field draw at defaults; 78 dead controls removed, 3 implemented; param lint + GL defaults ctest |
| shipped | 233eae7 | follow-ups: Tier-1 harness 8/13 FAILED -> 12/13 PASSED; capture answered by a frame armed after the request; PNG writes replace; autopilot/playlist/slideshow/randomize on totalBeatCount |
| design | plans | plan-routine-display-A, plan-beatclock (draft + 3 seats + final), plan-renderperf, plan-source-defects, plan-followups (all Fable, all followed) |
| gate | ctest | 667 at boot -> 691 -> 720 -> 729 -> 742 -> 749 -> 764 -> 774 -> 784 |
| gate | live | full battery GREEN after every merge (outputs 17/0, routine-display 16/0, beatclock 6/0, routines 105/0, render-state 32/0, ...); final run with perf rows at close |
| finding | tempo0 | take start tempo 0 / unknown bar grid when set_bpm + Record arrive within ~10 ms (scripted clients; 4/34 and 9/34; pre-existing) — fix A proposed, not built |
| finding | tier1 | Tier-1 red was mostly harness (stale params between loads, canvas resize per capture, brightness metric, stale expectations); 30 named lines remain |
| finding | deckclock | d_return_hitch RemoteDisconnected = cpp-httplib keep-alive race (server [KAI] log), ~1 in 3 runs on any build |
| slip | wf-syntax | launched the wave-1 workflow before the syntax check (checked after: OK) |
| slip | sendmessage | SendMessage to running workflow agents forked concurrent executors; my TaskStop then killed the copy that had my rulings (routine cue recoloured against my ruling; kept on the merits) |
| slip | refutation | called the keep-alive theory refuted on 3 samples and C1 a regression on n=1 — both wrong; corrected by a proper A/B |
| slip | fixture | read fixture B (a 2x2 screenshot) as a tiling bug before checking the input |
| slip | merge | a follow-ups merge silently refused because my notebook notes were uncommitted (merge output filtered by grep); caught when the commit said "nothing to commit" |
| rig | lock | lock starvation (immediate re-acquire) and perf rows waiting for idle CPU while holding the lock — packet rule (45 s) + row filters |
| rig | settings | a C3 fix-round mutant briefly created and deleted the real ~/Library/Audio-DNA/settings.json (absent before and after) |
