# juce_hooks.py -- applies the TEMPORARY diag-idle timing hooks to the PRIVATE JUCE copy (never build/_deps)
J='/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle/juce-src/modules'
def sub(path, old, new, count=1):
    p=J+'/'+path; s=open(p).read()
    if new in s: print('already', path); return
    if s.count(old)==0: print('SKIP (old text absent; assumed applied by an earlier round):', path, old[:50].replace(chr(10),' ')); return
    assert s.count(old)==count, (path, old[:60], s.count(old)); s=s.replace(old,new); open(p,'w').write(s)
sub('juce_events/messages/juce_MessageManager.h', '''class MessageManagerLock;
class ThreadPoolJob;''', '''class MessageManagerLock;
class ThreadPoolJob;

// DIAG-IDLE TEMPORARY (private JUCE copy, lane diag-idle): timing hook, null unless the app installs it.
struct JUCE_API DiagHook
{
    using Fn = void (*) (int kind, const char* name, double t0Ms, double durMs, double extra);
    static Fn fn;
    static double nowMs() noexcept;
};
#define JUCE_DIAG_T0(v) const double v = (::juce::DiagHook::fn != nullptr ? ::juce::DiagHook::nowMs() : 0.0)
#define JUCE_DIAG_END(v, kind, name, extra) do { if (auto* diagFn_ = ::juce::DiagHook::fn) diagFn_ (kind, name, v, ::juce::DiagHook::nowMs() - v, extra); } while (false)
''')
sub('juce_events/messages/juce_MessageManager.cpp', '''namespace juce
{
''', '''namespace juce
{

DiagHook::Fn DiagHook::fn = nullptr;
double DiagHook::nowMs() noexcept
{
    return std::chrono::duration<double, std::milli> (std::chrono::steady_clock::now().time_since_epoch()).count();
}
''')
sub('juce_events/native/juce_MessageQueue_mac.h', '''            JUCE_TRY
            {
                nextMessage->messageCallback();
            }
            JUCE_CATCH_EXCEPTION''', '''            JUCE_DIAG_T0 (diagT0);
            JUCE_TRY
            {
                nextMessage->messageCallback();
            }
            JUCE_CATCH_EXCEPTION
            JUCE_DIAG_END (diagT0, 1, typeid (*nextMessage).name(), (double) messages.size());''')
sub('juce_events/timers/juce_Timer.cpp', '''            JUCE_TRY
            {
                timer->timerCallback();
            }
            JUCE_CATCH_EXCEPTION''', '''            const char* diagName = typeid (*timer).name();
            const double diagPeriod = (double) timer->timerPeriodMs;
            JUCE_DIAG_T0 (diagT0);
            JUCE_TRY
            {
                timer->timerCallback();
            }
            JUCE_CATCH_EXCEPTION
            JUCE_DIAG_END (diagT0, 2, diagName, diagPeriod);''')
sub('juce_events/broadcasters/juce_AsyncUpdater.cpp', '''        if (shouldDeliver.compareAndSetBool (0, 1))
            owner.handleAsyncUpdate();''', '''        if (shouldDeliver.compareAndSetBool (0, 1))
        {
            const char* diagName = typeid (owner).name();
            JUCE_DIAG_T0 (diagT0);
            owner.handleAsyncUpdate();
            JUCE_DIAG_END (diagT0, 3, diagName, 0.0);
        }''')
