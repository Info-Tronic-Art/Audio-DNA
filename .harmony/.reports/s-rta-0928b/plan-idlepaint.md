# plan-idlepaint -- s-rta-0928b START HERE item 1: the always-present idle UI stall

Architect: Fable (Law #11 row 2). Executes: an opus builder in worktree `.claude/worktrees/rta0928b-idlepaint`, branch
`lane/idlepaint`, branched from main AFTER lane/mediaopen and lane/video merge (section 6). Inputs read in full:
`.harmony/.reports/s-rta-0928b/diag-idle.md` + `diag-idle-tools/` (arms-table, lagclass-*, compclass-*, rects-*, grid-*,
dispphase-*, ctx-*, instr.diff, run_arm.sh, idle.py, psdiff.py) + `audit-diag-idle.md`; CLAUDE.md UI Patterns + rules 1-4;
`docs/claude/architecture.md`; JUCE 8.0.4 (`build/_deps/juce-src`, cited below); the app's UI sources at main 328301d.
Every `file:line` below was re-read at main HEAD 328301d unless marked INFERRED / ASSUMED.

QUESTION: With nothing happening the message thread is busy 334 ms/s (537 on a 16-image deck) and stalls ~18 ms about
three times a second, because four always-animating widgets at opposite window edges (4 LayerStrip playheads 30 Hz,
WaveformDisplay 30 Hz, SignalBar 30 Hz, TopBar beat wheel 15 Hz) are invalidated each vblank and JUCE 8's CoreGraphics
peer redraws the UNION of the dirty rects in one drawRect = the whole window (diag-idle C1 + C2, 98.5 % of the lag).
What do we build so the UI looks and behaves identically but the peer stops repainting the window at idle?

APPROACH (decided): keep every periodic repaint OUT of the peer's dirty union, by two mechanisms and no others.
  (A) F1(a) for the two full-width panels that are direct children of MainComponent: SignalBar and WaveformDisplay each
      draw in their OWN layer-backed NSView (`NativeLayerHost`), added to the peer's view through `juce::NSViewComponent`,
      hit-test transparent. A custom `juce::CachedComponentImage` (`NativeLayerCache`) installed on the widget swallows
      every `repaint()` from the widget or its children (returns false: the peer is never dirtied) and marks the layer;
      the layer's `drawRect:` runs the SAME `paintEntireComponent` through the SAME `juce::CoreGraphicsContext` the peer
      uses, so the pixels are the peer's pixels. AppKit dirty-tracks each layer alone: no union with anything.
  (B) F4 for the LayerStrips (they live inside DeckView's `juce::Viewport` -- a native view cannot be clipped by a JUCE
      viewport, and 4-8 more layers x 30 Hz would eat the budget): the transport rect repaints only when the pixels it
      paints would change (`LayerStrip::transportViewOf`), and the per-tick clip-name repaint goes (nothing time-varying
      is painted there). `syncFromModel()` runs every tick as today (Pitfall 41).
  (C) F6 for the 10 Hz `ClipInspector::refresh()` repaint (the residual's 54 %): repaint only when a painted input
      changed (`paintKeyNow()`); the children repaint themselves.
  (D) The TopBar beat wheel stays in-peer: a 75x34 rect at 15 Hz costs ~0.7 ms a pass (lagclass-base "TOP BAR only",
      13.5 ms/s) and, with (A)-(C), has nothing left to union with -- the FULL-window passes (C2) disappear by
      construction. F5 is not needed.
  (E) Z-order: a native view sits ABOVE all JUCE content, so an in-peer overlay -- a PopupMenu shown with
      `withParentComponent(getTopLevelComponent())` (the mandated pattern), the TooltipWindow (parented to MainComponent,
      `MainComponent.cpp:512`), a ClipCell drag image (`ClipCell.cpp:243` -> a MainComponent child), the binding /
      MIDI-learn overlays -- that INTERSECTS a native widget switches that widget back to JUCE painting for the overlay's
      lifetime (`OverlayWatch` -> `NativeLayerCache::setFallback`). Behaviour while an overlay is up = today's, exactly.
Predicted after (INFERRED, derivation in 5.2): IDLE-HB-card window max median 2-5 ms (main 17.9-22.6), main-thread CPU
75-115 ms/s (main 321-345); IDLE-HB-many16 2-5 ms / 75-115 (main 17.8 / 531). Both gate rows RED on main today,
GREEN after, measured by `.harmony/probe-idle-paint.sh` (section 5).

---------------------------------------------------------------------------------------------------------------------
## 0. Facts re-derived from source (the mechanism and the four triggers)

| claim | where | label |
|---|---|---|
| `NSViewComponentPeer::repaint` only collects rects (`deferredRepaints.add`) | `juce_NSViewComponentPeer_mac.mm:1069-1077` | VERIFIED |
| `onVBlank` -> `setNeedsDisplayRectangles` -> one `setNeedsDisplayInRect:` per rect, then `deferredRepaints.clear()` | `:1084-1116` | VERIFIED |
| `drawRect:` -> `drawRectWithContext` -> `renderRect`: `CGContextConcatCTM(1,0,0,-1,0,height)`, `CoreGraphicsContext context(cg, height)`, `handlePaint(context)`; non-opaque component -> `CGContextClearRect` first | `:962-976`, `:1029-1036` | VERIFIED |
| `getRectsBeingDrawn` gives 1 rect on 10.13+; JUCE's only alternative is the Metal layer renderer (F2) | `:988-1005`, `BREAKING_CHANGES.md:1811-1834` | VERIFIED |
| peer view: `wantsLayer YES`, `NSViewLayerContentsRedrawDuringViewResize`, `layer.drawsAsynchronously = YES`; `isFlipped` YES; `isOpaque` = component.isOpaque() | `:216-221`, `:2628`, `:2131-2135` | VERIFIED |
| `Component::internalRepaintUnchecked`: a `cachedImage` whose `invalidate()` returns false STOPS the repaint before the peer / parent | `juce_Component.cpp:1603-1636` (gate `:1611-1614`) | VERIFIED |
| the parent paints a child through `paintWithinParentContext`: `g.setOrigin(getPosition()); cachedImage ? cachedImage->paint(g) : paintEntireComponent(g,false)` | `:1653-1661` | VERIFIED |
| `paintEntireComponent` never consults the component's OWN cachedImage | `:1742-1775` | VERIFIED |
| `CachedComponentImage` contract: "returns false if this object handles all repaint work internally" | `juce_CachedComponentImage.h:63-73` | VERIFIED |
| `juce::CoreGraphicsContext` is a public header (`juce_graphics.h:155`), ctor sets font smoothing + AA itself | `juce_CoreGraphicsContext_mac.mm:223-248` | VERIFIED |
| `NSViewComponent` (juce_gui_extra, linked: `CMakeLists.txt:575`): `NSViewAttachment` adds the NSView as a subview of the peer view, frames it to `peer->getAreaCoveredBy(owner)`, hides it when `!owner.isShowing()` | `juce_NSViewComponent_mac.mm:38-119` | VERIFIED |
| `ComponentListener` has `componentChildrenChanged` / `componentParentHierarchyChanged` / `componentBeingDeleted`; `addChildComponent` fires the first | `juce_ComponentListener.h:89,100,121`; `juce_Component.cpp:1336-1350` | VERIFIED |
| TooltipWindow with a parent is a CHILD of it (`addChildComponent`), shown/hidden with `setVisible` | `juce_TooltipWindow.cpp:47,95,183` | VERIFIED |
| a parented PopupMenu window is a visible child of the parent (`setVisible(true)`, `toFront(false)`) | `juce_PopupMenu.cpp:2161-2165` | VERIFIED |
| `startDragging(desc, comp)` defaults `allowDraggingToOtherJuceWindows = false` -> the drag image is a CHILD of the container (MainComponent, `MainComponent.h:68`) | `juce_DragAndDropContainer.h:104-109`, `.cpp:486-504`; `ClipCell.cpp:243` (the other five sites pass `true`) | VERIFIED |
| WaveformDisplay: `startTimerHz(30)` `:9`; `repaint()` `:58` (skipped when `rawCount_ < 2`, i.e. test mode); paint = rounded-rect bg + border + paths, NO text, corners unpainted (not opaque) | `WaveformDisplay.cpp:9,12-59,63-70` | VERIFIED |
| SignalBar: `startTimerHz(30)` `:34`; `timerCallback` updates every strip then `repaint()` of the whole bar `:111-123`; paint fills the whole bounds `:125-136`; not `setOpaque`; children = SignalStrips (text + meters), 3 buttons; the `+` menu has no parent component (a desktop window) | `SignalBar.cpp:34,111-136,199-213` | VERIFIED |
| LayerStrip: `setOpaque(true)` `:331`, `startTimerHz(30)` `:332`; timer repaints `transportBounds_` + `clipNameBounds_` every tick `:733-739`, then `syncFromModel()` `:741`, band hairline only while a routine plays `:743-746`; the transport rect paints in/out region + playhead from `clip->isPlayable/inPoint/outPoint/playheadPosition` `:531-560`; the clip-name box paints only `clipName_` (set by `updateClipName`, which repaints `:1013`) `:574-584` | `LayerStrip.cpp` | VERIFIED |
| LayerStrips are children of `gridContent_` inside `gridViewport_` | `DeckView.cpp:10-12,148,177,372-373` | VERIFIED |
| TopBar: `startTimerHz(15)` `:305`; `repaint(beatWheelBounds_.getUnion(barPhraseBounds_).expanded(2))` `:325-326` (26 + 44 px wide, adjacent: `:569-573`) | `TopBar.cpp` | VERIFIED |
| ClipInspector::refresh `:958-980` ends in an unconditional `repaint()` `:979`; called at ~10 Hz from `MainComponent::timerCallback` `:3759-3761` via `InspectorPanel::refresh` (active tab only, `InspectorPanel.cpp:180-188`) | | VERIFIED |
| MainComponent: `waveformDisplay_` value member `MainComponent.h:294`, added `:222`; `signalBar_` `:396`, created `:638-639`; tooltip `:512` (recreated `:2236`); overlays `:1821-1826` (`addChildComponent`, full-window when active `:2646-2659`); `paint` `:2348`; ctor ends `setSize(1280,800)` `:2224`; ApiServer wiring `:1877-1890`; `DragAndDropContainer` `.h:68` | | VERIFIED |
| ApiServer TEST-ONLY block: `ApiServer.h:209-211`, `ApiServer.cpp:283-288` (routes), `:1619-1639` (handler shape: parse, `callAsync`, answer at once) | | VERIFIED |
| `/api/state` reads counters on the HTTP thread (no message-thread hop) | `ApiServer.cpp:1297-1345` | VERIFIED (the shown part; the rest is the same shape) |
| `MessageHeartbeat.h` (mediaopen, uncommitted in `.claude/worktrees/rta0928b-mediaopen/src/api/`): one ping at a time, `takePeakMs()` resets; `POST /api/debug/heartbeat {on, period_ms}`; `/api/state.peak_message_stall_ms` | plan-mediaopen.md 4.8; the header itself | VERIFIED (content) / ASSUMED (merged before this lane) |
| `MainWindow` = `DocumentWindow`, native title bar, `setResizable(true, true)` (a `ResizableCornerComponent` child) | `Main.cpp:44-60` | VERIFIED |
| main build has `AUDIODNA_BUILD_TEST_SERVER=ON`, Release | `build/CMakeCache.txt:31,255` | VERIFIED |
| `.venv` has pyobjc (Quartz), PIL, numpy | `.venv/lib/python3*/site-packages` | VERIFIED |

---------------------------------------------------------------------------------------------------------------------
## 1. TRADEOFFS CONSIDERED

- **F1(a) own layer-backed views for the animating widgets -- ACCEPTED for SignalBar + WaveformDisplay.** The only option
  that removes the mechanism (AppKit unions the rects of ONE view; separate views are dirty-tracked separately) instead of
  paying it 30x a second. Cost lands where it belongs: the widget's own recording (SignalStrip 29.9 + WaveformDisplay 4.2
  ms/s in `compclass-base.txt`) plus a small per-commit overhead. Pixels identical by construction (same paint code, same
  CoreGraphicsContext, same async layer).
- **F1(a) for LayerStrip -- REJECTED**: the strips scroll inside `gridViewport_` (`DeckView.cpp:10-12`); a native subview
  is clipped by the peer view only, so a scrolled strip would draw over the deck tabs; and 4-8 layers x 30 Hz of extra
  drawRects erode the budget. F4 gives the same idle result (an image clip's strip is silent) at ~15 lines.
- **F1(b) GL quads in the compositor -- REJECTED**: puts UI drawing on the GL thread (rules 2/4), needs FeatureBus plumbing
  for meter values, and has the same Z-order problem (the GL view is a native subview too).
- **F2 `JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS` -- REJECTED**: MEASURED RED alone (11.7 ms window max, 262 ms/s;
  `arms-table.md` metal), moves ALL rasterization onto the main thread (WaveformDisplay 4.2 -> 70.6 ms/s), makes a quiet
  UI worse (floor 4.7 vs 1.6 ms), changes the rendering path app-wide (JUCE: "may slow rendering down").
- **F3 `setBufferedToImage` on the static panels -- REJECTED as the fix**: the pass count stays 30/s with a whole-window
  union, so the CA after-paint work (71 ms/s, `dispphase-base.txt`) and a ~4 ms pass stay; ~27 MB of caches at 2x; any
  child repaint (a hover) re-renders a whole panel image. Kept as a FUTURE option for interactive full passes.
- **F5 fold the wheel into the 30 Hz tick / make it native -- NOT NEEDED**: with (A)-(C) the wheel has no union partner;
  its passes are 0.15-0.7 ms. Re-open only if g3 shows unions with it.
- **F7 FilesBrowser paint culling -- DEFERRED**: after the fix the browser is not painted at idle; its 33-144 ms/s was
  union-induced. Interactive full passes (resize, deck switch) are another lane.
- **Patch JUCE -- REJECTED, nothing to patch**: the union is AppKit's (one drawRect per layer-backed view); JUCE's own
  answer is F2. Everything here uses public JUCE API (`CachedComponentImage`, `NSViewComponent`, `CoreGraphicsContext`).
- **Simplest alternative, F3 + F4 + F6 only** (the strongest counterargument: no ObjC++, no Z-order logic): predicted
  busy = noanim floor 67 - 33 (F6) + 30 passes x (CA post 1.9 + cached blits ~0.5 + widget recording ~1.2) ~ 140-150 ms/s
  and FULL passes of 5-8 ms remain (the unexplained 2.5x factor) -> at or over both gate thresholds, with the interactive
  regressions of F3. It loses on the gate, not on taste.
- **Strongest counterargument to (E)**: the fallback machinery is the complexity hot-spot (~150 lines) and the transition
  frame is INFERRED. Why it still wins: without it a popup or tooltip over the signal bar is INVISIBLE (a native view is
  above JUCE content -- `NSViewComponent.h:47-48` says so), which violates "behaves identically". The rule set is small
  (5.1 test), fails SAFE (an unrecognised overlay type only costs today's rendering while it is up), and the identity
  gate v2 proves the switch end to end.

---------------------------------------------------------------------------------------------------------------------
## 2. DECISION / SPEC

### 2.1 `src/ui/UiPaintCounters.h` (NEW, header-only, always compiled; read by a TEST-ONLY endpoint)
```cpp
#pragma once
#include <atomic>
#include <cstdint>
// s-rta-0928b idlepaint (Pitfall 55): witnesses of WHO repaints at idle. Relaxed atomics, bumped on the message thread,
// read by GET /api/debug/ui_paint (TEST_SERVER builds). No behaviour depends on them.
namespace uipaint {
enum Layer : int { Waveform = 0, SignalBar = 1, LayerCount = 2 };
struct Counters {
    std::atomic<uint64_t> layerDraws[LayerCount] {};          // NativeLayerHost::drawLayer calls
    std::atomic<int>      layerMode[LayerCount] { -1, -1 };    // -1 no host, 0 native, 1 fallback pending, 2 fallback
    std::atomic<uint64_t> layerFallbacks { 0 };                // native -> fallback switches
    std::atomic<uint64_t> layerStripTransportRepaints { 0 };   // LayerStrip transport-rect repaint() calls
    std::atomic<uint64_t> mainComponentPaints { 0 };           // MainComponent::paint calls ~= peer passes
    std::atomic<uint64_t> topBarPaints { 0 };                  // TopBar::paint calls
    std::atomic<uint64_t> clipInspectorRepaints { 0 };         // ClipInspector::refresh repaint() calls
};
inline Counters& counters() { static Counters c; return c; }
}
```
Increment sites (one line each): `MainComponent::paint` (`MainComponent.cpp:2348`, first statement), `TopBar::paint`
(`TopBar.cpp:442`), `LayerStrip::timerCallback` (2.5), `ClipInspector::refresh` (2.6), `NativeLayerHost::drawLayer` (2.4).

### 2.2 `src/ui/NativeLayerCache.h` (NEW, pure JUCE C++, header-only, ctest'd)
```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
// The juce::CachedComponentImage that hands a widget's repaints to its own CALayer (Pitfall 55). Native: every
// invalidate() marks the layer and returns false (juce_Component.cpp:1611-1614: the peer is never dirtied) and the
// parent's paint of the widget draws nothing (the layer covers it). Fallback: transparent -- invalidate() returns true,
// paint() paints the widget. Native -> Fallback is two-step so no stale pixel is ever shown: the widget is repainted
// in-peer first (FallbackPending), and the layer hides one message-loop turn AFTER that paint landed.
class NativeLayerCache final : public juce::CachedComponentImage, private juce::AsyncUpdater
{
public:
    struct Sink { virtual ~Sink() = default;
                  virtual void layerNeedsDisplay(juce::Rectangle<int> areaInComponent) = 0;
                  virtual void layerHidden(bool hidden) = 0; };
    enum class Mode : int { Native = 0, FallbackPending = 1, Fallback = 2 };
    NativeLayerCache(juce::Component& c, Sink& s) : component_(c), sink_(s) {}
    ~NativeLayerCache() override { cancelPendingUpdate(); }

    void paint(juce::Graphics& g) override                       // called by the PARENT's paint (paintWithinParentContext)
    {
        if (mode_ == Mode::Native) return;                       // the layer shows this rect
        component_.paintEntireComponent(g, false);               // exactly what JUCE does without a cache (:1660)
        if (mode_ == Mode::FallbackPending && g.getClipBounds().contains(component_.getLocalBounds()))
            triggerAsyncUpdate();                                // the whole widget is fresh beneath the layer: hide it next turn
    }
    bool invalidateAll() override { return invalidate(component_.getLocalBounds()); }
    bool invalidate(const juce::Rectangle<int>& area) override
    {
        if (mode_ != Mode::Native) return true;                  // the peer paints it
        const auto a = area.getIntersection(component_.getLocalBounds());
        if (!a.isEmpty()) sink_.layerNeedsDisplay(a);
        return false;                                            // the peer is never dirtied
    }
    void releaseResources() override {}

    void setFallback(bool wantFallback)                          // OverlayWatch -> NativeLayerHost -> here
    {
        if (wantFallback) { if (mode_ == Mode::Native) { mode_ = Mode::FallbackPending; component_.repaint(); } }
        else if (mode_ != Mode::Native) { cancelPendingUpdate(); mode_ = Mode::Native;
                                          sink_.layerHidden(false); sink_.layerNeedsDisplay(component_.getLocalBounds()); }
    }
    Mode mode() const noexcept { return mode_; }
    void flushForTest() { handleUpdateNowIfNeeded(); }
private:
    void handleAsyncUpdate() override { if (mode_ == Mode::FallbackPending) { mode_ = Mode::Fallback; sink_.layerHidden(true); } }
    juce::Component& component_; Sink& sink_; Mode mode_ = Mode::Native;
};
```
Invariants: (i) `paint()` in Native draws nothing -- correct only because the layer covers the widget's rect; for a
non-opaque widget (WaveformDisplay's rounded corners) the parent's background is painted by the parent itself
(`MainComponent::paint` `fillAll`, `:2350`; JUCE excludes only OPAQUE children, `juce_Component.cpp:1719-1727`), so the
composite is layer-over-parent-background = today's stacking. (ii) `component_.repaint()` in `setFallback(true)` goes
through `invalidateAll()` -> mode is FallbackPending -> returns true -> the peer repaints the WHOLE widget at the next
vblank -> `paint()` sees a full clip -> `triggerAsyncUpdate()` -> the layer hides after that display pass. (iii) Fallback
-> Native shows the layer and marks it all dirty in the same turn: AppKit displays it in the same transaction it appears
(INFERRED: standard CA commit order; if a stale frame were ever visible the v2 row would show it as a diff).

### 2.3 `src/ui/OverlayWatch.h/.cpp` (NEW, pure JUCE C++, ctest'd)
Purpose: the list of screen rects of every JUCE-drawn thing that can sit ABOVE a native widget inside the main window.
```cpp
class OverlayWatch final : private juce::ComponentListener
{
public:
    struct Client { virtual ~Client() = default; virtual void overlaysChanged(const std::vector<juce::Rectangle<int>>& screenRects) = 0; };
    explicit OverlayWatch(juce::Component& root);   // MainComponent; takes the BASELINE of root's children now (end of its ctor)
    ~OverlayWatch() override;
    void addOverlay(juce::Component* c);           // explicit: BindingOverlay, MidiLearnOverlay (full-window when active)
    void addClient(Client*); void removeClient(Client*);
    std::vector<juce::Rectangle<int>> visibleOverlayScreenRects() const;   // isShowing() ones only
    // pure rules (tested):
    static bool isRootOverlay(const juce::Component& child, const std::vector<juce::Component::SafePointer<juce::Component>>& baseline);
        // a juce::TooltipWindow (any time) -> true; a child NOT in the baseline (drag image, melatonin overlay, a recreated tooltip) -> true
    static bool isWindowOverlay(const juce::Component& child, const juce::Component& content);
        // a child of the top-level window that is not the content and not a juce::ResizableCornerComponent / ResizableBorderComponent
    static bool intersectsAny(juce::Rectangle<int> widgetScreen, const std::vector<juce::Rectangle<int>>& overlayScreens);
private:
    // listens to: root (componentChildrenChanged -> rescan; componentParentHierarchyChanged -> (re)attach to root.getTopLevelComponent()),
    // the top-level (componentChildrenChanged -> rescan), every tracked overlay (visibility / movedOrResized / beingDeleted -> recompute + notify).
};
```
Tracked set = explicit overlays + root children passing `isRootOverlay` + top-level children passing `isWindowOverlay`.
Recompute on any event; notify clients only when the rect list changed. Baseline pointers are `SafePointer`s (a deleted
child never aliases a new one). Cost: zero at idle (event-driven); a menu/tooltip show = one recompute of a 0-2 element list.
Enumerated in-peer overlay sources at main HEAD (VERIFIED by grep): 7 `withParentComponent` menu sites (top-level children
-> `isWindowOverlay`); `tooltipWindow_` (`isRootOverlay` by type); ClipCell drag image (post-baseline root child); binding /
MIDI-learn overlays (explicit); melatonin inspector overlay (debug builds; post-baseline root child). ComboBox popups, the
SignalBar `+` menu, AlertWindow/DialogWindow, FileChooser, the other five drag sites are DESKTOP windows (above every
native view) and need nothing. No `Slider::setPopupDisplayEnabled`, no `getParentComponentForMenuOptions` override.

### 2.4 `src/ui/NativeLayerHost.h` + `src/ui/NativeLayerHost.mm` (NEW; the .mm is macOS only)
```cpp
// NativeLayerHost.h
class NativeLayerHost final : public NativeLayerCache::Sink, public OverlayWatch::Client, private juce::ComponentListener
{
public:
    // nullptr when not macOS, or (TEST_SERVER builds) when ADNA_UI_NATIVE_LAYERS=0 -- the widget then paints as today.
    static std::unique_ptr<NativeLayerHost> attach(juce::Component& target, OverlayWatch& watch, uipaint::Layer id);
    ~NativeLayerHost() override;
    void overlaysChanged(const std::vector<juce::Rectangle<int>>& screenRects) override;  // fallback = intersects target.getScreenBounds()
    void layerNeedsDisplay(juce::Rectangle<int>) override; void layerHidden(bool) override;
    void drawLayer(void* cgContext, float w, float h);      // the NSView's drawRect: (below)
    bool targetIsOpaque() const { return target_.isOpaque(); }
private: ...   // target_, hostComp_ (juce::NSViewComponent), cache_ (raw; owned by the target), lastRects_, id_, teeth_
};
```
`attach` (the .mm):
1. `view = [[ADNANativeLayerView alloc] initWithFrame:]`; `view.owner = this`; `[view setWantsLayer: YES]`;
   `[view setLayerContentsRedrawPolicy: NSViewLayerContentsRedrawDuringViewResize]`; `[view layer].drawsAsynchronously = YES`
   -- the peer's exact setup (`juce_NSViewComponentPeer_mac.mm:218-220`).
2. `hostComp_ = std::make_unique<juce::NSViewComponent>(); hostComp_->setInterceptsMouseClicks(false, false);
   hostComp_->setAccessible(false); hostComp_->setView(view); [view release]; target.addAndMakeVisible(*hostComp_);
   hostComp_->setBounds(target.getLocalBounds());` -- a CHILD of the widget (SignalBar::resized / rebuildStrips touch only
   their own members, `SignalBar.cpp:37-59,139-171`; WaveformDisplay has no children), so the widget's code is untouched
   and `isShowing()` hides the NSView with the widget (`NSViewAttachment::componentVisibilityChanged`).
3. `target.addComponentListener(this)` -> `componentMovedOrResized`: `hostComp_->setBounds(target.getLocalBounds())` +
   re-evaluate fallback; `componentBeingDeleted`: detach.
4. `cache_ = new NativeLayerCache(target, *this); target.setCachedComponentImage(cache_);` (JUCE owns it,
   `juce_Component.cpp:552-558`); `uipaint::counters().layerMode[id] = 0`; `watch.addClient(this)`.
`~NativeLayerHost`: `watch.removeClient(this); target.removeComponentListener(this); target.setCachedComponentImage(nullptr)`
(JUCE repaints); `view.owner = nullptr; hostComp_->setView(nullptr); target.removeChildComponent(hostComp_.get())`.
```objc++
@interface ADNANativeLayerView : NSView  @property (nonatomic, assign) NativeLayerHost* owner; @end
@implementation ADNANativeLayerView
- (BOOL) isFlipped { return YES; }                                  // the peer's convention (:2628) -> JUCE coordinates
- (BOOL) isOpaque  { return self.owner != nullptr && self.owner->targetIsOpaque(); }   // (:2131-2135)
- (NSView*) hitTest: (NSPoint) p { (void) p; return nil; }        // every mouse / drag event reaches the JUCE peer view
- (BOOL) acceptsFirstResponder { return NO; }                       // never key
- (BOOL) isAccessibilityElement { return NO; }
- (void) drawRect: (NSRect) r
{ if (self.owner) self.owner->drawLayer((void*) [[NSGraphicsContext currentContext] CGContext], r.size.width, r.size.height); }
@end

void NativeLayerHost::drawLayer(void* cgv, float w, float h)
{
    auto* cg = (CGContextRef) cgv;
    if (w < 1.0f || h < 1.0f) return;                                                 // peer :964
    if (!target_.isOpaque()) CGContextClearRect(cg, CGContextGetClipBoundingBox(cg));  // peer :973-974
    const auto height = target_.getHeight();
    CGContextConcatCTM(cg, CGAffineTransformMake(1, 0, 0, -1, 0, height));           // peer :1033
    juce::CoreGraphicsContext context(cg, (float) height);                            // peer :1034 (public: juce_graphics.h:155)
    juce::Graphics g(context);
   #if AUDIODNA_TEST_SERVER
    if (teeth_) g.setOrigin(1, 0);                                                    // ADNA_UI_NATIVE_LAYERS_TEETH=1: v0's teeth
   #endif
    target_.paintEntireComponent(g, false);                                           // = paintWithinParentContext's call (:1660)
    uipaint::counters().layerDraws[id_].fetch_add(1, std::memory_order_relaxed);
}
void NativeLayerHost::layerNeedsDisplay(juce::Rectangle<int> a)
{ if (auto* v = (NSView*) hostComp_->getView()) [v setNeedsDisplayInRect: NSMakeRect(a.getX(), a.getY(), a.getWidth(), a.getHeight())]; }
void NativeLayerHost::layerHidden(bool hidden)
{ hostComp_->setVisible(!hidden);   /* through JUCE, so NSViewAttachment's own setHidden logic stays consistent */
  uipaint::counters().layerMode[id_] = (int) cache_->mode(); if (hidden) ++uipaint::counters().layerFallbacks; }
void NativeLayerHost::overlaysChanged(const std::vector<juce::Rectangle<int>>& r)
{ lastRects_ = r; cache_->setFallback(OverlayWatch::intersectsAny(target_.getScreenBounds(), r));
  uipaint::counters().layerMode[id_] = (int) cache_->mode(); }
```
Non-Apple: `NativeLayerHost.h` declares `attach` returning `nullptr` (`#if !JUCE_MAC` inline); nothing else changes.
`attach` also returns `nullptr` when `AUDIODNA_TEST_SERVER` and `getenv("ADNA_UI_NATIVE_LAYERS")` is `"0"` (read once).

### 2.5 `LayerStrip` (F4) -- `src/ui/LayerStrip.h` + `.cpp`
`LayerStrip.h` after `syncFromModel()` (`:51`):
```cpp
    // s-rta-0928b idlepaint (Pitfall 55): what the transport rect paints (paint() :531-560). The strip's 30 Hz timer
    // repaints the rect only when this changes: an image clip's strip is silent at idle, a playing sequence's repaints as
    // its playhead crosses a pixel. Pure; public for tests/test_layer_strip_transport_view.cpp.
    struct TransportView { bool showsClip = false; float inPoint = 0.0f, outPoint = 0.0f; int playheadX = 0;
                           bool operator==(const TransportView&) const = default; };
    static TransportView transportViewOf(const Layer* layer, juce::Rectangle<int> transportBounds);
```
private members after `transportBounds_` (`:147`): `TransportView lastTransportView_; bool transportViewValid_ = false;`
`LayerStrip.cpp`:
```cpp
LayerStrip::TransportView LayerStrip::transportViewOf(const Layer* layer, juce::Rectangle<int> transportBounds)
{
    TransportView v;
    const Clip* clip = layer != nullptr ? layer->getActiveClip() : nullptr;
    if (clip == nullptr || !clip->isPlayable()) return v;          // paint() draws only the fill + border then
    const auto tb = transportBounds.toFloat();
    v.showsClip = true; v.inPoint = clip->inPoint; v.outPoint = clip->outPoint;
    v.playheadX = static_cast<int>(tb.getX() + static_cast<float>(clip->playheadPosition) * tb.getWidth());   // = :553-554
    return v;
}
```
`timerCallback()` `:735-739` becomes:
```cpp
    // s-rta-0928b idlepaint (Pitfall 55): the transport rect repaints only when its pixels change; the clip-name box paints
    // nothing time-varying (updateClipName() repaints it on change, :1013) -- its per-tick repaint is gone.
    if (!transportBounds_.isEmpty())
    {
        const auto tv = transportViewOf(layer_, transportBounds_);
        if (!transportViewValid_ || tv != lastTransportView_)
        {
            lastTransportView_ = tv; transportViewValid_ = true;
            repaint(transportBounds_);
            uipaint::counters().layerStripTransportRepaints.fetch_add(1, std::memory_order_relaxed);
        }
    }
```
`transportViewValid_ = false;` added in `refresh()` (`:724`), `setLayer()` (`:691`) and `resized()` (`:608`, after
`transportBounds_` is set): a whole-strip or size repaint already drew the current model; the flag only forces one
compare-and-repaint on the next tick. `inPoint`/`outPoint` compare as floats exactly (any change moves an AA edge);
`playheadX` compares the int `drawVerticalLine` receives. `clip->playheadPosition` is `mutable double` written by the
render thread (`Clip.h:229`) -- the same benign read paint() does today; a torn read costs one extra or one late repaint.
Nothing else in the strip repaints periodically (`syncFromModel` repaints a fader only on change, bands only while
Playing) -- VERIFIED `:749-782, :743-746`.

### 2.6 `ClipInspector::refresh()` (F6) -- `src/ui/ClipInspector.h` + `.cpp`
`ClipInspector.h` (public, after `refresh()` `:58`):
```cpp
    // s-rta-0928b idlepaint (Pitfall 55): EVERY input paint() / paintTimeline() / paintSectionHeader() read (:440-577,
    // :1037-1135). refresh() (10 Hz) repaints only when it changes; the child widgets repaint themselves. A superset is
    // fine; a missing field is a stale inspector -- add, never remove. Public for tests/test_clip_inspector_paint_key.cpp.
    struct PaintKey { const Clip* clip = nullptr; bool fxDrop = false; juce::String name; int mediaType = 0, transportMode = 0;
                      bool playable = false; double playhead = 0.0; float inPoint = 0.0f, outPoint = 0.0f, beatDivision = 0.0f;
                      int sourceParamControls = 0, width = 0, height = 0;
                      bool operator==(const PaintKey&) const = default; };
    PaintKey paintKeyNow() const;
```
private: `PaintKey lastPaintKey_; bool paintKeyValid_ = false;`. `paintKeyNow()` fills from `clip_` (nullptr -> defaults +
`fxDrop`, width, height), `fxDropHighlight_` (`:184`), `sourceParamControls_.size()` (`:128`), `getWidth()/getHeight()`.
`refresh()` `:979` `repaint();` becomes:
```cpp
    const auto key = paintKeyNow();
    if (!paintKeyValid_ || key != lastPaintKey_) { lastPaintKey_ = key; paintKeyValid_ = true; repaint();
                                                   uipaint::counters().clipInspectorRepaints.fetch_add(1, std::memory_order_relaxed); }
```
`setClip()` (`:787-789`) sets `paintKeyValid_ = false` right after `clip_ = clip;`. Every other `repaint()` in the file stays.
LayerInspector / CompositionInspector / SignalInspector refresh() repaints are OUT of scope (the active tab at idle in both
fixtures is Clip; note them in the report as follow-ups).

### 2.7 MainComponent wiring
`MainComponent.h`: `#include "ui/NativeLayerHost.h"` `#include "ui/OverlayWatch.h"`; members declared AFTER
`midiLearnOverlay_` (`:407`) so they are destroyed BEFORE `signalBar_` (`:396`), `waveformDisplay_` (`:294`) and the two
overlays: `std::unique_ptr<OverlayWatch> overlayWatch_; std::unique_ptr<NativeLayerHost> waveformLayer_, signalBarLayer_;`.
`MainComponent.cpp` ctor, immediately before `setSize(1280, 800)` (`:2224`) -- after every `addAndMakeVisible` (the baseline):
```cpp
    // s-rta-0928b idlepaint (Pitfall 55): the two always-animating panels draw in their own CoreGraphics layers; an
    // in-peer overlay that crosses one (a parented PopupMenu, the tooltip, a ClipCell drag image, the binding overlays)
    // hands it back to JUCE painting while it is up.
    overlayWatch_ = std::make_unique<OverlayWatch>(*this);
    overlayWatch_->addOverlay(bindingOverlay_.get());
    overlayWatch_->addOverlay(midiLearnOverlay_.get());
    signalBar_->setOpaque(true);   // its paint() fills every pixel (SignalBar.cpp:125-136): pixel-identical, and an opaque layer
    waveformLayer_  = NativeLayerHost::attach(waveformDisplay_, *overlayWatch_, uipaint::Waveform);
    signalBarLayer_ = NativeLayerHost::attach(*signalBar_,      *overlayWatch_, uipaint::SignalBar);
```
`MainComponent::paint` (`:2348`): first statement `uipaint::counters().mainComponentPaints.fetch_add(1, std::memory_order_relaxed);`.
`TopBar::paint` (`TopBar.cpp:442`): same for `topBarPaints`.
TEST-ONLY menu callback, inside the existing `#if AUDIODNA_TEST_SERVER` region near `:1888-1890`:
```cpp
    apiServer_->onDebugUiTestMenu = [this](bool on, int x, int y) {
        if (!on) { juce::PopupMenu::dismissAllActiveMenus(); return; }
        juce::PopupMenu m; m.addItem(1, "Test item one"); m.addItem(2, "Test item two"); m.addItem(3, "Test item three");
        m.showMenuAsync(juce::PopupMenu::Options().withParentComponent(getTopLevelComponent())
                            .withTargetScreenArea(juce::Rectangle<int>(1, 1).withPosition(localPointToGlobal(juce::Point<int>(x, y)))),
                        [](int) {});
    };
```
`setTooltipsEnabled` (`:2229-2242`) needs NO change: a recreated TooltipWindow is a post-baseline root child AND a
TooltipWindow by type -> `isRootOverlay` true.

### 2.8 TEST-ONLY REST (ApiServer, `#if AUDIODNA_TEST_SERVER`; absent from a build without the flag)
`ApiServer.h:209-211` block gains `void handleDebugUiPaint(...)`, `void handleDebugUiTestMenu(...)` and a public
`std::function<void(bool on, int x, int y)> onDebugUiTestMenu;`. `ApiServer.cpp:283-288` gains
`server_.Get("/api/debug/ui_paint", ...)` and `server_.Post("/api/debug/ui_test_menu", ...)`; handlers next to `:1623`:
- `GET /api/debug/ui_paint` -> `{ok, waveform_layer_draws, signalbar_layer_draws, waveform_mode, signalbar_mode,
  layer_fallbacks, layer_strip_transport_repaints, main_component_paints, top_bar_paints, clip_inspector_repaints}`
  (cumulative int64 / int; from `uipaint::counters()` on the HTTP thread; no message-thread hop).
- `POST /api/debug/ui_test_menu {"on": bool, "x": int, "y": int}` (x, y in MainComponent coordinates) -> `callAsync` ->
  `onDebugUiTestMenu`; answers `{ok:true}` at once (the `stall_message_thread` shape, `:1623-1638`).
Heartbeat: REUSE mediaopen's `POST /api/debug/heartbeat {on, period_ms}` + `/api/state.peak_message_stall_ms`
(`src/api/MessageHeartbeat.h`). Reconciliation, named: if lane/mediaopen has NOT merged when this lane starts, this lane
does not start (section 6) -- it must edit ApiServer/TestServer anyway. Should Harmony still order an early start, the
builder copies `MessageHeartbeat.h` + the `/api/debug/heartbeat` route + the two `/api/state` fields VERBATIM from
plan-mediaopen.md 4.8 (same file name, same JSON keys), and the later merge keeps mediaopen's copy (identical or
"theirs"); the probe reads only those keys.

### 2.9 CMake
`CMakeLists.txt:408-412` (the `if(APPLE)` block) adds `src/ui/NativeLayerHost.mm`; `src/ui/OverlayWatch.cpp` joins the
main source list next to the other `src/ui/*.cpp`. No new dependency, no JUCE flag, no definition.

### 2.10 MUST-NOT-CHANGE (the identity contract; the reviewer checks each)
Pixels of every widget at every state (v0-v3); timer rates: WaveformDisplay 30, SignalBar 30, LayerStrip 30, TopBar 15 Hz
(g1/g2); `LayerStrip::syncFromModel` every tick (Pitfall 41) and the band hairline repaint `:743-746`; the routine cue
colours; `SignalBar.cpp` / `WaveformDisplay.cpp` / `TopBar.cpp` paint code (TopBar gains one counter line only); popup
parenting (`withParentComponent(getTopLevelComponent())`); `ResettableSlider`; every `DragAndDropTarget`; keyboard focus
(the NSView is never first responder); mouse / drag hit-testing (hitTest nil); `OutputWindow` (untouched); Renderer / GL;
no new mutex (everything on the message thread; counters are relaxed atomics); JUCE pinned, unpatched; non-Apple builds
behave as today (`attach` -> nullptr); production builds carry no env switches (both are `AUDIODNA_TEST_SERVER`-only).

---------------------------------------------------------------------------------------------------------------------
## 3. GATES (RED first: write each, watch it fail against the pre-change tree, then implement)

### 3.1 ctests (Catch2; registered at the EOF of `tests/CMakeLists.txt` in the shapes cited)
1. `tests/test_native_layer_cache.cpp` [idlepaint][cache] (headless JUCE widget under `ScopedJuceInitialiser_GUI`; a
   `juce::Component` subclass counting `paint()` calls; a fake `Sink` recording calls; painted into a `juce::Image` like
   `test_layer_strip_follows_model.cpp:83-94`; links `juce_gui_basics` only, the `test_png_fast` block shape):
   (a) Native: `invalidate(r)` returns false and the sink saw exactly r (clipped to local bounds); `invalidateAll()` -> the
   whole rect; `paint(g)` paints nothing (widget paint count 0). (b) `setFallback(true)`: mode FallbackPending; the next
   `paint(g)` with a full clip paints the widget once and, after `flushForTest()`, mode Fallback and `layerHidden(true)`
   seen once; a partial-clip paint does NOT hide. (c) In Fallback `invalidate` returns true and the sink sees nothing.
   (d) `setFallback(false)`: mode Native, `layerHidden(false)` then `layerNeedsDisplay(all)`. (e) a child's `repaint()`
   (a child component added to the widget) is swallowed in Native. RED on main: the header does not exist.
2. `tests/test_overlay_watch.cpp` [idlepaint][overlay]: a root with 3 baseline children and one `juce::TooltipWindow(&root)`;
   `OverlayWatch w(root)`: (a) rects empty; tooltip `setVisible(true)` + `setBounds` -> one rect = its bounds; hide -> empty.
   (b) a child added after the baseline, visible -> counted; a baseline child toggling visibility -> not counted.
   (c) `addOverlay(x)` counted only while visible. (d) `isWindowOverlay`: content false, a `juce::ResizableCornerComponent`
   false, another child true. (e) `intersectsAny` edge cases (touching edges = no intersection, JUCE semantics). (f) a fake
   Client sees `overlaysChanged` once per change, not per event. RED on main: absent.
3. `tests/test_layer_strip_transport_view.cpp` [idlepaint][strip] (links like `test_layer_strip_follows_model`):
   a `Layer` with an ImageSequence clip: `playheadPosition` 0.0 -> 0.005 on a 78-px rect: equal views (same int px);
   -> 0.02: differs (`playheadX` 1); `inPoint` 0.0 -> 0.01: differs; an Image clip: `showsClip` false whatever the
   playhead; null layer: default. And on a real `LayerStrip` (headless, `setLayer`, `resized` via `setBounds`): calling
   `syncFromModel()` still moves the V fader (Pitfall 41 unchanged, the existing test keeps passing). RED on main: absent.
4. `tests/test_clip_inspector_paint_key.cpp` [idlepaint][inspector] (links like the inspector tests already in
   `tests/CMakeLists.txt`, e.g. `test_layer_inspector_persistent_toggle`): the key changes when `playheadPosition`, `inPoint`,
   `outPoint`, `transportMode`, `name`, `mediaType`, `beatDivision`, `fxDropHighlight_` (via `itemDragEnter`) or the size
   changes, and is equal across two `refresh()` calls with nothing changed; `setClip(nullptr)` then a null-clip key.
   RED on main: absent.
Existing tests must stay green: `ctest` full run (main today 869/869 per the work log).

### 3.2 Live probe `.harmony/probe-idle-paint.sh` / `.py` / `.json` (rig: lock gate, `open -g`, production mode
unless a row says test mode, one app at a time, no Output window ever, no synthetic input, >= 5 launches per arm)
`.sh` = the probe-seq-vram.sh header verbatim (live-lock gate, refuse-if-running, port check, venv discovery, fresh
`mktemp -d`, final osascript quit + `adna_pids` wait, exit code) -- but the `.py` OWNS launching (one launch per arm
iteration: `open -g --stdout --stderr [--env ...] APP [--args --test-mode]`, health wait, quit, wait for exit), because
every gate row is a 5-launch arm and the identity rows alternate two bundles. Env: `IDLEPAINT_APP` (the app under test;
default `<root>/build/AudioDNA_artefacts/Release/Audio-DNA.app`), `IDLEPAINT_APP_BEFORE` (identity rows; default the MAIN
checkout's build), `IDLEPAINT_PY`, rows via `$2`. Fixtures: card = `.harmony/probe-routines.json` with `@ROOT@` -> root,
`load_composition` + `trigger_clip {layer, column 0}` per layer (the diag's `idle.py`); many16 = the card file's deck with
4 layers x 4 Image clips over 16 x 3840x2160 JPEGs written by PIL into `<out>/media` before the launch (gradient + index
text; deleted after); seq24 = one layer with a 24-frame 320x180 PNG sequence clip (`{"mediaType": 5, "sequenceFiles": [...],
"sequenceFps": 30.0, "speed": 1.0, "transportMode": 0, "loopMode": 0, "reverse": false}`, probe-seq-vram.py:143-146's
shape -- `speed` MUST be present, s-rta-0928b work log 18:15). Idle window: 6 s settle after the last trigger, then
`POST /api/debug/heartbeat {on:true, period_ms:4}`, `ps -M -p <pid>` sample, 30 s during which the ONLY traffic is one
`GET /api/state` + one `GET /api/debug/ui_paint` every 500 ms (60 windows; both handlers read atomics on the HTTP thread --
a deviation from the diag's request-free window, harmless to the main thread by construction, stated in the report),
`ps -M` sample. Per launch: `win_max_med` = median of the 60 `peak_message_stall_ms` values; `cpu_main` = (main-thread
utime+stime delta) / 30 s from `ps -M` (first thread row = the main thread; `diag-idle-tools/psdiff.py`, proven against
the instrumented busy time: stock 332 vs 344/349). Per arm: medians over launches. Compilers (`pgrep -x clang`) seen
during a window taint the launch -> re-run (run_arm.sh's rule). Every row prints the load average.

| row | arm | PASS | RED on current main (predicted) | GREEN after (predicted, INFERRED) |
|---|---|---|---|---|
| c0_preflight | 1 launch | health; `POST /api/debug/heartbeat` answers ok (else FAIL "app predates mediaopen's heartbeat"); Screen Recording preflight for v-rows (one `screencapture -x -o -l <wid>`: the PNG exists and is not a single colour; else the v-rows are SKIPPED, printed in words: grant Screen Recording to the host process) | passes on main once mediaopen merged | passes |
| **i1_idle_card** (IDLE-HB-card, THE gate) | 5 launches, production, card | median `win_max_med` <= 8 ms AND median `cpu_main` <= 150 ms/s | **17.9** ms [batches 17.9 / 21.4 / 22.6] / **321-345** ms/s (ps stock 332 [308-352]) -> FAIL both | 2-5 ms / 75-115 ms/s |
| **i2_idle_many16** (IDLE-HB-many16) | 5 launches, production, many16 | same thresholds | **17.8** / **531** -> FAIL both | 2-5 / 75-115 (the 16 cells no longer repaint at idle) |
| g1_anim_rates (guard, rides on i1's launches) | AFTER build | per launch, over the 30 s: `waveform_layer_draws`/s in [24, 36], `signalbar_layer_draws`/s in [24, 36], `top_bar_paints`/s in [12, 26] (15 Hz wheel + ~4 Hz fps/DSP label passes), `waveform_mode == 0 && signalbar_mode == 0` at the end, `layer_fallbacks == 0` | fields absent -> FAIL (RED by absence) | PASS |
| g2_strip_playhead (guard) | 1 launch, production, seq24 then card | seq24 triggered, 5 s: delta `layer_strip_transport_repaints` >= 40 (78-px bar, 0.8 s loop ~ 97 px/s, capped at 30/s -> ~140 expected); then load card + trigger, 6 s settle, 5 s: delta <= 2 (the flag's one compare-and-repaint per strip after a refresh) | absent -> FAIL | PASS |
| g3_peer_quiet (INFO) | rides on i1 | prints `main_component_paints`/s and `clip_inspector_repaints`/s | absent (main ~41 passes/s per lagclass-base) | ~15-22 /s and ~0 |
| x1_within_build_off (INFO, optional) | 3 launches, AFTER with `ADNA_UI_NATIVE_LAYERS=0`, card | prints the i1 metrics: the same binary without the layers must read like main (attribution within one build) | n/a | ~14-22 ms / ~300 (F4+F6 alone remove the strips' 118 repaint()/s and the inspector's, but the union of bar + waveform stays) |
| v0_capture_teeth | AFTER with `ADNA_UI_NATIVE_LAYERS_TEETH=1` vs AFTER, test mode, card | the comparator (v1's) REPORTS a difference whose bounding boxes lie inside the SignalBar and WaveformDisplay rects -> the row PASSES when the comparator catches the 1-px shift | n/a | PASS |
| **v1_identity_test_mode** | BEFORE vs AFTER, `--args --test-mode` (analysis off: meters and waveform static, bpm 0), states S1 default composition, S2 card, S3 many16; 5 s settle (8 s for S3, thumbnails) | per state: `np.array_equal` outside the allowed set; allowed = (a) the TopBar row's right 200 pt (fps label), (b) <= 4 diff clusters each <= 12x12 px within the bottom-left quadrant (the waveform's four rounded corners: layer-edge premultiplied-alpha rounding, <= 2/255) -- every cluster's bbox and max delta printed, a diff PNG saved | n/a (needs both builds) -- its teeth is v0 | PASS |
| v2_identity_fallback | AFTER only, test mode, card | capture A (native); `ui_test_menu {on:true, x:600, y:60}` (a point inside the SignalBar); poll `signalbar_mode == 2` within 1 s (waveform stays 0); capture B; `{on:false}`; poll `signalbar_mode == 0`; capture C. PASS: every A/B differing pixel lies inside R = (x-10..x+300, y-10..y+140) in MainComponent coordinates (the menu) AND some differ inside the SignalBar rect (the menu IS visible over the bar); `array_equal(A, C)` exactly | endpoint absent | PASS |
| v3_identity_production_masked (SHOULD) | BEFORE vs AFTER, production, card | masks: SignalBar rect, WaveformDisplay rect, TopBar row (audio-driven content); the rest `array_equal` | n/a | PASS |
Window capture: window id from `Quartz.CGWindowListCopyWindowInfo` (owner "Audio-DNA", layer 0, largest; the
probe-deck-tabs.sh:76-96 snippet), capture `screencapture -x -o -l <wid> <png>` (window-only, never full screen), decoded
with PIL+numpy. Both builds capture the same window geometry (`Main.cpp:66-71` sizes it to the display's userArea).
MainComponent -> capture coordinates: content origin = (0, title bar); the probe derives the title-bar height from the
CGWindowBounds height minus the content height it reads from `/api/...` -- NOT available; use 28 pt (the native title bar,
ASSUMED) and verify once by locating the TopBar's bottom border row (kPanelBorder colour) in the capture; scale = capture
px / window pt. Aggregate JSON + a one-line PASS/FAIL per row; exit 0 iff no FAIL (SKIP is not PASS).

### 3.3 Predicted-after derivation (INFERRED from the diagnosis's own numbers)
CPU/busy = noanim floor 62-67 (both fixtures) - ClipInspector 30-33 (F6) + native layer recording 37 (SignalStrip 29.9 +
SignalBar ~3 + WaveformDisplay 4.2, `compclass-base.txt` body) + per-commit overhead 30/s x (pre 0.15 + post scaled to
~4.4 MB of layer backing instead of 26.9 MB: ~0.1-0.3) ~ 8-14 + drawRect glue ~3 = **75-115 ms/s**. Evidence the CA
"post" cost scales with the dirty area, not the window: TOP BAR-only passes total 0.73 ms vs 1.89 ms post alone for a
BODY pass. Window max median: the worst vblank per 500 ms = SignalBar recording ~1.0 + waveform ~0.15 + CA ~0.3 +
a coinciding TopBar pass ~0.7 + timers ~0.3 = **2.5-5 ms**. If i1 reads GREEN on window max but RED on CPU, the lever (in
order): SignalBar repaints only the strips whose `SignalStrip::updateValue` changed a displayed value (the bar's
`repaint()` `:122` -> per-strip repaints); then F5 (the wheel native). Do NOT reach for F2.

### 3.4 Boris LOOK checkpoint (Harmony runs the critic panel later; this is the human list)
Open the Outputs menu (TopBar) -- it drops over the signal bar and must be fully visible; hover a TopBar button until its
tooltip shows over the bar; drag a clip cell across the waveform (the drag image must stay visible); drop a file from
Finder onto the deck with the drag path crossing the signal bar; the meters, waveform, beat wheel and a playing
sequence's strip playhead all still move; resize the window; Shift+Cmd+K binding overlay covers everything incl. the bar.

---------------------------------------------------------------------------------------------------------------------
## 4. SEQUENCING, FENCES, COMMITS

Fences (file:line, lanes in flight at 328301d): lane/mediaopen edits MainComponent open/drop paths, UndoService,
ImageSequence, `ClipCell.cpp:206` (the stat), CompositorEngine stats, `ApiServer.h/.cpp` + `TestServer.h/.cpp` state fields,
adds `src/api/MessageHeartbeat.h`; lane/video edits `VideoPlayer.*`, `Renderer.*`, `CompositorEngine.*` video paths,
`ApiServer.cpp` (+22) and `TestServer.cpp` (+22) (`git diff --stat main...lane/video`). THIS lane touches
`MainComponent.h/.cpp` (ctor tail `:2224`, `paint` `:2348`, ApiServer wiring `:1888`, header members), `ApiServer.h:209-211`,
`ApiServer.cpp:283-288 + ~1640`, `LayerStrip.h/.cpp`, `ClipInspector.h/.cpp`, `TopBar.cpp:442` (one line), `CMakeLists.txt:408-412`,
`tests/CMakeLists.txt` EOF, docs. => **Branch from main only AFTER both lanes merge** (ApiServer is shared with both; the
heartbeat comes from mediaopen). It never touches `ClipCell.cpp`, `SignalBar.cpp`, `WaveformDisplay.cpp`, Renderer, GL.

Commits on `lane/idlepaint` (worktree `.claude/worktrees/rta0928b-idlepaint`, build dir `build-lane` with
`-DAUDIODNA_BUILD_TEST_SERVER=ON`, JUCE from `build/_deps/juce-src` via `FETCHCONTENT_SOURCE_DIR_JUCE` like
`diag-idle-tools/build.sh`), each a `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>` trailer:
0. `test(s-rta-0928b idlepaint): probe-idle-paint (RED harness) + UiPaintCounters + TEST-ONLY /api/debug/ui_paint, ui_test_menu`
   -- counters at MainComponent::paint / TopBar::paint only; no behaviour change. THEN run `probe-idle-paint.sh <out> i1,i2`
   with `IDLEPAINT_APP` = the pre-change app (the main checkout's build after the merges) and record the RED numbers.
1. `perf(s-rta-0928b idlepaint): LayerStrip -- the transport rect repaints only when its pixels change; no per-tick clip-name repaint (F4)` + test 3.
2. `feat(s-rta-0928b idlepaint): NativeLayerCache + OverlayWatch (pure) + ctests` (tests 1-2; nothing attached yet).
3. `perf(s-rta-0928b idlepaint): SignalBar and WaveformDisplay draw in their own CoreGraphics layers (NativeLayerHost); in-peer overlays fall back` + MainComponent wiring + CMake.
4. `perf(s-rta-0928b idlepaint): ClipInspector::refresh repaints only when a painted input changed (F6)` + test 4.
5. `docs(s-rta-0928b idlepaint): Pitfall NN, CLAUDE.md index + UI Patterns (paid), architecture.md UI painting, APP-INVENTORY TEST-ONLY paragraph, work log`.
Then the GREEN run: `probe-idle-paint.sh <out>` (all rows; `IDLEPAINT_APP` = build-lane, `IDLEPAINT_APP_BEFORE` = the main
build), `ctest` full, and the lane report `.harmony/.reports/s-rta-0928b/idlepaint.md` (RED numbers, GREEN numbers, the
v1/v2 bbox listings, deviations).

---------------------------------------------------------------------------------------------------------------------
## 5. DOCS TEXT

**`docs/claude/pitfalls.md`** (append after 54; the number is "NN" until Harmony assigns it, expected 55):
NN. **JUCE 8's macOS peer repaints the UNION of every dirty rect in one `drawRect` -- a timer-driven `repaint()`
anywhere is paid by the whole window**: `NSViewComponentPeer::repaint` only collects rects
(`juce_NSViewComponentPeer_mac.mm:1074`) and hands them to AppKit at the next vblank (`:1089-1116`); a layer-backed NSView
gets ONE `drawRect` with their bounding box (`getRectsBeingDrawn` returned 1 rect in 1674/1674 passes; JUCE's own note
`BREAKING_CHANGES.md:1826-1834`) and JUCE paints every component inside it. Four always-animating widgets at opposite
window edges (4 LayerStrip playheads, SignalBar, WaveformDisplay at 30 Hz, the TopBar beat wheel at 15 Hz) made every
idle pass the whole window body (26/s, 7.9 ms) or the whole window (3.4/s, 18.7 ms): 334 ms/s of main-thread busy with
nothing happening, 537 on a 16-image deck, an 18 ms stall about three times a second (s-rta-0928b diag-idle; an
app-level scope never sees it -- the time is inside AppKit's display cycle). The fix is not to shrink each rect but to
keep periodic repaints OUT of the peer's union: (1) an always-animating panel draws in its own layer-backed NSView
(`NativeLayerHost`: a `CachedComponentImage` swallows its `repaint()`s and marks the layer, whose `drawRect` runs the same
`paintEntireComponent` through the same `CoreGraphicsContext` -- SignalBar, WaveformDisplay); (2) a widget whose timer
repaints a rect repaints it only when the pixels would change (`LayerStrip::transportViewOf`, `ClipInspector::paintKeyNow`);
(3) the TopBar's 15 Hz wheel stays in-peer because nothing unions with it any more. A native layer sits ABOVE all JUCE
content, so an in-peer overlay that intersects the widget -- a PopupMenu with `withParentComponent`, the TooltipWindow,
a ClipCell drag image (`startDragging`'s default keeps it in the container), the binding overlays -- switches the widget
back to JUCE painting for the overlay's lifetime (`OverlayWatch`; the layer hides only after the widget was repainted
beneath it). Never add a 15-30 Hz `repaint()` without one of the two mechanisms; never put a native-layer widget inside
a `juce::Viewport` (a native view is clipped by the peer only). `JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS`
(the Metal renderer) is a measured half-fix (11.7 ms window max; raster moves onto the main thread) and
`setBufferedToImage` on the static panels still pays the pass. Guards: `tests/test_native_layer_cache.cpp`,
`tests/test_overlay_watch.cpp`, `tests/test_layer_strip_transport_view.cpp`, `tests/test_clip_inspector_paint_key.cpp`;
live: `.harmony/probe-idle-paint.sh` (i1 / i2 the gate, g1 / g2 the animation guard, v0-v3 pixel identity).

**`CLAUDE.md`** (24,665 B today, cap 25,000: the two additions ~440 B are PAID by the two compressions below; the builder
runs `wc -c CLAUDE.md` after the edit and must read <= 25,000 -- if not, shorten line 143's parenthetical too):
- index line after `:231` ("54. ..."): `NN. The mac peer repaints the UNION of every dirty rect (a 30 Hz repaint anywhere = the whole window) -- before adding any timer-driven repaint().`
- UI Patterns, a new paragraph after "**Preview/Output panel never reshapes the picture**" (`:139`): `**Periodic repaints**: a timer's `repaint()` costs the whole window (Pitfall NN) -- an always-animating widget draws in its own layer (`NativeLayerHost`: SignalBar, WaveformDisplay) or repaints only on change (LayerStrip transport, `ClipInspector::refresh`); no new timed repaint without one of the two.`
- PAY 1, line `:97` (485 B -> ~350 B), replace the NOTE sentence with: `**NOTE**: the 135 shipped effects are inline strings in `src/render/EmbeddedShaders.h`, compiled at startup (the legacy `shaders/` dir was removed Wave 0); ShaderManager hot-reload is inert for them.`
- PAY 2, line `:167`: `- `TASKPLAN_V2.md` (all phases P1-P25 complete) is archived at `docs/archive/TASKPLAN_V2.md`.`

**`docs/claude/architecture.md`**: Source Tree `ui/` block (`:256-266`) gains
`│       ├── NativeLayerHost (.h/.mm), NativeLayerCache, OverlayWatch, UiPaintCounters  # Always-animating panels in their own CoreGraphics layers; in-peer overlays fall back (Pitfall NN)`
and a new subsection after "### UI Text Rules" (`:311-314`):
`### UI Painting (macOS)` -- "The message thread paints through ONE CoreGraphics-backed NSView per window; AppKit hands JUCE the
UNION of every rect repainted since the last vblank and JUCE paints every component inside it (Pitfall NN). Rules: a widget
that repaints on a timer either owns a layer (`NativeLayerHost::attach` in MainComponent's ctor -- SignalBar, WaveformDisplay;
never inside a `juce::Viewport`) or repaints a rect only when its pixels change (`LayerStrip::transportViewOf`,
`ClipInspector::paintKeyNow`). Native layers are above JUCE content: every in-peer overlay must be one `OverlayWatch` sees
(a child of the top-level window, a `TooltipWindow`, a child of MainComponent added after startup, or registered with
`addOverlay`). Witnesses: `GET /api/debug/ui_paint` (TEST_SERVER builds), `.harmony/probe-idle-paint.sh`."

**`.harmony/APP-INVENTORY.md`** (`:204-208`, the TEST-ONLY paragraph) gains: "`GET /api/debug/ui_paint` (s-rta-0928b
idlepaint) returns the UI paint counters (`src/ui/UiPaintCounters.h`: native-layer draws and modes, LayerStrip transport
repaints, MainComponent / TopBar paints, ClipInspector repaints) from atomics -- no message-thread hop; `POST
/api/debug/ui_test_menu {"on":bool,"x","y"}` shows / dismisses a 3-item PopupMenu parented to the top-level window at
MainComponent point (x, y) -- probe-idle-paint v2's overlay-fallback witness. TEST_SERVER-build env, read once at start:
`ADNA_UI_NATIVE_LAYERS=0` (no native layers: the within-build counterfactual x1), `ADNA_UI_NATIVE_LAYERS_TEETH=1` (the
layers paint 1 px right: the identity comparator's teeth v0)."

**Work log** `.harmony/s-rta-0928b-work.md`: one row per commit + the RED/GREEN numbers (Harmony's format).

---------------------------------------------------------------------------------------------------------------------
## 6. RISKS (what could go wrong; what to verify while building)

R1 (INFERRED, medium) **CA/AppKit behaviours assumed, not measured**: the un-fallback frame (layer shown + marked dirty in
one turn = drawn before it is presented), the hide-after-paint ordering (AsyncUpdater runs after the display pass), and
that `screencapture -l` composites native sublayers. v2 catches any stale/late frame that persists; a ONE-frame glitch at
a menu open cannot be captured -> the LOOK checkpoint (3.4). Fallback if a glitch is seen: hide the view synchronously
inside `paint()` (AppKit permits it) -- one line.
R2 (INFERRED, low-medium) **an overlay type the watch does not see** -> that overlay is hidden where it crosses the bar or
the waveform. Enumerated in 2.3; the failure is local and today's Z-order for the GL preview already has this property
(a native GL view above JUCE content; `Renderer.cpp:53`). If Boris finds one, `addOverlay` it.
R3 (INFERRED, medium) **CPU margin**: 75-115 predicted vs 150. Levers in 3.3; also the text-AA risk below could force an
opaque SignalBar (already chosen) -- nothing else.
R4 (INFERRED, low) **text anti-aliasing on a layer**: `CoreGraphicsContext` sets smoothing/AA itself (`:239-243`) and the
SignalBar layer is opaque (2.7), so SignalStrip text should match to the bit; the waveform has no text. v1 prints every
diff cluster; a bar-wide text diff = investigate `CGContextSetShouldSmoothFonts` parity before shipping, never mask it.
R5 (VERIFIED) **premultiplied-alpha rounding at the waveform's rounded corners** (a non-opaque layer composited by the
window server vs in-context blending): +/-1-2 LSB on <= 4 clusters of a few px -- allowed by v1 with a stated reason.
R6 (INFERRED, low) **F6 misses a paint input** -> a stale inspector text. Superset key + review; commit 4 is independently
revertible and NOT needed for the gate (only for margin).
R7 (ASSUMED) **Screen Recording (TCC) for `screencapture -l`** from the probe's host process; probe-deck-tabs.sh already used
it. c0 detects a blank capture and SKIPs (never PASSes) the v-rows.
R8 (VERIFIED) **gate metric drift**: the gate's CPU is `ps -M` main-thread CPU, 4 % under the diag's observer busy (321 vs
334) -- same threshold, stated. The heartbeat period is mediaopen's 4 ms (the diag used 2 ms): a max-lag metric for
stalls >= 4 ms is period-independent; the 8 ms threshold stands.
R9 (INFERRED, low) `NSViewComponent` adds an accessibility group per host; `setAccessible(false)` + `isAccessibilityElement
NO` keep VoiceOver silent. `hitTest: nil` also keeps NSDraggingDestination lookup on the peer view (drops still land).
R10 (VERIFIED) window resize / signal-bar Expanded mode: the layer redraws with the widget as today's peer did; Expanded
makes the bar layer the whole body (the user's choice, not idle).
R11 (INFERRED) the probe polls `/api/state` every 500 ms during the idle window (the peak resets on read): HTTP-thread work
only; if Harmony wants the diag's silent window, read the peak once at the end (loses the per-window median) -- a
`.json` switch `pollMs` (500 default; 0 = read once).

---------------------------------------------------------------------------------------------------------------------
## 7. OPEN QUESTIONS FOR HARMONY
Q1 Sequencing: confirm this lane starts only after lane/mediaopen AND lane/video merge (both edit ApiServer/TestServer;
   the heartbeat is mediaopen's). If an earlier start is wanted, rule the 2.8 reconciliation.
Q2 Is Screen Recording granted to the process that will run the probe (Terminal / the Claude host)? If not, v0-v3 SKIP
   and the identity proof falls to the critic panel + LOOK.
Q3 F6 (commit 4): include (margin, my recommendation) or defer (smallest change)?
Q4 Accept the stated exception: while an in-peer overlay crosses the bar or the waveform, the app paints exactly as today
   (the union cost returns for that overlay's lifetime). Behaviour identical; performance identical to today only then.
Q5 Assign NN (55 expected) and the CLAUDE.md pay-for edits (5) -- or name other lines to compress.

---------------------------------------------------------------------------------------------------------------------
## COMPACT
plan-idlepaint: SignalBar + WaveformDisplay draw in their OWN layer-backed NSViews (NativeLayerHost = NSViewComponent +
a CachedComponentImage that swallows repaints and marks the layer; drawRect mirrors the peer: flip CTM + CoreGraphicsContext
+ paintEntireComponent; hitTest nil; opaque bar / non-opaque waveform) so nothing unions with anything; LayerStrip (inside
a Viewport -> not native) repaints its transport rect only when its pixels change and drops the per-tick clip-name repaint
(F4); ClipInspector::refresh repaints only when a painted input changed (F6); TopBar wheel stays in-peer (nothing left to
union with). In-peer overlays (parented PopupMenus, TooltipWindow, ClipCell drag image, binding overlays) that intersect
a native widget hand it back to JUCE painting via OverlayWatch (repaint first, hide the layer one turn later). Rejected:
F2 (measured RED, app-wide raster on main), F3 (pays the union 30x/s), F1(b) GL, JUCE patch (nothing to patch). Gates:
4 ctests (cache modes, overlay rules, transport view, paint key); .harmony/probe-idle-paint.{sh,py,json}: i1_idle_card
(<= 8 ms window max median, <= 150 ms/s main CPU by ps -M; main 17.9 / 321-345 RED), i2_idle_many16 (17.8 / 531 RED),
g1 animation rates 30/30/15 via counters, g2 sequence playhead repaints >= 40 per 5 s and <= 2 at idle, v0 comparator
teeth (1-px env shift), v1 test-mode window captures BEFORE vs AFTER pixel-equal (fps label + <= 4 waveform-corner
clusters allowed), v2 menu-over-bar fallback and return pixel-equal, v3 production masked. Heartbeat = mediaopen's
MessageHeartbeat (/api/debug/heartbeat + /api/state.peak_message_stall_ms); counters via TEST-ONLY GET /api/debug/ui_paint;
POST /api/debug/ui_test_menu opens a parented test menu. Branch after mediaopen + video merge. Predicted after: 2-5 ms /
75-115 ms/s (INFERRED). Docs: Pitfall NN (55), CLAUDE.md index + UI Patterns paid by compressing rule 5's NOTE and line 167,
architecture.md "UI Painting (macOS)" + tree, APP-INVENTORY TEST-ONLY paragraph.

REPORT_FILE: .harmony/.reports/s-rta-0928b/plan-idlepaint.md
STATUS: DONE

## HARMONY ADOPTION (s-rta-0928b, 00:07) — OVERRIDES THE BODY WHERE THEY DIFFER
Plan authored by Fable (wf_f2b088f2-5f0). Attacked by attack-idlepaint-juce.md and attack-idlepaint-vj.md. Adopted: the
approach (A)-(E), F4, F6, the rejected list. Rulings:
- I1 (juce MUST 1, ADOPT) Native -> fallback hides the native layer SYNCHRONOUSLY, inside `setFallback(true)` (before the next
  vblank), and the widget repaints in-peer in the same batch — never one AsyncUpdater turn later: an overlay is never covered
  by a stale native layer. Fallback -> native: the layer is un-hidden only after it has drawn its first frame. Gate v2b: a
  TEST-visible counter of frames in which an in-peer overlay intersecting a native widget was visible while that widget's
  layer was not hidden (`ui_overlay_covered_frames`, counted where the peer's vblank batch is observable — or, if no such hook
  exists without patching JUCE, a burst of >= 20 window captures over the first 300 ms after the overlay appears, each
  asserting the popup's known pixels): must read 0. Teeth: the async variant (the body's design) must read > 0 or fail a
  capture — record it.
- I2 (juce MUST 2, ADOPT) LayerStrip reads `clip->playheadPosition` ONCE per tick into a member; the same value drives the
  compare AND the paint (paint never re-reads the model for the transport). Name which s166 read strategy this is
  (ConnectionEngine.h:12-13) in a code comment. g2 gains a per-tick check: during steady playback the painted playhead
  value advances on >= 95 % of ticks (a TEST-ONLY counter of repaints vs ticks-with-changed-value).
- I3 (vj MUST 2, ADOPT) The routine band hairline (LayerStrip.cpp:743-746) follows the same repaint-on-change rule (repaint
  only when its painted value changes). New arm g4: a routine spanning >= 3 layers playing; measure like i1 (5 launches).
  PASS thresholds = i1's (window max median <= 8 ms, main CPU <= 150 ms/s). If g4 fails on the fixed build, STOP and report
  the numbers (a design finding, never a re-threshold). RED arm = main.
- I4 (vj MUST 1, ADOPT) v2b (I1) also runs under load: the same routine as g4 playing, >= 5 runs, fallback latency measured
  in FRAMES (not a 1 s poll): the native layer is hidden within the overlay's first presented frame (I1) and restored <= 2
  frames after the overlay closes.
- I5 (vj SHOULD 3, ADOPT) v1b: drive both panels with a deterministic TEST-ONLY feature state (freeze the meter/waveform feed at
  a non-trivial driven value — reuse an Eyes/test hook if one exists, else a TEMPORARY hook), then capture the widget with
  its native layer and again forced to in-peer painting (TEST-ONLY fallback toggle) at the same frozen state: pixel-equal.
- I6 (vj SHOULD 4, REPORT) g5 report-only: main-thread CPU with a driven signal moving both meters and the waveform (the real
  show case), main vs fixed, 5 runs.
- I7 (juce SHOULD 1, ADOPT) State the real reason the waveform's unpainted corners stay correct (MainComponent is not repainted
  at idle), and prove the non-idle case: v1 includes a capture after a forced full-window repaint (a window resize via REST
  or a TEST-ONLY repaint call), before vs after pixel-equal.
- I8 (juce NIT + vj NIT, ADOPT) The probe asserts the precondition (peer view layer-backed; a TEST-ONLY state field) and
  measures the title-bar height per run from the window vs content bounds (no hard-coded 28 pt).
- I9 (Q1, RULED: start NOW from main 328301d) Do not wait for the video / mediaopen merges. Heartbeat: take mediaopen's
  `src/api/MessageHeartbeat.h` and its `POST /api/debug/heartbeat` + `peak_message_stall_ms` VERBATIM (read it from
  `git show lane/mediaopen:src/api/MessageHeartbeat.h` once committed, else from the mediaopen worktree's file) so the two
  lanes add byte-identical code; at merge Harmony keeps one. Rebase onto main when either lane merges, before your final
  gates; keep every other lane's ApiServer / TestServer / CMake / docs blocks.
- I10 (Q2) Step 0: take one window-only capture of the app (Quartz window id). If Screen Recording is denied, STOP and report
  (never change system permissions).
- I11 (Q3) F6 included. (Q4) The overlay exception is accepted. (Q5) Pitfall text "NN"; Harmony assigns at merge; pay for
  every CLAUDE.md byte (seqvram and video each added a line).
- I12 A critic panel seat (visual + UX + logic) reviews the before/after captures before Harmony's gate; Boris's LOOK list
  (3.4) goes on his page.
