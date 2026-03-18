#pragma once
#include "signal/Signal.h"
#include "mapping/MappingTypes.h"
#include "mapping/MappingEngine.h"

// AudioSignal: wraps a FeatureSnapshot field as a Signal.
// Uses the existing MappingSource enum and extractSource() function.
class AudioSignal : public Signal
{
public:
    AudioSignal(const std::string& name, MappingSource source, Category category)
        : Signal(name, Type::Audio, category)
        , source_(source) {}

    float getValue(const FeatureSnapshot& snapshot) const override
    {
        return MappingEngine::extractSource(source_, snapshot);
    }

    MappingSource getSource() const { return source_; }

private:
    MappingSource source_;
};
