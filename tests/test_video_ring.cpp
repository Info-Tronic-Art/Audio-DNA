// s-rta-0928b video: src/media/VideoRing.h -- the lock-free frame ring between a VideoPlayer's decode thread and the GL
// thread, and the pure catch-up / idle / hold policy (plan-video.md 4.3 + HARMONY ADOPTION V1 / V2). Headless: no FFmpeg,
// no GL, no JUCE.
#include <catch2/catch_test_macros.hpp>

#include "media/VideoRing.h"

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

using VideoRing::Ring;
using VideoRing::SlotState;

namespace
{
constexpr double kFd = 1.0 / 30.0;

SlotState stateOf(const Ring<3>& r, int i)
{
    return static_cast<SlotState>(r.header(i).state.load());
}

int publishOne(Ring<3>& r, double pts, uint32_t gen, uint64_t seq)
{
    const int s = r.acquireWrite();
    REQUIRE(s >= 0);
    r.publish(s, pts, gen, seq);
    return s;
}
} // namespace

TEST_CASE("pick takes the newest frame at or before the clock, frees the older one, keeps the future", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    const int s0 = publishOne(r, 0.000, 1, 1);
    const int s1 = publishOne(r, 1 * kFd, 1, 2);
    const int s2 = publishOne(r, 2 * kFd, 1, 3);
    const auto p = r.pick(0.040, 1, 0.5 * kFd);
    CHECK(p.slot == s1);
    CHECK(p.seq == 2);
    CHECK(stateOf(r, s1) == SlotState::Reading);
    CHECK(stateOf(r, s0) == SlotState::Free);
    CHECK(p.skipped == 1);
    CHECK(stateOf(r, s2) == SlotState::Ready);
    CHECK(p.readyAhead == 1);
}

TEST_CASE("pick frees every frame of another request generation and picks none of them", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    publishOne(r, 0.0, 1, 1);
    publishOne(r, 1 * kFd, 1, 2);
    publishOne(r, 2 * kFd, 1, 3);
    const auto p = r.pick(100.0, 2, 0.5 * kFd);
    CHECK(p.slot == -1);
    CHECK(p.staleDropped == 3);
    CHECK(r.freeCount() == 3);
}

TEST_CASE("nothing at or before the clock: no pick, nothing freed", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    publishOne(r, 1.0, 1, 1);
    publishOne(r, 1.0 + kFd, 1, 2);
    publishOne(r, 1.0 + 2 * kFd, 1, 3);
    const auto p = r.pick(0.5, 1, 0.5 * kFd);
    CHECK(p.slot == -1);
    CHECK(p.readyAhead == 3);
    CHECK(r.readyCount() == 3);
}

TEST_CASE("a full ring has no slot to write until the reader releases one", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    publishOne(r, 0.0, 1, 1);
    publishOne(r, 1 * kFd, 1, 2);
    publishOne(r, 2 * kFd, 1, 3);
    CHECK(r.acquireWrite() == -1);
    const auto p = r.pick(2 * kFd, 1, 0.5 * kFd);   // picks the newest, skips the two older ones
    REQUIRE(p.slot >= 0);
    CHECK(p.skipped == 2);
    const int w = r.acquireWrite();                 // a skipped slot is free again
    CHECK(w >= 0);
    CHECK(w != p.slot);
    r.abandon(w);
    r.release(p.slot);
    CHECK(r.freeCount() == 3);
}

TEST_CASE("a slot being read is never handed to the writer and is untouched by a second pick", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    const int s0 = publishOne(r, 0.0, 1, 1);
    const auto p = r.pick(0.0, 1, 0.5 * kFd);
    REQUIRE(p.slot == s0);
    for (int k = 0; k < 2; ++k)
    {
        const int w = r.acquireWrite();
        REQUIRE(w >= 0);
        CHECK(w != s0);
        r.publish(w, (k + 1) * kFd, 1, static_cast<uint64_t>(k + 2));
    }
    CHECK(r.acquireWrite() == -1);                  // the Reading slot is not Free
    const auto p2 = r.pick(10.0, 2, 0.5 * kFd);     // another generation: frees the two Ready slots, not the Reading one
    CHECK(p2.staleDropped == 2);
    CHECK(stateOf(r, s0) == SlotState::Reading);
    const auto p3 = r.pick(10.0, 1, 0.5 * kFd);
    CHECK(p3.slot == -1);
    CHECK(stateOf(r, s0) == SlotState::Reading);
    r.release(s0);
    CHECK(stateOf(r, s0) == SlotState::Free);
}

