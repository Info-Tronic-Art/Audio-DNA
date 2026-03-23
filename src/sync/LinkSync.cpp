#include "sync/LinkSync.h"
#include <chrono>

LinkSync::LinkSync()
#if AUDIODNA_HAS_LINK
    : link_(120.0) // Default tempo
#endif
{
#if AUDIODNA_HAS_LINK
    link_.setNumPeersCallback([this](std::size_t numPeers) {
        numPeers_.store(static_cast<int>(numPeers), std::memory_order_release);
    });
    link_.setTempoCallback([this](double tempo) {
        bpm_.store(tempo, std::memory_order_release);
    });
#endif
}

LinkSync::~LinkSync()
{
#if AUDIODNA_HAS_LINK
    link_.enable(false);
#endif
}

void LinkSync::setEnabled(bool enabled)
{
#if AUDIODNA_HAS_LINK
    link_.enable(enabled);
#endif
    enabled_.store(enabled, std::memory_order_release);
}

void LinkSync::setBPM(double bpm)
{
#if AUDIODNA_HAS_LINK
    if (enabled_.load(std::memory_order_acquire))
    {
        auto sessionState = link_.captureAppSessionState();
        auto now = link_.clock().micros();
        sessionState.setTempo(bpm, now);
        link_.commitAppSessionState(sessionState);
    }
#endif
    bpm_.store(bpm, std::memory_order_release);
}

void LinkSync::update()
{
#if AUDIODNA_HAS_LINK
    if (!enabled_.load(std::memory_order_acquire))
        return;

    auto sessionState = link_.captureAppSessionState();
    auto now = link_.clock().micros();

    bpm_.store(sessionState.tempo(), std::memory_order_release);
    beatPhase_.store(sessionState.phaseAtTime(now, quantum_), std::memory_order_release);
    numPeers_.store(static_cast<int>(link_.numPeers()), std::memory_order_release);
#endif
}

void LinkSync::requestBeatAtTime()
{
#if AUDIODNA_HAS_LINK
    if (!enabled_.load(std::memory_order_acquire))
        return;

    auto sessionState = link_.captureAppSessionState();
    auto now = link_.clock().micros();
    sessionState.requestBeatAtTime(0.0, now, quantum_);
    link_.commitAppSessionState(sessionState);
#endif
}
