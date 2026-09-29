#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// s-rta-0928b idlepaint (Pitfall NN): the juce::CachedComponentImage that hands a widget's repaints to its own CALayer
// (NativeLayerHost). JUCE asks a component's cachedImage first on every repaint: an invalidate() that returns false
// stops the repaint before the parent / the peer (juce_Component.cpp internalRepaintUnchecked), and the parent paints
// the widget through cachedImage->paint() (paintWithinParentContext). So:
//   Native          invalidate() marks the layer and returns false -- the peer is never dirtied; the parent's paint of
//                   the widget draws nothing (the layer covers it).
//   Fallback        (an in-peer overlay crosses the widget) transparent: invalidate() returns true, paint() paints the
//                   widget exactly as JUCE does without a cache; the layer is hidden.
//   RestorePending  (the overlay went away) the peer still paints the widget AND the layer draws (while hidden); the
//                   layer is shown one message-loop turn after its first draw -- never with a stale frame.
// Native -> Fallback is SYNCHRONOUS (Harmony adoption I1): the layer is hidden and the widget's area is handed to the
// peer (peerNeedsDisplay, the same display pass) inside setFallback(true) -- an overlay is never covered by a layer.
// FallbackPending exists only as the test teeth (hideAfterPaint): the plan body's variant that hides the layer one
// turn after the in-peer repaint landed -- it lets an overlay be covered for a frame, which probe-idle-paint must see.
class NativeLayerCache final : public juce::CachedComponentImage, private juce::AsyncUpdater
{
public:
    struct Sink
    {
        virtual ~Sink() = default;
        virtual void layerNeedsDisplay(juce::Rectangle<int> areaInComponent) = 0;   // mark the layer dirty
        virtual void layerShown(bool shown) = 0;                                     // show / hide the layer
        virtual void peerNeedsDisplay(juce::Rectangle<int> areaInComponent) = 0;    // mark the peer dirty NOW
    };
    enum class Mode : int { Native = 0, RestorePending = 1, Fallback = 2, FallbackPending = 3 };

    // Starts RestorePending: the peer paints the widget until the layer has drawn its first frame.
    NativeLayerCache(juce::Component& c, Sink& s, bool hideAfterPaint = false)
        : component_(c), sink_(s), hideAfterPaint_(hideAfterPaint) {}
    ~NativeLayerCache() override { cancelPendingUpdate(); }

    void paint(juce::Graphics& g) override                       // called by the PARENT's paint (paintWithinParentContext)
    {
        if (mode_ == Mode::Native)
            return;                                              // the layer shows this rect
        component_.paintEntireComponent(g, false);               // exactly what JUCE does without a cache
        if (mode_ == Mode::FallbackPending && g.getClipBounds().contains(component_.getLocalBounds()))
            triggerAsyncUpdate();                                // teeth only: hide the layer next turn
    }
    bool invalidateAll() override { return invalidate(component_.getLocalBounds()); }
    bool invalidate(const juce::Rectangle<int>& area) override
    {
        if (mode_ == Mode::Fallback || mode_ == Mode::FallbackPending)
            return true;                                         // the peer paints it
        const auto a = area.getIntersection(component_.getLocalBounds());
        if (!a.isEmpty())
            sink_.layerNeedsDisplay(a);
        return mode_ == Mode::RestorePending;                    // Native: the peer is never dirtied
    }
    void releaseResources() override {}

    void setFallback(bool wantFallback)
    {
        if (wantFallback)
        {
            if (mode_ == Mode::Fallback || mode_ == Mode::FallbackPending)
                return;
            const bool layerWasShown = mode_ == Mode::Native;
            cancelPendingUpdate();
            if (hideAfterPaint_ && layerWasShown)
            {
                mode_ = Mode::FallbackPending;                   // teeth: the layer stays up until the paint landed
                component_.repaint();
                return;
            }
            mode_ = Mode::Fallback;
            if (layerWasShown)
                sink_.layerShown(false);
            sink_.peerNeedsDisplay(component_.getLocalBounds());
            component_.repaint();
        }
        else if (mode_ == Mode::FallbackPending)
        {
            cancelPendingUpdate();                               // the layer never went away: it only needs a redraw
            mode_ = Mode::Native;
            sink_.layerNeedsDisplay(component_.getLocalBounds());
        }
        else if (mode_ == Mode::Fallback)
        {
            mode_ = Mode::RestorePending;
            sink_.layerNeedsDisplay(component_.getLocalBounds());
        }
    }

    // The host calls this after every draw of the layer: in RestorePending the layer is shown next turn.
    void layerDrew()
    {
        if (mode_ == Mode::RestorePending)
            triggerAsyncUpdate();
    }

    Mode mode() const noexcept { return mode_; }
    void flushForTest() { handleUpdateNowIfNeeded(); }

private:
    void handleAsyncUpdate() override
    {
        if (mode_ == Mode::RestorePending)
        {
            mode_ = Mode::Native;
            sink_.layerShown(true);
        }
        else if (mode_ == Mode::FallbackPending)
        {
            mode_ = Mode::Fallback;
            sink_.layerShown(false);
        }
    }

    juce::Component& component_;
    Sink& sink_;
    const bool hideAfterPaint_;
    Mode mode_ = Mode::RestorePending;
};
