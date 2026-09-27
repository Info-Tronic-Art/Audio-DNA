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
/ 19 categories / 681 params (108 GUI-selectable; 759 -> 681 s-rta-0927 source-defects: 78 controls no shader read removed) · **30 audio features** / 14-stage
pipeline · **58 mapping sources** / 24 curves · **32 default signals** · 8 live macros
(Global bank only) · **41 registered REST routes** (all functional; 27 core control + 7 `/api/perf/*` + `POST /api/audio/source` + 6 `/api/routine/*` [s-rta-0926 routines slice 1]; counted from `src/api/ApiServer.cpp`, s-rta-0926) ·
**14 OSC patterns** (subsystem LIVE — port 8000, 14/14 wired, Wave 1-B 2026-07-17 + `/audiodna/routine/{slot}` s-rta-0926; `/audiodna/signal` added s-rta-0925 mastersignal Step 1; `/audiodna/resync` added s-rta-0925 resync) · 21 binding actions (TriggerRoutine appended s-rta-0926) · 6 feedback presets ·
**689 unit tests / 70 Catch2 targets** (ctest -N enumerates 689 registered tests on lane/routine-display-0927 at its C7 commit, s-rta-0927; targets = `catch_discover_tests` calls in `tests/CMakeLists.txt`, s-rta-0927; the earlier "565 / 53" was s-rta-0926; not executed by this pass, re-run `ctest` before inheriting a pass/fail claim; historical delta chain below predates the recorder lanes and is not reconciled — re-derive, do not inherit: 114 → 176 across the Undo-v1 lane; 176 → 182 on 2026-07-30 AM: +1 clear-composite, +4 ThumbnailCache, +1 stale-mtime-race guard; 182 → 188 on 2026-07-30 PM2: +4 test_autopilot.cpp [FIRST autopilot coverage] + 2 test_renderer_source_confinement; 188 → 189 on 2026-08-02: +1 syphon_check_negative; 189 → 539 at s-rta-0925 close, HANDOFF.md; 539 → 565 across the s-rta-0926 wave incl. routines slice 1 [+14: test_routine.cpp cases 1-6/13, test_routine_engine.cpp cases 7-12/15]).

**2026-07-30 PM2 SURFACE DELTA (13-item queue session — reconcile rows below when next doing a full §3 pass):** LayerStrip = NEW FX-drop-target (layer-scope stack, one undo entry) · Clip menu items now selection-gated (grayed w/o selection) · Cmd+X = second shortcut on Clip>Clear · mixed image+video Finder drop lands BOTH (one composite undo) · MilkDrop group-header drag = whole-section playlist drop (any mode); the 3 Playlist-mode controls (cycle/timing/blend) are now REAL (were decorative) · click-on-playing-cell RESTARTS video/imageseq from in-point (sources still no-op — Boris ruling pending) · genre auto-switch now reconciles preview via handleDeckSwitch · autopilot advances off Source/Image cells (was frozen) · ApiServer: 6 endpoints marshalled to message thread, ok:true-always semantics; sanitizer build variants exist (ADNA_SANITIZE); §8 candidates CLEARED this session: dead startDrag() decl removed, ReinspectTarget path removed (B8).

**2026-08-02 SURFACE DELTA (Syphon+concurrency session — reconcile rows below on the next full §3 pass):** SYPHON OUTPUT NOW REAL (was silent no-op since May 18): SDK vendored via pinned FetchContent (SHA 71351d4b, BSD-3 attribution in THIRD_PARTY_LICENSES.md), AUDIODNA_BUILD_SYPHON default ON, server announces at boot (eager — Boris taste ruling pending), publishing gated by runtime toggle (menu + NEW REST: GET /api/syphon, POST /api/set_syphon — endpoint count 22→24 registered) · NEW tools target `syphon-check` (headless server-announce assert; +1 ctest) · API HARDENING: bind default 127.0.0.1 (AUDIODNA_API_BIND env restores wide), /api/inject_features UNREGISTERED in production (test-mode ctor flag; enum inputs clamped) · FeatureBus triple-buffer CAS → acq_rel (single-reader race closed; multi-reader redesign RATIFIED, .harmony/specs/featurebus-thread-safety-design.md — S2 seqlock queued) · OutputWindow: duplicate cross-thread mappingEngine_.processFrame call DELETED (KNOWN LIMITATION: SignalBar-expanded hides preview → mappings freeze on output until reattach; R10) · EffectChain.h boundary comment now states the two-GL-thread reality.

