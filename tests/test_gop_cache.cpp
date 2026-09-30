// test_gop_cache -- s-rta-0929b gopcache (plan-gopcache.md 3.3 / 4.1 + HARMONY ADOPTION GC6 / GC7 / GC9): the PURE policy of
// the decode-thread GOP cache (src/media/GopCache.h). Headless: no FFmpeg, no GL, no JUCE. RED on main: the header does not
// exist. Cases: (1) the shared Budget (cap = budget / active, floors win, give never below 0, the memory-pressure levels,
// budgetFor = min(2 GiB, RAM / 16), the one-prefetch token); (2) every closed-form distance vs a brute-force trajectory
// walk; (3) poolOf; (4) judgeStore; (5) planPrefetch / unservedAhead; (6) the amortization property of the whole policy on a
// simulated decoder (a reverse Loop over a GOP-250 clip); (7) retainFrames / prefetchAt arithmetic; (8) evictOne (GC6).
#include <catch2/catch_test_macros.hpp>

#include "media/GopCache.h"

#include <atomic>
#include <map>
#include <thread>
#include <vector>

using namespace GopCache;

namespace
{
// The trajectory walk: from cur, one step at a time in the clip's motion (Loop wraps, PingPong bounces at the ends without
// repeating the end frame, OneShot stops); the number of steps until f is shown again, or kFar.
int walk(Mode m, bool forward, int n, int cur, int f)
{
    if (f == cur)
        return 0;
    int r = cur;
    bool fwd = forward;
    for (int steps = 1; steps <= 4 * n + 4; ++steps)
    {
        if (fwd)
        {
            if (r + 1 < n)
                ++r;
            else if (m == Mode::Loop)
                r = 0;
            else if (m == Mode::PingPong)
            {
                fwd = false;
                r = n > 1 ? r - 1 : r;
            }
            else
                return kFar;
        }
        else
        {
            if (r > 0)
                --r;
            else if (m == Mode::Loop)
                r = n - 1;
            else if (m == Mode::PingPong)
            {
                fwd = true;
                r = n > 1 ? r + 1 : r;
            }
            else
                return kFar;
        }
        if (r == f)
            return steps;
    }
    return kFar;
}

std::vector<Slot> slotsOf(std::initializer_list<Slot> l)
{
    return std::vector<Slot>(l);
}
} // namespace

TEST_CASE("Budget: cap = budget / active caches; the total and the cap both refuse; floors win (over budget flagged)",
          "[gopcache][s-rta-0929b]")
{
    Budget b;
    b.total.store(1000);
    CHECK(b.capBytes() == 1000);             // no active cache: the whole budget
    b.activate();
    b.activate();
    CHECK(b.capBytes() == 500);
    CHECK(b.tryTake(400, 0, 100) == Budget::Take::Ok);           // within the cap and the total
    CHECK(b.tryTake(200, 400, 100) == Budget::Take::Refused);    // over this cache's cap (600 > 500)
    CHECK(b.tryTake(100, 0, 50) == Budget::Take::Ok);            // a second cache: 100 <= 500, total 500
    b.bytes.store(950);
    CHECK(b.tryTake(100, 100, 50) == Budget::Take::Refused);     // the total would pass 1000
    CHECK(b.tryTake(40, 0, 50) == Budget::Take::Ok);             // within its floor: taken, the total now 990 -> not over
    CHECK(b.bytes.load() == 990);
    CHECK(b.tryTake(40, 40, 100) == Budget::Take::OkOverBudget); // floor 100 >= 80: taken; 1030 > 1000 -> flagged
    CHECK(b.bytes.load() == 1030);
    b.give(5000);
    CHECK(b.bytes.load() == 0);              // never below 0
    b.deactivate();
    b.deactivate();
    b.deactivate();
    CHECK(b.active.load() == 0);             // never below 0
}

