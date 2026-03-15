#include "MainComponent.h"

MainComponent::MainComponent()
{
    setLookAndFeel(&lookAndFeel_);

    // UI components
    addAndMakeVisible(openImageButton_);
    addAndMakeVisible(fileLabel_);
    addAndMakeVisible(waveformDisplay_);
    addAndMakeVisible(audioReadoutPanel_);
    addAndMakeVisible(spectrumDisplay_);
    addAndMakeVisible(previewPanel_);

    fileLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    fileLabel_.setText("No file loaded", juce::dontSendNotification);

    addAndMakeVisible(savePresetButton_);
    addAndMakeVisible(loadPresetButton_);
    savePresetButton_.onClick = [this] { savePreset(); };
    loadPresetButton_.onClick = [this] { loadPreset(); };

    // Random on Beat label
    addAndMakeVisible(randomLabel_);
    randomLabel_.setText("Random on Beat", juce::dontSendNotification);
    randomLabel_.setFont(juce::Font(juce::FontOptions(11.0f)));
    randomLabel_.setColour(juce::Label::textColourId,
                           juce::Colour(AudioDNALookAndFeel::kTextSecondary));

    // Beat-synced random toggle + beat count selector
    addAndMakeVisible(beatRandomToggle_);
    beatRandomToggle_.setToggleState(false, juce::dontSendNotification);

    addAndMakeVisible(beatCountSelector_);
    beatCountSelector_.addItem("1", 1);
    beatCountSelector_.addItem("2", 2);
    beatCountSelector_.addItem("4", 3);
    beatCountSelector_.addItem("8", 4);
    beatCountSelector_.addItem("16", 5);
    beatCountSelector_.addItem("32", 6);
    beatCountSelector_.setSelectedId(3, juce::dontSendNotification); // default 4 beats
    beatCountSelector_.onChange = [this] {
        static const int counts[] = {1, 2, 4, 8, 16, 32};
        int idx = beatCountSelector_.getSelectedId() - 1;
        if (idx >= 0 && idx < 6)
            beatRandomCount_ = counts[idx];
    };

    // Sync button — resets beat counter to align with current beat
    addAndMakeVisible(syncButton_);
    syncButton_.onClick = [this] {
        beatCounter_ = 0;
        lastBeatPhase_ = 0.0f;
        // Flash the button briefly
        syncButton_.setColour(juce::TextButton::buttonColourId,
                              juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        juce::Timer::callAfterDelay(200, [this] {
            syncButton_.removeColour(juce::TextButton::buttonColourId);
        });
    };

    // Fast save button
    addAndMakeVisible(fastSaveButton_);
    fastSaveButton_.onClick = [this] { fastSave(); };

    // Determine starting fast save counter from existing files
    {
        auto dir = getFastSaveDir();
        if (dir.isDirectory())
        {
            int maxNum = 0;
            for (auto& f : dir.findChildFiles(juce::File::findFiles, false, "FX_Save_*.json"))
            {
                auto name = f.getFileNameWithoutExtension();
                auto numStr = name.fromLastOccurrenceOf("_", false, false);
                int num = numStr.getIntValue();
                if (num > maxNum) maxNum = num;
            }
            fastSaveCounter_ = maxNum + 1;
        }
    }

    // Audio source selector
    addAndMakeVisible(audioSourceSelector_);
    audioSourceSelector_.setTextWhenNothingSelected("File");
    audioSourceSelector_.addItem("Mic Input", 1);
    audioSourceSelector_.addItem("Audio File", 2);
    audioSourceSelector_.setSelectedId(1, juce::dontSendNotification);
    audioEngine_.setSourceMode(AudioEngine::SourceMode::MicInput); // Default to mic
    audioSourceSelector_.onChange = [this] {
        int sel = audioSourceSelector_.getSelectedId();
        if (sel == 1)
        {
            audioEngine_.setSourceMode(AudioEngine::SourceMode::MicInput);
            fileLabel_.setText("Mic: " + audioEngine_.getDeviceStatus(),
                              juce::dontSendNotification);
        }
        else if (sel == 2)
        {
            audioEngine_.setSourceMode(AudioEngine::SourceMode::File);
            // Prompt to load a file if none loaded
            if (!currentAudioFile_.existsAsFile())
            {
                fileChooser_ = std::make_unique<juce::FileChooser>(
                    "Select an audio file...", juce::File{},
                    "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg");
                auto flags = juce::FileBrowserComponent::openMode
                           | juce::FileBrowserComponent::canSelectFiles;
                fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
                    auto file = fc.getResult();
                    if (file == juce::File{}) return;
                    if (audioEngine_.loadFile(file))
                    {
                        currentAudioFile_ = file;
                        fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
                        audioEngine_.play();
                    }
                });
            }
            else
            {
                fileLabel_.setText(currentAudioFile_.getFileName(), juce::dontSendNotification);
                audioEngine_.play();
            }
        }
    };

    // Input gain slider
    addAndMakeVisible(inputGainSlider_);
    inputGainSlider_.setRange(0.0, 4.0, 0.01);
    inputGainSlider_.setValue(1.0, juce::dontSendNotification);
    inputGainSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    inputGainSlider_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 35, 20);
    inputGainSlider_.onValueChange = [this] {
        audioEngine_.setInputGain(static_cast<float>(inputGainSlider_.getValue()));
    };
    addAndMakeVisible(inputGainLabel_);

    // Dropdown labels
    auto setupLabel = [this](juce::Label& label, const juce::String& text) {
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(10.0f)));
        label.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));
        label.setJustificationType(juce::Justification::centredRight);
    };
    setupLabel(viewportLabel_,    "Viewport");
    setupLabel(outputLabel_,     "Output");
  #if AUDIODNA_HAS_CAMERA
    setupLabel(cameraLabel_,     "Camera");
  #endif
    setupLabel(audioSourceLabel_, "Audio Source");
    setupLabel(inputGainLabel_,   "Gain");
    setupLabel(masterLevelLabel_, "Video Level");
    setupLabel(imageBeatLabel_,   "Beats per Image");

  #if AUDIODNA_HAS_CAMERA
    // Camera input selector
    addAndMakeVisible(cameraSelector_);
    cameraSelector_.setTextWhenNothingSelected("Cam: Off");
    refreshCameraList();
    cameraSelector_.onChange = [this] {
        int selected = cameraSelector_.getSelectedId();
        if (selected == 1) // "Off"
            closeCamera();
        else if (selected > 1)
            openCamera(selected - 2);
    };
  #endif

    // Deck save/load
    addAndMakeVisible(deckSaveButton_);
    addAndMakeVisible(deckLoadButton_);
    deckSaveButton_.onClick = [this] { saveDeck(); };
    deckLoadButton_.onClick = [this] { loadDeck(); };

    // Bottom preset slots (10 buttons + dropdowns)
    for (int i = 0; i < kNumSlots; ++i)
    {
        auto& slot = presetSlots_[static_cast<size_t>(i)];
        slot.button = std::make_unique<juce::TextButton>(juce::String(i + 1));
        slot.button->setColour(juce::TextButton::buttonColourId,
                               juce::Colour(AudioDNALookAndFeel::kSurface));
        addAndMakeVisible(slot.button.get());

        int capturedSlot = i;
        slot.button->onClick = [this, capturedSlot] {
            auto& s = presetSlots_[static_cast<size_t>(capturedSlot)];
            if (s.loadedFile.existsAsFile())
            {
                if (PresetManager::loadPreset(s.loadedFile,
                                               previewPanel_.getEffectChain(),
                                               previewPanel_.getMappingEngine()))
                {
                    fileLabel_.setText("Slot " + juce::String(capturedSlot + 1) + ": "
                                      + s.loadedFile.getFileNameWithoutExtension(),
                                      juce::dontSendNotification);
                    if (effectsRackPanel_)
                        effectsRackPanel_->refreshFromChain();
                }
            }
        };

        slot.dropdown = std::make_unique<juce::ComboBox>();
        slot.dropdown->setTextWhenNothingSelected("--");
        addAndMakeVisible(slot.dropdown.get());
        populateSlotMenu(i);

        slot.dropdown->onChange = [this, capturedSlot] {
            auto& s = presetSlots_[static_cast<size_t>(capturedSlot)];
            int selected = s.dropdown->getSelectedId();
            if (selected > 0)
            {
                auto dir = getFastSaveDir();
                auto files = dir.findChildFiles(juce::File::findFiles, false, "*.json");
                files.sort();
                int idx = selected - 1;
                if (idx < static_cast<int>(files.size()))
                {
                    s.loadedFile = files[idx];
                    s.button->setButtonText(files[idx].getFileNameWithoutExtension()
                                            .replace("FX_Save_", "FX"));
                    s.button->setColour(juce::TextButton::buttonColourId,
                                        juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.4f));
                }
            }
        };
    }

    // FPS / CPU labels (right-aligned in top bar)
    addAndMakeVisible(fpsLabel_);
    addAndMakeVisible(cpuLabel_);
    fpsLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    cpuLabel_.setColour(juce::Label::textColourId,
                        juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    fpsLabel_.setJustificationType(juce::Justification::centredRight);
    cpuLabel_.setJustificationType(juce::Justification::centredRight);
    startTimerHz(30); // 30Hz for beat-synced randomization + UI updates

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

    openImageButton_.onClick = [this] { openImage(); };

    // Image folder + beat selector
    addAndMakeVisible(openFolderButton_);
    openFolderButton_.onClick = [this] { openImageFolder(); };

    addAndMakeVisible(imageBeatSelector_);
    {
        int id = 1;
        for (int b = 2; b <= 128; b *= 2)
            imageBeatSelector_.addItem(juce::String(b), id++);
    }
    imageBeatSelector_.setSelectedId(4, juce::dontSendNotification); // default 8
    imageBeatSelector_.onChange = [this] {
        static const int beats[] = {2, 4, 8, 16, 32, 64, 128};
        int idx = imageBeatSelector_.getSelectedId() - 1;
        if (idx >= 0 && idx < 7)
            slideshowBeats_ = beats[idx];
    };
    addAndMakeVisible(imageBeatLabel_);

    // Master video level slider
    addAndMakeVisible(masterLevelSlider_);
    masterLevelSlider_.setRange(0.0, 1.0, 0.01);
    masterLevelSlider_.setValue(1.0, juce::dontSendNotification);
    masterLevelSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    masterLevelSlider_.onValueChange = [this] {
        previewPanel_.getRenderer().setMasterLevel(static_cast<float>(masterLevelSlider_.getValue()));
    };
    addAndMakeVisible(masterLevelLabel_);

    audioEngine_.onError = [this](const juce::String& msg) {
        juce::MessageManager::callAsync([this, msg] {
            fileLabel_.setText(msg, juce::dontSendNotification);
        });
    };

    if (!audioEngine_.hasAudioDevice())
        fileLabel_.setText("No audio device found", juce::dontSendNotification);

    // Initialize effect library
    effectLibrary_.registerDefaults();

    // Create effects rack panel (needs renderer's MappingEngine and EffectChain)
    effectsRackPanel_ = std::make_unique<EffectsRackPanel>(
        previewPanel_.getMappingEngine(),
        previewPanel_.getEffectChain(),
        effectLibrary_);
    addAndMakeVisible(effectsRackPanel_.get());

    // === Keyboard Launcher Panel ===
    keyboardPanel_ = std::make_unique<KeyboardPanel>(keyboardLayout_);
    addAndMakeVisible(keyboardPanel_.get());
    keyboardPanel_->onKeyClicked = [this](KeySlot& key) { openKeyEditor(key); };

    // Wire keyboard layout to renderer for compositing
    previewPanel_.getRenderer().setKeyboardLayout(&keyboardLayout_);

    // === Key Editor (hidden by default) ===
    keyEditor_ = std::make_unique<KeyEditor>(effectLibrary_);
    addChildComponent(keyEditor_.get()); // hidden initially
    keyEditor_->onClose = [this] { closeKeyEditor(); };
    keyEditor_->onRequestImage = [this](KeySlot& key) { assignImageToKey(key); };

    // === Collapsible Panel Toggle Buttons ===
    auto setupToggle = [this](juce::TextButton& btn, bool& state, const juce::String& label) {
        addAndMakeVisible(btn);
        btn.setClickingTogglesState(true);
        btn.setToggleState(state, juce::dontSendNotification);
        btn.setButtonText(label);
        btn.setColour(juce::TextButton::buttonOnColourId,
                       juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f));
        btn.onClick = [this, &state, &btn] {
            state = btn.getToggleState();
            resized();
        };
    };
    setupToggle(toggleAudioBtn_, showAudioPanel_, "A");
    setupToggle(toggleFxBtn_, showFxPanel_, "FX");
    setupToggle(toggleWaveBtn_, showWavePanel_, "W");
    setupToggle(toggleKeysBtn_, showKeysPanel_, "K");
    setupToggle(togglePresetsBtn_, showPresetsPanel_, "P");

    // Start analysis
    analysisThread_.startThread(juce::Thread::Priority::high);

    setWantsKeyboardFocus(true);
    // Register as key listener on top-level component to catch keys globally
    addKeyListener(this);
    setSize(1280, 800);
}

