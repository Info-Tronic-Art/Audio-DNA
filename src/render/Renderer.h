#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/FullscreenQuad.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "effects/Effect.h"
#include "effects/EffectChain.h"
#include "mapping/MappingEngine.h"
#include "effects/EffectLibrary.h"
#include "features/FeatureBus.h"
#include "signal/SignalRegistry.h"
#include "routing/RoutingEngine.h"
#include "render/CompositorEngine.h"
#include "sources/SourceRegistry.h"
#include "media/VideoPlayer.h"
#include "media/ImageSequence.h"
#include "model/Clip.h"
#include "model/Deck.h"
#include "model/Autopilot.h"
#include <mutex>
#include <future>
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

    // Effect library (for looking up effect definitions by name)
    EffectLibrary& getEffectLibrary() { return effectLibrary_; }

    // Set the active deck for compositor rendering. Thread-safe.
    // Pass nullptr to disable deck compositing (reverts to single-image mode).
    void setActiveDeck(Deck* deck) { activeDeck_.store(deck, std::memory_order_release); }
    Deck* getActiveDeck() const { return activeDeck_.load(std::memory_order_acquire); }

    // P21: Set composition pointer for persistent layer rendering across decks.
    void setComposition(Composition* comp) { composition_ = comp; }

    // Callback when autopilot advances a clip (called async on message thread)
    void setOnAutopilotAdvanced(std::function<void()> fn) { onAutopilotAdvanced_ = std::move(fn); }

    // P23: Callback when genre changes (called async on message thread)
    // Params: new genre ID (0-7), confidence [0,1]
    void setOnGenreChanged(std::function<void(uint8_t, float)> fn) { onGenreChanged_ = std::move(fn); }

    // P23: Callback when structural state changes (called async on message thread)
    // Params: new state (0=normal, 1=buildup, 2=drop, 3=breakdown)
    void setOnStructuralStateChanged(std::function<void(uint8_t)> fn) { onStructuralStateChanged_ = std::move(fn); }

    // P20: Set per-type autopilot config (from Composition)
    void setPerTypeAutopilotConfig(const Composition::PerTypeAutopilotConfig* config)
    {
        autopilot_.setPerTypeConfig(config);
    }

    // P23: Enable smart random autopilot (energy-aware clip selection)
    void setSmartRandomEnabled(bool enabled) { autopilot_.setSmartRandomEnabled(enabled); }

    // Signal routing — P16: wire signals into render loop
    void setSignalRegistry(SignalRegistry* reg) { signalRegistry_ = reg; }
    SignalRegistry* getSignalRegistry() { return signalRegistry_; }
    RoutingEngine& getRoutingEngine() { return routingEngine_; }

    // Source registry — for creating procedural source instances
    SourceRegistry& getSourceRegistry() { return sourceRegistry_; }

    // P20.5: Set analysis thread pointer for PCM audio feed to projectM sources
    void setAnalysisThread(class AnalysisThread* at) { analysisThread_ = at; }

    // P22.6: Set video recorder for real-time frame capture
    void setVideoRecorder(class VideoRecorder* recorder) { videoRecorder_ = recorder; }

    // P22.1: Set Syphon output for inter-app texture sharing
    void setSyphonOutput(class SyphonOutput* syphon) { syphonOutput_ = syphon; }

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

    // === Frame Capture ===

    // Request a single frame capture. Blocks the calling thread until the GL
    // thread renders and saves the frame. Returns true on success.
    // Must NOT be called from the GL thread (deadlock).
    // timeOverride: if >= 0, overrides u_time for deterministic rendering.
    // width/height: if > 0, temporarily sets locked resolution.
    bool captureFrame(const juce::File& outputPath, float timeOverride = -1.0f,
                      int width = 0, int height = 0);

    // P22.7: Take a snapshot (PNG) to the snapshots directory.
    // Returns the saved file path, or empty on failure.
    // If autoImportCallback is set, calls it on message thread with the file.
    juce::File takeSnapshot();
    void setSnapshotDir(const juce::File& dir) { snapshotDir_ = dir; }
    juce::File getSnapshotDir() const { return snapshotDir_; }
    std::function<void(const juce::File&)> onSnapshotTaken;

    // Override u_time for deterministic test rendering. Set to < 0 to disable.
    void setTimeOverride(float t) { timeOverride_.store(t, std::memory_order_relaxed); }
    float getTimeOverride() const { return timeOverride_.load(std::memory_order_relaxed); }

