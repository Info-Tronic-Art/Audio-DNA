// test_render_thread_lint -- lane tsan (s-rta-1002; ruling .harmony/.reports/s-rta-0930/ruling-tsan.md amendment 8).
// Case 1 (T0): the GL thread never stores a clip's `playing` intent plainly. Renderer::syncMedia reads the intent
// once, pushes it to the player, and writes the player's state back with a compare-exchange on the value it read
// (src/render/ClipTransportSync.h), so a trigger or a pause landing inside the sync is never overwritten (the F5 /
// F13 lost update: an auto-played video stuck paused). The video AND the sequence branch both call it.
// DEBT (like test_hot_thread_io_lint): TEXTUAL. Line comments are stripped; a store reached through another name is
// not seen.
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cctype>
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
        { "core/DeckCommands.h", 25 },      // every deck / layer / column command body: runFenced; 1 local snapshot.
                                            // 22 -> 25 (fix stage, ruling-bf9b-merge AM-7), all inside runFenced:
                                            // AddDeckCmd undo retireOrEraseDeck in place of decks.erase (+1 -1), its
                                            // redo restoreRetiredDeck (+1); InsertDeckCmd undo retireOrEraseDeck beside
                                            // the kept decks.erase (+1), its redo restoreRetiredDeck (+1)
        { "core/UndoService.cpp", 2 },      // reapRetiredDecks: inside withDeckDetached itself
        { "recording/RoutineEngine.cpp", 2 },   // a local Footprint vector `layers`
        { "ui/ClipCell.cpp", 1 },           // ClipCell::setClip (a UI setter)
        { "ui/ClipCell.h", 1 },
        { "ui/ClipInspector.cpp", 1 },      // ClipInspector::setClip (a UI setter)
        { "ui/ClipInspector.h", 1 },
        { "ui/DeckView.cpp", 2 },           // ClipCell::setClip calls (a UI setter)
        { "ui/InspectorPanel.cpp", 1 },     // ClipInspector::setClip call (a UI setter)
        { "ui/InspectorRepoint.h", 1 },     // ClipInspector::setClip(nullptr) (a UI setter; fix stage, AM-2)
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

namespace
{
// The body of the first function / lambda whose header line contains `header`, from its first '{' to the matching
// '}' (code lines, line comments stripped; braces inside string literals are not expected in these bodies).
std::string bodyAfter(const std::vector<std::string>& lines, const std::string& header, size_t from = 0)
{
    for (size_t i = from; i < lines.size(); ++i)
    {
        if (lines[i].find(header) == std::string::npos)
            continue;
        std::string body;
        int depth = 0;
        bool open = false;
        for (size_t j = i; j < lines.size(); ++j)
        {
            for (char ch : lines[j])
            {
                if (ch == '{') { ++depth; open = true; }
                else if (ch == '}') --depth;
                if (open) body += ch;
                if (open && depth == 0)
                    return body;
            }
            if (open) body += '\n';
        }
        return body;
    }
    return {};
}
} // namespace

// Case 5 (lane bf9b; ruling-bf9b B4d SMOKE + ruling-bf10 H2 relayed in the plan's adoption item 2): a deck switch does
// exactly the index, the renderer's fence token, the grid's cells and the take capture (rulebook R7). The bodies of
// handleDeckSwitch, the tab click's onDeckSwitched handler and DeckView::showDeck -- one level, TEXT only (the behavioural proofs are T1 / K1 / K8) -- name none of the tuple writers or the preview refresh (B4d),
// and none of the MilkDrop / canvas calls (H2: a switch never loads a preset, resizes or releases projectM, never
// changes the canvas).
TEST_CASE("bf9b B4d / H2: a deck switch path touches nothing that plays (smoke, one level)", "[lint][bf9b]")
{
    const std::regex b4d(R"(triggerClip|clearActiveClip|setRuntime|updateRuntime|cancelPending|refreshPreview)");
    const std::regex h2(R"(loadPreset|releaseGL|ProjectM|projectM|outputWidth|outputHeight|setCanvas|[Cc]anvas\s*\()");
    struct Site { const char* file; const char* header; std::string from; };
    const Site sites[] = {
        { "MainComponent.cpp", "void MainComponent::handleDeckSwitch(", "" },
        { "MainComponent.cpp", "deckView_->onDeckSwitched = [", "" },
        { "ui/DeckView.cpp", "void DeckView::showDeck()", "" },
    };
    for (const auto& site : sites)
    {
        const auto lines = codeLines(site.file);
        size_t from = 0;
        if (!site.from.empty())
            for (size_t i = 0; i < lines.size(); ++i)
                if (lines[i].find(site.from) != std::string::npos) { from = i; break; }
        const std::string body = bodyAfter(lines, site.header, from);
        INFO(site.file << " " << site.header << ":\n" << body);
        REQUIRE_FALSE(body.empty());
        CHECK_FALSE(std::regex_search(body, b4d));
        CHECK_FALSE(std::regex_search(body, h2));
    }
}

