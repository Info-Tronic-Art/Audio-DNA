// bf10 lib-probe (s-rta-1002b, S1). Derived from diag10 pmprobe/probe.cpp, plus a mode flag and a fixture mode.
// Offscreen CGL 4.1 core context with NO drawable (framebuffer 0 has no storage), our own complete FBO bound.
//
//   libprobe plain   <preset> W H [frames=90]   our FBO bound, projectm_opengl_render_frame (stock behaviour)
//   libprobe fbo     <preset> W H [frames=90]   projectm_opengl_render_frame_fbo(pm, ourFBO)
//   libprobe solid   <preset> W H               fixture mode: 90 paced frames, uniform-colour stats at 30/60/90
//   libprobe time    <preset> W H               fixture mode: frame mean between reads 300 ms apart (3 pairs)
//   libprobe circle  <preset> W H               fixture mode: lit-blob count, bbox, aspect, centre offset
// Every fbo-mode call is followed by glGetError; all codes are printed as "ERRORS ..." (T5's baseline).
// Settings mirror ProjectMSource::initGL / applyParams (fps 60, mesh 48x36, aspect correction on,
// beat sensitivity 1.0, preset duration 32.5 s) plus preset lock (the tests lock the preset).
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <projectM-4/projectM.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <vector>

// Declared here (not only via the patched header) so that against the STOCK library the fbo modes fail at
// LINK time with an undefined symbol -- the I1 RED.
extern "C" void projectm_opengl_render_frame_fbo(projectm_handle instance, uint32_t framebuffer_object_id);

static std::map<unsigned, long> g_errors;
static long g_calls = 0;

struct Stats
{
    int w = 0, h = 0;
    int med[4] = {0, 0, 0, 0};
    double within6 = 0, alpha255 = 0, lit = 0, mean[3] = {0, 0, 0};
};

static Stats stats(const std::vector<unsigned char>& px, int W, int H)
{
    Stats s; s.w = W; s.h = H;
    const long N = (long) W * H;
    for (int c = 0; c < 4; ++c)
    {
        long hist[256] = {0};
        for (long i = 0; i < N; ++i) hist[px[i * 4 + c]]++;
        long acc = 0; int m = 0;
        for (; m < 256; ++m) { acc += hist[m]; if (acc * 2 >= N) break; }
        s.med[c] = m;
    }
    long in6 = 0, a255 = 0, lit = 0; double sum[3] = {0, 0, 0};
    for (long i = 0; i < N; ++i)
    {
        const unsigned char* p = &px[i * 4];
        bool ok = true;
        for (int c = 0; c < 3; ++c) { if (std::abs((int) p[c] - s.med[c]) > 6) ok = false; sum[c] += p[c]; }
        in6 += ok; a255 += (p[3] == 255); lit += (p[0] + p[1] + p[2] > 6);
    }
    s.within6 = 100.0 * in6 / N; s.alpha255 = 100.0 * a255 / N; s.lit = 100.0 * lit / N;
    for (int c = 0; c < 3; ++c) s.mean[c] = sum[c] / N;
    return s;
}

static void printStats(const char* tag, const Stats& s)
{
    printf("%s size=%dx%d median=(%d,%d,%d,%d) medmax=%d within6=%.3f%% alpha255=%.3f%% lit=%.3f%% mean=(%.2f,%.2f,%.2f)\n",
           tag, s.w, s.h, s.med[0], s.med[1], s.med[2], s.med[3], std::max(s.med[0], std::max(s.med[1], s.med[2])),
           s.within6, s.alpha255, s.lit, s.mean[0], s.mean[1], s.mean[2]);
}

