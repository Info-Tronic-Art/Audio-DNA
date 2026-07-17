# Lane 4 — Control / Data Plane Census

Re-normalization audit 2026-07-16. Read-only. Evidence = file:line.
Scope: `src/mapping/`, `src/routing/`, `src/signal/`, `src/binding/`, `src/midi/`,
`src/osc/`, `src/sync/`, `src/model/`.

Auditor note: CLAUDE.md's source-tree section does not document `src/routing`,
`src/signal`, `src/binding`, `src/midi`, `src/sync`, or `src/model` at all (it
lists a nonexistent `src/keyboard/`). This lane is the least-documented part of
the app; census below is built from source truth.

---

## 1. CENSUS

### 1.1 Mapping engine (`src/mapping/`)

**Pipeline** (`MappingEngine.cpp:135-216`, header contract `MappingEngine.h:8-21`):
per mapping, per frame: extract source from `FeatureSnapshot` → normalize
`(raw-inputMin)/(inputMax-inputMin)` clamped [0,1] → apply curve → scale to
[outputMin,outputMax] → per-mapping EMA smooth → **accumulate** into target param.
Multi-mapping semantics: `processFrame` first **resets every targeted param to 0**
(`MappingEngine.cpp:162`), then sums all enabled mappings, then clamps to [0,1]
(`MappingEngine.cpp:203-215`). So multiple mappings on one param sum; a base/manual
param value is destroyed once any mapping targets it.

**MappingSource** (`MappingTypes.h:6-87`): **58 real sources** + `Count` sentinel.
Groups: Amplitude 3 (RMS, Peak, RmsDB); Loudness 3 (LUFS, DynamicRange,
TransientDensity); Spectral 4 (Centroid, Flux, Flatness, Rolloff); 7-Band 7
(Sub, Bass, LowMid, Mid, HighMid, Presence, Brilliance); Rhythm 6 (OnsetStrength,
BeatPhase, BPM, BarPhase, PhrasePhase, BarCount); Structural 1 (StructuralState);
Pitch/harmony 4 (DominantPitch, PitchConfidence, DetectedKey, HarmonicChange);
Timbre 13 (MFCC0..MFCC12); Chroma 12 (ChromaC..ChromaB); Advanced-P25 5
(SidechainPump, SwingRatio, FormantPresence, ResonancePeak, ReeseBass). Each maps
1:1 to a FeatureSnapshot field in `MappingEngine::extractSource` (`MappingEngine.cpp:43-128`).

**MappingCurve** (`MappingTypes.h:90-120`): **24 usable curves** (indices 0-23) +
`Count`. 5 classic (Linear, Exponential x², Logarithmic log(1+9x)/log10, SCurve
smoothstep, Stepped floor(xN)/N default N=4) + 19 P24 easings (Circular/Back/
Elastic/Bounce/Cubic/Sine × In/Out/InOut = 18, plus Hold gate at index 23).
Implemented as pure fns in `CurveTransforms.h:13-218`, dispatched by
`CurveTransforms::applyCurve` switch cases 0-23 (`CurveTransforms.h:222-251`).

**Mapping struct** (`MappingTypes.h:123-140`, 10 fields): source, targetEffectId,
targetParamIndex, curve, inputMin(0)/inputMax(1), outputMin(0)/outputMax(1),
smoothing(0.15 EMA alpha), enabled(true).

**MappingSuggester** (`MappingSuggester.h/.cpp`): stateless AI suggester. Universal
suggestions from bass/mid/high/onset/centroid/rms activity + genre-specific for all
8 GenreDetector genres (House, Techno, DnB, HipHop, Ambient, Rock, PopElectronic,
JazzOther). Returns scored `Suggestion{sourceName,targetCategory,targetEffect,
targetParam,curveType,reason,relevance}`. **GHOST: never instantiated anywhere
in repo** (grep of src/tests empty).

### 1.2 Routing engine (`src/routing/`)

**Route struct** (`Route.h:8-41`, 17 fields): id; SourceType {Signal, Macro} +
sourceId; TargetScope {Clip, Layer, Global} + targetLayerId/targetClipId/
targetEffectIndex/targetParamIndex; transform outputMin(0)/outputMax(1)/inverted;
dialRangeMin(0)/dialRangeMax(1); threshold(0)/gain(1)/falloff(0.1); enabled.
`RouteTarget` (`Route.h:45-61`) = scope+layerId+clipId+effectIndex+paramIndex with
`operator==`; used internally by MacroLink (not referenced outside lane).

