#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "render/Renderer.h"
#include "features/FeatureBus.h"
#include "ui/LookAndFeel.h"

// PreviewPanel: JUCE Component that hosts the OpenGL Renderer.
// v2: Has [Preview] [Output] tabs.
//   Preview tab: shows selected clip/layer solo or full composition.
//   Output tab: shows what goes to the external display (full composition).
// Both tabs render via the same GL renderer — the tab just labels the context.
class PreviewPanel : public juce::Component
{
public:
    explicit PreviewPanel(FeatureBus& featureBus);
    ~PreviewPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Load an image file to display with effects
    void loadImage(const juce::File& imageFile);

    // Clear the preview — show black (no clip playing)
    void clearImage();

    // Queue a camera frame for display
    void queueCameraFrame(const juce::Image& frame);

    bool hasImage() const { return imageLoaded_; }

    // Accessors for renderer internals (used by EffectsRackPanel)
    MappingEngine& getMappingEngine() { return renderer_.getMappingEngine(); }
    EffectChain& getEffectChain() { return renderer_.getEffectChain(); }
    Renderer& getRenderer() { return renderer_; }

    // Tab control
    enum class Tab : int { Preview = 0, Output = 1 };
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }

private:
    // Inner component that hosts the GL context — keeps GL rendering
    // separate from the tab bar so JUCE 2D widgets remain visible.
    class GLHost : public juce::Component
    {
    public:
        GLHost() = default;
        void paint(juce::Graphics& g) override
        {
            if (!imageLoaded_)
            {
                g.fillAll(juce::Colour(0xff0a0a14));
                g.setColour(juce::Colour(0xff666666));
                g.setFont(14.0f);
                g.drawText("Load an image to see audio-reactive effects",
                            getLocalBounds(), juce::Justification::centred);
            }
        }
        bool imageLoaded_ = false;
    };

    GLHost glHost_;
    Renderer renderer_;
    bool imageLoaded_ = false;

    // Tabs
    Tab activeTab_ = Tab::Preview;
    juce::TextButton previewTabBtn_{"Preview"};
    juce::TextButton outputTabBtn_{"Output"};
    static constexpr int kTabBarHeight = 26;

    void updateTabButtonColors();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreviewPanel)
};
