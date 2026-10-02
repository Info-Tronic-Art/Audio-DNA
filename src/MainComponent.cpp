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
#include "recording/PerfStateCapture.h"
#include "recording/RoutineSlice.h"
#include "model/AppSettings.h"
#include "ui/UiPaintCounters.h"
#include <algorithm>

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

    // s-rta-0927 outputs-c3 (plan5 C3): the machine's settings.json -- read and written ONLY through AppSettings
    // (read-modify-write: "milkDropPresetDir" and "outputs" never clobber each other). TEST-ONLY override: in a
    // test-server build (AUDIODNA_TEST_SERVER) running --test-mode, an absolute path in AUDIODNA_SETTINGS_FILE
    // replaces it, and without one a scratch file does (AppSettings::testModeFile) -- a test-mode app never reads or
    // writes the user's real settings file. Anywhere else the variable is ignored.
    juce::File appSettingsFile(bool testMode)
    {
#if AUDIODNA_TEST_SERVER
        if (testMode)
        {
            const auto path = juce::SystemStats::getEnvironmentVariable("AUDIODNA_SETTINGS_FILE", {});
            const auto file = AppSettings::testModeFile(path);
            if (juce::File::isAbsolutePath(path))
                std::cerr << "[Settings] test mode: AUDIODNA_SETTINGS_FILE = " << path << std::endl;
            else
                std::cerr << "[Settings] test mode: AUDIODNA_SETTINGS_FILE unset or not absolute -- scratch settings "
                          << file.getFullPathName() << " (never the real settings.json)" << std::endl;
            return file;
        }
#else
        juce::ignoreUnused(testMode);
#endif
        return AppSettings::defaultFile();
    }

    // s-rta-0923 lane 3 (plan section 3.4): ControlPath builders for
    // manualWrite's 11 external writer sites (plan section 3.6). POSITIONAL
    // fields only, per ControlPath's own contract (D2's name resolution is
    // Program::compile's job, not this lane's) — but the names ARE filled
    // from the live model at call time so a take recorded through this
    // funnel is re-bindable later (D2).
    ControlPath compScalarPath(const juce::String& key)
    {
        ControlPath p;
        p.scope = ControlPath::Scope::Comp;
        p.control = "scalar";
        p.scalar = key.toStdString();
        return p;
    }

    ControlPath layerScalarPath(const Composition& comp, int deckIdx, int layerIdx, const juce::String& key)
    {
        ControlPath p;
        p.scope = ControlPath::Scope::Layer;
        p.deck = deckIdx;
        p.layer = layerIdx;
        p.control = "scalar";
        p.scalar = key.toStdString();
        if (deckIdx >= 0 && deckIdx < static_cast<int>(comp.decks.size()))
        {
            auto& deck = comp.decks[static_cast<size_t>(deckIdx)];
            p.deckName = deck.name;
            if (layerIdx >= 0 && layerIdx < static_cast<int>(deck.layers.size()))
            {
                auto& layer = deck.layers[static_cast<size_t>(layerIdx)];
                p.layerId = layer.id;
                p.layerName = layer.name;
            }
        }
        return p;
    }

    ControlPath clipScalarPath(const Composition& comp, int deckIdx, int layerIdx, int col, const juce::String& key)
    {
        ControlPath p = layerScalarPath(comp, deckIdx, layerIdx, key);
        p.scope = ControlPath::Scope::Clip;
        p.col = col;
        if (deckIdx >= 0 && deckIdx < static_cast<int>(comp.decks.size()))
        {
            auto& deck = comp.decks[static_cast<size_t>(deckIdx)];
            if (layerIdx >= 0 && layerIdx < static_cast<int>(deck.layers.size()))
            {
                auto& layer = deck.layers[static_cast<size_t>(layerIdx)];
                if (col >= 0 && col < static_cast<int>(layer.clips.size()) && layer.clips[static_cast<size_t>(col)].has_value())
                    p.clipName = layer.clips[static_cast<size_t>(col)]->name;
            }
        }
        return p;
    }

    // paramKey is passed in (not derived here) — the only caller (ApiServer's
    // set_param clip branch, plan section 3.6 site #10) already resolved it
    // from EffectLibrary while matching the param by name; this function has
    // no EffectLibrary access of its own and must not re-derive a lossy
    // stand-in (an index string) for the D2 name-resolution field.
    ControlPath clipParamPath(const Composition& comp, int deckIdx, int layerIdx, int col, int fxIdx,
                              int paramIdx, const juce::String& paramKey)
    {
        ControlPath p = clipScalarPath(comp, deckIdx, layerIdx, col, "");
        p.control = "param";
        p.scalar.clear();
        p.fx = fxIdx;
        p.param = paramIdx;
        p.paramKey = paramKey.toStdString();
        if (deckIdx >= 0 && deckIdx < static_cast<int>(comp.decks.size()))
        {
            auto& deck = comp.decks[static_cast<size_t>(deckIdx)];
            if (layerIdx >= 0 && layerIdx < static_cast<int>(deck.layers.size()))
            {
                auto& layer = deck.layers[static_cast<size_t>(layerIdx)];
                if (col >= 0 && col < static_cast<int>(layer.clips.size()) && layer.clips[static_cast<size_t>(col)].has_value())
                {
                    auto& clip = *layer.clips[static_cast<size_t>(col)];
                    if (fxIdx >= 0 && fxIdx < static_cast<int>(clip.effects.size()))
                        p.fxName = clip.effects[static_cast<size_t>(fxIdx)].effectName;
                }
            }
        }
        return p;
    }

    ControlPath macroPath(int i)
    {
        ControlPath p;
        p.scope = ControlPath::Scope::Macro;
        p.control = "macro";
        p.macroScope = 0;
        p.macro = i;
        return p;
    }

    // s167 D6a: OSC/MIDI/REST writes are all treated as the human hand
    // (Decaying rank — no release event) at the funnel; only widget drags
    // (Lane C4, gripHeld()/release() directly on the bound ParamConnection)
    // are Held.
    Hand handFor(Origin o, ParamConnection::Grip::Kind k)
    {
        if (o == Origin::Human) return k == ParamConnection::Grip::Kind::Held ? Hand::HumanHeld : Hand::HumanDecaying;
        return Hand::Lane;   // Replay, Routine, Preamble, Engine
    }

    // s-rta-0926 (routines-1a carried concern (a)): a trigger that switches a layer to a clip that
    // was never triggered and is paused auto-plays it when it lands (Layer::triggerClipImmediate's
    // first-activation rule -- at once, or at the quantized drain). Read BEFORE the trigger.
    bool triggerWillAutoPlay(Layer& layer, int column)
    {
        const Clip* clip = layer.getClipAt(column);
        return clip != nullptr && column != layer.runtime().activeClipColumn && !clip->hasBeenTriggered && !clip->playing;
    }
}