**RoutingEngine** (`RoutingEngine.cpp`): id-keyed add/remove (linear scan),
`getRoutesForTarget`/`getRoutesForSource`. `processFrame` (`RoutingEngine.cpp:77-126`):
read signal cached value → dial-range normalize → threshold (below → falloff toward
0 via lastValues_; above → `(norm-threshold)*gain` clamp) → invert → scale output →
EMA smooth → `ParamWriter` callback. **Wired into render loop**:
`Renderer.cpp:198 routingEngine_.processFrame(...)`. Macro sources noted as resolved
by MacroBank before routing (`RoutingEngine.cpp:91` comment).

**MacroBank / Dashboard** (`MacroBank.h`): "Dashboard Links" — `kNumMacros = 8` per
`Scope {Clip, Layer, Global}`. `Macro{name, manualValue(0.5), currentValue(0.5),
sourceSignalId(0=Manual), links vector, id}`; `MacroLink{RouteTarget target,
outputMin(0), outputMax(1), inverted}`. `updateValues` sets currentValue from manual
or signal cached value; usable as a route source. **Only the Global bank is
instantiated** (`MainComponent.h:207 globalMacroBank_{Scope::Global}`); no per-clip
or per-layer MacroBank exists anywhere → 8 live macros at runtime, not 24.

### 1.3 Signal system (`src/signal/`)

**Signal** base (`Signal.h`): Type {Audio, Oscillator, Envelope}; Category {Amplitude,
Bands, Rhythm, Pitch, Chroma, Timbre, Structure, Modulation}; id + visible flag;
`getValue(snapshot)` pure virtual, evaluated once/frame.

Concrete signals:
- **AudioSignal** (`AudioSignal.h`): wraps a MappingSource, delegates to
  `MappingEngine::extractSource`.
- **OscillatorSignal** (`OscillatorSignal.h`): 5 shapes (Sine, SawUp, SawDown,
  Triangle, Square); BPM-locked phase from `beatPhase + beatInBar` / beatDuration;
  amplitude, phaseOffset. Default Sine @ 1.0 beat/cycle.
- **EnvelopeSignal** (`EnvelopeSignal.h`): control-point curve (default 3-pt
  0→1→0), 3 segment curve types (Linear, Exponential, SCurve), beatDuration(4),
  amplitude, phaseOffset, oneShot(false), looping(true). Always BPM-locked; no ADSR,
  no external/onset trigger.
- **ClipPositionSignal** (`ClipPositionSignal.h`): atomic playhead position, updated
  by render thread via `setCurrentPosition`/`updateFromClip` (normalizes to in/out).
- **ChainedSignal** (`ChainedSignal.h/.cpp`): one signal modulates another; ChainMode
  {Multiply, Add, Gate, ScaleRange}; reads carrier/modulator via registry cached
  values, ignores snapshot. **GHOST: never instantiated anywhere in repo** (grep
  empty). `SignalRegistry::addSignal` wires a registry ptr into any ChainedSignal
  passed in (`SignalRegistry.cpp:90`), but nothing ever constructs one.

**SignalRegistry** (`SignalRegistry.cpp`): id-keyed store + parallel cachedValues_.
`initDefaults` registers **32 signals** (`SignalRegistry.cpp:4-84`): 8 visible audio
(Volume, Sub Bass, Bass, Mid, Air, Tempo, Beat Position, Hit), 21 hidden audio
(Peak, Punch, Hits Per Second, Low Mid, High Mid, Presence, Hit Strength, Bar
Position, Phrase Position, Bar Count, Brightness, Change, Noisiness, Note, Note
Confidence, Chord Change + P25 Sidechain Pump, Swing, Vocal Presence, Resonance,
Reese Bass), 1 Oscillator (Mod 1), 1 Envelope (Mod 2), 1 hidden ClipPosition.
`evaluateAll` recomputes all cached values in insertion order (`SignalRegistry.cpp:154`);
`getCachedValue` is an O(n) linear scan by id (`SignalRegistry.cpp:162`).
NOTE header comment (`SignalRegistry.h:23`) says "12 audio + 6 modulation" — stale;
actual is 29 audio + 3 modulation.

### 1.4 Binding system (`src/binding/`)

