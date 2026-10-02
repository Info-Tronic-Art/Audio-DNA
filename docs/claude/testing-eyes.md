# Eyes Visual Testing Harness Reference

> Moved from CLAUDE.md (claudemd-split). Build flag, endpoints, task-verification workflow.

---

### Visual Testing Harness (Eyes)

Build with `-DAUDIODNA_BUILD_TEST_SERVER=ON` to embed an HTTP test API. Run with `--test-mode` to start the server. Python scripts send commands (load image, enable effects, inject audio features, capture frames) and compare rendered PNGs against golden references using PSNR/SSIM.

See `tests/visual/TESTING.md` for the full API reference, Python client docs, and test authoring guide.

**Output frame path, offscreen (s-rta-0927 outputs-c1)**: `POST /api/set_output_tap {"enabled": bool}` forces the output tap (the main renderer publishes the canvas into the shared IOSurface frames as if an output window were live -- no window is opened). `POST /api/output_probe {"width", "height", "output_path"}` presents the newest shared frame through a PRIVATE GL context of the app with the same code the Output window uses and writes the PNG a display would show (alpha 255); response `{ok, gen, serial, slot, canvas_w, canvas_h}`, 409 when nothing was ever published. Test mode only (8080). Used by `.harmony/probe-outputs.sh`. Never test the Output window by opening it in an automated gate.

