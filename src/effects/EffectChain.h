#pragma once
#include <juce_opengl/juce_opengl.h>
#include "effects/Effect.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include <vector>
#include <memory>

// EffectChain: manages an ordered list of Effects and renders them
// using ping-pong FBOs.
//
// Rendering flow:
//   1. Bind FBO A, draw image texture through Effect 0's shader
//   2. Bind FBO B, draw FBO A's texture through Effect 1's shader
//   3. Swap and repeat for remaining effects
//   4. Final blit to screen (default framebuffer)
//
// All render methods must be called on the GL thread.
class EffectChain
{
public:
    EffectChain() = default;

    // Add an effect to the chain (takes ownership)
    void addEffect(std::unique_ptr<Effect> effect);

    // Get effect by index
    Effect* getEffect(int index);
    int getNumEffects() const { return static_cast<int>(effects_.size()); }

    // Render the full chain:
    //   - inputTexture: the loaded image texture
    //   - shaderMgr: to look up compiled shader programs
    //   - texMgr: for FBO ping-pong textures
    //   - quad: the fullscreen quad to draw
    //   - time: current time in seconds
    //   - resolution: viewport width/height
    //   - defaultFBO: the framebuffer to render the final result to
    void render(GLuint inputTexture,
                ShaderManager& shaderMgr,
                TextureManager& texMgr,
                FullscreenQuad& quad,
                float time,
                float width, float height,
                GLuint defaultFBO);

private:
    // Upload an effect's parameters as uniforms
    void uploadEffectUniforms(juce::OpenGLShaderProgram* program,
                              ShaderManager& shaderMgr,
                              const Effect& effect,
                              float time, float width, float height);

    std::vector<std::unique_ptr<Effect>> effects_;
};
