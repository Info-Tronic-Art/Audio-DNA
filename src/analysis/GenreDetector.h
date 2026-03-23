#pragma once
#include <cstdint>
#include <array>

// Real-time genre classification from audio features.
// Uses a multi-feature scoring approach with temporal smoothing.
//
// 8 genres detected:
//   0 = House        (steady 4-on-floor kick, 120-130 BPM, warm bass)
//   1 = Techno       (driving, 125-145 BPM, high transient density, spectral flux)
//   2 = DnB          (fast breakbeats, 160-180 BPM, heavy bass, syncopation)
//   3 = Hip-Hop      (slower groove, 80-100 BPM, strong bass + mids)
//   4 = Ambient      (sparse, low transient density, spectral flatness, sustained tones)
//   5 = Rock         (full spectrum, high peak levels, guitar frequency presence)
//   6 = Pop/Electronic (varied, mid-range BPM, bright, high spectral centroid)
//   7 = Jazz/Other   (complex harmony, variable BPM, chromatic complexity)
//
// Pre-allocates all state at construction — zero allocation in steady state.
// Process one hop's features via process(), then query detectedGenre()/confidence().
class GenreDetector
{
public:
    static constexpr int kNumGenres = 8;

    // Genre IDs
    static constexpr uint8_t kHouse       = 0;
    static constexpr uint8_t kTechno      = 1;
    static constexpr uint8_t kDnB         = 2;
    static constexpr uint8_t kHipHop      = 3;
    static constexpr uint8_t kAmbient     = 4;
    static constexpr uint8_t kRock        = 5;
    static constexpr uint8_t kPopElectronic = 6;
    static constexpr uint8_t kJazzOther   = 7;

    // Feature input struct (avoids coupling to FeatureSnapshot)
    struct Features
    {
        float bpm              = 0.0f;
        float rms              = 0.0f;
        float spectralCentroid = 0.0f;
        float spectralFlux     = 0.0f;
        float spectralFlatness = 0.0f;
        float spectralRolloff  = 0.0f;
        float transientDensity = 0.0f;
        float bandEnergies[7]  = {};      // Sub, Bass, LowMid, Mid, HighMid, Presence, Brilliance
        float chromagram[12]   = {};
        float mfccs[13]        = {};
        float dynamicRange     = 0.0f;
        uint8_t structuralState = 0;
        uint8_t trackerState   = 0;       // BPM tracker state (0=searching, 2=locked)
        float harmonicChangeDetection = 0.0f;
    };

    // sampleRate: audio sample rate in Hz
    // hopSize: samples per call to process()
    GenreDetector(float sampleRate, int hopSize);

    // Feed one hop's features. After this call, query detectedGenre()/confidence().
    void process(const Features& features);

    // --- Accessors (valid after process()) ---
    uint8_t detectedGenre()   const { return confirmedGenre_; }
    float   genreConfidence() const { return confidence_; }
    uint8_t energyState()     const { return energyState_; }

    // Get the genre name string for a genre ID
    static const char* genreName(uint8_t genre);

    // Get raw scores for all genres (for UI display)
    const float* genreScores() const { return smoothedScores_.data(); }

private:
    // Compute raw genre scores from features
    void computeScores(const Features& features);

    // Apply temporal smoothing to scores
    void smoothScores();

    // Classify from smoothed scores
    void classify();

    // Compute energy state from features
    void computeEnergyState(const Features& features);

    // Compute chromatic complexity (how many active pitch classes)
    float chromaticComplexity(const float chromagram[12]) const;

    // Raw per-genre scores (before smoothing)
    std::array<float, kNumGenres> rawScores_{};

    // Smoothed scores (EMA)
    std::array<float, kNumGenres> smoothedScores_{};

    // EMA alpha for score smoothing (~2 second window)
    float smoothAlpha_ = 0.0f;

    // Confirmed genre (after hysteresis)
    uint8_t confirmedGenre_ = kPopElectronic;  // Default to pop/electronic
    float confidence_ = 0.0f;

    // Energy state: 0=low, 1=medium, 2=high
    uint8_t energyState_ = 1;

    // Hysteresis: require sustained new genre before switching
    uint8_t candidateGenre_ = kPopElectronic;
    int holdCounter_ = 0;
    int holdThreshold_ = 0;  // ~2 seconds of consistency

    // EMA for energy state
    float energyEMA_ = 0.0f;
};
