// test_gop_cache_store -- s-rta-0929b gopcache (plan-gopcache.md 4.1 + HARMONY ADOPTION GC1-GC10): the REAL VideoPlayer
// (src/media/VideoPlayer.cpp + FFmpeg + the GOP cache), no GL, no decode thread -- the test is the GL thread (advanceFrame +
// a pick exactly like uploadToTexture's: the same ring call, the same Retire hand-back) and drives the writer itself
// (decodeStep through VideoPlayerTestAccess, a friend of VideoPlayer). Every player uses the malloc upload path (RGBA slots:
// no IOSurface needed) and, unless a case says otherwise, a local Budget (never the process-wide one).
// Identity: every frame a reversing player publishes is compared byte for byte with the SAME frame decoded forward from the
// file start by a test-side FFmpeg decode and converted by a test-side sws with the player's own parameters (SWS_BILINEAR,
// RGBA, bottom-up) -- the cache stores native-format copies that the one convertInto turns into ring slots.
// RED on main: VideoPlayer has no decodeStep / GOP cache (this file does not compile).
// c1: a direction change is ONE generation bump; c1b: the forward writer stepped without a thread shows a Loop clip's frames
// in order; c2: GopCache::Store on its own; c3: reverse served from the cache (identity, GC1 / GC2 / GC3 / GC5 / GC6 / GC10,
// the budget, the trim); c4: forward retention (R-9 PingPong, GC8 Loop), the flips and turns served from the cache, the
// forward hits after a reverse episode (the guard: never an older frame).
#include <catch2/catch_test_macros.hpp>

#include "media/GopCacheStore.h"
#include "media/VideoPlayer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <mach/mach.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

using Bytes = std::vector<uint8_t>;

struct VideoPlayerTestAccess
{
    static void mallocPath(VideoPlayer& p) { p.forcePath_ = VideoPlayer::UploadPath::Malloc; }
    static void setBudget(VideoPlayer& p, GopCache::Budget* b) { p.cacheBudget_ = b; }   // before open()
    static uint32_t gen(const VideoPlayer& p) { return p.gen_.load(); }
    static bool reverseNow(const VideoPlayer& p) { return p.reverseNow_; }
    static bool wantReverse(const VideoPlayer& p) { return p.wantReverse_.load(); }
    static double clock(const VideoPlayer& p) { return p.currentTime_; }
    static double frameDur(const VideoPlayer& p) { return p.frameDur_; }
    static bool intraOnly(const VideoPlayer& p) { return p.intraOnly_; }
    static int resident(const VideoPlayer& p) { return p.cache_->resident(); }
    static int64_t cacheBytes(const VideoPlayer& p) { return p.cache_->bytes(); }
    static int64_t frameBytes(const VideoPlayer& p) { return p.cache_->frameBytes(); }
    static bool isResident(const VideoPlayer& p, int rel) { return p.cache_->slotOf(rel) >= 0; }
    static int servedRel(const VideoPlayer& p) { return p.haveServed_ ? p.servedRel_ : -1; }
    // gopcache-fix F1: the trajectory's indices that are neither resident nor a known hole (what a PREFETCH would chase)
    static std::string uncovered(const VideoPlayer& p)
    {
        std::string out;
        for (int r = 0; r < p.nTraj(); ++r)
            if (p.cache_->slotOf(r) < 0 && !p.cache_->isHole(r))
                out += std::to_string(r) + " ";
        return out;
    }
    static bool pending(const VideoPlayer& p) { return p.pendingPublish_; }
    static int readyCount(const VideoPlayer& p) { return p.ring_.readyCount(); }
    static void unstorable(VideoPlayer& p) { p.cache_->configure(-1, p.width_, p.height_, p.cacheBudget_, p.stats_); }
    static int backPool(const VideoPlayer& p)
    {
        int n = 0;
        for (const auto& s : p.cache_->view())
            n += (s.frame >= 0 && s.frame < p.relOf(p.currentTime_)) ? 1 : 0;
        return n;
    }
    // gopcache-fix (the GC3 keyframe gate's tooth): every RESIDENT cache frame converted like the reference (sws SWS_BILINEAR
    // to RGBA, bottom-up) and compared with ref[its relative index]; the count of those that differ (or have no reference).
    static int cacheMismatches(const VideoPlayer& p, std::map<long, Bytes>& ref, int* checked)
    {
        int bad = 0;
        *checked = 0;
        for (int i = 0; i < p.cache_->slots(); ++i)
        {
            const int rel = p.cache_->view()[static_cast<size_t>(i)].frame;
            if (rel < 0)
                continue;
            const AVFrame* fr = p.cache_->frameAt(i);
            SwsContext* sws = sws_getContext(fr->width, fr->height, static_cast<AVPixelFormat>(fr->format), fr->width,
                                             fr->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
            Bytes b(static_cast<size_t>(fr->width) * static_cast<size_t>(fr->height) * 4);
            uint8_t* dst[4] = { b.data() + static_cast<size_t>(fr->height - 1) * static_cast<size_t>(fr->width) * 4, nullptr,
                                nullptr, nullptr };
            int ds[4] = { -fr->width * 4, 0, 0, 0 };
            sws_scale(sws, fr->data, fr->linesize, 0, fr->height, dst, ds);
            sws_freeContext(sws);
            ++*checked;
            bad += (ref.count(rel) == 0 || ref[rel] != b) ? 1 : 0;
        }
        return bad;
    }
    static bool step(VideoPlayer& p) { return p.decodeStep(); }
    static int stepUntilIdle(VideoPlayer& p, int limit = 200000)
    {
        int n = 0;
        while (n < limit && p.decodeStep())
            ++n;
        return n;
    }
    // The decode loop's trim branch (R-10): the GL thread asked, the writer purges its Free slots and drops the cache.
    static void trim(VideoPlayer& p)
    {
        p.trimRequested_.store(true);
        p.serviceTrim();
    }
    // The GL thread's pick (uploadToTexture without GL / budget): the frame index picked (-1 = none); `bytes` gets the
    // picked slot's RGBA (bottom-up rows, as written); `aheadDropped` counts the frames the pick freed as too far ahead of
    // the clock (published, never shown).
    static long pick(VideoPlayer& p, Bytes* bytes = nullptr, long* aheadDropped = nullptr)
    {
        const uint32_t g = p.gen_.load();
        const auto k = p.ring_.pick(p.currentTime_, g, 0.5 * p.frameDur_, (VideoPlayer::kWriterLookAhead + 1) * p.frameDur_,
                                    p.reverseNow_);
        if (aheadDropped != nullptr)
            *aheadDropped += k.aheadDropped;
        if (k.slot < 0)
            return -1;
        if (bytes != nullptr)
        {
            const uint8_t* b = p.slotBytes_[static_cast<size_t>(k.slot)];
            bytes->assign(b, b + static_cast<size_t>(p.rowBytes_) * static_cast<size_t>(p.height_));
        }
        const int prev = p.retire_.shown(k.slot, false);
        if (prev >= 0)
        {
            p.ring_.release(prev);
            p.releasedThisFrame_ = true;
        }
        return p.relOf(k.pts);
    }
    static double lastPickedPts(const VideoPlayer& p) { return p.ring_.header(p.retire_.held).pts; }
    // s-rta-0930 gop2: the decode-time estimate pinned by an emulating harness (the EMA would learn a 64x64 file's ~0.05 ms)
    static void pinDecodeMs(VideoPlayer& p, double ms) { p.decodeMsEma_ = ms; }
    // a PREFETCH run is in its lead-in: sought, not yet decoding inside its window
    static bool inPrefetchLeadIn(const VideoPlayer& p)
    {
        return p.run_->kind == GopCache::RunKind::Prefetch && !p.run_->seekPending
               && (!p.haveDecoded_ || p.lastDecodedRel_ < p.run_->windowLo);
    }
    static int gopEst(const VideoPlayer& p) { return p.gopFramesEst_; }
    static std::vector<int> keyRels(const VideoPlayer& p) { return p.keyRels_; }
};

namespace
{
juce::File fixture(const char* name)
{
    return juce::File(TEST_FIXTURES_DIR).getChildFile(name);
}

// Every frame of `file` decoded forward from the start and converted like VideoPlayer::convertInto on the malloc path
// (sws SWS_BILINEAR to RGBA, bottom-up via a negative stride): relative index (round((pts - first) / fd)) -> bytes.
std::map<long, Bytes> forwardDecode(const juce::File& file)
{
    std::map<long, Bytes> out;
    AVFormatContext* fmt = nullptr;
    REQUIRE(avformat_open_input(&fmt, file.getFullPathName().toRawUTF8(), nullptr, nullptr) == 0);
    REQUIRE(avformat_find_stream_info(fmt, nullptr) >= 0);
    int si = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i)
        if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            si = static_cast<int>(i);
    REQUIRE(si >= 0);
    auto* st = fmt->streams[si];
    const AVCodec* codec = avcodec_find_decoder(st->codecpar->codec_id);
    AVCodecContext* cc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cc, st->codecpar);
    cc->thread_count = 2;
    REQUIRE(avcodec_open2(cc, codec, nullptr) == 0);
    const double tb = av_q2d(st->time_base);
    // the player's frame duration: avg_frame_rate, else r_frame_rate (an MPEG-TS stream has no average)
    const double fd = 1.0 / av_q2d(st->avg_frame_rate.num > 0 && st->avg_frame_rate.den > 0 ? st->avg_frame_rate : st->r_frame_rate);
    SwsContext* sws = nullptr;
    AVFrame* fr = av_frame_alloc();
    AVPacket* pk = av_packet_alloc();
    double first = -1.0;
    auto take = [&] {
        while (avcodec_receive_frame(cc, fr) == 0)
        {
            const double pts = static_cast<double>(fr->pts) * tb;
            if (first < 0.0)
                first = pts;
            if (sws == nullptr)
                sws = sws_getContext(fr->width, fr->height, static_cast<AVPixelFormat>(fr->format), fr->width, fr->height,
                                     AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
            Bytes b(static_cast<size_t>(fr->width) * static_cast<size_t>(fr->height) * 4);
            uint8_t* dst[4] = { b.data() + static_cast<size_t>(fr->height - 1) * static_cast<size_t>(fr->width) * 4, nullptr, nullptr, nullptr };
            int ds[4] = { -fr->width * 4, 0, 0, 0 };
            sws_scale(sws, fr->data, fr->linesize, 0, fr->height, dst, ds);
            out.emplace(std::lround((pts - first) / fd), std::move(b));   // two frames at one index (VFR): the first, as the cache
        }
    };
    while (av_read_frame(fmt, pk) >= 0)
    {
        if (pk->stream_index == si && avcodec_send_packet(cc, pk) >= 0)
            take();
        av_packet_unref(pk);
    }
    avcodec_send_packet(cc, nullptr);
    take();
    sws_freeContext(sws);
    av_frame_free(&fr);
    av_packet_free(&pk);
    avcodec_free_context(&cc);
    avformat_close_input(&fmt);
    return out;
}

// gopcache-fix2 R1: every frame of `file` decoded forward from the start, converted like forwardDecode, keyed by its
// POSITION in the decoder's output (presentation) order -- 0, 1, 2 ... -- never by a timestamp: the reference for a stream
// whose timestamps cannot index it (a DivX-style AVI: no pts on its keyframes and P-frames).
std::map<long, Bytes> forwardDecodeByOrder(const juce::File& file)
{
    std::map<long, Bytes> out;
    AVFormatContext* fmt = nullptr;
    REQUIRE(avformat_open_input(&fmt, file.getFullPathName().toRawUTF8(), nullptr, nullptr) == 0);
    REQUIRE(avformat_find_stream_info(fmt, nullptr) >= 0);
    const int si = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    REQUIRE(si >= 0);
    const AVCodec* codec = avcodec_find_decoder(fmt->streams[si]->codecpar->codec_id);
    AVCodecContext* cc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cc, fmt->streams[si]->codecpar);
    cc->thread_count = 2;
    REQUIRE(avcodec_open2(cc, codec, nullptr) == 0);
    SwsContext* sws = nullptr;
    AVFrame* fr = av_frame_alloc();
    AVPacket* pk = av_packet_alloc();
    long position = 0;
    auto take = [&] {
        while (avcodec_receive_frame(cc, fr) == 0)
        {
            if (sws == nullptr)
                sws = sws_getContext(fr->width, fr->height, static_cast<AVPixelFormat>(fr->format), fr->width, fr->height,
                                     AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
            Bytes b(static_cast<size_t>(fr->width) * static_cast<size_t>(fr->height) * 4);
            uint8_t* dst[4] = { b.data() + static_cast<size_t>(fr->height - 1) * static_cast<size_t>(fr->width) * 4, nullptr, nullptr, nullptr };
            int ds[4] = { -fr->width * 4, 0, 0, 0 };
            sws_scale(sws, fr->data, fr->linesize, 0, fr->height, dst, ds);
            out.emplace(position++, std::move(b));
        }
    };
    while (av_read_frame(fmt, pk) >= 0)
    {
        if (pk->stream_index == si && avcodec_send_packet(cc, pk) >= 0)
            take();
        av_packet_unref(pk);
    }
    avcodec_send_packet(cc, nullptr);
    take();
    sws_freeContext(sws);
    av_frame_free(&fr);
    av_packet_free(&pk);
    avcodec_free_context(&cc);
    avformat_close_input(&fmt);
    return out;
}

void big(GopCache::Budget& b)
{
    b.total.store(int64_t{ 1 } << 30);
}

// A reversing player driven like the app: every render frame (dt) the clock advances, the writer runs until it has nothing
// to do (or not at all while `stalled`), the reader picks; late = a drawn frame whose clock had passed the shown frame by
// more than 1.5 frames (VideoRing::judge's rule).
struct Show
{
    VideoPlayer& p;
    std::vector<long> shown;   // every NEW frame picked, in order
    long late = 0, frames = 0;
    std::map<long, Bytes>* check = nullptr;   // identity reference (nullptr = none)
    long mismatches = 0;
    long aheadDropped = 0;                    // published frames the pick freed unshown (too far ahead of the clock)