int main(int argc, char** argv)
{
    if (argc < 5) { fprintf(stderr, "usage: libprobe <plain|fbo|solid|time|circle> <preset> W H [frames]\n"); return 2; }
    const std::string mode = argv[1];
    const char* preset = argv[2];
    const int W = atoi(argv[3]), H = atoi(argv[4]);
    const int frames = argc > 5 ? atoi(argv[5]) : 90;
    const bool useFbo = mode != "plain";

    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
                                        kCGLPFAAccelerated, kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
                                        (CGLPixelFormatAttribute) 0 };
    CGLPixelFormatObj pf; GLint n;
    if (CGLChoosePixelFormat(attrs, &pf, &n) != kCGLNoError || !pf) { printf("SKIP no CGL pixel format\n"); return 4; }
    CGLContextObj ctx; CGLCreateContext(pf, nullptr, &ctx); CGLSetCurrentContext(ctx);
    printf("GL_RENDERER=%s\n", (const char*) glGetString(GL_RENDERER));

    GLuint fbo, tex; glGenFramebuffers(1, &fbo); glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    printf("mode=%s preset=%s fbo=%u status_complete=%d\n", mode.c_str(), preset, fbo,
           glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    while (glGetError() != GL_NO_ERROR) {}

    projectm_handle pm = projectm_create();
    projectm_set_window_size(pm, W, H); projectm_set_fps(pm, 60); projectm_set_mesh_size(pm, 48, 36);
    projectm_set_aspect_correction(pm, true);
    projectm_set_beat_sensitivity(pm, 1.0f);
    projectm_set_preset_duration(pm, 32.5);
    projectm_set_preset_locked(pm, true);
    projectm_load_preset_file(pm, preset, false);
    while (glGetError() != GL_NO_ERROR) {}

    std::vector<float> pcm(512);
    std::vector<unsigned char> px((size_t) W * H * 4);
    auto renderOne = [&](int f) {
        for (int i = 0; i < 512; ++i) pcm[i] = 0.7f * std::sin(2.f * 3.14159f * 80.f * (f * 512 + i) / 48000.f);
        projectm_pcm_add_float(pm, pcm.data(), 512, PROJECTM_MONO);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, W, H);
        glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);  // a frame projectM did not draw stays black
        while (glGetError() != GL_NO_ERROR) {}
#ifndef PLAIN_ONLY   // -DPLAIN_ONLY: a build with no reference to the new symbol (links against the stock lib)
        if (useFbo) projectm_opengl_render_frame_fbo(pm, fbo);
        else
#else
        if (useFbo) { fprintf(stderr, "PLAIN_ONLY build\n"); std::exit(2); }
