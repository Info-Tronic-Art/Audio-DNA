# Mapping / Rendering Pipeline Reference

> Moved from CLAUDE.md (claudemd-split). Mapping system internals, temporal/time effects, audio uniforms, composition transform, cross-deck transitions, visual-issue debugging.

---

## The Mapping System

### How Mappings Work

```
Audio Feature (source) → Normalize to [0,1] → Apply Curve → Scale to output range → Smooth → Effect Parameter (target)
```

1. **Extract**: Read source value from `FeatureSnapshot` (e.g., `snapshot.rms`)
2. **Normalize**: `(raw - inputMin) / (inputMax - inputMin)` → [0, 1]
3. **Curve**: Apply transform function:
   - **Linear**: `y = x`
   - **Exponential**: `y = x^2.0` (emphasizes peaks)
   - **Logarithmic**: `y = log(1 + x * 9) / log(10)` (compresses peaks, lifts lows)
   - **S-Curve**: `y = x² * (3 - 2x)` (smoothstep, de-emphasizes extremes)
   - **Stepped**: `y = floor(x * N) / N` (quantized to N steps)
4. **Scale**: `outputMin + curved * (outputMax - outputMin)`
5. **Smooth**: EMA filter with per-mapping state (One-Euro variant removed from Smoother.h Wave 0)
6. **Write**: Set on target effect's parameter slot

### Render Thread Consumption

Each frame, the render thread:
1. Acquires latest `FeatureSnapshot` from triple buffer (atomic read, ~10ns)
2. Iterates all active `Mapping` objects, running the pipeline above
3. Writes computed values to each target `Effect`'s parameter slots
4. `EffectChain::render` uploads each effect's parameters as `glUniform1f` calls
5. Renders the effect chain (ping-pong FBOs)

### User Creates/Edits/Saves Mappings

- **Create**: Click "▼map" on any effect parameter → opens `MappingEditor`
- **Edit**: Select source feature dropdown, curve type dropdown, adjust input/output range sliders, smoothing knob
- **Save**: `PresetManager` serializes all effects + mappings to JSON
- Multiple mappings can target the same parameter (values are summed)

Master Signal (`Composition::masterSignal`, `CompScalar::Signal`, s-rta-0925) scales the reach of
every signal→parameter connection at the one point where the signal enters
(`ConnectionEngine::evaluate` for non-Macro sources; `MacroBank::updateValues`; v1
`MappingEngine::processFrame`); 1.0 (default) = bit-identical to no fader at all, 0.0 = every
connected control sits at its own hand value (a hand-turned macro keeps working at any depth).

---

### Debugging Visual Issues

1. Check shader uniform names match FeatureSnapshot field names exactly
2. Check that the effect is registered in EffectLibrary and enabled in the chain
3. Check FBO ping-pong: if effects look wrong when chained, the read/write FBOs may be swapped
4. Use shader hot-reload to iterate without restarting the app (inert for the shipped set — those shaders are compiled from `EmbeddedShaders.h` strings, not files)

---

### Time Effects & Temporal Architecture (P16)

**Temporal effects** (Echo, Posterize Time, Freeze, Frame Delay) use `u_prev_frame` — the previous frame's output stored in a per-layer temporal buffer. The `EffectDef::temporal = true` flag tells the system to bind and save previous frames.

**Two render paths both support temporal**:
- `EffectChain::render()` (global effects, single-image mode): has `prevFrameTexture_`/`prevFrameFBO_`. When temporal effects are active, the last effect always renders to FBO (never to screen), the frame is saved, then blitted to screen.
- `CompositorEngine::applyClipEffects()` (per-clip/layer deck mode): uses `layerTemporalBuffers_` map keyed by layer ID. Binds `u_prev_frame` from the layer's buffer, saves output after chain completes.

**Frame Ring Buffer** (`FrameRingBuffer` in CompositorEngine): stores 480 previous frames at 1/4 resolution for Screen Split and Frame Stutter effects. These effects are intercepted in `applyClipEffects()` before normal shader processing and rendered by the compositor directly — they don't use GLSL shaders at all. The ring buffer uses ~240MB VRAM at 1080p.

**Feedback System** (`FeedbackProcessor`): per-layer Larsen feedback loop. Each layer with `feedback.enabled` gets its own FBO pair. Applied after clip effects, before layer effects in `compositeDeck()`. 6 presets: Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch. UI in LayerInspector "Feedback" section.

**Signal Routing**: `SignalRegistry::evaluateAll()` and `RoutingEngine::processFrame()` run every frame in `Renderer::renderOpenGL()`. Renderer holds a `SignalRegistry*` (owned by MainComponent) and a `RoutingEngine`. TestServer exposes 5 signal/routing REST endpoints.

