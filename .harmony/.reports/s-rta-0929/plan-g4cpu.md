# plan-g4cpu -- s-rta-0929 START HERE 3: main-thread CPU while a routine plays on 3 layers

Architect: Fable (Law #11 row 2). Executes: an opus builder in a worktree branched from main adf9b8a (lane `g4cpu`, rows
c1.., commit messages "(s-rta-0929 g4cpu)"). Inputs read in full: plan-idlepaint.md (body + adoptions I1-I12, J1-J5,
K1-K4, L1-L3), idlepaint.md (r1, fix rounds 1-2, addendum 3), diag-idle.md + diag-idle-tools/summary (compclass-base,
lagclass-*, arms-table), review-idlepaint-gates-final.md, critic-idlepaint-final.md, `.harmony/probe-idle-paint.{sh,py,json}`,
pitfalls 41 / 57, CLAUDE.md UI Patterns, recording.md "Surfaces", BORIS_DECISIONS.md "Playback Behaviour", the UI sources
and JUCE 8.0.4 (`build/_deps/juce-src`). Every `file:line` below was re-read at main HEAD adf9b8a unless marked INFERRED /
ASSUMED. No app was launched by the architect; every number that is not a citation is a prediction for c1 to check.

QUESTION: With the idle stall gone (Pitfall 57), a loop routine playing on 3 layers (probe row g4) passes the stall bar
(5.2-5.6 ms) but costs 158-164 ms/s of main-thread CPU against an idle floor of 112-135 (same-batch gap +44 / +51 ms/s;
J2 made the CPU an INFO line). What do we build so g4's CPU is at or near the idle floor, with every routine pixel
(band hairline, V fill in the routine cue, pad sweep, bound knob, playheads) identical (K2) and moving at the cadence
Boris sees today -- and what IS achievable?

APPROACH (decided): measure first, then repaint hygiene in the peer -- no timer restructuring, no new mechanism.
  (A) c1 ATTRIBUTION (TEST-ONLY, kept): per-source repaint counters (pad, V/S fader, band, wheel, corner, inspector,
      param control, SignalStrip) and a per-display-pass log (the pass's union rect in MainComponent coordinates + its
      JUCE paint time, from `MainComponent::paint` to a new `paintOverChildren`), drained by `GET /api/debug/ui_passes`.
      Probe row a1 (INFO) prints passes/s and ms/s by rect class at g4 vs i1, and the ranked shares. The c1 report sets
      the g4 bar from that arithmetic BEFORE any lever commit (section 3.3).
  (B) c2 THE PAD (always built, predicted the largest single share): `RoutinePad::setSpec` compares `progress01` -- a new
      float every 30 Hz tick while a routine plays (`RoutinePad.cpp:6-11`) -- so the ROUTINES pad repaints 27-30/s, and
      because the pad sits at MainComponent x 254-344, y 150-172 while the strips' faders sit at x 126-148 down to y 482
      and the TopBar wheel at x 329-404, y 4-38, its rect turns every deck pass into a ~280 x 480 pt union. The pad
      repaints only when what it PAINTS changes (`RoutinePad::paintKeyOf`: the sweep as a pixel width, the bar digits,
      the frame state) -- the same rule as `LayerStrip::transportViewOf` (I2) and the band hairline (I3): 11-12/s.
  (C) c3 THE WHEEL (conditional on c1): if the 15 Hz beat-wheel repaint lands in the same vblank as the deck's repaints
      (JUCE timers that once fired in the same `callTimers` stay in phase, `juce_Timer.cpp:158-163` -- INFERRED), every
      such pass unions TopBar -> strips (~x 126-404, y 4-482); the TopBar then draws in its own CoreGraphics layer
      (`NativeLayerHost::attach`, the SignalBar's exact mechanism -- it is a direct MainComponent child, paints every
      pixel, has no Viewport). Built only if c1 measures >= 8 passes/s spanning both the TopBar row and the deck at g4.
  (D) c4 THE SIGNAL BAR (conditional on c1; the J2-filed lever): `SignalBar::timerCallback` repaints the WHOLE bar 30/s
      (`SignalBar.cpp:123`); with per-strip repaint-on-change the layer draws only the strips whose painted state changed.
      Same saving at i1 and g4 (it never touches the gap); its size is input-dependent (a live mic moves most strips),
      so c1 logs which strips change per tick and c4 is built only if the arithmetic gives >= 5 ms/s.
  (E) NOT built, with the reason: the V faders and hairlines already repaint only on change (JUCE's `Slider::setValue`
      snaps to the 0.01 interval and repaints only when the snapped value moved, `juce_Slider.cpp:207,218-235,471-474`;
      the hairline is I3); coalescing the strips' rects is a no-op (AppKit unions them anyway) and folding the strips'
      timer into MainComponent's tick is a wash (arithmetic in 1.); a native overlay drawing only the moving bits is two
      paint paths per widget (rejected); snapping the V fill to whole pixels gains nothing (the 0.01 interval is already
      0.76 px on a 76-px track).
WHAT IS ACHIEVABLE (INFERRED, arithmetic in 1.3): the routine legitimately changes pixels on ~20 distinct ticks a
second (sweep 11.3/s, hairline 9.5/s, V fill 7.5/s, near-independent phases) and AppKit charges ~0.3 ms per display pass
plus the intersecting components' paint (~0.3 ms), plus the engine's own ~3-5 ms/s: the physical floor with today's look
is ~idle + 17-20 ms/s. This plan predicts idle + 25-33 (c2 alone ~+33-38; with c3 when it applies ~+25-30). "At the
idle floor" is NOT reachable without changing what Boris sees; near it (within ~10 ms/s) is. The gate bar is derived
from c1's measured arithmetic (3.3), capped at +36, RED on main by 8-15 ms/s.

---------------------------------------------------------------------------------------------------------------------
## 0. Facts re-derived from source (label VERIFIED unless stated)

