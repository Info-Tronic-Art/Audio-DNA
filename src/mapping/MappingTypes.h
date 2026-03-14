#pragma once
#include <cstdint>

// Source: which audio feature drives a mapping.
// Each value corresponds to a field (or indexed sub-field) in FeatureSnapshot.
enum class MappingSource : uint8_t
{
    // Amplitude
    RMS = 0,
    Peak,
    RmsDB,

    // Loudness
    LUFS,
    DynamicRange,
    TransientDensity,

    // Spectral
    SpectralCentroid,
    SpectralFlux,
    SpectralFlatness,
    SpectralRolloff,

    // 7-Band energies
    BandSub,          // bandEnergies[0]
    BandBass,         // bandEnergies[1]
    BandLowMid,       // bandEnergies[2]
    BandMid,          // bandEnergies[3]
    BandHighMid,      // bandEnergies[4]
    BandPresence,     // bandEnergies[5]
    BandBrilliance,   // bandEnergies[6]

    // Onset / rhythm
    OnsetStrength,
    BeatPhase,
    BPM,

    // Structural
    StructuralState,

    // Pitch / harmony
    DominantPitch,
    PitchConfidence,
    DetectedKey,
    HarmonicChange,

    // Timbral (MFCCs)
    MFCC0,
    MFCC1,
    MFCC2,
    MFCC3,
    MFCC4,
    MFCC5,
    MFCC6,
    MFCC7,
    MFCC8,
    MFCC9,
    MFCC10,
    MFCC11,
    MFCC12,

    // Chroma
    ChromaC,
    ChromaCs,
    ChromaD,
    ChromaDs,
    ChromaE,
    ChromaF,
    ChromaFs,
    ChromaG,
    ChromaGs,
    ChromaA,
    ChromaAs,
    ChromaB,

    Count  // sentinel — total number of sources
};

// Curve: transform function applied after normalization.
enum class MappingCurve : uint8_t
{
    Linear = 0,
    Exponential,   // x^2.0  — emphasizes peaks
    Logarithmic,   // log(1 + 9x) / log(10)  — compresses peaks, lifts lows
    SCurve,        // smoothstep: x^2 * (3 - 2x)
    Stepped,       // floor(x * N) / N  — quantized

    Count
};

// A single mapping: routes one audio feature to one effect parameter.
struct Mapping
{
    MappingSource source = MappingSource::RMS;

    uint32_t targetEffectId   = 0;   // Index into EffectChain
    uint32_t targetParamIndex = 0;   // Index into Effect's param list

    MappingCurve curve = MappingCurve::Linear;

    float inputMin  = 0.0f;   // Source normalization range
    float inputMax  = 1.0f;
    float outputMin = 0.0f;   // Target output range
    float outputMax = 1.0f;

    float smoothing = 0.15f;  // EMA alpha (higher = less smoothing)

    bool enabled = true;
};
