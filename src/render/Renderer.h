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

    juce::OpenGLContext glContext_;
    FeatureBus& featureBus_;

    FullscreenQuad quad_;
    ShaderManager shaderMgr_{glContext_};
    TextureManager texMgr_;
    EffectChain effectChain_;
    MappingEngine mappingEngine_;
    UniformBridge uniformBridge_;  // Kept for reference, no longer used

    double startTime_ = 0.0;

    // Pending image load — protected by mutex (not on hot audio path)
    std::mutex pendingImageMutex_;
    juce::File pendingImageFile_;
    bool hasPendingImage_ = false;
};