private:
    // P25: Composition-level transform FBO
    GLuint compTransformFBO_ = 0;
    GLuint compTransformTexture_ = 0;
    int compTransformWidth_ = 0;
    int compTransformHeight_ = 0;
    void ensureCompTransformFBO(int width, int height);
    void applyCompTransform(GLuint defaultFBO, float vpX, float vpY, float vpW, float vpH);

    // P25: Cross-deck transition state
    GLuint prevDeckFBO_ = 0;
    GLuint prevDeckTexture_ = 0;
    int prevDeckWidth_ = 0;
    int prevDeckHeight_ = 0;
    void ensurePrevDeckFBO(int width, int height);
    int prevActiveDeckIndex_ = 0;
    float deckTransitionProgress_ = 1.0f;  // 1.0 = complete (no transition)
    float deckTransitionSpeed_ = 0.0f;     // Progress per frame (0 = instant)

    std::atomic<float> masterLevel_{1.0f};
    std::atomic<float> frameTimeMs_{0.0f};
    double renderProfileAccum_ = 0.0;
    int renderProfileCount_ = 0;
    static constexpr int kRenderProfileInterval = 300; // Log every N frames (~5s at 60fps)

    // Adaptive quality: track sustained high frame times
    int highFrameTimeCount_ = 0;
    static constexpr float kFrameTimeBudgetMs = 12.0f;
    static constexpr int kHighFrameTimeThreshold = 30; // ~0.5s sustained

    // Signal routing (P16)
    SignalRegistry* signalRegistry_ = nullptr;  // Owned by MainComponent
    RoutingEngine routingEngine_;

    // Compositor
    CompositorEngine compositor_;
    std::atomic<Deck*> activeDeck_{nullptr};
    Composition* composition_ = nullptr; // P21: for persistent layer rendering across decks
    Autopilot autopilot_;  // Processes beat-synced clip advancement
    std::function<void()> onAutopilotAdvanced_;  // UI refresh callback

    // P23: Genre/structural change detection
    uint8_t lastDetectedGenre_ = 6;     // Last confirmed genre (default Pop/Electronic)
    uint8_t lastStructuralState_ = 0;   // Last structural state
    std::function<void(uint8_t, float)> onGenreChanged_;
    std::function<void(uint8_t)> onStructuralStateChanged_;

    // Procedural sources
    SourceRegistry sourceRegistry_;
    std::unordered_map<std::string, std::unique_ptr<ProceduralSource>> activeSources_;

    // P20.5: Analysis thread for PCM audio feed to projectM
    AnalysisThread* analysisThread_ = nullptr;

    // P22: Output integrations (owned by MainComponent, not Renderer)
    VideoRecorder* videoRecorder_ = nullptr;
    SyphonOutput* syphonOutput_ = nullptr;

    // P20.5: Playlist cycling state tracking
    float lastPlaylistBeatPhase_ = 0.0f;
    uint8_t lastPlaylistStructState_ = 0;

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

    // Frame capture
    std::atomic<float> timeOverride_{-1.0f};
    std::mutex captureMutex_;
    std::atomic<bool> pendingCapture_{false};
    juce::File captureOutputPath_;
    int captureWidth_ = 0;
    int captureHeight_ = 0;
    std::promise<bool>* capturePromise_ = nullptr;
    juce::File snapshotDir_; // P22.7: where snapshots are saved

    // Process pending capture after render. Called from renderOpenGL().
    void processPendingCapture(float renderW, float renderH,
                               float vpX, float vpY, float vpW, float vpH);
};
