#pragma once
#include "analysis/FeatureSnapshot.h"
#include <string>
#include <cstdint>

// Signal: base class for any value stream used in routing.
// Audio features, oscillators, envelopes, and macros all derive from this.
// S166-L1: evaluated once per tick on the MESSAGE thread only, via
// SignalRegistry::evaluateAll (MainComponent::tickFeaturePipeline, 120Hz) —
// not the render thread. Confinement matters because Signal settings (e.g.
// OscillatorSignal/EnvelopeSignal fields) are mutated by the UI on this same
// thread.
class Signal
{
public:
    enum class Type : uint8_t
    {
        Audio,       // Wraps a FeatureSnapshot field
        Oscillator,  // BPM-locked waveform
        Envelope     // Custom curve
    };

    enum class Category : uint8_t
    {
        Amplitude, Bands, Rhythm, Pitch, Chroma, Timbre, Structure, Modulation
    };

    Signal(const std::string& name, Type type, Category category)
        : name_(name), type_(type), category_(category) {}

    virtual ~Signal() = default;

    // Evaluate and return current value [0, 1] (or natural range for some signals).
    // Called once per render frame. snapshot provides audio features, bpm/beatPhase
    // are used by oscillators for BPM-locked operation.
    virtual float getValue(const FeatureSnapshot& snapshot) const = 0;

    // Metadata
    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    Type getType() const { return type_; }
    Category getCategory() const { return category_; }

    // Unique ID for routing
    uint32_t getId() const { return id_; }
    void setId(uint32_t id) { id_ = id; }

    // Visibility in Signal Bar
    bool isVisible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

protected:
    std::string name_;
    Type type_;
    Category category_;
    uint32_t id_ = 0;
    bool visible_ = true;
};
