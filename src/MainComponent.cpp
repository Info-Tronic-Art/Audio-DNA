#include "MainComponent.h"
#include "ui/PreferencesDialog.h"
#include "analysis/BPMTracker.h"
#include "analysis/GenreDetector.h"
#include "effects/ISFShaderLoader.h"
#include "sources/ProjectMSource.h"
#include "core/CompositeCommand.h"
#include "core/DeckCommands.h"
#include "core/MediaReconnect.h"
#include "core/CompositionLoad.h"

static uint32_t s_nextClipId = 1000;

// L5 Quantize: translate the global Composition::quantizeMode control into a
// forced BeatSnapMode for one trigger. Off stays Off (no queueing). When the
// BPM tracker hasn't locked yet, an honest immediate trigger (Off) beats a
// trigger that may never drain — see BPMTracker::updatePhase, which pins
// phase_ at 0.0f while lockedBPM_ <= 0, freezing the beat-crossing edge that
// processPendingTrigger relies on to fire a queued trigger.
namespace
{
    Clip::BeatSnapMode quantizeModeToForcedSnap(Composition::QuantizeMode mode,
                                                 const FeatureSnapshot& snap)
    {
        if (mode == Composition::QuantizeMode::Off) return Clip::BeatSnapMode::Off;
        if (snap.trackerState != BPMTracker::STATE_LOCKED) return Clip::BeatSnapMode::Off;
        return (mode == Composition::QuantizeMode::NextBeat) ? Clip::BeatSnapMode::Beat
                                                               : Clip::BeatSnapMode::Bar;
    }

    // L7-JUKE: PreferencesDialog::show() seeds its MIDI tab from "the
    // current device id", but MidiOutputHandler only exposes the opened
    // device's display name (getDeviceName()), not the identifier it was
    // opened with, and MainComponent.h is outside this lane's fence so
    // there's nowhere to cache the identifier as a member. Best-effort
    // recovery: match the open device's name back against
    // getAvailableDevices(). If nothing is open, or no device with a
    // matching name is found, the dropdown just opens unselected — a
    // cosmetic gap only; openDevice() itself is unaffected either way.
    juce::String currentMidiOutputDeviceId(const MidiOutputHandler& handler)
    {
        if (!handler.isOpen())
            return {};
        auto name = handler.getDeviceName();
        for (const auto& dev : MidiOutputHandler::getAvailableDevices())
            if (dev.name == name)
                return dev.identifier;
        return {};
    }

    // S166-FAV: MilkDrop favorites/user-preset persistence. Shares the
    // Application Support/Audio-DNA folder with settings.json (see
    // save/loadMilkDropPresetDirSetting below) but is its own file, since
    // ProjectMPresetManager::saveUserData()/loadUserData() own a separate
    // JSON schema (favorites/userPresets arrays). A free function (not a
    // MainComponent method) so no MainComponent.h change is needed — same
    // reasoning as currentMidiOutputDeviceId() just above.
    juce::File projectMUserDataFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("Audio-DNA").getChildFile("milkdrop_userdata.json");
    }
}

