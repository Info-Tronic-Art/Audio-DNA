// test_projectm_canvas_gl -- BF10 (s-rta-1002b; .harmony/.reports/s-rta-1002b/plan-bf10.md I3, ruling-bf10.md
// amendments 1, 10, 11, 12): MilkDrop draws its final picture into its OWN canvas-sized framebuffer (outputFBO_),
// never framebuffer 0, and the caller's GL state survives every render() -- init, resize, preset-load and soft-cut
// frames included.
//
// The REAL ProjectMSource renders through the linked (patched) libprojectM in a PRIVATE CGL 4.1 core context (no
// window, no drawable: framebuffer 0 is GL_FRAMEBUFFER_UNDEFINED here, so anything drawn there is lost -- that is the
// RED-by-absence of amendment 1). The RETURNED texture is attached to a test FBO and decoded. On a machine with no GL
// pixel format every case SKIPs loudly.
//
// Fixtures (tests/fixtures/milkdrop/), validated and FROZEN in S1 (ruling amendment 13, commit c4aa435) -- a T-case
// failure is a code failure, never a reason to retune a fixture or a bar:
//   bd5f90035ee94fdf2a9a4a1960fb87b9bee2cdedd0e2f6c550959b3c50cc376c  bf10_solid.milk    A = (204, 51, 26, 255)
//   cd8038fdd7d37e26146f7298e3e5124ce30d46df4fabfeab79e2cbab09cc2fc8  bf10_solid_b.milk  B = (26, 178, 229, 255)
//   d64fc1d27b9c83a52f33ec886636772eaec1d86835b8a6b2eaefae4ccc21bb57  bf10_time.milk     red = f(time), period 2 s
//   99603d96cfd91df09ba1791eb7cab9e91d1186411d326cde2b57bdaeaedbbc7c  bf10_circle.milk   one round blob, centred
// S1 error baseline (every glGetError after each fbo-mode call, our complete FBO bound): NONE for all four fixtures,
// so T5 tolerates no GL error code -- with ONE exception (rulings-bf10-s2.md STOP 1): libprojectM 4.1.1's own
// projectm_create and the FIRST preset load raise GL_INVALID_ENUM (0x500), stock and patched alike, so the INIT frame
// (T5 (b)) tolerates exactly {0x500}; every other code, and any error on a later frame, still fails.
// FOUND 1 (rulings-bf10-s2.md): a preset without a comp shader writes its own alpha < 1; render() forces alpha 1, and
// T4 asserts bf10_circle (a no-comp preset) is alpha 255 on 100 % of its pixels.
#include <catch2/catch_test_macros.hpp>
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#include <OpenGL/OpenGL.h>
#include "sources/ProjectMSource.h"
#include "render/ShaderManager.h"
#include "render/FullscreenQuad.h"
#include "analysis/FeatureSnapshot.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

#ifndef AUDIODNA_MILKDROP_FIXTURE_DIR
#error "AUDIODNA_MILKDROP_FIXTURE_DIR must point at tests/fixtures/milkdrop"
#endif

using namespace juce::gl;

namespace
{
using Clock = std::chrono::steady_clock;
using Pixels = std::vector<uint8_t>;   // RGBA8

const std::string kSolid  = std::string(AUDIODNA_MILKDROP_FIXTURE_DIR) + "/bf10_solid.milk";
const std::string kSolidB = std::string(AUDIODNA_MILKDROP_FIXTURE_DIR) + "/bf10_solid_b.milk";
const std::string kTime   = std::string(AUDIODNA_MILKDROP_FIXTURE_DIR) + "/bf10_time.milk";
const std::string kCircle = std::string(AUDIODNA_MILKDROP_FIXTURE_DIR) + "/bf10_circle.milk";

constexpr std::array<int, 3> kA { 204, 51, 26 };    // bf10_solid, frozen in S1
constexpr std::array<int, 3> kB { 26, 178, 229 };   // bf10_solid_b, frozen in S1

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
    const CGLError err = CGLCreateContext(pf, nullptr, &ctx);
    CGLReleasePixelFormat(pf);
    return err == kCGLNoError ? ctx : nullptr;
}

// One private context + the arguments ProjectMSource::render takes (it ignores the ShaderManager and the quad).
struct Rig
{
    CGLContextObj ctx = nullptr;
    bool ok = false;
    juce::OpenGLContext juceCtx;   // never attached: ShaderManager only keeps the reference
    std::unique_ptr<ShaderManager> shaders;
    std::unique_ptr<FullscreenQuad> quad;
    FeatureSnapshot snap;
    std::vector<float> pcm = std::vector<float>(512);
    long frame = 0;

