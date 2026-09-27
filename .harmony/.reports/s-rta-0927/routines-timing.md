# s-rta-0927 lane routines-timing (T1 routine timing flake, T2 spare-ring hitch)

## BUILDER REPORT

STATUS: DONE
RESULT: T1: the flake is in the probe, not the canvas. The windows now select samples by the routine's own published position. GREEN 98 PASS / 0 FAIL on 15 of 15 runs: 5x the main app, 5x the lane build, and 5x the main app after Harmony rebuilt it with routine-display merged. T2: measured with options, nothing changed. Commits `9cbfbd5` (probe + notebook) and this report + evidence.
FACTS: `.harmony/probe-routines.sh` (rows loop / stack / jump / jumploop, helpers `rpos` / `pos_window` / `first_move_beat`); evidence `.harmony/.reports/s-rta-0927/routines-timing-evidence/analysis.txt`, `.harmony/.reports/s-rta-0927/routines-timing-evidence/replay-reps.txt`, `.harmony/.reports/s-rta-0927/routines-timing-evidence/logs-rep/reppost1-1.log`, `.harmony/.reports/s-rta-0927/routines-timing-evidence/t2/batch-t2.out`; T2 source `src/render/CompositorEngine.cpp:1671` (getOrCreateRingBuffer), `src/render/CompositorEngine.cpp:1753` (ring swap), `src/render/CompositorEngine.cpp:885` (first-frame-of-crossfade hand-over)
METHOD: I instrumented a scratch COPY of main's probe: timestamps around every HTTP call, status clockBeat per sample, render_frame t0/t1, and a repeat section that fires the two flaky starts 12x per launch. I built b766720 (pre-canvas). Per app I ran 5 full runs quiet, 6 loaded (post), and 2x12 repeated starts. The start of each routine was estimated from the samples themselves (ts - position x 0.5 s). The OLD and NEW row code was replayed offline on every recorded sample file. Teeth come from real recorded mutant scenarios. T2 polled /api/state peak_frame_time_ms every ~20 ms across a layer's first use, its first crossfade and its second crossfade (3x 1080p, 1x 4K).
CONFIDENCE+VERIFY: High for the T1 mechanism. It was reproduced live: the exact 96/2 row pair, with the failing sample at routine beat 1.195, which is the first recorded move. The probe-side fix is also high: NEW rows pass 102/102 full-run and 192/192 repeated-start evaluations, including the 4 live FAILs and a clock-slip run. Verify with `AUDIODNA_LOCK_OWNER=<you> ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8 bash .harmony/probe-routines.sh` in the worktree (needs the `.venv` symlink): expect `98 PASS / 0 FAIL` and position-based text in rows 8/11/11j.
UNKNOWNS/NOT-DONE: No app code changed. The app-side findings below are only in found_not_fixed. I did not isolate what holds the message thread 40-90 ms at a routine start that restores (no profiler allowed). The loaded-run stack anomaly (first move never applied) is not investigated.
NUANCE: The handoff's INFERRED cause (slow synchronous canvas readback stalls the app) is refuted by the data. Captures did get about 2x slower, but they do not stall the message thread, and the anchor lags are the same on both builds. The OLD 11j loop-landing row also caught a beat-clock slip. The new row does not, by design: slips remain caught by the wall-time grid rows (row 7). That one assertion moved from row 11j to row 7 on purpose. main moved on after I branched (4b0c39a -> 8573006, routine-display merged). My probe file does not conflict. notebook.md gets appends on both sides, so expect a trivial merge conflict there.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
The routine timing flake is a probe artefact. Rows 8, 11 and 11j ended their windows at a fixed wall time after a sampled "running" anchor. That anchor lags the routine's real start by 0.06-0.12 s on BOTH the pre-canvas and post-canvas builds, so each window ended -0.02..+0.05 s from the next recorded event. Those windows now select samples by the routine's own published position and end >= 0.1 s before the event. T2's hitch is 35.5-36.6 ms at 1080p and 38.5 ms at 4K on every first crossfade of a layer, the same as its first use. The options are costed below.

---