MainComponent::MainComponent(bool testMode, int testPort)
    : testMode_(testMode), testPort_(testPort)
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
    randomLabel_.setText("Random FX on Beat", juce::dontSendNotification);
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
                PresetManager::LoadStats stats;
                if (PresetManager::loadPreset(s.loadedFile,
                                               previewPanel_.getEffectChain(),
                                               previewPanel_.getMappingEngine(),
                                               &stats))
                {
                    // Preset-retarget-fix W5: no modal on this performance
                    // surface — flag dropped mappings inline in the label.
                    fileLabel_.setText("Slot " + juce::String(capturedSlot + 1) + ": "
                                      + s.loadedFile.getFileNameWithoutExtension()
                                      + (stats.dropped > 0 ? " (check mappings)" : ""),
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

    // W5 (outputwindow-arc-design.md): start the mapping tick. Unconditional
    // — no attach/visibility gating — so it survives preview detach.
    mappingTickTimer_.startTimerHz(kMappingTickHz);

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
    else
    {
        // Sample-rate guard: the analysis pipeline (LUFS K-weighting + all frequency
        // math) hardcodes AnalysisThread::kSampleRate (48 kHz) with no resampling, so a
        // device at another rate silently yields wrong features. We do NOT attempt
        // SR-independence here — warn only. This ctor runs on the message thread, so the
        // async alert is non-blocking and safe (no modal on the audio thread).
        const double actualSr = audioEngine_.getCurrentSampleRate();
        const int expectedSr = AnalysisThread::kSampleRate;
        if (actualSr > 0.0 && static_cast<int>(actualSr) != expectedSr)
        {
            std::cerr << "[Audio] WARNING: device sample rate is " << static_cast<int>(actualSr)
                      << " Hz but the analysis pipeline assumes " << expectedSr
                      << " Hz — audio features (LUFS, frequency, key, BPM) will be inaccurate."
                      << std::endl;
            if (!testMode_)
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Unsupported Sample Rate",
                    "Your audio device is running at " + juce::String(static_cast<int>(actualSr))
                        + " Hz, but Audio-DNA's analysis is tuned for " + juce::String(expectedSr)
                        + " Hz. Audio-reactive features may be inaccurate — set your output "
                          "device to " + juce::String(expectedSr) + " Hz for correct results.");
        }
    }

    // Initialize effect library
    effectLibrary_.registerDefaults();

    // Create effects rack panel (needs renderer's MappingEngine and EffectChain)
    effectsRackPanel_ = std::make_unique<EffectsRackPanel>(
        previewPanel_.getMappingEngine(),
        previewPanel_.getEffectChain(),
        effectLibrary_);
    addAndMakeVisible(effectsRackPanel_.get());

    // === Tooltip window ===
    tooltipWindow_ = std::make_unique<juce::TooltipWindow>(this, 600);

    // === v2: Signal Bar + Top Bar ===
    composition_.initDefault();
    signalRegistry_.initDefaults();
    previewPanel_.getRenderer().setSignalRegistry(&signalRegistry_);
    previewPanel_.getRenderer().setAnalysisThread(&analysisThread_);

    // P22: Wire output integrations to renderer
    previewPanel_.getRenderer().setVideoRecorder(&videoRecorder_);
    previewPanel_.getRenderer().setSyphonOutput(&syphonOutput_);

    topBar_ = std::make_unique<TopBar>(analysisThread_.getFeatureBus(), composition_);
    addAndMakeVisible(topBar_.get());

    // Wire TopBar audio source selector to AudioEngine
    auto& topAudioSrc = topBar_->getAudioSourceSelector();
    topAudioSrc.addItem("Mic Input", 1);
    topAudioSrc.addItem("Audio File", 2);
    topAudioSrc.setSelectedId(1, juce::dontSendNotification);
    topAudioSrc.onChange = [this] {
        int sel = topBar_->getAudioSourceSelector().getSelectedId();
        if (sel == 1)
        {
            audioEngine_.setSourceMode(AudioEngine::SourceMode::MicInput);
            fileLabel_.setText("Mic: " + audioEngine_.getDeviceStatus(),
                              juce::dontSendNotification);
        }
        else if (sel == 2)
        {
            audioEngine_.setSourceMode(AudioEngine::SourceMode::File);
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

    // Wire TopBar gain slider
    topBar_->getInputGainSlider().onValueChange = [this] {
        audioEngine_.setInputGain(static_cast<float>(topBar_->getInputGainSlider().getValue()));
    };

    // Wire TopBar master level
    topBar_->getMasterLevelSlider().onValueChange = [this] {
        previewPanel_.getRenderer().setMasterLevel(
            static_cast<float>(topBar_->getMasterLevelSlider().getValue()));
    };

    // Wire TopBar display selector
    auto& topDisplay = topBar_->getDisplaySelector();
    topDisplay.setTextWhenNothingSelected("Output: Off");
    // Populate from existing display list
    {
        topDisplay.clear();
        topDisplay.addItem("Off", 1);
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
            topDisplay.addItem(label, i + 2);
        }
        topDisplay.setSelectedId(1, juce::dontSendNotification);
    }
    topDisplay.onChange = [this] {
        int selected = topBar_->getDisplaySelector().getSelectedId();
        if (selected == 1)
            closeOutput();
        else if (selected > 1)
            openOutputOnDisplay(selected - 2);
    };

    topBar_->onTapTempo = [this](float tappedBPM) {
        if (auto* tracker = analysisThread_.getBpmTracker())
            tracker->setManualBPM(tappedBPM);
    };

    // L7-JUKE: Ableton Link toggle. linkSync_'s consumer loop (below, in the
    // main timer callback) is already live and gated on isEnabled() — this
    // was the only missing piece.
    topBar_->onLinkToggled = [this](bool enabled) { linkSync_.setEnabled(enabled); };

    topBar_->onManualBpmChanged = [this](bool manual, float bpm) {
        if (auto* tracker = analysisThread_.getBpmTracker())
        {
            tracker->setManualMode(manual);
            if (manual && bpm > 0.0f)
                tracker->setManualBPM(bpm);
        }
    };

    topBar_->onResync = [this] {
        // Reset beat counters
        beatCounter_ = 0;
        lastBeatPhase_ = 0.0f;
        // Reset beat phase and phrase/bar counters in the BPM tracker
        if (auto* tracker = analysisThread_.getBpmTracker())
        {
            tracker->resetBeatPhase();
            tracker->resetPhrase();
        }
    };

    // Global transport (TopBar Play/Pause/Stop). There is no single global
    // transport flag in the model; per-layer transport (LayerStrip) drives each
    // layer's active clip. The honest global mapping is therefore: apply
    // play/pause/stop to every layer's active clip on the ACTIVE deck. Stop =
    // pause + rewind to the clip's in-point (distinct from Pause, which holds).
    topBar_->onPlay = [this] {
        if (auto* deck = composition_.getActiveDeck())
            for (int l = 0; l < deck->getNumLayers(); ++l)
                if (auto* layer = deck->getLayer(l))
                    if (auto* clip = layer->getActiveClip())
                    {
                        clip->reverse = false;
                        clip->playing = true;
                    }
    };
    topBar_->onPause = [this] {
        if (auto* deck = composition_.getActiveDeck())
            for (int l = 0; l < deck->getNumLayers(); ++l)
                if (auto* layer = deck->getLayer(l))
                    if (auto* clip = layer->getActiveClip())
                        clip->playing = false;
    };
    topBar_->onStop = [this] {
        if (auto* deck = composition_.getActiveDeck())
            for (int l = 0; l < deck->getNumLayers(); ++l)
                if (auto* layer = deck->getLayer(l))
                    if (auto* clip = layer->getActiveClip())
                    {
                        clip->playing = false;
                        clip->playheadPosition = clip->inPoint;
                    }
    };

    signalBar_ = std::make_unique<SignalBar>(signalRegistry_, analysisThread_.getFeatureBus());
    addAndMakeVisible(signalBar_.get());
    signalBar_->onSizeChanged = [this] { resized(); };
    signalBar_->onSignalSelected = [this](Signal& signal) {
        if (inspectorPanel_)
            inspectorPanel_->inspectSignal(&signal);
    };

    // === v2: Deck View ===
    deckView_ = std::make_unique<DeckView>();
    addAndMakeVisible(deckView_.get());
    deckView_->setComposition(&composition_);

    // Wire the active deck and composition into the renderer for compositor rendering
    previewPanel_.getRenderer().setActiveDeck(composition_.getActiveDeck());
    previewPanel_.getRenderer().setComposition(&composition_);

    // P20: Wire per-type autopilot config into the renderer
    previewPanel_.getRenderer().setPerTypeAutopilotConfig(&composition_.perTypeAutopilot);

    // Refresh deck view when autopilot advances a clip
    previewPanel_.getRenderer().setOnAutopilotAdvanced([this]() {
        if (deckView_) deckView_->refresh();
    });

    // P23: Genre change callback — auto-switch deck or load genre preset
    previewPanel_.getRenderer().setOnGenreChanged([this](uint8_t genre, float confidence) {
        if (!composition_.autoPresetOnGenre) return;

        // Auto-switch deck if genre has an assigned deck. Routed through
        // handleDeckSwitch (2026-07-30 fix, completes d4f5d86's scope claim) so
        // this gets the same refreshPreviewFromActiveClip reconcile every other
        // switch path gets — without it, switching to an empty deck by genre
        // left the previous deck's clip still rendering in preview. Safe from
        // this callback: it already runs on the message thread (Renderer.cpp's
        // genre-change detection marshals via juce::MessageManager::callAsync
        // before invoking onGenreChanged_), and handleDeckSwitch itself pushes
        // no undo command (see onDeckSwitched's comment above, which already
        // names genre auto-switch as one of the non-user callers this bare
        // primitive is meant for) — genre-auto stays non-undoable, per doctrine.
        int deckIdx = composition_.genreDeckAssignment[genre];
        if (deckIdx >= 0 && deckIdx < static_cast<int>(composition_.decks.size())
            && deckIdx != composition_.activeDeckIndex)
        {
            handleDeckSwitch(deckIdx);
        }

        std::cerr << "[P23] Genre changed to: " << GenreDetector::genreName(genre)
                  << " (confidence: " << confidence << ")" << std::endl;
    });

    // P23: Structural state change callback — auto-switch decks on transitions
    previewPanel_.getRenderer().setOnStructuralStateChanged([this](uint8_t state) {
        if (!composition_.structuralSceneEnabled) return;

        std::cerr << "[P23] Structural state: " << static_cast<int>(state)
                  << (state == 0 ? " (normal)" : state == 1 ? " (buildup)"
                  : state == 2 ? " (drop)" : " (breakdown)") << std::endl;
    });

    deckView_->onClipTriggered = [this](int layerIdx, int col) {
        handleClipTrigger(layerIdx, col);
    };
    deckView_->onColumnTriggered = [this](int col) {
        handleColumnTrigger(col);
    };
    deckView_->onClipSelected = [this](int layerIdx, int col, bool /*addToSel*/) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        if (auto* clip = deck->getClip(layerIdx, col))
        {
            // Show clip in inspector
            if (inspectorPanel_)
                inspectorPanel_->inspectClip(clip,
                    EffectScope::clip(composition_.activeDeckIndex, layerIdx, col));

            // Load clip content into preview panel
            if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
            {
                previewPanel_.getRenderer().clearActiveSource();
                previewPanel_.loadImage(clip->mediaFile);
                currentImageFile_ = clip->mediaFile;
                if (outputWindow_)
                    outputWindow_->loadImage(clip->mediaFile);
                fileLabel_.setText(clip->mediaFile.getFileName(), juce::dontSendNotification);
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                currentImageFile_ = juce::File();
                fileLabel_.setText(juce::String(clip->sourceType), juce::dontSendNotification);
            }
        }
    };
    deckView_->onLayerSelected = [this](int layerIdx) {
        if (!inspectorPanel_) return;
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        if (auto* layer = deck->getLayer(layerIdx))
            inspectorPanel_->inspectLayer(layer,
                EffectScope::layer(composition_.activeDeckIndex, layerIdx));
    };
    deckView_->onLayerClearClip = [this](int layerIdx) {
        // #13: X-button clear rerouted through a command. Runtime-only (clips row
        // untouched), so NO GL fence. Mutate-then-push: clearActiveClip live, then
        // wrap the runtime before/after (skip if it was a true no-op).
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;
        LayerRuntimeSnapshot before = captureLayerRuntime(*layer);
        layer->clearActiveClip();
        LayerRuntimeSnapshot after = captureLayerRuntime(*layer);
        if (!(before == after))
        {
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<ClearActiveClipCmd>(
                makeLayerResolver(), composition_.activeDeckIndex, layerIdx,
                before, after, "Clear Layer Clip"));
            pushCommands(std::move(children), "Clear Layer Clip");
        }
        // A1 fix (2026-07-30): clearActiveClip() only resets the MODEL
        // (activeClipColumn/playing) — it never touches the renderer, so a
        // shader/projectM source kept re-rendering via Renderer.cpp's
        // compositor-empty fallback (activeSourceType_, set at trigger time,
        // was never cleared). Re-sync the renderer/preview state (NOT undo-
        // tracked — ephemeral render state, not model), which purges only
        // when appropriate. See refreshPreviewFromActiveClip's ownership rule.
        refreshPreviewFromActiveClip(*deck);
        if (deckView_) deckView_->refresh();
    };
    deckView_->onLayerBypass = [this](int layerIdx, bool bypassed) {
        // #14: LayerStrip already toggled layer->bypassed live and repainted its
        // button; just wrap the change for undo (before = !bypassed).
        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<ToggleLayerFlagCmd>(
            makeLayerResolver(), composition_.activeDeckIndex, layerIdx,
            ToggleLayerFlagCmd::Flag::Bypassed, !bypassed, bypassed,
            bypassed ? "Bypass Layer" : "Unbypass Layer"));
        pushCommands(std::move(children), bypassed ? "Bypass Layer" : "Unbypass Layer");
    };
    deckView_->onLayerSolo = [this](int layerIdx, bool solo) {
        // #15: same shape as bypass — LayerStrip toggled layer->solo live.
        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<ToggleLayerFlagCmd>(
            makeLayerResolver(), composition_.activeDeckIndex, layerIdx,
            ToggleLayerFlagCmd::Flag::Solo, !solo, solo,
            solo ? "Solo Layer" : "Unsolo Layer"));
        pushCommands(std::move(children), solo ? "Solo Layer" : "Unsolo Layer");
    };
    deckView_->onFileDropped = [this](int layerIdx, int col, const juce::File& file) {
        handleFileDrop(layerIdx, col, file);
        if (deckView_) deckView_->clearSelection();
    };
    deckView_->onMultiFileDropped = [this](int layerIdx, int col, const std::vector<juce::File>& files) {
        handleMultiFileDrop(layerIdx, col, files);
        if (deckView_) deckView_->clearSelection();
    };
    deckView_->onMultiVideoDropped = [this](int layerIdx, int col, const std::vector<juce::File>& files) {
        // Place each video in sequential cells on the same layer, as ONE undo
        // unit: a composite of SetClipCmds plus a column-count restore, since a
        // multi-video drop can grow numColumns past the current grid and undo
        // must shrink it back (carry-forward column-growth gap).
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        const int numColsBefore = deck->numColumns;
        // GL fence (2026-07-28): the growth loop below resizes EVERY layer's
        // clips vector (layer.clips.resize), the exact crash-proven reallocation
        // — one fence for the whole gesture (growth + placement), not per-cell.
        std::vector<CellEdit> edits;
        undoService_.withDeckDetached([&]
        {
            // Ensure enough columns exist
            int needed = col + static_cast<int>(files.size());
            while (deck->numColumns < needed)
            {
                deck->numColumns++;
                for (auto& layer : deck->layers)
                    layer.clips.resize(static_cast<size_t>(deck->numColumns));
            }
            // Place each video, collecting cell edits WITHOUT per-file history entries.
            for (int i = 0; i < static_cast<int>(files.size()); ++i)
                if (auto edit = applyFileDrop(layerIdx, col + i, files[static_cast<size_t>(i)]))
                    edits.push_back(*edit);
        });
        const int numColsAfter = deck->numColumns;

        const int n = static_cast<int>(files.size());
        const juce::String desc = (n == 1) ? juce::String("Drop Video")
                                           : "Drop " + juce::String(n) + " Videos";
        std::vector<std::unique_ptr<Command>> children;
        if (numColsAfter != numColsBefore)   // FIRST child → undoes LAST (restores count)
            children.push_back(std::make_unique<SetColumnCountCmd>(
                makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                numColsBefore, numColsAfter, "Resize Columns"));
        for (auto& edit : edits)
            children.push_back(makeSetClipCmd(composition_.activeDeckIndex, edit, desc));
        pushCommands(std::move(children), desc);

        if (deckView_)
        {
            deckView_->clearSelection();
            deckView_->rebuildGrid();
        }
    };
    // Mixed Finder drop (2026-07-30 fix): a multi-file drop mixing videos and
    // images used to silently discard the images (ClipCell::filesDropped ran
    // mutually-exclusive early-return branches, video-first). Route each media
    // type through its existing single-type primitive (image(s) → applyFileDrop
    // or applyMultiFileDrop for one cell; videos → applyFileDrop per sequential
    // cell, mirroring onMultiVideoDropped above) and combine every edit into ONE
    // composite so the whole drop is one undo entry, per house pattern.
    deckView_->onMixedFilesDropped = [this](int layerIdx, int col,
                                             const std::vector<juce::File>& images,
                                             const std::vector<juce::File>& videos) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        const int numColsBefore = deck->numColumns;

        // Images occupy 1 cell (single image, or 3+ as an ImageSequence) or
        // 2 cells (exactly 2 images spread — Boris ruling 2026-08-04: two
        // dropped images must read as two distinct clips, not merge into one
        // animated sequence). Videos start immediately after, one cell each
        // — same placement rule as the internal "files:" drag path
        // (ClipCell::itemDropped's videoStartCol).
        const int imageCellCount = images.empty() ? 0 : (images.size() == 2 ? 2 : 1);
        const int videoStartCol = col + imageCellCount;

        std::vector<CellEdit> edits;
        // GL fence (2026-07-28, round 3 class): column growth + setClip below can
        // reallocate every layer's clips vector — one fence for the whole
        // gesture, mirroring onMultiVideoDropped's shape.
        undoService_.withDeckDetached([&]
        {
            int needed = videoStartCol + static_cast<int>(videos.size());
            while (deck->numColumns < needed)
            {
                deck->numColumns++;
                for (auto& layer : deck->layers)
                    layer.clips.resize(static_cast<size_t>(deck->numColumns));
            }

            if (images.size() == 1)
            {
                if (auto edit = applyFileDrop(layerIdx, col, images[0]))
                    edits.push_back(*edit);
            }
            else if (images.size() == 2)
            {
                // Spread: same primitive as the video-spread pattern above
                // (applyFileDrop per cell), not applyMultiFileDrop — two
                // images must land as two separate clips.
                for (int i = 0; i < 2; ++i)
                    if (auto edit = applyFileDrop(layerIdx, col + i, images[static_cast<size_t>(i)]))
                        edits.push_back(*edit);
            }
            else if (images.size() > 2)
            {
                if (auto edit = applyMultiFileDrop(layerIdx, col, images))
                    edits.push_back(*edit);
            }

            for (int i = 0; i < static_cast<int>(videos.size()); ++i)
                if (auto edit = applyFileDrop(layerIdx, videoStartCol + i, videos[static_cast<size_t>(i)]))
                    edits.push_back(*edit);
        });
        const int numColsAfter = deck->numColumns;

        if (edits.empty()) return;

        const juce::String desc = "Drop " + juce::String(videos.size())
            + (videos.size() == 1 ? " Video + " : " Videos + ")
            + juce::String(images.size()) + (images.size() == 1 ? " Image" : " Images");
        std::vector<std::unique_ptr<Command>> children;
        if (numColsAfter != numColsBefore)   // FIRST child → undoes LAST (restores count)
            children.push_back(std::make_unique<SetColumnCountCmd>(
                makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                numColsBefore, numColsAfter, "Resize Columns"));
        for (auto& edit : edits)
            children.push_back(makeSetClipCmd(composition_.activeDeckIndex, edit, desc));
        pushCommands(std::move(children), desc);

        if (deckView_)
        {
            deckView_->clearSelection();
            deckView_->rebuildGrid();
        }
    };
    deckView_->onEffectDropped = [this](int layerIdx, int col, const juce::String& effectName) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;

        // Support multi-FX drop: comma-separated names
        auto fxNames = juce::StringArray::fromTokens(effectName, ",", "");
        const int numColsBefore = deck->numColumns;   // FX-onto-empty-cells can grow columns

        // If target cell has content, add ALL FX to its chain
        auto* existingClip = layer->getClipAt(col);
        bool hasContent = existingClip && (existingClip->hasMedia() || !existingClip->effects.empty());

        // Capture before-state of every column this drop will touch (one column
        // when appending to a chain, N consecutive columns for a multi-FX drop
        // onto empty cells) so the whole gesture is one undo unit.
        std::vector<CellEdit> edits;
        if (hasContent)
            edits.push_back({ layerIdx, col, snapshotCell(layer, col), std::nullopt });
        else
            for (int fi = 0; fi < fxNames.size(); ++fi)
                edits.push_back({ layerIdx, col + fi, snapshotCell(layer, col + fi), std::nullopt });

        if (hasContent)
        {
            // Add all FX to the existing clip's chain. GL fence (2026-07-28,
            // family-fence fix round 2): push_back reallocates existingClip->
            // effects, which the GL thread iterates directly (CompositorEngine
            // .cpp:242) via the getActiveClip() pointer — reviewer-ruled REAL,
            // blocker-class exposure. One fence for the whole drop (a
            // multi-select drop appends several effects in this one loop).
            undoService_.withDeckDetached([&]
            {
                for (const auto& fxName : fxNames)
                {
                    Clip::EffectSlot slot;
                    slot.effectName = fxName.toStdString();
                    const auto* def = effectLibrary_.getEffectDef(fxName);
                    if (def)
                        for (const auto& p : def->params)
                            slot.paramValues.push_back(p.defaultValue);
                    existingClip->effects.push_back(slot);
                }
            });
            if (!existingClip->hasMedia())
            {
                std::string nameStr;
                for (size_t i = 0; i < existingClip->effects.size(); ++i)
                {
                    if (i > 0) nameStr += " + ";
                    nameStr += existingClip->effects[i].effectName;
                }
                existingClip->name = nameStr;
            }
        }
        else
        {
            // Empty cell(s): each FX gets its own cell in consecutive columns.
            // GL fence (2026-07-28): ensureColumns below can grow the layer's
            // clips vector — the crash-proven reallocation class — one fence
            // for the whole drop, not per FX. (The hasContent branch above
            // pushes into existingClip->effects, a DIFFERENT vector on the
            // active clip — round 1 left it unfenced pending reviewer ruling;
            // round 2 fenced it too, see the withDeckDetached wrap above.)
            static uint32_t fxClipId = 5000;
            undoService_.withDeckDetached([&]
            {
                for (int fi = 0; fi < fxNames.size(); ++fi)
                {
                    int targetCol = col + fi;
                    layer->ensureColumns(targetCol + 1);
                    if (deck->numColumns < targetCol + 1)
                        deck->numColumns = targetCol + 1;

                    Clip newClip;
                    newClip.id = fxClipId++;
                    newClip.name = fxNames[fi].toStdString();
                    newClip.mediaType = Clip::MediaType::None;

                    Clip::EffectSlot slot;
                    slot.effectName = fxNames[fi].toStdString();
                    const auto* def = effectLibrary_.getEffectDef(fxNames[fi]);
                    if (def)
                        for (const auto& p : def->params)
                            slot.paramValues.push_back(p.defaultValue);
                    newClip.effects.push_back(slot);

                    deck->setClip(layerIdx, targetCol, newClip);
                }
            });
        }

        // Capture after-state and record the drop as one undo unit. A multi-FX
        // drop onto empty cells grows numColumns, so prepend a column-count
        // restore (undoes LAST) to close the column-growth gap for this path too.
        for (auto& edit : edits)
            edit.after = snapshotCell(layer, edit.column);
        const int numColsAfter = deck->numColumns;
        const juce::String fxDesc = fxNames.size() > 1 ? juce::String("Add Effects")
                                                       : "Add Effect '" + effectName + "'";
        std::vector<std::unique_ptr<Command>> children;
        if (numColsAfter != numColsBefore)
            children.push_back(std::make_unique<SetColumnCountCmd>(
                makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                numColsBefore, numColsAfter, "Resize Columns"));
        for (auto& edit : edits)
            children.push_back(makeSetClipCmd(composition_.activeDeckIndex, edit, fxDesc));
        pushCommands(std::move(children), fxDesc);

        if (deckView_) deckView_->rebuildGrid();
        if (inspectorPanel_)
        {
            auto* clip = layer->getClipAt(col);
            if (clip)
            {
                auto& ci = inspectorPanel_->getClipInspector();
                if (ci.getClip() == clip) ci.refresh();
            }
        }
    };
    // FX-drop-target on the channel strip (2026-07-30): drops anywhere on a
    // LayerStrip add the effect(s) to THAT LAYER's FX stack (layer->layerEffects,
    // the same vector LayerInspector's effect stack view shows once the layer is
    // selected) — not a specific clip's chain. Mirrors 9c316e6's panel-forward
    // pattern but the mutation lives here since LayerStrip has no embedded
    // EffectStackView to do it locally.
    deckView_->onLayerEffectDropped = [this](int layerIdx, const juce::String& effectDesc) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;

        // Support multi-select drop: comma-separated names (mirrors
        // EffectStackView::itemDropped)
        auto fxNames = juce::StringArray::fromTokens(effectDesc, ",", "");
        if (fxNames.isEmpty()) return;

        std::vector<Clip::EffectSlot> before = layer->layerEffects;

        int addedCount = 0;
        juce::String lastAddedName;
        // GL fence: push_back reallocates layer->layerEffects, which the GL
        // thread copy-reads at CompositorEngine.cpp:752 — same family-fence
        // class as every other structural effects-vector mutation.
        undoService_.withDeckDetached([&]
        {
            for (const auto& fxName : fxNames)
            {
                auto trimmed = fxName.trim();
                const auto* def = effectLibrary_.getEffectDef(trimmed);
                if (!def) continue;

                Clip::EffectSlot slot;
                slot.effectName = trimmed.toStdString();
                for (const auto& p : def->params)
                    slot.paramValues.push_back(p.defaultValue);

                layer->layerEffects.push_back(slot);
                ++addedCount;
                lastAddedName = trimmed;
            }
        });

        if (addedCount == 0) return;

        const juce::String desc = (addedCount == 1)
            ? "Add Effect '" + lastAddedName + "'"
            : "Add " + juce::String(addedCount) + " Effects";

        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<EffectStackCmd>(
            makeCompositionResolver(), makeDeckFence(),
            EffectScope::layer(composition_.activeDeckIndex, layerIdx),
            std::move(before), layer->layerEffects,
            makeEffectStackRefresh(), desc.toStdString()));
        pushCommands(std::move(children), desc);
    };
    deckView_->onSourceDropped = [this](int layerIdx, int col, const juce::String& sourceId) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;

        // Support multi-source drop: comma-separated IDs
        auto sourceIds = juce::StringArray::fromTokens(sourceId, ",", "");
        auto& srcRegistry = previewPanel_.getRenderer().getSourceRegistry();

        // Record the whole gesture as ONE undo unit (a composite of SetClipCmds
        // across N cells, plus a column-count restore since a multi-source drop
        // can grow numColumns past the grid — close the column-growth gap).
        const int numColsBefore = deck->numColumns;
        // GL fence (2026-07-28): ensureColumns below can grow the layer's clips
        // vector — the crash-proven reallocation class — one fence for the
        // whole drop, not per source.
        std::vector<CellEdit> edits;
        undoService_.withDeckDetached([&]
        {
            for (int si = 0; si < sourceIds.size(); ++si)
            {
                int targetCol = col + si;
                std::optional<Clip> before = snapshotCell(layer, targetCol);
                layer->ensureColumns(targetCol + 1);
                if (deck->numColumns < targetCol + 1)
                    deck->numColumns = targetCol + 1;

                Clip clip;
                clip.id = s_nextClipId++;
                clip.name = sourceIds[si].toStdString();
                clip.mediaType = Clip::MediaType::Source;
                clip.sourceType = sourceIds[si].toStdString();
                clip.playing = true;  // Sources are always "playing" (parity with image/video paths)

                auto tempSrc = srcRegistry.createSource(sourceIds[si].toStdString());
                if (tempSrc)
                {
                    for (int i = 0; i < tempSrc->getNumParams(); ++i)
                    {
                        const auto& p = tempSrc->getParam(i);
                        Clip::SourceParam sp;
                        sp.name = p.name;
                        sp.uniformName = p.uniformName;
                        sp.value = p.defaultValue;
                        sp.defaultValue = p.defaultValue;
                        clip.sourceParams.push_back(sp);
                    }
                }

                deck->setClip(layerIdx, targetCol, clip);
                edits.push_back({ layerIdx, targetCol, before, std::optional<Clip>(clip) });
            }
        });
        const int numColsAfter = deck->numColumns;

        const int n = sourceIds.size();
        const juce::String desc = (n == 1) ? juce::String("Drop Source")
                                           : "Drop " + juce::String(n) + " Sources";
        std::vector<std::unique_ptr<Command>> children;
        if (numColsAfter != numColsBefore)   // FIRST child → undoes LAST (restores count)
            children.push_back(std::make_unique<SetColumnCountCmd>(
                makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                numColsBefore, numColsAfter, "Resize Columns"));
        for (auto& edit : edits)
            children.push_back(makeSetClipCmd(composition_.activeDeckIndex, edit, desc));
        pushCommands(std::move(children), desc);

        if (deckView_) deckView_->rebuildGrid();

        // Refresh inspector
        if (inspectorPanel_)
        {
            auto* newClip = deck->getClip(layerIdx, col);
            if (newClip)
                inspectorPanel_->inspectClip(newClip,
                    EffectScope::clip(composition_.activeDeckIndex, layerIdx, col));
        }
    };
    deckView_->onClipMoved = [this](int srcLayer, int srcCol, int dstLayer, int dstCol) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        if (srcLayer == dstLayer && srcCol == dstCol) return;

        auto* srcL = deck->getLayer(srcLayer);
        auto* dstL = deck->getLayer(dstLayer);
        if (!srcL || !dstL) return;

        // Snapshot both cells + the column count BEFORE any mutation (spec §2
        // #3): a move onto a far column grows numColumns via ensureColumns, and
        // undo must restore the prior count. snapshotCell yields nullopt for an
        // empty / out-of-range cell.
        const int numColsBefore = deck->numColumns;
        std::optional<Clip> srcBefore = snapshotCell(srcL, srcCol);
        std::optional<Clip> dstBefore = snapshotCell(dstL, dstCol);

        // GL fence (2026-07-28): ensureColumns below can grow a layer's clips
        // vector — the crash-proven reallocation class — one fence for the
        // whole move/swap gesture.
        undoService_.withDeckDetached([&]
        {
            // Ensure destination has enough columns
            dstL->ensureColumns(dstCol + 1);
            if (deck->numColumns < dstCol + 1)
                deck->numColumns = dstCol + 1;

            // Swap clips between source and destination
            auto srcClip = srcL->getClipAt(srcCol)
                ? std::optional<Clip>(*srcL->getClipAt(srcCol))
                : std::nullopt;
            auto dstClip = dstL->getClipAt(dstCol)
                ? std::optional<Clip>(*dstL->getClipAt(dstCol))
                : std::nullopt;

            // Place source clip at destination
            if (srcClip.has_value())
                dstL->clips[static_cast<size_t>(dstCol)] = srcClip;
            else
                dstL->clips[static_cast<size_t>(dstCol)] = std::nullopt;

            // Place destination clip at source (swap)
            srcL->ensureColumns(srcCol + 1);
            if (dstClip.has_value())
                srcL->clips[static_cast<size_t>(srcCol)] = dstClip;
            else
                srcL->clips[static_cast<size_t>(srcCol)] = std::nullopt;
        });

        // Record the whole gesture as one undo unit. "Swap" when the target was
        // occupied (two clips exchange places), "Move" when it was empty. The
        // command's execute() re-applies the after-state (idempotent with the
        // mutation just done), matching the step-2 mutate-then-push pattern.
        const int numColsAfter = deck->numColumns;
        std::optional<Clip> srcAfter = snapshotCell(srcL, srcCol);
        std::optional<Clip> dstAfter = snapshotCell(dstL, dstCol);
        const juce::String desc = dstBefore.has_value() ? "Swap Clips" : "Move Clip";
        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<SwapClipsCmd>(
            makeDeckResolver(), makeDeckFence(), makeClipMediaHook(), makeClipMediaDisposeHook(),
            composition_.activeDeckIndex,
            srcLayer, srcCol, dstLayer, dstCol,
            srcBefore, srcAfter, dstBefore, dstAfter,
            numColsBefore, numColsAfter, desc.toStdString()));
        pushCommands(std::move(children), desc);

        if (deckView_) deckView_->rebuildGrid();
    };
    // === MilkDrop single preset drop onto cell ===
    deckView_->onMilkDropDropped = [this](int layerIdx, int col, const std::string& presetPath) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;

        std::optional<Clip> before = snapshotCell(layer, col);

        // Create a projectM source clip with the preset path stored
        Clip clip;
        clip.id = s_nextClipId++;
        clip.name = juce::File(presetPath).getFileNameWithoutExtension().toStdString();
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "projectm_visualizer";
        clip.playing = true;  // Sources are always "playing" (parity with image/video paths)

        // Populate source params from registry
        auto& srcRegistry = previewPanel_.getRenderer().getSourceRegistry();
        auto tempSrc = srcRegistry.createSource("projectm_visualizer");
        if (tempSrc)
        {
            for (int i = 0; i < tempSrc->getNumParams(); ++i)
            {
                const auto& p = tempSrc->getParam(i);
                Clip::SourceParam sp;
                sp.name = p.name;
                sp.uniformName = p.uniformName;
                sp.value = p.defaultValue;
                sp.defaultValue = p.defaultValue;
                clip.sourceParams.push_back(sp);
            }
        }

        // Store the preset path as a single-entry playlist
        Clip::PresetEntry entry;
        entry.presetPath = presetPath;
        entry.presetName = clip.name;
        clip.presetPlaylist.push_back(entry);

        // GL fence (2026-07-28): ensureColumns can grow the layer's clips
        // vector — the crash-proven reallocation class.
        undoService_.withDeckDetached([&]
        {
            layer->ensureColumns(col + 1);
            if (deck->numColumns < col + 1) deck->numColumns = col + 1;
            deck->setClip(layerIdx, col, clip);
        });
        pushClipEdits(composition_.activeDeckIndex,
                      { { layerIdx, col, before, std::optional<Clip>(clip) } },
                      "Drop '" + juce::String(clip.name) + "'");
        if (deckView_) deckView_->rebuildGrid();
        if (inspectorPanel_)
        {
            auto* newClip = deck->getClip(layerIdx, col);
            if (newClip) inspectorPanel_->inspectClip(newClip,
                EffectScope::clip(composition_.activeDeckIndex, layerIdx, col));
        }
    };

    // === MilkDrop multi-preset playlist drop onto cell ===
    deckView_->onMilkDropPlaylistDropped = [this](int layerIdx, int col, const std::vector<std::string>& presetPaths) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) return;

        std::optional<Clip> before = snapshotCell(layer, col);

        Clip clip;
        clip.id = s_nextClipId++;
        clip.name = "MilkDrop Playlist (" + std::to_string(presetPaths.size()) + ")";
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = "projectm_visualizer";
        clip.playing = true;  // Sources are always "playing" (parity with image/video paths)

        // Populate source params
        auto& srcRegistry = previewPanel_.getRenderer().getSourceRegistry();
        auto tempSrc = srcRegistry.createSource("projectm_visualizer");
        if (tempSrc)
        {
            for (int i = 0; i < tempSrc->getNumParams(); ++i)
            {
                const auto& p = tempSrc->getParam(i);
                Clip::SourceParam sp;
                sp.name = p.name;
                sp.uniformName = p.uniformName;
                sp.value = p.defaultValue;
                sp.defaultValue = p.defaultValue;
                clip.sourceParams.push_back(sp);
            }
        }

        // Store all presets as playlist
        for (const auto& path : presetPaths)
        {
            Clip::PresetEntry entry;
            entry.presetPath = path;
            entry.presetName = juce::File(path).getFileNameWithoutExtension().toStdString();
            clip.presetPlaylist.push_back(entry);
        }
        clip.playlistEnabled = true;

        // Honor the browser's current Playlist-mode controls (cycle mode,
        // timing, blend seconds) instead of hardcoding — those controls were
        // previously decorative (SIDE FINDING, scout-playlist-drop.md).
        auto& browser = browserPanel_->getMilkDropBrowser();
        switch (browser.getPlaylistCycleModeId())
        {
            case 2:  clip.playlistCycleMode = Clip::PlaylistCycleMode::RandomOther; break; // "Random"
            case 3:  clip.playlistCycleMode = Clip::PlaylistCycleMode::Sequential;  break; // "Sequential"
            default: clip.playlistCycleMode = Clip::PlaylistCycleMode::RandomBag;  break; // "Bag" (id 1)
        }
        clip.playlistTriggerBeats = browser.getPlaylistTriggerBeats();
        clip.playlistBlendSeconds = browser.getPlaylistBlendSeconds();

        // GL fence (2026-07-28): ensureColumns can grow the layer's clips
        // vector — the crash-proven reallocation class.
        undoService_.withDeckDetached([&]
        {
            layer->ensureColumns(col + 1);
            if (deck->numColumns < col + 1) deck->numColumns = col + 1;
            deck->setClip(layerIdx, col, clip);
        });
        pushClipEdits(composition_.activeDeckIndex,
                      { { layerIdx, col, before, std::optional<Clip>(clip) } },
                      "Drop '" + juce::String(clip.name) + "'");
        if (deckView_) deckView_->rebuildGrid();
        if (inspectorPanel_)
        {
            auto* newClip = deck->getClip(layerIdx, col);
            if (newClip) inspectorPanel_->inspectClip(newClip,
                EffectScope::clip(composition_.activeDeckIndex, layerIdx, col));
        }
    };

    deckView_->onDeckSwitched = [this](int deckIdx) {
        // #24: USER-initiated deck switch (tab click) — the ONLY switch path that
        // wraps an undo command. handleDeckSwitch is ALSO called by non-user paths
        // (REST, OSC, MIDI/controller bindings, genre auto-switch) which must NOT
        // push commands, so the wrap lives here at the user entry point, not inside
        // handleDeckSwitch. Mutate-then-push: switch live, then record before/after
        // (only if the active deck actually changed — a no-op switch pushes nothing).
        const int before = composition_.activeDeckIndex;

        // L5 Quantize follow-on: cancel (via the shared DeckCommands.h helper)
        // whichever layers on the deck we're about to LEAVE have a pending
        // quantized trigger, BEFORE calling handleDeckSwitch — capturing the
        // return value is only needed here, the one path that pushes an undo
        // command; handleDeckSwitch's own call to the same helper (below) then
        // finds nothing left to cancel and is a harmless no-op for this path,
        // exactly mirroring how handleClipTrigger captures rtBefore/rtAfter
        // around the live mutation rather than having triggerClip report it.
        std::vector<PendingTriggerSnapshot> cancelledOnLeave;
        if (auto* leavingDeck = composition_.getActiveDeck())
            cancelledOnLeave = cancelPendingTriggers(*leavingDeck);

        handleDeckSwitch(deckIdx);
        const int after = composition_.activeDeckIndex;
        if (before != after)
        {
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<SwitchDeckCmd>(
                makeCompositionResolver(), makeDeckActivateHook(),
                before, after, "Switch Deck", std::move(cancelledOnLeave)));
            pushCommands(std::move(children), "Switch Deck");
        }
    };

    // === v2: Inspector Panel ===
    inspectorPanel_ = std::make_unique<InspectorPanel>();
    addAndMakeVisible(inspectorPanel_.get());
    inspectorPanel_->setComposition(&composition_);
    inspectorPanel_->setEffectLibrary(&effectLibrary_);
    inspectorPanel_->setSignalRegistry(&signalRegistry_);
    inspectorPanel_->setMacroBank(&globalMacroBank_);

    // Undo v1 step 7: effect-stack edits (#27 add / #28 remove / #29 bypass) —
    // the EffectStackView has already performed the live mutation and rebuilt
    // itself; here we wrap the whole-vector before/after + scope as one command.
    // The scope was captured AT THE GESTURE (the view's scope_, set by whichever
    // host handed it its vector), so the command re-resolves the chain by
    // coordinate on undo/redo, never via the stored effects_ pointer.
    inspectorPanel_->setEffectPerformEdit(
        [this](const EffectScope& scope,
               std::vector<Clip::EffectSlot> before,
               std::vector<Clip::EffectSlot> after,
               const juce::String& description)
        {
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<EffectStackCmd>(
                makeCompositionResolver(), makeDeckFence(), scope,
                std::move(before), std::move(after),
                makeEffectStackRefresh(), description.toStdString()));
            pushCommands(std::move(children), description);
        });

    // Family-fence fix round 2 (2026-07-28): fence the EffectStackView's own
    // structural edits (push_back on FX drop, erase on delete) — same
    // withDeckDetached hook as every other structural mutation in this lane.
    inspectorPanel_->setEffectFenceHook(makeDeckFence());

    inspectorPanel_->getLayerInspector().onLayerNameChanged = [this]() {
        if (deckView_) deckView_->refresh();
    };
    inspectorPanel_->getClipInspector().onSourceParamsChanged = [this](Clip* clip) {
        if (clip && clip->mediaType == Clip::MediaType::Source)
            previewPanel_.getRenderer().updateActiveSourceParams(clip->sourceParams);
    };

    // Cuepoint jump: seek video/image sequence to the cuepoint position
    inspectorPanel_->getClipInspector().onCuepointJump = [this](Clip* clip, double pos) {
        if (!clip || !clip->isPlayable()) return;
        auto& renderer = previewPanel_.getRenderer();
        if (clip->mediaType == Clip::MediaType::Video)
        {
            auto* player = renderer.getVideoPlayer(clip->id);
            if (player) player->seekTo(pos);
        }
        else if (clip->mediaType == Clip::MediaType::ImageSequence)
        {
            auto* seq = renderer.getImageSequence(clip->id);
            if (seq) seq->seekTo(pos);
        }
    };

    // === v2: Browser Panel ===
    browserPanel_ = std::make_unique<BrowserPanel>();
    addAndMakeVisible(browserPanel_.get());
    browserPanel_->setEffectLibrary(&effectLibrary_);
    browserPanel_->setComposition(&composition_);
    // L3 (2026-09): wire the Comp/Decks browser's composition/deck load/save
    // callbacks — the "Save Composition" button and clicking a saved
    // composition or deck row were silent no-ops until now (onCompositionSave
    // only reads the model — menu-Save semantics: overwrite if a path is
    // known, else Save As into the compositions dir — so it does not need the
    // fence/undo-clear treatment loadComposition already gives Open).
    // onDeckLoad (STEP 3) APPENDS the deck rather than replacing the active
    // one — see appendDeckFromFile's header comment for why.
    browserPanel_->getCompDecksBrowser().onCompositionLoad = [this](const juce::File& f) {
        loadComposition(f);
    };
    browserPanel_->getCompDecksBrowser().onCompositionSave = [this] { saveComposition(); };
    browserPanel_->getCompDecksBrowser().onDeckLoad = [this](const juce::File& f) {
        appendDeckFromFile(f);
    };
    browserPanel_->getFXBrowser().onEffectActivated = [this](const juce::String& effectName) {
        DBG("FX Browser: activated effect " + effectName);
    };
    browserPanel_->getFilesBrowser().onFileActivated = [this](const juce::File& file) {
        previewPanel_.loadImage(file);
        currentImageFile_ = file;
        if (outputWindow_)
            outputWindow_->loadImage(file);
        fileLabel_.setText(file.getFileName(), juce::dontSendNotification);
    };
    browserPanel_->getSourcesBrowser().onSourceActivated = [this](const juce::String& sourceId) {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;

        // Find the first selected clip cell, or use layer 0 col 0
        int targetLayer = 0;
        int targetCol = 0;
        if (deckView_)
        {
            auto& sel = deckView_->getSelectedCells();
            if (!sel.empty()) { targetLayer = sel[0].layer; targetCol = sel[0].column; }
        }

        // Create a source clip with parameters from registry
        Clip clip;
        clip.id = s_nextClipId++;
        clip.name = sourceId.toStdString();
        clip.mediaType = Clip::MediaType::Source;
        clip.sourceType = sourceId.toStdString();
        clip.playing = true;  // Sources are always "playing" (parity with image/video paths)

        // Populate source parameters from the registry
        auto& srcRegistry = previewPanel_.getRenderer().getSourceRegistry();
        auto tempSrc = srcRegistry.createSource(sourceId.toStdString());
        if (tempSrc)
        {
            for (int i = 0; i < tempSrc->getNumParams(); ++i)
            {
                const auto& p = tempSrc->getParam(i);
                Clip::SourceParam sp;
                sp.name = p.name;
                sp.uniformName = p.uniformName;
                sp.value = p.defaultValue;
                sp.defaultValue = p.defaultValue;
                clip.sourceParams.push_back(sp);
            }
        }

        // GL fence (2026-07-28): setClip's internal ensureColumns can grow the
        // layer's clips vector — the crash-proven reallocation class.
        undoService_.withDeckDetached([&] { deck->setClip(targetLayer, targetCol, clip); });
        // Don't auto-trigger — user clicks cell to activate

        // Load source into preview renderer
        previewPanel_.getRenderer().setActiveSource(sourceId.toStdString(), clip.sourceParams);
        previewPanel_.getRenderer().clearImage();
        currentImageFile_ = juce::File();
        fileLabel_.setText(juce::String(sourceId), juce::dontSendNotification);

        if (deckView_) deckView_->rebuildGrid();

        // Refresh inspector to show the new source clip
        if (inspectorPanel_)
        {
            auto* newClip = deck->getClip(targetLayer, targetCol);
            if (newClip)
                inspectorPanel_->inspectClip(newClip,
                    EffectScope::clip(composition_.activeDeckIndex, targetLayer, targetCol));
        }
    };

    // === v2: MilkDrop Preset Browser Wiring ===
    // 2026-09-04 fix (regression since 22fcedc, see
    // .harmony/milkdrop-autoload-rootcause.md): this used to be gated on
    // `if (pmSource)`, where pmSource came from getOrCreateSource() — which
    // returns nullptr unless the GL context is attached, which it never is
    // at construction time. That skipped the whole block, including
    // setPresetManager, on every single launch. Preset scanning is pure
    // file/JSON work with no GL dependency, so it now runs unconditionally
    // against MainComponent's own hoisted presetManager_ instead of reaching
    // into a GL-thread-only source that doesn't exist yet.
    {
        // Scan bundled presets from the resources directory
        auto exePath = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
        // macOS: .app/Contents/MacOS/Audio-DNA → .app/Contents/Resources/
        auto bundledDir = exePath.getParentDirectory().getParentDirectory()
                                 .getChildFile("Resources").getChildFile("projectm_presets");
        if (!bundledDir.isDirectory())
        {
            // Development fallback: look relative to working directory
            bundledDir = juce::File::getCurrentWorkingDirectory().getChildFile("resources/projectm_presets");
        }

        // Also scan the Cream of the Crop collection if available
        auto creamDir = juce::File("/tmp/milkdrop-presets");

        milkDropBaseDirs_.clear();
        if (bundledDir.isDirectory())
            milkDropBaseDirs_.push_back(bundledDir.getFullPathName().toStdString());
        if (creamDir.isDirectory())
            milkDropBaseDirs_.push_back(creamDir.getFullPathName().toStdString());

        // Preferences > Video: apply a previously saved MilkDrop preset
        // folder BEFORE this initial scan (L7 fix — see
        // .harmony/session-c-new-findings.md FINDING 0b), so presets from
        // it appear on this very first launch/paint rather than only after
        // the user re-opens Preferences and re-picks.
        milkDropPresetDir_ = loadMilkDropPresetDirSetting();
        auto initialDirs = milkDropBaseDirs_;
        if (milkDropPresetDir_.isNotEmpty() && juce::File(milkDropPresetDir_).isDirectory())
            initialDirs.push_back(milkDropPresetDir_.toStdString());

        // setPresetDirectories()+rescan() (rather than direct scanDirectory()
        // calls, as before this fix) so presetDirs_ actually holds the full
        // set — required so a LATER rescan (triggered by a Preferences
        // change, see setMilkDropPresetDir()) doesn't wipe the bundled/cream
        // presets. presets_ is empty at this point (ctor), so this rescan()
        // is behavior-identical to the old direct scans for these two dirs.
        presetManager_.setPresetDirectories(initialDirs);
        presetManager_.rescan();

        if (bundledDir.isDirectory())
        {
            // Load mood/energy metadata manifest
            auto manifestFile = bundledDir.getChildFile("presets.json");
            if (manifestFile.existsAsFile())
                presetManager_.loadManifest(manifestFile.getFullPathName().toStdString());
        }

        // S166-FAV: restore favorites/user-preset flags saved from a prior
        // run. Must run AFTER rescan()/loadManifest() above — it matches
        // saved entries against presets_ by name/path, which is empty until
        // rescan() populates it. Safe to call unconfined here for the same
        // reason rescan()/loadManifest() are: the GL context is never
        // attached at construction time (see the "v2: MilkDrop Preset
        // Browser Wiring" comment above this block), so this is the message
        // thread and no ProjectMSource/PresetSelector can be mid-processFrame
        // to race it. A missing or malformed file is a silent no-op (see
        // ProjectMPresetManager::loadUserData()) — normal on first run.
        presetManager_.loadUserData(projectMUserDataFile().getFullPathName().toStdString());

        // Wire the browser to the (MainComponent-owned) preset manager. This
        // pointer outlives Renderer and every ProjectMSource, so it never
        // dangles even across GL context recreation — see
        // Renderer::openGLContextClosing().
        browserPanel_->getMilkDropBrowser().setPresetManager(&presetManager_);

        // Hand the manager to the renderer so it can inject it into each
        // ProjectMSource as the GL thread creates one — see
        // Renderer::getOrCreateSourceOnGLThread.
        previewPanel_.getRenderer().setProjectMPresetManager(&presetManager_);

        // Wire favorite-toggling through Renderer::toggleFavoritePreset()
        // (L7-JUKE), which confines the mutation to the GL thread — see its
        // declaration comment in Renderer.h for why. Unlike setPresetSelector
        // below, this does NOT need to wait for a real ProjectMSource to
        // exist (toggleFavoritePreset() only needs projectMPresetManager_,
        // set immediately above, and glContext_, which Renderer always
        // owns) — wire it directly here so it is live before this ctor
        // returns, i.e. before any UI click is possible.
        browserPanel_->getMilkDropBrowser().onToggleFavoriteRequested = [this](int idx) {
            previewPanel_.getRenderer().toggleFavoritePreset(idx);

            // S166-FAV: persist immediately so the new favorite survives a
            // quit, rather than waiting for a shutdown hook (matches this
            // ctor's own setMilkDropPresetDir()/saveMilkDropPresetDirSetting()
            // precedent: write-through on change, not on exit). Runs here on
            // the message thread AFTER toggleFavoritePreset() above returns
            // — that call blocks until its GL-thread round-trip (when the GL
            // context is attached) completes, so presets_ is already stable
            // by this line; saveUserData() only reads it. This does NOT put
            // I/O on the GL/render thread, and no new thread touches
            // presets_ — the message thread was already the sole caller of
            // this whole callback.
            auto userDataFile = projectMUserDataFile();
            userDataFile.getParentDirectory().createDirectory();
            presetManager_.saveUserData(userDataFile.getFullPathName().toStdString());
        };

        // The PresetSelector lives INSIDE ProjectMSource (GL-thread member,
        // runs inside its render()) and must NOT be hoisted like the manager
        // above — wire the browser's selector lazily, only once a real
        // source exists. This callback is pre-marshaled onto the message
        // thread by Renderer (matching setOnAutopilotAdvanced/
        // setOnGenreChanged elsewhere in this ctor), so it's safe to touch
        // browser UI state here. The browser is already null-guarded for a
        // missing selector, so preset listing/scanning above works at
        // startup with no selector present; jukebox playback lights up once
        // this fires.
        previewPanel_.getRenderer().setOnProjectMSourceCreated([this](ProjectMSource* pmSrc) {
            browserPanel_->getMilkDropBrowser().setPresetSelector(&pmSrc->getPresetSelector());
        });

        // Wire preset selection callback: load preset into projectM source
        browserPanel_->getMilkDropBrowser().onPresetSelected = [this](const std::string& path) {
            auto* src = dynamic_cast<ProjectMSource*>(
                previewPanel_.getRenderer().getOrCreateSource("projectm_visualizer"));
            if (src)
            {
                src->loadPreset(path, true); // smooth transition

                // Also set the source as active in the preview renderer
                previewPanel_.getRenderer().setActiveSource("projectm_visualizer");
                previewPanel_.getRenderer().clearImage();
                currentImageFile_ = juce::File();

                // Extract display name from path
                juce::File presetFile(path);
                fileLabel_.setText("MilkDrop: " + presetFile.getFileNameWithoutExtension(),
                                   juce::dontSendNotification);
            }
        };

        std::cerr << "[MilkDrop] Loaded " << presetManager_.getPresetCount() << " presets" << std::endl;
    }

    // === v2: Timing Window ===
    timingWindow_ = std::make_unique<TimingWindow>();
    addAndMakeVisible(timingWindow_.get());

    // === v2: Menu Bar ===
    menuBarModel_ = std::make_unique<AudioDNAMenuBar>();
    menuBarModel_->onMenuCommand = [this](int cmdId) { handleMenuCommand(cmdId); };
    menuBarModel_->isSyphonOutputEnabled = [this]() { return syphonOutput_.isEnabled(); };
    menuBarModel_->hasClipSelection = [this]() {
        return deckView_ && !deckView_->getSelectedCells().empty();
    };

    // Undo/Redo menu state: dynamic "Undo <description>" text + enabled flags,
    // and rebuild the native menu whenever history changes.
    menuBarModel_->getUndoState = [this]() {
        return std::make_pair(juce::String(undoManager_.undoDescription()),
                              undoManager_.canUndo());
    };
    menuBarModel_->getRedoState = [this]() {
        return std::make_pair(juce::String(undoManager_.redoDescription()),
                              undoManager_.canRedo());
    };
    undoManager_.onHistoryChanged = [this]() {
        if (menuBarModel_) menuBarModel_->menuItemsChanged();
    };

    // Undo plumbing: give the UndoService non-owning handles to the model,
    // renderer and deck grid so commands can re-resolve targets by
    // coordinate and refresh the UI after undo/redo.
    undoService_.setCollaborators(&composition_, &previewPanel_.getRenderer(),
                                  deckView_.get());

    // === v2: Binding System & MIDI (P9) ===
    bindingManager_.setActionCallback([this](const Binding& b, float val)
    {
        handleBindingAction(b, val);
    });

    bindingOverlay_ = std::make_unique<BindingOverlay>(bindingManager_, composition_);
    addChildComponent(bindingOverlay_.get()); // hidden initially
    bindingOverlay_->onBindingModeExit = [this]() { repaint(); };

    midiLearnOverlay_ = std::make_unique<MidiLearnOverlay>(bindingManager_, composition_);
    addChildComponent(midiLearnOverlay_.get()); // hidden initially
    midiLearnOverlay_->onLearnModeExit = [this]() { repaint(); };

    midiHandler_ = std::make_unique<MidiHandler>(bindingManager_);
    midiHandler_->start(audioEngine_.getDeviceManager());

    // Start analysis (skip in test mode — features are injected via HTTP).
    // R4 (featurebus-thread-safety-design.md): the single FeatureBus Writer
    // is claimed exactly once, here — production → AnalysisThread, test →
    // TestServer. createWriter() hands out one move-only handle; a second
    // claim returns an invalid one, so a second writer cannot appear.
    if (!testMode_)
    {
        analysisThread_.setFeatureBusWriter(analysisThread_.getFeatureBus().createWriter());
        analysisThread_.startThread(juce::Thread::Priority::high);
    }

