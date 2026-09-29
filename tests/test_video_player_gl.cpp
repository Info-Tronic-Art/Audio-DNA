// test_video_player_gl -- s-rta-0929 vupload (plan-vupload.md 4.8 + HARMONY ADOPTION VU1 / VU2 / VU3 / VU8): the REAL
// VideoPlayer (src/media/VideoPlayer.cpp + FFmpeg) uploading into a PRIVATE CGL 4.1 core context -- no window, no drawable,
// nothing shown on a screen. No decode thread is started (start() is never called): the test IS the writer when a case
// needs a second frame (VideoPlayerTestAccess, a friend of VideoPlayer), so every case is deterministic. If the machine
// has no GL pixel format (a headless CI Mac) every case SKIPs loudly.
//
// Cases: (1) identity -- frame 0 through each upload path (the IOSurface blit, the surface client upload, the malloc
// client upload) equals a test-side FFmpeg decode + sws RGBA of frame 0 within 1/255 per channel, alpha included, for a
// yuv420p H.264, an odd-size rawvideo rgba (a padded IOSurface row) and a HAP Alpha file; the shown slot is held
// (Reading) and the other two are Free. (2) after releaseGL() (a context loss) the FIRST uploadToTexture returns a
// non-zero texture with the same picture, pending false (RED on the pre-lane VideoPlayer: 0) -- and releaseGL zeroes
// every GL handle (VU3). (3) the blit fence: a signaled fence of the held slot is deleted without releasing it; an
// injected GL_WAIT_FAILED releases a non-held slot and counts video_fence_failed (VU2). (4) the budget: a player over
// its cap HOLDS (the frame stays Ready) and is force-admitted at its defer bound; the first frame, the post-release
// re-upload and the first frame after a seek are exempt (VU8).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoPlayer.h"
#include "media/VideoUploadBudget.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#include <OpenGL/OpenGL.h>
#include <IOSurface/IOSurfaceRef.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace juce::gl;

using Pixels = std::vector<uint8_t>;   // RGBA8

// The test's writer / inspector (friend of VideoPlayer). Every accessor runs on the test thread, which is both the GL
// thread and -- no decode thread runs -- the only writer.
struct VideoPlayerTestAccess
{
    static void forcePath(VideoPlayer& p, VideoPlayer::UploadPath path) { p.forcePath_ = path; }
    static VideoPlayer::UploadPath path(const VideoPlayer& p) { return p.path_; }
    static int held(const VideoPlayer& p) { return p.retire_.held; }
    static int freeCount(const VideoPlayer& p) { return p.ring_.freeCount(); }
    static int readyCount(const VideoPlayer& p) { return p.ring_.readyCount(); }
    static VideoRing::SlotState state(const VideoPlayer& p, int i)
    {
        return static_cast<VideoRing::SlotState>(p.ring_.header(i).state.load());
    }
    static GLuint rectTex(const VideoPlayer& p, int i) { return p.rectTex_[static_cast<size_t>(i)]; }
    static GLuint dstFbo(const VideoPlayer& p) { return p.dstFbo_; }
    static bool fenced(const VideoPlayer& p, int i) { return p.fence_[static_cast<size_t>(i)] != nullptr; }
    static void setFenceWait(VideoPlayer& p, unsigned (*fn)(void*)) { p.fenceWaitOverride_ = fn; }

    // Writer: take a Free slot, write `rgba` (top-down, w x h) into it bottom-up in the path's byte order,
    // publish it at `pts` in the CURRENT request generation. Returns the slot.
    static int writeFrame(VideoPlayer& p, const Pixels& rgba, double pts)
    {
        const int s = p.ring_.acquireWrite();
        REQUIRE(s >= 0);
        const bool bgra = p.path_ != VideoPlayer::UploadPath::Malloc;
        uint8_t* base = p.slotBytes_[static_cast<size_t>(s)];
        if (bgra)
            IOSurfaceLock(static_cast<IOSurfaceRef>(p.surf_[static_cast<size_t>(s)]), 0, nullptr);
        for (int y = 0; y < p.height_; ++y)
        {
            uint8_t* row = base + static_cast<size_t>(p.height_ - 1 - y) * static_cast<size_t>(p.rowBytes_);
            for (int x = 0; x < p.width_; ++x)
            {
                const uint8_t* src = &rgba[static_cast<size_t>((y * p.width_ + x) * 4)];
                uint8_t* d = row + x * 4;
                d[0] = bgra ? src[2] : src[0];
                d[1] = src[1];
                d[2] = bgra ? src[0] : src[2];
                d[3] = src[3];
            }
        }
        if (bgra)
            IOSurfaceUnlock(static_cast<IOSurfaceRef>(p.surf_[static_cast<size_t>(s)]), 0, nullptr);
        p.ring_.publish(s, pts, p.gen_.load(), ++p.seq_);
        return s;
    }
};

