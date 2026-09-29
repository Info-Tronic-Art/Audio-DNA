#pragma once
#include <juce_core/juce_core.h>
#include <juce_opengl/juce_opengl.h>
#include <array>
#include <string>
#include <atomic>
#include <memory>
#include "media/VideoRing.h"
#include "media/VideoStats.h"
#include "media/VideoUploadBudget.h"

// Forward declarations for FFmpeg types (C linkage)
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;

// VideoPlayer: Decodes video files via FFmpeg and uploads frames to OpenGL textures.
//
// Supports: MP4, MOV, QuickTime (H.264/H.265/ProRes), HAP/HAP Alpha, AVI.
// Alpha: HAP Alpha provides full RGBA. Other codecs may or may not have alpha.
//
// Threading model (s-rta-0928b video: no video frame is decoded, converted or flipped on the GL thread):
//   - message thread: open() (the FFmpeg contexts, the ring's 3 slots, frame 0 into slot 0, the thumbnail),
//     start() (the decode thread), close() (signals the thread, returns at once), getThumbnail(), and the
//     transport setters (atomics; seekTo from any thread is a request).
//   - GL thread: advanceFrame() / advanceClock() (the transport clock; a seek or Loop wrap bumps the request
//     generation), uploadToTexture() (picks the newest ring frame <= the clock, uploads it only when it is new,
//     never waits), releaseGL(), neverShown().
//   - decode thread (after start()): everything FFmpeg -- seek, decode, sws into a ring slot. It owns the contexts
//     and frees them itself on exit; drainRetiredMedia destroys a retired player only once it has exited.
class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    // Open a video file. Returns true on success. Call once per player (every open is a new player), before start():
    // from the message thread, or from MediaOpener's pool thread on an unpublished player (nothing else may hold it;
    // s-rta-0929 asyncload, adoption AL4). Starts NO thread: the whole synchronous prepare touches only this object (the
    // part-M seam).
    bool open(const juce::File& file);

    // Start the decode thread (Renderer::installVideoPlayer, after the player is in the map). Message thread.
    void start();

    // Stop decoding: signals the decode thread and returns at once (the thread frees the FFmpeg contexts itself);
    // a never-started player frees them here. Call from message thread.
    void close();

    // No decode thread runs (never started, or exited) -- drainRetiredMedia's destroy gate.
    bool threadDone() const { return !thread_.isThreadRunning(); }

    bool isOpen() const { return open_.load(std::memory_order_relaxed); }
    bool hasAlpha() const { return hasAlpha_.load(std::memory_order_relaxed); }

    // The file this player last opened (empty if never opened). Lets callers
    // detect an id-stable content swap (same clip id, different media file).
    // Unlocked read: safe only because every caller is message-thread confined
    // (undo/redo media hooks, drop handlers); would race sourceFile_ writes in
    // open() if ever called off the message thread.
    juce::File getFile() const { return sourceFile_; }

    // Video properties (valid after open)
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    double getDuration() const { return duration_; }
    double getFrameRate() const { return frameRate_; }
    int getTotalFrames() const { return totalFrames_; }

    // === Transport ===

    enum class LoopMode : uint8_t { Loop, PingPong, OneShot };

    void setSpeed(float speed) { speed_.store(speed, std::memory_order_relaxed); }
    float getSpeed() const { return speed_.load(std::memory_order_relaxed); }

    void setReverse(bool rev) { reverse_.store(rev, std::memory_order_relaxed); }
    bool getReverse() const { return reverse_.load(std::memory_order_relaxed); }

    void setLoopMode(LoopMode mode) { loopMode_.store(mode, std::memory_order_relaxed); }
    LoopMode getLoopMode() const { return loopMode_.load(std::memory_order_relaxed); }

    void setPlaying(bool playing) { playing_.store(playing, std::memory_order_relaxed); }
    bool isPlaying() const { return playing_.load(std::memory_order_relaxed); }

    // Seek to normalized position [0, 1]. Thread-safe (a request the GL thread applies to its clock).
    void seekTo(double normalizedPosition);

    // Get current playhead position [0, 1]. Thread-safe.
    double getPlayheadPosition() const { return playheadPosition_.load(std::memory_order_relaxed); }

    // === Frame Access (GL thread only) ===

    // Advance the playhead by dt seconds (scaled by speed/reverse/loop), post the wanted time to the decode thread
    // (a seek or a Loop wrap bumps the request generation) and wake it when it needs to run. Never decodes.
    void advanceFrame(double dt);

    // s-rta-0926b plan4 T3 (rule 15): advance the playhead exactly like advanceFrame() -- same transport math -- but
    // post nothing and wake nothing: for a clip whose deck is not on screen the decode thread idles, and the next
    // advanceFrame() makes it re-seek and catch up while the layer holds its last frame. A seek or wrap still bumps
    // the request generation (the ring's frames are stale). GL thread.
    void advanceClock(double dt);

    // Pick the newest ring frame with pts <= the clock (+ half a frame) of the current request generation and upload
    // it when it is new (glTexImage2D once, then glTexSubImage2D). Nothing picked: the texture of the last shown frame
    // (a HOLD; *pending = false), or 0 with *pending = true when this player has never shown a frame -- unless its first
    // frame FAILED (ADDENDUM W3: the decode thread gave up before any frame, or none came within kFirstFrameTimeoutMs
    // of the first call): then 0 with *pending = false, "no media". Never waits. Must be called on the GL thread.
    // s-rta-0929 vupload P1: with a budget, a NEW frame asks budget->admit() BEFORE the pick (VideoRing::peek); refused
    // = a HOLD of the shown frame (*pending false, the ring untouched), asked again next frame, force-admitted after
    // VideoUpload::maxDefer(frame duration / |speed|, renderDt) render frames. nullptr = no budget (ctests, tools).
    GLuint uploadToTexture(bool* pending, VideoUpload::Budget* budget = nullptr, double renderDt = 0.0);

    // GL thread: no frame uploaded yet and not FAILED (the C1 crossfade pause provider). A GL release does not make it
    // true again.
    bool neverShown() const { return open_.load(std::memory_order_relaxed) && !shown_.everShown && !firstFrameFailed_; }

    // Release the GL texture. Call from openGLContextClosing() / drainRetiredMedia() (GL thread).
    void releaseGL();

    // A thumbnail image (the first frame, made in open()). Call after open(), from message thread.
    juce::Image getThumbnail(int maxWidth, int maxHeight);

    // s-rta-0928b video: the Renderer's counters (/api/state). nullptr = none (the default). Before start().
    void setStats(VideoStats* s) { stats_ = s; }

    // s-rta-0929 vupload P4b (plan R-14): GL thread, the frame top (Renderer::scanVideoIdle). A shown player not drawn for
    // kTrimIdleMs drops its Ready slots (the reader's) and asks its parked decode thread (the writer) to purge its Free
    // ones (IOSurfaceSetPurgeable Empty; free() on the malloc path); the writer un-purges a slot when it next takes it.
    // The held slot (the frame on screen) is never purged. Once per idle spell (a draw re-arms it).
    void trimIfIdle(int64_t nowMs);
    static constexpr int64_t kTrimIdleMs = 1000;
    static int64_t nowMs();

    // s-rta-0929 vupload P3: how a new frame reaches texture_. Blit = the ring slots are IOSurfaces (BGRA), each bound once
    // to a GL_TEXTURE_RECTANGLE read FBO; one glBlitFramebuffer into texture_, fenced (macOS). Client = IOSurface slots,
    // uploaded with glTexSubImage2D (a player whose blit setup failed). Malloc = malloc'd RGBA slots + glTexSubImage2D
    // (non-Apple builds, a failed IOSurfaceCreate). Decided in open().
    enum class UploadPath : uint8_t { Blit, Client, Malloc };

    VideoPlayer(const VideoPlayer&) = delete;
    VideoPlayer& operator=(const VideoPlayer&) = delete;

