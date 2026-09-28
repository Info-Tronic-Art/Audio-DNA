// ImageSequence.cpp — Treats a set of images as a playable video clip.

#include "ImageSequence.h"
#include <algorithm>
#include <iostream>
#include <cstring>

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
    // s-rta-0928 R1.4: a new open -- results of earlier requests are dropped by their generation
    ++openGen_;
    requested_.assign(files_.size(), 0);
    failed_.assign(files_.size(), 0);
    ready_.clear();
    lastShown_ = -1;

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

void ImageSequence::ensureFrameState()
{
    const size_t n = files_.size();
    if (textures_.size() != n) { textures_.resize(n, 0); textureWidths_.resize(n, 0); textureHeights_.resize(n, 0); }
    if (requested_.size() != n) requested_.assign(n, 0);
    if (failed_.size() != n) failed_.assign(n, 0);
}

void ImageSequence::requestFrame(ImageDecode::Decoder& decoder, int idx)
{
    const auto i = static_cast<size_t>(idx);
    if (requested_[i] != 0 || textures_[i] != 0 || failed_[i] != 0 || outstanding_ >= kMaxOutstanding)
        return;
    requested_[i] = 1;
    ++outstanding_;
    decoder.request(files_[i], ImageDecode::Layout::StraightRGBA,
                    (static_cast<uint64_t>(openGen_) << 32) | static_cast<uint32_t>(idx), std::nullopt, box_);
}

// The next kLookAhead frames in the play direction: Loop wraps, PingPong follows its current direction, OneShot stops
// at the end.
void ImageSequence::requestAhead(ImageDecode::Decoder& decoder, int idx)
{
    const int n = static_cast<int>(files_.size());
    const auto mode = loopMode_.load(std::memory_order_relaxed);
    bool forward = !reverse_.load(std::memory_order_relaxed);
    if (mode == LoopMode::PingPong && !pingPongForward_)
        forward = !forward;
    int j = idx;
    for (int k = 0; k < kLookAhead; ++k)
    {
        j += forward ? 1 : -1;
        if (j >= n || j < 0)
        {
            if (mode != LoopMode::Loop)
                break;
            j = (j + n) % n;
        }
        requestFrame(decoder, j);
    }
}

// The GL calls of the old loadImageToTexture, exactly.
GLuint ImageSequence::uploadFrame(const ImageDecode::Result& r, int idx)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, r.rgba.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    textures_[static_cast<size_t>(idx)] = tex;
    textureWidths_[static_cast<size_t>(idx)] = r.w;
    textureHeights_[static_cast<size_t>(idx)] = r.h;
    return tex;
}

GLuint ImageSequence::getCurrentTexture(ImageDecode::Decoder& decoder, ImageTexCache::UploadBudget& budget,
                                        const SeqVram::Grant& grant, bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return 0;
    ensureFrameState();   // also after releaseGL (context loss), which empties the per-frame vectors
    const int n = static_cast<int>(files_.size());

    // Arrived frames of this open: upload within the frame's budget (the rest wait, never re-decoded).
    std::vector<ImageDecode::Result> got;
    if (box_->tryDrain(got))
        for (auto& r : got)
        {
            outstanding_ = std::max(0, outstanding_ - 1);
            ready_.push_back(std::move(r));
        }
    for (auto it = ready_.begin(); it != ready_.end();)
    {
        const uint32_t gen = static_cast<uint32_t>(it->tag >> 32);
        const int idx = static_cast<int>(static_cast<uint32_t>(it->tag));
        if (gen != openGen_ || idx < 0 || idx >= n || textures_[static_cast<size_t>(idx)] != 0)
        {
            it = ready_.erase(it);
            continue;
        }
        if (it->kind != ImageTexCache::Kind::Decoded)
        {
            failed_[static_cast<size_t>(idx)] = 1;   // never re-requested: the last frame repeats (R-7)
            it = ready_.erase(it);
            continue;
        }
        if (!budget.take(it->rgba.size()))
        {
            ++it;
            continue;
        }
        uploadFrame(*it, idx);
        if (grant.stats != nullptr)
            grant.stats->uploads.fetch_add(1, std::memory_order_relaxed);
        it = ready_.erase(it);
    }

    // s-rta-0928b seqvram: a returned frame index that differs from the previous one is a frame shown.
    auto shown = [&](int f) {
        if (f != lastReturned_ && grant.stats != nullptr)
            grant.stats->framesShown.fetch_add(1, std::memory_order_relaxed);
        lastReturned_ = f;
    };
    const int idx = std::clamp(currentFrameIndex_, 0, n - 1);
    if (textures_[static_cast<size_t>(idx)] != 0)
    {
        lastShown_ = idx;
        requestAhead(decoder, idx);
        shown(idx);
        return textures_[static_cast<size_t>(idx)];
    }
    requestFrame(decoder, idx);
    requestAhead(decoder, idx);
    if (lastShown_ >= 0 && lastShown_ < n && textures_[static_cast<size_t>(lastShown_)] != 0)
    {
        if (grant.stats != nullptr)
            grant.stats->lateFrames.fetch_add(1, std::memory_order_relaxed);
        shown(lastShown_);
        return textures_[static_cast<size_t>(lastShown_)];   // late frame: the last one repeats
    }
    if (failed_[static_cast<size_t>(idx)] != 0)
        return 0;   // nothing shown and this frame is broken: no media
    if (pending != nullptr)
        *pending = true;
    if (grant.stats != nullptr)
        grant.stats->pendingFrames.fetch_add(1, std::memory_order_relaxed);
    return 0;
}

// s-rta-0928b seqvram: every resident frame's texture (one per frame index on this build).
size_t ImageSequence::residentBytes() const
{
    size_t bytes = 0;
    for (size_t i = 0; i < textures_.size() && i < textureWidths_.size() && i < textureHeights_.size(); ++i)
        if (textures_[i] != 0)
            bytes += static_cast<size_t>(textureWidths_[i]) * static_cast<size_t>(textureHeights_[i]) * 4u;
    return bytes;
}

int ImageSequence::residentSlots() const
{
    return static_cast<int>(std::count_if(textures_.begin(), textures_.end(), [](GLuint t) { return t != 0; }));
}

// The same rule as getCurrentTexture's *pending, read only. The per-frame vectors may still be empty (never drawn, or
// after releaseGL): then nothing is resident and nothing has failed, so a non-empty open sequence is pending.
bool ImageSequence::firstFramePending() const
{
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return false;
    const int n = static_cast<int>(files_.size());
    if (static_cast<int>(textures_.size()) != n || static_cast<int>(failed_.size()) != n)
        return true;
    const auto idx = static_cast<size_t>(std::clamp(currentFrameIndex_, 0, n - 1));
    if (textures_[idx] != 0)
        return false;
    if (lastShown_ >= 0 && lastShown_ < n && textures_[static_cast<size_t>(lastShown_)] != 0)
        return false;
    return failed_[idx] == 0;
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
    // s-rta-0928 R1.4: every frame is requested again on the next context (ensureFrameState re-sizes).
    requested_.clear();
    ready_.clear();
    outstanding_ = 0;
    lastShown_ = -1;
    lastReturned_ = -1;
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
