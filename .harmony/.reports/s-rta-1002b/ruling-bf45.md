# RULING lane bf45 (s-rta-1002b) -- blind-council attacks on plan-bf45.md (BF4 envelope editor + BF5 sample envelopes)

Architect (Fable), 2026-10-02. Read-only. HEAD fa9604d / 9a832a0 (the plan cites 5e47d17; every line cited below was
re-read at HEAD). Seat papers, verbatim: .harmony/.reports/s-rta-1002b/attack-bf45-papers.md.
Scratch runs: python3 + numpy (no scipy, no app launched), session scratchpad
/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/
(envdsp.py = the plan's C1 DSP as written: RBJ Butterworth biquads at 48 kHz, LR4 = 2 x BW2, "2 x LR4" = 4 x BW2, 20 Hz
BW2 high-pass, RMS windows 40/20/10/20 ms, peak-hold release tau 120/80/60/80 ms, 1024-point max-pool ghost, RDP on
vertical distance with the plan's eps rule). Labels: VERIFIED = read at HEAD or run; INFERRED = derived, not run.

## 0. VERDICT

The plan is sound in shape and is AMENDED: 34 of 36 attacks accepted (several in part), 2 rejected (SIG-A11,
GATES-A12). Amendments AM1-AM30 below OVERRIDE the plan body. Ready to build.

Six defects no seat named were found while ruling and are folded in:
- X1 (AM16) C1 test 1's bar "lows mean <= 0.05 in the second half" fails every CORRECT build: the plan's own 120 ms
  release tail averages 0.093-0.123 over that half (scratch, 6 variants).
- X2 (AM14) a sample that starts mid-waveform (common: loops cut at a non-zero sample) puts Mids at -16.4 dB and Highs
  at -24.9 dB under the Lows of a pure 60 Hz file with zero-state filters -- both "not silent" under the -30 dB rule,
  so a bass-only file shows bogus Mids / Highs envelopes. A mirror pre-roll removes it (-98.9 / -231 dB).
- X3 (AM6) a juce::Viewport consumes Up / Down / PageUp / PageDown / Home / End while its vertical scrollbar shows
  (juce_Viewport.cpp:626-639) and the Signal tab lives in one (InspectorPanel.cpp:41-43): plan B1's "every other key
  returns false, so the launcher still works" is not true there.
- X4 (AM12) the plan's big view is as tall as the bottom row, which loses 96 px per layer (DeckView.cpp:326-340,
  MainComponent.cpp:2650-2700): ~507 / 411 / 219 px at 3 / 4 / 6 layers on the rig with the Dock hidden -- "bigger"
  fails exactly on big sets.
