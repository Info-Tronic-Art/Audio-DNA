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
    createFBO(transitionFBO_, transitionTex_, width, height);
    createFBO(feedbackFBO_, feedbackTex_, width, height);
    feedbackReady_ = false;
    glInitialized_ = true;
}

void CompositorEngine::releaseGL()
{
    deleteFBO(accumulatorFBO_, accumulatorTex_);
    deleteFBO(scratchFBO_, scratchTex_);
    deleteFBO(effectFBO_A_, effectTex_A_);
    deleteFBO(effectFBO_B_, effectTex_B_);
    deleteFBO(transitionFBO_, transitionTex_);
    deleteFBO(feedbackFBO_, feedbackTex_);

    for (auto& [path, tex] : textureCache_)
    {
        if (tex != 0)
            glDeleteTextures(1, &tex);
    }
    textureCache_.clear();

    // Release per-layer feedback processors
    for (auto& [id, proc] : feedbackProcessors_)
        proc->releaseGL();
    feedbackProcessors_.clear();

    // Release per-layer temporal buffers
    for (auto& [id, buf] : layerTemporalBuffers_)
    {
        if (buf.fbo != 0) glDeleteFramebuffers(1, &buf.fbo);
        if (buf.tex != 0) glDeleteTextures(1, &buf.tex);
    }
    layerTemporalBuffers_.clear();

    // Release per-layer output textures (Layer Router P20)
    for (auto& [id, fbo] : layerOutputFBOs_)
    {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
    }
    for (auto& [id, tex] : layerOutputTexStorage_)
    {
        if (tex != 0) glDeleteTextures(1, &tex);
    }
    layerOutputFBOs_.clear();
    layerOutputTexStorage_.clear();
    layerOutputTextures_.clear();

    // Release per-layer ring buffers
    for (auto& [id, ring] : layerRingBuffers_)
    {
        for (size_t i = 0; i < ring.fbos.size(); ++i)
        {
            if (ring.fbos[i] != 0) glDeleteFramebuffers(1, &ring.fbos[i]);
            if (ring.textures[i] != 0) glDeleteTextures(1, &ring.textures[i]);
        }
    }
    layerRingBuffers_.clear();

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
    deleteFBO(transitionFBO_, transitionTex_);
    deleteFBO(feedbackFBO_, feedbackTex_);
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    createFBO(effectFBO_A_, effectTex_A_, width, height);
    createFBO(effectFBO_B_, effectTex_B_, width, height);
    createFBO(transitionFBO_, transitionTex_, width, height);
    createFBO(feedbackFBO_, feedbackTex_, width, height);

    // P20: Invalidate layer output FBOs (they'll be recreated at new size)
    for (auto& [id, fbo] : layerOutputFBOs_)
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
    for (auto& [id, tex] : layerOutputTexStorage_)
        if (tex != 0) glDeleteTextures(1, &tex);
    layerOutputFBOs_.clear();
    layerOutputTexStorage_.clear();
    layerOutputTextures_.clear();
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

// === Layer Router Support (P20) ===

void CompositorEngine::ensureLayerOutputFBO(uint32_t layerId, int w, int h)
{
    auto it = layerOutputFBOs_.find(layerId);
    if (it != layerOutputFBOs_.end())
        return; // Already created

    GLuint fbo = 0, tex = 0;
    createFBO(fbo, tex, w, h);
    layerOutputFBOs_[layerId] = fbo;
    layerOutputTexStorage_[layerId] = tex;
    layerOutputTextures_[layerId] = tex;
}

void CompositorEngine::saveLayerOutput(uint32_t layerId, GLuint srcTex,
                                        ShaderManager& shaderMgr, FullscreenQuad& quad,
                                        int w, int h)
{
    ensureLayerOutputFBO(layerId, w, h);

    GLuint fbo = layerOutputFBOs_[layerId];
    auto* prog = shaderMgr.getProgram("passthrough");
    if (!prog) return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    quad.draw();
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

GLuint CompositorEngine::applyClipEffects(const std::vector<Clip::EffectSlot>& effects, GLuint inputTex,
                                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                                            float time, int w, int h,
                                            uint32_t layerId)
{
    if (effects.empty() || effectLibrary_ == nullptr)
        return inputTex;

    // Check if any effect in this chain is temporal (needs u_prev_frame)
    bool anyTemporal = false;
    for (const auto& slot : effects)
    {
        if (!slot.enabled || slot.bypassed) continue;
        const auto* def = effectLibrary_->getEffectDef(juce::String(slot.effectName));
        if (def && def->temporal) { anyTemporal = true; break; }
    }

    GLuint currentInput = inputTex;
    int writeFBO = 0; // 0 = effectFBO_A_, 1 = effectFBO_B_

    for (const auto& slot : effects)
    {
        if (!slot.enabled || slot.bypassed)
            continue;

        // Get the effect definition first to resolve shader name from display name
        const auto* def = effectLibrary_->getEffectDef(juce::String(slot.effectName));
        if (def == nullptr)
            continue;

        // Screen Split is handled specially — uses frame ring buffer, not normal shader
        if (def->shaderName == "screen_split")
        {
            GLuint splitResult = applyScreenSplit(currentInput, slot, shaderMgr, quad, layerId, w, h);
            if (splitResult != 0 && splitResult != currentInput)
            {
                currentInput = splitResult;
                // effectFBO_A_ was used by applyScreenSplit, next write goes to B
                writeFBO = 1;
            }
            continue;
        }

        // Frame Stutter is handled specially — uses frame ring buffer
        if (def->shaderName == "frame_delay")
        {
            float depthParam = (slot.paramValues.size() > 0) ? slot.paramValues[0] : 0.3f;
            float stutterParam = (slot.paramValues.size() > 1) ? slot.paramValues[1] : 0.0f;

            auto& ring = getOrCreateRingBuffer(layerId, w, h);
            pushFrameToRing(ring, currentInput, shaderMgr, quad, w, h);

            // depth: how far back to look. Map [0,1] to [1, 30] frames
            int maxDepth = static_cast<int>(1.0f + depthParam * 29.0f);

            // stutter: 0 = smooth constant delay, 1 = rapid random jumping
            int framesAgo;
            if (stutterParam < 0.01f)
            {
                // Pure delay — show frame from N frames ago
                framesAgo = maxDepth;
            }
            else
            {
                // Stutter: jump between different past frames based on time
                // Higher stutter = more erratic jumping
                float stutterRate = 1.0f + stutterParam * 15.0f; // 1-16 Hz
                float stutterPhase = std::fmod(time * stutterRate, 1.0f);
                // Quantize to create discrete jumps
                int numSteps = static_cast<int>(2.0f + stutterParam * 6.0f); // 2-8 steps
                int step = static_cast<int>(stutterPhase * static_cast<float>(numSteps));
                framesAgo = (step * maxDepth) / numSteps;
            }

            GLuint delayedTex = getFrameFromRing(ring, framesAgo);
            if (delayedTex != 0)
                currentInput = delayedTex;
            continue;
        }

        auto* program = shaderMgr.getProgram(def->shaderName);
        if (program == nullptr)
            continue;

        GLuint targetFBO = (writeFBO == 0) ? effectFBO_A_ : effectFBO_B_;

        glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_BLEND);

        program->use();

        // Bind input texture (unit 0)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentInput);
        auto texLoc = program->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        // Bind temporal previous frame (unit 1) for time effects (u_prev_frame)
        if (def->temporal)
        {
            auto& tempBuf = getOrCreateTemporalBuffer(layerId, w, h);
            auto prevLoc = program->getUniformIDFromName("u_prev_frame");
            if (prevLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tempBuf.tex);
                glUniform1i(prevLoc, 1);
                glActiveTexture(GL_TEXTURE0);
            }
        }

        // Bind feedback texture (unit 1) — previous frame's composited output
        // Only for non-temporal effects that use u_feedbackTex
        if (!def->temporal)
        {
            auto feedbackLoc = program->getUniformIDFromName("u_feedbackTex");
            if (feedbackLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, feedbackTex_);
                glUniform1i(feedbackLoc, 1);
                glActiveTexture(GL_TEXTURE0);
            }
        }

        // Global uniforms
        auto timeLoc = program->getUniformIDFromName("u_time");
        if (timeLoc >= 0)
            glUniform1f(timeLoc, time);
        auto resLoc = program->getUniformIDFromName("u_resolution");
        if (resLoc >= 0)
            glUniform2f(resLoc, static_cast<float>(w), static_cast<float>(h));

        // Audio feature uniforms (P18: audio-reactive effects)
        uploadAudioUniforms(program);

        // Effect parameter uniforms from slot values
        for (size_t p = 0; p < def->params.size() && p < slot.paramValues.size(); ++p)
        {
            auto loc = program->getUniformIDFromName(def->params[p].uniformName.c_str());
            if (loc >= 0)
                glUniform1f(loc, slot.paramValues[p]);
        }

        quad.draw();

        GLuint effectedTex = (writeFBO == 0) ? effectTex_A_ : effectTex_B_;

        // Apply dry/wet blend if < 1.0
        if (slot.dryWet < 0.999f)
        {
            // Blend effected result with pre-effect input
            int blendFBO = 1 - writeFBO;
            GLuint blendTargetFBO = (blendFBO == 0) ? effectFBO_A_ : effectFBO_B_;

            glBindFramebuffer(GL_FRAMEBUFFER, blendTargetFBO);
            glViewport(0, 0, w, h);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_BLEND);

            auto* dwProg = shaderMgr.getProgram("effect_dry_wet");
            if (dwProg)
            {
                dwProg->use();
                // Unit 0 = effected result
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, effectedTex);
                glUniform1i(dwProg->getUniformIDFromName("u_texture"), 0);
                // Unit 1 = original (pre-effect)
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, currentInput);
                glUniform1i(dwProg->getUniformIDFromName("u_original"), 1);
                // Dry/wet amount
                auto dwLoc = dwProg->getUniformIDFromName("u_drywet");
                if (dwLoc >= 0) glUniform1f(dwLoc, slot.dryWet);
                glActiveTexture(GL_TEXTURE0);

                quad.draw();
            }

            currentInput = (blendFBO == 0) ? effectTex_A_ : effectTex_B_;
            writeFBO = 1 - blendFBO;
        }
        else
        {
            currentInput = effectedTex;
            writeFBO = 1 - writeFBO;
        }
    }

    // Save current output as previous frame for temporal effects next frame
    if (anyTemporal && currentInput != inputTex)
    {
        auto& tempBuf = getOrCreateTemporalBuffer(layerId, w, h);
        saveToTemporalBuffer(tempBuf, currentInput, shaderMgr, quad, w, h);
    }

    return currentInput;
}