#if AUDIODNA_TEST_SERVER
    if (testMode_)
    {
        testServer_ = std::make_unique<TestServer>(
            previewPanel_.getRenderer(),
            analysisThread_.getFeatureBus().createWriter(),
            composition_,
            previewPanel_.getRenderer().getEffectChain(),
            previewPanel_.getRenderer().getSourceRegistry(),
            signalRegistry_,
            previewPanel_.getRenderer().getRoutingEngine(),
            testPort_);
        testServer_->start();
        std::cerr << "[Eyes] Test server started on port " << testPort_ << std::endl;
    }
#endif

    // P22.8: Start production API server (port 7070)
    // R6 (featurebus-thread-safety-design.md): inject_features is only
    // registered when testMode_ is true — production never exposes the
    // second-writer endpoint (P2 hardening).
    apiServer_ = std::make_unique<ApiServer>(
        previewPanel_.getRenderer(),
        analysisThread_.getFeatureBus(),
        composition_,
        previewPanel_.getRenderer().getEffectChain(),
        previewPanel_.getRenderer().getSourceRegistry(),
        signalRegistry_,
        previewPanel_.getRenderer().getRoutingEngine(),
        bindingManager_,
        7070,
        testMode_);
    apiServer_->onTriggerClip = [this](int layer, int column) { handleClipTrigger(layer, column); };
    apiServer_->onTriggerColumn = [this](int column) { handleColumnTrigger(column); };
    apiServer_->onSwitchDeck = [this](int deckIdx) { handleDeckSwitch(deckIdx); };
    apiServer_->onLoadComposition = [this](juce::File f) { loadComposition(f); };
    apiServer_->onSnapshot = [this]() {
        auto& renderer = previewPanel_.getRenderer();
        std::thread([&renderer]() { renderer.takeSnapshot(); }).detach();
    };
    apiServer_->onSetBpm = [this](float bpm) {
        // Same path as the TopBar manual-BPM toggle+edit (manual override).
        if (auto* tracker = analysisThread_.getBpmTracker())
        {
            tracker->setManualMode(true);
            tracker->setManualBPM(bpm);
        }
    };
#if AUDIODNA_TEST_SERVER
    // R4: test-mode inject_features on the production port relays through
    // the TestServer-held Writer (the only writer in test mode).
    if (testMode_ && testServer_)
        apiServer_->onInjectFeatures = [this](const FeatureSnapshot& snap) {
            testServer_->injectSnapshot(snap);
        };
#endif
    apiServer_->start();

    // P22.9: Set up OSC handler callbacks, then start listening (below).
    // OscHandler uses MessageLoopCallback, so these fire on the message thread;
    // the callAsync wrappers below match the existing trigger/deck callbacks.
    oscHandler_.onTriggerClip = [this](int layer, int column) {
        juce::MessageManager::callAsync([this, layer, column]() { handleClipTrigger(layer, column); });
    };
    oscHandler_.onSwitchDeck = [this](int deckIdx) {
        juce::MessageManager::callAsync([this, deckIdx]() { handleDeckSwitch(deckIdx); });
    };
    oscHandler_.onSetMaster = [this](float level) {
        composition_.masterOpacity = level;
    };
    oscHandler_.onSetLayerOpacity = [this](int layerIdx, float opacity) {
        if (auto* deck = composition_.getActiveDeck())
            if (auto* layer = deck->getLayer(layerIdx))
                layer->opacity = opacity;
    };
    oscHandler_.onSetLayerBypass = [this](int layerIdx, bool bypass) {
        if (auto* deck = composition_.getActiveDeck())
            if (auto* layer = deck->getLayer(layerIdx))
                layer->bypassed = bypass;
    };
    oscHandler_.onSetLayerSolo = [this](int layerIdx, bool solo) {
        if (auto* deck = composition_.getActiveDeck())
            if (auto* layer = deck->getLayer(layerIdx))
                layer->solo = solo;
    };
    oscHandler_.onSetLayerMute = [this](int layerIdx, bool mute) {
        if (auto* deck = composition_.getActiveDeck())
            if (auto* layer = deck->getLayer(layerIdx))
                layer->muted = mute;
    };
    oscHandler_.onSetBpm = [this](float bpm) {
        // Same manual-override path as apiServer_->onSetBpm / the TopBar manual-BPM toggle.
        if (auto* tracker = analysisThread_.getBpmTracker())
        {
            tracker->setManualMode(true);
            tracker->setManualBPM(bpm);
        }
    };
    oscHandler_.onSetMacro = [this](int macroIdx, float value) {
        // Same path as the AdjustMacro MIDI binding (global dashboard-link bank).
        if (macroIdx >= 0 && macroIdx < MacroBank::kNumMacros)
            globalMacroBank_.getMacro(macroIdx).manualValue = value;
    };
    oscHandler_.onSetEffectParam = [this](const juce::String& effectName,
                                          const juce::String& paramName, float value) {
        // Same path as ApiServer::handleSetParam global-effect-chain branch.
        auto& chain = previewPanel_.getRenderer().getEffectChain();
        for (int i = 0; i < chain.getNumEffects(); ++i)
        {
            auto* fx = chain.getEffect(i);
            if (fx && fx->getName() == effectName)
            {
                for (int pi = 0; pi < fx->getNumParams(); ++pi)
                {
                    if (fx->getParam(pi).name == paramName.toStdString())
                    {
                        fx->getParam(pi).value = value;
                        return;
                    }
                }
            }
        }
    };
    oscHandler_.onSnapshot = [this]() {
        auto& renderer = previewPanel_.getRenderer();
        std::thread([&renderer]() { renderer.takeSnapshot(); }).detach();
    };

    // P22.9: Start the OSC listener at startup (like ApiServer above). Previously
    // "on demand from preferences", but nothing ever called startListening(), so
    // OSC input was inert. Port 8000 is the de-facto OSC receive default (matches
    // a TouchOSC controller's default outgoing port); no preferences UI configures
    // it yet. startListening() logs port + success/failure internally.
    constexpr int kOscListenPort = 8000;
    if (oscHandler_.startListening(kOscListenPort))
        std::cerr << "[OSC] Input listening on port " << kOscListenPort << std::endl;
    else
        std::cerr << "[OSC] WARNING: OSC input disabled (could not bind port "
                  << kOscListenPort << ")" << std::endl;

    // P22.6: Video recorder callback
    videoRecorder_.onRecordingFinished = [this](bool success, const juce::File& file) {
        if (success)
            std::cerr << "[VideoRecorder] Saved: " << file.getFullPathName() << std::endl;
        else
            std::cerr << "[VideoRecorder] Recording failed" << std::endl;
    };

    setWantsKeyboardFocus(true);
    // Register as key listener on top-level component to catch keys globally
    addKeyListener(this);
    setSize(1280, 800);
}

void MainComponent::setTooltipsEnabled(bool enabled)
{
    tooltipsEnabled_ = enabled;
    // The TooltipWindow shows tips for any component under the mouse while it
    // exists; destroying it is the clean way to disable tooltips app-wide.
    if (enabled)
    {
        if (!tooltipWindow_)
            tooltipWindow_ = std::make_unique<juce::TooltipWindow>(this, 600);
    }
    else
    {
        tooltipWindow_.reset();
    }
}

juce::String MainComponent::loadMilkDropPresetDirSetting() const
{
    auto file = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("Audio-DNA").getChildFile("settings.json");
    if (!file.existsAsFile())
        return {};

    auto parsed = juce::JSON::parse(file.loadFileAsString());
    if (auto* obj = parsed.getDynamicObject())
        return obj->getProperty("milkDropPresetDir").toString();
    return {};
}

void MainComponent::saveMilkDropPresetDirSetting(const juce::String& dir) const
{
    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("Audio-DNA");
    settingsDir.createDirectory();

    auto* obj = new juce::DynamicObject();
    obj->setProperty("milkDropPresetDir", dir);
    settingsDir.getChildFile("settings.json").replaceWithText(juce::JSON::toString(juce::var(obj)));
}

void MainComponent::setMilkDropPresetDir(const juce::String& dir)
{
    if (dir == milkDropPresetDir_)
        return;

    milkDropPresetDir_ = dir;
    saveMilkDropPresetDirSetting(dir);

    auto allDirs = milkDropBaseDirs_;
    if (dir.isNotEmpty() && juce::File(dir).isDirectory())
        allDirs.push_back(dir.toStdString());

    // See Renderer::rescanMilkDropPresets() for why this must not call
    // presetManager_.setPresetDirectories()/rescan() directly: those
    // mutate presets_, which PresetSelector::processFrame reads every
    // frame with no lock on the GL thread.
    previewPanel_.getRenderer().rescanMilkDropPresets(allDirs);

    browserPanel_->getMilkDropBrowser().refresh();
}

