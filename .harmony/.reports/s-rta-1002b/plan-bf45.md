# PLAN lane bf45 (s-rta-1002b) -- BF4 hand-editable, bigger envelope editor + BF5 envelopes extracted from an audio sample

Architect (Fable), 2026-10-02, main HEAD 5e47d17. Read-only; every code claim cites file:line read at 5e47d17.
Scratch math check ran under /private/tmp (python, no app launched).

## (1) GOAL

Make every envelope hand-editable in a bigger, self-contained editor (drag / add / delete points, bend the segment
between points, lengths in beats AND bars, saved with the composition, undoable), and let a SILENT audio sample be
loaded into an envelope so its Whole / Lows / Mids / Highs loudness shape -- stretched to the envelope's length --
becomes editable envelopes.

Boris, verbatim: "Need to manually change envelopes. Need to expand envelope controls and setup format" / "Make it so
when you load a sample into the envelope that it will emulate the envelope of the audios behavior. So basically use a
sample to extract the envelope live, and then different envelopes can be pulled out of that sample with the envelope
creator". Answers: BF4 "default is good, bigger envelope, what would it's own window look like? perhaps we don't create
it's own window till later when we have finalized the ui or would you rather do it now to build it then change the ui
placement?"; BF5 "all defaults good" (binding-decisions.md:585-590).

Not in this lane: its own window (a later ~50-line host, fork F-3); a per-parameter drawn curve on a connection
(ConnSource::Kind::Envelope = "Timeline" = bf6); hearing the sample; a MIDI/key binding to restart an envelope (Q4).
Answer Harmony can relay to Boris's "what would its own window look like": the same panel in a resizable window you
park anywhere on the one performance screen (BORIS_DECISIONS.md "Layout": interface on ONE screen).

## (2) ESTABLISHED FACTS

A. What an envelope is today
- F1 VERIFIED. Exactly one envelope exists: registry signal "Mod 2" = `EnvelopeSignal("Mod 2", 4.0f)`
  (SignalRegistry.cpp:69-73), points (0,0),(0.5,1),(1,0) (EnvelopeSignal.h:25-28). Nothing creates another: the
  SignalBar [+] menu only un-hides hidden signals (SignalBar.cpp:181-228); `setPoints` / `addPoint` have no callers.
- F2 VERIFIED. A parameter uses an envelope through the UPC "Envelope" submenu, which lists registry envelope signals
  (UniversalParamControl.cpp:489-508); picking one makes a `Kind::Signal` connection keyed by NAME
  (UniversalParamControl.cpp:637-652; ParamConnection.h:37-38 "signalName -- the PERSISTENT key").
- F3 VERIFIED. A second "envelope" type exists: `ConnSource::Kind::Envelope` holding an `AutomationCurve`
  (ParamConnection.h:52-63); its only origin is the Timeline picker entry, a 4-beat ramp (ConnPicker.cpp:90-100;
  describeSource returns "Timeline", :122-126). bf6 owns it; this lane does not touch it.
- F4 VERIFIED. `AutomationCurve` is ruled the shared "one struct / one evaluator / one editor" for drawn and captured
  curves (AutomationCurve.h:6-11). `Breakpoint` has a per-segment Interp Linear/Hold/Smooth (:16-26), `eval` (:92-123),
  `toVar/fromVar` (:33-88). Users: recorded lanes (Lane.h:134; PerformanceRecorder.cpp:106,137 use aggregate init
  `{x, y, interp}`), Program.cpp:510, RoutineSlice.cpp:195-233, ConnPicker.cpp:96-99; ConnSerialization.cpp has its
  OWN point writer for connection curves (:119-135, :204-240), not `Breakpoint::toVar`.
- F5 VERIFIED. `EnvelopeSignal` = `std::vector<ControlPoint>` + ONE global CurveType Linear / Exponential (t*t) /
  SCurve (EnvelopeSignal.h:13-19, :90-101).
- F6 VERIFIED DEFECT. "One Shot" and "Looping" do nothing: cyclePhase is already fmod'ed into [0,1)
  (EnvelopeSignal.h:60-63), so `min(cyclePhase, 1.0f)` (:66-69) never changes it.
- F7 VERIFIED. No envelope field is saved (EnvelopeSignal.h:141-145 "no EnvelopeSignal field is"); Composition::toVar
  has no signals / envelopes key (Composition.h:310-440). Every envelope edit is lost at relaunch today.
- F8 VERIFIED. The editor is paint-only: SignalInspector has no mouse handlers (SignalInspector.h:15-69);
  paintCurveEditor draws straight lines + 8-px circles (SignalInspector.cpp:365-422) in a fixed 100-px box
  (SignalInspector.h:66); lengths 1/2/4/8/16 beats (SignalInspector.cpp:87-101); a global curve combo (:75-85);
  amplitude + phase are ResettableSliders with defaults (:5-16, :103-117) but carry no labels.

B. Threads -- where an envelope is evaluated and how its value leaves the message thread
- F9 VERIFIED. NOT on the render thread. `SignalRegistry::evaluateAll` is message-thread confined (jassert,
  SignalRegistry.cpp:166-178), called once per tick at 120 Hz by MainComponent::tickFeaturePipeline
  (MainComponent.cpp:4030-4039); the GL callback stopped evaluating it in S166-L1 (Renderer.cpp:528-535).
  ConnectionEngine::tick is message-thread confined too (ConnectionEngine.h:27-30, .cpp:294) and reads an envelope as a
  `Kind::Signal` cached float (ConnectionEngine.cpp:106-111).
- F10 VERIFIED. The value reaches other threads only as floats: relaxed `std::atomic<float>` cachedValues_
  (SignalRegistry.cpp:177, :186) -> LiveValue twins stored relaxed (ConnectionEngine.cpp:240-246, :269-273, :377) ->
  GL. No GL code reads the registry (grep of src/render, src/effects, src/sources, src/output: Renderer.h:154-155, :569
  only store a pointer; `getSignalRegistry()` has no caller).
- F11 VERIFIED. Off-message-thread registry reader: only TestServer::handleListSignals on the httplib thread
  (TestServer.cpp:1015-1043, route :177). Raw holders of a Signal: SignalInspector::signal_ (SignalInspector.h:30) and
  SignalStrip::signal_ (SignalStrip.h:49). addSignal / removeSignal reallocate cachedValues_ (SignalRegistry.cpp:88-120).
- INFERRED from F9-F11: an envelope edit needs NO new cross-thread publication, no Relaxed<T>, no lock. The definition
  is message-thread data; Pitfall 63 covers fields another thread reads, and none will. What DOES change: the registry
  becomes mutable at runtime (new / delete / rename envelopes), so F11's three holders need one choke point (item A6).

C. UI hosting constraints
- F12 VERIFIED. The Signal tab sits in a juce::Viewport (InspectorPanel.cpp:41-43), sized from getPreferredHeight
  (:96-101, :191-198), refreshed ~10 Hz (MainComponent.cpp:4154-4156 -> InspectorPanel::refresh :179-188).
- F13 VERIFIED. Pitfall 57 (pitfalls.md:121): a timer-driven repaint anywhere is paid by the whole window; an
  animating widget needs its own native layer or must repaint only on change, and "Never attach a widget that lives
  inside a juce::Viewport" (NativeLayerHost.h:19). => a moving playhead cannot live in the Signal tab.
