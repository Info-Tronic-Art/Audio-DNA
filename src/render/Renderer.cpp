#include "Renderer.h"
#include "render/EmbeddedShaders.h"
#include "sources/ProjectMSource.h"
#include "analysis/AnalysisThread.h"
#include "recording/VideoRecorder.h"
#include "output/SyphonOutput.h"
#include <iostream>
#include <chrono>
#include <string>
#include <random>

using namespace juce::gl;
Renderer::Renderer(const FeatureBus& featureBus)
    : featureBus_(featureBus)
{
}

Renderer::~Renderer()
{
    detach();
}

// Syphon status/toggle for REST (ApiServer). These just forward to
// SyphonOutput's own atomics (SyphonOutput.h) — no GL calls, safe from any
// thread. Defined here (not inline in Renderer.h) because SyphonOutput is
// only forward-declared in the header; this file already includes the full
// output/SyphonOutput.h for the syphonOutput_->init()/publishTexture() calls
// below.
bool Renderer::isSyphonEnabled() const
{
    return syphonOutput_ != nullptr && syphonOutput_->isEnabled();
}

bool Renderer::isSyphonAvailable() const
{
    return syphonOutput_ != nullptr && syphonOutput_->isInitialized();
}

void Renderer::setSyphonEnabled(bool enabled)
{
    if (syphonOutput_ != nullptr)
        syphonOutput_->setEnabled(enabled);
}

void Renderer::attachTo(juce::Component& component)
{
    glContext_.setOpenGLVersionRequired(juce::OpenGLContext::openGL4_1);
    glContext_.setRenderer(this);
    glContext_.setContinuousRepainting(true);
    glContext_.setComponentPaintingEnabled(false);
    glContext_.attachTo(component);
}

void Renderer::detach()
{
    glContext_.detach();
}

void Renderer::loadImage(const juce::File& imageFile)
{
    std::cerr << "[Renderer] loadImage: " << imageFile.getFullPathName() << std::endl;
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    pendingImageFile_ = imageFile;
    hasPendingImage_ = true;
}

void Renderer::clearImage()
{
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    pendingClearImage_ = true;
}

void Renderer::queueCameraFrame(const juce::Image& frame)
{
    std::lock_guard<std::mutex> lock(cameraFrameMutex_);
    pendingCameraFrame_ = frame;
    hasPendingCameraFrame_ = true;
}

void Renderer::setActiveSource(const std::string& sourceType,
                                const std::vector<Clip::SourceParam>& params)
{
    std::lock_guard<std::mutex> lock(activeSourceMutex_);
    activeSourceType_ = sourceType;
    activeSourceParams_ = params;
    hasActiveSource_ = true;
}

void Renderer::clearActiveSource()
{
    std::lock_guard<std::mutex> lock(activeSourceMutex_);
    activeSourceType_.clear();
    activeSourceParams_.clear();
    hasActiveSource_ = false;
}

void Renderer::updateActiveSourceParams(const std::vector<Clip::SourceParam>& params)
{
    std::lock_guard<std::mutex> lock(activeSourceMutex_);
    activeSourceParams_ = params;
}

