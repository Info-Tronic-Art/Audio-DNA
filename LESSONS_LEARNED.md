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
