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
#include "model/Clip.h"
#include <juce_core/juce_core.h>
#include <iostream>

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
}

void TestServer::injectSnapshot(const FeatureSnapshot& snap)
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

    injectSnapshot(injected);

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

    // Set locked resolution if requested
    if (width > 0 && height > 0)
        renderer_.setLockedResolution(width, height);

    bool ok = renderer_.captureFrame(juce::File(outputPath), timeVal, width, height);

    // Restore unlocked resolution
    if (width > 0 && height > 0)
        renderer_.setLockedResolution(0, 0);

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
    obj->setProperty("master_level", static_cast<double>(renderer_.getMasterLevel()));

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

    // Reset master level
    renderer_.setMasterLevel(1.0f);

    // Clear injected features
    FeatureSnapshot cleared;
    cleared.clear();
    injectSnapshot(cleared);

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

    // Build params list from JSON if provided
    std::vector<Clip::SourceParam> params;
    if (obj->hasProperty("params"))
    {
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

#endif // AUDIODNA_TEST_SERVER
