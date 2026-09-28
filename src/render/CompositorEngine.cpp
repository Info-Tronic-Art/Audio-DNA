#include "CompositorEngine.h"
#include "render/EmbeddedShaders.h"
#include "render/ScratchPool.h"
#include "render/RenderGeometry.h"
#include "render/FrameRing.h"
#include "render/LayerClock.h"
#include "render/PixelConvert.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>

using namespace juce::gl;

void CompositorEngine::initGL(int width, int height)
{
    fboWidth_ = width;
    fboHeight_ = height;
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    createFBO(effectFBO_A_, effectTex_A_, width, height);
    createFBO(effectFBO_B_, effectTex_B_, width, height);
    createFBO(effectFBO_C_, effectTex_C_, width, height);
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
    deleteFBO(effectFBO_C_, effectTex_C_);
    deleteFBO(transitionFBO_, transitionTex_);
    deleteFBO(feedbackFBO_, feedbackTex_);

    // s-rta-0928 R1.2: every image texture goes with the context; late results for the cleared entries Drop and the
    // next frame's lookup requests them again.
    for (GLuint tex : imageCache_.clearAll())
        if (tex != 0)
            glDeleteTextures(1, &tex);
    readyImages_.clear();
    lastMaskImageTex_.clear();
    layerOutputOwner_.clear();
    imagesPending_.store(0, std::memory_order_relaxed);
    imageTexCount_.store(0, std::memory_order_relaxed);
    imageTexBytes_.store(0, std::memory_order_relaxed);

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
    temporalBufferCount_.store(0, std::memory_order_relaxed);

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
    frameRingCount_.store(0, std::memory_order_relaxed);
    frameRingCellCount_.store(0, std::memory_order_relaxed);
    crossfadeStart_.clear();

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
    deleteFBO(effectFBO_C_, effectTex_C_);
    deleteFBO(transitionFBO_, transitionTex_);
    deleteFBO(feedbackFBO_, feedbackTex_);
    createFBO(accumulatorFBO_, accumulatorTex_, width, height);
    createFBO(scratchFBO_, scratchTex_, width, height);
    createFBO(effectFBO_A_, effectTex_A_, width, height);
    createFBO(effectFBO_B_, effectTex_B_, width, height);
    createFBO(effectFBO_C_, effectTex_C_, width, height);
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
    layerOutputOwner_.clear();   // s-rta-0928 R1.2: nothing to hold after a resize

    // s-rta-0926b plan4 item 1 (1C): a canvas-size change keeps every picture history.
    rescaleHistory(width, height);
}

