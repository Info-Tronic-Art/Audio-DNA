#pragma once
#include <juce_opengl/juce_opengl.h>
#include <juce_gui_basics/juce_gui_basics.h>

// TextureManager: loads image files into GL textures and creates FBO textures
// for the effect chain's ping-pong rendering.
//
// All GL methods must be called on the GL thread.
class TextureManager
{
public:
    TextureManager() = default;
    ~TextureManager();

    // Upload a JUCE Image directly (for camera frames): the convert (PixelConvert, premultiplied) + uploadPixels.
    // Must be called on the GL thread.
    bool uploadImage(const juce::Image& image);

    // s-rta-0928 R1.3: upload GL-ready RGBA8 rows (bottom-up, premultiplied, w*4 per row) into the main image texture
    // -- the GL half of uploadImage, verbatim (glTexSubImage2D at the same size, else a new texture). The legacy single
    // image's bytes come from ImageDecode (decoded off the GL thread). Must be called on the GL thread.
    bool uploadPixels(const uint8_t* rgba, int w, int h);

    // Create (or resize) the two FBO textures for ping-pong rendering.
    void createFBOs(int width, int height);

    // Release all GL resources.
    void release();

    // Getters
    GLuint getImageTexture() const { return imageTexID_; }
    int getImageWidth() const { return imageWidth_; }
    int getImageHeight() const { return imageHeight_; }
    bool hasImage() const { return imageTexID_ != 0; }

    // LUT loading (.cube files → GL_TEXTURE_3D)
    GLuint loadLUT(const juce::File& file);
    void releaseLUT(GLuint texId);

    // FBO access for ping-pong rendering
    GLuint getFBO(int index) const { return fbos_[index]; }
    GLuint getFBOTexture(int index) const { return fboTextures_[index]; }
    int getFBOWidth() const { return fboWidth_; }
    int getFBOHeight() const { return fboHeight_; }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

private:
    void releaseFBOs();

    // Image texture (manual GL upload with correct pixel format handling)
    GLuint imageTexID_ = 0;
    int imageWidth_ = 0;
    int imageHeight_ = 0;

    // Ping-pong FBOs (2 FBOs, 2 color textures)
    GLuint fbos_[2] = {0, 0};
    GLuint fboTextures_[2] = {0, 0};
    int fboWidth_ = 0;
    int fboHeight_ = 0;
};
