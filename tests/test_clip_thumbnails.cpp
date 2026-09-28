// test_clip_thumbnails -- s-rta-0928 restore (plan-restore.md Step 4, restore-diag.md cause 1): ClipThumbnails, the
// deck grid's image-thumbnail store. get() never decodes on the calling (message) thread; a miss queues ONE decode per
// file on a pool thread; a failure waits for a new mtime; an LRU-evicted entry comes back through one new decode; a
// completion that lands after the store is gone does nothing; a pending path is answered without a stat() (Harmony
// adoption D4). A ctest runs no message loop (JUCE_MODAL_LOOPS_PERMITTED is 0), so a manual poster queues each
// completion and the test thread drains it; a counting decoder records which thread decoded.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/ClipThumbnails.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <memory>
#include <vector>

namespace
{
// RAII: a unique scratch directory per process, removed on scope exit (test_thumbnail_cache.cpp's idiom: ctest runs
// every TEST_CASE as its own process, possibly concurrently).
struct ScratchDir
{
    ScratchDir()
        : dir(juce::File(juce::String(std::filesystem::temp_directory_path().string()))
                  .getChildFile("clip_thumbnails_test_" + juce::Uuid().toString()))
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

// Completions queue here; the test thread runs them (it plays the message thread).
struct ManualPoster
{
    juce::CriticalSection lock;
    std::vector<std::function<void()>> queue;
    std::atomic<int> posted { 0 };

    ClipThumbnails::Poster poster()
    {
        return [this](std::function<void()> fn) {
            {
                const juce::ScopedLock sl(lock);
                queue.push_back(std::move(fn));
            }
            ++posted;
        };
    }

    bool waitFor(int n, int timeoutMs = 5000)
    {
        const auto end = juce::Time::getMillisecondCounter() + static_cast<juce::uint32>(timeoutMs);
        while (posted.load() < n)
        {
            if (juce::Time::getMillisecondCounter() > end)
                return false;
            juce::Thread::sleep(1);
        }
        return true;
    }

    int drain()
    {
        std::vector<std::function<void()>> run;
        {
            const juce::ScopedLock sl(lock);
            run.swap(queue);
        }
        for (auto& fn : run)
            fn();
        return static_cast<int>(run.size());
    }
};

// Returns a 90x72 image (invalid for a file named in `fail`), recording the thread of every call.
struct CountingDecoder
{
    juce::CriticalSection lock;
    std::vector<juce::Thread::ThreadID> threads;
    juce::StringArray fail;

    ClipThumbnails::Decoder decoder()
    {
        return [this](const juce::File& f) {
            {
                const juce::ScopedLock sl(lock);
                threads.push_back(juce::Thread::getCurrentThreadId());
            }
            if (fail.contains(f.getFileName()))
                return juce::Image();
            juce::Image img(juce::Image::ARGB, ClipThumbnails::kWidth, ClipThumbnails::kHeight, true);
            img.clear(img.getBounds(), juce::Colours::orange);
            return img;
        };
    }

    int calls()
    {
        const juce::ScopedLock sl(lock);
        return static_cast<int>(threads.size());
    }

