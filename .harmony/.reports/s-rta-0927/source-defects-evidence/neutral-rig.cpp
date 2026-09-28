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
#include <iostream>

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

// A1.3 -- Newton 3D is a heightfield: at its registered defaults the camera looks at it from ABOVE, and the
// auto-rotation orbits around it (yaw) instead of tumbling under the floor (t = 30 / 45 were under it).
TEST_CASE("newton_3d draws a picture at its registered defaults and keeps it while auto-rotating", "[source-defaults][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("newton_3d");
    for (auto [w, h] : kSizes)
        for (float t : { 0.0f, 1.13f, 30.0f, 45.0f })
            requireVisible(rig, "newton_3d", EmbeddedShaders::sourceNewton3D, w, h, t, ps);
}

// A4 -- Sierpinski's subdivision depth is capped by the canvas: on a canvas of height 2^(k-1) every pixel centre has a
// 1 in binary digit k of both coordinates, so level k marked EVERY pixel a hole (256x256 at defaults, 1024x1024 at
// Iterations 0.75 were solid black). 1920x1080 is not dyadic and must keep rendering.
TEST_CASE("sierpinski draws a picture on power-of-two canvases", "[source-defaults][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("sierpinski");
    requireVisible(rig, "sierpinski", EmbeddedShaders::sourceSierpinski, 256, 256, 1.13f, ps);
    requireVisible(rig, "sierpinski", EmbeddedShaders::sourceSierpinski, 512, 512, 1.13f, ps);
    requireVisible(rig, "sierpinski", EmbeddedShaders::sourceSierpinski, 1920, 1080, 1.13f, ps);
    requireVisible(rig, "sierpinski iter 0.75", EmbeddedShaders::sourceSierpinski, 1024, 1024, 1.13f,
                   with(ps, "u_src_iterations", 0.75f));
}

// A5 -- Crystal Cavern flies along +z through a cave that used to END (~1 unit): black forever after ~2.3 s at the
// default Speed. The cave now repeats along the flight axis; a camera passing through a crystal wall must not flash
// the whole frame one flat colour. Swept every 0.05 s over the first 20 s at a small size (visible AND not flat:
// the per-pixel max channel's standard deviation >= 4), plus fixed times at both shipped sizes and at Speed 0.
TEST_CASE("crystal_cavern keeps drawing the cave as the camera flies", "[source-defaults][gl]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = registryParams("crystal_cavern");
    for (auto [w, h] : kSizes)
        for (float t : { 0.0f, 1.13f, 5.0f, 10.0f, 30.0f, 60.0f })
            requireVisible(rig, "crystal_cavern", EmbeddedShaders::sourceCrystalCavern, w, h, t, ps);
    for (float t : { 5.0f, 8.0f, 30.0f })
        requireVisible(rig, "crystal_cavern speed 0", EmbeddedShaders::sourceCrystalCavern, 256, 256, t,
                       with(ps, "u_src_speed", 0.0f));
    int black = 0, flat = 0;
    std::ostringstream bad;
    for (int i = 0; i <= 400; ++i)
    {
        const float t = 0.05f * float(i);
        const Stats s = stats(rig.render(EmbeddedShaders::sourceCrystalCavern, 96, 54, t, ps), 96, 54);
        const bool isBlack = s.p995 < 16.0 || s.lit < 0.05;
        const bool isFlat = !isBlack && s.sd < 4.0;
        if (isBlack) ++black;
        if (isFlat) ++flat;
        if ((isBlack || isFlat) && black + flat <= 12)
            bad << " t=" << t << (isBlack ? "(black" : "(flat") << " lit=" << s.lit << " sd=" << s.sd << ")";
    }
    INFO("96x54 sweep 0..20 s step 0.05: " << black << " black, " << flat << " flat frames;" << bad.str());
    CHECK(black == 0);
    CHECK(flat == 0);
}

