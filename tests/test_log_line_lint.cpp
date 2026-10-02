// test_log_line_lint -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T0 D6 / T7, ruling
// ruling-tsan.md amendment 2, Harmony adoption H3). Code that can run off the message thread never writes
// std::cerr: two threads streaming into std::cerr race on the shared ios_base state (family A of the s-rta-0929b
// TSan sweep: AnalysisThread vs the ApiServer thread; VideoPlayer's "Opened" line on two MediaOpen threads). Those
// files log with logLine(...) (src/core/LogLine.h: one std::fwrite of a whole formatted line).
// DEBT (like test_hot_thread_io_lint): TEXTUAL. Line comments are stripped; a std::cerr reached through another name
// is not seen. The list is the worker-thread file list of plan T7 (VideoPlayer.cpp whole, H3).
// Post-merge residual (R8, owned by lane bt2 until it merges): audio/AudioEngine.cpp and audio/DeviceGuard.cpp still
// write std::cerr; a post-merge sweep converts them and adds them here.
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

namespace
{
std::string stripLineComments(const std::string& text)
{
    std::stringstream in(text), out;
    for (std::string l; std::getline(in, l);)
    {
        const auto c = l.find("//");
        out << (c == std::string::npos ? l : l.substr(0, c)) << '\n';
    }
    return out.str();
}

// "file:line" of every code line holding `needle`.
std::vector<std::string> hits(const std::string& rel, const std::string& needle)
{
    std::ifstream f(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    std::stringstream in(stripLineComments(ss.str()));
    std::vector<std::string> out;
    int n = 0;
    for (std::string l; std::getline(in, l);)
    {
        ++n;
        if (l.find(needle) != std::string::npos)
            out.push_back(rel + ":" + std::to_string(n));
    }
    return out;
}
} // namespace

TEST_CASE("no std::cerr in code that runs off the message thread", "[tsan_lint][lint]")
{
    const std::vector<std::string> workerFiles = {
        "analysis/AnalysisThread.cpp", "api/ApiServer.cpp",        "test/TestServer.cpp",
        "render/Renderer.cpp",         "render/ImageDecode.h",     "render/TextureManager.cpp",
        "render/LUTLoader.cpp",        "recording/VideoRecorder.cpp", "core/MediaOpener.cpp",
        "media/ImageSequence.cpp",     "output/SyphonOutput.mm",   "output/SharedFrameSet.cpp",
        "output/OutputPresenter.cpp",  "media/VideoPlayer.cpp",
    };
    int total = 0;
    for (const auto& rel : workerFiles)
    {
        const auto h = hits(rel, "std::cerr");
        total += static_cast<int>(h.size());
        std::string where;
        for (const auto& x : h) where += (where.empty() ? "" : ", ") + x;
        INFO(rel << ": " << h.size() << " std::cerr: " << where);
        CHECK(h.empty());
    }
    INFO("total std::cerr in the worker-thread list: " << total);
    CHECK(total == 0);
}