    Rig()
    {
        ctx = makeContext();
        ok = ctx != nullptr;
        if (!ok) return;
        CGLSetCurrentContext(ctx);
        juce::gl::loadFunctions();   // dlsym-based on macOS: needs no JUCE context
        shaders = std::make_unique<ShaderManager>(juceCtx);
        quad = std::make_unique<FullscreenQuad>();
    }
    ~Rig()
    {
        if (!ok) return;
        quad.reset();
        shaders.reset();
        CGLSetCurrentContext(nullptr);
        CGLDestroyContext(ctx);
    }

    // One app-shaped frame: feed a block of audio, then render at (w, h).
    GLuint render(ProjectMSource& src, int w, int h)
    {
        for (size_t i = 0; i < pcm.size(); ++i)
            pcm[i] = 0.7f * std::sin(2.0f * 3.14159265f * 80.0f * float(frame * 512 + long(i)) / 48000.0f);
        src.feedAudio(pcm.data(), int(pcm.size()));
        ++frame;
        return src.render(*shaders, *quad, 0.0f, w, h, snap);
    }
};

#define REQUIRE_GL(rig) do { if (!(rig).ok) SKIP("no CGL OpenGL 4.1 pixel format on this machine -- offscreen GL gate NOT run"); } while (0)

void drainErrors() { while (glGetError() != GL_NO_ERROR) {} }

void texSize(GLuint tex, int& w, int& h)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Clear the RETURNED texture to the sentinel through the test's own FBO.
void clearTex(GLuint tex, float r, float g, float b, float a)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    int w = 0, h = 0;
    texSize(tex, w, h);
    glViewport(0, 0, w, h);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
}
void clearSentinel(GLuint tex) { clearTex(tex, 1.0f, 0.0f, 1.0f, 0.0f); }   // (255, 0, 255, 0)

Pixels readTex(GLuint tex, int w, int h)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    Pixels px(size_t(w) * size_t(h) * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    return px;
}

struct Stats
{
    int w = 0, h = 0;
    std::array<int, 4> med { 0, 0, 0, 0 };
    double within6 = 0;    // % of pixels with every RGB channel within +-6 of the per-channel median
    double alpha255 = 0;   // % of pixels with alpha 255
    long sentinel = 0;     // pixels exactly (255, 0, 255, 0)
    double meanRGB = 0;
};

Stats stats(const Pixels& px, int w, int h)
{
    Stats s; s.w = w; s.h = h;
    const size_t n = size_t(w) * size_t(h);
    for (int c = 0; c < 4; ++c)
    {
        std::array<size_t, 256> hist {};
        for (size_t i = 0; i < n; ++i) ++hist[px[i * 4 + size_t(c)]];
        size_t acc = 0; int m = 0;
        for (; m < 256; ++m) { acc += hist[size_t(m)]; if (acc * 2 >= n) break; }
        s.med[size_t(c)] = m;
    }
    size_t in6 = 0, a255 = 0; long sent = 0; double sum = 0;
    for (size_t i = 0; i < n; ++i)
    {
        const uint8_t* p = &px[i * 4];
        bool ok = true;
        for (int c = 0; c < 3; ++c) { if (std::abs(int(p[c]) - s.med[size_t(c)]) > 6) ok = false; sum += p[c]; }
        in6 += ok ? 1 : 0;
        a255 += p[3] == 255 ? 1 : 0;
        sent += (p[0] == 255 && p[1] == 0 && p[2] == 255 && p[3] == 0) ? 1 : 0;
    }
    s.within6 = 100.0 * double(in6) / double(n);
    s.alpha255 = 100.0 * double(a255) / double(n);
    s.sentinel = sent;
    s.meanRGB = sum / (3.0 * double(n));
    return s;
}

// % of pixels whose RGB lies within +-tol of `col`.
double pctNear(const Pixels& px, const std::array<int, 3>& col, int tol)
{
    const size_t n = px.size() / 4;
    size_t k = 0;
    for (size_t i = 0; i < n; ++i)
    {
        const uint8_t* p = &px[i * 4];
        if (std::abs(int(p[0]) - col[0]) <= tol && std::abs(int(p[1]) - col[1]) <= tol
            && std::abs(int(p[2]) - col[2]) <= tol)
            ++k;
    }
    return 100.0 * double(k) / double(n);
}

// % of pixels inside the per-channel box [min(A,B) - 6, max(A,B) + 6].
double pctInBox(const Pixels& px)
{
    const size_t n = px.size() / 4;
    size_t k = 0;
    for (size_t i = 0; i < n; ++i)
    {
        bool in = true;
        for (size_t c = 0; c < 3; ++c)
        {
            const int v = px[i * 4 + c];
            if (v < std::min(kA[c], kB[c]) - 6 || v > std::max(kA[c], kB[c]) + 6) in = false;
        }
        k += in ? 1 : 0;
    }
    return 100.0 * double(k) / double(n);
}