TEST_CASE("the half-frame tolerance: a frame just inside clock + tol is picked, just outside is not", "[video_ring][s-rta-0928b]")
{
    const double clock = 1.0, tol = 0.5 * kFd;
    {
        Ring<3> r;
        publishOne(r, clock + tol - 1e-9, 1, 1);
        CHECK(r.pick(clock, 1, tol).slot >= 0);
    }
    {
        Ring<3> r;
        publishOne(r, clock + tol + 1e-9, 1, 1);
        CHECK(r.pick(clock, 1, tol).slot == -1);
    }
}

TEST_CASE("release / abandon are defined no-ops on the wrong state", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    r.release(0);                                   // Free
    CHECK(stateOf(r, 0) == SlotState::Free);
    const int s = publishOne(r, 0.0, 1, 1);
    r.release(s);                                   // Ready
    CHECK(stateOf(r, s) == SlotState::Ready);
    r.abandon(s);                                   // Ready: abandon is the writer's Writing -> Free only
    CHECK(stateOf(r, s) == SlotState::Ready);
    const int w = r.acquireWrite();
    REQUIRE(w >= 0);
    r.release(w);                                   // Writing
    CHECK(stateOf(r, w) == SlotState::Writing);
    r.abandon(w);
    CHECK(stateOf(r, w) == SlotState::Free);
    r.release(-1);
    r.release(3);                                   // out of range: no-op
}

TEST_CASE("decide: re-seek behind by > 0.1 s or ahead by > 2 s of the newest frame; never during a catch-up", "[video_ring][s-rta-0928b]")
{
    const VideoRing::Policy p;
    CHECK(VideoRing::decide(5.0 - 0.11, 5.0, true, p) == VideoRing::Step::Reseek);
    CHECK(VideoRing::decide(5.0 + 2.01, 5.0, true, p) == VideoRing::Step::Reseek);
    CHECK(VideoRing::decide(5.0 - 0.09, 5.0, true, p) == VideoRing::Step::Decode);
    CHECK(VideoRing::decide(5.0 + 1.99, 5.0, true, p) == VideoRing::Step::Decode);
    CHECK(VideoRing::decide(100.0, 5.0, false, p) == VideoRing::Step::Decode);
}

TEST_CASE("shouldPublish drops frames > 1.5 frames behind; NONREF only when enabled and > 10 frames out", "[video_ring][s-rta-0928b]")
{
    VideoRing::Policy p;
    CHECK_FALSE(VideoRing::shouldPublish(5.0 - 1.6 * kFd, 5.0, kFd, p));
    CHECK(VideoRing::shouldPublish(5.0 - 1.4 * kFd, 5.0, kFd, p));
    CHECK(VideoRing::shouldPublish(5.0 + 3 * kFd, 5.0, kFd, p));
    CHECK_FALSE(VideoRing::useSkipNonRef(0.0, 5.0, kFd, p));
    p.skipNonRefInCatchUp = true;
    CHECK(VideoRing::useSkipNonRef(5.0 - 11 * kFd, 5.0, kFd, p));
    CHECK_FALSE(VideoRing::useSkipNonRef(5.0 - 9 * kFd, 5.0, kFd, p));
}

TEST_CASE("judge: New / Pending (never shown) / Late (> 1.5 frames while playing) / Held", "[video_ring][s-rta-0928b]")
{
    using VideoRing::Shown;
    CHECK(VideoRing::judge(true, false, true, 1.0, -1.0, kFd) == Shown::New);
    CHECK(VideoRing::judge(false, false, true, 1.0, -1.0, kFd) == Shown::Pending);
    CHECK(VideoRing::judge(false, true, true, 1.0, 1.0 - 1.6 * kFd, kFd) == Shown::Late);
    CHECK(VideoRing::judge(false, true, true, 1.0, 1.0 - 1.4 * kFd, kFd) == Shown::Held);
    CHECK(VideoRing::judge(false, true, false, 1.0, 0.0, kFd) == Shown::Held);   // paused: never late
}

