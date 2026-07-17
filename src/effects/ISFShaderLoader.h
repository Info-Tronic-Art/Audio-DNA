#pragma once
#include <juce_core/juce_core.h>
#include "effects/EffectLibrary.h"
#include <string>
#include <vector>

// ISFShaderLoader: imports Interactive Shader Format (ISF) shaders from isf.video
// and registers them as effects in the EffectLibrary.
//
// ISF shaders have JSON metadata in a comment block at the top:
//   /*{
//     "DESCRIPTION": "...",
//     "INPUTS": [
//       { "NAME": "intensity", "TYPE": "float", "DEFAULT": 0.5, "MIN": 0.0, "MAX": 1.0 },
//       { "NAME": "color", "TYPE": "color", "DEFAULT": [1,0,0,1] }
//     ],
//     "PASSES": [...]
//   }*/
//
// The loader:
//   1. Extracts the JSON metadata block
//   2. Parses INPUTS to create parameter definitions
//   3. Wraps the GLSL to match our uniform naming (u_[paramName])
//   4. Registers the effect in EffectLibrary with category "ISF"
//
// Limitations:
//   - Only single-pass ISF shaders supported (no multi-pass)
//   - image/audio/event inputs not supported (only float/bool/long/point2D/color)
//   - Requires OpenGL 4.1 (our minimum)
class ISFShaderLoader
{
public:
    struct ISFParam
    {
        std::string name;
        std::string type;       // "float", "bool", "long", "point2D", "color"
        float defaultValue = 0.5f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
    };

    struct ISFShader
    {
        std::string name;
        std::string description;
        std::string category;       // From CATEGORIES or "ISF"
        std::string glslSource;     // The fragment shader source (after ISF wrapper)
        std::vector<ISFParam> params;
        bool valid = false;
    };

    // Parse an ISF file and return the shader info.
    // Returns ISFShader with valid=true on success.
    static ISFShader parseISFFile(const juce::File& file);

    // Parse ISF from a string source.
    static ISFShader parseISFSource(const std::string& source, const std::string& name);

    // Convert an ISF shader to our GLSL format.
    // Wraps ISF's `isf_FragNormCoord`, `IMG_NORM_PIXEL`, `TIME`, `RENDERSIZE`
    // into our u_time, u_resolution, texture2D conventions.
    static std::string convertToGLSL(const ISFShader& isf);

    // Get the ISF import directory (user's ISF folder)
    static juce::File getISFDirectory();

private:
    // Extract the JSON metadata block from ISF source
    static std::string extractJSONBlock(const std::string& source);

    // Remove the JSON block to get pure GLSL
    static std::string extractGLSLBody(const std::string& source);
};
