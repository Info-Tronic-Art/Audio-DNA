#pragma once
#include <atomic>
#include <type_traits>

// Relaxed<T>: a model field that another thread reads (lane tsan, s-rta-1002; Pitfall 63). A plain field the GL thread
// reads while the message thread writes it is a data race (UB; ThreadSanitizer reports it). Relaxed<T> makes every
// access one relaxed atomic load or store: exactly Sacred Rule 2's "UI -> hot path via std::atomic<T>" for a field
// with ONE writer. It orders nothing: a value that must be seen together with another value lives in ONE atomic
// word (Layer's trigger tuple, LayerRuntimeCell), never in two Relaxed fields.
//
// Copyable, for the same reason as LiveValue (connect/LiveValue.h): Clip / Layer / Composition are value types
// (copied by undo snapshots, held by value in vectors), and std::atomic<T> has a deleted copy constructor. A copy is
// one relaxed load + one relaxed store.
//
// Reads and plain assignments compile unchanged (operator T / operator=(T)). There are NO compound operators (+=, ++):
// a read-modify-write is spelled fetchAdd (one atomic RMW), or load() / store() on purpose, so a lost update cannot
// hide behind `x += 1`. `auto x = field;` is a Relaxed copy, harmless.
template <class T>
class Relaxed
{
    static_assert(std::atomic<T>::is_always_lock_free, "Relaxed<T> needs a lock-free atomic (never a hidden lock)");

public:
    Relaxed() noexcept : v_(T{}) {}
    Relaxed(T x) noexcept : v_(x) {}   // implicit: `RelaxedFloat opacity = 1.0f;` member initializers stay as they are

    Relaxed(const Relaxed& o) noexcept : v_(o.load()) {}
    Relaxed(Relaxed&& o) noexcept : v_(o.load()) {}
    Relaxed& operator=(const Relaxed& o) noexcept { store(o.load()); return *this; }
    Relaxed& operator=(Relaxed&& o) noexcept { store(o.load()); return *this; }
    Relaxed& operator=(T x) noexcept { store(x); return *this; }

    operator T() const noexcept { return load(); }

    T load() const noexcept { return v_.load(std::memory_order_relaxed); }
    void store(T x) noexcept { v_.store(x, std::memory_order_relaxed); }

    // Strong compare-exchange: on failure `expected` receives the current value.
    bool compareExchange(T& expected, T desired) noexcept
    {
        return v_.compare_exchange_strong(expected, desired, std::memory_order_relaxed, std::memory_order_relaxed);
    }

    // One atomic read-modify-write; returns the value before the add (integral T other than bool).
    template <class U = T, std::enable_if_t<std::is_integral_v<U> && !std::is_same_v<U, bool>, int> = 0>
    T fetchAdd(T delta) noexcept
    {
        return v_.fetch_add(delta, std::memory_order_relaxed);
    }

private:
    std::atomic<T> v_;
};

using RelaxedFloat = Relaxed<float>;
using RelaxedDouble = Relaxed<double>;
using RelaxedBool = Relaxed<bool>;
using RelaxedInt = Relaxed<int>;
