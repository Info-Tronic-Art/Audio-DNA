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

    int   takeGlMaxDecodesPerCall() { return glMaxDecodesPerCall.exchange(0, std::memory_order_relaxed); }
    float takePeakUploadMs()        { return peakUploadMs.exchange(0.0f, std::memory_order_relaxed); }
    float takeMsgLockWaitMaxMs()    { return msgLockWaitMaxMs.exchange(0.0f, std::memory_order_relaxed); }

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
