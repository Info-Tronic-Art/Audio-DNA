#include <catch2/catch_test_macros.hpp>
#include "sources/ProjectMPresetManager.h"
#include <juce_core/juce_core.h>

// S166-FAV: ProjectMPresetManager::saveUserData()/loadUserData() were fully
// implemented but had ZERO call sites anywhere in the app — favorites and
// user presets never actually persisted across a restart. The real fix
// (wiring these into MainComponent's ctor and its favorite-toggle callback,
// see src/MainComponent.cpp) can't be exercised headlessly here: MainComponent
// requires a live GL context/GUI to construct, and this build+ctest-only
// gate is explicitly barred from launching the app. What CAN be tested at
// this layer, with real file I/O and real scanned presets (no mocks): the
// persistence CONTRACT that wiring relies on — a favorited preset survives
// a save + reload against a fresh manager instance (simulating an app
// restart), and a missing/malformed file is safe. See this lane's builder
// report for how "does MainComponent actually call these" is verified
// (code-reading citation, not an automated test — a real coverage gap,
// flagged there rather than papered over).

namespace
{
// Real .milk files on disk (not mocked) so scanDirectory()/rescan() populate
// genuine PresetInfo entries, exactly like a real preset folder would.
// Each TEST_CASE gets its own uniquely-named, freshly-cleared directory —
// catch_discover_tests runs each TEST_CASE as a separate process invocation
// of the same executable, so a shared directory would leak .milk files
// written by one test case into another's scan (caught live: the malformed-
// file test saw getPresetCount()==4, not 1, before this fix — leftover
// "Alpha"/"Beta"/"Gamma" from the round-trip test's shared dir).
juce::File presetTestDir(const juce::String& suffix)
{
    auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getChildFile("projectm_preset_manager_test_presets_" + suffix);
    dir.deleteRecursively();
    dir.createDirectory();
    return dir;
}

void writePresetFiles(const juce::File& dir, const juce::StringArray& names)
{
    for (const auto& name : names)
        dir.getChildFile(name + ".milk").replaceWithText("[preset]\nfRating=1\n");
}

juce::File userDataTestFile(const juce::String& suffix)
{
    auto f = juce::File::getSpecialLocation(juce::File::tempDirectory)
                 .getChildFile("projectm_preset_manager_test_userdata_" + suffix + ".json");
    f.deleteFile();
    return f;
}

// Finds a preset by display name (mirrors saveUserData()'s own by-name
// favorites lookup) so tests don't depend on scan order.
int indexOfPreset(const ProjectMPresetManager& mgr, const std::string& name)
{
    for (int i = 0; i < mgr.getPresetCount(); ++i)
        if (mgr.getPreset(i)->name == name)
            return i;
    return -1;
}
} // namespace

TEST_CASE("S166-FAV: a favorited preset survives save + reload in a fresh manager instance",
          "[preset-manager][persistence]")
{
    auto dir = presetTestDir("roundtrip");
    writePresetFiles(dir, {"Alpha", "Beta", "Gamma"});
    auto userDataFile = userDataTestFile("roundtrip");

    // "Session 1": scan, favorite "Beta", save.
    {
        ProjectMPresetManager mgr;
        mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
        mgr.rescan();
        REQUIRE(mgr.getPresetCount() == 3);

        int betaIdx = indexOfPreset(mgr, "Beta");
        REQUIRE(betaIdx >= 0);
        mgr.toggleFavorite(betaIdx);
        REQUIRE(mgr.getFavorites().size() == 1);

        mgr.saveUserData(userDataFile.getFullPathName().toStdString());
        REQUIRE(userDataFile.existsAsFile());
    }

    // "Session 2": a brand-new instance (simulates an app restart) rescans
    // the same directory (favorite=false by construction, like any fresh
    // scan) then loads the saved user data.
    {
        ProjectMPresetManager mgr;
        mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
        mgr.rescan();
        REQUIRE(mgr.getFavorites().empty()); // sanity: a fresh scan starts with none

        mgr.loadUserData(userDataFile.getFullPathName().toStdString());

        auto favorites = mgr.getFavorites();
        REQUIRE(favorites.size() == 1);
        REQUIRE(favorites[0]->name == "Beta");

        int alphaIdx = indexOfPreset(mgr, "Alpha");
        REQUIRE(alphaIdx >= 0);
        REQUIRE_FALSE(mgr.getPreset(alphaIdx)->favorite);
    }

    userDataFile.deleteFile();
}

