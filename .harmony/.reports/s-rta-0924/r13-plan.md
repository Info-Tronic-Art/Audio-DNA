# R13 plan — non-48 kHz devices: resample to the internal 48 kHz on the analysis thread, gate the spectrum to the device bandwidth, retire the recorder's `rateMismatch` meaning

Author: Architect (Fable), 2026-09-24. Read-only planning; nothing below is built.
Repo HEAD at authoring: `514976f` (main). Concurrent step-3 fix round is in flight on `src/MainComponent.{h,cpp}` (+ possibly `.harmony/probe-step3.sh`) — every hunk in those files is tagged **[POST-MERGE]** and lands only after that round merges.

---

## 0. VERDICT / APPROACH (read this first)

**Do (c), weighted almost entirely toward (a): resample the device stream to a fixed internal 48 kHz on the ANALYSIS thread, immediately after popping from the ring buffer, and keep every analysis stage exactly as it is at 48 kHz. Add one small, contained piece of rate-awareness — `SpectralFeatures` learns the device bandwidth (Nyquist) so bands and spectral statistics above it are reported as absent (0 + cleared validity bit) instead of normalised garbage. Publish the device rate and the band-validity mask in `FeatureSnapshot`. Retire the recorder's `rateMismatch` ("device ≠ 48000") and replace it with `rateChangedSinceArm` ("device rate ≠ rate at arm"), which is the only rate hazard the take still has.**

Why this and not (b) "make every stage rate-aware": the pipeline is not merely parameterised by a rate — it is BAKED at 48 kHz in ~30 places that are not constructor arguments: hop-count windows (`kOnsetWindowSize=256`, `kMedianWindowSize=48`, `kHysteresisHops=200`, `kEnvelopeLength=64`, `kIOIHistorySize=32`), per-hop EMA decays (`0.9995`), the K-weighting biquad coefficients (`LoudnessAnalyzer.cpp:11-29` are the ITU 48 kHz literals), the FFT bin resolution (23.4 Hz) and the feature cadence (93.75 Hz) that every smoothing constant downstream (SignalRegistry, connections, autopilot) was tuned against. Making all of that rate-aware is a wide, silent-failure surface (each constant a potential 3x error at 16 kHz) with a test matrix multiplied by the number of rates — for zero correctness gain over resampling. Resampling keeps the 48 kHz path BIT-IDENTICAL (bypass when the device is 48 kHz), keeps all 377 existing tests valid, and makes the analysis thread's timebase (`wallClockSeconds`, hop period) real-time-correct at any device rate.

Cost: one new class (`AnalysisResampler`, ~150 lines), ~40 lines in `AnalysisThread`, ~50 lines in `SpectralFeatures`, 2 snapshot fields, an atomic rate cell in `AudioCallback`, recorder rename, tests. Roughly one builder-day across 4 lanes. Zero allocation and zero locks on the analysis thread — including at a device hot-swap.

---

## 1. GROUND TRUTH (everything the plan rests on)

