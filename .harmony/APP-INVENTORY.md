# APP-INVENTORY — Audio-DNA (RealTimeAudio)

Living inventory — same-wave-update rule: any change to a surface or user-visible
function updates its row in that wave. Created 2026-07-16 (7-lane re-norm audit,
HEAD `9139dd4`). Every claim traces to a lane census in
`.audit/renorm-2026-07-16/` (`file:line` carried where cheap).

---

## §1 — App in one line

Audio-DNA is a C++20 / JUCE / OpenGL 4.1 desktop VJ application: it analyzes live
or file audio in real time (14-stage pipeline → 30 features) and drives a
Resolume-style deck/layer/clip compositor of 135 GLSL effects and 108 procedural
GPU sources, with audio→parameter mapping, signal routing, MIDI/keyboard binding,
autopilot, video/PNG recording, a fullscreen output window, and an always-on REST
control API.

---

## §2 — Counts row (drift detector)

3 windows · ~28 panel classes (18 live in v2) · 22 tabs · ~8 overlay/popup surfaces
(2 live, 1 orphaned, 3 popup pickers) + ~15 FileChoosers + 1 AlertWindow · 9-menu
menu bar (~45 items, no-op DBG stubs removed Wave 0; Output→Syphon toggle added Wave 1-A) · **135 effects** / 11 categories / 333 params ·
**15 transitions** (+1 deck transition) · **243 embedded shaders** · **108 sources**
/ 19 categories / 759 params (108 GUI-selectable) · **30 audio features** / 14-stage
pipeline · **58 mapping sources** / 24 curves · **32 default signals** · 8 live macros
(Global bank only) · **22 REST endpoints** (all functional) ·
**11 OSC patterns** (subsystem LIVE — port 8000, 11/11 wired, Wave 1-B 2026-07-17) · 19 binding actions · 6 feedback presets ·
**114 unit tests** (all PASS; +4 Wave 1-C persistence roundtrip/back-compat, +1 Wave 1-D waveform-snapshot seqlock regression).

---

## §3 — Surfaces (one row per window / panel / tab / overlay; FUNCTION-level)

Source: lane-5-ui-surfaces.md. "Live?" = reachable + operable in the shipping v2 UI.

### Windows

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| Main window (`Main.cpp:40`) | App launch; maximized to primary display, resizable 1280×720–3840×2160 | Hosts all main-window panels; global keyboard shortcuts; Finder file-drop target | yes |
| Native menu bar (`MenuBarModel.cpp`) | Top of screen (macOS) | 9 menus, ~45 items; no-op DBG stubs removed (Wave 0); Output menu gains a real "Syphon Output" toggle (Wave 1-A, ticks live state). Undo/Redo LIVE + dynamic (Undo v1 steps 1-3, 2026-07-19/20): "Undo <desc>"/"Redo <desc>" text, enable state tracks stacks, rebuilds via onHistoryChanged | yes |
| OutputWindow (`src/ui/OutputWindow.h:69`) | Output menu → Fullscreen:display / TopBar output combo / Cmd+F | Borderless always-on-top render on a chosen display; Escape closes; no on-surface controls | yes |
| PreferencesDialog (`PreferencesDialog.h:8`) | Audio-DNA menu → Preferences / About; modal, 3 tabs | See Prefs tab rows below | yes |

