// test_render_thread_lint -- lane tsan (s-rta-1002; ruling .harmony/.reports/s-rta-0930/ruling-tsan.md amendment 8).
// Case 1 (T0): the GL thread never stores a clip's `playing` intent plainly. Renderer::syncMedia reads the intent
// once, pushes it to the player, and writes the player's state back with a compare-exchange on the value it read
// (src/render/ClipTransportSync.h), so a trigger or a pause landing inside the sync is never overwritten (the F5 /
// F13 lost update: an auto-played video stuck paused). The video AND the sequence branch both call it.
// DEBT (like test_hot_thread_io_lint): TEXTUAL. Line comments are stripped; a store reached through another name is
// not seen.
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

namespace
{
std::vector<std::string> codeLines(const std::string& rel)
{
    std::ifstream f(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(f.good());
    std::vector<std::string> out;
    for (std::string l; std::getline(f, l);)
    {
        const auto c = l.find("//");
        out.push_back(c == std::string::npos ? l : l.substr(0, c));
    }
    return out;
}
} // namespace

TEST_CASE("Renderer.cpp: no plain store to a clip's playing; syncMedia goes through ClipTransportSync",
          "[tsan_lint][lint]")
{
    const auto lines = codeLines("render/Renderer.cpp");
    // `playing =` (an assignment, not ==) or `playing.store(` on a member access (`.playing` / `->playing`).
    const std::regex plainStore(R"((\.|->)playing\s*(=[^=]|\.store\s*\())");
    const std::regex syncCall(R"(ClipTransportSync::\w+\s*\()");
    std::string stores;
    int syncCalls = 0;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (std::regex_search(lines[i], plainStore))
            stores += (stores.empty() ? "" : ", ") + std::string("Renderer.cpp:") + std::to_string(i + 1);
        auto begin = std::sregex_iterator(lines[i].begin(), lines[i].end(), syncCall);
        syncCalls += static_cast<int>(std::distance(begin, std::sregex_iterator()));
    }
    INFO("plain stores to playing: " << (stores.empty() ? "none" : stores));
    CHECK(stores.empty());
    INFO("ClipTransportSync:: call sites: " << syncCalls);
    CHECK(syncCalls >= 2);
}

// Case 2 (T3; ruling amendment 8, G5): the GL thread loads a layer's trigger tuple ONCE per layer per pass, where its
// first read always sat (plan Fork 6), and every read of that layer in the pass uses the one tuple. The counts below
// are PINNED: a changed count is a new (or a lost) GL-thread load of the tuple and must be re-justified in review.
// A load is `runtime()` through `.` or `->`, or `getActiveClip(` (one load inside); line comments stripped. Sites:
//   CompositorEngine.cpp  runtime() 2: compositeDeck's layer loop; compositePersistentLayers' layer loop (each feeds
//                         incomingImagePending, the fade tick, renderLayerStages' observe and applyTransition).
//                         getActiveClip( 2: compositeDeck's hasActiveLayers pre-scan; hasPersistentContent (a
//                         separate yes / no pre-scan, ruling R-A9: a clear between it and the layer loop shows the
//                         layer empty one frame early, which the clear shows next frame anyway).
//   Renderer.cpp          runtime() 0; getActiveClip( 1: the MilkDrop preset-playlist loop (renderOpenGL).
//   DeckClock.h           runtime() 1: tick's layer loop (fade tick, active clock, outgoing clock); getActiveClip( 0.
//   Autopilot.cpp         runtime() 3: processFrame's end-of-video pass, pending-trigger pass, beat pass (advanceClip /
//                         smartAdvanceClip take the column from it); getActiveClip( 0.
TEST_CASE("render thread: one trigger-tuple load per layer per pass (pinned counts)", "[tsan_lint][lint]")
{
    struct Pin
    {
        const char* file;
        int runtimeLoads;
        int activeClipLoads;
    };
    const Pin pins[] = {
        { "render/CompositorEngine.cpp", 2, 2 },
        { "render/Renderer.cpp", 0, 1 },
        { "render/DeckClock.h", 1, 0 },
        { "model/Autopilot.cpp", 3, 0 },
    };
    const std::regex runtimeLoad(R"((\.|->)\s*runtime\s*\(\s*\))");
    const std::regex activeClipLoad(R"(getActiveClip\s*\()");
    for (const auto& pin : pins)
    {
        const auto lines = codeLines(pin.file);
        int runtimeLoads = 0, activeClipLoads = 0;
        std::string where;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const auto& l = lines[i];
            const int r = static_cast<int>(std::distance(std::sregex_iterator(l.begin(), l.end(), runtimeLoad),
                                                         std::sregex_iterator()));
            const int a = static_cast<int>(std::distance(std::sregex_iterator(l.begin(), l.end(), activeClipLoad),
                                                         std::sregex_iterator()));
            if (r + a > 0)
                where += " " + std::to_string(i + 1);
            runtimeLoads += r;
            activeClipLoads += a;
        }
        INFO(pin.file << ": runtime() " << runtimeLoads << ", getActiveClip( " << activeClipLoads << " at lines"
                      << where);
        CHECK(runtimeLoads == pin.runtimeLoads);
        CHECK(activeClipLoads == pin.activeClipLoads);
    }
}
