#pragma once

// OutputPresenter: presents the newest completed shared frame (SharedFrameSet::front()) into a framebuffer
// of THIS context, letter/pillar-boxed to the composition's shape (RenderGeometry::fitCanvas). One code path
// for the output window, the offscreen ctest (tests/test_shared_frame_gl.cpp) and the in-app probe
// (TestServer /api/output_probe) -- s-rta-0927 outputs-c1, .harmony/.reports/s-rta-0926b/plan5-final.md 8.2.
//
// Per context, never shared: this context's rect textures bound to the front generation's surfaces (each
// CFRetain'd while bound, so a retired generation outlives the pool's own release), and one read FBO per slot.

#include "output/SharedFrameSet.h"

namespace output
{
struct PresenterGLState
{
    unsigned int tex[SurfacePool::kSlots]{}, readFBO[SurfacePool::kSlots]{};
    IOSurfaceRef held[SurfacePool::kSlots]{};
    uint32_t boundGen = 0;
    int w = 0, h = 0;   // the bound generation's size

    // Context current: deletes the GL objects, CFReleases the held surfaces, boundGen = 0.
    void release();
};

// Presents frames.front() into targetFBO (0 = this context's window framebuffer) of targetW x targetH
// pixels: clears it black, (re)binds this context's textures when the front generation changed,
// re-binds the slot's read FBO (the reader-side rule of CGLIOSurface.h) and blits the frame into
// RenderGeometry::fitCanvas(W, H, targetW, targetH), bilinear. Returns false (target left black) when
// nothing has been published yet or that generation's surfaces are gone. Leaves targetFBO bound.
bool presentSharedFrame(SharedFrameSet& frames, PresenterGLState& st, unsigned int targetFBO, int targetW, int targetH);
} // namespace output
