STATUS: PARTIAL

# g4cpu lane report (s-rta-0929) -- plan-g4cpu.md + HARMONY ADOPTION G1-G10

RESULT: c1 (attribution, TEST-ONLY) is DONE and committed (ad35de1). Everything after c1 is STOPPED by the plan's own
pre-registered rule 3.3 (iii): the RED margin is 7.7 ms/s, the rule needs 8. c2 was NOT committed.
The c1 numbers settle the three conditional levers. c3 is not built (ruling G1). c4 is filed: the dirty span of the bar
is 1.00, so it would save about 0 ms/s. c2b is filed: LayerInspector repaints are 0/s.
I measured c2 once, as an experiment. I did not commit it, and I reverted it. It takes 21.7 ms/s off g4 in the same
invocation, which is 5-9 times more than c1's arithmetic predicted. It leaves g4 at idle + 28.6 ms/s. But the g4 CPU of one
binary moves by about 8 ms/s from one invocation to the next on this rig, so a relative CPU bar would have teeth only
about the size of that noise. Harmony has to rule on this (section 6).

FACTS (disk-cited):
- c1 commit ad35de1 on lane/g4cpu (`git -C <WT> log --oneline -3`); the final tree = ad35de1 + this report.
- G_red (the c1 binary, one invocation, 5 + 5, 10:02-10:14): i1 median 117.2, g4 median 159.9 -> 42.7 ms/s
  (scratchpad g4cpu/batch_gred.log).
- c2 experiment (10:21-10:33, one invocation): g4 on c2 145.9 vs g4 on c1 167.6 (-21.7); i1 on c2 117.3 -> gap 28.6
  (scratchpad g4cpu/batch_c2x.log; the c2 draft is saved at scratchpad g4cpu/c2draft/).
- a1 on the c1 binary (CPU time, 10:15-10:19): scratchpad g4cpu/batch_a1b.log (tables in section 2).
- ctest serial 920/920 on c1 at 10:02 and at the end (12:53, 19.60 s).
- Existing probes on the final (c1) app (G10): probe-idle-paint default rows GREEN except one v2b I4 line in F4 (max 3).
  The flake check passed 10/10 (5 on c1, 5 on main). probe-routines 109/0, probe-routine-display 16/0,
  probe-deck-tabs 6/0 (scratchpad g4cpu/F1-F8.log).

METHOD: I added the c1 witnesses and built them. The a1 row ran first on main (RED: "absent"), then on c1. Then I took
G_red from i1 + g4 run together in one invocation, derived the bar with rule 3.3 as amended by G2, and tested the c1
arithmetic by measuring c2 directly. Finally I re-ran every existing probe on the final app (G10). Perf numbers were taken
only under acquire_quiet_lock, and every launch prints the load.

CONFIDENCE+VERIFY: HIGH that c4 and c2b get ~0 (every SignalBar tick changes the whole span; the LayerInspector counter
reads 0). HIGH that c2 cuts g4 by 14-22 ms/s (two invocations: 159.9 / 167.6 -> 145.9). MEDIUM on any relative bar,
because g4 on the same c1 binary read 159.9 / 167.6 / 166.9 across three invocations. Verify: the raw lines in section 3.

UNKNOWNS-NOT-DONE: c2 (commit, ctest, g4 gate, v5), the c5 docs, and the LOOK list are all not done (STOP). The
~700 ms/s outlier launches (R2) are unexplained. The per-pass AppKit / CA cost is inferred from the c2 experiment, not
measured directly.

NUANCE: The c1 ranking's c2 arithmetic assumed a 0.3 ms AppKit base per pass. The experiment says a removed pad-only pass
saves about 1.2 ms of main-thread CPU. JUCE never runs MainComponent::paint for those passes: the opaque pad covers the
whole clip. So their cost is invisible to a JUCE-time log. The count of display passes is what predicts CPU.

HANDOFF-NEEDS: a Harmony ruling on c2 + the g4 gate (section 6); Harmony appends the notebook notes and work-log rows
(section 7).

