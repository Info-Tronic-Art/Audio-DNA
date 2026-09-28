#pragma once
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

// SeqVram: the texture memory of image sequences (s-rta-0928b seqvram, .harmony/.reports/s-rta-0928b/plan-seqvram.md).
// Pure -- no GL, no JUCE. Stats are the seq_* counters of /api/state (7070 + 8080); a Grant is what the Renderer hands
// a drawn sequence each frame.
namespace SeqVram
{
// All image sequences together (ASSUMED 1 GiB, H13: an M1 Pro with 32 GiB); the stills have their own budget
// (CompositorEngine kImagePrefetchBudgetBytes).
constexpr size_t kBudgetBytes = size_t{ 1 } << 30;

struct Stats   // relaxed atomics, written on the GL thread, read by /api/state
{
    std::atomic<int64_t> framesShown{ 0 };     // frame-index changes presented
    std::atomic<int64_t> lateFrames{ 0 };      // current frame not resident: the shown one repeated
    std::atomic<int64_t> pendingFrames{ 0 };   // nothing to show yet
    std::atomic<int64_t> uploads{ 0 };
    std::atomic<int64_t> slotReuses{ 0 };      // glTexSubImage2D into a recycled slot
    std::atomic<int64_t> evictions{ 0 };
    std::atomic<int64_t> staleDrops{ 0 };      // a decoded result no longer inside the window, dropped unuploaded
    std::atomic<int64_t> uploadDeferred{ 0 };  // H1: no slot free this frame, the result waits in ready_
    std::atomic<int64_t> residentBytes{ 0 };   // every sequence's allocated texture bytes (frame top)
    std::atomic<int> openCount{ 0 }, residentSlots{ 0 }, overBudget{ 0 };
};

struct Grant
{
    size_t allowanceBytes = 0;
    uint64_t frameSerial = 0;
    Stats* stats = nullptr;
};
} // namespace SeqVram
