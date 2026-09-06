#include "recording/PerformanceRecorder.h"
#include <juce_events/juce_events.h>

// R7: PerformanceRecorder is message-thread only. Same guard idiom as
// UndoManager.cpp/DeckCommands.h -- lets headless unit tests (no
// MessageManager instance running at all) call these directly while still
// enforcing the invariant whenever a MessageManager IS running (the app).
#define PERF_RECORDER_ASSERT_MESSAGE_THREAD()                                   \
    jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr       \
            || juce::MessageManager::existsAndIsCurrentThread())

bool PerformanceRecorder::start(const Composition& comp, RecorderClock& clock, const juce::File& takeFolder)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    (void) comp;   // checkpoint0 capture from a live Composition is step 3's job (see header)

    take_ = Take{};
    clock_ = &clock;
    folder_ = takeFolder;
    openGestures_.clear();
    recording_ = true;
    return true;
}

void PerformanceRecorder::discrete(const ControlPath& key, DiscretePoint&& point)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    if (!recording_ || clock_ == nullptr) return;

    DiscretePoint p = std::move(point);
    const auto stamp = clock_->now();
    p.s.seq = take_.nextSeq++;
    p.s.t = stamp.t;
    p.s.sample = stamp.sample;
    p.beat = stamp.beat;
    p.bpm = stamp.bpm;

    auto& lane = take_.lanes[key];
    lane.key = key;
    lane.kind = Lane::Kind::Discrete;
    lane.points.push_back(std::move(p));
}

void PerformanceRecorder::touch(const ControlPath& key, std::string grip)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    if (!recording_ || clock_ == nullptr) return;

    const auto stamp = clock_->now();
    OpenGesture og;
    og.g.grip = std::move(grip);
    og.g.origin = Origin::Human;
    og.g.bpmAtBegin = stamp.bpm;
    og.lastCoalesceT = stamp.t;
    // The curve stays empty until the first set() -- a grip call itself
    // carries no value (D6a); that first set() IS the exact "begin".
    openGestures_[key] = std::move(og);
}

void PerformanceRecorder::set(const ControlPath& key, float v)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    if (!recording_ || clock_ == nullptr) return;

    auto it = openGestures_.find(key);
    if (it == openGestures_.end())
        return;   // stray set() without a preceding touch() -- ignored (row 1 assumption, see report)

    const auto stamp = clock_->now();
    auto& og = it->second;

    // The BEGIN point (index 0, created below the first time set() is
    // called) must stay exact forever (D3) -- coalescing may only
    // overwrite a genuine MID point, so it needs at least 2 points to have
    // one that is not also the begin point. With exactly 1 point,
    // `pts.back()` IS `pts.front()`; overwriting it would silently corrupt
    // "begin" the moment a second set() lands inside the coalescing
    // window.
    const bool haveMidPoint = og.g.curve.pts.size() >= 2;
    const bool coalesce = haveMidPoint && (stamp.t - og.lastCoalesceT) < kCoalesceWindowSeconds;

    if (coalesce)
    {
        // Overwrite the in-progress MID breakpoint rather than growing the
        // curve -- bounds capture to <= 1 new point / 50 ms per lane (D3).
        // Both x (beat) and the parallel stamp move together -- Gesture's
        // stamps[i] must stay the same moment as curve.pts[i] (Lane.h),
        // never drift.
        og.g.curve.pts.back().x = stamp.beat;
        og.g.curve.pts.back().y = v;
        og.g.stamps.back() = { take_.nextSeq++, stamp.t, stamp.sample };
    }
    else
    {
        og.g.curve.pts.push_back({ stamp.beat, v, Breakpoint::Interp::Linear });
        og.g.stamps.push_back({ take_.nextSeq++, stamp.t, stamp.sample });
        og.lastCoalesceT = stamp.t;
    }
}

void PerformanceRecorder::release(const ControlPath& key)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    if (!recording_ || clock_ == nullptr) return;

    auto it = openGestures_.find(key);
    if (it == openGestures_.end())
        return;

    const auto stamp = clock_->now();
    auto& og = it->second;

    // The release point is EXACT regardless of the coalescing window
    // (D3) -- always appended as its own breakpoint, never merged into a
    // coalesced one. Skipped only if touch() was followed by no set() at
    // all (nothing to be exact ABOUT -- a touch-and-let-go with no move).
    if (!og.g.curve.pts.empty())
    {
        const float endValue = og.g.curve.pts.back().y;
        og.g.curve.pts.push_back({ stamp.beat, endValue, Breakpoint::Interp::Linear });
        og.g.stamps.push_back({ take_.nextSeq++, stamp.t, stamp.sample });
    }

    auto& lane = take_.lanes[key];
    lane.key = key;
    lane.kind = Lane::Kind::Continuous;
    lane.gestures.push_back(std::move(og.g));

    openGestures_.erase(it);
}

Take PerformanceRecorder::stop(const Composition& comp)
{
    PERF_RECORDER_ASSERT_MESSAGE_THREAD();
    (void) comp;   // checkpointEnd capture from a live Composition is step 3's job (see header)

    // Mirrors Player::stop's "let go of its hands" (R9): no lane is left
    // mid-touch in the saved take.
    for (auto& [key, og] : openGestures_)
    {
        if (!og.g.curve.pts.empty())
        {
            const auto stamp = clock_ != nullptr ? clock_->now() : ClockStamp{};
            const float endValue = og.g.curve.pts.back().y;
            og.g.curve.pts.push_back({ stamp.beat, endValue, Breakpoint::Interp::Linear });
            og.g.stamps.push_back({ take_.nextSeq++, stamp.t, stamp.sample });
        }
        auto& lane = take_.lanes[key];
        lane.key = key;
        lane.kind = Lane::Kind::Continuous;
        lane.gestures.push_back(std::move(og.g));
    }
    openGestures_.clear();

    recording_ = false;
    clock_ = nullptr;
    return std::move(take_);
}
