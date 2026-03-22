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
    GLuint compositeDeck(Deck& deck,
                         ShaderManager& shaderMgr,
                         FullscreenQuad& quad,
                         float time,
                         int width, int height);

    bool hasActiveLayers() const { return hasActiveLayers_; }

    // Copy the current composited frame into the persistent feedback buffer.
    // Call AFTER compositeDeck() each frame.
    void updateFeedbackBuffer(ShaderManager& shaderMgr, FullscreenQuad& quad, int w, int h);

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

    // Apply per-clip transform (position/scale/rotation) to a texture
    GLuint applyClipTransform(const Clip& clip, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h);

    // Apply per-clip effect chain to a texture, returns result texture ID.
    // Uses effectFBO_A_/B_ for ping-pong rendering.
    // layerId: used to key per-layer temporal buffers for time effects (u_prev_frame).
    GLuint applyClipEffects(const Clip& clip, GLuint inputTex,
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
};
