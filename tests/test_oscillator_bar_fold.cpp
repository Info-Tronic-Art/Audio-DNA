#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "signal/OscillatorSignal.h"
#include "signal/EnvelopeSignal.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>

using Catch::Matchers::WithinAbs;

// S166-L5a: OscillatorSignal::getValue (and EnvelopeSignal::getValue) folded
// beatPhase + beatInBar into a phase bounded to [0, 4) -- one bar -- so any
// beatDuration_ > 4 (e.g. the UI's "8 beat" option) could never complete a
// full cycle; it retraced the first fraction of its waveform forever. The
// fix extends the phase across bars via barCount (FeatureSnapshot.h: "bars
// since last phrase reset").
//
// barCount resets only on rare structural-transition events (drop hit /
// breakdown exit -- BPMTracker::updatePhrase, BPMTracker.cpp:408-450), NOT
// on a fixed period, and it resets independently of beatInBar/beatPhase
// (BPMTracker::resetPhrase, BPMTracker.cpp:452-457, touches only
// barCount_/phrasePhase_/prevDownbeatDetected_). These tests pin both
// halves of that trade-off: no artificial jump at a fixed "phrase" period,
// but a real jump at an actual structural-reset event.

TEST_CASE("OscillatorSignal 8-beat cycle completes across two bars (was bounded to 4/8)", "[signal][oscillator]")
{
    // beatDuration_ = 8 beats = 2 bars. SawUp's value IS cyclePhase, so it
    // is the simplest probe for "did the phase actually reach here".
    OscillatorSignal osc("test", OscillatorSignal::WaveShape::SawUp, 8.0f);

    FeatureSnapshot start;
    start.beatPhase = 0.0f;
    start.beatInBar = 0;
    start.barCount = 0;
    REQUIRE_THAT(osc.getValue(start), WithinAbs(0.0f, 0.001f));

    // Pre-fix, totalBeatPhase was bounded to [0,4), so cyclePhase for
    // beatDuration_=8 could never exceed 0.5. barCount=1 (second bar of the
    // 2-bar cycle) must now push cyclePhase past that ceiling.
    FeatureSnapshot secondBar;
    secondBar.beatPhase = 0.0f;
    secondBar.beatInBar = 0;
    secondBar.barCount = 1;
    REQUIRE_THAT(osc.getValue(secondBar), WithinAbs(0.5f, 0.001f));

    // Near the end of the second bar, the cycle should be nearly complete
    // (approaching 1.0), not stalled at ~0.5.
    FeatureSnapshot nearEnd;
    nearEnd.beatPhase = 0.99f;
    nearEnd.beatInBar = 3;
    nearEnd.barCount = 1;
    float valNearEnd = osc.getValue(nearEnd);
    REQUIRE(valNearEnd > 0.9f);

    // Monotonic, non-decreasing sweep across the full two bars (no
    // mid-cycle stall or wrap before completion).
    float prev = -1.0f;
    for (int barCount = 0; barCount <= 1; ++barCount)
    {
        for (int beatInBar = 0; beatInBar < 4; ++beatInBar)
        {
            for (float beatPhase : {0.0f, 0.25f, 0.5f, 0.75f})
            {
                FeatureSnapshot s;
                s.beatPhase = beatPhase;
                s.beatInBar = static_cast<uint8_t>(beatInBar);
                s.barCount = static_cast<uint16_t>(barCount);
                float v = osc.getValue(s);
                INFO("barCount=" << barCount << " beatInBar=" << beatInBar << " beatPhase=" << beatPhase);
                REQUIRE(v >= prev - 0.0001f);
                prev = v;
            }
        }
    }
    REQUIRE(prev > 0.9f); // reached near the top of the ramp, not stuck at 0.5
}

