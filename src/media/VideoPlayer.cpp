// VideoPlayer.cpp — FFmpeg-based video decoder with GL texture upload.
//
// Decodes MP4/MOV/AVI/HAP video files frame-by-frame, converts to RGBA,
// and uploads to an OpenGL texture for compositing.

#include "VideoPlayer.h"
#include <cstring>
#include <algorithm>
#include <iostream>

// FFmpeg headers (C linkage)
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

using namespace juce::gl;

VideoPlayer::VideoPlayer()
{
}

VideoPlayer::~VideoPlayer()
{
    close();
    releaseGL();
}

bool VideoPlayer::open(const juce::File& file)
{
    std::lock_guard<std::mutex> lock(ffmpegMutex_);

    // Close any previously open file
    if (open_.load(std::memory_order_relaxed))
    {
        // Reset state without lock (we already hold it)
        open_.store(false, std::memory_order_relaxed);

        if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
        if (rgbaFrame_) { av_frame_free(&rgbaFrame_); rgbaFrame_ = nullptr; }
        if (decodedFrame_) { av_frame_free(&decodedFrame_); decodedFrame_ = nullptr; }
        if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
        if (codecCtx_) { avcodec_free_context(&codecCtx_); codecCtx_ = nullptr; }
        if (formatCtx_) { avformat_close_input(&formatCtx_); formatCtx_ = nullptr; }
        frameBuffer_.clear();
        frameReady_ = false;
    }

    auto path = file.getFullPathName().toStdString();

    // Open input
    if (avformat_open_input(&formatCtx_, path.c_str(), nullptr, nullptr) < 0)
    {
        std::cerr << "[VideoPlayer] Failed to open: " << path << std::endl;
        return false;
    }

    if (avformat_find_stream_info(formatCtx_, nullptr) < 0)
    {
        std::cerr << "[VideoPlayer] Failed to find stream info" << std::endl;
        avformat_close_input(&formatCtx_);
        return false;
    }

    // Find video stream
    videoStreamIndex_ = -1;
    for (unsigned int i = 0; i < formatCtx_->nb_streams; ++i)
    {
        if (formatCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex_ = static_cast<int>(i);
            break;
        }
    }

    if (videoStreamIndex_ < 0)
    {
        std::cerr << "[VideoPlayer] No video stream found" << std::endl;
        avformat_close_input(&formatCtx_);
        return false;
    }

    auto* stream = formatCtx_->streams[videoStreamIndex_];
    auto* codecpar = stream->codecpar;

    // Find decoder
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec)
    {
        std::cerr << "[VideoPlayer] Unsupported codec: " << avcodec_get_name(codecpar->codec_id) << std::endl;
        avformat_close_input(&formatCtx_);
        return false;
    }

    codecCtx_ = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx_, codecpar);

    // Enable multi-threaded decoding
    codecCtx_->thread_count = 2;

    if (avcodec_open2(codecCtx_, codec, nullptr) < 0)
    {
        std::cerr << "[VideoPlayer] Failed to open codec" << std::endl;
        avcodec_free_context(&codecCtx_);
        avformat_close_input(&formatCtx_);
        return false;
    }

    // Allocate frames and packet
    decodedFrame_ = av_frame_alloc();
    rgbaFrame_ = av_frame_alloc();
    packet_ = av_packet_alloc();

    // Extract video properties
    width_ = codecCtx_->width;
    height_ = codecCtx_->height;

    // Frame rate
    if (stream->avg_frame_rate.den > 0 && stream->avg_frame_rate.num > 0)
        frameRate_ = av_q2d(stream->avg_frame_rate);
    else if (stream->r_frame_rate.den > 0 && stream->r_frame_rate.num > 0)
        frameRate_ = av_q2d(stream->r_frame_rate);
    else
        frameRate_ = 30.0;

    // Time base
    timeBase_ = av_q2d(stream->time_base);

    // Duration
    if (stream->duration > 0)
        duration_ = static_cast<double>(stream->duration) * timeBase_;
    else if (formatCtx_->duration > 0)
        duration_ = static_cast<double>(formatCtx_->duration) / AV_TIME_BASE;
    else
        duration_ = 0.0;

    totalFrames_ = (duration_ > 0.0 && frameRate_ > 0.0)
        ? static_cast<int>(duration_ * frameRate_ + 0.5)
        : 0;

    // Check for alpha channel
    auto pixFmt = codecCtx_->pix_fmt;
    bool alpha = (pixFmt == AV_PIX_FMT_RGBA || pixFmt == AV_PIX_FMT_BGRA ||
                  pixFmt == AV_PIX_FMT_ARGB || pixFmt == AV_PIX_FMT_ABGR ||
                  pixFmt == AV_PIX_FMT_YUVA420P || pixFmt == AV_PIX_FMT_YUVA444P ||
                  pixFmt == AV_PIX_FMT_PAL8 ||
                  // HAP Alpha codec typically uses RGBA
                  codecpar->codec_id == AV_CODEC_ID_HAP);
    hasAlpha_.store(alpha, std::memory_order_relaxed);

    // Set up swscale for conversion to RGBA
    auto dstFmt = AV_PIX_FMT_RGBA;
    swsCtx_ = sws_getContext(width_, height_, codecCtx_->pix_fmt,
                              width_, height_, dstFmt,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx_)
    {
        std::cerr << "[VideoPlayer] Failed to create swscale context" << std::endl;
        close();
        return false;
    }

    // Allocate RGBA frame buffer
    int bufSize = av_image_get_buffer_size(dstFmt, width_, height_, 1);
    frameBuffer_.resize(static_cast<size_t>(bufSize));
    av_image_fill_arrays(rgbaFrame_->data, rgbaFrame_->linesize,
                         frameBuffer_.data(), dstFmt, width_, height_, 1);
    frameBufferWidth_ = width_;
    frameBufferHeight_ = height_;

    // Decode first frame
    currentTime_ = 0.0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
    decodeNextFrame();
    convertFrameToRGBA();
    frameReady_ = true;

    open_.store(true, std::memory_order_relaxed);
    playing_.store(true, std::memory_order_relaxed);

    std::cerr << "[VideoPlayer] Opened: " << path
              << " (" << width_ << "x" << height_
              << ", " << frameRate_ << " fps"
              << ", " << duration_ << "s"
              << ", codec=" << avcodec_get_name(codecpar->codec_id)
              << ", alpha=" << (alpha ? "yes" : "no") << ")" << std::endl;

    return true;
}

