#pragma once
#include <juce_opengl/juce_opengl.h>
#include "keyboard/KeySlot.h"
#include "model/Deck.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include <unordered_map>
#include <string>

// CompositorEngine: multi-layer compositing for both v1 keyboard and v2 deck modes.
//
// v1 (keyboard): Each active key composited in activation order.
// v2 (deck): Each layer composited bottom-to-top with per-layer blend/keying.
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

    // Load an image for a specific key/clip. Call from message thread (queued).
    // Returns the GL texture ID, or 0 on failure.
    GLuint loadKeyImage(const juce::File& imageFile);

    // Get or create a texture for a key/clip's image file (cached).
    GLuint getKeyTexture(const juce::File& imageFile);

    // === v1: Keyboard-based compositing (backward compatible) ===
    GLuint composite(KeyboardLayout& layout,
                     ShaderManager& shaderMgr,
                     FullscreenQuad& quad,
                     float time,
                     int width, int height);

    bool hasActiveKeys() const { return hasActiveKeys_; }

    // === v2: Deck/Layer-based compositing ===
    // Composite all layers in the deck and return the result texture.
    // Returns 0 if no layers have active clips.
    GLuint compositeDeck(Deck& deck,
                         ShaderManager& shaderMgr,
                         FullscreenQuad& quad,
                         float time,
                         int width, int height);

    bool hasActiveLayers() const { return hasActiveLayers_; }

private:
    // Accumulator FBO — the composited result
    GLuint accumulatorFBO_ = 0;
    GLuint accumulatorTex_ = 0;

    // Scratch FBO for per-key/per-layer rendering (keying pass)
    GLuint scratchFBO_ = 0;
    GLuint scratchTex_ = 0;

    int fboWidth_ = 0;
    int fboHeight_ = 0;
    bool glInitialized_ = false;
    bool hasActiveKeys_ = false;
    bool hasActiveLayers_ = false;

    // Texture cache: file path → GL texture ID
    std::unordered_map<std::string, GLuint> textureCache_;

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);

    // v1: Apply keying mode from KeySlot
    void applyKeying(const KeySlot& key, GLuint srcTex, GLuint dstFBO,
                     ShaderManager& shaderMgr, FullscreenQuad& quad,
                     int w, int h);

    // v1: Blend using KeySlot blend mode
    void blendOntoAccumulator(const KeySlot& key, GLuint srcTex,
                              ShaderManager& shaderMgr, FullscreenQuad& quad,
                              int w, int h);

    // v2: Apply keying mode from Layer
    void applyLayerKeying(const Layer& layer, GLuint srcTex, GLuint dstFBO,
                          ShaderManager& shaderMgr, FullscreenQuad& quad,
                          int w, int h);

    // v2: Blend using Layer blend mode and opacity
    void blendLayerOntoAccumulator(const Layer& layer, GLuint srcTex,
                                   ShaderManager& shaderMgr, FullscreenQuad& quad,
                                   int w, int h);
};
