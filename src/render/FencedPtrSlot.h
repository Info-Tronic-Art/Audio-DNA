#pragma once
#include <atomic>
#include <cstdint>

// FencedPtrSlot (s-rta-0928b mediaopen, Harmony adoption P3): the renderer's active-deck pointer AND the
// UndoService::withDeckDetached fence state in ONE atomic word, so the GL thread reads both with a single load --
// there is no two-load window on the edge that begins the fence or on the edge that ends it.
//
// The word is the pointer with bit 0 as the "fenced" mark (every T the slot holds is at least 2-byte aligned).
//   set(p)             -- any pointer (incl. nullptr), NOT fenced: an ordinary store (setActiveDeck).
//   detachFenced()     -- nullptr AND fenced, in one store: the fence begins (withDeckDetached, before its GL drain).
//   set(restored)      -- the fence ends: the restored pointer, unfenced, in one store (the fence's RAII restore).
//   view()             -- ONE acquire load: {ptr, fenced}. Never "fenced with a deck", never a torn pair.
// A plain set(nullptr) is NOT a fence: a frame that sees it renders today's "nothing to render" path.
//
// Pure header (<atomic> only): tests/test_fenced_ptr_slot.cpp stresses it across two threads.
template <typename T>
class FencedPtrSlot
{
public:
    struct View
    {
        T* ptr = nullptr;
        bool fenced = false;
    };

    void set(T* p) noexcept
    {
        static_assert(alignof(T) >= 2, "bit 0 of the pointer carries the fence mark");
        word_.store(reinterpret_cast<std::uintptr_t>(p), std::memory_order_release);
    }

    void detachFenced() noexcept { word_.store(kFenced, std::memory_order_release); }

    T* get() const noexcept { return reinterpret_cast<T*>(word_.load(std::memory_order_acquire) & ~kFenced); }

    View view() const noexcept
    {
        const std::uintptr_t w = word_.load(std::memory_order_acquire);   // ONE load: pointer and mark together
        return { reinterpret_cast<T*>(w & ~kFenced), (w & kFenced) != 0 };
    }

private:
    static constexpr std::uintptr_t kFenced = 1;
    std::atomic<std::uintptr_t> word_{ 0 };
};
