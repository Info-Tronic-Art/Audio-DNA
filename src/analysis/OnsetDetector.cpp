#include "OnsetDetector.h"
#include <aubio/aubio.h>
#include <cstring>

OnsetDetector::OnsetDetector(int hopSize, int bufSize, int sampleRate)
    : hopSize_(hopSize)
{
    onset_ = new_aubio_onset(
        "specflux",                         // half-wave rectified spectral flux
        static_cast<uint_t>(bufSize),
        static_cast<uint_t>(hopSize),
        static_cast<uint_t>(sampleRate)
    );

    input_  = new_fvec(static_cast<uint_t>(hopSize));
    output_ = new_fvec(1);

    // Configure: tuned for VJ / music-reactive use
    aubio_onset_set_threshold(onset_, 0.3f);      // moderate sensitivity
    aubio_onset_set_silence(onset_, -70.0f);       // silence gate (dB)
    aubio_onset_set_minioi_ms(onset_, 50.0f);      // min 50ms between onsets
    aubio_onset_set_awhitening(onset_, 1);         // adaptive whitening on
}

OnsetDetector::~OnsetDetector()
{
    if (output_) del_fvec(output_);
    if (input_)  del_fvec(input_);
    if (onset_)  del_aubio_onset(onset_);
}

void OnsetDetector::process(const float* samples)
{
    // Copy samples into Aubio's fvec_t (zero-copy not possible: different layout)
    std::memcpy(input_->data, samples, static_cast<size_t>(hopSize_) * sizeof(float));

    aubio_onset_do(onset_, input_, output_);

    onsetDetected_ = (fvec_get_sample(output_, 0) != 0.0f);
    onsetStrength_ = aubio_onset_get_descriptor(onset_);
}

void OnsetDetector::setThreshold(float t)
{
    aubio_onset_set_threshold(onset_, t);
}

void OnsetDetector::setSilence(float dbThreshold)
{
    aubio_onset_set_silence(onset_, dbThreshold);
}

void OnsetDetector::setMinInterOnsetMs(float ms)
{
    aubio_onset_set_minioi_ms(onset_, ms);
}
