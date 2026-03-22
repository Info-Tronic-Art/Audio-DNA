# Audio-DNA v2: Effects & Sources Build Plan (P13-P20)

> This file tells Claude how to start any phase from P13-P20.
> User says: "kick off phase 13" → Claude reads CLAUDE.md, finds the "kick off" protocol,
> which points here for P13-P20. Claude reads this file, finds Phase 13, and executes.
>
> Merges all content from:
> - research/resolumeEffectSourceIntegration.md (Resolume parity: 32 effects, 12 sources)
> - research/archaosEffectSourceIntegration.md (ArKaos parity: 8 effects, 4 sources, 3 systems)
> - Original creative additions (25 effects, 25 sources)
>
> All deduplicated. Every phase is self-contained with exact instructions.

---

## How To Use

1. Open a new Claude Code session
2. Say: **"kick off phase 13"** (or 13.5, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25)
3. Claude reads CLAUDE.md → finds "kick off phase N" protocol → reads THIS file
4. Claude finds the phase section below, reads all listed files, executes all tasks
5. Claude self-validates: build passes, effects render, 60fps maintained
6. Claude reports to user with what to look for in the app

---

## Phase Status Tracker

| Phase | Description | Status | UI Validation Needed? |
|-------|-------------|--------|----------------------|
| **P13** | Effect Infrastructure (dry/wet, temporal, LUT, shared GLSL, new categories) | **COMPLETE** | YES — dry/wet slider visible on every effect, new categories in browsers |
| **P13.5** | Core Bug Fixes & Render Optimization (10 items, transitions moved to P14) | **COMPLETE** | YES — per-clip effects render, layer types work, tap tempo works |
| **P14** | Quick-Win Effects (20 shaders) + Transition Shaders (15) + UX fixes | **COMPLETE** | YES — 20 effects + 15 transitions in FX browser, FX delete, drag-drop inspectors, right-click reset, transport controls |
| **P15** | Medium Effects + Resolume Sources (24 items) | **COMPLETE** | YES — 14 effects + 12 sources visible |
| **P16** | Time Effects + Feedback System (10 items) | NOT STARTED | YES — time effects + feedback section in layer inspector |
| **P17** | Creative Sources: Math, 3D, Geometric, Nature, Lighting (19 sources) | NOT STARTED | YES — 19 new sources in browser |
| **P18** | Audio-Native Effects & Sources (20 items) | NOT STARTED | YES — audio-driven effects respond to music |
| **P19** | Complex Effects + Remaining Sources (20 items) | NOT STARTED | YES — complex effects + remaining sources visible |
| **P20** | Systems: Layer Router, Per-Type Automation, FFGL, Text, Simulations (19 items) | NOT STARTED | YES — layer router, FFGL plugins, text source |
| **P20.5** | projectM MilkDrop Visualizer Integration (10 items) | NOT STARTED | YES — MilkDrop presets render as source, audio-reactive, preset browser |

---

## Common Rules for ALL Phases (P13-P20)

### The 3-Step Process for Every New Effect

Every new effect follows this exact pattern. No exceptions.

**Step 1** — Write the GLSL fragment shader in `src/render/EmbeddedShaders.h`:
```cpp
static const char* myeffect_frag = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform sampler2D u_texture;
uniform float u_myeffect_param1;
uniform float u_myeffect_param2;
void main() {
    vec4 col = texture(u_texture, v_texCoord);
    // ... effect logic ...
    fragColor = col;
}
)";
```

**Step 2** — Register in `src/effects/EffectLibrary.cpp` inside `registerBuiltInEffects()`:
```cpp
registerEffect({"Display Name", "category", "myeffect", {
    {"param1", "u_myeffect_param1", 0.0f},
    {"param2", "u_myeffect_param2", 0.5f}
}});
```

**Step 3** — Compile in `src/render/Renderer.cpp` inside `compileEffectShaders()`:
```cpp
compileShader("myeffect", EmbeddedShaders::passthrough_vert, EmbeddedShaders::myeffect_frag);
```

That's it. The FX Browser auto-discovers the effect from its category string. The Inspector auto-generates parameter controls. Signal routing works on every parameter. Presets save/load automatically. No UI code per effect.

### The 3-Step Process for Every New Source

**Step 1** — Write the GLSL fragment shader in `src/render/EmbeddedShaders.h`:
```cpp
static const char* source_mysource_frag = R"(
#version 410 core
in vec2 v_texCoord;
out vec4 fragColor;
uniform float u_time;
uniform vec2 u_resolution;
uniform float u_rms;
uniform float u_bass;
uniform float u_mid;
uniform float u_high;
uniform float u_beatPhase;
uniform float u_spectralCentroid;
uniform float u_onsetStrength;
uniform float u_src_param1;
uniform float u_src_param2;
void main() {
    vec2 uv = v_texCoord;
    // ... source generation logic ...
    fragColor = vec4(color, 1.0);
}
)";
```

**Step 2** — Register in `src/sources/SourceRegistry.cpp` inside `registerSources()`:
```cpp
registerSource("mysource", [] {
    auto s = std::make_unique<ProceduralSource>();
    s->setName("Display Name");
    s->setShaderName("source_mysource");
    s->setCategory("Category");
    s->setStateful(false);
    s->addParam({"Param 1", "u_src_param1", 0.5f});
    s->addParam({"Param 2", "u_src_param2", 0.3f});
    return s;
});
```

**Step 3** — Compile in `src/render/Renderer.cpp` inside `compileSourceShaders()`:
```cpp
compileShader("source_mysource", EmbeddedShaders::passthrough_vert, EmbeddedShaders::source_mysource_frag);
```

Sources auto-appear in the Sources Browser by category. Stateful sources (ping-pong FBO) set `setStateful(true)`.

### Where to Find GLSL Implementation Details

Each phase lists which effects/sources to implement. The detailed GLSL code, parameter specs, uniform names, and audio mapping ideas are in:

| Document | What It Contains |
|----------|-----------------|
| `research/resolumeEffectSourceIntegration.md` Section 4 | 32 Resolume effects with full GLSL approach, params, uniforms |
| `research/resolumeEffectSourceIntegration.md` Section 6 | 12 Resolume sources with full specs |
| `research/resolumeEffectSourceIntegration.md` Section 10 | 25 original effects (OE1-OE25) + 25 original sources (OS1-OS25) |
| `research/archaosEffectSourceIntegration.md` Section 2 | 8 ArKaos effects with full GLSL |
| `research/archaosEffectSourceIntegration.md` Section 3 | 4 ArKaos sources |
| `research/archaosEffectSourceIntegration.md` Section 4 | Feedback Loop system architecture |
| `research/archaosEffectSourceIntegration.md` Section 5 | Per-Type Automation system |
| `research/archaosEffectSourceIntegration.md` Section 6 | FFGL Plugin Hosting system |
| `research/archaosEffectSourceIntegration.md` Section 10 | 34 creative expansion sources |

### Self-Validation Checklist (EVERY phase, no exceptions)

After completing all tasks in a phase, run this checklist in order:

1. **BUILD**: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` exits 0
2. **TESTS**: `cd build && ctest --output-on-failure` — ALL existing tests pass (113+ tests). If the phase adds new test cases, they pass too.
3. **LAUNCH**: App opens without crash, no GL errors in console
4. **NEW CONTENT**: Each new effect visible in FX browser under correct category. Each new source visible in Sources browser. Apply each to a test image and verify rendering + parameter sweep.
5. **PERFORMANCE**: FPS counter shows 60fps at 1080p with 3+ effects active simultaneously
6. **REGRESSION**: Spot-check 5 random EXISTING effects (Ripple, Hue Shift, Gaussian Blur, CRT, Kaleidoscope) — still work correctly
7. **REPORT**: Tell the user what changed and what to look for in the UI

### Test Files & What They Cover

| Test File | Tests | What It Validates |
|-----------|-------|------------------|
| `test_ring_buffer.cpp` | SPSC ring buffer | Audio pipeline integrity |
| `test_feature_bus.cpp` | Triple-buffer swap | Analysis → Render data flow |
| `test_smoother.cpp` | EMA + One-Euro | Signal smoothing math |
| `test_spectral_features.cpp` | Centroid, flux, flatness, rolloff, bands | Audio feature accuracy |
| `test_integration_pipeline.cpp` | End-to-end audio pipeline | Full analysis chain |
| `test_mapping_engine.cpp` | Source → curve → scale → target | Signal routing correctness |
| `test_bpm_stabilization.cpp` | Median, octave, hysteresis, phase | BPM lock reliability |
| `test_downbeat_detector.cpp` | Beat scoring, bar detection | Metrical hierarchy |
| `test_routing_engine.cpp` | Route processing | v2 routing system |
| `test_compositor.cpp` | Layer types, autopilot, deck ops | Compositing data model |
| `test_composition.cpp` | Save/load roundtrip | Serialization integrity |

### New Tests to Add Per Phase

| Phase | New Test File | What to Test |
|-------|--------------|-------------|
| **P13.5** | Add to `test_compositor.cpp` | Per-clip effect slot evaluation, FXOnly layer type data flow, Mask layer data flow, beat snap enforcement, crossfade progress advancement |
| **P14** | `test_effect_registration.cpp` (NEW) | Every new effect: name not empty, category valid, shader key not empty, param count > 0, defaults in [0,1] range |
| **P15** | Add to `test_effect_registration.cpp` | Same pattern for medium effects + source registration validation |
| **P16** | Add to `test_compositor.cpp` | FeedbackConfig serialization roundtrip, temporal effect flag |
| **P18** | `test_audio_effects.cpp` (NEW) | Audio-driven effects receive correct FeatureSnapshot uniforms |
| **P20** | `test_ffgl_host.cpp` (NEW) | Plugin directory scanning, metadata parsing |
| **P21** | `test_binding_modes.cpp` (NEW) | Piano mode state, velocity mapping, CC relative stepping |
| **P23** | `test_genre_detection.cpp` (NEW) | Genre classification accuracy against known feature vectors |

### What CANNOT be automatically tested (requires visual verification)

- Shader visual output quality (does the effect look right?)
- Performance under load (3+ effects + video + sources)
- UI layout and interaction (drag-drop, parameter sliders, browser navigation)
- Audio-visual synchronization (do visuals sync to beat?)

These are covered by the "UI validation" section at the end of each phase.

---

## Phase 13: Effect Infrastructure

**Goal**: Infrastructure that unlocks all future phases. Per-effect dry/wet, temporal buffer, LUT loading, shared GLSL utilities, new browser categories.

**Read first**: `CLAUDE.md`, this file (common rules above)

**Read before editing**:
- `src/effects/Effect.h` — Effect struct (add `dryWet` field here)
- `src/effects/EffectChain.h/cpp` — ping-pong FBO render chain (add dry/wet blend pass + `getPreviousFrameTexture()`)
- `src/render/EmbeddedShaders.h` — all shaders (add dry/wet composite shader + shared utility functions)
- `src/render/Renderer.cpp` — shader compilation (add new shaders to compile lists)
- `src/render/TextureManager.h/cpp` — texture loading (add `loadLUT()` method)
- `src/ui/ClipInspector.h/cpp` — parameter display (show dry/wet as first parameter per effect)
- `src/ui/FXBrowser.cpp` — effect categories (add 3 new categories)
- `src/ui/SourcesBrowser.cpp` — source categories (add 10 new categories)
- `src/effects/EffectLibrary.cpp` — registration pattern (reference only, don't change yet)

**Tasks**:

### P13.1 Per-Effect Dry/Wet Mix
- Add `float dryWet = 1.0f;` field to `Effect` struct in `Effect.h`
- Add a compositing shader `effect_drywet_frag` to `EmbeddedShaders.h`:
  ```glsl
  #version 410 core
  in vec2 v_texCoord;
  out vec4 fragColor;
  uniform sampler2D u_texture;    // effected
  uniform sampler2D u_original;   // pre-effect
  uniform float u_drywet;
  void main() {
      vec4 eff = texture(u_texture, v_texCoord);
      vec4 orig = texture(u_original, v_texCoord);
      fragColor = mix(orig, eff, u_drywet);
  }
  ```
- In `EffectChain::render()`, after rendering each effect: if `dryWet < 1.0`, run the dry/wet shader to blend the effected output with the pre-effect input (the pre-effect texture is the "read" FBO before the swap — it's already available)
- Compile the dry/wet shader in `Renderer::compileEffectShaders()`
- In `ClipInspector`, show dry/wet as the FIRST parameter slider of every effect (above effect-specific params), with a signal routing triangle like any other parameter

### P13.2 Temporal Buffer (Previous Frame Access)
- Add `GLuint getPreviousFrameTexture() const` method to `EffectChain` — returns the texture from the previous frame's render (the "other" FBO in the ping-pong pair that wasn't written to this frame)
- In `Renderer.cpp`, for effects registered with `temporal = true`: bind `u_prev_frame` uniform to this texture before rendering the effect
- Add a `bool temporal = false` field to the effect registration struct (EffectDef or similar)
- This is used by time-based effects in P16 (Ghost Trails, Frame Hold, Time Freeze, Feedback)

### P13.3 LUT Loading System
- Create `src/render/LUTLoader.h` and `src/render/LUTLoader.cpp`:
  - `static GLuint loadCubeFile(const juce::File& file)` — parses `.cube` format, returns GL_TEXTURE_3D handle
  - Format: skip lines starting with `#` or `TITLE`, read `LUT_3D_SIZE N`, then read N^3 lines of `R G B` floats, upload as `GL_TEXTURE_3D` with `GL_RGB32F`
- Add `GLuint loadLUT(const juce::File& file)` to `TextureManager`
- Add to `CMakeLists.txt` source list

### P13.4 Shared GLSL Utility Functions
- Add shared function blocks at the top of `EmbeddedShaders.h` as named string constants:
  ```cpp
  static const char* glsl_noise_functions = R"(
  float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
  float hash(float n) { return fract(sin(n) * 43758.5453); }
  vec2 hash2(vec2 p) { return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453); }
  float snoise(vec2 v) { /* simplex noise 2D */ }
  float snoise(vec3 v) { /* simplex noise 3D */ }
  float fbm(vec2 p) { float f=0.0; float w=0.5; for(int i=0;i<5;i++){f+=w*snoise(p);p*=2.0;w*=0.5;} return f; }
  )";

  static const char* glsl_util_functions = R"(
  vec2 rotateUV(vec2 uv, float angle) {
      float c = cos(angle), s = sin(angle);
      return mat2(c, -s, s, c) * uv;
  }
  vec3 hueRotate(vec3 col, float angle) {
      float c=cos(angle*6.28318), s=sin(angle*6.28318);
      mat3 m=mat3(0.299+0.701*c+0.168*s,0.587-0.587*c+0.330*s,0.114-0.114*c-0.497*s,
                  0.299-0.299*c-0.328*s,0.587+0.413*c+0.035*s,0.114-0.114*c+0.292*s,
                  0.299-0.299*c+1.250*s,0.587-0.587*c-1.050*s,0.114+0.886*c-0.203*s);
      return m*col;
  }
  )";

  static const char* glsl_sdf_functions = R"(
  float sdCircle(vec2 p, float r) { return length(p) - r; }
  float sdBox(vec2 p, vec2 b) { vec2 d=abs(p)-b; return length(max(d,0.0))+min(max(d.x,d.y),0.0); }
  float sdHexagon(vec2 p, float r) { vec2 q=abs(p); return max(q.x*0.866+q.y*0.5,q.y)-r; }
  float sdStar(vec2 p, float r, int n, float m) { /* star SDF */ }
  )";
  ```
- Shaders that need these include them by concatenating: `std::string(glsl_noise_functions) + shader_frag` during compilation in Renderer.cpp
- Add a helper in Renderer: `void compileShaderWithUtils(name, vert, frag, bool needsNoise, bool needsSDF, bool needsUtil)`

### P13.5 New Browser Categories
- In `src/ui/FXBrowser.cpp`, add to `buildCategoryList()`:
  ```cpp
  categories_.push_back({"time",      "Time",       juce::Colour(0xff00897b)});  // Teal
  categories_.push_back({"composite", "Composite",  juce::Colour(0xffec407a)});  // Pink
  categories_.push_back({"audio",     "Audio",      juce::Colour(0xffffd54f)});  // Gold
  ```