## T1 -- diagnosis (instrumented, run, not read)

### Setup
- PRE-CANVAS app: `b766720` built in the worktree (`build-pre`, Release, cmake exit 0). It has since been deleted to keep the tree clean; rebuild with the lane packet's commands. POST-CANVAS app: `/Users/boriskarpman/projects/RealTimeAudio/build/.../Audio-DNA.app` (6fe8abb code at the time of the diagnosis runs, binary mtime before 13:30).
- The SAME probe drove both: main's `.harmony/probe-routines.sh`, identical at b766720 and 6fe8abb (`git diff --stat b766720 6fe8abb -- .harmony/probe-routines.sh` is empty). Every endpoint it uses exists on the old binary, so all rows ran on both.
- The instrumented copy lives at `routines-timing-evidence/scripts/probe-routines-timed.sh` (patch: `scripts/patch.py`). The committed probe was never touched for measurement. Additions: `tc`/`ts` (time after the composition and status GETs) and `cb` (status clockBeat) per sample; `wait`/`edge` logging; t0/t1 around every render_frame (ref, pert, mid, rest, jpre, jmid); anchors T1..TJ. The repeat copy adds `scripts/rep-section.sh`: 12x per launch, (a) the Jump start of row 11j and (b) the stack of row 11, evaluated by the same OLD row code.
- Env: `ROUTINES_APP=<app> ROUTINES_RECORD_PAUSE=1.8 AUDIODNA_LOCK_OWNER=routines-timing`. The lock was held per run and released between runs. Quiet runs waited for `pgrep -x clang` / `clang\+\+` to be empty first. The load average is printed on every log's first line.

### Per-run results (quiet, interleaved pre/post; summary lines verbatim)
| run | summary line (verbatim) | load at start | render_frame ms (ref pert mid rest jpre jmid) |
|---|---|---|---|
| pre1 | 98 PASS / 0 FAIL | { 5.50 7.53 7.04 } | 49 48 50 49 46 49 |
| post1 | 98 PASS / 0 FAIL | { 3.76 6.42 6.67 } | 141 107 107 138 112 110 |
| pre2 | 98 PASS / 0 FAIL | { 3.74 4.30 4.45 } | 61 50 54 48 47 68 |
| post2 | 98 PASS / 0 FAIL | { 4.86 4.60 4.54 } | 149 110 106 136 103 108 |
| pre3 | 98 PASS / 0 FAIL | { 4.81 4.66 4.56 } | 51 63 79 54 51 50 |
| post3 | 98 PASS / 0 FAIL | { 5.76 5.14 4.79 } | 143 110 131 142 109 113 |
| pre4 | 98 PASS / 0 FAIL | { 4.67 4.94 4.79 } | 49 58 83 54 71 61 |
| post4 | 98 PASS / 0 FAIL | { 5.10 5.12 4.91 } | 153 109 108 134 113 108 |
| pre5 | 98 PASS / 0 FAIL | { 5.49 5.26 4.99 } | 54 52 73 58 54 63 |
| post5 | 98 PASS / 0 FAIL | { 6.14 5.72 5.21 } | 159 107 98 144 114 111 |

**Capture calls (probe client, HTTP round trip):**

| build | n | median | p90 | max | capture size |
|---|---|---|---|---|---|
| pre | 30 | 54 ms | 71 ms | 83 ms | panel size, 756x518 / 756x878 (app log `[Eyes] Processing capture: 756x878 vp=...`) |
| post | 30 | 112 ms | 144 ms | 159 ms | canvas 1920x1080 |

The app exposes no capture timing of its own, only the `[Eyes] Processing/Captured` stderr lines, which carry no time.

**Repeated starts (2 launches x 12 per app, quiet), OLD rows:**

| app | Jump starts FAILED | stack starts FAILED | Jump failures | full-run summary lines |
|---|---|---|---|---|
| pre | 0/24 | 0/24 | none | `97 PASS / 1 FAIL` (reppre1: the 11j loop landing, see "clock slip"), `98 PASS / 0 FAIL` |
| post | 2/24 | 0/24 | reppost1 rep 4 and reppost2 rep 12, each FAILing "hard cut" + "restored to 1.0", i.e. exactly the s-rta-0926b 96/2 pair | `98 PASS / 0 FAIL`, `98 PASS / 0 FAIL` |

