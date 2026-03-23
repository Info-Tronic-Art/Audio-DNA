#include "recording/VideoRecorder.h"
#include <juce_opengl/juce_opengl.h>
#include <iostream>

using namespace juce::gl;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoRecorder::VideoRecorder()
{
    // Pre-allocate pixel buffers (resized on first frame)
    for (auto& buf : pixelBuffers_)
        buf.ready.store(false, std::memory_order_relaxed);
}

VideoRecorder::~VideoRecorder()
{
    stopRecording();
}

bool VideoRecorder::startRecording(const juce::File& outputFile, const Config& config)
{
    if (state_.load(std::memory_order_relaxed) != State::Idle)
    {
        std::cerr << "[VideoRecorder] Already recording" << std::endl;
        return false;
    }

    config_ = config;
    outputFile_ = outputFile;
    framesWritten_.store(0, std::memory_order_relaxed);
    droppedFrames_.store(0, std::memory_order_relaxed);
    pts_ = 0;

    // Reset buffer states
    writeIndex_.store(0, std::memory_order_relaxed);
    readIndex_.store(0, std::memory_order_relaxed);
    for (auto& buf : pixelBuffers_)
        buf.ready.store(false, std::memory_order_relaxed);

    if (!initEncoder())
    {
        std::cerr << "[VideoRecorder] Failed to initialize encoder" << std::endl;
        closeEncoder();
        return false;
    }

    state_.store(State::Recording, std::memory_order_release);

    // Start encoder thread
    encoderThread_ = std::thread(&VideoRecorder::encoderThreadFunc, this);

    std::cerr << "[VideoRecorder] Recording started: " << outputFile.getFullPathName()
              << " (" << config.width << "x" << config.height << " @ " << config.fps << "fps)" << std::endl;

    return true;
}

void VideoRecorder::stopRecording()
{
    if (state_.load(std::memory_order_relaxed) != State::Recording)
        return;

    state_.store(State::Stopping, std::memory_order_release);

    // Wake up encoder thread
    encoderCV_.notify_one();

    if (encoderThread_.joinable())
        encoderThread_.join();

    // Flush remaining frames
    flushEncoder();
    closeEncoder();

    state_.store(State::Idle, std::memory_order_release);

    std::cerr << "[VideoRecorder] Recording stopped. Frames: " << framesWritten_.load()
              << ", Dropped: " << droppedFrames_.load() << std::endl;

    if (onRecordingFinished)
    {
        auto file = outputFile_;
        auto cb = onRecordingFinished;
        juce::MessageManager::callAsync([cb, file]() { cb(true, file); });
    }
}

double VideoRecorder::getRecordedDuration() const
{
    if (config_.fps <= 0) return 0.0;
    return static_cast<double>(framesWritten_.load(std::memory_order_relaxed)) / config_.fps;
}

