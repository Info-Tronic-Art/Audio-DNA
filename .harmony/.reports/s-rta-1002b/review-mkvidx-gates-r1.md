# Reviewer Verdict -- mkvidx gates r1
STATUS: DONE
VERDICT: PASS_WITH_NITS (APPROVE; 0 MUST, 2 SHOULD, 2 NIT)
REVIEWED: lane/mkvidx @ 79dd452f47d3e99303a82739e22695c699a3fde0 (read via git show / git diff against 9a832a0; binaries of build-lane / build-tsan run read-only)
METADATA: reviewer=gates-lens, builder_packet=mkvidx, date=2026-10-02

## Re-derived (VERIFIED, executed here, not recalled)
- RED on fa9604d: git-archive of fa9604d src + the head test TUs through the lane's rig.sh (copied, D repointed to my scratch):
  test_gop_cache.cpp -> 6 x "use of undeclared identifier 'keyIndexFrom'" (593-623); test_gop_cache_store "mkvidx*": 6 cases, 4 failed
  (T1 875/1/3240 intra 1; T2 FX1/FX3/FX7 2425/2141/2212 with keyRels {0}; T3 HAP 64/38/834, FX4 4/17/324; T4 FX2 4048/1180/452),
  shape + T3b pass by design; T2e on the Matroska file: keyRels {0} gop 90, 2 of 7 assertions fail (test_video_decode_trace.cpp:353-354).
  Every printed value == the ruling's G3 RED column.
- GREEN (build-lane, head sources): mkvidx* 6 cases / 384 assertions PASS; T2e + MP4 case 12 assertions PASS; test_gop_cache 164 / 14 PASS; whole
  test_gop_cache_store 1046 / 32, test_video_decode_trace 517 / 6, test_video_player_open 10 / 2 PASS; every printed row == the G3 GREEN column
  (libavformat 62.3.100); gop2 T1b/T1c/T1d lines == gop2.md's GREEN block. TSan build: T2e oracle PASS, 0 "WARNING: ThreadSanitizer" (and the lane's
  three TSan logs: 0 warnings, 1046/32, 517/6, 164/14).
- Tests drive the real code: emulateArm -> real VideoPlayer + Emulated + forwardDecode byte compare (mismatches 0); T2e = real decode thread.
- Mutant m3 (rebuild in seekToTimestamp, no decodeStep refresh) reproduced independently on a copied src (rig.sh + sed in scratch): fails ONLY
  T2 (b) (store.cpp:2032-2033) -- matches the lane's matrix. The lane's m1/m2/m4/m5/m6/m8/m9 matrix (diffs + results inline, copy-tree rig,
  deliverable never mutated, git diff 0 after each) is credible and each of m1-m6 has >= 1 tooth.
- Fixtures: sha256[:16] of all ten == AM5 (G5); FX1, FX2, FX4, FX6, FX7m regenerated here with brew FFmpeg 8.0 -> byte-identical hashes.
  Recipes are in the comment above the mkvidx cases (test_gop_cache_store.cpp ~1653-1680).
- ctest -N (build-lane) = 1122; exactly 8 new entries: mkvidx P (#986), threaded reverse on a Matroska file (#1001), shape (#1028), T1-T4 + T3b (#1029-1033).
- Greps on head: G8 (i) 1 hit, in runStep (VideoPlayer.cpp:1329, fn 1322); (ii) nextRel / target forward seeks 1 + 1 byte-identical; (iii) no reader of
  keyRels_/gopFramesEst_/keyIndex* outside VideoPlayer.{h,cpp}; (iv) decodeStep's first statement is readKeyIndex(false) (:783). G9: CLAUDE.md 23,976 B
  (<= 24,002 <= 25,000); both stale 1-entry-index strings gone; Pitfall 64 + CLAUDE.md line 64 (AM16 (c) text verbatim) present.
- readKeyIndex's readers are all decode-thread functions (forwardRetain/refreshView/enforceCap/planPrefetchRun/runStep...; callers :756/:809 in
  decodeLoop/decodeStep) -> no new cross-thread read; no mutex, no allocation off the decode thread, nothing on the audio callback.
