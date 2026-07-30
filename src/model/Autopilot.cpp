#include "Autopilot.h"
#include <cstdlib>

bool Autopilot::processFrame(Deck& deck, const FeatureSnapshot& snapshot)
{
    bool anyAdvanced = false;

    // === End of Video mode: check every frame (not just on beat crossings) ===
    for (auto& layer : deck.layers)
    {
        if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)
            continue;

        Clip* clip = layer.getActiveClip();
        if (clip == nullptr || !clip->isPlayable() || !clip->playing)
            continue;

        // Check if playhead reached the out point (or end)
        double outPos = (clip->outPoint > 0.01f) ? static_cast<double>(clip->outPoint) : 1.0;
        double threshold = outPos - 0.01; // Small margin to avoid floating-point edge cases

        if (clip->playheadPosition >= threshold)
        {
            // Check loops: count how many times we've looped
            clip->beatsPlayed++; // Reuse beatsPlayed as loop counter for end-of-video mode
            int loopsTarget = std::max(1, layer.autopilotLoops);

            if (clip->beatsPlayed >= loopsTarget)
            {
                Clip::AutopilotAction action = getActionForClip(*clip, layer);
                if (action != Clip::AutopilotAction::DoNothing)
                {
                    if (smartRandomEnabled_ && action == Clip::AutopilotAction::PlayRandom)
                        smartAdvanceClip(layer, layer.activeClipColumn, deck.numColumns, snapshot);
                    else
                        advanceClip(layer, layer.activeClipColumn, action, deck.numColumns);
                    anyAdvanced = true;
                }
            }
        }
    }

    // === Beat-based mode: only process on beat crossings ===
    bool beatCrossed = (snapshot.beatPhase < lastBeatPhase_ - 0.5f);
    lastBeatPhase_ = snapshot.beatPhase;

    if (!beatCrossed)
        return anyAdvanced;

    // Process any beat-snapped pending triggers on beat crossing
    // P21: pass beat position info for bar/2-bar/4-bar snap granularity
    for (auto& layer : deck.layers)
    {
        if (layer.pendingTriggerColumn >= 0)
        {
            layer.processPendingTrigger(static_cast<int>(snapshot.beatInBar),
                                         static_cast<int>(snapshot.barCount));
            anyAdvanced = true;
        }
    }

    for (auto& layer : deck.layers)
    {
        if (!layer.autopilotEnabled)
            continue;

        Clip* clip = layer.getActiveClip();
        if (clip == nullptr || !clip->playing)
            continue;

        if (layer.autopilotEndOfVideo && clip->isPlayable())
            continue; // Skip end-of-video layers with a playable clip (handled above);
                      // a non-playable active clip (Source/Image/Camera) falls through
                      // to beat-based advancement instead of freezing.

        // Increment beats played on this clip
        clip->beatsPlayed++;

        // P20: Use per-type timing if enabled, otherwise use per-clip/layer timing
        int targetBeats;
        Clip::AutopilotAction action;

        if (perTypeConfig_ && perTypeConfig_->perTypeEnabled)
        {
            targetBeats = getPerTypeBeats(layer);
            action = getPerTypeAction(layer);
        }
        else
        {
            targetBeats = getBeatsForClip(*clip, layer);
            action = getActionForClip(*clip, layer);
        }

        if (targetBeats <= 0)
            continue;

        if (clip->beatsPlayed >= targetBeats)
        {
            if (action != Clip::AutopilotAction::DoNothing)
            {
                // P23: Use smart random when enabled and action is PlayRandom
                if (smartRandomEnabled_ && action == Clip::AutopilotAction::PlayRandom)
                    smartAdvanceClip(layer, layer.activeClipColumn, deck.numColumns, snapshot);
                else
                    advanceClip(layer, layer.activeClipColumn, action, deck.numColumns);
                anyAdvanced = true;
            }
        }
    }

    return anyAdvanced;
}

