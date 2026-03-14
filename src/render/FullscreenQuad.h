#pragma once
#include <juce_opengl/juce_opengl.h>

// A fullscreen quad rendered as a triangle strip.
// Covers the entire viewport with UV coordinates [0,1].
// All GL resources are created/destroyed on the GL thread.
class FullscreenQuad
{
public:
    FullscreenQuad() = default;
    ~FullscreenQuad();

    // Must be called on the GL thread (e.g., in newOpenGLContextCreated).
    void init();

    // Must be called on the GL thread (e.g., in openGLContextClosing).
    void release();

    // Bind VAO and draw the quad. Assumes a shader is already bound.
    void draw() const;

    FullscreenQuad(const FullscreenQuad&) = delete;
    FullscreenQuad& operator=(const FullscreenQuad&) = delete;

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    bool initialized_ = false;
};