// === Clip transform ===

GLuint CompositorEngine::applyClipTransform(const Clip& clip, GLuint srcTex,
                                              ShaderManager& shaderMgr, FullscreenQuad& quad,
                                              int w, int h)
{
    constexpr float eps = 0.001f;
    bool needsTransform = (std::abs(clip.positionX) > eps ||
                           std::abs(clip.positionY) > eps ||
                           std::abs(clip.scale - 1.0f) > eps ||
                           std::abs(clip.rotation) > eps);

    GLuint transformedTex = srcTex;

    if (needsTransform)
    {
        auto* prog = shaderMgr.getProgram("layer_transform");
        if (prog != nullptr)
        {
            // Use effectFBO_A_ as scratch (it's not in use yet at this point)
            glBindFramebuffer(GL_FRAMEBUFFER, effectFBO_A_);
            glViewport(0, 0, w, h);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_BLEND);

            prog->use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTex);
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
            // Normalize position: pixels → normalized UV offset
            glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_translate"),
                        clip.positionX / static_cast<float>(w),
                        clip.positionY / static_cast<float>(h));
            glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_anchor"),
                        0.5f + clip.anchorX, 0.5f + clip.anchorY);
            glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_scale"),
                        clip.scale);
            glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_rotation"),
                        clip.rotation * 3.14159265f / 180.0f);

            quad.draw();

            transformedTex = effectTex_A_;
        }
    }

    // S167-L4b: bake clipOpacity in here (rather than only at the final
    // layer-opacity site) so a crossfading clip's OWN opacity survives into
    // applyTransition's blend below -- none of the transition_* shaders in
    // EmbeddedShaders.h (transition_dissolve, wipes, pushes, zoom, iris,
    // flip, cut, fade-to-black) take a per-input opacity uniform, so this is
    // the only place a clip pinned below 1.0 stays capped through a
    // crossfade. Targets effectFBO_B_/effectTex_B_ -- distinct from
    // effectFBO_A_ used above, so this pass never reads and writes the same
    // texture whether or not a positional transform ran first.
    return applyClipOpacity(clip.clipOpacity, transformedTex, effectFBO_B_, effectTex_B_,
                            shaderMgr, quad, w, h);
}

