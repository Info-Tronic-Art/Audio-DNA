#if AUDIODNA_TEST_SERVER

#include "test/TestServer.h"
#include "render/Renderer.h"
#include "features/FeatureBus.h"
#include "effects/EffectChain.h"
#include "effects/Effect.h"
#include "model/Composition.h"
#include "sources/SourceRegistry.h"
#include "sources/ProjectMSource.h"
#include "signal/SignalRegistry.h"
#include "routing/RoutingEngine.h"
#include "mapping/MappingEngine.h"
#include "model/Clip.h"
#include "effects/EffectLibrary.h"
#include "output/OutputPresenter.h"
#include "render/PixelConvert.h"
#include "render/PngWrite.h"
#include <juce_core/juce_core.h>
#if JUCE_MAC
 #include <OpenGL/OpenGL.h>   // after juce_gl.h (via Renderer.h): the output probe's private CGL context
 #include <libproc.h>         // s-rta-0929 vupload: phys_footprint_mb (proc_pid_rusage)
 #include <unistd.h>
#endif
#include <algorithm>
#include <iostream>
#include <vector>

TestServer::TestServer(Renderer& renderer,
                       FeatureBus::Writer featureBusWriter,
                       Composition& composition,
                       EffectChain& effectChain,
                       SourceRegistry& sourceRegistry,
                       SignalRegistry& signalRegistry,
                       RoutingEngine& routingEngine,
                       int port)
    : renderer_(renderer)
    , featureBusWriter_(std::move(featureBusWriter))
    , composition_(composition)
    , effectChain_(effectChain)
    , sourceRegistry_(sourceRegistry)
    , signalRegistry_(signalRegistry)
    , routingEngine_(routingEngine)
    , port_(port)
{
    setupRoutes();
}

TestServer::~TestServer()
{
    stop();
    destroyOutputProbe();
}

void TestServer::injectSnapshot(const FeatureSnapshot& snap, const InjectedOnsetCount& onsetIntent)
{
    // Several HTTP threads (this server's pool + the ApiServer relay) can
    // land here concurrently; the mutex keeps the single Writer's
    // stage-then-publish sequence atomic. HTTP threads only — never a
    // realtime thread — so a mutex is fine here.
    std::lock_guard<std::mutex> lock(injectMutex_);
    jassert(featureBusWriter_.isValid());
    FeatureSnapshot* staging = featureBusWriter_.acquireWrite();
    if (staging == nullptr)
        return;
    *staging = snap;
    // Onset render-path fix: mirror AnalysisThread's one-increment-per-onset-
    // hop bookkeeping. Resolved under the lock against the count the previous
    // inject actually published, so two concurrent injects land on +2, never
    // both on +1 (no caller-side baseline read).
    staging->onsetCount = onsetIntent.resolve(lastInjectedOnsetCount_);
    lastInjectedOnsetCount_ = staging->onsetCount;
    featureBusWriter_.publishWrite();
}

void TestServer::start()
{
    if (running_.load(std::memory_order_relaxed))
        return;

    running_.store(true, std::memory_order_relaxed);
    serverThread_ = std::thread([this]() {
        std::cerr << "[Eyes] HTTP server listening on port " << port_ << std::endl;
        if (!server_.listen("localhost", port_))
        {
            std::cerr << "[Eyes] Failed to start HTTP server on port " << port_ << std::endl;
            running_.store(false, std::memory_order_relaxed);
        }
    });
}

void TestServer::stop()
{
    if (!running_.load(std::memory_order_relaxed))
        return;

    server_.stop();
    if (serverThread_.joinable())
        serverThread_.join();
    running_.store(false, std::memory_order_relaxed);
    // s-rta-0927 outputs-c1: no HTTP thread can use the probe context any more; it goes before the renderer's
    // detach (MainComponent's shutdown law), and the shared frames it read outlive it.
    destroyOutputProbe();
    std::cerr << "[Eyes] HTTP server stopped" << std::endl;
}

// --- JSON helpers ---

std::string TestServer::jsonOk()
{
    return R"({"ok":true})";
}

std::string TestServer::jsonError(const std::string& message)
{
    // Use JUCE JSON for proper escaping
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", false);
    obj->setProperty("error", juce::String(message));
    return juce::JSON::toString(juce::var(obj)).toStdString();
}

// --- Route setup ---

void TestServer::setupRoutes()
{
    server_.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    server_.Post("/api/load_image", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadImage(req, res);
    });

    server_.Post("/api/set_effect", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetEffect(req, res);
    });

    server_.Post("/api/set_effect_chain", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetEffectChain(req, res);
    });

    server_.Post("/api/inject_features", [this](const httplib::Request& req, httplib::Response& res) {
        handleInjectFeatures(req, res);
    });

    server_.Post("/api/render_frame", [this](const httplib::Request& req, httplib::Response& res) {
        handleRenderFrame(req, res);
    });

    server_.Get("/api/state", [this](const httplib::Request& req, httplib::Response& res) {
        handleState(req, res);
    });

    server_.Post("/api/reset", [this](const httplib::Request& req, httplib::Response& res) {
        handleReset(req, res);
    });

    server_.Post("/api/load_source", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadSource(req, res);
    });

    server_.Post("/api/update_source_params", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdateSourceParams(req, res);
    });

    server_.Get("/api/sources", [this](const httplib::Request& req, httplib::Response& res) {
        handleListSources(req, res);
    });

    server_.Post("/api/load_milkdrop_preset", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadMilkDropPreset(req, res);
    });

    // P16: Signal/routing endpoints
    server_.Get("/api/signals", [this](const httplib::Request& req, httplib::Response& res) {
        handleListSignals(req, res);
    });

    server_.Post("/api/add_route", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddRoute(req, res);
    });

    server_.Post("/api/remove_route", [this](const httplib::Request& req, httplib::Response& res) {
        handleRemoveRoute(req, res);
    });

    server_.Get("/api/routes", [this](const httplib::Request& req, httplib::Response& res) {
        handleListRoutes(req, res);
    });

    server_.Post("/api/set_macro", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetMacro(req, res);
    });

    // W6 (outputwindow-arc): test-mode-only mapping add/remove — the
    // MappingTick freeze probe's enabler. Registered HERE only (TestServer,
    // 8080); the production ApiServer (7070) never registers these routes,
    // so a production probe 404s — the same registration gating as
    // inject_features (S2/R6 precedent).
    server_.Post("/api/add_mapping", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddMapping(req, res);
    });

    server_.Post("/api/remove_mapping", [this](const httplib::Request& req, httplib::Response& res) {
        handleRemoveMapping(req, res);
    });

    // S166-L8: composition-tier oracle (globalEffects + the four render-dead
    // scalars). Naming matches this file's existing convention: verb_noun
    // for POST, plural noun for GET (mirrors add_route/remove_route/routes).
    server_.Post("/api/add_global_effect", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddGlobalEffect(req, res);
    });

    server_.Post("/api/remove_global_effect", [this](const httplib::Request& req, httplib::Response& res) {
        handleRemoveGlobalEffect(req, res);
    });

    server_.Post("/api/set_global_effect_bypass", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetGlobalEffectBypass(req, res);
    });

    server_.Get("/api/global_effects", [this](const httplib::Request& req, httplib::Response& res) {
        handleListGlobalEffects(req, res);
    });

    server_.Post("/api/set_composition_params", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetCompositionParams(req, res);
    });

    server_.Get("/api/composition_params", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetCompositionParams(req, res);
    });

    server_.Post("/api/set_clip_opacity", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetClipOpacity(req, res);
    });

    // s-rta-0927 outputs-c1 (plan5-final.md 10.2): the output frame path, offscreen. Test mode only (this server);
    // the production ApiServer (7070) never registers them. No route here ever opens a window.
    server_.Post("/api/set_output_tap", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetOutputTap(req, res);
    });

    server_.Post("/api/output_probe", [this](const httplib::Request& req, httplib::Response& res) {
        handleOutputProbe(req, res);
    });

    // s-rta-0929 vupload (plan-vupload.md 4.6): detach + re-attach the preview panel's GL context on the message thread
    // -- JUCE's context-loss path (openGLContextClosing, then newOpenGLContextCreated on the render thread) -- at once or
    // after {"detached_ms": N}. Answers at once; poll gl_context_gen (/api/state) until it advances. Opens no window.
    // Test mode only (this server).
    server_.Post("/api/debug/gl_context_cycle", [this](const httplib::Request& req, httplib::Response& res) {
        handleGlContextCycle(req, res);
    });

    // s-rta-0927 outputs-c3 (plan5 C3): Restore Last Outputs with nothing to restore, and the display-poll A/B.
    // Neither can open a window (setOutputsTestHooks).
    server_.Post("/api/output_restore_last", [this](const httplib::Request& req, httplib::Response& res) {
        handleOutputRestoreLast(req, res);
    });

    server_.Post("/api/set_output_poll", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetOutputPoll(req, res);
    });
}

