#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "audio/AudioEngine.h"
#include "audio/RingBuffer.h"
#include "analysis/AnalysisThread.h"
#include "ui/LookAndFeel.h"
#include "ui/WaveformDisplay.h"
#include "ui/AudioReadoutPanel.h"
#include "ui/SpectrumDisplay.h"
#include "ui/PreviewPanel.h"
#include "ui/EffectsRackPanel.h"
#include "effects/EffectLibrary.h"
#include "ui/PresetManager.h"

class MainComponent : public juce::Component,
                      public juce::FileDragAndDropTarget
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    void openFile();
    void openImage();
    void savePreset();
    void loadPreset();
    void updateTransportButtons(bool isPlaying);

    AudioDNALookAndFeel lookAndFeel_;

    // Core audio pipeline
    RingBuffer<float> ringBuffer_{16384};
    AudioEngine audioEngine_{ringBuffer_};
    AnalysisThread analysisThread_{ringBuffer_};

    // Transport controls
    juce::TextButton openButton_{"Open Audio"};
    juce::TextButton openImageButton_{"Open Image"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton pauseButton_{"Pause"};
    juce::TextButton stopButton_{"Stop"};
    juce::ToggleButton loopToggle_{"Loop"};
    juce::TextButton savePresetButton_{"Save"};
    juce::TextButton loadPresetButton_{"Load"};
    juce::Label fileLabel_;

    // Display components
    WaveformDisplay waveformDisplay_{analysisThread_};
    AudioReadoutPanel audioReadoutPanel_{analysisThread_, analysisThread_.getFeatureBus()};
    SpectrumDisplay spectrumDisplay_{analysisThread_.getFeatureBus()};
    PreviewPanel previewPanel_{analysisThread_.getFeatureBus()};

    // Effects rack (right panel) — initialized after previewPanel_
    EffectLibrary effectLibrary_;
    std::unique_ptr<EffectsRackPanel> effectsRackPanel_;

    std::unique_ptr<juce::FileChooser> fileChooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
