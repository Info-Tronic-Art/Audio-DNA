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
#include "ui/DeckView.h"
#include "ui/InspectorPanel.h"
#include "ui/TimingWindow.h"
#include "ui/BrowserPanel.h"
#include "sources/ProjectMPresetManager.h"
#include "ui/MenuBarModel.h"
#include "signal/SignalRegistry.h"
#include "routing/MacroBank.h"
#include "model/Composition.h"
#include "model/ControlPath.h"
#include "connect/ConnectionEngine.h"
#include "connect/ConnClock.h"
#include "connect/ManualWrite.h"
#include "recording/Lane.h"
// s-rta-0923/0924 step 3 (Lane S3-B): the recorder host this component
// wires the tick, choke points and REST surface into (recorderHost_ below).
#include "recording/RecorderHost.h"
#include "ui/BindingOverlay.h"
#include "ui/MidiLearnOverlay.h"
#include "midi/MidiHandler.h"
#include "core/UndoManager.h"
#include "core/UndoService.h"
#include "core/ClipCommands.h"
#include "core/DeckCommands.h"
#include "core/EffectScope.h"
#include "core/EffectCommands.h"
#include "core/TriggerCommands.h"
#include <atomic>
#include <optional>
#include <memory>
#include <vector>
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

    // s-rta-0923 lane 3 (plan section 3.1/3.4; R8: this lane owns
    // manualWrite, the recorder only hooks it later). The funnel every
    // non-widget manual writer goes through — OSC/MIDI/REST sites (section
    // 3.6) and, later, the recorder's Player::Sink. Widgets grip the bound
    // ParamConnection directly (plan section 4.2, Lane C4) and do not pass
    // here. GripKind is `ParamConnection::Grip::Kind`; Origin is
    // `recording/Lane.h`'s Origin (included above).
    using GripKind = ParamConnection::Grip::Kind;
    bool manualWrite(const ControlPath& path, float valueNorm, GripKind kind, Origin origin);
    void manualRelease(const ControlPath& path, Origin origin);
    // Critic finding #4 (s-rta-0923-lane3-plan.md amendment 4): a touch-only
    // seam (opens/refreshes the grip with no value write) for step 3's
    // `Sink::touch` — a recorded gesture's grip must open BEFORE its first
    // value arrives. Wraps manualTouchCore exactly like manualWrite wraps
    // manualWriteCore.
    bool manualTouch(const ControlPath& path, GripKind kind, Origin origin);
    // STEP-3 HOOK SEAM (recorder): called AFTER the core decided.
    // `accepted == false` means refused (a lower rank tried to touch/write
    // over a higher one — see ManualWrite.h's Hand ranks).
    std::function<void(const ControlPath&, float valueNorm, GripKind, Origin, bool accepted)> onManualWrite;
    std::function<void(const ControlPath&, GripKind, Origin, bool accepted)> onManualTouch;
    std::function<void(const ControlPath&, Origin)> onManualRelease;

