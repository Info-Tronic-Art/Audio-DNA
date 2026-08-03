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

// Per-GL-context render state for EffectChain (scout-outputwindow-glcrash.md
// R1/R3; .harmony/specs/outputwindow-arc-design.md W1). The EffectChain is
// SHARED by reference between the main Renderer and the OutputWindow's
// OutputRenderer, whose GL contexts are UNSHARED — GL object names and
// program IDs from one context are meaningless in the other. Everything
// per-context therefore lives here, owned by each renderer alongside the
// ShaderManager/TextureManager/quad it already owns, and passed into
// render() by reference:
//   - uniformLocationCache: program-ID-keyed uniform locations. Program IDs
//     are only unique within one context's share group, so a cache shared
//     across contexts returns locations for the WRONG program (silent
//     wrong-uniform writes), and concurrent insert from two GL threads is
//     UB on unordered_map.
//   - prevFrame*: the temporal-effects previous-frame texture/FBO. Names
//     generated in one context must never be bound or deleted in the other.
// release() must be called from the owning renderer's openGLContextClosing()
// (context still current on that thread): the names and cached locations die
// with the context, and a recreated context must start cold — stale entries
// would poison lookups against the new context's recompiled programs.
struct EffectChainGLState
{
    // Previous frame texture for temporal effects (P13.2)
    GLuint prevFrameTexture = 0;
    GLuint prevFrameFBO = 0;
    int prevFrameWidth = 0;
    int prevFrameHeight = 0;

    // P13.5.11: Cached uniform locations (program ID + uniform name → location)
    std::unordered_map<std::string, GLint> uniformLocationCache;

    // Delete the GL objects and drop all cached locations. Call on the
    // owning context's GL thread while the context is still current.
    void release();
};

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

    // Non-copyable (effects_ holds unique_ptr<Effect>) and non-movable
    // (effectsMutex_ has neither a copy nor a move constructor). Both were
    // already implicitly deleted; explicit deletion here just documents it.
    // Construct in place — do not return/pass an EffectChain by value (see
    // tests/test_mapping_engine.cpp's makeChainWithEffect(), which takes an
    // EffectChain& out-parameter for exactly this reason).
    EffectChain(const EffectChain&) = delete;
    EffectChain& operator=(const EffectChain&) = delete;
    EffectChain(EffectChain&&) = delete;
    EffectChain& operator=(EffectChain&&) = delete;

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
    //   - glState: the CALLING renderer's own per-context state (uniform
    //     location cache + temporal prevFrame FBO — see EffectChainGLState)
    //   - snap: the calling renderer's own coherent FeatureSnapshot copy
    //     (its featureBus_.read()) for the audio-reactive uniforms
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
                EffectChainGLState& glState,
                const FeatureSnapshot& snap,
                float time,
                float width, float height,
                GLuint defaultFBO,
                float vpX = 0.0f, float vpY = 0.0f,
                float vpW = 0.0f, float vpH = 0.0f);

private:
    // Upload an effect's parameters as uniforms
    void uploadEffectUniforms(juce::OpenGLShaderProgram* program,
                              ShaderManager& shaderMgr,
                              EffectChainGLState& glState,
                              const FeatureSnapshot& snap,
                              const Effect& effect,
                              float time, float width, float height);

    // Run the dry/wet composite pass: blend effected result with pre-effect input
    void applyDryWet(GLuint effectedTexture, GLuint originalTexture,
                     float dryWet,
                     ShaderManager& shaderMgr, FullscreenQuad& quad,
                     GLuint targetFBO, float width, float height);

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
    //
    // DEFERRED BOUNDARY (2026-07-30 review advisory; updated 2026-08-02 —
    // second GL thread): effectsMutex_ guards the CONTAINER only — it does
    // NOT guard the CONTENTS each Effect* points to. Effect::enabled_ and
    // EffectParam::value (Effect.h) are read every frame on BOTH GL threads
    // now — the main Renderer's (EffectChain::render(), uniform upload) and
    // OutputWindow's second OpenGLContext, which shares this EffectChain by
    // reference (ctor + attachTo, OutputWindow.cpp:11-27) and calls render()
    // on its own GL thread (OutputWindow.cpp:151) — with zero synchronization
    // against ANY writer: message-thread UI (EffectsRackPanel toggle/knob
    // callbacks), HTTP writes (ApiServer/TestServer set_param/
    // set_effect_chain, now marshalled to the message thread but still
    // unsynchronized against either GL-thread reader), and MappingEngine's
    // audio-reactive writes. This is a real, pre-existing, narrower-severity
    // race (scalar tearing, not container corruption) — now doubled by the
    // second reader, deliberately NOT folded into this fix. Extending
    // effectsMutex_ to cover per-field access would mean holding it through
    // render()'s whole uniform-upload loop on BOTH GL threads (a real GL
    // hot-path cost, unlike the narrow scan this mutex currently guards) and
    // would need every UI/API writer updated too — a bigger design pass
    // (atomics per field, or a snapshot/double-buffer scheme matching the
    // Composition-mutation LAW pattern), not a same-commit tack-on.
    // KNOWN-DEFERRED residual, recorded in
    // .harmony/specs/outputwindow-arc-design.md (A2): EffectParam::value
    // stays a plain float; this msg→GL scalar-crossing set is the documented
    // deferred list for the TSan gate. (The per-context GL state — uniform
    // location cache + prevFrame FBO — moved to EffectChainGLState above,
    // same design doc W1; the shared parked snapshot was replaced by the
    // per-caller snapshot parameter, W2.)
    mutable std::mutex effectsMutex_;
    std::vector<std::unique_ptr<Effect>> effects_;

    void ensurePrevFrameFBO(EffectChainGLState& glState, int width, int height);
    void savePreviousFrame(EffectChainGLState& glState,
                           GLuint sourceTexture, int width, int height,
                           FullscreenQuad& quad, ShaderManager& shaderMgr);

    GLint getCachedUniformLocation(EffectChainGLState& glState,
                                   juce::OpenGLShaderProgram* program,
                                   const char* uniformName);
};