int Autopilot::getPerTypeBeats(const Layer& layer) const
{
    if (!perTypeConfig_) return 4;

    switch (layer.type)
    {
        case Layer::Type::Opaque:
            return perTypeConfig_->opaqueCycleBeats;
        case Layer::Type::Transparent:
        case Layer::Type::ThreeD:
        case Layer::Type::Mask:
            return perTypeConfig_->transparentCycleBeats;
        case Layer::Type::FXOnly:
            return perTypeConfig_->effectCycleBeats;
        default:
            return 4;
    }
}

Clip::AutopilotAction Autopilot::getPerTypeAction(const Layer& layer) const
{
    if (!perTypeConfig_) return Clip::AutopilotAction::PlayNext;

    bool shouldRandomize = perTypeConfig_->globalRandomize;

    if (!shouldRandomize)
    {
        switch (layer.type)
        {
            case Layer::Type::Transparent:
            case Layer::Type::ThreeD:
            case Layer::Type::Mask:
                shouldRandomize = perTypeConfig_->transparentRandomize;
                break;
            case Layer::Type::FXOnly:
                shouldRandomize = perTypeConfig_->effectRandomize;
                break;
            default:
                break;
        }
    }

    return shouldRandomize ? Clip::AutopilotAction::PlayRandom : Clip::AutopilotAction::PlayNext;
}

int Autopilot::getBeatsForClip(const Clip& clip, const Layer& layer) const
{
    Clip::AutopilotDuration dur = clip.autopilotDuration;
    int customBeats = clip.autopilotCustomBeats;

    if (dur == Clip::AutopilotDuration::LayerDetermined)
    {
        dur = layer.defaultAutopilotDuration;
        customBeats = layer.defaultAutopilotCustomBeats;
    }

    // Multiply by the layer's loop count
    int loops = std::max(1, layer.autopilotLoops);

    switch (dur)
    {
        case Clip::AutopilotDuration::Beat1:  return 1 * loops;
        case Clip::AutopilotDuration::Beat2:  return 2 * loops;
        case Clip::AutopilotDuration::Beat4:  return 4 * loops;
        case Clip::AutopilotDuration::Beat8:  return 8 * loops;
        case Clip::AutopilotDuration::Beat16: return 16 * loops;
        case Clip::AutopilotDuration::Beat32: return 32 * loops;
        case Clip::AutopilotDuration::Custom: return customBeats * loops;
        default: return 0; // LayerDetermined shouldn't get here
    }
}

Clip::AutopilotAction Autopilot::getActionForClip(const Clip& clip, const Layer& layer) const
{
    Clip::AutopilotAction action = clip.autopilotAction;
    if (action == Clip::AutopilotAction::LayerDetermined)
        action = layer.defaultAutopilotAction;
    return action;
}

