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

// === Deck/Layer-based compositing ===

GLuint CompositorEngine::compositeDeck(Deck& deck,
                                        ShaderManager& shaderMgr,
                                        FullscreenQuad& quad,
                                        float time,
                                        int width, int height)
{
    using namespace juce::gl;

    hasActiveLayers_ = false;

    // Check if any layer has an active clip
    for (const auto& layer : deck.layers)
    {
        if (layer.visible && !layer.bypassed && layer.getActiveClip() != nullptr)
        {
            hasActiveLayers_ = true;
            break;
        }
    }

    if (!hasActiveLayers_ || !glInitialized_)
        return 0;

    resize(width, height);

    // Clear accumulator to transparent black
    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Composite layers bottom to top (index 0 is bottom)
    for (auto& layer : deck.layers)
    {
        if (!layer.visible || layer.bypassed)
            continue;

        const Clip* clip = layer.getActiveClip();
        if (clip == nullptr)
            continue;

        // Handle layer types
        switch (layer.type)
        {
            case Layer::Type::Opaque:
            case Layer::Type::Transparent:
            {
                if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
                {
                    GLuint clipTex = getKeyTexture(clip->mediaFile);
                    if (clipTex == 0) continue;

                    if (layer.type == Layer::Type::Transparent)
                    {
                        // Apply keying → scratch FBO
                        applyLayerKeying(layer, clipTex, scratchFBO_, shaderMgr, quad, width, height);
                        // Blend scratch onto accumulator
                        blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
                    }
                    else
                    {
                        // Opaque: clear accumulator and draw directly
                        glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glDisable(GL_BLEND);
                        auto* prog = shaderMgr.getProgram("passthrough");
                        if (prog)
                        {
                            prog->use();
                            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
                        }
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, clipTex);
                        quad.draw();
                    }
                }
                break;
            }
            case Layer::Type::FXOnly:
                // FX Only: effects applied to accumulator (not implemented yet in render)
                break;
            case Layer::Type::Mask:
                // Mask: content becomes alpha mask for accumulator (not implemented yet)
                break;
            case Layer::Type::ThreeD:
                // 3D: not implemented yet
                break;
        }
    }

    return accumulatorTex_;
}

void CompositorEngine::applyLayerKeying(const Layer& layer, GLuint srcTex, GLuint dstFBO,
                                         ShaderManager& shaderMgr, FullscreenQuad& quad,
                                         int w, int h)
{
    using namespace juce::gl;

    glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Map Layer::KeyingMode to shader name (same shaders as v1)
    const char* shaderKey = "key_alpha";
    switch (layer.keyingMode)
    {
        case Layer::KeyingMode::Alpha:              shaderKey = "key_alpha"; break;
        case Layer::KeyingMode::LumaKey:            shaderKey = "key_luma"; break;
        case Layer::KeyingMode::InvertedLumaKey:    shaderKey = "key_inv_luma"; break;
        case Layer::KeyingMode::LumaIsAlpha:        shaderKey = "key_luma_alpha"; break;
        case Layer::KeyingMode::InvertedLumaIsAlpha:shaderKey = "key_inv_luma_alpha"; break;
        case Layer::KeyingMode::ChromaKey:          shaderKey = "key_chroma"; break;
        case Layer::KeyingMode::MaxRGB:             shaderKey = "key_max_rgb"; break;
        case Layer::KeyingMode::SaturationKey:      shaderKey = "key_saturation"; break;
        case Layer::KeyingMode::EdgeDetection:      shaderKey = "key_edge"; break;
        case Layer::KeyingMode::ThresholdMask:      shaderKey = "key_threshold"; break;
        case Layer::KeyingMode::ChannelR:           shaderKey = "key_channel_r"; break;
        case Layer::KeyingMode::ChannelG:           shaderKey = "key_channel_g"; break;
        case Layer::KeyingMode::ChannelB:           shaderKey = "key_channel_b"; break;
    }

    auto* prog = shaderMgr.getProgram(shaderKey);
    if (!prog)
        prog = shaderMgr.getProgram("passthrough");
    if (!prog)
        return;

    prog->use();

    auto loc = [&](const char* name) { return glGetUniformLocation(prog->getProgramID(), name); };
    glUniform1i(loc("u_texture"), 0);
    glUniform1f(loc("u_opacity"), layer.opacity);
    glUniform1f(loc("u_threshold"), layer.keyThreshold);
    glUniform1f(loc("u_softness"), layer.keySoftness);
    glUniform3f(loc("u_chroma_key_color"), layer.chromaKeyR, layer.chromaKeyG, layer.chromaKeyB);
    glUniform1f(loc("u_chroma_tolerance"), layer.chromaKeyTolerance);
    glUniform2f(loc("u_resolution"), static_cast<float>(w), static_cast<float>(h));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    quad.draw();
}

void CompositorEngine::blendLayerOntoAccumulator(const Layer& layer, GLuint srcTex,
                                                  ShaderManager& shaderMgr, FullscreenQuad& quad,
                                                  int w, int h)
{
    using namespace juce::gl;

    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, w, h);

    auto blendAndDraw = [&](GLenum sfactor, GLenum dfactor) {
        glEnable(GL_BLEND);
        glBlendFunc(sfactor, dfactor);
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
    };

    switch (layer.blendMode)
    {
        case Layer::MixMode::Additive:
            blendAndDraw(GL_SRC_ALPHA, GL_ONE);
            break;
        case Layer::MixMode::Normal:
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            {
                auto* prog = shaderMgr.getProgram("passthrough");
                if (prog) {
                    prog->use();
                    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
                }
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTex);
            quad.draw();
            glDisable(GL_BLEND);
            break;
        case Layer::MixMode::Screen:
            blendAndDraw(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
            break;
        case Layer::MixMode::Multiply:
            blendAndDraw(GL_DST_COLOR, GL_ZERO);
            break;
        case Layer::MixMode::Darken:
        case Layer::MixMode::Lighten:
            glEnable(GL_BLEND);
            glBlendEquation(layer.blendMode == Layer::MixMode::Darken ? GL_MIN : GL_MAX);
            glBlendFunc(GL_ONE, GL_ONE);
            {
                auto* prog = shaderMgr.getProgram("passthrough");
                if (prog) {
                    prog->use();
                    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
                }
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTex);
            quad.draw();
            glBlendEquation(GL_FUNC_ADD);
            glDisable(GL_BLEND);
            break;
        default:
            // Fallback: standard alpha blend for unimplemented modes
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            {
                auto* prog = shaderMgr.getProgram("passthrough");
                if (prog) {
                    prog->use();
                    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
                }
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTex);
            quad.draw();
            glDisable(GL_BLEND);
            break;
    }
}

