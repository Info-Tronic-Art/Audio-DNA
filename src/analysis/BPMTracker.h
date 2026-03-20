#pragma once
#include <aubio/aubio.h>
#include <cstdint>
#include <algorithm>
#include <atomic>

// Wraps aubio_tempo_t for real-time BPM tracking and beat phase, with a
// multi-stage stabilization pipeline on top of aubio's raw output:
//
//   1. BPM range gate (60-200 BPM, fold via halving/doubling)
//   2. Confidence gate (hold last good value during low-confidence periods)
//   3. Octave error correction (snap to locked octave if within 2x/0.5x)
//   4. Median filter (window of 48 estimates, ~500ms)
//   5. Hysteresis lock (2-second persistence required to change)
//
// Beat phase is driven by the locked BPM as a free-running sawtooth,
// with hard resets on high-confidence beat detections from aubio.
//
// Downbeat detection (Phase 2):
//   On each beat, a downbeat score is computed from bass energy + spectral flux
//   + harmonic change. A circular buffer of 16 beat scores is analyzed every 4
//   beats to find which position (0-3) has the highest average score. After 8+
//   consistent beats, the downbeat position locks and beatInBar/barPhase are
//   computed.
//
// All buffers are pre-allocated at construction -- zero allocation in steady state.
//
// Tracker state:
//   0 = SEARCHING (no reliable BPM yet)
//   1 = LOCKING   (candidate being confirmed)
//   2 = LOCKED    (solid, stable BPM)
class BPMTracker
{
public:
    static constexpr uint8_t STATE_SEARCHING = 0;
    static constexpr uint8_t STATE_LOCKING   = 1;
    static constexpr uint8_t STATE_LOCKED    = 2;

    // BPM range limits
    static constexpr float kMinBPM = 60.0f;
    static constexpr float kMaxBPM = 200.0f;

    // Median filter window size (~500ms at 93.75 hops/sec)
    static constexpr int kMedianWindowSize = 48;

    // Hysteresis: hops required to accept a new BPM (~2.1 seconds)
    static constexpr int kHysteresisHops = 200;

    // Minimum confidence to accept a BPM estimate
    static constexpr float kConfidenceThreshold = 0.1f;

    // BPM change threshold for hysteresis (BPM units)
    static constexpr float kBPMChangeThreshold = 2.0f;

    // Beat phase reset confidence threshold
    static constexpr float kBeatResetConfidence = 0.5f;

    // Downbeat detection constants
    static constexpr int kBeatScoreBufferSize = 16;  // circular buffer of beat scores
    static constexpr int kBeatsPerBar = 4;            // assuming 4/4 time
    static constexpr int kDownbeatLockThreshold = 8;  // beats of consistency before locking
    static constexpr float kDownbeatWeightBass = 0.5f;
    static constexpr float kDownbeatWeightFlux = 0.3f;
    static constexpr float kDownbeatWeightHCDF = 0.2f;

    // Phrase tracking constants
    static constexpr int kDefaultPhraseBars = 8;      // default: 8-bar phrases
    static constexpr int kMinPhraseBars = 1;
    static constexpr int kMaxPhraseBars = 32;

    // hopSize:    samples per call to process() (must match analysis hop, e.g. 512)
    // bufSize:    internal FFT size for the tempo tracker (typically 1024)
    // sampleRate: audio sample rate in Hz
    BPMTracker(int hopSize, int bufSize, int sampleRate);
    ~BPMTracker();

    // Non-copyable (owns Aubio C objects)
    BPMTracker(const BPMTracker&) = delete;
    BPMTracker& operator=(const BPMTracker&) = delete;

    // Feed one hop of audio samples (hopSize floats).
    // After this call, query bpm(), beatPhase(), beatDetected(), etc.
    void process(const float* samples);

    // Feed spectral features for downbeat scoring.
    // Call this AFTER process() on each hop, passing current spectral features.
    // The downbeat detector uses these on beat detections to score each beat position.
    // structuralState: current structural detector output (0=normal, 1=buildup, 2=drop, 3=breakdown)
    //   used for phrase reset on structural transitions.
    void feedDownbeatFeatures(float bassEnergy, float spectralFlux, float harmonicChange,
                              uint8_t structuralState = 0);

    // --- Accessors (valid after process()) ---
    float    bpm()           const { return lockedBPM_; }
    float    beatPhase()     const { return phase_; }
    bool     beatDetected()  const { return beatDetected_; }
    float    confidence()    const { return confidence_; }
    uint8_t  trackerState()  const { return trackerState_; }

    // Raw (unstabilized) BPM from aubio, for diagnostics
    float    rawBPM()        const { return rawBPM_; }

    // --- Downbeat accessors (valid after feedDownbeatFeatures()) ---
    uint8_t  beatInBar()        const { return beatInBar_; }
    float    barPhase()         const { return barPhase_; }
    bool     downbeatDetected() const { return downbeatDetected_; }
    bool     downbeatLocked()   const { return downbeatLocked_; }

