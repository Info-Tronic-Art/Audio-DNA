#pragma once
#include <juce_opengl/juce_opengl.h>
#include "model/Deck.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include "render/FeedbackProcessor.h"
#include "render/LayerStateKey.h"
#include "render/CrossfadeHistory.h"
#include "effects/EffectLibrary.h"
#include "effects/Effect.h"
#include "analysis/FeatureSnapshot.h"
#include <unordered_map>
#include <atomic>
#include <string>
#include <functional>
#include <memory>

// CompositorEngine: multi-layer compositing for v2 deck mode.
//
// Each layer composited bottom-to-top with per-layer blend/keying.
//
// Rendering pipeline (v2):
//   For each layer (bottom to top):
//     Active clip media → Clip Effects → Clip Keying (if Transparent) →
//     Layer Effects → Layer Blend Mode → Layer Opacity → Accumulator FBO
//   Mask layers: content becomes alpha mask applied to accumulator.
//   FX Only layers: effects applied to current accumulator state.
//   Global Effects → Master Opacity → Screen / Fullscreen Output
class CompositorEngine
{
public:
    CompositorEngine() = default;

    // Initialize GL resources. Call from newOpenGLContextCreated().
    void initGL(int width, int height);

    // Release GL resources. Call from openGLContextClosing().
    void releaseGL();

    // Resize FBOs if viewport changed.
    void resize(int width, int height);

    // Set the effect library for creating per-clip effect instances.
    void setEffectLibrary(EffectLibrary* lib) { effectLibrary_ = lib; }

    // Load an image for a specific key/clip. Call from message thread (queued).
    // Returns the GL texture ID, or 0 on failure.
    GLuint loadKeyImage(const juce::File& imageFile);

    // Get or create a texture for a key/clip's image file (cached).
    GLuint getKeyTexture(const juce::File& imageFile);

    // Callback to render a procedural source by its type ID.
    // Returns the GL texture ID of the rendered source, or 0 on failure.
    // clipSourceParams are per-clip parameter overrides.
    using SourceRenderFn = std::function<GLuint(const std::string& sourceId, float time, int w, int h,
                                                 const std::vector<Clip::SourceParam>* clipSourceParams)>;

    // Set the source rendering callback (provided by Renderer)
    void setSourceRenderer(SourceRenderFn fn) { sourceRenderFn_ = std::move(fn); }

    // Callback to get the current video frame texture for a clip.
    // The Renderer advances video playback and uploads frames; this just returns the texture.
    // Parameters: clip pointer, dt (frame delta time)
    // Returns GL texture ID, or 0 if no frame ready.
    using VideoFrameFn = std::function<GLuint(const Clip* clip, float dt)>;

    // Set the video frame callback (provided by Renderer)
    void setVideoFrameProvider(VideoFrameFn fn) { videoFrameFn_ = std::move(fn); }

    // === Deck/Layer-based compositing ===
    // Composite all layers in the deck and return the result texture.
    // Returns 0 if no layers have active clips.
    // dt: REAL measured seconds since the last frame (see Renderer::
    // renderOpenGL's lastFrameTimestampMs_) -- fed straight into video/
    // image-sequence playhead advancement (VideoPlayer::advanceFrame,
    // ImageSequence::advanceFrame both do `currentTime_ += dt * speed`,
    // literally, with no other timing source) AND crossfade progress
    // advancement (step = dt / transitionDuration -- S167-L4b DT-FIX,
    // second instance). A hardcoded 1/60 at both sites used to silently
    // couple playback speed / transition duration to the actual GL
    // callback rate (half speed/duration at 30fps, double at 120fps).
    GLuint compositeDeck(Deck& deck,
                         ShaderManager& shaderMgr,
                         FullscreenQuad& quad,
                         float time, float dt,
                         int width, int height);

    // P21: Composite only persistent layers from a non-active deck onto the
    // existing accumulator. Call AFTER compositeDeck() for the active deck.
    // s-rta-0926b R4: each persistent Opaque/Transparent layer gets the same
    // per-layer stages as on the active deck (renderLayerStages); what still
    // differs is listed at the definition.
    // dt: see compositeDeck()'s comment above -- same real measured delta,
    // same reason.
    void compositePersistentLayers(Deck& deck,
                                   ShaderManager& shaderMgr,
                                   FullscreenQuad& quad,
                                   float time, float dt,
                                   int width, int height);

