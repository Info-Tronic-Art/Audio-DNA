#include <catch2/catch_test_macros.hpp>
#include "ui/ThumbnailCache.h"

// Headless test of the cache-keying + LRU logic extracted from FilesBrowser's
// thumbnail pipeline. Uses real files on disk (with explicitly-set mtimes, to
// avoid relying on filesystem mtime-resolution timing) since the cache key is
// (full path + mtime).

namespace
{
juce::File makeTempFile(const juce::String& name)
{
    auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getChildFile("thumbnail_cache_test");
    dir.createDirectory();
    auto f = dir.getChildFile(name);
    f.replaceWithText("stub content");
    return f;
}

juce::Image makeImage(int tag)
{
    juce::Image img(juce::Image::ARGB, 2, 2, true);
    img.setPixelAt(0, 0, juce::Colour(static_cast<juce::uint32>(tag)));
    return img;
}
} // namespace

TEST_CASE("ThumbnailCache basic put/get", "[thumbnail-cache]")
{
    ThumbnailCache cache;
    auto file = makeTempFile("a.png");

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

    file.deleteRecursively();
}

TEST_CASE("ThumbnailCache treats a changed mtime as a miss", "[thumbnail-cache]")
{
    ThumbnailCache cache;
    auto file = makeTempFile("b.png");

    auto original = file.getLastModificationTime();
    cache.put(file, makeImage(2));
    REQUIRE(cache.get(file).isValid());

    // Deterministically move the mtime forward instead of sleeping —
    // filesystem mtime resolution can be coarser than a real edit-refresh
    // cycle would produce, which would make a sleep-based test flaky.
    REQUIRE(file.setLastModificationTime(original + juce::RelativeTime::seconds(5)));

    REQUIRE_FALSE(cache.get(file).isValid());

    file.deleteRecursively();
}

TEST_CASE("ThumbnailCache evicts least-recently-used entries at capacity", "[thumbnail-cache]")
{
    ThumbnailCache cache(3);

    auto fileA = makeTempFile("lru_a.png");
    auto fileB = makeTempFile("lru_b.png");
    auto fileC = makeTempFile("lru_c.png");
    auto fileD = makeTempFile("lru_d.png");

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

    fileA.getParentDirectory().deleteRecursively();
}

TEST_CASE("ThumbnailCache re-put at the same mtime overwrites without growing", "[thumbnail-cache]")
{
    ThumbnailCache cache;
    auto file = makeTempFile("refresh.png");

    cache.put(file, makeImage(1));
    REQUIRE(cache.size() == 1);

    cache.put(file, makeImage(9));
    REQUIRE(cache.size() == 1);
    REQUIRE(cache.get(file).isValid());

    file.deleteRecursively();
}

TEST_CASE("ThumbnailCache put(file, explicitMtime, image) keys by the captured mtime, not the file's live mtime",
          "[thumbnail-cache]")
{
    ThumbnailCache cache;
    auto file = makeTempFile("explicit_mtime.png");

    // Simulate a decode that captured the mtime on a background thread
    // BEFORE the file changed again on disk during the hop back to the
    // message thread (the race the explicit-mtime overload closes).
    auto mtimeAtDecode = file.getLastModificationTime();
    REQUIRE(file.setLastModificationTime(mtimeAtDecode + juce::RelativeTime::seconds(5)));

    cache.put(file, mtimeAtDecode, makeImage(7));

    // A get() against the file's CURRENT (now-different) mtime must miss —
    // the stale-content decode is never silently served as if it were fresh
    // under a current-looking key.
    REQUIRE_FALSE(cache.get(file).isValid());

    file.deleteRecursively();
}
