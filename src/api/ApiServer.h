#pragma once

#include <httplib.h>
#include <juce_core/juce_core.h>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include "core/LoadTicket.h"
#include <optional>
#include <string>
#include <vector>
#if AUDIODNA_TEST_SERVER
#include "api/MessageHeartbeat.h"
#endif

// Forward declarations
class Renderer;
class FeatureBus;
struct FeatureSnapshot;
struct InjectedOnsetCount;
class EffectChain;
class SourceRegistry;
class SignalRegistry;
class RoutingEngine;
struct Composition;
class BindingManager;

// ApiServer: Production HTTP REST API for external control.
//
// Provides endpoints to:
//   - Query app state (FPS, BPM, deck structure, effect params)
//   - Trigger clips and columns
//   - Set effect and source parameters
//   - Take snapshots, start/stop recording
//   - Query and inject audio features
//   - Manage signal routing
//
// Runs on a background thread. Thread-safe: uses existing thread-safe
// APIs on Renderer, FeatureBus, etc.
class ApiServer
{
public:
    ApiServer(Renderer& renderer,
              const FeatureBus& featureBus,
              Composition& composition,
              EffectChain& effectChain,
              SourceRegistry& sourceRegistry,
              SignalRegistry& signalRegistry,
              RoutingEngine& routingEngine,
              BindingManager& bindingManager,
              int port = 7070,
              bool allowFeatureInjection = false);

    ~ApiServer();

    // Start the HTTP server on a background thread.
    void start();

    // Stop the server and join the background thread.
    void stop();

    bool isRunning() const { return running_.load(std::memory_order_relaxed); }
    int getPort() const { return port_; }

    // Callbacks wired by MainComponent
    std::function<void(int layer, int column)> onTriggerClip;
    std::function<void(int column)> onTriggerColumn;
    std::function<void(int deckIndex)> onSwitchDeck;
    // POST /api/load_composition (S166): fires only after this handler has
    // already confirmed the file exists and passes validateComposition —
    // see handleLoadComposition. s-rta-0929 asyncload (plan R1): the load is STAGED (its videos open off the message
    // thread); the handler waits on the ticket (bounded, kLoadWaitMs) and answers {"ok":true} only once the staged swap
    // is done -- so "load then act" on one connection sees the new composition, as it always did.
    std::function<void(juce::File, std::shared_ptr<LoadTicket>)> onLoadComposition;
    std::function<void()> onSnapshot;
    std::function<void(float bpm)> onSetBpm;
    // POST /api/resync (s-rta-0925): manual Resync, same funnel as the TopBar button /
    // bound key or pad / take replay -- applyTempoCommand("resync", ...) -> requestResync().
    std::function<void()> onResync;
    // R4: this server holds no FeatureBus writer — test-mode
    // /api/inject_features relays the built snapshot to the TestServer-held
    // Writer through this callback (wired by MainComponent in test mode).
    // Onset render-path fix: the snapshot's onsetCount is NOT computed here --
    // the request's onset intent is passed through and resolved against the
    // previously injected count under TestServer::injectSnapshot's lock (the
    // one place both inject routes funnel into; see OnsetPulse.h).
    std::function<void(const FeatureSnapshot&, const InjectedOnsetCount&)> onInjectFeatures;
    // s-rta-0923 lane 3 (plan section 3.6, sites #9/#10): the message-thread
    // write these two endpoints used to do inline is now routed through
    // MainComponent::manualWrite so a layer-opacity / clip-effect-param REST
    // write participates in the D8 grip chain like every other writer. The
    // inline write is REMOVED from ApiServer.cpp; these callbacks are the
    // only thing the two handlers do now.
    std::function<void(int layer, float opacity)> onSetLayerOpacity;
    std::function<void(int layer, int column, int fxIndex, int paramIndex, const std::string& paramName, float value)> onSetClipEffectParam;
    // s-rta-0926b plan-fitmode: POST /api/set_clip_param {"param": "fitMode"} (message thread, active deck).
    std::function<void(int layer, int column, int fitMode)> onSetClipFitMode;
    // s-rta-0925 mastersignal Step 1: same shape as onSetLayerOpacity above --
    // handleSetMasterSignal only parses/validates the request; the actual
    // write is routed through MainComponent::manualWrite.
    std::function<void(float depth)> onSetMasterSignal;

