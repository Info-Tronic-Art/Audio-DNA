// test_render_thread_lint -- lane tsan (s-rta-1002; ruling .harmony/.reports/s-rta-0930/ruling-tsan.md amendment 8).
// Case 1 (T0): the GL thread never stores a clip's `playing` intent plainly. Renderer::syncMedia reads the intent
// once, pushes it to the player, and writes the player's state back with a compare-exchange on the value it read
// (src/render/ClipTransportSync.h), so a trigger or a pause landing inside the sync is never overwritten (the F5 /
// F13 lost update: an auto-played video stuck paused). The video AND the sequence branch both call it.
// DEBT (like test_hot_thread_io_lint): TEXTUAL. Line comments are stripped; a store reached through another name is
// not seen.
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
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
// A load is `runtime()` through `.` or `->`, `getActiveClip(` (one load inside), or -- lane bf9b, its successor --
// `playing(` / `playingClip(` through `.` or `->` (Composition::playing: one load inside); line comments stripped.
// Sites (lane bf9b, s-rta-1002b; re-pinned with the shared stack -- DeckClock.h is deleted, Layer has no
// getActiveClip any more):
//   CompositorEngine.cpp  runtime() 1: compositeShow's layer loop (feeds incomingImagePending, the fade tick,
//                         renderLayerStages' observe and applyTransition). playingClip( 1: compositeShow's
//                         hasActiveLayers pre-scan (was getActiveClip( 1). getActiveClip( 0.
//   Renderer.cpp          runtime() 0; playingClip( 1: the MilkDrop preset-playlist loop over the PLAYING layers
//                         (ruling-bf10 H3; was getActiveClip( 1). getActiveClip( 0.
//   Autopilot.cpp         runtime() 1: processFrame's pending-trigger pass. playing( 2: the end-of-video pass and the
//                         beat pass (advanceClip / smartAdvanceClip take the ref from it; was runtime() 3).
// The two other spellings of a load -- captureLayerRuntime( (the compat surface) and LayerClock::advanceCrossfade(
// (load + tick) -- are pinned at 0 in all three files (fix-round NIT, review-tsan-tests-r1 N5).
TEST_CASE("render thread: one trigger-tuple load per layer per pass (pinned counts)", "[tsan_lint][lint]")
{
    struct Pin
    {
        const char* file;
        int runtimeLoads;
        int activeClipLoads;
        int playingLoads;
    };
    const Pin pins[] = {
        { "render/CompositorEngine.cpp", 1, 0, 1 },
        { "render/Renderer.cpp", 0, 0, 1 },
        { "model/Autopilot.cpp", 1, 0, 2 },
    };
    const std::regex runtimeLoad(R"((\.|->)\s*runtime\s*\(\s*\))");
    const std::regex activeClipLoad(R"(getActiveClip\s*\()");
    const std::regex playingLoad(R"((\.|->)\s*playing(Clip)?\s*\()");
    const std::regex otherLoad(R"(captureLayerRuntime\s*\(|LayerClock::advanceCrossfade\s*\()");
    auto count = [](const std::string& l, const std::regex& re) {
        return static_cast<int>(std::distance(std::sregex_iterator(l.begin(), l.end(), re), std::sregex_iterator()));
    };
    for (const auto& pin : pins)
    {
        const auto lines = codeLines(pin.file);
        int runtimeLoads = 0, activeClipLoads = 0, playingLoads = 0, otherLoads = 0;
        std::string where;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const auto& l = lines[i];
            const int r = count(l, runtimeLoad), a = count(l, activeClipLoad), p = count(l, playingLoad),
                      o = count(l, otherLoad);
            if (r + a + p + o > 0)
                where += " " + std::to_string(i + 1);
            runtimeLoads += r;
            activeClipLoads += a;
            playingLoads += p;
            otherLoads += o;
        }
        INFO(pin.file << ": runtime() " << runtimeLoads << ", getActiveClip( " << activeClipLoads << ", playing( "
                      << playingLoads << ", captureLayerRuntime( / LayerClock::advanceCrossfade( " << otherLoads
                      << " at lines" << where);
        CHECK(runtimeLoads == pin.runtimeLoads);
        CHECK(activeClipLoads == pin.activeClipLoads);
        CHECK(playingLoads == pin.playingLoads);
        CHECK(otherLoads == 0);
    }
}

