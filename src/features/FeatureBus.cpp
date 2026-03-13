#include "features/FeatureBus.h"

FeatureBus::FeatureBus()
{
    for (auto& buf : buffers_)
        buf.clear();

    // Initial state: write=0, latest=1, read=2, no new data
    state_.store(encodeState(0, 1, 2, false), std::memory_order_relaxed);
}

FeatureSnapshot* FeatureBus::acquireWrite()
{
    uint8_t s = state_.load(std::memory_order_relaxed);
    uint8_t writeIdx = s & kWriteMask;
    return &buffers_[writeIdx];
}

void FeatureBus::publishWrite()
{
    // Swap write and latest slots, set the new-data flag.
    // CAS loop because the reader may concurrently swap latest and read.
    uint8_t expected = state_.load(std::memory_order_relaxed);
    uint8_t desired;

    do
    {
        uint8_t writeIdx  = expected & kWriteMask;
        uint8_t latestIdx = (expected & kLatestMask) >> kLatestShift;
        uint8_t readIdx   = (expected & kReadMask) >> kReadShift;

        // The old latest becomes the new write buffer; our write becomes latest.
        desired = encodeState(latestIdx, writeIdx, readIdx, true);
    }
    while (!state_.compare_exchange_weak(expected, desired,
                                          std::memory_order_release,
                                          std::memory_order_relaxed));
}

const FeatureSnapshot* FeatureBus::acquireRead()
{
    // Swap read and latest slots if new data is available.
    uint8_t expected = state_.load(std::memory_order_relaxed);

    if (!(expected & kNewFlag))
        return nullptr;

    uint8_t desired;

    do
    {
        if (!(expected & kNewFlag))
            return nullptr;

        uint8_t writeIdx  = expected & kWriteMask;
        uint8_t latestIdx = (expected & kLatestMask) >> kLatestShift;
        uint8_t readIdx   = (expected & kReadMask) >> kReadShift;

        // Swap read and latest, clear new-data flag.
        desired = encodeState(writeIdx, readIdx, latestIdx, false);
    }
    while (!state_.compare_exchange_weak(expected, desired,
                                          std::memory_order_acquire,
                                          std::memory_order_relaxed));

    // The old latest is now our read slot.
    uint8_t latestIdx = (expected & kLatestMask) >> kLatestShift;
    return &buffers_[latestIdx];
}

const FeatureSnapshot* FeatureBus::getLatestRead() const
{
    uint8_t s = state_.load(std::memory_order_acquire);
    uint8_t readIdx = (s & kReadMask) >> kReadShift;
    return &buffers_[readIdx];
}

bool FeatureBus::hasNewData() const
{
    return (state_.load(std::memory_order_relaxed) & kNewFlag) != 0;
}
