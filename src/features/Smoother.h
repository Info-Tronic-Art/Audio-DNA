#pragma once
#include <cmath>

// EMA (Exponential Moving Average) smoother with configurable alpha.
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
