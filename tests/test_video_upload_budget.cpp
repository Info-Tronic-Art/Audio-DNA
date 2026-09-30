// s-rta-0929 vupload P1: src/media/VideoUploadBudget.h -- the per-render-frame video upload budget (plan-vupload.md 4.1 /
// 4.8 + HARMONY ADOPTION VU8 / VU9). Pure: no GL, no JUCE, no FFmpeg. A small simulator drives it the way the Renderer
// and VideoPlayer::uploadToTexture do: a player's content frame n becomes pickable at render frame ceil(phase + n * K);
// every frame the player whose newest pickable frame is not yet uploaded asks admit(); a refused player holds and asks
// again next frame; an admitted one uploads its NEWEST pickable frame (an older one it never uploaded is a skipped frame).
#include <catch2/catch_test_macros.hpp>

#include "media/VideoUploadBudget.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

using VideoUpload::Budget;

namespace
{
struct Player
{
    double k = 4.0;          // render frames per content frame
    double phase = 0.0;      // render frame of content frame 0
    long lastUploaded = -1;  // content frame index
    int deferred = 0;
};

struct FrameResult
{
    int uploads = 0, cap = 0, atBound = 0, maxDeferred = 0, skipped = 0, requests = 0;
};

// One render frame t over every player, in visit order (the Renderer's map order: fixed).
FrameResult step(Budget& b, std::vector<Player>& ps, long t)
{
    FrameResult r;
    b.beginFrame();
    r.cap = b.cap;
    for (auto& p : ps)
    {
        if (t < std::ceil(p.phase))
            continue;
        const long newest = static_cast<long>(std::floor((static_cast<double>(t) - p.phase) / p.k + 1e-9));
        if (newest <= p.lastUploaded)
            continue;
        ++r.requests;
        const int md = VideoUpload::maxDefer(p.k, 1.0);
        const bool bound = p.deferred >= md;
        if (b.admit(p.deferred, md))
        {
            ++r.uploads;
            if (bound && r.uploads > b.cap)
                ++r.atBound;
            r.skipped += static_cast<int>(std::max(0L, newest - p.lastUploaded - 1)) * (p.lastUploaded >= 0 ? 1 : 0);
            p.lastUploaded = newest;
            p.deferred = 0;
        }
        else
        {
            ++p.deferred;
            r.maxDeferred = std::max(r.maxDeferred, p.deferred);
        }
    }
    return r;
}
} // namespace

TEST_CASE("P1 (1): 4 players on one render frame every 4th frame -- 2 + 2 uploads, none held > 1 frame, cap settles at 2",
          "[video_upload_budget][s-rta-0929]")
{
    Budget b;
    std::vector<Player> ps(4);
    long requestsTotal = 0, uploadsTotal = 0;
    int maxUploads = 0, maxDeferred = 0, lastCap = 0;
    for (long t = 0; t < 400; ++t)
    {
        const auto r = step(b, ps, t);
        maxUploads = std::max(maxUploads, r.uploads);
        maxDeferred = std::max(maxDeferred, r.maxDeferred);
        uploadsTotal += r.uploads;
        lastCap = r.cap;
        CHECK(r.skipped == 0);
    }
    for (const auto& p : ps)
        requestsTotal += p.lastUploaded + 1;
    CHECK(maxUploads <= 2);
    CHECK(maxDeferred <= 1);
    CHECK(lastCap == 2);
    CHECK(uploadsTotal == requestsTotal);   // every content frame uploaded exactly once
}

TEST_CASE("P1 (2): 8 players, K = 4 -- the cap reaches 3 within kHistory frames, no hold > 2 frames, nothing skipped",
          "[video_upload_budget][s-rta-0929]")
{
    Budget b;
    std::vector<Player> ps(8);
    int maxDeferred = 0, capAtHistory = 0, skipped = 0;
    for (long t = 0; t < 400; ++t)
    {
        const auto r = step(b, ps, t);
        maxDeferred = std::max(maxDeferred, r.maxDeferred);
        skipped += r.skipped;
        if (t == VideoUpload::kHistory)
            capAtHistory = r.cap;
    }
    CHECK(capAtHistory >= 3);
    CHECK(maxDeferred <= 2);
    CHECK(skipped == 0);
}

TEST_CASE("P1 (3)(4)(7): maxDefer = clamp(floor(K) - 1, 0, 2): 60 fps content -> 1, content at the render rate -> 0, a slow frame shrinks it",
          "[video_upload_budget][s-rta-0929]")
{
    const double r120 = 1.0 / 120.0;
    CHECK(VideoUpload::maxDefer(1.0 / 30.0, r120) == 2);   // K = 4
    CHECK(VideoUpload::maxDefer(1.0 / 60.0, r120) == 1);   // K = 2 (60 fps content)
    CHECK(VideoUpload::maxDefer(1.0 / 120.0, r120) == 0);  // K = 1
    CHECK(VideoUpload::maxDefer(1.0 / 144.0, r120) == 0);  // K < 1
    CHECK(VideoUpload::maxDefer(1.0 / 24.0, r120) == 2);   // K = 5: clamped at kMaxDeferFrames
    CHECK(VideoUpload::maxDefer(1.0 / 30.0, 0.025) == 0);  // a 25 ms render frame: K = 1.33 -> 0 (shrinks, never grows)
    CHECK(VideoUpload::maxDefer(1.0 / 30.0, 1.0 / 60.0) == 1);
    CHECK(VideoUpload::maxDefer(0.0, r120) == 0);
    CHECK(VideoUpload::maxDefer(1.0 / 30.0, 0.0) == 0);
    for (double dt = 0.001; dt < 0.3; dt += 0.0007)
        CHECK(VideoUpload::maxDefer(1.0 / 30.0, dt) <= VideoUpload::kMaxDeferFrames);
    // K <= 1: admit always, even over the cap
    Budget b;
    b.beginFrame();
    for (int i = 0; i < 10; ++i)
        CHECK(b.admit(0, 0));
}