bool medianNear(const Stats& s, const std::array<int, 3>& col, int tol)
{
    for (size_t c = 0; c < 3; ++c)
        if (std::abs(s.med[c] - col[c]) > tol) return false;
    return true;
}

void printStats(const char* tag, const Stats& s)
{
    std::printf("DATA %s size=%dx%d median=(%d,%d,%d,%d) within6=%.4f%% alpha255=%.4f%% sentinel=%ld\n", tag, s.w, s.h,
                s.med[0], s.med[1], s.med[2], s.med[3], s.within6, s.alpha255, s.sentinel);
}

void pace(Clock::time_point& next)
{
    next += std::chrono::milliseconds(16);
    std::this_thread::sleep_until(next);
}

std::unique_ptr<ProjectMSource> makeSource(const std::string& preset)
{
    auto src = std::make_unique<ProjectMSource>();
    src->setPresetLocked(true);
    src->loadPreset(preset, false);   // queued; the first render() (the init frame) loads it
    return src;
}

// Amendment 10's per-read bar for bf10_solid at (w, h).
void checkSolidRead(const char* tag, GLuint tex, int w, int h)
{
    int tw = 0, th = 0;
    texSize(tex, tw, th);
    const Pixels px = readTex(tex, w, h);
    const Stats s = stats(px, w, h);
    printStats(tag, s);
    INFO(tag << " tex=" << tw << "x" << th << " median=(" << s.med[0] << "," << s.med[1] << "," << s.med[2] << ","
              << s.med[3] << ") within6=" << s.within6 << "% alpha255=" << s.alpha255 << "% sentinel=" << s.sentinel);
    CHECK(tw == w);
    CHECK(th == h);
    CHECK(s.alpha255 == 100.0);
    CHECK(s.within6 >= 99.99);
    CHECK(medianNear(s, kA, 6));
    CHECK(s.sentinel == 0);
}
} // namespace

// T1 -- fill (amendment 10). One source, four canvas sizes in turn (the first render() at each size is the init or
// resize frame), then 30 frames, the returned texture cleared to the sentinel (255,0,255,0) before EACH one.
TEST_CASE("T1 MilkDrop fills its whole canvas-sized texture at every size", "[bf10][T1]")
{
    Rig rig;
    REQUIRE_GL(rig);
    auto src = makeSource(kSolid);
    const std::array<std::pair<int, int>, 4> sizes { { { 1280, 720 }, { 1920, 1080 }, { 1080, 1920 }, { 3840, 2160 } } };
    auto next = Clock::now();
    for (const auto& [w, h] : sizes)
    {
        GLuint tex = rig.render(*src, w, h);   // init / resize frame
        pace(next);
        for (int f = 1; f <= 30; ++f)
        {
            clearSentinel(tex);
            tex = rig.render(*src, w, h);
            pace(next);
            if (f == 1 || f == 2 || f == 10 || f == 20 || f == 30)
            {
                const std::string tag = "T1 " + std::to_string(w) + "x" + std::to_string(h) + " f=" + std::to_string(f);
                checkSolidRead(tag.c_str(), tex, w, h);
            }
        }
    }
    src->releaseGL();
}

// T2 -- canvas change: 1920x1080 (3 frames) -> 1080x1920 -> 1920x1080. From the FIRST frame at each new size the
// texture has the new size and alpha 255 everywhere; the fixture colour covers >= 99.99 % within <= 2 frames.
TEST_CASE("T2 MilkDrop follows a canvas change from the first frame", "[bf10][T2]")
{
    Rig rig;
    REQUIRE_GL(rig);
    auto src = makeSource(kSolid);
    auto next = Clock::now();
    GLuint tex = 0;
    for (int f = 0; f < 3; ++f) { tex = rig.render(*src, 1920, 1080); pace(next); }
    const std::array<std::pair<int, int>, 2> changes { { { 1080, 1920 }, { 1920, 1080 } } };
    for (const auto& [w, h] : changes)
    {
        int coveredAt = -1;
        for (int f = 1; f <= 5; ++f)
        {
            clearSentinel(tex);
            tex = rig.render(*src, w, h);
            pace(next);
            int tw = 0, th = 0;
            texSize(tex, tw, th);
            const Pixels px = readTex(tex, w, h);
            const Stats s = stats(px, w, h);
            const std::string tag = "T2 ->" + std::to_string(w) + "x" + std::to_string(h) + " f=" + std::to_string(f);
            printStats(tag.c_str(), s);
            INFO(tag << " tex=" << tw << "x" << th << " alpha255=" << s.alpha255 << "% within6=" << s.within6 << "%");
            CHECK(tw == w);
            CHECK(th == h);
            CHECK(s.alpha255 == 100.0);
            if (coveredAt < 0 && s.within6 >= 99.99 && medianNear(s, kA, 6))
                coveredAt = f;
        }
        std::printf("DATA T2 ->%dx%d fixture colour covered at frame %d (INFO)\n", w, h, coveredAt);
        INFO("->" << w << "x" << h << " covered at frame " << coveredAt);
        CHECK(coveredAt >= 1);
        CHECK(coveredAt <= 2);
    }
    src->releaseGL();
}

