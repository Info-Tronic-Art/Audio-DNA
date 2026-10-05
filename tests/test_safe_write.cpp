// test_safe_write -- lane one-save S1 (ruling-one-save A-1, rows SW-1..SW-4): the verified writer of
// src/core/SafeFileWrite.h. juce::File::replaceWithText drops the result of its temporary write and swaps whatever
// was stored, so a write cut short replaces the target with a cut-off file and reports success. The rows drive the
// real functions on real temp files; "half the bytes stored, success reported" is modelled with the WriteFn seam
// (a full disk is Harmony's measured row M-1 / OS-L21). Every case uses its own temp folder.
#include <catch2/catch_test_macros.hpp>
#include "core/SafeFileWrite.h"

namespace
{
struct TempDir
{
    // A name of its own per PROCESS: ctest runs the cases of one binary as parallel processes, and two of them asking
    // for "the next free name" at once would get the same folder.
    juce::File dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile("audiodna-test-safe-write-" + juce::Uuid().toString());
    TempDir() { REQUIRE(dir.createDirectory()); }
    ~TempDir() { dir.deleteRecursively(); }
    // Everything in the folder, hidden files included (the temporary file of a write is a hidden one).
    int entries() const
    {
        return dir.findChildFiles(juce::File::findFilesAndDirectories, false).size();
    }
};

juce::MemoryBlock bytesOf(const juce::File& f)
{
    juce::MemoryBlock b;
    REQUIRE(f.loadFileAsData(b));
    return b;
}

// The JUCE behaviour of ruling V1: half the bytes reach the file, and the writer says it went well.
const safewrite::WriteFn kHalfAndSaysOk = [](const juce::File& dest, const void* data, size_t size) {
    safewrite::writeBytes(dest, data, size / 2);
    return true;
};

// A writer that stores nothing and says so.
const safewrite::WriteFn kFails = [](const juce::File&, const void*, size_t) { return false; };

// A writer that stores half the bytes and says it failed (a short write that IS reported).
const safewrite::WriteFn kHalfAndFails = [](const juce::File& dest, const void* data, size_t size) {
    safewrite::writeBytes(dest, data, size / 2);
    return false;
};
} // namespace

TEST_CASE("safewrite: the bytes written are read back equal before the swap; the target then holds them", "[safewrite]")
{
    TempDir t;
    const juce::File target = t.dir.getChildFile("show.json");
    const juce::String text = juce::String::fromUTF8("{\n  \"name\": \"caf\xc3\xa9 \xe2\x99\xaa\",\n  \"n\": 1\n}");

    SECTION("a new target")
    {
        REQUIRE(safewrite::writeTextVerified(target, text));
        const auto got = bytesOf(target);
        REQUIRE(got.getSize() == text.getNumBytesAsUTF8());
        CHECK(std::memcmp(got.getData(), text.toRawUTF8(), got.getSize()) == 0);   // the UTF-8 bytes, as they are
        CHECK(t.entries() == 1);                                                   // no temporary file left behind
    }
    SECTION("an existing, LONGER target is replaced whole (never appended to, never left with a tail)")
    {
        REQUIRE(target.replaceWithText(juce::String::repeatedString("old old old ", 400)));
        REQUIRE(safewrite::writeTextVerified(target, text));
        const auto got = bytesOf(target);
        REQUIRE(got.getSize() == text.getNumBytesAsUTF8());
        CHECK(std::memcmp(got.getData(), text.toRawUTF8(), got.getSize()) == 0);
        CHECK(t.entries() == 1);
    }
    SECTION("the swap happens only after the read-back: the writer sees the old target still in place")
    {
        REQUIRE(target.replaceWithText("old"));
        bool targetWasOldDuringWrite = false;
        juce::File written;
        const safewrite::WriteFn spy = [&](const juce::File& dest, const void* data, size_t size) {
            written = dest;
            targetWasOldDuringWrite = target.loadFileAsString() == "old";
            return safewrite::writeBytes(dest, data, size);
        };
        REQUIRE(safewrite::writeTextVerified(target, text, spy));
        CHECK(targetWasOldDuringWrite);
        CHECK(written != target);                                // the bytes go to a temporary file ...
        CHECK(written.getParentDirectory() == t.dir);            // ... beside the target
        CHECK_FALSE(written.exists());                           // ... which the swap consumed
        CHECK(target.loadFileAsString() == text);
    }
    SECTION("an empty text is a write like any other")
    {
        REQUIRE(target.replaceWithText("old"));
        REQUIRE(safewrite::writeTextVerified(target, juce::String()));
        CHECK(target.existsAsFile());
        CHECK(target.getSize() == 0);
    }
}

