#include "sources/PresetSelector.h"
#include "sources/ProjectMPresetManager.h"

bool PresetSelector::processFrame(const FeatureSnapshot& snapshot)
{
    if (!enabled_ || !manager_ || manager_->getPresetCount() == 0)
        return false;

    // Detect bar boundary crossing (barPhase wraps from ~1 to ~0)
    bool barCrossing = (snapshot.barPhase < lastBarPhase_ && lastBarPhase_ > 0.5f);
    lastBarPhase_ = snapshot.barPhase;

    if (barCrossing)
        barsSinceLastSwitch_++;

    // Detect structural transitions
    bool structuralChange = (snapshot.structuralState != lastStructuralState_);
    lastStructuralState_ = snapshot.structuralState;

    // Schedule a switch on structural transitions
    if (structuralChange && barsSinceLastSwitch_ >= kMinBarsBetweenSwitches)
    {
        // DROP → high energy preset, BREAKDOWN → calm preset
        pendingSwitch_ = true;
    }

    // Execute pending switch on next bar boundary for clean transition
    if (pendingSwitch_ && barCrossing)
    {
        pendingSwitch_ = false;
        barsSinceLastSwitch_ = 0;

        const ProjectMPresetManager::PresetInfo* newPreset = nullptr;

        if (!moodFilter_.empty())
        {
            newPreset = manager_->randomPresetInMood(moodFilter_);
        }
        else if (energyMatching_)
        {
            // Pick mood based on structural state
            std::string targetMood;
            switch (snapshot.structuralState)
            {
                case 2: targetMood = "Energetic"; break;  // DROP
                case 1: targetMood = "Psychedelic"; break; // BUILDUP
                case 3: targetMood = "Calm"; break;        // BREAKDOWN
                default: targetMood = ""; break;           // NORMAL - any
            }

            if (!targetMood.empty())
                newPreset = manager_->randomPresetInMood(targetMood);
            else
                newPreset = manager_->randomPreset();
        }
        else
        {
            newPreset = manager_->randomPreset();
        }

        if (newPreset && onAutoSwitch)
            onAutoSwitch(newPreset->path);

        return true;
    }

    // Auto-switch after transitionBars if no structural change
    if (barsSinceLastSwitch_ >= transitionBars_ * 4 && barCrossing)
    {
        barsSinceLastSwitch_ = 0;
        auto* preset = manager_->randomPreset();
        if (preset && onAutoSwitch)
            onAutoSwitch(preset->path);
        return true;
    }

    return false;
}
