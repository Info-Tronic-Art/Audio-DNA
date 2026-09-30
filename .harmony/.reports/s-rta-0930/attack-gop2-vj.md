# ATTACK PAPER — lane gop2, seat vj (s-rta-0930)

Recovered by Harmony from the workflow journal (wf_d5f2da66-bc5): the seat returned its paper as structured output and did not write this file.

VERDICT: AMEND

## A1 [MUST] The plan ships no answer for the low-RAM column: below ~192 MB the bar still fails after the patch, and nothing is specified for the floor case.

EVIDENCE: plan E7 table: 160 MB x4 late 152 (bar is 40) and 128 MB late 1283 ('10 slots is below feasibility'). Budget is min(2 GiB, RAM/16) (per the commit log), so a 4 GB Mac gets ~256 MB total and an 8 GB Mac 512 MB. Four 1080p reversers on a tight machine is the performer's worst night. capSlots floors at kMinFrames=8 (GopCache.h:17, VideoPlayer.cpp:1045-1049), so the floor case is the design's normal mode there. G4 gates only 256 MB, and the 160/128 rows appear nowhere in the gates or docs.

FIX: Add a pre-registered G4b (INFO at minimum, ideally judged) at 160 and 128 MB. State what the performer sees there: a slideshow or hold, never garbage. Document the feasible floor (slots needed per 1080p reverser vs RAM). Consider a degrade policy, for example reversing players in a column over the floor drop to half rate or cap the active-reverser count, or at least warn in the UI. Put the numbers in rendering.md, not just the 256 MB win.

## A2 [MUST] Direction change, pause and speed change mid-play are never tested against the new in-place window, yet the window's gain assumes the clock keeps passing frames at the current rate.

EVIDENCE: servedDuringLead = floor(lead x decodeMs / frameMsEff) is computed at planning time (plan item 1). T1b (plan lines 242-243) covers one constant-speed reverse Loop for 9 s from the Loop wrap. T1a(iv) only checks a grid of avail0 <= window <= share. The A1 'safe when estimate is wrong' argument rests on farthest-next-use eviction (GopCache.h:199-261), but it is asserted and not tested for a stop or flip during the lead-in. The plan also says a window may overcommit slots now held by nearer frames, while the forward BEHIND pool (0 below 32 slots, GopCache.h:167-175) is what covers a flip. Shares of 21 slots have no Behind cover, so the flip case is exactly where the overcommit has no cushion.

FIX: Add a stepped test next to T1b: start a run with lead-in in flight, then (a) pause, (b) flip to forward, (c) reverse to 2x speed. Assert no shown-frame mismatch, late bounded, and that the cache keeps the frames nearest the clock. Add a mutant that overcommits and stalls the clock.

## A3 [SHOULD] The R3 dismissal 'no reachable benefit, .ts not accepted' relies on file extensions, but FFmpeg probes content, not names. A renamed or mislabelled MPEG-TS capture in a .mp4/.mov/.mkv name reaches the garbage path, and the plan leaves forward and reverse publish ungated.

EVIDENCE: E11 (MainComponent.cpp:5253-5254 extension list) plus E12 (MPEG-4 part 2 in TS: 606 wrong frames, up to 53 in a row). A VJ pulling broadcast or capture-card recordings will rename or rewrap. The new store gate only keeps garbage out of the CACHE; the DEMAND publish and forward seek still display it (plan C2/C3 rejected).

FIX: A cheap guard at open(): if formatCtx_->iformat->name contains mpegts/mpeg (index-less), log it and treat the clip as intra-only, or gate the landing-packet publish for that demuxer only. At minimum add a test that an mp4-named TS file does not show garbage, instead of documenting it away.

## A4 [SHOULD] The landing-key store gate widens trust in the demuxer exactly where the probe saw an unexplained anomaly, and the open-GOP leading-frame protection is weakened.

EVIDENCE: E12: MPEG-4-in-MP4 had 4/20 first outputs mismatch the reference by timestamp, dismissed as an INFERRED probe artefact. The current gate comment (VideoPlayer.cpp:1458-1459) says it exists because 'an open-GOP seek's leading frames reference the GOP before it'. With landedOnKeyPacket_, runSawKey_ turns true before any frame is output, so those leading frames become storable (only storable()/corrupt flag stands between them and the cache). E12 tested open-GOP H.264/HEVC on x264-made files only, not camera-made open-GOP (Sony/Canon/GoPro HEVC).

FIX: Resolve the 4/20 anomaly (INFERRED) before merge: run landing2.c with pts-ordered comparison. Add a real open-GOP fixture with leading B-frames and assert no stored frame differs from forward decode (the plan's own identity check). Or restrict the new acceptance to frames with pts >= landing packet pts.

## A5 [SHOULD] VFR/MPEG-TS/frozen-frame live feel is left as by-design with no runtime mitigation, and the 4K column result is called acceptable by default.

EVIDENCE: E16: 56% of frames never shown in reverse on screen-recording-like files; G6 (u10 4K) is INFO and printed, not judged; 4K row after patch is 14.4/s with late 298 (E7). Q2 default 'yes' ships a visibly stuttering column without Boris having seen it. The plan's B1/B2 ask him to verify after merge.

FIX: Make 4K reverse late a stated non-regression bound (late_patch <= late_HEAD, uploads/s >= HEAD) in G6 so it is judged. Gate merge on a Boris eyes check of the 4K column rather than defaulting.

## A6 [SHOULD] T1a(ii) pins exact window numbers (target 231, windowLo 218, seekFrom 218, iteration arithmetic) for a two-iteration heuristic, so the test locks the formula, not the behaviour; while T1b uses a pinned decodeMsEma_, so the live EMA path is never exercised by a test that can fail.

EVIDENCE: Plan lines 226-227 and 242-243: pinDecodeMs before EVERY step. K6 claims a rename breaks loudly but the EMA feedback (window grows when decode slows; K2) is never driven. m1 proves only that gain=0 fails.

FIX: Keep T1a(i),(iii),(iv). Replace the exact windowLo with a band (>= HEAD's, <= share). Add one T1b variant with decodeMs slewing (for example 2x slower mid-run, unpinned EMA) and assert late stays bounded.

## A7 [NIT] G4 prediction and PASS rule use only one fixture shape (GOP-250 1080p, 4 players); the tighter real-world case of mixed GOP lengths in one column (K3: gopFramesEst_ is the LONGEST interval) is untested.

EVIDENCE: K3 admits scene-cut GOPs over- or under-estimate lead; only invariant T1a(iv) covers it. A VJ column mixes a GOP-30 clip with a scene-cut 1080p clip.

FIX: Add an offline u8h run with a scene-cut GOP file (keyint variable) and one mixed column; report late, INFO.

## A8 [NIT] The rig-to-live factor is asserted (1.2-2.8x) but the live run is gated on Harmony after merge, and the fallback is a revert.

EVIDENCE: E6: rig late 44-101 vs live 123; K1. Patch rig late 0 x 2.8 = 0 gives no margin information; 192 MB late 0 but 160 MB 152, a 32 MB cliff.

FIX: Report the rig's patch result under artificial CPU load (stress-ng or 4 burner threads) at 192/224 MB so the cliff is located before merge.