### Main-window panels (v2)

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| TopBar (`TopBar.h:12`) | Always visible (top, 34px) | Audio-source combo (Mic/File); input-gain slider; **Play/Pause/Stop (WIRED Wave 1-D — global transport over the active deck's layers' active clips; Stop = pause + rewind to in-point; TopBar.cpp:31-33 → MainComponent.cpp:533)**; Tap-tempo; Resync; manual-BPM toggle + BPM edit; 5 multiplier buttons (/4 /2 x1 x2 x4); Quantize combo; Fade slider; Master slider; Output-display combo; beat wheel + bar/phrase + FPS/DSP readouts | yes |
| SignalBar (`SignalBar.h:15`) | Always visible (3 size modes) | `[+]` add-signal popup; shrink/grow buttons; N SignalStrip children (click = select for Signal inspector; display-only meter) | yes |
| DeckView (`DeckView.h:15`) | Main content grid (scrollable) | Column-trigger buttons (click = trigger column); deck-tab buttons (switch deck); hosts LayerStrip + ClipCell | yes |
| LayerStrip (`LayerStrip.h:23`) | Per-layer header in DeckView | Clear/Bypass/Solo; transport `< || > >|`; Speed/Keying/Opacity sliders; Blend+keying combo (13 keying + ~55 mix modes); Fade-speed slider + transition-mode combo; name-click select; clip-bar drag = scrub. Right-click: none | yes |
| ClipCell (`ClipCell.h:11`) | Per layer×column cell | Thumbnail click = trigger/retrigger; name-bar click = select (Cmd/Shift = multi-select); name-bar drag = move clip; drop targets: files, `fx:`, `source:`, `clip:`, `milkdrop:`, `milkdrop_playlist:`. Right-click: no-op | yes |
| PreviewPanel (`PreviewPanel.h:12`) | Bottom row | Preview / Output tab buttons (labels only — same GL host); GL preview | yes |
| WaveformDisplay (`WaveformDisplay.h:14`) | Under Preview | Scrolling waveform readout — no controls | yes (readout) |
| TimingWindow (`TimingWindow.h:8`) | Bottom row | BPM / Routing / Oscillators tab buttons — **all 3 tabs are EMPTY placeholders** | no (empty) |
| InspectorPanel (`InspectorPanel.h:23`) | Bottom row | 4 tab buttons (Clip/Layer/Composition/Signal); Pin button; auto-switch on selection | yes |
| BrowserPanel (`BrowserPanel.h:16`) | Bottom row | 6 tab buttons (Files/FX/Sources/Comp-Decks/Record/MilkDrop) | yes |
| Row1 v1 toolbar + 10 preset slots (`MainComponent.cpp:1350`) | Shown alongside v2 chrome | Open Image / Image Folder; Beats-per-image combo; Save/Load/FX-Save/Deck-Save/Deck-Load; 10 numbered preset slots (load button + assign combo) — duplicates v2 TopBar controls | yes (v1/v2 dup) |

### Inspector tabs

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| ClipInspector (`ClipInspector.h:25`) | Inspector → Clip tab (auto on clip select) | Dashboard (8 knobs + 8 source pickers); Transport (mode/loop/trigger/speed/reverse/duration); conditional Images-per-sec OR Beat-Division+Content-Beats; 8 cuepoint jump + 8 Set; Autopilot (action/duration/beat-snap); Source Parameters (UniversalParamControls); Video (opacity/W-H/blend/alpha/RGBA); Transform (pos/scale/rotation/anchor); Effects (EffectStackView); interactive timeline (in/out/playhead drag) | yes |
| LayerInspector (`LayerInspector.h:28`) | Inspector → Layer tab | Editable name; Dashboard; Autopilot (4 dir + trigger-mode + beat-count + loops); Layer (master/persistent/ignore-column); Video (blend/opacity/W-H/auto-size); Transition (blend ~55 + duration); Keying (Transparent only, 13 modes); Dry/Wet (FX-Only only); 3D controls (ThreeD only); Transform (5 UPCs); Feedback (enable + preset + 7 sliders); Layer Effects (EffectStackView) | yes |
| CompositionInspector (`CompositionInspector.h:23`) | Inspector → Composition tab | Dashboard; Autopilot (4 dir + duration + clip-loops + loop + master-layer); Per-Type Autopilot (enable + cycle sliders + randomize); Composition master/speed; Video opacity; Transform (5 UPCs); Global Effects (EffectStackView); Output resolution combo. Collapse triangles + P. buttons decorative | yes |
| SignalInspector (`SignalInspector.h:15`) | Inspector → Signal tab | Audio: threshold/gain/falloff. Oscillator: wave-shape (5) + beat-duration (6) + amplitude + phase. Envelope: curve-type (3) + beat-duration (5) + amplitude + phase + looping/one-shot toggles + **paint-only curve editor (NOT draggable)** | yes |

