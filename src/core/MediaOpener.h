#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "core/StagedLoad.h"

class VideoPlayer;
struct VideoStats;

// s-rta-0929 asyncload (plan-asyncload.md 5.3 + HARMONY ADOPTION AL1 / AL2 / AL3 / AL8): opens VideoPlayers OFF the
// message thread for a staged composition / deck load. One job per video clip on a 2-thread LOW-priority pool
// ("MediaOpen", QOS_CLASS_UTILITY): the job makes a NEW player (setStats, then VideoPlayer::open -- no GL, no thread, no
// Renderer state, on an object nothing else holds) and posts it to the message thread, where landed() applies it or
// drops it by generation (stagedload::Ledger) and calls onLanded (Apply; a null player = open() failed) and, after the
// batch's last Apply, onDone. An EMPTY batch runs onDone inside begin(). Jobs hold a WeakReference + their own data
// only: a landing after the owner is gone destroys the (unpublished) player where it lands. cancel() never waits: it
// drops the QUEUED jobs and lets an in-flight open land Stale. The destructor waits <= 5 s for an in-flight open and,
// if one is still inside FFmpeg (e.g. open(2) of a FIFO blocks forever), LEAKS the pool rather than let ~ThreadPool
// kill a thread blocked in FFmpeg (AL2). Message thread for every member except the counters (atomics, any thread).
class MediaOpener
{
public:
    struct Job
    {
        uint32_t clipId = 0;
        juce::File file;
    };
    using Landed = std::function<void(uint32_t clipId, std::unique_ptr<VideoPlayer> player)>;   // null = open() failed
    using Done = std::function<void()>;
    using Poster = std::function<void(std::function<void()>)>;

    static constexpr int kThreads = 2;
    static constexpr juce::Thread::Priority kPriority = juce::Thread::Priority::low;
    static constexpr int kShutdownTimeoutMs = 5000;

    explicit MediaOpener(VideoStats* stats);
    ~MediaOpener();

    // Message thread. Cancels the live batch first; `jobs` empty -> onDone runs before begin() returns.
    void begin(std::vector<Job> jobs, Landed onLanded, Done onDone);
    // Message thread. Non-blocking: queued jobs are deleted (counted in dropped()), in-flight opens land Stale.
    void cancel();

    // Any thread (relaxed atomics).
    int pending() const { return pending_.load(std::memory_order_relaxed); }             // live batch, not yet landed
    uint64_t batches() const { return batches_.load(std::memory_order_relaxed); }        // begin() calls
    uint64_t stale() const { return stale_.load(std::memory_order_relaxed); }            // landings of a retired batch
    uint64_t failed() const { return failed_.load(std::memory_order_relaxed); }          // Apply landings with no player
    uint64_t dropped() const { return dropped_->load(std::memory_order_relaxed); }       // queued jobs a cancel deleted

    // Tests: the landing poster (default MessageManager::callAsync), a gate every job runs first (on its pool thread),
    // and a job that occupies one pool thread until `fn` returns (so later jobs stay QUEUED). Before begin().
    void setPosterForTests(Poster p) { poster_ = std::move(p); }
    void setJobGateForTests(std::function<void()> gate) { gate_ = std::move(gate); }
    void addBlockerJobForTests(std::function<void()> fn) { pool_->addJob(std::move(fn)); }

    MediaOpener(const MediaOpener&) = delete;
    MediaOpener& operator=(const MediaOpener&) = delete;

private:
    class OpenJob;
    void landed(uint64_t gen, uint32_t clipId, std::unique_ptr<VideoPlayer> player);

    stagedload::Ledger ledger_;
    Landed onLanded_;
    Done onDone_;
    VideoStats* stats_ = nullptr;
    Poster poster_;
    std::function<void()> gate_;
    std::atomic<int> pending_{ 0 };
    std::atomic<uint64_t> batches_{ 0 }, stale_{ 0 }, failed_{ 0 };
    std::shared_ptr<std::atomic<uint64_t>> dropped_ = std::make_shared<std::atomic<uint64_t>>(0);   // outlives a leaked pool
    std::unique_ptr<juce::ThreadPool> pool_;   // a unique_ptr so the destructor can leak it (AL2)
    JUCE_DECLARE_WEAK_REFERENCEABLE(MediaOpener)
};
