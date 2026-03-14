#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&lookAndFeel_);

    // Transport buttons
    addAndMakeVisible(openButton_);
    addAndMakeVisible(playButton_);
    addAndMakeVisible(pauseButton_);
    addAndMakeVisible(stopButton_);
    addAndMakeVisible(fileLabel_);
    addAndMakeVisible(waveformDisplay_);
    addAndMakeVisible(audioReadoutPanel_);
    addAndMakeVisible(spectrumDisplay_);

    fileLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    fileLabel_.setText("No file loaded", juce::dontSendNotification);

    openButton_.onClick = [this] { openFile(); };
    playButton_.onClick = [this] { audioEngine_.play(); };
    pauseButton_.onClick = [this] { audioEngine_.pause(); };
    stopButton_.onClick = [this] { audioEngine_.stop(); };

    audioEngine_.onTransportStateChanged = [this](bool isPlaying) {
        juce::MessageManager::callAsync([this, isPlaying] {
            updateTransportButtons(isPlaying);
        });
    };

    updateTransportButtons(false);

    // Start analysis
    analysisThread_.startThread(juce::Thread::Priority::high);

    setSize(1280, 720);
}

MainComponent::~MainComponent()
{
    analysisThread_.stopThread(1000);
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(AudioDNALookAndFeel::kBackground));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(8);

    // Top bar: transport controls
    auto topBar = area.removeFromTop(36);
    openButton_.setBounds(topBar.removeFromLeft(70));
    topBar.removeFromLeft(8);
    playButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(4);
    pauseButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(4);
    stopButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(12);
    fileLabel_.setBounds(topBar);

    area.removeFromTop(8);

    // Left panel: audio readouts + spectrum
    auto leftPanel = area.removeFromLeft(220);
    audioReadoutPanel_.setBounds(leftPanel.removeFromTop(520));
    leftPanel.removeFromTop(8);
    spectrumDisplay_.setBounds(leftPanel);  // takes remaining space

    // Center: waveform (takes remaining space)
    waveformDisplay_.setBounds(area);
}

void MainComponent::openFile()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select an audio file...",
        juce::File{},
        "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        if (audioEngine_.loadFile(file))
        {
            fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
            audioEngine_.play();
        }
        else
        {
            fileLabel_.setText("Failed to load file", juce::dontSendNotification);
        }
    });
}

void MainComponent::updateTransportButtons(bool isPlaying)
{
    playButton_.setEnabled(!isPlaying);
    pauseButton_.setEnabled(isPlaying);
}
