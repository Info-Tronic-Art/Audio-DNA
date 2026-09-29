#!/usr/bin/env python3
"""patch_app4.py -- DIAG-VFPS TEMPORARY round 4 (after patch_app3.py): ADNA_VFPS_IOSURF=2 = TRUE zero-copy for the GL
side: each IOSurface ring slot is bound ONCE as a GL_TEXTURE_2D (CGLTexImageIOSurface2D) and that texture IS the
player's texture -- no upload, no blit. The shown slot is held; when a newer slot is picked, a fence is placed and the
old slot goes back to the writer when the fence signals (every draw that sampled it has completed)."""
W = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps"
def patch(path, old, new):
    p = W + "/" + path; s = open(p, newline="").read()
    assert s.count(old) == 1, (path, s.count(old), old[:60])
    open(p, "w", newline="").write(s.replace(old, new)); print("patched", path)
patch("src/diag/DiagVfps.h", "    int surfBindRect(void* s, unsigned tex, int w, int h);   // 0 = kCGLNoError\n",
      "    int surfBindRect(void* s, unsigned tex, int w, int h);   // 0 = kCGLNoError\n"
      "    int surfBind2D(void* s, unsigned tex, int w, int h);     // GL_TEXTURE_2D target (IOSURF=2); 0 = kCGLNoError\n")
patch("src/diag/DiagVfps.mm", "\nvoid stopAll()\n{\n", """int surfBind2D(void* s, unsigned tex, int w, int h)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    const CGLError e = CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_2D, GL_RGBA8, w, h, GL_BGRA,
                                              GL_UNSIGNED_INT_8_8_8_8_REV, (IOSurfaceRef) s, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return (int) e;
}

void stopAll()
{
""")
patch("src/media/VideoPlayer.h", "    GLsync dvfFence_[3] = { nullptr, nullptr, nullptr };\n",
      "    GLsync dvfFence_[3] = { nullptr, nullptr, nullptr };\n    int dvfShownSlot_ = -1;   // IOSURF=2: the slot whose texture is on screen\n")
patch("src/media/VideoPlayer.cpp", """    if (a.iosurf != 0)
    {
        GLint rd = 0, dr = 0;""", """    if (a.iosurf == 2)   // DIAG-VFPS TEMPORARY: the slot's IOSurface IS the texture (no upload, no blit)
    {
        if (dvfRectTex_[slot] == 0)
        {
            glGenTextures(1, &dvfRectTex_[slot]);
            const int e = dvf::surfBind2D(dvfSurf_[slot], dvfRectTex_[slot], width_, height_);
            if (e != 0) { dvf::rec(16, (double) dvf::now(), 2000.0 + e); std::cerr << "[DIAG-VFPS] CGLTexImageIOSurface2D(2D) " << e << std::endl; }
        }
        if (dvfShownSlot_ >= 0 && dvfShownSlot_ != slot)
        {
            if (dvfFence_[dvfShownSlot_] != nullptr) glDeleteSync(dvfFence_[dvfShownSlot_]);
            dvfFence_[dvfShownSlot_] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);   // released by dvfPollFences
        }
        dvfShownSlot_ = slot;
        logErr("iosurf2");
        return dvfRectTex_[slot];
    }
    if (a.iosurf != 0)
    {
        GLint rd = 0, dr = 0;""")
# releaseGL: texture_ aliases a slot texture under IOSURF=2
patch("src/media/VideoPlayer.cpp", """    for (int i = 0; i < 3; ++i)   // DIAG-VFPS TEMPORARY (c4a)
    {""", """    if (dvf::arms().iosurf == 2) { texture_ = 0; if (dvfShownSlot_ >= 0) ring_.release(dvfShownSlot_); dvfShownSlot_ = -1; }   // DIAG-VFPS TEMPORARY
    for (int i = 0; i < 3; ++i)   // DIAG-VFPS TEMPORARY (c4a)
    {""")
