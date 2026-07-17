#pragma once
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
    }
};
