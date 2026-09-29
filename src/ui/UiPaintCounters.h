#pragma once
#include <atomic>
#include <cstdint>

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
};
inline Counters& counters() { static Counters c; return c; }
}
