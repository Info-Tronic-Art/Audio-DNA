#include <catch2/catch_test_macros.hpp>
#include "model/Deck.h"
#include "model/Autopilot.h"
#include "analysis/FeatureSnapshot.h"

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
    // One "beat crossing" = phase ramps high then wraps low, matching the
    // detection in Autopilot::processFrame (beatPhase < lastBeatPhase_ - 0.5f).
    // Mirrors the inline pattern used in tests/test_compositor.cpp.
    void advanceOneBeat(Autopilot& autopilot, Deck& deck, FeatureSnapshot& snap)
    {
        snap.beatPhase = 0.99f;
        autopilot.processFrame(deck, snap);
        snap.beatPhase = 0.01f;
        autopilot.processFrame(deck, snap);
    }
}

TEST_CASE("End-of-Video mode: non-playable clip falls through to beat advancement", "[autopilot]")
{
    Deck deck;
    deck.initDefault();

    for (int c = 0; c < 3; ++c)
    {
        Clip clip;
        clip.name = "source_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "perlin_noise";
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat4;
        deck.setClip(0, c, clip);
    }

    auto* layer = deck.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = true;   // EoV mode — the frozen leg
    layer->triggerClip(0);               // first trigger auto-sets clip->playing = true

    REQUIRE(layer->getActiveClip() != nullptr);
    REQUIRE_FALSE(layer->getActiveClip()->isPlayable());
    REQUIRE(layer->getActiveClip()->playing);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("Does not advance before 4 beats (not frozen forever either)")
    {
        for (int beat = 0; beat < 3; ++beat)
            advanceOneBeat(autopilot, deck, snap);
        REQUIRE(layer->activeClipColumn == 0);
    }

    SECTION("Advances on beat 4 instead of freezing at playhead 0.0")
    {
        // Confirm the EoV frame-based loop cannot advance this clip (source
        // playhead never reaches the out-point threshold — it has no playhead).
        REQUIRE(layer->getActiveClip()->playheadPosition == 0.0);

        for (int beat = 0; beat < 4; ++beat)
            advanceOneBeat(autopilot, deck, snap);

        REQUIRE(layer->activeClipColumn == 1); // advanced, not stuck on column 0
    }
}

TEST_CASE("End-of-Video mode: playable (Video) clip threshold behavior unchanged", "[autopilot]")
{
    Deck deck;
    deck.initDefault();

    for (int c = 0; c < 2; ++c)
    {
        Clip clip;
        clip.name = "video_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Video;
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        deck.setClip(0, c, clip);
    }

    auto* layer = deck.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = true;
    layer->triggerClip(0);

    auto* clip = layer->getActiveClip();
    REQUIRE(clip != nullptr);
    REQUIRE(clip->isPlayable());
    REQUIRE(clip->playing);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("Does not advance while playhead is below the out-point threshold")
    {
        clip->playheadPosition = 0.5;
        autopilot.processFrame(deck, snap); // EoV is checked every frame, no beat cross needed
        REQUIRE(layer->activeClipColumn == 0);
    }

    SECTION("Advances once playhead reaches the out-point threshold")
    {
        clip->playheadPosition = 0.995; // outPoint defaults to 1.0, threshold is 0.99
        autopilot.processFrame(deck, snap);
        REQUIRE(layer->activeClipColumn == 1);
    }
}

TEST_CASE("On-Beat mode: advancement is gated on clip->playing (source parity)", "[autopilot]")
{
    Deck deck;
    deck.initDefault();

    for (int c = 0; c < 2; ++c)
    {
        Clip clip;
        clip.name = "source_" + std::to_string(c);
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "perlin_noise";
        clip.autopilotAction = Clip::AutopilotAction::PlayNext;
        clip.autopilotDuration = Clip::AutopilotDuration::Beat1;
        deck.setClip(0, c, clip);
    }

    auto* layer = deck.getLayer(0);
    layer->autopilotEnabled = true;
    layer->autopilotEndOfVideo = false; // On-Beat mode

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("playing = true (post-fix source-creation state) advances on beat")
    {
        layer->activeClipColumn = 0;
        auto* clip = layer->getClipAt(0);
        REQUIRE(clip != nullptr);
        clip->playing = true; // matches the clip.playing = true; set at the 4
                               // MainComponent.cpp source-creation sites

        advanceOneBeat(autopilot, deck, snap);
        REQUIRE(layer->activeClipColumn == 1);
    }

    SECTION("playing = false (pre-fix source-creation state) never advances")
    {
        layer->activeClipColumn = 0;
        auto* clip = layer->getClipAt(0);
        REQUIRE(clip != nullptr);
        clip->playing = false; // the bug: sources were born playing=false

        for (int beat = 0; beat < 4; ++beat)
            advanceOneBeat(autopilot, deck, snap);
        REQUIRE(layer->activeClipColumn == 0); // frozen
    }
}

TEST_CASE("advanceClip column selection skips empty columns", "[autopilot]")
{
    Deck deck;
    deck.initDefault();

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
        deck.setClip(0, c, clip);
    }

    auto* layer = deck.getLayer(0);
    layer->autopilotEnabled = true;
    layer->triggerClip(0);
    REQUIRE(layer->getClipAt(1) == nullptr);
    REQUIRE(layer->getClipAt(3) == nullptr);

    Autopilot autopilot;
    FeatureSnapshot snap;

    SECTION("PlayNext skips column 1 (empty) and lands on column 2")
    {
        advanceOneBeat(autopilot, deck, snap);
        REQUIRE(layer->activeClipColumn == 2);
    }

    SECTION("PlayNext wraps past the end back to column 0")
    {
        layer->triggerClip(4); // last occupied column
        advanceOneBeat(autopilot, deck, snap);
        REQUIRE(layer->activeClipColumn == 0); // wraps, skipping empty tail columns
    }
}
