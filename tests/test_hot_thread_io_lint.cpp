// test_hot_thread_io_lint -- no image decode on the message thread, no stat() on the GL thread or per cell paint
// (s-rta-0928b mediaopen, .harmony/.reports/s-rta-0928b/plan-mediaopen.md 4.9(4) / R-12).
//   (a) juce::ImageFileFormat::loadFrom occurs ONLY in the off-thread decoders: render/ImageDecode.h (the clip-image /
//       sequence-frame decoder pool), ui/ClipThumbnails.h (the grid's thumbnail pool), ui/FilesBrowser.cpp (its own
//       thumbnail pool). A sequence open decoded frame 0 twice on the message thread (ImageSequence::open + the
//       drop / load thumbnail): 55.7 + 40.8 ms per noisy 1080p PNG (diag-media S3).
//   (b) File::existsAsFile does not occur in render/CompositorEngine.cpp (the GL thread: 6 stats per frame, 1.0-1.55 ms
//       outliers while videos decode), media/ImageSequence.cpp (300 stats per 300-frame open) or ui/ClipCell.cpp (the
//       missing-file "!" at 30 Hz x every media cell). Presence is Clip::mediaMissing, set by MediaPresence's off-thread
//       sweep (src/core/MediaPresence.h).
// DEBT (like test_shader_param_lint): TEXTUAL. Line comments are stripped; a call reached through another name (a
// helper, a macro) is not seen, and a use in an allowed file is not proven to run off the message thread. The live rows
// of .harmony/probe-media-open.sh (message-thread stall, fence counters, pixels) prove the effect.
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

namespace
{
namespace fs = std::filesystem;

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

std::string readRel(const std::string& rel)
{
    std::ifstream in(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    return stripLineComments(ss.str());
}

// "file:line" of every code line holding `needle` in `text`.
std::vector<std::string> hits(const std::string& rel, const std::string& text, const std::string& needle)
{
    std::vector<std::string> out;
    std::stringstream in(text);
    int n = 0;
    for (std::string l; std::getline(in, l);)
    {
        ++n;
        if (l.find(needle) != std::string::npos)
            out.push_back(rel + ":" + std::to_string(n));
    }
    return out;
}

std::string join(const std::vector<std::string>& v)
{
    std::string s;
    for (const auto& x : v) s += (s.empty() ? "" : ", ") + x;
    return s;
}
} // namespace

TEST_CASE("ImageFileFormat::loadFrom only in the off-thread decoders", "[mediaopen][lint]")
{
    const std::set<std::string> allowed = { "render/ImageDecode.h", "ui/ClipThumbnails.h", "ui/FilesBrowser.cpp" };
    const fs::path root(AUDIODNA_SRC_DIR);
    std::vector<std::string> bad;
    int scanned = 0;
    for (const auto& e : fs::recursive_directory_iterator(root))
    {
        if (!e.is_regular_file()) continue;
        const auto ext = e.path().extension().string();
        if (ext != ".cpp" && ext != ".h" && ext != ".mm") continue;
        const auto rel = fs::relative(e.path(), root).generic_string();
        ++scanned;
        if (allowed.count(rel) != 0) continue;
        for (const auto& h : hits(rel, readRel(rel), "ImageFileFormat::loadFrom"))
            bad.push_back(h);
    }
    INFO("scanned " << scanned << " files; loadFrom outside the decoders: " << join(bad));
    CHECK(scanned > 100);
    CHECK(bad.empty());
}

TEST_CASE("no existsAsFile on the GL thread's compositor, the sequence, or the cell paint", "[mediaopen][lint]")
{
    for (const std::string rel : { "render/CompositorEngine.cpp", "media/ImageSequence.cpp", "ui/ClipCell.cpp" })
    {
        const auto h = hits(rel, readRel(rel), "existsAsFile");
        INFO(rel << ": " << h.size() << " stat(s): " << join(h));
        CHECK(h.empty());
    }
}
