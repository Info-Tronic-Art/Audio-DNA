#include <catch2/catch_test_macros.hpp>
#include "media/SeqVram.h"
#include <cstdint>
#include <deque>
#include <random>
#include <vector>

// s-rta-0928b seqvram: an image sequence plays through a BOUNDED window of recycled GL textures (src/media/SeqVram.h;
// plan .harmony/.reports/s-rta-0928b/plan-seqvram.md 4.4 + HARMONY ADOPTION H1, H2, H6, H10). These cases pin the
// trajectory distances per mode, the request / eviction plan (Belady order, never cur / lastShown, the floor), the
// slot table (reuse, respecify, Full, shrink, releaseAll) and the budget semantics (floors win), then run a small model
// of ImageSequence::getCurrentTexture for 2,000 steps with random seeks. Headless, no GL, no JUCE.

using namespace SeqVram;

namespace
{
constexpr size_t k1080 = 1920u * 1080u * 4u;   // 8,294,400
constexpr size_t k4k = 3840u * 2160u * 4u;     // 33,177,600

Transport tr(int n, int cur, bool forward, Mode mode)
{
    Transport t;
    t.n = n;
    t.cur = cur;
    t.forward = forward;
    t.mode = mode;
    t.outFrame = n - 1;
    return t;
}

std::vector<int> dist(const Transport& t)
{
    std::vector<int> d;
    distances(t, d);
    return d;
}

std::vector<uint8_t> flags(int n, std::initializer_list<int> on)
{
    std::vector<uint8_t> v(static_cast<size_t>(n), 0);
    for (int j : on)
        v[static_cast<size_t>(j)] = 1;
    return v;
}

std::vector<uint8_t> range(int n, int lo, int hi)   // [lo, hi]
{
    std::vector<uint8_t> v(static_cast<size_t>(n), 0);
    for (int j = lo; j <= hi; ++j)
        v[static_cast<size_t>(j)] = 1;
    return v;
}
} // namespace

TEST_CASE("(1) Loop distances wrap forward and in reverse", "[seq_vram][s-rta-0928b]")
{
    CHECK(dist(tr(6, 4, true, Mode::Loop)) == std::vector<int>{ 2, 3, 4, 5, 0, 1 });
    const auto r = dist(tr(6, 1, false, Mode::Loop));
    CHECK(r[1] == 0);
    CHECK(r[0] == 1);
    CHECK(r[5] == 2);
    CHECK(r[4] == 3);
    CHECK(r[2] == 5);
}

TEST_CASE("(2) PingPong distances reflect at both ends; a revisit of cur counts a step", "[seq_vram][s-rta-0928b]")
{
    // forward from 4 of 6: 5 (1), 4 again (2), 3 (3), 2 (4), 1 (5), 0 (6). (The plan body wrote dist[3] = 2 here,
    // against its own rule "a PingPong revisit of cur counts"; the trajectory is 4 5 4 3 2 1 0: advanceFrame reflects
    // the time at the end, so frame n-1 is followed by n-2.)
    const auto f = dist(tr(6, 4, true, Mode::PingPong));
    CHECK(f == std::vector<int>{ 6, 5, 4, 3, 0, 1 });
    const auto b = dist(tr(6, 1, false, Mode::PingPong));
    CHECK(b[0] == 1);
    CHECK(b[2] == 3);
    CHECK(b[5] == 6);
    CHECK(dist(tr(1, 0, true, Mode::PingPong)) == std::vector<int>{ 0 });
    CHECK(dist(tr(1, 0, false, Mode::Loop)) == std::vector<int>{ 0 });
}

TEST_CASE("(3) OneShot orders like Loop but never requests past its end", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(6, 4, true, Mode::OneShot);
    const auto d = dist(t);
    CHECK(d[0] == 2);   // a retrigger restarts at the in-point: 0 follows 5 in the eviction order
    const auto none = flags(6, {});
    auto p = plan(t, d, flags(6, { 4 }), none, none, 4, 8, 0, kMaxOutstanding);
    CHECK(p.request == std::vector<int>{ 5 });

    const auto r = tr(6, 1, false, Mode::OneShot);
    p = plan(r, dist(r), flags(6, { 1 }), none, none, 1, 8, 0, kMaxOutstanding);
    CHECK(p.request == std::vector<int>{ 0 });

    const auto e = tr(6, 5, true, Mode::OneShot);   // at the end: nothing ahead
    p = plan(e, dist(e), flags(6, { 5 }), none, none, 5, 8, 0, kMaxOutstanding);
    CHECK(p.request.empty());
}

