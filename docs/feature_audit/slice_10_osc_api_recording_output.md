# Slice 10: OSC / REST API / Video & Session Recording / Syphon-Spout-NDI / Test Server

Audit scope: `src/osc/`, `src/api/`, `src/recording/`, `src/output/`, `src/test/`.

---

## Section 9: Input / Output

### 9.1 OSC Input

**Source file**: `src/osc/OscHandler.h` + `src/osc/OscHandler.cpp`.

**Implementation**: `OscHandler` inherits `juce::OSCReceiver::Listener<MessageLoopCallback>`. All messages are dispatched on the JUCE message thread (thread-safe). UDP; port is configurable at `startListening(int port)` — no compiled-in default (caller provides). Logged to stderr on success/failure.

**Argument parsing** (OscHandler.cpp:52-55): accepts `float32` directly or `int32` coerced to float. Route keys parsed from address path via `juce::StringArray::fromTokens(address, "/", "")`.

**Address pattern table** — all 11 handled routes:

| Address pattern | Method:Line | Arg semantics | Action |
|---|---|---|---|
| `/audiodna/clip/{layer}/{column}` | cpp:58-70 | float `v>0` triggers (gate). | Invokes `onTriggerClip(layer, column)` on message thread. |
| `/audiodna/layer/{n}/opacity` | cpp:73-83 | float 0-1. | `onSetLayerOpacity(layer, value)`. |
| `/audiodna/layer/{n}/bypass` | cpp:86-96 | float, `v>0.5` = true. | `onSetLayerBypass(layer, bool)`. |
| `/audiodna/layer/{n}/solo` | cpp:99-109 | float, `v>0.5` = true. | `onSetLayerSolo(layer, bool)`. |
| `/audiodna/layer/{n}/mute` | cpp:112-122 | float, `v>0.5` = true. | `onSetLayerMute(layer, bool)`. |
| `/audiodna/deck/{n}` | cpp:125-135 | float `v>0` triggers. | `onSwitchDeck(deckIdx)`. |
| `/audiodna/master` | cpp:138-143 | float 0-1. | `onSetMaster(value)` (master opacity). |
| `/audiodna/bpm` | cpp:146-151 | float. | `onSetBpm(bpm)` — manual BPM. |
| `/audiodna/snapshot` | cpp:154-159 | any value (gate not checked). | `onSnapshot()` — PNG capture. |
| `/audiodna/macro/{n}` | cpp:162-172 | float 0-1. | `onSetMacro(macroIdx, value)`. |
| `/audiodna/effect/{name}/{param}` | cpp:175-187 | float. | `onSetEffectParam(effectName, paramName, value)` — targets global effect chain via string lookup. |

**Total: 11 OSC address patterns**.

**State queries**: `isListening()` (h:39), `getPort()` (h:40).

---

### 9.2 REST API (Production, Port 7070, Always On)

**Source file**: `src/api/ApiServer.h` + `src/api/ApiServer.cpp`. Always linked (httplib promoted from test-only in P22).

**Server config**:
- Default port: `7070` (ApiServer.h:43, parameter default).
- Bind address: `"0.0.0.0"` (all interfaces) (cpp:54).
- Runs on dedicated `std::thread` (cpp:52-59).
- **CORS**: `set_post_routing_handler` attaches to every response (cpp:102-106):
  - `Access-Control-Allow-Origin: *`
  - `Access-Control-Allow-Methods: GET, POST, OPTIONS`
  - `Access-Control-Allow-Headers: Content-Type`
- State: `isRunning()` (h:53), `getPort()` (h:54).
- Content-type: `application/json` for all responses.

**Endpoint table — 21 endpoints total**:

