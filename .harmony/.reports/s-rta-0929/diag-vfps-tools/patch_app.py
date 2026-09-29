#!/usr/bin/env python3
"""patch_app.py -- DIAG-VFPS TEMPORARY instrumentation of the worktree (Renderer.cpp, VideoPlayer.{h,cpp}).
Every inserted line carries DIAG-VFPS or dvf:: (strings check: DIAG-VFPS / ADNA_VFPS_)."""
W = "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps"

def patch(path, old, new):
    p = W + "/" + path
    s = open(p, newline="").read()
    assert s.count(old) == 1, (path, s.count(old), old[:70])
    open(p, "w", newline="").write(s.replace(old, new))
    print("patched", path)

R = "src/render/Renderer.cpp"
patch(R, '#include <random>\n', '#include <random>\n#include "diag/DiagVfps.h"   // DIAG-VFPS TEMPORARY\n#include <pthread.h>\n#include <sys/qos.h>\n')
patch(R, "    } callbackPeak{ peakCallbackMs_ };\n",
"""    } callbackPeak{ peakCallbackMs_ };

    // DIAG-VFPS TEMPORARY: one record set per frame (every return path).
    struct DvfFrameScope
    {
        uint64_t t0 = dvf::now();
        int c0 = dvf::cpu();
        uint64_t tRs = 0, tComp = 0, tChain = 0, tPres = 0;
        ~DvfFrameScope()
        {
            if (!dvf::on()) return;
            const auto& f = dvf::frame();
            dvf::rec(10, (double) t0, (double) dvf::now(), (double) c0, (double) dvf::cpu(), 0.0);
            dvf::rec(11, (double) t0, (double) f.uploadN, (double) f.uploadTicks, (double) f.videoSyncTicks, (double) f.memcpyTicks);
            dvf::rec(12, (double) t0, (double) tRs, (double) tComp, (double) tChain, (double) tPres);
        }
    } dvfScope;
    dvf::frame() = dvf::Frame{};
    {
        static bool dvfQosDone = false;   // DIAG-VFPS TEMPORARY: the H2 counterfactual (ADNA_VFPS_QOS)
        if (!dvfQosDone)
        {
            dvfQosDone = true;
            if (dvf::arms().qos == 1) pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
            else if (dvf::arms().qos == 2) pthread_set_qos_class_self_np(QOS_CLASS_UTILITY, 0);
            dvf::rec(15, (double) dvf::now(), (double) qos_class_self(), (double) dvf::arms().qos);
        }
        if ((frameCount_ & 127) == 0)
            dvf::swapInterval.store(glContext_.getSwapInterval());
    }
""")
patch(R, "    auto renderStart = std::chrono::high_resolution_clock::now();\n",
      "    auto renderStart = std::chrono::high_resolution_clock::now();\n    dvfScope.tRs = dvf::now();   // DIAG-VFPS TEMPORARY\n")
patch(R, "    if (sourceTexture == 0 && sourceActive)\n    {\n        // Render the procedural source to get a texture.",
      "    dvfScope.tComp = dvf::now();   // DIAG-VFPS TEMPORARY\n    if (sourceTexture == 0 && sourceActive)\n    {\n        // Render the procedural source to get a texture.")
patch(R, "    publishToOutputs(canvas.w, canvas.h);\n\n    // plan4 item 1: the panel shows the finished canvas",
      "    dvfScope.tChain = dvf::now();   // DIAG-VFPS TEMPORARY\n    publishToOutputs(canvas.w, canvas.h);\n\n    // plan4 item 1: the panel shows the finished canvas")
patch(R, "    presentCanvas(static_cast<GLuint>(defaultFBO), present);\n    gpuTimer.finish();\n",
      "    presentCanvas(static_cast<GLuint>(defaultFBO), present);\n    gpuTimer.finish();\n    dvfScope.tPres = dvf::now();   // DIAG-VFPS TEMPORARY\n")
patch(R, "        const float ms = static_cast<float>(static_cast<double>(ns) / 1.0e6);\n",
      "        const float ms = static_cast<float>(static_cast<double>(ns) / 1.0e6);\n        dvf::rec(14, (double) dvf::now(), (double) ns / 1.0e6, (double) gpuQueryFrame_);   // DIAG-VFPS TEMPORARY\n")
