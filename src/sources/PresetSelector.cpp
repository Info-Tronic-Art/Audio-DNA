#include "sources/PresetSelector.h"
#include "sources/ProjectMPresetManager.h"
#include <random>

namespace
{
    // Jukebox Pool: builds the candidate list for poolFilter_, optionally
    // narrowed further by a mood tag. "Curated" duplicates
    // MilkDropBrowser::getCuratedPresets()'s one-line predicate rather than
    // sharing it — MilkDropBrowser is UI-local and not visible from here,
    // and promoting the predicate to ProjectMPresetManager was flagged as an
    // optional follow-up, out of scope for this lane (see L7-JUKE packet).
    std::vector<const ProjectMPresetManager::PresetInfo*> poolCandidates(
        ProjectMPresetManager* manager, PresetSelector::PoolFilter filter,
        const std::string& mood)
    {
        std::vector<const ProjectMPresetManager::PresetInfo*> pool;
        switch (filter)
        {
            case PresetSelector::PoolFilter::Favorites:
                pool = manager->getFavorites();
                break;
            case PresetSelector::PoolFilter::Curated:
                for (const auto& p : manager->getAllPresets())
                    if (p.energy > 0.1f)
                        pool.push_back(&p);
                break;
            case PresetSelector::PoolFilter::All:
            default:
                for (const auto& p : manager->getAllPresets())
                    pool.push_back(&p);
                break;
        }

        if (mood.empty())
            return pool;

        std::vector<const ProjectMPresetManager::PresetInfo*> withMood;
        for (auto* p : pool)
            if (p->mood == mood)
                withMood.push_back(p);
        return withMood;
    }

    // Picks a random candidate and syncs manager's currentIndex_ to match,
    // mirroring ProjectMPresetManager::randomPresetInMood's own bookkeeping
    // (see ProjectMPresetManager.cpp) so getCurrentIndex()/getCurrentPreset()
    // stay consistent for a filtered (Pool-restricted) pick.
    const ProjectMPresetManager::PresetInfo* pickRandomFrom(
        ProjectMPresetManager* manager,
        const std::vector<const ProjectMPresetManager::PresetInfo*>& candidates)
    {
        if (candidates.empty())
            return nullptr;

        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
        auto* selected = candidates[static_cast<size_t>(dist(rng))];

        const auto& all = manager->getAllPresets();
        for (int i = 0; i < static_cast<int>(all.size()); ++i)
        {
            if (all[static_cast<size_t>(i)].path == selected->path)
            {
                manager->setCurrentIndex(i);
                break;
            }
        }
        return selected;
    }
}

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

        if (cycleMode_ == CycleMode::Sequential)
        {
            // Jukebox Mode: Sequential walks the manager's full preset order
            // via nextPreset() — this does not additionally honor poolFilter_
            // (nextPreset() has no filtering concept), matching the L7-JUKE
            // packet's literal instruction; flagged as an open combination
            // gap in the builder report, not silently resolved here.
            newPreset = manager_->nextPreset();
        }
        else if (poolFilter_ == PoolFilter::All)
        {
            // Unfiltered: keep the exact prior behavior byte-for-byte.
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
        }
        else
        {
            // Jukebox Pool: Curated/Favorites — pick within the restricted
            // pool, still honoring mood filter/energy matching on top.
            std::string targetMood = moodFilter_;
            if (targetMood.empty() && energyMatching_)
            {
                switch (snapshot.structuralState)
                {
                    case 2: targetMood = "Energetic"; break;
                    case 1: targetMood = "Psychedelic"; break;
                    case 3: targetMood = "Calm"; break;
                    default: targetMood = ""; break;
                }
            }
            newPreset = pickRandomFrom(manager_, poolCandidates(manager_, poolFilter_, targetMood));
            if (!newPreset && !targetMood.empty())
                newPreset = pickRandomFrom(manager_, poolCandidates(manager_, poolFilter_, ""));
        }

        if (newPreset && onAutoSwitch)
            onAutoSwitch(newPreset->path);

        return true;
    }

    // Auto-switch after transitionBars if no structural change
    if (barsSinceLastSwitch_ >= transitionBars_ * 4 && barCrossing)
    {
        barsSinceLastSwitch_ = 0;

        const ProjectMPresetManager::PresetInfo* preset = nullptr;
        if (cycleMode_ == CycleMode::Sequential)
            preset = manager_->nextPreset();
        else if (poolFilter_ == PoolFilter::All)
            preset = manager_->randomPreset();
        else
            preset = pickRandomFrom(manager_, poolCandidates(manager_, poolFilter_, ""));

        if (preset && onAutoSwitch)
            onAutoSwitch(preset->path);
        return true;
    }

    return false;
}
