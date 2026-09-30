#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>

// s-rta-0929b gopcache (.harmony/.reports/s-rta-0929b/plan-gopcache.md 3.3 + HARMONY ADOPTION GC6 / GC7 / GC9): the PURE
// policy of a VideoPlayer's decode-thread GOP cache -- the shared memory budget, the closed-form trajectory distances
// (farthest next use, never LRU: Pitfall 54's principle), the store / evict / prefetch decisions. No FFmpeg, no GL, no
// JUCE: tests/test_gop_cache.cpp drives every function. Frames are RELATIVE indices r in [0, n) of the clip's trajectory
// (the player maps pts <-> r); a "slot view" lists what each cache slot holds, an "index" maps r -> slot (-1 = absent).
namespace GopCache
{
constexpr int64_t kBudgetBytesMax = int64_t{ 2 } << 30;   // every player's cache together (GC7: min(this, RAM / 16))
constexpr int64_t kMemDivisor = 16;
constexpr int kMinFrames = 8;              // the floor per active cache (floors win, counted over_budget)
constexpr int kBehindFrames = 16;          // frames kept BEHIND the clock (a flip's cover; GC8 for forward Loop / OneShot)
constexpr int kMinRetainFrames = 16;       // PingPong forward retention floor (R-9)
constexpr double kRetainSafety = 2.0;      // PingPong forward retention: GOP x decode ms / frame ms x this
constexpr double kRetainBudgetFrac = 0.25; // Q2 (adopted): a player's forward retention <= 1/4 of the budget
constexpr int kFullDecodeFrames = 10;      // no NONREF skip within this many frames below a run's storage window (R-7)
constexpr int kDemandWindow = 16;          // a DEMAND run stores the frames just below its target (latency first: the
                                           // window below is a PREFETCH run's, in the idle steps)
constexpr int kDefaultGopFrames = 250;     // no container index
constexpr double kDecodeMsSeedPerMpix = 1.0;
constexpr int kFar = 1 << 30;

// GC7: the budget of every cache together on a machine with `memsize` bytes of RAM (0 = unknown -> the maximum).
inline int64_t budgetFor(uint64_t memsize)
{
    if (memsize == 0)
        return kBudgetBytesMax;
    return std::min<int64_t>(kBudgetBytesMax, static_cast<int64_t>(memsize / static_cast<uint64_t>(kMemDivisor)));
}

// Process-wide (sharedBudget(), GopCacheStore.h), used by the decode threads (bytes, active, the prefetch token) and the
// message thread's memory-pressure hook (pressure); ctests make their own. Lock-free: atomics only.
struct Budget
{
    std::atomic<int64_t> bytes{ 0 };         // every cache's frame bytes together
    std::atomic<int> active{ 0 };            // caches holding >= 1 frame
    std::atomic<int64_t> total{ kBudgetBytesMax };
    std::atomic<int> pressure{ 0 };          // GC7: 0 normal, 1 warn (half the budget), 2 critical (floors only)
    std::atomic<int> prefetchOwner{ 0 };     // GC9: the player running a PREFETCH run (0 = none)

    int64_t totalBytes() const
    {
        const int64_t t = total.load(std::memory_order_relaxed);
        const int p = pressure.load(std::memory_order_relaxed);
        return p >= 2 ? 0 : (p == 1 ? t / 2 : t);
    }
    // A cache's share: the budget over the caches holding a frame (floors win).
    int64_t capBytes() const { return totalBytes() / std::max(1, active.load(std::memory_order_relaxed)); }

