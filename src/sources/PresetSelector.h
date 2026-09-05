#pragma once
#include "analysis/FeatureSnapshot.h"
#include <string>
#include <functional>

class ProjectMPresetManager;

// PresetSelector: audio-driven automatic preset switching.
// Reads FeatureSnapshot each frame and decides when to switch presets
// based on structural transitions, energy, and BPM.
class PresetSelector
{
public:
    PresetSelector() = default;

    // Set the preset manager to control
    void setPresetManager(ProjectMPresetManager* mgr) { manager_ = mgr; }

    // Process a frame — may trigger a preset change
    // Returns true if a preset was switched
    bool processFrame(const FeatureSnapshot& snapshot);

    // Enable/disable auto-switching
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

    // Energy matching: switch to presets with similar energy
    void setEnergyMatching(bool enabled) { energyMatching_ = enabled; }
    bool isEnergyMatching() const { return energyMatching_; }

    // Mood filter: only auto-switch within this mood (empty = any)
    void setMoodFilter(const std::string& mood) { moodFilter_ = mood; }
    const std::string& getMoodFilter() const { return moodFilter_; }

    // Transition duration in bars (default 4)
    void setTransitionBars(int bars) { transitionBars_ = bars; }
    int getTransitionBars() const { return transitionBars_; }

    // Crossfade duration in seconds (0.5-5.0), consumed via projectM's
    // projectm_set_soft_cut_duration (default 2.0, matches the Jukebox
    // panel's blend slider default).
    void setBlendSeconds(float seconds) { blendSeconds_ = seconds; }
    float getBlendSeconds() const { return blendSeconds_; }

    // Jukebox Pool: restricts auto-switch candidates to a subset.
    enum class PoolFilter { All, Curated, Favorites };
    void setPoolFilter(PoolFilter filter) { poolFilter_ = filter; }
    PoolFilter getPoolFilter() const { return poolFilter_; }

    // Jukebox Mode: how the next auto-switch candidate is chosen.
    // Bag is not true no-repeat-until-exhausted (matches the existing
    // Playlist cycle-mode precedent's own simplification, see
    // Renderer.cpp's RandomBag handling) — it behaves like Random.
    enum class CycleMode { Bag, Random, Sequential };
    void setCycleMode(CycleMode mode) { cycleMode_ = mode; }
    CycleMode getCycleMode() const { return cycleMode_; }

    // Callback when auto-switching occurs
    std::function<void(const std::string& presetPath)> onAutoSwitch;

private:
    ProjectMPresetManager* manager_ = nullptr;
    bool enabled_ = false;
    bool energyMatching_ = true;
    std::string moodFilter_;
    int transitionBars_ = 4;
    float blendSeconds_ = 2.0f;
    PoolFilter poolFilter_ = PoolFilter::All;
    CycleMode cycleMode_ = CycleMode::Bag;

    // State tracking
    uint8_t lastStructuralState_ = 0;
    float lastBarPhase_ = 0.0f;
    bool pendingSwitch_ = false;
    int barsSinceLastSwitch_ = 0;
    static constexpr int kMinBarsBetweenSwitches = 4;
};