- Stray check: head tree has no .venv, no env-var hook, no instrumentation; the only added non-fixture file is the lane report (git add -f).
- Plan/AM conformance: AM1-AM4 (guard, triple trigger, run-only aim, no seekToRel), AM5-AM12, AM13 matrix, AM14 (1)-(7) incl. [CTRL-A] first, [PP-PARITY],
  frozen bars in probe-vupload.json (blob e770e5c), AM16 (a)-(e), AM17/18 -- all present. Selftest: SELFTEST PASS (25 checks). u7/u8 refactor
  (rev_scene / column_window) reads output-identical (same strings incl. "5 s window"); u13 clip/layer ids and extras match u7's reverse/turn scenes.

## Findings
1. [SHOULD] .harmony/probe-vupload-ab.py selftest does not discriminate most u13 thresholds (dimension 7, negative fixture per gate). Executed
   mutants on a scratch copy, each keeping the launch gate and neutralising ONE comparison, all still print SELFTEST PASS: [CPU] `bd <= cr * ad`
   (:343), [PARITY] `bd <= pm * be` (:344), [GUARD] `bl <= al + ls` and `bp >= ap - ps` (:354, each alone), [PP-PARITY] `fl <= gl + pp` (:358),
   [HAP]'s `dp <= hmax` (:337), and [FREEZE]/[MKV]/[HAP] late / uploads-min / bracket / mono / nonmono each alone (:330-336). AM14 (7) asked for exactly
   three TSVs and the lane delivered them (all-pass, frozen-B (a), too-few) -- so not a MUST -- but G6's verdict rests on these rules being able to FAIL.
   Fix (cheap, before G6; does not touch the frozen JSON blob): loop the all-pass TSV with ONE B-arm perturbation per rule (B (c) decoded_per_upload 2.0 ->
   only [HAP]; B (d) decoded 8.0 -> [CPU]+[PARITY]; B (d) late 100 / per_player_min 25 -> [GUARD]; B (f) late 100 -> [PP-PARITY]) and assert the exact
   FAIL tag; add an A-arm TSV that trips each [CTRL-A] STOP / non-discriminating branch (:316+; only the "reads as reproducing" side is asserted, :125-128).
2. [SHOULD] Settle the "bracket_ok 1, mono_ok 1 ... nonmono 0 in every B launch" binding BEFORE Harmony records the blob (lane noted the ambiguity,
   report "Deviations (Stage B)"): [FREEZE]/[MKV]/[HAP] take bracket_ok / mono_ok as MEDIANS (probe-vupload-ab.py:330-336), so 2 of 5 B launches
   showing a wrong frame or a non-falling capture sequence still PASS. Per-launch is stricter and matches the frozen-before-launch intent (mutant `bracket`
   above shows the selftest cannot tell the difference either way). Either bind all three per launch or state the median reading in "_u13".
3. [NIT] src/media/VideoPlayer.cpp:1612-1617 -- keyIndexWitnessed_ is never reset by open() (only keyIndexOpenKeys_ is, :1610), so a re-opened
   VideoPlayer logs the "Keyframe index:" witness at most once per instance, not per file. INFO-only (production opens a NEW VideoPlayer per file via
   MediaOpener); reset it in the atOpen branch for honesty with the "once per player" comment, or ignore.
4. [NIT] tests/test_gop_cache_store.cpp T3b: bars are the base's values, so vfrgap's improvement (327 -> 318 decodes) is not pinned (a regression back to
   327 passes). Ruled by AM11 (a guard against worse-than-base, teeth = m6 which the lane showed fails it) -- recorded only.

## Residual risks (not defects)
- Live A arm never launched (adoption allows one INFO smoke): [CTRL-A], [CPU] ratio and every B-vs-A rule are unmeasured until G6; the smoke's single-player
  reverse scenes (a)/(b) decoded 0 in-window (all 300 frames resident at the 2 GiB share), so they discriminate by uploads/s / late / captures only.
- 4K live, mkvmerge/GStreamer Matroska layouts, AV1 in ctest: unmeasured/scratch-only, as the ruling states (RR2, A6).
- Shared $TMPDIR across concurrent agents clobbered one scratch file during this review (re-done in a mktemp dir); no effect on results.