void Renderer::newOpenGLContextCreated()
{
    std::cerr << "[Renderer] GL context created. Version: "
              << glGetString(GL_VERSION) << std::endl;

    quad_.init();
    initShaders();

    // W7(iv) outputwindow-arc: one-shot per-context program-ID log. Compare
    // against the [OutputRenderer] lines when the output window opens —
    // overlapping ID sets confirm (disjoint sets refute) scout R1's INFERRED
    // cross-context program-ID collision claim.
    for (const char* name : { "passthrough", "hue_shift", "vignette" })
        if (auto* p = shaderMgr_.getProgram(name))
            std::cerr << "[Renderer] programID(" << name << ")="
                      << p->getProgramID() << std::endl;

    initEffectChain();
    compositor_.initGL(1920, 1080); // Will resize as needed
    compositor_.setEffectLibrary(&effectLibrary_);

    // Wire source rendering into compositor. S167-L4b: ignores the `time`
    // CompositorEngine passes (that's wall-clock, shared with clip effects/
    // transitions) and substitutes scaledTime_ instead, so masterSpeed scales
    // procedural-source animation rate without touching anything else's
    // timing -- see scaledTime_'s comment in Renderer.h.
    compositor_.setSourceRenderer([this](const std::string& sourceId, float /*time*/, int w, int h,
                                         const std::vector<Clip::SourceParam>* params) -> GLuint {
        return renderSource(sourceId, static_cast<float>(scaledTime_), w, h, params);
    });

    // Wire video frame provider into compositor
    compositor_.setVideoFrameProvider([this](const Clip* clip, float dt) -> GLuint {
        return getVideoFrameTexture(clip, dt);
    });

    // P22.1: Initialize the Syphon server on the GL thread. The Syphon server
    // needs the underlying NSOpenGLContext, which JUCE exposes via getRawContext().
    // No-op at runtime unless built with -DAUDIODNA_BUILD_SYPHON=ON and the
    // Syphon.framework is installed.
    if (syphonOutput_ != nullptr)
        syphonOutput_->init(glContext_.getRawContext());

    startTime_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void Renderer::renderOpenGL()
{
    // Release any media players closeMediaForClip() retired from the message
    // thread (media-leak fix, L1) — the only place this runs, since this
    // function is guaranteed to execute on the GL thread with a context
    // current.
    drainRetiredMedia();

    // Handle pending image load or clear (from message thread)
    {
        std::lock_guard<std::mutex> lock(pendingImageMutex_);
        if (pendingClearImage_)
        {
            texMgr_.release();
            pendingClearImage_ = false;
            hasPendingImage_ = false;
        }
        else if (hasPendingImage_)
        {
            std::cerr << "[Renderer] Processing pending image..." << std::endl;
            bool ok = texMgr_.loadImage(pendingImageFile_);
            std::cerr << "[Renderer] Image load " << (ok ? "OK" : "FAILED")
                      << ", hasImage=" << texMgr_.hasImage() << std::endl;
            hasPendingImage_ = false;
        }
    }

    // Handle pending camera frame
    {
        std::lock_guard<std::mutex> lock(cameraFrameMutex_);
        if (hasPendingCameraFrame_)
        {
            texMgr_.uploadImage(pendingCameraFrame_);
            hasPendingCameraFrame_ = false;
        }
    }

    // FPS tracking
    double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    ++frameCount_;
    if (now - fpsTimer_ >= 1.0)
    {
        currentFps_.store(static_cast<float>(frameCount_) / static_cast<float>(now - fpsTimer_),
                          std::memory_order_relaxed);
        frameCount_ = 0;
        fpsTimer_ = now;
    }

    // Clear to near-black
    juce::OpenGLHelpers::clear(juce::Colour(0xff0a0a14));

    // Check for active procedural source
    std::string currentSourceType;
    std::vector<Clip::SourceParam> currentSourceParams;
    bool sourceActive = false;
    {
        std::lock_guard<std::mutex> lock(activeSourceMutex_);
        if (hasActiveSource_ && !activeSourceType_.empty())
        {
            currentSourceType = activeSourceType_;
            currentSourceParams = activeSourceParams_;
            sourceActive = true;
        }
    }

    // Check for active deck compositing
    Deck* deck = activeDeck_.load(std::memory_order_acquire);
    bool deckActive = (deck != nullptr);

    // Check if we have anything to render
    if (!texMgr_.hasImage() && !sourceActive && !deckActive)
    {
        // Process pending frame capture even when there's nothing to render
        auto* comp2 = glContext_.getTargetComponent();
        float s2 = static_cast<float>(glContext_.getRenderingScale());
        float cw2 = comp2 ? static_cast<float>(comp2->getWidth()) * s2 : 256.0f;
        float ch2 = comp2 ? static_cast<float>(comp2->getHeight()) * s2 : 256.0f;
        processPendingCapture(cw2, ch2, 0, 0, cw2, ch2);
        return; // Nothing to render yet
    }

    // Read latest audio features (R5: coherent caller-owned value copy)
    const FeatureSnapshot snap = featureBus_.read();

    // S166-L1: SignalRegistry::evaluateAll() moved OFF this GL callback — it
    // is now confined to the message thread (MainComponent::tickFeaturePipeline,
    // 120Hz), the sole evaluator. GL-thread evaluation raced the UI's Signal
    // setters (SignalInspector) on the message thread; see Signal.h.
    // The dead routingEngine_.processFrame() call (RoutingEngine has no live
    // consumer — addRoute()'s only caller is TestServer.cpp, and no test
    // depends on per-frame route application; grepped tests/ + tests/visual/
    // for add_route/addRoute) is deleted with it.

    // W5 (outputwindow-arc-design.md): mappingEngine_.processFrame() moved
    // OFF this GL callback. A message-thread juce::Timer
    // (MainComponent::MappingTickTimer, kMappingTickHz) is now the SOLE
    // caller, running unconditionally so mapped params keep updating even
    // while this GL context is detached (previewPanel_ hidden). See
    // MainComponent::tickFeaturePipeline().

    // P13.5.9: Process autopilot (beat-synced clip advancement + beat snap)
    if (deckActive)
    {
        bool clipAdvanced = autopilot_.processFrame(*deck, snap);
        if (clipAdvanced && onAutopilotAdvanced_)
        {
            // Notify UI thread to refresh deck view
            auto callback = onAutopilotAdvanced_;
            juce::MessageManager::callAsync([callback]() { callback(); });
        }
    }

    // P23: Detect genre changes and fire callback
    {
        uint8_t currentGenre = snap.detectedGenre;
        if (currentGenre != lastDetectedGenre_ && snap.genreConfidence > 0.1f)
        {
            lastDetectedGenre_ = currentGenre;
            if (onGenreChanged_)
            {
                auto callback = onGenreChanged_;
                auto genre = currentGenre;
                auto conf = snap.genreConfidence;
                juce::MessageManager::callAsync([callback, genre, conf]() { callback(genre, conf); });
            }
        }

        uint8_t currentStructural = snap.structuralState;
        if (currentStructural != lastStructuralState_)
        {
            lastStructuralState_ = currentStructural;
            if (onStructuralStateChanged_)
            {
                auto callback = onStructuralStateChanged_;
                auto state = currentStructural;
                juce::MessageManager::callAsync([callback, state]() { callback(state); });
            }
        }
    }

    // P20.5: Process MilkDrop preset playlist cycling (beat-synced preset advance within clips)
    if (deckActive)
    {
        for (int li = 0; li < deck->getNumLayers(); ++li)
        {
            auto* layer = deck->getLayer(li);
            if (!layer || layer->activeClipColumn < 0) continue;

            auto* clip = layer->getActiveClip();
            if (!clip || clip->sourceType != "projectm_visualizer") continue;
            if (!clip->hasPresetPlaylist()) continue;
            if (clip->presetPlaylist.size() <= 1) continue;

            // Detect beat crossing
            bool beatCrossing = (snap.beatPhase < lastPlaylistBeatPhase_ - 0.5f);
            lastPlaylistBeatPhase_ = snap.beatPhase;

            if (beatCrossing)
            {
                clip->presetBeatsPlayed++;

                int targetBeats = clip->playlistTriggerBeats;
                if (clip->playlistTrigger == Clip::PlaylistTrigger::Bars)
                    targetBeats *= 4;
                else if (clip->playlistTrigger == Clip::PlaylistTrigger::Phrase)
                    targetBeats *= 16;

                if (clip->presetBeatsPlayed >= targetBeats)
                {
                    clip->presetBeatsPlayed = 0;
                    int listSize = static_cast<int>(clip->presetPlaylist.size());

                    // Advance based on cycle mode
                    int nextIdx = clip->presetPlaylistIndex;
                    switch (clip->playlistCycleMode)
                    {
                        case Clip::PlaylistCycleMode::Sequential:
                            nextIdx = (nextIdx + 1) % listSize;
                            break;
                        case Clip::PlaylistCycleMode::Reverse:
                            nextIdx = (nextIdx - 1 + listSize) % listSize;
                            break;
                        case Clip::PlaylistCycleMode::RandomOther:
                        {
                            static std::mt19937 rng(std::random_device{}());
                            int r = std::uniform_int_distribution<int>(0, listSize - 2)(rng);
                            nextIdx = (r >= nextIdx) ? r + 1 : r;
                            break;
                        }
                        case Clip::PlaylistCycleMode::RandomBag:
                        case Clip::PlaylistCycleMode::PingPong:
                        {
                            static std::mt19937 rng(std::random_device{}());
                            nextIdx = std::uniform_int_distribution<int>(0, listSize - 1)(rng);
                            break;
                        }
                    }

                    clip->presetPlaylistIndex = nextIdx;
                    const auto& entry = clip->presetPlaylist[static_cast<size_t>(nextIdx)];

                    // Load the next preset into the projectM source
                    auto* pmSource = dynamic_cast<ProjectMSource*>(
                        getOrCreateSource("projectm_visualizer"));
                    if (pmSource)
                        pmSource->loadPreset(entry.presetPath, true);
                }
            }

            // Also handle structural transitions (On Drop / On Breakdown)
            if (clip->playlistTrigger == Clip::PlaylistTrigger::OnDrop
                && snap.structuralState == 2 && lastPlaylistStructState_ != 2)
            {
                clip->presetBeatsPlayed = 0;
                static std::mt19937 rng(std::random_device{}());
                int listSize = static_cast<int>(clip->presetPlaylist.size());
                clip->presetPlaylistIndex = std::uniform_int_distribution<int>(0, listSize - 1)(rng);
                auto* pmSource = dynamic_cast<ProjectMSource*>(getOrCreateSource("projectm_visualizer"));
                if (pmSource)
                    pmSource->loadPreset(clip->presetPlaylist[static_cast<size_t>(clip->presetPlaylistIndex)].presetPath, true);
            }
            if (clip->playlistTrigger == Clip::PlaylistTrigger::OnBreakdown
                && snap.structuralState == 3 && lastPlaylistStructState_ != 3)
            {
                clip->presetBeatsPlayed = 0;
                static std::mt19937 rng(std::random_device{}());
                int listSize = static_cast<int>(clip->presetPlaylist.size());
                clip->presetPlaylistIndex = std::uniform_int_distribution<int>(0, listSize - 1)(rng);
                auto* pmSource = dynamic_cast<ProjectMSource*>(getOrCreateSource("projectm_visualizer"));
                if (pmSource)
                    pmSource->loadPreset(clip->presetPlaylist[static_cast<size_t>(clip->presetPlaylistIndex)].presetPath, true);
            }
            lastPlaylistStructState_ = snap.structuralState;
        }
    }

    // Calculate time (allow override for deterministic test rendering)
    float overrideT = timeOverride_.load(std::memory_order_relaxed);
    float time = (overrideT >= 0.0f) ? overrideT
        : static_cast<float>(juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime_);

    // S167-L4b: advance scaledTime_ for procedural sources (see its comment
    // in Renderer.h). In deterministic test-capture mode (timeOverride_ set)
    // track the override 1:1, scaled, instead of accumulating -- otherwise
    // render_frame's byte-identical-repeat guarantee would break, since
    // every real GL frame would still tick scaledTime_ forward even while
    // `time` itself stays pinned for the capture.
    float masterSpeedVal = (composition_ != nullptr) ? composition_->masterSpeed : 1.0f;
    if (overrideT >= 0.0f)
        scaledTime_ = static_cast<double>(overrideT) * static_cast<double>(masterSpeedVal);
    else
        scaledTime_ += (1.0 / 60.0) * static_cast<double>(masterSpeedVal);

    // S167-L4b DT-FIX: real measured frame delta, fed to
    // compositeDeck()/compositePersistentLayers() below for video/image-
    // sequence playhead advancement -- see lastFrameTimestampMs_'s comment
    // in Renderer.h for why this must be a REAL delta, not a hardcoded
    // 1/60. Clamped to [0, 0.25]s so a debugger pause, backgrounding, or the
    // very first frame (lastFrameTimestampMs_ == -1) can't make video jump
    // by an unbounded amount in one advanceFrame() call.
    double nowMs = juce::Time::getMillisecondCounterHiRes();
    float realDt = (lastFrameTimestampMs_ >= 0.0)
        ? static_cast<float>((nowMs - lastFrameTimestampMs_) / 1000.0)
        : (1.0f / 60.0f);
    realDt = std::clamp(realDt, 0.0f, 0.25f);
    lastFrameTimestampMs_ = nowMs;

    // Get physical pixel dimensions
    auto* component = glContext_.getTargetComponent();
    float scale = static_cast<float>(glContext_.getRenderingScale());
    float compW = component != nullptr ? static_cast<float>(component->getWidth())  * scale : 1.0f;
    float compH = component != nullptr ? static_cast<float>(component->getHeight()) * scale : 1.0f;

    // Check for locked resolution
    int lockW = lockedWidth_.load(std::memory_order_relaxed);
    int lockH = lockedHeight_.load(std::memory_order_relaxed);
    float renderW = (lockW > 0 && lockH > 0) ? static_cast<float>(lockW) : compW;
    float renderH = (lockW > 0 && lockH > 0) ? static_cast<float>(lockH) : compH;

    // Get the default framebuffer that JUCE's context uses
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    // Compute letterboxed viewport to maintain correct aspect ratio
    float aspectW = renderW;
    float aspectH = renderH;
    if (lockW <= 0 || lockH <= 0)
    {
        if (sourceActive)
        {
            // Sources use the render resolution directly (no image to aspect-match)
            aspectW = renderW;
            aspectH = renderH;
        }
        else
        {
            // In auto mode, use the loaded image's aspect ratio
            int imgW = texMgr_.getImageWidth();
            int imgH = texMgr_.getImageHeight();
            if (imgW > 0 && imgH > 0)
            {
                aspectW = static_cast<float>(imgW);
                aspectH = static_cast<float>(imgH);
            }
        }
    }

    float scaleX = compW / aspectW;
    float scaleY = compH / aspectH;
    float fitScale = std::min(scaleX, scaleY);
    float vpW = aspectW * fitScale;
    float vpH = aspectH * fitScale;
    float vpX = (compW - vpW) * 0.5f;
    float vpY = (compH - vpH) * 0.5f;

    // Render the effect chain with letterbox viewport for final output
    auto renderStart = std::chrono::high_resolution_clock::now();

    GLuint sourceTexture = 0;

    // Priority: deck compositor > active source > loaded image
    if (deckActive)
    {
        // P18: provide audio snapshot to compositor for audio-reactive effects
        compositor_.setLatestSnapshot(snap);
        sourceTexture = compositor_.compositeDeck(*deck, shaderMgr_, quad_, time, realDt,
                                                   static_cast<int>(renderW),
                                                   static_cast<int>(renderH));

        // P21: Composite persistent layers from non-active decks
        if (composition_)
        {
            for (auto& otherDeck : composition_->decks)
            {
                if (&otherDeck == deck) continue; // Skip active deck
                compositor_.compositePersistentLayers(otherDeck, shaderMgr_, quad_, time, realDt,
                                                       static_cast<int>(renderW),
                                                       static_cast<int>(renderH));
            }
        }

        // Update persistent feedback buffer for feedback effects
        compositor_.updateFeedbackBuffer(shaderMgr_, quad_,
                                          static_cast<int>(renderW),
                                          static_cast<int>(renderH));

        // S166: Apply Composition::globalEffects to the fully-composited
        // frame now that the active deck AND any persistent layers from
        // other decks have both been written into the accumulator —
        // CompositorEngine.h's documented pipeline stage between layer
        // compositing and Master Opacity/output. Guarded on sourceTexture
        // != 0 so a deck with no active layers (compositeDeck returned 0)
        // does not run effects over nothing.
        if (composition_ && sourceTexture != 0)
        {
            sourceTexture = compositor_.applyGlobalEffects(composition_->globalEffects,
                                                             sourceTexture, shaderMgr_, quad_,
                                                             time,
                                                             static_cast<int>(renderW),
                                                             static_cast<int>(renderH));
        }
    }

    if (sourceTexture == 0 && sourceActive)
    {
        // Render the procedural source to get a texture. S167-L4b: scaledTime_,
        // not wall-clock `time` -- see its comment in Renderer.h.
        const auto* paramsPtr = currentSourceParams.empty() ? nullptr : &currentSourceParams;
        sourceTexture = renderSource(currentSourceType, static_cast<float>(scaledTime_),
                                      static_cast<int>(renderW), static_cast<int>(renderH),
                                      paramsPtr);
    }

    if (sourceTexture == 0)
    {
        sourceTexture = texMgr_.getImageTexture();
    }

    if (sourceTexture == 0)
    {
        // No content — clear to black to avoid ghosting from previous frames
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
        glViewport(0, 0, static_cast<int>(compW), static_cast<int>(compH));
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Still process pending frame capture (captures the black frame)
        processPendingCapture(compW, compH, 0, 0, compW, compH);
        return;
    }

    // P18: pass this renderer's own snapshot copy for audio-reactive
    // uniforms (W2: the shared parked snapshot is gone — each GL context
    // reads the bus itself) and this context's own GL state (W1).
    effectChain_.render(sourceTexture,
                        shaderMgr_, texMgr_, quad_,
                        effectChainGLState_, snap,
                        time, renderW, renderH,
                        static_cast<GLuint>(defaultFBO),
                        vpX, vpY, vpW, vpH);

    // P25: Apply composition-level transform (position, scale, rotation)
    applyCompTransform(static_cast<GLuint>(defaultFBO), vpX, vpY, vpW, vpH);

    // P25: Cross-deck transition blending
    if (composition_ && deckTransitionProgress_ < 1.0f)
    {
        int w = static_cast<int>(vpW);
        int h = static_cast<int>(vpH);
        if (w > 0 && h > 0 && prevDeckTexture_ != 0)
        {
            // Copy current framebuffer (new deck) to a temp texture
            ensureCompTransformFBO(w, h);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compTransformFBO_);
            glBlitFramebuffer(
                static_cast<int>(vpX), static_cast<int>(vpY),
                static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
                0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            // Draw the transition shader (old deck → new deck)
            glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
            glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                       static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));

            auto* prog = shaderMgr_.getProgram("deck_transition");
            if (prog)
            {
                prog->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, prevDeckTexture_);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, compTransformTexture_);
                glActiveTexture(GL_TEXTURE0);

                auto l = prog->getUniformIDFromName("u_textureA");
                if (l >= 0) glUniform1i(l, 0);
                l = prog->getUniformIDFromName("u_textureB");
                if (l >= 0) glUniform1i(l, 1);
                l = prog->getUniformIDFromName("u_progress");
                if (l >= 0) glUniform1f(l, deckTransitionProgress_);
                l = prog->getUniformIDFromName("u_blendMode");
                if (l >= 0) glUniform1i(l, static_cast<int>(composition_->crossfaderBlendMode));

                glDisable(GL_BLEND);
                quad_.draw();
            }
        }

        // Advance transition progress
        deckTransitionProgress_ += deckTransitionSpeed_;
        if (deckTransitionProgress_ >= 1.0f)
            deckTransitionProgress_ = 1.0f;
    }

    // P25: Detect deck switch and initiate transition
    if (composition_)
    {
        int currentDeckIdx = composition_->activeDeckIndex;
        if (currentDeckIdx != prevActiveDeckIndex_)
        {
            // Save the current framebuffer as the "outgoing" deck texture
            int w = static_cast<int>(vpW);
            int h = static_cast<int>(vpH);
            if (w > 0 && h > 0 && deckTransitionProgress_ >= 1.0f)
            {
                ensurePrevDeckFBO(w, h);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDeckFBO_);
                glBlitFramebuffer(
                    static_cast<int>(vpX), static_cast<int>(vpY),
                    static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
                    0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
            }

            // Start transition based on composition's transition speed
            float transSpeed = composition_->globalTransitionSpeed;
            if (transSpeed > 0.001f)
            {
                deckTransitionProgress_ = 0.0f;
                // Speed in progress-per-frame: 1.0 / (transSpeed * fps)
                // Assume ~60fps
                deckTransitionSpeed_ = 1.0f / (transSpeed * 60.0f);
            }
            else
            {
                // Instant cut
                deckTransitionProgress_ = 1.0f;
            }

            prevActiveDeckIndex_ = currentDeckIdx;
        }
    }

    // Apply master level (dim/blackout) using DST_COLOR blend to multiply
    float level = masterLevel_.load(std::memory_order_relaxed);
    if (level < 0.99f)
    {
        glEnable(GL_BLEND);
        // DST = DST * SRC — drawing a constant-color quad multiplies the framebuffer
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);

        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
        glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                   static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));

        // Use the brightness shader with the existing image texture as dummy
        auto* prog = shaderMgr_.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            // Bind any texture (required by passthrough shader)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sourceTexture);
        }

        // Set the constant blend color via glBlendColor
        glBlendFunc(GL_ZERO, GL_CONSTANT_COLOR);
        glBlendColor(level, level, level, 1.0f);

        quad_.draw();

        glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_BLEND);
    }

    // S167-L4b: apply Composition::masterOpacity to the fully-composited
    // frame -- the owner's "ceiling" ruling (final = master * layer * clip)
    // for the composition-wide fader. Same dim-to-black technique as the
    // masterLevel_ block just above (glBlendColor as a constant multiplier),
    // not an alpha-channel bake, because this runs against defaultFBO -- the
    // actual output framebuffer -- where Syphon/recording/capture below read
    // RGB, not alpha. UNCONDITIONAL: deliberately no "opacity ~= 1.0, skip"
    // early-return -- masterOpacity was silently render-dead all session
    // (.harmony/probe-deck-path.sh: accepted, echoed back, changed not one
    // pixel) and a skip-when-default guard here is exactly the shape of bug
    // that produced that. Runs BEFORE videoRecorder_->submitFrame,
    // publishSyphonFrame, and processPendingCapture below, so Master Opacity
    // also dims what leaves the app, not just the on-screen preview.
    if (composition_ != nullptr)
    {
        float masterOpacityVal = composition_->masterOpacity;
        glEnable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
        glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                   static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));

        auto* prog = shaderMgr_.getProgram("passthrough");
        if (prog)
        {
            prog->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sourceTexture);
        }

        glBlendFunc(GL_ZERO, GL_CONSTANT_COLOR);
        glBlendColor(masterOpacityVal, masterOpacityVal, masterOpacityVal, 1.0f);

        quad_.draw();

        glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_BLEND);
    }

    // P13.5.10: Use CPU-side timing instead of glFinish() which stalls the GPU pipeline.
    // This measures CPU-side render submission time, not GPU execution time.
    // For GPU timing, use GL_TIME_ELAPSED queries (async, no stall).
    auto renderEnd = std::chrono::high_resolution_clock::now();
    double frameMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();

    // EMA smoothing for UI display
    float prevMs = frameTimeMs_.load(std::memory_order_relaxed);
    frameTimeMs_.store(prevMs + 0.1f * (static_cast<float>(frameMs) - prevMs), std::memory_order_relaxed);

    // Adaptive quality: if sustained high frame times, disable heaviest effect
    if (frameMs > static_cast<double>(kFrameTimeBudgetMs))
    {
        if (++highFrameTimeCount_ >= kHighFrameTimeThreshold)
        {
            // Find the last enabled effect and disable it
            for (int i = effectChain_.getNumEffects() - 1; i >= 0; --i)
            {
                auto* fx = effectChain_.getEffect(i);
                if (fx != nullptr && fx->isEnabled())
                {
                    fx->setEnabled(false);
                    std::cerr << "[Renderer] Adaptive quality: disabled '"
                              << fx->getName() << "' (frame time "
                              << static_cast<int>(frameMs * 10) / 10.0 << "ms)" << std::endl;
                    break;
                }
            }
            highFrameTimeCount_ = 0;
        }
    }
    else
    {
        highFrameTimeCount_ = 0;
    }

    // Periodic log
    renderProfileAccum_ += frameMs;
    if (++renderProfileCount_ >= kRenderProfileInterval)
    {
        double avgMs = renderProfileAccum_ / kRenderProfileInterval;
        int numEnabled = 0;
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            if (auto* fx = effectChain_.getEffect(i))
                if (fx->isEnabled()) ++numEnabled;
        }
        std::cerr << "[Render Profile] Avg frame: " << static_cast<int>(avgMs * 100) / 100.0
                  << " ms, " << numEnabled << " effects active, "
                  << static_cast<int>(renderW) << "x" << static_cast<int>(renderH) << std::endl;
        renderProfileAccum_ = 0.0;
        renderProfileCount_ = 0;
    }

    // P22.6: Submit frame to video recorder (if recording)
    if (videoRecorder_ != nullptr)
        videoRecorder_->submitFrame(static_cast<int>(renderW), static_cast<int>(renderH));

    // P22.1: Publish the final composited frame to Syphon clients. Gated on the
    // enabled flag (set from the message thread) and initialization, so no GPU
    // work happens when Syphon is off or unavailable.
    if (syphonOutput_ != nullptr && syphonOutput_->isEnabled() && syphonOutput_->isInitialized())
        publishSyphonFrame(static_cast<GLuint>(defaultFBO), vpX, vpY, vpW, vpH);

    // Process pending frame capture (Eyes test harness + P22.7 snapshots)
    processPendingCapture(renderW, renderH, vpX, vpY, vpW, vpH);
}

