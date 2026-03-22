#if AUDIODNA_TEST_SERVER

#include "test/TestServer.h"
#include "render/Renderer.h"
#include "features/FeatureBus.h"
#include "effects/EffectChain.h"
#include "effects/Effect.h"
#include "model/Composition.h"
#include <juce_core/juce_core.h>
#include <iostream>

TestServer::TestServer(Renderer& renderer,
                       FeatureBus& featureBus,
                       Composition& composition,
                       EffectChain& effectChain,
                       int port)
    : renderer_(renderer)
    , featureBus_(featureBus)
    , composition_(composition)
    , effectChain_(effectChain)
    , port_(port)
{
    setupRoutes();
}

TestServer::~TestServer()
{
    stop();
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

    // Write directly to the FeatureBus (analysis thread is not running in test mode)
    FeatureSnapshot* snap = featureBus_.acquireWrite();
    snap->clear();

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

    featureBus_.publishWrite();

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
    FeatureSnapshot* snap = featureBus_.acquireWrite();
    snap->clear();
    featureBus_.publishWrite();

    // Give GL thread a frame to process
    juce::Thread::sleep(50);

    res.set_content(jsonOk(), "application/json");
}

#endif // AUDIODNA_TEST_SERVER