The failing lines, verbatim:

- `FAIL  11j: a hard cut -- no sample strictly between 0.15 and 0.95 up to TJ+0.5 (72 samples, 1 in between: [0.5])`
- `FAIL  11j: on the bar L0 opacity is restored to 1.0 (TJ+0.1..TJ+0.5: 12 samples, [0.5, 1.0])`

Two-sided Fisher exact test on 0/24 vs 2/24: p ~ 0.49 (not a build difference).

**Loaded (post app, my -j3 lane build + another lane compiling, load avg 7.5-15; NOT perf numbers):** 6 runs. 5 were `98 PASS / 0 FAIL` and one was `94 PASS / 4 FAIL`. Its fails were 3 grid rows plus "earlier ramp stays quiet", a different mechanism (see "clock slip" and "stack anomaly").

### The failing samples (reppost1 rep 4 / reppost2 rep 12; `failing-rep-samples/*.json`)

| t - start (s) | t - TJ (s) | op0 | routine position (beats) |
|---|---|---|---|
| +0.565 | +0.452 | 1.000 | 1.109 |
| **+0.598** | **+0.485** | **0.500** | **1.195** (first recorded move, inside the old TJ+0.5 window) |
| +0.632 | +0.519 | 0.500 | 1.259 |

In that start, TJ - start = 0.113 s; in the reppost2 case it is 0.105 s, and the 0.5 was sampled at t - TJ = 0.495, routine beat 1.195. The routine did exactly what it recorded: the move landed at start + 0.598 s = beat 1.195. The window ended too late because TJ is late.

### Discrimination
1. **Anchor lag, pre vs post.**

   Full runs:

   | anchor | pre | post |
   |---|---|---|
   | T6 - start | 0.059-0.100 s | 0.075-0.106 s |
   | TJ - start | 0.063-0.115 s | 0.059-0.108 s |
   | T2 - start | 0.069-0.108 s | 0.070-0.107 s |

   Repeated starts:

   | anchor | pre medians | pre max | post medians | post max |
   |---|---|---|---|---|
   | Jump TJ - start | 0.078 / 0.080 s | 0.117 / 0.109 s | 0.089 / 0.074 s | 0.113 / 0.105 s |
   | stack T6 - start | 0.071 / 0.079 s | 0.089 / 0.092 s | 0.072 / 0.067 s | 0.083 / 0.089 s |

   Same distributions on both builds.
2. **Where the lag comes from.** A start whose restore fires holds the message thread 38-86 ms. This is measured as the maximum status staleness within +-0.25 s of the start, where staleness = ts - (w0 + clockBeat x 0.5). Before "running" is published, clockBeat stays frozen across 2-3 samples. The sampler's ~40 ms cadence and the `wait` helper's 50 ms poll add to it. A start without a restore (row 10, "start from now") stalls only 9-22 ms and its T4 lags only 0.002-0.054 s.
3. **Old-window margin = (start + first-move beat x 0.5) - (T + 0.5).**

   | window | margin across 16 full runs (both builds) |
   |---|---|
   | stack | -0.018..+0.039 s |
   | jump | -0.018..+0.049 s |
   | loop-8 "holds 0.9" end vs its glide start | -0.019..-0.058 s (ALWAYS past the event; it passed only on the 0.05 tolerance) |
   | 11j loop hold end vs loop point | -0.016..+0.040 s |

   The first-move beat varies per take between 1.173 and 1.237 beats (recorder stamps). The flake is a lottery on a sliver about 0-20 ms wide.
4. **Do failures coincide with long captures?** No. In the Jump repeats no capture runs at all, and they still failed 2/24. In the full runs, the two captures that overlap a sampler (mid.png, jmid.png) raise status staleness to only:

   | build | staleness during the capture | max staleness elsewhere in the run |
   |---|---|---|
   | pre | <= 18.3 ms | 40-79 ms |
   | post | <= 21.4 ms | 41-70 ms |

   HTTP GETs during the captures: <= 6.9 ms. The GL-thread capture does not touch the message thread or the REST reads.