MainComponent::MainComponent(bool testMode, int testPort)
    : testMode_(testMode), testPort_(testPort)
{
    setLookAndFeel(&lookAndFeel_);
    // decks-followup ITEM 1: also the JUCE-wide default, so a top-level window that never inherits
    // MainComponent's LookAndFeel (an AlertWindow shown with no associated component, or a PopupMenu
    // with no in-app parent) draws with it too, instead of JUCE's stock rounded LookAndFeel_V4.
    // Cleared in ~MainComponent() before lookAndFeel_ is destroyed (see AudioDNALookAndFeel::installAsDefault).
    lookAndFeel_.installAsDefault();

    // UI components
    addAndMakeVisible(openImageButton_);
    addAndMakeVisible(fileLabel_);
    addAndMakeVisible(waveformDisplay_);
    addAndMakeVisible(audioReadoutPanel_);
    addAndMakeVisible(spectrumDisplay_);
    addAndMakeVisible(previewPanel_);

    fileLabel_.setColour(juce::Label::textColourId,
                         juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    setFileLabel("No file loaded");

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
        beatCrossings_.reset();
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
            setFileLabel("Mic: " + audioEngine_.getDeviceStatus());
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
                        setFileLabel(file.getFileName());
                        applyAudioTransport("play", Origin::Human);
                    }
                });
            }
            else
            {
                setFileLabel(currentAudioFile_.getFileName());
                applyAudioTransport("play", Origin::Human);
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
  #if AUDIODNA_HAS_CAMERA
    setupLabel(cameraLabel_,     "Camera");
  #endif
    setupLabel(audioSourceLabel_, "Audio Source");
    setupLabel(inputGainLabel_,   "Gain");
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
                    setFileLabel("Slot " + juce::String(capturedSlot + 1) + ": "
                                      + s.loadedFile.getFileNameWithoutExtension()
                                      + (stats.dropped > 0 ? " (check mappings)" : ""));
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

    // s-rta-0926b plan4 S5: the per-deck "Viewport" resolution lock (hidden since v2) is retired --
    // the canvas is Composition::outputWidth x outputHeight (Composition inspector). The renderer's
    // lock survives as the TEST-ONLY canvas override (TestServer render_frame width/height).

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

    audioEngine_.onError = [this](const juce::String& msg) {
        juce::MessageManager::callAsync([this, msg] {
            setFileLabel(msg);
        });
    };

    // s-rta-0929b btguard (BG6): no allowed input / no allowed device is a persistent notice, not a file-label write
    // (every other label write would hide it). Plain words, never a modal.
    addChildComponent(audioDeviceNotice_);
    audioDeviceNotice_.setColour(juce::Label::textColourId, juce::Colour(AudioDNALookAndFeel::kMeterYellow));
    audioDeviceNotice_.setJustificationType(juce::Justification::centredRight);
    audioEngine_.onDeviceStateChanged = [this] { refreshAudioDeviceNotice(true); };
    refreshAudioDeviceNotice(false);
    if (audioEngine_.hasAudioDevice())
    {
        // R13: the analysis thread resamples the device stream to its fixed
        // internal rate (AnalysisThread::kSampleRate, 48 kHz) via
        // AnalysisResampler, so a non-48 kHz device is no longer a warning
        // case (nor a modal) -- just an informational log line.
        const double actualSr = audioEngine_.getCurrentSampleRate();
        const int expectedSr = AnalysisThread::kSampleRate;
        if (actualSr > 0.0 && static_cast<int>(actualSr) != expectedSr)
            std::cerr << "[Audio] device " << static_cast<int>(actualSr)
                      << " Hz -> analysis resamples to " << expectedSr << " Hz" << std::endl;
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
            setFileLabel("Mic: " + audioEngine_.getDeviceStatus());
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
                        setFileLabel(file.getFileName());
                        applyAudioTransport("play", Origin::Human);
                    }
                });
            }
            else
            {
                setFileLabel(currentAudioFile_.getFileName());
                applyAudioTransport("play", Origin::Human);
            }
        }
    };

    // Wire TopBar gain slider
    topBar_->getInputGainSlider().onValueChange = [this] {
        audioEngine_.setInputGain(static_cast<float>(topBar_->getInputGainSlider().getValue()));
    };

    // plan5 C2: the TopBar "Outputs" button opens the output item list -- the SAME list as the Output menu
    // (OutputManager::populateMenu); a pick goes through handleMenuCommand like a menu-bar pick.
    topBar_->getOutputsButton().onClick = [this] {
        juce::PopupMenu m;
        outputs_.populateMenu(m);
        m.showMenuAsync(juce::PopupMenu::Options()
                            .withParentComponent(getTopLevelComponent())
                            .withTargetComponent(&topBar_->getOutputsButton()),
                        [this](int id) { if (id != 0) handleMenuCommand(id); });
    };

    // s-rta-0923/0924 step 3 (plan section 3.3 B3): all tempo writers now go
    // through applyTempoCommand -- a MOVE of each site's body, not a change.
    topBar_->onTapTempo = [this](float tappedBPM) {
        applyTempoCommand("tap", tappedBPM, Origin::Human);
    };

    // L7-JUKE: Ableton Link toggle. linkSync_'s consumer loop (below, in the
    // main timer callback) is already live and gated on isEnabled() — this
    // was the only missing piece.
    topBar_->onLinkToggled = [this](bool enabled) { linkSync_.setEnabled(enabled); };

    topBar_->onManualBpmChanged = [this](bool manual, float bpm) {
        applyTempoCommand(manual ? "manual" : "auto", bpm, Origin::Human);
    };

    topBar_->onResync = [this] {
        applyTempoCommand("resync", 0.0f, Origin::Human);
    };

    // Global transport (TopBar Play/Pause/Stop). There is no single global
    // transport flag in the model; per-layer transport (LayerStrip) drives each
    // layer's active clip. The honest global mapping is therefore: apply
    // play/pause/stop to every layer's active clip on the ACTIVE deck. Stop =
    // pause + rewind to the clip's in-point (distinct from Pause, which holds).
    // s-rta-0923/0924 step 3 (plan section 3.3 B3): routed through
    // applyClipPlaying, one point per layer, sharing a `group` id.
    topBar_->onPlay = [this] {
        if (auto* deck = composition_.getActiveDeck())
        {
            const uint64_t group = recorderHost_.nextGroupId();
            for (int l = 0; l < deck->getNumLayers(); ++l)
                if (auto* layer = deck->getLayer(l))
                    if (const int col = layer->runtime().activeClipColumn; layer->getClipAt(col))
                        applyClipPlaying(l, col, "play", Origin::Human, group);
        }
    };
    topBar_->onPause = [this] {
        if (auto* deck = composition_.getActiveDeck())
        {
            const uint64_t group = recorderHost_.nextGroupId();
            for (int l = 0; l < deck->getNumLayers(); ++l)
                if (auto* layer = deck->getLayer(l))
                    if (const int col = layer->runtime().activeClipColumn; layer->getClipAt(col))
                        applyClipPlaying(l, col, "pause", Origin::Human, group);
        }
    };
    topBar_->onStop = [this] {
        // s-rta-0926b (Boris 2026-09-26, "ok we can keep stop for routines only"): Stop stops every
        // running and waiting routine, letting go of every control they hold -- and nothing else: no
        // clip is stopped, paused or rewound. The GlobalStop binding does the same.
        routineEngine_.stopAll();
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

    // s-rta-0928b mediaopen: file presence, off the GL thread and off every paint (MediaPresence). A flag that flipped
    // repaints the grid's "!" indicators.
    presence_.onChanged = [this] { if (deckView_) deckView_->refresh(); };
    presence_.start(composition_);

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
            // s-rta-0923/0924 step 3 (critic N2/A4): this is an automatic
            // (non-user) deck switch -- record it as Origin::Engine, not the
            // default Human.
            handleDeckSwitch(deckIdx, Origin::Engine);
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
                setFileLabel(clip->mediaFile.getFileName());
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                setFileLabel(juce::String(clip->sourceType));
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
        routineEngine_.stopOnLayer(composition_.activeDeckIndex, layerIdx);   // s-rta-0927: X clears the layer of routines too (not undoable, like Stop)
        const LayerRuntimeTransition t = layer->clearActiveClip();   // the exact before / after pair
        if (t.changed())
        {
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<ClearActiveClipCmd>(
                makeLayerResolver(), composition_.activeDeckIndex, layerIdx,
                t.before, t.after, "Clear Layer Clip"));
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
        // s-rta-0928b mediaopen: every video is opened BEFORE the fence (prepareFileDrop); the fence holds only the
        // column growth and the setClips.
        std::vector<PreparedDrop> prepared;
        for (int i = 0; i < static_cast<int>(files.size()); ++i)
            if (auto p = prepareFileDrop(layerIdx, col + i, files[static_cast<size_t>(i)]))
                prepared.push_back(std::move(*p));
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
            for (const auto& p : prepared)
                if (auto edit = commitDrop(p))
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
    // type through its existing single-type primitive (image(s) → prepareFileDrop
    // or prepareMultiFileDrop for one cell; videos → prepareFileDrop per sequential
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

        // s-rta-0928b mediaopen: every file is prepared (videos opened, the sequence opened) BEFORE the fence, in the
        // order the fenced loop used to open them (images, then videos); the fence holds only growth + setClips.
        std::vector<PreparedDrop> prepared;
        if (images.size() == 1)
        {
            if (auto p = prepareFileDrop(layerIdx, col, images[0]))
                prepared.push_back(std::move(*p));
        }
        else if (images.size() == 2)
        {
            // Spread: same primitive as the video-spread pattern above
            // (prepareFileDrop per cell), not prepareMultiFileDrop — two
            // images must land as two separate clips.
            for (int i = 0; i < 2; ++i)
                if (auto p = prepareFileDrop(layerIdx, col + i, images[static_cast<size_t>(i)]))
                    prepared.push_back(std::move(*p));
        }
        else if (images.size() > 2)
        {
            if (auto p = prepareMultiFileDrop(layerIdx, col, images))
                prepared.push_back(std::move(*p));
        }
        for (int i = 0; i < static_cast<int>(videos.size()); ++i)
            if (auto p = prepareFileDrop(layerIdx, videoStartCol + i, videos[static_cast<size_t>(i)]))
                prepared.push_back(std::move(*p));

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
            for (const auto& p : prepared)
                if (auto edit = commitDrop(p))
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
                            slot.addParam(p.defaultValue);
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
                            slot.addParam(p.defaultValue);
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
                    slot.addParam(p.defaultValue);

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

    // plan6 §6.4: the deck tab row -- "+" (New Deck / Load Deck..., deckIndex -1) and a tab's right-click menu.
    deckView_->onDeckAction = [this](int deckIndex, DeckTabRow::Action action) {
        switch (action)
        {
            case DeckTabRow::Action::NewDeck:    newDeck();                break;
            case DeckTabRow::Action::LoadDeck:   loadDeck();               break;
            case DeckTabRow::Action::SaveDeck:   saveDeck(deckIndex);      break;
            case DeckTabRow::Action::SaveDeckAs: saveDeckAs(deckIndex);    break;
            case DeckTabRow::Action::Rename:     renameDeck(deckIndex);    break;
            case DeckTabRow::Action::Duplicate:  duplicateDeck(deckIndex); break;
            case DeckTabRow::Action::Remove:     removeDeck(deckIndex);    break;
        }
    };
    // The 10-s "Undo Remove" button is bound to THAT removal: if anything else is on top of the undo stack the click
    // is a no-op and the button just hides (pushCommands hides it on any later command anyway).
    deckView_->onUndoHint = [this] {
        if (undoManager_.undoDescription() == "Remove Deck")
            handleMenuCommand(AudioDNAMenuBar::kCompUndo);
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
    // s-rta-0925 mastersignal Step 0: fired by ConnectionEngine::tick (120Hz,
    // message thread) right after a clip's sourceParams live twins publish --
    // keeps the standalone-source path's COPY (Renderer::activeSourceParams_,
    // taken at select time) current for connected source params even when no
    // Inspector tab is open to drive onSourceParamsChanged above.
    connectionEngine_.onSourceParamsPublished = [this](const Clip* clip) {
        if (clip)
            previewPanel_.getRenderer().updateActiveSourceParamsFor(clip->sourceType, clip->sourceParams);
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
    // plan6 §7: a library row is ONE click away from replacing everything on screen -- confirm first.
    browserPanel_->getCompDecksBrowser().onCompositionLoad = [this](const juce::File& f) {
        confirmReplaceShow("Open Composition",
                           "Open \"" + f.getFileNameWithoutExtension() + "\" and replace \""
                               + juce::String(composition_.name) + "\"?",
                           "Open", [this, f] { loadComposition(f); });
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
        setFileLabel(file.getFileName());
    };
    // s-rta-0924b step 4 (Lane S4-B): the Record tab drives the recorder through
    // the SAME perf* funnel the /api/perf/* endpoints use -- it never touches
    // recorderHost_ itself.
    {
        auto& rp = browserPanel_->getRecordPanel();
        rp.setTakesRoot(takesRoot());
        rp.onStatus   = [this] { return recorderHost_.status(); };
        rp.onRecord = [this](const RecordPanel::RecordRequest& r) {
            ApiServer::PerfRecordOpts o;
            o.name = r.name;
            o.audio = r.audio;
            if (r.overdub)
                o.overdubAssetId = juce::String(recorderHost_.status().loadedAssetId);
            return perfRecord(o);
        };
        rp.onStop     = [this] { return perfStop(); };
        rp.onPlay     = [this](bool withAudio) { return perfPlay(withAudio); };
        rp.onStopPlay = [this] { return perfStopPlay(); };
        rp.onLoad     = [this](const juce::File& f) { return perfLoad(f); };
        rp.onRepair   = [this] { return perfRepair(); };

        // Save Routine (s-rta-0926 lane 3): the SAME funnel /api/routine/save uses.
        rp.onSaveRoutine  = [this](const juce::String& name, int fromBar, int toBar) {
            ApiServer::RoutineSaveOpts o;
            o.name = name;
            o.useBars = true;
            o.fromBar = fromBar;
            o.toBar = toBar;
            return perfRoutineSave(o);
        };
    }
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
        setFileLabel(juce::String(sourceId));

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

                // Extract display name from path
                juce::File presetFile(path);
                setFileLabel("MilkDrop: " + presetFile.getFileNameWithoutExtension());
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
    // plan5 C2: the Output menu's display items + "All Outputs Off" are OutputManager's item list; a change in the
    // live outputs relabels the TopBar button and rebuilds the native menu (the ticks).
    menuBarModel_->populateOutputItems = [this](juce::PopupMenu& m) { outputs_.populateMenu(m); };
    // plan5 C3: the saved output set is LOADED here (settings.json "outputs") -- it opens nothing: the app never
    // opens an output at launch or on a composition/deck load; only Output > Restore Last Outputs does (Q1).
    outputs_.attachSettings(appSettingsFile(testMode_));
    outputs_.onLiveCountChanged = [this](int liveCount) {
        if (topBar_) topBar_->setLiveOutputCount(liveCount);
        if (menuBarModel_) menuBarModel_->menuItemsChanged();
    };
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
        testServer_->setOutputsStateProvider([this] { return outputs_.stateVar(); });   // plan5 C2, before start()
        testServer_->setMediaStateProvider([this] { return mediaStateVar(); });   // s-rta-0928b mediaopen, before start()
        testServer_->setLoadWitnessProvider([this] { return loadWitnessVar(); });   // s-rta-0929 asyncload, before start()
        // plan5 C3 (s-rta-0927 outputs-c3), test mode only: the manager's counters, "Restore Last Outputs" with
        // nothing to restore, the poll A/B. The restore hook re-checks ON the message thread and runs the menu
        // action's own handler only while nothing is restorable -- it can never open a window.
        testServer_->setOutputsTestHooks({
            [this] { return outputs_.statsVar(); },
            [this] {
                juce::MessageManager::callAsync([safe = juce::Component::SafePointer<MainComponent>(this)] {
                    if (safe != nullptr && safe->outputs_.restorableCount() == 0)
                        safe->handleMenuCommand(AudioDNAMenuBar::CommandID::kOutputRestoreLast);
                });
            },
            [this](bool enabled) { outputs_.setPollEnabled(enabled); } });
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
    apiServer_->onLoadComposition = [this](juce::File f, std::shared_ptr<LoadTicket> t) { loadComposition(f, std::move(t)); };
    apiServer_->onSnapshot = [this]() {
        auto& renderer = previewPanel_.getRenderer();
        std::thread([&renderer]() { renderer.takeSnapshot(); }).detach();
    };
    apiServer_->onSetBpm = [this](float bpm) {
        // Same path as the TopBar manual-BPM toggle+edit (manual override).
        applyTempoCommand("link", bpm, Origin::Human);
    };
    apiServer_->onResync = [this] {
        // s-rta-0925: same path as the TopBar Resync button.
        applyTempoCommand("resync", 0.0f, Origin::Human);
    };
    // s-rta-0923 lane 3 (plan section 3.6, site #9): the inline
    // `lay->opacity = opacity;` write was removed from
    // ApiServer::handleSetLayerOpacity; this callback is now the only place
    // that write happens, routed through manualWrite.
    apiServer_->onSetLayerOpacity = [this](int layerIdx, float opacity) {
        manualWrite(layerScalarPath(composition_, composition_.activeDeckIndex, layerIdx, "opacity"),
                   opacity, GripKind::Decaying, Origin::Human);
    };
    // s-rta-0925 mastersignal Step 1: same shape as onSetLayerOpacity above.
    apiServer_->onSetMasterSignal = [this](float depth) {
        manualWrite(compScalarPath("signal"), depth, GripKind::Decaying, Origin::Human);
    };
    // s-rta-0923 lane 3 (plan section 3.6, site #10): the inline
    // `fx.paramValues[pi] = value;` write was removed from
    // ApiServer::handleSetParam's clip branch; this callback is now the only
    // place that write happens, routed through manualWrite.
    // s-rta-0926b plan-fitmode: already on the message thread (ApiServer marshals it).
    apiServer_->onSetClipFitMode = [this](int layerIdx, int column, int mode) {
        setClipFitMode(layerIdx, column, mode);
    };
    apiServer_->onSetClipEffectParam = [this](int layerIdx, int column, int fxIndex, int paramIndex,
                                              const std::string& paramName, float value) {
        manualWrite(clipParamPath(composition_, composition_.activeDeckIndex, layerIdx, column, fxIndex,
                                  paramIndex, juce::String(paramName)),
                   value, GripKind::Decaying, Origin::Human);
    };

    // === s-rta-0923/0924 step 3 (Lane S3-B, plan section 3.3 B4/B5) ===
    // RecorderHost::Dispatch -- discrete replay re-dispatches through the
    // SAME choke points the human path uses (Origin::Replay).
    recorderHost_.dispatch.fire = [this](const Fired& f) -> bool {
        const auto& control = f.key.control;
        // s-rta-0925 (D4 preamble): a checkpoint-0 restore entry (Program::compile's buildPreamble)
        // carries Origin::Preamble on the point itself, but every handler below is still called with
        // the literal Origin::Replay -- so capture()'s `origin != Origin::Replay` gate and undo's
        // `origin == Origin::Human` gate stay exactly as they are; nothing from a preamble is ever
        // recorded or undoable, by construction, with no new gate to add anywhere. `immediate` is
        // the ONE thing that differs: it bypasses handleClipTrigger's beat-snap/quantize queue,
        // because a restore is not a performance trigger.
        const bool immediate = (f.p.origin == Origin::Preamble);
        if (control == "activeClip")
        {
            if (f.target.layer < 0) return false;
            if (f.p.v < 0)
                applyClearActiveClip(f.target.layer, Origin::Replay, f.target.deck);
            else
                handleClipTrigger(f.target.layer, f.p.v, Origin::Replay, f.target.deck, immediate);
            return true;
        }
        if (control == "activeDeck")
        {
            handleDeckSwitch(f.p.v, Origin::Replay);
            return true;
        }
        if (control == "tempo")
        {
            applyTempoCommand(f.p.action, static_cast<float>(f.p.v) / 100.0f, Origin::Replay);
            return true;
        }
        if (control == "audio")
        {
            applyAudioTransport(f.p.action, Origin::Replay);
            return true;
        }
        if (f.key.scope == ControlPath::Scope::Layer &&
            (control == "visible" || control == "solo" || control == "mute" ||
             control == "bypass" || control == "autopilot"))
        {
            if (f.target.layer < 0) return false;
            applyLayerFlag(f.target.layer, control, f.p.v != 0, Origin::Replay, f.target.deck);
            return true;
        }
        if (control == "bypass" && f.key.scope == ControlPath::Scope::Clip && f.target.fx >= 0)
        {
            if (f.target.layer < 0 || f.target.col < 0) return false;
            applyEffectBypass(f.target.layer, f.target.col, f.target.fx, f.p.v != 0, Origin::Replay, f.target.deck);
            return true;
        }
        if (control == "playing")
        {
            if (f.target.layer < 0 || f.target.col < 0) return false;
            applyClipPlaying(f.target.layer, f.target.col, f.p.action, Origin::Replay, 0, f.target.deck);
            return true;
        }
        if (control == "quantize")
        {
            composition_.quantizeMode = static_cast<Composition::QuantizeMode>(f.p.v);
            return true;
        }
        return false;
    };

    // Continuous replay (critic A1): hooks the connection lane's shipped
    // manualTouch/manualWrite/manualRelease funnel (R8) -- Origin::Replay
    // means handFor() gives it Hand::Lane, so a human hand always wins (D8),
    // and the capture hooks below never record these writes back (D6).
    recorderHost_.dispatch.continuous.touch   = [this](const ControlPath& k, const std::string& /*grip*/) {
        return manualTouch(k, GripKind::Held, Origin::Replay); };
    recorderHost_.dispatch.continuous.set     = [this](const ControlPath& k, float v) {
        return manualWrite(k, v, GripKind::Held, Origin::Replay); };
    recorderHost_.dispatch.continuous.release = [this](const ControlPath& k) { manualRelease(k, Origin::Replay); };

    recorderHost_.dispatch.capturePerfState = [this]() {
        return capturePerfState(composition_, analysisThread_.getFeatureBus().read().bpm, lastAudioAction_);
    };
    recorderHost_.dispatch.notify = [this](const std::string& msg) {
        std::cerr << "[Recorder] " << msg << std::endl;
        // s-rta-0924b S4-B: the Record panel shows it as its notice line (stored
        // only; applied at the panel's 4 Hz refresh). Message thread only (G10).
        // s-rta-0925: setNotice also stores the situation it was raised in: status() is read AFTER the event
        // (the host publishes synchronously at every transition, and the funnel notifies after the host call
        // returns).
        if (browserPanel_)
            browserPanel_->getRecordPanel().setNotice(msg, recorderHost_.status());
    };
    // s-rta-0925 end-of-replay (Boris ruling 2026-09-25 "hold, don't stop"): fired at most once per
    // play(), as tick()'s LAST statement, after publishStatus() -- status().finished is already true.
    recorderHost_.dispatch.replayFinished = [this] { onReplayFinished(); };

    // s-rta-0926 routines slice 1 (plan 4.1): the routine engine drives the SAME seams a take replay
    // does -- copies of the recorder's four dispatch lambdas (Origin::Replay: never captured into a
    // take, never pushed onto undo, Hand::Lane so a human hand always wins, D8) plus its own notice
    // line. Re-entrancy (plan 4.2): none of these lambdas names routineEngine_.
    routineEngine_.dispatch.fire    = recorderHost_.dispatch.fire;
    routineEngine_.dispatch.touch   = recorderHost_.dispatch.continuous.touch;
    routineEngine_.dispatch.set     = recorderHost_.dispatch.continuous.set;
    routineEngine_.dispatch.release = recorderHost_.dispatch.continuous.release;
    // s-rta-0926b plan3 C: a restore glide's "from" -- the NORMALISED value the control shows now,
    // resolved exactly as manualWrite resolves it (message thread, called from the engine's tick).
    routineEngine_.dispatch.read    = [this](const ControlPath& k) -> std::optional<float> {
        auto ref = resolveControl(composition_, globalMacroBank_, k);
        if (!ref || !ref->manual) return std::nullopt;
        // The twin is in the manual field's units (ConnectionEngine publishes toModel(y) for scalars);
        // `live` may be null (macros).
        const float model = ref->live ? ref->live->effective(ref->manual.load()) : ref->manual.load();
        return ref->toNorm ? ref->toNorm(model) : model;
    };
    routineEngine_.dispatch.notify  = [this](const std::string& msg) {
        std::cerr << "[Routine] " << msg << std::endl;
        if (browserPanel_)
            browserPanel_->getRecordPanel().setNotice(msg, recorderHost_.status());
    };

    // s-rta-0927 routine display (plan-routine-display-A.md 2.1-2.3): the deck's ROUTINES row and the strips'
    // bands run through the SAME perfRoutine* funnel /api/routine/*, OSC and bindings use.
    deckView_->onRoutineFired   = [this](int slot) { perfRoutineFire(slot); };
    deckView_->onRoutineRemoved = [this](int slot) { perfRoutineStop(slot, false); };
    deckView_->onRoutineSet     = [this](int slot, const RoutineSettingsChange& c) {
        ApiServer::RoutineSetOpts o;
        o.slot = slot;
        o.loop = c.loop;
        o.restoreState = c.restoreState;
        o.restoreStyle = c.restoreStyle;
        o.quantize = c.quantize;
        perfRoutineSet(o);
    };
    deckView_->onRoutineRename  = [this](int slot) { renameRoutine(slot); };
    deckView_->onRoutineDeleted = [this](int slot) { deleteRoutine(slot); };

    // Continuous capture: the recorder HOOKS the funnel's own accept/refuse
    // notification (critic A1/A2/N12) -- Human writes only; Replay writes are
    // filtered here too (handFor already routed them to Hand::Lane above).
    onManualWrite = [this](const ControlPath& k, float v, GripKind g, Origin o, bool ok) {
        if (o != Origin::Human) return;
        if (ok) recorderHost_.onHumanWrite(k, v, g == GripKind::Held ? "held" : "decaying");
        else    recorderHost_.noteHumanRefused(k);
    };
    onManualTouch = [this](const ControlPath& k, GripKind g, Origin o, bool ok) {
        if (o == Origin::Human && ok) recorderHost_.onHumanTouch(k, g == GripKind::Held ? "held" : "decaying");
    };
    onManualRelease = [this](const ControlPath& k, Origin o) {
        if (o == Origin::Human) recorderHost_.onHumanRelease(k);
    };

    // REST /api/perf/* (Lane S3-C's callbacks, wired here per critic A5).
    // s-rta-0924b S4-B: every body now lives in the perf* funnel the Record
    // panel shares (MainComponent.h); ApiServer's void callbacks discard the
    // returned refusal text (it has already gone through dispatch.notify).
    apiServer_->onPerfRecord   = [this](const ApiServer::PerfRecordOpts& opts) { perfRecord(opts); };
    apiServer_->onPerfStop     = [this] { perfStop(); };
    apiServer_->onPerfLoad     = [this](juce::File takeFolder) { perfLoad(takeFolder); };
    apiServer_->onPerfPlay     = [this](bool withAudio) { perfPlay(withAudio); };
    apiServer_->onPerfStopPlay = [this] { perfStopPlay(); };
    apiServer_->onPerfRepair   = [this] { perfRepair(); };
    // s-rta-0925 (probe enabler, plan section 5): a dev/probe control for the live gate -- puts the
    // app on the live input or (if loaded) the file transport, the same switch setAudioSourceModeSynced
    // performs for the human/Stop Playback paths.
    apiServer_->onAudioSource  = [this](const juce::String& mode) {
        setAudioSourceModeSynced(mode == "file" ? AudioEngine::SourceMode::File : AudioEngine::SourceMode::MicInput); };
    // Synchronous (critic A5(b)/N3): reads ONLY recorderHost_.status()'s
    // mutex-guarded copy -- never audioEngine_.getCurrentSampleRate()/
    // getCurrentAudioDevice() on the HTTP thread.
    apiServer_->onPerfStatus   = [this]() -> juce::var { return perfStatusVar(); };

    // s-rta-0926 routines slice 1 (plan 5.1): /api/routine/* -- the perfRoutine* funnel (shared with
    // OSC and bindings); refusals land in /api/routine/status lastError. Status is synchronous and
    // reads ONLY routineEngine_.status()'s mutex-guarded copy.
    apiServer_->onRoutineSave   = [this](const ApiServer::RoutineSaveOpts& o) { perfRoutineSave(o); };
    apiServer_->onRoutineFire   = [this](int slot) { perfRoutineFire(slot); };
    apiServer_->onRoutineStop   = [this](int slot, bool all) { perfRoutineStop(slot, all); };
    apiServer_->onRoutineSet    = [this](const ApiServer::RoutineSetOpts& o) { perfRoutineSet(o); };
    apiServer_->onRoutineRemove = [this](int slot) { perfRoutineRemove(slot); };
    apiServer_->onRoutineStatus = [this]() -> juce::var { return routineStatusVar(); };

#if AUDIODNA_TEST_SERVER
    // R4: test-mode inject_features on the production port relays through
    // the TestServer-held Writer (the only writer in test mode).
    if (testMode_ && testServer_)
        apiServer_->onInjectFeatures = [this](const FeatureSnapshot& snap,
                                              const InjectedOnsetCount& onsetIntent) {
            testServer_->injectSnapshot(snap, onsetIntent);
        };
#endif
    apiServer_->setOutputsStateProvider([this] { return outputs_.stateVar(); });   // plan5 C2, before start()
    apiServer_->setMediaStateProvider([this] { return mediaStateVar(); });   // s-rta-0928b mediaopen, before start()
    apiServer_->setLoadWitnessProvider([this] { return loadWitnessVar(); });   // s-rta-0929 asyncload, before start()
    // s-rta-0929 asyncload: the TEST-ONLY load routes' targets (the routes exist only in a TEST_SERVER build).
    apiServer_->onDebugLoadDeck = [this](juce::File f) { appendDeckFromFile(f); };
    apiServer_->onDebugDuplicateDeck = [this](int deckIndex) { duplicateDeck(deckIndex); };
    apiServer_->onDebugCancelLoad = [this] { cancelStagedOpen(LoadTicket::Outcome::Superseded); };
    apiServer_->onDebugUiText = [this] { return fileLabel_.getText(); };
    apiServer_->onDebugAudioNotice = [this] {   // s-rta-0929b btguard
        return audioDeviceNotice_.isVisible() ? audioDeviceNotice_.getText() : juce::String();
    };
#if AUDIODNA_TEST_SERVER
    apiServer_->setAudioDevicesProvider([this] { return audioEngine_.deviceStatusVar(); });   // s-rta-0929b btguard, before start()
#endif
    // s-rta-0928b mediaopen: the TEST-ONLY drop route's target (the route exists only in a TEST_SERVER build).
    apiServer_->onDebugDropFiles = [this](int layer, int column, const std::vector<juce::File>& files) {
        debugDropFiles(layer, column, files);
    };
#if AUDIODNA_TEST_SERVER
    // s-rta-0928b idlepaint (TEST-ONLY routes): an in-peer overlay parented to the top-level window at MainComponent
    // point (x, y) -- a 3-item PopupMenu (the mandated withParentComponent pattern) or a plain test panel (a background
    // app's PopupMenu is dismissed within ~50 ms); a whole-MainComponent repaint.
    apiServer_->onDebugUiTestMenu = [this](bool on, int x, int y, const juce::String& kind) {
        if (!on)
        {
            juce::PopupMenu::dismissAllActiveMenus();
            debugOverlayPanel_.reset();
            return;
        }
        if (kind == "panel")
        {
            struct Panel final : juce::Component
            {
                void paint(juce::Graphics& g) override
                {
                    g.fillAll(juce::Colour(0xff7a3a8a));
                    g.setColour(juce::Colours::white);
                    g.drawRect(getLocalBounds(), 2);
                    g.setFont(juce::Font(juce::FontOptions(14.0f)));
                    g.drawText("probe-idle-paint overlay", getLocalBounds(), juce::Justification::centred, false);
                }
            };
            auto* top = getTopLevelComponent();
            debugOverlayPanel_ = std::make_unique<Panel>();
            debugOverlayPanel_->setBounds(juce::Rectangle<int>(300, 140).withPosition(top->getLocalPoint(this, juce::Point<int>(x, y))));
            top->addAndMakeVisible(*debugOverlayPanel_);
            return;
        }
        juce::PopupMenu m;
        m.addItem(1, "Test item one");
        m.addItem(2, "Test item two");
        m.addItem(3, "Test item three");
        m.showMenuAsync(juce::PopupMenu::Options().withParentComponent(getTopLevelComponent())
                            .withTargetScreenArea(juce::Rectangle<int>(1, 1).withPosition(localPointToGlobal(juce::Point<int>(x, y)))),
                        [](int) {});
    };
    apiServer_->onDebugUiRepaintAll = [this] { repaint(); };
#endif
    apiServer_->start();

    // P22.9: Set up OSC handler callbacks, then start listening (below).
    // OscHandler uses MessageLoopCallback, so these fire on the message thread;
    // the callAsync wrappers below match the existing trigger/deck callbacks.
    oscHandler_.onTriggerClip = [this](int layer, int column) {
        juce::MessageManager::callAsync([this, layer, column]() { handleClipTrigger(layer, column); });
    };
    // s-rta-0926b plan-fitmode: MessageLoopCallback -> already on the message thread.
    oscHandler_.onSetClipFitMode = [this](int layerIdx, int column, int mode) {
        setClipFitMode(layerIdx, column, mode);
    };
    oscHandler_.onSwitchDeck = [this](int deckIdx) {
        juce::MessageManager::callAsync([this, deckIdx]() { handleDeckSwitch(deckIdx); });
    };
    oscHandler_.onSetMaster = [this](float level) {
        // s-rta-0923 lane 3 (plan section 3.6, site #1): routed through the
        // manualWrite funnel (Decaying rank — OSC has no release event).
        manualWrite(compScalarPath("opacity"), level, GripKind::Decaying, Origin::Human);
    };
    oscHandler_.onSetMasterSignal = [this](float depth) {
        // s-rta-0925 mastersignal Step 1: same funnel shape as onSetMaster.
        manualWrite(compScalarPath("signal"), depth, GripKind::Decaying, Origin::Human);
    };
    oscHandler_.onSetLayerOpacity = [this](int layerIdx, float opacity) {
        // s-rta-0923 lane 3 (plan section 3.6, site #2).
        manualWrite(layerScalarPath(composition_, composition_.activeDeckIndex, layerIdx, "opacity"),
                   opacity, GripKind::Decaying, Origin::Human);
    };
    // s-rta-0923/0924 step 3 (plan section 3.3 B3): routed through applyLayerFlag.
    oscHandler_.onSetLayerBypass = [this](int layerIdx, bool bypass) {
        applyLayerFlag(layerIdx, "bypass", bypass, Origin::Human);
    };
    oscHandler_.onSetLayerSolo = [this](int layerIdx, bool solo) {
        applyLayerFlag(layerIdx, "solo", solo, Origin::Human);
    };
    oscHandler_.onSetLayerMute = [this](int layerIdx, bool mute) {
        applyLayerFlag(layerIdx, "mute", mute, Origin::Human);
    };
    oscHandler_.onSetBpm = [this](float bpm) {
        // Same manual-override path as apiServer_->onSetBpm / the TopBar manual-BPM toggle.
        applyTempoCommand("link", bpm, Origin::Human);
    };
    oscHandler_.onResync = [this] {
        // s-rta-0925: same path as apiServer_->onResync / the TopBar Resync button.
        applyTempoCommand("resync", 0.0f, Origin::Human);
    };
    oscHandler_.onSetMacro = [this](int macroIdx, float value) {
        // s-rta-0923 lane 3 (plan section 3.6, site #3). Same path as the
        // AdjustMacro MIDI binding (global dashboard-link bank), now routed
        // through manualWrite.
        if (macroIdx >= 0 && macroIdx < MacroBank::kNumMacros)
            manualWrite(macroPath(macroIdx), value, GripKind::Decaying, Origin::Human);
    };
    oscHandler_.onTriggerRoutine = [this](int slot) {
        // s-rta-0926 routines slice 1: same funnel as /api/routine/fire and the TriggerRoutine
        // binding; callAsync like onTriggerClip, so a fire never runs inside the engine's tick.
        juce::MessageManager::callAsync([this, slot]() { perfRoutineFire(slot); });
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

    // s-rta-0928b idlepaint (Pitfall NN): the two always-animating panels draw in their own CoreGraphics layers (JUCE's
    // mac peer repaints the UNION of every dirty rect, so their 30 Hz repaints at opposite window edges repainted the
    // whole window). An in-peer overlay that crosses one (a parented PopupMenu, the tooltip, a ClipCell drag image,
    // the binding overlays) hands it back to JUCE painting while it is up. After every addAndMakeVisible (the baseline).
    overlayWatch_ = std::make_unique<OverlayWatch>(*this);
    overlayWatch_->addOverlay(bindingOverlay_.get());
    overlayWatch_->addOverlay(midiLearnOverlay_.get());
    signalBar_->setOpaque(true);   // its paint() fills every pixel (SignalBar::paint): same pixels, an opaque layer
    waveformLayer_  = NativeLayerHost::attach(waveformDisplay_, *overlayWatch_, uipaint::Waveform);
    signalBarLayer_ = NativeLayerHost::attach(*signalBar_, *overlayWatch_, uipaint::SignalBar);
#if AUDIODNA_TEST_SERVER
    apiServer_->onDebugUiNativeFallback = [this](bool on) {
        for (auto* host : { waveformLayer_.get(), signalBarLayer_.get() })
            if (host != nullptr)
                host->setForcedFallback(on);
    };
#endif
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
    return AppSettings(appSettingsFile(testMode_)).read(AppSettings::kMilkDropPresetDir).toString();
}

void MainComponent::saveMilkDropPresetDirSetting(const juce::String& dir) const
{
    // plan5 C3: read-modify-write -- the "outputs" key (OutputManager) survives this write, and vice versa.
    AppSettings(appSettingsFile(testMode_)).update(AppSettings::kMilkDropPresetDir, dir);
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

    // s-rta-0929 asyncload (AL6): AFTER the HTTP servers stop (ApiServer::stop() already finished every load ticket
    // Cancelled -- the server's sweep wins at quit, so this finish is a no-op compare-exchange), BEFORE the renderer
    // detaches: the staged load's adopted players go to the retire list detach() drains, queued opens are dropped and an
    // in-flight one lands on nobody (mediaOpener_'s destructor, member order, waits <= 5 s for it or leaks its pool).
    cancelStagedOpen(LoadTicket::Outcome::Cancelled);

    // s-rta-0923/0924 step 3 (critic A4/N9): flush/finalize/save any
    // in-progress take (or stop playback) BEFORE the audio device closes.
    // Placed immediately after apiServer_->stop() (not first): the shipped
    // "HTTP servers stop first" invariant stays as-is (post-quit callAsync
    // is refused anyway -- ApiServer.cpp), and shutdown() still needs a live
    // audioEngine_ (getAudioTap()) and composition_, both intact here.
    // s-rta-0926 routines: stop every routine first (its grips released while the model is live).
    routineEngine_.stopAll();
    recorderHost_.shutdown(composition_, audioEngine_.getAudioTap());
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

    // W3 (outputwindow-arc, scout R5): the output GL contexts obey the same
    // shutdown law — end their GL activity HERE, before any teardown below.
    // (s-rta-0927 outputs-c2: OutputManager::shutdown() detaches and destroys
    // every output window synchronously; idempotent, its destructor calls it
    // again. The outputs read only the shared IOSurface frames, which live
    // until the Renderer is destroyed -- no order against the main detach
    // above matters.)
    outputs_.shutdown();

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
    if (!testMode_)
        analysisThread_.stopThread(1000);
    lookAndFeel_.uninstallAsDefault();   // before lookAndFeel_'s own destruction below (teardown-order assert)
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    uipaint::counters().mainComponentPaints.fetch_add(1, std::memory_order_relaxed);   // s-rta-0928b idlepaint witness
#if AUDIODNA_TEST_SERVER
    uipaint::passBegin();   // s-rta-0929 g4cpu: the pass's JUCE paint time starts here (ends in paintOverChildren)
#endif
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
        recordUiGeometry();
        return;
    }

    // === Hide v1 controls that are now in TopBar or removed ===
    audioSourceLabel_.setVisible(false);
    audioSourceSelector_.setVisible(false);
    inputGainLabel_.setVisible(false);
    inputGainSlider_.setVisible(false);
    fpsLabel_.setVisible(false);
    cpuLabel_.setVisible(false);
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
    if (audioDeviceNotice_.isVisible())   // s-rta-0929b btguard (BG6): the notice keeps its whole sentence
    {
        const int w = juce::GlyphArrangement::getStringWidthInt(audioDeviceNotice_.getFont(), audioDeviceNotice_.getText()) + 12;
        audioDeviceNotice_.setBounds(row1.removeFromRight(juce::jmin(w, row1.getWidth() / 2)));
    }
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
    recordUiGeometry();
}

void MainComponent::recordUiGeometry()
{
#if AUDIODNA_TEST_SERVER
    // s-rta-0928b idlepaint (TEST-ONLY): where the panels are, for probe-idle-paint's window-capture mapping.
    auto& c = uipaint::counters();
    auto put = [](std::atomic<int> (&r)[4], juce::Rectangle<int> b) {
        r[0] = b.getX(); r[1] = b.getY(); r[2] = b.getWidth(); r[3] = b.getHeight();
    };
    c.mainW = getWidth();
    c.mainH = getHeight();
    put(c.signalBarRect, signalBar_ != nullptr ? signalBar_->getBounds() : juce::Rectangle<int>());
    put(c.waveformRect, waveformDisplay_.getBounds());
    put(c.topBarRect, topBar_ != nullptr ? topBar_->getBounds() : juce::Rectangle<int>());
    // s-rta-0929 g4cpu (TEST-ONLY): the rects probe-idle-paint a1 classifies each display pass by.
    put(c.deckRect, deckView_ != nullptr ? deckView_->getBounds() : juce::Rectangle<int>());
    put(c.padRowRect, deckView_ != nullptr ? getLocalArea(deckView_.get(), deckView_->getRoutinePadRowBounds())
                                           : juce::Rectangle<int>());
    put(c.stripColRect, deckView_ != nullptr ? getLocalArea(deckView_.get(), deckView_->getStripColumnBounds())
                                             : juce::Rectangle<int>());
    put(c.wheelRect, topBar_ != nullptr ? getLocalArea(topBar_.get(), topBar_->getWheelRepaintBounds()) : juce::Rectangle<int>());
    put(c.inspectorRect, inspectorPanel_ != nullptr ? inspectorPanel_->getBounds() : juce::Rectangle<int>());
    put(c.previewRect, previewPanel_.getBounds());   // s-rta-0929 g4cpu-fix (TEST-ONLY): probe-idle-paint v5's mask
#endif
}

// s-rta-0929 g4cpu (TEST-ONLY): one display pass ends here (JUCE calls paintOverChildren on every pass that reaches
// MainComponent, even when opaque children covered the clip and paint() was skipped) -> GET /api/debug/ui_passes.
void MainComponent::paintOverChildren(juce::Graphics& g)
{
#if AUDIODNA_TEST_SERVER
    const auto r = g.getClipBounds();
    uipaint::passEnd(r.getX(), r.getY(), r.getWidth(), r.getHeight());
#else
    juce::ignoreUnused(g);
#endif
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
        setFileLabel(file.getFileName());
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
            setFileLabel("Saved: " + saveFile.getFileName());
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
            setFileLabel("Loaded: " + file.getFileNameWithoutExtension());
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
    // s-rta-0929 asyncload: a staged load never outlives the model it was staged against (New Composition; a staged
    // load's own completion has already moved staged_ out, so this is then a no-op).
    cancelStagedOpen(LoadTicket::Outcome::Superseded);
    ++modelEpoch_;   // AL7: a queued Duplicate's source deck id belongs to the model it was read in

    // s-rta-0926 routines (plan R7): a running routine's Player holds coordinates into the OLD
    // model -- stop every routine (releasing its grips on the old model) before it is replaced.
    routineEngine_.stopAll();

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
    // s-rta-0928 R1.5: prefetch the new composition's images off the GL thread (active deck, active clips first) and
    // release the textures of images it no longer has -- at the next frame start, before any pass.
    renderer.getCompositor().postImageSet(compload::imagePaths(composition_));

    // Loading/replacing/appending is not itself undoable. clear() only
    // touches command history (never the model) — a stale command left alive
    // could re-resolve, by coordinate, a valid-but-WRONG cell in the new
    // model (memory-safe, semantically wrong — see the work packet §2 proof).
    undoManager_.clear();

    refreshUiAfterModelSwap();
}

// plan6 §7: the confirm before a composition is replaced by a one-click gesture (the library row, New
// Composition). Asynchronous (no modal loop); the callback runs only on OK. File > Open... (a two-step chooser)
// and REST /api/load_composition do not ask.
void MainComponent::confirmReplaceShow(const juce::String& title, const juce::String& question,
                                       const juce::String& okLabel, std::function<void()> proceed)
{
    // NoIcon + associatedComponent = this: the dialog is created by -- and draws with -- the app LookAndFeel.
    juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::NoIcon, title,
        question + "\n\nEverything playing now will be replaced. To keep the current composition, Cancel and save it first.",
        okLabel, "Cancel", this,
        juce::ModalCallbackFunction::create([proceed = std::move(proceed)](int result) {
            if (result == 1) proceed();
        }));
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

// s-rta-0929 asyncload (plan-asyncload.md 5.4 + HARMONY ADOPTION; Pitfall NN): the three loads (loadComposition,
// appendDeckFromFile, duplicateDeck) STAGE a private model on this thread (parse, validate, re-mint, reconcile -- as
// before), then beginStagedOpen seeds presence and hands one MediaOpener job per present video clip to the pool; each
// landing writes dims / alpha / thumbnail into the STAGED clip (a live Clip write would race the GL thread) and installs
// the player under its re-minted id (its decode thread parks until the first draw: frame 0 is ready at the cut); the
// last landing runs finishStagedLoad -- the staged sequences opened (no I/O) BEFORE the swap, then exactly the old
// swap / InsertDeckCmd / label code. An empty batch (no present video) completes inside beginStagedOpen, on this
// message, as before. The live composition keeps playing -- and answering triggers, edits and REST (R2) -- until the cut.
void MainComponent::setFileLabel(const juce::String& text)
{
    if (staged_ != nullptr && staged_->label.divert(text.toStdString()))
        return;   // AL5: held for a cancel; the label keeps "Loading <name>..."
    fileLabel_.setText(text, juce::dontSendNotification);
}

void MainComponent::refreshAudioDeviceNotice(bool relayout)
{
    juce::String text;
    switch (audioEngine_.getDeviceState())
    {
        case AudioEngine::DeviceState::NoDevice: text = "No audio device found - plug one in. Bluetooth is never used."; break;
        case AudioEngine::DeviceState::NoInput:  text = "No wired mic found - plug one in. Bluetooth is never used."; break;
        case AudioEngine::DeviceState::Ok:       break;
    }
    if (text == audioDeviceNotice_.getText() && audioDeviceNotice_.isVisible() == text.isNotEmpty())
        return;
    audioDeviceNotice_.setText(text, juce::dontSendNotification);
    audioDeviceNotice_.setVisible(text.isNotEmpty());
    if (relayout)
        resized();
}

void MainComponent::publishLoadWitness()
{
    stagedNow_.store(staged_ != nullptr ? 1 : 0, std::memory_order_relaxed);
    stagedPlayers_.store(staged_ != nullptr ? static_cast<int>(staged_->adopted.ids.size()) : 0, std::memory_order_relaxed);
    queuedNow_.store(static_cast<int>(loadQueue_.size()), std::memory_order_relaxed);
}

Clip* MainComponent::findStagedClip(uint32_t clipId)
{
    if (!staged_)
        return nullptr;
    auto inDeck = [clipId](Deck& deck) -> Clip* {
        for (auto& layer : deck.layers)
            for (auto& cell : layer.clips)
                if (cell.has_value() && cell->id == clipId)
                    return &*cell;
        return nullptr;
    };
    if (staged_->kind == stagedload::Kind::Composition)
    {
        for (auto& deck : staged_->comp.decks)
            if (auto* c = inDeck(deck))
                return c;
        return nullptr;
    }
    return inDeck(staged_->deck);
}

void MainComponent::beginStagedOpen(std::unique_ptr<StagedLoad> staged)
{
    cancelStagedOpen(LoadTicket::Outcome::Superseded);   // one staged load at a time (a Composition supersedes)
    staged_ = std::move(staged);
    std::vector<MediaOpener::Job> jobs;
    auto collect = [this, &jobs](Deck& deck) {
        {
            // s-rta-0928b mediaopen: seed Clip::mediaMissing on the STAGED deck (a stat per Image / Video clip); the
            // sweep keeps it current once the deck is live.
            LoadTiming::Scope t(loadTiming_, LoadTiming::Prep);
            presence::seed(deck);
        }
        for (auto& layer : deck.layers)
            for (auto& cell : layer.clips)
                if (cell.has_value() && cell->isPlayable() && cell->mediaType == Clip::MediaType::Video
                    && !cell->mediaMissing)   // a missing file: no job (R8), as the old :3027 skip
                    jobs.push_back({ cell->id, cell->mediaFile });
    };
    if (staged_->kind == stagedload::Kind::Composition)
        for (auto& deck : staged_->comp.decks)
            collect(deck);
    else
        collect(staged_->deck);
    if (!jobs.empty())
    {
        const auto loading = staged_->label.hold(fileLabel_.getText().toStdString(),
                                                 stagedload::loadingLabel(staged_->kind, staged_->name.toStdString()));
        fileLabel_.setText(juce::String::fromUTF8(loading.c_str()), juce::dontSendNotification);
    }
    publishLoadWitness();
    // `this` captures: mediaOpener_ is a member destroyed before `this`, and every landing reaches it through its
    // WeakReference. An empty batch runs finishStagedLoad before begin() returns. Nothing is touched after begin().
    mediaOpener_.begin(std::move(jobs),
                       [this](uint32_t id, std::unique_ptr<VideoPlayer> p) { onStagedLanded(id, std::move(p)); },
                       [this] { finishStagedLoad(); });
}

void MainComponent::onStagedLanded(uint32_t clipId, std::unique_ptr<VideoPlayer> player)
{
    const auto t0 = LoadTiming::Clock::now();
    Clip* clip = findStagedClip(clipId);
    if (clip != nullptr && player != nullptr)   // a null player = open() failed: the clip keeps its file defaults (R8)
    {
        clip->hasAlpha = player->hasAlpha();
        clip->clipWidth = player->getWidth();
        clip->clipHeight = player->getHeight();
        clip->thumbnail = player->getThumbnail(90, 72);
        previewPanel_.getRenderer().installVideoPlayer(clipId, std::move(player));   // its thread parks: nothing draws it
        staged_->adopted.add(clipId);
    }
    // (no staged clip: cannot happen while the batch is live -- the player dies here, unpublished)
    const double ms = LoadTiming::msSince(t0);
    loadTiming_.add(LoadTiming::Opens, ms);
    loadTiming_.noteOpen(ms);
    publishLoadWitness();
}

void MainComponent::finishStagedLoad()
{
    auto s = std::move(staged_);
    if (!s)
        return;
    s->label.clear();
    publishLoadWitness();
    auto& renderer = previewPanel_.getRenderer();
    {
        // The staged SEQUENCES open now (no I/O since mediaopen), under their ids, BEFORE the swap: the GL thread finds
        // them at the first frame after the cut (R12: a cancelled batch never opened one).
        LoadTiming::Scope t(loadTiming_, LoadTiming::Seq);
        auto openSeqs = [&renderer, &s](Deck& deck) {
            for (auto& layer : deck.layers)
                for (auto& cell : layer.clips)
                    if (cell.has_value() && cell->isPlayable() && cell->mediaType == Clip::MediaType::ImageSequence
                        && !cell->sequenceFiles.empty())
                    {
                        renderer.openImageSequenceForClip(cell->id, cell->sequenceFiles, cell->sequenceFps);
                        s->adopted.add(cell->id);
                    }
        };
        if (s->kind == stagedload::Kind::Composition)
            for (auto& deck : s->comp.decks)
                openSeqs(deck);
        else
            openSeqs(s->deck);
    }
    const auto done = juce::String::fromUTF8(stagedload::doneLabel(s->kind, s->name.toStdString()).c_str());
    if (s->kind == stagedload::Kind::Composition)
    {
        // 6. SWAP -- fenced; closes orphaned media by set difference, clears undo history, nulls the inspectors,
        //    rebuilds the grid (see swapCompositionModel / refreshUiAfterModelSwap above).
        {
            LoadTiming::Scope t(loadTiming_, LoadTiming::Swap);
            swapCompositionModel([this, &s] { composition_ = std::move(s->comp); });
        }
        // 7. LABEL
        LoadTiming::Scope t(loadTiming_, LoadTiming::Ui);
        setFileLabel(done);
        if (browserPanel_)
            browserPanel_->getCompDecksBrowser().refresh();
    }
    else
    {
        // 6. APPEND -- one undoable InsertDeckCmd (plan6 §5 A1-d), not a whole-model swap: an append retires no media
        //    (nothing to close), must not stop running routines, and must not wipe undo history. The command fences
        //    the push_back (Composition::appendDeck reallocates `decks`, which the GL thread walks lock-free --
        //    renderOpenGL()'s P21 persistent-layer loop), mints a fresh deck id, makes the deck active
        //    (withDeckDetached re-points the renderer), and cancels any pending quantized trigger on the deck being
        //    left (restored on undo). Undo disposes the appended deck's media; redo reopens it.
        const bool dup = s->kind == stagedload::Kind::DeckDuplicate;
        const char* what = dup ? "Duplicate Deck" : "Load Deck";
        {
            LoadTiming::Scope t(loadTiming_, LoadTiming::Swap);
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<InsertDeckCmd>(
                makeCompositionResolver(), makeDeckFence(), makeClipMediaHook(), makeClipMediaDisposeHook(),
                std::move(s->deck), what));
            pushCommands(std::move(children), what);
        }
        LoadTiming::Scope t(loadTiming_, LoadTiming::Ui);
        if (deckView_) deckView_->rebuildGrid();   // rebuilds the deck tabs too (setupDeckTabs)
        if (auto* active = composition_.getActiveDeck())
            refreshPreviewFromActiveClip(*active);
        // 7. LABEL
        setFileLabel(done);
        if (!dup && browserPanel_)
            browserPanel_->getCompDecksBrowser().refresh();
    }
    loadTiming_.end();
    if (s->ticket)
        s->ticket->finish(LoadTicket::Outcome::Done);
    s.reset();
    pumpLoadQueue();
}

void MainComponent::cancelStagedOpen(LoadTicket::Outcome why)
{
    mediaOpener_.cancel();   // never waits: queued opens dropped, an in-flight one lands Stale
    if (!staged_)
        return;   // (a staged load's own completion: the queue it may leave is pumped by finishStagedLoad)
    auto s = std::move(staged_);
    loadQueue_.clear();      // AL7: whatever retires a staged load drops the requests queued behind it
    auto& renderer = previewPanel_.getRenderer();
    for (auto id : s->adopted.takeAll())
        renderer.closeMediaForClip(id);   // the GL-drained retire list (their threads were parked, nothing drew them)
    if (s->label.held())
    {
        const auto shown = s->label.release(fileLabel_.getText().toStdString());
        fileLabel_.setText(juce::String::fromUTF8(shown.c_str()), juce::dontSendNotification);
    }
    if (s->ticket)
        s->ticket->finish(why);
    publishLoadWitness();
}

void MainComponent::pumpLoadQueue()
{
    // AL7: prepare the next queued Append / Duplicate against the NOW-live composition (a request whose preparation
    // fails, or a Duplicate whose source deck is gone, is skipped). A batch with no video completes inside
    // stageDeck*, which pumps again -- bounded by the queue (<= 8).
    while (staged_ == nullptr)
    {
        auto q = loadQueue_.pop();
        publishLoadWitness();
        if (!q)
            return;
        if (q->kind == stagedload::Kind::DeckAppend)
        {
            stageDeckAppend(juce::File(juce::String::fromUTF8(q->file.c_str())));
            continue;
        }
        std::vector<uint32_t> ids;
        for (const auto& d : composition_.decks)
            ids.push_back(d.id);
        const int idx = stagedload::resolveDuplicateSource(q->epoch, modelEpoch_, ids, q->deckId);
        if (idx < 0)
            setFileLabel(juce::String::fromUTF8(stagedload::sourceGoneLabel(q->name).c_str()));
        else
            stageDeckDuplicate(idx);
    }
}

namespace
{
// s-rta-0927 source-defects: the registry's CURRENT param list of a source type, for
// compload::reconcileSourceParams on every composition/deck load. nullopt for a type the registry does not know.
compload::SourceParamLookup sourceParamLookup(const SourceRegistry& registry)
{
    return [&registry](const std::string& type) -> std::optional<std::vector<compload::RegisteredSourceParam>> {
        auto src = registry.createSource(type);
        if (!src)
            return std::nullopt;
        std::vector<compload::RegisteredSourceParam> out;
        for (int i = 0; i < src->getNumParams(); ++i)
        {
            const auto& p = src->getParam(i);
            out.push_back({ p.name, p.uniformName, p.defaultValue });
        }
        return out;
    };
}
} // namespace

void MainComponent::loadComposition(const juce::File& file, std::shared_ptr<LoadTicket> ticket)
{
    // 1. STAGE — load into a private `incoming`, never the live composition_:
    //    a well-formed-but-wrong-shape file (FX preset, lone deck) would
    //    otherwise "succeed" via fromVar, leaving every hasProperty-guarded
    //    field at its OLD value — a stale hybrid of two compositions.
    loadTiming_.begin();   // s-rta-0929 asyncload: the load's cost split (/api/state load.timing)
    Composition incoming;
    bool parsed = false;
    {
        LoadTiming::Scope t(loadTiming_, LoadTiming::Parse);
        parsed = incoming.loadFromFile(file);
    }
    if (!parsed)
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Open Composition",
                file.getFileName() + ": could not read/parse file");
        if (ticket) ticket->finish(LoadTicket::Outcome::Failed);   // s-rta-0929 asyncload
        return;   // Live state untouched (a staged load stays staged).
    }

    // 2. VALIDATE — refuse (no decks / a deck with no layers) or repair
    //    (activeDeckIndex out of range, numColumns/padding) — on `incoming`
    //    only. Live state untouched either way.
    const auto tPrep = LoadTiming::Clock::now();
    if (auto reason = compload::validateComposition(incoming); !reason.empty())
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Open Composition",
                file.getFileName() + ": " + reason);
        if (ticket) ticket->finish(LoadTicket::Outcome::Failed);   // s-rta-0929 asyncload
        return;
    }

    // 3. RE-MINT — every clip gets a fresh id from the file-static mint.
    //    Saved ids are advisory (nothing persistent references them); a
    //    fresh id can never collide with a LIVE clip's id, which is what
    //    makes step 4 safe without needing L1-FU.
    compload::remintClipIds(incoming, s_nextClipId);

    // 3b. RECONCILE — every Source clip's params to the registry's current list
    //     (a file from an older build keeps controls no shader reads, and old
    //     defaults as the right-click reset target). On `incoming`, pre-swap.
    compload::reconcileSourceParams(incoming, sourceParamLookup(previewPanel_.getRenderer().getSourceRegistry()));
    loadTiming_.add(LoadTiming::Prep, LoadTiming::msSince(tPrep));

    // 4. STAGE THE OPENS (s-rta-0929 asyncload; was: every video opened here, on this thread) -- the videos open on
    //    MediaOpener's pool into `incoming`'s clips (never a live Clip: a post-swap write would race the GL thread);
    //    the output keeps showing the OLD comp until the last one lands; then finishStagedLoad swaps (step 6) and
    //    labels (step 7). AL7: a Composition supersedes everything staged or queued before it.
    // 5. NAME — design decision: composition name = file base name on Load
    //    (and Save As), so the browser row, the inspector label, and Collect
    //    Media's folder name all agree. Write on `incoming`, not the live
    //    object.
    incoming.name = file.getFileNameWithoutExtension().toStdString();
    auto s = std::make_unique<StagedLoad>();
    s->kind = stagedload::Kind::Composition;
    s->comp = std::move(incoming);
    s->name = file.getFileNameWithoutExtension();
    s->ticket = std::move(ticket);
    loadQueue_.clear();
    beginStagedOpen(std::move(s));
}