    bool hasActiveLayers() const { return hasActiveLayers_; }

    // Copy the current composited frame into the persistent feedback buffer.
    // Call AFTER compositeDeck() each frame.
    void updateFeedbackBuffer(ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);

    // S166: Apply the composition-wide Global Effects stack (see
    // Composition::globalEffects / EffectScope::global()) to the FINAL
    // composited frame. Call once per frame, AFTER compositeDeck() and any
    // compositePersistentLayers() calls have finished writing into the
    // accumulator — this is the "Global Effects" stage of the pipeline
    // documented in the class comment above (Global Effects -> Master
    // Opacity -> Screen / Fullscreen Output). Calls applyClipEffects directly
    // with the globalEffects vector — an empty globalEffects vector is a
    // true no-op (applyClipEffects returns inputTex before issuing any GL
    // call, same short-circuit as an empty per-clip/per-layer chain).
    GLuint applyGlobalEffects(const std::vector<Clip::EffectSlot>& globalEffects,
                              GLuint inputTex,
                              ShaderManager& shaderMgr, FullscreenQuad& quad,
                              float time, int w, int h);

    // Get the feedback texture (previous frame's output)
    GLuint getFeedbackTexture() const { return feedbackTex_; }
    bool isFeedbackReady() const { return feedbackReady_; }

    // s-rta-0926b R1: how many temporal buffers (u_prev_frame) and Screen Split /
    // Frame Stutter frame rings exist right now. Written on the GL thread where
    // they are created/released, read from any thread (/api/state) -- relaxed
    // atomics, same pattern as Renderer::frameTimeMs_.
    int getTemporalBufferCount() const { return temporalBufferCount_.load(std::memory_order_relaxed); }
    int getFrameRingCount() const { return frameRingCount_.load(std::memory_order_relaxed); }

private:
    // Accumulator FBO — the composited result
    GLuint accumulatorFBO_ = 0;
    GLuint accumulatorTex_ = 0;

    // Scratch FBO for per-key/per-layer rendering (keying pass)
    GLuint scratchFBO_ = 0;
    GLuint scratchTex_ = 0;

    // Effect scratch pool (per-clip/per-layer/global effect chains, clip
    // transform/opacity, layer transform, Screen Split). Every write into it
    // goes through pickEffectTarget() -- see ScratchPool.h for the rule. The
    // third member (C) exists so a pass that samples one pool texture while
    // its caller holds another (the transition's outgoing-clip chain) still
    // has a free target.
    GLuint effectFBO_A_ = 0;
    GLuint effectTex_A_ = 0;
    GLuint effectFBO_B_ = 0;
    GLuint effectTex_B_ = 0;
    GLuint effectFBO_C_ = 0;
    GLuint effectTex_C_ = 0;

    struct ScratchTarget { GLuint fbo; GLuint tex; };
    // A pool target that is neither readTex (what the pass samples) nor
    // holdTex (a texture the caller still needs afterwards; 0 = none).
    ScratchTarget pickEffectTarget(GLuint readTex, GLuint holdTex) const;
    bool isEffectPoolTexture(GLuint tex) const
    {
        return tex != 0 && (tex == effectTex_A_ || tex == effectTex_B_ || tex == effectTex_C_);
    }

    // Transition FBO (P14) — separate from scratch to avoid keying conflicts
    GLuint transitionFBO_ = 0;
    GLuint transitionTex_ = 0;

    // Persistent feedback buffer — survives across frames for feedback effects
    GLuint feedbackFBO_ = 0;
    GLuint feedbackTex_ = 0;
    bool feedbackReady_ = false;

    int fboWidth_ = 0;
    int fboHeight_ = 0;
    bool glInitialized_ = false;
    bool hasActiveLayers_ = false;

    // Texture cache: file path → GL texture ID
    std::unordered_map<std::string, GLuint> textureCache_;

    SourceRenderFn sourceRenderFn_;
    VideoFrameFn videoFrameFn_;
    EffectLibrary* effectLibrary_ = nullptr;