MainComponent::~MainComponent()
{
#if AUDIODNA_HAS_CAMERA
    closeCamera();
#endif
    outputWindow_.reset();
    analysisThread_.stopThread(1000);
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(AudioDNALookAndFeel::kBackground));

    // Draw input level meter
    if (!inputLevelMeterBounds_.isEmpty())
    {
        auto b = inputLevelMeterBounds_.toFloat();
        // Background
        g.setColour(juce::Colour(0xff1a1a2e));
        g.fillRoundedRectangle(b, 2.0f);

        // Level bar
        float level = std::min(1.0f, audioEngine_.getInputLevel());
        if (level > 0.001f)
        {
            auto bar = b.withWidth(b.getWidth() * level);
            // Green → yellow → red
            juce::Colour col = level < 0.6f ? juce::Colour(0xff00cc66)
                             : level < 0.85f ? juce::Colour(0xffcccc00)
                             : juce::Colour(0xffcc3333);
            g.setColour(col);
            g.fillRoundedRectangle(bar, 2.0f);
        }

        // Border
        g.setColour(juce::Colour(0xff333355));
        g.drawRoundedRectangle(b, 2.0f, 1.0f);
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(8);

    // === Row 1: Image + Camera + Audio Source + Presets ===
    auto row1 = area.removeFromTop(30);
    openImageButton_.setBounds(row1.removeFromLeft(75));
    row1.removeFromLeft(3);
    openFolderButton_.setBounds(row1.removeFromLeft(80));
    row1.removeFromLeft(2);
    imageBeatLabel_.setBounds(row1.removeFromLeft(85));
    imageBeatSelector_.setBounds(row1.removeFromLeft(45));
    row1.removeFromLeft(6);
  #if AUDIODNA_HAS_CAMERA
    cameraLabel_.setBounds(row1.removeFromLeft(42));
    cameraSelector_.setBounds(row1.removeFromLeft(100));
    row1.removeFromLeft(8);
  #endif
    audioSourceLabel_.setBounds(row1.removeFromLeft(70));
    audioSourceSelector_.setBounds(row1.removeFromLeft(90));
    row1.removeFromLeft(6);
    savePresetButton_.setBounds(row1.removeFromLeft(45));
    row1.removeFromLeft(2);
    loadPresetButton_.setBounds(row1.removeFromLeft(45));
    row1.removeFromLeft(4);
    fastSaveButton_.setBounds(row1.removeFromLeft(55));
    row1.removeFromLeft(4);
    deckSaveButton_.setBounds(row1.removeFromLeft(65));
    row1.removeFromLeft(2);
    deckLoadButton_.setBounds(row1.removeFromLeft(65));

    // Right side of row 1: stats
    cpuLabel_.setBounds(row1.removeFromRight(80));
    fpsLabel_.setBounds(row1.removeFromRight(80));

    fileLabel_.setBounds(row1);

    area.removeFromTop(3);

    // === Row 2: Camera (aligned under audio source) + Tools + Selectors ===
    auto row2 = area.removeFromTop(26);

    // Random on Beat controls (left side)
    randomLabel_.setBounds(row2.removeFromLeft(95));
    row2.removeFromLeft(2);
    beatRandomToggle_.setBounds(row2.removeFromLeft(60));
    beatCountSelector_.setBounds(row2.removeFromLeft(40));
    row2.removeFromLeft(2);
    syncButton_.setBounds(row2.removeFromLeft(38));
    row2.removeFromLeft(10);

    // Input gain + level meter
    inputGainLabel_.setBounds(row2.removeFromLeft(30));
    inputGainSlider_.setBounds(row2.removeFromLeft(100));
    row2.removeFromLeft(4);
    inputLevelMeterBounds_ = row2.removeFromLeft(60).reduced(0, 4);
    row2.removeFromLeft(8);

    // Right-aligned: Video Level, Output, Viewport
    masterLevelSlider_.setBounds(row2.removeFromRight(80));
    masterLevelLabel_.setBounds(row2.removeFromRight(60));
    row2.removeFromRight(6);
    displaySelector_.setBounds(row2.removeFromRight(120));
    outputLabel_.setBounds(row2.removeFromRight(40));
    row2.removeFromRight(6);
    resolutionSelector_.setBounds(row2.removeFromRight(100));
    viewportLabel_.setBounds(row2.removeFromRight(48));
    row2.removeFromRight(8);

    area.removeFromTop(4);

    // === Toggle buttons row ===
    auto toggleRow = area.removeFromTop(20);
    toggleAudioBtn_.setBounds(toggleRow.removeFromLeft(24));
    toggleRow.removeFromLeft(2);
    toggleFxBtn_.setBounds(toggleRow.removeFromLeft(24));
    toggleRow.removeFromLeft(2);
    toggleWaveBtn_.setBounds(toggleRow.removeFromLeft(24));
    toggleRow.removeFromLeft(2);
    toggleKeysBtn_.setBounds(toggleRow.removeFromLeft(24));
    toggleRow.removeFromLeft(2);
    togglePresetsBtn_.setBounds(toggleRow.removeFromLeft(24));
    area.removeFromTop(4);

    // === Bottom sections (keyboard + presets) — allocate from bottom up ===

    // Preset slots bar
    if (showPresetsPanel_)
    {
        auto slotBar = area.removeFromBottom(28);
        int slotWidth = slotBar.getWidth() / kNumSlots;
        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& slot = presetSlots_[static_cast<size_t>(i)];
            auto slotArea = slotBar.removeFromLeft(slotWidth);
            int btnWidth = slotArea.getWidth() / 3;
            slot.button->setBounds(slotArea.removeFromLeft(btnWidth));
            slotArea.removeFromLeft(2);
            slot.dropdown->setBounds(slotArea);
            slot.button->setVisible(true);
            slot.dropdown->setVisible(true);
        }
        area.removeFromBottom(4);
    }
    else
    {
        for (int i = 0; i < kNumSlots; ++i)
        {
            presetSlots_[static_cast<size_t>(i)].button->setVisible(false);
            presetSlots_[static_cast<size_t>(i)].dropdown->setVisible(false);
        }
    }

    // Keyboard panel
    if (showKeysPanel_ && keyboardPanel_)
    {
        int keysHeight = std::min(200, std::max(120, area.getHeight() / 4));
        keyboardPanel_->setBounds(area.removeFromBottom(keysHeight));
        keyboardPanel_->setVisible(true);
        area.removeFromBottom(4);
    }
    else if (keyboardPanel_)
    {
        keyboardPanel_->setVisible(false);
    }

    // === Main content area (left + center + right) ===
    int totalWidth = area.getWidth();
    int leftWidth = showAudioPanel_ ? std::max(200, static_cast<int>(totalWidth * 0.20f)) : 0;
    int rightWidth = showFxPanel_ ? std::max(200, static_cast<int>(totalWidth * 0.30f)) : 0;

    // Left panel: audio readouts + spectrum
    if (showAudioPanel_)
    {
        auto leftPanel = area.removeFromLeft(leftWidth);
        int spectrumHeight = std::max(120, static_cast<int>(leftPanel.getHeight() * 0.25f));
        spectrumDisplay_.setBounds(leftPanel.removeFromBottom(spectrumHeight));
        leftPanel.removeFromBottom(8);
        audioReadoutPanel_.setBounds(leftPanel);
        spectrumDisplay_.setVisible(true);
        audioReadoutPanel_.setVisible(true);
        area.removeFromLeft(8);
    }
    else
    {
        spectrumDisplay_.setVisible(false);
        audioReadoutPanel_.setVisible(false);
    }

    // Right panel: effects rack
    if (showFxPanel_ && effectsRackPanel_)
    {
        auto rightPanel = area.removeFromRight(rightWidth);
        effectsRackPanel_->setBounds(rightPanel);
        effectsRackPanel_->setVisible(true);
        area.removeFromRight(8);
    }
    else if (effectsRackPanel_)
    {
        effectsRackPanel_->setVisible(false);
    }

    // Center: preview + optional waveform
    if (showWavePanel_)
    {
        int waveformHeight = std::max(60, static_cast<int>(area.getHeight() * 0.12f));
        auto waveformArea = area.removeFromBottom(waveformHeight);
        waveformDisplay_.setBounds(waveformArea);
        waveformDisplay_.setVisible(true);
    }
    else
    {
        waveformDisplay_.setVisible(false);
    }

    previewPanel_.setBounds(area); // GL preview takes remaining space

    // Key Editor overlay — positioned above keyboard panel or at bottom
    if (showKeyEditor_ && keyEditor_)
    {
        int editorHeight = 280;
        auto editorBounds = getLocalBounds().reduced(8);
        editorBounds = editorBounds.removeFromBottom(editorHeight);
        keyEditor_->setBounds(editorBounds);
        keyEditor_->toFront(true);
    }
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

    // Escape = close key editor first, then output window
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        if (showKeyEditor_)
        {
            closeKeyEditor();
            return true;
        }
        if (outputWindow_ && outputWindow_->isVisible())
        {
            closeOutput();
            displaySelector_.setSelectedId(1, juce::dontSendNotification);
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

    // === Keyboard Launcher keys (when panel is visible) ===
    if (showKeysPanel_ && !mod.isCommandDown())
    {
        int keyCode = key.getKeyCode();
        char c = static_cast<char>(std::toupper(keyCode));

        // Shift+key = latch toggle regardless of latch setting
        if (mod.isShiftDown())
        {
            auto* slot = keyboardLayout_.findByKeyCode(keyCode, true);
            if (slot && !slot->isEmpty())
            {
                if (slot->active)
                {
                    keyboardLayout_.deactivateKey(*slot);
                }
                else
                {
                    keyboardLayout_.activateKey(*slot);
                    slot->shiftLatched = true;
                }
                if (keyboardPanel_)
                {
                    if (slot->active)
                        keyboardPanel_->keyActivated(*slot);
                    else
                        keyboardPanel_->keyDeactivated(*slot);
                }
                return true;
            }
        }

        handleKeySlotTrigger(c, true);
        return true;
    }

    return false;
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    // Global key listener — catches shift+key for latch from any focus context
    auto mod = key.getModifiers();
    if (showKeysPanel_ && mod.isShiftDown() && !mod.isCommandDown())
    {
        auto* slot = keyboardLayout_.findByKeyCode(key.getKeyCode(), true);
        if (slot && !slot->isEmpty())
        {
            if (slot->active)
            {
                keyboardLayout_.deactivateKey(*slot);
            }
            else
            {
                keyboardLayout_.activateKey(*slot);
                slot->shiftLatched = true;
            }
            if (keyboardPanel_)
            {
                if (slot->active)
                    keyboardPanel_->keyActivated(*slot);
                else
                    keyboardPanel_->keyDeactivated(*slot);
            }
            return true;
        }
    }
    return false;
}