// Case 3 (bf9 Stage P, s-rta-1002b; ruling-bf9 amendment 9): the Persistent layer feature is removed end to end. No
// identifier of it is left in any code line of src/ (*.h / *.cpp / *.mm, recursively; line comments stripped). The
// word is bare-word matched so the field's own declaration is caught too. Lane bf9b (ruling-bf9b B4e / plan F7):
// exactly ONE allow-listed file, src/model/ShowMigration.h -- the old-show converter is the only reader of a file's
// "persistent" key (it names the flag in the one conversion note, K7); it must still carry the key (not a stale
// allow-list entry).
TEST_CASE("no Persistent-feature identifier left in src/", "[lint][bf9]")
{
    namespace fs = std::filesystem;
    const std::regex feature(
        R"(\bpersistent\b|canBePersistent|compositePersistentLayers|hasPersistentContent|beginEmptyActiveDeck|persistentToggle_)");
    const fs::path root(AUDIODNA_SRC_DIR);
    std::vector<std::string> files;
    for (const auto& e : fs::recursive_directory_iterator(root))
    {
        if (!e.is_regular_file())
            continue;
        const auto ext = e.path().extension().string();
        if (ext == ".h" || ext == ".cpp" || ext == ".mm")
            files.push_back(fs::relative(e.path(), root).generic_string());
    }
    std::sort(files.begin(), files.end());
    REQUIRE(files.size() > 50);   // the walk reached src/
    const std::vector<std::string> allowed = { "model/ShowMigration.h" };
    std::string hits;
    int count = 0, allowedHits = 0;
    for (const auto& rel : files)
    {
        const bool isAllowed = std::find(allowed.begin(), allowed.end(), rel) != allowed.end();
        const auto lines = codeLines(rel);
        for (size_t i = 0; i < lines.size(); ++i)
            if (std::regex_search(lines[i], feature))
            {
                if (isAllowed)
                {
                    ++allowedHits;
                    continue;
                }
                ++count;
                hits += "\n  " + rel + ":" + std::to_string(i + 1);
            }
    }
    INFO(count << " Persistent-feature hit(s) in src/ code lines:" << (hits.empty() ? std::string(" none") : hits));
    CHECK(count == 0);
    INFO("allow-listed hits in model/ShowMigration.h: " << allowedHits);
    CHECK(allowed.size() == 1);
    CHECK(allowedHits > 0);
}

