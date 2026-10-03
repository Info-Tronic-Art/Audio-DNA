#include "api/ApiServer.h"
#include "core/LogLine.h"
#include "render/Renderer.h"
#include "features/FeatureBus.h"
#include "features/OnsetPulse.h"
#include "effects/EffectChain.h"
#include "effects/Effect.h"
#include "effects/EffectLibrary.h"
#include "model/Composition.h"
#include "model/Clip.h"
#include "core/CompositionLoad.h"
#include "sources/SourceRegistry.h"
#include "signal/SignalRegistry.h"
#include "routing/RoutingEngine.h"
#include "binding/BindingManager.h"
#include "connect/ScalarParams.h"
#if AUDIODNA_TEST_SERVER
#include "ui/UiPaintCounters.h"
#endif
#include <juce_core/juce_core.h>
#include <algorithm>
#include <iostream>
#include <limits>

namespace
{
// s-rta-0923 lane 3 (plan section 4.1 item 4 / section 3.6 #9-#10): the
// production oracle C5 reads. Adds "live" (every ScalarDef key -> owner.eff(s),
// the atomic twin's effective value falling back to the plain manual field —
// same eff() every renderer read (C2) and every widget (C4) uses) and
// "connected" (the subset of those keys whose ParamConnection isConnected())
// to `obj`. One template shared by Composition/Layer/Clip since all three
// expose the identical eff(ScalarEnum)/scalarConns<N> shape.
template <typename Owner, typename ScalarEnum, std::size_t N>
void addLiveBlock(juce::DynamicObject& obj, const Owner& owner,
                  const std::array<ParamConnection, N>& conns,
                  const std::array<ScalarDef, N>& defs)
{
    auto* liveObj = new juce::DynamicObject();
    juce::Array<juce::var> connectedArr;
    for (std::size_t i = 0; i < N; ++i)
    {
        auto s = static_cast<ScalarEnum>(i);
        liveObj->setProperty(juce::String(defs[i].key), static_cast<double>(owner.eff(s)));
        if (conns[i].isConnected())
            connectedArr.add(juce::String(defs[i].key));
    }
    obj.setProperty("live", juce::var(liveObj));
    obj.setProperty("connected", connectedArr);
}
} // namespace

ApiServer::ApiServer(Renderer& renderer,
                     const FeatureBus& featureBus,
                     Composition& composition,
                     EffectChain& effectChain,
                     SourceRegistry& sourceRegistry,
                     SignalRegistry& signalRegistry,
                     RoutingEngine& routingEngine,
                     BindingManager& bindingManager,
                     int port,
                     bool allowFeatureInjection)
    : renderer_(renderer)
    , featureBus_(featureBus)
    , composition_(composition)
    , effectChain_(effectChain)
    , sourceRegistry_(sourceRegistry)
    , signalRegistry_(signalRegistry)
    , routingEngine_(routingEngine)
    , bindingManager_(bindingManager)
    , port_(port)
    , allowFeatureInjection_(allowFeatureInjection)
{
    setupRoutes();
}

ApiServer::~ApiServer()
{
    stop();
}

void ApiServer::start()
{
    if (running_.load(std::memory_order_relaxed))
        return;

    // R8 (featurebus-thread-safety-design.md): bind loopback by default —
    // zero non-localhost clients exist in-repo. AUDIODNA_API_BIND overrides
    // (e.g. "0.0.0.0" or a specific interface) to restore remote-control workflows.
    std::string bindAddress = juce::SystemStats::getEnvironmentVariable(
        "AUDIODNA_API_BIND", "127.0.0.1").toStdString();

    running_.store(true, std::memory_order_relaxed);
    serverThread_ = std::thread([this, bindAddress]() {
        logLine("[API] HTTP server listening on ", bindAddress, ":", port_);
        if (!server_.listen(bindAddress, port_))
        {
            logLine("[API] Failed to start HTTP server on port ", port_);
            running_.store(false, std::memory_order_relaxed);
        }
    });
}

void ApiServer::stop()
{
    if (!running_.load(std::memory_order_relaxed))
        return;

    // s-rta-0929 asyncload: release every POST /api/load_composition waiter FIRST (Cancelled) and refuse new ones --
    // httplib joins its workers below, and the message thread (which would finish a ticket) does not pump during
    // ~MainComponent. Never waits for the message thread.
    {
        std::lock_guard<std::mutex> lk(ticketsMutex_);
        ticketsClosed_ = true;
        for (auto& w : tickets_)
            if (auto t = w.lock())
                t->finish(LoadTicket::Outcome::Cancelled);
        tickets_.clear();
    }

    server_.stop();
    if (serverThread_.joinable())
        serverThread_.join();
    running_.store(false, std::memory_order_relaxed);
    logLine("[API] HTTP server stopped");
}

// --- JSON helpers ---

std::string ApiServer::jsonOk()
{
    return R"({"ok":true})";
}

std::string ApiServer::jsonOk(const std::string& key, const std::string& value)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty(juce::String(key), juce::String(value));
    return juce::JSON::toString(juce::var(obj)).toStdString();
}

std::string ApiServer::jsonError(const std::string& message)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", false);
    obj->setProperty("error", juce::String(message));
    return juce::JSON::toString(juce::var(obj)).toStdString();
}

// --- Route setup ---

void ApiServer::setupRoutes()
{
    // CORS headers for all responses
    server_.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // Health check
    server_.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    // Status
    server_.Get("/api/status", [this](const httplib::Request& req, httplib::Response& res) {
        handleStatus(req, res);
    });

    // Composition state
    server_.Get("/api/composition", [this](const httplib::Request& req, httplib::Response& res) {
        handleComposition(req, res);
    });

    // Clip/column triggering
    server_.Post("/api/trigger_clip", [this](const httplib::Request& req, httplib::Response& res) {
        handleTriggerClip(req, res);
    });

    server_.Post("/api/trigger_column", [this](const httplib::Request& req, httplib::Response& res) {
        handleTriggerColumn(req, res);
    });

    // Parameter control
    server_.Post("/api/set_param", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetParam(req, res);
    });

    // s-rta-0926b plan-fitmode: per-clip FIELD writes (today: fitMode only).
    server_.Post("/api/set_clip_param", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetClipParam(req, res);
    });

    server_.Post("/api/set_layer_opacity", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetLayerOpacity(req, res);
    });

    server_.Post("/api/set_master_signal", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetMasterSignal(req, res);
    });

    // Deck switching
    server_.Post("/api/switch_deck", [this](const httplib::Request& req, httplib::Response& res) {
        handleSwitchDeck(req, res);
    });

    // Snapshot
    server_.Post("/api/snapshot", [this](const httplib::Request& req, httplib::Response& res) {
        handleSnapshot(req, res);
    });

    // BPM
    server_.Get("/api/bpm", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetBpm(req, res);
    });

    server_.Post("/api/set_bpm", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetBpm(req, res);
    });

    // s-rta-0925: manual Resync, same funnel as the TopBar button.
    server_.Post("/api/resync", [this](const httplib::Request& req, httplib::Response& res) {
        handleResync(req, res);
    });

    // Audio features
    server_.Get("/api/features", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFeatures(req, res);
    });

    // R6 (featurebus-thread-safety-design.md): only registered in test mode —
    // production requests 404 since the route was never added (P2 hardening).
    if (allowFeatureInjection_)
    {
        server_.Post("/api/inject_features", [this](const httplib::Request& req, httplib::Response& res) {
            handleInjectFeatures(req, res);
        });
    }

    // Media loading
    server_.Post("/api/load_image", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadImage(req, res);
    });

    server_.Post("/api/load_source", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadSource(req, res);
    });

    server_.Post("/api/load_composition", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadComposition(req, res);
    });

    // Effects
    server_.Post("/api/set_effect", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetEffect(req, res);
    });

    server_.Get("/api/effects", [this](const httplib::Request& req, httplib::Response& res) {
        handleListEffects(req, res);
    });

    server_.Get("/api/sources", [this](const httplib::Request& req, httplib::Response& res) {
        handleListSources(req, res);
    });

    // Frame capture
    server_.Post("/api/render_frame", [this](const httplib::Request& req, httplib::Response& res) {
        handleRenderFrame(req, res);
    });

    // Reset
    server_.Post("/api/reset", [this](const httplib::Request& req, httplib::Response& res) {
        handleReset(req, res);
    });

    // Batch effect chain configuration / combined state snapshot
    server_.Post("/api/set_effect_chain", [this](const httplib::Request& req, httplib::Response& res) { handleSetEffectChain(req, res); });
    server_.Get("/api/state", [this](const httplib::Request& req, httplib::Response& res) { handleState(req, res); });

    // Syphon output (P22.1) status / toggle
    server_.Get("/api/syphon", [this](const httplib::Request& req, httplib::Response& res) { handleGetSyphon(req, res); });
    server_.Post("/api/set_syphon", [this](const httplib::Request& req, httplib::Response& res) { handleSetSyphon(req, res); });

    // s-rta-0923 step 3 (Lane S3-C): performance recorder lifecycle. Pulled
    // forward from build-order row 5 -- the only production-mode arm/stop/play
    // surface that does not touch RecordPanel (step 4's file).
    server_.Post("/api/perf/record", [this](const httplib::Request& req, httplib::Response& res) { handlePerfRecord(req, res); });
    server_.Post("/api/perf/stop", [this](const httplib::Request& req, httplib::Response& res) { handlePerfStop(req, res); });
    server_.Post("/api/perf/load", [this](const httplib::Request& req, httplib::Response& res) { handlePerfLoad(req, res); });
    server_.Post("/api/perf/play", [this](const httplib::Request& req, httplib::Response& res) { handlePerfPlay(req, res); });
    server_.Post("/api/perf/stop_play", [this](const httplib::Request& req, httplib::Response& res) { handlePerfStopPlay(req, res); });
    server_.Post("/api/perf/repair", [this](const httplib::Request& req, httplib::Response& res) { handlePerfRepair(req, res); });
    server_.Get("/api/perf/status", [this](const httplib::Request& req, httplib::Response& res) { handlePerfStatus(req, res); });
    // s-rta-0925 (probe enabler, end-of-replay plan section 5): puts the app on the live input or (if
    // loaded) the file transport -- a dev/probe control, documented in the inventory row.
    server_.Post("/api/audio/source", [this](const httplib::Request& req, httplib::Response& res) { handleAudioSource(req, res); });
#if AUDIODNA_TEST_SERVER
    // s-rta-0927 beat clock (TEST-ONLY build path: absent from a build without AUDIODNA_BUILD_TEST_SERVER;
    // needs no --test-mode): sleeps the MESSAGE thread for `ms` (1..2000) -- the deterministic stall
    // probe-beatclock.sh and probe-routines row 7s use as their RED. Answers at once.
    server_.Post("/api/debug/stall_message_thread", [this](const httplib::Request& req, httplib::Response& res) { handleDebugStallMessageThread(req, res); });
    // s-rta-0928b mediaopen (TEST-ONLY, same build path): the message-thread heartbeat (/api/state peak_message_stall_ms)
    // and a Finder drop by path (the handlers ClipCell::filesDropped reaches). Both answer at once.
    server_.Post("/api/debug/heartbeat", [this](const httplib::Request& req, httplib::Response& res) { handleDebugHeartbeat(req, res); });
    server_.Post("/api/debug/drop_files", [this](const httplib::Request& req, httplib::Response& res) { handleDebugDropFiles(req, res); });
    // s-rta-0928b idlepaint (TEST-ONLY, same build path): the UI paint counters (read on the HTTP thread from atomics, no
    // message-thread hop), a parented test PopupMenu, forced native-layer fallback, a whole-MainComponent repaint.
    server_.Get("/api/debug/ui_paint", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiPaint(req, res); });
    // s-rta-0929 g4cpu (TEST-ONLY, same build path): the last display passes (clip rect + JUCE paint time + repaint
    // sources) and SignalBar change masks, read on the HTTP thread from the ring buffers (no message-thread hop).
    server_.Get("/api/debug/ui_passes", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiPasses(req, res); });
    server_.Post("/api/debug/ui_test_menu", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiTestMenu(req, res); });
    server_.Post("/api/debug/ui_native_fallback", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiNativeFallback(req, res); });
    server_.Post("/api/debug/ui_repaint_all", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiRepaintAll(req, res); });
    // s-rta-0929 asyncload (TEST-ONLY, same build path): the file label's text (read on the message thread), Load Deck /
    // Duplicate Deck by REST, and a cancel of the staged load.
    server_.Get("/api/debug/ui_text", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUiText(req, res); });
    server_.Post("/api/debug/load_deck", [this](const httplib::Request& req, httplib::Response& res) { handleDebugLoadDeck(req, res); });
    server_.Post("/api/debug/duplicate_deck", [this](const httplib::Request& req, httplib::Response& res) { handleDebugDuplicateDeck(req, res); });
    server_.Post("/api/debug/cancel_load", [this](const httplib::Request& req, httplib::Response& res) { handleDebugCancelLoad(req, res); });
    // Lane bf9b (TEST-ONLY, same build path; ruling-bf9b amendment 4(g)): Remove Deck of tab i (the tab menu's
    // function). Marshalled to the message thread; answers at once. The app's Undo by REST is the ui lane's ONE
    // POST /api/debug/undo below (Harmony ruling R-S3: no second registration).
    server_.Post("/api/debug/remove_deck", [this](const httplib::Request& req, httplib::Response& res) { handleDebugRemoveDeck(req, res); });
    // Lane bf9b fix round (TEST-ONLY, same build path): Save As... to an absolute path, no chooser (K7 / B5 save half).
    server_.Post("/api/debug/save_composition", [this](const httplib::Request& req, httplib::Response& res) { handleDebugSaveComposition(req, res); });
    // s-rta-0929b btguard (TEST-ONLY, same build path): the audio device policy's last scan and the opened devices.
    server_.Get("/api/debug/audio_devices", [this](const httplib::Request& req, httplib::Response& res) { handleDebugAudioDevices(req, res); });
    // s-rta-0930 bt2 (TEST-ONLY, same build path): swap the denied device names at runtime (the plug / unplug stand-in)
    // and stop the open device (the dead-input stand-in). Both answer at once.
    server_.Post("/api/debug/audio_deny", [this](const httplib::Request& req, httplib::Response& res) { handleDebugAudioDeny(req, res); });
    server_.Post("/api/debug/audio_stop", [this](const httplib::Request& req, httplib::Response& res) { handleDebugAudioStop(req, res); });
    // s-rta-1002b ui U3.4 (TEST-ONLY, same build path; ruling-ui.md AM6): the deck tab row and its in-place rename box.
    server_.Get("/api/debug/deck_tabs", [this](const httplib::Request& req, httplib::Response& res) { handleDebugDeckTabs(req, res); });
    server_.Post("/api/debug/deck_rename", [this](const httplib::Request& req, httplib::Response& res) { handleDebugDeckRename(req, res); });
    server_.Post("/api/debug/tab_click", [this](const httplib::Request& req, httplib::Response& res) { handleDebugTabClick(req, res); });
    server_.Post("/api/debug/tab_dblclick", [this](const httplib::Request& req, httplib::Response& res) { handleDebugTabDoubleClick(req, res); });
    server_.Post("/api/debug/undo", [this](const httplib::Request& req, httplib::Response& res) { handleDebugUndo(req, res); });
    // s-rta-1002b ui U2.6 (TEST-ONLY, same build path; plan-ui.md U2.6 + ruling-ui.md AM10): a clip's file info + Show in Finder.
    server_.Get("/api/debug/clip_media", [this](const httplib::Request& req, httplib::Response& res) { handleDebugClipMedia(req, res); });
    server_.Post("/api/debug/reveal_clip", [this](const httplib::Request& req, httplib::Response& res) { handleDebugRevealClip(req, res); });
    server_.Post("/api/debug/inspect_clip", [this](const httplib::Request& req, httplib::Response& res) { handleDebugInspectClip(req, res); });