void Renderer::openGLContextClosing()
{
    // Release all active procedural sources' GL resources, but do NOT destroy
    // the source objects themselves (do not activeSources_.clear() here).
    //
    // Originally this guarded MilkDropBrowser::presetManager_, which held a
    // raw interior pointer into ProjectMSource's by-value ProjectMPresetManager
    // (wired once at MainComponent construction) — destroying the source here
    // left that pointer dangling on the very next context close (e.g.
    // previewPanel_ hide/resize triggers a synchronous JUCE GL detach),
    // causing a use-after-free on the next browser paint/resize. The
    // 2026-09-04 autoload fix hoisted the manager OUT of ProjectMSource into
    // a MainComponent-owned member that outlives Renderer entirely, so that
    // dangling class is now structurally impossible for the manager (see
    // .harmony/milkdrop-autoload-rootcause.md).
    //
    // This rule REMAINS LOAD-BEARING for MilkDropBrowser::presetSelector_,
    // though: PresetSelector was deliberately NOT hoisted (it runs on the GL
    // thread inside ProjectMSource::render()) and stays a per-source member,
    // so destroying a source here would still dangle that pointer. Sources
    // now survive context close and lazily reinit their GL state on next
    // render() via the !glInitialized_ gate (see ProceduralSource::render,
    // ProjectMSource::render).
    for (auto& [id, src] : activeSources_)
        src->releaseGL();

    // Release all video player GL textures
    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        for (auto& [id, player] : videoPlayers_)
            player->releaseGL();
    }

    // Release all image sequence GL textures
    {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        for (auto& [id, seq] : imageSequences_)
            seq->releaseGL();
    }

    // Drain any media closeMediaForClip() retired but that hasn't been
    // through a renderOpenGL() frame yet (media-leak fix, L1 round 2). Reuse
    // drainRetiredMedia() rather than a second copy of its release loop —
    // this function runs on the GL thread with the SAME still-current
    // context drainRetiredMedia() requires, so it is a safe, ordinary call
    // here, not a special case. Without this, a retired-but-undrained player/
    // sequence would sit until the NEXT context's first renderOpenGL() call,
    // which would then call releaseGL() (glDeleteTextures) against a texture
    // ID that belonged to THIS (by then destroyed) context — the same
    // per-context-state class of bug EffectChainGLState::release() exists to
    // avoid (see the notebook 2026-08-02 entry on that class).
    drainRetiredMedia();

    compositor_.releaseGL();

    // W1: Release this context's EffectChain GL state (prevFrame FBO +
    // uniform location cache). The cache MUST die with the context — the
    // recreated context recompiles all programs, and stale program-ID-keyed
    // locations would poison lookups against the new programs.
    effectChainGLState_.release();

    // P25: Release composition transform FBO
    if (compTransformFBO_ != 0) { glDeleteFramebuffers(1, &compTransformFBO_); compTransformFBO_ = 0; }
    if (compTransformTexture_ != 0) { glDeleteTextures(1, &compTransformTexture_); compTransformTexture_ = 0; }
    // P25: Release previous deck FBO
    if (prevDeckFBO_ != 0) { glDeleteFramebuffers(1, &prevDeckFBO_); prevDeckFBO_ = 0; }
    if (prevDeckTexture_ != 0) { glDeleteTextures(1, &prevDeckTexture_); prevDeckTexture_ = 0; }

    // P22.1: Stop the Syphon server (must run on the GL thread while the context
    // is still alive) and release its blit FBO/texture.
    if (syphonOutput_ != nullptr)
        syphonOutput_->shutdown();
    if (syphonFBO_ != 0) { glDeleteFramebuffers(1, &syphonFBO_); syphonFBO_ = 0; }
    if (syphonTexture_ != 0) { glDeleteTextures(1, &syphonTexture_); syphonTexture_ = 0; }

    shaderMgr_.releaseAll();
    texMgr_.release();
    quad_.release();
}

