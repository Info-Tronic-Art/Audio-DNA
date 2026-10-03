#pragma once
#include "model/Composition.h"
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <atomic>
#include <functional>
#include <map>
#include <vector>

// MediaPresence (s-rta-0928b mediaopen, plan-mediaopen.md 4.6 + HARMONY ADOPTION P5): whether an Image / Video clip's
// mediaFile exists, as a per-clip runtime flag (Clip::mediaMissing) instead of a stat() on the GL thread (6 per frame
// in CompositorEngine) and per ClipCell paint (30 Hz x every media cell). Semantics kept: a missing file -> no content
// (FX-only / skipped, as the stat gave), now within <= 1 s + one message-loop hop (was: the next frame); until then a
// resident texture / open player keeps showing. A file that comes back is present again within the same bound.
//
// presence:: -- pure functions over a Composition (message thread; tests/test_media_presence.cpp):
//   mediaPaths -- the Image / Video clips' non-empty mediaFile paths, each once;
//   apply      -- set mediaMissing = !exists on every Image / Video clip whose path was seen; returns the flags CHANGED;
//   seed       -- mediaMissing = !existsAsFile() on a deck's Image / Video clips (a load's staged decks, before the swap).
// MediaPresenceSweeper -- a 1 Hz juce::Timer (message thread) that snapshots mediaPaths, stats them on ONE low-priority
// pool thread and applies the answers on the message thread (WeakReference completion: a sweep landing after the owner
// is gone finds nobody); one sweep in flight at a time. onChanged fires when a flag flipped (the owner repaints the
// grid's "!" indicators). sweeps() / changes() are atomics for /api/state "media" (any thread).
namespace presence
{
struct Seen
{
    juce::String path;
    bool exists = true;
};

inline bool tracks(const Clip& c)
{
    return (c.mediaType == Clip::MediaType::Image || c.mediaType == Clip::MediaType::Video) && c.mediaFile != juce::File();
}

inline std::vector<juce::String> mediaPaths(const Composition& comp)
{
    std::vector<juce::String> out;
    juce::StringArray seen;
    comp.forEachClip([&](const Clip& cell, const ClipSite&) {   // every deck box, retired ones included (bf9b)
        if (!tracks(cell))
            return;
        const auto p = cell.mediaFile.getFullPathName();
        if (!seen.contains(p))
        {
            seen.add(p);
            out.push_back(p);
        }
    });
    return out;
}

inline int apply(Composition& comp, const std::vector<Seen>& seen)
{
    std::map<juce::String, bool> exists;
    for (const auto& s : seen)
        exists[s.path] = s.exists;
    int changed = 0;
    comp.forEachClip([&](Clip& cell, const ClipSite&) {
        if (!tracks(cell))
            return;
        const auto it = exists.find(cell.mediaFile.getFullPathName());
        if (it == exists.end())
            return;   // not in this sweep (a clip added since the snapshot): its seed stands
        const bool missing = !it->second;
        if (cell.mediaMissing != missing)
        {
            cell.mediaMissing = missing;
            ++changed;
        }
    });
    return changed;
}

inline void seed(Deck& deck)
{
    for (auto& row : deck.rows)
        for (auto& cell : row.clips)
            if (cell.has_value())
                cell->mediaMissing = tracks(*cell) && !cell->mediaFile.existsAsFile();
}
} // namespace presence

class MediaPresenceSweeper : private juce::Timer
{
public:
    MediaPresenceSweeper() = default;

    ~MediaPresenceSweeper() override
    {
        stopTimer();
        masterReference.clear();                        // a sweep still queued finds nobody home
        pool_.removeAllJobs(true, kShutdownTimeoutMs);  // jobs capture no `this`
    }

    // Message thread. `comp` must outlive this object (MainComponent's composition_, declared before it).
    void start(Composition& comp, int intervalMs = 1000)
    {
        comp_ = &comp;
        startTimer(intervalMs);
    }

    std::function<void()> onChanged;

    juce::int64 sweeps() const { return sweeps_.load(std::memory_order_relaxed); }
    juce::int64 changes() const { return changes_.load(std::memory_order_relaxed); }

    MediaPresenceSweeper(const MediaPresenceSweeper&) = delete;
    MediaPresenceSweeper& operator=(const MediaPresenceSweeper&) = delete;

private:
    void timerCallback() override
    {
        if (comp_ == nullptr || inFlight_)
            return;
        auto paths = presence::mediaPaths(*comp_);
        inFlight_ = true;
        juce::WeakReference<MediaPresenceSweeper> self(this);
        pool_.addJob([self, paths = std::move(paths)] {
            std::vector<presence::Seen> seen;
            seen.reserve(paths.size());
            for (const auto& p : paths)
                seen.push_back({ p, juce::File(p).existsAsFile() });
            juce::MessageManager::callAsync([self, seen = std::move(seen)] {
                if (auto* sweeper = self.get())
                    sweeper->landed(seen);
            });
        });
    }

    void landed(const std::vector<presence::Seen>& seen)
    {
        inFlight_ = false;
        sweeps_.fetch_add(1, std::memory_order_relaxed);
        if (comp_ == nullptr)
            return;
        const int changed = presence::apply(*comp_, seen);
        if (changed > 0)
        {
            changes_.fetch_add(changed, std::memory_order_relaxed);
            if (onChanged)
                onChanged();
        }
    }

    static constexpr int kShutdownTimeoutMs = 5000;
    Composition* comp_ = nullptr;
    bool inFlight_ = false;
    std::atomic<juce::int64> sweeps_{ 0 }, changes_{ 0 };
    juce::ThreadPool pool_ { juce::ThreadPoolOptions{}.withNumberOfThreads(1).withThreadName("MediaPresence")
                                                     .withDesiredThreadPriority(juce::Thread::Priority::low) };
    JUCE_DECLARE_WEAK_REFERENCEABLE(MediaPresenceSweeper)
};