- In `src/ui/SourcesBrowser.cpp`, add to the categories list (after existing 6):
  ```cpp
  categories_.push_back({"Math",        juce::Colour(0xffab47bc)});
  categories_.push_back({"3D",          juce::Colour(0xffffca28)});
  categories_.push_back({"Organic",     juce::Colour(0xff66bb6a)});
  categories_.push_back({"Pattern",     juce::Colour(0xff4fc3f7)});
  categories_.push_back({"Particle",    juce::Colour(0xffff7043)});
  categories_.push_back({"Utility",     juce::Colour(0xff78909c)});
  categories_.push_back({"Lighting",    juce::Colour(0xffffd54f)});
  categories_.push_back({"Simulation",  juce::Colour(0xffef5350)});
  categories_.push_back({"Text",        juce::Colour(0xff8d6e63)});
  categories_.push_back({"Routing",     juce::Colour(0xff00897b)});
  ```

**UI validation**: Launch app. Open FX browser — should see 11 categories (8 existing + Time, Composite, Audio). Open Sources browser — should see 16 categories (6 existing + 10 new, all empty for now). Click any existing effect in inspector — dry/wet slider should appear as the first parameter.

---

## Phase 13.5: Core Bug Fixes & Render Optimization

**Goal**: Fix 11 broken/stubbed features found during deep dive analysis. These must work before adding 55+ new effects.

**Read first**: `CLAUDE.md`, this file

**Read before editing**:
- `src/render/CompositorEngine.h/cpp` — layer compositing (fix FX Only, Mask, 3D types + per-clip effects + layer transforms + clip transitions)
- `src/effects/EffectChain.h/cpp` — uniform caching, glFinish removal
- `src/render/Renderer.cpp` — glFinish removal, uniform upload optimization
- `src/render/ShaderManager.h/cpp` — uniform cache (exists but unused by EffectChain)
- `src/model/Autopilot.h/cpp` — clip triggering logic
- `src/MainComponent.cpp` — tap tempo + resync wiring (search for TODO comments)
- `src/model/Layer.h` — transition blend modes, layer transforms
- `src/model/Clip.h` — beatSnap enforcement

**Tasks**:

### P13.5.1 Per-Clip Effect Chain Rendering
- CompositorEngine currently only renders the global EffectChain
- Clip::effectSlots stores per-clip effects but they're never evaluated
- Fix: In `CompositorEngine::renderLayer()`, after getting the clip texture, run the clip's effect chain on it BEFORE the layer-level compositing
- This is critical — without it, per-clip effects (the primary use case) don't work

### P13.5.2 FX Only Layer Type
- `CompositorEngine.cpp:233` — Layer::Type::FXOnly is stubbed
- Fix: When layer type is FXOnly, apply the clip's effects to the ACCUMULATOR texture (the composited result so far) instead of to the clip's own media
- This turns the layer into an adjustment layer

### P13.5.3 Mask Layer Type
- `CompositorEngine.cpp:237` — Layer::Type::Mask is stubbed
- Fix: Render the clip as greyscale, use it as alpha mask on the accumulator
- Multiply accumulator alpha by mask luminance

### ~~P13.5.4 Transition Blend Mode Shaders~~ → MOVED TO P14.21-P14.35
- Transition shaders (15+ GLSL shaders for wipes, pushes, zooms, etc.) moved to P14 because they follow the same 3-step shader process as effects and fit naturally alongside the effects build.
- P13.5 focuses on core fixes only. P14 implements transitions after the compositing pipeline is fully working.

### P13.5.5 Layer Transform Application
- Layer.h has positionX/Y, layerScale, layerRotation, layerAnchorX/Y
- Never applied in rendering
- Fix: In CompositorEngine, after rendering the layer content, apply a transform shader (translate/scale/rotate around anchor) before compositing into the accumulator

### P13.5.6 Connect Tap Tempo
- `MainComponent.cpp:2617` — TODO comment, UI button exists but no hook
- Fix: Wire the tap button to `BPMTracker::tap()` or equivalent

### P13.5.7 Connect Resync
- `MainComponent.cpp:2621` — TODO comment, UI button exists but no hook
- Fix: Wire the resync button to `BPMTracker::resync()` (reset beat phase to 0)

### P13.5.8 Enforce Beat Snap
- Clip::beatSnap flag exists but is never checked in transport/triggering logic
- Fix: When triggering a clip with beatSnap=true, quantize the trigger to the next beat (or bar, per the global quantize setting)

### P13.5.9 Wire Autopilot Clip Triggering
- Autopilot::processFrame() doesn't actually trigger the next clip
- Fix: When autopilot timer expires, call the appropriate clip trigger method on the layer

### P13.5.10 Remove glFinish() — Use Async Timing
- `Renderer.cpp:299` — `glFinish()` forces GPU→CPU sync, stalls pipeline
- Fix: Remove `glFinish()`. Use `std::chrono::high_resolution_clock` for CPU-side frame timing. If GPU timing needed, use `GL_TIME_ELAPSED` query (async, no stall).

### P13.5.11 Cache Uniform Locations
- EffectChain calls `program->getUniformIDFromName()` for every parameter every frame
- ShaderManager already has a `uniformCache` map per program — it's just never used by EffectChain
- Fix: On first use of each uniform, cache the GLint location. Subsequent frames use the cached value.
- Impact: Eliminates ~78,600 string hash lookups/sec at 131 effects

**UI validation**: Load a clip with effects → effects should render on the clip (not just globally). Create an FX Only layer → should affect layers below. Create a Mask layer → should mask layers below. Click Tap Tempo → BPM should respond. Enable autopilot on a layer → clips should auto-advance. FPS should be stable at 60.

---

## Phase 14: Quick-Win Effects (20 simple shaders)

**Goal**: 20 new effects, all simple single-pass fragment shaders. No infrastructure dependencies beyond P13.

**Read first**: `CLAUDE.md`, this file (common rules + 3-step process)
**Read before editing**: `src/render/EmbeddedShaders.h`, `src/effects/EffectLibrary.cpp`, `src/render/Renderer.cpp`
**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Section 4 (for items marked "Resolume") and `research/archaosEffectSourceIntegration.md` Section 2 (for items marked "ArKaos")

**Tasks**: Implement ALL 20 effects using the 3-step process. For each effect, find its GLSL implementation in the doc referenced above.

| Task | Effect Name | Shader Key | Category | Detail In |
|------|------------|-----------|----------|-----------|
| P14.1 | Greyscale | `greyscale` | `color` | Resolume C1 |
| P14.2 | Threshold | `threshold` | `color` | Resolume C2 |
| P14.3 | Exposure | `exposure` | `color` | Resolume C3 |
| P14.4 | Vibrance | `vibrance` | `color` | Resolume C4 |
| P14.5 | Quad Mirror | `quad_mirror` | `warp` | Resolume W2 |
| P14.6 | Flip | `flip` | `warp` | Resolume W3 |
| P14.7 | Warp Field | `warp_field` | `warp` | Resolume W6 |
| P14.8 | Sharpen | `sharpen` | `blur` | Resolume B2 |
| P14.9 | Pixel Explosion | `pixel_explosion` | `glitch` | Resolume G3 |
| P14.10 | Color Flash | `color_flash` | `glitch` | Resolume G4 |
| P14.11 | Slide Wrap | `slide_wrap` | `warp` | Resolume R2 |
| P14.12 | Dot Field | `dot_field` | `3d` | Resolume D3 |
| P14.13 | Triangulate | `triangulate` | `pattern` | Resolume J2 |
| P14.14 | Auto Mask | `auto_mask` | `color` | Resolume J4 |
| P14.15 | Chroma Key | `chromakey_effect` | `color` | Resolume J5 |
| P14.16 | Tile Grid | `tile_grid` | `warp` | ArKaos 2.1 |
| P14.17 | Spot Zoom | `spot_zoom` | `warp` | ArKaos 2.6 |
| P14.18 | Neon Edge | `neon_edge` | `pattern` | ArKaos 2.7 |
| P14.19 | Cartoon Ink | `cartoon_ink` | `pattern` | ArKaos 2.8 |
| P14.20 | Pop Raster | `pop_raster` | `pattern` | ArKaos 2.4 |

