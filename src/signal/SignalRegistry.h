#pragma once
#include "signal/Signal.h"
#include "signal/AudioSignal.h"
#include "signal/OscillatorSignal.h"
#include "signal/EnvelopeSignal.h"
#include "signal/ClipPositionSignal.h"
#include <vector>
#include <memory>
#include <string>

// SignalRegistry: manages all active signals in the application.
// Populated at startup with default audio signals and modulation slots.
// Users can add/remove modulation signals.
class SignalRegistry
{
public:
    SignalRegistry() = default;

    // Initialize with default signals (12 audio + 6 modulation).
    void initDefaults();

    // Signal management
    void addSignal(std::unique_ptr<Signal> signal);
    bool removeSignal(uint32_t id);
    Signal* getSignal(uint32_t id);
    const Signal* getSignal(uint32_t id) const;
    Signal* getSignalByName(const std::string& name);

    // Iterate all signals
    int getNumSignals() const { return static_cast<int>(signals_.size()); }
    Signal* getSignalAt(int index);
    const Signal* getSignalAt(int index) const;

    // Get signals by category
    std::vector<Signal*> getSignalsByCategory(Signal::Category category);

    // Evaluate all signals for the current frame (updates cached values).
    void evaluateAll(const FeatureSnapshot& snapshot);

    // Get the last evaluated value for a signal (avoid re-evaluation).
    float getCachedValue(uint32_t signalId) const;

    // P24: Get the clip position signal for updating from render thread
    ClipPositionSignal* getClipPositionSignal();


private:
    std::vector<std::unique_ptr<Signal>> signals_;
    std::vector<float> cachedValues_;
    uint32_t nextId_ = 1;
};
