// bf10 S2: which call on ProjectMSource's init frame raises GL_INVALID_ENUM (0x500)? Steps mirror initGL + render().
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <projectM-4/projectM.h>
#include <cstdio>
#include <cstdlib>
static void chk(const char* step) {
    printf("%-40s", step);
    int n = 0;
    for (GLenum e; (e = glGetError()) != GL_NO_ERROR && n < 8; ++n) printf(" 0x%x", e);
    printf(n ? "\n" : " none\n");
}
int main(int argc, char** argv) {
    const char* preset = argv[1];
    CGLPixelFormatAttribute attrs[] = { kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core, (CGLPixelFormatAttribute) 0 };
    CGLPixelFormatObj pf; GLint np; CGLChoosePixelFormat(attrs, &pf, &np);
    CGLContextObj ctx; CGLCreateContext(pf, nullptr, &ctx); CGLSetCurrentContext(ctx);
    chk("context made current");
    const int W = 640, H = 360;
    GLuint tex, fbo; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenFramebuffers(1, &fbo); glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0); glBindFramebuffer(GL_FRAMEBUFFER, 0);
    chk("createFBO (ProceduralSource::createFBO)");
    projectm_handle pm = projectm_create();
    chk("projectm_create");
    projectm_set_window_size(pm, W, H); projectm_set_fps(pm, 60); projectm_set_mesh_size(pm, 48, 36); projectm_set_aspect_correction(pm, true);
    chk("set_window_size/fps/mesh/aspect");
    projectm_set_beat_sensitivity(pm, 1.0f); projectm_set_preset_duration(pm, 32.5); projectm_set_soft_cut_duration(pm, 2.0);
    chk("beat_sens/preset_duration/soft_cut");
    projectm_load_preset_file(pm, preset, false);
    chk("load_preset_file (hard)");
    for (int f = 0; f < 3; ++f) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo); glViewport(0, 0, W, H);
#ifdef STOCK
        projectm_opengl_render_frame(pm);
#else
        projectm_opengl_render_frame_fbo(pm, fbo);
#endif
        char b[64]; snprintf(b, sizeof b, "render frame %d", f); chk(b);
    }
    projectm_destroy(pm);
    chk("projectm_destroy");
    return 0;
}
