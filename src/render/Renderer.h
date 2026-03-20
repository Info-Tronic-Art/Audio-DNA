#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/FullscreenQuad.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "effects/Effect.h"
#include "effects/EffectChain.h"
#include "effects/UniformBridge.h"
#include "mapping/MappingEngine.h"
#include "effects/EffectLibrary.h"
#include "features/FeatureBus.h"
#include "render/CompositorEngine.h"
#include "sources/SourceRegistry.h"
#include "media/VideoPlayer.h"
#include "media/ImageSequence.h"
#include "model/Clip.h"
#include "model/Deck.h"
#include "model/Autopilot.h"
#include <mutex>
#include <unordered_map>

// Renderer: implements juce::OpenGLRenderer to drive the GL render loop.
//
// Owns the OpenGL context, fullscreen quad, shader/texture managers,
// the effect chain, and the uniform bridge.
//
// The render loop:
//   1. Acquire latest FeatureSnapshot from FeatureBus (lock-free)
//   2. Apply demo mappings (audio features → effect parameters)
//   3. Render the effect chain on the fullscreen quad
//
// All GL calls happen on the dedicated GL thread that JUCE manages.
class Renderer : public juce::OpenGLRenderer
{
public:
    explicit Renderer(FeatureBus& featureBus);
    ~Renderer() override;

    // Attach/detach the GL context to/from a component.
    void attachTo(juce::Component& component);
    void detach();

    // Load an image file to display. Thread-safe (queues for GL thread).
    void loadImage(const juce::File& imageFile);

    // Clear the loaded image so the renderer shows black. Thread-safe.
    void clearImage();

    // Set a procedural source to render (instead of an image). Thread-safe.
    void setActiveSource(const std::string& sourceType,
                         const std::vector<Clip::SourceParam>& params = {});

    // Clear the active source. Thread-safe.
    void clearActiveSource();

    // Update the active source's parameters. Thread-safe.
    void updateActiveSourceParams(const std::vector<Clip::SourceParam>& params);

    // Queue a camera frame for upload on the GL thread. Thread-safe.
    void queueCameraFrame(const juce::Image& frame);

    // --- OpenGLRenderer callbacks (called on GL thread) ---
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    juce::OpenGLContext& getContext() { return glContext_; }

    // Accessors for UI integration
    MappingEngine& getMappingEngine() { return mappingEngine_; }
    EffectChain& getEffectChain() { return effectChain_; }

    // Compositor engine
    CompositorEngine& getCompositor() { return compositor_; }

    // Set the active deck for compositor rendering. Thread-safe.
    // Pass nullptr to disable deck compositing (reverts to single-image mode).
    void setActiveDeck(Deck* deck) { activeDeck_.store(deck, std::memory_order_release); }
    Deck* getActiveDeck() const { return activeDeck_.load(std::memory_order_acquire); }

    // Callback when autopilot advances a clip (called async on message thread)
    void setOnAutopilotAdvanced(std::function<void()> fn) { onAutopilotAdvanced_ = std::move(fn); }

    // Source registry — for creating procedural source instances
    SourceRegistry& getSourceRegistry() { return sourceRegistry_; }

    // Get or create an active procedural source instance for a source type ID.
    // Returns nullptr if the source ID is not registered.
    ProceduralSource* getOrCreateSource(const std::string& sourceId);

    // === Video Playback ===

    // Open a video file for a clip. Returns true on success.
    // Call from message thread. The Renderer manages the VideoPlayer lifecycle.
    bool openVideoForClip(uint32_t clipId, const juce::File& videoFile);

    // Open an image sequence for a clip. Returns true on success.
    bool openImageSequenceForClip(uint32_t clipId, const std::vector<juce::File>& files, float fps);

    // Close video/sequence for a clip.
    void closeMediaForClip(uint32_t clipId);

    // Get the VideoPlayer for a clip (nullptr if none). For transport control.
    VideoPlayer* getVideoPlayer(uint32_t clipId);

    // Get the ImageSequence for a clip (nullptr if none). For transport control.
    ImageSequence* getImageSequence(uint32_t clipId);

