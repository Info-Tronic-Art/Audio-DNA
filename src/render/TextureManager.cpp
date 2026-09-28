#include "TextureManager.h"
#include "render/LUTLoader.h"
#include "render/PixelConvert.h"
#include <vector>
#include <iostream>
#include <chrono>

using namespace juce::gl;

TextureManager::~TextureManager()
{
    jassert(imageTexID_ == 0 && fbos_[0] == 0);
}

bool TextureManager::loadImage(const juce::File& imageFile)
{
    std::cerr << "[TextureManager] loading: " << imageFile.getFullPathName() << std::endl;

    // s-rta-0928 R1.0: the decode / convert / upload split of the legacy single image (GL thread).
    const auto t0 = std::chrono::steady_clock::now();
    auto image = juce::ImageFileFormat::loadFrom(imageFile);
    if (!image.isValid())
    {
        std::cerr << "[TextureManager] FAILED to decode image" << std::endl;
        return false;
    }
    const double decodeMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();

    const bool ok = uploadImage(image);
    std::cerr << "[Image] legacy " << imageFile.getFullPathName() << " (" << image.getWidth() << "x"
              << image.getHeight() << ") decode=" << juce::String(decodeMs, 1) << " convert="
              << juce::String(lastConvertMs_, 1) << " upload=" << juce::String(lastUploadMs_, 1) << " ms" << std::endl;
    return ok;
}

bool TextureManager::uploadImage(const juce::Image& image)
{
    if (!image.isValid())
        return false;

    int w = image.getWidth();
    int h = image.getHeight();

    const auto tConvert = std::chrono::steady_clock::now();   // s-rta-0928 R1.0 (loadImage's split line)
    // Convert to ARGB and extract pixels into a clean RGBA buffer for OpenGL: the raw premultiplied bytes, swizzled
    // B,G,R,A -> R,G,B,A and flipped. s-rta-0928 R1.1: one row pass, byte-identical to the old swizzle loop
    // (tests/test_pixel_convert.cpp).
    auto argbImage = image.convertedToFormat(juce::Image::ARGB);
    std::vector<uint8_t> rgbaPixels(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    {
        const juce::Image::BitmapData bitmapData(argbImage, juce::Image::BitmapData::readOnly);
        PixelConvert::argbToGlRgbaBottomUp(bitmapData, rgbaPixels.data(), false);
    }

    const auto tUpload = std::chrono::steady_clock::now();
    lastConvertMs_ = std::chrono::duration<double, std::milli>(tUpload - tConvert).count();
    const auto uploadDone = [this, tUpload] {
        lastUploadMs_ = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tUpload).count();
    };

    // If texture exists at same size, just update it (glTexSubImage2D is faster)
    if (imageTexID_ != 0 && imageWidth_ == w && imageHeight_ == h)
    {
        glBindTexture(GL_TEXTURE_2D, imageTexID_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h,
                        GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        uploadDone();
        return true;
    }

    // Create new texture
    if (imageTexID_ != 0)
    {
        glDeleteTextures(1, &imageTexID_);
        imageTexID_ = 0;
    }

    imageWidth_ = w;
    imageHeight_ = h;

    glGenTextures(1, &imageTexID_);
    glBindTexture(GL_TEXTURE_2D, imageTexID_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);
    uploadDone();

    std::cerr << "[TextureManager] texture uploaded, ID=" << imageTexID_
              << ", size=" << w << "x" << h << std::endl;
    return true;
}

void TextureManager::createFBOs(int width, int height)
{
    if (width == fboWidth_ && height == fboHeight_ && fbos_[0] != 0)
        return;

    releaseFBOs();

    fboWidth_ = width;
    fboHeight_ = height;

    glGenFramebuffers(2, fbos_);
    glGenTextures(2, fboTextures_);

    for (int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, fboTextures_[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, fbos_[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, fboTextures_[i], 0);

        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
            DBG("TextureManager: FBO " + juce::String(i) + " incomplete, status=" + juce::String(status));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureManager::releaseFBOs()
{
    if (fbos_[0] != 0)
    {
        glDeleteFramebuffers(2, fbos_);
        fbos_[0] = fbos_[1] = 0;
    }
    if (fboTextures_[0] != 0)
    {
        glDeleteTextures(2, fboTextures_);
        fboTextures_[0] = fboTextures_[1] = 0;
    }
    fboWidth_ = fboHeight_ = 0;
}

GLuint TextureManager::loadLUT(const juce::File& file)
{
    return LUTLoader::loadCubeFile(file);
}

void TextureManager::releaseLUT(GLuint texId)
{
    LUTLoader::releaseLUT(texId);
}

void TextureManager::release()
{
    if (imageTexID_ != 0)
    {
        glDeleteTextures(1, &imageTexID_);
        imageTexID_ = 0;
    }
    imageWidth_ = imageHeight_ = 0;
    releaseFBOs();
}
