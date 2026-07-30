#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Regression test for the single-owner-thread confinement pattern used by
// Renderer::getOrCreateSource() to guard activeSources_ (src/render/Renderer.h/.cpp)
// against the concurrent unordered_map mutation race identified in
// .harmony/scout-milkdrop-uaf.md (queue candidate 11): activeSources_ was
// read/inserted from the GL thread (renderOpenGL's playlist-advance logic,
// renderSource, the compositor source-renderer callback), the message thread
// (MilkDrop preset-manager wiring + preset-click callback in MainComponent.cpp),
// and HTTP worker threads (TestServer.cpp's set_preset handler) with no
// synchronization between them — a data race on unordered_map::operator[]
// insertion vs. concurrent find()/insert() from other threads.
//
// The fix confines ALL activeSources_ mutation to the GL thread: callers
// already on the GL thread (detected via
// juce::OpenGLContext::getCurrentContext() != &glContext_, a JUCE
// thread-local) run inline; callers elsewhere marshal through a blocking
// glContext_.executeOnGLThread(fn, /*block*/true) round-trip (house
// precedent: UndoService.cpp:89 / ApiServer.cpp's callAsync marshaling of
// writes onto their owning thread).
//
// Driving the real Renderer/JUCE OpenGLContext headless is infeasible: it
// requires a live, attached GL context with a running render thread, which
// test_compositor.cpp documents as unavailable in this unit-test harness
// (no target links CompositorEngine.cpp or Renderer.cpp for exactly this
// reason) — with no attached context, the real executeOnGLThread/execute()
// dispatch path can't be exercised at all. This harness instead reproduces
// the OWNERSHIP-CONFINEMENT MECHANISM (single owner thread + blocking
// marshal queue for everyone else) and stresses it under concurrent
// contention, the same "mirror the mechanism, not the subsystem" approach
// test_waveform_snapshot.cpp uses for AnalysisThread's seqlock. It proves
// the confinement PATTERN under TSan — it does NOT exercise Renderer.cpp's
// actual code — see the report for what an app-level behavioral gate must
// additionally cover.
namespace
{
class OwnerThreadMap
{
public:
    OwnerThreadMap() : ownerId_(std::this_thread::get_id()) {}

    // Mirrors Renderer::getOrCreateSource(): confine all map mutation to the
    // owner thread, marshaling non-owner callers via a blocking queue.
    int getOrCreate(const std::string& key)
    {
        if (std::this_thread::get_id() != ownerId_)
        {
            int result = -1;
            bool done = false;
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                queue_.push([this, key, &result, &done]
                {
                    result = getOrCreateOnOwner(key);
                    {
                        std::lock_guard<std::mutex> doneLock(doneMutex_);
                        done = true;
                    }
                    doneCv_.notify_all();
                });
            }
            queueCv_.notify_all();

            std::unique_lock<std::mutex> lock(doneMutex_);
            doneCv_.wait(lock, [&done] { return done; });
            return result;
        }
        return getOrCreateOnOwner(key);
    }

    // Owner thread loop: services the marshal queue until told to stop.
    void ownerLoop(std::atomic<bool>& stop)
    {
        while (!stop.load(std::memory_order_relaxed))
        {
            std::function<void()> work;
            {
                std::unique_lock<std::mutex> lock(queueMutex_);
                queueCv_.wait_for(lock, std::chrono::milliseconds(2),
                                   [this] { return !queue_.empty(); });
                if (!queue_.empty())
                {
                    work = std::move(queue_.front());
                    queue_.pop();
                }
            }
            if (work)
                work();
        }
    }

    // Owner-thread-only: read after the owner has stopped and joined.
    size_t creationCount() const { return creationCount_; }

private:
    int getOrCreateOnOwner(const std::string& key)
    {
        auto it = map_.find(key);
        if (it != map_.end())
            return it->second;
        int value = static_cast<int>(map_.size());
        map_[key] = value;
        ++creationCount_;
        return value;
    }

    std::thread::id ownerId_;
    std::unordered_map<std::string, int> map_;
    size_t creationCount_ = 0;

    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::queue<std::function<void()>> queue_;

    std::mutex doneMutex_;
    std::condition_variable doneCv_;
};
}  // namespace

TEST_CASE("Owner-thread-confined map creates each key exactly once under concurrent contention",
          "[renderer][concurrency]")
{
    OwnerThreadMap ownerMap;
    std::atomic<bool> stop{false};

    // The owner thread stands in for the GL thread: it's the only thread
    // that ever touches OwnerThreadMap's internal unordered_map directly.
    std::thread owner([&] { ownerMap.ownerLoop(stop); });

    // Several "non-owner" threads stand in for the message thread and HTTP
    // worker threads, hammering a small set of keys concurrently — the same
    // shape of contention (many callers, few distinct source ids) that
    // raced on activeSources_ before this fix.
    constexpr int kThreads = 8;
    constexpr int kItersPerThread = 2000;
    constexpr int kNumKeys = 5;

    std::vector<std::thread> callers;
    std::atomic<int> totalCalls{0};
    callers.reserve(kThreads);
    for (int t = 0; t < kThreads; ++t)
    {
        callers.emplace_back([&, t]
        {
            for (int i = 0; i < kItersPerThread; ++i)
            {
                std::string key = "source_" + std::to_string((t + i) % kNumKeys);
                ownerMap.getOrCreate(key);
                totalCalls.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& th : callers)
        th.join();

    stop.store(true, std::memory_order_relaxed);
    owner.join();

    REQUIRE(totalCalls.load() == kThreads * kItersPerThread);
    // Confinement must produce exactly one creation per distinct key, never
    // more — a duplicate would mean two threads raced past the owner-thread
    // guard and both inserted, exactly the class of bug this fix prevents.
    REQUIRE(ownerMap.creationCount() == kNumKeys);
}

TEST_CASE("Owner-thread caller runs inline without deadlocking itself",
          "[renderer][concurrency]")
{
    // Mirrors the GL-thread call sites in Renderer.cpp (renderOpenGL's
    // playlist-advance logic, renderSource, the compositor source-renderer
    // callback) that call getOrCreateSource() from a thread that IS the
    // owner: the confinement check must route these inline rather than
    // marshaling to self, which would deadlock (a blocking round-trip waits
    // for the owner thread to service its queue, and the owner thread can't
    // service its own queue while blocked waiting on itself).
    OwnerThreadMap ownerMap;
    int a = ownerMap.getOrCreate("projectm_visualizer");
    int b = ownerMap.getOrCreate("projectm_visualizer");
    int c = ownerMap.getOrCreate("checkerboard");

    REQUIRE(a == b);              // same key, same value, no duplicate creation
    REQUIRE(a != c);               // distinct keys get distinct values
    REQUIRE(ownerMap.creationCount() == 2);
}