    void frame(double dt, bool stalled = false)
    {
        p.advanceFrame(dt);
        if (!stalled)
            VideoPlayerTestAccess::stepUntilIdle(p);
        Bytes b;
        const long k = VideoPlayerTestAccess::pick(p, check != nullptr ? &b : nullptr, &aheadDropped);
        ++frames;
        if (k >= 0)
        {
            shown.push_back(k);
            if (check != nullptr && (check->count(k) == 0 || (*check)[k] != b))
                ++mismatches;
        }
        else if (!shown.empty())
        {
            const double clockNow = VideoPlayerTestAccess::clock(p);
            const double fd = VideoPlayerTestAccess::frameDur(p);
            if (std::fabs(clockNow - VideoPlayerTestAccess::lastPickedPts(p)) > 1.5 * fd)
                ++late;
        }
    }
};

// The player at frame `rel` reversing: seek there (one generation), then flip the direction (the next one).
void startReverseAt(VideoPlayer& p, double seekNorm)
{
    p.seekTo(seekNorm);
    p.advanceFrame(0.0);
    p.setReverse(true);
    p.advanceFrame(1.0 / 1000.0);
}

int64_t physFootprint()
{
    task_vm_info_data_t info{};
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS)
        return -1;
    return static_cast<int64_t>(info.phys_footprint);
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
    p.advanceFrame(dt);                                   // the flip: one bump
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);
    CHECK(VideoPlayerTestAccess::reverseNow(p));
    CHECK(VideoPlayerTestAccess::wantReverse(p));         // posted with the generation
    p.advanceFrame(dt);
    CHECK(VideoPlayerTestAccess::gen(p) == g0 + 1);      // reverse stays reverse (no wrap yet: 2 frames of clock left)
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