// Case 6 (lane bf9b S2c; ruling-bf9b amendment 10, B4g, B4a's "after S2c also zero SwitchDeckCmd"): a deck switch is
// never an Undo step (Q4's default). The tab click's onDeckSwitched handler is exactly `handleDeckSwitch(deckIdx);`, so
// every switch entry (tab, REST, OSC, bindings, replay) is the one function K1 / K8 drive live; that function pushes no
// command; no SwitchDeckCmd is left in src/ (code lines, line comments stripped). TEXT only.
TEST_CASE("bf9b B4g: a deck switch is never an Undo step -- the tab click is exactly handleDeckSwitch(deckIdx);",
          "[lint][bf9b]")
{
    namespace fs = std::filesystem;
    const auto mc = codeLines("MainComponent.cpp");
    std::string tab = bodyAfter(mc, "deckView_->onDeckSwitched = [");
    INFO("onDeckSwitched body:\n" << tab);
    tab.erase(std::remove_if(tab.begin(), tab.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }),
              tab.end());
    CHECK(tab == "{handleDeckSwitch(deckIdx);}");

    const std::string sw = bodyAfter(mc, "void MainComponent::handleDeckSwitch(");
    INFO("handleDeckSwitch body:\n" << sw);
    REQUIRE_FALSE(sw.empty());
    CHECK_FALSE(std::regex_search(sw, std::regex(R"(pushCommands|undoManager_|undoService_|Cmd\s*>)")));

    const std::regex name(R"(\bSwitchDeckCmd\b)");
    const fs::path root(AUDIODNA_SRC_DIR);
    std::string hits;
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
        const auto lines = codeLines(rel);
        for (size_t i = 0; i < lines.size(); ++i)
            if (std::regex_search(lines[i], name))
                hits += "\n  " + rel + ":" + std::to_string(i + 1);
    }
    REQUIRE(files > 50);   // the walk reached src/
    INFO("SwitchDeckCmd in src/ code lines:" << (hits.empty() ? std::string(" none") : hits));
    CHECK(hits.empty());
}

