# Output & Integration Reference

> Moved from CLAUDE.md (claudemd-split). Production REST API, OSC, MIDI output, video recording, Syphon, Smart Audio (genre/ISF/BPM recovery), Advanced Audio Analysis (P22-P23, P25).

---

### Output & Integration System (P22)

**Production REST API** (`ApiServer`, port 7070, always-on): 35 registered routes total (all functional; `/api/set_bpm` wired Wave 0 — drives the TopBar manual-BPM override path via the message thread; `/api/resync` added s-rta-0925 — manual Resync via `BPMTracker::requestResync()`, message thread → analysis thread, same funnel as the TopBar Resync button). cpp-httplib on a background thread with CORS headers. 27 core control endpoints: /api/health, /api/status, /api/composition (full deck/layer/clip tree), /api/trigger_clip, /api/trigger_column, /api/set_param, /api/set_layer_opacity, /api/set_master_signal, /api/switch_deck, /api/snapshot, /api/bpm, /api/set_bpm, /api/resync, /api/features, /api/inject_features (test-mode only — 404 in production), /api/load_image, /api/load_source, /api/load_composition, /api/set_effect, /api/effects, /api/sources, /api/render_frame, /api/reset, /api/set_effect_chain, /api/state, /api/syphon, /api/set_syphon. Plus 7 `/api/perf/*` performance-recorder endpoints and `POST /api/audio/source` — both documented in "Audio Store (Ruling 28)" in `docs/claude/recording.md`. All GL mutations go through existing thread-safe APIs.

**OSC Input** (`OscHandler`, `juce_osc` module): Receives OSC on configurable UDP port. Address patterns (13): `/audiodna/clip/{layer}/{column}`, `/audiodna/layer/{n}/opacity|bypass|solo|mute`, `/audiodna/deck/{n}`, `/audiodna/master`, `/audiodna/signal`, `/audiodna/bpm`, `/audiodna/resync`, `/audiodna/snapshot`, `/audiodna/effect/{name}/{param}`, `/audiodna/macro/{n}`. Uses `MessageLoopCallback` template parameter for thread-safe dispatch on JUCE message thread. **LIVE 2026-07-17 (Wave 1-B)**: `OscHandler::startListening(8000)` is called unconditionally at startup (like ApiServer), so the receiver binds UDP port 8000, and all 13/13 pattern callbacks are wired in MainComponent (`/audiodna/signal` added s-rta-0925 mastersignal Step 1; `/audiodna/resync` added s-rta-0925 resync) — each routing through the same handler as the equivalent REST/UI/MIDI path. Port is hardcoded (no preferences UI configures it yet).

**MIDI Output** (`MidiOutputHandler`): Sends note-on/off to hardware controllers (Launchpad X/Mini MK3) for clip state feedback. 5 states: Empty(off), Loaded(velocity 5), Playing(velocity 60), Triggered(velocity 52), ActiveWithFx(velocity 62). Polls deck state ~6Hz from timerCallback. Note mapping: `(layer+1)*10 + (column+1)` for Launchpad grid layout.

**Video Recording** (`VideoRecorder`): Real-time capture from GL framebuffer to H.264/ProRes/MJPEG via FFmpeg. Triple-buffered pixel readback (GL thread does `glReadPixels` into rotating CPU buffers, encoder thread picks up via condition variable). No mutex on GL thread hot path. Codec selection via `VideoRecorder::Config`. Menu: Output > Start/Stop Recording. Saves to ~/Documents/Audio-DNA/Recordings/.

**Snapshot** (`Renderer::takeSnapshot()`): Saves timestamped PNG to ~/Documents/Audio-DNA/Snapshots/. Uses existing `captureFrame()` infrastructure. Bindable via `Binding::Action::Snapshot`. Also available via REST API (`POST /api/snapshot`).

**Syphon Output** (`SyphonOutput`, macOS only, optional): Zero-copy GPU texture sharing via IOSurface. Obj-C++ wrapper around `SyphonServer`. Uses `__has_include(<Syphon/Syphon.h>)` for compile-time detection. Enable with `-DAUDIODNA_BUILD_SYPHON=ON` + install Syphon.framework to /Library/Frameworks/. **WIRED 2026-07-17 (Wave 1-A)**: `init()` runs on GL-context creation and the final composited frame is blit→published once per frame, gated on enabled + initialized; Output → "Syphon Output" menu toggle controls it (default OFF each boot, no persistence). Build-flag-gated — a runtime no-op unless built `-DAUDIODNA_BUILD_SYPHON=ON` with Syphon.framework installed. Post-publish, `publishSyphonFrame` re-binds `defaultFBO` so the subsequent `glReadPixels` capture path is unaffected.

