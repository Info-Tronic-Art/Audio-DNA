#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_opengl/juce_opengl.h>
#include "render/ImageDecode.h"
#include "render/ImageTexCache.h"
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

// ImageSequence: Treats a folder or set of image files as a video clip.
//
// When the user drags multiple PNGs/JPEGs onto a clip cell, they become
// an image sequence with configurable framerate and full transport controls.
//
// Each image is loaded into a GL texture on first access and cached.
// Supports the same transport model as VideoPlayer: speed, reverse, loop modes.
class ImageSequence
{
public:
    ImageSequence();
    ~ImageSequence();

    // Load images from a list of files. Sorts alphabetically.
    // Returns true if at least one image was loaded. Call from message thread.
    bool open(const std::vector<juce::File>& imageFiles);

    // Load all images from a directory (PNG/JPG/JPEG/BMP/TIFF).
    bool openDirectory(const juce::File& directory);

    void close();

    bool isOpen() const { return open_.load(std::memory_order_relaxed); }
    int getFrameCount() const { return static_cast<int>(files_.size()); }

    // Configurable frames per second (default 10 fps for image sequences)
    void setFps(float fps) { fps_.store(std::max(0.1f, fps), std::memory_order_relaxed); }
    float getFps() const { return fps_.load(std::memory_order_relaxed); }

    double getDuration() const;
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // === Transport (same interface as VideoPlayer) ===

    enum class LoopMode : uint8_t { Loop, PingPong, OneShot };

    void setSpeed(float speed) { speed_.store(speed, std::memory_order_relaxed); }
    float getSpeed() const { return speed_.load(std::memory_order_relaxed); }

    void setReverse(bool rev) { reverse_.store(rev, std::memory_order_relaxed); }
    bool getReverse() const { return reverse_.load(std::memory_order_relaxed); }

    void setLoopMode(LoopMode mode) { loopMode_.store(mode, std::memory_order_relaxed); }
    LoopMode getLoopMode() const { return loopMode_.load(std::memory_order_relaxed); }

    void setPlaying(bool playing) { playing_.store(playing, std::memory_order_relaxed); }
    bool isPlaying() const { return playing_.load(std::memory_order_relaxed); }

    void seekTo(double normalizedPosition);
    double getPlayheadPosition() const { return playheadPosition_.load(std::memory_order_relaxed); }

    // === Frame Access (GL thread only) ===

    // Advance the playhead by dt seconds. Call once per render frame.
    void advanceFrame(double dt);

    // Get the GL texture for the current frame. s-rta-0928 R1.4: frames decode OFF the GL thread (decoder), with a
    // look-ahead of the next kLookAhead frames in the play direction; the GL thread only uploads, within the frame's
    // upload budget. A current frame not resident yet shows the last frame shown (as a late video frame would);
    // *pending is set only when nothing has been shown yet. A frame that fails to decode is never re-requested (the
    // last frame repeats). Returns 0 if no frame available.
    GLuint getCurrentTexture(ImageDecode::Decoder& decoder, ImageTexCache::UploadBudget& budget, bool* pending);

    // Release all GL textures. Call from openGLContextClosing().
    void releaseGL();

    // Get the list of loaded files (for serialization/display)
    const std::vector<juce::File>& getFiles() const { return files_; }

    // Get a thumbnail from the first frame
    juce::Image getThumbnail(int maxWidth, int maxHeight);

    ImageSequence(const ImageSequence&) = delete;
    ImageSequence& operator=(const ImageSequence&) = delete;

private:
    std::vector<juce::File> files_;         // Sorted image files
    std::vector<GLuint> textures_;          // GL texture per frame (0 = not loaded)
    std::vector<int> textureWidths_;        // Width of each loaded texture
    std::vector<int> textureHeights_;       // Height of each loaded texture

    int width_ = 0;                         // Width of first image (representative)
    int height_ = 0;

    // Transport state
    std::atomic<bool> open_{false};
    std::atomic<float> fps_{10.0f};
    std::atomic<float> speed_{1.0f};
    std::atomic<bool> reverse_{false};
    std::atomic<LoopMode> loopMode_{LoopMode::Loop};
    std::atomic<bool> playing_{true};
    std::atomic<double> playheadPosition_{0.0};

    // Current position in seconds
    double currentTime_ = 0.0;
    bool pingPongForward_ = true;
    int currentFrameIndex_ = 0;

    // s-rta-0928 R1.4: off-GL-thread decode state (GL thread only, like textures_). Jobs deliver into box_ through a
    // weak_ptr (a retired sequence drops them); tag = openGen_ << 32 | frame index (a re-open drops the old ones).
    static constexpr int kLookAhead = 3;
    static constexpr int kMaxOutstanding = 4;
    std::shared_ptr<ImageDecode::Mailbox> box_ = std::make_shared<ImageDecode::Mailbox>();
    std::vector<ImageDecode::Result> ready_;     // decoded, waiting for upload budget
    std::vector<uint8_t> requested_;             // per frame: a job was issued (or it failed)
    std::vector<uint8_t> failed_;                // per frame: did not decode
    int outstanding_ = 0;                        // requested, result not yet arrived
    int lastShown_ = -1;
    uint32_t openGen_ = 0;
    void ensureFrameState();
    void requestFrame(ImageDecode::Decoder& decoder, int idx);
    void requestAhead(ImageDecode::Decoder& decoder, int idx);
    GLuint uploadFrame(const ImageDecode::Result& r, int idx);
};
