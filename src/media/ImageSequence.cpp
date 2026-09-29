// ImageSequence.cpp — Treats a set of images as a playable video clip.

#include "ImageSequence.h"
#include <algorithm>
#include <iostream>
#include <cstring>
#include <limits>

using namespace juce::gl;

ImageSequence::ImageSequence() = default;

ImageSequence::~ImageSequence()
{
    close();
}

bool ImageSequence::open(const std::vector<juce::File>& imageFiles)
{
    close();

    if (imageFiles.empty())
        return false;

    // Filter to supported image formats and sort alphabetically. s-rta-0928b mediaopen: NO stat -- a missing file is a
    // frame that decodes Failed (the previous frame repeats, renderleft R-7) instead of being dropped from the list (which
    // shifted the timing of every later frame); open() does no file I/O at all (it ran on the message thread: 300 stats
    // + a frame-0 decode per 300-frame sequence).
    files_.clear();
    for (const auto& f : imageFiles)
    {
        auto ext = f.getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
            ext == ".bmp" || ext == ".tiff" || ext == ".gif")
        {
            files_.push_back(f);
        }
    }

    if (files_.empty())
        return false;

    // Sort alphabetically for consistent ordering
    std::sort(files_.begin(), files_.end(), [](const juce::File& a, const juce::File& b) {
        return a.getFileName().compareNatural(b.getFileName()) < 0;
    });

    // Initialize texture tracking (s-rta-0928b seqvram: slots are GL objects, never touched here)
    textures_.assign(files_.size(), 0);
    // s-rta-0928 R1.4: a new open -- results of earlier requests are dropped by their generation
    ++openGen_;
    requested_.assign(files_.size(), 0);
    failed_.assign(files_.size(), 0);
    ready_.clear();
    lastShown_ = -1;
    // s-rta-0928b mediaopen: no frame-0 decode here -- frameBytesHint_ stays "unknown" (0: the window's floor, seqvram
    // H5) until the first upload sets it; nothing read width / height.

    seekRequested_.store(false, std::memory_order_relaxed);
    currentTime_ = 0.0;
    currentFrameIndex_ = 0;
    pingPongForward_ = true;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
    playing_.store(true, std::memory_order_relaxed);
    open_.store(true, std::memory_order_relaxed);

    std::cerr << "[ImageSequence] Opened " << files_.size() << " images"
              << " at " << fps_.load() << " fps" << std::endl;

    return true;
}

bool ImageSequence::openDirectory(const juce::File& directory)
{
    if (!directory.isDirectory())
        return false;

    std::vector<juce::File> images;
    for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*.png;*.jpg;*.jpeg;*.bmp;*.tiff;*.gif"))
        images.push_back(entry.getFile());

    return open(images);
}

void ImageSequence::close()
{
    open_.store(false, std::memory_order_relaxed);
    // Note: GL textures must be released on GL thread via releaseGL()
    files_.clear();
    // Don't clear textures_ here — releaseGL() handles that
    seekRequested_.store(false, std::memory_order_relaxed);
    currentTime_ = 0.0;
    currentFrameIndex_ = 0;
    playheadPosition_.store(0.0, std::memory_order_relaxed);
}

double ImageSequence::getDuration() const
{
    float fps = fps_.load(std::memory_order_relaxed);
    if (fps <= 0.0f) return 0.0;
    return static_cast<double>(files_.size()) / static_cast<double>(fps);
}

void ImageSequence::seekTo(double normalizedPosition)
{
    // s-rta-0928b mediaopen: a REQUEST (VideoPlayer's shape), consumed by advanceFrame on the GL thread -- the message-
    // thread callers (cue jump, beat snap, retrigger) used to write currentTime_ / currentFrameIndex_ while the GL thread
    // read and wrote them in advanceFrame / getCurrentTexture. The last request before an advance wins.
    seekTarget_.store(std::clamp(normalizedPosition, 0.0, 1.0), std::memory_order_relaxed);
    seekRequested_.store(true, std::memory_order_release);
}

