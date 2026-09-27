// test_source_defaults_gl -- the source/effect DEFAULTS gate (s-rta-0927 source-defects,
// .harmony/.reports/s-rta-0927/plan-source-defects.md T2): a procedural source at its REGISTERED defaults must draw a
// visible picture, at every time a user meets and on every canvas size we ship.
//
// The shipped GLSL (src/render/EmbeddedShaders.h) is compiled and rendered offscreen in a PRIVATE CGL context (no
// window, no drawable, nothing ever shown on a screen), with the registered default of every parameter PARSED from
// src/sources/SourceRegistry.cpp / src/effects/EffectLibrary.cpp at test time -- so the test renders exactly what a new
// clip gets (MainComponent seeds a clip from the registry defaults) and a registry edit is covered with no test edit.
// Uniforms match ProceduralSource::render: u_time, u_resolution, every param by uniform name, audio features 0
// (silence). Pixels are decoded here: "visible" = p99.5 of the per-pixel max channel >= 16 AND >= 5% of pixels lit
// (max channel > 16) -- the Tier-1 diagnosis metric (tier1-diag.md H3). On a machine with no GL pixel format every
// case SKIPs loudly.
#include <catch2/catch_test_macros.hpp>
#include "render/EmbeddedShaders.h"
#include <juce_opengl/juce_opengl.h>   // juce_gl.h must precede any Apple GL header
#include <OpenGL/OpenGL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef AUDIODNA_SRC_DIR
#error "AUDIODNA_SRC_DIR must point at src/"
#endif

using namespace juce::gl;

