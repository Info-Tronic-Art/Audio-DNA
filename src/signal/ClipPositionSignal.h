#pragma once
#include "signal/Signal.h"
#include "model/Clip.h"

// ClipPositionSignal: exposes the active clip's playhead position as a signal source.
// The value tracks clip->playheadPosition [0,1], normalized to in/out range.
// Updated each frame by the render thread via setCurrentPosition().
class ClipPositionSignal : public Signal
{
public:
    ClipPositionSignal(const std::string& name = "Clip Position")
        : Signal(name, Type::Audio, Category::Modulation) {}

    float getValue(const FeatureSnapshot& /*snapshot*/) const override
    {
        return currentPosition_.load(std::memory_order_relaxed);
    }

    // Called by the render thread each frame with the active clip's playhead
    void setCurrentPosition(float pos)
    {
        currentPosition_.store(std::clamp(pos, 0.0f, 1.0f), std::memory_order_relaxed);
    }

    // Convenience: update from a Clip pointer (handles null, in/out normalization)
    void updateFromClip(const Clip* clip)
    {
        if (!clip || !clip->hasMedia())
        {
            currentPosition_.store(0.0f, std::memory_order_relaxed);
            return;
        }

        // Normalize playhead to in/out range
        float pos = static_cast<float>(clip->playheadPosition);
        float range = clip->outPoint - clip->inPoint;
        float normalized = (range > 1e-6f)
            ? (pos - clip->inPoint) / range
            : 0.0f;
        currentPosition_.store(std::clamp(normalized, 0.0f, 1.0f), std::memory_order_relaxed);
    }

private:
    std::atomic<float> currentPosition_{0.0f};
};
