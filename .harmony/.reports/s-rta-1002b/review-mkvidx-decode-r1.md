# Reviewer Verdict -- mkvidx, lens decode, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST, 2 SHOULD, 1 NIT)
REVIEWED: lane/mkvidx @ 79dd452f47d3e99303a82739e22695c699a3fde0 (base 9a832a0; src/tests == fa9604d). Read via git show / git diff only.

## Answer
No defect that ships and no missed plan item. Decode path, thread ownership and the RUN-only aim conform to AM2 / AM3 / AM4.
Two SHOULD findings (both inside the ruled design's blind spots, neither a regression against fa9604d).

## Conformance (VERIFIED by reading + running)
- AM2 guard: VideoPlayer.cpp readKeyIndex starts `if (formatCtx_ == nullptr || videoStreamIndex_ < 0) return;` (~:1580). OK.
- AM3 triple trigger (entries, entry 0 ts, last ts; null entry = INT64_MIN) and `readKeyIndex(false)` as FIRST statement of decodeStep (:783). OK. intraOnly_ written only under atOpen (:1608-1609). OK.
- AM4: only runStep's seek changed (VideoPlayer.cpp:1329, `ptsOfRel(r.seekFrom) + 0.5*frameDur_ - runSeekBackSec_`); forwardStep (:873) and forwardIdle (:1519) are byte-identical to fa9604d in the diff (one seek hunk only). OK.
- Thread ownership: keyRels_ / gopFramesEst_ / keyIndex* written by open() before start(), then decode thread only; readers (planPrefetchRun, forwardRetain, forwardStep/forwardIdle thresholds, urgent line) are all decode-thread code. No new atomic / mutex / ring / GL touch (Pitfalls 56 / 60 / 62 untouched). Test accessor reads after join (test_video_decode_trace.cpp). OK.
- Steady state for MP4 / MOV / HAP / long-GOP Matroska: 3 O(1) accessor calls per step, no allocation unless the triple changed. OK (but see S1 for the all-intra Matroska case).
- Fixtures: all 10 sha256 prefixes recomputed from `git show 79dd452:tests/fixtures/*` == AM5. OK.
- Tests run on the lane's build-lane binaries (worktree clean at 79dd452): `test_gop_cache_store "mkvidx*"` All passed (384 assertions / 6 cases), `test_gop_cache "mkvidx*"` passed; printed values == G3 (T1 2186 = MP4, T3 HAP 0/75/78, FX4/FX4e 0/27/30).
- Tooth spot-check (mutant on a $TMPDIR copy of 79dd452 src, relinked with the lane rig; deliverable untouched): moving readKeyIndex from decodeStep to the head of seekToTimestamp -> mkvidx T2 (a) / (b) / (c) FAIL at test_gop_cache_store.cpp:2007-2009, :2032-2033, :1866. The new cases drive the real VideoPlayer and can fail.
- Stray: no .venv, no getenv/env hook, no instrumentation or mutant text in the diff (only the T3b comment's word "mutant"). CLAUDE.md = 23,976 B (<= 25,000); index line 64 == AM16 (c).
- Docs: Pitfall 64 text matches the code (run-only aim, trigger triple, open-only intra verdict).

## Findings
S1 SHOULD -- readKeyIndex is NOT O(1) / allocation-free on a Matroska file whose every frame is a keyframe.
  Where: src/media/VideoPlayer.cpp:783 (call), :1573-1603 (comment "O(1) unless the index changed", full rebuild + keyTs + k.rels vectors); VideoPlayer.h:285-292 comment.
  Evidence (VERIFIED, replica of the lane's rebuild logic, not the lane binary): forward-reading an all-intra H.264 or MJPEG .mkv grows the demuxer index by ONE entry per frame (idx2: 300 pkts -> 300 entries, EOF 1800). A 10-minute 18,000-frame MJPEG MKV: 18,000 rebuilds, avg 27 us, max 0.13 ms, 489 ms total; quadratic (a 1 h clip ~0.15 ms avg / ~0.8 ms max per frame plus ~1 MB of malloc/free per frame) on the decode thread. For intraOnly_ streams keyRels_ is never read (planPrefetchRun returns at :1281) -- the rebuild is pure waste there. Not a sacred thread, so SHOULD not MUST; but it contradicts the "no allocation in steady state / O(1)" claim and no test pins the rebuild count (AM9's fallback counter was not needed, so nothing counts rebuilds).
  Fix: when intraOnly_ skip the keyRels_ rebuild (recompute only gopFramesEst_ if wanted), or append incrementally when only `last` grew; add a VideoStats rebuild counter + a forward all-intra-MKV case asserting rebuilds << frames.

S2 SHOULD -- the open-time intra verdict (entries >= 2 && all keys && gap == 1) is fooled by a long-GOP Matroska whose first two keyframes are adjacent.
  Where: src/media/GopCache.h:493-494 (`k.intraOnly = entries >= 2 && keys == entries && k.gopFrames == 1`), decided once in VideoPlayer.cpp:1608-1609 and never revisited.
  Evidence: a Cues-at-end MKV made with `libx264 -g 250 -bf 0 -force_key_frames "0,0.03"` (keys at frames 0, 1, 251) opens with 2 index entries, both keyframes, gap 1 (idx probe: "entries 2 keys 2 gopFramesEst 1 keyRels {0 1}") -> intra-only at open. Replayed on the plan's proto3 binary (same B1 rule; NOT the lane binary): `mkvcost_proto3 adj3.mkv rev 21 3 9` -> "intra-only ... late 879 shown 1 decodes 3240 seeks 3240" = the X1 freeze; the same stream with ordinary keys: late 30 / shown 258. Not a regression (fa9604d calls it intra-only too, for any 2 keyframes), rare shape (min-keyint normally prevents it), but the lane's claim (Pitfall 64 (3), B1 "conservative") is wrong for it and the wrong-intra direction is the one that froze reverse.
  Fix / debt: file it beside F6 (the opposite direction). Candidate: at the first rebuild after open, if keyRels_ now shows a gap > 1 and the cache holds nothing, promote intraOnly_ -> false once (one-way); or require the gap rule over >= N entries.

N1 NIT -- keyIndexWitnessed_ is not reset in open() while keyIndexOpenKeys_ / intraOnly_ are (VideoPlayer.h:296, VideoPlayer.cpp:1612). A player object re-opened after close() would never log the witness line again. INFERRED (no reopen caller checked); INFO line only.

## SLIM
No excess code found in the diff (every new symbol is referenced: keyIndexFrom, readKeyIndex, the 5 members; test accessors used). DEBT_FILED for S1's wasted rebuild is the only candidate.

## Not reviewed (other lens)
.harmony/probe-vupload*.py / probe-video* / u13 rules and the live smoke; TSan (lane reports 0, not re-run); G6/G7 live A/B are Harmony's.

CONFIDENCE: High for conformance, thread ownership and test teeth (VERIFIED by running); S1 numbers VERIFIED on a replica; S2 index-level VERIFIED, freeze consequence VERIFIED on the prototype binary only.
