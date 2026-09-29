# Reviewer Verdict — seqvram-gates-r1
STATUS: DONE
VERDICT: FAIL
COMMIT: d2b441119720423d8d8a40186dc46f0a0050bce9 (lane/seqvram, base 5b78d43)

FILES: src/media/SeqVram.h (new), src/media/ImageSequence.h/.cpp, src/render/Renderer.h/.cpp,
src/api/ApiServer.cpp, src/test/TestServer.cpp, tests/test_seq_vram.cpp, tests/CMakeLists.txt,
.harmony/probe-seq-vram.{sh,py,json}, docs/claude/rendering.md, docs/claude/pitfalls.md, CLAUDE.md,
.harmony/.reports/s-rta-0928b/{seqvram.md,seqvram-scratch-rows.diff}

ISSUES:
MUST-1 (R-A, disk-verified): a real render-thread stall + visible frame loss ships in this commit.
  scanSequenceVram() (src/render/Renderer.cpp, added function after drainRetiredMedia) treats a sequence
  "idle" whenever `lastDrawnSerial()+1 < seqFrameSerial_`. During a crossfade start, CompositorEngine's
  pending-hold branch (src/render/CompositorEngine.cpp:1108 `else` branch containing the only call to
  `renderLayerStages`, gated behind `if(pending){...}else{ ... renderLayerStages(...) }` at :1108/:1122)
  is skipped while the incoming clip is pending -- confirmed independently by reading the code: the
  ONLY call site of applyTransition (which is what drives the outgoing sequence's getCurrentTexture via
  getClipTexture) is inside renderLayerStages, itself only reached from the `else` (non-pending) branch.
  So for 1-3 frames the outgoing sequence's lastDrawnSerial_ does not advance, scanSequenceVram then
  scores it idle and trims it to 2 slots once the incoming's first uploads push the total over budget.
  Builder-measured cost: seq_textures 129->5-6 in ~0.05-0.07s post-trigger, a 22-24ms render callback
  (3/7 final-app runs, 4/5 on a detail rig), then 4-11 late frames as the outgoing re-decodes mid-fade.
  This is a genuine, disk-confirmed regression in code that is already committed to the lane; it is
  disclosed honestly in the report (STATUS: PARTIAL, "needs a Harmony ruling before merge") but is not
  fixed and not gated by any adopted probe row.
MUST-2 (R-B, disk-verified): trimToMinimum's per-idle-sequence glDeleteTextures loop is uncapped
  (src/media/ImageSequence.cpp trimToMinimum, called from scanSequenceVram under imageSeqMutex_).
  Builder's scratch row v7c measured 5/5 runs at 21.9-22.4ms trimming one large idle sequence in a
  single frame (seq_evictions +127 in one poll). The adopted v7b row does not catch this because its
  three idle sequences are each smaller (fewer textures per trim), so the same code path costs ~10ms
  there and looks safe. Not fixed; candidate fix (cap deletions/frame) is named but not implemented.
