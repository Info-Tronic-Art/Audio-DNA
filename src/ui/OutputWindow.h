#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include "render/FullscreenQuad.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "effects/EffectChain.h"
#include "mapping/MappingEngine.h"
#include "features/FeatureBus.h"

// OutputRenderer: renders the same effect chain as the primary Renderer
// but in its own OpenGL context (for the output window/display).
//
// Shares FeatureBus, MappingEngine, and EffectChain with the primary
// Renderer — only GL-state objects (shaders, textures, FBOs, quad) are
// owned independently.
class OutputRenderer : public juce::OpenGLRenderer
{
public:
    OutputRenderer(FeatureBus& featureBus,
                   MappingEngine& mappingEngine,
                   EffectChain& effectChain);

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void attachTo(juce::Component& component);
    void detach();

    // Queue an image load (thread-safe)
    void loadImage(const juce::File& imageFile);

    juce::OpenGLContext& getContext() { return glContext_; }

private:
    void initShaders();

    juce::OpenGLContext glContext_;
    FeatureBus& featureBus_;
    MappingEngine& mappingEngine_;
    EffectChain& effectChain_;

    // Own GL state
    FullscreenQuad quad_;
    ShaderManager shaderMgr_{glContext_};
    TextureManager texMgr_;

    double startTime_ = 0.0;

    std::mutex pendingImageMutex_;
    juce::File pendingImageFile_;
    bool hasPendingImage_ = false;

    // Remember the last loaded image so we can reload on GL context recreation
    juce::File lastImageFile_;
};

// OutputWindow: a borderless fullscreen window for the VJ output display.
// Can be placed on any connected monitor.
class OutputWindow : public juce::DocumentWindow
{
public:
    OutputWindow(FeatureBus& featureBus,
                 MappingEngine& mappingEngine,
                 EffectChain& effectChain);
    ~OutputWindow() override;

    void closeButtonPressed() override;
    void resized() override;

    // Move window to the specified display and go fullscreen
    void goFullscreenOnDisplay(const juce::Displays::Display& display);

    // Load the same image as the preview
    void loadImage(const juce::File& imageFile);

    // Keyboard handling — Escape closes
    bool keyPressed(const juce::KeyPress& key) override;

private:
    // The GL rendering surface
    class OutputComponent : public juce::Component
    {
    public:
        OutputComponent() = default;
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::black);
        }
    };

    OutputComponent outputComponent_;
    OutputRenderer renderer_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputWindow)
};
