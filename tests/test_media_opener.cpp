// s-rta-0929 asyncload (plan-asyncload.md 5.3 / 5.8 test 3 + HARMONY ADOPTION AL3 / AL8 a): MediaOpener -- the REAL
// VideoPlayer (src/media/VideoPlayer.cpp + FFmpeg) opened on the 2-thread "MediaOpen" pool, headless, no GL context. The
// landings go through an injected poster (a mailbox this test thread drains -- the stand-in for the message thread), so
// every case is deterministic: (a) a batch lands its players and completes once; (b) a failed open lands a null player,
// counted; (c1) a cancel while BOTH jobs are running (a start latch) lands both Stale; (c2) a cancel while both jobs are
// still QUEUED (the workers held) deletes them -- counted as dropped, no landing ever arrives; (d) an empty batch
// completes inside begin(); (AL8 a) a thumbnail made on a pool thread equals one made on this thread, pixel for pixel;
// (AL2) destroying an opener whose open hangs forever (a FIFO) returns in ~5 s without killing the thread (forked child).
// Fixture: tests/fixtures/video_h264_64x64.mp4 (3,464 B testsrc2 64x64 H.264).
#include <catch2/catch_test_macros.hpp>

#include "core/MediaOpener.h"
#include "media/VideoPlayer.h"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <deque>
#include <mutex>
#include <thread>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
juce::File fixture(const char* name)
{
    return juce::File(TEST_FIXTURES_DIR).getChildFile(name);
}

// The message-thread stand-in: pool threads post, this thread drains.
struct Mailbox
{
    std::mutex m;
    std::condition_variable cv;
    std::deque<std::function<void()>> q;
    size_t posted = 0;

    void post(std::function<void()> fn)
    {
        {
            std::lock_guard<std::mutex> lk(m);
            q.push_back(std::move(fn));
            ++posted;
        }
        cv.notify_all();
    }
    bool waitPosted(size_t n, int ms)
    {
        std::unique_lock<std::mutex> lk(m);
        return cv.wait_for(lk, std::chrono::milliseconds(ms), [&] { return posted >= n; });
    }
    int drain()
    {
        std::deque<std::function<void()>> todo;
        {
            std::lock_guard<std::mutex> lk(m);
            todo.swap(q);
        }
        for (auto& fn : todo)
            fn();
        return static_cast<int>(todo.size());
    }
};

// A counting latch: count() arrivals, released by open().
struct Gate
{
    std::mutex m;
    std::condition_variable cv;
    int arrived = 0;
    bool released = false;

    void arriveAndWait()
    {
        std::unique_lock<std::mutex> lk(m);
        ++arrived;
        cv.notify_all();
        cv.wait(lk, [&] { return released; });
    }
    bool waitArrived(int n, int ms)
    {
        std::unique_lock<std::mutex> lk(m);
        return cv.wait_for(lk, std::chrono::milliseconds(ms), [&] { return arrived >= n; });
    }
    void open()
    {
        {
            std::lock_guard<std::mutex> lk(m);
            released = true;
        }
        cv.notify_all();
    }
};

struct Landings
{
    std::vector<std::pair<uint32_t, std::unique_ptr<VideoPlayer>>> got;
    int done = 0;
};

void begin(MediaOpener& o, std::vector<MediaOpener::Job> jobs, Landings& L)
{
    o.begin(std::move(jobs),
            [&L](uint32_t id, std::unique_ptr<VideoPlayer> p) { L.got.emplace_back(id, std::move(p)); },
            [&L] { ++L.done; });
}
} // namespace

TEST_CASE("(a) a batch lands every player off this thread and completes once", "[asyncload][opener]")
{
    const auto f = fixture("video_h264_64x64.mp4");
    REQUIRE(f.existsAsFile());
    Mailbox mb;
    MediaOpener o(nullptr);
    o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
    Landings L;
    begin(o, { { 1001, f }, { 1002, f } }, L);
    CHECK(o.pending() == 2);
    CHECK(o.batches() == 1);
    REQUIRE(mb.waitPosted(2, 10000));
    CHECK(L.got.empty());        // nothing lands before the "message thread" runs the posts
    CHECK(mb.drain() == 2);
    REQUIRE(L.got.size() == 2);
    for (auto& [id, p] : L.got)
    {
        CHECK((id == 1001 || id == 1002));
        REQUIRE(p != nullptr);
        CHECK(p->getWidth() == 64);
        CHECK(p->getHeight() == 64);
        CHECK(p->isOpen());
    }
    CHECK(L.done == 1);
    CHECK(o.pending() == 0);
    CHECK(o.failed() == 0);
    CHECK(o.stale() == 0);
}

TEST_CASE("(b) a failed open lands a null player, counted, and the batch still completes", "[asyncload][opener]")
{
    Mailbox mb;
    MediaOpener o(nullptr);
    o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
    Landings L;
    begin(o, { { 7, juce::File("/nonexistent/asyncload/none.mp4") } }, L);
    REQUIRE(mb.waitPosted(1, 10000));
    mb.drain();
    REQUIRE(L.got.size() == 1);
    CHECK(L.got[0].first == 7);
    CHECK(L.got[0].second == nullptr);
    CHECK(o.failed() == 1);
    CHECK(L.done == 1);
}

