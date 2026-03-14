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

    // FPS / CPU labels (right-aligned in top bar)
    addAndMakeVisible(fpsLabel_);
    addAndMakeVisible(cpuLabel_);
    fpsLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    cpuLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    fpsLabel_.setJustificationType(juce::Justification::centredRight);
    cpuLabel_.setJustificationType(juce::Justification::centredRight);
    startTimerHz(4); // Update stats 4x per second

    // Resolution selector for preview panel
    addAndMakeVisible(resolutionSelector_);
    resolutionSelector_.setTextWhenNothingSelected("Res: Auto");
    {
        int id = 1;
        resolutionSelector_.addItem("Auto", id++);

        // Standard resolutions
        resolutionSelector_.addItem("640x480", id++);
        resolutionSelector_.addItem("800x600", id++);
        resolutionSelector_.addItem("1280x720", id++);
        resolutionSelector_.addItem("1920x1080", id++);
        resolutionSelector_.addItem("2560x1440", id++);
        resolutionSelector_.addItem("3840x2160", id++);

        // Add connected display resolutions
        const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
        for (int i = 0; i < static_cast<int>(displays.size()); ++i)
        {
            const auto& d = displays[static_cast<size_t>(i)];
            juce::String label = juce::String(d.totalArea.getWidth())
                              + "x" + juce::String(d.totalArea.getHeight());
            if (d.isMain)
                label += " (main)";
            else
                label += " (display " + juce::String(i + 1) + ")";

            // Only add if not already a standard resolution
            bool isDuplicate = false;
            for (int j = 0; j < resolutionSelector_.getNumItems(); ++j)
            {
                if (resolutionSelector_.getItemText(j).startsWith(
                    juce::String(d.totalArea.getWidth()) + "x" + juce::String(d.totalArea.getHeight())))
                {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate)
                resolutionSelector_.addItem(label, id++);
        }

        resolutionSelector_.setSelectedId(1, juce::dontSendNotification);
    }
    resolutionSelector_.onChange = [this] {
        juce::String text = resolutionSelector_.getText();
        if (text == "Auto" || text.isEmpty())
        {
            previewPanel_.getRenderer().setLockedResolution(0, 0);
        }
        else
        {
            // Parse "WxH" or "WxH (label)"
            auto xPos = text.indexOfChar('x');
            if (xPos > 0)
            {
                int w = text.substring(0, xPos).getIntValue();
                auto rest = text.substring(xPos + 1);
                auto spacePos = rest.indexOfChar(' ');
                int h = (spacePos > 0) ? rest.substring(0, spacePos).getIntValue()
                                        : rest.getIntValue();
                if (w > 0 && h > 0)
                    previewPanel_.getRenderer().setLockedResolution(w, h);
            }
        }
    };

    // Display selector for output window
    addAndMakeVisible(displaySelector_);
    displaySelector_.setTextWhenNothingSelected("Output: Off");
    refreshDisplayList();
    displaySelector_.onChange = [this] {
        int selected = displaySelector_.getSelectedId();
        if (selected == 1) // "Off"
            closeOutput();
        else if (selected > 1)
            openOutputOnDisplay(selected - 2); // display index
    };

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

    setWantsKeyboardFocus(true);
    setSize(1280, 720);
}

