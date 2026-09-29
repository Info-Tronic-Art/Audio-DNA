// test_native_layer_cache -- s-rta-0928b idlepaint (plan-idlepaint.md 2.2 / 3.1 test 1 + Harmony adoption I1). The
// juce::CachedComponentImage that hands a widget's repaints to its own CALayer (src/ui/NativeLayerCache.h): Native swallows
// every repaint (the peer is never dirtied) and marks the layer; Fallback is transparent; Native -> Fallback hides the
// layer and hands the widget to the peer SYNCHRONOUSLY (I1); Fallback -> Native shows the layer only after it drew
// (RestorePending). Headless JUCE widgets under ScopedJuceInitialiser_GUI -- no window, no peer: the "peer" is a spy
// CachedComponentImage on the widget's parent (JUCE forwards a repaint the widget's cache let through to the parent,
// which asks ITS cache first -- juce_Component.cpp internalRepaintUnchecked). The AsyncUpdater turn is flushForTest().
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/NativeLayerCache.h"
#include <string>
#include <vector>

namespace
{
using Mode = NativeLayerCache::Mode;

struct FakeSink final : NativeLayerCache::Sink
{
    std::vector<std::string> log;   // "L x y w h" = layerNeedsDisplay, "S 1|0" = layerShown, "P x y w h" = peerNeedsDisplay
    static std::string r(juce::Rectangle<int> a)
    {
        return std::to_string(a.getX()) + " " + std::to_string(a.getY()) + " " + std::to_string(a.getWidth()) + " "
             + std::to_string(a.getHeight());
    }
    void layerNeedsDisplay(juce::Rectangle<int> a) override { log.push_back("L " + r(a)); }
    void layerShown(bool s) override { log.push_back(s ? "S 1" : "S 0"); }
    void peerNeedsDisplay(juce::Rectangle<int> a) override { log.push_back("P " + r(a)); }
    int count(const std::string& prefix) const
    {
        int n = 0;
        for (auto& e : log) n += e.rfind(prefix, 0) == 0 ? 1 : 0;
        return n;
    }
};

struct Widget final : juce::Component
{
    int paints = 0;
    void paint(juce::Graphics& g) override { ++paints; g.fillAll(juce::Colours::red); }
};

// The parent's cache: records every repaint that reached the parent (i.e. would have reached the peer).
struct ParentSpy final : juce::CachedComponentImage
{
    int* hits;
    explicit ParentSpy(int* h) : hits(h) {}
    void paint(juce::Graphics&) override {}
    bool invalidateAll() override { ++*hits; return false; }
    bool invalidate(const juce::Rectangle<int>&) override { ++*hits; return false; }
    void releaseResources() override {}
};

struct Rig
{
    juce::Component parent;
    Widget widget;
    FakeSink sink;
    int parentHits = 0;
    NativeLayerCache* cache = nullptr;
    explicit Rig(bool hideAfterPaint = false)
    {
        parent.setBounds(0, 0, 400, 300);
        parent.setVisible(true);                      // Pitfall 34: a Component is invisible by default
        widget.setBounds(10, 20, 200, 80);
        parent.addAndMakeVisible(widget);
        parent.setCachedComponentImage(new ParentSpy(&parentHits));
        cache = new NativeLayerCache(widget, sink, hideAfterPaint);
        widget.setCachedComponentImage(cache);        // JUCE owns it; its repaint() lands in RestorePending
        parentHits = 0;
        sink.log.clear();
    }
    void paintViaParent(juce::Rectangle<int> clip = { 0, 0, 200, 80 })
    {
        juce::Image img(juce::Image::ARGB, 200, 80, true);
        juce::Graphics g(img);
        g.reduceClipRegion(clip);
        cache->paint(g);
    }
    void toNative()
    {
        cache->layerDrew();
        cache->flushForTest();
        sink.log.clear();
        parentHits = 0;
    }
};
}

TEST_CASE("NativeLayerCache: starts RestorePending -- the peer paints the widget until the layer drew once",
          "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    CHECK(rig.cache->mode() == Mode::RestorePending);
    rig.widget.repaint(5, 5, 10, 10);
    CHECK(rig.parentHits == 1);                       // let through to the peer...
    CHECK(rig.sink.count("L ") == 1);                 // ...AND the (hidden) layer is marked
    rig.paintViaParent();
    CHECK(rig.widget.paints == 1);                    // in-peer painting
    CHECK(rig.sink.count("S ") == 0);                 // not shown before it drew
    rig.cache->layerDrew();
    CHECK(rig.cache->mode() == Mode::RestorePending); // shown one message-loop turn after the draw
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::Native);
    CHECK(rig.sink.count("S 1") == 1);
}