// A3 -- Dot Field on first add is a visible effect (Pitfalls 17/27), and its "depth" control does something.
namespace
{
GLuint rampTexture(int w, int h)   // a horizontal grey ramp 0..255 with a vertical colour tint: every luma present
{
    Pixels p(static_cast<size_t>(w * h * 4));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            uint8_t* px = &p[static_cast<size_t>((y * w + x) * 4)];
            const int g = x * 255 / (w - 1);
            px[0] = static_cast<uint8_t>(g);
            px[1] = static_cast<uint8_t>(std::min(255, g * (h - y) / h + 40 * y / h));
            px[2] = static_cast<uint8_t>(std::min(255, g * y / h + 20));
            px[3] = 255;
        }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, p.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}
} // namespace

TEST_CASE("Dot Field is visible at its registered defaults and every control changes the picture", "[source-defaults][gl][effect]")
{
    Rig rig; REQUIRE_GL(rig);
    const auto ps = effectParams("Dot Field");
    const GLuint tex = rampTexture(512, 512);
    for (auto [w, h] : kSizes)
    {
        const Pixels dflt = rig.render(EmbeddedShaders::dotField, w, h, 1.13f, ps, tex);
        const Stats s = stats(dflt, w, h);
        INFO("Dot Field " << w << "x" << h << " defaults: mean=" << s.mean << " p99.5=" << s.p995 << " lit=" << s.lit);
        CHECK(s.mean >= 5.0);
        for (const auto& p : ps)
            for (float v : { 0.0f, 1.0f })
            {
                if (std::fabs(v - p.value) < 0.2f) continue;
                const double q = psnr(dflt, rig.render(EmbeddedShaders::dotField, w, h, 1.13f, with(ps, p.uniform, v), tex));
                INFO("Dot Field " << w << "x" << h << " '" << p.name << "' " << p.value << " -> " << v << ": PSNR " << q);
                CHECK(q < 55.0);
            }
    }
    glDeleteTextures(1, &tex);
}

