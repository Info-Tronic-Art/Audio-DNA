# ATTACK PAPER — lane gop2, seat decode (s-rta-0930)

Recovered by Harmony from the workflow journal (wf_d5f2da66-bc5): the seat returned its paper as structured output and did not write this file.

VERDICT: AMEND

## A1 [MUST] Item 3's new store gate is validated only on codecs where it cannot fail; the one codec/container E12 saw misbehave (MPEG-4 part 2 in MP4, 4/20 first outputs mismatching by timestamp) is waved off as an 'INFERRED best-effort-timestamp artefact'. MP4 is an accepted container, so a real defect there would store wrong frames in the cache permanently.

EVIDENCE: plan E12 (lines 86-93) and C4 (176-180). The old gate needs a key-flagged decoded frame (VideoPlayer.cpp:1435-1436, :1460). C4 adds `|| landedOnKeyPacket_`, which also admits every decoded frame of a run that landed on a key packet, leading B-frame outputs included. The only evidence offered is that the lane's identity ctest passes on that codec. That ctest ran under the old gate, so it says nothing about the new one. T3 (lines 319-337) covers only the intra-refresh H.264 fixture.

FIX: Before c3, re-run landing2 on MPEG-4-in-MP4 comparing decoded PIXELS (not just timestamps) to settle whether the 4/20 is an artefact. Add a T3 case on an MPEG-4 part 2 B-frame MP4 fixture, plus an open-GOP H.264 fixture, with the new gate on: cacheMismatches == 0 and every shown frame equal to forwardDecode. If either fails, limit the new gate to h264 recovery-point cases, or require the frame to have no AV_FRAME_FLAG_CORRUPT and pts >= the landing pts.

## A2 [MUST] The in-place window is computed once, at planning, from the share at that moment. The share is cap = total/active (GopCache.h:54-55), and active changes while a 250-decode lead-in (~440 ms) is in flight. The stepped and offline rigs never change the player count mid-run, so the 'window shrinks, never stalls' claim is untested.

EVIDENCE: plan F-A A1 'safe when the estimate is wrong' (143-146). E7/E9 use 4 players all active from the start (u8h.cpp, gc7step.cpp with a single player). capBytes() divides by the live `active` count (GopCache.h:55). Memory pressure >= 2 makes totalBytes() 0, so the cap drops to the floor (GopCache.h:50-53). The gain counts slots 'the clock will free' but assumes they stay in THIS player's share. A player that activates mid-lead, a pressure step, or a deck return (u11 shape) shrinks the share under a planned window. The plan's only backstop is the store's farthest-next-use verdict, and the window's bottom stores are the farthest-next-use frames, so they are the ones refused or replaced.

FIX: Add a T1b variant (stepped, deterministic) that halves budget.total, or activates a second player, halfway through the lead-in. Assert late <= HEAD's for that scenario, shown-frame identity, and no stall. Consider re-planning or truncating `windowLo` in onRunFrame when capSlots() shrinks.

## A3 [SHOULD] T1a(ii) cannot reproduce the live planner state. The test hard-codes prefetchAt 19, but the live formula gives a lower value with the plan's own inputs.

EVIDENCE: GopCache.h:318-322, prefetchAt = max(cap/2, min(cap-2, ceil(gop*decodeMs/frameMsEff*1.2))). With gop 250, decode 1.8 ms and frame 33.3 ms: ceil(16.2) = 17, so pAt = 17, not 19 (VideoPlayer.cpp:1317 computes the same 'urgent' as 16 at 1.76 ms). With pAt 17 and u 17, planPrefetch returns None (GopCache.h:379 `u >= prefetchAtN`) even WITH a Lead. The plan's 'target 231, windowLo 218' result is therefore an artefact of an unrealistic input. The live plan fires at u <= 15, where avail0 is larger and the gain smaller.

FIX: Derive pAt in the test from GopCache::prefetchAt(21, 250, 1.8, 1000/30.0). Recompute the pre-registered target, windowLo and seekFrom. Keep one case at u = pAt - 1.

## A4 [SHOULD] The lead-in keyframe estimate `key = (lo/gopFrames)*gopFrames` assumes keyframes at multiples of the LONGEST GOP. The plan labels the error 'self-trimming' but has no test for the harmful direction.

EVIDENCE: plan Item 1 (lines 207-210) and K3 (469-471). gopFramesEst_ is the index's longest keyframe gap (VideoPlayer.cpp:293-296). A file whose first keyframe is not at index 0, or with scene-cut GOPs, puts the real keyframe nearer than the estimate. That overstates the lead, so the gain and the window are too big. The window's bottom frames are then decoded, stored, and replaced by nearer frames before the clock reaches them. The result is wasted decode plus later DEMAND misses, which is the exact failure the plan wants to remove. T1a(iv) only checks avail0 <= window <= share, which holds trivially.