void CompositorEngine::rescaleHistory(int width, int height)
{
    // Runs only from resize(), i.e. before any pass of the frame (R5-safe by construction). Temporal
    // buffers (u_prev_frame, incl. the crossfade outgoing slots) and feedback ping-pongs are rescale-
    // blitted -- recreating them black made Freeze / Echo / feedback pictures drop out for a hold
    // interval. Frame rings are dropped and recreated lazily at the new size by getOrCreateRingBuffer
    // (Screen Split / Frame Stutter cells fall back to the frames available, as on a fresh layer).
    for (auto& [key, buf] : layerTemporalBuffers_)
    {
        if (buf.tex == 0)
            continue;
        GLuint fbo = 0, tex = 0;
        createFBO(fbo, tex, width, height);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, buf.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(0, 0, buf.width, buf.height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glDeleteFramebuffers(1, &buf.fbo);
        glDeleteTextures(1, &buf.tex);
        buf.fbo = fbo;
        buf.tex = tex;
        buf.width = width;
        buf.height = height;   // temporalBufferCount_ unchanged: one buffer replaced by one
    }

    for (auto& [key, proc] : feedbackProcessors_)
        if (proc != nullptr)
            proc->resizePreserving(width, height);

    for (auto& [key, ring] : layerRingBuffers_)
    {
        if (!ring.initialized)
            continue;
        for (size_t i = 0; i < ring.fbos.size(); ++i)
        {
            if (ring.fbos[i] != 0) glDeleteFramebuffers(1, &ring.fbos[i]);
            if (ring.textures[i] != 0) glDeleteTextures(1, &ring.textures[i]);
        }
        frameRingCellCount_.fetch_sub(ring.allocatedCells, std::memory_order_relaxed);
        ring = FrameRingBuffer{};
        frameRingCount_.fetch_sub(1, std::memory_order_relaxed);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

// s-rta-0926 xfade: the single rule for the effect scratch pool (ScratchPool.h).
// Never returns readTex or holdTex; with three members at most two are
// excluded, so the pick always succeeds (index 0 is a defensive fallback only).
CompositorEngine::ScratchTarget CompositorEngine::pickEffectTarget(GLuint readTex, GLuint holdTex) const
{
    const GLuint texs[3] = { effectTex_A_, effectTex_B_, effectTex_C_ };
    const GLuint fbos[3] = { effectFBO_A_, effectFBO_B_, effectFBO_C_ };
    const int i = ScratchPool::pick(texs, readTex, holdTex);
    return (i >= 0) ? ScratchTarget{ fbos[i], texs[i] } : ScratchTarget{ fbos[0], texs[0] };
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

void CompositorEngine::saveLayerOutput(uint32_t layerId, uint32_t deckId, GLuint srcTex,
                                        ShaderManager& shaderMgr, FullscreenQuad& quad,
                                        int w, int h)
{
    ensureLayerOutputFBO(layerId, w, h);
    layerOutputOwner_[layerId] = LayerOutputOwner{ deckId, frameSerial_ };   // s-rta-0928 R1.2 (the hold)

    // s-rta-0926 xfade class sweep: a Layer Router clip routed to its OWN
    // layer hands back this layer's saved output as srcTex. The texture
    // already holds exactly that content, and drawing it into its own FBO
    // would sample the texture being rendered to (GL feedback loop, UB).
    if (srcTex == layerOutputTexStorage_[layerId])
        return;

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

// === s-rta-0928 R1.2: clip images decoded off the GL thread ===

GLuint CompositorEngine::getKeyTexture(const juce::File& imageFile, bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    if (decoder_ == nullptr)
    {
        jassertfalse;   // setImageDecoder was never called: nothing can decode
        return 0;
    }
    const auto path = imageFile.getFullPathName().toStdString();
    const auto l = imageCache_.lookup(path);
    if (l.request)
        decoder_->request(imageFile, ImageDecode::Layout::StraightRGBA, 0, std::nullopt, imageBox_);
    if (l.pending)
    {
        ++pendingImagesThisFrame_;
        if (pending != nullptr)
            *pending = true;
        return 0;
    }
    return l.tex;
}

// The GL calls of the old loadKeyImage, exactly (the bytes are the old loop's: tests/test_image_decode.cpp).
GLuint CompositorEngine::uploadImageTexture(const ImageDecode::Result& r)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, r.rgba.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

void CompositorEngine::deleteImageTexture(GLuint tex)
{
    if (tex == 0)
        return;
    glDeleteTextures(1, &tex);
    for (auto& [key, t] : lastMaskImageTex_)
        if (t == tex)
            t = 0;
}

void CompositorEngine::pumpImages()
{
    if (!glInitialized_ || decoder_ == nullptr || uploadBudget_ == nullptr)
        return;
    imagePumpFrames_.fetch_add(1, std::memory_order_relaxed);

    // R1.5: a composition swap posted its image set -- release the images it no longer has (today they were never
    // released), queue the rest for prefetch / re-validation.
    {
        std::unique_lock<std::mutex> lock(imageSetMutex_, std::try_to_lock);
        if (lock.owns_lock() && hasPostedImageSet_)
        {
            std::vector<std::string> set = std::move(postedImageSet_);
            hasPostedImageSet_ = false;
            lock.unlock();
            for (GLuint tex : imageCache_.applyImageSet(set))
                deleteImageTexture(tex);
        }
    }

    drained_.clear();
    if (imageBox_->tryDrain(drained_))
        for (auto& r : drained_)
            readyImages_.push_back(ReadyImage{ std::move(r), false });

    if (!readyImages_.empty())
    {
        // Demand (a frame is waiting for it) before prefetch; stable, so arrival order holds within each class.
        std::stable_partition(readyImages_.begin(), readyImages_.end(),
                              [this](const ReadyImage& ri) { return imageCache_.isDemand(ri.r.path); });
        std::vector<ReadyImage> keep;
        for (auto& ri : readyImages_)
        {
            if (!ri.accepted)
            {
                const auto act = imageCache_.onResult(ri.r.path, ri.r.kind, ri.r.stamp);
                if (act != ImageTexCache::Act::Upload)
                    continue;   // Drop / MarkFailed: no GL work
                ri.accepted = true;
            }
            else if (!imageCache_.awaitingUpload(ri.r.path))
                continue;       // dropped meanwhile (context loss / eviction)
            const size_t bytes = ri.r.rgba.size();
            if (!uploadBudget_->take(bytes))
            {
                keep.push_back(std::move(ri));   // B3: the bytes wait for a later frame, never re-decoded
                continue;
            }
            const auto t0 = std::chrono::steady_clock::now();
            const GLuint tex = uploadImageTexture(ri.r);
            const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
            if (ms > peakImageUploadMs_.load(std::memory_order_relaxed))
                peakImageUploadMs_.store(ms, std::memory_order_relaxed);
            deleteImageTexture(imageCache_.onUploaded(ri.r.path, tex, ri.r.w, ri.r.h, ri.r.stamp, bytes));
        }
        readyImages_.swap(keep);
    }

    // R1.5: one prefetch job in flight (the other decoder threads stay free for a frame's demand).
    if (auto job = imageCache_.nextPrefetch(kImagePrefetchBudgetBytes))
        decoder_->request(juce::File(juce::String(job->path)), ImageDecode::Layout::StraightRGBA, 0, job->known,
                          imageBox_);

    imagesPending_.store(imageCache_.pendingCount(), std::memory_order_relaxed);
    imageTexCount_.store(imageCache_.residentCount(), std::memory_order_relaxed);
    imageTexBytes_.store(static_cast<int64_t>(imageCache_.residentBytes()), std::memory_order_relaxed);
}

GLuint CompositorEngine::heldLayerOutput(uint32_t layerId, uint32_t deckId) const
{
    const auto o = layerOutputOwner_.find(layerId);
    if (o == layerOutputOwner_.end() || o->second.deckId != deckId || o->second.frame + 1 != frameSerial_)
        return 0;
    const auto t = layerOutputTexStorage_.find(layerId);
    return t != layerOutputTexStorage_.end() ? t->second : 0;
}

void CompositorEngine::touchLayerOutput(uint32_t layerId, uint32_t deckId)
{
    layerOutputOwner_[layerId] = LayerOutputOwner{ deckId, frameSerial_ };
}

bool CompositorEngine::incomingImagePending(const Layer& layer, const Clip* clip) const
{
    if (clip == nullptr || clip->mediaType != Clip::MediaType::Image)
        return false;
    if (layer.crossfadeProgress >= 1.0f || layer.previousClipColumn < 0)
        return false;   // not fading: nothing to pause
    if (layer.type != Layer::Type::Opaque && layer.type != Layer::Type::Transparent && layer.type != Layer::Type::Mask)
        return false;   // FX Only / 3D never show the clip's media
    return imageCache_.notResident(clip->mediaFile.getFullPathName().toStdString());
}

// === Per-clip effect chain rendering (P13.5.1) ===

GLuint CompositorEngine::applyClipEffects(const std::vector<Clip::EffectSlot>& effects, GLuint inputTex,
                                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                                            float time, int w, int h,
                                            uint64_t stateKey, GLuint holdTex)
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

    // s-rta-0926b render lane (R5): create/resize the temporal buffer HERE,
    // before any pass binds its target. getOrCreateTemporalBuffer's createFBO
    // rebinds GL_TEXTURE_2D on the active unit and leaves framebuffer 0 bound;
    // called inside a pass (after the target FBO and u_texture were bound, as it
    // used to be), the creation frame drew into framebuffer 0, sampled the new
    // black buffer as u_texture, and the chain's result was only the target's
    // glClear -- a fully transparent layer for one frame (measured: first Freeze
    // frame on a fresh layer 0/663768 non-zero pixels instead of 0.5 x image).
    // unordered_map element references stay valid across later insertions.
    TemporalBuffer* tempBuf = anyTemporal ? &getOrCreateTemporalBuffer(stateKey, w, h) : nullptr;

    GLuint currentInput = inputTex;
    // s-rta-0926 xfade: every pass below renders into pickEffectTarget(
    // currentInput, holdTex) -- never the texture it samples (the ms-white2 /
    // 4fca2c5 feedback loop, when a caller hands in a pool texture) and never
    // the texture the caller still holds (the crossfade clobber: the outgoing
    // clip's chain overwrote the incoming clip's result). With nothing held
    // this is the same A/B ping-pong as before (external -> A -> B -> A ...).
    // A chain that starts from a pool texture can END on that same texture
    // (A -> B -> A), so "did this chain render?" is tracked explicitly below,
    // never inferred from currentInput != inputTex.
    bool renderedAny = false;

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
            const ScratchTarget split = pickEffectTarget(currentInput, holdTex);
            GLuint splitResult = applyScreenSplit(currentInput, slot, shaderMgr, quad, stateKey, w, h,
                                                  split.fbo, split.tex);
            if (splitResult != 0 && splitResult != currentInput)
            {
                currentInput = splitResult;
                renderedAny = true;
            }
            continue;
        }

        // Frame Stutter is handled specially — uses frame ring buffer
        if (def->shaderName == "frame_delay")
        {
            float depthParam = (slot.paramValues.size() > 0) ? slot.effParam(0) : 0.3f;
            float stutterParam = (slot.paramValues.size() > 1) ? slot.effParam(1) : 0.0f;

            auto& ring = getOrCreateRingBuffer(stateKey, w, h);
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
            {
                currentInput = delayedTex;
                renderedAny = true;
            }
            continue;
        }

        auto* program = shaderMgr.getProgram(def->shaderName);
        if (program == nullptr)
            continue;

        const ScratchTarget target = pickEffectTarget(currentInput, holdTex);

        glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
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
        // -- the buffer was created before this pass bound anything (R5 above).
        if (def->temporal && tempBuf != nullptr)
        {
            auto prevLoc = program->getUniformIDFromName("u_prev_frame");
            if (prevLoc >= 0)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tempBuf->tex);
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
                glUniform1f(loc, slot.effParam(p));
        }

        quad.draw();

        // Apply dry/wet blend if < 1.0: mix(original, effected, dryWet), done
        // IN PLACE on the target with fixed-function blending -- the target
        // already holds the effected result (dst) and the pre-effect input is
        // drawn over it (src): src*(1-w) + dst*w == mix(src, dst, w), the
        // effect_dry_wet shader's formula. The old separate pass needed a
        // THIRD texture (sample effected + original, write elsewhere) and,
        // with only A/B, drew into the texture it sampled as u_original
        // whenever the input was itself a pool texture (every non-first
        // effect of a chain). Blending reads dst through the ROP, not a
        // sampler: no feedback loop, no extra texture.
        const float dryWet = slot.effDryWet();
        if (dryWet < 0.999f)
        {
            if (auto* pt = shaderMgr.getProgram("passthrough"))
            {
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_ONE_MINUS_CONSTANT_ALPHA, GL_CONSTANT_ALPHA);
                glBlendColor(0.0f, 0.0f, 0.0f, dryWet);
                pt->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, currentInput);
                glUniform1i(pt->getUniformIDFromName("u_texture"), 0);
                quad.draw();
                glDisable(GL_BLEND);
            }
        }

        currentInput = target.tex;
        renderedAny = true;
    }

    // Save current output as previous frame for temporal effects next frame.
    // s-rta-0926 xfade class sweep: this used to test currentInput != inputTex,
    // which is FALSE for an even-length chain that starts on a pool texture
    // (layer chain [Invert, Freeze] fed by a one-effect clip chain: A -> B -> A)
    // -- the save was skipped and that chain's u_prev_frame never advanced.
    if (tempBuf != nullptr && renderedAny)
        saveToTemporalBuffer(*tempBuf, currentInput, shaderMgr, quad, w, h);

    return currentInput;
}