void VideoPlayer::close()
{
    std::lock_guard<std::mutex> lock(ffmpegMutex_);

    open_.store(false, std::memory_order_relaxed);

    if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
    if (rgbaFrame_) { av_frame_free(&rgbaFrame_); rgbaFrame_ = nullptr; }
    if (decodedFrame_) { av_frame_free(&decodedFrame_); decodedFrame_ = nullptr; }
    if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
    if (codecCtx_) { avcodec_free_context(&codecCtx_); codecCtx_ = nullptr; }
    if (formatCtx_) { avformat_close_input(&formatCtx_); formatCtx_ = nullptr; }

    frameBuffer_.clear();
    frameReady_ = false;
    width_ = 0;
    height_ = 0;
    duration_ = 0.0;
    totalFrames_ = 0;
    currentTime_ = 0.0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
}

void VideoPlayer::seekTo(double normalizedPosition)
{
    normalizedPosition = std::clamp(normalizedPosition, 0.0, 1.0);
    seekTarget_.store(normalizedPosition, std::memory_order_relaxed);
    seekRequested_.store(true, std::memory_order_release);
}

void VideoPlayer::advanceFrame(double dt)
{
    if (!open_.load(std::memory_order_relaxed))
        return;

    // Handle seek request
    if (seekRequested_.load(std::memory_order_acquire))
    {
        seekRequested_.store(false, std::memory_order_relaxed);
        double target = seekTarget_.load(std::memory_order_relaxed);
        currentTime_ = target * duration_;
        seekToTimestamp(currentTime_);
        if (decodeNextFrame())
            convertFrameToRGBA();
        frameReady_ = true;
        playheadPosition_.store(target, std::memory_order_relaxed);
        return;
    }

    if (!playing_.load(std::memory_order_relaxed))
        return;

    if (duration_ <= 0.0)
        return;

    float speed = speed_.load(std::memory_order_relaxed);
    bool reverse = reverse_.load(std::memory_order_relaxed);
    auto loopMode = loopMode_.load(std::memory_order_relaxed);

    // Advance time
    double direction = (reverse != !pingPongForward_) ? -1.0 : 1.0;
    // In ping-pong, pingPongForward_ tracks the current direction
    if (loopMode != LoopMode::PingPong)
        direction = reverse ? -1.0 : 1.0;

    currentTime_ += dt * static_cast<double>(speed) * direction;

    // Handle boundaries
    if (currentTime_ >= duration_)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = std::fmod(currentTime_, duration_);
                seekToTimestamp(currentTime_);
                break;
            case LoopMode::PingPong:
                currentTime_ = duration_ - (currentTime_ - duration_);
                pingPongForward_ = false;
                break;
            case LoopMode::OneShot:
                currentTime_ = duration_;
                playing_.store(false, std::memory_order_relaxed);
                break;
        }
    }
    else if (currentTime_ < 0.0)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = duration_ + std::fmod(currentTime_, duration_);
                seekToTimestamp(currentTime_);
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

    // Update playhead position
    double pos = (duration_ > 0.0) ? (currentTime_ / duration_) : 0.0;
    playheadPosition_.store(std::clamp(pos, 0.0, 1.0), std::memory_order_relaxed);

    // Decode frame at current time
    if (decodeFrameAtTime(currentTime_))
    {
        convertFrameToRGBA();
        frameReady_ = true;
    }
}

