#include "sources/ProceduralSource.h"
#include <iostream>

using namespace juce::gl;

ProceduralSource::ProceduralSource(const std::string& id, const std::string& displayName,
                                   const std::string& category, const std::string& shaderName,
                                   bool stateful)
    : id_(id), displayName_(displayName), category_(category), shaderName_(shaderName),
      stateful_(stateful)
{
}

ProceduralSource::~ProceduralSource()
{
    // GL resources should be released before destructor via releaseGL()
}

void ProceduralSource::initGL(int width, int height)
{
    fboWidth_ = width;
    fboHeight_ = height;

    if (stateful_)
    {
        createFBO(pingFBO_[0], pingTex_[0], width, height);
        createFBO(pingFBO_[1], pingTex_[1], width, height);
        currentPing_ = 0;

        // Clear both ping-pong buffers to black with alpha=0
        // Alpha=0 signals "uninitialized" to stateful shaders that use
        // state.a < 0.1 as a seed trigger (e.g., Cellular Automata)
        for (int i = 0; i < 2; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingFBO_[i]);
            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    else
    {
        createFBO(outputFBO_, outputTex_, width, height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glInitialized_ = true;
}

void ProceduralSource::releaseGL()
{
    if (stateful_)
    {
        deleteFBO(pingFBO_[0], pingTex_[0]);
        deleteFBO(pingFBO_[1], pingTex_[1]);
    }
    else
    {
        deleteFBO(outputFBO_, outputTex_);
    }
    glInitialized_ = false;
}

void ProceduralSource::resize(int width, int height)
{
    if (width == fboWidth_ && height == fboHeight_)
        return;

    releaseGL();
    initGL(width, height);
}

void ProceduralSource::reset()
{
    if (!glInitialized_ || !stateful_) return;

    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO_[i]);
        glViewport(0, 0, fboWidth_, fboHeight_);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    currentPing_ = 0;
}

GLuint ProceduralSource::render(ShaderManager& shaderMgr, FullscreenQuad& quad,
                                 float time, int width, int height,
                                 const FeatureSnapshot& snapshot)
{
    if (!glInitialized_)
        initGL(width, height);

    resize(width, height);

    auto* program = shaderMgr.getProgram(shaderName_.c_str());
    if (!program)
        return 0;

    if (stateful_)
    {
        // Ping-pong: read from current, write to next
        int readIdx = currentPing_;
        int writeIdx = 1 - currentPing_;

        glBindFramebuffer(GL_FRAMEBUFFER, pingFBO_[writeIdx]);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        program->use();

        // Bind previous state as input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingTex_[readIdx]);
        auto texLoc = program->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        uploadUniforms(program, time, width, height, snapshot);
        quad.draw();

        currentPing_ = writeIdx;
        return pingTex_[writeIdx];
    }
    else
    {
        // Single-pass: render to output FBO
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO_);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        program->use();
        uploadUniforms(program, time, width, height, snapshot);
        quad.draw();

        return outputTex_;
    }
}

void ProceduralSource::uploadUniforms(juce::OpenGLShaderProgram* program,
                                       float time, int width, int height,
                                       const FeatureSnapshot& snapshot)
{
    auto loc = [&](const char* name) {
        return program->getUniformIDFromName(name);
    };

    // Global uniforms
    auto timeLoc = loc("u_time");
    if (timeLoc >= 0) glUniform1f(timeLoc, time);

    auto resLoc = loc("u_resolution");
    if (resLoc >= 0) glUniform2f(resLoc, static_cast<float>(width), static_cast<float>(height));

    // Audio feature uniforms — sources can react to audio
    auto rmsLoc = loc("u_rms");
    if (rmsLoc >= 0) glUniform1f(rmsLoc, snapshot.rms);

    auto bassLoc = loc("u_bass");
    if (bassLoc >= 0) glUniform1f(bassLoc, snapshot.bandEnergies[1]);

    auto midLoc = loc("u_mid");
    if (midLoc >= 0) glUniform1f(midLoc, snapshot.bandEnergies[3]);

    auto highLoc = loc("u_high");
    if (highLoc >= 0) glUniform1f(highLoc, snapshot.bandEnergies[5]);

    auto beatLoc = loc("u_beatPhase");
    if (beatLoc >= 0) glUniform1f(beatLoc, snapshot.beatPhase);

    auto centroidLoc = loc("u_spectralCentroid");
    if (centroidLoc >= 0) glUniform1f(centroidLoc, snapshot.spectralCentroid);

    auto onsetLoc = loc("u_onsetStrength");
    if (onsetLoc >= 0) glUniform1f(onsetLoc, snapshot.onsetStrength);

    // P18: Extended audio uniforms for audio-native sources
    auto l = loc("u_onsetDetected");
    if (l >= 0) glUniform1f(l, snapshot.onsetDetected ? 1.0f : 0.0f);
    l = loc("u_barPhase");
    if (l >= 0) glUniform1f(l, snapshot.barPhase);
    l = loc("u_phrasePhase");
    if (l >= 0) glUniform1f(l, snapshot.phrasePhase);
    l = loc("u_spectralFlux");
    if (l >= 0) glUniform1f(l, snapshot.spectralFlux);
    l = loc("u_dominantPitch");
    if (l >= 0) glUniform1f(l, snapshot.dominantPitch);
    l = loc("u_pitchConfidence");
    if (l >= 0) glUniform1f(l, snapshot.pitchConfidence);
    l = loc("u_detectedKey");
    if (l >= 0) glUniform1f(l, static_cast<float>(snapshot.detectedKey));
    l = loc("u_keyIsMajor");
    if (l >= 0) glUniform1f(l, snapshot.keyIsMajor ? 1.0f : 0.0f);
    l = loc("u_structuralState");
    if (l >= 0) glUniform1f(l, static_cast<float>(snapshot.structuralState));
    l = loc("u_bpm");
    if (l >= 0) glUniform1f(l, snapshot.bpm);
    l = loc("u_hcdf");
    if (l >= 0) glUniform1f(l, snapshot.harmonicChangeDetection);
    l = loc("u_bandEnergies");
    if (l >= 0) glUniform1fv(l, 7, snapshot.bandEnergies);
    l = loc("u_chromagram");
    if (l >= 0) glUniform1fv(l, 12, snapshot.chromagram);
    l = loc("u_mfccs");
    if (l >= 0) glUniform1fv(l, 13, snapshot.mfccs);

    // P23: Genre detection uniforms
    l = loc("u_genre");
    if (l >= 0) glUniform1f(l, static_cast<float>(snapshot.detectedGenre));
    l = loc("u_genreConfidence");
    if (l >= 0) glUniform1f(l, snapshot.genreConfidence);
    l = loc("u_energyState");
    if (l >= 0) glUniform1f(l, static_cast<float>(snapshot.energyState));

    // Feedback texture (unit 1) — previous frame's composited output
    auto feedbackLoc = loc("u_feedbackTex");
    if (feedbackLoc >= 0 && feedbackTex_ != 0)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, feedbackTex_);
        glUniform1i(feedbackLoc, 1);
        glActiveTexture(GL_TEXTURE0);
    }

    // Source-specific parameter uniforms
    for (int i = 0; i < getNumParams(); ++i)
    {
        const auto& param = params_[static_cast<size_t>(i)];
        auto paramLoc = program->getUniformIDFromName(param.uniformName.c_str());
        if (paramLoc >= 0)
            glUniform1f(paramLoc, param.value);
    }
}

void ProceduralSource::addParam(const std::string& name, const std::string& uniformName,
                                 float defaultValue, float min, float max)
{
    Param p;
    p.name = name;
    p.uniformName = uniformName;
    p.value = defaultValue;
    p.defaultValue = defaultValue;
    p.min = min;
    p.max = max;
    params_.push_back(p);
}

void ProceduralSource::setParamValue(int index, float value)
{
    if (index >= 0 && index < static_cast<int>(params_.size()))
        params_[static_cast<size_t>(index)].value = value;
}

void ProceduralSource::resetParams()
{
    for (auto& p : params_)
        p.value = p.defaultValue;
}

void ProceduralSource::createFBO(GLuint& fbo, GLuint& tex, int w, int h)
{
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ProceduralSource::deleteFBO(GLuint& fbo, GLuint& tex)
{
    if (fbo != 0) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
    if (tex != 0) { glDeleteTextures(1, &tex); tex = 0; }
}
