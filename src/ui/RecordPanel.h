#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// RecordPanel: settings recording controls (start/stop, playback).
// s168 step 1 (recorder core): SessionRecorder is deleted (see
// src/recording/{Take,Program,Player,PerformanceRecorder}.h) -- this panel
// is disabled with tooltips until spec steps 3/4 wire it; nothing is
// captured or played; MainComponent's TooltipWindow (MainComponent.cpp:471)
// shows the tooltips.
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

    // Update status display (call periodically)
    void refresh();

    // Get the output directory
    juce::File getOutputDir() const { return outputDir_; }

private:
    juce::TextButton recordBtn_{"Record"};
    juce::TextButton stopBtn_{"Stop"};
    juce::TextButton playBtn_{"Play"};
    juce::TextButton saveBtn_{"Save"};
    juce::TextButton loadBtn_{"Load"};
    juce::TextButton browseOutputBtn_{"Output Folder..."};

    juce::Label statusLabel_;
    juce::Label outputDirLabel_;
    juce::File outputDir_;

    juce::ComboBox formatSelector_;

    static constexpr int kLabelHeight = 14;
    static constexpr int kControlHeight = 28;
    static constexpr int kRowSpacing = 6;

    // Disabled buttons in this panel render at this opacity so they don't
    // read as active controls (the app LookAndFeel doesn't dim disabled
    // components -- see drawButtonBackground/drawButtonText in
    // src/ui/LookAndFeel.cpp, which draw purely from buttonColourId/
    // textColourOffId with no isEnabled() check). Component::setAlpha()
    // is a Component-level compositing property, so this stays scoped to
    // RecordPanel without touching the shared LookAndFeel.
    static constexpr float kDisabledAlpha = 0.4f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordPanel)
};
