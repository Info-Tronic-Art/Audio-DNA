#include "ui/RecordPanel.h"
#include "recording/SessionRecorder.h"

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
        if (recorder_) recorder_->startRecording();
        if (onStartRecording) onStartRecording();
    };

    // Stop button
    addAndMakeVisible(stopBtn_);
    stopBtn_.setEnabled(false);
    stopBtn_.onClick = [this] {
        recording_ = false;
        recordBtn_.setEnabled(true);
        stopBtn_.setEnabled(false);
        if (recorder_) recorder_->stopRecording();
        statusLabel_.setText("Stopped (" + juce::String(recorder_ ? recorder_->getNumEvents() : 0) + " events)",
                            juce::dontSendNotification);
        statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        if (onStopRecording) onStopRecording();
    };

    // Play button
    addAndMakeVisible(playBtn_);
    playBtn_.onClick = [this] {
        if (recorder_ && recorder_->getNumEvents() > 0)
        {
            recorder_->startPlayback();
            statusLabel_.setText("Playing...", juce::dontSendNotification);
            statusLabel_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kMeterGreen));
        }
        if (onPlayRecording) onPlayRecording();
    };

    // Save button
    addAndMakeVisible(saveBtn_);
    saveBtn_.onClick = [this] {
        if (!recorder_ || recorder_->getNumEvents() == 0) return;

        juce::File saveDir = outputDir_.exists() ? outputDir_
            : juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        auto chooser = std::make_shared<juce::FileChooser>(
            "Save recording...", saveDir.getChildFile("session.json"), "*.json");
        auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                if (recorder_->saveToFile(file))
                    statusLabel_.setText("Saved: " + file.getFileName(), juce::dontSendNotification);
                else
                    statusLabel_.setText("Save failed!", juce::dontSendNotification);
            }
        });
    };

    // Load button
    addAndMakeVisible(loadBtn_);
    loadBtn_.onClick = [this] {
        if (!recorder_) return;
        auto chooser = std::make_shared<juce::FileChooser>(
            "Load recording...", juce::File(), "*.json");
        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                if (recorder_->loadFromFile(file))
                    statusLabel_.setText("Loaded: " + file.getFileName()
                        + " (" + juce::String(recorder_->getNumEvents()) + " events)",
                        juce::dontSendNotification);
                else
                    statusLabel_.setText("Load failed!", juce::dontSendNotification);
            }
        });
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
    if (!recorder_) return;

    if (recorder_->isRecording())
    {
        eventCountLabel_.setText(juce::String(recorder_->getNumEvents()) + " events",
                                juce::dontSendNotification);
    }
    else if (recorder_->isPlaying())
    {
        // Update playback status
    }
    else if (!recording_ && recorder_->getNumEvents() > 0)
    {
        eventCountLabel_.setText(juce::String(recorder_->getNumEvents()) + " events, "
            + juce::String(recorder_->getDuration(), 1) + "s",
            juce::dontSendNotification);
    }
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
