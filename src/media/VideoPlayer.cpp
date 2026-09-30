// VideoPlayer.cpp — FFmpeg-based video decoder with GL texture upload.
//
// Decodes MP4/MOV/AVI/HAP video files frame-by-frame on a per-player decode thread, converts each frame to RGBA
// (bottom-up) into a 3-slot lock-free ring, and uploads the newest frame <= the clock to an OpenGL texture for
// compositing on the GL thread (s-rta-0928b video; plan-video.md).

#include "VideoPlayer.h"
#if JUCE_MAC
 #include <OpenGL/OpenGL.h>          // after juce_gl.h (via VideoPlayer.h)
 #include <OpenGL/CGLIOSurface.h>
 #include <IOSurface/IOSurfaceRef.h>
 #include <CoreFoundation/CoreFoundation.h>
#endif
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <cmath>
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
    releaseGL();
    for (size_t i = 0; i < slotBytes_.size(); ++i)
    {
#if JUCE_MAC
        if (surf_[i] != nullptr)
        {
            CFRelease(static_cast<IOSurfaceRef>(surf_[i]));   // s-rta-0929 vupload P3: the slot is an IOSurface
            surf_[i] = nullptr;
            slotBytes_[i] = nullptr;
            continue;
        }
#endif
        std::free(slotBytes_[i]);
        slotBytes_[i] = nullptr;
    }
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

    // A pixel format still unknown here (no frame decodes: an H.264 .mp4 cut to its header, an interrupted copy or
    // download) would reach sws_getContext as AV_PIX_FMT_NONE -- a libswscale assertion that aborts the whole app.
    // Fail the open instead: no player, "no media" (fix round 2; tests/test_video_player_open.cpp).
    if (codecCtx_->pix_fmt == AV_PIX_FMT_NONE)
    {
        std::cerr << "[VideoPlayer] Unknown pixel format (no frame decodes: a truncated or corrupt file): " << path
                  << " -- no media" << std::endl;
        freeFfmpeg();
        return false;
    }

    // s-rta-0929 vupload P3: the ring's slots are decided BEFORE the sws context (its destination format depends on it):
    // BGRA IOSurfaces on macOS (the blit path), else -- non-Apple, a failed IOSurfaceCreate, or the TEST-ONLY lever --
    // RGBA malloc'd blocks.
#if AUDIODNA_TEST_SERVER
    if (const char* force = std::getenv("ADNA_VIDEO_FORCE_FALLBACK"))   // TEST-ONLY (test-server builds): w10's arms
        forcePath_ = std::strcmp(force, "malloc") == 0 ? UploadPath::Malloc
                   : std::strcmp(force, "client") == 0 ? UploadPath::Client : forcePath_;
