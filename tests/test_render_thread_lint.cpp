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
