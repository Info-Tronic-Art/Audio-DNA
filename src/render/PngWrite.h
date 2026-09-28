#pragma once
#include <juce_graphics/juce_graphics.h>

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
}