MainComponent::~MainComponent()
{
    // Shutdown bundle piece 2 (2026-07-30, follow-up to cc5c0c3): stop the
    // HTTP servers FIRST, before detaching the GL renderer below. Closes a
    // disputed race class: ApiServer/TestServer route handlers reach into
    // the renderer, so an in-flight handler could otherwise still be
    // running during the detach() window. stop() blocks until server_.stop()
    // unblocks the listen() loop and serverThread_.join() returns — a few ms
    // httplib join, not UI teardown — so once these two calls return, no
    // HTTP worker thread can be touching the renderer. This still preserves
    // the shutdown scout's intent (scout-shutdown-sigbus.md): GL detaches
    // before ALL UI teardown further down, just after this non-UI server
    // join.
    if (apiServer_)
        apiServer_->stop();
#if AUDIODNA_TEST_SERVER
    if (testServer_)
        testServer_->stop();
#endif

    // Shutdown bundle piece 1 (2026-07-30, scout-shutdown-sigbus.md): detach
    // the GL renderer before any UI teardown. previewPanel_ is declared
    // before effectsRackPanel_ (MainComponent.h), so it destructs AFTER it —
    // without this, the OpenGL render thread stays live through the rest of
    // UI teardown (~PreviewPanel's own detach() runs ~48 members too late),
    // racing whatever UI-side destructor a member reallocation corrupts a
    // live juce::Label. detach() is already called from ~PreviewPanel and
    // ~Renderer in the normal teardown path, so this call is safe/idempotent
    // (JUCE's OpenGLContext::detach() no-ops when already detached) — it
    // just moves the first detach earlier.
    previewPanel_.getRenderer().detach();

    // W3 (outputwindow-arc, scout R5): the second GL context obeys the same
    // shutdown law — end its GL activity HERE, before any teardown below,
    // not 17 members later when outputWindow_.reset() runs. detach() is
    // idempotent, so the reset() further down stays where it is.
    if (outputWindow_)
        outputWindow_->getRenderer().detach();

    // Drop undo history on shutdown: commands hold model snapshots that must
    // not outlive the composition/renderer they refer to.
    undoManager_.clear();

    // P22: Stop remaining output/integration services
    oscHandler_.stopListening();
    midiOutputHandler_.closeDevice();
    videoRecorder_.stopRecording();

#if AUDIODNA_BUILD_INSPECTOR
    melatoninInspector_.reset();
#endif
#if AUDIODNA_HAS_CAMERA
    closeCamera();
#endif
    outputWindow_.reset();
    if (!testMode_)
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

    // Browser panel is now a real component (Phase 7) — no placeholder needed

    // Draw vertical dividers between bottom panels
    for (int i = 0; i < 3; ++i)
    {
        if (!vDividerBounds_[i].isEmpty())
        {
            auto vd = vDividerBounds_[i].toFloat();
            float midX = vd.getCentreX();
            g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
            g.drawVerticalLine(static_cast<int>(midX),
                               vd.getY() + 10.0f, vd.getBottom() - 10.0f);
        }
    }

    // Horizontal divider line below deck tabs
    if (!dividerBounds_.isEmpty())
    {
        float lineY = static_cast<float>(dividerBounds_.getCentreY());
        g.setColour(juce::Colour(0xff555577));
        g.drawHorizontalLine(static_cast<int>(lineY),
                             static_cast<float>(dividerBounds_.getX()),
                             static_cast<float>(dividerBounds_.getRight()));
    }

    // Horizontal divider — show up arrow on hover (can only drag up)
    if (hoveringHDivider_ && !dividerBounds_.isEmpty())
    {
        float cx = static_cast<float>(dividerBounds_.getCentreX());
        float cy = static_cast<float>(dividerBounds_.getCentreY());
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.7f));

        // Up arrow
        juce::Path up;
        up.addTriangle(cx - 5.0f, cy + 2.0f,
                       cx + 5.0f, cy + 2.0f,
                       cx, cy - 4.0f);
        g.fillPath(up);
    }

    // Vertical dividers — show left/right arrows on hover
    if (hoveringVDivider_ >= 0 && !vDividerBounds_[hoveringVDivider_].isEmpty())
    {
        auto vd = vDividerBounds_[hoveringVDivider_].toFloat();
        float cx = vd.getCentreX();
        float cy = vd.getCentreY();
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.7f));

        // Left arrow
        juce::Path left;
        left.addTriangle(cx - 6.0f, cy,
                         cx - 1.0f, cy - 5.0f,
                         cx - 1.0f, cy + 5.0f);
        g.fillPath(left);

        // Right arrow
        juce::Path right;
        right.addTriangle(cx + 6.0f, cy,
                          cx + 1.0f, cy - 5.0f,
                          cx + 1.0f, cy + 5.0f);
        g.fillPath(right);
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(4);

    // === v2: Top Bar (full width) ===
    if (topBar_)
    {
        topBar_->setBounds(area.removeFromTop(34));
        area.removeFromTop(1);
    }

    // === v2: Signal Bar (full width) ===
    bool signalBarExpanded = false;
    if (signalBar_)
    {
        int sbHeight = signalBar_->getPreferredHeight();
        if (sbHeight < 0)
        {
            // Expanded: take ALL remaining space, hide everything below
            signalBarExpanded = true;
            signalBar_->setBounds(area);
            signalBar_->setVisible(true);
            area = juce::Rectangle<int>();
        }
        else
        {
            signalBar_->setBounds(area.removeFromTop(sbHeight));
            signalBar_->setVisible(true);
            area.removeFromTop(1);
        }
    }

    // If signal bar is expanded, hide everything else and return
    if (signalBarExpanded)
    {
        previewPanel_.setVisible(false);
        waveformDisplay_.setVisible(false);
        audioReadoutPanel_.setVisible(false);
        spectrumDisplay_.setVisible(false);
        if (effectsRackPanel_) effectsRackPanel_->setVisible(false);
        if (deckView_) deckView_->setVisible(false);
        if (inspectorPanel_) inspectorPanel_->setVisible(false);
        if (browserPanel_) browserPanel_->setVisible(false);
        if (timingWindow_) timingWindow_->setVisible(false);
        return;
    }

    // === Hide v1 controls that are now in TopBar or removed ===
    audioSourceLabel_.setVisible(false);
    audioSourceSelector_.setVisible(false);
    inputGainLabel_.setVisible(false);
    inputGainSlider_.setVisible(false);
    masterLevelSlider_.setVisible(false);
    masterLevelLabel_.setVisible(false);
    displaySelector_.setVisible(false);
    outputLabel_.setVisible(false);
    fpsLabel_.setVisible(false);
    cpuLabel_.setVisible(false);
    viewportLabel_.setVisible(false);
    resolutionSelector_.setVisible(false);
    randomLabel_.setVisible(false);
    beatRandomToggle_.setVisible(false);
    beatCountSelector_.setVisible(false);
    syncButton_.setVisible(false);
    inputLevelMeterBounds_ = {};

    // === Row 1: Image + Camera + Presets (v1 compat, compact) ===
    auto row1 = area.removeFromTop(24);
    openImageButton_.setBounds(row1.removeFromLeft(70));
    row1.removeFromLeft(2);
    openFolderButton_.setBounds(row1.removeFromLeft(75));
    row1.removeFromLeft(2);
    imageBeatLabel_.setBounds(row1.removeFromLeft(80));
    imageBeatSelector_.setBounds(row1.removeFromLeft(50));
    row1.removeFromLeft(4);
  #if AUDIODNA_HAS_CAMERA
    cameraLabel_.setBounds(row1.removeFromLeft(42));
    cameraSelector_.setBounds(row1.removeFromLeft(100));
    row1.removeFromLeft(4);
  #endif
    savePresetButton_.setBounds(row1.removeFromLeft(40));
    row1.removeFromLeft(2);
    loadPresetButton_.setBounds(row1.removeFromLeft(40));
    row1.removeFromLeft(2);
    fastSaveButton_.setBounds(row1.removeFromLeft(50));
    row1.removeFromLeft(2);
    deckSaveButton_.setBounds(row1.removeFromLeft(60));
    row1.removeFromLeft(2);
    deckLoadButton_.setBounds(row1.removeFromLeft(60));
    fileLabel_.setBounds(row1);

    area.removeFromTop(2);

    // === v2: Deck View (main content area) ===
    // Deck takes the bulk of remaining space
    // Bottom: preset slots + keyboard (v1 compat)

    // Preset slots bar at bottom
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
        area.removeFromBottom(2);
    }

    // Hide v1 panels removed from v2 layout
    audioReadoutPanel_.setVisible(false);
    spectrumDisplay_.setVisible(false);
    if (effectsRackPanel_) effectsRackPanel_->setVisible(false);

    // === Deck + Bottom panels — locked together, drag up only ===
    int availableHeight = area.getHeight();

    // Natural height = snug fit around layers + triggers + tabs
    int naturalDeckHeight = deckView_ ? deckView_->getNaturalHeight() : 200;
    int maxDeckHeight = std::min(naturalDeckHeight, availableHeight - kMinBottomHeight);

    int deckHeight;
    if (deckDividerY_ > 0)
    {
        // User dragged up — cap at natural height (can't drag down past it)
        deckHeight = juce::jlimit(kMinDeckHeight, maxDeckHeight,
                                   deckDividerY_ - area.getY());
    }
    else
    {
        deckHeight = maxDeckHeight;
    }

    // Deck view
    if (deckView_)
    {
        deckView_->setBounds(area.removeFromTop(deckHeight));
        deckView_->setVisible(true);
    }
    else
    {
        area.removeFromTop(deckHeight);
    }

    // Thin visible divider line between deck and bottom panels
    // Takes a small strip from `area` so it doesn't overlap the deck
    dividerBounds_ = area.removeFromTop(kDividerHeight);

    // === Bottom panel area with draggable vertical dividers ===
    // 4 panels: preview | timing window | inspector | browser
    // 3 vertical dividers between them, positioned by vDividerFrac_[]
    bottomAreaX_ = area.getX();
    bottomAreaWidth_ = area.getWidth();
    int btmY = area.getY();
    int btmH = area.getHeight();

    // Compute panel edges from fractions
    int d0x = bottomAreaX_ + static_cast<int>(vDividerFrac_[0] * static_cast<float>(bottomAreaWidth_));
    int d1x = bottomAreaX_ + static_cast<int>(vDividerFrac_[1] * static_cast<float>(bottomAreaWidth_));
    int d2x = bottomAreaX_ + static_cast<int>(vDividerFrac_[2] * static_cast<float>(bottomAreaWidth_));

    // Store divider bounds for hit testing and painting
    vDividerBounds_[0] = { d0x, btmY, kVDividerWidth, btmH };
    vDividerBounds_[1] = { d1x, btmY, kVDividerWidth, btmH };
    vDividerBounds_[2] = { d2x, btmY, kVDividerWidth, btmH };

    // Panel bounds (between dividers)
    auto previewArea   = juce::Rectangle<int>(bottomAreaX_, btmY,
                                               d0x - bottomAreaX_, btmH);
    auto timingArea    = juce::Rectangle<int>(d0x + kVDividerWidth, btmY,
                                               d1x - d0x - kVDividerWidth, btmH);
    auto inspectorArea = juce::Rectangle<int>(d1x + kVDividerWidth, btmY,
                                               d2x - d1x - kVDividerWidth, btmH);
    auto browserArea   = juce::Rectangle<int>(d2x + kVDividerWidth, btmY,
                                               bottomAreaX_ + bottomAreaWidth_ - d2x - kVDividerWidth, btmH);

    // Preview + waveform (left)
    {
        int waveformHeight = std::max(30, static_cast<int>(previewArea.getHeight() * 0.12f));
        waveformDisplay_.setBounds(previewArea.removeFromBottom(waveformHeight));
        waveformDisplay_.setVisible(true);
        previewPanel_.setBounds(previewArea);
        previewPanel_.setVisible(true);
    }

    // Timing Window (center-left)
    if (timingWindow_)
    {
        timingWindow_->setBounds(timingArea);
        timingWindow_->setVisible(true);
    }

    // Inspector (center-right)
    if (inspectorPanel_)
    {
        inspectorPanel_->setBounds(inspectorArea);
        inspectorPanel_->setVisible(true);
    }

    // Browser panel (right)
    if (browserPanel_)
    {
        browserPanel_->setBounds(browserArea);
        browserPanel_->setVisible(true);
    }
    browserPlaceholderBounds_ = {}; // No longer a placeholder

    // Binding overlays — full window coverage (always on top)
    if (bindingOverlay_)
    {
        bindingOverlay_->setBounds(getLocalBounds());
        if (bindingOverlay_->isBindingModeActive())
            bindingOverlay_->toFront(false);
    }
    if (midiLearnOverlay_)
    {
        midiLearnOverlay_->setBounds(getLocalBounds());
        if (midiLearnOverlay_->isLearnModeActive())
            midiLearnOverlay_->toFront(false);
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

        PresetManager::LoadStats stats;
        if (PresetManager::loadPreset(file,
                                       previewPanel_.getEffectChain(),
                                       previewPanel_.getMappingEngine(),
                                       &stats))
        {
            fileLabel_.setText("Loaded: " + file.getFileNameWithoutExtension(),
                              juce::dontSendNotification);
            if (effectsRackPanel_)
                effectsRackPanel_->refreshFromChain();
            // Loading a composition is not itself undoable — drop stale history.
            undoManager_.clear();

            // Preset-retarget-fix W5: surface mapping-resolution issues on
            // the explicit Load path only — no modal on the slot/deck paths.
            if (stats.dropped > 0)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Some Mappings Dropped",
                    juce::String(stats.dropped) + " of " + juce::String(stats.mappingsTotal)
                    + " mapping(s) could not be re-targeted and were dropped:\n\n"
                    + stats.droppedDescriptions.joinIntoString("\n"));
            }
            else if (stats.legacyFile)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::InfoIcon,
                    "Legacy Preset",
                    "This preset was saved before targeting keys were added. Its "
                    "mappings loaded using their original chain positions — verify "
                    "they still target the right effects, then re-save to upgrade "
                    "this file permanently.");
            }
        }
    });
}

// L3 (2026-09): Composition persistence. File > Open/Save/Save As and
// Cmd+O/Cmd+S below operate on `composition_` — savePreset()/loadPreset()
// above remain the v1 FX-preset surface, reached only via their own row-1
// buttons (L-DEL's to delete). See
// .harmony/.work-packets/L3-composition-persistence.md §1/§5 STEP 2.

void MainComponent::refreshUiAfterModelSwap()
{
    // ORDER MATTERS. Null the inspectors FIRST: ClipInspector::clip_ /
    // LayerInspector::layer_ are raw pointers into Clip/Layer objects a model
    // swap just destroyed, and MainComponent::timerCallback() calls
    // inspectorPanel_->refresh() at ~10Hz unconditionally — an un-nulled
    // dangling pointer is dereferenced within ~100ms of this function
    // returning (ClipInspector::refresh() does `if (clip_) { syncFromClip(); }`,
    // and a dangling pointer is never null).
    if (inspectorPanel_)
    {
        inspectorPanel_->getClipInspector().setClip(nullptr);
        inspectorPanel_->getLayerInspector().setLayer(nullptr);
    }

    // DEVIATION from the work packet's literal step order (flagged in the
    // build report): the packet's §1 sequence calls
    // deckView_->clearSelection()/selectLayer(-1)/setActiveColumn(-1) here,
    // BEFORE rebuildGrid(). clearSelection() and selectLayer(-1) are safe at
    // this point (DeckView::updateSelectionVisuals() and LayerStrip::
    // setSelected() are coordinate/bool-only). But
    // DeckView::setActiveColumn(int) unconditionally calls DeckView::refresh(),
    // which — for any display row where the NEW active deck also has a layer
    // — calls the OLD LayerStrip's refresh() (LayerStrip.cpp:698:
    // `if (!layer_) return; layerName_ = juce::String(layer_->name);`).
    // `layer_` is a raw Layer* set by rebuildGrid()'s strip->setLayer(layer, …)
    // pointing into the OLD Deck, which is already destroyed by this point
    // (composition_ = std::move(incoming) inside the fence, above). That is a
    // second, independent UAF class from TRAP #1 (same shape: raw pointer +
    // an already-freed model object), on LayerStrip rather than
    // ClipInspector/LayerInspector, and it fires on essentially every normal
    // load (old and new decks both having a layer at row 0 is the common
    // case). Fix: run setActiveColumn(-1) AFTER rebuildGrid() has replaced
    // every LayerStrip/ClipCell with fresh ones pointing into the NEW model.
    if (deckView_)
    {
        deckView_->clearSelection();
        deckView_->selectLayer(-1);
        deckView_->rebuildGrid();   // destroys+recreates every ClipCell/LayerStrip
                                    // (their raw Clip*/Layer*) and the deck tabs.
        deckView_->setActiveColumn(-1);
    }

    if (inspectorPanel_)
    {
        // EffectStackView::refresh() (called by inspectorPanel_->refresh()
        // below) only recolors existing rows; it does not add/remove rows to
        // match a changed globalEffects size. rebuildCompositionEffects()
        // re-points the composition inspector's stack at the NEW
        // composition_.globalEffects and rebuilds row COUNT — must run
        // before the plain refresh() below, or the global-FX row count
        // stays stale (the OLD comp's).
        inspectorPanel_->rebuildCompositionEffects();
        inspectorPanel_->refresh();   // now safe: no dangling Clip*/Layer* remains.
    }

    // The renderer's global fallback (activeSourceType_ / loaded image) is not
    // deck state — the OLD comp's procedural source or still image would keep
    // rendering underneath an empty new deck without this (loaded layers have
    // activeClipColumn == -1, so nothing is "active" until triggered).
    if (auto* d = composition_.getActiveDeck())
        refreshPreviewFromActiveClip(*d);
}

void MainComponent::swapCompositionModel(const std::function<void()>& mutation)
{
    // Read OLD playable-clip ids while the old model is still live — reading
    // after `mutation` runs is too late, the ids it would report are gone.
    auto before = compload::playableClipIds(composition_);

    // GL fence: null activeDeck_, drain one in-flight GL frame, run `mutation`,
    // then re-point the renderer at composition_.getActiveDeck() (the NEW
    // active deck). This is the same mechanism kCompNew already used for
    // initDefault() — closes the reallocation-under-read UAF class for a
    // whole-composition swap too (Renderer::renderOpenGL()'s P21
    // persistent-layer loop over composition_->decks is nested under the
    // `deckActive` check, so nulling activeDeck_ fences it as well).
    undoService_.withDeckDetached(mutation);

    // Close by SET DIFFERENCE, after the fence: correct for a full swap/New
    // (closes every old id) AND for a deck-append (closes nothing — nothing
    // was retired). Doing this before the swap, or as "close everything old",
    // would black out the old comp's output before the cut and would be
    // wrong for deck-append (it would close media of decks that survive).
    auto after = compload::playableClipIds(composition_);
    auto& renderer = previewPanel_.getRenderer();
    for (auto id : compload::idsRetired(before, after))
        renderer.closeMediaForClip(id);

    // Loading/replacing/appending is not itself undoable. clear() only
    // touches command history (never the model) — a stale command left alive
    // could re-resolve, by coordinate, a valid-but-WRONG cell in the new
    // model (memory-safe, semantically wrong — see the work packet §2 proof).
    undoManager_.clear();

    refreshUiAfterModelSwap();
}

void MainComponent::openComposition()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Open Composition...",
        CompDecksBrowser::getCompositionsDir(),
        "*.json");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;
        loadComposition(file);
    });
}

// L3 STEP 3 (2026-09): per-clip media-open loop, factored out of
// loadComposition's OPEN NEW step so appendDeckFromFile (below) can share it
// verbatim rather than duplicate it. Mirrors applyFileDrop's video block and
// applyMultiFileDrop's sequence block — see loadComposition's step 4 comment
// for why this must run on the STAGED deck, before any fence/swap.
void MainComponent::openMediaForDeck(Deck& deck)
{
    auto& renderer = previewPanel_.getRenderer();
    for (auto& layer : deck.layers)
    {
        for (auto& cell : layer.clips)
        {
            if (!cell.has_value() || !cell->isPlayable()) continue;
            Clip& clip = *cell;
            if (clip.mediaType == Clip::MediaType::Video)
            {
                if (!clip.mediaFile.existsAsFile()) continue;   // non-fatal: skip, continue
                if (renderer.openVideoForClip(clip.id, clip.mediaFile))
                {
                    if (auto* p = renderer.getVideoPlayer(clip.id))
                    {
                        clip.hasAlpha = p->hasAlpha();
                        clip.clipWidth = p->getWidth();
                        clip.clipHeight = p->getHeight();
                        clip.thumbnail = p->getThumbnail(90, 72);
                    }
                }
            }
            else // ImageSequence
            {
                if (clip.sequenceFiles.empty()) continue;
                renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
                auto first = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
                if (first.isValid())
                    clip.thumbnail = first.rescaled(90, 72, juce::Graphics::lowResamplingQuality);
            }
        }
    }
}

void MainComponent::loadComposition(const juce::File& file)
{
    // 1. STAGE — load into a private `incoming`, never the live composition_:
    //    a well-formed-but-wrong-shape file (FX preset, lone deck) would
    //    otherwise "succeed" via fromVar, leaving every hasProperty-guarded
    //    field at its OLD value — a stale hybrid of two compositions.
    Composition incoming;
    if (!incoming.loadFromFile(file))
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Open Composition",
                file.getFileName() + ": could not read/parse file");
        return;   // Live state untouched.
    }

    // 2. VALIDATE — refuse (no decks / a deck with no layers) or repair
    //    (activeDeckIndex out of range, numColumns/padding) — on `incoming`
    //    only. Live state untouched either way.
    if (auto reason = compload::validateComposition(incoming); !reason.empty())
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Open Composition",
                file.getFileName() + ": " + reason);
        return;
    }

    // 3. RE-MINT — every clip gets a fresh id from the file-static mint.
    //    Saved ids are advisory (nothing persistent references them); a
    //    fresh id can never collide with a LIVE clip's id, which is what
    //    makes step 4 safe without needing L1-FU.
    compload::remintClipIds(incoming, s_nextClipId);

    // 4. OPEN NEW — open every playable clip's media under its new id and
    //    fill thumbnail/dims INTO `incoming`, BEFORE the swap (openMediaForDeck,
    //    shared with Step 3's appendDeckFromFile). Must happen pre-swap: a
    //    post-swap write into a live Clip would race the GL thread (juce::Image
    //    ref-count assignment = torn-read/crash class), and the output keeps
    //    showing the OLD comp for the whole file-probe duration instead of
    //    blacking out.
    for (auto& deck : incoming.decks)
        openMediaForDeck(deck);

    // 5. NAME — design decision: composition name = file base name on Load
    //    (and Save As), so the browser row, the inspector label, and Collect
    //    Media's folder name all agree. Write on `incoming`, not the live
    //    object.
    incoming.name = file.getFileNameWithoutExtension().toStdString();

    // 6. SWAP — fenced; closes orphaned media by set difference, clears undo
    //    history, nulls the inspectors, rebuilds the grid (see
    //    swapCompositionModel/refreshUiAfterModelSwap above).
    swapCompositionModel([this, &incoming] { composition_ = std::move(incoming); });

    // 7. LABEL
    fileLabel_.setText("Loaded: " + file.getFileNameWithoutExtension(), juce::dontSendNotification);
    if (browserPanel_)
        browserPanel_->getCompDecksBrowser().refresh();
}

void MainComponent::saveComposition()
{
    if (composition_.filePath != juce::File()
        && composition_.filePath.getParentDirectory().isDirectory())
    {
        if (composition_.saveToFile(composition_.filePath))
        {
            fileLabel_.setText("Saved: " + composition_.filePath.getFileName(),
                              juce::dontSendNotification);
            if (browserPanel_)
                browserPanel_->getCompDecksBrowser().refresh();
        }
        else if (!testMode_)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Save Composition",
                "Save failed: " + composition_.filePath.getFullPathName());
        }
    }
    else
    {
        saveCompositionAs();
    }
}

void MainComponent::saveCompositionAs()
{
    auto dir = CompDecksBrowser::getCompositionsDir();
    dir.createDirectory();

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Save Composition As...",
        dir.getChildFile(juce::String(composition_.name) + ".json"),
        "*.json");

    auto flags = juce::FileBrowserComponent::saveMode
               | juce::FileBrowserComponent::canSelectFiles
               | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;

        auto saveFile = file.hasFileExtension(".json") ? file
                            : file.withFileExtension("json");

        if (composition_.saveToFile(saveFile))
        {
            // saveToFile() is const and never sets filePath — only
            // loadFromFile() does. Save As must set it here, or a later
            // plain Save cannot find it.
            composition_.filePath = saveFile;
            composition_.name = saveFile.getFileNameWithoutExtension().toStdString();
            fileLabel_.setText("Saved: " + saveFile.getFileName(), juce::dontSendNotification);
            if (browserPanel_)
                browserPanel_->getCompDecksBrowser().refresh();
        }
        else if (!testMode_)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Save Composition",
                "Save failed: " + saveFile.getFullPathName());
        }
    });
}

// L3 STEP 3 (2026-09): the Comp/Decks browser's Decks-row click. APPENDS the
// saved deck into the live composition and makes it active — never replaces
// the active deck (see the header comment on appendDeckFromFile's
// declaration for why). Same STAGE -> VALIDATE -> RE-MINT -> OPEN NEW ->
// NAME -> SWAP shape as loadComposition, on a Deck instead of a Composition.
void MainComponent::appendDeckFromFile(const juce::File& file)
{
    // 1. STAGE + shape-check — a deck file's top level is `Deck::toVar()`'s
    //    shape ("layers"/"numColumns"/"name"/"id"), not a composition's
    //    ("decks") or an FX preset's. Refuse before touching the model:
    //    Deck::fromVar's own hasProperty-less getProperty calls would
    //    otherwise happily default-construct an empty/wrong Deck from either.
    auto parsed = juce::JSON::parse(file.loadFileAsString());
    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("layers"))
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Load Deck",
                file.getFileName() + ": not a deck file");
        return;   // Live state untouched.
    }

    Deck incoming;
    incoming.fromVar(parsed);

    // 2. VALIDATE — refuse (no layers) or repair (numColumns/padding), on
    //    `incoming` only. Live state untouched either way.
    if (auto reason = compload::validateDeck(incoming); !reason.empty())
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Load Deck",
                file.getFileName() + ": " + reason);
        return;
    }

    // 3. RE-MINT — every clip gets a fresh id from the SAME file-static mint
    //    loadComposition uses. A deck appended into a LIVE composition must
    //    never collide with an id already open in the renderer's media maps
    //    (or with another live deck's ids) — re-minting makes that
    //    impossible regardless of what the file's own ids were.
    compload::remintClipIds(incoming, s_nextClipId);

    // 4. OPEN NEW — shared with loadComposition's per-deck body.
    openMediaForDeck(incoming);

    // 5. NAME — a deck saved without a "name" key (fromVar's getProperty is
    //    unguarded and yields "" when the key is absent) falls back to the
    //    file's base name, matching loadComposition's convention.
    if (incoming.name.empty())
        incoming.name = file.getFileNameWithoutExtension().toStdString();

    // 6. SWAP — append, not replace: a performer clicking a saved deck
    //    mid-set must not lose the deck they are on. idsRetired(before, after)
    //    is empty for an append (nothing is orphaned), so nothing closes.
    //    The fence is still required: Composition::appendDeck's push_back
    //    can reallocate `decks`, which the GL thread walks lock-free
    //    (renderOpenGL()'s P21 persistent-layer loop) — same hazard class as
    //    the whole-composition swap, just on push_back instead of move-assign.
    //    The guard inside withDeckDetached re-points the renderer at the new
    //    active deck; rebuildGrid() (inside refreshUiAfterModelSwap) rebuilds
    //    the deck tabs too (setupDeckTabs() runs inside it).
    swapCompositionModel([this, &incoming] {
        // L5 Quantize follow-on (review round 2): appending a new deck
        // deactivates whichever deck was active — the same "queued trigger
        // freezes on an abandoned deck" bug AddDeckCmd/SwitchDeckCmd already
        // close, via the same shared helper (DeckCommands.h). This path isn't
        // undo-tracked (a raw model swap, like loadComposition — no Command
        // wraps appending a deck from a file), so there is no snapshot to
        // restore; the cancelled list is discarded.
        if (auto* leavingDeck = composition_.getActiveDeck())
            cancelPendingTriggers(*leavingDeck);
        composition_.activeDeckIndex = composition_.appendDeck(std::move(incoming));
    });

    // 7. LABEL
    fileLabel_.setText("Loaded deck: " + file.getFileNameWithoutExtension(), juce::dontSendNotification);
    if (browserPanel_)
        browserPanel_->getCompDecksBrowser().refresh();
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    auto mod = key.getModifiers();

    // If binding overlay is active, let it handle all keys
    if (bindingOverlay_ && bindingOverlay_->isBindingModeActive())
        return false; // Let the KeyListener on the overlay handle it
    if (midiLearnOverlay_ && midiLearnOverlay_->isLearnModeActive())
        return false;