// T3 -- live, no stale region: bf10_time at 1920x1080, paced frames, two reads 300 ms apart; every 64x64 tile's mean
// changes by >= 2 levels.
TEST_CASE("T3 MilkDrop keeps every region of the canvas live", "[bf10][T3]")
{
    Rig rig;
    REQUIRE_GL(rig);
    auto src = makeSource(kTime);
    const int w = 1920, h = 1080, tile = 64;
    auto next = Clock::now();
    const auto start = Clock::now();
    GLuint tex = 0;
    // Warm up 0.6 s of wall time: the first read then sits clear of the red channel's peak (t = 0.5 s), so the pair
    // never straddles an extremum of the 2 s period.
    while (Clock::now() - start < std::chrono::milliseconds(600)) { tex = rig.render(*src, w, h); pace(next); }
    auto tileMeans = [&](const Pixels& px) {
        std::vector<double> m;
        for (int ty = 0; ty < h; ty += tile)
            for (int tx = 0; tx < w; tx += tile)
            {
                double sum = 0; long cnt = 0;
                for (int y = ty; y < std::min(h, ty + tile); ++y)
                    for (int x = tx; x < std::min(w, tx + tile); ++x)
                    {
                        const uint8_t* p = &px[(size_t(y) * size_t(w) + size_t(x)) * 4];
                        sum += p[0] + p[1] + p[2]; ++cnt;
                    }
                m.push_back(sum / (3.0 * double(cnt)));
            }
        return m;
    };
    const auto a = tileMeans(readTex(tex, w, h));
    const auto t0 = Clock::now();
    while (Clock::now() - t0 < std::chrono::milliseconds(300)) { tex = rig.render(*src, w, h); pace(next); }
    const auto b = tileMeans(readTex(tex, w, h));
    double minDelta = 1e9;
    size_t stale = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        const double d = std::fabs(b[i] - a[i]);
        minDelta = std::min(minDelta, d);
        if (d < 2.0) ++stale;
    }
    std::printf("DATA T3 tiles=%zu stale(<2 levels)=%zu min_delta=%.2f\n", a.size(), stale, minDelta);
    INFO("tiles=" << a.size() << " stale=" << stale << " min_delta=" << minDelta);
    CHECK(stale == 0);
    src->releaseGL();
}

