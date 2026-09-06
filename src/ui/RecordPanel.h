#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// RecordPanel: settings recording controls (start/stop, playback).
// s168 step 1 (recorder core): SessionRecorder is deleted (see
// src/recording/{Take,Program,Player,PerformanceRecorder}.h) -- this panel
// is left as dead UI (already noted, G25) until step 3/4 wires it against
// the new PerformanceRecorder/Player classes.
class RecordPanel : public juce::Component
{
public:
    RecordPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks for actions that MainComponent handles
    std::function<void()> onStartRecording;
    std::function<void()> onStopRecording;
    std::function<void()> onPlayRecording;

    bool isRecording() const { return recording_; }

    // Update status display (call periodically)
    void refresh();

    // Get the output directory
    juce::File getOutputDir() const { return outputDir_; }

private:
    bool recording_ = false;

    juce::TextButton recordBtn_{"Record"};
    juce::TextButton stopBtn_{"Stop"};
    juce::TextButton playBtn_{"Play"};
    juce::TextButton saveBtn_{"Save"};
    juce::TextButton loadBtn_{"Load"};
    juce::TextButton browseOutputBtn_{"Output Folder..."};

    juce::Label statusLabel_;
    juce::Label eventCountLabel_;
    juce::Label outputDirLabel_;
    juce::File outputDir_;

    juce::ComboBox formatSelector_;

    static constexpr int kLabelHeight = 14;
    static constexpr int kControlHeight = 28;
    static constexpr int kRowSpacing = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordPanel)
};