INBOX-RECHECK: none

---------------------------------------------------------------------------------------------------------------------
## 1. What landed (commit ad35de1, "test(s-rta-0929 g4cpu): attribution ...")
TEST-ONLY witnesses, with no behaviour change. All the new timing sits under `AUDIODNA_TEST_SERVER`. The counter
increments are relaxed atomics, the same pattern the existing idlepaint counters use.
- `src/ui/UiPaintCounters.h`: new counters. `uipaint::bump(counter, Src)` also stamps the source bit into
  `pendingSources` (G3). `threadCpuUs()` is the main thread's CPU time (0 off Apple). Under TEST_SERVER there is a
  512-entry pass ring {t, rect, JUCE CPU us, JUCE wall us, source bits}, a 512-entry SignalBar mask ring, the geometry
  rects, and `passBegin` / `passEnd`.
- `RoutinePad` counts repaints, sweep ticks (the painted sweep width changed), paints, and sweep paints (the I2-style
  witness). `LayerStrip` counts fader repaints (the snapped value moved), V-fader paints in `OpacitySliderLookAndFeel`,
  and band paints (the clip hits a playing band). `TopBar` counts wheel repaints and gains `getWheelRepaintBounds()`.
  `DeckView` counts the corner repaint and gains `getRoutinePadRowBounds()` / `getStripColumnBounds()`.
  `LayerInspector::refresh` and `UniversalParamControl::updateValueDisplay` count their repaints.
  `SignalStrip::updateValue` returns bool: the painted key {value, peak while shown, flash while shown} changed. The
  `SignalBar` tick counter and mask are new; the bar still repaints whole. The existing ClipInspector / transport / band
  sites now go through `bump`.
- `MainComponent`: `paint` calls `passBegin`. A new `paintOverChildren` calls `passEnd` (its body is empty without
  TEST_SERVER). `recordUiGeometry` adds 5 rects. These are three small, separate hunks; the asyncload lane edits
  MainComponent.cpp too, so they are kept minimal.
- `NativeLayerHost.mm` (DEVIATION, TEST_SERVER only): the layer draw CPU / wall time. The plan's increment list does not
  name it, but without it the SignalBar layer's cost is invisible to the pass log, and c4's arithmetic needs it.
- `ApiServer`: `ui_paint` gains 15 counters, 4 layer-time fields and 5 rects. New `GET /api/debug/ui_passes` (HTTP
  thread, ring reads only).
- `.harmony/probe-idle-paint.{py,json}`: row `a1_attribution` (INFO, `"a1": {"launches": 3}`) plus the `PassLog` drain in
  `idle_window(passes=True)`. `.harmony/APP-INVENTORY.md`: one sentence.
Other deviations:
- The pass and layer times are thread CPU time (`CLOCK_THREAD_CPUTIME_ID`), with wall time kept beside them. My first a1
  batch used wall time, and wall time under the other lanes' compile load was preemption-dominated.
- The pad paint counter counts all 8 pads.
- idle.json is dumped without indent (it now holds the pass lists).

