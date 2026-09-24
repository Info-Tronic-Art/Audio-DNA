# JUCE upstream history vs. Bluetooth HFP CoreAudio temp-buffer overflow (8.0.4 → 8.0.15)

Research only — no source touched, app not launched. Read-only.

## VERIFIED: the fix commit

- **Commit**: [`f6df3e3ce181baaf97b41ac1dc71d11c26613330`](https://github.com/juce-framework/JUCE/commit/f6df3e3ce181baaf97b41ac1dc71d11c26613330) — "CoreAudio: Respect buffer size passed to audio callback" — author reuk, 2025-05-28. +143/-44 in `modules/juce_audio_devices/native/juce_CoreAudio_mac.cpp` — [source](https://github.com/juce-framework/JUCE/commit/f6df3e3ce181baaf97b41ac1dc71d11c26613330) — accessed 2026-09-24.
- **First tag containing it**: **8.0.8** (published 2025-06-02T10:28:49Z, [release](https://github.com/juce-framework/JUCE/releases/tag/8.0.8)). 8.0.7 was published 2025-04-08 (predates the commit), so f6df3e3 landed strictly between 8.0.7 and 8.0.8. Public CHANGE_LIST.md entry for 8.0.8: "Fixed a iOS simulator buffer size issue" — [CHANGE_LIST.md](https://github.com/juce-framework/JUCE/blob/master/CHANGE_LIST.md) accessed 2026-09-24. (The public wording undersells scope — it's a general CoreAudio callback-buffer-size guard, not iOS-simulator-only; confirmed via the commit diff and the forum thread below.)
- **Commit message**: "We now query the incoming buffers to see how many samples are available. If the callback's buffers will fit into our preallocated buffer (i.e. the length in samples is smaller or equal to the preallocated buffer), then we perform an audio callback with the provided data, even if the number of samples is smaller than expected. If the callback's buffers are larger than expected, we split the incoming buffer into chunks that are no larger than the prepared buffer-size."

## VERIFIED: forum thread confirms same failure class, same team fix

[CoreAudio crashing after initialising devices with different buffer sizes](https://forum.juce.com/t/coreaudio-crashing-after-initialising-devices-with-different-buffer-sizes/66207) (forum.juce.com, thread active 2025-05 to 2025-06) — accessed 2026-09-24:
- Reporter traces the bug to `CoreAudioInternal::reopen()`: after `audioObjectSetProperty(kAudioDevicePropertyBufferFrameSize, …)` "succeeds" (no error logged), `updateDetailsFromDevice()` can still read back the OLD (smaller) buffer size from the device before it has actually reconfigured — so `allocateTempBuffers()` sizes the temp buffer for the old (smaller) size while `bufferSize` member is set to the new (larger) requested size, i.e. a mismatch between allocated capacity and the size subsequently assumed/written on each callback.
- JUCE team (reuk) response: "We've now released a patch intended to fix this issue: CoreAudio: Respect buffer size passed to audio callback" — directly names f6df3e3, i.e. this is the officially-cited fix for exactly this class of "device silently gives us a different frame count than we sized the temp buffer for" overflow.
- This matches the RTA finding's mechanism at the byte level: a fixed-size `allocateTempBuffers()` block (RTA: 1296 bytes, `total_channels*(bufferSize+4)` floats) is written past by the input copy loop in `audioCallback` because the actual per-callback frame count the device delivers diverges from what was assumed when the block was sized — Bluetooth HFP devices (variable/renegotiated per-callback buffer sizes) are a known trigger for this same divergence, per the same forum thread and the "iOS simulator" framing in CHANGE_LIST (simulator and BT HFP share the "device reports one size, callback delivers another" root cause).
- ASSESSMENT (INFERRED, not verified against actual JUCE source diff line-by-line): f6df3e3 is very likely the fix for the RTA 4-byte overflow. Confidence is not VERIFIED because the RTA app pins `8.0.4` (`CMakeLists.txt` `GIT_TAG 8.0.4`) — pre-fix — and no build/test against 8.0.8+ was performed (task is read-only, no source edits, no app launch permitted).

## Other CoreAudio-combiner commits found, judged NOT the primary fix

- [`0638daf9`](https://github.com/WeAreROLI/JUCE/commit/0638daf9a82ceaabc4f4472d833e29b07976dabb) (2018) — pre-dates 8.0.4, already in the RTA's pinned version; adds buffer-size-mismatch handling to `AudioIODeviceCombiner::restart()`, not the per-callback overflow.
- [`1616c0e`](https://github.com/juce-framework/JUCE/commit/1616c0ee263cb39aacc25c418b50faaa0e9cdb01) "CoreAudio: Ensure devices are restarted correctly after changing sample rate" — fixes a `previousCallback`/`callback` regression, unrelated to buffer sizing.
- JUCE issue [#976](https://github.com/juce-framework/JUCE/issues/976) "Race condition on Device Start-Shutdown" — a *different*, already-fixed (pre-8.0.4, via `f1b6bbc9`) `AudioIODeviceCombiner::start`/`shutdown` race; not a buffer-overflow.
- JUCE issue [#796](https://github.com/juce-framework/JUCE/issues/796) "Data race on `AudioIODeviceCombiner::DeviceWrapper::isWaitingForInput`" — thread-safety issue, not memory overflow; status not confirmed fixed as of research date.

## Breaking changes 8.0.4 → 8.0.15 relevant to this app's linked modules

Source: [BREAKING_CHANGES.md](https://github.com/juce-framework/JUCE/blob/master/BREAKING_CHANGES.md) accessed 2026-09-24. App links `juce_audio_devices`, `juce_opengl`, `juce_gui_basics`, `juce_dsp`, `juce_osc` per `CLAUDE.md` tech-stack table (GIT_TAG 8.0.4).

- **8.0.9 — `juce_opengl` (app uses OpenGL 4.1 core for all effects rendering)**: `OpenGLFrameBuffer::readPixels()` and `writePixels()` gained a new required `RowOrder` parameter. Any direct call in Audio-DNA's `Renderer`/FBO code (e.g. snapshot/video-recording readback paths) would fail to compile against ≥8.0.9 without adding the new argument. **Action if upgrading**: grep for `readPixels(`/`writePixels(` calls on `OpenGLFrameBuffer` in `src/render/` and `src/recording/VideoRecorder.cpp`.
- **8.0.5 — `juce_gui_basics`/audio (`AudioTransportSource`, used for file playback per tech-stack table)**: `AudioTransportSource::hasStreamFinished()` behavior changed from "never returns true" (bug) to correctly returning true at stream end. **Possible issue**: any code/ChangeListener in `AudioEngine` relying on the old always-false behavior would see new completion callbacks after upgrade — worth auditing `src/audio/AudioEngine.cpp` end-of-file handling before upgrading.
- **8.0.13 — `juce_audio_processors`-adjacent**: `AudioProcessor::createEditor()` made private (not applicable — Audio-DNA is a standalone app, not a plugin; no `AudioProcessor` subclass per architecture).
- **8.0.5 — Projucer/Windows Arm32 removal, VS "Debug Information Format" defaults (8.0.5 and 8.0.7, reverted differently)**: build-config only, not API; no source-level action needed since project uses CMake directly.
- **No breaking changes found** for `juce_dsp` or `juce_osc` in the 8.0.4→8.0.15 window (module names do not appear in BREAKING_CHANGES.md for these versions).
- 9.0.0/9.0.1 breaking changes (SVG/Drawable rework, Linux EGL, zlib/libpng C-mode, WebView package path) are all outside the 8.0.x line and further away; noted only for completeness, not required for an 8.0.4→8.0.x patch bump.

## Recommendation (for Builder/human decision, not acted on here)

Bumping `GIT_TAG` from `8.0.4` to `8.0.8` or later (currently `8.0.15` is latest 8.0.x per CHANGE_LIST.md) is the most direct upstream fix path for the Bluetooth HFP overflow, gated on:
1. Auditing the two breaking changes above (`OpenGLFrameBuffer::readPixels/writePixels` RowOrder param; `AudioTransportSource::hasStreamFinished` semantics) for call sites in this codebase.
2. Re-running the ASan repro (BT HFP default in+out) against the bumped JUCE to VERIFY the overflow is actually gone — this research did not run or build against a newer JUCE, so the fix's applicability is INFERRED from the commit/forum description, not confirmed against this app's exact allocation math (1296 bytes / `total_channels*(bufferSize+4)` floats).