// --- Endpoint Handlers ---

void TestServer::handleHealth(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("status", "ready");
    obj->setProperty("gl_version", "4.1");
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("effects_count", effectChain_.getNumEffects());

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleLoadImage(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("filepath"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'filepath' field"), "application/json");
        return;
    }

    juce::File imageFile(obj->getProperty("filepath").toString());
    if (!imageFile.existsAsFile())
    {
        res.status = 404;
        res.set_content(jsonError("File not found: " + imageFile.getFullPathName().toStdString()),
                        "application/json");
        return;
    }

    renderer_.loadImage(imageFile);

    // Give the GL thread time to process the pending image load
    juce::Thread::sleep(100);

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleSetEffect(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("name"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'name' field"), "application/json");
        return;
    }

    juce::String effectName = obj->getProperty("name").toString();
    bool enabled = true;
    if (obj->hasProperty("enabled"))
        enabled = static_cast<bool>(obj->getProperty("enabled"));

    // Find the effect in the chain by display name
    Effect* found = nullptr;
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* fx = effectChain_.getEffect(i);
        if (fx && fx->getName() == effectName)
        {
            found = fx;
            break;
        }
    }

    if (!found)
    {
        res.status = 404;
        res.set_content(jsonError("Effect not found: " + effectName.toStdString()),
                        "application/json");
        return;
    }

    found->setEnabled(enabled);

    // Set parameters if provided
    if (obj->hasProperty("params"))
    {
        if (auto* paramsObj = obj->getProperty("params").getDynamicObject())
        {
            for (auto& prop : paramsObj->getProperties())
            {
                juce::String paramName = prop.name.toString();
                float paramValue = static_cast<float>(static_cast<double>(prop.value));

                for (int p = 0; p < found->getNumParams(); ++p)
                {
                    if (found->getParam(p).name == paramName.toStdString())
                    {
                        found->setParamValue(p, paramValue);
                        break;
                    }
                }
            }
        }
    }

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleSetEffectChain(const httplib::Request& req, httplib::Response& res)
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

    // First disable all effects
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* fx = effectChain_.getEffect(i);
        if (fx) fx->setEnabled(false);
    }

    // Then enable and configure requested effects
    auto* effectsArray = obj->getProperty("effects").getArray();
    if (!effectsArray)
    {
        res.status = 400;
        res.set_content(jsonError("'effects' must be an array"), "application/json");
        return;
    }

    for (const auto& fxVar : *effectsArray)
    {
        auto* fxObj = fxVar.getDynamicObject();
        if (!fxObj) continue;

        juce::String effectName = fxObj->getProperty("name").toString();

        for (int i = 0; i < effectChain_.getNumEffects(); ++i)
        {
            auto* fx = effectChain_.getEffect(i);
            if (fx && fx->getName() == effectName)
            {
                fx->setEnabled(true);

                // Set params
                if (fxObj->hasProperty("params"))
                {
                    if (auto* paramsObj = fxObj->getProperty("params").getDynamicObject())
                    {
                        for (auto& prop : paramsObj->getProperties())
                        {
                            juce::String paramName = prop.name.toString();
                            float paramValue = static_cast<float>(static_cast<double>(prop.value));

                            for (int p = 0; p < fx->getNumParams(); ++p)
                            {
                                if (fx->getParam(p).name == paramName.toStdString())
                                {
                                    fx->setParamValue(p, paramValue);
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleInjectFeatures(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj)
    {
        res.status = 400;
        res.set_content(jsonError("Expected JSON object"), "application/json");
        return;
    }

    // Build the snapshot locally, then publish through the single test-mode
    // Writer (the analysis thread is not running in test mode)
    FeatureSnapshot injected;
    injected.clear();
    FeatureSnapshot* snap = &injected;

    // Map JSON fields to FeatureSnapshot fields
    auto get = [&](const char* name) -> float {
        if (obj->hasProperty(name))
            return static_cast<float>(static_cast<double>(obj->getProperty(name)));
        return 0.0f;
    };

    snap->rms = get("rms");
    snap->peak = get("peak");
    snap->rmsDB = obj->hasProperty("rmsDB") ? get("rmsDB") : -100.0f;
    snap->lufs = obj->hasProperty("lufs") ? get("lufs") : -100.0f;
    snap->dynamicRange = get("dynamicRange");
    snap->transientDensity = get("transientDensity");
    snap->spectralCentroid = get("spectralCentroid");
    snap->spectralFlux = get("spectralFlux");
    snap->spectralFlatness = get("spectralFlatness");
    snap->spectralRolloff = get("spectralRolloff");
    snap->bpm = get("bpm");
    snap->beatPhase = get("beatPhase");
    snap->barPhase = get("barPhase");
    snap->phrasePhase = get("phrasePhase");
    snap->barCount = static_cast<uint16_t>(get("barCount"));
    snap->totalBarCount = static_cast<uint32_t>(get("totalBarCount")); // S168: additive twin of barCount
    snap->resyncBarOrigin = static_cast<uint32_t>(get("resyncBarOrigin")); // s-rta-0925: absent -> 0
    snap->totalBeatCount = static_cast<uint32_t>(get("totalBeatCount")); // s-rta-0927 beat clock: absent -> 0
    snap->structuralState = static_cast<uint8_t>(get("structuralState"));
    snap->dominantPitch = get("dominantPitch");
    snap->pitchConfidence = get("pitchConfidence");
    snap->detectedKey = obj->hasProperty("detectedKey")
        ? static_cast<int>(obj->getProperty("detectedKey")) : -1;
    snap->keyIsMajor = obj->hasProperty("keyIsMajor")
        ? static_cast<bool>(obj->getProperty("keyIsMajor")) : true;
    snap->harmonicChangeDetection = get("harmonicChangeDetection");
    snap->onsetDetected = obj->hasProperty("onsetDetected")
        ? static_cast<bool>(obj->getProperty("onsetDetected")) : false;
    snap->onsetStrength = get("onsetStrength");
    snap->beatInBar = static_cast<uint8_t>(get("beatInBar"));
    snap->downbeatDetected = obj->hasProperty("downbeatDetected")
        ? static_cast<bool>(obj->getProperty("downbeatDetected")) : false;

    // Band energies array
    if (obj->hasProperty("bandEnergies"))
    {
        if (auto* arr = obj->getProperty("bandEnergies").getArray())
        {
            for (int i = 0; i < std::min(7, arr->size()); ++i)
                snap->bandEnergies[i] = static_cast<float>(static_cast<double>((*arr)[i]));
        }
    }

    // Chromagram array
    if (obj->hasProperty("chromagram"))
    {
        if (auto* arr = obj->getProperty("chromagram").getArray())
        {
            for (int i = 0; i < std::min(12, arr->size()); ++i)
                snap->chromagram[i] = static_cast<float>(static_cast<double>((*arr)[i]));
        }
    }

    // MFCCs array
    if (obj->hasProperty("mfccs"))
    {
        if (auto* arr = obj->getProperty("mfccs").getArray())
        {
            for (int i = 0; i < std::min(13, arr->size()); ++i)
                snap->mfccs[i] = static_cast<float>(static_cast<double>((*arr)[i]));
        }
    }

    // P25: Advanced audio analysis features
    snap->sidechainPump = get("sidechainPump");
    snap->swingRatio = obj->hasProperty("swingRatio") ? get("swingRatio") : 0.5f;
    snap->formantPresence = get("formantPresence");
    snap->resonancePeak = get("resonancePeak");
    snap->reeseBass = get("reeseBass");

    // Onset render-path fix: intent only; the count is resolved under
    // injectSnapshot's lock. The cleared snapshot's onsetDetected is true
    // only if THIS request set it, so a bump is never a carried-over phantom.
    InjectedOnsetCount onsetIntent;
    if (obj->hasProperty("onsetCount"))
    {
        onsetIntent.hasExplicit = true;
        onsetIntent.explicitCount = static_cast<uint32_t>(
            std::max(0, static_cast<int>(obj->getProperty("onsetCount"))));
    }
    onsetIntent.bump = snap->onsetDetected;

    injectSnapshot(injected, onsetIntent);

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleRenderFrame(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("output_path"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'output_path' field"), "application/json");
        return;
    }

    juce::String outputPath = obj->getProperty("output_path").toString();
    float timeVal = obj->hasProperty("time")
        ? static_cast<float>(static_cast<double>(obj->getProperty("time"))) : 0.0f;
    int width = obj->hasProperty("width")
        ? static_cast<int>(obj->getProperty("width")) : 0;
    int height = obj->hasProperty("height")
        ? static_cast<int>(obj->getProperty("height")) : 0;

    // s-rta-0928 R2: captureFrame sets the TEST-ONLY canvas lock for this capture and restores the previous one, under
    // its capture flight lock (two concurrent calls at different sizes used to race on the lock here).
    bool ok = renderer_.captureFrame(juce::File(outputPath), timeVal, width, height, true,   // complete frame (R1, C2)
                                     Renderer::CaptureEncoding::Fast);                    // R3: the fast PNG writer

    if (ok)
    {
        auto* result = new juce::DynamicObject();
        result->setProperty("ok", true);
        result->setProperty("path", outputPath);
        result->setProperty("width", width);
        result->setProperty("height", height);
        res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
    }
    else
    {
        res.status = 500;
        res.set_content(jsonError("Frame capture failed"), "application/json");
    }
}

void TestServer::handleState(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("frame_time_ms", static_cast<double>(renderer_.getFrameTimeMs()));
    // s-rta-0926b R1: compositor history memory + the longest frame since the
    // previous /api/state read (reading resets it).
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
    // nothing). peak_gpu_time_ms resets on read like peak_frame_time_ms.
    obj->setProperty("gpu_time_ms", static_cast<double>(renderer_.getGpuTimeMs()));
    obj->setProperty("peak_gpu_time_ms", static_cast<double>(renderer_.takePeakGpuTimeMs()));
    // s-rta-0925: master_level is now the one master (composition_.eff()).
    obj->setProperty("master_level", static_cast<double>(composition_.eff(CompScalar::Opacity)));
    // s-rta-0928b mediaopen: fenced deck-less frames (cumulative): held the canvas / fell to black. Same fields as ApiServer.
    obj->setProperty("fence_hold_frames", static_cast<juce::int64>(renderer_.getFenceHoldFrames()));
    obj->setProperty("fence_black_frames", static_cast<juce::int64>(renderer_.getFenceBlackFrames()));
    if (mediaStateProvider_)
        obj->setProperty("media", mediaStateProvider_());   // s-rta-0928b mediaopen: MediaPresence sweeps / flips
    if (loadWitnessProvider_)
        obj->setProperty("load", loadWitnessProvider_());   // s-rta-0929 asyncload: the asynchronous-load witnesses
    // Onset render-path fix: frames on which the render-frame onset pulse fired.
    obj->setProperty("onset_pulse_frames", static_cast<juce::int64>(renderer_.getOnsetPulseFrames()));
    // s-rta-0928b video: video decodes off the GL thread (VideoPlayer decode thread + VideoRing; probe-video). Same
    // fields as ApiServer. gl_video_decode_calls counts avcodec calls made ON the render thread (0 by construction once
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
    // s-rta-0929 vupload (test mode only): GL contexts created so far (a context cycle advances it), the QoS class of the
    // thread that last ran renderOpenGL (-1 = no frame yet; 33 = QOS_CLASS_USER_INTERACTIVE, 21 = DEFAULT), and the
    // process's physical footprint in MB (proc_pid_rusage; the idle ring trim's witness).
    obj->setProperty("gl_context_gen", static_cast<juce::int64>(renderer_.getGlContextGen()));
    obj->setProperty("gl_thread_qos", renderer_.getGlThreadQos());
#if JUCE_MAC
    {
        rusage_info_v4 ri{};
        if (proc_pid_rusage(getpid(), RUSAGE_INFO_V4, reinterpret_cast<rusage_info_t*>(&ri)) == 0)
            obj->setProperty("phys_footprint_mb", static_cast<double>(ri.ri_phys_footprint) / (1024.0 * 1024.0));
    }
#endif
    // s-rta-0927 outputs-c1: the output frame path (additive). live = output windows open; tap = the TEST-ONLY
    // forced tap; frame_* = the newest completed shared frame (gen 0 = nothing published). Same fields as ApiServer.
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
        // s-rta-0927 outputs-c3 (test mode only): the output manager's counters (poll, reconcile, settings, restore).
        if (outputsTestHooks_.stats)
            outputs->setProperty("manager", outputsTestHooks_.stats());
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

    // Composition state
    obj->setProperty("active_deck", composition_.activeDeckIndex);
    obj->setProperty("num_decks", static_cast<int>(composition_.decks.size()));

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleReset(const httplib::Request&, httplib::Response& res)
{
    // Disable all effects and reset params
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* fx = effectChain_.getEffect(i);
        if (fx)
        {
            fx->setEnabled(false);
            fx->resetParams();
            fx->setDryWet(1.0f);
        }
    }

    // Clear loaded image
    renderer_.clearImage();

    // Clear active source
    renderer_.clearActiveSource();

    // Reset time override
    renderer_.setTimeOverride(-1.0f);

    // Reset master opacity (s-rta-0925: the one master)
    composition_.masterOpacity = 1.0f;

    // Clear injected features
    FeatureSnapshot cleared;
    cleared.clear();
    // Onset render-path fix: the count restarts at 0 -- every consumer's
    // OnsetPulse sees a backwards jump and re-baselines without a pulse.
    injectSnapshot(cleared, InjectedOnsetCount::zero());

    // Give GL thread a frame to process
    juce::Thread::sleep(50);

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleLoadSource(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("source_type"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'source_type' field"), "application/json");
        return;
    }

    std::string sourceType = obj->getProperty("source_type").toString().toStdString();

    if (!sourceRegistry_.isRegistered(sourceType))
    {
        res.status = 404;
        res.set_content(jsonError("Source not found: " + sourceType), "application/json");
        return;
    }

    // Seed the registry's full default list -- what a UI clip carries (MainComponent.cpp source-drop path) -- then
    // overlay the request's values. Without the seed the cached instance keeps whatever the previous caller set:
    // Renderer::renderOpenGL passes nullptr for an empty list and renderSource applies nothing (s-rta-0927
    // tier1-diag H1). createSource() is GL-free; handleListSources already calls it on this thread.
    std::vector<Clip::SourceParam> params;
    if (auto src = sourceRegistry_.createSource(sourceType))
    {
        for (int i = 0; i < src->getNumParams(); ++i)
        {
            const auto& p = src->getParam(i);
            Clip::SourceParam sp;
            sp.name = p.name;
            sp.uniformName = p.uniformName;
            sp.value = p.defaultValue;
            sp.defaultValue = p.defaultValue;
            params.push_back(sp);
        }
    }
    if (obj->hasProperty("params"))
    {
        if (auto* paramsObj = obj->getProperty("params").getDynamicObject())
        {
            for (auto& prop : paramsObj->getProperties())
            {
                const std::string uniform = prop.name.toString().toStdString();
                const float value = static_cast<float>(static_cast<double>(prop.value));
                auto it = std::find_if(params.begin(), params.end(),
                                       [&](const Clip::SourceParam& s) { return s.uniformName == uniform; });
                if (it != params.end())
                    it->value = value;
                else
                {
                    // Unknown name: appended as before; renderSource ignores a uniform the source does not have.
                    Clip::SourceParam sp;
                    sp.uniformName = uniform;
                    sp.value = value;
                    params.push_back(sp);
                }
            }
        }
    }

    renderer_.setActiveSource(sourceType, params);

    // Give GL thread a frame to initialize the source
    juce::Thread::sleep(100);

    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleUpdateSourceParams(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("params"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'params' field"), "application/json");
        return;
    }

    std::vector<Clip::SourceParam> params;
    if (auto* paramsObj = obj->getProperty("params").getDynamicObject())
    {
        for (auto& prop : paramsObj->getProperties())
        {
            Clip::SourceParam sp;
            sp.uniformName = prop.name.toString().toStdString();
            sp.value = static_cast<float>(static_cast<double>(prop.value));
            params.push_back(sp);
        }
    }

    renderer_.updateActiveSourceParams(params);
    res.set_content(jsonOk(), "application/json");
}

void TestServer::handleListSources(const httplib::Request&, httplib::Response& res)
{
    auto ids = sourceRegistry_.getRegisteredIds();

    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> sourcesArr;

    for (const auto& id : ids)
    {
        auto* srcObj = new juce::DynamicObject();
        srcObj->setProperty("id", juce::String(id));
        srcObj->setProperty("name", juce::String(sourceRegistry_.getDisplayName(id)));
        srcObj->setProperty("category", juce::String(sourceRegistry_.getCategory(id)));

        // Create a temporary instance to get param info
        auto src = sourceRegistry_.createSource(id);
        if (src)
        {
            juce::Array<juce::var> paramsArr;
            for (int p = 0; p < src->getNumParams(); ++p)
            {
                auto& param = src->getParam(p);
                auto* paramObj = new juce::DynamicObject();
                paramObj->setProperty("name", juce::String(param.name));
                paramObj->setProperty("uniform", juce::String(param.uniformName));
                paramObj->setProperty("default", static_cast<double>(param.defaultValue));
                paramObj->setProperty("min", static_cast<double>(param.min));
                paramObj->setProperty("max", static_cast<double>(param.max));
                paramsArr.add(juce::var(paramObj));
            }
            srcObj->setProperty("params", paramsArr);
        }

        sourcesArr.add(juce::var(srcObj));
    }

    obj->setProperty("sources", sourcesArr);
    obj->setProperty("count", static_cast<int>(ids.size()));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// === P16: Signal/Routing Endpoints ===

void TestServer::handleListSignals(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> signalsArr;

    for (int i = 0; i < signalRegistry_.getNumSignals(); ++i)
    {
        auto* sig = signalRegistry_.getSignalAt(i);
        if (!sig) continue;

        auto* sigObj = new juce::DynamicObject();
        sigObj->setProperty("id", static_cast<int>(sig->getId()));
        sigObj->setProperty("name", juce::String(sig->getName()));

        static const char* catNames[] = {"Amplitude", "Bands", "Rhythm", "Pitch", "Chroma", "Timbre", "Structure", "Modulation"};
        int catIdx = static_cast<int>(sig->getCategory());
        sigObj->setProperty("category", juce::String(catIdx < 8 ? catNames[catIdx] : "Unknown"));

        static const char* typeNames[] = {"audio", "oscillator", "envelope"};
        int typeIdx = static_cast<int>(sig->getType());
        sigObj->setProperty("type", juce::String(typeIdx < 3 ? typeNames[typeIdx] : "unknown"));

        sigObj->setProperty("value", static_cast<double>(signalRegistry_.getCachedValue(sig->getId())));

        signalsArr.add(juce::var(sigObj));
    }

    obj->setProperty("signals", signalsArr);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleAddRoute(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.isObject())
    {
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    Route route;
    route.sourceId = static_cast<uint32_t>(static_cast<int>(json.getProperty("source_signal_id", 0)));
    route.sourceType = Route::SourceType::Signal;

    // Target: resolve effect name to index in the global chain
    auto effectName = json.getProperty("target_effect", "").toString();
    auto paramName = json.getProperty("target_param", "").toString();

    bool found = false;
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* effect = effectChain_.getEffect(i);
        if (effect && effect->getName() == effectName)
        {
            route.targetEffectIndex = i;
            route.targetScope = Route::TargetScope::Global;

            for (int p = 0; p < effect->getNumParams(); ++p)
            {
                if (effect->getParam(p).name == paramName.toStdString())
                {
                    route.targetParamIndex = p;
                    found = true;
                    break;
                }
            }
            break;
        }
    }

    if (!found)
    {
        res.set_content(jsonError("Effect or param not found: " + effectName.toStdString() + "." + paramName.toStdString()), "application/json");
        return;
    }

    route.outputMin = static_cast<float>(static_cast<double>(json.getProperty("output_min", 0.0)));
    route.outputMax = static_cast<float>(static_cast<double>(json.getProperty("output_max", 1.0)));
    route.threshold = static_cast<float>(static_cast<double>(json.getProperty("threshold", 0.0)));
    route.gain = static_cast<float>(static_cast<double>(json.getProperty("gain", 1.0)));
    route.inverted = static_cast<bool>(json.getProperty("inverted", false));

    uint32_t routeId = routingEngine_.addRoute(route);

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("route_id", static_cast<int>(routeId));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleRemoveRoute(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int routeId = json.getProperty("id", 0);

    bool removed = routingEngine_.removeRoute(static_cast<uint32_t>(routeId));

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", removed);
    if (!removed)
        obj->setProperty("error", "Route not found");
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleListRoutes(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> routesArr;

    for (int i = 0; i < routingEngine_.getNumRoutes(); ++i)
    {
        auto* route = routingEngine_.getRouteAt(i);
        if (!route) continue;

        auto* rObj = new juce::DynamicObject();
        rObj->setProperty("id", static_cast<int>(route->id));
        rObj->setProperty("source_signal_id", static_cast<int>(route->sourceId));

        auto* sig = signalRegistry_.getSignal(route->sourceId);
        rObj->setProperty("source_name", sig ? juce::String(sig->getName()) : juce::String("unknown"));

        // Resolve target effect/param names
        auto* effect = effectChain_.getEffect(route->targetEffectIndex);
        rObj->setProperty("target_effect", effect ? effect->getName() : juce::String("unknown"));
        if (effect && route->targetParamIndex < effect->getNumParams())
            rObj->setProperty("target_param", juce::String(effect->getParam(route->targetParamIndex).name));

        rObj->setProperty("output_min", static_cast<double>(route->outputMin));
        rObj->setProperty("output_max", static_cast<double>(route->outputMax));
        rObj->setProperty("threshold", static_cast<double>(route->threshold));
        rObj->setProperty("gain", static_cast<double>(route->gain));
        rObj->setProperty("inverted", route->inverted);
        rObj->setProperty("enabled", route->enabled);

        routesArr.add(juce::var(rObj));
    }

    obj->setProperty("routes", routesArr);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleSetMacro(const httplib::Request& req, httplib::Response& res)
{
    // Stub: macro setting will be fully implemented when MacroBank integration is done
    auto json = juce::JSON::parse(juce::String(req.body));
    (void)json;

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("note", "Macro endpoint stub — full MacroBank integration pending");
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleAddMapping(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    if (!json.isObject())
    {
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    // W6 scope: the source is fixed to RMS (outputwindow-arc-design.md —
    // "add/remove RMS→param"); target effect/param resolve by name, the
    // same resolution as handleAddRoute.
    Mapping mapping;
    mapping.source = MappingSource::RMS;

    auto effectName = json.getProperty("target_effect", "").toString();
    auto paramName = json.getProperty("target_param", "").toString();

    bool found = false;
    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        auto* effect = effectChain_.getEffect(i);
        if (effect && effect->getName() == effectName)
        {
            mapping.targetEffectId = static_cast<uint32_t>(i);

            for (int p = 0; p < effect->getNumParams(); ++p)
            {
                if (effect->getParam(p).name == paramName.toStdString())
                {
                    mapping.targetParamIndex = static_cast<uint32_t>(p);
                    found = true;
                    break;
                }
            }
            break;
        }
    }

    if (!found)
    {
        res.set_content(jsonError("Effect or param not found: " + effectName.toStdString() + "." + paramName.toStdString()), "application/json");
        return;
    }

    mapping.inputMin  = static_cast<float>(static_cast<double>(json.getProperty("input_min", 0.0)));
    mapping.inputMax  = static_cast<float>(static_cast<double>(json.getProperty("input_max", 1.0)));
    mapping.outputMin = static_cast<float>(static_cast<double>(json.getProperty("output_min", 0.0)));
    mapping.outputMax = static_cast<float>(static_cast<double>(json.getProperty("output_max", 1.0)));
    mapping.smoothing = static_cast<float>(static_cast<double>(json.getProperty("smoothing", 0.15)));

    // mappings_ is message-thread-owned (EffectsRackPanel/PresetManager are
    // the only other writers, and the arc's confinement asserts enforce the
    // owner) — marshal the mutation, same callAsync/fire-and-forget shape
    // as ApiServer's write handlers. The settle sleep mirrors
    // handleLoadImage's queue-then-sleep idiom so the add has normally
    // landed by the time the caller's next request arrives.
    const int numBefore = renderer_.getMappingEngine().getNumMappings();
    juce::MessageManager::callAsync([this, mapping]() {
        renderer_.getMappingEngine().addMapping(mapping);
    });
    juce::Thread::sleep(50);

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("target_effect_index", static_cast<int>(mapping.targetEffectId));
    obj->setProperty("target_param_index", static_cast<int>(mapping.targetParamIndex));
    obj->setProperty("num_mappings_before", numBefore);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleRemoveMapping(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));
    int index = static_cast<int>(json.getProperty("index", 0));

    // Same marshal as handleAddMapping. num_mappings_before lets a caller
    // drain deterministically (repeat remove of index 0 until it reports 0)
    // despite the fire-and-forget apply.
    const int numBefore = renderer_.getMappingEngine().getNumMappings();
    juce::MessageManager::callAsync([this, index]() {
        renderer_.getMappingEngine().removeMapping(index);
    });
    juce::Thread::sleep(50);

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("num_mappings_before", numBefore);
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

// === S166-L8: Composition-Tier Oracle ===
//
// Gives the composition tier (Composition::globalEffects + masterOpacity,
// masterSpeed, Clip::clipOpacity) a surface a caller can drive and read
// back, since today no ctest target links CompositorEngine.cpp/Renderer.cpp
// (headless GL is unavailable in this rig) and no UI path outside a human
// eyeballing the screen can reach it either. (s-rta-0923 lane 3: this
// comment previously called all four scalars "render-dead" -- stale even
// before this lane; masterOpacity/masterSpeed/clipOpacity are already
// render-consumed, see Renderer.cpp:706, Renderer.cpp:415/1241,
// CompositorEngine.cpp:492/604/1239. The fourth, a separate per-composition
// opacity field, was the only genuinely render-dead one and has been
// removed -- it merged into masterOpacity, plan section 4.6.)

void TestServer::handleAddGlobalEffect(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    juce::String effectName = obj ? obj->getProperty("name").toString() : juce::String();
    if (!obj || !obj->hasProperty("name") || effectName.isEmpty())
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'name' field"), "application/json");
        return;
    }

    const auto* def = renderer_.getEffectLibrary().getEffectDef(effectName);
    if (def == nullptr)
    {
        res.status = 404;
        res.set_content(jsonError("Effect not found: " + effectName.toStdString()), "application/json");
        return;
    }

    Clip::EffectSlot slot;
    slot.effectName = effectName.toStdString();
    slot.enabled = obj->hasProperty("enabled") ? static_cast<bool>(obj->getProperty("enabled")) : true;
    slot.bypassed = false;
    slot.dryWet = obj->hasProperty("dryWet")
        ? static_cast<float>(static_cast<double>(obj->getProperty("dryWet"))) : 1.0f;

    // Defaults first (matches EffectStackView::itemDropped's construction,
    // EffectStackView.cpp:512-519), then apply any named overrides from
    // "params" — an unmatched param name is silently skipped, matching this
    // file's own handleSetEffect and ApiServer::handleSetParam's existing
    // convention for the same case.
    for (const auto& p : def->params)
        slot.addParam(p.defaultValue);

    int matchedParams = 0;
    if (obj->hasProperty("params"))
    {
        if (auto* paramsObj = obj->getProperty("params").getDynamicObject())
        {
            for (auto& prop : paramsObj->getProperties())
            {
                juce::String paramName = prop.name.toString();
                float paramValue = static_cast<float>(static_cast<double>(prop.value));
                for (size_t pi = 0; pi < def->params.size(); ++pi)
                {
                    if (def->params[pi].name == paramName.toStdString())
                    {
                        slot.paramValues[pi] = paramValue;
                        ++matchedParams;
                        break;
                    }
                }
            }
        }
    }

    // GL fence (finding, see report): composition_.globalEffects is iterated
    // directly (const ref, no copy) by CompositorEngine::applyGlobalEffects
    // on the GL thread every frame a deck is active (Renderer.cpp ~465-478,
    // landed in 694f8f3) — the EffectCommands.h comment on EffectStackCmd
    // claiming "Global scope (Composition::globalEffects) is NOT currently
    // read anywhere on the GL side" predates that commit and is now stale.
    // push_back can reallocate mid-iteration on the GL thread -> UB/crash.
    // This file's other vector-mutating handlers do NOT need this: P16's
    // handleAddRoute/handleRemoveRoute mutate RoutingEngine::routes_
    // unfenced, but RoutingEngine::processFrame has zero live callers
    // (dead code — Renderer.cpp:233's comment) so routes_ is never read on
    // the GL thread at all; handleAddMapping/handleRemoveMapping marshal
    // via callAsync because MappingEngine::mappings_ is MESSAGE-thread-owned
    // (ticked by MainComponent's Timer, never the GL thread), so callAsync
    // is the right confinement there. globalEffects has neither kind of
    // protection: it is a bare struct field with no container mutex (unlike
    // EffectChain's effectsMutex_) and no message-thread-confinement helper
    // reachable from here — UndoService::withDeckDetached (what the UI's
    // EffectStackCmd uses for this exact vector) lives on MainComponent,
    // which TestServer holds no reference to. The correct, already-
    // precedented fix is the "confinement" pattern documented at Renderer.h's
    // activeSources_ comment (~395-403): run the mutation itself on the GL
    // thread via a blocking executeOnGLThread round-trip, so there is no
    // window in which this thread's push_back can race a live iteration.
    //
    // Reviewer fix-round 1 (S166-L8): executeOnGLThread only constructs and
    // blocks on a BlockingWorker when the context has a live CachedImage
    // (vendored juce_OpenGLContext.cpp's execute(): `if (auto* c =
    // getCachedImage()) c->execute(...); else jassertfalse;`). With no
    // CachedImage — the state previewPanel_'s context enters when hidden or
    // zero-sized, per this repo's own isAttached() guards at
    // Renderer.cpp:823, :890, :924 — the `else` branch returns immediately
    // and the lambda above NEVER RUNS. In a Release build (jassertfalse is a
    // no-op) that silently leaves newIndex at -1 and globalEffects untouched
    // while this handler still reported {"ok":true} — a fabricated success,
    // exactly the lying-oracle failure this lane exists to prevent. Guard
    // and fail loudly instead of proceeding into a call that would silently
    // no-op.
    if (!renderer_.getContext().isAttached())
    {
        res.status = 503;
        res.set_content(jsonError("Renderer not attached (no GL context — cannot safely mutate globalEffects)"),
                        "application/json");
        return;
    }

    int newIndex = -1;
    renderer_.getContext().executeOnGLThread(
        [this, &slot, &newIndex](juce::OpenGLContext&) {
            composition_.globalEffects.push_back(slot);
            newIndex = static_cast<int>(composition_.globalEffects.size()) - 1;
        }, true /* block until the GL thread has actually applied it */);

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("index", newIndex);
    result->setProperty("name", effectName);
    result->setProperty("matched_params", matchedParams);
    result->setProperty("num_global_effects", static_cast<int>(composition_.globalEffects.size()));
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleRemoveGlobalEffect(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("index"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'index' field"), "application/json");
        return;
    }

    int index = static_cast<int>(obj->getProperty("index"));

    // Reviewer fix-round 1 (S166-L8): same detached-context gap as
    // handleAddGlobalEffect above — without it, an out-of-range index and a
    // detached context both fall through to "Index out of range", a
    // misleading diagnosis for the latter. Guard before the fence, not
    // after, so the error names the real cause.
    if (!renderer_.getContext().isAttached())
    {
        res.status = 503;
        res.set_content(jsonError("Renderer not attached (no GL context — cannot safely mutate globalEffects)"),
                        "application/json");
        return;
    }

    // GL fence — same reallocation-race reasoning as handleAddGlobalEffect
    // above: erase() shifts/resizes the live vector CompositorEngine
    // iterates every frame.
    bool removed = false;
    int numAfter = 0;
    renderer_.getContext().executeOnGLThread(
        [this, index, &removed, &numAfter](juce::OpenGLContext&) {
            if (index >= 0 && index < static_cast<int>(composition_.globalEffects.size()))
            {
                composition_.globalEffects.erase(composition_.globalEffects.begin() + index);
                removed = true;
            }
            numAfter = static_cast<int>(composition_.globalEffects.size());
        }, true);

    if (!removed)
    {
        res.status = 400;
        res.set_content(jsonError("Index out of range: " + std::to_string(index)), "application/json");
        return;
    }

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("removed_index", index);
    result->setProperty("num_global_effects", numAfter);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleSetGlobalEffectBypass(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("index") || !obj->hasProperty("bypassed"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'index' or 'bypassed' field"), "application/json");
        return;
    }

    int index = static_cast<int>(obj->getProperty("index"));
    bool bypassed = static_cast<bool>(obj->getProperty("bypassed"));

    if (index < 0 || index >= static_cast<int>(composition_.globalEffects.size()))
    {
        res.status = 400;
        res.set_content(jsonError("Index out of range: " + std::to_string(index)), "application/json");
        return;
    }

    // Value-only write on an already-live slot (no resize) — the same class
    // as this file's handleSetEffect (found->setEnabled(enabled)) and
    // ApiServer::handleSetParam, both unfenced. This mirrors EffectChain's
    // own accepted "DEFERRED BOUNDARY" (EffectChain.h effectsMutex_ comment):
    // the CONTAINER is what needs protecting against reallocation; a lone
    // bool flip on an element that already exists is the same narrow,
    // documented, accepted gap already present for every other per-field
    // effect write in this codebase (CompositorEngine.cpp reads slot.enabled/
    // slot.bypassed/slot.paramValues unsynchronized on the GL thread
    // regardless of which endpoint wrote them), not a new one this endpoint
    // introduces.
    composition_.globalEffects[static_cast<size_t>(index)].bypassed = bypassed;

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("index", index);
    result->setProperty("bypassed", bypassed);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleListGlobalEffects(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    juce::Array<juce::var> arr;

    auto& lib = renderer_.getEffectLibrary();
    for (size_t i = 0; i < composition_.globalEffects.size(); ++i)
    {
        const auto& slot = composition_.globalEffects[i];
        auto* slotObj = new juce::DynamicObject();
        slotObj->setProperty("index", static_cast<int>(i));
        slotObj->setProperty("name", juce::String(slot.effectName));
        slotObj->setProperty("enabled", slot.enabled);
        slotObj->setProperty("bypassed", slot.bypassed);
        slotObj->setProperty("dryWet", static_cast<double>(slot.dryWet));

        auto* paramsObj = new juce::DynamicObject();
        const auto* def = lib.getEffectDef(juce::String(slot.effectName));
        for (size_t pi = 0; pi < slot.paramValues.size(); ++pi)
        {
            juce::String key = (def && pi < def->params.size())
                ? juce::String(def->params[pi].name)
                : ("param" + juce::String(static_cast<int>(pi)));
            paramsObj->setProperty(key, static_cast<double>(slot.paramValues[pi]));
        }
        slotObj->setProperty("params", juce::var(paramsObj));

        arr.add(juce::var(slotObj));
    }

    obj->setProperty("global_effects", arr);
    obj->setProperty("count", static_cast<int>(composition_.globalEffects.size()));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleSetCompositionParams(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj)
    {
        res.status = 400;
        res.set_content(jsonError("Expected JSON object"), "application/json");
        return;
    }

    // masterOpacity/masterSpeed are read on the GL thread through eff()
    // (Renderer.cpp, S167-L4b block); this plain-float write matches every
    // other message-thread writer of these fields.
    bool any = false;
    if (obj->hasProperty("masterOpacity"))
    {
        composition_.masterOpacity = static_cast<float>(static_cast<double>(obj->getProperty("masterOpacity")));
        any = true;
    }
    if (obj->hasProperty("masterSpeed"))
    {
        composition_.masterSpeed = static_cast<float>(static_cast<double>(obj->getProperty("masterSpeed")));
        any = true;
    }
    // s-rta-0926b plan4 S8: the composition canvas size (test mode only) -- both keys together, ints in
    // [16, 7680] x [16, 4320]. Same plain-int write class as masterSpeed above; the GL thread debounces
    // the pair (Renderer::renderOpenGL's canvas block), so a half-applied write never reaches a frame.
    if (obj->hasProperty("outputWidth") || obj->hasProperty("outputHeight"))
    {
        const auto wv = obj->getProperty("outputWidth");
        const auto hv = obj->getProperty("outputHeight");
        const bool ints = (wv.isInt() || wv.isInt64()) && (hv.isInt() || hv.isInt64());
        const int w = ints ? static_cast<int>(wv) : 0;
        const int h = ints ? static_cast<int>(hv) : 0;
        if (!ints || w < 16 || w > 7680 || h < 16 || h > 4320)
        {
            res.status = 400;
            res.set_content(jsonError("outputWidth and outputHeight are required together, "
                                      "ints in [16, 7680] x [16, 4320]"), "application/json");
            return;
        }
        composition_.outputWidth = w;
        composition_.outputHeight = h;
        any = true;
    }

    if (!any)
    {
        res.status = 400;
        res.set_content(jsonError("No known field provided (expected masterOpacity, masterSpeed "
                                  "and/or outputWidth+outputHeight)"),
                        "application/json");
        return;
    }

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("masterOpacity", static_cast<double>(composition_.masterOpacity));
    result->setProperty("masterSpeed", static_cast<double>(composition_.masterSpeed));
    result->setProperty("outputWidth", composition_.outputWidth);
    result->setProperty("outputHeight", composition_.outputHeight);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleGetCompositionParams(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("masterOpacity", static_cast<double>(composition_.masterOpacity));
    obj->setProperty("masterSpeed", static_cast<double>(composition_.masterSpeed));
    obj->setProperty("outputWidth", composition_.outputWidth);     // plan4 S8
    obj->setProperty("outputHeight", composition_.outputHeight);

    // Per-clip clipOpacity readback — mirrors ApiServer::handleComposition's
    // decks -> layers -> clips nesting (ApiServer.cpp ~260-315), scoped to
    // just this field.
    juce::Array<juce::var> deckArray;
    for (size_t di = 0; di < composition_.decks.size(); ++di)
    {
        auto& deck = composition_.decks[di];
        auto* deckObj = new juce::DynamicObject();
        juce::Array<juce::var> layerArray;
        for (size_t li = 0; li < deck.layers.size(); ++li)
        {
            auto& layer = deck.layers[li];
            auto* layerObj = new juce::DynamicObject();
            juce::Array<juce::var> clipArray;
            for (size_t ci = 0; ci < layer.clips.size(); ++ci)
            {
                if (layer.clips[ci].has_value())
                {
                    auto& clip = *layer.clips[ci];
                    auto* clipObj = new juce::DynamicObject();
                    clipObj->setProperty("column", static_cast<int>(ci));
                    clipObj->setProperty("clipOpacity", static_cast<double>(clip.clipOpacity));
                    clipArray.add(juce::var(clipObj));
                }
            }
            layerObj->setProperty("layer", static_cast<int>(li));
            layerObj->setProperty("clips", clipArray);
            layerArray.add(juce::var(layerObj));
        }
        deckObj->setProperty("deck", static_cast<int>(di));
        deckObj->setProperty("layers", layerArray);
        deckArray.add(juce::var(deckObj));
    }
    obj->setProperty("decks", deckArray);

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void TestServer::handleSetClipOpacity(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("layer") || !obj->hasProperty("column") || !obj->hasProperty("clipOpacity"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'layer', 'column', or 'clipOpacity' field"), "application/json");
        return;
    }

    int layerIndex = static_cast<int>(obj->getProperty("layer"));
    int column = static_cast<int>(obj->getProperty("column"));
    float value = static_cast<float>(static_cast<double>(obj->getProperty("clipOpacity")));

    auto* deck = composition_.getActiveDeck();
    auto* clip = deck ? deck->getClip(layerIndex, column) : nullptr;
    if (!clip)
    {
        res.status = 404;
        res.set_content(jsonError("Clip not found at layer " + std::to_string(layerIndex)
                                   + " column " + std::to_string(column)), "application/json");
        return;
    }

    // Plain-float write, no fence: clipOpacity has no renderer consumer
    // today (same reasoning as handleSetCompositionParams above), and this
    // matches the message thread's own unfenced write to the same field
    // (ClipInspector.cpp:336, MainComponent.cpp:5719/5755).
    clip->clipOpacity = value;

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("layer", layerIndex);
    result->setProperty("column", column);
    result->setProperty("clipOpacity", static_cast<double>(value));
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleLoadMilkDropPreset(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    if (parsed.isVoid())
    {
        res.status = 400;
        res.set_content(jsonError("Invalid JSON"), "application/json");
        return;
    }

    auto* obj = parsed.getDynamicObject();
    if (!obj || !obj->hasProperty("preset_path"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'preset_path' field"), "application/json");
        return;
    }

    std::string presetPath = obj->getProperty("preset_path").toString().toStdString();

    // First ensure the projectm_visualizer source is active
    renderer_.setActiveSource("projectm_visualizer");
    juce::Thread::sleep(200); // Give GL thread time to create the source

    // Access the source and load the preset
    auto* source = renderer_.getOrCreateSource("projectm_visualizer");
    if (!source)
    {
        res.status = 500;
        res.set_content(jsonError("Failed to create projectM source"), "application/json");
        return;
    }

#ifdef AUDIODNA_HAS_PROJECTM
    auto* pmSource = dynamic_cast<ProjectMSource*>(source);
    if (pmSource)
    {
        pmSource->loadPreset(presetPath, false);

        // Generate synthetic audio (bass-heavy beat pattern) and feed to projectM
        // This ensures the preset has audio to react to even in test mode
        constexpr int kSynthSamples = 512;
        float synthPCM[kSynthSamples];
        for (int i = 0; i < kSynthSamples; ++i)
        {
            float t = static_cast<float>(i) / 48000.0f;
            // Bass drum at 120 BPM + some mid-range content
            synthPCM[i] = 0.7f * std::sin(2.0f * 3.14159f * 80.0f * t)
                        + 0.3f * std::sin(2.0f * 3.14159f * 440.0f * t)
                        + 0.1f * std::sin(2.0f * 3.14159f * 2000.0f * t);
        }

        // Feed audio multiple times and wait for GL thread to render
        for (int frame = 0; frame < 60; ++frame)
        {
            pmSource->feedAudio(synthPCM, kSynthSamples);
            juce::Thread::sleep(16); // ~60fps
        }

        res.set_content(jsonOk(), "application/json");
    }
    else
#endif
    {
        res.set_content(jsonError("projectM not available"), "application/json");
    }
}

// ---- s-rta-0927 outputs-c1 (plan5 slice C1, plan5-final.md 10.2): the output frame path, offscreen ----
// set_output_tap forces the main renderer's output tap on without any window (exactly as for a live output);
// output_probe presents the newest shared frame through a PRIVATE CGL context -- no window, no drawable -- with the
// same presentSharedFrame() the Output window uses, and writes what a display would show as a PNG. The probe runs on
// an HTTP thread in its own context (a harder path than production, where both sides share JUCE's one GL thread).

void TestServer::handleGlContextCycle(const httplib::Request& req, httplib::Response& res)
{
    // JUCE asserts the message thread in OpenGLContext::detach / attachTo: marshal, answer at once (Pitfall 31).
    // Optional {"detached_ms": N} (0-5000): re-attach N ms after the detach -- the preview hidden for a while (a
    // minimise): nothing is drawn, the decode threads park (250 ms), then the new context's first frames.
    if (renderer_.getContext().getTargetComponent() == nullptr)
    {
        res.status = 409;
        res.set_content(jsonError("the preview GL context is not attached"), "application/json");
        return;
    }
    int detachedMs = 0;
    const auto parsed = juce::JSON::parse(juce::String(req.body));   // kept alive: the object is owned by the var
    if (auto* obj = parsed.getDynamicObject(); obj != nullptr && obj->hasProperty("detached_ms"))
        detachedMs = juce::jlimit(0, 5000, static_cast<int>(obj->getProperty("detached_ms")));
    const auto genBefore = renderer_.getGlContextGen();
    juce::MessageManager::callAsync([this, detachedMs] {
        juce::Component::SafePointer<juce::Component> c(renderer_.getContext().getTargetComponent());
        if (c == nullptr)
            return;
        renderer_.detach();
        auto reattach = [this, c] {
            if (c != nullptr && renderer_.getContext().getTargetComponent() == nullptr)
                renderer_.attachTo(*c);
        };
        if (detachedMs > 0)
            juce::Timer::callAfterDelay(detachedMs, reattach);
        else
            reattach();
    });
    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("gen_before", static_cast<juce::int64>(genBefore));
    result->setProperty("detached_ms", detachedMs);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleSetOutputTap(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr || !obj->hasProperty("enabled"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'enabled' field"), "application/json");
        return;
    }
    const bool enabled = static_cast<bool>(obj->getProperty("enabled"));
    renderer_.setOutputTapForced(enabled);
    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("tap", enabled);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleOutputProbe(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr || !obj->hasProperty("output_path") || !obj->hasProperty("width") || !obj->hasProperty("height"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'width', 'height' or 'output_path'"), "application/json");
        return;
    }
    const int w = static_cast<int>(obj->getProperty("width"));
    const int h = static_cast<int>(obj->getProperty("height"));
    const juce::File outFile(obj->getProperty("output_path").toString());
    if (w <= 0 || h <= 0 || w > 8192 || h > 8192)
    {
        res.status = 400;
        res.set_content(jsonError("width/height must be 1..8192"), "application/json");
        return;
    }

#if JUCE_MAC
    using namespace juce::gl;
    std::lock_guard<std::mutex> lock(probeMutex_);   // one probe at a time: one private context, any HTTP thread
    if (probeContext_ == nullptr)
    {
        CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile,
                                            static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_GL4_Core),
                                            static_cast<CGLPixelFormatAttribute>(0) };
        CGLPixelFormatObj pf = nullptr;
        GLint n = 0;
        CGLContextObj ctx = nullptr;
        if (CGLChoosePixelFormat(attrs, &pf, &n) == kCGLNoError && pf != nullptr)
        {
            CGLCreateContext(pf, nullptr, &ctx);   // shares nothing with any other context
            CGLReleasePixelFormat(pf);
        }
        if (ctx == nullptr)
        {
            res.status = 500;
            res.set_content(jsonError("cannot create the probe's private GL context"), "application/json");
            return;
        }
        probeContext_ = ctx;
    }

    CGLSetCurrentContext(static_cast<CGLContextObj>(probeContext_));
    if (juce::gl::glBlitFramebuffer == nullptr)
        juce::gl::loadFunctions();   // normally already loaded by the main context (dlsym-based, context-free)

    GLuint tex = 0, fbo = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    auto& frames = renderer_.getSharedFrames();
    const bool presented = output::presentSharedFrame(frames, probeState_, fbo, w, h);
    const output::FrontFrame front = frames.front();
    std::vector<uint8_t> pixels(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    if (presented)
    {
        glFinish();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    const int canvasW = probeState_.w, canvasH = probeState_.h;
    const uint32_t gen = probeState_.boundGen;
    CGLSetCurrentContext(nullptr);

    if (!presented)
    {
        res.status = 409;
        res.set_content(jsonError("nothing published yet (no live output and the tap is off)"), "application/json");
        return;
    }

    // What the display shows: the window blits with blending off, so displayed RGB = canvas RGB; alpha forced
    // to 255. Vertical flip: GL rows are bottom-up.
    // s-rta-0928 R5 (C4): the row conversion, byte-identical to the old per-pixel setPixelColour(..., 255) loop
    // (tests/test_pixel_convert.cpp "forceOpaque matches the output_probe loop" -- its oracle IS that loop).
    juce::Image img(juce::Image::ARGB, w, h, false);
    {
        juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
        PixelConvert::rgbaBottomUpToARGB(pixels.data(), w, h, bmp, true);
    }
    const bool ok = PngWrite::writeReplacing(img, outFile);
    if (!ok)
    {
        res.status = 500;
        res.set_content(jsonError("cannot write " + outFile.getFullPathName().toStdString()), "application/json");
        return;
    }
    std::cerr << "[Eyes] Output probe: " << outFile.getFullPathName() << " (" << w << "x" << h << ", gen " << gen
              << " serial " << front.serial << ")" << std::endl;

    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("path", outFile.getFullPathName());
    result->setProperty("width", w);
    result->setProperty("height", h);
    result->setProperty("gen", static_cast<juce::int64>(gen));
    result->setProperty("serial", static_cast<juce::int64>(front.serial));
    result->setProperty("slot", front.slot);
    result->setProperty("canvas_w", canvasW);
    result->setProperty("canvas_h", canvasH);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
#else
    juce::ignoreUnused(w, h, outFile);
    res.status = 501;
    res.set_content(jsonError("output_probe is macOS-only"), "application/json");
#endif
}

// ---- s-rta-0927 outputs-c3 (plan5 slice C3): hot-plug / restore test hooks -- NEVER a window ----

void TestServer::handleOutputRestoreLast(const httplib::Request&, httplib::Response& res)
{
    if (!outputsTestHooks_.stats || !outputsTestHooks_.restoreLastIfNothingToOpen)
    {
        res.status = 501;
        res.set_content(jsonError("output test hooks not wired"), "application/json");
        return;
    }
    // Refuse whenever restoring could open a window: this route exists only to prove that an EMPTY / absent saved
    // set opens nothing. The message thread re-checks before it runs the menu action (MainComponent).
    const int restorable = static_cast<int>(outputsTestHooks_.stats()["restorable"]);
    if (restorable != 0)
    {
        res.status = 409;
        res.set_content(jsonError("refused: Restore Last Outputs would open " + std::to_string(restorable)
                                  + " output window(s) -- this test route never opens a window"),
                        "application/json");
        return;
    }
    outputsTestHooks_.restoreLastIfNothingToOpen();
    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("queued", true);   // runs on the message thread; watch outputs.manager.restore_calls
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::handleSetOutputPoll(const httplib::Request& req, httplib::Response& res)
{
    auto parsed = juce::JSON::parse(juce::String(req.body));
    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr || !obj->hasProperty("enabled"))
    {
        res.status = 400;
        res.set_content(jsonError("Missing 'enabled' field"), "application/json");
        return;
    }
    if (!outputsTestHooks_.setPollEnabled)
    {
        res.status = 501;
        res.set_content(jsonError("output test hooks not wired"), "application/json");
        return;
    }
    const bool enabled = static_cast<bool>(obj->getProperty("enabled"));
    outputsTestHooks_.setPollEnabled(enabled);
    auto* result = new juce::DynamicObject();
    result->setProperty("ok", true);
    result->setProperty("poll_enabled", enabled);
    res.set_content(juce::JSON::toString(juce::var(result)).toStdString(), "application/json");
}

void TestServer::destroyOutputProbe()
{
#if JUCE_MAC
    std::lock_guard<std::mutex> lock(probeMutex_);
    if (probeContext_ == nullptr)
        return;
    CGLSetCurrentContext(static_cast<CGLContextObj>(probeContext_));
    probeState_.release();
    CGLSetCurrentContext(nullptr);
    CGLDestroyContext(static_cast<CGLContextObj>(probeContext_));
    probeContext_ = nullptr;
#endif
}

#endif // AUDIODNA_TEST_SERVER
