#include <catch2/catch_test_macros.hpp>
#include "model/Composition.h"
#include "ShowFixture.h"
#include "model/Autopilot.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// Regression coverage for the autopilot advance-gate stall (scout dossier
// .harmony/scout-autopilot-sources.md, 2026-07-30): autopilot could never
// advance AWAY from a non-playable clip (Source/Image/Camera). Two gates:
// (1) End-of-Video mode froze permanently on a non-playable active clip
//     (Autopilot.cpp:11/:64 — no playhead ever advances a source).
// (2) On-Beat mode required clip->playing, which sources are born without
//     (fixed at the 4 MainComponent.cpp creation sites, not here — but the
//     gate itself is exercised directly below by constructing clips with
//     playing set/unset).

namespace
{
    // One beat = phase ramps high then wraps low WITH the unwrapped count moving on, as BPMTracker publishes it
    // (FeatureSnapshot::totalBeatCount += 1 per phase wrap; Pitfall 42's injection rule). Mirrors the inline
    // pattern used in tests/test_compositor.cpp.
    void advanceOneBeat(Autopilot& autopilot, Composition& show, FeatureSnapshot& snap)
    {
        snap.beatPhase = 0.99f;
        autopilot.processFrame(show, snap);
        snap.beatPhase = 0.01f;
        snap.totalBeatCount++;
        autopilot.processFrame(show, snap);
    }
}

TEST_CASE("End-of-Video mode: non-playable clip falls through to beat advancement", "[autopilot]")
{
    Composition show = ShowFixture::makeShow(1, 3, 12, false);   // lane bf9b: one deck box, 3 shared layers

    for (int c = 0; c < 3; ++c)
    {
        Clip clip;
        clip.name = "source_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "perlin_noise";
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat4;
        show.decks[0].setClip(0, c, clip);
    }

    auto* layer = show.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = true;   // EoV mode — the frozen leg
    show.fire(0, 0, 0);               // first trigger auto-sets clip->playing = true

    REQUIRE(show.playingClip(0) != nullptr);
    REQUIRE_FALSE(show.playingClip(0)->isPlayable());
    REQUIRE(show.playingClip(0)->playing);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("Does not advance before 4 beats (not frozen forever either)")
    {
        for (int beat = 0; beat < 3; ++beat)
            advanceOneBeat(autopilot, show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 0);
    }

    SECTION("Advances on beat 4 instead of freezing at playhead 0.0")
    {
        // Confirm the EoV frame-based loop cannot advance this clip (source
        // playhead never reaches the out-point threshold — it has no playhead).
        REQUIRE(show.playingClip(0)->playheadPosition == 0.0);

        for (int beat = 0; beat < 4; ++beat)
            advanceOneBeat(autopilot, show, snap);

        REQUIRE(layer->runtime().activeClipColumn == 1); // advanced, not stuck on column 0
    }
}

TEST_CASE("End-of-Video mode: playable (Video) clip threshold behavior unchanged", "[autopilot]")
{
    Composition show = ShowFixture::makeShow(1, 3, 12, false);   // lane bf9b: one deck box, 3 shared layers

    for (int c = 0; c < 2; ++c)
    {
        Clip clip;
        clip.name = "video_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Video;
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        show.decks[0].setClip(0, c, clip);
    }

    auto* layer = show.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = true;
    show.fire(0, 0, 0);

    auto* clip = show.playingClip(0);
    REQUIRE(clip != nullptr);
    REQUIRE(clip->isPlayable());
    REQUIRE(clip->playing);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("Does not advance while playhead is below the out-point threshold")
    {
        clip->playheadPosition = 0.5;
        autopilot.processFrame(show, snap); // EoV is checked every frame, no beat cross needed
        REQUIRE(layer->runtime().activeClipColumn == 0);
    }

    SECTION("Advances once playhead reaches the out-point threshold")
    {
        clip->playheadPosition = 0.995; // outPoint defaults to 1.0, threshold is 0.99
        autopilot.processFrame(show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 1);
    }
}

TEST_CASE("On-Beat mode: advancement is gated on clip->playing (source parity)", "[autopilot]")
{
    Composition show = ShowFixture::makeShow(1, 3, 12, false);   // lane bf9b: one deck box, 3 shared layers

    for (int c = 0; c < 2; ++c)
    {
        Clip clip;
        clip.name = "source_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "perlin_noise";
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat1;
        show.decks[0].setClip(0, c, clip);
    }

    auto* layer = show.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = false; // On-Beat mode

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("playing = true (post-fix source-creation state) advances on beat")
    {
        {
            LayerRuntimeSnapshot rt = layer->runtime();
            rt.activeClipColumn = 0;
            rt.activeDeckId = show.decks[0].id;
            layer->setRuntime(rt);
        }
        auto* clip = show.decks[0].getClip(0, 0);
        REQUIRE(clip != nullptr);
        clip->playing = true; // matches the clip.playing = true; set at the 4
                               // MainComponent.cpp source-creation sites

        advanceOneBeat(autopilot, show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 1);
    }

    SECTION("playing = false (pre-fix source-creation state) never advances")
    {
        {
            LayerRuntimeSnapshot rt = layer->runtime();
            rt.activeClipColumn = 0;
            rt.activeDeckId = show.decks[0].id;
            layer->setRuntime(rt);
        }
        auto* clip = show.decks[0].getClip(0, 0);
        REQUIRE(clip != nullptr);
        clip->playing = false; // the bug: sources were born playing=false

        for (int beat = 0; beat < 4; ++beat)
            advanceOneBeat(autopilot, show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 0); // frozen
    }
}

