# Live Performance Controls Reference

> Moved from CLAUDE.md (claudemd-split). Layer Router, Per-Type Autopilot, Live Performance Controls (P20-P21).

---

### Layer Router System (P20)

The Layer Router source (`layer_router`) lets one layer use another layer's rendered output as its input texture. This enables feedback loops, picture-in-picture, and cross-layer effects.

**Architecture**:
- `CompositorEngine::compositeDeck()` saves each layer's final clip texture (after effects, transform, before keying/blending) into `layerOutputTextures_` keyed by layer ID
- When a clip has `sourceType == "layer_router"`, `Renderer::renderSource()` intercepts it, reads the `u_src_layer` param to determine which layer index to read, and returns the saved texture
- The "Source Layer" param maps [0,1] to layer indices 0-9
- Self-reference safety: if a layer routes to itself, it gets the previous frame's output (one frame delay). Circular references between two layers produce feedback effects.
- Only the active deck's layers publish output; a persistent layer of another deck is never a router source (s-rta-0926b ruling R4-router: the router resolves indices in the active deck only, and publishing under another deck's layer id would collide with the active deck's same-id layer).

---

### Per-Type Autopilot (P20)

Composition-level automation that sets different beat timings per layer type:
- **Opaque layers**: cycle every N beats (default 16)
- **Transparent layers**: cycle every N beats (default 8), optional randomization
- **FX Only layers**: cycle every N beats (default 4), optional randomization
- Config in `Composition::PerTypeAutopilotConfig`, UI in CompositionInspector "Per-Type Autopilot" section

---

### Live Performance Controls (P21)

**Binding System Extensions**:

- `Binding::TriggerMode` — `Toggle` (default, press to toggle) or `Momentary` (held = active, release = deactivate)
- `Binding::CCMode` — `Absolute` (0-127 → 0-1) or `Relative` (< 64 = decrement, > 64 = increment, for endless encoders)
- `Binding::TargetMode` — `ByPosition` (survives reorder), `ThisItem` (follows clip by ID), `Selected` (current UI selection)
- `Binding::velocityToOpacity` — maps MIDI velocity to clip opacity on trigger
- New actions: `AdjustLayerOpacity`, `LayerTransport` (play/pause toggle), `ToggleEffectBypass`, `AdjustMacro`
- `GlobalStop` ("Stop" in the bind overlay) stops every running and waiting routine and nothing else, exactly like the TopBar Stop (`[]`, "Stop all routines") — no clip stop or rewind and no audio-transport stop; the Play / Pause binding still stops a playing audio file (s-rta-0926b).
- `TriggerRoutine` fires a routine whose own restore style (`Routine::restoreStyle`: Ease, the default, glides its knobs onto the bar; Jump cuts to them on the bar — `POST /api/routine/set {"restoreStyle": "ease"|"jump"}`) applies at its start, every loop return and a restart (s-rta-0926b; `docs/claude/recording.md`). The same routine fires from its pad in the deck's ROUTINES row (a press = fire/restart, never stop); a Momentary binding's release still stops it (s-rta-0927; `docs/claude/recording.md` "Surfaces").

**Persistent Layers**: `Layer::persistent = true` keeps a layer rendering even when its deck is not active. `CompositorEngine::compositePersistentLayers()` composites persistent layers from non-active decks after the active deck's layers (so always on top). `Renderer` holds a `Composition*` to iterate all decks. s-rta-0926b (ruling `.harmony/.reports/s-rta-0926b/ruling-render-forks.md`):
- A persistent layer gets every per-layer stage it gets on its own deck (`renderLayerStages`: clip transform + opacity, clip effects, clip-to-clip transition, feedback, layer effects, layer transform), with its state keyed by its own deck (`LayerStateKey`); its crossfades keep advancing while its deck is inactive.
- An **Opaque** persistent layer BLENDS OVER the active deck with its blend mode and honours layer opacity (through the alpha keying pass a Transparent layer uses) — it never hides the active deck. **Transparent** keys and blends as usual. **FX Only** (and a media-less effect clip on an Opaque/Transparent layer) applies its clip's effects over whatever is on screen at that point of the persistent pass.
- **Mask** and **3D** layers cannot be persistent: `Layer::canBePersistent(type)` is the one rule, used by the compositor and by the LayerInspector, whose Persistent toggle is disabled for them (tooltip "Persistent is available for Opaque, Transparent and FX Only layers").
- Persistent layers render over an EMPTY active deck (no active clip with content) exactly as over a black one (`CompositorEngine::beginEmptyActiveDeck` clears the accumulator to opaque black; the image/source fallback is used only when no deck has anything to draw).
- A persistent layer is never a Layer Router source (see Layer Router System above).

