#pragma once
#include "model/Clip.h"
#include "model/Layer.h"
#include "model/Deck.h"
#include "model/Composition.h"
#include "analysis/FeatureSnapshot.h"
#include "features/OnsetPulse.h"
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

    // Lane tsan (s-rta-1002; ruling amendment 13): the tuple transitions this frame APPLIED (before != after) -- a
    // queued trigger fired (processPendingTrigger) and an autopilot advance (triggerClip). The Renderer accumulates
    // them into /api/state render_pending_fired / render_autopilot_advances.
    struct FrameReport
    {
        uint32_t pendingFired = 0;
        uint32_t advances = 0;
    };

    // Call each frame with the current feature snapshot.
    // Checks all layers in the deck for autopilot triggers.
    // Returns true if any clip was auto-advanced. report (optional) receives the transitions applied.
    // GL thread: ONE tuple load per layer per pass; its triggers try at most 16 compare-exchanges (amendment 7).
    bool processFrame(Deck& deck, const FeatureSnapshot& snapshot, FrameReport* report = nullptr);

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

    // Advance to the next clip based on action. currentCol: the active column of the tuple the caller loaded.
    // Returns true when the trigger changed the tuple.
    bool advanceClip(Layer& layer, int currentCol, Clip::AutopilotAction action,
                     int numColumns) const;

    // P20: Get beat duration based on layer type (per-type automation)
    int getPerTypeBeats(const Layer& layer) const;

    // P20: Get action based on layer type
    Clip::AutopilotAction getPerTypeAction(const Layer& layer) const;

    // P23: Smart random — select clips based on energy/structural state
    bool smartAdvanceClip(Layer& layer, int currentCol,
                          int numColumns, const FeatureSnapshot& snapshot) const;

    // Whole beats since this instance's previous frame -- the FeatureSnapshot::totalBeatCount delta (Pitfall 42),
    // one baseline per instance (Pitfall 38)
    OnsetPulse beatCrossings_;

    // P20: Per-type autopilot config (owned by Composition, not us)
    const Composition::PerTypeAutopilotConfig* perTypeConfig_ = nullptr;

    // P23: Smart random mode
    bool smartRandomEnabled_ = false;
    uint8_t lastEnergyState_ = 1;
};
