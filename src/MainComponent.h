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
#include "ui/OutputWindow.h"
#include "ui/KeyboardPanel.h"
#include "keyboard/KeySlot.h"
#if AUDIODNA_HAS_CAMERA
 #include <juce_video/juce_video.h>
#endif

class MainComponent : public juce::Component,
                      public juce::FileDragAndDropTarget,
#if AUDIODNA_HAS_CAMERA
                      public juce::CameraDevice::Listener,
#endif
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Keyboard shortcuts
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

#if AUDIODNA_HAS_CAMERA
    // CameraDevice::Listener
    void imageReceived(const juce::Image& image) override;
#endif

private:
    void openImage();
    void savePreset();
    void loadPreset();
    void timerCallback() override;
    void refreshDisplayList();
    void openOutputOnDisplay(int displayIndex);
    void closeOutput();
    void randomizeAllEffects();
    void beatSyncRandomize();
    void fastSave();
    void loadSlotPreset(int slot, const juce::File& file);
    void populateSlotMenu(int slot);
    juce::File getFastSaveDir() const;
    void saveDeck();
    void loadDeck();
#if AUDIODNA_HAS_CAMERA
    void openCamera(int deviceIndex);
    void closeCamera();
    void refreshCameraList();
#endif
    void openImageFolder();
    void advanceSlideshow();

    AudioDNALookAndFeel lookAndFeel_;

    // Core audio pipeline
    RingBuffer<float> ringBuffer_{16384};
    AudioEngine audioEngine_{ringBuffer_};
    AnalysisThread analysisThread_{ringBuffer_};

    // Controls
    juce::TextButton openImageButton_{"Open Image"};
    juce::TextButton openFolderButton_{"Image Folder"};
    juce::ComboBox imageBeatSelector_;
    juce::Label imageBeatLabel_;

    // Image slideshow
    juce::Array<juce::File> slideshowImages_;
    int slideshowIndex_ = 0;
    int slideshowBeats_ = 8;         // Beats per image
    int slideshowBeatCounter_ = 0;
    float lastSlideshowBeatPhase_ = 0.0f;
    juce::Label masterLevelLabel_;
    juce::Slider masterLevelSlider_;
    juce::Label audioSourceLabel_;
    juce::TextButton savePresetButton_{"Save"};
    juce::TextButton loadPresetButton_{"Load"};
    juce::Label fileLabel_;
    juce::Label fpsLabel_;
    juce::Label cpuLabel_;

    // Display components
    WaveformDisplay waveformDisplay_{analysisThread_};
    AudioReadoutPanel audioReadoutPanel_{analysisThread_, analysisThread_.getFeatureBus()};
    SpectrumDisplay spectrumDisplay_{analysisThread_.getFeatureBus()};
    PreviewPanel previewPanel_{analysisThread_.getFeatureBus()};

    // Effects rack (right panel) — initialized after previewPanel_
    EffectLibrary effectLibrary_;
    std::unique_ptr<EffectsRackPanel> effectsRackPanel_;

    // Randomize controls
    juce::Label randomLabel_;
    juce::TextButton syncButton_{"Sync"};
    juce::ToggleButton beatRandomToggle_{"Beats"};
    juce::ComboBox beatCountSelector_;
    int beatRandomCount_ = 4;       // Randomize every N beats
    int beatCounter_ = 0;           // Counts beats since last randomize
    float lastBeatPhase_ = 0.0f;    // Track beat phase for edge detection
    int uiUpdateCounter_ = 0;       // Throttle UI label updates

    // Fast save
    juce::TextButton fastSaveButton_{"FX Save"};
    int fastSaveCounter_ = 1;

    // Deck save/load
    juce::TextButton deckSaveButton_{"Deck Save"};
    juce::TextButton deckLoadButton_{"Deck Load"};
    juce::File currentAudioFile_;  // Track loaded audio for deck save

    // Bottom preset slots (10 slots)
    static constexpr int kNumSlots = 10;
    struct PresetSlot
    {
        std::unique_ptr<juce::TextButton> button;
        std::unique_ptr<juce::ComboBox> dropdown;
        juce::File loadedFile;
    };
    std::array<PresetSlot, kNumSlots> presetSlots_;

    // Resolution lock
    juce::Label viewportLabel_;
    juce::ComboBox resolutionSelector_;

    // Output window
    juce::Label outputLabel_;
    juce::ComboBox displaySelector_;
    std::unique_ptr<OutputWindow> outputWindow_;
    juce::File currentImageFile_;  // Track loaded image for output window

    // Audio source selector
    juce::ComboBox audioSourceSelector_;
    juce::Slider inputGainSlider_;
    juce::Label inputGainLabel_;
    juce::Rectangle<int> inputLevelMeterBounds_;  // Drawn in paint()

#if AUDIODNA_HAS_CAMERA
    // Camera input
    juce::Label cameraLabel_{"", "Camera"};
    juce::ComboBox cameraSelector_;
    std::unique_ptr<juce::CameraDevice> cameraDevice_;
    bool cameraActive_ = false;
#endif

    std::unique_ptr<juce::FileChooser> fileChooser_;

    // === Keyboard Launcher (M7) ===
    KeyboardLayout keyboardLayout_;
    std::unique_ptr<KeyboardPanel> keyboardPanel_;
    void handleKeySlotTrigger(char keyChar, bool isDown);

    // === Collapsible Panel Toggles ===
    juce::TextButton toggleAudioBtn_{"A"};
    juce::TextButton toggleFxBtn_{"FX"};
    juce::TextButton toggleWaveBtn_{"W"};
    juce::TextButton toggleKeysBtn_{"K"};
    juce::TextButton togglePresetsBtn_{"P"};
    bool showAudioPanel_ = true;
    bool showFxPanel_ = true;
    bool showWavePanel_ = true;
    bool showKeysPanel_ = true;
    bool showPresetsPanel_ = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
