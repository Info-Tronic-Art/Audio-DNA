# vupload -- lane report (s-rta-0929)

## BUILDER REPORT

STATUS: DONE
RESULT: Fix round 1 (vupload-fix1) applied adoption addendum 2 (VU14-VU17) on lane/vupload, on top of 3a9e166 (3 commits, then this report commit). VU15's pre-registered w7 diagnosis fired rule (a). The render-thread QoS raise (P2, 11f200d) caused w7 (b)'s heavier tail, so it is reverted (8952f97) and u2 is INFO. u7 now asserts hold 0 (4769fe8), and w10 runs on all three upload paths in one invocation (c5dfcf9). ctest serial 962 / 962.
FACTS: VU15 raw table: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/vupload/fix1/vu15-summary.txt` (logs in `.../fix1/vu15/`). Revert commit 8952f97 (src/render/GLThreadQos.h deleted; Renderer.cpp / OutputWindow.cpp without raiseRenderThreadQos). `.harmony/probe-vupload.py` (qos_info, u7 hold check). `.harmony/probe-video-w10-all.sh` (NEW). `docs/claude/rendering.md`, `docs/claude/testing-eyes.md`. ctest: `ctest --test-dir build-lane -j1` -> 962 / 962. CLAUDE.md is unchanged (24,522 B).
METHOD: Each ruling was checked against the code first. VU15 used a 3-arm driver: probe-vupload-ab.sh's lock / taint / dialog rules, with a rotated round-robin start per round. It ran 10 launches per arm, quiet and 0 tainted, and the decision rule was applied by a script, exactly as written. NOQOS is a scratch build of `git archive 3a9e166` with 11f200d reverse-applied, in its own build dir (its src is byte-identical to the committed revert, and u2 read 21 on it). New / changed gates went RED first on the pre-lane app or on 3a9e166, then GREEN on 3a9e166 and on the final reverted app.
CONFIDENCE+VERIFY: High. Rule (a) fired with margin: FINAL - NOQOS = 9 (8 without FINAL's 835 ms outlier launch), and NOQOS 2 = MAIN 2. Harmony re-proves with: `ctest --test-dir build-lane -j1`; `probe-video-w10-all.sh <pre-lane w10 refs>` (GREEN, 30 / 30 captures at max |diff| 0); `probe-vupload-ab.sh <main app> <lane app> 5 .harmony/probe-vupload.sh u7_reverse_pingpong <out>` (4 / 4 PASS); `probe-vupload.sh u2_gl_thread_qos,u4a_context_cycle` (INFO 21, GREEN).
UNKNOWNS/NOT-DONE: (1) Round-0 items (1)-(4) still stand: fps4kBlitMin is null; GPU-driven context loss is INFERRED; the Output-window-open state is untestable; there is no flake verdict for Tier-1 reaction_diffusion. (2) After the revert, only w1c x 2 launches, w7 x 5, u2 / u4a, u7 (A/B x 5), w10 x 3 paths and ctest were re-run, as rule (a) asks. The full round-0 battery (w2c 4K, u4b, u6, probe-crossfade, etc.) was NOT re-run on the reverted app. (3) Why a USER_INTERACTIVE render thread delays the message thread (a P-core / scheduling hypothesis) is not measured; rule (a) needs only the attribution. (4) main is not merged in (as ordered). Pitfall 59 is taken on main (g4cpu), so this lane's NN needs a new number.
NUANCE: (a) VU14: the kWriterLookAhead fix (90b2ca9) is ADOPTED. Plan section 6's line now reads "reseek geometry measured from the writer's look-ahead (kWriterLookAhead = kSlots - 1)". No code change; no doc quoted section 6. (b) w7 (b) is SETTLED by VU15 rule (a). In the same interleaved set, the QoS raise was the whole tail: > 20 ms round trips MAIN 2 / FINAL 11 / NOQOS 2 of 50. After the revert, w7 x 5 gave REVERT 1 / 25 and MAIN 2 / 25, all PASS. (c) One FINAL launch (r5) hit 834.9 / 79.9 / 173.4 ms, the worst w7 round trip seen in this session. It is with QoS on and did not recur on NOQOS or REVERT. (d) The first reverted w1c launch had higher peak_callback medians (2.24-2.33 ms, bar 4.0) than the second (0.92-0.99), and (b) landed exactly on its bar at 118.5 (pooled 6 loads: 119.2). The round-0 QoS app's final loads were 0.84-1.66 ms. Every bar was met, but the margin on w1c (b) is thinner without QoS. (e) VU16's RED is "absent" on the pre-lane app. The c1 app (counter present, pre-u3) reads hold 0 in all four u7 scenes, so reverse play never showed the hold bug and the assertion is a regression guard.
HANDOFF-NEEDS: none

### SUMMARY
P1 budget, P2 QoS (reverted in fix round 1, VU15), P3 IOSurface + blit + fences, P4a held slot + context-loss re-upload, P4b idle purge, with RED harness, ctests, probes, docs; four council-driven additions (VU2 fence-fail, VU8 exemptions, VU10 race test, VU11 QoS everywhere) and one measured fix (writer look-ahead geometry, VU7).

### RED / GREEN table (raw numbers)
| gate | pre-lane (main 3e15613) / RED app | lane |
|---|---|---|
| w1c (a) median per-poll peak_callback_ms <= 4.0, every load | main 6.55 / 6.58 / 6.55 (RED) | c2 (P1 only, INFO) 3.84 / 3.77 / 3.71; final 0.84-1.66 over 6 loads |
| w1c (b) median fps >= 118.5 | main 105.3 [104.1, 105.3, 106.0] (RED; load 7) | c2 117.0 (INFO, VU12 Q4); final pooled 6 loads 119.7 [120.0, 119.92, 120.0, 119.02, 119.56, 118.0] |
| w1c (c) uploads >= 595, late 0 / (d) hold 0 | 604-608, late 0 / hold absent | 600-608, late 0, hold 0, cap 2 (3 once), max 2 uploads/frame |
| w1d (VU9, INFO) 8 x 1080p | 88.7-88.8 fps, late 0 | c2 100.6-101.1 fps, skipped 2 / 17, max 8/frame; final 119.9-120.0 fps, skipped 0, max 5/frame, late 0, hold 0 |
| w2c 4 x 4K (a) INFO, (b)-(e) | interleaved A/B launch medians [84.88, 88.89, 89.41, 89.95, 90.36] | c5 [100.16, 102.64, 103.19, 103.68, 104.22] -> fps4kBlitMin null (97 < 98.36); final 101.6-110.5 (95.8-96.9 at load 10-14); peak upload 0.25-0.37 ms (main 2.0-8.3); late 0, hold 0 |
| w10 identity, 5 formats x 2 frames, <= 1/255 | the reference (self-check f0 vs f150: 255) | max |diff| 0 on all 10 captures on the blit, surface-client and malloc paths (each witnessed by the "upload=" log line), and 20/20 again on the final app |
| u2 gl_thread_qos == 33 (INFO since fix round 1: P2 reverted, VU15) | main absent; c1 21 | c3 33, and 33 after u4a's cycles (VU11); reverted app 21 (INFO) |
| u4a hold_no_texture delta 0 over 3 cycles (0 / 500 / 0 ms detached) | c1 463, c3 472 (RED) | c4 0, final 0; gen delta 3; late after settle 0 |
| u4b purged 8, drop >= 150 MB | c1 purged 0, footprint -26 / -27 MB (RED) | c6 8 / -318, -293 MB (rss -252 once, ~0 other runs); final 8 / -284 / -293 MB; steady late 0, hold 0 |
| u6 crossfade under a column trigger | main late 0 (hold absent); c1 0 / 0 | c4 / final hold 0, late 0, mid-fade blend (mixed code cells), B after |
| u7 reverse / ping-pong (VU7) vs main | -- | u5 app FAIL (reverse g30 8.6 vs 11.2 /s; g250 4.0 vs 4.4, late 485 vs 457); kSlots 4 FAIL (8.6 / 3.8); look-ahead fix, 2 x 5 interleaved pooled: g30 rev 11.3 vs 11.2, late 212 vs 223.5; g250 rev 4.5 vs 4.3, late 456.5 vs 457.5; ping-pong 30.2-30.4, late 0: 4/4 PASS |
| test_video_player_gl (2) | main's VideoPlayer: `REQUIRE( tex != 0 ) with expansion: 0 != 0` | pass (blit + malloc) |
| test_video_ring new cases / test_video_upload_budget | do not compile on main (`no member named 'peek'`; `'media/VideoUploadBudget.h' file not found`) | pass |
| ctest full serial | 920 (main) | 962 / 962 (final) |

Bisect of the u7 drop (3 interleaved rounds, g30 reverse uploads/s): main 12.0 / 12.6 / 10.6, c2 11.4 / 12.0 / 11.4, c3 10.8 / 11.4 / 10.4, c4 8.6 / 7.8 / 8.4, c5 8.6 / 8.4 / 7.6; seeks per 5 s: main 50-52, c3 50-51, c4 38-39.

### FILES CHANGED
- src/media/VideoUploadBudget.h (NEW): the count budget, maxDefer, exempt, admit / charge.
- src/media/VideoRing.h: Ring::peek, Ring::dropReady, Retire<N>.
- src/media/VideoStats.h: 5 int64 + 2 int counters.
- src/media/VideoPlayer.{h,cpp}: budget-before-pick, held slot + P4a re-upload, IOSurface slots / blit / fences / fallbacks, pollFences (WAIT_FAILED), releaseGL zeroing, trimIfIdle / purgeFreeSlots / unpurge + re-bind, kWriterLookAhead, TEST-ONLY ADNA_VIDEO_FORCE_FALLBACK, "upload=" in the Opened line, friend VideoPlayerTestAccess.
- src/render/GLThreadQos.h (NEW; DELETED by the fix-round-1 revert 8952f97, VU15), src/render/Renderer.{h,cpp}: videoUploadBudget_, frame-top video_max_uploads_per_frame / gl_thread_qos, raiseRenderThreadQos, glContextGen_, scanVideoIdle.
- src/ui/OutputWindow.cpp: raiseRenderThreadQos() in Presenter::newOpenGLContextCreated (VU11; g4cpu does not touch this file) -- reverted in fix round 1 (VU15): the file is back to main's.
- src/api/ApiServer.cpp / src/test/TestServer.{h,cpp}: the 7 video_* fields; TestServer gl_context_gen / gl_thread_qos / phys_footprint_mb + POST /api/debug/gl_context_cycle {"detached_ms"}.
- tests/CMakeLists.txt: test_video_upload_budget, test_video_player_gl (Apple), IOSurface / OpenGL frameworks on test_video_player_open + test_media_opener.
- tests/test_video_upload_budget.cpp (NEW), tests/test_video_player_gl.cpp (NEW), tests/test_video_ring.cpp (+8 cases), tests/fixtures/video_rawrgba_63x37.mov (28 KB), video_hapa_64x64.mov (5 KB).
- .harmony/probe-video.{py,json} (w1c / w1d / w2c / w10, fixture support: dur / alpha / needEncoder), .harmony/probe-vupload.{sh,py,json} (NEW), .harmony/probe-vupload-ab.{sh,py} (NEW).
- docs/claude/{rendering,pitfalls,testing-eyes}.md, CLAUDE.md (index NN), .harmony/APP-INVENTORY.md (reconcile note).

### TESTS
- ctest serial at HEAD: 962 / 962 pass (27 s).
- TSan (Debug, build-tsan): test_video_ring / test_video_upload_budget / test_video_player_open x3 each: 0 warnings; test_video_player_gl (reported): 0 warnings.
- Teeth, run on mutated copies: budget (no force-admit -> (2)(5); kFloor 1 -> (1)); ring (held released on signal -> Retire (b); contextLost clears held -> (c)(e); side-effecting peek -> peek case); GL (WAIT_FAILED ignored -> (3); rect not zeroed -> (2); RGBA into the BGRA surface -> (1); held released at upload -> (1); purging the held slot -> (5); trim keeping Ready -> (5)). Without the re-bind flag, (5) still passes on this M1 Pro: plan risk 4's "the binding survives a purge" is VERIFIED here, and the re-bind stays the default (VU1).
- Final-app battery (VU13; the fix app 372097de unless noted): probe-video x2: run 1 112 / 0 GREEN, run 2 111 / 1 (w7 (b) 62.5 ms at load 10-14); probe-vupload 19 / 0; probe-media-open 38 / 0; probe-crossfade 35 / 0; probe-async-load GREEN. On the pre-fix final (c8c3f150, identical except the two look-ahead constants): probe-video x2 112 / 0 both, probe-seq-vram 66 / 0, probe-image-load 37 / 0, probe-capture 9 / 0, probe-idle-paint 13 / 0 (i1 / i2 main CPU 94.9 / 111.9 ms/s, g4 141.9), probe-outputs 17 / 0, and Tier-1: test_effects 3 / 3, test_audio_reactivity 4 / 4, test_time_sweep 1 / 1, test_performance 2 / 2, test_sources 3 / 4 (reaction_diffusion Diffusion A PSNR, pre-existing, untouched source).
- w6b (a) 5-run distribution (VU13), interleaved: main 4 / 5 PASS (r3: one poll at 18.7 ms; maxima 15.6-18.7); lane 5 / 5 PASS (maxima 3.8-6.6 ms).

### SLIM CHECK
nothing to cut. The ctest seam (VideoPlayerTestAccess, fenceWaitOverride_) is used by the GL test cases 3-5. ADNA_VIDEO_FORCE_FALLBACK is the ruled VU4 hook (TEST_SERVER builds only). The "upload=" log field is the witness that w10's arms really took their path.

### ISSUES
- The plan's u4a RED premise did not hold for immediate cycles; the row was strengthened (speed-0 layer + a 500 ms detach). A dangling-var bug in my first route version (the JSON var died before its object was read) was found by a live debug run and fixed before commit 1.
- VU7 bar failed, bisected, and was fixed in commit 90b2ca9 (see NUANCE (a)); kSlots stays 3.
- stash-guard hook blocked one combined `git add && git commit` (a peer session was live); the same files were then staged by name and committed with --file.

### SKILL_PROPOSALS
none.

### RISKS
- (RESOLVED in fix round 1) w7 (b) tail: VU15 attributed it to the QoS-raised render thread (rule (a)), and P2 was reverted (8952f97). REVERT's w7 tail matches main's.
- (low, fix round 1) Without the QoS raise, w1c (b)'s margin is thin: one launch landed exactly on 118.5 (pooled 6 loads 119.2). The w2c 4K numbers in the table above were measured WITH QoS and were not re-measured.
- (low) With the budget only (no blit) and a long render frame, a held content frame can be passed over. It is stated in the docs and bounded by maxDefer; 0 skips on the final app.
- (low) rss does not always show purged IOSurface pages (plan risk 5); phys_footprint does (-284 to -318 MB).

### METRICS
- Self-check: full builds clean (the only new warning, CGLErrorString deprecation, was removed); ctest 962/962; TSan clean; 3 probe batteries.
- Tool calls: about 230. Files read: about 45.

### KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative), so the posture was conservative: VideoPlayer.cpp consumers were listed by grep (app, test_video_player_open, test_media_opener), and all three were rebuilt and run. God nodes in scope: not queried. Risk level: NORMAL.

### PACKET QUALITY
- Clarity: HAD_TO_INFER. The plan body conflicted with itself (R-2's cap-2 arithmetic vs 4.1's re-ask counting; R-7's "reseekBehindSec stays a valid bound" vs VU7's no-regression bar), and u4a's RED premise did not hold.
- Missing context: CLAUDE.md was already at 24,363 B on main (the plan's paid trims were unnecessary); the TestServer listens on "localhost" (IPv6); probe-async-load's summary line counts only phase 2.
- Unused context: the CLAUDE.md trim list (section 10).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: plan-vupload.md + adoption, the three attack papers, the diag-vfps tools (instr.diff, run_mix.sh) and the asyncload battery script were all useful.

### STATUS
DONE (after fix round 1): every VU1-VU17 ruling is implemented and gated. VU14 adopted the look-ahead fix, and VU15 rule (a) removed the QoS raise that caused w7 (b)'s tail. Round-0 status was DONE_WITH_CONCERNS; both of its concerns are now ruled or resolved.

### NEXT ACTION
Harmony: trial-merge / rebase onto main (g4cpu merged at c432e3f; Pitfall 59 is taken, so this lane's "NN (vupload)" needs 60 or the next free number). Then gate and merge.

### NOTES FOR .harmony/notebook.md (Harmony appends)
- 2026-09-29 | A video frame's shown slot is held (VideoRing::Retire): the writer look-ahead is kSlots - 1, and the reseek distance / ahead-drop line must be measured from it (kWriterLookAhead). Measured from kSlots, reverse play lost 25 % of its frames, and kSlots 4 did not help | discovered: src/media/VideoPlayer.cpp decodeLoop + uploadToTexture pick.
- 2026-09-29 | TestServer listens on "localhost" (may be ::1): probe clients must use http://localhost:8080, never 127.0.0.1 | discovered: src/test/TestServer.cpp:84.
- 2026-09-29 | `auto* obj = juce::JSON::parse(x).getDynamicObject()` dangles (the var dies at the end of the statement): keep the parsed var alive | discovered: src/test/TestServer.cpp handleGlContextCycle (caught live: detached_ms read as 0).
- 2026-09-29 | A context cycle with an immediate re-attach does not starve a PLAYING video: the ~60 ms shader recompile refills every ring. The P4a bug needs a speed-0 / paused clip or a long detach to show | discovered: .harmony/probe-vupload.py u4a.
- 2026-09-29 | IOSurface blit into a sampler2D is per-pixel identical to the RGBA client upload (0 / 255 on yuv420p, yuv422p10le, ProRes 4444 alpha, HAP Alpha, HEVC 10-bit); the binding survives IOSurfaceSetPurgeable Empty -> NonVolatile on the M1 Pro | discovered: tests/test_video_player_gl.cpp, probe-video w10.


## Fix round 1 (vupload-fix1, 2026-09-29 19:25-19:58; plan-vupload.md HARMONY ADOPTION ADDENDUM 2, VU14-VU17)

Continued on lane/vupload from 3a9e166 (no reset, no rebase, main NOT merged in). App copies: the pre-lane app =
`apps/main-3e15613.app` (sha 9d216816); FINAL = 3a9e166's build-lane app, copied before the first change to
`apps/final-3a9e166.app` (372097de; `cmake --build` at 3a9e166 was a no-op, so this is the lane's final2 binary);
NOQOS = `apps/noqos.app` (bb08a66d); REVERT = the lane after 8952f97 (`apps/revert.app`, 2d1be8f4). All under
`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/vupload/`.

| ruling | commit | RED line (verbatim) | GREEN line (verbatim) |
|---|---|---|---|
| VU14 kWriterLookAhead adopted | none (report only) | -- | section 6 now reads "reseek geometry measured from the writer's look-ahead (kWriterLookAhead = kSlots - 1)"; no doc or probe quoted section 6 (`git grep` for reseekBehindSec / MUST NOT CHANGE: only code), and NUANCE (a) above was reworded |
| VU15 w7 (b) diagnosis -> rule (a) | 8952f97 (revert of 11f200d + u2 INFO + docs) | FINAL 3a9e166 r5: `FAIL  w7_message_thread_no_wait: (b) every trigger round trip [10.6, 8.1, 834.9, 79.9, 173.4] <= 30 ms`; pooled > 20 ms: MAIN 2, FINAL 11, NOQOS 2 (of 50 each) | REVERT w7 x 5 (MAIN interleaved): 5 / 5 `PASS  w7_message_thread_no_wait: (b) every trigger round trip [...] <= 30 ms`, > 20 ms REVERT 1 / 25 vs MAIN 2 / 25; w1c `PASS  w1c_column_trigger_1080x4: (b) median over 3 loads of the per-load median fps 118.5 >= 118.5 ([120.0, 116.8, 118.5])` and `... 119.5 >= 118.5 ([119.5, 119.6, 118.9])`, PY 10 PASS / 0 FAIL x 2; `INFO  u2_gl_thread_qos: the render thread's gl_thread_qos 21 -- QoS not applied (VU15) (21 = DEFAULT, 33 = USER_INTERACTIVE)` |
| VU16 u7 asserts hold 0 | 4769fe8 | main 3e15613: `FAIL  u7_reverse_pingpong[g30_1080_reverse]: video_hold_no_texture absent (the app predates it)` (x4 scenes, PY 0 PASS / 4 FAIL) | 3a9e166 and REVERT: `PASS  u7_reverse_pingpong[g30_1080_reverse]: (VU5) video_hold_no_texture delta over the 5 s window 0 == 0` (x4 scenes, PY 4 PASS / 0 FAIL) |
| VU17 w10 on 3 paths, one invocation | c5dfcf9 | main 3e15613: `FAIL  blit: witness -- 0 / 10 Opened lines say upload=iosurface-blit (the arm did not take its path)` (client / malloc alike) -> `PROBE-VIDEO-W10-ALL RED` | 3a9e166 and REVERT: `PASS  malloc: witness -- 10 / 10 Opened lines say upload=malloc` (blit / client alike), each arm `PY 34 PASS / 0 FAIL`, 30 / 30 captures `max |diff| vs the reference 0 <= 1` -> `PROBE-VIDEO-W10-ALL GREEN` |
| full ctest serial | at 8952f97 | -- | `100% tests passed, 0 tests failed out of 962` (26.84 s) |

