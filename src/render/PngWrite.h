#pragma once
#include <juce_graphics/juce_graphics.h>
#include <array>
#include <cstdint>

// PngWrite: the one PNG writer for captures (Renderer::captureFrame -> render_frame / takeSnapshot) and TestServer's
// output_probe. Proof: tests/test_png_write.cpp (s-rta-0927 follow-ups F2).
namespace PngWrite
{
// Writes img as a PNG to file, REPLACING any existing file. A juce::FileOutputStream opens an existing file at its
// END (juce_SharedCode_posix.h openHandle: lseek SEEK_END), so a plain stream write APPENDS a second PNG after the
// old one and every decoder returns the OLD picture (s-rta-0927 renderperf found_not_fixed #4; Pitfall 46).
inline bool writeReplacing(const juce::Image& img, const juce::File& file)
{
    file.getParentDirectory().createDirectory();
    if (file.existsAsFile() && !file.deleteFile())
        return false;
    juce::FileOutputStream fos(file);
    return fos.openedOk() && juce::PNGImageFormat().writeImageToStream(img, fos);
}

// s-rta-0928 renderleft R3: the fast writer render_frame uses (7070 + 8080): zlib level kFastPngLevel, filter 0, the
// same straight RGBA JUCE's writer emits (PixelConvert::rgbaBottomUpToPngScanlines) -- decoded pixels are identical,
// file bytes differ (no sBIT chunk, another compressor setting). Snapshots (the user's files) keep writeReplacing.
inline constexpr int kFastPngLevel = 1;

inline uint32_t crc32(const uint8_t* data, size_t n, uint32_t crc = 0xFFFFFFFFu) noexcept
{
    static const auto table = [] {
        std::array<uint32_t, 256> t{};
        for (uint32_t i = 0; i < 256; ++i)
        {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1u) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            t[i] = c;
        }
        return t;
    }();
    for (size_t i = 0; i < n; ++i)
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    return crc;
}

// Writes scanlines (h rows of filter byte + w*4 RGBA) as an 8-bit RGBA, non-interlaced PNG: signature, IHDR, one IDAT
// (the zlib stream of the scanlines at `level`), IEND. REPLACES an existing file (deletes first, Pitfall 46).
inline bool writeScanlinesReplacing(const uint8_t* scanlines, int w, int h, const juce::File& file, int level)
{
    if (w <= 0 || h <= 0 || scanlines == nullptr)
        return false;
    juce::MemoryOutputStream idat;
    {
        juce::GZIPCompressorOutputStream z(idat, level, 0);   // windowBits 0 = MAX_WBITS: a zlib (RFC 1950) stream
        if (!z.write(scanlines, static_cast<size_t>(h) * (1 + static_cast<size_t>(w) * 4)))
            return false;
        z.flush();
    }
    auto be32 = [](uint8_t* p, uint32_t v) {
        p[0] = static_cast<uint8_t>(v >> 24); p[1] = static_cast<uint8_t>(v >> 16);
        p[2] = static_cast<uint8_t>(v >> 8);  p[3] = static_cast<uint8_t>(v);
    };
    juce::MemoryOutputStream png;
    static const uint8_t sig[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    png.write(sig, 8);
    auto chunk = [&](const char* type, const uint8_t* data, size_t n) {
        uint8_t len[4]; be32(len, static_cast<uint32_t>(n));
        png.write(len, 4);
        png.write(type, 4);
        if (n > 0) png.write(data, n);
        uint32_t c = crc32(reinterpret_cast<const uint8_t*>(type), 4);
        if (n > 0) c = crc32(data, n, c);
        uint8_t crc[4]; be32(crc, c ^ 0xFFFFFFFFu);
        png.write(crc, 4);
    };
    uint8_t ihdr[13];
    be32(ihdr, static_cast<uint32_t>(w)); be32(ihdr + 4, static_cast<uint32_t>(h));
    ihdr[8] = 8; ihdr[9] = 6; ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;   // 8-bit RGBA, deflate, filter 0, no interlace
    chunk("IHDR", ihdr, sizeof(ihdr));
    chunk("IDAT", static_cast<const uint8_t*>(idat.getData()), idat.getDataSize());
    chunk("IEND", nullptr, 0);

    file.getParentDirectory().createDirectory();
    if (file.existsAsFile() && !file.deleteFile())
        return false;
    juce::FileOutputStream fos(file);
    return fos.openedOk() && fos.write(png.getData(), png.getDataSize());
}
}
