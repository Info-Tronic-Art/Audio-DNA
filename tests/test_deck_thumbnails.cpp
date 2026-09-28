// test_deck_thumbnails -- s-rta-0928 restore (plan-restore.md Steps 5-6, restore-diag.md cause 1). Every
// DeckView::refresh() -- once per discrete routine-restore entry, on every clip trigger -- used to decode each image
// cell's (and each image strip's) thumbnail from disk right on the message thread (ClipCell::updateThumbnail,
// LayerStrip::updateThumbnail): 99.6-99.9% of a 54-178 ms restore hold. The grid now pulls image thumbnails from
// DeckView's ClipThumbnails (decoded once per file off-thread) and re-derives a thumbnail only when its source changed.
// Headless JUCE widgets under ScopedJuceInitialiser_GUI (no window, no peer, no message loop: a completion lands only
// when the test drains the manual poster).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/DeckView.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <vector>

namespace
{
struct ScratchDir
{
    ScratchDir()
        : dir(juce::File(juce::String(std::filesystem::temp_directory_path().string()))
                  .getChildFile("deck_thumbnails_test_" + juce::Uuid().toString()))
    {
        REQUIRE(dir.createDirectory().wasOk());
    }

    ~ScratchDir() { dir.deleteRecursively(); }

    // A real PNG (the grid's old synchronous decode needs one to produce a thumbnail).
    juce::File makePng(const juce::String& name, juce::Colour c) const
    {
        auto f = dir.getChildFile(name);
        juce::Image img(juce::Image::ARGB, 160, 120, false);
        img.clear(img.getBounds(), c);
        juce::FileOutputStream fos(f);
        REQUIRE(fos.openedOk());
        REQUIRE(juce::PNGImageFormat().writeImageToStream(img, fos));
        return f;
    }

    juce::File dir;
};

Clip imageClip(uint32_t id, const juce::File& f)
{
    Clip c;
    c.id = id;
    c.name = f.getFileNameWithoutExtension().toStdString();
    c.mediaType = Clip::MediaType::Image;
    c.mediaFile = f;
    return c;
}

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

// Returns a 90x72 image, recording the thread of every call.
struct CountingDecoder
{
    juce::CriticalSection lock;
    std::vector<juce::Thread::ThreadID> threads;

