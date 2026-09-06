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

    // Save/Load: dead until step 3/4 wires this panel against
    // PerformanceRecorder/Take (see the header comment) -- the buttons
    // stay visible (G25's already-dead UI) rather than disappearing.
    addAndMakeVisible(saveBtn_);
    addAndMakeVisible(loadBtn_);

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

    // Event count
    addAndMakeVisible(eventCountLabel_);
    eventCountLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    eventCountLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

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

void RecordPanel::refresh()
{
    // Still has no caller (G25) -- SessionRecorder is deleted; step 3/4
    // re-wires this against PerformanceRecorder/Player's own status.
}

void RecordPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void RecordPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Section 1: Controls label + buttons
    auto lbl1 = area.removeFromTop(kLabelHeight);
    statusLabel_.setBounds(lbl1);

    auto ctrl = area.removeFromTop(kControlHeight);
    recordBtn_.setBounds(ctrl.removeFromLeft(56).reduced(1, 0));
    ctrl.removeFromLeft(2);
    stopBtn_.setBounds(ctrl.removeFromLeft(44).reduced(1, 0));
    ctrl.removeFromLeft(2);
    playBtn_.setBounds(ctrl.removeFromLeft(44).reduced(1, 0));
    ctrl.removeFromLeft(2);
    saveBtn_.setBounds(ctrl.removeFromLeft(44).reduced(1, 0));
    ctrl.removeFromLeft(2);
    loadBtn_.setBounds(ctrl.removeFromLeft(44).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Event count
    eventCountLabel_.setBounds(area.removeFromTop(kLabelHeight));

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