TEST_CASE("NativeLayerCache: Native swallows repaints (the peer never dirtied) and marks the layer", "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    rig.toNative();
    CHECK(rig.cache->invalidate({ 190, 70, 50, 50 }) == false);
    REQUIRE(rig.sink.log.size() == 1);
    CHECK(rig.sink.log[0] == "L 190 70 10 10");       // clipped to the widget's bounds
    CHECK(rig.cache->invalidateAll() == false);
    CHECK(rig.sink.log.back() == "L 0 0 200 80");
    rig.widget.repaint();
    CHECK(rig.parentHits == 0);
    rig.paintViaParent();
    CHECK(rig.widget.paints == 0);                    // the parent's paint of the widget draws nothing
}

TEST_CASE("NativeLayerCache: a CHILD's repaint is swallowed too in Native", "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    Widget child;
    child.setBounds(30, 10, 20, 20);
    rig.widget.addAndMakeVisible(child);
    rig.toNative();
    child.repaint();
    CHECK(rig.parentHits == 0);
    REQUIRE(rig.sink.count("L ") == 1);
    CHECK(rig.sink.log.back() == "L 30 10 20 20");    // in the widget's coordinates
}

TEST_CASE("NativeLayerCache: Native -> Fallback hides the layer and hands the widget to the peer SYNCHRONOUSLY (I1)",
          "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    rig.toNative();
    rig.cache->setFallback(true);
    CHECK(rig.cache->mode() == Mode::Fallback);       // no pending turn
    CHECK(rig.sink.count("S 0") == 1);
    CHECK(rig.sink.count("P 0 0 200 80") == 1);       // straight to the peer, same turn
    CHECK(rig.parentHits >= 1);                       // and JUCE's own repaint went through
    rig.paintViaParent();
    CHECK(rig.widget.paints == 1);
    rig.cache->setFallback(true);                     // idempotent
    CHECK(rig.sink.count("S 0") == 1);
}

TEST_CASE("NativeLayerCache: Fallback is transparent -- repaints reach the peer, the layer hears nothing",
          "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    rig.toNative();
    rig.cache->setFallback(true);
    rig.sink.log.clear();
    rig.parentHits = 0;
    CHECK(rig.cache->invalidate({ 0, 0, 10, 10 }) == true);
    CHECK(rig.cache->invalidateAll() == true);
    rig.widget.repaint();
    CHECK(rig.parentHits == 1);
    CHECK(rig.sink.log.empty());
}

TEST_CASE("NativeLayerCache: Fallback -> Native draws the hidden layer first and shows it one turn later",
          "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    rig.toNative();
    rig.cache->setFallback(true);
    rig.sink.log.clear();
    rig.cache->setFallback(false);
    CHECK(rig.cache->mode() == Mode::RestorePending);
    REQUIRE(rig.sink.log.size() == 1);
    CHECK(rig.sink.log[0] == "L 0 0 200 80");         // marked while still hidden
    rig.parentHits = 0;
    rig.widget.repaint();
    CHECK(rig.parentHits == 1);                       // the peer keeps the widget fresh meanwhile
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::RestorePending); // no draw yet -> never shown
    CHECK(rig.sink.count("S 1") == 0);
    rig.cache->layerDrew();
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::Native);
    REQUIRE(rig.sink.log.size() >= 2);
    CHECK(rig.sink.log[rig.sink.log.size() - 2] == "S 1");
    CHECK(rig.sink.log.back() == "P 0 0 200 80");     // a non-opaque widget: what lies beneath is repainted too
    rig.widget.setOpaque(true);                       // an opaque widget covers it all: no extra peer pass
    rig.cache->setFallback(true);
    rig.cache->setFallback(false);
    rig.cache->layerDrew();
    rig.cache->flushForTest();
    CHECK(rig.sink.log.back() == "S 1");
    rig.widget.setOpaque(false);
    rig.cache->setFallback(true);                     // and a new overlay hides it again at once
    CHECK(rig.cache->mode() == Mode::Fallback);
    CHECK(rig.sink.log.back() == "P 0 0 200 80");
}

TEST_CASE("NativeLayerCache: an overlay during RestorePending returns to Fallback without a show", "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig;
    rig.toNative();
    rig.cache->setFallback(true);
    rig.cache->setFallback(false);
    rig.cache->layerDrew();                           // the show is queued...
    rig.cache->setFallback(true);                     // ...but a new overlay arrives first
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::Fallback);
    CHECK(rig.sink.count("S 1") == 0);
}

TEST_CASE("NativeLayerCache teeth (hideAfterPaint = the plan body): the layer stays up until the in-peer paint landed",
          "[idlepaint][cache]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig rig(true);
    rig.toNative();
    rig.cache->setFallback(true);
    CHECK(rig.cache->mode() == Mode::FallbackPending);
    CHECK(rig.sink.count("S 0") == 0);                // an overlay would be covered now: the gate's teeth
    rig.paintViaParent({ 0, 0, 50, 50 });             // a partial repaint does not hide
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::FallbackPending);
    rig.paintViaParent();                             // the whole widget fresh beneath the layer
    CHECK(rig.cache->mode() == Mode::FallbackPending);
    rig.cache->flushForTest();
    CHECK(rig.cache->mode() == Mode::Fallback);
    CHECK(rig.sink.log.back() == "S 0");
}
