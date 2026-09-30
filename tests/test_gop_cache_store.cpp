// test_gop_cache_store -- s-rta-0929b gopcache (plan-gopcache.md 4.1 + HARMONY ADOPTION): the REAL VideoPlayer
// (src/media/VideoPlayer.cpp + FFmpeg), no GL, no decode thread -- the test is the GL thread (advanceFrame + a pick like
// uploadToTexture's) and drives the writer itself (VideoPlayerTestAccess, a friend of VideoPlayer). c1: a direction change
// is a discontinuity (a request-generation bump), whatever changes the direction (the reverse flag, a negative speed).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoPlayer.h"

#include <cstdint>
#include <string>
#include <vector>

struct VideoPlayerTestAccess
{
    static uint32_t gen(const VideoPlayer& p) { return p.gen_.load(); }
    static bool reverseNow(const VideoPlayer& p) { return p.reverseNow_; }
    static bool wantReverse(const VideoPlayer& p) { return p.wantReverse_.load(); }
};

namespace
{
juce::File fixture(const char* name)
{
    return juce::File(TEST_FIXTURES_DIR).getChildFile(name);
}
} // namespace

TEST_CASE("a direction change is a discontinuity: exactly one generation bump per flip, none while the direction holds",
          "[video_player][gopcache][s-rta-0929b]")
{
    VideoStats st;
    VideoPlayer p;
    p.setStats(&st);
    REQUIRE(p.open(fixture("video_h264_gop10_64x64.mp4")));
    const double dt = 1.0 / 120.0;
    p.advanceFrame(dt);
    const uint32_t g0 = VideoPlayerTestAccess::gen(p);
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0);          // forward stays forward: no bump
    p.setReverse(true);
    p.advanceFrame(dt);                                   // 0 - dt: the Loop wrap AND the flip share ONE bump
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);
    CHECK(VideoPlayerTestAccess::reverseNow(p));
    CHECK(VideoPlayerTestAccess::wantReverse(p));         // posted with the generation
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);      // reverse stays reverse
    p.setReverse(false);
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 2);
    CHECK_FALSE(VideoPlayerTestAccess::wantReverse(p));
    CHECK(st.directionChanges.load() == 2);
    // a negative speed runs the clock down with the reverse flag off: that too is reverse (S1 of the gates seat)
    p.setSpeed(-1.0f);
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 3);
    CHECK(VideoPlayerTestAccess::reverseNow(p));
    // speed 0 is no motion: the direction is kept, no bump
    p.setSpeed(0.0f);
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 3);
    CHECK(VideoPlayerTestAccess::reverseNow(p));
    p.close();
}

TEST_CASE("a PingPong reflection flips the direction with ONE generation bump (never mixed in the ring)",
          "[video_player][gopcache][s-rta-0929b]")
{
    VideoPlayer p;
    REQUIRE(p.open(fixture("video_h264_gop10_64x64.mp4")));   // 2.0 s
    p.setLoopMode(VideoPlayer::LoopMode::PingPong);
    p.seekTo(0.99);
    p.advanceFrame(0.0);
    const uint32_t g0 = VideoPlayerTestAccess::gen(p);
    p.advanceFrame(0.05);                                   // 1.98 + 0.05 -> reflected at 2.0: the top turn
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);
    CHECK(VideoPlayerTestAccess::reverseNow(p));
    p.advanceFrame(0.05);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);
    p.close();
}
