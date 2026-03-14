#include "FullscreenQuad.h"

using namespace juce::gl;

FullscreenQuad::~FullscreenQuad()
{
    // If release() wasn't called explicitly, resources leak.
    // In practice, Renderer calls release() in openGLContextClosing().
    jassert(!initialized_);
}

void FullscreenQuad::init()
{
    jassert(!initialized_);

    // 4 vertices: position (x, y) + texcoord (u, v)
    // Triangle strip: bottom-left, bottom-right, top-left, top-right
    static constexpr float vertices[] = {
        // x      y     u     v
        -1.0f, -1.0f,  0.0f, 0.0f,   // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
        -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
         1.0f,  1.0f,  1.0f, 1.0f    // top-right
    };

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Attribute 0: position (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

    // Attribute 1: texcoord (vec2)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<const void*>(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    initialized_ = true;
}

void FullscreenQuad::release()
{
    if (!initialized_)
        return;

    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
    vao_ = 0;
    vbo_ = 0;
    initialized_ = false;
}

void FullscreenQuad::draw() const
{
    jassert(initialized_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