TEST_CASE("(4) requests: cur first, then the next kLookAhead trajectory steps", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(10, 8, true, Mode::Loop);
    const auto d = dist(t);
    const auto none = flags(10, {});
    CHECK(plan(t, d, flags(10, { 8 }), none, none, 8, 8, 0, 4).request == std::vector<int>{ 9, 0, 1, 2 });
    CHECK(plan(t, d, flags(10, { 8 }), flags(10, { 9 }), none, 8, 8, 0, 4).request == std::vector<int>{ 0, 1, 2 });
    CHECK(plan(t, d, flags(10, { 8 }), flags(10, { 9 }), none, 8, 8, 0, 2).request == std::vector<int>{ 0, 1 });
    // a failed frame does not extend the look-ahead (it is one of the 4 steps)
    CHECK(plan(t, d, flags(10, { 8 }), flags(10, { 9 }), flags(10, { 0 }), 8, 8, 0, 4).request == std::vector<int>{ 1, 2 });
    // cur not resident: cur first (demand), within freeOutstanding
    CHECK(plan(t, d, none, none, none, -1, 8, 0, 4).request == std::vector<int>{ 8, 9, 0, 1 });
    CHECK(plan(t, d, none, none, none, -1, 8, 0, 1).request == std::vector<int>{ 8 });
    CHECK(plan(t, d, none, none, none, -1, 8, 0, 0).request.empty());
}

TEST_CASE("(5) eviction drops the frames shown farthest in the future, never cur or lastShown", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(300, 150, true, Mode::Loop);
    const auto none = flags(300, {});
    const auto res = range(300, 140, 153);
    const auto p = plan(t, dist(t), res, none, none, 149, 10, 1, 0);
    CHECK(p.evict == std::vector<int>{ 148, 147, 146, 145, 144 });
    CHECK(14 - static_cast<int>(p.evict.size()) + 1 == 10);
}

TEST_CASE("(6) the floor: cur and lastShown stay even over the allowance", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(20, 10, true, Mode::Loop);
    const auto none = flags(20, {});
    const auto p = plan(t, dist(t), flags(20, { 10, 9, 3 }), none, none, 9, 1, 0, 0);
    CHECK(p.evict == std::vector<int>{ 3 });
}

TEST_CASE("(7) PingPong near a bounce keeps the frames just behind (they are next)", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(40, 38, true, Mode::PingPong);
    const auto none = flags(40, {});
    const auto p = plan(t, dist(t), range(40, 30, 39), none, none, 37, 8, 0, 0);
    CHECK(p.evict == std::vector<int>{ 30, 31 });
}

TEST_CASE("(8) allowance: the free budget, never below the floor", "[seq_vram][s-rta-0928b]")
{
    const size_t floor = kMinWindowFrames * k1080;
    CHECK(allowance(0, floor) == kBudgetBytes);
    CHECK(allowance(kBudgetBytes - 1, floor) == floor);
    CHECK(allowance(kBudgetBytes + 12345, floor) == floor);
    CHECK(allowance(kBudgetBytes / 2, floor) == kBudgetBytes / 2);
    CHECK(allowanceFrames(kBudgetBytes, k1080) == 129);
    CHECK(allowanceFrames(kBudgetBytes, k4k) == 32);
    CHECK(allowanceFrames(kBudgetBytes, 0) == kMinWindowFrames);   // H5: size unknown -> the floor, never a guess
    CHECK(allowanceFrames(k1080, k1080) == kMinWindowFrames);
}

