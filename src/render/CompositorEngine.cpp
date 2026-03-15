#include "CompositorEngine.h"
#include "render/EmbeddedShaders.h"
#include <iostream>

using namespace juce::gl;

void CompositorEngine::initGL(int width, int height)
{
    fboWidth_ = width;
    fboHeight_ = height;
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    glInitialized_ = true;
}

void CompositorEngine::releaseGL()
{
    deleteFBO(accumulatorFBO_, accumulatorTex_);
    deleteFBO(scratchFBO_, scratchTex_);

    for (auto& [path, tex] : textureCache_)
    {
        if (tex != 0)
            glDeleteTextures(1, &tex);
    }
    textureCache_.clear();
    glInitialized_ = false;
}

void CompositorEngine::resize(int width, int height)
{
    if (width == fboWidth_ && height == fboHeight_)
        return;

    fboWidth_ = width;
    fboHeight_ = height;
    deleteFBO(accumulatorFBO_, accumulatorTex_);
    deleteFBO(scratchFBO_, scratchTex_);
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
}

void CompositorEngine::createFBO(GLuint& fbo, GLuint& tex, int w, int h)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CompositorEngine::deleteFBO(GLuint& fbo, GLuint& tex)
{
    if (fbo != 0) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
    if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
}

GLuint CompositorEngine::loadKeyImage(const juce::File& imageFile)
{
    auto path = imageFile.getFullPathName().toStdString();
    auto it = textureCache_.find(path);
    if (it != textureCache_.end())
        return it->second;

    juce::Image img = juce::ImageFileFormat::loadFrom(imageFile);
    if (!img.isValid())
        return 0;

    // Convert to RGBA
    img = img.convertedToFormat(juce::Image::ARGB);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // JUCE stores ARGB with premultiplied alpha in BGRA byte order
    // We need to convert to GL_RGBA
    int w = img.getWidth();
    int h = img.getHeight();
    std::vector<uint8_t> rgba(static_cast<size_t>(w * h * 4));

    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            auto pixel = bmp.getPixelColour(x, y);
            size_t idx = static_cast<size_t>((y * w + x) * 4);
            rgba[idx + 0] = pixel.getRed();
            rgba[idx + 1] = pixel.getGreen();
            rgba[idx + 2] = pixel.getBlue();
            rgba[idx + 3] = pixel.getAlpha();
        }
    }

    // Flip Y for OpenGL (bottom-up)
    std::vector<uint8_t> flipped(rgba.size());
    size_t rowBytes = static_cast<size_t>(w * 4);
    for (int y = 0; y < h; ++y)
        std::memcpy(flipped.data() + static_cast<size_t>(y) * rowBytes,
                     rgba.data() + static_cast<size_t>((h - 1 - y)) * rowBytes,
                     rowBytes);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, flipped.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    textureCache_[path] = tex;
    return tex;
}

GLuint CompositorEngine::getKeyTexture(const juce::File& imageFile)
{
    auto path = imageFile.getFullPathName().toStdString();
    auto it = textureCache_.find(path);
    if (it != textureCache_.end())
        return it->second;
    return loadKeyImage(imageFile);
}

GLuint CompositorEngine::composite(KeyboardLayout& layout,
                                    ShaderManager& shaderMgr,
                                    FullscreenQuad& quad,
                                    float time,
                                    int width, int height)
{
    auto activeKeys = layout.getActiveKeysSorted();
    hasActiveKeys_ = !activeKeys.empty();

    if (!hasActiveKeys_ || !glInitialized_)
        return 0;

    // Resize FBOs if needed
    resize(width, height);

    // Clear accumulator to transparent black
    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Composite each active key (bottom to top)
    for (auto* key : activeKeys)
    {
        if (key->isEmpty())
            continue;

        if (key->hasMedia() && key->mediaType == KeySlot::MediaType::Image)
        {
            // Get the key's image texture
            GLuint keyTex = getKeyTexture(key->mediaFile);
            if (keyTex == 0)
                continue;

            // Step 1: Apply keying to generate alpha → render to scratch FBO
            applyKeying(*key, keyTex, scratchFBO_, shaderMgr, quad, width, height);

            // Step 2: Blend scratch onto accumulator using key's blend mode
            blendOntoAccumulator(*key, scratchTex_, shaderMgr, quad, width, height);
        }
        // TODO: effects-only keys (apply effects to accumulator)
    }

    return accumulatorTex_;
}