patch(R, "    if (clip->mediaType == Clip::MediaType::Video)\n    {\n        // s-rta-0928b video (R-10): videoPlayerMutex_ guards the LOOKUP only.",
"""    if (clip->mediaType == Clip::MediaType::Video)
    {
        struct DvfSync { uint64_t t0 = dvf::now(); ~DvfSync() { dvf::frame().videoSyncTicks += dvf::now() - t0; } } dvfSync;   // DIAG-VFPS TEMPORARY
        // s-rta-0928b video (R-10): videoPlayerMutex_ guards the LOOKUP only.""")

H = "src/media/VideoPlayer.h"
patch(H, "    VideoStats* stats_ = nullptr;\n",
"""    VideoStats* stats_ = nullptr;

    // DIAG-VFPS TEMPORARY: counterfactual upload arms (ADNA_VFPS_CLIENT / _PBO).
    GLuint dvfClientTex_[3] = { 0, 0, 0 };
    int dvfHeldSlot_ = -1;
    GLuint dvfPbo_[2] = { 0, 0 };
    int dvfPboIdx_ = 0;
    bool dvfErrLogged_ = false;
    GLuint dvfUpload(const uint8_t* bytes, int slot, bool create);   // DIAG-VFPS TEMPORARY
""")

V = "src/media/VideoPlayer.cpp"
patch(V, '#include "VideoPlayer.h"\n', '#include "VideoPlayer.h"\n#include "diag/DiagVfps.h"   // DIAG-VFPS TEMPORARY\n#include <pthread.h>\n#include <sys/qos.h>\n')
patch(V, "    auto dstFmt = AV_PIX_FMT_RGBA;\n",
      "    auto dstFmt = (dvf::arms().bgra != 0 || dvf::arms().client != 0) ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA;   // DIAG-VFPS TEMPORARY (c2/c3)\n")
# upload block: replace the create/sub-image pair with the diag dispatcher
patch(V, """            const uint8_t* bytes = slotBytes_[static_cast<size_t>(p.slot)];
            if (!textureCreated_)
            {
                glGenTextures(1, &texture_);
                glBindTexture(GL_TEXTURE_2D, texture_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                             width_, height_,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
                textureCreated_ = true;
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, texture_);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                width_, height_,
                                GL_RGBA, GL_UNSIGNED_BYTE, bytes);
            }
""", """            const uint8_t* bytes = slotBytes_[static_cast<size_t>(p.slot)];
            // DIAG-VFPS TEMPORARY: the upload goes through the diag dispatcher (the shipped path when no arm is set).
            const uint64_t dvfT0 = dvf::now();
            texture_ = dvfUpload(bytes, p.slot, !textureCreated_);
            textureCreated_ = true;
            const uint64_t dvfT1 = dvf::now();
            dvf::frame().uploadTicks += dvfT1 - dvfT0;
            ++dvf::frame().uploadN;
            dvf::rec(13, (double) dvfT0, (double) (dvfT1 - dvfT0), (double) (reinterpret_cast<uintptr_t>(this) & 0xffff), (double) width_, 0.0);
""")
patch(V, """        lastShownPts_ = p.pts;
        ring_.release(p.slot);   // client-memory glTex*Image2D copies before it returns: the slot is free now
        releasedThisFrame_ = true;
        return texture_;
""", """        lastShownPts_ = p.pts;
        if (dvf::arms().client != 0)   // DIAG-VFPS TEMPORARY (c3): client storage reads the slot's bytes later -- hold it
        {
            if (dvfHeldSlot_ >= 0 && dvfHeldSlot_ != p.slot)
                ring_.release(dvfHeldSlot_);
            dvfHeldSlot_ = p.slot;
        }
        else
        ring_.release(p.slot);   // client-memory glTex*Image2D copies before it returns: the slot is free now
        releasedThisFrame_ = true;
        return texture_;
""")
patch(V, """void VideoPlayer::releaseGL()
{
    if (texture_ != 0)
""", """void VideoPlayer::releaseGL()
{
    if (dvf::arms().client != 0)   // DIAG-VFPS TEMPORARY: texture_ aliases one of the per-slot textures
    {
        glDeleteTextures(3, dvfClientTex_);
        dvfClientTex_[0] = dvfClientTex_[1] = dvfClientTex_[2] = 0;
        texture_ = 0;
    }
    if (dvfPbo_[0] != 0) { glDeleteBuffers(2, dvfPbo_); dvfPbo_[0] = dvfPbo_[1] = 0; }   // DIAG-VFPS TEMPORARY
    if (texture_ != 0)
""")
patch(V, """void VideoPlayer::decodeLoop()
{
""", """void VideoPlayer::decodeLoop()
{
    if (dvf::arms().qos == 1)   // DIAG-VFPS TEMPORARY: the H2 counterfactual
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0);
    dvf::rec(20, (double) dvf::now(), (double) qos_class_self(), (double) (reinterpret_cast<uintptr_t>(this) & 0xffff), (double) width_);
""")
patch(V, """        if (!decodeNextFrame())
        {
            if (!atEof_)""", """        const uint64_t dvfD0 = dvf::now();   // DIAG-VFPS TEMPORARY
        const bool dvfGot = decodeNextFrame();
        const uint64_t dvfD1 = dvf::now();
        if (!dvfGot)
        {
            if (!atEof_)""")
