#include "PitchTracker.h"
#include <aubio/aubio.h>
#include <cstring>

PitchTracker::PitchTracker(int hopSize, int bufSize, int sampleRate)
    : hopSize_(hopSize)
{
    pitch_ = new_aubio_pitch(
        "yinfft",                               // YIN with FFT — good speed/accuracy balance
        static_cast<uint_t>(bufSize),
        static_cast<uint_t>(hopSize),
        static_cast<uint_t>(sampleRate)
    );

    input_  = new_fvec(static_cast<uint_t>(hopSize));
    output_ = new_fvec(1);

    // Configure for VJ / music-reactive use
    aubio_pitch_set_unit(pitch_, "Hz");          // output in Hz
    aubio_pitch_set_tolerance(pitch_, 0.7f);     // YIN tolerance (lower = stricter)
    aubio_pitch_set_silence(pitch_, -60.0f);     // silence gate (dB)
}

PitchTracker::~PitchTracker()
{
    if (output_) del_fvec(output_);
    if (input_)  del_fvec(input_);
    if (pitch_)  del_aubio_pitch(pitch_);
}

void PitchTracker::process(const float* samples)
{
    // Copy samples into Aubio's fvec_t
    std::memcpy(input_->data, samples, static_cast<size_t>(hopSize_) * sizeof(float));

    aubio_pitch_do(pitch_, input_, output_);

    dominantPitch_ = fvec_get_sample(output_, 0);
    confidence_    = aubio_pitch_get_confidence(pitch_);
}
