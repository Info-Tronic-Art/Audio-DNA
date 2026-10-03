# FACTS: envelope editor (bf45) + drawable Timeline curve (bf6) -- s-rta-1003

Read-only fact sheet. Source tree: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (commit a7491d4, lane/bf9b).
All `file:line` below are in that tree unless prefixed `PLAN`/`RULING` (= /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b/plan-bf45.md, ruling-bf45.md, plan-bf6.md, ruling-bf6.md).
Labels: VERIFIED = read (cite) / INFERRED = reasoned / UNKNOWN-NEEDS-A-RUN = needs an app run (cheapest test named).
Precedence (PLAN-bf45.md:741-749, PLAN-bf6.md:765-773): HARMONY ADOPTION > ruling > plan body. Ruling-bf45 amendments AM1-AM30 override the plan.

Boris, verbatim (boris-feedback-backlog.md "Boris feedback of 2026-10-03"):
- "How do I access the bigger envelope, editor? I don't think I need it? I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window."
- "Long samples: squeeze the whole sample into the envelope length (up to 10 minutes), or pick a section? Default: the whole sample. I want to be able to use the whole thing or to shorten it manually but clicking to timing marks."
- "Explain this to me. Where do I draw the timeline curve? [ After this fix, "Clip Position" and "Timeline" do the same thing: the knob goes from its low end to its high end as the clip plays. Once you can draw the Timeline curve, Timeline follows your drawing and Clip Position stays a straight line. Keep both, or remove Clip Position? Default: keep both.]"

Binding record (binding-decisions.md:608ff, "2026-10-03 (s-rta-1003)"): "NO bigger envelope editor (SUPERSEDES 2026-10-02 BF4 "bigger envelope"); every envelope control lives in the Signal tab's editor; a long sample is used whole or trimmed by hand on timing marks." Backlog BF17 / BF19 / BF27.

---------------------------------------------------------------------------------------------------

## 0. HEADLINE FACTS

1. VERIFIED: nothing of bf45 or bf6 is built. `git log --grep=bf45` and `--grep=bf6` (all branches) return only two docs commits, c4c4f60 and af96de2 ("plans + rulings + seat papers ... Harmony adoptions"); c4c4f60 is an ancestor of a7491d4. grep of src/ and tests/ in the recon tree for EnvelopePanel / EnvelopeDef / EnvelopeService / SampleEnvelope / bendShape / clipTimelinePosition / ModelClipClock returns 0 files. So there is no "bigger envelope editor" to access today; Boris's question is about a design on paper.
2. VERIFIED: the Signal-tab envelope editor at a7491d4 is PAINT-ONLY (no mouse handler in src/ui/SignalInspector.*; grep "mouse" = 0 hits). Nothing can be dragged, added or deleted.
3. VERIFIED: the "Timeline" picker entry is still a 4-beat ramp on the beat clock, and ConnectionEngine::tick still passes `nullptr` as the clip clock at every site (src/connect/ConnectionEngine.cpp:327, 336, 337, 344, 345, 354, 355, 372). No UI anywhere edits a `ConnSource::Kind::Envelope` curve. The drawable Timeline curve is planned by NO lane (PLAN-bf6.md:152, 232-235 "NOT built here"; PLAN-bf45.md:22-23 "Not in this lane").
4. VERIFIED: in the adopted bf45 design the BIG VIEW is a single stage (S5 = E1 + E2) plus a few "if big view" clauses elsewhere; dropping it removes mostly the moving position line and the bigger canvas. The Signal-tab layout (AM11) already carries every control, with sample actions in the "..." menu.
5. VERIFIED (plan fact): the sample's band "ghost" is 1024 points over the whole file (PLAN-bf45.md:226-228; RULING AM14 :356-357). At 10 minutes that is 0.586 s per ghost point (INFERRED arithmetic, 600/1024). A trim to bar lines cannot be done precisely from the stored ghost on long samples; it needs either a re-read of the file or a finer stored ghost. No trim exists in any plan.

---------------------------------------------------------------------------------------------------

## 1. QUESTIONS ANSWERED

### Q1. The adopted envelope plan: stages, items, and what depends on the big view

Adopted: ruling-bf45.md IN FULL, stage order S1 -> S2 -> S5 -> S3 -> S4 (-> S6) (PLAN-bf45.md:741-747; RULING "FINAL BUILD STAGES"). Status: QUEUED behind in-flight lanes (PLAN-bf45.md:748). VERIFIED (read).

Stages (final, after ruling):

