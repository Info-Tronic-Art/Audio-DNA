// test_overlay_watch -- s-rta-0928b idlepaint (plan-idlepaint.md 2.3 / 3.1 test 2). OverlayWatch (src/ui/OverlayWatch.h)
// lists the screen rects of every JUCE-drawn thing that can sit ABOVE a native-layer widget: a TooltipWindow, a child of
// the root added after the baseline (a ClipCell drag image), an explicit overlay (binding / MIDI-learn), a child of the
// top-level window that is not the content or a resizer (a PopupMenu shown withParentComponent). Headless JUCE widgets
// under ScopedJuceInitialiser_GUI (no window, no peer: "visible" = visible up to the top).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/OverlayWatch.h"
#include <memory>
#include <vector>

namespace
{
using Rects = std::vector<juce::Rectangle<int>>;

struct Spy final : OverlayWatch::Client
{
    int calls = 0;
    Rects last;
    void overlaysChanged(const Rects& r) override { ++calls; last = r; }
};

struct Root
{
    juce::Component root, a, b, c;
    std::unique_ptr<juce::TooltipWindow> tip;
    Root()
    {
        root.setBounds(0, 0, 800, 600);
        root.setVisible(true);                         // Pitfall 34
        for (auto* ch : { &a, &b, &c })
            root.addAndMakeVisible(ch);
        a.setBounds(0, 0, 800, 40);
        b.setBounds(0, 40, 800, 80);
        c.setBounds(0, 120, 800, 480);
        tip = std::make_unique<juce::TooltipWindow>(&root, 600);   // a child of root, hidden
    }
};
}

TEST_CASE("OverlayWatch: a TooltipWindow counts only while visible", "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Root r;
    OverlayWatch w(r.root);
    Spy spy;
    w.addClient(&spy);
    CHECK(w.visibleOverlayScreenRects().empty());
    r.tip->setBounds(100, 50, 120, 24);
    r.tip->setVisible(true);
    REQUIRE(w.visibleOverlayScreenRects().size() == 1);
    CHECK(w.visibleOverlayScreenRects()[0] == juce::Rectangle<int>(100, 50, 120, 24));
    CHECK(spy.last == Rects { { 100, 50, 120, 24 } });
    r.tip->setVisible(false);
    CHECK(w.visibleOverlayScreenRects().empty());
    CHECK(spy.last.empty());
}

TEST_CASE("OverlayWatch: a child added after the baseline counts; a baseline child never does", "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Root r;
    OverlayWatch w(r.root);
    r.b.setVisible(false);
    r.b.setVisible(true);
    CHECK(w.visibleOverlayScreenRects().empty());      // the SignalBar-like baseline child toggling: not an overlay
    juce::Component drag;                               // a ClipCell drag image (a child of the container)
    drag.setBounds(300, 60, 64, 48);
    r.root.addAndMakeVisible(drag);
    REQUIRE(w.visibleOverlayScreenRects().size() == 1);
    CHECK(w.visibleOverlayScreenRects()[0] == juce::Rectangle<int>(300, 60, 64, 48));
    drag.setTopLeftPosition(310, 70);                  // a drag image moves
    CHECK(w.visibleOverlayScreenRects()[0] == juce::Rectangle<int>(310, 70, 64, 48));
    r.root.removeChildComponent(&drag);
    CHECK(w.visibleOverlayScreenRects().empty());
}

TEST_CASE("OverlayWatch: an explicit overlay counts only while visible", "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Root r;
    juce::Component binding;                            // BindingOverlay: a hidden baseline child, full-window
    binding.setBounds(0, 0, 800, 600);
    r.root.addChildComponent(binding);
    OverlayWatch w(r.root);
    w.addOverlay(&binding);
    CHECK(w.visibleOverlayScreenRects().empty());
    binding.setVisible(true);
    CHECK(w.visibleOverlayScreenRects() == Rects { { 0, 0, 800, 600 } });
    binding.setVisible(false);
    CHECK(w.visibleOverlayScreenRects().empty());
}

TEST_CASE("OverlayWatch: the top-level window's children (a parented PopupMenu) count; the content and resizers do not",
          "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Root r;
    OverlayWatch w(r.root);                             // built before the root has a window (MainComponent's ctor)
    juce::Component window;
    window.setBounds(0, 0, 800, 600);
    window.setVisible(true);
    window.addAndMakeVisible(r.root);                   // setContentOwned: the watch re-attaches to the new top-level
    juce::ResizableCornerComponent corner(&window, nullptr);
    corner.setBounds(784, 584, 16, 16);
    window.addAndMakeVisible(corner);
    CHECK(w.visibleOverlayScreenRects().empty());
    juce::Component menu;                               // PopupMenu's window with a parent component
    menu.setBounds(600, 60, 180, 90);
    window.addAndMakeVisible(menu);
    CHECK(w.visibleOverlayScreenRects() == Rects { { 600, 60, 180, 90 } });
    menu.setVisible(false);
    CHECK(w.visibleOverlayScreenRects().empty());
    window.removeChildComponent(&r.root);               // teardown order of the app: the content leaves first
}

TEST_CASE("OverlayWatch: the pure rules", "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::Component content, other, a;
    juce::ResizableCornerComponent corner(&content, nullptr);
    juce::ResizableBorderComponent border(&content, nullptr);
    CHECK_FALSE(OverlayWatch::isWindowOverlay(content, content));
    CHECK_FALSE(OverlayWatch::isWindowOverlay(corner, content));
    CHECK_FALSE(OverlayWatch::isWindowOverlay(border, content));
    CHECK(OverlayWatch::isWindowOverlay(other, content));

    juce::TooltipWindow tip(nullptr, 600);
    std::vector<juce::Component::SafePointer<juce::Component>> baseline { &a, &tip };
    CHECK(OverlayWatch::isRootOverlay(tip, baseline));  // a TooltipWindow by type, even in the baseline
    CHECK_FALSE(OverlayWatch::isRootOverlay(a, baseline));
    CHECK(OverlayWatch::isRootOverlay(other, baseline));

    const juce::Rectangle<int> bar(0, 40, 800, 80);
    CHECK_FALSE(OverlayWatch::intersectsAny(bar, {}));
    CHECK_FALSE(OverlayWatch::intersectsAny(bar, { { 0, 120, 100, 10 } }));   // touching the bottom edge: no
    CHECK_FALSE(OverlayWatch::intersectsAny(bar, { { 800, 40, 10, 10 } }));   // touching the right edge: no
    CHECK(OverlayWatch::intersectsAny(bar, { { 0, 0, 10, 10 }, { 0, 119, 1, 1 } }));
}

TEST_CASE("OverlayWatch: a client hears each change once, not each event", "[idlepaint][overlay]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Root r;
    OverlayWatch w(r.root);
    Spy spy;
    w.addClient(&spy);
    r.a.setBounds(0, 0, 800, 41);                       // a baseline child moving: nothing changed
    r.b.setVisible(false);
    r.b.setVisible(true);
    CHECK(spy.calls == 0);
    r.tip->setBounds(10, 10, 50, 20);                   // hidden: still nothing
    CHECK(spy.calls == 0);
    r.tip->setVisible(true);
    CHECK(spy.calls == 1);
    r.tip->setBounds(10, 10, 50, 20);                   // same rect again
    CHECK(spy.calls == 1);
    r.tip->setBounds(12, 10, 50, 20);
    CHECK(spy.calls == 2);
    w.removeClient(&spy);
    r.tip->setVisible(false);
    CHECK(spy.calls == 2);
}