### Transition Shaders (moved from P13.5.4)

These are clip-to-clip fade transitions — when one clip replaces another on the same layer, they smoothly crossfade (or wipe, push, etc.) over the transition duration. NOT the A/B bus DJ crossfader. Each uses `crossfadeProgress` (0→1) to blend between previous and new clip textures.

Register as transition shaders in EmbeddedShaders.h + compile in Renderer. CompositorEngine selects the correct shader based on `layer.transitionMode`.

| Task | Transition Name | Shader Key | GLSL Approach |
|------|----------------|-----------|---------------|
| P14.21 | Dissolve | `transition_dissolve` | `mix(prev, next, progress)` — simple opacity crossfade |
| P14.22 | Wipe Left | `transition_wipe_left` | `step(uv.x, progress)` selects prev or next |
| P14.23 | Wipe Right | `transition_wipe_right` | `step(1.0 - uv.x, progress)` |
| P14.24 | Wipe Up | `transition_wipe_up` | `step(1.0 - uv.y, progress)` |
| P14.25 | Wipe Down | `transition_wipe_down` | `step(uv.y, progress)` |
| P14.26 | Push Left | `transition_push_left` | Offset both UVs: prev slides out left, next slides in from right |
| P14.27 | Push Right | `transition_push_right` | Opposite of Push Left |
| P14.28 | Push Up | `transition_push_up` | Vertical push |
| P14.29 | Push Down | `transition_push_down` | Vertical push |
| P14.30 | Zoom In | `transition_zoom_in` | Next scales from center (0→1), prev fades |
| P14.31 | Zoom Out | `transition_zoom_out` | Prev scales out, next fades in |
| P14.32 | Iris Circle | `transition_iris` | Circular reveal: `smoothstep(progress, progress-0.05, length(uv-0.5))` |
| P14.33 | Flip Horizontal | `transition_flip_h` | First half: prev rotates to 90°. Second half: next rotates from 90° to 0° |
| P14.34 | Cut | `transition_cut` | Instant switch at progress=0.5 (no blend) |
| P14.35 | Fade to Black | `transition_fade_black` | First half: prev fades to black. Second half: black fades to next |

**UI validation**: Launch app. Open FX browser — 20 new effects visible. Trigger a clip on a layer that already has a clip playing with transition speed > 0 — verify smooth transition. Try changing transition mode in Layer Inspector — different visual transitions should appear.

---

## Phase 15: Medium Effects + Resolume Sources (24 items)

**Goal**: 12 medium-complexity effects + 12 new sources to reach Resolume source parity.

**Read first**: `CLAUDE.md`, this file
**Read before editing**: `src/render/EmbeddedShaders.h`, `src/effects/EffectLibrary.cpp`, `src/sources/SourceRegistry.cpp`, `src/render/Renderer.cpp`
**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Sections 4 and 6, `research/archaosEffectSourceIntegration.md` Section 2

**Tasks — Effects** (3-step process for each):

| Task | Effect Name | Shader Key | Category | Detail In |
|------|------------|-----------|----------|-----------|
| P15.1 | Palette Remap | `palette_remap` | `color` | Resolume C5 |
| P15.2 | Color Grade (LUT) | `lut_grade` | `color` | Resolume C6 |
| P15.3 | Bendoscope | `bendoscope` | `warp` | Resolume W1 |
| P15.4 | UV Remap | `uv_remap` | `warp` | Resolume W4 (needs noise utils) |
| P15.5 | Liquid Morph | `liquid_morph` | `warp` | Resolume W5 (needs noise utils) |
| P15.6 | Edge Blur | `edge_blur` | `blur` | Resolume B1 |
| P15.7 | Brush Strokes | `brush_strokes` | `pattern` | Resolume S1 |
| P15.8 | Fragment Burst | `fragment_burst` | `glitch` | Resolume G1 |
| P15.9 | Signal Destroy | `signal_destroy` | `glitch` | Resolume G5 |
| P15.10 | Line Cloner | `line_cloner` | `composite` | Resolume X1 |
| P15.11 | Radial Cloner | `radial_cloner` | `composite` | Resolume X2 |
| P15.12 | Cube Scatter | `cube_scatter` | `composite` | Resolume X3 |

**Tasks — Sources** (3-step source process for each):

| Task | Source Name | Source ID | Category | Detail In |
|------|-----------|----------|----------|-----------|
| P15.13 | Solid Color | `solid_color` | `Utility` | Resolume S1 |
| P15.14 | Strobe Light | `strobe_light` | `Utility` | Resolume S2 |
| P15.15 | Checkerboard | `checkerboard` | `Pattern` | Resolume S5 |
| P15.16 | Line Pattern | `line_pattern` | `Pattern` | Resolume S6 |
| P15.17 | Concentric Rings | `concentric_rings` | `Geometric` | Resolume S7 |
| P15.18 | Sine Oscillator | `sine_oscillator` | `Pattern` | Resolume S8 |
| P15.19 | Spiral Pattern | `spiral_pattern` | `Geometric` | Resolume S9 |
| P15.20 | Metaballs | `metaballs` | `Organic` | Resolume S10 |
| P15.21 | Terrain Lines | `terrain_lines` | `Geometric` | Resolume S11 |
| P15.22 | Shape Generator | `shape_generator` | `Geometric` | Resolume S4 (needs SDF utils) |
| P15.23 | Bump Light | `bump_light` | `pattern` | ArKaos 2.3 |
| P15.24 | Infinite Zoom | `infinite_zoom` | `warp` | ArKaos 2.2 |

**UI validation**: New effects in FX browser under their categories (first appearance of Composite category with 3 items). New sources in Sources browser (first appearance of Pattern, Organic, Utility categories). Each source generates animated visuals when loaded into a clip cell.

---

## Phase 16: Time Effects + Feedback System (10 items)

**Goal**: 5 temporal effects that use previous frame access + the Feedback Loop system (per-layer).

**Read first**: `CLAUDE.md`, this file
**Read before editing**:
- `src/effects/EffectChain.h/cpp` (temporal buffer from P13.2)
- `src/render/CompositorEngine.h/cpp` (feedback integrates here)
- `src/model/Layer.h/cpp` (add FeedbackConfig)
- `src/ui/LayerInspector.h/cpp` (add Feedback section)
- `src/render/EmbeddedShaders.h`, `src/effects/EffectLibrary.cpp`, `src/render/Renderer.cpp`

**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Section 4 (T1-T5, G2) and `research/archaosEffectSourceIntegration.md` Section 4 (Feedback Loop system)

**Tasks — Time Effects** (3-step process, all registered with `temporal = true`):

| Task | Effect Name | Shader Key | Category | Detail In |
|------|------------|-----------|----------|-----------|
| P16.1 | Ghost Trails | `ghost_trails` | `time` | Resolume T3 |
| P16.2 | Frame Hold | `frame_hold` | `time` | Resolume T5 |
| P16.3 | Time Freeze | `time_freeze` | `time` | Resolume G2 |
| P16.4 | Screen Split | `screen_split` | `glitch` | ArKaos 2.5 |
| P16.5 | Frame Delay | `frame_delay` | `time` | Resolume T1 |

**Tasks — Feedback Loop System** (from `archaosEffectSourceIntegration.md` Section 4):

| Task | Description | Files |
|------|-------------|-------|
| P16.6 | Create `FeedbackProcessor` class — owns an FBO pair, applies transform (zoom/rotate/hue shift) to previous frame, blends with current | `src/render/FeedbackProcessor.h/cpp` (NEW) |
| P16.7 | Add `FeedbackConfig` struct to Layer model: `bool enabled`, `float amount`, `float zoom`, `float rotation`, `float hueShift`, `float lumaKey` | `src/model/Layer.h/cpp` |
| P16.8 | Integrate into `CompositorEngine::renderLayer()` — after clip effects, before layer blend: if layer.feedback.enabled, run FeedbackProcessor | `src/render/CompositorEngine.h/cpp` |
| P16.9 | Add "Feedback" section to LayerInspector — 6 parameter sliders with signal routing triangles, plus a preset dropdown | `src/ui/LayerInspector.h/cpp` |
| P16.10 | Implement 6 feedback presets: Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch — each sets the 6 FeedbackConfig values to pre-defined combinations | `src/render/FeedbackProcessor.cpp` |