// T4 -- no stretch: bf10_circle at 1920x1080 and 1080x1920; the lit blob is round (|w/h - 1| <= 0.04) and centred
// (+-2 % of the canvas). If this fails on the fix, plan E10 is wrong: STOP and report, never retune.
TEST_CASE("T4 MilkDrop draws round shapes round at the canvas's shape", "[bf10][T4]")
{
    Rig rig;
    REQUIRE_GL(rig);
    const std::array<std::pair<int, int>, 2> sizes { { { 1920, 1080 }, { 1080, 1920 } } };
    for (const auto& [w, h] : sizes)
    {
        auto src = makeSource(kCircle);
        auto next = Clock::now();
        GLuint tex = 0;
        for (int f = 0; f < 90; ++f) { tex = rig.render(*src, w, h); pace(next); }
        const Pixels px = readTex(tex, w, h);
        // lit = max channel >= 128; 4-connected components; the largest is "the blob".
        std::vector<int> lab(size_t(w) * size_t(h), 0);
        auto litAt = [&](size_t i) { const uint8_t* p = &px[i * 4]; return std::max({ p[0], p[1], p[2] }) >= 128; };
        int comps = 0; long best = 0; int bx0 = 0, by0 = 0, bx1 = -1, by1 = -1;
        std::vector<size_t> stack;
        for (size_t i = 0; i < lab.size(); ++i)
        {
            if (lab[i] != 0 || !litAt(i)) continue;
            ++comps; long size = 0; int x0 = w, y0 = h, x1 = -1, y1 = -1;
            stack.assign(1, i); lab[i] = comps;
            while (!stack.empty())
            {
                const size_t j = stack.back(); stack.pop_back(); ++size;
                const int x = int(j % size_t(w)), y = int(j / size_t(w));
                x0 = std::min(x0, x); x1 = std::max(x1, x); y0 = std::min(y0, y); y1 = std::max(y1, y);
                const int nb[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
                for (const auto& d : nb)
                {
                    const int nx = x + d[0], ny = y + d[1];
                    if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue;
                    const size_t k = size_t(ny) * size_t(w) + size_t(nx);
                    if (lab[k] == 0 && litAt(k)) { lab[k] = comps; stack.push_back(k); }
                }
            }
            if (size > best) { best = size; bx0 = x0; by0 = y0; bx1 = x1; by1 = y1; }
        }
        const int bw = bx1 - bx0 + 1, bh = by1 - by0 + 1;
        const double ratio = bh > 0 ? double(bw) / double(bh) : 0.0;
        const double offx = 100.0 * ((bx0 + bx1 + 1) / 2.0 - w / 2.0) / w;
        const double offy = 100.0 * ((by0 + by1 + 1) / 2.0 - h / 2.0) / h;
        const Stats s = stats(px, w, h);
        std::printf("DATA T4 %dx%d blobs=%d bbox=(%d,%d)-(%d,%d) w=%d h=%d |w/h-1|=%.4f centre_off=(%.2f%%,%.2f%%)\n", w,
                    h, comps, bx0, by0, bx1, by1, bw, bh, std::fabs(ratio - 1.0), offx, offy);
        // FOUND 1 (rulings-bf10-s2.md): bf10_circle has no comp shader, so libprojectM writes the preset's own alpha
        // (its background is alpha 0); MilkDrop is a full-frame generator, so its output must be opaque.
        std::printf("DATA T4 %dx%d alpha255=%.4f%% median_alpha=%d\n", w, h, s.alpha255, s.med[3]);
        INFO(w << "x" << h << " blobs=" << comps << " w/h=" << ratio << " off=(" << offx << "%," << offy << "%)"
               << " alpha255=" << s.alpha255 << "%");
        CHECK(comps == 1);
        CHECK(std::fabs(ratio - 1.0) <= 0.04);
        CHECK(std::fabs(offx) <= 2.0);
        CHECK(std::fabs(offy) <= 2.0);
        CHECK(s.alpha255 == 100.0);
        src->releaseGL();
    }
}

namespace
{
// The caller's GL state of amendment 12, captured for comparison.
struct CallerState
{
    GLint draw = -1, read = -1, program = -1, vao = -1, active = -1, tex2D = -1;
    std::array<GLint, 4> viewport { 0, 0, 0, 0 };
    GLboolean blend = 0, depth = 0, stencil = 0, cull = 0, scissor = 0;
    std::array<GLint, 4> blendFuncs { 0, 0, 0, 0 };
    GLint depthFunc = 0;
    std::array<std::array<GLboolean, 4>, 4> masks {};
    std::array<GLint, 16> samplers {};
    std::array<GLint, 16> unitTex {};   // INFO only (F3)
};

CallerState capture()
{
    CallerState s;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.draw);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &s.read);
    glGetIntegerv(GL_VIEWPORT, s.viewport.data());
    glGetIntegerv(GL_CURRENT_PROGRAM, &s.program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &s.vao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &s.active);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &s.tex2D);
    s.blend = glIsEnabled(GL_BLEND);
    s.depth = glIsEnabled(GL_DEPTH_TEST);
    s.stencil = glIsEnabled(GL_STENCIL_TEST);
    s.cull = glIsEnabled(GL_CULL_FACE);
    s.scissor = glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_BLEND_SRC_RGB, &s.blendFuncs[0]);
    glGetIntegerv(GL_BLEND_DST_RGB, &s.blendFuncs[1]);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &s.blendFuncs[2]);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &s.blendFuncs[3]);
    glGetIntegerv(GL_DEPTH_FUNC, &s.depthFunc);
    for (GLuint i = 0; i < 4; ++i)
        glGetBooleani_v(GL_COLOR_WRITEMASK, i, s.masks[i].data());
    // Last: walking the units moves the active unit; put it back afterwards.
    for (GLuint u = 0; u < 16; ++u)
    {
        glActiveTexture(GL_TEXTURE0 + u);
        glGetIntegerv(GL_SAMPLER_BINDING, &s.samplers[u]);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &s.unitTex[u]);
    }
    glActiveTexture(static_cast<GLenum>(s.active));
    return s;
}

// The test's own objects for T5: a 64x64 sentinel FBO, a texture for unit 2, a trivial program and a VAO.
struct CallerObjects
{
    GLuint fbo = 0, fboTex = 0, unitTex = 0, program = 0, vao = 0;