**VERIFIED cause:** the probe windows are anchored to the first sample that saw "running". That sample lags the routine clock's start by 0.06-0.12 s: the start's restore stalls the message thread, plus sampling delay. The windows end 0.5 s after that anchor, while the first recorded move is at 0.587-0.618 s after the start. The canvas merge did not change the lag; the 2-in-5 rate on s-rta-0926b was a lottery run on this sliver.

**Strongest alternative: the slower canvas capture stalls the app (the handoff's inference).** It is ruled out by:
- `analysis.txt` "captures overlapping a sampler": during a 98-159 ms canvas capture the message thread keeps ticking (staleness <= 21 ms).
- The reppost Jump repeats (`logs-rep/reppost1-1.log` rep 4) failed with no capture in flight.
- The lags are equal on the pre and post builds.

**Second alternative: beat-clock slips.** It is ruled out for the reported rows. The two slips seen (below) moved events LATER. The reported failures need an event EARLY, or a window LATE.

### Clock slip (found, not fixed; app side)
The routine beat clock loses time when the message thread or the tracker stalls. Every later routine event then lands that much later in wall time.

| run | build | slip | what happened | effect |
|---|---|---|---|---|
| loadpost1 | post, load ~15 | +0.5 s = one whole beat | clockBeat frozen for 0.52 s in grid.json | grid rows FAIL x3 at +0.44..+0.45 s late |
| reppre1 | PRE, quiet | +0.155 s | clockBeat advanced only 0.021 beats over a 0.139 s freeze | OLD 11j loop landing FAIL at +8.3 s |

Source of the whole-beat case, VERIFIED by reading: `src/recording/RecorderClock.cpp:57` counts a beat only on a phase wrap of more than 0.5, so a tick gap longer than ~1 beat misses a wrap. The 0.155 s case looks like the tracker's phase itself pausing (INFERRED). Evidence: `anomalies/loadpost1-grid.json`, `anomalies/reppre1-jumploop.json`, `analysis.txt` "beat-clock discontinuities".

## T1 -- fix

### Commit
`9cbfbd5 probe(s-rta-0927 routines-timing T1): probe-routines timing windows on the routine's own clock`, touching only `.harmony/probe-routines.sh` and `.harmony/notebook.md`.

### Rule
`snap()` reads `/api/composition` BEFORE `/api/routine/status`. Both op0 and the status are written by the message thread. So a sample whose status says the routine is at position p read a look from position <= p, whatever the anchor lag, start stall or clock slip. Rows 8, 11 and 11j now select samples by that published position (`rpos` / `pos_window`) and end `MARGIN_BEATS = 0.2` beats (0.1 s at 120 BPM) before the next recorded event. The first-move beat is read from the take (`first_move_beat`: earliest L0 opacity point in [0, 16)), not assumed. Tolerances are unchanged. Two windows that used to accept >= 1 sample now need >= 3 (stricter).

| row | was (wall) | now (routine position) | teeth (still catches) |
|---|---|---|---|
| 8 loop "holds 0.9 until the return glide begins" | T2+7.0..T2+7.45 s: ended 0.02-0.06 s INTO the glide | beats 14.0..14.8 (glide starts at 15.0) | a missing 0.9 move (op0 would read 0.5) or a return glide that starts a beat or more early (the recorded Ease glide reaches 0.968-0.978 within its first 0.8 beats, beyond the 0.05 tolerance) |
| 11 stack "the later restore wins" | T6+0.12..T6+0.5 s: ended ON the first move | beats 0.4..m0-0.2 (0.995-1.016) | replayed on a start WITHOUT a restore (real `fromnow.json`, 17 runs): `no ... 9 samples, [0.1]`; a lost D9 arbitration would show the ramp's 0.2-0.8 values here |
| 11j "a hard cut -- no sample 0.15..0.95" | t < TJ+0.5 s | every sample before the routine passes m0-0.2 | replayed on the EASE start of the same run (real `glide.json`): `no ... 12 in between: [0.213, 0.27, ..., 0.942]` (the restoreStyle-ignored regression) |
| 11j "on the bar restored to 1.0" | TJ+0.1..TJ+0.5 s | beats 0.4..m0-0.2 | replayed on the no-restore start: `no ... [0.1]` |
| 11j loop "holds 0.9 through its last beat" | TJ+7.0..TJ+7.9 s: ended ON the loop point, held only by the ~80 ms loop-tick stall | beats 14.0..15.8 | replayed on the Ease loop (real `loop.json`): `no ... 0.8999..0.9676` / `0.9777` (the glide-not-disabled regression) |
| 11j loop "cuts to 1.0 ON the loop point" | first 1.0 at TJ+7.9..8.15 s | first 1.0 must be cycle 2 at <= 0.5 beats (0.25 s), or cycle 1 >= 15.8 (the looping tick writes before it publishes) | an early cut (first 1.0 in cycle 1 before 15.8) or a late/missing cut (> 0.5 beats into cycle 2) FAILs. It no longer measures the beat clock against wall time; row 7 grid does (loadpost1) |

### Offline replay of OLD vs NEW row code on the recorded samples
Files: `replay-runs*.txt`, `replay-reps.txt`; scripts `scripts/replay.sh`, `scripts/replay-reps.sh`.

| data set | NEW rows ok | OLD rows ok | TEETH scenarios |
|---|---|---|---|
| 17 full runs x 6 rows | 102/102 | 101/102 (the reppre1 clock-slip landing) | 68/68 `no` |
| 4 launches x 12 repeated starts x 4 rows | 192/192 | 188/192 (the 4 live FAILs, identical text to the live run) | -- |

### GREEN (committed probe, sha256 prefix 89c0ef7f345c, ROUTINES_RECORD_PAUSE=1.8, quiet)
- main app `/Users/boriskarpman/projects/RealTimeAudio/build/.../Audio-DNA.app` (pre-13:30 binary), 5x: `98 PASS / 0 FAIL` x5 (loads 2.79-5.47).
- lane build `build-lane` (lane/routines-timing-0927 = 4b0c39a + 9cbfbd5), 5x: `98 PASS / 0 FAIL` x5 (loads 2.68-6.64).
- main app REBUILT by Harmony at 13:30:18 (routine-display merged, 20cede2 code; the binary mtime was checked before and after the batch), 5x: `98 PASS / 0 FAIL` x5 (loads 2.48-5.49; `logs-green/greenmainB-*.log`).
- Logs: `logs-green/*.log`. Example of new row text, verbatim from greenmain1: `PASS  stack: the later restore wins -- L0 opacity holds 1.0 after Probe Routine starts, no ramp values (routine beats 0.4..0.995, first move at beat 1.195: 9 samples, [1.0])`.
- RED-first: none of these rows is new. All 6 are the existing rows with a re-based window, and they have to pass on both apps because the app is correct. The teeth table above replaces the RED step with real recorded mutant scenarios. The 4 live RED FAILs of the OLD rows are reproduced by the offline replay and pass under the NEW rows.

### ctest
`ctest --test-dir build-lane -j1`: `100% tests passed, 0 tests failed out of 667` (12.39 s).

---

## T2 options -- the one-time hitch when a layer's first crossfade creates its spare frame ring

### Where
The crossfade's first frame calls `handOverClipHistory` (`src/render/CompositorEngine.cpp:885-887`). That SWAPS the layer's ring into the outgoing slot (`src/render/CompositorEngine.cpp:1753`), so the incoming clip's Screen Split / Frame Stutter finds an empty entry. `getOrCreateRingBuffer` (`src/render/CompositorEngine.cpp:1671`, loop at `:1700`) then creates `kMaxRingFrames = 480` (`CompositorEngine.h:252`) textures + FBOs + clears in one frame. The call sites are Frame Stutter `:377` and Screen Split `:1823`. The layer's FIRST use of the effect pays the same cost.

### Measured
Main app, fresh launch per run, a fresh layer with 6 columns alternating A/B, each with [Screen Split 0.15 0.15 0.25 0], T = 1 s. `/api/state peak_frame_time_ms` (CPU-side frame time, reset on read) was polled every ~20 ms. Script: `scripts/t2.py`; output `t2/batch-t2.out`, `t2/*.json`. No compiler was running; the load average is on each line.

| run | canvas | load | first use (ring 0->1) | FIRST crossfade (spare, rings 1->2) | 2nd crossfade (no creation) | steady median / max |
|---|---|---|---|---|---|---|
| 1 | 1920x1080 | 8.96 (graphify rebuild on 1 core) | 35.92 ms @+0.056 s | **35.69 ms** @+0.070 s | 5.42 ms | 0.70 / 4.37 ms |
| 2 | 1920x1080 | 3.07 | 36.44 ms | **36.56 ms** @+0.063 s | 5.89 ms | 0.74 / 1.05 ms |
| 3 | 1920x1080 | 3.08 | 36.18 ms | **35.45 ms** @+0.076 s | 2.88 ms | 0.72 / 1.55 ms |
| 4 | 3840x2160 | 6.28 | 37.76 ms | **38.46 ms** @+0.070 s | 1.16 ms | 0.56 / 0.91 ms |

- Exactly one frame spikes. In the fade1 timeline the frames just before and after the spike read 0.6-0.9 ms. That works out to ~0.075 ms per ring cell (36 ms / 480).
- The handoff's 34-52 ms range and the 51.86 ms r1_counts flake match this 35-38 ms quiet value plus load: the r1_counts bar is 50 ms.
- temporal_buffers stayed 0 (no temporal effect in the fixture). With a temporal effect, a canvas-size spare temporal buffer is also created on the first fade: one FBO, 7.9 MiB at 1080p, 31.6 MiB at 4K. Not measured.

### VRAM per ring (computed from `RenderGeometry::ringDownscale`, `src/render/RenderGeometry.h:51`)
- 1080p: downscale 4 -> 480x270 cells x 4 B x 480 = 248,832,000 B = **237.3 MiB**.
- 4K: downscale 8 -> 480x270 -> **237.3 MiB** (cells are capped at 480 px wide).
- 1440p: 426x240 -> 187.2 MiB.

The spare ring doubles this per crossfading (deck, layer) chain that runs Split/Stutter.

### Options
| option | hitch | VRAM | complexity / risk |
|---|---|---|---|
| A. status quo | 35-38 ms once per chain at first use AND at first crossfade | 237 MiB spare only once the chain has crossfaded | none |
| B. create the spare with the primary ring | first crossfade 0 ms, first use ~72 ms (INFERRED: 2 x 36) | +237 MiB for every Split/Stutter chain, even one that never fades | ~10 lines, `getOrCreateRingBuffer`; strictly worse for live use |
| C. **lazy per-cell allocation** (allocate cell i on its first write; the incoming ring already starts empty, and `getFrameFromRing` clamps to `frameCount`) | ~0 ms. About +0.075 ms per frame for the first 480 frames (4-8 s at 60-120 fps); removes BOTH hitches | unchanged (grows to 237 MiB as the ring fills) | moderate, ~30-40 lines in CompositorEngine only (`FrameRingBuffer::allocated`, `pushFrameToRing`, the resize drop path). Live check: probe-render-state r1_counts (the peak bar could then be tightened); no Renderer.cpp touch. **Recommended.** |
| D. amortised pre-warm (on primary-ring creation, build the spare over the next ~20 frames, ~24 cells/frame ≈ 1.8 ms/frame) | fade 0 ms, first use still 36 ms | as B (+237 MiB per Split/Stutter chain) | moderate |
| E. compositor-wide recycled spare pool | first-ever creation still 36 ms; later chains 0 | bounded by pool size (+237 MiB per pooled ring) | high (ownership across chains/decks, resize) |
| F. one `GL_TEXTURE_2D_ARRAY` per ring (single `glTexImage3D`, `glFramebufferTextureLayer` per write; GL 4.1 has both) | ~1-3 ms (INFERRED, not measured) | unchanged | high: the Screen Split / Frame Stutter shaders in `EmbeddedShaders.h` move to `sampler2DArray` |
| G. size the ring to the effect's reach (Frame Stutter reads <= 30 frames back: 31 cells) | Stutter-only chain ~2.3 ms (INFERRED linear) | Stutter-only 15 MiB | moderate; a live param increase must grow the ring, which is a new hitch at grow time |

---

## found_not_fixed
1. **The beat clock loses time** under message-thread or tracker stalls. Seen as 0.5 s after a 0.55 s stall (load ~15) and 0.155 s after a 0.14 s stall (quiet, pre-canvas). Every later routine event is late by that amount. `RecorderClock.cpp:57` counts beats only on wraps. Options:
   - integrate the beat from the tick gap x bpm when the gap exceeds half a beat;
   - carry an unwrapped beat count in FeatureSnapshot.

   Engine and tracker semantics are out of this lane's scope. Evidence: `anomalies/`.
2. **A routine start (or loop return) whose restore fires holds the message thread 38-86 ms** (UI hitch at every such start). Starts without a restore: 9-22 ms. The cause is not isolated; no profiler is allowed.
3. **Loaded-run stack anomaly (loadpost1, load ~15):** Probe Routine ran past its first move (position 1.07 -> 6.6) while stacked on Probe Ramp, but L0 opacity stayed 1.0 for 3.2 s (never 0.5). Only "earlier ramp stays quiet" FAILED; the new "later restore wins" row passed correctly. Same run as the one-beat clock slip. Needs its own look. Evidence: `anomalies/loadpost1-stack.json`, `logs-loaded/loadpost1-1.log`.
4. **Capture cost (loose end 4):** a 1080p render_frame is 98-159 ms round trip (panel-size pre-canvas: 46-83 ms). It does not stall the message thread. Options unchanged (plan4 F4: PBO async readback; drop the per-pixel `setPixelColour` loop for a row memcpy + swizzle). Renderer.cpp was left alone as instructed.
5. **Rows left on wall-time anchors** (by design, they check the grid in wall time): 7 grid, 5g glide, 8g glide/landing/refire, 9g, 10 "start from now" (T4 lag 0.002-0.054 s; its window ends 0.1-0.15 s before the move). Rows 7 and 8g fail when the clock slips, which is correct: that is an app defect.

## FILES CHANGED
- `.harmony/probe-routines.sh`: helpers `LEN`, `MARGIN_BEATS`, `rpos`, `pos_window`, `first_move_beat`. Rows 8 (hold), 11 (later restore wins), 11j (hard cut, restored, loop hold, loop landing) use routine position; stack/jump take `$TAKE_FOLDER`; header note added.
- `.harmony/notebook.md`: correction of the "slower captures cause edge flakes" line, the position-window rule, the clock-slip finding, and the staleness-from-clockBeat method.
- `.harmony/.reports/s-rta-0927/routines-timing.md` + `routines-timing-evidence/`: this report, scripts, logs, analysis, replays, T2 data.

## PACKET QUALITY
- Clarity: CLEAR.
- Missing context:
  - main's build config (Release, TEST_SERVER and SYPHON ON) is not in the packet; I read it from `build/CMakeCache.txt`.
  - The packet's `cmake -S .. -B build-lane` has no build type; I matched main's.
- Unused context: none.
- Self-brief files: HANDOFF ledger (useful); notebook canvas entry (useful: 1080p capture ~150-180 ms figure; its causal "methods" line was wrong and is corrected); pitfalls 20 (useful).

## RIG
- Lock held per run and released between runs. At the end I do not hold it (outputs-c2 does), and no Audio-DNA I launched is running: the one running at the end is outputs-c2's build from worktree w5.
- `open -g` only. The Output window was never opened: every run's Quartz witness reads 0 Output windows.
- No lldb, Instruments or dtrace; no screenshots.
- `.venv` symlink removed before every commit. `build-pre` deleted; `build-lane` kept (untracked).