---

## §3 — Surfaces (one row per window / panel / tab / overlay; FUNCTION-level)

Source: lane-5-ui-surfaces.md. "Live?" = reachable + operable in the shipping v2 UI.

### Windows

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| Main window (`Main.cpp:40`) | App launch; maximized to primary display, resizable 1280×720–3840×2160 | Hosts all main-window panels; global keyboard shortcuts; Finder file-drop target | yes |
| Native menu bar (`MenuBarModel.cpp`) | Top of screen (macOS) | 9 menus, ~45 items; no-op DBG stubs removed (Wave 0); Output menu gains a real "Syphon Output" toggle (Wave 1-A, ticks live state); Output menu top = one tickable "Display N (WxH[, main])" item per connected display + "All Outputs Off" (s-rta-0927 outputs-c2, `OutputManager::populateMenu`). Undo/Redo LIVE + dynamic (Undo v1 COMPLETE steps 1-9, 2026-07-19→25): "Undo <desc>"/"Redo <desc>" text, enable state tracks stacks, rebuilds via onHistoryChanged | yes |
| OutputWindow (`src/ui/OutputWindow.h`) | Output menu → tick "Display N (…)" / TopBar "Outputs" button (same list) / Cmd+F (main display) — one window per display, any number (`OutputManager`, s-rta-0927 outputs-c2) | Borderless window at NORMAL level on a chosen display, never key (`windowIgnoresKeyPresses`); presents the composition canvas (shared IOSurface frames, letterboxed) -- s-rta-0927 outputs-c1; All Outputs Off / Cmd+Shift+Esc closes every output, Cmd+` raises the app window, plain Escape no longer closes (outputs-c2); no on-surface controls | yes |
| PreferencesDialog (`PreferencesDialog.h:8`) | Audio-DNA menu → Preferences / About; modal, 3 tabs | See Prefs tab rows below | yes |

### Main-window panels (v2)

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| TopBar (`TopBar.h:12`) | Always visible (top, 34px) | Audio-source combo (Mic/File); input-gain slider; **Play/Pause/Stop (WIRED Wave 1-D — global transport over the active deck's layers' active clips; Stop = pause + rewind to in-point; TopBar.cpp:31-33 → MainComponent.cpp:533)**; Tap-tempo; Resync; manual-BPM toggle + BPM edit; 5 multiplier buttons (/4 /2 x1 x2 x4); Quantize combo; Fade slider; Master slider (= composition master opacity; two-way linked with the Composition tab's Master knob, s-rta-0925); "Outputs: Off / N" button (opens the Output menu's display list; outputs-c2 — the Output-display combo is gone); beat wheel + bar-in-four + FPS/DSP readouts | yes |
| SignalBar (`SignalBar.h:15`) | Always visible (3 size modes) | `[+]` add-signal popup; shrink/grow buttons; N SignalStrip children (click = select for Signal inspector; display-only meter) | yes |
| DeckView (`DeckView.h:15`) | Main content grid (scrollable) | **ROUTINES row (s-rta-0927): 8 routine pads over the column numbers -- press = fire/restart (waiting: no-op), right-click = settings menu (Loop/Once, Restore first/Start from now, Start: Ease/Jump, Quantize, Rename..., Remove from layers, Delete routine... behind a confirm); waiting/playing frames, sweep + "5/8", red "!", 50 % off-deck + corner note ("· Drop on B" / "· Save one in the Record tab")**; column-trigger buttons (click = trigger column); deck-tab buttons (switch deck); hosts LayerStrip + ClipCell | yes |
| LayerStrip (`LayerStrip.h:23`) | Per-layer header in DeckView | Clear/Bypass/Solo (Clear also takes every routine off the layer, s-rta-0927); transport `< || > >|`; Speed/Keying/Opacity sliders (V and S follow the model at 30 Hz; V fill in the routine cue (chartreuse `kRoutineCue`) while a routine's hand grips opacity, s-rta-0927); Blend+keying combo (13 keying + ~55 mix modes); Fade-speed slider + transition-mode combo; name-click select; clip-bar drag = scrub; **routine bands over the picture (name + progress hairline; x = remove that routine from every layer; two at most, "+N"), s-rta-0927**. Right-click: none | yes |
| ClipCell (`ClipCell.h:11`) | Per layer×column cell | Thumbnail click = trigger/retrigger; name-bar click = select (Cmd/Shift = multi-select); name-bar drag = move clip; drop targets: files, `fx:`, `source:`, `clip:`, `milkdrop:`, `milkdrop_playlist:`. Right-click: no-op. **"SEQ N" badge on image-sequence cells** (9pt bold, SRC-tag slot, `kMeterGreen`) + **hover tooltip** ("Image sequence — N images at X.X images/sec") + name-bar right-anchors the "(N frames)" suffix so it survives truncation — all `7d3a203`. NOTE: sequence cells were previously INDISTINGUISHABLE from video cells (same paint branch) — that was the defect | yes |
| PreviewPanel (`PreviewPanel.h:12`) | Bottom row | Preview / Output tab buttons (labels only — same GL host); GL preview | yes |
| WaveformDisplay (`WaveformDisplay.h:14`) | Under Preview | Scrolling waveform readout — no controls | yes (readout) |
| TimingWindow (`TimingWindow.h:8`) | Bottom row | BPM / Routing / Oscillators tab buttons — **all 3 tabs are EMPTY placeholders** | no (empty) |
| InspectorPanel (`InspectorPanel.h:23`) | Bottom row | 4 tab buttons (Clip/Layer/Composition/Signal); Pin button; auto-switch on selection | yes |
| BrowserPanel (`BrowserPanel.h:16`) | Bottom row | 6 tab buttons (Files/FX/Sources/Comp-Decks/Record/MilkDrop) | yes |
| Row1 v1 toolbar + 10 preset slots (`MainComponent.cpp:1350`) | Shown alongside v2 chrome | Open Image / Image Folder; Beats-per-image combo; Save/Load/FX-Save/Deck-Save/Deck-Load; 10 numbered preset slots (load button + assign combo) — duplicates v2 TopBar controls | yes (v1/v2 dup) |

### Inspector tabs

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| ClipInspector (`ClipInspector.h:25`) | Inspector → Clip tab (auto on clip select) | Dashboard (8 knobs + 8 source pickers); Transport (mode/loop/trigger/speed/reverse/duration); conditional Images-per-sec OR Beat-Division+Content-Beats; 8 cuepoint jump + 8 Set; Autopilot (action/duration/beat-snap); Source Parameters (UniversalParamControls); Video (opacity/W-H/blend/alpha/RGBA); Transform (fit Stretch/Bars/Crop with a caption saying what the selected mode does -- greyed for a Source, s-rta-0926b plan-fitmode; pos/scale/rotation/anchor); Effects (EffectStackView); interactive timeline (in/out/playhead drag) | yes |
| LayerInspector (`LayerInspector.h:28`) | Inspector → Layer tab | Editable name; Dashboard; Autopilot (4 dir + trigger-mode + beat-count + loops); Layer (master/persistent/ignore-column); Video (blend/opacity/W-H/auto-size); Transition (blend ~55 + duration); Keying (Transparent only, 13 modes); Dry/Wet (FX-Only only); 3D controls (ThreeD only); Transform (5 UPCs); Feedback (enable + preset + 7 sliders); Layer Effects (EffectStackView) | yes |
| CompositionInspector (`CompositionInspector.h:23`) | Inspector → Composition tab | Dashboard; Autopilot (4 dir + duration + clip-loops + loop + master-layer); Per-Type Autopilot (enable + cycle sliders + randomize); Composition master/speed; Transform (5 UPCs); Global Effects (EffectStackView); Output resolution combo (presets 1920x1080 / 1280x720 / 2560x1440 / 3840x2160 / 1080x1920 portrait / 1080x1080 square / 1024x768 4:3, plus a "Custom (W x H)" item whenever the canvas matches no preset -- `src/ui/CanvasSizeCombo.h`, s-rta-0926b canvas fix round; = the composition canvas: the preview, recordings, Syphon, render_frame and snapshots are this size, s-rta-0926b plan4). Collapse triangles + P. buttons decorative. **Panel-wide FX drop target (2026-07-30): fx: drags land anywhere on the panel → global stack via existing undo-recorded path; multi-select = one undo entry** | yes |
| SignalInspector (`SignalInspector.h:15`) | Inspector → Signal tab | Audio: threshold/gain/falloff. Oscillator: wave-shape (5) + beat-duration (6) + amplitude + phase. Envelope: curve-type (3) + beat-duration (5) + amplitude + phase + looping/one-shot toggles + **paint-only curve editor (NOT draggable)** | yes |

### Browser tabs

| Surface | Reach / trigger | User-visible functions | Live? |
|---|---|---|---|
| Files (`FilesBrowser.cpp`) | Browser → Files | Up button; path bar; search; Grid/List toggle; file grid click/dbl-click/drag (`files:`); right-click = toggle favorite; directory navigate. **Perf rework (2026-07-30): async background thumbnail decode + path+mtime LRU cache + instant Grid/List toggle (no sync decode on open)** | yes |
| FX (`FXBrowser.cpp`) | Browser → FX | Search; 11 collapsible category headers; effect rows click / Cmd-Shift multi-select; drag `fx:name,name` to cell/stack | yes |
| Sources (`SourcesBrowser.cpp`) | Browser → Sources | Search; 19 category headers; **109 hand-listed source rows** (registry NOT used, but the 6 previously-absent registered sources added Wave 0; Simulation/Routing/MilkDrop headers now populated); click/multi-select; drag `source:id,id` | yes |
| Compositions (`CompDecksBrowser.cpp`) | Browser → Compositions | 2 collapsible sections; Save Composition wired; row click loads (compositions: confirm; decks: append as a new tab, undoable); right-click = Open / Show in Finder / Delete... (Trash, confirmed). Deck save/load live in the deck tab row (DeckView) — s-rta-0926b plan6 | yes |
| Record (`RecordPanel.cpp`, model `RecordPanelModel.h`) | Browser → Record | Record / Record Over / Stop Recording / Load Take... / Play (with audio) / Stop Playback / Repair; name field; record-audio switch; notice line; 4 Hz refresh over `RecorderHost::Status`; Save Routine row (From bar / To bar / Name / Save Routine) + its notice line -- the routine pad row moved to the deck's ROUTINES row (s-rta-0927) | yes (live-verified s-rta-0924b, 10 states over REST) |
| MilkDrop (`MilkDropBrowser.cpp`) | Browser → MilkDrop | 4 sub-tabs (Curated/Favorites/Recent/All); search; Prev/Next/Random/Lock nav; 3 play-modes (Jukebox/VJ-Clip/Playlist); Jukebox play + pool/mode/timing combos + blend; Playlist cycle/timing + blend; preset rows click/multi-select/drag; right-click = favorite. **2026-07-30: 30 curated presets bundle into the .app + auto-populate at launch; preset manager survives GL detach (crash-#2 UAF fixed — SignalBar expand/collapse safe)** | yes (needs libprojectM) |

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
| **Analysis thread** | `AnalysisThread` pulls 512-hop, resamples device rate -> fixed internal 48 kHz (`AnalysisResampler`, R13, bypass at 48 kHz), maintains 2048-window, runs 14-stage pipeline | FeatureSnapshot (43 fields, machine-counted 2026-09-24 -- includes R13's `sourceSampleRate`/`bandValidMask`), waveform snapshot (2048f), PCM double-buffer (512, for projectM) | `FeatureBus` triple-buffer (1 atomic packs write/latest/read + new-data flag, wait-free CAS); waveform = **seqlock** (Wave 1-D — reader retries on version change, strictly torn-read-free; replaced the count-release + plain-memcpy that could tear); PCM = index-swap double-buffer |
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

Source: `src/api/ApiServer.cpp` (route registrations counted by command, s-rta-0926). **42 registered
routes total: 27 core control endpoints (rows 1-27) + 7 `/api/perf/*` (rows 28-34) + `POST
/api/audio/source` (row 35) + 6 `/api/routine/*` (rows 36-41) + `POST /api/set_clip_param` (row 42,
s-rta-0926b plan-fitmode)**; all functional (`/api/set_bpm` wired
Wave 0; `/api/resync` added s-rta-0925 -- manual Resync via `requestResync()`, message thread ->
analysis thread). Rows 28-35 are the performance take recorder's REST surface (see "Audio Store /
Step 3" in `docs/claude/recording.md` for the full field-level detail of each); rows 36-41 are the
routines slice-1 surface (s-rta-0926 -- see "Routines" in `docs/claude/recording.md`).

| # | Method | Path | Action |
|---|---|---|---|
| 1 | GET | /api/health | ok, version 0.1.0, fps, effects_count |
| 2 | GET | /api/status | fps, frameTime, masterLevel (= composition master opacity eff(), s-rta-0925), activeDeck, renderOnsetPulses, bpm/phase/genre/energy |
| 3 | GET | /api/composition | full deck→layer→clip tree |
| 4 | POST | /api/trigger_clip | onTriggerClip(layer, column) |
| 5 | POST | /api/trigger_column | onTriggerColumn(column) |
| 6 | POST | /api/set_param | set clip-effect or global-chain param |
| 7 | POST | /api/set_layer_opacity | active-deck layer opacity |
| 8 | POST | /api/set_master_signal | Master Signal depth (s-rta-0925 mastersignal Step 1), via manualWrite(compScalarPath("signal")) |
| 9 | POST | /api/switch_deck | onSwitchDeck(deck) |
| 10 | POST | /api/snapshot | takeSnapshot() (blocks), returns path |
| 11 | GET | /api/bpm | bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount, totalBarCount, resyncBarOrigin, totalBeatCount (s-rta-0927 beat clock), downbeatDetected (level) |
| 12 | POST | /api/set_bpm | manual BPM override — setManualMode+setManualBPM via message thread (wired Wave 0) |
| 13 | POST | /api/resync | manual Resync via `BPMTracker::requestResync()` (s-rta-0925), same funnel as the TopBar Resync button |
| 14 | GET | /api/features | full FeatureSnapshot dump (incl. monotonic onsetCount) |
| 15 | POST | /api/inject_features | write FeatureBus (test/automation) — registered only when `allowFeatureInjection_` (test mode); 404 in production |
| 16 | POST | /api/load_image | loadImage() + 100ms GL sleep |
| 17 | POST | /api/load_source | setActiveSource() |
| 18 | POST | /api/load_composition | loadComposition() |
| 19 | POST | /api/set_effect | enable/disable + params on global-chain effect |
| 20 | GET | /api/effects | list global-chain effects |
| 21 | GET | /api/sources | list registered source ids |
| 22 | POST | /api/render_frame | captureFrame() to path |
| 23 | POST | /api/reset | clear image + source + disable all effects |
| 24 | POST | /api/set_effect_chain | batch disable-all + enable/configure requested |
| 25 | GET | /api/state | fps, frame_time, master_level (= composition master opacity eff(), s-rta-0925), gpu_time_ms / peak_gpu_time_ms (GL timer queries, s-rta-0926b plan4), effects[], decks |
| 26 | GET | /api/syphon | Syphon output enabled/initialized status (P22.1) |
| 27 | POST | /api/set_syphon | toggle Syphon output publishing |
| 28 | POST | /api/perf/record | arm: name, audio, audioFile, onsetMarkers, overdubAssetId |
| 29 | POST | /api/perf/stop | disarm + finalize the take |
| 30 | POST | /api/perf/load | load a take folder |
| 31 | POST | /api/perf/play | replay (`withAudio`: true replays audio points through the transport, false is silent wall-clock replay); restores checkpoint 0 first (s-rta-0925) |
| 32 | POST | /api/perf/stop_play | stop replay; give the live input back |
| 33 | POST | /api/perf/repair | crash recovery — re-derives a truncated/incomplete asset's frame count |
| 34 | GET | /api/perf/status | recording/playing/finished state, take-clock, `preambleCount/Fired/Refused/Unresolved`, `inputSource`, error counters |
| 35 | POST | /api/audio/source | dev/probe control — switches the live app between the live input and the loaded file transport (s-rta-0925) |
| 36 | POST | /api/routine/save | sliceRoutine() a piece of the loaded take into a named Routine, assigns a bank slot (s-rta-0926) |
| 37 | POST | /api/routine/fire | queue the routine PENDING; RoutineEngine starts it on the next bar (its own quantize, or the global Quantize override) |
| 38 | POST | /api/routine/stop | `{"slot":N}` or `{"all":true}` — stop() / stopAll(), releases every grip the routine holds |
| 39 | POST | /api/routine/set | edit a saved routine's `loop`/`restoreState`/`quantize`/`name` (any subset) |
| 40 | POST | /api/routine/remove | free a bank pad; erases the routine unless another pad still references it |
| 41 | GET | /api/routine/status | clock beat, per-slot state (empty/idle/pending/running), lanes/preamble/stacking counters, lastSaved/lastError |
| 42 | POST | /api/set_clip_param | per-clip field write, active deck: `{"layer","column","param":"fitMode","value":0\|1\|2}` (Stretch/Bars/Crop; other params -> "unknown param"); message thread, ok:true once well-formed, not undo-recorded; `/api/composition` reads `fitMode` back per clip (s-rta-0926b plan-fitmode) |

The 7 `handlePerfRecord`/`handlePerfStop`/`handlePerfLoad`/`handlePerfPlay`/`handlePerfStopPlay`/
`handlePerfRepair`/`handlePerfStatus` handlers return 503 "Recorder unavailable" when the
callback is unwired; `handleAudioSource` returns 503 "Audio source unavailable" in the same case.
Both mutate via `juce::MessageManager::callAsync` to the message thread. The 5 mutating
`handleRoutine*` handlers (rows 36-40) marshal the same way and return 503 "Routines unavailable"
when unwired; `handleRoutineStatus` (row 41) is synchronous, reading only `RoutineEngine::status()`
(a mutex-guarded copy, safe from any thread) -- never the composition's routine vectors.

Eyes TEST server (`src/test/TestServer.cpp`, port 8080, 28 endpoints -- recounted s-rta-0927 from `server_.Get/Post` in `setupRoutes`, incl. outputs-c1's `set_output_tap` + `output_probe`) is gated by
`AUDIODNA_BUILD_TEST_SERVER=ON` + `--test-mode` (OFF by default) — separate surface.

TEST-ONLY build path on the production port (not a counted row): `POST /api/debug/stall_message_thread
{"ms":1..2000}` (s-rta-0927 beat clock) sleeps the MESSAGE thread once for `ms` -- the deterministic stall
`.harmony/probe-beatclock.sh` and `probe-routines.sh` row 7s use. Compiled only with
`AUDIODNA_BUILD_TEST_SERVER=ON` (`#if AUDIODNA_TEST_SERVER`, `ApiServer.cpp`); needs no `--test-mode`;
a build without the flag 404s it.

### OSC input (`src/osc/OscHandler.cpp`) — 15 patterns, subsystem **LIVE** (Wave 1-B, 2026-07-17)

`startListening(8000)` is called unconditionally at startup (`MainComponent.cpp:1207-1211`,
like ApiServer); receiver binds UDP port 8000 (de-facto OSC receive default). All 14/14
callbacks are now wired (`MainComponent.cpp:2083-2148`, s-rta-0926 re-derived), each routing through the same
handler as the equivalent REST/UI/MIDI path (clip/deck/snapshot → same as REST; bpm →
manual-override tracker; resync → `BPMTracker::requestResync()`, same funnel as the TopBar Resync
button, s-rta-0925; layer opacity/bypass/solo/mute → active-deck layer fields; macro →
global dashboard-link bank; effect param → global effect-chain; signal → Master Signal depth,
s-rta-0925 mastersignal Step 1; routine → fire a bank pad, same funnel as `/api/routine/fire`,
s-rta-0926 routines slice 1). Delivery is on the message
thread (`MessageLoopCallback`). Patterns:
`/audiodna/clip/{layer}/{column}`, `/clip/{layer}/{column}/fit <int 0..2>` (Stretch/Bars/Crop, matched BEFORE
the bare clip trigger, s-rta-0926b plan-fitmode), `/layer/{n}/opacity|bypass|solo|mute`, `/deck/{n}`,
`/master`, `/signal`, `/bpm`, `/resync`, `/snapshot`, `/macro/{n}`, `/effect/{name}/{param}`,
`/routine/{slot}` (value > 0 fires; 0 ignored).
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
- **Binding / MIDI** — `BindingManager`: **21 actions** (TriggerRoutine appended s-rta-0926), InputType Keyboard/MidiNote/MidiCC,
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
  (REAL, via REST/OSC/menu); performance take recorder (`src/recording/`: `AudioTap` second
  fan-out in `CombinedCallback`, `AudioStore` `~/Documents/Audio-DNA/Audio/<id>.adna-audio`,
  `RecorderClock`, `PerformanceRecorder`, `Take` v3 / `Lane` / `TempoMap` / `PerfState`,
  `Program` + `Player`, `RecorderHost`; takes at `~/Documents/Audio-DNA/Takes/<name>.adna-take/take.json`) —
  **LIVE**, replaces the removed `SessionRecorder` (s168, 3736f02); gates: probe-step3 93/0
  (s-rta-0926), probe-onset-render 13/0 (s-rta-0924b), probe-finalize-loop 40/0
  (s-rta-0924b), STEP3_LONG 20 min 82/0, drift +0.28 ms (s-rta-0924b).
- **Genre / energy intelligence** — `GenreDetector` (8 genres + 3-band energy, ~2s EMA +
  ~3s hysteresis internally); drives auto-preset/deck switch + structural scene triggering.
  (GenreSmoothing + MappingSuggester removed Wave 0 — see §8.)

---

## §7 — Small print

Source: lane-5 §3.

- **Keyboard shortcuts** (`MainComponent::keyPressed :1591`): Shift+Cmd+I inspector,
  Shift+Cmd+K keyboard-bind, Shift+Cmd+M MIDI-learn, Cmd+Shift+Esc all outputs off, Cmd+` app to front, Escape swallowed (no output effect, outputs-c2), Cmd+Z /
  Cmd+Shift+Z undo/redo (**LIVE for ALL structural edits incl. triggers** — Undo v1
  COMPLETE steps 1-9: drops, replace/lock/clear, drag move/swap, layer/deck/column
  ops, effect stacks ×3 scopes, clip/column triggers with same-layer merge;
  autopilot/remote-autonomous paths excluded by design), Cmd+S save preset, Cmd+F
  toggle the output on the main display, Cmd+O load preset. Non-Cmd keys → BindingManager; key-up →
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
| Undo / redo | BUILD-COMPLETE 2026-07-19→25 (Undo v1 steps 1-9; commits 7c8d286/7921572/daa9361/6d2def4/7f87094/316a2bf/d90e953/4ee2dac/0a1c882) — ALL structural edits: clip cells, composites/column ops, layer ops (GL-fenced), deck ops (fence fixes latent renderer re-point), effect stacks ×3 scopes, clip/column TRIGGERS with same-layer merge (REST/OSC/MIDI undoable; autopilot never); Cmd+Z + dynamic menu live; tests 170. Remaining: Boris-assisted manual e2e run (.harmony/undo-v1-manual-e2e.md; TCC Allow first); known cosmetics: expanded-FX-row collapse + deck-tab highlight on undo (pre-existing refresh path, follow-up awaiting ratification); accepted risk-#5 family: playing not restored, first-trigger auto-play skip after undo | src/core/ClipCommands.h; DeckCommands.h; EffectCommands.h; EffectScope.h; TriggerCommands.h; UndoService.h/.cpp; MediaReconnect.h; .harmony/undo-v1-ledger.md; .harmony/undo-v1-manual-e2e.md |
| Performance recorder | LIVE (s168 core 3736f02; step 3 wiring s-rta-0924; Record panel s-rta-0924b; replay-restore preamble s-rta-0925) — replaces the removed `SessionRecorder` (section 8, removed s168, 3736f02) | src/recording/*; MainComponent.cpp:1945-2048, 5074-5300 |
| Routines (slice 1) | LIVE (s-rta-0926, commits 58b14d7 + ebbff22) — save a piece of a loaded take as a named routine in an 8-slot bank; fire restores its preamble then plays its lanes on the next bar; loop/once, "Start from now", gesture-begin stacking arbitration for two routines on one control. Surfaces: 6 `/api/routine/*` REST routes, `/audiodna/routine/{slot}` OSC, `Binding::Action::TriggerRoutine` (8 whole-word overlay targets, no bank-strip UI yet — lane 3 not built, cut per plan). Deferred to slice 2+: no `routine` take lane / `via`-tagged capture, no lane editor UI, no per-slot re-target table, no import/export file, routine-vs-replay arbitration is tick order not gesture-begin order (R13, disclosed) | src/model/Routine.h; src/recording/RoutineSlice.{h,cpp}; src/recording/RoutineEngine.{h,cpp}; src/api/ApiServer.cpp:281-286,1520-1697; src/osc/OscHandler.cpp:177-190; src/binding/Binding.h; docs/claude/recording.md "Routines" |
| OSC subsystem | LIVE 2026-07-17 (Wave 1-B) — `startListening(8000)` called at startup; 14/14 callbacks wired (`/audiodna/signal` added s-rta-0925 mastersignal Step 1; `/audiodna/resync` added s-rta-0925 resync; `/audiodna/routine/{slot}` added s-rta-0926 routines slice 1; port hardcoded, no prefs UI) | OscHandler.cpp:15; MainComponent.cpp:2083-2148 |
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
| 48kHz hardcode | RESOLVED 2026-09-24 (s-rta-0924, R13) — the runtime guard/warning is deleted; the analysis thread now resamples any device rate to its fixed internal 48 kHz (`AnalysisResampler`, bit-identical bypass at 48 kHz); `SpectralFeatures` gates bands/stats to the device bandwidth (`bandValidMask`); `FeatureSnapshot::sourceSampleRate` publishes the device rate fed in. `kSampleRate=48000` + K-weighting coeffs still assume 48 kHz BY DESIGN — that is the fixed internal rate everything resamples to, not an unaddressed gap | AnalysisResampler.h/.cpp; AnalysisThread.h:49 (comment), :94 (resampler_); SpectralFeatures.h (setInputBandwidthHz/bandValidMask); FeatureSnapshot.h:93-94; MainComponent.h (analysisThread_ ctor); MainComponent.cpp (startup log line, no guard) |
| Dual mapping engines | RISK — MappingEngine (resets-then-accumulates) and RoutingEngine both write the same params every frame; order undefined | Renderer.cpp:198, :210 |
| Only Global MacroBank | GAP — 3-scope enum but only Global instantiated → 8 live macros, not 24 | MainComponent.h:207 |
| OutputWindow shader table | RESOLVED (s-rta-0927 outputs-c1) — `OutputRenderer` and its ~80-shader table are deleted; the window compiles no programs and presents the main canvas | — |
| Waveform snapshot torn read | FIXED 2026-07-17 (Wave 1-D) — replaced count-release + plain-memcpy with a seqlock (reader retries on version change → strictly torn-read-free; a plain double buffer was tried first but the torn-read stress test showed it still tears when the writer laps the reader). Threaded regression test added | AnalysisThread.cpp:337 (writer), :365 (reader); tests/test_waveform_snapshot.cpp |