| Stage | Items | What it delivers | Big-view dependency |
|---|---|---|---|
| S1 | A1-A8 + AM1 (Once = idle until START, monotone clock, Resync rules), AM2 (legacy curve mapping), AM3 (JSON golden), AM4 (save+relaunch, missing-name note), AM5 (describeWhere / USED BY), AM20 (sanitizers), AM23 (test-only lint), AM24, AM26, AM28 | Model + persistence + undo + registry sync, no new UI. A1 `Breakpoint::bend`; A2 `EnvelopeDef` + `Composition::envelopes`; A3 `EnvelopeSignal` evaluates the def (Loop/Once); A4 commands; A5 `ConnVisitor`; A6 `EnvelopeService` + `syncAll`; A7 old controls write through the service; A8 TEST-ONLY REST | none |
| S2 | B1-B5 + AM6 (keys/focus), AM7 (real-gesture tests G-1..G-12), AM8 (targets/diamonds/hints), AM9 (words), AM10 (colours), AM11 (tab layout), AM21 (SrcEnvelope paint source), AM25 (P1-P8) | The editor itself: `EnvelopeEditLogic` (hit test, snap, clamps, add/delete, bend handle, keys), `EnvelopeCanvas`, `EnvelopePanel`, `SignalInspector` hosting, TEST-ONLY `/api/debug/envelope_ui` | PARTLY (see list below) |
| S5 | E1 (big view, per AM12) + E2 (playhead, per AM13, now REQUIRED) | The BIG VIEW. THIS is the stage that builds it | ALL of it |
| S3 | C1-C2 + AM14, AM15, AM16 | Offline sample analysis (decode, band split, RMS, ghost, RDP) + loader pool. Headless | none |
| S4 | D1-D3 + AM15 (UI), AM17, AM18, AM19 | Sample UI: LOAD SAMPLE, BAND, DETAIL, MAKE LOWS/MIDS/HIGHS, RESET TO SAMPLE, drop onto canvas, file label, notices | PARTLY (buttons vs menu entries) |
| S6 | F (optional, only if Q6=yes) | Envelopes travel with saved decks | none |

Which stage builds the BIG VIEW: S5 = E1 + E2 (PLAN-bf45.md:517-537; RULING AM12 :325-343, AM13 :341-343). VERIFIED. Not "Stage E" in the ruling's table; the ruling renamed the order to S5 and made E2 required. Merge order S1 -> S2 -> S5 -> S3 -> S4 (RULING "FINAL BUILD STAGES"); "BF4 delivered = first Boris checkpoint" is after S5 (AM27, RULING :450).

Items in OTHER stages that assume the big view exists (VERIFIED by reading; each with cite):
1. Moving position line "only in the big view": PLAN F-8 (PLAN-bf45.md:188-192); E2 (:531-537); RULING AM13 (:341-343) "E2 moves from OPTIONAL into S5"; UX-A8 ruling rejects an in-tab cue (RULING :122-127); Q2 (RULING :608-609); gates G4.2, G6.2 v6; probe `/api/debug/ui_passes` SrcEnvelope check. Also AM1 `lastPhase(uid)` "for E2" (RULING :212).
2. Editing precision: PLAN F-3(d) (:151-152) says the tab canvas (~430 x 150-250 px) is "too tight for sample envelopes": a 4-bar envelope at 1/4-beat snap = 64 columns of 6.7 px. INFERRED extension: 16 bars (the longest LENGTH) at 1/4 beat = 256 columns = ~1.6 px per snap step on a 413 px canvas. The precision aids that do NOT need the big view: Shift = fine drag at 1/10 speed on both axes, Cmd = no snap, arrow keys (one snap step; Up/Down 0.01 / 0.1 with Shift), Tab to next point (PLAN B1 :351-367; AM6 :256-272). Snap lines drawn only where >= 6 px apart (PLAN B2 :373-374).
3. Esc behaviour: PLAN B1 (:363) "Esc: deselect. With nothing selected, Esc = close the big view"; RULING AM6 (:268-269) "Plain Esc with nothing selected: MainComponent's SwallowEscape branch (MainComponent.cpp:3854 in the old tree) closes the envelope big view when it is open and still returns true; outputs untouched". Without the big view, plain Esc with nothing selected does nothing (it is already swallowed). The selected-point case (Esc = deselect) is independent.
4. Header buttons: PLAN B3 header row `[envelope picker ComboBox][NEW][... menu][BIG VIEW / CLOSE]` (:392-393); RULING AM11 (:314-317) tab header = picker, NEW, "...", BIG VIEW, USED BY line. Test P5 "header NEW / ... / BIG VIEW are adjacent" (RULING :571). The big view also shows a title row "ENVELOPE CREATOR" (AM9 :305-306) and a Signal-tab placeholder "Editing in the big view" (PLAN E1 :525).
5. Hints: AM8 (RULING :298-299): two hint lines only in the big view; in the tab the text is the canvas tooltip and "there is no hint line".
6. Sample actions: AM11 (RULING :316-317) in the tab, LOAD SAMPLE..., REMOVE SAMPLE, MAKE LOWS/MIDS/HIGHS and RESET TO SAMPLE "live in the header '...' menu in the tab (buttons in the big view)". AM12 (:337-338): the big view's side layout shows "the full sample section as buttons".
7. Layout bars / tests: AM11 bars (RULING :319-323) include big host 1720 x 356 (canvas >= 1400 x 300) and "big canvas width >= 3.0 x tab canvas width"; E1 RED tests (parent == host after expand, == SignalInspector after collapse, selection survives); `/api/debug/envelope_ui` returns {tabCanvasRect, bigCanvasRect, deckVisible, canvasRepaints} (AM28 :456); gate e13 (RULING :547-550), G6.2 v5/v6, G4.2.
8. Side-by-side panel layout (>= 600 px wide): PLAN B3 :403-404; ruling AM11/AM12 moves it to the big host only; tab layout is the < 600 px stacked grid.
9. Esc/Viewport key issue X3 (AM6): the Signal tab is inside a juce::Viewport that eats Up/Down while its scrollbar shows (RULING :34-38, :256-272). This is a TAB problem (cured by `forwardKey`), not a big-view problem; it stays if the big view is dropped.
10. The big view hides the clip grid: `deckView_->setVisible(false)` while open (AM12 :325-337); closes on CLOSE, Esc, selecting a non-envelope signal, composition swap, SignalBar expand. Boris question Q1 (RULING :605-607) and risk RR3 are only about this.
11. Boris questions Q1 and Q2 and "What only Boris can check" 8.1 (RULING :598-629) assume the big view. Docs: AM30 rendering.md items "the big view over the grid (AM12)".
12. Not big-view: stages S1, S3, most of S2 (B1, B2, B3 except the BIG VIEW button, B4, B5 except `expand`), S4 logic, S6.

