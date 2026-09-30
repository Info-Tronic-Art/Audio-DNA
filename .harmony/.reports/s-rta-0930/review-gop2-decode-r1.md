# Reviewer Verdict - gop2 decode r1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
REVIEWED_COMMIT: 48dd81b433a3aa67bdcc379f865c9e6012b30306 (base 655d232)
FILES: src/media/GopCache.h, src/media/VideoPlayer.{h,cpp}, tests/test_gop_cache{,_store}.cpp, 3 fixtures, probe-vupload-ab.{sh,py}, docs (rendering/pitfalls/testing-eyes/CLAUDE.md)
ISSUES: 0 MUST / 1 SHOULD / 3 NIT

VERIFIED (read via git show of the pinned head; ran the lane's prebuilt test binaries read-only):
- A1: keyRels_ built in open() from keyframe index entries relative to the FIRST key's DTS; empty -> grid fallback; keyAtOrBefore matches the ruling text exactly; planPrefetch two-iteration loop matches; kInPlaceMargin=1; avail0==0 and gopFrames==0 -> no Lead path.
- A2: storeGateOpens identical to ruling; static_assert(AV_NOPTS_VALUE == kNoPts) present; flags reset in seekToTimestamp (covers fallback seek), set on first VIDEO packet in decodeNextFrame; every run starts via runStep's seek (seekPending default true), the overshoot re-seek resets flags -> no stale landing state. Only Demand/Prefetch runs exist in the cpp (no seek-less Reposition run).
- Thread ownership: keyRels_ written only in open() (unpublished player), read on the decode thread; landing flags only touched in seekToTimestamp/decodeNextFrame (decode thread; open()'s own first decode runs with firstPacketSinceSeek_=false). No new mutex / atomic / allocation per step (Lead is a stack struct).
- Forward golden traces: decodeNextFrame/seekToTimestamp only gain flag writes; ran test_video_decode_trace 3x -> 510/5 each time. test_gop_cache 148/13; test_gop_cache_store "gop2*" 139/4 with the builder's printed values.
- Tests drive the real VideoPlayer (stepped harness), RED values equal the ruling's independent main numbers (69/56/403.., 169/77/3321); T1a/predicate are compile-RED on base (as ruled); T1d/T1c speed/shrink/flip are guards by design (ruling).
- Nothing stray in the diff (no .venv, no getenv/ADNA_ hook, no cerr); CLAUDE.md 23,996 B; docs carry (a)-(e), placeholder for G4 left by design; build-lane/ and build-lane-tsan/ are untracked dirs in the worktree only.
- probe .sh: adna() in the rig's lock.sh prints the app's pid (ps ucomm), so the burner taint is live only while the app runs.

SHOULD
- MKV/WebM (accepted by extension): VERIFIED with ffmpeg 8 that open()+first decode leaves a 1-entry index (libx264 .mkv with and without cues, libvpx .webm; MP4/AVI have 300). keys==1 -> gopFramesEst_=totalFrames_ (pre-existing) and keyRels_={0}, so every window's lead-in = lo frames and the gain clamps to the whole Future share: the window is always maximal whatever the real GOP. INFERRED cost: extra dropped decodes, bounded by the share (same class as the 2x-pin +11 %); no stall. The docs say "the longest-interval grid only without an index" - inaccurate for these containers; no test covers a 1-entry index. Fix: state it in rendering.md / Pitfall 62 (or file a follow-up to refresh keyRels_/gopFramesEst_ after the first seek, when the cues parse).

NIT
- GopCache.h servedDuringLead: static_cast<int>(floor(...)) is UB if lead*decodeMs/frameMsEff > INT_MAX; prefetchAt clamps with min(need, 1e9) for the same reason. Unreachable at real inputs (~1e6 frames x 1e3 ms / 2 ms = 5e8) - clamp anyway.
- keyRels_ is DTS-based (R3 of the ruling) and never refreshed; same staleness class as gopFramesEst_.
- Untracked build-lane/ and build-lane-tsan/ in the worktree: make sure the merge does not add them.