void ImageSequence::advanceFrame(double dt)
{
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return;

    // s-rta-0928b mediaopen: a pending seek lands here, BEFORE the playing check (a cue jump on a paused sequence moves
    // the frame; the clock-only off-screen path calls advanceFrame too, so an off-screen seek lands as well). The GL
    // thread's own out-point wrap (Renderer::syncMedia) now lands one frame later -- video's behaviour.
    if (seekRequested_.exchange(false, std::memory_order_acq_rel))
    {
        const double p = seekTarget_.load(std::memory_order_relaxed);
        currentTime_ = p * getDuration();
        currentFrameIndex_ = std::clamp(static_cast<int>(p * static_cast<double>(files_.size() - 1)),
                                        0, static_cast<int>(files_.size()) - 1);
        playheadPosition_.store(p, std::memory_order_relaxed);
    }

    if (!playing_.load(std::memory_order_relaxed))
        return;

    double dur = getDuration();
    if (dur <= 0.0)
        return;

    float speed = speed_.load(std::memory_order_relaxed);
    bool reverse = reverse_.load(std::memory_order_relaxed);
    auto loopMode = loopMode_.load(std::memory_order_relaxed);

    double direction = 1.0;
    if (loopMode == LoopMode::PingPong)
        direction = pingPongForward_ ? 1.0 : -1.0;
    if (reverse)
        direction = -direction;

    currentTime_ += dt * static_cast<double>(speed) * direction;

    // Handle boundaries
    if (currentTime_ >= dur)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = std::fmod(currentTime_, dur);
                break;
            case LoopMode::PingPong:
                currentTime_ = dur - (currentTime_ - dur);
                pingPongForward_ = false;
                break;
            case LoopMode::OneShot:
                currentTime_ = dur;
                playing_.store(false, std::memory_order_relaxed);
                break;
        }
    }
    else if (currentTime_ < 0.0)
    {
        switch (loopMode)
        {
            case LoopMode::Loop:
                currentTime_ = dur + std::fmod(currentTime_, dur);
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

    // Calculate current frame index from time
    float fps = fps_.load(std::memory_order_relaxed);
    int frameIdx = static_cast<int>(currentTime_ * static_cast<double>(fps));
    frameIdx = std::clamp(frameIdx, 0, static_cast<int>(files_.size()) - 1);
    currentFrameIndex_ = frameIdx;

    // Update playhead
    double pos = (dur > 0.0) ? (currentTime_ / dur) : 0.0;
    playheadPosition_.store(std::clamp(pos, 0.0, 1.0), std::memory_order_relaxed);
}

void ImageSequence::ensureFrameState()
{
    const size_t n = files_.size();
    if (textures_.size() != n) textures_.assign(n, 0);
    if (requested_.size() != n) requested_.assign(n, 0);
    if (failed_.size() != n) failed_.assign(n, 0);
}

void ImageSequence::requestFrame(ImageDecode::Decoder& decoder, int idx)
{
    const auto i = static_cast<size_t>(idx);
    if (requested_[i] != 0 || textures_[i] != 0 || failed_[i] != 0 || outstanding_ >= kMaxOutstanding)
        return;
    requested_[i] = 1;
    ++outstanding_;
    decoder.request(files_[i], ImageDecode::Layout::StraightRGBA,
                    (static_cast<uint64_t>(openGen_) << 32) | static_cast<uint32_t>(idx), std::nullopt, box_);
}

// s-rta-0928b seqvram: the trajectory SeqVram plans along -- the play direction (reverse and PingPong's leg folded in,
// as the old requestAhead did), the loop mode and the clip's in/out points.
SeqVram::Transport ImageSequence::transport(int cur, const SeqVram::Grant& grant) const
{
    SeqVram::Transport t;
    t.n = static_cast<int>(files_.size());
    t.cur = cur;
    const auto mode = loopMode_.load(std::memory_order_relaxed);
    bool forward = !reverse_.load(std::memory_order_relaxed);
    if (mode == LoopMode::PingPong && !pingPongForward_)
        forward = !forward;
    t.forward = forward;
    t.mode = mode == LoopMode::PingPong ? SeqVram::Mode::PingPong
           : mode == LoopMode::OneShot  ? SeqVram::Mode::OneShot
                                        : SeqVram::Mode::Loop;
    SeqVram::setRange(t, grant.inPoint, grant.outPoint);
    return t;
}

// Into a recycled slot (SeqVram::Slots): the same bytes, format and type as the old one-texture-per-frame upload, so the
// texels are identical (no code changes the pixel-store state). Create = the old GL calls exactly.
bool ImageSequence::uploadFrame(const ImageDecode::Result& r, int idx, int cap, SeqVram::Stats* stats)
{
    const auto a = slots_.acquire(idx, r.w, r.h, cap);
    switch (a.act)
    {
        case SeqVram::Slots::Act::Full:
            // H1: no free slot and none may be created (the floor holds cur + lastShown): not uploaded this frame; the
            // result stays in ready_ and is judged again next frame. F4: the caller checks canAcquire first, so this is
            // defensive only.
            if (stats != nullptr)
                stats->uploadDeferred.fetch_add(1, std::memory_order_relaxed);
            return false;
        case SeqVram::Slots::Act::Create:
        {
            GLuint tex = 0;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, r.rgba.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            slots_.bind(a.slot, tex);
            break;
        }
        case SeqVram::Slots::Act::Respecify:
            glBindTexture(GL_TEXTURE_2D, slots_.texOf(a.slot));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, r.w, r.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, r.rgba.data());
            break;
        case SeqVram::Slots::Act::Reuse:
            glBindTexture(GL_TEXTURE_2D, slots_.texOf(a.slot));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, r.w, r.h, GL_RGBA, GL_UNSIGNED_BYTE, r.rgba.data());
            if (stats != nullptr)
                stats->slotReuses.fetch_add(1, std::memory_order_relaxed);
            break;
    }
    textures_[static_cast<size_t>(idx)] = slots_.texOf(a.slot);
    frameBytesHint_ = static_cast<size_t>(r.w) * static_cast<size_t>(r.h) * 4u;
    if (stats != nullptr)
        stats->uploads.fetch_add(1, std::memory_order_relaxed);
    return true;
}