## 2. a1 -- the attribution (c1 binary, quiet, CPU time; 10:14:59-10:19:24, load 5.5-7.8)
RED on main (pre-change app, 09:50:52): `INFO  a1_attribution: absent -- GET /api/debug/ui_passes is not in this build (predates s-rta-0929 g4cpu)`
Raw lines from c1 (verbatim; two long field lists are elided with "..."):
```
INFO  a1_attribution i1 (card): cpu_main median 107.6 ms/s | display passes 14.9/s (MainComponent paints 14.87/s; paint() skipped on 0.0/s) | JUCE paint 6.2 ms/s CPU (6.8 wall) | layers (CPU / wall ms/s): SignalBar 29.22/s 36.09 / 42.18, waveform 29.18/s 6.12 / 7.19 | cpu - JUCE peer - layers = 59.2 ms/s (AppKit / CA / timers / engine)
INFO  a1_attribution i1 (card) sources/s: pad repaints 0.00 (...) | wheel 14.87 (TopBar paints 14.87) | corner 0.00 | LayerInspector 0.00 | param control 0.00 | ClipInspector 0.00 | transport 0.00 | SignalStrip changes 269.75 over 29.21 bar ticks
INFO  a1_attribution g4 (routine): cpu_main median 168.6 ms/s | display passes 35.7/s (MainComponent paints 26.38/s; paint() skipped on 9.3/s) | JUCE paint 23.5 ms/s CPU (25.5 wall) | layers (CPU / wall ms/s): SignalBar 28.86/s 31.65 / 35.64, waveform 28.86/s 5.06 / 5.72 | cpu - JUCE peer - layers = 108.4 ms/s (AppKit / CA / timers / engine)
INFO  a1_attribution g4 (routine) sources/s: pad repaints 28.86 (paints 46.63, sweep ticks 11.41, sweep paints 11.40) | fader repaints 22.92 (V paints 22.92) | band repaints 28.61 (paints 44.77) | wheel 14.59 (TopBar paints 14.58) | corner 0.00 | LayerInspector 0.00 | param control 0.00 | ClipInspector 0.00 | transport 0.00 | SignalStrip changes 261.51 over 28.86 bar ticks
INFO  a1_attribution g4 (routine) by rect class (...): padrow+stripcol+deck: 11.8/s 684/1925 us 11.7 ms/s | wheel+padrow+stripcol+deck: 3.1/s 1074/2861 us 5.3 ms/s | wheel+padrow+deck: 4.7/s 523/1408 us 3.7 ms/s | wheel: 6.8/s 239/1464 us 2.8 ms/s | padrow+deck: 9.3/s absent/absent us 0.0 ms/s [837 skipped]
INFO  a1_attribution g4 (routine) by source class (G3) (...): pad+band: 6.1/s 639/1786 us 5.5 ms/s | pad+wheel: 5.1/s 523/1407 us 3.7 ms/s [38 skipped] | pad+fader: 3.2/s 728/2038 us 3.6 ms/s | pad+fader+wheel: 2.2/s 1050/2769 us 3.3 ms/s | wheel: 5.6/s 240/1480 us 2.4 ms/s | pad+band+wheel: 1.2/s 1366/3201 us 2.2 ms/s | pad+fader+band: 1.9/s 701/1885 us 1.7 ms/s | pad+fader+band+wheel: 0.4/s 1074/2858 us 0.6 ms/s | none: 1.2/s 137/557 us 0.4 ms/s [2 skipped] | pad: 8.8/s absent/absent us 0.0 ms/s [797 skipped]
INFO  a1_attribution g4 (routine) G3 cross-check: passes over the pad row without a pad request 0.0/s (union sweeps) | pad requests whose pass missed the pad row 0.0/s | c3 record (G1, not built): passes spanning the wheel AND the deck 7.8/s by rect, 8.9/s by source (wheel + a deck source)
INFO  a1_attribution lever c2 (pad paint key): pad repaints 28.9/s -> key changes ~11.9/s (removes 59 %); pad-only passes 8.8/s (0.0 ms/s JUCE + 0.3 ms base each); union growth by the pad in shared passes 1.4 ms/s -> saving ~2.4 ms/s at g4, 0 at i1
INFO  a1_attribution lever c4 (i1): SignalBar ticks with a change 2631/2631, mean dirty span 1.00 of the bar | layer 36.09 ms/s measured -> saving ~0.0 ms/s (the plan's compclass scaling: ~0.0 ms/s)
INFO  a1_attribution lever c4 (g4): SignalBar ticks with a change 2599/2599, mean dirty span 1.00 of the bar | layer 31.65 ms/s measured -> saving ~0.0 ms/s (the plan's compclass scaling: ~0.0 ms/s)
INFO  a1_attribution lever c2b (i1): LayerInspector repaints 0.00/s, passes carrying it 0.00 ms/s JUCE (c2b is built iff >= 2 ms/s in i1 or g4)
INFO  a1_attribution lever c2b (g4): LayerInspector repaints 0.00/s, passes carrying it 0.00 ms/s JUCE (c2b is built iff >= 2 ms/s in i1 or g4)
```
The a1 routine arm's r2 read 708.4 ms/s (see R2). The cpu medians are robust to it; the pooled pass tables include it.

