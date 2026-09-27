// test_shared_frame_gl -- THE OFFSCREEN GATE of the output frame path (s-rta-0927 outputs-c1 = plan5 slice C1,
// .harmony/.reports/s-rta-0926b/plan5-final.md 0.6 / 10.1): the real SharedFrameSet (writer) and the real
// presentSharedFrame (reader) across THREE PRIVATE CGL contexts -- W (writer), A and B (readers) -- with no
// window and no drawable (FBO rendering only), all on this thread via CGLSetCurrentContext. Nothing is ever
// shown on a screen. If the machine has no GL pixel format (a headless CI Mac) every case SKIPs loudly.
#include <catch2/catch_test_macros.hpp>
#include "output/SharedFrameSet.h"
#include "output/OutputPresenter.h"
#include "render/RenderGeometry.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#include <OpenGL/OpenGL.h>
#include <CoreFoundation/CoreFoundation.h>
#include <cstdint>
#include <vector>

using namespace juce::gl;

namespace
{
CGLContextObj makeContext()
{
    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile,
                                        static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_GL4_Core),
                                        static_cast<CGLPixelFormatAttribute>(0) };
    CGLPixelFormatObj pf = nullptr;
    GLint n = 0;
    if (CGLChoosePixelFormat(attrs, &pf, &n) != kCGLNoError || pf == nullptr)
        return nullptr;
    CGLContextObj ctx = nullptr;
    const CGLError err = CGLCreateContext(pf, nullptr, &ctx);   // NOT shared with any other context
    CGLReleasePixelFormat(pf);
    return err == kCGLNoError ? ctx : nullptr;
}

void makeCurrent(CGLContextObj ctx)
{
    CGLSetCurrentContext(ctx);
    static bool loaded = false;
    if (!loaded && ctx != nullptr)
    {
        juce::gl::loadFunctions();   // dlsym-based on macOS: needs no JUCE context
        loaded = true;
    }
}

using Pixels = std::vector<uint8_t>;   // RGBA8, row 0 = bottom (GL order)

Pixels pattern(int w, int h, int seed)
{
    Pixels p(static_cast<size_t>(w * h * 4));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            uint8_t* px = &p[static_cast<size_t>((y * w + x) * 4)];
            px[0] = static_cast<uint8_t>(x * 7 + seed * 31 + 1);
            px[1] = static_cast<uint8_t>(y * 13 + seed * 17 + 2);
            px[2] = static_cast<uint8_t>(x * y + seed * 5 + 3);
            px[3] = 255;
        }
    return p;
}

// An RGBA8 texture + FBO in the CURRENT context (the writer's canvas, a reader's target).
struct Surface
{
    GLuint tex = 0, fbo = 0;
    int w = 0, h = 0;

    void create(int width, int height)
    {
        w = width; h = height;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        REQUIRE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void upload(const Pixels& p)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    Pixels read() const
    {
        Pixels p(static_cast<size_t>(w * h * 4));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p.data());
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        return p;
    }
    void destroy()
    {
        if (fbo != 0) glDeleteFramebuffers(1, &fbo);
        if (tex != 0) glDeleteTextures(1, &tex);
        fbo = tex = 0;
    }
};

// RGB of `got` inside rect r (target coords) == `src` (r.w x r.h) exactly; RGB outside r == 0.
bool sameRGBInRect(const Pixels& got, int tw, int th, const RenderGeometry::Rect& r, const Pixels& src, int* bad = nullptr)
{
    int mismatches = 0;
    for (int y = 0; y < th; ++y)
        for (int x = 0; x < tw; ++x)
        {
            const uint8_t* g = &got[static_cast<size_t>((y * tw + x) * 4)];
            const bool inside = x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
            for (int c = 0; c < 3; ++c)
            {
                const uint8_t want = inside ? src[static_cast<size_t>(((y - r.y) * r.w + (x - r.x)) * 4 + c)] : 0;
                if (g[c] != want)
                    ++mismatches;
            }
        }
    if (bad != nullptr)
        *bad = mismatches;
    return mismatches == 0;
}

// W, A, B + one frame set. W owns a 64x36 canvas.
struct Rig
{
    CGLContextObj W = nullptr, A = nullptr, B = nullptr;
    output::SharedFrameSet frames;
    output::PresenterGLState stA, stB;
    Surface canvas;
    bool ok = false;