#if AUDIODNA_BUILD_INSPECTOR
    // Shift+Cmd+I = toggle Melatonin Inspector
    if (key.isKeyCode('I') && mod.isCommandDown() && mod.isShiftDown())
    {
        if (!melatoninInspector_)
            melatoninInspector_ = std::make_unique<melatonin::Inspector>(*this);

        melatoninInspector_->setVisible(!melatoninInspector_->isVisible());
        return true;
    }
#endif

    // Shift+Cmd+K = toggle keyboard binding mode
    if (key.isKeyCode('K') && mod.isCommandDown() && mod.isShiftDown())
    {
        enterKeyboardBindingMode();
        return true;
    }

    // Shift+Cmd+M = toggle MIDI learn mode
    if (key.isKeyCode('M') && mod.isCommandDown() && mod.isShiftDown())
    {
        enterMidiLearnMode();
        return true;
    }

    // Escape = close output window
    if (key.isKeyCode(juce::KeyPress::escapeKey))
    {
        if (outputWindow_ && outputWindow_->isVisible())
        {
            closeOutput();
            displaySelector_.setSelectedId(1, juce::dontSendNotification);
        }
        return true;
    }

    // Cmd/Ctrl+Z = Undo, Cmd/Ctrl+Shift+Z = Redo
    if (key.isKeyCode('Z') && mod.isCommandDown())
    {
        if (mod.isShiftDown())
        {
            const bool movesLayer = undoManager_.redoAffectsLayerOrder();
            if (undoManager_.redo()) refreshAfterUndoRedo(movesLayer);
        }
        else
        {
            const bool movesLayer = undoManager_.undoAffectsLayerOrder();
            if (undoManager_.undo()) refreshAfterUndoRedo(movesLayer);
        }
        return true;
    }

    // Cmd/Ctrl+X = Clear selected clip(s) (Clip > Clear). No clipboard concept
    // exists at HEAD: MenuBarModel.h reserves kClipCut/kClipCopy/kClipPaste/
    // kClipCopyEffects/kClipPasteEffects enum values, but none is ever added
    // to a menu (MenuBarModel.cpp's Clip menu builds only Clear/Replace
    // Content/Lock Content) or handled in handleMenuCommand — grep-confirmed
    // 2026-07-30. So this is Cmd+X as a second shortcut on the existing Clear
    // action (smallest honest thing), not cut-to-clipboard; "Cut" would be a
    // lie in a menu label. kClipClear's own handler is already a safe no-op
    // with nothing selected, so no extra guard is needed here.
    if (key.isKeyCode('X') && mod.isCommandDown())
    {
        handleMenuCommand(AudioDNAMenuBar::kClipClear);
        return true;
    }

    // Cmd/Ctrl+S = save composition (L3, 2026-09; was save preset — the menu's
    // own Save items carry no KeyPress, MenuBarModel.cpp's `menu.addItem(kCompSave,
    // "Save", true, false)`, so this shortcut is wired only here). savePreset()'s
    // row-1 button caller is untouched — that's the FX-preset surface, L-DEL's.
    if (key.isKeyCode('S') && mod.isCommandDown())
    {
        handleMenuCommand(AudioDNAMenuBar::kCompSave);
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

    // Cmd/Ctrl+O = open composition (L3, 2026-09; was load preset — same
    // no-KeyPress-on-the-menu-item reasoning as Cmd+S above). loadPreset()'s
    // row-1 button caller is untouched.
    if (key.isKeyCode('O') && mod.isCommandDown())
    {
        handleMenuCommand(AudioDNAMenuBar::kCompOpen);
        return true;
    }

    // === v2 Binding System: try bound keys first ===
    if (!mod.isCommandDown())
    {
        if (bindingManager_.processKeyDown(key.getKeyCode(),
                                            mod.isShiftDown(),
                                            mod.isCommandDown(),
                                            mod.isAltDown()))
        {
            return true; // A binding handled this key
        }
    }

    return false;
}

bool MainComponent::keyPressed(const juce::KeyPress& /*key*/, juce::Component* /*originatingComponent*/)
{
    return false;
}

bool MainComponent::keyStateChanged(bool isKeyDown)
{
    // Route key releases to the binding manager for momentary/piano mode
    if (!isKeyDown)
    {
        // JUCE doesn't tell us WHICH key was released in keyStateChanged,
        // so we poll all currently-bound keyboard keys to detect releases.
        // This is the standard JUCE pattern for key-up detection.
        for (int i = 0; i < bindingManager_.getNumBindings(); ++i)
        {
            auto* b = bindingManager_.getBindingAt(i);
            if (!b || !b->enabled) continue;
            if (b->inputType != Binding::InputType::Keyboard) continue;
            if (b->triggerMode != Binding::TriggerMode::Momentary) continue;

            // Check if this bound key is currently released
            if (!juce::KeyPress::isKeyCurrentlyDown(b->keyCode))
            {
                // Fire the release action
                handleBindingAction(*b, 0.0f);
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

void MainComponent::tickFeaturePipeline()
{
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();

    // S166-L1: SignalRegistry is confined to the message thread — this is
    // now the ONE call site that evaluates it, once per tick, before anything
    // reads the registry's cached values (macros below, SignalBar's own
    // repaint timer). GL threads no longer evaluate; see Signal.h and
    // SignalRegistry::evaluateAll.
    signalRegistry_.evaluateAll(snap);

    previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain());

    // L9 (modulation-freeze fix, 2026-09-05): drive the shared global
    // MacroBank and every Inspector's signal/macro-driven effect-param
    // modulation from this same unconditional 120Hz message-thread timer,
    // instead of the ~10Hz InspectorPanel::refresh() timer gated to whichever
    // tab is active — see InspectorPanel::tickModulation() /
    // EffectStackView::tickModulation(). MacroBank updates first so
    // tickModulation()'s getMacroValue() reads this tick's value rather than
    // the previous one.
    globalMacroBank_.updateValues(signalRegistry_);
    if (inspectorPanel_) inspectorPanel_->tickModulation();
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

        // v2: Feed stats to TopBar
        if (topBar_)
        {
            topBar_->setFps(fps);
            topBar_->setDspLoad(cpu);
        }
    }

    // Repaint input level meter
    if (!inputLevelMeterBounds_.isEmpty())
        repaint(inputLevelMeterBounds_);

    // Refresh inspector at ~10Hz to show signal-driven values
    if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
        inspectorPanel_->refresh();

    // P21: Ableton Link sync — update cached state and feed BPM tracker
    if (linkSync_.isEnabled())
    {
        linkSync_.update();
        double linkBPM = linkSync_.getBPM();
        if (linkBPM > 0.0)
        {
            if (auto* tracker = analysisThread_.getBpmTracker())
            {
                tracker->setManualMode(true);
                tracker->setManualBPM(static_cast<float>(linkBPM));
            }
        }
    }

    // P22.10: Update MIDI output pad feedback (~6Hz)
    if (uiUpdateCounter_ == 0 && midiOutputHandler_.isOpen())
        midiOutputHandler_.updateFromDeck(composition_.getActiveDeck());

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
    return PresetManager::getFxSaveDirectory();
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

    PresetManager::LoadStats stats;
    if (PresetManager::loadPreset(file,
                                   previewPanel_.getEffectChain(),
                                   previewPanel_.getMappingEngine(),
                                   &stats))
    {
        // Preset-retarget-fix W5: no modal on this performance surface —
        // flag dropped mappings inline in the label instead.
        fileLabel_.setText("Slot " + juce::String(slot + 1) + ": "
                          + file.getFileNameWithoutExtension()
                          + (stats.dropped > 0 ? " (check mappings)" : ""),
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
        PresetManager::getDeckDirectory(),
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
        if (!slideshowImages_.isEmpty())
            deck.imageFolderPath = slideshowImages_[0].getParentDirectory();
        deck.slideshowBeatsPerImage = slideshowBeats_;
        deck.beatRandomCount = beatRandomCount_;
        deck.beatRandomEnabled = beatRandomToggle_.getToggleState();
        deck.audioSourceMode = audioSourceSelector_.getSelectedId();
        deck.viewportResolution = resolutionSelector_.getSelectedId();
        deck.outputDisplay = displaySelector_.getSelectedId();
        deck.inputGain = static_cast<float>(inputGainSlider_.getValue());
        deck.masterVideoLevel = static_cast<float>(masterLevelSlider_.getValue());
        deck.showAudioPanel = true;
        deck.showFxPanel = true;
        deck.showWavePanel = true;
        deck.showKeysPanel = true;
        deck.showPresetsPanel = true;

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
        PresetManager::getDeckDirectory(),
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

        // Restore settings
        slideshowBeats_ = deck.slideshowBeatsPerImage > 0 ? deck.slideshowBeatsPerImage : 8;
        beatRandomCount_ = deck.beatRandomCount > 0 ? deck.beatRandomCount : 4;
        beatRandomToggle_.setToggleState(deck.beatRandomEnabled, juce::dontSendNotification);

        // Restore beats per image selector
        {
            const int beats[] = { 2, 4, 8, 16, 32, 64, 128 };
            for (int i = 0; i < 7; ++i)
                if (beats[i] == slideshowBeats_)
                    { imageBeatSelector_.setSelectedId(i + 1, juce::dontSendNotification); break; }
        }

        // Restore beat random count selector
        {
            const int counts[] = { 1, 2, 4, 8, 16, 32 };
            for (int i = 0; i < 6; ++i)
                if (counts[i] == beatRandomCount_)
                    { beatCountSelector_.setSelectedId(i + 1, juce::dontSendNotification); break; }
        }

        // Restore image folder slideshow
        if (deck.imageFolderPath.isDirectory())
        {
            slideshowImages_.clear();
            for (const auto& f : deck.imageFolderPath.findChildFiles(
                juce::File::findFiles, false, "*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tiff"))
                slideshowImages_.add(f);
            slideshowImages_.sort();
            slideshowIndex_ = 0;
            slideshowBeatCounter_ = 0;
        }

        // Restore UI selectors — use sendNotificationSync so handlers fire
        if (deck.viewportResolution > 0)
            resolutionSelector_.setSelectedId(deck.viewportResolution, juce::sendNotificationSync);

        // Restore audio source — trigger onChange to switch engine mode
        if (deck.audioSourceMode > 0)
            audioSourceSelector_.setSelectedId(deck.audioSourceMode, juce::sendNotificationSync);

        // Restore output display
        if (deck.outputDisplay > 1)
            displaySelector_.setSelectedId(deck.outputDisplay, juce::sendNotificationSync);

        // Restore input gain and master level
        if (deck.inputGain > 0.0f)
            inputGainSlider_.setValue(deck.inputGain, juce::sendNotificationSync);
        masterLevelSlider_.setValue(deck.masterVideoLevel, juce::sendNotificationSync);

        // Panel visibility is no longer user-togglable (v2 layout)
        // Ignore saved panel states — kept for backward compat in deck files

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

        // Loading a legacy deck replaces app state — drop stale undo history.
        undoManager_.clear();

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
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    float phase = snap.beatPhase;

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
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    float phase = snap.beatPhase;

    // Detect beat: phase wrapped around (went from high to low)
    bool beatDetected = (phase < lastBeatPhase_ - 0.5f);
    lastBeatPhase_ = phase;

    if (!beatDetected)
        return;

    // === Global effects randomize (existing behavior) ===
    ++beatCounter_;
    if (beatRandomToggle_.getToggleState() && beatCounter_ >= beatRandomCount_)
    {
        beatCounter_ = 0;
        juce::MessageManager::callAsync([this] { randomizeAllEffects(); });
    }

}

// === P23: ISF Shader Import ===

void MainComponent::handleImportISF()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Import ISF Shader",
        ISFShaderLoader::getISFDirectory(),
        "*.fs;*.isf;*.frag");

    chooser->launchAsync(juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser](const juce::FileChooser& fc) {
            auto results = fc.getResults();
            if (results.isEmpty()) return;

            auto file = results[0];
            auto isf = ISFShaderLoader::parseISFFile(file);

            if (!isf.valid)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "Import Failed",
                    "Could not parse ISF shader: " + file.getFileName());
                return;
            }

            // Convert to our GLSL format
            auto glsl = ISFShaderLoader::convertToGLSL(isf);

            // Register in effect library
            EffectLibrary::EffectDef def;
            def.name = juce::String("ISF: " + isf.name);
            def.category = "isf";
            def.shaderName = "isf_" + isf.name;

            for (const auto& param : isf.params)
            {
                EffectLibrary::ParamDef pd;
                pd.name = param.name;
                pd.uniformName = "u_isf_" + param.name;
                pd.defaultValue = param.defaultValue;
                def.params.push_back(std::move(pd));
            }

            // D2 (preset-retarget-fix): reject on name/shaderName collision
            // or intra-def duplicate param/uniform names — both are used as
            // preset targeting keys (D1) and must stay unique.
            if (!previewPanel_.getRenderer().getEffectLibrary().registerDynamic(def))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::MessageBoxIconType::WarningIcon,
                    "ISF Import Rejected",
                    "\"" + juce::String(isf.name) + "\" was not imported: its name, "
                    "shader key, or a parameter name/uniform collides with an "
                    "existing effect definition.");
                return;
            }

            // Compile the shader
            // Note: ShaderManager needs GL context. Queue for GL thread compilation.
            std::cerr << "[ISF] Imported: " << isf.name << " with "
                      << isf.params.size() << " params" << std::endl;

            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::InfoIcon,
                "ISF Import Successful",
                "Imported \"" + juce::String(isf.name) + "\" with "
                + juce::String(static_cast<int>(isf.params.size())) + " parameters.\n\n"
                + "Find it in the FX Browser under the ISF category.");
        });
}

// === v2: Deck View Handlers ===

void MainComponent::handleClipTrigger(int layerIndex, int column)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return;

    auto* layer = deck->getLayer(layerIndex);
    if (!layer) return;

    // Undo capture (mutate-then-push, spec §2 row 1 / step 8). Snapshot the
    // per-layer runtime + the target cell clip's `playing` BEFORE the trigger.
    // Runtime-only field writes → NO GL fence; the command re-resolves its target
    // by coordinate. Autopilot never reaches this handler (it calls
    // Layer::triggerClip directly from the GL thread), so autopilot triggers
    // create no commands.
    const LayerRuntimeSnapshot rtBefore = captureLayerRuntime(*layer);
    std::optional<bool> playBefore;
    if (const Clip* tc = layer->getClipAt(column)) playBefore = tc->playing;

    // Retrigger-restart (2026-07-30, Boris's recorded expectation): clicking the
    // already-playing cell is this same column == layer->activeClipColumn case.
    const bool wasRetrigger = (column == layer->activeClipColumn);

    // L5 Quantize: the global Quantize control forces a beat-snap granularity
    // on this one trigger (queues it) unless it's Off or the tracker isn't
    // locked yet — see quantizeModeToForcedSnap above.
    const FeatureSnapshot quantizeSnap = analysisThread_.getFeatureBus().read();
    const auto forcedSnap = quantizeModeToForcedSnap(composition_.quantizeMode, quantizeSnap);
    layer->triggerClip(column, forcedSnap);

    const LayerRuntimeSnapshot rtAfter = captureLayerRuntime(*layer);
    std::optional<bool> playAfter;
    if (const Clip* tc = layer->getClipAt(column)) playAfter = tc->playing;

    // Load the clip content into preview
    if (auto* clip = layer->getActiveClip())
    {
        // Mark as triggered (for future use)
        clip->hasBeenTriggered = true;
        // Apply beat snap: sync playhead to current beat phase on trigger
        if (clip->beatSnap && clip->isPlayable())
        {
            const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
            if (snap.beatPhase >= 0.0f)
            {
                double beatPos = static_cast<double>(snap.beatPhase);
                clip->playheadPosition = beatPos;

                // Seek the video player or image sequence
                auto& renderer = previewPanel_.getRenderer();
                if (clip->mediaType == Clip::MediaType::Video)
                {
                    auto* player = renderer.getVideoPlayer(clip->id);
                    if (player) player->seekTo(beatPos);
                }
                else if (clip->mediaType == Clip::MediaType::ImageSequence)
                {
                    auto* seq = renderer.getImageSequence(clip->id);
                    if (seq) seq->seekTo(beatPos);
                }
            }
        }
        else if (wasRetrigger && clip->isPlayable())
        {
            // Retrigger-restart: Layer::triggerClipImmediate's retrigger branch
            // (column == activeClipColumn) already resets the MODEL's
            // playheadPosition to inPoint, but the renderer overwrites
            // clip->playheadPosition FROM the player's actual position every
            // frame — so without also seeking the player itself, the model
            // reset was invisible (scout-diagnosed emergent no-op, triage
            // 2026-07-30). Seek the real player/sequence to in-point, mirroring
            // the beat-snap seek above and the cuepoint-jump seek elsewhere in
            // this file. Does not touch clip->playing (retrigger preserves
            // play/pause state, per Layer.h) or any undo-tracked field, so the
            // existing "retrigger pushes no history entry" behavior is unchanged.
            auto& renderer = previewPanel_.getRenderer();
            if (clip->mediaType == Clip::MediaType::Video)
            {
                auto* player = renderer.getVideoPlayer(clip->id);
                if (player) player->seekTo(clip->inPoint);
            }
            else if (clip->mediaType == Clip::MediaType::ImageSequence)
            {
                auto* seq = renderer.getImageSequence(clip->id);
                if (seq) seq->seekTo(clip->inPoint);
            }
        }

        if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
        {
            previewPanel_.getRenderer().clearActiveSource();
            previewPanel_.loadImage(clip->mediaFile);
            currentImageFile_ = clip->mediaFile;
            if (outputWindow_)
                outputWindow_->loadImage(clip->mediaFile);
            fileLabel_.setText(clip->mediaFile.getFileName(), juce::dontSendNotification);
        }
        else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
        {
            previewPanel_.getRenderer().setActiveSource(clip->sourceType);
            previewPanel_.getRenderer().clearImage();
            currentImageFile_ = juce::File();
            fileLabel_.setText(juce::String(clip->sourceType), juce::dontSendNotification);
        }
        else if (clip->mediaType == Clip::MediaType::Video && clip->mediaFile.existsAsFile())
        {
            // Ensure video player is open for this clip
            auto& renderer = previewPanel_.getRenderer();
            if (!renderer.getVideoPlayer(clip->id))
                renderer.openVideoForClip(clip->id, clip->mediaFile);

            // Video clips rendered via compositor — clear single-image path
            renderer.clearActiveSource();
            renderer.clearImage();
            currentImageFile_ = juce::File();
            fileLabel_.setText(clip->mediaFile.getFileName(), juce::dontSendNotification);
        }
        else if (clip->mediaType == Clip::MediaType::ImageSequence && !clip->sequenceFiles.empty())
        {
            // Ensure image sequence is open for this clip
            auto& renderer = previewPanel_.getRenderer();
            if (!renderer.getImageSequence(clip->id))
                renderer.openImageSequenceForClip(clip->id, clip->sequenceFiles, clip->sequenceFps);

            renderer.clearActiveSource();
            renderer.clearImage();
            currentImageFile_ = juce::File();
            auto frameCount = static_cast<int>(clip->sequenceFiles.size());
            fileLabel_.setText(juce::String(clip->name) + " (" + juce::String(frameCount) + " frames)",
                              juce::dontSendNotification);
        }
    }
    else
    {
        // No active clip — clear preview
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        currentImageFile_ = juce::File();
        fileLabel_.setText("", juce::dontSendNotification);
    }

    if (deckView_)
        deckView_->refresh();

    // Push the trigger command unless it changed nothing (spec §3: retrigger of
    // the already-active cell early-outs into a playhead reset — no runtime and
    // no target-`playing` change → pushes nothing, so no inert history entry).
    // Consecutive same-layer triggers coalesce in UndoManager::perform via
    // TriggerClipCmd::canMergeWith/mergeWith — one history slot per layer run.
    if (!(rtBefore == rtAfter) || playBefore != playAfter)
    {
        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<TriggerClipCmd>(
            makeLayerResolver(), composition_.activeDeckIndex, layerIndex, column,
            rtBefore, rtAfter, playBefore, playAfter, "Trigger Clip"));
        pushCommands(std::move(children), "Trigger Clip");
    }
}

void MainComponent::handleColumnTrigger(int column)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return;

    // Undo capture (mutate-then-push, spec §2 row 2 / step 8): a column trigger
    // is a composite of one TriggerClipCmd per NON-ignoring layer that actually
    // changes — mirroring Deck::triggerColumn, which skips ignoreColumnTrigger
    // layers. Snapshot each considered layer's runtime + target-`playing` BEFORE.
    const int numLayers = deck->getNumLayers();
    std::vector<LayerRuntimeSnapshot> before(static_cast<size_t>(numLayers));
    std::vector<std::optional<bool>> playBefore(static_cast<size_t>(numLayers));
    std::vector<bool> considered(static_cast<size_t>(numLayers), false);
    for (int l = 0; l < numLayers; ++l)
    {
        auto* layer = deck->getLayer(l);
        if (!layer || layer->ignoreColumnTrigger) continue;  // excluded, as triggerColumn does
        considered[static_cast<size_t>(l)] = true;
        before[static_cast<size_t>(l)] = captureLayerRuntime(*layer);
        if (const Clip* tc = layer->getClipAt(column))
            playBefore[static_cast<size_t>(l)] = tc->playing;
    }

    // L5 Quantize: same forced-snap decision as handleClipTrigger, applied once
    // for the whole column so every non-ignoring layer queues/fires together.
    const FeatureSnapshot quantizeSnap = analysisThread_.getFeatureBus().read();
    const auto forcedSnap = quantizeModeToForcedSnap(composition_.quantizeMode, quantizeSnap);
    deck->triggerColumn(column, forcedSnap);

    // One child per considered layer whose runtime or target-`playing` changed;
    // pushCommands composites them into one slot (a single changed layer collapses
    // to a lone TriggerClipCmd, which may then merge into a prior same-layer run —
    // accepted, consistent with spec §3's per-layer merge).
    std::vector<std::unique_ptr<Command>> children;
    for (int l = 0; l < numLayers; ++l)
    {
        if (!considered[static_cast<size_t>(l)]) continue;
        auto* layer = deck->getLayer(l);
        if (!layer) continue;
        const LayerRuntimeSnapshot after = captureLayerRuntime(*layer);
        std::optional<bool> playAfter;
        if (const Clip* tc = layer->getClipAt(column))
            playAfter = tc->playing;
        if (!(before[static_cast<size_t>(l)] == after)
            || playBefore[static_cast<size_t>(l)] != playAfter)
            children.push_back(std::make_unique<TriggerClipCmd>(
                makeLayerResolver(), composition_.activeDeckIndex, l, column,
                before[static_cast<size_t>(l)], after,
                playBefore[static_cast<size_t>(l)], playAfter, "Trigger Column"));
    }
    pushCommands(std::move(children), "Trigger Column");

    if (deckView_)
    {
        deckView_->setActiveColumn(column);
        deckView_->refresh();
    }

    // Load the bottom-most active clip's content into preview
    bool foundActiveClip = false;
    for (int i = 0; i < deck->getNumLayers(); ++i)
    {
        auto* layer = deck->getLayer(i);
        if (!layer) continue;
        if (auto* clip = layer->getActiveClip())
        {
            if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
            {
                previewPanel_.getRenderer().clearActiveSource();
                previewPanel_.loadImage(clip->mediaFile);
                currentImageFile_ = clip->mediaFile;
                if (outputWindow_)
                    outputWindow_->loadImage(clip->mediaFile);
                fileLabel_.setText(clip->mediaFile.getFileName(), juce::dontSendNotification);
                foundActiveClip = true;
                break;
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                currentImageFile_ = juce::File();
                fileLabel_.setText(juce::String(clip->sourceType), juce::dontSendNotification);
                foundActiveClip = true;
                break;
            }
        }
    }

    if (!foundActiveClip)
    {
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        currentImageFile_ = juce::File();
        fileLabel_.setText("", juce::dontSendNotification);
    }
}