Checking the 2.2 predictions against the measurements:

| prediction | measured |
|---|---|
| pad repaints 27-30/s | 28.86 |
| fader repaints 22.5/s | 22.92 |
| band repaints 28.7/s | 28.61 |
| wheel 14.9/s | 14.59 |
| corner / inspector / param control 0 | 0 / 0 / 0 |
| passes 27-30/s | 35.7/s. The extra 9.3/s are pad-only passes whose MainComponent::paint JUCE skipped (the clip lies inside the opaque pad). The existing `main_component_paints` counter (26.4/s) never counted them. |
| "padrow+stripcol+deck(+wheel)" 13-15/s at 1.0-1.4 ms if the timers are locked | 14.9/s (11.8 + 3.1) at median 0.68 / 1.07 ms CPU. The wheel coincides 7.8/s: partly locked. |
| i1: wheel 14.9/s alone at ~0.45-0.5 ms | 14.9/s, median 0.24 ms CPU (p90 1.46) |

The lever decisions, as the c1 table decides them:
- c3 is NOT built (ruling G1). Recorded for a future lane: passes spanning the wheel AND the deck run 7.8/s by rect and
  8.9/s by source. The plan's own criterion (>= 8/s) is borderline met by source and not met by rect.
- c4 is FILED. The rig's live input moves every strip on every tick (262-270 painted-key changes/s over 29 ticks x 14
  strips), so the dirty span is 1.00 of the bar and the saving is ~0 < 5 ms/s. The G4 stale-strip gate was therefore
  not built either. With a silent input the picture would differ, but Boris's show is music: the plan's R5 case.
- c2b is FILED: LayerInspector repaints are 0/s in both arms (the fixture keeps the Clip tab active).

## 3. The bar (3.3 + G2) -- STOP by rule (iii)
- G_red on the c1 binary, one invocation (5 + 5), raw:
  `PASS  i1_idle_card (IDLE-HB-card): window max median 4.0 ms [3.7-4.2] (<= 8.0) AND main CPU median 117.2 ms/s [114.9-118.6] (<= 150.0)`
  `PASS  g4_routine (a loop routine on 3 layers, adoption I3): window max median 5.0 ms [4.7-5.2] (<= 8.0) | main CPU median 159.9 ms/s [158.2-169.1] (INFO, ruling J2)`
  `INFO  g4_routine CPU (INFO, ruling J2): main-thread CPU median BEFORE 160.6 ms/s [159.9-161.2] | AFTER 159.9 ms/s [158.2-169.1] (i1's 150.0 ms/s is not applied to g4)`
  -> G_red = 159.9 - 117.2 = **42.7 ms/s**. Main's own g4 arm reads 160.6, so the c1 counters cost nothing measurable.