TEST_CASE("advanceClip column selection skips empty columns", "[autopilot]")
{
    Composition show = ShowFixture::makeShow(1, 3, 12, false);   // lane bf9b: one deck box, 3 shared layers

    // Occupy columns 0, 2, 4; leave 1 and 3 empty (never assigned — genuinely
    // unoccupied, distinct from the clearCell() regression covered elsewhere).
    for (int c : { 0, 2, 4 })
    {
        Clip clip;
        clip.name = "clip_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Image;
        clip.playing = true;
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat1;
        show.decks[0].setClip(0, c, clip);
    }

    auto* layer = show.getLayer(0);
    layer->autopilotEnabled = true;
    show.fire(0, 0, 0);
    REQUIRE(show.decks[0].getClip(0, 1) == nullptr);
    REQUIRE(show.decks[0].getClip(0, 3) == nullptr);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("PlayNext skips column 1 (empty) and lands on column 2")
    {
        advanceOneBeat(autopilot, show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 2);
    }

    SECTION("PlayNext wraps past the end back to column 0")
    {
        show.fire(0, 0, 4); // last occupied column
        advanceOneBeat(autopilot, show, snap);
        REQUIRE(layer->runtime().activeClipColumn == 0); // wraps, skipping empty tail columns
    }
}

// ---------------------------------------------------------------------------------------------------------------
// s-rta-0927 follow-ups F3 (Pitfall 42): the beat-mode autopilot takes the FeatureSnapshot::totalBeatCount delta
// instead of detecting a phase wrap (beatPhase < last - 0.5). A GL stall of g beats used to lose beats (one crossing
// at most, none when the gap's end phase is not below its start phase minus 0.5); the delta is exact.
// Stream: 120 BPM, GL at 60 fps -> b(k) = k / 30 beats; snapshot = what BPMTracker publishes (phase = frac(b),
// totalBeatCount = floor(b), bpm 120).
namespace
{
    FeatureSnapshot beatSnap(double b)
    {
        FeatureSnapshot s;
        const double whole = std::floor(b);
        s.beatPhase = static_cast<float>(b - whole);
        s.totalBeatCount = static_cast<uint32_t>(whole);
        s.bpm = 120.0f;
        return s;
    }

    // Layer 0 on autopilot Beat4 / PlayNext over three columns (the retired test_deck_clock.cpp (f)'s setup); lane bf9b:
    // one deck box under the shared layers.
    Composition beat4Show()
    {
        Composition show = ShowFixture::makeShow(1, 3, 12, false);
        for (int c = 0; c < 3; ++c)
        {
            Clip clip;
            clip.name = "img_" + std::to_string(c);
            clip.mediaType = Clip::MediaType::Image;
            clip.autopilotAction = Clip::AutopilotAction::PlayNext;
            clip.autopilotDuration = Clip::AutopilotDuration::Beat4;
            show.decks[0].setClip(0, c, clip);
        }
        show.getLayer(0)->autopilotEnabled = true;
        show.fire(0, 0, 0);   // first trigger sets clip->playing
        return show;
    }

    // Ticks k = 0 .. kEnd (inclusive) at 1/30 beat each.
    void runTicks(Autopilot& ap, Composition& show, int kEnd)
    {
        for (int k = 0; k <= kEnd; ++k)
            ap.processFrame(show, beatSnap(k / 30.0));
    }
}

TEST_CASE("F3 stall: a 1.1-beat GL stall loses no beat (3.4 -> 4.5)", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;
    runTicks(ap, show, 102);                                    // b = 3.4: three beats played
    REQUIRE(show.playingClip(0)->beatsPlayed == 3);
    REQUIRE(show.getLayer(0)->runtime().activeClipColumn == 0);
    ap.processFrame(show, beatSnap(135 / 30.0));                // one tick at b = 4.5: phase 0.4 -> 0.5, count 3 -> 4
    CHECK(show.getLayer(0)->runtime().activeClipColumn == 1);
}

