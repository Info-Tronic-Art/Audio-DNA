#pragma once
#include "recording/TempoMap.h"
#include <cstdint>

struct FeatureSnapshot;   // src/analysis/FeatureSnapshot.h -- RecorderClock.cpp includes it

// ClockStamp: the "now" reading D1's RecorderClock hands out -- the same
// three-clock shape every captured point stamps (Stamp in Lane.h carries
// the seq+t+sample subset; `beat`/`bpm` live alongside it on the point).
struct ClockStamp
{
    double t = 0.0;
    double beat = 0.0;
    uint64_t sample = 0;
    float bpm = 0.0f;
};

// RecorderClock -- D1: fed by the 120 Hz message-thread tick (G12), turns
// each FeatureSnapshot into the take's three-clock timebase and grows the
// take-level TempoMap. One instance per take being recorded.
//
// Beat integration rule (D1): phase = snap.beatPhase; a backward jump of
// >= 0.5 is the ordinary sawtooth wrap (G14 precedent) -- one more whole
// beat elapsed. A SMALLER backward jump (resync/tap/relock -- BPMTracker.h
// phase reset) is absorbed into an offset so `beat` stays monotonic and
// non-decreasing instead of dipping; an anchor `why:"reset"` is written.
// While bpm == 0 (no lock, nothing tapped) the beat clock does not advance
// at all -- the segment is unmetered -- and resumes continuity (the same
// absorption trick) the moment a lock returns.
//
// Thread confinement (R7) is the CALLER's job (PerformanceRecorder /
// step 3's tickFeaturePipeline hook); this class holds no thread-affinity
// state of its own to assert on.
class RecorderClock
{
public:
    // wallNow: absolute wall-clock seconds (e.g.
    // juce::Time::getMillisecondCounterHiRes() / 1000.0) -- the FIRST
    // tick() call establishes t = 0 (recording start), matching G3.
    //
    // deliveredSamples: the audio callback's delivered-sample counter
    // (D10, AudioTap -- step 2). Until AudioTap lands, callers pass 0 and
    // every `sample` reading is 0 throughout the take; T1 (step 2) is what
    // actually proves sample alignment, not this class.
    void tick(const FeatureSnapshot& snap, double wallNow, uint64_t deliveredSamples);

    ClockStamp now() const { return current_; }
    const TempoMap& tempo() const { return tempo_; }

private:
    bool haveTicked_ = false;
    double startWall_ = 0.0;

    double wholeBeats_ = 0.0;
    double beatOffset_ = 0.0;
    double lastPhase_ = 0.0;
    float lastBpm_ = -1.0f;   // sentinel: never ticked

    TempoMap tempo_;
    ClockStamp current_;

    static constexpr float kBpmChangeThreshold = 0.05f;
};
