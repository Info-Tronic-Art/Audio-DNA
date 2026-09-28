#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

// SeqVram: the texture memory of image sequences (s-rta-0928b seqvram, .harmony/.reports/s-rta-0928b/plan-seqvram.md).
// Pure -- no GL, no JUCE -- so tests/test_seq_vram.cpp drives it directly; ImageSequence owns the GL calls the returned
// actions name, the Renderer hands every drawn sequence a Grant once per frame.
//
// A sequence plays through a BOUNDED window of recycled textures:
//   - distances(): how many playback steps until each frame is shown again, following the sequence's own trajectory
//     (Loop wraps, PingPong bounces, OneShot orders like Loop because a retrigger restarts at the in-point, reverse
//     flips it, an active out point jumps to the in point as Renderer::syncMedia does).
//   - plan(): request the current frame (demand) and the next kLookAhead trajectory frames; evict the resident frame
//     shown farthest in the future (Belady for a cyclic pass -- never LRU), never the current frame and never the frame
//     on screen (lastShown), and only down to what the allowance holds.
//   - Slots: a per-sequence table of GL textures recycled with glTexSubImage2D (a frame of another size re-specifies a
//     slot); never glGen/glDelete per frame.
//   - allowance(): the free share of kBudgetBytes (all sequences together), never below kMinWindowFrames frames: FLOORS
//     WIN (H10) -- when the drawn sequences' floors alone exceed the budget, the total exceeds it and seq_over_budget
//     says so. Settled total <= max(kBudgetBytes, sum of the drawn floors) + one frame's uploads.
namespace SeqVram
{
// All image sequences together (ASSUMED 1 GiB, H13: an M1 Pro with 32 GiB); the stills have their own budget
// (CompositorEngine kImagePrefetchBudgetBytes).
constexpr size_t kBudgetBytes = size_t{ 1 } << 30;
// Look-ahead 4 covers 30 fps at 1080p and 10 fps at 4K with one decode of latency; the floor = cur + shown + 4 ahead + 2
// arriving.
constexpr int kLookAhead = 4, kMaxOutstanding = 4, kMinWindowFrames = 8;
constexpr int kFar = 1 << 30;   // distance of a frame the trajectory never reaches (outside the in/out range)

struct Stats   // relaxed atomics, written on the GL thread, read by /api/state
{
    std::atomic<int64_t> framesShown{ 0 };     // frame-index changes presented
    std::atomic<int64_t> lateFrames{ 0 };      // current frame not resident: the shown one repeated
    std::atomic<int64_t> pendingFrames{ 0 };   // nothing to show yet
    std::atomic<int64_t> uploads{ 0 };
    std::atomic<int64_t> slotReuses{ 0 };      // glTexSubImage2D into a recycled slot
    std::atomic<int64_t> evictions{ 0 };
    std::atomic<int64_t> staleDrops{ 0 };      // a decoded result no longer inside the window, dropped unuploaded
    std::atomic<int64_t> uploadDeferred{ 0 };  // H1: no slot free this frame, the result waits in ready_
    std::atomic<int64_t> residentBytes{ 0 };   // every sequence's allocated texture bytes (frame top)
    std::atomic<int> openCount{ 0 }, residentSlots{ 0 }, overBudget{ 0 };
};

struct Grant
{
    size_t allowanceBytes = 0;
    uint64_t frameSerial = 0;
    Stats* stats = nullptr;
    float inPoint = 0.0f, outPoint = 1.0f;   // H6: the clip's in/out points (normalized), for the trajectory
};

// This sequence may hold: the free budget, never less than its floor.
inline size_t allowance(size_t othersBytes, size_t minBytes)
{
    return std::max(minBytes, kBudgetBytes > othersBytes ? kBudgetBytes - othersBytes : size_t{ 0 });
}

// The allowance in frames of frameBytes; kMinWindowFrames while the frame size is unknown (H5: never a guess).
inline int allowanceFrames(size_t allowanceBytes, size_t frameBytes)
{
    if (frameBytes == 0)
        return kMinWindowFrames;
    return static_cast<int>(std::max<size_t>(static_cast<size_t>(kMinWindowFrames), allowanceBytes / frameBytes));
}

enum class Mode : uint8_t { Loop, PingPong, OneShot };

struct Transport
{
    int n = 0;
    int cur = 0;
    bool forward = true;   // the direction the playhead moves now (reverse and PingPong's leg folded in)
    Mode mode = Mode::Loop;
    // H6: in/out points as frame indices. inFrame = where a retrigger / the out-point jump lands (seekTo's
    // int(inPoint * (n - 1))). ranged = the out point is active (outPoint < 1): past outFrame the playhead jumps to
    // inFrame (Loop and PingPong -- Renderer::syncMedia seeks, it never reflects there) and OneShot stops.
    bool ranged = false;
    int inFrame = 0;
    int outFrame = 0;
};

// Fills t's in/out fields from normalized points by syncMedia's rules: frame f is on screen while its playhead
// [f/n, (f+1)/n) is below outPoint, so outFrame = ceil(outPoint * n) - 1.
inline void setRange(Transport& t, float inPoint, float outPoint)
{
    const int n = t.n;
    if (n <= 0)
        return;
    t.inFrame = std::clamp(static_cast<int>(static_cast<double>(inPoint) * static_cast<double>(n - 1)), 0, n - 1);
    t.ranged = outPoint < 1.0f;
    const int out = static_cast<int>(std::ceil(static_cast<double>(outPoint) * static_cast<double>(n))) - 1;
    t.outFrame = t.ranged ? std::clamp(out, t.inFrame, n - 1) : n - 1;
}

struct Pos
{
    int f = 0;
    bool fwd = true;
};

// True when a OneShot sequence at p would stop (it never crosses its end: requests stop here).
inline bool atOneShotEnd(const Transport& t, Pos p)
{
    const int hi = t.ranged ? t.outFrame : t.n - 1;
    return p.fwd ? p.f >= hi : p.f <= 0;
}

// One playback step (the next frame index shown) of the trajectory.
inline Pos step(const Transport& t, Pos p)
{
    const int n = t.n;
    if (n <= 1)
        return p;
    const int hi = t.ranged ? t.outFrame : n - 1;
    if (t.ranged && p.f > hi)
        return { t.inFrame, p.fwd };   // past the out point: syncMedia seeks to the in point
    if (p.fwd)
    {
        if (p.f < hi)
            return { p.f + 1, true };
        if (t.ranged)
            return { t.inFrame, true };                     // the out point: seekTo(inPoint) (OneShot: a retrigger)
        if (t.mode == Mode::PingPong)
            return { n - 2, false };                        // reflect: n-1 -> n-2
        return { t.mode == Mode::OneShot ? t.inFrame : 0, true };   // Loop wraps; a OneShot retrigger restarts at in
    }
    if (p.f > 0)
        return { p.f - 1, false };
    if (t.mode == Mode::PingPong)
        return { 1, true };                                 // reflect: 0 -> 1
    return { t.ranged ? t.inFrame : n - 1, false };         // reverse wraps to the end (an active out point: to in)
}

// dist[j] = playback steps from cur until frame j is shown (0 at cur; kFar if never), by ONE walk of the trajectory
// (<= 2n + 2 steps). Steps count frame changes, so a PingPong revisit of cur counts (n = 6, cur = 1, backward:
// dist[0] = 1, dist[2] = 3).
inline void distances(const Transport& t, std::vector<int>& dist)
{
    const int n = std::max(0, t.n);
    dist.assign(static_cast<size_t>(n), kFar);
    if (n == 0)
        return;
    Pos p{ std::clamp(t.cur, 0, n - 1), t.forward };
    dist[static_cast<size_t>(p.f)] = 0;
    int found = 1;
    for (int k = 1; k <= 2 * n + 2 && found < n; ++k)
    {
        p = step(t, p);
        auto& d = dist[static_cast<size_t>(p.f)];
        if (d == kFar)
        {
            d = k;
            ++found;
        }
    }
}

struct Plan
{
    std::vector<int> request, evict;
};

// Requests: cur first if not resident / requested / failed (demand), then the frames of the next kLookAhead trajectory
// steps that are none of those -- counted in steps, so a failed or resident frame does not extend the look-ahead; a
// OneShot never requests past its end; at most freeOutstanding. Evictions: while resident - evicted + incoming >
// allowanceFrames, the resident frame with the largest dist that is neither cur nor lastShown; stop when none is
// evictable (the floor: cur and lastShown stay even over the allowance).
inline Plan plan(const Transport& t, const std::vector<int>& dist, const std::vector<uint8_t>& resident,
                 const std::vector<uint8_t>& requested, const std::vector<uint8_t>& failed, int lastShown,
                 int allowanceFrames, int incoming, int freeOutstanding)
{
    Plan out;
    const int n = t.n;
    if (n <= 0 || static_cast<int>(dist.size()) != n || static_cast<int>(resident.size()) != n
        || static_cast<int>(requested.size()) != n || static_cast<int>(failed.size()) != n)
        return out;
    const int cur = std::clamp(t.cur, 0, n - 1);
    auto need = [&](int j) {
        const auto i = static_cast<size_t>(j);
        return resident[i] == 0 && requested[i] == 0 && failed[i] == 0
               && std::find(out.request.begin(), out.request.end(), j) == out.request.end();
    };
    int room = std::max(0, freeOutstanding);
    if (room > 0 && need(cur))
    {
        out.request.push_back(cur);
        --room;
    }
    Pos p{ cur, t.forward };
    for (int k = 0; k < kLookAhead && room > 0; ++k)
    {
        if (t.mode == Mode::OneShot && atOneShotEnd(t, p))
            break;
        p = step(t, p);
        if (need(p.f))
        {
            out.request.push_back(p.f);
            --room;
        }
    }

    int count = 0;
    std::vector<int> candidates;
    for (int j = 0; j < n; ++j)
        if (resident[static_cast<size_t>(j)] != 0)
        {
            ++count;
            if (j != cur && j != lastShown)
                candidates.push_back(j);
        }
    const int excess = count + std::max(0, incoming) - allowanceFrames;
    if (excess > 0 && !candidates.empty())
    {
        std::stable_sort(candidates.begin(), candidates.end(), [&](int a, int b) {
            return dist[static_cast<size_t>(a)] > dist[static_cast<size_t>(b)];
        });
        candidates.resize(static_cast<size_t>(std::min<int>(excess, static_cast<int>(candidates.size()))));
        out.evict = std::move(candidates);
    }
    return out;
}

// A decoded result is still wanted iff its frame is cur, lastShown, or inside the window (dist < allowanceFrames).
inline bool wanted(const std::vector<int>& dist, int j, int cur, int lastShown, int allowanceFrames)
{
    if (j == cur || j == lastShown)
        return true;
    return j >= 0 && j < static_cast<int>(dist.size()) && dist[static_cast<size_t>(j)] < allowanceFrames;
}

// A per-sequence table of equal-size GL textures (GL-thread owned; the caller does the GL calls the returned Act names).
// Evicting frees a slot (no GL call); shrink() hands back free slots beyond a smaller allowance; releaseAll() hands back
// every texture and empties the table (H2: a new context never reuses an old name). Out-of-range slots are no-ops (H1).
class Slots
{
public:
    // Reuse = glTexSubImage2D into the slot (same size); Respecify = glTexImage2D on the slot (another size; parameters
    // persist on the texture object); Create = glGenTextures + glTexImage2D + parameters, then bind(); Full = no slot
    // free and none may be created: do not upload (H1).
    enum class Act : uint8_t { Reuse, Respecify, Create, Full };
    struct Acquire
    {
        int slot = -1;
        Act act = Act::Full;
    };

