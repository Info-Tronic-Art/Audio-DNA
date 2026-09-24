#include "ui/RecordPanel.h"

RecordPanel::RecordPanel()
{
    // Record button
    addAndMakeVisible(recordBtn_);
    recordBtn_.setComponentID("record");
    recordBtn_.setEnabled(false);
    recordBtn_.setAlpha(kDisabledAlpha);
    recordBtn_.setTooltip("Recording is coming in a later build. Nothing is captured yet.");
    recordBtn_.onClick = [this] {
        if (onStartRecording) onStartRecording();
    };

    // Stop button
    addAndMakeVisible(stopBtn_);
    stopBtn_.setComponentID("stop");
    stopBtn_.setEnabled(false);
    stopBtn_.setAlpha(kDisabledAlpha);
    stopBtn_.setTooltip("Recording is coming in a later build.");
    stopBtn_.onClick = [this] {
        if (onStopRecording) onStopRecording();
    };

    // Play button
    addAndMakeVisible(playBtn_);
    playBtn_.setComponentID("play");
    playBtn_.setEnabled(false);
    playBtn_.setAlpha(kDisabledAlpha);
    playBtn_.setTooltip("Playback of a recorded performance is coming in a later build.");
    playBtn_.onClick = [this] {
        if (onPlayRecording) onPlayRecording();
    };

    // Save/Load: disabled with tooltips until step 3/4 wires this panel
    // against PerformanceRecorder/Take (see the header comment) -- the
    // buttons stay visible (G25's already-dead UI) rather than disappearing.
    addAndMakeVisible(saveBtn_);
    saveBtn_.setComponentID("save");
    saveBtn_.setEnabled(false);
    saveBtn_.setAlpha(kDisabledAlpha);
    saveBtn_.setTooltip("Saving a recorded performance is coming in a later build.");

    addAndMakeVisible(loadBtn_);
    loadBtn_.setComponentID("load");
    loadBtn_.setEnabled(false);
    loadBtn_.setAlpha(kDisabledAlpha);
    loadBtn_.setTooltip("Loading a recorded performance is coming in a later build.");

    // Format selector
    addAndMakeVisible(formatSelector_);
    formatSelector_.addItem("JSON Events", 1);
    formatSelector_.addItem("Video (coming later)", 2);
    formatSelector_.setSelectedId(1, juce::dontSendNotification);
    formatSelector_.setEnabled(false);
    formatSelector_.setAlpha(kDisabledAlpha);
    formatSelector_.setTooltip("Choosing a recording format is coming in a later build.");

    // Status
    addAndMakeVisible(statusLabel_);
    statusLabel_.setText("Performance recorder coming in a later build", juce::dontSendNotification);
    statusLabel_.setFont(juce::Font(juce::FontOptions(10.0f)));
    statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    // Output directory
    addAndMakeVisible(browseOutputBtn_);
    browseOutputBtn_.setEnabled(false);
    browseOutputBtn_.setAlpha(kDisabledAlpha);
    browseOutputBtn_.setTooltip("Choosing where recordings are saved is coming in a later build.");
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
    outputDirLabel_.setAlpha(kDisabledAlpha);
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

    // Section 1: buttons, with the status label BELOW the row -- a hover
    // tooltip over any button must never cover the status message.
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

    statusLabel_.setBounds(area.removeFromTop(kLabelHeight));

    area.removeFromTop(kRowSpacing);

    // Section 2: Format
    auto fmt = area.removeFromTop(kControlHeight);
    formatSelector_.setBounds(fmt.removeFromLeft(180).reduced(1, 0));

    area.removeFromTop(kRowSpacing);

    // Section 3: Output directory
    auto out = area.removeFromTop(kControlHeight);
    browseOutputBtn_.setBounds(out.removeFromLeft(120).reduced(1, 0));
    out.removeFromLeft(6);
    outputDirLabel_.setBounds(out);
}