void CompositorEngine::applyKeying(const KeySlot& key, GLuint srcTex, GLuint dstFBO,
                                    ShaderManager& shaderMgr, FullscreenQuad& quad,
                                    int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Select keying shader based on mode
    const char* shaderKey = nullptr;
    switch (key.keyingMode)
    {
        case KeySlot::KeyingMode::Alpha:              shaderKey = "key_alpha"; break;
        case KeySlot::KeyingMode::LumaKey:            shaderKey = "key_luma"; break;
        case KeySlot::KeyingMode::InvertedLumaKey:    shaderKey = "key_inv_luma"; break;
        case KeySlot::KeyingMode::LumaIsAlpha:        shaderKey = "key_luma_alpha"; break;
        case KeySlot::KeyingMode::InvertedLumaIsAlpha:shaderKey = "key_inv_luma_alpha"; break;
        case KeySlot::KeyingMode::ChromaKey:          shaderKey = "key_chroma"; break;
        case KeySlot::KeyingMode::MaxRGB:             shaderKey = "key_max_rgb"; break;
        case KeySlot::KeyingMode::SaturationKey:      shaderKey = "key_saturation"; break;
        case KeySlot::KeyingMode::EdgeDetection:      shaderKey = "key_edge"; break;
        case KeySlot::KeyingMode::ThresholdMask:      shaderKey = "key_threshold"; break;
        case KeySlot::KeyingMode::ChannelR:           shaderKey = "key_channel_r"; break;
        case KeySlot::KeyingMode::ChannelG:           shaderKey = "key_channel_g"; break;
        case KeySlot::KeyingMode::ChannelB:           shaderKey = "key_channel_b"; break;
        case KeySlot::KeyingMode::VignetteAlpha:      shaderKey = "key_vignette"; break;
    }

    // For now, use a simple passthrough with alpha if shader not yet compiled
    auto* prog = shaderMgr.getProgram(shaderKey ? shaderKey : "key_alpha");
    if (!prog)
        prog = shaderMgr.getProgram("passthrough");
    if (!prog)
        return;

    prog->use();

    // Set common uniforms
    auto loc = [&](const char* name) { return glGetUniformLocation(prog->getProgramID(), name); };
    glUniform1i(loc("u_texture"), 0);
    glUniform1f(loc("u_opacity"), key.opacity);
    glUniform1f(loc("u_threshold"), key.keyThreshold);
    glUniform1f(loc("u_softness"), key.keySoftness);
    glUniform3f(loc("u_chroma_key_color"), key.chromaKeyR, key.chromaKeyG, key.chromaKeyB);
    glUniform1f(loc("u_chroma_tolerance"), key.chromaKeyTolerance);
    glUniform1f(loc("u_chroma_softness"), key.chromaKeySoftness);
    glUniform2f(loc("u_resolution"), static_cast<float>(w), static_cast<float>(h));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    quad.draw();
}

void CompositorEngine::blendOntoAccumulator(const KeySlot& key, GLuint srcTex,
                                             ShaderManager& shaderMgr, FullscreenQuad& quad,
                                             int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, w, h);

    // For Additive blend mode, use GL blending (fastest, most common VJ mode)
    if (key.blendMode == KeySlot::BlendMode::Additive)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive with alpha
        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        quad.draw();
        glDisable(GL_BLEND);
        return;
    }

    // For Normal blend, use standard alpha blending
    if (key.blendMode == KeySlot::BlendMode::Normal)
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        quad.draw();
        glDisable(GL_BLEND);
        return;
    }

    // For Screen mode, use GL: ONE, ONE_MINUS_SRC_COLOR
    if (key.blendMode == KeySlot::BlendMode::Screen)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        quad.draw();
        glDisable(GL_BLEND);
        return;
    }

    // For Multiply mode, use GL: DST_COLOR, ZERO
    if (key.blendMode == KeySlot::BlendMode::Multiply)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        quad.draw();
        glDisable(GL_BLEND);
        return;
    }

    // For Darken/Lighten, use GL blend equations
    if (key.blendMode == KeySlot::BlendMode::Darken || key.blendMode == KeySlot::BlendMode::Lighten)
    {
        glEnable(GL_BLEND);
        glBlendEquation(key.blendMode == KeySlot::BlendMode::Darken ? GL_MIN : GL_MAX);
        glBlendFunc(GL_ONE, GL_ONE);
        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        quad.draw();
        glBlendEquation(GL_FUNC_ADD); // reset
        glDisable(GL_BLEND);
        return;
    }

    // For all other blend modes, fall back to standard alpha blending
    // (Proper implementation requires a 2-texture shader reading both src and dst,
    //  which will be added when the blend mode shader system is complete)
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    auto* prog = shaderMgr.getProgram("passthrough");
    if (prog)
    {
        prog->use();
        glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    quad.draw();
    glDisable(GL_BLEND);
}
