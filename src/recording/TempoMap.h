#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <cstdint>

// TempoMap -- D1: a take-level tempo map, anchors {t, beat, sample, bpm,
// why} appended whenever RecorderClock decides the mapping between the
// three clocks needs re-anchoring (locked BPM change > 0.05, a resync/tap/
// relock "reset", a lock/unlock edge, at minimum). Between anchors, beat(t)
// is linear at the anchor's bpm (constant -- unmetered -- while bpm == 0);
// sample(t) is linear between the anchors' own (t, sample) readings. Used
// by edit operations (LATER, D5) to re-derive one domain after a move in
// another; row 1 only needs it built and queryable.
struct TempoAnchor
{
    double t = 0.0;
    double beat = 0.0;
    uint64_t sample = 0;
    float bpm = 0.0f;
    std::string why;

    juce::var toVar() const;
    static TempoAnchor fromVar(const juce::var& v);
};

class TempoMap
{
public:
    std::vector<TempoAnchor> a;

    // Inserts keeping `a` sorted by t (capture appends in time order, so
    // this is a push_back in the common case; a general insert keeps the
    // invariant even if a caller ever appends out of order).
    void append(TempoAnchor anchor);

    // beat(t): linear at the bracketing anchor's bpm; constant (unmetered)
    // while that anchor's bpm == 0 (D1). Empty map -> 0.
    double beatAt(double t) const;

    // t(beat): inverse of beatAt within a metered segment. A beat value
    // that falls inside an UNMETERED segment (bpm == 0, beat does not
    // advance there) is not invertible -- returns that segment's anchor.t
    // as the canonical answer (documented limitation, not a crash).
    double tAt(double beat) const;

    // sample(t): linear between the bracketing anchors' own (t, sample)
    // readings (D10's delivered-sample counter is the source, not a
    // device-rate computation -- the anchors already carry real readings) --
    // but only when that segment spans >= 1 s of t. A shorter segment (a 12
    // ms start/lock pair can imply ~85,000 samples/s) or a single-anchor map
    // (no segment to measure at all) extrapolates at `nominalRate` instead
    // (the resolved asset's sample rate; s-rta-0928b plan-sampleat A1,
    // FILED by tempo.md section 8).
    uint64_t sampleAt(double t, double nominalRate) const;

    juce::var toVar() const;
    static TempoMap fromVar(const juce::var& v);
};
