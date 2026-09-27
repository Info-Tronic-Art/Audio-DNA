#pragma once

// FrameRing: the pure index math of CompositorEngine::FrameRingBuffer (Screen Split / Frame Stutter).
// No GL, no JUCE -- unit-tested headless by tests/test_frame_ring.cpp.
// writeIndex = the NEXT cell to be written; frameCount = cells written in this lifetime (saturates at capacity).
// Every index readIndex returns lies in the written set {writeIndex-1, ..., writeIndex-frameCount} mod capacity --
// the invariant that lets a cell be created on its first write (s-rta-0927 plan-renderperf C1).
// -1 = the ring holds nothing.
namespace FrameRing
{
constexpr int readIndex(int writeIndex, int frameCount, int framesAgo, int capacity) noexcept
{
    if (frameCount <= 0 || capacity <= 0)
        return -1;
    const int maxDelay = frameCount - 1;
    if (framesAgo > maxDelay) framesAgo = maxDelay;
    if (framesAgo < 0) framesAgo = 0;
    return (writeIndex - 1 - framesAgo + capacity * 2) % capacity;
}
} // namespace FrameRing