TEST_CASE("OscillatorSignal Sine 8-beat cycle reaches both extremes", "[signal][oscillator]")
{
    // Sine peaks (value=1.0) at cyclePhase=0.25 and troughs (value=0.0) at
    // cyclePhase=0.75. For beatDuration_=8, cyclePhase=0.75 requires
    // totalBeatPhase=6, which needs barCount=1 (4*1 + beatInBar=2). Pre-fix
    // (no barCount term), totalBeatPhase was bounded to [0,4), so cyclePhase
    // could never exceed 0.5 -- the trough was UNREACHABLE.
    OscillatorSignal osc("test", OscillatorSignal::WaveShape::Sine, 8.0f);

    FeatureSnapshot atPeak; // totalBeatPhase = 2 -> cyclePhase = 0.25
    atPeak.beatPhase = 0.0f;
    atPeak.beatInBar = 2;
    atPeak.barCount = 0;
    REQUIRE_THAT(osc.getValue(atPeak), WithinAbs(1.0f, 0.01f));

    FeatureSnapshot atTrough; // totalBeatPhase = 6 -> cyclePhase = 0.75
    atTrough.beatPhase = 0.0f;
    atTrough.beatInBar = 2;
    atTrough.barCount = 1;
    REQUIRE_THAT(osc.getValue(atTrough), WithinAbs(0.0f, 0.01f));
}

TEST_CASE("OscillatorSignal short beat durations unchanged across bars", "[signal][oscillator]")
{
    // Success criterion: values at 0.25, 0.5, 1, 2, 4 beats must be
    // UNCHANGED by the fix. For these durations 4*barCount/beatDuration_ is
    // always an integer number of whole cycles, so folding it in must be a
    // no-op after fmod -- verified here across several distinct bar counts,
    // not just barCount=0.
    const float durations[] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f};
    const OscillatorSignal::WaveShape shapes[] = {
        OscillatorSignal::WaveShape::Sine,
        OscillatorSignal::WaveShape::SawUp,
        OscillatorSignal::WaveShape::Square,
        OscillatorSignal::WaveShape::Triangle,
        OscillatorSignal::WaveShape::SawDown,
    };
    const uint16_t barCounts[] = {0, 1, 2, 3, 5, 10, 50};

    FeatureSnapshot baseline;
    baseline.beatPhase = 0.37f;
    baseline.beatInBar = 2;
    baseline.barCount = 0;

    for (float d : durations)
    {
        for (auto shape : shapes)
        {
            OscillatorSignal osc("test", shape, d);
            float expected = osc.getValue(baseline);

            for (uint16_t bc : barCounts)
            {
                FeatureSnapshot s = baseline;
                s.barCount = bc;
                float actual = osc.getValue(s);
                INFO("beatDuration=" << d << " shape=" << static_cast<int>(shape) << " barCount=" << bc);
                REQUIRE_THAT(actual, WithinAbs(expected, 0.0005f));
            }
        }
    }
}