**Tasks — Signal Routing Integration (REQUIRED for signal tests to activate)**:

| Task | Description | Files |
|------|-------------|-------|
| P16.11 | Wire `RoutingEngine::processFrame()` into `Renderer::renderOpenGL()` — after FeatureSnapshot acquire, call `signalRegistry_.evaluateAll(snapshot)` then `routingEngine_.processFrame(signals, writer)` where writer sets effect params | `src/render/Renderer.h/cpp` |
| P16.12 | Pass `SignalRegistry&` and `RoutingEngine&` to TestServer constructor. Add 5 REST endpoints: `GET /api/signals` (list signals + cached values), `POST /api/add_route` (create route), `DELETE /api/remove_route/{id}`, `GET /api/routes` (list routes + current output), `POST /api/set_macro` | `src/test/TestServer.h/cpp`, `src/MainComponent.cpp` |
| P16.13 | Add `list_signals()`, `add_route()`, `remove_route()`, `list_routes()`, `set_macro()` to Python VJAppController | `tests/visual/vj_controller.py` |
| P16.14 | Remove `pytest.skip()` from `tests/visual/test_signals.py::TestSignalRouteEndToEnd` — all signal route tests should now pass | `tests/visual/test_signals.py` |

**Verification spec**: See `tests/visual/SIGNAL_TEST_SPEC.md` for full API endpoint specs, test coverage matrix, and integration prerequisites.

**UI validation**: Apply Ghost Trails to a video clip — should see motion persistence. Enable feedback on a layer with a clip + Tile Grid effect — should create infinite zoom tunnel. Feedback params should have signal routing triangles. Time category should appear in FX browser with 4-5 effects. Signal route tests (`pytest tests/visual/test_signals.py -v`) should all pass.

---

## Phase 17: Creative Sources — Math, 3D, Geometric, Nature, Lighting (19 sources)

**Goal**: 19 visually stunning procedural sources from the ArKaos creative expansion.

**Read first**: `CLAUDE.md`, this file
**Read before editing**: `src/render/EmbeddedShaders.h`, `src/sources/SourceRegistry.cpp`, `src/render/Renderer.cpp`
**Implementation detail**: `research/archaosEffectSourceIntegration.md` Section 10 (all categories)

**Tasks** (3-step source process for each):

| Task | Source Name | Source ID | Category | Detail In (Section 10) |
|------|-----------|----------|----------|----------------------|
| P17.1 | Lissajous Weaver | `lissajous_weaver` | `Math` | A.3 |
| P17.2 | Fermat Spiral Garden | `fermat_spiral` | `Math` | A.4 |
| P17.3 | Hyperbolic Tiling | `hyperbolic_tiling` | `Math` | A.2 |
| P17.4 | Penrose Pulse | `penrose_pulse` | `Math` | A.5 |
| P17.5 | Moire Interference | `moire_interference` | `Geometric` | C.4 |
| P17.6 | Crystal Cavern | `crystal_cavern` | `3D` | B.1 (ray-marched) |
| P17.7 | Infinite Corridor | `infinite_corridor` | `3D` | B.2 (ray-marched) |
| P17.8 | Orbit Chamber | `orbit_chamber` | `3D` | B.4 (ray-marched) |
| P17.9 | Astral Grid | `astral_grid` | `Geometric` | B.3 |
| P17.10 | Radial Burst | `radial_burst` | `Geometric` | C.2 |
| P17.11 | Hex Grid | `hex_grid` | `Geometric` | C.3 |
| P17.12 | Sacred Geometry | `sacred_geometry` | `Geometric` | C.6 |
| P17.13 | Fire Wall | `fire_wall` | `Nature` | F.1 |
| P17.14 | Water Caustics | `water_caustics` | `Nature` | F.2 |
| P17.15 | Electric Arc | `electric_arc` | `Nature` | F.4 |
| P17.16 | Laser Scanner | `laser_scanner` | `Lighting` | E.2 |
| P17.17 | Scroll Plane | `scroll_plane` | `3D` | ArKaos 3.1 |
| P17.18 | Rotating Cube Map | `rotating_cube_map` | `3D` | ArKaos 3.2 |
| P17.19 | Dual Plane Drift | `dual_plane_drift` | `3D` | ArKaos 3.3 |

**Note**: Ray-marched sources (Crystal Cavern, Infinite Corridor, Orbit Chamber) must stay under 3ms at 1080p. Limit ray march iterations if needed.

**UI validation**: All 19 sources visible in browser. Math, 3D, Geometric, Nature, Lighting categories populated. Each source generates animated visuals. Ray-marched sources maintain 60fps.

---

## Phase 18: Audio-Native Effects & Sources (20 items)

**Goal**: Effects and sources that directly exploit our 42+ audio features. THIS IS OUR DIFFERENTIATOR — none of these are possible in Resolume or ArKaos.

**Read first**: `CLAUDE.md`, this file
**Read before editing**: `src/render/EmbeddedShaders.h`, `src/effects/EffectLibrary.cpp`, `src/sources/SourceRegistry.cpp`, `src/render/Renderer.cpp`, `src/analysis/FeatureSnapshot.h` (for uniform names)
**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Section 10 (OE1-OE10) and `research/archaosEffectSourceIntegration.md` Section 10 Category D

**Important**: These effects need additional audio uniforms beyond the standard set. Check what `FeatureSnapshot` fields are available and upload the ones each effect needs. Effects receive uniforms via `UniformBridge`. Sources receive them via `ProceduralSource::uploadUniforms()`.

**Tasks — Audio-Driven Effects** (3-step process):

| Task | Effect Name | Shader Key | Category | Audio Features | Detail In |
|------|------------|-----------|----------|---------------|-----------|
| P18.1 | Harmonic Displacement | `harmonic_displace` | `audio` | chromagram[12], HCDF | OE2 |
| P18.2 | Timbral Mosaic | `timbral_mosaic` | `audio` | mfccs[13] | OE4 |
| P18.3 | Structural Morph | `structural_morph` | `audio` | structuralState | OE5 |
| P18.4 | Pitch Chromatic Shift | `pitch_chroma_shift` | `color` | dominantPitch, detectedKey | OE6 |
| P18.5 | Key Palette | `key_palette` | `color` | detectedKey, keyIsMajor | OE9 |
| P18.6 | Transient Flash | `transient_flash` | `animation` | onsetDetected, onsetStrength | OE10 |
| P18.7 | Beat Ripple | `beat_ripple` | `audio` | onsetDetected, beatPhase | OE3 |
| P18.8 | Rhythm Slice | `rhythm_slice` | `glitch` | beatPhase, barPhase, phrasePhase | OE8 |
| P18.9 | Density Wave | `density_wave` | `warp` | beatPhase, rms | OE7 |
| P18.10 | Chroma Dissolve | `chroma_dissolve` | `color` | chromagram[12] | OE16 |

**Tasks — Audio-Native Sources** (3-step source process):

| Task | Source Name | Source ID | Category | Audio Features | Detail In |
|------|-----------|----------|----------|---------------|-----------|
| P18.11 | Spectrum Landscape | `spectrum_landscape` | `Audio-Visual` | bandEnergies[7], spectralCentroid | D.1 |
| P18.12 | Chromatic Ring | `chromatic_ring` | `Audio-Visual` | chromagram[12], detectedKey | D.2 |
| P18.13 | Band Tower | `band_tower` | `Audio-Visual` | bandEnergies[7], spectralCentroid | D.3 |
| P18.14 | Timbral Nebula | `timbral_nebula` | `Audio-Visual` | mfccs[13], spectralCentroid | D.4 |
| P18.15 | Structural Landscape | `structural_landscape` | `Audio-Visual` | structuralState, rms, flux, bpm | D.5 |
| P18.16 | Cymatics | `cymatics` | `Audio-Visual` | dominantPitch, pitchConfidence | OS1 |
| P18.17 | Spectral Waterfall | `spectral_waterfall` | `Audio-Visual` | bandEnergies[7] (stateful) | OS2 |
| P18.18 | Spectral Ring | `spectral_ring` | `Audio-Visual` | bandEnergies[7], beatPhase | OS3 |

