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
#ifndef _WIN32
#include <csignal>
#include <sys/resource.h>
#endif

namespace
{
struct TempSettings
{
    // Lane one-save S1: a name of its own per PROCESS (ctest runs the cases as parallel processes; two asking for "the
    // next free name" at once got the same folder, and the new cases count the folder's entries).
    juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("audiodna-test-app-settings-" + juce::Uuid().toString());
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

TEST_CASE("AppSettings: test mode never falls back to the user's real settings file", "[app_settings]")
{
    // MainComponent's appSettingsFile() in a test-server build running --test-mode: AUDIODNA_SETTINGS_FILE if it is an
    // absolute path, else a scratch file -- a --test-mode launch without the variable must not write the real file.
    const auto real = AppSettings::defaultFile();
    SECTION("an absolute AUDIODNA_SETTINGS_FILE is used as is")
    {
        CHECK(AppSettings::testModeFile("/tmp/audiodna-probe/settings.json") == juce::File("/tmp/audiodna-probe/settings.json"));
    }
    SECTION("unset or not absolute: a scratch file in the temp directory, the same one for the whole run")
    {
        for (const juce::String env : { juce::String(), juce::String("settings.json"), juce::String("rel/dir/s.json") })
        {
            INFO("AUDIODNA_SETTINGS_FILE = '" << env << "'");
            const auto f = AppSettings::testModeFile(env);
            REQUIRE(f != real);   // REQUIRE: a broken fallback stops here, before the write below could reach it
            REQUIRE(f.isAChildOf(juce::File::getSpecialLocation(juce::File::tempDirectory)));
            CHECK(f == AppSettings::testModeFile({}));
        }
        // Once written, the same file is read back: the MilkDrop folder / the output set survive within the run.
        const auto f = AppSettings::testModeFile({});
        REQUIRE(f != real);
        REQUIRE(f.isAChildOf(juce::File::getSpecialLocation(juce::File::tempDirectory)));
        REQUIRE(AppSettings(f).update("probe", 7));
        CHECK(AppSettings::testModeFile({}) == f);
        CHECK(static_cast<int>(AppSettings(AppSettings::testModeFile({})).read("probe")) == 7);
        CHECK(f.deleteFile());
    }
}

TEST_CASE("AppSettings: the default file is <userApplicationDataDirectory>/Audio-DNA/settings.json", "[app_settings]")
{
    const auto f = AppSettings::defaultFile();
    CHECK(f.getFileName() == "settings.json");
    CHECK(f.getParentDirectory().getFileName() == "Audio-DNA");
    CHECK(f.getParentDirectory().getParentDirectory()
          == juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory));
}

// ---- lane one-save S1 (ruling-one-save A-15, A-1; rows AS-2..AS-4): the settings.json mend and the verified write.
// update() used to rewrite a file it could not read as a fresh object -- every other key in it was gone for good --
// and wrote through juce::File::replaceWithText, which swaps a cut-off temporary file in and reports success.

TEST_CASE("appsettings: an unreadable file is copied to settings.json.unreadable before the rewrite", "[app_settings]")
{
    TempSettings t;
    REQUIRE(t.file.getParentDirectory().createDirectory());
    const juce::File kept = t.file.getSiblingFile("settings.json.unreadable");
    const AppSettings s(t.file);

    const juce::String first = "{\"outputs\": [1, 2, 3], \"milkDropPresetDir\": \"/a/b\"";   // cut off: no closing brace
    REQUIRE(t.file.replaceWithText(first));
    REQUIRE(s.update("a", 1));
    REQUIRE(kept.existsAsFile());
    CHECK(kept.loadFileAsString() == first);              // the file as it was, to mend by hand
    REQUIRE(t.parsed().getDynamicObject() != nullptr);    // and settings.json is a valid object again
    CHECK(static_cast<int>(t.parsed()["a"]) == 1);

    // A readable file is never copied: the kept copy stays what it was.
    REQUIRE(s.update("b", 2));
    CHECK(kept.loadFileAsString() == first);
    CHECK(static_cast<int>(s.read("a")) == 1);

    // Unreadable again (a root that is not an object): the OLDER copy is replaced by the newer file.
    const juce::String second = "[\"not\", \"an\", \"object\"]";
    REQUIRE(t.file.replaceWithText(second));
    REQUIRE(s.update("c", 3));
    CHECK(kept.loadFileAsString() == second);
    CHECK(static_cast<int>(s.read("c")) == 3);

    // A missing file and an empty file are not "unreadable": nothing to keep.
    TempSettings fresh;
    const AppSettings s2(fresh.file);
    REQUIRE(s2.update("a", 1));
    REQUIRE(fresh.file.replaceWithText(""));
    REQUIRE(s2.update("a", 2));
    CHECK_FALSE(fresh.file.getSiblingFile("settings.json.unreadable").exists());
}

TEST_CASE("appsettings: when that copy cannot be made the file is left as it is and update returns false", "[app_settings]")
{
    TempSettings t;
    REQUIRE(t.file.getParentDirectory().createDirectory());
    const juce::String bad = "{not json";
    REQUIRE(t.file.replaceWithText(bad));
    // Where the copy would go there is a folder that is not empty: no file can take that name.
    const juce::File blocker = t.file.getSiblingFile("settings.json.unreadable");
    REQUIRE(blocker.createDirectory());
    REQUIRE(blocker.getChildFile("x").replaceWithText("x"));

    const AppSettings s(t.file);
    CHECK_FALSE(s.update("a", 1));
    CHECK(t.file.loadFileAsString() == bad);              // not rewritten
    CHECK(blocker.isDirectory());
    CHECK(blocker.getChildFile("x").loadFileAsString() == "x");
    CHECK(t.file.getParentDirectory().findChildFiles(juce::File::findFilesAndDirectories, false).size() == 2);
}

#ifndef _WIN32   // RLIMIT_FSIZE / SIGXFSZ are POSIX: this case has no Windows form
TEST_CASE("appsettings: a half-written update leaves the old file and returns false", "[app_settings]")
{
    TempSettings t;
    const AppSettings s(t.file);
    REQUIRE(s.update("outputs", "the set as it was"));
    juce::MemoryBlock before;
    REQUIRE(t.file.loadFileAsData(before));

    // A REAL half write, no seam: for the length of one update() this process may not write any file past 64 bytes
    // (RLIMIT_FSIZE; the signal that comes with it is ignored), so the kernel cuts the update's temporary file short
    // exactly as a full disk does. juce::File::replaceWithText swaps that cut-off file in and answers true.
    const auto oldHandler = std::signal(SIGXFSZ, SIG_IGN);
    struct rlimit old {};
    REQUIRE(getrlimit(RLIMIT_FSIZE, &old) == 0);
    struct rlimit cut = old;
    cut.rlim_cur = 64;
    REQUIRE(setrlimit(RLIMIT_FSIZE, &cut) == 0);
    const bool ok = s.update("big", juce::String::repeatedString("0123456789", 400));
    const int restored = setrlimit(RLIMIT_FSIZE, &old);
    REQUIRE(restored == 0);
    std::signal(SIGXFSZ, oldHandler);

    CHECK_FALSE(ok);
    juce::MemoryBlock after;
    REQUIRE(t.file.loadFileAsData(after));
    CHECK(after == before);                               // the old file, byte for byte
    CHECK(s.read("outputs").toString() == "the set as it was");
    CHECK(t.file.getParentDirectory().findChildFiles(juce::File::findFilesAndDirectories, false).size() == 1);

    // With the limit gone the same update goes through.
    REQUIRE(s.update("big", juce::String::repeatedString("0123456789", 400)));
    CHECK(s.read("big").toString().length() == 4000);
    CHECK(s.read("outputs").toString() == "the set as it was");
}
#endif