void VideoRecorder::submitFrame(int framebufferWidth, int framebufferHeight)
{
    if (state_.load(std::memory_order_acquire) != State::Recording)
        return;

    int wi = writeIndex_.load(std::memory_order_relaxed);
    auto& buf = pixelBuffers_[static_cast<size_t>(wi)];

    // If this buffer is still being read by the encoder, drop the frame
    if (buf.ready.load(std::memory_order_acquire))
    {
        droppedFrames_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    // Use configured resolution, or framebuffer size if smaller
    int captureW = std::min(config_.width, framebufferWidth);
    int captureH = std::min(config_.height, framebufferHeight);

    // Resize buffer if needed
    size_t requiredSize = static_cast<size_t>(captureW) * static_cast<size_t>(captureH) * 4;
    if (buf.data.size() != requiredSize)
        buf.data.resize(requiredSize);

    buf.width = captureW;
    buf.height = captureH;

    // Read pixels from GL framebuffer (RGBA)
    glReadPixels(0, 0, captureW, captureH, GL_RGBA, GL_UNSIGNED_BYTE, buf.data.data());

    // Mark buffer as ready for encoding
    buf.ready.store(true, std::memory_order_release);

    // Advance write index (triple buffer)
    writeIndex_.store((wi + 1) % kNumBuffers, std::memory_order_relaxed);

    // Wake encoder thread
    encoderCV_.notify_one();
}

void VideoRecorder::encoderThreadFunc()
{
    while (state_.load(std::memory_order_acquire) == State::Recording)
    {
        int ri = readIndex_.load(std::memory_order_relaxed);
        auto& buf = pixelBuffers_[static_cast<size_t>(ri)];

        if (buf.ready.load(std::memory_order_acquire))
        {
            // Encode this frame
            if (encodeFrame(buf.data.data(), buf.width, buf.height))
                framesWritten_.fetch_add(1, std::memory_order_relaxed);

            buf.ready.store(false, std::memory_order_release);
            readIndex_.store((ri + 1) % kNumBuffers, std::memory_order_relaxed);
        }
        else
        {
            // Wait for a frame to become ready
            std::unique_lock<std::mutex> lock(encoderMutex_);
            encoderCV_.wait_for(lock, std::chrono::milliseconds(10));
        }
    }

    // Drain remaining buffers
    for (int i = 0; i < kNumBuffers; ++i)
    {
        int ri = readIndex_.load(std::memory_order_relaxed);
        auto& buf = pixelBuffers_[static_cast<size_t>(ri)];
        if (buf.ready.load(std::memory_order_acquire))
        {
            encodeFrame(buf.data.data(), buf.width, buf.height);
            buf.ready.store(false, std::memory_order_release);
            readIndex_.store((ri + 1) % kNumBuffers, std::memory_order_relaxed);
            framesWritten_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

// --- FFmpeg encoder setup ---

bool VideoRecorder::initEncoder()
{
    // Determine container format
    std::string containerFormat;
    if (!config_.container.empty())
        containerFormat = config_.container;
    else
        containerFormat = (config_.codec == Codec::ProRes) ? "mov" : "mp4";

    // Allocate output context
    int ret = avformat_alloc_output_context2(&formatCtx_, nullptr,
                                              containerFormat.c_str(),
                                              outputFile_.getFullPathName().toRawUTF8());
    if (ret < 0 || !formatCtx_)
    {
        std::cerr << "[VideoRecorder] Failed to allocate output context" << std::endl;
        return false;
    }

    // Find encoder
    const char* codecName = nullptr;
    switch (config_.codec)
    {
        case Codec::H264:   codecName = "libx264"; break;
        case Codec::ProRes: codecName = "prores_ks"; break;
        case Codec::MJPEG:  codecName = "mjpeg"; break;
    }

    const AVCodec* codec = avcodec_find_encoder_by_name(codecName);
    if (!codec)
    {
        // Fallback to default encoder for the format
        codec = avcodec_find_encoder(formatCtx_->oformat->video_codec);
        if (!codec)
        {
            std::cerr << "[VideoRecorder] No encoder found for " << codecName << std::endl;
            return false;
        }
    }

    // Create stream
    videoStream_ = avformat_new_stream(formatCtx_, nullptr);
    if (!videoStream_)
    {
        std::cerr << "[VideoRecorder] Failed to create video stream" << std::endl;
        return false;
    }

    // Allocate codec context
    codecCtx_ = avcodec_alloc_context3(codec);
    if (!codecCtx_)
    {
        std::cerr << "[VideoRecorder] Failed to allocate codec context" << std::endl;
        return false;
    }

    codecCtx_->width = config_.width;
    codecCtx_->height = config_.height;
    codecCtx_->time_base = {1, config_.fps};
    codecCtx_->framerate = {config_.fps, 1};

    if (config_.codec == Codec::ProRes)
    {
        codecCtx_->pix_fmt = AV_PIX_FMT_YUV422P10LE;
        av_opt_set_int(codecCtx_->priv_data, "profile", config_.quality, 0); // 0-5
    }
    else if (config_.codec == Codec::MJPEG)
    {
        codecCtx_->pix_fmt = AV_PIX_FMT_YUVJ420P;
        codecCtx_->qmin = 2;
        codecCtx_->qmax = 31;
    }
    else // H264
    {
        codecCtx_->pix_fmt = AV_PIX_FMT_YUV420P;
        av_opt_set(codecCtx_->priv_data, "preset", "ultrafast", 0);
        av_opt_set(codecCtx_->priv_data, "tune", "zerolatency", 0);
        char crf[8];
        snprintf(crf, sizeof(crf), "%d", config_.quality);
        av_opt_set(codecCtx_->priv_data, "crf", crf, 0);
    }

    if (formatCtx_->oformat->flags & AVFMT_GLOBALHEADER)
        codecCtx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Open codec
    ret = avcodec_open2(codecCtx_, codec, nullptr);
    if (ret < 0)
    {
        std::cerr << "[VideoRecorder] Failed to open codec: " << ret << std::endl;
        return false;
    }

    // Copy codec params to stream
    ret = avcodec_parameters_from_context(videoStream_->codecpar, codecCtx_);
    if (ret < 0)
    {
        std::cerr << "[VideoRecorder] Failed to copy codec params" << std::endl;
        return false;
    }

    videoStream_->time_base = codecCtx_->time_base;

    // Open output file
    if (!(formatCtx_->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&formatCtx_->pb,
                        outputFile_.getFullPathName().toRawUTF8(),
                        AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            std::cerr << "[VideoRecorder] Failed to open output file" << std::endl;
            return false;
        }
    }

    // Write header
    ret = avformat_write_header(formatCtx_, nullptr);
    if (ret < 0)
    {
        std::cerr << "[VideoRecorder] Failed to write header" << std::endl;
        return false;
    }

    // Allocate frames
    frame_ = av_frame_alloc();
    frame_->format = codecCtx_->pix_fmt;
    frame_->width = config_.width;
    frame_->height = config_.height;
    av_frame_get_buffer(frame_, 0);

    // Allocate packet
    packet_ = av_packet_alloc();

    // Setup color space converter (RGBA → YUV)
    swsCtx_ = sws_getContext(config_.width, config_.height, AV_PIX_FMT_RGBA,
                              config_.width, config_.height, codecCtx_->pix_fmt,
                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx_)
    {
        std::cerr << "[VideoRecorder] Failed to create sws context" << std::endl;
        return false;
    }

    return true;
}

void VideoRecorder::closeEncoder()
{
    if (formatCtx_)
    {
        if (formatCtx_->pb)
        {
            av_write_trailer(formatCtx_);
            if (!(formatCtx_->oformat->flags & AVFMT_NOFILE))
                avio_closep(&formatCtx_->pb);
        }
        avformat_free_context(formatCtx_);
        formatCtx_ = nullptr;
    }

    if (codecCtx_)
    {
        avcodec_free_context(&codecCtx_);
        codecCtx_ = nullptr;
    }

    if (frame_)
    {
        av_frame_free(&frame_);
        frame_ = nullptr;
    }

    if (packet_)
    {
        av_packet_free(&packet_);
        packet_ = nullptr;
    }

    if (swsCtx_)
    {
        sws_freeContext(swsCtx_);
        swsCtx_ = nullptr;
    }

    videoStream_ = nullptr;
}

bool VideoRecorder::encodeFrame(const uint8_t* rgbaData, int width, int height)
{
    if (!codecCtx_ || !frame_ || !swsCtx_ || !formatCtx_)
        return false;

    // GL returns bottom-to-top, FFmpeg expects top-to-bottom.
    // Flip vertically by using negative stride in sws_scale.
    const uint8_t* srcSlice[1] = { rgbaData + static_cast<size_t>(width) * (static_cast<size_t>(height) - 1) * 4 };
    int srcStride[1] = { -(width * 4) };  // Negative stride = vertical flip

    av_frame_make_writable(frame_);
    sws_scale(swsCtx_, srcSlice, srcStride, 0, height,
              frame_->data, frame_->linesize);

    frame_->pts = pts_++;

    // Send frame to encoder
    int ret = avcodec_send_frame(codecCtx_, frame_);
    if (ret < 0)
    {
        std::cerr << "[VideoRecorder] Error sending frame: " << ret << std::endl;
        return false;
    }

    // Receive and write encoded packets
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCtx_, packet_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
        {
            std::cerr << "[VideoRecorder] Error receiving packet: " << ret << std::endl;
            return false;
        }

        av_packet_rescale_ts(packet_, codecCtx_->time_base, videoStream_->time_base);
        packet_->stream_index = videoStream_->index;

        ret = av_interleaved_write_frame(formatCtx_, packet_);
        av_packet_unref(packet_);
        if (ret < 0)
        {
            std::cerr << "[VideoRecorder] Error writing packet: " << ret << std::endl;
            return false;
        }
    }

    return true;
}

bool VideoRecorder::flushEncoder()
{
    if (!codecCtx_ || !formatCtx_)
        return false;

    // Send null frame to flush
    avcodec_send_frame(codecCtx_, nullptr);

    int ret = 0;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecCtx_, packet_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            return false;

        av_packet_rescale_ts(packet_, codecCtx_->time_base, videoStream_->time_base);
        packet_->stream_index = videoStream_->index;

        av_interleaved_write_frame(formatCtx_, packet_);
        av_packet_unref(packet_);
    }

    return true;
}