namespace
{
using Path = VideoPlayer::UploadPath;

CGLContextObj makeContext()
{
    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile,
                                        static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_GL4_Core),
                                        static_cast<CGLPixelFormatAttribute>(0) };
    CGLPixelFormatObj pf = nullptr;
    GLint n = 0;
    if (CGLChoosePixelFormat(attrs, &pf, &n) != kCGLNoError || pf == nullptr)
        return nullptr;
    CGLContextObj ctx = nullptr;
    const CGLError err = CGLCreateContext(pf, nullptr, &ctx);
    CGLReleasePixelFormat(pf);
    return err == kCGLNoError ? ctx : nullptr;
}

// One private context for the whole run (made current on first use; the test runner is single-threaded).
bool glReady()
{
    static CGLContextObj ctx = [] {
        CGLContextObj c = makeContext();
        if (c != nullptr)
        {
            CGLSetCurrentContext(c);
            juce::gl::loadFunctions();   // dlsym-based on macOS: needs no JUCE context
        }
        return c;
    }();
    if (ctx == nullptr)
    {
        WARN("SKIP: no OpenGL 4.1 core pixel format on this machine -- test_video_player_gl did not run");
        return false;
    }
    CGLSetCurrentContext(ctx);
    return true;
}

juce::File fixture(const char* name)
{
    return juce::File(TEST_FIXTURES_DIR).getChildFile(name);
}

// Frame 0 of `file` decoded by FFmpeg here, converted like VideoPlayer does (sws, SWS_BILINEAR) to RGBA, top-down.
Pixels referenceFrame0(const juce::File& file, int& w, int& h)
{
    AVFormatContext* fmt = nullptr;
    REQUIRE(avformat_open_input(&fmt, file.getFullPathName().toRawUTF8(), nullptr, nullptr) == 0);
    REQUIRE(avformat_find_stream_info(fmt, nullptr) >= 0);
    int si = -1;
    for (unsigned i = 0; i < fmt->nb_streams; ++i)
        if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) { si = static_cast<int>(i); break; }
    REQUIRE(si >= 0);
    const AVCodec* codec = avcodec_find_decoder(fmt->streams[si]->codecpar->codec_id);
    REQUIRE(codec != nullptr);
    AVCodecContext* cc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cc, fmt->streams[si]->codecpar);
    REQUIRE(avcodec_open2(cc, codec, nullptr) == 0);
    AVFrame* fr = av_frame_alloc();
    AVPacket* pk = av_packet_alloc();
    bool got = false;
    while (!got && av_read_frame(fmt, pk) >= 0)
    {
        if (pk->stream_index == si && avcodec_send_packet(cc, pk) >= 0)
            got = avcodec_receive_frame(cc, fr) == 0;
        av_packet_unref(pk);
    }
    if (!got)
    {
        avcodec_send_packet(cc, nullptr);
        got = avcodec_receive_frame(cc, fr) == 0;
    }
    REQUIRE(got);
    w = cc->width;
    h = cc->height;
    Pixels out(static_cast<size_t>(w * h * 4));
    SwsContext* sws = sws_getContext(w, h, static_cast<AVPixelFormat>(fr->format), w, h, AV_PIX_FMT_RGBA, SWS_BILINEAR,
                                     nullptr, nullptr, nullptr);
    REQUIRE(sws != nullptr);
    uint8_t* dst[4] = { out.data(), nullptr, nullptr, nullptr };
    int ds[4] = { w * 4, 0, 0, 0 };
    sws_scale(sws, fr->data, fr->linesize, 0, h, dst, ds);
    sws_freeContext(sws);
    av_packet_free(&pk);
    av_frame_free(&fr);
    avcodec_free_context(&cc);
    avformat_close_input(&fmt);
    return out;
}

// The texture's pixels, flipped to top-down (GL row 0 = the bottom row).
Pixels readTexture(GLuint tex, int w, int h)
{
    Pixels up(static_cast<size_t>(w * h * 4));
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, up.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    Pixels down(up.size());
    for (int y = 0; y < h; ++y)
        std::memcpy(&down[static_cast<size_t>(y * w * 4)], &up[static_cast<size_t>((h - 1 - y) * w * 4)],
                    static_cast<size_t>(w * 4));
    return down;
}

int maxDiff(const Pixels& a, const Pixels& b)
{
    REQUIRE(a.size() == b.size());
    int m = 0;
    for (size_t i = 0; i < a.size(); ++i)
        m = std::max(m, std::abs(static_cast<int>(a[i]) - static_cast<int>(b[i])));
    return m;
}