---

### Audio Uniform System (P18)

Effect shaders and source shaders can access all 42+ audio features via uniforms. The uniform uploading is implemented in three places:

- **`ProceduralSource::uploadUniforms()`** — for procedural sources. Uploads all basic + extended uniforms.
- **`CompositorEngine::uploadAudioUniforms()`** — for per-clip/layer effects in deck mode. Called via `setLatestSnapshot()` before `compositeDeck()`.
- **`EffectChain::uploadEffectUniforms()`** — for global effects in single-image mode. Called via `setLatestSnapshot()` before `render()`.

**Available uniforms in all shaders** (effect and source):

| Uniform | Type | Source |
|---------|------|--------|
| `u_rms` | float | RMS amplitude |
| `u_bass`, `u_mid`, `u_high` | float | Band energies [1], [3], [5] |
| `u_beatPhase`, `u_barPhase`, `u_phrasePhase` | float | Beat/bar/phrase sawtooths |
| `u_spectralCentroid`, `u_spectralFlux` | float | Spectral features |
| `u_onsetStrength`, `u_onsetDetected` | float | Onset. `u_onsetDetected` = 1.0 on exactly the first render frame (per GL context) that observes >= 1 new onset since that context's previous frame, else 0.0 — derived from the `onsetCount` delta (`OnsetPulse`), so never lost at any fps and never duplicated above the ~93.75 Hz analysis rate; >= 2 onsets in one frame (only under a > 50 ms stall) collapse into one pulse. `u_onsetStrength` is the LATEST hop's ODF (continuous) |
| `u_dominantPitch`, `u_pitchConfidence` | float | Pitch detection |
| `u_detectedKey`, `u_keyIsMajor` | float | Key detection (-1 to 11, 0/1) |
| `u_structuralState` | float | 0=normal, 1=buildup, 2=drop, 3=breakdown |
| `u_bpm` | float | Current BPM |
| `u_hcdf` | float | Harmonic change detection function |
| `u_bandEnergies[7]` | float array | All 7 frequency bands |
| `u_chromagram[12]` | float array | 12 pitch classes (C through B) |
| `u_mfccs[13]` | float array | 13 MFCC coefficients |
| `u_genre` | float | Detected genre (0-7): House/Techno/DnB/HipHop/Ambient/Rock/Pop/Jazz |
| `u_genreConfidence` | float | Genre classification confidence [0, 1] |
| `u_energyState` | float | Overall energy level (0=low, 1=medium, 2=high) |
| `u_sidechainPump` | float | Bass/mid anti-correlation [0, 1] (P25) |
| `u_swingRatio` | float | Timing swing 0.5=straight, >0.5=swung (P25) |
| `u_formantPresence` | float | Vocal formant energy [0, 1] (P25) |
| `u_resonancePeak` | float | Spectral kurtosis [0, 1] (P25) |
| `u_reeseBass` | float | Bass spectral spread [0, 1] (P25) |

**Important**: These uniforms are available in every shader but only consume GPU resources if the shader declares them. Unused uniforms are silently ignored by `glGetUniformLocation` returning -1.

Master Signal does NOT scale these uniforms (Boris 2026-09-25 Q2): every effect/source that reads
the beat clock or an audio uniform directly keeps pulsing at any Master Signal depth, including 0%.
The fader only reaches signal→parameter connections (`ConnectionEngine`, `MacroBank`, v1
`MappingEngine`), never a GL-thread uniform read.

---

### Composition-Level Transform (P25)

The `comp_transform` shader applies position/scale/rotation to the entire final output. Applied after the effect chain renders, before the master level dim. Uses `glBlitFramebuffer` to copy the framebuffer, then renders the transform shader.

Fields in `Composition`: `compPositionX/Y` (normalized offset), `compScale` (1.0=100%), `compRotation` (degrees), `compAnchorX/Y`. UI controls already exist in the CompositionInspector's Transform section.

---

### Cross-Deck Transitions (P25)

When `Composition::activeDeckIndex` changes, the Renderer saves the current frame as the "outgoing" deck texture and blends to the new deck over `Composition::globalTransitionSpeed` seconds. Three blend modes: Alpha (crossfade), Add (additive), Multiply. Uses `Composition::crossfaderBlendMode` for the blend mode.

The `deck_transition` shader takes two textures (`u_textureA` = outgoing, `u_textureB` = incoming) and a progress uniform. Frame is saved to `prevDeckTexture_` on deck switch detection.

---
