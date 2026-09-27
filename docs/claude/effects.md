# Effects Library Reference

> Moved from CLAUDE.md (claudemd-split). Effect categories, transitions, effect chain architecture, FX drag-and-drop, autopilot, manual BPM, tooltips.

---

## Effects Library

135 effects across 11 categories + 15 transition shaders. All parameters normalized to [0.0, 1.0] — the shader maps to internal ranges. All shaders are embedded in `src/render/EmbeddedShaders.h`.

### Effect Categories (135 total)

| Category | Count | Examples |
|----------|-------|---------|
| **3D / Depth** | 9 | Perspective Tilt, Cylinder Wrap, Sphere Wrap, Tunnel, Page Curl, Parallax Layers, Dot Field, Luminance Terrain, Voxel Matrix |
| **Warp** | 27 | Ripple, Bulge, Wave, Liquid, Kaleidoscope, Fisheye, Swirl, Polar Coords, Twirl, Shear, Elastic Bounce, Ripple Pond, Diamond Distort, Barrel Distort, Sine Grid, Glitch Displace, Quad Mirror, Flip, Warp Field, Slide Wrap, Tile Grid, Spot Zoom, Bendoscope, UV Remap, Liquid Morph, Infinite Zoom, Density Wave |
| **Color** | 31 | Hue Shift, Saturation, Brightness, Duotone, Chromatic Aberration, Invert, Posterize, Color Shift, Thermal, Contrast, Sepia, Cross Process, Split Tone, Color Halftone, Dither, Heat Map, Selective Color, Film Grain, Gamma Levels, Solarize, Greyscale, Threshold, Exposure, Vibrance, Auto Mask, Chroma Key, Palette Remap, Color Grade, Pitch Chromatic Shift, Key Palette, Chroma Dissolve |
| **Glitch** | 15 | Pixel Scatter, RGB Split, Block Glitch, Scanlines, Digital Rain, Noise, Mirror, Pixelate, Pixel Explosion, Color Flash, Fragment Burst, Signal Destroy, Rhythm Slice, Data Corruption, Glitch Sort |
| **Pattern** | 19 | CRT Simulation, VHS Effect, ASCII Art, Dot Matrix, Crosshatch, Emboss, Oil Paint, Pencil Sketch, Voronoi Glass, Cross Stitch, Night Vision, Triangulate, Neon Edge, Cartoon Ink, Pop Raster, Brush Strokes, Bump Light, Monitor Wall, Topographic Lines |
| **Animation** | 6 | Strobe, Pulse, Slit Scan, Point Zoom, Directional Feedback, Transient Flash |
| **Audio** | 4 | Harmonic Displacement, Timbral Mosaic, Structural Morph, Beat Ripple |
| **Time** | 6 | Echo (temporal trails with Add/Screen/Max/Blend operators), Posterize Time (frame rate reduction), Freeze (full-frame freeze), Screen Split (CCTV grid with per-cell delay via ring buffer), Frame Stutter (time-jump rewind via ring buffer), Channel Delay (per-RGB temporal offset) |
| **Blend** | 5 | Double Exposure, Frosted Glass, Prism Refract, Rain on Glass, Hexagonalize |
| **Composite** | 3 | Line Cloner, Radial Cloner, Cube Scatter |
| **Blur/Post** | 10 | Gaussian Blur, Zoom Blur, Shake, Vignette, Motion Blur, Glow, Edge Detect, Sharpen, Edge Blur, Drop Shadow |

### Transition Shaders (15 total)

Clip-to-clip transitions driven by `layer.crossfadeProgress` (0→1). Selected via `layer.transitionMode`. Rendered by `CompositorEngine::applyTransition()` using a dedicated `transitionFBO_`.

| Type | Transitions |
|------|-------------|
| **Standard** | Dissolve, Cut |
| **Wipe** | Wipe Left/Right/Up/Down |
| **Push** | Push Left/Right/Up/Down |
| **Zoom** | Zoom In, Zoom Out |
| **Other** | Iris Circle, Flip Horizontal, Fade to Black |

