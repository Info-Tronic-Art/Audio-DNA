#include "CompositorEngine.h"
#include "render/EmbeddedShaders.h"
#include <iostream>
#include <cmath>

using namespace juce::gl;

void CompositorEngine::initGL(int width, int height)
{
    fboWidth_ = width;
    fboHeight_ = height;
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    createFBO(effectFBO_A_, effectTex_A_, width, height);
    createFBO(effectFBO_B_, effectTex_B_, width, height);
    glInitialized_ = true;
}

void CompositorEngine::releaseGL()
{
    deleteFBO(accumulatorFBO_, accumulatorTex_);
    deleteFBO(scratchFBO_, scratchTex_);
    deleteFBO(effectFBO_A_, effectTex_A_);
    deleteFBO(effectFBO_B_, effectTex_B_);

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
    deleteFBO(effectFBO_A_, effectTex_A_);
    deleteFBO(effectFBO_B_, effectTex_B_);
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    createFBO(effectFBO_A_, effectTex_A_, width, height);
    createFBO(effectFBO_B_, effectTex_B_, width, height);
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

// === Per-clip effect chain rendering (P13.5.1) ===

GLuint CompositorEngine::applyClipEffects(const Clip& clip, GLuint inputTex,
                                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                                            float time, int w, int h)
{
    if (clip.effects.empty() || effectLibrary_ == nullptr)
        return inputTex;

    GLuint currentInput = inputTex;
    int writeFBO = 0; // 0 = effectFBO_A_, 1 = effectFBO_B_

    for (const auto& slot : clip.effects)
    {
        if (!slot.enabled || slot.bypassed)
            continue;

        auto* program = shaderMgr.getProgram(juce::String(slot.effectName));
        if (program == nullptr)
            continue;

        // Get the effect definition to know uniform names
        const auto* def = effectLibrary_->getEffectDef(juce::String(slot.effectName));
        if (def == nullptr)
            continue;

        GLuint targetFBO = (writeFBO == 0) ? effectFBO_A_ : effectFBO_B_;

        glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_BLEND);

        program->use();

        // Bind input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentInput);
        auto texLoc = program->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        // Global uniforms
        auto timeLoc = program->getUniformIDFromName("u_time");
        if (timeLoc >= 0)
            glUniform1f(timeLoc, time);
        auto resLoc = program->getUniformIDFromName("u_resolution");
        if (resLoc >= 0)
            glUniform2f(resLoc, static_cast<float>(w), static_cast<float>(h));

        // Effect parameter uniforms from slot values
        for (size_t p = 0; p < def->params.size() && p < slot.paramValues.size(); ++p)
        {
            auto loc = program->getUniformIDFromName(def->params[p].uniformName.c_str());
            if (loc >= 0)
                glUniform1f(loc, slot.paramValues[p]);
        }

        quad.draw();

        currentInput = (writeFBO == 0) ? effectTex_A_ : effectTex_B_;
        writeFBO = 1 - writeFBO; // ping-pong
    }

    return currentInput;
}

// === Layer transform (P13.5.5) ===

GLuint CompositorEngine::applyLayerTransform(const Layer& layer, GLuint srcTex,
                                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                                               int w, int h)
{
    // Skip transform if all values are at defaults
    constexpr float eps = 0.001f;
    bool needsTransform = (std::abs(layer.positionX) > eps ||
                           std::abs(layer.positionY) > eps ||
                           std::abs(layer.layerScale - 1.0f) > eps ||
                           std::abs(layer.layerRotation) > eps);
    if (!needsTransform)
        return srcTex;

    auto* prog = shaderMgr.getProgram("layer_transform");
    if (prog == nullptr)
        return srcTex;

    // Render transformed result into scratch FBO
    glBindFramebuffer(GL_FRAMEBUFFER, scratchFBO_);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    prog->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_translate"),
                layer.positionX, layer.positionY);
    glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_anchor"),
                layer.layerAnchorX, layer.layerAnchorY);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_scale"),
                layer.layerScale);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_rotation"),
                layer.layerRotation);

    quad.draw();

    return scratchTex_;
}

// === FX Only layer (P13.5.2) ===