namespace Base {
const char* dotField = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform sampler2D u_texture;
    uniform float u_dotfield_size;
    uniform float u_dotfield_spacing;
    uniform float u_dotfield_depth;
    void main() {
        float gridSize = (u_dotfield_spacing * 0.04) + 0.01;
        vec2 gridPos = floor(v_texCoord / gridSize) * gridSize + gridSize * 0.5;
        vec4 gridColor = texture(u_texture, gridPos);
        float luma = dot(gridColor.rgb, vec3(0.299, 0.587, 0.114));
        float dist = distance(v_texCoord, gridPos);
        float dotRadius = luma * u_dotfield_size * gridSize * 0.6;
        float d = smoothstep(dotRadius, dotRadius - 0.001, dist);
        fragColor = vec4(gridColor.rgb * d, d);
    }
)BASE";
const char* sourceMandelbulb = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_iterations;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_detail;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_glow;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    float mandelbulbDE(vec3 pos, float power, out float trap) {
        vec3 z = pos;
        float dr = 1.0, r = 0.0;
        trap = 1e10;
        for (int i = 0; i < 12; i++) {
            r = length(z);
            if (r > 2.0) break;
            trap = min(trap, length(z));
            float theta = acos(clamp(z.z / r, -1.0, 1.0));
            float phi = atan(z.y, z.x);
            dr = pow(r, power - 1.0) * power * dr + 1.0;
            float zr = pow(r, power);
            theta *= power; phi *= power;
            z = zr * vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
            z += pos;
        }
        return 0.5 * log(r) * r / dr;
    }

    vec3 calcNormal(vec3 p, float pw) {
        vec2 e = vec2(0.001, 0.0);
        float dummy;
        return normalize(vec3(
            mandelbulbDE(p+e.xyy,pw,dummy) - mandelbulbDE(p-e.xyy,pw,dummy),
            mandelbulbDE(p+e.yxy,pw,dummy) - mandelbulbDE(p-e.yxy,pw,dummy),
            mandelbulbDE(p+e.yyx,pw,dummy) - mandelbulbDE(p-e.yyx,pw,dummy)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        float rawPow = u_src_power;
        float power = 2.0 + rawPow * rawPow * 14.0;

        // Auto-rotation: 0.5 = stopped, <0.5 = reverse, >0.5 = forward
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float angleX = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float angleY = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(angleY) * rotX(angleX);

        // Camera distance: zoom [0,1] -> [5.0, 0.3]
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 target = vec3(0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        // Cross-section: single or multi-slice
        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;

        float glowAmt = u_src_glow * u_src_glow * 4.0;
        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        float trapVal = 0.0;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float trap;
            float d = mandelbulbDE(p, power, trap);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 500.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float tdmy;
                    float td = mandelbulbDE(tp, power, tdmy);
                    float tg = 1.0 / (1.0 + td * td * 500.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            // Feedback: modulate glow with periodic rings for echo/feedback look
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; trapVal = trap; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, power);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + trapVal * 2.0;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.3) * glow * 0.015 * (1.0 + glowAmt);

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)BASE";
const char* sourceJuliaSet3D = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_cx;
    uniform float u_src_cy;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_iterations;
    uniform float u_src_color_shift;
    uniform float u_src_slice;
    uniform float u_src_glow;
    uniform float u_src_palette;
    uniform float u_src_location;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    // Quaternion multiply: q = (x, y, z, w) = xi + yj + zk + w
    vec4 qmul(vec4 a, vec4 b) {
        return vec4(
            a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
            a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
            a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
            a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z);
    }

    float julia3DDE(vec3 pos, vec4 c, out float trap) {
        vec4 z = vec4(pos, 0.0);
        float dz = 1.0;
        trap = 1e10;
        for (int i = 0; i < 10; i++) {
            dz = 2.0 * length(z) * dz;
            z = qmul(z, z) + c;
            trap = min(trap, length(z.xyz));
            if (dot(z, z) > 4.0) break;
        }
        float r = length(z);
        return 0.5 * r * log(r) / max(dz, 0.0001);
    }

    vec3 calcNormal(vec3 p, vec4 c) {
        vec2 e = vec2(0.001, 0.0);
        float dummy;
        return normalize(vec3(
            julia3DDE(p+e.xyy,c,dummy) - julia3DDE(p-e.xyy,c,dummy),
            julia3DDE(p+e.yxy,c,dummy) - julia3DDE(p-e.yxy,c,dummy),
            julia3DDE(p+e.yyx,c,dummy) - julia3DDE(p-e.yyx,c,dummy)));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;

        // Preset quaternion c-values
        vec4 presets[6];
        presets[0] = vec4(-0.2, 0.6, 0.2, 0.0);
        presets[1] = vec4(-0.213, -0.0410, -0.563, -0.560);
        presets[2] = vec4(-0.1, 0.6, 0.0, 0.0);
        presets[3] = vec4(-0.291, -0.399, 0.339, 0.437);
        presets[4] = vec4(0.185, 0.478, 0.125, -0.392);
        presets[5] = vec4(-0.4, 0.6, 0.2, -0.1);

        vec4 c;
        float locIdx = u_src_location * 5.0;
        if (locIdx > 0.1) {
            int li = int(locIdx);
            c = presets[min(li, 5)];
        } else {
            float rawCx = (u_src_cx - 0.5);
            float rawCy = (u_src_cy - 0.5);
            c = vec4(sign(rawCx)*rawCx*rawCx*4.0, sign(rawCy)*rawCy*rawCy*4.0, 0.0, 0.0);
        }

        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 0, camDist);
        vec3 fwd = normalize(-ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        float sliceZ = (u_src_slice - 0.5) * 3.0;
        bool useSlice = abs(u_src_slice - 0.5) > 0.01;
        int sliceCount = int(u_src_slice_count * 4.0) + 1;
        float sliceSpacing = 0.1 + u_src_slice_dist * 0.8;
        float glowAmt = u_src_glow * u_src_glow * 4.0;
        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0, glow = 0.0, trapVal = 0.0;
        bool hit = false;
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float trap;
            float d = julia3DDE(p, c, trap);
            if (useSlice) {
                if (sliceCount <= 1) {
                    d = max(d, abs(p.z - sliceZ) - 0.02);
                } else {
                    float md = 1e10;
                    for (int s = 0; s < 5; s++) {
                        if (s >= sliceCount) break;
                        float z = sliceZ + float(s - sliceCount/2) * sliceSpacing;
                        md = min(md, abs(p.z - z) - 0.02);
                    }
                    d = max(d, md);
                }
            }
            float glowBase = 1.0 / (1.0 + d * d * 500.0);
            float trailGlow = 0.0;
            if (d < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float tdmy;
                    float td = julia3DDE(tp, c, tdmy);
                    float tg = 1.0 / (1.0 + td * td * 500.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            // Feedback: modulate glow with periodic rings for echo/feedback look
            if (u_src_feedback > 0.01 && d < 0.5) {
                float fbRings = sin(d * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (d < 0.001) { hit = true; trapVal = trap; break; }
            t += d;
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            vec3 p = ro + rd * t;
            vec3 n = calcNormal(p, c);
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n, light), 0.0) * 0.7 + 0.3;
            float hue = u_src_color_shift + trapVal * 2.0;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.15, 0.1, 0.3) * glow * 0.015 * (1.0 + glowAmt);

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)BASE";
const char* sourceSierpinski = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_mode;
    uniform float u_src_zoom;
    uniform float u_src_iterations;
    uniform float u_src_rotation;
    uniform float u_src_color_shift;
    uniform float u_src_dive_speed;
    uniform float u_src_palette;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        // Zoom: continuous with dive speed, no wrap
        float diveRate = u_src_dive_speed * u_src_dive_speed * 2.0;
        float zoomExp = u_src_zoom * 5.0 + u_time * diveRate * 0.12;
        zoomExp = min(zoomExp, 5.0);
        float zoomPow = exp(zoomExp * 0.5);
        float rot = u_time * (u_src_rotation - 0.5) * 1.0;
        float c = cos(rot), s = sin(rot);
        uv = vec2(uv.x*c - uv.y*s, uv.x*s + uv.y*c);
        uv /= zoomPow;
        // Auto-scale iterations with zoom
        int baseIter = int(u_src_iterations * 12.0) + 3;
        int zoomIter = int(zoomExp * 1.5);
        int maxIter = min(baseIter + zoomIter, 20);
        bool isCarpet = u_src_mode > 0.5;
        float val = 1.0;
        float iterFrac = 0.0;
        if (isCarpet) {
            // Sierpinski Carpet: remove center cell at each scale
            vec2 p = fract(uv * 0.5 + 0.5);
            for (int i = 0; i < 20; i++) {
                if (i >= maxIter) break;
                p *= 3.0;
                vec2 cell = floor(p);
                if (cell.x == 1.0 && cell.y == 1.0) { val = 0.0; iterFrac = float(i)/float(maxIter); break; }
                p = fract(p);
            }
        } else {
            // Sierpinski Triangle: modular arithmetic approach
            // A point is in the gasket if at every scale, it's NOT in the upper-right quadrant
            vec2 p = fract(uv * 0.5 + 0.5);
            for (int i = 0; i < 20; i++) {
                if (i >= maxIter) break;
                p *= 2.0;
                vec2 cell = floor(p);
                // If both coordinates are >= 1 (upper-right), it's a hole
                if (cell.x >= 1.0 && cell.y >= 1.0) { val = 0.0; iterFrac = float(i)/float(maxIter); break; }
                p = fract(p);
            }
        }
        int palIdx = int(u_src_palette * 7.0);
        float hue = u_src_color_shift + iterFrac;
        vec3 col;
        if (u_src_palette < 0.01 && u_src_color_shift < 0.01) col = vec3(val);
        else col = val * fracPalette(hue, palIdx);
        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)BASE";
const char* sourceNewton3D = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_power;
    uniform float u_src_rotation_x;
    uniform float u_src_rotation_y;
    uniform float u_src_damping;
    uniform float u_src_color_shift;
    uniform float u_src_height;
    uniform float u_src_palette;
    uniform float u_src_zoom;
    uniform float u_src_speed;
    uniform float u_src_trail_dist;
    uniform float u_src_trail_fade;
    uniform float u_src_slice_count;
    uniform float u_src_slice_dist;
    uniform float u_src_feedback;
    uniform float u_rms;

    vec3 fracPalette(float t, int idx) {
        vec3 a,b,c,d;
        if(idx==0){a=vec3(.5,.2,.05);b=vec3(.5,.3,.15);c=vec3(1,.7,.4);d=vec3(0,.15,.2);}
        else if(idx==1){a=vec3(0,.3,.5);b=vec3(0,.3,.3);c=vec3(1,1,1);d=vec3(0,.2,.5);}
        else if(idx==2){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.1,.2);}
        else if(idx==3){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,0,0);}
        else if(idx==4){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(1,1,1);d=vec3(0,.33,.67);}
        else if(idx==5){a=vec3(.5,.5,.5);b=vec3(.5,.5,.5);c=vec3(2,1,1);d=vec3(.5,.2,.25);}
        else if(idx==6){a=vec3(.7,.8,1);b=vec3(.3,.2,0);c=vec3(1,1,1);d=vec3(.5,.6,.7);}
        else{a=vec3(.5,.3,.2);b=vec3(.5,.4,.3);c=vec3(1,.7,.4);d=vec3(0,.05,.3);}
        return a+b*cos(6.28318*(c*t+d));
    }

    mat3 rotY(float a) { float c=cos(a),s=sin(a); return mat3(c,0,s, 0,1,0, -s,0,c); }
    mat3 rotX(float a) { float c=cos(a),s=sin(a); return mat3(1,0,0, 0,c,-s, 0,s,c); }

    vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x); }
    vec2 cdiv(vec2 a, vec2 b) { return cmul(a, vec2(b.x,-b.y)) / dot(b,b); }

    // Newton iteration count as height + root coloring
    void newtonIterate(vec2 uv, int n, float damp, out float height, out int rootIdx) {
        vec2 z = uv;
        float minDist = 1e10;
        rootIdx = 0;
        int i;
        for (i = 0; i < 30; i++) {
            float r = length(z);
            if (r < 0.0001) break;
            float theta = atan(z.y, z.x);
            vec2 zn = pow(r, float(n)) * vec2(cos(float(n)*theta), sin(float(n)*theta));
            vec2 zn1 = pow(r, float(n-1)) * vec2(cos(float(n-1)*theta), sin(float(n-1)*theta));
            vec2 fz = zn - vec2(1.0, 0.0);
            vec2 fpz = float(n) * zn1;
            z -= damp * cdiv(fz, fpz);
            for (int k = 0; k < 8; k++) {
                if (k >= n) break;
                float ra = 6.28318 * float(k) / float(n);
                vec2 root = vec2(cos(ra), sin(ra));
                float d = length(z - root);
                if (d < minDist) { minDist = d; rootIdx = k; }
            }
            if (minDist < 0.001) break;
        }
        height = 1.0 - float(i) / 30.0;
    }

    float newtonDE(vec3 p, int n, float damp, float heightScale) {
        float h; int root;
        newtonIterate(p.xz, n, damp, h, root);
        return p.y - h * heightScale;
    }

    void main() {
        vec2 uv = (v_texCoord - 0.5) * 2.0;
        float aspect = u_resolution.x / u_resolution.y;
        uv.x *= aspect;
        int n = int(u_src_power * u_src_power * 5.0) + 3;
        float damp = 0.5 + u_src_damping * 1.0;
        float heightScale = 0.2 + u_src_height * 1.5;
        float autoSpeed = (u_src_speed - 0.5) * 2.0;
        float rx = u_src_rotation_x * 6.28318 + u_time * autoSpeed + 0.3;
        float ry = u_src_rotation_y * 6.28318 + u_time * autoSpeed * 0.7;
        mat3 rot = rotY(ry) * rotX(rx);
        float camDist = mix(5.0, 0.3, u_src_zoom);
        vec3 ro = rot * vec3(0, 1.5, camDist);
        vec3 target = vec3(0, 0.2, 0);
        vec3 fwd = normalize(target - ro);
        vec3 right = normalize(cross(fwd, vec3(0,1,0)));
        vec3 up = cross(right, fwd);
        vec3 rd = normalize(fwd + uv.x * right + uv.y * up);

        int numTrails = int(u_src_trail_dist * 5.0);
        float trailDecay = 0.3 + u_src_trail_fade * 0.5;

        float t = 0.0;
        float glow = 0.0;
        bool hit = false;
        vec3 hitP = vec3(0);
        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * t;
            float d = newtonDE(p, n, damp, heightScale);
            float glowBase = 1.0 / (1.0 + d * d * 100.0);
            float trailGlow = 0.0;
            if (abs(d) < 0.1 && numTrails > 0) {
                for (int tr = 0; tr < 5; tr++) {
                    if (tr >= numTrails) break;
                    float offset = float(tr + 1) * 0.02;
                    vec3 tp = p + rd * offset;
                    float td = newtonDE(tp, n, damp, heightScale);
                    float tg = 1.0 / (1.0 + td * td * 100.0);
                    trailGlow += tg * pow(trailDecay, float(tr + 1));
                }
            }
            glow += glowBase + trailGlow * 0.3;
            if (u_src_feedback > 0.01 && abs(d) < 0.5) {
                float fbRings = sin(abs(d) * u_src_feedback * 100.0) * 0.5 + 0.5;
                glow += fbRings * u_src_feedback * glowBase * 3.0;
            }
            if (abs(d) < 0.005) { hit = true; hitP = p; break; }
            t += max(d * 0.5, 0.005); // Conservative step
            if (t > 10.0) break;
        }
        vec3 col = vec3(0.0);
        if (hit) {
            // Normal via finite differences
            vec2 e = vec2(0.01, 0.0);
            vec3 n3 = normalize(vec3(
                newtonDE(hitP+e.xyy,n,damp,heightScale) - newtonDE(hitP-e.xyy,n,damp,heightScale),
                newtonDE(hitP+e.yxy,n,damp,heightScale) - newtonDE(hitP-e.yxy,n,damp,heightScale),
                newtonDE(hitP+e.yyx,n,damp,heightScale) - newtonDE(hitP-e.yyx,n,damp,heightScale)));
            vec3 light = normalize(vec3(1, 2, 3));
            float diff = max(dot(n3, light), 0.0) * 0.7 + 0.3;
            float h; int root;
            newtonIterate(hitP.xz, n, damp, h, root);
            float hue = float(root) / float(n) + u_src_color_shift;
            int palIdx = int(u_src_palette * 7.0);
            col = diff * fracPalette(hue, palIdx);
        }
        col += vec3(0.1, 0.15, 0.2) * glow * 0.008;

        col *= 0.8 + u_rms * 0.4;
        fragColor = vec4(col, 1.0);
    }
)BASE";
const char* sourceCrystalCavern = R"BASE(
    #version 410 core
    in vec2 v_texCoord;
    out vec4 fragColor;
    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_src_speed;
    uniform float u_src_crystal_size;
    uniform float u_src_reflectivity;
    uniform float u_src_light_color;
    uniform float u_src_fog;
    uniform float u_src_complexity;
    uniform float u_rms;
    uniform float u_bass;

    float hash(vec3 p) { return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453); }

    float caveDE(vec3 p, float crystalSize) {
        // Menger-like folded cave
        float scale = mix(1.5, 3.0, crystalSize);
        int iters = int(u_src_complexity * 4.0) + 2;
        float d = length(max(abs(p) - vec3(1.0), 0.0));
        float s = 1.0;
        for (int i = 0; i < 6; i++) {
            if (i >= iters) break;
            p = abs(p);
            if (p.x < p.y) p.xy = p.yx;
            if (p.x < p.z) p.xz = p.zx;
            if (p.y < p.z) p.yz = p.zy;
            p = p * scale - vec3(scale - 1.0);
            if (p.z < -0.5 * (scale - 1.0)) p.z += scale - 1.0;
            s *= scale;
        }
        return length(p) / s - 0.01;
    }

    void main() {
        vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;
        float t = u_time * (0.2 + u_src_speed * 0.8);
        vec3 ro = vec3(sin(t * 0.3) * 0.5, cos(t * 0.2) * 0.3, t);
        vec3 rd = normalize(vec3(uv, 0.8));
        // Rotate camera
        float ca = t * 0.1;
        float cc = cos(ca), ss = sin(ca);
        rd.xz = mat2(cc, -ss, ss, cc) * rd.xz;

        float totalDist = 0.0;
        vec3 col = vec3(0.0);
        float crystalSize = u_src_crystal_size;

        for (int i = 0; i < 80; i++) {
            vec3 p = ro + rd * totalDist;
            float d = caveDE(p, crystalSize);
            if (d < 0.002) {
                // Normal via gradient
                vec2 e = vec2(0.001, 0.0);
                vec3 n = normalize(vec3(
                    caveDE(p+e.xyy, crystalSize) - caveDE(p-e.xyy, crystalSize),
                    caveDE(p+e.yxy, crystalSize) - caveDE(p-e.yxy, crystalSize),
                    caveDE(p+e.yyx, crystalSize) - caveDE(p-e.yyx, crystalSize)
                ));
                // Light
                vec3 lightDir = normalize(vec3(sin(t), 0.5, cos(t)));
                float diff = max(dot(n, lightDir), 0.0);
                float spec = pow(max(dot(reflect(rd, n), lightDir), 0.0), 16.0 + u_src_reflectivity * 48.0);
                float hue = u_src_light_color;
                vec3 lightCol = 0.5 + 0.5 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
                col = lightCol * (diff * 0.6 + spec * u_src_reflectivity + 0.1);
                // Fog
                float fog = exp(-totalDist * (0.1 + u_src_fog * 0.5));
                col *= fog;
                col *= 0.8 + u_bass * 0.4;
                break;
            }
            totalDist += d;
            if (totalDist > 20.0) break;
        }
        fragColor = vec4(col, 1.0);
    }
)BASE";
}
TEST_CASE("NEUTRAL: new shaders vs pre-change shaders at the values old files hold", "[neutral]")
{
    Rig rig; REQUIRE_GL(rig);
    const GLuint tex = rampTexture(512, 512);
    auto dot = with(effectParams("Dot Field"), "u_dotfield_size", 0.3f);
    for (auto [w, h] : kSizes) {
        const double q = psnr(rig.render(EmbeddedShaders::dotField, w, h, 1.13f, dot, tex), rig.render(Base::dotField, w, h, 1.13f, dot, tex));
        std::cout << "NEUTRAL Dot Field size 0.3 depth 0.4 " << w << "x" << h << " PSNR(new, base) = " << q << std::endl;
    }
    struct C { const char* id; const char* nf; const char* bf; };
    const C cs[] = { {"mandelbulb", EmbeddedShaders::sourceMandelbulb, Base::sourceMandelbulb},
                     {"julia_set_3d", EmbeddedShaders::sourceJuliaSet3D, Base::sourceJuliaSet3D},
                     {"sierpinski", EmbeddedShaders::sourceSierpinski, Base::sourceSierpinski} };
    for (const auto& c : cs) {
        std::vector<Param> ps;
        try { ps = registryParams(c.id); } catch (...) {}
        for (auto [w, h] : kSizes)
          for (float t : {0.0f, 1.13f, 7.0f}) {
            const double q = psnr(rig.render(c.nf, w, h, t, ps), rig.render(c.bf, w, h, t, ps));
            std::cout << "NEUTRAL " << c.id << " defaults " << w << "x" << h << " t=" << t << " PSNR(new, base) = " << q << std::endl;
          }
    }
    auto np = with(registryParams("newton_3d"), "u_src_rotation_x", 0.0f);
    for (auto [w, h] : kSizes) {
        const double q = psnr(rig.render(EmbeddedShaders::sourceNewton3D, w, h, 0.0f, np), rig.render(Base::sourceNewton3D, w, h, 0.0f, np));
        std::cout << "NEUTRAL newton_3d t=0 " << w << "x" << h << " PSNR(new, base) = " << q << std::endl;
    }
    glDeleteTextures(1, &tex);
}
