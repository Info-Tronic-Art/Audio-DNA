// test_show_backup -- lane one-save S1 (ruling-one-save A-2; rows SB-1..SB-7): before ANY existing file that is not
// a readable version-2 show is written over, its bytes are copied to <its folder>/backups/<name>.<tag>.json and the
// copy is verified; when the copy cannot be made, NOTHING is written. Drives the real showfile::saveWithBackup (the
// order MainComponent::writeShow runs) with a real Composition::saveToFile as the write, on real temp files.
#include <catch2/catch_test_macros.hpp>
#include "OneSaveFixture.h"
#include "core/ShowFile.h"

using namespace OneSaveFixture;
using showfile::Backup;

namespace
{
// A save of `show` over `target`, as MainComponent::writeShow does it. `writes` counts the show writes that ran.
showfile::SaveOutcome save(const Composition& show, const juce::File& target, int* writes = nullptr,
                           const safewrite::WriteFn& copyWriter = safewrite::writeBytes)
{
    return showfile::saveWithBackup(
        target,
        [&](const juce::File& f) {
            if (writes != nullptr)
                ++*writes;
            return show.saveToFile(f, showfile::ShowExtras{});
        },
        copyWriter);
}

juce::Array<juce::File> backupsIn(const juce::File& dir)
{
    return dir.getChildFile("backups").findChildFiles(juce::File::findFilesAndDirectories, false);
}

bool isVersion2Show(const juce::File& f)
{
    const juce::var v = juce::JSON::parse(f.loadFileAsString());
    return keysOf(v).size() > 1 && keysOf(v).front() == "version" && static_cast<int>(v["version"]) == 2
        && v["decks"].isArray();
}
} // namespace

TEST_CASE("showbackup: an old-shape target is copied byte for byte to backups/<name>.v0.json before the write",
          "[showbackup]")
{
    TempDir t;
    const juce::File target = t.writeVar("old.json", oldShowVar());
    const juce::MemoryBlock original = bytesOf(target);
    Composition show;
    REQUIRE(show.loadFromFile(target));

    const juce::File copy = t.dir.getChildFile("backups").getChildFile("old.v0.json");
    CHECK(showfile::backupFileFor(target, "v0") == copy);

    // "Before the write": when the show write runs, the copy is already there and the target is still the old file.
    bool copyThereAtWrite = false, targetOldAtWrite = false;
    const auto outcome = showfile::saveWithBackup(target, [&](const juce::File& f) {
        copyThereAtWrite = copy.existsAsFile() && bytesOf(copy) == original;
        targetOldAtWrite = bytesOf(target) == original;
        return show.saveToFile(f, showfile::ShowExtras{});
    });
    CHECK(outcome.saved);
    CHECK(outcome.backup == Backup::Made);
    CHECK(copyThereAtWrite);
    CHECK(targetOldAtWrite);
    REQUIRE(copy.existsAsFile());
    CHECK(bytesOf(copy) == original);                    // byte for byte
    CHECK(backupsIn(t.dir).size() == 1);
    CHECK(isVersion2Show(target));                       // and the target is the new shape now
    CHECK(bytesOf(target) != original);
}

TEST_CASE("showbackup: a version-1 target gives .v1.json, a version-3 target .v3.json; a version-2 target gives none",
          "[showbackup]")
{
    const Composition show = currentShow();
    SECTION("version 1")
    {
        TempDir t;
        const juce::File target = t.writeVar("a.json", v1ShowVar());
        const auto original = bytesOf(target);
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::Made);
        REQUIRE(backupsIn(t.dir).size() == 1);
        CHECK(backupsIn(t.dir)[0].getFileName() == "a.v1.json");
        CHECK(bytesOf(backupsIn(t.dir)[0]) == original);
        CHECK(isVersion2Show(target));
    }
    SECTION("version 3 -- a show from a later build is kept before this build writes version 2 over it")
    {
        TempDir t;
        const juce::File target = t.writeVar("a.json", v3ShowVar());
        const auto original = bytesOf(target);
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::Made);
        REQUIRE(backupsIn(t.dir).size() == 1);
        CHECK(backupsIn(t.dir)[0].getFileName() == "a.v3.json");
        CHECK(bytesOf(backupsIn(t.dir)[0]) == original);
        CHECK(isVersion2Show(target));
    }
    SECTION("version 2: no copy, no backups folder")
    {
        TempDir t;
        const juce::File target = t.dir.getChildFile("a.json");
        REQUIRE(show.saveToFile(target, showfile::ShowExtras{}));
        CHECK(showfile::backupBeforeOverwrite(target) == Backup::NotNeeded);
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::NotNeeded);
        CHECK_FALSE(t.dir.getChildFile("backups").exists());
    }
}

