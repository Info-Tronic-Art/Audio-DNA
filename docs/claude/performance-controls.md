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

**Ableton Link**: Optional (`-DAUDIODNA_BUILD_LINK=ON`). `LinkSync` class wraps `ableton::Link`, updates cached BPM/phase via atomics. When enabled, overrides BPM tracker with Link's tempo via manual mode. **NOTE**: `AUDIODNA_BUILD_LINK` defaults OFF (`CMakeLists.txt`), so in a default build `AUDIODNA_HAS_LINK` is undefined and every `LinkSync` method compiles to a no-op.

**Key-up routing**: `MainComponent::keyStateChanged()` polls all momentary-bound keys and fires release actions. MIDI note-off already routed through `BindingManager::processMidiNoteOff()`.

---