void MainComponent::saveComposition()
{
    if (composition_.filePath != juce::File()
        && composition_.filePath.getParentDirectory().isDirectory())
    {
        if (composition_.saveToFile(composition_.filePath))
        {
            setFileLabel("Saved: " + composition_.filePath.getFileName());
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
            setFileLabel("Saved: " + saveFile.getFileName());
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

// L3 STEP 3 (2026-09) + plan6 §6.4: the library's Decks-row click and Load
// Deck... APPENDS the saved deck into the live composition as a new tab and
// makes it active — never replaces the deck on screen (see the header comment
// on appendDeckFromFile's declaration for why). STAGE -> VALIDATE -> RE-MINT ->
// OPEN NEW -> NAME on a private Deck (like loadComposition), then ONE undoable
// InsertDeckCmd instead of a whole-model swap.
void MainComponent::appendDeckFromFile(const juce::File& file)
{
    // s-rta-0929 asyncload (AL7): behind a staged load this request waits its turn (FIFO) and is prepared when dequeued.
    const auto name = file.getFileNameWithoutExtension().toStdString();
    switch (stagedload::admit(stagedload::Kind::DeckAppend, staged_ != nullptr, loadQueue_.size()))
    {
        case stagedload::Admit::Enqueue:
            loadQueue_.push({ stagedload::Kind::DeckAppend, file.getFullPathName().toStdString(), 0, 0, name });
            publishLoadWitness();
            return;
        case stagedload::Admit::Refuse:
            setFileLabel(juce::String::fromUTF8(stagedload::queueFullLabel(name).c_str()));
            return;
        case stagedload::Admit::Begin:
        case stagedload::Admit::Supersede:
            break;
    }
    stageDeckAppend(file);
}

void MainComponent::stageDeckAppend(const juce::File& file)
{
    // 1. STAGE + shape-check — a deck file's top level is `Deck::toVar()`'s
    //    shape ("layers"/"numColumns"/"name"/"id"), not a composition's
    //    ("decks") or an FX preset's. Refuse before touching the model:
    //    Deck::fromVar's own hasProperty-less getProperty calls would
    //    otherwise happily default-construct an empty/wrong Deck from either.
    loadTiming_.begin();   // s-rta-0929 asyncload: the load's cost split (/api/state load.timing)
    const auto tParse = LoadTiming::Clock::now();
    auto parsed = juce::JSON::parse(file.loadFileAsString());
    loadTiming_.add(LoadTiming::Parse, LoadTiming::msSince(tParse));
    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("layers"))
    {
        if (!testMode_)
            juce::AlertWindow::showMessageBoxAsync(
                juce::MessageBoxIconType::WarningIcon,
                "Load Deck",
                file.getFileName() + ": not a deck file");
        return;   // Live state untouched (a queued request behind it is pumped by the caller).
    }

    const auto tPrep = LoadTiming::Clock::now();
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

    // 3b. RECONCILE — as loadComposition (source params to the registry's current list).
    compload::reconcileSourceParams(incoming, sourceParamLookup(previewPanel_.getRenderer().getSourceRegistry()));
    loadTiming_.add(LoadTiming::Prep, LoadTiming::msSince(tPrep));

    // 4. (s-rta-0929 asyncload) the videos open on MediaOpener's pool into `incoming` -- beginStagedOpen below.
    // 5. NAME — ALWAYS the file's base name (plan6 §6.4): the library row the
    //    user clicked IS the file name, and Save Deck As enforces name == file.
    //    The library link (Deck::sourceFile, runtime only) lets Save Deck
    //    overwrite this same file later.
    incoming.name = file.getFileNameWithoutExtension().toStdString();
    incoming.sourceFile = file;

    // 6-7. APPEND + LABEL at the completion (finishStagedLoad): one undoable InsertDeckCmd "Load Deck", the grid, the
    //      preview, "Loaded deck: <name>", the browser.
    auto s = std::make_unique<StagedLoad>();
    s->kind = stagedload::Kind::DeckAppend;
    s->deck = std::move(incoming);
    s->name = file.getFileNameWithoutExtension();
    beginStagedOpen(std::move(s));
}

// plan6 §6.4 — the deck tab row's handlers. Message thread; each guards its
// deck index (menus and choosers are async, so the model can change meanwhile).

void MainComponent::newDeck()
{
    // #21: command-owns-the-mutation (push_back is non-idempotent). The
    // fenced AddDeckCmd appends the deck (3 layers, fresh id), makes it active,
    // and re-points the renderer (via withDeckDetached's re-resolve) —
    // perform() runs it. The new deck is empty: reconcile the preview fallback
    // like a switch to an empty deck.
    std::vector<std::unique_ptr<Command>> children;
    children.push_back(std::make_unique<AddDeckCmd>(
        makeCompositionResolver(), makeDeckFence(), "Add Deck"));
    pushCommands(std::move(children), "Add Deck");
    if (deckView_) deckView_->rebuildGrid();
    if (auto* active = composition_.getActiveDeck())
        refreshPreviewFromActiveClip(*active);
}

void MainComponent::loadDeck()
{
    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Load Deck...",
        CompDecksBrowser::getDecksDir(),
        "*.json");

    auto flags = juce::FileBrowserComponent::openMode
               | juce::FileBrowserComponent::canSelectFiles;

    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;
        appendDeckFromFile(file);
    });
}