Pixels pattern(int w, int h, int seed)
{
    Pixels p(static_cast<size_t>(w * h * 4));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            uint8_t* px = &p[static_cast<size_t>((y * w + x) * 4)];
            px[0] = static_cast<uint8_t>(x * 7 + seed * 31 + 1);
            px[1] = static_cast<uint8_t>(y * 13 + seed * 17 + 2);
            px[2] = static_cast<uint8_t>(x * y + seed * 5 + 3);
            px[3] = static_cast<uint8_t>(255 - ((x + y + seed) & 63));
        }
    return p;
}

const char* pathName(Path p)
{
    return p == Path::Blit ? "blit" : (p == Path::Client ? "surface client upload" : "malloc");
}

GLuint upload(VideoPlayer& v, bool& pending, VideoUpload::Budget* b = nullptr)
{
    pending = true;
    return v.uploadToTexture(&pending, b, 1.0 / 120.0);
}
} // namespace

TEST_CASE("(1) identity: frame 0 through every upload path equals FFmpeg + sws RGBA within 1/255 (yuv420p, odd-size rgba, HAP Alpha)",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    for (const char* name : { "video_h264_64x64.mp4", "video_rawrgba_63x37.mov", "video_hapa_64x64.mov" })
        for (Path path : { Path::Blit, Path::Client, Path::Malloc })
        {
            INFO(name << " via the " << pathName(path) << " path");
            int w = 0, h = 0;
            const Pixels ref = referenceFrame0(fixture(name), w, h);
            VideoPlayer v;
            VideoPlayerTestAccess::forcePath(v, path);
            REQUIRE(v.open(fixture(name)));
            REQUIRE(VideoPlayerTestAccess::path(v) == path);
            bool pending = true;
            const GLuint tex = upload(v, pending);
            REQUIRE(tex != 0);
            CHECK_FALSE(pending);
            CHECK(maxDiff(readTexture(tex, w, h), ref) <= 1);
            const int held = VideoPlayerTestAccess::held(v);
            REQUIRE(held >= 0);
            CHECK(VideoPlayerTestAccess::state(v, held) == VideoRing::SlotState::Reading);
            CHECK(VideoPlayerTestAccess::freeCount(v) == 2);
            v.releaseGL();
        }
}

TEST_CASE("(2) after a GL release the first draw re-uploads the shown frame: a non-zero texture, same picture, not pending (VU3: handles zeroed)",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    for (Path path : { Path::Blit, Path::Malloc })
    {
        INFO("the " << pathName(path) << " path");
        int w = 0, h = 0;
        const Pixels ref = referenceFrame0(fixture("video_h264_64x64.mp4"), w, h);
        VideoPlayer v;
        VideoPlayerTestAccess::forcePath(v, path);
        REQUIRE(v.open(fixture("video_h264_64x64.mp4")));
        bool pending = true;
        REQUIRE(upload(v, pending) != 0);
        for (int cycle = 0; cycle < 2; ++cycle)
        {
            v.releaseGL();
            CHECK(VideoPlayerTestAccess::rectTex(v, 0) == 0);
            CHECK(VideoPlayerTestAccess::dstFbo(v) == 0);
            const GLuint tex = upload(v, pending);
            REQUIRE(tex != 0);    // RED on the pre-lane VideoPlayer: 0 (the shown slot was released at its upload)
            CHECK_FALSE(pending);
            CHECK(maxDiff(readTexture(tex, w, h), ref) <= 1);
            CHECK(VideoPlayerTestAccess::freeCount(v) == 2);
        }
    }
}

