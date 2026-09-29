// VideoPlayer.cpp — FFmpeg-based video decoder with GL texture upload.
//
// Decodes MP4/MOV/AVI/HAP video files frame-by-frame on a per-player decode thread, converts each frame to RGBA
// (bottom-up) into a 3-slot lock-free ring, and uploads the newest frame <= the clock to an OpenGL texture for
// compositing on the GL thread (s-rta-0928b video; plan-video.md).

#include "VideoPlayer.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

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
    // Normally reached with the decode thread already exited (drainRetiredMedia's gate); at shutdown (~Renderer)
    // the join waits for at most one read + decode.
    close();
    thread_.stopThread(3000);
    freeFfmpeg();
    for (auto*& s : slotBytes_)
    {
        std::free(s);
        s = nullptr;
    }
    releaseGL();
}

int64_t VideoPlayer::nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool VideoPlayer::open(const juce::File& file)
{
    // Every open is a new player (Renderer::openVideoForClip); a second open would race the decode thread.
    if (open_.load(std::memory_order_relaxed) || thread_.isThreadRunning())
        return false;

    sourceFile_ = file;
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

    // Allocate frame and packet
    decodedFrame_ = av_frame_alloc();
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
    frameDur_ = 1.0 / frameRate_;

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
        freeFfmpeg();
        return false;
    }

    // The ring's slots: width * height * 4 bytes each, page-lazy (RSS grows when a slot is first written).
    rowBytes_ = width_ * 4;
    const size_t slotSize = static_cast<size_t>(rowBytes_) * static_cast<size_t>(height_);
    for (auto*& s : slotBytes_)
    {
        s = static_cast<uint8_t*>(std::malloc(slotSize));
        if (s == nullptr)
        {
            std::cerr << "[VideoPlayer] Failed to allocate a frame slot" << std::endl;
            freeFfmpeg();
            return false;
        }
    }

    // Decode the first frame into slot 0 (gen 0) and make the thumbnail from it: a fresh trigger without a seek is
    // never pending. Its pts is clamped to <= 0 so it is current from clock 0 (a stream whose first pts is a frame
    // or two late would otherwise wait for its own clock). A failed first decode publishes nothing: the player is
    // pending until the decode thread lands a frame.
    currentTime_ = 0.0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
    double thumbMs = 0.0;
    if (decodeNextFrame())
    {
        const double pts0 = decodedFrame_->pts >= 0 ? static_cast<double>(decodedFrame_->pts) * timeBase_ : 0.0;
        const int s = ring_.acquireWrite();
        convertInto(s);
        ring_.publish(s, std::min(pts0, 0.0), 0, ++seq_);
        newestPts_ = lastDecodedPts_ = pts0;
        haveNewest_ = haveDecoded_ = true;
        const auto t0 = std::chrono::steady_clock::now();
        makeThumbnail();
        thumbMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    }

    open_.store(true, std::memory_order_relaxed);
    playing_.store(true, std::memory_order_relaxed);

    std::cerr << "[VideoPlayer] Opened: " << path
              << " (" << width_ << "x" << height_
              << ", " << frameRate_ << " fps"
              << ", " << duration_ << "s"
              << ", codec=" << avcodec_get_name(codecpar->codec_id)
              << ", alpha=" << (alpha ? "yes" : "no")
              << ", thumb=" << thumbMs << " ms)" << std::endl;

    return true;
}

void VideoPlayer::start()
{
    if (!open_.load(std::memory_order_relaxed) || thread_.isThreadRunning())
        return;
    if (stats_) ++stats_->threadsRunning;
    if (!thread_.startThread(kDecodeThreadPriority))
    {
        if (stats_) --stats_->threadsRunning;
        std::cerr << "[VideoPlayer] Failed to start the decode thread: " << sourceFile_.getFullPathName()
                  << " (the player shows its first frame only)" << std::endl;
    }
}

void VideoPlayer::close()
{
    open_.store(false, std::memory_order_relaxed);
    if (thread_.isThreadRunning())
    {
        thread_.signalThreadShouldExit();
        thread_.notify();
    }
    else
        freeFfmpeg();   // never started (or already exited: then a no-op)
}

