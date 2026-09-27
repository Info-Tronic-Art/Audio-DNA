// test_comp_decks_browser -- decks-followup ITEM 2: the Compositions-browser library's Decks
// section must list only files that load as a v2 deck (Deck::toVar()'s shape: a top-level "layers"
// key). A legacy v1 PresetManager::saveDeck file ("type":"deck", audioFile/fx/slots, no "layers")
// lives in the exact same directory on disk (~/Library/AudioDNA/Decks collides case-insensitively
// with the browser's own decks dir on APFS -- decks.md UNKNOWNS) and cannot be loaded by anything in
// the app (appendDeckFromFile's shape check wants "layers"; PresetManager::loadDeck needs a caller
// that already owns a DeckState/EffectChain/MappingEngine, which the browser does not have). Fixture
// files are written to a temp dir -- the real ~/Library/AudioDNA/Decks is never touched.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/CompDecksBrowser.h"

namespace
{
    juce::File makeTempDir()
    {
        auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("audiodna_test_comp_decks_browser_"
                                     + juce::String(juce::Random::getSystemRandom().nextInt(1000000)));
        dir.createDirectory();
        return dir;
    }
}

TEST_CASE("CompDecksBrowser::isV2DeckFile accepts a v2 deck file (top-level \"layers\")",
          "[compdecksbrowser][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto dir = makeTempDir();

    auto v2 = dir.getChildFile("A.json");
    v2.replaceWithText(R"({"name":"A","id":0,"numColumns":12,"layers":[]})");

    CHECK(CompDecksBrowser::isV2DeckFile(v2));

    dir.deleteRecursively();
}

TEST_CASE("CompDecksBrowser::isV2DeckFile rejects a legacy v1 PresetManager::saveDeck file "
          "(\"type\":\"deck\", no \"layers\") -- RED on main: it was listed unconditionally",
          "[compdecksbrowser][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto dir = makeTempDir();

    // Exactly PresetManager::saveDeck's shape (src/ui/PresetManager.cpp).
    auto legacy = dir.getChildFile("feafeda.deck.json");
    legacy.replaceWithText(
        R"({"type":"deck","audioFile":"","imageFile":"","imageFolderPath":"",)"
        R"("slideshowBeatsPerImage":8,"beatRandomCount":4,"beatRandomEnabled":false,)"
        R"("audioSourceMode":1,"viewportResolution":0,"outputDisplay":1,"inputGain":1.0,)"
        R"("masterVideoLevel":1.0,"showAudioPanel":true,"showFxPanel":true,"showWavePanel":true,)"
        R"("showKeysPanel":true,"showPresetsPanel":true,"slots":[],"fx":{}})");

    CHECK_FALSE(CompDecksBrowser::isV2DeckFile(legacy));

    dir.deleteRecursively();
}

TEST_CASE("CompDecksBrowser::isV2DeckFile rejects a non-existent/unparseable file",
          "[compdecksbrowser][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto dir = makeTempDir();

    CHECK_FALSE(CompDecksBrowser::isV2DeckFile(dir.getChildFile("missing.json")));

    auto garbage = dir.getChildFile("garbage.json");
    garbage.replaceWithText("not json at all");
    CHECK_FALSE(CompDecksBrowser::isV2DeckFile(garbage));

    dir.deleteRecursively();
}

TEST_CASE("A directory holding one legacy v1 file and one v2 deck file: the v2-shape filter keeps "
          "only the v2 file (the exact filter scanForFiles applies to the Decks section)",
          "[compdecksbrowser][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    auto dir = makeTempDir();

    auto legacy = dir.getChildFile("old.deck.json");
    legacy.replaceWithText(R"({"type":"deck","audioFile":"","fx":{},"slots":[]})");
    auto v2 = dir.getChildFile("B.json");
    v2.replaceWithText(R"({"name":"B","id":1,"numColumns":12,"layers":[]})");

    auto files = dir.findChildFiles(juce::File::findFiles, false, "*.json");
    files.sort();

    int v2Count = 0, legacyCount = 0;
    for (auto& f : files)
    {
        if (CompDecksBrowser::isV2DeckFile(f))
            ++v2Count;
        else
            ++legacyCount;
    }
    CHECK(v2Count == 1);
    CHECK(legacyCount == 1);   // RED on main: this file was still listed (no filter existed)

    dir.deleteRecursively();
}
