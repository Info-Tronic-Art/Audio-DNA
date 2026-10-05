// test_show_file -- lane one-save S1 (ruling-one-save A-1, A-3, A-4; rows SF-1..SF-10): the show file's version and
// its two extra blocks. Drives the real Composition (toVar / fromVar / saveToFile / loadFromFile), the real
// showfile::versionOf and real temp files; the fixtures are tests/OneSaveFixture.h.
#include <catch2/catch_test_macros.hpp>
#include "OneSaveFixture.h"
#include "core/ShowFile.h"
#include <limits>

using namespace OneSaveFixture;

namespace
{
juce::var parseFile(const juce::File& f)
{
    return juce::JSON::parse(f.loadFileAsString());
}

juce::var keysBlock()
{
    return juce::JSON::parse(R"({"bindings":[{"inputType":0,"keyCode":81,"action":0}],"version":1})");
}

juce::var layoutBlock()
{
    return juce::JSON::parse(R"({"deckDividerY":300,"vDividerFrac":[0.3,0.55,0.8]})");
}

juce::String text(const juce::var& v) { return juce::JSON::toString(v); }
} // namespace

TEST_CASE("showfile: toVar writes version 2 first and no outputDisplay; saveToFile adds keys and layout", "[showfile]")
{
    const Composition show = currentShow();

    const juce::var v = show.toVar();
    const auto keys = keysOf(v);
    REQUIRE_FALSE(keys.empty());
    CHECK(keys.front() == "version");
    CHECK(v["version"].isInt());
    CHECK(static_cast<int>(v["version"]) == 2);
    CHECK(showfile::kShowVersion == 2);
    CHECK_FALSE(v.hasProperty("outputDisplay"));
    CHECK_FALSE(v.hasProperty("keys"));       // the two blocks are the file writer's, never toVar's
    CHECK_FALSE(v.hasProperty("layout"));

    TempDir t;
    SECTION("empty extras: both blocks are still written, as empty objects")
    {
        const juce::File f = t.dir.getChildFile("s.json");
        REQUIRE(show.saveToFile(f, showfile::ShowExtras{}));
        const juce::var saved = parseFile(f);
        const auto fileKeys = keysOf(saved);
        REQUIRE(fileKeys.size() == keys.size() + 2);
        CHECK(fileKeys.front() == "version");
        CHECK(static_cast<int>(saved["version"]) == 2);
        CHECK(fileKeys[fileKeys.size() - 2] == "keys");
        CHECK(fileKeys.back() == "layout");
        REQUIRE(saved["keys"].getDynamicObject() != nullptr);
        CHECK(saved["keys"].getDynamicObject()->getProperties().size() == 0);
        REQUIRE(saved["layout"].getDynamicObject() != nullptr);
        CHECK(saved["layout"].getDynamicObject()->getProperties().size() == 0);
        CHECK_FALSE(saved.hasProperty("outputDisplay"));
        // The file's very first key, in its text.
        CHECK(f.loadFileAsString().removeCharacters(" \r\n\t").startsWith("{\"version\":2,"));
    }
    SECTION("given extras are written as they are")
    {
        const juce::File f = t.dir.getChildFile("s.json");
        REQUIRE(show.saveToFile(f, showfile::ShowExtras{ keysBlock(), layoutBlock() }));
        const juce::var saved = parseFile(f);
        CHECK(text(saved["keys"]) == text(keysBlock()));
        CHECK(text(saved["layout"]) == text(layoutBlock()));
        CHECK(keysOf(saved).front() == "version");
    }
}

