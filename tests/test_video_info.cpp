// s-rta-1002b ui U1 (BF3 data; plan-ui.md U1.1 / U1.2 + ruling AM9 / AM11): src/media/VideoInfo.h -- a video's codec,
// size and frame rate in a performer's words -- and the info the REAL VideoPlayer keeps from open().
// [videoinfo]: the pure formatter (codecLabel's whole table incl. the fallbacks, fpsText, codecLine / frameLine /
// describe).
// [videoinfo][open]: VideoPlayer::open() on VALID fixtures only, in-process (the cut-header file stays in its forked-child
// test, test_video_player_open.cpp); each player is closed and never started. Expected values = ffprobe 8.0.
// Fixtures added by U1.2 (ffmpeg 8.0, /opt/homebrew/bin; testsrc2 64x64, 0.2 s):
//   video_prores_hq_64x64_2997.mov     ffmpeg -f lavfi -i testsrc2=s=64x64:r=30000/1001:d=0.2 -c:v prores_ks -profile:v 3
//                                      (20,345 B; prores HQ apch 30000/1001)
//   video_hapq_64x64_60.mov            ffmpeg -f lavfi -i testsrc2=s=64x64:r=60:d=0.2 -c:v hap -format hap_q
//                                      (25,312 B; hap HapY 60/1)
//   video_hevc_main10_64x64_23976.mp4  ffmpeg -f lavfi -i testsrc2=s=64x64:r=24000/1001:d=0.2 -c:v libx265
//                                      -pix_fmt yuv420p10le -x265-params log-level=none (4,529 B; hevc Main 10 24000/1001)
#include <catch2/catch_test_macros.hpp>

#include "media/VideoInfo.h"
#include "media/VideoPlayer.h"

#include <string>

using videoinfo::fourcc;

TEST_CASE("codecLabel: H.264 and HEVC carry their profile", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("h264", fourcc('a', 'v', 'c', '1'), "High") == "H.264 High");
    CHECK(videoinfo::codecLabel("h264", 0, "Constrained Baseline") == "H.264 Constrained Baseline");
    CHECK(videoinfo::codecLabel("h264", 0, "") == "H.264");
    CHECK(videoinfo::codecLabel("hevc", fourcc('h', 'e', 'v', '1'), "Main 10") == "HEVC Main 10");
    CHECK(videoinfo::codecLabel("hevc", 0, "") == "HEVC");
}

TEST_CASE("codecLabel: ProRes by profile, else by FourCC, else plain", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("prores", 0, "Proxy") == "ProRes 422 Proxy");
    CHECK(videoinfo::codecLabel("prores", 0, "LT") == "ProRes 422 LT");
    CHECK(videoinfo::codecLabel("prores", 0, "Standard") == "ProRes 422");
    CHECK(videoinfo::codecLabel("prores", 0, "HQ") == "ProRes 422 HQ");
    CHECK(videoinfo::codecLabel("prores", 0, "4444") == "ProRes 4444");
    CHECK(videoinfo::codecLabel("prores", 0, "XQ") == "ProRes 4444 XQ");
    // No profile: the FourCC decides.
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', 'c', 'o'), "") == "ProRes 422 Proxy");
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', 'c', 's'), "") == "ProRes 422 LT");
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', 'c', 'n'), "") == "ProRes 422");
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', 'c', 'h'), "") == "ProRes 422 HQ");
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', '4', 'h'), "") == "ProRes 4444");
    CHECK(videoinfo::codecLabel("prores", fourcc('a', 'p', '4', 'x'), "") == "ProRes 4444 XQ");
    // Neither.
    CHECK(videoinfo::codecLabel("prores", 0, "") == "ProRes");
    CHECK(videoinfo::codecLabel("prores", fourcc('x', 'x', 'x', 'x'), "") == "ProRes");
}