bool MainComponent::keyStateChanged(bool /*isKeyDown*/)
{
    // Handle key releases for momentary mode in keyboard launcher
    if (!showKeysPanel_)
        return false;

    // Check all launcher keys for release
    for (auto& key : keyboardLayout_.keys)
    {
        if (!key.active || key.latched || key.shiftLatched)
            continue;

        // Check if the physical key is still held
        bool stillHeld = juce::KeyPress::isKeyCurrentlyDown(std::tolower(key.keyChar));
        // Also check for number row
        if (key.row == 0)
            stillHeld = juce::KeyPress::isKeyCurrentlyDown(key.keyChar);

        if (!stillHeld)
        {
            keyboardLayout_.deactivateKey(key);
            if (keyboardPanel_)
                keyboardPanel_->keyDeactivated(key);
        }
    }
    return false;
}

void MainComponent::handleKeySlotTrigger(char keyChar, bool isDown)
{
    auto* slot = keyboardLayout_.findByChar(keyChar);
    if (slot == nullptr || slot->isEmpty())
        return;

    if (isDown)
    {
        if (slot->latched)
        {
            keyboardLayout_.toggleKey(*slot);
        }
        else
        {
            if (!slot->active)
                keyboardLayout_.activateKey(*slot);
        }
        if (keyboardPanel_)
        {
            if (slot->active)
                keyboardPanel_->keyActivated(*slot);
            else
                keyboardPanel_->keyDeactivated(*slot);
        }
    }
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
                currentAudioFile_ = file;
                audioSourceSelector_.setSelectedId(2, juce::dontSendNotification);
                audioEngine_.setSourceMode(AudioEngine::SourceMode::File);
                fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
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
    // Update FPS/CPU labels at ~4Hz (every 8th call at 30Hz)
    if (++uiUpdateCounter_ >= 8)
    {
        uiUpdateCounter_ = 0;
        float fps = previewPanel_.getRenderer().getFps();
        float cpu = analysisThread_.getCpuLoad();
        float frameMs = previewPanel_.getRenderer().getFrameTimeMs();
        fpsLabel_.setText(juce::String(static_cast<int>(fps + 0.5f)) + "fps "
                          + juce::String(frameMs, 1) + "ms",
                          juce::dontSendNotification);
        cpuLabel_.setText("DSP " + juce::String(cpu, 1) + "%",
                          juce::dontSendNotification);
    }

    // Repaint input level meter
    if (!inputLevelMeterBounds_.isEmpty())
        repaint(inputLevelMeterBounds_);

    // Beat-synced randomization (runs at 30Hz for accurate beat detection)
    if (beatRandomToggle_.getToggleState())
        beatSyncRandomize();

    // Image slideshow advance
    if (!slideshowImages_.isEmpty())
        advanceSlideshow();
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

juce::File MainComponent::getFastSaveDir() const
{
    return PresetManager::getPresetsDirectory().getChildFile("fast_saves");
}

void MainComponent::fastSave()
{
    auto dir = getFastSaveDir();
    dir.createDirectory();

    auto fileName = "FX_Save_" + juce::String(fastSaveCounter_) + ".json";
    auto file = dir.getChildFile(fileName);

    if (PresetManager::savePreset(file,
                                   file.getFileNameWithoutExtension(),
                                   previewPanel_.getEffectChain(),
                                   previewPanel_.getMappingEngine()))
    {
        fileLabel_.setText("Saved: " + fileName, juce::dontSendNotification);
        ++fastSaveCounter_;

        // Refresh all slot dropdown menus
        for (int i = 0; i < kNumSlots; ++i)
            populateSlotMenu(i);

        // Flash the button
        fastSaveButton_.setColour(juce::TextButton::buttonColourId,
                                  juce::Colour(AudioDNALookAndFeel::kAccentCyan));
        juce::Timer::callAfterDelay(300, [this] {
            fastSaveButton_.removeColour(juce::TextButton::buttonColourId);
        });
    }
}

void MainComponent::loadSlotPreset(int slot, const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    if (PresetManager::loadPreset(file,
                                   previewPanel_.getEffectChain(),
                                   previewPanel_.getMappingEngine()))
    {
        fileLabel_.setText("Slot " + juce::String(slot + 1) + ": "
                          + file.getFileNameWithoutExtension(),
                          juce::dontSendNotification);
        if (effectsRackPanel_)
            effectsRackPanel_->refreshFromChain();
    }
}

void MainComponent::populateSlotMenu(int slot)
{
    auto& s = presetSlots_[static_cast<size_t>(slot)];

    // Remember what file was selected before repopulating
    auto previousFile = s.loadedFile;

    s.dropdown->clear(juce::dontSendNotification);

    auto dir = getFastSaveDir();
    if (!dir.isDirectory())
        return;

    auto files = dir.findChildFiles(juce::File::findFiles, false, "*.json");
    files.sort();

    int restoreId = 0;
    for (int i = 0; i < static_cast<int>(files.size()); ++i)
    {
        s.dropdown->addItem(files[i].getFileNameWithoutExtension(), i + 1);
        if (previousFile == files[i])
            restoreId = i + 1;
    }

    // Restore previous selection
    if (restoreId > 0)
        s.dropdown->setSelectedId(restoreId, juce::dontSendNotification);
}

void MainComponent::saveDeck()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Save deck...",
        PresetManager::getPresetsDirectory(),
        "*.deck.json");

    auto flags = juce::FileBrowserComponent::saveMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        auto saveFile = file.hasFileExtension(".deck.json") ? file
                            : juce::File(file.getFullPathName() + ".deck.json");

        PresetManager::DeckState deck;
        deck.audioFile = currentAudioFile_;
        deck.imageFile = currentImageFile_;

        // Collect slot assignments
        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& s = presetSlots_[static_cast<size_t>(i)];
            deck.slotFiles.add(s.loadedFile.getFullPathName());
        }

        if (PresetManager::saveDeck(saveFile, deck,
                                     previewPanel_.getEffectChain(),
                                     previewPanel_.getMappingEngine()))
        {
            fileLabel_.setText("Deck saved: " + saveFile.getFileNameWithoutExtension(),
                              juce::dontSendNotification);
        }
    });
}

