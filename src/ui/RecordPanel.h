#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// RecordPanel: settings recording controls (start/stop, playback).
// Records parameter changes and clip triggers as timestamped events.
// Full implementation in Phase 12 — this is the UI shell.
class RecordPanel : public juce::Component
{
public:
    RecordPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks
    std::function<void()> onStartRecording;
    std::function<void()> onStopRecording;
    std::function<void()> onPlayRecording;

    bool isRecording() const { return recording_; }

private:
    bool recording_ = false;

    juce::TextButton recordBtn_{"Record"};
    juce::TextButton stopBtn_{"Stop"};
    juce::TextButton playBtn_{"Play"};
    juce::TextButton browseOutputBtn_{"Output Folder..."};

    juce::Label statusLabel_;
    juce::Label outputDirLabel_;
    juce::File outputDir_;

    juce::ComboBox formatSelector_;

    static constexpr int kLabelHeight = 14;
    static constexpr int kControlHeight = 28;
    static constexpr int kRowSpacing = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordPanel)
};
