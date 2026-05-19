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
#include "ui/SignalBar.h"
#include "ui/TopBar.h"
#include "ui/ProgrammingMode.h"
#include "ui/DeckView.h"
#include "ui/InspectorPanel.h"
#include "ui/TimingWindow.h"
#include "ui/BrowserPanel.h"
#include "ui/MenuBarModel.h"
#include "signal/SignalRegistry.h"
#include "routing/MacroBank.h"
#include "model/Composition.h"
#include "ui/BindingOverlay.h"
#include "ui/MidiLearnOverlay.h"
#include "midi/MidiHandler.h"
#include "core/UndoManager.h"
#include "recording/SessionRecorder.h"
#include "recording/VideoRecorder.h"
#include "sync/LinkSync.h"
#include "api/ApiServer.h"
#include "osc/OscHandler.h"
#include "midi/MidiOutputHandler.h"
#include "output/SyphonOutput.h"
#if AUDIODNA_TEST_SERVER
 #include "test/TestServer.h"
#endif
#if AUDIODNA_BUILD_INSPECTOR
 #include <melatonin_inspector/melatonin_inspector.h>
#endif
#if AUDIODNA_HAS_CAMERA
 #include <juce_video/juce_video.h>
#endif

class MainComponent : public juce::Component,
                      public juce::DragAndDropContainer,
                      public juce::FileDragAndDropTarget,
                      public juce::KeyListener,
#if AUDIODNA_HAS_CAMERA
                      public juce::CameraDevice::Listener,
#endif
                      private juce::Timer
{
public:
    MainComponent(bool testMode = false, int testPort = 8080);
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Keyboard shortcuts (Component override)
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;

    // KeyListener — catches keys globally regardless of focus
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

#if AUDIODNA_HAS_CAMERA
    // CameraDevice::Listener
    void imageReceived(const juce::Image& image) override;
#endif

    // Menu bar model — accessible for MainWindow to set on the native title bar
    juce::MenuBarModel* getMenuBarModel() { return menuBarModel_.get(); }

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

    // Menu command handler
    void handleMenuCommand(int commandId);

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

    // === v2: Signal Bar + Top Bar + Deck ===
    Composition composition_;
    UndoManager undoManager_;
    SignalRegistry signalRegistry_;
    MacroBank globalMacroBank_{MacroBank::Scope::Global};
    SessionRecorder sessionRecorder_;
    LinkSync linkSync_;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;
    bool tooltipsEnabled_ = true;
    std::unique_ptr<TopBar> topBar_;
    std::unique_ptr<SignalBar> signalBar_;
    std::unique_ptr<ProgrammingMode> programmingMode_;
    std::unique_ptr<DeckView> deckView_;
    std::unique_ptr<InspectorPanel> inspectorPanel_;
    std::unique_ptr<BrowserPanel> browserPanel_;

    // === v2: Menu Bar ===
    std::unique_ptr<AudioDNAMenuBar> menuBarModel_;

    // === v2: Binding System & MIDI (P9) ===
    BindingManager bindingManager_;
    std::unique_ptr<BindingOverlay> bindingOverlay_;
    std::unique_ptr<MidiLearnOverlay> midiLearnOverlay_;
    std::unique_ptr<MidiHandler> midiHandler_;
    void buildBindableTargets(std::vector<BindingOverlay::BindableTarget>& targets);
    void handleBindingAction(const Binding& binding, float value);
    void enterKeyboardBindingMode();
    void enterMidiLearnMode();
    void exitAllBindingModes();

    // Resizable horizontal divider between deck and bottom panels
    int deckDividerY_ = -1; // -1 = auto (snap to bottom of layers)
    bool draggingDivider_ = false;
    static constexpr int kDividerHeight = 5;
    static constexpr int kMinDeckHeight = 120;
    static constexpr int kMinBottomHeight = 100;
    juce::Rectangle<int> dividerBounds_;
    juce::Rectangle<int> browserPlaceholderBounds_;
    std::unique_ptr<TimingWindow> timingWindow_;

    // Resizable vertical dividers between bottom panels
    // 4 panels: preview | timing | inspector | browser
    // 3 dividers between them, stored as fractional X positions [0,1] within bottom area
    static constexpr int kVDividerWidth = 5;
    static constexpr int kMinPanelWidth = 120;
    float vDividerFrac_[3] = { 0.22f, 0.50f, 0.75f }; // initial fractions
    juce::Rectangle<int> vDividerBounds_[3];
    int draggingVDivider_ = -1; // -1 = none, 0/1/2 = which divider
    bool hoveringHDivider_ = false;
    int hoveringVDivider_ = -1; // -1 = none
    int bottomAreaX_ = 0;       // left edge of bottom panel area
    int bottomAreaWidth_ = 0;   // total width of bottom panel area
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

    void handleImportISF();
    void handleClipTrigger(int layerIndex, int column);
    void handleColumnTrigger(int column);
    void handleFileDrop(int layerIndex, int column, const juce::File& file);
    void handleMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files);
    void handleDeckSwitch(int deckIndex);

    // Test mode
    bool testMode_ = false;
    int testPort_ = 8080;
#if AUDIODNA_TEST_SERVER
    std::unique_ptr<TestServer> testServer_;
#endif
#if AUDIODNA_BUILD_INSPECTOR
    std::unique_ptr<melatonin::Inspector> melatoninInspector_;
#endif

    // === P22: Output & Integration ===
    std::unique_ptr<ApiServer> apiServer_;
    OscHandler oscHandler_;
    MidiOutputHandler midiOutputHandler_;
    VideoRecorder videoRecorder_;
    SyphonOutput syphonOutput_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
