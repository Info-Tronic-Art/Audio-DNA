#pragma once
#include <cmath>

// EMA (Exponential Moving Average) smoother with configurable alpha.
// Also supports One-Euro filter mode with adaptive cutoff.
// Used by render/UI threads for display smoothing of audio features.
class Smoother
{
public:
    // alpha: smoothing factor in (0, 1]. Higher = less smoothing.
    //        alpha = 1.0 means no smoothing (passthrough).
    explicit Smoother(float alpha = 0.3f)
        : alpha_(alpha) {}

    // Process one sample and return smoothed value.
    float process(float input)
    {
        if (!initialized_)
        {
            value_ = input;
            initialized_ = true;
            return value_;
        }
        value_ += alpha_ * (input - value_);
        return value_;
    }

    // Get current smoothed value without advancing.
    float value() const { return value_; }

    // Reset state.
    void reset()
    {
        value_ = 0.0f;
        initialized_ = false;
    }

    void setAlpha(float alpha) { alpha_ = alpha; }
    float alpha() const { return alpha_; }

private:
    float alpha_;
    float value_ = 0.0f;
    bool  initialized_ = false;
};

// One-Euro filter: adaptive low-pass that smooths slow movements
// but reacts quickly to fast changes. Good for noisy sensor data.
class OneEuroFilter
{
public:
    // minCutoff: minimum cutoff frequency (Hz) — smoothing at rest
    // beta: speed coefficient — how much speed increases cutoff
    // dCutoff: cutoff for derivative filtering
    OneEuroFilter(float rate = 93.75f, float minCutoff = 1.0f,
                  float beta = 0.007f, float dCutoff = 1.0f)
        : rate_(rate), minCutoff_(minCutoff), beta_(beta), dCutoff_(dCutoff) {}

    float process(float input)
    {
        if (!initialized_)
        {
            value_ = input;
            dValue_ = 0.0f;
            initialized_ = true;
            return value_;
        }

        // Estimate derivative
        float dAlpha = smoothingAlpha(dCutoff_);
        float rawD = (input - value_) * rate_;
        dValue_ += dAlpha * (rawD - dValue_);

        // Adaptive cutoff
        float cutoff = minCutoff_ + beta_ * std::fabs(dValue_);
        float alpha = smoothingAlpha(cutoff);

        value_ += alpha * (input - value_);
        return value_;
    }

    float value() const { return value_; }

    void reset()
    {
        value_ = 0.0f;
        dValue_ = 0.0f;
        initialized_ = false;
    }

private:
    float smoothingAlpha(float cutoff) const
    {
        float tau = 1.0f / (2.0f * 3.14159265f * cutoff);
        float te = 1.0f / rate_;
        return te / (te + tau);
    }

    float rate_;
    float minCutoff_;
    float beta_;
    float dCutoff_;
    float value_ = 0.0f;
    float dValue_ = 0.0f;
    bool  initialized_ = false;
};