- X5 (AM12) an opaque panel laid OVER the clip grid is still painted whenever a covered LayerStrip playhead repaints
  (JUCE paints the opaque sibling's intersecting region) -- INFERRED from JUCE's paint traversal, not measured. The grid
  is hidden, not covered.
- X6 (AM30) Pitfall 64 is now assigned to mkvidx (.harmony/s-rta-1002b-work.md:18); this lane's pitfall takes the
  next free number.

## 1. EVIDENCE RE-DERIVED FOR THIS RULING

- V1 VERIFIED. Once today: cyclePhase is fmod'ed (EnvelopeSignal.h:60-63), so min(.,1) is a no-op (:66-69);
  getValue is const (:31); the default curve is Linear (:154); the only setCurveType caller is SignalInspector.cpp:82.
- V2 VERIFIED. Every registry signal is evaluated every 120 Hz tick, used or not (SignalRegistry.cpp:166-178,
  MainComponent.cpp:4030-4037): a Once pinned "on the first evaluation" fires at launch / load.
- V3 VERIFIED. The house message-thread beat clock is raw = totalBeatCount + beatPhase with a decrease absorbed as
  continuity (RecorderClock.h:22-32; Pitfall 42, pitfalls.md:93); totalBeatCount is never reset or rewound
  (FeatureSnapshot.h:127-137). A manual Resync is an explicit published event: resyncBarOrigin, written only by
  BPMTracker::applyResync (FeatureSnapshot.h:118-126). Boris's ruling call 9: a MANUAL Resync re-aligns tempo shapes
  to the new downbeat (binding-decisions.md:534-543); gateOnce re-pins a once connection on it
  (ConnectionEngine.cpp:44-69). bf2 shifts the PUBLISHED beat fields and keeps totalBeatCount monotone
  (plan-bf2.md:150, :514-516, :667), so a reader of totalBeatCount + beatPhase follows the sync dial automatically.
- V4 VERIFIED. JUCE bubbles keyPressed / keyStateChanged up the parent chain (juce_ComponentPeer.cpp:196-221,
  :235-262), BUT a Viewport in the chain consumes Up/Down/Page/Home/End while its vertical scrollbar shows
  (juce_Viewport.cpp:610-639), and an unconsumed Tab moves focus (juce_ComponentPeer.cpp:223-228). Launcher keys are
  user bindings run last in MainComponent::keyPressed (:3910-3920); arrows, Tab and Space are bindable
  (BindingOverlay.cpp:164-165, :238-250); src has no default bindings (grep). Plain Esc is swallowed and never touches
  outputs (MainComponent.cpp:3854). Chords handled before bindings: Shift+Cmd+K/M, Cmd+Shift+Esc, Cmd+`, Cmd+F,
  Cmd+Z / Shift+Cmd+Z, Cmd+X, Cmd+S, Cmd+O (:3803-3910).
- V5 VERIFIED. Real juce::MouseEvents in headless tests are a house pattern (tests/test_layer_strip_follows_model.cpp:
  26-32, plus 6 other test files), and so are component-snapshot pixel checks (same file, cuePixels).
- V6 VERIFIED (sizes at a given window INFERRED). The window opens maximized to the primary display's user area
  (Main.cpp:64-70); the rig display is 3456 x 2234 Retina = 1728 x 1117 pt (system_profiler). Bottom row height =
  MainComponent height - 189 - deck, deck = 68 + 96 x layers (MainComponent.cpp:2554-2700; SignalBar 84 px,
  SignalBar.cpp:101-110; DeckView.cpp:326-340, DeckView.h:164-169; 3 default layers, Deck.h:25); panels 22/28/25/25 %
  (MainComponent.h:498); tab bar 26 px (InspectorPanel.h:108); content width = inspector - 12 (InspectorPanel.cpp:
  88-101). At MainComponent 1728 x 1052 (Dock hidden): bottom row 507 px, inspector 425 x 507, Signal-tab content
  413 x 481, EnvelopePanel host in it ~413 x 439 (minus the 26-px section header and 16-px pad, SignalInspector.h:
  62-66), deck area 1720 x 356.
- V7 VERIFIED. Take-over precedents: the expanded SignalBar hides every panel incl. deckView_ (MainComponent.cpp:
  2587-2599); BORIS_DECISIONS.md:173 "Bottom Focus Bar -- Click to expand upward".
- V8 VERIFIED. All three composition save callers go through Composition::saveToFile -> toVar (MainComponent.cpp:3406,
  :3448, :6560; Composition.h:677-681). No REST save route exists (ApiServer.cpp:160-341). /api/load_composition is
  a production route (ApiServer.cpp:251).
- V9 VERIFIED. TEST-ONLY routes are compile-gated (#if AUDIODNA_TEST_SERVER, ApiServer.cpp:299-329; option OFF by
  default, CMakeLists.txt:62, definition :463-464; build/ has it ON) and need no --test-mode (ApiServer.cpp:300-301).
- V10 VERIFIED. apply_sanitizers is per target and acts only in a dir configured with ADNA_SANITIZE
  (cmake/Sanitizers.cmake:19-25); build-asan has ADNA_SANITIZE=address.
- V11 VERIFIED. Breakpoint::toVar (AutomationCurve.h:33-48); takes write curves through AutomationCurve::toVar
  (Lane.h:144); the tests/fixtures/take_*.json files hold no curve points (0 "curve" / "interp" keys).
- V12 VERIFIED. juce LinkwitzRileyFilter asserts cutoff < rate / 2 (juce_LinkwitzRileyFilter.cpp:55) and prewarps
  with tan(pi fc / fs) (:146). JUCE's Core Audio reader uses ExtAudioFile (juce_CoreAudioFormat.cpp:419, :549).
- V13 VERIFIED. The analysis thread runs at Priority::high (MainComponent.cpp:1862); low-priority decode pools already
  run beside it (ClipThumbnails.h:107-108, MediaPresence.h:155-156, MediaOpener.h:36); no analysis-hop overrun metric
  exists (grep of src/analysis and ApiServer.cpp).
- V14 VERIFIED. Repaint-source bits uipaint::Src use bits 0-8 (UiPaintCounters.h:76-77); native layers attach via
  NativeLayerHost::attach(comp, *overlayWatch_, id) (MainComponent.cpp:2318-2319); a native layer may not live in a
  Viewport (NativeLayerHost.h:18); each SignalStrip shows its signal's live value from the 30 Hz SignalBar timer
  (SignalStrip.h:22-25).
- V15 VERIFIED. Colour constants (LookAndFeel.h:22-36; kTextSecondary 0xff808090 :28); BORIS_DECISIONS colours
  (:13-19) and hard visual rules (:58-69).
- S1 SCRATCH, two-tone Mids leakage relative to the strongest band (the rule: under -30 dB = silent): a hard switch at
  an arbitrary phase -16.4 / -16.9 dB (NOT silent -> plan C1-1 fails); the naive zero-crossing generator -31.1 dB
  causal (a 1.1 dB margin) / -35.2 dB centred; 20 ms Hann fades on every segment + mirror pre-roll: -68.3 / -72.6 /
  -72.8 dB over three phases.
- S2 SCRATCH. Plan C1-1 "lows mean over the second half" = 0.093-0.123 in every variant (bar <= 0.05 fails); lows MAX
  over x >= 0.75 = 0.012-0.016.
- S3 SCRATCH, file-start step (60 Hz starting at full amplitude): zero-state filters -> Mids -16.4 dB, Highs -24.9 dB;
  taper 5 / 10 / 20 ms -> -41.4 / -56.6 / -69.7 dB; mirror pre-roll 100 ms -> -98.9 / -231 dB. A DC 0.2-only file reads
  0.094 / 0.064 / 0.021 / 0.008 (whole / lows / mids / highs) without the pre-roll, <= 2.5e-5 with it. An all-zero file
  crashed the scratch model on the division by the strongest peak (the SIG-A7 bug class, reproduced).
- S4 SCRATCH, decimation: 60 s with 12 sparse 3 ms 8 kHz bursts: the plan's hop (len / 4096 = 14.65 ms) leaves 8/12
  bursts >= 0.5 with peaks 0.0-1.0 depending on phase; a fixed 1 ms hop gives 12/12 at 1.0. At 10 minutes the plan's
  hop is 146 ms.
- S5 SCRATCH, clicks (gen-click-wav.py burst at 0.25 / 0.75 / 1.25 / 1.75 s of 2 s, centred window, pre-roll, RDP):
  24 / 34 / 44 points at detail 0.1 / 0.5 / 0.9; maxima at x 0.122-0.130 / 0.3803 / 0.6305 / 0.8807 (offsets -0.003 ..
  +0.006 from 0.125 / 0.375 / 0.625 / 0.875); troughs at x 0.25 / 0.5 / 0.75 = 0.051-0.117 (0.052-0.064 at 0.5).
- S6 SCRATCH, AAC: the S1 fixture through `afconvert -f m4af -d aac` and back (Apple's ExtAudioFile path): 96000 /
  96000 frames, lows envelope Pearson r = 1.0000 at lag 0.
- S7 SCRATCH, bend: f(t, 0.282) equals t*t only at t = 0.5 (f(0.25) = 0.0916 vs 0.0625; max |diff| 0.040); the
  minimax bend vs t*t is 0.272 (max 0.033).

## 2. PER-ATTACK RULINGS

UX / VISUAL
- UX-A1 ACCEPT -> AM7, AM25 (V5: the house already drives real MouseEvents headless).
- UX-A2 ACCEPT -> AM11, AM12. Confirmed and worse (X4, V6): the plan's big-view canvas is ~650 px wide vs ~405 in the
  tab (1.6x) and its height collapses with layers. Sub-fixes REJECTED: the whole bottom row (hides the preview, still
  collapses), the floating window now (Boris: "perhaps we don't create its own window till later",
  binding-decisions.md:585-588). "Taller / wider" is folded into Q1.
- UX-A3 ACCEPT -> AM5 (USED BY + named refusal), AM12 (the new host keeps the Inspector visible). The "Timing column +
  part of the Inspector / overlay anchored to the tab" layout REJECTED: smaller, and it does not cure X4.
- UX-A4 ACCEPT -> AM29 (honest answer); AM6's key rules bind any later window host.
- UX-A5 ACCEPT -> AM29 Q7; AM9 (header "ENVELOPE CREATOR").
- UX-A6 ACCEPT -> AM17.
- UX-A7 ACCEPT in part -> AM8. REJECTED: a "?" button (BORIS_DECISIONS.md:67 "No emoji / decorative icons"); a BEND
  mode (dim diamonds make bending discoverable without a mode); raising the ruler contrast (kTextSecondary #808090 on
  #111111 = 4.86:1, above WCAG AA 4.5:1; BORIS_DECISIONS.md:18 labels #888). "Add and delete are double-click only" is
  not so: the plan's right-click menu has Add point here / Delete point (B2) and the Delete key deletes (B1).
- UX-A8 ACCEPT in part -> AM13 (E2 required). The in-tab cue REJECTED: by V2 the marker would move every tick while
  the tab shows an envelope -> continuous in-peer repaints whose rects union with every other dirty rect (Pitfall 57,
  pitfalls.md:121), and the cure (a native layer) is barred inside a Viewport (NativeLayerHost.h:18). The live VALUE
  is already on screen in the envelope's SignalBar strip (V14).
- UX-A9 ACCEPT -> AM6 (X3 is the stronger form; the collision grep result is recorded in AM6).
- UX-A10 ACCEPT in part -> AM9. REJECTED: "DELAY" (collides with BF2's sync dial, "the delay or speeding ahead",
  backlog:10-11); "SMOOTHNESS" (DETAIL + the live point count + a tooltip says it); renaming "Mod 2" or a display name
  (the name is the persistent connection key, ParamConnection.h:37-38; old files connect by it; the Mod 2 anchors
  test_oscillator_bar_fold.cpp:256-307; Rename exists, plan F-9).
- UX-A11 ACCEPT -> AM10.
- UX-A12 ACCEPT -> AM27 (re-order + checkpoints). S1 is not split: every S1 item is an S2 prerequisite.

SIGNAL / AUDIO
- SIG-A1 ACCEPT -> AM1 (idle until START). REJECTED: start on bind (binding happens long before the moment; the first
  bind would fire a shared envelope for every user); start on a clip edge (a registry envelope is ONE shared value per
  tick, V2; per-connection clip re-pinning is gateOnce's lane, ConnectionEngine.cpp:44-47); a "test only" label (START
  is a real panel control; Q4 offers a pad).
- SIG-A2 ACCEPT -> AM1 (monotone clock + explicit Resync event; tests O3-O8; V3).
- SIG-A3 ACCEPT -> AM16 (S1 measured the fragility; the counter-case is made consistent with AM15).
- SIG-A4 ACCEPT in part -> AM15 (quiet bands stay usable; scaling doc). REJECTED: 99th-percentile normalisation (the
  peak-hold follower stretches one pop over its 60-120 ms tau = 3-6 % of a 2 s loop, so the 99th percentile still lands
  on the pop's tail; on drum loops it flattens every accent to 1.0); an absolute dBFS floor (a quiet but clean sample
  would lose every band).
- SIG-A5 ACCEPT -> AM14 (S4).
- SIG-A6 ACCEPT in part -> AM14 (centred windows; mirror pre-roll -- X2 shows the start step hits every band, not only
  the 20 Hz high-pass). REJECTED: explicit group-delay compensation (<= ~3.6 ms for the 2 x LR4 lows, INFERRED
  analytically, under one 16.7 ms frame) and "+-1 ghost point" (a centred 20 ms window over a 5 ms burst has a 15 ms
  flat top = 7.7 ghost points at 2 s, S5) -> +-0.01 around explicit centres instead of +-0.02.
- SIG-A7 ACCEPT -> AM14, AM16 (V12, S3).
- SIG-A8 ACCEPT -> AM2 (S7). Mod 2 itself is Linear (V1), so its behaviour does not change; the regression test
  becomes an error bound.
- SIG-A9 ACCEPT in part -> AM18. REJECTED: an auto-fit LENGTH / beat-aligned import (not asked; Boris accepted
  "stretched to the set beat / bar length", binding-decisions.md:589-590).
- SIG-A10 ACCEPT -> AM29 Q7.
- SIG-A11 REJECT. V13: no hop-overrun metric exists, so the row needs new instrumentation on the sacred analysis
  thread for a NIT; the loader's single low-priority thread sits beside a Priority::high analysis thread exactly like
  the three decode pools already shipping. Kept as residual risk RR6.

GATES / TESTS
- GATES-A1 ACCEPT -> AM26 + gate G1.3 (Harmony runs the RED commits itself).
- GATES-A2 ACCEPT in part -> AM3. REJECTED: the take / routine re-serialisation goldens (V11: the take fixtures hold no
  curve points, and every curve writer delegates to AutomationCurve::toVar, which the literal golden pins).
- GATES-A3 ACCEPT in part -> AM4 (e10 through the real save function + relaunch; the missing-name note). REJECTED: one
  case per save caller (V8: one choke point).
- GATES-A4 ACCEPT -> AM6, AM7.
- GATES-A5 ACCEPT -> AM16 (explicit x), gate e12.
- GATES-A6 ACCEPT -> AM1, AM28 (/api/envelope/start), gate e11.
- GATES-A7 ACCEPT -> AM22.
- GATES-A8 ACCEPT -> AM19 (AAC bar derived, S6), AM21 (deterministic G4), AM24 (TSan scope, e8 count).
- GATES-A9 ACCEPT -> AM21.
- GATES-A10 ACCEPT -> AM20.
- GATES-A11 ACCEPT -> AM25.
- GATES-A12 REJECT. (1) The live analysis and a sample are scaled differently by design (a slowly decaying running max,
  SpectralFeatures.cpp:166-193, vs the band's own peak), so a correlation bar measures the scaling, not correctness,
  and playing a fixture through the live input adds real-time nondeterminism; synthetic ground truth (C1, e6, e12) is
  the stronger oracle. What Boris meant by "live" is a question (Q7), not a gate. (2) "Setup format" is answered:
  Boris "default is good" (binding-decisions.md:585) to the default reading that already included bar durations
  (boris-feedback-backlog.md:43-45); it is recorded as a default taken (AM29).
- GATES-A13 ACCEPT, mechanism replaced -> AM23. By V9 a non-test-mode launch of a test-server build still serves the
  routes, so a 404 row could never pass for the right reason; a source lint is deterministic.

## ARCHITECT RULING (s-rta-1002b)

AM1-AM30 OVERRIDE plan-bf45.md wherever they differ; everything not amended stands.

### Amendments

AM1 Once = idle until START, on a monotone clock (replaces A3's Once bullet; SIG-A1, SIG-A2, GATES-A6).
- Clock: EnvelopeSignal keeps raw = totalBeatCount + beatPhase (double) exactly as RecorderClock (V3); raw < lastRaw is
  a realign that restarted the beat and is absorbed into an offset, so mono = raw + offset never decreases. Mutable
  members, message thread only (getValue is const).
- States Idle / Armed / Running(startMono) / Done.
  - A load, launch, composition swap, or a LOOP -> ONCE change leaves it Idle: value = y(0) x HEIGHT.
  - START (EnvelopeService::start(uid) = the panel's START button = TEST-ONLY POST /api/envelope/start) -> Armed. At
    the next evaluation: barPos = beatInBar + beatPhase; start = mono + (barPos < 1e-4 ? 0 : 4 - barPos) (the next bar
    line; Q3 default) -> Running. START while Running or Done re-arms (restart on the next bar).
  - Running: value = curve.eval(clamp((mono - start) / lengthBeats, 0, 1)) x HEIGHT, y(0) before the start; Done once
    mono - start >= lengthBeats: y(1), held.
  - Manual Resync = resyncBarOrigin differs from the previous evaluation (the first evaluation only records it). A
    Running Once restarts at the Resync downbeat (start = mono - barPos of that tick) -- Boris's ruling call 9 and the
    gateOnce precedent. Idle and Done are untouched: a Resync re-aligns what moves; it never starts what is still.
    (Two Resyncs inside one bar leave the origin unchanged and count once: documented, accepted.)
  - No other heuristic: no backward-jump detection; a Tap, a structural reset (barCount) or a stall never re-pins.
- Loop: the cycle-phase block (EnvelopeSignal.h:35-63) stays verbatim (bf2 may edit it; bf2 shifts published fields,
  V3, so both modes follow the sync dial).
- setDef(def, resetRun): EnvelopeService passes resetRun = true from syncAll after a swap / load and on a LOOP <-> ONCE
  change, false for every other edit (a running Once survives a point edit). Compat setters setOneShot / setLooping map
  to the mode only and never arm.
- lastPhase(uid) for E2: Loop = cycle phase; Running = elapsed fraction; Done = 1; Idle = none (no playhead).
- UI: [LOOP|ONCE] + START (enabled in ONCE; tooltip "Plays the shape once, starting on the next bar"); OFFSET
  disabled in ONCE (as planned). A pad / key for START = Q4 (default not now).
- Tests, test_oscillator_bar_fold [envelope][once], ramp (0,0)->(1,1), 4 beats, values +-1e-4:
  O1 never started: 100 ticks over mono 0..9 incl. a bar line -> every value 0.0. RED on main's own API (main returns
     the looping ramp, 0.25 at beat 1).
  O2 START at mono 5.5 (beatInBar 1, beatPhase 0.5) -> 0.0 at 6.0, 7.9 and 8.0; 0.25 at 9.0; 0.5 at 10.0; 1.0 at 12.0,
     13.0, 20.0.
  O3 a realign in the first half while Running (raw 9.3 -> 9.0): the value never decreases across it.
  O4 a realign in the second half (totalBeatCount +1, beatPhase 0.7 -> 0.0): elapsed advances by exactly 0.3 beat.
  O5 a structural reset (barCount -> 0, totalBarCount continues), resetPhaseOnStructural true and false: a Running
     Once is unaffected.
  O6 a 2.3-beat stall in one tick: elapsed +2.3; a Once with 1.0 beat left lands on y(1).
  O7 a manual Resync while Running: y(0) on that tick, 0.25 one beat later.
  O8 a manual Resync while Idle -> stays 0.0; while Done -> stays 1.0.
  O9 every existing test_oscillator_bar_fold case passes unchanged (Mod 2 anchors :256-307).

AM2 Legacy curve mapping (SIG-A8; corrects F25 and A7). bend +0.282 equals t*t at t = 0.5 only (max |diff| 0.040,
S7). The S1 interim combo maps Exponential -> bend 0.27 on every segment (minimax vs t*t, max error 0.033) and S-Curve
-> Interp::Smooth (exact: both smoothstep, EnvelopeSignal.h:96-97 vs AutomationCurve.h:110-112). No quadratic option is
kept: nothing persisted uses Exponential (F7, V1) and the combo is removed in S2 (B4). Test: |f(t, 0.27) - t^2| <=
0.034 at 101 points. Docs: "the old Exponential became the bend preset 0.27 (within 0.034 of t*t)".

AM3 Literal golden (GATES-A2). The A1 case "a bend-0 curve's JSON equals the pre-change string" compares
juce::JSON::toString(curve.toVar(), true) of the curve {(0, 0, linear), (0.333333, 1, hold), (1, 0.25, smooth)} with a
LITERAL string in tests/test_connection.cpp, committed in the S1 stub-RED commit BEFORE Breakpoint::bend exists. It is
a [guard]: it must PASS at the stub commit (proof it came from main's code) and at GREEN.

AM4 Save + relaunch end to end (GATES-A3).
- NEW TEST-ONLY POST /api/debug/save_composition: callAsync saveComposition() (the Cmd+S / menu Save function,
  MainComponent.cpp:3401, :3897, :6501), answers {ok, path} after the write (2 s Box pattern). One function covers all
  three callers (V8).
- Gate row e10 (in the gate list).
- Missing-name note (unit, test_envelope): a composition with "envelopes": [] and one connection naming "Mod 2" ->
  after fromVar + syncAll the connection reads 0 and envelopeLoadNote says "1 control uses an envelope this set does
  not have: Mod 2" (syncAll walks ConnVisitor for Kind::Signal names that resolve to no registry signal).

AM5 USED BY + a refusal that names the controls (UX-A3). NEW ConnVisitor describeWhere(Where, const Composition&)
returns the names the inspectors show ("Master", "Layer 2 > Opacity", "Layer 2 > Ripple > Amount", "Clip Kick > Zoom",
"Macro 3", ...). EnvelopeService::usersOf(name) returns them. The panel header shows one secondary line "USED BY:
<first two> (+N)" or "NOT USED"; the delete refusal notice reads "Used by Layer 2 > Opacity and 1 more -- disconnect
them first"; REST manage delete returns {ok: false, users: N, usedBy: [...]}. Test: one connection per placement ->
the describeWhere strings equal a list the builder writes from the inspectors' labels in the stub-RED commit.

AM6 Keys and focus (UX-A9, GATES-A4, R4; replaces B1's key bullet and R4's mitigation).
- The canvas wants keyboard focus ONLY while a point is selected: selecting a point grabs focus; deselecting (Esc, a
  click on empty canvas, focusLost, visibilityChanged / parentHierarchyChanged = tab switch, big view open / close)
  clears the selection and calls returnFocus() (MainComponent wires grabKeyboardFocus()).
- Bound keys win: before consuming any key the canvas asks isLauncherKey(key) (MainComponent: an enabled Keyboard
  binding with that keyCode and the same Shift / Cmd / Alt -- the BindingManager loop of MainComponent.cpp:3930-3953).
  A bound key is never consumed by the editor.
- Every key the editor does not consume goes to forwardKey(key) (MainComponent wires [this](k) { return keyPressed(k);
  }) and the canvas returns its result: no Viewport scroll and no Tab focus traversal in between (X3, V4).
  keyStateChanged is not overridden (it bubbles; Component's default returns false).
- Consumed set (unchanged from B1, only while a point is selected and the key is unbound): Left / Right, Up / Down,
  Delete / Backspace, Tab / Shift-Tab, Esc = deselect.
- Plain Esc with nothing selected: MainComponent's SwallowEscape branch (MainComponent.cpp:3854) closes the envelope
  big view when it is open and still returns true; outputs untouched.
- Collision grep result (UX-A9): MainComponent::keyPressed handles only Cmd chords and the output keys before the
  user bindings (V4); the editor consumes no Cmd chord; plain Esc collides with nothing (already swallowed); arrows /
  Tab / Delete / Space collide only with Boris's own bindings, which now always win.

AM7 Real gestures (UX-A1, GATES-A4). test_envelope_ui gains canvas cases built with real juce::MouseEvent / KeyPress
(the V5 helper). Geometry: canvas 400 x 200, 1-bar envelope, SNAP 1/4 beat, Mod 2 default; pixel <-> value through the
canvas's own helpers; the UndoManager entry count read before and after. Each row is [red] at the S2 stub commit:
- G-1 drag the middle point +50 px x, +40 px y, release -> the point at the snapped x and dragged y; undo count +1.
- G-2 click a point without moving -> selected; undo +0; definition unchanged.
- G-3 double-click (numberOfClicks 2) on empty space at x 0.30 -> a new point at x 0.3125 (snapped), selected; +1.
- G-4 double-click that point -> deleted; +1.
- G-5 double-click an endpoint -> not deleted; notice "The first and last points stay".
- G-6 drag a segment's diamond up 30 px -> bend == bendFromMidFraction(expected u) within 1e-3; +1.
- G-7 mouseMove from segment 0 to segment 1 -> the canvas's recorded repaint rects are exactly the two segments'
  diamond rects; no full-canvas repaint.
- G-8 a point selected: Left / Right / Up / Down / Delete / Tab / Esc -> true with the B1 effects; 'a', Space, Cmd+Z ->
  forwarded (the forwardKey stub called once each; the canvas returns the stub's value).
- G-9 Left bound as a launcher key (isLauncherKey stub true), a point selected -> forwarded; the point does not move.
- G-10 focusLost with a point selected -> selection cleared; returnFocus called once; undo +0.
- G-11 the canvas inside EnvelopePanel inside a juce::Viewport with its vertical scrollbar showing, a point selected,
  Up bound -> Up reaches forwardKey, not the Viewport.
- G-12 right-button mouseDown on a point / a segment / empty space -> the menu-builder hook receives Point / Segment /
  Empty.

AM8 Targets and discoverability (UX-A7). Points are 9-px squares (selected: 11 px with a 1-px white hairline); hit
radius 12 px (nearest wins; the selected point wins a tie). Every segment at least 24 px wide with |dy| >= 1e-3 shows
a dim 5-px diamond (kTextSecondary at 50 %); the hovered segment and the selected point's two segments show the full
7-px cyan diamond; dense curves (median spacing < 6 px) show no dim diamonds. Hint: in the big view two short lines
("Drag points. Double-click adds or removes." / "Drag a diamond to bend. Right-click for shapes."); in the tab the same
text is the canvas tooltip and there is no hint line.

AM9 Words (UX-A10, UX-A5). LEVEL -> HEIGHT. SHIFT -> OFFSET (shown in beats; stored as the cycle fraction
phaseOffset with EnvelopeSignal.h:63 semantics, so it stretches with LENGTH like the points). DETAIL stays, with the
live point count beside it and the tooltip "Fewer points (smoother) to more points (closer to the sample)". PLAY =
[LOOP|ONCE] + START (AM1). Every control has a tooltip. The Signal-tab section header "ENVELOPE SETTINGS"
(SignalInspector.cpp:187) becomes "ENVELOPE CREATOR" and the big view's title row reads the same (Boris's word,
backlog:17-19).

AM10 Colours (UX-A11). The canvas uses kAccentCyan / kTextSecondary / kTextPrimary / kPanelBorder (LookAndFeel.h:
25-32) and four NEW named constants appended to AudioDNALookAndFeel: kCanvasBackground 0xff111111, kGridBar 0xff3a3a3a,
kGridBeat 0xff2a2a2a, kGridSnap 0xff202020. No hex colour literal in EnvelopeCanvas.cpp / EnvelopePanel.cpp (G6.1 P8).

AM11 The tab really bigger (UX-A2; replaces B3 "Layout", B4's ">= 100 px at 430 x 230" and E1's "2x" case).
- Tab layout (< 600 px wide): header row (picker, NEW, "...", BIG VIEW, USED BY line); the canvas; a 2-column
  label-left control grid: LENGTH | SNAP, PLAY [LOOP|ONCE][START] | HEIGHT, OFFSET | DETAIL (DETAIL only with a
  sample); one BAND row only with a sample. LOAD SAMPLE..., REMOVE SAMPLE, MAKE LOWS / MIDS / HIGHS and RESET TO
  SAMPLE live in the header "..." menu in the tab (buttons in the big view). Notice line only while a notice is live.
- Canvas height = max(160, panel height - controls); below that the viewport scrolls; the canvas never drops under
  160 px (today 100, SignalInspector.h:66).
- Pre-registered layout bars (test_envelope_ui, EnvelopePanel laid out at literal host sizes, V6):
  - tab host 413 x 439: canvas >= 400 x 300 without a sample, >= 400 x 270 with one;
  - tab host 413 x 200: canvas height == 160 and the panel's preferred height > 200;
  - big host 1720 x 356 (the deck area, AM12): canvas >= 1400 x 300;
  - big canvas width >= 3.0 x tab canvas width (at 413 x 439 vs 1720 x 356).

AM12 Big view over the clip grid (UX-A2, UX-A3, X4, X5; replaces E1's host bounds).
- Host bounds = deckView_'s bounds, set right after deckView_->setBounds (MainComponent.cpp:2693), full window width.
  While open: deckView_->setVisible(false) (precedent: the expanded SignalBar, MainComponent.cpp:2587-2594 -- the hidden
  grid's playhead repaints then cost nothing, X5); host toFront(false) below the binding overlays (:2765-2777);
  resized() keeps the grid hidden while open (one condition at :2694). Close: host hidden, grid shown. The expanded
  SignalBar hides the host with every other panel.
- Why: the largest in-place area, and it does not shrink as layers are added (X4). Preview, Inspector, Files browser
  and SignalBar stay visible: the Inspector concern (UX-A3) is resolved, a sample still drags from Files, and the
  SignalBar strip shows the envelope's live value. Keys and MIDI still fire clips; the mouse cannot until CLOSE (Q1).
- Inside the host: side layout -- canvas left, a 240-px control column right, the full sample section as buttons, the
  2-line hint, title row "ENVELOPE CREATOR".
- Closes on: CLOSE, plain Esc with nothing selected (AM6), selecting a non-envelope signal, a composition swap, the
  SignalBar expanding.
- E1's headless RED keeps: parent == host after expand, == SignalInspector after collapse, the selection survives.
  The grid-hidden / panels-visible check is live (e13).

AM13 E2 playhead required (UX-A8). E2 moves from OPTIONAL into S5 (Q2's default is yes; no answer = default). Its
NativeLayerHost is attached to a playhead component in the host (not in a Viewport, NativeLayerHost.h:18), registered
like the existing two layers (MainComponent.cpp:2318-2319, *overlayWatch_), with a new uipaint::Layer id.

AM14 Sample DSP (SIG-A5, SIG-A6, SIG-A7, X2; replaces C1's DSP bullets).
- Decode: mono = the mean of all channels; NaN / Inf -> 0, counted; more than 1 % -> notice "This file has damaged
  audio; the bad parts count as silence".
- Mirror pre-roll: every filter (the 20 Hz high-pass and every LR4 stage) first runs over the first
  min(100 ms, length - 1 sample) of the mono signal reversed; that output is discarded; the analysed signal is
  unchanged (S3).
- Bands run at the file's own rate. If 4000 Hz >= 0.45 x sampleRate (rate <= 8888 Hz) the Highs band is unavailable:
  no 4 kHz filter is built, Mids = high-pass 250 Hz only, notice "This sample's rate is too low for highs" (V12).
- Time base: a fixed 1 ms hop for every length (replaces max(1 ms, length / 4096)). Each band's sum of squares runs at
  the audio rate; the RMS window (40 / 20 / 10 / 20 ms) is CENTRED on each hop point (offline, non-causal is free); the
  peak-hold release follower (tau 120 / 80 / 60 / 80 ms) runs on the 1 ms grid. Ghost = 1024 points max-pooled from
  that fine envelope (linear interpolation when it has fewer than 1024 points). Memory <= 600 k floats per band at
  10 minutes (S4).
- The remaining filter group delay (<= ~3.6 ms) is not compensated (under one frame).
- Silence: the strongest band's peak == 0, or every band under 1e-3 -> status Silent, notice "This sample is silent",
  no command issued (the division the scratch model hit, S3).

AM15 Quiet bands (SIG-A4). The relative rule stays (peak under the strongest x 10^(-30/20), or under 1e-3 = QUIET),
but a quiet band is NOT disabled: its button is dimmed with the tooltip "Mids are 31 dB quieter than the loudest band
-- the shape may be noise" (the measured dB) and stays selectable. MAKE LOWS / MIDS / HIGHS skips quiet bands and names
them in the notice. Normalisation stays per-band peak. Docs line: "Live bands read against a slowly decaying running
max (SpectralFeatures.cpp:166-193) = loudness relative to the recent past; a sample envelope's 1.0 = that band's
loudest moment in the sample."

AM16 C1 fixtures and bars (SIG-A3, GATES-A5, X1; replaces C1 tests 1 and 2, adds 9-11).
- Fixture rule: no abrupt edge anywhere in a synthetic tone fixture; every segment has a 20 ms Hann fade-in and
  fade-out.
- C1-1 two-tone: 1.0 s 60 Hz then 1.0 s 12 kHz, -6 dBFS, 48 kHz mono. Bars (x = ghost index / 1023):
  lows mean over x <= 0.45 >= 0.6 (measured 0.97); lows MAX over x >= 0.75 <= 0.05 (measured 0.012); highs max over
  x <= 0.45 <= 0.05 (measured 0.000); highs mean over x >= 0.55 >= 0.6 (measured 0.997); mids QUIET at <= -40 dB
  relative (measured <= -68 dB). The plan's "lows mean <= 0.05 in the second half" is deleted (X1).
- C1-1b real mids 31 dB down: 60 Hz at -6 dBFS + 1 kHz at -37 dBFS, 2 s, fades -> mids QUIET (measured -31.3 dB)
  AND selectable: its ghost mean over x in [0.1, 0.9] >= 0.9.
- C1-1c abrupt start: 2 s of cos 60 Hz starting at full amplitude, no fade -> mids and highs QUIET at <= -40 dB
  relative (measured -98.9 / -231 with the pre-roll; -16.4 / -24.9 without -> [red] until AM14).
- C1-2 clicks: 2.0 s, bursts of the gen-click-wav.py shape (5 ms, 0.5 ms attack, 1 kHz 85 % + seeded noise 15 %,
  decay k 5, amplitude 30000/32768) at 0.25 / 0.75 / 1.25 / 1.75 s, no noise floor -> Whole at DETAIL 0.5: exactly 4
  interior local maxima >= 0.5, at x within +-0.01 of 0.125 / 0.375 / 0.625 / 0.875; values at x 0.25 / 0.5 / 0.75
  <= 0.2.
- C1-6 points(0.9) >= points(0.5) >= points(0.1), all <= 256 (measured 44 / 34 / 24 on C1-2).
- C1-9 decimation: 60 s, 12 bursts (3 ms, 8 kHz, exponential decay 1 ms) at 2.5 + 5k s, jittered +-1.5 s (seeded) ->
  the highs ghost is >= 0.5 within +-120 ms of every burst (12/12).
- C1-10 rates: an 8 kHz source -> highs unavailable and every ghost finite; 11.025 kHz -> four valid bands.
- C1-11 silence: all-zero -> Silent; DC 0.2 -> Silent; a source with NaN and Inf samples -> every ghost finite.
- Unchanged: C1-3 (a sustained 60 Hz -> lows <= 8 points), C1-4, C1-5, C1-7, C1-8.
- .harmony/gen-envelope-wav.py (stdlib) writes the live fixtures to exactly these specs (two-tone, C1-1b, clicks).

AM17 Hand edits are protected (UX-A6; replaces D1's "Points rebuilt from the sample -- Undo brings yours back").
EnvelopeSample gains bool edited (saved "edited": true; absent = false). Any committed point / bend / add / delete on an
envelope that has a sample sets it; LENGTH / HEIGHT / OFFSET / mode never do. While edited: BAND and DETAIL are
disabled with the tooltip "Your edits are kept. RESET TO SAMPLE rebuilds the points from the sample."; RESET TO SAMPLE
(enabled only then) rebuilds the points from the ghost with the current band / detail as ONE EnvelopeEditCmd and
clears the flag. DETAIL's live re-simplify during a drag runs only while not edited. Component test: load -> edit a
point -> BAND disabled; RESET -> points == simplify(ghost, band, detail) and edited false; one undo -> the edited
points and the flag return.

AM18 Sample length vs envelope length (SIG-A9). The file label reads "kick_loop.wav  1.9 s = 3.8 beats at 120 BPM"
(recomputed on the 10 Hz refresh; repainted only when the text changes). Docs: the shape is analysed in the sample's
own time, then stretched to LENGTH, so its attack and decay stretch with it.

AM19 AAC bar (GATES-A8, plan R10). e6's m4a sub-row: lows ghost Pearson r >= 0.99 at the best lag AND |best lag| <=
5 ms (derivation V12 + S6: 96000 / 96000 frames, r = 1.0000 at lag 0). A lag near 44-48 ms means the reader did not
trim the AAC priming: the row FAILS and trimming via kAudioFilePropertyPacketTableInfo in the file adapter is in scope.
GET /api/envelopes?ghost=1 returns the four ghosts (base64 u16) plus a ghostHash per envelope.

AM20 UAF guard (GATES-A10). apply_sanitizers on test_envelope, test_envelope_ui and test_sample_envelope (V10). The
guard asserts, WITHOUT dereferencing, that every SignalStrip's and the SignalInspector's held Signal* is a member of
the registry's live pointer set after the remove + one SignalBar::timerCallback, then compares ids (registry ids are
never reused, nextId_++). Added case: hold the inspector across a remove AND a composition swap (service.syncAll with a
composition lacking the envelope) -> it points at a live signal or nullptr. G1.4 runs it in build-asan.

AM21 Idle paint, deterministic (GATES-A8, GATES-A9). Append SrcEnvelope = 1u << 9 to uipaint::Src (V14), stamped by
EnvelopeCanvas / EnvelopePanel repaint() (the existing TEST-ONLY counter pattern). G4's FAIL-able rows are the canvas
repaint counter and the SrcEnvelope pass count (bar 0 each); timing rows are INFO by label (no conditional escape). The
A/B baseline is the main merge commit immediately before the bf45 stage under test, rebuilt with the same toolchain,
interleaved, >= 5 runs per arm.

AM22 Probe teeth (GATES-A7). probe-envelope.py gets --selftest: for every row, a canned CORRECT response must PASS and
every canned WRONG response listed in the gate list must FAIL (offline, no app). The e2 expected values are computed in
the .py from the F-4 formula (never ported from C++) and pre-registered in the gate list. "404 on main" is recorded as
INFO, never as a row's teeth. Every clock injection sends the full tuple {beatPhase, beatInBar, totalBeatCount,
totalBarCount, barCount, resyncBarOrigin, bpm 120}; values are read after the reading is stable for two polls.

AM23 TEST-ONLY lint (GATES-A13). Gate row e14: a python source lint (no build): every new route string (/api/envelopes,
/api/envelope/edit, /manage, /sample, /start, /api/debug/undo, /api/debug/envelope_ui, /api/debug/save_composition)
and every MainComponent wiring of their callbacks sits between `#if AUDIODNA_TEST_SERVER` and its `#endif`.

AM24 TSan scope and e8 count (GATES-A8). G2.2 counts every TSan report with a frame in a file this lane adds or edits
(incl. TestServer::handleListSignals, the new ApiServer handlers and their Box helpers, the MainComponent wiring); bar
0; reports elsewhere are listed INFO. e8's final count = the /api/signals count read before the run + the envelopes
the run leaves (no literal 32).

AM25 Visual gate made concrete (GATES-A11, UX-A1). G6.1 = deterministic in-process checks in test_envelope_ui on
component snapshots (V5 pattern), rows P1-P8 in the gate list; G6.2 = live captures v1-v12, each with a pre-registered
must-be-visible list; G6.3 = critic seats with ONE fixed checklist. Harmony rules each FAIL; every checklist FAIL is
fixed before the Boris page (no round cap); a FAIL that recurs after its fix goes to the Architect; off-checklist
findings are logged and Harmony triages them. The captures stay staged by B5 (honestly labelled); the gestures
themselves are proven by AM7.

AM26 Stub-RED discipline (GATES-A1). Every new test target / section lands first in a stub-RED commit: the test file +
the new headers with compiling stub bodies (default / no-op); each case is tagged [red] (must FAIL there) or [guard]
(must PASS on both sides: the AM3 golden, O9, unchanged anchors). The builder report lists each stage's stub SHA.
Harmony builds the named test targets at that SHA itself (G1.3); a builder-reported log path is not evidence. To avoid a
re-download, point FetchContent at the existing build/_deps/juce-src.

AM27 Stage order and Boris checkpoints (UX-A12). S1 -> S2 -> S5 (E1 + E2) = BF4 delivered = the first Boris
checkpoint. S3 runs in parallel from S1. S4 after S2 + S3 = the BF5 checkpoint. S6 only if Q6 says yes. S1 is never
shown to Boris alone (its old toggles have no START).

AM28 New TEST-ONLY routes and probe rows. Routes (all inside #if AUDIODNA_TEST_SERVER): POST /api/envelope/start
{uid|name}; POST /api/debug/save_composition; GET /api/envelopes?ghost=1; /api/debug/envelope_ui also returns
{tabCanvasRect, bigCanvasRect, deckVisible, canvasRepaints}. Rows e10-e14 are added to G3 (gate list).

AM29 Boris answer and questions (UX-A4, UX-A5, SIG-A10). The own-window answer is rewritten; Q1 is reworded for AM12;
Q7 ("live") is added; Q2 now confirms a built default. "Setup format" is NOT re-asked (GATES-A12 ruling); the
BORIS_DECISIONS entry records "lengths in beats and bars, snap, loop / once" as a default he accepted.

AM30 Docs (on top of plan section 6). rendering.md "Envelopes" adds: the Once state machine (AM1), keys / focus
(AM6), the big view over the grid (AM12), the DSP time base, pre-roll and quiet bands with the live-vs-sample scaling
line (AM14, AM15), the legacy curve mapping (AM2), the stretch time base (AM18). performance-controls.md (bindings):
"a focus-taking editor never consumes a bound launcher key and forwards every unconsumed key to
MainComponent::keyPressed -- a juce::Viewport eats Up / Down while its scrollbar shows". The plan's pitfall ("proposed
64") takes the next free number Harmony assigns (64 went to mkvidx, X6).

### FINAL BUILD STAGES

| Stage | Items | Builds after | Boris checkpoint |
|---|---|---|---|
| S1 | A1-A8 + AM1, AM2, AM3, AM4, AM5 (describeWhere, usersOf), AM20 (sanitizers), AM23, AM24, AM26, AM28 (routes) | bf7 and bf2 merged (plan R5 rebase rules); if bf9b merged first, A5 mirrors its ConnectionEngine::tick traversal | none |
| S2 | B1-B5 + AM6, AM7, AM8, AM9, AM10, AM11, AM21 (SrcEnvelope), AM25 (P1-P8) | S1 | none |
| S5 | E1 per AM12 + E2 per AM13 (required) | S2 | BF4: G1-G6 BF4 rows, then the Boris page |
| S3 | C1-C2 + AM14, AM15 (flags), AM16 | S1's types (own worktree, parallel) | none |
| S4 | D1-D3 + AM15 (UI), AM17, AM18, AM19 | S2 + S3 | BF5: G3 e6 / e7 / e10 / e12, G6 v3 / v4 / v12, Boris page |
| S6 | F (optional) | S4 | only if Q6 = yes |

Merge order: S1 -> S2 -> S5 -> S3 -> S4 (-> S6). Each stage begins with its stub-RED commit (AM26).

### FINAL GATES (Harmony copies gate strings only from this list)

G1 UNIT (after each stage merge; final run after S4)
- G1.1 Full ctest on the merged build: 0 failures.
- G1.2 test_envelope, test_envelope_ui, test_sample_envelope, test_connection, test_oscillator_bar_fold: 0 failures,
  including O1-O9 (AM1), G-1..G-12 (AM7), P1-P8 (G6.1), C1-1 / 1b / 1c / 2 / 3 / 4 / 5 / 6 / 7 / 8 / 9 / 10 / 11
  (AM16), the AM11 layout bars, the AM17 component case, the AM3 golden, the AM4 missing-name note, the AM5
  describeWhere list, the AM20 UAF cases, the AM2 bound, and the plan's A1 / A2 / A5 / A6 / B1 / B4 / C2 / D3 / E1
  cases as amended.
- G1.3 RED history, Harmony-run: at each stage's stub-RED SHA Harmony builds the named test targets in a scratch
  worktree and runs them: 100 % of [red] cases FAIL and 100 % of [guard] cases PASS. O1 also FAILS against main's
  own API.
- G1.4 ASan: test_envelope, test_envelope_ui, test_sample_envelope built and run in build-asan: 0 reports.

G2 TSAN
- G2.1 probe-tsan-unit.sh on test_sample_envelope [loader]: 0 reports.
- G2.2 App TSan build running probe-envelope e8: 0 reports with a frame in a file this lane adds or edits (incl.
  TestServer::handleListSignals, the new ApiServer handlers and Box helpers, the MainComponent wiring); other reports
  listed INFO.
- G2.3 S1 order: the TSan run on the commit BEFORE the handleListSignals hop is recorded (a handleListSignals report =
  RED shown; none = INFO "race not reproduced"); after the hop, G2.2 applies.

G3 LIVE PROBE .harmony/probe-envelope.sh + .py + .json (a probe-capture.sh clone: live lock, open -g --args
--test-mode, 7070 + 8080, Connection: close, graceful quit, never an Output window, no synthetic input; fixture
compositions generated into $TMPDIR and pre-registered in the .json). Every row must PASS; a FAIL triggers a fix round.
- G3.0 selftest: `probe-envelope.py --selftest` -> every canned correct response PASSES and every canned WRONG
  response named below FAILS (offline).
- e1 Fresh launch: GET /api/envelopes == [Mod 2: (0,0)(0.5,1)(1,0), 4 beats, loop, height 1, offset 0].
  WRONG: any other default.
- e2 Edit Mod 2 to [(0,0), (0.25,0.2, bend 0.5), (0.75,0.9), (1,1)], 4 beats, loop; inject beats 0.5 / 1.5 / 2.0 /
  2.5 / 3.5 -> /api/signals "Mod 2" == 0.1000 / 0.2240 / 0.2875 / 0.4555 / 0.9500 (+-0.01).
  WRONG: bend sign flipped = 0.1000 / 0.6445 / 0.8125 / 0.8760 / 0.9500.
- e3 /api/debug/undo -> e1's points; redo -> e2's points. WRONG: undo leaves e2's points.
- e4 manage new -> "Envelope 1" in /api/signals; duplicate -> "Envelope 1 copy"; load a fixture whose comp scalar
  connects to "Envelope 1"; rename it "Kick" -> users("Kick") == 1; delete "Kick" -> {ok: false, users: 1, usedBy:
  [the label pre-registered in the .json]}; delete an unused envelope -> gone from /api/signals.
  WRONG: rename leaving users 0; a delete in use succeeding.
- e5 Load a fixture with bends + a sample whose file does not exist -> definitions intact, loadNote empty, notice
  "Sample file not found -- the shape is kept", ghost present.
- e6 Sample, fixtures from gen-envelope-wav.py (AM16 specs): the click file -> status ok within 5 s, durationSec within
  1 %, Whole has exactly 4 maxima >= 0.5 at x within +-0.01 of 0.125 / 0.375 / 0.625 / 0.875, 2 <= points <= 256; the
  two-tone -> the C1-1 bars; `afconvert -f m4af -d aac` of the two-tone -> lows ghost r >= 0.99 at the best lag and
  |best lag| <= 5 ms. WRONG: 3 or 5 maxima; lows / highs swapped; a 46 ms lag.
- e7 pull_bands on the two-tone -> "<stem> Lows" + "<stem> Highs" created; Mids skipped and named in the notice.
  WRONG: a Mids envelope created.
- e8 Concurrency: 200 x (manage new, manage delete) interleaved with 200 x GET /api/signals (8080) -> every response
  well-formed JSON; the app alive; final /api/signals count == the count read before the run + the envelopes left.
- e9 After quit no Audio-DNA process remains; no foreign REST traffic (the probe-capture.sh FOREIGN check).
- e10 Save + relaunch: load a fixture copy from $TMPDIR via /api/load_composition; edit Mod 2 (e2's points, ONCE,
  lengthBeats 8, HEIGHT 0.8); manage new "E10" and sample the click file into it (status ok); GET /api/envelopes?ghost=1
  -> A; POST /api/debug/save_composition -> ok and the saved file has "envelopes" with 2 entries and E10's ghost;
  graceful quit; relaunch; load the same path; GET /api/envelopes?ghost=1 -> B. Bar: B == A on uid, name,
  lengthBeats, amplitude, phaseOffset, mode, points (x, y, interp, bend to 1e-6), sample {fileName, durationSec, band,
  detail, silent, edited} and ghostHash (live value excluded). WRONG: B == the default Mod 2 (key dropped).
- e11 Once ("E11": (0,0)->(1,1), 4 beats, once; mono = totalBeatCount + beatPhase): mono 0.0 / 2.0 / 5.0 -> <= 0.01;
  at mono 5.5 (beatInBar 1, beatPhase 0.5) POST /api/envelope/start; mono 6.0 / 7.9 / 8.0 -> <= 0.01; 9.0 -> 0.25;
  10.0 -> 0.50; 11.0 -> 0.75; 12.0 / 13.0 / 20.0 -> >= 0.99. START at mono 20.5; 25.0 -> 0.25; 25.3 -> 0.325; a manual
  Resync snapshot (totalBeatCount 25, beatPhase 0, beatInBar 0, resyncBarOrigin = totalBarCount) -> <= 0.01;
  totalBeatCount 26, beatPhase 0, beatInBar 1 -> 0.25; run to Done (>= 0.99) and Resync again -> stays >= 0.99; a
  never-started once envelope "E11b" through both Resyncs -> <= 0.01. All +-0.01.
  WRONG: starts at once on START (0.125 at 6.0); loops (0.25 at 13.0); Resync ignored while running (0.325 after it);
  a Resync starting an idle envelope.
- e12 Stretch: the click file into "E12" (Whole, DETAIL 0.5, loop). lengthBeats 8: beats 1 / 3 / 5 / 7 -> >= 0.5;
  beats 2 / 4 / 6 -> <= 0.2. lengthBeats 4: beats 0.5 / 1.5 / 2.5 / 3.5 -> >= 0.5; beats 1 / 2 / 3 -> <= 0.2.
  WRONG: length ignored (peaks at 0.5 / 1.5 / 2.5 / 3.5 while lengthBeats is 8).
- e13 Bigger, live: default composition at the launch window; /api/debug/envelope_ui with the Signal tab and then the
  big view: big canvas area >= 2.5 x tab canvas area; tab canvas height >= 160; while the big view is open
  deckVisible == false and the preview, Inspector and Browser rects are non-empty (/api/debug/ui_paint).
  WRONG: a big view as tall as the bottom row.
- e14 TEST-ONLY lint (AM23): 0 violations.

G4 IDLE PAINT (probe-idle-paint.sh i1 / i2 / g4)
- G4.1 Signal tab showing Mod 2 (loop, running), 10 s idle: canvas repaint counter delta == 0 AND ui_passes carrying
  SrcEnvelope == 0.
- G4.2 Big view open with the E2 playhead running, 10 s idle: ui_passes carrying SrcEnvelope == 0 AND the new layer's
  layerDraws > 0.
- G4.3 INFO: idle main-thread ms/s, interleaved A/B against the main merge commit immediately before the stage, same
  toolchain, >= 5 runs per arm; reported, never a FAIL.

G5 PERF, INFO: tickFeaturePipeline cost with 12 envelopes x 256 points vs the default, interleaved A/B, >= 5 runs per
arm; expected < 0.05 ms per tick.

G6 VISUAL WORK GATE (before Boris sees anything)
- G6.1 Deterministic, inside G1.2 (test_envelope_ui, component snapshots):
  P1 every unselected point is a square: the 4 corner pixels of its 9 x 9 box are the point colour;
  P2 the selected point is 11 px with a white 1-px outline;
  P3 >= 90 % of canvas columns hold a kAccentCyan pixel (+-24/255) within 2 px of the evaluated curve;
  P4 every TextButton's 4 corner pixels equal its own fill / border (no rounding);
  P5 touching: LOOP.right == ONCE.left; the BAND buttons are adjacent; the header NEW / ... / BIG VIEW are adjacent;
  P6 every button and header text equals its upper case;
  P7 every numeric readout uses the monospace typeface;
  P8 no hex colour literal in EnvelopeCanvas.cpp / EnvelopePanel.cpp (grep).
- G6.2 Live captures by Quartz window id (screencapture -l; decoded; non-black), each with its must-be-visible list:
  v1 Signal tab, Mod 2 default: header ENVELOPE CREATOR; picker "Mod 2"; canvas >= 160 px tall; 3 square points; ruler
     bar number 1; LENGTH 1 BAR; SNAP 1/4 BEAT; LOOP lit; HEIGHT 100 %; OFFSET 0.
  v2 hand-made envelope (staged by B5): >= 5 points; one selected (11 px, white outline); dim diamonds on the wide
     segments; one full cyan diamond on the hovered segment; the drag readout "bar.beat value%".
  v3 sample envelope (Lows) with its ghost behind the curve; BAND row with LOWS lit; DETAIL with its point count; the
     file label "... s = N beats at B BPM".
  v4 notices: "Reading ...", "Sample file not found -- the shape is kept", a quiet band's dimmed button with its dB
     tooltip.
  v5 big view over the clip grid: grid hidden; preview, Inspector, Browser, SignalBar visible; canvas full width;
     control column right; 2-line hint.
  v6 big view with the playhead line.
  v7-v9 menus (segment shape, "...", picker) via the TEMPORARY env-hook snapshot, reverted before merge.
  v10 delete refused, naming its user.
  v11 SignalBar with the new envelope strips.
  v12 an edited sample envelope: BAND / DETAIL disabled, RESET TO SAMPLE enabled.
- G6.3 Critic seats visual-design, UX, graphic-design, logic, interaction-logic; ONE fixed checklist = the G6.2 lists +
  BORIS_DECISIONS.md:58-69 (no radius, no shadow, no gradient, 1-px hairlines, buttons touch, ALL-CAPS headers and mode
  labels, monospace only for numbers) + one accent (:13-19) + the AM6 key rules + the AM7 gestures + undo granularity
  + expand / collapse + the picker vs SignalBar selection. Harmony rules each FAIL; every checklist FAIL is fixed before
  the Boris page (no round cap); a FAIL recurring after its fix goes to the Architect; off-checklist findings are
  logged and triaged by Harmony. Then .harmony/.reports/s-rta-1002b/boris-checks.html: the captures, the defaults
  taken, the checks below, the questions.

### FINAL BORIS QUESTIONS (plain words; each has a default, so the build never waits)

ANSWER TO RELAY for "what would its own window look like?": "It would be the same envelope editor in its own window.
On one screen it floats over the app and covers whatever is under it, so you would keep moving it around; on a second
screen it could sit beside the app. That is why we are building the bigger view inside the app now and the window
later, once the layout is final. The editor is one piece, so putting it in a window later is a small job."

- Q1 "The bigger envelope view opens over the clip grid. The preview, inspector and file browser stay visible, and
  your keys and MIDI still fire clips; CLOSE or Esc brings the grid back. It is much wider than the small one and a
  bit taller. Is that the 'bigger' you want?" Default: yes.
- Q2 "The bigger view shows a moving line where the envelope is right now. The small view cannot -- it would make the
  whole app redraw all the time. OK?" Default: yes (built).
- Q3 "'Play once': when you press START, should it start on the next bar or right away?" Default: on the next bar.
- Q4 "Do you want to fire START from a pad or a key while performing?" Default: not now (a later lane).
- Q5 "Long samples: squeeze the whole sample into the envelope length (up to 10 minutes), or pick a section?"
  Default: the whole sample.
- Q6 "If you save a deck and load it into a different set, should its envelopes come along?" Default: not in this
  lane; envelopes are saved with the set.
- Q7 "When you load a sample, the app reads the whole file once, silently, and turns its loudness into an envelope
  stretched to the length you set; after that the envelope plays with the beat. It does not listen to the music
  playing at that moment. Is that what you meant by 'extract the envelope live'?" Default: yes, as built.

Defaults taken without a question (go on the Boris page): Once never starts by itself (START does; switching to ONCE
does not); a Resync restarts a running once-envelope at the new downbeat and never starts an idle one; a quiet sample
band stays usable with a warning; hand edits survive BAND / DETAIL until RESET TO SAMPLE; keys bound to the launcher
always reach it, even while a point is selected; setup format = lengths in beats and bars, snap, loop / once.

What only Boris can check (replaces plan section 8): 8.1 the bigger view over the grid (Q1); 8.2 the feel of dragging,
double-click add / remove, the diamonds, the 1/4-beat snap and the readout; 8.3 do Lows / Mids / Highs move the visuals
the way the sample sounds; 8.4 with a point selected, do his bound keys still fire as expected; 8.5 LOOP / ONCE / START
and a Resync during a once; 8.6 the names of pulled-out envelopes and their SignalBar strips; 8.7 the RESET TO SAMPLE
flow after a hand edit.

### RESIDUAL RISKS

- RR1 bf9b (decks are boxes of clips; plan still a skeleton) may move layers out of decks: ConnVisitor and
  describeWhere must mirror the post-bf9b ConnectionEngine::tick; the A5 tick cross-check catches a missed placement.
  AM12 anchors to whatever deckView_'s bounds are.
- RR2 bf2 may edit EnvelopeSignal's phase block (plan R5 rule stands); AM1's Once reads published fields that bf2
  already shifts (V3), so no extra wiring is expected -- re-check at rebase.
- RR3 AM12 hides the clip grid while the big view is open: no mouse-firing of clips until CLOSE. Q1 lets Boris veto.
  Strongest counterargument to this ruling: the plan's side-by-side host keeps the grid visible. It loses because that
  host is only as tall as the bottom row (219 px at 6 layers, X4), so "bigger" would fail on exactly the sets that need
  it, while the small tab editor still edits with nothing hidden.
- RR4 AM6 calls MainComponent::keyPressed directly for forwarded keys; the binding / MIDI-learn overlays are still
  honoured because keyPressed checks them first (MainComponent.cpp:3807-3811).
- RR5 e6's AAC row may FAIL if JUCE's reader keeps the AAC priming (AM19 makes the trim in scope).
- RR6 (SIG-A11) a 10-minute decode competes for a core with analysis on a small machine; same arrangement as the
  shipping decode pools (V13); no metric exists to gate it.
- RR7 X5 is INFERRED from JUCE's paint traversal, not measured; e13's deckVisible == false check makes it moot.

STATUS: COMPLETE -- 36 attacks ruled (34 accepted, 2 rejected: SIG-A11, GATES-A12); 30 amendments AM1-AM30 override plan-bf45.md; stages S1 -> S2 -> S5 -> S3 -> S4 (-> S6); gates G1-G6 pre-registered; 7 Boris questions with defaults; ready_to_build: yes
