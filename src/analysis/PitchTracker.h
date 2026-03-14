#pragma once
#include <aubio/aubio.h>

// Wraps aubio_pitch_t for real-time pitch detection.
// All Aubio objects are pre-allocated at construction — zero allocation
// in steady state.  Feed one hop of audio per call to process().
//
// Uses the "yinfft" method (YIN with FFT) for a good balance of speed
// and accuracy.  Outputs dominant pitch in Hz and detection confidence.
class PitchTracker
{
public:
    // hopSize:    samples per call to process() (must match analysis hop, e.g. 512)
    // bufSize:    internal analysis window size (typically 2048)
    // sampleRate: audio sample rate in Hz
    PitchTracker(int hopSize, int bufSize, int sampleRate);
    ~PitchTracker();

    // Non-copyable (owns Aubio C objects)
    PitchTracker(const PitchTracker&) = delete;
    PitchTracker& operator=(const PitchTracker&) = delete;

    // Feed one hop of audio samples (hopSize floats).
    // After this call, query dominantPitch() and confidence().
    void process(const float* samples);

    // --- Accessors (valid after process()) ---
    float dominantPitch() const { return dominantPitch_; }
    float confidence()    const { return confidence_; }

private:
    aubio_pitch_t* pitch_  = nullptr;
    fvec_t*        input_  = nullptr;   // hop-sized input buffer
    fvec_t*        output_ = nullptr;   // single-element output (pitch in Hz)

    int hopSize_;

    float dominantPitch_ = 0.0f;
    float confidence_    = 0.0f;
};
