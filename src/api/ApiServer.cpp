#include "api/ApiServer.h"
#include "render/Renderer.h"
#include "features/FeatureBus.h"
#include "effects/EffectChain.h"
#include "effects/Effect.h"
#include "effects/EffectLibrary.h"
#include "model/Composition.h"
#include "model/Clip.h"
#include "sources/SourceRegistry.h"
#include "signal/SignalRegistry.h"
#include "routing/RoutingEngine.h"
#include "binding/BindingManager.h"
#include "recording/SessionRecorder.h"
#include <juce_core/juce_core.h>
#include <iostream>

ApiServer::ApiServer(Renderer& renderer,
                     FeatureBus& featureBus,
                     Composition& composition,
                     EffectChain& effectChain,
                     SourceRegistry& sourceRegistry,
                     SignalRegistry& signalRegistry,
                     RoutingEngine& routingEngine,
                     BindingManager& bindingManager,
                     SessionRecorder& sessionRecorder,
                     int port)
    : renderer_(renderer)
    , featureBus_(featureBus)
    , composition_(composition)
    , effectChain_(effectChain)
    , sourceRegistry_(sourceRegistry)
    , signalRegistry_(signalRegistry)
    , routingEngine_(routingEngine)
    , bindingManager_(bindingManager)
    , sessionRecorder_(sessionRecorder)
    , port_(port)
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

    running_.store(true, std::memory_order_relaxed);
    serverThread_ = std::thread([this]() {
        std::cerr << "[API] HTTP server listening on port " << port_ << std::endl;
        if (!server_.listen("0.0.0.0", port_))
        {
            std::cerr << "[API] Failed to start HTTP server on port " << port_ << std::endl;
            running_.store(false, std::memory_order_relaxed);
        }
    });
}

void ApiServer::stop()
{
    if (!running_.load(std::memory_order_relaxed))
        return;

    server_.stop();
    if (serverThread_.joinable())
        serverThread_.join();
    running_.store(false, std::memory_order_relaxed);
    std::cerr << "[API] HTTP server stopped" << std::endl;
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

    server_.Post("/api/set_layer_opacity", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetLayerOpacity(req, res);
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

    // Audio features
    server_.Get("/api/features", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFeatures(req, res);
    });

    server_.Post("/api/inject_features", [this](const httplib::Request& req, httplib::Response& res) {
        handleInjectFeatures(req, res);
    });

    // Media loading
    server_.Post("/api/load_image", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadImage(req, res);
    });

    server_.Post("/api/load_source", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadSource(req, res);
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
}

// --- Endpoint handlers ---

