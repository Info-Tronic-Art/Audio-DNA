#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/Renderer.h"
#include "features/FeatureBus.h"

// PreviewPanel: JUCE Component that hosts the OpenGL Renderer.
// Lives in the center of the MainComponent layout.
// The user loads an image via file chooser, and the image is displayed
// with audio-reactive effects applied in real-time.
class PreviewPanel : public juce::Component
{
public:
    explicit PreviewPanel(FeatureBus& featureBus);
    ~PreviewPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Load an image file to display with effects
    void loadImage(const juce::File& imageFile);

    // Queue a camera frame for display
    void queueCameraFrame(const juce::Image& frame);

    bool hasImage() const { return imageLoaded_; }

    // Accessors for renderer internals (used by EffectsRackPanel)
    MappingEngine& getMappingEngine() { return renderer_.getMappingEngine(); }
    EffectChain& getEffectChain() { return renderer_.getEffectChain(); }
    Renderer& getRenderer() { return renderer_; }

private:
    Renderer renderer_;
    bool imageLoaded_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewPanel)
};