void VideoPlayer::freeFfmpeg()
{
    if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
    if (decodedFrame_) { av_frame_free(&decodedFrame_); decodedFrame_ = nullptr; }
    if (packet_) { av_packet_free(&packet_); packet_ = nullptr; }
    if (codecCtx_) { avcodec_free_context(&codecCtx_); codecCtx_ = nullptr; }
    if (formatCtx_) { avformat_close_input(&formatCtx_); formatCtx_ = nullptr; }
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

    // A seek request lands on the clock (the decode thread serves it: the generation bump below).
    bool jumped = false;
    if (seekRequested_.load(std::memory_order_acquire))
    {
        seekRequested_.store(false, std::memory_order_relaxed);
        double target = seekTarget_.load(std::memory_order_relaxed);
        currentTime_ = target * duration_;
        playheadPosition_.store(target, std::memory_order_relaxed);
        jumped = true;
    }
    else
        advanceTransport(dt);

    // The wanted time BEFORE the generation (release): a decode thread that sees the new generation sees its time.
    wantTime_.store(currentTime_, std::memory_order_release);
    const bool genChanged = jumped || discontinuity_;
    discontinuity_ = false;
    if (genChanged)
        gen_.fetch_add(1, std::memory_order_acq_rel);

    // The draw stamp keeps the thread awake; wake it when it has work: a slot came free, the request changed, or it
    // may be parked (no draw for > 100 ms).
    const int64_t now = nowMs();
    const bool wake = releasedThisFrame_ || genChanged || now - lastDrawMs_.load(std::memory_order_relaxed) > 100;
    lastDrawMs_.store(now, std::memory_order_release);
    releasedThisFrame_ = false;
    if (wake)
        thread_.notify();
}

void VideoPlayer::advanceClock(double dt)
{
    if (!open_.load(std::memory_order_relaxed))
        return;

    // A pending seek lands on the clock only; the ring's frames are stale (generation bump). The decode thread is
    // not woken: the next advanceFrame() does it, and the thread then re-seeks and catches up.
    if (seekRequested_.load(std::memory_order_acquire))
    {
        seekRequested_.store(false, std::memory_order_relaxed);
        double target = seekTarget_.load(std::memory_order_relaxed);
        currentTime_ = target * duration_;
        playheadPosition_.store(target, std::memory_order_relaxed);
        gen_.fetch_add(1, std::memory_order_acq_rel);
        return;
    }

    advanceTransport(dt);
    if (discontinuity_)
    {
        discontinuity_ = false;
        gen_.fetch_add(1, std::memory_order_acq_rel);
    }
}

bool VideoPlayer::advanceTransport(double dt)
{
    if (!playing_.load(std::memory_order_relaxed))
        return false;

    if (duration_ <= 0.0)
        return false;

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
                discontinuity_ = true;   // s-rta-0928b: the decode thread re-seeks (was seekToTimestamp here)
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
                discontinuity_ = true;   // s-rta-0928b: the decode thread re-seeks (was seekToTimestamp here)
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
    return true;
}

GLuint VideoPlayer::uploadToTexture(bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    if (!open_.load(std::memory_order_relaxed))
        return texture_;   // hold after close, as before

    // Frames the clock moved away from (reverse / ping-pong) are freed once they are more than a ring's worth of
    // frames ahead: forward play never gets that far ahead, and they would otherwise keep the writer out.
    const auto p = ring_.pick(currentTime_, gen_.load(std::memory_order_acquire), 0.5 * frameDur_,
                              (kSlots + 1) * frameDur_);
    if (stats_ && p.skipped > 0)
        stats_->framesSkipped += p.skipped;

    if (p.slot >= 0)
    {
        if (shown_.needsUpload(p.seq))
        {
            const auto uploadStart = std::chrono::steady_clock::now();
            const uint8_t* bytes = slotBytes_[static_cast<size_t>(p.slot)];
            if (!textureCreated_)
            {
                glGenTextures(1, &texture_);
                glBindTexture(GL_TEXTURE_2D, texture_);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                             width_, height_,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
                textureCreated_ = true;
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, texture_);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                                width_, height_,
                                GL_RGBA, GL_UNSIGNED_BYTE, bytes);
            }
            shown_.onUpload(p.seq);
            if (stats_)
            {
                ++stats_->uploads;
                VideoStats::noteMax(stats_->peakUploadMs, std::chrono::duration<float, std::milli>(
                                                              std::chrono::steady_clock::now() - uploadStart).count());
            }
        }
        lastShownPts_ = p.pts;
        ring_.release(p.slot);   // client-memory glTex*Image2D copies before it returns: the slot is free now
        releasedThisFrame_ = true;
        return texture_;
    }

    switch (VideoRing::judge(false, shown_.everShown, playing_.load(std::memory_order_relaxed), currentTime_,
                             lastShownPts_, frameDur_))
    {
        case VideoRing::Shown::Late:
            if (stats_) { ++stats_->holdFrames; ++stats_->lateFrames; }
            break;
        case VideoRing::Shown::Pending:
            if (stats_) { ++stats_->pendingFrames; ++stats_->pendingNow; }
            if (pending != nullptr)
                *pending = true;
            return 0;
        case VideoRing::Shown::Held:
        case VideoRing::Shown::New:
            if (stats_) ++stats_->holdFrames;
            break;
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
    shown_.onReleaseGL();   // the next pick re-uploads; everShown stays (V1)
    // A paused clip would never pick a new frame: ask the decode thread for the current frame again.
    if (open_.load(std::memory_order_relaxed) && !playing_.load(std::memory_order_relaxed))
    {
        gen_.fetch_add(1, std::memory_order_acq_rel);
        thread_.notify();
    }
}