// Overwrite the deck's library file when it has one that still matches the
// deck (the folder exists and the file's base name is the deck's name — a
// Rename breaks the link); otherwise Save Deck As... (mirrors Composition >
// Save falling through to Save As).
void MainComponent::saveDeck(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;
    const Deck& deck = composition_.decks[static_cast<size_t>(deckIndex)];
    if (deck.sourceFile != juce::File()
        && deck.sourceFile.getParentDirectory().isDirectory()
        && deck.sourceFile.getFileNameWithoutExtension().toStdString() == deck.name)
        writeDeckFile(deck, deck.sourceFile);
    else
        saveDeckAs(deckIndex);
}

void MainComponent::saveDeckAs(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;

    auto dir = CompDecksBrowser::getDecksDir();
    dir.createDirectory();

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Save Deck As...",
        dir.getChildFile(juce::String(composition_.decks[static_cast<size_t>(deckIndex)].name) + ".json"),
        "*.json");

    auto flags = juce::FileBrowserComponent::saveMode
               | juce::FileBrowserComponent::canSelectFiles
               | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser_->launchAsync(flags, [this, deckIndex](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File{})
            return;
        if (deckIndex >= static_cast<int>(composition_.decks.size()))
            return;   // the deck went away while the chooser was open

        auto saveFile = file.hasFileExtension(".json") ? file
                            : file.withFileExtension("json");

        auto& deck = composition_.decks[static_cast<size_t>(deckIndex)];
        if (writeDeckFile(deck, saveFile))
        {
            // Like Save Composition As: the deck takes the file's name and
            // remembers the file (NOT undoable — a file write, like Save As).
            deck.sourceFile = saveFile;
            deck.name = saveFile.getFileNameWithoutExtension().toStdString();
            if (deckView_) deckView_->refresh();   // relabel + tooltip
        }
    });
}

// One deck -> one library file, in Deck::toVar()'s shape (top-level "layers" —
// what appendDeckFromFile's shape check accepts; the shape the old browser
// "Save Deck" button wrote).
bool MainComponent::writeDeckFile(const Deck& deck, const juce::File& file)
{
    if (file.replaceWithText(juce::JSON::toString(deck.toVar())))
    {
        setFileLabel("Saved deck: " + file.getFileNameWithoutExtension());
        if (browserPanel_)
            browserPanel_->getCompDecksBrowser().refresh();
        return true;
    }
    if (!testMode_)
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "Save Deck",
            "Save failed: " + file.getFullPathName());
    return false;
}

void MainComponent::renameDeck(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;
    const std::string oldName = composition_.decks[static_cast<size_t>(deckIndex)].name;

    // No explicit setLookAndFeel(&lookAndFeel_) needed: this top-level window falls back to the app
    // LookAndFeel via installAsDefault() (decks-followup ITEM 1) -- verified pixel-identical to the
    // prior per-site workaround (test_lookandfeel_square.cpp; decks-followup-shots/06-rename-dialog).
    auto* w = new juce::AlertWindow("Rename Deck", "", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor("name", juce::String(oldName));
    w->addButton("Rename", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    // deleteWhenDismissed = true: ModalComponentManager runs this callback BEFORE
    // it deletes the window, so reading w's text editor inside it is safe.
    w->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, deckIndex, w, oldName](int result) {
            const auto text = w->getTextEditorContents("name").trim();
            if (result != 1 || text.isEmpty()
                || deckIndex >= static_cast<int>(composition_.decks.size())
                || text.toStdString() == oldName)
                return;
            std::vector<std::unique_ptr<Command>> children;
            children.push_back(std::make_unique<RenameDeckCmd>(
                makeCompositionResolver(), deckIndex, oldName, text.toStdString(), "Rename Deck"));
            pushCommands(std::move(children), "Rename Deck");
            if (deckView_) deckView_->refresh();   // relabel the tab
        }), true);
}

void MainComponent::duplicateDeck(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;

    // s-rta-0929 asyncload (AL7): behind a staged load this request waits its turn (FIFO), naming its source deck by id
    // (+ the model epoch), and is prepared -- the value copy made -- when dequeued.
    const auto& src = composition_.decks[static_cast<size_t>(deckIndex)];
    switch (stagedload::admit(stagedload::Kind::DeckDuplicate, staged_ != nullptr, loadQueue_.size()))
    {
        case stagedload::Admit::Enqueue:
            loadQueue_.push({ stagedload::Kind::DeckDuplicate, {}, src.id, modelEpoch_, src.name });
            publishLoadWitness();
            return;
        case stagedload::Admit::Refuse:
            setFileLabel(juce::String::fromUTF8(stagedload::queueFullLabel(src.name).c_str()));
            return;
        case stagedload::Admit::Begin:
        case stagedload::Admit::Supersede:
            break;
    }
    stageDeckDuplicate(deckIndex);
}

void MainComponent::stageDeckDuplicate(int deckIndex)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;

    // A value copy under "<name> copy" with every clip re-minted (a clip id is
    // live in at most ONE cell — makeClipMediaDisposeHook's FUTURE-FRAGILE
    // note) and no queued trigger; media opened on the staged copy under its
    // new ids BEFORE the fenced append (s-rta-0929 asyncload: on MediaOpener's pool).
    loadTiming_.begin();   // s-rta-0929 asyncload: the load's cost split (/api/state load.timing)
    const auto tPrep = LoadTiming::Clock::now();
    auto s = std::make_unique<StagedLoad>();
    s->kind = stagedload::Kind::DeckDuplicate;
    s->deck = compload::duplicateDeck(composition_.decks[static_cast<size_t>(deckIndex)], s_nextClipId);
    s->name = juce::String(s->deck.name);   // "<name> copy" (the label's name, R13)
    loadTiming_.add(LoadTiming::Prep, LoadTiming::msSince(tPrep));
    beginStagedOpen(std::move(s));
}

// Remove ANY deck (a tab's menu names it; the Deck menu passes the active one).
// No dialog — the tab row shows a 10-s "Undo Remove" button instead (plan6 R1),
// and Composition > Undo works as for every deck command.
void MainComponent::removeDeck(int deckIndex)
{
    if (composition_.decks.size() <= 1
        || deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
        return;   // a composition keeps at least one deck

    const bool activeChanges = (deckIndex == composition_.activeDeckIndex);
    const juce::String name(composition_.decks[static_cast<size_t>(deckIndex)].name);

    // Null both inspectors FIRST: the erased deck's Layer/Clip objects die, and
    // an inspector can be showing one of them even when a BACKGROUND deck is
    // removed (handleDeckSwitch never re-points the inspectors).
    if (inspectorPanel_)
    {
        inspectorPanel_->getClipInspector().setClip(nullptr);
        inspectorPanel_->getLayerInspector().setLayer(nullptr);
    }

    // #22: command-owns-the-mutation. Snapshot the full Deck VALUE + the prior
    // active index; the fenced execute() erases and keeps the on-screen deck
    // object active (its index drops by one when a deck before it goes).
    Deck removedCopy = composition_.decks[static_cast<size_t>(deckIndex)];
    std::vector<std::unique_ptr<Command>> children;
    children.push_back(std::make_unique<RemoveDeckCmd>(
        makeCompositionResolver(), makeDeckFence(),
        makeClipMediaHook(), makeClipMediaDisposeHook(),
        deckIndex, std::move(removedCopy), composition_.activeDeckIndex, "Remove Deck"));
    pushCommands(std::move(children), "Remove Deck");

    if (deckView_)
    {
        if (activeChanges)
        {
            deckView_->clearSelection();
            deckView_->selectLayer(-1);
        }
        deckView_->rebuildGrid();
        // After rebuildGrid (refreshUiAfterModelSwap's order): setActiveColumn
        // refreshes the strips, which must already point into the live model.
        if (activeChanges)
            deckView_->setActiveColumn(-1);
    }
    if (activeChanges)
        if (auto* active = composition_.getActiveDeck())
            refreshPreviewFromActiveClip(*active);
    if (inspectorPanel_)
        inspectorPanel_->refresh();

    if (deckView_)
        deckView_->showUndoHint("Undo Remove \"" + name + "\"");
    setFileLabel("Removed deck \"" + name + "\"");
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

    // plan5 7.2-7.3 (s-rta-0927 outputs-c2): the output keys, classified in ONE place
    // (output::classifyOutputKey, tests/test_output_menu_model.cpp). Cmd+Shift+Esc is tested BEFORE the bare-Escape
    // case: KeyPress::isKeyCode compares the key code only, so a modifier-blind Escape branch would swallow it.
    switch (output::classifyOutputKey(key))
    {
        case output::OutputKey::CloseAll:        // Cmd+Shift+Esc = PANIC: close every output
            outputs_.closeAll();
            return true;
        case output::OutputKey::RaiseApp:        // Cmd+` = the app window back above an output that covers it
            if (auto* top = getTopLevelComponent())
                top->toFront(true);
            return true;
        case output::OutputKey::ToggleMain:      // Cmd+F = the output on the main display (Boris: leave Cmd+F as-is)
            outputs_.toggleDisplay(outputs_.mainDisplayIndex());
            return true;
        case output::OutputKey::SwallowEscape:   // plan5 Q2: plain Esc no longer touches outputs; still swallowed
            return true;
        case output::OutputKey::None:
            break;
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
                setFileLabel(file.getFileName());
                applyAudioTransport("play", Origin::Human);
            }
        }
        else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                 ext == ".gif" || ext == ".bmp" || ext == ".tiff")
        {
            previewPanel_.loadImage(file);
            setFileLabel(file.getFileName());
        }
    }
}