// === Clip transform ===

GLuint CompositorEngine::applyClipTransform(const Clip& clip, GLuint srcTex,
                                              ShaderManager& shaderMgr, FullscreenQuad& quad,
                                              int w, int h, GLuint holdTex)
{
    constexpr float eps = 0.001f;
    // S-RTA-0923 LANE 3 C2: read through the connection twin (LiveValue,
    // atomic relaxed load) with fallback to the raw manual field -- so a
    // connected scalar (e.g. an LFO driving Position X) actually renders.
    // Read once into locals; GL-thread-safe (LiveValue::effective is a
    // relaxed atomic load, same crossing the raw floats already used).
    const float effPositionX = clip.eff(ClipScalar::PosX);
    const float effPositionY = clip.eff(ClipScalar::PosY);
    const float effScale     = clip.eff(ClipScalar::Scale);
    const float effRotation  = clip.eff(ClipScalar::Rotation);
    const float effAnchorX   = clip.eff(ClipScalar::AnchorX);
    const float effAnchorY   = clip.eff(ClipScalar::AnchorY);
    const float effClipOpacity = clip.eff(ClipScalar::Opacity);

    // s-rta-0926b plan-fitmode. Only media with a picture of its own is fitted: a Source renders AT the
    // canvas size (compositeDeck's sourceRenderFn_ call) and Camera has no deck path. Size = the texture's
    // real size, never clipWidth/clipHeight (only the video open sites set those). Stretch runs today's
    // code: no query, and fit stays {1,1}.
    ClipFit::Scale fit;                                                   // {1,1}
    if (clip.fitMode != ClipFit::Mode::Stretch
        && (clip.mediaType == Clip::MediaType::Image || clip.mediaType == Clip::MediaType::Video
            || clip.mediaType == Clip::MediaType::ImageSequence))
    {
        GLint tw = 0, th = 0;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &tw);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);
        fit = ClipFit::scale(clip.fitMode, tw, th, w, h);
    }
    const bool fitActive = (fit.x != 1.0f || fit.y != 1.0f);             // exact: scale() returns {1,1} verbatim

    bool needsTransform = (std::abs(effPositionX) > eps ||
                           std::abs(effPositionY) > eps ||
                           std::abs(effScale - 1.0f) > eps ||
                           std::abs(effRotation) > eps) || fitActive;

    GLuint transformedTex = srcTex;

    if (needsTransform)
    {
        auto* prog = shaderMgr.getProgram("layer_transform");
        if (prog != nullptr)
        {
            // Pool target away from srcTex and from anything the caller holds.
            const ScratchTarget xf = pickEffectTarget(srcTex, holdTex);
            glBindFramebuffer(GL_FRAMEBUFFER, xf.fbo);
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
                        effPositionX / static_cast<float>(w),
                        effPositionY / static_cast<float>(h));
            glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_anchor"),
                        0.5f + effAnchorX, 0.5f + effAnchorY);
            glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_scale"),
                        effScale);
            glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_rotation"),
                        effRotation * 3.14159265f / 180.0f);
            // plan-fitmode: layer_transform is shared with applyLayerTransform and uniform values persist
            // per program, so u_fitEnabled is set on EVERY draw (0 or 1), never left to the default.
            glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_fitEnabled"), fitActive ? 1 : 0);
            glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_fitScale"), fit.x, fit.y);

            quad.draw();

            transformedTex = xf.tex;
        }
    }

    // S167-L4b: bake clipOpacity in here (rather than only at the final
    // layer-opacity site) so a crossfading clip's OWN opacity survives into
    // applyTransition's blend below -- none of the transition_* shaders in
    // EmbeddedShaders.h (transition_dissolve, wipes, pushes, zoom, iris,
    // flip, cut, fade-to-black) take a per-input opacity uniform, so this is
    // the only place a clip pinned below 1.0 stays capped through a
    // crossfade. Its target is picked away from transformedTex (s-rta-0926
    // xfade pool rule), so this pass never reads and writes the same
    // texture whether or not a positional transform ran first.
    //
    // s-rta-0925 ms-white FIX: forceCopy=needsTransform. When a transform
    // ran, transformedTex IS effectTex_A_ (this function's own scratch
    // above) -- the caller of applyClipTransform (compositeDeck) feeds the
    // returned texture straight into applyClipEffects, whose ping-pong
    // ALWAYS writes its first enabled effect to effectFBO_A_. Left as a
    // true no-op at the default 1.0 opacity, that handed effectTex_A_ back
    // verbatim, so the very next effect pass sampled and rendered to the
    // same texture in one draw call -- a GL feedback-loop hazard that
    // rendered as a fully transparent ("white") frame on this Metal-backed
    // driver. Forcing the copy through effectFBO_B_/effectTex_B_ here
    // breaks the alias; when there was no transform, transformedTex is the
    // original (non-scratch) srcTex and the true no-op remains safe.
    // (s-rta-0926 xfade: applyClipEffects now picks its targets away from its
    // input itself, so this copy is belt-and-braces, kept unchanged.)
    const ScratchTarget op = pickEffectTarget(transformedTex, holdTex);
    return applyClipOpacity(effClipOpacity, transformedTex, op.fbo, op.tex,
                            shaderMgr, quad, w, h, /*forceCopy=*/needsTransform);
}

