#include "core/MediaOpener.h"
#include "media/VideoPlayer.h"
#include <juce_events/juce_events.h>
#include <iostream>

// One open. Runs on a "MediaOpen" pool thread and touches ONLY its own unpublished player and its own data (the job,
// the WeakReference, the poster copy): never the MediaOpener's members, never the renderer -- `stats` is only stored
// into the player (a plain pointer store before start(); open() never reads it, VideoPlayer.h setStats). A job the
// pool deletes without running (a cancel / the destructor dropped it while QUEUED) counts itself in `dropped`.
class MediaOpener::OpenJob final : public juce::ThreadPoolJob
{
public:
    OpenJob(juce::WeakReference<MediaOpener> owner, uint64_t gen, Job job, VideoStats* stats, Poster poster,
            std::function<void()> gate, std::shared_ptr<std::atomic<uint64_t>> dropped)
        : juce::ThreadPoolJob("MediaOpen job"), owner_(std::move(owner)), gen_(gen), job_(std::move(job)),
          stats_(stats), poster_(std::move(poster)), gate_(std::move(gate)), dropped_(std::move(dropped))
    {
    }

    ~OpenJob() override
    {
        if (!ran_)
            dropped_->fetch_add(1, std::memory_order_relaxed);
    }

    JobStatus runJob() override
    {
        ran_ = true;   // read by the destructor, which the pool runs on this same thread after the job finishes
        if (gate_)
            gate_();
        auto player = std::make_unique<VideoPlayer>();
        player->setStats(stats_);
        if (!player->open(job_.file))
            player.reset();   // R8: the landing carries no player -> the staged clip keeps its file defaults
        // A std::function must be copyable: the player rides in a shared box.
        struct Box
        {
            uint64_t gen;
            uint32_t clipId;
            std::unique_ptr<VideoPlayer> player;
        };
        auto box = std::make_shared<Box>(Box{ gen_, job_.clipId, std::move(player) });
        std::function<void()> land = [owner = owner_, box] {
            if (auto* o = owner.get())
                o->landed(box->gen, box->clipId, std::move(box->player));
            // else: the owner is gone -- the player dies here, unpublished (no GL object, no thread)
        };
        if (poster_)
            poster_(std::move(land));
        else
            juce::MessageManager::callAsync(std::move(land));   // false after quit began: the player dies here, unpublished
        return jobHasFinished;
    }

private:
    juce::WeakReference<MediaOpener> owner_;
    uint64_t gen_;
    Job job_;
    VideoStats* stats_;
    Poster poster_;
    std::function<void()> gate_;
    std::shared_ptr<std::atomic<uint64_t>> dropped_;
    bool ran_ = false;
};

MediaOpener::MediaOpener(VideoStats* stats)
    : stats_(stats),
      pool_(std::make_unique<juce::ThreadPool>(juce::ThreadPoolOptions{}.withNumberOfThreads(kThreads)
                                                   .withThreadName("MediaOpen")
                                                   .withDesiredThreadPriority(kPriority)))
{
}

MediaOpener::~MediaOpener()
{
    masterReference.clear();   // a landing still queued on the message thread finds nobody home
    if (!pool_->removeAllJobs(true, kShutdownTimeoutMs))
    {
        // AL2: a job is still inside VideoPlayer::open (FFmpeg may block forever, e.g. open(2) of a FIFO). ~ThreadPool
        // would stopThread(500) and then KILL that thread inside FFmpeg -- a crash at quit. Leak the pool instead: its
        // threads end with the process; the job holds only its own player and a WeakReference (now null).
        std::cerr << "[MediaOpener] an open did not return within " << kShutdownTimeoutMs
                  << " ms at shutdown: its pool is left running (intentional leak)" << std::endl;
        (void) pool_.release();
    }
}

void MediaOpener::begin(std::vector<Job> jobs, Landed onLanded, Done onDone)
{
    cancel();
    const auto gen = ledger_.begin(static_cast<int>(jobs.size()));
    batches_.store(ledger_.batches(), std::memory_order_relaxed);
    pending_.store(ledger_.pending(), std::memory_order_relaxed);
    if (jobs.empty())
    {
        ledger_.cancel();
        if (onDone)
            onDone();
        return;
    }
    onLanded_ = std::move(onLanded);
    onDone_ = std::move(onDone);
    for (auto& job : jobs)
        pool_->addJob(new OpenJob(juce::WeakReference<MediaOpener>(this), gen, std::move(job), stats_, poster_, gate_,
                                  dropped_),
                      true);
}

void MediaOpener::cancel()
{
    ledger_.cancel();
    pending_.store(0, std::memory_order_relaxed);
    onLanded_ = nullptr;
    onDone_ = nullptr;
    // VERIFIED (JUCE juce_ThreadPool.cpp:279-338; plan 5.0 (a), adoption AL8 c): removeAllJobs deletes every QUEUED job
    // under the pool lock and, with timeOutMs 0, returns after one isJobRunning pass -- it never waits for an ACTIVE job
    // (that one finishes its open and lands Stale). interruptRunningJobs false: an open has no exit point anyway.
    pool_->removeAllJobs(false, 0);
}

void MediaOpener::landed(uint64_t gen, uint32_t clipId, std::unique_ptr<VideoPlayer> player)
{
    if (ledger_.land(gen) == stagedload::Ledger::Landing::Stale)
    {
        stale_.fetch_add(1, std::memory_order_relaxed);
        return;   // the player dies here, unpublished
    }
    pending_.store(ledger_.pending(), std::memory_order_relaxed);
    if (!player)
        failed_.fetch_add(1, std::memory_order_relaxed);
    if (onLanded_)
        onLanded_(clipId, std::move(player));
    if (ledger_.complete(gen))
    {
        // Moved out FIRST and nothing touched after d() returns: the completion may begin() the next batch or cancel()
        // re-entrantly (MainComponent::swapCompositionModel cancels at its top).
        auto d = std::move(onDone_);
        onDone_ = nullptr;
        onLanded_ = nullptr;
        ledger_.cancel();
        if (d)
            d();
    }
}
