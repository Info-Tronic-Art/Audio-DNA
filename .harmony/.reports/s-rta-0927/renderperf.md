# s-rta-0927 lane renderperf (plan-renderperf C0-C3)

STATUS: DONE_WITH_CONCERNS
RESULT: Both render fixes are in and every gate passed, with one concern. (1) Frame-ring cells are now created on their first write. The one-time frame at a layer's first Screen Split / Frame Stutter use and at its first crossfade dropped from 32.7-47.7 ms to 9.0-14.2 ms. With both images uploaded beforehand it is 1.7-5.4 ms (dc7adf9 in the same test: 26.3-28.7 ms). (2) A capture now costs the GL thread only glReadPixels: 2.3-5.0 ms at 1080p (was 94-110 ms) and 7.6 ms at 4K (was ~372 ms). The PNG bytes are the same as before. Four commits: 7c2fbc4, aa72396, 147a937, 1a24467. The concern: the plan's relative GREEN check (first-fade peak within 2x of the second fade's) fails on cold runs. The leftover 9-14 ms is the first upload of the clip's image (`loadKeyImage`), not the ring. The absolute 16.7 ms bar holds on all 4 runs.
FACTS: `src/render/CompositorEngine.cpp` (getOrCreateRingBuffer, pushFrameToRing, getFrameFromRing); `src/render/FrameRing.h`; `tests/test_frame_ring.cpp`; `src/render/PixelConvert.h`; `tests/test_pixel_convert.cpp`; `src/render/Renderer.cpp` (captureFrame, processPendingCapture); `.harmony/probe-render-state.py` (r1_counts); evidence `.harmony/.reports/s-rta-0927/renderperf-evidence/` (live/*, ctest/*, tier1/*, TEETH-*.txt, scripts/*)
METHOD: I built the worktree (`build-lane`, Release, TEST_SERVER=ON) and kept a signed copy of the app after each commit's build (C0, C1, C2, C3). Each new test or row was run RED first: on the dc7adf9 main app (hitch, r1_counts, capture round trip, PNG hashes) or on mutated COPIES (ctest teeth). Every live run held /tmp/audiodna-live.lock, and perf runs waited until no compiler was running (load average printed). Each app was launched with `open -g` and quit via osascript. A Quartz sampler watched the window list during every batch. I committed from the index, and a replay of the patch scripts on dc7adf9 reproduces the working tree byte for byte.
CONFIDENCE: high for the ring fix and byte identity: ctest invariant, live sha256 equal at 3 sizes, 7 probes GREEN, and Tier-1 fails the same 8 tests as the base app. Medium on how the leftover hitch splits (image upload vs 4K canvas allocations): inferred from the warm/cold contrast, not profiled. VERIFY: `ctest --test-dir build-lane -j1 -R "readIndex|every alpha|forceOpaque|odd sizes|PNG bytes"`; `RSTATE_APP=<app> bash .harmony/probe-render-state.sh /tmp r1_counts` (lock held).
UNKNOWNS: C4 is deferred (the `frame_ring_cells` field and output_probe using PixelConvert). No live witness shows cells growing one at a time; the peak rows and the source diff carry that claim. What the 3.7 ms left at 4K warm is made of was not measured.
NUANCE: Port 7070's render_frame ignores width/height (only 8080 test mode locks the canvas), so the 720p and 4K captures set the size through the composition's outputWidth/outputHeight. The HTTP round trip of a capture is unchanged (~95-116 ms at 1080p, ~350 ms at 4K) because the PNG encode still runs before the response. Only the GL thread got cheaper.
HANDOFF-NEEDS: Harmony: merge lane/renderperf-0927 (4 commits + report commit; base dc7adf9, main is at 0766f06, which is docs only, no overlap), then run the behavioral gate; the Reviewer should read pushFrameToRing's ~8-line diff and the `assign` in getOrCreateRingBuffer. Decide on found_not_fixed #1 (image upload hitch).

INBOX-RECHECK: none

## SUMMARY
The plan (`.harmony/.reports/s-rta-0927/plan-renderperf.md`) was followed item by item. C0: timing on the capture log line. C1: lazy per-cell ring allocation plus the FrameRing invariant test and the tightened r1_counts bar. C2: byte-identical row conversion plus its ctest. C3: the GL thread keeps only the read. Deviations and drift are listed under DRIFT.

## FILES CHANGED (by commit)
- **7c2fbc4 C0**: `src/render/Renderer.cpp`. processPendingCapture times read / convert / png and appends ` read=R convert=C png=P ms` after `(WxH)` on `[Eyes] Captured frame:`. The prefix is verbatim.
- **aa72396 C1**:
  - `src/render/CompositorEngine.h`: `FrameRingBuffer::allocatedCells`, `frameRingCellCount_` + `getFrameRingCellCount()`, and a rewritten ring comment.
  - `src/render/CompositorEngine.cpp`: getOrCreateRingBuffer creates no GL object and uses `assign` (not `resize`). pushFrameToRing creates cell `idx` when its texture is 0, with no clear. getFrameFromRing uses `FrameRing::readIndex`. releaseGL and rescaleHistory account the cells.
  - NEW `src/render/FrameRing.h`, NEW `tests/test_frame_ring.cpp`, `tests/CMakeLists.txt` (block).
  - `.harmony/probe-render-state.py` r1_counts: bars on first use AND first fade, second-fade peak printed, `wait_no_compiler`, load average printed, docstring calibration. `.harmony/probe-render-state.json`: `peakMaxMs` 50.0 -> 16.7 plus `_why`.
  - Docs: `docs/claude/pitfalls.md` (Pitfall 20), `docs/claude/rendering.md` (ring paragraph + "no hitch"), `.harmony/HANDOFF.md` item 2.
- **147a937 C2**: NEW `src/render/PixelConvert.h`, NEW `tests/test_pixel_convert.cpp`, `tests/CMakeLists.txt` (block), `src/render/Renderer.cpp` (loop -> `PixelConvert::rgbaBottomUpToARGB`, still on the GL thread).
- **1a24467 C3**: `src/render/Renderer.h` (`capturePixels_`, `captureReadW_/H_`, `captureReadMs_`; `captureOutputPath_` removed because nothing read it any more) and `src/render/Renderer.cpp`:
  - processPendingCapture: read -> set_value(true).
  - captureFrame: move the pixels out under captureMutex_, then convert, then write the PNG inside a stream scope, then log.
  - `.harmony/HANDOFF.md` item 4.
- **report commit**: this file, `renderperf-evidence/`, `.harmony/notebook.md` (one entry).

## RED -> GREEN (raw lines verbatim; all perf rows quiet: `compilers=[0/0]`, load printed)

### Hitch table (t2.py: /api/state peak_frame_time_ms polled ~15 ms; 6 cols A/B each [Screen Split 0.15 0.15 0.25 0], T = 1 s; fresh app per run)

| run | app | canvas | load | first use | FIRST fade | 2nd fade | steady median / max | rings |
|---|---|---|---|---|---|---|---|---|
| 1 | dc7adf9 | 1080p | 3.29 | 41.65 | 38.65 | 1.84 | 0.73 / 1.28 | 0->1->2 |
| 2 | dc7adf9 | 1080p | 3.37 | 32.72 | 34.37 | 2.23 | 0.73 / 1.54 | 0->1->2 |
| 3 | dc7adf9 | 1080p | 3.39 | 38.03 | 32.74 | 1.61 | 0.71 / 1.46 | 0->1->2 |
| 4 | dc7adf9 | 4K | 3.08 | 47.71 | 35.88 | 1.13 | 0.62 / 1.37 | 0->1->2 |
| 1 | C1 | 1080p | 3.03 | **12.10** | **12.59** | 1.87 | 0.75 / 1.68 | 0->1->2 |
| 2 | C1 | 1080p | 2.96 | **13.34** | **8.98** | 1.82 | 0.71 / 1.54 | 0->1->2 |
| 3 | C1 | 1080p | 2.81 | **9.54** | **10.18** | 1.62 | 0.74 / 1.41 | 0->1->2 |
| 4 | C1 | 4K | 4.18 | **14.21** | **10.93** | 0.99 | 0.67 / 1.04 | 0->1->2 |

- Absolute bar (<= 16.7 ms on first use AND first fade, all 4 runs): RED on dc7adf9 4/4, GREEN on C1 4/4.
- Relative bar (<= 2x the same run's second fade): **not met on the cold runs**. The plan says to look before accepting a 6-16.7 ms first-fade peak.
- temporal_buffers stayed 0, so the leftover is not the spare temporal buffer. The t2warm diagnostic below shows it is the first upload of each image.

Verbatim example lines (`renderperf-evidence/live/red-base/t2-base-1.log`, `live/c1/t2-C1-1.log`):
```
first-use: n=60 max peak 41.65 ms at +0.083 s, frame_rings 0->1, temporal 0->0, median peak 0.66
fade1: n=60 max peak 38.65 ms at +0.054 s, frame_rings 1->2, temporal 0->0, median peak 0.71
first-use: n=59 max peak 12.10 ms at +0.033 s, frame_rings 0->1, temporal 0->0, median peak 0.72
fade1: n=60 max peak 12.59 ms at +0.032 s, frame_rings 1->2, temporal 0->0, median peak 0.74
```

### Warm-media diagnostic (t2warm.py = t2.py plus a second layer that shows A then B before the first use)
| run | app | canvas | load | first use | FIRST fade | 2nd fade |
|---|---|---|---|---|---|---|
| 1 | dc7adf9 | 1080p | 2.97 | 26.33 | 28.70 | 1.56 |
| 2 | dc7adf9 | 1080p | 3.21 | 27.40 | 26.64 | 1.70 |
| 1 | C1 | 1080p | 3.78 | 1.67 | 2.05 | 1.74 |
| 2 | C1 | 1080p | 3.52 | 1.90 | 1.82 | 1.90 |
| 3 | C1 | 1080p | 3.02 | 1.76 | 5.44 (@+0.816 s; steady max 7.83 in the same run) | 2.55 |
| 4 | C1 | 4K | 2.86 | 3.83 | 3.68 | 1.14 |

- The ring alone was 26-29 ms on dc7adf9 and is at the noise floor on C1.
- The cold ~10 ms is `CompositorEngine::loadKeyImage`: PNG decode + per-pixel getPixelColour + glTexImage2D on the GL thread, once per image path (found_not_fixed #1).
- The relative check passes on warm 1080p runs 1-2. Run 3's 5.44 ms peak is 0.8 s after the fade start, so it is not the fade frame. The relative check fails at 4K warm (3.8 vs 1.1 ms), which is INFERRED to be canvas-size allocations (found_not_fixed #2).

### probe-render-state r1_counts (the new bars)
RED, dc7adf9 app, row alone (`live/red-base/rstate-r1counts-base.log`):
```
      r1_counts: frame_rings 0 -> 2 (+2), temporal_buffers 0 -> 0 (+0); peak_frame_time_ms first use 37.87, first fade 39.42, second fade (no creation) 1.53; load avg 3.63 3.90 5.12
PASS  r1_counts: 5 fades on one layer (Screen Split on every clip) hold exactly 2 rings (+2) and <= 2 temporal buffers (+0)
FAIL  r1_counts: longest frame across the layer's first use (its ring created) 37.87 ms <= 16.7 ms
FAIL  r1_counts: longest frame across the first fade (spare ring created) 39.42 ms <= 16.7 ms
PY 1 PASS / 2 FAIL
PROBE-RENDER-STATE RED
```
GREEN, C1 app, full probe x3 quiet: `PY 32 PASS / 0 FAIL` x3 (render-state 31/0 -> 32/0 as the plan expected). The r1_counts lines were:
```
      r1_counts: frame_rings 4 -> 6 (+2), temporal_buffers 9 -> 9 (+0); peak_frame_time_ms first use 1.59, first fade 1.66, second fade (no creation) 1.60; load avg 3.77 3.44 3.52
      r1_counts: frame_rings 4 -> 6 (+2), temporal_buffers 9 -> 9 (+0); peak_frame_time_ms first use 1.44, first fade 1.57, second fade (no creation) 1.56; load avg 3.03 3.23 3.73
      r1_counts: frame_rings 4 -> 6 (+2), temporal_buffers 9 -> 9 (+0); peak_frame_time_ms first use 1.35, first fade 1.62, second fade (no creation) 1.68; load avg 4.14 3.38 3.57
```
C1, row alone on a fresh app: `first use 9.02, first fade 11.75, second fade (no creation) 5.42` -> `PY 3 PASS / 0 FAIL`. The images are not cached yet on a fresh app, so this includes their first upload.

### Capture (fixture: one layer, opacity 0.5 -> alpha 191 everywhere, clip P16_01_baseline.png, no effects; `render_frame {time: 0.0}`; sizes via the composition's output size)
The `[Eyes] Captured frame` split (ms). C0 = before (all on the GL thread). C3 = after (GL thread = read only; convert and png run on the caller):

| build | size | read | convert | png | GL-thread share | HTTP round trip |
|---|---|---|---|---|---|---|
| dc7adf9 | 1080p | - | - | - | (no line) | 95.8-115.0 |
| dc7adf9 | 4K | - | - | - | - | 365.3 |
| C0 | 1080p x5 | 2.3-4.5 | 12.5-20.9 | 78.6-84.3 | 94-110 | 103.4-112.4 |
| C0 | 720p x2 | 1.0-3.7 | 6.5-12.9 | 36.7-49.0 | 44-66 | 60.6-78.4 |
| C0 | 4K | 7.9 | 68.4 | 296.2 | 372.5 | 375.9 |
| C2 | 1080p x5 | 2.2-4.2 | **3.7-6.4** | 78.2-86.9 | 84-98 | 95.9-104.2 |
| C2 | 4K | 8.3 | **23.9** | 298.6 | 330.8 | 344.4 |
| C3 | 1080p x5 | **2.3-5.0** | 3.9-7.8 (caller) | 79.3-92.3 (caller) | **2.3-5.0** | 93.0-115.6 |
| C3 | 720p x2 | **1.7-2.5** | 1.8-4.8 (caller) | 38.9-44.9 (caller) | **1.7-2.5** | 56.5-63.6 |
| C3 | 4K | **7.6** | 27.3 (caller) | 305.6 (caller) | **7.6** | 350.5 |

The C3 `read` bar (<= 16.7 ms at 1080p) is met; no PBO is needed for captures. The HTTP round trip does not shrink much (the PNG encode dominates and still runs before the response), as the plan predicted. Verbatim (`live/red2-c0/capt-C0.log`, `live/c3/capt-C3.log`):
```
[Eyes] Captured frame: SP/capt/C0/C0_t1080_1.png (1920x1080) read=2.4 convert=12.5 png=82.4 ms
[Eyes] Captured frame: SP/capt/C0/C0_t4k_1.png (3840x2160) read=7.9 convert=68.4 png=296.2 ms
[Eyes] Captured frame: SP/capt/C3/C3_t1080_1.png (1920x1080) read=2.4 convert=4.3 png=80.0 ms
[Eyes] Captured frame: SP/capt/C3/C3_t4k_1.png (3840x2160) read=7.6 convert=27.3 png=305.6 ms
```

### Live byte-identity A/B (sha256 of the PNG files; plus PIL `np.array_equal` on each pair)
| size | dc7adf9 (determinism x2) | C0 | C2 (ii) | C3 (iii) |
|---|---|---|---|---|
| 1920x1080 | 1eb4374362b4... x2 (and x3 timing) | same | same | same |
| 1280x720 | eb1f5191ac13... x2 | same | same | same |
| 3840x2160 | 650ef6677958... | same | same | same |

`PIL def1 vs def2: shape (1080, 1920, 4) array_equal=True mean RGBA [16.83, 0.97, 12.22, 191.0] alpha min/max 191/191` (identical line on all four builds). Alpha 191 means the premultiply branch was exercised live. The files are non-empty (62-299 KB).

### ctest (full, serial `-j1`)
- dc7adf9 + C0: `100% tests passed, 0 tests failed out of 729`
- C1: `out of 732` (+3 test_frame_ring)
- C2: `out of 736` (+4 test_pixel_convert)
- C3: `out of 736`

The plan's "720" was main at 1636785; outputs-c2 added 9.

### ctest teeth (mutated COPIES, deliverable sha256 unchanged before/after: `TEETH-*.txt`)
- test_frame_ring:
  - unmutated copy: `All tests passed (9 assertions in 3 test cases)`
  - `- framesAgo` -> `+ framesAgo`: `test cases: 3 | 1 passed | 2 failed`
  - clamp to frameCount removed (reads cells not written in this lifetime): `test cases: 3 | 3 failed`
  - test copy whose reset keeps `written[]`: passes, as the plan predicts.
- test_pixel_convert, `p.premultiply()` commented out: `test cases: 4 | 1 passed | 3 failed`. every-alpha fails on both backings, odd sizes fail for the non-opaque variants, PNG bytes fail, and forceOpaque passes.

### Probes on my builds (no threshold edits except the plan's r1_counts bar)
| probe | build | result |
|---|---|---|
| probe-canvas | C0 / C2 / C3 | 15/0 / 15/0 / 15/0 |
| probe-effects-parity | C1 / C3 | 46/0 / 46/0 |
| probe-render-state | C1 x3 / C2 / C3 | 32/0 x3 / 32/0 / 32/0 |
| probe-fitmode | C2 / C3 | 10/0 / 10/0 |
| probe-crossfade | C3 | 35/0 |
| probe-deck-clock | C3 | 10/0 |
| probe-outputs | C3 | 13/0 (the plan's "10/0" predates outputs-c2's rows) |

Summary lines are in `live/*/*.log` and `live/batch-*.out`.

### tests/visual Tier-1 (exactly test_sources / test_effects / test_audio_reactivity / test_time_sweep / test_performance; test mode; `AUDIODNA_NO_SPAWN=1`)
- dc7adf9: `8 failed, 5 passed in 1116.12s (0:18:36)`
- C3: `8 failed, 5 passed in 1122.30s (0:18:42)`
- The outcome lists are IDENTICAL (`diff` empty; `tier1/base-outcomes.txt`, `tier1/C3-outcomes.txt`). These are the same 8 pre-existing failures outputs-c1 recorded.

### Screen safety
- Every batch ran a Quartz window sampler. Every sampler reported `Output-named Audio-DNA windows []`, with at most 1 on-screen Audio-DNA layer-0 window.
- After every quit the full window list showed `audio-dna windows 0, Output-named 0`.
- The Output window was never opened, and test_output_window_level.py was never run.
- No screencapture, synthetic input, debugger, or temporary env hook was used.

## DRIFT / DEVIATIONS
- D1 Base is dc7adf9, not 1636785: outputs-c2 merged between plan and dispatch.
  - The cited source lines hold. HANDOFF items moved to lines 81-85, and ctest is 729, not 720.
  - The plan's fence on TestServer/ApiServer (C2 lane) is now lifted, but C4 stays deferred per the dispatch.
  - main moved to 0766f06 during the lane (`.harmony/s-rta-0927-work.md` only). I did not rebase; there is no overlap.
- D2 7070 `/api/render_frame` (ApiServer) has no width/height. The plan's "width/height 1280x720" only exists on 8080, so I used the composition's outputWidth/outputHeight.
- D3 The plan's GREEN relative check fails on cold runs (image upload), as described above. I diagnosed it with t2warm instead of accepting it silently.
- D4 probe-render-state had no `wait_no_compiler`. I copied probe-canvas.py's into it for r1_counts, as the plan's quiet rule asks.
- D5 test_pixel_convert's oracle folds the two old loops into one function with `if (forceOpaque)`. Each loop body is verbatim.
- D6 One extra mutant (clamp removed) beyond the plan's two.
- D7 C0's `png=` excludes the FileOutputStream tail flush (destructor after the log). C3's includes it: the scope closes before the log.
- D8 Commit trailer: `Co-Authored-By: Claude Opus 5.5 (1M context)` per this session's attribution instruction (the plan named another model).
- D9 The dispatch's "hitch row" RED/GREEN used both the t2 table and r1_counts (row alone and full).

## ISSUES
- None blocking. `git status` in the worktree is clean except the kept untracked `build-lane/`.

## found_not_fixed
1. **Image first-upload hitch (~9-14 ms per new image path, GL thread).** `CompositorEngine::loadKeyImage` decodes the PNG, copies it pixel by pixel with `getPixelColour`, and uploads it inside the frame that first shows the image. It is now the whole first-use / first-fade hitch in the cold t2 runs. Options: decode on a loader thread, and/or a PixelConvert-style row copy.
2. **4K warm leftover (3.7-3.8 ms vs 1.1 ms second fade)** on first use / first fade. INFERRED to be canvas-size allocations (ScratchPool / chain FBOs) at 4K. Not measured.
3. **A capture's HTTP round trip is still dominated by the PNG encode** (~80 ms at 1080p, ~300 ms at 4K). Out of plan scope (a lower PNG compression level would cut it).
4. **`juce::FileOutputStream` appends to an existing file.** VERIFIED: `juce_SharedCode_posix.h:490-496` does `lseek(f, 0, SEEK_END)` when the file exists. So a render_frame / snapshot to an existing path leaves the OLD PNG first in the file. This predates the lane and the probes delete first. takeSnapshot's names have 1-second resolution, so two snapshots in one second append.
5. **Two concurrent captureFrame callers**: the second overwrites `capturePromise_`, and the first times out after 5 s. With C3, such a racing pair could also take each other's pixels. The race itself predates C3. INFERRED; not exercised.
6. C4 (deferred): `frame_ring_cells` in /api/state + probe row `r1_cells`, and output_probe -> `PixelConvert::rgbaBottomUpToARGB(..., true)`. Its test case already exists. The C2 lane has merged, so this is unblocked.

## RISKS
- R2 (plan): a cell read before its first write. Guarded by the ctest invariant on the real `FrameRing::readIndex`, the 0-texture fallbacks, and the picture rows (r1_ring, r2_ring, r5_hold all GREEN on C1/C2/C3). The `assign` is load-bearing and must not be "simplified" to `resize`.
- R3 (plan): GL state at the lazy create. It now happens once per push for 480 pushes, at the same point where the bulk creation happened before. The picture rows of render-state, parity and crossfade are GREEN.
- The r1_counts 16.7 ms bar carries a ~10 ms image-upload term when the row runs alone on a fresh app (9.02 / 11.75 ms measured). Headroom is ~5 ms there and ~15 ms in the full probe.

## BORIS CHECKS (what to look for in the app)
1. Put Screen Split (or Frame Stutter) on a clip whose image is already showing somewhere, then trigger it. There should be no stutter at the start. Before, one frame was ~27 ms.
2. Crossfade between two clips that both have Screen Split, the first time on that layer. It should be smooth. A small hitch can remain if the incoming clip's image was never shown before (found_not_fixed #1).
3. Screen Split cells right after a Resolution change fill in over the next frames exactly as before. No black or garbage cell.
4. `curl -X POST 127.0.0.1:7070/api/snapshot` while a clip plays at 1080p. The picture should no longer freeze for ~0.1 s (the GL thread now pays ~3 ms).

## METRICS
- 4 plan commits + 1 report commit.
- ctest 729 -> 736.
- Live runs: t2 x14, probe runs x17 (2 of them single-row r1_counts), Tier-1 x2, capture A/B x5.
- Lock held only per batch (8 batches), with a >= 45 s cooldown between them.

## PACKET QUALITY
- Clarity: CLEAR. The plan was specified to the line.
- Missing context: 7070 render_frame ignores width/height (D2). The plan's ctest/probe counts predate outputs-c2 (D1). The plan did not anticipate that the image upload shares the hitch window, though its R4 mentions media upload as "inside the noise"; measured, it is ~10 ms.
- Unused context: none.
- Self-brief files: CLAUDE.md (worktree) was useful. `.harmony/notebook.md` was read by grep of its headings only (176 KB), and its last entries were useful. The routines-timing report T2 and t2.py were essential.

## KNOWLEDGE CONTEXT
- Tools used: grep only (no KNOWLEDGE_TOOLS block; graphify not queried).
- Impact authority: grep, not authoritative. Conservative posture: every caller of the ring functions was read (2 getOrCreate/push pairs, 2 readers, 3 release paths).
- Risk level: NORMAL.

## ERRATA
- Commit 1a24467's message says "1080p 93-110 ms" for the old GL-thread share; the measured sums are 94.4-109.7 ms
  (C0 table above). HANDOFF item 4 is corrected to 94-110 in the report commit.

## Fix round (lane renderperf-fix; base e617312, fix commit 7500718)

STATUS: DONE
RESULT: The one MUST is confirmed and fixed. On C3, a second capture could hand its pixels to the first caller, which then returned ok:true with the wrong picture. Each capture's read now reaches its own caller by value through its own promise. RED on e617312 with a temporary hook: capture A wrote capture B's picture (B's size, then B's colour at the same size) and B failed. GREEN on 7500718 with the same hook, twice: each capture got its own picture. Without the hook, the capture bytes match C3 exactly, ctest 736/736, 7 probes GREEN, Tier-1 outcome list identical to C3.
FACTS: `src/render/Renderer.h` (`struct CaptureRead`, `std::promise<CaptureRead>* capturePromise_`; `capturePixels_` / `captureReadW_` / `captureReadH_` / `captureReadMs_` deleted); `src/render/Renderer.cpp` (captureFrame: `CaptureRead read = future.get()`; processPendingCapture: `set_value(std::move(read))`); evidence `.harmony/.reports/s-rta-0927/renderperf-evidence/fix/` (live/race/*.log, live/fix1, live/fix2, live/tier1, live/capt/sha256-FIX-vs-C3.txt, ctest-fix.log, hook-strings-count.txt, scripts/)
METHOD: I confirmed the finding by reading the code, then reproduced it live. I built three apps from build-lane: e617312 + temporary hook (redhook), 7500718 + the same hook (greenhook), and 7500718 without the hook (fix). The hook was never committed. A new scratch probe, capture_race, runs two overlapping 8080 render_frame calls and decodes every PNG. Every live run held /tmp/audiodna-live.lock as `renderperf-fix` in 4 batches, released between batches with >= 45 s cooldown. Apps were launched with `open -g` and quit with osascript, and a Quartz window sampler ran in every batch.
CONFIDENCE: high. The race reproduced on the first try exactly as the code reading predicted, including the log lines. The fix removes the shared state instead of narrowing the window. VERIFY: `git -C <wt> show 7500718 -- src`; `grep -c capturePixels_ src/render/Renderer.*` = 0; `renderperf-evidence/fix/live/race/race-redhook.log` vs `race-greenhook.log`.
UNKNOWNS: The live RED needs the temporary hook; without it the window is microseconds and a frame is 8-16 ms. The probe cannot be re-run on the committed build (by design: `h_hook` FAILs there). No ctest covers this: Renderer.cpp is not linked into any ctest target.
NUANCE: A separate older race is still there (found_not_fixed #5, first half). A second captureFrame can still overwrite capturePromise_ while the first is waiting for the GL thread, so the first times out after 5 s. It fails safely and never gives wrong data. It predates C3 and was not part of this finding.
HANDOFF-NEEDS: Harmony: behavioral gate on lane/renderperf-0927 (now 5 plan/fix commits + 2 report commits), then merge. The Reviewer should read the 7500718 diff (about 40 lines in 2 files).

INBOX-RECHECK: none

### Finding verification (MUST: C3 shared capture members race)
- **Confirmed by reading the code** (e617312 `src/render/Renderer.cpp`):
  - processPendingCapture wrote `capturePixels_`, `captureReadW_/H_` and `captureReadMs_`, then called `set_value(true)` and `capturePromise_ = nullptr`. That frees the slot for the next capture.
  - captureFrame took `captureMutex_` only AFTER `future.get()` and moved out whatever those members held at that moment.
  - The callers are 8080 `/api/render_frame` (TestServer.cpp:588), 7070 `/api/render_frame` (ApiServer.cpp:1164) and `takeSnapshot` (Renderer.cpp, which runs on the snapshot thread in MainComponent.cpp:6563 and ApiServer.cpp:713). They can all be in flight at the same time.
- **Confirmed live** (RED, `renderperf-evidence/fix/live/race/race-redhook.log`, raw lines):
```
      r1_sizes: A ok=True [...] -> 320x180 centre red; B ok=False [... "error": "Frame capture failed"] -> none; B sent 506 ms after A, A answered at 1511 ms, overlap=True
FAIL  r1_sizes: capture A (640x360) returns ok and its PNG decodes 640x360 (320x180 centre red)
FAIL  r1_sizes: capture B (320x180) returns ok and its PNG decodes 320x180 (none)
      r2_content: load blue (200, '{"ok":true}'); A ok=True [...] -> 640x360 centre blue; B ok=False [...] -> none; B sent 932 ms after A, A answered at 1526 ms, overlap=True
FAIL  r2_content: capture A (armed while red showed) returns ok and decodes RED -- its own frame (640x360 centre blue)
FAIL  r2_content: capture B (armed after blue loaded) returns ok and decodes BLUE (none)
PY 3 PASS / 4 FAIL
[Eyes] Captured frame: r1_A.png (320x180) read=0.3 convert=0.1 png=1.9 ms
[Eyes] Invalid capture dimensions: 320x180
[Eyes] Captured frame: r2_A.png (640x360) read=0.8 convert=0.4 png=6.9 ms
[Eyes] Invalid capture dimensions: 640x360
PROBE-CAPTURE-RACE RED
```
  - r2_content is the silent case the finding described. Both captures are 640x360, so the size check could not catch the swap: A returned ok:true with B's (blue) picture.
  - Nothing was refuted.

### The fix (7500718)
- `Renderer::CaptureRead {ok, pixels, width, height, readMs}` replaces the four shared members. The promise is now `std::promise<CaptureRead>`.
- processPendingCapture fills a local CaptureRead (the same `glReadPixels` into the same kind of vector, still under captureMutex_ and gated on the lock size) and calls `set_value(std::move(read))`. The invalid-size path sends `CaptureRead{}` (ok=false), the same result as the old `set_value(false)`.
- captureFrame's own `future.get()` returns the read. After the signal nothing shared is read, so no later capture can reach it.
- Unchanged:
  - no new mutex, and the GL-thread locking is the same;
  - the 5 s timeout path;
  - the `[Eyes] Captured frame: <path> (WxH) read= convert= png= ms` line;
  - PixelConvert / PNG;
  - GL-thread cost: still one vector allocation per capture, as in C0 and C3.
- This is the finding's first suggested option. I did not take the second option (a request-scoped single-flight guard on the calling side). The by-value handoff removes the shared state itself and adds no lock.

### GREEN (`renderperf-evidence/fix/live/race/race-greenhook.log`, `race-greenhook-2.log`, raw)
```
PASS  r1_sizes: capture A (640x360) returns ok and its PNG decodes 640x360 (640x360 centre red)
PASS  r1_sizes: capture B (320x180) returns ok and its PNG decodes 320x180 (320x180 centre red)
PASS  r2_content: capture A (armed while red showed) returns ok and decodes RED -- its own frame (640x360 centre red)
PASS  r2_content: capture B (armed after blue loaded) returns ok and decodes BLUE (640x360 centre blue)
PY 7 PASS / 0 FAIL
PASS  h_hook: the collect-delay hook was armed (5 captures delayed)
PROBE-CAPTURE-RACE GREEN
```
(`PY 7 PASS / 0 FAIL` and `PROBE-CAPTURE-RACE GREEN` on both greenhook runs.)

### The temporary hook (never committed)
- `renderperf-evidence/fix/scripts/hook.py add|remove` inserts one line: when `AUDIODNA_TEST_CAPTURE_COLLECT_DELAY_MS` is set, captureFrame sleeps that long after its future is ready and before it collects. That is the same spot in both trees (before `if (!future.get())` on e617312, before `CaptureRead read = future.get()` on 7500718). It also adds `<thread>` / `<cstdlib>`.
- It was removed after each hooked build.
- Renderer.cpp sha256 after removal equals the pre-hook sha256 for both e617312 and the fix (`renderer-sha256.txt`, `shasum -c` OK).
- `strings` count of the variable name: redhook 1, greenhook 1, **fix 0**, and 0 for the final build-lane binary (`hook-strings-count.txt`).
- Probe design (capture_race.py docstring):
  - A (640x360) arms; the GL thread signals it within a frame or two; the hook holds A for 1.5 s.
  - B arms 0.5 s later (r1, 320x180) or 0.9 s later after the blue image is loaded (r2, same 640x360), and is serviced inside A's window.
  - Every output path is deleted first (FileOutputStream appends, found_not_fixed #4).
  - Each overlap row also requires that B was sent before A's answer arrived.
  - `h_hook` FAILs the run if the hook never fired.

### Gates on the unhooked fix build (apps/fix = build-lane at 7500718)
- Build: `cmake --build build-lane -j3` exit 0. There are 24 warnings, all in JUCE headers under `_deps/juce-src` (`build-fix-summary.txt`), and none in Renderer.cpp.
- ctest full serial: `100% tests passed, 0 tests failed out of 736` (`ctest-fix.log`).
- Capture byte identity (7070 render_frame, the same fixture as C0-C3): sha256 equals C3 (which equals dc7adf9) at 1920x1080 x5, 1280x720 x2 and 3840x2160 (`live/capt/sha256-FIX-vs-C3.txt`). PIL `array_equal=True`, alpha 191/191.
  - Split timings: 1080p read 2.3-4.1 ms, 4K read 7.8 ms, png 79.7-98.3 / 305.7 ms.
  - These timings were not quiet-gated (SKIPQUIET): compilers were 0/0 at step start and end, load 4.14-4.45.
- Probes (existing, none re-thresholded):

| probe | result |
|---|---|
| canvas | `PY 15 PASS / 0 FAIL` GREEN |
| effects-parity | `PY 46 PASS / 0 FAIL` GREEN |
| render-state | `PY 32 PASS / 0 FAIL` GREEN (r1_counts first use 1.57, first fade 1.57 ms) |
| fitmode | `PY 10 PASS / 0 FAIL` GREEN |
| crossfade | `PY 35 PASS / 0 FAIL` GREEN |
| deck-clock | `PY 10 PASS / 0 FAIL` GREEN |
| outputs | `PY 13 PASS / 0 FAIL` GREEN |

- tests/visual Tier-1 (exactly test_sources / test_effects / test_audio_reactivity / test_time_sweep / test_performance, test mode): `8 failed, 5 passed in 1128.14s (0:18:48)`. `diff` against C3's outcome list is empty; these are the same 8 pre-existing failures.

### Screen safety (fix round)
- Window samplers in all 4 batches: `Output-named Audio-DNA windows []`, max 1 on-screen layer-0 Audio-DNA window.
- After every batch: `audio-dna windows 0, Output-named 0`.
- No Output window, no test_output_window_level.py, no screencapture, no synthetic input, no debugger.
- The one env hook was reverted, and the final build's strings count is 0.
- `.venv` symlink removed at every batch end.

### found_not_fixed (fix-round update)
- #5 splits in two:
  - (a) Two captures swapping pixels: **FIXED** (7500718).
  - (b) Still open, older than C3: a second captureFrame armed while the first is still waiting for the GL thread overwrites `capturePromise_`. The first then times out after 5 s, and its timeout path clears whatever capture is pending then. It fails safely and never gives wrong data. The finding's second option (a caller-side single-flight guard) would close it; not done here.
- #1-#4 and #6 unchanged.

### Fix-round DRIFT
- F1 The commit message for 7500718 was written before the live RED/GREEN runs. The runs then matched it (RED 3/4 on redhook, GREEN 7/0 x2), so no amend was needed.
- F2 capture_race lives in the evidence dir, not `.harmony/`: it needs the temporary hook and cannot pass on a committed build.
- F3 The capture-timing step skipped the quiet wait (SKIPQUIET) so it would not hold the shared lock. Compilers were 0/0 at both ends. No perf claim rests on it; it is the byte-identity check.

### Fix-round METRICS
- 1 fix commit (7500718) + this report commit.
- Live: 3 race runs, 1 capture A/B, 7 probes, 1 Tier-1.
- 4 lock holds as `renderperf-fix`: 19:00:06-19:00:43, 19:07:29-19:15:59, 19:19:05-19:24:18, 19:25:03-19:44:58.