ProceduralSource* Renderer::getOrCreateSource(const std::string& sourceId)
{
    // activeSources_ is GL-thread-owned (see the comment at its
    // declaration in Renderer.h). Marshal onto the GL thread when called
    // from elsewhere; run inline when already there (marshaling to self
    // would deadlock the blocking round-trip).
    if (juce::OpenGLContext::getCurrentContext() != &glContext_)
    {
        // Defense-in-depth (task #24, adjudicated with reviewer-shutdown-lane):
        // a detach-transition TOCTOU in JUCE's own teardown is REAL and
        // JUCE-inherent, not hypothetical. CachedImage::stop() (vendored
        // juce_OpenGLContext.cpp) checks workQueue.size() ONCE, then
        // pause() -> RenderThread::remove() sets flags.setSafe(false) as
        // its first action — the only drain path (renderFrame's
        // isListChanging() check) bails before servicing anything already
        // there. A BlockingWorker whose execute() call reads
        // pendingDestruction as still false, then lands its
        // workQueue.add()/triggerRepaint() AFTER stop()'s one-time check
        // but before remove() finishes, is never drained and its
        // WaitableEvent is never signaled — the calling thread hangs
        // forever. cc5c0c3 made this reachable: it moved GL detach to be
        // the FIRST statement in ~MainComponent(), before the HTTP servers
        // (whose worker threads are the only non-message-thread callers of
        // this method) are stopped, so a live HTTP request can now be
        // in-flight during the detach transition. Closed structurally by
        // reordering ~MainComponent() to stop those servers before
        // detaching (MISC lane); this isAttached() check is defense-in-depth
        // on top of that — it is itself a check-then-act read (a caller can
        // still lose the race to execute()'s own internal check), so it
        // narrows the window to JUCE's own already-narrow one rather than
        // being a second independent close.
        if (!glContext_.isAttached())
            return nullptr;

        ProceduralSource* result = nullptr;
        glContext_.executeOnGLThread([this, sourceId, &result](juce::OpenGLContext&)
        {
            result = getOrCreateSourceOnGLThread(sourceId);
        }, /*blockUntilFinished*/ true);
        return result;
    }

    return getOrCreateSourceOnGLThread(sourceId);
}

ProceduralSource* Renderer::getOrCreateSourceOnGLThread(const std::string& sourceId)
{
    auto it = activeSources_.find(sourceId);
    if (it != activeSources_.end())
        return it->second.get();

    auto source = sourceRegistry_.createSource(sourceId);
    if (!source)
        return nullptr;

    auto* ptr = source.get();
    activeSources_[sourceId] = std::move(source);

    // MilkDrop preset wiring (2026-09-04 fix — see
    // .harmony/milkdrop-autoload-rootcause.md). The preset MANAGER is pure
    // file/JSON scanning with no GL dependency; it's now MainComponent-owned
    // and scanned unconditionally at startup, so just inject the pointer
    // here — cheap, GL-thread-safe, no marshaling needed. The preset
    // SELECTOR stays a per-source, GL-thread member (PresetSelector::
    // processFrame runs inside ProjectMSource::render()) and must NOT be
    // hoisted, so instead notify listeners (the MilkDrop browser) that a
    // real source now exists to wire against — marshaled onto the message
    // thread since that wiring touches browser UI state, matching
    // onAutopilotAdvanced_/onGenreChanged_ above.
    if (auto* pmSource = dynamic_cast<ProjectMSource*>(ptr))
    {
        pmSource->setPresetManager(projectMPresetManager_);

        if (onProjectMSourceCreated_)
        {
            auto callback = onProjectMSourceCreated_;
            juce::MessageManager::callAsync([callback, pmSource]() { callback(pmSource); });
        }
    }

    return ptr;
}

void Renderer::rescanMilkDropPresets(const std::vector<std::string>& dirs)
{
    if (projectMPresetManager_ == nullptr)
        return;

    auto doRescan = [this, dirs]()
    {
        projectMPresetManager_->setPresetDirectories(dirs);
        projectMPresetManager_->rescan();
    };

    // No GL thread is running while the context is detached — no
    // ProjectMSource can be mid-processFrame — so it's safe to mutate
    // inline on the caller's thread. Mirrors the isAttached() defense-in-
    // depth check in getOrCreateSource() above.
    if (!glContext_.isAttached())
    {
        doRescan();
        return;
    }

    // presets_ is GL-thread-read (see the declaration comment in
    // Renderer.h). Marshal onto the GL thread when called from elsewhere
    // (the message thread, via Preferences); run inline when already
    // there — marshaling to self would deadlock the blocking round-trip,
    // same reasoning as getOrCreateSource() above.
    if (juce::OpenGLContext::getCurrentContext() != &glContext_)
    {
        glContext_.executeOnGLThread([&doRescan](juce::OpenGLContext&) { doRescan(); },
                                      /*blockUntilFinished*/ true);
        return;
    }

    doRescan();
}

void Renderer::toggleFavoritePreset(int index)
{
    if (projectMPresetManager_ == nullptr)
        return;

    auto doToggle = [this, index]()
    {
        projectMPresetManager_->toggleFavorite(index);
    };

    // No GL thread is running while the context is detached — no
    // ProjectMSource can be mid-processFrame — so it's safe to mutate
    // inline on the caller's thread. Mirrors rescanMilkDropPresets() above.
    if (!glContext_.isAttached())
    {
        doToggle();
        return;
    }

    // PresetInfo::favorite is GL-thread-read (see toggleFavoritePreset()'s
    // declaration comment in Renderer.h). Marshal onto the GL thread when
    // called from elsewhere (the message thread, via the preset browser's
    // right-click-to-favorite); run inline when already there — marshaling
    // to self would deadlock the blocking round-trip, same reasoning as
    // rescanMilkDropPresets() above.
    if (juce::OpenGLContext::getCurrentContext() != &glContext_)
    {
        glContext_.executeOnGLThread([&doToggle](juce::OpenGLContext&) { doToggle(); },
                                      /*blockUntilFinished*/ true);
        return;
    }

    doToggle();
}

GLuint Renderer::renderSource(const std::string& sourceId, float time, int width, int height,
                               const std::vector<Clip::SourceParam>* clipSourceParams)
{
    // P20: Layer Router — return another layer's saved output texture
    if (sourceId == "layer_router")
    {
        // Read the "Source Layer" param to determine which layer to route from
        float layerParam = 0.0f;
        if (clipSourceParams)
        {
            for (const auto& cp : *clipSourceParams)
            {
                if (cp.uniformName == "u_src_layer")
                {
                    layerParam = cp.value;
                    break;
                }
            }
        }
        // Map [0,1] to layer index. Assumes max ~10 layers.
        int layerIndex = static_cast<int>(layerParam * 9.0f + 0.5f);

        // Find layer ID from index in the active deck
        Deck* deck = activeDeck_.load(std::memory_order_acquire);
        if (deck)
        {
            if (layerIndex >= 0 && layerIndex < deck->getNumLayers())
            {
                uint32_t targetLayerId = deck->layers[static_cast<size_t>(layerIndex)].id;
                GLuint tex = compositor_.getLayerOutputTexture(targetLayerId);
                if (tex != 0)
                    return tex;
            }
        }
        return 0; // No layer output available yet
    }

    auto* source = getOrCreateSource(sourceId);
    if (!source)
        return 0;

    // Apply clip-level source parameters if provided
    if (clipSourceParams)
    {
        for (const auto& cp : *clipSourceParams)
        {
            for (int i = 0; i < source->getNumParams(); ++i)
            {
                if (source->getParam(i).uniformName == cp.uniformName)
                {
                    source->setParamValue(i, cp.value);
                    break;
                }
            }
        }
    }

    // Provide compositor feedback texture for sources that use it
    source->setFeedbackTexture(compositor_.getFeedbackTexture());

    // P20.5: Feed PCM audio to projectM sources
    if (sourceId == "projectm_visualizer" && analysisThread_)
    {
        if (auto* pmSource = dynamic_cast<ProjectMSource*>(source))
        {
            float pcmBuf[AnalysisThread::kPCMSnapshotSize];
            int count = analysisThread_->getPCMSamples(pcmBuf, AnalysisThread::kPCMSnapshotSize);
            if (count > 0)
                pmSource->feedAudio(pcmBuf, count);
        }
    }

    // Get latest audio snapshot for audio-reactive sources
    const FeatureSnapshot snap = featureBus_.read();

    return source->render(shaderMgr_, quad_, time, width, height, snap);
}

bool Renderer::openVideoForClip(uint32_t clipId, const juce::File& videoFile)
{
    auto player = std::make_unique<VideoPlayer>();
    if (!player->open(videoFile))
        return false;

    // Retire whatever media (video OR image-sequence) currently occupies
    // this clip id through closeMediaForClip()'s existing GL-thread-drained
    // retire list, instead of letting the operator[] assignment below
    // destroy a live VideoPlayer in place (message-thread destroy, L1-FU,
    // 2026-09 — same hazard class L1's retire list exists to fix, reached
    // via reconnect-on-replace (makeClipMediaHook) and "Replace Content"
    // rather than Clip>Clear). Only done AFTER the new player has opened
    // successfully, so a failed open leaves the currently-live media
    // untouched, matching this function's existing no-op-on-failure
    // contract.
    closeMediaForClip(clipId);

    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    videoPlayers_[clipId] = std::move(player);
    return true;
}

bool Renderer::openImageSequenceForClip(uint32_t clipId, const std::vector<juce::File>& files, float fps)
{
    auto seq = std::make_unique<ImageSequence>();
    seq->setFps(fps);
    if (!seq->open(files))
        return false;

    // See openVideoForClip's comment above — same reuse of the retire list,
    // and this direction also closes the "Replace Content on an
    // ImageSequence clip" leak (old imageSequences_[clipId] entry would
    // otherwise never be found by anything once the id starts being used
    // as a video id instead).
    closeMediaForClip(clipId);

    std::lock_guard<std::mutex> lock(imageSeqMutex_);
    imageSequences_[clipId] = std::move(seq);
    return true;
}

void Renderer::closeMediaForClip(uint32_t clipId)
{
    // May run on the message thread (undo/redo, Clear) with no GL context
    // current. close() itself is thread-safe (VideoPlayer::close() is
    // FFmpeg-only; ImageSequence::close() only resets CPU-side state — see
    // each class's threading-model comment), so it runs here immediately,
    // freeing the decoder right away. The unique_ptr is then handed to the
    // retire list instead of being erased-and-destructed in place: erasing
    // here would run ~VideoPlayer()/an eventual releaseGL() with no context
    // current (see the GL-THREAD DESTROY GUARD comment on the retire members
    // in Renderer.h). drainRetiredMedia() does the actual GL-thread release.
    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clipId);
        if (it != videoPlayers_.end())
        {
            it->second->close();
            std::lock_guard<std::mutex> retireLock(retiredMediaMutex_);
            retiredVideoPlayers_.push_back(std::move(it->second));
            videoPlayers_.erase(it);
        }
    }
    {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        auto it = imageSequences_.find(clipId);
        if (it != imageSequences_.end())
        {
            it->second->close();
            std::lock_guard<std::mutex> retireLock(retiredMediaMutex_);
            retiredImageSequences_.push_back(std::move(it->second));
            imageSequences_.erase(it);
        }
    }
}