TEST_CASE("showfile: keys and layout round-trip through saveToFile / loadFromFile as loadedExtras, and toVar never "
          "carries them", "[showfile]")
{
    TempDir t;
    const juce::File f = t.dir.getChildFile("s.json");
    REQUIRE(currentShow().saveToFile(f, showfile::ShowExtras{ keysBlock(), layoutBlock() }));

    Composition loaded;
    REQUIRE(loaded.loadFromFile(f));
    CHECK(loaded.loadedVersion == 2);
    CHECK(text(loaded.loadedExtras.keys) == text(keysBlock()));
    CHECK(text(loaded.loadedExtras.layout) == text(layoutBlock()));
    CHECK(text(loaded.toVar()) == text(currentShow().toVar()));   // the show itself, whole
    CHECK_FALSE(loaded.toVar().hasProperty("keys"));
    CHECK_FALSE(loaded.toVar().hasProperty("layout"));

    SECTION("taking the extras hands them over once and clears them")
    {
        const showfile::ShowExtras taken = loaded.takeLoadedExtras();
        CHECK(text(taken.keys) == text(keysBlock()));
        CHECK(text(taken.layout) == text(layoutBlock()));
        CHECK(loaded.loadedExtras.keys.isVoid());
        CHECK(loaded.loadedExtras.layout.isVoid());
    }
    SECTION("a file with neither block gives void extras, also after a file that had them")
    {
        const juce::File bare = t.writeVar("bare.json", currentShow().toVar());
        REQUIRE(loaded.loadFromFile(bare));
        CHECK(loaded.loadedExtras.keys.isVoid());
        CHECK(loaded.loadedExtras.layout.isVoid());
    }
    SECTION("a new show forgets them")
    {
        loaded.initDefault();
        CHECK(loaded.loadedExtras.keys.isVoid());
        CHECK(loaded.loadedExtras.layout.isVoid());
        CHECK(loaded.loadedVersion == 2);
    }
}

TEST_CASE("showfile: an old-shape file reads as version 0 and converts exactly as before (decks, layers, the one note)",
          "[showfile]")
{
    const juce::var old = oldShowVar();
    REQUIRE_FALSE(old.hasProperty("version"));
    REQUIRE_FALSE(old.hasProperty("layers"));
    CHECK(showfile::versionOf(old) == 0);

    // "As before" = what the converter itself gives for this file.
    std::vector<Layer> wantLayers;
    std::vector<Deck> wantDecks;
    uint32_t nextId = 100;
    const std::string wantNote = ShowMigration::convertShow(old, wantLayers, wantDecks, nextId);
    REQUIRE(wantLayers.size() == 3);
    REQUIRE(wantNote.rfind("old show converted:", 0) == 0);

    TempDir t;
    Composition c;
    REQUIRE(c.loadFromFile(t.writeVar("old.json", old)));
    CHECK(c.loadedVersion == 0);
    CHECK(c.migrationNote == wantNote);
    CHECK(c.migrationNote.find('\n') == std::string::npos);                       // ONE note
    CHECK(c.migrationNote.find("Deck 2 row 3: settings dropped") != std::string::npos);
    REQUIRE(c.layers.size() == wantLayers.size());
    for (size_t i = 0; i < wantLayers.size(); ++i)
        CHECK(text(c.layers[i].toVar()) == text(wantLayers[i].toVar()));
    CHECK(c.layers[1].opacity == 0.8f);                                           // Deck 1's row settings win
    CHECK(c.layers[2].opacity == 1.0f);
    REQUIRE(c.decks.size() == 2);
    CHECK(c.decks[0].name == "Deck 1");
    CHECK(c.decks[1].name == "Deck 2");
    REQUIRE(c.decks[1].getClip(1, 1) != nullptr);
    CHECK(c.decks[1].getClip(1, 1)->name == "d2r2b");
    CHECK(c.loadedExtras.keys.isVoid());                                          // an old file has neither block
    CHECK(c.loadedExtras.layout.isVoid());
}

TEST_CASE("showfile: a version-less bf9b file reads as version 1 and loads unchanged", "[showfile]")
{
    const juce::var v1 = v1ShowVar();
    REQUIRE_FALSE(v1.hasProperty("version"));
    CHECK(showfile::versionOf(v1) == 1);

    TempDir t;
    Composition c;
    REQUIRE(c.loadFromFile(t.writeVar("v1.json", v1)));
    CHECK(c.loadedVersion == 1);
    CHECK(c.migrationNote.empty());
    juce::var again = c.toVar();
    again.getDynamicObject()->removeProperty("version");
    CHECK(text(again) == text(v1));                                               // every key it had, unchanged
}

TEST_CASE("showfile: a version-2 file is never converted by key presence", "[showfile]")
{
    // The old-shape fixture's content under "version": 2 -- no top-level "layers", rows that carry layer settings.
    juce::var v = oldShowVar();
    v.getDynamicObject()->setProperty("version", 2);
    REQUIRE(ShowMigration::isLegacyShow(v));                                      // the presence test WOULD say "old"
    CHECK(showfile::versionOf(v) == 2);

    Composition c;
    c.fromVar(v);
    CHECK(c.loadedVersion == 2);
    CHECK(c.migrationNote.empty());                                               // no conversion ran
    REQUIRE(c.layers.size() == 3);                                                // one default layer per deck row
    for (const auto& l : c.layers)
        CHECK(l.opacity == 1.0f);                                                 // not row 2's 0.8 from the rows
    REQUIRE(c.decks.size() == 2);
    REQUIRE(c.decks[0].getClip(0, 0) != nullptr);                                 // the clips are still read
    CHECK(c.decks[0].getClip(0, 0)->name == "d1r1");
}