sub('juce_gui_basics/components/juce_Component.cpp', '''    if (flags.dontClipGraphicsFlag && getNumChildComponents() == 0)
    {
        paint (g);
    }
    else
    {
        Graphics::ScopedSaveState ss (g);

        if (! (detail::ComponentHelpers::clipObscuredRegions (*this, g, clipBounds, {}) && g.isClipEmpty()))
            paint (g);
    }
''', '''    {
    JUCE_DIAG_T0 (diagT0);
    if (flags.dontClipGraphicsFlag && getNumChildComponents() == 0)
    {
        paint (g);
    }
    else
    {
        Graphics::ScopedSaveState ss (g);

        if (! (detail::ComponentHelpers::clipObscuredRegions (*this, g, clipBounds, {}) && g.isClipEmpty()))
            paint (g);
    }
    JUCE_DIAG_END (diagT0, 4, typeid (*this).name(), (double) clipBounds.getWidth() * (double) clipBounds.getHeight());
    }
''')
sub('juce_gui_basics/components/juce_Component.cpp', '''    Graphics::ScopedSaveState ss (g);
    paintOverChildren (g);
}''', '''    Graphics::ScopedSaveState ss (g);
    JUCE_DIAG_T0 (diagT0b);
    paintOverChildren (g);
    JUCE_DIAG_END (diagT0b, 4, typeid (*this).name(), -1.0);
}''')
sub('juce_gui_basics/windows/juce_ComponentPeer.cpp', '''    JUCE_TRY
    {
        component.paintEntireComponent (g, true);
    }
    JUCE_CATCH_EXCEPTION
''', '''    JUCE_DIAG_T0 (diagT0);
    JUCE_TRY
    {
        component.paintEntireComponent (g, true);
    }
    JUCE_CATCH_EXCEPTION
    JUCE_DIAG_END (diagT0, 6, typeid (component).name(), 0.0);
''')
sub('juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm', '''        auto* cg = (CGContextRef) [[NSGraphicsContext currentContext] CGContext];
        drawRectWithContext (cg, r);''', '''        auto* cg = (CGContextRef) [[NSGraphicsContext currentContext] CGContext];
        JUCE_DIAG_T0 (diagT0);
        drawRectWithContext (cg, r);
        JUCE_DIAG_END (diagT0, 5, typeid (component).name(), (double) (r.size.width * r.size.height));''')
sub('juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm', '''        for (auto& i : deferredRepaints)
            [view setNeedsDisplayInRect: makeNSRect (i)];
''', '''        {
            double diagArea = 0.0;
            for (auto& i : deferredRepaints)
                diagArea += (double) (i.getWidth() * i.getHeight());
            if (auto* diagFn_ = ::juce::DiagHook::fn)
                diagFn_ (7, typeid (component).name(), ::juce::DiagHook::nowMs(), 0.0, diagArea);
        }
        for (auto& i : deferredRepaints)
            [view setNeedsDisplayInRect: makeNSRect (i)];
''')
G='juce_opengl/opengl/juce_OpenGLContext.cpp'
sub(G, '''    void paint (Graphics&) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            updateViewportSize();
        }''', '''    void paint (Graphics&) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            JUCE_DIAG_T0 (diagT0);
            updateViewportSize();
            JUCE_DIAG_END (diagT0, 11, "gl.cachedImage.paint.updateViewportSize", 0.0);
        }''')
sub(G, '''            scopedLock.emplace (mmLock);
''', '''            JUCE_DIAG_T0 (diagLockWait);
            scopedLock.emplace (mmLock);
            JUCE_DIAG_END (diagLockWait, 12, "gl.mmLock.wait", 0.0);
''')
sub(G, '''                    paintComponent (currentAreaAndScale);
''', '''                    JUCE_DIAG_T0 (diagPc);
                    paintComponent (currentAreaAndScale);
                    JUCE_DIAG_END (diagPc, 13, "gl.paintComponent.underMMLock", 0.0);
''')
sub(G, '''        bufferSwapper.swap();
        return RenderStatus::nominal;''', '''        {
            JUCE_DIAG_T0 (diagSw);
            bufferSwapper.swap();
            JUCE_DIAG_END (diagSw, 14, "gl.swap", 0.0);
        }
        return RenderStatus::nominal;''')