GLuint CompositorEngine::applyClipOpacity(float opacity, GLuint srcTex, GLuint dstFBO, GLuint dstTex,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                                           int w, int h)
{
    constexpr float eps = 0.001f;
    if (std::abs(opacity - 1.0f) <= eps)
        return srcTex; // true no-op -- no GL call issued

    // fix-needed(s167): this used to bake opacity into ALPHA via the shared
    // "opacity_blend" program (col.a *= u_opacity), on the theory that the
    // final layer-composite would consume that alpha. Measured on the live
    // app it does not, reliably: blendLayerOntoAccumulator's Multiply/
    // Screen/Darken/Lighten blend modes never reference GL_SRC_ALPHA for the
    // src operand at all (alpha is a complete no-op under those), and a
    // layer left at the Opaque type with layer.opacity at its 1.0 default
    // disables GL_BLEND outright (CompositorEngine.cpp's compositeDeck,
    // Opaque branch) -- a straight overwrite that never reads alpha either.
    // "clip_opacity_blend" scales RGB directly instead (dims toward black),
    // the same technique masterOpacity and layer.opacity's own Opaque-path
    // multiply already rely on -- it survives regardless of which blend
    // mode or layer type ends up consuming this texture, and composes
    // correctly with layer.opacity's alpha-consuming Normal/Additive blend
    // (leaving alpha untouched here means that later stage still applies
    // its own factor on top, giving master*layer*clip rather than
    // double-counting clip's contribution).
    auto* prog = shaderMgr.getProgram("clip_opacity_blend");
    if (prog == nullptr)
        return srcTex;

    glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_opacity"), opacity);

    quad.draw();

    return dstTex;
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

void CompositorEngine::applyFXOnlyLayer(const Clip& clip, const Layer& layer,
                                          ShaderManager& shaderMgr,
                                          FullscreenQuad& quad, float time, int w, int h)
{
    if (clip.effects.empty() || effectLibrary_ == nullptr)
        return;

    // Apply the clip's effects to the accumulator texture
    GLuint result = applyClipEffects(clip.effects, accumulatorTex_, shaderMgr, quad, time, w, h, layer.id);

    // S167-L4b: fold the clip's own opacity in here, the FX-Only layer's
    // constant-alpha blend -- owner's ruling is that master/layer/clip
    // opacity multiply (see combinedOpacity()'s comment), so a clip pinned
    // below 1.0 dilutes the FX-Only blend even further than layer.opacity
    // alone would.
    float effOpacity = combinedOpacity(layer.opacity, clip.clipOpacity);

    if (result != accumulatorTex_)
    {
        // If opacity < 1, blend between original accumulator and FX'd result
        // First, save the original accumulator to scratch
        if (effOpacity < 0.999f)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, scratchFBO_);
            glViewport(0, 0, w, h);
            glDisable(GL_BLEND);
            auto* pt = shaderMgr.getProgram("passthrough");
            if (pt) {
                pt->use();
                glUniform1i(glGetUniformLocation(pt->getProgramID(), "u_texture"), 0);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, accumulatorTex_);
            quad.draw();
        }

        // Copy FX'd result to accumulator
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

        // Blend original back if opacity < 1
        if (effOpacity < 0.999f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
            glBlendColor(0.0f, 0.0f, 0.0f, 1.0f - effOpacity);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, scratchTex_);
            quad.draw();
            glDisable(GL_BLEND);
        }
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
                                        float time, float dt,
                                        int width, int height)
{
    using namespace juce::gl;

    hasActiveLayers_ = false;

    // Solo: if any layer in the deck is soloed, only soloed layers render.
    // visible/bypassed are checked first below (same `||` skip condition) so
    // they still gate as before — solo narrows the remaining set further, it
    // does not un-hide a hidden layer or un-bypass a bypassed one.
    bool anySolo = false;
    for (const auto& layer : deck.layers)
    {
        if (layer.solo) { anySolo = true; break; }
    }

    // Check if any layer has an active clip with content
    for (const auto& layer : deck.layers)
    {
        if (!layer.visible || layer.bypassed || (anySolo && !layer.solo)) continue;
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
        // Layers with effects (even without media) are active — FX applies to accumulator
        if (!clip->effects.empty())
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
        if (!layer.visible || layer.bypassed || (anySolo && !layer.solo))
            continue;

        // P14: Advance crossfade progress each frame. S167-L4b DT-FIX:
        // `speed` here is actually a DURATION in seconds (transitionSpeed
        // is a misleading name inherited from the model -- see its slider
        // wiring in LayerInspector.cpp/LayerStrip.cpp, both duration-in-
        // seconds UI), so step = dt / duration is the frame-rate-
        // independent progress increment: cumulative progress after real
        // elapsed time T is T / duration, completing exactly at T ==
        // duration regardless of callback rate. Real dt (function param),
        // not a hardcoded 1/60 -- see compositeDeck()'s header comment.
        // Not gated on timeOverride_/`time`, same reasoning as
        // lastFrameTimestampMs_'s comment in Renderer.h: crossfadeProgress
        // is persistent per-layer state (like previousClipColumn) that
        // already advances every real GL callback regardless of
        // deterministic test-capture mode, so there is no
        // render_frame byte-identical-repeat contract covering it to
        // preserve here either -- only the rate was wrong.
        if (layer.crossfadeProgress < 1.0f && layer.previousClipColumn >= 0)
        {
            float speed = layer.transitionSpeed;
            if (speed <= 0.0f) speed = 0.5f; // default transition duration in seconds
            float step = dt / speed;
            layer.crossfadeProgress = std::min(layer.crossfadeProgress + step, 1.0f);
            if (layer.crossfadeProgress >= 1.0f)
                layer.previousClipColumn = -1; // transition complete
        }

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
                    // S167-L4b DT-FIX: real measured dt (function param), not
                    // a hardcoded 1/60 -- see compositeDeck()'s header comment.
                    clipTex = videoFrameFn_(clip, dt);
                }

                // If no media but clip has effects, apply as FX-only (affects layers below)
                if (clipTex == 0)
                {
                    if (clip->hasEffects())
                        applyFXOnlyLayer(*clip, layer, shaderMgr, quad, time, width, height);
                    continue;
                }

                // Apply per-clip transform (position, scale, rotation)
                clipTex = applyClipTransform(*clip, clipTex, shaderMgr, quad, width, height);

                // P13.5.1: Apply per-clip effects
                clipTex = applyClipEffects(clip->effects, clipTex, shaderMgr, quad, time, width, height, layer.id);

                // P14: Apply clip-to-clip transition if crossfading. S167-L4b
                // DT-FIX: real measured dt (function param) -- see
                // compositeDeck()'s header comment.
                clipTex = applyTransition(layer, clipTex, time, shaderMgr, quad, width, height, dt);

                // P16: Apply feedback (Larsen loop) if enabled
                if (layer.feedback.enabled && layer.feedback.amount > 0.001f)
                {
                    auto& fbProc = getOrCreateFeedbackProcessor(layer.id);
                    clipTex = fbProc.process(clipTex, layer.feedback, shaderMgr, quad, width, height);
                }

                // Apply per-layer effects (same mechanism as per-clip effects)
                if (!layer.layerEffects.empty())
                {
                    clipTex = applyClipEffects(layer.layerEffects, clipTex, shaderMgr, quad, time, width, height, layer.id);
                }

                // P13.5.5: Apply layer transform
                clipTex = applyLayerTransform(layer, clipTex, shaderMgr, quad, width, height);

                // P20: Save layer output for Layer Router sources
                saveLayerOutput(layer.id, clipTex, shaderMgr, quad, width, height);

                if (layer.type == Layer::Type::Transparent)
                {
                    // Apply keying → scratch FBO
                    applyLayerKeying(layer, clipTex, scratchFBO_, shaderMgr, quad, width, height);
                    // Blend scratch onto accumulator
                    blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
                }
                else
                {
                    // Opaque: clear accumulator and draw with opacity
                    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    if (layer.opacity < 0.999f)
                    {
                        // Use alpha blending to apply opacity over black
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                    else
                    {
                        glDisable(GL_BLEND);
                    }

                    auto* prog = shaderMgr.getProgram("opacity_blend");
                    if (!prog) prog = shaderMgr.getProgram("passthrough");
                    if (prog)
                    {
                        prog->use();
                        glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
                        auto opLoc = glGetUniformLocation(prog->getProgramID(), "u_opacity");
                        if (opLoc >= 0)
                            glUniform1f(opLoc, layer.opacity);
                    }
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, clipTex);
                    quad.draw();
                    glDisable(GL_BLEND);
                }
                break;
            }
            case Layer::Type::FXOnly:
            {
                // P13.5.2: Apply clip's effects to the accumulator (with opacity)
                applyFXOnlyLayer(*clip, layer, shaderMgr, quad, time, width, height);
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
                    // S167-L4b DT-FIX: real measured dt (function param), not
                    // a hardcoded 1/60 -- see compositeDeck()'s header comment.
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

void CompositorEngine::compositePersistentLayers(Deck& deck,
                                                  ShaderManager& shaderMgr,
                                                  FullscreenQuad& quad,
                                                  float time, float dt,
                                                  int width, int height)
{
    using namespace juce::gl;

    if (!glInitialized_) return;

    // Solo: same read class as visible/bypassed (see compositeDeck() above).
    // Scanned per-deck — solo is a per-Layer field, not deck-scoped, so this
    // deck's own solo state must be evaluated independently of whichever deck
    // is currently active.
    bool anySolo = false;
    for (const auto& layer : deck.layers)
    {
        if (layer.solo) { anySolo = true; break; }
    }

    // Iterate layers, compositing only those marked persistent and with active clips
    for (auto& layer : deck.layers)
    {
        if (!layer.persistent || !layer.visible || layer.bypassed || (anySolo && !layer.solo))
            continue;

        const Clip* clip = layer.getActiveClip();
        if (clip == nullptr)
            continue;

        // Only composite Opaque/Transparent persistent layers for now
        if (layer.type != Layer::Type::Opaque && layer.type != Layer::Type::Transparent)
            continue;

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
            // S167-L4b DT-FIX: real measured dt (function param), not a
            // hardcoded 1/60 -- see compositePersistentLayers()'s header
            // comment (CompositorEngine.h).
            clipTex = videoFrameFn_(clip, dt);
        }

        if (clipTex == 0) continue;

        // Apply clip effects
        GLuint processedTex = applyClipEffects(clip->effects, clipTex,
                                                shaderMgr, quad, time, width, height,
                                                layer.id);
        if (processedTex == 0) processedTex = clipTex;

        // Keying for transparent layers
        if (layer.type == Layer::Type::Transparent)
        {
            applyLayerKeying(layer, processedTex, scratchFBO_, shaderMgr, quad, width, height);
            blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
        }
        else
        {
            blendLayerOntoAccumulator(layer, processedTex, shaderMgr, quad, width, height);
        }
    }
}

