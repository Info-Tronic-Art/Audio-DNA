#!/usr/bin/env python3
"""patch_app2.py -- DIAG-VFPS TEMPORARY round 2: the c4a IOSurface arm (ADNA_VFPS_IOSURF=1) -- each ring slot is an
IOSurface (BGRA) written by sws on the decode thread, bound ONCE to a GL_TEXTURE_RECTANGLE (CGLTexImageIOSurface2D), and a
new frame is a GPU blit (glBlitFramebuffer) into the player's GL_TEXTURE_2D; the slot is released when the blit's fence
signals. Prices a zero-copy upload (software decode kept). Also the UPCAP arm lives in patch round 1b (applied inline)."""
W = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps"

def patch(path, old, new):
    p = W + "/" + path
    s = open(p, newline="").read()
    assert s.count(old) == 1, (path, s.count(old), old[:70])
    open(p, "w", newline="").write(s.replace(old, new))
    print("patched", path)

patch("src/diag/DiagVfps.h", "        int upcap = 0;        // Q1: at most N video uploads per render frame (a player over the cap holds one more frame)\n",
      "        int upcap = 0;        // Q1: at most N video uploads per render frame (a player over the cap holds one more frame)\n"
      "        int iosurf = 0;       // c4a: IOSurface ring slots + CGLTexImageIOSurface2D rectangle textures + a GPU blit per new frame\n")
patch("src/diag/DiagVfps.h", "    extern std::atomic<int> swapInterval;\n",
      "    extern std::atomic<int> swapInterval;\n\n"
      "    // c4a helpers (IOSurface / CGL live in the .mm): a BGRA IOSurface, its lock, and its binding to a rectangle texture.\n"
      "    void* surfCreate(int w, int h, int* bytesPerRow, uint8_t** base);\n"
      "    void surfLock(void* s);\n    void surfUnlock(void* s);\n    void surfRelease(void* s);\n"
      "    int surfBindRect(void* s, unsigned tex, int w, int h);   // 0 = kCGLNoError\n")
patch("src/diag/DiagVfps.mm", '    theArms.upcap = envInt("ADNA_VFPS_UPCAP");\n',
      '    theArms.upcap = envInt("ADNA_VFPS_UPCAP");\n    theArms.iosurf = envInt("ADNA_VFPS_IOSURF");\n')
patch("src/diag/DiagVfps.mm", "\\tupcap=%d\\n\", tb.numer", "\\tupcap=%d\\tiosurf=%d\\n\", tb.numer")
patch("src/diag/DiagVfps.mm", "(int) qos_class_self(), theArms.upcap);\n", "(int) qos_class_self(), theArms.upcap, theArms.iosurf);\n")
patch("src/diag/DiagVfps.mm", "#include <CoreVideo/CoreVideo.h>\n",
      "#include <CoreVideo/CoreVideo.h>\n#include <IOSurface/IOSurface.h>\n#include <OpenGL/OpenGL.h>\n#include <OpenGL/CGLIOSurface.h>\n#include <OpenGL/gl3.h>\n")
patch("src/diag/DiagVfps.mm", "void stopAll()\n{\n", """void* surfCreate(int w, int h, int* bytesPerRow, uint8_t** base)
{
    NSDictionary* props = @{ (id) kIOSurfaceWidth: @(w), (id) kIOSurfaceHeight: @(h), (id) kIOSurfaceBytesPerElement: @4,
                             (id) kIOSurfacePixelFormat: @((unsigned) 'BGRA') };
    IOSurfaceRef s = IOSurfaceCreate((CFDictionaryRef) props);
    if (s == nullptr)
        return nullptr;
    *bytesPerRow = (int) IOSurfaceGetBytesPerRow(s);
    IOSurfaceLock(s, 0, nullptr);
    *base = (uint8_t*) IOSurfaceGetBaseAddress(s);
    IOSurfaceUnlock(s, 0, nullptr);
    return (void*) s;
}
void surfLock(void* s) { IOSurfaceLock((IOSurfaceRef) s, 0, nullptr); }
void surfUnlock(void* s) { IOSurfaceUnlock((IOSurfaceRef) s, 0, nullptr); }
void surfRelease(void* s) { if (s != nullptr) CFRelease((IOSurfaceRef) s); }
int surfBindRect(void* s, unsigned tex, int w, int h)
{
    glBindTexture(GL_TEXTURE_RECTANGLE, tex);
    const CGLError e = CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE, GL_RGBA8, w, h, GL_BGRA,
                                              GL_UNSIGNED_INT_8_8_8_8_REV, (IOSurfaceRef) s, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    return (int) e;
}

void stopAll()
{
""")

