#pragma once

#if AUDIODNA_TEST_SERVER

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

// TestServer: Embedded HTTP API for the Eyes visual testing harness.
//
// Provides REST/JSON endpoints that allow Python test scripts to:
//   - Load images and sources into the renderer
//   - Enable/disable effects and set parameters
//   - Inject synthetic audio features (bypassing the analysis thread)
//   - Capture deterministic rendered frames to disk
//   - Query engine state
//
// Runs on a background thread. All GL mutations go through the Renderer's
// pending-operation queue (same pattern as pendingImageFile_).
//
// Thread safety: httplib runs its own thread pool. All methods that touch
// Renderer/FeatureBus/EffectChain use their existing thread-safe APIs.
class TestServer
{
public:
    TestServer(Renderer& renderer,
               FeatureBus& featureBus,
               Composition& composition,
               EffectChain& effectChain,
               SourceRegistry& sourceRegistry,
               SignalRegistry& signalRegistry,
               RoutingEngine& routingEngine,
               int port = 8080);

    ~TestServer();

    // Start the HTTP server on a background thread.
    void start();

    // Stop the server and join the background thread.
    void stop();

    bool isRunning() const { return running_.load(std::memory_order_relaxed); }

    TestServer(const TestServer&) = delete;
    TestServer& operator=(const TestServer&) = delete;

private:
    void setupRoutes();

    // Endpoint handlers
    void handleHealth(const httplib::Request& req, httplib::Response& res);
    void handleLoadImage(const httplib::Request& req, httplib::Response& res);
    void handleSetEffect(const httplib::Request& req, httplib::Response& res);
    void handleSetEffectChain(const httplib::Request& req, httplib::Response& res);
    void handleInjectFeatures(const httplib::Request& req, httplib::Response& res);
    void handleRenderFrame(const httplib::Request& req, httplib::Response& res);
    void handleState(const httplib::Request& req, httplib::Response& res);
    void handleReset(const httplib::Request& req, httplib::Response& res);
    void handleLoadSource(const httplib::Request& req, httplib::Response& res);
    void handleUpdateSourceParams(const httplib::Request& req, httplib::Response& res);
    void handleListSources(const httplib::Request& req, httplib::Response& res);

    // Signal/routing endpoints (P16)
    void handleListSignals(const httplib::Request& req, httplib::Response& res);
    void handleAddRoute(const httplib::Request& req, httplib::Response& res);
    void handleRemoveRoute(const httplib::Request& req, httplib::Response& res);
    void handleListRoutes(const httplib::Request& req, httplib::Response& res);
    void handleSetMacro(const httplib::Request& req, httplib::Response& res);

    // JSON helpers
    std::string jsonOk();
    std::string jsonError(const std::string& message);

    Renderer& renderer_;
    FeatureBus& featureBus_;
    Composition& composition_;
    EffectChain& effectChain_;
    SourceRegistry& sourceRegistry_;
    SignalRegistry& signalRegistry_;
    RoutingEngine& routingEngine_;

    int port_;
    httplib::Server server_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
};

#endif // AUDIODNA_TEST_SERVER