**Reverse / ping-pong rows (s-rta-0929b gopcache)**: `.harmony/probe-vupload.sh` u7 runs nine scenes (g30 / g250 x reverse / ping-pong forward window / ping-pong turn at in-point 0.7, a speed-2 g250 reverse, two forward -> reverse flips over REST: a hand-built take replayed by `/api/perf/load` + `/api/perf/play`) with absolute bars on the non-forward scenes (uploads/s >= 28.5 x speed, late <= 10, the mid-window capture within a 1-frame playhead bracket, 20 paced captures falling strictly, `video_reverse_nonmonotonic` 0), judged on the lane app's MEDIAN over >= 5 interleaved launches by `probe-vupload-ab.py`; u8 (a column of four 1080p reversers: pooled and slowest-player uploads/s, `video_gopcache_bytes` under the budget; with `VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256`, a TEST-SERVER budget override), u9 (the idle trim drops a reversing player's cache: bytes 0, phys_footprint falls by >= half of it), u10 (4K reverse, INFO), u11 (the switch back to a deck of four reversers: the column's longest upload gap), u12 (a frame shown reversing vs the same frame shown forward at speed 0: max |diff| 0, per upload path). `probe-vupload-ab.py` rules u8 per cap group of the lane arm (B); the pre-lane arm's lines are its baseline (a capped-only run no longer invents 'cap 0.0' rules); `probe-vupload-ab.py --selftest` checks the summarizer offline; the A/B driver taints a launch that saw a burner (`yes` / `stress-ng` / `ffmpeg`) while its app ran and refuses to start with an orphaned one (s-rta-0930 gop2). u13 (s-rta-1002b mkvidx, Pitfall 64) reverses a GOP-250 1080p clip as Matroska with its Cues at the front / at the end and a QuickTime 1/600 HAP clip (u7's reverse scene), four 60 s reversers MKV vs MP4 by one column trigger and a 60 s ping-pong turn MKV vs MP4, each printing the decode thread's `Keyframe index:` witness count; `probe-vupload-ab.py` reads [CTRL-A] first (the pre-lane arm must freeze on the front-Cues file, fail the HAP bars and pay the Matroska cost, else the rule it guards is read as non-discriminating), then [FREEZE] / [MKV] / [HAP] (u7's reverse bars, but the capture bracket, the falling captures and nonmono 0 in EVERY lane launch, not as medians; HAP also <= 1.25 decodes per upload), [CPU] (the four-player Matroska decodes per upload <= 0.7 x the pre-lane app's), [PARITY] (<= 1.15 x the same column in MP4), [GUARD] and [PP-PARITY]; bars frozen in `probe-vupload.json` "_u13".

**GL context cycle, test mode only (s-rta-0929 vupload)**: `POST /api/debug/gl_context_cycle {"detached_ms": N}` (N 0-5000, default 0) detaches the preview panel's GL context and re-attaches it N ms later on the message thread (the same JUCE path as a preview hide / app minimise / Output layout change: `openGLContextClosing`, then `newOpenGLContextCreated`; every shader recompiles, ~60 ms) -- poll `gl_context_gen` (8080 `/api/state`) until it advances; answers `{ok, gen_before, detached_ms}` at once; no window is opened. 8080 `/api/state` also carries `gl_thread_qos` (the render thread's QoS class, sampled every frame: 21 = DEFAULT; INFO -- the USER_INTERACTIVE raise was reverted, VU15) and `phys_footprint_mb`. TEST_SERVER env `ADNA_VIDEO_FORCE_FALLBACK=malloc|client` forces every video player off the IOSurface blit (probe-video w10's arms). Used by `.harmony/probe-vupload.sh` u2 (INFO) / u4a / u4b.

**Audio device policy witnesses, test-server builds (s-rta-0929b btguard)**: `GET /api/debug/audio_devices` (production port, no `--test-mode`) returns the last device scan (every CoreAudio device with its transport, `transport_read_ok`, aggregate members, allowed flag and reason; the filtered lists and defaults; `skipped` / `unmapped`), the opened devices (`opened`: names, rate, buffer, active channels), `state` (`ok` / `no-input` / `no-device`), `opens` (device starts since launch, INFO) and `reapplies` (the device-gone reconciler), read from a mutex-guarded copy published on the message thread (never the manager on the HTTP thread). `GET /api/debug/ui_text` also answers `audio_notice` (the no-input / no-device notice beside the file label, "" when hidden). Env `ADNA_AUDIO_DENY_DEVICES=<name>[;<name>]` (exact JUCE names, read once at launch) treats those devices as denied (reason `test-denied`) -- it drives the guard's HIDING branch, never the transport branch -- `.harmony/probe-btguard.sh` arms B / C / D. Never connect a Bluetooth device for a gate. `POST /api/debug/audio_deny {"names": ["<name>", ...]}` (same build path; `[]` = none, `"*"` = every device) replaces that set at runtime and runs the guard's device-list-change path on the message thread (inner rescan -> rebuild -> JUCE's listeners -> the reconciler) -- the probe's stand-in for plugging / unplugging a device; `POST /api/debug/audio_stop` (no body) stops the open device as JUCE's combiner does when its input dies (the manager keeps it; the reconciler acts on the next device scan). The status gains `last_reapply` (`no-device` / `adopt-input` / `input-lost` / `device-stopped`, "" before the first re-apply) and `lost_input` (the mic a re-apply lost and replaced by another, "" otherwise); `state` may read `mic-replaced`.

**Output hot-plug / saved set, screen-safe (s-rta-0927 outputs-c3)**: 8080 `/api/state.outputs.manager` = `{poll_enabled, poll_ticks, reconciles, settings_writes, restore_calls, interrupted, saved, restorable}` (`OutputManager::statsVar`, atomics). `POST /api/output_restore_last` runs the "Restore Last Outputs" menu action's own handler ONLY while `restorable == 0` (409 otherwise; the app re-checks on the message thread) -- it can never open a window. `POST /api/set_output_poll {"enabled": bool}` pauses / resumes the 30 Hz display poll (the `o_poll_idle` A/B). In a test-server build running `--test-mode`, `AUDIODNA_SETTINGS_FILE=<absolute path>` replaces the user's settings.json (unset or not absolute: a scratch file in `~/Library/Caches/Audio-DNA/`, never the real one); `.harmony/probe-outputs.sh` points every run at `$OUT/settings.json` (absent, or seeded from `OUTP_SETTINGS`). Test mode only (8080).

```bash
# Build with Eyes
cmake -B build -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON
cmake --build build --config Release -j$(sysctl -n hw.ncpu)

# Run visual tests
pip install -r tests/visual/requirements-test.txt
cd tests/visual && pytest test_render_pipeline.py -v
```

#### Using Eyes for Task Verification

**After any task that changes effects, sources, shaders, or the render pipeline**, use Eyes to verify everything still works before presenting to the user:

1. **Launch the app in test mode** (if not already running):
   ```bash
   ./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA --test-mode --test-port=8080 &
   sleep 5
   ```

2. **Run the standard test suite**:
   ```bash
   source .venv/bin/activate
   AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_render_pipeline.py -v
   ```

3. **Test specific things you changed** using the Python API:
   ```python
   import requests
   BASE = "http://localhost:8080"
   IMG = "/Users/boriskarpman/Documents/RealTimeAudio/tests/fixtures/test_card.png"

   # Load image, enable effect, capture frame
   requests.post(f"{BASE}/api/load_image", json={"filepath": IMG})
   requests.post(f"{BASE}/api/set_effect", json={"name": "Ripple", "enabled": True, "params": {"intensity": 0.5}})
   requests.post(f"{BASE}/api/render_frame", json={"output_path": "/tmp/test.png", "time": 1.0})
   ```

4. **Test all effects or sources in bulk** (sweep params 0.0→1.0, verify PSNR changes from baseline).

5. **Kill the test app** when done: `pkill -f "Audio-DNA.*--test-mode"`

#### Key Endpoints Quick Reference

| Endpoint | What it does |
|---|---|
| `GET /api/health` | Check app is ready |
| `POST /api/load_image` | `{"filepath": "..."}` -- decoded off the GL thread; the handler's 100 ms sleep is no longer load-bearing: the next `render_frame` waits for the picture (Pitfall 53) |
| `POST /api/set_effect` | `{"name": "...", "enabled": true, "params": {...}}` |
| `POST /api/set_effect_chain` | `{"effects": [{"name": "...", "params": {...}}, ...]}` |
| `POST /api/inject_features` | `{"rms": 0.8, "beatPhase": 0.5, ...}` |
| `POST /api/render_frame` | `{"output_path": "...", "time": 1.0}` — deterministic capture; concurrent calls (8080 and 7070) are served one at a time, each at its own width/height (Pitfall 52); answered only by a frame with no image still decoding -- a capture right after a trigger or load shows the new picture (Pitfall 53); a video HOLD (the last shown frame while its decode thread catches up after a seek) is not pending: `render_frame` may capture it, as is a frame held by the per-frame video upload budget (<= 2 render frames) -- poll `video_late_frames` until it stops moving before a frame-accuracy check (probe-video `settle_late`); a video whose first frame never decodes is FAILED (no media), not pending -- `render_frame` answers (probe-video w9); written by the fast PNG writer (zlib level 1, filter 0: same decoded pixels as JUCE's writer, different file bytes; the app log line ends `enc=fast`) -- compare captures by DECODED pixels, never by file hash; `/api/snapshot` keeps JUCE's writer (`enc=archive`) |
| `GET /api/state` | Full engine state (all effects, params, FPS) |
| `POST /api/reset` | Clear everything for next test |

#### Source Testing via Eyes

To test procedural sources, use the deck/clip API to load a source into a cell, then capture a frame. Sources are set via `Renderer::setActiveSource()` which the test server exposes through clip loading. For direct source testing, use `POST /api/load_source`:

```python
# Test a procedural source
requests.post(f"{BASE}/api/reset", json={})
# Set active source directly on the renderer
requests.post(f"{BASE}/api/load_source", json={"source_type": "perlin_noise"})
requests.post(f"{BASE}/api/render_frame", json={"output_path": "/tmp/source_test.png", "time": 1.0})
```

`load_source` seeds the registry's full default param list and overlays any `params` you pass (Pitfall 47) --
without the seed the session-cached source instance kept whatever the previous caller set, across `load_source`
and `/api/reset`. `update_source_params` REPLACES the list with the given subset; since the instance already holds
the defaults, "load, then update one param" renders defaults + that param. The Tier-1 modules that capture at
256x256 pin the composition to 256x256 for their module (`conftest.py` `tier1_canvas_256`): with the composition and
the capture lock the same size the canvas never resizes between captures, so stateful sources keep their state.

#### Probe rig rules (`.harmony/probe-*.sh`, distinct from Eyes above)

The production-mode live probes in `.harmony/probe-*.sh` (not the `--test-mode` Eyes harness) each
refuse (exit 64) unless `/tmp/audiodna-live.lock/owner` exists, and honor `AUDIODNA_LOCK_OWNER` if
set. They detect/terminate the app via `adna_pids`/`adna_running`/`adna_kill` helpers defined in
each script, which filter `ps -o ucomm=` for exactly `Audio-DNA` -- the kernel's real exec-time
process name, immune to a build's linker/compiler command line containing that path as an `-o`
argument, and immune to a process spoofing argv[0] via `exec -a .../MacOS/Audio-DNA <cmd>` (both
would falsely match a plain `pgrep -f`/`pgrep -x` substring or comm check). `probe-crossfade.py` and
`probe-render-state.py`'s `cap()` also delete any pre-existing output PNG before requesting a new
one and require the written file's mtime to postdate the request (ported from
`probe-effects-parity.py`'s `cap()`), so a probe never decodes a stale frame from a previous run.

---
