#pragma once
#include "model/Clip.h"
#include "model/Layer.h"
#include "model/Deck.h"
#include "analysis/FeatureSnapshot.h"
#include <cstdint>

// Autopilot: manages automatic clip advancement based on beat timing.
// Each layer can have autopilot enabled, with per-clip or per-layer defaults.
// Bar-aware durations align clip changes to downbeats when possible.
class Autopilot
{
public:
    Autopilot() = default;

    // Call each frame with the current feature snapshot.
    // Checks all layers in the deck for autopilot triggers.
    // Returns true if any clip was auto-advanced.
    bool processFrame(Deck& deck, const FeatureSnapshot& snapshot);

private:
    // Get the beat duration for a clip (resolving LayerDetermined)
    int getBeatsForClip(const Clip& clip, const Layer& layer) const;

    // Get the action for a clip (resolving LayerDetermined)
    Clip::AutopilotAction getActionForClip(const Clip& clip, const Layer& layer) const;

    // Advance to the next clip based on action
    void advanceClip(Layer& layer, int currentCol, Clip::AutopilotAction action,
                     int numColumns) const;

    // Track beats for each layer
    float lastBeatPhase_ = 0.0f;
    bool lastOnBeat_ = false;
};
