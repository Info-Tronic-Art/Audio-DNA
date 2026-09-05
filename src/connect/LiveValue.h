#pragma once
#include <atomic>
#include <cmath>

// LiveValue: the per-parameter twin the engine publishes into, sitting next
// to the existing manual field. NAN ("not driven this tick") makes
// effective() fall back to the manual value, so a never-connected -- or
// currently-gripped -- parameter behaves exactly as it does today.
//
// Copyable (not move-only), because EffectSlot / Clip / Layer are ordinary
// value types: snapshotted whole by EffectStackCmd's before/after vectors,
// copied by Clip::replaceContent, held by value inside Layer::clips and
// Deck::layers. This copy ctor/assignment is the HARD HAZARD s167-l2 was
// warned about -- std::atomic<float> has a deleted copy constructor, so
// none of those value-copy call sites compile once EffectSlot/Clip/Layer
// gain a LiveValue member without this.
struct LiveValue
{
    std::atomic<float> v{NAN};

    LiveValue() = default;
    LiveValue(const LiveValue& o) : v(o.v.load(std::memory_order_relaxed)) {}
    LiveValue& operator=(const LiveValue& o)
    {
        v.store(o.v.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }
    LiveValue(LiveValue&& o) noexcept : v(o.v.load(std::memory_order_relaxed)) {}
    LiveValue& operator=(LiveValue&& o) noexcept
    {
        v.store(o.v.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    float effective(float manual) const
    {
        float x = v.load(std::memory_order_relaxed);
        return std::isnan(x) ? manual : x;
    }
};