GLuint CompositorEngine::applyGlobalEffects(const std::vector<Clip::EffectSlot>& globalEffects,
                                             GLuint inputTex,
                                             ShaderManager& shaderMgr, FullscreenQuad& quad,
                                             float time, int w, int h)
{
    if (globalEffects.empty())
        return inputTex;                // true no-op — matches applyClipEffects' own
                                        // early-return; no GL call issued either way

    // kGlobalEffectsLayerId keeps this call's temporal buffer / screen-split
    // ring buffer from aliasing a real layer's.
    return applyClipEffects(globalEffects, inputTex, shaderMgr, quad, time, w, h,
                            kGlobalEffectsLayerId);
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

// === Phase 14: Clip texture helper ===

GLuint CompositorEngine::getClipTexture(const Clip& clip, float time, int w, int h, float dt)
{
    if (clip.mediaType == Clip::MediaType::Image && clip.mediaFile.existsAsFile())
        return getKeyTexture(clip.mediaFile);

    if (clip.mediaType == Clip::MediaType::Source && !clip.sourceType.empty() && sourceRenderFn_)
    {
        const auto* params = clip.sourceParams.empty() ? nullptr : &clip.sourceParams;
        return sourceRenderFn_(clip.sourceType, time, w, h, params);
    }

    if ((clip.mediaType == Clip::MediaType::Video ||
         clip.mediaType == Clip::MediaType::ImageSequence) && videoFrameFn_)
        return videoFrameFn_(&clip, dt);

    return 0;
}

// === Phase 14: Transition shader name mapping ===

juce::String CompositorEngine::getTransitionShaderName(Layer::MixMode mode)
{
    switch (mode)
    {
        case Layer::MixMode::WipeLeft:    return "transition_wipe_left";
        case Layer::MixMode::WipeRight:   return "transition_wipe_right";
        case Layer::MixMode::WipeUp:      return "transition_wipe_up";
        case Layer::MixMode::WipeDown:    return "transition_wipe_down";
        case Layer::MixMode::PushLeft:    return "transition_push_left";
        case Layer::MixMode::PushRight:   return "transition_push_right";
        case Layer::MixMode::PushUp:      return "transition_push_up";
        case Layer::MixMode::PushDown:    return "transition_push_down";
        case Layer::MixMode::ZoomIn:      return "transition_zoom_in";
        case Layer::MixMode::ZoomOut:     return "transition_zoom_out";
        case Layer::MixMode::WipeEllipse: return "transition_iris";
        case Layer::MixMode::Flip:        return "transition_flip_h";
        case Layer::MixMode::Cut:         return "transition_cut";
        case Layer::MixMode::ToBlack:     return "transition_fade_black";
        case Layer::MixMode::Dissolve:
        default:                          return "transition_dissolve";
    }
}

// === Phase 14: Transition rendering ===

GLuint CompositorEngine::applyTransition(Layer& layer, GLuint newClipTex, float time,
                                          ShaderManager& shaderMgr, FullscreenQuad& quad,
                                          int w, int h, float dt)
{
    using namespace juce::gl;

    // If crossfade is complete or no previous clip, just return the new texture
    if (layer.crossfadeProgress >= 1.0f || layer.previousClipColumn < 0)
        return newClipTex;

    // Get previous clip texture
    Clip* prevClip = layer.getClipAt(layer.previousClipColumn);
    if (prevClip == nullptr)
        return newClipTex;

    GLuint prevTex = getClipTexture(*prevClip, time, w, h, dt);
    if (prevTex == 0)
        return newClipTex;

    // Apply previous clip's effects too
    prevTex = applyClipEffects(prevClip->effects, prevTex, shaderMgr, quad, time, w, h, layer.id);

    // S167-L4b: the outgoing clip keeps its OWN opacity through the
    // crossfade too, not just the incoming one (baked above in
    // applyClipTransform) -- same reasoning, see that function's comment.
    // scratchFBO_/scratchTex_ aren't touched again until after this
    // function returns (feedback/layer-effects/layer-transform/keying all
    // come later in compositeDeck's per-layer sequence), so they're free
    // here as scratch.
    prevTex = applyClipOpacity(prevClip->clipOpacity, prevTex, scratchFBO_, scratchTex_,
                               shaderMgr, quad, w, h);

    // Render transition into dedicated transitionFBO (avoids conflicting with scratch/keying)
    juce::String shaderName = getTransitionShaderName(layer.transitionMode);
    auto* prog = shaderMgr.getProgram(shaderName);
    if (prog == nullptr)
        prog = shaderMgr.getProgram("transition_dissolve"); // fallback

    if (prog == nullptr)
        return newClipTex;

    glBindFramebuffer(GL_FRAMEBUFFER, transitionFBO_);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    prog->use();
    GLuint pid = prog->getProgramID();

    // Bind new clip to texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, newClipTex);
    glUniform1i(glGetUniformLocation(pid, "u_texture"), 0);

    // Bind previous clip to texture unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, prevTex);
    glUniform1i(glGetUniformLocation(pid, "u_prevTexture"), 1);

    // Set crossfade progress
    glUniform1f(glGetUniformLocation(pid, "u_crossfadeProgress"), layer.crossfadeProgress);

    quad.draw();

    // Reset active texture
    glActiveTexture(GL_TEXTURE0);

    return transitionTex_;
}

// === Feedback Buffer ===

void CompositorEngine::updateFeedbackBuffer(ShaderManager& shaderMgr, FullscreenQuad& quad,
                                              int w, int h)
{
    if (!glInitialized_ || feedbackFBO_ == 0)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, feedbackFBO_);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);

    auto* prog = shaderMgr.getProgram("passthrough");
    if (prog)
    {
        prog->use();
        glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accumulatorTex_);
    quad.draw();

    feedbackReady_ = true;
}

