#pragma once
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <limits>

// s-rta-0928b video: the lock-free frame ring between a VideoPlayer's decode thread (ONE writer) and the GL thread (ONE
// reader), and the pure catch-up policy. No FFmpeg, no GL, no JUCE: tests/test_video_ring.cpp drives it headless.
//
// The ring holds only the slot HEADERS (state, pts, request generation, sequence number); the slot BYTES live in the
// VideoPlayer (allocated in open()). A slot's state moves Free -> Writing (writer, CAS) -> Ready (writer, release store)
// -> Reading (reader, CAS) -> Free (reader), or Ready -> Free (reader: a stale / passed-over / unreachable frame), or
// Writing -> Free (writer: abandon). Only the writer leaves Free/Writing, only the reader leaves Ready/Reading, so no slot
// ever has two owners and the reader never waits. Every CAS is acq_rel on success / acquire on failure (V3): a slot's
// fields and bytes are written before its Ready store (release) and read after the reader's acquire load / CAS.
namespace VideoRing
{
enum class SlotState : uint8_t { Free, Writing, Ready, Reading };

struct SlotHeader
{
    std::atomic<uint8_t> state{ static_cast<uint8_t>(SlotState::Free) };
    double pts = -1.0;
    uint32_t gen = 0;
    uint64_t seq = 0;
};

template <int N = 3>
class Ring
{
public:
    static constexpr int kSlots = N;

    // Writer: CAS Free -> Writing on the first Free slot; -1 = none (the ring is full).
    int acquireWrite()
    {
        for (int i = 0; i < N; ++i)
            if (cas(i, SlotState::Free, SlotState::Writing))
                return i;
        return -1;
    }

    // Writer: the fields, then state.store(Ready, release).
    void publish(int slot, double pts, uint32_t gen, uint64_t seq)
    {
        auto& s = slots_[static_cast<size_t>(slot)];
        s.pts = pts;
        s.gen = gen;
        s.seq = seq;
        s.state.store(static_cast<uint8_t>(SlotState::Ready), std::memory_order_release);
    }

    // Writer: Writing -> Free (a frame dropped after its slot was taken). No-op on any other state.
    void abandon(int slot) { cas(slot, SlotState::Writing, SlotState::Free); }

    struct Pick
    {
        int slot = -1;
        double pts = -1.0;
        uint64_t seq = 0;
        int skipped = 0;        // same-gen Ready frames at or before the chosen one, passed over (freed)
        int staleDropped = 0;   // Ready frames of another request generation (freed)
        int aheadDropped = 0;   // same-gen Ready frames further ahead of the clock than dropAheadSec (freed)
        int readyAhead = 0;     // same-gen Ready frames ahead of the clock, kept (the future)
    };

    // Reader: the newest Ready slot with gen == gen && pts <= clock + tolSec -> Ready -> Reading (the caller uploads it,
    // then release()s it). Stale-gen Ready -> Free; same-gen Ready with pts <= the chosen pts -> Free (skipped); same-gen
    // Ready with pts > clock + tolSec + dropAheadSec -> Free (a frame the clock moved away from -- reverse / ping-pong play:
    // it would never become current and would keep the writer out of the ring); other Ready slots ahead of the clock stay.
    // Never waits.
    Pick pick(double clock, uint32_t gen, double tolSec,
              double dropAheadSec = std::numeric_limits<double>::infinity())
    {
        Pick p;
        const double limit = clock + tolSec;
        for (int i = 0; i < N; ++i)
        {
            auto& s = slots_[static_cast<size_t>(i)];
            if (s.state.load(std::memory_order_acquire) != static_cast<uint8_t>(SlotState::Ready))
                continue;
            if (s.gen != gen)
            {
                if (cas(i, SlotState::Ready, SlotState::Free))
                    ++p.staleDropped;
                continue;
            }
            if (s.pts <= limit)
            {
                if (p.slot < 0 || s.pts > p.pts || (s.pts == p.pts && s.seq > p.seq))
                {
                    p.slot = i;
                    p.pts = s.pts;
                    p.seq = s.seq;
                }
            }
            else if (s.pts > limit + dropAheadSec)
            {
                if (cas(i, SlotState::Ready, SlotState::Free))
                    ++p.aheadDropped;
            }
            else
                ++p.readyAhead;
        }
        if (p.slot < 0)
            return p;
        if (!cas(p.slot, SlotState::Ready, SlotState::Reading))   // cannot fail: only the reader leaves Ready
        {
            p.slot = -1;
            return p;
        }
        for (int i = 0; i < N; ++i)
        {
            if (i == p.slot)
                continue;
            auto& s = slots_[static_cast<size_t>(i)];
            if (s.state.load(std::memory_order_acquire) == static_cast<uint8_t>(SlotState::Ready) && s.gen == gen
                && s.pts <= p.pts && cas(i, SlotState::Ready, SlotState::Free))
                ++p.skipped;
        }
        return p;
    }

    // Reader: Reading -> Free. No-op on any other state (defined behaviour, not an assert-only path).
    void release(int slot) { cas(slot, SlotState::Reading, SlotState::Free); }