**Pre-work for text sources**:

| Task | Description |
|------|-------------|
| P18.19 | Create bitmap font texture atlas: 8x8 pixel grid, 128 ASCII chars, embed as static byte array in `EmbeddedShaders.h`. Upload as GL_TEXTURE_2D in Renderer. |
| P18.20 | Scrolling Text Wall source (`text_wall`) — uses bitmap font to render scrolling text. Category: `Text`. Detail in ArKaos G.1. |

**UI validation**: Play music. Harmonic Displacement visibly changes when chords change. Key Palette auto-selects warm/cool colors based on major/minor key. Cymatics pattern changes with pitch. Spectral Waterfall scrolls showing frequency content over time. Audio category in FX browser shows 3 effects.

---

## Phase 19: Complex Effects + Remaining Sources (20 items)

**Goal**: All remaining effects and sources that aren't audio-native or system-level.

**Read first**: `CLAUDE.md`, this file
**Read before editing**: `src/render/EmbeddedShaders.h`, `src/effects/EffectLibrary.cpp`, `src/sources/SourceRegistry.cpp`, `src/render/Renderer.cpp`
**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Sections 4 and 10

**Tasks — Complex Effects** (3-step process):

| Task | Effect Name | Shader Key | Category | Detail In |
|------|------------|-----------|----------|-----------|
| P19.1 | Luminance Terrain | `luma_terrain` | `3d` | Resolume D1 |
| P19.2 | Voxel Matrix | `voxel_matrix` | `3d` | Resolume D2 |
| P19.3 | Drop Shadow | `drop_shadow` | `blur` | Resolume J3 |
| P19.4 | Monitor Wall | `monitor_wall` | `pattern` | Resolume J1 |
| P19.5 | Channel Delay | `channel_delay` | `time` | Resolume T2 (temporal=true) |
| P19.6 | Data Corruption | `data_corruption` | `glitch` | OE22 |
| P19.7 | Glitch Sort | `glitch_sort` | `glitch` | OE24 |
| P19.8 | Topographic Lines | `topo_lines` | `pattern` | OE20 |

**Tasks — Remaining Sources** (3-step source process):

| Task | Source Name | Source ID | Category | Detail In |
|------|-----------|----------|----------|-----------|
| P19.9 | Superformula | `superformula` | `Math` | OS6 |
| P19.10 | Truchet Labyrinth | `truchet_labyrinth` | `Math` | OS8 |
| P19.11 | Rose Curves | `rose_curves` | `Math` | OS25 |
| P19.12 | Fibonacci Spiral | `fibonacci_spiral` | `Math` | OS21 |
| P19.13 | Lightning Storm | `lightning_storm` | `Particle` | OS11 |
| P19.14 | Fire | `fire` | `Nature` (stateful) | OS12 |
| P19.15 | Starfield | `starfield` | `Particle` | OS14 |
| P19.16 | Particle Nebula | `particle_nebula` | `Particle` | OS10 |
| P19.17 | Radar Sweep | `radar_sweep` | `Geometric` | OS22 |
| P19.18 | Glitch Grid | `glitch_grid` | `Pattern` | OS24 |
| P19.19 | DNA Helix | `dna_helix` | `3D` | OS20 |
| P19.20 | Dot Matrix Wave | `dot_matrix_wave` | `Geometric` | ArKaos C.5 |

**UI validation**: All new effects and sources visible in browsers. Complex effects (Luminance Terrain, Voxel Matrix) create visible 3D-like distortions. All sources generate animated visuals at 60fps.

---

## Phase 20: Systems + Infrastructure Features

**Goal**: Major system features — Layer Router, Per-Type Automation, FFGL Plugin Hosting, Text Animator, Stateful Simulations.

**Read first**: `CLAUDE.md`, this file
**Read before editing**: Listed per task below
**Implementation detail**: `research/resolumeEffectSourceIntegration.md` Section 6 (S3, S12) and `research/archaosEffectSourceIntegration.md` Sections 4, 5, 6, 10

**This is the largest phase — can be split across multiple sessions.**

### P20 Part A: Layer Router Source
**Read**: `src/render/Renderer.cpp`, `src/render/CompositorEngine.h/cpp`, `src/sources/SourceRegistry.cpp`

| Task | Description | Files |
|------|-------------|-------|
| P20.1 | Create `RouterSource` class extending `ProceduralSource` — holds source layer index, `getTexture()` returns saved output from that layer | `src/sources/RouterSource.h/cpp` (NEW) |
| P20.2 | In `CompositorEngine::render()`, save each layer's rendered output texture to an indexed array before compositing | `src/render/CompositorEngine.h/cpp` |
| P20.3 | Register Layer Router in `SourceRegistry` with a `sourceLayer` parameter | `src/sources/SourceRegistry.cpp` |
| P20.4 | Self-reference safety: prevent layer routing to itself, detect and break circular references | `src/sources/RouterSource.cpp` |

### P20 Part B: Per-Type Automation
**Read**: `src/model/Composition.h`, `src/model/Autopilot.h/cpp`, `src/ui/CompositionInspector.h/cpp`
**Detail**: `archaosEffectSourceIntegration.md` Section 5

| Task | Description | Files |
|------|-------------|-------|
| P20.5 | Add `PerTypeAutopilotConfig` to Composition model — separate timers for Opaque, Transparent, and FX layers | `src/model/Composition.h` |
| P20.6 | Refactor `Autopilot::advance()` to track per-type state independently | `src/model/Autopilot.h/cpp` |
| P20.7 | Add per-type sections to CompositionInspector UI | `src/ui/CompositionInspector.h/cpp` |

### P20 Part C: FFGL Plugin Hosting
**Read**: `src/effects/EffectLibrary.h/cpp`, `src/ui/FXBrowser.h/cpp`, `src/ui/PreferencesDialog.h/cpp`
**Detail**: `archaosEffectSourceIntegration.md` Section 6

| Task | Description | Files |
|------|-------------|-------|
| P20.8 | Create `FFGLHost` — scans directories for .bundle/.dll plugins, probes each for metadata (name, params, type) | `src/plugins/FFGLHost.h/cpp` (NEW) |
| P20.9 | Create `FFGLPlugin` — manages single plugin instance lifecycle, param get/set, processGL() | `src/plugins/FFGLPlugin.h/cpp` (NEW) |
| P20.10 | Create `FFGLEffect` — adapts FFGLPlugin to our Effect interface so it works in EffectChain | `src/plugins/FFGLEffect.h/cpp` (NEW) |
| P20.11 | Add "Plugins" category to FX Browser — shows discovered FFGL plugins | `src/ui/FXBrowser.h/cpp` |
| P20.12 | Add plugin preferences — scan directories, rescan button, blacklist | `src/ui/PreferencesDialog.h/cpp` |
| P20.13 | GL state save/restore + crash isolation — `glPushAttrib`/`glPopAttrib` wrapper, try/catch, timeout, auto-disable on failure | `src/plugins/FFGLPlugin.cpp` |
| P20.14 | Update `CMakeLists.txt` for dynamic loader libs (`dlopen` on macOS/Linux, `LoadLibrary` on Windows) | `CMakeLists.txt` |

### P20 Part D: Text Animator Source
**Read**: `src/sources/SourceRegistry.cpp`, `src/render/Renderer.cpp`
**Detail**: `resolumeEffectSourceIntegration.md` Section 6, S3

| Task | Description | Files |
|------|-------------|-------|
| P20.15 | Create `TextSource` extending `ProceduralSource` — rasterizes text to `juce::Image` via `Graphics::drawText()`, uploads to GL texture | `src/sources/TextSource.h/cpp` (NEW) |
| P20.16 | Fragment shader for text animation — scroll, pulse, wave, typewriter modes applied to the rasterized texture | `src/render/EmbeddedShaders.h` |
| P20.17 | Register Text Animator in SourceRegistry — string param for text content, float params for size/color/animation | `src/sources/SourceRegistry.cpp` |

### P20 Part E: Stateful Simulation Sources
**Read**: `src/sources/SourceRegistry.cpp` (see Reaction-Diffusion and Cellular Automata for stateful source pattern)
**Detail**: `archaosEffectSourceIntegration.md` Section 10, A.1 + H.1 + H.2