#endif
    const bool surfaces = forcePath_ != UploadPath::Malloc && createSurfaces();
    path_ = surfaces ? (forcePath_ == UploadPath::Client ? UploadPath::Client : UploadPath::Blit) : UploadPath::Malloc;
    if (!surfaces && forcePath_ != UploadPath::Malloc && stats_ != nullptr)
        ++stats_->surfaceFallbacks;

    // Set up swscale for conversion to RGBA (BGRA into an IOSurface: the same values, the byte order GL_BGRA reads)
    auto dstFmt = surfaces ? AV_PIX_FMT_BGRA : AV_PIX_FMT_RGBA;
    swsCtx_ = sws_getContext(width_, height_, codecCtx_->pix_fmt,
                              width_, height_, dstFmt,
                              SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!swsCtx_)
    {
        std::cerr << "[VideoPlayer] Failed to create swscale context" << std::endl;
        freeFfmpeg();
        return false;
    }

    if (!surfaces)
    {
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
    }

    // Decode the first frame into slot 0 (gen 0) and make the thumbnail from it: a fresh trigger without a seek is
    // never pending. Its pts is clamped to <= 0 so it is current from clock 0 (a stream whose first pts is a frame
    // or two late would otherwise wait for its own clock). A failed first decode publishes nothing: the player is
    // pending until the decode thread lands a frame -- or FAILED (W3) when it cannot (uploadToTexture).
    currentTime_ = 0.0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
    double thumbMs = 0.0;
    if (decodeNextFrame())
    {
        everDecoded_ = true;
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
              << ", upload=" << (path_ == UploadPath::Blit ? "iosurface-blit" : path_ == UploadPath::Client ? "iosurface-client" : "malloc")
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

GLuint VideoPlayer::uploadToTexture(bool* pending, VideoUpload::Budget* budget, double renderDt)
{
    if (pending != nullptr)
        *pending = false;
    if (!open_.load(std::memory_order_relaxed))
        return texture_;   // hold after close, as before
    const int64_t drawMs = nowMs();
    if (firstDrawMs_ < 0)
        firstDrawMs_ = drawMs;   // W3: the first draw request starts the first-frame timeout
    // s-rta-0929b gopcache: a drawn spell starts at the first draw after > 100 ms without one (a deck return, a trigger);
    // the wait from it (or from the last upload) to the next upload is video_max_upload_gap_ms.
    if (lastDrawCallMs_ < 0 || drawMs - lastDrawCallMs_ > 100)
        gapFromMs_ = drawMs;
    lastDrawCallMs_ = drawMs;
    pollFences();   // P3: a slot whose blit has completed goes back to the writer (the held one stays)
    trimmed_ = false;   // P4b: drawn again -- the next idle spell trims again

    // s-rta-0929 vupload P1: the per-frame upload budget is asked BEFORE the pick (peek: no state change), so a refused
    // player leaves the ring exactly as it was and HOLDS its shown frame -- never pending, never late (R-4, R-6). Exempt
    // (VU8): the first frame, the first upload after a GL release, the first frame of a new request generation.
    const uint32_t gen = gen_.load(std::memory_order_acquire);
    bool asked = false;
    if (budget != nullptr)
    {
        const auto pk = ring_.peek(currentTime_, gen, 0.5 * frameDur_);
        if (pk.slot >= 0 && shown_.needsUpload(pk.seq))
        {
            asked = true;
            const double speed = std::fabs(static_cast<double>(speed_.load(std::memory_order_relaxed)));
            const double contentFrameSec = speed > 1e-6 ? frameDur_ / speed : 1.0e9;   // speed 0: no next frame
            const bool isExempt = VideoUpload::exempt(shown_.everShown, textureCreated_, gen != shownGen_);
            if (!budget->admit(deferredFrames_, VideoUpload::maxDefer(contentFrameSec, renderDt), isExempt))
            {
                ++deferredFrames_;
                if (stats_) { ++stats_->uploadsDeferred; ++stats_->holdFrames; }
                return texture_;
            }
        }
    }
    deferredFrames_ = 0;

    // Frames the clock moved away from (reverse / ping-pong) are freed once they are more than the writer's look-ahead
    // (+1) frames ahead: forward play never gets that far ahead, and they would otherwise keep the writer out.
    const auto p = ring_.pick(currentTime_, gen, 0.5 * frameDur_, (kWriterLookAhead + 1) * frameDur_);
    if (stats_ && p.skipped > 0)
        stats_->framesSkipped += p.skipped;

    if (p.slot >= 0)
    {
        if (shown_.needsUpload(p.seq))
        {
            if (budget != nullptr && !asked)
                budget->charge();   // VU10: published between the peek and the pick -- this step's upload, counted
            shownGen_ = gen;
            const auto uploadStart = std::chrono::steady_clock::now();
            uploadSlot(p.slot);
            shown_.onUpload(p.seq);
            if (stats_)
            {
                ++stats_->uploads;
                ++stats_->playerUploads[statsSlot_];
                if (playing_.load(std::memory_order_relaxed))
                    VideoStats::noteMax(stats_->maxUploadGapMs, static_cast<float>(drawMs - gapFromMs_));
                VideoStats::noteMax(stats_->peakUploadMs, std::chrono::duration<float, std::milli>(
                                                              std::chrono::steady_clock::now() - uploadStart).count());
            }
            gapFromMs_ = drawMs;
        }
        lastShownPts_ = p.pts;
        // P4a: this slot is now the one ON SCREEN (held); the previously shown one goes back to the writer.
        const int prev = retire_.shown(p.slot, fence_[static_cast<size_t>(p.slot)] != nullptr);
        if (prev >= 0)
        {
            ring_.release(prev);
            releasedThisFrame_ = true;
        }
        return texture_;
    }

    // P4a: a GL context loss deleted the texture -- the held slot (still Reading) is the picture on the FIRST draw of the
    // new context, before any newer frame is at or before the clock (a paused / speed-0 / just-returned clip).
    if (!textureCreated_ && retire_.held >= 0)
        uploadSlot(retire_.held);

    // W3: never shown -> FAILED once the decode thread gave up before any frame or the first frame is overdue.
    if (!shown_.everShown)
    {
        const bool gaveUp = firstFrameGaveUp_.load(std::memory_order_acquire);
        const bool failed = VideoRing::firstFrameFailed(false, gaveUp, firstDrawMs_, nowMs());
        if (failed && !firstFrameFailed_ && !gaveUp)
            std::cerr << "[VideoPlayer] No first frame within " << VideoRing::kFirstFrameTimeoutMs
                      << " ms of the first draw: " << sourceFile_.getFullPathName() << " -- no media" << std::endl;
        firstFrameFailed_ = failed;
    }

    switch (VideoRing::judge(false, shown_.everShown, playing_.load(std::memory_order_relaxed), currentTime_,
                             lastShownPts_, frameDur_, firstFrameFailed_))
    {
        case VideoRing::Shown::Late:
            if (stats_) { ++stats_->holdFrames; ++stats_->lateFrames; }
            break;
        case VideoRing::Shown::Pending:
            if (stats_) { ++stats_->pendingFrames; ++stats_->pendingNow; }
            if (pending != nullptr)
                *pending = true;
            return 0;
        case VideoRing::Shown::Failed:
            return 0;   // W3: no media (*pending stays false) -- a crossfade onto it runs, render_frame answers
        case VideoRing::Shown::Held:
        case VideoRing::Shown::New:
            if (stats_) ++stats_->holdFrames;
            break;
    }
    // s-rta-0929 vupload: a HOLD (Held / Late) of a shown player with no picture -- texture 0 with *pending false: the
    // compositor runs the clip's effects FX-only over the layers below (Pitfall 53's trap). The witness; must stay 0.
    if (texture_ == 0 && stats_)
        ++stats_->holdNoTexture;
    return texture_;
}

void VideoPlayer::uploadSlot(int slot)
{
    if (path_ == UploadPath::Blit && blitSlot(slot))
        return;
    if (path_ != UploadPath::Malloc)
    {
        clientUploadSurface(slot);
        return;
    }
    // Client-memory upload: glTexImage2D once, then glTexSubImage2D (both copy before they return). The slot stays the
    // reader's until a newer frame is shown (retire_), so the same slot can be uploaded again after a context loss.
    const uint8_t* bytes = slotBytes_[static_cast<size_t>(slot)];
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
}

void VideoPlayer::releaseGL()
{
    // P3 (adoption VU3, SharedFrameSet::releaseGL's rule): every GL handle deleted AND zeroed -- a stale name from a dead
    // context would make the lazy creation skip. The fences are dropped, never glDeleteSync'd: a sync dies with its
    // context; the fenced slots go back to the writer below (retire_.contextLost).
    for (size_t i = 0; i < rectTex_.size(); ++i)
    {
        if (readFbo_[i] != 0) { glDeleteFramebuffers(1, &readFbo_[i]); readFbo_[i] = 0; }
        if (rectTex_[i] != 0) { glDeleteTextures(1, &rectTex_[i]); rectTex_[i] = 0; }
        fence_[i] = nullptr;
    }
    if (dstFbo_ != 0) { glDeleteFramebuffers(1, &dstFbo_); dstFbo_ = 0; }
    // P4a: the held slot (the frame on screen) is KEPT -- the next context's first draw re-uploads it.
    std::array<int, kSlots> toRelease{};
    const int n = retire_.contextLost(toRelease);
    for (int i = 0; i < n; ++i)
        ring_.release(toRelease[static_cast<size_t>(i)]);
    if (n > 0)
        releasedThisFrame_ = true;
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

void VideoPlayer::noteNoFirstFrame(const char* why)
{
    // W3: only before ANY frame of this file decoded; once. uploadToTexture turns the player FAILED ("no media").
    if (everDecoded_ || firstFrameGaveUp_.load(std::memory_order_relaxed))
        return;
    firstFrameGaveUp_.store(true, std::memory_order_release);
    std::cerr << "[VideoPlayer] No first frame (" << why << " before any frame): " << sourceFile_.getFullPathName()
              << " -- no media" << std::endl;
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
    // The writer runs up to kWriterLookAhead frames ahead of the shown frame (the old decode ran at most one): "behind"
    // must exceed that look-ahead, or forward play of a slow clip (a 24 fps clip: 3 frames > 0.1 s) would re-seek after
    // every few frames -- and no more than it (+1 frame), or reverse play re-seeks later than it needs to.
    pol.reseekBehindSec = std::max(pol.reseekBehindSec, (kWriterLookAhead + 1) * frameDur_);
    if (stats_) ++stats_->threadsAwake;

    while (!thread_.threadShouldExit())
    {
        // s-rta-0929 vupload P4b: the GL thread trimmed this idle player -- the writer purges its Free slots (and parks
        // again below). Before the idle check: trimIfIdle notifies a parked thread for exactly this.
        if (trimRequested_.exchange(false, std::memory_order_acq_rel))
            purgeFreeSlots();

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
            {
                noteNoFirstFrame("a decode error");   // W3: before any frame -> FAILED
                continue;               // a decode error: the next packet
            }
            if (!drained_)
            {
                drainDecoder(g, pol);   // EOF: the last frames frame-threading held back now show
                continue;
            }
            noteNoFirstFrame("EOF");    // W3: drained and still no frame -> FAILED
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
    everDecoded_ = true;
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
    if (!unpurge(s))   // P4b: a purged slot back in use (malloc path: the allocation failed -- drop this frame)
    {
        ring_.abandon(s);
        return;
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
    // s-rta-0929 vupload P3: an IOSurface slot is written under its lock (the CPU-write / GPU-read protocol).
#if JUCE_MAC
    auto* surface = static_cast<IOSurfaceRef>(surf_[static_cast<size_t>(slot)]);
    if (surface != nullptr)
        IOSurfaceLock(surface, 0, nullptr);
#endif
    uint8_t* dst[4] = { slotBytes_[static_cast<size_t>(slot)] + static_cast<size_t>(height_ - 1) * static_cast<size_t>(rowBytes_),
                        nullptr, nullptr, nullptr };
    int dstStride[4] = { -rowBytes_, 0, 0, 0 };
    sws_scale(swsCtx_, decodedFrame_->data, decodedFrame_->linesize, 0, height_, dst, dstStride);
#if JUCE_MAC
    if (surface != nullptr)
        IOSurfaceUnlock(surface, 0, nullptr);
#endif
}

// ---- s-rta-0929 vupload P3: IOSurface ring slots + one blit per new frame (plan-vupload.md 4.4 + adoption VU2 / VU3) ----

bool VideoPlayer::createSurfaces()
{
#if JUCE_MAC
    // SurfacePool.cpp's recipe: BGRA, 4 bytes per element (the codebase's IOSurface precedent). bytesPerRow may be padded
    // (63 px -> 256 B): the conversion and the client-upload fallback both use rowBytes_.
    const int32_t w = width_, h = height_, bpe = 4;
    const uint32_t pixelFormat = 'BGRA';
    CFNumberRef nw = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &w);
    CFNumberRef nh = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &h);
    CFNumberRef nb = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bpe);
    CFNumberRef nf = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixelFormat);
    const void* keys[] = { kIOSurfaceWidth, kIOSurfaceHeight, kIOSurfaceBytesPerElement, kIOSurfacePixelFormat };
    const void* values[] = { nw, nh, nb, nf };
    CFDictionaryRef props = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 4, &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
    bool ok = true;
    size_t bpr = 0;
    for (size_t i = 0; i < surf_.size() && ok; ++i)
    {
        IOSurfaceRef sf = IOSurfaceCreate(props);
        surf_[i] = sf;
        ok = sf != nullptr && (i == 0 || IOSurfaceGetBytesPerRow(sf) == bpr);
        if (sf != nullptr && i == 0)
            bpr = IOSurfaceGetBytesPerRow(sf);
    }
    CFRelease(props);
    CFRelease(nw); CFRelease(nh); CFRelease(nb); CFRelease(nf);
    if (!ok || bpr < static_cast<size_t>(width_) * 4)
    {
        for (auto*& sf : surf_)
            if (sf != nullptr) { CFRelease(static_cast<IOSurfaceRef>(sf)); sf = nullptr; }
        std::cerr << "[VideoPlayer] IOSurface slots unavailable (" << width_ << "x" << height_
                  << "): the malloc + glTexSubImage2D path" << std::endl;
        return false;
    }
    rowBytes_ = static_cast<int>(bpr);
    for (size_t i = 0; i < surf_.size(); ++i)   // stable for the surface's life (IOSurfaceGetBaseAddress)
        slotBytes_[i] = static_cast<uint8_t*>(IOSurfaceGetBaseAddress(static_cast<IOSurfaceRef>(surf_[i])));
    return true;
#else
    return false;
#endif
}