| claim | where |
|---|---|
| RoutinePad repaints when `samePad` differs; `samePad` compares `progress01` exactly (a float that changes every tick while Playing) | `src/ui/RoutinePad.cpp:6-11, 21-29` |
| The pad paints the sweep as `sweepW = roundToInt(w * jlimit(0,1,progress01))` (int px), bar ticks, "bar/total" digits, the frame, "!", the name, LOOP, the restart mark | `RoutinePad.cpp:50-147` |
| `DeckView::setRoutineView` (every MainComponent 30 Hz tick, `MainComponent.cpp:3894-3899`) calls `setSpec` on all 8 pads, repaints the corner only when the note changed, then `fanRoutineBands` -> `LayerStrip::setRoutineBands` (repaints only when slot/state/name changed, `LayerStrip.cpp:844-853`) | `DeckView.cpp:542-561` |
| `deriveRoutineDeckView` sets `pad.progress01 = position / lengthBeats` while Playing; `status.slots[i].position` is the engine's beat position (120 Hz tick) | `RoutineDeckView.h:177-186`, `RoutineEngine.cpp:904-905` |
| LayerStrip's own 30 Hz timer: `timerTick` = transport on change (I2) + `syncFromModel` + band hairline on width change (I3) | `LayerStrip.cpp:334, 772-807` |
| `syncFromModel` calls `opacitySlider_.setValue(shown, dontSendNotification)` whenever `|getValue() - shown| > 1e-4`; the slider's range interval is 0.01 (`setRange(0.0, 1.0, 0.01)`) | `LayerStrip.cpp:809-842, 397, 430` |
| JUCE `Slider::setValue` snaps to the interval (`constrainedValue` = `normRange.snapToLegalValue`) and calls `owner.repaint()` ONLY when the snapped value differs from the last one | `juce_Slider.cpp:201-239, 471-474` |
| The V fill is a float rect (`fillRect(Rectangle<float>)` -> `CGContextFillRect` with AA on): any value step moves an anti-aliased edge; no further quantisation is possible without a look change | `LayerStrip.cpp:126-135`, `juce_CoreGraphicsContext_mac.mm:243, 521-541` |
| JUCE culls `drawText` outside the clip (`clipRegionIntersects`) -- text-heavy paints outside a small union are free | `juce_GraphicsContext.cpp:404-408` |
| TopBar: 15 Hz timer repaints `beatWheelBounds_.getUnion(barPhraseBounds_).expanded(2)` every tick unconditionally; `paint` fills every pixel of its bounds | `TopBar.cpp:306, 314-331, 442-462` |
| TopBar geometry (`resized`, `area = reduced(4,2)`): wheel at local x 331-355, bar readout 358-402 -> the repaint rect is MainComponent x 329-404, y 4-38 | `TopBar.cpp:562-576`; TopBar at Main (4,4) `MainComponent.cpp:2530-2532` |
| Deck geometry: DeckView at Main y = 4+34+1+84+1+24+2 = 150, x = 4; ROUTINES row 22 px (pad i at deck x 250 + 90 i, so pad 1 = Main x 254-344, y 150-172); column triggers 172-194; grid 194-482 (3 x 96); strip 250 wide; V slider at strip x 122-144 (Main 126-148); thumb 76 px at 144-220 (Main 148-224); hairline rect = thumb x 32 px | `MainComponent.cpp:2524-2660`, `DeckView.cpp:57-88, 362-375`, `DeckView.h:154-160`, `LayerStrip.cpp:612-694` |
| SignalBar: 30 Hz timer updates every strip then `repaint()` of the whole bar; the layer then draws the whole bar | `SignalBar.cpp:34, 111-124`; `NativeLayerHost.mm:203-233` |
| SignalStrip paints: 2-decimal value text, a float-rect meter fill, a float peak line (shown while `peakValue_ > 0.01`), a flash while `flashAlpha_ > 0.05`; `updateValue` smooths (alpha 0.3), holds/decays the peak, decays the flash | `SignalStrip.cpp:10-37, 121-200` |
| 14 strips are visible by default (12 audio + Mod 1 / Mod 2), 40 px each in Normal size | `SignalRegistry.cpp:5-7, 49-58`; `SignalStrip.cpp:44-53` |
| MainComponent's 30 Hz timer: the input-meter repaint is inert in v2 (`inputLevelMeterBounds_ = {}`), the inspector refreshes at ~10 Hz (Clip tab: repaint on paint-key change) | `MainComponent.cpp:450, 2583, 3857-3862`; `ClipInspector.cpp:960-991` |
| `LayerInspector::refresh` still ends in an unconditional `repaint()` (10 Hz while the Layer tab is active) -- NOT in the g4 fixture (the Clip tab is active), FILED | `LayerInspector.cpp:795-799` |
| `UniversalParamControl::setParamValue` -> `updateValueDisplay` -> `repaint()` unconditionally, but it is called from refresh paths only for Source clips / effect rows -- the card clip's Brightness row is refreshed by `EffectStackView::refresh` (VERIFIED callers `EffectStackView.cpp:159-215`); c1 counts it | `UniversalParamControl.cpp:338-352, 677-680` |
| JUCE timers: ONE `CallTimersMessage` per TimerThread wake; `callTimers` fires every timer whose countdown <= 0 and resets each to its period AT FIRE TIME -> timers that once fired together keep firing together (INFERRED lock-in after any stall > their phase gap; the diag's 3.35/s wheel coincidences at idle say the 15 Hz timer was NOT locked then -- c1 measures it under g4) | `juce_Timer.cpp:106-178` |
| The g4 fixture: 3 layers x the test card, one 16-beat LOOP routine at a manual 120 BPM (8.0 s per loop) whose three lanes ramp opacity 0.4 -> 1.0 linearly, quantize off, restore off | `probe-idle-paint.py:276-303, 312-337` |
| Per-pass component costs on main at idle (BODY passes, 29.65/s): RoutinePad 13.5 ms/s (57 us/pad), LayerStrip 3.8 (32 us/strip), ResettableSlider 8.9, ClipCell 6.1, DeckView 7.4, TextButton 43.0, Label 12.1; a TopBar-only pass 0.73 ms total with ~0.49 ms of JUCE paint | `diag-idle-tools/summary/compclass-base.txt`, `lagclass-base.txt` (C3) |
| g4 on the final idlepaint build: window max 5.2-5.6 ms, CPU 158.2 / 162.6 / 163.6 ms/s; i1 in the same batches 114.2 / 134.9* / 112.6 (*different binary); band repaints 28.7/s (3 x 9.6); MainComponent paints 27/s vs 14.9 at idle | `idlepaint.md` (r1 table, fix round 1 G3, fix round 2 F2) |
| `inject_features` (test mode) sets bpm, beatPhase, beatInBar, barCount, totalBeatCount (not trackerState); the routine clock integrates `totalBeatCount + beatPhase` and does not advance while bpm == 0; `fire` with no beat available starts NOW | `ApiServer.cpp:867-910`, `RecorderClock.h:22-30`, `RoutineEngine.cpp:752-758` |

### Derived rates for the fixture (arithmetic on VERIFIED constants)
Loop 16 beats @ 120 BPM = 8.000 s. Pad sweep: 90 px / 8 s = 11.25 pixel steps/s (+0.5/s bar digits). Hairline: 76 px / 8 s =
9.5/s (matches the measured 9.6/s -- the consistency check for the geometry above). V fader: 0.6 / 8 s = 0.075 per s /
0.01 interval = 7.5 snapped steps/s per strip (all three strips step in the same tick: same curve, same clock). Wheel: 15/s.
Pad key changes: 11.75/s. Ticks with ANY deck pixel change (independent phases): 30 x (1 - 0.625 x 0.683 x 0.75) = 20.4/s.

---------------------------------------------------------------------------------------------------------------------
## 1. TRADEOFFS CONSIDERED

- **Pad repaint-on-painted-change (c2) -- ACCEPTED.** ~12 lines + a pure function + a test; pixel-identical by construction
  (same paint code, fewer calls); removes the ONLY 30/s repainter in the routine path (everything else already repaints on
  change). Predicted share: the pad + its union growth (column-1 cells, the column trigger, the ROUTINES corner text, the
  deck background) in 27-30 passes/s -> 11.75/s. Saving 6-11 ms/s alone; more when the wheel also leaves (the union's
  x-range is then bounded by the bands at 224 instead of the pad's 344 / the wheel's 404).
- **TopBar in its own layer (c3) -- ACCEPTED CONDITIONALLY.** Removes the TopBar -> deck unions (each drags the TopBar's
  transport buttons and labels, row 1's buttons and the SignalBar's rect (native, ~free) into a deck pass) and takes the
  wheel's 15/s off the peer at idle too (~0.73 -> ~0.45 ms per pass). Worth ~8-12 ms/s if the wheel and the deck batch
  coincide ~15/s (locked timers), ~3-4 ms/s if they coincide ~3/s (free-running). A third instance of a proven mechanism
  (SignalBar / WaveformDisplay; OverlayWatch, v2/v2b) -- but it puts the app's only text field (`bpmEditField_`) and every
  transport / tempo control under a native view, so it is built only when c1 shows it pays and it gets its own overlay row.
- **Per-strip SignalBar repaints (c4) -- ACCEPTED CONDITIONALLY** (the J2-filed lever). It cannot move the g4 - i1 gap
  (the bar's cost is the same in both arms); it lowers both absolutes by (static-strip fraction) x ~30 ms/s (SignalStrip
  29.9 ms/s at 26.3 passes on main; ~35 ms/s in the layer now). With a live mic most strips wobble every tick and the
  dirty union is a contiguous span, so the saving may be ~0; in Boris's show (music) it IS 0. Built only on c1's numbers.
- **Deck grid in its own layer (Option B) -- REJECTED for now.** DeckView is a direct MainComponent child (`:648-649`),
  so it is attachable; it would confine the deck's unions like c3 does (the deck leaves the peer instead of the wheel) at
  the same predicted saving (~8-12) but with far more interactive surface under a native view (clip cells' drag/drop
  targets, 3 popup sites, the tab row, tooltips on every button, the scrollbars). c3 gets the same union win cheaper.
  Re-open only if c1 shows the peer's whole-tree traversal (not the union content) dominating a deck pass.
- **A NativeLayerHost OVERLAY drawing only the moving elements (hairline, V fill, sweep, playhead) over the in-peer
  deck -- REJECTED.** The overlay must draw pixels identical to the in-peer widget's (the V fill sits UNDER the "V" label
  and the border; the fill's AA edge blends over the slider's background), which means every element gets two paint
  paths (in-layer and in-peer for the fallback) plus scroll-offset tracking over the Viewport -- Pitfall 13's dual-path
  trap, for ~10 ms/s.
