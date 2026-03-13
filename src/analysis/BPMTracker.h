#pragma once
#include <aubio/aubio.h>
#include <cstdint>

// Wraps aubio_tempo_t for real-time BPM tracking and beat phase.
// All Aubio objects are pre-allocated at construction — zero allocation
// in steady state.  Feed one hop of audio per call to process().
//
// Outputs:
//   - bpm:          current tempo estimate (BPM), 0 if unknown
//   - beatPhase:    [0, 1) sawtooth that ramps between beats
//   - beatDetected: true on the hop where a beat lands
//   - confidence:   how confident the tracker is in its tempo estimate
class BPMTracker
{
public:
    // hopSize:    samples per call to process() (must match analysis hop, e.g. 512)
    // bufSize:    internal FFT size for the tempo tracker (typically 1024)
    // sampleRate: audio sample rate in Hz
    BPMTracker(int hopSize, int bufSize, int sampleRate);
    ~BPMTracker();

    // Non-copyable (owns Aubio C objects)
    BPMTracker(const BPMTracker&) = delete;
    BPMTracker& operator=(const BPMTracker&) = delete;

    // Feed one hop of audio samples (hopSize floats).
    // After this call, query bpm(), beatPhase(), beatDetected().
    void process(const float* samples);

    // --- Accessors (valid after process()) ---
    float bpm()          const { return bpm_; }
    float beatPhase()    const { return beatPhase_; }
    bool  beatDetected() const { return beatDetected_; }
    float confidence()   const { return confidence_; }

    // --- Configuration ---
    void setThreshold(float t);
    void setSilence(float dbThreshold);

private:
    aubio_tempo_t* tempo_  = nullptr;
    fvec_t*        input_  = nullptr;   // hop-sized input buffer
    fvec_t*        output_ = nullptr;   // single-element output (beat position in samples)

    int hopSize_;
    int sampleRate_;

    float bpm_          = 0.0f;
    float beatPhase_    = 0.0f;
    bool  beatDetected_ = false;
    float confidence_   = 0.0f;

    // Beat phase tracking: count samples since last beat
    uint64_t samplesSinceLastBeat_ = 0;
    float    currentPeriodSamples_ = 0.0f;  // beat period from aubio, in samples
};