void VideoPlayer::ensureTexture()
{
    if (textureCreated_)
        return;
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    textureCreated_ = true;
}

void VideoPlayer::fallBack(const char* why)
{
    // The blit could not be set up (a CGL bind error, an incomplete FBO): this player uploads its surface bytes with
    // glTexSubImage2D from now on (VU3). Once per player, counted (video_surface_fallbacks), one log line.
    path_ = UploadPath::Client;
    if (!fallbackCounted_)
    {
        fallbackCounted_ = true;
        if (stats_ != nullptr)
            ++stats_->surfaceFallbacks;
        std::cerr << "[VideoPlayer] IOSurface blit unavailable (" << why << "): " << sourceFile_.getFullPathName()
                  << " -- glTexSubImage2D of the surface" << std::endl;
    }
}

void VideoPlayer::clientUploadSurface(int slot)
{
#if JUCE_MAC
    // The BGRA surface bytes, rows rowBytes_ apart (padded for odd widths), under a read-only lock.
    auto* surface = static_cast<IOSurfaceRef>(surf_[static_cast<size_t>(slot)]);
    ensureTexture();
    glBindTexture(GL_TEXTURE_2D, texture_);
    IOSurfaceLock(surface, kIOSurfaceLockReadOnly, nullptr);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowBytes_ / 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                    slotBytes_[static_cast<size_t>(slot)]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    IOSurfaceUnlock(surface, kIOSurfaceLockReadOnly, nullptr);
#else
    (void) slot;
#endif
}

