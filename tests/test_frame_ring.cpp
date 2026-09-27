#include <catch2/catch_test_macros.hpp>
#include "render/FrameRing.h"
#include <vector>

// s-rta-0927 plan-renderperf C1: a Screen Split / Frame Stutter ring cell (texture + FBO) is created the first
// time it is WRITTEN (CompositorEngine::pushFrameToRing), never in bulk -- creating all 480 cells in one frame
// cost 35.5-38.5 ms. That is only safe if the read path never hands out a cell that has not been written in the
// ring's current lifetime. FrameRing::readIndex is the read path's index math (CompositorEngine::getFrameFromRing
// calls it); these cases pin it against the historical inline expression and prove the written-before-read
// invariant over a long push sequence that includes a crossfade hand-over's reset.

namespace
{
constexpr int kCap = 480;   // CompositorEngine::kMaxRingFrames

// The inline expression getFrameFromRing used before C1 (CompositorEngine.cpp at 1636785), copied verbatim
// after its clamp. frameCount >= 1 here (the old code returned 0 before reaching it).
int historicalIndex(int writeIndex, int frameCount, int framesAgo)
{
    int maxDelay = frameCount - 1;
    if (framesAgo > maxDelay) framesAgo = maxDelay;
    if (framesAgo < 0) framesAgo = 0;
    return (writeIndex - 1 - framesAgo + kCap * 2) % kCap;
}

// The two-line advance of pushFrameToRing, copied.
void advance(int& writeIndex, int& frameCount)
{
    writeIndex = (writeIndex + 1) % kCap;
    if (frameCount < kCap)
        frameCount++;
}
} // namespace

TEST_CASE("readIndex equals the historical inline expression", "[frame_ring][s-rta-0927]")
{
    long checked = 0, mismatches = 0;
    for (int wi = 0; wi < kCap; ++wi)
        for (int fc = 1; fc <= kCap; ++fc)
        {
            auto one = [&](int ago)
            {
                ++checked;
                if (FrameRing::readIndex(wi, fc, ago, kCap) != historicalIndex(wi, fc, ago))
                {
                    if (++mismatches <= 5)
                        UNSCOPED_INFO("writeIndex " << wi << " frameCount " << fc << " framesAgo " << ago << ": "
                                      << FrameRing::readIndex(wi, fc, ago, kCap) << " != "
                                      << historicalIndex(wi, fc, ago));
                }
            };
            for (int ago = 0; ago <= fc + 3; ++ago)
                one(ago);
            one(3780);   // Screen Split's reach: 8x8 cells x 60 frames/cell, far past the capacity
            one(-1);
        }
    INFO("checked " << checked << " inputs");
    CHECK(mismatches == 0);
}

TEST_CASE("an index readIndex returns has always been written in the current lifetime", "[frame_ring][s-rta-0927]")
{
    std::vector<bool> written(static_cast<size_t>(kCap), false);
    int writeIndex = 0, frameCount = 0;
    long reads = 0, unwritten = 0;
    for (int push = 1; push <= 1500; ++push)
    {
        if (push == 700)
        {
            // handOverClipHistory: the incoming ring (swapped in) starts empty. Its cells may already exist --
            // they are reused, never read before this lifetime writes them again.
            writeIndex = 0;
            frameCount = 0;
            std::fill(written.begin(), written.end(), false);
        }
        written[static_cast<size_t>(writeIndex)] = true;   // pushFrameToRing writes cell writeIndex, then advances
        advance(writeIndex, frameCount);

        auto check = [&](int ago)
        {
            ++reads;
            const int idx = FrameRing::readIndex(writeIndex, frameCount, ago, kCap);
            if (idx < 0 || idx >= kCap || !written[static_cast<size_t>(idx)])
            {
                if (++unwritten <= 5)
                    UNSCOPED_INFO("push " << push << " framesAgo " << ago << " -> index " << idx
                                  << " (writeIndex " << writeIndex << ", frameCount " << frameCount << ")");
            }
        };
        for (int ago = 0; ago <= 600; ++ago)
            check(ago);
        check(3780);
    }
    INFO(reads << " reads");
    CHECK(unwritten == 0);
}

TEST_CASE("empty ring / after reset returns -1 until the next push", "[frame_ring][s-rta-0927]")
{
    CHECK(FrameRing::readIndex(0, 0, 0, kCap) == -1);
    CHECK(FrameRing::readIndex(0, 0, 3780, kCap) == -1);
    CHECK(FrameRing::readIndex(137, 0, 5, kCap) == -1);   // any writeIndex: frameCount 0 holds nothing
    CHECK(FrameRing::readIndex(5, 3, 0, 0) == -1);        // no capacity

    int writeIndex = 0, frameCount = 0;
    advance(writeIndex, frameCount);                      // one push into cell 0
    CHECK(FrameRing::readIndex(writeIndex, frameCount, 0, kCap) == 0);
    CHECK(FrameRing::readIndex(writeIndex, frameCount, 3780, kCap) == 0);   // clamped to the only frame
    CHECK(FrameRing::readIndex(writeIndex, frameCount, -1, kCap) == 0);
}
