#pragma once

#include <httplib.h>
#include <juce_core/juce_core.h>
#include <thread>
#include <atomic>
#include <string>

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
    // see handleLoadComposition.
    std::function<void(juce::File)> onLoadComposition;
    std::function<void()> onSnapshot;
    std::function<void(float bpm)> onSetBpm;
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
    void handleSetLayerOpacity(const httplib::Request& req, httplib::Response& res);
    void handleSwitchDeck(const httplib::Request& req, httplib::Response& res);
    void handleSnapshot(const httplib::Request& req, httplib::Response& res);
    void handleGetBpm(const httplib::Request& req, httplib::Response& res);
    void handleSetBpm(const httplib::Request& req, httplib::Response& res);
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

    int port_;
    // R6 (featurebus-thread-safety-design.md): production = not registered
    // (ctor flag from testMode_) so inject_features 404s outside test mode.
    bool allowFeatureInjection_;
    httplib::Server server_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
};