// A1 fix (2026-07-30): the previewPanel_ renderer's fallback state
// (activeSourceType_ / loaded image / currentImageFile_ / fileLabel_) is
// GLOBAL to the whole deck, but a layer's X-clear is PER-LAYER — naively
// purging that global state on any layer's clear could blank a DIFFERENT
// layer's still-playing visual even though nothing about ITS clip changed.
//
// OWNERSHIP RULE: the renderer's fallback path only matters when
// Renderer.cpp's compositor returns no content for the WHOLE active deck
// (CompositorEngine::hasActiveLayers_ false — no visible, non-bypassed layer
// has an active clip with media/effects); otherwise the compositor output
// takes priority and the fallback state is not visually used at all. So
// after a layer's clip is cleared, rescan every layer in the SAME deck (not
// just the cleared one) for a still-active Image/Source clip and re-point
// the preview at it — exactly mirroring handleColumnTrigger's post-trigger
// preview refresh above (:3054-3092), which already performs this same
// rescan-or-purge after every trigger. Only purge when NO layer anywhere in
// the deck still owns active content, matching the exact condition under
// which Renderer.cpp's fallback would otherwise render stale content.
void MainComponent::refreshPreviewFromActiveClip(Deck& deck)
{
    bool foundActiveClip = false;
    for (int i = 0; i < deck.getNumLayers(); ++i)
    {
        auto* otherLayer = deck.getLayer(i);
        if (!otherLayer) continue;
        if (auto* clip = otherLayer->getActiveClip())
        {
            if (clip->mediaType == Clip::MediaType::Image && clip->mediaFile.existsAsFile())
            {
                previewPanel_.getRenderer().clearActiveSource();
                previewPanel_.loadImage(clip->mediaFile);
                currentImageFile_ = clip->mediaFile;
                if (outputWindow_)
                    outputWindow_->loadImage(clip->mediaFile);
                fileLabel_.setText(clip->mediaFile.getFileName(), juce::dontSendNotification);
                foundActiveClip = true;
                break;
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                currentImageFile_ = juce::File();
                fileLabel_.setText(juce::String(clip->sourceType), juce::dontSendNotification);
                foundActiveClip = true;
                break;
            }
        }
    }

    if (!foundActiveClip)
    {
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        currentImageFile_ = juce::File();
        fileLabel_.setText("", juce::dontSendNotification);
    }
}

// === Undo command construction helpers (Undo v1 step 2) ===

ClipLayerResolver MainComponent::makeLayerResolver()
{
    return [this](int deckIndex, int layerIndex) {
        return undoService_.resolveLayer(deckIndex, layerIndex);
    };
}

ClipDeckResolver MainComponent::makeDeckResolver()
{
    return [this](int deckIndex) {
        return undoService_.resolveDeck(deckIndex);
    };
}

ClipMediaHook MainComponent::makeClipMediaHook()
{
    // Risk #4 guard, "attach" half: reconnect-if-missing keeps undo/redo
    // correct now that closeMediaForClip is actually wired up (media-leak
    // fix, L1) — a clip landing back in a cell via undo must reopen media
    // this same clip's dispose (below) may have just closed.
    return [this](const Clip& clip) {
        if (!clip.isPlayable()) return;
        auto& renderer = previewPanel_.getRenderer();
        if (clip.mediaType == Clip::MediaType::Video)
        {
            // Reopen if the player is missing OR loaded a different file than
            // the clip now wants (id-stable content swap on replace-undo/redo).
            const bool exists = renderer.getVideoPlayer(clip.id) != nullptr;
            if (needsVideoReopen(renderer.getVideoPlayerFile(clip.id), clip.mediaFile, exists))
                renderer.openVideoForClip(clip.id, clip.mediaFile);
        }
        else if (clip.mediaType == Clip::MediaType::ImageSequence)
        {
            // Image sequences can't be content-swapped under an existing id
            // (replace only produces Image/Video; sequences always get a fresh
            // id), so reconnect-if-missing is sufficient here.
            if (!renderer.getImageSequence(clip.id))
                renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
        }
    };
}

ClipMediaDisposeHook MainComponent::makeClipMediaDisposeHook()
{
    // Risk #4 guard, "close" half (media-leak fix, L1, 2026-09). Commands
    // call this with a clip that is LEAVING a cell and whose id they've
    // already determined (by their own before/after-snapshot orphan check —
    // see SetClipCmd/SwapClipsCmd/ClearLayerClipsCmd/RemoveColumnCmd) is not
    // retained by anything THAT COMMAND just wrote.
    //
    // FUTURE-FRAGILE — READ BEFORE TOUCHING CLIPBOARD/DUPLICATE: this is safe
    // ONLY because a clip id can appear in AT MOST one live cell at a time —
    // ids are minted at 6 sites, are monotonic with no reuse, and there is no
    // clipboard/duplicate feature today. Under that invariant, "this command
    // is retiring id X" can only ever mean "X is not live anywhere," because
    // nothing else could hold X. The liveness scan below is the SECOND,
    // defensive half of that guarantee: it re-walks every deck/layer/cell in
    // the live composition and skips the close if the id turns up anywhere,
    // so a violated invariant fails SAFE (a leak) instead of unsafe (closing
    // a handle a visible clip still needs). If a future wave adds copy/paste
    // or any other way to put the same clip id in a second cell, this is the
    // function that needs to change — a per-command orphan check alone would
    // no longer be sufficient once an id can be live in more than one place.
    return [this](const Clip& clip) {
        if (!clip.isPlayable()) return;
        for (auto& deck : composition_.decks)
            for (auto& layer : deck.layers)
                for (auto& cell : layer.clips)
                    if (cell.has_value() && cell->id == clip.id)
                        return;   // still live somewhere — do not close
        previewPanel_.getRenderer().closeMediaForClip(clip.id);
    };
}

DeckFenceHook MainComponent::makeDeckFence()
{
    // Fence structure-changing layer mutations through UndoService::withDeckDetached
    // (GL fence, validated in build step 1). Runs on the message thread; execute/
    // undo/redo of AddLayerCmd/RemoveLayerCmd/MoveLayerCmd all route through here.
    return [this](const std::function<void()>& mutation) {
        undoService_.withDeckDetached(mutation);
    };
}

CompositionResolver MainComponent::makeCompositionResolver()
{
    // The Composition is a stable member — its address never changes; deck-vector
    // commands re-resolve it each apply to stay pointer-free (the decks vector
    // inside is what reallocates, which the DeckFenceHook guards).
    return [this]() -> Composition* { return &composition_; };
}

DeckActivateHook MainComponent::makeDeckActivateHook()
{
    // SwitchDeckCmd re-points the renderer at the current active deck on
    // execute/undo/redo — the same atomic handoff handleDeckSwitch performs live.
    return [this]() {
        previewPanel_.getRenderer().setActiveDeck(composition_.getActiveDeck());
    };
}

std::function<void()> MainComponent::makeEffectStackRefresh()
{
    // Fired by EffectStackCmd on execute/undo/redo — a lightweight notification
    // (recolor / re-value / re-size inspector content); it does NOT rebuild rows.
    // That only means the COMMAND's own apply() never rebuilds; it does NOT keep
    // an expanded row open across undo/redo. Every undo/redo call site then runs
    // refreshAfterUndoRedo, which unconditionally re-points the inspectors
    // (setClip/setLayer/rebuildCompositionEffects -> setEffects -> rebuildRows,
    // where expanded=false is hard-coded), collapsing all rows — and that is what
    // actually reflects a row-COUNT change. Row-expansion preservation is a
    // separate follow-up (a pointer/scope-aware skip in the refresh path), NOT a
    // guarantee of this command.
    return [this]() {
        if (inspectorPanel_)
            inspectorPanel_->refresh();
    };
}

std::optional<Clip> MainComponent::snapshotCell(Layer* layer, int column)
{
    if (layer == nullptr) return std::nullopt;
    if (Clip* clip = layer->getClipAt(column))
        return std::optional<Clip>(*clip);
    return std::nullopt;
}

std::unique_ptr<Command> MainComponent::makeSetClipCmd(int deckIndex, const CellEdit& edit,
                                                       const juce::String& description)
{
    return std::make_unique<SetClipCmd>(makeLayerResolver(), makeDeckFence(), makeClipMediaHook(),
                                        makeClipMediaDisposeHook(),
                                        deckIndex, edit.layerIndex, edit.column,
                                        edit.before, edit.after, description.toStdString());
}

void MainComponent::pushCommands(std::vector<std::unique_ptr<Command>> children,
                                 const juce::String& compositeDescription)
{
    if (children.empty())
        return;
    if (children.size() == 1)
    {
        undoManager_.perform(std::move(children.front()));
        return;
    }
    auto composite = std::make_unique<CompositeCommand>(compositeDescription.toStdString());
    for (auto& child : children)
        composite->add(std::move(child));
    if (!composite->isEmpty())   // guard: never perform an empty composite
        undoManager_.perform(std::move(composite));
}

void MainComponent::pushClipEdits(int deckIndex, std::vector<CellEdit> edits,
                                  const juce::String& description)
{
    std::vector<std::unique_ptr<Command>> children;
    children.reserve(edits.size());
    for (auto& edit : edits)
        children.push_back(makeSetClipCmd(deckIndex, edit, description));
    pushCommands(std::move(children), description);
}

void MainComponent::refreshAfterUndoRedo(bool affectsLayerOrder)
{
    // Grid rebuild via the shared helper (active deck unchanged in step 2).
    undoService_.syncAfterModelChange(UndoService::SyncScope::Grid);

    // P24.13: undo/redo of a layer reorder round-trips through this same
    // rebuild path (MoveLayerCmd::undo/execute -> deck->moveLayer), which
    // shifts layer indices with the layer count unchanged — the same stale-
    // selection mis-map as the live Move Layer Up/Down handlers (see the
    // comments there), just reached via Cmd+Z/Cmd+Shift+Z or the Composition
    // menu instead of the direct handler. Those handlers clear the multi-cell
    // clip selection right next to their own rebuildGrid(); mirror that here
    // for the undo/redo direction. affectsLayerOrder is Command::
    // affectsLayerOrder() of the command that was just processed, captured by
    // the caller via undoManager_.undoAffectsLayerOrder()/
    // redoAffectsLayerOrder() (matching direction) BEFORE calling undo()/
    // redo() — a structural flag rather than a stringly-typed description
    // match, so it can't misfire on an unrelated command that happens to
    // share display text with a layer move.
    if (deckView_ && affectsLayerOrder)
        deckView_->clearSelection();

    // Re-point the clip inspector BY COORDINATE (the currently selected cell)
    // so an undo that emptied/replaced that cell can't leave a dangling Clip*.
    // Uses setClip directly to avoid switching the active inspector tab.
    if (inspectorPanel_ && deckView_)
    {
        Clip* fresh = nullptr;
        EffectScope clipScope = EffectScope::none();
        const auto& selection = deckView_->getSelectedCells();
        if (!selection.empty())
        {
            fresh = undoService_.resolveClip(composition_.activeDeckIndex,
                                             selection.front().layer,
                                             selection.front().column);
            clipScope = EffectScope::clip(composition_.activeDeckIndex,
                                          selection.front().layer,
                                          selection.front().column);
        }
        // setClip re-points the clip inspector's effect stack too, so an effect
        // add/remove/bypass undo rebuilds the shown clip chain with the right scope.
        inspectorPanel_->getClipInspector().setClip(fresh, clipScope);

        // Re-point the layer inspector BY COORDINATE too (risk #3): a layer
        // add/remove/move undo can leave it holding a dangling Layer*. A stale
        // index resolves to nullptr, which setLayer clears null-safely.
        const int selLayer = deckView_->getSelectedLayerIndex();
        Layer* freshLayer = (selLayer >= 0)
            ? undoService_.resolveLayer(composition_.activeDeckIndex, selLayer)
            : nullptr;
        const EffectScope layerScope = (selLayer >= 0)
            ? EffectScope::layer(composition_.activeDeckIndex, selLayer)
            : EffectScope::none();
        inspectorPanel_->getLayerInspector().setLayer(freshLayer, layerScope);

        // Global effects live on the Composition, not a selected cell, so the two
        // re-points above don't reach them. Rebuild the composition inspector's
        // stack so a global effect add/remove/bypass undo/redo reflects too.
        inspectorPanel_->rebuildCompositionEffects();
    }
}

std::optional<MainComponent::CellEdit>
MainComponent::applyFileDrop(int layerIndex, int column, const juce::File& file)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return std::nullopt;

    // P24.5: Check content lock before replacing
    if (auto* existing = deck->getClip(layerIndex, column))
    {
        if (existing->contentLocked)
            return std::nullopt; // Silently refuse — locked content
    }

    // Capture before-state for undo (nullopt if the cell was empty).
    std::optional<Clip> before = snapshotCell(deck->getLayer(layerIndex), column);

    Clip clip;
    clip.name = file.getFileNameWithoutExtension().toStdString();
    clip.mediaFile = file;
    clip.id = s_nextClipId++;

    auto ext = file.getFileExtension().toLowerCase();
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
        ext == ".gif" || ext == ".bmp" || ext == ".tiff")
    {
        clip.mediaType = Clip::MediaType::Image;
        clip.playing = true;  // Images are always "playing" (static display)
    }
    else if (ext == ".mov" || ext == ".avi" || ext == ".mp4" ||
             ext == ".mkv" || ext == ".webm" || ext == ".m4v")
    {
        clip.mediaType = Clip::MediaType::Video;
        clip.playing = true;

        // Open the video file in the renderer's VideoPlayer
        auto& renderer = previewPanel_.getRenderer();
        if (renderer.openVideoForClip(clip.id, file))
        {
            auto* player = renderer.getVideoPlayer(clip.id);
            if (player)
            {
                clip.hasAlpha = player->hasAlpha();
                clip.clipWidth = player->getWidth();
                clip.clipHeight = player->getHeight();
                clip.thumbnail = player->getThumbnail(90, 72);
            }
        }
    }

    deck->setClip(layerIndex, column, clip);

    return CellEdit{ layerIndex, column, before, std::optional<Clip>(clip) };
}

void MainComponent::handleFileDrop(int layerIndex, int column, const juce::File& file)
{
    // GL fence (2026-07-28, round 3): applyFileDrop's internal deck->setClip
    // call can grow the layer's clips vector (Deck::setClip -> ensureColumns)
    // — the crash-proven reallocation class. One fence for this single-cell
    // drop gesture (mirrors every other single-cell drop handler, e.g.
    // onSourceActivated above). onMultiVideoDropped already fences its own
    // growth+placement loop around applyFileDrop, so applyFileDrop itself is
    // NOT fenced internally — that would nest under the loop's outer fence.
    std::optional<CellEdit> edit;
    undoService_.withDeckDetached([&] { edit = applyFileDrop(layerIndex, column, file); });
    if (edit)
    {
        pushClipEdits(composition_.activeDeckIndex, { *edit },
                      "Drop '" + juce::String(edit->after->name) + "'");
        if (deckView_)
            deckView_->rebuildGrid();
    }
}

std::optional<MainComponent::CellEdit>
MainComponent::applyMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return std::nullopt;

    // P24.5: Check content lock before replacing (mirrors applyFileDrop —
    // this check was missing here, letting a multi-image drop silently
    // overwrite a content-locked cell).
    if (auto* existing = deck->getClip(layerIndex, column))
    {
        if (existing->contentLocked)
            return std::nullopt; // Silently refuse — locked content
    }

    // Capture before-state for undo (nullopt if the cell was empty).
    std::optional<Clip> before = snapshotCell(deck->getLayer(layerIndex), column);

    Clip clip;
    clip.id = s_nextClipId++;
    clip.mediaType = Clip::MediaType::ImageSequence;
    clip.sequenceFiles = files;
    clip.sequenceFps = 2.5f;  // Default: 2.5 images per second
    clip.playing = true;

    // Sort and name from the first file
    std::sort(clip.sequenceFiles.begin(), clip.sequenceFiles.end(),
              [](const juce::File& a, const juce::File& b) {
                  return a.getFileName().compareNatural(b.getFileName()) < 0;
              });

    if (!clip.sequenceFiles.empty())
    {
        clip.name = clip.sequenceFiles[0].getParentDirectory().getFileName().toStdString()
                  + " (" + std::to_string(clip.sequenceFiles.size()) + " frames)";

        // Thumbnail from first image
        auto firstImg = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
        if (firstImg.isValid())
            clip.thumbnail = firstImg.rescaled(90, 72, juce::Graphics::lowResamplingQuality);
    }

    // Open the image sequence in the renderer
    auto& renderer = previewPanel_.getRenderer();
    renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);

    deck->setClip(layerIndex, column, clip);

    return CellEdit{ layerIndex, column, before, std::optional<Clip>(clip) };
}

void MainComponent::handleMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
    // Boris ruling 2026-08-04: exactly 2 images spread across 2 cells instead
    // of merging into one ImageSequence (the surprise that triggered this
    // fix — two dropped images must read as two clips, not one animation).
    // This handler is reached both by a direct Finder drop of images only
    // (no video — ClipCell::filesDropped) and by the internal "files:" drag
    // path when videos.empty() (ClipCell::itemDropped) — 3+ still falls
    // through to applyMultiFileDrop below, mirroring onMixedFilesDropped's
    // threshold so every drop path agrees.
    if (files.size() == 2)
    {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        const int numColsBefore = deck->numColumns;

        // GL fence (2026-07-28, round 3 class): column growth + setClip below
        // can reallocate every layer's clips vector — one fence for the whole
        // gesture, mirroring onMultiVideoDropped's shape.
        std::vector<CellEdit> edits;
        undoService_.withDeckDetached([&]
        {
            int needed = column + 2;
            while (deck->numColumns < needed)
            {
                deck->numColumns++;
                for (auto& layer : deck->layers)
                    layer.clips.resize(static_cast<size_t>(deck->numColumns));
            }
            for (int i = 0; i < 2; ++i)
                if (auto edit = applyFileDrop(layerIndex, column + i, files[static_cast<size_t>(i)]))
                    edits.push_back(*edit);
        });
        const int numColsAfter = deck->numColumns;

        if (edits.empty()) return;

        const juce::String desc = "Drop 2 Images";
        std::vector<std::unique_ptr<Command>> children;
        if (numColsAfter != numColsBefore)   // FIRST child → undoes LAST (restores count)
            children.push_back(std::make_unique<SetColumnCountCmd>(
                makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                numColsBefore, numColsAfter, "Resize Columns"));
        for (auto& edit : edits)
            children.push_back(makeSetClipCmd(composition_.activeDeckIndex, edit, desc));
        pushCommands(std::move(children), desc);

        if (deckView_)
            deckView_->rebuildGrid();
        return;
    }

    // GL fence (2026-07-28, round 3): setClip's internal ensureColumns can
    // grow the layer's clips vector — the crash-proven reallocation class.
    std::optional<CellEdit> edit;
    undoService_.withDeckDetached([&] { edit = applyMultiFileDrop(layerIndex, column, files); });
    if (!edit) return;

    pushClipEdits(composition_.activeDeckIndex, { *edit },
                  "Drop '" + juce::String(edit->after->name) + "'");

    if (deckView_)
        deckView_->rebuildGrid();
}

void MainComponent::handleDeckSwitch(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;

    // L5 Quantize fix: cancel any pending quantized trigger left waiting on the
    // deck we're LEAVING, via the shared DeckCommands.h helper (also used by
    // AddDeckCmd and appendDeckFromFile's deck-append — every deck-deactivation
    // path shares this one implementation now). Autopilot::processFrame only
    // drains the deck the renderer currently points at (Renderer.cpp's
    // `deckActive` gate), so a pending trigger on a deactivated deck freezes
    // rather than fires, then fires arbitrarily late whenever that deck is
    // reactivated and a beat next crosses — a cell lighting up nobody asked
    // for, mid-set. Lives here (not in the caller) so every switch path
    // inherits it — user tab click, REST, OSC, MIDI/controller SwitchDeck
    // bindings, and genre auto-switch alike; onDeckSwitched (the one path that
    // needs the cancelled list for undo) already called this same helper
    // itself, so this call is a harmless no-op for that path. Guarded on an
    // ACTUAL deck change: deckIndex == activeDeckIndex is a same-deck no-op
    // switch and must not disturb that deck's pending triggers.
    if (deckIndex != composition_.activeDeckIndex)
    {
        if (auto* leavingDeck = composition_.getActiveDeck())
            cancelPendingTriggers(*leavingDeck);
    }

    composition_.activeDeckIndex = deckIndex;

    // Update renderer's active deck pointer
    auto* deck = composition_.getActiveDeck();
    previewPanel_.getRenderer().setActiveDeck(deck);

    // setActiveDeck only repoints the compositor's deck pointer — the
    // renderer's fallback preview state (activeSourceType_ / loaded image)
    // is global and untouched by it, so an empty newly-active deck kept
    // showing the previous deck's clip (see refreshPreviewFromActiveClip's
    // ownership rule above: reconcile the fallback after any switch that can
    // change what the active deck actually has to show).
    if (deck)
        refreshPreviewFromActiveClip(*deck);

    if (deckView_)
        deckView_->rebuildGrid();
}

// === Menu Command Handler ===

