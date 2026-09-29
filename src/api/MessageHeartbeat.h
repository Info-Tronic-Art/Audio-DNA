#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <algorithm>
#include <atomic>
#include <memory>

// MessageHeartbeat (s-rta-0928b mediaopen, Harmony adoption P6): a TEST-ONLY witness of how long the MESSAGE thread
// stays frozen. A background thread posts one ping at a time (juce::MessageManager::callAsync) every period; when the
// ping runs it records (now - posted) and the next one is posted. A message thread frozen for 1.6 s leaves one ping
// waiting 1.6 s -- recorded when it finally runs. takePeakMs reads (and resets) an atomic, so the HTTP thread can read
// it while the message thread is still frozen and a poller keeps the max over a window.
// Opt-in: nothing runs until start() (a 250 Hz callAsync in every TEST_SERVER launch would sit under every other
// probe's timing rows). Reusable: the idle-stall lane gates on it too. Header-only (juce_core + juce_events).
// Threads: start() / stop() on ONE thread (the message thread: the REST handler posts them there); isOn() /
// takePeakMs() from any thread. The ping state is shared with every queued ping, so a ping that runs after stop() or
// after destruction writes a live-but-unread cell.
class MessageHeartbeat
{
public:
    MessageHeartbeat() = default;
    ~MessageHeartbeat() { stop(); }

    // period_ms is clamped to 1..50. Restarting resets the peak.
    void start(int periodMs)
    {
        stop();
        state_->periodMs.store(std::clamp(periodMs, 1, 50), std::memory_order_relaxed);
        state_->peakMs.store(0.0f, std::memory_order_relaxed);
        thread_ = std::make_unique<Pinger>(state_);
        thread_->startThread(juce::Thread::Priority::normal);
        on_.store(true, std::memory_order_release);
    }

    void stop()
    {
        on_.store(false, std::memory_order_release);
        if (thread_ != nullptr)
        {
            thread_->stopThread(1000);
            thread_.reset();
        }
    }

    bool isOn() const { return on_.load(std::memory_order_acquire); }

    // The longest wait of a ping since the previous call (0 when off); reading resets it.
    double takePeakMs()
    {
        if (!isOn())
            return 0.0;
        return static_cast<double>(state_->peakMs.exchange(0.0f, std::memory_order_relaxed));
    }

    MessageHeartbeat(const MessageHeartbeat&) = delete;
    MessageHeartbeat& operator=(const MessageHeartbeat&) = delete;

private:
    struct State
    {
        std::atomic<int> periodMs{ 4 };
        std::atomic<float> peakMs{ 0.0f };
        std::atomic<bool> outstanding{ false };
    };

    class Pinger : public juce::Thread
    {
    public:
        explicit Pinger(std::shared_ptr<State> s) : juce::Thread("MessageHeartbeat"), state_(std::move(s)) {}
        void run() override
        {
            while (!threadShouldExit())
            {
                if (!state_->outstanding.exchange(true, std::memory_order_acq_rel))
                {
                    const double t0 = juce::Time::getMillisecondCounterHiRes();
                    juce::MessageManager::callAsync([s = state_, t0] {
                        const auto waited = static_cast<float>(juce::Time::getMillisecondCounterHiRes() - t0);
                        if (waited > s->peakMs.load(std::memory_order_relaxed))
                            s->peakMs.store(waited, std::memory_order_relaxed);
                        s->outstanding.store(false, std::memory_order_release);
                    });
                }
                wait(state_->periodMs.load(std::memory_order_relaxed));
            }
        }

    private:
        std::shared_ptr<State> state_;
    };

    const std::shared_ptr<State> state_ = std::make_shared<State>();   // never replaced: queued pings share it
    std::unique_ptr<Pinger> thread_;
    std::atomic<bool> on_{ false };
};