void MainComponent::loadDeck()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Load deck...",
        PresetManager::getPresetsDirectory(),
        "*.deck.json");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        PresetManager::DeckState deck;
        if (!PresetManager::loadDeck(file, deck,
                                      previewPanel_.getEffectChain(),
                                      previewPanel_.getMappingEngine()))
        {
            fileLabel_.setText("Failed to load deck", juce::dontSendNotification);
            return;
        }

        // Load audio
        if (deck.audioFile.existsAsFile())
        {
            if (audioEngine_.loadFile(deck.audioFile))
            {
                currentAudioFile_ = deck.audioFile;
                audioEngine_.setSourceMode(AudioEngine::SourceMode::File);
                audioSourceSelector_.setSelectedId(2, juce::dontSendNotification);
                audioEngine_.play();
            }
        }

        // Load image
        if (deck.imageFile.existsAsFile())
        {
            previewPanel_.loadImage(deck.imageFile);
            currentImageFile_ = deck.imageFile;
            if (outputWindow_)
                outputWindow_->loadImage(deck.imageFile);
        }

        // Restore slot assignments
        for (int i = 0; i < kNumSlots && i < deck.slotFiles.size(); ++i)
        {
            auto& s = presetSlots_[static_cast<size_t>(i)];
            auto slotFile = juce::File(deck.slotFiles[i]);
            if (slotFile.existsAsFile())
            {
                s.loadedFile = slotFile;
                s.button->setButtonText(slotFile.getFileNameWithoutExtension()
                                        .replace("FX_Save_", "FX"));
                s.button->setColour(juce::TextButton::buttonColourId,
                                    juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.4f));
            }
            else
            {
                s.loadedFile = juce::File();
                s.button->setButtonText(juce::String(i + 1));
                s.button->removeColour(juce::TextButton::buttonColourId);
            }
            populateSlotMenu(i);
        }

        if (effectsRackPanel_)
            effectsRackPanel_->refreshFromChain();

        fileLabel_.setText("Deck: " + file.getFileNameWithoutExtension(),
                          juce::dontSendNotification);
    });
}

