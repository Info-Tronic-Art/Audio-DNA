#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

// s-rta-0929 vupload P1: the per-render-frame VIDEO upload budget (count-based; mirrors ImageTexCache::UploadBudget's
// frame-top reset, never shared with it: an image load must not starve a video, plan-vupload.md R-1). Owned by the
// Renderer, begun at the frame top, consulted by every drawn player's uploadToTexture BEFORE its pick (VideoRing::peek):
// a player refused here HOLDS its shown frame (never pending, never late) and asks again next frame. Pure: no JUCE, no
// GL, no FFmpeg -- tests/test_video_upload_budget.cpp.
//
// The bound (adoption VU9): on every render frame, uploads <= cap + (requesters at their defer bound, force-admitted).
// No requester waits more than maxDefer render frames, and maxDefer < K (render frames per content frame), so the budget
// never skips a content frame and no layer starves -- for any number of players. The cap adapts to the demand (the
// mean of NEW frames asking per render frame over the last kHistory frames, re-asks of a held frame excluded), so
// synchronized players spread over the frames instead of landing on one (4 x 30 fps on 120 Hz: 2 + 2, the measured
// cap-2 arm of diag-vfps).
namespace VideoUpload
{
constexpr int kFloor = 2;            // no demand history yet: 4 x 30 fps on 120 Hz spreads 2 + 2 (the measured cap-2 arm)
constexpr int kHistory = 16;         // >= 4 content-frame periods at 30 fps / 120 Hz
constexpr int kMaxDeferFrames = 2;   // a ready frame is held at most this many render frames (16.7 ms at 120 Hz)

// Render frames a ready frame may wait without the NEXT content frame becoming pickable first: floor(K) - 1, K = render
// frames per content frame (contentFrameSec / renderDt; contentFrameSec = the frame duration / |speed|). K <= 1 (content
// at or above the render rate): never deferred.
inline int maxDefer(double contentFrameSec, double renderDt)
{
    if (contentFrameSec <= 0.0 || renderDt <= 0.0)
        return 0;
    const int k = static_cast<int>(std::floor(contentFrameSec / renderDt + 1e-9));
    return std::clamp(k - 1, 0, kMaxDeferFrames);
}

// Exempt from the budget (plan R-5 + adoption VU8): a player that has never shown a frame (the first picture: C1 / C3
// wait on it), the first upload after a GL release (its picture must come back on the first frame), and the first frame
// of a new request generation (a seek / retrigger / Loop wrap: the retrigger-to-visible latency does not change).
inline bool exempt(bool everShown, bool textureCreated, bool newGeneration)
{
    return !everShown || !textureCreated || newGeneration;
}

struct Budget
{
    int cap = kFloor, used = 0, requests = 0;
    std::array<int, kHistory> hist{};
    int h = 0;

    // Frame top: the previous frame's demand into the history; cap = max(kFloor, ceil(mean demand) + 1).
    void beginFrame()
    {
        hist[static_cast<size_t>(h)] = requests;
        h = (h + 1) % kHistory;
        int sum = 0;
        for (int x : hist)
            sum += x;
        cap = std::max(kFloor, static_cast<int>(std::ceil(static_cast<double>(sum) / kHistory - 1e-9)) + 1);
        used = 0;
        requests = 0;
    }

    // One drawn player with a NEW frame ready, deferred `deferredFrames` render frames so far. True = upload now (it
    // counts against this frame's cap). False = hold the shown frame and ask again next frame. A first ask
    // (deferredFrames == 0) is demand; a re-ask of a held frame is not (it would inflate the cap by the deferrals).
    bool admit(int deferredFrames, int maxDeferFrames, bool isExempt = false)
    {
        if (deferredFrames == 0)
            ++requests;
        if (isExempt || used < cap || deferredFrames >= maxDeferFrames)
        {
            ++used;
            return true;
        }
        return false;
    }

    // A frame published between the player's admission check (peek: nothing new) and its pick is uploaded in the same
    // step: it is charged here -- demand and one upload -- so request / used accounting never misses an upload
    // (adoption VU10).
    void charge()
    {
        ++requests;
        ++used;
    }
};
} // namespace VideoUpload
