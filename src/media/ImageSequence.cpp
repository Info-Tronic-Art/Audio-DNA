// ImageSequence.cpp — Treats a set of images as a playable video clip.

#include "ImageSequence.h"
#include "render/PixelConvert.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <chrono>

using namespace juce::gl;

ImageSequence::ImageSequence() = default;

ImageSequence::~ImageSequence()
{
    close();
}

bool ImageSequence::open(const std::vector<juce::File>& imageFiles)
{
    close();

    if (imageFiles.empty())
        return false;

    // Filter to supported image formats and sort alphabetically
    files_.clear();
    for (const auto& f : imageFiles)
    {
        auto ext = f.getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tiff" || ext == ".gif")
        {
            if (f.existsAsFile())
                files_.push_back(f);
        }
    }

    if (files_.empty())
        return false;

    // Sort alphabetically for consistent ordering
    std::sort(files_.begin(), files_.end(), [](const juce::File& a, const juce::File& b) {
        return a.getFileName().compareNatural(b.getFileName()) < 0;
    });

    // Initialize texture tracking
    textures_.resize(files_.size(), 0);
    textureWidths_.resize(files_.size(), 0);
    textureHeights_.resize(files_.size(), 0);

    // Get dimensions from first image
    auto firstImg = juce::ImageFileFormat::loadFrom(files_[0]);
    if (firstImg.isValid())
    {
        width_ = firstImg.getWidth();
        height_ = firstImg.getHeight();
    }
    else
    {
        width_ = 1920;
        height_ = 1080;
    }

    currentTime_ = 0.0;
    currentFrameIndex_ = 0;
    pingPongForward_ = true;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
    playing_.store(true, std::memory_order_relaxed);
    open_.store(true, std::memory_order_relaxed);

    std::cerr << "[ImageSequence] Opened " << files_.size() << " images"
              << " (" << width_ << "x" << height_ << ")"
              << " at " << fps_.load() << " fps" << std::endl;

    return true;
}

bool ImageSequence::openDirectory(const juce::File& directory)
{
    if (!directory.isDirectory())
        return false;

    std::vector<juce::File> images;
    for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*.png;*.jpg;*.jpeg;*.bmp;*.tiff;*.gif"))
        images.push_back(entry.getFile());

    return open(images);
}

void ImageSequence::close()
{
    open_.store(false, std::memory_order_relaxed);
    // Note: GL textures must be released on GL thread via releaseGL()
    files_.clear();
    // Don't clear textures_ here — releaseGL() handles that
    width_ = 0;
    height_ = 0;
    currentTime_ = 0.0;
    currentFrameIndex_ = 0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
}

double ImageSequence::getDuration() const
{
    float fps = fps_.load(std::memory_order_relaxed);
    if (fps <= 0.0f) return 0.0;
    return static_cast<double>(files_.size()) / static_cast<double>(fps);
}

void ImageSequence::seekTo(double normalizedPosition)
{
    normalizedPosition = std::clamp(normalizedPosition, 0.0, 1.0);
    double dur = getDuration();
    currentTime_ = normalizedPosition * dur;
    currentFrameIndex_ = static_cast<int>(normalizedPosition * static_cast<double>(files_.size() - 1));
    currentFrameIndex_ = std::clamp(currentFrameIndex_, 0, static_cast<int>(files_.size()) - 1);
    playheadPosition_.store(normalizedPosition, std::memory_order_relaxed);
}

