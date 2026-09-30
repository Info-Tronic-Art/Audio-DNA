# Verify sweep F25 - std::cerr race in VideoPlayer::open, tsan-5 (tsan.50099)
STATUS: DONE
VERDICT: BENIGN (Family A, same class as F1/F8/F23; tool report accurate, consequence cosmetic)

## Evidence
- VERIFIED (raw tsan.50099 lines ~170-190): write size 8 at 0x1ef720380 = "global std::__1::cerr" (cerr base 0x...360,
  +0x20). T21 frame #0/#1 = libc++ __pad_and_output pad_and_output.h:80 via __put_character_sequence
  (const char* insertion); T22 same frames. Both reach us via VideoPlayer::open -> MediaOpener::OpenJob::runJob
  (src/core/MediaOpener.cpp:33) -> juce::ThreadPool worker. Both threads created by MediaOpener's pool ctor (MediaOpener.cpp:68).
- VERIFIED (src/media/VideoPlayer.cpp:253-260): one chained `std::cerr << ... << std::endl`; line 255 = the
  `", "` literal after width_, line 260 = the `" ms)"` literal. No lock/atomic/post between the two threads' chains.
- INFERRED (F8 read pad_and_output.h:54 vs :80 in tsan.49175): the size-8 word is ios_base::width_, which libc++
  resets with width(0) after every formatted string insertion. Both threads write 0 over 0.
- VERIFIED (grep src): no cerr.width/precision/fill/rdbuf/setf, no setw, no sync_with_stdio anywhere -> width is
  always 0, so a torn/interleaved write cannot change any output.

## Refutation attempts
- Hidden synchronisation? No. cerr is unit-buffered and libc++ backs it with a stdio streambuf (INFERRED; not
  re-read this pass), so the character bytes are serialised by stdio's FILE lock, but the stream's own state
  (width_) is not - TSan is technically right (C++ [iostream.objects]: concurrent use is race-free for chars only).
- Tool artefact? No: real unsynchronised same-address writes on two real threads. Just a same-value write.
- Touches app data / heap? No. Other operands are per-VideoPlayer members or locals (thumbMs local).

## Live-show impact
- Worst case: two "[VideoPlayer] Opened:" lines interleave on stderr (cosmetic). No crash, no torn app value, no
  visual or audio effect; runs on MediaOpen pool threads, never audio/GL/message thread. Requires two concurrent
  video opens (a deck load with 2+ videos; seen in 1/6 launches). Interleave verified plausible (app-err.log has
  19 "Opened:" lines) but not inspected for garbling.

## Smallest fix (optional, not required for a show)
- Build the line into a std::ostringstream / juce::String and emit once (`std::cerr << line << '\n'`), or a
  shared logLine() helper. One helper covers F8, F23-F26. Or a TSan suppression race:std::__1::cerr for sweeps.
