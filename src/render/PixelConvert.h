#pragma once
#include <juce_graphics/juce_graphics.h>
#include <cstdint>

// PixelConvert: GL RGBA8 rows (bottom-up, tightly packed w*4) -> a juce::Image::ARGB BitmapData, byte-for-byte what
// the per-pixel setPixelColour(x, y, Colour(r, g, b, a)) loop produced (premultiplied B,G,R,A; forceOpaque = the
// output_probe variant, alpha 255). Proof: tests/test_pixel_convert.cpp (s-rta-0927 plan-renderperf C2).
namespace PixelConvert
{
inline void rgbaBottomUpToARGB(const uint8_t* rgba, int w, int h, juce::Image::BitmapData& dst, bool forceOpaque) noexcept
{
    jassert(dst.pixelFormat == juce::Image::ARGB && dst.pixelStride == 4 && dst.width == w && dst.height == h);
    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = rgba + static_cast<size_t>(h - 1 - y) * static_cast<size_t>(w) * 4;
        auto* out = reinterpret_cast<juce::PixelARGB*>(dst.getLinePointer(y));
        for (int x = 0; x < w; ++x, src += 4)
        {
            juce::PixelARGB p(forceOpaque ? uint8_t(255) : src[3], src[0], src[1], src[2]);
            p.premultiply();
            out[x].set(p);
        }
    }
}

// s-rta-0928 renderleft R1.1 -- the reverse direction: JUCE ARGB BitmapData (premultiplied B,G,R,A) -> GL RGBA8 rows,
// bottom-up, tightly packed w*4. unpremultiply = the bytes of the old getPixelColour loops
// (CompositorEngine::loadKeyImage, ImageSequence::loadImageToTexture: straight RGBA); false =
// TextureManager::uploadImage's raw swizzle (premultiplied RGBA). Proof: tests/test_pixel_convert.cpp.
inline void argbToGlRgbaBottomUp(const juce::Image::BitmapData& src, uint8_t* dst, bool unpremultiply) noexcept
{
    jassert(src.pixelFormat == juce::Image::ARGB && src.pixelStride == 4);
    const int w = src.width, h = src.height;
    for (int y = 0; y < h; ++y)
    {
        const auto* in = reinterpret_cast<const juce::PixelARGB*>(src.getLinePointer(y));
        uint8_t* out = dst + static_cast<size_t>(h - 1 - y) * static_cast<size_t>(w) * 4;
        for (int x = 0; x < w; ++x, out += 4)
        {
            juce::PixelARGB p = in[x];
            if (unpremultiply) p.unpremultiply();
            out[0] = p.getRed(); out[1] = p.getGreen(); out[2] = p.getBlue(); out[3] = p.getAlpha();
        }
    }
}
} // namespace PixelConvert
