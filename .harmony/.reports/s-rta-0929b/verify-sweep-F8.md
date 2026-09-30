# Verify sweep F8 — std::cerr race in VideoPlayer::open ("Opened:" line)
STATUS: DONE
VERDICT: BENIGN (Family A, same class as F1; tool report is accurate but harmless)

## What the race is
- VERIFIED (tsan.49175 lines ~46-58): racing address 0x1ef720380 = cerr+0x20; "Location is global 'std::__1::cerr'".
  Frames are libc++ pad_and_output.h:54 (read) vs :80 (write) = the ios_base::width field
  (operator<< of a const char* runs os.width(0) after every insertion). Not app data.
- VERIFIED (src/media/VideoPlayer.cpp:252-260): one chained `std::cerr << ... << std::endl`, no lock around it.
- VERIFIED (src/core/MediaOpener.cpp:33 + :68-71): runJob() calls player->open() on a juce::ThreadPool
  with kThreads workers, so two video opens run open() concurrently on T21/T22.
- VERIFIED (raw log): the report is 1 of 6 launches (tsan-3); app-err.log has 5 "Opened:" lines, so
  overlapping opens is a real (if uncommon) pattern.

## Refutation attempts
- Synchronised by a lock/atomic the tool can't see? No: nothing serialises the cerr chain (VERIFIED read).
  The C++ standard only guarantees no data race on the stdio-synced *characters*, not on the stream's
  formatting state, so TSan is technically right; it is not a tool artefact.
- Does anything in the app set cerr width/fill/flags? INFERRED benign: grep of src for sync_with_stdio/rdbuf
  found nothing; a width set by nobody means both threads write the value 0 over 0 (INFERRED from
  reading the frames; I did not grep every `setw` on cerr).
- Other operands (width_, frameRate_, duration_, path_, thumbMs) are per-VideoPlayer instances or locals,
  one per thread: no sharing (VERIFIED, open() members + locals at :240-260).

## Live-show impact
- Worst case: two "[VideoPlayer] Opened:" log lines interleave character-wise on stderr (cosmetic).
  A torn width would only affect padding, and nothing requests padding.
- No crash, no heap write, no visual effect. Stderr is not on the audio/render path; the cost is a blocking
  write, and the cerr write happens in the MediaOpen thread, not audio/GL.
- Likelihood of the race itself: seen 1/6 launches; likelihood of any visible symptom: ~nil.

## Smallest fix (optional)
- Build the line into a std::ostringstream / juce::String and emit with one `std::cerr << line` (or a shared
  `logLine()` helper that takes a static std::mutex; the mutex is off the audio thread, so Sacred Rule 2 is
  not touched). One helper would also silence F1 and F23-F26 (the whole of Family A) if used by
  AnalysisThread.cpp:84 -- BUT the analysis thread is near real-time, so use it there only outside the hop loop
  (F1 is a startup line, INFERRED).
- Or suppress: add a TSan suppression `race:std::__1::cerr` for sweeps.
