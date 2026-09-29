STATUS: DONE
(fix round 1 at the end supersedes the STOP below: c2 landed per addendum 2 G11-G15)

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


---------------------------------------------------------------------------------------------------------------------
## Fix round 1 (g4cpu-fix, answering critic-g4cpu-r1) -- full report: .harmony/.reports/s-rta-0929/g4cpu-fix.md
- MUST 1 ("CPU drops" is false at HEAD): correct about the tree. This lane makes NO CPU claim; c2 is still STOPPED
  pending section 6. The look claim is reframed as: the TEST-ONLY instrumentation (c1 + g4cpu-fix) changes no pixels.
- MUST 2 / SHOULD 5: v5_routine_identity is built (commit 2df6876, a default row; TEST_SERVER-only `preview_rect`).
  RED on main: teeth "absent". GREEN (this build vs main): P1 (6.5 beats) and P2 (10.5) 0 px differ, max delta 0.
  The routine cue is present in all 4 frames (12213 / 14973 px). The teeth show 9 clusters, all in the pad row (sweep,
  digit) or the strip column (V fill tops, hairlines). Region diff PNGs are in scratchpad g4cpu-fix/evidence/.
- Not done: the bound-knob region. Showing a routine-held UniversalParamControl needs a layer selection, which has no
  REST / composition path; it needs a TEMPORARY hook in both builds. c1's only change there is one counter line after
  repaint().
- SHOULD 3 (a synthetic G3 test): conditional on a future hard gate, not built. SHOULD 4: no fix required.
- ctest serial 920/920 (13:52). Screen safety: 0 Output windows, 0 dialogs after every batch.

---------------------------------------------------------------------------------------------------------------------
## Fix round 1 (g4cpu-fix1, 14:05-16:07) -- HARMONY ADOPTION ADDENDUM 2 (G11-G15) implemented
STATUS: DONE. Branch lane/g4cpu, continued from c1d216c (no reset / rebase). Commits: 8e7fb2e (G11), f19d5ba (G12),
00bb672 (G15), and this report. The BEFORE app for every RED run and for the interleaved arms is a copy of the app built
from c1d216c (the build was a no-op at HEAD: binary sha256 56fbb898...; copy at scratchpad g4cpu-fix1/c1dapp). The c2
app is build-lane at 8e7fb2e (sha256 4c058af7..., copy at g4cpu-fix1/c2app). The docs and report commits do not change
the binary.

RESULT: c2 is landed. A ROUTINES pad now repaints only when what it paints changes (`RoutinePad::paintKeyOf`). On
interleaved launches (c1 and c2 alternating, 6 per arm, one invocation), g4 went from 176.0 to 155.4 ms/s of main-thread
CPU (paired per-round median -21.6). The idle card stayed at 116.4 vs 117.5 (paired +0.7). As G11 ruled, the CPU stays
an INFO line. The new cadence rows are RED on c1d216c and GREEN on c2. v5 reads 0 px differ at both beat positions on
c2. Every existing probe is GREEN on c2. ctest serial is 925/925.

