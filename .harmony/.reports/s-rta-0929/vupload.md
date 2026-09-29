# vupload -- lane report (s-rta-0929)

## BUILDER REPORT

STATUS: DONE_WITH_CONCERNS
RESULT: Plan items u0-u5 + docs landed on lane/vupload (9 commits over main 3e15613, head = the report commit); every VU1-VU13 ruling applied; the final app passes the new rows and the existing battery, with three concerns listed below (a u3 fix beyond the plan body, w7 (b)'s heavier tail, fps4kBlitMin left null by the 4.9 rule).
FACTS: `src/media/VideoUploadBudget.h` (P1 budget), `src/media/VideoRing.h` (peek / dropReady / Retire), `src/media/VideoPlayer.cpp` (IOSurface slots, blit, fences, held slot, idle purge, kWriterLookAhead), `src/render/GLThreadQos.h` (P2), `src/render/Renderer.cpp` (frame-top budget, scanVideoIdle, gl_context_gen / gl_thread_qos), `src/test/TestServer.cpp` (POST /api/debug/gl_context_cycle), `tests/test_video_upload_budget.cpp`, `tests/test_video_player_gl.cpp`, `tests/test_video_ring.cpp`, `.harmony/probe-vupload.py`, `.harmony/probe-vupload-ab.sh`, `.harmony/probe-video.py` (w1c / w1d / w2c / w10), `docs/claude/rendering.md`, `docs/claude/pitfalls.md` (entry "NN (vupload)"), `CLAUDE.md` (24,522 B).
METHOD: The plan was followed item by item, one commit per item. Every new row / test went RED first, on the pre-lane app copy (main 3e15613 build, sha 9d216816) or on the commit-1 app. Mutation teeth were run on scratch COPIES only, never on the deliverable (the deliverable's sha was re-checked). All perf numbers came from interleaved A/B launches (probe-vupload-ab.sh, the run_mix pattern) taken under acquire_quiet_lock, with the load average printed. The final-app battery (VU13) and the 5 Tier-1 files were run, then ctest serial.
CONFIDENCE+VERIFY: High on correctness (per-pixel identity is max |diff| 0 on 3 upload paths x 5 formats x 2 frames; the GL ctest runs on real CGL; TSan is clean), and high on the 1080p perf gates. Harmony re-proves with: `ctest --test-dir build-lane -j1` (962/962); `probe-video.sh` twice on the final app (VIDEO_REF_DIR = the pre-lane w10 captures); `probe-vupload.sh u2_gl_thread_qos,u4a_context_cycle,u4b_idle_ring_trim,u6_crossfade_video`; `probe-vupload-ab.sh <main app> <lane app> 5 .harmony/probe-vupload.sh u7_reverse_pingpong <out>` (expect 4/4 PASS).
UNKNOWNS/NOT-DONE: (1) fps4kBlitMin stays null: the plan 4.9 rule refused a knife-edge bar (97 < 98.36), so w2c (a) is INFO. (2) VU6: a real GPU-driven context loss was not tested; the detach / attach cycle is the gate (INFERRED to share JUCE's path). (3) VU11's "preview detached while an Output window is open" state is not testable (no gate opens an Output window). (4) No flake verdict for Tier-1 test_sources' reaction_diffusion "Diffusion A". It failed 1/1 here (PSNR 58.6, limit 55); the source is untouched by this lane, and the same pre-existing failure is recorded in s-rta-0928's tier1.md. (5) w7 (b) attribution is not settled (see NUANCE).
NUANCE: (a) THE u3 FIX IS BEYOND THE PLAN BODY (commit 90b2ca9): VU7's u7 failed because the held slot makes reverse play re-seek one frame later per cycle (-25 % frames). VU7's kSlots 4 fallback did not fix it. The fix measures the reseek distance and the ahead-drop line from the writer's look-ahead (kSlots - 1). Plan section 6 lists reseekBehindSec under MUST NOT CHANGE; this change keeps main's reseek GEOMETRY instead, and it needs Harmony's ruling. (b) w7 (b) (existing, never re-thresholded) FAILED in 2 of about 22 lane-app launches (31.5 ms at load 4-6; 62.5 ms at load 10-14) and in 0 of 15 main launches. The lane's tail is heavier (7 of 50 round trips in 20-30 ms vs 1 of 75 on main). Interleaved sets: main vs c6 5/5 both PASS; main vs final 5/5 both PASS; c2 vs c3 (QoS only) 5/5 both PASS, c3 max 23.4 vs 19.0. w7 (a) (the lock-wait gate) stays about 0.0004 ms. (c) Budget demand counts FIRST asks only: the plan's formula counted re-asks, and that settles w1c at cap 3 instead of R-2's cap 2 (ctest (1) fails that way). (d) maxDefer uses frame duration / |speed| (the plan used the frame duration only); without that, a 2x clip would skip frames. (e) u4a gained a speed-0 layer and a 500 ms detach. Without them the predicted RED never shows: commit-1 app 0 over 3 immediate cycles, because the 60 ms recompile lets every playing ring refill. With them: commit-1 app 463, lane 0.
HANDOFF-NEEDS: none

### SUMMARY
P1 budget, P2 QoS, P3 IOSurface + blit + fences, P4a held slot + context-loss re-upload, P4b idle purge, with RED harness, ctests, probes, docs; four council-driven additions (VU2 fence-fail, VU8 exemptions, VU10 race test, VU11 QoS everywhere) and one measured fix (writer look-ahead geometry, VU7).

### RED / GREEN table (raw numbers)
| gate | pre-lane (main 3e15613) / RED app | lane |
|---|---|---|
| w1c (a) median per-poll peak_callback_ms <= 4.0, every load | main 6.55 / 6.58 / 6.55 (RED) | c2 (P1 only, INFO) 3.84 / 3.77 / 3.71; final 0.84-1.66 over 6 loads |
| w1c (b) median fps >= 118.5 | main 105.3 [104.1, 105.3, 106.0] (RED; load 7) | c2 117.0 (INFO, VU12 Q4); final pooled 6 loads 119.7 [120.0, 119.92, 120.0, 119.02, 119.56, 118.0] |
| w1c (c) uploads >= 595, late 0 / (d) hold 0 | 604-608, late 0 / hold absent | 600-608, late 0, hold 0, cap 2 (3 once), max 2 uploads/frame |
| w1d (VU9, INFO) 8 x 1080p | 88.7-88.8 fps, late 0 | c2 100.6-101.1 fps, skipped 2 / 17, max 8/frame; final 119.9-120.0 fps, skipped 0, max 5/frame, late 0, hold 0 |
| w2c 4 x 4K (a) INFO, (b)-(e) | interleaved A/B launch medians [84.88, 88.89, 89.41, 89.95, 90.36] | c5 [100.16, 102.64, 103.19, 103.68, 104.22] -> fps4kBlitMin null (97 < 98.36); final 101.6-110.5 (95.8-96.9 at load 10-14); peak upload 0.25-0.37 ms (main 2.0-8.3); late 0, hold 0 |
| w10 identity, 5 formats x 2 frames, <= 1/255 | the reference (self-check f0 vs f150: 255) | max |diff| 0 on all 10 captures on the blit, surface-client and malloc paths (each witnessed by the "upload=" log line), and 20/20 again on the final app |
| u2 gl_thread_qos == 33 | main absent; c1 21 | c3 33, and 33 after u4a's cycles (VU11) |
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
- src/render/GLThreadQos.h (NEW), src/render/Renderer.{h,cpp}: videoUploadBudget_, frame-top video_max_uploads_per_frame / gl_thread_qos, raiseRenderThreadQos, glContextGen_, scanVideoIdle.
- src/ui/OutputWindow.cpp: raiseRenderThreadQos() in Presenter::newOpenGLContextCreated (VU11; g4cpu does not touch this file).
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
- (medium) w7 (b) tail: the lane's message-thread trigger round trip has a heavier tail under load (2 FAILs over 30 ms in about 22 launches vs 0 in 15). A candidate cause is the QoS-raised render thread competing with the main thread (plan risk 7), or simply 20-40 % more frames rendered per second. Mitigation: rule on it; an attribution run over more launches under a controlled load.
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
DONE_WITH_CONCERNS: every plan item and ruling is implemented and gated. The u3 look-ahead fix needs Harmony's ruling (it changes a constant the plan listed as must-not-change, in order to keep main's reverse-play behaviour). w7 (b)'s heavier tail on the lane app is reported, not fixed.

### NEXT ACTION
Harmony: rule on 90b2ca9 and on w7 (b); number the pitfall (NN -> 59 if asyncload stays 58); merge.

### NOTES FOR .harmony/notebook.md (Harmony appends)
- 2026-09-29 | A video frame's shown slot is held (VideoRing::Retire): the writer look-ahead is kSlots - 1, and the reseek distance / ahead-drop line must be measured from it (kWriterLookAhead). Measured from kSlots, reverse play lost 25 % of its frames, and kSlots 4 did not help | discovered: src/media/VideoPlayer.cpp decodeLoop + uploadToTexture pick.
- 2026-09-29 | TestServer listens on "localhost" (may be ::1): probe clients must use http://localhost:8080, never 127.0.0.1 | discovered: src/test/TestServer.cpp:84.
- 2026-09-29 | `auto* obj = juce::JSON::parse(x).getDynamicObject()` dangles (the var dies at the end of the statement): keep the parsed var alive | discovered: src/test/TestServer.cpp handleGlContextCycle (caught live: detached_ms read as 0).
- 2026-09-29 | A context cycle with an immediate re-attach does not starve a PLAYING video: the ~60 ms shader recompile refills every ring. The P4a bug needs a speed-0 / paused clip or a long detach to show | discovered: .harmony/probe-vupload.py u4a.
- 2026-09-29 | IOSurface blit into a sampler2D is per-pixel identical to the RGBA client upload (0 / 255 on yuv420p, yuv422p10le, ProRes 4444 alpha, HAP Alpha, HEVC 10-bit); the binding survives IOSurfaceSetPurgeable Empty -> NonVolatile on the M1 Pro | discovered: tests/test_video_player_gl.cpp, probe-video w10.

INBOX-RECHECK: none