    // s-rta-0923 step 3 (Lane S3-C, plan section 3.4 pulled forward from build-order
    // row 5, amended per s-rta-0924 critic A5): the ONLY production-mode trigger
    // surface the recorder can be verified through (RecordPanel is step 4's file).
    // MainComponent (Lane S3-B) assigns these against RecorderHost. Every callback
    // here is marshalled to the message thread the same way as every other model
    // write in this file (see handleSetParam's clip-effect branch note) EXCEPT
    // onPerfStatus, which MUST be synchronous and MUST read nothing but
    // RecorderHost::status()'s mutex-guarded copy — never
    // audioEngine_.getCurrentSampleRate()/getCurrentAudioDevice() directly (critic
    // A5(b)/N3: the message thread may be mid-restart of the audio device). The
    // returned juce::var is a fully-built status object (deviceRate, rateChangedSinceArm,
    // humanRefused, and the rest of RecorderHost::Status) — this file only
    // serializes it, never shapes it. Any callback left unassigned (recorder not
    // yet wired) answers 503 {"ok":false,"error":...} — never a crash, never a
    // silent 200.
    struct PerfRecordOpts
    {
        juce::String name;
        bool audio = true;
        juce::String audioFile;
        bool onsetMarkers = false;
        juce::String overdubAssetId;
    };
    std::function<void(const PerfRecordOpts&)> onPerfRecord;
    std::function<void()> onPerfStop;
    std::function<void(juce::File takeFolder)> onPerfLoad;
    std::function<void(bool withAudio)> onPerfPlay;
    std::function<void()> onPerfStopPlay;
    std::function<void()> onPerfRepair;
    std::function<juce::var()> onPerfStatus;   // synchronous; see comment above
    // s-rta-0925 (probe enabler, end-of-replay plan section 5): a dev/probe control -- puts the app on
    // the live input or (if loaded) the file transport. Same marshal posture as every onPerf* callback.
    std::function<void(const juce::String& mode)> onAudioSource;

    // s-rta-0926 routines slice 1 (plan-routines-s1-final.md 5.1): /api/routine/*. Same posture as
    // /api/perf/*: every mutating route is marshalled to the message thread (callAsync) and answers
    // 503 when unassigned; the outcome (or the refusal, in whole words) is read back from
    // /api/routine/status (lastSaved / lastError), which is synchronous and reads ONLY
    // RoutineEngine::status()'s mutex-guarded copy.
    struct RoutineSaveOpts
    {
        juce::String name;
        bool useBars = false;               // true: fromBar/toBar (1-based, inclusive); false: fromBeat/toBeat
        double fromBeat = 0.0, toBeat = 0.0;
        int fromBar = 0, toBar = 0;
        int slot = -1;                      // -1 = first free pad
        std::optional<bool> loop, restoreState, wholeBars;
        juce::String quantize;              // "" = the routine default (bar)
        juce::String takeFolder;            // "" = the loaded take
    };
    struct RoutineSetOpts
    {
        int slot = -1;
        std::optional<bool> loop, restoreState;
        juce::String quantize, name;        // "" = unchanged
        juce::String restoreStyle;          // "" = unchanged; "ease" | "jump" (s-rta-0926b)
    };
    std::function<void(const RoutineSaveOpts&)> onRoutineSave;
    std::function<void(int slot)> onRoutineFire;
    std::function<void(int slot, bool all)> onRoutineStop;
    std::function<void(const RoutineSetOpts&)> onRoutineSet;
    std::function<void(int slot)> onRoutineRemove;
    std::function<juce::var()> onRoutineStatus;   // synchronous; see comment above