    // --- Phrase tracking accessors ---
    uint16_t barCount()         const { return barCount_; }
    float    phrasePhase()      const { return phrasePhase_; }
    int      phraseBars()       const { return phraseBars_; }

    // --- Configuration ---
    void setThreshold(float t);
    void setSilence(float dbThreshold);
    void setPhraseBars(int bars);

    // Reset phrase/bar counters (called on Resync)
    void resetPhrase();

    // Override BPM from external tap tempo (bypasses stabilization pipeline).
    // Sets the locked BPM immediately and resets beat phase.
    void setManualBPM(float bpm);

    // Manual mode: freeze the stabilization pipeline, use manually-set BPM.
    // Beat phase still runs from the locked BPM value.
    void setManualMode(bool enabled);
    bool isManualMode() const { return manualMode_; }

    // Reset beat phase to 0 (called on Resync)
    void resetBeatPhase();

    // --- Testing support ---
    // Process a raw BPM value through the stabilization pipeline without aubio.
    // Used by unit tests to verify the pipeline in isolation.
    void processRawBPM(float rawBpm, float conf, bool beat);

private:
    aubio_tempo_t* tempo_  = nullptr;
    fvec_t*        input_  = nullptr;   // hop-sized input buffer
    fvec_t*        output_ = nullptr;   // single-element output (beat position in samples)

    int   hopSize_;
    int   sampleRate_;

    // Raw aubio outputs
    float rawBPM_       = 0.0f;
    float confidence_   = 0.0f;
    bool  beatDetected_ = false;

    // === Stabilization pipeline state ===

    // Median filter: circular buffer of recent BPM estimates
    float medianBuffer_[kMedianWindowSize] = {};
    float sortBuffer_[kMedianWindowSize] = {};   // scratch for nth_element
    int   medianPos_   = 0;
    int   medianCount_ = 0;

    // Locked BPM and hysteresis
    float   lockedBPM_          = 0.0f;
    float   candidateBPM_       = 0.0f;
    int     consistencyCounter_ = 0;
    float   lastConfidentBPM_   = 0.0f;

    // Tracker state
    uint8_t trackerState_ = STATE_SEARCHING;

    // Manual mode flag (set from UI thread, read from analysis thread)
    std::atomic<bool> manualMode_{false};

    // === Beat phase (free-running from locked BPM) ===
    float phase_ = 0.0f;

    // === Downbeat detection state ===
    // Circular buffer of per-beat downbeat scores
    float beatScores_[kBeatScoreBufferSize] = {};
    int   beatScorePos_ = 0;        // write position in circular buffer
    int   totalBeatsScored_ = 0;    // total beats scored since start

    // Per-position (0-3) score accumulators for finding the strongest beat
    float positionScoreSums_[kBeatsPerBar] = {};
    int   positionScoreCounts_[kBeatsPerBar] = {};

    // Downbeat lock state
    bool    downbeatLocked_ = false;
    int     lockedDownbeatPos_ = 0;    // which position (0-3) is the downbeat
    int     downbeatConsistency_ = 0;  // consecutive analyses agreeing on position
    int     beatCounter_ = 0;          // counts beats mod 4 from locked downbeat

    // Per-hop output
    uint8_t beatInBar_ = 0;           // 0-3 (0 = downbeat)
    float   barPhase_ = 0.0f;         // [0, 1) over 4 beats
    bool    downbeatDetected_ = false; // true on the hop where beat 1 lands

    // Cached spectral features for scoring (set by feedDownbeatFeatures)
    float cachedBassEnergy_ = 0.0f;
    float cachedSpectralFlux_ = 0.0f;
    float cachedHarmonicChange_ = 0.0f;

    // === Phrase tracking state ===
    uint16_t barCount_ = 0;            // bars since last phrase reset
    float    phrasePhase_ = 0.0f;      // [0, 1) sawtooth over N bars
    int      phraseBars_ = kDefaultPhraseBars; // configurable phrase length
    bool     prevDownbeatDetected_ = false;    // edge detection for bar counting
    uint8_t  prevStructuralState_ = 0;         // for detecting structural transitions

    // --- Internal pipeline methods ---

    // Fold a BPM value into the [kMinBPM, kMaxBPM] range via halving/doubling
    static float foldBPMToRange(float bpm);

    // Correct octave errors relative to the locked BPM
    float correctOctaveError(float bpm) const;

    // Push a value into the median filter and return the current median
    float pushAndMedian(float bpm);

    // Run the stabilization pipeline on a raw BPM + confidence
    void runPipeline(float rawBpm, float conf, bool beat);

    // Update the free-running beat phase
    void updatePhase(bool beat, float conf);

    // Score a beat and update downbeat detection
    void scoreBeat();

    // Analyze beat scores to find the downbeat position
    void analyzeDownbeatPosition();

    // Update barPhase based on current beat position and phase
    void updateBarPhase();

    // Update phrase tracking (bar count and phrase phase)
    void updatePhrase(uint8_t structuralState);
};
