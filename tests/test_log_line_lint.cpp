// test_log_line_lint -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T0 D6 / T7, ruling
// ruling-tsan.md amendment 2, Harmony adoption H3). Code that can run off the message thread never writes
// std::cerr: two threads streaming into std::cerr race on the shared ios_base state (family A of the s-rta-0929b
// TSan sweep: AnalysisThread vs the ApiServer thread; VideoPlayer's "Opened" line on two MediaOpen threads). Those
// files log with logLine(...) (src/core/LogLine.h: one std::fwrite of a whole formatted line).
// DEBT (like test_hot_thread_io_lint): TEXTUAL. Line comments are stripped; a std::cerr reached through another name
// is not seen. The list is the worker-thread file list of plan T7 (VideoPlayer.cpp whole, H3).
// Post-merge residual (R8, owned by lane bt2 until it merges): audio/AudioEngine.cpp and audio/DeviceGuard.cpp still
// write std::cerr; a post-merge sweep converts them and adds them here.
// Fix round (ruling F3, CLAUDE.md Sacred Rule 3): a thread that must not allocate in its steady state logs with the
// zero-heap logLinef (a stack buffer, one fwrite): the analysis thread never calls logLine (its ostringstream +
// std::string allocate), nor do the GL thread's periodic Render Profile / Adaptive quality lines; logLinef's output
// is pinned against the text the old std::cerr chain printed.
#include <catch2/catch_test_macros.hpp>
#include "core/LogLine.h"
#include <cstdio>
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

// The code lines (comments stripped) of `rel` holding `needle`.
std::vector<std::string> codeLines(const std::string& rel, const std::string& needle)
{
    std::ifstream f(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(f.good());
    std::stringstream ss;
    ss << f.rdbuf();
    std::stringstream in(stripLineComments(ss.str()));
    std::vector<std::string> out;
    for (std::string l; std::getline(in, l);)
        if (l.find(needle) != std::string::npos)
            out.push_back(l);
    return out;
}

// What the old `std::cerr << a << b ...` chain printed for these operands (logLine's text).
template <class... A>
std::string streamed(const A&... a)
{
    std::ostringstream os;
    (os << ... << a);
    os << '\n';
    return os.str();
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

TEST_CASE("threads that must not allocate log with the zero-heap logLinef", "[tsan_lint][lint]")
{
    // The analysis thread (rate change, the profile interval): no logLine at all, its four statements are logLinef.
    const auto ostreamLines = hits("analysis/AnalysisThread.cpp", "logLine(");
    std::string where;
    for (const auto& x : ostreamLines) where += (where.empty() ? "" : ", ") + x;
    INFO("analysis/AnalysisThread.cpp logLine( at: " << where);
    CHECK(ostreamLines.empty());
    CHECK(hits("analysis/AnalysisThread.cpp", "logLinef(").size() == 4);
    // The GL thread's periodic lines.
    for (const std::string needle : { "[Render Profile] Avg frame", "Adaptive quality: disabled" })
    {
        const auto lines = codeLines("render/Renderer.cpp", needle);
        INFO("render/Renderer.cpp line holding \"" << needle << "\"");
        REQUIRE(lines.size() == 1);
        CHECK(lines[0].find("logLinef(") != std::string::npos);
    }
}

TEST_CASE("logLinef: one whole line in a stack buffer, the old std::cerr text, truncation marked", "[log_line]")
{
    char buf[kLogLineMax];
    auto fmt = [&buf](std::size_t n) { return std::string(buf, n); };

    // The AnalysisThread lines: identical to what the std::cerr chain printed.
    CHECK(fmt(formatLogLinef(buf, sizeof buf, "[Analysis] source rate %d Hz -> %s, bandwidth %d Hz", 44100,
                             "resampling to 48000 Hz", 22050))
          == streamed("[Analysis] source rate ", 44100, " Hz -> ", "resampling to 48000 Hz", ", bandwidth ", 22050,
                      " Hz"));
    CHECK(fmt(formatLogLinef(buf, sizeof buf, "[Analysis Profile] Per-stage avg (us) over %d hops:", 500))
          == "[Analysis Profile] Per-stage avg (us) over 500 hops:\n");
    CHECK(fmt(formatLogLinef(buf, sizeof buf, "  %s: %d us", "Spectral", 42)) == "  Spectral: 42 us\n");
    CHECK(fmt(formatLogLinef(buf, sizeof buf, "  TOTAL: %d us (%d%% of hop period)", 412, 38))
          == streamed("  TOTAL: ", 412, " us (", 38, "% of hop period)"));
    // The GL Render Profile line: %g prints a double exactly as an ostream does (8.33, 16 not 16.0, 0.5).
    for (const double ms : { 8.33, 16.0, 0.5, 123.45 })
        CHECK(fmt(formatLogLinef(buf, sizeof buf, "[Render Profile] Avg frame: %g ms, %d effects active, %dx%d", ms, 3,
                                 1920, 1080))
              == streamed("[Render Profile] Avg frame: ", ms, " ms, ", 3, " effects active, ", 1920, "x", 1080));

    // Truncation: a 24-byte buffer keeps 22 characters, the last 14 replaced by the marker, then '\n' and NUL.
    char small[24];
    const std::size_t n = formatLogLinef(small, sizeof small, "%s", "abcdefghijklmnopqrstuvwxyz0123456789");
    CHECK(n == sizeof small - 1);
    CHECK(std::string(small, n) == std::string("abcdefgh") + kLogLineTruncated + "\n");
    CHECK(small[n] == '\0');
    // A line that exactly fits is not marked.
    CHECK(std::string(small, formatLogLinef(small, sizeof small, "%s", "0123456789012345678901"))
          == "0123456789012345678901\n");
    // A buffer too small for the marker writes nothing.
    char tiny[8];
    CHECK(formatLogLinef(tiny, sizeof tiny, "x") == 0);

    // logLinefTo writes exactly the formatted line to the FILE* (logLinef is the same with stderr).
    std::FILE* f = std::tmpfile();
    REQUIRE(f != nullptr);
    logLinefTo(f, "  %s: %d us", "Spectral", 42);
    std::rewind(f);
    char back[64] = {};
    const std::size_t got = std::fread(back, 1, sizeof back - 1, f);
    std::fclose(f);
    CHECK(std::string(back, got) == "  Spectral: 42 us\n");
}