    // s-rta-0927 outputs-c2 (plan5 C2): /api/state.outputs.displays -- the display list of the Output menu
    // (OutputManager::stateVar(): built on the message thread, read here as a mutex-guarded copy; never
    // Desktop::getDisplays() off the message thread). Set it BEFORE start(): HTTP threads only read it.
    void setOutputsStateProvider(std::function<juce::var()> provider) { outputsStateProvider_ = std::move(provider); }
    // s-rta-0928b mediaopen: /api/state.media (MainComponent::mediaStateVar: atomics only). Set it BEFORE start().
    void setMediaStateProvider(std::function<juce::var()> provider) { mediaStateProvider_ = std::move(provider); }
    // s-rta-0929 asyncload: /api/state.load (MainComponent::loadWitnessVar: atomics + a mutex-guarded copy). Before start().
    void setLoadWitnessProvider(std::function<juce::var()> provider) { loadWitnessProvider_ = std::move(provider); }

    // s-rta-0928b mediaopen (TEST-ONLY route, AUDIODNA_BUILD_TEST_SERVER): POST /api/debug/drop_files {layer, column,
    // files[]} -- MainComponent hands the files to the handlers a Finder drop onto that cell reaches (ClipCell::classifyDrop,
    // then the same DeckView callback). Marshalled to the message thread; answers at once.
    std::function<void(int layer, int column, const std::vector<juce::File>& files)> onDebugDropFiles;

    // s-rta-0928b idlepaint (TEST-ONLY routes, AUDIODNA_BUILD_TEST_SERVER): POST /api/debug/ui_test_menu {on, x, y, kind}
    // shows (on) / removes an in-peer overlay parented to the top-level window at MainComponent point (x, y): kind "menu"
    // (default) = a 3-item PopupMenu (JUCE dismisses it within ~50 ms while the app is in the background), "panel" = a
    // plain 300 x 140 test component (it stays until {on:false}); POST /api/debug/ui_native_fallback {on} forces the
    // native-layer panels to in-peer painting (on) or back; POST /api/debug/ui_repaint_all repaints the whole
    // MainComponent. Marshalled to the message thread; answer at once.
    std::function<void(bool on, int x, int y, const juce::String& kind)> onDebugUiTestMenu;
    std::function<void(bool on)> onDebugUiNativeFallback;
    std::function<void()> onDebugUiRepaintAll;