    CallerObjects()
    {
        glGenTextures(1, &fboTex);
        glBindTexture(GL_TEXTURE_2D, fboTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
        glGenTextures(1, &unitTex);
        glBindTexture(GL_TEXTURE_2D, unitTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        const char* vs = "#version 410 core\nlayout(location=0) in vec2 p; void main(){ gl_Position = vec4(p, 0, 1); }";
        const char* fs = "#version 410 core\nout vec4 c; void main(){ c = vec4(1); }";
        auto sh = [](GLenum type, const char* src) {
            const GLuint s = glCreateShader(type);
            glShaderSource(s, 1, &src, nullptr);
            glCompileShader(s);
            return s;
        };
        const GLuint v = sh(GL_VERTEX_SHADER, vs), f = sh(GL_FRAGMENT_SHADER, fs);
        program = glCreateProgram();
        glAttachShader(program, v);
        glAttachShader(program, f);
        glLinkProgram(program);
        glDeleteShader(v);
        glDeleteShader(f);
        glGenVertexArrays(1, &vao);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~CallerObjects()
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &fboTex);
        glDeleteTextures(1, &unitTex);
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
    }

    // Amendment 12's entry state.
    void enter()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, 64, 64);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(17.0f / 255.0f, 34.0f / 255.0f, 51.0f / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(1, 2, 33, 44);
        glUseProgram(program);
        glBindVertexArray(vao);
        for (GLuint u = 0; u < 16; ++u)
            glBindSampler(u, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, unitTex);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDepthFunc(GL_LEQUAL);
        drainErrors();
    }

    // % of the sentinel FBO's pixels still exactly (17, 34, 51, 255). Binds the sentinel for READ.
    double sentinelIntact()
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        std::vector<uint8_t> px(64 * 64 * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        size_t k = 0;
        for (size_t i = 0; i < 64 * 64; ++i)
            k += (px[i * 4] == 17 && px[i * 4 + 1] == 34 && px[i * 4 + 2] == 51 && px[i * 4 + 3] == 255) ? 1 : 0;
        return 100.0 * double(k) / (64.0 * 64.0);
    }
};

// Measure one render(): the entry state is set right before it, then compared field by field. `tolerated` = the GL
// error codes this frame may raise (empty = none; the init frame's {0x500} is libprojectM's own, STOP 1).
void measuredFrame(const char* tag, Rig& rig, ProjectMSource& src, CallerObjects& caller, int w, int h,
                   const std::set<GLenum>& tolerated = {})
{
    caller.enter();
    const CallerState before = capture();
    drainErrors();
    rig.render(src, w, h);
    std::set<GLenum> errors;
    for (GLenum e; (e = glGetError()) != GL_NO_ERROR;) errors.insert(e);
    const CallerState after = capture();
    const double intact = caller.sentinelIntact();
    std::string errs, samplers, texs;
    for (const GLenum e : errors) { char b[16]; std::snprintf(b, sizeof b, " 0x%x", e); errs += b; }
    for (size_t u = 0; u < 16; ++u)
    {
        if (after.samplers[u] != 0) samplers += " u" + std::to_string(u) + "=" + std::to_string(after.samplers[u]);
        if (u != 2 && after.unitTex[u] != before.unitTex[u])
            texs += " u" + std::to_string(u) + "=" + std::to_string(after.unitTex[u]);
    }
    std::printf("DATA T5 %s draw=%d read=%d (sentinel %d) viewport=(%d,%d,%d,%d) errors=[%s ] samplers=[%s ] "
                "sentinel_intact=%.2f%% other_unit_textures(INFO)=[%s ]\n",
                tag, after.draw, after.read, before.draw, after.viewport[0], after.viewport[1], after.viewport[2],
                after.viewport[3], errs.c_str(), samplers.c_str(), intact, texs.c_str());
    INFO("T5 " << tag << ": draw=" << after.draw << " read=" << after.read << " (sentinel " << before.draw << ")"
               << " errors=[" << errs << " ] samplers=[" << samplers << " ] sentinel_intact=" << intact << "%");
    CHECK(after.draw == GLint(caller.fbo));
    CHECK(after.read == GLint(caller.fbo));
    CHECK((after.viewport == std::array<GLint, 4> { 1, 2, 33, 44 }));
    CHECK(after.program == before.program);
    CHECK(after.vao == before.vao);
    CHECK(after.active == GLint(GL_TEXTURE2));
    CHECK(after.tex2D == before.tex2D);
    CHECK(after.blend == before.blend);
    CHECK(after.blendFuncs == before.blendFuncs);
    CHECK(after.depth == before.depth);
    CHECK(after.stencil == before.stencil);
    CHECK(after.cull == before.cull);
    CHECK(after.scissor == before.scissor);
    CHECK(after.depthFunc == before.depthFunc);
    CHECK(after.masks == before.masks);
    CHECK(samplers.empty());
    CHECK(intact == 100.0);
    std::set<GLenum> untolerated;
    for (const GLenum e : errors)
        if (tolerated.count(e) == 0) untolerated.insert(e);
    if (!tolerated.empty())
        std::printf("DATA T5 %s drained=[%s ] tolerated={0x500} (INFO: libprojectM projectm_create + first load)\n", tag,
                    errs.c_str());
    CHECK(untolerated.empty());   // the drained set is empty or a subset of `tolerated`
}
} // namespace

