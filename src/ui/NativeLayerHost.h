#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/NativeLayerCache.h"
#include "ui/OverlayWatch.h"
#include "ui/UiPaintCounters.h"
#include <memory>
#include <vector>

// s-rta-0928b idlepaint (Pitfall 57): an always-animating panel (SignalBar, WaveformDisplay) draws in its OWN layer-backed
// NSView instead of the window's one CoreGraphics view. JUCE 8's mac peer hands AppKit every rect repainted since the
// last vblank and AppKit redraws their UNION in one drawRect, so a 30 Hz repaint at one window edge plus another at the
// opposite edge repainted the whole window 30 times a second. A layer of its own is dirty-tracked alone.
// attach(): an ADNANativeLayerView (hit-test transparent, never key, no accessibility element) is added to the peer's view
// through a juce::NSViewComponent child of the widget; a NativeLayerCache installed on the widget swallows the widget's
// repaints (and its children's) and marks the layer; the layer's drawRect runs the SAME paintEntireComponent through the
// SAME juce::CoreGraphicsContext set-up the peer uses (flip CTM, flipHeight) -- the peer's pixels. An in-peer overlay
// (OverlayWatch) that crosses the widget switches it back to JUCE painting for the overlay's lifetime.
// Never attach a widget that lives inside a juce::Viewport: a native view is clipped by the peer's view only.
// Message thread only.
#if JUCE_MAC
class NativeLayerHost final : public NativeLayerCache::Sink,
                              public OverlayWatch::Client,
                              private juce::ComponentListener
{
public:
    // nullptr when (TEST_SERVER builds) ADNA_UI_NATIVE_LAYERS=0 -- the widget then paints as it always did.
    static std::unique_ptr<NativeLayerHost> attach(juce::Component& target, OverlayWatch& watch, uipaint::Layer id);
    ~NativeLayerHost() override;

    // OverlayWatch::Client: fall back while any in-peer overlay crosses the widget.
    void overlaysChanged(const std::vector<juce::Rectangle<int>>& screenRects) override;
    // POST /api/debug/ui_native_fallback (TEST-ONLY): paint in-peer regardless of overlays (on) / back to the rule.
    void setForcedFallback(bool on);

    // NativeLayerCache::Sink
    void layerNeedsDisplay(juce::Rectangle<int> areaInComponent) override;
    void layerShown(bool shown) override;
    void peerNeedsDisplay(juce::Rectangle<int> areaInComponent) override;

    void drawLayer(void* cgContext, float w, float h);   // the NSView's drawRect:
    bool targetIsOpaque() const { return target_.isOpaque(); }

    struct Impl;   // the .mm's ObjC state (the view, the TEST-ONLY vblank witness)

private:
    NativeLayerHost(juce::Component& target, OverlayWatch& watch, uipaint::Layer id);
    void componentMovedOrResized(juce::Component&, bool, bool) override;
    void componentVisibilityChanged(juce::Component&) override;
    void apply();                                       // fallback = forced_ || an overlay crosses the widget
    void publishMode();

    juce::Component& target_;
    OverlayWatch& watch_;
    const uipaint::Layer id_;
    std::unique_ptr<Impl> impl_;
    NativeLayerCache* cache_ = nullptr;                 // owned by target_ (setCachedComponentImage)
    std::vector<juce::Rectangle<int>> lastRects_;
    bool forced_ = false;
    bool shown_ = false;
    bool teethShift_ = false;                           // TEST_SERVER: ADNA_UI_NATIVE_LAYERS_TEETH=shift

    JUCE_DECLARE_NON_COPYABLE(NativeLayerHost)
};
#else
// Non-Apple builds: no native layers -- every widget paints as it always did.
class NativeLayerHost final
{
public:
    static std::unique_ptr<NativeLayerHost> attach(juce::Component&, OverlayWatch&, uipaint::Layer) { return nullptr; }
    void setForcedFallback(bool) {}
};
#endif