void ApiServer::handleHealth(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("version", "0.1.0");
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleStatus(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("fps", static_cast<double>(renderer_.getFps()));
    obj->setProperty("frameTimeMs", static_cast<double>(renderer_.getFrameTimeMs()));
    obj->setProperty("masterLevel", static_cast<double>(renderer_.getMasterLevel()));
    obj->setProperty("activeDeck", composition_.activeDeckIndex);

    // BPM info from feature bus
    const FeatureSnapshot* snap = featureBus_.getLatestRead();
    if (snap)
    {
        obj->setProperty("bpm", static_cast<double>(snap->bpm));
        obj->setProperty("beatPhase", static_cast<double>(snap->beatPhase));
        obj->setProperty("barPhase", static_cast<double>(snap->barPhase));
        obj->setProperty("phrasePhase", static_cast<double>(snap->phrasePhase));
        obj->setProperty("structuralState", static_cast<int>(snap->structuralState));
        obj->setProperty("detectedGenre", static_cast<int>(snap->detectedGenre));
        obj->setProperty("genreConfidence", static_cast<double>(snap->genreConfidence));
        obj->setProperty("energyState", static_cast<int>(snap->energyState));
    }

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleComposition(const httplib::Request&, httplib::Response& res)
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    obj->setProperty("activeDeck", composition_.activeDeckIndex);
    obj->setProperty("numDecks", static_cast<int>(composition_.decks.size()));
    obj->setProperty("masterOpacity", static_cast<double>(composition_.masterOpacity));

    // Deck details
    juce::Array<juce::var> deckArray;
    for (size_t di = 0; di < composition_.decks.size(); ++di)
    {
        auto& deck = composition_.decks[di];
        auto* deckObj = new juce::DynamicObject();
        deckObj->setProperty("name", juce::String(deck.name));
        deckObj->setProperty("numLayers", static_cast<int>(deck.layers.size()));
        deckObj->setProperty("numColumns", deck.numColumns);

        juce::Array<juce::var> layerArray;
        for (size_t li = 0; li < deck.layers.size(); ++li)
        {
            auto& layer = deck.layers[li];
            auto* layerObj = new juce::DynamicObject();
            layerObj->setProperty("id", static_cast<int>(layer.id));
            layerObj->setProperty("name", juce::String(layer.name));
            layerObj->setProperty("opacity", static_cast<double>(layer.opacity));
            layerObj->setProperty("visible", layer.visible);
            layerObj->setProperty("muted", layer.muted);
            layerObj->setProperty("solo", layer.solo);
            layerObj->setProperty("bypassed", layer.bypassed);
            layerObj->setProperty("activeClipColumn", layer.activeClipColumn);
            layerObj->setProperty("blendMode", static_cast<int>(layer.blendMode));

            juce::Array<juce::var> clipArray;
            for (size_t ci = 0; ci < layer.clips.size(); ++ci)
            {
                if (layer.clips[ci].has_value())
                {
                    auto& clip = *layer.clips[ci];
                    auto* clipObj = new juce::DynamicObject();
                    clipObj->setProperty("id", static_cast<int>(clip.id));
                    clipObj->setProperty("name", juce::String(clip.name));
                    clipObj->setProperty("column", static_cast<int>(ci));
                    clipObj->setProperty("playing", clip.playing);
                    clipObj->setProperty("mediaType", static_cast<int>(clip.mediaType));
                    clipObj->setProperty("sourceType", juce::String(clip.sourceType));
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
        auto* deck = composition_.getActiveDeck();
        if (!deck)
        {
            res.set_content(jsonError("No active deck"), "application/json");
            return;
        }
        auto* lay = deck->getLayer(layer);
        if (!lay)
        {
            res.set_content(jsonError("Invalid layer"), "application/json");
            return;
        }
        auto* clip = lay->getClipAt(column);
        if (!clip)
        {
            res.set_content(jsonError("Invalid clip"), "application/json");
            return;
        }

        for (auto& fx : clip->effects)
        {
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
                            if (pi < fx.paramValues.size())
                                fx.paramValues[pi] = value;
                            res.set_content(jsonOk(), "application/json");
                            return;
                        }
                    }
                }
            }
        }
        res.set_content(jsonError("Effect or param not found on clip"), "application/json");
    }
    else
    {
        // Global effect chain
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
                        res.set_content(jsonOk(), "application/json");
                        return;
                    }
                }
            }
        }
        res.set_content(jsonError("Effect or param not found"), "application/json");
    }
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

    auto* deck = composition_.getActiveDeck();
    if (!deck)
    {
        res.set_content(jsonError("No active deck"), "application/json");
        return;
    }

    auto* lay = deck->getLayer(layer);
    if (!lay)
    {
        res.set_content(jsonError("Invalid layer"), "application/json");
        return;
    }

    lay->opacity = opacity;
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
    const FeatureSnapshot* snap = featureBus_.getLatestRead();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    if (snap)
    {
        obj->setProperty("bpm", static_cast<double>(snap->bpm));
        obj->setProperty("beatPhase", static_cast<double>(snap->beatPhase));
        obj->setProperty("barPhase", static_cast<double>(snap->barPhase));
        obj->setProperty("phrasePhase", static_cast<double>(snap->phrasePhase));
        obj->setProperty("beatInBar", static_cast<int>(snap->beatInBar));
        obj->setProperty("barCount", static_cast<int>(snap->barCount));
    }
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

    // This would need access to BPMTracker — for now, just acknowledge
    res.set_content(jsonOk(), "application/json");
}

void ApiServer::handleGetFeatures(const httplib::Request&, httplib::Response& res)
{
    const FeatureSnapshot* snap = featureBus_.getLatestRead();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);

    if (snap)
    {
        obj->setProperty("rms", static_cast<double>(snap->rms));
        obj->setProperty("peak", static_cast<double>(snap->peak));
        obj->setProperty("rmsDB", static_cast<double>(snap->rmsDB));
        obj->setProperty("lufs", static_cast<double>(snap->lufs));
        obj->setProperty("spectralCentroid", static_cast<double>(snap->spectralCentroid));
        obj->setProperty("spectralFlux", static_cast<double>(snap->spectralFlux));
        obj->setProperty("spectralFlatness", static_cast<double>(snap->spectralFlatness));
        obj->setProperty("bpm", static_cast<double>(snap->bpm));
        obj->setProperty("beatPhase", static_cast<double>(snap->beatPhase));
        obj->setProperty("onsetDetected", snap->onsetDetected);
        obj->setProperty("onsetStrength", static_cast<double>(snap->onsetStrength));
        obj->setProperty("dominantPitch", static_cast<double>(snap->dominantPitch));
        obj->setProperty("structuralState", static_cast<int>(snap->structuralState));

        // P23: Genre detection
        obj->setProperty("detectedGenre", static_cast<int>(snap->detectedGenre));
        obj->setProperty("genreConfidence", static_cast<double>(snap->genreConfidence));
        obj->setProperty("energyState", static_cast<int>(snap->energyState));

        juce::Array<juce::var> bands;
        for (int i = 0; i < 7; ++i)
            bands.add(static_cast<double>(snap->bandEnergies[i]));
        obj->setProperty("bandEnergies", bands);

        juce::Array<juce::var> chroma;
        for (int i = 0; i < 12; ++i)
            chroma.add(static_cast<double>(snap->chromagram[i]));
        obj->setProperty("chromagram", chroma);
    }

    res.set_content(juce::JSON::toString(juce::var(obj)).toStdString(), "application/json");
}

