# Lane hyg — packet (s-rta-1002b, Harmony-authored; mechanical execution of residuals already ruled — no design content)

GOAL: close three filed residues with zero behaviour change: (1) R8 — the remaining std::cerr in the bt2 audio files become
logLine / logLinef and those files join the log-line lint; (2) every "Pitfall NN" placeholder left in code / tests / probes
gets its real number; (3) docs/claude/pitfalls.md is in numeric physical order and the CLAUDE.md pitfall index lists every
number pitfalls.md defines.

SOURCES THAT RULED THIS (read the named parts):
- .harmony/.reports/s-rta-0930/ruling-tsan.md :422 (R8 residual files = VideoPlayer.cpp outside open() [since converted
  whole, H3], src/audio/AudioEngine.cpp, src/audio/DeviceGuard.cpp) and plan-tsan.md R8 (~:552: "a post-merge mechanical
  sweep adds them to the lint list").
- tests/test_log_line_lint.cpp header comment (:7-8 names the R8 residual) and its file list (~:87-91).
- src/core/LogLine.h (what logLine / logLinef do, which thread may call which).
- HANDOFF ledger s-rta-0929b item 7 / s-rta-1002 item 9: "Pitfall NN" comments from the idlepaint / asyncload lanes
  (MainComponent.h:162/471, UiPaintCounters.h, NativeLayer*, LayerStrip, ClipInspector, OverlayWatch, tests,
  .harmony/probe-idle-paint.sh) never got numbers; pitfalls.md physical order 57, 59, 56, 58, 60, 61, 62.

ITEMS (one commit each, in this order):
H1 R8 sweep. Convert EVERY std::cerr in src/audio/AudioEngine.cpp and src/audio/DeviceGuard.cpp to logLine / logLinef
   (identical message text; the same newline / flush behaviour as the other converted files). Add both files to the lint
   list in tests/test_log_line_lint.cpp and update its header comment (R8 closed). RED FIRST: add the two files to the
   list BEFORE converting, run the lint test, paste the failure naming the cerr lines; then convert; GREEN.
   If ANY of these cerr sites is reachable from the audio device callback (Sacred Rule 1: no syscalls there), do NOT convert
   it (logLine is a syscall too): STOP that site and report it with the call chain (file:line) — Harmony rules it.
H2 Audit (report only, unless the rule below fires) of every OTHER std::cerr left in src/ (today: MainComponent.cpp 15,
   OscHandler.cpp 3, MidiOutputHandler.cpp 3, RecorderHost.cpp 2, PresetManager.cpp, OutputWindow.cpp, RoutineSlice.cpp,
   OutputManager.cpp, ISFShaderLoader.cpp 1 each; LogLine.h's own are the logger): for each site name the thread(s) that
   can execute it, with the call-chain evidence (file:line). RULE: a site that can run on ANY thread other than the JUCE
   message thread (OSC receiver, MIDI input callback, httplib, decode / worker threads, render / GL, analysis) is converted
   like H1 and its file joins the lint list (RED first the same way). Message-thread-only sites stay unchanged (state in
   the report why that is race-free given what logLine writes to).
H3 "Pitfall NN" numbering. grep -rn 'Pitfall NN' over src/ tests/ .harmony/*.sh .harmony/*.py docs/ (EXCLUDE
   .harmony/HANDOFF.md, .harmony/notebook.md, .harmony/sessions/, .harmony/.reports/, BORIS_DECISIONS.md, any *-work.md:
   those are history and quote the convention). Map each to the docs/claude/pitfalls.md entry it refers to BY SUBJECT
   (table in the report: site | quoted comment | pitfall number | the pitfalls.md heading that matches). If a site matches
   no entry, leave it and list it (Harmony decides). Comment-only edits.
H4 docs/claude/pitfalls.md physical order: move whole entries so the numbers ascend; NO text change (prove it: the sorted
   multiset of lines before == after, e.g. diff <(git show HEAD~:docs/claude/pitfalls.md | sort) <(sort the new file) is
   empty). Then check the CLAUDE.md "Common Pitfalls Index": every number defined in pitfalls.md must have an index line
   (59 looks missing — verify). Add any missing line in the index's style (one line, "-- before <trigger>."). CLAUDE.md is
   capped at 25,000 bytes (24,006 B now): pay for every added byte by MOVING text (never deleting meaning) from CLAUDE.md
   into the docs/claude/*.md file the trigger table already points to; report bytes before / after.

GATES (yours, in the lane; Harmony re-runs them after):
- Release build of the worktree (0 new warnings in touched files), full ctest serial under the ctest mutex: total count
  must equal main's (1114) — H1/H2 change no test count unless you add a lint case; say which.
- test_log_line_lint RED (H1, and H2 if it fires) then GREEN, both outputs verbatim.
- grep -c 'std::cerr' per file before / after (table).
- One open -g test-mode launch of YOUR build with --stderr captured (lock helper, acquire_lock), quit cleanly: show that an
  AudioEngine / DeviceGuard log line still appears in stderr with the same text as on main's app (run main's app the same
  way for the BEFORE line, one at a time). No Output window; 0 UNC windows after.
- Harmony constraint: zero behaviour change. No source edits beyond the cerr -> logLine lines, the lint list / header, the
  comment numbers, the pitfalls.md move and the CLAUDE.md index line(s) with their paid-for move.