void ImageSequence::advanceFrame(double dt)
{
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return;

    if (!playing_.load(std::memory_order_relaxed))
        return;

    double dur = getDuration();
    if (dur <= 0.0)
        return;

    float speed = speed_.load(std::memory_order_relaxed);
    bool reverse = reverse_.load(std::memory_order_relaxed);
    auto loopMode = loopMode_.load(std::memory_order_relaxed);

    double direction = 1.0;
    if (loopMode == LoopMode::PingPong)
        direction = pingPongForward_ ? 1.0 : -1.0;
    if (reverse)
        direction = -direction;

    currentTime_ += dt * static_cast<double>(speed) * direction;

    // Handle boundaries
    if (currentTime_ >= dur)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = std::fmod(currentTime_, dur);
                break;
            case LoopMode::PingPong:
                currentTime_ = dur - (currentTime_ - dur);
                pingPongForward_ = false;
                break;
            case LoopMode::OneShot:
                currentTime_ = dur;
                playing_.store(false, std::memory_order_relaxed);
                break;
        }
    }
    else if (currentTime_ < 0.0)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = dur + std::fmod(currentTime_, dur);
                break;
            case LoopMode::PingPong:
                currentTime_ = -currentTime_;
                pingPongForward_ = true;
                break;
            case LoopMode::OneShot:
                currentTime_ = 0.0;
                playing_.store(false, std::memory_order_relaxed);
                break;
        }
    }

    // Calculate current frame index from time
    float fps = fps_.load(std::memory_order_relaxed);
    int frameIdx = static_cast<int>(currentTime_ * static_cast<double>(fps));
    frameIdx = std::clamp(frameIdx, 0, static_cast<int>(files_.size()) - 1);
    currentFrameIndex_ = frameIdx;

    // Update playhead
    double pos = (dur > 0.0) ? (currentTime_ / dur) : 0.0;
    playheadPosition_.store(std::clamp(pos, 0.0, 1.0), std::memory_order_relaxed);
}

GLuint ImageSequence::getCurrentTexture()
{
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return 0;

    int idx = std::clamp(currentFrameIndex_, 0, static_cast<int>(files_.size()) - 1);

    // Lazy load: create texture on first access
    if (textures_[static_cast<size_t>(idx)] == 0)
        return loadImageToTexture(idx);

    return textures_[static_cast<size_t>(idx)];
}

void ImageSequence::releaseGL()
{
    for (auto& tex : textures_)
    {
        if (tex != 0)
        {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    }
    textures_.clear();
    textureWidths_.clear();
    textureHeights_.clear();
}

juce::Image ImageSequence::getThumbnail(int maxWidth, int maxHeight)
{
    if (files_.empty())
        return {};

    auto img = juce::ImageFileFormat::loadFrom(files_[0]);
    if (!img.isValid())
        return {};

    float scaleX = static_cast<float>(maxWidth) / static_cast<float>(img.getWidth());
    float scaleY = static_cast<float>(maxHeight) / static_cast<float>(img.getHeight());
    float scale = std::min(scaleX, scaleY);
    int thumbW = std::max(1, static_cast<int>(static_cast<float>(img.getWidth()) * scale));
    int thumbH = std::max(1, static_cast<int>(static_cast<float>(img.getHeight()) * scale));

    return img.rescaled(thumbW, thumbH, juce::Graphics::lowResamplingQuality);
}

GLuint ImageSequence::loadImageToTexture(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= static_cast<int>(files_.size()))
        return 0;

    // s-rta-0928 R1.0: the decode / convert / upload split of a sequence frame's GL-thread load.
    using ImgClock = std::chrono::steady_clock;
    const auto msSince = [](ImgClock::time_point t) {
        return std::chrono::duration<double, std::milli>(ImgClock::now() - t).count();
    };
    const auto tDecode = ImgClock::now();
    auto img = juce::ImageFileFormat::loadFrom(files_[static_cast<size_t>(frameIndex)]);
    if (!img.isValid())
        return 0;

    img = img.convertedToFormat(juce::Image::ARGB);
    const double decodeMs = msSince(tDecode);
    const auto tConvert = ImgClock::now();
    int w = img.getWidth();
    int h = img.getHeight();

    // Convert JUCE ARGB → GL RGBA (straight) and flip Y. s-rta-0928 R1.1: one row pass, byte-identical to the old
    // getPixelColour loop (tests/test_pixel_convert.cpp).
    std::vector<uint8_t> rgba(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    {
        const juce::Image::BitmapData bmp(img, juce::Image::BitmapData::readOnly);
        PixelConvert::argbToGlRgbaBottomUp(bmp, rgba.data(), true);
    }

    const double convertMs = msSince(tConvert);
    const auto tUpload = ImgClock::now();
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    std::cerr << "[Image] sequence " << files_[static_cast<size_t>(frameIndex)].getFileName() << " (" << w << "x" << h
              << ") decode=" << juce::String(decodeMs, 1) << " convert=" << juce::String(convertMs, 1)
              << " upload=" << juce::String(msSince(tUpload), 1) << " ms" << std::endl;

    textures_[static_cast<size_t>(frameIndex)] = tex;
    textureWidths_[static_cast<size_t>(frameIndex)] = w;
    textureHeights_[static_cast<size_t>(frameIndex)] = h;

    return tex;
}