- Rule 3.3 with c1's arithmetic: saved(c2) ~2.4 -> residual 40.3 > 30 -> (ii) STOP.
- That arithmetic's weakest input is the 0.3 ms per-pass base, so I tested it by measuring c2 directly. This was an
  experiment: the c2 patch plus a separate c1 app copy as BEFORE, one invocation (10:21-10:33, load 4.6-8.5). It was
  not committed, and the tree was restored from HEAD afterwards. Raw lines:
  `PASS  i1_idle_card (IDLE-HB-card): window max median 4.5 ms [4.2-4.6] (<= 8.0) AND main CPU median 117.3 ms/s [113.9-122.6] (<= 150.0)`
  `PASS  g4_routine (a loop routine on 3 layers, adoption I3): window max median 5.2 ms [5.0-29.9] (<= 8.0) | main CPU median 145.9 ms/s [142.9-708.3] (INFO, ruling J2)`
  `INFO  g4_routine CPU (INFO, ruling J2): main-thread CPU median BEFORE 167.6 ms/s [158.2-723.7] | AFTER 145.9 ms/s [142.9-708.3] (i1's 150.0 ms/s is not applied to g4)`
  Per-launch medians (from idle.json):

  | counter | c2 | c1 |
  |---|---|---|
  | pad repaints/s | 11.56 | 29.68 |
  | pad paints/s | 22.9 | 43.2 |
  | MainComponent paints/s | 19.5 | 27.0 |
  | sweep paints / ticks | 11.42 / 11.42 | 11.40 / 11.42 |
  | band paints/s | 36.1 | 44.1 |
  | V-fader paints/s | 22.9 | 22.9 |

- G2's re-derivation on the final (c2) numbers: residual = the c2 gap 28.6 -> bar = min(36, round(28.6 + 6)) = 35.
  Rule (iii) then asks for G_red > bar by >= 8. 42.7 - 35 = **7.7 < 8 -> STOP**.
- The honest reading: the teeth equal the rig's drift. The same c1 binary's g4 read 159.9 (gred), 167.6 (c2x BEFORE)
  and 166.9 (F2) in three invocations, while its i1 read 117.2 / 115.3. If I took the c2x invocation's own c1 arm as
  G_red, I would get 50.3 and a margin of 15.3. But the pre-registered G_red is the first same-invocation pair, and
  moving it would be a re-threshold.
- One more finding that blocks c2 as written: the G7 cadence rows with the plan's bounds, read on paint counts, FAIL on
  the c2 binary. Pad paints are 22.9/s against a bound of [8, 16]: the counter counts every pad's paint (pad 2 is inside
  the wheel + strip unions) and the union sweeps too. Band paints are 36.1/s against [20, 36]. The plan's bounds are the
  fixture's REQUEST constants. On paint counts they need a Harmony decision: either bounds derived from paint counts, or
  a counter restricted to the playing pad.