// s-rta-0923 lane 3 (plan section 3.4): manualWrite/manualRelease/manualTouch
// are the thin MainComponent wrapper over the headless core in
// src/connect/ManualWrite.h (Lane C1 implements the core bodies; this
// wrapper's only job is Origin+GripKind -> Hand and the recorder's hook
// seam — R8, this lane owns manualWrite, the recorder only hooks it later).
bool MainComponent::manualWrite(const ControlPath& p, float v, GripKind k, Origin o)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualWriteCore(*ref, v, handFor(o, k), k, connNow(), composition_.gripHoldMs);
    if (onManualWrite) onManualWrite(p, v, k, o, ok);
    return ok;
}

void MainComponent::manualRelease(const ControlPath& p, Origin o)
{
    if (auto ref = resolveControl(composition_, globalMacroBank_, p))
        manualReleaseCore(*ref, handFor(o, GripKind::Held));
    if (onManualRelease) onManualRelease(p, o);
}

bool MainComponent::manualTouch(const ControlPath& p, GripKind k, Origin o)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    auto ref = resolveControl(composition_, globalMacroBank_, p);
    bool ok = ref.has_value() && manualTouchCore(*ref, handFor(o, k), k, connNow(), composition_.gripHoldMs);
    if (onManualTouch) onManualTouch(p, k, o, ok);
    return ok;
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

    // Master Signal (s-rta-0925 mastersignal Step 1): ONE read per tick,
    // hoisted ABOVE all three consumers below (v1 MappingEngine, MacroBank,
    // ConnectionEngine::Context) -- folds critic-plan-mastersignal.md's
    // BLOCKING ordering finding. When CompScalar::Signal is itself
    // connected this is the PREVIOUS tick's twin (~8ms lag at 120Hz,
    // accepted) -- its own connection is evaluated at full depth
    // regardless (ConnectionEngine.cpp's fullDepthIndex exemption), so it
    // can still reach 0 one tick later.
    const float signalDepth = composition_.eff(CompScalar::Signal);

    previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain(), signalDepth);

    // L9 (modulation-freeze fix, 2026-09-05): drive the shared global
    // MacroBank and every Inspector's signal/macro-driven effect-param
    // modulation from this same unconditional 120Hz message-thread timer,
    // instead of the ~10Hz InspectorPanel::refresh() timer gated to whichever
    // tab is active — see InspectorPanel::tickModulation() /
    // EffectStackView::tickModulation(). MacroBank updates first so
    // tickModulation()'s getMacroValue() reads this tick's value rather than
    // the previous one.
    globalMacroBank_.updateValues(signalRegistry_, signalDepth);

    // s-rta-0923/0924 step 3 (Lane S3-B, plan section 3.3 B1, critic A3): the
    // recorder's clock tick + Player::advanceTo run BEFORE the connection
    // engine evaluates below, so a replayed gesture's grip is current when
    // the engine decides what to publish (ConnectionEngine.cpp's own
    // ORDERING FACT). `now` is hoisted above the block and shared with the
    // engine's ctx.now just below -- ONE clock for grips and recorder stamps
    // (ConnClock.h), not two unequal readings.
    const double now = connNow();
    // s-rta-0925: mirror the audio source mode for /api/perf/status.inputSource (HTTP-thread read) --
    // one message-thread write site, self-healing whatever moved the mode; never a device read off
    // the HTTP thread (the same posture as every other perfStatusVar() field).
    inputSourceMirror_.store(audioEngine_.getSourceMode() == AudioEngine::SourceMode::File ? 1 : 0,
                              std::memory_order_relaxed);
    {
        std::optional<int64_t> transportFrames;
        if (recorderHost_.needsTransportFrames())
            transportFrames = audioEngine_.getTransportSource().getNextReadPosition();
        recorderHost_.tick(snap, now, audioEngine_.getDeliveredSamples(),
                           audioEngine_.getAudioTap(), transportFrames,
                           audioEngine_.getCurrentSampleRate());
    }

    // s-rta-0926 routines slice 1 (plan 4.5): AFTER the replay (so a routine fired over a set
    // replay writes last each tick -- R13, tick order) and BEFORE the connection engine below (so
    // a routine gesture's grip is current when the engine decides what to publish). The global
    // Quantize override is the clip trigger's own rule (quantizeModeToForcedSnap).
    routineEngine_.tick(snap, now, composition_,
                        static_cast<RoutineSnap>(quantizeModeToForcedSnap(composition_.quantizeMode, snap)),
                        snap.trackerState == BPMTracker::STATE_LOCKED && snap.bpm > 0.0f);

    // S-RTA-0923 LANE 3: the ONE evaluator for every ParamConnection (s166
    // spec section 4.2/4.3). Macros were updated just above (they are
    // sources); ConnectionEngine::tick IS called here, right after the
    // recorder's tick above (ConnectionEngine.cpp's own ORDERING FACT — see
    // the COLLISION NOTICE in the lane 3 plan section 5, C3). tickModulation()
    // (below) is the pre-existing effect/source-param path (retired later by
    // the effect-param lane, plan section 11) and coexists safely: the
    // engine below publishes only into scalarLive twins; tickModulation()
    // still writes the old fields directly.
    const float dt = (lastConnTick_ > 0.0)
                        ? std::clamp(static_cast<float>(now - lastConnTick_), 0.0f, 0.05f)
                        : 1.0f / static_cast<float>(kMappingTickHz);
    lastConnTick_ = now;
    ConnectionEngine::Context ctx{ signalRegistry_, globalMacroBank_, snap, dt, now,
                                   composition_.gripHoldMs, composition_.handBackGlideMs, signalDepth };
    connectionEngine_.tick(composition_, ctx);

    if (inspectorPanel_) inspectorPanel_->tickModulation();
}

void MainComponent::timerCallback()
{
    // plan5 C3: the hot-plug backstop, on EVERY tick (30 Hz) -- reconciles the outputs only when the display list
    // differs from the last one seen; otherwise a cached comparison, nothing else.
    outputs_.pollDisplays();
#if AUDIODNA_TEST_SERVER
    // s-rta-0929 asyncload (R9): CoreAudio's own processor-overload count (kAudioDeviceProcessorOverload), sampled here.
    if (auto* dev = audioEngine_.getDeviceManager().getCurrentAudioDevice())
        audioXruns_.store(dev->getXRunCount(), std::memory_order_relaxed);
#endif

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

        // s-rta-0924b step 4: the Record tab's ~4 Hz refresh (spec row 4).
        if (browserPanel_)
            browserPanel_->getRecordPanel().refresh(recorderHost_.status(),
                                                    juce::Time::getMillisecondCounterHiRes() / 1000.0);
    }

    // Repaint input level meter
    if (!inputLevelMeterBounds_.isEmpty())
        repaint(inputLevelMeterBounds_);

    // Refresh inspector at ~10Hz to show signal-driven values
    if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
        inspectorPanel_->refresh();

    // P21: Ableton Link sync — update cached state and feed BPM tracker
    // (s-rta-0926b bpm2: never enabled in a default build -- LinkSync::isAvailable() is false)
    if (linkSync_.isEnabled())
    {
        linkSync_.update();
        double linkBPM = linkSync_.getBPM();
        if (linkBPM > 0.0)
            applyTempoCommand("link", static_cast<float>(linkBPM), Origin::Human);
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

    // s-rta-0927 routine display: the ROUTINES row and the strips' bands follow the routine engine's status
    // (one status() copy per tick, pushed -- never DeckView::refresh()).
    if (deckView_)
    {
        std::vector<juce::String> deckNames, layerNames;
        for (const auto& d : composition_.decks)
            deckNames.push_back(juce::String(d.name));
        if (const auto* deck = composition_.getActiveDeck())
            for (const auto& l : deck->layers)
                layerNames.push_back(juce::String(l.name));
        deckView_->setRoutineView(deriveRoutineDeckView(routineEngine_.status(), composition_.activeDeckIndex,
                                                        deckNames, layerNames));
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
        setFileLabel("Saved: " + fileName);
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
        setFileLabel("Slot " + juce::String(slot + 1) + ": "
                          + file.getFileNameWithoutExtension()
                          + (stats.dropped > 0 ? " (check mappings)" : ""));
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

#if AUDIODNA_HAS_CAMERA
void MainComponent::imageReceived(const juce::Image& image)
{
    // Called from camera thread — queue frame for GL thread
    previewPanel_.queueCameraFrame(image);
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
            setFileLabel("No images found in folder");
            return;
        }

        slideshowIndex_ = 0;
        slideshowBeatCounter_ = 0;
        slideshowBeatCrossings_.reset();

        // Load first image
        auto first = slideshowImages_[0];
        previewPanel_.loadImage(first);
        // s-rta-0928 R1.3: decode the next one ahead (off the GL thread), so the first advance shows at once.
        previewPanel_.getRenderer().prefetchLegacyImage(slideshowImages_[1 % slideshowImages_.size()]);

        setFileLabel("Folder: " + dir.getFileName() + " ("
                          + juce::String(slideshowImages_.size()) + " images)");
    });
}

void MainComponent::advanceSlideshow()
{
    if (slideshowImages_.isEmpty())
        return;

    // Beats since the previous tick: the totalBeatCount delta, never the beatPhase wrap (Pitfall 42)
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    const uint32_t beats = slideshowBeatCrossings_.consume(snap.totalBeatCount);

    if (beats > 0)
    {
        slideshowBeatCounter_ += static_cast<int>(beats);
        if (slideshowBeatCounter_ >= slideshowBeats_)
        {
            slideshowBeatCounter_ = 0;
            slideshowIndex_ = (slideshowIndex_ + 1) % slideshowImages_.size();

            auto img = slideshowImages_[slideshowIndex_];
            previewPanel_.loadImage(img);
            // s-rta-0928 R1.3: decode the next one ahead (off the GL thread).
            previewPanel_.getRenderer().prefetchLegacyImage(
                slideshowImages_[(slideshowIndex_ + 1) % slideshowImages_.size()]);
        }
    }
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
        setFileLabel("Camera failed to open");
        return;
    }

    cameraActive_ = true;

    // Add a listener that receives frames
    cameraDevice_->addListener(this);

    setFileLabel("Camera: " + devices[deviceIndex]);
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
    // Beats since the previous tick: the totalBeatCount delta, never the beatPhase wrap (Pitfall 42)
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    const uint32_t beats = beatCrossings_.consume(snap.totalBeatCount);

    if (beats == 0)
        return;

    // === Global effects randomize (existing behavior) ===
    beatCounter_ += static_cast<int>(beats);
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

void MainComponent::setClipFitMode(int layerIdx, int column, int mode)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck)
        return;
    auto* clip = deck->getClip(layerIdx, column);
    if (!clip)
        return;
    clip->fitMode = ClipFit::clampMode(mode);
    if (inspectorPanel_) inspectorPanel_->refresh();
}

void MainComponent::handleClipTrigger(int layerIndex, int column, Origin origin, int deckIndex, bool immediate)
{
    // s-rta-0923/0924 step 3 (plan section 3.3 B2): deckIndex < 0 means "the
    // active deck" (every pre-existing caller); a Replay dispatch passes the
    // Fired's resolved target deck explicitly, which may not be the active
    // one. Bounds-checked; a non-Human origin failing to resolve is logged
    // (G5's silent early-out stays silent only on the Human path, where the
    // UI already refused the click).
    Deck* deck = nullptr;
    if (deckIndex < 0)
        deck = composition_.getActiveDeck();
    else if (deckIndex < static_cast<int>(composition_.decks.size()))
        deck = &composition_.decks[static_cast<size_t>(deckIndex)];
    if (!deck)
    {
        if (origin != Origin::Human && recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify("handleClipTrigger: deck unresolved");
        return;
    }
    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;

    auto* layer = deck->getLayer(layerIndex);
    if (!layer) return;

    // Undo capture (mutate-then-push, spec §2 row 1 / step 8). The trigger returns
    // the exact before / after pair of the layer's tuple (one compare-exchange,
    // lane tsan); the target cell clip's `playing` is read BEFORE the trigger.
    // Runtime-only writes → NO GL fence; the command re-resolves its target
    // by coordinate. Autopilot never reaches this handler (it calls
    // Layer::triggerClip directly from the GL thread), so autopilot triggers
    // create no commands.
    std::optional<bool> playBefore;
    if (const Clip* tc = layer->getClipAt(column)) playBefore = tc->playing;
    const bool autoPlays = triggerWillAutoPlay(*layer, column);   // captured below (concern (a))

    LayerRuntimeTransition t;
    if (immediate)
    {
        // s-rta-0925 (D4 preamble): the checkpoint-0 restore bypasses beat-snap/quantize entirely --
        // it is putting the model back the way it was at Record, not a performance trigger. Mirrors
        // Layer::triggerClip's own empty-cell branch since triggerClipImmediate
        // alone would leave the active column pointing at an empty cell.
        if (layer->getClipAt(column))
            t = layer->triggerClipImmediate(column);
        else
            t = layer->clearActiveClip();
    }
    else
    {
        // L5 Quantize: the global Quantize control forces a beat-snap granularity
        // on this one trigger (queues it) unless it's Off or the tracker isn't
        // locked yet — see quantizeModeToForcedSnap above.
        const FeatureSnapshot quantizeSnap = analysisThread_.getFeatureBus().read();
        const auto forcedSnap = quantizeModeToForcedSnap(composition_.quantizeMode, quantizeSnap);
        t = layer->triggerClip(column, forcedSnap);
    }

    const LayerRuntimeSnapshot rtBefore = t.before;
    const LayerRuntimeSnapshot rtAfter = t.after;
    // Retrigger-restart (2026-07-30, Boris's recorded expectation): clicking the
    // already-playing cell is the column-already-active case.
    const bool wasRetrigger = (t.before.activeClipColumn == column);
    std::optional<bool> playAfter;
    if (const Clip* tc = layer->getClipAt(column)) playAfter = tc->playing;

    // Preview/deck-view refresh only when the targeted deck is the active
    // one (plan section 3.3 B2) -- a Replay dispatch may target a deck the
    // user isn't currently looking at; the model mutation above still ran
    // regardless.
    if (resolvedDeckIndex == composition_.activeDeckIndex)
    {
    // Load the clip content into preview
    if (auto* clip = layer->getClipAt(t.after.activeClipColumn))
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
            setFileLabel(clip->mediaFile.getFileName());
        }
        else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
        {
            previewPanel_.getRenderer().setActiveSource(clip->sourceType);
            previewPanel_.getRenderer().clearImage();
            setFileLabel(juce::String(clip->sourceType));
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
            setFileLabel(clip->mediaFile.getFileName());
        }
        else if (clip->mediaType == Clip::MediaType::ImageSequence && !clip->sequenceFiles.empty())
        {
            // Ensure image sequence is open for this clip
            auto& renderer = previewPanel_.getRenderer();
            if (!renderer.getImageSequence(clip->id))
                renderer.openImageSequenceForClip(clip->id, clip->sequenceFiles, clip->sequenceFps);

            renderer.clearActiveSource();
            renderer.clearImage();
            auto frameCount = static_cast<int>(clip->sequenceFiles.size());
            setFileLabel(juce::String(clip->name) + " (" + juce::String(frameCount) + " frames)");
        }
    }
    else
    {
        // No active clip — clear preview
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        setFileLabel("");
    }

    if (deckView_)
        deckView_->refresh();
    }

    // Push the trigger command unless it changed nothing (spec §3: retrigger of
    // the already-active cell early-outs into a playhead reset — no runtime and
    // no target-`playing` change → pushes nothing, so no inert history entry).
    // Consecutive same-layer triggers coalesce in UndoManager::perform via
    // TriggerClipCmd::canMergeWith/mergeWith — one history slot per layer run.
    // s-rta-0923/0924 step 3 (plan section 3.3 B2): undo push is gated on
    // Origin::Human -- a Replay dispatch re-runs the same mutation but must
    // never create a new undo entry.
    if (origin == Origin::Human && (!(rtBefore == rtAfter) || playBefore != playAfter))
    {
        std::vector<std::unique_ptr<Command>> children;
        children.push_back(std::make_unique<TriggerClipCmd>(
            makeLayerResolver(), resolvedDeckIndex, layerIndex, column,
            rtBefore, rtAfter, playBefore, playAfter, "Trigger Clip"));
        pushCommands(std::move(children), "Trigger Clip");
    }

    // Capture (plan section 3.3 B2): never reached with Origin::Replay (the
    // host's own capture() also filters on this -- belt and braces).
    if (origin != Origin::Replay)
    {
        ControlPath key = layerScalarPath(composition_, resolvedDeckIndex, layerIndex, "");
        key.control = "activeClip";
        key.scalar.clear();
        DiscretePoint p;
        p.origin = origin;
        p.v = column;
        p.retrigger = wasRetrigger;
        recorderHost_.capture(key, std::move(p));
        if (autoPlays)
            captureAutoPlay(resolvedDeckIndex, layerIndex, column, origin, 0);
    }
}

// s-rta-0926 (routines-1a carried concern (a)): the trigger's auto-play as a `playing` point, right
// after the activeClip point -- so a take (and a routine cut from it) knows the clip was playing.
// "resume" sets playing only (the auto-play never touches reverse); replaying it on a clip that is
// already playing is a no-op.
void MainComponent::captureAutoPlay(int deckIndex, int layerIndex, int column, Origin origin, uint64_t group)
{
    ControlPath key = clipScalarPath(composition_, deckIndex, layerIndex, column, "");
    key.control = "playing";
    key.scalar.clear();
    DiscretePoint p;
    p.origin = origin;
    p.action = "resume";
    p.v = 1;
    p.group = group;
    recorderHost_.capture(key, std::move(p));
}