| # | Method | Path | Handler (cpp) | Request body | Response body |
|---|---|---|---|---|---|
| 1 | GET | `/api/health` | cpp:204 | — | `{ok, version:"0.1.0", fps}` |
| 2 | GET | `/api/status` | cpp:213 | — | `{ok, fps, frameTimeMs, masterLevel, activeDeck, bpm, beatPhase, barPhase, phrasePhase, structuralState, detectedGenre, genreConfidence, energyState}` |
| 3 | GET | `/api/composition` | cpp:239 | — | Full deck tree: `{ok, activeDeck, numDecks, masterOpacity, decks:[{name, numLayers, numColumns, layers:[{id, name, opacity, visible, muted, solo, bypassed, activeClipColumn, blendMode, clips:[{id, name, column, playing, mediaType, sourceType}]}]}]}` |
| 4 | POST | `/api/trigger_clip` | cpp:299 | `{layer:int, column:int}` | `{ok}` (async via `MessageManager::callAsync`) |
| 5 | POST | `/api/trigger_column` | cpp:321 | `{column:int}` | `{ok}` |
| 6 | POST | `/api/set_param` | cpp:342 | `{layer?, column?, effect:str, param:str, value:float}` — optional layer/column targets per-clip effect; else global chain | `{ok}` or `{ok:false, error}` |
| 7 | POST | `/api/set_layer_opacity` | cpp:426 | `{layer:int, opacity:float}` | `{ok}` |
| 8 | POST | `/api/switch_deck` | cpp:456 | `{deck:int}` | `{ok}` |
| 9 | POST | `/api/snapshot` | cpp:477 | — | `{ok, file:"<path>"}` — writes PNG via `renderer_.takeSnapshot()` |
| 10 | GET | `/api/bpm` | cpp:487 | — | `{ok, bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount}` |
| 11 | POST | `/api/set_bpm` | cpp:504 | `{bpm:float}` | `{ok}` — **stub** (validates only; no BPMTracker hookup) |
| 12 | GET | `/api/features` | cpp:519 | — | `{ok, rms, peak, rmsDB, lufs, spectralCentroid, spectralFlux, spectralFlatness, bpm, beatPhase, onsetDetected, onsetStrength, dominantPitch, structuralState, detectedGenre, genreConfidence, energyState, bandEnergies[7], chromagram[12]}` |
| 13 | POST | `/api/inject_features` | cpp:560 | Any subset of: `rms, peak, rmsDB, bpm, beatPhase, barPhase, phrasePhase, spectralCentroid, spectralFlux, onsetStrength, onsetDetected, structuralState, detectedGenre, genreConfidence, energyState, bandEnergies[]` | `{ok}` — writes to FeatureBus triple buffer |
| 14 | POST | `/api/load_image` | cpp:606 | `{filepath:str}` | `{ok}` or error |
| 15 | POST | `/api/load_source` | cpp:628 | `{source_type:str, params?:{name:value,...}}` | `{ok}` |
| 16 | POST | `/api/set_effect` | cpp:655 | `{name:str, enabled:bool=true, params?:{name:value,...}}` | `{ok}` — operates on global EffectChain |
| 17 | GET | `/api/effects` | cpp:709 | — | `{ok, effects:[{name, enabled, params:[{name, value}]}]}` |
| 18 | GET | `/api/sources` | cpp:740 | — | `{ok, sources:[{id, name, category}]}` |
| 19 | POST | `/api/render_frame` | cpp:759 | `{output_path:str, time:float=-1.0}` | `{ok}` or error — PNG via `renderer_.captureFrame()` |
| 20 | POST | `/api/reset` | cpp:778 | — | `{ok}` — clears image, source, disables all global effects |

Note: only 20 concrete handlers; routes table at cpp:99-200 registers exactly 20 paths.

**Callbacks wired in MainComponent**: `onTriggerClip`, `onTriggerColumn`, `onSwitchDeck`, `onSnapshot` (h:57-60).

---

### 9.3 Test Server (Conditional `AUDIODNA_BUILD_TEST_SERVER`, Port 8080)

**Source file**: `src/test/TestServer.h` + `src/test/TestServer.cpp`. Gated by `#if AUDIODNA_TEST_SERVER`.

**Server config**:
- Default port: `8080` (TestServer.h:43, ctor default).
- Bind address: `"localhost"` only (cpp:50) — loopback test harness.
- No CORS headers (unlike production API).

**Endpoint table — 16 endpoints total**:

| # | Method | Path | Handler (cpp) | Purpose / Body |
|---|---|---|---|---|
| 1 | GET | `/api/health` | cpp:162 | `{status:"ready", gl_version:"4.1", fps, effects_count}` |
| 2 | POST | `/api/load_image` | cpp:173 | `{filepath:str}` → loads image via `renderer_.loadImage()`; sleeps 100ms for GL. |
| 3 | POST | `/api/set_effect` | cpp:208 | `{name:str, enabled?:bool, params?:{}}` → toggles and sets global effect params. |
| 4 | POST | `/api/set_effect_chain` | cpp:278 | `{effects:[{name, params?:{}}]}` → disables all, re-enables + sets listed effects. |
| 5 | POST | `/api/inject_features` | cpp:355 | Full FeatureSnapshot fields: `rms, peak, rmsDB, lufs, dynamicRange, transientDensity, spectralCentroid/Flux/Flatness/Rolloff, bpm, beatPhase, barPhase, phrasePhase, barCount, structuralState, dominantPitch, pitchConfidence, detectedKey, keyIsMajor, harmonicChangeDetection, onsetDetected, onsetStrength, beatInBar, downbeatDetected, bandEnergies[7], chromagram[12], mfccs[13], sidechainPump, swingRatio, formantPresence, resonancePeak, reeseBass`. |
| 6 | POST | `/api/render_frame` | cpp:456 | `{output_path:str, time?:float, width?:int, height?:int}` → deterministic PNG; honours `renderer_.setLockedResolution()`. |
| 7 | GET | `/api/state` | cpp:508 | `{fps, frame_time_ms, master_level, effects:[{name, category, enabled, dry_wet, params:[{name, value, default}]}], active_deck, num_decks}` |
| 8 | POST | `/api/reset` | cpp:550 | Disables/resets all effects, resets dryWet=1, `renderer_.clearImage()`, `clearActiveSource()`, `setTimeOverride(-1)`, `setMasterLevel(1)`, clears FeatureBus. Sleeps 50ms. |
| 9 | POST | `/api/load_source` | cpp:587 | `{source_type:str, params?:{uniformName:value}}` → `renderer_.setActiveSource()`; 100ms sleep. |
| 10 | POST | `/api/update_source_params` | cpp:638 | `{params:{uniformName:value}}` → `renderer_.updateActiveSourceParams()`. |
| 11 | GET | `/api/sources` | cpp:672 | `{sources:[{id, name, category, params:[{name, uniform, default, min, max}]}], count}` |
| 12 | POST | `/api/load_milkdrop_preset` | cpp:868 | `{preset_path:str}` → activates `projectm_visualizer` source, loads preset, feeds 60 frames of synthetic 80Hz + 440Hz + 2kHz PCM audio. Gated by `AUDIODNA_HAS_PROJECTM`. |
| 13 | GET | `/api/signals` | cpp:715 | `{signals:[{id, name, category, type, value}]}` — categories: Amplitude/Bands/Rhythm/Pitch/Chroma/Timbre/Structure/Modulation. Types: audio/oscillator/envelope. |
| 14 | POST | `/api/add_route` | cpp:746 | `{source_signal_id:int, target_effect:str, target_param:str, output_min?:float, output_max?:float, threshold?:float, gain?:float, inverted?:bool}` → `{ok, route_id}`. |
| 15 | POST | `/api/remove_route` | cpp:805 | `{id:int}` → `{ok}` or `{ok:false, error:"Route not found"}`. |
| 16 | GET | `/api/routes` | cpp:819 | `{routes:[{id, source_signal_id, source_name, target_effect, target_param, output_min, output_max, threshold, gain, inverted, enabled}]}` |
| 17 | POST | `/api/set_macro` | cpp:856 | **Stub** — returns `{ok:true, note:"Macro endpoint stub — full MacroBank integration pending"}`. |

Routes registered at cpp:88-158. **Total: 17 endpoints (including stub macro).**

---

### 9.4 Video Recording

**Source file**: `src/recording/VideoRecorder.h` + `src/recording/VideoRecorder.cpp`.