sub(G, '''                const auto status = x->renderFrame (messageManagerLock);
''', '''                JUCE_DIAG_T0 (diagRf);
                const auto status = x->renderFrame (messageManagerLock);
                JUCE_DIAG_END (diagRf, 15, "gl.renderFrame", (double) (int) status);
''')
print('ok')
# --- round 2: who invalidates (kind 8), and the drawRect rectangle (kind 9: extra = x*1e4+y, dur = w*1e4+h)
C='juce_gui_basics/components/juce_Component.cpp'
sub(C, '''void Component::repaint()
{
    internalRepaintUnchecked (getLocalBounds(), true);
}

void Component::repaint (int x, int y, int w, int h)
{
    internalRepaint ({ x, y, w, h });
}

void Component::repaint (Rectangle<int> area)
{
    internalRepaint (area);
}''', '''void Component::repaint()
{
    if (auto* diagFn_ = ::juce::DiagHook::fn)
        if (flags.visibleFlag && isShowing())
            diagFn_ (8, typeid (*this).name(), ::juce::DiagHook::nowMs(), 0.0, (double) getWidth() * (double) getHeight());
    internalRepaintUnchecked (getLocalBounds(), true);
}

void Component::repaint (int x, int y, int w, int h)
{
    if (auto* diagFn_ = ::juce::DiagHook::fn)
        if (flags.visibleFlag && isShowing())
            diagFn_ (8, typeid (*this).name(), ::juce::DiagHook::nowMs(), 1.0, (double) w * (double) h);
    internalRepaint ({ x, y, w, h });
}

void Component::repaint (Rectangle<int> area)
{
    if (auto* diagFn_ = ::juce::DiagHook::fn)
        if (flags.visibleFlag && isShowing())
            diagFn_ (8, typeid (*this).name(), ::juce::DiagHook::nowMs(), 1.0, (double) area.getWidth() * (double) area.getHeight());
    internalRepaint (area);
}''')
sub('juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm', '''        JUCE_DIAG_T0 (diagT0);
        drawRectWithContext (cg, r);''', '''        JUCE_DIAG_T0 (diagT0);
        if (auto* diagFn_ = ::juce::DiagHook::fn)
            diagFn_ (9, "drawRect.rect", diagT0, (double) std::round (r.size.width) * 1e4 + (double) std::round (r.size.height),
                     (double) std::round (r.origin.x) * 1e4 + (double) std::round (r.origin.y));
        drawRectWithContext (cg, r);''')
print('ok2')
# --- round 3: ADNA_DIAG_METAL=1 -> JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS metal layer renderer at RUNTIME
# (macro compiled in, the renderer + the multi-rect branch only when the env var is set: unset == stock JUCE path);
# kind 10 = [view getRectsBeingDrawn] count (extra) and summed area (dur) per drawRect.
P='juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm'
sub(P, '''namespace juce
{''', '''// DIAG-IDLE TEMPORARY: compiled in, runtime-gated by ADNA_DIAG_METAL (see diagMetalOn)
#ifndef JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS
 #define JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS 1
#endif

namespace juce
{
static bool diagMetalOn()
{
    static const bool on = [] { const char* e = std::getenv ("ADNA_DIAG_METAL"); return e != nullptr && e[0] == '1'; }();
    return on;
}''')
sub(P, '''        if (@available (macOS 10.14, *))
        {
            metalRenderer = CoreGraphicsMetalLayerRenderer::create();
            layerDelegate.reset (JuceCALayerDelegate::construct (this));
        }''', '''        if (@available (macOS 10.14, *))
        {
            if (diagMetalOn())
            {
                metalRenderer = CoreGraphicsMetalLayerRenderer::create();
                layerDelegate.reset (JuceCALayerDelegate::construct (this));
            }
        }''')
sub(P, '''        if (usingCoreGraphics && metalRenderer == nullptr)
        {
            const NSRect* rects = nullptr;
            NSInteger numRects = 0;
            [view getRectsBeingDrawn: &rects count: &numRects];
''', '''        if (usingCoreGraphics && metalRenderer == nullptr && diagMetalOn())
        {
            const NSRect* rects = nullptr;
            NSInteger numRects = 0;
            [view getRectsBeingDrawn: &rects count: &numRects];
''')
sub(P, '''        JUCE_DIAG_T0 (diagT0);
        if (auto* diagFn_ = ::juce::DiagHook::fn)
            diagFn_ (9,''', '''        JUCE_DIAG_T0 (diagT0);
        if (auto* diagFn_ = ::juce::DiagHook::fn)
        {
            const NSRect* diagRects = nullptr;
            NSInteger diagNum = 0;
            [view getRectsBeingDrawn: &diagRects count: &diagNum];
            double diagA = 0.0;
            for (NSInteger k = 0; k < diagNum; ++k)
                diagA += diagRects[k].size.width * diagRects[k].size.height;
            diagFn_ (10, "drawRect.rectsBeingDrawn", diagT0, diagA, (double) diagNum);
        }
        if (auto* diagFn_ = ::juce::DiagHook::fn)
            diagFn_ (9,''')
