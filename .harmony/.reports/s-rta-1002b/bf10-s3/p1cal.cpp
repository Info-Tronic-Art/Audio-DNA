// bf10 S3 scratch: choose m2's pinned preset P1 OFFLINE through the REAL lane ProjectMSource (no app).
// args: preset W H warm_s audio(0 silence|1 sine) -> prints the m2 tile metric for two reads 0.5 s apart.
#include <juce_opengl/juce_opengl.h>
#include <OpenGL/OpenGL.h>
#include "sources/ProjectMSource.h"
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>
#include <algorithm>
using namespace juce::gl;
static std::vector<unsigned char> readTex(GLuint tex, int W, int H)
{
    GLuint fbo; glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    std::vector<unsigned char> px((size_t) W * H * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &fbo);
    return px;
}
int main(int argc, char** argv)
{
    if (argc < 6) return 2;
    const char* preset = argv[1]; const int W = atoi(argv[2]), H = atoi(argv[3]);
    const double warm = atof(argv[4]); const int audio = atoi(argv[5]);
    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core, (CGLPixelFormatAttribute) 0 };
    CGLPixelFormatObj pf; GLint n; CGLChoosePixelFormat(attrs, &pf, &n);
    CGLContextObj ctx; CGLCreateContext(pf, nullptr, &ctx); CGLSetCurrentContext(ctx);
    juce::gl::loadFunctions();
    juce::OpenGLContext jc; ShaderManager sm(jc); FullscreenQuad quad; FeatureSnapshot snap;
    ProjectMSource src; src.setPresetLocked(true); src.loadPreset(preset, false);
    std::vector<float> pcm(512, 0.0f);
    GLuint tex = 0; std::vector<unsigned char> a, b;
    const auto t0 = std::chrono::steady_clock::now(); auto next = t0; long f = 0;
    auto el = [&] { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); };
    double ta = -1, tb = -1;
    while (true)
    {
        if (audio) for (int i = 0; i < 512; ++i) pcm[i] = 0.7f * std::sin(2.f * 3.14159f * 80.f * (f * 512 + i) / 48000.f) + 0.3f * std::sin(2.f * 3.14159f * 440.f * (f * 512 + i) / 48000.f);
        src.feedAudio(pcm.data(), 512);
        tex = src.render(sm, quad, 0.f, W, H, snap); ++f;
        if (ta < 0 && el() >= warm) { ta = el(); a = readTex(tex, W, H); }
        else if (ta >= 0 && el() >= ta + 0.5) { tb = el(); b = readTex(tex, W, H); break; }
        next += std::chrono::milliseconds(16); std::this_thread::sleep_until(next);
    }
    long a255 = 0, b255 = 0; for (size_t i = 3; i < a.size(); i += 4) { a255 += a[i] == 255; b255 += b[i] == 255; }
    int tiles = 0, bad = 0; double minFrac = 1.0; double lum = 0;
    for (int ty = 0; ty < H; ty += 64) for (int tx = 0; tx < W; tx += 64)
    {
        long ch = 0, tot = 0;
        for (int y = ty; y < std::min(H, ty + 64); ++y) for (int x = tx; x < std::min(W, tx + 64); ++x)
        {
            const unsigned char* p = &a[((size_t) y * W + x) * 4]; const unsigned char* q = &b[((size_t) y * W + x) * 4];
            int d = std::max({ std::abs(p[0] - q[0]), std::abs(p[1] - q[1]), std::abs(p[2] - q[2]) });
            ch += d > 8; ++tot;
        }
        double fr = (double) ch / tot; ++tiles; if (fr < 0.20) ++bad; minFrac = std::min(minFrac, fr);
    }
    for (size_t i = 0; i < b.size(); i += 4) lum += 0.2126 * b[i] + 0.7152 * b[i + 1] + 0.0722 * b[i + 2];
    printf("P1CAL %dx%d audio=%d ta=%.3f tb=%.3f frames=%ld alpha255=%.3f%%/%.3f%% tiles=%d bad=%d min_tile_changed=%.1f%% meanlum=%.1f\n",
           W, H, audio, ta, tb, f, 100.0 * a255 / ((double) W * H), 100.0 * b255 / ((double) W * H), tiles, bad, 100 * minFrac, lum / ((double) W * H));
    src.releaseGL();
    return 0;
}