juce::Image VideoPlayer::getThumbnail(int maxWidth, int maxHeight)
{
    if (!thumbnail_.isValid() || width_ <= 0 || height_ <= 0)
        return {};
    const float scale = std::min(static_cast<float>(maxWidth) / static_cast<float>(width_),
                                 static_cast<float>(maxHeight) / static_cast<float>(height_));
    const int thumbW = std::max(1, static_cast<int>(static_cast<float>(width_) * scale));
    const int thumbH = std::max(1, static_cast<int>(static_cast<float>(height_) * scale));
    if (thumbW == thumbnail_.getWidth() && thumbH == thumbnail_.getHeight())
        return thumbnail_;
    return thumbnail_.rescaled(thumbW, thumbH, juce::Graphics::lowResamplingQuality);
}

// === Private implementation ===

void VideoPlayer::makeThumbnail()
{
    // R-16: the first decoded frame straight to <= 90 x 72 RGBA (SWS_AREA), top-down -- the size every caller asks
    // (getThumbnail(90, 72)); the old path converted the full frame, flipped it and rescaled it on the message thread.
    const float scale = std::min(90.0f / static_cast<float>(width_), 72.0f / static_cast<float>(height_));
    const int tw = std::max(1, static_cast<int>(static_cast<float>(width_) * scale));
    const int th = std::max(1, static_cast<int>(static_cast<float>(height_) * scale));
    SwsContext* small = sws_getContext(width_, height_, codecCtx_->pix_fmt, tw, th, AV_PIX_FMT_RGBA,
                                       SWS_AREA, nullptr, nullptr, nullptr);
    if (small == nullptr)
        return;
    std::vector<uint8_t> rgba(static_cast<size_t>(tw) * static_cast<size_t>(th) * 4);
    uint8_t* dst[4] = { rgba.data(), nullptr, nullptr, nullptr };
    int dstStride[4] = { tw * 4, 0, 0, 0 };
    sws_scale(small, decodedFrame_->data, decodedFrame_->linesize, 0, height_, dst, dstStride);
    sws_freeContext(small);

    juce::Image img(juce::Image::ARGB, tw, th, false);
    juce::Image::BitmapData bmp(img, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < th; ++y)
        for (int x = 0; x < tw; ++x)
        {
            const size_t i = (static_cast<size_t>(y) * static_cast<size_t>(tw) + static_cast<size_t>(x)) * 4;
            bmp.setPixelColour(x, y, juce::Colour(rgba[i], rgba[i + 1], rgba[i + 2], rgba[i + 3]));
        }
    thumbnail_ = img;
}

void VideoPlayer::park()
{
    if (stats_) --stats_->threadsAwake;
    thread_.wait(-1);
    if (stats_) ++stats_->threadsAwake;
}

void VideoPlayer::decodeLoop()
{
    // plan-video R-5: today's GL-thread rules, moved here and made non-blocking for the GL thread.
    VideoRing::Policy pol;
    pol.skipNonRefInCatchUp = kSkipNonRefInCatchUp;
    // The writer runs up to kSlots frames ahead of the clock (the old decode ran at most one): "behind" must exceed
    // that look-ahead, or forward play of a < 30 fps clip (3 frames > 0.1 s) would re-seek after every third frame.
    pol.reseekBehindSec = std::max(pol.reseekBehindSec, (kSlots + 1) * frameDur_);
    if (stats_) ++stats_->threadsAwake;

    while (!thread_.threadShouldExit())
    {
        // Rule 15: a player that is not drawn (its deck off screen) decodes nothing.
        if (VideoRing::idleStep(nowMs(), lastDrawMs_.load(std::memory_order_acquire), pol) == VideoRing::Idle::Park)
        {
            park();
            continue;
        }

        const uint32_t g = gen_.load(std::memory_order_acquire);
        const double want = wantTime_.load(std::memory_order_acquire);
        if (g != myGen_ || VideoRing::decide(want, newestPts_, haveNewest_, pol) == VideoRing::Step::Reseek)
        {
            myGen_ = g;
            seekToTimestamp(want);   // the keyframe at or before want; the catch-up follows
            haveNewest_ = haveDecoded_ = false;
        }

        codecCtx_->skip_frame = (haveDecoded_ && VideoRing::useSkipNonRef(lastDecodedPts_, want, frameDur_, pol))
                                    ? AVDISCARD_NONREF
                                    : AVDISCARD_DEFAULT;

        if (!decodeNextFrame())
        {
            if (!atEof_)
                continue;               // a decode error: the next packet
            if (!drained_)
            {
                drainDecoder(g, pol);   // EOF: the last frames frame-threading held back now show
                continue;
            }
            thread_.wait(20);           // EOF: the Loop wrap's generation bump (or a seek) re-seeks
            continue;
        }
        onDecoded(g, pol);
    }

    freeFfmpeg();   // the thread owns the contexts: nobody else touches them after start()
    if (stats_) { --stats_->threadsAwake; --stats_->threadsRunning; }
}