TEST_CASE("(8b) H10 floors win: settled total <= max(budget, sum of drawn floors) + one frame", "[seq_vram][s-rta-0928b]")
{
    // N drawn sequences, each wanting all of its frames; each frame every sequence is granted the free budget of the
    // RUNNING total (H3), shrinks at once to its allowance and grows by at most one upload.
    struct Seq { size_t fb; int n; int held; };
    for (const auto& scene : std::vector<std::vector<Seq>>{
             { { k1080, 300, 0 } },
             { { k1080, 120, 0 }, { k1080, 180, 0 } },
             { { k1080, 150, 0 }, { k1080, 150, 0 }, { k1080, 150, 0 } },
             { { k4k, 20, 0 }, { k4k, 20, 0 }, { k4k, 20, 0 }, { k4k, 20, 0 }, { k4k, 20, 0 } },
             { { k4k, 40, 0 }, { k1080, 300, 0 } } })
    {
        auto seqs = scene;
        size_t floors = 0, maxFrame = 0;
        for (const auto& s : seqs)
        {
            floors += kMinWindowFrames * s.fb;
            maxFrame = std::max(maxFrame, s.fb);
        }
        const size_t bound = std::max(kBudgetBytes, floors) + maxFrame;
        size_t worst = 0;
        for (int frame = 0; frame < 2000; ++frame)
        {
            size_t total = 0;
            for (const auto& s : seqs)
                total += static_cast<size_t>(s.held) * s.fb;
            for (auto& s : seqs)
            {
                const size_t mine = static_cast<size_t>(s.held) * s.fb;
                const int cap = allowanceFrames(allowance(total - mine, kMinWindowFrames * s.fb), s.fb);
                const int next = std::min({ s.n, cap, s.held + 1 });
                total = total - mine + static_cast<size_t>(next) * s.fb;
                s.held = next;
            }
            if (frame > 1000)
                worst = std::max(worst, total);
        }
        CHECK(worst <= bound);
        if (floors > kBudgetBytes)
            CHECK(worst > kBudgetBytes);   // seq_over_budget: the floors exceed the budget, the total says so
    }
}

TEST_CASE("(9) wanted: in a full window a result outside it is stale unless it is cur or lastShown", "[seq_vram][s-rta-0928b]")
{
    const auto t = tr(300, 150, true, Mode::Loop);
    const auto d = dist(t);
    CHECK(wanted(d, 151, 150, 149, 10, false));
    CHECK(wanted(d, 159, 150, 149, 10, false));
    CHECK_FALSE(wanted(d, 160, 150, 149, 10, false));
    CHECK_FALSE(wanted(d, 148, 150, 149, 10, false));
    CHECK(wanted(d, 149, 150, 149, 10, false));   // lastShown
    CHECK(wanted(d, 150, 150, 149, 10, false));   // cur
    CHECK_FALSE(wanted(d, 300, 150, 149, 10, true));
    CHECK_FALSE(wanted(d, -1, 150, 149, 10, true));
    // under the allowance a result is kept wherever it lies (it needs no eviction)
    CHECK(wanted(d, 148, 150, 149, 10, true));
    CHECK(wanted(d, 20, 150, 149, 10, true));
}

TEST_CASE("(9b) lap 1: the in-point frame decoded after the playhead moved on is kept for a retrigger", "[seq_vram][s-rta-0928b]")
{
    // 60 Hz render frames, a 30 fps Loop of 300 frames, allowance 129, decode latency 2 render frames, one upload per
    // frame (the UploadBudget at 1080p): frame 0 is requested at cur 0 and arrives at cur 1 -- 299 steps ahead. It must
    // be uploaded (the window has room), stay resident through lap 1 (Belady evicts the frames just passed), and be
    // resident when a retrigger seeks to it at cur 131 (probe-seq-vram v4).
    const int n = 300, cap = 129, lat = 2;
    std::vector<uint8_t> res(n, 0), req(n, 0), fail(n, 0);
    std::vector<int> d;
    Slots slots;
    std::deque<std::pair<int, int>> inflight;
    std::vector<int> ready;
    int lastShown = -1, out = 0, cur = 0;
    uint32_t tex = 1;
    for (int rf = 0; cur != 131 || rf % 2 != 0; ++rf)
    {
        const auto t = tr(n, cur, true, Mode::Loop);
        distances(t, d);
        while (!inflight.empty() && inflight.front().second <= rf)
        {
            ready.push_back(inflight.front().first);
            inflight.pop_front();
            --out;
        }
        int resident = 0;
        for (auto r : res)
            resident += r;
        int inc = 0;
        for (auto it = ready.begin(); it != ready.end();)
        {
            if (!wanted(d, *it, cur, lastShown, cap, resident + inc < cap))
            {
                req[static_cast<size_t>(*it)] = 0;
                it = ready.erase(it);
                continue;
            }
            ++inc;
            ++it;
        }
        const auto p = plan(t, d, res, req, fail, lastShown, cap, inc, kMaxOutstanding - out);
        for (int j : p.evict)
        {
            slots.release(j);
            res[static_cast<size_t>(j)] = 0;
            req[static_cast<size_t>(j)] = 0;
        }
        if (!ready.empty())
        {
            const auto a = slots.acquire(ready.front(), 1, 1, cap);
            REQUIRE(a.act != Slots::Act::Full);
            if (a.act == Slots::Act::Create)
                slots.bind(a.slot, tex++);
            res[static_cast<size_t>(ready.front())] = 1;
            ready.erase(ready.begin());
        }
        for (int j : p.request)
        {
            req[static_cast<size_t>(j)] = 1;
            inflight.emplace_back(j, rf + lat);
            ++out;
        }
        if (res[static_cast<size_t>(cur)] != 0)
            lastShown = cur;
        if (rf % 2 == 1)
            cur = (cur + 1) % n;
    }
    int resident = 0;
    for (auto r : res)
        resident += r;
    CHECK(resident == cap);   // the window is full: evictions have run
    for (int j = 0; j < 8; ++j)
        CHECK(res[static_cast<size_t>(j)] == 1);
}

