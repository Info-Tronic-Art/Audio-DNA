#pragma once
#include <juce_opengl/juce_opengl.h>
#include "model/Layer.h"
#include "render/ShaderManager.h"
#include "render/FullscreenQuad.h"
#include <vector>
#include <string>

// Named feedback presets
struct FeedbackPreset
{
    std::string name;
    FeedbackConfig config;
};

// FeedbackProcessor: owns an FBO pair for per-layer feedback.
// Each frame:
//   1. Read previous frame from feedback FBO
//   2. Apply transform (scale, rotate, offset) to previous frame
//   3. Blend with current input at feedback.amount
//   4. Store result for next frame
class FeedbackProcessor
{
public:
    FeedbackProcessor() = default;
    ~FeedbackProcessor();

    // Initialize GL resources. Call from openGLContextCreated.
    void initGL(int width, int height);

    // Release GL resources.
    void releaseGL();

    // Process one frame of feedback.
    GLuint process(GLuint inputTexture,
                   const FeedbackConfig& config,
                   ShaderManager& shaderMgr,
                   FullscreenQuad& quad,
                   int width, int height);

    bool isInitialized() const { return initialized_; }

    // Get the list of built-in feedback presets
    static const std::vector<FeedbackPreset>& getPresets();

    // Apply a preset by name. Returns true if found.
    static bool applyPreset(const std::string& name, FeedbackConfig& config);

private:
    GLuint fbos_[2] = {};
    GLuint textures_[2] = {};
    int currentBuffer_ = 0;
    int width_ = 0;
    int height_ = 0;
    bool initialized_ = false;

    void ensureSize(int width, int height);
};