**Inactive decks keep time** (s-rta-0926b plan4 item 2; Boris: "finish the fade. when we load a new deck that does not touch the clips playing in the layer"): every frame `Renderer::renderOpenGL` runs `DeckClock::tick` (`src/render/DeckClock.h`) for every deck except the active one, inside the `deckActive` fence (so `UndoService::withDeckDetached` covers it like `compositePersistentLayers`). For every layer the active-deck path would composite (visible, not bypassed, solo rule) it advances the clip-to-clip crossfade with the same clock (`LayerClock::advanceCrossfade`, which `CompositorEngine::advanceCrossfade` forwards to) -- a fade started on a deck finishes while another deck is shown. Persistent layers are owned by `compositePersistentLayers` (their crossfade always; their media when `Layer::canBePersistent`) and are never advanced twice. B2 (Boris Q1: "keep playing"): the active clip's -- and during a fade the outgoing clip's -- video / image-sequence CLOCK advances too, with no decode and no upload (`Renderer::tickMediaClock` -> `VideoPlayer::advanceClock`; the player's decode thread idles off screen and, on return, re-seeks and catches up while the layer holds its last frame -- rendering.md "Video playback"), and autopilot runs for every deck with its own beat-crossing baseline (`AutopilotBank`, one `Autopilot` per deck index -- Pitfall 38). One-shot clips reach their end and stop exactly as if watched. Leaving a deck still cancels its pending quantized triggers (L5). `/api/composition` reports per layer `previousClipColumn` / `crossfadeProgress` / `persistent` and per clip `playheadPosition`. Live: `.harmony/probe-deck-clock.sh`.

**Deck tab row**: '+' = New Deck / Load Deck...; right-click a tab = Save Deck / Save Deck As... / Rename Deck... / Duplicate Deck / Remove Deck (the menu is headed by the deck's name; Remove shows a 10-s `Undo Remove "<name>"` button flush right in the row, no dialog); the Deck menu mirrors every action for the active deck. The Compositions browser tab is the library (row click = open / append as a new tab; right-click = Open / Show in Finder / Delete... to the Trash, confirmed). `DeckView::DeckTabButton` intercepts `isPopupMenu()` in `mouseDown` because a JUCE Button fires `onClick` on ANY mouse button (a right-click used to switch decks). Geometry and menus: `src/ui/DeckTabRow.h` (pure, `tests/test_deck_tab_row.cpp`). (Moved from CLAUDE.md's UI Patterns, s-rta-0926b canvas merge.)

**Beat Snap Granularity**: `Clip::BeatSnapMode` enum (Off, Beat, Bar, TwoBar, FourBar). `Layer::processPendingTrigger(beatInBar, barCount)` now checks the snap granularity before firing queued triggers.

**Ableton Link**: Optional (`-DAUDIODNA_BUILD_LINK=ON`, which defines `AUDIODNA_HAS_LINK=1`). `LinkSync` class wraps `ableton::Link`, updates cached BPM/phase via atomics. The only way to switch it on is the TopBar "Link" toggle (no REST, OSC, MIDI-binding or menu route; the on/off state is not saved in settings, presets, compositions or takes, so every launch starts with Link off). When enabled, MainComponent's ~30 Hz timer re-sends Link's tempo (`applyTempoCommand("link", bpm, Human)`): manual mode on + `BPMTracker::followExternalTempo()`.
- **Tempo only, never the phase** (s-rta-0926b bpm2, LINK-RAMP ruling b): a Link tempo, changed or unchanged, never realigns the beat phase -- it only changes the rate the phase runs at, so a peer's tempo ramp gives a continuous phase (`test_bpm_stabilization` "[link]"). Following Link's beat/bar phase is **not implemented**: `LinkSync::getBeatPhase()` is kept up to date but nothing reads it, so the app's beats are not aligned to other Link peers' beats. Tap and Resync realign; a REST/OSC/typed `set_bpm` never does (s-rta-0926b plan3 A).
- **Default build (`AUDIODNA_BUILD_LINK` OFF, `CMakeLists.txt`)**: Link is not compiled in and is honestly unavailable. `LinkSync::isAvailable()` is false, `setEnabled(true)` is ignored (`isEnabled()` stays false) and `getBPM()` returns 0, so the timer never feeds a tempo and nothing can force manual mode at a made-up 120 BPM (`test_link_sync`). The TopBar toggle keeps its place but is disabled and dimmed (0.4 alpha), with the tooltip "Ableton Link is not included in this build" (`test_topbar_link_toggle`).

**Key-up routing**: `MainComponent::keyStateChanged()` polls all momentary-bound keys and fires release actions. MIDI note-off already routed through `BindingManager::processMidiNoteOff()`.

---