#if AUDIODNA_HAS_CAMERA
void MainComponent::imageReceived(const juce::Image& image)
{
    // Called from camera thread — queue frame for GL thread
    previewPanel_.queueCameraFrame(image);

    // Also send to output window if active
    if (outputWindow_)
        outputWindow_->getRenderer().queueCameraFrame(image);
}
#endif

void MainComponent::openImageFolder()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select image folder...", juce::File{});

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectDirectories;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto dir = fc.getResult();
        if (!dir.isDirectory())
            return;

        slideshowImages_.clear();
        for (const auto& f : dir.findChildFiles(juce::File::findFiles, false,
                "*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tiff"))
        {
            slideshowImages_.add(f);
        }
        slideshowImages_.sort();

        if (slideshowImages_.isEmpty())
        {
            fileLabel_.setText("No images found in folder", juce::dontSendNotification);
            return;
        }

        slideshowIndex_ = 0;
        slideshowBeatCounter_ = 0;
        lastSlideshowBeatPhase_ = 0.0f;

        // Load first image
        auto first = slideshowImages_[0];
        previewPanel_.loadImage(first);
        currentImageFile_ = first;
        if (outputWindow_)
            outputWindow_->loadImage(first);

        fileLabel_.setText("Folder: " + dir.getFileName() + " ("
                          + juce::String(slideshowImages_.size()) + " images)",
                          juce::dontSendNotification);
    });
}