### Table (ruling -> commit -> RED line -> GREEN line)
| ruling | commit | RED (verbatim) | GREEN (verbatim) |
|---|---|---|---|
| G11 c2 + ctest | 8e7fb2e | build of test_routine_pad_paint_key on the c1d216c tree: `tests/test_routine_pad_paint_key.cpp:47:23: error: no member named 'paintKeyOf' in 'RoutinePad'` | `100% tests passed, 0 tests failed out of 11` (-R paintKeyOf\|RoutinePad: the 5 new + 6 existing); full serial `100% tests passed, 0 tests failed out of 925` (15:40) |
| G11 teeth (scratch copies; the deliverable's sha256 89451c56... is unchanged before and after) | -- | drop `k.bar`: `test cases:  5 \|  3 passed \| 2 failed`; drop `k.restartPending`: `test cases:  5 \|  4 passed \| 1 failed` | (the real key) 5 / 5 pass |
| G12-1 pad requests | f19d5ba | c1d216c app: `FAIL  g4_routine cadence (G12-1): ROUTINES pad repaint requests/s median 29.2 [29.2, 29.2, 29.2, 29.2, 29.3] in [8.0, 16.0]` | c2 (F2 15:17-15:25): `PASS  g4_routine cadence (G12-1): ROUTINES pad repaint requests/s median 11.6 [11.6, 11.5, 11.6, 11.6, 11.6] in [8.0, 16.0]` |
| G12-2 sweep | f19d5ba | main (pre-g4cpu): `FAIL  g4_routine cadence (G12): the g4cpu paint / change counters are absent from this build` (c1d216c: PASS 1.000 -- c1 misses no update, so this row's RED is by absence) | `PASS  g4_routine cadence (G12-2): routine_pad_sweep_paints / routine_pad_sweep_ticks = 57.09 / 57.09 per s summed over 5 launches = 1.000 (>= 0.95)` |
| G12-3 band | f19d5ba | main: the same absent line (c1d216c: PASS 1.584) | `PASS  g4_routine cadence (G12-3 band): layer_strip_band_paints / layer_strip_band_repaints = 177.98 / 143.70 per s summed over 5 launches = 1.239 (>= 0.95)` |
| G12-3 V fader | f19d5ba | main: the same absent line (c1d216c: PASS 1.000) | `PASS  g4_routine cadence (G12-3 V fader): layer_strip_fader_paints / layer_strip_fader_repaints = 114.42 / 114.42 per s summed over 5 launches = 1.000 (>= 0.95)` |
| G11 v5 on c2 | (row from 2df6876, unchanged) | (2df6876: main vs main -> teeth "absent") | `PASS  v5_routine_identity P1 (routine position 6.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)` / `PASS  ... P2 (routine position 10.5 beats): ... 0 px differ (max delta 0) ...` / teeth `9 violation cluster(s): 3 in the pad row (bbox 40 pt wide, >= 15), 6 in the strip column (>= 1), 0 elsewhere` |
| G11 CPU (INFO) | -- | interleaved c1 arm: g4 176.0 [172.5-183.4], i1 116.4 [115.2-117.6] | c2 arm: g4 155.4 [153.2-156.2], i1 117.5 [114.9-118.9] |
| G13 | -- | -- | diff stat below |
| G14 | -- | -- | 26 clean interleaved launches, no outlier; the phase of the earlier outliers is named below |
| G15 docs | 00bb672 | -- | 57 amended, NN appended, architecture.md and recording.md updated; CLAUDE.md untouched (24,980 B) |

### G11 -- c2 landed (8e7fb2e)
- I re-checked the draft against HEAD c1d216c before applying it: `git apply --check` passes, and the applied files are
  byte-identical to c2draft/RoutinePad.{h,cpp}. The key holds every field that `paint` / `paintContent` read: number,
  name, state, onShownDeck (the 50 % layer), loop, warning, restartPending, bar, barsTotal, and progress01, which enters
  only through sweepW at `getWidth()`. The tooltip is compared separately, as before. No other caller of
  `samePad` / `sweepWidthOf` exists (grep).
- ctest `tests/test_routine_pad_paint_key.cpp` [g4cpu][pad] has 5 cases (plan 2.3 (a)-(e)) and is registered at the EOF
  of tests/CMakeLists.txt (the test_routine_pad_press link set). I reconfigured after the CMake change.

### G11 -- the CPU effect from INTERLEAVED arms (INFO; the c1d216c copy as BEFORE)
Driver: scratchpad g4cpu-fix1/mix.py + batch_mix.sh, the diag-vfps run_mix.sh pattern. It uses the probe's own launch /
fixture / idle window, with the pass log drained. Each round launches c1 i1, c2 i1, c1 g4, c2 g4, and even rounds put c2
first. The lock was held for 3 rounds at a time. A compiler stopped round 4 once; the wrapper resumed and re-ran round 4
whole. Times: 14:19-14:28 and 14:58-15:07. Load 5.2-12.7. The stray `yes` (pid 83720) ran throughout. Raw aggregate
(verbatim):
```
27 launches (2 tainted, kept in the list, excluded from medians)
c1 i1: n=6 cpu median 116.4 [115.2-117.6] | JUCE 6.7 SB 39.1 WF 6.5 rest 64.1 | passes 14.9/s | pad 0.00/s
c1 g4: n=6 cpu median 176.0 [172.5-183.4] | JUCE 29.2 SB 40.6 WF 6.2 rest 99.9 | passes 39.0/s | pad 29.21/s
c2 i1: n=7 cpu median 117.5 [114.9-118.9] | JUCE 6.7 SB 39.6 WF 6.6 rest 64.4 | passes 14.9/s | pad 0.00/s
c2 g4: n=6 cpu median 155.4 [153.2-156.2] | JUCE 17.9 SB 40.8 WF 6.3 rest 89.8 | passes 31.8/s | pad 11.56/s
g4: c1 176.0 -> c2 155.4 = -20.6 ms/s | i1: c1 116.4 -> c2 117.5 = +1.2 ms/s | gap over i1: c1 +59.6, c2 +37.8 | gap change (delta g4 - delta i1) -21.8 ms/s
paired per-round c2 - c1 (i1): +1.1, +0.0, +0.4, +2.3, -1.4, +2.7 | median +0.7
paired per-round c2 - c1 (g4): -16.3, -25.4, -23.9, -17.6, -19.3, -27.3 | median -21.6
```
(c2 i1 has n=7 because the first hold's clean c2_i1_r4, at 14:29:31, stays in the list. The wrapper then re-ran round 4
whole after the compiler stop.)

Every launch is printed in the per-launch list in scratchpad g4cpu-fix1/mix.log, including the 2 tainted launches (c1
i1 r4 119.3 and c2 i1 r4 119.9, neither an outlier). The list splits each launch's main-thread CPU into JUCE peer +
SignalBar layer + waveform layer + rest.

What the split says: c2's -20.6 ms/s is -11.3 of JUCE peer paint (the union reach the pad no longer adds) and -10.1 of
the "rest" (AppKit / Core Animation / timers / engine). Passes fell by 7.2/s. So a removed pass costs about 1.4 ms outside
JUCE (INFERRED from the arithmetic). This corrects my c1 report's "~18 pad passes/s x 1.2 ms". Requests fell by 17.6/s,
but most removed requests shared a vblank with another source. The layers did not change (SB 40.6 / 40.8).

The quiet a1 on c2 (F1c, 15:58-16:06) gives the AFTER attribution:
- `INFO  a1_attribution g4 (routine): cpu_main median 140.9 ms/s | display passes 30.9/s (MainComponent paints 19.45/s; paint() skipped on 11.4/s) | JUCE paint 16.1 ms/s CPU (18.0 wall) | ...`
- `INFO  a1_attribution g4 - i1 (these launches): cpu 32.3 ms/s | passes +16.0/s | JUCE peer paint +9.8 ms/s CPU | SignalBar layer +2.6 | waveform layer -0.1 ms/s CPU`
- The c3 record at c2: 4.3 / 5.3 wheel+deck coincident passes/s, by rect / by source, down from 7.8 / 8.9.

### G12 -- cadence rows (f19d5ba)
`row_g4_cadence` runs on g4's own 5 launches. It needs no extra launches and leaves the window max gate and the CPU INFO
untouched. It prints the absolute paint rates as INFO. On c2:
`pad paints (all 8 pads) 20.7, 21.5, 20.4, 20.6, 20.9 | band paints 36.0, 35.5, 35.8, 36.0, 34.8 | V-fader paints 22.9 x5`.

Limit of (3) (INFERRED): a union sweep can paint a strip whose own change request was lost, so (3) proves no update was
missed only as a lower bound. The fader change counter also counts S-fader moves; S never moves in g4.

### G13 -- the bound knob (argument, no hook arm)
`git diff --stat adf9b8a..HEAD -- src/ui/UniversalParamControl.*` gives `src/ui/UniversalParamControl.cpp | 2 +`. The
two lines are `#include "ui/UiPaintCounters.h"` and one `uipaint::bump(...paramControlRepaints...)` after the existing
`repaint()` in `updateValueDisplay` (c1). `git diff --stat c1d216c..HEAD` lists only RoutinePad.{h,cpp},
tests/CMakeLists.txt and tests/test_routine_pad_paint_key.cpp for code. The knob's paint code and its inputs (the model,
RoutineEngine, ConnPicker) are untouched, so c1 and c2 cannot move a bound-knob pixel. The only thing c2 changes is when
the pad asks for a repaint.

### G14 -- the ~700 ms/s outlier launches (No Unexplained Residue)
- It did not recur: 26 clean interleaved launches (12 routine, 14 card, both apps) all fall in 110-184 ms/s. The
  final-app batches below have none either; the highest is g4 r5 at 207.5.
- The phase is named, from the stored idle.json of the old outliers. The a1b r2 outlier (c1, 10:19, 708.4 ms/s) had
  JUCE peer paint 18.2 ms/s, SignalBar layer 15.8 and waveform layer 3.1. So about 670 ms/s sat in the "rest" phase:
  outside JUCE paint and outside the native layer draws, i.e. AppKit / Core Animation outside drawRect, timers, the
  routine engine, or the REST handlers.
- Its window max median was about 30 ms: one ~30 ms busy block in every 500 ms poll window. Its display passes
  (27.8/s) and layer draws (27.7/s vs 29.2) were slightly fewer, not more. So it was not paint-driven.
- The other two outliers (ce04R6 g4 r5 on c2, BEFORE r1 on c1) show the same signature: window max ~30 ms and
  SB / WF draws 27.6/s. They have no pass log.
- Ruled out: the binary (it happened on c1 and on c2, and not on main in ~15 routine launches), the `yes` process (it
  ran through every clean launch too), and a compiler (the launches were not tainted).
- Not established: which timer or handler held the ~30 ms block.
- Cheapest next test: if it recurs, the mix driver already records the per-launch phase split and the thread table
  (`ps -M`). One more step would split "rest": TEST_SERVER thread-CPU stamps around `MainComponent::timerCallback`,
  `RoutineEngine::tick` and `LayerStrip::timerTick`, the same `threadCpuUs()` helper as the pass log (FILED).

### G15 -- docs (00bb672)
- `docs/claude/pitfalls.md` 57: rule (2) now lists `RoutinePad::paintKeyOf`. "a routine playing on 3 layers still
  costs 163 ms/s -- ..." becomes "a routine playing on 3 layers: 176.0 -> 155.4 ms/s, idle + 38 -- s-rta-0929 g4cpu,
  Pitfall NN". A new NN entry follows 57 (Harmony numbers it).
- NN covers:
  - the paint-key rule;
  - the interleaved pass / JUCE / rest split;
  - "count display passes, not JUCE paint time";
  - the Slider snap;
  - why the CPU stays INFO;
  - the TopBar filed;
  - the witnesses and the guard.
  It cites no BORIS_DECISIONS (G5).
- `docs/claude/architecture.md` "UI Painting": paintKeyOf joins the on-change list, and ui_passes plus "a1 prints who
  repaints" join the witnesses. `docs/claude/recording.md` "Surfaces": a pad repaints on its paint key, a band on its
  hairline width.
- CLAUDE.md is untouched: 24,980 B, no index line (G9). APP-INVENTORY already carries the ui_paint / ui_passes sentence
  from c1 / g4cpu-fix, so nothing is added.

### Existing probes on the final (c2) app -- re-run, never re-thresholded
| batch | time | rows | result |
|---|---|---|---|
| F1 | 15:08-15:16 | c0 PASS x3; i1 FAIL (CPU 157.2 > 150); g1 PASS; i2 PASS 131.2 | i1 FAIL -> re-check F1c |
| F1c | 15:45-15:58 | i1 on c2; i1 on c1d216c | both PASS |
| F2 | 15:17-15:25 | g2 PASS x3 (146 / 146 = 1.000); g4 PASS (window max 4.9) + 4 cadence rows PASS | GREEN |
| F3 -> F1c | 15:58-16:06 | a1 INFO (above); g5 INFO 109.9 ms/s | (F3 at 15:26 was SKIP / TAINTED: a compiler) |
| F4 | 15:26-15:29 | v0 PASS x2; v1 S1-S4 PASS (0 px); v1n 0 px | GREEN |
| F5 | 15:30-15:33 | v2 PASS x6; v2b I1 / I4 (max 1) / teeth PASS; v3 PASS; v3p PASS (0 px x2); v4 INFO (the J1 first-pass class, 35393 px, as before); v5 PASS x4 | GREEN |
| F6 | 15:34-15:35 | probe-routines (ROUTINES_BUILD_DIR=build-lane, pause 1.8) | `109 PASS / 0 FAIL` |
| F7 | 15:36-15:37 | probe-routine-display | `16 PASS / 0 FAIL` |
| F8 | 15:39 | probe-deck-tabs | `6 PASS / 0 FAIL` |
| F9 | 15:40 | ctest serial `-j1`, under the lock | `100% tests passed, 0 tests failed out of 925` |

The F1 i1 line, verbatim:
`FAIL  i1_idle_card (IDLE-HB-card): window max median 6.3 ms [5.7-6.7] (<= 8.0) AND main CPU median 157.2 ms/s [148.5-170.5] (<= 150.0)`

The re-check, verbatim:
- c2: `PASS  i1_idle_card (IDLE-HB-card): window max median 4.5 ms [4.2-4.8] (<= 8.0) AND main CPU median 115.3 ms/s [114.9-117.6] (<= 150.0)`
- c1d216c: `PASS  i1_idle_card (IDLE-HB-card): window max median 4.6 ms [4.3-5.3] (<= 8.0) AND main CPU median 110.6 ms/s [109.9-111.6] (<= 150.0)`

Verdict: the F1 FAIL was a rig excursion, not c2. The evidence:
- The same c2 binary read 114.9-119.9 at i1 in the 7 interleaved launches 30-45 minutes earlier, and 115.3 in F1c.
- At i1, c2 only runs setSpec on idle pads whose key never changes (pad repaints 0.00/s).
- During the same window (15:17-15:25) the main binary's g4 BEFORE arm read 219.6 ms/s [201.2-230.8], against 171.5 an
  hour earlier (RED batch): every binary was inflated.
- F1 had no top-process sampler. The F1b / F1c samplers (top.log) show the ChatGPT Codex renderer at 22-40 % and other
  lanes' cmake / clang coming and going.

Screen safety: after every batch `audio-dna windows 0, Output-named 0` and `UserNotificationCenter windows on screen:
0`. No Output window was opened, there was no synthetic input and no debugger or `sample`, and
test_output_window_level.py was not run. No TEMPORARY hook was used; `strings | grep -c ADNA_TEMP_` gives 0.
I LOOKED at v5-after-P1.png (F5):
- pad "1 Sweep 2/4" is playing with its teal sweep and frame;
- a "Sweep" band sits on L1-L3, and the three V faders are in chartreuse;
- "Outputs: Off", and there is no Output window.

### LOOK list for Boris (c5, unchanged by c2 -- nothing may look different)
1. Fire a routine that spans 3 layers. The pad's teal sweep creeps and "n/4" counts. Each band's hairline creeps. Each V
   fader glides in chartreuse.
2. Press the pad again. The restart mark appears left of "n/4", and the sweep restarts on the bar.
3. Right-click the pad: the menu opens. Hover over it: the tooltip shows.

### Notes for Harmony to append (notebook), corrections included
- (replaces my c1 note "~18 pad passes/s x ~1.2 ms") Interleaved c1 / c2, 6 launches per arm, one invocation: removing
  17.6 pad requests/s removed 7.2 display passes/s. It saved 11.3 ms/s of JUCE peer paint (union reach) plus 10.1 ms/s
  of non-JUCE main-thread CPU, about 1.4 ms per pass. Count passes (`paintOverChildren`), not JUCE time. |
  scratchpad g4cpu-fix1/mix.log, src/ui/UiPaintCounters.h passEnd
- An interleaved A/B (alternating launch by launch in one invocation) resolves a 20 ms/s CPU effect with a paired
  spread of ~±5. Separate invocations drift ~8 ms/s, and on 09-29 one 15-minute window inflated every binary by
  ~40-50 ms/s (i1 157 / main g4 220). A lone CPU FAIL needs an interleaved re-check with the other binary before it
  is a finding. | .harmony/probe-idle-paint.py gate
- The 700 ms/s launches are a "rest"-phase block (~30 ms per 500 ms), not paint. | g4cpu.md fix round 1 G14

### Work-log rows (.harmony/s-rta-0929-work.md)
| t | kind | item | result |
|---|---|---|---|
| 14:05 | build | HEAD c1d216c no-op build; app copied as BEFORE (c1dapp) | sha256 56fbb898 |
| 14:05 | ctest | RED test_routine_pad_paint_key on c1d216c | compile error "no member named 'paintKeyOf'" |
| 14:06 | build + ctest | c2 applied (draft re-verified), GREEN 11/11; teeth drop bar / restartPending -> FAIL | commit 8e7fb2e |
| 14:10-14:18 | probe | RED g4 cadence: c1d216c (pad requests 29.2 -> FAIL), main (absent -> FAIL) | commit f19d5ba |
| 14:19-15:07 | probe | interleaved c1 / c2 x 6 (i1 + g4) | g4 176.0 -> 155.4 (-20.6; paired -21.6); i1 116.4 / 117.5 |
| 15:08 | commit | docs 57 + NN, architecture, recording | 00bb672 |
| 15:08-15:40 | probe | final app: idle-paint all rows, routines 109/0, display 16/0, deck-tabs 6/0, ctest 925/925 | GREEN except F1 i1 (rig excursion) |
| 15:45-16:06 | probe | i1 re-check c2 / c1d216c PASS 115.3 / 110.6; a1 + g5 on c2 | GREEN |

### PACKET QUALITY (fix round 1)
- Clarity: CLEAR. G12 (3) names "change ticks" for the band and the V fader; I used the existing change counters (band
  repaints on a hairline width change, fader repaints on a snapped value change).
- Missing context: none. The task said "the c1 app you already keep" while the BEFORE build it named was c1d216c. I used
  the c1d216c copy: the same code as c1 plus the TEST-ONLY preview_rect, so it is the closer twin of c2.
- Unused context: none.
- Self-brief files: plan-g4cpu.md + addendum 2, g4cpu.md, the c2 draft, and the probe sources. All useful.

INBOX-RECHECK: none

## Rebase (g4cpu-rebase)

**Merge commit:** `3b6ff75` -- `merge(s-rta-0929 g4cpu-rebase): main into lane/g4cpu — tests/CMakeLists.txt kept both lanes' registrations` (parents: `380efed` lane/g4cpu, `3e15613` main). Trial merge (`git merge --no-commit --no-ff main`) auto-merged every file except one.

**Conflict resolution -- `tests/CMakeLists.txt` (3 hunks, same logical block):** g4cpu's `test_routine_pad_paint_key` registration (add_executable / include dirs / link libs / compile defs / compile options / apply_sanitizers / catch_discover_tests) was appended at the same file location as asyncload's three new registrations (`test_load_ticket`, `test_staged_load`, `test_media_opener`). Kept BOTH sides verbatim, in this order: `test_routine_pad_paint_key` (g4cpu, complete block, unmodified) then `test_load_ticket` / `test_staged_load` / `test_media_opener` (asyncload, complete block, unmodified) -- nothing dropped, nothing duplicated. Verified post-resolution: no `<<<<<<<`/`=======`/`>>>>>>>` markers anywhere in the merged tree; each of the 4 `add_executable(test_routine_pad_paint_key|test_load_ticket|test_staged_load|test_media_opener` names appears exactly once; `catch_discover_tests(` count is 99 (one per registered executable).

**Semantic sanity on the auto-merged files (no conflict, checked anyway):**
- `docs/claude/pitfalls.md`: index (CLAUDE.md) has `57.` then `58.`; the body has both g4cpu's amended Pitfall 57 entry (the mac-peer union fix) and its follow-on `NN.` detail entry (left as `NN`, per instruction -- Harmony numbers it at merge), plus asyncload's Pitfall 58 (`A load is staged...`), already numbered, at the tail. The body's `55, 57, NN, 56, 58` ordering (56 physically after 57/NN) is pre-existing in both `HEAD` and `main` before this merge (confirmed via `git show HEAD:...` / `git show main:...`) -- not a merge artifact.
- `docs/claude/recording.md`: both lanes touched the same "Surfaces" sentence (`through deriveRoutineDeckView...`). Merged result keeps g4cpu's clause (`; a pad repaints only when its painted state changes (RoutinePad::paintKeyOf -- the sweep in pixels, Pitfall NN)`) in the same sentence, immediately followed by asyncload's whole new "UI pattern (moved verbatim from CLAUDE.md...)" / "Routine pads and bands" paragraph -- both lanes' additions present, nothing clobbered.
- `src/MainComponent.cpp/.h`, `src/api/ApiServer.cpp/.h`: auto-merged clean (no markers); confirmed by a full clean build below (both lanes' code compiles and links).

**Build:** `cmake -S WT -B WT/build-lane` (rc=0, warnings only: FetchContent_Populate deprecation notice for syphon, pre-existing) then `cmake --build WT/build-lane -j3` (rc=0, all targets built incl. `test_routine_pad_paint_key`, `test_load_ticket`, `test_staged_load`, `test_media_opener`, `AudioDNA`).

**ctest count line (verbatim):**
```
100% tests passed, 0 tests failed out of 947

Total Test time (real) =  26.29 sec
```

**Probe summary lines (verbatim, `.harmony/probe-idle-paint.sh /tmp "g4_routine,v5_routine_identity,i1_idle_card"`, merged app under `AUDIODNA_LOCK_OWNER=g4cpu-rebase`, BEFORE = main checkout's `build/AudioDNA_artefacts/Release/Audio-DNA.app`):**
```
PASS  g4_routine (a loop routine on 3 layers, adoption I3): window max median 4.7 ms [4.4-5.0] (<= 8.0) | main CPU median 143.5 ms/s [141.3-146.2] (INFO, ruling J2)
PASS  v5_routine_identity P1 (routine position 6.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
PASS  v5_routine_identity P2 (routine position 10.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
PASS  v5_routine_identity cue: routine-cue pixels in the strip column (V fill + band name), BEFORE P1 / P2, AFTER P1 / P2: 12213 / 14973 / 12213 / 14973 (each >= 300: a routine plays in every frame)
PASS  v5_routine_identity teeth: AFTER P1 vs P2 -- 9 violation cluster(s): 3 in the pad row (bbox 40 pt wide, >= 15), 6 in the strip column (>= 1), 0 elsewhere (== 0; the preview is masked: INFO below)
PASS  i1_idle_card (IDLE-HB-card): window max median 4.0 ms [3.5-4.4] (<= 8.0) AND main CPU median 110.9 ms/s [109.9-110.9] (<= 150.0)

11 PASS / 0 FAIL / 0 SKIP (16:34:31, load 5.06 5.76 5.55)
PASS  app terminated

PROBE-IDLE-PAINT GREEN
```
(g4_routine's r2/r3 launches were flagged `TAINTED (compiler seen) -- re-run` mid-run -- an unrelated background compiler process, not the merged code; the probe re-launched r6/r7 to compensate and the row still PASSed on its own gates.)

**Load averages:** start `load: { 5.44 5.49 5.18 }`; end (after quit) `load 5.06 5.76 5.55`. Load rose mid-run (peak ~7.7 1-min avg during v5's two-app launch) then settled by the end of i1_idle_card -- consistent with the rig's own launch/compile background noise noted elsewhere in this report, not a merged-code effect (i1's own CPU/window-max gates, which are load-normalized via the 5-launch median, PASSed).

**Post-run rig state:** `outwins` -> `audio-dna windows 0, Output-named 0`. Lock released (`16:34:31 lock released`). `WT/.venv` symlink removed before commit (see below). Tree clean except `build-lane/`.

**End state:** WT clean except `build-lane/` (untracked); no app running; lock released; `main` not touched, not pushed to.
