#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_core/juce_core.h>

// LUTLoader: loads .cube LUT files into GL_TEXTURE_3D handles.
// Used by the Color Grade (LUT) effect in P15.
//
// .cube format:
//   Lines starting with # or TITLE are comments/metadata.
//   LUT_3D_SIZE N → N^3 entries follow.
//   Each line: R G B (floats in [0, 1]).
//   Entries ordered: B fastest, G middle, R slowest.
class LUTLoader
{
public:
    // Parse a .cube file and upload to a GL_TEXTURE_3D.
    // Returns the texture handle, or 0 on failure.
    // Must be called on the GL thread.
    static GLuint loadCubeFile(const juce::File& file);

    // Release a LUT texture.
    static void releaseLUT(GLuint texId);
};