TEST_CASE("showbackup: a second save makes no second backup; an existing identical backup is kept", "[showbackup]")
{
    TempDir t;
    const juce::File target = t.writeVar("old.json", oldShowVar());
    const auto original = bytesOf(target);
    Composition show;
    REQUIRE(show.loadFromFile(target));

    REQUIRE(save(show, target).backup == Backup::Made);
    const auto second = save(show, target);              // the target is a version-2 show now
    CHECK(second.saved);
    CHECK(second.backup == Backup::NotNeeded);
    REQUIRE(backupsIn(t.dir).size() == 1);

    // The old file comes back under the same name (he copies it in again): its identical copy is kept, none added.
    REQUIRE(target.replaceWithData(original.getData(), original.getSize()));
    const juce::File copy = backupsIn(t.dir)[0];
    const auto copyTime = copy.getLastModificationTime();
    const auto third = save(show, target);
    CHECK(third.saved);
    CHECK(third.backup == Backup::AlreadyThere);
    REQUIRE(backupsIn(t.dir).size() == 1);
    CHECK(bytesOf(copy) == original);
    CHECK(copy.getLastModificationTime() == copyTime);   // not rewritten
    CHECK(isVersion2Show(target));
}

TEST_CASE("showbackup: when the copy cannot be made nothing is written and the target's bytes are unchanged",
          "[showbackup]")
{
    TempDir t;
    const juce::File target = t.writeVar("old.json", oldShowVar());
    const auto original = bytesOf(target);
    t.write("backups", "a plain FILE where the backups folder would go");
    Composition show;
    REQUIRE(show.loadFromFile(target));

    int writes = 0;
    const auto outcome = save(show, target, &writes);
    CHECK_FALSE(outcome.saved);
    CHECK(outcome.backup == Backup::Failed);
    CHECK(writes == 0);                                  // the show write never ran
    CHECK(bytesOf(target) == original);
    CHECK(t.dir.getChildFile("backups").existsAsFile()); // his file named "backups" is as it was
    CHECK(t.dir.getChildFile("backups").loadFileAsString() == "a plain FILE where the backups folder would go");
    CHECK(t.dir.findChildFiles(juce::File::findFilesAndDirectories, false).size() == 2);   // nothing else appeared
}

TEST_CASE("showbackup: an existing different backup is never overwritten (the copy takes the next free name)",
          "[showbackup]")
{
    TempDir t;
    const juce::File target = t.writeVar("old.json", oldShowVar());
    const auto original = bytesOf(target);
    const juce::File backups = t.dir.getChildFile("backups");
    REQUIRE(backups.createDirectory());
    const juce::File earlier = backups.getChildFile("old.v0.json");
    REQUIRE(earlier.replaceWithText("an earlier copy with OTHER bytes"));
    Composition show;
    REQUIRE(show.loadFromFile(target));

    const auto outcome = save(show, target);
    CHECK(outcome.saved);
    CHECK(outcome.backup == Backup::Made);
    CHECK(earlier.loadFileAsString() == "an earlier copy with OTHER bytes");
    const juce::File next = backups.getChildFile("old.v0 (2).json");
    CHECK(showfile::backupFileFor(target, "v0", 2) == next);
    REQUIRE(next.existsAsFile());
    CHECK(bytesOf(next) == original);
    CHECK(backupsIn(t.dir).size() == 2);

    // Again with the old bytes back: "(2)" holds them already -- kept, no "(3)".
    REQUIRE(target.replaceWithData(original.getData(), original.getSize()));
    CHECK(save(show, target).backup == Backup::AlreadyThere);
    CHECK(backupsIn(t.dir).size() == 2);
}