bool VideoPlayer::blitSlot(int slot)
{
#if JUCE_MAC
    const auto i = static_cast<size_t>(slot);
    GLint prevRead = 0, prevDraw = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevRead);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDraw);
    auto restore = [&] {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(prevRead));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(prevDraw));
    };
    const bool rebind = rebind_[i].exchange(false, std::memory_order_acq_rel);   // P4b / VU1: un-purged since
    if (rectTex_[i] != 0 && rebind)
    {
        // The slot's IOSurface was purged (Empty) and made non-volatile again by the writer: re-specify the rectangle
        // texture on it (the default path, not a fallback -- the binding's page state after a purge is not relied on).
        glBindTexture(GL_TEXTURE_RECTANGLE, rectTex_[i]);
        const CGLError err = CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE, GL_RGBA8, width_, height_,
                                                    GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                                                    static_cast<IOSurfaceRef>(surf_[i]), 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        if (err != kCGLNoError)
        {
            restore();
            fallBack(("CGLTexImageIOSurface2D (re-bind) error " + std::to_string(static_cast<int>(err))).c_str());
            return false;
        }
    }
    if (rectTex_[i] == 0)
    {
        // Once per slot per context: a rectangle texture on the slot's IOSurface (the storage IS the surface: CPU
        // writes under IOSurfaceLock are what the GPU reads) and a read FBO on it (checked once, VU3).
        glGenTextures(1, &rectTex_[i]);
        glBindTexture(GL_TEXTURE_RECTANGLE, rectTex_[i]);
        const CGLError err = CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE, GL_RGBA8, width_, height_,
                                                    GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                                                    static_cast<IOSurfaceRef>(surf_[i]), 0);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        if (err != kCGLNoError)
        {
            restore();
            fallBack(("CGLTexImageIOSurface2D error " + std::to_string(static_cast<int>(err))).c_str());
            return false;
        }
        glGenFramebuffers(1, &readFbo_[i]);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo_[i]);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, rectTex_[i], 0);
        if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            restore();
            fallBack("the slot's read FBO is incomplete");
            return false;
        }
    }
    if (dstFbo_ == 0)
    {
        ensureTexture();
        glGenFramebuffers(1, &dstFbo_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo_);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            restore();
            fallBack("the texture's draw FBO is incomplete");
            return false;
        }
    }
    // One GPU copy, rect (BGRA storage, RGBA8) -> texture_ (RGBA8): the same values, alpha included. The slot is
    // bottom-up like texture_, so the 1:1 copy keeps the orientation. The scissor test would clip a blit.
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo_[i]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo_);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    restore();
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    // The slot may be written again only once the GPU has read it: fenced; no glFlush (the same context polls it).
    if (fence_[i] != nullptr)
        glDeleteSync(static_cast<GLsync>(fence_[i]));
    fence_[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    return true;
#else
    (void) slot;
    return false;
#endif
}