TEST_CASE("showfile: a broken keys or layout block still loads the show; loadedExtras carry the raw vars", "[showfile]")
{
    juce::var v = currentShow().toVar();
    v.getDynamicObject()->setProperty("keys", "x");
    v.getDynamicObject()->setProperty("layout", juce::JSON::parse(R"({"vDividerFrac":[0,0,0]})"));

    TempDir t;
    Composition c;
    REQUIRE(c.loadFromFile(t.writeVar("broken.json", v)));
    CHECK(c.loadedVersion == 2);
    REQUIRE(c.decks.size() == 2);
    CHECK(c.decks[1].name == "Deck 2");
    CHECK(c.layers.size() == 3);
    CHECK(c.loadedExtras.keys.isString());
    CHECK(c.loadedExtras.keys.toString() == "x");
    CHECK(text(c.loadedExtras.layout) == text(juce::JSON::parse(R"({"vDividerFrac":[0,0,0]})")));
}

TEST_CASE("showfile: a version-3 file loads best-effort, unknown keys ignored, loadedVersion 3", "[showfile]")
{
    const juce::var v3 = v3ShowVar();
    CHECK(showfile::versionOf(v3) == 3);

    TempDir t;
    Composition c;
    REQUIRE(c.loadFromFile(t.writeVar("v3.json", v3)));
    CHECK(c.loadedVersion == 3);
    CHECK(c.migrationNote.empty());
    REQUIRE(c.decks.size() == 2);
    CHECK(c.layers.size() == 3);
    CHECK(c.layers[1].opacity == 0.6f);
    const juce::var out = c.toVar();
    CHECK_FALSE(out.hasProperty("futureThing"));                                  // ignored, and not written back
    CHECK(static_cast<int>(out["version"]) == 2);                                 // this build writes its own version

    // A later version WITHOUT "layers" is not converted by key presence either.
    juce::var bare = v3;
    bare.getDynamicObject()->removeProperty("layers");
    Composition d;
    d.fromVar(bare);
    CHECK(d.loadedVersion == 3);
    CHECK(d.migrationNote.empty());
}

TEST_CASE("showfile: a file that has outputDisplay loads; the six other old keys are still written and read", "[showfile]")
{
    Composition src = currentShow();
    src.crossfaderPhase = 0.25f;
    src.crossfaderBlendMode = Composition::CrossfaderBlendMode::Multiply;
    src.crossfaderBehaviour = Composition::CrossfaderBehaviour::Smooth;
    src.crossfaderCurve = Composition::CrossfaderCurve::SCurve;
    src.smartAutopilotEnabled = true;
    src.genrePresetNames[3] = "genre three";

    juce::var v = src.toVar();
    for (const char* key : { "crossfaderPhase", "crossfaderBlendMode", "crossfaderBehaviour", "crossfaderCurve",
                             "genrePresetNames", "smartAutopilotEnabled" })
    {
        INFO(key);
        CHECK(v.hasProperty(key));
    }
    CHECK_FALSE(v.hasProperty("outputDisplay"));

    v.getDynamicObject()->setProperty("outputDisplay", 3);    // a file from before this lane
    TempDir t;
    Composition c;
    REQUIRE(c.loadFromFile(t.writeVar("with-output-display.json", v)));
    REQUIRE(c.decks.size() == 2);
    CHECK(c.outputWidth == 1280);
    CHECK(c.outputHeight == 720);
    CHECK(c.crossfaderPhase == 0.25f);
    CHECK(c.crossfaderBlendMode == Composition::CrossfaderBlendMode::Multiply);
    CHECK(c.crossfaderBehaviour == Composition::CrossfaderBehaviour::Smooth);
    CHECK(c.crossfaderCurve == Composition::CrossfaderCurve::SCurve);
    CHECK(c.smartAutopilotEnabled);
    CHECK(c.genrePresetNames[3] == "genre three");
    CHECK_FALSE(c.toVar().hasProperty("outputDisplay"));      // the key is ignored and never written again
}