TEST_CASE("codecLabel: HAP by FourCC, else plain HAP", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("hap", fourcc('H', 'a', 'p', '1'), "") == "HAP");
    CHECK(videoinfo::codecLabel("hap", fourcc('H', 'a', 'p', '5'), "") == "HAP Alpha");
    CHECK(videoinfo::codecLabel("hap", fourcc('H', 'a', 'p', 'Y'), "") == "HAP Q");
    CHECK(videoinfo::codecLabel("hap", fourcc('H', 'a', 'p', 'M'), "") == "HAP Q Alpha");
    CHECK(videoinfo::codecLabel("hap", fourcc('H', 'a', 'p', 'A'), "") == "HAP Alpha Only");
    CHECK(videoinfo::codecLabel("hap", 0, "") == "HAP");
}

TEST_CASE("codecLabel: the named codecs", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("dxv", 0, "") == "DXV");
    CHECK(videoinfo::codecLabel("mjpeg", 0, "Baseline") == "Motion JPEG");
    CHECK(videoinfo::codecLabel("mpeg4", 0, "Simple Profile") == "MPEG-4 Part 2");
    CHECK(videoinfo::codecLabel("mpeg4", fourcc('F', 'M', 'P', '4'), "Advanced Simple Profile") == "MPEG-4 Part 2");
    CHECK(videoinfo::codecLabel("mpeg2video", 0, "Main") == "MPEG-2");
    CHECK(videoinfo::codecLabel("qtrle", 0, "") == "QuickTime Animation");
    CHECK(videoinfo::codecLabel("png", 0, "") == "PNG");
    CHECK(videoinfo::codecLabel("rawvideo", fourcc('R', 'G', 'B', 'A'), "") == "Uncompressed");
    CHECK(videoinfo::codecLabel("vp8", 0, "") == "VP8");
    CHECK(videoinfo::codecLabel("vp9", 0, "Profile 0") == "VP9");
    CHECK(videoinfo::codecLabel("av1", 0, "Main") == "AV1");
    CHECK(videoinfo::codecLabel("cfhd", 0, "") == "CineForm");
}

TEST_CASE("codecLabel: DNxHD, or the DNxHR profile", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("dnxhd", 0, "") == "DNxHD");
    CHECK(videoinfo::codecLabel("dnxhd", 0, "DNXHD") == "DNxHD");
    CHECK(videoinfo::codecLabel("dnxhd", 0, "DNXHR HQ") == "DNxHR HQ");
    CHECK(videoinfo::codecLabel("dnxhd", 0, "DNXHR 444") == "DNxHR 444");
}

TEST_CASE("codecLabel: any other codec is its FFmpeg name upper-cased; no name is unknown", "[videoinfo]")
{
    CHECK(videoinfo::codecLabel("weird", 0, "") == "WEIRD");
    CHECK(videoinfo::codecLabel("theora", 0, "") == "THEORA");
    CHECK(videoinfo::codecLabel("", 0, "") == "");
    CHECK(videoinfo::codecLabel("", fourcc('a', 'v', 'c', '1'), "High") == "");
}

TEST_CASE("fpsText: 3 decimals, trailing zeros stripped; no rate is empty", "[videoinfo]")
{
    CHECK(videoinfo::fpsText(30000.0 / 1001.0) == "29.97");
    CHECK(videoinfo::fpsText(24000.0 / 1001.0) == "23.976");
    CHECK(videoinfo::fpsText(60000.0 / 1001.0) == "59.94");
    CHECK(videoinfo::fpsText(25.0) == "25");
    CHECK(videoinfo::fpsText(30.0) == "30");
    CHECK(videoinfo::fpsText(85.0 / 4.0) == "21.25");
    CHECK(videoinfo::fpsText(0.0) == "");
    CHECK(videoinfo::fpsText(-1.0) == "");
}

TEST_CASE("codecLine / frameLine / describe", "[videoinfo]")
{
    const VideoInfo v{ "H.264 High", 1920, 1080, 30000.0 / 1001.0 };
    CHECK(v.known());
    CHECK(videoinfo::codecLine(v) == "H.264 High");
    CHECK(videoinfo::frameLine(v) == "1920 x 1080, 29.97 frames per second");
    CHECK(videoinfo::describe(v) == "H.264 High, 1920 x 1080, 29.97 frames per second");

    const VideoInfo noRate{ "H.264 High", 1920, 1080, 0.0 };
    CHECK(videoinfo::codecLine(noRate) == "H.264 High");
    CHECK(videoinfo::frameLine(noRate) == "1920 x 1080");
    CHECK(videoinfo::describe(noRate) == "H.264 High, 1920 x 1080");

    CHECK_FALSE(VideoInfo{}.known());
}