FIX: Use the real index: the actual keyframe at or before `lo` (the index already knows keyframes, since seekFrom resolves to one). Add a T1b case with a scene-cut / irregular-GOP fixture and assert late <= HEAD.

## A5 [SHOULD] Token starvation and CPU cost are not modelled. The window fires whenever u < prefetchAt, so in a 21-frame share with GOP 250 essentially every cycle becomes a full-GOP-decode run, and GC9 serializes them across 4 players.

EVIDENCE: plan E5/E7 (dec/up 10.8) and the GC9 token at VideoPlayer.cpp:1314-1320. A sliding window costs about G^2/2W = ~1500 decodes per GOP per player (plan A7). At 1.8 ms each across four players, decode uses ~1.3 cores of wall time against the audio-analysis and render threads, with no governor. The plan expects 'late 0' partly because the rig has no GL or compositor, yet E6 itself shows the rig undercounts live late by about 1.2-2.8x. Serialized ~440 ms runs x 4 players is ~1.8 s of token holding against a ~2.1 s per-player cycle (gopFrames cover at 16 frames), leaving little slack. G4 may pass while a busier live load fails.

FIX: Pre-register a fallback. Measure the token-wait distribution (time from plan to grant) in u8h. Add a rig row at load 6-9 with 4 concurrent players PLUS a busy render thread (a spin) to bound the live factor before Harmony spends live-gate time.

## A6 [SHOULD] landedOnKeyPacket_ is set once per seek and read for the whole run, and it is shared with DEMAND and forward seeks. The re-seek path and the av_seek_frame fallback are not addressed.

EVIDENCE: plan Item 3 (lines 310-315); runSawKey_ is reset at VideoPlayer.cpp:1345. seekToTimestamp falls back to av_seek_frame(0) when the first seek fails (VideoPlayer.cpp:1590-1593). The run seeks back again at :1428-1431 (`seekPending`) when it lands above its window. If landedOnKeyPacket_ is not reset together with runSawKey_ at :1345 and on every re-seek, a stale 'true' from a previous key landing can open the gate for a later non-key landing (the TS case), while m4 tests only the always-true extreme. With thread_count 2 (VideoPlayer.cpp:128) and frame threading, output lags the packet, so the flag is per run, not per frame.

FIX: Reset `landedOnKeyPacket_` and `firstPacketSinceSeek_` in the same place runSawKey_ is cleared (:1345) and in seekToTimestamp. Add a mutant m5: a stale flag across two seeks (key landing, then TS-style non-key landing in one player) must still fail the GC3 TS case.

## A7 [SHOULD] 'Forward golden-trace identity' is asserted as verified, but the plan's own patch edits decodeNextFrame (a hot forward path) and planPrefetchRun's signature. The offline 510/5 run used scratch p6.py, which carries NOINPLACE/NOLANDKEY switches, not the clean patch the builder will write.

EVIDENCE: E8 (lines 70-72) and the EVIDENCE INDEX (523-524: 'The builder writes a clean version without them'). The committed-suite pass on a switchable variant does not prove the clean patch is identical. The two new flags are member writes on the forward path; the plan says they are 'decode-thread only' without confirming that no other thread calls seekToTimestamp / decodeNextFrame (e.g. the open() probe seeks).

FIX: Make G1 an explicit acceptance item on the committed clean diff, not on p6.py. Grep every caller of seekToTimestamp / decodeNextFrame and record the thread for each. TSan G2 must include the flag members in test_video_decode_trace (it does, but state that it is run on the clean diff).

## A8 [NIT] The VFR 'by design' decision is argued from ffprobe-simulated tables (scratch vfr/count.py), not from the player. No committed test pins the 0 % phone-jitter or the 4.4 % mixed-rate numbers; the docs will print these numbers as fact.

EVIDENCE: E16 (lines 112-124). The committed R2 case floor stays unchanged (Item 4, line 356). The doc text in section 6(c) quotes 4.4 / 9.1 / 23.5 / 56 %.

FIX: Either label those percentages 'simulated from ffprobe timestamps' in rendering.md, or add one committed fixture case that counts frames never shown in reverse on a jittered 30 fps file (expect 0).

## A9 [NIT] The G4 prediction 'B late 0-15' rests on an undocumented rig-to-live factor, and the PARTIAL branch (40 < late < A) depends on a Harmony re-threshold (A2) that would touch a committed test.

EVIDENCE: plan section 5 G4 (lines 384-393) and E7. The 160 MB offline result (late 152) shows the margin is one budget step: the patch holds 0 at 192 MB but fails by 160 MB. A live machine with less headroom than the rig can sit on that cliff at 256 MB.

FIX: State the 192-to-160 MB cliff in the gate text and run G4 also at 192 MB on B so the margin is measured live, not inferred.

