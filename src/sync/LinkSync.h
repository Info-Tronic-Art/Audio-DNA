#pragma once

#if AUDIODNA_HAS_LINK
#include <ableton/Link.hpp>
#endif

#include <atomic>
#include <cstdint>

// LinkSync: Ableton Link integration for network tempo synchronization.
// When enabled, syncs BPM and beat phase with other Link-enabled apps on the network.
// This is a thin wrapper around Ableton's Link library.
//
// Thread safety: Link callbacks fire from network threads. We use atomics
// for all state that the render/analysis threads read.
class LinkSync
{
public:
    LinkSync();
    ~LinkSync();

    // Enable/disable Link synchronization.
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_.load(std::memory_order_acquire); }

    // Get the current synced BPM. Returns 0 if Link is not active.
    double getBPM() const { return bpm_.load(std::memory_order_acquire); }

    // Set the tempo (propagates to all Link peers).
    void setBPM(double bpm);

    // Get the current beat phase [0, quantum) — typically [0, 4) for 4/4 time.
    double getBeatPhase() const { return beatPhase_.load(std::memory_order_acquire); }

    // Get the quantum (beats per phase cycle, default 4).
    double getQuantum() const { return quantum_; }
    void setQuantum(double q) { quantum_ = q; }

    // Get the number of connected peers (excluding self).
    int getNumPeers() const { return numPeers_.load(std::memory_order_acquire); }

    // Call each frame to update cached state from Link's session.
    // Should be called from the render/analysis thread.
    void update();

    // Force a phase reset (align to downbeat).
    void requestBeatAtTime();

private:
    std::atomic<bool> enabled_{false};
    std::atomic<double> bpm_{120.0};
    std::atomic<double> beatPhase_{0.0};
    std::atomic<int> numPeers_{0};
    double quantum_ = 4.0;

#if AUDIODNA_HAS_LINK
    ableton::Link link_;
#endif
};