## 4. RISKS / findings
R1. The g4 gap is mostly passes, not paint. c1 shows +20.8 passes/s and +17.3 ms/s of JUCE peer paint (CPU) for a gap of
    42.7-61. The rest is AppKit / CA per pass plus the routine engine. A pass whose JUCE paint is skipped still costs
    about 1.2 ms (c2's measured -21.7 ms/s for -18 pad requests/s; INFERRED from the experiment).
R2. Three of the ~22 routine-fixture launches read 708-724 ms/s main-thread CPU with a window max of about 30 ms (a1b r2
    on c1; c2x AFTER r5 on c2 and BEFORE r1 on c1, back to back at ~10:28-10:30). None of the ~30 card launches did,
    and none of the main-build launches did. The cause is UNKNOWN. My guess (INFERRED, not verified) is the main thread
    placed on an efficiency core under machine load. A stray `yes` process (pid 83720, orphaned under launchd, 99 %
    CPU, started ~05:02, before this session) was running the whole time. It is NOT mine and I did NOT kill it.
    Harmony may want to.
R3. The first a1 batch (09:50, compilers running) was discarded: i1 read 298-386 ms/s, 4 of 6 launches tainted. Every
    number above rode on the stray `yes` and on two other lanes' work.
R4. The c1 pass log times a pass only when MainComponent::paint runs (a pad-only pass has us = -1). The rect and source
    classes still count every pass.

## 5. Final-app probes (G10: re-run, never re-thresholded) + ctest
probe-idle-paint on c1 (build-lane), default rows:

| batch | time | rows | result |
|---|---|---|---|
| F1 | 10:40-10:48 | c0 PASS x3; i1; g1 PASS; i2 | GREEN |
| F2 | 11:00-11:07 | g2 PASS x3 (148 / 148 = 1.000); g4 | GREEN |
| F3 | 11:25-11:32 | g5 INFO 114.6 ms/s; v0 PASS x2; v1 S1-S4 PASS (0 px differ); v1n INFO | GREEN |
| F4 | 11:43-11:46 | v2 PASS x6; v2b I1 PASS, teeth PASS, I4 FAIL; v3 PASS; v3p PASS (0 px); v4 INFO | RED on the I4 line only |

Raw lines:
- F1 i1: `PASS  i1_idle_card (IDLE-HB-card): window max median 4.2 ms [3.9-4.3] (<= 8.0) AND main CPU median 115.3 ms/s [110.6-117.6] (<= 150.0)`
- F1 i2: `PASS  i2_idle_many16 (IDLE-HB-many16): window max median 4.3 ms [3.8-4.4] (<= 8.0) AND main CPU median 120.3 ms/s [114.6-123.3] (<= 150.0)`
- F2 g4: `PASS  g4_routine (a loop routine on 3 layers, adoption I3): window max median 5.0 ms [4.7-5.4] (<= 8.0) | main CPU median 166.9 ms/s [163.5-174.2] (INFO, ruling J2)`; BEFORE (main) 166.2.
- F4 v2b: `FAIL  v2b_fallback_frames (I4): the layer is back <= 2 vblanks after the overlay closed (max 3)`

The I4 flake check (F8, 12:37-12:45): 5 runs on c1 and 5 on main, alternating, one lock hold. All 10 are
`PASS  v2b_fallback_frames (I4)`. Restore max per run: c1 2 / 1 / 1 / 1 / 2, main 2 / 2 / 2 / 2 / 1. I1 and teeth PASS
10/10 (covered 10-13). Verdict: FLAKE, 1 of 6 runs on c1. It is the same class idlepaint.md recorded ("restore max 3
once in 10 cycles" on a pre-5db815f binary). c1 does not cause it.

Other probes:
- probe-routines (ROUTINES_BUILD_DIR=build-lane, ROUTINES_RECORD_PAUSE=1.8, 12:01): `109 PASS / 0 FAIL`.
- probe-routine-display (build-lane, 12:05): `16 PASS / 0 FAIL`.
- probe-deck-tabs (build-lane, 12:18): `6 PASS / 0 FAIL`.
- ctest serial (`ctest --test-dir build-lane -j1`, 12:53:32, under the lock): `100% tests passed, 0 tests failed out of 920` (19.60 s).

Screen safety:
- After every batch: `audio-dna windows 0, Output-named 0` and `UserNotificationCenter windows on screen: 0`.
- No Output window was opened by any path. There was no synthetic input, and I used no debugger / sample.
  test_output_window_level.py was never run.
- I looked at a sample frame (v1 after-S2, test mode, card fixture): normal UI, the ROUTINES row, no Output window.

## 6. For Harmony -- the ruling needed (the 3.3 (ii) options, with data)
(a) Land c2 with the g4 CPU as an INFO line (J2 stays). c2 takes 14-22 ms/s off g4 (idle + 28.6 in the same invocation),
    is pixel-identical by construction, and comes with its ctest drafted. The G7 cadence bounds would need re-deriving
    for paint counts, or the pad-paint counter narrowed to the playing pad (section 3). The v5 row is drafted.
    Estimated at about one short lane.
(b) Land c2 with the g4 CPU gate at bar 35 (G2 re-derived). Its teeth are 7.7-15.3 ms/s, depending on which c1 arm is
    taken, and that equals the rig's between-invocation drift (~8). Expect a flaky gate.
(c) Option B, or a look change. c1 shows passes as the currency. After c2, the remaining ~29 ms/s is ~20 pixel-change
    ticks/s of sweep, hairline and V fill, each paying the ~1 ms pass cost, plus the engine.
Drafts ready in the scratchpad (not in the tree):
- `g4cpu/c2draft/`: the RoutinePad c2 patch + files.
- `g4cpu/test_routine_pad_paint_key.cpp`: plan 2.3 (a)-(e).
- `g4cpu/g4_row.py`: the G2 / G7 g4 gate.
- `g4cpu/v5_row.py`: the v5 routine identity. It needs `preview_rect` added to recordUiGeometry, to mask the preview's
  opacity render.

## 7. Notes for Harmony to append
Notebook (.harmony/notebook.md):
- A JUCE-time pass log undercounts what a display pass costs. JUCE skips MainComponent::paint (and any parent's paint)
  when opaque children cover the clip (juce_ComponentHelpers.h `clipObscuredRegions` recurses), so a pass inside one
  opaque widget (a RoutinePad) has no JUCE time. Yet removing ~18 such pad passes/s saved ~21.7 ms/s of main-thread
  CPU: ~1.2 ms per pass in AppKit / CA. Count passes (`paintOverChildren` always runs), not JUCE paint time.
  | src/ui/UiPaintCounters.h passEnd | s-rta-0929 g4cpu
- Main-thread CPU (ps -M utime + stime) of the same binary and fixture drifts ~8 ms/s between probe invocations on the
  shared rig (g4 on c1: 159.9 / 167.6 / 166.9). A relative CPU bar needs teeth well above 8 ms/s, or several invocations.
  | .harmony/probe-idle-paint.py gate
- Paint-execution counters include union sweeps, and RoutinePad's includes all 8 pads (pad 2 is swept by the wheel +
  strip unions). A cadence bound set from repaint REQUEST rates fails on paint counts (c2: pad paints 22.9/s vs requests
  11.6/s). Filter to the playing pad, or derive the bounds from measured paints. | src/ui/RoutinePad.cpp paint
- A stray orphaned `yes` (pid 83720, since ~05:02 on 09-29) ate one P-core all session; the perf rows rode on it.
Work-log rows (.harmony/s-rta-0929-work.md):

| t | kind | item | result |
|---|---|---|---|
| 09:18 | build | g4cpu STEP 0 (lane/g4cpu from main adf9b8a; build-lane with FETCHCONTENT from main build/_deps) | full build 09:31-09:50 |
| 09:57 | probe | a1 batch 1 (c1 wall time) | DISCARDED: compilers ran, i1 298-386 ms/s tainted |
| 10:14 | probe | c1 ctest + i1 / g4 (5 + 5, one invocation) | 920/920; G_red 42.7 (117.2 / 159.9); main g4 160.6 |
| 10:19 | probe | a1 on c1 (CPU time) | c4 span 1.00 -> filed; c2b 0/s -> filed; c3 record 7.8 / 8.9 coincident passes/s |
| 10:20 | commit | c1 ad35de1 | test-only witnesses + a1 |
| 10:33 | probe | c2 EXPERIMENT (uncommitted, reverted) | g4 145.9 vs c1 167.6; gap 28.6; bar 35 -> 3.3 (iii) margin 7.7 < 8 -> STOP |
| 12:45 | probe | final-app probes (G10) + v2b I4 flake check | idle-paint GREEN except the v2b I4 flake (10/10 PASS on re-run); routines 109/0; display 16/0; deck-tabs 6/0 |
| 12:53 | check | final ctest serial | 920/920 |

## PACKET QUALITY
- Clarity: HAD_TO_INFER. Once c1's arithmetic is shown wrong, rule 3.3 (ii) / (iii) and G2 ("re-derived on the final
  app's numbers") point different ways. I applied both, and both say STOP. G7 said "paint counts" but carried request-rate
  bounds.
- Missing context: the per-pass AppKit cost, and the fact that pad-only passes skip JUCE paint (c1 found both). The stray
  `yes` load.
- Unused context: the c3 / c4 build specs (not built, per G1 and the c1 numbers).
- Self-brief files: plan-g4cpu.md + adoption, both attack files, probe-idle-paint.{sh,py,json}, idlepaint.md (the v2b
  history), and the UI sources. All useful.