    // Render a procedural source and return the output texture ID.
    // Must be called on the GL thread.
    // If clipSourceParams is provided, applies those param values to the source.
    GLuint renderSource(const std::string& sourceId, float time, int width, int height,
                        const std::vector<Clip::SourceParam>* clipSourceParams = nullptr);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

private:
    void initShaders();
    void initEffectChain();
    void compileAllShaders();

    // Compile a shader with optional shared GLSL utility prepends.
    // Prepends the requested utility blocks before the fragment shader source.
    void compileShaderWithUtils(const juce::String& name, const char* frag,
                                 bool needsNoise = false, bool needsSDF = false,
                                 bool needsUtil = false);

    juce::OpenGLContext glContext_;
    FeatureBus& featureBus_;

    FullscreenQuad quad_;
    ShaderManager shaderMgr_{glContext_};
    TextureManager texMgr_;
    EffectChain effectChain_;
    EffectLibrary effectLibrary_;  // Persistent library for compositor per-clip effects
    MappingEngine mappingEngine_;
    UniformBridge uniformBridge_;  // Kept for reference, no longer used

    double startTime_ = 0.0;

    // FPS tracking
    std::atomic<float> currentFps_{0.0f};
    int frameCount_ = 0;
    double fpsTimer_ = 0.0;

    // Locked render resolution (0,0 = follow component size)
    std::atomic<int> lockedWidth_{0};
    std::atomic<int> lockedHeight_{0};

public:
    float getFps() const { return currentFps_.load(std::memory_order_relaxed); }

    // Set a fixed render resolution. Pass (0,0) to follow component size.
    void setLockedResolution(int w, int h)
    {
        lockedWidth_.store(w, std::memory_order_relaxed);
        lockedHeight_.store(h, std::memory_order_relaxed);
    }
    int getLockedWidth() const { return lockedWidth_.load(std::memory_order_relaxed); }
    int getLockedHeight() const { return lockedHeight_.load(std::memory_order_relaxed); }

    // Master output level (0 = black, 1 = full brightness)
    void setMasterLevel(float level) { masterLevel_.store(level, std::memory_order_relaxed); }
    float getMasterLevel() const { return masterLevel_.load(std::memory_order_relaxed); }

    // Render frame time tracking
    float getFrameTimeMs() const { return frameTimeMs_.load(std::memory_order_relaxed); }

private:
    std::atomic<float> masterLevel_{1.0f};
    std::atomic<float> frameTimeMs_{0.0f};
    double renderProfileAccum_ = 0.0;
    int renderProfileCount_ = 0;
    static constexpr int kRenderProfileInterval = 300; // Log every N frames (~5s at 60fps)

    // Adaptive quality: track sustained high frame times
    int highFrameTimeCount_ = 0;
    static constexpr float kFrameTimeBudgetMs = 12.0f;
    static constexpr int kHighFrameTimeThreshold = 30; // ~0.5s sustained

    // Compositor
    CompositorEngine compositor_;
    std::atomic<Deck*> activeDeck_{nullptr};
    Autopilot autopilot_;  // Processes beat-synced clip advancement
    std::function<void()> onAutopilotAdvanced_;  // UI refresh callback

    // Procedural sources
    SourceRegistry sourceRegistry_;
    std::unordered_map<std::string, std::unique_ptr<ProceduralSource>> activeSources_;

    // Video players — keyed by clip ID
    std::mutex videoPlayerMutex_;
    std::unordered_map<uint32_t, std::unique_ptr<VideoPlayer>> videoPlayers_;

    // Image sequences — keyed by clip ID
    std::mutex imageSeqMutex_;
    std::unordered_map<uint32_t, std::unique_ptr<ImageSequence>> imageSequences_;

    // Get video frame texture for a clip (used as compositor callback)
    GLuint getVideoFrameTexture(const Clip* clip, float dt);

    // Pending image load — protected by mutex (not on hot audio path)
    std::mutex pendingImageMutex_;
    juce::File pendingImageFile_;
    bool hasPendingImage_ = false;
    bool pendingClearImage_ = false;

    // Camera frame queue
    std::mutex cameraFrameMutex_;
    juce::Image pendingCameraFrame_;
    bool hasPendingCameraFrame_ = false;

    // Active procedural source for direct preview
    std::mutex activeSourceMutex_;
    std::string activeSourceType_;
    std::vector<Clip::SourceParam> activeSourceParams_;
    bool hasActiveSource_ = false;
};
