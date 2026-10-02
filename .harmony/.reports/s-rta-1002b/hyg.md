# Lane hyg report (s-rta-1002b)

STATUS: DONE (H1-H4 committed, one commit each (H2 report-only); gates below)
Base: 5b507b1 (lane/hyg at main head). Commits: H1 27c3848, H3 ded10a2, H4 95f1a46 (H2 = report-only, no commit: the rule did not fire).

## H1 -- R8 sweep (commit 27c3848)
13 std::cerr -> logLine in src/audio/AudioEngine.cpp (6) and src/audio/DeviceGuard.cpp (7); both files added to the
workerFiles list in tests/test_log_line_lint.cpp; header comment updated (R8 closed). `#include "core/LogLine.h"` added to both;
the now-unused `#include <iostream>` of DeviceGuard.cpp replaced by it.

Audio-callback reachability (NONE of the 13 sites is reachable from the device IO callback, so none was held back):
- AudioEngine ctor/`onReapplied` lambda (AudioEngine.cpp:15, :31, :36): the reconciler's re-apply (message thread / its timer) and the ctor.
- setSourceMode (AudioEngine.cpp:165/172): callers are MainComponent.cpp:303/308/313/546/551/3986/5688 (UI + ctor), not the callback;
  the IO callback class (CombinedCallback.h / AudioCallback.cpp) holds no cerr and calls none of these.
- debugStopDevice (:318) / GuardedDeviceType::setTestDeniedNames (DeviceGuard.cpp:116): reached from ApiServer.cpp:2124 / :2137, both
  `juce::MessageManager::callAsync` -> message thread.
- DeviceGuard.cpp:55 createDevice, :106/:109 rebuild, :217/:231 re-apply, :257 productionConfig: device open / device-list change /
  reconciler paths (JUCE AudioDeviceManager on the message thread), never inside audioDeviceIOCallbackWithContext.
Edge case kept identical: `std::cerr << hidden` printed the skipped-device block (each line already ends in '\n') and nothing when empty;
the conversion is `if (hidden.isNotEmpty()) logLine(hidden.dropLastCharacters(1));` (logLine appends its own '\n'; no extra blank line).
Everything else is a straight `<<` -> `,` swap of identical operands; `std::endl` -> logLine's '\n' (stderr is unbuffered, fwrite to it is immediate).

RED (before the conversion, the two files added to the list; verbatim from build-lane/tests/test_log_line_lint):
```
/.../tests/test_log_line_lint.cpp:102: FAILED:
  CHECK( h.empty() )
with expansion:
  false
with message:
  audio/AudioEngine.cpp: 6 std::cerr: audio/AudioEngine.cpp:15, audio/
  AudioEngine.cpp:31, audio/AudioEngine.cpp:36, audio/AudioEngine.cpp:165,
  audio/AudioEngine.cpp:172, audio/AudioEngine.cpp:318

/.../tests/test_log_line_lint.cpp:102: FAILED:
  CHECK( h.empty() )
with expansion:
  false
with message:
  audio/DeviceGuard.cpp: 7 std::cerr: audio/DeviceGuard.cpp:55, audio/
  DeviceGuard.cpp:106, audio/DeviceGuard.cpp:109, audio/DeviceGuard.cpp:116,
  audio/DeviceGuard.cpp:217, audio/DeviceGuard.cpp:231, audio/DeviceGuard.cpp:
  257

/.../tests/test_log_line_lint.cpp:105: FAILED:
  CHECK( total == 0 )
with expansion:
  13 == 0
with message:
  total std::cerr in the worker-thread list: 13

test cases:  3 |  2 passed | 1 failed
assertions: 58 | 55 passed | 3 failed
```
GREEN (after): `All tests passed (58 assertions in 3 test cases)`. Test count unchanged (the lint is one existing TEST_CASE; no case added).

grep -c 'std::cerr' per file, before -> after: src/audio/AudioEngine.cpp 6 -> 0; src/audio/DeviceGuard.cpp 7 -> 0.

## H2 -- audit of every other std::cerr in src/ (report only; the RULE DID NOT FIRE: no site converted, lint list unchanged)
Every site below runs on the JUCE message thread only. Why that is race-free: the lint's hazard is two threads streaming into std::cerr's
shared ios_base state. After H1 every file that can run off the message thread writes through logLine (an ostringstream local to the
caller + one fwrite of the whole line on the stdio-locked FILE), never std::cerr; so std::cerr has ONE user thread (the message
thread) and cannot race; an interleaved line from the message thread's cerr and another thread's logLine is whole-line at the stdio level
(std::cerr is synced with stdio by default), never a data race.

