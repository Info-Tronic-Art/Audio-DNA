// test_video_decode_trace -- s-rta-0929b gopcache (plan-gopcache.md 3.5 / 7 + HARMONY ADOPTION GC12): the GOLDEN TRACE of
// the REAL VideoPlayer's forward writer. A fixed-clock schedule (GOP-30 64x64 fixture: half-frame render steps, a mid-GOP
// seek with a NONREF catch-up, full-frame steps across the Loop wrap) is played against the real decode thread (start()):
// after every clock step the test waits until the writer is quiescent (the ring is full or the writer sits at EOF: no
// counter moves for 60 ms), records every Ready frame it has not seen before (seq, request generation, frame index) and
// then picks like uploadToTexture (the same pick call, the same Retire hand-back; no GL). The trace (every published
// frame in publish order, every pick, the decode / drop / seek counters) was RECORDED ON THE PRE-REFACTOR WRITER (main
// d88d2ea: decodeLoop with its ring-full wait loop) and is the literal below: the c1b refactor (decodeLoop = an outer
// loop around decodeStep) must reproduce it exactly, threaded AND stepped without a thread.
#include <catch2/catch_test_macros.hpp>

#include "media/VideoPlayer.h"

#include <chrono>
#include <cstdlib>
#include <cmath>
#include <map>
#include <string>
#include <thread>
#include <tuple>

struct VideoPlayerTestAccess
{
    static double frameDur(const VideoPlayer& p) { return p.frameDur_; }
    static int readyCount(const VideoPlayer& p) { return p.ring_.readyCount(); }
    static int freeCount(const VideoPlayer& p) { return p.ring_.freeCount(); }

    // Every Ready slot (seq -> gen, frame index) not yet in `seen`, appended to `order` in seq order.
    static void observe(const VideoPlayer& p, std::map<uint64_t, std::pair<uint32_t, long>>& seen, std::string& order)
    {
        std::map<uint64_t, std::pair<uint32_t, long>> now;
        for (int i = 0; i < VideoPlayer::kSlots; ++i)
        {
            const auto& h = p.ring_.header(i);
            if (h.state.load(std::memory_order_acquire) != static_cast<uint8_t>(VideoRing::SlotState::Ready))
                continue;
            if (seen.count(h.seq) == 0)
                now[h.seq] = { h.gen, std::lround(h.pts / p.frameDur_) };
        }
        for (const auto& [seq, gf] : now)
        {
            seen[seq] = gf;
            order += std::to_string(gf.first) + ":" + std::to_string(gf.second) + " ";
        }
    }

    // The GL thread's pick (uploadToTexture without GL and without a budget): the frame index picked, or -1.
    static long readerPick(VideoPlayer& p)
    {
        const uint32_t g = p.gen_.load(std::memory_order_acquire);
        const auto k = p.ring_.pick(p.currentTime_, g, 0.5 * p.frameDur_, (VideoPlayer::kWriterLookAhead + 1) * p.frameDur_);
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

// No counter / ring state moves for `quietMs` (the writer waits for a slot, or sits at EOF). False after `limitMs`.
bool waitQuiescent(const VideoPlayer& p, const VideoStats& st, int quietMs = 60, int limitMs = 5000)
{
    using Clock = std::chrono::steady_clock;
    auto key = [&] {
        return std::make_tuple(st.framesDecoded.load(), st.framesDropped.load(), st.seeks.load(),
                               VideoPlayerTestAccess::readyCount(p), VideoPlayerTestAccess::freeCount(p));
    };
    const auto t0 = Clock::now();
    auto last = key();
    auto since = Clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t0).count() < limitMs)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        const auto k = key();
        if (k != last)
        {
            last = k;
            since = Clock::now();
        }
        else if (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - since).count() >= quietMs)
            return true;
    }
    return false;
}

struct Trace
{
    std::string published, picks, counters;
};