namespace
{
std::string readFile(const std::string& rel)
{
    std::ifstream in(std::string(AUDIODNA_SRC_DIR) + "/" + rel);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

struct Param { std::string name, uniform; float value; };

// The registered params of source `id`, in registry order: the addParam calls between registerSource("id" and the next
// registerSource( in SourceRegistry.cpp. Every addParam in the span must parse (a call this regex cannot read FAILS,
// it is never silently dropped).
std::vector<Param> registryParams(const std::string& id)
{
    const std::string reg = readFile("sources/SourceRegistry.cpp");
    const auto start = reg.find("registerSource(\"" + id + "\"");
    REQUIRE(start != std::string::npos);
    auto end = reg.find("registerSource(", start + 1);
    if (end == std::string::npos) end = reg.size();
    const std::string span = reg.substr(start, end - start);
    std::vector<Param> out;
    static const std::regex re(R"re(addParam\("([^"]+)",\s*"(\w+)",\s*([0-9.]+)f\))re");
    for (auto it = std::sregex_iterator(span.begin(), span.end(), re); it != std::sregex_iterator(); ++it)
        out.push_back({ (*it)[1].str(), (*it)[2].str(), std::stof((*it)[3].str()) });
    size_t calls = 0;
    for (auto p = span.find("addParam("); p != std::string::npos; p = span.find("addParam(", p + 1)) ++calls;
    INFO("source " << id << ": " << calls << " addParam calls, " << out.size() << " parsed");
    REQUIRE(calls == out.size());
    REQUIRE(!out.empty());
    return out;
}

// The registered params of effect `name` (EffectLibrary.cpp registerEffect({"name", "cat", "key", { {..}, .. }).
std::vector<Param> effectParams(const std::string& name)
{
    const std::string lib = readFile("effects/EffectLibrary.cpp");
    const auto start = lib.find("registerEffect({\"" + name + "\"");
    REQUIRE(start != std::string::npos);
    const auto end = lib.find("}});", start);
    REQUIRE(end != std::string::npos);
    const std::string span = lib.substr(start, end - start);
    std::vector<Param> out;
    static const std::regex re(R"re(\{"([^"]+)",\s*"(\w+)",\s*([0-9.]+)f\})re");
    for (auto it = std::sregex_iterator(span.begin(), span.end(), re); it != std::sregex_iterator(); ++it)
        out.push_back({ (*it)[1].str(), (*it)[2].str(), std::stof((*it)[3].str()) });
    REQUIRE(!out.empty());
    return out;
}

std::vector<Param> with(std::vector<Param> ps, const std::string& uniform, float v)
{
    bool found = false;
    for (auto& p : ps)
        if (p.uniform == uniform) { p.value = v; found = true; }
    REQUIRE(found);
    return ps;
}

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

using Pixels = std::vector<uint8_t>;   // RGBA8

struct Stats { double mean = 0, p995 = 0, lit = 0, sd = 0; };

Stats stats(const Pixels& px, int w, int h)
{
    const size_t n = static_cast<size_t>(w) * static_cast<size_t>(h);
    std::vector<int> mx(n);
    double sum = 0, msum = 0, msq = 0;
    size_t lit = 0;
    for (size_t i = 0; i < n; ++i)
    {
        const uint8_t* p = &px[i * 4];
        const int m = std::max({ int(p[0]), int(p[1]), int(p[2]) });
        mx[i] = m;
        sum += p[0] + p[1] + p[2];
        msum += m; msq += double(m) * m;
        if (m > 16) ++lit;
    }
    Stats s;
    s.mean = sum / (3.0 * double(n));
    s.lit = double(lit) / double(n);
    const double mm = msum / double(n);
    s.sd = std::sqrt(std::max(0.0, msq / double(n) - mm * mm));
    auto k = static_cast<size_t>(std::floor(0.995 * double(n - 1)));
    std::nth_element(mx.begin(), mx.begin() + static_cast<long>(k), mx.end());
    s.p995 = mx[k];
    return s;
}

double psnr(const Pixels& a, const Pixels& b)
{
    double se = 0; size_t n = 0;
    for (size_t i = 0; i < a.size(); i += 4)
        for (size_t c = 0; c < 3; ++c) { const double d = double(a[i + c]) - double(b[i + c]); se += d * d; ++n; }
    if (se == 0) return 1e9;
    return 10.0 * std::log10(255.0 * 255.0 / (se / double(n)));
}

// One private context + a fullscreen quad; renders a fragment shader into an RGBA8 FBO of any size.
struct Rig
{
    CGLContextObj ctx = nullptr;
    GLuint vao = 0, vbo = 0;
    bool ok = false;

    Rig()
    {
        ctx = makeContext();
        ok = ctx != nullptr;
        if (!ok) return;
        CGLSetCurrentContext(ctx);
        juce::gl::loadFunctions();   // dlsym-based on macOS: needs no JUCE context
        // pos.xy, uv.xy -- EmbeddedShaders::vertex: a_position @0, a_texCoord @1
        const float quad[] = { -1, -1, 0, 0,   1, -1, 1, 0,   -1, 1, 0, 1,
                                1, -1, 1, 0,   1,  1, 1, 1,   -1, 1, 0, 1 };
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, reinterpret_cast<void*>(8));
    }
    ~Rig()
    {
        if (!ok) return;
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        CGLSetCurrentContext(nullptr);
        CGLDestroyContext(ctx);
    }

    static GLuint shader(GLenum type, const char* src)
    {
        const GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint okc = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &okc);
        if (!okc)
        {
            char log[2048] = {};
            glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            FAIL("shader compile failed: " << log);
        }
        return s;
    }

    GLuint program(const char* frag)
    {
        const GLuint v = shader(GL_VERTEX_SHADER, EmbeddedShaders::vertex);
        const GLuint f = shader(GL_FRAGMENT_SHADER, frag);
        const GLuint p = glCreateProgram();
        glAttachShader(p, v); glAttachShader(p, f);
        glBindAttribLocation(p, 0, "a_position");
        glBindAttribLocation(p, 1, "a_texCoord");
        glLinkProgram(p);
        GLint okl = 0;
        glGetProgramiv(p, GL_LINK_STATUS, &okl);
        REQUIRE(okl);
        glDeleteShader(v); glDeleteShader(f);
        return p;
    }

    // Render `frag` at w x h, time t, params by uniform name; `inputTex` (if non-zero) bound to unit 0 as u_texture.
    Pixels render(const char* frag, int w, int h, float t, const std::vector<Param>& params, GLuint inputTex = 0)
    {
        const GLuint prog = program(frag);
        GLuint tex = 0, fbo = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        REQUIRE(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        glViewport(0, 0, w, h);
        glDisable(GL_BLEND);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        if (auto l = glGetUniformLocation(prog, "u_time"); l >= 0) glUniform1f(l, t);
        if (auto l = glGetUniformLocation(prog, "u_resolution"); l >= 0) glUniform2f(l, float(w), float(h));
        for (const auto& p : params)
            if (auto l = glGetUniformLocation(prog, p.uniform.c_str()); l >= 0) glUniform1f(l, p.value);
        if (inputTex != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputTex);
            if (auto l = glGetUniformLocation(prog, "u_texture"); l >= 0) glUniform1i(l, 0);
        }
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        Pixels px(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        REQUIRE(glGetError() == GL_NO_ERROR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
        glDeleteProgram(prog);
        return px;
    }
};

#define REQUIRE_GL(rig) do { if (!(rig).ok) SKIP("no CGL OpenGL 4.1 pixel format on this machine -- offscreen GL gate NOT run"); } while (0)

// Visible at (w, h, t): print the numbers either way so a RED run names what it saw.
void requireVisible(Rig& rig, const char* id, const char* frag, int w, int h, float t, const std::vector<Param>& ps)
{
    const Stats s = stats(rig.render(frag, w, h, t, ps), w, h);
    INFO(id << " " << w << "x" << h << " t=" << t << ": mean=" << s.mean << " p99.5=" << s.p995 << " lit=" << s.lit);
    CHECK(s.p995 >= 16.0);
    CHECK(s.lit >= 0.05);
}

const std::vector<std::pair<int, int>> kSizes = { { 256, 256 }, { 1920, 1080 } };
} // namespace

// A1.1 -- Julia Set at its registered defaults frames the set's boundary, not its black interior.
TEST_CASE("julia_set draws a picture at its registered defaults", "[source-defaults][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("julia_set");
    for (auto [w, h] : kSizes)
        for (float t : { 0.0f, 1.13f, 10.0f })
            requireVisible(rig, "julia_set", EmbeddedShaders::sourceJuliaSet, w, h, t, ps);
}

// A1.2 -- Burning Ship at its registered defaults shows the ship (the shader's documented 0.5/0.5, zoom 0).
TEST_CASE("burning_ship draws a picture at its registered defaults", "[source-defaults][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("burning_ship");
    for (auto [w, h] : kSizes)
        for (float t : { 0.0f, 1.13f, 10.0f })
            requireVisible(rig, "burning_ship", EmbeddedShaders::sourceBurningShip, w, h, t, ps);
}