    int readyCount() const { return count(SlotState::Ready); }
    int freeCount() const { return count(SlotState::Free); }
    const SlotHeader& header(int i) const { return slots_[static_cast<size_t>(i)]; }

private:
    std::array<SlotHeader, N> slots_;

    bool cas(int i, SlotState from, SlotState to)
    {
        if (i < 0 || i >= N)
            return false;
        auto expect = static_cast<uint8_t>(from);
        return slots_[static_cast<size_t>(i)].state.compare_exchange_strong(
            expect, static_cast<uint8_t>(to), std::memory_order_acq_rel, std::memory_order_acquire);
    }

    int count(SlotState st) const
    {
        int n = 0;
        for (const auto& s : slots_)
            n += (s.state.load(std::memory_order_acquire) == static_cast<uint8_t>(st)) ? 1 : 0;
        return n;
    }
};

// ---- the decode thread's policy (VideoPlayer::decodeLoop), today's GL-thread rules moved off the GL thread ----
enum class Step : uint8_t { Decode, Reseek };

struct Policy
{
    double reseekBehindSec = 0.1;     // VideoPlayer.cpp decodeFrameAtTime's "diff < -0.1 || diff > 2.0" (pre-thread)
    double reseekAheadSec = 2.0;
    double dropBehindFrames = 1.5;    // a decoded frame this far behind the wanted time is dropped unconverted
    int fullDecodeFrames = 10;        // NONREF skipping stops this many frames before the target (the landing is decoded)
    bool skipNonRefInCatchUp = false;
    int64_t idleMs = 250;             // no draw request for this long -> the thread parks (rule 15: no decode off screen)
};

// Seek (to the keyframe before want) when the wanted time is behind the newest published frame or far ahead of it.
// No newest frame (right after a seek: the catch-up) -> keep decoding forward, never re-seek (the old S1b loop).
inline Step decide(double want, double newestPts, bool haveNewest, const Policy& p)
{
    return (!haveNewest || (want >= newestPts - p.reseekBehindSec && want <= newestPts + p.reseekAheadSec))
               ? Step::Decode
               : Step::Reseek;
}

// A decoded frame is converted and published only if it is not too far behind the wanted time.
inline bool shouldPublish(double framePts, double want, double frameDur, const Policy& p)
{
    return framePts >= want - p.dropBehindFrames * frameDur;
}

// AVDISCARD_NONREF while the catch-up is more than fullDecodeFrames from the target (a non-ref frame is never
// referenced, and it would be dropped anyway).
inline bool useSkipNonRef(double framePts, double want, double frameDur, const Policy& p)
{
    return p.skipNonRefInCatchUp && (want - framePts) > p.fullDecodeFrames * frameDur;
}

// V2 (HARMONY ADOPTION): checked at the top of the decode loop AND inside the ring-full wait -- a thread whose player
// has not been drawn for idleMs parks (wait until notified) even mid-catch-up or with a full ring.
enum class Idle : uint8_t { Run, Park };
inline Idle idleStep(int64_t nowMs, int64_t lastDrawMs, const Policy& p)
{
    return (nowMs - lastDrawMs > p.idleMs) ? Idle::Park : Idle::Run;
}

// ---- the GL thread's verdict when uploadToTexture picks nothing ----
// Failed (ADDENDUM W3): never shown and never going to be -- "no media" (texture 0, NOT pending), like a failed image.
enum class Shown : uint8_t { New, Held, Late, Pending, Failed };

inline Shown judge(bool picked, bool shownBefore, bool playing, double clock, double lastShownPts, double frameDur,
                   bool failed = false)
{
    if (picked)
        return Shown::New;
    if (!shownBefore)
        return failed ? Shown::Failed : Shown::Pending;
    return (playing && std::fabs(clock - lastShownPts) > 1.5 * frameDur) ? Shown::Late : Shown::Held;
}

// ADDENDUM W3: a player that has never shown a frame is FAILED -- not pending, so C1 never waits on it and the
// render_frame gate does not hang -- once its decode thread gave up before any frame (a decode error, or EOF), or when no
// frame arrived within kFirstFrameTimeoutMs of its first draw request (firstDrawMs < 0 = never drawn: no clock runs yet).
// A player that has shown a frame is never failed (a hold); a frame that lands after the verdict still shows (judge: New).
constexpr int64_t kFirstFrameTimeoutMs = 2000;
inline bool firstFrameFailed(bool shownBefore, bool gaveUp, int64_t firstDrawMs, int64_t nowMs,
                             int64_t timeoutMs = kFirstFrameTimeoutMs)
{
    if (shownBefore)
        return false;
    return gaveUp || (firstDrawMs >= 0 && nowMs - firstDrawMs >= timeoutMs);
}

// V1 (HARMONY ADOPTION): "shown before" is a flag set on the first upload and NEVER cleared by a GL release (context
// loss) -- only a new open() starts a new player. A context loss mid-hold stays a HOLD, never Pending.
struct ShownState
{
    bool everShown = false;
    uint64_t lastUploadedSeq = 0;
    void onUpload(uint64_t seq) { everShown = true; lastUploadedSeq = seq; }
    void onReleaseGL() { lastUploadedSeq = 0; }   // the next pick re-uploads; everShown stays
    bool needsUpload(uint64_t seq) const { return seq != lastUploadedSeq; }
};
} // namespace VideoRing
