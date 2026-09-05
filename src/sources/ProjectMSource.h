#pragma once
#include "sources/ProceduralSource.h"
#include "sources/ProjectMPresetManager.h"
#include "sources/PresetSelector.h"
#include <memory>
#include <vector>
#include <mutex>

#ifdef AUDIODNA_HAS_PROJECTM
#include <projectM-4/projectM.h>
#endif

// ProjectMSource: renders MilkDrop presets via libprojectM-4.
//
// Unlike other ProceduralSources, this does NOT use a GLSL shader.
// Instead, it delegates to projectM's internal renderer which manages
// its own shaders, feedback loops, and warp meshes.
//
// The render() override:
//   1. Saves current GL state
//   2. Feeds PCM audio to projectM
//   3. Calls projectm_opengl_render_frame_fbo()
//   4. Restores GL state
//   5. Returns the FBO texture
class ProjectMSource : public ProceduralSource
{
public:
    ProjectMSource();
    ~ProjectMSource() override;

    // GL lifecycle
    void initGL(int width, int height) override;
    void releaseGL() override;
    void resize(int width, int height) override;
    void reset() override;

    // Render — feeds audio, calls projectM, returns texture
    GLuint render(ShaderManager& shaderMgr, FullscreenQuad& quad,
                  float time, int width, int height,
                  const FeatureSnapshot& snapshot) override;

    // Feed PCM audio samples to projectM. Call before render().
    // Thread-safe: copies samples into internal buffer.
    void feedAudio(const float* samples, int numSamples);

    // Preset management. The manager is now MainComponent-owned (hoisted out
    // 2026-09-04 — pure file/JSON scanning, no GL dependency, so it can be
    // scanned unconditionally at startup before any GL context exists; see
    // .harmony/milkdrop-autoload-rootcause.md). Renderer injects the pointer
    // here at GL-thread source-creation time. Non-owning; may be null if a
    // source is somehow created before MainComponent has wired one up.
    void setPresetManager(ProjectMPresetManager* mgr)
    {
        presetManager_ = mgr;
        presetSelector_.setPresetManager(mgr);
    }

    // Load a specific preset by file path.
    // Thread-safe: queues the load for the GL thread if called from another thread.
    void loadPreset(const std::string& path, bool smooth = true);

    // Load next/prev/random preset
    void nextPreset(bool smooth = true);
    void prevPreset(bool smooth = true);
    void randomPreset(bool smooth = true);

    // Lock/unlock current preset (prevents auto-switching)
    void setPresetLocked(bool locked) { presetLocked_ = locked; }
    bool isPresetLocked() const { return presetLocked_; }

    // Audio-driven preset selection
    PresetSelector& getPresetSelector() { return presetSelector_; }

    // Current preset info
    std::string getCurrentPresetName() const;
    std::string getCurrentPresetPath() const;

    ProjectMSource(const ProjectMSource&) = delete;
    ProjectMSource& operator=(const ProjectMSource&) = delete;

private:
    // GL state save/restore around projectM rendering
    struct GLState
    {
        GLint viewport[4];
        GLint framebuffer;
        GLint program;
        GLint vao;
        GLint activeTexture;
        GLint texture2D;
        GLboolean blend;
        GLboolean depthTest;
        GLboolean stencilTest;
        GLboolean cullFace;
        GLboolean scissorTest;
        GLint blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha;
        GLint depthFunc;
    };
    void saveGLState(GLState& state);
    void restoreGLState(const GLState& state);

#ifdef AUDIODNA_HAS_PROJECTM
    projectm_handle pm_ = nullptr;
#endif

    // Audio PCM buffer for projectM
    static constexpr int kPCMBufferSize = 2048;
    std::mutex pcmMutex_;
    std::vector<float> pcmBuffer_;
    int pcmSampleCount_ = 0;

    // Preset management. presetManager_ is non-owning (see setPresetManager
    // above) — the ProjectMPresetManager itself now lives in MainComponent.
    ProjectMPresetManager* presetManager_ = nullptr;
    PresetSelector presetSelector_;
    bool presetLocked_ = false;
    std::string currentPresetPath_;

    // Param application
    void applyParams();

    // Pending preset load (queued from non-GL threads)
    std::mutex pendingPresetMutex_;
    std::string pendingPresetPath_;
    bool hasPendingPreset_ = false;
    bool pendingPresetSmooth_ = false;
};