TEST_CASE("a PingPong reflection flips the direction with ONE generation bump on the frame of the turn",
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

TEST_CASE("reverse from the cache: frames 45, 44, 43 ... in order, one DEMAND seek + one PREFETCH seek, every frame "
          "byte-identical to a forward decode", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    auto ref = forwardDecode(f);
    REQUIRE(ref.size() == 60);
    REQUIRE(ref[44] != ref[45]);                            // a sanity tooth: neighbouring frames differ
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    startReverseAt(p, 45.0 / 60.0);
    const auto seeks0 = st.seeks.load();
    Show s{ p };
    s.check = &ref;
    for (int i = 0; i < 4 * 30; ++i)    // 1 s of reverse at a 120 Hz render
        s.frame(1.0 / 120.0);
    REQUIRE(s.shown.size() >= 29);
    CHECK(s.shown[0] == 45);
    CHECK(s.shown[1] == 44);
    CHECK(s.shown[2] == 43);
    bool ordered = true;
    for (size_t i = 1; i < s.shown.size(); ++i)
        ordered = ordered && s.shown[i] == s.shown[i - 1] - 1;
    CHECK(ordered);
    CHECK(s.mismatches == 0);
    CHECK(s.late == 0);
    // the DEMAND run (keyframe 40), one PREFETCH (keyframe 0: frames 0..39), and the PREFETCH of the Loop wrap's future
    // (frames 46..59, from keyframe 40) -- then everything is resident
    CHECK(st.seeks.load() - seeks0 == 3);
    for (int k = 0; k < 60; ++k)
        CHECK(VideoPlayerTestAccess::isResident(p, k));
    CHECK(st.gopCacheHits.load() >= 28);
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

TEST_CASE("GC1: a DEMAND frame waiting for a ring slot is published with ITS pixels after the run decoded other frames",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    auto ref = forwardDecode(f);
    GopCache::Budget budget;
    big(budget);
    VideoStats st;
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    VideoPlayerTestAccess::unstorable(p);   // nothing can be stored: every served frame goes straight from the decoder
    startReverseAt(p, 45.0 / 60.0);
    VideoPlayerTestAccess::pick(p);          // the reader drops open()'s frame 0 (another generation): 3 slots for the writer
    // the reader never picks: the writer publishes 45, 44, 43 (the ring is full), then decodes 42 with a DEMAND run: pending
    for (int i = 0; i < 500 && !(VideoPlayerTestAccess::readyCount(p) == 3 && VideoPlayerTestAccess::pending(p)); ++i)
        VideoPlayerTestAccess::step(p);
    REQUIRE(VideoPlayerTestAccess::readyCount(p) == 3);
    REQUIRE(VideoPlayerTestAccess::pending(p));
    const auto decoded0 = st.framesDecoded.load();
    for (int i = 0; i < 3; ++i)             // idle work: three more decodes (a PREFETCH run) while 42 waits
        VideoPlayerTestAccess::step(p);
    CHECK(st.framesDecoded.load() - decoded0 >= 3);
    // the reader frees a slot (the clock at frame 44: 44 is picked, 45 above it is passed over)
    for (int i = 0; i < 40 && VideoPlayerTestAccess::clock(p) > 44.0 / 30.0; ++i)
        p.advanceFrame(1.0 / 120.0);
    Bytes b;
    REQUIRE(VideoPlayerTestAccess::pick(p, &b) == 44);
    REQUIRE(VideoPlayerTestAccess::step(p));
    CHECK_FALSE(VideoPlayerTestAccess::pending(p));
    // walk the clock down to 42: its bytes are frame 42's (not the last frame the prefetch decoded)
    long got = -1;
    std::string picks;
    for (int i = 0; i < 80 && got != 42; ++i)
    {
        p.advanceFrame(1.0 / 120.0);
        Bytes bb;
        const long k = VideoPlayerTestAccess::pick(p, &bb);
        if (k >= 0)
            picks += std::to_string(k) + " ";
        if (k == 42)
        {
            got = k;
            b = bb;
        }
    }
    CAPTURE(picks, VideoPlayerTestAccess::readyCount(p), VideoPlayerTestAccess::clock(p));
    REQUIRE(got == 42);
    CHECK(b == ref[42]);
    p.close();
}

TEST_CASE("the budget: a cache capped at its floor (8 frames) never holds more, never evicts the served frame, and costs "
          "<= G / (2 x floor(share / 2)) + 1 decodes per served frame (+ the first DEMAND run)", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    auto ref = forwardDecode(f);
    VideoStats st;
    GopCache::Budget budget;
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    budget.total.store(1);   // nothing but the floor (kMinFrames) -- a cache under any other budget share
    startReverseAt(p, 50.0 / 60.0);
    Show s{ p };
    s.check = &ref;
    const auto dec0 = st.framesDecoded.load();
    int maxResident = 0;
    while (s.shown.size() < 31 && s.frames < 2000)
    {
        s.frame(1.0 / 120.0);
        maxResident = std::max(maxResident, VideoPlayerTestAccess::resident(p));
    }
    REQUIRE(s.shown.size() >= 31);
    CHECK(maxResident <= GopCache::kMinFrames);
    CHECK(s.mismatches == 0);
    const double perFrame = static_cast<double>(st.framesDecoded.load() - dec0) / static_cast<double>(s.shown.size());
    const int share = GopCache::kMinFrames - GopCache::behindCapFor(GopCache::Mode::Loop, false, GopCache::kMinFrames, 0);
    CAPTURE(perFrame, share, st.gopCacheEvictions.load());
    // the pure policy's steady-state bound (test_gop_cache's amortization case) + the first DEMAND run (one GOP of decode)
    // spread over the frames served here
    CHECK(perFrame <= 10.0 / (2.0 * (share / 2)) + 1.0 + 10.0 / static_cast<double>(s.shown.size()));
    CHECK(st.gopCacheEvictions.load() > 0);
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

TEST_CASE("GC5: intra-only is the codec's or the container index's verdict at open -- never a cache, never a prefetch",
          "[video_player][gopcache][s-rta-0929b]")
{
    for (const char* name : { "video_rawrgba_63x37.mov", "video_hapa_64x64.mov", "video_h264_allintra_64x64.mp4" })
    {
        CAPTURE(name);
        VideoStats st;
        GopCache::Budget budget;
    big(budget);
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        p.setStats(&st);
        REQUIRE(p.open(fixture(name)));
        CHECK(VideoPlayerTestAccess::intraOnly(p));
        p.setReverse(true);
        p.advanceFrame(1.0 / 1000.0);
        Show s{ p };
        for (int i = 0; i < 60; ++i)
            s.frame(1.0 / 120.0);
        CHECK(s.shown.size() >= 10);
        CHECK(VideoPlayerTestAccess::resident(p) == 0);
        CHECK(st.gopCacheRuns.load() == 0);                 // no PREFETCH
        CHECK(VideoPlayerTestAccess::cacheBytes(p) == 0);
        p.close();
    }
    VideoPlayer q;
    REQUIRE(q.open(fixture("video_h264_gop10_64x64.mp4")));
    CHECK_FALSE(VideoPlayerTestAccess::intraOnly(q));
    q.close();
}

TEST_CASE("R-10: the idle trim clears the cache -- nothing resident, no bytes, the budget inactive",
          "[video_player][gopcache][s-rta-0929b]")
{
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(fixture("video_h264_gop10_64x64.mp4")));
    startReverseAt(p, 45.0 / 60.0);
    Show s{ p };
    for (int i = 0; i < 20; ++i)
        s.frame(1.0 / 120.0);
    REQUIRE(VideoPlayerTestAccess::resident(p) > 8);
    REQUIRE(budget.active.load() == 1);
    REQUIRE(budget.bytes.load() > 0);
    VideoPlayerTestAccess::trim(p);
    CHECK(VideoPlayerTestAccess::resident(p) == 0);
    CHECK(VideoPlayerTestAccess::cacheBytes(p) == 0);
    CHECK(budget.bytes.load() == 0);
    CHECK(budget.active.load() == 0);
    CHECK(st.gopCacheBytes.load() == 0);
    CHECK(st.gopCacheFrames.load() == 0);
    CHECK(st.gopCacheActive.load() == 0);
    p.close();
}

TEST_CASE("GC2: reverse at 2x and 4x keeps up with the clock (never frozen, late <= 3); after a 300 ms writer stall it jumps "
          "to the clock", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    for (float speed : { 2.0f, 4.0f })
    {
        CAPTURE(speed);
        VideoStats st;
        GopCache::Budget budget;
    big(budget);
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        p.setStats(&st);
        REQUIRE(p.open(f));
        p.setSpeed(speed);
        startReverseAt(p, 58.0 / 60.0);
        Show s{ p };
        for (int i = 0; i < 90; ++i)   // 0.75 s: 45 (2x) / 90 (4x) frames of content -> never a wrap at 2x
            s.frame(1.0 / 120.0);
        CHECK(s.late <= 3);
        bool monotone = true;
        for (size_t i = 1; i < s.shown.size(); ++i)
            monotone = monotone && s.shown[i] < s.shown[i - 1];
        if (speed == 2.0f)
        {
            CHECK(monotone);
            CHECK(s.shown.size() >= 40);
        }
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
    // the stall: 36 render frames (300 ms) with no writer step, then the writer runs again
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    startReverseAt(p, 58.0 / 60.0);
    Show s{ p };
    for (int i = 0; i < 24; ++i)
        s.frame(1.0 / 120.0);
    for (int i = 0; i < 36; ++i)
        s.frame(1.0 / 120.0, true);
    const long lateAtResume = s.late;
    const size_t shownAtResume = s.shown.size();
    for (int i = 0; i < 24; ++i)
        s.frame(1.0 / 120.0);
    CHECK(s.late - lateAtResume <= 3);                                 // back on the clock within a few frames
    REQUIRE(s.shown.size() > shownAtResume);
    const long clockRel = std::lround(VideoPlayerTestAccess::clock(p) / VideoPlayerTestAccess::frameDur(p));
    CHECK(std::labs(s.shown.back() - clockRel) <= 1);                  // the shown frame is the clock's
    CHECK(st.framesDropped.load() > 0);                                // the frames the clock passed were skipped
    p.close();
}

TEST_CASE("GC3: open-GOP, a non-zero start time and a VFR file -- every frame a full reverse lap serves equals a forward "
          "decode; no seek storm", "[video_player][gopcache][s-rta-0929b]")
{
    for (const char* name : { "video_h264_opengop_64x64.mp4", "video_h264_start1s_64x64.mp4", "video_h264_vfr_64x64.mp4" })
    {
        CAPTURE(name);
        const auto f = fixture(name);
        auto ref = forwardDecode(f);
        REQUIRE(ref.size() == 60);
        VideoStats st;
        GopCache::Budget budget;
    big(budget);
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        p.setStats(&st);
        REQUIRE(p.open(f));
        p.setReverse(true);
        p.advanceFrame(1.0 / 1000.0);   // the Loop wrap to the end, reversing
        Show s{ p };
        s.check = &ref;
        for (int i = 0; i < 4 * 55; ++i)
            s.frame(1.0 / 120.0);
        CHECK(s.shown.size() >= 20);
        CHECK(s.mismatches == 0);
        CHECK(st.seeks.load() <= 12);   // a DEMAND + a few PREFETCH runs, never one seek per frame
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
}

// gopcache-fix F1 (GC3 "every run has a negative result", for EVERY run kind): a file whose frame index has holes -- a VFR
// file with dropped frames (the relative index is pts / the AVERAGE frame duration: some indices are never output) and an
// MPEG-4 part 2 MP4 with B-frames -- reversed in Loop for three laps with the whole file in budget. After the first lap
// everything the decoder can output is resident and every index it never outputs is a known hole: laps 2 and 3 cost no
// seek and no run (RED on a02c93c: a PREFETCH run that passed an un-fetchable index marked nothing, so the index stayed the
// prefetch target -- one seek + catch-up per served frame, forever). The h264 GOP-10 file is the control.
TEST_CASE("GC3 holes: a reverse Loop over a file with index holes (VFR drops, MPEG-4 B-frames) stops seeking after one lap",
          "[video_player][gopcache][s-rta-0929b]")
{
    for (const char* name : { "video_h264_vfrgap_64x64.mp4", "video_mpeg4_bf2_64x64.mp4", "video_h264_gop10_64x64.mp4" })
    {
        CAPTURE(name);
        auto ref = forwardDecode(fixture(name));   // the indices a forward decode outputs: none may become a hole
        VideoStats st;
        GopCache::Budget budget;
        big(budget);
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        p.setStats(&st);
        REQUIRE(p.open(fixture(name)));
        p.setReverse(true);
        p.advanceFrame(1.0 / 1000.0);   // the Loop wrap to the end, reversing
        Show s{ p };
        s.check = &ref;
        const int lap = static_cast<int>(std::ceil(p.getDuration() * 120.0));
        for (int i = 0; i < lap + 60; ++i)   // one lap (+ half a second: the wrap's own window)
            s.frame(1.0 / 120.0);
        const auto seeks1 = st.seeks.load(), runs1 = st.gopCacheRuns.load(), misses1 = st.gopCacheMisses.load();
        const size_t shown1 = s.shown.size();
        const std::string uncovered1 = VideoPlayerTestAccess::uncovered(p);
        // no false hole: every index a forward decode outputs is resident after the lap (an MPEG-4 file's frame 118 -- a
        // B-frame decoded after keyframe 119 -- is reached by seeking further back, never written off as a hole)
        std::string missing;
        for (const auto& [rel, bytes] : ref)
            if (!VideoPlayerTestAccess::isResident(p, static_cast<int>(rel)))
                missing += std::to_string(rel) + " ";
        CAPTURE(missing);
        CHECK(missing.empty());
        for (int i = 0; i < 2 * lap; ++i)
            s.frame(1.0 / 120.0);
        CAPTURE(uncovered1, VideoPlayerTestAccess::uncovered(p));
        CAPTURE(seeks1, runs1, misses1, st.seeks.load(), st.gopCacheRuns.load(), st.gopCacheMisses.load(),
                s.shown.size() - shown1, st.framesDecoded.load(), s.late);
        // never frozen (how many of the VFR file's frames a lap shows is the R2 case's gate, not this test's)
        CHECK(s.shown.size() - shown1 >= 40);
        CHECK(st.seeks.load() - seeks1 <= 1);
        CHECK(st.gopCacheRuns.load() - runs1 <= 1);
        CHECK(st.gopCacheMisses.load() - misses1 <= 1);
        CHECK(s.mismatches == 0);
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
}

// gopcache-fix2 R1: a DivX-style AVI (MPEG-4 part 2, 2 B-frames: every keyframe and P-frame has NO pts) reversed in Loop
// with the whole file in budget. A pts-less frame is indexed by its own best-effort time (the same from the file start and
// after any seek), else by the previous output's index + 1 -- never given the run's target index. Every frame SHOWN and
// every frame RESIDENT equals the forward decode's frame at the same position from the file start (output order: this
// file's timestamps cannot index it), all 120 frames become resident, and laps 2-3 cost no seek. RED on 98994c6: a run's
// pts-less first output (its keyframe) was stored at the run's TARGET index and ended the run -- reverse showed keyframe
// pictures (one seek + one decode per frame).
TEST_CASE("R1: a pts-less AVI reversed in Loop -- every frame shown and cached is the forward decode's frame at that "
          "position; laps 2-3 cost no seek", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_mpeg4_bf2_64x64.avi");
    auto ref = forwardDecodeByOrder(f);
    REQUIRE(ref.size() == 120);
    REQUIRE(ref[29] != ref[30]);   // a sanity tooth: the keyframe differs from the frame before it
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    p.setReverse(true);
    p.advanceFrame(1.0 / 1000.0);   // the Loop wrap to the end, reversing
    Show s{ p };
    s.check = &ref;
    const int lap = static_cast<int>(std::ceil(p.getDuration() * 120.0));
    for (int i = 0; i < lap + 60; ++i)   // one lap (+ half a second: the wrap's own window)
        s.frame(1.0 / 120.0);
    const auto seeks1 = st.seeks.load(), runs1 = st.gopCacheRuns.load(), misses1 = st.gopCacheMisses.load();
    const auto decoded1 = st.framesDecoded.load();
    const size_t shown1 = s.shown.size();
    std::string missing;
    for (int k = 0; k < 120; ++k)
        if (!VideoPlayerTestAccess::isResident(p, k))
            missing += std::to_string(k) + " ";
    CAPTURE(missing, seeks1, runs1, misses1, decoded1, shown1);
    CHECK(missing.empty());
    int checked = 0;
    const int badResident = VideoPlayerTestAccess::cacheMismatches(p, ref, &checked);
    CHECK(checked == 120);
    CHECK(badResident == 0);
    for (int i = 0; i < 2 * lap; ++i)
        s.frame(1.0 / 120.0);
    CAPTURE(st.seeks.load(), st.gopCacheRuns.load(), st.gopCacheMisses.load(), st.framesDecoded.load(), s.shown.size(), s.late);
    CHECK(s.shown.size() - shown1 >= 2 * 110);   // ~120 frames a lap: every frame of the file, in reverse
    CHECK(s.mismatches == 0);
    CHECK(st.seeks.load() - seeks1 <= 1);
    CHECK(st.gopCacheRuns.load() - runs1 <= 1);
    CHECK(st.gopCacheMisses.load() - misses1 <= 1);
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

// gopcache-fix2 R2: a VFR file (a 30 fps capture with ~30 % of its frames dropped: 85 frames, an AVERAGE frame duration of
// 47 ms over a 33 ms grid) reversed in Loop with the whole file in budget, three laps. The reverse writer publishes a frame
// no further below the clock than kWriterLookAhead + 1 frame durations measured on the frame's OWN pts -- inside the line
// the pick frees frames at. 98994c6 measured it in relative indices: a frame of index wantRel - 3 whose pts sits early in
// its 47 ms bucket was up to ~4 frame durations below the clock, freed by the pick unshown (28 of the 63 frames published a
// lap), and the writer never came back for it that lap. Every lap: no published frame is freed unshown, and the lap shows
// at least 0.9 x main's median of distinct frames a lap (main d88d2ea, threaded reverse Loop of this file, 5 runs x 3
// laps: median 42 -> >= 38). RED on 98994c6: 35 distinct a lap, 28 freed unshown.
TEST_CASE("R2: a VFR file reversed in Loop -- no published frame is freed unshown; each lap shows >= 0.9 x main's distinct "
          "frames", "[video_player][gopcache][s-rta-0929b]")
{
    constexpr long kMainMedianPerLap = 42;   // main d88d2ea, threaded, 5 runs x 3 laps (lane report gopcache.md, fix round 2)
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(fixture("video_h264_vfrgap_64x64.mp4")));
    p.setReverse(true);
    p.advanceFrame(1.0 / 1000.0);   // the Loop wrap to the end, reversing
    Show s{ p };
    const int lap = static_cast<int>(std::lround(p.getDuration() * 120.0));
    for (int l = 0; l < 3; ++l)
    {
        const size_t from = s.shown.size();
        const long ahead0 = s.aheadDropped;
        for (int i = 0; i < lap; ++i)
            s.frame(1.0 / 120.0);
        const std::set<long> distinct(s.shown.begin() + static_cast<std::ptrdiff_t>(from), s.shown.end());
        CAPTURE(l, distinct.size(), s.aheadDropped - ahead0, VideoPlayerTestAccess::resident(p), st.seeks.load());
        CHECK(s.aheadDropped - ahead0 == 0);
        CHECK(static_cast<long>(distinct.size()) * 10 >= kMainMedianPerLap * 9);
    }
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

// gopcache-fix (the GC3 keyframe gate's tooth): an MPEG-TS file has no keyframe index -- the demuxer seeks by timestamp and
// lands on the frame asked for, a NON-keyframe: the MPEG-4 part 2 decoder then outputs every P-frame after it, predicted
// from pictures it never saw (key 0, not flagged corrupt). A run stores only frames decoded after its keyframe (runSawKey_),
// so every frame RESIDENT in the cache equals a forward decode. Tooth: without the gate the cache holds the garbage frames.
// (What a DEMAND run PUBLISHES before its keyframe is the decoder's output as-is -- main's forward seek on this file shows
// the same frames: outside this gate.)
TEST_CASE("GC3 keyframe gate: an MPEG-TS seek lands on a non-keyframe -- no frame decoded before the run's keyframe is stored",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_mpeg4_64x64.ts");
    auto ref = forwardDecode(f);
    REQUIRE(ref.size() == 60);
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    startReverseAt(p, 45.0 / 60.0);
    Show s{ p };
    for (int i = 0; i < 4 * 90; ++i)
        s.frame(1.0 / 120.0);
    int checked = 0;
    const int bad = VideoPlayerTestAccess::cacheMismatches(p, ref, &checked);
    CAPTURE(checked, s.shown.size(), st.seeks.load(), st.gopCacheRuns.load(), st.gopCacheMisses.load());
    CHECK(checked >= 20);   // the runs stored frames (from keyframe 30 up): the gate was exercised, not bypassed
    CHECK(bad == 0);
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

TEST_CASE("GC6: a resident cache shrinks to its share when a second cache joins the budget (per step, not at a store)",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    GopCache::Budget budget;
    VideoPlayer a, b;
    for (VideoPlayer* p : { &a, &b })
    {
        VideoPlayerTestAccess::mallocPath(*p);
        VideoPlayerTestAccess::setBudget(*p, &budget);
        REQUIRE(p->open(f));
    }
    budget.total.store(40 * VideoPlayerTestAccess::frameBytes(a));   // room for 40 frames together
    startReverseAt(a, 59.0 / 60.0);
    Show sa{ a };
    for (int i = 0; i < 60 && VideoPlayerTestAccess::resident(a) <= 24; ++i)
        sa.frame(1.0 / 120.0);
    REQUIRE(VideoPlayerTestAccess::resident(a) > 24);                // A alone: most of the budget
    startReverseAt(b, 59.0 / 60.0);
    Show sb{ b };
    for (int i = 0; i < 30; ++i)
    {
        sb.frame(1.0 / 120.0);
        sa.frame(1.0 / 120.0);
    }
    const int64_t share = budget.capBytes();
    CHECK(budget.active.load() == 2);
    CHECK(VideoPlayerTestAccess::cacheBytes(a) <= std::max(share, 8 * VideoPlayerTestAccess::frameBytes(a)));
    CHECK(budget.bytes.load() <= budget.total.load() + 16 * VideoPlayerTestAccess::frameBytes(a));
    CHECK(sb.shown.size() >= 7);
    a.close();
    b.close();
}

TEST_CASE("GC10: the cache's memory really returns -- 100 frames filled, cleared, 20 players opened / filled / closed: the "
          "physical footprint ends within 15 % of the filled peak's growth", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop60_640x360.mp4");   // 345,600 B a frame: 100 frames = 34.6 MB
    GopCache::Budget budget;
    big(budget);
    {   // warm-up: FFmpeg / sws / JUCE one-time allocations stay with the process -- not the cache's memory
        VideoPlayer w;
        VideoPlayerTestAccess::mallocPath(w);
        VideoPlayerTestAccess::setBudget(w, &budget);
        REQUIRE(w.open(f));
        startReverseAt(w, 119.0 / 120.0);
        Show s{ w };
        for (int i = 0; i < 20; ++i)
            s.frame(1.0 / 120.0);
        w.close();
    }
    const int64_t base = physFootprint();
    REQUIRE(base > 0);
    int64_t peak = base;
    {
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        REQUIRE(p.open(f));
        startReverseAt(p, 119.0 / 120.0);
        Show s{ p };
        for (int i = 0; i < 400 && VideoPlayerTestAccess::resident(p) < 100; ++i)
            s.frame(1.0 / 120.0);
        REQUIRE(VideoPlayerTestAccess::resident(p) >= 100);
        peak = physFootprint();
        VideoPlayerTestAccess::trim(p);
        p.close();
    }
    for (int k = 0; k < 20; ++k)
    {
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        REQUIRE(p.open(f));
        startReverseAt(p, 119.0 / 120.0);
        Show s{ p };
        for (int i = 0; i < 20; ++i)
            s.frame(1.0 / 120.0);
        p.close();
    }
    const int64_t end = physFootprint();
    CAPTURE(base, peak, end);
    CHECK(peak - base > 25'000'000);
    CHECK(end - base < (peak - base) * 15 / 100);
    CHECK(budget.bytes.load() == 0);
    CHECK(budget.active.load() == 0);
}

TEST_CASE("S8: a 640x360 GOP-60 clip in a small cache (24 frames) costs a bounded number of decodes per served frame",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop60_640x360.mp4");
    VideoStats st;
    GopCache::Budget budget;
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    budget.total.store(24 * VideoPlayerTestAccess::frameBytes(p));
    startReverseAt(p, 119.0 / 120.0);
    Show s{ p };
    const auto dec0 = st.framesDecoded.load();
    for (int i = 0; i < 4 * 60; ++i)
        s.frame(1.0 / 120.0);
    REQUIRE(s.shown.size() >= 55);
    const double perFrame = static_cast<double>(st.framesDecoded.load() - dec0) / static_cast<double>(s.shown.size());
    const int share = 24 - GopCache::behindCapFor(GopCache::Mode::Loop, false, 24, 0);
    CAPTURE(perFrame, share);
    CHECK(perFrame <= 60.0 / (2.0 * (share / 2)) + 1.0 + 60.0 / static_cast<double>(s.shown.size()));   // + the first DEMAND
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

// gopcache-fix (GC10's tooth): the footprint case above cannot tell an mmap-backed slot from a malloc-backed one (345 KB frames:
// a $TMPDIR mutant with av_frame_get_buffer returned its memory there too -- only the live 1080p u9 row saw malloc keep
// 870 MB). This asserts the mechanism itself: every slot's planes are ONE page-aligned anonymous mapping of whole pages,
// and a freed slot's pages are unmapped at once (msync on them fails with ENOMEM) -- a malloc'd block stays mapped.
TEST_CASE("GC10: a cache slot is its own page mapping, unmapped the moment the slot is freed (clear / freeSlot)",
          "[gopcache][s-rta-0929b]")
{
    GopCache::Budget budget;
    big(budget);
    GopCache::Store store;
    store.configure(AV_PIX_FMT_YUV420P, 64, 64, &budget, nullptr);
    std::vector<AVFrame*> src;
    for (int i = 0; i < 4; ++i)
        src.push_back(patternFrame(64, 64, i));
    const size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    std::vector<std::pair<uint8_t*, size_t>> maps;
    for (int i = 0; i < 4; ++i)
    {
        REQUIRE(store.store(src[static_cast<size_t>(i)], i, i / 30.0, { GopCache::Keep::Store, -1 },
                            GopCache::Slot{ i, i, GopCache::Pool::Future }, 0));
        const AVFrame* f = store.frameAt(store.slotOf(i));
        REQUIRE(f->buf[0] != nullptr);
        CHECK(f->buf[1] == nullptr);                                            // all planes in one buffer
        CHECK(reinterpret_cast<uintptr_t>(f->buf[0]->data) % page == 0);       // page-aligned
        CHECK(f->buf[0]->size % page == 0);                                     // whole pages
        CHECK(static_cast<int64_t>(f->buf[0]->size) == store.frameBytes());
        maps.emplace_back(f->buf[0]->data, f->buf[0]->size);
        CHECK(msync(f->buf[0]->data, f->buf[0]->size, MS_ASYNC) == 0);          // mapped while held
    }
    store.freeSlot(store.slotOf(0));
    errno = 0;
    CHECK(msync(maps[0].first, maps[0].second, MS_ASYNC) == -1);
    CHECK(errno == ENOMEM);                                                     // the freed slot's pages are gone
    CHECK(msync(maps[1].first, maps[1].second, MS_ASYNC) == 0);                 // the others stay
    store.clear();
    for (size_t i = 1; i < maps.size(); ++i)
    {
        errno = 0;
        CHECK(msync(maps[i].first, maps[i].second, MS_ASYNC) == -1);
        CHECK(errno == ENOMEM);
    }
    for (auto* f : src)
        av_frame_free(&f);
}

namespace
{
// Forward play of a fresh player from frame 0 for `frames` content frames at a 120 Hz render; returns what was shown.
std::vector<long> playForward(VideoPlayer& p, Show& s, int contentFrames)
{
    for (int i = 0; i < 4 * contentFrames; ++i)
        s.frame(1.0 / 120.0);
    return s.shown;
}
} // namespace

TEST_CASE("c4: forward retention -- PingPong keeps its retainFrames (R-9), Loop its last 16 (GC8) behind the clock; never more",
          "[video_player][gopcache][s-rta-0929b]")
{
    for (auto mode : { VideoPlayer::LoopMode::PingPong, VideoPlayer::LoopMode::Loop, VideoPlayer::LoopMode::OneShot })
    {
        CAPTURE(static_cast<int>(mode));
        VideoStats st;
        GopCache::Budget budget;
        big(budget);
        VideoPlayer p;
        VideoPlayerTestAccess::mallocPath(p);
        VideoPlayerTestAccess::setBudget(p, &budget);
        p.setStats(&st);
        REQUIRE(p.open(fixture("video_h264_gop10_64x64.mp4")));
        p.setLoopMode(mode);
        Show s{ p };
        playForward(p, s, 40);
        REQUIRE(s.shown.size() >= 38);
        // 64x64 GOP 10 decodes in microseconds: R-9's GOP x decode ms / frame ms x 2 is under the floor -> 16 either way
        CHECK(VideoPlayerTestAccess::backPool(p) == GopCache::kBehindFrames);
        CHECK(VideoPlayerTestAccess::resident(p) <= GopCache::kBehindFrames + 3 + 1);   // + the writer's look-ahead
        CHECK(st.seeks.load() == 0);          // retention never touches the decoder
        p.close();
    }
}

TEST_CASE("c4: GC8 -- a forward Loop clip flipped to reverse is served at once from its retained frames (hits, no hold), "
          "then from the runs; every frame identical to a forward decode", "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");
    auto ref = forwardDecode(f);
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    Show s{ p };
    s.check = &ref;
    playForward(p, s, 35);                              // forward to ~frame 35
    const size_t atFlip = s.shown.size();
    const long lateAtFlip = s.late;
    const auto hits0 = st.gopCacheHits.load();
    p.setReverse(true);
    for (int i = 0; i < 4 * 20; ++i)                    // 20 frames of reverse
        s.frame(1.0 / 120.0);
    REQUIRE(s.shown.size() >= atFlip + 18);
    CHECK(s.late - lateAtFlip <= 1);                    // the flip is not a hold
    CHECK(st.gopCacheHits.load() - hits0 >= 15);        // the first reverse frames came from the retained ones
    bool down = true;
    for (size_t i = atFlip + 1; i < s.shown.size(); ++i)
        down = down && s.shown[i] < s.shown[i - 1];
    CHECK(down);
    CHECK(s.mismatches == 0);
    CHECK(st.reverseNonmonotonic.load() == 0);
    p.close();
}

TEST_CASE("c4: the PingPong top turn is served from the retained frames; the bottom turn from the window; forward hits "
          "after a manual reverse -> forward flip never show an older frame (the guard); identity throughout",
          "[video_player][gopcache][s-rta-0929b]")
{
    const auto f = fixture("video_h264_gop10_64x64.mp4");   // 60 frames, 2 s
    auto ref = forwardDecode(f);
    VideoStats st;
    GopCache::Budget budget;
    big(budget);
    VideoPlayer p;
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    p.setLoopMode(VideoPlayer::LoopMode::PingPong);
    Show s{ p };
    s.check = &ref;
    // 2 s up, the top turn, 2 s down, the bottom turn, 0.5 s up
    for (int i = 0; i < 4 * 30 * 4 + 60; ++i)
        s.frame(1.0 / 120.0);
    CHECK(s.mismatches == 0);
    CHECK(s.late <= 4);                                 // <= 2 per turn
    // the turns: the sequence rises to the top, falls to the bottom, rises again (the turn frame is shown again in the new
    // generation -- one repeated upload of the same picture per turn: collapsed here)
    std::vector<long> v;
    for (long k : s.shown)
        if (v.empty() || v.back() != k)
            v.push_back(k);
    size_t top = 0;
    for (size_t i = 1; i < v.size(); ++i)
        if (v[i] > v[top])
            top = i;
    CHECK(v[top] >= 58);
    bool up = true, down = true;
    for (size_t i = 1; i <= top; ++i)
        up = up && v[i] > v[i - 1];
    size_t bottom = top;
    for (size_t i = top + 1; i < v.size() && v[i] < v[i - 1]; ++i)
        bottom = i;
    for (size_t i = top + 1; i <= bottom; ++i)
        down = down && v[i] < v[i - 1];
    CHECK(up);
    CHECK(down);
    CHECK(v[bottom] <= 1);
    CHECK(v.size() > bottom + 5);
    for (size_t i = bottom + 1; i < v.size(); ++i)
        CHECK(v[i] > v[i - 1]);                       // after the bottom turn: forward, never an older frame
    CHECK(st.reverseNonmonotonic.load() == 0);
    // a manual flip mid-file: reverse 10 frames, then forward again -- the forward hits and the decoder's catch-up never
    // publish a frame at or below the newest one
    p.setLoopMode(VideoPlayer::LoopMode::Loop);
    p.seekTo(40.0 / 60.0);
    p.advanceFrame(0.0);
    p.setReverse(true);
    Show t{ p };
    t.check = &ref;
    for (int i = 0; i < 40; ++i)
        t.frame(1.0 / 120.0);
    const size_t atFlip = t.shown.size();
    p.setReverse(false);
    for (int i = 0; i < 120; ++i)
        t.frame(1.0 / 120.0);
    REQUIRE(t.shown.size() >= atFlip + 25);
    for (size_t i = atFlip + 1; i < t.shown.size(); ++i)
        CHECK(t.shown[i] >= t.shown[i - 1]);          // the flip frame may be shown again (a new generation), never older
    size_t repeats = 0;
    for (size_t i = atFlip + 1; i < t.shown.size(); ++i)
        repeats += t.shown[i] == t.shown[i - 1] ? 1 : 0;
    CHECK(repeats <= 1);
    CHECK(t.mismatches == 0);
    CHECK(t.late <= 2);
    p.close();
}

namespace
{
// s-rta-0930 gop2: the key-flagged PACKETS and the key-flagged DECODED frames of a forward decode of `file` (a fixture's
// sanity tooth: the GOP structure the case depends on).
void keyCounts(const juce::File& file, int* keyPackets, int* keyFrames)
{
    *keyPackets = *keyFrames = 0;
    AVFormatContext* fmt = nullptr;
    REQUIRE(avformat_open_input(&fmt, file.getFullPathName().toRawUTF8(), nullptr, nullptr) == 0);
    REQUIRE(avformat_find_stream_info(fmt, nullptr) >= 0);
    const int si = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    REQUIRE(si >= 0);
    const AVCodec* codec = avcodec_find_decoder(fmt->streams[si]->codecpar->codec_id);
    AVCodecContext* cc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cc, fmt->streams[si]->codecpar);
    REQUIRE(avcodec_open2(cc, codec, nullptr) == 0);
    AVFrame* fr = av_frame_alloc();
    AVPacket* pk = av_packet_alloc();
    auto take = [&] {
        while (avcodec_receive_frame(cc, fr) == 0)
            *keyFrames += (fr->flags & AV_FRAME_FLAG_KEY) != 0 ? 1 : 0;
    };
    while (av_read_frame(fmt, pk) >= 0)
    {
        if (pk->stream_index == si)
        {
            *keyPackets += (pk->flags & AV_PKT_FLAG_KEY) != 0 ? 1 : 0;
            if (avcodec_send_packet(cc, pk) >= 0)
                take();
        }
        av_packet_unref(pk);
    }
    avcodec_send_packet(cc, nullptr);
    take();
    av_frame_free(&fr);
    av_packet_free(&pk);
    avcodec_free_context(&cc);
    avformat_close_input(&fmt);
}

// s-rta-0930 gop2 (GC7): a deterministic real-time emulation -- the REAL player reversing, the writer stepped with at most
// K FFmpeg decodes (the framesDecoded delta) per 120 Hz render frame, the decode-time estimate pinned before EVERY step to
// the emulated t_d = 8.333 / K ms (x pinFactor; or the harness's own 0.9 / 0.1 EMA of t_d from `ema`, emulateEma), then the
// pick and the committed Show's late rule (counted only while playing). It counts decodes, never time: reruns are identical.
// late / shown are split at the event (phase 0 before it fires, 1 after); every picked frame is compared with `ref`.
struct Emulated
{
    VideoPlayer& p;
    VideoStats& st;
    std::map<long, Bytes>& ref;
    int K = 3;
    double pinFactor = 1.0;
    bool emulateEma = false;
    double ema = 0.0;
    long late[2] = { 0, 0 }, shown[2] = { 0, 0 };
    long mismatches = 0, lastShown = -1;

    double td() const { return 1000.0 / 120.0 / K; }
    void frame(int phase)
    {
        p.advanceFrame(1.0 / 120.0);
        const long d0 = st.framesDecoded.load();
        for (int s = 0; s < 20000; ++s)
        {
            VideoPlayerTestAccess::pinDecodeMs(p, emulateEma ? ema : td() * pinFactor);
            const long before = st.framesDecoded.load();
            if (before - d0 >= K || !VideoPlayerTestAccess::step(p))
                break;
            for (long d = before; d < st.framesDecoded.load(); ++d)
                ema = 0.9 * ema + 0.1 * td();
        }
        Bytes b;
        const long k = VideoPlayerTestAccess::pick(p, &b);
        if (k >= 0)
        {
            shown[phase] += k != lastShown ? 1 : 0;
            lastShown = k;
            mismatches += (ref.count(k) == 0 || ref[k] != b) ? 1 : 0;
        }
        else if (lastShown >= 0 && p.isPlaying()
                 && std::fabs(VideoPlayerTestAccess::clock(p) - VideoPlayerTestAccess::lastPickedPts(p))
                        > 1.5 * VideoPlayerTestAccess::frameDur(p))
            ++late[phase];
    }
    long lateTotal() const { return late[0] + late[1]; }
    long shownTotal() const { return shown[0] + shown[1]; }
};

// The player opened with a local Budget of `capFrames` frames, reversing from the Loop wrap (the T1b start).
void openCapped(VideoPlayer& p, VideoStats& st, GopCache::Budget& budget, const juce::File& f, int capFrames)
{
    VideoPlayerTestAccess::mallocPath(p);
    VideoPlayerTestAccess::setBudget(p, &budget);
    p.setStats(&st);
    REQUIRE(p.open(f));
    budget.total.store(capFrames * VideoPlayerTestAccess::frameBytes(p));
    p.setReverse(true);
    p.advanceFrame(1.0 / 1000.0);   // the Loop wrap to the end, reversing
}
} // namespace

// s-rta-0930 gop2 (GC7, plan item 1 + ruling A5): a GOP-250 file (keys 0 and 250) reversed for 9 s from the Loop wrap in a
// small share, under an emulated decoder of K decodes per render frame. main 655d232 planned a PREFETCH run only once half
// the share was free -- too little cover for the lead-in from keyframe 0 into the top of GOP 0 (the live u8 row: late 123
// per 5 s at 256 MB). The in-place window counts the slots the clock frees during the lead-in. Pre-registered (late /
// shown): main (a) 69 / 246, (b) 56 / 250, (c) 0 / 269, (d) 403 / 160, (e) 69 / 246, (f) 69 / 246; RED on 655d232: (a) (b)
// (d) (e) (f).
TEST_CASE("gop2 (GC7): a GOP-250 reverse Loop in a small share keeps up with an emulated decoder", "[video_player][gopcache][s-rta-0930]")
{
    const auto f = fixture("video_h264_gop250_64x64.mp4");
    auto ref = forwardDecode(f);
    REQUIRE(ref.size() == 300);
    int keyPackets = 0, keyFrames = 0;
    keyCounts(f, &keyPackets, &keyFrames);
    REQUIRE(keyPackets == 2);   // keys 0 and 250: the fixture still has the u8 shape
    REQUIRE(keyFrames == 2);
    struct Config
    {
        const char* name;
        int cap, K;
        double pinFactor;
        bool ema;
        long lateMax, shownMin;
    };
    for (const Config& c : { Config{ "(a) CAP 21, K 3", 21, 3, 1.0, false, 4, 265 }, Config{ "(b) CAP 16, K 4", 16, 4, 1.0, false, 4, 265 },
                             Config{ "(c) CAP 21, K 5", 21, 5, 1.0, false, 4, 265 }, Config{ "(d) CAP 21, K 2", 21, 2, 1.0, false, 100, 240 },
                             Config{ "(e) CAP 21, K 3, EMA from a 2x seed", 21, 3, 2.0, true, 4, 265 },
                             Config{ "(f) CAP 21, K 3, pinned 2x", 21, 3, 2.0, false, 4, 265 } })
    {
        VideoStats st;
        GopCache::Budget budget;
        VideoPlayer p;
        openCapped(p, st, budget, f, c.cap);
        REQUIRE(VideoPlayerTestAccess::gopEst(p) == 250);
        Emulated e{ p, st, ref, c.K, c.ema ? 1.0 : c.pinFactor, c.ema };
        e.ema = e.td() * c.pinFactor;
        for (int i = 0; i < 9 * 120; ++i)
            e.frame(0);
        int checked = 0;
        const int bad = VideoPlayerTestAccess::cacheMismatches(p, ref, &checked);
        std::printf("gop2 T1b %-36s late %ld shown %ld mismatches %ld cacheMismatches %d/%d nonmono %lld decodes %lld\n", c.name,
                    e.lateTotal(), e.shownTotal(), e.mismatches, bad, checked, static_cast<long long>(st.reverseNonmonotonic.load()),
                    static_cast<long long>(st.framesDecoded.load()));
        CAPTURE(c.name, e.lateTotal(), e.shownTotal(), checked, st.framesDecoded.load(), st.gopCacheMisses.load());
        CHECK(e.lateTotal() <= c.lateMax);
        CHECK(e.shownTotal() >= c.shownMin);
        CHECK(e.mismatches == 0);
        CHECK(bad == 0);
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
}

// s-rta-0930 gop2 (ruling A5, T1c): the T1b harness (CAP 21, K 3) with ONE event fired, before that frame's advanceFrame, at
// the first render frame >= 1.6 s where a PREFETCH run is in its lead-in. Pre-registered totals (main -> amended): pause
// 33 -> 0, 2x speed 558 -> 337, a halved budget 469 -> 428, a decoder slowing from 5 to 3 decodes a frame 38 -> 0, a flip
// to forward 3 (post 0) -> 31 (pre 0). RED on 655d232: pause and slew; the rest are guards (never later than main; a flip
// never stalls: post-flip late <= the forward catch-up's bound ceil(gopFramesEst_ / K) + 6 = 90).
TEST_CASE("gop2 (GC7): a pause, a 2x speed change, a halved budget, a slower decoder or a flip DURING a PREFETCH lead-in -- "
          "identity holds, never later than main, a flip never stalls", "[video_player][gopcache][s-rta-0930]")
{
    const auto f = fixture("video_h264_gop250_64x64.mp4");
    auto ref = forwardDecode(f);
    REQUIRE(ref.size() == 300);
    for (const char* ev : { "pause", "speed", "shrink", "slew", "flip" })
    {
        const std::string event = ev;
        VideoStats st;
        GopCache::Budget budget;
        VideoPlayer p;
        openCapped(p, st, budget, f, 21);
        Emulated e{ p, st, ref, event == "slew" ? 5 : 3 };
        long fired = -1, resumeAt = -1;
        for (long i = 0; i < 9 * 120; ++i)
        {
            const int phase = fired >= 0 ? 1 : 0;
            if (fired < 0 && i >= 192 && VideoPlayerTestAccess::inPrefetchLeadIn(p))
            {
                fired = i;
                if (event == "pause")
                {
                    p.setPlaying(false);
                    resumeAt = i + 120;
                }
                else if (event == "speed")
                    p.setSpeed(2.0f);
                else if (event == "shrink")
                    budget.total.store(budget.total.load() / 2);
                else if (event == "slew")
                    e.K = 3;
                else
                    p.setReverse(false);
            }
            if (resumeAt >= 0 && i == resumeAt)
            {
                p.setPlaying(true);
                resumeAt = -1;
            }
            e.frame(phase);
        }
        int checked = 0;
        const int bad = VideoPlayerTestAccess::cacheMismatches(p, ref, &checked);
        std::printf("gop2 T1c %-6s fired@%ld late pre %ld post %ld (total %ld) shown pre %ld post %ld mismatches %ld "
                    "cacheMismatches %d/%d nonmono %lld\n", ev, fired, e.late[0], e.late[1], e.lateTotal(), e.shown[0], e.shown[1],
                    e.mismatches, bad, checked, static_cast<long long>(st.reverseNonmonotonic.load()));
        CAPTURE(event, fired, e.late[0], e.late[1], e.shown[0], e.shown[1], checked);
        REQUIRE(fired >= 0);
        if (event == "pause" || event == "slew")
            CHECK(e.lateTotal() <= 4);
        else if (event == "speed")
            CHECK(e.lateTotal() <= 558);
        else if (event == "shrink")
            CHECK(e.lateTotal() <= 469);
        else
        {
            const int bound = (VideoPlayerTestAccess::gopEst(p) + e.K - 1) / e.K + 6;
            CHECK(bound == 90);
            CHECK(e.late[1] <= bound);
            CHECK(e.late[0] <= 4);
        }
        CHECK(e.mismatches == 0);
        CHECK(bad == 0);
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
}

// s-rta-0930 gop2 (ruling A1 / A6, T1d): a scene-cut file (keys 0 37 150 190 213 262: the longest interval 113 is the GOP
// estimate) reversed like T1b. The lead-in comes from the container index's real keyframe below a window: the grid of
// multiples of the longest GOP over-states it (the plan's grid spent 1388 / 1690 decodes -- more than main's 1213 / 1461).
// RED: m5 (keys ignored) on the c2 code; main passes (its window never counts a lead-in).
TEST_CASE("gop2 (GC7): on a scene-cut file the window's lead-in comes from the index's real keyframes -- never more decodes "
          "than main", "[video_player][gopcache][s-rta-0930]")
{
    const auto f = fixture("video_h264_scenecut_64x64.mp4");
    auto ref = forwardDecode(f);
    REQUIRE(ref.size() == 300);
    struct Config
    {
        const char* name;
        int cap, K;
        long decodesMax;   // main 655d232's count
    };
    for (const Config& c : { Config{ "CAP 16, K 3", 16, 3, 1213 }, Config{ "CAP 13, K 4", 13, 4, 1461 } })
    {
        VideoStats st;
        GopCache::Budget budget;
        VideoPlayer p;
        openCapped(p, st, budget, f, c.cap);
        CHECK(VideoPlayerTestAccess::keyRels(p) == std::vector<int>{ 0, 37, 150, 190, 213, 262 });
        CHECK(VideoPlayerTestAccess::gopEst(p) == 113);
        Emulated e{ p, st, ref, c.K };
        for (int i = 0; i < 9 * 120; ++i)
            e.frame(0);
        int checked = 0;
        const int bad = VideoPlayerTestAccess::cacheMismatches(p, ref, &checked);
        const long decodes = static_cast<long>(st.framesDecoded.load());
        std::printf("gop2 T1d %-12s late %ld shown %ld decodes %ld (main %ld) mismatches %ld cacheMismatches %d/%d nonmono %lld\n",
                    c.name, e.lateTotal(), e.shownTotal(), decodes, c.decodesMax, e.mismatches, bad, checked,
                    static_cast<long long>(st.reverseNonmonotonic.load()));
        CAPTURE(c.name, e.lateTotal(), e.shownTotal(), decodes, checked);
        CHECK(e.lateTotal() <= 4);
        CHECK(e.shownTotal() >= 265);
        CHECK(decodes <= c.decodesMax);
        CHECK(e.mismatches == 0);
        CHECK(bad == 0);
        CHECK(st.reverseNonmonotonic.load() == 0);
        p.close();
    }
}