private:
    friend struct VideoPlayerTestAccess;   // tests/test_video_player_gl.cpp: the test is the writer (no decode thread)

    // The most recently opened file (for id-stable content-swap detection).
    juce::File sourceFile_;

    // FFmpeg state -- the message thread's in open() (and in close() for a never-started player), the decode
    // thread's after start().
    AVFormatContext* formatCtx_ = nullptr;
    AVCodecContext* codecCtx_ = nullptr;
    AVFrame* decodedFrame_ = nullptr;
    AVPacket* packet_ = nullptr;
    SwsContext* swsCtx_ = nullptr;
    int videoStreamIndex_ = -1;

    // Video properties (written in open(), read-only after)
    int width_ = 0;
    int height_ = 0;
    int rowBytes_ = 0;
    double duration_ = 0.0;
    double frameRate_ = 30.0;
    double frameDur_ = 1.0 / 30.0;
    int totalFrames_ = 0;
    double timeBase_ = 0.0;   // Stream time base in seconds per tick

    // The ring: 3 slots of width x height pixels, each written bottom-up (GL order) by sws_scale with a negative
    // destination stride (rowBytes_ apart); the headers are the lock-free protocol. s-rta-0929 vupload P3: on macOS the
    // slots are BGRA IOSurfaces (surf_, base addresses in slotBytes_, released in the destructor); otherwise RGBA
    // malloc'd blocks (freed in the destructor).
    static constexpr int kSlots = 3;
    std::array<uint8_t*, kSlots> slotBytes_{};
    VideoRing::Ring<kSlots> ring_;
    UploadPath path_ = UploadPath::Malloc;
    UploadPath forcePath_ = UploadPath::Blit;   // TEST-ONLY lever (ADNA_VIDEO_FORCE_FALLBACK / VideoPlayerTestAccess)
    bool fallbackCounted_ = false;
    std::array<void*, kSlots> surf_{};          // IOSurfaceRef (macOS; void* keeps IOSurface out of this header)
    // GL thread: per slot a GL_TEXTURE_RECTANGLE bound to its IOSurface + a read FBO on it, the blit's fence (GLsync);
    // one draw FBO on texture_. Created lazily in uploadSlot, every handle zeroed by releaseGL (a sync dies with its
    // context -- dropped, never deleted there).
    std::array<GLuint, kSlots> rectTex_{}, readFbo_{};
    std::array<void*, kSlots> fence_{};
    GLuint dstFbo_ = 0;
    unsigned (*fenceWaitOverride_)(void*) = nullptr;   // ctest seam (a GL_WAIT_FAILED fence); nullptr = glClientWaitSync
    // P4b: the idle trim. trimmed_ (GL thread) = this idle spell is trimmed; trimRequested_ (GL -> decode thread) = purge
    // the Free slots; purged_ (decode thread only: only the writer leaves Free); rebind_ (decode -> GL thread, set before
    // the slot's publish): the slot was un-purged -- re-run CGLTexImageIOSurface2D before its next blit (adoption VU1).
    bool trimmed_ = false;
    std::atomic<bool> trimRequested_{ false };
    std::array<bool, kSlots> purged_{};
    std::array<std::atomic<bool>, kSlots> rebind_{};

    // GL texture + GL-thread-only picking state
    GLuint texture_ = 0;
    bool textureCreated_ = false;
    VideoRing::ShownState shown_;      // V1: everShown survives releaseGL
    double lastShownPts_ = -1.0;
    int64_t firstDrawMs_ = -1;         // W3: the first uploadToTexture call (-1 = never drawn)
    bool firstFrameFailed_ = false;    // W3: VideoRing::firstFrameFailed, re-judged while nothing has been shown
    bool releasedThisFrame_ = false;
    bool discontinuity_ = false;       // a Loop wrap inside advanceTransport (-> a generation bump)
    int deferredFrames_ = 0;           // s-rta-0929 vupload P1: render frames the ready frame has been held by the budget
    uint32_t shownGen_ = 0;            // P1 / VU8: the request generation of the last uploaded frame (a new one is exempt)
    // s-rta-0929 vupload P4a: the slot of the frame ON SCREEN stays Reading until a newer frame is shown (retire_.held),
    // so after a GL context loss the first draw re-uploads it -- a shown player never returns texture 0.
    VideoRing::Retire<kSlots> retire_;

    // Transport state (atomics for cross-thread access)
    std::atomic<bool> open_{false};
    std::atomic<bool> hasAlpha_{false};
    std::atomic<float> speed_{1.0f};
    std::atomic<bool> reverse_{false};
    std::atomic<LoopMode> loopMode_{LoopMode::Loop};
    std::atomic<bool> playing_{true};
    std::atomic<double> playheadPosition_{0.0};  // [0, 1]

    // Seek request
    std::atomic<bool> seekRequested_{false};
    std::atomic<double> seekTarget_{0.0};

    // GL -> decode thread: the wanted time, the request generation (a seek / wrap = a discontinuity), the last draw.
    std::atomic<double> wantTime_{0.0};
    std::atomic<uint32_t> gen_{0};
    std::atomic<int64_t> lastDrawMs_{0};

    // decode thread -> GL thread (W3): the decode thread reached EOF, or a decode error, before any frame decoded.
    std::atomic<bool> firstFrameGaveUp_{false};

    // Current transport position in seconds (GL thread)
    double currentTime_ = 0.0;
    bool pingPongForward_ = true;

    // Decode-thread-only state
    double newestPts_ = -1.0;          // the newest PUBLISHED frame (decide()'s reference)
    bool haveNewest_ = false;
    double lastDecodedPts_ = -1.0;     // the newest DECODED frame (the NONREF predicate during a catch-up)
    bool haveDecoded_ = false;
    uint32_t myGen_ = 0;
    uint64_t seq_ = 0;
    bool drained_ = false;
    bool atEof_ = false;               // decodeNextFrame() stopped at the end of the stream (not a decode error)
    bool everDecoded_ = false;         // a frame of this file ever decoded (open()'s frame 0 included) -- W3

    juce::Image thumbnail_;            // made in open() (message thread)

    VideoStats* stats_ = nullptr;

    // R-13 levers (named, each with its trigger in plan-video.md): V5 -- NONREF skipping in a catch-up is ON.
    static constexpr bool kSkipNonRefInCatchUp = true;
    static constexpr juce::Thread::Priority kDecodeThreadPriority = juce::Thread::Priority::normal;

    struct DecodeThread final : juce::Thread
    {
        explicit DecodeThread(VideoPlayer& o) : juce::Thread("VideoDecode"), owner(o) {}
        void run() override { owner.decodeLoop(); }
        VideoPlayer& owner;
    };
    DecodeThread thread_{ *this };

    // plan4 T3: the transport math shared by advanceFrame() and advanceClock() -- speed / reverse / loop /
    // ping-pong / one-shot, the playhead store. A Loop wrap sets discontinuity_ (s-rta-0928b: was a demuxer seek on
    // the GL thread). Returns false when the clock did not run (not playing, or no duration).
    bool advanceTransport(double dt);

    // Decode thread
    void decodeLoop();
    void onDecoded(uint32_t gen, const VideoRing::Policy& pol);   // drop, or convert into a slot and publish
    void drainDecoder(uint32_t gen, const VideoRing::Policy& pol); // EOF: the frames frame-threading held back
    void park();                                                   // wait until notified (threadsAwake accounting)
    void noteNoFirstFrame(const char* why);                        // W3: EOF / a decode error before any frame
    bool seekToTimestamp(double timeSec);
    bool decodeNextFrame();
    void convertInto(int slot);                                    // sws_scale bottom-up (negative stride) into a slot
    void uploadSlot(int slot);                                     // GL thread: slot -> texture_ (created on first use)
    bool createSurfaces();                                         // open(): 3 BGRA IOSurfaces (macOS)
    bool blitSlot(int slot);                                       // P3: the IOSurface blit; false = fell back to Client
    void clientUploadSurface(int slot);                            // P3: glTexSubImage2D of a slot's BGRA surface bytes
    void ensureTexture();                                          // texture_ as GL_RGBA8 w x h, no data
    void fallBack(const char* why);                                // Blit -> Client (once per player, counted)
    void pollFences();                                             // P3: signaled blits give their slot back
    void purgeFreeSlots();                                         // P4b: decode thread -- purge the Free slots
    bool unpurge(int slot);                                        // P4b: decode thread, after acquireWrite
    void freeFfmpeg();                                             // idempotent
    void makeThumbnail();                                          // open(): the first frame -> <= 90x72
};