GLuint VideoPlayer::uploadToTexture()
{
    if (!open_.load(std::memory_order_relaxed) || !frameReady_)
        return texture_;

    if (frameBuffer_.empty() || frameBufferWidth_ <= 0 || frameBufferHeight_ <= 0)
        return 0;

    if (!textureCreated_)
    {
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     frameBufferWidth_, frameBufferHeight_,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer_.data());
        textureCreated_ = true;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        frameBufferWidth_, frameBufferHeight_,
                        GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer_.data());
    }

    return texture_;
}

void VideoPlayer::releaseGL()
{
    if (texture_ != 0)
    {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
    textureCreated_ = false;
}

juce::Image VideoPlayer::getThumbnail(int maxWidth, int maxHeight)
{
    if (!open_.load(std::memory_order_relaxed) || frameBuffer_.empty())
        return {};

    // Create JUCE Image from the current RGBA frame buffer
    juce::Image img(juce::Image::ARGB, frameBufferWidth_, frameBufferHeight_, false);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);

    for (int y = 0; y < frameBufferHeight_; ++y)
    {
        for (int x = 0; x < frameBufferWidth_; ++x)
        {
            // frameBuffer_ is RGBA, top-to-bottom (already flipped for GL in convertFrameToRGBA)
            // For thumbnail we want top-to-bottom (normal image orientation)
            // The frame buffer is stored bottom-to-top for GL, so flip y back
            int srcY = frameBufferHeight_ - 1 - y;
            size_t idx = static_cast<size_t>((srcY * frameBufferWidth_ + x) * 4);
            bmp.setPixelColour(x, y, juce::Colour(
                frameBuffer_[idx], frameBuffer_[idx + 1],
                frameBuffer_[idx + 2], frameBuffer_[idx + 3]));
        }
    }

    // Scale to thumbnail size
    float scaleX = static_cast<float>(maxWidth) / static_cast<float>(frameBufferWidth_);
    float scaleY = static_cast<float>(maxHeight) / static_cast<float>(frameBufferHeight_);
    float scale = std::min(scaleX, scaleY);
    int thumbW = static_cast<int>(static_cast<float>(frameBufferWidth_) * scale);
    int thumbH = static_cast<int>(static_cast<float>(frameBufferHeight_) * scale);

    return img.rescaled(std::max(1, thumbW), std::max(1, thumbH),
                        juce::Graphics::lowResamplingQuality);
}

// === Private implementation ===