| Task | Source Name | Source ID | Category | FBOs | Detail In |
|------|-----------|----------|----------|------|-----------|
| P20.18 | Strange Attractor Field | `strange_attractor` | `Simulation` | 1 | A.1 |
| P20.19 | Gravity Well | `gravity_well` | `Simulation` | 1 | H.1 |
| P20.20 | Fluid Dynamics | `fluid_dynamics` | `Simulation` | 3 | H.2 (most complex — may need 512x512 internal resolution) |

**UI validation**: Layer Router: set one layer to use Layer Router source pointing at another layer — should see feedback loop. FFGL: scan a directory with FFGL plugins — they should appear in FX browser. Text Animator: load in clip cell, type text — should render animated text. Fluid Dynamics: play music — onsets should create splashes.

---

## Phase 20.5: projectM MilkDrop Visualizer Integration (10 items)

**Goal**: Integrate libprojectM as a ProceduralSource, giving users access to 10,000+ MilkDrop audio-reactive visualizer presets. Our 42-feature audio analysis drives intelligent preset selection that no other projectM host can match.

**Read first**: `CLAUDE.md`, this file (common rules)
**Read before editing**:
- `src/sources/ProceduralSource.h/cpp` — base class to subclass
- `src/sources/SourceRegistry.cpp` — registration pattern
- `src/render/Renderer.cpp` — `renderSource()` method, audio feature flow
- `src/render/CompositorEngine.cpp` — how source textures enter the compositor
- `src/analysis/FeatureSnapshot.h` — available audio features for preset selection
- `src/ui/SourcesBrowser.cpp` — category display pattern
- `src/ui/PreferencesDialog.h/cpp` — directory preferences pattern (from FFGL in P20)

**Dependencies**: P20 (reuses FFGL GL state isolation pattern and directory preferences UI)

**Library**: libprojectM-4 (LGPL 2.1, C API)
- API: `projectm_create()`, `projectm_pcm_add_float()`, `projectm_opengl_render_frame_fbo()`, `projectm_load_preset_file()`, `projectm_destroy()`
- Renders to any FBO via `projectm_opengl_render_frame_fbo(handle, fboId)`
- Accepts raw float PCM via `projectm_pcm_add_float(handle, samples, count, PROJECTM_MONO)`
- OpenGL 3.3+ (compatible with our 4.1 target)
- Install: `brew install libprojectm` on macOS