### VU15 -- every launch (w7 only, 10 per arm, interleaved round-robin with the start arm rotated per round, quiet)
Driver: `fix1/ab3.sh` (probe-vupload-ab.sh generalised to N arms: acquire_quiet_lock, a launch that saw a compiler is
re-run, UserNotificationCenter STOP). 19:31:42-19:37:45, 0 tainted, UNC windows 0 and Output-named windows 0 after every
launch. NOQOS witness before the batch: `FAIL  u2_gl_thread_qos: the render thread's gl_thread_qos 21 == 33` (old u2 row)
vs FINAL `PASS ... 33 == 33`. NOQOS src == the committed revert's src (`diff -r` empty). Summary by `fix1/w7sum.py`:
```
ARM MAIN: launches 10, round trips 50, pooled > 20 ms 2, per-launch max [29.8, 29.0, 16.7, 14.8, 18.8, 15.8, 16.0, 11.5, 14.6, 18.1] (median 16.7, worst 29.8), (b) FAILs 0
ARM FINAL: launches 10, round trips 50, pooled > 20 ms 11, per-launch max [18.8, 25.5, 22.5, 13.1, 834.9, 16.2, 22.1, 27.7, 24.6, 22.8] (median 22.8, worst 834.9), (b) FAILs 1
ARM NOQOS: launches 10, round trips 50, pooled > 20 ms 2, per-launch max [9.5, 13.6, 23.4, 13.2, 14.5, 16.7, 14.0, 16.4, 17.1, 16.8] (median 16.4, worst 23.4), (b) FAILs 0

FINAL  r1  rtts [8.5, 8.5, 10.4, 18.8, 18.3] max  18.8 >20ms 0 lockwait 0.00029 load 6.70 6.00 6.35 (b) PASS | PROBE-VIDEO GREEN
MAIN   r1  rtts [5.0, 4.9, 4.8, 29.8, 15.1] max  29.8 >20ms 1 lockwait 0.00038 load 6.25 5.90 6.32 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r1  rtts [6.4, 7.5, 7.4, 9.5, 9.3] max   9.5 >20ms 0 lockwait 0.00038 load 6.50 5.98 6.34 (b) PASS | PROBE-VIDEO GREEN
FINAL  r2  rtts [12.6, 25.5, 7.8, 16.2, 22.7] max  25.5 >20ms 2 lockwait 0.00033 load 6.44 5.98 6.34 (b) PASS | PROBE-VIDEO GREEN
MAIN   r2  rtts [4.8, 5.8, 12.1, 13.1, 29.0] max  29.0 >20ms 1 lockwait 0.00050 load 13.04 7.53 6.89 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r2  rtts [7.1, 7.6, 8.8, 12.7, 13.6] max  13.6 >20ms 0 lockwait 0.00017 load 8.03 6.37 6.47 (b) PASS | PROBE-VIDEO GREEN
FINAL  r3  rtts [5.3, 4.7, 5.3, 22.5, 10.2] max  22.5 >20ms 1 lockwait 0.00079 load 10.55 7.45 6.88 (b) PASS | PROBE-VIDEO GREEN
MAIN   r3  rtts [6.8, 6.1, 12.9, 12.2, 16.7] max  16.7 >20ms 0 lockwait 0.00046 load 11.36 7.50 6.89 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r3  rtts [4.5, 10.5, 23.4, 19.0, 21.5] max  23.4 >20ms 2 lockwait 0.00075 load 12.13 7.52 6.89 (b) PASS | PROBE-VIDEO GREEN
FINAL  r4  rtts [4.6, 4.4, 5.7, 9.5, 13.1] max  13.1 >20ms 0 lockwait 0.00017 load 9.08 7.31 6.84 (b) PASS | PROBE-VIDEO GREEN
MAIN   r4  rtts [10.7, 14.8, 7.0, 6.2, 7.3] max  14.8 >20ms 0 lockwait 0.00062 load 9.54 7.34 6.84 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r4  rtts [6.0, 10.1, 10.5, 13.2, 11.0] max  13.2 >20ms 0 lockwait 0.00029 load 8.93 7.36 6.86 (b) PASS | PROBE-VIDEO GREEN
FINAL  r5  rtts [10.6, 8.1, 834.9, 79.9, 173.4] max 834.9 >20ms 3 lockwait 0.00042 load 8.57 7.33 6.86 (b) FAIL | PROBE-VIDEO RED
MAIN   r5  rtts [4.3, 17.3, 18.8, 16.7, 17.5] max  18.8 >20ms 0 lockwait 0.00025 load 7.70 7.21 6.83 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r5  rtts [4.3, 5.9, 8.5, 14.5, 13.1] max  14.5 >20ms 0 lockwait 0.00025 load 8.09 7.27 6.84 (b) PASS | PROBE-VIDEO GREEN
FINAL  r6  rtts [16.2, 6.0, 13.1, 7.3, 10.3] max  16.2 >20ms 0 lockwait 0.00067 load 7.60 7.18 6.83 (b) PASS | PROBE-VIDEO GREEN
MAIN   r6  rtts [8.0, 4.3, 5.2, 11.7, 15.8] max  15.8 >20ms 0 lockwait 0.00029 load 6.51 6.97 6.75 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r6  rtts [4.3, 9.8, 8.3, 16.6, 16.7] max  16.7 >20ms 0 lockwait 0.00029 load 7.05 7.09 6.79 (b) PASS | PROBE-VIDEO GREEN
FINAL  r7  rtts [5.6, 9.7, 15.7, 22.1, 11.1] max  22.1 >20ms 1 lockwait 0.00033 load 11.79 8.13 7.17 (b) PASS | PROBE-VIDEO GREEN
MAIN   r7  rtts [16.0, 5.9, 9.6, 10.9, 13.5] max  16.0 >20ms 0 lockwait 0.00058 load 7.62 7.19 6.83 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r7  rtts [10.7, 14.0, 4.6, 11.9, 7.2] max  14.0 >20ms 0 lockwait 0.00033 load 11.29 8.15 7.19 (b) PASS | PROBE-VIDEO GREEN
FINAL  r8  rtts [6.3, 27.7, 4.3, 10.6, 13.2] max  27.7 >20ms 1 lockwait 0.00033 load 10.25 8.03 7.16 (b) PASS | PROBE-VIDEO GREEN
MAIN   r8  rtts [5.9, 11.0, 6.8, 9.3, 11.5] max  11.5 >20ms 0 lockwait 0.00046 load 8.54 7.78 7.09 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r8  rtts [8.2, 7.4, 10.4, 16.4, 14.6] max  16.4 >20ms 0 lockwait 0.00033 load 9.37 7.91 7.12 (b) PASS | PROBE-VIDEO GREEN
FINAL  r9  rtts [12.9, 22.6, 19.9, 24.6, 19.2] max  24.6 >20ms 2 lockwait 0.00062 load 8.62 7.90 7.16 (b) PASS | PROBE-VIDEO GREEN
MAIN   r9  rtts [7.5, 6.6, 9.4, 14.6, 13.0] max  14.6 >20ms 0 lockwait 0.00017 load 8.99 7.94 7.17 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r9  rtts [9.4, 6.0, 8.3, 11.8, 17.1] max  17.1 >20ms 0 lockwait 0.00025 load 8.82 7.87 7.13 (b) PASS | PROBE-VIDEO GREEN
FINAL  r10 rtts [5.8, 4.8, 15.9, 18.9, 22.8] max  22.8 >20ms 1 lockwait 0.00021 load 7.66 7.72 7.11 (b) PASS | PROBE-VIDEO GREEN
MAIN   r10 rtts [5.4, 7.1, 7.8, 15.0, 18.1] max  18.1 >20ms 0 lockwait 0.00046 load 8.23 7.83 7.14 (b) PASS | PROBE-VIDEO GREEN
NOQOS  r10 rtts [6.6, 7.6, 10.8, 11.9, 16.8] max  16.8 >20ms 0 lockwait 0.00029 load 6.95 7.57 7.06 (b) PASS | PROBE-VIDEO GREEN

DECISION INPUT: MAIN 2, FINAL 11, NOQOS 2; FINAL - NOQOS = 9
(a) FINAL-NOQOS>=4 [True] AND NOQOS<=MAIN+2 [True] -> True
(b) |FINAL-NOQOS|<4 [False] AND FINAL>=MAIN+4 [True] -> False
(c) FINAL<=MAIN+3 -> False
FIRED: ['a']
```
Rule (a) FIRED, so the ruling's actions were taken: 11f200d reverted (8952f97, a revert commit); u2 is INFO "QoS not
applied (VU15)" (u4a (f) too); docs updated (rendering.md P2: tried, reverted, why; testing-eyes.md gl_thread_qos 21);
w1c 3 loads x 2 launches + w7 x 5 re-run on the reverted app with bars unchanged.

