// test_video_player_gl -- s-rta-0929 vupload (plan-vupload.md 4.8 + HARMONY ADOPTION VU1 / VU2 / VU3 / VU8): the REAL
// VideoPlayer (src/media/VideoPlayer.cpp + FFmpeg) uploading into a PRIVATE CGL 4.1 core context -- no window, no drawable,
// nothing shown on a screen. No decode thread is started (start() is never called): the test IS the writer when a case
// needs a second frame (VideoPlayerTestAccess, a friend of VideoPlayer), so every case is deterministic. If the machine
// has no GL pixel format (a headless CI Mac) every case SKIPs loudly.
//
// Cases (plan item u3 = P4a, the client-upload path; the IOSurface paths and cases 3-5 arrive with plan items u4 / u5):
// (1) identity -- frame 0 uploaded equals a test-side FFmpeg decode + sws RGBA of frame 0 within 1/255 per channel, alpha
// included, for a yuv420p H.264, an odd-size rawvideo rgba and a HAP Alpha file; the shown slot is held (Reading) and
// the other two are Free. (2) after releaseGL() (a context loss) the FIRST uploadToTexture returns a non-zero texture
// with the same picture, pending false (RED on the pre-lane VideoPlayer: 0).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoPlayer.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#include <OpenGL/OpenGL.h>

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
// thread and -- no decode thread runs -- the only writer. VUPLOAD_RED_MAIN: the RED build of case (2) against the
// pre-lane VideoPlayer (no friend, no held slot) -- only the public API.
#ifndef VUPLOAD_RED_MAIN
struct VideoPlayerTestAccess
{
    static int held(const VideoPlayer& p) { return p.retire_.held; }
    static int freeCount(const VideoPlayer& p) { return p.ring_.freeCount(); }
    static VideoRing::SlotState state(const VideoPlayer& p, int i)
    {
        return static_cast<VideoRing::SlotState>(p.ring_.header(i).state.load());
    }
};
#endif

namespace
{
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

GLuint upload(VideoPlayer& v, bool& pending)
{
    pending = true;
    return v.uploadToTexture(&pending);   // the pre-lane signature: case (2) also builds against main's VideoPlayer
}
} // namespace

#ifndef VUPLOAD_RED_MAIN
TEST_CASE("(1) identity: frame 0 uploaded equals FFmpeg + sws RGBA within 1/255 (yuv420p, odd-size rgba, HAP Alpha)",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    for (const char* name : { "video_h264_64x64.mp4", "video_rawrgba_63x37.mov", "video_hapa_64x64.mov" })
        {
            INFO(name);
            int w = 0, h = 0;
            const Pixels ref = referenceFrame0(fixture(name), w, h);
            VideoPlayer v;
            REQUIRE(v.open(fixture(name)));
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
#endif

TEST_CASE("(2) after a GL release the first draw re-uploads the shown frame: a non-zero texture, same picture, not pending",
          "[video_player_gl][s-rta-0929]")
{
    if (!glReady())
        return;
    {
        int w = 0, h = 0;
        const Pixels ref = referenceFrame0(fixture("video_h264_64x64.mp4"), w, h);
        VideoPlayer v;
        REQUIRE(v.open(fixture("video_h264_64x64.mp4")));
        bool pending = true;
        REQUIRE(upload(v, pending) != 0);
        for (int cycle = 0; cycle < 2; ++cycle)
        {
            v.releaseGL();
            const GLuint tex = upload(v, pending);
            REQUIRE(tex != 0);    // RED on the pre-lane VideoPlayer: 0 (the shown slot was released at its upload)
            CHECK_FALSE(pending);
            CHECK(maxDiff(readTexture(tex, w, h), ref) <= 1);
#ifndef VUPLOAD_RED_MAIN
            CHECK(VideoPlayerTestAccess::freeCount(v) == 2);
#endif
        }
    }
}