    // Prefers a free slot of the same size, then any free slot, then Create while size() < cap; marks the slot
    // occupied by frame. Full changes nothing.
    Acquire acquire(int frame, int w, int h, int cap)
    {
        const int mine = slotOf(frame);
        if (mine >= 0)
            return reuseOrRespecify(mine, frame, w, h);
        int anyFree = -1;
        for (int i = 0; i < size(); ++i)
        {
            const Slot& s = slots_[static_cast<size_t>(i)];
            if (s.frame >= 0)
                continue;
            if (s.tex != 0 && s.w == w && s.h == h)
                return reuseOrRespecify(i, frame, w, h);
            if (anyFree < 0)
                anyFree = i;
        }
        if (anyFree >= 0)
            return reuseOrRespecify(anyFree, frame, w, h);
        if (size() < cap)
        {
            slots_.push_back(Slot{ 0, w, h, frame });
            return { size() - 1, Act::Create };
        }
        return {};
    }

    void bind(int slot, uint32_t tex)   // after Create: the new texture name
    {
        if (valid(slot))
            slots_[static_cast<size_t>(slot)].tex = tex;
    }

    void release(int frame)   // the slot becomes free (its VRAM kept for reuse)
    {
        const int s = slotOf(frame);
        if (s >= 0)
            slots_[static_cast<size_t>(s)].frame = -1;
    }