SHOULD-1 (spec/gate blind spot, not the builder's invention): the H8 adoption's v8_crossfade_two_long
  (b) asserts `peak_frame_time_ms <= 16.7`. Per CLAUDE.md's own documented distinction, the frame-top
  scan/trim runs before `renderStart` and is only visible in `peak_callback_ms`, not
  `peak_frame_time_ms` -- so the adopted metric structurally cannot see the R-A stall it was meant to
  gate. The builder correctly diagnosed this (UNKNOWNS section) and even added the right metric
  (peak_callback_ms) as an unrequested extra assertion on v7b (a', D5) -- but did not (and per the rig's
  "no redesign" instruction, could not) retrofit it onto v8, where the regression actually lives. A
  future round should add `(b') peak_callback_ms <= 16.7` to v8 once R-A is fixed, or this gate will
  stay green over a real stutter.

VERIFIED (disk, this review, not recall):
- Read the full diff (5b78d43..d2b4411) for every file in FILES; SeqVram.h, ImageSequence.h/.cpp,
  Renderer.h/.cpp match plan-seqvram.md section 4 and Harmony adoption H1-H17 point-for-point (H1 Full/
  uploadDeferred, H2 releaseAll+clear, H3 running total, H4 lastShown independent of cur, H5
  allowanceFrames floor before frame size known, H6 Transport in/out range + setRange, H10 floors-win
  invariant) -- confirmed by reading the actual code, not the report's prose.
- Built test_seq_vram is up to date (object/binary post-dates test_seq_vram.cpp) and I ran it myself:
  `./build-lane/tests/test_seq_vram` -> "All tests passed (4889249 assertions in 17 test cases)".
- ctest -N on build-lane lists exactly 865 registered tests, matching the claimed 865/865.
- Manually hand-traced SeqVram::distances() for PingPong forward n=6 cur=4 and confirmed it produces
  {6,5,4,3,0,1}, matching test (2) and the builder's D1 deviation note (the plan body's own dist[3]=2
  contradicted its stated rule; the code and test are correct, the plan prose had the bug).
  Independently hand-verified test (12)'s H6 ranged-trajectory numbers against the adopted example.
- probe-seq-vram.json thresholds (budgetMB 1024, frameMB1080 7.91, frameMB4k 31.64, minWindowFrames 8,
  shownTol 3, lateMaxAfterJump 12, boxTol 3.0, peakMaxMs 16.7) are byte-for-byte the plan's constants --
  no silent re-thresholding found in v1/v2/v4/v6/v8/v9/v7/v7b/v3/v10.
  v4_retrigger_hit, v8_crossfade_two_long, v9_three_decoding, v10_floors_win, v7 (c') exactly match the
  H7-H11 adopted PASS conditions, letter for letter.
- probe-image-load.py and probe-crossfade.py are untouched in this diff (H16 respected).
- git diff main..lane -- docs CLAUDE.md is additive only (one pitfall paragraph, one index line, one
  rendering.md sentence replaced 1:1); CLAUDE.md wc -c = 24,665 (<= 25,000).
- No new mutex anywhere in the diff (grep for std::mutex/lock_guard: only the pre-existing
  imageSeqMutex_ and the pre-existing videoPlayerMutex_ appear). No file under src/model or
  src/render/CompositorEngine.* touched (fence respected). No .venv symlink, no stray files in the
  commit (git show --name-only on d2b4411 lists only the 2 report files).
- Independently confirmed via source read (not just trusting the report) that CompositorEngine's
  pending-hold branch (~:1093-1116) is the ONLY branch active while an incoming image/sequence is
  pending, and that it never reaches renderLayerStages -> applyTransition -> the outgoing sequence's
  getCurrentTexture -- i.e., R-A's mechanism is real, not a builder misreading.

NOT RE-RUN LIVE (time/rig cost; taken as INFERRED from the report + independent code trace above):
the live probe-seq-vram 60/0 double-green run, probe-image-load 37/0, probe-crossfade 35/0,
render-state 35/0, canvas 15/0, and the full serial ctest 865/865 were not independently re-executed
against the running app (would require the live-lock rig); the pure ctest count and the pure
test_seq_vram binary were independently executed and matched the report's claims.

SUMMARY: 17 files, 2 MUST findings (R-A, R-B: disk-verified live regressions that ship with this
commit, undisclosed by any adopted gate, explicitly unresolved by the builder pending a Harmony
ruling), 1 SHOULD (the H8(b) gate's peak_frame_time_ms blind spot). Everything else reviewed --
the pure SeqVram policy, the Slots table, the ImageSequence/Renderer wiring, the 17 ctest cases, the
10 probe rows and their thresholds, and the additive docs -- matches the plan and Harmony adoption
exactly and is independently verified correct. Verdict is FAIL only because of the two MUST items:
this is exactly the "defect that ships" case the round-1 gate exists to catch before merge, not a
reason to discard the otherwise-excellent implementation.
METADATA: reviewer=reviewer-agent, builder_packet=seqvram, date=2026-09-28
