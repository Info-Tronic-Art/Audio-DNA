#include <catch2/catch_test_macros.hpp>
#include <string>

// s-rta-0924 step3 S3-B follow-up fix: commit b5931e8 consolidated
// TopBar's Play/Pause/Stop AND the LayerTransport binding's pad-toggle onto
// a single choke point, MainComponent::applyClipPlaying(). Before that
// commit, LayerTransport's toggle body was a bare `clip->playing =
// !clip->playing;` — it never touched clip->reverse. After consolidation it
// called applyClipPlaying(..., "play", ...) on the pause->play leg, and
// "play" unconditionally does `clip->reverse = false;` — a live-performance
// regression: pressing a pad to resume a paused, REVERSED clip silently
// flipped it forward.
//
// The fix (this lane) adds a "resume" action to applyClipPlaying — same as
// "play" (clip->playing = true) but WITHOUT the reverse reset — and makes
// LayerTransport's pause->play leg use "resume" instead of "play".
// TopBar's Play button is unchanged: it still sends "play" and still resets
// reverse (that IS its intended semantics — see MainComponent.cpp's
// topBar_->onPlay comment).
//
// applyClipPlaying() lives in MainComponent.cpp, which needs the full JUCE
// GUI stack and is not linked into any ctest target (see
// test_clip_replace_media_retire.cpp / test_renderer_source_confinement.cpp
// for the same, already-documented limitation in this repo). Following
// their established precedent ("mirror the mechanism, not the subsystem"),
// this test reproduces the small state machine that matters here — the
// action-string switch inside applyClipPlaying, and the ternary
// LayerTransport uses to pick an action — rather than exercising
// MainComponent.cpp's actual code.

namespace
{

// Mirrors the two Clip fields applyClipPlaying touches (Clip.h: `playing`,
// `reverse`).
struct ClipSim
{
    bool playing = false;
    bool reverse = false;
};

// Mirrors MainComponent::applyClipPlaying's action switch AS FIXED by this
// lane (MainComponent.cpp, case block starting `if (action == "play")`).
void applyClipPlaying_Fixed(ClipSim& clip, const std::string& action)
{
    if (action == "play") { clip.reverse = false; clip.playing = true; }
    else if (action == "resume") { clip.playing = true; }
    else if (action == "pause") { clip.playing = false; }
    else if (action == "stop") { clip.playing = false; }
    else if (action == "reverse") { clip.reverse = !clip.reverse; }
}

// Mirrors applyClipPlaying BEFORE this lane's fix: no "resume" action
// existed, so LayerTransport's pause->play leg had to send "play".
void applyClipPlaying_PreFix(ClipSim& clip, const std::string& action)
{
    if (action == "play") { clip.reverse = false; clip.playing = true; }
    else if (action == "pause") { clip.playing = false; }
    else if (action == "stop") { clip.playing = false; }
    else if (action == "reverse") { clip.reverse = !clip.reverse; }
}

// Mirrors the LayerTransport binding body AS FIXED by this lane
// (MainComponent.cpp, case Binding::Action::LayerTransport): toggles via
// "resume" (not "play") on the pause->play leg.
void layerTransportToggle_Fixed(ClipSim& clip)
{
    applyClipPlaying_Fixed(clip, clip.playing ? "pause" : "resume");
}

// Mirrors the LayerTransport binding body as it shipped in commit b5931e8
// (the regression this lane fixes): pause->play leg sent "play".
void layerTransportToggle_Regressed(ClipSim& clip)
{
    applyClipPlaying_PreFix(clip, clip.playing ? "pause" : "play");
}

// Mirrors TopBar's onPlay handler (MainComponent.cpp): always sends "play"
// — this is unchanged by this lane and is documented as intentional.
void topBarPlay(ClipSim& clip)
{
    applyClipPlaying_Fixed(clip, "play");
}

} // namespace

TEST_CASE("LayerTransport resume does not reset a reversed, paused clip's direction", "[transport][layer-transport]")
{
    ClipSim clip;
    clip.reverse = true;
    clip.playing = false;

    layerTransportToggle_Fixed(clip);

    CHECK(clip.playing == true);
    CHECK(clip.reverse == true);
}

TEST_CASE("LayerTransport pause leg still pauses, and does not touch reverse", "[transport][layer-transport]")
{
    ClipSim clip;
    clip.reverse = true;
    clip.playing = true;

    layerTransportToggle_Fixed(clip);

    CHECK(clip.playing == false);
    CHECK(clip.reverse == true);
}

TEST_CASE("the pre-fix LayerTransport toggle reproduces the regression (proves the test above has teeth)", "[transport][layer-transport]")
{
    ClipSim clip;
    clip.reverse = true;
    clip.playing = false;

    layerTransportToggle_Regressed(clip);

    CHECK(clip.playing == true);
    // Without the fix, "play" always clears reverse -- this is the bug.
    CHECK(clip.reverse == false);
}

TEST_CASE("TopBar Play still resets reverse (unchanged, intentional semantics)", "[transport][topbar]")
{
    ClipSim clip;
    clip.reverse = true;
    clip.playing = false;

    topBarPlay(clip);

    CHECK(clip.playing == true);
    CHECK(clip.reverse == false);
}

TEST_CASE("resume on an already-forward clip is a plain no-op on direction", "[transport][layer-transport]")
{
    ClipSim clip;
    clip.reverse = false;
    clip.playing = false;

    layerTransportToggle_Fixed(clip);

    CHECK(clip.playing == true);
    CHECK(clip.reverse == false);
}
