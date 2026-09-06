#include "recording/RecorderClock.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>

void RecorderClock::tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples)
{
    if (!haveTicked_)
    {
        haveTicked_ = true;
        startWall_ = wallNow;
        lastPhase_ = snap.beatPhase;
        lastBpm_ = snap.bpm;
        wholeBeats_ = 0.0;
        beatOffset_ = 0.0;

        tempo_.append({ 0.0, 0.0, deliveredSamples, snap.bpm, "start" });
        current_ = { 0.0, 0.0, deliveredSamples, snap.bpm };
        return;
    }

    const double t = wallNow - startWall_;
    const bool wasUnmetered = (lastBpm_ <= 0.0f);
    const bool isUnmetered = (snap.bpm <= 0.0f);

    if (isUnmetered)
    {
        if (!wasUnmetered)
            tempo_.append({ t, wholeBeats_ + lastPhase_ + beatOffset_, deliveredSamples, 0.0f, "unmetered" });
        // Frozen: wholeBeats_/lastPhase_/beatOffset_ untouched while unmetered.
    }
    else if (wasUnmetered)
    {
        // Re-locked: absorb the discontinuity exactly like a resync so the
        // frozen value carries forward continuously (D1's "the clock
        // follows the tracker").
        const double target = wholeBeats_ + lastPhase_ + beatOffset_;
        beatOffset_ = target - wholeBeats_ - snap.beatPhase;
        lastPhase_ = snap.beatPhase;
        tempo_.append({ t, target, deliveredSamples, snap.bpm, "lock" });
    }
    else
    {
        const double phase = snap.beatPhase;
        if (phase < lastPhase_ - 0.5)
        {
            wholeBeats_ += 1.0;              // ordinary sawtooth wrap (G14 precedent)
            lastPhase_ = phase;
        }
        else if (phase < lastPhase_)
        {
            // Smaller backward jump: resync/tap/relock. Absorb into the
            // offset so `beat` never dips.
            const double target = wholeBeats_ + lastPhase_ + beatOffset_;
            beatOffset_ = target - wholeBeats_ - phase;
            lastPhase_ = phase;
            tempo_.append({ t, target, deliveredSamples, snap.bpm, "reset" });
        }
        else
        {
            lastPhase_ = phase;
        }

        if (std::fabs(snap.bpm - lastBpm_) > kBpmChangeThreshold)
            tempo_.append({ t, wholeBeats_ + lastPhase_ + beatOffset_, deliveredSamples, snap.bpm, "bpm" });
    }

    lastBpm_ = snap.bpm;
    current_.t = t;
    current_.beat = wholeBeats_ + lastPhase_ + beatOffset_;
    current_.sample = deliveredSamples;
    current_.bpm = snap.bpm;
}