| # | Claim | Evidence | Confidence |
|---|-------|----------|------------|
| G1 | Analysis rate is a compile-time constant; every analyzer is constructed ONCE with it | `src/analysis/AnalysisThread.h:45-47` (`kBlockSize 2048, kHopSize 512, kSampleRate 48000`); `AnalysisThread.cpp:23-41` (all 12 modules built with `kSampleRate`/`kHopSize`); `:45` `hopsPerSecond_` | VERIFIED |
| G2 | The ring buffer carries raw DEVICE-rate mono samples; the analysis thread pops exactly 512 of them per hop and treats them as 48 kHz | `src/audio/AudioCallback.cpp:20-44` (downmix + `push`); `AnalysisThread.cpp:61-91` (`pop(hopBuffer, kHopSize)`); `:306-308` (`wallClockSeconds = total / kSampleRate`) | VERIFIED |
| G3 | In FILE mode the transport already resamples the file to the DEVICE rate, so the ring buffer is device-rate in BOTH modes | `src/audio/AudioEngine.cpp:49-50` passes `reader->sampleRate` as `sourceSampleRateToCorrectFor`; JUCE `juce_audio_devices/sources/juce_AudioTransportSource.cpp:86-97, 236-237` builds a `ResamplingAudioSource` with ratio `source/deviceRate` | VERIFIED |
| G4 | The audio callback must stay alloc/lock/IO-free (sacred rule) | `CLAUDE.md` Development Rules 1; `AudioCallback.cpp` is memcpy + push only | VERIFIED |
| G5 | `audioDeviceAboutToStart(device)` is called with the callback thread quiesced, BEFORE the first block of a (re)started device, and the app already relies on that to allocate there | `AudioCallback.cpp:47-50` (`monoBuffer_.resize`); JUCE `juce_CoreAudio_mac.cpp:686-693` (`start()` calls `audioDeviceAboutToStart` before installing the IOProc); `juce_AudioDeviceManager.cpp:971-981` (`addAudioCallback` fires it immediately if a device is open); `:1074-1088` (`audioDeviceAboutToStartInt`). Device restarts on rate change go through `restart()`/`restartAsync()` (`juce_CoreAudio_mac.cpp:1372-1376, 1637`) = stop → aboutToStart → start | VERIFIED (JUCE 8.0.4 source in `build/_deps/juce-src`) |
| G6 | Rate-dependent math inside analyzers: bin↔Hz in `SpectralFeatures.cpp:28-29,47,124`, `MFCCExtractor.cpp:52`, `ChromaExtractor.cpp:30`, `AdvancedAudioAnalyzer.cpp:14-28`; hop timing in `BPMTracker.cpp:34-36,200-201`, `StructuralDetector.cpp:3-24`, `GenreDetector.cpp:6-15`, `AdvancedAudioAnalyzer.cpp:12`; aubio objects take the rate at construction (`OnsetDetector.cpp:8-13`, `BPMTracker.cpp:10-15`, `PitchTracker.cpp:8-13`); K-weighting coefficients are 48 kHz literals (`LoudnessAnalyzer.cpp:11-29`, window `:6`) | VERIFIED |
| G7 | Hop-count constants NOT parameterised by rate: `AnalysisThread.h:128` (`kOnsetWindowSize 256`), `BPMTracker.h:44,47` (48 / 200 hops), `AdvancedAudioAnalyzer.h:43,53` (64 / 32), `SpectralFeatures.cpp:69,145` (`0.9995` per-hop decay) | VERIFIED |
| G8 | Spectral stats iterate ALL bins to Nyquist; flatness skips only exact-zero bins; each band is normalised by its OWN running max | `SpectralFeatures.cpp:45-50` (centroid), `:57-63` (flux), `:90-99` (flatness, `p > 1e-20`), `:119-127` (rolloff), `:132-150` (bands, `bandMaxEnergy_`) — so a band that contains only residue above the device Nyquist normalises that residue to 1.0 (the "garbage" mode) | VERIFIED |
| G9 | `FeatureSnapshot` is a 320-byte trivially-copyable POD with a size static_assert in the seqlock bus | `src/analysis/FeatureSnapshot.h:5-98`; `src/features/FeatureBus.h:127-132` (`sizeof == 320`, `kSnapshotWords = 80`) | VERIFIED |
| G10 | Content of `FeatureSnapshot` ends at byte 308; 12 bytes of tail padding exist under `alignas(64)` → adding `float` + `uint8_t` keeps sizeof 320 | hand layout count of `FeatureSnapshot.h:8-84` | INFERRED (the static_assert at `FeatureBus.h:129` is the check; if it fires, set `kSnapshotWords = 84` and `== 336`) |
| G11 | No consumer reads `snap.wallClockSeconds` or `snap.timestamp` outside the analysis thread | `grep -rn wallClockSeconds\|timestamp src` → only `AnalysisThread.cpp`, `FeatureSnapshot.h` | VERIFIED |
| G12 | `AnalysisThread` is constructed in exactly one place; member order is ring → engine → analysis | `src/MainComponent.h:252-254`; no test constructs it (`grep "AnalysisThread(" tests` empty) | VERIFIED |
| G13 | Recorder rate handling today: `ArmOptions.analysisRate` (default 48000) vs `deviceRate` → `rateMismatch_` + arm-time notify "beat clock unreliable (R13)"; `tick()` recomputes it every tick; published in `Status` | `src/recording/RecorderHost.h:88,97,194-197,237-238,281-282`; `RecorderHost.cpp:149-150,170-178,348-349,676-677` | VERIFIED |
| G14 | The take's `sample` stamps and WAV/segment rate are DEVICE-domain and self-consistent at any rate; a mid-take rate change self-stops the tap | `src/audio/CombinedCallback.h:105-134` (`deliveredSamples_`), `:142-160` (`tap.prepare(rate, …)`); `src/recording/AudioTap.cpp:35-69` (`rateOrChannelsChanged → stopInternal()`); `RecorderHost.cpp:366-372` (self-stop → `lastError_`) | VERIFIED |
| G15 | MainComponent wiring on current main (post-W2): arm passes `analysisRate = AnalysisThread::kSampleRate` (`src/MainComponent.cpp:2063`), `deviceRate = getCurrentSampleRate()` (`:2054`); `onPerfStatus` JSON emits `deviceRate`/`rateMismatch` (`:2155-2156`); `recorderHost_.tick(…, audioEngine_.getCurrentSampleRate())` (`:3468-3470`); the startup 48 kHz warn + modal is `:541-565` | VERIFIED at HEAD `514976f` |
| G16 | The live gate refuses to arm unless `deviceRate == 48000 && rateMismatch == false` and stderr lacks "analysis pipeline assumes" | `.harmony/probe-step3.sh:174-197`; `:266` asserts `segments[0].rate == 48000`; `:298` reads `rateMismatch` | VERIFIED |
| G17 | JUCE ships heap-free, fixed-size stream interpolators; strict `process(ratio, in, out, numOut)` returns the input count consumed; per call it pushes at most `floor(1 + ratio*numOut)` input samples | `juce_audio_basics/utilities/juce_GenericInterpolator.h` (`process`, `interpolateImpl` loop: `while (pos >= 1.0) push; pos += speedRatio`), `juce_Interpolators.h:106-111` (Lagrange, 5-sample memory, latency 2), `:49-104` (WindowedSinc, 200-sample memory, 201-tap loop, latency 100) | VERIFIED |
| G18 | Measured 5-point Lagrange response (offset-averaged): passband droop -0.27 dB @0.25 fs_in, -0.66 @0.30, -1.31 @0.35, -2.19 @0.40, -3.06 @0.45; upsampling image rejection of the continuous kernel: -6.5 dB @0.5 fs_in, -13 @0.6, -31 @0.75, ≥-30 dB for all of 0.75–3.0 fs_in except the notch-free band edges | computed this session (pure-Python DTFT of the Lagrange kernel; script executed, not stored) | VERIFIED (numeric) |
| G19 | Aubio research note says "do not resample to 44100; pass 48000 to constructors" | `research/LIB_aubio.md:936` | VERIFIED — consistent with this plan (aubio stays at 48000; the DEVICE stream is resampled TO 48 kHz). `research/ARCH_audio_io.md:1029` explicitly recommends a device-agnostic pipeline with resampling in the I/O layer |
| G20 | Analysis thread runs only in production mode; `--test-mode` injects snapshots | `src/MainComponent.cpp:1866-1870`; HANDOFF RIG FACTS | VERIFIED |
| G21 | App sources are enumerated in CMake (not globbed); warnings include `-Wconversion`, no `-Werror` seen | `CMakeLists.txt:98,104,109`; `cmake/CompilerWarnings.cmake:13,20` | VERIFIED / INFERRED (no `-Werror` in grep) |
| G22 | Bluetooth HFP devices flip between 16 kHz (mic active) and 44.1/48 kHz (A2DP) at runtime → hot-swap is a FREQUENT path on this rig, not an edge case | HANDOFF `:2633`; `.harmony/s-rta-0923-work.md:22`; `tests/test_bt_device_shapes.cpp:1-40` (models 48000/2ch → 16000/1ch restart) | VERIFIED (observed) |

---

## 2. DECISION AND TRADEOFFS

