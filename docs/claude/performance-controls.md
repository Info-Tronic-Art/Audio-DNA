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

**Persistent Layers**: `Layer::persistent = true` keeps a layer rendering even when its deck is not active. `CompositorEngine::compositePersistentLayers()` composites persistent layers from non-active decks after the active deck's layers. `Renderer` holds a `Composition*` to iterate all decks.

**Beat Snap Granularity**: `Clip::BeatSnapMode` enum (Off, Beat, Bar, TwoBar, FourBar). `Layer::processPendingTrigger(beatInBar, barCount)` now checks the snap granularity before firing queued triggers.

**Ableton Link**: Optional (`-DAUDIODNA_BUILD_LINK=ON`, which defines `AUDIODNA_HAS_LINK=1`). `LinkSync` class wraps `ableton::Link`, updates cached BPM/phase via atomics. The only way to switch it on is the TopBar "Link" toggle (no REST, OSC, MIDI-binding or menu route; the on/off state is not saved in settings, presets, compositions or takes, so every launch starts with Link off). When enabled, MainComponent's ~30 Hz timer re-sends Link's tempo (`applyTempoCommand("link", bpm, Human, linkTick=true)`): manual mode on + `BPMTracker::followExternalTempo()`.
- **Tempo only, never the phase** (s-rta-0926b bpm2, LINK-RAMP ruling b): a Link tempo, changed or unchanged, never realigns the beat phase -- it only changes the rate the phase runs at, so a peer's tempo ramp gives a continuous phase (`test_bpm_stabilization` "[link]"). Following Link's beat/bar phase is **not implemented**: `LinkSync::getBeatPhase()` is kept up to date but nothing reads it, so the app's beats are not aligned to other Link peers' beats. Tap, Resync and an explicit REST/OSC `set_bpm` still realign as before.
- **Default build (`AUDIODNA_BUILD_LINK` OFF, `CMakeLists.txt`)**: Link is not compiled in and is honestly unavailable. `LinkSync::isAvailable()` is false, `setEnabled(true)` is ignored (`isEnabled()` stays false) and `getBPM()` returns 0, so the timer never feeds a tempo and nothing can force manual mode at a made-up 120 BPM (`test_link_sync`). The TopBar toggle keeps its place but is disabled and dimmed (0.4 alpha), with the tooltip "Ableton Link is not included in this build" (`test_topbar_link_toggle`).

**Key-up routing**: `MainComponent::keyStateChanged()` polls all momentary-bound keys and fires release actions. MIDI note-off already routed through `BindingManager::processMidiNoteOff()`.

---