    // Audio feature snapshot for audio-reactive effects. Owned VALUE (R7,
    // featurebus-thread-safety-design.md): a copy parked here can never
    // dangle, unlike the previous caller-stack pointer.
    FeatureSnapshot latestSnapshot_{};

public:
    // Set the latest audio feature snapshot for audio-reactive effects.
    // Call before compositeDeck() each frame; the snapshot is copied.
    void setLatestSnapshot(const FeatureSnapshot& snap) { latestSnapshot_ = snap; }

private:

    // Per-frame GL state below (feedback processors, temporal buffers, frame
    // rings) is keyed by a LayerStateKey (render/LayerStateKey.h): deck id +
    // layer id (+ which chain), or LayerStateKey::kGlobalEffects for the
    // composition's Global Effects chain (S166: never a real layer's key).
    // s-rta-0926b R2: keying by layer id alone shared that state between
    // layers of different decks (a persistent layer vs the active deck).

    // Per-layer feedback processors (key: LayerStateKey::clipChain)
    std::unordered_map<uint64_t, std::unique_ptr<FeedbackProcessor>> feedbackProcessors_;

    // Get or create a feedback processor for a layer
    FeedbackProcessor& getOrCreateFeedbackProcessor(uint64_t stateKey);

    // Per-layer temporal FBOs for time effects (u_prev_frame)
    // Each layer that uses temporal effects gets its own persistent prev-frame buffer.
    struct TemporalBuffer {
        GLuint fbo = 0;
        GLuint tex = 0;
        int width = 0;
        int height = 0;
    };
    std::unordered_map<uint64_t, TemporalBuffer> layerTemporalBuffers_;

    // Get or create a temporal buffer for a chain, resized if needed. Creates
    // GL objects and leaves framebuffer 0 bound: call it BEFORE a pass binds
    // its target (s-rta-0926b R5).
    TemporalBuffer& getOrCreateTemporalBuffer(uint64_t stateKey, int w, int h);

    // Save a texture into a temporal buffer (passthrough copy)
    void saveToTemporalBuffer(TemporalBuffer& buf, GLuint srcTex,
                              ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);

    // Frame ring buffer for Screen Split / Frame Stutter.
    // Stores up to 480 previous frames at 1/4 x 1/4 of the render size:
    // 248.8 MB (237 MiB) at 1080p, 995 MB at 4K; two per (deck, layer) clip
    // chain after a fade with Split/Stutter on both sides (s-rta-0926b R1: the
    // outgoing slot).
    static constexpr int kMaxRingFrames = 480;
    static constexpr int kRingDownscale = 4; // store at 1/4 resolution
    struct FrameRingBuffer {
        std::vector<GLuint> fbos;
        std::vector<GLuint> textures;
        int writeIndex = 0;
        int ringWidth = 0;  // stored resolution (downscaled)
        int ringHeight = 0;
        int frameCount = 0;
        bool initialized = false;
    };
    std::unordered_map<uint64_t, FrameRingBuffer> layerRingBuffers_;

    FrameRingBuffer& getOrCreateRingBuffer(uint64_t stateKey, int w, int h);

    // s-rta-0926b R1: live counts of the two maps above (see the getters).
    std::atomic<int> temporalBufferCount_{ 0 };
    std::atomic<int> frameRingCount_{ 0 };

    // s-rta-0926b R1: one crossfade-start detector per clip chain (key:
    // LayerStateKey::clipChain). Compositor state, not a Layer field.
    std::unordered_map<uint64_t, CrossfadeStartDetector> crossfadeStart_;

    // s-rta-0926b R1: at the first frame of a crossfade, hand the layer's
    // clip-chain history (clipKey) to its outgoing slot (outKey): COPY the
    // temporal buffer (the incoming clip starts from the layer's last picture,
    // exactly as after a cut) and SWAP the frame ring (the incoming clip's ring
    // starts empty; the outgoing clip keeps its frames). May create GL objects
    // (the spare buffer) and ends with framebuffer 0 bound: call it OUTSIDE any
    // pass, before the incoming clip's chain runs.
    void handOverClipHistory(uint64_t clipKey, uint64_t outKey,
                             ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);
    void pushFrameToRing(FrameRingBuffer& ring, GLuint srcTex,
                         ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);
    GLuint getFrameFromRing(const FrameRingBuffer& ring, int framesAgo) const;