// The slot becomes free (its VRAM is kept for the next frame's upload); the frame may be requested again.
void ImageSequence::evictFrame(int idx, SeqVram::Stats* stats)
{
    const auto i = static_cast<size_t>(idx);
    slots_.release(idx);
    textures_[i] = 0;
    requested_[i] = 0;
    if (stats != nullptr)
        stats->evictions.fetch_add(1, std::memory_order_relaxed);
}

GLuint ImageSequence::getCurrentTexture(ImageDecode::Decoder& decoder, ImageTexCache::UploadBudget& budget,
                                        const SeqVram::Grant& grant, bool* pending)
{
    if (pending != nullptr)
        *pending = false;
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return 0;
    ensureFrameState();   // also after releaseGL (context loss), which empties the per-frame vectors
    const int n = static_cast<int>(files_.size());
    SeqVram::Stats* const stats = grant.stats;
    lastDrawnSerial_ = grant.frameSerial;

    // 1. The frame to show, read ONCE (H4: seekTo writes currentFrameIndex_ on the message thread -- pre-existing; a
    //    stale read can mis-order one request, but eviction protects lastShown_, GL-thread state, independently, so it
    //    can never evict the frame on screen), its trajectory and this frame's allowance in frames.
    const int cur = std::clamp(currentFrameIndex_, 0, n - 1);
    const SeqVram::Transport t = transport(cur, grant);
    SeqVram::distances(t, dist_);
    const int cap = SeqVram::allowanceFrames(grant.allowanceBytes, frameBytesHint_);

    // 2. Arrived frames of this open (outstanding_ drops at arrival, before any drop -- R11). A result for a frame now
    //    outside the window is dropped unuploaded when keeping it would need an eviction (the frame may be requested
    //    again); under the allowance it is kept (SeqVram::wanted).
    std::vector<ImageDecode::Result> got;
    if (box_->tryDrain(got))
        for (auto& r : got)
        {
            outstanding_ = std::max(0, outstanding_ - 1);
            ready_.push_back(std::move(r));
        }
    int incoming = 0;
    const int residentCount = static_cast<int>(std::count_if(textures_.begin(), textures_.end(),
                                                             [](GLuint tex) { return tex != 0; }));
    for (auto it = ready_.begin(); it != ready_.end();)
    {
        const uint32_t gen = static_cast<uint32_t>(it->tag >> 32);
        const int idx = static_cast<int>(static_cast<uint32_t>(it->tag));
        if (gen != openGen_ || idx < 0 || idx >= n || textures_[static_cast<size_t>(idx)] != 0)
        {
            it = ready_.erase(it);
            continue;
        }
        if (it->kind != ImageTexCache::Kind::Decoded)
        {
            failed_[static_cast<size_t>(idx)] = 1;   // never re-requested: the last frame repeats (R-7)
            it = ready_.erase(it);
            continue;
        }
        if (!SeqVram::wanted(dist_, idx, cur, lastShown_, cap, residentCount + incoming < cap))
        {
            requested_[static_cast<size_t>(idx)] = 0;
            if (stats != nullptr)
                stats->staleDrops.fetch_add(1, std::memory_order_relaxed);
            it = ready_.erase(it);
            continue;
        }
        ++incoming;
        ++it;
    }

    // 3. The plan: evict BEFORE uploading (the farthest-shown frames, never cur / lastShown_), then hand back the slots
    //    a smaller allowance no longer holds.
    residentFlags_.resize(static_cast<size_t>(n));
    for (size_t j = 0; j < static_cast<size_t>(n); ++j)
        residentFlags_[j] = textures_[j] != 0 ? 1 : 0;
    const auto plan = SeqVram::plan(t, dist_, residentFlags_, requested_, failed_, lastShown_, cap, incoming,
                                    kMaxOutstanding - outstanding_);
    for (int j : plan.evict)
        evictFrame(j, stats);
    //    F2: at most the frame's shared delete budget; the spare slots left over stay free and go on later frames.
    if (slots_.size() > cap)
        deleteTextures(slots_.shrink(cap, grant.deletes != nullptr ? grant.deletes->left : std::numeric_limits<int>::max()),
                       grant.deletes, stats);

    // 4. Upload within the frame's budget (the rest wait, never re-decoded; no slot this frame -> wait too, H1). F4: the
    //    slot is checked BEFORE the upload budget is taken, so a result that has no slot never spends it.
    for (auto it = ready_.begin(); it != ready_.end();)
    {
        const int idx = static_cast<int>(static_cast<uint32_t>(it->tag));
        if (!slots_.canAcquire(idx, cap))
        {
            if (stats != nullptr)
                stats->uploadDeferred.fetch_add(1, std::memory_order_relaxed);
            ++it;
            continue;
        }
        if (!budget.take(it->rgba.size()) || !uploadFrame(*it, idx, cap, stats))
        {
            ++it;
            continue;
        }
        it = ready_.erase(it);
    }

    // 5. Requests: the current frame (demand) and the next SeqVram::kLookAhead trajectory frames.
    for (int j : plan.request)
        requestFrame(decoder, j);

    // 6. The current frame, else the last one shown (a late frame repeats), else pending / no media.
    auto shown = [&](int f) {
        if (f != lastReturned_ && stats != nullptr)
            stats->framesShown.fetch_add(1, std::memory_order_relaxed);
        lastReturned_ = f;
    };
    if (textures_[static_cast<size_t>(cur)] != 0)
    {
        lastShown_ = cur;
        shown(cur);
        return textures_[static_cast<size_t>(cur)];
    }
    if (lastShown_ >= 0 && lastShown_ < n && textures_[static_cast<size_t>(lastShown_)] != 0)
    {
        if (stats != nullptr)
            stats->lateFrames.fetch_add(1, std::memory_order_relaxed);
        shown(lastShown_);
        return textures_[static_cast<size_t>(lastShown_)];   // late frame: the last one repeats
    }
    if (failed_[static_cast<size_t>(cur)] != 0)
        return 0;   // nothing shown and this frame is broken: no media
    if (pending != nullptr)
        *pending = true;
    if (stats != nullptr)
        stats->pendingFrames.fetch_add(1, std::memory_order_relaxed);
    return 0;
}