### Browser tabs

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| Files (`FilesBrowser.cpp`) | Browser → Files | Up button; path bar; search; Grid/List toggle; file grid click/dbl-click/drag (`files:`); right-click = toggle favorite; directory navigate | yes |
| FX (`FXBrowser.cpp`) | Browser → FX | Search; 11 collapsible category headers; effect rows click / Cmd-Shift multi-select; drag `fx:name,name` to cell/stack | yes |
| Sources (`SourcesBrowser.cpp`) | Browser → Sources | Search; 19 category headers; **109 hand-listed source rows** (registry NOT used, but the 6 previously-absent registered sources added Wave 0; Simulation/Routing/MilkDrop headers now populated); click/multi-select; drag `source:id,id` | yes |
| Comp/Decks (`CompDecksBrowser.cpp`) | Browser → Comp/Decks | Save-Composition + Save-Deck; 2 collapsible sections; entry click = load; right-click = delete file | yes |
| Record (`RecordPanel.cpp`) | Browser → Record | Record/Stop/Play/Save/Load/Output-Folder buttons; Format combo (JSON); status + event-count labels (**Play fires nothing — playback dead**) | partial |
| MilkDrop (`MilkDropBrowser.cpp`) | Browser → MilkDrop | 4 sub-tabs (Curated/Favorites/Recent/All); search; Prev/Next/Random/Lock nav; 3 play-modes (Jukebox/VJ-Clip/Playlist); Jukebox play + pool/mode/timing combos + blend; Playlist cycle/timing + blend; preset rows click/multi-select/drag; right-click = favorite | yes (needs libprojectM) |

### Prefs tabs (`PreferencesDialog.cpp`)

| Tab | State | Functions |
|---|---|---|
| General | wired | Show-Tooltips — genuinely wired Wave 1-D: toggle → MainComponent::setTooltipsEnabled → creates/destroys the shared TooltipWindow (in-session only; no settings store to persist). Confirm-on-quit toggle removed Wave 0 |
| Video | wired | MilkDrop preset-dir edit + Browse (FPS/Render-Res inert combos + Audio tab removed Wave 0) |
| About | display | version + credits |

(8→3 tabs Wave 0: Audio/MIDI/Recording/Defaults/Feedback removed as empty/inert.)

### Overlays / popups

| Surface | Reach / trigger | Functions | Live? |
|---|---|---|---|
| BindingOverlay (`BindingOverlay.cpp`) | Shortcuts → Edit Keyboard / Shift+Cmd+K | Fullscreen; click target then press key to bind; Escape exits | yes |
| MidiLearnOverlay (`MidiLearnOverlay.cpp`) | Shortcuts → Edit MIDI / Shift+Cmd+M | Same workflow, listens for MIDI note/CC | yes |
| MappingEditor (`MappingEditor.cpp`) | Only from EffectsRackPanel map buttons | Source/Curve/In-Out/Smoothing/Enabled/Randomize/Delete — **UNREACHABLE in v2 (rack hidden)** | no |
| UniversalParamControl source picker (`UniversalParamControl.cpp:311`) | Connect triangle on any inspector param | Cascading popup: Manual / Audio signals / BPM-sync / Oscillators / Envelopes / Clip Position / Timeline / Macros; right-click = reset; [-]/[+]; Invert + Range | yes |
| MacroPanel source picker (`MacroPanel.cpp:109`) | Dashboard knob source button | Popup: Manual + Signals submenu | yes |
| SignalBar add-signal menu (`SignalBar.cpp:179`) | `[+]` button | Popup of hidden signals | yes |
| FileChoosers (~15) | Various (open/save image, preset, deck, bindings, layout, record, MilkDrop dir) | Native OS dialogs | yes |
| AlertWindow | ISF import result (`:2418`) | OK (ISF import registers a non-rendering phantom — see §8) | partial |

