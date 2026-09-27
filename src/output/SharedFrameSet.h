#pragma once

// SharedFrameSet: the composition canvas, copied once per frame into IOSurface-backed slots that every
// output window presents (s-rta-0927 outputs-c1 = plan5 slice C1, .harmony/.reports/s-rta-0926b/
// plan5-final.md section 8.1).
//
// Why IOSurface and not a GL share group: the main GL context dies whenever the preview panel hides or
// the app minimises. An IOSurface is owned by the app, not by a context -- the outputs keep presenting
// the last published frame (a frozen picture, never black) and the main context rebinds the SAME
// surfaces when it returns. No GL object name is ever shared between contexts.
//
// Two halves:
//   SurfacePool     -- the IOSurfaces (no GL). Writer-owned; readers only ever call retainSurface().
//   SharedFrameSet  -- the writer's GL objects (main context) + the atomic "front" frame readers present.
// All of JUCE's GL contexts render on ONE shared thread; the frame path needs atomics only. The pool's
// mutex is taken on a generation change, by retainSurface() (a reader rebinding after a generation
// change) and by tick() only while a retired generation is ageing -- never on the steady-state frame path.
//
// This header is GL-free on purpose (GL names are unsigned int, the fence an opaque pointer) so the
// pool can be unit-tested without a GL context (tests/test_surface_pool.cpp).

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

#if defined(__APPLE__)
 #include <IOSurface/IOSurfaceRef.h>
#else
typedef struct __IOSurface* IOSurfaceRef;
#endif

namespace output
{
// The newest COMPLETED frame. gen 0 = nothing published yet.
struct FrontFrame
{
    uint32_t gen = 0, serial = 0;
    int slot = 0;
};

inline bool operator==(const FrontFrame& a, const FrontFrame& b) noexcept
{
    return a.gen == b.gen && a.serial == b.serial && a.slot == b.slot;
}

// One 64-bit word: (gen << 40) | (serial << 8) | slot -- gen 24 bits, serial 32 bits, slot 8 bits.
constexpr uint64_t packFront(uint32_t gen, uint32_t serial, int slot) noexcept
{
    return (static_cast<uint64_t>(gen & 0xFFFFFFu) << 40) | (static_cast<uint64_t>(serial) << 8)
         | static_cast<uint64_t>(static_cast<uint8_t>(slot));
}

constexpr FrontFrame unpackFront(uint64_t v) noexcept
{
    return { static_cast<uint32_t>(v >> 40) & 0xFFFFFFu, static_cast<uint32_t>(v >> 8),
             static_cast<int>(v & 0xFFu) };
}

// ---- SurfacePool: the IOSurface half. NO GL. ----
class SurfacePool
{
public:
    // Grace between "no longer front" and "rewritten" = kSlots - 2 main frames (= 2).
    static constexpr int kSlots = 4;
    // A reader may still be bound to a replaced generation for a frame or two; it is released this many
    // publishes after it was replaced (a reader's own CFRetain keeps it alive beyond that if needed).
    static constexpr int kRetireFrames = 120;

    SurfacePool() = default;
    ~SurfacePool();
    SurfacePool(const SurfacePool&) = delete;
    SurfacePool& operator=(const SurfacePool&) = delete;

    // New size (or first call): a new generation of kSlots surfaces ('BGRA', 4 bytes per element); the old
    // generation is retired. Returns true iff a new generation was created. No-op when unchanged, when
    // either side is <= 0, and when the surfaces cannot be created (the current generation stays).
    bool ensure(int w, int h);

    // Once per publish: retired generations age; each is released kRetireFrames ticks after it retired.
    void tick();

    // Writer thread only.
    uint32_t generation() const noexcept { return current_.gen; }
    int width() const noexcept { return current_.w; }
    int height() const noexcept { return current_.h; }
    IOSurfaceRef surface(int slot) const noexcept;   // current generation, NOT retained

    // Any thread: CFRetain'd surface of generation `gen`, or nullptr (unknown / released generation, bad
    // slot). The caller CFReleases it.
    IOSurfaceRef retainSurface(uint32_t gen, int slot);

private:
    struct Generation
    {
        uint32_t gen = 0;
        int w = 0, h = 0;
        IOSurfaceRef s[kSlots]{};
        int age = 0;
    };
    static void releaseGeneration(Generation& g);

    mutable std::mutex mutex_;
    std::atomic<int> retiredCount_{ 0 };
    Generation current_;
    std::vector<Generation> retired_;
};

// ---- SharedFrameSet: the GL half. The main context is the only writer. ----
class SharedFrameSet
{
public:
    SharedFrameSet() = default;
    SharedFrameSet(const SharedFrameSet&) = delete;
    SharedFrameSet& operator=(const SharedFrameSet&) = delete;

    // Main GL thread, once per frame, with the canvas final: copies the canvas (canvasFBO, w x h) into the
    // next slot and offers the PREVIOUS call's slot once its fence has completed (+1 frame of latency;
    // never two copies in flight). Leaves canvasFBO bound as GL_FRAMEBUFFER.
    void publish(unsigned int canvasFBO, int w, int h);

    // Main context closing (context current): deletes THIS context's textures/FBOs and DROPS the pending
    // fence without glDeleteSync (a sync object dies with its context). front() and the pool survive: the
    // outputs keep presenting the last frame; the next publish() rebinds the same surfaces.
    void releaseGL();

    // Any thread: one acquire load.
    FrontFrame front() const noexcept { return unpackFront(front_.load(std::memory_order_acquire)); }
    // Any thread: the size of the last offered frame (0 x 0 before the first) -- stats only.
    int frontWidth() const noexcept { return static_cast<int>(frontSize_.load(std::memory_order_relaxed) >> 32); }
    int frontHeight() const noexcept { return static_cast<int>(frontSize_.load(std::memory_order_relaxed) & 0xFFFFFFFFu); }

    SurfacePool& pool() noexcept { return pool_; }

private:
    bool bindCurrentGeneration();   // rect textures on the pool's surfaces + one FBO per slot

    SurfacePool pool_;
    std::atomic<uint64_t> front_{ 0 };
    std::atomic<uint64_t> frontSize_{ 0 };
    unsigned int tex_[SurfacePool::kSlots]{}, fbo_[SurfacePool::kSlots]{};
    uint32_t boundGen_ = 0;            // generation this context's tex_/fbo_ are bound to (0 = none)
    void* pendingFence_ = nullptr;     // GLsync of the last blit, not yet offered
    int pendingSlot_ = -1;
    int write_ = -1;                   // last slot written (strict round-robin)
    uint32_t serial_ = 0;
};
} // namespace output
