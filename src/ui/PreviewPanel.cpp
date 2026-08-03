#include "PreviewPanel.h"

PreviewPanel::PreviewPanel(const FeatureBus& featureBus)
    : renderer_(featureBus)
{
    // Tab buttons
    addAndMakeVisible(previewTabBtn_);
    addAndMakeVisible(outputTabBtn_);

    previewTabBtn_.onClick = [this] { setActiveTab(Tab::Preview); };
    outputTabBtn_.onClick  = [this] { setActiveTab(Tab::Output); };

    // GL host — the renderer attaches to this inner component,
    // so the tab bar (JUCE 2D) stays above the GL surface.
    addAndMakeVisible(glHost_);

    updateTabButtonColors();

    // Attach the GL context to the inner host component (not this)
    renderer_.attachTo(glHost_);
}

PreviewPanel::~PreviewPanel()
{
    renderer_.detach();
}

void PreviewPanel::paint(juce::Graphics& g)
{
    // Tab bar background
    auto tabArea = getLocalBounds().removeFromTop(kTabBarHeight);
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface));
    g.fillRect(tabArea);

    // Border below tabs
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawHorizontalLine(kTabBarHeight - 1, 0.0f, static_cast<float>(getWidth()));
}

void PreviewPanel::resized()
{
    auto area = getLocalBounds();
    auto tabBar = area.removeFromTop(kTabBarHeight);

    int tabWidth = std::min(80, tabBar.getWidth() / 2);
    previewTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));
    outputTabBtn_.setBounds(tabBar.removeFromLeft(tabWidth));

    // GL host fills the content area below tabs — visible on both tabs
    // Preview = selected clip solo, Output = full composition
    // (Both render from the same renderer for now)
    glHost_.setBounds(area);
}

void PreviewPanel::loadImage(const juce::File& imageFile)
{
    renderer_.loadImage(imageFile);
    imageLoaded_ = true;
    glHost_.imageLoaded_ = true;
    glHost_.repaint();
}

void PreviewPanel::clearImage()
{
    renderer_.clearImage();
    imageLoaded_ = false;
    glHost_.imageLoaded_ = false;
    glHost_.repaint();
}

void PreviewPanel::queueCameraFrame(const juce::Image& frame)
{
    renderer_.queueCameraFrame(frame);
    if (!imageLoaded_)
    {
        imageLoaded_ = true;
        glHost_.imageLoaded_ = true;
        glHost_.repaint();
    }
}

void PreviewPanel::setActiveTab(Tab tab)
{
    if (activeTab_ == tab)
        return;
    activeTab_ = tab;
    updateTabButtonColors();
    repaint();
}

void PreviewPanel::updateTabButtonColors()
{
    auto setTabStyle = [](juce::TextButton& btn, bool active) {
        if (active)
        {
            btn.setColour(juce::TextButton::buttonColourId,
                          juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
            btn.setColour(juce::TextButton::textColourOffId,
                          juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        }
        else
        {
            btn.setColour(juce::TextButton::buttonColourId,
                          juce::Colour(AudioDNALookAndFeel::kSurface));
            btn.setColour(juce::TextButton::textColourOffId,
                          juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        }
    };

    setTabStyle(previewTabBtn_, activeTab_ == Tab::Preview);
    setTabStyle(outputTabBtn_,  activeTab_ == Tab::Output);
}
