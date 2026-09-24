# R13 plan — adversarial critic (lanes B then A only; C/D deferred)

Critic: Architect (Fable), 2026-09-24, read-only. Plan under review: `.harmony/.reports/s-rta-0924/r13-plan.md`. Disk HEAD `546b66c` (plan authored at `514976f`; the only commit since is the plan doc — no source drift).

## VERDICT: BUILDABLE-WITH-FIXES

The design holds (resample on the analysis thread, bypass at 48 kHz, gate the spectrum, publish provenance). Every A/B `file:line` cited resolves on disk; the sacred rules are respected; the bypass path is bit-identical; the snapshot layout claim is compiled-verified. Four test specs must be amended before a builder writes fail-first tests against them, and the wave's live gate must be replaced because it depends on work this wave does not build.

## VERIFIED against disk (A/B-relevant)

- `AnalysisThread.h:45-47,128,149` constants; `AnalysisThread.cpp:18-46` (all 12 modules built once with `kSampleRate`), `:63-91` loop head, `:305-308` timing, `:325-339` profile block (13 names, loop `< 13`, `kNumStages = 14` → slot 13 free for "Resample"). VERIFIED.
- `AudioCallback.cpp:8-45` callback body (memcpy/downmix + push only), `:47-50` `audioDeviceAboutToStart` (already allocates there by design). `CombinedCallback.h:145` forwards `audioDeviceAboutToStart` to `AudioCallback` → the rate store is reachable. `AudioEngine.cpp:21` registers `combinedCallback_`. VERIFIED.
- JUCE 8.0.4 (`build/_deps/juce-src`): `juce_CoreAudio_mac.cpp:686-693` `start()` calls `audioDeviceAboutToStart` BEFORE `AudioDeviceStart` (`:721`); `juce_AudioDeviceManager.cpp:971-981, 1074-1085`. G5 VERIFIED. `juce_AudioTransportSource.cpp:86-95` builds a `ResamplingAudioSource` with ratio `source/deviceRate`. G3 VERIFIED.
- `juce_GenericInterpolator.h:84-89` `reset()` = 3 scalar stores + `std::fill` over `float lastInputSamples[5]` (`:391`); strict `process()` (`:101-111`) returns the count consumed; pushes per call ≤ `floor(1 + ratio*numOut)` (`:365-388`, `subSamplePos ∈ [ratio, 1+ratio)` at entry). G17 VERIFIED; `inputNeededFor = ceil(ratio*numOut)+2` is a safe bound; at `kMaxRatio 8`: 4098 ≤ 4160. No heap on reset/rate change. Butterworth biquads: 7 floats each — no heap.
- `SpectralFeatures.cpp` loops `:45,:58,:80,:90,:119`; `0.9995` at `:69,:145`; bands `:132-150`; rolloff default `:118`. G8 VERIFIED. Default bandwidth 24000 → `binLimit_ = min(1025, floor(24000*2048/48000)+1) = 1025` and `bandRangesEff_ == bandRanges_` → bit-identical.
- Validity arithmetic: 8000 → Brilliance 14 % invalid, Presence 100 % → 0x3F; 4000 → 0x1F; 11025 → 0x3F; 16000 → 71 % → 0x7F; 24000 → 0x7F. T3.1 table VERIFIED.
- `FeatureSnapshot.h:5-98` + `FeatureBus.h:127-141`: compiled a verbatim copy with the two appended fields (clang++ -std=c++20): `reeseBass@304 sourceSampleRate@308 bandValidMask@312 sizeof=320 alignof=64`, trivially copyable. G10 VERIFIED — `FeatureBus.h` stays untouched. `FeatureBus.cpp:10` (`staging_.clear()`) and `TestServer.cpp:437` (`injected.clear()`) both route through `clear()` → 0x7F default propagates to pre-first-publish and test-mode reads.
- `MainComponent.h:252-254` member order ring → engine → analysis (lifetime of the cell pointer is safe: engine outlives the analysis thread, whose dtor `stopThread(1000)` runs first). Only constructor site (`grep`). Default arg keeps `AnalysisThread analysisThread_{ringBuffer_}` compiling → until lane D the app runs permanent bypass. VERIFIED.
- No ctest target includes `AnalysisThread.h` (test_waveform_snapshot mentions it in a comment only) → adding `<juce_audio_basics/...>` via `AnalysisResampler.h` breaks no Catch2-only target; app links `juce_audio_basics` (`CMakeLists.txt:539`). `test_integration_pipeline` already links `juce_audio_basics` (`tests/CMakeLists.txt:144`). VERIFIED.
- `ctest -N` → 377 tests. Existing `test_spectral_features.cpp` cases all construct at 48000 with no bandwidth call → T3.4 claim holds. VERIFIED.
- Warnings: `-Wold-style-cast -Wconversion -Wsign-conversion -Wdouble-promotion`, no `-Werror` (`cmake/CompilerWarnings.cmake:13-27`). G21 VERIFIED.
- Concurrency: A owns `src/analysis/AnalysisResampler.*`, `src/analysis/AnalysisThread.*`, `src/audio/AudioCallback.*`, `src/audio/AudioEngine.h`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/test_analysis_resampler.cpp`, `tests/test_integration_pipeline.cpp`; B owns `src/analysis/SpectralFeatures.*`, `src/analysis/FeatureSnapshot.h`, `tests/test_spectral_features.cpp`. NONE under `src/recording/*` or `src/MainComponent.*`. Only one worktree exists (main); the step-3 fix round has not branched yet (work log: diagnosis agent dispatched, no src edits). Shared-surface risk = the two CMake files (additive) and possibly `AudioEngine.h` (A adds one accessor; the fix round has no known reason to touch it). VERIFIED / INFERRED for the fix-round scope.
- §3.9 (`ApiServer.cpp`) is needed by nothing in A or B: T1/T2/T3 are headless; §7 "done when" for A/B does not reference REST. VERIFIED. (`handleGetFeatures` at `:634`, not `handleFeatures` — lane D nit.)

## RE-DERIVED numerics (independent of the plan's script)

- 5-point Lagrange kernel DTFT: passband −0.28 dB @0.25 fs_in, −0.69 @0.30, −1.42 @0.35, −2.57 @0.40, −4.22 @0.45; images −6.5 dB @0.5, −10.1 @0.56, −13.2 @0.6, −31.1 @0.75, ≈ −92 dB at fs_in ± 0.06 (5th-order null). Image levels match G18; passband is ~1 dB worse than G18 at 0.45 fs_in (plan: −3.06). Risk §8.1 should say "up to ~−4 dB at 19.8 kHz on a 44.1 k device".
- 4th-order Butterworth fc 21.6 kHz: −0.4 dB @16 k, −1.9 @20 k, −5.2 @24 k, −11.7 @30 k, −21.4 @40 k, −24.7 @44 k. 8th-order would be −1.1 @20 k, −22.8 @30 k, −42.8 @40 k. At ratio exactly 2.0 the Lagrange stage evaluates at offset 0 every output = pure decimation → the prefilter is the ONLY anti-alias element. T1.4 "30 kHz → < 1 %" (−40 dB) is unpassable as written.
- Empirical BPM (this rig, `BPMTracker.cpp` + `/opt/homebrew/lib/libaubio.dylib`, 120-BPM train, 20 ms noise bursts, −6 dBFS, 12 s, fixed seed; tracker at 48 k): 48 k control → LOCKED at 8.43 s, bpm 121.37 (raw 121.7, conf 1.54); 44.1 k fed unresampled → LOCKED, 131.86; 16 k fed unresampled → never locks (bpm 0, SEARCHING); 96 k fed unresampled → 60.34. `BPMTracker` locks after 24 confident hops (`BPMTracker.cpp:134-137`, `kConfidenceThreshold 0.1`).

## BLOCKING (apply before any builder writes tests)

1. T1.4 anti-alias case is unpassable with the specified prefilter (−11.7 dB at 30 kHz vs the demanded −40 dB). Amend per A-3.
2. §5 "Live gates" (`/api/features.sourceSampleRate == 48000`, `bandValidMask == 127`) need §3.9 AND lane D — neither is in this wave; until D the app publishes `sourceSampleRate = 0`. Replace per A-1 or the wave's gate fails by construction.
3. T2.4 (and the gating half of T2) cannot pass unless the test itself calls `setInputBandwidthHz` on the PipelineRunner — it is not `AnalysisThread`. Amend per A-5.
4. T3.2's spectrum shape makes old-code Brilliance read 1.0 for real 6-8 kHz energy, not the residue mode it claims to pin. Amend per B-3.

## AMENDMENTS (binding, verbatim — lanes B and A)

See the structured output `amendments` field (identical text).

## RISKS / non-binding notes

- §8.1 passband wording (−4 dB, not −3 dB, at 0.45 fs_in). §6 "≥ 96 kHz: normal" → "−1.9 dB at 20 kHz from the prefilter". Neither changes the decision.
- The one `std::cerr` line on a rate change is a syscall on the analysis thread — same class as the existing profile print (`AnalysisThread.cpp:331`); acceptable, keep it to that one line.
- Hot-swap residue (≤ ~30 ms mis-rated audio) accepted as in the plan §8.3.
- Fix-round overlap: none by file; re-check `git diff --stat` of the fix-round branch against `AudioEngine.h`, `CMakeLists.txt`, `tests/CMakeLists.txt` at merge time.

STATUS: CRITIC COMPLETE — BUILDABLE-WITH-FIXES; 4 blocking amendments (A-1, A-3, A-5, B-3) + binding amendments A-2, A-4, A-6, A-7, A-8, B-1, B-2, B-4.
