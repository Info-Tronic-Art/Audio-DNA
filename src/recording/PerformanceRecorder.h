#pragma once
#include "recording/Take.h"
#include "recording/RecorderClock.h"
#include <juce_core/juce_core.h>
#include <map>
#include <string>

struct Composition;

// PerformanceRecorder -- s167 D5/D6: capture ONLY, message-thread only
// (R7 -- jassert'd, not merely commented). Builds a Take incrementally as
// the app calls in; hands it back on stop(). D6's flood rule holds by
// construction here too: nothing in src/recording/ polls anything -- every
// point/gesture exists because a caller made exactly one call.
//
// AudioTap (D10.1, step 2 -- Build order row 2) is NOT part of this
// packet's TARGET FILES. start() therefore does not take one; step 2 wires
// it in alongside the real checkpoint0 capture (both need a live
// Composition/AudioEngine this packet does not touch -- see the builder
// report's DEVIATIONS).
class PerformanceRecorder
{
public:
    // `comp` is accepted now so the signature does not change again in
    // step 3, but checkpoint0 stays default/empty this step -- capturing
    // it FROM a live Composition is MainComponent wiring (out of scope,
    // "model + tests only").
    bool start(const Composition& comp, RecorderClock& clock, const juce::File& takeFolder);

    // Stamps (seq/t/beat/sample/bpm) are minted here from the clock,
    // authoritative; the caller fills v/action/retrigger/origin, and a
    // shared `group` id for points meant to fire together (e.g. a column
    // trigger's per-layer points) -- this class never invents one.
    void discrete(const ControlPath& key, DiscretePoint&& point);

    // Continuous-lane gesture capture (D3/D6): touch() opens it (grip:
    // "held"/"decaying", no value -- D6a, a grip call carries none); set()
    // coalesces to <= 1 point / 50 ms per lane, begin and end always exact
    // (D3); release() closes it. A stray set()/release() without a
    // preceding touch() is ignored (defensive; row 1 assumption, disclosed
    // in the report).
    void touch(const ControlPath& key, std::string grip);
    void set(const ControlPath& key, float v);
    void release(const ControlPath& key);

    // Finalizes checkpointEnd (default/empty this step, same reason as
    // checkpoint0) and synthesizes an exact release for every gesture
    // still open (mirrors Player::stop/R9 -- no lane is left mid-touch in
    // the saved take). Returns the completed take by value (R1 -- no
    // pointer/reference into recorder-owned state survives the call).
    Take stop(const Composition& comp);

    bool isRecording() const { return recording_; }

private:
    RecorderClock* clock_ = nullptr;
    Take take_;
    juce::File folder_;
    bool recording_ = false;

    struct OpenGesture { Gesture g; double lastCoalesceT = 0.0; };
    std::map<ControlPath, OpenGesture> openGestures_;

    static constexpr double kCoalesceWindowSeconds = 0.050;   // July section 2.3 / D3
};
