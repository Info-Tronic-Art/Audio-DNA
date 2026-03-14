#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_core/juce_core.h>
#include <unordered_map>
#include <string>

// ShaderManager: compiles vertex + fragment shaders, links programs,
// caches uniform locations, and supports hot-reload from the shaders/ directory.
//
// All methods must be called on the GL thread unless noted otherwise.
class ShaderManager
{
public:
    explicit ShaderManager(juce::OpenGLContext& context);
    ~ShaderManager() = default;

    // Set the directory where .vert/.frag files live.
    // Can be called from the message thread before GL init.
    void setShadersDirectory(const juce::File& dir);

    // Compile a shader program from vertex + fragment source strings.
    // Returns the program name on success, empty string on failure.
    // The program is stored internally and can be retrieved by name.
    bool compileProgram(const juce::String& name,
                        const juce::String& vertexSource,
                        const juce::String& fragmentSource);

    // Compile a shader program from files in the shaders directory.
    // vertFile/fragFile are filenames (not full paths).
    bool compileProgramFromFiles(const juce::String& name,
                                 const juce::String& vertFile,
                                 const juce::String& fragFile);

    // Get a compiled program by name. Returns nullptr if not found.
    juce::OpenGLShaderProgram* getProgram(const juce::String& name) const;

    // Get a cached uniform location. Returns -1 if not found.
    GLint getUniformLocation(const juce::String& programName,
                             const juce::String& uniformName);

    // Reload all programs from their original source files.
    // Returns number of programs that failed to reload.
    int reloadAll();

    // Release all GL resources.
    void releaseAll();

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

private:
    struct ProgramEntry
    {
        std::unique_ptr<juce::OpenGLShaderProgram> program;
        juce::String vertFile;
        juce::String fragFile;
        std::unordered_map<std::string, GLint> uniformCache;
    };

    juce::OpenGLContext& context_;
    juce::File shadersDir_;
    std::unordered_map<std::string, ProgramEntry> programs_;
};