**Binding struct** (`Binding.h`): InputType {Keyboard, MidiNote, MidiCC} + keyCode/
mods / midiChannel(0=any)/midiNote/midiCC. **Action enum — 19 actions** (`Binding.h:24-45`):
TriggerClip, TriggerColumn, ToggleLayerBypass/Solo/Mute/Autopilot/Visible,
LayerTransport, ToggleEffectBypass, AdjustMacro, AdjustLayerOpacity, SwitchDeck,
TapTempo, Resync, GlobalPlayPause, GlobalStop, MasterOpacity, Snapshot, ToggleRecording.
TriggerMode {Toggle(default), Momentary} (`Binding.h:50-55`); CCMode {Absolute,
Relative} + ccStepSize(0.01) (`Binding.h:59-65`); **TargetMode {ByPosition, ThisItem,
Selected}** + targetClipId (`Binding.h:69-76`); velocityToOpacity flag (`Binding.h:79`);
action params targetLayerIndex/targetColumn/targetDeckIndex/targetEffectIndex/
targetMacroIndex.

**BindingManager** (`BindingManager.cpp`): id-keyed store; `processKeyDown/Up`,
`processMidiNoteOn/Off`, `processMidiCC`. Momentary mode fires value 0 on key-up
(`:78-102`) / note-off (`:129-150`); toggle ignores release. Relative CC accumulates
per-(channel,cc) around 0.5, delta `(value-64)*ccStepSize` clamped [0,1]
(`:161-201`). Binding-capture mode routes next input to captureCallback (MIDI-learn).
JSON preset save/load (`toVar/fromVar/saveToFile/loadFromFile`, `:214-306`).
NOTE: relative-CC if/else branches (`:179-182`) are identical — harmless redundancy.
NOTE: velocityToOpacity is stored + velocity passed as callback value; opacity
application lives in the MainComponent action handler (out of lane).

### 1.5 MIDI (`src/midi/`)

**MidiHandler** (`MidiHandler.cpp`): JUCE MidiInputCallback. `start` enables ALL
available inputs and registers callbacks; hot-plug via `enableDevice`. Incoming
messages posted to message thread via `callAsync` then dispatched to BindingManager
(NoteOn/NoteOff/Controller) (`:70-102`). Wired: `MainComponent.cpp:1081-1082`.

**MidiOutputHandler** (`MidiOutputHandler.cpp`): Launchpad/APC pad feedback.
5 PadStates → velocities: Empty=0(noteoff), Loaded=5(dim amber), Playing=60(green),
Triggered=52(flash green), ActiveWithFx=62(bright yellow) (`MidiOutputHandler.h:53-67`).
`updateFromDeck` diffs against cached `padStates_` (10×20) and sends only changed
cells (`:63-126`); note layout `(row+1)*10+(col+1)` Launchpad X (`:166-179`). Polled
~50ms from MainComponent timer (`MainComponent.cpp:1818-1819`). Wired.

### 1.6 OSC input (`src/osc/`)

**OscHandler** (`OscHandler.cpp`): JUCE OSCReceiver, MessageLoopCallback. **11 address
patterns** dispatched in `oscMessageReceived` (`OscHandler.cpp:43-187`):
1. `/audiodna/clip/{layer}/{column}` — trigger clip (value>0) (`:57`)
2. `/audiodna/layer/{n}/opacity` (`:73`)
3. `/audiodna/layer/{n}/bypass` (value>0.5) (`:86`)
4. `/audiodna/layer/{n}/solo` (`:99`)
5. `/audiodna/layer/{n}/mute` (`:112`)
6. `/audiodna/deck/{n}` (value>0) (`:125`)
7. `/audiodna/master` (`:138`)
8. `/audiodna/bpm` (`:146`)
9. `/audiodna/snapshot` (`:154`)
10. `/audiodna/macro/{n}` (`:162`)
11. `/audiodna/effect/{name}/{param}` (`:175`)

**DEAD AT RUNTIME**: `OscHandler::startListening(port)` (`OscHandler.cpp:15`) is never
called anywhere in src/tests (grep empty) — the receiver never connects. Additionally
only 5 of 11 callbacks are wired in MainComponent (`MainComponent.cpp:1127-1141`:
onTriggerClip, onSwitchDeck, onSetMaster, onSetLayerOpacity, onSnapshot). The other
6 (onSetLayerBypass, onSetLayerSolo, onSetLayerMute, onSetBpm, onSetMacro,
onSetEffectParam) are never assigned → no-op even if listening were started.