void Renderer::drainRetiredMedia()
{
    // GL-thread only — called every frame from renderOpenGL() AND once more
    // from openGLContextClosing() (round 2 fix, so nothing retired-but-
    // undrained survives into the next context), both of which guarantee a
    // current GL context. Swap the retired lists out under lock so the
    // (possibly slow) GL teardown calls below never run while holding
    // retiredMediaMutex_ — closeMediaForClip() on the message thread only
    // needs that mutex for the brief hand-off, not for the whole drain.
    std::vector<std::unique_ptr<VideoPlayer>> videoToRetire;
    std::vector<std::unique_ptr<ImageSequence>> seqToRetire;
    {
        std::lock_guard<std::mutex> lock(retiredMediaMutex_);
        if (retiredVideoPlayers_.empty() && retiredImageSequences_.empty())
            return;
        videoToRetire.swap(retiredVideoPlayers_);
        seqToRetire.swap(retiredImageSequences_);
    }
    for (auto& player : videoToRetire) player->releaseGL();
    for (auto& seq : seqToRetire) seq->releaseGL();
    // videoToRetire/seqToRetire go out of scope here, destroying each player/
    // sequence. VideoPlayer's destructor re-runs close()+releaseGL() (both
    // already-idempotent no-ops at this point); ImageSequence's destructor
    // re-runs close() (also idempotent).
}

VideoPlayer* Renderer::getVideoPlayer(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    auto it = videoPlayers_.find(clipId);
    return (it != videoPlayers_.end()) ? it->second.get() : nullptr;
}

juce::File Renderer::getVideoPlayerFile(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    auto it = videoPlayers_.find(clipId);
    return (it != videoPlayers_.end()) ? it->second->getFile() : juce::File();
}

ImageSequence* Renderer::getImageSequence(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(imageSeqMutex_);
    auto it = imageSequences_.find(clipId);
    return (it != imageSequences_.end()) ? it->second.get() : nullptr;
}

GLuint Renderer::getVideoFrameTexture(const Clip* clip, float dt)
{
    if (!clip)
        return 0;

    // S167-L4b: composition-wide speed multiplier, folded in below via
    // effectiveClipSpeed() only for non-BPM-synced clips -- BPM-synced
    // transport stays tempo-locked, unaffected by masterSpeed.
    const float masterSpeedVal = (composition_ != nullptr) ? composition_->masterSpeed : 1.0f;

    if (clip->mediaType == Clip::MediaType::Video)
    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clip->id);
        if (it == videoPlayers_.end())
            return 0;

        auto* player = it->second.get();

        // Sync transport state from clip (only set playing if clip wants to play,
        // don't override if player stopped due to OneShot boundary)
        if (clip->playing && !player->isPlaying())
            player->setPlaying(true);
        else if (!clip->playing)
            player->setPlaying(false);

        // Only sync reverse for non-PingPong modes (PingPong manages direction internally)
        if (clip->loopMode != Clip::LoopMode::PingPong)
            player->setReverse(clip->reverse);

        switch (clip->loopMode)
        {
            case Clip::LoopMode::Loop:     player->setLoopMode(VideoPlayer::LoopMode::Loop); break;
            case Clip::LoopMode::PingPong: player->setLoopMode(VideoPlayer::LoopMode::PingPong); break;
            case Clip::LoopMode::OneShot:  player->setLoopMode(VideoPlayer::LoopMode::OneShot); break;
        }

        if (clip->transportMode == Clip::TransportMode::BPMSync)
        {
            // BPM Sync: adjust speed so video loops in beatDivision beats.
            const FeatureSnapshot snap = featureBus_.read();
            if (snap.bpm > 0.0f && clip->beatDivision > 0.0f)
            {
                // speed = videoBeats / beatDivision
                // e.g., 8-beat video over 4 beats = 2x speed
                player->setSpeed(clip->videoBeats / clip->beatDivision);
            }
        }
        else
        {
            player->setSpeed(effectiveClipSpeed(clip->speed, masterSpeedVal, false));
        }

        player->advanceFrame(static_cast<double>(dt));
        clip->playheadPosition = player->getPlayheadPosition();

        // Propagate player state back to clip model (OneShot stops, PingPong reverses)
        clip->playing = player->isPlaying();

        // Enforce in/out points
        if (clip->outPoint < 1.0f && clip->playheadPosition >= static_cast<double>(clip->outPoint))
        {
            if (clip->loopMode == Clip::LoopMode::OneShot)
            {
                clip->playing = false;
                player->setPlaying(false);
            }
            else
            {
                player->seekTo(static_cast<double>(clip->inPoint));
                clip->playheadPosition = static_cast<double>(clip->inPoint);
            }
        }

        return player->uploadToTexture();
    }
    else if (clip->mediaType == Clip::MediaType::ImageSequence)
    {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        auto it = imageSequences_.find(clip->id);
        if (it == imageSequences_.end())
            return 0;

        auto* seq = it->second.get();

        // Sync transport state from clip. S167-L4b: masterSpeed folds in only
        // when NOT BPM-synced (effectiveClipSpeed's isBpmSynced guard) --
        // ImageSequence::advanceFrame() (media/ImageSequence.cpp) uses
        // speed_ unconditionally, in BOTH transport modes, so this is the
        // one place that decision has to be made for the sequence path.
        seq->setSpeed(effectiveClipSpeed(clip->speed, masterSpeedVal,
                                          clip->transportMode == Clip::TransportMode::BPMSync));
        if (clip->loopMode != Clip::LoopMode::PingPong)
            seq->setReverse(clip->reverse);
        if (clip->playing && !seq->isPlaying())
            seq->setPlaying(true);
        else if (!clip->playing)
            seq->setPlaying(false);
        seq->setFps(clip->sequenceFps);
        switch (clip->loopMode)
        {
            case Clip::LoopMode::Loop:     seq->setLoopMode(ImageSequence::LoopMode::Loop); break;
            case Clip::LoopMode::PingPong: seq->setLoopMode(ImageSequence::LoopMode::PingPong); break;
            case Clip::LoopMode::OneShot:  seq->setLoopMode(ImageSequence::LoopMode::OneShot); break;
        }

        if (clip->transportMode == Clip::TransportMode::BPMSync)
        {
            // BPM Sync: cycle through images over beatDivision beats.
            const FeatureSnapshot snap = featureBus_.read();
            if (snap.bpm > 0.0f && clip->beatDivision > 0.0f)
            {
                int numFrames = seq->getFrameCount();
                if (numFrames > 0)
                {
                    // Cycle all frames over beatDivision beats,
                    // scaled by content beats ratio.
                    // FPS = numFrames * BPM / (beatDivision * 60)
                    float secondsPerCycle = clip->beatDivision * 60.0f / snap.bpm;
                    seq->setFps(static_cast<float>(numFrames) / secondsPerCycle);
                }
            }
            seq->advanceFrame(static_cast<double>(dt));
        }
        else
        {
            seq->advanceFrame(static_cast<double>(dt));
        }
        clip->playheadPosition = seq->getPlayheadPosition();
        clip->playing = seq->isPlaying();

        // Enforce in/out points
        if (clip->outPoint < 1.0f && clip->playheadPosition >= static_cast<double>(clip->outPoint))
        {
            if (clip->loopMode == Clip::LoopMode::OneShot)
            {
                clip->playing = false;
                seq->setPlaying(false);
            }
            else
            {
                seq->seekTo(static_cast<double>(clip->inPoint));
                clip->playheadPosition = static_cast<double>(clip->inPoint);
            }
        }

        return seq->getCurrentTexture();
    }

    return 0;
}

void Renderer::initShaders()
{
    compileAllShaders();
}

