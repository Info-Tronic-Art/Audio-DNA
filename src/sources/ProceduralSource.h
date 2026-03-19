#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_core/juce_core.h>
#include "render/ShaderManager.h"
#include "render/FullscreenQuad.h"
#include "analysis/FeatureSnapshot.h"
#include <string>
#include <vector>

// ProceduralSource: base class for procedural content generators.
//
// A source generates a texture each frame via a GLSL fragment shader.
// Sources act as the "media" for a clip — they replace image/video content
// and feed into the same effect chain pipeline.
//
// Most sources are single-pass fragment shaders (render to FBO each frame).
// Stateful sources (Reaction-Diffusion, Cellular Automata) use ping-pong FBOs
// to persist state between frames.
class ProceduralSource
{
public:
    struct Param
    {
        std::string name;           // Display name (e.g., "Octaves")
        std::string uniformName;    // GLSL uniform name (e.g., "u_src_octaves")
        float value = 0.5f;
        float defaultValue = 0.5f;
        float min = 0.0f;
        float max = 1.0f;
    };

    ProceduralSource(const std::string& id, const std::string& displayName,
                     const std::string& category, const std::string& shaderName,
                     bool stateful = false);
    virtual ~ProceduralSource();

    // Initialize GL resources (FBOs for stateful sources). Call on GL thread.
    virtual void initGL(int width, int height);

    // Release GL resources. Call on GL thread.
    virtual void releaseGL();

    // Render the source to an internal FBO and return the texture ID.
    // Non-stateful sources render fresh each frame.
    // Stateful sources read previous state and write new state (ping-pong).
    virtual GLuint render(ShaderManager& shaderMgr, FullscreenQuad& quad,
                          float time, int width, int height,
                          const FeatureSnapshot& snapshot);

    // Resize internal FBOs if needed.
    virtual void resize(int width, int height);

    // Reset state (clear ping-pong buffers for stateful sources).
    virtual void reset();

    // Identity
    const std::string& getId() const { return id_; }
    const std::string& getDisplayName() const { return displayName_; }
    const std::string& getCategory() const { return category_; }
    const std::string& getShaderName() const { return shaderName_; }

    // Parameters
    int getNumParams() const { return static_cast<int>(params_.size()); }
    Param& getParam(int index) { return params_[static_cast<size_t>(index)]; }
    const Param& getParam(int index) const { return params_[static_cast<size_t>(index)]; }
    void setParamValue(int index, float value);
    void resetParams();

    // Whether this source needs ping-pong FBOs (stateful)
    bool isStateful() const { return stateful_; }

    // Whether GL has been initialized
    bool isGLInitialized() const { return glInitialized_; }

    ProceduralSource(const ProceduralSource&) = delete;
    ProceduralSource& operator=(const ProceduralSource&) = delete;

    // Add a parameter definition. Call during construction.
    void addParam(const std::string& name, const std::string& uniformName,
                  float defaultValue = 0.5f, float min = 0.0f, float max = 1.0f);

protected:

    // Upload source-specific uniforms (params + audio features).
    // Called by render() before drawing the quad.
    virtual void uploadUniforms(juce::OpenGLShaderProgram* program,
                                float time, int width, int height,
                                const FeatureSnapshot& snapshot);

    std::string id_;
    std::string displayName_;
    std::string category_;
    std::string shaderName_;
    std::vector<Param> params_;
    bool stateful_ = false;

    // Single output FBO (non-stateful sources)
    GLuint outputFBO_ = 0;
    GLuint outputTex_ = 0;

    // Ping-pong FBOs (stateful sources)
    GLuint pingFBO_[2] = {0, 0};
    GLuint pingTex_[2] = {0, 0};
    int currentPing_ = 0; // which buffer holds current state

    int fboWidth_ = 0;
    int fboHeight_ = 0;
    bool glInitialized_ = false;

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);
};