### 1.7 Ableton Link sync (`src/sync/`)

**LinkSync** (`LinkSync.h/.cpp`): thin wrapper over `ableton::Link` behind
`AUDIODNA_HAS_LINK`. Atomics for enabled/bpm/beatPhase/numPeers; quantum default 4.
`update()` pulls tempo/phase/peers from session; `setBPM` propagates to peers;
`requestBeatAtTime` aligns downbeat. Wired: `MainComponent.cpp:1803-1806` (when
enabled, overrides BPM tracker). **`AUDIODNA_BUILD_LINK` defaults OFF**
(`CMakeLists.txt:35`) → in a default build `AUDIODNA_HAS_LINK` is undefined and every
LinkSync method is a compiled no-op (setBPM only stores/echoes the value). Documented
as optional.

### 1.8 Core data model (`src/model/`)

**Clip** (`Clip.h`, ~60 fields): identity; MediaType {None, Image, Video, Camera,
Source, ImageSequence}; media file/camera/sourceType; image-sequence (files, fps 2.5,
beatDivision 4, videoBeats 4); sourceParams; per-clip EffectSlot chain
{effectName, paramValues, dryWet(1), enabled, bypassed}; TransportMode {Timeline,
BPMSync}; LoopMode {Loop, PingPong, OneShot}; speed/reverse/startOffset; in/out
points; **BeatSnapMode {Off, Beat, Bar, TwoBar, FourBar}** (`Clip.h:72-80`) + legacy
beatSnap bool; 8 cuepoints; **AutopilotAction 8** {LayerDetermined, DoNothing,
PlayNext, PlayPrevious, PlayRandom, PlayFirst, PlayLast, PlaySpecific} +
autopilotSpecificCol; **AutopilotDuration 8** {LayerDetermined, Beat1/2/4/8/16/32,
Custom} + customBeats; video props (clipOpacity, w/h, BlendOverride, AlphaType,
channel RGBA); per-clip transform (posX/Y, scale, rotation, anchorX/Y); MilkDrop
preset playlist (PlaylistCycleMode 5, PlaylistTrigger 6, blend, enable); contentLocked;
runtime playing/playheadPosition/beatsPlayed/hasBeenTriggered. `replaceContent`
preserves effects+transport+transform+autopilot (`Clip.h:170-241`).

**Layer** (`Layer.h`, ~40 fields + FeedbackConfig): Type {Opaque, Transparent,
FXOnly, ThreeD, Mask}; controls (opacity, visible, bypassed, solo, muted,
autopilotEnabled, ignoreColumnTrigger, persistent, folded); **MixMode enum** (25
blend modes + ~30 transitions in one list, `Layer.h:59-93`); KeyingMode 13; 3D
controls; per-layer w/h + AutoSizeMode 5; transitionMode/transitionBlendMode/
transitionSpeed; per-layer transform; FeedbackConfig {enabled, amount, scaleX/Y,
rotation, offsetX/Y, lumaKey, presetName}; per-layer EffectSlot chain; autopilot
defaults (action PlayNext, duration Beat4, customBeats, autopilotLoops, endOfVideo);
`clips` = vector<optional<Clip>> (one per column). Trigger logic with beat-snap queue
(`triggerClip`/`triggerClipImmediate`/`processPendingTrigger`, `Layer.h:190-286`):
snap granularity honored — Beat=every beat, Bar=beatInBar==0, TwoBar/FourBar=downbeat
& barCount%2/%4==0.

**Deck** (`Deck.h`): name/id; layers vector; numColumns(12); default 3 layers (L0
Opaque, rest Transparent) × 12 cols; add/remove/move layer, add/remove column,
`triggerColumn` (respects ignoreColumnTrigger), setClip.

**Composition** (`Composition.h`, ~30 fields): decks vector + activeDeckIndex; global
EffectSlot chain; masterOpacity/masterSpeed/compOpacity; CrossFader (phase,
CrossfaderBlendMode 3, Behaviour 2, Curve 3); comp transform; globalTransitionSpeed,
bpmMultiplier, QuantizeMode 3; composition autopilot (AutopilotDirection 4,
DurationMode 3, clipLoops, loop, masterLayer); **PerTypeAutopilotConfig 11 fields**
(opaque/transparent/effect cycle beats + randomize + perTypeEnabled default false);
genre-aware automation (autoPresetOnGenre, smartAutopilotEnabled, structuralScene,
genreDeckAssignment[8], genrePresetNames[8]); output w/h/display.

