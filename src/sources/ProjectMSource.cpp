#include "sources/ProjectMSource.h"
#include <iostream>
#include <cstring>

using namespace juce::gl;

ProjectMSource::ProjectMSource()
    : ProceduralSource("projectm_visualizer", "MilkDrop Visualizer", "MilkDrop", "", false)
{
    pcmBuffer_.resize(kPCMBufferSize, 0.0f);

    // Parameters exposed in the clip inspector
    addParam("Beat Sensitivity", "u_src_beat_sens", 0.5f);
    addParam("Speed", "u_src_speed", 0.5f);
    addParam("Warp Amount", "u_src_warp", 0.5f);
    addParam("Decay", "u_src_decay", 0.5f);
    addParam("Gamma", "u_src_gamma", 0.5f);

    // presetSelector_'s manager pointer is wired externally via
    // setPresetManager() once Renderer creates this source on the GL thread
    // (see ProjectMSource.h) — presetManager_ no longer lives here.

    // L7-JUKE prerequisite fix: onAutoSwitch was declared and invoked by
    // PresetSelector::processFrame() but never assigned anywhere, so Jukebox
    // autopilot silently mutated the manager's bookkeeping (currentIndex_)
    // without ever pushing the new preset to the render engine after the
    // initial manual seed. loadPreset() already queues onto the GL thread
    // under a mutex (see loadPreset() below), so it's safe to call directly
    // from this callback even though processFrame() runs on the GL thread.
    presetSelector_.onAutoSwitch = [this](const std::string& path) {
        loadPreset(path, true);
    };
}

ProjectMSource::~ProjectMSource()
{
    // releaseGL() should have been called before this
}

