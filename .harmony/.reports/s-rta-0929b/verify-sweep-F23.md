# Verify sweep F23 - std::cerr race in VideoPlayer::open ("Opened:" line), tsan-5
STATUS: DONE
VERDICT: BENIGN (Family A, same class as F1/F8; the tool report is accurate but harmless)

## Evidence
- VERIFIED (src/media/VideoPlayer.cpp:253-260): one chained `std::cerr << ... << std::endl`, no lock around it.
  Line 253 = first insertion (read of cerr state), line 254 = next insertion (write of size 8) -> matches F23's
  read@253 (T21) vs write@254 (T22).
- VERIFIED (src/core/MediaOpener.cpp:26-33): runJob() calls player->open() on a juce::ThreadPool worker; two
  concurrent video opens run open() on T21/T22 at once. Nothing serialises the cerr chain.
- VERIFIED (sweep.md:66): "Location is global 'std::__1::cerr'", size 8 -> a libc++ ios_base field (width),
  not app data. Same signature as F8 (verify-sweep-F8.md, which read the tsan.49175 frames: pad_and_output.h
  width read/write). INFERRED that F23 has the identical frames.
- NOT VERIFIED: the raw file runs/tsan-5/tsan.50099 is not on disk (runs/ dir absent under s-rta-0929b), so I
  could not read F23's full frame list; conclusion rests on the sweep table row + F8's raw-frame reading.

## Refutation attempts
- Synchronised by a lock/atomic/message post the tool cannot see? No (VERIFIED: no lock, no post between the
  two open() calls). Not a tool artefact: the C++ standard guarantees no race on the characters only, not on
  stream format state, so TSan is technically right.
- Does the app ever set cerr width/fill/precision (which would make the torn value matter)? VERIFIED by grep of
  src: no cerr setw/width/precision/rdbuf/sync_with_stdio hits. So both threads write width=0 over 0.
- Other operands (width_, frameRate_, duration_, path_, thumbMs) are per-VideoPlayer members / locals (VERIFIED).

## Live-show impact
- Worst case: two "[VideoPlayer] Opened:" lines interleave on stderr (cosmetic). No crash, no heap write, no
  visual change, not on audio/GL threads (MediaOpen pool). Likelihood: seen 1/6 launches, needs 2 concurrent
  video opens; likelihood of any visible symptom ~nil.

## Smallest fix (optional)
- Build the line in a std::ostringstream / juce::String and emit once (`std::cerr << line`), or a shared
  logLine() helper with a static mutex (off the audio thread; Sacred Rule 2 untouched). One helper covers
  F8, F23-F26. Or a TSan suppression `race:std::__1::cerr` for sweeps.
