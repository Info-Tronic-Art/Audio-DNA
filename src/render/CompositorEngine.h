#pragma once
#include <juce_opengl/juce_opengl.h>
#include "keyboard/KeySlot.h"
#include "render/ShaderManager.h"
#include "render/TextureManager.h"
#include "render/FullscreenQuad.h"
#include <unordered_map>
#include <string>

// CompositorEngine: multi-layer compositing for the keyboard launcher.
//
// Each active key is rendered onto an accumulator FBO in activation order.
// Keys with images get their image rendered, keying mode applied (to generate
// alpha), then blended onto the accumulator using the key's blend mode.
// Keys with effects-only apply their effects directly to the accumulator.
//
// The final composited result texture is returned for the global effect chain.
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

    // Load an image for a specific key. Call from message thread (queued).
    // Returns the GL texture ID, or 0 on failure.
    GLuint loadKeyImage(const juce::File& imageFile);

    // Get or create a texture for a key's image file (cached).
    GLuint getKeyTexture(const juce::File& imageFile);

    // Composite all active keys and return the result texture.
    // Returns 0 if no keys are active (caller should render normally).
    GLuint composite(KeyboardLayout& layout,
                     ShaderManager& shaderMgr,
                     FullscreenQuad& quad,
                     float time,
                     int width, int height);

    bool hasActiveKeys() const { return hasActiveKeys_; }

private:
    // Accumulator FBO — the composited result of all keys
    GLuint accumulatorFBO_ = 0;
    GLuint accumulatorTex_ = 0;

    // Scratch FBO for per-key rendering (keying pass)
    GLuint scratchFBO_ = 0;
    GLuint scratchTex_ = 0;

    int fboWidth_ = 0;
    int fboHeight_ = 0;
    bool glInitialized_ = false;
    bool hasActiveKeys_ = false;

    // Texture cache: file path → GL texture ID
    std::unordered_map<std::string, GLuint> textureCache_;

    void createFBO(GLuint& fbo, GLuint& tex, int w, int h);
    void deleteFBO(GLuint& fbo, GLuint& tex);

    // Apply keying mode to generate alpha, rendering from srcTex to dstFBO
    void applyKeying(const KeySlot& key, GLuint srcTex, GLuint dstFBO,
                     ShaderManager& shaderMgr, FullscreenQuad& quad,
                     int w, int h);

    // Blend srcTex onto accumulatorFBO using the key's blend mode
    void blendOntoAccumulator(const KeySlot& key, GLuint srcTex,
                              ShaderManager& shaderMgr, FullscreenQuad& quad,
                              int w, int h);
};
