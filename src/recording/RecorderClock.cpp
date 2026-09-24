#include "recording/RecorderClock.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>

void RecorderClock::anchor(double t, double beat, uint64_t sample, float bpm, const char* why)
{
    tempo_.append({ t, beat, sample, bpm, why });
    lastAnchorBeat_ = beat;
}

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

        anchor(0.0, 0.0, deliveredSamples, snap.bpm, "start");
        current_ = { 0.0, 0.0, deliveredSamples, snap.bpm };
        return;
    }

    const double t = wallNow - startWall_;
    const bool wasUnmetered = (lastBpm_ <= 0.0f);
    const bool isUnmetered = (snap.bpm <= 0.0f);

    if (isUnmetered)
    {
        if (!wasUnmetered)
            anchor(t, wholeBeats_ + lastPhase_ + beatOffset_, deliveredSamples, 0.0f, "unmetered");
        // Frozen: wholeBeats_/lastPhase_/beatOffset_ untouched while unmetered.
        // No periodic anchors while unmetered (D1's documented limitation --
        // several equal-beat anchors would move TempoMap::tAt's canonical
        // answer for that beat).
    }
    else if (wasUnmetered)
    {
        // Re-locked: absorb the discontinuity exactly like a resync so the
        // frozen value carries forward continuously (D1's "the clock
        // follows the tracker").
        const double target = wholeBeats_ + lastPhase_ + beatOffset_;
        beatOffset_ = target - wholeBeats_ - snap.beatPhase;
        lastPhase_ = snap.beatPhase;
        anchor(t, target, deliveredSamples, snap.bpm, "lock");
    }
    else
    {
        bool anchorWrittenThisTick = false;
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
            anchor(t, target, deliveredSamples, snap.bpm, "reset");
            anchorWrittenThisTick = true;
        }
        else
        {
            lastPhase_ = phase;
        }

        const double beatNow = wholeBeats_ + lastPhase_ + beatOffset_;
        if (std::fabs(snap.bpm - lastBpm_) > kBpmChangeThreshold)
        {
            anchor(t, beatNow, deliveredSamples, snap.bpm, "bpm");
            anchorWrittenThisTick = true;
        }

        // Review fix (c): a steady-tempo take otherwise never anchors again
        // after "start", so TempoMap::sampleAt has no second reading to
        // interpolate against and beatAt drifts with the tracker's bpm
        // rounding. Re-anchor at least every kPeriodicAnchorBeats.
        if (!anchorWrittenThisTick && beatNow - lastAnchorBeat_ >= kPeriodicAnchorBeats)
            anchor(t, beatNow, deliveredSamples, snap.bpm, "periodic");
    }

    lastBpm_ = snap.bpm;
    current_.t = t;
    current_.beat = wholeBeats_ + lastPhase_ + beatOffset_;
    current_.sample = deliveredSamples;
    current_.bpm = snap.bpm;
}