TEST_CASE("showfile: old show -> save -> load -> save gives two equal files that carry version 2", "[showfile]")
{
    TempDir t;
    Composition first;
    REQUIRE(first.loadFromFile(t.writeVar("old.json", oldShowVar())));
    REQUIRE(first.loadedVersion == 0);

    const juce::File save1 = t.dir.getChildFile("save1.json");
    REQUIRE(first.saveToFile(save1, showfile::ShowExtras{}));
    Composition second;
    REQUIRE(second.loadFromFile(save1));
    CHECK(second.loadedVersion == 2);
    CHECK(second.migrationNote.empty());                      // a version-2 file: nothing converted
    const juce::File save2 = t.dir.getChildFile("save2.json");
    REQUIRE(second.saveToFile(save2, second.loadedExtras));

    CHECK(bytesOf(save1) == bytesOf(save2));
    const juce::var v = parseFile(save1);
    CHECK(keysOf(v).front() == "version");
    CHECK(static_cast<int>(v["version"]) == 2);
    CHECK(v["layers"].isArray());
    const juce::String txt = save1.loadFileAsString();
    CHECK_FALSE(txt.contains("persistent"));
    CHECK_FALSE(txt.contains("globalTransitionSpeed"));
    CHECK_FALSE(txt.contains("outputDisplay"));
}

TEST_CASE("showfile: a version of 0, 1, -3, \"x\", true or 2.5 is no version -- with layers the file reads as 1, "
          "without as 0", "[showfile]")
{
    const std::vector<juce::var> notVersions = { juce::var(0), juce::var(1), juce::var(-3), juce::var("x"),
                                                 juce::var(true), juce::var(2.5), juce::var("2"), juce::var(2.0),
                                                 juce::var(false), juce::var(juce::int64(-5)), juce::var() };
    for (const auto& value : notVersions)
    {
        INFO("version = " << juce::JSON::toString(value, true).toStdString());
        juce::var withLayers = v1ShowVar();
        withLayers.getDynamicObject()->setProperty("version", value);
        CHECK(showfile::versionOf(withLayers) == 1);

        juce::var without = oldShowVar();
        without.getDynamicObject()->setProperty("version", value);
        CHECK(showfile::versionOf(without) == 0);

        // ... and the loader follows: the old-shape file is converted, whatever that value says.
        Composition c;
        c.fromVar(without);
        CHECK(c.loadedVersion == 0);
        CHECK_FALSE(c.migrationNote.empty());
        CHECK(c.layers.size() == 3);
        CHECK(c.layers[1].opacity == 0.8f);
    }

    // What IS a version: an integer of 2 or more, also when the JSON parser hands it over as a 64-bit integer.
    for (const juce::int64 n : { juce::int64(2), juce::int64(3), juce::int64(7) })
    {
        juce::var a = oldShowVar();
        a.getDynamicObject()->setProperty("version", static_cast<int>(n));
        CHECK(showfile::versionOf(a) == static_cast<int>(n));
        juce::var b = oldShowVar();
        b.getDynamicObject()->setProperty("version", n);
        CHECK(showfile::versionOf(b) == static_cast<int>(n));
    }
    CHECK(showfile::versionOf(juce::JSON::parse(R"({"version": 2, "decks": []})")) == 2);
    CHECK(showfile::versionOf(juce::JSON::parse(R"({"version": 2.0, "decks": []})")) == 0);
    CHECK(showfile::versionOf(juce::JSON::parse(R"({"version": "2", "layers": [], "decks": []})")) == 1);

    // An integer too large for an int is still a version of 2 or more: it is never cut down to 0 (an old-shape
    // conversion), to 2 (no copy before an overwrite) or to a negative number.
    for (const char* big : { "2147483648", "3000000000", "4294967296", "4294967298" })
    {
        INFO("version = " << big);
        const juce::var parsed = juce::JSON::parse("{\"version\": " + juce::String(big) + ", \"decks\": []}");
        REQUIRE(parsed.getDynamicObject()->getProperty("version").isInt64());
        CHECK(showfile::versionOf(parsed) == std::numeric_limits<int>::max());
        CHECK(showfile::backupTagFor(parsed) == "v2147483647");

        juce::var oldShape = oldShowVar();
        oldShape.getDynamicObject()->setProperty("version", juce::var(juce::String(big).getLargeIntValue()));
        Composition c;
        c.fromVar(oldShape);
        CHECK(c.loadedVersion == std::numeric_limits<int>::max());
        CHECK(c.migrationNote.empty());                   // stated 2 or more: not converted by key presence
    }

    // Not an object at all: no version to state, and nothing of the old shape either.
    CHECK(showfile::versionOf(juce::var()) == 1);
    CHECK(showfile::versionOf(juce::var("text")) == 1);
}