TEST_CASE("F3 stall: a 0.7-beat stall that begins at phase 0.5 and contains the wrap (3.5 -> 4.2)", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;
    runTicks(ap, show, 105);                                    // b = 3.5
    REQUIRE(show.playingClip(0)->beatsPlayed == 3);
    ap.processFrame(show, beatSnap(126 / 30.0));                // b = 4.2: 0.2 < 0.5 - 0.5 is false for the wrap reader
    CHECK(show.getLayer(0)->runtime().activeClipColumn == 1);
}

TEST_CASE("F3 stall: a 2.3-beat stall adds two beats (3.4 -> 5.7)", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;
    runTicks(ap, show, 102);                                    // b = 3.4
    REQUIRE(show.playingClip(0)->beatsPlayed == 3);
    ap.processFrame(show, beatSnap(171 / 30.0));                // b = 5.7: count 3 -> 5, the wrap reader sees nothing
    CHECK(show.getLayer(0)->runtime().activeClipColumn == 1);             // beatsPlayed 5 >= 4: one advance
}

TEST_CASE("F3 pin: without a stall the count reader advances on exactly the ticks the wrap reader did", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;

    // Expected advance ticks from the OLD detector on the same stream: every 4th wrap crossing.
    std::vector<int> expected;
    float last = 0.0f;
    int crossings = 0;
    for (int k = 0; k <= 16 * 30; ++k)
    {
        const float phase = beatSnap(k / 30.0).beatPhase;
        if (phase < last - 0.5f && (++crossings % 4) == 0)
            expected.push_back(k);
        last = phase;
    }
    REQUIRE(expected.size() == 4);

    std::vector<int> observed;
    int col = show.getLayer(0)->runtime().activeClipColumn;
    for (int k = 0; k <= 16 * 30; ++k)
    {
        ap.processFrame(show, beatSnap(k / 30.0));
        if (show.getLayer(0)->runtime().activeClipColumn != col)
        {
            observed.push_back(k);
            col = show.getLayer(0)->runtime().activeClipColumn;
        }
    }
    CHECK(observed == expected);
}

TEST_CASE("F3 pin: realign semantics match the wrap reader (BPMTracker hard realign)", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;

    SECTION("a realign from the second half of a beat counts one beat (count +1, phase -> 0)")
    {
        runTicks(ap, show, 111);                                // b = 3.7
        REQUIRE(show.playingClip(0)->beatsPlayed == 3);
        FeatureSnapshot s = beatSnap(111 / 30.0);
        s.beatPhase = 0.0f; s.totalBeatCount += 1;
        ap.processFrame(show, s);
        CHECK(show.getLayer(0)->runtime().activeClipColumn == 1);
    }

    SECTION("a realign from the first half counts nothing (count +0, phase -> 0)")
    {
        runTicks(ap, show, 99);                                 // b = 3.3
        REQUIRE(show.playingClip(0)->beatsPlayed == 3);
        FeatureSnapshot s = beatSnap(99 / 30.0);
        s.beatPhase = 0.0f;
        ap.processFrame(show, s);
        CHECK(show.getLayer(0)->runtime().activeClipColumn == 0);
        CHECK(show.playingClip(0)->beatsPlayed == 3);
    }
}

TEST_CASE("F3 pin: a writer reset (count back to 0) is a baseline, not a crossing", "[autopilot][stall]")
{
    Composition show = beat4Show();
    Autopilot ap;
    runTicks(ap, show, 102);                                    // b = 3.4, count 3
    REQUIRE(show.playingClip(0)->beatsPlayed == 3);
    FeatureSnapshot cleared;                                    // test-mode /api/reset publishes a cleared snapshot
    cleared.clear();
    ap.processFrame(show, cleared);
    CHECK(show.getLayer(0)->runtime().activeClipColumn == 0);
    CHECK(show.playingClip(0)->beatsPlayed == 3);
    runTicks(ap, show, 30);                                     // the stream restarts at b = 0 and wraps once at b = 1
    CHECK(show.getLayer(0)->runtime().activeClipColumn == 1);
}

TEST_CASE("F3 law: no beat-crossing reader detects a phase wrap any more", "[autopilot][law]")
{
#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif
    // Every beat-CROSSING consumer takes the totalBeatCount delta through OnsetPulse (Pitfall 42). The old detector
    // was `<x>Phase < <last> - 0.5`; this is the guard against it coming back in the three files that had it.
    const std::regex wrap(R"([Pp]hase\s*<\s*\w+\s*-\s*0\.5)");
    std::vector<std::string> hits;
    for (const char* rel : { "model/Autopilot.cpp", "render/Renderer.cpp", "MainComponent.cpp" })
    {
        std::ifstream in(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
        REQUIRE(in.good());
        std::string line;
        int n = 0;
        while (std::getline(in, line))
        {
            ++n;
            if (std::regex_search(line, wrap))
                hits.push_back(std::string(rel) + ":" + std::to_string(n) + ": " + line);
        }
    }
    std::ostringstream all;
    for (const auto& h : hits) all << h << "\n";
    INFO("wrap detectors still present:\n" << all.str());
    CHECK(hits.empty());
}