// Case 7 (lane bf9b fix stage, s-rta-1003; ruling-bf9b-merge AM-8 = B4h, plus Harmony's adoption item 2). The memory
// fix has three places no unit test can drive, pinned here as TEXT (code lines, line comments stripped):
// (i)   MainComponent.cpp holds exactly ONE `undoService_.onLayerStackMoved =` and its statement calls
//       repointInspectorsAfterStackMove( -- the function cases AS1 / AS2 / AS3 / AS3b / AS4 / AS5 / AS6 drive; and
//       exactly ONE `undoService_.onFencedEdit =` whose statement calls clearClipInspectorIfUnowned( (AS7).
// (ii)  the first call in the body of LayerInspector::setLayer and of ClipInspector::setClip is
//       `forgetScalarBindings();`, and that function forgets all 7 / 6 scalar controls (forgetConnection()).
// (iii) UndoService.cpp: the hand-over lambda holds the one `onLayerStackMoved(` call and the one `onFencedEdit(`
//       call, and is called on both exits of withDeckDetached (`handOver();` twice).
TEST_CASE("bf9b B4h: the stack-move hook is wired to repointInspectorsAfterStackMove, setLayer / setClip forget their "
          "scalar bindings first, and the fence hands over on both exits", "[lint][bf9b]")
{
    auto count = [](const std::vector<std::string>& lines, const std::string& token) {
        int n = 0;
        for (const auto& l : lines)
            for (size_t at = l.find(token); at != std::string::npos; at = l.find(token, at + token.size()))
                ++n;
        return n;
    };
    auto countIn = [](const std::string& text, const std::string& token) {
        int n = 0;
        for (size_t at = text.find(token); at != std::string::npos; at = text.find(token, at + token.size()))
            ++n;
        return n;
    };
    auto squeezed = [](std::string text) {
        text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }),
                   text.end());
        return text;
    };

    SECTION("(i) MainComponent's two hook statements")
    {
        const auto mc = codeLines("MainComponent.cpp");
        CHECK(count(mc, "undoService_.onLayerStackMoved =") == 1);
        const std::string moved = bodyAfter(mc, "undoService_.onLayerStackMoved =");
        INFO("onLayerStackMoved statement:\n" << moved);
        CHECK(countIn(moved, "repointInspectorsAfterStackMove(") == 1);
        CHECK(countIn(moved, "getSelectedLayerIndex()") == 1);
        CHECK(count(mc, "undoService_.onFencedEdit =") == 1);
        const std::string edited = bodyAfter(mc, "undoService_.onFencedEdit =");
        INFO("onFencedEdit statement:\n" << edited);
        CHECK(countIn(edited, "clearClipInspectorIfUnowned(") == 1);
    }
    SECTION("(ii) setLayer / setClip forget first")
    {
        const auto li = codeLines("ui/LayerInspector.cpp");
        const std::string setLayer = squeezed(bodyAfter(li, "void LayerInspector::setLayer("));
        INFO("LayerInspector::setLayer body: " << setLayer);
        CHECK(setLayer.rfind("{forgetScalarBindings();", 0) == 0);
        CHECK(countIn(bodyAfter(li, "void LayerInspector::forgetScalarBindings("), "forgetConnection()") == 7);
        const auto ci = codeLines("ui/ClipInspector.cpp");
        const std::string setClip = squeezed(bodyAfter(ci, "void ClipInspector::setClip("));
        INFO("ClipInspector::setClip body: " << setClip);
        CHECK(setClip.rfind("{forgetScalarBindings();", 0) == 0);
        CHECK(countIn(bodyAfter(ci, "void ClipInspector::forgetScalarBindings("), "forgetConnection()") == 6);
    }
    SECTION("(iii) UndoService's hand-over")
    {
        const auto us = codeLines("core/UndoService.cpp");
        const std::string handOver = bodyAfter(us, "auto handOver = [");
        INFO("hand-over lambda:\n" << handOver);
        REQUIRE_FALSE(handOver.empty());
        CHECK(countIn(handOver, "onLayerStackMoved(") == 1);
        CHECK(count(us, "onLayerStackMoved(") == 1);
        CHECK(countIn(handOver, "onFencedEdit(") == 1);
        CHECK(count(us, "onFencedEdit(") == 1);
        CHECK(count(us, "handOver();") == 2);
    }
}

// Case 8 (lane bf9b fix stage, s-rta-1003; ruling-bf9b-merge AM-12 = B4j, plus Harmony's adoption items 9-11). Boris:
// "The layer strip does not need to show the deck a clip is playing from."; asked whether the deck-tab dot stays:
// "drop"; "I don't wanna see an under removed button at all. We just use control Z."; "We don't need any text
// indicating what has happened or what has happened. [...] Please remove it cleanly and completely." No identifier of
// the strip's source-deck badge, the tab dot, the tab row's Undo Remove button or the load-notice label is left in
// src/ (*.h / *.cpp / *.mm, recursively). WHOLE lines -- comments included: nothing dead left behind.
TEST_CASE("bf9b B4j: no source-deck badge, tab dot, Undo Remove button or load notice identifier left in src/",
          "[lint][bf9b]")
{
    namespace fs = std::filesystem;
    const std::regex badgeAndDot(   // the ruling's B4j pattern, verbatim
        R"(SourceBadge|sourceBadge|onSourceDeckClicked|kBadge|syncTabDots|tabDotShownForTest|kDotColour|dotBounds)");
    const std::regex announcements(   // adoption items 9-11
        R"(undoHint|UndoHint|undoRemoveHint|Undo Remove|loadNotice|LoadNotice|load_notice|\bNoticeLabel\b|Removed deck)");
    const fs::path root(AUDIODNA_SRC_DIR);
    std::string hits;
    int files = 0, count = 0;
    for (const auto& e : fs::recursive_directory_iterator(root))
    {
        if (!e.is_regular_file())
            continue;
        const auto ext = e.path().extension().string();
        if (ext != ".h" && ext != ".cpp" && ext != ".mm")
            continue;
        ++files;
        const auto rel = fs::relative(e.path(), root).generic_string();
        std::ifstream f(e.path());
        REQUIRE(f.good());
        int n = 0;
        for (std::string l; std::getline(f, l);)
        {
            ++n;
            if (std::regex_search(l, badgeAndDot) || std::regex_search(l, announcements))
            {
                ++count;
                hits += "\n  " + rel + ":" + std::to_string(n);
            }
        }
    }
    REQUIRE(files > 50);   // the walk reached src/
    INFO(count << " hit(s) in src/:" << (hits.empty() ? std::string(" none") : hits));
    CHECK(count == 0);
}