- **Fold the strips' 30 Hz ticks into MainComponent's tick ("coalescing") -- REJECTED as a wash.** Same-batch: ~20.4
  deck passes/s x (0.3 base + ~0.5 content); separate batches: ~26 passes/s x (0.3 + ~0.3). Within +/-2 ms/s either way,
  and it rewrites Pitfall 41's "the strip's own timer" for nothing. Merging the three strips' rects into one is a no-op
  (AppKit unions the dirty rects of one view, diag-idle "the dirty region really is the whole union").
- **Quantise the V fill / hairline / playhead further -- NOT POSSIBLE without a look change, and pointless.** Hairline and
  playhead are already integer-pixel on-change (I2/I3). The V fill's edge is anti-aliased, so any value step is a pixel
  change; snapping it to whole pixels would change the look (rejected by BORIS_DECISIONS "how they look must not change")
  and would drop the rate only from 7.5/s to ~5.7/s (0.6 x 76 px / 8 s).
- **Wheel repaint-on-change (static when no tempo) -- FILED, not built here.** ~8 ms/s at idle (14.9 passes x ~0.5),
  0 at g4 (a manual 120 BPM animates it) -- it widens the g4 - i1 gap and changes g1's idle expectation; a separate
  idle-floor lane.
- **`LayerInspector::refresh` paint key (F6 for the Layer tab) -- FILED.** Not in the g4 fixture; in real use a routine
  driving a layer while its tab is open costs ~10 passes/s x 2-3 ms. Own lane.
- **F2 / `setBufferedToImage` / JUCE patch -- REJECTED** (plan-idlepaint 1, unchanged).
- **Strongest counterargument to the whole approach**: "halving the gap is not the goal; take the strips out of the peer".
  Why it loses: a strip cannot own a layer (Viewport, Pitfall 57), Option B moves the SAME ~20 event passes/s into a
  layer with the same per-pass floor, and the residual after this plan (~idle + 25-33) is within ~10 ms/s of the physical
  floor of the current look (~idle + 17-20). The remaining lever is a look change, which is Boris's call, not a plan's.

---------------------------------------------------------------------------------------------------------------------
## 2. DECISION / SPEC

### 2.1 Predicted shares (INFERRED; c1 prints the measured table in the same shape, then ranks)
Gap on main-with-counters (the c1 binary) at g4 vs i1, predicted 44-51 ms/s, decomposed:

| # | share | mechanism | predicted ms/s | lever |
|---|---|---|---|---|
| S1 | union growth from the PAD in every deck pass | pad 30/s -> every deck pass spans x .. 344, y 150 ..: + pad (57 us) + 3 column-1 ClipCells (~50 us each) + 1 column trigger + the ROUTINES corner text + DeckView fill | 27-30 x ~0.35 = 9-11 | c2 -> 11.75 x 0.35 = 4 (saves 5-7) |
| S2 | union growth from the WHEEL landing in a deck pass | TopBar transport buttons + labels + row-1 buttons + traversal over the SignalBar rect | if locked: 13-15 x ~0.6 = 8-9; if free: 3-4 x 0.6 = 2 | c3 (locked case) -> 0 |
| S3 | the deck content itself | 3 strips (32 us each + clipped draws), 3 V sliders (~30 us), 3 blend combos (~20 us), bands (thumb blit + 2 texts x 3) on hairline ticks | 20-30 passes x 0.2-0.35 = 5-9 | none (the look) |
| S4 | per-pass base (AppKit dirty tracking + CA commit + JUCE tree traversal) for passes that would not exist at idle | 12-15 extra passes/s (30 deck passes minus the 15 wheel passes idle already pays) x ~0.3 | 4-5 | c2 removes ~30-20.4 = 9.6 empty ticks/s only when the wheel is not in them: -1.5 |
| S5 | the routine engine + status + view derivation on the message thread | `routineEngine_.tick` 120 Hz with 3 lanes; `status()` copy + `deriveRoutineDeckView` (8 pads of strings, a map) + `setRoutineView` 30/s | 3-6 | none |
| S6 | unexplained (the diag's "FULL vs BODY 2.5x" class) | -- | the rest to 44-51 | c1 names it |
c1's table replaces this one; the levers are then ranked by MEASURED share. The order of magnitude claim that matters:
S1 + S2 (the pad's and the wheel's union growth) are the only shares a code change can remove without changing the look.

### 2.2 c1 -- attribution (TEST-ONLY, kept; no behaviour change)
`src/ui/UiPaintCounters.h` (`:7-34`) gains, in `Counters`:
```cpp
    // s-rta-0929 g4cpu: who repaints while a routine plays (relaxed atomics, message thread; GET /api/debug/ui_paint).
    std::atomic<uint64_t> routinePadRepaints { 0 };            // RoutinePad::setSpec -> repaint()
    std::atomic<uint64_t> routinePadSweepTicks { 0 };          // setSpec calls whose painted sweep width changed
    std::atomic<uint64_t> routinePadSweepPaints { 0 };         // paints whose painted sweep width moved (the I2-style witness)
    std::atomic<uint64_t> layerStripFaderRepaints { 0 };       // V / S fader setValue calls that changed the snapped value
    std::atomic<uint64_t> topBarWheelRepaints { 0 };           // TopBar::timerCallback repaint() calls
    std::atomic<uint64_t> deckCornerRepaints { 0 };            // DeckView corner-note repaints
    std::atomic<uint64_t> layerInspectorRepaints { 0 };        // LayerInspector::refresh repaint() calls
    std::atomic<uint64_t> paramControlRepaints { 0 };          // UniversalParamControl::updateValueDisplay repaint() calls
    std::atomic<uint64_t> signalStripChanges { 0 };            // SignalStrip::updateValue calls whose painted key changed
    std::atomic<uint64_t> signalBarTicks { 0 };                // SignalBar::timerCallback calls
#if AUDIODNA_TEST_SERVER
    // The last 512 display passes: the pass's clip rect in MainComponent coordinates and its JUCE paint time
    // (MainComponent::paint -> paintOverChildren, steady_clock), plus the SignalStrip change mask of the last 512 bar
    // ticks (bit i = strip i changed). Written on the message thread, read by GET /api/debug/ui_passes (torn reads are
    // tolerable: INFO only). Fixed size, no allocation.
    struct Pass { std::atomic<int64_t> tUs { 0 }; std::atomic<int32_t> x { 0 }, y { 0 }, w { 0 }, h { 0 }, us { 0 }; };
    Pass passes[512]; std::atomic<uint32_t> passSeq { 0 };
    std::atomic<uint64_t> stripMasks[512]; std::atomic<uint32_t> stripMaskSeq { 0 };
    std::atomic<int> deckRect[4] {}, padRowRect[4] {}, stripColRect[4] {}, wheelRect[4] {}, inspectorRect[4] {};
#endif
```
Increment sites (one line each, the existing shape): `RoutinePad::setSpec` (repaints + sweep ticks -- computed via the
c2 key; in c1 the tick counter compares `roundToInt(getWidth() * progress01)` of old vs new spec inline),
`RoutinePad::paint` (sweep paints, like `LayerStrip::paint:553-557`), `LayerStrip::syncFromModel` (after each
`setValue`: `if (slider.getValue() != before) ++faderRepaints` -- the snapped value moved => JUCE repainted),
`TopBar::timerCallback:327`, `DeckView::setRoutineView:549`, `LayerInspector::refresh:798`,
`UniversalParamControl::updateValueDisplay:679`, `SignalStrip::updateValue` (c1 adds the painted-key compare and returns
bool -- used by c4 later; in c1 the bar still repaints wholesale), `SignalBar::timerCallback` (ticks + the mask).
`MainComponent`: `paint` (`:2429`) records `g.getClipBounds()` + a start stamp in a member; a NEW
`void paintOverChildren(juce::Graphics&) override` (declared unconditionally, body `#if AUDIODNA_TEST_SERVER`) stores the
entry (`passes[seq % 512]`, then `passSeq++`). `recordUiGeometry` (`:2745-2759`) also writes deckRect, padRowRect
(deck x + 250 .., y .. +22), stripColRect (gridViewport bounds intersected with x < deck x + 250 -- DeckView exposes
`getGridViewportBounds()` + `kLayerStripWidth`), wheelRect (TopBar exposes `getWheelRepaintBounds()` = the `:327` rect),
inspectorRect. `ApiServer` (`:286-300`, `:1753-1785`): `ui_paint` gains the ten counters; NEW `GET /api/debug/ui_passes`
returns `{ok, seq, passes:[{t,x,y,w,h,us}...] (the 512 newest, oldest first), strip_seq, strip_masks:[...]}` -- read on
the HTTP thread, no message-thread hop. `APP-INVENTORY.md:204-212` gains one sentence.

Probe row **a1_attribution** (INFO; `probe-idle-paint.py`; CFG `"a1": {"launches": 3}`): arms card (i1's fixture) and
routine (g4's), 3 launches each, the standard window (6 s settle, 30 s, heartbeat on), draining `ui_passes` every 500 ms
(the same poll the window already does for `ui_paint`). Per pass: class = the sorted set of {wheel, padrow, stripcol,
deck, inspector} rects it intersects (else "other"). Prints per arm: repaint sources/s (all ten + the existing ones),
passes/s, and per class: passes/s, median / p90 JUCE us, JUCE ms/s; then `cpu_main` and `cpu_main - juce_ms_per_s`
(= AppKit/CA/glue + timers + engine). Then the RANKING: for each lever the arithmetic saving from the measured classes
(c2: (passes with padrow) x (median us of "padrow+..." classes - median us of the same class without padrow); c3: (passes
with wheel AND deck) x (median us with wheel - without) + 14.9 x (0.73 - 0.45) ms; c4: (1 - median fraction of the
strips' width covered by the per-tick dirty span, from strip_masks) x (SignalStrip ms/s from compclass-base 29.9 scaled
by sb_draws/29.65)). The a1 line also prints the same at i1 (card) for the "extra passes over idle" number.
Prediction to compare against: at g4 pad repaints 27-30/s, fader 22.5/s (3 x 7.5), band 28.7/s, wheel 14.9/s, corner 0,
inspector 0, param 0; passes 27-30/s; classes "padrow+stripcol+deck(+wheel)" ~13-15/s at ~1.0-1.4 ms JUCE if locked.
At i1: wheel 14.9/s alone (~0.45-0.5 ms JUCE), ~4/s TopBar label passes, everything else 0.

### 2.3 c2 -- RoutinePad repaints only when its painted state changes
`src/ui/RoutinePad.h` (public, after `getSlot()` `:23`):
```cpp
    // s-rta-0929 g4cpu (Pitfall 57 rule 2): everything paint() draws, AS it draws it -- the sweep as a pixel width, never
    // progress01 (a new float every 30 Hz tick while a routine plays, which made the pad repaint 30 times a second and
    // its 90x22 rect union with the strips' and the wheel's into a 280x480 pt pass). setSpec() repaints only when this
    // changes at the pad's current width. Pure; public for tests/test_routine_pad_paint_key.cpp.
    struct PaintKey
    {
        int number = 0;
        juce::String name;
        RoutineDeckView::State state = RoutineDeckView::State::Empty;
        bool onShownDeck = true, loop = false, warning = false, restartPending = false;
        int sweepW = 0, bar = 0, barsTotal = 0;
        bool operator==(const PaintKey&) const = default;
    };
    static PaintKey paintKeyOf(const RoutineDeckView::Pad& spec, int width);
```
private: `int lastPaintedSweepW_ = -1;` (the witness). `src/ui/RoutinePad.cpp`: delete `samePad` (`:4-12`);
```cpp
RoutinePad::PaintKey RoutinePad::paintKeyOf(const RoutineDeckView::Pad& spec, int width)
{
    PaintKey k;
    k.number = spec.number; k.name = spec.name; k.state = spec.state; k.onShownDeck = spec.onShownDeck;
    k.loop = spec.loop; k.warning = spec.warning; k.restartPending = spec.restartPending;
    k.bar = spec.bar; k.barsTotal = spec.barsTotal;
    if (spec.state == RoutineDeckView::State::Playing)   // paintContent: the sweep exists only while Playing
        k.sweepW = juce::roundToInt(static_cast<float>(width) * juce::jlimit(0.0f, 1.0f, spec.progress01));
    return k;
}

void RoutinePad::setSpec(const RoutineDeckView::Pad& spec)
{
    const auto before = paintKeyOf(spec_, getWidth()), after = paintKeyOf(spec, getWidth());
    if (spec.tooltip != spec_.tooltip)
        setTooltip(spec.tooltip);
    spec_ = spec;
    if (after.sweepW != before.sweepW)
        uipaint::counters().routinePadSweepTicks.fetch_add(1, std::memory_order_relaxed);
    if (!(after == before))
    {
        repaint();
        uipaint::counters().routinePadRepaints.fetch_add(1, std::memory_order_relaxed);
    }
}
```
`paintContent` (`:61-65`): keep the formula; after computing `sweepW`, the witness:
`if (sweepW != lastPaintedSweepW_) { lastPaintedSweepW_ = sweepW; ++routinePadSweepPaints; }` (only inside the
`Playing` branch; reset `lastPaintedSweepW_ = -1` when not Playing so a re-fire counts again). Every field `paintContent`
reads is in the key (number, name, state, onShownDeck (the 50 % layer), loop (LOOP tag), warning ("!"), restartPending
(the mark), bar / barsTotal (digits + ticks), progress01 only through sweepW). `tooltip` and the menu-only fields
(quantize, restoreFirst, startEase) are not painted and not in the key -- as today (`samePad` ignored them too). A resize
repaints through JUCE as always; the key at the new width then differs on the next tick at most once.
ctest `tests/test_routine_pad_paint_key.cpp` [g4cpu][pad] (links like `test_routine_pad_press`, `tests/CMakeLists.txt:1585-1611`):
(a) 90-px pad, Playing, progress 0.300 vs 0.304 -> equal keys (sweep 27 = 27); 0.300 vs 0.311 -> differ (27 vs 28);
(b) bar 2 -> 3 differs; name / state / warning / restartPending / onShownDeck / loop each differ; barsTotal differs;
(c) a Waiting pad with progress 0.9 has sweepW 0 (not painted) and equals one with progress 0.1;
(d) on a real `RoutinePad` (`ScopedJuceInitialiser_GUI`, `setBounds(0,0,90,22)`): `setSpec` with the sub-pixel change
leaves `routinePadRepaints` unchanged and a tooltip-only change leaves it unchanged; the pixel-crossing change bumps it
by exactly 1; (e) a `createComponentSnapshot` of the pad at progress 0.300 equals the one at 0.304 pixel for pixel
(`Image` compare -- the pixel proof of (a)). RED on main: `paintKeyOf` absent (compile error).

### 2.4 c3 -- TopBar in its own layer (CONDITIONAL: c1 measured >= 8 passes/s at g4 whose rect spans both the TopBar row and the deck, i.e. class contains wheel AND (padrow | stripcol | deck))
`src/ui/UiPaintCounters.h:8`: `enum Layer : int { Waveform = 0, SignalBar = 1, TopBar = 2, LayerCount = 3 };`
`layerMode` initialiser `{ -1, -1, -1 }`. `src/MainComponent.h:420`: `..., signalBarLayer_, topBarLayer_;`.
`src/MainComponent.cpp` after `:2298`:
```cpp
    // s-rta-0929 g4cpu: the 15 Hz beat wheel unioned with a playing routine's deck repaints (TopBar -> strips); the
    // TopBar draws in its own layer like the SignalBar. Its paint() fills every pixel (TopBar.cpp paint): an opaque layer.
    topBar_->setOpaque(true);
    topBarLayer_ = NativeLayerHost::attach(*topBar_, *overlayWatch_, uipaint::TopBar);
```
`ApiServer::handleDebugUiPaint`: + `topbar_layer_draws`, `topbar_mode`; `handleDebugUiNativeFallback` applies to it too
(same loop as the other two). `recordUiGeometry` already writes `topBarRect`. Nothing in `TopBar.cpp` changes.
Preconditions the builder verifies before attaching (each a one-line check in the c3 commit message): `TopBar::paint`
fills `getLocalBounds()` (`:444-451`); TopBar has no `juce::Viewport`; its 7 `PopupMenu` sites are desktop windows
(ComboBox) or parented to the top level (`OverlayWatch::isWindowOverlay`) -- `grep -n "showMenuAsync\|withParentComponent"
src/ui/TopBar.cpp src/output/*.cpp`; no `setPopupDisplayEnabled` on its sliders; `bpmEditField_` is a `juce::TextEditor`
whose caret is a child component (repaints in the layer). g1 keeps `top_bar_paints` in [12, 26] (TopBar::paint runs
inside the layer's drawRect, the counter still counts) and adds `topbar_layer_draws/s` in [12, 26] and `topbar_mode == 0`;
g3 prints `main_component_paints/s` (expected ~0-1 at idle after c3: the fps label lives in the TopBar). New row
**v2t_topbar_fallback** = v2 with the panel at MainComponent (350, 20) (over the wheel): `topbar_mode` -> 2 within 1 s,
A == C (K2), the panel visible over the TopBar, the other two layers stay native; v2b's frame witness runs for the
TopBar too (the same `ADNA_UI_OVERLAY_WITNESS` machinery counts every host). x1 (`ADNA_UI_NATIVE_LAYERS=0`) covers it.
Identity: v1 / v3 / v3p compare the TopBar row outside the fps / beat-tempo masks: the button-corner class (rounded
`LookAndFeel_V4` buttons, 1/255) is tolerated by K2; a text difference > 1/255 = FAIL = a finding -> revert c3 (it is
one commit) and file. LOOK (Boris): the Outputs menu drops over the TopBar and is fully visible; click Manual, type a BPM
and Enter -- the field edits, the wheel follows; hover Tap until its tooltip shows.

### 2.5 c4 -- SignalBar repaints only the strips whose painted state changed (CONDITIONAL: c1's arithmetic >= 5 ms/s at i1)
`src/ui/SignalStrip.h`: `bool updateValue(float newValue);   // -> true when what paint() draws changed` + private
`struct Painted { float value = -1.f, peak = -1.f, flash = -1.f; bool operator==(const Painted&) const = default; }
Painted painted_;`. `SignalStrip.cpp:10-37`: after the existing updates compute
`Painted now { displayValue_, peakValue_ > 0.01f ? peakValue_ : -1.f, flashAlpha_ > 0.05f ? flashAlpha_ : -1.f };`
(exactly the three quantities the three `paint*` bodies branch or fill on; floats compared exactly because every
change moves an anti-aliased edge or the 2-decimal text) -> `const bool changed = !(now == painted_); painted_ = now;
if (changed) ++signalStripChanges; return changed;`. `SignalBar.cpp:111-124`:
```cpp
    for (auto& strip : strips_)
        if (strip->updateValue(registry_.getCachedValue(strip->getSignal().getId())))
            strip->repaint();        // s-rta-0929 g4cpu: the bar's own paint (fill + border) is static; only a strip that
                                     // changed marks the layer (NativeLayerCache -> the dirty union of changed strips)
    uipaint::counters().signalBarTicks.fetch_add(1, std::memory_order_relaxed);
```
and the bar-wide `repaint()` goes. Pixel identity by construction: the layer's backing store keeps the untouched
strips; a repainted strip is drawn by the same code with the clip = the dirty union (`drawLayer` ->
`paintEntireComponent`), the bar's fill under it included. g1: at i1 `signalbar_layer_draws/s` <= 36 (a silent input may
draw ~0/s -- that is the point); the lower bound [24, 36] moves to g4 (manual 120 BPM: Beat Position / Bar Position /
Mod 1 change every tick) and g5. v1b (native vs in-peer at a frozen state) unchanged. `SignalRegistry` / `Signal` untouched.

### 2.6 The g4 gate (c2 commit; probe-idle-paint.py `g4_routine` rewritten) -- see 3.2 for RED / GREEN
### 2.7 MUST-NOT-CHANGE (the reviewer checks each)
The routine's pixels at any position: pad look (`paintContent` untouched), band hairline + name + x (`paintRoutineBands`
untouched), the V fill and its routine cue (`OpacitySliderLookAndFeel`, `syncFromModel` untouched), the bound knob's cue
(`UniversalParamControl` paint untouched -- counters only), the transport playhead (I2 untouched); every timer rate
(LayerStrip 30, MainComponent 30, TopBar 15, SignalBar 30, WaveformDisplay 30 Hz); `syncFromModel` every tick (Pitfall
41); `setRoutineBands` / `fanRoutineBands` / `deriveRoutineDeckView` / `RoutineEngine` untouched; the pad's press = Fire,
right-click = menu, tooltip updates on change; SignalStrip / TopBar / LayerStrip paint code; no new timer, no new
mutex, no JUCE patch; OutputWindow untouched; production builds carry no env switch (everything new under
`AUDIODNA_TEST_SERVER` except the two paint-key rules); non-Apple builds unchanged (c3's `attach` -> nullptr);
CLAUDE.md stays <= 25,000 B (no CLAUDE.md edit is planned: the UI Patterns line `:141` already states both rules).

---------------------------------------------------------------------------------------------------------------------
## 3. GATES (RED first, every one; ctests drive real code; probe rows on the rig's rules)

### 3.1 ctests (Catch2; registered at the EOF of `tests/CMakeLists.txt` in the cited block shapes)
1. `tests/test_routine_pad_paint_key.cpp` [g4cpu][pad] -- 2.3 (a)-(e). RED on main: `paintKeyOf` absent.
2. `tests/test_signal_strip_painted.cpp` [g4cpu][signalbar] (c4 only; links `SignalStrip.cpp` + `LookAndFeel.cpp` +
   `SignalRegistry.cpp` like `test_param_control_routine_cue`'s set, `juce_gui_basics`): a strip fed the same value 60
   times returns true while the smoothing / peak / flash still move and false once they have settled (the peak decays
   below 0.01 -> the key stops changing); a value change returns true once per change; a "Hit" strip's flash returns
   true while `flashAlpha_ > 0.05` then false. RED on main: `updateValue` returns void (compile error).
3. Existing: `test_routine_pad_press`, `test_routine_deck_view`, `test_layer_strip_follows_model` (Pitfall 41),
   `test_layer_strip_transport_view`, `test_native_layer_cache`, `test_overlay_watch` stay green. Full serial ctest
   (`-j1`, the s-rta-0928b posture): 920/920 today (work log 08:40) -> 920 + new.

### 3.2 Live probe rows (`.harmony/probe-idle-paint.{sh,py,json}` -- same header, lock gate, `open -g`, no Output window,
no synthetic input, >= 5 launches per gate arm, compiler taint re-run, load printed per launch)
| row | arm | PASS | RED on the pre-change app (predicted) | GREEN after (predicted, INFERRED) |
|---|---|---|---|---|
| a1_attribution (INFO, new, default) | card + routine, 3 launches each, `ui_passes` drained | prints the 2.2 tables + the ranking + the derived bar (3.3) | fields absent on main -> INFO "absent"; on the c1 binary the 2.2 prediction table is checked | the same rows on the final binary: the "after" attribution |
| **g4_routine** (REWRITTEN, the gate) | 5 launches AFTER (routine) + i1's 5 launches from the SAME invocation (`SUMMARY["i1_idle_card"]`, else FAIL "run i1 first") | (1) window max median <= 8.0 ms; (2) **cpu median(g4) - cpu median(i1) <= CFG.g4.cpuOverIdleMsPerS** (set by 3.3, cap 36); (3) cadence: `routine_pad_repaints/s` in [8, 16] and `routine_pad_sweep_paints / routine_pad_sweep_ticks` >= 0.95; `layer_strip_band_repaints/s` in [20, 36]; `layer_strip_fader_repaints/s` in [15, 30]; INFO: BEFORE (main) arm as today, the absolute numbers, MainComponent paints/s | main proper: counters absent -> FAIL; the c1 binary: gap **44-51** > bar -> FAIL, pad repaints **27-30/s** > 16 -> FAIL | gap 25-33 (<= bar), pad 11.5-12.5/s, ratio 1.00, band 28-29, fader 22-23; window max 4.5-5.5 |
| **v5_routine_identity** (new, default) | test mode, BEFORE (main) vs AFTER; routine fixture; `inject_features {bpm:120,totalBeatCount:100,beatPhase:0}`, `routine/fire 0` (no beat available -> starts now, `RoutineEngine.cpp:752-758`), poll `running`; `inject_features {bpm:120,totalBeatCount:106,beatPhase:0.5}`; 0.7 s; `/api/routine/status` position in [6.4, 6.6] else SKIP "clock did not advance" (never PASS); capture P1; `inject {totalBeatCount:110,beatPhase:0.5}`; 0.7 s; position in [10.4, 10.6]; capture P2 | BEFORE P1 vs AFTER P1 and P2 vs P2: 0 K2 violations outside the fps mask + the SignalBar rect (Mod 1 is live, v1 S2's rule); TEETH: AFTER P1 vs P2 has >= 1 violation cluster and EVERY violation lies inside padRowRect or stripColRect (the routine moved: sweep 37 -> 59 px, hairline 31 -> 50, V fill 0.64 -> 0.79) and the pad cluster's bbox is >= 15 px wide at 1x; prints the clusters | n/a (needs two builds); the teeth is the P1/P2 shift; a comparator that misses the shift FAILS the row | PASS |
| v2t_topbar_fallback (c3 only) | AFTER, test mode | 2.4 | endpoint present but `topbar_mode` absent -> FAIL | PASS |
| i1, i2, g1, g2, g3, v0, v1, v1b, v2, v2b, v3, v3p, v4, x1, g5 | unchanged rows | unchanged thresholds (g1 per 2.4 / 2.5 only when c3 / c4 land) | -- | GREEN unchanged (i1 / i2 may read lower with c4) |
The g4 pad-cadence bounds: 90 px / 8 s = 11.25 + 0.5 digit changes -> [8, 16] holds for a 60-140 px pad at 100-140 BPM;
band [20, 36] = 3 x 9.5; fader [15, 30] = 3 x 7.5 (the ramp 0.6 / 8 s at interval 0.01) -- the strip count and fixture are
the probe's own, so these are constants of the fixture, not tunables. `.json` gains `"a1": {"launches": 3}`,
`"g4": {..., "cpuOverIdleMsPerS": <c1>, "padRepaintsPerS": [8, 16], "minPadPaintRatio": 0.95, "bandRepaintsPerS": [20, 36],
"faderRepaintsPerS": [15, 30]}`, `"v5": {"beat0": 100, "beat1": [106, 0.5], "beat2": [110, 0.5], "positionTol": 0.1,
"settleS": 0.7, "minPadShiftPt": 15}`. Never re-threshold after 3.3: a FAIL is a finding.

### 3.3 The bar, derived once from c1's quiet numbers (pre-registered here, recorded in the lane report BEFORE c2)
`bar = min(36, round(residual + 6))` where `residual = G_red - saved`, `G_red` = median(g4 cpu) - median(i1 cpu) on the c1
binary (5 launches each, one invocation; predicted 44-51), `saved` = the a1 ranking's arithmetic for the levers that WILL
be built (c2 always; c3 / c4 per their criteria). Prior prediction: saved 15-22 -> residual 25-33 -> bar 31-36. Rules:
(i) the bar is written down before c2 lands and never moved; (ii) if `residual > 30` the builder STOPS after c1 and
reports (the levers cannot produce a gate with >= 8 ms/s of teeth; Harmony rules: accept an INFO line, order Option B, or
rule on a look change); (iii) RED is proven by the c1 binary: `G_red > bar` by >= 8 ms/s, else STOP likewise.
Why relative, not absolute: i1 itself read 112.6 / 114.2 / 134.9 across batches (idlepaint.md) -- an absolute bar of 150
would pass or fail on the day's idle floor, not on the routine's cost; the gap isolates the routine (same run, same
machine state, minutes apart). The absolute numbers are printed beside it for Boris.

### 3.4 Boris LOOK checkpoint (Harmony's critic panel first; the human list)
Fire a routine that spans 3 layers: the pad's teal sweep creeps and "n/4" counts; each band's hairline creeps; each V
fader glides in chartreuse; press the pad again -> the restart mark appears left of "n/4" and the sweep restarts on the
bar; right-click the pad -> the menu; hover it -> the tooltip. With c3: the Outputs menu over the TopBar, Manual + a typed
BPM, the wheel turning at the typed tempo, a tooltip over Tap. Nothing may look different from today.

---------------------------------------------------------------------------------------------------------------------
## 4. COMMITS (each builds, ctest serial green, every existing probe row GREEN; `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`)
Worktree `.claude/worktrees/rta0929-g4cpu`, branch `lane/g4cpu` from main adf9b8a; build dir `build-lane`,
`-DAUDIODNA_BUILD_TEST_SERVER=ON`, JUCE via `FETCHCONTENT_SOURCE_DIR_JUCE=<main>/build/_deps/juce-src`
(diag-idle-tools/build.sh). Perf batches never overlap a commit (the graphify post-commit hook, idlepaint.md notebook).
- c1 `test(s-rta-0929 g4cpu): attribution -- per-source repaint counters, per-pass log + GET /api/debug/ui_passes (TEST-ONLY), probe row a1`
  -- no behaviour change. THEN: a1 on the c1 binary (card + routine); the RED g4 numbers (`G_red`, pad repaints/s); the
  3.2 predictions checked; the 3.3 bar written into the lane report + `.json`; the c3 / c4 decisions written down.
- c2 `perf(s-rta-0929 g4cpu): RoutinePad repaints only when its painted state changes (paintKeyOf); g4 CPU is a gate again (bar over i1) + v5 routine identity`
  -- RED first: the new ctest against the c1 tree (compile error), g4's cadence + bar FAIL on the c1 binary (already
  recorded), v5's teeth (P1 vs P2 shift) on the c1 binary.
- c3 (conditional) `perf(s-rta-0929 g4cpu): the TopBar draws in its own CoreGraphics layer (NativeLayerHost); v2t overlay row`
- c4 (conditional) `perf(s-rta-0929 g4cpu): SignalBar repaints only the strips whose painted state changed; g1 lower bound moves to g4/g5`
- c5 `docs(s-rta-0929 g4cpu): Pitfall 57 amendment + NN, architecture.md, recording.md, APP-INVENTORY, work log rows`
Then the GREEN run: `probe-idle-paint.sh <out>` (ALL rows incl. a1 / g4 / v5 / v2t, `IDLEPAINT_APP` = build-lane,
`IDLEPAINT_APP_BEFORE` = the main build), ctest serial, the lane report `.harmony/.reports/s-rta-0929/g4cpu.md`
(a1 BEFORE / AFTER tables, RED / GREEN raw lines per row, the bar derivation, deviations, LOOK list).

---------------------------------------------------------------------------------------------------------------------
## 5. DOCS TEXT (`docs/claude/*.md`; CLAUDE.md untouched -- 24,980 B; see Q4 for the paid alternative)
**`docs/claude/pitfalls.md:121` (57), replace** "(idle card: 21.8 -> 4.5 ms window max, 347 -> 135 ms/s main-thread CPU; 16 4K
images: 16.4 -> 4.2 ms, 500 -> 111 ms/s; a routine playing on 3 layers still costs 163 ms/s -- its band hairlines and V
faders repaint in-peer)" **with** "(idle card: 21.8 -> 4.5 ms window max, 347 -> 135 ms/s main-thread CPU; 16 4K images:
16.4 -> 4.2 ms, 500 -> 111 ms/s; a routine playing on 3 layers: 163 -> <after> ms/s, idle + <gap> -- s-rta-0929 g4cpu,
Pitfall NN)".
**`docs/claude/pitfalls.md` append (NN; Harmony numbers it)**:
NN. **A model-driven widget's change test must compare what it PAINTS, not what it reads -- and JUCE's Slider already
snaps**: `RoutinePad::setSpec` compared `progress01` (a fresh float every 30 Hz tick while a routine plays) and repainted
the ROUTINES pad 30 times a second; its 90x22 rect at the top of the deck unioned with the strips' band / V-fader repaints
(left column, three rows down) and the TopBar wheel into a ~280x480 pt pass on every tick: +44-51 ms/s of main-thread
CPU over idle for one routine on 3 layers (s-rta-0929 g4cpu a1). The pad now repaints on `paintKeyOf` (the sweep as a
pixel width, the bar digits, the frame state, the marks) -- the `LayerStrip::transportViewOf` (I2) / band-hairline (I3)
rule -- and the TopBar <draws in its own layer (c3) | stays in-peer (c3 skipped: measured <n> coincident passes/s)>.
Two facts that save a wrong lever: `Slider::setValue` snaps to the range interval and repaints only when the snapped
value moved (`juce_Slider.cpp` `constrainedValue` / `setValue`) -- the V fader at interval 0.01 already repaints ~7.5/s
under a 0.6-per-8-s ramp, never 30/s; and JUCE culls `drawText` outside the clip, so a text-heavy `paint()` inside a
small union is cheap -- what costs is the UNION's reach (every component it touches) and AppKit's ~0.3 ms per pass
(measured: a TopBar-only pass 0.73 ms with 0.49 of JUCE paint). Coalescing repaint rects or timers into one tick is a
wash (AppKit unions them anyway); the floor of a routine's cost is its ~20 pixel-change ticks/s x (pass base + content)
+ the engine (~idle + 17-20 ms/s); nothing below that without changing the look. Witnesses: `GET /api/debug/ui_paint`
(routine pad / fader / wheel / strip counters), `GET /api/debug/ui_passes` (the last 512 passes' rects + JUCE time),
`.harmony/probe-idle-paint.sh` a1 (attribution), g4 (bar over i1 + cadence), v5 (routine pixels at two injected beat
positions). Guard: `tests/test_routine_pad_paint_key.cpp`.
**`docs/claude/architecture.md:317-329` "UI Painting (macOS)"**: the on-change list becomes "(`LayerStrip::transportViewOf`,
the band hairline, `RoutinePad::paintKeyOf`, `ClipInspector::paintKeyNow`<, per-strip `SignalStrip::updateValue`>)"; the
layer list "SignalBar, WaveformDisplay<, TopBar>"; Witnesses gain `GET /api/debug/ui_passes` (TEST_SERVER: the last 512
passes) and "probe-idle-paint a1 prints who repaints and what each pass costs -- run it before touching a timed repaint".
**`docs/claude/recording.md:98-107` "Surfaces"**, after "through `deriveRoutineDeckView` (`src/ui/RoutineDeckView.h`)": ";
a pad repaints only when its painted state changes (`RoutinePad::paintKeyOf` -- the sweep in pixels, Pitfall NN), a band
only when its hairline width changes".
**`.harmony/APP-INVENTORY.md:204-212`**: after the `ui_paint` sentence: "`ui_paint` also carries the s-rta-0929 g4cpu
counters (routine pad repaints / sweep ticks / sweep paints, LayerStrip fader repaints, TopBar wheel repaints, deck corner,
LayerInspector, param-control and SignalStrip changes<, the TopBar layer's draws / mode>); `GET /api/debug/ui_passes` returns
the last 512 display passes (clip rect in MainComponent coordinates + JUCE paint us) and the last 512 SignalBar tick
change masks, plus the deck / pad-row / strip-column / wheel / inspector rects -- probe-idle-paint a1's attribution."
**Work log** `.harmony/s-rta-0929-work.md`: one row per commit (`| t | kind | item | result |`) with the RED / GREEN raw lines.

---------------------------------------------------------------------------------------------------------------------
## 6. RISKS (strongest counterargument first; what to verify while building)
R1 (INFERRED, the strongest) **The plan halves the gap; it does not reach the idle floor.** Every remaining ms is a pixel
   Boris sees moving (sweep, hairline, V fill) paid at AppKit's per-pass floor, plus the engine. If Harmony wants
   "at the floor", the only honest levers are a look change (fewer distinct change ticks -- e.g. the hairline and sweep
   stepping on the same tick, or a 2-px step) or Option B; both are rulings, not plans. 3.3 (ii) stops the lane if the
   measured residual leaves no teeth.
R2 (INFERRED, medium) **The predicted shares are wrong in detail** -- the timer lock-in (S2) and the per-pass base are
   read from JUCE source and the diag's idle numbers, not measured under g4. That is what c1 is for; c3 / c4 are decided
   on measurement, c2 is right regardless (the pad IS the only 30/s repainter, VERIFIED by `samePad`).
R3 (INFERRED, medium; c3 only) **A TopBar overlay or input path the layer breaks**: an unparented in-peer overlay over the
   TopBar (none found by grep at adf9b8a -- the builder re-greps), the text field's caret / focus (JUCE-drawn child, peer
   keys -- the same shape as the SignalBar's buttons), a Slider popup (none). v2t + v2b + LOOK cover it; c3 is one commit.
R4 (VERIFIED, low) **c2 misses a painted field** -> a stale pad. The key is built from `paintContent`'s reads (2.3) and the
   test pins each field; `tooltip` is handled separately as today.
R5 (VERIFIED, low) **c4's exact-float key never settles for a live input** -> no saving, no harm (every change is a pixel
   change); with a silent input the peak decays below 0.01 in ~3 s and the flash below 0.05 in ~1 s, then the strip is
   quiet. The c1 mask log shows which case the rig is in before c4 is built.
R6 (ASSUMED) **v5's determinism**: the routine clock follows injected `totalBeatCount + beatPhase` with `bpm > 0` in test
   mode (Pitfall 42's statement); the row SKIPs (never PASSes) if `position` does not land, and the identity proof then
   falls to the cadence counters + LOOK -- report it.
R7 (VERIFIED) **Batch drift of the idle floor** (112-135 across days) -- the relative bar; both arms in one invocation.
R8 (VERIFIED, rig) the graphify post-commit hook loads the CPU; a compiler in the window taints the launch (re-run).

---------------------------------------------------------------------------------------------------------------------
## 7. OPEN QUESTIONS FOR HARMONY
Q1 Accept the target as stated: near the floor (~idle + 25-33 predicted, bar from c1 capped at +36), not at it -- or rule
   now on a look change (R1) / Option B so the builder plans for it from c1.
Q2 c3 / c4 are conditional on c1's numbers with the criteria in 2.4 / 2.5 (>= 8 coincident passes/s; >= 5 ms/s). Confirm
   the builder decides from the a1 table without a round-trip, reporting the numbers.
Q3 The g4 gate's cadence guards use the fixture's own constants ([8,16] / [20,36] / [15,30]) -- confirm they are
   acceptable as pass/fail (they are the "same visual cadence" proof; v5 is the "same pixels" proof).
Q4 NN: a new pitfall entry (5.) needs a CLAUDE.md index line (~110 B) the file cannot hold; my recommendation is the
   amendment to 57 plus the NN entry in pitfalls.md WITHOUT an index line (57's index line already says "before adding any
   timer-driven repaint()"). If Harmony wants the index line, PAY by compressing `CLAUDE.md:141` (177 B) to: "**Periodic
   repaints**: a timed `repaint()` costs the whole window (Pitfall 57 / NN): own layer (`NativeLayerHost`) or repaint only
   on change -- compare what you paint." (~150 B) and line `:236` stays; net +~80 B -> also shorten `:232` (53) by
   dropping "(the FX-only trap)" (-18 B) -- still short; so the honest answer is: no index line without a real cut.
Q5 The filed follow-ups (wheel on-change at idle ~-8 ms/s; `LayerInspector::refresh` paint key) -- own lanes?

---------------------------------------------------------------------------------------------------------------------
## COMPACT
plan-g4cpu: g4's +44-51 ms/s over idle is display passes: the ROUTINES pad compares progress01 (a new float each 30 Hz
tick) and repaints 30/s (`RoutinePad.cpp:6-11`); its rect (Main x 254-344, y 150-172) unions with the strips' V faders
(x 126-148, y 194-482; already on-change: JUCE snaps to the 0.01 interval, ~7.5/s) and hairlines (on-change, 9.5/s) and
the 15 Hz wheel (x 329-404, y 4-38) into a ~280x480 pt pass per tick. Levers: c1 attribution (TEST-ONLY counters +
per-pass rect/time log + `GET /api/debug/ui_passes`, probe row a1 -- ranks shares, sets the bar); c2 pad repaint on
`paintKeyOf` (sweep px, digits, state) -> ~11.75/s (always); c3 TopBar in its own NativeLayerHost (only if c1 shows >= 8
wheel+deck passes/s); c4 per-strip SignalBar repaints (only if c1 shows >= 5 ms/s; never moves the gap). Not built: timer
folding / rect coalescing (a wash: AppKit unions anyway), a moving-bits overlay (dual paint paths), V-fill snapping (look
change, ~0 gain), Option B (same win as c3, more surface). Achievable: idle + 25-33 (floor of the look ~+17-20; not the
idle floor). Gates: ctest paint-key (+ strip painted-key if c4); g4 = window max <= 8 AND cpu(g4) - cpu(i1, same run) <=
bar (from c1, cap 36; RED 44-51 on the c1 binary) AND cadence (pad repaints 8-16/s, paints/ticks >= 0.95, bands 20-36,
faders 15-30; RED pad 27-30); v5 routine identity in test mode at two injected beat positions (BEFORE vs AFTER K2, teeth
= the P1/P2 shift confined to the pad row / strip column); v2t (c3). i1/i2/v0-v3 unchanged. Docs: 57 amended, NN,
architecture.md, recording.md, APP-INVENTORY; CLAUDE.md untouched (Q4).

REPORT_FILE: .harmony/.reports/s-rta-0929/plan-g4cpu.md
STATUS: DONE

## HARMONY ADOPTION (s-rta-0929, 09:17) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_26a06f99-8e8). Attacked blind by attack-g4cpu-juce.md and attack-g4cpu-vj.md. c1 + c2 ADOPTED.
- G1 (juce MUST 1, RULING) c3 (TopBar in a NativeLayerHost) is NOT built in this lane — FILED. It moves an interactive,
  focus-taking widget (play / stop / tap / Link / Outputs / the BPM TextEditor) behind a hitTest-nil native view, and this
  rig cannot click-test it (no synthetic input); the gain (a few ms/s) does not buy that risk. Record c1's wheel+deck
  coincident-pass numbers so a future lane can decide with data.
- G2 (juce MUST 2, ADOPT) The g4 bar is derived AFTER the lever decision, from c1's table, counting every landed lever's
  effect on BOTH arms (gap change = delta g4 - delta i1), and re-derived on the final app's numbers before it is frozen; the
  cap stays +36 ms/s over i1 of the same invocation (Q1: ACCEPT "near the floor"; no look change — that would be Boris's
  call and ~15 ms/s does not justify asking).
- G3 (juce SHOULD 4, ADOPT) c1 attributes each display pass by the per-source counters that FIRED in that tick (stamp them
  into the pass record); the rect-union intersection is a cross-check only. Report both and any disagreement.
- G4 (vj MUST 1, ADOPT) c4 (per-strip SignalBar repaints) lands only if c1 shows >= 5 ms/s for it AND with a stale-strip
  gate: after >= 30 driven value changes through the partial-repaint path (TEST-ONLY injected features), the layer's
  pixels equal a from-scratch full paint of the same final state (K2, <= 1/255), plus teeth (skip one strip's repaint ->
  FAIL). Without that gate c4 is filed, not built.
- G5 (vj MUST 2, ADOPT — and the cause is Harmony's packet) There is NO Boris ruling "how they look must not change": that
  phrase was Harmony's own constraint in the planning packet, parenthesised after a BORIS_DECISIONS citation. Strike the
  citation; V-fill snapping stays rejected on its own merits (a look change for ~0 gain). No doc or comment may cite
  BORIS_DECISIONS for it.
- G6 (vj SHOULD 3, REJECT with reason) g4's window-max bar stays 8.0 ms: it is the I3 family bar shared with i1/i2 and was
  set for a shared machine; tightening it now would flake under other lanes' load. This lane's gate is the CPU row.
- G7 (vj SHOULD 4, ADOPT) The cadence guards gain an actual-paint witness: count RoutinePad::paint / LayerStrip band and V
  fader paint() executions (TEST-ONLY counters) beside the repaint() requests; the pass/fail cadence rows read the PAINT
  counts (Q3: cadence guards ARE pass/fail, on paints); the request counts print as INFO.
- G8 (Q5) LayerInspector::refresh paint key (10 Hz unconditional) is IN this lane as c2b if c1 shows it >= 2 ms/s in i1 or
  g4, with the same pixel-identity proof as c2 (v5-style BEFORE vs AFTER at a frozen state + ctest); else filed. The beat
  wheel repaint-on-change at idle: filed.
- G9 (Q4, ADOPT the recommendation) Pitfall 57 amended + NN entry in docs/claude/pitfalls.md; NO CLAUDE.md index line
  (CLAUDE.md untouched). vj NIT 5: fix the word "unconditionally" (TopBar.cpp:325 guard).
- G10 Existing probes re-run on the final app, never re-thresholded: probe-idle-paint (all rows), probe-routines
  (ROUTINES_BUILD_DIR, pause 1.8), probe-routine-display, probe-deck-tabs; ctest serial. Another build lane (asyncload,
  MainComponent.cpp) runs concurrently — keep your MainComponent edits minimal and in their own hunks (a rebase is likely);
  diag-vfps shares the live lock. Take perf numbers only via acquire_quiet_lock.

## HARMONY ADOPTION ADDENDUM 2 (s-rta-0929, 14:02) — the 3.3 (iii) STOP ruled (overrides G2 / G7 where they differ)
Lane state: c1 ad35de1 (TEST-ONLY attribution) + v5 2df6876 (routine identity row, RED main-vs-main by absence, GREEN 0 px)
+ report c1d216c. c2 measured once uncommitted: g4 -21.7 ms/s (-14 vs another invocation), idle + 28.6.
- G11 (RULING option (a)) Land c2 (the scratchpad draft g4cpu/c2draft + its ctest test_routine_pad_paint_key.cpp) with the
  g4 CPU staying an INFO line (J2 stands): the rig's between-invocation drift (~8 ms/s) equals the teeth, so a CPU bar would
  be a flaky gate. c2's correctness proof is pixel identity (v5, 0 px at both beat positions) + the paint-key ctest (RED by
  absence + teeth: drop one key field -> FAIL). The CPU benefit is REPORTED from INTERLEAVED arms (c1 app vs c2 app,
  alternating launch by launch, >= 5 per arm, same invocation, i1 + g4 each; the diag-vfps run_mix.sh pattern), as INFO.
- G12 (G7 amended) Cadence pass/fail rows measure MISSED VISUAL UPDATES, not absolute paint rates (paints beyond requests
  are union side effects): (1) pad repaint REQUESTS in [8, 16]/s (the lever's direct effect; c1 reads 29.7 -> RED);
  (2) sweep paints / sweep pixel-change ticks >= 0.95 (every tick whose painted sweep changes gets a paint); (3) band and
  V-fader paints >= their change ticks x 0.95. Absolute paint rates print as INFO.
- G13 Bound-knob region: c2 touches only RoutinePad; UniversalParamControl code is untouched by c1 / c2 (show the diff
  stat), so no knob pixel can move — recorded as the argument, no hook arm.
- G14 (No Unexplained Residue) The ~700 ms/s outlier launches (R2: one per arm in the c2x invocation, both apps) — in the
  interleaved batch record per launch the main-thread CPU by run-loop phase (the c1 tools) and the load average; if an
  outlier recurs, name its phase / the process that ran; if not, file what was established and ruled out with the
  cheapest next test. Never drop an outlier launch from a median silently (print it).
- G15 Docs as the plan (57 amended + NN in pitfalls.md, no CLAUDE.md line, G9); the c5 LOOK list goes to the report.
  Existing probes re-run on the final app, never re-thresholded (probe-idle-paint all rows, probe-routines, probe-routine-
  display, probe-deck-tabs); ctest serial.
