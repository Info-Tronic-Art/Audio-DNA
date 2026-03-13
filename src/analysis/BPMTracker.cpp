#include "BPMTracker.h"
#include <aubio/aubio.h>
#include <cstring>
#include <cmath>

BPMTracker::BPMTracker(int hopSize, int bufSize, int sampleRate)
    : hopSize_(hopSize),
      sampleRate_(sampleRate)
{
    tempo_ = new_aubio_tempo(
        "default",                              // method (only "default" supported)
        static_cast<uint_t>(bufSize),
        static_cast<uint_t>(hopSize),
        static_cast<uint_t>(sampleRate)
    );

    input_  = new_fvec(static_cast<uint_t>(hopSize));
    output_ = new_fvec(1);

    // Configure for VJ / music-reactive use
    aubio_tempo_set_silence(tempo_, -70.0f);     // silence gate (dB)
    aubio_tempo_set_threshold(tempo_, 0.3f);     // peak-picking threshold
}

BPMTracker::~BPMTracker()
{
    if (output_) del_fvec(output_);
    if (input_)  del_fvec(input_);
    if (tempo_)  del_aubio_tempo(tempo_);
}

void BPMTracker::process(const float* samples)
{
    // Copy samples into Aubio's fvec_t
    std::memcpy(input_->data, samples, static_cast<size_t>(hopSize_) * sizeof(float));

    aubio_tempo_do(tempo_, input_, output_);

    // Check if a beat was detected this hop
    beatDetected_ = (fvec_get_sample(output_, 0) != 0.0f);

    // Read BPM and confidence from Aubio
    bpm_        = aubio_tempo_get_bpm(tempo_);
    confidence_ = aubio_tempo_get_confidence(tempo_);

    // Get beat period in samples for phase computation
    float period = aubio_tempo_get_period(tempo_);
    if (period > 0.0f)
        currentPeriodSamples_ = period;

    // Beat phase: [0, 1) sawtooth ramp between beats
    if (beatDetected_)
    {
        samplesSinceLastBeat_ = 0;
    }

    if (currentPeriodSamples_ > 0.0f)
    {
        beatPhase_ = static_cast<float>(samplesSinceLastBeat_) / currentPeriodSamples_;
        // Clamp to [0, 1) — if tracker drifts, phase can exceed 1
        beatPhase_ = std::fmod(beatPhase_, 1.0f);
    }
    else
    {
        beatPhase_ = 0.0f;
    }

    samplesSinceLastBeat_ += static_cast<uint64_t>(hopSize_);
}

void BPMTracker::setThreshold(float t)
{
    aubio_tempo_set_threshold(tempo_, t);
}

void BPMTracker::setSilence(float dbThreshold)
{
    aubio_tempo_set_silence(tempo_, dbThreshold);
}
