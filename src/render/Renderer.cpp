#include "Renderer.h"
#include "render/EmbeddedShaders.h"
#include "render/DeckClock.h"
#include "render/PixelConvert.h"
#include "render/PngWrite.h"
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
    legacyReq_ = LegacyRequest{ imageFile, false, legacyReq_.gen + 1 };
}

void Renderer::clearImage()
{
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    legacyReq_ = LegacyRequest{ juce::File(), true, legacyReq_.gen + 1 };
}

void Renderer::prefetchLegacyImage(const juce::File& imageFile)
{
    std::lock_guard<std::mutex> lock(pendingImageMutex_);
    legacyPrefetch_ = imageFile;
}

// s-rta-0928 R1.3: the frame top -- take the newest request and prefetch path, sort the arrived decode results. O(1)
// apart from moving results; never decodes (it used to decode + convert + upload here, on EVERY trigger or selection
// of an image clip, even in deck mode where this picture is not shown -- outside the frame timer, F2).
void Renderer::takeLegacyRequests()
{
    LegacyRequest req;
    juce::File prefetch;
    {
        std::lock_guard<std::mutex> lock(pendingImageMutex_);
        req = legacyReq_;
        prefetch = legacyPrefetch_;
        legacyPrefetch_ = juce::File();
    }

    std::vector<ImageDecode::Result> got;
    legacyBox_->tryDrain(got);
    for (auto& r : got)
    {
        if (r.tag == kLegacyPrefetchTag)
        {
            if (r.path == legacyPrefetchInFlight_)
                legacyPrefetchInFlight_.clear();
            if (r.kind == ImageTexCache::Kind::Decoded && r.path != legacyResidentPath_)
                legacyParked_ = std::move(r);
            continue;
        }
        if (r.tag != legacyInFlightGen_)
            continue;                  // stale (a context loss reset the gens)
        legacyInFlightGen_ = 0;
        if (r.tag == legacyWantGen_ && !legacyWantDone_)
            legacyReady_ = std::move(r);
        // else: an older request's result -- the newest is issued by resolveLegacy
    }

    if (req.gen != legacyHandledGen_)
    {
        legacyHandledGen_ = req.gen;
        legacyReady_.reset();
        if (req.clear)
        {
            texMgr_.release();
            legacyResidentPath_.clear();
            legacyWant_ = juce::File();
            legacyWantGen_ = 0;
            legacyWantDone_ = true;
        }
        else
        {
            legacyWant_ = req.file;      // lazy: decoded + uploaded only when a frame needs it (resolveLegacy)
            legacyWantGen_ = req.gen;
            legacyWantDone_ = false;
        }
    }

    const auto pp = prefetch.getFullPathName().toStdString();
    if (prefetch != juce::File() && pp != legacyResidentPath_ && pp != legacyPrefetchInFlight_
        && !(legacyParked_ && legacyParked_->path == pp))
    {
        legacyPrefetchInFlight_ = pp;
        imageDecoder_.request(prefetch, ImageDecode::Layout::PremultipliedRGBA, kLegacyPrefetchTag, std::nullopt,
                              legacyBox_);
    }
}