**Architecture** (h:21-35):
- GL thread calls `submitFrame(framebufferWidth, framebufferHeight)` after rendering.
- `glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, ...)` into CPU buffer.
- Triple-buffered pixel readback (`kNumBuffers = 3`, h:91), atomic write/read indices.
- Dedicated encoder thread picks up ready buffers; sws_scale RGBA→YUV; encodes via FFmpeg.
- Frame drops counted in `droppedFrames_` when all buffers busy (cpp:112).
- No mutex on GL hot path.

**Codec options** (h:43, enum class `Codec`):

| Codec | FFmpeg encoder (cpp:205-208) | Pixel format (cpp:243-262) | Quality semantic | Default container |
|---|---|---|---|---|
| `H264` | `libx264` | `AV_PIX_FMT_YUV420P` | `crf` 0-51 (lower=better), preset `ultrafast`, tune `zerolatency`. Default `quality=23`. | `mp4` |
| `ProRes` | `prores_ks` | `AV_PIX_FMT_YUV422P10LE` | `profile` 0-5 (quality field maps to `profile`) | `mov` |
| `MJPEG` | `mjpeg` | `AV_PIX_FMT_YUVJ420P` | `qmin=2, qmax=31` (fixed; quality field unused) | `mp4` |

**Config struct** (h:46-54):
- `codec` (default H264)
- `width=1920`, `height=1080`
- `fps=30`
- `quality=23`
- `container` (auto-selected if empty, cpp:186-189)

**Runtime flow**:
- `startRecording(juce::File outputFile, Config config)` (h:58) — opens FFmpeg context (avformat_alloc_output_context2), creates stream, opens codec, writes header, allocates frame/packet, sets up sws_getContext RGBA→YUV with `SWS_FAST_BILINEAR`, spawns encoder thread.
- `stopRecording()` (h:61) — flushes encoder, writes trailer, joins thread. Async `onRecordingFinished(success, file)` callback on message thread.
- `submitFrame(fbW, fbH)` (h:66) — clamps capture to min(config, framebuffer).
- Encoder flip: negative sws stride to convert GL bottom-up to top-down (cpp:377-378).

**State queries**:
- `isRecording()` (h:69)
- `getRecordedDuration()` (h:70) = `framesWritten_ / fps`
- `getRecordedFrameCount()` (h:71)
- `getDroppedFrameCount()` (h:72)

**State machine** (h:81): `Idle`, `Recording`, `Stopping` (atomic).

**Output path**: Caller-supplied `juce::File`. CLAUDE.md notes Menu > Output > Start Recording saves to `~/Documents/Audio-DNA/Recordings/` — directory convention is in the menu handler, not VideoRecorder itself.

**No audio**: h:34-35 — video-only capture; audio not muxed.

---

### 9.5 Session Recording

**Source file**: `src/recording/SessionRecorder.h` + `src/recording/SessionRecorder.cpp`.

**Event types** (h:26-35, enum `EventType : uint8_t`):
- `ParameterChange` — effect/source/transform param value
- `ClipTrigger` — clip activated (layer, column)
- `ColumnTrigger` — entire column activated
- `MacroChange` — macro knob value
- `TransportChange` — play/pause/stop/speed/reverse
- `EffectToggle` — enabled/disabled/bypassed
- `CuepointJump` — jumped to cuepoint

**Event struct** (h:37-57):
- `timestamp:double` — seconds since recording start
- `type:EventType`
- `targetId:uint32_t` (clip ID or layer index)
- `paramIndex:uint32_t` (param or macro index)
- `value:float`
- `layerIndex:int = -1` / `columnIndex:int = -1`
- `action:string` (for TransportChange)
- `effectName:string` / `enabled:bool` (for EffectToggle)

**Transport controls**:
- `startRecording()` / `stopRecording()` / `isRecording()` (h:60-62)
- `startPlayback()` / `stopPlayback()` / `isPlaying()` (h:74-76)
- `advancePlayback(dt)` → vector of Event pointers that should fire this frame (h:80) — auto-stops at end (cpp:135-137).

**Record APIs** (h:65-71): one method per event type, all thread-safe via `juce::CriticalSection lock_`. All record methods early-exit if `!recording_` (e.g., cpp:22).

