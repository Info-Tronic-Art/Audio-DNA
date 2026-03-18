#include "ui/RecordPanel.h"

RecordPanel::RecordPanel()
{
    // Record button
    addAndMakeVisible(recordBtn_);
    recordBtn_.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff442222));
    recordBtn_.onClick = [this] {
        recording_ = true;
        recordBtn_.setEnabled(false);
        stopBtn_.setEnabled(true);
        statusLabel_.setText("Recording...", juce::dontSendNotification);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kMeterRed));
        if (onStartRecording) onStartRecording();
    };

    // Stop button
    addAndMakeVisible(stopBtn_);
    stopBtn_.setEnabled(false);
    stopBtn_.onClick = [this] {
        recording_ = false;
        recordBtn_.setEnabled(true);
        stopBtn_.setEnabled(false);
        statusLabel_.setText("Stopped", juce::dontSendNotification);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        if (onStopRecording) onStopRecording();
    };

    // Play button
    addAndMakeVisible(playBtn_);
    playBtn_.onClick = [this] {
        if (onPlayRecording) onPlayRecording();
    };

    // Format selector
    addAndMakeVisible(formatSelector_);
    formatSelector_.addItem("JSON Events", 1);
    formatSelector_.addItem("Video (Future)", 2);
    formatSelector_.setSelectedId(1, juce::dontSendNotification);
    formatSelector_.setEnabled(true);

    // Status
    addAndMakeVisible(statusLabel_);
    statusLabel_.setText("Ready", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    // Output directory
    addAndMakeVisible(browseOutputBtn_);
    browseOutputBtn_.onClick = [this] {
        auto chooser = std::make_shared<juce::FileChooser>("Select output folder...");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
        chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc) {
            auto dir = fc.getResult();
            if (dir.isDirectory())
            {
                outputDir_ = dir;
                outputDirLabel_.setText(dir.getFileName(), juce::dontSendNotification);
            }
        });
    };

    addAndMakeVisible(outputDirLabel_);
    outputDirLabel_.setText("(default)", juce::dontSendNotification);
    outputDirLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    outputDirLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
}

void RecordPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Note about Phase 12
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.4f));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("Full recording implementation in Phase 12",
               getLocalBounds().removeFromBottom(20), juce::Justification::centred, false);
}

void RecordPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Section 1: Controls label + buttons
    auto lbl1 = area.removeFromTop(kLabelHeight);
    statusLabel_.setBounds(lbl1.removeFromRight(lbl1.getWidth() / 2));
    // paint "Controls" label manually is gone — use the status label area

    auto ctrl = area.removeFromTop(kControlHeight);
    recordBtn_.setBounds(ctrl.removeFromLeft(64).reduced(1, 0));
    ctrl.removeFromLeft(2);
    stopBtn_.setBounds(ctrl.removeFromLeft(52).reduced(1, 0));
    ctrl.removeFromLeft(2);
    playBtn_.setBounds(ctrl.removeFromLeft(52).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Section 2: Format
    area.removeFromTop(kLabelHeight); // "Format" label space
    auto fmt = area.removeFromTop(kControlHeight);
    formatSelector_.setBounds(fmt.removeFromLeft(180).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Section 3: Output directory
    area.removeFromTop(kLabelHeight); // "Output" label space
    auto out = area.removeFromTop(kControlHeight);
    browseOutputBtn_.setBounds(out.removeFromLeft(120).reduced(1, 0));
    out.removeFromLeft(6);
    outputDirLabel_.setBounds(out);
}