void MainComponent::advanceSlideshow()
{
    if (slideshowImages_.isEmpty())
        return;

    // Read beat phase
    const auto* snap = analysisThread_.getFeatureBus().acquireRead();
    if (!snap)
        snap = analysisThread_.getFeatureBus().getLatestRead();
    if (!snap)
        return;

    float phase = snap->beatPhase;

    // Detect beat wrap
    if (phase < lastSlideshowBeatPhase_ - 0.5f)
    {
        ++slideshowBeatCounter_;
        if (slideshowBeatCounter_ >= slideshowBeats_)
        {
            slideshowBeatCounter_ = 0;
            slideshowIndex_ = (slideshowIndex_ + 1) % slideshowImages_.size();

            auto img = slideshowImages_[slideshowIndex_];
            previewPanel_.loadImage(img);
            currentImageFile_ = img;
            if (outputWindow_)
                outputWindow_->loadImage(img);
        }
    }
    lastSlideshowBeatPhase_ = phase;
}

#if AUDIODNA_HAS_CAMERA
void MainComponent::refreshCameraList()
{
    cameraSelector_.clear(juce::dontSendNotification);
    cameraSelector_.addItem("Off", 1);

    auto devices = juce::CameraDevice::getAvailableDevices();
    for (int i = 0; i < devices.size(); ++i)
        cameraSelector_.addItem(devices[i], i + 2);

    cameraSelector_.setSelectedId(1, juce::dontSendNotification);
}

