#include "EffectChain.h"

using namespace juce::gl;

void EffectChain::addEffect(std::unique_ptr<Effect> effect)
{
    effect->setOrder(static_cast<int>(effects_.size()));
    effects_.push_back(std::move(effect));
}

Effect* EffectChain::getEffect(int index)
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
        return effects_[static_cast<size_t>(index)].get();
    return nullptr;
}

void EffectChain::render(GLuint inputTexture,
                          ShaderManager& shaderMgr,
                          TextureManager& texMgr,
                          FullscreenQuad& quad,
                          float time,
                          float width, float height,
                          GLuint defaultFBO,
                          float vpX, float vpY,
                          float vpW, float vpH)
{
    // If no explicit viewport, use full render area
    if (vpW <= 0.0f || vpH <= 0.0f)
    {
        vpX = 0.0f;
        vpY = 0.0f;
        vpW = width;
        vpH = height;
    }
    // Collect enabled effects
    std::vector<Effect*> activeEffects;
    for (auto& e : effects_)
    {
        if (e->isEnabled())
            activeEffects.push_back(e.get());
    }

    if (activeEffects.empty())
    {
        // No effects — just draw the image directly to screen
        auto* passthrough = shaderMgr.getProgram("passthrough");
        if (passthrough == nullptr)
            return;

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                   static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));

        passthrough->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        auto texLoc = passthrough->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        quad.draw();

        // Save current frame as previous frame for temporal effects
        // (even with no effects, temporal effects next frame need the input)
        return;
    }

    // Ensure FBOs exist at the right size
    texMgr.createFBOs(static_cast<int>(width), static_cast<int>(height));

    // Check if any active effect needs dry/wet compositing (dryWet < 1.0)
    // We need a third FBO for the pre-effect snapshot when dry/wet < 1.0
    // We reuse the existing 2 FBOs for ping-pong, plus the default FBO for final output

    GLuint currentInput = inputTexture;
    int writeFBO = 0; // ping-pong index: alternates 0, 1

    for (size_t i = 0; i < activeEffects.size(); ++i)
    {
        auto* effect = activeEffects[i];
        auto* program = shaderMgr.getProgram(effect->getShaderName());
        if (program == nullptr)
            continue;

        bool isLast = (i == activeEffects.size() - 1);
        bool needsDryWet = (effect->getDryWet() < 0.99f);

        // For dry/wet: remember pre-effect input texture
        GLuint preEffectTexture = currentInput;

        if (isLast && !needsDryWet)
        {
            // Render final effect directly to screen with letterbox viewport
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
            glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                       static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));
        }
        else
        {
            // Render to FBO for next effect to read (full FBO size)
            glBindFramebuffer(GL_FRAMEBUFFER, texMgr.getFBO(writeFBO));
            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        }
        glClear(GL_COLOR_BUFFER_BIT);

        program->use();

        // Bind input texture (cached location)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentInput);
        auto texLoc = getCachedUniformLocation(program, "u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        // For temporal effects, bind the previous frame texture
        if (effect->isTemporal() && prevFrameTexture_ != 0)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, prevFrameTexture_);
            auto prevLoc = getCachedUniformLocation(program, "u_prev_frame");
            if (prevLoc >= 0)
                glUniform1i(prevLoc, 1);
            glActiveTexture(GL_TEXTURE0);
        }

        // Upload effect-specific uniforms
        uploadEffectUniforms(program, shaderMgr, *effect, time, width, height);

        quad.draw();

        if (isLast && !needsDryWet)
        {
            // Done — rendered to screen
        }
        else if (isLast && needsDryWet)
        {
            // Effect rendered to FBO; now dry/wet composite to screen
            GLuint effectedTex = texMgr.getFBOTexture(writeFBO);

            glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
            glViewport(static_cast<GLint>(vpX), static_cast<GLint>(vpY),
                       static_cast<GLsizei>(vpW), static_cast<GLsizei>(vpH));
            glClear(GL_COLOR_BUFFER_BIT);

            applyDryWet(effectedTex, preEffectTexture, effect->getDryWet(),
                        shaderMgr, quad, defaultFBO, width, height);
        }
        else if (needsDryWet)
        {
            // Mid-chain dry/wet: effect wrote to writeFBO, now composite in place
            GLuint effectedTex = texMgr.getFBOTexture(writeFBO);
            int compositeFBO = 1 - writeFBO; // use the other FBO as composite target

            glBindFramebuffer(GL_FRAMEBUFFER, texMgr.getFBO(compositeFBO));
            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
            glClear(GL_COLOR_BUFFER_BIT);

            applyDryWet(effectedTex, preEffectTexture, effect->getDryWet(),
                        shaderMgr, quad, texMgr.getFBO(compositeFBO), width, height);

            currentInput = texMgr.getFBOTexture(compositeFBO);
            writeFBO = writeFBO; // next effect will write to the FBO we didn't just use for composite
            // Actually we need to swap: next write goes to the FBO that held the effected result
            writeFBO = 1 - compositeFBO;
        }
        else
        {
            // Normal case: no dry/wet needed
            currentInput = texMgr.getFBOTexture(writeFBO);
            writeFBO = 1 - writeFBO; // ping-pong
        }
    }

    // After rendering: check if any effect is temporal and save the frame
    // for next render's u_prev_frame binding
    bool anyTemporal = false;
    for (auto& e : effects_)
    {
        if (e->isEnabled() && e->isTemporal())
        {
            anyTemporal = true;
            break;
        }
    }
    if (anyTemporal)
    {
        // Save the final output. For the last effect, it went to screen (defaultFBO).
        // We need to capture what's in the last FBO that was written to.
        // The easiest approach: copy from the last FBO texture we have.
        // If the last effect wrote directly to defaultFBO, we use the input that was rendered.
        // For simplicity, save the currentInput (which is the last intermediate result).
        if (currentInput != inputTexture && currentInput != 0)
        {
            savePreviousFrame(currentInput, static_cast<int>(width), static_cast<int>(height),
                              quad, shaderMgr);
        }
    }
}