TEST_CASE("V1: a GL release (context loss) keeps 'shown before' -- a hold stays Held/Late, never Pending", "[video_ring][s-rta-0928b]")
{
    using VideoRing::Shown;
    VideoRing::ShownState s;
    CHECK(VideoRing::judge(false, s.everShown, true, 1.0, -1.0, kFd) == Shown::Pending);   // a fresh player
    s.onUpload(7);
    CHECK_FALSE(s.needsUpload(7));
    s.onReleaseGL();
    CHECK(s.needsUpload(7));                        // the next pick re-uploads into a new texture
    CHECK(VideoRing::judge(false, s.everShown, true, 1.0, 1.0 - 0.5 * kFd, kFd) == Shown::Held);
    CHECK(VideoRing::judge(false, s.everShown, true, 1.0, 0.5, kFd) == Shown::Late);
}

TEST_CASE("W3: a player whose first frame never decodes is FAILED -- no media, not pending", "[video_ring][s-rta-0928b]")
{
    using VideoRing::Shown;
    using VideoRing::firstFrameFailed;
    REQUIRE(VideoRing::kFirstFrameTimeoutMs == 2000);
    // The decode thread gave up (EOF or a decode error before any frame): FAILED at once -- no media, never Pending.
    CHECK(firstFrameFailed(false, true, 10'000, 10'001));
    CHECK(firstFrameFailed(false, true, -1, 10'000));
    CHECK(VideoRing::judge(false, false, true, 1.0, -1.0, kFd, firstFrameFailed(false, true, 10'000, 10'001))
          == Shown::Failed);
    // No verdict from the thread: Pending until kFirstFrameTimeoutMs after the first draw request, then FAILED.
    CHECK_FALSE(firstFrameFailed(false, false, 10'000, 11'999));
    CHECK(VideoRing::judge(false, false, true, 1.0, -1.0, kFd, firstFrameFailed(false, false, 10'000, 11'999))
          == Shown::Pending);
    CHECK(firstFrameFailed(false, false, 10'000, 12'000));
    CHECK(VideoRing::judge(false, false, false, 0.0, -1.0, kFd, firstFrameFailed(false, false, 10'000, 12'000))
          == Shown::Failed);   // paused too
    // Never drawn: no timeout runs (a player on an off-screen deck is not failed by the clock).
    CHECK_FALSE(firstFrameFailed(false, false, -1, 1'000'000));
    // Shown before: never FAILED -- a hold stays Held / Late, whatever the thread reports later.
    CHECK_FALSE(firstFrameFailed(true, true, 10'000, 99'000));
    CHECK(VideoRing::judge(false, true, true, 1.0, 1.0 - 0.5 * kFd, kFd, true) == Shown::Held);
    CHECK(VideoRing::judge(false, true, true, 1.0, 0.5, kFd, true) == Shown::Late);
    // A frame that lands after the verdict still shows.
    CHECK(VideoRing::judge(true, false, true, 1.0, -1.0, kFd, true) == Shown::New);
}

TEST_CASE("V2: the idle rule parks the decode thread 250 ms after the last draw, ring full or not", "[video_ring][s-rta-0928b]")
{
    const VideoRing::Policy p;
    Ring<3> r;
    publishOne(r, 0.0, 1, 1);
    publishOne(r, kFd, 1, 2);
    publishOne(r, 2 * kFd, 1, 3);
    REQUIRE(r.acquireWrite() == -1);                // the ring-full wait
    CHECK(VideoRing::idleStep(10'300, 10'000, p) == VideoRing::Idle::Park);   // last draw 300 ms ago -> park
    CHECK(VideoRing::idleStep(10'100, 10'000, p) == VideoRing::Idle::Run);    // 100 ms ago -> keep waiting for a slot
}

TEST_CASE("reverse play: a frame the clock moved away from is freed so the writer is never locked out", "[video_ring][s-rta-0928b]")
{
    Ring<3> r;
    publishOne(r, 5.0 + 1 * kFd, 1, 1);
    publishOne(r, 5.0 + 2 * kFd, 1, 2);
    publishOne(r, 5.0 + 3 * kFd, 1, 3);
    REQUIRE(r.acquireWrite() == -1);
    const double dropAhead = 4 * kFd;               // (ring slots + 1) frames: forward play never reaches it
    auto p = r.pick(5.0, 1, 0.5 * kFd, dropAhead);  // the clock at 5.0 (moving backwards): all three still near
    CHECK(p.slot == -1);
    CHECK(p.aheadDropped == 0);
    CHECK(p.readyAhead == 3);
    p = r.pick(5.0 - 3 * kFd, 1, 0.5 * kFd, dropAhead);   // the drop line: clock + tol + 4 fd = 5.0 + 1.5 fd
    CHECK(p.aheadDropped == 2);                     // 5.0 + 2 fd and + 3 fd are past it
    CHECK(p.readyAhead == 1);
    CHECK(r.acquireWrite() >= 0);
    // forward play never drops: three frames ahead of a picked one are within clock + tol + 4 fd
    Ring<3> f;
    publishOne(f, 1 * kFd, 1, 1);
    publishOne(f, 2 * kFd, 1, 2);
    publishOne(f, 3 * kFd, 1, 3);
    const auto q = f.pick(0.0, 1, 0.5 * kFd, dropAhead);
    CHECK(q.aheadDropped == 0);
    CHECK(q.readyAhead == 3);
    // decide() with VideoPlayer's look-ahead-aware "behind" ((slots + 1) frames): the writer's forward look-ahead
    // (newest up to clock + tol + 3 frames) never re-seeks; a clock more than 4 frames behind the newest (reverse) does.
    // The fixed 0.1 s would re-seek forward play (3.5 frames at 30 fps = 0.117 s).
    VideoRing::Policy pol;
    const VideoRing::Policy fixed;
    pol.reseekBehindSec = 4 * kFd;
    CHECK(VideoRing::decide(0.0, 3.5 * kFd, true, pol) == VideoRing::Step::Decode);
    CHECK(VideoRing::decide(0.0, 4.5 * kFd, true, pol) == VideoRing::Step::Reseek);
    CHECK(VideoRing::decide(0.0, 3.5 * kFd, true, fixed) == VideoRing::Step::Reseek);
}

TEST_CASE("stress: one writer thread, one reader thread, 200,000 frames, generation bumps; no torn slot, no double owner", "[video_ring][s-rta-0928b]")
{
    constexpr int kFrames = 200000;
    Ring<3> r;
    struct Payload { uint64_t w[8]; };
    std::vector<Payload> bytes(3);
    std::atomic<int> owner[3];
    for (auto& o : owner) o.store(0);
    std::atomic<uint32_t> gen{ 1 };
    std::atomic<bool> writerDone{ false };
    std::atomic<int> ownerViolations{ 0 }, tornReads{ 0 }, staleGenPicks{ 0 }, orderViolations{ 0 };

    std::thread writer([&] {
        for (int k = 0; k < kFrames; ++k)
        {
            const uint32_t g = gen.load(std::memory_order_acquire);
            int s;
            while ((s = r.acquireWrite()) < 0)
                std::this_thread::yield();
            int expect = 0;
            if (!owner[s].compare_exchange_strong(expect, 1)) ++ownerViolations;
            const uint64_t seq = static_cast<uint64_t>(k) + 1;
            for (auto& w : bytes[static_cast<size_t>(s)].w) w = seq;
            owner[s].store(0);
            r.publish(s, k / 30.0, g, seq);
        }
        writerDone.store(true, std::memory_order_release);
    });

    int picks = 0;
    double lastPts = -1.0;
    uint32_t lastGen = gen.load();
    while (true)
    {
        const uint32_t g = gen.load(std::memory_order_acquire);
        if (g != lastGen) { lastGen = g; lastPts = -1.0; }
        const auto p = r.pick(1e12, g, 0.5 * kFd);
        if (p.slot >= 0)
        {
            int expect = 0;
            if (!owner[p.slot].compare_exchange_strong(expect, 2)) ++ownerViolations;
            const auto& h = r.header(p.slot);
            if (h.gen != g) ++staleGenPicks;
            for (auto w : bytes[static_cast<size_t>(p.slot)].w)
                if (w != h.seq) { ++tornReads; break; }
            if (p.pts < lastPts) ++orderViolations;
            lastPts = p.pts;
            if (++picks % 7 == 0) std::this_thread::yield();   // hold the slot a little: the writer runs around it
            owner[p.slot].store(0);
            r.release(p.slot);
            if (picks % 997 == 0) gen.fetch_add(1, std::memory_order_acq_rel);   // a reader-side "seek"
        }
        else if (writerDone.load(std::memory_order_acquire) && r.readyCount() == 0)
            break;
        else
            std::this_thread::yield();
    }
    writer.join();
    r.pick(1e12, gen.load() + 1, 0.0);              // free whatever the last generation left Ready
    CHECK(picks > 0);
    CHECK(ownerViolations.load() == 0);
    CHECK(tornReads.load() == 0);
    CHECK(staleGenPicks.load() == 0);
    CHECK(orderViolations.load() == 0);
    CHECK(r.freeCount() == 3);
}

// ---- s-rta-0929 vupload (plan-vupload.md 4.2 + HARMONY ADOPTION VU2 / VU10) ----

TEST_CASE("peek: the slot the following pick takes, with no state change", "[video_ring][s-rta-0929]")
{
    Ring<3> r;
    const int s0 = publishOne(r, 0.000, 1, 1);
    const int s1 = publishOne(r, 1 * kFd, 1, 2);
    const int s2 = publishOne(r, 2 * kFd, 1, 3);
    const auto k = r.peek(0.040, 1, 0.5 * kFd);
    CHECK(k.slot == s1);
    CHECK(k.seq == 2);
    CHECK(stateOf(r, s0) == SlotState::Ready);   // nothing freed, nothing taken
    CHECK(stateOf(r, s1) == SlotState::Ready);
    CHECK(stateOf(r, s2) == SlotState::Ready);
    const auto p = r.pick(0.040, 1, 0.5 * kFd);
    CHECK(p.slot == k.slot);
    CHECK(p.seq == k.seq);
    CHECK(p.pts == k.pts);
}

TEST_CASE("peek: stale-generation frames are never chosen (and not freed); nothing qualifying -> none", "[video_ring][s-rta-0929]")
{
    Ring<3> r;
    publishOne(r, 0.0, 1, 1);
    const int s1 = publishOne(r, 1 * kFd, 2, 2);
    const auto k = r.peek(100.0, 2, 0.5 * kFd);
    CHECK(k.slot == s1);
    CHECK(r.readyCount() == 2);                  // the stale one is still Ready (pick frees it)
    const auto p = r.pick(100.0, 2, 0.5 * kFd);
    CHECK(p.slot == s1);
    CHECK(p.staleDropped == 1);

    Ring<3> e;
    publishOne(e, 5.0, 1, 1);
    CHECK(e.peek(0.0, 1, 0.5 * kFd).slot == -1);   // only a future frame
    CHECK(e.pick(0.0, 1, 0.5 * kFd).slot == -1);
    Ring<3> z;
    CHECK(z.peek(0.0, 1, 0.5 * kFd).slot == -1);   // empty
}

TEST_CASE("dropReady: every Ready slot -> Free; Reading and Writing untouched", "[video_ring][s-rta-0929]")
{
    Ring<3> r;
    publishOne(r, 0.0, 1, 1);
    publishOne(r, 1 * kFd, 1, 2);
    const auto p = r.pick(0.0, 1, 0.5 * kFd);      // slot of pts 0 -> Reading, pts 1 fd stays Ready
    REQUIRE(p.slot >= 0);
    const int w = r.acquireWrite();                 // the third -> Writing
    REQUIRE(w >= 0);
    CHECK(r.dropReady() == 1);
    CHECK(stateOf(r, p.slot) == SlotState::Reading);
    CHECK(stateOf(r, w) == SlotState::Writing);
    CHECK(r.readyCount() == 0);
    CHECK(r.dropReady() == 0);
}

TEST_CASE("Retire (a): without fences the previously shown slot is released when a newer frame is shown", "[video_ring][s-rta-0929]")
{
    VideoRing::Retire<3> t;
    CHECK(t.shown(0, false) == -1);
    CHECK(t.held == 0);
    CHECK(t.shown(1, false) == 0);
    CHECK(t.held == 1);
    CHECK(t.shown(1, false) == -1);                 // the same slot again (a re-upload): nothing to release
}

TEST_CASE("Retire (b): a fenced slot waits for its fence; the held slot is never released by a signal", "[video_ring][s-rta-0929]")
{
    VideoRing::Retire<3> t;
    t.shown(0, true);
    CHECK(t.shown(1, true) == -1);                  // 0 is still fenced: not yet
    CHECK(t.signaled(0));                           // 0's fence signals -> release now
    CHECK_FALSE(t.signaled(1));                     // 1 is the held one: stays (its fence is done)
    CHECK(t.held == 1);
    CHECK(t.shown(2, true) == 1);                   // 1's fence already signaled: released at once
    t.shown(0, true);                               // 2 fenced when 0 is shown
    CHECK(t.signaled(2));
}

TEST_CASE("Retire (c)(e): a context loss releases the fenced slots but never the held one; no fence survives", "[video_ring][s-rta-0929]")
{
    VideoRing::Retire<3> t;
    t.shown(0, true);
    t.shown(1, true);                               // held 1, 0 fenced
    std::array<int, 3> out{ -1, -1, -1 };
    const int n = t.contextLost(out);
    REQUIRE(n == 1);
    CHECK(out[0] == 0);
    CHECK(t.held == 1);                             // the next context's picture
    CHECK_FALSE(t.fenced[0]);
    CHECK_FALSE(t.fenced[1]);
    CHECK_FALSE(t.fenced[2]);
    std::array<int, 3> again{ -1, -1, -1 };
    CHECK(t.contextLost(again) == 0);               // nothing left to release; held still 1
    CHECK(t.held == 1);
}

TEST_CASE("Retire (d): a signal on an unfenced slot releases nothing", "[video_ring][s-rta-0929]")
{
    VideoRing::Retire<3> t;
    CHECK_FALSE(t.signaled(2));
    t.shown(0, false);
    CHECK_FALSE(t.signaled(0));
    CHECK_FALSE(t.signaled(-1));
    CHECK_FALSE(t.signaled(3));
}

TEST_CASE("VU10 stress: a writer publishing between peek() and pick() -- pick never returns an older frame or none, and every upload was admitted or charged exactly once",
          "[video_ring][s-rta-0929]")
{
    // The reader mirrors VideoPlayer::uploadToTexture's order: peek -> (admission on the peeked frame) -> pick -> upload
    // -> the previously shown slot released when a newer one is shown. A frame the writer published
    // after the peek is what pick may return instead: it must be NEWER than the peeked one (never older, never none),
    // and a pick when the peek saw nothing is "charged" -- so admitted + charged == uploads.
    constexpr int kFrames = 100000;
    Ring<3> r;
    std::atomic<bool> writerDone{ false };
    std::atomic<double> clock{ 0.0 };
    std::thread writer([&] {
        for (int k = 0; k < kFrames; ++k)
        {
            int s;
            while ((s = r.acquireWrite()) < 0)
            {
                if (writerDone.load(std::memory_order_relaxed)) return;
                std::this_thread::yield();
            }
            r.publish(s, k * kFd, 1, static_cast<uint64_t>(k) + 1);
            clock.store(k * kFd, std::memory_order_release);   // the clock follows the writer (forward play)
        }
        writerDone.store(true, std::memory_order_release);
    });
    int shownSlot = -1;                                   // the shown frame's slot: released when a newer one is shown
    uint64_t lastSeq = 0;
    long admitted = 0, charged = 0, uploads = 0, olderThanPeek = 0, noneAfterPeek = 0, orderViolations = 0;
    while (!(writerDone.load(std::memory_order_acquire) && r.readyCount() == 0))
    {
        const double c = clock.load(std::memory_order_acquire);
        const auto k = r.peek(c, 1, 0.5 * kFd);
        const bool asked = k.slot >= 0 && k.seq != lastSeq;
        if (asked) ++admitted;
        std::this_thread::yield();                       // widen the window between peek and pick
        const auto p = r.pick(c + (asked ? 0.0 : kFd), 1, 0.5 * kFd);   // (a later clock read, as the next frame would)
        if (p.slot < 0)
        {
            if (k.slot >= 0) ++noneAfterPeek;
            continue;
        }
        if (k.slot >= 0 && p.seq < k.seq) ++olderThanPeek;
        if (p.seq != lastSeq)
        {
            if (!asked) ++charged;
            ++uploads;
            if (p.seq < lastSeq) ++orderViolations;
            lastSeq = p.seq;
        }
        if (shownSlot >= 0 && shownSlot != p.slot)
            r.release(shownSlot);
        shownSlot = p.slot;
    }
    writerDone.store(true);
    writer.join();
    CHECK(uploads > 0);
    CHECK(olderThanPeek == 0);
    CHECK(noneAfterPeek == 0);
    CHECK(orderViolations == 0);
    CHECK(admitted + charged == uploads);
}