    Rig()
    {
        W = makeContext(); A = makeContext(); B = makeContext();
        ok = W != nullptr && A != nullptr && B != nullptr;
        if (!ok)
            return;
        makeCurrent(W);
        canvas.create(64, 36);
    }
    ~Rig()
    {
        if (W != nullptr) { makeCurrent(W); frames.releaseGL(); canvas.destroy(); }
        if (A != nullptr) { makeCurrent(A); stA.release(); }
        if (B != nullptr) { makeCurrent(B); stB.release(); }
        makeCurrent(nullptr);
        for (auto c : { W, A, B })
            if (c != nullptr) CGLDestroyContext(c);
    }

    // Writer: `n` publishes of the canvas, each followed by glFinish (so the next poll sees the fence done).
    void publish(int n = 1)
    {
        makeCurrent(W);
        for (int i = 0; i < n; ++i)
        {
            frames.publish(canvas.fbo, canvas.w, canvas.h);
            glFinish();
        }
    }

    // Reader: present into a fresh tw x th target of `ctx` and read it back.
    Pixels present(CGLContextObj ctx, output::PresenterGLState& st, int tw, int th, bool* presented = nullptr)
    {
        makeCurrent(ctx);
        Surface t;
        t.create(tw, th);
        const bool p = output::presentSharedFrame(frames, st, t.fbo, tw, th);
        glFinish();
        Pixels px = t.read();
        t.destroy();
        if (presented != nullptr)
            *presented = p;
        return px;
    }
};

#define REQUIRE_GL(rig) do { if (!(rig).ok) SKIP("no CGL OpenGL 4.1 pixel format on this machine -- offscreen GL gate NOT run"); } while (0)
} // namespace

TEST_CASE("shared frames: roundtrip_byte_identical", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 1);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);

    rig.frames.publish(rig.canvas.fbo, 64, 36);
    CHECK(rig.frames.front().gen == 0);   // the copy is in flight: nothing is offered yet
    glFinish();
    rig.frames.publish(rig.canvas.fbo, 64, 36);
    CHECK(rig.frames.front() == output::FrontFrame{ 1, 1, 0 });

    bool presented = false;
    const Pixels got = rig.present(rig.A, rig.stA, 64, 36, &presented);
    CHECK(presented);
    int bad = -1;
    CHECK(sameRGBInRect(got, 64, 36, { 0, 0, 64, 36 }, p1, &bad));
    CHECK(bad == 0);
}

TEST_CASE("shared frames: nothing published -> the reader clears black and reports false", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    bool presented = true;
    const Pixels got = rig.present(rig.A, rig.stA, 16, 16, &presented);
    CHECK_FALSE(presented);
    CHECK(sameRGBInRect(got, 16, 16, { 0, 0, 0, 0 }, {}));
}

TEST_CASE("shared frames: letterbox_geometry", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 2);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);
    rig.publish(2);

    const auto fit = RenderGeometry::fitCanvas(64, 36, 64, 64);
    REQUIRE(fit.x == 0);
    REQUIRE(fit.y == 14);
    REQUIRE(fit.w == 64);
    REQUIRE(fit.h == 36);
    const Pixels got = rig.present(rig.A, rig.stA, 64, 64);
    int bad = -1;
    CHECK(sameRGBInRect(got, 64, 64, fit, p1, &bad));   // bars black, the picture 1:1 inside
    CHECK(bad == 0);
}

TEST_CASE("shared frames: reader_tracks_new_frames", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 3), p2 = pattern(64, 36, 4);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);
    rig.publish(2);
    CHECK(sameRGBInRect(rig.present(rig.A, rig.stA, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));
    const uint32_t serial0 = rig.frames.front().serial;

    makeCurrent(rig.W);
    rig.canvas.upload(p2);
    rig.publish(2);   // offers the in-flight p1 copy, then the p2 copy
    CHECK(rig.frames.front().serial == serial0 + 2);
    int bad = -1;
    CHECK(sameRGBInRect(rig.present(rig.A, rig.stA, 64, 36), 64, 36, { 0, 0, 64, 36 }, p2, &bad));   // LIVE, not a stale slot
    CHECK(bad == 0);
}