What is LEFT if the big view is dropped (INFERRED from the lists above; no design offered):
- Everything except S5 and its clauses: data model, persistence, Once/START, undo, USED BY, the whole editor (drag / add / delete / bend / snap / keys), the sample analysis and sample UI.
- The Signal tab hosts the one editor; the plan already has the tab layout (AM11) with pre-registered bars: tab host 413 x 439 -> canvas >= 400 x 300 (>= 400 x 270 with a sample); host 413 x 200 -> canvas height == 160 and the panel scrolls (preferred height > 200). VERIFIED these are the plan's bars; the 413 x 439 host size is the ruling's rig computation (V6) at a 1728 x 1052 window (INFERRED for any other window).
- Lost: the moving "where is it now" line (no home; see Section 2 constraints), the 3x wider canvas, the 2-line hint, buttons instead of "..." menu entries, the two Boris questions Q1/Q2, gates G4.2 / G6 v5-v6 / e13.
- The value is still visible live: each envelope has a SignalBar strip showing its live value at 30 Hz (RULING V14 :84-86; SignalStrip.h:22-25 per ruling). VERIFIED (ruling), not re-read here.

Controls that would have to fit in the Signal tab's editor (all already in AM11's tab layout; VERIFIED list from RULING :312-317 and PLAN B3/D1):
- Header row: envelope picker, NEW, "..." (Duplicate, Rename, Delete, sample actions), USED BY line.
- Canvas (>= 160 px tall) with ruler, grid, curve, point squares, dim/full bend diamonds, ghost, drag readout.
- Grid of controls: LENGTH | SNAP; PLAY [LOOP|ONCE] [START] | HEIGHT; OFFSET | DETAIL; one BAND row (only with a sample); a file label "kick_loop.wav  1.9 s = 3.8 beats at 120 BPM"; notice line (only while live).
- Missing from every plan and therefore also from the tab layout: any TRIM control (start / end marks on the sample) and any marks drawn over the sample's own time (see Q3).

### Q2. The Signal tab's envelope editor at a7491d4

Component: `SignalInspector` (src/ui/SignalInspector.h:15, .cpp), hosted as `signalInspector_` inside `signalViewport_` (a juce::Viewport) in `InspectorPanel` (src/ui/InspectorPanel.cpp:38-41; InspectorPanel.h:96, :102). VERIFIED. Reached by clicking a SignalBar strip (SignalStrip::mouseDown -> onSelected, src/ui/SignalStrip.cpp:87-91 -> SignalBar::onSignalSelected -> MainComponent.cpp:640-643 `inspectorPanel_->inspectSignal`).

Size VERIFIED: canvas = fixed `kCurveEditorHeight = 100` px (SignalInspector.h:66), full content width; content width = inspector width - 12 (InspectorPanel.cpp:88-89, :228-229 in refresh); inspector = 25 % of the bottom row (vDividerFrac_ {0.22, 0.50, 0.75}, MainComponent.h:518). Preferred height for an Envelope = 18+4+4+18 + (100+4) + 22*5 + 16 = 274 px (SignalInspector.cpp:302-309; INFERRED arithmetic from the constants). Tab bar 26 px (InspectorPanel.h:108). Content height does not follow the viewport height (no `setAvailableHeight`; that is a bf45 item B4).

What it can edit today (VERIFIED, SignalInspector.cpp):
| Control | Code | Does it work? |
|---|---|---|
| Curve combo Linear / Exponential / S-Curve (global, all segments) | :75-85 -> `setCurveType` | Yes. t, t*t, smoothstep on the interpolation factor (EnvelopeSignal.h:90-101) |
| Length combo 1/2/4/8/16 beats (no bars) | :87-101 -> `setBeatDuration` | Yes. Length is NOT 1/2/4 bars; no "bars" labels |
| Amplitude slider 0-1 (ResettableSlider, default 1) | :103-109 | Yes (scales output) |
| Phase slider 0-1 (ResettableSlider, default 0) | :111-117 | Yes (cycle-fraction offset, EnvelopeSignal.h:63) |
| "Looping" toggle | :119-126 -> `setLooping` | NO EFFECT. `cyclePhase` is already `fmod`-ed into [0,1) (EnvelopeSignal.h:60-63), so `min(cyclePhase, 1.0f)` (:66-69) never changes it |
| "One Shot" toggle | :128-135 -> `setOneShot` | NO EFFECT, same line |
| Points: drag / add / delete | none | NO. No mouse handler; `setPoints` / `addPoint` have no callers (grep over src/ and tests/: 0 hits outside the class). Only the three fixed points (0,0)(0.5,1)(1,0) set in the constructor (EnvelopeSignal.h:25-28) |
| Segment bend / per-segment shape | none | NO. One global CurveType only |
| Length in bars, play once with START, snap, selecting an envelope, new/duplicate/rename/delete | none | Do not exist |
| Save / load | none | NO. "no EnvelopeSignal field is [serialized]" (EnvelopeSignal.h:141-145); Composition::toVar has no envelope key (PLAN-bf45.md:44-45 F7; not re-read in Composition.h here -> INFERRED from the plan + the header comment) |

