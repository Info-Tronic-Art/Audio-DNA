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
                          GLuint defaultFBO)
{
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
        // We need a passthrough shader for this
        auto* passthrough = shaderMgr.getProgram("passthrough");
        if (passthrough == nullptr)
            return;

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        passthrough->use();

        // Bind image texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputTexture);
        auto texLoc = passthrough->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        quad.draw();
        return;
    }

    // Ensure FBOs exist at the right size
    texMgr.createFBOs(static_cast<int>(width), static_cast<int>(height));

    GLuint currentInput = inputTexture;
    int writeFBO = 0; // ping-pong index: alternates 0, 1

    for (size_t i = 0; i < activeEffects.size(); ++i)
    {
        auto* effect = activeEffects[i];
        auto* program = shaderMgr.getProgram(effect->getShaderName());
        if (program == nullptr)
            continue;

        bool isLast = (i == activeEffects.size() - 1);

        if (isLast)
        {
            // Render final effect directly to screen
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        }
        else
        {
            // Render to FBO for next effect to read
            glBindFramebuffer(GL_FRAMEBUFFER, texMgr.getFBO(writeFBO));
        }

        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        glClear(GL_COLOR_BUFFER_BIT);

        program->use();

        // Bind input texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, currentInput);
        auto texLoc = program->getUniformIDFromName("u_texture");
        if (texLoc >= 0)
            glUniform1i(texLoc, 0);

        // Upload effect-specific uniforms
        uploadEffectUniforms(program, shaderMgr, *effect, time, width, height);

        quad.draw();

        if (!isLast)
        {
            // Next effect reads from the FBO we just wrote to
            currentInput = texMgr.getFBOTexture(writeFBO);
            writeFBO = 1 - writeFBO; // ping-pong
        }
    }
}

void EffectChain::uploadEffectUniforms(juce::OpenGLShaderProgram* program,
                                        ShaderManager& /*shaderMgr*/,
                                        const Effect& effect,
                                        float time, float width, float height)
{
    // Global uniforms
    auto timeLoc = program->getUniformIDFromName("u_time");
    if (timeLoc >= 0)
        glUniform1f(timeLoc, time);

    auto resLoc = program->getUniformIDFromName("u_resolution");
    if (resLoc >= 0)
        glUniform2f(resLoc, width, height);

    // Effect-specific parameter uniforms
    for (int i = 0; i < effect.getNumParams(); ++i)
    {
        const auto& param = effect.getParam(i);
        auto loc = program->getUniformIDFromName(param.uniformName.c_str());
        if (loc >= 0)
            glUniform1f(loc, param.value);
    }
}
