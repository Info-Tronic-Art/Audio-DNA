# Domain Glossary — Audio-DNA

## Audio Analysis

**FeatureSnapshot**: Fixed-size POD struct (alignas(64)) containing all extracted audio features for one analysis hop. The unit of transfer between Analysis and Render threads via triple-buffer atomic swap.
_Avoid_: "audio data", "analysis result" — too vague.

**Hop**: One analysis window step (512 samples = 10.7ms at 48kHz). Each hop triggers the full 16-stage analysis pipeline.
_Avoid_: "frame" when referring to audio — "frame" means a visual render frame.

**SPSC Ring Buffer**: Single-producer single-consumer lock-free circular buffer (16384 floats, ~341ms at 48kHz). Carries raw audio from the audio callback to the analysis thread. Power-of-two sized, cache-line padded.
_Avoid_: "queue", "audio buffer" — too generic.

**Triple Buffer**: Three FeatureSnapshot instances with atomic index rotation. Writer publishes to the next slot; reader always gets the latest complete snapshot. Zero contention, zero blocking.
_Avoid_: "double buffer" — different mechanism.

**Spectral Centroid**: Weighted average frequency of the magnitude spectrum. Higher = brighter sound. Measured in Hz.

**Spectral Flux**: Half-wave rectified frame-to-frame magnitude difference. Measures how fast the spectrum is changing. Normalized per-session.

**Spectral Flatness**: Geometric mean / arithmetic mean of magnitudes. 0 = pure tone, 1 = white noise. Also called Wiener entropy.

**MFCC**: Mel-Frequency Cepstral Coefficients. 13 coefficients derived from a 40-band mel filterbank + DCT. Captures timbral fingerprint.

**Chromagram**: 12-element array mapping FFT energy to pitch classes (C through B). Sum = 1. Used for key detection.

**HCDF**: Harmonic Change Detection Function. Euclidean distance between consecutive chromagram frames. Spikes at chord changes.

**Structural State**: State machine output (0=normal, 1=buildup, 2=drop, 3=breakdown). Derived from multi-scale EMA envelope comparison (100ms/1s/4s/16s).

**BPM Tracker**: Aubio-based tempo estimation. Includes stabilization pipeline: range gate → confidence filter → octave correction → median filter → hysteresis.

**Beat Phase**: Sawtooth wave [0, 1) synchronized to detected beats. Resets to 0 on each beat.

**Bar Phase**: (beatInBar + beatPhase) / 4. Sawtooth over 4 beats.

**Phrase Phase**: Sawtooth over N bars (default 8), resets on structural transitions.

## Visual System

**Effect**: A named GLSL fragment shader with typed parameters (all normalized to [0, 1]). Applied to an image texture on a fullscreen quad. 135 effects across 11 categories.
_Avoid_: "filter" — too generic. "Shader" is acceptable when referring to the GLSL code specifically.

**Effect Chain**: Ordered list of effects rendered via ping-pong FBOs. Input → FBO A (effect 1) → FBO B (effect 2) → ... → output.
_Avoid_: "pipeline" when referring to effects — "pipeline" means the analysis pipeline.

**Ping-Pong FBOs**: Two framebuffer objects alternating as read/write targets during multi-effect rendering. Avoids read-after-write hazards.

**Procedural Source**: A GLSL shader that generates visuals from code (fractals, patterns, noise) instead of a loaded image. 108 sources across multiple categories. Registered in SourceRegistry.

**Compositor Engine**: Manages deck/layer compositing. Renders clips with per-clip effects → transitions → per-layer effects → transforms → keying → blend onto accumulator. Handles temporal buffers, frame ring buffer, feedback, and layer output capture.