TEST_CASE("P1 (5): a burst of 12 on one frame at cap 2 -- every one admitted within maxDefer + 1 frames",
          "[video_upload_budget][s-rta-0929]")
{
    Budget b;
    std::vector<Player> ps(12);   // K = 4, all on frame 0
    std::vector<long> admittedAt(12, -1);
    for (long t = 0; t < 4; ++t)
    {
        step(b, ps, t);
        for (size_t i = 0; i < ps.size(); ++i)
            if (admittedAt[i] < 0 && ps[i].lastUploaded == 0)
                admittedAt[i] = t;
    }
    for (long at : admittedAt)
    {
        CHECK(at >= 0);
        CHECK(at <= VideoUpload::maxDefer(4.0, 1.0));
    }
}

TEST_CASE("P1 (6) VU9: fuzz -- N = 4 / 8 / 16 synchronized (and random-phase) players, K in {2, 2.4, 4, 4.8, 5}: the stated bound",
          "[video_upload_budget][s-rta-0929]")
{
    // The bound (VideoUploadBudget.h): per render frame, uploads <= cap + (requesters force-admitted at their defer
    // bound); no requester held more than maxDefer frames; no content frame skipped. Printed: the most uploads on one
    // frame after the warm-up (2 x kHistory) for each shape -- the numbers docs/claude/rendering.md quotes.
    std::mt19937 rng(20260929);
    for (int n : { 4, 8, 16 })
        for (double k : { 2.0, 2.4, 4.0, 4.8, 5.0 })
            for (int sync = 0; sync < 2; ++sync)
            {
                Budget b;
                std::vector<Player> ps(static_cast<size_t>(n));
                std::uniform_real_distribution<double> ph(0.0, k);
                for (auto& p : ps)
                {
                    p.k = k;
                    p.phase = sync ? 0.0 : ph(rng);
                }
                const int md = VideoUpload::maxDefer(k, 1.0);
                int steadyMax = 0, steadyForced = 0;
                for (long t = 0; t < 2000; ++t)
                {
                    const auto r = step(b, ps, t);
                    INFO("N " << n << " K " << k << (sync ? " sync" : " random") << " frame " << t);
                    REQUIRE(r.uploads <= r.cap + r.atBound);
                    REQUIRE(r.maxDeferred <= md);
                    REQUIRE(r.skipped == 0);
                    if (t >= 2 * VideoUpload::kHistory)
                    {
                        steadyMax = std::max(steadyMax, r.uploads);
                        steadyForced = std::max(steadyForced, r.atBound);
                    }
                }
                // The 4-player column trigger at 30 fps on 120 Hz (the w1c scene): exactly the measured cap-2 arm.
                if (n == 4 && k == 4.0)
                    CHECK(steadyMax <= 2);
                // 8 synchronized players: the adapted cap absorbs the burst (no force-admit in steady state).
                if (n == 8 && k >= 4.0)
                    CHECK(steadyForced == 0);
                WARN("VU9 bound: N " << n << " K " << k << (sync ? " sync" : " random") << ": steady max uploads/frame "
                                     << steadyMax << ", force-admitted over the cap " << steadyForced);
            }
}

TEST_CASE("VU8: exempt -- never shown, first upload after a GL release, first frame of a new generation; exempt uploads count as used",
          "[video_upload_budget][s-rta-0929]")
{
    CHECK(VideoUpload::exempt(false, true, false));
    CHECK(VideoUpload::exempt(true, false, false));
    CHECK(VideoUpload::exempt(true, true, true));
    CHECK_FALSE(VideoUpload::exempt(true, true, false));

    Budget b;
    b.beginFrame();
    CHECK(b.admit(0, 2));
    CHECK(b.admit(0, 2));
    CHECK_FALSE(b.admit(0, 2));        // over the cap (2)
    CHECK(b.admit(0, 2, true));        // a retrigger's first frame: admitted over the cap
    CHECK(b.used == 3);
    CHECK_FALSE(b.admit(1, 2));        // a held re-ask is still refused
    CHECK(b.admit(2, 2));              // at its defer bound: force-admitted
    CHECK(b.requests == 4);            // the re-asks are not demand
}

TEST_CASE("VU10: charge() -- a frame that arrived between peek and pick is counted as demand and one upload",
          "[video_upload_budget][s-rta-0929]")
{
    Budget b;
    b.beginFrame();
    b.charge();
    CHECK(b.used == 1);
    CHECK(b.requests == 1);
    CHECK(b.admit(0, 2));
    CHECK_FALSE(b.admit(0, 2));
}
