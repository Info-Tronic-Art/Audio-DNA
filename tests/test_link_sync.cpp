// s-rta-0926b bpm2: Ableton Link in a DEFAULT build (AUDIODNA_BUILD_LINK OFF, so
// AUDIODNA_HAS_LINK is not defined -- this test target never defines it, exactly
// like the default app build). Link is not compiled in, so LinkSync must be honestly
// unavailable: whatever calls setEnabled(true) -- the TopBar toggle, or anything else --
// it must never report enabled and never hand out a tempo. Pre-fix it did both
// (enabled_ was a plain flag and bpm_ defaulted to 120), so MainComponent's ~30 Hz
// timer forced the tracker into manual mode at a made-up 120 BPM.
//
// The loop below performs, per UI tick, the calls MainComponent::timerCallback makes
// ("P21: Ableton Link sync": isEnabled() -> update() -> getBPM() -> a positive tempo goes
// to applyTempoCommand("link", bpm, Human, linkTick=true)), and for a fed tempo the two
// tracker calls that applyTempoCommand's "link" + linkTick branch makes (setManualMode(true),
// followExternalTempo). MainComponent.cpp itself cannot be linked into a unit test, so the
// test drives the real LinkSync and the real BPMTracker through that call sequence.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "analysis/BPMTracker.h"
#include "sync/LinkSync.h"

using Catch::Matchers::WithinAbs;

namespace
{
    // One analysis hop in AnalysisThread's stage-5 order, with no audio.
    void quietHop(BPMTracker& tracker)
    {
        tracker.processRawBPM(0.0f, 0.0f, false);
        tracker.feedDownbeatFeatures(0.0f, 0.0f, 0.0f, 0);
    }

    // MainComponent::timerCallback's Link block; returns whether a tempo was fed.
    bool linkUiTick(LinkSync& link, BPMTracker& tracker)
    {
        if (!link.isEnabled())
            return false;
        link.update();
        const double linkBPM = link.getBPM();
        if (linkBPM <= 0.0)
            return false;
        tracker.setManualMode(true);                                   // applyTempoCommand("link", ..., linkTick)
        tracker.followExternalTempo(static_cast<float>(linkBPM));
        return true;
    }
}

TEST_CASE("default build (no Ableton Link compiled): enabling Link neither reports enabled nor "
          "feeds a tempo -- the tracker is never forced into manual mode at a fake 120 BPM",
          "[link][sync][s-rta-0926b]")
{
    LinkSync link;
    BPMTracker tracker(512, 1024, 48000);
    tracker.setManualMode(true);
    tracker.setManualBPM(97.0f);      // an operator tempo that Link must not overwrite
    tracker.setManualMode(false);     // back to auto, as the operator left it
    for (int i = 0; i < 10; ++i) quietHop(tracker);
    REQUIRE_FALSE(tracker.isManualMode());
    const float bpmBefore = tracker.bpm();
    REQUIRE_THAT(bpmBefore, WithinAbs(97.0, 1e-3));

    // The TopBar toggle's callback (MainComponent: onLinkToggled -> linkSync_.setEnabled).
    link.setEnabled(true);

    const double hopsPerTick = (48000.0 / 512.0) / 30.0;   // 3.125 hops between UI ticks
    int ticks = 0, fed = 0;
    double nextTick = 0.0;
    for (int h = 0; h < 188; ++h)   // ~2 s
    {
        if (h >= nextTick)
        {
            if (linkUiTick(link, tracker)) ++fed;
            ++ticks;
            nextTick += hopsPerTick;
        }
        quietHop(tracker);
    }

    INFO("UI ticks " << ticks << ", Link tempos fed " << fed << ", isEnabled " << link.isEnabled()
         << ", getBPM " << link.getBPM() << ", tracker bpm " << tracker.bpm()
         << ", manual " << tracker.isManualMode());
    REQUIRE(ticks >= 60);                       // non-vacuous: ~30 ticks/s for 2 s
    REQUIRE_FALSE(link.isEnabled());            // RED pre-fix: true
    REQUIRE(link.getBPM() == 0.0);              // RED pre-fix: 120 (the fake default)
    REQUIRE(fed == 0);                          // RED pre-fix: every tick fed 120
    REQUIRE_FALSE(tracker.isManualMode());      // RED pre-fix: forced into manual mode
    REQUIRE_THAT(tracker.bpm(), WithinAbs(bpmBefore, 1e-3));   // RED pre-fix: 120

    // Turning it off again is harmless and leaves the same state.
    link.setEnabled(false);
    REQUIRE_FALSE(link.isEnabled());
    REQUIRE(link.getBPM() == 0.0);
}