void Renderer::compileAllShaders()
{
    auto compile = [&](const juce::String& name, const char* frag) {
        if (shaderMgr_.compileProgram(name, EmbeddedShaders::vertex, frag))
            std::cerr << "[Renderer]   " << name << ": OK" << std::endl;
        else
            std::cerr << "[Renderer]   " << name << ": FAILED" << std::endl;
    };

    // Core
    compile("passthrough",          EmbeddedShaders::passthrough);
    compile("opacity_blend",        EmbeddedShaders::opacityBlend);
    compile("effect_dry_wet",       EmbeddedShaders::effectDryWet);
    compile("effect_drywet",        EmbeddedShaders::effectDryWet);
    compile("comp_transform",       EmbeddedShaders::compTransform);
    compile("deck_transition",      EmbeddedShaders::deckTransition);

    // Warp
    compile("ripple",               EmbeddedShaders::ripple);
    compile("bulge",                EmbeddedShaders::bulge);
    compile("wave",                 EmbeddedShaders::wave);
    compile("liquid",               EmbeddedShaders::liquid);
    compile("kaleidoscope",         EmbeddedShaders::kaleidoscope);
    compile("fisheye",              EmbeddedShaders::fisheye);
    compile("swirl",                EmbeddedShaders::swirl);

    // Color
    compile("hue_shift",            EmbeddedShaders::hueShift);
    compile("saturation",           EmbeddedShaders::saturation);
    compile("brightness",           EmbeddedShaders::brightness);
    compile("duotone",              EmbeddedShaders::duotone);
    compile("chromatic_aberration", EmbeddedShaders::chromaticAberration);
    compile("invert",               EmbeddedShaders::invert);
    compile("posterize",            EmbeddedShaders::posterize);
    compile("color_shift",          EmbeddedShaders::colorShift);
    compile("thermal",              EmbeddedShaders::thermal);
    compile("color_matrix",         EmbeddedShaders::colorMatrix);

    // Glitch
    compile("pixel_scatter",        EmbeddedShaders::pixelScatter);
    compile("rgb_split",            EmbeddedShaders::rgbSplit);
    compile("block_glitch",         EmbeddedShaders::blockGlitch);
    compile("scanlines",            EmbeddedShaders::scanlines);
    compile("digital_rain",         EmbeddedShaders::digitalRain);
    compile("noise_overlay",        EmbeddedShaders::noiseOverlay);
    compile("mirror",               EmbeddedShaders::mirror);
    compile("pixelate",             EmbeddedShaders::pixelate);

    // Blur/Post
    compile("gaussian_blur",        EmbeddedShaders::gaussianBlur);
    compile("zoom_blur",            EmbeddedShaders::zoomBlur);
    compile("shake",                EmbeddedShaders::shake);
    compile("vignette",             EmbeddedShaders::vignette);
    compile("motion_blur",          EmbeddedShaders::motionBlur);
    compile("glow",                 EmbeddedShaders::glow);
    compile("edge_detect",          EmbeddedShaders::edgeDetect);

    // 3D/Depth
    compile("perspective_tilt",     EmbeddedShaders::perspectiveTilt);
    compile("cylinder_wrap",        EmbeddedShaders::cylinderWrap);
    compile("sphere_wrap",          EmbeddedShaders::sphereWrap);
    compile("tunnel",               EmbeddedShaders::tunnel);
    compile("page_curl",            EmbeddedShaders::pageCurl);
    compile("parallax_layers",      EmbeddedShaders::parallaxLayers);

    // Additional Warp
    compile("polar_coords",         EmbeddedShaders::polarCoords);
    compile("twirl",                EmbeddedShaders::twirl);
    compile("shear",                EmbeddedShaders::shear);
    compile("elastic_bounce",       EmbeddedShaders::elasticBounce);
    compile("ripple_pond",          EmbeddedShaders::ripplePond);
    compile("diamond_distort",      EmbeddedShaders::diamondDistort);
    compile("barrel_distort",       EmbeddedShaders::barrelDistort);
    compile("sine_grid",            EmbeddedShaders::sineGrid);
    compile("glitch_displace",      EmbeddedShaders::glitchDisplace);

    // Additional Color
    compile("sepia",                EmbeddedShaders::sepia);
    compile("cross_process",        EmbeddedShaders::crossProcess);
    compile("split_tone",           EmbeddedShaders::splitTone);
    compile("color_halftone",       EmbeddedShaders::colorHalftone);
    compile("ordered_dither",       EmbeddedShaders::orderedDither);
    compile("heat_map",             EmbeddedShaders::heatMap);
    compile("selective_color",      EmbeddedShaders::selectiveColor);
    compile("film_grain",           EmbeddedShaders::filmGrain);
    compile("gamma_levels",         EmbeddedShaders::gammaLevels);
    compile("solarize",             EmbeddedShaders::solarize);

    // Pattern/Stylization
    compile("crt_simulation",       EmbeddedShaders::crtSimulation);
    compile("vhs_effect",           EmbeddedShaders::vhsEffect);
    compile("ascii_art",            EmbeddedShaders::asciiArt);
    compile("dot_matrix",           EmbeddedShaders::dotMatrix);
    compile("crosshatch",           EmbeddedShaders::crosshatch);
    compile("emboss",               EmbeddedShaders::emboss);
    compile("oil_paint",            EmbeddedShaders::oilPaint);
    compile("pencil_sketch",        EmbeddedShaders::pencilSketch);
    compile("voronoi_glass",        EmbeddedShaders::voronoiGlass);
    compile("cross_stitch",         EmbeddedShaders::crossStitch);
    compile("night_vision",         EmbeddedShaders::nightVision);

    // Animation
    compile("strobe",               EmbeddedShaders::strobe);
    compile("pulse",                EmbeddedShaders::pulse);
    compile("slit_scan",            EmbeddedShaders::slitScan);

    // Blend/Composite
    compile("double_exposure",      EmbeddedShaders::doubleExposure);
    compile("frosted_glass",        EmbeddedShaders::frostedGlass);
    compile("prism_refract",        EmbeddedShaders::prismRefract);
    compile("rain_on_glass",        EmbeddedShaders::rainOnGlass);
    compile("hexagonalize",         EmbeddedShaders::hexagonalize);

    // === Keying/transparency shaders (for keyboard launcher compositing) ===
    compile("key_alpha",            EmbeddedShaders::transparencyAlpha);
    compile("key_luma",             EmbeddedShaders::transparencyLumaKey);
    compile("key_chroma",           EmbeddedShaders::transparencyChromaKey);
    compile("key_max_rgb",          EmbeddedShaders::transparencyLight);  // max(R,G,B)

    // Reuse luma key shader with inverted semantics via uniforms for other modes
    // (the shader handles threshold direction, so inverted luma = just flip threshold)
    compile("key_inv_luma",         EmbeddedShaders::transparencyLumaKey);
    compile("key_luma_alpha",       EmbeddedShaders::transparencyLight);  // luminance → alpha
    compile("key_inv_luma_alpha",   EmbeddedShaders::transparencyAlpha);  // fallback
    compile("key_saturation",       EmbeddedShaders::transparencyLumaKey); // reuse with sat
    compile("key_edge",             EmbeddedShaders::transparencyAlpha);   // fallback
    compile("key_threshold",        EmbeddedShaders::transparencyAlpha);   // fallback
    compile("key_channel_r",        EmbeddedShaders::transparencyAlpha);   // fallback
    compile("key_channel_g",        EmbeddedShaders::transparencyAlpha);   // fallback
    compile("key_channel_b",        EmbeddedShaders::transparencyAlpha);   // fallback
    compile("key_vignette",         EmbeddedShaders::transparencyAlpha);   // fallback

    // === Procedural source shaders (Phase 10) ===
    compile("source_perlin_noise",          EmbeddedShaders::sourcePerlinNoise);
    compile("source_plasma",                EmbeddedShaders::sourcePlasma);
    compile("source_voronoi",               EmbeddedShaders::sourceVoronoi);
    compile("source_kaleido_fractal",       EmbeddedShaders::sourceKaleidoFractal);
    compile("source_mandelbrot",            EmbeddedShaders::sourceMandelbrot);
    compile("source_geometric_tunnel",      EmbeddedShaders::sourceGeometricTunnel);
    compile("source_color_gradient",        EmbeddedShaders::sourceColorGradient);
    compile("source_audio_waveform",        EmbeddedShaders::sourceAudioWaveform);
    compile("source_reaction_diffusion",    EmbeddedShaders::sourceReactionDiffusion);
    compile("source_cellular_automata",     EmbeddedShaders::sourceCellularAutomata);

    // Compositor utility shaders (P13.5)
    compile("layer_transform",              EmbeddedShaders::layer_transform);
    compile("mask_luminance",               EmbeddedShaders::mask_luminance);

    // === Phase 14: Quick-Win Effects (20 new effects) ===
    compile("greyscale",            EmbeddedShaders::greyscale);
    compile("threshold",            EmbeddedShaders::threshold);
    compile("exposure",             EmbeddedShaders::exposure);
    compile("vibrance",             EmbeddedShaders::vibrance);
    compile("quad_mirror",          EmbeddedShaders::quadMirror);
    compile("flip",                 EmbeddedShaders::flip);
    compile("warp_field",           EmbeddedShaders::warpField);
    compile("sharpen",              EmbeddedShaders::sharpen);
    compile("pixel_explosion",      EmbeddedShaders::pixelExplosion);
    compile("color_flash",          EmbeddedShaders::colorFlash);
    compile("slide_wrap",           EmbeddedShaders::slideWrap);
    compile("dot_field",            EmbeddedShaders::dotField);
    compile("triangulate",          EmbeddedShaders::triangulate);
    compile("auto_mask",            EmbeddedShaders::autoMask);
    compile("chromakey_effect",     EmbeddedShaders::chromakeyEffect);
    compile("tile_grid",            EmbeddedShaders::tileGrid);
    compile("spot_zoom",            EmbeddedShaders::spotZoom);
    compile("neon_edge",            EmbeddedShaders::neonEdge);
    compile("cartoon_ink",          EmbeddedShaders::cartoonInk);
    compile("pop_raster",           EmbeddedShaders::popRaster);

    // === Phase 15: Medium Effects (14 new effects) ===
    compile("palette_remap",        EmbeddedShaders::paletteRemap);
    compile("lut_grade",            EmbeddedShaders::lutGrade);
    compile("bendoscope",           EmbeddedShaders::bendoscope);
    compile("uv_remap",             EmbeddedShaders::uvRemap);
    compile("liquid_morph",         EmbeddedShaders::liquidMorph);
    compile("edge_blur",            EmbeddedShaders::edgeBlur);
    compile("brush_strokes",        EmbeddedShaders::brushStrokes);
    compile("fragment_burst",       EmbeddedShaders::fragmentBurst);
    compile("signal_destroy",       EmbeddedShaders::signalDestroy);
    compile("line_cloner",          EmbeddedShaders::lineCloner);
    compile("radial_cloner",        EmbeddedShaders::radialCloner);
    compile("cube_scatter",         EmbeddedShaders::cubeScatter);
    compile("infinite_zoom",        EmbeddedShaders::infiniteZoom);
    compile("bump_light",           EmbeddedShaders::bumpLight);
    compile("feedback",             EmbeddedShaders::feedback);
    compile("directional_feedback", EmbeddedShaders::directionalFeedback);

    // === Phase 15: Sources (12 new procedural sources) ===
    compile("source_solid_color",       EmbeddedShaders::sourceSolidColor);
    compile("source_strobe_light",      EmbeddedShaders::sourceStrobeLight);
    compile("source_checkerboard",      EmbeddedShaders::sourceCheckerboard);
    compile("source_line_pattern",      EmbeddedShaders::sourceLinePattern);
    compile("source_concentric_rings",  EmbeddedShaders::sourceConcentricRings);
    compile("source_sine_oscillator",   EmbeddedShaders::sourceSineOscillator);
    compile("source_spiral_pattern",    EmbeddedShaders::sourceSpiralPattern);
    compile("source_metaballs",         EmbeddedShaders::sourceMetaballs);
    compile("source_terrain_lines",     EmbeddedShaders::sourceTerrainLines);
    compile("source_shape_generator",   EmbeddedShaders::sourceShapeGenerator);
    compile("source_bump_light",        EmbeddedShaders::sourceBumpLight);
    compile("source_infinite_zoom",     EmbeddedShaders::sourceInfiniteZoom);
    compile("source_spiral_tunnel",    EmbeddedShaders::sourceSpiralTunnel);
    compile("source_wireframe_3d",     EmbeddedShaders::sourceWireframe3D);
    compile("source_line_generator",   EmbeddedShaders::sourceLineGenerator);
    compile("source_zigzag_lines",     EmbeddedShaders::sourceZigzagLines);
    compile("source_star_burst",       EmbeddedShaders::sourceStarBurst);
    compile("source_polygon_lines",    EmbeddedShaders::sourcePolygonLines);
    compile("source_waveform_lines",   EmbeddedShaders::sourceWaveformLines);
    compile("source_lissajous",        EmbeddedShaders::sourceLissajous);
    compile("source_spirograph",       EmbeddedShaders::sourceSpirograph);
    compile("source_angular_grid",     EmbeddedShaders::sourceAngularGrid);
    compile("source_fractal_tree",     EmbeddedShaders::sourceFractalTree);
    compile("source_laser_scan",       EmbeddedShaders::sourceLaserScan);
    compile("source_moire_lines",      EmbeddedShaders::sourceMoireLines);

    // Fractal sources
    compile("source_julia_set",       EmbeddedShaders::sourceJuliaSet);
    compile("source_burning_ship",    EmbeddedShaders::sourceBurningShip);
    compile("source_newton_fractal",  EmbeddedShaders::sourceNewtonFractal);
    compile("source_sierpinski",      EmbeddedShaders::sourceSierpinski);
    compile("source_apollonian",      EmbeddedShaders::sourceApollonian);

    // 3D Fractal sources (ray marched)
    compile("source_mandelbulb",      EmbeddedShaders::sourceMandelbulb);
    compile("source_menger_sponge",   EmbeddedShaders::sourceMengerSponge);
    compile("source_kifs",            EmbeddedShaders::sourceKIFS);

    // New 3D Fractal sources
    compile("source_julia_set_3d",    EmbeddedShaders::sourceJuliaSet3D);
    compile("source_burning_ship_3d", EmbeddedShaders::sourceBurningShip3D);
    compile("source_newton_3d",       EmbeddedShaders::sourceNewton3D);
    compile("source_sierpinski_tetra", EmbeddedShaders::sourceSierpinskiTetra);
    compile("source_apollonian_3d",   EmbeddedShaders::sourceApollonian3D);

    // Raymarched Torus / Tunnel sources
    compile("source_striped_torus",    EmbeddedShaders::sourceStripedTorus);
    compile("source_spiral_vortex",    EmbeddedShaders::sourceSpiralVortex);
    compile("source_checker_torus",    EmbeddedShaders::sourceCheckerTorus);
    compile("source_ribbed_vortex",    EmbeddedShaders::sourceRibbedVortex);
    compile("source_wormhole_tunnel",  EmbeddedShaders::sourceWormholeTunnel);
    compile("source_twisted_torus",    EmbeddedShaders::sourceTwistedTorus);
    compile("source_wormhole",         EmbeddedShaders::sourceWormhole);
    compile("source_torus_hole",       EmbeddedShaders::sourceTorusHole);

    // === Phase 17: Creative Sources (19 new sources) ===
    // Math sources
    compile("source_lissajous_weaver",   EmbeddedShaders::sourceLissajousWeaver);
    compile("source_fermat_spiral",      EmbeddedShaders::sourceFermatSpiral);
    compile("source_hyperbolic_tiling",  EmbeddedShaders::sourceHyperbolicTiling);
    compile("source_penrose_pulse",      EmbeddedShaders::sourcePenrosePulse);
    // Geometric sources
    compile("source_moire_interference", EmbeddedShaders::sourceMoireInterference);
    compile("source_astral_grid",        EmbeddedShaders::sourceAstralGrid);
    compile("source_radial_burst",       EmbeddedShaders::sourceRadialBurst);
    compile("source_hex_grid",           EmbeddedShaders::sourceHexGrid);
    compile("source_sacred_geometry",    EmbeddedShaders::sourceSacredGeometry);
    // 3D ray-marched sources
    compile("source_crystal_cavern",     EmbeddedShaders::sourceCrystalCavern);
    compile("source_infinite_corridor",  EmbeddedShaders::sourceInfiniteCorridor);
    compile("source_orbit_chamber",      EmbeddedShaders::sourceOrbitChamber);
    // Nature sources
    compile("source_fire_wall",          EmbeddedShaders::sourceFireWall);
    compile("source_water_caustics",     EmbeddedShaders::sourceWaterCaustics);
    compile("source_electric_arc",       EmbeddedShaders::sourceElectricArc);
    // Lighting sources
    compile("source_laser_scanner",      EmbeddedShaders::sourceLaserScanner);
    // ArKaos 3D sources
    compile("source_scroll_plane",       EmbeddedShaders::sourceScrollPlane);
    compile("source_rotating_cube_map",  EmbeddedShaders::sourceRotatingCubeMap);
    compile("source_dual_plane_drift",   EmbeddedShaders::sourceDualPlaneDrift);

    // === Phase 16: Time Effects (temporal) ===
    compile("ghost_trails",         EmbeddedShaders::ghostTrails);
    compile("frame_hold",           EmbeddedShaders::frameHold);
    compile("time_freeze",          EmbeddedShaders::timeFreeze);
    compile("screen_split",         EmbeddedShaders::screenSplit);
    compile("frame_delay",          EmbeddedShaders::frameDelay);

    // === Phase 16: Feedback blend shader (layer-level) ===
    compile("feedback_blend",       EmbeddedShaders::feedbackBlend);

    // === Phase 18: Audio-Native Effects ===
    compile("harmonic_displace",    EmbeddedShaders::harmonicDisplace);
    compile("timbral_mosaic",       EmbeddedShaders::timbralMosaic);
    compile("structural_morph",     EmbeddedShaders::structuralMorph);
    compile("pitch_chroma_shift",   EmbeddedShaders::pitchChromaShift);
    compile("key_palette",          EmbeddedShaders::keyPalette);
    compile("transient_flash",      EmbeddedShaders::transientFlash);
    compile("beat_ripple",          EmbeddedShaders::beatRipple);
    compile("rhythm_slice",         EmbeddedShaders::rhythmSlice);
    compile("density_wave",         EmbeddedShaders::densityWave);
    compile("chroma_dissolve",      EmbeddedShaders::chromaDissolve);

    // === Phase 18: Audio-Native Sources ===
    compile("source_spectrum_landscape",    EmbeddedShaders::sourceSpectrumLandscape);
    compile("source_chromatic_ring",        EmbeddedShaders::sourceChromaticRing);
    compile("source_band_tower",            EmbeddedShaders::sourceBandTower);
    compile("source_timbral_nebula",        EmbeddedShaders::sourceTimbralNebula);
    compile("source_structural_landscape",  EmbeddedShaders::sourceStructuralLandscape);
    compile("source_cymatics",              EmbeddedShaders::sourceCymatics);
    compile("source_spectral_waterfall",    EmbeddedShaders::sourceSpectralWaterfall);
    compile("source_spectral_ring",         EmbeddedShaders::sourceSpectralRing);
    compile("source_text_wall",             EmbeddedShaders::sourceTextWall);

    // === Phase 19: Complex Effects ===
    compile("luma_terrain",     EmbeddedShaders::lumaTerrain);
    compile("voxel_matrix",     EmbeddedShaders::voxelMatrix);
    compile("monitor_wall",     EmbeddedShaders::monitorWall);
    compile("drop_shadow",      EmbeddedShaders::dropShadow);
    compile("channel_delay",    EmbeddedShaders::channelDelay);
    compile("topo_lines",       EmbeddedShaders::topoLines);
    compile("data_corrupt",     EmbeddedShaders::dataCorrupt);
    compile("glitch_sort",      EmbeddedShaders::glitchSort);

    // === Phase 19: Remaining Sources ===
    compile("source_superformula",     EmbeddedShaders::sourceSuperformula);
    compile("source_truchet",          EmbeddedShaders::sourceTruchet);
    compile("source_rose",             EmbeddedShaders::sourceRoseCurves);
    compile("source_fibonacci",        EmbeddedShaders::sourceFibonacci);
    compile("source_lightning",        EmbeddedShaders::sourceLightning);
    compile("source_fire",             EmbeddedShaders::sourceFire);
    compile("source_starfield",        EmbeddedShaders::sourceStarfield);
    compile("source_nebula",           EmbeddedShaders::sourceParticleNebula);
    compile("source_radar",            EmbeddedShaders::sourceRadar);
    compile("source_glitch_grid",      EmbeddedShaders::sourceGlitchGrid);
    compile("source_dna",              EmbeddedShaders::sourceDNAHelix);
    compile("source_dot_matrix",       EmbeddedShaders::sourceDotMatrixWave);

    // === Phase 14: Transition Shaders (15 clip-to-clip transitions) ===
    compile("transition_dissolve",      EmbeddedShaders::transitionDissolve);
    compile("transition_wipe_left",     EmbeddedShaders::transitionWipeLeft);
    compile("transition_wipe_right",    EmbeddedShaders::transitionWipeRight);
    compile("transition_wipe_up",       EmbeddedShaders::transitionWipeUp);
    compile("transition_wipe_down",     EmbeddedShaders::transitionWipeDown);
    compile("transition_push_left",     EmbeddedShaders::transitionPushLeft);
    compile("transition_push_right",    EmbeddedShaders::transitionPushRight);
    compile("transition_push_up",       EmbeddedShaders::transitionPushUp);
    compile("transition_push_down",     EmbeddedShaders::transitionPushDown);
    compile("transition_zoom_in",       EmbeddedShaders::transitionZoomIn);
    compile("transition_zoom_out",      EmbeddedShaders::transitionZoomOut);
    compile("transition_iris",          EmbeddedShaders::transitionIris);
    compile("transition_flip_h",        EmbeddedShaders::transitionFlipH);
    compile("transition_cut",           EmbeddedShaders::transitionCut);
    compile("transition_fade_black",    EmbeddedShaders::transitionFadeBlack);

    // === Phase 20: System Sources ===
    compile("source_text_animator",     EmbeddedShaders::sourceTextAnimator);
    compile("source_strange_attractor", EmbeddedShaders::sourceStrangeAttractor);
    compile("source_gravity_well",      EmbeddedShaders::sourceGravityWell);
    compile("source_fluid_dynamics",    EmbeddedShaders::sourceFluidDynamics);
    compile("source_layer_router",      EmbeddedShaders::sourceLayerRouter);

    std::cerr << "[Renderer] All shaders compiled." << std::endl;
}

