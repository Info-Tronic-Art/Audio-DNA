// test_surface_pool -- the IOSurface half of output::SharedFrameSet, headless: NO GL context
// (s-rta-0927 outputs-c1 = plan5 slice C1, .harmony/.reports/s-rta-0926b/plan5-final.md 8.1 / 10.1).
#include <catch2/catch_test_macros.hpp>
#include "output/SharedFrameSet.h"
#include <CoreFoundation/CoreFoundation.h>

using output::SurfacePool;

TEST_CASE("SurfacePool: first ensure creates generation 1 of kSlots BGRA surfaces", "[surface_pool]")
{
    SurfacePool pool;
    REQUIRE(SurfacePool::kSlots == 4);
    REQUIRE(pool.generation() == 0);
    REQUIRE(pool.ensure(1920, 1080));
    REQUIRE(pool.generation() == 1);
    REQUIRE(pool.width() == 1920);
    REQUIRE(pool.height() == 1080);
    for (int i = 0; i < SurfacePool::kSlots; ++i)
    {
        IOSurfaceRef s = pool.surface(i);
        REQUIRE(s != nullptr);
        CHECK(IOSurfaceGetWidth(s) == 1920);
        CHECK(IOSurfaceGetHeight(s) == 1080);
        CHECK(IOSurfaceGetPixelFormat(s) == static_cast<OSType>('BGRA'));
        CHECK(IOSurfaceGetBytesPerElement(s) == 4);
        for (int j = 0; j < i; ++j)
            CHECK(pool.surface(j) != s);   // kSlots DISTINCT surfaces
    }
    CHECK(pool.surface(SurfacePool::kSlots) == nullptr);
    CHECK(pool.surface(-1) == nullptr);
}

TEST_CASE("SurfacePool: ensure with the same size is a no-op; invalid sizes are ignored", "[surface_pool]")
{
    SurfacePool pool;
    REQUIRE(pool.ensure(1920, 1080));
    IOSurfaceRef s0 = pool.surface(0);
    CHECK_FALSE(pool.ensure(1920, 1080));
    CHECK(pool.generation() == 1);
    CHECK(pool.surface(0) == s0);
    CHECK_FALSE(pool.ensure(0, 1080));
    CHECK_FALSE(pool.ensure(1920, -1));
    CHECK(pool.generation() == 1);
}

TEST_CASE("SurfacePool: a new size retires the old generation, released kRetireFrames ticks later", "[surface_pool]")
{
    SurfacePool pool;
    REQUIRE(pool.ensure(1920, 1080));
    REQUIRE(pool.ensure(1280, 720));
    REQUIRE(pool.generation() == 2);
    CHECK(IOSurfaceGetWidth(pool.surface(0)) == 1280);

    // The retired generation is still reachable by a reader that has not rebound yet.
    IOSurfaceRef old = pool.retainSurface(1, 0);
    REQUIRE(old != nullptr);
    CHECK(IOSurfaceGetWidth(old) == 1920);
    CFRelease(old);

    for (int i = 0; i < SurfacePool::kRetireFrames - 1; ++i)
        pool.tick();
    old = pool.retainSurface(1, 0);
    CHECK(old != nullptr);   // 119 ticks: still there
    if (old != nullptr)
        CFRelease(old);
    pool.tick();             // the 120th
    CHECK(pool.retainSurface(1, 0) == nullptr);

    IOSurfaceRef cur = pool.retainSurface(2, 3);
    REQUIRE(cur != nullptr);
    CHECK(cur == pool.surface(3));
    CFRelease(cur);
    CHECK(pool.retainSurface(9, 0) == nullptr);   // never existed
    CHECK(pool.retainSurface(0, 0) == nullptr);   // "nothing published"
    CHECK(pool.retainSurface(2, SurfacePool::kSlots) == nullptr);
}

TEST_CASE("SurfacePool: a reader's retain outlives the pool's release", "[surface_pool]")
{
    SurfacePool pool;
    REQUIRE(pool.ensure(64, 36));
    IOSurfaceRef held = pool.retainSurface(1, 1);
    REQUIRE(held != nullptr);
    REQUIRE(pool.ensure(32, 18));
    for (int i = 0; i < SurfacePool::kRetireFrames; ++i)
        pool.tick();
    CHECK(pool.retainSurface(1, 1) == nullptr);
    CHECK(IOSurfaceGetWidth(held) == 64);   // still a valid object: the reader's own retain
    CFRelease(held);
}

TEST_CASE("FrontFrame: pack / unpack round trip, including the serial wrap", "[surface_pool]")
{
    using output::packFront;
    using output::unpackFront;
    const output::FrontFrame a{ 1, 1, 0 };
    CHECK(unpackFront(packFront(1, 1, 0)) == a);
    const output::FrontFrame b{ 0xFFFFFFu, 0xFFFFFFFFu, 3 };
    CHECK(unpackFront(packFront(0xFFFFFFu, 0xFFFFFFFFu, 3)) == b);
    uint32_t serial = 0xFFFFFFFFu;
    ++serial;   // wraps to 0: the packing never bleeds into gen or slot
    const output::FrontFrame c{ 7, 0, 2 };
    CHECK(unpackFront(packFront(7, serial, 2)) == c);
    CHECK(unpackFront(0) == output::FrontFrame{});   // gen 0 = nothing published
}