void VideoPlayer::pollFences()
{
    // Timeout 0: never waits (Pitfall 56). Signaled -> the slot goes back to the writer unless it is the one on screen.
    // GL_WAIT_FAILED (adoption VU2, SharedFrameSet.cpp's rule): that copy's state is unknown -- the fence is deleted and
    // the slot released like a signaled one (never stuck Reading), counted (video_fence_failed).
    for (size_t i = 0; i < fence_.size(); ++i)
    {
        if (fence_[i] == nullptr)
            continue;
        const GLenum r = fenceWaitOverride_ != nullptr ? static_cast<GLenum>(fenceWaitOverride_(fence_[i]))
                                                       : glClientWaitSync(static_cast<GLsync>(fence_[i]), 0, 0);
        if (r == GL_TIMEOUT_EXPIRED)
            continue;
        glDeleteSync(static_cast<GLsync>(fence_[i]));
        fence_[i] = nullptr;
        if (r == GL_WAIT_FAILED && stats_ != nullptr)
            ++stats_->fenceFailed;
        if (retire_.signaled(static_cast<int>(i)))
        {
            ring_.release(static_cast<int>(i));
            releasedThisFrame_ = true;
        }
    }
}

// ---- s-rta-0929 vupload P4b: the idle ring trim (plan-vupload.md R-14; two owners by construction) ----