#endif
        projectm_opengl_render_frame(pm);
        ++g_calls;
        for (GLenum e; (e = glGetError()) != GL_NO_ERROR;) { if (g_errors[e]++ == 0) printf("FIRST_ERROR 0x%x at call %ld (frame arg %d)\n", e, g_calls, f); }
    };
    auto readBack = [&]() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    };
    auto pace = [](std::chrono::steady_clock::time_point& next) {
        next += std::chrono::milliseconds(16);
        std::this_thread::sleep_until(next);
    };
    int rc = 0;
    auto next = std::chrono::steady_clock::now();

    if (mode == "plain" || mode == "fbo")
    {
        for (int f = 0; f < frames; ++f) renderOne(f);
        GLint drawAfter = -1; glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawAfter);
        printf("draw binding after last call=%d (ours=%u)\n", drawAfter, fbo);
        readBack();
        Stats s = stats(px, W, H);
        printStats("FINAL", s);
        long litPx = std::lround(s.lit * W * H / 100.0);
        printf("pixels in OUR fbo after %d calls: %ld / %d non-black\n", frames, litPx, W * H);
    }
    else if (mode == "solid")
    {
        for (int f = 1; f <= 90; ++f)
        {
            renderOne(f); pace(next);
            if (f == 30 || f == 60 || f == 90)
            {
                readBack(); Stats s = stats(px, W, H);
                char tag[32]; snprintf(tag, sizeof tag, "READ f=%d", f); printStats(tag, s);
                const int mm = std::max(s.med[0], std::max(s.med[1], s.med[2]));
                const bool ok = s.within6 >= 99.9 && mm >= 64 && s.alpha255 >= 100.0;
                if (!ok) rc = 1;
            }
        }
    }
    else if (mode == "time")
    {
        int f = 0;
        for (; f < 30; ++f) { renderOne(f); pace(next); }
        double minDelta = 1e9;
        for (int pair = 0; pair < 3; ++pair)
        {
            readBack(); Stats a = stats(px, W, H);
            auto t0 = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - t0 < std::chrono::milliseconds(300)) { renderOne(f++); pace(next); }
            readBack(); Stats b = stats(px, W, H);
            const double ma = (a.mean[0] + a.mean[1] + a.mean[2]) / 3, mb = (b.mean[0] + b.mean[1] + b.mean[2]) / 3;
            const double dt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
            printf("PAIR %d dt=%.3fs meanA=%.2f meanB=%.2f delta=%.2f alpha255A=%.3f%% alpha255B=%.3f%% within6B=%.3f%%\n",
                   pair, dt, ma, mb, std::fabs(mb - ma), a.alpha255, b.alpha255, b.within6);
            minDelta = std::min(minDelta, std::fabs(mb - ma));
        }
        printf("TIME min_delta=%.2f frames=%d\n", minDelta, f);
        if (minDelta < 2.0) rc = 1;
        while (f < 90) { renderOne(f++); pace(next); }
    }
    else if (mode == "circle")
    {
        for (int f = 1; f <= 90; ++f) { renderOne(f); pace(next); }
        readBack(); Stats s = stats(px, W, H);
        printStats("READ f=90", s);
        // lit = max channel >= 128; 4-connected components
        std::vector<int> lab((size_t) W * H, 0);
        int comps = 0; long bestSize = 0; int bx0 = 0, by0 = 0, bx1 = -1, by1 = -1;
        std::vector<int> stack;
        auto litAt = [&](int i) { const unsigned char* p = &px[(size_t) i * 4]; return std::max(p[0], std::max(p[1], p[2])) >= 128; };
        for (int i = 0; i < W * H; ++i)
        {
            if (!litAt(i) || lab[i]) continue;
            ++comps; long size = 0; int x0 = W, y0 = H, x1 = -1, y1 = -1;
            stack.assign(1, i); lab[i] = comps;
            while (!stack.empty())
            {
                int j = stack.back(); stack.pop_back(); ++size;
                int x = j % W, y = j / W;
                x0 = std::min(x0, x); x1 = std::max(x1, x); y0 = std::min(y0, y); y1 = std::max(y1, y);
                const int nb[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                for (auto& d : nb)
                {
                    int nx = x + d[0], ny = y + d[1];
                    if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
                    int k = ny * W + nx;
                    if (!lab[k] && litAt(k)) { lab[k] = comps; stack.push_back(k); }
                }
            }
            if (size > bestSize) { bestSize = size; bx0 = x0; by0 = y0; bx1 = x1; by1 = y1; }
        }
        const int bw = bx1 - bx0 + 1, bh = by1 - by0 + 1;
        const double ratio = bh > 0 ? (double) bw / bh : 0;
        const double cx = (bx0 + bx1 + 1) / 2.0, cy = (by0 + by1 + 1) / 2.0;
        const double offx = 100.0 * (cx - W / 2.0) / W, offy = 100.0 * (cy - H / 2.0) / H;
        printf("CIRCLE blobs=%d largest=%ld bbox=(%d,%d)-(%d,%d) w=%d h=%d w/h=%.4f |w/h-1|=%.4f centre_off=(%.2f%%,%.2f%%)\n",
               comps, bestSize, bx0, by0, bx1, by1, bw, bh, ratio, std::fabs(ratio - 1), offx, offy);
        if (comps != 1 || std::fabs(ratio - 1) > 0.04 || std::fabs(offx) > 2 || std::fabs(offy) > 2) rc = 1;
    }
    else { fprintf(stderr, "unknown mode\n"); return 2; }

    printf("ERRORS calls=%ld", g_calls);
    if (g_errors.empty()) printf(" none");
    for (auto& e : g_errors) printf(" 0x%x x%ld", e.first, e.second);
    printf("\nRESULT %s\n", rc == 0 ? "OK" : "FAIL");
    projectm_destroy(pm);
    return rc;
}
