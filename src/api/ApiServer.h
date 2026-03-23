#pragma once

#include <httplib.h>
#include <thread>
#include <atomic>
#include <string>

// Forward declarations
class Renderer;
class FeatureBus;
class EffectChain;
class SourceRegistry;
class SignalRegistry;
class RoutingEngine;
struct Composition;
class BindingManager;
class SessionRecorder;

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
              FeatureBus& featureBus,
              Composition& composition,
              EffectChain& effectChain,
              SourceRegistry& sourceRegistry,
              SignalRegistry& signalRegistry,
              RoutingEngine& routingEngine,
              BindingManager& bindingManager,
              SessionRecorder& sessionRecorder,
              int port = 7070);

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
    std::function<void()> onSnapshot;

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
    void handleSetEffect(const httplib::Request& req, httplib::Response& res);
    void handleListEffects(const httplib::Request& req, httplib::Response& res);
    void handleListSources(const httplib::Request& req, httplib::Response& res);
    void handleRenderFrame(const httplib::Request& req, httplib::Response& res);
    void handleReset(const httplib::Request& req, httplib::Response& res);

    // JSON helpers
    std::string jsonOk();
    std::string jsonOk(const std::string& key, const std::string& value);
    std::string jsonError(const std::string& message);

    Renderer& renderer_;
    FeatureBus& featureBus_;
    Composition& composition_;
    EffectChain& effectChain_;
    SourceRegistry& sourceRegistry_;
    SignalRegistry& signalRegistry_;
    RoutingEngine& routingEngine_;
    BindingManager& bindingManager_;
    SessionRecorder& sessionRecorder_;

    int port_;
    httplib::Server server_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
};