    // Screen Split: render a grid of delayed copies of the clip texture
    // Returns the composited grid texture, or 0 if not a screen split effect.
    // Renders into dstFBO/dstTex (a pool target picked by the caller).
    GLuint applyScreenSplit(GLuint clipTex, const Clip::EffectSlot& slot,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            uint64_t stateKey, int w, int h,
                            GLuint dstFBO, GLuint dstTex);

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);

    // Upload audio feature uniforms to the current shader program.
    // Used by audio-reactive effects (P18) that need chromagram, MFCCs, etc.
    void uploadAudioUniforms(juce::OpenGLShaderProgram* program) const;

    // Apply per-clip transform (position/scale/rotation) to a texture, then
    // the clip's own opacity. holdTex: a texture the CALLER still needs after
    // this call (applyTransition's incoming-clip result); no pass writes it.
    GLuint applyClipTransform(const Clip& clip, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h, GLuint holdTex = 0);

    // S167-L4b, fix-needed(s167): bake a clip's per-clip opacity into a
    // texture via a single GL pass using the "clip_opacity_blend" program
    // (col.rgb *= u_opacity, alpha untouched -- see the .cpp comment at the
    // call site for why RGB and not alpha: several blend modes in
    // blendLayerOntoAccumulator never read alpha at all, and an Opaque-type
    // layer left at its 1.0 opacity default disables blending outright, so
    // an alpha-only reduction is invisible in both cases). True no-op
    // (returns srcTex unchanged, no GL call) at the default 1.0 opacity,
    // matching this file's other needsTransform-style early-outs. dstFBO/
    // dstTex let each caller pick a scratch buffer that won't alias srcTex.
    // s-rta-0925 ms-white FIX: the no-op early-return is only safe when
    // srcTex is NOT about to be reused as a write target by the caller's
    // next stage. applyClipTransform's needsTransform branch renders into
    // effectFBO_A_/effectTex_A_ as scratch, and applyClipEffects' ping-pong
    // always starts its first write at effectFBO_A_ -- so at the default
    // 1.0 opacity the early return used to hand back effectTex_A_ verbatim,
    // which applyClipEffects then bound as BOTH the sampled input and the
    // render target of the same FBO (a read/write-same-texture feedback
    // loop, UB per the GL spec -- resolved to a fully transparent frame on
    // the Metal-backed macOS GL driver). forceCopy lets a caller that knows
    // its srcTex may alias an upcoming write target suppress the no-op and
    // always render into dstFBO/dstTex instead.
    GLuint applyClipOpacity(float opacity, GLuint srcTex, GLuint dstFBO, GLuint dstTex,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            int w, int h, bool forceCopy = false);

    // S167-L4b: pure opacity product for the FX-Only constant-alpha blend.
    // Owner's ruling (2026-09-05): master/layer/clip opacity multiply, so a
    // clip pinned at 0.5 can never exceed 50% regardless of the layer's own
    // opacity. No clamp needed -- both inputs are already normalized [0,1]
    // sliders, so their product is itself in [0,1].
    static float combinedOpacity(float layerOpacity, float clipOpacity)
    {
        return layerOpacity * clipOpacity;
    }

    // Apply an effect chain to a texture, returns result texture ID.
    // Renders into the effect scratch pool (pickEffectTarget per pass).
    // stateKey (LayerStateKey): keys the chain's temporal buffer (u_prev_frame)
    // and Screen Split / Frame Stutter ring.
    // holdTex: a texture the CALLER still needs after this call returns (e.g.
    // applyTransition's incoming-clip result); no pass of this chain writes it.
    GLuint applyClipEffects(const std::vector<Clip::EffectSlot>& effects, GLuint inputTex,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            float time, int w, int h,
                            uint64_t stateKey, GLuint holdTex = 0);

    // Apply layer transform (translate/scale/rotate) to a texture
    GLuint applyLayerTransform(const Layer& layer, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h);

    // Apply a keying mode (normally layer.keyingMode) with the layer's key
    // parameters and opacity (u_opacity). s-rta-0926b R4-opaque: the mode is
    // explicit so a persistent Opaque layer can run the Alpha key purely to
    // apply its layer opacity.
    void applyLayerKeying(const Layer& layer, Layer::KeyingMode mode, GLuint srcTex, GLuint dstFBO,
                          ShaderManager& shaderMgr, FullscreenQuad& quad,
                          int w, int h);

    // Blend using Layer blend mode and opacity
    void blendLayerOntoAccumulator(const Layer& layer, GLuint srcTex,
                                   ShaderManager& shaderMgr, FullscreenQuad& quad,
                                   int w, int h);

    // Apply FX Only layer: run clip effects on the accumulator
    void applyFXOnlyLayer(const Clip& clip, const Layer& layer, uint64_t stateKey,
                          ShaderManager& shaderMgr,
                          FullscreenQuad& quad, float time, int w, int h);

    // Apply Mask layer: use clip content as luminance mask on accumulator
    void applyMaskLayer(const Clip& clip, GLuint clipTex,
                        ShaderManager& shaderMgr, FullscreenQuad& quad,
                        int w, int h);

    // Get texture for any clip (image, source, video, or image sequence)
    GLuint getClipTexture(const Clip& clip, float time, int w, int h, float dt);

    // Apply transition shader: blend previous clip texture with new clip texture
    // Returns the blended texture. The outgoing clip gets the same per-clip
    // stages as the active clip (transform + opacity, then effects); newClipTex
    // is HELD across all of them (holdTex=newClipTex).
    // outgoingKey: LayerStateKey::outgoingChain of the layer -- the outgoing
    // clip's chain keeps its own temporal buffer / frame ring for the whole
    // fade, never the incoming clip's (s-rta-0926b R1, handOverClipHistory).
    GLuint applyTransition(Layer& layer, uint64_t outgoingKey, GLuint newClipTex, float time,
                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                           int w, int h, float dt);

    // P14 + S167-L4b DT-FIX: advance a layer's clip-to-clip crossfade by the
    // real frame delta (see compositeDeck()'s header comment). Called once per
    // frame for every visible layer that is composited, active deck or
    // persistent (s-rta-0926b R4).
    static void advanceCrossfade(Layer& layer, float dt);

    // s-rta-0926b R4: every stage an Opaque/Transparent layer applies to its
    // active clip's texture before compositing -- clip transform + opacity,
    // clip effects, clip-to-clip transition, feedback, layer effects, layer
    // transform. ONE function for the active deck (compositeDeck) and for
    // persistent layers of other decks (compositePersistentLayers), so a
    // persistent layer renders like the same layer would on the active deck.
    GLuint renderLayerStages(Layer& layer, uint32_t deckId, const Clip& clip, GLuint clipTex,
                             ShaderManager& shaderMgr, FullscreenQuad& quad,
                             float time, float dt, int w, int h);

    // Map Layer::MixMode transition enum to shader name
    static juce::String getTransitionShaderName(Layer::MixMode mode);

    // === Layer Router Support (P20) ===
    // Per-layer saved output textures, keyed by layer ID.
    // Updated during compositeDeck() after each layer is rendered.
    // Layer Router sources read from this map.
    std::unordered_map<uint32_t, GLuint> layerOutputTextures_;

    // Dedicated FBOs for saving per-layer output (separate from effect/scratch FBOs)
    std::unordered_map<uint32_t, GLuint> layerOutputFBOs_;
    std::unordered_map<uint32_t, GLuint> layerOutputTexStorage_;

    // Create/get a layer output FBO/texture pair
    void ensureLayerOutputFBO(uint32_t layerId, int w, int h);

    // Save the current clip texture into the layer's output storage
    void saveLayerOutput(uint32_t layerId, GLuint srcTex,
                         ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);

public:
    // Get the saved output texture for a specific layer (for Layer Router)
    GLuint getLayerOutputTexture(uint32_t layerId) const
    {
        auto it = layerOutputTextures_.find(layerId);
        return (it != layerOutputTextures_.end()) ? it->second : 0;
    }

    // Get all available layer IDs that have saved output
    std::vector<uint32_t> getAvailableLayerIds() const
    {
        std::vector<uint32_t> ids;
        for (const auto& [id, tex] : layerOutputTextures_)
            ids.push_back(id);
        return ids;
    }
};