    int callsOn(juce::Thread::ThreadID id)
    {
        const juce::ScopedLock sl(lock);
        return static_cast<int>(std::count(threads.begin(), threads.end(), id));
    }
};
} // namespace

TEST_CASE("ClipThumbnails S1: get() never decodes on the calling thread; a miss queues one decode per file", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makeFile("a.png");
    const auto b = scratch.makeFile("b.png");
    ManualPoster post;
    CountingDecoder dec;
    ClipThumbnails store;
    store.setBackendsForTests(dec.decoder(), post.poster());
    int landed = 0;
    store.onLanded = [&landed] { ++landed; };

    for (int i = 0; i < 10; ++i)
    {
        CHECK_FALSE(store.get(a).isValid());
        CHECK_FALSE(store.get(b).isValid());
    }
    REQUIRE(post.waitFor(2));
    CHECK(dec.calls() == 2);
    CHECK(dec.callsOn(juce::Thread::getCurrentThreadId()) == 0);
    CHECK(store.decodesQueued() == 2);

    CHECK(post.drain() == 2);
    CHECK(landed == 2);
    const auto ia = store.get(a), ib = store.get(b);
    REQUIRE(ia.isValid());
    REQUIRE(ib.isValid());
    CHECK(ia.getWidth() == 90);
    CHECK(ia.getHeight() == 72);
    CHECK(ib.getWidth() == 90);
    CHECK(store.decodesQueued() == 2);
    CHECK(dec.calls() == 2);
}

TEST_CASE("ClipThumbnails S2: a failed decode is not retried at the same mtime; a new mtime is a new decode", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makeFile("a.png");
    ManualPoster post;
    CountingDecoder dec;
    dec.fail.add("a.png");
    ClipThumbnails store;
    store.setBackendsForTests(dec.decoder(), post.poster());

    CHECK_FALSE(store.get(a).isValid());
    REQUIRE(post.waitFor(1));
    post.drain();
    for (int i = 0; i < 5; ++i)
        CHECK_FALSE(store.get(a).isValid());
    CHECK(store.decodesQueued() == 1);

    REQUIRE(a.setLastModificationTime(a.getLastModificationTime() + juce::RelativeTime::seconds(2.0)));
    CHECK_FALSE(store.get(a).isValid());
    CHECK(store.decodesQueued() == 2);
    REQUIRE(post.waitFor(2));
    post.drain();
}

TEST_CASE("ClipThumbnails S3: an entry the LRU evicted comes back through one new decode", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makeFile("a.png");
    const auto b = scratch.makeFile("b.png");
    ManualPoster post;
    CountingDecoder dec;
    ClipThumbnails store(1);
    store.setBackendsForTests(dec.decoder(), post.poster());

    store.get(a);
    REQUIRE(post.waitFor(1));
    post.drain();
    CHECK(store.get(a).isValid());
    store.get(b);                                     // b's landing evicts a (one entry)
    REQUIRE(post.waitFor(2));
    post.drain();
    CHECK(store.get(b).isValid());
    CHECK(store.decodesQueued() == 2);

    CHECK_FALSE(store.get(a).isValid());              // evicted: queued again, never stuck invalid
    CHECK(store.decodesQueued() == 3);
    REQUIRE(post.waitFor(3));
    post.drain();
    CHECK(store.get(a).isValid());
}

TEST_CASE("ClipThumbnails S4: a completion that lands after the store is gone does nothing", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makeFile("a.png");
    ManualPoster post;                                // outlives the store: its queue holds the completion
    CountingDecoder dec;
    auto landed = std::make_shared<int>(0);
    {
        ClipThumbnails store;
        store.setBackendsForTests(dec.decoder(), post.poster());
        store.onLanded = [landed] { ++*landed; };
        store.get(a);
        REQUIRE(post.waitFor(1));
    }                                                 // the store is destroyed with its completion still queued
    CHECK(post.drain() == 1);
    CHECK(*landed == 0);
}

TEST_CASE("ClipThumbnails S5: decodeThumbnail -- a real PNG gives 90x72; a text or missing file gives invalid", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto png = scratch.dir.getChildFile("real.png");
    {
        juce::Image img(juce::Image::ARGB, 300, 200, false);
        img.clear(img.getBounds(), juce::Colours::teal);
        juce::FileOutputStream fos(png);
        REQUIRE(fos.openedOk());
        REQUIRE(juce::PNGImageFormat().writeImageToStream(img, fos));
    }
    const auto thumb = ClipThumbnails::decodeThumbnail(png);
    REQUIRE(thumb.isValid());
    CHECK(thumb.getWidth() == 90);
    CHECK(thumb.getHeight() == 72);
    CHECK_FALSE(ClipThumbnails::decodeThumbnail(scratch.makeFile("text.png")).isValid());
    CHECK_FALSE(ClipThumbnails::decodeThumbnail(scratch.dir.getChildFile("missing.png")).isValid());
}

// Harmony adoption D4: a cell whose thumbnail is still decoding re-pulls on EVERY DeckView::refresh; that must cost a
// set lookup, never a stat() of the file (the store counts every stat it makes).
TEST_CASE("ClipThumbnails S6: a pending path is answered without a stat", "[thumbnails]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makeFile("a.png");
    ManualPoster post;
    CountingDecoder dec;
    ClipThumbnails store;
    store.setBackendsForTests(dec.decoder(), post.poster());

    store.get(a);                                     // the miss: stats, then queues
    const int statsAtQueue = store.fileStats();
    CHECK(statsAtQueue > 0);
    for (int i = 0; i < 30; ++i)
        CHECK_FALSE(store.get(a).isValid());          // pending: no stat
    CHECK(store.fileStats() == statsAtQueue);
    CHECK(store.lookups() == 31);
    REQUIRE(post.waitFor(1));
    post.drain();
}