void Autopilot::advanceClip(Layer& layer, int currentCol, Clip::AutopilotAction action,
                             int numColumns) const
{
    if (currentCol < 0)
        return;

    int nextCol = -1;

    switch (action)
    {
        case Clip::AutopilotAction::PlayNext:
        {
            // Find next column with a clip
            for (int offset = 1; offset < numColumns; ++offset)
            {
                int col = (currentCol + offset) % numColumns;
                if (layer.getClipAt(col) != nullptr)
                {
                    nextCol = col;
                    break;
                }
            }
            break;
        }
        case Clip::AutopilotAction::PlayPrevious:
        {
            for (int offset = 1; offset < numColumns; ++offset)
            {
                int col = (currentCol - offset + numColumns) % numColumns;
                if (layer.getClipAt(col) != nullptr)
                {
                    nextCol = col;
                    break;
                }
            }
            break;
        }
        case Clip::AutopilotAction::PlayRandom:
        {
            // Collect all columns with clips (excluding current)
            std::vector<int> candidates;
            for (int c = 0; c < numColumns; ++c)
            {
                if (c != currentCol && layer.getClipAt(c) != nullptr)
                    candidates.push_back(c);
            }
            if (!candidates.empty())
                nextCol = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
            break;
        }
        case Clip::AutopilotAction::PlayFirst:
        {
            for (int c = 0; c < numColumns; ++c)
            {
                if (layer.getClipAt(c) != nullptr)
                {
                    nextCol = c;
                    break;
                }
            }
            break;
        }
        case Clip::AutopilotAction::PlayLast:
        {
            for (int c = numColumns - 1; c >= 0; --c)
            {
                if (layer.getClipAt(c) != nullptr)
                {
                    nextCol = c;
                    break;
                }
            }
            break;
        }
        case Clip::AutopilotAction::PlaySpecific:
        {
            // Use the clip's autopilotSpecificCol
            if (auto* clip = layer.getActiveClip())
            {
                if (clip->autopilotSpecificCol >= 0
                    && clip->autopilotSpecificCol < numColumns
                    && layer.getClipAt(clip->autopilotSpecificCol) != nullptr)
                {
                    nextCol = clip->autopilotSpecificCol;
                }
            }
            break;
        }
        default:
            break;
    }

    if (nextCol >= 0 && nextCol != currentCol)
    {
        layer.triggerClip(nextCol);
    }
}

void Autopilot::smartAdvanceClip(Layer& layer, int currentCol,
                                  int numColumns, const FeatureSnapshot& snapshot) const
{
    // Collect all columns with clips (excluding current)
    std::vector<int> candidates;
    for (int c = 0; c < numColumns; ++c)
    {
        if (c != currentCol && layer.getClipAt(c) != nullptr)
            candidates.push_back(c);
    }

    if (candidates.empty())
        return;

    // If only 1-2 candidates, just pick randomly (not enough for smart selection)
    if (candidates.size() <= 2)
    {
        int nextCol = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
        if (nextCol >= 0) layer.triggerClip(nextCol);
        return;
    }

    // Score each candidate based on position-implied energy vs current energy state.
    // Convention: lower column indices = calmer, higher = more intense.
    // Energy state: 0=low, 1=medium, 2=high
    // Structural state: 0=normal, 1=buildup, 2=drop, 3=breakdown
    //
    // Strategy:
    //   - Drop (2) → prefer high-energy clips (last third)
    //   - Buildup (1) → prefer medium-high clips (escalating)
    //   - Breakdown (3) → prefer low-energy clips (first third)
    //   - Normal (0) → prefer clips matching energy state

    float targetIntensity = 0.5f; // default: medium
    switch (snapshot.structuralState)
    {
        case 2: // Drop — intense
            targetIntensity = 0.85f;
            break;
        case 1: // Buildup — escalating
            targetIntensity = 0.65f;
            break;
        case 3: // Breakdown — calm
            targetIntensity = 0.15f;
            break;
        default: // Normal — follow energy
            targetIntensity = static_cast<float>(snapshot.energyState) / 2.0f;
            break;
    }

    // Score each candidate: higher score = better match
    std::vector<float> scores(candidates.size(), 0.0f);
    float maxCol = static_cast<float>(numColumns - 1);

    for (size_t i = 0; i < candidates.size(); ++i)
    {
        float clipIntensity = (maxCol > 0.0f)
            ? static_cast<float>(candidates[i]) / maxCol
            : 0.5f;

        // Score is inverse of distance to target intensity
        float dist = std::fabs(clipIntensity - targetIntensity);
        scores[i] = 1.0f - dist;

        // Add small random jitter to prevent always picking the same clip
        scores[i] += (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * 0.2f;
    }

    // Pick the candidate with the highest score
    size_t bestIdx = 0;
    float bestScore = scores[0];
    for (size_t i = 1; i < scores.size(); ++i)
    {
        if (scores[i] > bestScore)
        {
            bestScore = scores[i];
            bestIdx = i;
        }
    }

    layer.triggerClip(candidates[bestIdx]);
}
