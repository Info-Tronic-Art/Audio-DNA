# Audio-DNA: Lessons Learned

> Only verified fixes go here. Each entry has been confirmed working by a human.

---

## LL-001: JUCE AudioTransportSource requires a TimeSliceThread for buffered playback

**Date**: 2026-03-13
**Milestone**: M1
**Symptom**: SIGSEGV crash on `pthread_mutex_lock` when loading an audio file (MP3). Crash in `juce::TimeSliceThread::moveToFrontOfQueue()` called from `juce::BufferingAudioSource::setNextReadPosition()`.
**Root Cause**: `AudioTransportSource::setSource()` was called with `nullptr` as the `TimeSliceThread*` parameter while requesting a non-zero read-ahead buffer size (32768 samples). JUCE's `BufferingAudioSource` needs a valid thread to perform background disk I/O. Dereferencing the null thread pointer caused the segfault.
**Fix**: Added a `juce::TimeSliceThread readAheadThread_{"AudioReadAhead"}` member to `AudioEngine`. Start it in the constructor, pass `&readAheadThread_` to `setSource()`, stop it in the destructor.
**Files**: `src/audio/AudioEngine.h`, `src/audio/AudioEngine.cpp`
**Verified By**: Human confirmed app loads and plays MP3 without crash.
**Verification Date**: 2026-03-13
**Rule**: When using `AudioTransportSource::setSource()` with a non-zero buffer size, always provide a valid `TimeSliceThread*`. Passing `nullptr` with buffer size 0 is also valid (no buffering), but not recommended for file playback.

---

## LL-002: JUCE Recreates GL Context on Window Move/Resize

**Date**: 2026-03-14
**Milestone**: M5
**Symptom**: Output window shows black screen despite image loaded and shaders compiled. Console shows GL context created twice — second time has no image.
**Root Cause**: JUCE destroys and recreates the OpenGL context when a component's parent window moves, resizes, or changes visibility (`setBounds()`, `setVisible()`). All GL state (textures, FBOs, shader programs) is lost. The image was loaded between the first and second context creation, so it was wiped.
**Fix**: Store the last loaded image file path outside of GL state. In `newOpenGLContextCreated()`, re-queue the image for loading if one was previously loaded.
**Files**: `src/ui/OutputWindow.h`, `src/ui/OutputWindow.cpp`
**Verified By**: Human confirmed output window displays image after selecting display.
**Verification Date**: 2026-03-14
**Rule**: Any OpenGL renderer that may have its context recreated must reload all GL resources (textures, shaders, FBOs) in `newOpenGLContextCreated()`. Never assume GL state persists across context recreation.

---

## LL-003: macOS Native Fullscreen Breaks GL Rendering

**Date**: 2026-03-14
**Milestone**: M5
**Symptom**: Using `setFullScreen(true)` on macOS creates a new desktop Space. GL context is destroyed during the animation and may not reinitialize properly, resulting in permanent black window.
**Root Cause**: macOS native fullscreen triggers a Space transition that JUCE handles by detaching and reattaching the GL context. The timing of this transition can leave the context in a bad state.
**Fix**: Don't use native macOS fullscreen for GL output windows. Instead, simulate fullscreen with a borderless window: `setTitleBarHeight(0)`, `setUsingNativeTitleBar(false)`, `setBounds(display.totalArea)`, `setAlwaysOnTop(true)`.
**Files**: `src/ui/OutputWindow.cpp`
**Verified By**: Human confirmed fullscreen output works on primary display.
**Verification Date**: 2026-03-14
**Rule**: For any window hosting an OpenGL context on macOS, avoid `setFullScreen(true)`. Use borderless + setBounds + alwaysOnTop instead.

---

## LL-004: EffectChain::render() Overrides glViewport

**Date**: 2026-03-14
**Milestone**: M5
**Symptom**: Letterboxing set via `glViewport()` before calling `effectChain_.render()` is ignored — image stretches to fill entire component.
**Root Cause**: `EffectChain::render()` internally calls `glViewport(0, 0, width, height)` for every pass including the final blit to the default framebuffer, overriding any previously set viewport.
**Fix**: Added explicit viewport parameters (`vpX, vpY, vpW, vpH`) to `EffectChain::render()`. Intermediate FBO passes use full FBO dimensions; only the final pass to the default framebuffer uses the letterboxed viewport.
**Files**: `src/effects/EffectChain.h`, `src/effects/EffectChain.cpp`, `src/render/Renderer.cpp`
**Verified By**: Human confirmed image displays centered with correct proportions at locked resolution.
**Verification Date**: 2026-03-14
**Rule**: Never set `glViewport()` before calling a function that also sets it internally. Pass viewport parameters through the call chain so the final output pass can apply letterboxing.