**Session management**:
- `getNumEvents()` (h:83)
- `getDuration()` (h:84) — timestamp of last event.
- `saveToFile(juce::File)` (h:87) — JSON.
- `loadFromFile(juce::File)` (h:88) — JSON.
- `clear()` (h:91).

**JSON schema** (cpp:165-217):
```
{ "version": 1,
  "events": [
    { "t": float, "type": 0-6,
      // ParameterChange: "targetId", "paramIndex", "value"
      // ClipTrigger: "layer", "column"
      // ColumnTrigger: "column"
      // MacroChange: "macroIndex", "value"
      // TransportChange: "action", "value"
      // EffectToggle: "effect", "enabled"
      // CuepointJump: "clipId", "cuepointIndex", "position"
    }, ...
  ]
}
```

**Time source**: `juce::Time::getMillisecondCounterHiRes()/1000.0` (cpp:3-6).

---

### 9.6 Syphon Output (macOS optional)

**Source**: `src/output/SyphonOutput.h` + `src/output/SyphonOutput.mm`.

**Platform**:
- macOS-only (`#if __APPLE__`); non-macOS gets a no-op stub class (h:56-70).
- Runtime detection via `__has_include(<Syphon/Syphon.h>)` (mm:10-17) → sets `AUDIODNA_HAS_SYPHON`. If absent, framework-dependent code compiles as stubs.
- Obj-C++ wrapper (`.mm`) with `SyphonServer`. Uses `CGLContextObj` from `NSOpenGLContext`.

**API**:
- `init(void* nsOpenGLContext)` — GL-thread-only (h:29).
- `publishTexture(GLuint texId, int width, int height)` — GL-thread-only; publishes full `NSMakeRect(0,0,w,h)`, `GL_TEXTURE_2D`, `flipped:NO` (mm:51-61).
- `setServerName(std::string)` — default `"Audio-DNA"` (h:53).
- `shutdown()` — GL-thread-only; called in dtor.
- `setEnabled(bool)` / `isEnabled()` — gating without destroying server (h:42-43).
- `isInitialized()` (h:45).

**Enable**: CMake `-DAUDIODNA_BUILD_SYPHON=ON` + `Syphon.framework` installed in `/Library/Frameworks/`.

---

### 9.7 Syphon Input (macOS optional)

**Source**: `src/output/SyphonInput.h` + `src/output/SyphonInput.mm`.

**Platform**: Same gating as Syphon Output. Non-macOS stub (h:61-73).

**API**:
- `init(void* nsOpenGLContext)` (h:28)
- `listServers()` → `vector<pair<appName, serverName>>` (h:44). Uses `[SyphonServerDirectory sharedDirectory].servers`; reads `SyphonServerDescriptionAppNameKey` + `SyphonServerDescriptionNameKey` (mm:143-163).
- `connectToServer(appName, serverName)` → bool (h:31). Finds matching desc; creates `SyphonClient` with `CGLContextObj`.
- `disconnect()` (h:34).
- `getLatestTexture(int& outWidth, int& outHeight)` → GLuint; 0 if no new frame (h:38). Uses `[client newFrameImage]`, reads `textureSize` and `textureName`. Comment (mm:119-120): texture only valid until next call.
- `hasNewFrame()` (h:41) — `[client hasNewFrame]`.
- `shutdown()` / `isConnected()` (h:47-49).

---

### 9.8 Spout Output (Windows stub)

**Source**: `src/output/SpoutOutput.h` (header-only, no `.cpp`).

**Status**: Full stub. All methods empty/no-op (h:18-32).

**API surface** (for parity with Syphon):
- `init()`, `publishTexture(texId, w, h)`, `setServerName(name)` (default `"Audio-DNA"`), `shutdown()`, `setEnabled(bool)`, `isEnabled()`, `isInitialized()` (always false).

**Implementation note** (h:8-10): "requires the Spout2 SDK (https://github.com/leadedge/Spout2) which uses DirectX shared textures." No compile-time flag wired.

---

### 9.9 NDI Input / Output (stubs)