    enum class Take : uint8_t { Ok, OkOverBudget, Refused };
    // n more bytes for a cache holding `mine`: allowed iff mine + n <= floorBytes (floors win -- OkOverBudget when the
    // total then exceeds the budget) OR (mine + n <= capBytes() AND bytes + n <= totalBytes()).
    Take tryTake(int64_t n, int64_t mine, int64_t floorBytes)
    {
        if (mine + n <= floorBytes)
        {
            const int64_t after = bytes.fetch_add(n, std::memory_order_relaxed) + n;
            return after > totalBytes() ? Take::OkOverBudget : Take::Ok;
        }
        if (mine + n > capBytes())
            return Take::Refused;
        int64_t cur = bytes.load(std::memory_order_relaxed);
        do
        {
            if (cur + n > totalBytes())
                return Take::Refused;
        } while (!bytes.compare_exchange_weak(cur, cur + n, std::memory_order_relaxed));
        return Take::Ok;
    }
    void give(int64_t n)
    {
        int64_t cur = bytes.load(std::memory_order_relaxed);
        while (!bytes.compare_exchange_weak(cur, std::max<int64_t>(0, cur - n), std::memory_order_relaxed)) {}
    }
    void activate() { active.fetch_add(1, std::memory_order_relaxed); }
    void deactivate()
    {
        int cur = active.load(std::memory_order_relaxed);
        while (!active.compare_exchange_weak(cur, std::max(0, cur - 1), std::memory_order_relaxed)) {}
    }
    // GC9: one PREFETCH run at a time across every player (DEMAND runs are never held back). id != 0.
    bool takePrefetch(int id)
    {
        int e = 0;
        return prefetchOwner.compare_exchange_strong(e, id, std::memory_order_acq_rel) || e == id;
    }
    void releasePrefetch(int id)
    {
        int e = id;
        prefetchOwner.compare_exchange_strong(e, 0, std::memory_order_acq_rel);
    }
};

enum class Mode : uint8_t { Loop, PingPong, OneShot };

// Steps until relative frame f is shown again, from cur moving `forward` (or not) along the clip of n frames; closed form:
//   forward Loop:     f >= cur ? f - cur : n - cur + f         reverse Loop:     f <= cur ? cur - f : cur + n - f
//   forward PingPong: f >= cur ? f - cur : 2(n-1) - cur - f    reverse PingPong: f <= cur ? cur - f : cur + f
//   forward OneShot:  f >= cur ? f - cur : kFar                reverse OneShot:  f <= cur ? cur - f : kFar
inline int distance(Mode m, bool forward, int n, int cur, int f)
{
    if (forward)
    {
        if (f >= cur)
            return f - cur;
        switch (m)
        {
            case Mode::Loop: return n - cur + f;
            case Mode::PingPong: return 2 * (n - 1) - cur - f;
            case Mode::OneShot: return kFar;
        }
        return kFar;
    }
    if (f <= cur)
        return cur - f;
    switch (m)
    {
        case Mode::Loop: return cur + n - f;
        case Mode::PingPong: return cur + f;
        case Mode::OneShot: return kFar;
    }
    return kFar;
}

enum class Pool : uint8_t { Future, Behind, None };

// Where a frame belongs now. REVERSE: Loop / OneShot -- the kBehindFrames frames just above cur are Behind (a manual flip's
// cover; key = f - cur), the rest Future with their trajectory distance (Loop: the frames at the top of the file are the
// wrap's future) or None (OneShot: above the cover, never shown again); PingPong -- everything Future (frames above cur
// come back after the bounce: cur + f). FORWARD (retention, GC8 / R-9): frames below cur within `retain` are Behind (key
// = cur - f), frames at or above cur Future (forward hits served from a reverse episode's frames), the rest None.
inline Pool poolOf(Mode m, bool forward, int n, int cur, int f, int retain = kBehindFrames)
{
    if (forward)
    {
        if (f >= cur)
            return Pool::Future;
        return (cur - f <= retain) ? Pool::Behind : Pool::None;
    }
    if (m == Mode::PingPong || f <= cur)
        return Pool::Future;
    if (f - cur <= kBehindFrames)
        return Pool::Behind;
    return distance(m, false, n, cur, f) >= kFar ? Pool::None : Pool::Future;
}

// The eviction key of f in its pool (smaller = keep): Future = the trajectory distance; Behind = how far behind the clock.
inline int keyOf(Mode m, bool forward, int n, int cur, int f, Pool p)
{
    if (p == Pool::Behind)
        return forward ? cur - f : f - cur;
    if (p == Pool::None)
        return kFar;
    return distance(m, forward, n, cur, f);
}

// The Behind pool's share of capSlots: reverse Loop / OneShot min(kBehindFrames, capSlots / 4) -- none under
// kBehindMinSlots (a cover of a few frames is no cover, and a tiny cache needs every slot for its window); reverse
// PingPong 0 (its passed frames are Future); forward: `retain` (PingPong R-9, Loop / OneShot GC8's kBehindFrames) capped
// at capSlots / 2.
constexpr int kBehindMinSlots = 32;
inline int behindCapFor(Mode m, bool forward, int capSlots, int retain)
{
    if (forward)
        return std::max(0, std::min(retain, capSlots / 2));
    if (m == Mode::PingPong || capSlots < kBehindMinSlots)
        return 0;
    return std::min(kBehindFrames, capSlots / 4);
}

// The store's per-slot policy view (refreshed by the player before a decision): the relative frame held (-1 = a free
// slot, memory kept), its pool and key.
struct Slot
{
    int frame = -1;
    int key = kFar;
    Pool pool = Pool::None;
};

enum class Keep : uint8_t { Store, Replace, Drop };
struct Verdict
{
    Keep keep = Keep::Drop;
    int slot = -1;   // Store: a free slot, or -1 = a NEW slot; Replace: the slot to overwrite
};

// f decoded (not resident): Store into a free slot, else over a None-pool resident, else a new slot while slots < capSlots
// -- if its pool has room (Future count < capSlots - behindCap, Behind count < behindCap); a full pool takes a None
// resident's slot first, else
// pool replaces its resident with the largest key if that key > key(f) (never protectA / protectB: the wanted frame and
// the one being served); else Drop. A frame of pool None is never stored. Also, when the cache is at capSlots and the
// OTHER pool is over its share, that pool's largest key gives way.
inline Verdict judgeStore(const std::vector<Slot>& slots, int capSlots, int behindCap, int f, int key, Pool pool,
                          int protectA, int protectB)
{
    Verdict v;
    if (pool == Pool::None || capSlots <= 0)
        return v;
    int nFuture = 0, nBehind = 0, freeSlot = -1, noneSlot = -1;
    for (int i = 0; i < static_cast<int>(slots.size()); ++i)
    {
        const auto& s = slots[static_cast<size_t>(i)];
        if (s.frame < 0)
        {
            if (freeSlot < 0)
                freeSlot = i;
            continue;
        }
        if (s.pool == Pool::Future)
            ++nFuture;
        else if (s.pool == Pool::Behind)
            ++nBehind;
        else if (noneSlot < 0 && s.frame != protectA && s.frame != protectB)
            noneSlot = i;
    }
    (void) f;
    const int poolCap = pool == Pool::Behind ? behindCap : capSlots - behindCap;
    const int inPool = pool == Pool::Behind ? nBehind : nFuture;
    auto largest = [&](Pool p) {
        int best = -1;
        for (int i = 0; i < static_cast<int>(slots.size()); ++i)
        {
            const auto& s = slots[static_cast<size_t>(i)];
            if (s.frame < 0 || s.pool != p || s.frame == protectA || s.frame == protectB)
                continue;
            if (best < 0 || s.key > slots[static_cast<size_t>(best)].key)
                best = i;
        }
        return best;
    };
    if (inPool < poolCap)
    {
        if (freeSlot >= 0)
            return { Keep::Store, freeSlot };
        if (noneSlot >= 0)
            return { Keep::Replace, noneSlot };
        if (static_cast<int>(slots.size()) < capSlots)
            return { Keep::Store, -1 };
        const Pool other = pool == Pool::Behind ? Pool::Future : Pool::Behind;
        const int otherCap = pool == Pool::Behind ? capSlots - behindCap : behindCap;
        const int inOther = pool == Pool::Behind ? nFuture : nBehind;
        if (inOther > otherCap)
        {
            const int b = largest(other);
            if (b >= 0)
                return { Keep::Replace, b };
        }
    }
    if (noneSlot >= 0)   // a frame of no use left over from another direction / lap goes before anything of value
        return { Keep::Replace, noneSlot };
    const int b = largest(pool);
    if (b >= 0 && slots[static_cast<size_t>(b)].key > key)
        return { Keep::Replace, b };
    return v;
}

// GC6: over its cap (another cache joined, memory pressure): the slot to FREE now -- a None-pool resident first, else the
// largest key over both pools; never protectA / protectB; -1 = nothing evictable.
inline int evictOne(const std::vector<Slot>& slots, int protectA, int protectB)
{
    int best = -1;
    for (int i = 0; i < static_cast<int>(slots.size()); ++i)
    {
        const auto& s = slots[static_cast<size_t>(i)];
        if (s.frame < 0 || s.frame == protectA || s.frame == protectB)
            continue;
        const int64_t k = s.pool == Pool::None ? int64_t{ kFar } * 2 : s.key;
        const int64_t bk = best < 0 ? -1
                                    : (slots[static_cast<size_t>(best)].pool == Pool::None ? int64_t{ kFar } * 2
                                                                                            : slots[static_cast<size_t>(best)].key);
        if (k > bk)
            best = i;
    }
    return best;
}

// The next frame of a REVERSE trajectory after r (Loop wraps 0 -> n-1; PingPong / OneShot end at 0: -1).
inline int nextReverse(Mode m, int n, int r)
{
    if (r > 0)
        return r - 1;
    return (m == Mode::Loop && n > 0) ? n - 1 : -1;
}

inline bool resident(const std::vector<int>& index, int r)
{
    return r >= 0 && r < static_cast<int>(index.size()) && index[static_cast<size_t>(r)] >= 0;
}

// Reverse: how many frames after `served` along the trajectory are resident and contiguous (at most n - 1).
inline int unservedAhead(const std::vector<int>& index, Mode m, int n, int served)
{
    int u = 0;
    for (int r = nextReverse(m, n, served); r >= 0 && u < n - 1 && resident(index, r); r = nextReverse(m, n, r))
        ++u;
    return u;
}

// When a PREFETCH run may start: fewer than this many unserved frames resident ahead. Half the slots at least, and enough
// to cover one GOP of decode at the clip's effective frame period (x 1.2), never all of them. (A run also needs its minimum
// window free -- planPrefetch's minWindow, the player passes half the Future share.)
inline int prefetchAt(int capSlots, int gopFrames, double decodeMs, double frameMsEff)
{
    const double need = frameMsEff > 0.0 ? std::ceil(gopFrames * decodeMs / frameMsEff * 1.2) : static_cast<double>(capSlots);
    return std::max(capSlots / 2, std::min(capSlots - 2, static_cast<int>(std::min(need, 1.0e9))));
}

enum class RunKind : uint8_t { None, Demand, Prefetch, Reposition };
struct Run
{
    RunKind kind = RunKind::None;
    int target = -1;       // the top of the storage window (relative); Demand: the frame to publish
    int seekFrom = -1;     // seek to the keyframe at or before this frame
    int windowLo = -1;     // store frames in [windowLo, target]
    bool seekPending = true;
    int decoded = 0;
};

// Future slots a run may fill: free / new slots within the Future share, plus Future residents farther than `d` (a store
// replaces them as the window fills) and None residents -- at most capSlots - behindCap.
inline int availableFor(const std::vector<Slot>& slots, int capSlots, int behindCap, int d)
{
    const int share = capSlots - behindCap;
    int nFuture = 0, farther = 0, nNone = 0, nUsed = 0;
    for (const auto& s : slots)
    {
        if (s.frame < 0)
            continue;
        ++nUsed;
        if (s.pool == Pool::Future)
        {
            ++nFuture;
            if (s.key > d)
                ++farther;
        }
        else if (s.pool == Pool::None)
            ++nNone;
    }
    (void) nUsed;
    const int room = std::max(0, share - nFuture);
    return std::max(0, std::min(share, room + farther + nNone));
}

// REVERSE: a PREFETCH run when fewer than `prefetchAtN` unserved frames are resident ahead of `served`, the trajectory
// continues (Loop wraps to n - 1 after frame 0; PingPong / OneShot plan nothing below 0; intra-only files never prefetch)
// and at least `minWindow` Future slots are available (a run costs a keyframe seek + the catch-up however few frames it
// stores: a 1-frame window would cost a GOP of decode per frame): target = the first non-resident frame after the resident
// run; windowLo = max(0, target - (available - 1)) (available as availableFor at the target's distance), seekFrom =
// windowLo -- ONE keyframe seek covers the whole next window.
inline Run planPrefetch(const std::vector<int>& index, const std::vector<Slot>& slots, Mode m, int n, int cur, int served,
                        int capSlots, int behindCap, int prefetchAtN, bool intraOnly, int minWindow = 1)
{
    Run r;
    if (intraOnly || n <= 1 || served < 0)
        return r;
    int u = 0, t = nextReverse(m, n, served);
    while (t >= 0 && u < n - 1 && resident(index, t))
    {
        ++u;
        t = nextReverse(m, n, t);
    }
    if (t < 0 || u >= n - 1 || u >= prefetchAtN)
        return r;
    const int avail = availableFor(slots, capSlots, behindCap, distance(m, false, n, cur, t));
    if (avail <= 0 || avail < minWindow)
        return r;
    r.kind = RunKind::Prefetch;
    r.target = t;
    r.windowLo = std::max(0, t - (avail - 1));
    // never below the resident frames under the target (the Loop wrap's window: the top of the file above a resident
    // start) -- the seek lands on the keyframe at or before the first frame the run actually stores
    for (int f = t - 1; f >= r.windowLo; --f)
        if (resident(index, f))
        {
            r.windowLo = f + 1;
            break;
        }
    r.seekFrom = r.windowLo;
    return r;
}

// PingPong forward retention size (R-9 + Q2): GOP x decode ms / frame ms x kRetainSafety, at least kMinRetainFrames, at most
// capFrames (the caller passes min(capSlots / 2, the budget's kRetainBudgetFrac in frames)).
inline int retainFrames(int gopFrames, double decodeMs, double frameMsEff, int capFrames)
{
    const double need = frameMsEff > 0.0 ? std::ceil(gopFrames * decodeMs / frameMsEff * kRetainSafety) : kMinRetainFrames;
    const int want = std::max(kMinRetainFrames, static_cast<int>(std::min(need, 1.0e9)));
    return std::max(0, std::min(want, capFrames));
}
} // namespace GopCache
