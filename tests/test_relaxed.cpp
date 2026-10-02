// test_relaxed -- lane tsan (s-rta-1002; plan .harmony/.reports/s-rta-0930/plan-tsan.md T1): Relaxed<T>
// (src/model/Relaxed.h), the copyable relaxed-atomic model field. GREEN-only: the value semantics a plain field had
// (copy / move / implicit read / assignment) plus the two atomic read-modify-writes the lane relies on.
#include <catch2/catch_test_macros.hpp>
#include "model/Relaxed.h"
#include <type_traits>
#include <utility>

static_assert(std::is_nothrow_move_constructible_v<Relaxed<double>>);
static_assert(std::is_nothrow_copy_constructible_v<RelaxedFloat>);
static_assert(std::is_nothrow_move_assignable_v<RelaxedBool>);
static_assert(std::is_nothrow_copy_assignable_v<RelaxedInt>);

TEST_CASE("Relaxed: copy and move preserve the value", "[relaxed]")
{
    RelaxedDouble a = 0.25;
    RelaxedDouble b = a;              // copy ctor
    RelaxedDouble c = std::move(a);   // move ctor
    REQUIRE(b.load() == 0.25);
    REQUIRE(c.load() == 0.25);

    RelaxedDouble d;
    REQUIRE(d.load() == 0.0);         // value-initialized
    d = b;                            // copy assign
    REQUIRE(d.load() == 0.25);
    RelaxedDouble e;
    e = std::move(c);                 // move assign
    REQUIRE(e.load() == 0.25);
}

TEST_CASE("Relaxed: implicit read and plain assignment compile like the plain field", "[relaxed]")
{
    RelaxedFloat f = 1.0f;
    const float x = f;                // operator T
    REQUIRE(x == 1.0f);
    f = 0.5f;                         // operator=(T)
    REQUIRE(f == 0.5f);               // operator T inside a comparison
    RelaxedBool b = false;
    b = true;
    REQUIRE(b);
    if (!b) FAIL("operator bool read");
    f.store(0.75f);
    REQUIRE(f.load() == 0.75f);
}

TEST_CASE("Relaxed: compareExchange succeeds on a match and reports the current value on a mismatch", "[relaxed]")
{
    RelaxedBool playing = false;
    bool expected = false;
    REQUIRE(playing.compareExchange(expected, true));
    REQUIRE(playing.load() == true);

    expected = false;                 // stale: the value is true now
    REQUIRE_FALSE(playing.compareExchange(expected, false));
    REQUIRE(expected == true);        // failure writes the current value into expected
    REQUIRE(playing.load() == true);  // and stores nothing
}

TEST_CASE("Relaxed: fetchAdd is one read-modify-write returning the previous value", "[relaxed]")
{
    RelaxedInt beats = 7;
    REQUIRE(beats.fetchAdd(1) == 7);
    REQUIRE(beats.load() == 8);
    REQUIRE(beats.fetchAdd(3) + 3 == 11);
    REQUIRE(beats == 11);
}
