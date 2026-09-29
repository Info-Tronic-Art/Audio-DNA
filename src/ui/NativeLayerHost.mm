// s-rta-0928b idlepaint (Pitfall NN): see NativeLayerHost.h. macOS only; built without ARC (like JUCE's own Obj-C++).
#import <AppKit/AppKit.h>                          // before JUCE: its CoreGraphics helpers use AppKit types
#import <objc/message.h>                           // (and its ObjC helpers the runtime)
#import <objc/runtime.h>
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1               // juce::CFUniquePtr (juce_core.h), used by ...
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1   // ... juce::CoreGraphicsContext (juce_graphics.h)
#include "ui/NativeLayerHost.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <cstdlib>
#include <cstring>

@interface ADNANativeLayerView : NSView
@property (nonatomic, assign) NativeLayerHost* owner;
@end

@implementation ADNANativeLayerView
- (BOOL) isFlipped { return YES; }                                   // the peer's convention -> JUCE coordinates
- (BOOL) isOpaque  { return self.owner != nullptr && self.owner->targetIsOpaque(); }
- (NSView*) hitTest: (NSPoint) p { (void) p; return nil; }         // every mouse / drag event reaches the JUCE peer view
- (BOOL) acceptsFirstResponder { return NO; }                        // never key
- (BOOL) isAccessibilityElement { return NO; }
- (void) drawRect: (NSRect) r
{
    if (self.owner != nullptr)
        self.owner->drawLayer ((void*) [[NSGraphicsContext currentContext] CGContext], (float) r.size.width, (float) r.size.height);
}
@end

namespace
{
#if AUDIODNA_TEST_SERVER
const char* envOr(const char* name, const char* dflt)
{
    const char* v = std::getenv(name);
    return v != nullptr ? v : dflt;
}
#endif
}

struct NativeLayerHost::Impl
{
    ADNANativeLayerView* view = nil;                    // retained by hostComp's attachment
    std::unique_ptr<juce::NSViewComponent> hostComp;
#if AUDIODNA_TEST_SERVER
    // ADNA_UI_OVERLAY_WITNESS=1 (adoption I1 / I4): per vblank, was an in-peer overlay up over the widget while its layer
    // still showed; and how many vblanks after an overlay closed the layer showed again.
    juce::VBlankAttachment vblank;
    bool overlayWasUp = false;
    int framesSinceClose = -1;
#endif
};

std::unique_ptr<NativeLayerHost> NativeLayerHost::attach(juce::Component& target, OverlayWatch& watch, uipaint::Layer id)
{
#if AUDIODNA_TEST_SERVER
    if (std::strcmp(envOr("ADNA_UI_NATIVE_LAYERS", "1"), "0") == 0)
        return nullptr;                                  // the within-build counterfactual (probe row x1)
#endif
    return std::unique_ptr<NativeLayerHost>(new NativeLayerHost(target, watch, id));
}

NativeLayerHost::NativeLayerHost(juce::Component& target, OverlayWatch& watch, uipaint::Layer id)
    : target_(target), watch_(watch), id_(id), impl_(std::make_unique<Impl>())
{
    bool hideAfterPaint = false;
#if AUDIODNA_TEST_SERVER
    const char* teeth = envOr("ADNA_UI_NATIVE_LAYERS_TEETH", "");
    teethShift_ = std::strcmp(teeth, "shift") == 0;       // probe v0: the layers paint 1 px right
    hideAfterPaint = std::strcmp(teeth, "asynchide") == 0; // probe v2b: the plan body's late hide
#endif

    // The peer's own layer set-up (juce_NSViewComponentPeer_mac.mm: wantsLayer, redraw policy, async drawing).
    ADNANativeLayerView* view = [[ADNANativeLayerView alloc] initWithFrame: NSMakeRect(0, 0, 1, 1)];
    view.owner = this;
    [view setWantsLayer: YES];
    [view setLayerContentsRedrawPolicy: NSViewLayerContentsRedrawDuringViewResize];
    [view layer].drawsAsynchronously = YES;
    [view setAlphaValue: 0.0];                           // hidden until the layer has drawn its first frame
    impl_->view = view;

    impl_->hostComp = std::make_unique<juce::NSViewComponent>();
    impl_->hostComp->setInterceptsMouseClicks(false, false);
    impl_->hostComp->setAccessible(false);
    impl_->hostComp->setView(view);
    [view release];                                      // the attachment retains it
    target_.addAndMakeVisible(*impl_->hostComp);
    impl_->hostComp->setBounds(target_.getLocalBounds());
    target_.addComponentListener(this);

    cache_ = new NativeLayerCache(target_, *this, hideAfterPaint);   // starts RestorePending: the peer paints it
    target_.setCachedComponentImage(cache_);             // JUCE owns it
    watch_.addClient(this);
    lastRects_ = watch_.visibleOverlayScreenRects();
    apply();
    layerNeedsDisplay(target_.getLocalBounds());
    publishMode();

#if AUDIODNA_TEST_SERVER
    if (std::strcmp(envOr("ADNA_UI_OVERLAY_WITNESS", "0"), "1") == 0)
    {
        impl_->vblank = juce::VBlankAttachment(&target_, [this] {
            auto& c = uipaint::counters();
            const bool up = target_.isShowing()
                         && OverlayWatch::intersectsAny(target_.getScreenBounds(), watch_.visibleOverlayScreenRects());
            if (up && shown_)
                c.overlayCoveredFrames.fetch_add(1, std::memory_order_relaxed);
            if (up)
            {
                impl_->overlayWasUp = true;
                impl_->framesSinceClose = -1;
            }
            else if (impl_->overlayWasUp)
            {
                ++impl_->framesSinceClose;                // 0 = the first vblank after the close
                if (shown_)
                {
                    c.restoreFramesLast = impl_->framesSinceClose;
                    if (impl_->framesSinceClose > c.restoreFramesMax.load(std::memory_order_relaxed))
                        c.restoreFramesMax = impl_->framesSinceClose;
                    impl_->overlayWasUp = false;
                }
            }
        });
    }
#endif
}