// Case 9 (lane bf9b fix stage, s-rta-1003; ruling-bf9b-merge AM-9 = B4i). K5 with Ableton Link ON has no live driver
// (probe-boxes prints it BLOCKED: no Link build, no toggle route), so this is the text evidence that the boxes model
// does not depend on Link: no Link identifier in the model, the render thread, the commands, the deck grid, or in the
// three functions a deck switch and a (queued) trigger run through. TEXT only (code lines, line comments stripped);
// NO count pin on MainComponent.cpp as a whole -- the sync lanes rewrite its Link tick.
TEST_CASE("bf9b B4i: no Link identifier in the model, render, core, the deck grid or the switch / trigger handlers",
          "[lint][bf9b]")
{
    namespace fs = std::filesystem;
    const std::regex link(R"(LinkSync|linkSync_|AUDIODNA_HAS_LINK)");
    const fs::path root(AUDIODNA_SRC_DIR);
    std::string hits;
    int files = 0;
    const auto scan = [&](const std::string& rel)
    {
        ++files;
        const auto lines = codeLines(rel);
        for (size_t i = 0; i < lines.size(); ++i)
            if (std::regex_search(lines[i], link))
                hits += "\n  " + rel + ":" + std::to_string(i + 1);
    };
    for (const char* dir : { "model", "render", "core" })
        for (const auto& e : fs::recursive_directory_iterator(root / dir))
        {
            if (!e.is_regular_file())
                continue;
            const auto ext = e.path().extension().string();
            if (ext == ".h" || ext == ".cpp" || ext == ".mm")
                scan(fs::relative(e.path(), root).generic_string());
        }
    scan("ui/DeckView.cpp");
    REQUIRE(files > 30);   // the walk reached the three directories
    INFO("Link identifiers in src/model, src/render, src/core, src/ui/DeckView.cpp:"
         << (hits.empty() ? std::string(" none") : hits));
    CHECK(hits.empty());

    const auto mc = codeLines("MainComponent.cpp");
    for (const char* header : { "void MainComponent::handleDeckSwitch(", "void MainComponent::handleClipTrigger(",
                                "void MainComponent::handleColumnTrigger(" })
    {
        const std::string body = bodyAfter(mc, header);
        INFO(header << ":\n" << body);
        REQUIRE_FALSE(body.empty());
        CHECK_FALSE(std::regex_search(body, link));
    }
}

// Case 10 (lane bf9b fix stage 5, s-rta-1003; visual gate B7 problem 5 = B4k). MainComponent::removeDeck is not
// reachable headless (case AS8 of test_show_model drives the command and the hand-over under it), so its two
// statements that took the Layer tab off its layer are pinned as TEXT (code lines, line comments stripped): its body
// holds no `setLayer(` (the Layer inspector is never emptied by a Remove Deck) and no `selectLayer(` (the selected
// layer row and its highlight stay, also when the removed deck is the one shown), and still empties the Clip
// inspector (its effect scope names a deck index the erase shifts).
TEST_CASE("bf9b B4k: MainComponent::removeDeck never empties the Layer inspector nor drops the selected layer row",
          "[lint][bf9b]")
{
    const auto mc = codeLines("MainComponent.cpp");
    const std::string body = bodyAfter(mc, "void MainComponent::removeDeck(");
    INFO("MainComponent::removeDeck:\n" << body);
    REQUIRE(body.find("RemoveDeckCmd") != std::string::npos);   // the right function, whole
    CHECK(body.find("setLayer(") == std::string::npos);
    CHECK(body.find("selectLayer(") == std::string::npos);
    CHECK(body.find("getClipInspector().setClip(nullptr)") != std::string::npos);
}