**Temporal Effect**: An effect that uses `u_prev_frame` (the previous frame's output). Requires a per-layer temporal buffer. Flag: `EffectDef::temporal = true`.

**Frame Ring Buffer**: Stores 480 previous frames at 1/4 resolution (~240MB VRAM at 1080p). Used by Screen Split and Frame Stutter effects.

**Feedback Processor**: Per-layer Larsen feedback loop. 6 presets: Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch.

**Uniform Bridge**: Maps effect parameter values to `glUniform1f` calls. Naming: `u_[effectName]_[paramName]` for effect-specific, `u_time`/`u_resolution` for globals.

## Mapping & Routing

**Mapping**: Routes an audio feature (source) to an effect parameter (target). Pipeline: extract → normalize → curve → scale → smooth → write.
_Avoid_: "connection", "link" — too vague.

**Curve Transform**: Mathematical function applied during mapping. 5 types: Linear, Exponential (x²), Logarithmic, S-Curve (smoothstep), Stepped (quantized).

**Signal**: A named, typed value in the SignalRegistry. Can be an audio feature, a UI control, or a computed value. Signals feed into the routing engine.

**Chained Signal**: A signal derived from other signals via mathematical operations. Enables complex routing topologies.

**Routing Engine**: Evaluates all signal connections each frame, feeding computed values into the render pipeline.

## Composition Model

**Clip**: Media content (image, video, camera, procedural source, image sequence) with per-clip effects, transport settings (timeline or BPM sync), cue points, and in/out points.
_Avoid_: "media", "content" — too vague.

**Layer**: A horizontal row in a deck. Contains columns of clips, layer-level effects, opacity, blend mode, transition settings. Types: Opaque, Transparent, FX Only, Mask.

**Deck**: A grid of layers × columns. One deck is active at a time. Decks can switch with cross-deck transitions.

**Composition**: Top-level container. Owns multiple decks, global settings, per-type autopilot config, genre-deck assignments.

**Autopilot**: Auto-advances clips in a layer. Trigger modes: On Beat (N beats × loops) or End of Video. Smart mode uses structural state + energy level for intelligent selection.

## Design System (v10)

**Intent Layer**: One of three independent sources of visual control that coexist during performance: Signal (audio-driven automation), Hits (pre-programmed timeline events), and Live VJ (real-time human actions). All three compose simultaneously — they are not modes.
_Avoid_: "mode" — intent layers are concurrent, not mutually exclusive.

**AUTO/OVERRIDE**: Per-field state machine for every routable parameter. AUTO = value driven by Signal layer routes. OVERRIDE = value locked by Hits or Live VJ action, indicated by orange #ff4500. Release is per-field only — no global mass-release.
_Avoid_: "manual mode", "locked" without specifying the OVERRIDE concept.

**Hit**: A scheduled event on the Hits timeline lane that fires at a specific beat-quantized timestamp. Carries a payload (baseline changes, route modifications, clip triggers). Displayed as a pill on the timeline.
_Avoid_: "keyframe" except in analogies — Hits are richer than simple keyframes.

**Top Chrome**: The 98px three-strip header area at the top of the application window: menu bar + TopBar + SignalBar.

**Bottom Focus Bar**: Contextual panel at the bottom of the composition area. Three states: collapsed summary, expanded macro view, expanded Hit anatomy (Hit Inspector).

## Performance & Control

**Binding**: Maps a keyboard key or MIDI note/CC to an action. Three target modes: ByPosition (survives reorder), ThisItem (follows clip by ID), Selected (current UI selection). Two trigger modes: Toggle, Momentary.

**MIDI Learn**: Assigns MIDI CC or note to any parameter. Supports Absolute (0-127→0-1) and Relative (for endless encoders) modes.

**Beat Snap**: Quantizes clip trigger timing to beat boundaries. Modes: Off, Beat, Bar, TwoBar, FourBar.

**Persistent Layer**: A layer that keeps rendering even when its deck is not active. Enables background visuals across deck switches.

## Integration

**Eyes**: Visual testing harness. HTTP test server (port 8080) with REST endpoints for headless effect/source verification. Injects audio features, captures deterministic frames, compares against golden references via PSNR/SSIM.

**Syphon**: macOS inter-app GPU texture sharing via IOSurface. Zero-copy. Enables sending to MadMapper, VDMX, OBS. Optional build flag.

**ISF**: Interactive Shader Format. Community shader format from isf.video. ISFShaderLoader converts ISF metadata + GLSL to Audio-DNA's GLSL 410 format.
