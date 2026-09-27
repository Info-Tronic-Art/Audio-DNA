// SharedFrameSet -- the GL half (the main context is the only writer). The IOSurface half is SurfacePool.cpp.
// Protocol: .harmony/.reports/s-rta-0926b/plan5-final.md section 8.1; nothing else touches front_.
#include "output/SharedFrameSet.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#if JUCE_MAC
 #include <OpenGL/OpenGL.h>
 #include <OpenGL/CGLIOSurface.h>
#endif
#include <iostream>

using namespace juce::gl;

namespace output
{
void SharedFrameSet::publish(unsigned int canvasFBO, int w, int h)
{
#if JUCE_MAC
    const bool newGen = pool_.ensure(w, h);
    pool_.tick();
    if (pool_.generation() == 0 || pool_.width() != w || pool_.height() != h)
        return;   // no surfaces for this size (creation failed): the outputs keep the last frame

    if (newGen || boundGen_ != pool_.generation())
    {
        // First frame, a size change, or a fresh context after releaseGL(). A fence still pending here
        // belongs to THIS (live, current) context -- releaseGL() already dropped any older one -- so it is
        // deleted properly before its slot's objects go.
        if (pendingFence_ != nullptr)
            glDeleteSync(static_cast<GLsync>(pendingFence_));
        pendingFence_ = nullptr;
        releaseGL();
        if (!bindCurrentGeneration())
        {
            releaseGL();
            return;
        }
        boundGen_ = pool_.generation();
        // front_ is NOT touched: readers keep presenting the previous frame (they hold CFRetains) until the
        // first frame of this binding is offered below. Same generation after a context loss: continue the
        // rotation after the slot readers are showing, so it keeps its two frames of grace.
        const FrontFrame f = front();
        write_ = (f.gen == boundGen_) ? f.slot : -1;
    }

    if (pendingFence_ != nullptr)   // last call's copy: complete on the GPU?
    {
        const GLenum r = glClientWaitSync(static_cast<GLsync>(pendingFence_), 0, 0);   // non-blocking poll
        if (r == GL_TIMEOUT_EXPIRED)
            return;   // GPU behind: skip this frame, never two copies in flight
        glDeleteSync(static_cast<GLsync>(pendingFence_));
        pendingFence_ = nullptr;
        if (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED)
        {
            // Only COMPLETED frames are offered.
            frontSize_.store((static_cast<uint64_t>(static_cast<uint32_t>(w)) << 32) | static_cast<uint32_t>(h),
                             std::memory_order_relaxed);
            front_.store(packFront(boundGen_, ++serial_, pendingSlot_), std::memory_order_release);
        }
        // GL_WAIT_FAILED: that copy's state is unknown -- never offer it; continue with a fresh one.
        pendingSlot_ = -1;
    }

    write_ = (write_ + 1) % SurfacePool::kSlots;   // strict round-robin
    glBindFramebuffer(GL_READ_FRAMEBUFFER, canvasFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[write_]);
    // 1:1 copy, RGBA8 canvas -> RGBA8 rect texture on BGRA storage (both fixed-point).
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    pendingFence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    pendingSlot_ = write_;
    glFlush();   // the writer-side rule (CGLIOSurface.h: flush, then the reader re-binds)
    glBindFramebuffer(GL_FRAMEBUFFER, canvasFBO);
#else
    (void) canvasFBO; (void) w; (void) h;
#endif
}

void SharedFrameSet::releaseGL()
{
    for (int i = 0; i < SurfacePool::kSlots; ++i)
    {
        if (fbo_[i] != 0) { glDeleteFramebuffers(1, &fbo_[i]); fbo_[i] = 0; }
        if (tex_[i] != 0) { glDeleteTextures(1, &tex_[i]); tex_[i] = 0; }
    }
    pendingFence_ = nullptr;   // dropped, never glDeleteSync'd: a sync object dies with its context
    pendingSlot_ = -1;
    boundGen_ = 0;
}

bool SharedFrameSet::bindCurrentGeneration()
{
#if JUCE_MAC
    const int w = pool_.width(), h = pool_.height();
    CGLContextObj cgl = CGLGetCurrentContext();
    if (cgl == nullptr)
        return false;
    glGenTextures(SurfacePool::kSlots, tex_);
    glGenFramebuffers(SurfacePool::kSlots, fbo_);
    bool ok = true;
    for (int i = 0; i < SurfacePool::kSlots && ok; ++i)
    {
        glBindTexture(GL_TEXTURE_RECTANGLE, tex_[i]);
        const CGLError err = CGLTexImageIOSurface2D(cgl, GL_TEXTURE_RECTANGLE, GL_RGBA8, w, h, GL_BGRA,
                                                    GL_UNSIGNED_INT_8_8_8_8_REV, pool_.surface(i), 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        if (err != kCGLNoError)
        {
            std::cerr << "[SharedFrameSet] CGLTexImageIOSurface2D failed: " << CGLErrorString(err) << std::endl;
            ok = false;
            break;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, tex_[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "[SharedFrameSet] slot FBO incomplete" << std::endl;
            ok = false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return ok;
#else
    return false;
#endif
}
} // namespace output
