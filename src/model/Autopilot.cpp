#include "Autopilot.h"
#include <cstdlib>

bool Autopilot::processFrame(Deck& deck, const FeatureSnapshot& snapshot)
{
    // Detect beat crossing (beatPhase wraps from ~1.0 to ~0.0)
    bool beatCrossed = (snapshot.beatPhase < lastBeatPhase_ - 0.5f);
    lastBeatPhase_ = snapshot.beatPhase;

    if (!beatCrossed)
        return false;

    bool anyAdvanced = false;

    for (auto& layer : deck.layers)
    {
        if (!layer.autopilotEnabled)
            continue;

        Clip* clip = layer.getActiveClip();
        if (clip == nullptr || !clip->playing)
            continue;

        // Increment beats played on this clip
        clip->beatsPlayed++;

        // Check if it's time to advance
        int targetBeats = getBeatsForClip(*clip, layer);
        if (targetBeats <= 0)
            continue; // No auto-advance (infinite duration)

        if (clip->beatsPlayed >= targetBeats)
        {
            Clip::AutopilotAction action = getActionForClip(*clip, layer);
            if (action != Clip::AutopilotAction::DoNothing)
            {
                advanceClip(layer, layer.activeClipColumn, action, deck.numColumns);
                anyAdvanced = true;
            }
        }
    }

    return anyAdvanced;
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

    switch (dur)
    {
        case Clip::AutopilotDuration::Beat1:  return 1;
        case Clip::AutopilotDuration::Beat2:  return 2;
        case Clip::AutopilotDuration::Beat4:  return 4;
        case Clip::AutopilotDuration::Beat8:  return 8;
        case Clip::AutopilotDuration::Beat16: return 16;
        case Clip::AutopilotDuration::Beat32: return 32;
        case Clip::AutopilotDuration::Custom: return customBeats;
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