void Renderer::compileShaderWithUtils(const juce::String& name, const char* frag,
                                       bool needsNoise, bool needsSDF, bool needsUtil)
{
    // Build the fragment shader by prepending utility blocks before the main shader.
    // The #version directive must come first, so we extract it from the shader,
    // insert utilities after it, then append the rest.
    std::string fragStr(frag);
    std::string prefix;

    // Find and extract the #version line
    auto versionPos = fragStr.find("#version");
    std::string versionLine;
    std::string afterVersion;
    if (versionPos != std::string::npos)
    {
        auto lineEnd = fragStr.find('\n', versionPos);
        if (lineEnd == std::string::npos) lineEnd = fragStr.size();
        versionLine = fragStr.substr(versionPos, lineEnd - versionPos + 1);
        afterVersion = fragStr.substr(lineEnd + 1);
    }
    else
    {
        versionLine = "#version 410 core\n";
        afterVersion = fragStr;
    }

    std::string combined = versionLine;
    if (needsNoise) combined += std::string(EmbeddedShaders::glslNoiseFunctions);
    if (needsUtil)  combined += std::string(EmbeddedShaders::glslUtilFunctions);
    if (needsSDF)   combined += std::string(EmbeddedShaders::glslSDFFunctions);
    combined += afterVersion;

    if (shaderMgr_.compileProgram(name, EmbeddedShaders::vertex, combined.c_str()))
        std::cerr << "[Renderer]   " << name << ": OK (with utils)" << std::endl;
    else
        std::cerr << "[Renderer]   " << name << ": FAILED" << std::endl;
}

void Renderer::initEffectChain()
{
    // newOpenGLContextCreated() (which calls this) re-fires whenever the GL
    // context is closed and recreated during the app's life — e.g.
    // previewPanel_ hide/zero-size triggers a synchronous JUCE GL detach on
    // the message thread (see openGLContextClosing()), and the next
    // attach/resize recreates the context. Effects are plain CPU-side data
    // (no GL handles — see Effect.h), so they must be populated exactly
    // ONCE: re-running this on every recreation would silently duplicate
    // every effect in effectChain_ (EffectsRackPanel indexes effects_ by
    // position, so duplicates corrupt its UI and leak) each time the cycle
    // repeats. Guard against re-population.
    if (effectChain_.getNumEffects() > 0)
        return;

    // Load ALL effects from the EffectLibrary, organized by category
    // Effects are added in category order: warp, color, glitch, blur
    effectLibrary_.registerDefaults();

    static const juce::String categoryOrder[] = {
        "3d", "warp", "color", "glitch", "pattern", "animation", "blend", "blur",
        "time", "composite", "audio"
    };

    for (const auto& cat : categoryOrder)
    {
        auto names = effectLibrary_.getEffectsByCategory(cat);
        for (const auto& name : names)
        {
            auto effect = effectLibrary_.createEffect(name);
            if (effect)
            {
                // Start all effects disabled — user enables what they want
                effect->setEnabled(false);
                effectChain_.addEffect(std::move(effect));
            }
        }
    }

    std::cerr << "[Renderer] Loaded " << effectChain_.getNumEffects()
              << " effects from library." << std::endl;

    // No demo effects — user enables what they want via the FX browser.
    // A4 (outputwindow-arc-design.md): the mappingEngine_.clearAll() that
    // used to run here is DELETED. It was a GL-thread write to state the
    // codebase treats as message-thread-owned (see MappingEngine.h/A6), and
    // mappings can already exist before this first-attach guard ever fires
    // — TestServer constructs and starts listening in the MainComponent
    // ctor, in test mode, before the window is shown — so this clear could
    // wipe a mapping installed pre-attach. See the C3 work packet A4
    // ruling for the full analysis (no-op proof falsified; callAsync
    // fallback rejected as strictly worse).
}

