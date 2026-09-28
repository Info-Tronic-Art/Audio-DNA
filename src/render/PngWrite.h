#pragma once
#include <juce_graphics/juce_graphics.h>

// PngWrite: the one PNG writer for captures (Renderer::captureFrame -> render_frame / takeSnapshot) and TestServer's
// output_probe. Proof: tests/test_png_write.cpp (s-rta-0927 follow-ups F2).
namespace PngWrite
{
// Writes img as a PNG to file, REPLACING any existing file.
inline bool writeReplacing(const juce::Image& img, const juce::File& file)
{
    file.getParentDirectory().createDirectory();
    juce::FileOutputStream fos(file);
    return fos.openedOk() && juce::PNGImageFormat().writeImageToStream(img, fos);
}
}
