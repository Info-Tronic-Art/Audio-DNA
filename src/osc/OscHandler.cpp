#include "osc/OscHandler.h"
#include <iostream>

OscHandler::OscHandler()
{
    receiver_.addListener(this);
}

OscHandler::~OscHandler()
{
    stopListening();
    receiver_.removeListener(this);
}

bool OscHandler::startListening(int port)
{
    if (listening_.load(std::memory_order_relaxed))
        stopListening();

    if (receiver_.connect(port))
    {
        port_ = port;
        listening_.store(true, std::memory_order_relaxed);
        std::cerr << "[OSC] Listening on port " << port << std::endl;
        return true;
    }

    std::cerr << "[OSC] Failed to listen on port " << port << std::endl;
    return false;
}

void OscHandler::stopListening()
{
    if (!listening_.load(std::memory_order_relaxed))
        return;

    receiver_.disconnect();
    listening_.store(false, std::memory_order_relaxed);
    port_ = 0;
    std::cerr << "[OSC] Stopped listening" << std::endl;
}

void OscHandler::oscMessageReceived(const juce::OSCMessage& message)
{
    auto address = message.getAddressPattern().toString();

    // Require at least one argument for most messages
    if (message.isEmpty())
        return;

    float value = 0.0f;
    if (message[0].isFloat32())
        value = message[0].getFloat32();
    else if (message[0].isInt32())
        value = static_cast<float>(message[0].getInt32());

    // /audiodna/clip/{layer}/{column}
    if (address.startsWith("/audiodna/clip/"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        // parts: "", "audiodna", "clip", "{layer}", "{column}"
        if (parts.size() >= 5)
        {
            int layer = parts[3].getIntValue();
            int column = parts[4].getIntValue();
            if (value > 0.0f && onTriggerClip)
                onTriggerClip(layer, column);
        }
        return;
    }

    // /audiodna/layer/{n}/opacity
    if (address.startsWith("/audiodna/layer/") && address.endsWith("/opacity"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 5)
        {
            int layer = parts[3].getIntValue();
            if (onSetLayerOpacity)
                onSetLayerOpacity(layer, value);
        }
        return;
    }

    // /audiodna/layer/{n}/bypass
    if (address.startsWith("/audiodna/layer/") && address.endsWith("/bypass"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 5)
        {
            int layer = parts[3].getIntValue();
            if (onSetLayerBypass)
                onSetLayerBypass(layer, value > 0.5f);
        }
        return;
    }

    // /audiodna/layer/{n}/solo
    if (address.startsWith("/audiodna/layer/") && address.endsWith("/solo"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 5)
        {
            int layer = parts[3].getIntValue();
            if (onSetLayerSolo)
                onSetLayerSolo(layer, value > 0.5f);
        }
        return;
    }

    // /audiodna/layer/{n}/mute
    if (address.startsWith("/audiodna/layer/") && address.endsWith("/mute"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 5)
        {
            int layer = parts[3].getIntValue();
            if (onSetLayerMute)
                onSetLayerMute(layer, value > 0.5f);
        }
        return;
    }

    // /audiodna/deck/{n}
    if (address.startsWith("/audiodna/deck/"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 4)
        {
            int deckIdx = parts[3].getIntValue();
            if (value > 0.0f && onSwitchDeck)
                onSwitchDeck(deckIdx);
        }
        return;
    }

    // /audiodna/master
    if (address == "/audiodna/master")
    {
        if (onSetMaster)
            onSetMaster(value);
        return;
    }

    // /audiodna/bpm
    if (address == "/audiodna/bpm")
    {
        if (onSetBpm)
            onSetBpm(value);
        return;
    }

    // /audiodna/snapshot
    if (address == "/audiodna/snapshot")
    {
        if (onSnapshot)
            onSnapshot();
        return;
    }

    // /audiodna/macro/{n}
    if (address.startsWith("/audiodna/macro/"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        if (parts.size() >= 4)
        {
            int macroIdx = parts[3].getIntValue();
            if (onSetMacro)
                onSetMacro(macroIdx, value);
        }
        return;
    }

    // /audiodna/effect/{name}/{param}
    if (address.startsWith("/audiodna/effect/"))
    {
        auto parts = juce::StringArray::fromTokens(address, "/", "");
        // parts: "", "audiodna", "effect", "{name}", "{param}"
        if (parts.size() >= 5)
        {
            juce::String effectName = parts[3];
            juce::String paramName = parts[4];
            if (onSetEffectParam)
                onSetEffectParam(effectName, paramName, value);
        }
        return;
    }
}