void VideoPlayer::onDecoded(uint32_t gen, const VideoRing::Policy& pol)
{
    // No pts -> take it (the old decodeFrameAtTime's "use whatever we got").
    const double pts = decodedFrame_->pts >= 0 ? static_cast<double>(decodedFrame_->pts) * timeBase_
                                               : wantTime_.load(std::memory_order_acquire);
    lastDecodedPts_ = pts;
    haveDecoded_ = true;
    if (stats_) ++stats_->framesDecoded;

    // A catch-up chases the moving clock: frames behind it are dropped without a conversion.
    if (!VideoRing::shouldPublish(pts, wantTime_.load(std::memory_order_acquire), frameDur_, pol))
    {
        if (stats_) ++stats_->framesDropped;
        return;
    }

    int s;
    while ((s = ring_.acquireWrite()) < 0)   // the ring is full: the reader frees a slot and notifies
    {
        if (thread_.threadShouldExit() || gen_.load(std::memory_order_acquire) != gen)
            return;   // exiting, or a seek arrived: this frame is stale
        if (VideoRing::idleStep(nowMs(), lastDrawMs_.load(std::memory_order_acquire), pol) == VideoRing::Idle::Park)
        {
            park();   // V2: off screen while the ring is full -- park, and drop this frame (the clock moved on)
            return;
        }
        if (VideoRing::decide(wantTime_.load(std::memory_order_acquire), newestPts_, haveNewest_, pol)
            == VideoRing::Step::Reseek)
            return;   // the clock moved away (reverse play): the top of the loop re-seeks
        thread_.wait(20);
    }
    convertInto(s);
    ring_.publish(s, pts, gen, ++seq_);
    newestPts_ = pts;
    haveNewest_ = true;
}

void VideoPlayer::drainDecoder(uint32_t gen, const VideoRing::Policy& pol)
{
    avcodec_send_packet(codecCtx_, nullptr);
    while (!thread_.threadShouldExit() && avcodec_receive_frame(codecCtx_, decodedFrame_) == 0)
        onDecoded(gen, pol);
    drained_ = true;
}

bool VideoPlayer::seekToTimestamp(double timeSec)
{
    if (!formatCtx_)
        return false;

    int64_t timestamp = static_cast<int64_t>(timeSec / timeBase_);

    if (stats_) ++stats_->seeks;
    int ret = av_seek_frame(formatCtx_, videoStreamIndex_, timestamp,
                            AVSEEK_FLAG_BACKWARD);
    if (ret < 0)
    {
        // Try seeking from the beginning
        ret = av_seek_frame(formatCtx_, videoStreamIndex_, 0, AVSEEK_FLAG_BACKWARD);
    }

    if (codecCtx_)
        avcodec_flush_buffers(codecCtx_);
    drained_ = false;

    return ret >= 0;
}

bool VideoPlayer::decodeNextFrame()
{
    atEof_ = false;
    if (!formatCtx_ || !codecCtx_ || !decodedFrame_ || !packet_)
        return false;

    while (true)
    {
        int ret = av_read_frame(formatCtx_, packet_);
        if (ret < 0)
        {
            // End of file or error
            av_packet_unref(packet_);
            atEof_ = true;
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

void VideoPlayer::convertInto(int slot)
{
    // Bottom-up (GL order) in one pass: the destination starts at the slot's last row with a negative stride (sws
    // honours it -- plan-video F14, re-verified at 320x180 / 1080p / 4K for yuv420p and yuv422p10le). No flip, no
    // allocation.
    uint8_t* dst[4] = { slotBytes_[static_cast<size_t>(slot)] + static_cast<size_t>(height_ - 1) * static_cast<size_t>(rowBytes_),
                        nullptr, nullptr, nullptr };
    int dstStride[4] = { -rowBytes_, 0, 0, 0 };
    sws_scale(swsCtx_, decodedFrame_->data, decodedFrame_->linesize, 0, height_, dst, dstStride);
}
