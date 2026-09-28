# renderleft (s-rta-0928) -- lane report

STATUS: DONE_WITH_CONCERNS
RESULT: R1 (R1.0-R1.5), R2, R3, R5 shipped. R4 needed no code: no target reached the 70 % bar, so it is filed with the
E0-E3 table. 12 commits on lane/renderleft-0928, rebased onto main 15c5f8d. The rebased head builds, and a serial
ctest passes 846/846. Every probe gate is GREEN on the rebased app (image-load x2, capture, render-state, canvas) or on
the pre-rebase head app r3 (crossfade, effects-parity, fitmode, outputs, deck-clock, Tier-1), at main's counts plus the
new rows. The concerns are listed under UNKNOWNS: the per-commit builds were verified before the rebase, not after it;
two i5 readings went over the bar in an old row order and are unexplained; the R4 excess is left in place.
INBOX-RECHECK: none

## FACTS (every run dir: SP = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/renderleft)
- Apps. RED = SP/base-app (main build 233eae7 = 6db8d67's code). Per-commit copies are in SP/apps/{r10,r2,r5,r11,r12,r13,r14,r15,r3}, all on the old base 6db8d67. SP/apps/final is the rebased head (code == c129b8d). SP/apps/{r12-nogate,head-nogate,r12-nopause} are the teeth builds, each followed by a restore with the sha printed (SP/teeth-*.log).
- R1.0 split, measured on the R1.0 app (SP/runs/split-r10.log, load avg 5.8-6.0; every line has the form `[Image] loadKeyImage|legacy <path> (WxH) decode= convert= upload= ms`):

  | size | decode | convert (old loop) | upload |
  |---|---|---|---|
  | 756x878 | 9.7-10.2 ms | 1.7 | 0.6-0.7 |
  | 1920x1080 | 28.0-28.3 ms | 5.5-5.7 | 1.9-2.0 |
  | 3840x2160 | 110.2-113.6 ms | 23.0-23.8 | 1.6-3.4 |

  The legacy line appears on EVERY trigger, with the same decode (F2 confirmed). peak_callback_ms per trigger was 25-28 ms, 69-71 ms and 261-275 ms for the three sizes; peak_frame_time_ms was 13-14, 37 and 141-146 ms.
  R1-h ruling: the 4K upload median is 3.1 ms, which is <= 8 ms, so R1.2 uploads with one glTexImage2D per image (U1). U2 was not built.
- probe-image-load on main (SP/runs/main-imgload.log): `PY 11 PASS / 15 FAIL`.
  - i1 1080 38.77 ms, i1 4k 138.31 ms, i2 1080 36.30 ms, i2 4k 143.48 ms, i6 46.20 ms.
  - peak_callback_ms, image_*: absent (main-imgload-2.log for the reworked rows).
  - i3 / i3s PASS (a synchronous decode, the guard).
  - i2m p0 0.50 (RED).
- probe-image-load on the R1.0 app (SP/runs/r10-imgload.log):
  - i1 4k: frame 135.40 ms, callback 260.99 ms.
  - i5: 34.40 ms by value (F2).
- probe-image-load on the final app, `PROBE-IMAGE-LOAD GREEN` twice (SP/runs/final-imgload-1.log, -2.log, load avg 3.1-4.8):
  - i1: 4.16/3.80 ms (1080), 1.33/1.56 ms (4k). i2: 3.77/3.43 ms (1080), 2.84/3.04 ms (4k).
  - i5 4.10/5.90 ms. i6 4.43/4.29 ms.
  - i7: 6 images resident after 0.34 s; peak callback 7.29/4.45 ms; held 0; textures 1 after the swap.
  - i2m p0 0.021/0.012. i3 / i3s dbox 0.00.
  - i4 held 57/57, i4m held 58/57, skipped 0.
- Demand path on the R1.4 app, which has no prefetch (SP/runs/r14-imgload.log):
  - i1 4k: frames held 15, callback 7.19 ms. i1 1080: held 5, callback 3.32 ms. i2 4k: held 16.
  - i4 held 66, i4m held 63.
  - i7 (1)/(4) FAIL, as expected before R1.5.
- Teeth, live:
  - head-nogate (the capture gate commented out): i3 FAIL dbox 76.73, i3s FAIL dbox 55.70 (SP/batch3.log "teeth no-gate").
  - r12-nopause (the C1 crossfade pause removed): i2m FAIL p0 0.720 (SP/batch3.log "teeth no-pause").
  - Restore sha printed equal for each (SP/teeth-builds-r12.log, teeth-build-headnogate.log).
- probe-capture:
  - main (SP/runs/main-capt.log): cr1 3 of 4 `ok=False 5.02 s`, cr2 320x180 `ok=False 5.01 s`.
  - R2 app (SP/runs/r2-capt.log) and final (final-capt.log): cr1 4/4 in <= 0.14 s, cr2 both, array_equal to the solo capture.
- R3 calibration on main, CAPT_CALIBRATE=1 (load 5.13): 1080p png median 81.4 ms, round trip 97.4 ms; 4K png 303.6 ms, round trip 334.7 ms.
  - Head: png 13.9 ms, round trip 28.6 ms. Final: 9.9 / 26.2 ms. 4K: png 38.1-38.7 ms, round trip 84.8-89.0 ms. `enc=fast` on every line.
  - Bench, indicative only (load ~7; `test_png_fast "[.bench]" -s`):

    | frame | JUCE writer | level-1 writer |
    |---|---|---|
    | 1080p smooth | 69 ms, 11 KB | 23 ms, 746 KB |
    | 1080p noise | 556 ms | 121 ms |
    | 4K smooth | 265 ms | 61 ms |

- R5:
  - main r1_cells FAIL (field absent). R5 app r1_cells +36 cells in 0.3 s, +157 in 1.3 s (bound 199), rings +1.
  - output_probe PNG sha256 is equal main vs head for 4 files (probe_1920x1080, _of_720p, _after_B, _portrait).
- Byte identity (abpix; SP/abpix.py, SP/abcmp.py; logs SP/runs/abpix-*.log):
  - main S1 hashes = the known 1eb43743...339e / eb1f5191...b8156 / 650ef667...3ac7.
  - r11 and r15 vs main: every scene array_equal AND sha256 equal (`ABPIX SAME`).
  - r3 and final vs main: arrays equal on all scenes; render_frame sha differs by design (S1-S4); the snapshot S5 sha is equal.
- Probe gates (main -> lane):
  - render-state: 32 PASS + r1_cells FAIL -> 35 PASS / 0.
  - canvas: 15/0 -> 15/0.
  - crossfade: 35/0 -> 35/0.
  - effects-parity: 46/0 -> 46/0.
  - fitmode: 10/0 -> 10/0.
  - outputs (window-safe rows): 11/0 -> 11/0.
  - deck-clock: 10/0 -> 10/0.
  - Logs: SP/runs/main-*.log, head-*.log, final-*.log.
- Tier-1 on head r3 (exactly the 5 files, test mode): `1 failed, 12 passed in 923.57s`. The source_param_failures.txt sha256 is `0524d3351d425df59647adaf49297deae0c5f43be7ed0f6bb89fd006a08188e7`, identical to main's (tier1-residual-diag.md). SP/runs/tier1-head/.
- ctest:
  - 784 on R1.0 (unchanged); +3 R1.1 = 787; +18 R1.2 = 805; +1 R1.5 = 806; +4 R3 = 810 (all on base 6db8d67).
  - Rebased head, serial -j1: `100% tests passed, 0 tests failed out of 846` (SP/ctest-final.log).
- ctest teeth (SP/teeth-r11.log, teeth-r12.log, teeth-r15.log, teeth-r3.log):
  - R1.1: dropping unpremultiply FAILs the straight case and not the premultiplied case; swapping R/B FAILs both.
  - R1.2: onResult accepting an unknown path FAILs (4); requesting on every pending lookup FAILs (1); a draining destructor FAILs decode (5).
  - R1.5: an unbounded P FAILs (9).
  - R3: dropping the premultiply round trip FAILs (1).
- B4 quit time, osascript quit -> process gone:
  - main: 2.151 / 2.089 / 2.197 s.
  - lane with a cold noisy 4K prefetch in flight (images_pending 1): 2.277 / 2.182 / 2.177 s. That adds <= 0.13 s against the 0.5 s bar. A running decode cannot be interrupted; the ~110 ms 4K decode is the worst case.
- R4 (SP/batch3.log T2VAR, head app r3, 4K, the canvas resized before each run):
  - E0 excess first-use 1.47 / 2.26 / 2.24 ms, first-fade 2.35 / 2.17 / 1.89.
  - E1 (transition target written first): 1.45-(-2.31) / 1.99-5.47, noisy.
  - E2 (router FBO created first): 2.09-2.28, no reduction.
  - E3 (effect pool + transition written first): 0.33 / 0.79 / 1.03 and 0.52 / 0.81 / 1.39. That removes 78 / 65 / 54 % (first use) and 78 / 63 / 26 % (first fade).
  - None meets ">= 70 % in 3/3", so nothing is VERIFIED and there is no code (the plan's rule). Run 1 was at load 12.
- Rebased twice onto a moving main (tempo, tier1, restore merges). Pitfall numbers are taken at commit time: this lane's are 52 (one capture at a time) and 53 (images decode off the GL thread); 48-51 belong to the other lanes. CLAUDE.md is 24,482 bytes.

## METHOD
Every item followed the plan, with the adoption rulings applied, in the plan's commit order. The probes were written
first (R1.0) and run RED on main's app (base-app) and on the R1.0 app. Each later commit was built, ctest'd and teeth'd
at creation. The live gates ran on copied apps under the lock helper. Byte identity is checked by decoding arrays
(abpix S1-S5), plus sha256 until R3. I looked at sample frames: abpix S2 (the alpha overlay over the 2x2 grid) and an
i2m mid-dissolve frame at p 0.23, the warm orange mixing into the gradient. Both look as intended.

## CONFIDENCE + VERIFY
High for R2, R3, R5 and the image paths' pixels: live RED -> GREEN, byte identity, teeth. High for the no-GL-thread
decode (a code read: no decode call is left on the GL thread; `grep -n "loadFrom" src/render src/media` finds only
ImageDecode.h, the sequence dims and thumbnails). Medium for the timing of the hold: at 120 Hz the counters give 5
frames at 1080p, 15-16 at 4K and 57 for an 8K noisy image. Verify: `bash .harmony/probe-image-load.sh` and
`bash .harmony/probe-capture.sh` (lock held, IMGLOAD_APP / CAPT_APP set).

## UNKNOWNS / NOT DONE
- **Per-commit builds after the rebase.** Each commit was built and ctest'd before the rebase (base 6db8d67). After the rebases only the head was rebuilt: 846/846 serial. The per-commit trees were checked for CMake integrity (tests/CMakeLists.txt, SP/chk.sh: no deletions, test_tempo_start intact, balanced parens delta) and pitfall numbering. They were not rebuilt one by one.
- **Unexplained i5 readings.** Head runs 2 and 3, with the OLD row order, where i5 ran after the 8K/filler rows on the pre-restore base: i5 max callback 16.85 and 20.73 ms (frame max 9.9 / 10.9 ms). They did not reproduce:
  - the final runs, with i5 before the filler rows: 4.10 / 5.90 ms;
  - an eviction diagnostic (SP/diag_evict.py, 2 x 2 runs): after releasing 1 GiB of textures, callbacks were 3.1-4.9 ms, the same as without.
  - The eviction hypothesis is refuted; the cause is unknown. A candidate, not checked: the message-thread thumbnail decodes of 8K cells on the pre-restore base.
- **R4.** The ~2 ms excess at 4K stays. E3 suggests the effect pool's first write explains 26-78 % of it; unverified.
- **Handoff and APP-INVENTORY left to Harmony.** .harmony/HANDOFF.md items 3 / ledger 3 are not edited (a shared file, merge-conflict prone). The numbers are here for Harmony. APP-INVENTORY is not updated.

## NUANCE
- R1.5's prefetch warms every composition image. On the head, a composition clip's first trigger therefore normally shows at once (i1/i2 held 0), and the hold path is reached only by a clip beyond the 1 GiB budget or a sequence. That is why the pending rows load a hidden 1 GiB filler layer.
- The fast PNG writer (filter 0, level 1) makes SMOOTH captures much bigger: 1080p smooth 746 KB vs 11 KB. Probe out dirs grow. Noise captures are about the same size.
- The legacy single image keeps its old semantics on a context loss: it is not re-loaded.

## HANDOFF-NEEDS
Harmony: merge lane/renderleft-0928 (a clean fast-forward on 15c5f8d at report time). Append the notebook notes below.
Put the Boris checks on the page. Close HANDOFF item 3 with the numbers above.

### SUMMARY
- R1: the image decode is off the GL thread on every path:
  - clip images: ImageDecode + ImageTexCache, the hold, the capture gate, the C1 crossfade pause;
  - the legacy single image: lazy, and the slideshow prefetches;
  - image sequences: look-ahead;
  - composition prefetch and release.
- R2: one capture at a time.
- R3: a fast PNG for render_frame.
- R5: frame_ring_cells and output_probe via PixelConvert.
- R4: filed.

### FILES CHANGED
- Code: src/render/{ImageDecode.h (new), ImageTexCache.h (new), CompositorEngine.h/.cpp, Renderer.h/.cpp, TextureManager.h/.cpp, PixelConvert.h, PngWrite.h}, src/media/ImageSequence.h/.cpp, src/core/CompositionLoad.h (imagePaths), src/MainComponent.cpp (+1 postImageSet, +2 slideshow prefetch), src/api/ApiServer.cpp and src/test/TestServer.cpp (state fields, render_frame complete/fast, output_probe).
- Tests: tests/{test_image_tex_cache.cpp, test_image_decode.cpp, test_png_fast.cpp} (new), test_pixel_convert.cpp (+3), test_composition.cpp (+1), CMakeLists (3 blocks).
- Probes: .harmony/probe-{image-load,capture}.{sh,py,json} (new), probe-render-state.{py,json} (r1_cells).
- Docs: CLAUDE.md (index 52, 53), docs/claude/{pitfalls.md (37, 46 amended; 52, 53), rendering.md (Image loading, frame_ring_cells, peak_callback_ms), testing-eyes.md}.

### TESTS
The ctest results and teeth are listed in FACTS. The live probe results, RED -> GREEN, are listed in FACTS.

### ISSUES / DEVIATIONS
- D1 Pitfall numbers are 52/53, not the plan's 48/49; other lanes took 48-51.
- D2 B1: `legacyPendingThisFrame_` is declared in R1.2, false until R1.3.
- D3 The plan's teeth "decoder destructor without removeAllJobs" cannot bite: juce::ThreadPool's own destructor calls removeAllJobs(true, 5000) (juce_ThreadPool.cpp:130-134). I used a draining destructor instead, which FAILs case 5.
- D4 The pending rows (i2m, i3, i3s, i4, i4m) were reworked after R1.5: a 1 GiB hidden filler layer, an 8K cold image, a wait for each trigger to land. i2m now runs on a 1080p canvas, renamed i2m_fade_start. The row order puts the perf rows first. The main RED and teeth runs were redone on the reworked rows.
- D5 C1's pause applies to IMAGE clips only. A sequence's first frame is not paused: its pending case is "nothing shown yet", and it counts for the gate (C3).
- D6 C4: i4m checks counters only. A held frame never answers render_frame (the gate), so the picture is not capturable by design.
- D7 test_image_tex_cache adds (7b) budget spill (B3) and (11b) Failed retried after a swap (R-7): 13 cases, not 11.
- D8 Additional finding: `ImageSequence::getCurrentTexture` indexed the emptied `textures_` after a context loss (UB). Fixed in R1.4 (ensureFrameState).

### FOUND, NOT FIXED
- F16: video decodes on the GL thread.
- ImageSequence keeps every frame's texture (a 300-frame 1080p sequence is ~2.5 GB).
- existsAsFile() per image clip per frame on the GL thread.
- A legacy load is posted on every deck-mode image trigger (O(1) now, but pointless).
- ImageSequence::open / openMediaForDeck decode frame 0 on the message thread.
- The R4 4K first-use/first-fade excess of ~2 ms, with the E0-E3 table.
- The i5 over-bar readings in the old row order (UNKNOWNS).

### RISKS
- The hold is a new visual state. It lasts ~5 frames at 1080p and ~15 at 4K, at 120 Hz, when a clip beyond the prefetch runs demand.
- A dissolve onto such an image starts late by the decode time (by design, C1).
- Composition loads now decode every image (1 GiB budget), using memory and CPU at load.

### METRICS
12 commits; 34 files, +3363 / -272 against the merge base.

### PACKET QUALITY
- Clarity: CLEAR. The plan plus the adoption section were precise.
- Missing context: the prefetch (R1.5) makes the R1.2 pending rows unreachable on the head. The plan did not foresee it.
- Unused context: none.
- Self-brief files: plan-renderleft.md (useful), the renderperf-evidence scripts (useful: capt.py, t2warm.py), notebook headings (useful: the CMake-append rule, Connection: close).
- Self-assembly: no DEPARTMENT field.

### KNOWLEDGE CONTEXT
- Tools used: grep.
- Impact authority: grep (not authoritative; took a conservative posture).
- God nodes in scope: n/a.
- Risk level: ELEVATED (Renderer/CompositorEngine core).
- Dependencies discovered: ClipCell thumbnails, since fixed by the restore lane.
- Queries made: 0.

### NOTEBOOK NOTES (for Harmony to append)
- make on macOS uses 1-second mtimes. After restoring a mutated file, `sleep 1; touch` it before rebuilding, or the object keeps the mutation (the R1.5 teeth restore needed it).
- juce::ThreadPool's destructor already removes all jobs. Teeth for "queued jobs are dropped" must mutate toward a draining destructor.
- ctest runs Catch2 cases as parallel processes. Temp dirs named by currentTimeMillis collide; use juce::Uuid.
- From R1.5, a composition load prefetches its images. A probe that must see a pending image needs >= 1 GiB resident before the subject (the hidden filler layer in probe-image-load).
- A rebase conflict in tests/CMakeLists.txt: resolve as HEAD's file plus the applied commit's added lines (SP/fixcmake.py). A marker-level "ours + theirs" resolver split another lane's block and broke the configure.

### BORIS CHECKS
1. A never-seen image clip beyond the prefetch now holds that layer's last picture for ~5 frames (1080p) or ~15 frames (4K) at 120 Hz, instead of the whole output hitching 37 ms / 140 ms. Does that feel right?
2. A dissolve onto such an image starts when the image lands (up to ~0.1 s late at 4K) and then runs its full length.
3. Loading a composition now decodes all its images in the background (up to 1 GiB); switching compositions frees the old ones.
4. The slideshow advances with no stutter (the next image is prefetched). This is a manual check; I did not drive the UI.

### STATUS
DONE_WITH_CONCERNS

### NEXT ACTION
Harmony: an independent review of the lane, then the behavioral gate, then the merge.

## Fix round (renderleft-fix, 2026-09-28 14:00-14:25)
Commits on lane/renderleft-0928 after 5f1536f: 15f1ff8 (pitfalls 48-50), abefe7e (C1 sequences + row i2ms), 7f56369 (row i3n),
a3691e1 (imagePaths dedup), f6b7e0a (i3n own-snapshot exclusion + bound), then the report commit. Full detail:
.harmony/.reports/s-rta-0928/renderleft-fix.md. FSP = scratchpad/renderleft-fix.
- MUST 1 (pitfalls 48/49/50 lost in the rebase) -- VERIFIED and FIXED. On 5f1536f, pitfalls.md went from 47 to 51. The
  three entries are restored verbatim from 15c5f8d (lines 105-110). `git diff 15c5f8d -- docs/claude/pitfalls.md` now shows
  only this lane's lines: the 37 and 46 amendments, plus the new 52 and 53. Content check (FSP/patchcmp.sh): for every file, the lane's patch
  before the first rebase (6db8d67..79bafe7) was compared with its patch now (15c5f8d..tree). Result: 29 files SAME; the four docs (CLAUDE.md, pitfalls.md,
  rendering.md, testing-eyes.md) the same apart from the renumbering 48/49 -> 52/53; probe-image-load.py differs only in the intended restore-lane comment.
- MUST 2 (C1 skipped image sequences) -- VERIFIED and FIXED. `incomingImagePending` returned false for a non-Image
  clip. New `ImageSequence::firstFramePending() const` = getCurrentTexture's *pending rule, read only; Renderer provides it
  (`setSequencePendingProvider`); incomingImagePending asks it for ImageSequence. New row i2ms_seq_fade_start:
  - RED on the pre-fix lane app (5f1536f code): `FAIL  i2ms_seq_fade_start: the dissolve starts when the image lands (first answered frame p 0.578 <= 0.08)` (run 2: p 0.553).
  - RED on main: p 0.262 / 0.261 (the in-frame decode's dt).
  - GREEN x2: `PASS  i2ms_seq_fade_start: the dissolve starts when the image lands (first answered frame p 0.000 <= 0.08)`.
  - I looked at the mid-dissolve frame f2 (p 0.20-0.24): the warm orange blends into the gradient.
- MUST 3 (C2 snapshot row missing) -- VERIFIED (no such row) and ADDED: i3n_snapshot_while_pending.
  - RED on main: `FAIL ... answers (ok True) in 1422 ms <= 500` + `FAIL ... the held picture ... (dbox(snap, capA) 76.72 <= 3.0)`.
  - Teeth (takeSnapshot with completeFrame=true; restore sha equal): 887 ms, snapshot = the cold picture -> 2 FAIL.
  - GREEN (final x2, and the pre-fix lane app, which already did C2 right): 72-84 ms, dbox(snap, held) 0.00; render_frame right after waits 665-739 ms and shows the new picture.
- SHOULD 4 (imagePaths O(n^2)) -- VERIFIED, FIXED (unordered_set seen-set; order unchanged; compload::imagePaths passes).
- Gates on the final app (FSP/apps/final = HEAD code):
  - probe-image-load `PY 37 PASS / 0 FAIL` + `PROBE-IMAGE-LOAD GREEN` x2 (load 4.0-7.5).
  - probe-crossfade `PY 35 PASS / 0 FAIL` GREEN (main 35/0).
  - ctest serial -j1 `100% tests passed, 0 tests failed out of 846`.
