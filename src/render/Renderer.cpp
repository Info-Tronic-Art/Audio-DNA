#include "Renderer.h"
#include "render/EmbeddedShaders.h"
#include <iostream>
#include <chrono>
#include <string>

using namespace juce::gl;
Renderer::Renderer(FeatureBus& featureBus)
    : featureBus_(featureBus)
{
}

Renderer::~Renderer()
{
    detach();
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
    initEffectChain();
    compositor_.initGL(1920, 1080); // Will resize as needed
    compositor_.setEffectLibrary(&effectLibrary_);

    // Wire source rendering into compositor
    compositor_.setSourceRenderer([this](const std::string& sourceId, float time, int w, int h,
                                         const std::vector<Clip::SourceParam>* params) -> GLuint {
        return renderSource(sourceId, time, w, h, params);
    });

    // Wire video frame provider into compositor
    compositor_.setVideoFrameProvider([this](const Clip* clip, float dt) -> GLuint {
        return getVideoFrameTexture(clip, dt);
    });

    startTime_ = juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void Renderer::renderOpenGL()
{
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
        return; // Nothing to render yet

    // Read latest audio features (lock-free)
    const FeatureSnapshot* snap = featureBus_.acquireRead();
    if (snap == nullptr)
        snap = featureBus_.getLatestRead();

    // Build a default snapshot if none available
    FeatureSnapshot defaultSnap;
    if (snap == nullptr)
        snap = &defaultSnap;

    // Apply audio→effect mappings via MappingEngine
    mappingEngine_.processFrame(*snap, effectChain_);

    // P13.5.9: Process autopilot (beat-synced clip advancement + beat snap)
    if (deckActive)
    {
        bool clipAdvanced = autopilot_.processFrame(*deck, *snap);
        if (clipAdvanced && onAutopilotAdvanced_)
        {
            // Notify UI thread to refresh deck view
            auto callback = onAutopilotAdvanced_;
            juce::MessageManager::callAsync([callback]() { callback(); });
        }
    }

    // Calculate time
    float time = static_cast<float>(
        juce::Time::getMillisecondCounterHiRes() / 1000.0 - startTime_);

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
        sourceTexture = compositor_.compositeDeck(*deck, shaderMgr_, quad_, time,
                                                   static_cast<int>(renderW),
                                                   static_cast<int>(renderH));
    }

    if (sourceTexture == 0 && sourceActive)
    {
        // Render the procedural source to get a texture
        const auto* paramsPtr = currentSourceParams.empty() ? nullptr : &currentSourceParams;
        sourceTexture = renderSource(currentSourceType, time,
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
        return;
    }

    effectChain_.render(sourceTexture,
                        shaderMgr_, texMgr_, quad_,
                        time, renderW, renderH,
                        static_cast<GLuint>(defaultFBO),
                        vpX, vpY, vpW, vpH);

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
}

void Renderer::openGLContextClosing()
{
    // Release all active procedural sources
    for (auto& [id, src] : activeSources_)
        src->releaseGL();
    activeSources_.clear();

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

    compositor_.releaseGL();
    shaderMgr_.releaseAll();
    texMgr_.release();
    quad_.release();
}

ProceduralSource* Renderer::getOrCreateSource(const std::string& sourceId)
{
    auto it = activeSources_.find(sourceId);
    if (it != activeSources_.end())
        return it->second.get();

    auto source = sourceRegistry_.createSource(sourceId);
    if (!source)
        return nullptr;

    auto* ptr = source.get();
    activeSources_[sourceId] = std::move(source);
    return ptr;
}

GLuint Renderer::renderSource(const std::string& sourceId, float time, int width, int height,
                               const std::vector<Clip::SourceParam>* clipSourceParams)
{
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

    // Get latest audio snapshot for audio-reactive sources
    const FeatureSnapshot* snap = featureBus_.acquireRead();
    if (!snap) snap = featureBus_.getLatestRead();

    FeatureSnapshot defaultSnap;
    if (!snap) snap = &defaultSnap;

    return source->render(shaderMgr_, quad_, time, width, height, *snap);
}

bool Renderer::openVideoForClip(uint32_t clipId, const juce::File& videoFile)
{
    auto player = std::make_unique<VideoPlayer>();
    if (!player->open(videoFile))
        return false;

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

    std::lock_guard<std::mutex> lock(imageSeqMutex_);
    imageSequences_[clipId] = std::move(seq);
    return true;
}

void Renderer::closeMediaForClip(uint32_t clipId)
{
    {
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        auto it = videoPlayers_.find(clipId);
        if (it != videoPlayers_.end())
        {
            it->second->close();
            videoPlayers_.erase(it);
        }
    }
    {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        auto it = imageSequences_.find(clipId);
        if (it != imageSequences_.end())
        {
            it->second->close();
            imageSequences_.erase(it);
        }
    }
}

VideoPlayer* Renderer::getVideoPlayer(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    auto it = videoPlayers_.find(clipId);
    return (it != videoPlayers_.end()) ? it->second.get() : nullptr;
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
            const FeatureSnapshot* snap = featureBus_.acquireRead();
            if (!snap) snap = featureBus_.getLatestRead();
            if (snap && snap->bpm > 0.0f && clip->beatDivision > 0.0f)
            {
                // speed = videoBeats / beatDivision
                // e.g., 8-beat video over 4 beats = 2x speed
                player->setSpeed(clip->videoBeats / clip->beatDivision);
            }
        }
        else
        {
            player->setSpeed(clip->speed);
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

        // Sync transport state from clip
        seq->setSpeed(clip->speed);
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
            const FeatureSnapshot* snap = featureBus_.acquireRead();
            if (!snap) snap = featureBus_.getLatestRead();
            if (snap && snap->bpm > 0.0f && clip->beatDivision > 0.0f)
            {
                int numFrames = seq->getFrameCount();
                if (numFrames > 0)
                {
                    // Cycle all frames over beatDivision beats,
                    // scaled by content beats ratio.
                    // FPS = numFrames * BPM / (beatDivision * 60)
                    float secondsPerCycle = clip->beatDivision * 60.0f / snap->bpm;
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
    compile("effect_drywet",        EmbeddedShaders::effectDryWet);

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

    // No demo effects or mappings — user enables what they want via the FX browser
    mappingEngine_.clearAll();
}