**Architecture**: projectM becomes a `ProceduralSource` subclass. It generates a complete texture from audio (not an effect — it doesn't transform existing content). As a source, it gets deck placement, effect chain, layer compositing, blend modes, transforms, and signal routing for free.

**Audio strategy**: Two layers:
1. Feed raw PCM to projectM (let it use its own internal FFT/beat detection — every preset works unmodified)
2. Use our `FeatureSnapshot` for smart preset selection (structural transitions, BPM-synced switching, energy matching)

**Tasks**:

| Task | Description | Files |
|------|-------------|-------|
| P20.5.1 | Add libprojectm-4 dependency via Homebrew `find_package` + fallback FetchContent | `CMakeLists.txt`, `cmake/FindProjectM.cmake` (NEW) |
| P20.5.2 | Create `ProjectMSource` extending `ProceduralSource` — manages `projectm_handle`, overrides `initGL()` (create instance + configure), `render()` (feed audio + render to FBO), `releaseGL()` (destroy instance) | `src/sources/ProjectMSource.h/cpp` (NEW) |
| P20.5.3 | GL state save/restore wrapper around `projectm_opengl_render_frame_fbo()` — save blend, depth, stencil, viewport, shader, VAO; call projectM; restore all. Reuse pattern from FFGL (P20.13) | `src/sources/ProjectMSource.cpp` |
| P20.5.4 | PCM audio feed: in `Renderer::renderSource()`, when source is projectM, copy latest 512 samples from analysis buffer and call `projectm_pcm_add_float()` before rendering | `src/sources/ProjectMSource.cpp`, `src/render/Renderer.cpp` |
| P20.5.5 | `ProjectMPresetManager` class — scans directories for `.milk` files, stores list (name, path, mood tag, energy tag), supports load/next/prev/random, preset locking, favorites (starred) | `src/sources/ProjectMPresetManager.h/cpp` (NEW) |
| P20.5.6 | Register `projectm_visualizer` in `SourceRegistry` with params: Preset Index (int), Beat Sensitivity [0,1], Speed [0,1], Warp Amount [0,1], Decay [0,1], Gamma [0,1] | `src/sources/SourceRegistry.cpp` |
| P20.5.7 | Create `MilkDropBrowser` — dedicated browser tab component (6th tab in BrowserPanel). Layout: search bar at top, collapsible mood/style folders (Calm, Energetic, Psychedelic, Geometric, Dark, Minimal), "User Presets" section for presets created/customized in our app, "Favorites" section (starred presets), Auto-DJ controls at bottom (enable/disable smart transitions, energy matching toggle, transition duration). Follow same dark VJ style as FXBrowser/SourcesBrowser. Click preset to load into active clip, drag preset onto deck cell. | `src/ui/MilkDropBrowser.h/cpp` (NEW) |
| P20.5.7b | Add "MilkDrop" tab to `BrowserPanel` — 6th tab after Record. Add `Tab::MilkDrop = 5` to enum, add tab button, wire up `MilkDropBrowser` component. Browser becomes: Files / FX / Sources / Comp-Decks / Record / MilkDrop | `src/ui/BrowserPanel.h/cpp` |
| P20.5.8 | `PresetSelector` helper — reads `FeatureSnapshot`, auto-switches presets: high-energy on `structuralState==DROP`, calm on `BREAKDOWN`, transitions synced to `barPhase` beat 1. Respects mood filter from MilkDropBrowser. | `src/sources/PresetSelector.h/cpp` (NEW) |
| P20.5.9 | Preferences UI: projectM preset directory paths (add/remove), auto-transition enable/disable, transition duration slider, beat sensitivity | `src/ui/PreferencesDialog.h/cpp` |
| P20.5.10 | Bundle 50-100 curated presets from Cream of the Crop collection, pre-tagged with mood/energy metadata in a `presets.json` manifest | `resources/projectm_presets/` |

**MilkDrop Browser Tab Layout**:
```
┌─────────────────────────────────┐
│ 🔍 Search presets...            │  ← Filter by name
├─────────────────────────────────┤
│ ★ Favorites (12)             ▼  │  ← Starred presets (collapsible)
│   Geiss - Soft Flower           │
│   Rovastar - Cosmic Dust        │
│   ...                           │
├─────────────────────────────────┤
│ 👤 User Presets (3)          ▼  │  ← Created/customized in app
│   My Chill Preset               │
│   ...                           │
├─────────────────────────────────┤
│ 🌊 Calm (24)                 ▼  │  ← Mood folders (collapsible)
│ ⚡ Energetic (31)            ▼  │
│ 🌀 Psychedelic (18)          ▼  │
│ 🔷 Geometric (15)            ▼  │
│ 🌑 Dark (9)                  ▼  │
│ ◻️ Minimal (7)               ▼  │
│ 📁 All Presets (104)         ▼  │  ← Unfiltered full list
├─────────────────────────────────┤
│ Auto-DJ  [ON]  Energy [Match]   │  ← Smart transition controls
│ Transition: ████░░ 4 bars       │
└─────────────────────────────────┘
```

**Technical notes**:

- projectM renders its own feedback loop internally (warps previous frame + overlays shapes). Do NOT try to replicate this with our P16 feedback system — they are independent.
- Do NOT replace projectM's internal audio analysis with our FeatureSnapshot. Presets depend on projectM's specific `bass`/`mid`/`treb` variables. Our features are for preset *selection*, not preset *modification*.
- MilkDrop presets take 1-3ms at 1080p — within the existing render budget alongside our effects.
- Preset changes via `projectm_load_preset_file()` are thread-safe in libprojectm 4.x.
- The MilkDrop tab is a full browser panel (like FXBrowser), NOT a category inside SourcesBrowser. This is because 100+ presets (scaling to 10,000+) needs its own search, mood filtering, favorites, and Auto-DJ controls that don't belong in the source category list.

**UI validation**: Play music. Click "MilkDrop" tab in Browser → see mood folders with presets. Search for a preset by name → filtered list. Click a preset → animated MilkDrop visualizer renders in preview, reacting to audio. Star a preset → appears in Favorites section. Apply Hue Shift effect on top → effect chain works on projectM output. Place in deck cell → composites with other layers. Enable Auto-DJ → presets change on structural transitions, synced to bar boundaries. 60fps maintained.

---

## Phase 21: Live Performance Controls

**Goal**: Essential controls for live VJ performance.

**Read first**: `CLAUDE.md`, this file, `memory/potentialImprovements.md` (Tier 1 items)

**Tasks**:

| Task | Feature | Description |
|------|---------|-------------|
| P21.1 | Piano/Momentary keyboard mode | Key held = active, release = off. For keyboard bindings. |
| P21.2 | Piano/Momentary MIDI mode | Same for MIDI notes. Essential for pad controllers. |
| P21.3 | MIDI Velocity mode | Pad force (0-127) maps to parameter value |
| P21.4 | MIDI CC Relative mode | Endless rotary encoders with step size and loop |
| P21.5 | 3 Targeting Modes | By Position (survives reorder), This Item (follows specific clip), Selected (current selection) |
| P21.6 | Persistent clips across deck switches | Clips marked persistent keep playing when deck changes |
| P21.7 | Non-interrupting deck switches | Deck switch never stops currently-playing clips |
| P21.8 | Ableton Link integration | Open-source network tempo sync protocol |
| P21.9 | Right-click reset to default | Any parameter resets on right-click |
| P21.10 | Per-clip Beat Snap granularity | Beat/bar/2-bar/4-bar per clip (not just global bool) |

---

## Phase 22: Output & Integration

**Goal**: Professional output and integration features.

**Tasks**:

| Task | Feature | Description |
|------|---------|-------------|
| P22.1 | Syphon output (macOS) | Inter-app GPU texture sharing |
| P22.2 | Spout output (Windows) | Inter-app GPU texture sharing |
| P22.3 | Syphon/Spout input | Receive textures from other apps |
| P22.4 | NDI output | Network video output |
| P22.5 | NDI input | Network video input |
| P22.6 | Video recording | Real-time capture of composition output with codec selection |
| P22.7 | Snapshot (PNG capture) | Single frame export, auto-import to clip slot |
| P22.8 | Basic REST API | Get/set parameters, trigger clips from HTTP |
| P22.9 | OSC input support | Float precision control from TouchOSC, Max/MSP, etc. |
| P22.10 | MIDI Output / pad feedback | 5 clip state colors for Launchpad/APC controllers |

---

## Phase 23: Smart Audio Features

**Goal**: Innovative audio-driven features that differentiate Audio-DNA.

**Tasks**:

| Task | Feature | Description |
|------|---------|-------------|
| P23.1 | Genre detection | Real-time genre classification from audio features (8 genres) |
| P23.2 | Auto-preset selection | Genre change triggers automatic visual preset switch |
| P23.3 | AI mapping suggestions | Analyze audio → suggest signal-to-parameter routes |
| P23.4 | Smart Random autopilot | Energy state → intelligent clip selection (buildup=escalate, drop=intense, breakdown=calm) |
| P23.5 | Structural scene triggering | Auto-switch decks on buildup→drop→breakdown |
| P23.6 | ISF shader import | Import community GLSL shaders from isf.video |
| P23.7 | Per-genre FFT tuning | Auto-adjust FFT size, hop size, envelope attack/release per genre |
| P23.8 | Smart BPM recovery | Hold last good BPM during silence/speech, resume gracefully |

---

## Phase 24: Workflow Polish

**Goal**: Professional workflow features for daily use.

**Tasks**:

| Task | Feature | Description |
|------|---------|-------------|
| P24.1 | 21 easing functions | Elastic, Back, Bounce, Circular × In/Out/InOut + Hold |
| P24.2 | Signal chaining | Envelope shapes another signal (Bass → Envelope → Param) |
| P24.3 | Clip Position signal source | Parameter tracks video playhead position |
| P24.4 | Content replacement | Swap media while keeping effects/mappings |
| P24.5 | Lock Content | Prevent accidental clip replacement on layer |
| P24.6 | Layout presets | Save/load panel arrangements |
| P24.7 | Missing file indicators + relocate | Red markers + intelligent sibling file discovery |
| P24.8 | Collect Media | Package composition + all files for portability |
| P24.9 | Undo improvements | Auto-navigate to affected item, action description in toolbar |
| P24.10 | Binding presets | Export/import keyboard+MIDI bindings separately |
| P24.11 | Clip panel update lock | Prevent inspector auto-switching during MIDI performance |
| P24.12 | Layer folding | Collapse/expand layers to save space |
| P24.13 | Layer drag reorder | Drag name handles to reorder |

---

## Phase 25: Advanced Audio Analysis

**Goal**: Deep audio analysis features for genre-specific and advanced mapping.

**Tasks**:

| Task | Feature | Description |
|------|---------|-------------|
| P25.1 | Sidechain detection | Cross-correlation bass+mid for techno pump effect |
| P25.2 | Swing detection | Inter-onset histogram → swing ratio for hip hop groove |
| P25.3 | Formant tracking | Detect vocal presence via spectral flatness in 300-3000 Hz |
| P25.4 | Resonance peak tracking | Filter resonance via spectral kurtosis (200-8000 Hz) |
| P25.5 | Reese bass detection | Spectral spread in 30-200 Hz for DnB reese bass |
| P25.6 | Audio stem separation | ML models (Demucs) → separate drums/bass/vocals/other |
| P25.7 | Composition-level transform | Scale/rotate/position entire output for projection mapping |
| P25.8 | Cross-deck transitions | Blend mode for deck changes |

---

## Dependency Graph

```
P13 (Infrastructure) ──► COMPLETE
   │
   ├──► P13.5 (Bug Fixes) ──► STRONGLY RECOMMENDED before P14
   │
   ├──► P14 (Quick Effects)
   ├──► P15 (Medium Effects + Sources) ──── needs P13.3 for LUT, P13.4 for noise
   ├──► P16 (Time Effects + Feedback) ──── needs P13.2 for temporal buffer
   ├──► P17 (Creative Sources) ──────────── needs P13.4 for noise/SDF
   ├──► P18 (Audio-Native) ──────────────── needs FeatureSnapshot uniforms
   ├──► P19 (Complex + Remaining)
   └──► P20 (Systems)

P14 through P19 have NO dependencies on each other — can run in any order after P13.
P20 can run after P16 (feedback system must exist for Layer Router testing).

P21 through P25 can run in any order after P20.
P21 (Live Performance) and P22 (Output) are independent.
P23 (Smart Audio) needs P25 for some features.
P24 (Workflow) is independent.
```

## Master Schedule

| Phase | Sessions | Delivers | Running Total |
|-------|----------|----------|---------------|
| **P13** | ~~1-2~~ | ~~infra~~ | **COMPLETE** |
| **P13.5** | 2-3 | 11 bug fixes + render optimization | 76 FX, 10 src, +fixes |
| **P14** | 2 | 20 simple effects | **96** FX, 10 src |
| **P15** | 2-3 | 12 effects + 12 sources | **108** FX, **22** src |
| **P16** | 2-3 | 5 time effects + feedback system | **113** FX, 22 src, +feedback |
| **P17** | 2-3 | 19 creative sources | 113 FX, **41** src |
| **P18** | 2-3 | 10 audio effects + 10 audio sources | **123** FX, **51** src |
| **P19** | 2-3 | 8 complex effects + 12 sources | **131** FX, **63** src |
| **P20** | 4-6 | 3 systems + 5 sources | 131 FX, **68** src, +3 systems |
| **P21** | 2-3 | 10 live performance controls | +live performance |
| **P22** | 3-4 | 10 output/integration features | +Syphon, NDI, recording, API |
| **P23** | 2-3 | 8 smart audio features | +genre detect, ISF, smart autopilot |
| **P24** | 2-3 | 13 workflow polish features | +easing, signal chain, layout presets |
| **P25** | 2-3 | 8 advanced audio analysis features | +sidechain, stems, formant |
| **Total** | **~30-40** | | **131 FX + 68 src + 8 systems + 59 features** |