NativeLayerHost::~NativeLayerHost()
{
#if AUDIODNA_TEST_SERVER
    impl_->vblank = juce::VBlankAttachment();
#endif
    watch_.removeClient(this);
    target_.removeComponentListener(this);
    target_.setCachedComponentImage(nullptr);            // deletes cache_; JUCE repaints the widget in-peer
    cache_ = nullptr;
    impl_->view.owner = nullptr;
    impl_->hostComp->setView(nullptr);
    target_.removeChildComponent(impl_->hostComp.get());
    uipaint::counters().layerMode[id_] = -1;
}

void NativeLayerHost::overlaysChanged(const std::vector<juce::Rectangle<int>>& screenRects)
{
    lastRects_ = screenRects;
    apply();
}

void NativeLayerHost::setForcedFallback(bool on)
{
    forced_ = on;
    apply();
}

void NativeLayerHost::apply()
{
    if (cache_ == nullptr)
        return;
    const bool overlayHit = target_.isShowing() && OverlayWatch::intersectsAny(target_.getScreenBounds(), lastRects_);
    cache_->setFallback(forced_ || overlayHit);
    publishMode();
}

void NativeLayerHost::publishMode()
{
    if (cache_ != nullptr)
        uipaint::counters().layerMode[id_] = static_cast<int>(cache_->mode());
}

void NativeLayerHost::componentMovedOrResized(juce::Component&, bool, bool)
{
    impl_->hostComp->setBounds(target_.getLocalBounds());
    apply();
}

void NativeLayerHost::componentVisibilityChanged(juce::Component&)
{
    apply();
}

void NativeLayerHost::layerNeedsDisplay(juce::Rectangle<int> a)
{
    if (impl_->view != nil)
        [impl_->view setNeedsDisplayInRect: NSMakeRect(a.getX(), a.getY(), a.getWidth(), a.getHeight())];
}

void NativeLayerHost::layerShown(bool shown)
{
    if (impl_->view != nil)
        [impl_->view setAlphaValue: shown ? 1.0 : 0.0];
    shown_ = shown;
    if (!shown)
        uipaint::counters().layerFallbacks.fetch_add(1, std::memory_order_relaxed);
    publishMode();
}

void NativeLayerHost::peerNeedsDisplay(juce::Rectangle<int> a)
{
    // Straight to AppKit (JUCE's own repaint() waits for the next vblank): the in-peer widget lands in the same display
    // pass that hides the layer.
    if (auto* peer = target_.getPeer())
    {
        const auto r = peer->getAreaCoveredBy(target_);
        const auto p = a.translated(r.getX(), r.getY());
        [(NSView*) peer->getNativeHandle() setNeedsDisplayInRect: NSMakeRect(p.getX(), p.getY(), p.getWidth(), p.getHeight())];
    }
}

void NativeLayerHost::drawLayer(void* cgv, float w, float h)
{
    auto cg = static_cast<CGContextRef>(cgv);
    if (cg == nullptr || w < 1.0f || h < 1.0f || cache_ == nullptr)
        return;                                                                       // (peer: drawRect)
    auto& c = uipaint::counters();
    if (c.peerLayerBacked.load(std::memory_order_relaxed) < 0)
    {
        NSView* super = [impl_->view superview];
        c.peerLayerBacked = (super != nil && [super wantsLayer] && [super layer] != nil) ? 1 : 0;
    }
    if (!target_.isOpaque())
        CGContextClearRect(cg, CGContextGetClipBoundingBox(cg));                     // (peer: drawRectWithContext)
    const auto height = target_.getHeight();
    CGContextConcatCTM(cg, CGAffineTransformMake(1, 0, 0, -1, 0, height));            // (peer: renderRect)
    juce::CoreGraphicsContext context(cg, (float) height);
#if AUDIODNA_TEST_SERVER
    const auto t0 = std::chrono::steady_clock::now();   // s-rta-0929 g4cpu c1: the layer's JUCE paint time (TEST witness)
    const auto cpu0 = uipaint::threadCpuUs();
#endif
    {
        juce::Graphics g(context);
        if (teethShift_)
            g.setOrigin(1, 0);                                                        // probe v0's teeth
        target_.paintEntireComponent(g, false);                                       // = paintWithinParentContext
    }
    c.layerDraws[id_].fetch_add(1, std::memory_order_relaxed);
#if AUDIODNA_TEST_SERVER
    c.layerDrawUs[id_].fetch_add(static_cast<uint64_t>(uipaint::threadCpuUs() - cpu0), std::memory_order_relaxed);
    c.layerDrawWallUs[id_].fetch_add(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
                                         std::chrono::steady_clock::now() - t0).count()), std::memory_order_relaxed);
#endif
    cache_->layerDrew();
}