void MainComponent::openCamera(int deviceIndex)
{
    closeCamera();

    auto devices = juce::CameraDevice::getAvailableDevices();
    if (deviceIndex < 0 || deviceIndex >= devices.size())
        return;

    std::cerr << "[Camera] Opening: " << devices[deviceIndex] << std::endl;

    cameraDevice_.reset(juce::CameraDevice::openDevice(deviceIndex,
        0, 0, // min size (0 = default)
        1920, 1080, // max size
        false)); // don't use high quality stills

    if (cameraDevice_ == nullptr)
    {
        fileLabel_.setText("Camera failed to open", juce::dontSendNotification);
        return;
    }

    cameraActive_ = true;

    // Add a listener that receives frames
    cameraDevice_->addListener(this);

    fileLabel_.setText("Camera: " + devices[deviceIndex], juce::dontSendNotification);
}

void MainComponent::closeCamera()
{
    if (cameraDevice_)
    {
        cameraDevice_->removeListener(this);
        cameraDevice_.reset();
    }
    cameraActive_ = false;
}
#endif

void MainComponent::randomizeAllEffects()
{
    auto& chain = previewPanel_.getEffectChain();
    auto& mapping = previewPanel_.getMappingEngine();
    juce::Random rng;

    int numEffects = chain.getNumEffects();

    // Randomly enable 3-7 effects
    int numToEnable = rng.nextInt({3, 8});

    // Disable unlocked effects, remove unlocked mappings
    for (int i = 0; i < numEffects; ++i)
    {
        if (effectsRackPanel_ && effectsRackPanel_->isEffectLocked(i))
            continue;
        auto* fx = chain.getEffect(i);
        if (fx) fx->setEnabled(false);
    }

    // Remove mappings for unlocked effects only
    for (int i = mapping.getNumMappings() - 1; i >= 0; --i)
    {
        auto* m = mapping.getMapping(i);
        if (m && effectsRackPanel_ && !effectsRackPanel_->isEffectLocked(static_cast<int>(m->targetEffectId)))
            mapping.removeMapping(i);
    }

    // Randomly enable some effects with random params and mappings
    // Useful audio sources for random mapping (skip MFCCs/Chromas)
    static const MappingSource usefulSources[] = {
        MappingSource::RMS, MappingSource::Peak, MappingSource::SpectralCentroid,
        MappingSource::SpectralFlux, MappingSource::SpectralFlatness,
        MappingSource::BandSub, MappingSource::BandBass, MappingSource::BandLowMid,
        MappingSource::BandMid, MappingSource::BandHighMid, MappingSource::BandPresence,
        MappingSource::OnsetStrength, MappingSource::BeatPhase,
        MappingSource::TransientDensity, MappingSource::HarmonicChange,
        MappingSource::DynamicRange
    };
    int numSources = static_cast<int>(sizeof(usefulSources) / sizeof(usefulSources[0]));

    static const MappingCurve curves[] = {
        MappingCurve::Linear, MappingCurve::Exponential,
        MappingCurve::Logarithmic, MappingCurve::SCurve
    };

    for (int enabled = 0, attempts = 0; enabled < numToEnable && attempts < numEffects * 3; ++attempts)
    {
        int idx = rng.nextInt(numEffects);
        if (effectsRackPanel_ && effectsRackPanel_->isEffectLocked(idx))
            continue;
        auto* fx = chain.getEffect(idx);
        if (fx && !fx->isEnabled())
        {
            fx->setEnabled(true);

            // Randomize parameters
            for (int p = 0; p < fx->getNumParams(); ++p)
                fx->setParamValue(p, rng.nextFloat());

            // Create a random mapping for the first 1-2 params
            int paramsToMap = rng.nextInt({1, std::min(3, fx->getNumParams() + 1)});
            for (int p = 0; p < paramsToMap; ++p)
            {
                Mapping m;
                m.source = usefulSources[rng.nextInt(numSources)];
                m.targetEffectId = static_cast<uint32_t>(idx);
                m.targetParamIndex = static_cast<uint32_t>(p);
                m.curve = curves[rng.nextInt(4)];
                m.inputMin = 0.0f;
                m.inputMax = 1.0f;
                m.outputMin = rng.nextFloat() * 0.3f;
                m.outputMax = 0.5f + rng.nextFloat() * 0.5f;
                m.smoothing = rng.nextFloat() * 0.4f;
                m.enabled = true;
                mapping.addMapping(m);
            }

            ++enabled;
        }
    }

    if (effectsRackPanel_)
        effectsRackPanel_->refreshFromChain();
}