void MainComponent::handleColumnTrigger(int column, Origin origin, int deckIndex)
{
    Deck* deck = nullptr;
    if (deckIndex < 0)
        deck = composition_.getActiveDeck();
    else if (deckIndex < static_cast<int>(composition_.decks.size()))
        deck = &composition_.decks[static_cast<size_t>(deckIndex)];
    if (!deck)
    {
        if (origin != Origin::Human && recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify("handleColumnTrigger: deck unresolved");
        return;
    }
    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;

    // Undo capture (mutate-then-push, spec §2 row 2 / step 8): a column trigger
    // is a composite of one TriggerClipCmd per NON-ignoring layer that actually
    // changes — mirroring Deck::triggerColumn, which skips ignoreColumnTrigger
    // layers. Each layer's before / after is the exact pair its trigger returned
    // (Deck::triggerColumn's out vector); target-`playing` is read BEFORE.
    const int numLayers = deck->getNumLayers();
    std::vector<std::optional<bool>> playBefore(static_cast<size_t>(numLayers));
    std::vector<bool> considered(static_cast<size_t>(numLayers), false);
    std::vector<bool> autoPlays(static_cast<size_t>(numLayers), false);   // concern (a), see captureAutoPlay
    for (int l = 0; l < numLayers; ++l)
    {
        auto* layer = deck->getLayer(l);
        if (!layer || layer->ignoreColumnTrigger) continue;  // excluded, as triggerColumn does
        considered[static_cast<size_t>(l)] = true;
        autoPlays[static_cast<size_t>(l)] = triggerWillAutoPlay(*layer, column);
        if (const Clip* tc = layer->getClipAt(column))
            playBefore[static_cast<size_t>(l)] = tc->playing;
    }

    // L5 Quantize: same forced-snap decision as handleClipTrigger, applied once
    // for the whole column so every non-ignoring layer queues/fires together.
    const FeatureSnapshot quantizeSnap = analysisThread_.getFeatureBus().read();
    const auto forcedSnap = quantizeModeToForcedSnap(composition_.quantizeMode, quantizeSnap);
    std::vector<std::optional<LayerRuntimeTransition>> transitions;
    deck->triggerColumn(column, forcedSnap, &transitions);

    // One child per considered layer whose runtime or target-`playing` changed;
    // pushCommands composites them into one slot (a single changed layer collapses
    // to a lone TriggerClipCmd, which may then merge into a prior same-layer run —
    // accepted, consistent with spec §3's per-layer merge). Gated on Origin::Human
    // (plan section 3.3 B2) -- a Replay dispatch never creates an undo entry.
    // Capture (one activeClip point per changed layer, shared `group` id) shares
    // the same considered/after loop; never reached with Origin::Replay.
    std::vector<std::unique_ptr<Command>> children;
    const uint64_t captureGroup = (origin != Origin::Replay) ? recorderHost_.nextGroupId() : 0;
    for (int l = 0; l < numLayers; ++l)
    {
        if (!considered[static_cast<size_t>(l)]) continue;
        auto* layer = deck->getLayer(l);
        if (!layer || static_cast<size_t>(l) >= transitions.size() || !transitions[static_cast<size_t>(l)]) continue;
        const LayerRuntimeTransition& t = *transitions[static_cast<size_t>(l)];
        std::optional<bool> playAfter;
        if (const Clip* tc = layer->getClipAt(column))
            playAfter = tc->playing;
        const bool changed = t.changed() || playBefore[static_cast<size_t>(l)] != playAfter;
        if (changed && origin == Origin::Human)
            children.push_back(std::make_unique<TriggerClipCmd>(
                makeLayerResolver(), resolvedDeckIndex, l, column,
                t.before, t.after,
                playBefore[static_cast<size_t>(l)], playAfter, "Trigger Column"));

        if (origin != Origin::Replay)
        {
            ControlPath key = layerScalarPath(composition_, resolvedDeckIndex, l, "");
            key.control = "activeClip";
            key.scalar.clear();
            DiscretePoint p;
            p.origin = origin;
            p.v = column;
            p.group = captureGroup;
            recorderHost_.capture(key, std::move(p));
            if (autoPlays[static_cast<size_t>(l)])
                captureAutoPlay(resolvedDeckIndex, l, column, origin, captureGroup);
        }
    }
    if (origin == Origin::Human)
        pushCommands(std::move(children), "Trigger Column");

    // Preview/deck-view refresh only when the targeted deck is active (plan
    // section 3.3 B2) -- see the identical rule in handleClipTrigger above.
    if (resolvedDeckIndex == composition_.activeDeckIndex)
    {
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
                setFileLabel(clip->mediaFile.getFileName());
                foundActiveClip = true;
                break;
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                setFileLabel(juce::String(clip->sourceType));
                foundActiveClip = true;
                break;
            }
        }
    }

    if (!foundActiveClip)
    {
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        setFileLabel("");
    }
    }
}

// A1 fix (2026-07-30): the previewPanel_ renderer's fallback state
// (activeSourceType_ / loaded image / fileLabel_) is
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
                setFileLabel(clip->mediaFile.getFileName());
                foundActiveClip = true;
                break;
            }
            else if (clip->mediaType == Clip::MediaType::Source && !clip->sourceType.empty())
            {
                previewPanel_.getRenderer().setActiveSource(clip->sourceType, clip->sourceParams);
                previewPanel_.getRenderer().clearImage();
                setFileLabel(juce::String(clip->sourceType));
                foundActiveClip = true;
                break;
            }
        }
    }

    if (!foundActiveClip)
    {
        previewPanel_.getRenderer().clearActiveSource();
        previewPanel_.clearImage();
        setFileLabel("");
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
    // plan6 §6.2: any later command (a clip placed, a tab switch, a trigger) retires the Remove-Deck undo hint --
    // Undo would then no longer mean "un-remove".
    if (deckView_) deckView_->hideUndoHint();
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

std::optional<MainComponent::PreparedDrop>
MainComponent::prepareFileDrop(int layerIndex, int column, const juce::File& file)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return std::nullopt;

    // P24.5: Check content lock before replacing
    if (auto* existing = deck->getClip(layerIndex, column))
    {
        if (existing->contentLocked)
            return std::nullopt; // Silently refuse — locked content
    }

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

    return PreparedDrop{ layerIndex, column, std::move(clip) };
}

std::optional<MainComponent::CellEdit> MainComponent::commitDrop(const PreparedDrop& prepared)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return std::nullopt;

    // Capture before-state for undo (nullopt if the cell was empty).
    std::optional<Clip> before = snapshotCell(deck->getLayer(prepared.layerIndex), prepared.column);
    deck->setClip(prepared.layerIndex, prepared.column, prepared.clip);
    return CellEdit{ prepared.layerIndex, prepared.column, before, std::optional<Clip>(prepared.clip) };
}

void MainComponent::handleFileDrop(int layerIndex, int column, const juce::File& file)
{
    // s-rta-0928b mediaopen: open the media BEFORE the fence (prepareFileDrop); the fence holds only the setClip.
    auto prepared = prepareFileDrop(layerIndex, column, file);
    if (!prepared) return;

    // GL fence (2026-07-28, round 3): commitDrop's deck->setClip
    // call can grow the layer's clips vector (Deck::setClip -> ensureColumns)
    // — the crash-proven reallocation class. One fence for this single-cell
    // drop gesture (mirrors every other single-cell drop handler, e.g.
    // onSourceActivated above). onMultiVideoDropped already fences its own
    // growth+placement loop around commitDrop, so commitDrop itself is
    // NOT fenced internally — that would nest under the loop's outer fence.
    std::optional<CellEdit> edit;
    undoService_.withDeckDetached([&] { edit = commitDrop(*prepared); });
    if (edit)
    {
        pushClipEdits(composition_.activeDeckIndex, { *edit },
                      "Drop '" + juce::String(edit->after->name) + "'");
        if (deckView_)
            deckView_->rebuildGrid();
    }
}

juce::var MainComponent::mediaStateVar() const
{
    // s-rta-0928b mediaopen: /api/state "media" -- MediaPresence's completed sweeps and flags flipped (cumulative).
    auto* obj = new juce::DynamicObject();
    obj->setProperty("presence_sweeps", presence_.sweeps());
    obj->setProperty("presence_changed", presence_.changes());
    return juce::var(obj);
}

juce::var MainComponent::loadWitnessVar() const
{
    // s-rta-0929 asyncload: /api/state "load" (any thread: MediaOpener's atomics, the staged-load mirrors, LoadTiming's
    // mutex-guarded copy). staged_players = players adopted by the staged load, not yet live (video_players counts them).
    auto* obj = new juce::DynamicObject();
    obj->setProperty("opens_pending", mediaOpener_.pending());
    obj->setProperty("open_batches", static_cast<juce::int64>(mediaOpener_.batches()));
    obj->setProperty("opens_stale", static_cast<juce::int64>(mediaOpener_.stale()));
    obj->setProperty("opens_failed", static_cast<juce::int64>(mediaOpener_.failed()));
    obj->setProperty("opens_dropped", static_cast<juce::int64>(mediaOpener_.dropped()));
    obj->setProperty("staged", stagedNow_.load(std::memory_order_relaxed));
    obj->setProperty("staged_players", stagedPlayers_.load(std::memory_order_relaxed));
    obj->setProperty("queued", queuedNow_.load(std::memory_order_relaxed));
    obj->setProperty("timing", loadTiming_.var());
#if AUDIODNA_TEST_SERVER
    // TEST-ONLY audio witnesses (plan R9): CoreAudio's overload count, the callback's inter-arrival gap (reset on read)
    // and period, and the analysis ring's overruns.
    auto& ae = const_cast<AudioEngine&>(audioEngine_);
    obj->setProperty("audio_xruns", audioXruns_.load(std::memory_order_relaxed));
    obj->setProperty("audio_callbacks", static_cast<juce::int64>(ae.audioCallbacks()));
    obj->setProperty("audio_callback_gap_max_ms", ae.takeAudioGapMaxMs());
    const double rate = ae.sourceSampleRateCell().load(std::memory_order_acquire);
    const int period = ae.audioPeriodSamples();
    obj->setProperty("audio_callback_period_ms", rate > 0.0 ? 1000.0 * period / rate : 0.0);
    obj->setProperty("analysis_ring_overruns", static_cast<juce::int64>(ae.ringOverruns()));
#endif
    return juce::var(obj);
}

void MainComponent::debugDropFiles(int layerIndex, int column, const std::vector<juce::File>& files)
{
    if (!deckView_)
        return;
    juce::StringArray paths;
    for (const auto& f : files)
        paths.add(f.getFullPathName());
    ClipCell::dispatchDrop(ClipCell::classifyDrop(paths), layerIndex, column, deckView_->onFileDropped,
                           deckView_->onMultiFileDropped, deckView_->onMultiVideoDropped, deckView_->onMixedFilesDropped);
}

std::optional<MainComponent::PreparedDrop>
MainComponent::prepareMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
    auto* deck = composition_.getActiveDeck();
    if (!deck) return std::nullopt;

    // P24.5: Check content lock before replacing (mirrors prepareFileDrop —
    // this check was missing here, letting a multi-image drop silently
    // overwrite a content-locked cell).
    if (auto* existing = deck->getClip(layerIndex, column))
    {
        if (existing->contentLocked)
            return std::nullopt; // Silently refuse — locked content
    }

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
        // s-rta-0928b mediaopen: no frame-0 decode for a thumbnail here -- the grid pulls it from ClipThumbnails.
    }

    // Open the image sequence in the renderer
    auto& renderer = previewPanel_.getRenderer();
    renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);

    return PreparedDrop{ layerIndex, column, std::move(clip) };
}

void MainComponent::handleMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files)
{
    // Boris ruling 2026-08-04: exactly 2 images spread across 2 cells instead
    // of merging into one ImageSequence (the surprise that triggered this
    // fix — two dropped images must read as two clips, not one animation).
    // This handler is reached both by a direct Finder drop of images only
    // (no video — ClipCell::filesDropped) and by the internal "files:" drag
    // path when videos.empty() (ClipCell::itemDropped) — 3+ still falls
    // through to prepareMultiFileDrop below, mirroring onMixedFilesDropped's
    // threshold so every drop path agrees.
    if (files.size() == 2)
    {
        auto* deck = composition_.getActiveDeck();
        if (!deck) return;
        const int numColsBefore = deck->numColumns;

        // s-rta-0928b mediaopen: both prepared BEFORE the fence; the fence holds growth + the two setClips.
        std::vector<PreparedDrop> prepared;
        for (int i = 0; i < 2; ++i)
            if (auto p = prepareFileDrop(layerIndex, column + i, files[static_cast<size_t>(i)]))
                prepared.push_back(std::move(*p));

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
            for (const auto& p : prepared)
                if (auto edit = commitDrop(p))
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

    // s-rta-0928b mediaopen: the sequence is opened BEFORE the fence (prepareMultiFileDrop).
    auto prepared = prepareMultiFileDrop(layerIndex, column, files);
    if (!prepared) return;

    // GL fence (2026-07-28, round 3): setClip's internal ensureColumns can
    // grow the layer's clips vector — the crash-proven reallocation class.
    std::optional<CellEdit> edit;
    undoService_.withDeckDetached([&] { edit = commitDrop(*prepared); });
    if (!edit) return;

    pushClipEdits(composition_.activeDeckIndex, { *edit },
                  "Drop '" + juce::String(edit->after->name) + "'");

    if (deckView_)
        deckView_->rebuildGrid();
}

void MainComponent::handleDeckSwitch(int deckIndex, Origin origin)
{
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_.decks.size()))
    {
        if (origin != Origin::Human && recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify("handleDeckSwitch: deck unresolved");
        return;
    }

    // s-rta-0923/0924 step 3 (plan section 3.3 B2): capture only on an
    // ACTUAL change -- a same-deck no-op switch records nothing. handleDeckSwitch
    // never pushes an undo command itself (deckView_->onDeckSwitched, the user
    // entry point, does that) so there is nothing to gate on origin here beyond
    // the capture call, which the host also filters (never Origin::Replay).
    const bool actualChange = deckIndex != composition_.activeDeckIndex;
    if (actualChange && origin != Origin::Replay)
    {
        ControlPath key;
        key.scope = ControlPath::Scope::Comp;
        key.control = "activeDeck";
        DiscretePoint p;
        p.origin = origin;
        p.v = deckIndex;
        recorderHost_.capture(key, std::move(p));
    }

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

// s-rta-0923/0924 step 3 (Lane S3-B, plan section 3.3 B3, D6b): the five
// choke points every writer of a discrete control funnels through. Each
// captures a point only when origin != Origin::Replay (the host's own
// capture() filters on this too -- belt and braces, plan text).

// s-rta-0925 (D4 preamble, plan section 3.5 item 3): shared deck resolution for the four
// dispatch-table handlers below -- the SAME B2 pattern handleClipTrigger already uses
// (MainComponent.cpp:4090-4100).
Deck* MainComponent::deckForDispatch(int deckIndex, const char* who, Origin origin)
{
    Deck* deck = nullptr;
    if (deckIndex < 0)
        deck = composition_.getActiveDeck();
    else if (deckIndex < static_cast<int>(composition_.decks.size()))
        deck = &composition_.decks[static_cast<size_t>(deckIndex)];
    if (!deck && origin != Origin::Human && recorderHost_.dispatch.notify)
        recorderHost_.dispatch.notify(std::string(who) + ": deck unresolved");
    return deck;
}

void MainComponent::applyClearActiveClip(int layerIndex, Origin origin, int deckIndex)
{
    auto* deck = deckForDispatch(deckIndex, "applyClearActiveClip", origin);
    if (!deck) return;
    auto* layer = deck->getLayer(layerIndex);
    // One load for the early-out; a GL-thread transition landing before the clear can only make a clip active (a fade
    // tick, autopilot, a fired queued trigger -- never a clear), so the clear below is still the dispatched intent.
    if (!layer || layer->runtime().activeClipColumn < 0) return;

    layer->clearActiveClip();
    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;
    if (resolvedDeckIndex == composition_.activeDeckIndex)
    {
        previewPanel_.getRenderer().setActiveDeck(composition_.getActiveDeck());
        if (deckView_) deckView_->refresh();
    }

    if (origin != Origin::Replay)
    {
        ControlPath key = layerScalarPath(composition_, resolvedDeckIndex, layerIndex, "");
        key.control = "activeClip";
        key.scalar.clear();
        DiscretePoint p;
        p.origin = origin;
        p.v = -1;
        recorderHost_.capture(key, std::move(p));
    }
}

// Dispatch table (critic N10) -- a MOVE of each site's existing body, not a
// behaviour change: "tap" calls the tracker's BPM setter only (TopBar's own
// prior behaviour); "manual" turns manual mode on and applies the BPM if
// positive; "auto" turns manual mode off; "resync" requests a Resync that the
// analysis thread applies (s-rta-0925: BPMTracker::requestResync(), no longer
// a direct message-thread write into the tracker); "link" (also REST/OSC
// set_bpm) turns manual mode on and applies the BPM.
// s-rta-0926b: setManualBPM / followExternalTempo are requests too now -- this
// message-thread function writes nothing the analysis thread owns; the tracker
// applies the tempo at the start of its next hop (~10.7 ms), which is when the
// old direct write first reached the published FeatureSnapshot anyway.
// s-rta-0926b plan3 A: a tempo VALUE ("manual", "link") never realigns the beat
// (followExternalTempo); only the beat gestures do -- "tap" (setManualBPM) and "resync".
void MainComponent::applyTempoCommand(const std::string& action, float bpm, Origin origin)
{
    auto* tracker = analysisThread_.getBpmTracker();
    if (action == "tap")
    {
        if (tracker) tracker->setManualBPM(bpm);
    }
    else if (action == "manual")
    {
        if (tracker)
        {
            tracker->setManualMode(true);
            if (bpm > 0.0f) tracker->followExternalTempo(bpm);
        }
    }
    else if (action == "auto")
    {
        if (tracker) tracker->setManualMode(false);
    }
    else if (action == "resync")
    {
        beatCounter_ = 0;
        beatCrossings_.reset();
        if (tracker) tracker->requestResync();
    }
    else if (action == "link")
    {
        if (tracker)
        {
            tracker->setManualMode(true);
            tracker->followExternalTempo(bpm);   // Link, REST/OSC set_bpm, a replayed value: never realigns
        }
    }

    if (origin == Origin::Replay) return;

    // R3 (critic B1): only capture a Link tick's tempo point when the BPM
    // actually moved by a meaningful amount -- Link ticks at ~30Hz.
    if (action == "link")
    {
        if (std::abs(bpm - lastLinkCapturedBpm_) < 0.01f) return;
        lastLinkCapturedBpm_ = bpm;
    }

    ControlPath key;
    key.scope = ControlPath::Scope::Comp;
    key.control = "tempo";
    DiscretePoint p;
    p.origin = origin;
    p.action = action;
    // Centi-BPM (plan section 3.3 B3): 0 for auto/resync, which carry no BPM.
    p.v = (action == "auto" || action == "resync") ? 0 : juce::roundToInt(bpm * 100.0f);
    recorderHost_.capture(key, std::move(p));
}

// ---------------------------------------------------------------------------
// s-rta-0924b step 4 (Lane S4-B): the perf* funnel -- ONE path for REST
// (/api/perf/*) and the Record panel. Bodies moved from the former
// apiServer_->onPerf* lambdas; each failure branch now also RETURNS the text it
// notifies ("" = success). Message thread only (RecorderHost asserts it).
// ---------------------------------------------------------------------------

juce::File MainComponent::takesRoot()
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
               .getChildFile("Audio-DNA").getChildFile("Takes");
}

void MainComponent::setAudioSourceModeSynced(AudioEngine::SourceMode mode)
{
    // A programmatic source-mode switch keeps BOTH "Audio" selectors honest
    // (dontSendNotification: their onChange handlers would prompt for a file).
    audioEngine_.setSourceMode(mode);
    const int id = (mode == AudioEngine::SourceMode::File) ? 2 : 1;
    audioSourceSelector_.setSelectedId(id, juce::dontSendNotification);
    if (topBar_)
        topBar_->getAudioSourceSelector().setSelectedId(id, juce::dontSendNotification);
    if (mode == AudioEngine::SourceMode::MicInput)
        setFileLabel("Mic: " + audioEngine_.getDeviceStatus());
    else if (currentAudioFile_.existsAsFile())
        setFileLabel(currentAudioFile_.getFileName());
}

std::string MainComponent::perfRecord(const ApiServer::PerfRecordOpts& opts)
{
    if (recorderHost_.isRecording())
    {
        const std::string msg = "A take is already recording.";
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }

    // A5(c)/N8: switch to File mode only if not already there, and read
    // deviceRate/channels AFTER the switch (a mode change can restart
    // the device at a different rate/channel count).
    if (opts.audioFile.isNotEmpty())
    {
        juce::File f(opts.audioFile);
        if (f.existsAsFile() && audioEngine_.loadFile(f))
            currentAudioFile_ = f;
        if (audioEngine_.getSourceMode() != AudioEngine::SourceMode::File)
            setAudioSourceModeSynced(AudioEngine::SourceMode::File);
    }

    RecorderHost::ArmOptions armOpts;
    juce::String name = opts.name.isNotEmpty()
        ? opts.name
        : juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H%M%S");
    armOpts.takeFolder = takesRoot().getChildFile(name + ".adna-take");
    armOpts.audio = opts.audio;
    armOpts.audioMode = (audioEngine_.getSourceMode() == AudioEngine::SourceMode::File) ? "file" : "input";
    armOpts.deviceRate = audioEngine_.getCurrentSampleRate();
    // s-rta-0929b btguard (BG7): no audio device (the device policy found nothing allowed) = rate 0 and no channels --
    // nothing to capture, and a 0 must never size the tap's writer: the take records without audio.
    if (armOpts.deviceRate <= 0.0)
        armOpts.audio = false;
    if (auto* dev = audioEngine_.getDeviceManager().getCurrentAudioDevice())
        armOpts.deviceChannels = dev->getActiveOutputChannels().countNumberOfSetBits();
    armOpts.appVersion = juce::JUCEApplication::getInstance()
        ? juce::JUCEApplication::getInstance()->getApplicationVersion().toStdString()
        : "0.1.0";
    armOpts.onsetMarkers = opts.onsetMarkers;
    if (opts.overdubAssetId.isNotEmpty())
        armOpts.overdubAssetId = opts.overdubAssetId.toStdString();
    armOpts.gripHoldMs = composition_.gripHoldMs;
    // s-rta-0928 take start (Pitfall 48): every tempo / Tap / Resync / manual-mode command that ran before this
    // Record is a BPMTracker request the analysis thread applies at its next hop. The take's t = 0 -- and
    // where beat 0 sits in its bar (plan-routines 3.6), read from the same snapshot -- waits for the
    // snapshot that carries them (RecorderHost::tick). Test mode never starts the analysis
    // thread (the TestServer writes the bus): nothing to wait for there.
    if (!testMode_)
        if (auto* tracker = analysisThread_.getBpmTracker())
            armOpts.startAfterTrackerRequest = tracker->postedRequestSeq();

    auto result = recorderHost_.arm(composition_, audioEngine_.getAudioTap(), armOpts);
    if (!result.ok)
    {
        const std::string msg = "Could not start the take: " + result.error;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }
    if (opts.audioFile.isNotEmpty())
        applyAudioTransport("play", Origin::Human);
    return {};
}

