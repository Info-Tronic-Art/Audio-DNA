#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>

// s-rta-0928b idlepaint (Pitfall NN): witnesses of WHO repaints at idle. Relaxed atomics, bumped on the message thread,
// read by GET /api/debug/ui_paint (TEST_SERVER builds). No behaviour depends on them.
namespace uipaint {
enum Layer : int { Waveform = 0, SignalBar = 1, LayerCount = 2 };
struct Counters {
    std::atomic<uint64_t> layerDraws[LayerCount] {};          // NativeLayerHost::drawLayer calls
    std::atomic<int>      layerMode[LayerCount] { -1, -1 };    // -1 no host, 0 native, 1 restore pending, 2 fallback
    std::atomic<uint64_t> layerFallbacks { 0 };                // native -> fallback switches
    std::atomic<uint64_t> layerStripTransportRepaints { 0 };   // LayerStrip transport-rect repaint() calls
    std::atomic<uint64_t> layerStripPlayheadTicks { 0 };       // strip ticks whose playhead pixel moved (adoption I2)
    std::atomic<uint64_t> layerStripPlayheadPaints { 0 };      // strip paints whose painted playhead pixel moved (I2)
    std::atomic<uint64_t> layerStripBandRepaints { 0 };        // routine band hairline repaint() calls (adoption I3)
    std::atomic<uint64_t> mainComponentPaints { 0 };           // MainComponent::paint calls ~= peer passes
    std::atomic<uint64_t> topBarPaints { 0 };                  // TopBar::paint calls
    std::atomic<uint64_t> clipInspectorRepaints { 0 };         // ClipInspector::refresh repaint() calls
    // TEST_SERVER builds (adoption I1 / I4): per vblank, a native layer was still showing while an in-peer overlay
    // crossed its widget (must stay 0); vblanks from an overlay's close to its layer showing again (last / max).
    std::atomic<uint64_t> overlayCoveredFrames { 0 };
    std::atomic<int>      restoreFramesLast { -1 };
    std::atomic<int>      restoreFramesMax { -1 };
    std::atomic<int>      peerLayerBacked { -1 };              // -1 unknown, 0 / 1: the peer NSView is layer-backed (I8)
    // TEST_SERVER builds: MainComponent's size and the two panels' / the TopBar's bounds in MainComponent coordinates,
    // written by MainComponent::resized() -- the probe maps window captures with them (adoption I8).
    std::atomic<int>      mainW { 0 }, mainH { 0 };
    std::atomic<int>      signalBarRect[4] {};
    std::atomic<int>      waveformRect[4] {};
    std::atomic<int>      topBarRect[4] {};

