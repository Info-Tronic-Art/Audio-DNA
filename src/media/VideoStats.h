#pragma once
#include <atomic>
#include <cstdint>

// s-rta-0928b video: counters for /api/state (relaxed atomics; "take*" = reset on read). Owned by Renderer, shared by
// every VideoPlayer through a pointer (nullptr = no stats, as in ctests). Pure: no JUCE, no FFmpeg.
struct VideoStats
{
    std::atomic<int>      players{ 0 }, threadsRunning{ 0 }, threadsAwake{ 0 }, pendingNow{ 0 };
    std::atomic<int64_t>  glDecodeCalls{ 0 }, uploads{ 0 }, framesDecoded{ 0 }, framesDropped{ 0 }, framesSkipped{ 0 },
                          seeks{ 0 }, lateFrames{ 0 }, pendingFrames{ 0 }, holdFrames{ 0 };
    std::atomic<int>      glMaxDecodesPerCall{ 0 };                           // reset on read
    std::atomic<float>    peakUploadMs{ 0.0f }, msgLockWaitMaxMs{ 0.0f };      // reset on read
    // s-rta-0929 vupload: uploads held back by the per-frame budget (VideoUploadBudget.h), drawn frames of a shown player
    // that returned texture 0 (the FX-only witness: must stay 0), ring slots purged while idle, blit fences that failed,
    // players that fell back from the IOSurface blit to a client upload; the budget's cap this frame (INFO) and the most
    // video uploads on one render frame (reset on read).
    std::atomic<int64_t>  uploadsDeferred{ 0 }, holdNoTexture{ 0 }, slotsPurged{ 0 }, fenceFailed{ 0 }, surfaceFallbacks{ 0 };
    std::atomic<int>      uploadCap{ 0 }, maxUploadsPerFrame{ 0 };
    // s-rta-0929b gopcache (plan-gopcache.md 3.8 + adoption GC4 / GC6): the decode threads' GOP caches -- bytes / frames /
    // caches holding a frame (gauges), the budget's per-cache cap (INFO), hits (a cached frame published), misses (demand
    // runs), runs (prefetch + reposition), run decodes, evictions, drops (decoded, not stored), stores taken over the budget
    // under the floor. GL thread: a reversing player showed a frame ABOVE the previous one in the same generation (must
    // stay 0), the effective direction changed; the longest wait between two uploads of one drawn, playing player and the
    // longest single writer step (both reset on read); uploads per player slot (a player takes slot n % kPlayerSlots at
    // setStats; the probe's per-player minimum).
    std::atomic<int64_t>  gopCacheBytes{ 0 }, gopCacheFrames{ 0 }, gopCacheCapBytes{ 0 }, gopCacheHits{ 0 },
                          gopCacheMisses{ 0 }, gopCacheRuns{ 0 }, gopCacheRunDecodes{ 0 }, gopCacheEvictions{ 0 },
                          gopCacheDrops{ 0 }, gopCacheOverBudget{ 0 }, reverseNonmonotonic{ 0 }, directionChanges{ 0 };
    std::atomic<int>      gopCacheActive{ 0 };
    // s-rta-1002b mkvidx-fix (R1): the decode threads' keyframe-model rebuilds (VideoPlayer::readKeyIndex after open(): the
    // demuxer's index changed). A long-GOP Matroska file rebuilds once per keyframe read; an intra-only stream never.
    std::atomic<int64_t>  keyIndexRebuilds{ 0 };
    std::atomic<float>    maxUploadGapMs{ 0.0f }, writerStepMaxMs{ 0.0f };   // reset on read
    static constexpr int  kPlayerSlots = 32;
    std::atomic<int>      nextPlayerSlot{ 0 };
    std::atomic<int64_t>  playerUploads[kPlayerSlots] = {};

    int   takeGlMaxDecodesPerCall() { return glMaxDecodesPerCall.exchange(0, std::memory_order_relaxed); }
    int   takeMaxUploadsPerFrame()  { return maxUploadsPerFrame.exchange(0, std::memory_order_relaxed); }
    float takePeakUploadMs()        { return peakUploadMs.exchange(0.0f, std::memory_order_relaxed); }
    float takeMsgLockWaitMaxMs()    { return msgLockWaitMaxMs.exchange(0.0f, std::memory_order_relaxed); }
    float takeMaxUploadGapMs()      { return maxUploadGapMs.exchange(0.0f, std::memory_order_relaxed); }
    float takeWriterStepMaxMs()     { return writerStepMaxMs.exchange(0.0f, std::memory_order_relaxed); }

    static void noteMax(std::atomic<float>& a, float v)
    {
        float c = a.load(std::memory_order_relaxed);
        while (v > c && !a.compare_exchange_weak(c, v, std::memory_order_relaxed)) {}
    }
    static void noteMax(std::atomic<int>& a, int v)
    {
        int c = a.load(std::memory_order_relaxed);
        while (v > c && !a.compare_exchange_weak(c, v, std::memory_order_relaxed)) {}
    }
};