TEST_CASE("S166-FAV: loading a missing user-data file is a silent no-op (normal on first run)",
          "[preset-manager][persistence]")
{
    auto dir = presetTestDir("missing");
    writePresetFiles(dir, {"Solo"});
    auto missingFile = userDataTestFile("missing");
    REQUIRE_FALSE(missingFile.existsAsFile());

    ProjectMPresetManager mgr;
    mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
    mgr.rescan();
    int countBefore = mgr.getPresetCount();

    mgr.loadUserData(missingFile.getFullPathName().toStdString());

    REQUIRE(mgr.getPresetCount() == countBefore);
    REQUIRE(mgr.getFavorites().empty());
}

TEST_CASE("S166-FAV: loading a malformed/truncated user-data file does not crash and sets no favorites",
          "[preset-manager][persistence]")
{
    auto dir = presetTestDir("malformed");
    writePresetFiles(dir, {"Solo"});

    SECTION("truncated JSON")
    {
        auto badFile = userDataTestFile("truncated");
        badFile.replaceWithText(R"({"favorites": ["Solo)"); // cut off mid-string, no closing

        ProjectMPresetManager mgr;
        mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
        mgr.rescan();

        REQUIRE_NOTHROW(mgr.loadUserData(badFile.getFullPathName().toStdString()));
        REQUIRE(mgr.getFavorites().empty());
        REQUIRE(mgr.getPresetCount() == 1); // presets_ untouched, nothing wiped

        badFile.deleteFile();
    }

    SECTION("valid JSON, wrong shape (favorites is not an array)")
    {
        auto badFile = userDataTestFile("wrongshape");
        badFile.replaceWithText(R"({"favorites": "Solo", "userPresets": 42})");

        ProjectMPresetManager mgr;
        mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
        mgr.rescan();

        REQUIRE_NOTHROW(mgr.loadUserData(badFile.getFullPathName().toStdString()));
        REQUIRE(mgr.getFavorites().empty());
        REQUIRE(mgr.getUserPresets().empty());

        badFile.deleteFile();
    }
}

TEST_CASE("S166-FAV: saveUserData() writes the documented favorites/userPresets JSON schema",
          "[preset-manager][persistence]")
{
    auto dir = presetTestDir("schema");
    writePresetFiles(dir, {"Alpha", "Beta"});
    auto userDataFile = userDataTestFile("schema");

    ProjectMPresetManager mgr;
    mgr.setPresetDirectories({dir.getFullPathName().toStdString()});
    mgr.rescan();
    int alphaIdx = indexOfPreset(mgr, "Alpha");
    REQUIRE(alphaIdx >= 0);
    mgr.toggleFavorite(alphaIdx);

    mgr.saveUserData(userDataFile.getFullPathName().toStdString());

    auto parsed = juce::JSON::parse(userDataFile.loadFileAsString());
    auto* root = parsed.getDynamicObject();
    REQUIRE(root != nullptr);

    auto* favArray = root->getProperty("favorites").getArray();
    REQUIRE(favArray != nullptr);
    REQUIRE(favArray->size() == 1);
    REQUIRE((*favArray)[0].toString() == "Alpha");

    // Nothing in the codebase sets userPreset yet (no public setter exists
    // on ProjectMPresetManager — see this lane's builder report) — it
    // should round-trip as an empty array, not crash or fabricate entries.
    auto* userArray = root->getProperty("userPresets").getArray();
    REQUIRE(userArray != nullptr);
    REQUIRE(userArray->isEmpty());

    userDataFile.deleteFile();
}