| Option | What it is | Verdict | Why |
|--------|-----------|---------|-----|
| **(a) Resample to internal 48 kHz on the analysis thread** | `AnalysisResampler` between `RingBuffer::pop` and the 2048-sample overlap window; every stage untouched | **ACCEPTED (core)** | Contains the change to ONE seam; 48 kHz path bit-identical (bypass); all G7 hop constants, LUFS coefficients, cadence, and tests stay valid; timebase becomes real-time-correct; hot-swap = reset a fixed-size interpolator, no allocation |
| (a') Resample in the audio callback | interpolator inside `AudioCallback::audioDeviceIOCallbackWithContext` | REJECTED | Violates the sacred rule's spirit (DSP on the RT thread, G4) for no benefit; the analysis thread has 5x headroom and already owns all DSP |
| **(b) Make every stage rate-aware** | rate/hop parameters threaded through all analyzers; re-derive K-weighting per rate; re-create aubio objects on switch | REJECTED as the primary fix | G6+G7: ~30 baked constants + 2 biquads + FFT resolution + cadence all change meaning per device; re-creating aubio objects on a hot-swap ALLOCATES on the analysis thread (or forces a stop/start); test matrix × rates; product feel changes per device (31 Hz feature cadence at 16 kHz with 512-sample hops) |
| **(c) Hybrid = (a) + bandwidth gating in `SpectralFeatures` + provenance fields** | as (a), plus the device Nyquist is known to the spectral stage | **ACCEPTED — this is the plan** | (a) alone leaves G8's garbage mode: Brilliance on a 16 kHz device normalises its own residue to 1.0, flatness collapses toward 0 from image residue in 2/3 of the bins. Gating is ~50 lines in one module and makes degradation honest |
| (d) Force the device to 48 kHz via `setAudioDeviceSetup` | ask CoreAudio for 48000 | REJECTED as the fix (optional preference later) | HFP profiles offer only 8/16 kHz; CoreAudio nominal-rate changes are system-wide and can fail/flap; does nothing for the 96 kHz interface case |
| Interpolator: `juce::LagrangeInterpolator` (chosen) vs `WindowedSincInterpolator` | 5-tap vs 201-tap | **Lagrange** | G18: droop ≤1.3 dB below 0.35 fs_in (15 kHz at 44.1 k); images above the source Nyquist land ONLY where the gating (c) already excludes them (bands/stats > Nyquist); MFCC (≤8 kHz), formant/resonance/reese ranges and chroma normalisation are insensitive at -6…-30 dB; aubio onset/tempo see mirrored (correlated) flux; LUFS sees ≤0.01 % power. WindowedSinc costs ~40x (201 MACs/output sample ≈ 0.3-0.5 ms/hop, INFERRED) and adds 100 source-samples of latency (6.25 ms at 16 kHz). The type is ONE `using` alias — swap if Boris's ear/eye disagrees |
| Anti-alias pre-filter for ratio > 1 (device > 48 kHz) | 4th-order Butterworth LP at 21.6 kHz (source-rate-relative), 2 hand-rolled biquads, applied at staging time | **ACCEPTED (small)** | Lagrange does not band-limit before decimation; a 96 kHz interface can carry 24-48 kHz content that would fold into 0-24 kHz. ~25 lines, zero cost when ratio ≤ 1, testable with a 30 kHz tone |

**Strongest counterargument to the recommendation and why it loses.** "Resampling throws away the device's native resolution (7.8 Hz bins at 16 kHz), adds DSP and interpolation artefacts, and aubio was designed to run at the native rate." It loses because (1) the pipeline's meaning is defined at 48 kHz in ~30 places that are not parameters (G6/G7) — native-rate analysis would change what every smoothing/hysteresis constant MEANS per device and would need to re-create aubio objects (allocation) on every Bluetooth HFP↔A2DP flip (G22); (2) the artefacts are measured (G18) and every consumer that could see them is either gated (c) or insensitive at the measured levels; (3) the 48 kHz path stays bit-identical, so the common rig cannot regress. The residual truth — ≤3 dB droop in the top 15 % of a 44.1 kHz device's band and -6…-13 dB images just above a 16 kHz device's Nyquist — is bounded, named in RISKS, and has a one-line escape hatch (WindowedSinc).

---

## 3. SPEC — file-by-file

### 3.1 NEW `src/analysis/AnalysisResampler.h` / `.cpp` (lane A)

Purpose: pull ONE 512-sample hop at 48 kHz out of a device-rate SPSC ring. Fixed-size, no heap, no locks, analysis-thread-only.

```cpp
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cstdint>
#include "audio/RingBuffer.h"

// R13: converts the device-rate mono stream in the analysis ring buffer to the
// fixed internal analysis rate (48 kHz). Lives on the analysis thread only.
// Zero heap after construction; a rate change (device hot-swap) resets state
// in O(1) with no allocation. When the source rate IS 48 kHz (or unknown, 0)
// pullHop() is a plain ring pop — bit-identical to the pre-R13 behaviour.
class AnalysisResampler
{
public:
    static constexpr double kTargetRate   = 48000.0;
    static constexpr int    kMaxOutputHop = 512;              // == AnalysisThread::kHopSize
    static constexpr int    kMaxRatio     = 8;                // source rates up to 384 kHz
    static constexpr int    kStagingCapacity = kMaxOutputHop * kMaxRatio + 64;   // 4160 floats

    using Interpolator = juce::LagrangeInterpolator;         // ONE-LINE swap point (WindowedSincInterpolator if ever needed)

    // Analysis thread only. `hz <= 0` means unknown -> bypass (treated as 48 kHz).
    // Reconfigures ratio, anti-alias filter, and resets interpolator + staging when the rate changes.
    void setSourceRate(double hz);
    double sourceRate() const        { return sourceRate_; }
    double ratio() const             { return ratio_; }        // source samples per output sample
    bool   isBypass() const          { return bypass_; }
    float  inputBandwidthHz() const;                            // min(sourceRate/2, kTargetRate/2); 24000 in bypass
    int    inputNeededFor(int numOut) const;                    // (int)ceil(ratio_*numOut) + 2 (>= JUCE's floor(1+ratio*numOut) bound, G17)

    // Produce exactly numOut samples at 48 kHz from `ring`. Returns false (and
    // produces nothing) when the ring does not yet hold enough source samples;
    // the caller sleeps and retries. NOTE: may have staged (popped) some source
    // samples before returning false — those are kept for the next call.
    bool pullHop(RingBuffer<float>& ring, float* out, int numOut);

private:
    struct Biquad { float b0=0,b1=0,b2=0,a1=0,a2=0,s1=0,s2=0; float tick(float x); void reset(); };
    void configureLowpass();   // RBJ LPF, fc = 0.45*kTargetRate, Q = {0.54120, 1.30656}; only used when ratio_ > 1

    double sourceRate_ = 0.0, ratio_ = 1.0;
    bool   bypass_ = true, lowpassActive_ = false;
    Interpolator interp_;
    Biquad lp1_, lp2_;
    std::array<float, kStagingCapacity> staging_{};
    int staged_ = 0;
};
```

`pullHop` algorithm (the whole class is this):
```
if (bypass_) { if (ring.availableToRead() < numOut) return false; ring.pop(out, numOut); return true; }   // == today
needed = inputNeededFor(numOut);            // <= kStagingCapacity by construction (assert)
while (staged_ < needed) {
    avail = ring.availableToRead(); if (avail == 0) return false;
    n = ring.pop(staging_ + staged_, min(avail, needed - staged_));
    if (lowpassActive_) for i in [staged_, staged_+n): staging_[i] = lp2_.tick(lp1_.tick(staging_[i]));   // filter each source sample exactly once
    staged_ += n;
}
used = interp_.process(ratio_, staging_.data(), out, numOut);   // strict variant; used <= needed (G17)
memmove(staging_, staging_ + used, (staged_ - used) * sizeof(float)); staged_ -= used;
return true;
```
`setSourceRate(hz)`: `bypass_ = (hz <= 0) || |hz - 48000| < 0.5`; `ratio_ = bypass ? 1 : clamp(hz/48000, 1/64, kMaxRatio)` (log once if clamped — unreachable in practice); `interp_.reset(); staged_ = 0; lowpassActive_ = ratio_ > 1.0; if (lowpassActive_) configureLowpass();` RBJ LPF per biquad: `w0 = 2π·21600/hz; α = sin(w0)/(2Q); b0 = (1−cos w0)/2; b1 = 1−cos w0; b2 = b0; a0 = 1+α; a1 = −2cos w0; a2 = 1−α;` divide all by a0; direct-form-II-transposed exactly like `LoudnessAnalyzer::processBiquad` (`LoudnessAnalyzer.cpp:32-39`).

CMake: add `src/analysis/AnalysisResampler.cpp` next to `AnalysisThread.cpp` in `CMakeLists.txt:104`.

### 3.2 `src/analysis/AnalysisThread.h` / `.cpp` (lane A)

- Ctor: `explicit AnalysisThread(RingBuffer<float>& ringBuffer, const std::atomic<double>* sourceRateCell = nullptr);` — `nullptr` ⇒ permanent bypass (headless/unknown). Store the pointer.
- Members: `AnalysisResampler resampler_; double lastSourceRate_ = -1.0;` (init -1 so the first loop iteration configures even for 0/48000).
- `kSampleRate` KEEPS its name and value; add the comment "internal analysis rate — the device stream is resampled to this (AnalysisResampler); the device rate is `FeatureSnapshot::sourceSampleRate`".
- `kNumStages` stays 14; name slot 13 "Resample" and print 14 stages in the profile block (`AnalysisThread.cpp:325-339`, loop bound 13 → 14).
- `run()` loop head (`AnalysisThread.cpp:63-91`) becomes:
```
const double rate = sourceRateCell_ ? sourceRateCell_->load(std::memory_order_acquire) : 0.0;
if (rate != lastSourceRate_) {
    lastSourceRate_ = rate;
    resampler_.setSourceRate(rate);
    spectralFeatures_->setInputBandwidthHz(resampler_.inputBandwidthHz());
    std::cerr << "[Analysis] source rate " << (int)rate << " Hz -> "
              << (resampler_.isBypass() ? "48 kHz path (no resampling)" : "resampling to 48000 Hz")
              << ", bandwidth " << (int)resampler_.inputBandwidthHz() << " Hz\n";   // once per change; message thread not involved
}
auto t0 = now();
if (!resampler_.pullHop(ringBuffer_, hopBuffer.data(), kHopSize)) { sleep(1); continue; }
stageTimesUs_[13] += elapsed(t0);
const size_t read = kHopSize;          // replaces the variable pop count; everything below unchanged
```
- Per hop, next to the timing block (`:305-308`): `snap->sourceSampleRate = (float)resampler_.sourceRate(); snap->bandValidMask = spectralFeatures_->bandValidMask();`
- `wallClockSeconds` and `hopPeriodUs` stay divided by `kSampleRate` — now correct real time at any device rate (today at 16 kHz the analysis clock runs 3x slow).
- Nothing else in `run()` changes. No analyzer is re-created on a rate change.

### 3.3 `src/analysis/SpectralFeatures.h` / `.cpp` (lane B — lands first)

- New API: `void setInputBandwidthHz(float hz);` (default = `sampleRate_/2` → identical to today), `float inputBandwidthHz() const;`, `uint8_t bandValidMask() const;` (bit b = band b; 0x7F = all valid).
- New state: `int binLimit_` (exclusive upper bin = `min(numBins_, (int)floor(hz*fftSize_/sampleRate_) + 1)`; 1025 for 24 kHz, 342 for 8 kHz), `std::array<BandRange,7> bandRangesEff_` (each `high = min(high, binLimit_)`), `uint8_t bandValidMask_ = 0x7F`, `float inputBandwidthHz_`.
- Validity rule (documented in the header): band b is valid iff `coverage = clamp((hz − edgeLo)/(edgeHi − edgeLo), 0, 1) >= 0.5`. Consequences: 16 kHz device (hz 8000) → Brilliance 6-20k covered 14 % → INVALID; Presence 4-6k 100 % → valid; mask 0x3F. 8 kHz HFP (hz 4000) → Presence 0 % invalid, HighMid valid → 0x1F. 22.05 kHz (11025) → Brilliance 36 % → invalid. 32 kHz (16000) → 71 % → valid (truncated summation). 44.1/48/96 → 0x7F.
- Loops `:45, :58, :80, :90, :119` use `k < binLimit_` instead of `k < numBins_`; band summation uses `bandRangesEff_`; an INVALID band reports `bandEnergies_[b] = 0` and does NOT update `bandMaxEnergy_[b]` (so switching back to 48 kHz recovers cleanly); rolloff default becomes `min(sampleRate_/2, inputBandwidthHz_)`.
- `setInputBandwidthHz` is pure arithmetic (no allocation); called from the analysis thread only.

### 3.4 `src/analysis/FeatureSnapshot.h` (lane B)

Append at the END of the struct (after `reeseBass`, before `clear()`), keeping sizeof 320 (G10):
```cpp
// R13 provenance. sourceSampleRate = the device rate the analysis was fed
// from (Hz; 0 = unknown/no device/test mode). Analysis itself always runs at
// AnalysisThread::kSampleRate (48 kHz): the device stream is resampled to it.
// bandValidMask: bit b set when bandEnergies[b] is meaningful at this source
// rate; bands mostly above the source Nyquist read 0 and have the bit clear
// (16 kHz Bluetooth HFP: bit 6 / Brilliance ("Air") is clear -> 0x3F).
float   sourceSampleRate = 0.0f;
uint8_t bandValidMask    = 0x7F;
```
`clear()` adds `bandValidMask = 0x7F;`. If `FeatureBus.h:129` fires, update `kSnapshotWords` (80 → 84) and the literal (320 → 336) — mechanical, same lane.

### 3.5 `src/audio/AudioCallback.h` / `.cpp` + `src/audio/AudioEngine.h` (lane A)

- `AudioCallback`: `std::atomic<double> sampleRate_{0.0};` + `const std::atomic<double>& sampleRateCell() const { return sampleRate_; }` + `static_assert(std::atomic<double>::is_always_lock_free)`. In `audioDeviceAboutToStart(device)` (`:47-50`): `sampleRate_.store(device->getCurrentSampleRate(), std::memory_order_release);` (message thread, callback quiesced — G5). `audioDeviceStopped()` leaves it (a stopped device pushes nothing).
- `AudioEngine.h`: `const std::atomic<double>& sourceSampleRateCell() const { return audioCallback_.sourceSampleRateCell…(); }` (one-liner next to `getCurrentSampleRate()` `:38`).
- The audio callback body (`AudioCallback.cpp:8-45`) is NOT touched.

### 3.6 `src/MainComponent.h` / `.cpp` — **[POST-MERGE, lane D]**

1. `MainComponent.h:254`: `AnalysisThread analysisThread_{ringBuffer_, &audioEngine_.sourceSampleRateCell()};` (member order 252-254 makes this valid).
2. `MainComponent.cpp:541-565`: DELETE the "Unsupported Sample Rate" cerr + modal (it is now false and fires on every launch with the headset). Replace with one info line when `actualSr != 48000`: `"[Audio] device " << sr << " Hz -> analysis resamples to 48000 Hz"`.
3. `MainComponent.cpp:2063`: delete `armOpts.analysisRate = …` (field removed, 3.7).
4. `MainComponent.cpp:2156`: `obj->setProperty("rateChangedSinceArm", s.rateChangedSinceArm);` (key rename, 3.7).
5. Optional: `getDeviceStatus()` label (`AudioEngine.cpp:81-87`) may append " (analysis 48 kHz)" — lane E.

### 3.7 `src/recording/RecorderHost.h` / `.cpp` + `tests/test_recorder_host.cpp` (lane C)

`rateMismatch` RETIRES in its current meaning and CHANGES to the only hazard that survives R13:
- REMOVE `ArmOptions::analysisRate` (`RecorderHost.h:93-97`) and `armedAnalysisRate_` (`:237`, `.cpp:149`).
- REMOVE the arm-time mismatch block + notify "beat clock unreliable (R13)" (`RecorderHost.cpp:170-178`). At arm: `armedDeviceRate_ = opts.deviceRate` (already), `rateChangedSinceArm_ = false`, `rateChangeNotified_ = false`.
- RENAME `Status::rateMismatch` → `rateChangedSinceArm` (`:197`, `:282`, `.cpp:349, 677`); semantics in `tick()`: `rateChangedSinceArm_ = deviceRate > 0 && armedDeviceRate_ > 0 && |deviceRate − armedDeviceRate_| > 0.5` (recording or not; it is a Status fact). On the false→true edge while `recording_`: `lastError_ = "device sample rate changed mid-take: <armed> -> <now> Hz; `sample` stamps after this point are mixed-domain (audio tap stopped if it was running)"` + `dispatch.notify` ONCE (edge-triggered). This covers the audio == false case that the tap self-stop path (`:366-372`) cannot see.
- Keep `Status::deviceRate`. The take format does not change (`sample` stays device-domain, G14).
- Comments in `src/api/ApiServer.cpp:1198, 1317-1319` mention `rateMismatch` — update wording only.
- `tests/test_recorder_host.cpp:155`: delete `opts.analysisRate = 48000.0;`. New cases: (i) arm with `deviceRate = 44100`, tick at 44100 → `status().rateChangedSinceArm == false`, `lastError` empty, ZERO notifies (FakeDispatch must count notifies; add a counter if it does not); (ii) arm at 48000, tick at 48000 then tick at 16000 → `rateChangedSinceArm == true`, `lastError` non-empty, exactly ONE notify across three further ticks at 16000; (iii) the existing self-stop case (`:700-745`) keeps passing (it now also gets the rate-change message — assert `lastError` non-empty only).

### 3.8 `.harmony/probe-step3.sh` — **[POST-MERGE, lane D]**

- `:174-197`: precondition becomes "perf/status readable, `deviceRate` > 0, `rateChangedSinceArm == false`"; DELETE the `== 48000` requirement and the stderr grep (`:187-190`, and the `R13_OK` line `:192-194`). Add an informational row: `/api/features` → `sourceSampleRate == deviceRate` and `bandValidMask` printed.
- `:266`: `segments[0].rate == $DEV_RATE` (the value read at arm), not the literal 48000.
- `:298`: read `rateChangedSinceArm`.

### 3.9 `src/api/ApiServer.cpp` (lane A, 2 lines, optional but cheap for the live probe)

`handleFeatures` (`:636-660`) adds `sourceSampleRate` and `bandValidMask`; `handleStatus` (`:280-300`) adds `sourceSampleRate`. (These read the snapshot copy already taken; no threading change.)

### 3.10 Tests + CMake (see §5 for content)

- NEW `tests/test_analysis_resampler.cpp` + target in `tests/CMakeLists.txt` (lane A; links `AnalysisResampler.cpp`, `juce_core`, `juce_audio_basics`, `juce_dsp` for the FFT helper, `juce_audio_devices` if the FakeAudioIODevice cell test is included — copy the `test_audio_tap_sync` block `:325-358` as the template).
- `tests/CMakeLists.txt:125-137`: add `${SRC_DIR}/analysis/AnalysisResampler.cpp` to `test_integration_pipeline` (lane A).
- `tests/test_integration_pipeline.cpp`: new rate-invariance cases (lane A) — no CMake change beyond the line above.
- `tests/test_spectral_features.cpp`: bandwidth/mask cases (lane B) — no CMake change.
- `tests/test_recorder_host.cpp`: 3.7 cases (lane C) — no CMake change.
- Only lane A touches `tests/CMakeLists.txt` → no CMake merge conflicts (repo gotcha: interleaved test targets).

### 3.11 Docs (lane D)

`CLAUDE.md`: 4-thread model (`:25-28`: "pulls device-rate samples … resamples to the internal 48 kHz (AnalysisResampler) …"), chain diagram `:126`, latency table `:138` (+ row "2b. Resample (non-48 kHz devices only) ~20-60 µs/hop"), `:339`, Debugging Audio Issues `:870` ("the ANALYSIS domain is 48 kHz; the DEVICE/recorder domain is the device rate — never assume either is the other"), FeatureSnapshot table (+2 rows), Common Pitfalls #29 (the two domains + "bands above the device Nyquist are absent, not zero-energy"). `.harmony/APP-INVENTORY.md:292` row → resolved. `HANDOFF.md` loose-end 3 → resolved; step-3 critic B5/A5 → superseded by this plan.

---

## 4. THREADING ANALYSIS (including device hot-swap)

| Thread | Before R13 | After R13 | Alloc/lock/IO in steady state |
|--------|-----------|-----------|-------------------------------|
| Audio callback (RT) | downmix + `ring.push` | UNCHANGED | none (unchanged) |
| Message thread, `audioDeviceAboutToStart` | `monoBuffer_.resize`; `tap.prepare` | + one `atomic<double>` release-store of the device rate | allocation already happens here by design (G5); not a hot path |
| Analysis thread | `pop(512)` → pipeline | acquire-load the rate cell (1 atomic/iter); on change: `setSourceRate` (reset fixed arrays, recompute ~10 floats), `setInputBandwidthHz` (recompute 7 bin ranges); every hop: `pullHop` (pops, optional 2 biquads, Lagrange over 512 outputs, memmove ≤ 4 k floats) | ZERO heap, ZERO locks, ZERO syscalls beyond the existing `sleep(1)`; the one `std::cerr` line fires only on a rate change (same class as the existing periodic profile print) |
| GL / UI / API threads | read snapshot copies | read 2 more POD fields | none |

Hot-swap sequence (e.g. HFP flip 48 k → 16 k, G22): JUCE stops the device (callback quiesced) → `audioDeviceAboutToStart(16 kHz)` on the message thread → `AudioCallback` stores 16000 (release) [+ `monoBuffer_` resize, `tap.prepare` as today] → device starts → audio thread pushes 16 kHz samples. The analysis thread, at its next iteration (≤ 1 ms + one hop), sees the new rate (acquire) and reconfigures in O(1). Residue: whatever OLD-rate samples were still in the ring (≤ ~1 hop, since the thread drains continuously) plus any partial staging are interpreted at the new ratio → ≤ ~10-30 ms of mis-rated audio, invisible behind multi-second smoothing. No drain is attempted (an SPSC consumer draining while the producer restarts would just drop a few NEW samples — pointless). Analyzer internal state (EMAs, running maxes, aubio history) carries across the switch; it re-adapts within seconds; invalid bands' running maxes are frozen (3.3) so nothing "learns" garbage. Ordering guarantee "rate store happens-before first new-rate push": G5 (aboutToStart precedes IOProc install in `CoreAudioIODevice::start`).

Ring capacity note: 16384 floats = 341 ms at 48 k, 170 ms at 96 k, 85 ms at 192 k, 1.02 s at 16 k. The analysis thread polls every 1 ms; R12's silent-overflow margin halves at 96 k — still ~16x the hop period. No change.

CPU: Lagrange ≈ 5 coefficient products + push per output sample → ~20-60 µs per 512-sample hop (INFERRED; the new "Resample" profile slot verifies). Bypass at 48 kHz costs one atomic load.

---

## 5. TESTS (fail-first) — exact cases and tolerances

Fail-first protocol for the builder of lane A: write T2/T3 first with `AnalysisResampler::pullHop` temporarily forced to bypass (one line) — the non-48 k cases go RED with the WRONG values named below; remove the line, implement, GREEN. Keep the control (48 k) cases passing throughout.

**T1 — `tests/test_analysis_resampler.cpp` (NEW, lane A)**
- T1.1 bypass bit-exact: source 48000, push 10 hops of arbitrary data → `pullHop` output == input samples exactly.
- T1.2 sample-count accounting / no drift: for rate ∈ {8000, 16000, 22050, 32000, 44100, 88200, 96000, 192000}: push N = 20 s of source samples, count hops produced → `|hops*512 − N*48000/rate| ≤ 2*512`; and `used ≤ inputNeededFor(512)` on every call (assert inside the loop; G17 bound).
- T1.3 spectral fidelity: 1 kHz sine, amplitude 0.5, at each rate → resampled 48 k → 2048-pt FFT (`FFTProcessor`) → peak bin == `round(1000*2048/48000)` (=43) ±1 and output RMS within 2 % of 0.3536.
- T1.4 anti-alias: 30 kHz sine at 96000 → output RMS < 1 % of input RMS (the tone must vanish, not fold to 18 kHz). Fail-first: without the pre-filter the folded tone survives at roughly full amplitude.
- T1.5 rate switch mid-stream: 2 s at 16000, `setSourceRate(48000)`, 2 s at 48000 → no assertion failure, hop count per segment within ±2 hops of expectation; run under ASan (`apply_sanitizers`).
- T1.6 rate cell wiring (needs the `FakeAudioIODevice` from `test_audio_tap_sync.cpp:38-60`): `AudioCallback::audioDeviceAboutToStart(fake@16000)` → `sampleRateCell().load() == 16000`; then `@48000` → 48000.

**T2 — rate-invariance, `tests/test_integration_pipeline.cpp` (lane A)** — generate at the SOURCE rate, run through `AnalysisResampler` (via a `RingBuffer<float>`), feed hops to the existing `PipelineRunner`:
- T2.1 centroid: 1 kHz sine at 16000 / 44100 / 48000 → last snapshot `spectralCentroid == 1000 ± 25` (one bin). Fail-first values without resampling: 3000 (16 k), 1088 (44.1 k).
- T2.2 LUFS: 1 kHz sine at −20 dBFS at 16000 vs 48000 → `|lufs16 − lufs48| ≤ 0.5`. Fail-first: ≈ +2.5 LU at 16 k (the K-weighting shelf sees "3 kHz"; INFERRED from the BS.1770 curve — the builder records the actual red value).
- T2.3 BPM: 120-BPM click train (20 ms decaying noise burst, −6 dBFS, 12 s) at 44100 and 48000 → `bpm() == 120 ± 3` and `trackerState() == LOCKED`. Fail-first at 44.1 k: reads ≈ 130.6 (=120·48/44.1). 16000 is ALSO asserted `120 ± 3` after the fix, but is NOT used as the fail-first (aubio at 3x speed may pick a subharmonic of 360 that happens to be 120 — ambiguous red). The 48 k control must lock first; if it does not, tune the click, not the tolerance.
- T2.4 provenance: after T2.1 at 16000 the pipeline's `SpectralFeatures::bandValidMask() == 0x3F` and `bandEnergies[6] == 0`; at 48000 `0x7F`.

**T3 — `tests/test_spectral_features.cpp` (lane B, lands before A)**
- T3.1 mask rule: `setInputBandwidthHz` 24000 → 0x7F; 16000 → 0x7F; 11025 → 0x3F; 8000 → 0x3F; 4000 → 0x1F.
- T3.2 garbage-mode fail-first: flat spectrum 1.0 for bins ≤ 8 kHz and 1e-4 residue above (models image residue) with bandwidth 8000: Brilliance energy == 0 (old code: 1.0 — its own residue normalised); flatness > 0.9 (old code: collapses toward 0 because the 1e-4 bins dominate the log-mean); centroid within one bin of the in-band centroid (4 kHz).
- T3.3 truncated-but-valid band: bandwidth 16000 → bit 6 set, Brilliance sums only 6-16 kHz (construct a spectrum with energy only at 18 kHz → Brilliance == 0 after truncation).
- T3.4 default behaviour unchanged: without `setInputBandwidthHz` every existing case in the file passes untouched.

**T4 — `tests/test_recorder_host.cpp` (lane C)** — the three cases in §3.7. Fail-first: case (i) is RED today (arm at 44100 notifies "beat clock unreliable (R13)").

**T5 — optional (lane A, M): `tests/test_analysis_thread_rate_switch.cpp`** — real `AnalysisThread` + `RingBuffer` + `std::atomic<double>` cell + `FeatureBus::Writer`; push 3 s of 1 kHz at 16000 (cell 16000), poll `featureBus.read().spectralCentroid` → 1000 ± 40 and `sourceSampleRate == 16000`, `bandValidMask == 0x3F`; flip cell to 48000 + push 3 s at 48000 → centroid 1000 ± 40, mask 0x7F; run under ASan/TSan. Links all analyzers + aubio (copy the `test_integration_pipeline` target block). Timing-tolerant (poll ≤ 3 s).

**Live gates (Harmony)**: build rc 0; ctest (377 + new) green; production app on the built-in mic: `/api/features.sourceSampleRate == 48000`, `bandValidMask == 127`, `[Analysis Profile]` shows Resample 0 µs (bypass); `probe-step3.sh` green with the relaxed precondition. Then the only-Boris checks (§8).

---

## 6. LOW-RATE DEVICES — honest degradation, spelled out

| Feature | 16 kHz HFP (bandwidth 8 kHz) | 8 kHz HFP (4 kHz) | 22.05/24 kHz | 44.1 kHz | ≥ 96 kHz |
|---------|------------------------------|-------------------|--------------|----------|----------|
| bandEnergies 0-4 (Sub…HighMid ≤ 4 kHz) | normal | HighMid valid (100 %); lower normal | normal | normal | normal |
| Presence 4-6 k | normal | **absent** (0, bit 5 clear) | normal | normal | normal |
| Brilliance/"Air" 6-20 k | **absent** (0, bit 6 clear; 14 % coverage) | absent | absent (36-43 %) | normal | normal |
| centroid / rolloff / flatness / flux | computed over ≤ 8 kHz only — truthful for what the device delivers (centroid reads lower than a 48 k capture of the same source: that is physics, not a bug) | ≤ 4 kHz | ≤ Nyquist | normal (≤ 3 dB droop above 15 kHz, G18) | normal |
| MFCC (20-8000 Hz mel) | top 1-2 mel bands sit at the device edge (attenuated by the device's own AA filter) — slight timbre bias, no garbage | top ~half of the mel range absent → MFCC shape shifts; genre scoring degrades gracefully | fine | fine | fine |
| chroma / key / HCDF | fine (fundamentals + low harmonics ≤ 8 kHz) | fine | fine | fine | fine |
| onset / BPM / beat phase / bars | CORRECT timing (the point of R13); onsets from ≤ 8 kHz content — cymbal-only onsets weaker | correct timing | correct | correct (today 8.8 % off) | correct |
| pitch (yinfft) | fine | fine below 2 kHz | fine | fine | fine |
| LUFS / RMS / peak / DR | LUFS of the band-limited signal (slightly lower than full-band) — truthful | same | same | same | same |
| formant 300-3 k / reese 30-200 / resonance 200-8 k | fine / fine / edge at 8 k | fine / fine / truncated | fine | fine | fine |
| genre / energy / structural | thresholds tuned on 48 k content → "bright" genres score lower on band-limited input; ONLY-BORIS check | same, stronger | fine | fine | fine |
| File mode with a low-rate OUTPUT device | the transport resamples the file DOWN to the device (G3) → analysis quality is capped by the device even for a 48 k file. Honest but lossy; a future option is to analyse the file at its native rate (out of scope) | | | | |

Rule enforced by 3.3: an absent band reads exactly 0 and is flagged; it never reads "alive" on residue. UI today will show a flat "Air" meter at 16 kHz (honest). Lane E may render "n/a at 16 kHz".

---

## 7. LANES (disjoint by file; each in its own worktree AND its own cmake `-B` dir)

| Lane | Owns (exclusive) | Depends on | Size | Done when |
|------|------------------|------------|------|-----------|
| **R13-B** spectral gating + snapshot fields | `src/analysis/SpectralFeatures.{h,cpp}`, `src/analysis/FeatureSnapshot.h`, (`src/features/FeatureBus.h` only if the static_assert fires), `tests/test_spectral_features.cpp` | — | S (~2 h) | T3 green; all existing spectral tests untouched and green; `sizeof(FeatureSnapshot)` assert passes |
| **R13-C** recorder semantics | `src/recording/RecorderHost.{h,cpp}`, `tests/test_recorder_host.cpp`, `src/api/ApiServer.cpp` (comment wording only) | — (∥ with B) | S (~1-2 h) | T4 green; `grep -rn analysisRate src tests` → only the [POST-MERGE] `MainComponent.cpp:2063` line remains (lane D deletes it) |
| **R13-A** resampler + thread + rate cell | NEW `src/analysis/AnalysisResampler.{h,cpp}`, `src/analysis/AnalysisThread.{h,cpp}`, `src/audio/AudioCallback.{h,cpp}`, `src/audio/AudioEngine.h`, `CMakeLists.txt` (+1 line), `tests/CMakeLists.txt`, NEW `tests/test_analysis_resampler.cpp`, `tests/test_integration_pipeline.cpp`, `src/api/ApiServer.cpp` §3.9 (2 lines — coordinate with C's comment edits or leave to D) | B merged (calls `setInputBandwidthHz`/`bandValidMask`) | M (~4-6 h) | T1, T2 (fail-first recorded), optional T5 green; app builds with the OLD `MainComponent.h:254` signature still compiling (default arg) |
| **R13-D** wiring + gate + docs **[POST-MERGE]** | `src/MainComponent.{h,cpp}` (4 hunks §3.6), `.harmony/probe-step3.sh` §3.8, `CLAUDE.md`, `.harmony/APP-INVENTORY.md`, `.harmony/HANDOFF.md` loose-end | A + C merged AND the step-3 fix round merged | S (~1 h) | live gate green; `grep "analysis pipeline assumes"` gone; `/api/features` shows provenance |
| **R13-E** (optional) UI honesty | `src/ui/SignalStrip.cpp`, `src/ui/AudioReadoutPanel.cpp`, `src/ui/TopBar.cpp` | B merged | S | "Air: n/a at 16 kHz" style hint; device rate visible in the status label |

Order: B ∥ C → A → D (→ E). The `MainComponent.h:254` change is the ONLY thing that makes the resampler live in the app — until D lands, the app runs the bypass path (cell pointer defaults to `nullptr`) and nothing regresses.

---

## 8. RISKS (and the cheapest check for each)

1. **Interpolator fidelity (the strongest self-doubt).** Lagrange droop up to −3 dB at 0.45 fs_in (19.8 kHz on a 44.1 k device) and −6…−13 dB images just above a 16 kHz device's Nyquist (G18). Bounded and gated (§2), but it IS a quality choice. Check: T1.3 at 44.1 k with a 15 kHz tone (expect −1.3 ± 0.5 dB); if Boris's meters look dull on a 44.1 k interface, flip `using Interpolator = juce::WindowedSincInterpolator;` and re-run the profile slot (expect ~0.3-0.5 ms/hop).
2. **`sizeof(FeatureSnapshot)` static_assert.** INFERRED tail padding of 12 bytes. Check: it either compiles or names the exact fix (§3.4).
3. **Old-rate residue at a hot-swap (≤ ~30 ms).** Accepted; not testable meaningfully. If a visible hiccup appears at HFP flips, the fix is a generation counter next to the rate cell and a drain-on-change — ~15 lines, same files (lane A).
4. **The Bluetooth STARTUP crash (HANDOFF loose-end 2) is unrelated but will confound the live 16 kHz test** — 3/4 launches crashed inside JUCE's combiner before any app code. Boris deferred it. Do not attribute a startup crash to R13; the headless T5 + the 44.1 k interface are the reliable proofs.
5. **Recorder API rename.** `rateMismatch` → `rateChangedSinceArm` changes a JSON key the probe reads; both are in-repo (G16), no external consumer. If Harmony prefers zero churn, keep the key name and ONLY change semantics + wording — but then the name lies (it no longer means mismatch vs analysis); the plan recommends the rename.
6. **CPU on the analysis thread at 192 kHz** (ratio 4: 2048 source samples popped + filtered per hop): ~4x the Lagrange work plus 2 biquads on 2048 samples ≈ still < 150 µs (INFERRED). The profile slot is the check.
7. **Genre/energy thresholds on band-limited input** — degrade, not garbage. Only-Boris.
8. **Concurrent edits.** MainComponent and probe-step3.sh are being touched by the step-3 fix round; lane D waits. Lanes A/B/C touch no file that round is known to touch (`RecorderHost` was S3-A's, merged `c737d3d`; verify at dispatch with `git diff --stat` of the fix-round branch).

---

## 9. WHAT ONLY BORIS CAN CHECK

1. **The 16 kHz Bluetooth headset (soundcore P31i), production mode**: play a known-BPM track into the mic → TopBar BPM reads the track's tempo (today it does not), the beat wheel follows, "Air" meter is flat (expected, honest), Presence and below move. `/api/features.sourceSampleRate == 16000`, `bandValidMask == 63`.
2. **HFP ↔ A2DP flips mid-set** (start/stop a call, toggle the mic): no crash, BPM re-locks within a few seconds, no stuck meters.
3. **A 44.1 kHz interface/mixer** (his "T2 on the real interface" item): BPM exact (today 8.8 % high); the recorder's take `segments[0].rate == 44100` and `rateChangedSinceArm == false`.
4. **Product call**: should the "Air" strip say "not available at this sample rate" (lane E), or is a flat meter enough until the UI rewrite?
5. **Product call**: in file mode on a low-rate output device the analysis is capped by the device (§6 last row) — acceptable for now, or should file analysis run at the file's native rate later?

---

## 10. COMPACT DEFINITION OF DONE

- `ctest` all green (existing 377 + T1-T4, T5 optional) on a clean build; ASan build of T1.5/T5 clean.
- 48 kHz path bit-identical: T1.1; `[Analysis Profile]` Resample slot reads 0 µs on the built-in mic.
- Fail-first evidence recorded in the builder report for T2.1/T2.2/T2.3(44.1 k)/T3.2/T4(i) with the red values.
- `grep -rn "kSampleRate" src` → only `AnalysisThread.h`, `AnalysisThread.cpp`, `MainComponent.cpp` (info line), comments; `grep -rn analysisRate src tests` → empty; `grep -rn '"rateMismatch"' src .harmony/probe-step3.sh` → empty.
- Live: `probe-step3.sh` green with the relaxed R13 row; `/api/features` carries `sourceSampleRate`/`bandValidMask`.
- Docs per §3.11; HANDOFF loose-end 3 closed with the only-Boris list above carried forward.

STATUS: PLAN COMPLETE — read-only; nothing built. Lanes B ∥ C can dispatch now; A after B; D after A+C and the step-3 fix-round merge.