patch(V, """        onDecoded(g, pol);
    }

    freeFfmpeg();""", """        onDecoded(g, pol);
        dvf::rec(21, (double) dvfD0, (double) (dvfD1 - dvfD0), (double) (dvf::now() - dvfD1), (double) (reinterpret_cast<uintptr_t>(this) & 0xffff), 0.0);   // DIAG-VFPS TEMPORARY
    }

    freeFfmpeg();""")
patch(V, """    int s;
    while ((s = ring_.acquireWrite()) < 0)   // the ring is full: the reader frees a slot and notifies
    {""", """    int s;
    const uint64_t dvfW0 = dvf::now();   // DIAG-VFPS TEMPORARY
    while ((s = ring_.acquireWrite()) < 0)   // the ring is full: the reader frees a slot and notifies
    {""")
patch(V, """    convertInto(s);
    ring_.publish(s, pts, gen, ++seq_);
    newestPts_ = pts;""", """    const uint64_t dvfS0 = dvf::now();   // DIAG-VFPS TEMPORARY
    convertInto(s);
    dvf::rec(22, (double) dvfS0, (double) (dvf::now() - dvfS0), (double) (dvfS0 - dvfW0), (double) (reinterpret_cast<uintptr_t>(this) & 0xffff), 0.0);
    ring_.publish(s, pts, gen, ++seq_);
    newestPts_ = pts;""")
# the dispatcher itself, appended
p = W + "/" + V
s = open(p, newline="").read()
s += r'''
// DIAG-VFPS TEMPORARY: the upload dispatcher. No arm = the shipped path (GL_RGBA / GL_UNSIGNED_BYTE, one texture).
GLuint VideoPlayer::dvfUpload(const uint8_t* bytes, int slot, bool create)
{
    const auto& a = dvf::arms();
    const bool bgra = a.bgra != 0 || a.client != 0;
    const GLenum fmt = bgra ? GL_BGRA : GL_RGBA;
    const GLenum type = (a.bgra == 1 || a.client != 0) ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_BYTE;
    auto params = []
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    };
    auto logErr = [this](const char* where)
    {
        const GLenum e = glGetError();
        if (e != GL_NO_ERROR && !dvfErrLogged_)
        {
            dvfErrLogged_ = true;
            dvf::rec(16, (double) dvf::now(), (double) e);
            std::cerr << "[DIAG-VFPS] GL error 0x" << std::hex << e << std::dec << " at " << where << std::endl;
        }
    };
    if (a.client != 0)
    {
        GLuint& t = dvfClientTex_[slot];
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        if (t == 0)
        {
            glGenTextures(1, &t);
            glBindTexture(GL_TEXTURE_2D, t);
            params();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, fmt, type, bytes);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, t);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, fmt, type, bytes);
        }
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
        logErr("client");
        return t;
    }
    if (create)
    {
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        params();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, fmt, type, bytes);
        logErr("create");
        return texture_;
    }
    glBindTexture(GL_TEXTURE_2D, texture_);
    if (a.noUpload != 0)
        return texture_;   // c1: reuse the first upload
    if (a.pbo != 0)
    {
        const GLsizeiptr size = static_cast<GLsizeiptr>(rowBytes_) * height_;
        if (dvfPbo_[0] == 0)
            glGenBuffers(2, dvfPbo_);
        dvfPboIdx_ ^= 1;
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dvfPbo_[dvfPboIdx_]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);   // orphan
        const uint64_t m0 = dvf::now();
        if (void* dst = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT))
        {
            std::memcpy(dst, bytes, static_cast<size_t>(size));
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        dvf::frame().memcpyTicks += dvf::now() - m0;
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, fmt, type, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        logErr("pbo");
        return texture_;
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, fmt, type, bytes);
    logErr("sub");
    return texture_;
}
'''
open(p, "w", newline="").write(s)
print("appended dvfUpload")