### Effect Chain Architecture

Effects can be applied at three independent levels — no layer type change is required:

| Level | Data Location | How to Add |
|-------|---------------|-----------|
| **Per-clip** | `Clip::effects` (vector of `EffectSlot`) | Drag FX from browser onto a cell, or onto the clip inspector effect stack |
| **Per-layer** | `Layer::layerEffects` | Drag FX onto the layer inspector effect stack |
| **Global** | `effectChain_` in Renderer | Via the effects rack or composition inspector |

Cells hold clips (images, image sequences, videos) AND procedural sources. FX are independent of media type and layer type.

**Single-image mode**: Input image → FBO A (Effect 1) → FBO B (Effect 2) → FBO A (Effect 3) → ... → Screen. Ping-pong between two FBOs.

**Deck compositing mode** (v2 rendering pipeline per layer):
```
Clip texture → Per-clip effects → Transition blend (if crossfading) → Per-layer effects → Layer transform → Keying (if Transparent) → Blend onto accumulator
```
FX Only layers apply their clip's effects to the composited accumulator. Mask layers use their content as a luminance alpha mask. Global effects (via `effectChain_` in Renderer) run on the final composited output after all layers.

**Important**: `CompositorEngine::applyClipEffects()` resolves shader names via `EffectLibrary::getEffectDef(displayName)->shaderName`, NOT directly from `slot.effectName`. The `slot.effectName` stores the display name (e.g., "Ripple"), while shaders are compiled under snake_case keys (e.g., "ripple").

### FX Drag-and-Drop

Effects are dragged from the FX Browser and dropped onto:
- **Deck cells** — adds the effect to the clip's per-clip effect chain (`Clip::effects`)
- **ClipInspector** — drops anywhere on the inspector add to clip effects (cyan highlight on hover)
- **LayerInspector** — drops anywhere on the inspector add to layer effects (cyan highlight on hover)
- **EffectStackView** — drops directly onto the effect stack area

Each effect row in EffectStackView has: [B] bypass button, effect name, [X] delete button. Effects can be expanded to show parameter sliders. Each slider supports right-click to reset to default value.

MainComponent inherits `juce::DragAndDropContainer`. FXBrowser's FXListContent initiates drags via `startDragging("fx:effectName", ...)`. ClipCell, ClipInspector, LayerInspector, and EffectStackView all implement `juce::DragAndDropTarget`.

### Autopilot System

Autopilot auto-advances clips in a layer. Two trigger modes:
- **On Beat** — advances after N beats (1/2/4/8/16/32), multiplied by the loops count
- **End of Video** — advances when `clip->playheadPosition >= outPoint` (checked every frame, not just on beat crossings)

The `Autopilot` class runs in `Renderer::renderOpenGL()` via `autopilot_.processFrame()`. When clips advance, `onAutopilotAdvanced_` fires async on the message thread to refresh the DeckView.

Layer autopilot fields: `autopilotEnabled`, `autopilotEndOfVideo`, `autopilotLoops`, `defaultAutopilotAction`, `defaultAutopilotDuration`.

### Manual BPM Mode

TopBar has a "Manual" toggle. When enabled:
- An editable BPM text field appears (type value, press Enter)
- `BPMTracker::setManualMode(true)` freezes the stabilization pipeline
- Beat phase still runs from the manually-set BPM. Typing a BPM (or a REST/OSC set_bpm) changes the tempo only -- the beat keeps running; Tap or Resync realign it (s-rta-0926b plan3 A).
- All beat-driven features (beatPhase, barPhase, phrasePhase, autopilot) work without audio

### Tooltip System

`juce::TooltipWindow` in MainComponent (600ms delay). Any component with `setTooltip()` shows tooltips on hover. Preferences → General has a "Show Tooltips" toggle. Comprehensive tooltip coverage is scheduled for P26 (final build phase).

---
