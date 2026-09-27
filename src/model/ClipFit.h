#pragma once
#include <cstdint>

// ClipFit: how a clip's picture meets the composition canvas (s-rta-0926b plan-fitmode).
// Pure integer/float math -- no GL, no JUCE -- unit-tested headless by tests/test_clip_fit.cpp.
//
//   Stretch (default) -- the picture fills the canvas, its shape follows the canvas (today's output).
//   Bars              -- the picture keeps its own shape, centred; the rest of the canvas is transparent.
//   Crop              -- the picture keeps its own shape and covers the canvas; the overflow is cut.
//
// The fit is applied by the clip-transform pass (CompositorEngine::applyClipTransform, the
// layer_transform shader's u_fitEnabled / u_fitScale), BEFORE the clip's position/scale/rotation.
// The picture's size is the media TEXTURE's size, never Clip::clipWidth/clipHeight.
namespace ClipFit
{
enum class Mode : uint8_t { Stretch = 0, Bars = 1, Crop = 2 };   // JSON / REST / OSC ints

// Any int outside 0..2 (an old or hand-edited file, a bad REST/OSC value) -> Stretch.
constexpr Mode clampMode(int v) noexcept
{
    return (v >= 0 && v <= 2) ? static_cast<Mode>(v) : Mode::Stretch;
}

struct Scale { float x = 1.0f, y = 1.0f; };   // {1,1} = identity, exactly
struct Rect  { int x = 0, y = 0, w = 0, h = 0; };

// UV scale for layer_transform's `uv = (uv - 0.5) * s + 0.5` (a picture of iw x ih on a canvas of
// cw x ch). a = iw*ch, b = ih*cw (64-bit): a > b = the picture is wider than the canvas.
// Returns {1,1} EXACTLY for Stretch, any input <= 0, or a == b (same aspect) -- the caller skips the
// fit entirely on {1,1}.
//   Bars: wider -> {1, a/b}; taller -> {b/a, 1}      Crop: wider -> {b/a, 1}; taller -> {1, a/b}
constexpr Scale scale(Mode m, int iw, int ih, int cw, int ch) noexcept
{
    if (m == Mode::Stretch || iw <= 0 || ih <= 0 || cw <= 0 || ch <= 0)
        return {};
    const long long a = static_cast<long long>(iw) * ch;
    const long long b = static_cast<long long>(ih) * cw;
    if (a == b)
        return {};
    const float ab = static_cast<float>(static_cast<double>(a) / static_cast<double>(b));
    const float ba = static_cast<float>(static_cast<double>(b) / static_cast<double>(a));
    const bool wider = a > b;
    if (m == Mode::Bars)
        return wider ? Scale{ 1.0f, ab } : Scale{ ba, 1.0f };
    return wider ? Scale{ ba, 1.0f } : Scale{ 1.0f, ab };   // Crop
}

// Bars: the rect the picture occupies IN THE CANVAS (the largest same-aspect rect that fits,
// centred). Same integer math as RenderGeometry::fitCanvas(iw, ih, cw, ch), so the two agree.
// Empty (all 0) when any input <= 0.
constexpr Rect barsRect(int iw, int ih, int cw, int ch) noexcept
{
    if (iw <= 0 || ih <= 0 || cw <= 0 || ch <= 0)
        return {};
    int w = 0, h = 0;
    if (static_cast<long long>(cw) * ih <= static_cast<long long>(ch) * iw)
    {
        w = cw;
        h = static_cast<int>(static_cast<long long>(cw) * ih / iw);
    }
    else
    {
        h = ch;
        w = static_cast<int>(static_cast<long long>(ch) * iw / ih);
    }
    return { (cw - w) / 2, (ch - h) / 2, w, h };
}

// Crop: the part of the PICTURE that stays visible, in picture pixels (centred).
// Empty (all 0) when any input <= 0.
constexpr Rect cropRect(int iw, int ih, int cw, int ch) noexcept
{
    if (iw <= 0 || ih <= 0 || cw <= 0 || ch <= 0)
        return {};
    int w = 0, h = 0;
    if (static_cast<long long>(cw) * ih <= static_cast<long long>(ch) * iw)
    {
        h = ih;
        w = static_cast<int>(static_cast<long long>(ih) * cw / ch);
    }
    else
    {
        w = iw;
        h = static_cast<int>(static_cast<long long>(iw) * ch / cw);
    }
    return { (iw - w) / 2, (ih - h) / 2, w, h };
}
} // namespace ClipFit
