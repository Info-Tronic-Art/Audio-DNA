#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&lookAndFeel_);

    // Transport buttons
    addAndMakeVisible(openButton_);
    addAndMakeVisible(openImageButton_);
    addAndMakeVisible(playButton_);
    addAndMakeVisible(pauseButton_);
    addAndMakeVisible(stopButton_);
    addAndMakeVisible(fileLabel_);
    addAndMakeVisible(waveformDisplay_);
    addAndMakeVisible(audioReadoutPanel_);
    addAndMakeVisible(spectrumDisplay_);
    addAndMakeVisible(previewPanel_);

    fileLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    fileLabel_.setText("No file loaded", juce::dontSendNotification);

    addAndMakeVisible(loopToggle_);
    loopToggle_.setToggleState(false, juce::dontSendNotification);
    loopToggle_.onClick = [this] {
        audioEngine_.setLooping(loopToggle_.getToggleState());
    };

    addAndMakeVisible(savePresetButton_);
    addAndMakeVisible(loadPresetButton_);
    savePresetButton_.onClick = [this] { savePreset(); };
    loadPresetButton_.onClick = [this] { loadPreset(); };

    openButton_.onClick = [this] { openFile(); };
    openImageButton_.onClick = [this] { openImage(); };
    playButton_.onClick = [this] { audioEngine_.play(); };
    pauseButton_.onClick = [this] { audioEngine_.pause(); };
    stopButton_.onClick = [this] { audioEngine_.stop(); };

    audioEngine_.onTransportStateChanged = [this](bool isPlaying) {
        juce::MessageManager::callAsync([this, isPlaying] {
            updateTransportButtons(isPlaying);
        });
    };

    updateTransportButtons(false);

    // Initialize effect library
    effectLibrary_.registerDefaults();

    // Create effects rack panel (needs renderer's MappingEngine and EffectChain)
    effectsRackPanel_ = std::make_unique<EffectsRackPanel>(
        previewPanel_.getMappingEngine(),
        previewPanel_.getEffectChain(),
        effectLibrary_);
    addAndMakeVisible(effectsRackPanel_.get());

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
    openButton_.setBounds(topBar.removeFromLeft(90));
    topBar.removeFromLeft(4);
    openImageButton_.setBounds(topBar.removeFromLeft(90));
    topBar.removeFromLeft(8);
    playButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(4);
    pauseButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(4);
    stopButton_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(8);
    loopToggle_.setBounds(topBar.removeFromLeft(60));
    topBar.removeFromLeft(12);
    savePresetButton_.setBounds(topBar.removeFromLeft(50));
    topBar.removeFromLeft(4);
    loadPresetButton_.setBounds(topBar.removeFromLeft(50));
    topBar.removeFromLeft(12);
    fileLabel_.setBounds(topBar);

    area.removeFromTop(8);

    // Left panel: audio readouts + spectrum
    auto leftPanel = area.removeFromLeft(220);
    audioReadoutPanel_.setBounds(leftPanel.removeFromTop(520));
    leftPanel.removeFromTop(8);
    spectrumDisplay_.setBounds(leftPanel); // takes remaining space

    area.removeFromLeft(8);

    // Right panel: effects rack
    auto rightPanel = area.removeFromRight(220);
    if (effectsRackPanel_)
        effectsRackPanel_->setBounds(rightPanel);

    area.removeFromRight(8);

    // Center: split between preview (top) and waveform (bottom)
    auto waveformArea = area.removeFromBottom(100);
    previewPanel_.setBounds(area);             // GL preview takes the bulk
    waveformDisplay_.setBounds(waveformArea);  // waveform at the bottom
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
            audioEngine_.setLooping(loopToggle_.getToggleState());
            audioEngine_.play();
        }
        else
        {
            fileLabel_.setText("Failed to load file", juce::dontSendNotification);
        }
    });
}

void MainComponent::openImage()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select an image file...",
        juce::File{},
        "*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tiff");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        previewPanel_.loadImage(file);
        fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
    });
}

void MainComponent::savePreset()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Save preset...",
        PresetManager::getPresetsDirectory(),
        "*.json");

    auto flags = juce::FileBrowserComponent::saveMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        auto saveFile = file.hasFileExtension(".json") ? file
                            : file.withFileExtension("json");

        if (PresetManager::savePreset(saveFile,
                                       saveFile.getFileNameWithoutExtension(),
                                       previewPanel_.getEffectChain(),
                                       previewPanel_.getMappingEngine()))
        {
            fileLabel_.setText("Saved: " + saveFile.getFileName(),
                              juce::dontSendNotification);
        }
    });
}

void MainComponent::loadPreset()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Load preset...",
        PresetManager::getPresetsDirectory(),
        "*.json");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        if (PresetManager::loadPreset(file,
                                       previewPanel_.getEffectChain(),
                                       previewPanel_.getMappingEngine()))
        {
            fileLabel_.setText("Loaded: " + file.getFileNameWithoutExtension(),
                              juce::dontSendNotification);
            if (effectsRackPanel_)
                effectsRackPanel_->refreshFromChain();
        }
    });
}

void MainComponent::updateTransportButtons(bool isPlaying)
{
    playButton_.setEnabled(!isPlaying);
    pauseButton_.setEnabled(isPlaying);
}
