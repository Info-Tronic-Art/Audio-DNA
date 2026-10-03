#include "Autopilot.h"
#include <cstdlib>

namespace
{
// Autopilot runs on the GL thread, which never waits: its triggers try at most this many compare-exchanges of the
// layer's tuple word (lane tsan, ruling amendment 7), then retry at the next beat crossing. Every advance is DECIDED
// from the tuple processFrame loaded once (its active ref), so it passes that ref as triggerClip's onlyIfActive: a
// clear or an immediate user trigger of another column that landed since the load stands, the advance is a no-op
// (s-rta-1002 fix round, F2). A user trigger queued by beat snap after the load, or a retrigger of the same column,
// can still be cancelled by the advance, as before this guard.
constexpr int kRenderTriggerAttempts = 16;
}

const Deck* Autopilot::sourceDeck(const Composition& show, ClipRef current)
{
    const int idx = show.findDeckIndexById(current.deckId);   // live decks only: a retired source never advances
    return idx >= 0 ? &show.decks[static_cast<size_t>(idx)] : nullptr;
}

bool Autopilot::processFrame(Composition& show, const FeatureSnapshot& snapshot, FrameReport* report)
{
    bool anyAdvanced = false;
    FrameReport counts;
    auto finish = [&] {
        if (report != nullptr)
            *report = counts;
        return anyAdvanced;
    };

    // === End of Video mode: check every frame (not just on beat crossings) ===
    for (int i = 0; i < show.getNumLayers(); ++i)
    {
        Layer& layer = show.layers[static_cast<size_t>(i)];
        if (!layer.autopilotEnabled || !layer.autopilotEndOfVideo)
            continue;

        const auto playing = show.playing(i);   // lane tsan: ONE tuple load for this layer
        Clip* clip = playing.clip;
        if (clip == nullptr || !clip->isPlayable() || !clip->playing)
            continue;

        // Check if playhead reached the out point (or end)
        double outPos = (clip->outPoint > 0.01f) ? static_cast<double>(clip->outPoint) : 1.0;
        double threshold = outPos - 0.01; // Small margin to avoid floating-point edge cases

        if (clip->playheadPosition >= threshold)
        {
            // Check loops: count how many times we've looped. Reuse beatsPlayed as loop counter for end-of-video
            // mode. Lane tsan: ONE atomic add (a trigger's concurrent reset to 0 is never lost), tested on its result.
            const int loopsPlayed = clip->beatsPlayed.fetchAdd(1) + 1;
            int loopsTarget = std::max(1, layer.autopilotLoops);

            if (loopsPlayed >= loopsTarget)
            {
                Clip::AutopilotAction action = getActionForClip(*clip, layer);
                const Deck* source = sourceDeck(show, playing.ref);
                if (action != Clip::AutopilotAction::DoNothing && source != nullptr)
                {
                    const RowClips rows = show.rowClips(i);
                    const bool changed = (smartRandomEnabled_ && action == Clip::AutopilotAction::PlayRandom)
                        ? smartAdvanceClip(layer, rows, *source, playing.ref, snapshot)
                        : advanceClip(layer, rows, *source, playing.ref, action);
                    counts.advances += changed ? 1u : 0u;
                    anyAdvanced = true;
                }
            }
        }
    }

    // === Beat-based mode: only process on beat crossings ===
    // The totalBeatCount delta, never the beatPhase wrap: 0 or 1 at frame rate, > 1 only across a stall, whose
    // whole beats a wrap reader lost (Pitfall 42).
    const uint32_t beats = beatCrossings_.consume(snapshot.totalBeatCount);
    if (beats == 0)
        return finish();

    // Process any beat-snapped pending triggers on beat crossing (whatever deck they were fired from: bf9b F11)
    // P21: pass beat position info for bar/2-bar/4-bar snap granularity
    for (int i = 0; i < show.getNumLayers(); ++i)
    {
        Layer& layer = show.layers[static_cast<size_t>(i)];
        if (layer.runtime().pendingTriggerColumn >= 0)   // lane tsan: ONE tuple load for this layer
        {
            const auto t = layer.processPendingTrigger(static_cast<int>(snapshot.beatInBar),
                                                       static_cast<int>(snapshot.barCount), show.rowClips(i));
            counts.pendingFired += t.changed() ? 1u : 0u;
            anyAdvanced = true;
        }
    }

    for (int i = 0; i < show.getNumLayers(); ++i)
    {
        Layer& layer = show.layers[static_cast<size_t>(i)];
        if (!layer.autopilotEnabled)
            continue;

        const auto playing = show.playing(i);   // lane tsan: ONE tuple load for this layer
        Clip* clip = playing.clip;
        if (clip == nullptr || !clip->playing)
            continue;

        if (layer.autopilotEndOfVideo && clip->isPlayable())
            continue; // Skip end-of-video layers with a playable clip (handled above);
                      // a non-playable active clip (Source/Image/Camera) falls through
                      // to beat-based advancement instead of freezing.

        // Add the beats played on this clip since the previous frame (lane tsan: ONE atomic add, tested on its result)
        const int beatsPlayed = clip->beatsPlayed.fetchAdd(static_cast<int>(beats)) + static_cast<int>(beats);

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

        if (beatsPlayed >= targetBeats)
        {
            const Deck* source = sourceDeck(show, playing.ref);
            if (action != Clip::AutopilotAction::DoNothing && source != nullptr)
            {
                const RowClips rows = show.rowClips(i);
                // P23: Use smart random when enabled and action is PlayRandom
                const bool changed = (smartRandomEnabled_ && action == Clip::AutopilotAction::PlayRandom)
                    ? smartAdvanceClip(layer, rows, *source, playing.ref, snapshot)
                    : advanceClip(layer, rows, *source, playing.ref, action);
                counts.advances += changed ? 1u : 0u;
                anyAdvanced = true;
            }
        }
    }

    return finish();
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

bool Autopilot::advanceClip(Layer& layer, const RowClips& rows, const Deck& source, ClipRef current,
                             Clip::AutopilotAction action) const
{
    const int currentCol = current.column;
    if (currentCol < 0)
        return false;
    const int numColumns = source.numColumns;
    const ClipRow* row = source.getRow(rows.row);
    if (row == nullptr)
        return false;

    int nextCol = -1;

    switch (action)
    {
        case Clip::AutopilotAction::PlayNext:
        {
            // Find next column with a clip
            for (int offset = 1; offset < numColumns; ++offset)
            {
                int col = (currentCol + offset) % numColumns;
                if (row->getClipAt(col) != nullptr)
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
                if (row->getClipAt(col) != nullptr)
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
                if (c != currentCol && row->getClipAt(c) != nullptr)
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
                if (row->getClipAt(c) != nullptr)
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
                if (row->getClipAt(c) != nullptr)
                {
                    nextCol = c;
                    break;
                }
            }
            break;
        }
        case Clip::AutopilotAction::PlaySpecific:
        {
            // Use the clip's autopilotSpecificCol (the active clip of the caller's tuple: no second load)
            if (auto* clip = row->getClipAt(currentCol))
            {
                if (clip->autopilotSpecificCol >= 0
                    && clip->autopilotSpecificCol < numColumns
                    && row->getClipAt(clip->autopilotSpecificCol) != nullptr)
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
        return layer.triggerClip(ClipRef{ source.id, nextCol }, rows, Clip::BeatSnapMode::Off, kRenderTriggerAttempts,
                                 current).changed();
    return false;
}

bool Autopilot::smartAdvanceClip(Layer& layer, const RowClips& rows, const Deck& source, ClipRef current,
                                  const FeatureSnapshot& snapshot) const
{
    const int currentCol = current.column;
    const int numColumns = source.numColumns;
    const ClipRow* row = source.getRow(rows.row);
    if (row == nullptr)
        return false;
    // Collect all columns with clips (excluding current)
    std::vector<int> candidates;
    for (int c = 0; c < numColumns; ++c)
    {
        if (c != currentCol && row->getClipAt(c) != nullptr)
            candidates.push_back(c);
    }

    if (candidates.empty())
        return false;

    // If only 1-2 candidates, just pick randomly (not enough for smart selection)
    if (candidates.size() <= 2)
    {
        int nextCol = candidates[static_cast<size_t>(std::rand()) % candidates.size()];
        return nextCol >= 0
            && layer.triggerClip(ClipRef{ source.id, nextCol }, rows, Clip::BeatSnapMode::Off, kRenderTriggerAttempts,
                                 current).changed();
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

    return layer.triggerClip(ClipRef{ source.id, candidates[bestIdx] }, rows, Clip::BeatSnapMode::Off,
                             kRenderTriggerAttempts, current).changed();
}