- F14 VERIFIED. Bottom row = preview | TimingWindow | Inspector | Browser at 22/28/25/25 % (MainComponent.h:498;
  layout MainComponent.cpp:2707-2760); the TimingWindow's three tabs are empty placeholders (TimingWindow.h:5-7;
  APP-INVENTORY.md TimingWindow row).
- F15 VERIFIED. Boris's standing visual rules: no border-radius anywhere, no shadows, no gradients, 1-px hairlines,
  buttons touch, no emoji / decorative icons, ALL-CAPS headers and mode labels, monospace only for numbers
  (BORIS_DECISIONS.md:58-71); one accent (cyan), red for danger only (:13-19); label-left inspector grammar (:180-187);
  "label-above-value" rejected (:389-411). CHECKED, NOT IN CONFLICT: Hits-section "No drag-and-drop" (:117) is about
  creating Hits; "Timeline view" rejected (:393) is the app-wide arrangement view, not a per-signal curve editor.
- F16 VERIFIED. Undo: Command {execute, undo, description, canMergeWith, mergeWith} (Command.h:7-35);
  UndoManager::perform executes, then merges into the previous command (UndoManager.cpp:12-47), 100 entries, message
  thread; Cmd+Z / Shift+Cmd+Z in MainComponent::keyPressed (MainComponent.cpp:3860-3873) and the Composition menu
  (~:6469-6480); "first execute is a no-op" precedent (TriggerCommands.h:24). Composition swaps clear history (:3029).
- F17 VERIFIED. The composition owns its routines (Composition.h:41-49, "D9 inside the thing it belongs to") -- the
  precedent for composition-owned envelopes. initDefault :172-192; toVar :310-440; fromVar :442-674 (hasProperty-guarded;
  parsed on the message thread into a private `incoming`, MainComponent.cpp:3339-3344). Every swap goes through
  swapCompositionModel (MainComponent.cpp:2988-3031: ++modelEpoch_ :2993, undoManager_.clear() :3029,
  refreshUiAfterModelSwap :2924).

D. Sample path
- F18 VERIFIED. Decoders: AudioEngine calls registerBasicFormats (AudioEngine.cpp:7) = WAV, AIFF, FLAC
  (JUCE_USE_FLAC default 1), Ogg Vorbis (JUCE_USE_OGGVORBIS default 1), and on macOS Core Audio (MP3, M4A/AAC, CAF ...:
  kAudioFileGlobalInfo_AllExtensions); JUCE's own MP3 decoder is off (juce_AudioFormatManager.cpp:63-87;
  juce_audio_formats.h:74-101; juce_CoreAudioFormat.cpp:77-80). AudioFormatReader::read(AudioBuffer<float>*) reads
  more than 2 channels and converts to float (juce_AudioFormatReader.cpp:151-215).
- F19 VERIFIED. juce_dsp is linked (CMakeLists.txt:589): juce::dsp::LinkwitzRileyFilter (crossover
  `processSample(ch, x, &low, &high)`, juce_LinkwitzRileyFilter.h:123-128) and BallisticsFilter (attack / release,
  peak / RMS, juce_BallisticsFilter.h:68-90).
- F20 VERIFIED. Live analysis bands: 20/60/250/500/2000/4000/6000/20000 Hz (SpectralFeatures.cpp:10-12); band level =
  linear sqrt(power) over a slowly decaying running max (:166-193). So Lows = 20-250 (Sub + Bass), Mids = 250-4000,
  Highs = 4000-20000 line up with what the app already calls Sub Bass / Bass / Mid / Air.
- F21 VERIFIED. House async pattern = ClipThumbnails (ClipThumbnails.h:17-111): low-priority juce::ThreadPool,
  injectable decoder + poster for tests, WeakReference liveness, removeAllJobs(true, 5000) in the destructor.
- F22 VERIFIED. Internal drags from the Files browser are "files:p1|p2" (FilesBrowser.cpp:138); the browser lists
  .wav/.aiff/.aif/.mp3/.flac/.ogg (FilesBrowser.cpp:544-551).
- F23 VERIFIED. Gate plumbing: a TEST-ONLY route block (ApiServer.cpp:300-331); message-thread read with a 2 s
  Box + WaitableEvent (ApiServer.cpp:2011-2042); write via callAsync (:2323-2328); /api/inject_features in test mode
  (:233-241) sets beatPhase / beatInBar / barCount / totalBarCount / resyncBarOrigin (:905-935); a PopupMenu dies
  when the app is not frontmost, so menu shots use a TEMPORARY env hook + createComponentSnapshot
  (probe-deck-tabs.sh:16-24); a click-track generator exists (.harmony/gen-click-wav.py).
- F24 VERIFIED. test_oscillator_bar_fold links only Catch2 (tests/CMakeLists.txt:237-242) and holds the Mod 2
  regression anchors (test_oscillator_bar_fold.cpp:256-307, :420-451, :729-730).
- F25 VERIFIED (scratch). The bend math in F-4 is exact (inverse round-trip error 3e-16; f(0.5, +1) = 0.02,
  f(0.5, -1) = 0.98; the old "Exponential t*t" equals bend +0.282). RDP on a synthetic 1-bar drum loop (4 kicks +
  16 hats, 1024-point ghost, 60 ms release) gives 40-100 points across detail 0..1, max error = the tolerance.
- F26 VERIFIED STALE DOC. docs/claude/rendering.md:87 still says evaluateAll runs in Renderer::renderOpenGL (F9).

## (3) DESIGN FORKS

F-1 Which envelope becomes editable
- CHOSEN (a): the registry envelope signal -- what the Signal tab edits today (APP-INVENTORY.md:75), named and shared
  by any number of controls. "different envelopes can be pulled out of that sample with the envelope creator" needs
  several named envelopes. Its curve becomes the shared AutomationCurve (F4 ruling).
- (b) a per-parameter AutomationCurve on each connection (Kind::Envelope). Loses: one sample could not drive many
  controls; the editor would live in every parameter's picker; Kind::Envelope is "Timeline" (bf6). Still possible
  later: the editor component edits any AutomationCurve.

F-2 Where the data lives and how it is saved
- CHOSEN (a): `Composition::envelopes` (std::vector<EnvelopeDef>) -- saved, loaded, swapped and undone with the
  composition; the registry's EnvelopeSignal is a CACHE synced by name through ONE function. Evidence: the controls
  that use an envelope are composition data keyed by name (F2); routines precedent (F17); swaps already funnel
  through one function (F17).
- (b) the registry owns the data and MainComponent splices it into the JSON at save. Loses: Composition::toVar is const
  and has three save callers (MainComponent.cpp:3406, :3448, :6560); New / Load / undo each need registry surgery; two
  sources of truth.
- (c) EnvelopeSignal points into composition memory. Loses: dangles across a swap (the UAF class documented at
  MainComponent.cpp:2929-2958).

F-3 Where the bigger editor lives ("bigger in place now, own window later")
- CHOSEN (a): ONE self-contained `EnvelopePanel` (picker + canvas + controls) with two hosts. (1) The Signal tab,
  filling the tab's height (canvas at least 120 px; today fixed 100). (2) An in-place BIG VIEW: the same instance is
  re-parented into a MainComponent host covering the TimingWindow + Inspector columns (~53 % of the bottom width).
  Preview and browser stay visible, so a sample can still be dragged from Files. Its own window later = a third host.
- (b) its own window now: Boris undecided, Harmony recommended later, the UI placement is not final.
- (c) move it into the empty TimingWindow for good: a placement decision the TimingWindow's future (BPM / Routing tabs)
  may contradict.