void MainComponent::handleMenuCommand(int commandId)
{
    using C = AudioDNAMenuBar::CommandID;

    // Output fullscreen commands (dynamic range)
    if (commandId >= C::kOutputFullscreenBase && commandId < C::kOutputWindowed)
    {
        int displayIdx = commandId - C::kOutputFullscreenBase;
        openOutputOnDisplay(displayIdx);
        return;
    }

    switch (commandId)
    {
        // --- Audio-DNA menu ---
        case C::kPreferences:
            PreferencesDialog::show(this, tooltipsEnabled_,
                                    [this](bool enabled) { setTooltipsEnabled(enabled); },
                                    milkDropPresetDir_,
                                    [this](juce::String dir) { setMilkDropPresetDir(dir); },
                                    currentMidiOutputDeviceId(midiOutputHandler_),
                                    [this](juce::String id) { midiOutputHandler_.openDevice(id); });
            break;
        case C::kAbout:
            PreferencesDialog::show(this, tooltipsEnabled_,
                                    [this](bool enabled) { setTooltipsEnabled(enabled); },
                                    milkDropPresetDir_,
                                    [this](juce::String dir) { setMilkDropPresetDir(dir); },
                                    currentMidiOutputDeviceId(midiOutputHandler_),
                                    [this](juce::String id) { midiOutputHandler_.openDevice(id); });
            // TODO: auto-switch to About tab
            break;
        case C::kQuit:
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            break;
        case C::kImportISF:
            handleImportISF();
            break;

        // --- Composition menu ---
        case C::kCompUndo:
        {
            const bool movesLayer = undoManager_.undoAffectsLayerOrder();
            if (undoManager_.undo()) refreshAfterUndoRedo(movesLayer);
            break;
        }
        case C::kCompRedo:
        {
            const bool movesLayer = undoManager_.redoAffectsLayerOrder();
            if (undoManager_.redo()) refreshAfterUndoRedo(movesLayer);
            break;
        }
        case C::kCompNew:
            // GL fence (2026-07-28 fix round 1, reviewer-prescribed): initDefault()
            // does decks.clear()+push_back, reallocating composition_.decks under
            // an unlocked GL read — the same reallocation class one level up from
            // the Column->New crash. undoManager_.clear() only touches command
            // history (never the model), so it stays outside the fence; the UI
            // refresh calls read the model AFTER the fence has already restored
            // the renderer's active deck, so they are safe there too. Sequential,
            // not nested — the fence has already returned before either runs.
            // L3 (2026-09): routed through the shared swap helper so New also
            // closes orphaned media and re-points the inspectors.
            swapCompositionModel([this] { composition_.initDefault(); });
            break;
        case C::kCompOpen:
            openComposition();
            break;
        case C::kCompSave:
            saveComposition();
            break;
        case C::kCompSaveAs:
            saveCompositionAs();
            break;

        case C::kCompCollectMedia:
        {
            // P24.8: Collect all media files into a folder alongside the composition
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Collect Media — Choose Destination Folder",
                juce::File::getSpecialLocation(juce::File::userDesktopDirectory));
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    auto destDir = results.getFirst().getChildFile(
                        juce::String(composition_.name) + "_media");
                    destDir.createDirectory();
                    int copied = 0;
                    // Iterate all decks/layers/clips
                    for (auto& deck : composition_.decks)
                    {
                        for (int l = 0; l < deck.getNumLayers(); ++l)
                        {
                            auto* layer = deck.getLayer(l);
                            if (!layer) continue;
                            for (auto& clipOpt : layer->clips)
                            {
                                if (!clipOpt.has_value()) continue;
                                auto& clip = clipOpt.value();
                                if (clip.mediaFile != juce::File() && clip.mediaFile.existsAsFile())
                                {
                                    auto dest = destDir.getChildFile(clip.mediaFile.getFileName());
                                    if (!dest.existsAsFile())
                                    {
                                        clip.mediaFile.copyFileTo(dest);
                                        ++copied;
                                    }
                                    clip.mediaFile = dest; // Relink to collected copy
                                }
                                for (auto& sf : clip.sequenceFiles)
                                {
                                    if (sf.existsAsFile())
                                    {
                                        auto dest = destDir.getChildFile(sf.getFileName());
                                        if (!dest.existsAsFile())
                                            sf.copyFileTo(dest);
                                        sf = dest;
                                    }
                                }
                            }
                        }
                    }
                    // Also save composition JSON
                    auto compFile = destDir.getParentDirectory().getChildFile(
                        juce::String(composition_.name) + ".json");
                    composition_.saveToFile(compFile);
                    DBG("Collected " + juce::String(copied) + " media files to " + destDir.getFullPathName());
                });
            break;
        }

        case C::kCompRelocateFiles:
        {
            // P24.7: Relocate missing files — choose folder to search
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Choose Folder to Search for Missing Files",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory));
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    auto searchDir = results.getFirst();
                    int found = 0;
                    for (auto& deck : composition_.decks)
                    {
                        for (int l = 0; l < deck.getNumLayers(); ++l)
                        {
                            auto* layer = deck.getLayer(l);
                            if (!layer) continue;
                            for (auto& clipOpt : layer->clips)
                            {
                                if (!clipOpt.has_value()) continue;
                                auto& clip = clipOpt.value();
                                if (clip.mediaFile != juce::File() && !clip.mediaFile.existsAsFile())
                                {
                                    // Search for file by name in the search directory
                                    auto name = clip.mediaFile.getFileName();
                                    auto candidate = searchDir.getChildFile(name);
                                    if (candidate.existsAsFile())
                                    {
                                        clip.mediaFile = candidate;
                                        ++found;
                                    }
                                    else
                                    {
                                        // Deep search — check subdirectories
                                        auto matches = searchDir.findChildFiles(
                                            juce::File::findFiles, true, name);
                                        if (!matches.isEmpty())
                                        {
                                            clip.mediaFile = matches.getFirst();
                                            ++found;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (deckView_) deckView_->rebuildGrid();
                    DBG("Relocated " + juce::String(found) + " missing files");
                });
            break;
        }

        // --- Deck menu ---
        case C::kDeckNew:
        {
            // #21: command-owns-the-mutation (push_back is non-idempotent). The
            // fenced AddDeckCmd appends the deck, makes it active, and re-points
            // the renderer (via withDeckDetached's re-resolve) — perform() runs it.
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<AddDeckCmd>(
                makeCompositionResolver(), makeDeckFence(), "Add Deck"));
            pushCommands(std::move(children), "Add Deck");
            if (deckView_) deckView_->rebuildGrid();
            break;
        }
        case C::kDeckRemove:
            // #22: command-owns-the-mutation. Snapshot the full Deck VALUE + the
            // prior active index BEFORE building the command (it does not pre-erase
            // — the fenced execute() erases). Guard mirrors HEAD: keep >=1 deck.
            if (composition_.decks.size() > 1)
            {
                const int removeIdx = composition_.activeDeckIndex;
                Deck removedCopy = composition_.decks[static_cast<size_t>(removeIdx)];
                std::vector<std::unique_ptr<Command>> children;
                children.push_back(std::make_unique<RemoveDeckCmd>(
                    makeCompositionResolver(), makeDeckFence(),
                    makeClipMediaHook(), makeClipMediaDisposeHook(),
                    removeIdx, std::move(removedCopy), removeIdx, "Remove Deck"));
                pushCommands(std::move(children), "Remove Deck");
                if (deckView_) deckView_->rebuildGrid();
            }
            break;
        case C::kDeckClearClips:
            // Whole-deck clip clear = one composite of ClearLayerClipsCmd, one per
            // layer that actually has content (already-empty layers are skipped, so
            // clearing an empty deck pushes nothing).
            if (auto* deck = composition_.getActiveDeck())
            {
                // GL fence (2026-07-28): clips.clear() below is a full-vector
                // replace, the crash-proven reallocation class — ONE fence for
                // the whole gesture (every layer), not per layer.
                std::vector<std::unique_ptr<Command>> children;
                undoService_.withDeckDetached([&]
                {
                    for (int l = 0; l < deck->getNumLayers(); ++l)
                    {
                        auto* layer = deck->getLayer(l);
                        if (layer == nullptr) continue;
                        LayerClipsSnapshot before = captureLayerClips(*layer);
                        if (!layerClipsSnapshotHasContent(before)) continue;
                        layer->clips.clear();
                        layer->ensureColumns(deck->numColumns);
                        layer->clearActiveClip();
                        LayerClipsSnapshot after = captureLayerClips(*layer);
                        children.push_back(std::make_unique<ClearLayerClipsCmd>(
                            makeLayerResolver(), makeDeckFence(), makeClipMediaHook(),
                            makeClipMediaDisposeHook(),
                            composition_.activeDeckIndex, l,
                            std::move(before), std::move(after), "Clear Layer Clips"));
                    }
                });
                pushCommands(std::move(children), "Clear Deck Clips");
                // A1-companion fix (reviewer finding, 2026-07-30): clearActiveClip()
                // above never purged the renderer, so wiping the only active
                // shader/projectM clip via "Clear Deck Clips" reproduced the
                // stale-render symptom A1 fixed for the X-button clear. Same
                // ownership rule (see refreshPreviewFromActiveClip's doc comment).
                refreshPreviewFromActiveClip(*deck);
                if (deckView_) deckView_->rebuildGrid();
            }
            break;

        // --- Layer menu ---
        case C::kLayerNew:
        case C::kLayerInsertAbove:
        case C::kLayerInsertBelow:
            // #16: all three menu items append a layer via Deck::addLayer (no
            // insert-shift at HEAD). Command owns the fenced mutation (perform()
            // runs it) — addLayer is non-idempotent, so no mutate-then-push here.
            if (composition_.getActiveDeck())
            {
                std::vector<std::unique_ptr<Command>> children;
                children.push_back(std::make_unique<AddLayerCmd>(
                    makeDeckResolver(), makeDeckFence(),
                    composition_.activeDeckIndex, "Add Layer"));
                pushCommands(std::move(children), "Add Layer");
                if (deckView_) deckView_->rebuildGrid();
            }
            break;
        case C::kLayerRemove:
            // #17: removes the LAST layer (HEAD behavior). Snapshot the full Layer
            // BEFORE building the command; the command owns the fenced erase.
            if (auto* deck = composition_.getActiveDeck())
            {
                if (deck->getNumLayers() > 1)
                {
                    const int removeIdx = deck->getNumLayers() - 1;
                    Layer removed = *deck->getLayer(removeIdx);
                    std::vector<std::unique_ptr<Command>> children;
                    children.push_back(std::make_unique<RemoveLayerCmd>(
                        makeDeckResolver(), makeDeckFence(), makeClipMediaHook(),
                        makeClipMediaDisposeHook(),
                        composition_.activeDeckIndex, removeIdx,
                        std::move(removed), "Remove Layer"));
                    pushCommands(std::move(children), "Remove Layer");
                    if (deckView_) deckView_->rebuildGrid();
                }
            }
            break;
        case C::kLayerClearClips:
        {
            // BUG FIX 2026-07-17 (Wave 1-D): body was identical to kDeckClearClips
            // and wiped the ENTIRE deck. Clear only the SELECTED layer's clips.
            int selLayer = deckView_ ? deckView_->getSelectedLayerIndex() : -1;
            if (selLayer >= 0)
            {
                if (auto* deck = composition_.getActiveDeck())
                {
                    if (auto* layer = deck->getLayer(selLayer))
                    {
                        LayerClipsSnapshot before = captureLayerClips(*layer);
                        if (layerClipsSnapshotHasContent(before))  // skip a no-op clear
                        {
                            // GL fence (2026-07-28): clips.clear() is a full-
                            // vector replace, the crash-proven reallocation class.
                            undoService_.withDeckDetached([&]
                            {
                                layer->clips.clear();
                                layer->ensureColumns(deck->numColumns);
                                layer->clearActiveClip();
                            });
                            LayerClipsSnapshot after = captureLayerClips(*layer);
                            std::vector<std::unique_ptr<Command>> children;
                            children.push_back(std::make_unique<ClearLayerClipsCmd>(
                                makeLayerResolver(), makeDeckFence(), makeClipMediaHook(),
                                makeClipMediaDisposeHook(),
                                composition_.activeDeckIndex, selLayer,
                                std::move(before), std::move(after), "Clear Layer Clips"));
                            pushCommands(std::move(children), "Clear Layer Clips");
                            // A1-companion fix (reviewer finding, 2026-07-30): same gap
                            // as Clear Deck Clips above — clearActiveClip() never
                            // purged the renderer, so "Clear Layer Clips" on the only
                            // active shader/projectM clip reproduced A1's symptom.
                            refreshPreviewFromActiveClip(*deck);
                            if (deckView_) deckView_->rebuildGrid();
                        }
                    }
                }
            }
            break;
        }

        case C::kLayerFold:
        {
            // P24.12 / #20: Toggle fold on the selected layer. Field-level bool →
            // ToggleLayerFlagCmd, no fence. Mutate-then-push (toggle live, wrap).
            int selLayer = deckView_ ? deckView_->getSelectedLayerIndex() : -1;
            if (selLayer >= 0)
            {
                if (auto* deck = composition_.getActiveDeck())
                {
                    if (auto* layer = deck->getLayer(selLayer))
                    {
                        const bool before = layer->folded;
                        layer->folded = !layer->folded;
                        const juce::String desc = layer->folded ? "Fold Layer" : "Unfold Layer";
                        std::vector<std::unique_ptr<Command>> children;
                        children.push_back(std::make_unique<ToggleLayerFlagCmd>(
                            makeLayerResolver(), composition_.activeDeckIndex, selLayer,
                            ToggleLayerFlagCmd::Flag::Folded, before, layer->folded,
                            desc.toStdString()));
                        pushCommands(std::move(children), desc);
                        if (deckView_) deckView_->rebuildGrid();
                    }
                }
            }
            break;
        }
        case C::kLayerMoveUp:
        {
            // P24.13 / #18: Move selected layer up. Command owns the fenced move;
            // undo moves it back down (moveLayer(to,from) is the exact inverse).
            int selLayer = deckView_ ? deckView_->getSelectedLayerIndex() : -1;
            if (selLayer > 0)
            {
                if (composition_.getActiveDeck())
                {
                    std::vector<std::unique_ptr<Command>> children;
                    children.push_back(std::make_unique<MoveLayerCmd>(
                        makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                        selLayer, selLayer - 1, "Move Layer Up"));
                    pushCommands(std::move(children), "Move Layer Up");
                    // Reorder shifts layer indices with the layer count unchanged, so a
                    // multi-cell clip selection captured before the move now names the
                    // WRONG layer (same screen row, different underlying layer) — clear
                    // it here, right next to the rebuild, same as selectLayer already
                    // re-points the single-layer selection.
                    if (deckView_) { deckView_->rebuildGrid(); deckView_->selectLayer(selLayer - 1); deckView_->clearSelection(); }
                }
            }
            break;
        }
        case C::kLayerMoveDown:
        {
            // P24.13 / #18: Move selected layer down.
            int selLayer = deckView_ ? deckView_->getSelectedLayerIndex() : -1;
            if (auto* deck = composition_.getActiveDeck())
            {
                if (selLayer >= 0 && selLayer < deck->getNumLayers() - 1)
                {
                    std::vector<std::unique_ptr<Command>> children;
                    children.push_back(std::make_unique<MoveLayerCmd>(
                        makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                        selLayer, selLayer + 1, "Move Layer Down"));
                    pushCommands(std::move(children), "Move Layer Down");
                    // See kLayerMoveUp above: reorder shifts layer indices with the
                    // layer count unchanged, so a stale multi-cell clip selection would
                    // silently name the wrong layer post-move — clear it on rebuild.
                    if (deckView_) { deckView_->rebuildGrid(); deckView_->selectLayer(selLayer + 1); deckView_->clearSelection(); }
                }
            }
            break;
        }

        // --- Column menu ---
        case C::kColumnNew:
        case C::kColumnInsertBefore:
        case C::kColumnInsertAfter:
            // All three menu items append a column via Deck::addColumn (no
            // insert-shift at HEAD). One SetColumnCountCmd (N -> N+1); undo drops it.
            if (auto* deck = composition_.getActiveDeck())
            {
                const int before = deck->numColumns;
                // GL fence (2026-07-28): addColumn's ensureColumns growth is
                // the scout-diagnosed, disassembly-verified crash mechanism
                // (message-thread clips.resize under an unlocked GL read) —
                // see .harmony/notebook.md LAW entry.
                undoService_.withDeckDetached([deck] { deck->addColumn(); });
                std::vector<std::unique_ptr<Command>> children;
                children.push_back(std::make_unique<SetColumnCountCmd>(
                    makeDeckResolver(), makeDeckFence(), composition_.activeDeckIndex,
                    before, deck->numColumns, "Add Column"));
                pushCommands(std::move(children), "Add Column");
                if (deckView_) deckView_->rebuildGrid();
            }
            break;
        case C::kColumnRemove:
            if (auto* deck = composition_.getActiveDeck())
            {
                if (deck->numColumns > 1)
                {
                    // Snapshot the last column's cell per layer BEFORE removal so
                    // undo can re-insert them (removeColumn erases cells, not hides).
                    const int col = deck->numColumns - 1;
                    const int before = deck->numColumns;
                    std::vector<std::optional<Clip>> removed;
                    removed.reserve(deck->layers.size());
                    for (auto& layer : deck->layers)
                        removed.push_back(snapshotCell(&layer, col));
                    // GL fence (2026-07-28): removeColumn erases a cell from
                    // every layer's clips vector — same reallocation class.
                    undoService_.withDeckDetached([deck, col] { deck->removeColumn(col); });
                    std::vector<std::unique_ptr<Command>> children;
                    children.push_back(std::make_unique<RemoveColumnCmd>(
                        makeDeckResolver(), makeDeckFence(), makeClipMediaHook(),
                        makeClipMediaDisposeHook(),
                        composition_.activeDeckIndex, col, before,
                        std::move(removed), "Remove Column"));
                    pushCommands(std::move(children), "Remove Column");
                    if (deckView_) deckView_->rebuildGrid();
                }
            }
            break;

        // --- Clip menu ---
        case C::kClipClear:
            // Clear selected clips
            if (deckView_ && !deckView_->getSelectedCells().empty())
            {
                auto* deck = composition_.getActiveDeck();
                if (deck)
                {
                    int deckIdx = composition_.activeDeckIndex;
                    // A2 fix (2026-07-30): kClipClear used to overwrite each
                    // selected cell with a blank Clip{} (HEAD behavior) — still
                    // has_value(), so autopilot's occupancy scans accepted the
                    // blank cell and a cleared active cell left activeClipColumn
                    // dangling on it. clearCell() now vacates the cell to a
                    // GENUINE nullopt (edits record after=nullopt so undo/redo
                    // via SetClipCmd round-trips exact-restore <-> truly-empty).
                    // GL fence (2026-07-28 family): clearCell()'s cell.reset()
                    // destroys an occupied Clip's interior vectors (effects,
                    // etc.) in place — same crash-proven reallocation/UAF class
                    // as the deck/layer clears and SetClipCmd's own apply().
                    std::vector<CellEdit> edits;
                    std::vector<std::unique_ptr<Command>> runtimeChildren;
                    undoService_.withDeckDetached([&]
                    {
                        for (auto& cell : deckView_->getSelectedCells())
                        {
                            auto* layer = deck->getLayer(cell.layer);
                            std::optional<Clip> before = snapshotCell(layer, cell.column);
                            deck->clearCell(cell.layer, cell.column);
                            edits.push_back({ cell.layer, cell.column, before, std::nullopt });

                            // activeClipColumn must never dangle on a now-empty
                            // cell. Wrap the runtime reset as its own undo child
                            // (ClearActiveClipCmd) so undo restores the layer's
                            // active-cell pointer alongside the clip content.
                            if (layer != nullptr && layer->activeClipColumn == cell.column)
                            {
                                LayerRuntimeSnapshot rtBefore = captureLayerRuntime(*layer);
                                layer->clearActiveClip();
                                LayerRuntimeSnapshot rtAfter = captureLayerRuntime(*layer);
                                if (!(rtBefore == rtAfter))
                                    runtimeChildren.push_back(std::make_unique<ClearActiveClipCmd>(
                                        makeLayerResolver(), deckIdx, cell.layer,
                                        rtBefore, rtAfter, "Clear Clip"));
                            }
                        }
                    });
                    int count = static_cast<int>(edits.size());
                    juce::String desc = count > 1 ? "Clear " + juce::String(count) + " Clips"
                                                   : juce::String("Clear Clip");
                    std::vector<std::unique_ptr<Command>> children;
                    for (auto& edit : edits)
                        children.push_back(makeSetClipCmd(deckIdx, edit, desc));
                    for (auto& rc : runtimeChildren)
                        children.push_back(std::move(rc));
                    pushCommands(std::move(children), desc);
                    // A1-adjacent: clearing the ACTIVE cell can leave the
                    // renderer's global fallback state stale (see
                    // refreshPreviewFromActiveClip's ownership rule).
                    if (!runtimeChildren.empty())
                        refreshPreviewFromActiveClip(*deck);
                    deckView_->rebuildGrid();
                }
            }
            break;

        case C::kClipReplaceContent:
        {
            // P24.4: Replace content keeping effects — open file chooser
            if (deckView_ && !deckView_->getSelectedCells().empty())
            {
                auto cell = deckView_->getSelectedCells().front();
                fileChooser_ = std::make_unique<juce::FileChooser>(
                    "Replace Content",
                    juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                    "*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tiff;*.mov;*.avi;*.mp4;*.mkv;*.webm;*.m4v");
                fileChooser_->launchAsync(
                    juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                    [this, cell](const juce::FileChooser& fc)
                    {
                        auto results = fc.getResults();
                        if (results.isEmpty()) return;
                        auto file = results.getFirst();
                        auto* deck = composition_.getActiveDeck();
                        if (!deck) return;
                        auto* existing = deck->getClip(cell.layer, cell.column);
                        if (!existing) return;

                        // Capture before-state for undo.
                        std::optional<Clip> before = std::optional<Clip>(*existing);

                        // Build new content clip
                        Clip newContent;
                        newContent.name = file.getFileNameWithoutExtension().toStdString();
                        newContent.mediaFile = file;
                        auto ext = file.getFileExtension().toLowerCase();
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                            ext == ".gif" || ext == ".bmp" || ext == ".tiff")
                        {
                            newContent.mediaType = Clip::MediaType::Image;
                            newContent.playing = true;

                            // Media-leak fix (S166-LEAK, 2026-09): the outgoing
                            // clip id may currently hold a live VideoPlayer or
                            // ImageSequence entry (existing was Video/
                            // ImageSequence before this replace). The Video
                            // branch below retires it for free inside
                            // openVideoForClip(); this Image branch never
                            // called an open function at all, so the outgoing
                            // entry was never retired and leaked forever
                            // (decoder + map entry, both immortal). Route it
                            // through the same closeMediaForClip() retire
                            // mechanism L1-FU uses (GL-thread-deferred
                            // destroy) instead of inventing a second release
                            // path. Safe/no-op when existing->id has no media
                            // (Image->Image replace).
                            previewPanel_.getRenderer().closeMediaForClip(existing->id);
                        }
                        else
                        {
                            newContent.mediaType = Clip::MediaType::Video;
                            newContent.playing = true;
                            auto& renderer = previewPanel_.getRenderer();
                            if (renderer.openVideoForClip(existing->id, file))
                            {
                                if (auto* player = renderer.getVideoPlayer(existing->id))
                                {
                                    newContent.hasAlpha = player->hasAlpha();
                                    newContent.clipWidth = player->getWidth();
                                    newContent.clipHeight = player->getHeight();
                                    newContent.thumbnail = player->getThumbnail(90, 72);
                                }
                            }
                        }

                        // GL fence (2026-07-28, family-fence fix round 2, ruled
                        // exposed per the kClipClear precedent): replaceContent
                        // mutates `existing` in place — a Clip the GL thread may
                        // hold via getActiveClip() — and reassigns SEVERAL of its
                        // internal vectors (effects via move, sourceParams/
                        // sequenceFiles/presetPlaylist via copy) plus the
                        // thumbnail Image, all unsynchronized with the GL
                        // thread's read of those same fields (clip.effects
                        // iterated directly; the others read during compositing/
                        // playback). Same reallocation-on-a-possibly-active-clip
                        // class as kClipClear, just via one in-place call instead
                        // of a whole-Clip overwrite.
                        bool replaced = false;
                        undoService_.withDeckDetached([&] { replaced = existing->replaceContent(newContent); });
                        if (replaced)
                        {
                            // replaceContent mutated the clip in place (keeping
                            // effects/transport); record only if it actually
                            // changed (returns false when contentLocked).
                            pushClipEdits(composition_.activeDeckIndex,
                                          { { cell.layer, cell.column, before,
                                              std::optional<Clip>(*existing) } },
                                          "Replace Content");
                        }
                        if (deckView_) deckView_->rebuildGrid();
                        if (inspectorPanel_) inspectorPanel_->refresh();
                    });
            }
            break;
        }

        case C::kClipLockContent:
        {
            // P24.5: Toggle content lock on selected clip
            if (deckView_ && !deckView_->getSelectedCells().empty())
            {
                auto* deck = composition_.getActiveDeck();
                if (deck)
                {
                    int deckIdx = composition_.activeDeckIndex;
                    std::vector<std::unique_ptr<Command>> children;
                    for (auto& cell : deckView_->getSelectedCells())
                    {
                        if (auto* clip = deck->getClip(cell.layer, cell.column))
                        {
                            bool before = clip->contentLocked;
                            bool after = !before;
                            clip->contentLocked = after;
                            children.push_back(std::make_unique<ToggleClipLockCmd>(
                                makeLayerResolver(), deckIdx, cell.layer, cell.column,
                                before, after, after ? "Lock Content" : "Unlock Content"));
                        }
                    }
                    pushCommands(std::move(children), "Toggle Content Lock");
                    if (deckView_) deckView_->refresh();
                    if (inspectorPanel_) inspectorPanel_->refresh();
                }
            }
            break;
        }

        // --- Output menu ---
        case C::kOutputDisabled:
            closeOutput();
            break;
        case C::kOutputSnapshot:
        {
            // P22.7: Take a snapshot on a background thread (captureFrame blocks)
            auto& renderer = previewPanel_.getRenderer();
            std::thread([&renderer]() {
                renderer.takeSnapshot();
            }).detach();
            break;
        }
        case C::kOutputStartRecording:
        {
            if (!videoRecorder_.isRecording())
            {
                auto docsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                   .getChildFile("Audio-DNA").getChildFile("Recordings");
                docsDir.createDirectory();
                auto now = juce::Time::getCurrentTime();
                auto filename = "recording_" + now.formatted("%Y%m%d_%H%M%S") + ".mp4";
                auto outputFile = docsDir.getChildFile(filename);

                VideoRecorder::Config cfg;
                cfg.codec = VideoRecorder::Codec::H264;
                cfg.width = 1920;
                cfg.height = 1080;
                cfg.fps = 30;
                cfg.quality = 23;

                videoRecorder_.startRecording(outputFile, cfg);
            }
            break;
        }
        case C::kOutputStopRecording:
        {
            if (videoRecorder_.isRecording())
                videoRecorder_.stopRecording();
            break;
        }
        case C::kOutputSyphon:
        {
            // Toggle Syphon output publishing. The atomic flag is read each frame
            // by the GL thread (Renderer::renderOpenGL). Default OFF each boot.
            syphonOutput_.setEnabled(!syphonOutput_.isEnabled());
            break;
        }

        // --- Shortcuts menu ---
        case C::kShortcutsEditKeyboard:
            enterKeyboardBindingMode();
            break;
        case C::kShortcutsEditMIDI:
            enterMidiLearnMode();
            break;
        case C::kShortcutsStop:
            exitAllBindingModes();
            break;
        case C::kShortcutsExportBindings:
        {
            auto bindDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("Audio-DNA").getChildFile("bindings");
            bindDir.createDirectory();
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Export Bindings", bindDir, "*.json");
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    bindingManager_.saveToFile(results.getFirst().withFileExtension("json"));
                });
            break;
        }
        case C::kShortcutsImportBindings:
        {
            auto bindDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("Audio-DNA").getChildFile("bindings");
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Import Bindings", bindDir, "*.json");
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    bindingManager_.loadFromFile(results.getFirst());
                });
            break;
        }

        // --- View menu ---
        // P24.6: Layout presets
        case C::kViewSaveLayout:
        {
            auto layoutDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                 .getChildFile("Audio-DNA").getChildFile("layouts");
            layoutDir.createDirectory();
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Save Layout", layoutDir, "*.json");
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    auto file = results.getFirst().withFileExtension("json");
                    auto* obj = new juce::DynamicObject();
                    obj->setProperty("deckDividerY", deckDividerY_);
                    juce::Array<juce::var> vd;
                    for (int i = 0; i < 3; ++i)
                        vd.add(static_cast<double>(vDividerFrac_[i]));
                    obj->setProperty("vDividerFrac", vd);
                    file.replaceWithText(juce::JSON::toString(juce::var(obj)));
                });
            break;
        }
        case C::kViewLoadLayout:
        {
            auto layoutDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                                 .getChildFile("Audio-DNA").getChildFile("layouts");
            fileChooser_ = std::make_unique<juce::FileChooser>(
                "Load Layout", layoutDir, "*.json");
            fileChooser_->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this](const juce::FileChooser& fc) {
                    auto results = fc.getResults();
                    if (results.isEmpty()) return;
                    auto json = results.getFirst().loadFileAsString();
                    auto parsed = juce::JSON::parse(json);
                    if (auto* obj = parsed.getDynamicObject())
                    {
                        if (obj->hasProperty("deckDividerY"))
                            deckDividerY_ = static_cast<int>(obj->getProperty("deckDividerY"));
                        if (auto* vd = obj->getProperty("vDividerFrac").getArray())
                        {
                            for (int i = 0; i < std::min(3, static_cast<int>(vd->size())); ++i)
                                vDividerFrac_[i] = static_cast<float>(static_cast<double>((*vd)[i]));
                        }
                        resized();
                    }
                });
            break;
        }
        case C::kViewResetLayout:
        {
            deckDividerY_ = -1;
            vDividerFrac_[0] = 0.22f;
            vDividerFrac_[1] = 0.50f;
            vDividerFrac_[2] = 0.75f;
            resized();
            break;
        }

        default:
            DBG("Menu command not yet implemented: " + juce::String(commandId));
            break;
    }
}