std::string MainComponent::perfStop()
{
    auto result = recorderHost_.disarm(composition_, audioEngine_.getAudioTap());
    if (!result.ok)
    {
        const std::string msg = result.error == "not recording"
            ? std::string("Nothing is recording.")
            : "Could not stop the take: " + result.error;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }
    // s-rta-0924b step-4 fix plan F3 (plan 4.5 B1 #6): say where the take went. A finalize problem
    // the host already notified is kept in the same line, so "Saved" never hides it.
    if (recorderHost_.dispatch.notify)
    {
        std::string saved = "Saved: " + result.takeFolder.getFileNameWithoutExtension().toStdString();
        if (!result.error.empty())
            saved += ", but its audio had a problem: " + result.error;
        recorderHost_.dispatch.notify(saved);
    }
    return {};
}

std::string MainComponent::perfLoad(const juce::File& takeFolder)
{
    auto result = recorderHost_.load(takeFolder);
    if (!result.ok)
    {
        const std::string msg = "Could not load the take: " + result.error;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }
    return {};
}

std::string MainComponent::perfPlay(bool withAudio)
{
    auto mode = withAudio ? RecorderHost::PlayMode::WithAudio : RecorderHost::PlayMode::WallClock;
    auto result = recorderHost_.play(mode, composition_);
    if (!result.ok)
    {
        const std::string msg = result.error == "no take loaded"
            ? std::string("No take is loaded. Use Load Take... first.")
            : "Could not play: " + result.error;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }
    if (withAudio && result.wav.existsAsFile())
    {
        if (audioEngine_.loadFile(result.wav))
        {
            currentAudioFile_ = result.wav;
            // Remember the input to go back to at Stop Playback (plan 5 #5).
            if (!sourceModeBeforeReplay_)
                sourceModeBeforeReplay_ = audioEngine_.getSourceMode();
            if (audioEngine_.getSourceMode() != AudioEngine::SourceMode::File)
                setAudioSourceModeSynced(AudioEngine::SourceMode::File);
            applyAudioTransport("play", Origin::Human);
        }
    }

    // s-rta-0925 (D4 preamble, Boris ruling 2026-09-25): one plain notice -- Play restores the
    // checkpoint-0 look before playing the recorded moves (F3: whole words, no jargon, never a modal).
    if (recorderHost_.dispatch.notify)
    {
        const auto st = recorderHost_.status();
        const juce::File takeFolder(st.loadedTakeFolder);
        std::string msg = "Playing " + takeFolder.getFileNameWithoutExtension().toStdString()
            + ": the look from when Record was pressed is restored first.";
        if (st.preambleUnresolved > 0)
            msg += ", but " + std::to_string(st.preambleUnresolved)
                 + " settings could not be restored (a layer, deck or clip no longer exists)";
        if (st.preambleRefused > 0)
            msg += ", and " + std::to_string(st.preambleRefused)
                 + " controls you are holding were left alone";
        recorderHost_.dispatch.notify(msg);
    }
    return {};
}

std::string MainComponent::perfStopPlay()
{
    std::string msg;
    // Harmony ruling 1 (s-rta-0924b): an overdub's clock IS the replayed audio's
    // transport, so Stop Playback ends a recording overdub first (disarm:
    // finalize + save). Overdub is then clear, so the transport stop below is
    // no longer refused by applyAudioTransport's R-A8 guard -- the audio really
    // stops. A plain recording keeps running.
    const auto stopped = recorderHost_.stopPlayback(composition_, audioEngine_.getAudioTap());
    if (stopped.overdubStopped)
    {
        const std::string note = stopped.overdub.ok
            ? "Stopped the playback and the take recorded over it. Saved: "
                  + stopped.overdub.takeFolder.getFileNameWithoutExtension().toStdString()
            : "Stopped the playback, but the take recorded over it could not be saved: " + stopped.overdub.error;
        if (!stopped.overdub.ok)
            msg = note;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(note);
    }

    applyAudioTransport("stop", Origin::Human);
    restoreInputAfterReplay();
    return msg;
}

// The input that was active before a play-with-audio comes back -- Stop Playback and the natural end
// of the take (onReplayFinished) share this. Never while a take is still recording (a mode change can
// restart the device and re-prepare the tap mid-take); the pending mode is dropped either way
// (unchanged behaviour from perfStopPlay's original tail).
void MainComponent::restoreInputAfterReplay()
{
    if (sourceModeBeforeReplay_ && !recorderHost_.isRecording())
        setAudioSourceModeSynced(*sourceModeBeforeReplay_);
    sourceModeBeforeReplay_.reset();
}

// s-rta-0925 end-of-replay (Boris ruling 2026-09-25 "hold, don't stop", binding-decisions.md). The
// host has already pinned the replay at its end and stopped the Player; the look stays exactly as it
// is. Nothing is reset here. Three app-level things:
void MainComponent::onReplayFinished()
{
    const auto st = recorderHost_.status();   // published by the host BEFORE this callback: finished == true
    // (1) A take recording over this audio: its clock IS the transport, which has just ended (5.2) --
    //     end and save it exactly as Stop Recording would (Harmony ruling 1's reasoning), never let it
    //     stamp against a frozen clock.
    if (st.recording && st.overdub)
        perfStop();
    // (2) The input comes back. Only a with-audio replay set sourceModeBeforeReplay_; wall-clock never
    //     touched the input.
    const bool wasWithAudio = sourceModeBeforeReplay_.has_value();
    restoreInputAfterReplay();
    // (3) One plain notice (whole words, never a modal).
    if (recorderHost_.dispatch.notify)
    {
        const std::string name = juce::File(st.loadedTakeFolder).getFileNameWithoutExtension().toStdString();
        std::string msg = "Finished " + name + ": holding the last look.";
        if (wasWithAudio && audioEngine_.getSourceMode() == AudioEngine::SourceMode::MicInput)
            msg += " Listening to the live input again.";
        recorderHost_.dispatch.notify(msg);
    }
}

std::string MainComponent::perfRepair()
{
    auto err = recorderHost_.repairLoadedAudio(
        juce::JUCEApplication::getInstance()
            ? juce::JUCEApplication::getInstance()->getApplicationVersion().toStdString()
            : "0.1.0");
    if (!err.empty())
    {
        const std::string msg = "Could not repair the audio: " + err;
        if (recorderHost_.dispatch.notify)
            recorderHost_.dispatch.notify(msg);
        return msg;
    }
    return {};
}

juce::var MainComponent::perfStatusVar() const
{
    const auto s = recorderHost_.status();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("recording", s.recording);
    obj->setProperty("playing", s.playing);
    obj->setProperty("overdub", s.overdub);
    obj->setProperty("takeFolder", juce::String(s.takeFolder));
    obj->setProperty("assetId", juce::String(s.assetId));
    obj->setProperty("audioMode", juce::String(s.audioMode));
    obj->setProperty("playMode", juce::String(s.playMode));
    obj->setProperty("lastError", juce::String(s.lastError));
    obj->setProperty("lastFinalizeError", juce::String(s.lastFinalizeError));   // s-rta-0924b
    obj->setProperty("finalizeErrors", s.finalizeErrors);                        // s-rta-0924b (never reset)
    obj->setProperty("t", s.t);
    obj->setProperty("beat", s.beat);
    obj->setProperty("sample", static_cast<juce::int64>(s.sample));
    obj->setProperty("bpm", static_cast<double>(s.bpm));
    obj->setProperty("lanes", s.lanes);
    obj->setProperty("points", s.points);
    obj->setProperty("gestures", s.gestures);
    obj->setProperty("markers", s.markers);
    obj->setProperty("gapDetection", s.gapDetection);
    obj->setProperty("framesWritten", static_cast<juce::int64>(s.framesWritten));
    obj->setProperty("gaps", s.gaps);
    obj->setProperty("position", s.position);
    obj->setProperty("length", s.length);
    obj->setProperty("unresolved", s.unresolved);
    obj->setProperty("reboundByPosition", s.reboundByPosition);
    obj->setProperty("reboundByName", s.reboundByName);
    obj->setProperty("invalid", s.invalid);
    obj->setProperty("skipped", s.skipped);
    obj->setProperty("continuousUnavailable", s.continuousUnavailable);
    obj->setProperty("refusedByHand", s.refusedByHand);
    obj->setProperty("audioStatus", juce::String(s.audioStatus));
    obj->setProperty("deviceRate", s.deviceRate);
    // R13-D: rateChangedSinceArm replaces rateMismatch as the published
    // JSON key (the deprecated RecorderHost::Status::rateMismatch mirror
    // was removed once its only caller -- pre-lane-D MainComponent.cpp --
    // was gone; s-rta-0924 cleanup lane). sourceSampleRate
    // mirrors deviceRate here -- onPerfStatus reads ONLY the mutex-
    // guarded Status copy (critic A5(b)/N3), and the analysis thread
    // always resamples to its own fixed internal 48 kHz, so deviceRate
    // IS the "source" rate at the recorder/provenance level.
    obj->setProperty("rateChangedSinceArm", s.rateChangedSinceArm);
    obj->setProperty("sourceSampleRate", s.deviceRate);
    obj->setProperty("humanRefused", s.humanRefused);
    // s-rta-0924b step 4 (S4-A, additive): loaded-take facts and the playback
    // position in seconds -- the Record panel's own inputs, published here too.
    obj->setProperty("loadedTakeFolder", juce::String(s.loadedTakeFolder));
    obj->setProperty("loadedRecordedAt", juce::String(s.loadedRecordedAt));
    obj->setProperty("loadedDuration", s.loadedDuration);
    obj->setProperty("loadedLanes", s.loadedLanes);
    obj->setProperty("loadedAssetId", juce::String(s.loadedAssetId));
    obj->setProperty("audioReason", juce::String(s.audioReason));
    obj->setProperty("positionSeconds", s.positionSeconds);
    obj->setProperty("lengthSeconds", s.lengthSeconds);
    // s-rta-0925 (D4 preamble): the checkpoint-0 restore fired at Play -- all four are 0 while not
    // playing (RecorderHost::publishStatus only fills them inside its `playing_ && player_` block).
    obj->setProperty("preambleCount", s.preambleCount);
    obj->setProperty("preambleFired", s.preambleFired);
    obj->setProperty("preambleRefused", s.preambleRefused);
    obj->setProperty("preambleUnresolved", s.preambleUnresolved);
    // s-rta-0925 end-of-replay (Boris ruling 2026-09-25 "hold, don't stop"): finished is 0 while not
    // playing (RecorderHost::publishStatus only fills it inside its `playing_ && player_` block).
    obj->setProperty("finished", s.finished);
    // s-rta-0925: the additive exception to "reads ONLY recorderHost_.status()" -- inputSourceMirror_
    // is a relaxed atomic written once per tick on the message thread (MainComponent.h's own comment).
    obj->setProperty("inputSource", inputSourceMirror_.load(std::memory_order_relaxed) == 1 ? "file" : "input");
    return juce::var(obj);
}

// === s-rta-0926 routines slice 1 (plan-routines-s1-final.md 5.1-5.3): the routine funnel ===
// REST, OSC and bindings all land here, on the message thread. Each returns "" or the refusal text;
// a refusal also goes to /api/routine/status lastError and to the notice line.

std::string MainComponent::perfRoutineSave(const ApiServer::RoutineSaveOpts& opts)
{
    auto refuse = [this](const std::string& msg) {
        routineEngine_.setLastError(msg);
        if (routineEngine_.dispatch.notify)
            routineEngine_.dispatch.notify(msg);
        return msg;
    };

    // The take: a named folder, else the one loaded with Load Take.
    std::optional<Take> fromFolder;
    const Take* take = nullptr;
    std::string takeFolder;
    if (opts.takeFolder.isNotEmpty())
    {
        LoadStats stats;
        fromFolder = Take::load(juce::File(opts.takeFolder), stats);
        if (!fromFolder)
            return refuse("Could not read the take at " + opts.takeFolder.toStdString()
                          + (stats.refusalReason.empty() ? std::string(".") : ": " + stats.refusalReason));
        take = &*fromFolder;
        takeFolder = opts.takeFolder.toStdString();
    }
    else
    {
        take = recorderHost_.loadedTake();
        takeFolder = recorderHost_.status().loadedTakeFolder;
    }
    if (take == nullptr)
        return refuse("No take is loaded. Use Load Take... first.");

    // The range: take beats, or 1-based inclusive bars on the take's own bar grid.
    const bool barsUnknown = opts.useBars && take->meta.startBeatInBar < 0.0;
    SliceRequest req;
    req.fromBeat = opts.useBars ? takeBeatOfBar(*take, opts.fromBar) : opts.fromBeat;
    req.toBeat = opts.useBars ? takeBeatOfBar(*take, opts.toBar + 1) : opts.toBeat;
    req.wholeBars = opts.wholeBars.value_or(true);
    req.takeFolder = takeFolder;

    // The pad: the one asked for (an occupied pad is replaced), else the first free one.
    const int slot = opts.slot >= 0 ? opts.slot : composition_.firstFreeRoutineSlot();
    if (opts.slot >= Composition::kRoutineBankSize)
        return refuse("There is no routine pad " + std::to_string(opts.slot + 1) + "; the pads are 1 to "
                      + std::to_string(Composition::kRoutineBankSize) + ".");
    if (slot < 0)
        return refuse("The routine bank is full. Remove a routine or choose a pad to replace.");
    req.name = opts.name.isNotEmpty() ? opts.name.toStdString() : "Routine " + std::to_string(slot + 1);

    SliceResult cut = sliceRoutine(*take, req, previewPanel_.getRenderer().getEffectLibrary());
    if (!cut.error.empty() || !cut.routine)
        return refuse(RoutineEngine::saveRefusalText(cut.error));

    Routine routine = std::move(*cut.routine);
    if (opts.loop)         routine.loop = *opts.loop;
    if (opts.restoreState) routine.restoreState = *opts.restoreState;
    if (opts.quantize.isNotEmpty())
        routine.quantize = Routine::quantizeFromString(opts.quantize);

    std::string replaced;
    if (const Routine* old = composition_.routineInSlot(slot))
    {
        replaced = old->name;
        routineEngine_.stop(slot);   // its Player holds the old routine's program
    }
    const std::string uuid = routine.uuid;
    const std::string name = routine.name;
    const int lanes = static_cast<int>(routine.lanes.size());
    const int restores = static_cast<int>(routine.preamble.size());
    composition_.routines.push_back(std::move(routine));
    if (!composition_.assignRoutineSlot(slot, uuid))
        return refuse("Could not put the routine on pad " + std::to_string(slot + 1) + ".");

    RoutineEngine::Status::LastSaved saved;
    saved.slot = slot;
    saved.uuid = uuid;
    saved.name = name;
    saved.lanes = lanes;
    saved.preambleEntries = restores;
    saved.preambleUnknown = cut.preambleUnknown;
    saved.dropped = cut.droppedLanes;
    routineEngine_.setLastSaved(saved);

    std::string msg = "Saved routine " + name + " to pad " + std::to_string(slot + 1) + ": "
                    + std::to_string(lanes) + (lanes == 1 ? " timeline, " : " timelines, ")
                    + std::to_string(restores) + (restores == 1 ? " restore" : " restores");
    if (!cut.droppedLanes.empty())
    {
        msg += ", dropped: ";
        for (size_t i = 0; i < cut.droppedLanes.size(); ++i)
            msg += (i > 0 ? ", " : "") + cut.droppedLanes[i];
    }
    if (!replaced.empty())
        msg += " (replaced " + replaced + ")";
    if (barsUnknown)
        msg += "; bars counted from the start of the take";
    if (routineEngine_.dispatch.notify)
        routineEngine_.dispatch.notify(msg);
    return {};
}

std::string MainComponent::perfRoutineFire(int slot)
{
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    return routineEngine_.fire(composition_, slot,
                               static_cast<RoutineSnap>(quantizeModeToForcedSnap(composition_.quantizeMode, snap)),
                               snap.trackerState == BPMTracker::STATE_LOCKED && snap.bpm > 0.0f);
}

std::string MainComponent::perfRoutineStop(int slot, bool all)
{
    if (all)
        routineEngine_.stopAll();
    else
        routineEngine_.stop(slot);
    return {};
}

std::string MainComponent::perfRoutineSet(const ApiServer::RoutineSetOpts& opts)
{
    Routine* routine = composition_.routineInSlot(opts.slot);
    if (routine == nullptr)
    {
        const std::string msg = "Routine pad " + std::to_string(opts.slot + 1) + " is empty.";
        routineEngine_.setLastError(msg);
        if (routineEngine_.dispatch.notify)
            routineEngine_.dispatch.notify(msg);
        return msg;
    }
    // A running routine picks up loop / restore at its next end, quantize at its next (re)start,
    // the restore style (Ease / Jump) when it next plans a restore (a fire, a re-fire, or the last beat
    // of a loop), the name at once (the bank listing is re-read every tick).
    if (opts.loop)         routine->loop = *opts.loop;
    if (opts.restoreState) routine->restoreState = *opts.restoreState;
    if (opts.restoreStyle.isNotEmpty())
        routine->restoreStyle = Routine::restoreStyleFromString(opts.restoreStyle);
    if (opts.quantize.isNotEmpty())
        routine->quantize = Routine::quantizeFromString(opts.quantize);
    if (opts.name.isNotEmpty())
        routine->name = opts.name.toStdString();
    if (routineEngine_.dispatch.notify)
    {
        const char* when = "on the next bar";
        switch (routine->quantize)
        {
            case Clip::BeatSnapMode::Off:     when = "at once"; break;
            case Clip::BeatSnapMode::Beat:    when = "on the next beat"; break;
            case Clip::BeatSnapMode::Bar:     when = "on the next bar"; break;
            case Clip::BeatSnapMode::TwoBar:  when = "on the next two-bar line"; break;
            case Clip::BeatSnapMode::FourBar: when = "on the next four-bar line"; break;
        }
        routineEngine_.dispatch.notify("Routine " + routine->name + ": "
                                       + (routine->loop ? "loops" : "plays once") + ", "
                                       + (routine->restoreState
                                              ? (routine->restoreStyle == Routine::RestoreStyle::Jump
                                                     ? "restores first (jump)" : "restores first (ease)")
                                              : "starts from now")
                                       + ", starts " + when);
    }
    return {};
}

std::string MainComponent::perfRoutineRemove(int slot)
{
    routineEngine_.stop(slot);
    if (!composition_.removeRoutineSlot(slot))
    {
        const std::string msg = "Routine pad " + std::to_string(slot + 1) + " is empty.";
        routineEngine_.setLastError(msg);
        if (routineEngine_.dispatch.notify)
            routineEngine_.dispatch.notify(msg);
        return msg;
    }
    if (routineEngine_.dispatch.notify)
        routineEngine_.dispatch.notify("Removed the routine on pad " + std::to_string(slot + 1) + ".");
    return {};
}

