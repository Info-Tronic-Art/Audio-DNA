#pragma once
#include <aubio/aubio.h>

// Wraps aubio_onset_t for real-time onset detection.
// All Aubio objects are pre-allocated at construction — zero allocation
// in steady state.  Feed one hop of audio per call to process().
//
// Uses the "specflux" method (half-wave rectified spectral flux) with
// adaptive threshold.  Configurable silence gate and minimum inter-onset
// interval.
class OnsetDetector
{
public:
    // hopSize: samples per call to process() (must match analysis hop, e.g. 512)
    // bufSize: internal FFT size for the onset detection function (typically 1024)
    // sampleRate: audio sample rate in Hz
    OnsetDetector(int hopSize, int bufSize, int sampleRate);
    ~OnsetDetector();

    // Non-copyable (owns Aubio C objects)
    OnsetDetector(const OnsetDetector&) = delete;
    OnsetDetector& operator=(const OnsetDetector&) = delete;

    // Feed one hop of audio samples (hopSize floats).
    // After this call, query onsetDetected() and onsetStrength().
    void process(const float* samples);

    // --- Accessors (valid after process()) ---
    bool  onsetDetected() const { return onsetDetected_; }
    float onsetStrength() const { return onsetStrength_; }

    // --- Configuration ---
    void setThreshold(float t);
    void setSilence(float dbThreshold);
    void setMinInterOnsetMs(float ms);

private:
    aubio_onset_t* onset_ = nullptr;
    fvec_t*        input_ = nullptr;   // hop-sized input buffer
    fvec_t*        output_ = nullptr;  // single-element output

    int hopSize_;

    bool  onsetDetected_ = false;
    float onsetStrength_ = 0.0f;
};