// T5 -- the caller's GL state survives render() on every frame kind (amendment 12): (a) steady, (b) init, (c) resize,
// (d) a pending hard load, (e) a pending soft load (that frame and the next 10).
TEST_CASE("T5 render() leaves the caller's GL state exactly as it found it", "[bf10][T5]")
{
    Rig rig;
    REQUIRE_GL(rig);
    CallerObjects caller;
    const int w = 640, h = 360;
    auto warm = [&](ProjectMSource& src, int frames) {
        auto next = Clock::now();
        for (int f = 0; f < frames; ++f) { rig.render(src, w, h); pace(next); }
    };

    SECTION("(a) steady frame")
    {
        auto src = makeSource(kSolid);
        warm(*src, 10);
        measuredFrame("(a) steady", rig, *src, caller, w, h);
        src->releaseGL();
    }
    SECTION("(b) init frame")
    {
        // rulings-bf10-s2.md STOP 1: projectm_create + the first preset load raise GL_INVALID_ENUM inside libprojectM
        // (stock and patched alike, bf10-s2/errprobe.log). The init frame tolerates exactly {0x500}; the frame after
        // it tolerates nothing.
        auto src = makeSource(kSolid);
        measuredFrame("(b) init", rig, *src, caller, w, h, { GLenum(GL_INVALID_ENUM) });
        measuredFrame("(b) init +1", rig, *src, caller, w, h);
        src->releaseGL();
    }
    SECTION("(c) resize frame")
    {
        auto src = makeSource(kSolid);
        warm(*src, 10);
        measuredFrame("(c) resize", rig, *src, caller, 960, 540);
        src->releaseGL();
    }
    SECTION("(d) pending hard load")
    {
        auto src = makeSource(kSolid);
        warm(*src, 10);
        src->loadPreset(kSolidB, false);
        measuredFrame("(d) hard load", rig, *src, caller, w, h);
        src->releaseGL();
    }
    SECTION("(e) pending soft load, then 10 frames of the soft cut")
    {
        auto src = makeSource(kSolid);
        warm(*src, 10);
        src->getPresetSelector().setBlendSeconds(1.0f);
        src->loadPreset(kSolidB, true);
        auto next = Clock::now();
        for (int f = 0; f <= 10; ++f)
        {
            const std::string tag = "(e) soft load +" + std::to_string(f);
            measuredFrame(tag.c_str(), rig, *src, caller, w, h);
            pace(next);
        }
        src->releaseGL();
    }
}