TEST_CASE("Live-app regression anchors: Mod 1 (OscillatorSignal Sine, duration 1) and Mod 2 (EnvelopeSignal, duration 4) unchanged at barCount=0", "[signal][oscillator][envelope]")
{
    // Measured directly off the running app via /api/signals by the
    // team-lead sweeping beatPhase with beatInBar=0, barCount=0 (S166-L5a
    // build-slot handoff). These are real, not predicted, values -- the fix
    // must reproduce them exactly since barCount=0 contributes nothing.
    //
    // CORRECTION (independent reviewer + team-lead, post-hoc): Mod 2 is NOT
    // an OscillatorSignal SawUp at duration 2 -- it is registered as
    // EnvelopeSignal("Mod 2", 4.0f) with its default control points
    // (SignalRegistry.cpp:70), a triangle. The 0.0/0.125/0.25/0.375 sweep is
    // real and unchanged -- it happens to numerically coincide with a
    // duration-2 SawUp because it only sampled the envelope's rising linear
    // segment (0,0)->(0.5,1), where t = cyclePhase/0.5 = 2*(beatPhase/4) =
    // beatPhase/2, same arithmetic as a SawUp at half the duration. The
    // measurement was right; the object it was attributed to was wrong.
    // Both Mod 1 and Mod 2 constructions below are verified directly against
    // SignalRegistry.cpp:65-70, not inferred from the measurement.
    OscillatorSignal mod1("Mod 1", OscillatorSignal::WaveShape::Sine, 1.0f);
    EnvelopeSignal mod2("Mod 2", 4.0f);

    const float sweep[] = {0.0f, 0.25f, 0.5f, 0.75f};
    const float mod1Expected[] = {0.5000f, 1.0000f, 0.5000f, 0.0000f};
    const float mod2Expected[] = {0.0000f, 0.1250f, 0.2500f, 0.3750f};

    for (int i = 0; i < 4; ++i)
    {
        FeatureSnapshot s;
        s.beatPhase = sweep[i];
        s.beatInBar = 0;
        s.barCount = 0;

        INFO("beatPhase=" << sweep[i]);
        REQUIRE_THAT(mod1.getValue(s), WithinAbs(mod1Expected[i], 0.001f));
        REQUIRE_THAT(mod2.getValue(s), WithinAbs(mod2Expected[i], 0.001f));
    }

    // The distinguishing assertion the mislabeled measurement lacked: at
    // cyclePhase=0.5 (totalBeatPhase=2, beatInBar=2, barCount=0, duration 4)
    // the REAL Mod 2 sits exactly on its peak control point -> value 1.0.
    // A SawUp at duration 2 could never produce this: at the same snapshot
    // its cyclePhase = fmod(2/2, 1.0) = 0.0, so a SawUp would read 0.0, not
    // 1.0 -- this single point is what tells the two hypotheses apart, and
    // its absence is why the wrong object passed before.
    FeatureSnapshot atPeak;
    atPeak.beatPhase = 0.0f;
    atPeak.beatInBar = 2;
    atPeak.barCount = 0;
    REQUIRE_THAT(mod2.getValue(atPeak), WithinAbs(1.0f, 0.001f));
}

TEST_CASE("EnvelopeSignal completes a full cycle over 8 beats (2 bars)", "[signal][envelope]")
{
    // Default control points ramp 0 -> 1 (at cyclePhase 0.5) -> 0 (at
    // cyclePhase 1.0). With beatDuration_=8 the peak must land at bar
    // boundary barCount=1 (totalBeatPhase=4, cyclePhase=0.5), not at the
    // end of bar 0 as it would if the fix were missing.
    EnvelopeSignal env("test", 8.0f);

    FeatureSnapshot start;
    start.beatPhase = 0.0f;
    start.beatInBar = 0;
    start.barCount = 0;
    REQUIRE_THAT(env.getValue(start), WithinAbs(0.0f, 0.01f));

    FeatureSnapshot peak;
    peak.beatPhase = 0.0f;
    peak.beatInBar = 0;
    peak.barCount = 1;
    REQUIRE_THAT(env.getValue(peak), WithinAbs(1.0f, 0.01f));

    FeatureSnapshot nearEnd;
    nearEnd.beatPhase = 0.99f;
    nearEnd.beatInBar = 3;
    nearEnd.barCount = 1;
    REQUIRE(env.getValue(nearEnd) < 0.1f); // ramping back down toward 0
}

TEST_CASE("EnvelopeSignal 16-beat option (SignalInspector's longest envelope duration) completes across 4 bars", "[signal][envelope]")
{
    // SignalInspector::envBeatDurationSelector_ offers up to 16 beats (4
    // bars) for envelopes -- a worse pre-fix case than the oscillator's 8:
    // totalBeatPhase bounded to [0,4) meant cyclePhase never exceeded
    // 4/16 = 0.25, a quarter of the cycle, so the envelope never even
    // reached its peak (position 0.5) before this fix.
    EnvelopeSignal env("test", 16.0f);

    FeatureSnapshot start;
    start.beatPhase = 0.0f;
    start.beatInBar = 0;
    start.barCount = 0;
    REQUIRE_THAT(env.getValue(start), WithinAbs(0.0f, 0.01f));

    // Peak (control point position 0.5) requires totalBeatPhase=8, i.e.
    // barCount=2 -- unreachable before the fix.
    FeatureSnapshot peak;
    peak.beatPhase = 0.0f;
    peak.beatInBar = 0;
    peak.barCount = 2;
    REQUIRE_THAT(env.getValue(peak), WithinAbs(1.0f, 0.01f));

    FeatureSnapshot nearEnd;
    nearEnd.beatPhase = 0.99f;
    nearEnd.beatInBar = 3;
    nearEnd.barCount = 3;
    REQUIRE(env.getValue(nearEnd) < 0.1f); // ramped back down toward 0
}