#endif

    // s-rta-0926 routines slice 1 (plan-routines-s1-final.md 5.1): save a slice of the loaded take
    // as a routine on a pad, fire / stop / edit / remove it, and read the bank back.
    server_.Post("/api/routine/save", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineSave(req, res); });
    server_.Post("/api/routine/fire", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineFire(req, res); });
    server_.Post("/api/routine/stop", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineStop(req, res); });
    server_.Post("/api/routine/set", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineSet(req, res); });
    server_.Post("/api/routine/remove", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineRemove(req, res); });
    server_.Get("/api/routine/status", [this](const httplib::Request& req, httplib::Response& res) { handleRoutineStatus(req, res); });
}

// --- Endpoint handlers ---

void ApiServer::handleHealth(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("status", "ready");
    obj->setProperty("version", "0.1.0");
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("effects_count", effectChain_.getNumEffects());
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleStatus(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("frameTimeMs", static_cast<double>(renderer_.getFrameTimeMs()));
    // s-rta-0925: masterLevel is now the one master (composition_.eff() --
    // the fader is a widget-grip view of the same CompScalar::Opacity).
    obj->setProperty("masterLevel", static_cast<double>(composition_.eff(CompScalar::Opacity)));
    // s-rta-0925 mastersignal Step 1: same pattern, for the Signal fader.
    obj->setProperty("masterSignal", static_cast<double>(composition_.eff(CompScalar::Signal)));
    obj->setProperty("activeDeck", composition_.activeDeckIndex.load());
    // Onset render-path fix: frames on which the main Renderer's onset pulse
    // fired. Live oracle: after a click train its delta must EQUAL the
    // /api/features onsetCount delta (one pulse frame per onset at any fps).
    obj->setProperty("renderOnsetPulses", static_cast<juce::int64>(renderer_.getOnsetPulseFrames()));

    // BPM info from feature bus (R5: caller-owned value copy)
    const FeatureSnapshot snap = featureBus_.read();
    obj->setProperty("bpm", static_cast<double>(snap.bpm));
    obj->setProperty("beatPhase", static_cast<double>(snap.beatPhase));
    obj->setProperty("barPhase", static_cast<double>(snap.barPhase));
    obj->setProperty("phrasePhase", static_cast<double>(snap.phrasePhase));
    obj->setProperty("structuralState", static_cast<int>(snap.structuralState));
    obj->setProperty("detectedGenre", static_cast<int>(snap.detectedGenre));
    obj->setProperty("genreConfidence", static_cast<double>(snap.genreConfidence));
    obj->setProperty("energyState", static_cast<int>(snap.energyState));

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleComposition(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("activeDeck", composition_.activeDeckIndex.load());
    obj->setProperty("numDecks", static_cast<int>(composition_.decks.size()));
    obj->setProperty("masterOpacity", static_cast<double>(composition_.masterOpacity));
    // s-rta-0925 mastersignal Step 1: the raw field, next to masterOpacity;
    // live.signal / connected come for free from addLiveBlock below.
    obj->setProperty("masterSignal", static_cast<double>(composition_.masterSignal));
    // s-rta-0923 lane 3: the production oracle (plan section 4.1 item 4;
    // C5's probe-lane3.sh reads d['live']['positionX'] etc). Absent on the
    // pre-change binary — that absence IS C5's fail-first gate.
    addLiveBlock<Composition, CompScalar>(*obj, composition_, composition_.scalarConns, compScalarDefs());

    // Lane bf9b (plan-bf9b F8, ruling-bf9b amendment 4(g)): the TRUTH is the top-level "layers" -- the shared stack,
    // each layer's settings and what it plays as {deck, deckId, column, clipId, retired} refs -- plus
    // "retiredDeckCount". decks[d].layers[l] stays as a legacy MIRROR: the shared layer's settings, its
    // activeClipColumn / previousClipColumn as seen from deck d (-1 when the ref names another deck), and deck d's row-l
    // clips. REST never reads a retired deck (a ref into one reports retired: true, clipId -1).
    obj->setProperty("retiredDeckCount", composition_.getNumRetiredDecks());
    auto refVar = [this](ClipRef ref, int row) {
        auto* r = new juce::DynamicObject();
        const int deckIndex = ref.column >= 0 ? composition_.findDeckIndexById(ref.deckId) : -1;
        r->setProperty("deck", deckIndex);
        r->setProperty("deckId", ref.column >= 0 ? static_cast<int>(ref.deckId) : -1);
        r->setProperty("column", ref.column);
        const Clip* clip = deckIndex >= 0 ? composition_.decks[static_cast<size_t>(deckIndex)].getClip(row, ref.column)
                                          : nullptr;
        r->setProperty("clipId", clip != nullptr ? static_cast<int>(clip->id) : -1);
        r->setProperty("retired", ref.column >= 0 && deckIndex < 0);
        return juce::var(r);
    };
    juce::Array<juce::var> sharedArray;
    for (int li = 0; li < composition_.getNumLayers(); ++li)
    {
        const Layer& layer = composition_.layers[static_cast<size_t>(li)];
        auto* layerObj = new juce::DynamicObject();
        layerObj->setProperty("index", li);
        layerObj->setProperty("id", static_cast<int>(layer.id));
        layerObj->setProperty("name", juce::String(layer.name));
        layerObj->setProperty("type", static_cast<int>(layer.type));
        layerObj->setProperty("opacity", static_cast<double>(layer.opacity));
        layerObj->setProperty("visible", layer.visible);
        layerObj->setProperty("muted", layer.muted);
        layerObj->setProperty("solo", layer.solo);
        layerObj->setProperty("bypassed", layer.bypassed);
        layerObj->setProperty("ignoreColumnTrigger", layer.ignoreColumnTrigger);
        layerObj->setProperty("blendMode", static_cast<int>(layer.blendMode));
        const LayerRuntimeSnapshot rt = layer.runtime();   // one consistent tuple (lane tsan)
        layerObj->setProperty("activeClip", refVar(rt.activeRef(), li));
        layerObj->setProperty("previousClip", refVar(rt.previousRef(), li));
        layerObj->setProperty("pendingClip", refVar(rt.pendingRef(), li));
        layerObj->setProperty("crossfadeProgress", static_cast<double>(rt.crossfadeProgress));
        addLiveBlock<Layer, LayerScalar>(*layerObj, layer, layer.scalarConns, layerScalarDefs());
        sharedArray.add(juce::var(layerObj));
    }
    obj->setProperty("layers", sharedArray);

    // Deck details (the boxes; their "layers" are the legacy mirror)
    juce::Array<juce::var> deckArray;
    for (size_t di = 0; di < composition_.decks.size(); ++di)
    {
        auto& deck = composition_.decks[di];
        auto* deckObj = new juce::DynamicObject();
        deckObj->setProperty("name", juce::String(deck.name));
        deckObj->setProperty("id", static_cast<int>(deck.id));
        deckObj->setProperty("numLayers", deck.getNumRows());
        deckObj->setProperty("numColumns", deck.numColumns);

        juce::Array<juce::var> layerArray;
        for (int li = 0; li < deck.getNumRows() && li < composition_.getNumLayers(); ++li)
        {
            auto& layer = composition_.layers[static_cast<size_t>(li)];
            const ClipRow& row = deck.rows[static_cast<size_t>(li)];
            auto* layerObj = new juce::DynamicObject();
            layerObj->setProperty("id", static_cast<int>(layer.id));
            layerObj->setProperty("name", juce::String(layer.name));
            layerObj->setProperty("opacity", static_cast<double>(layer.opacity));
            layerObj->setProperty("visible", layer.visible);
            layerObj->setProperty("muted", layer.muted);
            layerObj->setProperty("solo", layer.solo);
            layerObj->setProperty("bypassed", layer.bypassed);
            const LayerRuntimeSnapshot rt = layer.runtime();   // one consistent tuple (lane tsan)
            // The mirror: a column of THIS deck, -1 when the ref names another deck (bf9b F8).
            layerObj->setProperty("activeClipColumn", rt.activeDeckId == deck.id ? rt.activeClipColumn : -1);
            // s-rta-0926b plan4 T7: each layer's fade state, witnessable over REST.
            layerObj->setProperty("previousClipColumn", rt.previousDeckId == deck.id ? rt.previousClipColumn : -1);
            layerObj->setProperty("crossfadeProgress", static_cast<double>(rt.crossfadeProgress));
            layerObj->setProperty("blendMode", static_cast<int>(layer.blendMode));
            addLiveBlock<Layer, LayerScalar>(*layerObj, layer, layer.scalarConns, layerScalarDefs());

            juce::Array<juce::var> clipArray;
            for (size_t ci = 0; ci < row.clips.size(); ++ci)
            {
                if (row.clips[ci].has_value())
                {
                    auto& clip = *row.clips[ci];
                    auto* clipObj = new juce::DynamicObject();
                    clipObj->setProperty("id", static_cast<int>(clip.id));
                    clipObj->setProperty("name", juce::String(clip.name));
                    clipObj->setProperty("column", static_cast<int>(ci));
                    clipObj->setProperty("playing", clip.playing.load());
                    clipObj->setProperty("fitMode", static_cast<int>(clip.fitMode));   // plan-fitmode
                    clipObj->setProperty("playheadPosition", clip.playheadPosition.load());   // plan4 T7
                    clipObj->setProperty("mediaType", static_cast<int>(clip.mediaType));
                    clipObj->setProperty("sourceType", juce::String(clip.sourceType));
                    // s-rta-0928b mediaopen: presence (Clip::mediaMissing, the 1 Hz sweep), the media's size, and the
                    // grid thumbnail's size (0 x 0 = none) -- the witnesses of a drop / load / presence change.
                    clipObj->setProperty("mediaMissing", clip.mediaMissing);
                    clipObj->setProperty("clipWidth", clip.clipWidth);
                    clipObj->setProperty("clipHeight", clip.clipHeight);
                    clipObj->setProperty("thumbnailW", clip.thumbnail.getWidth());
                    clipObj->setProperty("thumbnailH", clip.thumbnail.getHeight());
                    addLiveBlock<Clip, ClipScalar>(*clipObj, clip, clip.scalarConns, clipScalarDefs());

                    // s-rta-0926 routines (plan 5.1, additive): the clip's effect stack with each
                    // parameter's EFFECTIVE value (effParam -- what the renderer reads), named from
                    // the EffectLibrary def the same way handleSetParam resolves them.
                    juce::Array<juce::var> fxArray;
                    for (const auto& fx : clip.effects)
                    {
                        auto* fxObj = new juce::DynamicObject();
                        fxObj->setProperty("name", juce::String(fx.effectName));
                        fxObj->setProperty("bypassed", fx.bypassed);
                        const auto* def = renderer_.getEffectLibrary().getEffectDef(juce::String(fx.effectName));
                        juce::Array<juce::var> paramArray;
                        for (size_t pi = 0; pi < fx.paramValues.size(); ++pi)
                        {
                            auto* pObj = new juce::DynamicObject();
                            pObj->setProperty("name", (def != nullptr && pi < def->params.size())
                                                          ? juce::String(def->params[pi].name) : juce::String());
                            pObj->setProperty("value", static_cast<double>(fx.effParam(pi)));
                            paramArray.add(juce::var(pObj));
                        }
                        fxObj->setProperty("params", paramArray);
                        fxArray.add(juce::var(fxObj));
                    }
                    clipObj->setProperty("effects", fxArray);
                    clipArray.add(juce::var(clipObj));
                }
            }
            layerObj->setProperty("clips", clipArray);
            layerArray.add(juce::var(layerObj));
        }
        deckObj->setProperty("layers", layerArray);
        deckArray.add(juce::var(deckObj));
    }
    obj->setProperty("decks", deckArray);

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleTriggerClip(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int layer = static_cast<int>(json.getProperty("layer", -1));
    int column = static_cast<int>(json.getProperty("column", -1));

    if (layer < 0 || column < 0)
    {
        res.set_content(jsonError("Missing 'layer' or 'column'"), "application/json");
        return;
    }

    if (onTriggerClip)
    {
        // `this`-capture safety: see handleSetParam's clip-effect branch note.
        juce::MessageManager::callAsync([this, layer, column]() {
            onTriggerClip(layer, column);
        });
    }

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleTriggerColumn(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int column = static_cast<int>(json.getProperty("column", -1));

    if (column < 0)
    {
        res.set_content(jsonError("Missing 'column'"), "application/json");
        return;
    }

    if (onTriggerColumn)
    {
        // `this`-capture safety: see handleSetParam's clip-effect branch note.
        juce::MessageManager::callAsync([this, column]() {
            onTriggerColumn(column);
        });
    }

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSetParam(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int layer = static_cast<int>(json.getProperty("layer", -1));
    int column = static_cast<int>(json.getProperty("column", -1));
    juce::String effectName = json.getProperty("effect", "").toString();
    juce::String paramName = json.getProperty("param", "").toString();
    float value = static_cast<float>(static_cast<double>(json.getProperty("value", 0.0)));

    if (effectName.isEmpty() || paramName.isEmpty())
    {
        res.set_content(jsonError("Missing 'effect' or 'param'"), "application/json");
        return;
    }

    // If layer/column specified, target clip effect; otherwise target global effect chain
    if (layer >= 0 && column >= 0)
    {
        // Clip effects live on the composition model, which is otherwise mutated
        // only on the message thread — marshal this write there too, same
        // callAsync/fire-and-forget shape as trigger_clip/trigger_column/
        // switch_deck/set_bpm. Model-state validation (deck/layer/clip/effect
        // lookup) now happens on the message thread, so it can no longer be
        // reported back synchronously; the response is unconditional 'ok' once
        // the request itself is well-formed (matches those sibling endpoints).
        //
        // Raw `this` capture is safe even across ApiServer/MainComponent
        // teardown: MessageManager::MessageBase::post() checks
        // quitMessagePosted and refuses (and destroys) queued callbacks once
        // app quit begins; ~MainComponent() only runs after the message
        // dispatch loop has exited; and ApiServer::stop() joins every
        // httplib worker thread before returning, so no handler can still be
        // in flight when ApiServer itself is torn down. Same reasoning
        // applies at every other callAsync site in this file, including the
        // 4 pre-existing ones (trigger_clip/trigger_column/switch_deck/
        // set_bpm) — this comment is the shared reference for all 10.
        // LOAD-BEARING INVARIANT: this safety depends on ApiServer having
        // exactly ONE lifecycle — constructed once, stop()'d exactly once,
        // post-quit-signal, via the single stop() call in ~MainComponent().
        // A future second lifecycle path (hot-restart, port-conflict
        // reconfigure, anything that stop()s/restarts ApiServer while the
        // app keeps running) would reopen a UAF across all 10 sites at once,
        // since quitMessagePosted would not yet be set to protect them.
        // s-rta-0923 lane 3 (plan section 3.6, site #10): keep the name->index
        // resolution here (it needs renderer_.getEffectLibrary(), which this
        // lambda already captures via `this`); the inline
        // `fx.paramValues[pi] = value;` write is REMOVED — resolution ends
        // by firing onSetClipEffectParam, which MainComponent routes through
        // manualWrite (Decaying rank, Origin::Human).
        juce::MessageManager::callAsync([this, layer, column, effectName, paramName, value]() {
            auto* deck = composition_.getActiveDeck();   // a cell of the SHOWN deck (bf9b R6: a box context)
            if (!deck)
                return;
            auto* clip = deck->getClip(layer, column);
            if (!clip)
                return;

            for (size_t fi = 0; fi < clip->effects.size(); ++fi)
            {
                auto& fx = clip->effects[fi];
                if (fx.effectName == effectName.toStdString())
                {
                    // Look up param index by name from EffectLibrary
                    auto& lib = renderer_.getEffectLibrary();
                    auto* def = lib.getEffectDef(juce::String(fx.effectName));
                    if (def)
                    {
                        for (size_t pi = 0; pi < def->params.size(); ++pi)
                        {
                            if (def->params[pi].name == paramName.toStdString())
                            {
                                if (pi < fx.paramValues.size() && onSetClipEffectParam)
                                    onSetClipEffectParam(layer, column, static_cast<int>(fi),
                                                         static_cast<int>(pi), paramName.toStdString(), value);
                                return;
                            }
                        }
                    }
                }
            }
        });

        res.set_content(jsonOk(), "application/json");
    }
    else
    {
        // Global effect chain — same callAsync marshal as the clip branch
        // above; effectChain_ is otherwise mutated only on the message thread.
        // `this`-capture safety: see the clip-effect branch's note above.
        juce::MessageManager::callAsync([this, effectName, paramName, value]() {
            for (int i = 0; i < effectChain_.getNumEffects(); ++i)
            {
                auto* fx = effectChain_.getEffect(i);
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
        });

        res.set_content(jsonOk(), "application/json");
    }
}

void ApiServer::handleSetClipParam(const httplib::Request& req, httplib::Response& res)
{
    // s-rta-0926b plan-fitmode: {"layer": L, "column": C, "param": "fitMode", "value": 0|1|2}. The shape
    // is checked here on the HTTP thread; the write is marshalled to the message thread (active deck, like
    // set_param) and the response is unconditional 'ok' once the request is well-formed -- the sibling
    // class. Not undo-recorded, like every other per-clip field write.
    auto json = juce::JSON::parse(juce::String(req.body));
    const int layer = static_cast<int>(json.getProperty("layer", -1));
    const int column = static_cast<int>(json.getProperty("column", -1));
    const juce::String param = json.getProperty("param", "").toString();
    const juce::var value = json.getProperty("value", juce::var());

    if (layer < 0 || column < 0)
    {
        res.set_content(jsonError("Missing 'layer' or 'column'"), "application/json");
        return;
    }
    if (param != "fitMode")
    {
        res.set_content(jsonError("unknown param"), "application/json");
        return;
    }
    if (!(value.isInt() || value.isInt64()) || static_cast<int>(value) < 0 || static_cast<int>(value) > 2)
    {
        res.set_content(jsonError("'value' must be an int 0..2 (0 Stretch, 1 Bars, 2 Crop)"), "application/json");
        return;
    }
    const int mode = static_cast<int>(value);

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, layer, column, mode]() {
        if (onSetClipFitMode)
            onSetClipFitMode(layer, column, mode);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSetLayerOpacity(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int layer = static_cast<int>(json.getProperty("layer", -1));
    float opacity = static_cast<float>(static_cast<double>(json.getProperty("opacity", 1.0)));

    if (layer < 0)
    {
        res.set_content(jsonError("Missing 'layer'"), "application/json");
        return;
    }

    // Layer state lives on the composition model, otherwise mutated only on
    // the message thread — marshal the write there too (same callAsync/
    // fire-and-forget shape as trigger_clip/trigger_column/switch_deck/
    // set_bpm). s-rta-0923 lane 3 (plan section 3.6, site #9): the inline
    // `lay->opacity = opacity;` write is REMOVED — this endpoint now only
    // fires onSetLayerOpacity, which MainComponent routes through
    // manualWrite (Decaying rank, Origin::Human) so a REST write joins the
    // same D8 grip chain as every other writer. Deck/layer validation still
    // happens on the message thread (inside the callback) and can no longer
    // be reported back synchronously — response is unconditional 'ok' once
    // the request itself is well-formed, matching the sibling endpoints.
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, layer, opacity]() {
        if (onSetLayerOpacity)
            onSetLayerOpacity(layer, opacity);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSetMasterSignal(const httplib::Request& req, httplib::Response& res)
{
    // s-rta-0925 mastersignal Step 1: same shape as handleSetLayerOpacity --
    // this handler only parses/validates; the write itself is routed
    // through MainComponent::manualWrite (Decaying rank, Origin::Human) so
    // a REST write joins the same D8 grip chain as every other writer.
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("value"))
    {
        res.set_content(jsonError("Missing 'value'"), "application/json");
        return;
    }
    float depth = static_cast<float>(static_cast<double>(json.getProperty("value", 1.0)));

    juce::MessageManager::callAsync([this, depth]() {
        if (onSetMasterSignal)
            onSetMasterSignal(depth);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSwitchDeck(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int deckIdx = static_cast<int>(json.getProperty("deck", -1));

    if (deckIdx < 0)
    {
        res.set_content(jsonError("Missing 'deck'"), "application/json");
        return;
    }

    if (onSwitchDeck)
    {
        // `this`-capture safety: see handleSetParam's clip-effect branch note.
        juce::MessageManager::callAsync([this, deckIdx]() {
            onSwitchDeck(deckIdx);
        });
    }

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSnapshot(const httplib::Request&, httplib::Response& res)
{
    // Take snapshot on a background thread (captureFrame blocks)
    auto file = renderer_.takeSnapshot();
    if (file.existsAsFile())
        res.set_content(jsonOk("file", file.getFullPathName().toStdString()), "application/json");
    else
        res.set_content(jsonError("Snapshot failed"), "application/json");
}

void ApiServer::handleGetBpm(const httplib::Request&, httplib::Response& res)
{
    const FeatureSnapshot snap = featureBus_.read();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("bpm", static_cast<double>(snap.bpm));
    obj->setProperty("beatPhase", static_cast<double>(snap.beatPhase));
    obj->setProperty("barPhase", static_cast<double>(snap.barPhase));
    obj->setProperty("phrasePhase", static_cast<double>(snap.phrasePhase));
    obj->setProperty("beatInBar", static_cast<int>(snap.beatInBar));
    obj->setProperty("barCount", static_cast<int>(snap.barCount));
    // s-rta-0925: the downbeat LEVEL (held for the whole first beat -- see
    // FeatureSnapshot::downbeatDetected), read in the SAME snapshot as
    // totalBarCount below so a poller can compare its own rising-edge count
    // with the counter delta from one coherent read (probe-downbeat-level.sh).
    obj->setProperty("downbeatDetected", snap.downbeatDetected);
    // S168: additive twin of barCount -- never rewound by a structural
    // reset. Needed so a live sweep can prove the ConnectionShaper/
    // OscillatorSignal/EnvelopeSignal monotonic-fold fix through the app,
    // not just in a unit test.
    obj->setProperty("totalBarCount", static_cast<int>(snap.totalBarCount));
    // s-rta-0925: totalBarCount at the last MANUAL Resync (0 until the first one) -- read in the
    // same coherent snapshot so a poller can compute barsSinceResync() itself.
    obj->setProperty("resyncBarOrigin", static_cast<int>(snap.resyncBarOrigin));
    obj->setProperty("totalBeatCount", static_cast<juce::int64>(snap.totalBeatCount));   // s-rta-0927 beat clock
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleSetBpm(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    float bpm = static_cast<float>(static_cast<double>(json.getProperty("bpm", 0.0)));

    if (bpm <= 0.0f)
    {
        res.set_content(jsonError("Missing or invalid 'bpm'"), "application/json");
        return;
    }

    // Marshal to the message thread — same manual-BPM override the TopBar uses.
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    if (onSetBpm)
    {
        juce::MessageManager::callAsync([this, bpm]() {
            onSetBpm(bpm);
        });
    }

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleResync(const httplib::Request&, httplib::Response& res)
{
    // s-rta-0925: bodyless POST -- no body parsing needed (pitfall 31, httplib v0.57.1
    // answers an unframed body immediately; no CPPHTTPLIB_SERVER_READ_TIMEOUT stall).
    // Marshal to the message thread — same manual-Resync path the TopBar button uses.
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    if (onResync)
    {
        juce::MessageManager::callAsync([this]() {
            onResync();
        });
    }

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleGetFeatures(const httplib::Request&, httplib::Response& res)
{
    const FeatureSnapshot snap = featureBus_.read();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);

    obj->setProperty("rms", static_cast<double>(snap.rms));
    obj->setProperty("peak", static_cast<double>(snap.peak));
    obj->setProperty("rmsDB", static_cast<double>(snap.rmsDB));
    obj->setProperty("lufs", static_cast<double>(snap.lufs));
    obj->setProperty("spectralCentroid", static_cast<double>(snap.spectralCentroid));
    obj->setProperty("spectralFlux", static_cast<double>(snap.spectralFlux));
    obj->setProperty("spectralFlatness", static_cast<double>(snap.spectralFlatness));
    obj->setProperty("bpm", static_cast<double>(snap.bpm));
    obj->setProperty("beatPhase", static_cast<double>(snap.beatPhase));
    // onsetDetected is the latest analysis hop's one-hop flag -- rate-
    // dependent for a poller (missed below ~93.75 Hz, repeated above). A
    // client that must count onsets diffs onsetCount (unsigned 32-bit,
    // monotonic for the process lifetime) between its own polls: HTTP polls
    // carry no client identity, so this stateless counter is the only
    // multi-client-correct semantic (onset render-path fix). int64 so the
    // value never reads negative.
    obj->setProperty("onsetDetected", snap.onsetDetected);
    obj->setProperty("onsetCount", static_cast<juce::int64>(snap.onsetCount));
    obj->setProperty("onsetStrength", static_cast<double>(snap.onsetStrength));
    obj->setProperty("dominantPitch", static_cast<double>(snap.dominantPitch));
    obj->setProperty("structuralState", static_cast<int>(snap.structuralState));

    // P23: Genre detection
    obj->setProperty("detectedGenre", static_cast<int>(snap.detectedGenre));
    obj->setProperty("genreConfidence", static_cast<double>(snap.genreConfidence));
    obj->setProperty("energyState", static_cast<int>(snap.energyState));

    // R13 (lane D): provenance -- the device rate the analysis was actually
    // fed from (analysis itself always runs at the fixed internal 48 kHz;
    // AnalysisResampler bridges the two) and which bandEnergies bits are
    // meaningful at that source rate (0x7F = all valid).
    obj->setProperty("sourceSampleRate", static_cast<double>(snap.sourceSampleRate));
    obj->setProperty("bandValidMask", static_cast<int>(snap.bandValidMask));

    juce::Array<juce::var> bands;
    for (int i = 0; i < 7; ++i)
        bands.add(static_cast<double>(snap.bandEnergies[i]));
    obj->setProperty("bandEnergies", bands);

    juce::Array<juce::var> chroma;
    for (int i = 0; i < 12; ++i)
        chroma.add(static_cast<double>(snap.chromagram[i]));
    obj->setProperty("chromagram", chroma);

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleInjectFeatures(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));

    // Start from the current published snapshot, override provided fields
    FeatureSnapshot snap = featureBus_.read();

    // Override with provided values
    if (json.hasProperty("rms")) snap.rms = static_cast<float>(static_cast<double>(json["rms"]));
    if (json.hasProperty("peak")) snap.peak = static_cast<float>(static_cast<double>(json["peak"]));
    if (json.hasProperty("rmsDB")) snap.rmsDB = static_cast<float>(static_cast<double>(json["rmsDB"]));
    if (json.hasProperty("bpm")) snap.bpm = static_cast<float>(static_cast<double>(json["bpm"]));
    if (json.hasProperty("beatPhase")) snap.beatPhase = static_cast<float>(static_cast<double>(json["beatPhase"]));
    if (json.hasProperty("barPhase")) snap.barPhase = static_cast<float>(static_cast<double>(json["barPhase"]));
    if (json.hasProperty("phrasePhase")) snap.phrasePhase = static_cast<float>(static_cast<double>(json["phrasePhase"]));
    // S166-L5a: same clamp-to-real-range precedent as R6 below
    // (structuralState/detectedGenre) -- FeatureSnapshot.h documents
    // beatInBar as 0-3 (which beat in the bar) and barCount as uint16_t
    // (bars since last phrase reset). Needed so /api/signals can be swept
    // from outside to prove the OscillatorSignal/EnvelopeSignal bar-fold fix.
    if (json.hasProperty("beatInBar"))
        snap.beatInBar = static_cast<uint8_t>(std::clamp(static_cast<int>(json["beatInBar"]), 0, 3));
    if (json.hasProperty("barCount"))
        snap.barCount = static_cast<uint16_t>(std::clamp(static_cast<int>(json["barCount"]), 0, 65535));
    // S168: additive twin of barCount -- see FeatureSnapshot.h. Same clamp
    // precedent as barCount above; juce::var's int property is 32-bit
    // signed, so clamp to INT_MAX rather than totalBarCount's real
    // (uint32_t) range -- no legitimate sweep needs bars beyond that.
    if (json.hasProperty("totalBarCount"))
        snap.totalBarCount = static_cast<uint32_t>(std::clamp(static_cast<int>(json["totalBarCount"]), 0,
                                                                std::numeric_limits<int>::max()));
    // s-rta-0925: same clamp precedent as totalBarCount above -- needed so a live sweep can drive
    // barsSinceResync() directly through the OscillatorSignal/EnvelopeSignal/ConnectionShaper fold.
    if (json.hasProperty("resyncBarOrigin"))
        snap.resyncBarOrigin = static_cast<uint32_t>(std::clamp(static_cast<int>(json["resyncBarOrigin"]), 0,
                                                                  std::numeric_limits<int>::max()));
    // s-rta-0927 beat clock: same clamp precedent -- an injected moving beatPhase must move totalBeatCount
    // with it, or the routine/take clock reads every wrap as a realign and freezes (Pitfall 42).
    if (json.hasProperty("totalBeatCount"))
        snap.totalBeatCount = static_cast<uint32_t>(std::clamp(static_cast<int>(json["totalBeatCount"]), 0,
                                                                 std::numeric_limits<int>::max()));
    if (json.hasProperty("spectralCentroid")) snap.spectralCentroid = static_cast<float>(static_cast<double>(json["spectralCentroid"]));
    if (json.hasProperty("spectralFlux")) snap.spectralFlux = static_cast<float>(static_cast<double>(json["spectralFlux"]));
    if (json.hasProperty("onsetStrength")) snap.onsetStrength = static_cast<float>(static_cast<double>(json["onsetStrength"]));
    if (json.hasProperty("onsetDetected")) snap.onsetDetected = static_cast<bool>(json["onsetDetected"]);
    // Onset render-path fix: the request's onset INTENT only -- the count
    // itself is resolved under TestServer::injectSnapshot's lock (never from
    // this unlocked bus read), so concurrent injects cannot lose a bump.
    // Bump only when THIS request explicitly set onsetDetected:true, so an
    // onsetDetected=true carried over from an earlier inject is no phantom.
    InjectedOnsetCount onsetIntent;
    if (json.hasProperty("onsetCount"))
    {
        onsetIntent.hasExplicit = true;
        onsetIntent.explicitCount = static_cast<uint32_t>(std::clamp(static_cast<int>(json["onsetCount"]), 0,
                                                                     std::numeric_limits<int>::max()));
    }
    onsetIntent.bump = json.hasProperty("onsetDetected") && snap.onsetDetected;
    // R6 (featurebus-thread-safety-design.md): clamp to the enum's real range
    // (FeatureSnapshot.h) so even the test-mode injection path can't store
    // nonsense values — 0=normal, 1=buildup, 2=drop, 3=breakdown.
    if (json.hasProperty("structuralState"))
        snap.structuralState = static_cast<uint8_t>(std::clamp(static_cast<int>(json["structuralState"]), 0, 3));

    // P23: Genre detection fields
    // 0=House, 1=Techno, 2=DnB, 3=HipHop, 4=Ambient, 5=Rock, 6=Pop/Electronic, 7=Jazz/Other
    if (json.hasProperty("detectedGenre"))
        snap.detectedGenre = static_cast<uint8_t>(std::clamp(static_cast<int>(json["detectedGenre"]), 0, 7));
    if (json.hasProperty("genreConfidence")) snap.genreConfidence = static_cast<float>(static_cast<double>(json["genreConfidence"]));
    // 0=low, 1=medium, 2=high
    if (json.hasProperty("energyState"))
        snap.energyState = static_cast<uint8_t>(std::clamp(static_cast<int>(json["energyState"]), 0, 2));

    if (json.hasProperty("bandEnergies"))
    {
        auto* arr = json["bandEnergies"].getArray();
        if (arr)
            for (int i = 0; i < std::min(7, arr->size()); ++i)
                snap.bandEnergies[i] = static_cast<float>(static_cast<double>((*arr)[i]));
    }

    // R4: this server holds no writer — relay to the TestServer-held Writer.
    // Only reachable in test mode (route registration is gated on
    // allowFeatureInjection_), where MainComponent wires the callback.
    if (!onInjectFeatures)
    {
        res.status = 503;
        res.set_content(jsonError("Feature injection unavailable (no test-mode writer)"), "application/json");
        return;
    }
    onInjectFeatures(snap, onsetIntent);
    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleLoadImage(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String filepath = json.getProperty("filepath", "").toString();

    if (filepath.isEmpty())
    {
        res.set_content(jsonError("Missing 'filepath'"), "application/json");
        return;
    }

    juce::File f(filepath);
    if (!f.existsAsFile())
    {
        res.set_content(jsonError("File not found"), "application/json");
        return;
    }

    renderer_.loadImage(f);

    // Give the GL thread time to process the pending image load
    juce::Thread::sleep(100);

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleLoadSource(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String sourceType = json.getProperty("source_type", "").toString();

    if (sourceType.isEmpty())
    {
        res.set_content(jsonError("Missing 'source_type'"), "application/json");
        return;
    }

    std::vector<Clip::SourceParam> params;
    if (json.hasProperty("params"))
    {
        auto* paramsObj = json["params"].getDynamicObject();
        if (paramsObj)
        {
            for (const auto& prop : paramsObj->getProperties())
                params.push_back({prop.name.toString().toStdString(), "",
                                  static_cast<float>(static_cast<double>(prop.value)), 0.5f});
        }
    }

    renderer_.setActiveSource(sourceType.toStdString(), params);
    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleLoadComposition(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String path = json.getProperty("path", "").toString();

    if (path.isEmpty())
    {
        res.set_content(jsonError("Missing 'path'"), "application/json");
        return;
    }

    // {"ok":false,"reason":"..."} for the two failure modes below — distinct
    // from jsonError()'s {"ok":false,"error":"..."} shape used for malformed
    // requests (missing 'path' above), matching the endpoint's contract.
    auto jsonFail = [](const juce::String& reason) {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("ok", false);
        obj->setProperty("reason", reason);
        return juce::JSON::toString(juce::var(obj)).toStdString();
    };

    juce::File f(path);
    if (!f.existsAsFile())
    {
        res.set_content(jsonFail("File not found"), "application/json");
        return;
    }

    // Pre-check on a throwaway staged copy so this endpoint can answer
    // ok/false synchronously — loadComposition itself repeats the same
    // STAGE/VALIDATE steps on the message thread before the live swap; this
    // duplicates only the read-only prefix, never the live composition_.
    Composition staged;
    if (!staged.loadFromFile(f))
    {
        res.set_content(jsonFail("could not read/parse file"), "application/json");
        return;
    }
    if (auto reason = compload::validateComposition(staged); !reason.empty())
    {
        res.set_content(jsonFail(reason), "application/json");
        return;
    }

    if (!onLoadComposition)
    {
        res.set_content(jsonOk(), "application/json");
        return;
    }

    // s-rta-0929 asyncload (plan R1): answer only once the staged swap is done (or the load failed / was superseded /
    // the app quit) -- the ticket the message thread finishes. Bounded: kLoadWaitMs.
    auto ticket = std::make_shared<LoadTicket>();
    {
        std::lock_guard<std::mutex> lk(ticketsMutex_);
        if (ticketsClosed_)
        {
            res.set_content(jsonFail("cancelled"), "application/json");
            return;
        }
        tickets_.erase(std::remove_if(tickets_.begin(), tickets_.end(),
                                      [](const std::weak_ptr<LoadTicket>& w) { return w.expired(); }),
                       tickets_.end());
        tickets_.push_back(ticket);
    }
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    const bool posted = juce::MessageManager::callAsync([this, f, ticket]() {
        onLoadComposition(f, ticket);
    });
    if (!posted)   // the quit began: the message thread will never run it
    {
        ticket->finish(LoadTicket::Outcome::Cancelled);
        res.set_content(jsonFail("cancelled"), "application/json");
        return;
    }
    switch (ticket->wait(kLoadWaitMs))
    {
        case LoadTicket::Outcome::Done:       res.set_content(jsonOk(), "application/json"); break;
        case LoadTicket::Outcome::Failed:     res.set_content(jsonFail("could not load"), "application/json"); break;
        case LoadTicket::Outcome::Superseded: res.set_content(jsonFail("superseded by a newer load"), "application/json"); break;
        case LoadTicket::Outcome::Cancelled:  res.set_content(jsonFail("cancelled"), "application/json"); break;
        case LoadTicket::Outcome::Pending:    res.set_content(jsonFail("timed out"), "application/json"); break;
    }
}

void ApiServer::handleSetEffect(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String name = json.getProperty("name", "").toString();
    bool enabled = static_cast<bool>(json.getProperty("enabled", true));

    if (name.isEmpty())
    {
        res.set_content(jsonError("Missing 'name'"), "application/json");
        return;
    }

    // Extract requested param updates from the request JSON (safe: no model
    // access) before marshalling the effect-chain find+write below.
    std::vector<std::pair<std::string, float>> paramUpdates;
    if (json.hasProperty("params"))
    {
        auto* paramsObj = json["params"].getDynamicObject();
        if (paramsObj)
        {
            for (const auto& prop : paramsObj->getProperties())
                paramUpdates.emplace_back(prop.name.toString().toStdString(),
                                           static_cast<float>(static_cast<double>(prop.value)));
        }
    }

    // Effect chain lives on the composition-adjacent model, otherwise
    // mutated only on the message thread — marshal the find-by-name +
    // enabled/param writes there too (same callAsync pattern as set_param).
    // "Effect not found" can no longer be reported back synchronously now
    // that the lookup runs on the message thread — matches the trade-off
    // already made for set_param/set_layer_opacity.
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, name, enabled, paramUpdates]() {
        Effect* found = nullptr;
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            auto* fx = effectChain_.getEffect(i);
            if (fx && fx->getName() == name)
            {
                found = fx;
                break;
            }
        }
        if (!found)
            return;

        found->setEnabled(enabled);

        for (const auto& [paramName, paramValue] : paramUpdates)
        {
            for (int pi = 0; pi < found->getNumParams(); ++pi)
            {
                if (found->getParam(pi).name == paramName)
                {
                    found->getParam(pi).value = paramValue;
                    break;
                }
            }
        }
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleListEffects(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);

    juce::Array<juce::var> effectArray;
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* fx = effectChain_.getEffect(i);
        if (!fx) continue;

        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", fx->getName());
        fxObj->setProperty("enabled", fx->isEnabled());

        juce::Array<juce::var> params;
        for (int pi = 0; pi < fx->getNumParams(); ++pi)
        {
            auto* pObj = new juce::DynamicObject();
            pObj->setProperty("name", juce::String(fx->getParam(pi).name));
            pObj->setProperty("value", static_cast<double>(fx->getParam(pi).value));
            params.add(juce::var(pObj));
        }
        fxObj->setProperty("params", params);
        effectArray.add(juce::var(fxObj));
    }
    obj->setProperty("effects", effectArray);

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleListSources(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);

    juce::Array<juce::var> sourceArray;
    for (const auto& id : sourceRegistry_.getRegisteredIds())
    {
        auto* srcObj = new juce::DynamicObject();
        srcObj->setProperty("id", juce::String(id));
        srcObj->setProperty("name", juce::String(sourceRegistry_.getDisplayName(id)));
        srcObj->setProperty("category", juce::String(sourceRegistry_.getCategory(id)));
        sourceArray.add(juce::var(srcObj));
    }
    obj->setProperty("sources", sourceArray);

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleRenderFrame(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String outputPath = json.getProperty("output_path", "").toString();
    float time = static_cast<float>(static_cast<double>(json.getProperty("time", -1.0)));

    if (outputPath.isEmpty())
    {
        res.set_content(jsonError("Missing 'output_path'"), "application/json");
        return;
    }

    // s-rta-0928 R1 (C2): render_frame waits for a frame with no image still decoding; a snapshot does not.
    bool ok = renderer_.captureFrame(juce::File(outputPath), time, 0, 0, true,
                                     Renderer::CaptureEncoding::Fast);   // R3: the fast PNG writer
    if (ok)
        res.set_content(jsonOk(), "application/json");
    else
        res.set_content(jsonError("Frame capture failed"), "application/json");
}

void ApiServer::handleReset(const httplib::Request&, httplib::Response& res)
{
    // renderer_ calls left as-is: out of scope for this marshal pass
    // (renderer-internal thread-safety needs a separate design look).
    renderer_.clearImage();
    renderer_.clearActiveSource();

    // effectChain_ is otherwise mutated only on the message thread — marshal
    // this disable loop there too (same callAsync pattern as the other
    // effect-chain writes above).
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this]() {
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            if (auto* fx = effectChain_.getEffect(i))
                fx->setEnabled(false);
        }
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleSetEffectChain(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("effects"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'effects' array"), "application/json");
        return;
    }

    auto* effectsArray = obj->getProperty("effects").getArray();
    if (!effectsArray)
    {
        res.status = 400;
        res.set_content(jsonError("'effects' must be an array"), "application/json");
        return;
    }

    // Extract the requested effect list from the request JSON (safe: no
    // model access) before marshalling the effect-chain writes below.
    struct RequestedEffect
    {
        juce::String name;
        std::vector<std::pair<std::string, float>> params;
    };
    std::vector<RequestedEffect> requested;
    for (const auto& fxVar : *effectsArray)
    {
        auto* fxObj = fxVar.getDynamicObject();
        if (!fxObj) continue;

        RequestedEffect entry;
        entry.name = fxObj->getProperty("name").toString();
        if (fxObj->hasProperty("params"))
        {
            if (auto* paramsObj = fxObj->getProperty("params").getDynamicObject())
            {
                for (auto& prop : paramsObj->getProperties())
                    entry.params.emplace_back(prop.name.toString().toStdString(),
                                               static_cast<float>(static_cast<double>(prop.value)));
            }
        }
        requested.push_back(std::move(entry));
    }

    // effectChain_ is otherwise mutated only on the message thread — marshal
    // disable-all/enable-requested there too (same callAsync pattern as the
    // other effect-chain writes above).
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, requested]() {
        // Disable all, then enable requested
        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
            if (auto* fx = effectChain_.getEffect(i))
                fx->setEnabled(false);

        for (const auto& entry : requested)
        {
            for (int i = 0; i < effectChain_.getNumEffects(); ++i)
            {
                auto* fx = effectChain_.getEffect(i);
                if (fx && fx->getName() == entry.name)
                {
                    fx->setEnabled(true);
                    for (const auto& [paramName, paramValue] : entry.params)
                    {
                        for (int p = 0; p < fx->getNumParams(); ++p)
                        {
                            if (fx->getParam(p).name == paramName)
                            {
                                fx->getParam(p).value = paramValue;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    });

    res.set_content(jsonOk(), "application/json");
}
void ApiServer::handleState(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("frame_time_ms", static_cast<double>(renderer_.getFrameTimeMs()));
    // s-rta-0926b R1: compositor history memory + the longest frame since the
    // previous /api/state read (reading resets it). Same fields as TestServer.
    obj->setProperty("temporal_buffers", renderer_.getCompositor().getTemporalBufferCount());
    obj->setProperty("frame_rings", renderer_.getCompositor().getFrameRingCount());
    // s-rta-0928 R5 (C4): ring cells created so far, over every ring (cells are created on first write).
    obj->setProperty("frame_ring_cells", renderer_.getCompositor().getFrameRingCellCount());
    obj->setProperty("peak_frame_time_ms", static_cast<double>(renderer_.takePeakFrameTimeMs()));
    // s-rta-0928 R1.0: the longest WHOLE render callback since the previous read (resets on read).
    obj->setProperty("peak_callback_ms", static_cast<double>(renderer_.takePeakCallbackMs()));
    // s-rta-0928 R1.2: clip images decoded off the GL thread -- frames a layer held its last picture / drew nothing
    // while its image decoded (cumulative), images still decoding, resident image textures (count, MB), the longest
    // single upload since the previous read (resets on read), and frames whose image pump ran (B2: every frame).
    {
        auto& comp = renderer_.getCompositor();
        obj->setProperty("image_hold_frames", static_cast<juce::int64>(comp.getImageHoldFrames()));
        obj->setProperty("image_skip_frames", static_cast<juce::int64>(comp.getImageSkipFrames()));
        obj->setProperty("images_pending", comp.getImagesPending());
        obj->setProperty("image_textures", comp.getImageTextureCount());
        obj->setProperty("image_texture_mb", comp.getImageTextureMB());
        obj->setProperty("peak_image_upload_ms", static_cast<double>(comp.takePeakImageUploadMs()));
        obj->setProperty("image_pump_frames", static_cast<juce::int64>(comp.getImagePumpFrames()));
    }
    // s-rta-0928b seqvram: image sequences' texture memory (all sequences, summed at the frame top) and frame counters
    // (cumulative). seq_textures = allocated textures, seq_late_frames = frames the current frame was not resident and the
    // shown one repeated, seq_pending_frames = frames with nothing to show yet. Same fields in ApiServer and TestServer.
    {
        const auto& sq = renderer_.getSeqStats();
        obj->setProperty("seq_open", sq.openCount.load(std::memory_order_relaxed));
        obj->setProperty("seq_textures", sq.residentSlots.load(std::memory_order_relaxed));
        obj->setProperty("seq_texture_mb", static_cast<double>(sq.residentBytes.load(std::memory_order_relaxed)) / (1024.0 * 1024.0));
        obj->setProperty("seq_over_budget", sq.overBudget.load(std::memory_order_relaxed));
        obj->setProperty("seq_frames_shown", static_cast<juce::int64>(sq.framesShown.load(std::memory_order_relaxed)));
        obj->setProperty("seq_late_frames", static_cast<juce::int64>(sq.lateFrames.load(std::memory_order_relaxed)));
        obj->setProperty("seq_pending_frames", static_cast<juce::int64>(sq.pendingFrames.load(std::memory_order_relaxed)));
        obj->setProperty("seq_uploads", static_cast<juce::int64>(sq.uploads.load(std::memory_order_relaxed)));
        obj->setProperty("seq_slot_reuses", static_cast<juce::int64>(sq.slotReuses.load(std::memory_order_relaxed)));
        obj->setProperty("seq_evictions", static_cast<juce::int64>(sq.evictions.load(std::memory_order_relaxed)));
        obj->setProperty("seq_stale_drops", static_cast<juce::int64>(sq.staleDrops.load(std::memory_order_relaxed)));
        obj->setProperty("seq_upload_deferred", static_cast<juce::int64>(sq.uploadDeferred.load(std::memory_order_relaxed)));
        // fix round F2: glDeleteTextures of the per-frame paths (idle trim, shrink, retire drain), <= 8 per frame
        obj->setProperty("seq_deletes", static_cast<juce::int64>(sq.deletes.load(std::memory_order_relaxed)));
        // fix round F3: the textures of the sequences drawn within the last kIdleFrames frames (not idle)
        obj->setProperty("seq_drawn_textures", sq.drawnSlots.load(std::memory_order_relaxed));
    }
    // s-rta-0926b plan4 A-opt: GPU time of the frame's GL work (timer queries; 0 = driver reported
    // nothing). Same fields as TestServer.
    obj->setProperty("gpu_time_ms", static_cast<double>(renderer_.getGpuTimeMs()));
    obj->setProperty("peak_gpu_time_ms", static_cast<double>(renderer_.takePeakGpuTimeMs()));
    // s-rta-0925: master_level is now the one master (composition_.eff()).
    obj->setProperty("master_level", static_cast<double>(composition_.eff(CompScalar::Opacity)));
    // s-rta-0928b video: video decodes off the GL thread (VideoPlayer decode thread + VideoRing; probe-video). Same
    // fields as TestServer. gl_video_decode_calls counts avcodec calls made ON the render thread (0 by construction once
    // the decode thread lands); *_max_* / peak_* reset on read.
    {
        auto& v = renderer_.getVideoStats();
        obj->setProperty("video_players", v.players.load(std::memory_order_relaxed));
        obj->setProperty("video_threads", v.threadsRunning.load(std::memory_order_relaxed));
        obj->setProperty("video_threads_awake", v.threadsAwake.load(std::memory_order_relaxed));   // not parked (V2)
        obj->setProperty("gl_video_decode_calls", static_cast<juce::int64>(v.glDecodeCalls.load(std::memory_order_relaxed)));
        obj->setProperty("gl_video_max_decodes_per_call", v.takeGlMaxDecodesPerCall());
        obj->setProperty("video_uploads", static_cast<juce::int64>(v.uploads.load(std::memory_order_relaxed)));
        obj->setProperty("video_frames_decoded", static_cast<juce::int64>(v.framesDecoded.load(std::memory_order_relaxed)));
        obj->setProperty("video_frames_dropped", static_cast<juce::int64>(v.framesDropped.load(std::memory_order_relaxed)));   // catch-up: decoded, not converted
        obj->setProperty("video_frames_skipped", static_cast<juce::int64>(v.framesSkipped.load(std::memory_order_relaxed)));   // GL: an older ready frame passed over
        obj->setProperty("video_seeks", static_cast<juce::int64>(v.seeks.load(std::memory_order_relaxed)));
        obj->setProperty("video_hold_frames", static_cast<juce::int64>(v.holdFrames.load(std::memory_order_relaxed)));   // drawn frames that re-showed the last frame
        obj->setProperty("video_late_frames", static_cast<juce::int64>(v.lateFrames.load(std::memory_order_relaxed)));   // ... while the clock had passed the next frame
        obj->setProperty("video_pending_frames", static_cast<juce::int64>(v.pendingFrames.load(std::memory_order_relaxed)));   // nothing shown yet
        obj->setProperty("videos_pending", v.pendingNow.load(std::memory_order_relaxed));   // this frame
        obj->setProperty("peak_video_upload_ms", static_cast<double>(v.takePeakUploadMs()));
        obj->setProperty("msg_video_lock_wait_max_ms", static_cast<double>(v.takeMsgLockWaitMaxMs()));
        // s-rta-0929 vupload: the per-frame upload budget, the FX-only witness (must stay 0), the idle ring trim, the
        // IOSurface blit's fence failures / client-upload fallbacks; the cap is INFO; max uploads per frame resets on read.
        obj->setProperty("video_uploads_deferred", static_cast<juce::int64>(v.uploadsDeferred.load(std::memory_order_relaxed)));
        obj->setProperty("video_hold_no_texture", static_cast<juce::int64>(v.holdNoTexture.load(std::memory_order_relaxed)));
        obj->setProperty("video_slots_purged", static_cast<juce::int64>(v.slotsPurged.load(std::memory_order_relaxed)));
        obj->setProperty("video_fence_failed", static_cast<juce::int64>(v.fenceFailed.load(std::memory_order_relaxed)));
        obj->setProperty("video_surface_fallbacks", static_cast<juce::int64>(v.surfaceFallbacks.load(std::memory_order_relaxed)));
        obj->setProperty("video_upload_cap", v.uploadCap.load(std::memory_order_relaxed));
        obj->setProperty("video_max_uploads_per_frame", v.takeMaxUploadsPerFrame());
        // s-rta-0929b gopcache: the decode threads' GOP caches (gauges: bytes / frames / active; cap INFO; the rest
        // cumulative), the reverse-order witness (must stay 0), direction changes, the longest upload gap of a drawn
        // player and the longest writer step (reset on read), uploads per player slot.
        obj->setProperty("video_gopcache_bytes", static_cast<juce::int64>(v.gopCacheBytes.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_frames", static_cast<juce::int64>(v.gopCacheFrames.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_active", v.gopCacheActive.load(std::memory_order_relaxed));
        obj->setProperty("video_gopcache_cap_bytes", static_cast<juce::int64>(v.gopCacheCapBytes.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_hits", static_cast<juce::int64>(v.gopCacheHits.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_misses", static_cast<juce::int64>(v.gopCacheMisses.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_runs", static_cast<juce::int64>(v.gopCacheRuns.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_run_decodes", static_cast<juce::int64>(v.gopCacheRunDecodes.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_evictions", static_cast<juce::int64>(v.gopCacheEvictions.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_drops", static_cast<juce::int64>(v.gopCacheDrops.load(std::memory_order_relaxed)));
        obj->setProperty("video_gopcache_over_budget", static_cast<juce::int64>(v.gopCacheOverBudget.load(std::memory_order_relaxed)));
        obj->setProperty("video_reverse_nonmonotonic", static_cast<juce::int64>(v.reverseNonmonotonic.load(std::memory_order_relaxed)));
        obj->setProperty("video_direction_changes", static_cast<juce::int64>(v.directionChanges.load(std::memory_order_relaxed)));
        obj->setProperty("video_max_upload_gap_ms", static_cast<double>(v.takeMaxUploadGapMs()));
        obj->setProperty("video_writer_step_max_ms", static_cast<double>(v.takeWriterStepMaxMs()));
        {
            juce::Array<juce::var> pu;
            for (const auto& u : v.playerUploads)
                pu.add(static_cast<juce::int64>(u.load(std::memory_order_relaxed)));
            obj->setProperty("video_player_uploads", pu);
        }
    }
    // s-rta-0928b mediaopen: frames the renderer found inside a withDeckDetached fence with no deck (cumulative):
    // hold = it re-presented the canvas as the previous frame left it; black = it fell to the "nothing to render" path.
    // Same fields as TestServer.
    obj->setProperty("fence_hold_frames", static_cast<juce::int64>(renderer_.getFenceHoldFrames()));
    obj->setProperty("fence_black_frames", static_cast<juce::int64>(renderer_.getFenceBlackFrames()));
    // Lane tsan (s-rta-1002; ruling amendment 13): the GL thread's writes of the Layer trigger tuple (cumulative):
    // queued triggers it fired on a beat, autopilot advances it applied, fade ticks that adopted a concurrent
    // trigger's tuple (reported, never a bar). Same fields in ApiServer and TestServer.
    obj->setProperty("render_pending_fired", static_cast<juce::int64>(renderer_.getRenderPendingFired()));
    obj->setProperty("render_autopilot_advances", static_cast<juce::int64>(renderer_.getRenderAutopilotAdvances()));
    obj->setProperty("render_tuple_adopts", static_cast<juce::int64>(renderer_.getRenderTupleAdopts()));
    // s-rta-0928b mediaopen: {presence_sweeps, presence_changed} (MediaPresence). Same field as TestServer.
    if (mediaStateProvider_)
        obj->setProperty("media", mediaStateProvider_());
    // s-rta-0929 asyncload: the asynchronous-load witnesses (MainComponent::loadWitnessVar). Same field as TestServer.
    if (loadWitnessProvider_)
        obj->setProperty("load", loadWitnessProvider_());
#if AUDIODNA_TEST_SERVER
    // s-rta-0928b mediaopen (TEST-ONLY): the message-thread heartbeat (POST /api/debug/heartbeat) -- the longest wait of
    // a ping since the previous read (resets on read; 0 while off).
    obj->setProperty("message_heartbeat_on", heartbeat_.isOn());
    obj->setProperty("peak_message_stall_ms", heartbeat_.takePeakMs());
#endif
    // s-rta-0927 outputs-c1: the output frame path (additive). Same fields as TestServer.
    {
        auto& frames = renderer_.getSharedFrames();
        const output::FrontFrame front = frames.front();
        auto* outputs = new juce::DynamicObject();
        outputs->setProperty("live", renderer_.getLiveOutputCount());
        outputs->setProperty("tap", renderer_.isOutputTapForced());
        outputs->setProperty("frame_gen", static_cast<juce::int64>(front.gen));
        outputs->setProperty("frame_serial", static_cast<juce::int64>(front.serial));
        outputs->setProperty("canvas_w", frames.frontWidth());
        outputs->setProperty("canvas_h", frames.frontHeight());
        // s-rta-0927 outputs-c2: [{index, x, y, w, h, scale, main, live, label}] -- the Output menu's display list.
        if (outputsStateProvider_)
            outputs->setProperty("displays", outputsStateProvider_());
        obj->setProperty("outputs", juce::var(outputs));
    }

    // Effects state
    juce::Array<juce::var> effectsArr;
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* fx = effectChain_.getEffect(i);
        if (!fx) continue;
        auto* fxObj = new juce::DynamicObject();
        fxObj->setProperty("name", fx->getName());
        fxObj->setProperty("category", fx->getCategory());
        fxObj->setProperty("enabled", fx->isEnabled());
        fxObj->setProperty("dry_wet", static_cast<double>(fx->getDryWet()));
        juce::Array<juce::var> paramsArr;
        for (int p = 0; p < fx->getNumParams(); ++p)
        {
            auto& param = fx->getParam(p);
            auto* paramObj = new juce::DynamicObject();
            paramObj->setProperty("name", juce::String(param.name));
            paramObj->setProperty("value", static_cast<double>(param.value));
            paramObj->setProperty("default", static_cast<double>(param.defaultValue));
            paramsArr.add(juce::var(paramObj));
        }
        fxObj->setProperty("params", paramsArr);
        effectsArr.add(juce::var(fxObj));
    }
    obj->setProperty("effects", effectsArr);
    obj->setProperty("active_deck", composition_.activeDeckIndex.load());
    obj->setProperty("num_decks", static_cast<int>(composition_.decks.size()));

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleGetSyphon(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("enabled", renderer_.isSyphonEnabled());
    obj->setProperty("available", renderer_.isSyphonAvailable());
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleSetSyphon(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("enabled"))
    {
        res.set_content(jsonError("Missing 'enabled'"), "application/json");
        return;
    }
    bool enabled = static_cast<bool>(json["enabled"]);

    // Just flips SyphonOutput's atomic enabled_ flag — no GL calls, safe to
    // call directly from the HTTP thread. Mirrors the Output > Syphon Output
    // menu toggle (MainComponent.cpp), which flips the same atomic from the
    // message thread with no marshaling either.
    renderer_.setSyphonEnabled(enabled);

    res.set_content(jsonOk(), "application/json");
}

// --- s-rta-0923 step 3 (Lane S3-C): /api/perf/* ---
//
// Same callAsync marshal shape as every other model write in this file (see
// handleSetParam's clip-effect branch note for the `this`-capture safety
// argument, which applies identically here) with one addition: every
// unassigned callback below answers 503 before touching the request body, so
// a build where MainComponent has not yet wired RecorderHost answers cleanly
// instead of silently no-op'ing 200 like the pre-existing endpoints do.
// onPerfStatus is the one exception to the marshal pattern -- it is called
// synchronously, off whatever thread the request landed on, because
// RecorderHost::status() is a mutex-guarded copy safe from any thread (same
// posture as handleGetBpm/handleGetFeatures reading FeatureBus::read()).

void ApiServer::handlePerfRecord(const httplib::Request& req, httplib::Response& res)
{
    if (!onPerfRecord)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    PerfRecordOpts opts;
    opts.name = json.getProperty("name", "").toString();
    opts.audio = static_cast<bool>(json.getProperty("audio", true));
    opts.audioFile = json.getProperty("audioFile", "").toString();
    opts.onsetMarkers = static_cast<bool>(json.getProperty("onsetMarkers", false));
    opts.overdubAssetId = json.getProperty("overdubAssetId", "").toString();

    // Arm-time validation (device rate, store/tap failures) runs on the
    // message thread inside RecorderHost::arm and can no longer be reported
    // back synchronously -- same trade-off already made for
    // set_param/set_layer_opacity/set_effect. Refusals surface through
    // /api/perf/status (lastError/rateChangedSinceArm), not this response.
    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, opts]() {
        onPerfRecord(opts);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfStop(const httplib::Request&, httplib::Response& res)
{
    if (!onPerfStop)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this]() {
        onPerfStop();
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfLoad(const httplib::Request& req, httplib::Response& res)
{
    if (!onPerfLoad)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String folder = json.getProperty("folder", "").toString();

    if (folder.isEmpty())
    {
        res.set_content(jsonError("Missing 'folder'"), "application/json");
        return;
    }

    juce::File takeFolder(folder);

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, takeFolder]() {
        onPerfLoad(takeFolder);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfPlay(const httplib::Request& req, httplib::Response& res)
{
    if (!onPerfPlay)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    bool withAudio = static_cast<bool>(json.getProperty("withAudio", false));

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, withAudio]() {
        onPerfPlay(withAudio);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfStopPlay(const httplib::Request&, httplib::Response& res)
{
    if (!onPerfStopPlay)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this]() {
        onPerfStopPlay();
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfRepair(const httplib::Request&, httplib::Response& res)
{
    if (!onPerfRepair)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this]() {
        onPerfRepair();
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handlePerfStatus(const httplib::Request&, httplib::Response& res)
{
    if (!onPerfStatus)
    {
        res.status = 503;
        res.set_content(jsonError("Recorder unavailable"), "application/json");
        return;
    }

    // Synchronous -- critic A5(b)/N3: this handler reads nothing but
    // RecorderHost::status() (mutex-guarded copy, safe from any thread).
    // MainComponent's onPerfStatus assignment is the one place deviceRate,
    // rateChangedSinceArm and humanRefused get read -- from RecorderHost::
    // Status, published by tick() -- never from
    // audioEngine_.getCurrentSampleRate()/getCurrentAudioDevice() on this
    // thread.
    res.set_content(juce::JSON::toString(onPerfStatus()).toStdString(), "application/json");
}

// s-rta-0925 (probe enabler, end-of-replay plan section 5): same posture as handlePerfPlay -- 503 when
// unassigned, parse, marshal to the message thread. Switching to "file" with no file loaded leaves
// the transport silent (a dev/probe control, not a production affordance).
void ApiServer::handleAudioSource(const httplib::Request& req, httplib::Response& res)
{
    if (!onAudioSource)
    {
        res.status = 503;
        res.set_content(jsonError("Audio source unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    juce::String mode = json.getProperty("mode", "").toString();
    if (mode != "input" && mode != "file")
    {
        res.status = 400;
        res.set_content(jsonError("mode must be \"input\" or \"file\""), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, mode]() {
        onAudioSource(mode);
    });

    res.set_content(jsonOk(), "application/json");
}

#if AUDIODNA_TEST_SERVER
// s-rta-0927 beat clock (TEST-ONLY): sleeps inside ONE message, so the 120 Hz MappingTickTimer fires exactly
// once on wake (juce_Timer.cpp callTimers resets the countdown on that fire) -- the recorded loadpost1
// signature, on a quiet machine.
void ApiServer::handleDebugStallMessageThread(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("ms"))
    {
        res.status = 400;
        res.set_content(jsonError("ms (1..2000) required"), "application/json");
        return;
    }
    const int ms = std::clamp(static_cast<int>(json["ms"]), 1, 2000);
    juce::MessageManager::callAsync([ms]() { juce::Thread::sleep(ms); });
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("ms", ms);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0928b mediaopen (TEST-ONLY): {"on": bool, "period_ms": 1..50 (default 4)}. The heartbeat starts / stops on the
// message thread (MessageHeartbeat's one-thread rule); /api/state reads its peak from an atomic.
void ApiServer::handleDebugHeartbeat(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("on"))
    {
        res.status = 400;
        res.set_content(jsonError("on (bool) required"), "application/json");
        return;
    }
    const bool on = static_cast<bool>(json["on"]);
    const int periodMs = std::clamp(json.hasProperty("period_ms") ? static_cast<int>(json["period_ms"]) : 4, 1, 50);
    juce::MessageManager::callAsync([this, on, periodMs]() {
        if (on) heartbeat_.start(periodMs);
        else    heartbeat_.stop();
    });
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("on", on);
    obj->setProperty("period_ms", periodMs);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0928b mediaopen (TEST-ONLY): {"layer": L, "column": C, "files": ["/abs/path", ...]} -> the handlers a Finder drop
// onto that cell of the active deck reaches (MainComponent::debugDropFiles). Paths are not checked here: a Finder drop
// does not check them either.
void ApiServer::handleDebugDropFiles(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    const auto* arr = json["files"].getArray();
    if (!json.hasProperty("layer") || !json.hasProperty("column") || arr == nullptr || arr->isEmpty())
    {
        res.status = 400;
        res.set_content(jsonError("layer, column and files (non-empty array of absolute paths) required"), "application/json");
        return;
    }
    if (!onDebugDropFiles)
    {
        res.status = 503;
        res.set_content(jsonError("drop_files not wired"), "application/json");
        return;
    }
    const int layer = static_cast<int>(json["layer"]);
    const int column = static_cast<int>(json["column"]);
    std::vector<juce::File> files;
    for (const auto& f : *arr)
        if (juce::File::isAbsolutePath(f.toString()))
            files.emplace_back(f.toString());
    juce::MessageManager::callAsync([this, layer, column, files]() { onDebugDropFiles(layer, column, files); });
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("files", static_cast<int>(files.size()));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0928b idlepaint (TEST-ONLY): the UI paint counters, cumulative, read from relaxed atomics on the HTTP thread.
void ApiServer::handleDebugUiPaint(const httplib::Request&, httplib::Response& res)
{
    auto& c = uipaint::counters();
    const auto rl = std::memory_order_relaxed;
    auto rect = [rl](const std::atomic<int> (&r)[4]) {
        juce::Array<juce::var> a;
        for (const auto& v : r) a.add(v.load(rl));
        return juce::var(a);
    };
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("waveform_layer_draws", static_cast<juce::int64>(c.layerDraws[uipaint::Waveform].load(rl)));
    obj->setProperty("signalbar_layer_draws", static_cast<juce::int64>(c.layerDraws[uipaint::SignalBar].load(rl)));
    obj->setProperty("waveform_mode", c.layerMode[uipaint::Waveform].load(rl));
    obj->setProperty("signalbar_mode", c.layerMode[uipaint::SignalBar].load(rl));
    obj->setProperty("layer_fallbacks", static_cast<juce::int64>(c.layerFallbacks.load(rl)));
    obj->setProperty("layer_strip_transport_repaints", static_cast<juce::int64>(c.layerStripTransportRepaints.load(rl)));
    obj->setProperty("layer_strip_playhead_ticks", static_cast<juce::int64>(c.layerStripPlayheadTicks.load(rl)));
    obj->setProperty("layer_strip_playhead_paints", static_cast<juce::int64>(c.layerStripPlayheadPaints.load(rl)));
    obj->setProperty("layer_strip_band_repaints", static_cast<juce::int64>(c.layerStripBandRepaints.load(rl)));
    obj->setProperty("main_component_paints", static_cast<juce::int64>(c.mainComponentPaints.load(rl)));
    obj->setProperty("top_bar_paints", static_cast<juce::int64>(c.topBarPaints.load(rl)));
    obj->setProperty("clip_inspector_repaints", static_cast<juce::int64>(c.clipInspectorRepaints.load(rl)));
    obj->setProperty("ui_overlay_covered_frames", static_cast<juce::int64>(c.overlayCoveredFrames.load(rl)));
    obj->setProperty("ui_restore_frames_last", c.restoreFramesLast.load(rl));
    obj->setProperty("ui_restore_frames_max", c.restoreFramesMax.load(rl));
    obj->setProperty("peer_layer_backed", c.peerLayerBacked.load(rl));
    obj->setProperty("main_w", c.mainW.load(rl));
    obj->setProperty("main_h", c.mainH.load(rl));
    obj->setProperty("signalbar_rect", rect(c.signalBarRect));
    obj->setProperty("waveform_rect", rect(c.waveformRect));
    obj->setProperty("topbar_rect", rect(c.topBarRect));
    // s-rta-0929 g4cpu (plan-g4cpu 2.2, G7): who repaints / what paints while a routine plays.
    obj->setProperty("routine_pad_repaints", static_cast<juce::int64>(c.routinePadRepaints.load(rl)));
    obj->setProperty("routine_pad_sweep_ticks", static_cast<juce::int64>(c.routinePadSweepTicks.load(rl)));
    obj->setProperty("routine_pad_sweep_paints", static_cast<juce::int64>(c.routinePadSweepPaints.load(rl)));
    obj->setProperty("routine_pad_paints", static_cast<juce::int64>(c.routinePadPaints.load(rl)));
    obj->setProperty("layer_strip_fader_repaints", static_cast<juce::int64>(c.layerStripFaderRepaints.load(rl)));
    obj->setProperty("layer_strip_fader_paints", static_cast<juce::int64>(c.layerStripFaderPaints.load(rl)));
    obj->setProperty("layer_strip_band_paints", static_cast<juce::int64>(c.layerStripBandPaints.load(rl)));
    obj->setProperty("top_bar_wheel_repaints", static_cast<juce::int64>(c.topBarWheelRepaints.load(rl)));
    obj->setProperty("deck_corner_repaints", static_cast<juce::int64>(c.deckCornerRepaints.load(rl)));
    obj->setProperty("layer_inspector_repaints", static_cast<juce::int64>(c.layerInspectorRepaints.load(rl)));
    obj->setProperty("param_control_repaints", static_cast<juce::int64>(c.paramControlRepaints.load(rl)));
    obj->setProperty("signal_strip_changes", static_cast<juce::int64>(c.signalStripChanges.load(rl)));
    obj->setProperty("signal_bar_ticks", static_cast<juce::int64>(c.signalBarTicks.load(rl)));
    obj->setProperty("waveform_layer_draw_us", static_cast<juce::int64>(c.layerDrawUs[uipaint::Waveform].load(rl)));
    obj->setProperty("signalbar_layer_draw_us", static_cast<juce::int64>(c.layerDrawUs[uipaint::SignalBar].load(rl)));
    obj->setProperty("waveform_layer_draw_wall_us", static_cast<juce::int64>(c.layerDrawWallUs[uipaint::Waveform].load(rl)));
    obj->setProperty("signalbar_layer_draw_wall_us", static_cast<juce::int64>(c.layerDrawWallUs[uipaint::SignalBar].load(rl)));
    obj->setProperty("deck_rect", rect(c.deckRect));
    obj->setProperty("pad_row_rect", rect(c.padRowRect));
    obj->setProperty("strip_col_rect", rect(c.stripColRect));
    obj->setProperty("wheel_rect", rect(c.wheelRect));
    obj->setProperty("inspector_rect", rect(c.inspectorRect));
    obj->setProperty("preview_rect", rect(c.previewRect));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0929 g4cpu (TEST-ONLY): the newest display passes (up to 500 of the 512-entry ring: the writer is at most a few
// entries ahead while we read) as {t, x, y, w, h, us, src}, oldest first, and the newest SignalBar change masks.
void ApiServer::handleDebugUiPasses(const httplib::Request&, httplib::Response& res)
{
    auto& c = uipaint::counters();
    const auto rl = std::memory_order_relaxed;
    constexpr uint32_t kKeep = 500;
    const uint32_t seq = c.passSeq.load(std::memory_order_acquire);
    juce::Array<juce::var> passes;
    for (uint32_t k = seq > kKeep ? seq - kKeep : 0; k < seq; ++k)
    {
        const auto& p = c.passes[k % uipaint::Counters::kRing];
        juce::Array<juce::var> e;
        e.add(static_cast<juce::int64>(p.tUs.load(rl)));
        e.add(p.x.load(rl)); e.add(p.y.load(rl)); e.add(p.w.load(rl)); e.add(p.h.load(rl)); e.add(p.us.load(rl));
        e.add(static_cast<int>(p.src.load(rl)));
        e.add(p.wus.load(rl));
        passes.add(juce::var(e));
    }
    const uint32_t mseq = c.stripMaskSeq.load(std::memory_order_acquire);
    juce::Array<juce::var> masks;
    for (uint32_t k = mseq > kKeep ? mseq - kKeep : 0; k < mseq; ++k)
        masks.add(static_cast<juce::int64>(c.stripMasks[k % uipaint::Counters::kRing].load(rl)));
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("seq", static_cast<juce::int64>(seq));
    obj->setProperty("passes", passes);   // [t_us, x, y, w, h, cpu us (-1: paint() skipped), src bits, wall us]
    obj->setProperty("strip_seq", static_cast<juce::int64>(mseq));
    obj->setProperty("strip_masks", masks);
    obj->setProperty("signalbar_strips", c.signalBarStrips.load(rl));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0928b idlepaint (TEST-ONLY): {"on": bool, "x": int, "y": int, "kind": "menu" | "panel"} (MainComponent
// coordinates) -> the parented test overlay (onDebugUiTestMenu) on the message thread.
void ApiServer::handleDebugUiTestMenu(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("on"))
    {
        res.status = 400;
        res.set_content(jsonError("on (bool) required"), "application/json");
        return;
    }
    if (!onDebugUiTestMenu)
    {
        res.status = 503;
        res.set_content(jsonError("ui_test_menu not wired"), "application/json");
        return;
    }
    const bool on = static_cast<bool>(json["on"]);
    const int x = static_cast<int>(json.getProperty("x", 0));
    const int y = static_cast<int>(json.getProperty("y", 0));
    const juce::String kind = json.getProperty("kind", "menu").toString();
    if (kind != "menu" && kind != "panel")
    {
        res.status = 400;
        res.set_content(jsonError("kind must be \"menu\" or \"panel\""), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this, on, x, y, kind]() { onDebugUiTestMenu(on, x, y, kind); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0928b idlepaint (TEST-ONLY): {"on": bool} -> the native-layer panels paint in-peer (on) or in their layers.
void ApiServer::handleDebugUiNativeFallback(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("on"))
    {
        res.status = 400;
        res.set_content(jsonError("on (bool) required"), "application/json");
        return;
    }
    if (!onDebugUiNativeFallback)
    {
        res.status = 503;
        res.set_content(jsonError("ui_native_fallback not wired"), "application/json");
        return;
    }
    const bool on = static_cast<bool>(json["on"]);
    juce::MessageManager::callAsync([this, on]() { onDebugUiNativeFallback(on); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0928b idlepaint (TEST-ONLY): repaint the whole MainComponent (the non-idle full-window pass, adoption I7).
void ApiServer::handleDebugUiRepaintAll(const httplib::Request&, httplib::Response& res)
{
    if (!onDebugUiRepaintAll)
    {
        res.status = 503;
        res.set_content(jsonError("ui_repaint_all not wired"), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this]() { onDebugUiRepaintAll(); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0929 asyncload (TEST-ONLY): the file label's text, read ON the message thread. The handler waits <= 2 s for the
// answer (a frozen message thread answers {"ok":false,...} instead); the shared box outlives a late answer.
void ApiServer::handleDebugUiText(const httplib::Request&, httplib::Response& res)
{
    if (!onDebugUiText)
    {
        res.status = 503;
        res.set_content(jsonError("ui_text not wired"), "application/json");
        return;
    }
    struct Box { juce::WaitableEvent done; juce::String text, notice, load, layer, clip, tab; };
    auto box = std::make_shared<Box>();
    const bool posted = juce::MessageManager::callAsync([this, box]() {
        box->text = onDebugUiText();
        if (onDebugAudioNotice)
            box->notice = onDebugAudioNotice();   // s-rta-0929b btguard
        if (onDebugLoadNotice)
            box->load = onDebugLoadNotice();      // lane bf9b S3.4
        if (onDebugInspectedLayer)
            box->layer = onDebugInspectedLayer(); // lane bf9b fix stage (AM-6)
        if (onDebugInspectedClip)
            box->clip = onDebugInspectedClip();
        if (onDebugInspectorTab)
            box->tab = onDebugInspectorTab();
        box->done.signal();
    });
    auto* obj = new juce::DynamicObject();
    if (!posted || !box->done.wait(2000))
    {
        obj->setProperty("ok", false);
        obj->setProperty("reason", "message thread did not answer within 2 s");
    }
    else
    {
        obj->setProperty("ok", true);
        obj->setProperty("file_label", box->text);
        obj->setProperty("audio_notice", box->notice);
        obj->setProperty("load_notice", box->load);
        obj->setProperty("inspected_layer", box->layer);
        obj->setProperty("inspected_clip", box->clip);
        obj->setProperty("inspector_tab", box->tab);
    }
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// s-rta-0929 asyncload (TEST-ONLY): {"path": "/abs/deck.json"} -> Load Deck... of that file (the library's Decks row).
void ApiServer::handleDebugLoadDeck(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    const juce::String path = json.getProperty("path", "").toString();
    if (path.isEmpty() || !juce::File::isAbsolutePath(path) || !juce::File(path).existsAsFile())
    {
        res.status = 400;
        res.set_content(jsonError("path (an existing absolute file) required"), "application/json");
        return;
    }
    if (!onDebugLoadDeck)
    {
        res.status = 503;
        res.set_content(jsonError("load_deck not wired"), "application/json");
        return;
    }
    const juce::File f(path);
    juce::MessageManager::callAsync([this, f]() { onDebugLoadDeck(f); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0929 asyncload (TEST-ONLY): {"deck": i} -> the deck tab menu's Duplicate of deck i.
void ApiServer::handleDebugDuplicateDeck(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("deck"))
    {
        res.status = 400;
        res.set_content(jsonError("deck (int) required"), "application/json");
        return;
    }
    if (!onDebugDuplicateDeck)
    {
        res.status = 503;
        res.set_content(jsonError("duplicate_deck not wired"), "application/json");
        return;
    }
    const int deck = static_cast<int>(json["deck"]);
    juce::MessageManager::callAsync([this, deck]() { onDebugDuplicateDeck(deck); });
    res.set_content(jsonOk(), "application/json");
}

// Lane bf9b (TEST-ONLY): {"deck": i} -> the deck tab menu's Remove Deck of deck i.
void ApiServer::handleDebugRemoveDeck(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("deck"))
    {
        res.status = 400;
        res.set_content(jsonError("deck (int) required"), "application/json");
        return;
    }
    if (!onDebugRemoveDeck)
    {
        res.status = 503;
        res.set_content(jsonError("remove_deck not wired"), "application/json");
        return;
    }
    const int deck = static_cast<int>(json["deck"]);
    juce::MessageManager::callAsync([this, deck]() { onDebugRemoveDeck(deck); });
    res.set_content(jsonOk(), "application/json");
}

// Lane bf9b fix round (TEST-ONLY): {"path": "<absolute .json>"} -> File > Save As... to that file, no chooser.
void ApiServer::handleDebugSaveComposition(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    const juce::String path = json.getProperty("path", "").toString();
    if (path.isEmpty() || !juce::File::isAbsolutePath(path) || !juce::File(path).getParentDirectory().isDirectory())
    {
        res.status = 400;
        res.set_content(jsonError("path (an absolute file in an existing folder) required"), "application/json");
        return;
    }
    if (!onDebugSaveComposition)
    {
        res.status = 503;
        res.set_content(jsonError("save_composition not wired"), "application/json");
        return;
    }
    const juce::File f(path);
    juce::MessageManager::callAsync([this, f]() { onDebugSaveComposition(f); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0929b btguard (TEST-ONLY): reads ONLY the mutex-guarded copy AudioEngine publishes on the message thread.
void ApiServer::handleDebugAudioDevices(const httplib::Request&, httplib::Response& res)
{
    if (!audioDevicesProvider_)
    {
        res.status = 503;
        res.set_content(jsonError("audio devices not wired"), "application/json");
        return;
    }
    res.set_content(juce::JSON::toString(audioDevicesProvider_(), true).toStdString(), "application/json");
}

// s-rta-0930 bt2 (TEST-ONLY): {"names": ["<exact JUCE device name>", ...]} ([] = none) -> AudioEngine::debugSetDeniedDevices
// on the message thread: the denied set is REPLACED and the guard's device-list-change path runs.
void ApiServer::handleDebugAudioDeny(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    const auto* arr = json["names"].getArray();
    bool allStrings = arr != nullptr;
    if (arr != nullptr)
        for (const auto& n : *arr)
            allStrings = allStrings && n.isString();
    if (!allStrings)
    {
        res.status = 400;
        res.set_content(jsonError("names (an array of device names) required"), "application/json");
        return;
    }
    if (!onDebugAudioDeny)
    {
        res.status = 503;
        res.set_content(jsonError("audio_deny not wired"), "application/json");
        return;
    }
    juce::StringArray names;
    for (const auto& n : *arr)
        names.add(n.toString());
    juce::MessageManager::callAsync([this, names]() { onDebugAudioDeny(names); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0930 bt2 (TEST-ONLY, bodyless -- answered at once, Pitfall 31): AudioEngine::debugStopDevice on the message thread.
void ApiServer::handleDebugAudioStop(const httplib::Request&, httplib::Response& res)
{
    if (!onDebugAudioStop)
    {
        res.status = 503;
        res.set_content(jsonError("audio_stop not wired"), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this]() { onDebugAudioStop(); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-0929 asyncload (TEST-ONLY): cancel the staged load (MainComponent::cancelStagedOpen, Superseded).
void ApiServer::handleDebugCancelLoad(const httplib::Request&, httplib::Response& res)
{
    if (!onDebugCancelLoad)
    {
        res.status = 503;
        res.set_content(jsonError("cancel_load not wired"), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this]() { onDebugCancelLoad(); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U3.4 (TEST-ONLY): the deck tab row + rename box, read ON the message thread (the handleDebugUiText
// shape: <= 2 s wait; a frozen message thread answers {"ok":false,...}; the shared box outlives a late answer).
void ApiServer::handleDebugDeckTabs(const httplib::Request&, httplib::Response& res)
{
    if (!onDebugDeckTabs)
    {
        res.status = 503;
        res.set_content(jsonError("deck_tabs not wired"), "application/json");
        return;
    }
    struct Box { juce::WaitableEvent done; juce::var state; };
    auto box = std::make_shared<Box>();
    const bool posted = juce::MessageManager::callAsync([this, box]() {
        auto state = onDebugDeckTabs();
        if (auto* o = state.getDynamicObject())
            o->setProperty("ok", true);
        box->state = state;
        box->done.signal();
    });
    if (!posted || !box->done.wait(2000))
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("ok", false);
        obj->setProperty("reason", "message thread did not answer within 2 s");
        res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
        return;
    }
    res.set_content(juce::JSON::toString(box->state).toStdString(), "application/json");
}

// s-rta-1002b ui U3.4 (TEST-ONLY): {"deck": i, "op": "begin"|"type"|"enter"|"tab"|"escape"|"focus_lost"|"outside_click",
// "text": "..."} -> the rename box function a click / key reaches (DeckView::renameOpForTests).
void ApiServer::handleDebugDeckRename(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    const juce::String op = json.getProperty("op", "").toString();
    static const juce::StringArray kOps { "begin", "type", "enter", "tab", "escape", "focus_lost", "outside_click" };
    if (!kOps.contains(op) || (op == "begin" && !json.hasProperty("deck")))
    {
        res.status = 400;
        res.set_content(jsonError("op (begin|type|enter|tab|escape|focus_lost|outside_click) required; begin needs deck"),
                        "application/json");
        return;
    }
    if (!onDebugDeckRename)
    {
        res.status = 503;
        res.set_content(jsonError("deck_rename not wired"), "application/json");
        return;
    }
    const int deck = static_cast<int>(json.getProperty("deck", -1));
    const juce::String text = json.getProperty("text", "").toString();
    juce::MessageManager::callAsync([this, deck, op, text]() { onDebugDeckRename(deck, op, text); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U3.4 (TEST-ONLY): {"deck": i} -> the tab button's own onClick (DeckView::clickTabForTests).
void ApiServer::handleDebugTabClick(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("deck"))
    {
        res.status = 400;
        res.set_content(jsonError("deck (int) required"), "application/json");
        return;
    }
    if (!onDebugTabClick)
    {
        res.status = 503;
        res.set_content(jsonError("tab_click not wired"), "application/json");
        return;
    }
    const int deck = static_cast<int>(json["deck"]);
    juce::MessageManager::callAsync([this, deck]() { onDebugTabClick(deck); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U3.4 (TEST-ONLY): {"deck": i} -> a double-click on that tab in JUCE's order
// (DeckView::doubleClickTabForTests).
void ApiServer::handleDebugTabDoubleClick(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("deck"))
    {
        res.status = 400;
        res.set_content(jsonError("deck (int) required"), "application/json");
        return;
    }
    if (!onDebugTabDoubleClick)
    {
        res.status = 503;
        res.set_content(jsonError("tab_dblclick not wired"), "application/json");
        return;
    }
    const int deck = static_cast<int>(json["deck"]);
    juce::MessageManager::callAsync([this, deck]() { onDebugTabDoubleClick(deck); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U3.4 (TEST-ONLY): {"redo": false|true} (body optional, answered at once -- Pitfall 31) -> Edit > Undo /
// Redo (MainComponent::handleMenuCommand kCompUndo / kCompRedo).
void ApiServer::handleDebugUndo(const httplib::Request& req, httplib::Response& res)
{
    if (!onDebugUndo)
    {
        res.status = 503;
        res.set_content(jsonError("undo not wired"), "application/json");
        return;
    }
    const auto json = juce::JSON::parse(juce::String(req.body));
    const bool redo = static_cast<bool>(json.getProperty("redo", false));
    juce::MessageManager::callAsync([this, redo]() { onDebugUndo(redo); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U2.6 (TEST-ONLY): ?layer=L&column=C -> the active deck's clip there, its cell and the Clip inspector,
// read ON the message thread (the handleDebugDeckTabs shape: <= 2 s wait; the shared box outlives a late answer).
void ApiServer::handleDebugClipMedia(const httplib::Request& req, httplib::Response& res)
{
    if (!req.has_param("layer") || !req.has_param("column"))
    {
        res.status = 400;
        res.set_content(jsonError("layer and column (int) query parameters required"), "application/json");
        return;
    }
    if (!onDebugClipMedia)
    {
        res.status = 503;
        res.set_content(jsonError("clip_media not wired"), "application/json");
        return;
    }
    const int layer = juce::String(req.get_param_value("layer")).getIntValue();
    const int column = juce::String(req.get_param_value("column")).getIntValue();
    struct Box { juce::WaitableEvent done; juce::var state; };
    auto box = std::make_shared<Box>();
    const bool posted = juce::MessageManager::callAsync([this, box, layer, column]() {
        auto state = onDebugClipMedia(layer, column);
        if (auto* o = state.getDynamicObject())
            o->setProperty("ok", true);
        box->state = state;
        box->done.signal();
    });
    if (!posted || !box->done.wait(2000))
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("ok", false);
        obj->setProperty("reason", "message thread did not answer within 2 s");
        res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
        return;
    }
    res.set_content(juce::JSON::toString(box->state).toStdString(), "application/json");
}

namespace
{
// {"layer": L, "column": C} -> true and the two ints, else false.
bool parseLayerColumn(const httplib::Request& req, int& layer, int& column)
{
    const auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.hasProperty("layer") || !json.hasProperty("column"))
        return false;
    layer = static_cast<int>(json["layer"]);
    column = static_cast<int>(json["column"]);
    return true;
}
} // namespace

// s-rta-1002b ui U2.6 (TEST-ONLY): {"layer": L, "column": C} -> that cell's menu completion (as if "Show in Finder" was
// chosen; DeckView::revealCellForTests -> MainComponent::revealClipAt, which only records under --test-mode).
void ApiServer::handleDebugRevealClip(const httplib::Request& req, httplib::Response& res)
{
    int layer = 0, column = 0;
    if (!parseLayerColumn(req, layer, column))
    {
        res.status = 400;
        res.set_content(jsonError("layer and column (int) required"), "application/json");
        return;
    }
    if (!onDebugRevealClip)
    {
        res.status = 503;
        res.set_content(jsonError("reveal_clip not wired"), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this, layer, column]() { onDebugRevealClip(layer, column); });
    res.set_content(jsonOk(), "application/json");
}

// s-rta-1002b ui U2.6 (TEST-ONLY): {"layer": L, "column": C} -> that cell's name-bar click (select + inspect).
void ApiServer::handleDebugInspectClip(const httplib::Request& req, httplib::Response& res)
{
    int layer = 0, column = 0;
    if (!parseLayerColumn(req, layer, column))
    {
        res.status = 400;
        res.set_content(jsonError("layer and column (int) required"), "application/json");
        return;
    }
    if (!onDebugInspectClip)
    {
        res.status = 503;
        res.set_content(jsonError("inspect_clip not wired"), "application/json");
        return;
    }
    juce::MessageManager::callAsync([this, layer, column]() { onDebugInspectClip(layer, column); });
    res.set_content(jsonOk(), "application/json");
}
#endif

// --- s-rta-0926 routines slice 1 (plan-routines-s1-final.md 5.1): /api/routine/* ---
//
// Same shape as /api/perf/* above: 503 when unwired, a malformed request is refused here (400, in
// words), everything else is marshalled to the message thread and its outcome (or refusal) is read
// back from /api/routine/status -- lastSaved after a save, lastError after any refusal.

namespace
{
    // Reads an optional boolean key; absent -> std::nullopt (the funnel keeps the routine's value).
    std::optional<bool> optionalBool(const juce::var& json, const char* key)
    {
        if (auto* obj = json.getDynamicObject())
            if (obj->hasProperty(key))
                return static_cast<bool>(obj->getProperty(key));
        return std::nullopt;
    }

    bool hasKey(const juce::var& json, const char* key)
    {
        auto* obj = json.getDynamicObject();
        return obj != nullptr && obj->hasProperty(key);
    }

    bool quantizeKnown(const juce::String& q)
    {
        return q.isEmpty() || q == "off" || q == "beat" || q == "bar" || q == "2bar" || q == "4bar";
    }
}

void ApiServer::handleRoutineSave(const httplib::Request& req, httplib::Response& res)
{
    if (!onRoutineSave)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    RoutineSaveOpts opts;
    opts.name = json.getProperty("name", "").toString();
    if (hasKey(json, "fromBar") && hasKey(json, "toBar"))
    {
        opts.useBars = true;
        opts.fromBar = static_cast<int>(json.getProperty("fromBar", 0));
        opts.toBar = static_cast<int>(json.getProperty("toBar", 0));
    }
    else if (hasKey(json, "fromBeat") && hasKey(json, "toBeat"))
    {
        opts.fromBeat = static_cast<double>(json.getProperty("fromBeat", 0.0));
        opts.toBeat = static_cast<double>(json.getProperty("toBeat", 0.0));
    }
    else
    {
        res.status = 400;
        res.set_content(jsonError("Give fromBeat and toBeat, or fromBar and toBar."), "application/json");
        return;
    }
    opts.slot = static_cast<int>(json.getProperty("slot", -1));
    opts.loop = optionalBool(json, "loop");
    opts.restoreState = optionalBool(json, "restoreState");
    opts.wholeBars = optionalBool(json, "wholeBars");
    opts.quantize = json.getProperty("quantize", "").toString();
    opts.takeFolder = json.getProperty("takeFolder", "").toString();
    if (!quantizeKnown(opts.quantize))
    {
        res.status = 400;
        res.set_content(jsonError("quantize must be off, beat, bar, 2bar or 4bar."), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, opts]() {
        onRoutineSave(opts);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleRoutineFire(const httplib::Request& req, httplib::Response& res)
{
    if (!onRoutineFire)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    if (!hasKey(json, "slot"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'slot'"), "application/json");
        return;
    }
    const int slot = static_cast<int>(json.getProperty("slot", -1));

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, slot]() {
        onRoutineFire(slot);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleRoutineStop(const httplib::Request& req, httplib::Response& res)
{
    if (!onRoutineStop)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    const bool all = static_cast<bool>(json.getProperty("all", false));
    if (!all && !hasKey(json, "slot"))
    {
        res.status = 400;
        res.set_content(jsonError("Give 'slot', or 'all': true"), "application/json");
        return;
    }
    const int slot = static_cast<int>(json.getProperty("slot", -1));

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, slot, all]() {
        onRoutineStop(slot, all);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleRoutineSet(const httplib::Request& req, httplib::Response& res)
{
    if (!onRoutineSet)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    if (!hasKey(json, "slot"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'slot'"), "application/json");
        return;
    }
    RoutineSetOpts opts;
    opts.slot = static_cast<int>(json.getProperty("slot", -1));
    opts.loop = optionalBool(json, "loop");
    opts.restoreState = optionalBool(json, "restoreState");
    opts.quantize = json.getProperty("quantize", "").toString();
    opts.name = json.getProperty("name", "").toString();
    opts.restoreStyle = json.getProperty("restoreStyle", "").toString();
    if (!quantizeKnown(opts.quantize))
    {
        res.status = 400;
        res.set_content(jsonError("quantize must be off, beat, bar, 2bar or 4bar."), "application/json");
        return;
    }
    if (opts.restoreStyle.isNotEmpty() && opts.restoreStyle != "ease" && opts.restoreStyle != "jump")
    {
        res.status = 400;
        res.set_content(jsonError("restoreStyle must be ease or jump."), "application/json");
        return;
    }

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, opts]() {
        onRoutineSet(opts);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleRoutineRemove(const httplib::Request& req, httplib::Response& res)
{
    if (!onRoutineRemove)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    auto json = juce::JSON::parse(juce::String(req.body));
    if (!hasKey(json, "slot"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'slot'"), "application/json");
        return;
    }
    const int slot = static_cast<int>(json.getProperty("slot", -1));

    // `this`-capture safety: see handleSetParam's clip-effect branch note.
    juce::MessageManager::callAsync([this, slot]() {
        onRoutineRemove(slot);
    });

    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleRoutineStatus(const httplib::Request&, httplib::Response& res)
{
    if (!onRoutineStatus)
    {
        res.status = 503;
        res.set_content(jsonError("Routines unavailable"), "application/json");
        return;
    }

    // Synchronous: MainComponent's onRoutineStatus reads nothing but RoutineEngine::status()
    // (a mutex-guarded copy, safe from any thread) -- never the composition's routine vectors.
    res.set_content(juce::JSON::toString(onRoutineStatus()).toStdString(), "application/json");
}
