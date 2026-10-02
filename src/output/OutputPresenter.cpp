#include "output/OutputPresenter.h"
#include "core/LogLine.h"
#include "render/RenderGeometry.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#if JUCE_MAC
 #include <OpenGL/OpenGL.h>
 #include <OpenGL/CGLIOSurface.h>
 #include <CoreFoundation/CoreFoundation.h>
#endif
#include <iostream>

using namespace juce::gl;

namespace output
{
void PresenterGLState::release()
{
    for (int i = 0; i < SurfacePool::kSlots; ++i)
    {
        if (readFBO[i] != 0) { glDeleteFramebuffers(1, &readFBO[i]); readFBO[i] = 0; }
        if (tex[i] != 0) { glDeleteTextures(1, &tex[i]); tex[i] = 0; }
#if JUCE_MAC
        if (held[i] != nullptr) { CFRelease(held[i]); held[i] = nullptr; }
#endif
    }
    boundGen = 0;
    w = h = 0;
}

namespace
{
#if JUCE_MAC
// Binds this context's rect textures + read FBOs to generation `gen`. False = that generation is gone.
bool bindGeneration(SharedFrameSet& frames, PresenterGLState& st, uint32_t gen)
{
    st.release();
    for (int i = 0; i < SurfacePool::kSlots; ++i)
    {
        st.held[i] = frames.pool().retainSurface(gen, i);
        if (st.held[i] == nullptr)
        {
            st.release();
            return false;
        }
    }
    st.w = static_cast<int>(IOSurfaceGetWidth(st.held[0]));
    st.h = static_cast<int>(IOSurfaceGetHeight(st.held[0]));
    CGLContextObj cgl = CGLGetCurrentContext();
    if (cgl == nullptr || st.w <= 0 || st.h <= 0)
    {
        st.release();
        return false;
    }
    glGenTextures(SurfacePool::kSlots, st.tex);
    glGenFramebuffers(SurfacePool::kSlots, st.readFBO);
    for (int i = 0; i < SurfacePool::kSlots; ++i)
    {
        glBindTexture(GL_TEXTURE_RECTANGLE, st.tex[i]);
        const CGLError err = CGLTexImageIOSurface2D(cgl, GL_TEXTURE_RECTANGLE, GL_RGBA8, st.w, st.h, GL_BGRA,
                                                    GL_UNSIGNED_INT_8_8_8_8_REV, st.held[i], 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, st.readFBO[i]);
        if (err == kCGLNoError)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, st.tex[i], 0);
        if (err != kCGLNoError || glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            logLine("[OutputPresenter] cannot bind shared frame slot ", i);
            st.release();
            return false;
        }
    }
    st.boundGen = gen;
    return true;
}
#endif
} // namespace

bool presentSharedFrame(SharedFrameSet& frames, PresenterGLState& st, unsigned int targetFBO, int targetW, int targetH)
{
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glViewport(0, 0, targetW, targetH);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

#if JUCE_MAC
    const FrontFrame f = frames.front();
    if (f.gen == 0 || targetW <= 0 || targetH <= 0)
        return false;
    if (st.boundGen != f.gen && !bindGeneration(frames, st, f.gen))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
        return false;
    }

    const auto r = RenderGeometry::fitCanvas(st.w, st.h, targetW, targetH);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, st.readFBO[f.slot]);   // re-bind every frame: the reader-side rule
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, st.w, st.h, r.x, r.y, r.x + r.w, r.y + r.h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    return true;
#else
    (void) frames; (void) st;
    static bool logged = false;
    if (!logged) { logged = true; logLine("[OutputPresenter] shared frames are macOS-only: output stays black"); }
    return false;
#endif
}
} // namespace output
