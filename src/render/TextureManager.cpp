#include "TextureManager.h"
#include <vector>
#include <iostream>

using namespace juce::gl;

TextureManager::~TextureManager()
{
    jassert(imageTexID_ == 0 && fbos_[0] == 0);
}

bool TextureManager::loadImage(const juce::File& imageFile)
{
    std::cerr << "[TextureManager] loading: " << imageFile.getFullPathName() << std::endl;

    auto image = juce::ImageFileFormat::loadFrom(imageFile);
    if (!image.isValid())
    {
        std::cerr << "[TextureManager] FAILED to decode image" << std::endl;
        return false;
    }

    // Delete old texture if any
    if (imageTexID_ != 0)
    {
        glDeleteTextures(1, &imageTexID_);
        imageTexID_ = 0;
    }

    imageWidth_ = image.getWidth();
    imageHeight_ = image.getHeight();

    std::cerr << "[TextureManager] image size " << imageWidth_ << "x" << imageHeight_ << std::endl;

    // Convert to ARGB and extract pixels into a clean RGBA buffer for OpenGL.
    // JUCE ARGB format stores pixels as 0xAARRGGBB in memory (native endian).
    // On little-endian (x86/ARM): byte order is BB GG RR AA.
    // We convert manually to GL_RGBA byte order: RR GG BB AA.
    auto argbImage = image.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bitmapData(argbImage, juce::Image::BitmapData::readOnly);

    std::vector<uint8_t> rgbaPixels(static_cast<size_t>(imageWidth_ * imageHeight_ * 4));

    for (int y = 0; y < imageHeight_; ++y)
    {
        // Flip Y: OpenGL texture origin is bottom-left, image is top-left
        auto* srcRow = bitmapData.getLinePointer(imageHeight_ - 1 - y);
        auto* dstRow = &rgbaPixels[static_cast<size_t>(y * imageWidth_ * 4)];

        for (int x = 0; x < imageWidth_; ++x)
        {
            // JUCE ARGB pixel: bytes are [B, G, R, A] on little-endian
            auto* srcPixel = srcRow + x * 4;
            auto* dstPixel = dstRow + x * 4;

            dstPixel[0] = srcPixel[2]; // R
            dstPixel[1] = srcPixel[1]; // G
            dstPixel[2] = srcPixel[0]; // B
            dstPixel[3] = srcPixel[3]; // A
        }
    }

    glGenTextures(1, &imageTexID_);
    glBindTexture(GL_TEXTURE_2D, imageTexID_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 imageWidth_, imageHeight_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE,
                 rgbaPixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    std::cerr << "[TextureManager] texture uploaded, ID=" << imageTexID_ << std::endl;
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
