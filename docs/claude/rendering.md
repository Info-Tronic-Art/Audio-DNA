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
- `CompositorEngine::applyClipEffects()` (per-clip/layer deck mode): uses the `layerTemporalBuffers_` map keyed by `LayerStateKey` (`src/render/LayerStateKey.h`) = deck id + layer id + which chain — never by layer id alone (ids repeat in every deck). A layer has three chain keys: `clipChain` (the active clip's effects, also keys the feedback processor), `outgoingChain` (the OUTGOING clip's effects during a clip-to-clip crossfade) and `layerChain` (layer effects). Binds `u_prev_frame` from the chain's buffer, saves output after the chain completes.
- **Crossfades (s-rta-0926b R1)**: at the first frame of every crossfade (`CrossfadeStartDetector`, `src/render/CrossfadeHistory.h`) `handOverClipHistory` COPIES the layer's clip-chain temporal buffer into its outgoing slot and SWAPS the frame rings; the outgoing chain uses the slot for the whole fade. So the outgoing clip keeps its own Echo/Freeze/Screen Split look, and the incoming clip starts from the picture the layer was just showing (exactly what a cut gives it) with an empty ring. Nothing happens at fade end — the slot is the layer's spare for the next fade.

**Frame Ring Buffer** (`FrameRingBuffer` in CompositorEngine): stores 480 previous frames for Screen Split and Frame Stutter effects, downscaled per canvas by `RenderGeometry::ringDownscale(canvasW)` -- at least 1/4 and never a cell wider than 480 px (s-rta-0926b plan4 1E), so one ring is 237 MiB of VRAM at 1080p (1/4) AND at 4K (1/8), 187 MiB at 1440p (1/6); above 1080p the cells are softer than the canvas. Cells are CREATED ON FIRST WRITE, one per pushed frame (`pushFrameToRing`, s-rta-0927 plan-renderperf C1) -- never allocate a ring in bulk inside a frame (480 cells in one frame = 35.5-38.5 ms measured); a ring reaches its full size only after 480 pushed frames. A cell is never read before it is written (`FrameRing::readIndex`, `tests/test_frame_ring.cpp`). These effects are intercepted in `applyClipEffects()` before normal shader processing and rendered by the compositor directly — they don't use GLSL shaders at all. One ring per (deck, layer) chain that uses them — up to two per layer clip chain once it has crossfaded with Split/Stutter on both sides (the outgoing slot). Rings and temporal buffers are created lazily and freed when the GL context closes; `/api/state` reports `frame_rings` / `temporal_buffers` / `frame_ring_cells` (ring cells created so far, over every ring -- s-rta-0928 R5) (and `peak_frame_time_ms`, the longest frame since the previous read; `peak_callback_ms`, the longest WHOLE render callback -- every return path, incl. the work before `renderStart` and after `renderEnd` that the frame timer misses (s-rta-0928 R1.0); `gpu_time_ms` / `peak_gpu_time_ms`, GL timer queries).

