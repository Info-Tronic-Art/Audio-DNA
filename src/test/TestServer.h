#pragma once

#if AUDIODNA_TEST_SERVER

#include <httplib.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include "features/FeatureBus.h"
#include "features/OnsetPulse.h"
#include "output/OutputPresenter.h"

// Forward declarations
class Renderer;
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
               FeatureBus::Writer featureBusWriter,
               Composition& composition,
               EffectChain& effectChain,
               SourceRegistry& sourceRegistry,
               SignalRegistry& signalRegistry,
               RoutingEngine& routingEngine,
               int port = 8080);

    ~TestServer();

    // Publish an injected snapshot through the single test-mode Writer
    // (R4). Serialized internally: httplib runs a thread pool, and the
    // ApiServer's test-mode /api/inject_features relays here too, so
    // several HTTP threads can inject concurrently.
    // Onset render-path fix: the published onsetCount is resolved HERE, under
    // injectMutex_, from lastInjectedOnsetCount_ and the caller's intent --
    // the ONE place both inject routes (this server's and the ApiServer
    // relay) compute it. snap.onsetCount as passed in is ignored.
    void injectSnapshot(const FeatureSnapshot& snap, const InjectedOnsetCount& onsetIntent);

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
    void handleLoadMilkDropPreset(const httplib::Request& req, httplib::Response& res);

    // Signal/routing endpoints (P16)
    void handleListSignals(const httplib::Request& req, httplib::Response& res);
    void handleAddRoute(const httplib::Request& req, httplib::Response& res);
    void handleRemoveRoute(const httplib::Request& req, httplib::Response& res);
    void handleListRoutes(const httplib::Request& req, httplib::Response& res);
    void handleSetMacro(const httplib::Request& req, httplib::Response& res);

    // W6 (outputwindow-arc-design.md): test-mode-only mapping add/remove —
    // the MappingTick freeze probe's enabler (RMS → effect param).
    void handleAddMapping(const httplib::Request& req, httplib::Response& res);
    void handleRemoveMapping(const httplib::Request& req, httplib::Response& res);

    // S166-L8: composition-tier oracle. Composition::globalEffects (post-
    // composite chain, applied on the GL thread every frame since 694f8f3 —
    // see handleAddGlobalEffect's fence comment) plus the four render-dead
    // composition/clip scalars, all otherwise unreachable from outside the
    // app.
    void handleAddGlobalEffect(const httplib::Request& req, httplib::Response& res);
    void handleRemoveGlobalEffect(const httplib::Request& req, httplib::Response& res);
    void handleSetGlobalEffectBypass(const httplib::Request& req, httplib::Response& res);
    void handleListGlobalEffects(const httplib::Request& req, httplib::Response& res);
    void handleSetCompositionParams(const httplib::Request& req, httplib::Response& res);
    void handleGetCompositionParams(const httplib::Request& req, httplib::Response& res);
    void handleSetClipOpacity(const httplib::Request& req, httplib::Response& res);

    // s-rta-0927 outputs-c1 (plan5-final.md 10.2): the output frame path, offscreen -- NO window ever.
    // set_output_tap forces the main renderer's output tap; output_probe presents the newest shared frame through a
    // private CGL context (created lazily, destroyed by destroyOutputProbe() from stop()/the destructor) with the same
    // presentSharedFrame() the Output window uses, and writes the PNG a display would show.
    void handleSetOutputTap(const httplib::Request& req, httplib::Response& res);
    void handleOutputProbe(const httplib::Request& req, httplib::Response& res);
    void destroyOutputProbe();

    // JSON helpers
    std::string jsonOk();
    std::string jsonError(const std::string& message);

    Renderer& renderer_;
    FeatureBus::Writer featureBusWriter_;
    std::mutex injectMutex_;  // serializes injectSnapshot across HTTP threads
    // Onset render-path fix: the onsetCount of the last published injected
    // snapshot. Read AND written only inside injectSnapshot, under
    // injectMutex_ -- plain member, the mutex is its only synchronization.
    uint32_t lastInjectedOnsetCount_ = 0;
    Composition& composition_;
    EffectChain& effectChain_;
    SourceRegistry& sourceRegistry_;
    SignalRegistry& signalRegistry_;
    RoutingEngine& routingEngine_;

    // output_probe: one private GL context (a CGLContextObj; void* keeps Apple GL headers out of this header) and its
    // presenter state, used by one HTTP thread at a time under probeMutex_ (HTTP threads only, never a GL frame path).
    std::mutex probeMutex_;
    void* probeContext_ = nullptr;
    output::PresenterGLState probeState_;

    int port_;
    httplib::Server server_;
    std::thread serverThread_;
    std::atomic<bool> running_{false};
};

#endif // AUDIODNA_TEST_SERVER