TEST_CASE("shared frames: generation_change", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 5), p3 = pattern(32, 18, 6);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);
    rig.publish(2);
    CHECK(sameRGBInRect(rig.present(rig.A, rig.stA, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));
    CHECK(rig.stA.boundGen == 1);

    makeCurrent(rig.W);
    Surface canvas2;
    canvas2.create(32, 18);
    canvas2.upload(p3);
    for (int i = 0; i < 2; ++i)
    {
        rig.frames.publish(canvas2.fbo, 32, 18);
        glFinish();
    }
    CHECK(rig.frames.front().gen == 2);
    CHECK(rig.frames.frontWidth() == 32);
    CHECK(rig.frames.frontHeight() == 18);
    int bad = -1;
    CHECK(sameRGBInRect(rig.present(rig.A, rig.stA, 32, 18), 32, 18, { 0, 0, 32, 18 }, p3, &bad));
    CHECK(bad == 0);
    CHECK(rig.stA.boundGen == 2);

    IOSurfaceRef old = rig.frames.pool().retainSurface(1, 0);
    CHECK(old != nullptr);   // retired, not yet released
    if (old != nullptr)
        CFRelease(old);
    makeCurrent(rig.W);
    for (int i = 0; i < output::SurfacePool::kRetireFrames; ++i)
        rig.frames.publish(canvas2.fbo, 32, 18);
    glFinish();
    CHECK(rig.frames.pool().retainSurface(1, 0) == nullptr);

    // B never bound generation 1.
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 32, 18), 32, 18, { 0, 0, 32, 18 }, p3));
    makeCurrent(rig.W);
    canvas2.destroy();
}

TEST_CASE("shared frames: reader_destroyed_mid_run", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 7), p3 = pattern(64, 36, 8);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);
    rig.publish(2);
    CHECK(sameRGBInRect(rig.present(rig.A, rig.stA, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));

    // Output A dies (its window closes) while B stays live.
    makeCurrent(rig.A);
    rig.stA.release();
    makeCurrent(nullptr);
    CGLDestroyContext(rig.A);
    rig.A = nullptr;

    makeCurrent(rig.W);
    rig.canvas.upload(p3);
    rig.publish(2);
    int bad = -1;
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 64, 36), 64, 36, { 0, 0, 64, 36 }, p3, &bad));
    CHECK(bad == 0);
}

TEST_CASE("shared frames: writer_context_loss_keeps_last_frame", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const Pixels p1 = pattern(64, 36, 9), p4 = pattern(64, 36, 10);
    makeCurrent(rig.W);
    rig.canvas.upload(p1);
    rig.publish(2);
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));
    const output::FrontFrame before = rig.frames.front();

    // The main context dies (preview hidden / app minimised): openGLContextClosing -> releaseGL, context gone.
    makeCurrent(rig.W);
    rig.frames.releaseGL();
    rig.canvas.destroy();
    makeCurrent(nullptr);
    CGLDestroyContext(rig.W);
    rig.W = nullptr;
    CHECK(rig.frames.front() == before);
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 64, 36), 64, 36, { 0, 0, 64, 36 }, p1));   // frozen, not black

    // The main context returns: the SAME surfaces are rebound, publishing resumes.
    rig.W = makeContext();
    REQUIRE(rig.W != nullptr);
    makeCurrent(rig.W);
    rig.canvas.create(64, 36);
    rig.canvas.upload(p4);
    rig.publish(2);
    const output::FrontFrame after = rig.frames.front();
    CHECK(after.gen == before.gen);   // same generation: no new surfaces
    CHECK(after.serial > before.serial);
    int bad = -1;
    CHECK(sameRGBInRect(rig.present(rig.B, rig.stB, 64, 36), 64, 36, { 0, 0, 64, 36 }, p4, &bad));
    CHECK(bad == 0);
}

TEST_CASE("shared frames: slot_rotation", "[shared_frame_gl]")
{
    Rig rig;
    REQUIRE_GL(rig);
    REQUIRE(output::SurfacePool::kSlots == 4);
    makeCurrent(rig.W);
    rig.canvas.upload(pattern(64, 36, 11));
    rig.publish(1);
    std::vector<int> slots;
    for (int i = 0; i < 8; ++i)
    {
        rig.publish(1);
        slots.push_back(rig.frames.front().slot);
    }
    bool seen[4] = { false, false, false, false };
    for (size_t i = 0; i < slots.size(); ++i)
    {
        REQUIRE(slots[i] >= 0);
        REQUIRE(slots[i] < 4);
        seen[slots[i]] = true;
        if (i > 0)
            CHECK(slots[i] == (slots[i - 1] + 1) % 4);   // strict round-robin over all four slots
    }
    CHECK((seen[0] && seen[1] && seen[2] && seen[3]));
    CHECK(slots.front() == 0);
}