// === Frame Capture (Eyes test harness) ===

bool Renderer::captureFrame(const juce::File& outputPath, float timeOverride,
                            int width, int height)
{
    // Clamp dimensions to safe max (avoid huge allocations)
    if (width > 1920) width = 1920;
    if (height > 1080) height = 1080;

    std::promise<bool> promise;
    auto future = promise.get_future();

    {
        std::lock_guard<std::mutex> lock(captureMutex_);
        captureOutputPath_ = outputPath;
        captureWidth_ = width;
        captureHeight_ = height;
        capturePromise_ = &promise;
        pendingCapture_.store(true, std::memory_order_release);
    }

    // Set time override for this frame
    float prevTime = timeOverride_.load(std::memory_order_relaxed);
    if (timeOverride >= 0.0f)
        timeOverride_.store(timeOverride, std::memory_order_relaxed);

    // Wait for GL thread to process (max 5 seconds)
    auto status = future.wait_for(std::chrono::seconds(5));

    // Restore time override
    timeOverride_.store(prevTime, std::memory_order_relaxed);

    if (status == std::future_status::timeout)
    {
        std::cerr << "[Eyes] Frame capture timed out after 5s" << std::endl;
        std::lock_guard<std::mutex> lock(captureMutex_);
        pendingCapture_.store(false, std::memory_order_relaxed);
        capturePromise_ = nullptr;
        return false;
    }

    return future.get();
}

void Renderer::processPendingCapture(float renderW, float renderH,
                                      float vpX, float vpY, float vpW, float vpH)
{
    // Quick check without lock (avoids lock contention on every frame)
    if (!pendingCapture_.load(std::memory_order_acquire))
        return;

    std::lock_guard<std::mutex> lock(captureMutex_);
    if (!pendingCapture_.load(std::memory_order_relaxed) || capturePromise_ == nullptr)
        return;

    std::cerr << "[Eyes] Processing capture: " << renderW << "x" << renderH
              << " vp=(" << vpX << "," << vpY << "," << vpW << "," << vpH << ")" << std::endl;

    // Determine capture area
    int readX = static_cast<int>(vpX);
    int readY = static_cast<int>(vpY);
    int readW = static_cast<int>(vpW > 0 ? vpW : renderW);
    int readH = static_cast<int>(vpH > 0 ? vpH : renderH);

    if (readW <= 0 || readH <= 0)
    {
        std::cerr << "[Eyes] Invalid capture dimensions: " << readW << "x" << readH << std::endl;
        capturePromise_->set_value(false);
        pendingCapture_.store(false, std::memory_order_relaxed);
        capturePromise_ = nullptr;
        return;
    }

    // Read pixels from the current framebuffer (default FBO after render)
    std::vector<uint8_t> pixels(static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4);
    glReadPixels(readX, readY, readW, readH, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    // Create JUCE image and copy pixels (flip vertically: GL origin is bottom-left)
    juce::Image img(juce::Image::ARGB, readW, readH, false);
    {
        juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < readH; ++y)
        {
            const auto* srcRow = pixels.data() + static_cast<size_t>(readH - 1 - y) * static_cast<size_t>(readW) * 4;
            for (int x = 0; x < readW; ++x)
            {
                bmp.setPixelColour(x, y,
                    juce::Colour(srcRow[x * 4],     // R
                                 srcRow[x * 4 + 1], // G
                                 srcRow[x * 4 + 2], // B
                                 srcRow[x * 4 + 3]  // A
                    ));
            }
        }
    }

    // Write PNG
    captureOutputPath_.getParentDirectory().createDirectory();
    juce::FileOutputStream fos(captureOutputPath_);
    bool ok = false;
    if (fos.openedOk())
    {
        juce::PNGImageFormat pngFormat;
        ok = pngFormat.writeImageToStream(img, fos);
    }

    if (ok)
        std::cerr << "[Eyes] Captured frame: " << captureOutputPath_.getFullPathName()
                  << " (" << readW << "x" << readH << ")" << std::endl;
    else
        std::cerr << "[Eyes] Failed to write PNG: " << captureOutputPath_.getFullPathName() << std::endl;

    capturePromise_->set_value(ok);
    pendingCapture_ = false;
    capturePromise_ = nullptr;
}

// P22.7: Take a snapshot to the snapshots directory
juce::File Renderer::takeSnapshot()
{
    // Ensure snapshot directory exists
    juce::File dir = snapshotDir_;
    if (dir == juce::File{})
        dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                  .getChildFile("Audio-DNA").getChildFile("Snapshots");
    dir.createDirectory();

    // Generate timestamped filename
    auto now = juce::Time::getCurrentTime();
    auto filename = "snapshot_" + now.formatted("%Y%m%d_%H%M%S") + ".png";
    auto outputFile = dir.getChildFile(filename);

    // Remove the 1920x1080 cap for user snapshots — use current render resolution
    bool ok = captureFrame(outputFile);

    if (ok)
    {
        std::cerr << "[Snapshot] Saved: " << outputFile.getFullPathName() << std::endl;
        if (onSnapshotTaken)
        {
            auto file = outputFile;
            auto cb = onSnapshotTaken;
            juce::MessageManager::callAsync([cb, file]() { cb(file); });
        }
        return outputFile;
    }

    std::cerr << "[Snapshot] Failed to save snapshot" << std::endl;
    return {};
}

// P25: Ensure composition transform FBO exists at the right size
void Renderer::ensureCompTransformFBO(int width, int height)
{
    if (compTransformTexture_ != 0 && compTransformWidth_ == width && compTransformHeight_ == height)
        return;

    if (compTransformFBO_ != 0) glDeleteFramebuffers(1, &compTransformFBO_);
    if (compTransformTexture_ != 0) glDeleteTextures(1, &compTransformTexture_);

    glGenTextures(1, &compTransformTexture_);
    glBindTexture(GL_TEXTURE_2D, compTransformTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &compTransformFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, compTransformFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compTransformTexture_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    compTransformWidth_ = width;
    compTransformHeight_ = height;
}

// P25: Apply composition-level transform (position, scale, rotation)
void Renderer::applyCompTransform(GLuint defaultFBO, float vpX, float vpY, float vpW, float vpH)
{
    if (!composition_) return;

    // Check if any transform is non-default
    float posX = composition_->compPositionX;
    float posY = composition_->compPositionY;
    float scale = composition_->compScale;
    float rotation = composition_->compRotation;
    float anchorX = composition_->compAnchorX;
    float anchorY = composition_->compAnchorY;

    bool isDefault = (std::abs(posX) < 0.001f && std::abs(posY) < 0.001f &&
                      std::abs(scale - 1.0f) < 0.001f && std::abs(rotation) < 0.01f);
    if (isDefault) return;

    int w = static_cast<int>(vpW);
    int h = static_cast<int>(vpH);
    if (w <= 0 || h <= 0) return;

    ensureCompTransformFBO(w, h);

    // Copy current framebuffer content to the transform texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compTransformFBO_);
    glBlitFramebuffer(
        static_cast<int>(vpX), static_cast<int>(vpY),
        static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Now render the transform shader to the default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
               static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto* prog = shaderMgr_.getProgram("comp_transform");
    if (!prog) return;

    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, compTransformTexture_);

    auto loc = [&](const char* name) {
        return prog->getUniformIDFromName(name);
    };

    auto l = loc("u_texture");
    if (l >= 0) glUniform1i(l, 0);
    l = loc("u_comp_position");
    if (l >= 0) glUniform2f(l, posX, posY);
    l = loc("u_comp_scale");
    if (l >= 0) glUniform1f(l, scale);
    l = loc("u_comp_rotation");
    // Convert degrees to radians
    if (l >= 0) glUniform1f(l, rotation * 3.14159265f / 180.0f);
    l = loc("u_comp_anchor");
    if (l >= 0) glUniform2f(l, anchorX, anchorY);

    glDisable(GL_BLEND);
    quad_.draw();
}

// P25: Ensure previous deck FBO exists at the right size
void Renderer::ensurePrevDeckFBO(int width, int height)
{
    if (prevDeckTexture_ != 0 && prevDeckWidth_ == width && prevDeckHeight_ == height)
        return;

    if (prevDeckFBO_ != 0) glDeleteFramebuffers(1, &prevDeckFBO_);
    if (prevDeckTexture_ != 0) glDeleteTextures(1, &prevDeckTexture_);

    glGenTextures(1, &prevDeckTexture_);
    glBindTexture(GL_TEXTURE_2D, prevDeckTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &prevDeckFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, prevDeckFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevDeckTexture_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    prevDeckWidth_ = width;
    prevDeckHeight_ = height;
}

// P22.1: Ensure the Syphon publish FBO/texture exists at the right size
void Renderer::ensureSyphonFBO(int width, int height)
{
    if (syphonTexture_ != 0 && syphonWidth_ == width && syphonHeight_ == height)
        return;

    if (syphonFBO_ != 0) glDeleteFramebuffers(1, &syphonFBO_);
    if (syphonTexture_ != 0) glDeleteTextures(1, &syphonTexture_);

    glGenTextures(1, &syphonTexture_);
    glBindTexture(GL_TEXTURE_2D, syphonTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &syphonFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, syphonFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, syphonTexture_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    syphonWidth_ = width;
    syphonHeight_ = height;
}

// P22.1: Blit the final composited output (the letterboxed viewport region of
// the default framebuffer) into the Syphon texture and publish it to clients.
void Renderer::publishSyphonFrame(GLuint defaultFBO, float vpX, float vpY, float vpW, float vpH)
{
    int w = static_cast<int>(vpW);
    int h = static_cast<int>(vpH);
    if (w <= 0 || h <= 0)
        return;

    ensureSyphonFBO(w, h);

    // Copy the final-output region into the Syphon texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, syphonFBO_);
    glBlitFramebuffer(
        static_cast<int>(vpX), static_cast<int>(vpY),
        static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Restore the default framebuffer so any subsequent readback (frame capture)
    // reads from the correct target.
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

    // Publish to connected Syphon clients (internally no-op if disabled/uninitialized)
    syphonOutput_->publishTexture(syphonTexture_, w, h);
}