MainComponent::~MainComponent()
{
    outputWindow_.reset(); // Destroy output window before renderer
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

    // Right-aligned stats, display selector, and resolution selector
    cpuLabel_.setBounds(topBar.removeFromRight(80));
    fpsLabel_.setBounds(topBar.removeFromRight(70));
    topBar.removeFromRight(4);
    displaySelector_.setBounds(topBar.removeFromRight(140));
    topBar.removeFromRight(4);
    resolutionSelector_.setBounds(topBar.removeFromRight(130));
    topBar.removeFromRight(8);

    fileLabel_.setBounds(topBar);

    area.removeFromTop(8);

    // Proportional panel layout: left 20%, center 50%, right 30%
    int totalWidth = area.getWidth();
    int leftWidth = std::max(200, static_cast<int>(totalWidth * 0.20f));
    int rightWidth = std::max(200, static_cast<int>(totalWidth * 0.30f));

    // Left panel: audio readouts + spectrum
    auto leftPanel = area.removeFromLeft(leftWidth);
    int spectrumHeight = std::max(120, static_cast<int>(leftPanel.getHeight() * 0.25f));
    audioReadoutPanel_.setBounds(leftPanel.removeFromBottom(spectrumHeight).removeFromBottom(spectrumHeight - 8));
    // Actually: readout on top, spectrum on bottom
    // Re-do: readout gets most of the height, spectrum at bottom
    leftPanel = getLocalBounds().reduced(8);
    leftPanel.removeFromTop(44); // top bar + gap
    leftPanel = leftPanel.removeFromLeft(leftWidth);
    spectrumHeight = std::max(120, static_cast<int>(leftPanel.getHeight() * 0.25f));
    spectrumDisplay_.setBounds(leftPanel.removeFromBottom(spectrumHeight));
    leftPanel.removeFromBottom(8);
    audioReadoutPanel_.setBounds(leftPanel);

    area.removeFromLeft(8);

    // Right panel: effects rack
    auto rightPanel = area.removeFromRight(rightWidth);
    if (effectsRackPanel_)
        effectsRackPanel_->setBounds(rightPanel);

    area.removeFromRight(8);

    // Center: split between preview (top) and waveform (bottom)
    int waveformHeight = std::max(60, static_cast<int>(area.getHeight() * 0.15f));
    auto waveformArea = area.removeFromBottom(waveformHeight);
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
        currentImageFile_ = file;
        if (outputWindow_)
            outputWindow_->loadImage(file);
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

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    auto mod = key.getModifiers();

    // Space = play/pause toggle
    if (key.isKeyCode(juce::KeyPress::spaceKey))
    {
        if (audioEngine_.isPlaying())
            audioEngine_.pause();
        else
            audioEngine_.play();
        return true;
    }

    // Escape = close output window, or stop audio
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        if (outputWindow_ && outputWindow_->isVisible())
        {
            closeOutput();
            displaySelector_.setSelectedId(1, juce::dontSendNotification);
        }
        else
        {
            audioEngine_.stop();
        }
        return true;
    }

    // Cmd/Ctrl+S = save preset
    if (key.isKeyCode('S') && mod.isCommandDown())
    {
        savePreset();
        return true;
    }

    // Cmd/Ctrl+F = fullscreen output on primary display
    if (key.isKeyCode('F') && mod.isCommandDown())
    {
        if (outputWindow_ && outputWindow_->isVisible())
        {
            closeOutput();
            displaySelector_.setSelectedId(1, juce::dontSendNotification);
        }
        else
        {
            openOutputOnDisplay(0);
            displaySelector_.setSelectedId(2, juce::dontSendNotification);
        }
        return true;
    }

    // Cmd/Ctrl+O = load preset
    if (key.isKeyCode('O') && mod.isCommandDown())
    {
        loadPreset();
        return true;
    }

    // 1-9 = toggle effects
    int keyChar = key.getKeyCode();
    if (keyChar >= '1' && keyChar <= '9' && !mod.isCommandDown())
    {
        int effectIdx = keyChar - '1';
        if (effectIdx < previewPanel_.getEffectChain().getNumEffects())
        {
            auto* fx = previewPanel_.getEffectChain().getEffect(effectIdx);
            if (fx != nullptr)
            {
                fx->setEnabled(!fx->isEnabled());
                if (effectsRackPanel_)
                    effectsRackPanel_->refreshFromChain();
            }
        }
        return true;
    }

    return false;
}

bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto file = juce::File(f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" ||
            ext == ".mp3" || ext == ".flac" || ext == ".ogg" ||
            ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".gif" || ext == ".bmp" || ext == ".tiff")
            return true;
    }
    return false;
}

void MainComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (const auto& f : files)
    {
        auto file = juce::File(f);
        auto ext = file.getFileExtension().toLowerCase();

        if (ext == ".wav" || ext == ".aiff" || ext == ".aif" ||
            ext == ".mp3" || ext == ".flac" || ext == ".ogg")
        {
            if (audioEngine_.loadFile(file))
            {
                fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
                audioEngine_.setLooping(loopToggle_.getToggleState());
                audioEngine_.play();
            }
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                 ext == ".gif" || ext == ".bmp" || ext == ".tiff")
        {
            previewPanel_.loadImage(file);
            currentImageFile_ = file;
            if (outputWindow_)
                outputWindow_->loadImage(file);
            fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
        }
    }
}

void MainComponent::timerCallback()
{
    float fps = previewPanel_.getRenderer().getFps();
    float cpu = analysisThread_.getCpuLoad();

    fpsLabel_.setText(juce::String(static_cast<int>(fps + 0.5f)) + " fps",
                      juce::dontSendNotification);
    cpuLabel_.setText("DSP " + juce::String(cpu, 1) + "%",
                      juce::dontSendNotification);
}

void MainComponent::refreshDisplayList()
{
    displaySelector_.clear(juce::dontSendNotification);
    displaySelector_.addItem("Off", 1);

    const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    for (int i = 0; i < static_cast<int>(displays.size()); ++i)
    {
        const auto& d = displays[static_cast<size_t>(i)];
        juce::String label = "Display " + juce::String(i + 1);
        label += " (" + juce::String(d.totalArea.getWidth())
              + "x" + juce::String(d.totalArea.getHeight()) + ")";
        if (d.isMain)
            label += " main";
        displaySelector_.addItem(label, i + 2);
    }

    displaySelector_.setSelectedId(1, juce::dontSendNotification);
}

void MainComponent::openOutputOnDisplay(int displayIndex)
{
    const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
        return;

    if (!outputWindow_)
    {
        outputWindow_ = std::make_unique<OutputWindow>(
            analysisThread_.getFeatureBus(),
            previewPanel_.getMappingEngine(),
            previewPanel_.getEffectChain());

        // Load the same image if one is loaded
        if (currentImageFile_.existsAsFile())
            outputWindow_->loadImage(currentImageFile_);
    }

    outputWindow_->goFullscreenOnDisplay(displays[static_cast<size_t>(displayIndex)]);
}

void MainComponent::closeOutput()
{
    if (outputWindow_)
    {
        outputWindow_->setVisible(false);
        outputWindow_.reset();
    }
}

void MainComponent::updateTransportButtons(bool isPlaying)
{
    playButton_.setEnabled(!isPlaying);
    pauseButton_.setEnabled(isPlaying);
}