TEST_CASE("(3) the blit fence: a signaled fence of the held slot is deleted, the slot kept; GL_WAIT_FAILED releases a non-held slot (VU2)",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    VideoStats stats;
    VideoPlayer v;
    v.setStats(&stats);
    VideoPlayerTestAccess::forcePath(v, Path::Blit);
    REQUIRE(v.open(fixture("video_h264_64x64.mp4")));
    bool pending = true;
    REQUIRE(upload(v, pending) != 0);
    const int held0 = VideoPlayerTestAccess::held(v);
    CHECK(VideoPlayerTestAccess::fenced(v, held0));
    glFinish();
    REQUIRE(upload(v, pending) != 0);                       // polls: signaled -> deleted, the slot stays the held one
    CHECK_FALSE(VideoPlayerTestAccess::fenced(v, held0));
    CHECK(VideoPlayerTestAccess::state(v, held0) == VideoRing::SlotState::Reading);
    CHECK(VideoPlayerTestAccess::freeCount(v) == 2);

    // A second frame while the first one's copy is (made to look) still running: the old slot waits for its fence.
    VideoPlayerTestAccess::setFenceWait(v, [](void*) -> unsigned { return GL_TIMEOUT_EXPIRED; });
    REQUIRE(upload(v, pending) != 0);                       // re-fence nothing; (held0 has no fence now)
    const int s1 = VideoPlayerTestAccess::writeFrame(v, pattern(64, 64, 1), 0.0);
    REQUIRE(upload(v, pending) != 0);                       // shows s1: held0 has no fence -> released at once
    CHECK(VideoPlayerTestAccess::held(v) == s1);
    CHECK(VideoPlayerTestAccess::state(v, held0) == VideoRing::SlotState::Free);
    const int s2 = VideoPlayerTestAccess::writeFrame(v, pattern(64, 64, 2), 0.0);
    REQUIRE(upload(v, pending) != 0);                       // shows s2: s1's fence "pending" -> s1 stays Reading
    CHECK(VideoPlayerTestAccess::held(v) == s2);
    CHECK(VideoPlayerTestAccess::state(v, s1) == VideoRing::SlotState::Reading);
    CHECK(VideoPlayerTestAccess::freeCount(v) == 1);

    VideoPlayerTestAccess::setFenceWait(v, [](void*) -> unsigned { return GL_WAIT_FAILED; });
    REQUIRE(upload(v, pending) != 0);                       // both fences "fail": s1 released, s2 (held) kept
    CHECK(VideoPlayerTestAccess::state(v, s1) == VideoRing::SlotState::Free);
    CHECK(VideoPlayerTestAccess::state(v, s2) == VideoRing::SlotState::Reading);
    CHECK_FALSE(VideoPlayerTestAccess::fenced(v, s1));
    CHECK_FALSE(VideoPlayerTestAccess::fenced(v, s2));
    CHECK(stats.fenceFailed.load() == 2);
    CHECK(VideoPlayerTestAccess::freeCount(v) == 2);
    VideoPlayerTestAccess::setFenceWait(v, nullptr);
    v.releaseGL();
}

TEST_CASE("(4) the budget: over the cap a player HOLDS (its frame stays Ready) until its defer bound; first frame, post-release and post-seek uploads are exempt (VU8)",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    VideoStats stats;
    VideoPlayer v;
    v.setStats(&stats);
    REQUIRE(v.open(fixture("video_h264_64x64.mp4")));
    VideoUpload::Budget b;
    auto frame = [&b] { b.beginFrame(); b.cap = 0; };   // a budget that admits nothing but the exempt / the bound
    bool pending = true;
    frame();
    const GLuint t0 = upload(v, pending, &b);
    REQUIRE(t0 != 0);                                        // never shown: exempt
    CHECK_FALSE(pending);
    const Pixels p1 = pattern(64, 64, 7);
    VideoPlayerTestAccess::writeFrame(v, p1, 0.0);
    frame();
    CHECK(upload(v, pending, &b) == t0);                     // deferred 1: the frame stays Ready
    CHECK_FALSE(pending);
    CHECK(VideoPlayerTestAccess::readyCount(v) == 1);
    frame();
    upload(v, pending, &b);                                  // deferred 2
    CHECK(VideoPlayerTestAccess::readyCount(v) == 1);
    CHECK(stats.uploadsDeferred.load() == 2);
    frame();
    const GLuint t1 = upload(v, pending, &b);                // at the bound (maxDefer 2 at 30 fps / 120 Hz): admitted
    CHECK(VideoPlayerTestAccess::readyCount(v) == 0);
    CHECK(maxDiff(readTexture(t1, 64, 64), p1) <= 1);

    v.releaseGL();                                           // a context loss, then a new frame: exempt
    const Pixels p2 = pattern(64, 64, 8);
    VideoPlayerTestAccess::writeFrame(v, p2, 0.0);
    frame();
    const GLuint t2 = upload(v, pending, &b);
    REQUIRE(t2 != 0);
    CHECK(maxDiff(readTexture(t2, 64, 64), p2) <= 1);
    CHECK(stats.uploadsDeferred.load() == 2);

    v.seekTo(0.0);                                           // a retrigger: the request generation moves on
    v.advanceFrame(0.0);
    const Pixels p3 = pattern(64, 64, 9);
    VideoPlayerTestAccess::writeFrame(v, p3, 0.0);           // the first frame of the new generation: exempt
    frame();
    const GLuint t3 = upload(v, pending, &b);
    CHECK(maxDiff(readTexture(t3, 64, 64), p3) <= 1);
    CHECK(stats.uploadsDeferred.load() == 2);
    v.releaseGL();
}
