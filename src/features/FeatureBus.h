#pragma once
#include <atomic>
#include <array>
#include "analysis/FeatureSnapshot.h"

// Lock-free triple-buffer for publishing FeatureSnapshots from the analysis
// thread to the render/UI thread.
//
// Triple-buffer overview:
//   3 slots: one owned by the writer, one is the "latest" ready for the reader,
//   and one owned by the reader. An atomic uint8 encodes slot assignments as a
//   packed bitfield: bits [1:0] = write slot, bits [3:2] = latest slot,
//   bits [5:4] = read slot, bit 6 = "new data available" flag.
//
// The writer calls acquireWrite() to get a pointer, fills it, then calls
// publishWrite(). The reader calls acquireRead() to get the latest snapshot
// (swapping its current read slot with the latest slot atomically).
//
// All operations are wait-free.

class FeatureBus
{
public:
    FeatureBus();

    // --- Writer API (analysis thread) ---

    // Get a pointer to the current write buffer. Fill it, then call publishWrite().
    FeatureSnapshot* acquireWrite();

    // Mark the write buffer as the new "latest" and swap in a fresh write buffer.
    void publishWrite();

    // --- Reader API (render/UI thread) ---

    // Get a pointer to the most recent published snapshot.
    // Returns nullptr if no new data has been published since the last call.
    // When non-null, the returned pointer remains valid until the next acquireRead() call.
    const FeatureSnapshot* acquireRead();

    // Get the last-read snapshot (even if no new data). Never returns nullptr
    // after at least one publishWrite() + acquireRead() cycle.
    const FeatureSnapshot* getLatestRead() const;

    // Returns true if the writer has published data that the reader hasn't consumed.
    bool hasNewData() const;

private:
    // Slot encoding within the atomic state byte:
    //   bits [1:0] = write index  (0, 1, or 2)
    //   bits [3:2] = latest index (0, 1, or 2)
    //   bits [5:4] = read index   (0, 1, or 2)
    //   bit  [6]   = new data flag
    static constexpr uint8_t kWriteMask  = 0x03;       // bits 0-1
    static constexpr uint8_t kLatestShift = 2;
    static constexpr uint8_t kLatestMask = 0x0C;       // bits 2-3
    static constexpr uint8_t kReadShift  = 4;
    static constexpr uint8_t kReadMask   = 0x30;       // bits 4-5
    static constexpr uint8_t kNewFlag    = 0x40;        // bit 6

    static uint8_t encodeState(uint8_t write, uint8_t latest, uint8_t read, bool newData)
    {
        return static_cast<uint8_t>(
            (write & 0x03)
            | ((latest & 0x03) << kLatestShift)
            | ((read & 0x03) << kReadShift)
            | (newData ? kNewFlag : 0));
    }

    alignas(64) std::array<FeatureSnapshot, 3> buffers_{};
    alignas(64) std::atomic<uint8_t> state_;
};