    // Textures of FREE slots removed until size() <= cap (to delete); occupied slots stay.
    std::vector<uint32_t> shrink(int cap)
    {
        std::vector<uint32_t> gone;
        for (int i = size() - 1; i >= 0 && size() > std::max(0, cap); --i)
        {
            const Slot s = slots_[static_cast<size_t>(i)];
            if (s.frame >= 0)
                continue;
            if (s.tex != 0)
                gone.push_back(s.tex);
            slots_.erase(slots_.begin() + i);
        }
        return gone;
    }

    std::vector<uint32_t> releaseAll()   // every texture (context loss / retire); the table is emptied (H2)
    {
        std::vector<uint32_t> gone;
        for (const Slot& s : slots_)
            if (s.tex != 0)
                gone.push_back(s.tex);
        slots_.clear();
        return gone;
    }

    int slotOf(int frame) const
    {
        if (frame < 0)
            return -1;
        for (int i = 0; i < size(); ++i)
            if (slots_[static_cast<size_t>(i)].frame == frame)
                return i;
        return -1;
    }
    uint32_t texOf(int slot) const { return valid(slot) ? slots_[static_cast<size_t>(slot)].tex : 0u; }
    int frameOf(int slot) const { return valid(slot) ? slots_[static_cast<size_t>(slot)].frame : -1; }

    size_t allocatedBytes() const   // every allocated slot, free or not = the VRAM held
    {
        size_t b = 0;
        for (const Slot& s : slots_)
            if (s.tex != 0)
                b += static_cast<size_t>(s.w) * static_cast<size_t>(s.h) * 4u;
        return b;
    }
    int size() const { return static_cast<int>(slots_.size()); }
    int occupied() const
    {
        return static_cast<int>(std::count_if(slots_.begin(), slots_.end(), [](const Slot& s) { return s.frame >= 0; }));
    }

private:
    struct Slot
    {
        uint32_t tex = 0;
        int w = 0, h = 0, frame = -1;
    };
    std::vector<Slot> slots_;

    bool valid(int slot) const { return slot >= 0 && slot < size(); }

    Acquire reuseOrRespecify(int i, int frame, int w, int h)
    {
        Slot& s = slots_[static_cast<size_t>(i)];
        const Act act = s.tex == 0 ? Act::Create : (s.w == w && s.h == h ? Act::Reuse : Act::Respecify);
        s.frame = frame;
        s.w = w;
        s.h = h;
        return { i, act };
    }
};
} // namespace SeqVram