bool VideoPlayer::decodeFrameAtTime(double timeSec)
{
    if (!formatCtx_ || !codecCtx_)
        return false;

    // Calculate target PTS
    auto* stream = formatCtx_->streams[videoStreamIndex_];
    int64_t targetPts = static_cast<int64_t>(timeSec / timeBase_);

    // If the decoded frame is close enough, skip decode
    if (decodedFrame_->pts >= 0)
    {
        double frameDuration = 1.0 / frameRate_;
        double framePtsTime = static_cast<double>(decodedFrame_->pts) * timeBase_;
        if (std::abs(framePtsTime - timeSec) < frameDuration * 0.5)
            return false; // Current frame is still valid
    }

    // Check if we need to seek (going backwards or jumping far ahead)
    double framePtsTime = (decodedFrame_->pts >= 0)
        ? static_cast<double>(decodedFrame_->pts) * timeBase_
        : -1.0;
    double diff = timeSec - framePtsTime;

    if (diff < -0.1 || diff > 2.0)
    {
        // Need to seek
        seekToTimestamp(timeSec);
    }

    // Decode frames until we reach or pass the target time
    int maxAttempts = 30; // Don't decode too many frames per render
    while (maxAttempts-- > 0)
    {
        if (!decodeNextFrame())
        {
            // End of stream — wrap for looping
            return false;
        }

        if (decodedFrame_->pts >= 0)
        {
            double decodedTime = static_cast<double>(decodedFrame_->pts) * timeBase_;
            if (decodedTime >= timeSec - (1.0 / frameRate_) * 0.5)
                return true; // Got a frame at or past target time
        }
        else
        {
            return true; // No PTS info — use whatever we got
        }
    }

    return true; // Used up attempts, return what we have
}

bool VideoPlayer::seekToTimestamp(double timeSec)
{
    if (!formatCtx_)
        return false;

    auto* stream = formatCtx_->streams[videoStreamIndex_];
    int64_t timestamp = static_cast<int64_t>(timeSec / timeBase_);

    int ret = av_seek_frame(formatCtx_, videoStreamIndex_, timestamp,
                            AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
    {
        // Try seeking from the beginning
        ret = av_seek_frame(formatCtx_, videoStreamIndex_, 0, AVSEEK_FLAG_BACKWARD);
    }

    if (codecCtx_)
        avcodec_flush_buffers(codecCtx_);

    return ret >= 0;
}

bool VideoPlayer::decodeNextFrame()
{
    if (!formatCtx_ || !codecCtx_ || !decodedFrame_ || !packet_)
        return false;

    while (true)
    {
        int ret = av_read_frame(formatCtx_, packet_);
        if (ret < 0)
        {
            // End of file or error
            av_packet_unref(packet_);
            return false;
        }

        if (packet_->stream_index != videoStreamIndex_)
        {
            av_packet_unref(packet_);
            continue;
        }

        ret = avcodec_send_packet(codecCtx_, packet_);
        av_packet_unref(packet_);

        if (ret < 0)
            continue;

        ret = avcodec_receive_frame(codecCtx_, decodedFrame_);
        if (ret == 0)
            return true;  // Got a frame
        if (ret == AVERROR(EAGAIN))
            continue;      // Need more packets
        // Other error
        return false;
    }
}

void VideoPlayer::convertFrameToRGBA()
{
    if (!decodedFrame_ || !rgbaFrame_ || !swsCtx_)
        return;

    // Convert to RGBA
    sws_scale(swsCtx_,
              decodedFrame_->data, decodedFrame_->linesize,
              0, height_,
              rgbaFrame_->data, rgbaFrame_->linesize);

    // Flip vertically for OpenGL (bottom-to-top)
    // rgbaFrame_ data is top-to-bottom, we need bottom-to-top
    int rowBytes = width_ * 4;
    std::vector<uint8_t> tempRow(static_cast<size_t>(rowBytes));

    uint8_t* data = frameBuffer_.data();
    for (int y = 0; y < height_ / 2; ++y)
    {
        uint8_t* top = data + static_cast<size_t>(y * rowBytes);
        uint8_t* bot = data + static_cast<size_t>((height_ - 1 - y) * rowBytes);
        std::memcpy(tempRow.data(), top, static_cast<size_t>(rowBytes));
        std::memcpy(top, bot, static_cast<size_t>(rowBytes));
        std::memcpy(bot, tempRow.data(), static_cast<size_t>(rowBytes));
    }
}