namespace
{
struct SoftCutRead { double t = 0; int w = 0, h = 0, tw = 0, th = 0; Stats s; double inBox = 0, nearA = 0, nearB = 0; };

SoftCutRead readSoftCut(GLuint tex, int w, int h, double t)
{
    SoftCutRead r; r.t = t; r.w = w; r.h = h;
    texSize(tex, r.tw, r.th);
    const Pixels px = readTex(tex, w, h);
    r.s = stats(px, w, h);
    r.inBox = pctInBox(px);
    r.nearA = pctNear(px, kA, 6);
    r.nearB = pctNear(px, kB, 6);
    std::printf("DATA soft-cut t=%.3fs size=%dx%d tex=%dx%d median=(%d,%d,%d,%d) alpha255=%.4f%% in_box=%.4f%% "
                "near_B=%.4f%% near_A=%.4f%%\n", t, w, h, r.tw, r.th, r.s.med[0], r.s.med[1], r.s.med[2], r.s.med[3],
                r.s.alpha255, r.inBox, r.nearB, r.nearA);
    return r;
}

// A read mid-transition (rulings-bf10-s2.md STOP 2; covers a cross-fade AND a wipe -- projectM picks its transition
// shader at random): >= 5 % of the pixels within 6 of A AND >= 5 % within 6 of B (a wipe), OR the median >= 10 levels
// from both A and B in some channel (a cross-fade). Only a read inside the blend can meet it.
bool midTransition(const SoftCutRead& r)
{
    if (r.nearA >= 5.0 && r.nearB >= 5.0) return true;
    for (size_t c = 0; c < 3; ++c)
        if (std::abs(r.s.med[c] - kA[c]) >= 10 && std::abs(r.s.med[c] - kB[c]) >= 10) return true;
    return false;
}

// T6 / T6b: bf10_solid, blend 1.0 s, soft load of bf10_solid_b; frames paced ~16 ms until 2.0 s of WALL time.
// `changeAt` < 0: no canvas change (T6); else the canvas switches 1920x1080 -> 1080x1920 that long after the load.
void softCut(Rig& rig, double changeAt)
{
    auto src = makeSource(kSolid);
    src->getPresetSelector().setBlendSeconds(1.0f);   // applied by the warm-up frames, before the soft load
    auto next = Clock::now();
    GLuint tex = 0;
    for (int f = 0; f < 30; ++f) { tex = rig.render(*src, 1920, 1080); pace(next); }
    src->loadPreset(kSolidB, true);
    const auto t0 = Clock::now();
    std::vector<SoftCutRead> reads;
    std::vector<SoftCutRead> tail;   // the last 5 frames
    int frame = 0, sinceChange = -1;
    bool changed = false;
    while (true)
    {
        const double t = std::chrono::duration<double>(Clock::now() - t0).count();
        if (t >= 2.0) break;
        if (changeAt >= 0 && !changed && t >= changeAt) { changed = true; sinceChange = 0; }
        const int w = changed ? 1080 : 1920, h = changed ? 1920 : 1080;
        tex = rig.render(*src, w, h);
        ++frame;
        if (sinceChange >= 0) ++sinceChange;
        const double tr = std::chrono::duration<double>(Clock::now() - t0).count();
        const bool extra = sinceChange >= 1 && sinceChange <= 3;
        if (frame % 10 == 0 || extra)
        {
            if (extra) std::printf("DATA soft-cut change+%d\n", sinceChange);
            reads.push_back(readSoftCut(tex, w, h, tr));
            // +1 / +2 frames after a change may still be re-warming: the box clause starts at +3.
            if (sinceChange == 1 || sinceChange == 2) reads.back().inBox = -1.0;
        }
        else if (tr >= 1.85)   // the last frames before the 2.0 s end: keep the last 5
        {
            tail.push_back(readSoftCut(tex, w, h, tr));
            if (tail.size() > 5) tail.erase(tail.begin());
        }
        pace(next);
    }
    for (const auto& r : tail) reads.push_back(r);
    std::printf("DATA soft-cut frames=%d reads=%zu\n", frame, reads.size());
    bool anyMid = false;
    for (const auto& r : reads)
    {
        INFO("t=" << r.t << " size=" << r.w << "x" << r.h << " tex=" << r.tw << "x" << r.th << " median=(" << r.s.med[0]
                  << "," << r.s.med[1] << "," << r.s.med[2] << "," << r.s.med[3] << ") alpha255=" << r.s.alpha255
                  << "% in_box=" << r.inBox << "% near_B=" << r.nearB << "%");
        CHECK(r.tw == r.w);
        CHECK(r.th == r.h);
        CHECK(r.s.alpha255 == 100.0);
        if (r.inBox >= 0.0) CHECK(r.inBox == 100.0);
        if (r.t > 1.5) CHECK(r.nearB >= 99.99);
        anyMid = anyMid || midTransition(r);
    }
    if (changeAt < 0)   // T6 only: the transition itself drew into our FBO, and the window ends on B
    {
        const auto last = std::max_element(reads.begin(), reads.end(),
                                           [](const SoftCutRead& a, const SoftCutRead& b) { return a.t < b.t; });
        const double lastNearB = last == reads.end() ? 0.0 : last->nearB;
        std::printf("DATA soft-cut mid_transition_read=%d last_read_near_B=%.4f%%\n", anyMid ? 1 : 0, lastNearB);
        INFO("a read mid-transition (>= 5 % near A AND >= 5 % near B, or a median >= 10 from both): " << anyMid
             << "; the window's last read near_B=" << lastNearB << "%");
        CHECK(anyMid);
        CHECK(lastNearB >= 95.0);
    }
    src->releaseGL();
}
} // namespace

// T6 -- soft cut in wall time (amendment 11; mid-transition clause per rulings-bf10-s2.md STOP 2): every read alpha 255
// inside the A..B box, one read mid-transition (wipe or cross-fade), the window's last read >= 95 % near B, every read
// after 1.5 s equals B.
TEST_CASE("T6 a MilkDrop soft cut draws into the canvas texture", "[bf10][T6]")
{
    Rig rig;
    REQUIRE_GL(rig);
    softCut(rig, -1.0);
}

// T6b -- the same soft cut with a canvas change (1920x1080 -> 1080x1920) ~0.3 s into it; extra reads at change +1..+3.
TEST_CASE("T6b a MilkDrop soft cut survives a canvas change", "[bf10][T6b]")
{
    Rig rig;
    REQUIRE_GL(rig);
    softCut(rig, 0.3);
}