// === Resizable divider mouse handling ===

void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    auto pos = event.position.toInt();

    // Horizontal divider
    if (dividerBounds_.contains(pos))
    {
        draggingDivider_ = true;
        return;
    }

    // Vertical dividers
    for (int i = 0; i < 3; ++i)
    {
        if (vDividerBounds_[i].contains(pos))
        {
            draggingVDivider_ = i;
            return;
        }
    }

    Component::mouseDown(event);
}

void MainComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (draggingDivider_)
    {
        auto area = getLocalBounds().reduced(4);
        int topOffset = area.getY();
        if (topBar_) topOffset += 34 + 1;
        if (signalBar_)
        {
            int sbh = signalBar_->getPreferredHeight();
            if (sbh > 0) topOffset += sbh + 1;
        }
        topOffset += 24 + 2; // row1

        int naturalDeckHeight = deckView_ ? deckView_->getNaturalHeight() : 200;
        int minY = topOffset + kMinDeckHeight;
        // Max = natural position (can drag up but not down past layers)
        int maxY = topOffset + naturalDeckHeight;

        deckDividerY_ = juce::jlimit(minY, maxY, event.position.roundToInt().y);
        resized();
        repaint();
        return;
    }

    if (draggingVDivider_ >= 0)
    {
        int mouseX = event.position.roundToInt().x;
        float frac = static_cast<float>(mouseX - bottomAreaX_)
                   / static_cast<float>(bottomAreaWidth_);

        // Clamp: each divider must stay between its neighbors with kMinPanelWidth gap
        float minFrac = static_cast<float>(kMinPanelWidth)
                      / static_cast<float>(bottomAreaWidth_);
        float maxFrac = 1.0f - minFrac;

        // Left neighbor
        float leftLimit = (draggingVDivider_ > 0)
            ? vDividerFrac_[draggingVDivider_ - 1] + minFrac
            : minFrac;

        // Right neighbor
        float rightLimit = (draggingVDivider_ < 2)
            ? vDividerFrac_[draggingVDivider_ + 1] - minFrac
            : maxFrac;

        vDividerFrac_[draggingVDivider_] = juce::jlimit(leftLimit, rightLimit, frac);
        resized();
        repaint();
        return;
    }

    Component::mouseDrag(event);
}

void MainComponent::mouseUp(const juce::MouseEvent& /*event*/)
{
    if (draggingDivider_)
    {
        draggingDivider_ = false;
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
    if (draggingVDivider_ >= 0)
    {
        draggingVDivider_ = -1;
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void MainComponent::mouseMove(const juce::MouseEvent& event)
{
    auto pos = event.position.toInt();

    bool prevHoverH = hoveringHDivider_;
    int prevHoverV = hoveringVDivider_;

    hoveringHDivider_ = false;
    hoveringVDivider_ = -1;

    if (dividerBounds_.contains(pos))
    {
        hoveringHDivider_ = true;
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            if (vDividerBounds_[i].contains(pos))
            {
                hoveringVDivider_ = i;
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
                break;
            }
        }
    }

    if (!hoveringHDivider_ && hoveringVDivider_ < 0
        && !draggingDivider_ && draggingVDivider_ < 0)
        setMouseCursor(juce::MouseCursor::NormalCursor);

    // Repaint if hover state changed
    if (hoveringHDivider_ != prevHoverH || hoveringVDivider_ != prevHoverV)
        repaint();
}

// === P9: Binding System Implementation ===

void MainComponent::enterKeyboardBindingMode()
{
    if (midiLearnOverlay_ && midiLearnOverlay_->isLearnModeActive())
        midiLearnOverlay_->exitLearnMode();

    if (bindingOverlay_)
    {
        if (bindingOverlay_->isBindingModeActive())
        {
            bindingOverlay_->exitBindingMode();
            return;
        }

        std::vector<BindingOverlay::BindableTarget> targets;
        buildBindableTargets(targets);
        bindingOverlay_->setBindableTargets(targets);
        bindingOverlay_->enterBindingMode();
    }
}

void MainComponent::enterMidiLearnMode()
{
    if (bindingOverlay_ && bindingOverlay_->isBindingModeActive())
        bindingOverlay_->exitBindingMode();

    if (midiLearnOverlay_)
    {
        if (midiLearnOverlay_->isLearnModeActive())
        {
            midiLearnOverlay_->exitLearnMode();
            return;
        }

        std::vector<BindingOverlay::BindableTarget> targets;
        buildBindableTargets(targets);
        midiLearnOverlay_->setBindableTargets(targets);
        midiLearnOverlay_->enterLearnMode(&audioEngine_.getDeviceManager());
    }
}

void MainComponent::exitAllBindingModes()
{
    if (bindingOverlay_ && bindingOverlay_->isBindingModeActive())
        bindingOverlay_->exitBindingMode();
    if (midiLearnOverlay_ && midiLearnOverlay_->isLearnModeActive())
        midiLearnOverlay_->exitLearnMode();
}

void MainComponent::buildBindableTargets(std::vector<BindingOverlay::BindableTarget>& targets)
{
    targets.clear();

    auto* deck = composition_.getActiveDeck();
    if (!deck) return;

    int numLayers = deck->getNumLayers();
    int numCols = deck->numColumns;

    // Global actions at the top
    int topY = 70; // Below the title text
    int gx = 20;
    int gw = 100;
    int gh = 30;
    int gap = 6;

    targets.push_back({ { gx, topY, gw, gh }, "Tap Tempo",
                         Binding::Action::TapTempo, 0, 0, 0, 0, 0 });
    gx += gw + gap;
    targets.push_back({ { gx, topY, gw, gh }, "Resync",
                         Binding::Action::Resync, 0, 0, 0, 0, 0 });
    gx += gw + gap;
    targets.push_back({ { gx, topY, gw, gh }, "Play / Pause",
                         Binding::Action::GlobalPlayPause, 0, 0, 0, 0, 0 });
    gx += gw + gap;
    targets.push_back({ { gx, topY, gw, gh }, "Stop",
                         Binding::Action::GlobalStop, 0, 0, 0, 0, 0 });
    gx += gw + gap;
    targets.push_back({ { gx, topY, gw, gh }, "Master Opacity",
                         Binding::Action::MasterOpacity, 0, 0, 0, 0, 0 });

    // Column triggers
    int colStartX = 240; // Approximate: after layer strip area
    int colY = topY + gh + 20;
    int colW = 80;
    int colH = 24;

    for (int c = 0; c < numCols && c < 20; ++c)
    {
        targets.push_back({ { colStartX + c * (colW + 2), colY, colW, colH },
                             "Column " + juce::String(c + 1),
                             Binding::Action::TriggerColumn,
                             0, c, 0, 0, 0 });
    }

    // Clip cells + layer controls
    int clipY = colY + colH + 10;
    int clipH = 50;
    int layerGap = 6;

    for (int l = numLayers - 1; l >= 0; --l) // Top layer = highest index (Resolume style)
    {
        int row = (numLayers - 1 - l);
        int y = clipY + row * (clipH + layerGap);

        // Layer controls on left
        int lx = 20;
        int lw = 32;
        int lh = clipH;

        targets.push_back({ { lx, y, lw, lh }, "L" + juce::String(l + 1) + " Bypass",
                             Binding::Action::ToggleLayerBypass, l, 0, 0, 0, 0 });
        lx += lw + 2;
        targets.push_back({ { lx, y, lw, lh }, "L" + juce::String(l + 1) + " Solo",
                             Binding::Action::ToggleLayerSolo, l, 0, 0, 0, 0 });
        lx += lw + 2;
        targets.push_back({ { lx, y, lw, lh }, "L" + juce::String(l + 1) + " Mute",
                             Binding::Action::ToggleLayerMute, l, 0, 0, 0, 0 });
        lx += lw + 2;
        targets.push_back({ { lx, y, lw, lh }, "L" + juce::String(l + 1) + " Auto",
                             Binding::Action::ToggleLayerAutopilot, l, 0, 0, 0, 0 });
        lx += lw + 2;
        targets.push_back({ { lx, y, lw, lh }, "L" + juce::String(l + 1) + " Visible",
                             Binding::Action::ToggleLayerVisible, l, 0, 0, 0, 0 });

        // Clip cells
        for (int c = 0; c < numCols && c < 20; ++c)
        {
            targets.push_back({ { colStartX + c * (colW + 2), y, colW, clipH },
                                 "L" + juce::String(l + 1) + " C" + juce::String(c + 1),
                                 Binding::Action::TriggerClip,
                                 l, c, 0, 0, 0 });
        }
    }

    // Deck switch targets at the bottom
    int deckY = clipY + numLayers * (clipH + layerGap) + 10;
    for (int d = 0; d < static_cast<int>(composition_.decks.size()) && d < 10; ++d)
    {
        targets.push_back({ { 20 + d * (colW + 2), deckY, colW, 28 },
                             "Deck " + juce::String(d + 1),
                             Binding::Action::SwitchDeck,
                             0, 0, d, 0, 0 });
    }
}

void MainComponent::handleBindingAction(const Binding& binding, float value)
{
    // === Resolve target layer/column based on targeting mode ===
    int resolvedLayer = binding.targetLayerIndex;
    int resolvedColumn = binding.targetColumn;

    if (binding.targetMode == Binding::TargetMode::Selected)
    {
        // Use whatever clip/layer is currently selected in the inspector
        if (auto* deck = composition_.getActiveDeck())
        {
            // Find the selected clip — use the first layer's active clip
            for (int li = 0; li < static_cast<int>(deck->layers.size()); ++li)
            {
                if (deck->layers[static_cast<size_t>(li)].activeClipColumn >= 0)
                {
                    resolvedLayer = li;
                    resolvedColumn = deck->layers[static_cast<size_t>(li)].activeClipColumn;
                    break;
                }
            }
        }
    }
    else if (binding.targetMode == Binding::TargetMode::ThisItem && binding.targetClipId > 0)
    {
        // Find the clip by ID across all layers/columns in the active deck
        if (auto* deck = composition_.getActiveDeck())
        {
            bool found = false;
            for (int li = 0; li < static_cast<int>(deck->layers.size()) && !found; ++li)
            {
                auto& layer = deck->layers[static_cast<size_t>(li)];
                for (int ci = 0; ci < static_cast<int>(layer.clips.size()) && !found; ++ci)
                {
                    if (layer.clips[static_cast<size_t>(ci)].has_value() &&
                        layer.clips[static_cast<size_t>(ci)]->id == binding.targetClipId)
                    {
                        resolvedLayer = li;
                        resolvedColumn = ci;
                        found = true;
                    }
                }
            }
        }
    }
    // ByPosition: use binding.targetLayerIndex / targetColumn directly (default)

    switch (binding.action)
    {
        case Binding::Action::TriggerClip:
        {
            if (value > 0.0f)
            {
                // Apply MIDI velocity to clip opacity if enabled
                if (binding.velocityToOpacity && binding.inputType == Binding::InputType::MidiNote)
                {
                    if (auto* deck = composition_.getActiveDeck())
                    {
                        auto* layer = deck->getLayer(resolvedLayer);
                        if (layer)
                        {
                            auto* clip = layer->getClipAt(resolvedColumn);
                            if (clip)
                                clip->clipOpacity = value; // velocity already normalized 0-1
                        }
                    }
                }
                handleClipTrigger(resolvedLayer, resolvedColumn);
            }
            else if (binding.triggerMode == Binding::TriggerMode::Momentary)
            {
                // Momentary release: clear the active clip on this layer
                if (auto* deck = composition_.getActiveDeck())
                {
                    auto* layer = deck->getLayer(resolvedLayer);
                    if (layer && layer->activeClipColumn == resolvedColumn)
                    {
                        layer->clearActiveClip();
                        previewPanel_.getRenderer().setActiveDeck(composition_.getActiveDeck());
                        if (deckView_) deckView_->refresh();
                    }
                }
            }
            break;
        }

        case Binding::Action::TriggerColumn:
        {
            if (value > 0.0f)
            {
                // Apply velocity to all clips in the column if enabled
                if (binding.velocityToOpacity && binding.inputType == Binding::InputType::MidiNote)
                {
                    if (auto* deck = composition_.getActiveDeck())
                    {
                        for (auto& layer : deck->layers)
                        {
                            auto* clip = layer.getClipAt(resolvedColumn);
                            if (clip)
                                clip->clipOpacity = value;
                        }
                    }
                }
                handleColumnTrigger(resolvedColumn);
            }
            else if (binding.triggerMode == Binding::TriggerMode::Momentary)
            {
                // Momentary release: clear all layers in this column
                if (auto* deck = composition_.getActiveDeck())
                {
                    for (auto& layer : deck->layers)
                    {
                        if (layer.activeClipColumn == resolvedColumn)
                            layer.clearActiveClip();
                    }
                    previewPanel_.getRenderer().setActiveDeck(composition_.getActiveDeck());
                    if (deckView_) deckView_->refresh();
                }
            }
            break;
        }

        case Binding::Action::ToggleLayerBypass:
        case Binding::Action::ToggleLayerSolo:
        case Binding::Action::ToggleLayerMute:
        case Binding::Action::ToggleLayerAutopilot:
        case Binding::Action::ToggleLayerVisible:
        {
            if (value > 0.0f)
            {
                auto* deck = composition_.getActiveDeck();
                if (deck)
                {
                    auto* layer = deck->getLayer(resolvedLayer);
                    if (layer)
                    {
                        switch (binding.action)
                        {
                            case Binding::Action::ToggleLayerBypass:
                                layer->bypassed = !layer->bypassed;
                                break;
                            case Binding::Action::ToggleLayerSolo:
                                layer->solo = !layer->solo;
                                break;
                            case Binding::Action::ToggleLayerMute:
                                layer->muted = !layer->muted;
                                break;
                            case Binding::Action::ToggleLayerAutopilot:
                                layer->autopilotEnabled = !layer->autopilotEnabled;
                                break;
                            case Binding::Action::ToggleLayerVisible:
                                layer->visible = !layer->visible;
                                break;
                            default:
                                break;
                        }
                        if (deckView_) deckView_->refresh();
                    }
                }
            }
            break;
        }

        case Binding::Action::SwitchDeck:
            if (value > 0.0f)
                handleDeckSwitch(binding.targetDeckIndex);
            break;

        case Binding::Action::TapTempo:
            if (value > 0.0f)
            {
                double now = juce::Time::getMillisecondCounterHiRes() / 1000.0;
                static std::array<double, 8> bindingTapTimes{};
                static int bindingTapCount = 0;
                static double bindingLastTapTime = 0.0;
                if (now - bindingLastTapTime > 2.0)
                    bindingTapCount = 0;
                if (bindingTapCount < static_cast<int>(bindingTapTimes.size()))
                    bindingTapTimes[static_cast<size_t>(bindingTapCount)] = now;
                ++bindingTapCount;
                bindingLastTapTime = now;
                if (bindingTapCount >= 2)
                {
                    int n = std::min(bindingTapCount, static_cast<int>(bindingTapTimes.size()));
                    double total = bindingTapTimes[static_cast<size_t>(n - 1)] - bindingTapTimes[0];
                    if (total > 0.0)
                    {
                        float tappedBPM = static_cast<float>(60.0 / (total / (n - 1)));
                        if (auto* tracker = analysisThread_.getBpmTracker())
                            tracker->setManualBPM(tappedBPM);
                    }
                }
            }
            break;

        case Binding::Action::Resync:
            if (value > 0.0f)
            {
                beatCounter_ = 0;
                lastBeatPhase_ = 0.0f;
                if (auto* tracker = analysisThread_.getBpmTracker())
                {
                    tracker->resetBeatPhase();
                    tracker->resetPhrase();
                }
            }
            break;

        case Binding::Action::GlobalPlayPause:
            if (value > 0.0f)
            {
                if (audioEngine_.isPlaying())
                    audioEngine_.stop();
                else
                    audioEngine_.play();
            }
            break;

        case Binding::Action::GlobalStop:
            if (value > 0.0f)
                audioEngine_.stop();
            break;

        case Binding::Action::MasterOpacity:
            composition_.masterOpacity = value;
            break;

        case Binding::Action::AdjustLayerOpacity:
        {
            if (auto* deck = composition_.getActiveDeck())
            {
                auto* layer = deck->getLayer(resolvedLayer);
                if (layer)
                    layer->opacity = value;
            }
            break;
        }

        case Binding::Action::LayerTransport:
        {
            if (value > 0.0f)
            {
                if (auto* deck = composition_.getActiveDeck())
                {
                    auto* layer = deck->getLayer(resolvedLayer);
                    if (layer)
                    {
                        auto* clip = layer->getActiveClip();
                        if (clip)
                        {
                            clip->playing = !clip->playing;
                            if (deckView_) deckView_->refresh();
                        }
                    }
                }
            }
            break;
        }

        case Binding::Action::ToggleEffectBypass:
        {
            if (value > 0.0f)
            {
                if (auto* deck = composition_.getActiveDeck())
                {
                    auto* layer = deck->getLayer(resolvedLayer);
                    if (layer)
                    {
                        auto* clip = layer->getActiveClip();
                        if (clip && binding.targetEffectIndex >= 0 &&
                            binding.targetEffectIndex < static_cast<int>(clip->effects.size()))
                        {
                            auto& fx = clip->effects[static_cast<size_t>(binding.targetEffectIndex)];
                            fx.bypassed = !fx.bypassed;
                        }
                    }
                }
            }
            break;
        }

        case Binding::Action::AdjustMacro:
        {
            // CC value → macro knob
            if (binding.targetMacroIndex >= 0 && binding.targetMacroIndex < MacroBank::kNumMacros)
                globalMacroBank_.getMacro(binding.targetMacroIndex).manualValue = value;
            break;
        }

        case Binding::Action::Snapshot:
        {
            if (value > 0.0f)
            {
                auto& renderer = previewPanel_.getRenderer();
                std::thread([&renderer]() {
                    renderer.takeSnapshot();
                }).detach();
            }
            break;
        }

        case Binding::Action::ToggleRecording:
        {
            if (value > 0.0f)
            {
                if (videoRecorder_.isRecording())
                {
                    videoRecorder_.stopRecording();
                }
                else
                {
                    auto docsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                       .getChildFile("Audio-DNA").getChildFile("Recordings");
                    docsDir.createDirectory();
                    auto now = juce::Time::getCurrentTime();
                    auto filename = "recording_" + now.formatted("%Y%m%d_%H%M%S") + ".mp4";
                    VideoRecorder::Config cfg;
                    cfg.codec = VideoRecorder::Codec::H264;
                    cfg.width = 1920; cfg.height = 1080; cfg.fps = 30; cfg.quality = 23;
                    videoRecorder_.startRecording(docsDir.getChildFile(filename), cfg);
                }
            }
            break;
        }
    }
}