TEST_CASE("EnvelopeSignal default duration (4.0, the boundary case) is unchanged across bars", "[signal][envelope]")
{
    const uint16_t barCounts[] = {0, 1, 2, 5, 10};
    FeatureSnapshot baseline;
    baseline.beatPhase = 0.6f;
    baseline.beatInBar = 1;
    baseline.barCount = 0;

    EnvelopeSignal env("test"); // default beatDuration_ = 4.0f
    float expected = env.getValue(baseline);

    for (uint16_t bc : barCounts)
    {
        FeatureSnapshot s = baseline;
        s.barCount = bc;
        INFO("barCount=" << bc);
        REQUIRE_THAT(env.getValue(s), WithinAbs(expected, 0.0005f));
    }
}

TEST_CASE("Phrase-reset trade-off is pinned: no jump from a fixed period, a real jump from an actual reset", "[signal][oscillator]")
{
    // beatDuration_=12 (3-bar cycle) deliberately does NOT divide the
    // default 8-bar phrase length evenly, so it exercises the trade-off
    // named in the S166-L5a work packet.
    OscillatorSignal osc("test", OscillatorSignal::WaveShape::SawUp, 12.0f);

    // (1) Crossing a "phrase" boundary (8 bars) with NO structural-reset
    // event -- barCount simply keeps incrementing, exactly as
    // BPMTracker::updatePhrase does absent a drop/breakdown transition --
    // must NOT jump. This pins the choice of raw barCount (not
    // barCount % phraseBars_ / phrasePhase): the oscillator does not care
    // about the phrase boundary at all.
    FeatureSnapshot beforePhraseBoundary;
    beforePhraseBoundary.beatPhase = 0.9f;
    beforePhraseBoundary.beatInBar = 3;
    beforePhraseBoundary.barCount = 7; // last bar before an 8-bar phrase boundary
    float justBefore = osc.getValue(beforePhraseBoundary);

    FeatureSnapshot afterPhraseBoundary;
    afterPhraseBoundary.beatPhase = 0.0f;
    afterPhraseBoundary.beatInBar = 0;
    afterPhraseBoundary.barCount = 8; // one hop later, no reset occurred
    float justAfter = osc.getValue(afterPhraseBoundary);

    // beatDuration_=12 -> cyclePhase (a 3-bar-per-cycle sawtooth) wraps at
    // cycle boundaries barCount=0,3,6,9...; barCount=7->8 is mid-cycle (both
    // fall in the 6-8 cycle), so the two samples one hop apart should keep
    // climbing smoothly with no reset-induced drop.
    INFO("justBefore=" << justBefore << " justAfter=" << justAfter);
    REQUIRE(justAfter > justBefore); // kept climbing, no reset-induced drop

    // (2) An ACTUAL structural-transition phrase reset: barCount snaps to 0
    // (BPMTracker::resetPhrase / the drop-entry branch in updatePhrase)
    // while beatInBar/beatPhase continue undisturbed from wherever the beat
    // clock already was (those fields are owned by updateBeatPhase /
    // updateBarPhase, not touched by the phrase-reset paths). This DOES
    // jump the oscillator phase -- the accepted, named trade-off.
    FeatureSnapshot midCycle;
    midCycle.beatPhase = 0.5f;
    midCycle.beatInBar = 2;
    midCycle.barCount = 5;
    float beforeReset = osc.getValue(midCycle);

    FeatureSnapshot justAfterReset = midCycle;
    justAfterReset.barCount = 0; // structural-transition reset; beat clock unchanged

    float afterReset = osc.getValue(justAfterReset);

    INFO("beforeReset=" << beforeReset << " afterReset=" << afterReset);
    REQUIRE(std::abs(afterReset - beforeReset) > 0.3f); // a real, visible jump
}