**Autopilot** (`Autopilot.cpp`): `processFrame` runs end-of-video mode every frame
(`:9-41`) + beat-based mode on beat crossings (`:43-107`); resolves per-clip vs
per-layer duration/action (LayerDetermined fallback), or per-type when enabled;
`advanceClip` implements PlayNext/Previous/Random/First/Last/Specific
(`:189-285`); `smartAdvanceClip` scores candidates by column-implied intensity vs
structural/energy state (`:287-368`). Also drains beat-snap pending triggers on beat
crossing (`:52-60`).

**Persistence** (JSON via juce::var): `Composition::saveToFile/loadFromFile`
(`Composition.h:247-262`) → Deck::toVar → Layer::toVar (`Layer.cpp:3-65`) →
Clip::toVar (`Clip.cpp:3-80`); BindingManager has its own preset JSON
(`BindingManager.cpp:214-306`). Serialization is **incomplete** — see FLAGGED #5.

---

## 2. DOC-VERIFY (doc-file:line → claim → code truth)

**Accurate (spot-verified):**
- CLAUDE.md:1115 OSC address-pattern list — matches all 11 code patterns
  (`OscHandler.cpp:57-175`). (But see #D1: subsystem is inert.)
- CLAUDE.md:467 "Layer autopilot fields: autopilotEnabled, autopilotEndOfVideo,
  autopilotLoops, defaultAutopilotAction, defaultAutopilotDuration" — all present
  (`Layer.h:50,147,146,143,144`).
- CLAUDE.md:1105 Beat-snap enum {Off, Beat, Bar, TwoBar, FourBar} — matches
  `Clip.h:72-80`; `Layer::processPendingTrigger` granularity matches (`Layer.h:254-286`).
- CLAUDE.md:1097-1102 trigger/CC/velocity binding claims — match `Binding.h:50-79`.
- FEATURE_INVENTORY.md:73,102,106,108 model field/enum counts — match source.
- FEATURE_INVENTORY.md:75 "32 default signals" breakdown — matches
  `SignalRegistry.cpp:4-84`.

**Discrepancies:**

D1. CLAUDE.md:11 ("OSC input (juce_osc)" listed as a capability) + FEATURE_INVENTORY.md:290
    + FEATURES.md OSC section → imply OSC input works → **`OscHandler::startListening`
    (`OscHandler.cpp:15`) is never called** in src/tests (grep empty); receiver never
    connects. OSC input is non-functional at runtime.

D2. FEATURE_INVENTORY.md:290-301 lists **10** OSC patterns (omits
    `/audiodna/effect/{name}/{param}`) → code has **11** (`OscHandler.cpp:175`). Also
    only 5/11 callbacks wired (`MainComponent.cpp:1127-1141`); bypass/solo/mute/bpm/
    macro/effect callbacks never assigned.

D3. FEATURE_INVENTORY.md:71 ("23 curve types") and :79 ("23 usable + 1 Count = 24
    enum") → **24 usable curves** (indices 0-23) + Count (`MappingTypes.h:90-120`;
    dispatcher 24 cases `CurveTransforms.h:222-251`). Undercounts by 1.
    (FEATURES.md:558 "24 curve functions" and its curve table 0-23 are correct.)

D4. FEATURE_INVENTORY.md:71,78 ("24 macros total", Scope×8) → **only the Global
    MacroBank is instantiated** (`MainComponent.h:207`); no per-clip/per-layer bank
    exists (grep empty). 8 live macros at runtime, not 24. Model supports 3 scopes;
    1 wired.

D5. CLAUDE.md:563 "Dashboard system (8 link knobs per clip/layer/global)" → same as
    D4: only the Global dashboard exists (`MainComponent.h:207`). Per-clip/per-layer
    dashboards not implemented.

D6. FEATURE_INVENTORY.md:83 lists "Chained Signal" as a signal-connect popup source;
    CLAUDE.md:566 lists per-parameter connect sources → **ChainedSignal is never
    instantiated** anywhere in repo (`ChainedSignal.h:10`; grep empty). Derived/
    chained modulation is a ghost. (FEATURES.md:704/793 describe it but do not flag
    ghost status.)

D7. CLAUDE.md:11 lists "AI mapping suggestions" as a live capability; CLAUDE.md:1135
    describes MappingSuggester → **never instantiated** (grep empty). FEATURES.md:559,666
    correctly call it a ghost/no-UI-caller; CLAUDE.md overclaims.

D8. RoutingEngine.h:12-13 "Replaces MappingEngine for v2" → **both run every frame**:
    `Renderer.cpp:198` (routingEngine_) AND `Renderer.cpp:210` (mappingEngine_). Not
    replaced — they coexist and can both write the same effect params. (FEATURE_INVENTORY.md:91
    correctly notes both exist.)

D9. (code-comment, not doc) SignalRegistry.h:23 "12 audio + 6 modulation" → actual
    initDefaults = 29 audio + 3 modulation = 32 (`SignalRegistry.cpp:4-84`).

D10. CLAUDE.md source tree omits this entire lane (src/routing, src/signal, src/binding,
    src/midi, src/sync, src/model undocumented; lists nonexistent src/keyboard).

---

## 3. HEALTH READ

**SOLID / wired:**
- Mapping pipeline + 58 sources + 24 curves (`MappingEngine.cpp`, `CurveTransforms.h`).
- RoutingEngine in render loop with dial-range/threshold/gain/falloff/invert/smooth
  (`Renderer.cpp:198`).
- SignalRegistry + AudioSignal/Oscillator/Envelope/ClipPosition (32 defaults).
- BindingManager (keyboard+MIDI, 3 target modes, toggle/momentary, abs/rel CC, JSON
  presets) + MidiHandler (input, hot-plug) + MidiOutputHandler (Launchpad pad feedback,
  change-diffed).
- Autopilot (beat + end-of-video + per-type + smart-energy) wired via Renderer.
- Deck/Layer/Clip/Composition model + beat-snap trigger queue.

**FLAGGED:**

F1. **OSC subsystem inert** — `startListening` never called (`OscHandler.cpp:15`, no
    caller). Entire OSC input feature dead at runtime.

F2. **6/11 OSC callbacks unwired** — bypass/solo/mute/bpm/macro/effectParam never
    assigned (`MainComponent.cpp:1127-1141`). No-op even if listening started.

F3. **ChainedSignal — ghost** (`ChainedSignal.h:10`): never instantiated. Chained/
    derived modulation unreachable. Also `getValue` ignores snapshot and reads
    cached values (`ChainedSignal.cpp:5-31`); if evaluated before its carrier/
    modulator in insertion-order `evaluateAll`, gets stale prior-frame values.

F4. **MappingSuggester — ghost** (`MappingSuggester.cpp`): never instantiated; no UI
    or API caller. ~266 LOC fully implemented but unreachable.

F5. **Serialization is lossy** across all three model entities:
    - `Clip::toVar` (`Clip.cpp:3-80`) drops: clipOpacity, per-clip transform
      (pos/scale/rotation/anchor), blendOverride, alphaType, channel R/G/B/A, MilkDrop
      preset playlist, EffectSlot.dryWet, clipWidth/clipHeight, and beatDivision/
      videoBeats when there are no sequenceFiles.
    - `Layer::toVar` (`Layer.cpp:3-65`) drops: per-layer transform, FeedbackConfig
      (all feedback), transitionMode/transitionBlendMode, layerWidth/Height, autoSize,
      autopilotLoops, autopilotEndOfVideo.
    - `Composition::toVar` (`Composition.h:162-198`) drops: masterSpeed, compOpacity,
      crossfader (all), comp transform, composition autopilot (direction/mode/loops/
      loop/masterLayer), PerTypeAutopilotConfig, all genre-aware fields.
    Save→load loses substantial state. (FEATURE_INVENTORY.md:122 notes the Composition
    subset only.)

F6. **Only Global MacroBank instantiated** (`MainComponent.h:207`) — per-clip/per-
    layer dashboards absent despite 3-scope enum.

F7. **LinkSync no-op in default build** — `AUDIODNA_BUILD_LINK` OFF
    (`CMakeLists.txt:35`); all methods compile out (documented optional, so health
    note not a discrepancy).

F8. **Dual mapping engines write the same params** — MappingEngine resets targeted
    params to 0 then accumulates (`MappingEngine.cpp:162`), destroying manual/base
    values; RoutingEngine writes the same params via ParamWriter. Both run each frame
    (D8); interaction order undefined between the two subsystems.

**Minor:** relative-CC redundant identical if/else (`BindingManager.cpp:179-182`);
`getCachedValue` O(n) linear scan per lookup (`SignalRegistry.cpp:162`, fine at ~32
signals).