TEST_CASE("Budget: GC7 -- min(2 GiB, RAM / 16); memory pressure halves it (warn) or leaves only the floors (critical)",
          "[gopcache][s-rta-0929b]")
{
    CHECK(budgetFor(uint64_t{ 32 } << 30) == int64_t{ 2 } << 30);
    CHECK(budgetFor(uint64_t{ 16 } << 30) == int64_t{ 1 } << 30);
    CHECK(budgetFor(uint64_t{ 8 } << 30) == int64_t{ 512 } << 20);
    CHECK(budgetFor(0) == kBudgetBytesMax);
    Budget b;
    b.total.store(1000);
    b.activate();
    b.pressure.store(1);
    CHECK(b.capBytes() == 500);
    CHECK(b.tryTake(600, 0, 10) == Budget::Take::Refused);
    b.pressure.store(2);
    CHECK(b.capBytes() == 0);
    CHECK(b.tryTake(20, 0, 10) == Budget::Take::Refused);        // over the floor, nothing else is allowed
    CHECK(b.tryTake(10, 0, 10) == Budget::Take::OkOverBudget);   // the floor still wins
}

TEST_CASE("Budget: GC9 -- one PREFETCH token across players; the holder may re-take it; release frees it", "[gopcache][s-rta-0929b]")
{
    Budget b;
    CHECK(b.takePrefetch(1));
    CHECK(b.takePrefetch(1));
    CHECK_FALSE(b.takePrefetch(2));
    b.releasePrefetch(2);                    // not the holder: no effect
    CHECK_FALSE(b.takePrefetch(2));
    b.releasePrefetch(1);
    CHECK(b.takePrefetch(2));
}