// s-rta-0928 R1.3: called ONLY where a frame shows the legacy single image (no deck picture, no source). A wanted file
// that is not resident is requested (one demand decode in flight), adopted from the prefetch, or uploaded when its
// bytes are here (within the frame's upload budget). Until then legacyPendingThisFrame_ holds a render_frame back and
// the current texture (the previous picture) or black shows. Unchanged (the resident file, same stamp) and Failed
// (the old picture stays, as a failed load always did) end the request.
void Renderer::resolveLegacy()
{
    if (legacyWantDone_ || legacyWantGen_ == 0)
        return;
    const auto path = legacyWant_.getFullPathName().toStdString();
    if (!legacyReady_ && legacyParked_ && legacyParked_->path == path)
    {
        legacyReady_ = std::move(legacyParked_);
        legacyParked_.reset();
    }
    if (legacyReady_)
    {
        if (legacyReady_->kind == ImageTexCache::Kind::Decoded)
        {
            if (!uploadBudget_.take(legacyReady_->rgba.size()))
            {
                legacyPendingThisFrame_ = true;   // this frame's budget is spent: the next frame uploads
                return;
            }
            texMgr_.uploadPixels(legacyReady_->rgba.data(), legacyReady_->w, legacyReady_->h);
            legacyResidentPath_ = legacyReady_->path;
            legacyResidentStamp_ = legacyReady_->stamp;
        }
        legacyReady_.reset();
        legacyWantDone_ = true;
        return;
    }
    if (legacyInFlightGen_ == 0)
    {
        std::optional<ImageTexCache::Stamp> known;
        if (path == legacyResidentPath_ && texMgr_.hasImage())
            known = legacyResidentStamp_;   // a re-load of the resident file: a stat, no decode
        legacyInFlightGen_ = legacyWantGen_;
        imageDecoder_.request(legacyWant_, ImageDecode::Layout::PremultipliedRGBA, legacyWantGen_, known, legacyBox_);
    }
    legacyPendingThisFrame_ = true;
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

void Renderer::updateActiveSourceParamsFor(const std::string& sourceType,
                                            const std::vector<Clip::SourceParam>& params)
{
    std::lock_guard<std::mutex> lock(activeSourceMutex_);
    if (!hasActiveSource_ || activeSourceType_ != sourceType)
        return;
    activeSourceParams_ = params;
}

void Renderer::newOpenGLContextCreated()
{
    std::cerr << "[Renderer] GL context created. Version: "
              << glGetString(GL_VERSION) << std::endl;

    quad_.init();
    initShaders();

    // Onset render-path fix: a (re-)attached context starts looking afresh -- no stale-delta
    // flash for onsets that happened while it was detached.
    onsetPulse_.reset();

    // W7(iv) outputwindow-arc: one-shot per-context program-ID log (its
    // [OutputRenderer] counterpart went with that renderer, s-rta-0927
    // outputs-c1: the output window compiles no programs any more).
    for (const char* name : { "passthrough", "hue_shift", "vignette" })
        if (auto* p = shaderMgr_.getProgram(name))
            std::cerr << "[Renderer] programID(" << name << ")="
                      << p->getProgramID() << std::endl;

    initEffectChain();
    compositor_.initGL(1920, 1080); // Will resize as needed
    compositor_.setEffectLibrary(&effectLibrary_);
    compositor_.setImageDecoder(&imageDecoder_, &uploadBudget_);   // s-rta-0928 R1.2

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
    compositor_.setVideoFrameProvider([this](const Clip* clip, float dt, bool* pending) -> GLuint {
        return getVideoFrameTexture(clip, dt, pending);
    });
    // renderleft-fix: C1's crossfade pause for an image sequence whose first frame still decodes (no side effect).
    compositor_.setSequencePendingProvider([this](const Clip* clip) -> bool {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        auto it = imageSequences_.find(clip->id);
        return it != imageSequences_.end() && it->second->firstFramePending();
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
    // s-rta-0928 R1.0: the WHOLE callback (every return), incl. the work before renderStart (pending legacy image,
    // camera upload, autopilot) and after renderEnd (recorder, Syphon, capture read) that peak_frame_time_ms misses.
    struct CallbackPeak
    {
        std::atomic<float>& peak;
        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        ~CallbackPeak()
        {
            const float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
            if (ms > peak.load(std::memory_order_relaxed)) peak.store(ms, std::memory_order_relaxed);
        }
    } callbackPeak{ peakCallbackMs_ };

    // Before anything reads timeOverride_: which captures were armed when this frame started (captureArmSeq_).
    frameArmSeq_ = captureArmSeq_.load(std::memory_order_acquire);

    // s-rta-0928 R1.2: this frame's image bookkeeping, then the decoded images uploaded -- EVERY frame, before any
    // pass and before any early return (B2), within the frame's upload budget. No decode here, ever.
    compositor_.beginFrame();
    uploadBudget_.reset();
    videoStats_.pendingNow.store(0, std::memory_order_relaxed);   // s-rta-0928b video: videos_pending is per frame
    compositor_.pumpImages();

    // Release any media players closeMediaForClip() retired from the message
    // thread (media-leak fix, L1) — the only place this runs, since this
    // function is guaranteed to execute on the GL thread with a context
    // current. s-rta-0928b seqvram F2: the frame's texture-delete budget first (the drain, the idle trim and the
    // sequences' shrinks share it).
    seqDeletes_.reset();
    drainRetiredMedia();
    // s-rta-0928b seqvram: the image sequences' texture bytes, every frame, before any early return.
    scanSequenceVram();

    // Handle pending image load or clear (from message thread). s-rta-0928 R1.3: O(1) -- the decode runs off the GL
    // thread and only when a frame needs the legacy image (resolveLegacy below).
    legacyPendingThisFrame_ = false;
    takeLegacyRequests();

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

    // Read latest audio features (R5: coherent caller-owned value copy).
    // Onset render-path fix: read the bus FIRST (before the early return below) so idle
    // frames keep the pulse baseline current, then derive this frame's pulse from the
    // monotonic onsetCount delta. frameSnap_.onsetDetected now means "at least one onset
    // since this context's previous frame" -- loss-free below the analysis rate,
    // duplicate-free above it -- and is the ONE snapshot every uploader sees this frame
    // (see Renderer.h).
    frameSnap_ = featureBus_.read();
    frameSnap_.onsetDetected = onsetPulse_.consume(frameSnap_.onsetCount) > 0u;
    if (frameSnap_.onsetDetected)
        onsetPulseFrames_.fetch_add(1u, std::memory_order_relaxed);
    const FeatureSnapshot& snap = frameSnap_;

    // === s-rta-0926b plan4 item 1: the composition canvas ===
    // Boris 2026-09-26: "the preview and output display window in the lower left corner should not
    // change aspect ratios. they should be what the composition is setup for". The frame renders ONCE,
    // offscreen, into canvasFBO_ at the composition's size; the window (the lower-left panel) only
    // presents it, letter/pillar-boxed (presentCanvas, the last pass), and every output -- recorder,
    // Syphon, render_frame, snapshots -- reads the canvas. This block runs before the first early
    // return, and ensureCanvasFBO runs before any pass (R5).
    GLint defaultFBO = 0;   // the window's framebuffer: cleared to the bar colour above
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
    GpuTimerScope gpuTimer{ *this, beginGpuTimer() };

    // The PRESENT size: the target component's physical pixels.
    auto* component = glContext_.getTargetComponent();
    float scale = static_cast<float>(glContext_.getRenderingScale());
    float compW = component != nullptr ? static_cast<float>(component->getWidth())  * scale : 1.0f;
    float compH = component != nullptr ? static_cast<float>(component->getHeight()) * scale : 1.0f;

    {
        // Plain-int reads of message-thread-written fields: the house class (globalTransitionSpeed and
        // activeDeckIndex below). The debounce removes the one new hazard -- a half-applied pair (the
        // Composition inspector writes width, then height) reallocating everything for one frame.
        const int reqW = composition_ != nullptr ? composition_->outputWidth : 0;
        const int reqH = composition_ != nullptr ? composition_->outputHeight : 0;
        if (reqW != candW_ || reqH != candH_) { candW_ = reqW; candH_ = reqH; }   // first sight: wait a frame
        else                                  { stableW_ = candW_; stableH_ = candH_; }
    }
    const uint64_t lockPacked = lockedSize_.load(std::memory_order_relaxed);   // ONE load: never half-applied
    const RenderGeometry::Size canvas = RenderGeometry::resolveCanvas(
        static_cast<int>(static_cast<uint32_t>(lockPacked >> 32)), static_cast<int>(static_cast<uint32_t>(lockPacked)),
        stableW_, stableH_);

    // P25: Detect a deck switch and start the cross-deck transition. s-rta-0926b plan4 F2: detected HERE, at the
    // top of the frame, because only here does the canvas still hold the previous frame -- the outgoing deck's
    // last picture, exactly what was on screen (mid-transition too). Detected after the composite (as it used to
    // be), the "outgoing" copy was the NEW deck's first frame and every deck transition was a cut. Blitted
    // (scaled if the canvas size changes this frame) into prevDeckFBO_ before ensureCanvasFBO / the clear (R5).
    // activeDeckIndex is read after the acquire-load of activeDeck_ above: a frame that already renders the new
    // deck always sees the new index.
    if (composition_ != nullptr)
    {
        const int currentDeckIdx = composition_->activeDeckIndex;
        if (currentDeckIdx != prevActiveDeckIndex_)
        {
            // globalTransitionSpeed is a DURATION in seconds (see its comment in Composition.h), same
            // misleading-name pattern as Layer::transitionSpeed. S167-L4b DT-FIX: progress-per-SECOND
            // (1.0 / duration), multiplied by the real measured dt each frame -- not a hardcoded assume-60fps
            // progress-per-frame constant.
            const float transSpeed = composition_->globalTransitionSpeed;
            if (transSpeed > 0.001f && canvasTex_ != 0)
            {
                ensurePrevDeckFBO(canvas.w, canvas.h);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDeckFBO_);
                glBlitFramebuffer(0, 0, canvasW_, canvasH_, 0, 0, canvas.w, canvas.h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                deckTransitionProgress_ = 0.0f;
                deckTransitionSpeed_ = 1.0f / transSpeed;
            }
            else
            {
                deckTransitionProgress_ = 1.0f;   // Instant cut
            }
            prevActiveDeckIndex_ = currentDeckIdx;
        }
    }

    ensureCanvasFBO(canvas.w, canvas.h);
    const float renderW = static_cast<float>(canvas.w);
    const float renderH = static_cast<float>(canvas.h);
    const RenderGeometry::Rect present = RenderGeometry::fitCanvas(canvas.w, canvas.h,
                                                                   static_cast<int>(compW), static_cast<int>(compH));
    // Undrawn canvas pixels are black.
    glBindFramebuffer(GL_FRAMEBUFFER, canvasFBO_);
    glViewport(0, 0, canvas.w, canvas.h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // s-rta-0928 R1.3: nothing else can draw -- the legacy image is needed (decoded / uploaded lazily).
    if (!sourceActive && !deckActive)
        resolveLegacy();

    // Check if we have anything to render
    if (!texMgr_.hasImage() && !sourceActive && !deckActive)
    {
        // Nothing to render yet: a pending capture gets the black canvas-sized frame. The outputs show it too
        // (what the panel shows, never a stale picture).
        publishToOutputs(canvas.w, canvas.h);
        processPendingCapture();
        presentCanvas(static_cast<GLuint>(defaultFBO), present);
        return;
    }

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

    // P13.5.9: Process autopilot (beat-synced clip advancement + beat snap). plan4 T5: the active deck's own
    // instance (by deck index); the decks that are not on screen run theirs at the inactive-deck tick below.
    if (deckActive)
    {
        size_t activeIndex = 0;
        if (composition_ != nullptr && !composition_->decks.empty()
            && deck >= composition_->decks.data() && deck < composition_->decks.data() + composition_->decks.size())
            activeIndex = static_cast<size_t>(deck - composition_->decks.data());
        bool clipAdvanced = autopilots_.forIndex(activeIndex).processFrame(*deck, snap);
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
    // Beats since the previous frame -- the totalBeatCount delta, taken ONCE per frame so every playlist layer sees
    // it (the old per-layer wrap baseline let only the first layer see a crossing; Pitfalls 38 / 42).
    const uint32_t playlistBeats = playlistBeatCrossings_.consume(snap.totalBeatCount);
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

            if (playlistBeats > 0)
            {
                clip->presetBeatsPlayed += static_cast<int>(playlistBeats);

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

    // S167-L4b DT-FIX: real measured frame delta, fed to scaledTime_ below
    // (procedural-source clock) and to compositeDeck()/
    // compositePersistentLayers() further down (video/image-sequence
    // playhead advancement) -- see lastFrameTimestampMs_'s comment in
    // Renderer.h for why this must be a REAL delta, not a hardcoded 1/60.
    // Clamped to [0, 0.25]s so a debugger pause, backgrounding, or the very
    // first frame (lastFrameTimestampMs_ == -1) can't make video (or
    // procedural-source time) jump by an unbounded amount in one frame.
    // Computed here, BEFORE scaledTime_, so both consumers share this one
    // timing source instead of scaledTime_ keeping its own independent
    // 1/60 tick (that second, parallel timing source was the bug: see
    // scaledTime_'s comment in Renderer.h).
    double nowMs = juce::Time::getMillisecondCounterHiRes();
    float realDt = (lastFrameTimestampMs_ >= 0.0)
        ? static_cast<float>((nowMs - lastFrameTimestampMs_) / 1000.0)
        : (1.0f / 60.0f);
    realDt = std::clamp(realDt, 0.0f, 0.25f);
    lastFrameTimestampMs_ = nowMs;

    // S167-L4b: advance scaledTime_ for procedural sources (see its comment
    // in Renderer.h). In deterministic test-capture mode (timeOverride_ set)
    // track the override 1:1, scaled, instead of accumulating -- otherwise
    // render_frame's byte-identical-repeat guarantee would break, since
    // every real GL frame would still tick scaledTime_ forward even while
    // `time` itself stays pinned for the capture.
    // S-RTA-0923 LANE 3 C2: eff() twin read (atomic relaxed load + manual
    // fallback, GL-thread-safe) so a connection driving Composition ▸ Speed
    // actually renders.
    float masterSpeedVal = (composition_ != nullptr) ? composition_->eff(CompScalar::Speed) : 1.0f;
    if (overrideT >= 0.0f)
        scaledTime_ = static_cast<double>(overrideT) * static_cast<double>(masterSpeedVal);
    else
        scaledTime_ += static_cast<double>(realDt) * static_cast<double>(masterSpeedVal);

    // Every pass below renders at the canvas size (renderW x renderH) into canvasFBO_ (plan4 item 1).
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
            // s-rta-0926b: an EMPTY active deck (compositeDeck returned 0) used
            // to drop every persistent layer as well -- the fallback below was
            // presented and the accumulator they had drawn into was discarded.
            // When another deck has persistent content, the frame starts as over
            // a black active deck instead.
            if (sourceTexture == 0)
            {
                for (const auto& otherDeck : composition_->decks)
                {
                    if (&otherDeck != deck && CompositorEngine::hasPersistentContent(otherDeck))
                    {
                        sourceTexture = compositor_.beginEmptyActiveDeck(static_cast<int>(renderW),
                                                                         static_cast<int>(renderH));
                        break;
                    }
                }
            }

            for (auto& otherDeck : composition_->decks)
            {
                if (&otherDeck == deck) continue; // Skip active deck
                compositor_.compositePersistentLayers(otherDeck, shaderMgr_, quad_, time, realDt,
                                                       static_cast<int>(renderW),
                                                       static_cast<int>(renderH));
            }

            // plan4 item 2 -- decks that are not on screen keep time (Boris 2026-09-26: "finish the fade ...
            // does not touch the clips playing in the layer"). Inside `if (deckActive)` on purpose:
            // withDeckDetached's fence (active deck = nullptr) covers this exactly as it covers
            // compositePersistentLayers above. Not gated on sourceTexture: an empty active deck still lets
            // the other decks run. Persistent layers are owned by compositePersistentLayers (DeckClock).
            for (size_t di = 0; di < composition_->decks.size(); ++di)
            {
                Deck& other = composition_->decks[di];
                if (&other == deck) continue;
                // B2 (Boris Q1: "keep playing"): media clocks run without decoding; autopilot keeps advancing.
                DeckClock::tick(other, realDt, [this](const Clip* c, float dt) { tickMediaClock(c, dt); });
                if (autopilots_.forIndex(di).processFrame(other, snap) && onAutopilotAdvanced_)
                {
                    auto callback = onAutopilotAdvanced_;
                    juce::MessageManager::callAsync([callback]() { callback(); });
                }
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

    // The legacy single image (the fallback below) is FITTED inside the canvas over black, never
    // stretched (plan4 S2); every other source fills the canvas.
    bool legacyImage = false;
    if (sourceTexture == 0)
    {
        resolveLegacy();   // s-rta-0928 R1.3: this frame shows the legacy image
        sourceTexture = texMgr_.getImageTexture();
        legacyImage = (sourceTexture != 0);
    }

    if (sourceTexture == 0)
    {
        // No content -- the canvas is already cleared to black (canvas block above). A pending
        // capture gets that black frame, and so do the outputs.
        publishToOutputs(canvas.w, canvas.h);
        processPendingCapture();
        presentCanvas(static_cast<GLuint>(defaultFBO), present);
        return;
    }

    RenderGeometry::Rect vp{ 0, 0, canvas.w, canvas.h };
    if (legacyImage)
    {
        const auto fit = RenderGeometry::fitCanvas(texMgr_.getImageWidth(), texMgr_.getImageHeight(),
                                                   canvas.w, canvas.h);
        if (fit.w > 0 && fit.h > 0)
            vp = fit;
    }

    // P18: pass this renderer's own snapshot copy for audio-reactive
    // uniforms (W2: the shared parked snapshot is gone — each GL context
    // reads the bus itself) and this context's own GL state (W1).
    effectChain_.render(sourceTexture,
                        shaderMgr_, texMgr_, quad_,
                        effectChainGLState_, snap,
                        time, renderW, renderH,
                        canvasFBO_,
                        static_cast<float>(vp.x), static_cast<float>(vp.y),
                        static_cast<float>(vp.w), static_cast<float>(vp.h));

    // P25: Apply composition-level transform (position, scale, rotation)
    applyCompTransform(canvasFBO_, 0.0f, 0.0f, renderW, renderH);

    // S167-L4b: apply Composition::masterOpacity to the fully-composited
    // frame -- the owner's "ceiling" ruling (final = master * layer * clip)
    // for the composition-wide fader. Same dim-to-black technique as the
    // former masterLevel_ block just above (removed s-rta-0925: it was a
    // second, compounding multiply), glBlendColor as a constant multiplier,
    // not an alpha-channel bake, because this runs against the canvas -- the
    // actual output picture (plan4: it used to be the window framebuffer) --
    // where Syphon/recording/capture below read RGB, not alpha. UNCONDITIONAL: deliberately no "opacity ~= 1.0, skip"
    // early-return -- masterOpacity was silently render-dead all session
    // (.harmony/probe-deck-path.sh: accepted, echoed back, changed not one
    // pixel) and a skip-when-default guard here is exactly the shape of bug
    // that produced that. Runs BEFORE videoRecorder_->submitFrame,
    // publishSyphonFrame, and processPendingCapture below, so Master Opacity
    // also dims what leaves the app, not just the on-screen preview.
    if (composition_ != nullptr)
    {
        // S-RTA-0923 LANE 3 C2: eff() twin read (see the comment on the
        // masterSpeedVal read above); unconditional per the S167-L4b comment
        // above this block, unchanged.
        float masterOpacityVal = composition_->eff(CompScalar::Opacity);
        glEnable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, canvasFBO_);
        glViewport(0, 0, canvas.w, canvas.h);

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

    // P25: Cross-deck transition blending. s-rta-0926b plan4 F2: AFTER master opacity -- the outgoing picture
    // (prevDeckFBO_, the canvas as it left the app, see the top of the frame) is already final, so the incoming
    // one is made final first; blending the two final pictures starts exactly on the frame that was on screen
    // (blending before master opacity would dim the outgoing picture twice).
    if (composition_ && deckTransitionProgress_ < 1.0f)
    {
        const int w = canvas.w;
        const int h = canvas.h;
        if (prevDeckTexture_ != 0)
        {
            // Copy the canvas (new deck) to a temp texture
            ensureCompTransformFBO(w, h);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compTransformFBO_);
            glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

            // Draw the transition shader (old deck → new deck)
            glBindFramebuffer(GL_FRAMEBUFFER, canvasFBO_);
            glViewport(0, 0, w, h);

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

        // Advance transition progress. S167-L4b DT-FIX (deck-transition
        // instance, found sweeping for the same shape as the layer-
        // crossfade fix above): deckTransitionSpeed_ is progress-per-SECOND
        // now (see its computation below), so multiplying by the real
        // measured frame delta keeps total transition duration constant
        // regardless of callback rate -- a hardcoded progress-per-frame
        // constant here used to make deck transitions run 2x fast at
        // 120fps / 2x slow at 30fps, same as the layer-crossfade defect.
        deckTransitionProgress_ += deckTransitionSpeed_ * realDt;
        if (deckTransitionProgress_ >= 1.0f)
            deckTransitionProgress_ = 1.0f;
    }

    // s-rta-0927 outputs-c1: the canvas is final here (the deck transition above is its last writer) -- copy it
    // once into the shared frames the output windows present. Inside the measured window: its cost shows in
    // frame_time_ms / gpu_time_ms. No-op (zero cost) when no output is live.
    publishToOutputs(canvas.w, canvas.h);

    // plan4 item 1: the panel shows the finished canvas, letter/pillar-boxed (inside the measured
    // window -- it is this frame's work). The canvas itself is untouched: the outputs below read it.
    presentCanvas(static_cast<GLuint>(defaultFBO), present);
    gpuTimer.finish();

    // P13.5.10: Use CPU-side timing instead of glFinish() which stalls the GPU pipeline.
    // This measures CPU-side render submission time, not GPU execution time.
    // GPU time: the GL_TIME_ELAPSED queries of plan4 A-opt (getGpuTimeMs(), async, no stall).
    auto renderEnd = std::chrono::high_resolution_clock::now();
    double frameMs = std::chrono::duration<double, std::milli>(renderEnd - renderStart).count();

    // EMA smoothing for UI display
    float prevMs = frameTimeMs_.load(std::memory_order_relaxed);
    frameTimeMs_.store(prevMs + 0.1f * (static_cast<float>(frameMs) - prevMs), std::memory_order_relaxed);
    // s-rta-0926b R1: un-smoothed peak (a one-frame hitch is invisible in the EMA)
    if (static_cast<float>(frameMs) > peakFrameTimeMs_.load(std::memory_order_relaxed))
        peakFrameTimeMs_.store(static_cast<float>(frameMs), std::memory_order_relaxed);

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

    // P22.6: Submit frame to video recorder (if recording). plan4 S2: it reads the
    // canvas (submitFrame's glReadPixels reads the bound READ framebuffer).
    if (videoRecorder_ != nullptr)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
        videoRecorder_->submitFrame(canvas.w, canvas.h);
    }

    // P22.1: Publish the final composited frame to Syphon clients. Gated on the
    // enabled flag (set from the message thread) and initialization, so no GPU
    // work happens when Syphon is off or unavailable.
    if (syphonOutput_ != nullptr && syphonOutput_->isEnabled() && syphonOutput_->isInitialized())
        publishSyphonFrame(canvasFBO_, 0.0f, 0.0f, renderW, renderH);

    // Process pending frame capture (Eyes test harness + P22.7 snapshots)
    processPendingCapture();

    // Leave the window framebuffer bound, as every pass used to.
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(defaultFBO));
}

// plan4 item 1: the canvas FBO (same shape as ensurePrevDeckFBO: RGBA8, linear, clamp); recreated
// only on a size change. Creates GL objects and leaves framebuffer 0 bound: called at the top of the
// frame, before any pass (R5).
void Renderer::ensureCanvasFBO(int width, int height)
{
    if (canvasTex_ != 0 && canvasW_ == width && canvasH_ == height)
        return;

    if (canvasFBO_ != 0) glDeleteFramebuffers(1, &canvasFBO_);
    if (canvasTex_ != 0) glDeleteTextures(1, &canvasTex_);

    glGenTextures(1, &canvasTex_);
    glBindTexture(GL_TEXTURE_2D, canvasTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &canvasFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, canvasFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, canvasTex_, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    canvasW_ = width;
    canvasH_ = height;
}

// plan4 item 1: the panel is a viewer. Draws the canvas into the window framebuffer inside `present`
// (RenderGeometry::fitCanvas -- the rest of the window keeps its clear colour: the bars), box-filtered
// down to the panel's pixels. Blend off: the canvas's RGBA is copied as is.
void Renderer::presentCanvas(GLuint windowFBO, const RenderGeometry::Rect& present)
{
    glBindFramebuffer(GL_FRAMEBUFFER, windowFBO);
    if (canvasTex_ == 0 || present.w <= 0 || present.h <= 0)
        return;

    auto* prog = shaderMgr_.getProgram("present_box");
    const bool box = (prog != nullptr);
    if (!box)
        prog = shaderMgr_.getProgram("passthrough");
    if (prog == nullptr)
        return;

    glViewport(present.x, present.y, present.w, present.h);
    glDisable(GL_BLEND);
    prog->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, canvasTex_);
    auto l = prog->getUniformIDFromName("u_texture");
    if (l >= 0) glUniform1i(l, 0);
    if (box)
    {
        l = prog->getUniformIDFromName("u_texelSize");
        if (l >= 0) glUniform2f(l, 1.0f / static_cast<float>(canvasW_), 1.0f / static_cast<float>(canvasH_));
        l = prog->getUniformIDFromName("u_taps");
        if (l >= 0) glUniform1i(l, RenderGeometry::presentTaps(canvasW_, present.w));
    }
    quad_.draw();
}

// s-rta-0927 outputs-c1 (plan5 slice C1, plan5-final.md 8.5): the output tap. Read-only for everything
// downstream: SharedFrameSet::publish copies the canvas and leaves canvasFBO_ bound.
void Renderer::publishToOutputs(int canvasW, int canvasH)
{
    if (liveOutputs_.load(std::memory_order_relaxed) <= 0 && !outputTapForced_.load(std::memory_order_relaxed))
        return;
    jassert(juce::OpenGLContext::getCurrentContext() == &glContext_);
    sharedFrames_.publish(canvasFBO_, canvasW, canvasH);
}

// plan4 A-opt: GPU time per frame from two alternated GL_TIME_ELAPSED queries. A query's result is read
// only once GL_QUERY_RESULT_AVAILABLE says so -- this never blocks; a frame whose slot is still in
// flight is simply not timed.
bool Renderer::beginGpuTimer()
{
    if (gpuQueries_[0] == 0)
        glGenQueries(2, gpuQueries_);
    if (gpuQueries_[0] == 0)
        return false;

    const unsigned slot = gpuQueryFrame_ & 1u;
    if (gpuQueryPending_[slot])
    {
        GLint available = 0;
        glGetQueryObjectiv(gpuQueries_[slot], GL_QUERY_RESULT_AVAILABLE, &available);
        if (available == 0)
            return false;
        GLuint64 ns = 0;
        glGetQueryObjectui64v(gpuQueries_[slot], GL_QUERY_RESULT, &ns);
        gpuQueryPending_[slot] = false;
        const float ms = static_cast<float>(static_cast<double>(ns) / 1.0e6);
        const float prev = gpuTimeMs_.load(std::memory_order_relaxed);
        gpuTimeMs_.store(prev + 0.1f * (ms - prev), std::memory_order_relaxed);   // EMA like frameTimeMs_
        if (ms > peakGpuTimeMs_.load(std::memory_order_relaxed))
            peakGpuTimeMs_.store(ms, std::memory_order_relaxed);
    }
    glBeginQuery(GL_TIME_ELAPSED, gpuQueries_[slot]);
    return true;
}

void Renderer::GpuTimerScope::finish()
{
    if (!active)
        return;
    active = false;
    glEndQuery(GL_TIME_ELAPSED);
    r.gpuQueryPending_[r.gpuQueryFrame_ & 1u] = true;
    ++r.gpuQueryFrame_;
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
    drainRetiredMedia(true);

    compositor_.releaseGL();

    // W1: Release this context's EffectChain GL state (prevFrame FBO +
    // uniform location cache). The cache MUST die with the context — the
    // recreated context recompiles all programs, and stale program-ID-keyed
    // locations would poison lookups against the new programs.
    effectChainGLState_.release();

    // plan4 item 1: the canvas dies with the context; ensureCanvasFBO recreates it on the first frame
    // back (the canvasTex_ == 0 check). The A-opt timer queries likewise.
    if (canvasFBO_ != 0) { glDeleteFramebuffers(1, &canvasFBO_); canvasFBO_ = 0; }
    if (canvasTex_ != 0) { glDeleteTextures(1, &canvasTex_); canvasTex_ = 0; }
    canvasW_ = canvasH_ = 0;
    if (gpuQueries_[0] != 0) { glDeleteQueries(2, gpuQueries_); gpuQueries_[0] = gpuQueries_[1] = 0; }
    gpuQueryPending_[0] = gpuQueryPending_[1] = false;

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

    // s-rta-0927 outputs-c1: this context's slot textures/FBOs go; the pending fence is dropped (it dies with the
    // context). The shared surfaces and the front frame stay: the outputs keep the last picture, and the next
    // context's first publish rebinds the same surfaces.
    sharedFrames_.releaseGL();

    shaderMgr_.releaseAll();
    texMgr_.release();
    quad_.release();

    // s-rta-0928 R1.3: the legacy texture died with texMgr_ (as before: nothing re-loads it on the next context);
    // in-flight results are dropped by their stale tags.
    legacyResidentPath_.clear();
    legacyWant_ = juce::File();
    legacyWantGen_ = 0;
    legacyWantDone_ = true;
    legacyInFlightGen_ = 0;
    legacyPrefetchInFlight_.clear();
    legacyReady_.reset();
    legacyParked_.reset();
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
                    layerParam = cp.live.effective(cp.value);
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
                    source->setParamValue(i, cp.live.effective(cp.value));
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

    // Audio snapshot for audio-reactive sources: the frame's pulse-bearing copy, not a fresh
    // read (onset render-path fix). A fresh read here could see a DIFFERENT hop than the
    // frame's other uploaders, and each source clip would consume/miss the pulse on its own.
    const FeatureSnapshot& snap = frameSnap_;

    return source->render(shaderMgr_, quad_, time, width, height, snap);
}

bool Renderer::openVideoForClip(uint32_t clipId, const juce::File& videoFile)
{
    auto player = std::make_unique<VideoPlayer>();
    if (!player->open(videoFile))
        return false;
    player->setStats(&videoStats_);   // s-rta-0928b video counters

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
    videoStats_.players.store(static_cast<int>(videoPlayers_.size()), std::memory_order_relaxed);
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
        const auto waitStart = std::chrono::steady_clock::now();   // s-rta-0928b video: msg_video_lock_wait_max_ms
        std::lock_guard<std::mutex> lock(videoPlayerMutex_);
        noteMsgVideoLockWait(waitStart);
        auto it = videoPlayers_.find(clipId);
        if (it != videoPlayers_.end())
        {
            it->second->close();
            std::lock_guard<std::mutex> retireLock(retiredMediaMutex_);
            retiredVideoPlayers_.push_back(std::move(it->second));
            videoPlayers_.erase(it);
            videoStats_.players.store(static_cast<int>(videoPlayers_.size()), std::memory_order_relaxed);
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

void Renderer::drainRetiredMedia(bool contextClosing)
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
    // s-rta-0928b seqvram F2: a retired sequence deletes at most the frame's remaining texture-delete budget; one that
    // still holds textures goes back on the list (ahead of anything retired meanwhile) for the next frames. Context
    // close releases everything now.
    std::vector<std::unique_ptr<ImageSequence>> seqLeft;
    size_t leftBytes = 0;
    int leftSlots = 0;
    for (auto& seq : seqToRetire)
    {
        if (contextClosing)
            seq->releaseGL();
        else if (!seq->releaseGLWithin(seqDeletes_, &seqStats_))
        {
            leftBytes += seq->residentBytes();
            leftSlots += seq->residentSlots();
            seqLeft.push_back(std::move(seq));
        }
    }
    if (!seqLeft.empty())
    {
        std::lock_guard<std::mutex> lock(retiredMediaMutex_);
        for (auto& seq : retiredImageSequences_)
            seqLeft.push_back(std::move(seq));
        retiredImageSequences_.swap(seqLeft);
    }
    seqRetiredBytes_ = leftBytes;
    seqRetiredSlots_ = leftSlots;
    // videoToRetire/seqToRetire go out of scope here, destroying each player/
    // sequence (the released ones; the moved-from entries are null). VideoPlayer's destructor re-runs
    // close()+releaseGL() (both already-idempotent no-ops at this point); ImageSequence's destructor
    // re-runs close() (also idempotent).
}

// s-rta-0928b seqvram: frame top, GL thread. Sums every open sequence's texture bytes; only when the total is over
// SeqVram::kBudgetBytes, trims the IDLE sequences (not drawn for SeqVram::kIdleFrames frames, fix round F1: a Pitfall 53
// hold skips a fading layer's outgoing chain for 1-3 frames -- that chain is not idle) to their current + shown frames,
// least-recently drawn first, until it is not. Drawn sequences are never trimmed here: each shrinks to
// its own allowance in getCurrentTexture (floors win, H10). The total seeds this frame's grants (syncMedia keeps it
// running, H3); the idle sequences' bytes above their minimum are reclaimable in a drawn sequence's grant (F3). GL
// deletes here only in the pressure trim, under imageSeqMutex_ (as getCurrentTexture uploads under it), within the
// frame's shared delete budget (F2).
void Renderer::scanSequenceVram()
{
    ++seqFrameSerial_;
    size_t total = 0, idleBytes = 0, idleMin = 0;
    int slots = 0, open = 0, idleSlots = 0;
    {
        std::lock_guard<std::mutex> lock(imageSeqMutex_);
        std::vector<ImageSequence*> idle;
        for (auto& [id, seq] : imageSequences_)
        {
            total += seq->residentBytes();
            ++open;
            if (SeqVram::isIdle(seq->lastDrawnSerial(), seqFrameSerial_))
                idle.push_back(seq.get());
        }
        if (total > SeqVram::kBudgetBytes && !idle.empty())
        {
            std::sort(idle.begin(), idle.end(), [](const ImageSequence* a, const ImageSequence* b) {
                return a->lastDrawnSerial() < b->lastDrawnSerial();
            });
            for (auto* seq : idle)
            {
                // F2: the trim deletes within the frame's shared budget; the evicted slots it cannot delete stay
                // allocated (free) and go on later frames while the total is still over the budget.
                if (total <= SeqVram::kBudgetBytes || seqDeletes_.left <= 0)
                    break;
                if (seq->residentSlots() <= 2)
                    continue;
                const size_t before = seq->residentBytes();
                seq->trimToMinimum(&seqStats_, seqDeletes_);
                total -= before - std::min(before, seq->residentBytes());
            }
        }
        for (auto* seq : idle)
        {
            idleBytes += seq->residentBytes();
            idleMin += seq->trimmedBytes();
            idleSlots += seq->residentSlots();
        }
        for (auto& [id, seq] : imageSequences_)
            slots += seq->residentSlots();
    }
    seqResidentTotal_ = total;
    seqIdleBytes_ = idleBytes;
    seqIdleMinBytes_ = idleMin;
    seqStats_.drawnSlots.store(slots - idleSlots, std::memory_order_relaxed);
    // Reported: the live sequences plus the retired ones still releasing (F2); the grants and the pressure use the live.
    seqStats_.residentBytes.store(static_cast<int64_t>(total + seqRetiredBytes_), std::memory_order_relaxed);
    seqStats_.residentSlots.store(slots + seqRetiredSlots_, std::memory_order_relaxed);
    seqStats_.openCount.store(open, std::memory_order_relaxed);
    seqStats_.overBudget.store(total > SeqVram::kBudgetBytes ? 1 : 0, std::memory_order_relaxed);
}

void Renderer::noteMsgVideoLockWait(std::chrono::steady_clock::time_point waitStart)
{
    VideoStats::noteMax(videoStats_.msgLockWaitMaxMs,
                        std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - waitStart).count());
}

VideoPlayer* Renderer::getVideoPlayer(uint32_t clipId)
{
    const auto waitStart = std::chrono::steady_clock::now();   // s-rta-0928b video: msg_video_lock_wait_max_ms
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    noteMsgVideoLockWait(waitStart);
    auto it = videoPlayers_.find(clipId);
    return (it != videoPlayers_.end()) ? it->second.get() : nullptr;
}

juce::File Renderer::getVideoPlayerFile(uint32_t clipId)
{
    const auto waitStart = std::chrono::steady_clock::now();   // s-rta-0928b video: msg_video_lock_wait_max_ms
    std::lock_guard<std::mutex> lock(videoPlayerMutex_);
    noteMsgVideoLockWait(waitStart);
    auto it = videoPlayers_.find(clipId);
    return (it != videoPlayers_.end()) ? it->second->getFile() : juce::File();
}

ImageSequence* Renderer::getImageSequence(uint32_t clipId)
{
    std::lock_guard<std::mutex> lock(imageSeqMutex_);
    auto it = imageSequences_.find(clipId);
    return (it != imageSequences_.end()) ? it->second.get() : nullptr;
}

GLuint Renderer::getVideoFrameTexture(const Clip* clip, float dt, bool* pending)
{
    return syncMedia(clip, dt, true, pending);
}

GLuint Renderer::syncMedia(const Clip* clip, float dt, bool decode, bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    if (!clip)
        return 0;

    // S167-L4b: composition-wide speed multiplier, folded in below via
    // effectiveClipSpeed() only for non-BPM-synced clips -- BPM-synced
    // transport stays tempo-locked, unaffected by masterSpeed.
    // S-RTA-0923 LANE 3 C2: eff() twin read (see Renderer::renderOpenGL's
    // masterSpeedVal comment).
    const float masterSpeedVal = (composition_ != nullptr) ? composition_->eff(CompScalar::Speed) : 1.0f;

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

        if (decode)
            player->advanceFrame(static_cast<double>(dt));
        else
            player->advanceClock(static_cast<double>(dt));   // plan4 T4: the clock only, no decode
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

        return decode ? player->uploadToTexture() : 0;
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

        // plan4 T4: no lazy PNG load for a deck that is not on screen. s-rta-0928 R1.4: the frames decode off the GL
        // thread (look-ahead); a sequence with nothing to show yet is PENDING, and counts for the render_frame gate
        // (C3: the same framePendingImages counter the compositor's images bump). s-rta-0928b seqvram: the sequence
        // gets a SeqVram::Grant (its window's allowance).
        if (!decode)
            return 0;
        bool seqPending = false;
        // s-rta-0928b seqvram: the allowance = the free share of the sequence budget as the frame stands (a RUNNING
        // total, H3: the frame-top sum, updated after each drawn sequence), never below the floor. F3: the idle
        // sequences' bytes above their minimum are reclaimable (the frame-top scan trims them once the total is over);
        // a sequence idle at the frame top and drawn now is no longer idle.
        const size_t mine = seq->residentBytes();
        if (SeqVram::isIdle(seq->lastDrawnSerial(), seqFrameSerial_))
        {
            seqIdleBytes_ -= std::min(seqIdleBytes_, mine);
            seqIdleMinBytes_ -= std::min(seqIdleMinBytes_, seq->trimmedBytes());
        }
        const size_t others = seqResidentTotal_ - std::min(seqResidentTotal_, mine);
        const SeqVram::Grant grant{ SeqVram::drawnAllowance(seqResidentTotal_, mine, seqIdleBytes_, seqIdleMinBytes_,
                                                            seq->minWindowBytes()),
                                    seqFrameSerial_, &seqStats_, clip->inPoint, clip->outPoint, &seqDeletes_ };
        const GLuint tex = seq->getCurrentTexture(imageDecoder_, uploadBudget_, grant, &seqPending);
        seqResidentTotal_ = others + seq->residentBytes();
        if (seqPending)
        {
            compositor_.notePendingImage();
            if (pending != nullptr)
                *pending = true;
        }
        return tex;
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
    compile("present_box",          EmbeddedShaders::presentBox);   // plan4: canvas -> panel
    compile("opacity_blend",        EmbeddedShaders::opacityBlend);
    compile("clip_opacity_blend",   EmbeddedShaders::clipOpacityBlend);
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
                            int width, int height, bool completeFrame, CaptureEncoding enc)
{
    std::promise<CaptureRead> promise;
    auto future = promise.get_future();

    // s-rta-0928 R2: one capture at a time, from arm to read. A second caller used to overwrite capturePromise_ (the
    // first then timed out after 5 s and its timeout path cleared the second's pending capture), restore the time
    // override LIFO-wrong (live frames froze at the other caller's time) and race the TEST-ONLY canvas lock
    // (TestServer set / cleared it outside this function). The flight lock owns all three; it is never taken on the
    // GL thread, and it is released before the convert + PNG below, so concurrent callers still overlap that work.
    std::unique_lock<std::timed_mutex> flight(captureFlight_, std::defer_lock);
    if (!flight.try_lock_for(std::chrono::seconds(5)))
    {
        std::cerr << "[Eyes] Frame capture timed out after 5s (another capture in flight)" << std::endl;
        return false;
    }
    const uint64_t prevLock = lockedSize_.load(std::memory_order_relaxed);
    if (width > 0 && height > 0)
        setLockedResolution(width, height);   // moved from TestServer (R2); restored below on both exits

    // Set time override for this frame -- BEFORE arming: the frame that answers must have read it (captureArmSeq_)
    float prevTime = timeOverride_.load(std::memory_order_relaxed);
    if (timeOverride >= 0.0f)
        timeOverride_.store(timeOverride, std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> lock(captureMutex_);
        capturePromise_ = &promise;
        pendingCaptureComplete_ = completeFrame;
        pendingCaptureSeq_ = captureArmSeq_.fetch_add(1, std::memory_order_acq_rel) + 1;
        pendingCapture_.store(true, std::memory_order_release);
    }

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
        lockedSize_.store(prevLock, std::memory_order_relaxed);
        return false;
    }

    // s-rta-0927 plan-renderperf C3: the GL thread read the canvas (processPendingCapture) and went on rendering;
    // the conversion, the PNG encode and the file write happen here, on this already-waiting thread. Same pixels,
    // same bytes (PixelConvert == the old setPixelColour loop), and the file is complete before the return (the
    // stream's scope closes -- and flushes -- before the log line). The read arrives by value through THIS call's
    // own future (fix round): nothing shared is read after the signal, so a second capture armed and serviced
    // before this line cannot hand its pixels to this caller.
    CaptureRead read = future.get();
    lockedSize_.store(prevLock, std::memory_order_relaxed);
    flight.unlock();   // R2: the next caller arms now; this one converts + encodes below, off the flight
    if (!read.ok)
        return false;
    std::vector<uint8_t>& pixels = read.pixels;
    const int readW = read.width;
    const int readH = read.height;
    const double readMs = read.readMs;
    if (readW <= 0 || readH <= 0 || pixels.size() != static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4)
    {
        std::cerr << "[Eyes] Invalid capture dimensions: " << readW << "x" << readH << std::endl;
        return false;
    }

    using CaptureClock = std::chrono::steady_clock;
    const auto msSince = [](CaptureClock::time_point t) {
        return std::chrono::duration<double, std::milli>(CaptureClock::now() - t).count();
    };

    // Create JUCE image and copy pixels (flip vertically: GL origin is bottom-left). Row conversion, byte-identical
    // to the old per-pixel setPixelColour loop (s-rta-0927 plan-renderperf C2; tests/test_pixel_convert.cpp).
    double convertMs = 0.0, pngMs = 0.0;
    bool ok = false;
    if (enc == CaptureEncoding::Fast)
    {
        // s-rta-0928 R3: PNG scanlines straight from the GL rows (the bytes JUCE's writer would emit), zlib level 1.
        auto tConvert = CaptureClock::now();
        std::vector<uint8_t> scan(static_cast<size_t>(readH) * (1 + static_cast<size_t>(readW) * 4));
        PixelConvert::rgbaBottomUpToPngScanlines(pixels.data(), readW, readH, scan.data());
        convertMs = msSince(tConvert);
        auto tPng = CaptureClock::now();
        ok = PngWrite::writeScanlinesReplacing(scan.data(), readW, readH, outputPath, PngWrite::kFastPngLevel);
        pngMs = msSince(tPng);
    }
    else
    {
        auto tConvert = CaptureClock::now();
        juce::Image img(juce::Image::ARGB, readW, readH, false);
        {
            juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
            PixelConvert::rgbaBottomUpToARGB(pixels.data(), readW, readH, bmp, false);
        }
        convertMs = msSince(tConvert);

        // Write PNG
        auto tPng = CaptureClock::now();
        ok = PngWrite::writeReplacing(img, outputPath);   // replaces an existing file (F2)
        pngMs = msSince(tPng);
    }

    // C0's split: read = the GL thread's whole share; convert + png ran here.
    if (ok)
        std::cerr << "[Eyes] Captured frame: " << outputPath.getFullPathName()
                  << " (" << readW << "x" << readH << ")"
                  << " read=" << juce::String(readMs, 1) << " convert=" << juce::String(convertMs, 1)
                  << " png=" << juce::String(pngMs, 1) << " ms"
                  << (enc == CaptureEncoding::Fast ? " enc=fast" : " enc=archive") << std::endl;
    else
        std::cerr << "[Eyes] Failed to write PNG: " << outputPath.getFullPathName() << std::endl;

    return ok;
}

void Renderer::processPendingCapture()
{
    // Quick check without lock (avoids lock contention on every frame)
    if (!pendingCapture_.load(std::memory_order_acquire))
        return;

    // plan4 item 1: TestServer's render_frame sets the test lock just before it requests the capture. A
    // frame whose canvas was sized before that store must not answer it -- the next frame renders at
    // the lock's exact size.
    const uint64_t lockPacked = lockedSize_.load(std::memory_order_relaxed);
    const int lockW = static_cast<int>(static_cast<uint32_t>(lockPacked >> 32));
    const int lockH = static_cast<int>(static_cast<uint32_t>(lockPacked));
    if (lockW > 0 && lockH > 0 && (lockW != canvasW_ || lockH != canvasH_))
        return;

    std::lock_guard<std::mutex> lock(captureMutex_);
    if (!pendingCapture_.load(std::memory_order_relaxed) || capturePromise_ == nullptr)
        return;
    // Armed after this frame started: this frame may have rendered at the previous time. The next frame answers.
    if (frameArmSeq_ < pendingCaptureSeq_)
        return;
    // s-rta-0928 R1: a frame that held or skipped a layer (an image still decoding) never answers a render_frame --
    // render_frame right after a load or trigger shows the picture, never the placeholder. The decode always ends
    // (every job delivers a result) and the caller's 5 s timeout is the backstop. A user snapshot does not wait (C2).
    if (pendingCaptureComplete_ && (compositor_.framePendingImages() > 0 || legacyPendingThisFrame_))
        return;

    // plan4 item 1: the capture is the whole canvas, exactly its size.
    const int readW = canvasW_;
    const int readH = canvasH_;
    std::cerr << "[Eyes] Processing capture: canvas " << readW << "x" << readH << std::endl;

    if (canvasFBO_ == 0 || readW <= 0 || readH <= 0)
    {
        std::cerr << "[Eyes] Invalid capture dimensions: " << readW << "x" << readH << std::endl;
        capturePromise_->set_value(CaptureRead{});
        pendingCapture_.store(false, std::memory_order_relaxed);
        capturePromise_ = nullptr;
        return;
    }

    // s-rta-0927 plan-renderperf C3: ONLY the read stays on the GL thread -- the caller of captureFrame (blocked on
    // the promise for the whole capture anyway) converts, encodes and writes the PNG. The read is timed for C0's
    // "[Eyes] Captured frame: ... read=" field (the capture's cost is invisible to the frame timer: it runs after
    // renderEnd).
    const auto tRead = std::chrono::steady_clock::now();
    CaptureRead read;
    read.pixels.resize(static_cast<size_t>(readW) * static_cast<size_t>(readH) * 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO_);
    glReadPixels(0, 0, readW, readH, GL_RGBA, GL_UNSIGNED_BYTE, read.pixels.data());
    read.readMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tRead).count();
    read.width = readW;
    read.height = readH;
    read.ok = true;

    // Handed to this request's own future by value -- never parked in a Renderer member (fix round).
    capturePromise_->set_value(std::move(read));
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
    // Two snapshots in one second: the second gets snapshot_..._2.png instead of overwriting the first (F2).
    auto outputFile = dir.getChildFile(filename).getNonexistentSibling(false);

    // plan4 item 1: a user snapshot is the whole composition canvas (outputWidth x outputHeight).
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
void Renderer::applyCompTransform(GLuint targetFBO, float vpX, float vpY, float vpW, float vpH)
{
    if (!composition_) return;

    // Check if any transform is non-default
    // S-RTA-0923 LANE 3 C2: eff() twin read (see Renderer::renderOpenGL's
    // masterSpeedVal comment) -- a connection driving any of these correctly
    // trips the isDefault early-return below (R-E, s-rta-0923-lane3-plan.md).
    float posX = composition_->eff(CompScalar::PosX);
    float posY = composition_->eff(CompScalar::PosY);
    float scale = composition_->eff(CompScalar::Scale);
    float rotation = composition_->eff(CompScalar::Rotation);
    float anchorX = composition_->eff(CompScalar::AnchorX);
    float anchorY = composition_->eff(CompScalar::AnchorY);

    bool isDefault = (std::abs(posX) < 0.001f && std::abs(posY) < 0.001f &&
                      std::abs(scale - 1.0f) < 0.001f && std::abs(rotation) < 0.01f);
    if (isDefault) return;

    int w = static_cast<int>(vpW);
    int h = static_cast<int>(vpH);
    if (w <= 0 || h <= 0) return;

    ensureCompTransformFBO(w, h);

    // Copy current framebuffer content to the transform texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targetFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, compTransformFBO_);
    glBlitFramebuffer(
        static_cast<int>(vpX), static_cast<int>(vpY),
        static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Now render the transform shader back into the target (plan4: the canvas)
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
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
    // Units fix: posX/posY are pixel offsets (posX3840/posY2160 space,
    // reference 3840x2160 canvas) but u_comp_position is applied by the
    // shader as `uv -= u_comp_position * 0.5` -- normalize to that
    // half-canvas-shift convention (ScalarMath::posPxToCompUniformX/Y,
    // ScalarParams.h) so a non-zero position no longer renders solid black.
    if (l >= 0) glUniform2f(l, ScalarMath::posPxToCompUniformX(posX), ScalarMath::posPxToCompUniformY(posY));
    l = loc("u_comp_scale");
    if (l >= 0) glUniform1f(l, scale);
    l = loc("u_comp_rotation");
    // Convert degrees to radians
    if (l >= 0) glUniform1f(l, rotation * 3.14159265f / 180.0f);
    l = loc("u_comp_anchor");
    // Same units fix as u_comp_position above -- u_comp_anchor is applied
    // with the same `* 0.5` convention (EmbeddedShaders.h compTransform).
    if (l >= 0) glUniform2f(l, ScalarMath::posPxToCompUniformX(anchorX), ScalarMath::posPxToCompUniformY(anchorY));

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

// P22.1: Blit the final composited output (plan4: the whole canvas) into the
// Syphon texture and publish it to clients.
void Renderer::publishSyphonFrame(GLuint srcFBO, float vpX, float vpY, float vpW, float vpH)
{
    int w = static_cast<int>(vpW);
    int h = static_cast<int>(vpH);
    if (w <= 0 || h <= 0)
        return;

    ensureSyphonFBO(w, h);

    // Copy the final-output region into the Syphon texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, syphonFBO_);
    glBlitFramebuffer(
        static_cast<int>(vpX), static_cast<int>(vpY),
        static_cast<int>(vpX + vpW), static_cast<int>(vpY + vpH),
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Leave the source bound (the capture below binds its own READ framebuffer).
    glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);

    // Publish to connected Syphon clients (internally no-op if disabled/uninitialized)
    syphonOutput_->publishTexture(syphonTexture_, w, h);
}