void ProjectMSource::initGL(int width, int height)
{
    fboWidth_ = width;
    fboHeight_ = height;

    // Create our output FBO to capture projectM's render
    createFBO(outputFBO_, outputTex_, width, height);

#ifdef AUDIODNA_HAS_PROJECTM
    // Create projectM instance
    pm_ = projectm_create();
    if (pm_)
    {
        projectm_set_window_size(pm_, width, height);
        projectm_set_fps(pm_, 60);
        projectm_set_mesh_size(pm_, 48, 36);
        projectm_set_aspect_correction(pm_, true);

        // Apply initial params
        applyParams();

        // Load first preset if available
        if (presetManager_ && presetManager_->getPresetCount() > 0)
        {
            auto* preset = presetManager_->getCurrentPreset();
            if (preset)
                loadPreset(preset->path, false);
        }
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glInitialized_ = true;
}

void ProjectMSource::releaseGL()
{
#ifdef AUDIODNA_HAS_PROJECTM
    if (pm_)
    {
        projectm_destroy(pm_);
        pm_ = nullptr;
    }
#endif

    deleteFBO(outputFBO_, outputTex_);
    glInitialized_ = false;
}

void ProjectMSource::resize(int width, int height)
{
    if (width == fboWidth_ && height == fboHeight_)
        return;

    fboWidth_ = width;
    fboHeight_ = height;

    // Resize our capture FBO
    deleteFBO(outputFBO_, outputTex_);
    createFBO(outputFBO_, outputTex_, width, height);

#ifdef AUDIODNA_HAS_PROJECTM
    if (pm_)
        projectm_set_window_size(pm_, width, height);
#endif
}

void ProjectMSource::reset()
{
    // Clear FBO to black
    if (outputFBO_ != 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO_);
        glViewport(0, 0, fboWidth_, fboHeight_);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

GLuint ProjectMSource::render(ShaderManager& /*shaderMgr*/, FullscreenQuad& /*quad*/,
                               float /*time*/, int width, int height,
                               const FeatureSnapshot& snapshot)
{
    if (!glInitialized_)
        initGL(width, height);

    resize(width, height);

#ifdef AUDIODNA_HAS_PROJECTM
    if (!pm_)
        return outputTex_;

    // Process pending preset load (queued from non-GL threads)
    {
        std::lock_guard<std::mutex> lock(pendingPresetMutex_);
        if (hasPendingPreset_)
        {
            currentPresetPath_ = pendingPresetPath_;
            projectm_load_preset_file(pm_, pendingPresetPath_.c_str(), pendingPresetSmooth_);
            hasPendingPreset_ = false;
        }
    }

    // Apply parameter changes
    applyParams();

    // Auto-switch presets based on audio analysis
    if (!presetLocked_)
        presetSelector_.processFrame(snapshot);

    // Feed PCM audio to projectM
    {
        std::lock_guard<std::mutex> lock(pcmMutex_);
        if (pcmSampleCount_ > 0)
        {
            projectm_pcm_add_float(pm_, pcmBuffer_.data(),
                                    static_cast<unsigned int>(pcmSampleCount_),
                                    PROJECTM_MONO);
            pcmSampleCount_ = 0;
        }
    }

    // Save GL state
    GLState savedState;
    saveGLState(savedState);

    // projectM renders to FBO 0 (default framebuffer) internally,
    // managing its own multi-pass rendering pipeline.
    // We render to screen, then copy the result to our FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fboWidth_, fboHeight_);
    projectm_opengl_render_frame(pm_);

    // Copy the rendered result to our output FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outputFBO_);
    glBlitFramebuffer(0, 0, fboWidth_, fboHeight_,
                      0, 0, fboWidth_, fboHeight_,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Restore GL state
    restoreGLState(savedState);
#else
    // No projectM: render a placeholder pattern
    // (Rendered by base class shader if one were set, but we don't have one)
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO_);
    glViewport(0, 0, width, height);
    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    (void)snapshot;
#endif

    return outputTex_;
}

void ProjectMSource::feedAudio(const float* samples, int numSamples)
{
    std::lock_guard<std::mutex> lock(pcmMutex_);
    int toCopy = std::min(numSamples, kPCMBufferSize);
    std::memcpy(pcmBuffer_.data(), samples + (numSamples - toCopy),
                static_cast<size_t>(toCopy) * sizeof(float));
    pcmSampleCount_ = toCopy;
}

void ProjectMSource::loadPreset(const std::string& path, bool smooth)
{
    // Queue for GL thread — projectM compiles shaders internally
    std::lock_guard<std::mutex> lock(pendingPresetMutex_);
    pendingPresetPath_ = path;
    pendingPresetSmooth_ = smooth;
    hasPendingPreset_ = true;
}

void ProjectMSource::nextPreset(bool smooth)
{
    if (!presetManager_) return;
    auto* preset = presetManager_->nextPreset();
    if (preset)
        loadPreset(preset->path, smooth);
}

void ProjectMSource::prevPreset(bool smooth)
{
    if (!presetManager_) return;
    auto* preset = presetManager_->prevPreset();
    if (preset)
        loadPreset(preset->path, smooth);
}

void ProjectMSource::randomPreset(bool smooth)
{
    if (!presetManager_) return;
    auto* preset = presetManager_->randomPreset();
    if (preset)
        loadPreset(preset->path, smooth);
}

std::string ProjectMSource::getCurrentPresetName() const
{
    auto* preset = presetManager_ ? presetManager_->getCurrentPreset() : nullptr;
    return preset ? preset->name : "";
}

std::string ProjectMSource::getCurrentPresetPath() const
{
    return currentPresetPath_;
}

void ProjectMSource::applyParams()
{
#ifdef AUDIODNA_HAS_PROJECTM
    if (!pm_) return;

    for (int i = 0; i < getNumParams(); ++i)
    {
        const auto& p = getParam(i);
        if (p.uniformName == "u_src_beat_sens")
            projectm_set_beat_sensitivity(pm_, p.value * 2.0f); // 0-2 range
        else if (p.uniformName == "u_src_speed")
            projectm_set_preset_duration(pm_, 5.0 + (1.0 - static_cast<double>(p.value)) * 55.0); // 5-60 seconds
        else if (p.uniformName == "u_src_gamma")
        {
            // Gamma not directly settable in all projectM-4 builds;
            // placeholder for future API extension
        }
    }

    // L7-JUKE: Jukebox Blend slider — crossfade duration, not preset dwell
    // time (that's projectm_set_preset_duration above, driven by the
    // unrelated "Speed" param; don't confuse the two).
    projectm_set_soft_cut_duration(pm_, static_cast<double>(presetSelector_.getBlendSeconds()));
#endif
}

// === GL State Save/Restore ===

void ProjectMSource::saveGLState(GLState& state)
{
    glGetIntegerv(GL_VIEWPORT, state.viewport);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &state.framebuffer);
    glGetIntegerv(GL_CURRENT_PROGRAM, &state.program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state.vao);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &state.activeTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &state.texture2D);
    state.blend = glIsEnabled(GL_BLEND);
    state.depthTest = glIsEnabled(GL_DEPTH_TEST);
    state.stencilTest = glIsEnabled(GL_STENCIL_TEST);
    state.cullFace = glIsEnabled(GL_CULL_FACE);
    state.scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_BLEND_SRC_RGB, &state.blendSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &state.blendDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &state.blendSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &state.blendDstAlpha);
    glGetIntegerv(GL_DEPTH_FUNC, &state.depthFunc);
}

void ProjectMSource::restoreGLState(const GLState& state)
{
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
    glViewport(state.viewport[0], state.viewport[1],
               state.viewport[2], state.viewport[3]);
    glUseProgram(static_cast<GLuint>(state.program));
    glBindVertexArray(static_cast<GLuint>(state.vao));
    glActiveTexture(static_cast<GLenum>(state.activeTexture));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state.texture2D));

    auto setGLState = [](GLenum cap, GLboolean enabled) {
        if (enabled) glEnable(cap); else glDisable(cap);
    };
    setGLState(GL_BLEND, state.blend);
    setGLState(GL_DEPTH_TEST, state.depthTest);
    setGLState(GL_STENCIL_TEST, state.stencilTest);
    setGLState(GL_CULL_FACE, state.cullFace);
    setGLState(GL_SCISSOR_TEST, state.scissorTest);

    glBlendFuncSeparate(static_cast<GLenum>(state.blendSrcRGB),
                        static_cast<GLenum>(state.blendDstRGB),
                        static_cast<GLenum>(state.blendSrcAlpha),
                        static_cast<GLenum>(state.blendDstAlpha));
    glDepthFunc(static_cast<GLenum>(state.depthFunc));
}
