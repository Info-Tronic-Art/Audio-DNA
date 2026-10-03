// bf10 S2 scratch: render through the REAL ProjectMSource (lane build) and write the returned texture as a PPM so a
// human can LOOK at a frame. Not part of the repo.
#include <juce_opengl/juce_opengl.h>
#include <OpenGL/OpenGL.h>
#include "sources/ProjectMSource.h"
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>
using namespace juce::gl;
int main(int argc, char** argv)
{
    if (argc < 5) return 2;
    const char* preset = argv[1]; const int W = atoi(argv[2]), H = atoi(argv[3]); const char* out = argv[4];
    const int frames = argc > 5 ? atoi(argv[5]) : 90;
    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core, (CGLPixelFormatAttribute) 0 };
    CGLPixelFormatObj pf; GLint n; CGLChoosePixelFormat(attrs, &pf, &n);
    CGLContextObj ctx; CGLCreateContext(pf, nullptr, &ctx); CGLSetCurrentContext(ctx);
    juce::gl::loadFunctions();
    juce::OpenGLContext jc; ShaderManager sm(jc); FullscreenQuad quad; FeatureSnapshot snap;
    ProjectMSource src; src.setPresetLocked(true); src.loadPreset(preset, false);
    std::vector<float> pcm(512);
    GLuint tex = 0;
    auto next = std::chrono::steady_clock::now();
    for (int f = 0; f < frames; ++f)
    {
        for (int i = 0; i < 512; ++i) pcm[i] = 0.7f * std::sin(2.f * 3.14159f * 80.f * (f * 512 + i) / 48000.f) + 0.3f * std::sin(2.f * 3.14159f * 1500.f * (f * 512 + i) / 48000.f);
        src.feedAudio(pcm.data(), 512);
        tex = src.render(sm, quad, 0.f, W, H, snap);
        next += std::chrono::milliseconds(16); std::this_thread::sleep_until(next);
    }
    GLuint fbo; glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    std::vector<unsigned char> px((size_t) W * H * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    FILE* fp = fopen(out, "wb"); fprintf(fp, "P6\n%d %d\n255\n", W, H);
    long a255 = 0;
    for (int y = H - 1; y >= 0; --y) for (int x = 0; x < W; ++x) { const unsigned char* p = &px[((size_t) y * W + x) * 4]; fwrite(p, 1, 3, fp); a255 += p[3] == 255; }
    fclose(fp);
    printf("%s %dx%d frames=%d alpha255=%.3f%%\n", out, W, H, frames, 100.0 * a255 / ((double) W * H));
    src.releaseGL();
    return 0;
}
