# Verify F26 - TSan race on std::cerr (VideoPlayer.cpp:257 vs :260), tsan-6 / tsan.50561
VERDICT: BENIGN (real unsynchronised race; libc++ stream-state class), severity LOW, owner app (fix) / libc++ (mechanism)

## What the report says (VERIFIED, raw tsan.50561 lines 52-130)
- Two MediaOpen ThreadPool workers (T21, T22; MediaOpener.cpp:33 -> VideoPlayer::open) run the one
  "[VideoPlayer] Opened: ..." std::cerr chain at the same moment.
- Racing 8-byte word 0x1ef720380 = cerr (0x1ef720360) + 0x20, inside libc++ std::cerr (ios_base state).
  Frame is libc++ __pad_and_output (pad_and_output.h:54): reads the stream width and resets it with width(0).
- VERIFIED src/media/VideoPlayer.cpp:257 = `<< ", codec=" << avcodec_get_name(...)`; :260 = `<< ", thumb=" ...
  << std::endl`; both are lines of the ONE statement opening at :253. Line numbers match HEAD 90b2ca9
  (git diff of the file is empty). Same object also races F1 (AnalysisThread.cpp:84) - Family A.

## Refutation attempts
- Synchronised by something TSan cannot see? NO (VERIFIED). No lock around the chain in VideoPlayer::open;
  MediaOpener runs several workers (MediaOpener.cpp:68). No sync_with_stdio / rdbuf redirect in src
  (grep empty), so cerr is the default stdio-synced, unit-buffered stream. Race is real by the letter (UB).
- Tool artefact? NO for the race; it only fires when two videos open concurrently (hit 1/6 launches).
- Benign pattern? YES. INFERRED (libc++ knowledge, libc++ source not re-read): the racing field is the format
  width, which stays 0 in this app (no setw/width on cerr in VideoPlayer.cpp), so lost/torn writes store the
  same value. Bytes go through the stdio-synced streambuf (fwrite, locked per call), so no buffer is corrupted.
- Not app data: TSan says "Location is global 'std::__1::cerr'" in libc++.dylib, not a heap block (VERIFIED).

## Live-show impact
- Crash: none expected (INFERRED). Torn value: none observable (width always 0). Wrong visual: no.
- Only visible effect: two "Opened:" log lines may interleave mid-line in stderr (log cosmetics).
- Likelihood: race needs 2+ concurrent video opens (a deck/composition load with several videos); harm ~nil.
- ASSUMED: nothing else changes cerr format state (precision/flags) globally; if something did, a number
  could print misformatted - still log-only.

## Smallest fix (proposed, not applied; read-only role)
Format the line into one juce::String/ostringstream and emit with a single fputs(..., stderr) (stdio locks per
call), or a shared logLine() helper with a small static mutex. Do it once for F1 (AnalysisThread.cpp:84),
ApiServer and VideoPlayer to silence all of Family A. Priority LOW: only to clear sanitizer noise before
real TSan triage.