# metal path: time each drawRectangleList (kind 16) -- it runs from displayLayer, not drawRect
sub(P, '''        deferredRepaints = metalRenderer->drawRectangleList (static_cast<CAMetalLayer*> (layer),''', '''        JUCE_DIAG_T0 (diagMt);
        const auto diagNumRects = (double) deferredRepaints.getNumRectangles();
        const ScopeGuard diagMtEnd { [&] { JUCE_DIAG_END (diagMt, 16, "metal.drawRectangleList", diagNumRects); } };
        deferredRepaints = metalRenderer->drawRectangleList (static_cast<CAMetalLayer*> (layer),''')
print('ok3')
# --- round 4: ADNA_DIAG_DIRTYGRID=1 -> kind 19: fraction of a 32x20 cell grid over the view that AppKit reports as
# needing display ([view needsToDrawRect:]) in this drawRect (the TRUE dirty region, vs the union rect drawRect gets)
sub(P, '''            diagFn_ (10, "drawRect.rectsBeingDrawn", diagT0, diagA, (double) diagNum);
        }''', '''            diagFn_ (10, "drawRect.rectsBeingDrawn", diagT0, diagA, (double) diagNum);
            static const bool diagGrid = [] { const char* e = std::getenv ("ADNA_DIAG_DIRTYGRID"); return e != nullptr && e[0] == '1'; }();
            if (diagGrid)
            {
                const auto vb = [view bounds];
                int dirty = 0, inRect = 0;
                for (int gy = 0; gy < 20; ++gy)
                    for (int gx = 0; gx < 32; ++gx)
                    {
                        const NSRect cell = NSMakeRect (vb.origin.x + vb.size.width * gx / 32.0, vb.origin.y + vb.size.height * gy / 20.0,
                                                        vb.size.width / 32.0, vb.size.height / 20.0);
                        if (NSIntersectsRect (cell, r)) ++inRect;
                        if ([view needsToDrawRect: cell]) ++dirty;
                    }
                diagFn_ (19, "drawRect.dirtyGrid", diagT0, (double) inRect, (double) dirty);
            }
        }''')
print('ok4')
# --- round 5: kind 20 = the CGContext type AppKit hands drawRect (CGContextGetType via dlsym; a display-list context =
# drawsAsynchronously recording, a bitmap context = synchronous raster), extra = type id, dur = layer bounds area match
sub(P, '''        if (auto* diagFn_ = ::juce::DiagHook::fn)
            diagFn_ (9,''', '''        if (auto* diagFn_ = ::juce::DiagHook::fn)
        {
            using GetTypeFn = int (*) (CGContextRef);
            static const auto getType = (GetTypeFn) dlsym (RTLD_DEFAULT, "CGContextGetType");
            diagFn_ (20, "drawRect.cgContextType", diagT0, (double) (view.layer != nil && view.layer.drawsAsynchronously ? 1 : 0),
                     getType != nullptr ? (double) getType (cg) : -1.0);
        }
        if (auto* diagFn_ = ::juce::DiagHook::fn)
            diagFn_ (9,''')
print('ok5')
sub(P, '''// DIAG-IDLE TEMPORARY: compiled in, runtime-gated by ADNA_DIAG_METAL (see diagMetalOn)''', '''#include <dlfcn.h>
// DIAG-IDLE TEMPORARY: compiled in, runtime-gated by ADNA_DIAG_METAL (see diagMetalOn)''')
print('ok5b')
# --- round 6: kind 21 = main-thread CPU time consumed inside drawRect (CLOCK_THREAD_CPUTIME_ID), extra = cpu ms,
# dur = wall ms -- computing vs waiting inside a pass
sub(P, '''        JUCE_DIAG_T0 (diagT0);
        if (auto* diagFn_ = ::juce::DiagHook::fn)
        {
            const NSRect* diagRects = nullptr;''', '''        JUCE_DIAG_T0 (diagT0);
        const auto diagCpu0 = clock_gettime_nsec_np (CLOCK_THREAD_CPUTIME_ID);
        const ScopeGuard diagCpuEnd { [&] {
            if (auto* diagFn_ = ::juce::DiagHook::fn)
                diagFn_ (21, "drawRect.threadCpu", diagT0, ::juce::DiagHook::nowMs() - diagT0,
                         (double) (clock_gettime_nsec_np (CLOCK_THREAD_CPUTIME_ID) - diagCpu0) / 1.0e6);
        } };
        if (auto* diagFn_ = ::juce::DiagHook::fn)
        {
            const NSRect* diagRects = nullptr;''')
print('ok6')
