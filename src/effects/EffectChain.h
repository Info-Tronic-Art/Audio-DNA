#pragma once
#include <juce_opengl/juce_opengl.h>
#include "effects/Effect.h"
#include "analysis/FeatureSnapshot.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

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

    // Non-copyable (effects_ holds unique_ptr<Effect>). Movable: std::mutex
    // itself has no move constructor, so effectsMutex_ is excluded from the
    // move and the moved-to object gets a fresh, unlocked mutex — safe
    // because moves only ever happen at construction time, single-threaded
    // (e.g. tests/test_mapping_engine.cpp's makeChainWithEffect() helper
    // returning a local EffectChain by value); production code never moves
    // a live, concurrently-accessed EffectChain.
    EffectChain(const EffectChain&) = delete;
    EffectChain& operator=(const EffectChain&) = delete;
    EffectChain(EffectChain&& other) noexcept;
    EffectChain& operator=(EffectChain&& other) noexcept;

    // Add an effect to the chain (takes ownership). Thread-safe — see
    // effectsMutex_ below.
    void addEffect(std::unique_ptr<Effect> effect);

    // Get effect by index. Thread-safe.
    Effect* getEffect(int index);
    // Thread-safe (out-of-line: takes effectsMutex_; see EffectChain.cpp).
    int getNumEffects() const;

    // Render the full chain:
    //   - inputTexture: the loaded image texture
    //   - shaderMgr: to look up compiled shader programs
    //   - texMgr: for FBO ping-pong textures
    //   - quad: the fullscreen quad to draw
    //   - time: current time in seconds
    //   - resolution: viewport width/height
    //   - defaultFBO: the framebuffer to render the final result to
    // Render the effect chain.
    // width/height: the internal render resolution (used for FBOs and uniforms).
    // vpX/vpY/vpW/vpH: the final output viewport on the default framebuffer
    //                   (for letterboxing). If vpW <= 0, uses (0, 0, width, height).
    void render(GLuint inputTexture,
                ShaderManager& shaderMgr,
                TextureManager& texMgr,
                FullscreenQuad& quad,
                float time,
                float width, float height,
                GLuint defaultFBO,
                float vpX = 0.0f, float vpY = 0.0f,
                float vpW = 0.0f, float vpH = 0.0f);

    // Get the previous frame's texture (for temporal effects like Ghost Trails).
    // Returns the texture from the last completed render, or 0 if none.
    GLuint getPreviousFrameTexture() const { return prevFrameTexture_; }

    // Set the latest audio feature snapshot for audio-reactive effects.
    // Pointer must remain valid through the next render() call.
    void setLatestSnapshot(const FeatureSnapshot* snap) { latestSnapshot_ = snap; }

private:
    // Upload an effect's parameters as uniforms
    void uploadEffectUniforms(juce::OpenGLShaderProgram* program,
                              ShaderManager& shaderMgr,
                              const Effect& effect,
                              float time, float width, float height);

    // Run the dry/wet composite pass: blend effected result with pre-effect input
    void applyDryWet(GLuint effectedTexture, GLuint originalTexture,
                     float dryWet,
                     ShaderManager& shaderMgr, FullscreenQuad& quad,
                     GLuint targetFBO, float width, float height);

    const FeatureSnapshot* latestSnapshot_ = nullptr;

    // effects_ is structurally mutated by addEffect() (called from
    // Renderer::initEffectChain() on the GL thread) and read from the GL
    // thread (render(), adaptive-quality/profile scans in Renderer.cpp),
    // the message thread (EffectsRackPanel's 10Hz timer, PresetManager),
    // and HTTP worker threads (ApiServer/TestServer effect-chain queries)
    // with no prior synchronization — a data race on vector push_back vs.
    // concurrent size()/operator[] reads. effectsMutex_ guards every access
    // to effects_ (addEffect/getEffect/getNumEffects, and the enabled-effect
    // collection pass in render()). Effect* pointers obtained under the lock
    // remain valid without holding it afterward: effects_ only ever grows
    // (no removal API), and vector reallocation moves the unique_ptr
    // handles, not the pointed-to Effect objects, so a stable Effect*
    // survives a later addEffect() safely.
    mutable std::mutex effectsMutex_;
    std::vector<std::unique_ptr<Effect>> effects_;

    // Previous frame texture for temporal effects (P13.2)
    // Stores the output of the last completed render for use by temporal effects.
    GLuint prevFrameTexture_ = 0;
    GLuint prevFrameFBO_ = 0;
    int prevFrameWidth_ = 0;
    int prevFrameHeight_ = 0;

    void ensurePrevFrameFBO(int width, int height);
    void savePreviousFrame(GLuint sourceTexture, int width, int height,
                           FullscreenQuad& quad, ShaderManager& shaderMgr);

    // P13.5.11: Cached uniform locations (program ID + uniform name → location)
    // Key: (programID << 32) | hash(uniformName)  — simplified to string key
    std::unordered_map<std::string, GLint> uniformLocationCache_;

    GLint getCachedUniformLocation(juce::OpenGLShaderProgram* program,
                                   const char* uniformName);
};