---

## §4 — Data lanes + writer boundary (4-thread model)

Source: lane-1-audio-analysis.md §2-4.

| Thread | Role | Writes | Lock-free handoff out |
|---|---|---|---|
| **Audio RT callback** | `AudioCallback` reads output channels, mono-downmixes (gain 1/numCh) | RingBuffer (16384 floats, ~341ms) | `RingBuffer<float>` SPSC (power-of-two, alignas(64); overflow drops silently) |
| **Analysis thread** | `AnalysisThread` pulls 512-hop, maintains 2048-window, runs 14-stage pipeline | FeatureSnapshot (40 fields), waveform snapshot (2048f), PCM double-buffer (512, for projectM) | `FeatureBus` triple-buffer (1 atomic packs write/latest/read + new-data flag, wait-free CAS); waveform = **seqlock** (Wave 1-D — reader retries on version change, strictly torn-read-free; replaced the count-release + plain-memcpy that could tear); PCM = index-swap double-buffer |
| **Message (JUCE) thread** | UI events, timers, MIDI dispatch (`callAsync`), REST callbacks (`onTrigger*`), menu commands | Composition/Deck/Layer/Clip model; manual effect params; bindings; presets | JUCE message queue |
| **Render / GL thread** | `Renderer::renderOpenGL` reads latest FeatureSnapshot, runs `routingEngine.processFrame` (`Renderer.cpp:198`) + `mappingEngine.processFrame` (`:210`) + `signalRegistry.evaluateAll`, composites, VideoRecorder GL readback | Effect params (from mapping/routing); GL/FBO state; ClipPositionSignal playhead | — (consumer of FeatureBus) |

**Writer-boundary hazards:**
- **Effect params have two+ writers.** `MappingEngine::processFrame` first resets every
  targeted param to 0 then accumulates all enabled mappings (`MappingEngine.cpp:162`) —
  destroying any manual/base value. `RoutingEngine` writes the same params via
  `ParamWriter` the same frame. Both run every frame; interaction order between the two
  subsystems is undefined (lane-4 D8/F8).
- **Model** is message-thread-owned; **FeatureSnapshot** is analysis-write / GL-read.
- Aubio ctors (`new_aubio_*`) are used without null-guards (lane-1 RISK).

---

## §5 — Write surface

### REST API — production server (`src/api/ApiServer.cpp`, port 7070, always-on, CORS)

Source: lane-6-io-api.md. 22 endpoints; all functional (`/api/set_bpm` wired Wave 0).

| # | Method | Path | Action |
|---|---|---|---|
| 1 | GET | /api/health | ok, version 0.1.0, fps, effects_count |
| 2 | GET | /api/status | fps, frameTime, masterLevel, activeDeck, bpm/phase/genre/energy |
| 3 | GET | /api/composition | full deck→layer→clip tree |
| 4 | POST | /api/trigger_clip | onTriggerClip(layer, column) |
| 5 | POST | /api/trigger_column | onTriggerColumn(column) |
| 6 | POST | /api/set_param | set clip-effect or global-chain param |
| 7 | POST | /api/set_layer_opacity | active-deck layer opacity |
| 8 | POST | /api/switch_deck | onSwitchDeck(deck) |
| 9 | POST | /api/snapshot | takeSnapshot() (blocks), returns path |
| 10 | GET | /api/bpm | bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount |
| 11 | POST | /api/set_bpm | manual BPM override — setManualMode+setManualBPM via message thread (wired Wave 0) |
| 12 | GET | /api/features | full FeatureSnapshot dump |
| 13 | POST | /api/inject_features | write FeatureBus (test/automation) |
| 14 | POST | /api/load_image | loadImage() + 100ms GL sleep |
| 15 | POST | /api/load_source | setActiveSource() |
| 16 | POST | /api/set_effect | enable/disable + params on global-chain effect |
| 17 | GET | /api/effects | list global-chain effects |
| 18 | GET | /api/sources | list registered source ids |
| 19 | POST | /api/render_frame | captureFrame() to path |
| 20 | POST | /api/reset | clear image + source + disable all effects |
| 21 | POST | /api/set_effect_chain | batch disable-all + enable/configure requested |
| 22 | GET | /api/state | fps, frame_time, master_level, effects[], decks |