private:
    void openImage();
    void savePreset();
    void loadPreset();
    // L3 (2026-09): Composition persistence — File > Open/Save/Save As and
    // Cmd+O/Cmd+S now operate on `composition_`, not the v1 FX preset (which
    // savePreset()/loadPreset() above still serve via their own row-1
    // buttons). swapCompositionModel/refreshUiAfterModelSwap are the shared
    // GL-fence + undo-clear + inspector-null helper kCompNew is also based
    // on — see .harmony/.work-packets/L3-composition-persistence.md §1.
    void openComposition();
    void loadComposition(const juce::File& file);
    void saveComposition();
    void saveCompositionAs();
    void swapCompositionModel(const std::function<void()>& mutation);
    void refreshUiAfterModelSwap();
    // L3 STEP 3 (2026-09): the Comp/Decks browser's Decks rows append a saved
    // deck into the live composition rather than replacing it — a performer
    // clicking a saved deck mid-set must not lose the deck they are on, and
    // append closes no live media (zero outgoing ids). openMediaForDeck is
    // the per-clip media-open loop factored out of loadComposition's OPEN
    // NEW step (§1 step 4) so both callers share one body. NOT named
    // loadDeck — that name is the legacy PresetManager path below.
    void openMediaForDeck(Deck& deck);
    void appendDeckFromFile(const juce::File& file);
    void timerCallback() override;
    // W5 (outputwindow-arc-design.md): named seam for the mapping tick, so
    // the A1 routing/signal-extraction follow-up can join here later
    // without re-plumbing. Reads the feature bus once and drives
    // MappingEngine::processFrame — see mappingTickTimer_ below.
    void tickFeaturePipeline();
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

    // === Undo command construction (Undo v1 step 2) ===
    // One in-flight cell change: coordinate + before/after value snapshots.
    struct CellEdit
    {
        int layerIndex = 0;
        int column = 0;
        std::optional<Clip> before;
        std::optional<Clip> after;
    };
    // Hooks the clip commands use, bound to this component's model/renderer.
    ClipLayerResolver makeLayerResolver();
    ClipDeckResolver makeDeckResolver();
    ClipMediaHook makeClipMediaHook();
    // Close half of the risk #4 guard (media-leak fix, L1) — see the doc
    // comment on ClipMediaDisposeHook (ClipCommands.h) and on the function
    // definition (MainComponent.cpp) for the liveness-scan safety guard.
    ClipMediaDisposeHook makeClipMediaDisposeHook();
    // GL fence for structure-changing layer commands (add/remove/move) — binds to
    // UndoService::withDeckDetached so execute/undo/redo fence the deck->layers mutation.
    DeckFenceHook makeDeckFence();
    // Re-resolve the live Composition for deck-vector commands (add/remove/switch),
    // which reach the decks vector + activeDeckIndex (beyond a single Deck).
    CompositionResolver makeCompositionResolver();
    // Re-point the renderer's active deck for SwitchDeckCmd (no fence needed —
    // a switch is an atomic pointer handoff, not a decks-vector mutation).
    DeckActivateHook makeDeckActivateHook();
    // Notify the open inspector after an effect-stack edit (EffectStackCmd). A
    // lightweight recolor/re-value refresh; the command's own apply() never
    // rebuilds rows. It does NOT preserve row expansion across undo/redo —
    // refreshAfterUndoRedo unconditionally rebuilds afterward (see the .cpp).
    std::function<void()> makeEffectStackRefresh();
    // Snapshot a cell (nullopt if empty / out of range).
    static std::optional<Clip> snapshotCell(Layer* layer, int column);
    // Build a single SetClipCmd for one cell edit.
    std::unique_ptr<Command> makeSetClipCmd(int deckIndex, const CellEdit& edit,
                                            const juce::String& description);
    // Record cell edits as one undo unit (single command, or a CompositeCommand
    // when more than one cell changed). Skips empty edit lists / empty composites.
    void pushClipEdits(int deckIndex, std::vector<CellEdit> edits,
                       const juce::String& description);
    // Push a ready list of commands as one undo unit (single or composite).
    void pushCommands(std::vector<std::unique_ptr<Command>> children,
                      const juce::String& compositeDescription);
    // Grid + inspector refresh after an undo/redo (re-inspect by coordinate).
    // affectsLayerOrder mirrors the command that was just undone/redone's
    // Command::affectsLayerOrder() (captured by the caller via
    // undoManager_.undoAffectsLayerOrder()/redoAffectsLayerOrder(), matching
    // direction, before calling undo()/redo()) — used to clear the now-
    // possibly-mis-mapped multi-cell clip selection after a layer-reorder
    // round-trip (P24.13).
    void refreshAfterUndoRedo(bool affectsLayerOrder);

    AudioDNALookAndFeel lookAndFeel_;

    // MilkDrop preset manager, hoisted out of ProjectMSource (2026-09-04
    // autoload fix — see .harmony/milkdrop-autoload-rootcause.md). Pure
    // file/JSON scanning with no GL dependency, so it's scanned
    // unconditionally at construction instead of being gated on a
    // GL-thread-only source that doesn't exist yet. Declared FIRST among
    // MainComponent's members (deliberately — has no dependency on anything
    // else, so this is safe) so it is destroyed LAST: C++ tears members down
    // in reverse declaration order, and both previewPanel_ (transitively:
    // Renderer -> ProjectMSource) and browserPanel_ (MilkDropBrowser) hold
    // raw pointers into this manager. Declaring it after either of them
    // would destroy it first, dangling both — the exact class of UAF this
    // codebase already treats as load-bearing (see the do-not-clear rule at
    // Renderer::openGLContextClosing()).
    ProjectMPresetManager presetManager_;

    // Fixed MilkDrop preset sources scanned unconditionally at startup
    // (bundled resources + Cream-of-the-Crop, computed once in the ctor).
    // Preferences > Video's user directory is appended on top of this base
    // set on every (re)scan — see setMilkDropPresetDir() — so picking a
    // custom folder never loses the built-in presets.
    std::vector<std::string> milkDropBaseDirs_;

    // Core audio pipeline
    RingBuffer<float> ringBuffer_{16384};
    AudioEngine audioEngine_{ringBuffer_};
    // R13 lane D: the analysis thread reads the device rate from
    // audioEngine_'s cell (member order above puts audioEngine_ before
    // analysisThread_, so the reference is valid at construction) and
    // resamples to its fixed internal 48 kHz (AnalysisResampler, lane A).
    AnalysisThread analysisThread_{ringBuffer_, &audioEngine_.sourceSampleRateCell()};

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

    // W5 (outputwindow-arc-design.md/U2): dedicated message-thread timer
    // driving the audio->effect mapping tick at a fixed cadence, independent
    // of GL attach/visibility state and of MainComponent's own 30Hz UI timer
    // above (a single juce::Timer object can only run one callback at one
    // rate, so this is a second, separate Timer rather than folding into
    // timerCallback()). Runs UNCONDITIONALLY for MainComponent's whole
    // lifetime so mapped params keep updating even while previewPanel_'s GL
    // context is detached.
    class MappingTickTimer : public juce::Timer
    {
    public:
        explicit MappingTickTimer(MainComponent& owner) : owner_(owner) {}
        void timerCallback() override { owner_.tickFeaturePipeline(); }
    private:
        MainComponent& owner_;
    };
    // A5 (outputwindow-arc-design.md): matches the MEASURED attached render
    // rate (~120fps). The Smoother has no dt term, so the tick rate IS the
    // smoothing time constant — 60Hz would double the smoothing feel.
    static constexpr int kMappingTickHz = 120;
    MappingTickTimer mappingTickTimer_{*this};

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
    UndoService undoService_;
    SignalRegistry signalRegistry_;
    MacroBank globalMacroBank_{MacroBank::Scope::Global};
    // s-rta-0923 lane 3 (plan section 4.1 piece 1): the ONE evaluator for
    // every ParamConnection, ticked once per tickFeaturePipeline() call
    // (after globalMacroBank_.updateValues, before inspectorPanel_->
    // tickModulation()). lastConnTick_ is the previous tick's connNow(); 0.0
    // means "no previous tick yet" (first tick falls back to 1/kMappingTickHz
    // as dt, matching the timer's nominal period).
    ConnectionEngine connectionEngine_;
    double lastConnTick_ = 0.0;
    LinkSync linkSync_;
    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;
    bool tooltipsEnabled_ = true;
    // Preferences > Video: user's custom MilkDrop preset folder (empty =
    // none set). Persisted to settings.json — see save/loadMilkDropPresetDirSetting().
    juce::String milkDropPresetDir_;
    std::unique_ptr<TopBar> topBar_;
    std::unique_ptr<SignalBar> signalBar_;
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
    // s-rta-0923/0924 step 3 (Lane S3-B, plan section 3.3 B2, critic A3/A4):
    // origin/deckIndex are defaulted so all nine existing (Human, active-deck)
    // callers compile unchanged; RecorderHost's Dispatch::fire is the only
    // caller that ever passes Origin::Replay (+ a specific deckIndex from the
    // compiled Fired's ResolvedTarget).
    // s-rta-0925 (D4 preamble, plan section 3.5): `immediate` bypasses the
    // beat-snap/quantize queue entirely (Layer::triggerClipImmediate directly,
    // or clearActiveClip() for an empty cell) -- the checkpoint-0 restore is
    // not a performance trigger, it is putting the model back the way it was.
    // Defaulted false so every existing (performance-trigger) caller is
    // unaffected; only dispatch.fire's Preamble branch passes true.
    void handleClipTrigger(int layerIndex, int column, Origin origin = Origin::Human, int deckIndex = -1,
                           bool immediate = false);
    void handleColumnTrigger(int column, Origin origin = Origin::Human, int deckIndex = -1);
    // A1 fix (2026-07-30): re-sync the previewPanel_ renderer's global fallback
    // state (activeSourceType_ / loaded image) to whichever layer still owns
    // active content after a layer's clip is cleared. Mirrors handleColumnTrigger's
    // post-trigger preview refresh (scan for the first layer with an active
    // Image/Source clip; purge only if none remain) so clearing one layer's X
    // can never blank a DIFFERENT layer's still-playing visual.
    void refreshPreviewFromActiveClip(Deck& deck);
    void handleFileDrop(int layerIndex, int column, const juce::File& file);
    // Perform one image/video file drop into a cell (build clip, open media,
    // setClip) WITHOUT pushing an undo command. Returns the resulting cell edit
    // (nullopt if refused — no deck or content-locked). handleFileDrop wraps this
    // as one command; multi-video drop collects N edits into one composite.
    std::optional<CellEdit> applyFileDrop(int layerIndex, int column, const juce::File& file);
    void handleMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files);
    // Perform one image-sequence drop (build the ImageSequence clip, open it in
    // the renderer, setClip) WITHOUT pushing an undo command. Returns the
    // resulting cell edit (nullopt if refused — no deck). Mirrors applyFileDrop's
    // shape; handleMultiFileDrop wraps this as one command, and the mixed-drop
    // handler (2026-07-30) combines it with applyFileDrop's video edits into one
    // composite so an image+video Finder drop is a single undo entry.
    std::optional<CellEdit> applyMultiFileDrop(int layerIndex, int column, const std::vector<juce::File>& files);
    void handleDeckSwitch(int deckIndex, Origin origin = Origin::Human);

    // s-rta-0923/0924 step 3 (Lane S3-B, plan section 3.3 B3, D6b): the five
    // choke points every writer of a discrete control funnels through, so
    // RecorderHost can capture Human writes and replay Replay ones through
    // the SAME mutation code every other origin uses. Each captures (via
    // recorderHost_.capture) only when origin != Origin::Replay (belt and
    // braces -- the host filters on this too, RecorderHost.h's capture() doc).
    // s-rta-0925 (D4 preamble, plan section 3.5 item 3): each gains a trailing
    // `int deckIndex = -1` (the SAME pattern as handleClipTrigger's B2 fix) --
    // -1 means "the active deck" (every existing caller); a Replay/Preamble
    // dispatch passes the Fired's resolved target deck explicitly, which may
    // not be the active one. `deckForDispatch` resolves it and issues the
    // existing "deck unresolved" notice for a non-Human origin that fails to
    // resolve; each handler's capture key uses the RESOLVED index instead of
    // composition_.activeDeckIndex, and any UI refresh stays gated to the
    // active deck exactly as handleClipTrigger's own refresh is.
    Deck* deckForDispatch(int deckIndex, const char* who, Origin origin);
    void applyClearActiveClip(int layerIndex, Origin origin, int deckIndex = -1);
    void applyTempoCommand(const std::string& action, float bpm, Origin origin);
    void applyAudioTransport(const std::string& action, Origin origin);
    void applyLayerFlag(int layerIndex, const std::string& flag, bool value, Origin origin, int deckIndex = -1);
    void applyEffectBypass(int layerIndex, int column, int fxIndex, bool value, Origin origin, int deckIndex = -1);
    void applyClipPlaying(int layerIndex, int column, const std::string& action, Origin origin,
                          uint64_t group = 0, int deckIndex = -1);

    // s-rta-0924b step 4 (Lane S4-B): ONE funnel for REST (/api/perf/*) and the
    // Record panel. Each returns "" on success, else the refusal/failure text --
    // the same text is also sent through recorderHost_.dispatch.notify. The
    // bodies are the former apiServer_->onPerf* lambda bodies, moved; the only
    // behaviour changes are (a) programmatic source-mode switches go through
    // setAudioSourceModeSynced (the "Audio" selectors follow), (b) Stop Playback
    // restores the source mode active before a play-with-audio, and (c) Harmony
    // ruling 1: Stop Playback during an overdub ends the overdub first
    // (RecorderHost::stopPlayback), so the transport stop is not refused.
    std::string perfRecord(const ApiServer::PerfRecordOpts& opts);
    std::string perfStop();
    std::string perfLoad(const juce::File& takeFolder);
    std::string perfPlay(bool withAudio);
    std::string perfStopPlay();
    std::string perfRepair();
    juce::var   perfStatusVar() const;                 // /api/perf/status; reads ONLY recorderHost_.status()
    static juce::File takesRoot();                     // ~/Documents/Audio-DNA/Takes
    void setAudioSourceModeSynced(AudioEngine::SourceMode mode);   // engine + BOTH selectors (dontSendNotification)
    std::optional<AudioEngine::SourceMode> sourceModeBeforeReplay_; // set by perfPlay(withAudio), consumed by perfStopPlay
    void onReplayFinished();            // Dispatch::replayFinished handler (s-rta-0925 end-of-replay)
    void restoreInputAfterReplay();     // shared tail of perfStopPlay and onReplayFinished (one policy)
    // s-rta-0925: the audio source mode as of the last tick, for /api/perf/status.inputSource -- written on the message
    // thread in tickFeaturePipeline (one write site, self-healing whatever moved the mode), read on the HTTP thread.
    // The one additive exception to perfStatusVar's "reads ONLY recorderHost_.status()" rule; never a device read.
    std::atomic<int> inputSourceMirror_{ 0 };   // 0 = input (MicInput), 1 = file

    // Enable/disable the shared tooltip window (Preferences → Show Tooltips).
    void setTooltipsEnabled(bool enabled);

    // Preferences → Video → MilkDrop Presets folder pref (L7). Updates
    // milkDropPresetDir_, persists it, rebuilds the manager's directory
    // list (milkDropBaseDirs_ + this one) and rescans (via Renderer, which
    // confines the mutation to the GL thread — see
    // Renderer::rescanMilkDropPresets()), then refreshes the browser.
    void setMilkDropPresetDir(const juce::String& dir);
    // settings.json lives at userApplicationDataDirectory/Audio-DNA/,
    // matching the JSON+DynamicObject idiom already used for View > Save/
    // Load Layout (MainComponent.cpp, kViewSaveLayout/kViewLoadLayout) —
    // the only difference is these run automatically instead of via an
    // explicit user-facing file chooser.
    juce::String loadMilkDropPresetDirSetting() const;
    void saveMilkDropPresetDirSetting(const juce::String& dir) const;

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

    // s-rta-0923/0924 step 3 (Lane S3-A contract / Lane S3-B wiring): owns
    // the recording/playback lifecycle (arm/tick/disarm/load/play). Declared
    // AFTER syphonOutput_ (the previous last member) so it is destroyed
    // FIRST, before audioEngine_ and composition_ -- both still needed by
    // the shutdown() call in ~MainComponent().
    RecorderHost recorderHost_{ AudioStore(AudioStore::defaultRoot()) };
    // The recorder's checkpoint capture (PerfState.audioAction) needs the
    // last transport action; applyAudioTransport is its only writer.
    std::string lastAudioAction_ = "stop";
    // R3 throttle (plan section 3.3 B3): Ableton Link ticks the tempo choke
    // point at ~30Hz -- only capture a tempo point when the BPM actually
    // moved by a meaningful amount.
    float lastLinkCapturedBpm_ = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