GLuint CompositorEngine::applyClipOpacity(float opacity, GLuint srcTex, GLuint dstFBO, GLuint dstTex,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                                           int w, int h, bool forceCopy)
{
    constexpr float eps = 0.001f;
    if (!forceCopy && std::abs(opacity - 1.0f) <= eps)
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
    // S-RTA-0923 LANE 3 C2: eff() twin read (see applyClipTransform's comment).
    const float effPositionX    = layer.eff(LayerScalar::PosX);
    const float effPositionY    = layer.eff(LayerScalar::PosY);
    const float effLayerScale   = layer.eff(LayerScalar::Scale);
    const float effLayerRotation = layer.eff(LayerScalar::Rotation);
    const float effLayerAnchorX = layer.eff(LayerScalar::AnchorX);
    const float effLayerAnchorY = layer.eff(LayerScalar::AnchorY);

    // Skip transform if all values are at defaults
    constexpr float eps = 0.001f;
    bool needsTransform = (std::abs(effPositionX) > eps ||
                           std::abs(effPositionY) > eps ||
                           std::abs(effLayerScale - 1.0f) > eps ||
                           std::abs(effLayerRotation) > eps);
    if (!needsTransform)
        return srcTex;

    auto* prog = shaderMgr.getProgram("layer_transform");
    if (prog == nullptr)
        return srcTex;

    // s-rta-0926 xfade class sweep: render into the effect pool, NOT
    // scratchFBO_. A Transparent layer keys its texture INTO scratchFBO_
    // right after this (compositeDeck), so returning scratchTex_ made the
    // keying pass sample the texture it draws into (GL feedback loop, UB --
    // measured harmless on this driver only because keying's glClear is
    // deferred to the tile, not a guarantee). Nothing is held here.
    const ScratchTarget lt = pickEffectTarget(srcTex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, lt.fbo);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    prog->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_texture"), 0);
    glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_translate"),
                effPositionX, effPositionY);
    glUniform2f(glGetUniformLocation(prog->getProgramID(), "u_anchor"),
                effLayerAnchorX, effLayerAnchorY);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_scale"),
                effLayerScale);
    glUniform1f(glGetUniformLocation(prog->getProgramID(), "u_rotation"),
                effLayerRotation);
    // plan-fitmode LOAD-BEARING: the shared program keeps the last clip's u_fitEnabled = 1; a layer
    // transform must never fit the picture a second time.
    glUniform1i(glGetUniformLocation(prog->getProgramID(), "u_fitEnabled"), 0);

    quad.draw();

    return lt.tex;
}

// === FX Only layer (P13.5.2) ===

