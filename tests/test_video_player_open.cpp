// s-rta-0928b video fix round 2: the REAL VideoPlayer (src/media/VideoPlayer.cpp + FFmpeg), headless, no GL context.
// (1) A video whose pixel format is unknown before a frame decodes (an H.264 .mp4 cut to its header: an interrupted
//     copy or download) must fail open() cleanly -- "no media" -- instead of passing AV_PIX_FMT_NONE to sws_getContext,
//     which trips a libswscale assertion and aborts the whole app. Each open runs in a forked CHILD process, so the RED
//     (the abort) is a failed assertion here, never a crashed test runner.
// (2) Control: a valid tiny H.264 .mp4 still opens (the guard rejects only the unknown format).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoPlayer.h"

#include <csignal>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
struct ChildResult
{
    bool exited = false;   // the child returned normally (not killed by a signal)
    int code = -1;         // its exit code: 0 = open() failed, 1 = open() succeeded, 2 = opened with a wrong size
    int signal = 0;        // the signal that killed it (SIGABRT = 6 for the libswscale assertion)
};

// open() the file in a forked child; the child reports the result through its exit code and never returns to Catch2.
ChildResult openInChild(const juce::File& file, int expectW, int expectH)
{
    ChildResult r;
    const pid_t pid = fork();
    REQUIRE(pid >= 0);
    if (pid == 0)
    {
        int code = 0;
        {
            VideoPlayer p;
            if (p.open(file))
                code = (p.getWidth() == expectW && p.getHeight() == expectH) ? 1 : 2;
            p.close();
        }
        _exit(code);
    }
    int status = 0;
    REQUIRE(waitpid(pid, &status, 0) == pid);
    r.exited = WIFEXITED(status);
    if (r.exited)
        r.code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        r.signal = WTERMSIG(status);
    return r;
}

juce::File fixture(const char* name)
{
    return juce::File(TEST_FIXTURES_DIR).getChildFile(name);
}
} // namespace

TEST_CASE("a video whose pixel format is unknown without a frame fails open() cleanly (no abort)", "[video_player][s-rta-0928b]")
{
    // 1,539 B: a 2 s 1080p x264 .mp4 (+faststart) cut to ftyp + moov + the mdat header + 8 bytes. ffprobe:
    // codec_name=h264, pix_fmt=unknown (no frame decodes, so the decoder never learns its format).
    const auto f = fixture("video_h264_cut_header.mp4");
    REQUIRE(f.existsAsFile());
    const auto r = openInChild(f, 1920, 1080);
    CAPTURE(r.exited, r.code, r.signal);
    REQUIRE(r.exited);        // RED (before the guard): killed by SIGABRT (signal 6) inside sws_getContext
    REQUIRE(r.code == 0);     // open() returned false: no player -> "no media"
}

TEST_CASE("a valid H.264 video still opens (the pixel-format guard rejects only an unknown format)", "[video_player][s-rta-0928b]")
{
    // 3,464 B: testsrc2 64x64, 30 fps, 0.2 s, libx264 yuv420p (+faststart).
    const auto f = fixture("video_h264_64x64.mp4");
    REQUIRE(f.existsAsFile());
    const auto r = openInChild(f, 64, 64);
    CAPTURE(r.exited, r.code, r.signal);
    REQUIRE(r.exited);
    REQUIRE(r.code == 1);     // opened, 64 x 64
}