TEST_CASE("showbackup: a missing or empty target needs no copy; a target that does not parse, or is not a show, is "
          "copied to backups/<name>.other.json", "[showbackup]")
{
    const Composition show = currentShow();
    SECTION("missing")
    {
        TempDir t;
        const juce::File target = t.dir.getChildFile("new.json");
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::NotNeeded);
        CHECK_FALSE(t.dir.getChildFile("backups").exists());
        CHECK(isVersion2Show(target));
    }
    SECTION("empty")
    {
        TempDir t;
        const juce::File target = t.write("empty.json", "");
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::NotNeeded);
        CHECK_FALSE(t.dir.getChildFile("backups").exists());
        CHECK(isVersion2Show(target));
    }
    SECTION("does not parse (the 10 bytes of F-GARBAGE)")
    {
        TempDir t;
        const juce::File target = t.write("x.json", kGarbage);
        REQUIRE(target.getSize() == 10);
        const auto outcome = save(show, target);
        CHECK(outcome.saved);
        CHECK(outcome.backup == Backup::Made);
        const juce::File copy = t.dir.getChildFile("backups").getChildFile("x.other.json");
        REQUIRE(copy.existsAsFile());
        CHECK(copy.getSize() == 10);
        CHECK(copy.loadFileAsString() == kGarbage);
        CHECK(backupsIn(t.dir).size() == 1);
        CHECK(isVersion2Show(target));
    }
    SECTION("parses, but is not a show: an array, an object without decks, a version-2 object without decks")
    {
        for (const char* content : { "[1, 2, 3]", R"({"type": "deck", "fx": [], "slots": []})",
                                     R"({"version": 2, "name": "no decks here"})", "\"a string\"", "42" })
        {
            INFO(content);
            TempDir t;
            const juce::File target = t.write("x.json", content);
            const auto outcome = save(show, target);
            CHECK(outcome.saved);
            CHECK(outcome.backup == Backup::Made);
            const juce::File copy = t.dir.getChildFile("backups").getChildFile("x.other.json");
            REQUIRE(copy.existsAsFile());
            CHECK(copy.loadFileAsString() == content);
            CHECK(isVersion2Show(target));
        }
    }
}

TEST_CASE("showbackup: a copy that cannot be verified is Failed and nothing is written", "[showbackup]")
{
    TempDir t;
    const juce::File target = t.writeVar("old.json", oldShowVar());
    const auto original = bytesOf(target);
    Composition show;
    REQUIRE(show.loadFromFile(target));

    // The copy's writer stores half the bytes and reports success (a full disk, as juce::File's own text writer
    // would leave it): the read-back sees it.
    const safewrite::WriteFn halfAndSaysOk = [](const juce::File& dest, const void* data, size_t size) {
        safewrite::writeBytes(dest, data, size / 2);
        return true;
    };
    int writes = 0;
    const auto outcome = save(show, target, &writes, halfAndSaysOk);
    CHECK_FALSE(outcome.saved);
    CHECK(outcome.backup == Backup::Failed);
    CHECK(writes == 0);
    CHECK(bytesOf(target) == original);
    CHECK(backupsIn(t.dir).isEmpty());                   // the cut-off copy is not left behind as "the backup"

    // With a working writer the same save then goes through.
    const auto retry = save(show, target, &writes);
    CHECK(retry.saved);
    CHECK(retry.backup == Backup::Made);
    CHECK(writes == 1);
    CHECK(bytesOf(t.dir.getChildFile("backups").getChildFile("old.v0.json")) == original);
}