FeedbackProcessor& CompositorEngine::getOrCreateFeedbackProcessor(uint32_t layerId)
{
    auto it = feedbackProcessors_.find(layerId);
    if (it != feedbackProcessors_.end())
        return *it->second;

    auto proc = std::make_unique<FeedbackProcessor>();
    auto& ref = *proc;
    feedbackProcessors_[layerId] = std::move(proc);
    return ref;
}

CompositorEngine::TemporalBuffer& CompositorEngine::getOrCreateTemporalBuffer(uint32_t layerId, int w, int h)
{
    auto& buf = layerTemporalBuffers_[layerId];
    if (buf.tex != 0 && buf.width == w && buf.height == h)
        return buf;

    // Release old if size changed
    if (buf.fbo != 0) { glDeleteFramebuffers(1, &buf.fbo); buf.fbo = 0; }
    if (buf.tex != 0) { glDeleteTextures(1, &buf.tex); buf.tex = 0; }

    buf.width = w;
    buf.height = h;
    createFBO(buf.fbo, buf.tex, w, h);

    // Clear to black so first frame's u_prev_frame is black
    glBindFramebuffer(GL_FRAMEBUFFER, buf.fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return buf;
}

void CompositorEngine::saveToTemporalBuffer(TemporalBuffer& buf, GLuint srcTex,
                                             ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h)
{
    auto* prog = shaderMgr.getProgram("passthrough");
    if (!prog || buf.fbo == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, buf.fbo);
    glViewport(0, 0, w, h);
    glDisable(GL_BLEND);
    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// === Frame Ring Buffer for Screen Split ===

CompositorEngine::FrameRingBuffer& CompositorEngine::getOrCreateRingBuffer(uint32_t layerId, int w, int h)
{
    int rw = std::max(1, w / kRingDownscale);
    int rh = std::max(1, h / kRingDownscale);

    auto& ring = layerRingBuffers_[layerId];
    if (ring.initialized && ring.ringWidth == rw && ring.ringHeight == rh)
        return ring;

    // Release old
    if (ring.initialized)
    {
        for (size_t i = 0; i < ring.fbos.size(); ++i)
        {
            if (ring.fbos[i] != 0) glDeleteFramebuffers(1, &ring.fbos[i]);
            if (ring.textures[i] != 0) glDeleteTextures(1, &ring.textures[i]);
        }
    }

    ring.ringWidth = rw;
    ring.ringHeight = rh;
    ring.writeIndex = 0;
    ring.frameCount = 0;
    ring.fbos.resize(static_cast<size_t>(kMaxRingFrames), 0);
    ring.textures.resize(static_cast<size_t>(kMaxRingFrames), 0);

    for (int i = 0; i < kMaxRingFrames; ++i)
    {
        createFBO(ring.fbos[static_cast<size_t>(i)],
                  ring.textures[static_cast<size_t>(i)], rw, rh);
        glBindFramebuffer(GL_FRAMEBUFFER, ring.fbos[static_cast<size_t>(i)]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ring.initialized = true;
    return ring;
}

void CompositorEngine::pushFrameToRing(FrameRingBuffer& ring, GLuint srcTex,
                                        ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h)
{
    (void)w; (void)h; // full-res dimensions not used — we render at ring resolution
    auto* prog = shaderMgr.getProgram("passthrough");
    if (!prog) return;

    auto idx = static_cast<size_t>(ring.writeIndex);
    glBindFramebuffer(GL_FRAMEBUFFER, ring.fbos[idx]);
    glViewport(0, 0, ring.ringWidth, ring.ringHeight);
    glDisable(GL_BLEND);
    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ring.writeIndex = (ring.writeIndex + 1) % kMaxRingFrames;
    if (ring.frameCount < kMaxRingFrames)
        ring.frameCount++;
}

GLuint CompositorEngine::getFrameFromRing(const FrameRingBuffer& ring, int framesAgo) const
{
    if (!ring.initialized || ring.frameCount == 0)
        return 0;

    int maxDelay = ring.frameCount - 1;
    if (framesAgo > maxDelay) framesAgo = maxDelay;
    if (framesAgo < 0) framesAgo = 0;

    int idx = (ring.writeIndex - 1 - framesAgo + kMaxRingFrames * 2) % kMaxRingFrames;
    return ring.textures[static_cast<size_t>(idx)];
}

// === Screen Split: render grid with per-cell delay ===

GLuint CompositorEngine::applyScreenSplit(GLuint clipTex, const Clip::EffectSlot& slot,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                                           uint32_t layerId, int w, int h)
{
    if (effectLibrary_ == nullptr) return clipTex;

    const auto* def = effectLibrary_->getEffectDef("Screen Split");
    if (def == nullptr) return clipTex;

    // Read params: columns, rows, delay, mode
    float colsParam = (slot.paramValues.size() > 0) ? slot.paramValues[0] : 0.15f;
    float rowsParam = (slot.paramValues.size() > 1) ? slot.paramValues[1] : 0.15f;
    float delayParam = (slot.paramValues.size() > 2) ? slot.paramValues[2] : 0.5f;
    float modeParam = (slot.paramValues.size() > 3) ? slot.paramValues[3] : 0.0f;

    int cols = static_cast<int>(2.0f + colsParam * 6.0f); // 2-8
    int rows = static_cast<int>(2.0f + rowsParam * 6.0f); // 2-8
    // delay: frames between cells. Slider [0,1] → [0, 60] frames per cell.
    // At 1.0 with a 2x2 grid (4 cells), total span = 180 frames = 3 seconds at 60fps.
    int framesPerCell = static_cast<int>(std::round(60.0f * delayParam));

    // Get or create ring buffer, push current frame
    auto& ring = getOrCreateRingBuffer(layerId, w, h);
    pushFrameToRing(ring, clipTex, shaderMgr, quad, w, h);

    // Render the grid into effectFBO_A_
    glBindFramebuffer(GL_FRAMEBUFFER, effectFBO_A_);
    glViewport(0, 0, w, h);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // dark gray background (grid lines)
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    auto* prog = shaderMgr.getProgram("passthrough");
    if (!prog) return clipTex;

    float cellW = static_cast<float>(w) / static_cast<float>(cols);
    float cellH = static_cast<float>(h) / static_cast<float>(rows);
    float border = 2.0f; // pixel border

    int totalCells = cols * rows;

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            // Calculate cell index based on mode
            int cellIndex;
            if (modeParam < 0.33f)
            {
                // Sequential: left-to-right, top-to-bottom
                cellIndex = r * cols + c;
            }
            else if (modeParam < 0.66f)
            {
                // Reverse: bottom-right to top-left
                cellIndex = (rows - 1 - r) * cols + (cols - 1 - c);
            }
            else
            {
                // Diagonal
                cellIndex = r + c;
            }

            int framesAgo = cellIndex * framesPerCell;

            // Get the texture for this cell's delay
            GLuint cellTex = getFrameFromRing(ring, framesAgo);
            if (cellTex == 0) cellTex = clipTex; // fallback to current

            // Calculate viewport for this cell (with border inset)
            float x = static_cast<float>(c) * cellW + border;
            float y = static_cast<float>(rows - 1 - r) * cellH + border; // GL coords: bottom-up
            float cw = cellW - border * 2.0f;
            float ch = cellH - border * 2.0f;

            glViewport(static_cast<GLint>(x), static_cast<GLint>(y),
                       static_cast<GLsizei>(cw), static_cast<GLsizei>(ch));

            prog->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cellTex);
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
            quad.draw();
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return effectTex_A_;
}

void CompositorEngine::uploadAudioUniforms(juce::OpenGLShaderProgram* program) const
{
    if (!program) return;

    auto loc = [&](const char* name) {
        return program->getUniformIDFromName(name);
    };

    const auto& snap = latestSnapshot_;

    // Basic audio features
    auto l = loc("u_rms");      if (l >= 0) glUniform1f(l, snap.rms);
    l = loc("u_bass");           if (l >= 0) glUniform1f(l, snap.bandEnergies[1]);
    l = loc("u_mid");            if (l >= 0) glUniform1f(l, snap.bandEnergies[3]);
    l = loc("u_high");           if (l >= 0) glUniform1f(l, snap.bandEnergies[5]);
    l = loc("u_beatPhase");      if (l >= 0) glUniform1f(l, snap.beatPhase);
    l = loc("u_barPhase");       if (l >= 0) glUniform1f(l, snap.barPhase);
    l = loc("u_phrasePhase");    if (l >= 0) glUniform1f(l, snap.phrasePhase);
    l = loc("u_spectralCentroid"); if (l >= 0) glUniform1f(l, snap.spectralCentroid);
    l = loc("u_spectralFlux");   if (l >= 0) glUniform1f(l, snap.spectralFlux);
    l = loc("u_onsetStrength");  if (l >= 0) glUniform1f(l, snap.onsetStrength);
    l = loc("u_onsetDetected");  if (l >= 0) glUniform1f(l, snap.onsetDetected ? 1.0f : 0.0f);
    l = loc("u_dominantPitch");  if (l >= 0) glUniform1f(l, snap.dominantPitch);
    l = loc("u_pitchConfidence"); if (l >= 0) glUniform1f(l, snap.pitchConfidence);
    l = loc("u_detectedKey");    if (l >= 0) glUniform1f(l, static_cast<float>(snap.detectedKey));
    l = loc("u_keyIsMajor");     if (l >= 0) glUniform1f(l, snap.keyIsMajor ? 1.0f : 0.0f);
    l = loc("u_structuralState"); if (l >= 0) glUniform1f(l, static_cast<float>(snap.structuralState));
    l = loc("u_bpm");            if (l >= 0) glUniform1f(l, snap.bpm);
    l = loc("u_hcdf");           if (l >= 0) glUniform1f(l, snap.harmonicChangeDetection);

    // 7-band energies as array
    l = loc("u_bandEnergies");
    if (l >= 0) glUniform1fv(l, 7, snap.bandEnergies);

    // 12-note chromagram as array
    l = loc("u_chromagram");
    if (l >= 0) glUniform1fv(l, 12, snap.chromagram);

    // 13 MFCCs as array
    l = loc("u_mfccs");
    if (l >= 0) glUniform1fv(l, 13, snap.mfccs);

    // P23: Genre detection uniforms
    l = loc("u_genre");
    if (l >= 0) glUniform1f(l, static_cast<float>(snap.detectedGenre));
    l = loc("u_genreConfidence");
    if (l >= 0) glUniform1f(l, snap.genreConfidence);
    l = loc("u_energyState");
    if (l >= 0) glUniform1f(l, static_cast<float>(snap.energyState));

    // P25: Advanced audio analysis uniforms
    l = loc("u_sidechainPump");
    if (l >= 0) glUniform1f(l, snap.sidechainPump);
    l = loc("u_swingRatio");
    if (l >= 0) glUniform1f(l, snap.swingRatio);
    l = loc("u_formantPresence");
    if (l >= 0) glUniform1f(l, snap.formantPresence);
    l = loc("u_resonancePeak");
    if (l >= 0) glUniform1f(l, snap.resonancePeak);
    l = loc("u_reeseBass");
    if (l >= 0) glUniform1f(l, snap.reeseBass);
}