    ClipThumbnails::Decoder decoder()
    {
        return [this](const juce::File&) {
            {
                const juce::ScopedLock sl(lock);
                threads.push_back(juce::Thread::getCurrentThreadId());
            }
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

template <typename T>
void collect(juce::Component& root, std::vector<T*>& out)
{
    for (int i = 0; i < root.getNumChildComponents(); ++i)
    {
        auto* child = root.getChildComponent(i);
        if (auto* t = dynamic_cast<T*>(child))
            out.push_back(t);
        collect(*child, out);
    }
}

std::vector<ClipCell*> imageCells(DeckView& dv)
{
    std::vector<ClipCell*> all, out;
    collect(dv, all);
    for (auto* c : all)
        if (c->getClip() != nullptr && c->getClip()->mediaType == Clip::MediaType::Image)
            out.push_back(c);
    return out;
}

std::vector<LayerStrip*> strips(DeckView& dv)
{
    std::vector<LayerStrip*> out;
    collect(dv, out);
    return out;
}
} // namespace

TEST_CASE("DeckView B0: a refresh never decodes a fresh image on the calling thread", "[thumbnails][deck]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    ScratchDir scratch;
    const auto a = scratch.makePng("a.png", juce::Colours::red);
    const auto b = scratch.makePng("b.png", juce::Colours::green);

    Composition comp;
    comp.initDefault();                               // 3 layers x 12 columns
    auto& deck = comp.decks[0];
    deck.setClip(0, 0, imageClip(101, a));
    deck.setClip(0, 2, imageClip(102, b));
    deck.setClip(2, 1, imageClip(103, a));
    deck.layers[0].activeClipColumn = 0;

    DeckView dv;
    dv.setSize(1400, 600);
    dv.setComposition(&comp);
    for (int i = 0; i < 3; ++i)
        dv.refresh();

    const auto cells = imageCells(dv);
    REQUIRE(cells.size() == 3);
    for (auto* c : cells)
    {
        INFO("image cell at layer " << c->getLayerIndex() << ", column " << c->getColumn());
        CHECK_FALSE(c->hasThumbnail());               // RED before s-rta-0928: decoded synchronously, right here
    }
    const auto all = strips(dv);
    REQUIRE(all.size() == 3);
    for (auto* s : all)
        if (s->getLayerIndex() == 0)
            CHECK_FALSE(s->hasThumbnail());           // L0's active clip is an image
}

namespace
{
// B1/B2's deck: 8 layers, image clips in columns 0-3 of every layer (32 image cells) from 4 distinct files (column c
// shows file c), every layer's active column 0. The store's backends are the counting decoder and the manual poster.
struct StormRig
{
    ScratchDir scratch;
    std::vector<juce::File> files;
    Composition comp;
    ManualPoster post;
    CountingDecoder dec;
    DeckView dv;

    StormRig()
    {
        for (int c = 0; c < 4; ++c)
            files.push_back(scratch.makePng("img" + juce::String(c) + ".png", juce::Colour::fromHSV(0.2f * c, 0.8f, 0.9f, 1.0f)));
        comp.initDefault();
        auto& deck = comp.decks[0];
        for (int i = 0; i < 5; ++i)
            deck.addLayer();
        uint32_t id = 200;
        for (int l = 0; l < deck.getNumLayers(); ++l)
        {
            for (int c = 0; c < 4; ++c)
                deck.setClip(l, c, imageClip(id++, files[static_cast<size_t>(c)]));
            deck.layers[static_cast<size_t>(l)].activeClipColumn = 0;
        }
        dv.getThumbnails().setBackendsForTests(dec.decoder(), post.poster());
        dv.setSize(1400, 900);
        dv.setComposition(&comp);
    }

    bool allImageCellsShow(bool want)
    {
        const auto cells = imageCells(dv);
        return cells.size() == 32
               && std::all_of(cells.begin(), cells.end(), [want](ClipCell* c) { return c->hasThumbnail() == want; });
    }

    bool allStripsShow(bool want)
    {
        const auto all = strips(dv);
        return all.size() == 8
               && std::all_of(all.begin(), all.end(), [want](LayerStrip* s) { return s->hasThumbnail() == want; });
    }
};
} // namespace

TEST_CASE("DeckView B1: a refresh storm decodes nothing on the calling thread; each image file decodes once, off-thread",
          "[thumbnails][deck]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    StormRig rig;
    for (int i = 0; i < 30; ++i)
        rig.dv.refresh();
    REQUIRE(rig.post.waitFor(4));
    CHECK(rig.dec.calls() == 4);                      // one decode per distinct file, not per cell / strip / refresh
    CHECK(rig.dec.callsOn(juce::Thread::getCurrentThreadId()) == 0);
    CHECK(rig.allImageCellsShow(false));              // nothing has landed yet
    CHECK(rig.allStripsShow(false));

    CHECK(rig.post.drain() == 4);                     // each landing re-runs refresh(): the waiting cells re-pull
    CHECK(rig.allImageCellsShow(true));
    CHECK(rig.allStripsShow(true));

    for (int i = 0; i < 30; ++i)
        rig.dv.refresh();
    CHECK(rig.dec.calls() == 4);
    CHECK(rig.dv.getThumbnails().decodesQueued() == 4);
}

TEST_CASE("DeckView B2: an unchanged cell re-derives nothing; a changed strip re-pulls once", "[thumbnails][deck]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    StormRig rig;
    rig.dv.refresh();
    REQUIRE(rig.post.waitFor(4));
    rig.post.drain();
    REQUIRE(rig.allImageCellsShow(true));

    auto& store = rig.dv.getThumbnails();
    const int lookups = store.lookups();
    for (int i = 0; i < 30; ++i)
        rig.dv.refresh();
    CHECK(store.lookups() == lookups);                // 32 image cells + 8 strips x 30 refreshes: compares only

    rig.comp.decks[0].layers[0].activeClipColumn = 1;
    rig.dv.refresh();
    CHECK(store.lookups() == lookups + 1);            // only L0's strip: its active clip's source changed
    CHECK(rig.dec.calls() == 4);                      // that file is already decoded
}