void CompositorEngine::applyFXOnlyLayer(const Clip& clip, ShaderManager& shaderMgr,
                                          FullscreenQuad& quad, float time, int w, int h)
{
    if (clip.effects.empty() || effectLibrary_ == nullptr)
        return;

    // Apply the clip's effects to the accumulator texture
    GLuint result = applyClipEffects(clip, accumulatorTex_, shaderMgr, quad, time, w, h);

    if (result != accumulatorTex_)
    {
        // Copy result back to accumulator
        glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
        glViewport(0, 0, w, h);
        glDisable(GL_BLEND);

        auto* prog = shaderMgr.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, result);
        quad.draw();
    }
}

// === Mask layer (P13.5.3) ===

void CompositorEngine::applyMaskLayer(const Clip& /*clip*/, GLuint clipTex,
                                        ShaderManager& shaderMgr, FullscreenQuad& quad,
                                        int w, int h)
{
    auto* prog = shaderMgr.getProgram("mask_luminance");
    if (prog == nullptr)
        return;

    // Copy current accumulator to scratch first
    glBindFramebuffer(GL_FRAMEBUFFER, scratchFBO_);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    {
        auto* pt = shaderMgr.getProgram("passthrough");
        if (pt)
        {
            pt->use();
            glUniform1i(glGetUniformLocation(pt->getProgramID(), "u_texture"), 0);
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, accumulatorTex_);
        quad.draw();
    }

    // Now render mask result back to accumulator
    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    prog->use();

    // Bind mask texture (clip content) to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clipTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);

    // Bind accumulator copy (scratch) to unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scratchTex_);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_accumulator"), 1);

    glActiveTexture(GL_TEXTURE0);
    quad.draw();
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

    // Check if any layer has an active clip with content
    for (const auto& layer : deck.layers)
    {
        if (!layer.visible || layer.bypassed) continue;
        const Clip* clip = layer.getActiveClip();
        if (clip == nullptr) continue;
        if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
        { hasActiveLayers_ = true; break; }
        if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
        { hasActiveLayers_ = true; break; }
        if (clip->mediaType == Clip::MediaType::Video && clip->mediaFile.existsAsFile())
        { hasActiveLayers_ = true; break; }
        if (clip->mediaType == Clip::MediaType::ImageSequence && !clip->sequenceFiles.empty())
        { hasActiveLayers_ = true; break; }
        // FXOnly layers are active if they have effects (even without media)
        if (layer.type == Layer::Type::FXOnly && !clip->effects.empty())
        { hasActiveLayers_ = true; break; }
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
                // Get clip texture — image, procedural source, video, or image sequence
                GLuint clipTex = 0;
                if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
                {
                    clipTex = getKeyTexture(clip->mediaFile);
                }
                else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty() && sourceRenderFn_)
                {
                    const auto* params = clip->sourceParams.empty() ? nullptr : &clip->sourceParams;
                    clipTex = sourceRenderFn_(clip->sourceType, time, width, height, params);
                }
                else if ((clip->mediaType == Clip::MediaType::Video ||
                          clip->mediaType == Clip::MediaType::ImageSequence) && videoFrameFn_)
                {
                    float dt = 1.0f / 60.0f;
                    clipTex = videoFrameFn_(clip, dt);
                }

                if (clipTex == 0) continue;

                // P13.5.1: Apply per-clip effects
                clipTex = applyClipEffects(*clip, clipTex, shaderMgr, quad, time, width, height);

                // P13.5.5: Apply layer transform
                clipTex = applyLayerTransform(layer, clipTex, shaderMgr, quad, width, height);

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
                break;
            }
            case Layer::Type::FXOnly:
            {
                // P13.5.2: Apply clip's effects to the accumulator
                applyFXOnlyLayer(*clip, shaderMgr, quad, time, width, height);
                break;
            }
            case Layer::Type::Mask:
            {
                // P13.5.3: Use clip content as luminance mask on accumulator
                GLuint clipTex = 0;
                if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
                    clipTex = getKeyTexture(clip->mediaFile);
                else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty() && sourceRenderFn_)
                {
                    const auto* params = clip->sourceParams.empty() ? nullptr : &clip->sourceParams;
                    clipTex = sourceRenderFn_(clip->sourceType, time, width, height, params);
                }
                else if ((clip->mediaType == Clip::MediaType::Video ||
                          clip->mediaType == Clip::MediaType::ImageSequence) && videoFrameFn_)
                {
                    float dt = 1.0f / 60.0f;
                    clipTex = videoFrameFn_(clip, dt);
                }

                if (clipTex != 0)
                    applyMaskLayer(*clip, clipTex, shaderMgr, quad, width, height);
                break;
            }
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
