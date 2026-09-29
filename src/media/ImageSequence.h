#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_opengl/juce_opengl.h>
#include "render/ImageDecode.h"
#include "render/ImageTexCache.h"
#include "media/SeqVram.h"
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

// ImageSequence: Treats a folder or set of image files as a video clip.
//
// When the user drags multiple PNGs/JPEGs onto a clip cell, they become
// an image sequence with configurable framerate and full transport controls.
//
// Frames decode off the GL thread and play through a BOUNDED window of recycled GL textures (s-rta-0928b seqvram,
// SeqVram.h): a sequence that fits its allowance keeps every frame, a longer one evicts the frame shown farthest in the
// future -- never the current frame, never the one on screen.
// Supports the same transport model as VideoPlayer: speed, reverse, loop modes.
class ImageSequence
{
public:
    ImageSequence();
    ~ImageSequence();

    // Load images from a list of files. Sorts alphabetically.
    // Returns true if at least one path has an image extension. s-rta-0928b mediaopen: any thread; does NO file I/O
    // (no stat, no decode) -- a file that is missing decodes Failed when its frame comes up and the previous frame repeats.
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

    // s-rta-0928b mediaopen: any thread; a request consumed by the next advanceFrame (GL thread), as VideoPlayer's.
    void seekTo(double normalizedPosition);
    double getPlayheadPosition() const { return playheadPosition_.load(std::memory_order_relaxed); }

    // === Frame Access (GL thread only) ===

    // Advance the playhead by dt seconds. Call once per render frame.
    void advanceFrame(double dt);

    // Get the GL texture for the current frame. s-rta-0928 R1.4: frames decode OFF the GL thread (decoder), with a
    // look-ahead of the next kLookAhead frames in the play direction; the GL thread only uploads, within the frame's
    // upload budget. A current frame not resident yet shows the last frame shown (as a late video frame would);
    // *pending is set only when nothing has been shown yet. A frame that fails to decode is never re-requested (the
    // last frame repeats). Returns 0 if no frame available. s-rta-0928b seqvram: frames live in a bounded window of
    // recycled slots (SeqVram.h) sized by grant.allowanceBytes; the request look-ahead and the eviction order follow
    // the trajectory (SeqVram::distances, with the clip's in/out points from the grant); the grant carries the seq_*
    // counters (SeqVram::Stats) this call bumps. *pending and the late-frame repeat as before.
    GLuint getCurrentTexture(ImageDecode::Decoder& decoder, ImageTexCache::UploadBudget& budget,
                             const SeqVram::Grant& grant, bool* pending);

    // s-rta-0928 renderleft-fix (C1 for sequences): true when getCurrentTexture would report *pending as the state
    // stands -- nothing shown yet and the current frame not resident (and not failed). No decode, no request, no
    // upload: CompositorEngine asks it BEFORE advancing a crossfade onto this sequence (GL thread only).
    bool firstFramePending() const;

    // s-rta-0928b seqvram (GL thread): the texture bytes / textures this sequence holds now (allocated slots), the
    // frame serial of its last draw, its floor in bytes (kMinWindowFrames frames; 0 while the frame size is unknown),
    // and the Renderer's pressure trim: evict all but the current and the shown frame, delete the spare slots.
    size_t residentBytes() const;
    int residentSlots() const;
    uint64_t lastDrawnSerial() const { return lastDrawnSerial_; }
    size_t minWindowBytes() const { return static_cast<size_t>(SeqVram::kMinWindowFrames) * frameBytesHint_; }
    // Fix round F3: the bytes trimToMinimum leaves (current + shown frame) -- an idle sequence's unreclaimable part.
    size_t trimmedBytes() const { return std::min(residentBytes(), 2u * frameBytesHint_); }
    // Fix round F2: deletes at most deletes.left textures (the frame's shared budget); spare slots it could not delete
    // stay allocated (free) for a later call.
    void trimToMinimum(SeqVram::Stats* stats, SeqVram::DeleteBudget& deletes);

    // Release all GL textures. Call from openGLContextClosing().
    void releaseGL();
    // Fix round F2, the retire drain (GL thread): deletes at most deletes.left textures; true once none is left (then
    // the rest of releaseGL's state is cleared too).
    bool releaseGLWithin(SeqVram::DeleteBudget& deletes, SeqVram::Stats* stats);

    // Get the list of loaded files (for serialization/display)
    const std::vector<juce::File>& getFiles() const { return files_; }

    ImageSequence(const ImageSequence&) = delete;
    ImageSequence& operator=(const ImageSequence&) = delete;

private:
    std::vector<juce::File> files_;         // Sorted image files
    std::vector<GLuint> textures_;          // per frame: its slot's GL texture (0 = not resident)
    SeqVram::Slots slots_;                  // s-rta-0928b seqvram: the recycled textures (owns every GL name)
    std::vector<int> dist_;                 // scratch: SeqVram::distances of this frame
    std::vector<uint8_t> residentFlags_;    // scratch: textures_[j] != 0
    size_t frameBytesHint_ = 0;             // w * h * 4 of a frame (set by the first upload); 0 = unknown
    uint64_t lastDrawnSerial_ = 0;          // the Renderer's frame serial of the last getCurrentTexture

    // Transport state
    std::atomic<bool> open_{false};
    std::atomic<float> fps_{10.0f};
    std::atomic<float> speed_{1.0f};
    std::atomic<bool> reverse_{false};
    std::atomic<LoopMode> loopMode_{LoopMode::Loop};
    std::atomic<bool> playing_{true};
    std::atomic<double> playheadPosition_{0.0};
    // s-rta-0928b mediaopen: seekTo's request (any thread) and its target; consumed at the top of advanceFrame.
    std::atomic<bool> seekRequested_{false};
    std::atomic<double> seekTarget_{0.0};

    // Current position in seconds
    double currentTime_ = 0.0;
    bool pingPongForward_ = true;
    int currentFrameIndex_ = 0;

    // s-rta-0928 R1.4: off-GL-thread decode state (GL thread only, like textures_). Jobs deliver into box_ through a
    // weak_ptr (a retired sequence drops them); tag = openGen_ << 32 | frame index (a re-open drops the old ones).
    static constexpr int kMaxOutstanding = SeqVram::kMaxOutstanding;
    std::shared_ptr<ImageDecode::Mailbox> box_ = std::make_shared<ImageDecode::Mailbox>();
    std::vector<ImageDecode::Result> ready_;     // decoded, waiting for upload budget
    std::vector<uint8_t> requested_;             // per frame: a job was issued (or it failed)
    std::vector<uint8_t> failed_;                // per frame: did not decode
    int outstanding_ = 0;                        // requested, result not yet arrived
    int lastShown_ = -1;
    uint32_t openGen_ = 0;
    int lastReturned_ = -1;                      // s-rta-0928b seqvram: the frame index returned last (seq_frames_shown)
    void ensureFrameState();
    void requestFrame(ImageDecode::Decoder& decoder, int idx);
    bool uploadFrame(const ImageDecode::Result& r, int idx, int cap, SeqVram::Stats* stats);   // false: no slot (H1)
    void evictFrame(int idx, SeqVram::Stats* stats);
    void deleteTextures(const std::vector<uint32_t>& gone, SeqVram::DeleteBudget* deletes, SeqVram::Stats* stats);
    SeqVram::Transport transport(int cur, const SeqVram::Grant& grant) const;
};