TEST_CASE("(10) Slots: create, full, reuse, respecify, shrink, releaseAll, bytes", "[seq_vram][s-rta-0928b]")
{
    Slots s;
    auto a = s.acquire(0, 256, 256, 2);
    CHECK((a.slot == 0 && a.act == Slots::Act::Create));
    s.bind(a.slot, 7);
    a = s.acquire(1, 256, 256, 2);
    CHECK((a.slot == 1 && a.act == Slots::Act::Create));
    s.bind(a.slot, 9);
    a = s.acquire(2, 256, 256, 2);
    CHECK(a.act == Slots::Act::Full);
    CHECK(a.slot == -1);
    CHECK(s.allocatedBytes() == 2u * 256u * 256u * 4u);

    s.release(0);
    a = s.acquire(2, 256, 256, 2);
    CHECK((a.slot == 0 && a.act == Slots::Act::Reuse && s.texOf(0) == 7u));
    CHECK(s.slotOf(2) == 0);
    CHECK(s.slotOf(0) == -1);

    s.release(1);
    a = s.acquire(3, 1920, 1080, 2);
    CHECK((a.slot == 1 && a.act == Slots::Act::Respecify && s.texOf(1) == 9u));
    CHECK(s.allocatedBytes() == 256u * 256u * 4u + k1080);   // free ones included, at their size

    // a free slot of the SAME size is preferred over another free one
    s.release(2);
    s.release(3);
    a = s.acquire(4, 1920, 1080, 2);
    CHECK((a.slot == 1 && a.act == Slots::Act::Reuse));

    // shrink hands back FREE slots only
    CHECK(s.occupied() == 1);
    const auto gone = s.shrink(1);
    CHECK(gone == std::vector<uint32_t>{ 7 });
    CHECK(s.size() == 1);
    CHECK(s.slotOf(4) == 0);
    CHECK(s.texOf(0) == 9u);
    CHECK(s.shrink(0).empty());   // occupied stays
    CHECK(s.size() == 1);

    const auto all = s.releaseAll();
    CHECK(all == std::vector<uint32_t>{ 9 });
}

TEST_CASE("(10b) H1: Full changes nothing, and out-of-range slots are no-ops", "[seq_vram][s-rta-0928b]")
{
    // cap 2, both slots held by cur + lastShown, one incoming result: acquire returns Full
    Slots s;
    s.bind(s.acquire(10, 64, 64, 2).slot, 1);   // cur
    s.bind(s.acquire(9, 64, 64, 2).slot, 2);    // lastShown
    const auto a = s.acquire(11, 64, 64, 2);
    CHECK(a.act == Slots::Act::Full);
    CHECK(s.size() == 2);
    CHECK(s.occupied() == 2);
    CHECK(s.slotOf(11) == -1);
    CHECK(s.slotOf(10) == 0);
    CHECK(s.slotOf(9) == 1);
    s.bind(a.slot, 99);   // -1: no-op
    s.bind(7, 99);
    s.release(11);        // not mapped: no-op
    CHECK(s.texOf(-1) == 0u);
    CHECK(s.texOf(2) == 0u);
    CHECK(s.frameOf(-1) == -1);
    CHECK(s.frameOf(5) == -1);
    CHECK(s.texOf(0) == 1u);
    CHECK(s.texOf(1) == 2u);
    CHECK(s.allocatedBytes() == 2u * 64u * 64u * 4u);

    // the plan cannot evict either (the floor) -- the caller defers the upload
    const auto t = tr(20, 10, true, Mode::Loop);
    const auto none = flags(20, {});
    CHECK(plan(t, dist(t), flags(20, { 10, 9 }), none, none, 9, 2, 1, 0).evict.empty());
}