H = "src/media/VideoPlayer.h"
patch(H, "    GLuint dvfUpload(const uint8_t* bytes, int slot, bool create);   // DIAG-VFPS TEMPORARY\n",
      "    GLuint dvfUpload(const uint8_t* bytes, int slot, bool create);   // DIAG-VFPS TEMPORARY\n"
      "    void* dvfSurf_[3] = { nullptr, nullptr, nullptr };               // DIAG-VFPS TEMPORARY (c4a)\n"
      "    GLuint dvfRectTex_[3] = { 0, 0, 0 };\n    GLuint dvfRectFbo_[3] = { 0, 0, 0 };\n    GLuint dvfDstFbo_ = 0;\n"
      "    GLsync dvfFence_[3] = { nullptr, nullptr, nullptr };\n    void dvfPollFences();\n")

V = "src/media/VideoPlayer.cpp"
patch(V, """    close();
    thread_.stopThread(3000);
    freeFfmpeg();
    for (auto*& s : slotBytes_)
    {""", """    close();
    thread_.stopThread(3000);
    freeFfmpeg();
    for (int i = 0; i < 3; ++i)   // DIAG-VFPS TEMPORARY (c4a): the slots are IOSurfaces, never malloc'd
        if (dvfSurf_[i] != nullptr) { dvf::surfRelease(dvfSurf_[i]); dvfSurf_[i] = nullptr; slotBytes_[static_cast<size_t>(i)] = nullptr; }
    for (auto*& s : slotBytes_)
    {""")
patch(V, "    auto dstFmt = (dvf::arms().bgra != 0 || dvf::arms().client != 0) ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA;   // DIAG-VFPS TEMPORARY (c2/c3)\n",
      "    auto dstFmt = (dvf::arms().bgra != 0 || dvf::arms().client != 0 || dvf::arms().iosurf != 0) ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA;   // DIAG-VFPS TEMPORARY (c2/c3/c4a)\n")
patch(V, """    for (auto*& s : slotBytes_)
    {
        s = static_cast<uint8_t*>(std::malloc(slotSize));""", """    if (dvf::arms().iosurf != 0)   // DIAG-VFPS TEMPORARY (c4a): IOSurface slots
    {
        for (int i = 0; i < 3; ++i)
        {
            int bpr = 0; uint8_t* base = nullptr;
            dvfSurf_[i] = dvf::surfCreate(width_, height_, &bpr, &base);
            if (dvfSurf_[i] == nullptr || bpr < rowBytes_) { std::cerr << "[DIAG-VFPS] IOSurface create failed" << std::endl; freeFfmpeg(); return false; }
            slotBytes_[static_cast<size_t>(i)] = base;
            rowBytes_ = bpr;
        }
    }
    else
    for (auto*& s : slotBytes_)
    {
        s = static_cast<uint8_t*>(std::malloc(slotSize));""")
patch(V, """    int dstStride[4] = { -rowBytes_, 0, 0, 0 };
    sws_scale(swsCtx_, decodedFrame_->data, decodedFrame_->linesize, 0, height_, dst, dstStride);
}""", """    int dstStride[4] = { -rowBytes_, 0, 0, 0 };
    if (dvfSurf_[slot] != nullptr) dvf::surfLock(dvfSurf_[slot]);   // DIAG-VFPS TEMPORARY (c4a)
    sws_scale(swsCtx_, decodedFrame_->data, decodedFrame_->linesize, 0, height_, dst, dstStride);
    if (dvfSurf_[slot] != nullptr) dvf::surfUnlock(dvfSurf_[slot]);
}""")
patch(V, """        lastShownPts_ = p.pts;
        if (dvf::arms().client != 0)""", """        lastShownPts_ = p.pts;
        if (dvf::arms().iosurf != 0)   // DIAG-VFPS TEMPORARY (c4a): released when the blit's fence signals (dvfPollFences)
        {
        }
        else if (dvf::arms().client != 0)""")