### VU15 (a) re-runs on the reverted app (2d1be8f4)
- w1c x 2 launches (19:43:22-19:44:22, load 4.1-4.8): launch 1 fps [120.0, 116.8, 118.5], peak_callback medians 2.33 /
  2.32 / 2.24 ms, uploads 602 / 604 / 604, late 0, hold 0, cap 2; launch 2 fps [119.46, 119.59, 118.93], peak 0.92 / 0.99
  / 0.93 ms, uploads 602 / 604 / 606, late 0, hold 0. Pooled 6 loads: median fps 119.2 (bar 118.5). Both PROBE-VIDEO GREEN.
- w7 x 5, MAIN interleaved (19:45:07-19:46:55, load 3.8-6.3):
```
ARM REVERT: launches 5, round trips 25, pooled > 20 ms 1, per-launch max [12.7, 19.8, 20.7, 15.3, 12.9] (median 15.3, worst 20.7), (b) FAILs 0
ARM MAIN: launches 5, round trips 25, pooled > 20 ms 2, per-launch max [12.4, 27.9, 13.4, 9.0, 10.9] (median 12.4, worst 27.9), (b) FAILs 0

MAIN   r1  rtts [4.3, 4.9, 4.0, 12.4, 9.7] max  12.4 >20ms 0 lockwait 0.00062 load 3.83 5.09 6.02 (b) PASS | PROBE-VIDEO GREEN
REVERT r1  rtts [4.8, 5.6, 8.9, 12.7, 11.7] max  12.7 >20ms 0 lockwait 0.00021 load 3.89 5.14 6.05 (b) PASS | PROBE-VIDEO GREEN
MAIN   r2  rtts [13.2, 27.9, 15.1, 26.4, 16.8] max  27.9 >20ms 2 lockwait 0.01375 load 4.10 5.10 6.01 (b) PASS | PROBE-VIDEO GREEN
REVERT r2  rtts [4.7, 7.0, 16.5, 19.8, 18.2] max  19.8 >20ms 0 lockwait 0.00042 load 4.64 5.18 6.03 (b) PASS | PROBE-VIDEO GREEN
MAIN   r3  rtts [3.7, 6.6, 8.4, 12.7, 13.4] max  13.4 >20ms 0 lockwait 0.00071 load 6.32 5.52 6.12 (b) PASS | PROBE-VIDEO GREEN
REVERT r3  rtts [10.7, 6.4, 8.7, 12.2, 20.7] max  20.7 >20ms 1 lockwait 0.00054 load 4.94 5.22 6.03 (b) PASS | PROBE-VIDEO GREEN
MAIN   r4  rtts [4.0, 5.4, 7.0, 6.5, 9.0] max   9.0 >20ms 0 lockwait 0.00017 load 6.22 5.52 6.12 (b) PASS | PROBE-VIDEO GREEN
REVERT r4  rtts [5.8, 4.2, 6.9, 8.4, 15.3] max  15.3 >20ms 0 lockwait 0.00021 load 5.57 5.41 6.07 (b) PASS | PROBE-VIDEO GREEN
MAIN   r5  rtts [5.0, 4.9, 10.0, 10.9, 4.6] max  10.9 >20ms 0 lockwait 0.00021 load 4.34 5.14 5.96 (b) PASS | PROBE-VIDEO GREEN
REVERT r5  rtts [12.9, 5.3, 4.3, 5.3, 12.0] max  12.9 >20ms 0 lockwait 0.00042 load 4.86 5.26 6.01 (b) PASS | PROBE-VIDEO GREEN
```
- u2 / u4a: INFO 21 before and after the cycles; u4a (a)-(e) PASS (gen delta 3, hold_no_texture 0, late after settle 0).
- Not required by rule (a), but run because the QoS change could affect it: u7 A/B main vs REVERT, 5 x 2 interleaved
  (19:52:20-19:57:53): `PASS  g30_1080_reverse: uploads/s A 11.6 B 11.4 (>= 0.9 x A: 10.44), late A 229.0 B 209.0`,
  `PASS  a1080_g250_reverse: uploads/s A 4.2 B 4.6`, both ping-pong PASS (30.2-30.4) -> 4 / 4 PASS; REVERT hold 0 in all 20
  scenes. (A single u7 launch on REVERT had shown g30 reverse at 9.4 uploads/s; the 5-launch set is 10.4-12.0.) The A
  arm's launches end PROBE-VUPLOAD RED because of VU16's "absent" FAILs on the pre-lane app; the ab verdict reads DATA lines.