| site | what | thread evidence |
|---|---|---|
| MainComponent.cpp:82,84 | appSettingsFile (test mode) | callers MainComponent.cpp:1808 (ctor), 2348 / 2354 (load/save MilkDrop dir setting, called from the ctor :1688 and from setMilkDropPresetDir :2357 <- UI lambdas :6449/:6457) |
| MainComponent.cpp:507 | "[Audio] device N Hz" | ctor body |
| MainComponent.cpp:702, 710 | genre / structural callbacks | Renderer.cpp:569-585 marshals both via `juce::MessageManager::callAsync` before calling the callback |
| MainComponent.cpp:1792 | "[MilkDrop] Loaded N presets" | ctor body |
| MainComponent.cpp:1893 | "[Eyes] Test server started" | ctor body, after testServer_->start() (the caller thread, not the server thread) |
| MainComponent.cpp:2034 | recorderHost_.dispatch.notify | RecorderHost methods that notify (RecorderHost.cpp:293/369/387/446/488/540) all sit under RECORDER_HOST_ASSERT_MESSAGE_THREAD (:46, asserted at :119/:125/:167/:301/:424/...) |
| MainComponent.cpp:2066 | routineEngine_.dispatch.notify | RoutineEngine.cpp:214 notify() <- startNow :468 / :687 under ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD (:21, asserted :526 / :681) |
| MainComponent.cpp:2293, 2295 | OSC startListening result | ctor body |
| MainComponent.cpp:2301, 2303 | VideoRecorder onRecordingFinished | VideoRecorder.cpp:88-92 invokes it through `juce::MessageManager::callAsync` |
| MainComponent.cpp:4376 | openCamera | UI path (camera selector), message thread |
| MainComponent.cpp:4574 | "[ISF] Imported" | handleImportISF (:4515) FileChooser async callback, message thread |
| osc/OscHandler.cpp:24,28,40 | startListening / stopListening only (the receiver-thread path oscMessageReceived has no cerr) | callers: MainComponent.cpp:2292 (ctor), :2440 (dtor), OscHandler dtor :11 |
| midi/MidiOutputHandler.cpp:31,37,47 | openDevice / closeDevice (MIDI OUT; no input callback here) | callers MainComponent.cpp:6451/:6459 (UI lambdas), :2441 (dtor), handler dtor :16/:21 |
| recording/RecorderHost.cpp:313, 605 | take start / startDue | stop() (:301 asserts message thread) and startDue <- tick() (:470; message-thread-asserted) |
| recording/RoutineSlice.cpp:453 | takeBeatOfBar | only caller MainComponent.cpp:6037/6038 inside perfRoutineSave, reached from UI (:1589) or ApiServer.cpp:2228 inside `callAsync` (:2227) |
| ui/PresetManager.cpp:190 | loadPreset version note | callers MainComponent.cpp:397 / 2882 / 4235 (UI), PresetManager.cpp:582 (loadDeck <- MainComponent.cpp:3580 UI); no REST / worker caller (TestServer.cpp:1746 is ProjectM's loadPreset, a different function) |
| ui/OutputWindow.cpp:55 | OutputWindow ctor | constructed by OutputManager, message thread |
| output/OutputManager.cpp:304 | persistWanted | every entry point carries JUCE_ASSERT_MESSAGE_THREAD (:93/:107/:127/:218/:227/:281/:330); header line 5 "Message thread only" |
| effects/ISFShaderLoader.cpp:36 | parseISFSource | only caller MainComponent.cpp:4529 (the import chooser, message thread) |
(LogLine.h's own `std::cerr` mentions are comments.) Residual: the message-thread assertions above are jassert/JUCE_ASSERT (Debug); in Release the evidence is the call-chain read, not a runtime check. Flag for Harmony: a future off-thread caller of any of these would need the same conversion; the lint list is the guard only for the listed files.

## H3 -- "Pitfall NN" numbering (commit ded10a2; comment-only; 14 sites; `grep -rn 'Pitfall NN' src tests docs .harmony/*.sh .harmony/*.py CLAUDE.md` -> 0 now)
Subjects matched against docs/claude/pitfalls.md headings (57 = "JUCE 8's macOS peer repaints the UNION of every dirty rect ... a timer-driven repaint() anywhere is paid by the whole window", whose text lists NativeLayerHost / OverlayWatch / LayerStrip::transportViewOf / ClipInspector::paintKeyNow and the guards test_native_layer_cache, test_overlay_watch, test_layer_strip_transport_view, test_clip_inspector_paint_key, probe-idle-paint.sh; 58 = "A load is staged, never opened on the message thread ..."):
| site | quoted comment | pitfall |
|---|---|---|
| src/MainComponent.h:162 | s-rta-0929 asyncload: a composition / deck load is STAGED ... | 58 |
| src/MainComponent.h:471 | s-rta-0928b idlepaint: two always-animating panels draw in their own CoreGraphics layers | 57 |
| src/MainComponent.cpp:2310 | idlepaint: the two always-animating panels draw in their own CoreGraphics layers | 57 |
| src/MainComponent.cpp:3067 | asyncload: the three loads STAGE a private model ... | 58 |
| src/ui/UiPaintCounters.h:7 | idlepaint: witnesses of WHO repaints at idle | 57 (see note) |
| src/ui/ClipInspector.h:60 | idlepaint: EVERY input paint() read; refresh() repaints only when it changes | 57 (names ClipInspector::paintKeyNow, guard test_clip_inspector_paint_key) |
| src/ui/ClipInspector.cpp:981 | idlepaint: a 10 Hz repaint joined the peer's union | 57 |
| src/ui/NativeLayerCache.h:4 | idlepaint: the CachedComponentImage that hands repaints to a CALayer | 57 (mechanism (1)) |
| src/ui/NativeLayerHost.h:9 / NativeLayerHost.mm:1 | idlepaint: an always-animating panel draws in its OWN layer-backed view | 57 |
| src/ui/OverlayWatch.h:5 | idlepaint: screen rects of everything that can sit ABOVE a native-layer widget | 57 (the OverlayWatch paragraph) |
| src/ui/LayerStrip.h:53 | idlepaint: what the transport rect paints | 57 (LayerStrip::transportViewOf) |
| src/ui/LayerStrip.cpp:779 | idlepaint: the peer repaints the UNION of every rect repainted since the last vblank | 57 |
| tests/test_clip_inspector_paint_key.cpp:3 | repaints only when something paint() shows changed | 57 |
| tests/test_layer_strip_transport_view.cpp:3 | made the mac peer repaint the whole window 30 times a second | 57 |
| .harmony/probe-idle-paint.sh:5 | whole window 30 times a second | 57 |
Note (Harmony may re-rule): UiPaintCounters.h matches 57 by lane + subject ("who repaints at idle" = the union attribution); pitfall 59's "Witnesses" line also names the `GET /api/debug/ui_paint` endpoint these counters serve (the routine pad ones). No site was left unmapped.

## H4 -- pitfalls.md order + CLAUDE.md index (commit 95f1a46)
- docs/claude/pitfalls.md physical order was 55, 57, 59, `---`, 56, 58, 60-63; now 55, 56, 57, 58, 59, `---`, 60-63 (numbers ascend 1..63). Whole entries moved, no text change.
  Proof: `diff <(git show 5b507b1:docs/claude/pitfalls.md | grep -v '^$' | sort) <(grep -v '^$' docs/claude/pitfalls.md | sort)` -> empty (NONBLANK_MULTISET_EQUAL).
  DEVIATION from the packet's strict `diff <(sort)` check: the plain sorted multiset differs by +2 BLANK lines (68 vs 66): entries 56 and 58 were glued to their neighbours with no blank line (a markdown paragraph continuation); separating them adds two blank separators. File 79,173 -> 79,175 B. Strict equality is not reachable with a correct blank line between every entry (checked the slot arithmetic); say so if you want the glued form back.
- CLAUDE.md "Common Pitfalls Index": 59 WAS missing (index had 1-58, 60-63; pitfalls.md defines 1-63). Added, after 58:
  `59. A model-driven widget's change test compares what it PAINTS, never what it reads -- before adding a timer-driven `repaint()` to a widget that shows a model value.`
  Paid for by shortening the "Outputs" UI-pattern paragraph, whose dropped clauses already live in docs/claude/integration.md "Output windows" (`OutputManager::populateMenu` :22, Cmd+` / Cmd+F keys :23, saved set + Restore Last Outputs + `outputs` beside `milkDropPresetDir` via AppSettings :26-27, hot-plug return :25); the paragraph now points there. No meaning deleted, nothing added to integration.md (it already held every dropped clause).
  CLAUDE.md bytes: 24,006 -> 24,002 (cap 25,000). Every number in pitfalls.md (1-63) now has an index line (checked by script).

## Gates
- Release build of the worktree (build-lane, -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON, FETCHCONTENT sources from main's build/_deps read-only): `cmake --build` EXIT 0 twice (the second after the H3/H4 edits, so the final tree is built). Warnings in the touched files: 0 (grep of both build logs for `warning:` in src/audio/AudioEngine.cpp, src/audio/DeviceGuard.cpp, tests/test_log_line_lint.cpp -> empty; the warnings that exist are JUCE / system-header ones, unchanged by comment-only / cerr->logLine edits).
- Full ctest, serial (-j1, under mkdir /tmp/audiodna-ctest.lock, released): `100% tests passed, 0 tests failed out of 1114` (Total Test time 100.67 s). Count == main's 1114: H1-H4 add no test case.
- test_log_line_lint RED then GREEN: both verbatim in H1 above (RED 3 failed assertions naming the 13 lines; GREEN `All tests passed (58 assertions in 3 test cases)`).
- grep -c 'std::cerr' (code + comments, per file) before -> after: src/audio/AudioEngine.cpp 6 -> 0; src/audio/DeviceGuard.cpp 7 -> 0; all other files unchanged (MainComponent.cpp 15, OscHandler 3, MidiOutputHandler 3, RecorderHost 2, PresetManager/OutputWindow/RoutineSlice/OutputManager/ISFShaderLoader 1 each, LogLine.h 3 comment mentions); whole src/ 44 -> 31.
- Live stderr, BEFORE (main's app, build/AudioDNA_artefacts/Release) vs AFTER (this worktree's build), each one `open -g --stdout/--stderr <app> --args --test-mode` under the live lock (acquire_lock ... quit_app ... release_lock), one at a time, sequence: launch, POST /api/debug/audio_deny {"names":["hyg-nonexistent-device"]}, POST audio_deny {"names":[]}, POST audio_stop, quit. `diff` of the `[AudioEngine]` lines = IDENTICAL (7 lines, both runs):
```
[AudioEngine] audio devices: input "MacBook Pro Microphone" (bltn), output "MacBook Pro Speakers" (bltn); skipped: none
[AudioEngine] Switched to mic input mode
[AudioEngine] TEST-ONLY audio_deny: hyg-nonexistent-device
[AudioEngine] TEST-ONLY audio_deny: none
[AudioEngine] TEST-ONLY audio_stop: stopped "MacBook Pro Speakers" (the manager keeps it)
[AudioEngine] re-applying the device policy (device-stopped)
[AudioEngine] audio devices: input "MacBook Pro Microphone" (bltn), output "MacBook Pro Speakers" (bltn); skipped: none
```
  Covers AudioEngine.cpp:15/:36 (describeDevices), :165 (setSourceMode), :318 (audio_stop) and DeviceGuard.cpp:116 (audio_deny), :217 (re-apply). NOT exercised live (no trigger without a real device change / a refused open): AudioEngine.cpp:31, :172, DeviceGuard.cpp:55, :106, :109, :231, :257 -- covered by the lint + the text-identical operands only (inference, not a live check). Logs: scratchpad hyg-all/run/{main,after}/app-err.log.
  After each run: app quit cleanly (`app running after quit: no`), `audio-dna windows 0, Output-named 0`, UserNotificationCenter windows 0 (>= 15 s after the quit). Lock released (helper). No Output window opened, no TCC prompt, no re-signed bundle, no input synthesized. CPU context: both compilers were running during the BEFORE run (no perf number is claimed).
- Final state: `git status` clean (build-lane ignored), .venv symlink never created (probes not needed), no app running, no TEMPORARY env-var hook was added (strings check n/a).

## Notes for .harmony/notebook.md (Harmony appends)
- 2026-10-02 s-rta-1002b hyg: R8 closed -- every file that can run off the message thread (incl. audio/AudioEngine.cpp, audio/DeviceGuard.cpp) is in test_log_line_lint's list; the 31 std::cerr left in src/ are message-thread-only (call-chain table in the hyg report H2). A new off-thread caller of any of those needs the same logLine conversion + a lint-list entry.
- logLine appends its '\n': a multi-line string that already ends in '\n' (DeviceGuard::rebuild's `hidden`) must drop its last char first, and an EMPTY one must not be logged at all (std::cerr << "" printed nothing).
- docs/claude/pitfalls.md entries 56 and 58 were glued to their neighbours (no blank line, a markdown paragraph continuation); fixed with the reorder.
- Pitfall numbers: idlepaint lane = 57 (+59 for the routine-pad paint key), asyncload lane = 58; "next free Pitfall number" is still 64.
