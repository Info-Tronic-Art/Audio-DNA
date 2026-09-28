#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ImageTexCache: the bookkeeping of the compositor's image textures, decoded off the GL thread (s-rta-0928 renderleft
// R1.2 / R1.5). Pure -- no GL, no JUCE -- so tests/test_image_tex_cache.cpp drives it directly; CompositorEngine owns
// one instance on the GL thread and does the GL calls the returned actions name. Named to avoid juce::ImageCache.
//
// One entry per image path:
//   Pending   a decode job is in flight (or its decoded bytes wait for this frame's upload budget); a frame that needs
//             it gets "pending" -- NEVER "no media" (a 0 clip texture would run the clip's effects as FX Only).
//   Resident  the GL texture exists; a re-validation (a composition swap) may be in flight meanwhile.
//   Failed    the file did not decode: tex 0 ("no media", today's semantics), sticky until the next composition swap.
// Prefetch (applyImageSet / nextPrefetch): at most ONE prefetch job in flight (P = 1), so a demand decode always finds
// a free decoder thread; no NEW prefetch while the resident bytes are at or over the budget (re-validations of
// resident images are always issued; demand lookups are never refused).
namespace ImageTexCache
{
struct Stamp
{
    int64_t mtimeMs = 0;
    int64_t size = -1;
    friend bool operator==(const Stamp&, const Stamp&) = default;
};

enum class Kind : uint8_t { Decoded, Unchanged, Failed };   // what a decode job delivers
enum class State : uint8_t { Pending, Resident, Failed };
enum class Act : uint8_t { Drop, Upload, MarkFailed };      // what the GL wrapper must do with a result

struct Lookup
{
    uint32_t tex = 0;
    bool pending = false;
    bool request = false;   // the caller must issue a decode job for this path now
};

struct PrefetchJob
{
    std::string path;
    std::optional<Stamp> known;   // set = a re-validation: the decoder skips the decode when the stamp is unchanged
};

class Cache   // GL-thread owned
{
public:
    // miss -> a Pending (demand) entry + request = true ONCE; Pending -> pending (a demand for a path that is only
    // queued for prefetch requests it now); Resident -> tex; Failed -> tex 0, not pending, no request (sticky).
    Lookup lookup(const std::string& path)
    {
        auto it = map_.find(path);
        if (it == map_.end())
        {
            Entry e; e.state = State::Pending; e.demand = true; e.inFlight = true;
            map_.emplace(path, e);
            return { 0, true, true };
        }
        Entry& e = it->second;
        switch (e.state)
        {
            case State::Resident: return { e.tex, false, false };
            case State::Failed:   return { 0, false, false };
            case State::Pending:
            default:
            {
                e.demand = true;
                const bool req = !e.inFlight && !e.awaitingUpload;
                if (req) e.inFlight = true;
                return { 0, true, req };
            }
        }
    }

    // Only a Pending or re-validating entry takes a result; anything else (a path cleared or evicted meanwhile, a
    // second result for a resident image) -> Drop. Upload: the caller uploads the bytes -- now, or on a later frame
    // when this frame's upload budget is spent (the entry waits, its bytes are kept: no re-decode) -- then calls
    // onUploaded.
    Act onResult(const std::string& path, Kind kind, const Stamp& s)
    {
        if (prefetchInFlight_ && path == prefetchPath_)
            prefetchInFlight_ = false;
        auto it = map_.find(path);
        if (it == map_.end())
            return Act::Drop;
        Entry& e = it->second;
        if (e.state == State::Pending && e.inFlight)
        {
            e.inFlight = false;
            if (kind == Kind::Decoded) { e.awaitingUpload = true; return Act::Upload; }
            if (kind == Kind::Unchanged) { e.inFlight = false; return Act::Drop; }   // not reachable: no known stamp
            e.state = State::Failed;
            e.demand = false;
            return Act::MarkFailed;
        }
        if (e.state == State::Resident && e.revalidating)
        {
            e.revalidating = false;
            if (kind == Kind::Decoded && !(s == e.stamp)) { e.awaitingUpload = true; return Act::Upload; }
            return Act::Drop;   // Unchanged, or a failed re-validation: the resident texture stays
        }
        return Act::Drop;
    }

    // Is a decoded result for this path still wanted (onResult said Upload and nothing dropped the entry since)?
    bool awaitingUpload(const std::string& path) const
    {
        auto it = map_.find(path);
        return it != map_.end() && it->second.awaitingUpload;
    }

    // -> the texture the caller must delete: the one this upload REPLACED (0 = none), or `tex` itself when the entry
    // is gone (never expected: the caller checks awaitingUpload first).
    uint32_t onUploaded(const std::string& path, uint32_t tex, int w, int h, const Stamp& s, size_t bytes)
    {
        auto it = map_.find(path);
        if (it == map_.end() || !it->second.awaitingUpload)
            return tex;
        Entry& e = it->second;
        const uint32_t old = (e.state == State::Resident) ? e.tex : 0;
        if (e.state == State::Resident)
            residentBytes_ -= std::min(residentBytes_, e.bytes);
        e.state = State::Resident;
        e.tex = tex; e.w = w; e.h = h; e.stamp = s; e.bytes = bytes;
        e.awaitingUpload = false; e.inFlight = false; e.revalidating = false;
        residentBytes_ += bytes;
        return old;
    }