- w10 three paths on REVERT: PROBE-VIDEO-W10-ALL GREEN (30 / 30 at max |diff| 0). I looked at one malloc-arm frame
  (pr4444a f0): the colour bars and the alpha ramp show the warm layer on the left, and the code band is black (= 0).
- ctest serial at 8952f97: 962 / 962.

### Rig
The lock was held per batch (longest 11 min) and released between batches, with the helper's 45 s cooldown. At most one
Audio-DNA ran at a time, always `open -g`. No Output window was opened (Output-named windows 0 after every launch). No
debugger, no screen capture, no synthetic input, and no UserNotificationCenter window appeared. The NOQOS scratch build
is in `fix1/build-noqos` (not committed). No .venv symlink was needed (VIDEO_PY was set explicitly). build-lane is kept.

### Notes for .harmony/notebook.md (fix round 1; Harmony appends)
- 2026-09-29 | Raising JUCE's GL render thread to QoS USER_INTERACTIVE (pthread_set_qos_class_self_np in newOpenGLContextCreated) delays the MESSAGE thread: w7 image-trigger round trips > 20 ms went 2 -> 11 of 50 (one launch 835 ms), and reverting it alone brought them back to 2 (10 x 3 interleaved launches). A render-thread QoS change needs a message-thread latency gate, not only fps | discovered: src/render/Renderer.cpp newOpenGLContextCreated (reverted 8952f97), .harmony/probe-video.py w7.
- 2026-09-29 | A pre-registered 3-arm diagnosis (MAIN / FINAL / FINAL-minus-one-commit) needs the minus-one arm's src built from `git archive <sha>` + a reverse-applied `git diff <c>^ <c>` in a scratch dir and build dir, never a second worktree: this adds no .git metadata, and `diff -r` against the later revert commit proves the arm IS that commit | discovered: fix1/noqos_build.sh.

INBOX-RECHECK: none