TEST_CASE("(c1) a cancel while both opens are running lands both Stale", "[asyncload][opener]")
{
    const auto f = fixture("video_h264_64x64.mp4");
    Mailbox mb;
    Gate gate;
    MediaOpener o(nullptr);
    o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
    o.setJobGateForTests([&gate] { gate.arriveAndWait(); });
    Landings L;
    begin(o, { { 1, f }, { 2, f } }, L);
    REQUIRE(gate.waitArrived(2, 10000));   // both jobs are running (2 pool threads)
    o.cancel();
    CHECK(o.pending() == 0);
    gate.open();
    REQUIRE(mb.waitPosted(2, 10000));
    mb.drain();
    CHECK(o.stale() == 2);
    CHECK(o.dropped() == 0);
    CHECK(L.got.empty());
    CHECK(L.done == 0);
}

TEST_CASE("(c2) a cancel while both opens are still queued deletes them: dropped, no landing ever", "[asyncload][opener]")
{
    const auto f = fixture("video_h264_64x64.mp4");
    Mailbox mb;
    Gate hold;
    MediaOpener o(nullptr);
    o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
    for (int i = 0; i < MediaOpener::kThreads; ++i)
        o.addBlockerJobForTests([&hold] { hold.arriveAndWait(); });
    REQUIRE(hold.waitArrived(MediaOpener::kThreads, 10000));   // every worker is held
    Landings L;
    begin(o, { { 1, f }, { 2, f } }, L);
    o.cancel();
    CHECK(o.dropped() == 2);
    hold.open();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CHECK(mb.drain() == 0);
    CHECK(o.stale() == 0);
    CHECK(L.got.empty());
    CHECK(L.done == 0);
}

TEST_CASE("(d) an empty batch completes inside begin()", "[asyncload][opener]")
{
    MediaOpener o(nullptr);
    o.setPosterForTests([](std::function<void()>) { FAIL("an empty batch posts nothing"); });
    Landings L;
    begin(o, {}, L);
    CHECK(L.done == 1);
    CHECK(o.pending() == 0);
    CHECK(o.batches() == 1);
}

TEST_CASE("(AL8 a) a thumbnail made on a pool thread equals one made on this thread", "[asyncload][opener]")
{
    const auto f = fixture("video_h264_64x64.mp4");
    Mailbox mb;
    MediaOpener o(nullptr);
    o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
    Landings L;
    begin(o, { { 1, f } }, L);
    REQUIRE(mb.waitPosted(1, 10000));
    mb.drain();
    REQUIRE(L.got.size() == 1);
    REQUIRE(L.got[0].second != nullptr);
    VideoPlayer here;
    REQUIRE(here.open(f));
    const juce::Image a = L.got[0].second->getThumbnail(90, 72);
    const juce::Image b = here.getThumbnail(90, 72);
    REQUIRE(a.isValid());
    REQUIRE(a.getWidth() == b.getWidth());
    REQUIRE(a.getHeight() == b.getHeight());
    int diff = 0;
    for (int y = 0; y < a.getHeight(); ++y)
        for (int x = 0; x < a.getWidth(); ++x)
            if (a.getPixelAt(x, y).getARGB() != b.getPixelAt(x, y).getARGB())
                ++diff;
    CHECK(diff == 0);
}

TEST_CASE("(AL2) an opener whose open hangs forever is destroyed within ~5 s, its thread left alone (no kill, no crash)",
          "[asyncload][opener]")
{
    // A FIFO with no writer: open(2) inside avformat_open_input blocks forever (the hung-open class). Everything runs in a
    // forked CHILD: the hung thread dies with the child, and a crash is a failed assertion here, never a crashed runner.
    char dir[] = "/tmp/asyncload_fifo_XXXXXX";
    REQUIRE(mkdtemp(dir) != nullptr);
    const std::string fifo = std::string(dir) + "/hung.mp4";
    REQUIRE(mkfifo(fifo.c_str(), 0600) == 0);
    const auto t0 = std::chrono::steady_clock::now();
    const pid_t pid = fork();
    REQUIRE(pid >= 0);
    if (pid == 0)
    {
        {
            Mailbox mb;
            Gate started;
            MediaOpener o(nullptr);
            o.setPosterForTests([&mb](std::function<void()> fn) { mb.post(std::move(fn)); });
            o.setJobGateForTests([&started] {   // records that the job reached open(); never holds it
                std::lock_guard<std::mutex> lk(started.m);
                ++started.arrived;
                started.cv.notify_all();
            });
            Landings L;
            begin(o, { { 1, juce::File(juce::String(fifo)) } }, L);
            if (!started.waitArrived(1, 10000))
                _exit(3);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));   // now inside open(2)
        }   // ~MediaOpener: waits <= 5 s, then leaks the pool
        _exit(0);
    }
    int status = 0;
    pid_t r = 0;
    while ((r = waitpid(pid, &status, WNOHANG)) == 0
           && std::chrono::steady_clock::now() - t0 < std::chrono::seconds(20))
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    if (r == 0)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
    unlink(fifo.c_str());
    rmdir(dir);
    CAPTURE(s, r, status);
    REQUIRE(r == pid);                  // the child returned (no hang past 20 s)
    REQUIRE(WIFEXITED(status));         // not killed by a signal (no crash)
    CHECK(WEXITSTATUS(status) == 0);
    CHECK(s < 12.0);                    // ~5 s: removeAllJobs(true, 5000) then the leak
}