Eyes TEST server (`src/test/TestServer.cpp`, port 8080, 17 endpoints) is gated by
`AUDIODNA_BUILD_TEST_SERVER=ON` + `--test-mode` (OFF by default) — separate surface.

### OSC input (`src/osc/OscHandler.cpp`) — 11 patterns, subsystem **LIVE** (Wave 1-B, 2026-07-17)

`startListening(8000)` is called unconditionally at startup (`MainComponent.cpp:1207-1211`,
like ApiServer); receiver binds UDP port 8000 (de-facto OSC receive default). All 11/11
callbacks are now wired (`MainComponent.cpp:1132-1204`), each routing through the same
handler as the equivalent REST/UI/MIDI path (clip/deck/snapshot → same as REST; bpm →
manual-override tracker; layer opacity/bypass/solo/mute → active-deck layer fields; macro →
global dashboard-link bank; effect param → global effect-chain). Delivery is on the message
thread (`MessageLoopCallback`). Patterns:
`/audiodna/clip/{layer}/{column}`, `/layer/{n}/opacity|bypass|solo|mute`, `/deck/{n}`,
`/master`, `/bpm`, `/snapshot`, `/macro/{n}`, `/effect/{name}/{param}`.
Port is hardcoded (no preferences UI configures it yet — matches absence of a settings store).

### Persistence (JSON via juce::var) — **COMPLETE** for model entities (Wave 1-C, 2026-07-17; was LOSSY, lane-4 F5)

Clip/Layer/Composition now round-trip every persistent-intent field. Additive schema —
old presets load unchanged (missing keys fall back to struct defaults via `hasProperty`
guards). Proof: `tests/test_composition.cpp` — per-entity full-field roundtrip tests +
back-compat test (old-format var → struct defaults, no crash). The column below is now
what *genuinely remains runtime-only* (deliberately excluded), not a loss.

| Entity | Path | Runtime-only (deliberately NOT serialized) |
|---|---|---|
| Clip | `Clip::toVar/fromVar` (Clip.cpp) | playing, playheadPosition, beatsPlayed, hasBeenTriggered, thumbnail (GL/UI); presetPlaylistIndex + presetBeatsPlayed (mutable playlist cursors) |
| Layer | `Layer::toVar/fromVar` (Layer.cpp) | activeClipColumn, previousClipColumn, crossfadeProgress, pendingTriggerColumn |
| Composition | `Composition::toVar/fromVar` (Composition.h) | filePath (set on load), nextDeckId_ (runtime id counter) |
| Bindings | `BindingManager` (BindingManager.cpp:214-306) | separate preset JSON (keyboard/MIDI bindings) — complete |

---

## §6 — Cross-cutting families

Source: lanes 2 + 4.

- **Mapping** — `MappingEngine`: 58 mapping sources (1:1 to FeatureSnapshot fields) × 24
  curves (5 classic + 19 P24 easings) → normalize → curve → scale → per-mapping EMA →
  accumulate into target param. Multiple mappings on one param sum.
- **Signal routing** — `SignalRegistry` (32 default signals: 29 audio + 3 modulation) +
  `RoutingEngine` (dial-range → threshold → gain → falloff → invert → EMA → ParamWriter),
  wired at `Renderer.cpp:198`. `MacroBank` "Dashboard Links": **only the Global bank is
  instantiated → 8 live macros, not 24** (`MainComponent.h:207`). Signals: AudioSignal,
  OscillatorSignal (5 shapes, BPM-locked), EnvelopeSignal (control-point, BPM-locked),
  ClipPositionSignal. (ChainedSignal removed Wave 0 — see §8.)
