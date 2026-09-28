#include "/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w10/tests/test_source_defaults_gl.cpp"
#include <cstdio>
#include <cstdlib>
static std::string slurp(const char* p) { std::ifstream in(p); std::stringstream ss; ss << in.rdbuf(); return ss.str(); }
TEST_CASE("explore", "[explore]")
{
    Rig rig; REQUIRE_GL(rig);
    const std::string frag = slurp(std::getenv("CAV_FRAG"));
    const int w = std::atoi(std::getenv("CAV_W")), h = std::atoi(std::getenv("CAV_H"));
    const float t0 = std::atof(std::getenv("CAV_T0")), t1 = std::atof(std::getenv("CAV_T1")), dt = std::atof(std::getenv("CAV_DT"));
    const char* out = std::getenv("CAV_OUT");
    auto ps = registryParams("crystal_cavern");
    int i = 0;
    for (float t = t0; t <= t1 + 1e-4f; t = t0 + dt * float(++i))
    {
        const Pixels px = rig.render(frag.c_str(), w, h, t, ps);
        char fn[1024]; std::snprintf(fn, sizeof fn, "%s/f_%05d_%.3f.ppm", out, i, t);
        FILE* f = std::fopen(fn, "wb"); std::fprintf(f, "P6\n%d %d\n255\n", w, h);
        for (int y = h - 1; y >= 0; --y) for (int x = 0; x < w; ++x) std::fwrite(&px[size_t((y * w + x) * 4)], 1, 3, f);
        std::fclose(f);
    }
}
TEST_CASE("gputime", "[perf]")
{
    Rig rig; REQUIRE_GL(rig);
    const int w = 1920, h = 1080;
    for (const char* v : { "mandelbulb", "julia_set_3d" })
    {
        std::string vs(v); float ax = -1; if (auto at = vs.find('@'); at != std::string::npos) { ax = std::stof(vs.substr(at + 1)); vs = vs.substr(0, at); }
        auto ps = registryParams(vs == "head" || vs == "fix" || vs == "base" ? std::string("crystal_cavern") : (vs.rfind("newton", 0) == 0 ? std::string("newton_3d") : vs));
        if (ax >= 0) ps = with(ps, "u_src_rotation_x", ax);
        const std::string frag = slurp((std::string(std::getenv("CAV_DIR")) + "/" + vs + ".frag").c_str());
        const GLuint prog = rig.program(frag.c_str());
        GLuint tex = 0, fbo = 0, q = 0;
        glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        glViewport(0, 0, w, h); glUseProgram(prog); glBindVertexArray(rig.vao);
        glUniform2f(glGetUniformLocation(prog, "u_resolution"), float(w), float(h));
        for (const auto& p : ps) if (auto l = glGetUniformLocation(prog, p.uniform.c_str()); l >= 0) glUniform1f(l, p.value);
        glGenQueries(1, &q);
        std::vector<double> ms;
        for (int i = 0; i < 240; ++i)
        {
            glUniform1f(glGetUniformLocation(prog, "u_time"), 0.125f * float(i));
            glBeginQuery(GL_TIME_ELAPSED, q); glDrawArrays(GL_TRIANGLES, 0, 6); glEndQuery(GL_TIME_ELAPSED);
            GLuint64 ns = 0; glGetQueryObjectui64v(q, GL_QUERY_RESULT, &ns); ms.push_back(double(ns) / 1e6);
        }
        std::sort(ms.begin(), ms.end());
        double sum = 0; for (double x : ms) sum += x;
        std::printf("%s: GPU ms/frame 1920x1080 over t=0..30 (240 frames): mean %.3f median %.3f p95 %.3f max %.3f\n", v, sum / ms.size(), ms[ms.size() / 2], ms[size_t(ms.size() * 0.95)], ms.back());
        glDeleteQueries(1, &q); glDeleteFramebuffers(1, &fbo); glDeleteTextures(1, &tex); glDeleteProgram(prog);
    }
}