TEST_CASE("(10c) H2: releaseAll empties the table; the next acquire creates", "[seq_vram][s-rta-0928b]")
{
    Slots s;
    s.bind(s.acquire(0, 64, 64, 4).slot, 11);
    s.bind(s.acquire(1, 64, 64, 4).slot, 12);
    s.release(1);
    CHECK(s.releaseAll() == std::vector<uint32_t>{ 11, 12 });
    CHECK(s.size() == 0);
    CHECK(s.occupied() == 0);
    CHECK(s.allocatedBytes() == 0u);
    CHECK(s.slotOf(0) == -1);
    const auto a = s.acquire(0, 64, 64, 4);
    CHECK((a.slot == 0 && a.act == Slots::Act::Create));
    CHECK(s.texOf(0) == 0u);
}

TEST_CASE("(12) H6: an active out point wraps the trajectory to the in point", "[seq_vram][s-rta-0928b]")
{
    Transport t = tr(300, 197, true, Mode::Loop);
    t.ranged = true;
    t.inFrame = 100;
    t.outFrame = 199;
    const auto d = dist(t);
    CHECK(d[198] == 1);
    CHECK(d[199] == 2);
    CHECK(d[100] == 3);
    CHECK(d[101] == 4);
    for (int j = 0; j < 300; ++j)
        if (j < 100 || j > 199)
            CHECK(d[static_cast<size_t>(j)] == kFar);
    const auto none = flags(300, {});
    const auto p = plan(t, d, flags(300, { 197 }), none, none, 197, 8, 0, 4);
    CHECK(p.request == std::vector<int>{ 198, 199, 100, 101 });
    // nothing outside [100, 199] is ever requested, from anywhere in the range
    for (int cur = 100; cur <= 199; ++cur)
    {
        Transport c = t;
        c.cur = cur;
        const auto pc = plan(c, dist(c), none, none, none, -1, 8, 0, 4);
        for (int j : pc.request)
            CHECK((j >= 100 && j <= 199));
    }
    // allowance 8: an outside-range resident frame goes before any in-range one
    auto res = range(300, 190, 197);
    res[20] = 1;
    const auto e = plan(t, d, res, none, none, 196, 8, 1, 0);
    REQUIRE(e.evict.size() == 2u);
    CHECK(e.evict[0] == 20);
    CHECK(e.evict[1] == 195);   // the in-range frame shown farthest ahead: just behind cur, after the wrap to 100
    // OneShot stops at the out point
    Transport o = t;
    o.mode = Mode::OneShot;
    CHECK(plan(o, dist(o), flags(300, { 197 }), none, none, 197, 8, 0, 4).request == std::vector<int>{ 198, 199 });
    // past the out point (an out point moved under the playhead): the next frame is the in point
    Transport past = t;
    past.cur = 250;
    CHECK(dist(past)[100] == 1);
}

TEST_CASE("(12b) H6: setRange maps normalized in/out points by syncMedia's rules", "[seq_vram][s-rta-0928b]")
{
    Transport t = tr(300, 0, true, Mode::Loop);
    setRange(t, 0.0f, 1.0f);
    CHECK_FALSE(t.ranged);
    CHECK(t.inFrame == 0);
    CHECK(t.outFrame == 299);
    setRange(t, 0.25f, 0.5f);
    CHECK(t.ranged);
    CHECK(t.inFrame == 74);    // seekTo: int(0.25 * 299)
    CHECK(t.outFrame == 149);  // the playhead of frame 150 = 150/300 >= 0.5: the jump
    setRange(t, 0.25f, 2.0f / 3.0f);
    CHECK(t.outFrame == 200);  // 2/3 as a float is 0.66666669 > 200/300: frame 200 is on screen before the jump
    setRange(t, 0.5f, 0.5f);
    CHECK(t.outFrame == t.inFrame);
    setRange(t, 0.25f, 1.0f);  // an in point alone: the natural wrap still goes to 0 (fmod), a retrigger to in
    CHECK_FALSE(t.ranged);
    CHECK(t.inFrame == 74);
    t.cur = 299;
    CHECK(dist(t)[0] == 1);
}