**Image loading (s-rta-0928 renderleft R1)**: no image file is decoded on the GL thread. `ImageDecode::Decoder` (`src/render/ImageDecode.h`, a Renderer member: 3 low-priority `juce::ThreadPool` threads, no GL call) decodes and converts to GL-ready RGBA rows (`PixelConvert::argbToGlRgbaBottomUp`, byte-identical to the old per-pixel loops); the GL thread only uploads, in `CompositorEngine::pumpImages` -- EVERY frame, first thing (`renderOpenGL`), within ONE per-frame `UploadBudget` shared by every image path (the first upload of a frame always, more only while the frame's total stays <= 8 MiB; a result that misses it keeps its bytes for a later frame). The bookkeeping is pure (`ImageTexCache::Cache`, `tests/test_image_tex_cache.cpp`): a clip image is Pending (decoding), Resident or Failed (no media, as before). **Pending is never "no media"** -- a 0 clip texture on an Opaque/Transparent layer runs the clip's effects as FX Only (Pitfall 53). While its image decodes, an active-deck Opaque/Transparent layer HOLDS its last picture (its Layer Router output saved by the previous frame, `layerOutputTexStorage_`, owner = deck + frame serial) with its current keying/opacity and no stage (effects, ring, feedback, transition) run on it; a Mask holds its last image mask; a persistent layer, or a layer with nothing to hold (its first picture, a fresh deck, the frame after a resize), draws nothing; a pending OUTGOING image shows the incoming clip alone. A crossfade whose incoming image is pending -- or whose incoming image sequence has nothing to show yet (`ImageSequence::firstFramePending`, no side effect) -- does not advance (C1: the dissolve starts when the picture lands). `render_frame` (7070 + 8080) is answered only by a frame with no image pending (`captureFrame(..., completeFrame = true)`; a user snapshot does not wait). `/api/state`: `image_hold_frames` / `image_skip_frames` (cumulative), `images_pending`, `image_textures` / `image_texture_mb`, `peak_image_upload_ms` (reset on read), `image_pump_frames`. Every composition swap (`MainComponent::swapCompositionModel`: load, New, deck append) posts the composition's image set (`compload::imagePaths`: active deck, active clips first) -- the next frame releases the textures of images no longer in it (they used to live until context loss) and prefetches the rest, ONE decode in flight, up to 1 GiB of resident images (`kImagePrefetchBudgetBytes`, ASSUMED; demand loads are never refused); resident images are re-validated by their {mtime, size} stamp (a changed file is re-decoded and replaces its texture; a failed one is retried). Textures are also freed at context loss (`releaseGL`). **The legacy single image** (the fallback when no deck layer and no source draws; Eyes `load_image`, every clip trigger / selection, the slideshow) is decoded (`PremultipliedRGBA`, `TextureManager::uploadImage`'s bytes) and uploaded only when a frame actually shows it (`Renderer::resolveLegacy`); `loadImage` / `clearImage` are O(1) requests, the last call wins; a re-load of the resident file costs a stat (the decoder compares the {mtime, size} stamp); the slideshow prefetches its next image (`prefetchLegacyImage`). It used to decode + upload at the top of the frame on EVERY image trigger or selection, even in deck mode where it is not shown, outside the frame timer. **Image sequences (s-rta-0928b seqvram)** decode their frames on the same decoder, 4 frames ahead along the sequence's own trajectory (`SeqVram::distances`: Loop wraps, PingPong bounces, reverse flips, an active out point jumps to the in point as `syncMedia` does; <= 4 outstanding), and play through a BOUNDED window of recycled GL textures (`SeqVram::Slots`: a frame is uploaded into a free slot with `glTexSubImage2D`, never glGen/glDelete per frame). All sequences together hold at most `SeqVram::kBudgetBytes` (1 GiB, ASSUMED) -- separate from the stills' prefetch budget: a sequence that fits keeps every frame (no re-decode after its first pass); one that does not gets the free budget as its window (never below 8 frames), evicting the frame shown farthest in the future -- never the current frame, never the one on screen. Floors win: when the drawn sequences' 8-frame floors alone exceed the budget (five 4K sequences), the total exceeds it and `seq_over_budget` = 1 says so. Every frame the Renderer sums the sequences' resident bytes at the frame top (`scanSequenceVram`) and, only over the budget, trims IDLE sequences -- not drawn for `kIdleFrames` = 60 frames (inactive decks / columns; a Pitfall 53 hold skips a fading layer's outgoing chain for 1-3 frames, which never makes it idle) -- to their current + shown frames, least-recently drawn first. Drawn sequences outrank idle ones: a drawn sequence's grant counts only the drawn others' bytes and the idle ones' 2-frame minimum (`SeqVram::drawnAllowance`); the grants of the frame keep a running total. At most `kMaxDeletesPerFrame` = 8 `glDeleteTextures` a frame, shared by the trim, every shrink and the retire drain (one bulk delete of ~127 1080p textures stalled the render callback ~22 ms): slots not deleted yet stay allocated (free, reusable, counted); context loss releases all. A frame not resident (a seek outside the window, a deck switched back -- its clock ran on without decoding) shows the last frame shown for one decode; `*pending` only when nothing was ever shown (the `render_frame` gate, C3); a frame that fails to decode is never retried (the last frame repeats). The decoder pool (3 threads, shared with every image) is the throughput ceiling: a 4K sequence above ~10 fps, or several long sequences re-decoding at once on a loaded machine, repeat frames (`seq_late_frames`) -- as a 4K first pass always did. `/api/state`: `seq_open`, `seq_textures` (allocated slots), `seq_texture_mb`, `seq_over_budget`, `seq_frames_shown`, `seq_late_frames`, `seq_pending_frames`, `seq_uploads`, `seq_slot_reuses`, `seq_evictions`, `seq_stale_drops`, `seq_upload_deferred`, `seq_deletes`, `seq_drawn_textures` (the sequences not idle). `ImageSequence::releaseGL` deletes every slot and empties the table (context loss / retire). Guards: `tests/test_seq_vram.cpp`; live `.harmony/probe-seq-vram.sh` (Pitfall 54).

**Video playback (s-rta-0928b video)**: no video frame is decoded, converted or flipped on the GL thread. `VideoPlayer` runs ONE decode thread per player (`juce::Thread`, normal priority; FFmpeg's own `thread_count` 2 stays) that feeds a 3-slot RGBA ring (`src/media/VideoRing.h`: lock-free, one writer / one reader; each slot is written bottom-up in one pass by `sws_scale` with a negative destination stride, so there is no flip and no allocation per frame). The GL thread keeps the transport clock (`advanceTransport`, unchanged), posts the wanted time, and in `uploadToTexture` picks the newest ring frame with pts <= clock + half a frame of the current request generation, uploading it only when it changed (`glTexSubImage2D`, outside the image `UploadBudget`: bounded by one upload per drawn player per frame). A seek (`seekTo` from any thread, the out-point, a cue, a retrigger) and a Loop wrap bump the generation; the thread re-seeks to the keyframe and catches up, dropping frames without converting them (`video_frames_dropped`; non-reference frames are not even decoded while more than 10 frames from the target, `kSkipNonRefInCatchUp`) until it reaches the clock; a far jump or reverse play (more than a ring's look-ahead -- (slots + 1) frames, at least 0.1 s -- behind, or > 2 s ahead of the newest decoded frame) re-seeks the same way, and the reader frees frames the clock moved away from -- reverse or ping-pong on a long-GOP file therefore shows correct frames at the rate one seek + catch-up allows (a GOP cache is filed). No frame at or before the clock = HOLD the last shown frame (`video_hold_frames`; `video_late_frames` when the clock has passed the next content frame), never 0 / no media; a player that has never shown a frame is PENDING (Pitfall NN / 53: the layer holds its last picture or draws nothing, a crossfade onto it waits (C1, `incomingImagePending` through the media-pending provider), `render_frame` waits (C3)) -- `open()` still decodes frame 0 on the message thread into slot 0 and builds the thumbnail there (a <= 90x72 `SWS_AREA` conversion), so pending happens only for a broken first frame or a seek before the first draw; a player whose first frame never comes -- the decode thread reaches EOF or a decode error before any frame, or no frame within `VideoRing::kFirstFrameTimeoutMs` (2 s) of its first draw -- is FAILED instead: texture 0, NOT pending, "no media" like a failed image, so a crossfade onto it completes and `render_frame` answers (s-rta-0928b video fix round W3; a frame that lands later still shows). A file whose pixel format is unknown without a frame -- an H.264 .mp4 cut before its first frame -- fails `open()` (no player: "no media"; it used to ABORT the app: `sws_getContext` with `AV_PIX_FMT_NONE` trips a libswscale assertion -- fix round 2, `tests/test_video_player_open.cpp`, a forked child per open); a GL release (context loss) never makes a shown player pending again. Rule 15: an off-screen deck's player advances its clock only (`advanceClock`); its thread parks after 250 ms without a draw request (also inside a full-ring wait: `video_threads_awake`) and, on return, re-seeks and catches up while the layer holds -- bounded by one GOP of decode (the whole output used to run at 14 / 3.8 fps for ~3.4 s with the picture frozen on keyframe + 29: diag-media S1b). `videoPlayerMutex_` guards the `videoPlayers_` lookup only; `close()` signals the thread and returns; the thread frees the FFmpeg contexts itself; `drainRetiredMedia` releases a retired player's texture at once and destroys the object only once its thread has exited; `Renderer::installVideoPlayer` (retire, insert, start) is the seam for an asynchronous open. The last 1-2 frames of a clip now show (the decoder is drained at EOF). `/api/state` (both servers): `video_players`, `video_threads`, `video_threads_awake`, `gl_video_decode_calls` (0 by construction), `gl_video_max_decodes_per_call`, `video_uploads`, `video_frames_decoded` / `_dropped` / `_skipped`, `video_seeks`, `video_hold_frames`, `video_late_frames`, `video_pending_frames`, `videos_pending`, `peak_video_upload_ms`, `msg_video_lock_wait_max_ms`. Levers (named constants): `kSkipNonRefInCatchUp` (on), `kDecodeThreadPriority` (normal), FFmpeg `thread_count` (2). Live: `.harmony/probe-video.sh`.

**Canvas size changes keep history (plan4 1C)**: the render size is the composition canvas (`Composition::outputWidth x outputHeight`, Pitfall 37), so `CompositorEngine::resize` runs on a resolution change, never on a window resize. It recreates the base FBOs and then `rescaleHistory` linear-blits every temporal buffer (incl. the crossfade outgoing slots) and every `FeedbackProcessor` ping-pong (`resizePreserving`) to the new size -- Freeze / Echo / feedback pictures survive -- and drops every frame ring (recreated lazily at the new size; Split/Stutter cells fall back to the frames available). NOT rescaled (accepted): the legacy `EffectChainGLState` prevFrame (single-image mode) and stateful procedural sources (they reset on any resize). Ring cells are recreated lazily, one per frame -- no hitch.

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

### Composition Canvas and the Preview Panel (s-rta-0926b plan4)

> Moved from CLAUDE.md's UI Patterns (s-rta-0926b canvas merge, CLAUDE.md byte cap); the canvas render rule is Pitfall 37.

**Preview/Output panel never reshapes the picture**: the canvas is the composition's size and shape (Composition inspector > Output Settings resolution: 16:9 / portrait / square / 4:3 presets, and "Custom (W x H)" for any other size, so the dropdown never names a size the canvas is not); the lower-left panel shows it letter/pillar-boxed at any window size, never stretched to the panel. A window/panel resize reallocates nothing.

**The output tap (s-rta-0927 outputs-c1, Pitfall 40)**: while an output window is live, `Renderer::publishToOutputs` copies the final canvas once per frame into `output::SharedFrameSet` (4 IOSurface-backed slots): after the canvas's last writer (the deck transition, which runs after master opacity) and before `presentCanvas`, and on the two early paths after the canvas clear (an empty app publishes black). A slot is offered to the outputs only once its fence has completed (+1 frame of latency). Each output window's own context blits the newest offered slot, letterboxed (`RenderGeometry::fitCanvas`), once per refresh of its display (`output::presentSharedFrame`, swap interval 0). Zero cost when no output is live; the cost with one shows in `frame_time_ms` / `gpu_time_ms`. The outputs show exactly the canvas the panel shows (and `render_frame` captures); they keep the last frame while the preview is hidden.

---

### Per-clip Fit Mode (s-rta-0926b plan-fitmode)

`Clip::fitMode` (`ClipFit::Mode`, `src/model/ClipFit.h`): **Stretch** (0, default -- the picture fills the canvas, today's output), **Bars** (1 -- its own shape, centred, the rest TRANSPARENT so lower layers show; over nothing it reads black), **Crop** (2 -- its own shape covering the canvas, overflow cut). Stage order: fit -> clip transform -> clip opacity -> clip effects -> transition -> feedback -> layer effects -> layer transform; the fit is the last step of `layer_transform`'s inverse UV chain (`u_fitEnabled` / `u_fitScale` = `ClipFit::scale()`), inside `CompositorEngine::applyClipTransform` -- the one pass every media clip goes through (active deck, persistent layers, the outgoing clip of a crossfade). Only Image / Video / ImageSequence are fitted: a Source renders at the canvas size (nothing to fit), Camera has no deck path, Mask layers skip the transform pass (a portrait mask still stretches). The picture's size comes from the texture (`glGetTexLevelParameteriv`), never `clipWidth/clipHeight` (Pitfall 39). Stretch, or a picture already the canvas's shape, runs no query and no extra pass. Set by the Clip inspector's Fit combo (Transform section), `POST /api/set_clip_param` and OSC `/audiodna/clip/{l}/{c}/fit`; saved as the clip JSON key `fitMode` (missing / out of range -> Stretch). Live: `.harmony/probe-fitmode.sh`.

---

### Composition-Level Transform (P25)

The `comp_transform` shader applies position/scale/rotation to the entire final output. Applied after the effect chain renders, before the master level dim. Uses `glBlitFramebuffer` to copy the framebuffer, then renders the transform shader.

Fields in `Composition`: `compPositionX/Y` (normalized offset), `compScale` (1.0=100%), `compRotation` (degrees), `compAnchorX/Y`. UI controls already exist in the CompositionInspector's Transform section.

---

### Cross-Deck Transitions (P25)

When `Composition::activeDeckIndex` changes, the Renderer saves the current frame as the "outgoing" deck texture and blends to the new deck over `Composition::globalTransitionSpeed` seconds. Three blend modes: Alpha (crossfade), Add (additive), Multiply. Uses `Composition::crossfaderBlendMode` for the blend mode.

The `deck_transition` shader takes two textures (`u_textureA` = outgoing, `u_textureB` = incoming) and a progress uniform. Frame is saved to `prevDeckTexture_` on deck switch detection.

**The outgoing picture is the canvas's PREVIOUS frame (s-rta-0926b plan4 F2)**: the switch is detected at the TOP of `Renderer::renderOpenGL` (the canvas block, before the canvas is cleared), where the canvas still holds the last frame that left the app -- that is blitted into `prevDeckFBO_`. It used to be detected after the new deck was composited, so the "outgoing" copy was the NEW deck's first frame and most deck transitions were cuts (a race decided: measured 2 of 3 switches were cuts on the pre-change app). The transition pass runs AFTER master opacity (both inputs are then final pictures), so it starts exactly on the frame that was on screen; a switch during a running transition starts from the blend that was showing. Live: `.harmony/probe-canvas.sh` row `f2_deck_transition`.

---