**Syphon Input / Spout / NDI**: REMOVED 2026-07-17 (Wave 0) — `SyphonInput` (.mm/.h, orphaned), `SpoutOutput.h` (Windows header-only no-op), `NdiOutput.h`/`NdiInput.h` (stubs) were all deleted.

---

### Smart Audio Features (P23)

**Genre Detection** (`GenreDetector`): Real-time 8-genre classification from audio features. Uses multi-feature scoring (BPM range, spectral profile, transient density, chromatic complexity) with ~2s EMA smoothing and ~3s hysteresis. Runs as stage 13 in the AnalysisThread pipeline. Zero allocation in steady state. Genres: House (0), Techno (1), DnB (2), Hip-Hop (3), Ambient (4), Rock (5), Pop/Electronic (6), Jazz/Other (7). Also tracks energy state (0=low, 1=medium, 2=high).

**Auto-Preset Selection**: When `composition.autoPresetOnGenre` is enabled, genre changes fire `Renderer::onGenreChanged_` callback on the message thread. Can auto-switch decks via `composition.genreDeckAssignment[8]` (genre→deck index mapping).

**AI Mapping Suggestions** — REMOVED 2026-07-17 (Wave 0): `MappingSuggester` (.cpp/.h) was a ghost (never instantiated, no UI or API caller) and has been deleted.

**Smart Random Autopilot**: When `smartRandomEnabled` is active and autopilot action is PlayRandom, clips are selected based on structural state and energy level instead of pure random. Convention: lower column indices = calmer content, higher = more intense. Drop→intense clips, breakdown→calm clips.

**Structural Scene Triggering**: `Renderer::onStructuralStateChanged_` fires on structural transitions (normal/buildup/drop/breakdown). Controlled by `composition.structuralSceneEnabled`.

**ISF Shader Import** (`ISFShaderLoader`): Imports Interactive Shader Format (.isf/.fs) shaders from isf.video. Parses JSON metadata from comment blocks, extracts parameter definitions (float/bool/long), wraps GLSL with ISF compatibility defines (`TIME`, `RENDERSIZE`, `isf_FragNormCoord`, `IMG_NORM_PIXEL`), converts to GLSL 410. Registers as effects in EffectLibrary with "ISF" category. Menu: Audio-DNA > Import ISF Shader... **PHANTOM**: parse/convert/register work and the effect appears in the FX browser, but `MainComponent::handleImportISF()` never compiles or queues the converted GLSL for the GL thread (the compile step is a TODO no-op) — so imported ISF effects render nothing. The "Import Successful" dialog is misleading.

**Per-Genre Smoothing** — REMOVED 2026-07-17 (Wave 0): `GenreSmoothing.h` was dead code (never instantiated, not wired into MappingEngine, self-refs only) and has been deleted; its One-Euro filter variant was also removed from `Smoother.h`.

**Smart BPM Recovery**: `BPMTracker::feedSilenceDetection(rms)` detects silence (RMS < 0.005 for 300ms) and holds the last good BPM. Phase continues free-running during silence. Resumes after 100ms of audio above threshold. Prevents BPM jumping to 0 during DJ transitions or track endings.

---

### Advanced Audio Analysis (P25)

**AdvancedAudioAnalyzer** (`src/analysis/AdvancedAudioAnalyzer.h/cpp`): Computes 5 advanced spectral features per hop, added as stage 14 in the analysis pipeline. All buffers pre-allocated, zero allocation in steady state.

| Feature | Algorithm | Output | Uniform | Use Case |
|---------|-----------|--------|---------|----------|
| **Sidechain Pump** | Pearson correlation of bass vs mid envelopes (64-hop window). Negative r = pumping. | [0, 1] | `u_sidechainPump` | Techno/house sidechain detection |
| **Swing Ratio** | Inter-onset interval histogram. Consecutive pairs long/short ratio. | [0.5, ~0.67] | `u_swingRatio` | Hip-hop shuffle detection |
| **Formant Presence** | Energy ratio in 300-3000 Hz vocal range vs total. Adaptive normalization. | [0, 1] | `u_formantPresence` | Vocal content detection |
| **Resonance Peak** | Spectral kurtosis in 200-8000 Hz. High = sharp filter peaks. | [0, 1] | `u_resonancePeak` | Filter sweep/synth resonance |
| **Reese Bass** | Spectral spread (weighted std dev) in 30-200 Hz. Wide = detuned/wobble. | [0, 1] | `u_reeseBass` | DnB reese bass detection |

All 5 features are available as MappingSource enum values (`SidechainPump`, `SwingRatio`, `FormantPresence`, `ResonancePeak`, `ReeseBass`), as hidden signals in SignalRegistry, and as shader uniforms in all 3 render paths.

---