TEST_CASE("safewrite: a writer that stores half the bytes and reports success leaves the target's bytes unchanged and "
          "returns false", "[safewrite]")
{
    TempDir t;
    const juce::File target = t.dir.getChildFile("show.json");
    const juce::String old = "{\"version\": 2, \"decks\": [], \"name\": \"the show as it was\"}";
    REQUIRE(target.replaceWithText(old));
    const auto before = bytesOf(target);
    const juce::String text = juce::String::repeatedString("{\"a\": 1234567890}\n", 300);

    CHECK_FALSE(safewrite::writeTextVerified(target, text, kHalfAndSaysOk));
    CHECK(bytesOf(target) == before);            // the old show, byte for byte
    CHECK(t.entries() == 1);                     // and the cut-off temporary file is gone

    // The same writer onto a target that does not exist yet: nothing appears.
    const juce::File fresh = t.dir.getChildFile("fresh.json");
    CHECK_FALSE(safewrite::writeTextVerified(fresh, text, kHalfAndSaysOk));
    CHECK_FALSE(fresh.exists());
    CHECK(t.entries() == 1);
}

TEST_CASE("safewrite: a writer that fails leaves the target unchanged and no temporary file behind", "[safewrite]")
{
    TempDir t;
    const juce::File target = t.dir.getChildFile("show.json");
    REQUIRE(target.replaceWithText("the show as it was"));
    const auto before = bytesOf(target);
    const juce::String text = juce::String::repeatedString("new new new ", 200);

    CHECK_FALSE(safewrite::writeTextVerified(target, text, kFails));
    CHECK(bytesOf(target) == before);
    CHECK(t.entries() == 1);

    CHECK_FALSE(safewrite::writeTextVerified(target, text, kHalfAndFails));   // a short write that is reported
    CHECK(bytesOf(target) == before);
    CHECK(t.entries() == 1);

    // The production writer into a folder that does not exist: false, and nothing is created.
    const juce::File nowhere = t.dir.getChildFile("no-such-folder").getChildFile("show.json");
    CHECK_FALSE(safewrite::writeTextVerified(nowhere, text));
    CHECK_FALSE(nowhere.getParentDirectory().exists());
    CHECK(t.entries() == 1);
}

TEST_CASE("safewrite: copyVerified removes a copy whose bytes differ and returns false", "[safewrite]")
{
    TempDir t;
    const juce::File from = t.dir.getChildFile("from.json");
    const juce::File to = t.dir.getChildFile("to.json");
    juce::MemoryBlock data;
    for (int i = 0; i < 5000; ++i)
        data.append("\x00\x01\xff\x7f" "abc\n", 8);   // binary-safe: NUL bytes, high bytes
    REQUIRE(from.replaceWithData(data.getData(), data.getSize()));

    SECTION("a good copy holds the same bytes")
    {
        REQUIRE(safewrite::copyVerified(from, to));
        CHECK(bytesOf(to) == data);
        CHECK(bytesOf(from) == data);
    }
    SECTION("an older copy is replaced, not appended to")
    {
        REQUIRE(to.replaceWithText(juce::String::repeatedString("older ", 20000)));
        REQUIRE(safewrite::copyVerified(from, to));
        CHECK(bytesOf(to) == data);
    }
    SECTION("half the bytes stored, success reported: the copy is removed, false")
    {
        CHECK_FALSE(safewrite::copyVerified(from, to, kHalfAndSaysOk));
        CHECK_FALSE(to.exists());
        CHECK(bytesOf(from) == data);
    }
    SECTION("a writer that fails: no copy, false")
    {
        CHECK_FALSE(safewrite::copyVerified(from, to, kHalfAndFails));
        CHECK_FALSE(to.exists());
    }
    SECTION("a source that is not there: false, nothing made")
    {
        CHECK_FALSE(safewrite::copyVerified(t.dir.getChildFile("missing.json"), to));
        CHECK_FALSE(to.exists());
    }
}
