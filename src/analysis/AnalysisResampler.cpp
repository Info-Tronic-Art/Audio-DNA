#include "AnalysisResampler.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    constexpr double kPi = 3.14159265358979323846;
}

float AnalysisResampler::Biquad::tick(float x)
{
    // Direct form II transposed — same construction as LoudnessAnalyzer::processBiquad.
    float y = b0 * x + s1;
    s1 = b1 * x - a1 * y + s2;
    s2 = b2 * x - a2 * y;
    return y;
}

void AnalysisResampler::Biquad::reset()
{
    s1 = 0.0f;
    s2 = 0.0f;
}

void AnalysisResampler::setSourceRate(double hz)
{
    bypass_ = (hz <= 0.0) || (std::fabs(hz - kTargetRate) < 0.5);
    sourceRate_ = (hz <= 0.0) ? 0.0 : hz;
    ratio_ = bypass_ ? 1.0
                      : std::clamp(hz / kTargetRate, 1.0 / 64.0, static_cast<double>(kMaxRatio));

    interp_.reset();
    staged_ = 0;

    lowpassActive_ = ratio_ > 1.0;
    if (lowpassActive_)
        configureLowpass();
}

float AnalysisResampler::inputBandwidthHz() const
{
    if (bypass_)
        return static_cast<float>(kTargetRate / 2.0);
    return static_cast<float>(std::min(sourceRate_ / 2.0, kTargetRate / 2.0));
}

int AnalysisResampler::inputNeededFor(int numOut) const
{
    return static_cast<int>(std::ceil(ratio_ * static_cast<double>(numOut))) + 2;
}

void AnalysisResampler::configureLowpass()
{
    // 4th-order Butterworth low-pass, realised as 2 cascaded RBJ biquads, at
    // fc = 21600 Hz RELATIVE TO THE SOURCE RATE. Only used when ratio_ > 1
    // (device rate above 48 kHz) so the decimation below doesn't fold
    // above-24kHz content back into the analysis band.
    const double hz = sourceRate_;
    const double fc = 21600.0;
    const double w0 = 2.0 * kPi * fc / hz;
    const double cosw0 = std::cos(w0);
    const double sinw0 = std::sin(w0);

    auto configureOne = [&](Biquad& bq, double q)
    {
        const double alpha = sinw0 / (2.0 * q);
        const double b0 = (1.0 - cosw0) / 2.0;
        const double b1 = 1.0 - cosw0;
        const double b2 = b0;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cosw0;
        const double a2 = 1.0 - alpha;

        bq.b0 = static_cast<float>(b0 / a0);
        bq.b1 = static_cast<float>(b1 / a0);
        bq.b2 = static_cast<float>(b2 / a0);
        bq.a1 = static_cast<float>(a1 / a0);
        bq.a2 = static_cast<float>(a2 / a0);
        bq.reset();
    };

    configureOne(lp1_, 0.54120);
    configureOne(lp2_, 1.30656);
}

bool AnalysisResampler::pullHop(RingBuffer<float>& ring, float* out, int numOut)
{
    if (bypass_)
    {
        if (ring.availableToRead() < static_cast<size_t>(numOut))
            return false;
        ring.pop(out, static_cast<size_t>(numOut));
        return true;
    }

    const int needed = inputNeededFor(numOut);
    jassert(needed <= kStagingCapacity);

    while (staged_ < needed)
    {
        const size_t avail = ring.availableToRead();
        if (avail == 0)
            return false;

        const size_t want = static_cast<size_t>(needed - staged_);
        const size_t n = ring.pop(staging_.data() + staged_, std::min(avail, want));

        if (lowpassActive_)
        {
            for (int i = staged_; i < staged_ + static_cast<int>(n); ++i)
                staging_[static_cast<size_t>(i)] = lp2_.tick(lp1_.tick(staging_[static_cast<size_t>(i)]));
        }

        staged_ += static_cast<int>(n);
    }

    const int used = interp_.process(ratio_, staging_.data(), out, numOut);
    const size_t remaining = static_cast<size_t>(staged_ - used);
    std::memmove(staging_.data(), staging_.data() + used, remaining * sizeof(float));
    staged_ -= used;
    return true;
}