void VideoPlayer::trimIfIdle(int64_t now)
{
    // GL thread (the reader): drops the Ready slots (only the reader leaves Ready) and asks the writer to purge the Free
    // ones. Idle = no draw for kTrimIdleMs (the thread parks at 250 ms; a crossfade's outgoing skip or a fenced frame's
    // hold is 1-3 frames, never idle). The held slot stays Reading: never purged.
    if (trimmed_ || !shown_.everShown || !open_.load(std::memory_order_relaxed)
        || now - lastDrawMs_.load(std::memory_order_acquire) <= kTrimIdleMs)
        return;
    pollFences();
    ring_.dropReady();
    trimmed_ = true;
    trimRequested_.store(true, std::memory_order_release);
    thread_.notify();
}

void VideoPlayer::purgeFreeSlots()
{
    // Decode thread (the writer): only the writer leaves Free, so a Free slot seen here stays Free while it is purged.
    for (size_t i = 0; i < purged_.size(); ++i)
    {
        if (purged_[i]
            || ring_.header(static_cast<int>(i)).state.load(std::memory_order_acquire)
                   != static_cast<uint8_t>(VideoRing::SlotState::Free))
            continue;
#if JUCE_MAC
        if (surf_[i] != nullptr)
            IOSurfaceSetPurgeable(static_cast<IOSurfaceRef>(surf_[i]), kIOSurfacePurgeableEmpty, nullptr);
        else
#endif
        {
            std::free(slotBytes_[i]);
            slotBytes_[i] = nullptr;
        }
        purged_[i] = true;
        if (stats_ != nullptr)
            ++stats_->slotsPurged;
    }
}

bool VideoPlayer::unpurge(int slot)
{
    // Decode thread, the slot just taken (Writing): back to non-volatile memory BEFORE it is written; the GL thread
    // re-binds its rectangle texture before the next blit (rebind_, published with the slot's Ready store).
    const auto i = static_cast<size_t>(slot);
    if (!purged_[i])
        return true;
#if JUCE_MAC
    if (surf_[i] != nullptr)
    {
        IOSurfaceSetPurgeable(static_cast<IOSurfaceRef>(surf_[i]), kIOSurfacePurgeableNonVolatile, nullptr);
        rebind_[i].store(true, std::memory_order_release);
        purged_[i] = false;
        return true;
    }
#endif
    slotBytes_[i] = static_cast<uint8_t*>(std::malloc(static_cast<size_t>(rowBytes_) * static_cast<size_t>(height_)));
    if (slotBytes_[i] == nullptr)
        return false;
    purged_[i] = false;
    return true;
}