void CompositorEngine::applyFXOnlyLayer(const Clip& clip, const Layer& layer, uint64_t stateKey,
                                          ShaderManager& shaderMgr,
                                          FullscreenQuad& quad, float time, int w, int h)
{
    if (clip.effects.empty() || effectLibrary_ == nullptr)
        return;

    // Apply the clip's effects to the accumulator texture
    GLuint result = applyClipEffects(clip.effects, accumulatorTex_, shaderMgr, quad, time, w, h, stateKey);

    // S167-L4b: fold the clip's own opacity in here, the FX-Only layer's
    // constant-alpha blend -- owner's ruling is that master/layer/clip
    // opacity multiply (see combinedOpacity()'s comment), so a clip pinned
    // below 1.0 dilutes the FX-Only blend even further than layer.opacity
    // alone would.
    // S-RTA-0923 LANE 3 C2: eff() twin read (see applyClipTransform's comment).
    float effOpacity = combinedOpacity(layer.eff(LayerScalar::Opacity), clip.eff(ClipScalar::Opacity));

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

void CompositorEngine::advanceCrossfade(Layer& layer, float dt)
{
    // s-rta-0926b plan4 T1: the body lives in LayerClock (pure) so DeckClock::tick -- decks that are not on
    // screen -- runs the very same clock. Real dt (function param), not a hardcoded 1/60 -- see
    // compositeDeck()'s header comment and LayerClock::advanceCrossfade's.
    LayerClock::advanceCrossfade(layer, dt);
}

GLuint CompositorEngine::renderLayerStages(Layer& layer, uint32_t deckId, const Clip& clip, GLuint clipTex,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                                           float time, float dt, int width, int height)
{
    // s-rta-0926b R2: this layer's state (temporal buffers, frame rings,
    // feedback) is keyed by deck AND layer id -- layer ids repeat across decks.
    const uint64_t clipKey = LayerStateKey::clipChain(deckId, layer.id);

    // s-rta-0926b R1: while the layer crossfades, the outgoing clip's chain runs
    // on its own key. At the first frame of each crossfade the layer's clip-chain
    // history is handed to it (copy buffer, swap ring) -- BEFORE the incoming
    // clip's chain reads or writes that history this frame. Nothing happens at
    // fade end: the slot idles as the spare for the next fade.
    const uint64_t outKey = LayerStateKey::outgoingChain(deckId, layer.id);
    if (crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.activeClipColumn,
                                         layer.crossfadeProgress))
        handOverClipHistory(clipKey, outKey, shaderMgr, quad, width, height);

    // Apply per-clip transform (position, scale, rotation) + clip opacity
    clipTex = applyClipTransform(clip, clipTex, shaderMgr, quad, width, height);

    // P13.5.1: Apply per-clip effects
    clipTex = applyClipEffects(clip.effects, clipTex, shaderMgr, quad, time, width, height, clipKey);

    // P14: Apply clip-to-clip transition if crossfading. S167-L4b
    // DT-FIX: real measured dt (function param) -- see
    // compositeDeck()'s header comment.
    clipTex = applyTransition(layer, outKey, clipTex, time, shaderMgr, quad, width, height, dt);

    // P16: Apply feedback (Larsen loop) if enabled
    if (layer.feedback.enabled && layer.feedback.amount > 0.001f)
    {
        auto& fbProc = getOrCreateFeedbackProcessor(clipKey);
        clipTex = fbProc.process(clipTex, layer.feedback, shaderMgr, quad, width, height);
    }

    // Apply per-layer effects (same mechanism as per-clip effects)
    if (!layer.layerEffects.empty())
    {
        // Own temporal/ring state (LayerStateKey::layerChain): never the clip chain's.
        clipTex = applyClipEffects(layer.layerEffects, clipTex, shaderMgr, quad, time, width, height,
                                   LayerStateKey::layerChain(deckId, layer.id));
    }

    // P13.5.5: Apply layer transform
    return applyLayerTransform(layer, clipTex, shaderMgr, quad, width, height);
}

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
        if (clipHasContent(*clip))
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

        const Clip* clip = layer.getActiveClip();
        // C1 (s-rta-0928): a crossfade onto an image that is still decoding waits for it (the layer holds meanwhile).
        if (!incomingImagePending(layer, clip))
            advanceCrossfade(layer, dt);

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
                bool pending = false;   // s-rta-0928 R1.2: the picture is still decoding (NOT "no media")
                if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
                {
                    clipTex = getKeyTexture(clip->mediaFile, &pending);
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
                    clipTex = videoFrameFn_(clip, dt, &pending);   // R1.4: a sequence with nothing to show yet
                }

                if (pending)
                {
                    // s-rta-0928 R1: the picture is still decoding -- never "no media" (that would run the clip's
                    // effects as FX Only, below). Hold this layer's LAST picture (its Layer Router output from the
                    // previous frame); the layer's current keying/opacity still apply below, and no effect, ring,
                    // feedback or transition runs on the stand-in. Nothing to hold -> the layer draws nothing.
                    clipTex = heldLayerOutput(layer.id, deck.id);
                    if (clipTex == 0)
                    {
                        imageSkipFrames_.fetch_add(1, std::memory_order_relaxed);
                        continue;
                    }
                    imageHoldFrames_.fetch_add(1, std::memory_order_relaxed);
                    touchLayerOutput(layer.id, deck.id);
                }
                else
                {
                    // If no media but clip has effects, apply as FX-only (affects layers below)
                    if (clipTex == 0)
                    {
                        if (clip->hasEffects())
                            applyFXOnlyLayer(*clip, layer, LayerStateKey::clipChain(deck.id, layer.id),
                                             shaderMgr, quad, time, width, height);
                        continue;
                    }

                    // Clip transform + opacity, clip effects, transition, feedback,
                    // layer effects, layer transform (s-rta-0926b R4: shared with
                    // compositePersistentLayers).
                    clipTex = renderLayerStages(layer, deck.id, *clip, clipTex, shaderMgr, quad,
                                                time, dt, width, height);

                    // P20: Save layer output for Layer Router sources
                    saveLayerOutput(layer.id, deck.id, clipTex, shaderMgr, quad, width, height);
                }

                if (layer.type == Layer::Type::Transparent)
                {
                    // Apply keying → scratch FBO
                    applyLayerKeying(layer, layer.keyingMode, clipTex, scratchFBO_, shaderMgr, quad, width, height);
                    // Blend scratch onto accumulator
                    blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
                }
                else
                {
                    // Opaque: clear accumulator and draw with opacity
                    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    // S-RTA-0923 LANE 3 C2: eff() twin read (see applyClipTransform's comment).
                    const float effLayerOpacity = layer.eff(LayerScalar::Opacity);

                    if (effLayerOpacity < 0.999f)
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
                            glUniform1f(opLoc, effLayerOpacity);
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
                applyFXOnlyLayer(*clip, layer, LayerStateKey::clipChain(deck.id, layer.id),
                                 shaderMgr, quad, time, width, height);
                break;
            }
            case Layer::Type::Mask:
            {
                // P13.5.3: Use clip content as luminance mask on accumulator
                GLuint clipTex = 0;
                bool pending = false;   // s-rta-0928 R1.2
                if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
                    clipTex = getKeyTexture(clip->mediaFile, &pending);
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
                    clipTex = videoFrameFn_(clip, dt, &pending);   // R1.4
                }

                // s-rta-0928 R1.2: a Mask whose image still decodes holds its last IMAGE mask; nothing to hold -> no
                // mask this frame.
                const uint64_t maskKey = LayerStateKey::clipChain(deck.id, layer.id);
                if (pending)
                {
                    const auto held = lastMaskImageTex_.find(maskKey);
                    const GLuint heldTex = held != lastMaskImageTex_.end() ? held->second : 0;
                    if (heldTex != 0)
                    {
                        imageHoldFrames_.fetch_add(1, std::memory_order_relaxed);
                        applyMaskLayer(*clip, heldTex, shaderMgr, quad, width, height);
                    }
                    else
                        imageSkipFrames_.fetch_add(1, std::memory_order_relaxed);
                    break;
                }
                lastMaskImageTex_[maskKey] = (clip->mediaType == Clip::MediaType::Image) ? clipTex : 0;

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

