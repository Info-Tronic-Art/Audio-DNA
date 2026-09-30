// VideoPlayer.cpp — FFmpeg-based video decoder with GL texture upload.
//
// Decodes MP4/MOV/AVI/HAP video files frame-by-frame on a per-player decode thread, converts each frame to RGBA
// (bottom-up) into a 3-slot lock-free ring, and uploads the newest frame <= the clock to an OpenGL texture for
// compositing on the GL thread (s-rta-0928b video; plan-video.md).

#include "VideoPlayer.h"
#include "media/GopCacheStore.h"
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
    : cache_(std::make_unique<GopCache::Store>()), run_(std::make_unique<GopCache::Run>())
{
    static std::atomic<int> nextId{ 0 };
    playerId_ = nextId.fetch_add(1, std::memory_order_relaxed) + 1;   // GC9: never 0
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

    // Allocate frame and packet (and the pending frame: a decoded frame waiting for a ring slot, GC1)
    decodedFrame_ = av_frame_alloc();
    pendingFrame_ = av_frame_alloc();
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

    // s-rta-0929b gopcache c1b: the decode thread's policy, set here (decodeStep() runs it with or without the loop).
    // The writer runs up to kWriterLookAhead frames ahead of the shown frame (the old decode ran at most one): "behind"
    // must exceed that look-ahead, or forward play of a slow clip (a 24 fps clip: 3 frames > 0.1 s) would re-seek after
    // every few frames -- and no more than it (+1 frame), or reverse play re-seeks later than it needs to.
    pol_ = VideoRing::Policy{};
    pol_.skipNonRefInCatchUp = kSkipNonRefInCatchUp;
    pol_.reseekBehindSec = std::max(pol_.reseekBehindSec, (kWriterLookAhead + 1) * frameDur_);

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
        convertInto(decodedFrame_, s);
        ring_.publish(s, std::min(pts0, 0.0), 0, ++seq_);
        newestPts_ = lastDecodedPts_ = pts0;
        haveNewest_ = haveDecoded_ = true;
        const auto t0 = std::chrono::steady_clock::now();
        makeThumbnail();
        thumbMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    }

    // s-rta-0929b gopcache: the GOP cache is configured here (nothing allocated -- Pitfall 58; the first store allocates, on
    // the decode thread). Relative frame 0 = the first decoded frame's pts (GC3: a stream need not start at 0). GC5:
    // intra-only = the codec says so (HAP / ProRes / MJPEG / DNxHD / raw) or every index entry is a keyframe -- never
    // learned from runs. The GOP estimate = the index's longest keyframe interval (R-9; no index: kDefaultGopFrames).
    // gopcache-fix2 R1: the first frame's INDEX time (a pts-less first frame: its best-effort time, the scale every later
    // frame of the stream is indexed on).
    double firstIndexTime = 0.0;
    firstPts_ = (everDecoded_ && indexTimeOf(decodedFrame_, &firstIndexTime)) ? firstIndexTime : 0.0;
    nFrames_ = std::max(1, totalFrames_);
    {
        const auto id = codecpar->codec_id;
        intraOnly_ = id == AV_CODEC_ID_HAP || id == AV_CODEC_ID_PRORES || id == AV_CODEC_ID_MJPEG || id == AV_CODEC_ID_DNXHD
                     || id == AV_CODEC_ID_RAWVIDEO;
        int keys = 0, entries = 0;
        int64_t lastKeyTs = 0, maxGapTs = 0;   // index timestamps are DTS: the first may be negative (B-frame delay)
        std::vector<int64_t> keyTs;            // gop2 (A1): every keyframe entry's timestamp, in index order
#if LIBAVFORMAT_VERSION_MAJOR >= 59
        entries = avformat_index_get_entries_count(stream);
        for (int i = 0; i < entries; ++i)
        {
            const AVIndexEntry* e = avformat_index_get_entry(stream, i);
            if (e == nullptr || (e->flags & AVINDEX_KEYFRAME) == 0)
                continue;
            if (keys > 0)
                maxGapTs = std::max(maxGapTs, e->timestamp - lastKeyTs);
            ++keys;
            lastKeyTs = e->timestamp;
            keyTs.push_back(e->timestamp);
        }
#endif
        if (entries >= 2 && keys == entries)
            intraOnly_ = true;
        // gop2 (A1): the keyframes as relative indices, from the FIRST keyframe's timestamp (index timestamps are DTS: this
        // removes the B-frame delay); none without keyframe entries (the grid fallback)
        keyRels_.clear();
        for (const int64_t ts : keyTs)
            keyRels_.push_back(std::max(0, static_cast<int>(std::lround(static_cast<double>(ts - keyTs.front()) * timeBase_ / frameDur_))));
        if (keys >= 2 && maxGapTs > 0)
            gopFramesEst_ = std::max(1, static_cast<int>(std::lround(static_cast<double>(maxGapTs) * timeBase_ / frameDur_)));
        else if (keys == 1 && totalFrames_ > 0)
            gopFramesEst_ = totalFrames_;   // one keyframe: the whole file is one GOP
    }
    decodeMsEma_ = GopCache::kDecodeMsSeedPerMpix * static_cast<double>(width_) * static_cast<double>(height_) / 1.0e6;
    if (cacheBudget_ == nullptr)
        cacheBudget_ = &GopCache::sharedBudget();
    cache_->configure(static_cast<int>(codecCtx_->pix_fmt), width_, height_, cacheBudget_, stats_);
    cache_->ensureIndex(nFrames_ + 2);

    open_.store(true, std::memory_order_relaxed);
    playing_.store(true, std::memory_order_relaxed);

    std::cerr << "[VideoPlayer] Opened: " << path
              << " (" << width_ << "x" << height_
              << ", " << frameRate_ << " fps"
              << ", " << duration_ << "s"
              << ", codec=" << avcodec_get_name(codecpar->codec_id)
              << ", alpha=" << (alpha ? "yes" : "no")
              << ", upload=" << (path_ == UploadPath::Blit ? "iosurface-blit" : path_ == UploadPath::Client ? "iosurface-client" : "malloc")
              << ", thumb=" << thumbMs << " ms"
              << ", cacheFrame=" << cache_->frameBytes() << " B, gop=" << gopFramesEst_
              << (intraOnly_ ? ", intra-only" : "") << ", firstPts=" << firstPts_ << ")" << std::endl;

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
    clearCache();   // s-rta-0929b gopcache R-10: the cache's frames and budget share go with the FFmpeg contexts
    if (swsCtx_) { sws_freeContext(swsCtx_); swsCtx_ = nullptr; }
    if (decodedFrame_) { av_frame_free(&decodedFrame_); decodedFrame_ = nullptr; }
    if (pendingFrame_) { av_frame_free(&pendingFrame_); pendingFrame_ = nullptr; }
    pendingPublish_ = false;
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

    // The wanted time BEFORE the generation (release): a decode thread that sees the new generation sees its time (and,
    // s-rta-0929b gopcache, its direction).
    wantTime_.store(currentTime_, std::memory_order_release);
    wantReverse_.store(reverseNow_, std::memory_order_release);
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
        wantReverse_.store(reverseNow_, std::memory_order_release);
        gen_.fetch_add(1, std::memory_order_acq_rel);
        return;
    }

    advanceTransport(dt);
    if (discontinuity_)
    {
        discontinuity_ = false;
        wantReverse_.store(reverseNow_, std::memory_order_release);
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

    // s-rta-0929b gopcache (plan R-2): the clock's effective direction from here on -- after a PingPong reflection, so the
    // turn is ONE generation bump on the frame it happens (the sign of the motion: a negative speed from a composition file
    // runs the clock down too); a change is a discontinuity -- a generation bump, like a Loop wrap -- so the ring never
    // holds both directions' look-ahead. Speed 0 (no motion) keeps the direction.
    const double dirAfter = loopMode == LoopMode::PingPong ? ((reverse != !pingPongForward_) ? -1.0 : 1.0)
                                                          : (reverse ? -1.0 : 1.0);
    const double motion = static_cast<double>(speed) * dirAfter;
    if (motion != 0.0 && (motion < 0.0) != reverseNow_)
    {
        reverseNow_ = motion < 0.0;
        discontinuity_ = true;
        if (stats_ != nullptr)
            ++stats_->directionChanges;
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
        const auto pk = ring_.peek(currentTime_, gen, 0.5 * frameDur_, reverseNow_);
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
    // s-rta-0929b gopcache: the pick mirrors its comparisons while the clock runs down (reverseNow_, plan 3.2).
    const auto p = ring_.pick(currentTime_, gen, 0.5 * frameDur_, (kWriterLookAhead + 1) * frameDur_, reverseNow_);
    if (stats_ && p.skipped > 0)
        stats_->framesSkipped += p.skipped;

    if (p.slot >= 0)
    {
        if (shown_.needsUpload(p.seq))
        {
            if (budget != nullptr && !asked)
                budget->charge();   // VU10: published between the peek and the pick -- this step's upload, counted
            // s-rta-0929b gopcache (adoption GC4): a reversing player never shows a frame above the one it showed before in
            // the same request generation (must stay 0).
            if (reverseNow_ && stats_ != nullptr && gen == shownGen_ && shown_.everShown && p.pts > lastShownPts_)
                ++stats_->reverseNonmonotonic;
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
    // s-rta-0929b gopcache c1b (plan R-15): an outer loop -- exit / trim / park / wait -- around decodeStep(), one unit of
    // progress that never blocks. The ring-full wait of the old loop is one publish attempt per step with this loop's
    // 20 ms wait between attempts (the reader's release notifies, as before); the idle park is checked once per iteration.
    // The policy (pol_) is set in open(): a writer stepped without this loop (the ctests) runs the same rules.
    if (stats_) ++stats_->threadsAwake;

    while (!thread_.threadShouldExit())
    {
        // s-rta-0929 vupload P4b: the GL thread trimmed this idle player -- the writer purges its Free slots (and parks
        // again below). Before the idle check: trimIfIdle notifies a parked thread for exactly this.
        serviceTrim();   // s-rta-0929b gopcache R-10: the cache is dropped with the Free slots

        // Rule 15: a player that is not drawn (its deck off screen) decodes nothing. V2: a frame waiting for a ring slot is
        // dropped when the thread parks (the clock moved on while it was off screen).
        if (VideoRing::idleStep(nowMs(), lastDrawMs_.load(std::memory_order_acquire), pol_) == VideoRing::Idle::Park)
        {
            endRun();   // s-rta-0929b gopcache: a parked player holds no run (GC9: nor the one-PREFETCH token)
            park();
            pendingPublish_ = false;
            continue;
        }

        const auto t0 = std::chrono::steady_clock::now();
        const bool progressed = decodeStep();
        if (stats_ != nullptr)   // S1 (adoption, decode seat): the longest single writer step (INFO)
            VideoStats::noteMax(stats_->writerStepMaxMs,
                                std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count());
        if (!progressed)
            thread_.wait(20);   // a full ring (the reader's release notifies) or EOF (a wrap's generation bump re-seeks)
    }

    freeFfmpeg();   // the thread owns the contexts: nobody else touches them after start()
    if (stats_) { --stats_->threadsAwake; --stats_->threadsRunning; }
}

bool VideoPlayer::decodeStep()
{
    // A frame waiting for a ring slot: one attempt (today's ring-full loop body). Reverse: a DEMAND run's frame -- while it
    // waits, the run's decodes continue (it lives in pendingFrame_, never in decodedFrame_: GC1).
    if (pendingPublish_)
    {
        if (tryPublishPending())
            return true;
        return revGen_ ? idleWork(wantTime_.load(std::memory_order_acquire)) : false;
    }

    // EOF: the frames frame-threading held back, one per step (drainDecoder's loop).
    if (draining_)
    {
        if (!thread_.threadShouldExit() && avcodec_receive_frame(codecCtx_, decodedFrame_) == 0)
        {
            onDecoded(myGen_);
            return true;
        }
        draining_ = false;
        drained_ = true;
        return true;
    }

    const uint32_t g = gen_.load(std::memory_order_acquire);
    const double want = wantTime_.load(std::memory_order_acquire);
    const bool rev = wantReverse_.load(std::memory_order_acquire);
    enforceCap(want);   // GC6: every step, not only at store decisions
    if (g != myGen_)
    {
        // A new request (a seek, a wrap, a direction change): a run ends (its frames stay resident); reverse plans its own
        // seeks; forward seeks as before unless the wanted frame is resident (hits first, c4).
        myGen_ = g;
        // c4: forward hits only where they help -- a reverse episode just ended (a flip, a PingPong bottom turn) or the
        // wanted frame is resident (a seek back into the retained frames); plain forward play decodes as before
        hitsArmed_ = !rev && !intraOnly_ && (revGen_ || cache_->slotOf(relOf(want)) >= 0);
        revGen_ = rev;
        endRun();
        haveServed_ = false;
        servedFromCache_ = false;
        repoSeeked_ = false;
        prefetchBlockedAt_ = -1;
        if (revGen_ || hitsArmed_)
            haveNewest_ = false;
        else
        {
            seekToTimestamp(want);   // the keyframe at or before want; the catch-up follows
            haveNewest_ = haveDecoded_ = false;
        }
    }
    if (revGen_)
        return reverseStep(want);
    return forwardStep(g, want);
}

bool VideoPlayer::forwardStep(uint32_t g, double want)
{
    // c4: a forward HIT -- the next frame is resident (a reverse episode's frames after a flip, a PingPong bottom turn, a
    // seek back into the retained frames): published from the cache while the decoder is repositioned in idle steps.
    const int nextRel = haveNewest_ ? relOf(newestPts_) + 1 : relOf(want);
    if (hitsArmed_ && cache_->slots() > 0)
    {
        const int slot = cache_->slotOf(nextRel);
        if (slot >= 0 && VideoRing::shouldPublish(cache_->ptsAt(slot), want, frameDur_, pol_))
        {
            if (!publishCached(slot))
                return forwardIdle(want);
            servedFromCache_ = true;
            return true;
        }
    }
    if (hitsArmed_ && !haveNewest_ && !servedFromCache_)
    {
        // armed at the generation change, but the wanted frame is not resident (any more): the change's seek, deferred
        hitsArmed_ = false;
        seekToTimestamp(want);
        haveDecoded_ = false;
    }
    else if (VideoRing::decide(want, newestPts_, haveNewest_, pol_) == VideoRing::Step::Reseek)
    {
        seekToTimestamp(want);   // the keyframe at or before want; the catch-up follows
        haveNewest_ = haveDecoded_ = false;
        servedFromCache_ = false;
    }
    else if (servedFromCache_)
    {
        // after hits: the decoder must output the frame after the last one served -- seek unless it is at most a GOP below
        // it (then decode forward: the guard in onDecoded drops what was already served)
        servedFromCache_ = false;
        if (!haveDecoded_ || lastDecodedRel_ + 1 > nextRel || nextRel - (lastDecodedRel_ + 1) > gopFramesEst_)
        {
            seekToTimestamp(ptsOfRel(nextRel));
            haveDecoded_ = false;
        }
    }

    codecCtx_->skip_frame = (haveDecoded_ && VideoRing::useSkipNonRef(lastDecodedPts_, want, frameDur_, pol_))
                                ? AVDISCARD_NONREF
                                : AVDISCARD_DEFAULT;

    if (!decodeNextFrame())
    {
        if (!atEof_)
        {
            noteNoFirstFrame("a decode error");   // W3: before any frame -> FAILED
            return true;                          // a decode error: the next packet
        }
        if (!drained_)
        {
            avcodec_send_packet(codecCtx_, nullptr);   // EOF: the last frames frame-threading held back now show
            draining_ = true;
            return true;
        }
        noteNoFirstFrame("EOF");   // W3: drained and still no frame -> FAILED
        return false;              // EOF: the Loop wrap's generation bump (or a seek) re-seeks
    }
    onDecoded(g);
    return true;
}

void VideoPlayer::onDecoded(uint32_t gen)
{
    // No pts -> take it (the old decodeFrameAtTime's "use whatever we got").
    const double pts = decodedFrame_->pts >= 0 ? static_cast<double>(decodedFrame_->pts) * timeBase_
                                               : wantTime_.load(std::memory_order_acquire);
    lastDecodedPts_ = pts;
    // gopcache-fix2 R1: the decoder's position by the frame's own time (a pts-less frame's clock time is no index)
    double indexTime = pts;
    lastDecodedRel_ = relOf(indexTimeOf(decodedFrame_, &indexTime) ? indexTime : pts);
    haveDecoded_ = true;
    everDecoded_ = true;
    if (stats_) ++stats_->framesDecoded;

    // A catch-up chases the moving clock: frames behind it are dropped without a conversion. c4: so is a frame at or
    // below the newest one published -- ONLY while cache hits are in play (hitsArmed_ / servedFromCache_: the decoder then
    // re-outputs frames the cache already served). gopcache-fix F2: plain forward play never applies it -- a frame without
    // a pts takes the clock's time, so two decoded in one clock tick would compare equal and the second would be dropped
    // (a DivX-style AVI: shown 196 -> 155 / 480 render frames); main's writer publishes both.
    if (!VideoRing::shouldPublish(pts, wantTime_.load(std::memory_order_acquire), frameDur_, pol_)
        || ((hitsArmed_ || servedFromCache_) && haveNewest_ && pts <= newestPts_ + 0.5 * frameDur_))
    {
        if (stats_) ++stats_->framesDropped;
        return;
    }

    // Pending until a ring slot takes it (GC1: its own AVFrame -- the next decode never overwrites it).
    av_frame_unref(pendingFrame_);
    av_frame_move_ref(pendingFrame_, decodedFrame_);
    pendingGen_ = gen;
    pendingPts_ = pts;
    pendingPublish_ = true;
    tryPublishPending();
}

bool VideoPlayer::tryPublishPending()
{
    const int s = ring_.acquireWrite();
    if (s < 0)   // the ring is full: the reader frees a slot and notifies
    {
        if (thread_.threadShouldExit() || gen_.load(std::memory_order_acquire) != pendingGen_)
        {
            pendingPublish_ = false;   // exiting, or a seek arrived: this frame is stale
            av_frame_unref(pendingFrame_);
            return true;
        }
        if (!revGen_ && VideoRing::decide(wantTime_.load(std::memory_order_acquire), newestPts_, haveNewest_, pol_)
                            == VideoRing::Step::Reseek)
        {
            pendingPublish_ = false;   // the clock moved away (reverse play): the next step re-seeks
            av_frame_unref(pendingFrame_);
            return true;
        }
        return false;
    }
    pendingPublish_ = false;
    if (!unpurge(s))   // P4b: a purged slot back in use (malloc path: the allocation failed -- drop this frame)
    {
        ring_.abandon(s);
        av_frame_unref(pendingFrame_);
        return true;
    }
    convertInto(pendingFrame_, s);
    if (!revGen_)
    {
        // c4: GC8 / R-9 (a copy: the ring slot is written), at the frame's own index -- gopcache-fix2 R1: never at a pts-less
        // frame's clock time (pendingPts_); a frame with no index time is not retained
        double indexTime = 0.0;
        if (indexTimeOf(pendingFrame_, &indexTime))
            retainForward(relOf(indexTime), indexTime, pendingFrame_);
        hitsArmed_ = false;   // the decoder has taken over from the cache
    }
    av_frame_unref(pendingFrame_);   // its buffer back to the decoder's pool now (never later than the old loop did)
    ring_.publish(s, pendingPts_, pendingGen_, ++seq_);
    newestPts_ = pendingPts_;
    haveNewest_ = true;
    if (revGen_ && pendingRel_ >= 0)
    {
        servedRel_ = pendingRel_;
        haveServed_ = true;
    }
    pendingRel_ = -1;
    return true;
}

// ---- s-rta-0929b gopcache c3: the reverse step machine (plan-gopcache.md 3.5 + HARMONY ADOPTION GC1-GC9) ----

int VideoPlayer::relOf(double pts) const
{
    return static_cast<int>(std::lround((pts - firstPts_) / frameDur_));
}

double VideoPlayer::ptsOfRel(int rel) const
{
    return firstPts_ + static_cast<double>(rel) * frameDur_;
}

// gopcache-fix2 R1: the time a decoded frame is INDEXED by (its relative index, the time the cache keeps with it): its pts,
// else libavcodec's best-effort timestamp. A DivX-style AVI (MPEG-4 part 2 with B-frames) has no pts on its keyframes and
// P-frames; their best-effort time follows the packet order, is the same from the file start and after any seek, and is on
// the scale of the B-frames' pts. false = neither (the stream's last frame, drained at EOF): the caller decides.
bool VideoPlayer::indexTimeOf(const AVFrame* f, double* t) const
{
    const int64_t ts = f->pts >= 0 ? f->pts : f->best_effort_timestamp;
    if (ts == AV_NOPTS_VALUE || ts < 0)
        return false;
    *t = static_cast<double>(ts) * timeBase_;
    return true;
}

int VideoPlayer::nTraj() const
{
    return lastRel_ >= 0 ? lastRel_ + 1 : std::max(1, nFrames_);
}

double VideoPlayer::frameMsEff() const
{
    const double sp = std::fabs(static_cast<double>(speed_.load(std::memory_order_relaxed)));
    return sp > 1e-6 ? 1000.0 * frameDur_ / sp : 1.0e9;
}

int64_t VideoPlayer::floorBytes() const
{
    return static_cast<int64_t>(GopCache::kMinFrames) * cache_->frameBytes();
}

// This cache's share of the budget in frames (floors win).
int VideoPlayer::capSlots() const
{
    const int64_t fb = std::max<int64_t>(1, cache_->frameBytes());
    return std::max(GopCache::kMinFrames, static_cast<int>(std::min<int64_t>(cacheBudget_->capBytes() / fb, 1 << 24)));
}

// Forward retention: PingPong keeps enough for its top turn (R-9, capped at a quarter of the budget -- Q2); Loop / OneShot
// keep the last kBehindFrames (GC8: a flip's cover). Intra-only files keep nothing (a seek + one decode is cheap).
int VideoPlayer::forwardRetain() const
{
    if (intraOnly_)
        return 0;
    if (loopMode_.load(std::memory_order_relaxed) == LoopMode::PingPong)
    {
        const int64_t fb = std::max<int64_t>(1, cache_->frameBytes());
        const int quarter = static_cast<int>(static_cast<double>(cacheBudget_->totalBytes()) * GopCache::kRetainBudgetFrac
                                             / static_cast<double>(fb));
        return GopCache::retainFrames(gopFramesEst_, decodeMsEma_, frameMsEff(), std::min(capSlots() / 2, quarter));
    }
    return GopCache::kBehindFrames;
}

static GopCache::Mode cacheModeOf(VideoPlayer::LoopMode m)
{
    return m == VideoPlayer::LoopMode::PingPong ? GopCache::Mode::PingPong
           : m == VideoPlayer::LoopMode::OneShot ? GopCache::Mode::OneShot
                                                 : GopCache::Mode::Loop;
}

// Every resident frame's pool and key at the clock's frame `cur` (the policy view the pure decisions read).
void VideoPlayer::refreshView(int cur, bool forward)
{
    const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
    const int n = nTraj();
    const int retain = forward ? forwardRetain() : GopCache::kBehindFrames;
    for (auto& s : cache_->view())
    {
        if (s.frame < 0)
            continue;
        s.pool = GopCache::poolOf(mode, forward, n, cur, s.frame, retain);
        s.key = GopCache::keyOf(mode, forward, n, cur, s.frame, s.pool);
    }
}

void VideoPlayer::enforceCap(double want)
{
    if (cache_->slots() == 0)
        return;
    const int64_t cap = std::max(floorBytes(), cacheBudget_->capBytes());
    if (stats_ != nullptr)
        stats_->gopCacheCapBytes.store(cap, std::memory_order_relaxed);
    if (cache_->bytes() <= cap)
        return;
    // a kept empty slot goes first; else the farthest next use (never the served / wanted frame)
    for (int i = 0; i < cache_->slots(); ++i)
        if (cache_->view()[static_cast<size_t>(i)].frame < 0)
        {
            cache_->freeSlot(i);
            return;
        }
    refreshView(relOf(want), !revGen_);
    const int s = GopCache::evictOne(cache_->view(), runPublishAt_, haveServed_ ? servedRel_ : -1);
    if (s >= 0)
        cache_->freeSlot(s);
}

void VideoPlayer::serviceTrim()
{
    // s-rta-0929 vupload P4b: the GL thread trimmed this idle player -- the writer purges its Free slots (and parks again
    // in the loop). Before the idle check: trimIfIdle notifies a parked thread for exactly this. s-rta-0929b gopcache R-10:
    // and drops its GOP cache (its bytes back to the budget; the next DEMAND run rebuilds it).
    if (trimRequested_.exchange(false, std::memory_order_acq_rel))
    {
        purgeFreeSlots();
        clearCache();
    }
}

void VideoPlayer::clearCache()
{
    endRun();
    prefetchBlockedAt_ = -1;   // F1: the cache is empty -- every target may be planned again
    if (cache_ != nullptr)
        cache_->clear();
    haveServed_ = false;
}

void VideoPlayer::endRun()
{
    // F1: a window nothing could be stored in -- its TARGET is not planned again (keyed to the served frame, the same
    // un-fetchable target came back after every served frame: one seek + catch-up each, forever); the DEMAND run reaches it
    if (run_ != nullptr && run_->kind == GopCache::RunKind::Prefetch && runStored_ == 0 && run_->decoded > 0)
        prefetchBlockedAt_ = run_->target;
    runStored_ = 0;
    runPrevRel_ = -1;
    runLastOutRel_ = -1;
    runLanded_ = false;
    runOvershoots_ = 0;
    runSeekBackSec_ = 0.0;
    if (runHasToken_ && cacheBudget_ != nullptr)
        cacheBudget_->releasePrefetch(playerId_);
    runHasToken_ = false;
    if (run_ != nullptr)
        *run_ = GopCache::Run{};
    runPublishAt_ = -1;
}

bool VideoPlayer::storeDecoded(int rel, double pts, bool forward, int cur, int protect)
{
    const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
    const int n = nTraj();
    const int capS = capSlots();
    const int retain = forward ? forwardRetain() : 0;
    const int behindCap = GopCache::behindCapFor(mode, forward, capS, retain);
    refreshView(cur, forward);
    const auto pool = GopCache::poolOf(mode, forward, n, cur, rel, forward ? retain : GopCache::kBehindFrames);
    const int key = GopCache::keyOf(mode, forward, n, cur, rel, pool);
    const auto v = GopCache::judgeStore(cache_->view(), capS, behindCap, rel, key, pool, protect, haveServed_ ? servedRel_ : -1);
    const bool ok = v.keep != GopCache::Keep::Drop
                    && cache_->store(decodedFrame_, rel, pts, v, GopCache::Slot{ rel, key, pool }, floorBytes());
    if (!ok && stats_ != nullptr)
        ++stats_->gopCacheDrops;
    return ok;
}

bool VideoPlayer::publishCached(int slot)
{
    const int s = ring_.acquireWrite();
    if (s < 0)
        return false;
    if (!unpurge(s))   // P4b: a purged slot back in use (malloc path: the allocation failed)
    {
        ring_.abandon(s);
        return false;
    }
    const double pts = cache_->ptsAt(slot);
    const int rel = cache_->view()[static_cast<size_t>(slot)].frame;
    convertInto(cache_->frameAt(slot), s);   // the SAME conversion as a decoded frame: the pixels of forward play
    ring_.publish(s, pts, myGen_, ++seq_);
    newestPts_ = pts;
    haveNewest_ = true;
    if (revGen_)
    {
        servedRel_ = rel;
        haveServed_ = true;
    }
    if (stats_ != nullptr)
        ++stats_->gopCacheHits;
    return true;
}

bool VideoPlayer::publishDecodedNow(int rel, double pts)
{
    const int s = ring_.acquireWrite();
    if (s >= 0 && unpurge(s))
    {
        convertInto(decodedFrame_, s);
        ring_.publish(s, pts, myGen_, ++seq_);
        newestPts_ = pts;
        haveNewest_ = true;
        servedRel_ = rel;
        haveServed_ = true;
        return true;
    }
    if (s >= 0)
        ring_.abandon(s);
    // the ring is full: pending in its own frame (GC1) -- the run's next decodes never touch it
    av_frame_unref(pendingFrame_);
    av_frame_move_ref(pendingFrame_, decodedFrame_);
    pendingGen_ = myGen_;
    pendingPts_ = pts;
    pendingRel_ = rel;
    pendingPublish_ = true;
    return false;
}

// Reverse (plan 3.5 reverseStep + GC2 / GC3): the next frame down from the last one served -- never behind the clock's frame
// (a writer that lags jumps to the clock: the skipped frames are counted dropped), never more than the look-ahead below it;
// a resident frame is published at once (a HIT); a miss starts a DEMAND run (unless the running run decodes toward it).
bool VideoPlayer::reverseStep(double want)
{
    const int wantRel = relOf(want);
    int next;
    if (!haveServed_)
        next = wantRel;
    else
    {
        next = servedRel_ - 1;
        if (next > wantRel)   // GC2: the writer LAGS (its frames are still above the clock) -> the clock's frame
        {
            if (stats_ != nullptr)
                stats_->framesDropped += next - wantRel;
            next = wantRel;
        }
    }
    next = std::min(next, nTraj() - 1);   // the clock above the last frame (a wrap lands at the duration): the last frame
    while (next >= 0 && cache_->isHole(next))   // GC3: a frame the decoder never outputs is skipped, never sought again
        --next;
    if (next < 0)
        return idleWork(want);   // below the first frame: the GL thread's wrap / reflection brings a new generation
    const int slot = intraOnly_ ? -1 : cache_->slotOf(next);
    // GC2: far enough AHEAD of the clock -- run work instead. gopcache-fix2 R2: measured in TIME on the frame's own pts (a
    // resident frame's; the nominal time of a miss), after the holes are walked: the pick frees a frame more than
    // (kWriterLookAhead + 1) frame durations + half a frame below the clock, and a VFR file's index (the AVERAGE frame
    // duration) put a frame of index wantRel - 3 up to ~4 frame durations below it -- published, freed unshown, never
    // published again that lap (98994c6: 28 of the 63 frames a lap).
    if (haveServed_ && (slot >= 0 ? cache_->ptsAt(slot) : ptsOfRel(next)) < want - (kWriterLookAhead + 1) * frameDur_)
        return idleWork(want);
    if (slot >= 0)
        return publishCached(slot) ? true : idleWork(want);
    // a miss: the running run keeps going when it will still decode `next`; else a DEMAND run for it
    const auto& r = *run_;
    const bool covers = r.kind != GopCache::RunKind::None && next >= r.windowLo && next <= r.target
                        && (r.seekPending ? r.seekFrom <= next : (!haveDecoded_ || lastDecodedRel_ < next));
    if (covers)
    {
        if (runPublishAt_ != next)
        {
            runPublishAt_ = next;
            if (r.kind == GopCache::RunKind::Prefetch && stats_ != nullptr)
                ++stats_->gopCacheMisses;   // a prefetch overtaken by the clock serves the miss
        }
    }
    else
        startDemand(next, want);
    return runStep(want);
}

void VideoPlayer::startDemand(int rel, double want)
{
    endRun();
    auto& r = *run_;
    r.kind = GopCache::RunKind::Demand;
    r.target = rel;
    r.seekFrom = rel;   // the keyframe at or before it
    r.seekPending = true;
    runPublishAt_ = rel;
    if (intraOnly_)
    {
        r.windowLo = rel + 1;   // GC5: nothing stored -- one seek + one decode + a direct publish
        return;
    }
    // latency first: the frames just below the target (kDemandWindow, within the free share), NONREF skipping below them
    // like a forward catch-up; the window further down is a PREFETCH run's, in the idle steps
    const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
    const int capS = capSlots();
    refreshView(relOf(want), false);
    const int avail = GopCache::availableFor(cache_->view(), capS, GopCache::behindCapFor(mode, false, capS, 0), 0);
    r.windowLo = std::max(0, rel - std::max(0, std::min(avail, GopCache::kDemandWindow) - 1));
}

void VideoPlayer::planPrefetchRun(double want)
{
    if (intraOnly_ || !haveServed_)
        return;
    const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
    const int n = nTraj();
    const int cur = relOf(want);
    const int capS = capSlots();
    const int behindCap = GopCache::behindCapFor(mode, false, capS, 0);
    refreshView(cur, false);
    const int pAt = GopCache::prefetchAt(capS, gopFramesEst_, decodeMsEma_, frameMsEff());
    // gop2 (GC7): the in-place window -- the lead-in from the index's keyframe below the window at the measured decode time
    const auto r = GopCache::planPrefetch(cache_->index(), cache_->view(), mode, n, cur, servedRel_, capS, behindCap, pAt,
                                          false, std::max(1, (capS - behindCap) / 2),
                                          GopCache::Lead{ decodeMsEma_, frameMsEff(), gopFramesEst_, &keyRels_ });
    if (r.kind != GopCache::RunKind::Prefetch || r.target == prefetchBlockedAt_)
        return;
    // GC9: one PREFETCH run at a time across players (a deck's players returning together fill one after another) --
    // unless this player would run dry before a run of its own could land (fewer resident frames ahead than one GOP of
    // decode covers): staggered, never starved
    runHasToken_ = cacheBudget_->takePrefetch(playerId_);
    if (!runHasToken_)
    {
        const int urgent = std::max(2, static_cast<int>(std::ceil(gopFramesEst_ * decodeMsEma_ / frameMsEff() * 1.2)));
        if (GopCache::unservedAhead(cache_->index(), mode, n, servedRel_) >= urgent)
            return;
    }
    *run_ = r;
    runPublishAt_ = -1;
}

// The ring is full (or the writer is ahead): one decode of a run. Reverse only here; forward reposition is c4's.
bool VideoPlayer::idleWork(double want)
{
    if (!revGen_)
        return forwardIdle(want);
    if (run_->kind == GopCache::RunKind::None)
        planPrefetchRun(want);
    if (run_->kind == GopCache::RunKind::None)
        return false;
    return runStep(want);
}

bool VideoPlayer::runStep(double want)
{
    auto& r = *run_;
    if (r.seekPending)
    {
        seekToTimestamp(ptsOfRel(r.seekFrom) - runSeekBackSec_);   // F1: further back after an overshoot
        r.seekPending = false;
        haveDecoded_ = false;
        runSawKey_ = false;
        runLanded_ = false;
        runPrevRel_ = -1;
        runLastOutRel_ = -1;
        if (stats_ != nullptr && runOvershoots_ == 0)   // a run's re-seek is still that run
            ++(r.kind == GopCache::RunKind::Demand ? stats_->gopCacheMisses : stats_->gopCacheRuns);
    }
    // R-7: non-reference frames are skipped only well below the storage window (a skipped frame is a hole in it)
    const int lo = runPublishAt_ >= 0 ? std::min(r.windowLo, runPublishAt_) : r.windowLo;
    codecCtx_->skip_frame = (haveDecoded_ && lo - lastDecodedRel_ > GopCache::kFullDecodeFrames) ? AVDISCARD_NONREF
                                                                                               : AVDISCARD_DEFAULT;
    const auto t0 = std::chrono::steady_clock::now();
    if (!decodeNextFrame())
    {
        if (!atEof_)
            return true;   // a decode error: the next packet
        // EOF: the frames frame-threading holds back, then the run ends; the last frame is now known
        avcodec_send_packet(codecCtx_, nullptr);
        while (!thread_.threadShouldExit() && avcodec_receive_frame(codecCtx_, decodedFrame_) == 0)
        {
            onRunFrame(want);
            if (r.kind == GopCache::RunKind::None)
                break;   // the run ended on this frame
            if (r.seekPending)
                return true;   // F1: the landing overshot the window -- the run seeks again (the seek flushes the rest)
        }
        drained_ = true;
        if (haveDecoded_)
            lastRel_ = std::max(lastRel_, lastDecodedRel_);
        endRun();
        return true;
    }
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    decodeMsEma_ = 0.9 * decodeMsEma_ + 0.1 * ms;
    onRunFrame(want);
    return true;
}

void VideoPlayer::onRunFrame(double want)
{
    auto& r = *run_;
    // gopcache-fix2 R1: the frame's index comes from its OWN time -- its pts, else its best-effort time (indexTimeOf); a
    // frame with neither (the stream's last frame, drained at EOF) takes the previous output's index + 1 when that output
    // was inside the window (NONREF skipping ends kFullDecodeFrames below it: no frame was skipped in between); else its
    // index is unknown and it is neither stored nor published. Never the run's target (98994c6: a pts-less keyframe was
    // stored as the frame the run was sent for).
    const int lo = runPublishAt_ >= 0 ? std::min(r.windowLo, runPublishAt_) : r.windowLo;
    double pts = 0.0;
    bool known = indexTimeOf(decodedFrame_, &pts);
    int rel = known ? relOf(pts) : -1;
    if (!known && runLastOutRel_ >= 0 && runLastOutRel_ >= lo)
    {
        rel = runLastOutRel_ + 1;
        pts = ptsOfRel(rel);
        known = true;
    }
    runLastOutRel_ = known ? rel : -1;
    if (known)
    {
        lastDecodedPts_ = pts;
        lastDecodedRel_ = rel;
        haveDecoded_ = true;
    }
    everDecoded_ = true;
    ++r.decoded;
    if (stats_ != nullptr)
    {
        ++stats_->framesDecoded;
        ++stats_->gopCacheRunDecodes;
    }
    const bool hasPts = decodedFrame_->pts >= 0;
    if (!runLanded_)
    {
        // F1: the first output since the seek sits ABOVE the frame the run was sent for -- the demuxer picked a keyframe
        // by its DTS, and a B-frame stream's keyframe decodes before the frames shown just below it (which then never come
        // out of this seek: an MPEG-4 part 2 file's last GOP). Seek again, further back, instead of calling them holes.
        // gopcache-fix2 R1: a first output with no index at all (a last GOP whose keyframe is drained at EOF) cannot be
        // judged -- seek a GOP further back, so the frames before it index it.
        runLanded_ = true;
        if (r.kind != GopCache::RunKind::None && (!known || rel > r.seekFrom) && runOvershoots_ < kMaxRunOvershoots
            && ptsOfRel(r.seekFrom) - runSeekBackSec_ > firstPts_)
        {
            ++runOvershoots_;
            runSeekBackSec_ = std::max(2.0 * runSeekBackSec_,
                                       known ? (rel - r.seekFrom + 1 + std::max(1, codecCtx_->has_b_frames)) * frameDur_
                                             : std::max(1, gopFramesEst_) * frameDur_);
            r.seekPending = true;
            return;
        }
    }
    // GC3 + gop2 R3: a key-flagged frame, or a frame at / after a demuxer-key landing's pts -- a recovery point's frames
    // (intra-refresh H.264) are output once recovered but never key-flagged; an MPEG-TS landing packet is never key-flagged,
    // so its garbage stays out
    static_assert(AV_NOPTS_VALUE == GopCache::kNoPts);
    if (GopCache::storeGateOpens((decodedFrame_->flags & AV_FRAME_FLAG_KEY) != 0, landedOnKeyPacket_, decodedFrame_->pts,
                                 landingPts_))
        runSawKey_ = true;
    if (!known)
    {
        runPrevRel_ = -1;   // R1: an unknown index -- nothing stored, published or inferred across it
        return;
    }
    // F1 (GC3 "every run has a negative result", for EVERY run kind): after the run's keyframe the decoder's output is in
    // pts order, so an index between two consecutive outputs inside the window is one it never outputs -- a hole: walked
    // over by the reverse step AND by the prefetch planner (a PREFETCH that passed it used to mark nothing, so it stayed
    // the next target). A frame without a pts breaks the chain (its index is unknown: nothing is inferred across it).
    if (runSawKey_ && hasPts)
    {
        if (runPrevRel_ >= 0)
        {
            for (int k = std::max(runPrevRel_ + 1, lo); k < rel && k <= r.target; ++k)
                cache_->markHole(k);   // only an index still unknown (-1) becomes a hole
        }
        runPrevRel_ = std::max(runPrevRel_, rel);
    }
    else if (!hasPts)
        runPrevRel_ = -1;
    // GC3: only frames decoded after the run's keyframe (an open-GOP seek's leading frames reference the GOP before it) and
    // not flagged corrupt are stored
    const int cur = relOf(want);
    if (!intraOnly_ && runSawKey_ && rel >= r.windowLo && rel <= r.target && cache_->slotOf(rel) < 0
        && cache_->storable(decodedFrame_))
        runStored_ += storeDecoded(rel, pts, false, cur, runPublishAt_) ? 1 : 0;
    // publish: the first decoded frame at or above min(the frame asked for, the clock's frame NOW) -- the clock kept falling
    // while the run decoded upward (a forward catch-up's shouldPublish, mirrored); the frames above it are behind the clock
    if (runPublishAt_ >= 0 && rel >= std::min(runPublishAt_, std::max(0, relOf(wantTime_.load(std::memory_order_acquire)))))
    {
        const int target = runPublishAt_;
        runPublishAt_ = -1;
        int slot = rel <= target ? cache_->slotOf(rel) : -1;
        if (rel > target)   // GC3: passed without output -- a hole; the nearest resident frame below stands in
        {
            cache_->markHole(target);
            for (int k = target - 1; k >= std::max(0, target - 3) && slot < 0; --k)
                slot = cache_->slotOf(k);
        }
        if (slot >= 0)
            publishCached(slot);   // the ring full: the next step serves it as a hit
        else
            publishDecodedNow(rel, pts);
        r.target = std::min(r.target, rel);   // the run ends here
    }
    if (rel >= r.target)
        endRun();
}

// ---- s-rta-0929b gopcache c4: forward retention (GC8 / R-9) and the REPOSITION run behind forward hits ----

// A forward frame just published is kept (a copy in the native format) so a flip to reverse -- the reverse flag on a Loop /
// OneShot clip (GC8), a PingPong top turn (R-9) -- is served from the cache while the first DEMAND / PREFETCH run decodes.
// The pool rules keep the newest `forwardRetain()` frames behind the clock; older ones are replaced first.
void VideoPlayer::retainForward(int rel, double pts, const AVFrame* src)
{
    if (intraOnly_ || forwardRetain() <= 0 || rel < 0 || cache_->slotOf(rel) >= 0 || !cache_->storable(src))
        return;
    const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
    const int n = nTraj();
    const int cur = relOf(wantTime_.load(std::memory_order_acquire));
    const int capS = capSlots();
    const int retain = forwardRetain();
    refreshView(cur, true);
    const auto pool = GopCache::poolOf(mode, true, n, cur, rel, retain);
    const int key = GopCache::keyOf(mode, true, n, cur, rel, pool);
    const auto v = GopCache::judgeStore(cache_->view(), capS, GopCache::behindCapFor(mode, true, capS, retain), rel, key,
                                        pool, rel, -1);
    if (v.keep == GopCache::Keep::Drop || !cache_->store(src, rel, pts, v, GopCache::Slot{ rel, key, pool }, floorBytes()))
        if (stats_ != nullptr)
            ++stats_->gopCacheDrops;
}

// Forward hits are being served: bring the decoder to the frame after the cache's contiguous top (one decode per step),
// storing that frame -- it becomes the next hit, and the decoder then outputs the one after it (no seek at the switch).
bool VideoPlayer::forwardIdle(double want)
{
    (void) want;
    if (!hitsArmed_ || !haveNewest_ || cache_->slots() == 0)
        return false;
    const int nextRel = relOf(newestPts_) + 1;
    if (cache_->slotOf(nextRel) < 0)
        return false;   // no hits ahead: the plain forward decode serves the next frame
    int top = nextRel;
    while (cache_->slotOf(top + 1) >= 0)
        ++top;
    const int target = top + 1;
    if (target >= nTraj() || (haveDecoded_ && lastDecodedRel_ == top))
        return false;   // the end, or the decoder is in place (its next output is `target`)
    if (!repoSeeked_ && (!haveDecoded_ || lastDecodedRel_ >= target || target - lastDecodedRel_ > gopFramesEst_))
    {
        seekToTimestamp(ptsOfRel(target));
        haveDecoded_ = false;
        repoSeeked_ = true;
        if (stats_ != nullptr)
            ++stats_->gopCacheRuns;
    }
    codecCtx_->skip_frame = AVDISCARD_DEFAULT;
    if (!decodeNextFrame())
    {
        if (atEof_)
            lastRel_ = std::max(lastRel_, lastDecodedRel_);
        return false;
    }
    double pts = 0.0;   // gopcache-fix2 R1: the frame's own index time, never the target's
    if (!indexTimeOf(decodedFrame_, &pts))
    {
        if (stats_ != nullptr)
        {
            ++stats_->framesDecoded;
            ++stats_->gopCacheRunDecodes;
            ++stats_->framesDropped;
        }
        return true;   // no index: not stored (the plain forward decode serves what the cache cannot)
    }
    const int rel = relOf(pts);
    lastDecodedPts_ = pts;
    lastDecodedRel_ = rel;
    haveDecoded_ = true;
    if (stats_ != nullptr)
    {
        ++stats_->framesDecoded;
        ++stats_->gopCacheRunDecodes;
    }
    if (rel >= target && cache_->slotOf(rel) < 0 && cache_->storable(decodedFrame_))
    {
        const auto mode = cacheModeOf(loopMode_.load(std::memory_order_relaxed));
        const int cur = relOf(wantTime_.load(std::memory_order_acquire));
        const int capS = capSlots();
        const int retain = forwardRetain();
        refreshView(cur, true);
        const auto pool = GopCache::poolOf(mode, true, nTraj(), cur, rel, retain);
        const int key = GopCache::keyOf(mode, true, nTraj(), cur, rel, pool);
        const auto v = GopCache::judgeStore(cache_->view(), capS, GopCache::behindCapFor(mode, true, capS, retain), rel,
                                            key, pool, rel, -1);
        if (v.keep == GopCache::Keep::Drop
            || !cache_->store(decodedFrame_, rel, pts, v, GopCache::Slot{ rel, key, pool }, floorBytes()))
            if (stats_ != nullptr)
                ++stats_->gopCacheDrops;
    }
    else if (stats_ != nullptr)
        ++stats_->framesDropped;
    return true;
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
    firstPacketSinceSeek_ = true;   // gop2 R3: the next video packet read is this seek's landing
    landedOnKeyPacket_ = false;
    landingPts_ = AV_NOPTS_VALUE;

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
        if (firstPacketSinceSeek_)   // gop2 R3: the seek's landing packet (written here, read only by a run's store gate)
        {
            firstPacketSinceSeek_ = false;
            landedOnKeyPacket_ = (packet_->flags & AV_PKT_FLAG_KEY) != 0;
            landingPts_ = packet_->pts;
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

void VideoPlayer::convertInto(const AVFrame* src, int slot)
{
    // Bottom-up (GL order) in one pass: the destination starts at the slot's last row with a negative stride (sws
    // honours it -- plan-video F14, re-verified at 320x180 / 1080p / 4K for yuv420p and yuv422p10le). No flip, no
    // allocation.
    // s-rta-0929 vupload P3: an IOSurface slot is written under its lock (the CPU-write / GPU-read protocol).
    // s-rta-0929b gopcache: `src` = the decoded frame, the pending frame or a GOP-cache copy -- the only slot writer.
#if JUCE_MAC
    auto* surface = static_cast<IOSurfaceRef>(surf_[static_cast<size_t>(slot)]);
    if (surface != nullptr)
        IOSurfaceLock(surface, 0, nullptr);
#endif
    uint8_t* dst[4] = { slotBytes_[static_cast<size_t>(slot)] + static_cast<size_t>(height_ - 1) * static_cast<size_t>(rowBytes_),
                        nullptr, nullptr, nullptr };
    int dstStride[4] = { -rowBytes_, 0, 0, 0 };
    sws_scale(swsCtx_, src->data, src->linesize, 0, height_, dst, dstStride);
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
