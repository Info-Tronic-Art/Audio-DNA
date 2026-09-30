# Verify sweep F1 (TSan std::cerr race, AnalysisThread.cpp:84 vs ApiServer.cpp:93 lambda)
VERDICT: BENIGN (real race in the C++ sense, no live-show effect). Owner: system (libc++ / unsynchronised std::cerr use in app).
Read: raw tsan-1/tsan.39597 (full), AnalysisThread.cpp:70-85, ApiServer.cpp:82-100, libc++ pad_and_output.h (SDK MacOSX26.2).

## What the report is (VERIFIED)
- Both frames are inside libc++ std::__pad_and_output. Read at pad_and_output.h:54 = `__iob.width()`; write at :80 = `__iob.width(0)`. Location = global std::cerr +0x20 = the ios_base width field (size 8). (offset->field mapping INFERRED from the two source lines; the lines themselves VERIFIED.)
- App side: T29 = AnalysisThread::run (:84, the one-time "[Analysis] source rate ..." line, printed when rate != lastSourceRate_, i.e. first loop iteration); T30 = the httplib thread lambda (ApiServer.cpp:93-94, "[API] HTTP server listening ..." at thread start). Both fire once at startup, hence 6/6 launches incl. idle.
## Refutation attempts
- Synchronised by something TSan cannot see? NO. std::cerr has no lock in libc++ for the formatted-output path (the standard only forbids data races on the stream object for non-synced use; sync_with_stdio makes stdio-level calls safe, not the ios_base fields). No app mutex around the two writes (VERIFIED by reading both sites). So the race is real.
- Tool artefact? No; both accesses are genuine plain 8-byte ops on the same address.
## Why it cannot hurt (VERIFIED unless labelled)
- The only shared mutable datum is width_. Nobody sets a width: grep of src for std::setw/setprecision/setfill/fixed/hex/showpoint and for cerr.width/precision/setf/fill/flags/rdbuf/tie/imbue = 0 hits (VERIFIED, 106 std::cerr sites, none with a manipulator). width_ is therefore 0 at every read and every write stores 0: value-identical race, no torn/wrong value possible (aligned 8-byte on arm64: no tearing INFERRED).
- Same-family fields touched (fill, flags, state bits) are only read in these paths; rdstate is written only on sputn failure (badbit/failbit), which needs a stderr write error; then all cerr output would stop -- diagnostics only.
- The character output goes through cerr's stdoutbuf -> FILE* stderr, which libc takes flockfile on per write (INFERRED from libc++ design; not read in this session). Worst case = two log lines interleaved mid-line.
- No heap object, no app data, no audio/render thread involved (frames: analysis + httplib startup thread only).
## Live-show impact
Nothing: no crash, no torn value, no visual effect. Worst realistic case: garbled/interleaved stderr text at startup. Likelihood of even that: tiny (two one-line prints within the same microseconds at boot). Not a show-time event; all later cerr lines from other threads are the same class (F8, F23-F26).
## Smallest fix (optional, low value)
Do not fix for stability. For a clean TSan baseline: add a TSan suppression `race:std::__1::__pad_and_output` (or `race:std::__1::cerr`), or route logs through one mutex-guarded helper / a single fwrite(stderr) per line. Not worth the diff otherwise.
## Risks / caveats
- Verified only the reported instance; the family-wide claim (all 106 cerr sites manipulator-free) is by grep, multi-line manipulators would still match the pattern (none found).
- If someone later adds `std::setw` / `std::fixed` to a cerr line, the race becomes value-changing (garbled formatting, still no memory unsafety).
