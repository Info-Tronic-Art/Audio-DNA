#pragma once
#include <juce_opengl/juce_opengl.h>
#include "model/Deck.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
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

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);

    // Apply per-clip transform (position/scale/rotation) to a texture
    GLuint applyClipTransform(const Clip& clip, GLuint srcTex,
                               ShaderManager& shaderMgr, FullscreenQuad& quad,
                               int w, int h);

    // Apply per-clip effect chain to a texture, returns result texture ID.
    // Uses effectFBO_A_/B_ for ping-pong rendering.
    GLuint applyClipEffects(const Clip& clip, GLuint inputTex,
                            ShaderManager& shaderMgr, FullscreenQuad& quad,
                            float time, int w, int h);

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
