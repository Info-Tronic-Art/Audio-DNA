// s-rta-0929 asyncload (plan-asyncload.md 5.1 / 5.8 test 1): LoadTicket -- the waitable outcome of one composition load
// that POST /api/load_composition waits on. A finish from another thread releases the waiter; the first finish wins; a
// wait on an unfinished ticket times out as Pending.
#include <catch2/catch_test_macros.hpp>

#include "core/LoadTicket.h"

#include <chrono>
#include <thread>

using Outcome = LoadTicket::Outcome;

TEST_CASE("a finish from another thread releases the waiter", "[asyncload][ticket]")
{
    LoadTicket t;
    std::thread th([&t] {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        t.finish(Outcome::Done);
    });
    const auto t0 = std::chrono::steady_clock::now();
    const auto o = t.wait(2000);
    const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    th.join();
    CHECK(o == Outcome::Done);
    CHECK(ms < 1500.0);   // released by the finish, not by the timeout
}

TEST_CASE("the first finish wins", "[asyncload][ticket]")
{
    LoadTicket t;
    CHECK(t.finish(Outcome::Done));
    CHECK_FALSE(t.finish(Outcome::Failed));
    CHECK_FALSE(t.finish(Outcome::Cancelled));
    CHECK(t.outcome() == Outcome::Done);
    CHECK(t.wait(10) == Outcome::Done);
    CHECK(t.wait(10) == Outcome::Done);   // every later wait returns at once
}

TEST_CASE("a wait on an unfinished ticket times out as Pending", "[asyncload][ticket]")
{
    LoadTicket t;
    CHECK(t.wait(10) == Outcome::Pending);
    CHECK(t.outcome() == Outcome::Pending);
    CHECK(t.finish(Outcome::Superseded));
    CHECK(t.wait(10) == Outcome::Superseded);
}
