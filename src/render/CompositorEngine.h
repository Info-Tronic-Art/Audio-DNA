#pragma once
#include <juce_opengl/juce_opengl.h>
#include "model/Deck.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include "render/FeedbackProcessor.h"
#include "effects/EffectLibrary.h"
#include "effects/Effect.h"
#include "analysis/FeatureSnapshot.h"
#include <unordered_map>
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

private:
    // Accumulator FBO — the composited result
    GLuint accumulatorFBO_ = 0;
    GLuint accumulatorTex_ = 0;

    // Scratch FBO for per-key/per-layer rendering (keying pass)
    GLuint scratchFBO_ = 0;
    GLuint scratchTex_ = 0;

    // Effect ping-pong FBOs for per-clip effect chains
    GLuint effectFBO_A_ = 0;
    GLuint effectTex_A_ = 0;
    GLuint effectFBO_B_ = 0;
    GLuint effectTex_B_ = 0;

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

    // S166: reserved layerId passed to applyClipEffects() for the Global
    // Effects call (see applyGlobalEffects() above), so its temporal buffer
    // (layerTemporalBuffers_ below) and screen-split ring buffer
    // (layerRingBuffers_ below) never alias a real layer's. Real layer ids
    // are assigned sequentially starting at 0 (Deck.h) — including 0 itself,
    // the bottom layer — so 0 is NOT a safe "no real layer" sentinel; the
    // max uint32_t value is.
    static constexpr uint32_t kGlobalEffectsLayerId = 0xFFFFFFFFu;

    // Per-layer feedback processors (keyed by layer ID)
    std::unordered_map<uint32_t, std::unique_ptr<FeedbackProcessor>> feedbackProcessors_;

    // Get or create a feedback processor for a layer
    FeedbackProcessor& getOrCreateFeedbackProcessor(uint32_t layerId);

    // Per-layer temporal FBOs for time effects (u_prev_frame)
    // Each layer that uses temporal effects gets its own persistent prev-frame buffer.
    struct TemporalBuffer {
        GLuint fbo = 0;
        GLuint tex = 0;
        int width = 0;
        int height = 0;
    };
    std::unordered_map<uint32_t, TemporalBuffer> layerTemporalBuffers_;

    // Get or create a temporal buffer for a layer, resized if needed
    TemporalBuffer& getOrCreateTemporalBuffer(uint32_t layerId, int w, int h);

    // Save a texture into a temporal buffer (passthrough copy)
    void saveToTemporalBuffer(TemporalBuffer& buf, GLuint srcTex,
                              ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);

    // Frame ring buffer for Screen Split / Frame Stutter.
    // Stores up to 480 previous frames at reduced resolution (~120MB at 480x270).
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
    std::unordered_map<uint32_t, FrameRingBuffer> layerRingBuffers_;

    FrameRingBuffer& getOrCreateRingBuffer(uint32_t layerId, int w, int h);
    void pushFrameToRing(FrameRingBuffer& ring, GLuint srcTex,
                         ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);
    GLuint getFrameFromRing(const FrameRingBuffer& ring, int framesAgo) const;

    // Screen Split: render a grid of delayed copies of the clip texture
    // Returns the composited grid texture, or 0 if not a screen split effect.
    GLuint applyScreenSplit(GLuint clipTex, const Clip::EffectSlot& slot,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            uint32_t layerId, int w, int h);

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);

    // Upload audio feature uniforms to the current shader program.
    // Used by audio-reactive effects (P18) that need chromagram, MFCCs, etc.
    void uploadAudioUniforms(juce::OpenGLShaderProgram* program) const;

    // Apply per-clip transform (position/scale/rotation) to a texture
    GLuint applyClipTransform(const Clip& clip, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h);

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
    GLuint applyClipOpacity(float opacity, GLuint srcTex, GLuint dstFBO, GLuint dstTex,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            int w, int h);

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
    // Uses effectFBO_A_/B_ for ping-pong rendering.
    // layerId: used to key per-layer temporal buffers for time effects (u_prev_frame).
    GLuint applyClipEffects(const std::vector<Clip::EffectSlot>& effects, GLuint inputTex,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            float time, int w, int h,
                            uint32_t layerId = 0);

    // Apply layer transform (translate/scale/rotate) to a texture
    GLuint applyLayerTransform(const Layer& layer, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h);

    // Apply keying mode from Layer
    void applyLayerKeying(const Layer& layer, GLuint srcTex, GLuint dstFBO,
                          ShaderManager& shaderMgr, FullscreenQuad& quad,
                          int w, int h);

    // Blend using Layer blend mode and opacity
    void blendLayerOntoAccumulator(const Layer& layer, GLuint srcTex,
                                   ShaderManager& shaderMgr, FullscreenQuad& quad,
                                   int w, int h);

    // Apply FX Only layer: run clip effects on the accumulator
    void applyFXOnlyLayer(const Clip& clip, const Layer& layer,
                          ShaderManager& shaderMgr,
                          FullscreenQuad& quad, float time, int w, int h);

    // Apply Mask layer: use clip content as luminance mask on accumulator
    void applyMaskLayer(const Clip& clip, GLuint clipTex,
                        ShaderManager& shaderMgr, FullscreenQuad& quad,
                        int w, int h);

    // Get texture for any clip (image, source, video, or image sequence)
    GLuint getClipTexture(const Clip& clip, float time, int w, int h, float dt);

    // Apply transition shader: blend previous clip texture with new clip texture
    // Returns the blended texture. Uses scratchFBO_ as intermediate.
    GLuint applyTransition(Layer& layer, GLuint newClipTex, float time,
                           ShaderManager& shaderMgr, FullscreenQuad& quad,
                           int w, int h, float dt);

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
