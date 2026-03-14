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
#include <mutex>

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

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

private:
    void initShaders();
    void initEffectChain();
    void compileAllShaders();

    juce::OpenGLContext glContext_;
    FeatureBus& featureBus_;

    FullscreenQuad quad_;
    ShaderManager shaderMgr_{glContext_};
    TextureManager texMgr_;
    EffectChain effectChain_;
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

    // Pending image load — protected by mutex (not on hot audio path)
    std::mutex pendingImageMutex_;
    juce::File pendingImageFile_;
    bool hasPendingImage_ = false;

    // Camera frame queue
    std::mutex cameraFrameMutex_;
    juce::Image pendingCameraFrame_;
    bool hasPendingCameraFrame_ = false;
};