// s-rta-0928b seqvram: the Renderer's pressure trim of an IDLE sequence (not drawn for SeqVram::kIdleFrames frames: an
// inactive deck or column): every resident frame but the current and the shown one is evicted, the spare slots are
// deleted within the frame's shared budget (F2; the rest stay free until a later trim).
void ImageSequence::trimToMinimum(SeqVram::Stats* stats, SeqVram::DeleteBudget& deletes)
{
    const int n = static_cast<int>(textures_.size());
    if (n == 0)
        return;
    const int cur = std::clamp(currentFrameIndex_, 0, n - 1);
    for (int j = 0; j < n; ++j)
        if (textures_[static_cast<size_t>(j)] != 0 && j != cur && j != lastShown_)
            evictFrame(j, stats);
    deleteTextures(slots_.shrink(2, deletes.left), &deletes, stats);
}

// F2: the textures a slot operation handed back, deleted and counted against the frame's budget (seq_deletes).
void ImageSequence::deleteTextures(const std::vector<uint32_t>& gone, SeqVram::DeleteBudget* deletes, SeqVram::Stats* stats)
{
    for (GLuint tex : gone)
        glDeleteTextures(1, &tex);
    if (deletes != nullptr)
        deletes->spend(gone.size());
    if (stats != nullptr)
        stats->deletes.fetch_add(static_cast<int64_t>(gone.size()), std::memory_order_relaxed);
}