**NdiOutput** (`src/output/NdiOutput.h`, header-only):
- Status: stub gated by `#if AUDIODNA_HAS_NDI` (never defined by the build).
- API: `init(sourceName)` returns false; `sendFrame(rgba, w, h)` no-op; `shutdown()` no-op; `setEnabled`/`isEnabled` (h:58-59); `isInitialized()` always false.
- Comments note the planned SDK calls (`NDIlib_send_create`, `NDIlib_send_send_video_v2`).
- Threading comment (h:21-23): planned same triple-buffer pattern as VideoRecorder.
- Default source name: `"Audio-DNA"` (h:63).

**NdiInput** (`src/output/NdiInput.h`, header-only):
- Status: stub.
- API: `listSources()` returns `{}`; `connectToSource(name)` returns false; `disconnect()`; `hasNewFrame()` returns false; `getLatestFrame(outW, outH)` returns nullptr; `isConnected()` returns false.
- Planned: `NDIlib_find_create`, `NDIlib_recv_create`, `NDIlib_find_get_current_sources`, `NDIlib_recv_destroy`.

---

### 9.10 Snapshot (PNG capture)

**Entry points**:
1. **OSC**: `/audiodna/snapshot` → `onSnapshot()` callback (OscHandler.cpp:154-159).
2. **REST (prod)**: `POST /api/snapshot` → `renderer_.takeSnapshot()` returning `{ok, file:"<path>"}` (ApiServer.cpp:477-485).
3. **Binding**: `Binding::Action::Snapshot` (per CLAUDE.md P22 notes).
4. **Test server**: rendered frames go through `POST /api/render_frame` with explicit `output_path` (TestServer.cpp:456-506) — distinct code path from snapshot (uses `captureFrame`, not `takeSnapshot`).

**Destination**: `renderer_.takeSnapshot()` writes timestamped PNG to `~/Documents/Audio-DNA/Snapshots/` (path convention documented in CLAUDE.md P22; Renderer implementation not in this slice).

---

## Section 20 (partial): Cross-references

- **OscHandler** callbacks wired by `MainComponent` → fanned out to existing thread-safe APIs (no direct OSC → GL mutation).
- **ApiServer** shared refs: `Renderer, FeatureBus, Composition, EffectChain, SourceRegistry, SignalRegistry, RoutingEngine, BindingManager, SessionRecorder` (ApiServer.h:34-43). Posts mutations via `juce::MessageManager::callAsync`.
- **TestServer** refs: `Renderer, FeatureBus, Composition, EffectChain, SourceRegistry, SignalRegistry, RoutingEngine` (TestServer.h:36-43).
- **VideoRecorder** consumes the final GL framebuffer via `glReadPixels` post-render; independent of audio pipeline. Uses FFmpeg: `libavformat, libavcodec, libavutil, libswscale` (cpp:7-13).
- **SessionRecorder** is an orthogonal event recorder. Not wired into VideoRecorder (separate domains: video pixels vs event stream).
- **SyphonOutput** publishes texture at end of `renderOpenGL()` (per CLAUDE.md P22), same hook point as `VideoRecorder::submitFrame()`.
- **SyphonInput** texture used as clip source (per CLAUDE.md P22 note).
- **Snapshot**: `Renderer::takeSnapshot()` referenced but defined outside this slice.
- **Genre/advanced-audio fields** from P23/P25 are exposed via `GET /api/status`, `GET /api/features`, and `POST /api/inject_features` — including `sidechainPump, swingRatio, formantPresence, resonancePeak, reeseBass` (TestServer.cpp:444-449).
- **`POST /api/set_bpm`** (production) is documented as "This would need access to BPMTracker — for now, just acknowledge" (cpp:515) — **known stub**.
- **`POST /api/set_macro`** (test) is documented stub (TestServer.cpp:857-866).
- **`POST /api/render_frame`** (production) uses `renderer_.captureFrame(file, time)` without explicit width/height (ApiServer.cpp:771). Test server variant accepts `width/height` and toggles `setLockedResolution()`.
- Known infra limitation (per CLAUDE.md pitfall #28): test-server `/api/render_frame` captures raw source/image output **without the global effect chain applied**. Verify effect rendering via `/api/state` readback instead of PSNR against captures.
