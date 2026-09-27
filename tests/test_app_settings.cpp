// test_app_settings -- s-rta-0927 outputs-c3 = plan5 slice C3 (.harmony/.reports/s-rta-0926b/plan5-final.md sections
// 9, 10.1, 13 R8): settings.json is ONE object with independent keys, written read-modify-write, so the MilkDrop
// preset folder and the output set never clobber each other. Before C3 the MilkDrop writer
// (MainComponent::saveMilkDropPresetDirSetting) replaced the whole file with a one-key object -- it would have erased
// "outputs" (and vice versa). The code under test is src/model/AppSettings.cpp with the app's own key constants and
// output::wantedToVar -- the calls MainComponent and OutputManager make. Every case uses its own temp file; the
// user's real settings file is never touched.
#include <catch2/catch_test_macros.hpp>
#include "model/AppSettings.h"
#include "output/OutputTargets.h"

namespace
{
struct TempSettings
{
    juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getNonexistentChildFile("audiodna-test-app-settings", "", false);
    juce::File file = dir.getChildFile("Audio-DNA").getChildFile("settings.json");
    TempSettings() { REQUIRE(dir.createDirectory()); }
    ~TempSettings() { dir.deleteRecursively(); }
    juce::var parsed() const { return juce::JSON::parse(file.loadFileAsString()); }
};

const output::DisplayInfo kLaptop { 0, 0, 1728, 1117, 2.0, true };
const output::DisplayInfo kProjector { 1728, 0, 1920, 1080, 1.0, false };
} // namespace

TEST_CASE("AppSettings: update is read-modify-write -- two keys, both kept", "[app_settings]")
{
    TempSettings t;
    const AppSettings s(t.file);
    REQUIRE(s.update("a", 1));
    REQUIRE(s.update("b", 2));
    const auto root = t.parsed();
    REQUIRE(root.getDynamicObject() != nullptr);
    CHECK(static_cast<int>(root["a"]) == 1);
    CHECK(static_cast<int>(root["b"]) == 2);
    CHECK(static_cast<int>(s.read("a")) == 1);
    REQUIRE(s.update("a", 3));   // replacing one key keeps the other
    CHECK(static_cast<int>(s.read("a")) == 3);
    CHECK(static_cast<int>(s.read("b")) == 2);
}

TEST_CASE("AppSettings: a missing file or key reads as void, and reading never creates the file", "[app_settings]")
{
    TempSettings t;
    const AppSettings s(t.file);
    CHECK(s.read("milkDropPresetDir").isVoid());
    CHECK_FALSE(t.file.exists());
    REQUIRE(s.update("a", 1));
    CHECK(s.read("missing").isVoid());
}

TEST_CASE("AppSettings: a corrupt file reads as empty and the next update rewrites a valid object", "[app_settings]")
{
    for (const char* bad : { "{not json", "[1, 2, 3]", "", "\"a string\"" })
    {
        TempSettings t;
        REQUIRE(t.file.getParentDirectory().createDirectory());
        REQUIRE(t.file.replaceWithText(bad));
        const AppSettings s(t.file);
        INFO("file text: " << bad);
        CHECK(s.read("a").isVoid());
        REQUIRE(s.update("a", 1));
        const auto root = t.parsed();
        REQUIRE(root.getDynamicObject() != nullptr);
        CHECK(static_cast<int>(root["a"]) == 1);
    }
}

TEST_CASE("AppSettings: the MilkDrop folder and the output set coexist, whichever is written last", "[app_settings]")
{
    TempSettings t;
    const AppSettings s(t.file);
    // The MilkDrop writer first (Preferences > Video), then the outputs writer (OutputManager), then MilkDrop again.
    REQUIRE(s.update(AppSettings::kMilkDropPresetDir, "/Users/x/MilkDrop"));
    REQUIRE(s.update(AppSettings::kOutputs, output::wantedToVar({ kLaptop, kProjector })));
    CHECK(s.read(AppSettings::kMilkDropPresetDir).toString() == "/Users/x/MilkDrop");
    REQUIRE(s.update(AppSettings::kMilkDropPresetDir, "/Users/x/Other"));
    const auto outputs = output::wantedFromVar(s.read(AppSettings::kOutputs));
    REQUIRE(outputs.size() == 2);
    CHECK(outputs[0] == kLaptop);
    CHECK(outputs[1] == kProjector);
    CHECK(s.read(AppSettings::kMilkDropPresetDir).toString() == "/Users/x/Other");
    // The keys are the ones the app has always used on disk.
    CHECK(juce::String(AppSettings::kMilkDropPresetDir) == "milkDropPresetDir");
    CHECK(juce::String(AppSettings::kOutputs) == "outputs");
}

TEST_CASE("AppSettings: keys this build does not know survive every write", "[app_settings]")
{
    TempSettings t;
    REQUIRE(t.file.getParentDirectory().createDirectory());
    REQUIRE(t.file.replaceWithText(R"({"futureKey": {"n": 7}, "milkDropPresetDir": "/p"})"));
    const AppSettings s(t.file);
    REQUIRE(s.update(AppSettings::kOutputs, output::wantedToVar({})));
    const auto root = t.parsed();
    CHECK(static_cast<int>(root["futureKey"]["n"]) == 7);
    CHECK(root["milkDropPresetDir"].toString() == "/p");
    CHECK(root["outputs"]["targets"].isArray());
}

TEST_CASE("AppSettings: the default file is <userApplicationDataDirectory>/Audio-DNA/settings.json", "[app_settings]")
{
    const auto f = AppSettings::defaultFile();
    CHECK(f.getFileName() == "settings.json");
    CHECK(f.getParentDirectory().getFileName() == "Audio-DNA");
    CHECK(f.getParentDirectory().getParentDirectory()
          == juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory));
}