- (d) the tab only: at default size the canvas is ~430 x 150-250 px; a 4-bar envelope at 1/4-beat snap = 64 columns
  of 6.7 px, too tight for sample envelopes.

F-4 Segment bend model
- CHOSEN (a): one curvature per segment. `Breakpoint::bend` in [-1, 1] on Linear segments:
  f(t) = (e^(k t) - 1) / (e^k - 1), with k = bend * 2 ln 49 (f(t) = t when |k| < 1e-4). The handle sits at the segment
  midpoint. Exact inverse: bend = 2 ln(1/u - 1) / (2 ln 49), with u = (y_mouse - ya) / (yb - ya) clamped to
  [0.02, 0.98]. The curve is monotone, keeps exact endpoints, and bend and -bend are point reflections (F25).
  Hold ("Step") and Smooth ("S-curve") stay as segment shapes. This covers Boris's own curve vocabulary
  (BORIS_DECISIONS.md:111: linear / ease-in / ease-out / S-curve / exponential / step / instant-cut). Instant-cut =
  two points 1e-4 apart.
- (b) Bezier, two handles per segment: harder to use and can overshoot [0, 1].
- (c) power curve t^p: up-bends and down-bends feel different.

F-5 Sample result: editable points or a dense curve
- CHOSEN (a): editable points, from an RDP simplification of a 1024-point "ghost" per band (at most 256 points, a
  Detail slider), with the ghost embedded in the composition and drawn behind the curve. Evaluation keeps the one curve
  evaluator. Changing Band or Detail re-simplifies from the ghost, without the file.
- (b) a dense curve evaluated directly: not hand-editable (BF4) unless a second editing mode is built.

F-6 Sample analysis method
- CHOSEN (a), offline on a low-priority pool: decode -> mono -> 20 Hz high-pass -> band split with cascaded
  Linkwitz-Riley filters (2 x LR4 = 48 dB/oct) at 250 Hz / 4 kHz -> per-band short RMS (window at least one low-band
  period) -> peak-hold release follower -> normalise each band to its own peak (a band more than 30 dB under the
  strongest = "no lows in this sample") -> max-pool / interpolate to 1024 -> RDP.
- (b) FFT frames like the live analysis (2048 / 512): the fixed 43 ms windows smear a short sample stretched over
  bars, and short windows lose the lows.
- (c) play the sample through the live analysis thread ("extract live" literally): touches the sacred pipeline, needs
  real-time playback, is not deterministic, and cannot stretch to a bar length.

F-7 The sample file: copy it or reference it
- CHOSEN (a): reference the absolute path (shown as the file name) and embed the 4 band ghosts (base64 of u16,
  ~11 KB). A composition moved to another machine keeps every sample envelope working, shape AND re-banding. The file
  is only needed to re-read it.
- (b) copy it into a composition folder: no such folder exists unless Collect Media runs, and it duplicates files.
- (c) reference only: a missing file leaves nothing to show or re-band.

F-8 A playhead in the editor
- CHOSEN (a): none in the Signal tab (F13). Optional stage E2 adds one to the BIG VIEW only, through NativeLayerHost.
  That is legal there because the big view is not inside a Viewport.
- (b) a playhead everywhere, painted in the peer: whole-window passes whenever the tab is open, which is exactly the
  regression the idle-paint gate exists to catch.

F-9 Managing envelopes
- CHOSEN: New / Duplicate / Rename / Delete in the panel header, plus "New envelope" at the top of the SignalBar [+]
  menu, so an envelope can always be created. Delete is refused while a control uses the envelope (counted by a
  connection visitor). Rename rewrites every connection's name in the same undo step. Names stay the persistent key
  (F2); each definition also gets a `uid` so the editor and the loader stay bound across a rename.
- Runner-up: delete without a guard. Loses: the controls it drove would silently read 0.

## (4) ITEMS

### Stage A -- model, evaluation, persistence, undo, registry sync (no new UI)

A1 AutomationCurve bend. src/connect/AutomationCurve.h:
- `Breakpoint`: add `float bend = 0.0f;` AFTER `interp`, so every `{x, y, interp}` aggregate init still compiles.
- Add `static float bendShape(float t, float bend)` and `static float bendFromMidFraction(float u)` (F-4).
- `eval`: a Linear segment applies bendShape when bend != 0; Hold and Smooth ignore it.
- `Breakpoint::toVar` writes "bend" ONLY when non-zero; `fromVar` reads it clamped to [-1, 1], absent = 0.
- Behaviour change: none for any existing curve (all have bend 0). Takes, Timeline and routines stay byte-identical.
- ConnSerialization is NOT touched: connection curves have no bend. Note for the future per-parameter editor.
- RED: new cases in tests/test_connection.cpp (AutomationCurve section :617+):
  - bendShape(0.5, b) == 1 / (e^(bK/2) + 1);
  - the inverse round-trips;
  - a Linear segment with bend 0.5 gives the bent value;
  - Hold and Smooth ignore bend;
  - the JSON of a bend-0 curve equals the pre-change string exactly;
  - "bend" round-trips.
  Fails on main: does not compile (no `Breakpoint::bend`). GREEN: these plus test_connection, test_take,
  test_recorder_host and the routine tests, unchanged. Risk: low.

