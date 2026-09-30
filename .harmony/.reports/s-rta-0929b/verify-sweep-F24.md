# Verify sweep F24 - std::cerr race, VideoPlayer::open "Opened:" line (256 vs 254)
STATUS: DONE
VERDICT: BENIGN (Family A; tool report accurate, harm cosmetic only)

## Facts
- VERIFIED (tsan.50099 lines 109-130): write size 8 at 0x1ef720380 by T22 (VideoPlayer.cpp:256) vs prior write by T21
  (VideoPlayer.cpp:254); frames are libc++ pad_and_output.h; "Location is global 'std::__1::cerr'" (addr = cerr+0x20).
  Both threads are MediaOpen pool workers: runJob() -> VideoPlayer::open (MediaOpener.cpp:33; pool of kThreads, :68-71).
- VERIFIED (src/media/VideoPlayer.cpp:252-260): one unlocked chained `std::cerr << ... << std::endl`; the
  frames :253/:254/:255/:256/:260 in the raw file are the successive operands of this ONE statement (the same race
  reported as 4 pairs in this pid; F23-F26 are all it).
- VERIFIED: the 8-byte word is the ios_base width_ field: pad_and_output writes os.width(0) after each padded insert
  (:80) and reads it (:54). Not app data.

## Refutation attempts
- Hidden synchronisation? No: nothing serialises the chain. libc++ cerr is stdio-synced by default (INFERRED; grep of
  src for sync_with_stdio / cerr.rdbuf / setw on cerr: none, VERIFIED), so per-insertion fwrite is stdio-locked
  and the streambuf is not corrupted; only formatting state and insertion ORDER are unprotected. Not a tool artefact.
- Torn value? Both writers store 0 over 0 (nobody sets cerr width), aligned 8B store: no observable tear (INFERRED).
- Third-party benign pattern? It is app code racing on a libc++ global; the standard makes it a formal race but
  it is a known-benign class.

## Live-show impact
- VERIFIED symptom, real: tsan-5 app-err.log line 301 shows two "Opened:" lines character-interleaved
  ("(1920x[VideoPlayer] Opened: ... (1080, 30 fps, 6s1920x1080, codec=, h264 ..."). Happens when two videos
  open concurrently (multi-video deck load / column trigger). Cosmetic log garbling only.
- No crash, no heap write, no visual effect; not on audio/GL threads (worker pool). Cost: a blocking stderr write on an
  off-thread worker. Likelihood of the race: high whenever >=2 opens overlap (1/6 sweep launches under TSan);
  likelihood of show impact: nil.

## Smallest fix (optional, low value)
- Build the line in a juce::String / std::ostringstream and emit with ONE `std::cerr << line << '\n'`, or a tiny
  shared `logLine()` with a static std::mutex (workers only; never the audio thread). Same helper would quiet F1/F8/F23-F26.
  Alternatively a TSan suppression `race:std::__1::cerr` for sweeps. No app-behaviour change required.
