// test_clip_transport_sync -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T4, ruling
// ruling-tsan.md amendments 8 / 16). Renderer::syncMedia's transport sync (src/render/ClipTransportSync.h, the video
// AND the sequence branch) against a fake player: the intent `playing` is read once, and the write-back of the
// player's state is a compare-exchange on that read, so a message-thread trigger or pause landing INSIDE the sync
// window survives. The window is reproduced deterministically: the "message thread" writes between pushIntent and
// writeBack. GREEN-only; the teeth are the amendment-16 mutant "the syncMedia write-back as a plain store".
#include <catch2/catch_test_macros.hpp>
#include "render/ClipTransportSync.h"

namespace
{
// The player concept syncMedia uses (VideoPlayer / ImageSequence): play state, playhead, seek. advance() is the
// player's own clock: it moves the playhead while playing and stops itself at the end in OneShot (as the real ones).
struct FakePlayer
{
    bool playingState = false;
    double head = 0.0;
    int setPlayingCalls = 0;
    bool isPlaying() const { return playingState; }
    void setPlaying(bool p) { playingState = p; ++setPlayingCalls; }
    double getPlayheadPosition() const { return head; }
    void seekTo(double t) { head = t; }
    void advance(double step) { if (playingState) head += step; }
};

Clip videoClip()
{
    Clip c;
    c.mediaType = Clip::MediaType::Video;
    c.loopMode = Clip::LoopMode::Loop;
    return c;
}
} // namespace

TEST_CASE("a trigger that sets playing = true inside the sync window survives the write-back", "[clip_transport_sync]")
{
    Clip clip = videoClip();
    clip.playing = false;              // a never-played clip: the player is stopped
    FakePlayer player;

    const bool wanted = ClipTransportSync::pushIntent(clip, player);   // GL: reads the intent (false)
    REQUIRE_FALSE(wanted);
    REQUIRE_FALSE(player.isPlaying());
    clip.playing = true;               // message thread: the trigger's auto-play lands inside the window
    player.advance(0.01);
    ClipTransportSync::writeBack(clip, player, wanted);   // GL: the player is still stopped

    CHECK(clip.playing.load() == true);   // the trigger stands (a plain store would write the player's false)
    // The next sync pushes it: the video starts.
    const bool next = ClipTransportSync::pushIntent(clip, player);
    CHECK(next == true);
    CHECK(player.isPlaying());
}

TEST_CASE("a OneShot stop with an unchanged intent writes false", "[clip_transport_sync]")
{
    Clip clip = videoClip();
    clip.loopMode = Clip::LoopMode::OneShot;
    clip.inPoint = 0.1f;
    clip.outPoint = 0.5f;
    clip.playing = true;
    FakePlayer player;
    player.playingState = true;
    player.head = 0.25;

    const bool wanted = ClipTransportSync::pushIntent(clip, player);
    REQUIRE(wanted);
    player.advance(0.5);               // crosses the out-point (0.75 >= 0.5)
    ClipTransportSync::writeBack(clip, player, wanted);

    CHECK(clip.playing.load() == false);              // the OneShot stopped the model ...
    CHECK_FALSE(player.isPlaying());                  // ... and the player
    CHECK(clip.playheadPosition.load() == 0.75);      // the playhead the sync read (out-point tested on it)

    // A player that stopped ITSELF (end of media) with the intent still "play": the write-back clears the intent.
    Clip ended = videoClip();
    ended.playing = true;
    FakePlayer stopped;
    stopped.playingState = true;
    const bool w2 = ClipTransportSync::pushIntent(ended, stopped);
    stopped.playingState = false;      // the player's own stop during its advance
    ClipTransportSync::writeBack(ended, stopped, w2);
    CHECK(ended.playing.load() == false);
}

TEST_CASE("a pause landing inside the sync window survives the write-back", "[clip_transport_sync]")
{
    Clip clip = videoClip();
    clip.playing = true;
    FakePlayer player;
    player.playingState = true;

    const bool wanted = ClipTransportSync::pushIntent(clip, player);   // GL: reads "play"
    REQUIRE(wanted);
    clip.playing = false;              // message thread: the transport UI pauses inside the window
    player.advance(0.01);              // the player still plays this frame
    ClipTransportSync::writeBack(clip, player, wanted);

    CHECK(clip.playing.load() == false);   // the pause stands (a plain store would write the player's true)
    ClipTransportSync::pushIntent(clip, player);
    CHECK_FALSE(player.isPlaying());       // and the next sync pauses the player
}