A2 EnvelopeDef + composition persistence.
- NEW src/signal/EnvelopeDef.h (juce_core only):
  - `struct EnvelopeDef { std::string uid, name; float lengthBeats = 4, amplitude = 1, phaseOffset = 0;
    enum class Mode { Loop, Once } mode = Loop; AutomationCurve curve; std::optional<EnvelopeSample> sample; }`
  - `struct EnvelopeSample { std::string path, fileName; double durationSec; enum class Band { Whole, Lows, Mids,
    Highs } band = Whole; float detail = 0.5f; std::array<bool,4> silent; std::shared_ptr<const GhostBands> ghost; }`.
    GhostBands = 4 x std::vector<float>(1024), immutable, shared between copies.
  - `makeMod2Default()`.
  - toVar / fromVar. Keys: uid, name, lengthBeats, amplitude, phaseOffset, mode "loop|once", points (Breakpoint::toVar),
    sample {path, fileName, durationSec, band "whole|lows|mids|highs", detail, silent[4], ghost {n: 1024, whole,
    lows, mids, highs: base64 little-endian u16}}. An unknown enum string loads the default (D12 "ADD, never
    REDEFINE").
  - `sanitize(def, note)`: sort by x; clamp x and y to [0, 1]; drop duplicate x; force endpoints at x 0 and 1; at most
    256 points; clamp bend; clamp lengthBeats to [0.25, 256]; mint a uid (juce::Uuid) when missing.
- src/model/Composition.h:
  - field `std::vector<EnvelopeDef> envelopes = { EnvelopeDef::makeMod2Default() };`
  - non-serialized `std::string envelopeLoadNote;` and `uint64_t envelopesRevision = 0;`
  - initDefault() (:172-192) resets envelopes to { Mod 2 }.
  - toVar adds "envelopes" after "routineBank" (:429-437).
  - fromVar, after the routines block (:638-672): key absent -> keep the default { Mod 2 }, which is exactly today's
    runtime state for an old file (F7); key present -> parse + sanitize, and unreadable entries are counted into
    envelopeLoadNote (never silent).
  - fromVar never touches the registry: it runs on `incoming` (F17).
- RED: tests/test_envelope.cpp (NEW target, test_connection's link pattern, tests/CMakeLists.txt:595-641):
  - a definition with bends, a sample and a ghost round-trips;
  - old JSON without the key -> exactly the Mod 2 default;
  - an empty array -> no envelopes;
  - the sanitize cases;
  - the ghost survives base64 to within 1/65535.
  Fails on main: does not compile (no `Composition::envelopes`).

A3 EnvelopeSignal evaluates the definition. src/signal/EnvelopeSignal.h:
- KEEP the cycle-phase block verbatim (:35-63 at 5e47d17, plus anything bf2 adds there).
- Replace only the point lookup + interpolation (:65-104):
  - Loop -> `curve.eval(cyclePhase)`.
  - Once -> the start is pinned at the next bar line (ceil((beats - 1e-4) / 4) * 4) on the first evaluation, on
    `requestRestart()`, or on a backward clock jump of more than 1 beat (a manual Resync, mirroring
    ConnectionEngine.cpp:59-62). The value is y(0) before the start and y(1) once one cycle has passed.
  - Then multiply by amplitude.
- Members: a copy of the definition without the ghost (setDef / def()), plus mutable once-state and lastPhase (message
  thread only, like ParamConnection::state).
- Keep `ctor(name, beats)`, `setResetPhaseOnStructural`, and thin compat setters setPoints / setOneShot / setLooping /
  setBeatDuration / setAmplitude / setPhaseOffset (old intent: Once <=> oneShot && !looping), so the RED test below runs
  unchanged before and after.
- tests/CMakeLists.txt: test_oscillator_bar_fold (:240) gains juce::juce_core + the 4 JUCE defines
  (AutomationCurve.h includes juce_core).
- RED (behavioural, written FIRST against main's own API):
  - `setPoints({{0,0},{1,1}})`, `setOneShot(true)`, `setLooping(false)`; evaluate at beat 0, then at total beat 5.0
    (beatInBar 1, barsSinceResync 1) -> expect 1.0 (held end). Main returns 0.25 -> FAILS on main.
  - Also: a bent segment's value; a Resync re-pins; every existing case in test_oscillator_bar_fold passes unchanged
    (Mod 2 anchors :256-307).

A4 Commands. NEW src/core/EnvelopeCommands.h:
- `EnvelopeEditCmd(uid, before, after, what, mergeKey)`. Its first execute is a no-op (the state is already live). It
  merges into the previous EnvelopeEditCmd when the uid and a non-zero mergeKey match, so one slider drag = one entry.
- `EnvelopeListCmd(beforeVector, afterVector, what)` for New / Duplicate / Delete / Make Lows-Mids-Highs.
- `RenameEnvelopeCmd(uid, oldName, newName)`: renames and rewrites connection names (A5) in both directions.
- All three re-resolve by uid through EnvelopeService and never store pointers (UndoService.h:10-15). No GL fence:
  the GL thread never reads envelopes (F10).

A5 Connection visitor. NEW src/connect/ConnVisitor.h/.cpp:
- `forEachConnection(Composition&, MacroBank*, fn(ParamConnection&, Where))` visits exactly ConnectionEngine::tick's set
  (ConnectionEngine.cpp:317-384): macro conns, comp scalars, globalEffects paramConns + dryWetConn, and per deck the
  layer scalars and layerEffects, then per clip the scalars, effects and sourceParams.
- `countSignalUsers(name)` also counts the legacy `MacroBank::Macro::sourceSignalId` (by id, MacroBank.h:30-54).
- `renameSignalUsers(old, new)`.
- RED (test_envelope):
  - a composition with one Kind::Signal "Mod 2" connection at each of the 8 placements + one legacy macro ->
    countSignalUsers == 9;
  - renameSignalUsers rewrites all 8;
  - a cross-check: run ConnectionEngine::tick with "Mod 2" cached at 0.7 and assert every placement's twin moved, so a
    placement the visitor misses fails the test.
  Fails on main: does not compile.

A6 EnvelopeService + registry sync -- the ONE mutation path. NEW src/signal/EnvelopeService.h/.cpp:
- Headless. Collaborators: Composition*, SignalRegistry*, MacroBank*, UndoManager*. Message-thread jassert like
  UndoManager.cpp:8-10.
- API: find(uid) / findByName, revision(), usersOf(name); live (preview during a gesture, no command) / commit;
  create ("Envelope N") / duplicate ("X copy") / rename / remove (refused with the user count while in use);
  restart(uid); lastPhase(uid).
- `syncAll()`: the registry's EnvelopeSignals := composition.envelopes, by name (update in place, add missing, remove
  extra). A definition whose name equals a non-envelope signal (e.g. "Bass") is renamed "Bass (envelope)" with a note.
  Then `onRegistryChanged()` fires IN THE SAME CALL.
- MainComponent wiring (constructor, next to the SignalBar hookup MainComponent.cpp:647-653):
  - `onRegistryChanged` = `signalBar_->rebuildStrips()` + re-point the Signal tab: inspectSignal(the registry signal
    with the same name, or nullptr), via a new InspectorPanel::getSignalInspector().
  - `syncAll()` runs once after initDefaults (:526) and at the end of swapCompositionModel, after
    undoManager_.clear() and before refreshUiAfterModelSwap() (:3029-3031).
- TestServer::handleListSignals (TestServer.cpp:1015-1043) reads on the message thread (Box + 2 s WaitableEvent,
  ApiServer.cpp:2011-2042 pattern), same JSON. Tell tsan-r5 this route is done.
- RED-first order inside S1: commit the service + REST first, run probe row e8 under the app TSan build and record the
  handleListSignals report, THEN commit the hop (so the race this lane creates is shown before it is closed).
- RED (test_envelope):
  - sync adds / updates / removes, and renames on a collision;
  - cachedValues size == signals size;
  - onRegistryChanged fires once per structural change and never for a point edit;
  - remove is refused while in use;
  - undo / redo of every operation restores the composition AND the registry;
  - one slider drag merges into one undo entry.
- RED (test_envelope_ui, NEW target, ScopedJuceInitialiser_GUI pattern, tests/CMakeLists.txt:2991-3034): delete the
  inspected envelope, pump SignalBar::timerCallback once -> no strip whose getSignal() is the removed object and
  SignalInspector::getSignal() != removed (the UAF guard; run under ASan). Fails on main: does not compile.

A7 The old Signal-tab controls write through the service, so Stage A alone persists edits. SignalInspector.cpp
envelope block (:74-135):
- The curve combo applies Linear / Exponential (bend 0.282) / S-curve to ALL segments.
- Length, amplitude, phase and looping / one-shot -> EnvelopeEditCmd, through a std::function the SignalInspector
  exposes and MainComponent wires.
- refresh()'s Envelope branch (:274-295) reads the definition (by name) instead of the signal.

A8 TEST-ONLY REST (ApiServer.cpp TEST-ONLY block :300-331). Handlers hop to the message thread with a Box (2 s) and
answer with the outcome:
- `GET /api/envelopes`: every definition (uid, name, lengthBeats, amplitude, phaseOffset, mode,
  points [{x, y, interp, bend}], users, sample summary + ghost present, the live cached value), plus revision,
  loadNote and loader {pending, lastStatus, lastError}.
- `POST /api/envelope/edit {uid|name, fields..., points}`.
- `POST /api/envelope/manage {op: new|duplicate|rename|delete, uid|name, newName}` -> {ok, reason, users}.
- `POST /api/debug/undo {redo}`: the kCompUndo / kCompRedo code (~MainComponent.cpp:6469-6480).
- MainComponent sets the on* callbacks. The production build has none of these routes.

### Stage B -- the editor (BF4, visible)

B1 EnvelopeEditLogic, headless. NEW src/ui/EnvelopeEditLogic.h/.cpp (no JUCE GUI types).
- Inputs: the definition, the canvas rect, the snap step in beats, the selection, the modifiers.
- hitTest priority:
  1. a point within 8 px (nearest; the selected point wins a tie);
  2. a visible bend handle within 7 px;
  3. the segment line within 6 px of the drawn curve at that x;
  4. empty.
- Snap: x rounds to the snap grid unless Cmd is held; Shift = fine (1/10 speed) on both axes.
- Clamps: x stays between prev.x + 1e-4 and next.x - 1e-4; the endpoints' x is locked to 0 and 1; y in [0, 1].
- Double-click on empty space or a segment adds a point at the snapped x and the mouse y, and selects it. Refused at
  256 points (notice "Envelope is full (256 points)").
- Double-click a middle point deletes it; on an endpoint, notice "The first and last points stay".
- Handle drag sets bend = bendFromMidFraction((y - ya) / (yb - ya)). The handle is hidden when |yb - ya| < 1e-3.
  Double-click a handle = bend 0.
- Keys, consumed only while the canvas has focus AND a point is selected:
  - Left / Right: one snap step (1/64 of the cycle when Snap is Off);
  - Up / Down: 0.01 (0.1 with Shift);
  - Delete / Backspace: delete (not an endpoint);
  - Tab / Shift-Tab: next / previous point;
  - Esc: deselect. With nothing selected, Esc = "close the big view".
  EVERY other key returns false (letters, space, Cmd+Z), so the keyboard launcher and the app undo still work
  (MainComponent::keyPressed :3803+).
- Commits: one commit (before / after) per gesture at mouse-up; a click without movement commits nothing; arrow
  nudges on one point merge within one key-repeat run.
- RED: test_envelope [editor] cases for each rule, with fixed geometry: canvas 400 x 200, 1-bar envelope, 1/4-beat
  snap = 16 columns of 25 px. Fails on main: does not compile.

B2 EnvelopeCanvas. src/ui/EnvelopeCanvas.h/.cpp: a juce::Component + FileDragAndDropTarget + DragAndDropTarget; a thin
view over B1.
- Painting (F15 rules):
  - background #111111; 1-px grid: beats #2a2a2a, bars #3a3a3a with bar numbers in a 14-px ruler (monospace 10 pt,
    kTextSecondary); snap lines #202020 only where they are at least 6 px apart;
  - the ghost (Stage D) as a kTextSecondary area at 25 %;
  - the curve as a 2-px kAccentCyan line sampled every 2 px, with a 12 % fill below;
  - points as 7-px SQUARES (no circles: "no border-radius"); selected = 9 px with a 1-px white hairline; endpoints carry
    a 1-px vertical tick (a cue that they only move vertically);
  - dense curves (median point spacing < 6 px) draw points as 3-px ticks, with full squares only near the mouse or
    when selected;
  - bend handle = a 7-px hollow diamond, shown only for the hovered segment and the selected point's two segments;
  - while dragging, a monospace readout next to the cursor: "bar.beat  value%".
- Repaints ONLY on input or a model-revision change; no timer (Pitfall 57). A hover change repaints the two affected
  rects.
- Right-click opens a context menu for whatever is under the mouse, via showMenuAsync(...withParentComponent(
  getTopLevelComponent())) (CLAUDE.md UI rule): point -> Delete point; segment or handle -> Curve / S-curve / Step,
  Straighten; empty -> Add point here, Reset to default shape.
- Drop highlight: 1-px cyan border + 15 % fill.

B3 EnvelopePanel. src/ui/EnvelopePanel.h/.cpp, the self-contained editor:
- Header row: [envelope picker ComboBox][NEW][... menu: Duplicate, Rename..., Delete][BIG VIEW / CLOSE].
- The canvas.
- Controls:
  - LENGTH: 1 beat, 2 beats, 1 bar, 2 bars, 4 bars, 8 bars, 16 bars (= 1, 2, 4, 8, 16, 32, 64 beats). An off-list
    loaded value shows as an extra "N beats" item (the CanvasSizeCombo precedent).
  - SNAP: Off, 1 bar, 1/2 bar, 1 beat, 1/2 beat, 1/4 beat (default), 1/8 beat. Session UI state, not saved.
  - PLAY: [LOOP|ONCE] touching buttons + RESTART (enabled in Once; starts on the next bar).
  - LEVEL: ResettableSlider 0-100 %, default 100 %.
  - SHIFT: ResettableSlider 0..length in beats, default 0, disabled in Once.
  - One hint line in secondary text: "Drag points - double-click adds or removes - drag the middle handle to bend -
    right-click for shapes". A notice line appears only while a notice is live.
- Layout: stacked (controls under the canvas) below 600 px wide; side by side (a 240-px control column on the right,
  the canvas full height) at 600 px or more.
- Model binding: binds to a definition by uid and pulls from EnvelopeService only when revision() changed (Pitfall 41:
  it follows REST and undo writes, and does not repaint when nothing changed). Model pulls are ignored during a drag.
- Every slider is a ResettableSlider with setDefaultValue (Pitfall 5). A slider drag commits with mergeKey = that drag
  (one undo entry); a right-click reset commits too.
- Rename uses the app's async AlertWindow pattern (MainComponent.cpp:3686).

B4 SignalInspector hosting. SignalInspector.h/.cpp:
- For Signal::Type::Envelope, own and show the EnvelopePanel (std::unique_ptr).
- Remove the old envelope members: curveTypeSelector_, envBeatDurationSelector_ (copy its CURRENT item list first;
  see the bf7 note under shared files), envAmplitudeSlider_, envPhaseSlider_, the two toggles, curveEditorBounds_,
  paintCurveEditor.
- getPreferredHeight() for an envelope = max(content minimum, availableHeight_). InspectorPanel calls
  setAvailableHeight(signalViewport_.getHeight()) before its setSize calls (InspectorPanel.cpp:96-101, :191-198).
- The Audio and Oscillator branches are untouched (bf7 may own the oscillator combo).
- The SignalBar [+] menu (SignalBar.cpp:181-228) gains "New envelope" at the top (EnvelopeService::create, then
  inspect it).
- RED, behavioural on main (test_envelope_ui): `SignalInspector si; si.setSignal(&mod2); si.setSize(430, 330);
  getComponentAt(215, 120)` must be a child that intercepts mouse clicks, with height >= 150. Main returns the
  inspector itself (paint-only) -> FAILS.
- Further cases:
  - canvas >= 100 px at 430 x 230 (never smaller than today);
  - every Slider in the panel is a ResettableSlider with hasDefaultValue();
  - the Length items equal the agreed list;
  - Snap defaults to 1/4 beat; SHIFT is disabled in ONCE;
  - an edit through the service reaches the panel at the next refresh(), and a refresh() without a change triggers
    zero canvas repaints (counter).

B5 TEST-ONLY UI hook for captures: `GET|POST /api/debug/envelope_ui {inspect: name, select: i, hover_segment: i,
snap, expand}`. It applies to the live panel on the message thread and returns {bound uid, selected, hovered,
expanded, canvas rect, canvas repaint count}. Menus are captured through a TEMPORARY env hook (probe-deck-tabs.sh
pattern), reverted before merge.

### Stage C -- sample analysis core (headless; can build in parallel with Stage B -- needs only A2's types)

C1 NEW src/signal/SampleEnvelope.h/.cpp.
- `PcmSource { int read(float* mono, int max); double sampleRate; int64 lengthSamples; }`. The file adapter calls
  registerBasicFormats + createReaderFor, reads AudioBuffer<float>(numChannels, 8192) and averages all channels.
- `analyse(PcmSource&, Options{capSeconds = 600, minSeconds = 0.05}, cancelFlag)` -> {status Ok | TooShort |
  Unreadable | Truncated, durationSec, sampleRate, GhostBands, silent[4]}.
- DSP. The constants are the builder's starting point; the tests are the bar.
  - 20 Hz high-pass.
  - Lows = 2 x LR4 low-pass at 250 Hz. Highs = 2 x LR4 high-pass at 4 kHz. Mids = 2 x LR4 high-pass at 250 Hz, then
    2 x LR4 low-pass at 4 kHz. Whole = the high-passed input.
  - Per band, a sliding RMS (window 40 / 20 / 10 / 20 ms for lows / mids / highs / whole), sampled every hop =
    max(1 ms, length / 4096).
  - A peak-hold release follower, tau = 120 / 80 / 60 / 80 ms.
  - Normalise each band to its own peak. A band whose peak is below the strongest band x 10^(-30/20), or below 1e-3,
    is silent (flat 0).
  - Ghost = 1024 points over the analysed length (max-pool when compressing, linear when stretching).
- `simplify(ghost, detail, maxPoints = 256)` -> AutomationCurve. RDP on vertical distance with
  eps = 0.2 * 0.05^detail, multiplied by 1.25 until there are at most 256 points; endpoints at x = 0 and 1; Linear
  segments; bend 0.
- RED: tests/test_sample_envelope.cpp (NEW target: juce_core, juce_audio_basics, juce_audio_formats, juce_dsp), all on
  synthetic PcmSources:
  1. 1 s of 60 Hz, then 1 s of 12 kHz, at -6 dBFS -> lows mean >= 0.6 in the first half and <= 0.05 in the second;
     highs the mirror image; mids flagged silent.
  2. 4 click bursts (gen-click-wav.py shape) at 0 / 0.5 / 1.0 / 1.5 s of 2 s -> the Whole curve at detail 0.5 has
     exactly 4 local maxima >= 0.5, at x within +-0.02 of 0 / 0.25 / 0.5 / 0.75 (+ the burst offset).
  3. A sustained 1 s 60 Hz tone -> lows at detail 0.5 has <= 8 points (no ripple vertices).
  4. 0.03 s -> TooShort; random bytes -> Unreadable.
  5. A 3 s source with capSeconds 2 -> Truncated, durationSec == 2.
  6. points(detail 0.9) >= points(0.5) >= points(0.1), all <= 256.
  7. An in-memory WAV (WavAudioFormat -> MemoryOutputStream -> reader) gives the same ghost as the direct PcmSource,
     within 1e-4.
  8. A 3-channel input averages all channels.
  Fails on main: does not compile.

C2 NEW src/signal/SampleEnvelopeLoader.h, the ClipThumbnails pattern (F21):
- A 1-thread low-priority pool "EnvSample"; analyser and poster injectable; WeakReference; the cancel flag is checked
  every 8192-frame block; removeAllJobs(true, 5000) in the destructor.
- `request(uid, file, epoch)` -> token. A newer request for the same uid supersedes the older one.
  `landed(uid, token, epoch, result)` runs on the message thread.
- MainComponent declares it AFTER EnvelopeService, so it is destroyed first.
- RED (test_sample_envelope [loader]): a manual poster gets one landed call with the token; a superseded result is
  dropped; destroying the loader with a queued job -> no callback, no crash. Fails on main: does not compile.

### Stage D -- sample UI (BF5, visible)

D1 SAMPLE rows in EnvelopePanel.
- [LOAD SAMPLE...]: an async FileChooser, wildcard = formatManager.getWildcardForAllFormats(), start folder =
  userMusicDirectory. Next to it the file label "kick_loop.wav - 1.9 s", or the hint "or drop an audio file on the
  envelope".
- With a sample loaded:
  - BAND [WHOLE|LOWS|MIDS|HIGHS] touching buttons; silent bands are disabled with the tooltip
    "No lows in this sample".
  - DETAIL: ResettableSlider, default 0.5, with a monospace point count. Re-simplifies live during a drag; one undo
    entry at drag end.
  - [MAKE LOWS / MIDS / HIGHS]: three NEW envelopes "<stem> Lows / Mids / Highs" (made unique), silent bands skipped and
    named in the notice, one EnvelopeListCmd.
  - The "..." menu gains "Remove sample" (keeps the points, drops the ghost).
- The canvas draws the bound band's ghost.
- After a band / detail change rebuilds the points, the notice says "Points rebuilt from the sample -- Undo brings
  yours back".
- Notices:
  - "Reading kick_loop.wav..."
  - "Not an audio file this app can read"
  - "Too short to make an envelope (under 0.05 s)"
  - "Long sample: the first 10 minutes were used"
  - "Sample file not found -- the shape is kept" (shown on bind when the path is missing; never blocks).

D2 Drop.
- EnvelopeCanvas accepts Finder drops (any file the format manager can open) and internal "files:" drags
  (FilesBrowser.cpp:138). Both go to the same loadSampleFromPaths(first path) as the button.
- A result is applied as ONE EnvelopeEditCmd (undo restores the hand-made points), starting with band Whole.
- It is dropped when the uid is gone or epoch != modelEpoch_ (a composition swap happened, F17).

D3 TEST-ONLY `POST /api/envelope/sample {uid|name, path, band, detail, pull_bands, remove}` calls the same
loadSampleFromPaths (async; GET /api/envelopes shows the loader status).
- RED: probe rows e6-e7 (404 on main).
- Component test: a landed result binds the ghost, enables bands per silent[], and one undo restores the previous
  points.

### Stage E -- bigger, in place

E1 Big view.
- MainComponent owns `envelopeHost_` (a plain Component, kBackground fill, 1-px kPanelBorder, opaque).
- [BIG VIEW] re-parents the ONE EnvelopePanel from SignalInspector into the host; selection and snap are kept.
- In MainComponent::resized(), bottom block (right after inspectorPanel_->setBounds, MainComponent.cpp:2749-2753): host
  bounds = the TimingWindow + Inspector columns (d0x + kVDividerWidth .. d2x, full btmH); toFront(false) BEFORE the
  binding-overlay toFront (:2762-2767).
- [CLOSE], or Esc with nothing selected, re-parents it back. The Signal tab shows "Editing in the big view" meanwhile.
- Selecting a non-envelope signal or swapping the composition closes the big view first.
- RED (test_envelope_ui): after expand, the panel's parent == the host; after collapse, == SignalInspector; the
  selection survives; at the default fractions the big-view canvas is at least 2x the tab canvas width. Fails on main:
  does not compile.

E2 OPTIONAL (Q2): a playhead in the big view only.
- On expand, attach a NativeLayerHost to the host (legal: it is not in a Viewport, NativeLayerHost.h:19) with a new
  uipaint::Layer id (UiPaintCounters.h + the ui_paint export, ApiServer.cpp ~:1860-1905).
- A 30 Hz timer reads EnvelopeService::lastPhase(uid) and repaints only the playhead column, only when its pixel
  changes. Detach on collapse.
- RED: with the big view open and idle, /api/debug/ui_passes shows zero in-peer passes sourced by the host. Fails
  on main: there is no host.

### Stage F -- OPTIONAL (Q6): envelopes travel with saved decks

A deck save writes the envelopes its connections reference (ConnVisitor limited to that deck); appending a deck merges
names that are missing. Touches the deck save / append paths in MainComponent (shared with bf9 and ui). Build only if
Boris wants it.

### BUILD STAGES

| Stage | Items | Builds after | Ships alone? |
|---|---|---|---|
| S1 | A1-A8 | -- | YES: envelopes are saved, Once works, REST and gates exist; no new editor |
| S2 | B1-B5 | S1 | YES: delivers BF4 |
| S3 | C1-C2 | -- (own worktree, in parallel with S2) | invisible on its own |
| S4 | D1-D3 | S2 + S3 | YES: delivers BF5 |
| S5 | E1 (+ optional E2) | S2 | YES: can ship before S4 |
| S6 | F (optional) | S4 | -- |

Merge order inside the lane: S1 -> S2 -> S3 -> S4 -> S5 (-> S6).

### FILES SHARED WITH OTHER LANES (exact blocks)

- **src/ui/SignalInspector.cpp/.h.** bf45 touches the envelope constructor block :74-135, resized() Envelope case
  :221-236, refresh() Envelope branch :274-295, getPreferredHeight :307, paintCurveEditor :365-422, and the members
  :43-52 / :66. bf7 likely edits :87-101 / :280-287 (envelope lengths) and :41-56 / :258-267 (oscillator).
  Rule: bf7 merges first; bf45 copies the post-bf7 envelope length list into EnvelopePanel and leaves the oscillator
  code alone. If bf7 exposed a shared beat/bar list, LENGTH uses it.
- **src/signal/EnvelopeSignal.h.** bf2 (sync dial) may change the phase block :35-63. bf45 keeps that block verbatim
  and replaces only :65-104 + the members.
- **src/model/Composition.h.** bf45 touches the field block near routines :41-49, initDefault :172-192, the toVar
  tail :424-438 and the fromVar tail :638-673. bf9 / bf1 touch other blocks.
- **src/MainComponent.cpp.** bf45 touches:
  - the constructor (registry init :526-527, SignalBar hookup :647-653, ApiServer callbacks ~:2128);
  - the end of swapCompositionModel :3029-3031;
  - the resized() bottom block :2743-2760 (S5).
  bf9 (deck switch), bf1 (record) and ui (rename) edit other functions.
- **src/api/ApiServer.cpp/.h.** New handlers in the TEST-ONLY block :300-331, marshalled from birth (tsan-r5
  marshals the older routes later).
- **src/test/TestServer.cpp.** handleListSignals :1015-1043 (tsan-r5's R7 class; tell tsan-r5 it is done).
- **src/ui/InspectorPanel.cpp/.h.** getSignalInspector(); setAvailableHeight before the :96-101 / :191-198 setSize
  calls.
- **src/ui/SignalBar.cpp.** showAddSignalMenu :181-228.
- **src/connect/AutomationCurve.h.** Additive Breakpoint field; bf6 reads the struct and needs no change.
- **CMakeLists.txt (app sources) + tests/CMakeLists.txt.** Append-only: new targets, and the
  test_oscillator_bar_fold link line :240.
- **NOT touched (bf6 core):** ConnPicker.*, ConnectionEngine.*, ConnSerialization.*, UniversalParamControl.*.

## (5) GATES (Harmony runs after the merge)

G1 Unit tests.
- The full ctest suite on the merged build: 0 failures.
- The new targets test_envelope, test_envelope_ui and test_sample_envelope pass.
- The builder's report holds the log path showing each new test FAILED or did not compile on 5e47d17, and the A3 Once
  case failing at 0.25.

G2 TSan.
- probe-tsan-unit.sh on test_sample_envelope [loader] (threaded): 0 reports.
- The app TSan scenario (probe-tsan.sh infrastructure) running probe row e8: 0 reports whose stack contains
  SignalRegistry / EnvelopeSignal / EnvelopeService / SampleEnvelope.
- RED shown on S1 before the handleListSignals hop (A6).

G3 Live probe `.harmony/probe-envelope.sh` + `.py` + `.json`, a clone of probe-capture.sh: live lock, `open -g
--args --test-mode`, 7070 + 8080, `Connection: close`, graceful quit, never an Output window, no synthetic input.
- e1: on a fresh launch, GET /api/envelopes = exactly [Mod 2: (0,0)(0.5,1)(1,0), 4 beats, loop]. RED on main: 404.
- e2: /api/envelope/edit makes Mod 2 a ramp with bend 0.5 on its middle segment; /api/inject_features drives the
  clock to 5 positions; /api/signals "Mod 2" == the formula value (computed in the .py) within 0.01 at each.
- e3: /api/debug/undo -> the previous points; redo -> the edited points.
- e4: manage new -> "Envelope 1" appears in /api/signals; duplicate; then load a fixture composition where a comp
  scalar connects to "Envelope 1" and rename it to "Kick" -> /api/envelopes users("Kick") == 1; delete "Kick" ->
  refused {users: 1}; delete an unused envelope -> gone from /api/signals.
- e5: load a fixture with bends + one sample whose path does not exist -> envelopes intact, loadNote empty, sample
  notice "not found", ghost present.
- e6 sample:
  - a gen-click-wav.py 2 s click file (4 clicks) -> status ok within 5 s; durationSec within 1 %;
    2 <= points <= 256; Whole has 4 maxima >= 0.5;
  - a two-tone fixture (NEW .harmony/gen-envelope-wav.py, python stdlib) -> the lows / highs result of C1 test 1;
  - `afconvert -f m4af -d aac` of the two-tone -> lows ghost Pearson r >= 0.95 against the WAV's; report the lag.
- e7: pull_bands on the two-tone -> "<stem> Lows" + "<stem> Highs" created; mids skipped with a notice.
- e8 concurrency: 200 x (manage new, manage delete) interleaved with 200 x GET /api/signals (8080) -> every response
  well-formed, the app alive, final count == 32 + the remaining envelopes.
- e9: after quit no Audio-DNA process remains; no foreign REST traffic (the probe-capture.sh FOREIGN check).
- Decision rule: every row must PASS; a FAIL triggers a fix round; a row whose teeth cannot be shown RED on main is
  INFO.

G4 Idle paint (Pitfall 57). probe-idle-paint.sh i1 / i2 / g4 with the Signal tab showing Mod 2
(/api/debug/envelope_ui inspect).
- Deterministic teeth: the canvas repaint counter stays 0 over 10 s idle (bar: 0).
- Timing rows: INTERLEAVED A/B, main 5e47d17 vs merged, at least 5 runs per arm, after a ps check for CPU burners.
  Bar: merged median idle main-thread ms/s <= main median + max(10 %, 2 x MAD(main)). If MAD >= the effect, the row
  is INFO.
- With E2: the big view open and idle -> zero in-peer passes sourced by the host.

G5 Perf, INFO only. tickFeaturePipeline cost with 12 envelopes x 256 points vs the default, interleaved A/B, at least
5 runs per arm. Expected < 0.05 ms per tick (message thread).

G6 VISUAL WORK GATE, before Boris sees anything.
- Captures by Quartz window id only (screencapture -l, decoded and checked non-black):
  - v1: the Signal tab with Mod 2 default (stacked layout);
  - v2: a hand-made envelope with bends, a selected point, a hovered segment handle and the readout;
  - v3: a dense sample envelope with its ghost (Lows), bands and Detail;
  - v4: notices (reading..., not found, no lows);
  - v5: the big view (side layout) at the default dividers;
  - v6: (E2) the big view with the playhead;
  - v7-v9: menus via the TEMPORARY hook snapshot: segment shape, "...", the picker list;
  - v10: the delete-refused notice;
  - v11: the SignalBar with new envelope strips.
- Critic panel:
  - visual-design;
  - UX;
  - graphic-design, holding F15 (no rounded corners, touching buttons, ALL-CAPS mode labels, monospace numbers, one
    accent);
  - logic;
  - a dedicated interaction-logic critic for selection, drag and snap, double-click add / remove, keyboard capture vs
    launcher keys, undo granularity, expand / collapse, and the picker vs SignalBar selection.
- At most 1 fix round per finding class. Then the Boris page .harmony/.reports/s-rta-1002b/boris-checks.html: the
  captures, the defaults taken, the section 8 checks and the section 9 questions.

## (6) DOCS

- **docs/claude/rendering.md.**
  - Fix :87: signal evaluation runs on the message thread (S166-L1), and RoutingEngine::processFrame was deleted.
  - Add "### Envelopes (s-rta-1002b BF4/BF5)":
    - data: Composition::envelopes, EnvelopeDef, uid vs name;
    - EnvelopeSignal is a cache synced by EnvelopeService::syncAll;
    - evaluation and Once semantics;
    - the bend formula;
    - editor rules: hit priority, snap, keys, undo;
    - hosting: the tab, the big view, and no playhead inside a Viewport;
    - sample analysis: constants, formats, caps;
    - the save format (ghost base64);
    - the TEST-ONLY routes.
- **docs/claude/architecture.md.** Source-tree lines :235-237, :274, :304-306 (the new files).
- **docs/claude/testing-eyes.md.** probe-envelope, /api/debug/envelope_ui and /api/debug/undo witness lines.
- **docs/claude/pitfalls.md.** "Pitfall NN" (proposed 64; Harmony assigns):

  > The signal registry changes at runtime (envelopes): an envelope's data is Composition::envelopes and
  > EnvelopeSignal is a cache. Never edit the cache, and never hold a Signal& / Signal* / registry index across a
  > message-loop turn. Every add / remove / rename goes through EnvelopeService, whose syncAll() rebuilds the
  > SignalBar and re-points the Signal tab in the same call. Nothing reads the registry off the message thread
  > (TestServer /api/signals hops).

  Guards: test_envelope [sync], test_envelope_ui (UAF), probe-envelope e8, G2.
- **CLAUDE.md** (24,002 of 25,000 B after hyg). Additions:
  - a Pitfall index line 64 after :227 (~150 B);
  - :14 capabilities, add "hand-drawn and sample-derived envelopes" (~42 B);
  - :247 trigger-table row, add "envelopes" (~11 B).
  Payment: move the per-category source breakdown "(3D 24, ... Routing 1)" on :14 (200 B) to APP-INVENTORY, whose
  header says counts live there. Harmony settles against the size at merge time.
- **.harmony/APP-INVENTORY.md.**
  - :75 SignalInspector row: the new editor, controls, big view, sample;
  - :105: the Envelope submenu lists composition envelopes;
  - section 6, :285: the EnvelopeSignal description;
  - the TEST-ONLY routes (the ~:215-235 block);
  - ctest targets +3; probes + probe-envelope.
- **BORIS_DECISIONS.md.** A new entry "Envelopes (2026-10-02, BF4/BF5)" holding:
  - his quotes;
  - the defaults taken: bigger in place, own window later; drag / add / delete / bend; bar lengths; silent sample
    stretched to the length; Lows / Mids / Highs;
  - the answers to section 9.

## (7) RISKS (each with its cheapest discriminating test)

| # | Risk | Cheapest discriminating test |
|---|---|---|
| R1 | A stale Signal& / Signal* after an envelope is deleted (SignalStrip, SignalInspector) -> UAF at the next 30 / 10 Hz tick. | The A6 test_envelope_ui case under ASan. |
| R2 | Per-band normalisation amplifies filter leakage, so an empty band looks full. | C1 test 1 (mids silent) + the -30 dB weak-band rule. |
| R3 | Ripple vertices explode the point count on sustained bass. | C1 test 3 (<= 8 points). |
| R4 | The focused editor eats arrows / Delete / Tab while a point is selected, so a launcher binding on those keys does not fire. | The B1 key-consumption table + Boris check 8.4. |
| R5 | bf2 / bf7 rebase collisions (the EnvelopeSignal phase block; the length combo). | After rebase, `git diff <bf2 merge>..HEAD -- src/signal/EnvelopeSignal.h` shows nothing inside the phase block; the Length-items test reads the post-bf7 list. |
| R6 | Rename does not rewrite DECK files on disk, so a later Append Deck shows that control at 0. | A probe fixture (deck saved before the rename, appended after) shows it. Documented; Stage F fixes it if Boris wants. |
| R7 | An accidental timer in the panel brings Pitfall 57 back. | The G4 repaint-counter row (deterministic). |
| R8 | Ghost size in saved files (~11 KB per sample envelope; x4 after Make L/M/H). | e6 prints the file size; INFO. |
| R9 | A sample result lands after a composition swap and writes into the wrong composition. | The C2 epoch case + the D2 rule. |
| R10 | Core Audio's m4a decode adds priming samples (~2,112 = ~44 ms), shifting the curve slightly. | e6 correlation >= 0.95, with the lag reported. |
| R11 | Strongest counterargument to the whole plan: a GLOBAL envelope library would let Boris reuse sample envelopes across sets. | It loses now: the controls that use an envelope are composition data naming it, so the composition must carry the envelope to stay portable. A library (save / load an envelope preset) can be added later. |

## (8) WHAT ONLY BORIS CAN CHECK

- 8.1 Is the Signal-tab editor big enough day to day, and is the big view (covering the Timing + Inspector panels) the
  right "bigger"?
- 8.2 The feel of dragging, double-click add / remove, the bend handle, the default snap (1/4 beat) and the readout.
- 8.3 Sample envelopes: do Lows / Mids / Highs move the visuals the way the sample sounds (how fast they fall back, how
  much detail by default)?
- 8.4 While a point is selected the arrows and Delete belong to the editor: does that ever fight his keyboard-launcher
  keys?
- 8.5 Loop / Once + Restart (starts on the next bar).
- 8.6 The names of pulled-out envelopes, and the new strips showing up in the SignalBar.

## (9) QUESTIONS FOR BORIS (plain words; each has a default so the build never waits)

- Q1 "The bigger envelope view covers the two middle panels (the empty timing panel and the inspector); your preview
  and file browser stay visible. OK?" Default: yes.
- Q2 "Do you want a moving line showing where the envelope is right now? It only works in the bigger view -- in the
  small one it would make the whole app redraw all the time." Default: yes, in the bigger view only (E2).
- Q3 "'Play once': when you press Restart, should it start on the next bar or right away?" Default: on the next bar.
- Q4 "Do you want to fire 'Restart' from a pad or a key while performing?" Default: not now (a later lane).
- Q5 "Long samples: squeeze the whole sample into the envelope length (up to 10 minutes), or pick a section?"
  Default: the whole sample; picking a section can come later.
- Q6 "If you save a deck and load it into a different set, should its envelopes come along?" Default: not in this
  lane; for now they are saved with the set.

STATUS: COMPLETE -- plan ready for Harmony adoption (stages S1-S6, gates G1-G6, 6 Boris questions with defaults)