// The schedule: 40 half-frame steps (a 60 fps render of a 30 fps clip), a seek to frame 55 (keyframe 30: a 25-frame
// catch-up, NONREF skipping for its first 15 frames), 80 full-frame steps (clock 1.83 s -> the Loop wrap at 3.0 s ->
// 1.5 s). `idle` makes the writer quiescent after each clock step.
template <typename Idle>
Trace runSchedule(VideoPlayer& p, VideoStats& st, Idle idle)
{
    Trace t;
    std::map<uint64_t, std::pair<uint32_t, long>> seen;
    const double fd = VideoPlayerTestAccess::frameDur(p);
    for (int i = 0; i <= 120; ++i)
    {
        if (i == 40)
        {
            p.seekTo(55.0 / 90.0);
            p.advanceFrame(0.0);
        }
        else
            p.advanceFrame(i < 40 ? 0.5 * fd : fd);
        REQUIRE(idle());
        VideoPlayerTestAccess::observe(p, seen, t.published);
        t.picks += std::to_string(VideoPlayerTestAccess::readerPick(p)) + " ";
    }
    t.counters = "decoded " + std::to_string(st.framesDecoded.load()) + " dropped " + std::to_string(st.framesDropped.load())
                 + " seeks " + std::to_string(st.seeks.load());
    return t;
}

// RECORDED on the pre-refactor writer (main d88d2ea, s-rta-0929b c0). Never edit to make a change pass.
const char* const kGoldenPublished =
    "0:0 0:1 0:2 0:3 0:4 0:5 0:6 0:7 0:8 0:9 0:10 0:11 0:12 0:13 0:14 0:15 0:16 0:17 0:18 0:19 0:20 0:21 "
    "0:22 1:54 1:55 1:56 1:57 1:58 1:59 1:60 1:61 1:62 1:63 1:64 1:65 1:66 1:67 1:68 1:69 1:70 1:71 1:72 "
    "1:73 1:74 1:75 1:76 1:77 1:78 1:79 1:80 1:81 1:82 1:83 1:84 1:85 1:86 1:87 1:88 1:89 2:0 2:1 2:2 "
    "2:3 2:4 2:5 2:6 2:7 2:8 2:9 2:10 2:11 2:12 2:13 2:14 2:15 2:16 2:17 2:18 2:19 2:20 2:21 2:22 2:23 "
    "2:24 2:25 2:26 2:27 2:28 2:29 2:30 2:31 2:32 2:33 2:34 2:35 2:36 2:37 2:38 2:39 2:40 2:41 2:42 2:43 "
    "2:44 2:45 2:46 ";
const char* const kGoldenPicks =
    "1 -1 2 -1 -1 3 4 -1 5 -1 -1 6 -1 7 8 -1 9 -1 10 -1 11 -1 -1 12 -1 13 -1 14 -1 15 16 -1 17 -1 18 -1 "
    "19 -1 20 -1 -1 55 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 "
    "84 85 86 87 88 89 -1 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 "
    "30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 ";
const char* const kGoldenCounters = "decoded 125 dropped 18 seeks 2";
} // namespace

TEST_CASE("golden trace: the forward writer publishes exactly the recorded frames, picks and counters (threaded)",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop30_64x64.mp4");
    REQUIRE(f.existsAsFile());
    VideoStats st;
    VideoPlayer p;
    p.setStats(&st);
    REQUIRE(p.open(f));
    REQUIRE(p.getTotalFrames() == 90);
    p.start();
    const auto t = runSchedule(p, st, [&] { return waitQuiescent(p, st); });
    p.close();
    if (const char* out = std::getenv("GOLDEN_TRACE_OUT"))   // how the literals below were recorded (c0, main's writer)
        juce::File(out).replaceWithText(t.published + "\n" + t.picks + "\n" + t.counters + "\n");
    UNSCOPED_INFO("published: " << t.published);
    UNSCOPED_INFO("picks: " << t.picks);
    UNSCOPED_INFO("counters: " << t.counters);
    CHECK(t.published == kGoldenPublished);
    CHECK(t.picks == kGoldenPicks);
    CHECK(t.counters == kGoldenCounters);
}