void EffectChain::applyDryWet(GLuint effectedTexture, GLuint originalTexture,
                                float dryWet,
                                ShaderManager& shaderMgr, FullscreenQuad& quad,
                                GLuint targetFBO, float width, float height)
{
    auto* program = shaderMgr.getProgram("effect_drywet");
    if (!program)
    {
        // Fallback: just draw the effected texture
        program = shaderMgr.getProgram("passthrough");
        if (!program) return;
        program->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, effectedTexture);
        auto texLoc = program->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);
        quad.draw();
        return;
    }

    program->use();

    // Bind effected texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, effectedTexture);
    auto texLoc = program->getUniformIDFromName("u_texture");
    if (texLoc >= 0)
        glUniform1i(texLoc, 0);

    // Bind original texture to unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, originalTexture);
    auto origLoc = program->getUniformIDFromName("u_original");
    if (origLoc >= 0)
        glUniform1i(origLoc, 1);

    // Set dry/wet uniform
    auto dwLoc = program->getUniformIDFromName("u_drywet");
    if (dwLoc >= 0)
        glUniform1f(dwLoc, dryWet);

    glActiveTexture(GL_TEXTURE0);
    quad.draw();
}

GLint EffectChain::getCachedUniformLocation(juce::OpenGLShaderProgram* program,
                                              const char* uniformName)
{
    // Build cache key from program ID and uniform name
    auto progID = static_cast<unsigned int>(program->getProgramID());
    std::string key = std::to_string(progID) + ":" + uniformName;

    auto it = uniformLocationCache_.find(key);
    if (it != uniformLocationCache_.end())
        return it->second;

    GLint loc = program->getUniformIDFromName(uniformName);
    uniformLocationCache_[key] = loc;
    return loc;
}

void EffectChain::uploadEffectUniforms(juce::OpenGLShaderProgram* program,
                                        ShaderManager& /*shaderMgr*/,
                                        const Effect& effect,
                                        float time, float width, float height)
{
    // Global uniforms (cached)
    auto timeLoc = getCachedUniformLocation(program, "u_time");
    if (timeLoc >= 0)
        glUniform1f(timeLoc, time);

    auto resLoc = getCachedUniformLocation(program, "u_resolution");
    if (resLoc >= 0)
        glUniform2f(resLoc, width, height);

    // Effect-specific parameter uniforms (cached)
    for (int i = 0; i < effect.getNumParams(); ++i)
    {
        const auto& param = effect.getParam(i);
        auto loc = getCachedUniformLocation(program, param.uniformName.c_str());
        if (loc >= 0)
            glUniform1f(loc, param.value);
    }
}

void EffectChain::ensurePrevFrameFBO(int width, int height)
{
    if (prevFrameTexture_ != 0 && prevFrameWidth_ == width && prevFrameHeight_ == height)
        return;

    // Release old
    if (prevFrameFBO_ != 0)
    {
        glDeleteFramebuffers(1, &prevFrameFBO_);
        prevFrameFBO_ = 0;
    }
    if (prevFrameTexture_ != 0)
    {
        glDeleteTextures(1, &prevFrameTexture_);
        prevFrameTexture_ = 0;
    }

    prevFrameWidth_ = width;
    prevFrameHeight_ = height;

    glGenTextures(1, &prevFrameTexture_);
    glBindTexture(GL_TEXTURE_2D, prevFrameTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &prevFrameFBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFrameFBO_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, prevFrameTexture_, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void EffectChain::savePreviousFrame(GLuint sourceTexture, int width, int height,
                                     FullscreenQuad& quad, ShaderManager& shaderMgr)
{
    ensurePrevFrameFBO(width, height);
    if (prevFrameFBO_ == 0) return;

    // Copy the source texture to our previous frame FBO
    auto* passthrough = shaderMgr.getProgram("passthrough");
    if (!passthrough) return;

    glBindFramebuffer(GL_FRAMEBUFFER, prevFrameFBO_);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    passthrough->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    auto texLoc = passthrough->getUniformIDFromName("u_texture");
    if (texLoc >= 0)
        glUniform1i(texLoc, 0);

    quad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