// Case 4 (lane bf9b, ruling-bf9b amendment 3(b), B4f): every src call site that changes the size or element storage
// of the box / stack structure -- Composition::decks / retiredDecks_ / layers, Deck::rows, ClipRow::clips -- is a
// FENCED site (the lane report's fence audit table: the enclosing UndoService::withDeckDetached / DeckFenceHook, a
// staged unpublished composition (Pitfall 58), or a local value the GL thread never sees). The GL thread resolves refs
// into ANY live or retired deck, so an unfenced writer of any of them races it. Over codeLines() of src/**/*.{h,cpp,mm},
// excluding the model files that define the structure (ClipRow.h, Deck.h, Composition.h, ShowMigration.h), the per-file
// count of the structure-writing calls below must equal the pinned map: a new site fails until it is audited (the
// tsan ruling's pinned-count pattern). The regex is the ruling's plus the two structure writers S2a added
// (insertLayerWithRows, insertDeckKeepingId -- S2a deviation 3(a), decided here). TEXTUAL (risk R-3): a benign call
// (a UI setClip setter, clear() on a local vector named `rows`) is pinned too; a writer reached through a reference
// alias is not seen -- the R-bf9b TSAN case is the behavioural net.
TEST_CASE("bf9b: box / stack structure writers sit only in audited sites (pinned counts)", "[lint][bf9b]")
{
    namespace fs = std::filesystem;
    const std::regex method(
        R"(\b(appendDeck|addDeck|retireOrEraseDeck|restoreRetiredDeck|reapRetiredDecks|insertLayer|insertLayerWithRows|insertDeckKeepingId|eraseLayer|moveLayer|addColumn|removeColumn|ensureColumns|setClip|clearCell)\s*\()");
    const std::regex container(
        R"(\b(decks|retiredDecks_|layers|rows|clips)\s*\.\s*(push_back|emplace_back|insert|erase|resize|assign|clear|swap)\s*\()");
    const std::vector<std::string> excluded = { "model/ClipRow.h", "model/Deck.h", "model/Composition.h",
                                                "model/ShowMigration.h" };
    // The audited sites (lane report "FENCE AUDIT TABLE"), per file.
    const std::map<std::string, int> pinned = {
        { "MainComponent.cpp", 25 },        // drop handlers, cell / row clears, column add / remove: withDeckDetached;
                                            // 3 x ClipInspector::setClip (a UI setter)
        { "core/ClipCommands.h", 2 },       // SetClipCmd / SwapClipsCmd apply: runFenced (DeckFenceHook)
        { "core/CompositionLoad.h", 1 },    // validateDeck: a staged, unpublished composition / deck (Pitfall 58)
        { "core/DeckCommands.h", 22 },      // every deck / layer / column command body: runFenced; 1 local snapshot
        { "core/UndoService.cpp", 2 },      // reapRetiredDecks: inside withDeckDetached itself
        { "recording/RoutineEngine.cpp", 2 },   // a local Footprint vector `layers`
        { "ui/ClipCell.cpp", 1 },           // ClipCell::setClip (a UI setter)
        { "ui/ClipCell.h", 1 },
        { "ui/ClipInspector.cpp", 1 },      // ClipInspector::setClip (a UI setter)
        { "ui/ClipInspector.h", 1 },
        { "ui/DeckView.cpp", 2 },           // ClipCell::setClip calls (a UI setter)
        { "ui/InspectorPanel.cpp", 1 },     // ClipInspector::setClip call (a UI setter)
    };
    const fs::path root(AUDIODNA_SRC_DIR);
    std::map<std::string, int> found;
    std::string where;
    int files = 0;
    for (const auto& e : fs::recursive_directory_iterator(root))
    {
        if (!e.is_regular_file())
            continue;
        const auto ext = e.path().extension().string();
        if (ext != ".h" && ext != ".cpp" && ext != ".mm")
            continue;
        ++files;
        const auto rel = fs::relative(e.path(), root).generic_string();
        if (std::find(excluded.begin(), excluded.end(), rel) != excluded.end())
            continue;
        const auto lines = codeLines(rel);
        for (size_t i = 0; i < lines.size(); ++i)
        {
            const auto& l = lines[i];
            const int n = static_cast<int>(std::distance(std::sregex_iterator(l.begin(), l.end(), method),
                                                         std::sregex_iterator()))
                        + static_cast<int>(std::distance(std::sregex_iterator(l.begin(), l.end(), container),
                                                         std::sregex_iterator()));
            if (n > 0)
            {
                found[rel] += n;
                where += "\n  " + rel + ":" + std::to_string(i + 1);
            }
        }
    }
    REQUIRE(files > 50);   // the walk reached src/
    std::string diff;
    for (const auto& [rel, n] : found)
    {
        const auto it = pinned.find(rel);
        const int want = it == pinned.end() ? 0 : it->second;
        if (n != want)
            diff += "\n  " + rel + ": " + std::to_string(n) + " (pinned " + std::to_string(want) + ")";
    }
    for (const auto& [rel, n] : pinned)
        if (found.find(rel) == found.end())
            diff += "\n  " + rel + ": 0 (pinned " + std::to_string(n) + ")";
    INFO("structure-writer sites:" << where);
    INFO("counts that differ from the audited pins:" << (diff.empty() ? std::string(" none") : diff));
    CHECK(diff.empty());
}
