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
    // s-rta-0927 beat clock: the tracker's continuous beat time -- no wrap to detect (Pitfall 42).
    const double raw = static_cast<double>(snap.totalBeatCount) + static_cast<double>(snap.beatPhase);

    if (!haveTicked_)
    {
        haveTicked_ = true;
        startWall_ = wallNow;
        lastRaw_ = raw;
        lastBpm_ = snap.bpm;
        // `beat` counts from Record (D1 "beats since record start"), not from the tracker's count or its
        // last beat line: Record lands mid-beat (s-rta-0926 routine-grid).
        beatOffset_ = -raw;

        anchor(0.0, 0.0, deliveredSamples, snap.bpm, "start");
        current_ = { 0.0, 0.0, deliveredSamples, snap.bpm };
        return;
    }

    const double t = wallNow - startWall_;
    const bool wasUnmetered = (lastBpm_ <= 0.0f);
    const bool isUnmetered = (snap.bpm <= 0.0f);
    const double lastBeat = current_.beat;

    if (isUnmetered)
    {
        if (!wasUnmetered)
            anchor(t, lastBeat, deliveredSamples, 0.0f, "unmetered");
        // Frozen: beatOffset_/lastRaw_ untouched while unmetered.
        // No periodic anchors while unmetered (D1's documented limitation --
        // several equal-beat anchors would move TempoMap::tAt's canonical
        // answer for that beat).
    }
    else if (wasUnmetered)
    {
        // Re-locked: absorb the discontinuity so the frozen value carries
        // forward continuously (D1's "the clock follows the tracker").
        beatOffset_ = lastBeat - raw;
        lastRaw_ = raw;
        anchor(t, lastBeat, deliveredSamples, snap.bpm, "lock");
    }
    else
    {
        bool anchorWrittenThisTick = false;
        if (raw < lastRaw_)
        {
            // A realign that RESTARTED the beat (|d| < 0.5 by BPMTracker's rule) or a writer restart (test
            // injection): absorb into the offset so `beat` never dips (D1). A comparison on doubles --
            // never an unsigned subtraction across the count's wrap.
            beatOffset_ = lastBeat - raw;
            anchor(t, lastBeat, deliveredSamples, snap.bpm, "reset");
            anchorWrittenThisTick = true;
        }
        lastRaw_ = raw;

        const double beatNow = raw + beatOffset_;
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
    current_.beat = isUnmetered ? lastBeat : raw + beatOffset_;
    current_.sample = deliveredSamples;
    current_.bpm = snap.bpm;
}