- **Binding / MIDI** — `BindingManager`: **19 actions**, InputType Keyboard/MidiNote/MidiCC,
  3 target modes (ByPosition/ThisItem/Selected), Toggle/Momentary, Absolute/Relative CC,
  MIDI-learn capture, JSON presets. `MidiHandler` (all inputs, hot-plug).
  `MidiOutputHandler` (Launchpad/APC pad feedback, 5 pad states, change-diffed).
- **Autopilot** (`Autopilot.cpp`) — end-of-video mode (every frame) + beat-based mode (on
  beat crossings) + per-type config + smart-energy scoring (structural/energy state).
- **Feedback** — per-layer `FeedbackProcessor` (FBO ping-pong, `feedback_blend` shader) +
  `CompositorEngine::updateFeedbackBuffer` accumulator passthrough. **6 presets**: Zoom In,
  Spiral, Drift, Kaleidoscope, Echo, Stretch (`FeedbackProcessor.cpp:131-142`).
- **Transitions** — 15 clip-to-clip (Dissolve default … ToBlack), enum-mapped +
  compiled + dedicated `transitionFBO_`; plus 1 separate `deck_transition` (cross-deck A/B).
- **Recording / snapshot** — `VideoRecorder` (REAL: FFmpeg H.264/ProRes/MJPEG,
  triple-buffered GL readback, wired to Output menu, video-only no audio); PNG snapshot
  (REAL, via REST/OSC/menu); `SessionRecorder` (**PARTIAL** — captures clip triggers only,
  playback dead).
- **Genre / energy intelligence** — `GenreDetector` (8 genres + 3-band energy, ~2s EMA +
  ~3s hysteresis internally); drives auto-preset/deck switch + structural scene triggering.
  (GenreSmoothing + MappingSuggester removed Wave 0 — see §8.)

---

## §7 — Small print

Source: lane-5 §3.

- **Keyboard shortcuts** (`MainComponent::keyPressed :1591`): Shift+Cmd+I inspector,
  Shift+Cmd+K keyboard-bind, Shift+Cmd+M MIDI-learn, Escape close-output, Cmd+Z /
  Cmd+Shift+Z undo/redo (**LIVE for clip-grid edits** — Undo v1 steps 1-3: all drops,
  replace/lock/clear, drag move/swap; structural ops [layers/decks/columns/effects/
  triggers] pending steps 4-9), Cmd+S save preset, Cmd+F
  toggle fullscreen output, Cmd+O load preset. Non-Cmd keys → BindingManager; key-up →
  momentary bindings.
- **Tooltips**: `juce::TooltipWindow` (600ms); coverage sparse (TopBar, ClipInspector,
  LayerInspector only); toggle in Preferences → General now actually enables/disables
  it (Wave 1-D — creates/destroys the TooltipWindow; in-session only, no persistence).
- **Drag-and-drop matrix**: Finder files → main window (audio→engine, image→preview) and
  → ClipCell; FXBrowser `fx:` → ClipCell / EffectStackView / Clip+Layer inspectors;
  SourcesBrowser `source:` → ClipCell; FilesBrowser `files:` → ClipCell; MilkDropBrowser
  `milkdrop:` / `milkdrop_playlist:` → ClipCell; ClipCell `clip:` → ClipCell (move/swap).
  DragAndDropTargets: ClipCell, ClipInspector, LayerInspector, EffectStackView.
- **Theme**: single `AudioDNALookAndFeel` dark theme (cyan/magenta); LayerStrip adds 7
  inline LAF subclasses.

---

## §8 — FLAGGED (consolidated dead / ghost / stub / orphan — all lanes)

