// SurfacePool -- the IOSurface half of output::SharedFrameSet (declared in SharedFrameSet.h). No GL, so the
// pool is unit-tested headless (tests/test_surface_pool.cpp). macOS only; elsewhere every call is a no-op
// and the outputs stay black (plan5-final.md 8.7).
#include "output/SharedFrameSet.h"

#if defined(__APPLE__)
 #include <CoreFoundation/CoreFoundation.h>
#endif

namespace output
{
SurfacePool::~SurfacePool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    releaseGeneration(current_);
    for (auto& g : retired_)
        releaseGeneration(g);
    retired_.clear();
}

void SurfacePool::releaseGeneration(Generation& g)
{
#if defined(__APPLE__)
    for (auto& s : g.s)
        if (s != nullptr) { CFRelease(s); s = nullptr; }
#endif
    g = Generation{};
}

#if defined(__APPLE__)
namespace
{
IOSurfaceRef createSurface(int w, int h)
{
    const int32_t width = w, height = h, bytesPerElement = 4;
    const uint32_t pixelFormat = 'BGRA';   // kCVPixelFormatType_32BGRA (SyphonServerBase.m:252-258)
    CFNumberRef nw = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &width);
    CFNumberRef nh = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &height);
    CFNumberRef nb = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bytesPerElement);
    CFNumberRef nf = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixelFormat);
    const void* keys[] = { kIOSurfaceWidth, kIOSurfaceHeight, kIOSurfaceBytesPerElement, kIOSurfacePixelFormat };
    const void* values[] = { nw, nh, nb, nf };
    CFDictionaryRef props = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 4,
                                               &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    IOSurfaceRef s = IOSurfaceCreate(props);
    CFRelease(props);
    CFRelease(nw); CFRelease(nh); CFRelease(nb); CFRelease(nf);
    return s;
}
} // namespace
#endif

bool SurfacePool::ensure(int w, int h)
{
#if defined(__APPLE__)
    if (w <= 0 || h <= 0)
        return false;
    if (current_.gen != 0 && current_.w == w && current_.h == h)
        return false;

    Generation next;
    next.w = w;
    next.h = h;
    for (auto& s : next.s)
    {
        s = createSurface(w, h);
        if (s == nullptr)
        {
            releaseGeneration(next);   // keep the current generation; publish() skips a size it cannot serve
            return false;
        }
    }
    // 24-bit generation counter (FrontFrame packing), never 0.
    next.gen = (current_.gen % 0xFFFFFFu) + 1u;

    std::lock_guard<std::mutex> lock(mutex_);
    if (current_.gen != 0)
    {
        current_.age = 0;
        retired_.push_back(current_);
        retiredCount_.store(static_cast<int>(retired_.size()), std::memory_order_relaxed);
    }
    current_ = next;
    return true;
#else
    (void) w; (void) h;
    return false;
#endif
}

void SurfacePool::tick()
{
    if (retiredCount_.load(std::memory_order_relaxed) == 0)
        return;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = retired_.begin(); it != retired_.end();)
    {
        if (++it->age >= kRetireFrames)
        {
            releaseGeneration(*it);
            it = retired_.erase(it);
        }
        else
            ++it;
    }
    retiredCount_.store(static_cast<int>(retired_.size()), std::memory_order_relaxed);
}

IOSurfaceRef SurfacePool::surface(int slot) const noexcept
{
    return (slot >= 0 && slot < kSlots) ? current_.s[slot] : nullptr;
}

IOSurfaceRef SurfacePool::retainSurface(uint32_t gen, int slot)
{
#if defined(__APPLE__)
    if (gen == 0 || slot < 0 || slot >= kSlots)
        return nullptr;
    std::lock_guard<std::mutex> lock(mutex_);
    const Generation* g = nullptr;
    if (current_.gen == gen)
        g = &current_;
    else
        for (const auto& r : retired_)
            if (r.gen == gen) { g = &r; break; }
    if (g == nullptr || g->s[slot] == nullptr)
        return nullptr;
    CFRetain(g->s[slot]);
    return g->s[slot];
#else
    (void) gen; (void) slot;
    return nullptr;
#endif
}
} // namespace output
