#include <catch2/catch_test_macros.hpp>
#include "render/FencedPtrSlot.h"
#include <atomic>
#include <thread>

// s-rta-0928b mediaopen (Harmony adoption P3): the renderer's active deck and the withDeckDetached fence state live in
// ONE atomic word (src/render/FencedPtrSlot.h) so the GL thread reads both with one load. These cases pin the single-
// thread semantics and stress the two-thread protocol: a writer that runs the fence's exact store sequence (detach
// fenced, restore a deck) against a reader that must never see a torn pair -- "fenced with a deck" or "unfenced
// without one". Headless, no GL, no JUCE (registered like test_frame_ring).

namespace
{
struct alignas(8) FakeDeck { int id = 0; };
}

TEST_CASE("FencedPtrSlot single-thread semantics", "[mediaopen][fence]")
{
    FakeDeck a{ 1 }, b{ 2 };
    FencedPtrSlot<FakeDeck> slot;
    auto v = slot.view();
    CHECK(v.ptr == nullptr);
    CHECK_FALSE(v.fenced);

    slot.set(&a);
    v = slot.view();
    CHECK(v.ptr == &a);
    CHECK_FALSE(v.fenced);
    CHECK(slot.get() == &a);

    slot.detachFenced();          // the fence begins: no deck AND fenced, one store
    v = slot.view();
    CHECK(v.ptr == nullptr);
    CHECK(v.fenced);
    CHECK(slot.get() == nullptr);

    slot.set(&b);                 // the fence ends: the restored deck, unfenced, one store
    v = slot.view();
    CHECK(v.ptr == &b);
    CHECK_FALSE(v.fenced);

    slot.set(nullptr);            // a plain null is NOT a fence
    v = slot.view();
    CHECK(v.ptr == nullptr);
    CHECK_FALSE(v.fenced);
}

TEST_CASE("FencedPtrSlot: a reader never sees a torn fence edge (two threads)", "[mediaopen][fence][stress]")
{
    FakeDeck decks[2] = { { 1 }, { 2 } };
    FencedPtrSlot<FakeDeck> slot;
    slot.set(&decks[0]);          // from here on the writer never stores a bare null

    constexpr int kFences = 300000;
    std::atomic<bool> started{ false }, done{ false };
    std::atomic<long> fencedWithDeck{ 0 }, unfencedWithoutDeck{ 0 }, sawFenced{ 0 }, sawDeck{ 0 }, reads{ 0 };

    std::thread reader([&] {
        long fwd = 0, uwd = 0, sf = 0, sd = 0, n = 0;
        started.store(true, std::memory_order_release);
        while (!done.load(std::memory_order_acquire))
        {
            const auto v = slot.view();
            ++n;
            if (v.fenced) { ++sf; if (v.ptr != nullptr) ++fwd; }
            else          { ++sd; if (v.ptr == nullptr) ++uwd; }
            if (v.ptr != nullptr && v.ptr != &decks[0] && v.ptr != &decks[1])
                ++fwd;   // a pointer the writer never stored: a torn word
        }
        fencedWithDeck = fwd; unfencedWithoutDeck = uwd; sawFenced = sf; sawDeck = sd; reads = n;
    });

    while (!started.load(std::memory_order_acquire)) {}
    volatile int spin = 0;                    // each state lasts a few dozen ns: the reader lands in both
    for (int i = 0; i < kFences; ++i)
    {
        slot.detachFenced();                  // UndoService::withDeckDetached: begin
        for (int k = 0; k < 16; ++k) spin = spin + 1;
        slot.set(&decks[(i + 1) % 2]);        // ActiveDeckRestoreGuard: end (the deck may have moved)
        for (int k = 0; k < 16; ++k) spin = spin + 1;
    }
    done.store(true, std::memory_order_release);
    reader.join();

    INFO("reads " << reads.load() << ", fenced seen " << sawFenced.load() << ", deck seen " << sawDeck.load());
    CHECK(fencedWithDeck.load() == 0);
    CHECK(unfencedWithoutDeck.load() == 0);
    // Not vacuous: the reader saw both states.
    CHECK(sawFenced.load() > 0);
    CHECK(sawDeck.load() > 0);
}
