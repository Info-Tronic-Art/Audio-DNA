// test_gop_cache_store -- s-rta-0929b gopcache (plan-gopcache.md 4.1 + HARMONY ADOPTION): the REAL VideoPlayer
// (src/media/VideoPlayer.cpp + FFmpeg), no GL, no decode thread -- the test is the GL thread (advanceFrame + a pick like
// uploadToTexture's) and drives the writer itself (VideoPlayerTestAccess, a friend of VideoPlayer). c1: a direction change
// is a discontinuity (a request-generation bump), whatever changes the direction (the reverse flag, a negative speed).
// c1b: the forward writer stepped without a thread publishes a Loop clip's frames in order (plan case 7).
// c2: GopCache::Store on its own -- native-format copies, the budget's refusals and floors, reuse, every byte returned.
#include <catch2/catch_test_macros.hpp>

#include "media/GopCacheStore.h"
#include "media/VideoPlayer.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

struct VideoPlayerTestAccess
{
    static uint32_t gen(const VideoPlayer& p) { return p.gen_.load(); }
    static bool reverseNow(const VideoPlayer& p) { return p.reverseNow_; }
    static bool wantReverse(const VideoPlayer& p) { return p.wantReverse_.load(); }
    // c1b: the writer without a thread, and the GL thread's pick (uploadToTexture without GL / budget)
    static int stepUntilIdle(VideoPlayer& p, int limit = 200000)
    {
        int n = 0;
        while (n < limit && p.decodeStep())
            ++n;
        return n;
    }
    static long pick(VideoPlayer& p)
    {
        const uint32_t g = p.gen_.load();
        const auto k = p.ring_.pick(p.currentTime_, g, 0.5 * p.frameDur_, (VideoPlayer::kWriterLookAhead + 1) * p.frameDur_,
                                    p.reverseNow_);
        if (k.slot < 0)
            return -1;
        const int prev = p.retire_.shown(k.slot, false);
        if (prev >= 0)
        {
            p.ring_.release(prev);
            p.releasedThisFrame_ = true;
        }
        return std::lround(k.pts / p.frameDur_);
    }
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

TEST_CASE("c1b: forward Loop, the writer stepped without a thread -- 30 frames published and shown in order, one each",
          "[video_player][gopcache][s-rta-0929b]")
{
    VideoStats st;
    VideoPlayer p;
    p.setStats(&st);
    REQUIRE(p.open(fixture("video_h264_gop10_64x64.mp4")));
    std::vector<long> shown;
    for (int i = 0; i < 4 * 30 && shown.size() < 30; ++i)
    {
        p.advanceFrame(1.0 / 120.0);
        VideoPlayerTestAccess::stepUntilIdle(p);
        const long k = VideoPlayerTestAccess::pick(p);
        if (k >= 0 && (shown.empty() || shown.back() != k))
            shown.push_back(k);
    }
    REQUIRE(shown.size() == 30);
    for (size_t i = 0; i < shown.size(); ++i)
        CHECK(shown[i] == static_cast<long>(i));   // frame 0 is open()'s, then every frame once
    CHECK(st.seeks.load() == 0);
    CHECK(st.framesDropped.load() == 0);
    p.close();
}

namespace
{
AVFrame* patternFrame(int w, int h, int seed)
{
    AVFrame* f = av_frame_alloc();
    f->format = AV_PIX_FMT_YUV420P;
    f->width = w;
    f->height = h;
    REQUIRE(av_frame_get_buffer(f, 0) == 0);
    for (int plane = 0; plane < 3; ++plane)
    {
        const int ph = plane == 0 ? h : h / 2, pw = plane == 0 ? w : w / 2;
        for (int y = 0; y < ph; ++y)
            for (int x = 0; x < pw; ++x)
                f->data[plane][y * f->linesize[plane] + x] = static_cast<uint8_t>((seed * 31 + plane * 7 + y * 3 + x) & 0xff);
    }
    return f;
}

bool samePlanes(const AVFrame* a, const AVFrame* b)
{
    for (int plane = 0; plane < 3; ++plane)
    {
        const int ph = plane == 0 ? a->height : a->height / 2, pw = plane == 0 ? a->width : a->width / 2;
        for (int y = 0; y < ph; ++y)
            if (std::memcmp(a->data[plane] + y * a->linesize[plane], b->data[plane] + y * b->linesize[plane],
                            static_cast<size_t>(pw)) != 0)
                return false;
    }
    return true;
}
} // namespace

TEST_CASE("c2: GopCache::Store -- copies in the native format, the budget refuses past the cap (floors win), a replace reuses "
          "memory, freeSlot / clear give every byte back, holes stay", "[gopcache][s-rta-0929b]")
{
    VideoStats st;
    GopCache::Budget budget;
    GopCache::Store store;
    store.configure(AV_PIX_FMT_YUV420P, 64, 64, &budget, &st);
    std::vector<AVFrame*> src;
    for (int i = 0; i < 12; ++i)
        src.push_back(patternFrame(64, 64, i));
    // the first slot settles the estimate against the REAL allocation (GC7: real bytes are counted)
    REQUIRE(store.store(src[0], 0, 0.0, { GopCache::Keep::Store, -1 }, GopCache::Slot{ 0, 0, GopCache::Pool::Future }, 0));
    const int64_t est = store.frameBytes();
    REQUIRE(est >= 64 * 64 * 3 / 2);
    CHECK(budget.bytes.load() == est);
    store.clear();
    CHECK(budget.bytes.load() == 0);
    budget.total.store(10 * est);
    // a new slot per frame until the budget refuses at 10 frames
    int stored = 0;
    for (int i = 0; i < 12; ++i)
        stored += store.store(src[static_cast<size_t>(i)], i, i / 30.0, { GopCache::Keep::Store, -1 },
                              GopCache::Slot{ i, i, GopCache::Pool::Future }, 4 * est)
                      ? 1
                      : 0;
    CHECK(stored == 10);                                 // exactly the budget (the floor, 4 frames, is inside it)
    CHECK(store.resident() == stored);
    CHECK(budget.bytes.load() == store.bytes());
    CHECK(st.gopCacheBytes.load() == store.bytes());
    CHECK(st.gopCacheFrames.load() == stored);
    CHECK(budget.active.load() == 1);
    CHECK(st.gopCacheActive.load() == 1);
    for (int i = 0; i < stored; ++i)                     // a copy, not a reference: the source may change
    {
        REQUIRE(store.slotOf(i) >= 0);
        CHECK(samePlanes(store.frameAt(store.slotOf(i)), src[static_cast<size_t>(i)]));
    }
    // a replace reuses the slot's memory: no new bytes; the old frame is gone from the index
    const int64_t bytesBefore = store.bytes();
    const int s0 = store.slotOf(0);
    REQUIRE(store.store(src[11], 11, 11 / 30.0, { GopCache::Keep::Replace, s0 }, GopCache::Slot{ 11, 0, GopCache::Pool::Future }, 0));
    CHECK(store.bytes() == bytesBefore);
    CHECK(store.slotOf(0) == -1);
    CHECK(store.slotOf(11) == s0);
    CHECK(samePlanes(store.frameAt(s0), src[11]));
    CHECK(st.gopCacheEvictions.load() == 1);
    // a frame of another format / size, or flagged corrupt, is never stored (GC3)
    AVFrame* odd = patternFrame(32, 32, 99);
    CHECK_FALSE(store.store(odd, 20, 0.0, { GopCache::Keep::Store, -1 }, GopCache::Slot{ 20, 0, GopCache::Pool::Future }, 100 * est));
    src[5]->flags |= AV_FRAME_FLAG_CORRUPT;
    CHECK_FALSE(store.storable(src[5]));
    // holes stay across a clear; freeSlot returns memory
    store.markHole(30);
    CHECK(store.isHole(30));
    store.freeSlot(0);
    CHECK(store.bytes() == bytesBefore - store.frameBytes());
    CHECK(budget.bytes.load() == store.bytes());
    store.clear();
    CHECK(store.bytes() == 0);
    CHECK(store.resident() == 0);
    CHECK(budget.bytes.load() == 0);
    CHECK(budget.active.load() == 0);
    CHECK(st.gopCacheBytes.load() == 0);
    CHECK(st.gopCacheFrames.load() == 0);
    CHECK(st.gopCacheActive.load() == 0);
    CHECK(store.isHole(30));
    for (auto* f : src)
        av_frame_free(&f);
    av_frame_free(&odd);
}
