#include "FeedbackProcessor.h"

using namespace juce::gl;

FeedbackProcessor::~FeedbackProcessor()
{
    // GL resources should be released via releaseGL() before destruction
}

void FeedbackProcessor::initGL(int width, int height)
{
    width_ = width;
    height_ = height;

    for (int i = 0; i < 2; ++i)
    {
        glGenTextures(1, &textures_[i]);
        glBindTexture(GL_TEXTURE_2D, textures_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &fbos_[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, fbos_[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, textures_[i], 0);
    }

    // Clear both to black
    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbos_[i]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    currentBuffer_ = 0;
    initialized_ = true;
}

void FeedbackProcessor::releaseGL()
{
    for (int i = 0; i < 2; ++i)
    {
        if (fbos_[i] != 0)
        {
            glDeleteFramebuffers(1, &fbos_[i]);
            fbos_[i] = 0;
        }
        if (textures_[i] != 0)
        {
            glDeleteTextures(1, &textures_[i]);
            textures_[i] = 0;
        }
    }
    initialized_ = false;
}

void FeedbackProcessor::ensureSize(int width, int height)
{
    if (width == width_ && height == height_ && initialized_)
        return;

    releaseGL();
    initGL(width, height);
}

GLuint FeedbackProcessor::process(GLuint inputTexture,
                                   const FeedbackConfig& config,
                                   ShaderManager& shaderMgr,
                                   FullscreenQuad& quad,
                                   int width, int height)
{
    ensureSize(width, height);
    if (!initialized_)
        return inputTexture;

    auto* prog = shaderMgr.getProgram("feedback_blend");
    if (prog == nullptr)
        return inputTexture;

    // Previous frame is in the "read" buffer
    int readBuffer = currentBuffer_;
    int writeBuffer = 1 - currentBuffer_;

    // Render blended result into write buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbos_[writeBuffer]);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    prog->use();

    // Unit 0: current frame (input)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_currentFrame"), 0);

    // Unit 1: previous frame (feedback buffer)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_[readBuffer]);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_previousFrame"), 1);

    // Feedback parameters
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_amount"), config.amount);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_scaleX"), config.scaleX);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_scaleY"), config.scaleY);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_rotation"), config.rotation);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_offsetX"), config.offsetX);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_offsetY"), config.offsetY);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_feedback_lumaKey"), config.lumaKey);

    glActiveTexture(GL_TEXTURE0);
    quad.draw();

    // Swap buffers: write becomes read for next frame
    currentBuffer_ = writeBuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return textures_[writeBuffer];
}

// === Feedback Presets ===

const std::vector<FeedbackPreset>& FeedbackProcessor::getPresets()
{
    static const std::vector<FeedbackPreset> presets = {
        {"Zoom In", {true, 0.85f, 0.97f, 0.97f, 0.0f, 0.0f, 0.0f, 0.0f, "Zoom In"}},
        {"Spiral",  {true, 0.80f, 0.98f, 0.98f, 2.0f, 0.0f, 0.0f, 0.0f, "Spiral"}},
        {"Drift",   {true, 0.75f, 1.0f,  1.0f,  0.0f, 0.01f, 0.0f, 0.0f, "Drift"}},
        {"Kaleidoscope", {true, 0.90f, 0.95f, 0.95f, 5.0f, 0.0f, 0.0f, 0.0f, "Kaleidoscope"}},
        {"Echo",    {true, 0.60f, 1.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.0f, "Echo"}},
        {"Stretch", {true, 0.85f, 1.02f, 0.98f, 0.0f, 0.0f, 0.0f, 0.0f, "Stretch"}}
    };
    return presets;
}

bool FeedbackProcessor::applyPreset(const std::string& name, FeedbackConfig& config)
{
    for (const auto& preset : getPresets())
    {
        if (preset.name == name)
        {
            config = preset.config;
            return true;
        }
    }
    return false;
}
