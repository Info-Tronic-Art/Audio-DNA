#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

struct alignas(64) FeatureSnapshot
{
    // Timing
    uint64_t timestamp = 0;
    double   wallClockSeconds = 0.0;

    // Amplitude
    float rms  = 0.0f;
    float peak = 0.0f;
    float rmsDB = -100.0f;             // 20*log10(rms), dBFS

    // Loudness
    float lufs = -100.0f;              // Momentary loudness (ITU-R BS.1770)
    float dynamicRange = 0.0f;         // Crest factor (peak/RMS)

    // Transient density
    float transientDensity = 0.0f;     // onsets/sec in sliding window

    // Spectral
    float spectralCentroid  = 0.0f;    // Hz — center of mass of spectrum
    float spectralFlux      = 0.0f;    // Normalized — frame-to-frame spectral change
    float spectralFlatness  = 0.0f;    // [0, 1] — tonal vs noisy (Wiener entropy)
    float spectralRolloff   = 0.0f;    // Hz — frequency below 85% energy

    // 7-Band Energies (normalized)
    // Sub(20-60), Bass(60-250), LowMid(250-500), Mid(500-2k),
    // HighMid(2k-4k), Presence(4k-6k), Brilliance(6k-20k)
    float bandEnergies[7] = {};

    // Onset detection
    bool  onsetDetected  = false;      // true if onset this frame
    float onsetStrength  = 0.0f;       // raw onset detection function value

    // Rhythm / tempo
    float bpm       = 0.0f;           // stabilized/locked tempo estimate (BPM), 0 if unknown
    float beatPhase  = 0.0f;           // [0, 1) smooth sawtooth ramp between beats (locked BPM driven)
    uint8_t trackerState = 0;          // 0=searching, 1=locking, 2=locked

    // Metrical hierarchy (downbeat detection)
    uint8_t beatInBar = 0;             // 0-3 (0 = downbeat) — which beat in the bar
    float   barPhase = 0.0f;           // [0, 1) over 4 beats — bar-level sawtooth
    bool    downbeatDetected = false;  // true on the hop where beat 1 lands

    // Phrase tracking
    uint16_t barCount = 0;             // bars since last phrase reset
    uint32_t totalBarCount = 0;        // S168: monotonic bars since transport start --
                                        // same advance events as barCount, but NEVER
                                        // rewound by a phrase/structural reset (barCount
                                        // is, on entering a drop / leaving a breakdown).
                                        // Use for anything that must never jump backward
                                        // mid-gesture (e.g. OscillatorSignal).
    float    phrasePhase = 0.0f;       // [0, 1) sawtooth over N bars (configurable)

    // Structural
    uint8_t structuralState = 0;       // 0=normal, 1=buildup, 2=drop, 3=breakdown

    // Chroma & harmony
    float chromagram[12] = {};         // C through B pitch classes, normalized sum=1
    float dominantPitch  = 0.0f;       // Hz — detected fundamental frequency
    float pitchConfidence = 0.0f;      // [0, 1] — pitch detection reliability
    int   detectedKey    = -1;         // 0-11 (C=0), -1=unknown
    bool  keyIsMajor     = true;       // major vs minor

    // Timbral
    float mfccs[13] = {};              // Mel-frequency cepstral coefficients

    // Harmonic change
    float harmonicChangeDetection = 0.0f;  // HCDF — frame-to-frame chroma distance

    // Genre detection (P23)
    uint8_t detectedGenre = 6;             // 0=House, 1=Techno, 2=DnB, 3=HipHop, 4=Ambient, 5=Rock, 6=Pop/Electronic, 7=Jazz/Other
    float   genreConfidence = 0.0f;        // [0, 1] — how dominant the top genre is
    uint8_t energyState = 1;               // 0=low, 1=medium, 2=high — overall energy level
    float   genreScores[8] = {};           // Raw smoothed scores for all 8 genres