    // s-rta-0929 asyncload (TEST-ONLY routes, AUDIODNA_BUILD_TEST_SERVER): GET /api/debug/ui_text answers the file label's
    // text (read ON the message thread; the handler waits <= 2 s -- also a responsiveness witness); POST
    // /api/debug/load_deck {path} = Load Deck... of that file (appendDeckFromFile); POST /api/debug/duplicate_deck {deck}
    // = the tab menu's Duplicate; POST /api/debug/cancel_load cancels the staged load. The three POSTs are marshalled to
    // the message thread and answer at once.
    std::function<void(juce::File)> onDebugLoadDeck;
    std::function<void(int deckIndex)> onDebugDuplicateDeck;
    std::function<void()> onDebugCancelLoad;
    // Lane bf9b (TEST-ONLY routes, ruling-bf9b amendment 4(g)): POST /api/debug/remove_deck {deck} = the tab menu's
    // Remove Deck. Marshalled to the message thread; answers at once. (Undo by REST: the ui lane's onDebugUndo below.)
    std::function<void(int deckIndex)> onDebugRemoveDeck;
    // Lane bf9b fix round (TEST-ONLY): POST /api/debug/save_composition {path} = File > Save As... to that absolute
    // path (no chooser) -- the live driver of K7 / B5's "save + reload". Marshalled to the message thread; answers at
    // once (the caller polls for the file).
    std::function<void(juce::File)> onDebugSaveComposition;
    std::function<juce::String()> onDebugUiText;
    // s-rta-0929b btguard (TEST-ONLY): /api/debug/ui_text "audio_notice" -- the no-input / no-device notice beside the
    // file label ("" when hidden), read in the same message-thread hop as file_label.
    std::function<juce::String()> onDebugAudioNotice;
    // Lane bf9b fix stage (TEST-ONLY; ruling-bf9b-merge AM-6): /api/debug/ui_text "inspected_layer" / "inspected_clip"
    // -- the name of the layer / clip the Layer / Clip inspector is bound to ("" when none) -- and "inspector_tab" --
    // the active inspector tab's name ("Clip", "Layer", "Composition", "Signal"). Same message-thread hop as file_label.
    std::function<juce::String()> onDebugInspectedLayer;
    std::function<juce::String()> onDebugInspectedClip;
    std::function<juce::String()> onDebugInspectorTab;
#if AUDIODNA_TEST_SERVER
    // s-rta-0929b btguard (TEST-ONLY route, production port, no --test-mode): GET /api/debug/audio_devices answers
    // AudioEngine::deviceStatusVar() (a mutex-guarded copy published on the message thread). Set it BEFORE start().
    void setAudioDevicesProvider(std::function<juce::var()> provider) { audioDevicesProvider_ = std::move(provider); }
    // s-rta-0930 bt2 (TEST-ONLY routes, same build path): POST /api/debug/audio_deny {"names": [...]} replaces the denied
    // device names ([] = none) and runs the guard's device-list-change path; POST /api/debug/audio_stop (no body) stops
    // the open device (the manager keeps it). Marshalled to the message thread; answer at once.
    std::function<void(const juce::StringArray& names)> onDebugAudioDeny;
    std::function<void()> onDebugAudioStop;
#endif
#if AUDIODNA_TEST_SERVER
    // s-rta-1002b ui U3.4 (TEST-ONLY routes, AUDIODNA_BUILD_TEST_SERVER; ruling-ui.md AM6) -- the deck tab row and its
    // in-place rename box. GET /api/debug/deck_tabs answers onDebugDeckTabs() read ON the message thread (<= 2 s wait):
    // {ok, active, row_width, tab_row_builds, focus_home_count, undo:{top, redo_top, index, size}, tabs:[{index, id, name,
    // label, tooltip, x, y, w, h, showing}], editor:{open, deck_id, deck_index, text, x, y, w, h}}. POST
    // /api/debug/deck_rename {deck, op, text} (op: begin | type | enter | tab | escape | focus_lost | outside_click), POST
    // /api/debug/tab_click {deck} (the tab's own onClick), POST /api/debug/tab_dblclick {deck} (a double-click in JUCE's
    // order) and POST /api/debug/undo {redo} (Edit > Undo / Redo) are marshalled to the message thread and answer at once.
    std::function<juce::var()> onDebugDeckTabs;
    std::function<void(int deckIndex, const juce::String& op, const juce::String& text)> onDebugDeckRename;
    std::function<void(int deckIndex)> onDebugTabClick;
    std::function<void(int deckIndex)> onDebugTabDoubleClick;
    std::function<void(bool redo)> onDebugUndo;
    // s-rta-1002b ui U2.6 (TEST-ONLY routes, same build path; plan-ui.md U2.6 + ruling-ui.md AM10) -- a clip's file info
    // and Show in Finder. GET /api/debug/clip_media?layer=L&column=C answers onDebugClipMedia(L, C) read ON the message
    // thread (<= 2 s wait), for the ACTIVE deck: {ok, layer, column, cell, media_type, line, lines, tooltip (the cell's
    // own), path_tip, reveal_target, file_backed, missing, video:{codec, width, height, fps}|null, menu:[...],
    // inspector_shows, inspector_line, inspector_lines, inspector_button_visible, last_revealed, reveal_count}. POST
    // /api/debug/reveal_clip {layer, column} (through the cell's menu completion, DeckView::revealCellForTests) and POST
    // /api/debug/inspect_clip {layer, column} (the cell's name-bar click) are marshalled and answer at once.
    std::function<juce::var(int layer, int column)> onDebugClipMedia;
    std::function<void(int layer, int column)> onDebugRevealClip;
    std::function<void(int layer, int column)> onDebugInspectClip;
#endif

    ApiServer(const ApiServer&) = delete;
    ApiServer& operator=(const ApiServer&) = delete;

private:
    void setupRoutes();