bool CompositorEngine::clipHasContent(const Clip& clip)
{
    if (clip.mediaType == Clip::MediaType::Image && clip.mediaFile.existsAsFile())
        return true;
    if (clip.mediaType == Clip::MediaType::Source && !clip.sourceType.empty())
        return true;
    if (clip.mediaType == Clip::MediaType::Video && clip.mediaFile.existsAsFile())
        return true;
    if (clip.mediaType == Clip::MediaType::ImageSequence && !clip.sequenceFiles.empty())
        return true;
    // Layers with effects (even without media) are active — FX applies to accumulator
    return !clip.effects.empty();
}

bool CompositorEngine::hasPersistentContent(const Deck& deck)
{
    // The same gates as compositePersistentLayers() below, in the same order.
    bool anySolo = false;
    for (const auto& layer : deck.layers)
    {
        if (layer.solo) { anySolo = true; break; }
    }
    for (const auto& layer : deck.layers)
    {
        if (!layer.persistent || !layer.visible || layer.bypassed || (anySolo && !layer.solo))
            continue;
        const Clip* clip = layer.getActiveClip();
        if (clip != nullptr && Layer::canBePersistent(layer.type) && clipHasContent(*clip))
            return true;
    }
    return false;
}

GLuint CompositorEngine::beginEmptyActiveDeck(int width, int height)
{
    if (!glInitialized_)
        return 0;

    // compositeDeck() returned before its resize + clear (nothing on the active
    // deck to draw); do both here, as compositeDeck does before its first layer.
    resize(width, height);

    // Opaque black, not compositeDeck's transparent black: the persistent layers
    // then land on exactly what an Opaque black clip at full opacity leaves in
    // the accumulator (compositeDeck's Opaque branch clears to opaque black and
    // draws the clip with blending off), so an EMPTY active deck and a BLACK
    // active deck give the same frame, alpha included.
    glBindFramebuffer(GL_FRAMEBUFFER, accumulatorFBO_);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
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

        // s-rta-0926b R4: a persistent layer's crossfade keeps running while its
        // deck is inactive, exactly as it would on the active deck (it used to
        // freeze until the deck was active again, and the layer hard-cut to the
        // incoming clip meanwhile). C1 (s-rta-0928): except while its incoming
        // image is still decoding.
        const Clip* clip = layer.getActiveClip();
        if (!incomingImagePending(layer, clip))
            advanceCrossfade(layer, dt);

        if (clip == nullptr)
            continue;

        // s-rta-0926b R4-types: Opaque / Transparent / FX Only (Layer::
        // canBePersistent -- the same rule disables the LayerInspector toggle).
        // Mask and 3D persistent layers stay skipped.
        if (!Layer::canBePersistent(layer.type))
            continue;

        // FX Only: the clip's effects over the accumulator as it stands at this
        // point of the persistent pass (the active deck + persistent layers of
        // lower-index decks) -- the same call and key as on its own deck.
        if (layer.type == Layer::Type::FXOnly)
        {
            applyFXOnlyLayer(*clip, layer, LayerStateKey::clipChain(deck.id, layer.id),
                             shaderMgr, quad, time, width, height);
            continue;
        }

        GLuint clipTex = 0;
        bool pending = false;   // s-rta-0928 R1.2
        if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
        {
            clipTex = getKeyTexture(clip->mediaFile, &pending);
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
            clipTex = videoFrameFn_(clip, dt, &pending);   // R1.4
        }

        // s-rta-0928 R1.2: a persistent layer whose image still decodes draws nothing this frame (it saves no Layer
        // Router output to hold) -- never "no media" (the FX-only branch below).
        if (pending)
        {
            imageSkipFrames_.fetch_add(1, std::memory_order_relaxed);
            continue;
        }

        // A media-less clip with effects on an Opaque/Transparent layer applies
        // as FX Only, exactly as on the active deck (s-rta-0926b R4-types).
        if (clipTex == 0)
        {
            if (clip->hasEffects())
                applyFXOnlyLayer(*clip, layer, LayerStateKey::clipChain(deck.id, layer.id),
                                 shaderMgr, quad, time, width, height);
            continue;
        }

        // s-rta-0926b R4: the same per-layer stages as on the active deck
        // (clip transform + opacity, clip effects, transition, feedback, layer
        // effects, layer transform). It used to run only the clip effects.
        // Deliberately different from an active-deck layer (ruling
        // .harmony/.reports/s-rta-0926b/ruling-render-forks.md): an Opaque
        // persistent layer blends over the active deck with its blend mode and
        // layer opacity (it never clears the accumulator); no Layer Router
        // output is saved (the router addresses the active deck only); Mask /
        // 3D persistent layers are skipped above.
        GLuint processedTex = renderLayerStages(layer, deck.id, *clip, clipTex, shaderMgr, quad,
                                                time, dt, width, height);
        if (processedTex == 0) processedTex = clipTex;

        // Keying for transparent layers
        if (layer.type == Layer::Type::Transparent)
        {
            applyLayerKeying(layer, layer.keyingMode, processedTex, scratchFBO_, shaderMgr, quad, width, height);
            blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
        }
        else if (layer.eff(LayerScalar::Opacity) < 0.999f)
        {
            // s-rta-0926b R4-opaque (ruling (1)): a persistent Opaque layer sits
            // on TOP of the active deck (persistent layers composite after it),
            // so it blends over it with its blend mode -- it never clears the
            // accumulator the way an active-deck Opaque layer does (that would
            // black out the whole active deck from a default-typed layer). Its
            // layer opacity used to be ignored here; it now goes through the
            // same alpha keying pass (u_opacity) a Transparent layer gets.
            // processedTex is never scratchTex_ (applyLayerTransform renders
            // into the effect pool), so the pass never samples its own target.
            applyLayerKeying(layer, Layer::KeyingMode::Alpha, processedTex, scratchFBO_, shaderMgr, quad,
                             width, height);
            blendLayerOntoAccumulator(layer, scratchTex_, shaderMgr, quad, width, height);
        }
        else
        {
            // Opacity 1.0: the direct blend, unchanged (no keying pass).
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

    // LayerStateKey::kGlobalEffects keeps this call's temporal buffer /
    // screen-split ring buffer from aliasing a real layer's.
    return applyClipEffects(globalEffects, inputTex, shaderMgr, quad, time, w, h,
                            LayerStateKey::kGlobalEffects);
}

void CompositorEngine::applyLayerKeying(const Layer& layer, Layer::KeyingMode mode, GLuint srcTex, GLuint dstFBO,
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
    switch (mode)
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
    // S-RTA-0923 LANE 3 C2: eff() twin read (see applyClipTransform's comment).
    glUniform1f(loc("u_opacity"), layer.eff(LayerScalar::Opacity));
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

GLuint CompositorEngine::getClipTexture(const Clip& clip, float time, int w, int h, float dt, bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    // s-rta-0928 R1.2: a pending OUTGOING image returns 0 -- applyTransition shows the incoming clip alone.
    if (clip.mediaType == Clip::MediaType::Image && clip.mediaFile.existsAsFile())
        return getKeyTexture(clip.mediaFile, pending);

    if (clip.mediaType == Clip::MediaType::Source && !clip.sourceType.empty() && sourceRenderFn_)
    {
        const auto* params = clip.sourceParams.empty() ? nullptr : &clip.sourceParams;
        return sourceRenderFn_(clip.sourceType, time, w, h, params);
    }

    if ((clip.mediaType == Clip::MediaType::Video ||
         clip.mediaType == Clip::MediaType::ImageSequence) && videoFrameFn_)
        return videoFrameFn_(&clip, dt, pending);

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

GLuint CompositorEngine::applyTransition(Layer& layer, uint64_t outgoingKey, GLuint newClipTex, float time,
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

    // s-rta-0926 xfade: newClipTex is HELD from here to the transition draw,
    // across two passes that could overwrite it:
    //  1. getClipTexture(prevClip): procedural sources are cached per source
    //     TYPE (Renderer::getOrCreateSource), so when both clips are the same
    //     type the outgoing render lands in the SAME output texture the
    //     incoming clip is still using (both dissolve inputs showed the
    //     outgoing clip). Only then -- incoming result still that shared
    //     texture, i.e. no pool pass ran on it -- copy it into the pool first.
    //     One full-frame copy, only while such a crossfade runs.
    //  2. the outgoing clip's effect chain: runs with holdTex = newClipTex, so
    //     no pass of it writes the incoming result (both clips effected: the
    //     incoming result sat in effectTex_A_, the outgoing chain started at
    //     effectFBO_A_ and overwrote it).
    const Clip* newClip = layer.getActiveClip();
    if (newClip != nullptr
        && newClip->mediaType == Clip::MediaType::Source && prevClip->mediaType == Clip::MediaType::Source
        && !newClip->sourceType.empty() && newClip->sourceType == prevClip->sourceType
        && !isEffectPoolTexture(newClipTex))
    {
        if (auto* pt = shaderMgr.getProgram("passthrough"))
        {
            const ScratchTarget keep = pickEffectTarget(newClipTex, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, keep.fbo);
            glViewport(0, 0, w, h);
            glDisable(GL_BLEND);
            pt->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, newClipTex);
            glUniform1i(pt->getUniformIDFromName("u_texture"), 0);
            quad.draw();
            newClipTex = keep.tex;
        }
    }

    GLuint prevTex = getClipTexture(*prevClip, time, w, h, dt);
    if (prevTex == 0)
        return newClipTex;

    // s-rta-0926b render lane (R3): the outgoing clip goes through the SAME
    // per-clip stages, in the same order, as it did while it was the active
    // clip (compositeDeck: applyClipTransform = transform + clip opacity, then
    // applyClipEffects). This used to run effects -> opacity with no transform:
    // an outgoing clip with Position/Scale/Rotation snapped to identity for the
    // whole crossfade (measured: every dissolve frame fit the UNSCALED clip,
    // residual 0.09-0.13, vs 5.3-21.8 against the clip as it was shown), and an
    // outgoing clip with opacity < 1 and a non-linear effect changed look at
    // transition start (0.5 x invert(A) instead of invert(0.5 x A)). Neither
    // pass writes the held incoming result.
    prevTex = applyClipTransform(*prevClip, prevTex, shaderMgr, quad, w, h, /*holdTex=*/newClipTex);
    // s-rta-0926b R1: the outgoing chain keys its temporal buffer / frame ring
    // by the layer's OUTGOING slot (handed over at fade start, renderLayerStages)
    // -- it used to share the incoming chain's key, so each clip read the
    // other's output as its history (1/3 ghost at Freeze 0.5; with Frame
    // Stutter each clip showed the other outright).
    prevTex = applyClipEffects(prevClip->effects, prevTex, shaderMgr, quad, time, w, h, outgoingKey,
                               /*holdTex=*/newClipTex);

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

FeedbackProcessor& CompositorEngine::getOrCreateFeedbackProcessor(uint64_t stateKey)
{
    auto it = feedbackProcessors_.find(stateKey);
    if (it != feedbackProcessors_.end())
        return *it->second;

    auto proc = std::make_unique<FeedbackProcessor>();
    auto& ref = *proc;
    feedbackProcessors_[stateKey] = std::move(proc);
    return ref;
}

CompositorEngine::TemporalBuffer& CompositorEngine::getOrCreateTemporalBuffer(uint64_t stateKey, int w, int h)
{
    auto& buf = layerTemporalBuffers_[stateKey];
    if (buf.tex != 0 && buf.width == w && buf.height == h)
        return buf;

    // Release old if size changed
    if (buf.tex != 0) temporalBufferCount_.fetch_sub(1, std::memory_order_relaxed);
    if (buf.fbo != 0) { glDeleteFramebuffers(1, &buf.fbo); buf.fbo = 0; }
    if (buf.tex != 0) { glDeleteTextures(1, &buf.tex); buf.tex = 0; }

    buf.width = w;
    buf.height = h;
    createFBO(buf.fbo, buf.tex, w, h);
    temporalBufferCount_.fetch_add(1, std::memory_order_relaxed);

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

CompositorEngine::FrameRingBuffer& CompositorEngine::getOrCreateRingBuffer(uint64_t stateKey, int w, int h)
{
    // Initialises (or re-sizes) the ring's bookkeeping only: it creates NO GL object -- every cell is created on
    // its first write in pushFrameToRing (s-rta-0927 plan-renderperf C1). A size change deletes the old cells.
    // plan4 1E: a ring cell is never stored wider than 480 px (kRingDownscale is the minimum).
    const int ds = RenderGeometry::ringDownscale(w);
    int rw = std::max(1, w / ds);
    int rh = std::max(1, h / ds);

    auto& ring = layerRingBuffers_[stateKey];
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
        frameRingCount_.fetch_sub(1, std::memory_order_relaxed);
        frameRingCellCount_.fetch_sub(ring.allocatedCells, std::memory_order_relaxed);
        ring.allocatedCells = 0;
    }

    ring.ringWidth = rw;
    ring.ringHeight = rh;
    ring.writeIndex = 0;
    ring.frameCount = 0;
    // assign, never resize: on a size-change re-init the vectors still hold the handles deleted just above, and
    // pushFrameToRing creates a cell only where the handle is 0 -- a kept stale handle would be bound as a
    // deleted FBO. Load-bearing (plan-renderperf C1, R2).
    ring.fbos.assign(static_cast<size_t>(kMaxRingFrames), 0);
    ring.textures.assign(static_cast<size_t>(kMaxRingFrames), 0);

    ring.initialized = true;
    frameRingCount_.fetch_add(1, std::memory_order_relaxed);
    return ring;
}

void CompositorEngine::handOverClipHistory(uint64_t clipKey, uint64_t outKey,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h)
{
    // s-rta-0926b R1 (ruling B'). Runs outside any pass: it may create the spare
    // buffer (createFBO rebinds GL_TEXTURE_2D on the active unit and leaves
    // framebuffer 0 bound -- the R5 lesson); every later pass binds its own FBO
    // and textures, so nothing needs restoring.

    // Temporal buffer: COPY layer -> outgoing slot. The outgoing clip keeps its
    // Echo / Freeze / Posterize Time history unbroken; the incoming clip reads
    // the layer buffer (the picture the layer was just showing) as its first
    // u_prev_frame -- exactly what a cut gives it.
    auto itL = layerTemporalBuffers_.find(clipKey);
    if (itL != layerTemporalBuffers_.end() && itL->second.tex != 0)
    {
        const GLuint layerTex = itL->second.tex;
        auto& out = getOrCreateTemporalBuffer(outKey, w, h);   // creates/resizes; src != dst always
        saveToTemporalBuffer(out, layerTex, shaderMgr, quad, w, h);
    }
    else
    {
        // Nothing to copy: a stale spare from an earlier fade must not be read
        // as this outgoing clip's history -- clear it to a fresh (transparent
        // black) history.
        auto itO = layerTemporalBuffers_.find(outKey);
        if (itO != layerTemporalBuffers_.end() && itO->second.fbo != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, itO->second.fbo);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    // Frame ring: SWAP (O(1), vector moves). The outgoing Screen Split / Frame
    // Stutter keeps its frames; the incoming clip's ring starts empty (the same
    // degradation as a fresh layer: getFrameFromRing clamps to the frames
    // available). A missing entry on either key is created lazily on first use.
    std::swap(layerRingBuffers_[clipKey], layerRingBuffers_[outKey]);
    auto& incoming = layerRingBuffers_[clipKey];
    if (incoming.initialized)
    {
        incoming.frameCount = 0;
        incoming.writeIndex = 0;
    }
}

void CompositorEngine::pushFrameToRing(FrameRingBuffer& ring, GLuint srcTex,
                                        ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h)
{
    (void)w; (void)h; // full-res dimensions not used — we render at ring resolution
    auto* prog = shaderMgr.getProgram("passthrough");
    if (!prog) return;

    auto idx = static_cast<size_t>(ring.writeIndex);
    if (ring.textures[idx] == 0)   // plan-renderperf C1: a cell is created on its first write, never in bulk.
    {                              // createFBO leaves framebuffer 0 bound and rebinds GL_TEXTURE_2D on the active
        createFBO(ring.fbos[idx], ring.textures[idx], ring.ringWidth, ring.ringHeight);   // unit (R5) -- both are
        ++ring.allocatedCells;                                                             // re-bound right below.
        frameRingCellCount_.fetch_add(1, std::memory_order_relaxed);
    }
    // No clear: the passthrough draw below overwrites every byte of the cell (full viewport, blend off, no
    // scissor), and no read can return this cell before this push completes (FrameRing::readIndex).
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
    if (!ring.initialized)
        return 0;
    // The pure index math (tests/test_frame_ring.cpp): the index is always a cell written in this lifetime.
    const int idx = FrameRing::readIndex(ring.writeIndex, ring.frameCount, framesAgo, kMaxRingFrames);
    return idx < 0 ? 0 : ring.textures[static_cast<size_t>(idx)];
}

// === Screen Split: render grid with per-cell delay ===

GLuint CompositorEngine::applyScreenSplit(GLuint clipTex, const Clip::EffectSlot& slot,
                                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                                           uint64_t stateKey, int w, int h,
                                           GLuint dstFBO, GLuint dstTex)
{
    if (effectLibrary_ == nullptr) return clipTex;

    const auto* def = effectLibrary_->getEffectDef("Screen Split");
    if (def == nullptr) return clipTex;

    // Read params: columns, rows, delay, mode
    float colsParam = (slot.paramValues.size() > 0) ? slot.effParam(0) : 0.15f;
    float rowsParam = (slot.paramValues.size() > 1) ? slot.effParam(1) : 0.15f;
    float delayParam = (slot.paramValues.size() > 2) ? slot.effParam(2) : 0.5f;
    float modeParam = (slot.paramValues.size() > 3) ? slot.effParam(3) : 0.0f;

    int cols = static_cast<int>(2.0f + colsParam * 6.0f); // 2-8
    int rows = static_cast<int>(2.0f + rowsParam * 6.0f); // 2-8
    // delay: frames between cells. Slider [0,1] → [0, 60] frames per cell.
    // At 1.0 with a 2x2 grid (4 cells), total span = 180 frames = 3 seconds at 60fps.
    int framesPerCell = static_cast<int>(std::round(60.0f * delayParam));

    // Get or create ring buffer, push current frame
    auto& ring = getOrCreateRingBuffer(stateKey, w, h);
    pushFrameToRing(ring, clipTex, shaderMgr, quad, w, h);

    // Render the grid into the caller's pool target (picked away from clipTex
    // and from anything the caller holds -- s-rta-0926 xfade pool rule)
    glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
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
    return dstTex;
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
