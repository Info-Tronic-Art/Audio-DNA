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

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void openFile();
    void openImage();
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
    juce::Label fileLabel_;

    // Display components
    WaveformDisplay waveformDisplay_{analysisThread_};
    AudioReadoutPanel audioReadoutPanel_{analysisThread_, analysisThread_.getFeatureBus()};
    SpectrumDisplay spectrumDisplay_{analysisThread_.getFeatureBus()};
    PreviewPanel previewPanel_{analysisThread_.getFeatureBus()};

    std::unique_ptr<juce::FileChooser> fileChooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