void MainComponent::renameRoutine(int slot)
{
    const Routine* routine = composition_.routineInSlot(slot);
    if (routine == nullptr)
        return;
    const juce::String oldName(routine->name);
    const std::string uuid = routine->uuid;

    // The renameDeck idiom: the app LookAndFeel is the default, so the window is square with a bold title.
    auto* w = new juce::AlertWindow("Rename Routine", "", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor("name", oldName);
    w->addButton("Rename", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    // deleteWhenDismissed = true: ModalComponentManager runs this callback BEFORE it deletes the window.
    w->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, slot, w, oldName, uuid](int result) {
            const auto text = w->getTextEditorContents("name").trim();
            const Routine* now = composition_.routineInSlot(slot);
            if (result != 1 || text.isEmpty() || text == oldName || now == nullptr || now->uuid != uuid)
                return;
            ApiServer::RoutineSetOpts o;
            o.slot = slot;
            o.name = text;
            perfRoutineSet(o);   // the name changes at once (the bank listing is re-read every tick)
        }), true);
}

void MainComponent::deleteRoutine(int slot)
{
    const Routine* routine = composition_.routineInSlot(slot);
    if (routine == nullptr)
        return;
    const juce::String name = routine->name.empty() ? "Routine " + juce::String(slot + 1) : juce::String(routine->name);
    const std::string uuid = routine->uuid;

    // A destructive library action sits behind a confirm (plan6); "Remove from layers" is the non-destructive one.
    auto* w = new juce::AlertWindow("Delete Routine",
                                    "Delete routine \"" + name + "\" from this show? This cannot be undone.",
                                    juce::MessageBoxIconType::NoIcon);
    w->addButton("Delete", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    w->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, slot, uuid](int result) {
            const Routine* now = composition_.routineInSlot(slot);
            if (result != 1 || now == nullptr || now->uuid != uuid)
                return;
            perfRoutineRemove(slot);   // stops it, then erases it from the composition
        }), true);
}

juce::var MainComponent::routineStatusVar() const
{
    const auto s = routineEngine_.status();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("clockBeat", s.clockBeat);
    obj->setProperty("beatAvailable", s.beatAvailable);
    obj->setProperty("fires", s.fires);
    obj->setProperty("lastError", juce::String(s.lastError));

    auto* saved = new juce::DynamicObject();
    saved->setProperty("slot", s.lastSaved.slot);
    saved->setProperty("uuid", juce::String(s.lastSaved.uuid));
    saved->setProperty("name", juce::String(s.lastSaved.name));
    saved->setProperty("lanes", s.lastSaved.lanes);
    saved->setProperty("preambleEntries", s.lastSaved.preambleEntries);
    saved->setProperty("preambleUnknown", s.lastSaved.preambleUnknown);
    juce::Array<juce::var> dropped;
    for (const auto& d : s.lastSaved.dropped)
        dropped.add(juce::String(d));
    saved->setProperty("dropped", dropped);
    obj->setProperty("lastSaved", juce::var(saved));

    juce::Array<juce::var> bank;
    for (const auto& sl : s.slots)
    {
        auto* p = new juce::DynamicObject();
        p->setProperty("slot", sl.slot);
        p->setProperty("uuid", juce::String(sl.uuid));
        p->setProperty("name", juce::String(sl.name));
        p->setProperty("lengthBeats", sl.lengthBeats);
        p->setProperty("loop", sl.loop);
        p->setProperty("restoreState", sl.restoreState);
        p->setProperty("restoreStyle", juce::String(sl.restoreStyle));   // s-rta-0926b: "ease" | "jump"
        p->setProperty("quantize", juce::String(sl.quantize));
        p->setProperty("lanes", sl.lanes);
        p->setProperty("preambleEntries", sl.preambleEntries);
        p->setProperty("state", juce::String(sl.state));
        p->setProperty("position", sl.position);
        p->setProperty("cycle", sl.cycle);
        p->setProperty("restarts", sl.restarts);
        p->setProperty("startedTotalBar", static_cast<juce::int64>(sl.startedTotalBar));
        p->setProperty("unresolved", sl.unresolved);
        p->setProperty("reboundByPosition", sl.reboundByPosition);
        p->setProperty("reboundByName", sl.reboundByName);
        p->setProperty("preambleUnresolved", sl.preambleUnresolved);
        p->setProperty("preambleCount", sl.preambleCount);
        p->setProperty("preambleFired", sl.preambleFired);
        p->setProperty("preambleRefused", sl.preambleRefused);
        p->setProperty("skipped", sl.skipped);
        p->setProperty("yielded", sl.yielded);
        p->setProperty("glides", sl.glides);   // s-rta-0926b plan3 C: restore glides started, not yet released
        p->setProperty("holdMs", sl.holdMs);         // s-rta-0928: the last start / loop return's message-thread hold (ms; -1 none)
        p->setProperty("holdMsMax", sl.holdMsMax);   // ... and the longest this run
        // s-rta-0927 routine display: where a pending/running routine plays (idle: -1 / [] / false / 0 / "")
        p->setProperty("deck", sl.deck);
        juce::Array<juce::var> layers;
        for (int l : sl.layers)
            layers.add(l);
        p->setProperty("layers", layers);
        p->setProperty("touchesComp", sl.touchesComp);
        p->setProperty("restartPending", sl.restartPending);
        p->setProperty("fireSeq", static_cast<juce::int64>(sl.fireSeq));
        p->setProperty("startsOn", juce::String(sl.startsOn));
        bank.add(juce::var(p));
    }
    obj->setProperty("bank", bank);
    return juce::var(obj);
}

void MainComponent::applyAudioTransport(const std::string& action, Origin origin)
{
    if (action == "stop")
    {
        // R-A8/v2 R3 (critic A5): refuse a backward transport jump while an
        // overdub is armed -- AudioEngine::stop() resets position to 0.0,
        // which would break the overdub's monotonic sample stamps.
        if (recorderHost_.status().overdub)
        {
            if (recorderHost_.dispatch.notify)
                recorderHost_.dispatch.notify("audio stop refused: overdub in progress (R-A8)");
            return;
        }
        audioEngine_.stop();
    }
    else if (action == "play")
    {
        audioEngine_.play();
    }
    else if (action == "pause")
    {
        audioEngine_.pause();
    }
    else
    {
        return;
    }
    lastAudioAction_ = action;

    if (origin == Origin::Replay) return;

    ControlPath key;
    key.scope = ControlPath::Scope::Comp;
    key.control = "audio";
    DiscretePoint p;
    p.origin = origin;
    p.action = action;
    p.v = 0;   // matches L1's bridge convention for audio-control points
    recorderHost_.capture(key, std::move(p));
}

void MainComponent::applyLayerFlag(int layerIndex, const std::string& flag, bool value, Origin origin, int deckIndex)
{
    auto* deck = deckForDispatch(deckIndex, "applyLayerFlag", origin);
    if (!deck) return;
    auto* layer = deck->getLayer(layerIndex);
    if (!layer) return;

    if (flag == "visible") layer->visible = value;
    else if (flag == "solo") layer->solo = value;
    else if (flag == "mute") layer->muted = value;
    else if (flag == "bypass") layer->bypassed = value;
    else if (flag == "autopilot") layer->autopilotEnabled = value;
    else return;

    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;
    if (resolvedDeckIndex == composition_.activeDeckIndex && deckView_) deckView_->refresh();

    if (origin == Origin::Replay) return;
    ControlPath key = layerScalarPath(composition_, resolvedDeckIndex, layerIndex, "");
    key.control = flag;
    key.scalar.clear();
    DiscretePoint p;
    p.origin = origin;
    p.v = value ? 1 : 0;
    recorderHost_.capture(key, std::move(p));
}

void MainComponent::applyEffectBypass(int layerIndex, int column, int fxIndex, bool value, Origin origin, int deckIndex)
{
    auto* deck = deckForDispatch(deckIndex, "applyEffectBypass", origin);
    if (!deck) return;
    auto* layer = deck->getLayer(layerIndex);
    if (!layer) return;
    auto* clip = layer->getClipAt(column);
    if (!clip || fxIndex < 0 || fxIndex >= static_cast<int>(clip->effects.size())) return;

    auto& slot = clip->effects[static_cast<size_t>(fxIndex)];
    slot.bypassed = value;

    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;

    if (origin == Origin::Replay) return;
    ControlPath key = clipScalarPath(composition_, resolvedDeckIndex, layerIndex, column, "");
    key.fx = fxIndex;
    key.fxName = slot.effectName;
    key.control = "bypass";
    key.scalar.clear();
    DiscretePoint p;
    p.origin = origin;
    p.v = value ? 1 : 0;
    recorderHost_.capture(key, std::move(p));
}

void MainComponent::applyClipPlaying(int layerIndex, int column, const std::string& action, Origin origin,
                                     uint64_t group, int deckIndex)
{
    auto* deck = deckForDispatch(deckIndex, "applyClipPlaying", origin);
    if (!deck) return;
    auto* layer = deck->getLayer(layerIndex);
    if (!layer) return;
    auto* clip = layer->getClipAt(column);
    if (!clip) return;

    if (action == "play") { clip->reverse = false; clip->playing = true; }
    // "resume" is "play" without the reverse reset -- used by LayerTransport's
    // pad pause/play toggle (s-rta-0924 step3 fix), which must not flip a
    // reversed clip forward the way TopBar's Play button intentionally does.
    else if (action == "resume") { clip->playing = true; }
    else if (action == "pause") { clip->playing = false; }
    else if (action == "stop") { clip->playing = false; clip->playheadPosition = clip->inPoint; }
    else if (action == "reverse") { clip->reverse = !clip->reverse; }
    else return;

    const int resolvedDeckIndex = (deckIndex < 0) ? composition_.activeDeckIndex : deckIndex;
    if (resolvedDeckIndex == composition_.activeDeckIndex && deckView_) deckView_->refresh();

    if (origin == Origin::Replay) return;
    ControlPath key = clipScalarPath(composition_, resolvedDeckIndex, layerIndex, column, "");
    key.control = "playing";
    key.scalar.clear();
    DiscretePoint p;
    p.origin = origin;
    p.action = action;
    p.v = clip->playing ? 1 : 0;
    p.group = group;
    recorderHost_.capture(key, std::move(p));
}

// === Menu Command Handler ===

void MainComponent::handleMenuCommand(int commandId)
{
    using C = AudioDNAMenuBar::CommandID;

    // Output display items (dynamic range): a tickable toggle per display (plan5 C2)
    if (commandId >= C::kOutputFullscreenBase && commandId < C::kOutputWindowed)
    {
        outputs_.toggleDisplay(commandId - C::kOutputFullscreenBase);
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
            // plan6 §7: confirmed first -- New replaces everything playing.
            confirmReplaceShow("New Composition",
                               "Start a new composition and replace \"" + juce::String(composition_.name) + "\"?",
                               "New", [this] { swapCompositionModel([this] { composition_.initDefault(); }); });
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
        // plan6 §6.3: the Deck menu mirrors the deck tab row; tab actions act on the ACTIVE deck here.
        case C::kDeckNew:
            newDeck();
            break;
        case C::kDeckLoad:
            loadDeck();
            break;
        case C::kDeckSave:
            saveDeck(composition_.activeDeckIndex);
            break;
        case C::kDeckSaveAs:
            saveDeckAs(composition_.activeDeckIndex);
            break;
        case C::kDeckRename:
            renameDeck(composition_.activeDeckIndex);
            break;
        case C::kDeckDuplicate:
            duplicateDeck(composition_.activeDeckIndex);
            break;
        case C::kDeckRemove:
            removeDeck(composition_.activeDeckIndex);
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
                            if (layer != nullptr)
                            {
                                // Clears only if cell.column is the active one (one compare-exchange, no
                                // separate check first); the exact before / after pair.
                                const LayerRuntimeTransition t = layer->clearActiveClip(cell.column);
                                if (t.changed())
                                    runtimeChildren.push_back(std::make_unique<ClearActiveClipCmd>(
                                        makeLayerResolver(), deckIdx, cell.layer,
                                        t.before, t.after, "Clear Clip"));
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
        case C::kOutputDisabled:   // "All Outputs Off" (plan5 C2)
            outputs_.closeAll();
            break;
        case C::kOutputRestoreLast:   // "Restore Last Outputs" (plan5 C3) -- the ONLY way the saved set opens
            outputs_.restoreLast();
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
                // s-rta-0926b plan4 S6: a recording is the composition canvas, exactly its size.
                cfg.width = composition_.outputWidth > 0 ? composition_.outputWidth : 1920;
                cfg.height = composition_.outputHeight > 0 ? composition_.outputHeight : 1080;
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
    gx += gw + gap;
    targets.push_back({ { gx, topY, gw, gh }, "Master Signal",
                         Binding::Action::MasterSignal, 0, 0, 0, 0, 0 });

    // s-rta-0926 routines slice 1 (plan 5.3): one row of 8 routine pads under the global row.
    int routineY = topY + gh + gap;
    for (int slot = 0; slot < RoutineEngine::kBankSize; ++slot)
    {
        BindingOverlay::BindableTarget t{ { 20 + slot * (gw + gap), routineY, gw, gh },
                                          "Routine " + juce::String(slot + 1),
                                          Binding::Action::TriggerRoutine, 0, 0, 0, 0, 0 };
        t.routineSlot = slot;
        targets.push_back(t);
    }

    // Column triggers
    int colStartX = 240; // Approximate: after layer strip area
    int colY = routineY + gh + 20;
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
                if (const int col = deck->layers[static_cast<size_t>(li)].runtime().activeClipColumn; col >= 0)
                {
                    resolvedLayer = li;
                    resolvedColumn = col;
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
                // Apply MIDI velocity to clip opacity if enabled. s-rta-0923
                // lane 3 (plan section 3.6, site #7).
                if (binding.velocityToOpacity && binding.inputType == Binding::InputType::MidiNote)
                {
                    manualWrite(clipScalarPath(composition_, composition_.activeDeckIndex, resolvedLayer, resolvedColumn, "opacity"),
                               value, GripKind::Decaying, Origin::Human); // velocity already normalized 0-1
                }
                handleClipTrigger(resolvedLayer, resolvedColumn);
            }
            else if (binding.triggerMode == Binding::TriggerMode::Momentary)
            {
                // Momentary release: clear this layer if the pad's clip is active, or cancel its
                // trigger if it is still queued (released before its beat: it never latches on).
                // One compare-exchange (Layer::releaseMomentary), no separate check first.
                if (auto* deck = composition_.getActiveDeck())
                {
                    auto* layer = deck->getLayer(resolvedLayer);
                    if (layer && layer->releaseMomentary(resolvedColumn).changed())
                    {
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
                // Apply velocity to all clips in the column if enabled.
                // s-rta-0923 lane 3 (plan section 3.6, site #8).
                if (binding.velocityToOpacity && binding.inputType == Binding::InputType::MidiNote)
                {
                    if (auto* deck = composition_.getActiveDeck())
                    {
                        for (int li = 0; li < static_cast<int>(deck->layers.size()); ++li)
                        {
                            auto& layer = deck->layers[static_cast<size_t>(li)];
                            if (layer.getClipAt(resolvedColumn))
                                manualWrite(clipScalarPath(composition_, composition_.activeDeckIndex, li, resolvedColumn, "opacity"),
                                           value, GripKind::Decaying, Origin::Human);
                        }
                    }
                }
                handleColumnTrigger(resolvedColumn);
            }
            else if (binding.triggerMode == Binding::TriggerMode::Momentary)
            {
                // Momentary release: clear every layer playing this column, and cancel the
                // column's trigger on every layer where it is still queued (Layer::releaseMomentary).
                if (auto* deck = composition_.getActiveDeck())
                {
                    for (auto& layer : deck->layers)
                        layer.releaseMomentary(resolvedColumn);
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
                        // s-rta-0923/0924 step 3 (plan section 3.3 B3): routed
                        // through applyLayerFlag (also does the deckView_->refresh()).
                        switch (binding.action)
                        {
                            case Binding::Action::ToggleLayerBypass:
                                applyLayerFlag(resolvedLayer, "bypass", !layer->bypassed, Origin::Human);
                                break;
                            case Binding::Action::ToggleLayerSolo:
                                applyLayerFlag(resolvedLayer, "solo", !layer->solo, Origin::Human);
                                break;
                            case Binding::Action::ToggleLayerMute:
                                applyLayerFlag(resolvedLayer, "mute", !layer->muted, Origin::Human);
                                break;
                            case Binding::Action::ToggleLayerAutopilot:
                                applyLayerFlag(resolvedLayer, "autopilot", !layer->autopilotEnabled, Origin::Human);
                                break;
                            case Binding::Action::ToggleLayerVisible:
                                applyLayerFlag(resolvedLayer, "visible", !layer->visible, Origin::Human);
                                break;
                            default:
                                break;
                        }
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
                        applyTempoCommand("tap", tappedBPM, Origin::Human);
                    }
                }
            }
            break;

        case Binding::Action::Resync:
            if (value > 0.0f)
                applyTempoCommand("resync", 0.0f, Origin::Human);
            break;

        case Binding::Action::GlobalPlayPause:
            if (value > 0.0f)
                applyAudioTransport(audioEngine_.isPlaying() ? "stop" : "play", Origin::Human);
            break;

        case Binding::Action::GlobalStop:
            // s-rta-0926b: the "Stop" binding follows the TopBar Stop -- routines only (Boris 2026-09-26).
            // The audio file's stop stays on the Play / Pause binding (it stops a playing file).
            if (value > 0.0f)
                routineEngine_.stopAll();
            break;

        case Binding::Action::TriggerRoutine:
            // s-rta-0926 routines slice 1 (plan 5.3): press fires (or restarts at the next boundary);
            // a Momentary binding's release stops it -- hold-to-run.
            if (value > 0.0f)
                perfRoutineFire(binding.targetRoutineSlot);
            else if (binding.triggerMode == Binding::TriggerMode::Momentary)
                perfRoutineStop(binding.targetRoutineSlot, false);
            break;

        case Binding::Action::MasterOpacity:
            // s-rta-0923 lane 3 (plan section 3.6, site #4).
            manualWrite(compScalarPath("opacity"), value, GripKind::Decaying, Origin::Human);
            break;

        case Binding::Action::MasterSignal:
            // s-rta-0925 mastersignal Step 1: same funnel shape as MasterOpacity.
            manualWrite(compScalarPath("signal"), value, GripKind::Decaying, Origin::Human);
            break;

        case Binding::Action::AdjustLayerOpacity:
        {
            // s-rta-0923 lane 3 (plan section 3.6, site #5).
            manualWrite(layerScalarPath(composition_, composition_.activeDeckIndex, resolvedLayer, "opacity"),
                       value, GripKind::Decaying, Origin::Human);
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
                        const int col = layer->runtime().activeClipColumn;
                        auto* clip = layer->getClipAt(col);
                        if (clip)
                            // "resume" (not "play") on the pause->play leg: a live
                            // pad toggle must not force a reversed clip forward.
                            applyClipPlaying(resolvedLayer, col,
                                             clip->playing ? "pause" : "resume", Origin::Human);
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
                        const int col = layer->runtime().activeClipColumn;
                        auto* clip = layer->getClipAt(col);
                        if (clip && binding.targetEffectIndex >= 0 &&
                            binding.targetEffectIndex < static_cast<int>(clip->effects.size()))
                        {
                            auto& fx = clip->effects[static_cast<size_t>(binding.targetEffectIndex)];
                            applyEffectBypass(resolvedLayer, col,
                                              binding.targetEffectIndex, !fx.bypassed, Origin::Human);
                        }
                    }
                }
            }
            break;
        }

        case Binding::Action::AdjustMacro:
        {
            // CC value → macro knob. s-rta-0923 lane 3 (plan section 3.6, site #6).
            if (binding.targetMacroIndex >= 0 && binding.targetMacroIndex < MacroBank::kNumMacros)
                manualWrite(macroPath(binding.targetMacroIndex), value, GripKind::Decaying, Origin::Human);
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
                    // s-rta-0926b plan4 S6: the composition canvas, exactly its size.
                    cfg.width = composition_.outputWidth > 0 ? composition_.outputWidth : 1920;
                    cfg.height = composition_.outputHeight > 0 ? composition_.outputHeight : 1080;
                    cfg.fps = 30; cfg.quality = 23;
                    videoRecorder_.startRecording(docsDir.getChildFile(filename), cfg);
                }
            }
            break;
        }
    }
}