namespace
{
struct Opened
{
    bool ok = false;
    VideoInfo info;
};

// open() the fixture in-process, copy the info, close the never-started player.
Opened openFixture(const char* name)
{
    const auto f = juce::File(TEST_FIXTURES_DIR).getChildFile(name);
    REQUIRE(f.existsAsFile());
    Opened o;
    VideoPlayer p;
    o.ok = p.open(f);
    o.info = p.getInfo();
    p.close();
    return o;
}
} // namespace

TEST_CASE("open() keeps the info: H.264 High .mp4, 30 frames per second exactly", "[videoinfo][open]")
{
    const auto o = openFixture("video_h264_64x64.mp4");   // ffprobe: h264 High avc1 64x64 30/1
    REQUIRE(o.ok);
    CHECK(o.info.known());
    CHECK(o.info.codec == "H.264 High");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(o.info.fps == 30.0);
    CHECK(videoinfo::describe(o.info) == "H.264 High, 64 x 64, 30 frames per second");
}

TEST_CASE("open() keeps the info: HAP Alpha .mov", "[videoinfo][open]")
{
    const auto o = openFixture("video_hapa_64x64.mov");   // hap Hap5 64x64 30/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "HAP Alpha");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "30");
}

TEST_CASE("open() keeps the info: ProRes 422 HQ .mov at 29.97", "[videoinfo][open]")
{
    const auto o = openFixture("video_prores_hq_64x64_2997.mov");   // prores HQ apch 64x64 30000/1001
    REQUIRE(o.ok);
    CHECK(o.info.codec == "ProRes 422 HQ");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "29.97");
}

TEST_CASE("open() keeps the info: HAP Q .mov at 60", "[videoinfo][open]")
{
    const auto o = openFixture("video_hapq_64x64_60.mov");   // hap HapY 64x64 60/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "HAP Q");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "60");
}

TEST_CASE("open() keeps the info: HEVC Main 10 .mp4 at 23.976", "[videoinfo][open]")
{
    const auto o = openFixture("video_hevc_main10_64x64_23976.mp4");   // hevc Main 10 hev1 64x64 24000/1001
    REQUIRE(o.ok);
    CHECK(o.info.codec == "HEVC Main 10");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "23.976");
}

TEST_CASE("open() keeps the info: MPEG-4 Part 2 .avi", "[videoinfo][open]")
{
    const auto o = openFixture("video_mpeg4_bf2_64x64.avi");   // mpeg4 Advanced Simple Profile FMP4 64x64 30/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "MPEG-4 Part 2");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "30");
}

TEST_CASE("open() keeps the info: a variable-rate H.264 shows its average rate (21.25), not r_frame_rate", "[videoinfo][open]")
{
    const auto o = openFixture("video_h264_vfrgap_64x64.mp4");   // h264 High avc1 64x64, avg 85/4, r 30/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "H.264 High");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "21.25");
}

TEST_CASE("open() keeps the info: uncompressed RGBA .mov, 63 x 37", "[videoinfo][open]")
{
    const auto o = openFixture("video_rawrgba_63x37.mov");   // rawvideo RGBA, profile unknown, 63x37 30/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "Uncompressed");
    CHECK(o.info.width == 63);
    CHECK(o.info.height == 37);
    CHECK(videoinfo::fpsText(o.info.fps) == "30");
}

TEST_CASE("open() keeps the info: MPEG-4 Part 2 in an MPEG transport stream (.ts)", "[videoinfo][open]")
{
    const auto o = openFixture("video_mpeg4_64x64.ts");   // mpeg4 Simple Profile, tag 0x0010, 64x64 30/1
    REQUIRE(o.ok);
    CHECK(o.info.codec == "MPEG-4 Part 2");
    CHECK(o.info.width == 64);
    CHECK(o.info.height == 64);
    CHECK(videoinfo::fpsText(o.info.fps) == "30");
}