namespace
{
// A small model of ImageSequence::getCurrentTexture: the same order (drain -> stale drop -> plan -> evict -> shrink ->
// upload within Slots -> request -> return), a decoder that delivers a request after `lat` steps.
struct Model
{
    int n;
    Mode mode;
    bool forward = true;
    int cur = 0;
    int lastShown = -1;
    Slots slots;
    std::vector<uint8_t> resident, requested, failed;
    std::vector<int> d;
    std::deque<std::pair<int, int>> inflight;   // (frame, arrives at step)
    std::vector<int> ready;
    uint32_t nextTex = 1;
    int outstanding = 0;

    Model(int n_, Mode m) : n(n_), mode(m), resident(static_cast<size_t>(n_), 0), requested(static_cast<size_t>(n_), 0),
                            failed(static_cast<size_t>(n_), 0) {}

    void advance()
    {
        if (mode == Mode::Loop)
            cur = forward ? (cur + 1) % n : (cur + n - 1) % n;
        else
        {
            const Pos p = step(tr(n, cur, forward, mode), Pos{ cur, forward });
            cur = p.f;
            forward = p.fwd;
        }
    }

    // returns the requests made this step
    std::vector<int> frame(int stepNo, int cap, int lat)
    {
        const Transport t = tr(n, cur, forward, mode);
        distances(t, d);
        while (!inflight.empty() && inflight.front().second <= stepNo)
        {
            ready.push_back(inflight.front().first);
            inflight.pop_front();
            --outstanding;
        }
        int incoming = 0;
        int count = 0;
        for (auto r : resident)
            count += r;
        for (auto it = ready.begin(); it != ready.end();)
        {
            if (resident[static_cast<size_t>(*it)] != 0) { it = ready.erase(it); continue; }
            if (!wanted(d, *it, cur, lastShown, cap, count + incoming < cap)) { requested[static_cast<size_t>(*it)] = 0; it = ready.erase(it); continue; }
            ++incoming;
            ++it;
        }
        const auto p = plan(t, d, resident, requested, failed, lastShown, cap, incoming, kMaxOutstanding - outstanding);
        for (int j : p.evict)
        {
            REQUIRE(j != cur);
            REQUIRE(j != lastShown);
            slots.release(j);
            resident[static_cast<size_t>(j)] = 0;
            requested[static_cast<size_t>(j)] = 0;
        }
        if (slots.size() > cap)
            slots.shrink(cap);
        for (auto it = ready.begin(); it != ready.end();)
        {
            const auto a = slots.acquire(*it, 64, 64, cap);
            if (a.act == Slots::Act::Full) { ++it; continue; }
            if (a.act == Slots::Act::Create)
                slots.bind(a.slot, nextTex++);
            resident[static_cast<size_t>(*it)] = 1;
            it = ready.erase(it);
        }
        for (int j : p.request)
        {
            requested[static_cast<size_t>(j)] = 1;
            inflight.emplace_back(j, stepNo + lat);
            ++outstanding;
        }
        if (resident[static_cast<size_t>(cur)] != 0)
            lastShown = cur;
        return p.request;
    }
};
} // namespace

TEST_CASE("(11) invariants over 2,000 steps: Loop / PingPong / reverse, random seeks", "[seq_vram][s-rta-0928b]")
{
    std::mt19937 rng(928);
    for (const Mode mode : { Mode::Loop, Mode::PingPong })
        for (const bool fwd : { true, false })
            for (const int cap : { 8, 129 })
            {
                Model m(300, mode);
                m.forward = fwd;
                int seeks = 0, demands = 0;
                for (int s = 0; s < 2000; ++s)
                {
                    bool seek = false;
                    if (s % 97 == 96)
                    {
                        m.cur = static_cast<int>(rng() % 300u);
                        seek = true;
                        ++seeks;
                    }
                    const auto c = static_cast<size_t>(m.cur);
                    const bool needCur = m.resident[c] == 0 && m.requested[c] == 0 && m.failed[c] == 0
                                         && m.outstanding < kMaxOutstanding;
                    const int shownBefore = m.lastShown;
                    const auto req = m.frame(s, cap, 2);
                    // resident never > allowance
                    int res = 0;
                    for (auto r : m.resident)
                        res += r;
                    REQUIRE(res <= cap);
                    REQUIRE(m.slots.size() <= cap);
                    REQUIRE(m.slots.occupied() == res);
                    // resident[j] iff mapped to a slot
                    for (int j = 0; j < 300; ++j)
                        REQUIRE((m.resident[static_cast<size_t>(j)] != 0) == (m.slots.slotOf(j) >= 0));
                    // the frame on screen before this step (lastShown) and cur are never evicted
                    if (shownBefore >= 0)
                        REQUIRE(m.resident[static_cast<size_t>(shownBefore)] != 0);
                    // after a seek the request list starts with cur (when it needs a decode)
                    if (seek && needCur)
                    {
                        ++demands;
                        REQUIRE(!req.empty());
                        REQUIRE(req.front() == m.cur);
                    }
                    m.advance();
                }
                CHECK(seeks == 20);
                CHECK(demands > 0);
            }
}