// s-rta-0928b seqvram: the allocated slots (free ones included) = the VRAM this sequence holds.
size_t ImageSequence::residentBytes() const
{
    return slots_.allocatedBytes();
}

int ImageSequence::residentSlots() const
{
    return slots_.size();
}

// The same rule as getCurrentTexture's *pending, read only. The per-frame vectors may still be empty (never drawn, or
// after releaseGL): then nothing is resident and nothing has failed, so a non-empty open sequence is pending.
bool ImageSequence::firstFramePending() const
{
    if (!open_.load(std::memory_order_relaxed) || files_.empty())
        return false;
    const int n = static_cast<int>(files_.size());
    if (static_cast<int>(textures_.size()) != n || static_cast<int>(failed_.size()) != n)
        return true;
    const auto idx = static_cast<size_t>(std::clamp(currentFrameIndex_, 0, n - 1));
    if (textures_[idx] != 0)
        return false;
    if (lastShown_ >= 0 && lastShown_ < n && textures_[static_cast<size_t>(lastShown_)] != 0)
        return false;
    return failed_[idx] == 0;
}

void ImageSequence::releaseGL()
{
    // s-rta-0928b seqvram: the slots own every texture name (textures_ only aliases them); releaseAll empties the
    // table, so a new context never reuses a name of this one (H2).
    for (GLuint tex : slots_.releaseAll())
        glDeleteTextures(1, &tex);
    textures_.clear();
    // s-rta-0928 R1.4: every frame is requested again on the next context (ensureFrameState re-sizes).
    requested_.clear();
    ready_.clear();
    outstanding_ = 0;
    lastShown_ = -1;
    lastReturned_ = -1;
}

// s-rta-0928b seqvram F2: a retired sequence releases its textures over several frames, within the frame's budget.
bool ImageSequence::releaseGLWithin(SeqVram::DeleteBudget& deletes, SeqVram::Stats* stats)
{
    deleteTextures(slots_.releaseSome(deletes.left), &deletes, stats);
    if (slots_.size() > 0)
        return false;
    releaseGL();   // no texture left: clears the per-frame state (nothing to delete)
    return true;
}