    // s-rta-0929 g4cpu (plan-g4cpu 2.2, adoptions G3 / G7): who repaints -- and what actually paints -- while a routine
    // plays. Relaxed atomics, message thread; GET /api/debug/ui_paint. No behaviour depends on them.
    std::atomic<uint64_t> routinePadRepaints { 0 };            // RoutinePad::setSpec -> repaint()
    std::atomic<uint64_t> routinePadSweepTicks { 0 };          // setSpec calls whose painted sweep width changed
    std::atomic<uint64_t> routinePadSweepPaints { 0 };         // pad paints whose painted sweep width moved
    std::atomic<uint64_t> routinePadPaints { 0 };              // RoutinePad::paint executions (G7)
    std::atomic<uint64_t> layerStripFaderRepaints { 0 };       // V / S fader setValue calls that moved the snapped value
    std::atomic<uint64_t> layerStripFaderPaints { 0 };         // V fader paint executions (G7)
    std::atomic<uint64_t> layerStripBandPaints { 0 };          // strip paints that drew a playing band's rect (G7)
    std::atomic<uint64_t> topBarWheelRepaints { 0 };           // TopBar::timerCallback repaint() calls
    std::atomic<uint64_t> deckCornerRepaints { 0 };            // DeckView ROUTINES corner-note repaints
    std::atomic<uint64_t> layerInspectorRepaints { 0 };        // LayerInspector::refresh repaint() calls
    std::atomic<uint64_t> paramControlRepaints { 0 };          // UniversalParamControl::updateValueDisplay repaint() calls
    std::atomic<uint64_t> signalStripChanges { 0 };            // SignalStrip::updateValue calls whose painted key changed
    std::atomic<uint64_t> signalBarTicks { 0 };                // SignalBar::timerCallback calls
    std::atomic<uint64_t> layerDrawUs[LayerCount] {};          // NativeLayerHost::drawLayer main-thread CPU time, us
    std::atomic<uint64_t> layerDrawWallUs[LayerCount] {};      // ... and its wall time, us
#if AUDIODNA_TEST_SERVER
    // The display passes' log: the pass's clip rect in MainComponent coordinates, its JUCE paint time (MainComponent::
    // paint -> paintOverChildren: the main thread's CPU time `us` and the wall time `wus`; -1 when paint() was skipped
    // because opaque children covered the clip)
    // and the repaint sources that fired since the previous pass (G3: Src bits). Plus the SignalStrip change mask of the
    // SignalBar's ticks (bit i = strip i changed). Ring buffers, written on the message thread, read by GET
    // /api/debug/ui_passes (torn reads are tolerable: INFO only). Fixed size, no allocation.
    static constexpr int kRing = 512;
    struct Pass { std::atomic<int64_t> tUs { 0 }; std::atomic<int32_t> x { 0 }, y { 0 }, w { 0 }, h { 0 }, us { 0 }, wus { 0 };
                  std::atomic<uint32_t> src { 0 }; };
    Pass passes[kRing];
    std::atomic<uint32_t> passSeq { 0 };
    std::atomic<uint64_t> stripMasks[kRing] {};
    std::atomic<uint32_t> stripMaskSeq { 0 };
    std::atomic<int> signalBarStrips { 0 };                    // the SignalBar's strip count (the mask's width)
    std::atomic<uint32_t> pendingSources { 0 };
    std::atomic<int> deckRect[4] {}, padRowRect[4] {}, stripColRect[4] {}, wheelRect[4] {}, inspectorRect[4] {};
    std::atomic<int> previewRect[4] {};                        // g4cpu-fix: the PreviewPanel (v5 masks its opacity render)
    int64_t passStartUs = -1, passStartCpuUs = -1;             // message thread only
#endif
};
inline Counters& counters() { static Counters c; return c; }

// s-rta-0929 g4cpu (G3): the repaint sources, stamped into the next display pass's record.
enum Src : uint32_t { SrcPad = 1u << 0, SrcFader = 1u << 1, SrcBand = 1u << 2, SrcWheel = 1u << 3, SrcCorner = 1u << 4,
                      SrcLayerInspector = 1u << 5, SrcParam = 1u << 6, SrcTransport = 1u << 7, SrcClipInspector = 1u << 8 };
inline void bump(std::atomic<uint64_t>& counter, uint32_t src)
{
    counter.fetch_add(1, std::memory_order_relaxed);
#if AUDIODNA_TEST_SERVER
    counters().pendingSources.fetch_or(src, std::memory_order_relaxed);
#else
    (void) src;
#endif
}

// The calling thread's CPU time (the pass / layer timings are CPU time: wall time also counts preemption on a loaded
// machine).
inline int64_t threadCpuUs()
{
#if defined(__APPLE__)
    return static_cast<int64_t>(clock_gettime_nsec_np(CLOCK_THREAD_CPUTIME_ID) / 1000);
#else
    return 0;
#endif
}

#if AUDIODNA_TEST_SERVER
inline int64_t nowUs()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
// MainComponent::paint (when JUCE runs it) -> passBegin; MainComponent::paintOverChildren (always) -> passEnd.
inline void passBegin() { counters().passStartUs = nowUs(); counters().passStartCpuUs = threadCpuUs(); }
inline void passEnd(int x, int y, int w, int h)
{
    auto& c = counters();
    const auto rl = std::memory_order_relaxed;
    const int64_t t = nowUs(), cpu = threadCpuUs();
    auto& p = c.passes[c.passSeq.load(rl) % Counters::kRing];
    p.tUs.store(t, rl);
    p.x.store(x, rl); p.y.store(y, rl); p.w.store(w, rl); p.h.store(h, rl);
    p.us.store(c.passStartCpuUs >= 0 ? static_cast<int32_t>(cpu - c.passStartCpuUs) : -1, rl);
    p.wus.store(c.passStartUs >= 0 ? static_cast<int32_t>(t - c.passStartUs) : -1, rl);
    p.src.store(c.pendingSources.exchange(0, rl), rl);
    c.passStartUs = c.passStartCpuUs = -1;
    c.passSeq.fetch_add(1, std::memory_order_release);
}
#endif
}