TEST_CASE("Budget: concurrent takers never exceed the total (CAS), give returns every byte", "[gopcache][s-rta-0929b]")
{
    Budget b;
    b.total.store(1'000'000);
    for (int i = 0; i < 4; ++i)
        b.activate();
    std::atomic<int64_t> taken{ 0 };
    std::vector<std::thread> ts;
    for (int t = 0; t < 4; ++t)
        ts.emplace_back([&] {
            int64_t mine = 0;
            for (int i = 0; i < 20000; ++i)
                if (b.tryTake(100, mine, 0) == Budget::Take::Ok)
                    mine += 100;
            taken += mine;
            b.give(mine);
        });
    for (auto& t : ts)
        t.join();
    CHECK(taken.load() <= 1'000'000);
    CHECK(taken.load() >= 900'000);          // each of 4 caps is 250,000: they fill
    CHECK(b.bytes.load() == 0);
}

TEST_CASE("distance: every closed form equals a brute-force trajectory walk (n in {1, 2, 7, 30}, all modes, both directions)",
          "[gopcache][s-rta-0929b]")
{
    int mismatches = 0;
    for (int n : { 1, 2, 7, 30 })
        for (Mode m : { Mode::Loop, Mode::PingPong, Mode::OneShot })
            for (bool fwd : { true, false })
                for (int cur = 0; cur < n; ++cur)
                    for (int f = 0; f < n; ++f)
                    {
                        const int d = distance(m, fwd, n, cur, f), w = walk(m, fwd, n, cur, f);
                        if (d != w)
                        {
                            ++mismatches;
                            UNSCOPED_INFO("n " << n << " mode " << int(m) << " fwd " << fwd << " cur " << cur << " f " << f
                                               << ": " << d << " vs walk " << w);
                        }
                    }
    CHECK(mismatches == 0);
}

TEST_CASE("poolOf: reverse Loop keeps the 16 frames above cur Behind, the wrap's frames Future; PingPong all Future; forward retains below",
          "[gopcache][s-rta-0929b]")
{
    const int n = 300, cur = 100;
    CHECK(poolOf(Mode::Loop, false, n, cur, 90) == Pool::Future);
    CHECK(poolOf(Mode::Loop, false, n, cur, 101) == Pool::Behind);
    CHECK(poolOf(Mode::Loop, false, n, cur, 116) == Pool::Behind);
    CHECK(poolOf(Mode::Loop, false, n, cur, 117) == Pool::Future);   // above the cover: the wrap brings it back
    CHECK(keyOf(Mode::Loop, false, n, cur, 117, Pool::Future) == cur + n - 117);
    CHECK(poolOf(Mode::OneShot, false, n, cur, 117) == Pool::None);  // OneShot never comes back up
    CHECK(poolOf(Mode::OneShot, false, n, cur, 110) == Pool::Behind);
    CHECK(poolOf(Mode::PingPong, false, n, cur, 250) == Pool::Future);
    CHECK(keyOf(Mode::PingPong, false, n, cur, 250, Pool::Future) == cur + 250);
    CHECK(poolOf(Mode::Loop, true, n, cur, 90) == Pool::Behind);      // forward: the last frames played (GC8 / R-9)
    CHECK(keyOf(Mode::Loop, true, n, cur, 90, Pool::Behind) == 10);
    CHECK(poolOf(Mode::Loop, true, n, cur, 83) == Pool::None);        // beyond the default retention of 16
    CHECK(poolOf(Mode::PingPong, true, n, cur, 60, 50) == Pool::Behind);
    CHECK(poolOf(Mode::Loop, true, n, cur, 120) == Pool::Future);     // a forward hit (from a reverse episode)
    CHECK(behindCapFor(Mode::Loop, false, 64, 0) == 16);
    CHECK(behindCapFor(Mode::Loop, false, 32, 0) == 8);
    CHECK(behindCapFor(Mode::PingPong, false, 64, 0) == 0);
    CHECK(behindCapFor(Mode::Loop, true, 64, 16) == 16);
    CHECK(behindCapFor(Mode::PingPong, true, 64, 50) == 32);          // capped at capSlots / 2
}

TEST_CASE("judgeStore: free slot / new slot / replace the farthest of the same pool; never the protected; pools never mix",
          "[gopcache][s-rta-0929b]")
{
    // a free slot is used first
    auto v = judgeStore(slotsOf({ { 5, 5, Pool::Future }, { -1, kFar, Pool::None } }), 4, 0, 7, 3, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Store);
    CHECK(v.slot == 1);
    // room: a new slot
    v = judgeStore(slotsOf({ { 5, 5, Pool::Future } }), 4, 0, 7, 3, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Store);
    CHECK(v.slot == -1);
    // full: replace the largest key when the new key is smaller
    std::vector<Slot> full = { { 1, 10, Pool::Future }, { 2, 40, Pool::Future }, { 3, 20, Pool::Future } };
    v = judgeStore(full, 3, 0, 9, 15, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Replace);
    CHECK(v.slot == 1);
    // ... drop when larger
    v = judgeStore(full, 3, 0, 9, 50, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Drop);
    // never the protected frames (frame 2 = the one being served): the next largest goes
    v = judgeStore(full, 3, 0, 9, 15, Pool::Future, 2, -1);
    CHECK(v.keep == Keep::Replace);
    CHECK(v.slot == 2);
    // a None resident gives way before anything else in a full cache
    std::vector<Slot> withNone = { { 1, 10, Pool::Future }, { 2, kFar, Pool::None }, { 3, 20, Pool::Future } };
    v = judgeStore(withNone, 3, 0, 9, 99, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Replace);
    CHECK(v.slot == 1);
    // a Behind frame never takes a Future slot: the Behind share (1) is full -> it replaces its own pool or drops
    std::vector<Slot> mixed = { { 1, 10, Pool::Future }, { 2, 3, Pool::Behind }, { 3, 20, Pool::Future } };
    v = judgeStore(mixed, 3, 1, 9, 1, Pool::Behind, -1, -1);
    CHECK(v.keep == Keep::Replace);
    CHECK(v.slot == 1);
    v = judgeStore(mixed, 3, 1, 9, 5, Pool::Behind, -1, -1);
    CHECK(v.keep == Keep::Drop);
    // ... and a Future frame never takes the Behind slot (its share 2 is full with 2 Future residents)
    v = judgeStore(mixed, 3, 1, 9, 50, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Drop);
    v = judgeStore(mixed, 3, 1, 9, 5, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Replace);
    CHECK(v.slot == 2);
    // growth stops at capSlots
    v = judgeStore(slotsOf({ { 1, 1, Pool::Future }, { 2, 2, Pool::Future } }), 2, 0, 9, 5, Pool::Future, -1, -1);
    CHECK(v.keep == Keep::Drop);
    // a frame of pool None is never stored
    v = judgeStore(slotsOf({}), 4, 0, 9, kFar, Pool::None, -1, -1);
    CHECK(v.keep == Keep::Drop);
}

TEST_CASE("evictOne (GC6): None first, then the largest key over both pools, never the protected", "[gopcache][s-rta-0929b]")
{
    CHECK(evictOne(slotsOf({ { 1, 10, Pool::Future }, { 2, 3, Pool::Behind }, { 3, kFar, Pool::None } }), -1, -1) == 2);
    CHECK(evictOne(slotsOf({ { 1, 10, Pool::Future }, { 2, 30, Pool::Behind }, { 3, 20, Pool::Future } }), -1, -1) == 1);
    CHECK(evictOne(slotsOf({ { 1, 10, Pool::Future }, { 2, 30, Pool::Behind }, { 3, 20, Pool::Future } }), 2, -1) == 2);
    CHECK(evictOne(slotsOf({ { 1, 10, Pool::Future } }), 1, -1) == -1);
    CHECK(evictOne(slotsOf({ { -1, kFar, Pool::None } }), -1, -1) == -1);
}

TEST_CASE("unservedAhead / planPrefetch: a run below the resident run; one seek covers the window; the Loop wrap; the ends",
          "[gopcache][s-rta-0929b]")
{
    const int n = 300;
    std::vector<int> index(n, -1);
    std::vector<Slot> slots;
    auto put = [&](int f, int key) {
        index[static_cast<size_t>(f)] = static_cast<int>(slots.size());
        slots.push_back({ f, key, Pool::Future });
    };
    for (int f = 90; f <= 99; ++f)   // served 100: frames 90..99 resident
        put(f, 100 - f);
    CHECK(unservedAhead(index, Mode::Loop, n, 100) == 10);
    CHECK(planPrefetch(index, slots, Mode::Loop, n, 100, 100, 64, 0, 10, false).kind == RunKind::None);   // 10 >= 10
    auto r = planPrefetch(index, slots, Mode::Loop, n, 100, 100, 64, 0, 32, false);
    REQUIRE(r.kind == RunKind::Prefetch);
    CHECK(r.target == 89);
    CHECK(r.windowLo == 89 - (64 - 10 - 1));   // 54 slots left in the Future share
    CHECK(r.seekFrom == r.windowLo);
    CHECK(planPrefetch(index, slots, Mode::Loop, n, 100, 100, 64, 0, 32, true).kind == RunKind::None);   // intra-only
    // the window never goes below 0
    std::vector<int> i2(n, -1);
    std::vector<Slot> s2;
    i2[5] = 0;
    s2.push_back({ 5, 1, Pool::Future });
    r = planPrefetch(i2, s2, Mode::Loop, n, 6, 6, 64, 0, 32, false);
    REQUIRE(r.kind == RunKind::Prefetch);
    CHECK(r.target == 4);
    CHECK(r.windowLo == 0);
    // Loop: after frame 0 the trajectory wraps -> the target is n - 1
    std::vector<int> i3(n, -1);
    std::vector<Slot> s3;
    for (int f = 0; f <= 4; ++f)
    {
        i3[static_cast<size_t>(f)] = static_cast<int>(s3.size());
        s3.push_back({ f, 5 - f, Pool::Future });
    }
    r = planPrefetch(i3, s3, Mode::Loop, n, 5, 5, 64, 0, 32, false);
    REQUIRE(r.kind == RunKind::Prefetch);
    CHECK(r.target == n - 1);
    // OneShot / PingPong: nothing below 0
    CHECK(planPrefetch(i3, s3, Mode::OneShot, n, 5, 5, 64, 0, 32, false).kind == RunKind::None);
    CHECK(planPrefetch(i3, s3, Mode::PingPong, n, 5, 5, 64, 0, 32, false).kind == RunKind::None);
    CHECK(unservedAhead(i3, Mode::PingPong, n, 5) == 5);
    // a cache full of nearer frames: nothing to gain
    std::vector<int> i4(n, -1);
    std::vector<Slot> s4;
    for (int f = 60; f <= 99; ++f)
    {
        i4[static_cast<size_t>(f)] = static_cast<int>(s4.size());
        s4.push_back({ f, 100 - f, Pool::Future });
    }
    CHECK(planPrefetch(i4, s4, Mode::Loop, n, 100, 100, 40, 0, 45, false).kind == RunKind::None);
    // the minimum window: 54 available -> a run with minWindow 32, none with minWindow 60
    CHECK(planPrefetch(index, slots, Mode::Loop, n, 100, 100, 64, 0, 32, false, 32).kind == RunKind::Prefetch);
    CHECK(planPrefetch(index, slots, Mode::Loop, n, 100, 100, 64, 0, 32, false, 60).kind == RunKind::None);
}

TEST_CASE("prefetchAt / retainFrames: the arithmetic at this plan's four (resolution, GOP) points", "[gopcache][s-rta-0929b]")
{
    // 1080p GOP 30 / 250 and 4K GOP 250 at 30 fps (frame 33.3 ms), 2 GiB / 1 player -> 690 (1080p) or 172 (4K) slots
    CHECK(prefetchAt(690, 30, 2.0, 33.3) == 345);
    CHECK(prefetchAt(690, 250, 2.0, 33.3) == 345);
    CHECK(prefetchAt(172, 250, 6.8, 33.3) == 86);      // one GOP of 4K decode covers 62 frames: half the slots (86) wins
    CHECK(prefetchAt(8, 250, 2.0, 33.3) == 6);         // the floor cache: never all of it
    CHECK(retainFrames(30, 2.0, 33.3, 345) == 16);     // 30 x 2 / 33.3 x 2 = 3.6 -> the floor 16
    CHECK(retainFrames(250, 2.0, 33.3, 345) == 31);    // ceil(30.03) = 31
    CHECK(retainFrames(250, 6.8, 33.3, 86) == 86);     // 102 > the cap
    CHECK(retainFrames(250, 2.0, 33.3, 10) == 10);     // a cap under the floor wins
}

namespace
{
// A simulated reverse Loop over a clip of n frames with ONE keyframe at 0 (GOP = n): the player's policy loop (demand when
// the wanted frame is missing, prefetch below the resident run, greedy store by judgeStore, serve one frame per step)
// driven by GopCache's pure functions. Returns decodes per shown frame for lap `lapWanted` (0-based).
double simulateReverse(int n, int capSlots, int laps, int lapWanted, int* residentAtEnd = nullptr)
{
    std::vector<int> index(static_cast<size_t>(n), -1);
    std::vector<Slot> slots;
    int served = -1;
    long decodes = 0;
    std::vector<long> lapDecodes(static_cast<size_t>(laps), 0);
    const int behindCap = behindCapFor(Mode::Loop, false, capSlots, 0);
    auto refresh = [&](int cur) {
        for (auto& s : slots)
            if (s.frame >= 0)
            {
                s.pool = poolOf(Mode::Loop, false, n, cur, s.frame);
                s.key = keyOf(Mode::Loop, false, n, cur, s.frame, s.pool);
            }
    };
    auto store = [&](int f, int cur, int protect) {
        if (index[static_cast<size_t>(f)] >= 0)
            return;
        const Pool p = poolOf(Mode::Loop, false, n, cur, f);
        const auto v = judgeStore(slots, capSlots, behindCap, f, keyOf(Mode::Loop, false, n, cur, f, p), p, protect, served);
        if (v.keep == Keep::Drop)
            return;
        int slot = v.slot;
        if (slot < 0)
        {
            slot = static_cast<int>(slots.size());
            slots.push_back({});
        }
        auto& s = slots[static_cast<size_t>(slot)];
        if (s.frame >= 0)
            index[static_cast<size_t>(s.frame)] = -1;
        s = { f, keyOf(Mode::Loop, false, n, cur, f, p), p };
        index[static_cast<size_t>(f)] = slot;
    };
    // one run: decode from keyframe 0 up to `target`, storing [windowLo, target]
    auto run = [&](int windowLo, int target, int cur, int lap) {
        for (int f = 0; f <= target; ++f)
        {
            ++decodes;
            ++lapDecodes[static_cast<size_t>(lap)];
            if (f >= windowLo)
            {
                refresh(cur);
                store(f, cur, target);
            }
        }
    };
    const int pAt = prefetchAt(capSlots, n, 2.0, 33.3);
    for (int lap = 0; lap < laps; ++lap)
        for (int k = 0; k < n; ++k)
        {
            const int want = n - 1 - k;   // the reverse clock's frame
            refresh(want);
            if (index[static_cast<size_t>(want)] < 0)
            {
                const int avail = availableFor(slots, capSlots, behindCap, 0);
                run(std::max(0, want - std::max(0, avail - 1)), want, want, lap);   // DEMAND
            }
            served = want;
            refresh(want);
            const auto pr = planPrefetch(index, slots, Mode::Loop, n, want, served, capSlots, behindCap, pAt, false,
                                         std::max(1, (capSlots - behindCap) / 2));
            if (pr.kind == RunKind::Prefetch)
                run(pr.windowLo, pr.target, want, lap);
        }
    if (residentAtEnd != nullptr)
    {
        *residentAtEnd = 0;
        for (int i : index)
            *residentAtEnd += i >= 0 ? 1 : 0;
    }
    return static_cast<double>(lapDecodes[static_cast<size_t>(lapWanted)]) / n;
}
} // namespace

// Deviation from plan 4.1 (6), recorded in the lane report: the plan's bound G / (2 floor(slots / 2)) + 1 counts every slot
// as a Future slot, but its own R-5 reserves the Behind share (min(16, slots / 4)) in a reverse Loop; a run window is half
// of the Future share, so the bound is G / (2 floor(share / 2)) + 1 with share = slots - behindCapFor(...).
TEST_CASE("amortization: a reverse Loop over a GOP-250 clip costs <= G / (2 x floor(share / 2)) + 1 decodes per shown frame; "
          "a clip that fits is decoded once, then served from the cache", "[gopcache][s-rta-0929b]")
{
    const int G = 250;
    for (int slots : { 16, 32, 64, 128 })
    {
        const double dpf = simulateReverse(G, slots, 2, 1);
        const int share = slots - behindCapFor(Mode::Loop, false, slots, 0);
        const double bound = static_cast<double>(G) / (2.0 * (share / 2)) + 1.0;
        CAPTURE(slots, dpf, bound);
        CHECK(dpf <= bound);
    }
    int resident = 0;
    CHECK(simulateReverse(G, 300, 2, 0, &resident) <= 1.0 + 1e-9);   // the first lap: one decode per frame
    CHECK(simulateReverse(G, 300, 2, 1) == 0.0);                     // the second: none -- the clip is resident
    CHECK(resident == G);
}