void ApiServer::handleInjectFeatures(const httplib::Request& req, httplib::Response& res)
{
    auto json = juce::JSON::parse(juce::String(req.body));

    FeatureSnapshot snap{};
    // Copy existing if available
    const FeatureSnapshot* existing = featureBus_.getLatestRead();
    if (existing) snap = *existing;

    // Override with provided values
    if (json.hasProperty("rms")) snap.rms = static_cast<float>(static_cast<double>(json["rms"]));
    if (json.hasProperty("peak")) snap.peak = static_cast<float>(static_cast<double>(json["peak"]));
    if (json.hasProperty("rmsDB")) snap.rmsDB = static_cast<float>(static_cast<double>(json["rmsDB"]));
    if (json.hasProperty("bpm")) snap.bpm = static_cast<float>(static_cast<double>(json["bpm"]));
    if (json.hasProperty("beatPhase")) snap.beatPhase = static_cast<float>(static_cast<double>(json["beatPhase"]));
    if (json.hasProperty("barPhase")) snap.barPhase = static_cast<float>(static_cast<double>(json["barPhase"]));
    if (json.hasProperty("phrasePhase")) snap.phrasePhase = static_cast<float>(static_cast<double>(json["phrasePhase"]));
    if (json.hasProperty("spectralCentroid")) snap.spectralCentroid = static_cast<float>(static_cast<double>(json["spectralCentroid"]));
    if (json.hasProperty("spectralFlux")) snap.spectralFlux = static_cast<float>(static_cast<double>(json["spectralFlux"]));
    if (json.hasProperty("onsetStrength")) snap.onsetStrength = static_cast<float>(static_cast<double>(json["onsetStrength"]));
    if (json.hasProperty("onsetDetected")) snap.onsetDetected = static_cast<bool>(json["onsetDetected"]);
    if (json.hasProperty("structuralState")) snap.structuralState = static_cast<uint8_t>(static_cast<int>(json["structuralState"]));

    // P23: Genre detection fields
    if (json.hasProperty("detectedGenre")) snap.detectedGenre = static_cast<uint8_t>(static_cast<int>(json["detectedGenre"]));
    if (json.hasProperty("genreConfidence")) snap.genreConfidence = static_cast<float>(static_cast<double>(json["genreConfidence"]));
    if (json.hasProperty("energyState")) snap.energyState = static_cast<uint8_t>(static_cast<int>(json["energyState"]));

    if (json.hasProperty("bandEnergies"))
    {
        auto* arr = json["bandEnergies"].getArray();
        if (arr)
            for (int i = 0; i < std::min(7, arr->size()); ++i)
                snap.bandEnergies[i] = static_cast<float>(static_cast<double>((*arr)[i]));
    }

    // Write to triple buffer
    FeatureSnapshot* writeBuf = featureBus_.acquireWrite();
    if (writeBuf)
    {
        *writeBuf = snap;
        featureBus_.publishWrite();
    }
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

    // Find effect in chain by name
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
    {
        res.set_content(jsonError("Effect not found: " + name.toStdString()), "application/json");
        return;
    }

    found->setEnabled(enabled);

    if (json.hasProperty("params"))
    {
        auto* paramsObj = json["params"].getDynamicObject();
        if (paramsObj)
        {
            for (const auto& prop : paramsObj->getProperties())
            {
                for (int pi = 0; pi < found->getNumParams(); ++pi)
                {
                    if (found->getParam(pi).name == prop.name.toString().toStdString())
                    {
                        found->getParam(pi).value = static_cast<float>(static_cast<double>(prop.value));
                        break;
                    }
                }
            }
        }
    }

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

    bool ok = renderer_.captureFrame(juce::File(outputPath), time);
    if (ok)
        res.set_content(jsonOk(), "application/json");
    else
        res.set_content(jsonError("Frame capture failed"), "application/json");
}

void ApiServer::handleReset(const httplib::Request&, httplib::Response& res)
{
    renderer_.clearImage();
    renderer_.clearActiveSource();

    for (int i = 0; i < effectChain_.getNumEffects(); ++i)
    {
        if (auto* fx = effectChain_.getEffect(i))
            fx->setEnabled(false);
    }

    res.set_content(jsonOk(), "application/json");
}
