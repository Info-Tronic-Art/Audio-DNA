#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "audio/RingBuffer.h"
#include <array>
#include <thread>
#include <numeric>
#include <vector>

using Catch::Approx;

TEST_CASE("RingBuffer capacity is power of two", "[ringbuffer]")
{
    SECTION("exact power of two")
    {
        RingBuffer<float> rb(1024);
        REQUIRE(rb.capacity() == 1024);
    }

    SECTION("rounds up to next power of two")
    {
        RingBuffer<float> rb(1000);
        REQUIRE(rb.capacity() == 1024);
    }

    SECTION("small values")
    {
        RingBuffer<float> rb(3);
        REQUIRE(rb.capacity() == 4);
    }
}

TEST_CASE("RingBuffer basic push and pop", "[ringbuffer]")
{
    RingBuffer<float> rb(16);

    SECTION("push then pop returns same data")
    {
        std::array<float, 4> input = {1.0f, 2.0f, 3.0f, 4.0f};
        std::array<float, 4> output{};

        size_t pushed = rb.push(input.data(), 4);
        REQUIRE(pushed == 4);
        REQUIRE(rb.availableToRead() == 4);

        size_t popped = rb.pop(output.data(), 4);
        REQUIRE(popped == 4);
        REQUIRE(rb.availableToRead() == 0);

        for (int i = 0; i < 4; ++i)
            REQUIRE(output[static_cast<size_t>(i)] == Approx(input[static_cast<size_t>(i)]));
    }

    SECTION("partial pop")
    {
        std::array<float, 4> input = {10.0f, 20.0f, 30.0f, 40.0f};
        rb.push(input.data(), 4);

        std::array<float, 2> output{};
        size_t popped = rb.pop(output.data(), 2);
        REQUIRE(popped == 2);
        REQUIRE(output[0] == Approx(10.0f));
        REQUIRE(output[1] == Approx(20.0f));
        REQUIRE(rb.availableToRead() == 2);
    }

    SECTION("pop from empty returns zero")
    {
        std::array<float, 4> output{};
        size_t popped = rb.pop(output.data(), 4);
        REQUIRE(popped == 0);
    }
}

TEST_CASE("RingBuffer fills to capacity", "[ringbuffer]")
{
    RingBuffer<float> rb(8);  // capacity = 8

    std::array<float, 8> input{};
    std::iota(input.begin(), input.end(), 1.0f);

    size_t pushed = rb.push(input.data(), 8);
    REQUIRE(pushed == 8);
    REQUIRE(rb.availableToRead() == 8);

    // Buffer is full — push should return 0
    float extra = 99.0f;
    pushed = rb.push(&extra, 1);
    REQUIRE(pushed == 0);
}

TEST_CASE("RingBuffer wraps around correctly", "[ringbuffer]")
{
    RingBuffer<float> rb(8);

    // Fill halfway, drain, fill again to force wrap
    std::array<float, 6> a{};
    std::iota(a.begin(), a.end(), 1.0f);
    rb.push(a.data(), 6);

    std::array<float, 6> discard{};
    rb.pop(discard.data(), 6);

    // Now write pointer is at 6. Push 6 more — wraps around past capacity.
    std::array<float, 6> b = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};
    size_t pushed = rb.push(b.data(), 6);
    REQUIRE(pushed == 6);

    std::array<float, 6> output{};
    size_t popped = rb.pop(output.data(), 6);
    REQUIRE(popped == 6);

    for (int i = 0; i < 6; ++i)
        REQUIRE(output[static_cast<size_t>(i)] == Approx(b[static_cast<size_t>(i)]));
}

TEST_CASE("RingBuffer concurrent push/pop", "[ringbuffer]")
{
    constexpr size_t kCapacity = 16384;
    constexpr size_t kTotalSamples = 500000;
    constexpr size_t kChunkSize = 128;

    RingBuffer<float> rb(kCapacity);

    std::vector<float> produced(kTotalSamples);
    std::iota(produced.begin(), produced.end(), 0.0f);

    std::vector<float> consumed(kTotalSamples, -1.0f);

    // Producer thread
    std::thread producer([&]()
    {
        size_t offset = 0;
        while (offset < kTotalSamples)
        {
            size_t remaining = kTotalSamples - offset;
            size_t toWrite = std::min(kChunkSize, remaining);
            size_t written = rb.push(produced.data() + offset, toWrite);
            offset += written;
            if (written == 0)
                std::this_thread::yield();
        }
    });

    // Consumer thread
    std::thread consumer([&]()
    {
        size_t offset = 0;
        while (offset < kTotalSamples)
        {
            size_t remaining = kTotalSamples - offset;
            size_t toRead = std::min(kChunkSize, remaining);
            size_t read = rb.pop(consumed.data() + offset, toRead);
            offset += read;
            if (read == 0)
                std::this_thread::yield();
        }
    });

    producer.join();
    consumer.join();

    // Verify every sample arrived in order
    for (size_t i = 0; i < kTotalSamples; ++i)
        REQUIRE(consumed[i] == Approx(produced[i]));
}