| Item | Status | Evidence (file:line) |
|---|---|---|
| Syphon output | WIRED 2026-07-17 (Wave 1-A) — init() on GL-context create (getRawContext → NSOpenGLContext), final composited frame blit→publishTexture() once/frame gated on enabled+initialized; Output→"Syphon Output" toggle (default OFF each boot, no persistence). Runtime no-op unless built `-DAUDIODNA_BUILD_SYPHON=ON` + Syphon.framework installed | Renderer.cpp newOpenGLContextCreated/renderOpenGL/publishSyphonFrame; MenuBarModel.cpp:117; MainComponent.cpp kOutputSyphon handler |
| Syphon input | REMOVED 2026-07-17 (Wave 0) — SyphonInput .mm/.h deleted (was orphaned, 0 refs) | — |
| Spout output | REMOVED 2026-07-17 (Wave 0) — SpoutOutput.h deleted (was header-only no-op) | — |
| NDI output / input | REMOVED 2026-07-17 (Wave 0) — NdiOutput.h + NdiInput.h deleted (were stubs) | — |
| Undo / redo | PARTIAL-LIVE 2026-07-19/20 (Undo v1 steps 1-3, commits 7c8d286/7921572/daa9361) — SetClipCmd/ToggleClipLockCmd/SwapClipsCmd + CompositeCommand wrap ALL clip-cell edit sites; Cmd+Z + dynamic menu live; value-copy snapshots, coordinate-addressed; GL fence validated; media reconnect file-compare (MediaReconnect.h). Structural ops (layer/deck/column/effect-stack/trigger) = spec steps 4-9, queued | src/core/ClipCommands.h; UndoService.h/.cpp; CompositeCommand.h; MediaReconnect.h; .harmony/undo-v1-ledger.md |
| Session playback | DEAD — `advancePlayback()` never called; capture = clip triggers only (6/7 record* unused) | SessionRecorder.cpp; MainComponent.cpp:2472; RecordPanel.cpp:38 |
| OSC subsystem | LIVE 2026-07-17 (Wave 1-B) — `startListening(8000)` called at startup; 11/11 callbacks wired (port hardcoded, no prefs UI) | OscHandler.cpp:15; MainComponent.cpp:1132-1211 |
| ISF import | PHANTOM — converted GLSL never compiled/queued; effect registers + shows but never renders; "Import Successful" dialog misleads | MainComponent.cpp:2426-2454 |
| ISFShaderLoader::registerISFEffect | REMOVED 2026-07-17 (Wave 0) — dead no-op stub deleted (registerDynamic is the real path, kept) | — |
| Shader hot-reload | INERT — all shipped shaders compiled from embedded strings; `reloadAll()` skips file-less programs | ShaderManager.cpp:115-116 |
| shaders/ disk files (5) | REMOVED 2026-07-17 (Wave 0) — hue_shift/rgb_split/ripple/vignette/passthrough deleted (shipped shaders are embedded strings) | — |
| UniformBridge / applyDemoMappings | REMOVED 2026-07-17 (Wave 0) — deleted (superseded by MappingEngine); Renderer.h member/include removed | — |
| MappingSuggester | REMOVED 2026-07-17 (Wave 0) — .cpp/.h deleted (was ghost, 0 callers) | — |
| ChainedSignal | REMOVED 2026-07-17 (Wave 0) — .cpp/.h deleted + SignalRegistry dynamic_cast wiring removed | — |
| GenreSmoothing | REMOVED 2026-07-17 (Wave 0) — GenreSmoothing.h deleted (was dead, self-refs only) | — |
| OneEuroFilter | REMOVED 2026-07-17 (Wave 0) — class removed from Smoother.h + 5 orphaned tests (EMA Smoother kept) | — |
| Orphaned tuning setters | REMOVED 2026-07-17 (Wave 0) — OnsetDetector setThreshold/setSilence/setMinInterOnsetMs + BPMTracker setThreshold/setSilence/setPhraseBars deleted (phrase length stays default 8) | — |
| 6 sources missing from SourcesBrowser | RESOLVED 2026-07-17 (Wave 0) — all 6 added to the GUI list (strange_attractor/gravity_well/fluid_dynamics under Simulation; text_animator under Text; layer_router under Routing; projectm_visualizer under new MilkDrop header) | SourcesBrowser.cpp |
| source_fluid_display | REMOVED 2026-07-17 (Wave 0) — orphan shader string + compile call deleted (no source referenced it) | — |
| EffectsRackPanel + MappingEditor | HIDDEN/UNREACHABLE — rack `setVisible(false)` only; MappingEditor only opens from the rack | MainComponent.cpp:1401; EffectsRackPanel.cpp:498 |
| AudioReadoutPanel + SpectrumDisplay | HIDDEN — built, permanently `setVisible(false)` in v2 | MainComponent.cpp:1399-1400 |
| ProgrammingMode | REMOVED 2026-07-17 (Wave 0) — .cpp/.h + MainComponent refs + View→Programming Mode menu item deleted | — |
| ~35 menu items | RESOLVED 2026-07-17 (Wave 0) — 37 no-op menu items removed from the menus (enum values kept as fullscreen-range sentinel); Undo/Redo left for Wave 2 | — |
| 4/8 Prefs tabs empty + audio/video combos inert | RESOLVED 2026-07-17 (Wave 0) — Prefs collapsed 8→3 tabs (General/Video/About); MIDI/Recording/Defaults/Feedback + Audio tab + inert combos + Confirm-on-quit removed | — |
| TimingWindow 3 tabs | EMPTY — BPM/Routing/Oscillators tabs paint only the tab name | TimingWindow.cpp:51-65 |
| TopBar transport buttons | WIRED 2026-07-17 (Wave 1-D) — Play/Pause/Stop now drive global transport over the active deck's layers' active clips (Stop = pause + rewind to in-point) | TopBar.cpp:31-33; MainComponent.cpp:533 |
| Layer → Clear Clips | BUG FIXED 2026-07-17 (Wave 1-D) — handler body was byte-identical to Deck→Clear Clips and wiped the ENTIRE deck; now clears only the SELECTED layer (deckView_ getSelectedLayerIndex) | MainComponent.cpp:3080 |
| /api/set_bpm | WIRED 2026-07-17 (Wave 0) — now drives the TopBar manual-BPM override path (onSetBpm → setManualMode+setManualBPM, marshalled to message thread) | ApiServer.cpp:510-527 |
| Ableton Link | NO-OP in default build — `AUDIODNA_BUILD_LINK` OFF → every LinkSync method compiles out | CMakeLists.txt:35; LinkSync.cpp |
| FeatureSnapshot::clear() genre default | FIXED 2026-07-17 (Wave 0) — clear() now restores detectedGenre=6/energyState=1 struct defaults (+ unit test) | FeatureSnapshot.h:81-91 |
| 48kHz hardcode | RUNTIME GUARD ADDED 2026-07-17 (Wave 1-D) — startup now warns (cerr + one-shot AlertWindow) when the device SR != 48000; SR-independence still deferred (no resampling; `kSampleRate=48000` + K-weighting coeffs + all freq math still assume 48k) | AnalysisThread.h:46; LoudnessAnalyzer.cpp:13-29; MainComponent.cpp:370 (guard); AudioEngine.cpp getCurrentSampleRate |
| Dual mapping engines | RISK — MappingEngine (resets-then-accumulates) and RoutingEngine both write the same params every frame; order undefined | Renderer.cpp:198, :210 |
| Only Global MacroBank | GAP — 3-scope enum but only Global instantiated → 8 live macros, not 24 | MainComponent.h:207 |
| OutputWindow shader table | DUP/LAG — OutputRenderer keeps a separate ~80-shader compile table that can lag the main Renderer's 275 | OutputWindow.cpp:157,164-210 |
| Waveform snapshot torn read | FIXED 2026-07-17 (Wave 1-D) — replaced count-release + plain-memcpy with a seqlock (reader retries on version change → strictly torn-read-free; a plain double buffer was tried first but the torn-read stress test showed it still tears when the writer laps the reader). Threaded regression test added | AnalysisThread.cpp:337 (writer), :365 (reader); tests/test_waveform_snapshot.cpp |
