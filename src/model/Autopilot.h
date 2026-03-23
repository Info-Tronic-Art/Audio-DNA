#pragma once
#include "model/Clip.h"
#include "model/Layer.h"
#include "model/Deck.h"
#include "model/Composition.h"
#include "analysis/FeatureSnapshot.h"
#include <cstdint>
#include <unordered_map>

// Autopilot: manages automatic clip advancement based on beat timing.
// Each layer can have autopilot enabled, with per-clip or per-layer defaults.
// Bar-aware durations align clip changes to downbeats when possible.
// P20: Also supports per-type automation (separate timers for Opaque/Transparent/FX layers).
class Autopilot
{
public:
    Autopilot() = default;

    // Call each frame with the current feature snapshot.
    // Checks all layers in the deck for autopilot triggers.
    // Returns true if any clip was auto-advanced.
    bool processFrame(Deck& deck, const FeatureSnapshot& snapshot);

    // P23: Enable smart random mode (energy-aware clip selection)
    void setSmartRandomEnabled(bool enabled) { smartRandomEnabled_ = enabled; }

    // P20: Set the composition-level per-type config (called from MainComponent)
    void setPerTypeConfig(const Composition::PerTypeAutopilotConfig* config)
    {
        perTypeConfig_ = config;
    }

private:
    // Get the beat duration for a clip (resolving LayerDetermined)
    int getBeatsForClip(const Clip& clip, const Layer& layer) const;

    // Get the action for a clip (resolving LayerDetermined)
    Clip::AutopilotAction getActionForClip(const Clip& clip, const Layer& layer) const;

    // Advance to the next clip based on action
    void advanceClip(Layer& layer, int currentCol, Clip::AutopilotAction action,
                     int numColumns) const;

    // P20: Get beat duration based on layer type (per-type automation)
    int getPerTypeBeats(const Layer& layer) const;

    // P20: Get action based on layer type
    Clip::AutopilotAction getPerTypeAction(const Layer& layer) const;

    // P23: Smart random — select clips based on energy/structural state
    void smartAdvanceClip(Layer& layer, int currentCol,
                          int numColumns, const FeatureSnapshot& snapshot) const;

    // Track beats for each layer
    float lastBeatPhase_ = 0.0f;
    bool lastOnBeat_ = false;

    // P20: Per-type autopilot config (owned by Composition, not us)
    const Composition::PerTypeAutopilotConfig* perTypeConfig_ = nullptr;

    // P23: Smart random mode
    bool smartRandomEnabled_ = false;
    uint8_t lastEnergyState_ = 1;
};
