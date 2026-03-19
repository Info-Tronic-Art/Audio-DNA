#pragma once
#include <juce_core/juce_core.h>
#include <juce_opengl/juce_opengl.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

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
// Threading model:
//   - open()/close() called from message thread
//   - advanceFrame()/uploadToTexture() called from GL thread each frame
//   - Transport state (speed, reverse, loop) set from message thread via atomics
//
// The player decodes one frame ahead and holds it in a CPU-side buffer.
// uploadToTexture() pushes the decoded frame to a GL texture (glTexSubImage2D).
class VideoPlayer
{
public:
    VideoPlayer();
    ~VideoPlayer();

    // Open a video file. Returns true on success. Call from message thread.
    bool open(const juce::File& file);

    // Close and release all resources. Call from message thread.
    void close();

    bool isOpen() const { return open_.load(std::memory_order_relaxed); }
    bool hasAlpha() const { return hasAlpha_.load(std::memory_order_relaxed); }

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

    // Seek to normalized position [0, 1]. Thread-safe.
    void seekTo(double normalizedPosition);

    // Get current playhead position [0, 1]. Thread-safe.
    double getPlayheadPosition() const { return playheadPosition_.load(std::memory_order_relaxed); }

    // === Frame Access (GL thread only) ===

    // Advance the playhead by dt seconds (scaled by speed/reverse/loop).
    // Decodes the next frame into the CPU buffer if needed.
    // Call once per render frame from GL thread.
    void advanceFrame(double dt);

    // Upload the current decoded frame to a GL texture.
    // Creates the texture on first call, reuses thereafter.
    // Returns the GL texture ID, or 0 if no frame is ready.
    // Must be called on the GL thread.
    GLuint uploadToTexture();

    // Release the GL texture. Call from openGLContextClosing().
    void releaseGL();

    // Get a thumbnail image (first frame). Call after open(), from message thread.
    juce::Image getThumbnail(int maxWidth, int maxHeight);

    VideoPlayer(const VideoPlayer&) = delete;
    VideoPlayer& operator=(const VideoPlayer&) = delete;

private:
    // FFmpeg state
    AVFormatContext* formatCtx_ = nullptr;
    AVCodecContext* codecCtx_ = nullptr;
    AVFrame* decodedFrame_ = nullptr;
    AVFrame* rgbaFrame_ = nullptr;
    AVPacket* packet_ = nullptr;
    SwsContext* swsCtx_ = nullptr;
    int videoStreamIndex_ = -1;

    // Video properties
    int width_ = 0;
    int height_ = 0;
    double duration_ = 0.0;
    double frameRate_ = 30.0;
    int totalFrames_ = 0;
    double timeBase_ = 0.0;   // Stream time base in seconds per tick

    // CPU frame buffer (RGBA or RGB, flipped for OpenGL)
    std::vector<uint8_t> frameBuffer_;
    int frameBufferWidth_ = 0;
    int frameBufferHeight_ = 0;
    bool frameReady_ = false;

    // GL texture
    GLuint texture_ = 0;
    bool textureCreated_ = false;

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

    // Current decode position in seconds
    double currentTime_ = 0.0;
    bool pingPongForward_ = true;

    // Decode the frame at the current time position
    bool decodeFrameAtTime(double timeSec);

    // Seek to a specific timestamp in the stream
    bool seekToTimestamp(double timeSec);

    // Decode the next frame from the stream
    bool decodeNextFrame();

    // Convert decoded frame to RGBA and store in frameBuffer_
    void convertFrameToRGBA();

    // Mutex for protecting FFmpeg state during open/close
    std::mutex ffmpegMutex_;
};