    // Would a frame asking for this path now get "pending" (no entry yet, or a decode / upload outstanding)? No side
    // effect (the crossfade pause asks it before the lookup).
    bool notResident(const std::string& path) const
    {
        auto it = map_.find(path);
        return it == map_.end() || it->second.state == State::Pending;
    }

    bool isDemand(const std::string& path) const
    {
        auto it = map_.find(path);
        return it != map_.end() && it->second.demand;
    }

    // R1.5 -- a composition swap: entries NOT in `ordered` are dropped (returns their textures, to delete; their
    // late results Drop); members are queued in order -- new (and previously Failed) paths for prefetch, resident
    // ones for re-validation (a changed file is re-decoded and replaces its texture). Replaces any earlier queue.
    std::vector<uint32_t> applyImageSet(const std::vector<std::string>& ordered)
    {
        std::vector<uint32_t> dead;
        std::unordered_set<std::string> keep(ordered.begin(), ordered.end());
        for (auto it = map_.begin(); it != map_.end();)
        {
            if (keep.count(it->first) == 0)
            {
                if (it->second.state == State::Resident)
                {
                    dead.push_back(it->second.tex);
                    residentBytes_ -= std::min(residentBytes_, it->second.bytes);
                }
                it = map_.erase(it);
            }
            else if (it->second.state == State::Failed)
                it = map_.erase(it);   // a repaired file shows after the next swap (R-7)
            else
                ++it;
        }
        queue_.assign(ordered.begin(), ordered.end());
        return dead;
    }

    // R1.5: the next prefetch / re-validation job, or nothing (one already in flight, the queue is empty, or only
    // new paths remain while resident bytes >= budgetBytes). Marks the entry in flight.
    std::optional<PrefetchJob> nextPrefetch(size_t budgetBytes)
    {
        if (prefetchInFlight_)
            return std::nullopt;
        while (!queue_.empty())
        {
            std::string path = std::move(queue_.front());
            queue_.pop_front();
            auto it = map_.find(path);
            if (it == map_.end())
            {
                if (residentBytes_ >= budgetBytes)
                    continue;   // over budget: no NEW prefetch (a frame that needs it still requests it)
                Entry e; e.state = State::Pending; e.inFlight = true;
                map_.emplace(path, e);
                prefetchInFlight_ = true; prefetchPath_ = path;
                return PrefetchJob{ path, std::nullopt };
            }
            Entry& e = it->second;
            if (e.state == State::Resident && !e.revalidating && !e.awaitingUpload)
            {
                e.revalidating = true;
                prefetchInFlight_ = true; prefetchPath_ = path;
                return PrefetchJob{ path, e.stamp };
            }
            // Pending (a demand decode already runs) or Failed: nothing to issue
        }
        return std::nullopt;
    }

    // Context loss: every texture (to delete), every entry and the queue go; late results Drop and the next lookup
    // requests again.
    std::vector<uint32_t> clearAll()
    {
        std::vector<uint32_t> dead;
        for (auto& [p, e] : map_)
            if (e.state == State::Resident && e.tex != 0)
                dead.push_back(e.tex);
        map_.clear(); queue_.clear();
        prefetchInFlight_ = false; prefetchPath_.clear();
        residentBytes_ = 0;
        return dead;
    }

    int residentCount() const { return countState(State::Resident); }
    size_t residentBytes() const { return residentBytes_; }
    int pendingCount() const { return countState(State::Pending); }
    bool prefetchInFlight() const { return prefetchInFlight_; }

private:
    struct Entry
    {
        State state = State::Pending;
        uint32_t tex = 0;
        int w = 0, h = 0;
        Stamp stamp;
        size_t bytes = 0;
        bool demand = false;           // a frame asked for it (demand results upload first)
        bool inFlight = false;         // a decode job is outstanding
        bool awaitingUpload = false;   // decoded bytes wait for an upload (onResult said Upload)
        bool revalidating = false;     // resident; a re-validation job is outstanding
    };

    int countState(State s) const
    {
        int n = 0;
        for (const auto& [p, e] : map_)
            if (e.state == s) ++n;
        return n;
    }

    std::unordered_map<std::string, Entry> map_;
    std::deque<std::string> queue_;
    bool prefetchInFlight_ = false;
    std::string prefetchPath_;
    size_t residentBytes_ = 0;
};

// The per-frame upload budget, shared by every image path (the compositor, the legacy single image, sequences): the
// first upload of a frame is always granted, more only while the frame's total stays <= cap. GL thread, reset every
// frame.
struct UploadBudget
{
    size_t cap = 8u << 20;
    size_t used = 0;
    bool any = false;
    void reset() { used = 0; any = false; }
    bool take(size_t bytes)
    {
        if (any && used + bytes > cap) return false;
        any = true; used += bytes;
        return true;
    }
};
} // namespace ImageTexCache
