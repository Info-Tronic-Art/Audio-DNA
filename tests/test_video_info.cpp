// s-rta-1002b ui U1 (BF3 data; plan-ui.md U1.1 / U1.2 + ruling AM9 / AM11): src/media/VideoInfo.h -- a video's codec,
// size and frame rate in a performer's words -- and the info the REAL VideoPlayer keeps from open().
// [videoinfo]: the pure formatter (codecLabel's whole table incl. the fallbacks, fpsText, codecLine / frameLine /
// describe).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoInfo.h"

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