void MainComponent::beatSyncRandomize()
{
    // Read current beat phase from the feature bus
    const auto* snap = analysisThread_.getFeatureBus().acquireRead();
    if (!snap)
        snap = analysisThread_.getFeatureBus().getLatestRead();
    if (!snap)
        return;

    float phase = snap->beatPhase;

    // Detect beat: phase wrapped around (went from high to low)
    if (phase < lastBeatPhase_ - 0.5f)
    {
        ++beatCounter_;
        if (beatCounter_ >= beatRandomCount_)
        {
            beatCounter_ = 0;
            juce::MessageManager::callAsync([this] { randomizeAllEffects(); });
        }
    }
    lastBeatPhase_ = phase;
}

void MainComponent::openKeyEditor(KeySlot& key)
{
    if (keyEditor_)
    {
        keyEditor_->setKey(&key);
        showKeyEditor_ = true;
        keyEditor_->setVisible(true);
        resized();
        // Keep keyboard focus on MainComponent so key shortcuts still work
        grabKeyboardFocus();
    }
}

void MainComponent::closeKeyEditor()
{
    showKeyEditor_ = false;
    if (keyEditor_)
    {
        keyEditor_->setVisible(false);
        keyEditor_->setKey(nullptr);
    }
    if (keyboardPanel_)
        keyboardPanel_->refresh();
    resized();
}

void MainComponent::assignImageToKey(KeySlot& key)
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select image for key " + juce::String::charToString(key.keyChar),
        juce::File{},
        "*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tiff");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this, &key](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (!file.existsAsFile())
            return;

        key.mediaType = KeySlot::MediaType::Image;
        key.mediaFile = file;

        if (keyEditor_ && keyEditor_->getKey() == &key)
            keyEditor_->setKey(&key); // refresh editor

        if (keyboardPanel_)
            keyboardPanel_->refresh();
    });
}