Only ONE envelope exists: registry signal "Mod 2" = `EnvelopeSignal("Mod 2", 4.0f)` (src/signal/SignalRegistry.cpp:69-73). The SignalBar [+] menu only un-hides hidden signals (SignalBar.cpp:181-228 in the plan; read here at :186-230: items are hidden signals only, "All signals visible" otherwise). A parameter uses an envelope through the picker's "Envelope" submenu (src/ui/UniversalParamControl.cpp:489-508), which becomes a `Kind::Signal` connection keyed by name (:637-651).

Header text says "ENVELOPE SETTINGS" (SignalInspector.cpp:187); header comment still claims "draggable points" (SignalInspector.h:13) but APP-INVENTORY.md:75 and the code say paint-only. VERIFIED.

Constraints VERIFIED:
- Viewport: the Signal tab sits in a juce::Viewport (InspectorPanel.cpp:38-41, scrollbars vertical only `setScrollBarsShown(true,false)`). NativeLayerHost.h:18 (read at :19 in the file): "Never attach a widget that lives inside a juce::Viewport: a native view is clipped by the peer's view only."
- Pitfall 57 (docs/claude/pitfalls.md:121): JUCE 8's mac peer repaints the UNION of every dirty rect in one drawRect; a timer-driven repaint anywhere costs the whole window; an always-animating widget must use a NativeLayerHost layer or repaint only when its pixels change. Pitfall 59 (:127): a model-driven widget's change test must compare what it PAINTS, not what it reads. Pitfall 41 (:91): a widget showing a model value must follow the model from a timer.
- Consequence (INFERRED from NativeLayerHost.h + Pitfall 57; also the ruling's UX-A8 ruling, RULING :122-127): a moving playhead inside the Signal tab has no legal home today. The ruling says: the marker would move every tick (all registry signals are evaluated each 120 Hz tick, RULING V2) giving continuous in-peer repaints, and the native-layer cure is barred in a Viewport.
- The InspectorPanel refresh runs ~10 Hz (MainComponent.cpp ~4154 per the plan; not re-read here).

### Q3. Sample envelopes: length, bands, where a manual TRIM would attach; timing marks over the sample

What the plan specifies (VERIFIED, plan/ruling text):
- Length of the analysed file: capped at 10 minutes (`capSeconds = 600`), `Truncated` status and notice "Long sample: the first 10 minutes were used" (PLAN C1 :441-443; D1 notices :497-503); min 0.05 s ("Too short ..."). RULING C1-5: a 3 s source with cap 2 -> Truncated, durationSec == 2.
- Whole sample squeezed into the envelope's LENGTH. Boris Q5 default "the whole sample" (PLAN Q5 :734-735; RULING Q5 :612-613). "The shape is analysed in the sample's own time, then stretched to LENGTH, so its attack and decay stretch with it" (AM18 :401-404 / AM30). LENGTH choices 1, 2, 4, 8, 16, 32, 64 beats = 1 beat, 2 beats, 1 bar ... 16 bars (PLAN B3 :395-396).
- Bands: Whole, Lows (<250 Hz), Mids (250-4000 Hz), Highs (>4 kHz) (PLAN C1 :444-453; RULING AM14 :345-360). Time base: fixed 1 ms hop for every length (RULING AM14); per-band normalisation to its own peak; relative -30 dB rule marks a band QUIET but still selectable (AM15). Ghost = 1024 points per band, max-pooled from the 1 ms envelope, embedded in the composition as base64 u16 (~11 KB) and drawn behind the curve; points are an RDP simplification (<= 256 points) controlled by DETAIL (PLAN F-5/F-7, A2). The reference to the file is the absolute path; the file is only needed to re-read.
- "Make Lows / Mids / Highs" creates 3 named envelopes (PLAN D1 :492-493).
- AM17: hand edits protect the points; BAND and DETAIL are disabled once `edited`; RESET TO SAMPLE rebuilds.
- File label shows duration in seconds, beats at the current BPM (AM18).

Timing marks over the sample: NO. VERIFIED (absence). The plan draws a ruler with bar numbers and beat / bar / snap grid lines over the ENVELOPE's cycle (PLAN B2 :373-376), not over the sample. The sample's own time axis is never drawn, no waveform, no marks at the sample's bar lines. The file label (duration "= N beats at B BPM") is the only link to musical time. PLAN Q5 explicitly defers: "picking a section can come later".

Where a manual TRIM (start / end snapped to timing marks) would attach (INFERRED; the plan has nothing, so these are attachment points, not a design):
- Data: `EnvelopeSample` (PLAN A2 :226-228: path, fileName, durationSec, band, detail, silent[4], ghost, + AM17 `edited`). A trim would be new fields on this struct (start/end in seconds or as fractions of durationSec), saved with it.
- Analysis: SampleEnvelope::analyse (PLAN C1 :439-455) currently takes the whole file with `capSeconds`; the 1 ms-hop band envelope is built then reduced to 1024 ghost points and discarded. A trim before the ghost means analysing only [start,end] (the analyser already reads the file in 8192-frame blocks with a cancel flag, PLAN C2 :472-479).
- Resolution problem (INFERRED arithmetic): the stored ghost is 1024 points over the WHOLE file. 600 s -> 0.586 s/point; 60 s -> 58.6 ms/point; one bar at 120 BPM = 2 s = ~3.4 ghost points at 10 min. A trim snapped to bar lines that is re-derived from the stored ghost alone would be coarse on long samples; a trim that re-reads the file works only while the file exists (PLAN F-7 keeps the path; "Sample file not found -- the shape is kept" notice). Which of the two is needed is a design choice for the planner.
- The trim marks would need a time axis for the SAMPLE (bar lines at the current BPM relative to sample start). That axis does not exist in the canvas (it draws the envelope's own cycle); the BPM comes from the live tempo (`FeatureSnapshot::bpm`, src/analysis/FeatureSnapshot.h:40) via the AM18 label. Whether the sample's first downbeat is at 0 is UNKNOWN (no beat detection of the file is planned).
- UI home: the AM11 tab layout has no row for it; the canvas currently shows the envelope cycle, not the sample.

### Q4. The drawable Timeline curve

Decision record: binding-decisions.md:258 "8. BUILD THE DRAWABLE TIMELINE CURVE. Verbatim: "build."" VERIFIED. Also s166 spec D6 (.harmony/specs/s166-universal-connection-architecture.md:701-702: "Timeline = a drawable per-control curve ... [Build it (L5).]") and s167 spec :370-376 (one struct / one editor / one evaluator for a drawn Timeline and a recorded lane; "Same curve editor in both places", header "Ripple > Amplitude > Timeline" vs "Friday take > Layer 2 > Opacity", s167 spec :1100-1102). VERIFIED.

Where would Boris draw it? Nowhere is decided. VERIFIED (absence): no plan or ruling gives the editor a host. What the documents say:
- PLAN-bf6.md:152-162 ("contract for whoever builds the editor"): bf45's editor component is "the natural host" ("the s167-l2 ruling is one struct / one evaluator / one editor for AutomationCurve"); it would edit `ConnSource::env.curve` of the knob's own connection; x axis = the clip from its in point to its out point (labelled in the clip's time or bars/beats); a vertical line at `clipTimelinePosition()`; y = the knob's range after RANGE / INVERT; for a layer knob "the playing clip"; not offered where the picker greys out Timeline; "a small per-knob curve, not the rejected 'Timeline view'" (BORIS_DECISIONS.md:393); an edit must be an undo step.
- But PLAN-bf45.md:22-23 and F-1 (b) (:128-130) explicitly leave out "a per-parameter drawn curve on a connection (ConnSource::Kind::Envelope = Timeline = bf6)" and note only "the editor component edits any AutomationCurve" as a later possibility; and bf6 F7-A (:232-235) says "not in bf6". So each lane points at the other. This is the gap Boris's question 'Where do I draw the timeline curve?' hits. Backlog BF19: "explanation owed; the default (keep both) stands until he says".
- Which tab: no tab chosen. The knob lives in the Clip, Layer or Composition inspector (picker at UniversalParamControl.cpp:511-514; bind sites ClipInspector.cpp:811-862, LayerInspector.cpp:753-767, CompositionInspector.cpp:399-413 per PLAN-bf6.md:87-96); the Signal tab edits registry signals (SignalInspector, by selecting a SignalBar strip) and the Timeline connection is NOT a registry signal. So the Signal tab does not show a Timeline curve. INFERRED from SignalInspector.cpp (it only inspects `Signal*`) + ConnPicker.cpp:90-100 (Timeline is a per-connection `Kind::Envelope`, not a registry signal).

What exists today (VERIFIED):
- `AutomationCurve` (src/connect/AutomationCurve.h:64-123): sorted `Breakpoint{x, y, interp Linear|Hold|Smooth}` with `eval`, `toVar/fromVar`. Used by src/connect/ParamConnection.h, ConnectionShaper.h, src/recording/Lane.h, Program.h.
- `ConnSource::Kind::Envelope` with `env.clock` (Beats | ClipPosition), `env.cycleBeats`, `env.curve` (ParamConnection.h). Evaluated in ConnectionEngine.cpp:123-137: Beats -> `playbackXform(bn/cycleBeats)`; else `clock ? clock->position() : 0` (here clock is always nullptr, so ClipPosition-clock envelopes read position 0).
- Only origin of Kind::Envelope: `sourceFromPicker(Timeline)` = Beats clock, 4 beats, points (0,0)->(1,1) Linear (src/connect/ConnPicker.cpp:90-100); `describeSource` returns "Timeline" for every Envelope (:121-126). The menu item is labelled just "Timeline" (id 3); the code comment above it says "per-parameter keyframes -- placeholder for future" (UniversalParamControl.cpp:~509-514).
- `Kind::ClipPosition` ("Clip Position" menu item id 2): reads `clock ? clock->position() : 0` (ConnectionEngine.cpp:139-143 approx; nullptr -> 0). The registry signal "Clip Position" is hidden, dead (ClipPositionSignal.h, registered SignalRegistry.cpp:74-80, no caller of its update).
- Serialization of an Envelope connection: ConnSerialization.cpp (clock, points) -- exists; no UI to author its points. No test drives an editor.
- Because of that, TODAY: Timeline = a 4-beat ramp driven by the beat clock (not the clip), and Clip Position = always 0. The two do NOT "do the same thing" yet (Boris's quoted explanation describes the state AFTER bf6 Stage 1).

What bf6 Stage 1 changes (VERIFIED, plan + ruling; not built):
- Item 1: wire the existing `ClipClock` seam in `ConnectionEngine::tick` (one `ModelClipClock` per clip and per layer; clip knobs follow their clip, layer knobs follow the clip playing in the layer, the incoming one during a fade; composition/macro/global-effect connections get none); NaN position for a clip with no playhead (image, source, camera, effects-only) -> the knob keeps its hand value; `clipXform` clamps instead of `frac` so a OneShot parked at 1.0 holds the end; position = (playhead - in)/(out - in).
- Item 2: `sourceFromPicker(Timeline)` -> `Envelope{clock = ClipPosition, (0,0)->(1,1) Linear}`.
- Item 3: save marker `"envFormat": 2`; old files: the exact pre-bf6 Timeline pick loads as clip-clocked (RULING AM-3 limits it to clip and layer level); old Signal("Clip Position") loads as Kind::ClipPosition.
- Item 4: retire registry signal "Clip Position".
- Item 5/Stage 2: menus offer both items only where a playhead exists (greyed "video clips only" on stills/sources; not offered on Composition controls); BLOCKED on Boris's Q1 (stills and sources). Item 6: probe-timeline. Item 8 conditional (re-fire flicker, AM-6).
- Result: after Stage 1, Timeline and Clip Position both give a straight 0->1 ramp over the clip. The ONLY difference is that Timeline's curve CAN be edited (once an editor exists). No editor is in bf6 or bf45.
- bf6 is NOT buildable until bf9b merges (contracts C1 playing-clip accessor, C2 the walk, C3 playhead == player at activation; PLAN-bf6.md:765-769, RULING AM-1). At a7491d4 bf9b has changed the walk to `comp.forEachLayer(...)` and `comp.forEachClip(...)` (ConnectionEngine.cpp:326-362) with the same nullptr clock, so P0 is now answerable.

### Q5. Is anything of bf45 or bf6 already on this branch or on main?

VERIFIED (git -C /Users/boriskarpman/projects/RealTimeAudio):
- `git log --oneline -5 --grep=bf45` -> c4c4f60 "docs(s-rta-1002b): Harmony adoptions bf45 / bf1 (D1 mutexes approved) / bf6 (waits for bf9b)", af96de2 "docs(s-rta-1002b): plans + rulings + seat papers bf1 / bf2 / bf6 / bf9 (Stage P) / bf45; diag-bf6; bf2 Harmony adoption".
- `git log --oneline -5 --grep=bf6` -> the same two commits.
- Both are docs-only. `git merge-base --is-ancestor c4c4f60 a7491d4` is true (docs are on the branch); `git branch -a --list '*bf45*' '*bf6*'` shows no bf45/bf6 branch (only lane/bf9b).
- Source at a7491d4: no EnvelopePanel / EnvelopeDef / EnvelopeService / SampleEnvelope / bendShape / clipTimelinePosition / ModelClipClock (grep src + tests = 0 files). Also not present: `Breakpoint::bend`, `Composition::envelopes`, `/api/envelopes`.
- Shared-file overlap that already landed from bf9b and that bf45/bf6 must rebase on: ConnectionEngine::tick walk (forEachLayer / forEachClip), TEST-ONLY `POST /api/debug/undo` and `POST /api/debug/save_composition` (src/api/ApiServer.cpp:326-329). NOTE: ruling-bf45 AM4 / A8 plans to ADD these two routes; they exist already, so a bf45 builder should reuse them (INFERRED from reading both).

---------------------------------------------------------------------------------------------------

## 2. TABLES

### 2a. bf45 items that mention the big view
| Where | Item | Needs the big view? |
|---|---|---|
| PLAN F-3 (:143-152) | one EnvelopePanel, two hosts (tab, big view; window later = third host) | yes (host 2) |
| PLAN F-8 (:188-192), E2 (:531-537), RULING AM13 | playhead in big view only, via NativeLayerHost, 30 Hz timer, repaint only on pixel change | yes |
| PLAN B1 (:363), RULING AM6 (:268) | Esc with nothing selected closes the big view | yes (that clause only) |
| PLAN B3 (:392), RULING AM11 | header [BIG VIEW / CLOSE] button | yes |
| RULING AM8 (:298-299) | 2 hint lines in big view only; tab = tooltip | yes (hint) |
| RULING AM11 (:316-317), AM12 (:337-338) | sample buttons in big view, "..." menu in tab | partly |
| RULING AM11 layout bars (:319-323) | big host 1720 x 356 bars, 3x ratio | yes |
| RULING AM12 (:325-343) | big view = host over deckView_, grid hidden, closes on CLOSE / Esc / non-envelope signal / swap / SignalBar expand | yes |
| RULING AM28 (:454-456) | `/api/debug/envelope_ui` fields bigCanvasRect, deckVisible | yes |
| Gates G3 e13, G4.2, G6.2 v5 / v6; Q1, Q2; "what only Boris can check" 8.1 | | yes |
| PLAN E1 (:519-525) | Signal tab shows "Editing in the big view" | yes |

### 2b. bf6 / drawable curve
| Piece | State at a7491d4 | Planned |
|---|---|---|
| Timeline picker output | Beats clock, 4 beats, 0->1 ramp (ConnPicker.cpp:90-100) | bf6 Item 2: ClipPosition clock |
| Clip clock to the engine | nullptr at all sites (ConnectionEngine.cpp:327-372) | bf6 Item 1 |
| Clip Position | reads 0 (nullptr clock) | bf6 Item 1 |
| Editor for Kind::Envelope curve | none | none in any plan (contract only, PLAN-bf6.md:152-162) |
| Undo for picker change | none (PLAN-bf6.md:113-115 E15, :752 O3) | an editor edit must be undoable (contract) |
| Menu greying | none | bf6 Stage 2, blocked on Q1 |

---------------------------------------------------------------------------------------------------

## 3. WHAT EXISTS TODAY vs WHAT BORIS ASKED (plain gaps, no design)

| Boris | Today (a7491d4) | Gap |
|---|---|---|
| "How do I access the bigger envelope editor?" | None exists, none built. The adopted plan would build one over the clip grid (S5). | His question has a plain answer: it is not built; the plan he would have been shown to later (BF4) is now reversed by BF27. The adopted plan must be amended (S5 and its clauses). |
| "I want all of the controls to work in that one [Signal tab]" | Tab editor is paint-only; Looping/One Shot toggles do nothing; length in beats only (1/2/4/8/16); one global curve type; only the Mod 2 envelope; nothing saved; no new/duplicate/rename. | Everything in S1+S2 (+S4 for samples) must land in the tab; the plan's tab layout (AM11) already allocates all controls. The tab is 413 px wide on the rig (ruling's computation), canvas >= 160 px. |
| "Use the whole [sample]" | No sample import exists (S3/S4 unbuilt). Plan default is the whole sample stretched to LENGTH, up to 10 min. | Matches the plan default. |
| "or to shorten it manually but clicking to timing marks" | No trim in any plan. No timing marks over the sample (only the envelope's own ruler). Ghost is 1024 points over the whole file (0.586 s/point at 10 min). | New requirement: trim start/end on marks. Needs a sample time axis with bar lines at the current BPM, a place in `EnvelopeSample`, and a decision on re-reading the file vs a finer stored ghost. |
| "Where do I draw the timeline curve?" | Nowhere. No editor for `ConnSource::Kind::Envelope`; today Timeline is a 4-beat beat-clock ramp. bf6 (unbuilt, waits for bf9b) makes it follow the clip but does not make it drawable; bf45 does not include it either. | An editor host for a per-knob curve is unassigned. Boris's Q2 (keep both menu entries) is also unanswered; the default stands. |
| Moving "where is it now" line | Not in the tab (constraint: Viewport + Pitfall 57). Only in the big view (S5/E2) in the adopted plan. | If the big view is dropped there is no home for a playhead; the SignalBar strip shows the live value (ruling V14). |

---------------------------------------------------------------------------------------------------

## 4. UNKNOWN-NEEDS-A-RUN

1. UNKNOWN-NEEDS-A-RUN: the real Signal-tab content size on the rig (the ruling's 413 x 439 is arithmetic at a 1728 x 1052 window). Cheapest test: launch (test build), `GET /api/debug/ui_text` / `/api/debug/envelope_ui` do not exist for this yet; use `POST /api/snapshot` (7070) or a Quartz window capture of the Signal tab on "Mod 2" and measure the 100 px canvas width. No REST route returns component bounds today.
2. UNKNOWN-NEEDS-A-RUN: whether a Viewport-free Signal tab (or an overlay anchored in the tab) could host a NativeLayerHost playhead without the clipping defect. Cheapest test: after S2, attach a layer to a child of the Signal tab and check `/api/debug/ui_passes` + a Quartz capture. (NativeLayerHost.h:18 forbids it today.)
3. UNKNOWN-NEEDS-A-RUN: how precise a bar-line trim from a stored 1024-point ghost is on a 60 s and a 10 min sample. Cheapest test: a unit test on `SampleEnvelope::analyse` once S3 exists (synthetic click track at 120 BPM, 600 s, trim to bars 8-16, compare to re-analysis of the trimmed range).
4. UNKNOWN-NEEDS-A-RUN: that the sample's downbeat sits at time 0 so that bar lines at the current BPM line up (needs a real loop file; no beat detection of the file is planned). Cheapest test: load a click-track WAV from .harmony/gen-click-wav.py with a known offset and compare to bar lines.
5. UNKNOWN-NEEDS-A-RUN: the AAC priming trim (RULING RR5): the reader may keep ~44 ms of priming, shifting a trim point. Cheapest test: ruling's e6 m4a row (afconvert -f m4af -d aac), lag <= 5 ms.
6. UNKNOWN-NEEDS-A-RUN: that the bf45 `EnvelopeCanvas` hosted inside `signalViewport_` behaves under key/scroll handling (Viewport eats Up/Down; RULING X3 is read from JUCE source, not run). Cheapest test: RULING AM7 G-11 (unit, real MouseEvent/KeyPress).

---------------------------------------------------------------------------------------------------

## 5. FILES A BUILDER WOULD TOUCH (paths only; absolute, recon tree)

Dropping the big view and putting everything in the Signal tab:
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/SignalInspector.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/SignalInspector.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/InspectorPanel.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/InspectorPanel.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/SignalBar.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/signal/EnvelopeSignal.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/signal/SignalRegistry.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/signal/SignalRegistry.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/connect/AutomationCurve.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/model/Composition.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/MainComponent.cpp (constructor SignalBar hookup :640-643, swapCompositionModel end; NOT the resized() bottom block if no big view)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/api/ApiServer.cpp (TEST-ONLY block :299-329; /api/debug/undo and /api/debug/save_composition already present)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/test/TestServer.cpp (handleListSignals :1019-1043)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/LookAndFeel.h (4 new colour constants, AM10)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/src/ui/UiPaintCounters.h (SrcEnvelope, AM21)
- New (from the plans): src/signal/EnvelopeDef.h, EnvelopeService.h/.cpp, SampleEnvelope.h/.cpp, SampleEnvelopeLoader.h; src/core/EnvelopeCommands.h; src/connect/ConnVisitor.h/.cpp; src/ui/EnvelopeEditLogic.h/.cpp, EnvelopeCanvas.h/.cpp, EnvelopePanel.h/.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/CMakeLists.txt and /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon/tests/CMakeLists.txt

Only if the big view were kept: src/MainComponent.cpp resized() bottom block (deckView_ bounds, ~:2693, binding overlays ~:2765-2777), MainComponent.h (envelopeHost_), UiPaintCounters.h layer id, NativeLayerHost usage.

Drawable Timeline curve (not in any plan; likely set): src/connect/ConnPicker.cpp, ConnectionEngine.h/.cpp, ConnectionShaper.h/.cpp, ConnSerialization.cpp, src/ui/UniversalParamControl.h/.cpp (the Timeline menu item/host), ClipInspector.cpp, LayerInspector.cpp, CompositionInspector.cpp, EffectStackView.h/.cpp, plus the AutomationCurve editor component.

Trim (not in any plan): EnvelopeDef.h (EnvelopeSample), SampleEnvelope.cpp, EnvelopePanel/EnvelopeCanvas (a sample time axis), the ghost embed format.

---------------------------------------------------------------------------------------------------

## 6. EXISTING TESTS / PROBES AND REST ROUTES THAT COVER THIS AREA

Existing tests at a7491d4 (VERIFIED by listing tests/):
- tests/test_oscillator_bar_fold.cpp -- includes signal/EnvelopeSignal.h; holds the Mod 2 regression anchors (PLAN-bf45.md:115-116: :256-307, :420-451, :729-730).
- tests/test_connection.cpp -- AutomationCurve section (:617ff: eval Linear/Hold/Smooth, clamp, smoothstep) and the Envelope connection cases (clipPosition clock round-trip around :655-673 per PLAN-bf6.md:100-101).
- tests/test_conn_picker.cpp -- :63-72 pins the Timeline = Beats-clock ramp; :115-121 describeSource "Timeline".
- tests/test_master_signal_link.cpp, tests/test_signal_depth.cpp, tests/test_take.cpp, tests/test_take_v1_transport.cpp (take / routine curves).
- No test of any envelope editor, `SignalInspector`, `InspectorPanel` Signal tab, or `ClipClock` wiring (planned: test_envelope, test_envelope_ui, test_sample_envelope, test_timeline_clock, test_timeline_picker).
- Existing UI-test patterns to reuse (VERIFIED by ruling V5): tests/test_layer_strip_follows_model.cpp:26-32 (real juce::MouseEvents headless).

Existing probes (VERIFIED by listing .harmony): probe-capture.{sh,py,json} (template for probe-envelope per PLAN G3), probe-idle-paint.{sh,py,json} (i1/i2/g4 idle paint gate), probe-tsan.sh / probe-tsan-unit.sh, probe-deck-tabs.sh (menu snapshot hook pattern), probe-lane3.sh + probe-lane3-clip/layer/comp.json (connection fixtures with clip "conns"), probe-mastersignal.sh, probe-crossfade.{sh,py,json}. Not present: probe-envelope, probe-timeline (planned).

REST routes that can drive this area WITHOUT synthetic input (VERIFIED in src/api/ApiServer.cpp:163-344 and src/test/TestServer.cpp:128-265):
- 8080 (test server) GET /api/signals (list incl. "Mod 2" with its live cached `value`; reads the registry on the httplib thread, TestServer.cpp:1019-1043) -- the only way to read an envelope's value. No route returns envelope POINTS / length / mode today.
- 7070 POST /api/inject_features (test mode; sets beatPhase / beatInBar / barCount / totalBarCount / resyncBarOrigin, ApiServer.cpp:237) -- drives the beat clock so an envelope can be sampled at chosen beats; with GET /api/signals this verifies Mod 2 as a function of beat.
- 7070 POST /api/load_composition, GET /api/composition (clips' playheadPosition and the clip / layer / composition `live` scalars; effect param effective values -- PLAN-bf6.md:120-125), POST /api/set_param, /api/set_clip_param, /api/trigger_clip, /api/switch_deck, POST /api/debug/undo and /api/debug/save_composition (TEST-ONLY, #if AUDIODNA_TEST_SERVER, lines :326-329), /api/debug/ui_paint, /api/debug/ui_passes, /api/debug/ui_text, POST /api/snapshot, POST /api/render_frame (picture check).
- 8080 POST /api/add_route / /api/remove_route / GET /api/routes, /api/add_mapping, /api/set_macro -- legacy mapping routes; not the connection source picker.
- Timeline test route: none. A Timeline knob can be set only by a composition fixture whose "conns" contain an envelope source (PLAN-bf6.md:503; `.harmony/probe-lane3-clip.json` shape) loaded with /api/load_composition, then read back from GET /api/composition `live` values. Clip position needs a real video clip (fixtures via ffmpeg lavfi, PLAN-bf6 Item 6).
- Missing routes the plans add (TEST-ONLY): GET /api/envelopes, POST /api/envelope/edit /manage /sample /start, GET|POST /api/debug/envelope_ui (PLAN-bf45 A8, B5, D3; RULING AM28).
- Pitfall 57 gate: GET /api/debug/ui_passes + probe-idle-paint.sh (planned `SrcEnvelope` stamp, AM21).
