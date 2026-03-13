#pragma once
#include <atomic>
#include <cstddef>
#include <cstring>
#include <new>
#include <type_traits>

// Single-Producer Single-Consumer lock-free ring buffer.
// Power-of-two capacity. acquire/release atomics. Cache-line padded.
template <typename T>
class RingBuffer
{
    static_assert(std::is_trivially_copyable_v<T>);

public:
    explicit RingBuffer(size_t requestedCapacity)
        : capacity_(nextPowerOfTwo(requestedCapacity)),
          mask_(capacity_ - 1),
          buffer_(new T[capacity_]{})
    {
    }

    ~RingBuffer() { delete[] buffer_; }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // Producer: push count items. Returns number actually written.
    size_t push(const T* data, size_t count) noexcept
    {
        const auto w = writePos_.load(std::memory_order_relaxed);
        const auto r = readPos_.load(std::memory_order_acquire);
        const auto available = capacity_ - (w - r);
        const auto toWrite = (count < available) ? count : available;

        for (size_t i = 0; i < toWrite; ++i)
            buffer_[(w + i) & mask_] = data[i];

        writePos_.store(w + toWrite, std::memory_order_release);
        return toWrite;
    }

    // Consumer: pop count items into dest. Returns number actually read.
    size_t pop(T* dest, size_t count) noexcept
    {
        const auto r = readPos_.load(std::memory_order_relaxed);
        const auto w = writePos_.load(std::memory_order_acquire);
        const auto available = w - r;
        const auto toRead = (count < available) ? count : available;

        for (size_t i = 0; i < toRead; ++i)
            dest[i] = buffer_[(r + i) & mask_];

        readPos_.store(r + toRead, std::memory_order_release);
        return toRead;
    }

    size_t availableToRead() const noexcept
    {
        const auto w = writePos_.load(std::memory_order_acquire);
        const auto r = readPos_.load(std::memory_order_relaxed);
        return w - r;
    }

    size_t capacity() const noexcept { return capacity_; }

private:
    static size_t nextPowerOfTwo(size_t v)
    {
        v--;
        v |= v >> 1; v |= v >> 2; v |= v >> 4;
        v |= v >> 8; v |= v >> 16; v |= v >> 32;
        return v + 1;
    }

    const size_t capacity_;
    const size_t mask_;
    T* buffer_;

    // Cache-line padded to prevent false sharing
    alignas(64) std::atomic<size_t> writePos_{0};
    alignas(64) std::atomic<size_t> readPos_{0};
};