patch(V, """    // DIAG-VFPS TEMPORARY (Q1 counterfactual ADNA_VFPS_UPCAP): over this frame's upload cap -> hold (pick next frame).""",
      """    if (dvf::arms().iosurf != 0)   // DIAG-VFPS TEMPORARY (c4a)
        dvfPollFences();
    // DIAG-VFPS TEMPORARY (Q1 counterfactual ADNA_VFPS_UPCAP): over this frame's upload cap -> hold (pick next frame).""")
patch(V, """    if (dvfPbo_[0] != 0) { glDeleteBuffers(2, dvfPbo_); dvfPbo_[0] = dvfPbo_[1] = 0; }   // DIAG-VFPS TEMPORARY
""", """    if (dvfPbo_[0] != 0) { glDeleteBuffers(2, dvfPbo_); dvfPbo_[0] = dvfPbo_[1] = 0; }   // DIAG-VFPS TEMPORARY
    for (int i = 0; i < 3; ++i)   // DIAG-VFPS TEMPORARY (c4a)
    {
        if (dvfFence_[i] != nullptr) { glDeleteSync(dvfFence_[i]); dvfFence_[i] = nullptr; ring_.release(i); }
        if (dvfRectFbo_[i] != 0) { glDeleteFramebuffers(1, &dvfRectFbo_[i]); dvfRectFbo_[i] = 0; }
        if (dvfRectTex_[i] != 0) { glDeleteTextures(1, &dvfRectTex_[i]); dvfRectTex_[i] = 0; }
    }
    if (dvfDstFbo_ != 0) { glDeleteFramebuffers(1, &dvfDstFbo_); dvfDstFbo_ = 0; }
""")
patch(V, """    if (a.client != 0)
    {
        GLuint& t = dvfClientTex_[slot];""", """    if (a.iosurf != 0)
    {
        GLint rd = 0, dr = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &rd);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dr);
        if (dvfRectTex_[slot] == 0)
        {
            glGenTextures(1, &dvfRectTex_[slot]);
            const int e = dvf::surfBindRect(dvfSurf_[slot], dvfRectTex_[slot], width_, height_);
            if (e != 0) { dvf::rec(16, (double) dvf::now(), 1000.0 + e); std::cerr << "[DIAG-VFPS] CGLTexImageIOSurface2D " << e << std::endl; }
            glGenFramebuffers(1, &dvfRectFbo_[slot]);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, dvfRectFbo_[slot]);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, dvfRectTex_[slot], 0);
        }
        if (create)
        {
            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);
            params();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
            glGenFramebuffers(1, &dvfDstFbo_);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dvfDstFbo_);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, dvfRectFbo_[slot]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dvfDstFbo_);
        glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(rd));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(dr));
        if (dvfFence_[slot] != nullptr) glDeleteSync(dvfFence_[slot]);
        dvfFence_[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        logErr("iosurf");
        return texture_;
    }
    if (a.client != 0)
    {
        GLuint& t = dvfClientTex_[slot];""")
p = W + "/" + V
s = open(p, newline="").read()
s += r'''
// DIAG-VFPS TEMPORARY (c4a): a slot whose blit has completed on the GPU goes back to the writer.
void VideoPlayer::dvfPollFences()
{
    for (int i = 0; i < 3; ++i)
    {
        if (dvfFence_[i] == nullptr)
            continue;
        const GLenum r = glClientWaitSync(dvfFence_[i], 0, 0);
        if (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED)
        {
            glDeleteSync(dvfFence_[i]);
            dvfFence_[i] = nullptr;
            ring_.release(i);
            releasedThisFrame_ = true;
        }
    }
}
'''
open(p, "w", newline="").write(s)
print("appended dvfPollFences")