    // Advanced audio analysis (P25)
    float sidechainPump = 0.0f;            // [0, 1] — bass/mid anti-correlation (sidechain compression detection)
    float swingRatio = 0.5f;               // [0.5, ~0.67] — 0.5=straight, >0.5=swung timing
    float formantPresence = 0.0f;          // [0, 1] — vocal formant energy concentration (300-3000 Hz)
    float resonancePeak = 0.0f;            // [0, 1] — spectral kurtosis (sharp peaks vs flat)
    float reeseBass = 0.0f;               // [0, 1] — bass spectral spread (reese/wobble detection)

    // R13 provenance. sourceSampleRate = the device rate the analysis was fed
    // from (Hz; 0 = unknown/no device/test mode). Analysis itself always runs at
    // AnalysisThread::kSampleRate (48 kHz): the device stream is resampled to it.
    // bandValidMask: bit b set when bandEnergies[b] is meaningful at this source
    // rate; bands mostly above the source Nyquist read 0 and have the bit clear
    // (16 kHz Bluetooth HFP: bit 6 / Brilliance ("Air") is clear -> 0x3F).
    float   sourceSampleRate = 0.0f;
    uint8_t bandValidMask    = 0x7F;

    // Onset pulse-loss fix: onsetDetected/onsetStrength above are a ONE-HOP PULSE -- consumers
    // read FeatureBus::read() (always-latest triple/seqlock buffer), so a hop published while a
    // consumer wasn't looking is lost, timestamp bookkeeping or not (proven: the app's own
    // OnsetDetector config replayed offline over a click train detects ~all clicks, but the live
    // app's RecorderHost missed ~13% via its 120 Hz tick reading a slower, always-latest bus).
    // onsetCount is a monotonic count of onsets detected since AnalysisThread started, incremented
    // once per hop where onsetDetected is true and published in EVERY snapshot (not just the hop
    // the onset happened on). A consumer recovers exactly how many onsets it missed by comparing
    // consecutive counts (delta = new - old, unsigned subtraction so it stays correct across a
    // wrap) instead of ever needing to catch the pulse itself. Never reset mid-stream in
    // production -- clear() (below) only runs once, before AnalysisThread's first publish
    // (FeatureBus's constructor) or in test-mode-only harnesses (TestServer.cpp) that replace the
    // writer entirely; see RecorderHost::tick()'s onsetCountBaseline_ for the reference consumer.
    uint32_t onsetCount = 0;

    void clear()
    {
        std::memset(this, 0, sizeof(FeatureSnapshot));
        rmsDB = -100.0f;
        lufs = -100.0f;
        detectedKey = -1;
        keyIsMajor = true;
        swingRatio = 0.5f;  // 0.5 = straight timing
        detectedGenre = 6;  // struct default (Pop/Electronic)
        energyState = 1;    // struct default (medium)
        bandValidMask = 0x7F;
    }
};

// Layout proof (R13 onset-pulse-loss fix): onsetCount was added into the struct's existing tail
// padding (alignas(64) rounds sizeof up to a multiple of 64; 313 bytes of real fields left 7 bytes
// of padding before this change) so sizeof(FeatureSnapshot) stays exactly 320 -- FeatureBus.h's own
// static_assert(sizeof(FeatureSnapshot) == 320, ...) would fail to compile otherwise, since its
// seqlock payload word count (kSnapshotWords * sizeof(uint32_t)) is derived from this exact size.
static_assert(offsetof(FeatureSnapshot, onsetCount) == 316,
              "onsetCount must land in the struct's existing tail padding (alignof-4 after "
              "bandValidMask at offset 312+1=313, rounded up to 316) without moving any other "
              "field's offset -- if this fails, a field was inserted/resized somewhere above and "
              "the layout needs re-auditing, not just re-numbering this constant");
static_assert(sizeof(FeatureSnapshot) == 320,
              "adding onsetCount must not change the overall FeatureSnapshot size");
