#pragma once

// RenderGeometry: pure integer geometry for the composition canvas (s-rta-0926b plan4 item 1).
// No GL, no JUCE -- unit-tested headless by tests/test_render_geometry.cpp.
//
// The canvas IS the composition's picture: Renderer renders every frame once, offscreen, at the
// canvas size; the preview panel only presents it (fitCanvas = letter/pillar-boxed, never
// stretched). Every consumer -- recorder, Syphon, render_frame, snapshots -- reads the canvas.
namespace RenderGeometry
{
struct Size { int w = 0, h = 0; };
struct Rect { int x = 0, y = 0, w = 0, h = 0; };

inline constexpr Size kDefaultCanvas{ 1920, 1080 };

// The canvas size for this frame. lockedW/H = the TEST-ONLY override (Renderer::setLockedResolution,
// used by TestServer render_frame width/height); compW/H = Composition::outputWidth/outputHeight.
// A pair with either side <= 0 is ignored; the last resort is 1920x1080 (a composition JSON without
// the keys used to load 0x0).
constexpr Size resolveCanvas(int lockedW, int lockedH, int compW, int compH) noexcept
{
    if (lockedW > 0 && lockedH > 0)
        return { lockedW, lockedH };
    if (compW > 0 && compH > 0)
        return { compW, compH };
    return kDefaultCanvas;
}

// The largest rect with the canvas's aspect that fits a view of viewW x viewH pixels, centred.
// Empty (all 0) when any input <= 0.
constexpr Rect fitCanvas(int canvasW, int canvasH, int viewW, int viewH) noexcept
{
    if (canvasW <= 0 || canvasH <= 0 || viewW <= 0 || viewH <= 0)
        return {};
    int w = 0, h = 0;
    if (static_cast<long long>(viewW) * canvasH <= static_cast<long long>(viewH) * canvasW)
    {
        w = viewW;
        h = static_cast<int>(static_cast<long long>(viewW) * canvasH / canvasW);
    }
    else
    {
        h = viewH;
        w = static_cast<int>(static_cast<long long>(viewH) * canvasW / canvasH);
    }
    return { (viewW - w) / 2, (viewH - h) / 2, w, h };
}

// Frame-ring downscale for a canvas width: never store a ring cell wider than 480 px, and never
// less than the historical 1/4 (CompositorEngine::kRingDownscale). 1080p -> 4, 1440p -> 6, 4K -> 8.
constexpr int ringDownscale(int canvasW) noexcept
{
    const int ds = (canvasW + 479) / 480;
    return ds < 4 ? 4 : ds;
}

// Box-filter taps per axis for presenting a canvas of canvasW pixels into presentW pixels:
// ceil(canvasW / presentW) clamped to 1..8. 1 = plain bilinear (also for an upscale).
constexpr int presentTaps(int canvasW, int presentW) noexcept
{
    if (canvasW <= 0 || presentW <= 0)
        return 1;
    const int n = (canvasW + presentW - 1) / presentW;
    return n < 1 ? 1 : (n > 8 ? 8 : n);
}
} // namespace RenderGeometry