TEST_CASE("(13) F1: a sequence is idle only after kIdleFrames frames without a draw", "[seq_vram][s-rta-0928b]")
{
    // the frame being started is `serial`; missed = serial - 1 - lastDrawn
    CHECK(kIdleFrames == 60);
    CHECK_FALSE(isIdle(100, 100));                        // drawn this frame
    CHECK_FALSE(isIdle(100, 101));                        // drawn last frame
    CHECK_FALSE(isIdle(100, 104));                        // a Pitfall 53 hold: 3 frames skipped
    CHECK_FALSE(isIdle(100, 100 + 1 + 59));               // 59 frames missed
    CHECK(isIdle(100, 100 + 1 + 60));                     // 60 frames missed
    CHECK(isIdle(0, 1 + 60));                             // never drawn (serial 0), 60 frames in
    CHECK_FALSE(isIdle(0, 1 + 59));
    CHECK_FALSE(isIdle(200, 100));                        // a serial behind the last draw is never idle
}

TEST_CASE("(14) F2: shrink and releaseSome hand back at most maxDeletes textures a call", "[seq_vram][s-rta-0928b]")
{
    CHECK(kMaxDeletesPerFrame == 8);
    DeleteBudget b;
    CHECK(b.left == 8);
    b.spend(3);
    CHECK(b.left == 5);
    b.spend(10);
    CHECK(b.left == 0);
    b.reset();
    CHECK(b.left == 8);

    // an idle trim of a full 129-slot window (cur + lastShown stay): at most 8 deletes a frame, done in 16 frames
    Slots s;
    for (int f = 0; f < 129; ++f)
        s.bind(s.acquire(f, 1920, 1080, 129).slot, static_cast<uint32_t>(f + 1));
    for (int f = 0; f < 129; ++f)
        if (f != 64 && f != 63)
            s.release(f);
    int frames = 0;
    while (s.size() > 2)
    {
        DeleteBudget frame;
        const auto gone = s.shrink(2, frame.left);
        frame.spend(gone.size());
        REQUIRE(gone.size() <= static_cast<size_t>(kMaxDeletesPerFrame));
        REQUIRE(!gone.empty());
        ++frames;
        // the slots not deleted yet stay allocated (free, counted, reusable)
        CHECK(s.allocatedBytes() == static_cast<size_t>(s.size()) * k1080);
    }
    CHECK(frames == 16);   // 127 spare slots / 8 a frame
    CHECK(s.slotOf(64) >= 0);
    CHECK(s.slotOf(63) >= 0);
    CHECK(s.shrink(2, 0).empty());

    // a budget of 0 deletes nothing; a free slot left over is reused, never re-created
    Slots r;
    for (int f = 0; f < 4; ++f)
        r.bind(r.acquire(f, 64, 64, 4).slot, static_cast<uint32_t>(10 + f));
    r.release(3);
    CHECK(r.shrink(1, 0).empty());
    CHECK(r.size() == 4);
    CHECK(r.acquire(7, 64, 64, 1).act == Slots::Act::Reuse);

    // the retire drain: occupied or not, at most maxDeletes a call; empty at the end, and the next acquire creates
    CHECK(r.releaseSome(3) == std::vector<uint32_t>{ 13, 12, 11 });
    CHECK(r.size() == 1);
    CHECK(r.releaseSome(0).empty());
    CHECK(r.size() == 1);
    CHECK(r.releaseSome(8) == std::vector<uint32_t>{ 10 });
    CHECK(r.size() == 0);
    CHECK(r.allocatedBytes() == 0u);
    CHECK(r.acquire(0, 64, 64, 4).act == Slots::Act::Create);
}
