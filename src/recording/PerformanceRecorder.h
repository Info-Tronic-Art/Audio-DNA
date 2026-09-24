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
    // in the report). A second touch() on a key whose gesture is still
    // open closes that gesture EXACTLY at the current stamp (as release()
    // would) before opening the new one -- captured breakpoints are never
    // discarded; a grip change (held -> decaying) records as two adjacent
    // gestures.
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

    // s-rta-0924 step 3 (S3-A, plan section 3.2 / critic A2): the host
    // needs a checkpoint0 setter (checkpoint capture from a live
    // Composition is step 3's job, see start()'s comment above), a
    // read-only view of the in-progress document for periodic saves
    // (v2 5.6 #1), and a way to tell whether a continuous write's key
    // already has a gesture open -- MainComponent's manualWrite path is a
    // single touch+write call with no preceding touch() of its own
    // (critic B2), so the host's onHumanWrite() must open the gesture
    // itself when this returns false, or PerformanceRecorder::set's
    // "stray set() without a preceding touch() -- ignored" rule (above)
    // would silently drop every OSC/MIDI/REST continuous capture.
    void setCheckpoint0(PerfState s) { take_.checkpoint0 = std::move(s); }
    const Take& current() const { return take_; }
    bool hasOpenGesture(const ControlPath& k) const { return openGestures_.count(k) != 0; }

private:
    RecorderClock* clock_ = nullptr;
    Take take_;
    juce::File folder_;
    bool recording_ = false;

    struct OpenGesture { Gesture g; double lastCoalesceT = 0.0; };
    std::map<ControlPath, OpenGesture> openGestures_;

    // Shared exact-end helper (addendum 4a): appends the exact end
    // breakpoint (pts.back().y at stamp.beat, stamp {nextSeq++, stamp.t,
    // stamp.sample}) if `og.g.curve.pts` is non-empty, then moves `og.g`
    // into `take_.lanes[key]` (kind Continuous). Does NOT erase from
    // `openGestures_` -- callers do, since touch()'s close-and-reopen
    // needs the entry gone before it inserts the new one, while stop()
    // clears the whole map afterward. Used by release(), stop(), and
    // touch()'s double-touch close.
    void finishGesture(const ControlPath& key, OpenGesture& og, const ClockStamp& stamp);

    static constexpr double kCoalesceWindowSeconds = 0.050;   // July section 2.3 / D3
};
