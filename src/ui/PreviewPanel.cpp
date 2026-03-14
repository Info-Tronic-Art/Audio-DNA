#include "PreviewPanel.h"

PreviewPanel::PreviewPanel(FeatureBus& featureBus)
    : renderer_(featureBus)
{
    // Attach the GL context to this component
    renderer_.attachTo(*this);
}

PreviewPanel::~PreviewPanel()
{
    renderer_.detach();
}

void PreviewPanel::paint(juce::Graphics& g)
{
    // Only paint JUCE 2D content when no image is loaded.
    // When an image is loaded, the OpenGL renderer handles everything.
    if (!imageLoaded_)
    {
        g.fillAll(juce::Colour(0xff0a0a14));
        g.setColour(juce::Colour(0xff666666));
        g.setFont(16.0f);
        g.drawText("Load an image to see audio-reactive effects",
                    getLocalBounds(), juce::Justification::centred);
    }
}

void PreviewPanel::resized()
{
    // The GL context auto-resizes with the component
}

void PreviewPanel::loadImage(const juce::File& imageFile)
{
    renderer_.loadImage(imageFile);
    imageLoaded_ = true;
    repaint(); // Clear the "load an image" text
}