    // Endpoint handlers
    void handleHealth(const httplib::Request& req, httplib::Response& res);
    void handleStatus(const httplib::Request& req, httplib::Response& res);
    void handleComposition(const httplib::Request& req, httplib::Response& res);
    void handleTriggerClip(const httplib::Request& req, httplib::Response& res);
    void handleTriggerColumn(const httplib::Request& req, httplib::Response& res);
    void handleSetParam(const httplib::Request& req, httplib::Response& res);
    void handleSetClipParam(const httplib::Request& req, httplib::Response& res);
    void handleSetLayerOpacity(const httplib::Request& req, httplib::Response& res);
    void handleSetMasterSignal(const httplib::Request& req, httplib::Response& res);
    void handleSwitchDeck(const httplib::Request& req, httplib::Response& res);
    void handleSnapshot(const httplib::Request& req, httplib::Response& res);
    void handleGetBpm(const httplib::Request& req, httplib::Response& res);
    void handleSetBpm(const httplib::Request& req, httplib::Response& res);
    void handleResync(const httplib::Request& req, httplib::Response& res);
    void handleGetFeatures(const httplib::Request& req, httplib::Response& res);
    void handleInjectFeatures(const httplib::Request& req, httplib::Response& res);
    void handleLoadImage(const httplib::Request& req, httplib::Response& res);
    void handleLoadSource(const httplib::Request& req, httplib::Response& res);
    void handleLoadComposition(const httplib::Request& req, httplib::Response& res);
    void handleSetEffect(const httplib::Request& req, httplib::Response& res);
    void handleListEffects(const httplib::Request& req, httplib::Response& res);
    void handleListSources(const httplib::Request& req, httplib::Response& res);
    void handleRenderFrame(const httplib::Request& req, httplib::Response& res);
    void handleReset(const httplib::Request& req, httplib::Response& res);
    void handleSetEffectChain(const httplib::Request& req, httplib::Response& res);
    void handleState(const httplib::Request& req, httplib::Response& res);
    void handleGetSyphon(const httplib::Request& req, httplib::Response& res);
    void handleSetSyphon(const httplib::Request& req, httplib::Response& res);

