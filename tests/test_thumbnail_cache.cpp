#include <catch2/catch_test_macros.hpp>
#include "ui/ThumbnailCache.h"

#include <filesystem>

// Headless test of the cache-keying + LRU logic extracted from FilesBrowser's
// thumbnail pipeline. Uses real files on disk (with explicitly-set mtimes, to
// avoid relying on filesystem mtime-resolution timing) since the cache key is
// (full path + mtime).
//
// Scratch files live in a per-PROCESS unique directory. catch_discover_tests
// registers every TEST_CASE as its own ctest test (own process) and
// `ctest -j8` runs sibling cases concurrently -- with one shared directory,
// the LRU case's teardown deleted the "changed mtime" case's file mid-test
// (flaky failure). The directory is under the OS temp dir
// (std::filesystem::temp_directory_path, $TMPDIR on macOS), NOT
// juce::File::tempDirectory, which on macOS resolves to the real user cache
// dir ~/Library/Caches/<exe>/ and is never removed.

namespace
{
// RAII: unique scratch directory, removed on scope exit -- including when a
// REQUIRE throws mid-test, so a failing run leaves nothing behind.
struct ScratchDir
{
    ScratchDir()
        : dir(juce::File(juce::String(std::filesystem::temp_directory_path().string()))
                  .getChildFile("thumbnail_cache_test_" + juce::Uuid().toString()))
    {
        REQUIRE(dir.createDirectory().wasOk());
    }

    ~ScratchDir() { dir.deleteRecursively(); }

    juce::File makeFile(const juce::String& name) const
    {
        auto f = dir.getChildFile(name);
        REQUIRE(f.replaceWithText("stub content"));
        return f;
    }

    juce::File dir;
};

juce::Image makeImage(int tag)
{
    juce::Image img(juce::Image::ARGB, 2, 2, true);
    img.setPixelAt(0, 0, juce::Colour(static_cast<juce::uint32>(tag)));
    return img;
}
} // namespace

TEST_CASE("ThumbnailCache basic put/get", "[thumbnail-cache]")
{
    ScratchDir scratch;
    ThumbnailCache cache;
    auto file = scratch.makeFile("a.png");

    SECTION("miss before any put")
    {
        REQUIRE_FALSE(cache.get(file).isValid());
    }

    SECTION("put then get hits at the same mtime")
    {
        cache.put(file, makeImage(1));
        auto result = cache.get(file);
        REQUIRE(result.isValid());
        REQUIRE(result.getWidth() == 2);
    }
}

TEST_CASE("ThumbnailCache treats a changed mtime as a miss", "[thumbnail-cache]")
{
    ScratchDir scratch;
    ThumbnailCache cache;
    auto file = scratch.makeFile("b.png");

    auto original = file.getLastModificationTime();
    cache.put(file, makeImage(2));
    REQUIRE(cache.get(file).isValid());

    // Deterministically move the mtime forward instead of sleeping --
    // filesystem mtime resolution can be coarser than a real edit-refresh
    // cycle would produce, which would make a sleep-based test flaky.
    REQUIRE(file.setLastModificationTime(original + juce::RelativeTime::seconds(5)));

    REQUIRE_FALSE(cache.get(file).isValid());
}

TEST_CASE("ThumbnailCache evicts least-recently-used entries at capacity", "[thumbnail-cache]")
{
    ScratchDir scratch;
    ThumbnailCache cache(3);

    auto fileA = scratch.makeFile("lru_a.png");
    auto fileB = scratch.makeFile("lru_b.png");
    auto fileC = scratch.makeFile("lru_c.png");
    auto fileD = scratch.makeFile("lru_d.png");

    cache.put(fileA, makeImage(1));
    cache.put(fileB, makeImage(2));
    cache.put(fileC, makeImage(3));
    REQUIRE(cache.size() == 3);

    // Touch A so it becomes most-recently-used; B is now the LRU entry.
    REQUIRE(cache.get(fileA).isValid());

    cache.put(fileD, makeImage(4));
    REQUIRE(cache.size() == 3);

    REQUIRE(cache.get(fileB).isValid() == false);  // evicted
    REQUIRE(cache.get(fileA).isValid());
    REQUIRE(cache.get(fileC).isValid());
    REQUIRE(cache.get(fileD).isValid());
}

TEST_CASE("ThumbnailCache re-put at the same mtime overwrites without growing", "[thumbnail-cache]")
{
    ScratchDir scratch;
    ThumbnailCache cache;
    auto file = scratch.makeFile("refresh.png");

    cache.put(file, makeImage(1));
    REQUIRE(cache.size() == 1);

    cache.put(file, makeImage(9));
    REQUIRE(cache.size() == 1);
    REQUIRE(cache.get(file).isValid());
}

TEST_CASE("ThumbnailCache put(file, explicitMtime, image) keys by the captured mtime, not the file's live mtime",
          "[thumbnail-cache]")
{
    ScratchDir scratch;
    ThumbnailCache cache;
    auto file = scratch.makeFile("explicit_mtime.png");

    // Simulate a decode that captured the mtime on a background thread
    // BEFORE the file changed again on disk during the hop back to the
    // message thread (the race the explicit-mtime overload closes).
    auto mtimeAtDecode = file.getLastModificationTime();
    REQUIRE(file.setLastModificationTime(mtimeAtDecode + juce::RelativeTime::seconds(5)));

    cache.put(file, mtimeAtDecode, makeImage(7));

    // A get() against the file's CURRENT (now-different) mtime must miss --
    // the stale-content decode is never silently served as if it were fresh
    // under a current-looking key.
    REQUIRE_FALSE(cache.get(file).isValid());
}
