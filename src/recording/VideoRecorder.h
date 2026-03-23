#pragma once

#include <juce_core/juce_core.h>
#include <juce_opengl/juce_opengl.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <vector>
#include <functional>

// Forward declarations — FFmpeg
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct SwsContext;

// VideoRecorder: Real-time video capture from the GL framebuffer.
//
// Threading model:
//   - GL thread: calls submitFrame() after rendering, which does glReadPixels
//     into a CPU-side buffer (triple-buffered to avoid stalls).
//   - Encoder thread: picks up completed pixel buffers, converts to YUV420p
//     via sws_scale, encodes via FFmpeg, writes to the output container.
//
// Supported codecs:
//   - H.264 (libx264) — best compatibility
//   - ProRes (prores_ks) — best quality for editing
//   - MJPEG — fast encoding, large files
//
// The recorder only captures video. Audio is NOT included in the recording
// (session recording captures the event stream separately).
class VideoRecorder
{
public:
    VideoRecorder();
    ~VideoRecorder();

    // Codec selection
    enum class Codec { H264, ProRes, MJPEG };

    // Configuration
    struct Config
    {
        Codec codec = Codec::H264;
        int width = 1920;
        int height = 1080;
        int fps = 30;
        int quality = 23;       // CRF for H264 (0-51, lower=better), quality for ProRes (0-5)
        std::string container;  // "mp4", "mov" — auto-selected based on codec if empty
    };

    // Start recording to a file. Call from message thread.
    // Returns true on success.
    bool startRecording(const juce::File& outputFile, const Config& config);

    // Stop recording. Blocks until the encoder thread finishes flushing.
    void stopRecording();

    // Submit a frame from the GL thread. Reads the current framebuffer
    // via glReadPixels. Call this at the end of renderOpenGL().
    // Does nothing if not recording.
    void submitFrame(int framebufferWidth, int framebufferHeight);

    // State queries
    bool isRecording() const { return state_.load(std::memory_order_relaxed) == State::Recording; }
    double getRecordedDuration() const;
    int getRecordedFrameCount() const { return framesWritten_.load(std::memory_order_relaxed); }
    int getDroppedFrameCount() const { return droppedFrames_.load(std::memory_order_relaxed); }

    // Callback: fired on message thread when recording stops (including on error)
    std::function<void(bool success, const juce::File& file)> onRecordingFinished;

    VideoRecorder(const VideoRecorder&) = delete;
    VideoRecorder& operator=(const VideoRecorder&) = delete;

private:
    enum class State { Idle, Recording, Stopping };
    std::atomic<State> state_{State::Idle};

    // Encoder thread
    void encoderThreadFunc();
    std::thread encoderThread_;
    std::mutex encoderMutex_;
    std::condition_variable encoderCV_;

    // Triple-buffered pixel readback
    static constexpr int kNumBuffers = 3;
    struct PixelBuffer
    {
        std::vector<uint8_t> data;
        int width = 0;
        int height = 0;
        std::atomic<bool> ready{false};
    };
    std::array<PixelBuffer, kNumBuffers> pixelBuffers_;
    std::atomic<int> writeIndex_{0};  // GL thread writes here
    std::atomic<int> readIndex_{0};   // Encoder thread reads here

    // FFmpeg context
    AVFormatContext* formatCtx_ = nullptr;
    AVCodecContext* codecCtx_ = nullptr;
    AVStream* videoStream_ = nullptr;
    AVFrame* frame_ = nullptr;
    AVFrame* rgbFrame_ = nullptr;
    AVPacket* packet_ = nullptr;
    SwsContext* swsCtx_ = nullptr;

    Config config_;
    juce::File outputFile_;
    std::atomic<int> framesWritten_{0};
    std::atomic<int> droppedFrames_{0};
    int64_t pts_ = 0;

    // FFmpeg setup/teardown
    bool initEncoder();
    void closeEncoder();
    bool encodeFrame(const uint8_t* rgbaData, int width, int height);
    bool flushEncoder();
};