    // s-rta-0923 step 3 (Lane S3-C) -- /api/perf/*
    void handlePerfRecord(const httplib::Request& req, httplib::Response& res);
    void handlePerfStop(const httplib::Request& req, httplib::Response& res);
    void handlePerfLoad(const httplib::Request& req, httplib::Response& res);
    void handlePerfPlay(const httplib::Request& req, httplib::Response& res);
    void handlePerfStopPlay(const httplib::Request& req, httplib::Response& res);
    void handlePerfRepair(const httplib::Request& req, httplib::Response& res);
    void handlePerfStatus(const httplib::Request& req, httplib::Response& res);
    void handleAudioSource(const httplib::Request& req, httplib::Response& res);
#if AUDIODNA_TEST_SERVER
    void handleDebugStallMessageThread(const httplib::Request& req, httplib::Response& res);   // s-rta-0927 beat clock (TEST-ONLY)
    // s-rta-0928b mediaopen (TEST-ONLY): the message-thread heartbeat on / off, and a Finder drop by path.
    void handleDebugHeartbeat(const httplib::Request& req, httplib::Response& res);
    void handleDebugDropFiles(const httplib::Request& req, httplib::Response& res);
    MessageHeartbeat heartbeat_;   // /api/state message_heartbeat_on / peak_message_stall_ms
    // s-rta-0928b idlepaint (TEST-ONLY): the UI paint counters (src/ui/UiPaintCounters.h) and the three UI hooks above.
    void handleDebugUiPaint(const httplib::Request& req, httplib::Response& res);
    void handleDebugUiPasses(const httplib::Request& req, httplib::Response& res);   // s-rta-0929 g4cpu
    void handleDebugUiTestMenu(const httplib::Request& req, httplib::Response& res);
    void handleDebugUiNativeFallback(const httplib::Request& req, httplib::Response& res);
    void handleDebugUiRepaintAll(const httplib::Request& req, httplib::Response& res);
    // s-rta-0929 asyncload (TEST-ONLY): see onDebugUiText / onDebugLoadDeck / onDebugDuplicateDeck / onDebugCancelLoad.
    void handleDebugUiText(const httplib::Request& req, httplib::Response& res);
    void handleDebugLoadDeck(const httplib::Request& req, httplib::Response& res);
    void handleDebugDuplicateDeck(const httplib::Request& req, httplib::Response& res);
    void handleDebugCancelLoad(const httplib::Request& req, httplib::Response& res);
    void handleDebugRemoveDeck(const httplib::Request& req, httplib::Response& res);   // lane bf9b
    void handleDebugSaveComposition(const httplib::Request& req, httplib::Response& res);   // lane bf9b fix round
    void handleDebugAudioDevices(const httplib::Request& req, httplib::Response& res);   // s-rta-0929b btguard
    void handleDebugAudioDeny(const httplib::Request& req, httplib::Response& res);      // s-rta-0930 bt2
    void handleDebugAudioStop(const httplib::Request& req, httplib::Response& res);      // s-rta-0930 bt2
    // s-rta-1002b ui U3.4 (TEST-ONLY): see onDebugDeckTabs / onDebugDeckRename / onDebugTabClick / onDebugUndo.
    void handleDebugDeckTabs(const httplib::Request& req, httplib::Response& res);
    void handleDebugDeckRename(const httplib::Request& req, httplib::Response& res);
    void handleDebugTabClick(const httplib::Request& req, httplib::Response& res);
    void handleDebugTabDoubleClick(const httplib::Request& req, httplib::Response& res);
    void handleDebugUndo(const httplib::Request& req, httplib::Response& res);
    // s-rta-1002b ui U2.6 (TEST-ONLY): see onDebugClipMedia / onDebugRevealClip / onDebugInspectClip.
    void handleDebugClipMedia(const httplib::Request& req, httplib::Response& res);
    void handleDebugRevealClip(const httplib::Request& req, httplib::Response& res);
    void handleDebugInspectClip(const httplib::Request& req, httplib::Response& res);
    std::function<juce::var()> audioDevicesProvider_;   // set before start(); see setAudioDevicesProvider
#endif

    // s-rta-0926 routines slice 1 -- /api/routine/*
    void handleRoutineSave(const httplib::Request& req, httplib::Response& res);
    void handleRoutineFire(const httplib::Request& req, httplib::Response& res);
    void handleRoutineStop(const httplib::Request& req, httplib::Response& res);
    void handleRoutineSet(const httplib::Request& req, httplib::Response& res);
    void handleRoutineRemove(const httplib::Request& req, httplib::Response& res);
    void handleRoutineStatus(const httplib::Request& req, httplib::Response& res);

    // JSON helpers
    std::string jsonOk();
    std::string jsonOk(const std::string& key, const std::string& value);
    std::string jsonError(const std::string& message);

    Renderer& renderer_;
    const FeatureBus& featureBus_;
    Composition& composition_;
    EffectChain& effectChain_;
    SourceRegistry& sourceRegistry_;
    SignalRegistry& signalRegistry_;
    RoutingEngine& routingEngine_;
    BindingManager& bindingManager_;

    std::function<juce::var()> outputsStateProvider_;   // set before start(); see setOutputsStateProvider
    std::function<juce::var()> mediaStateProvider_;     // set before start(); see setMediaStateProvider
    std::function<juce::var()> loadWitnessProvider_;    // set before start(); see setLoadWitnessProvider
    // s-rta-0929 asyncload: every load ticket a handler may be waiting on. stop() finishes them all Cancelled (and
    // refuses new ones) BEFORE httplib joins its workers -- a worker blocked in a wait would hang the quit.
    static constexpr int kLoadWaitMs = 60000;
    std::mutex ticketsMutex_;
    std::vector<std::weak_ptr<LoadTicket>> tickets_;
    bool ticketsClosed_ = false;   // guarded by ticketsMutex_
    int port_;
    // R6 (featurebus-thread-safety-design.md): production = not registered
    // (ctor flag from testMode_) so inject_features 404s outside test mode.
    bool allowFeatureInjection_;
    httplib::Server server_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
};
