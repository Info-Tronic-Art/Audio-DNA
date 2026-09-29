#pragma once
#include <juce_core/juce_core.h>
#include <algorithm>
#include <chrono>
#include <mutex>

// s-rta-0929 asyncload (plan-asyncload.md section 2 / 5.6): the LAST composition / deck load's cost split, for
// /api/state "load".timing. Written on the MESSAGE thread by the three load paths (loadComposition,
// appendDeckFromFile, duplicateDeck) -- begin() at the top, RAII Scopes around each step, end() when the load is on
// screen; read by any thread through var() (a mutex-guarded copy of the last finished load).
//   parse_ms  file read + JSON parse          prep_ms  validate + re-mint + reconcile + presence seed
//   opens_ms  the message thread's video-open cost (a synchronous open; after the async lane: the landings)
//   open_count / open_max_ms  videos opened (landed) and the longest one
//   seq_ms    sequence opens (no I/O)         swap_ms  the swap / InsertDeckCmd       ui_ms  grid + preview + label
//   msg_ms    the sum of the above (the message thread's whole share)
//   total_ms  wall time from begin() to end() (includes any asynchronous wait)
class LoadTiming
{
public:
    using Clock = std::chrono::steady_clock;
    enum Field { Parse = 0, Prep, Opens, Seq, Swap, Ui, kNumFields };

    static double msSince(Clock::time_point t0)
    {
        return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    }

    // Message thread. A begin() while a load is still being timed (a superseded one) starts over.
    void begin()
    {
        cur_ = {};
        t0_ = Clock::now();
        active_ = true;
    }
    void add(Field f, double ms)
    {
        if (active_)
            cur_.ms[f] += ms;
    }
    // One video opened (or landed) that took `ms` on the message thread; the caller also adds it to Opens.
    void noteOpen(double ms)
    {
        if (!active_)
            return;
        ++cur_.openCount;
        cur_.openMaxMs = std::max(cur_.openMaxMs, ms);
    }
    void end()
    {
        if (!active_)
            return;
        active_ = false;
        cur_.totalMs = msSince(t0_);
        std::lock_guard<std::mutex> lk(m_);
        last_ = cur_;
        ++loads_;
    }
    bool active() const { return active_; }

    // RAII: adds the scope's wall time to `f`.
    struct Scope
    {
        Scope(LoadTiming& t, Field f) : t_(t), f_(f), t0_(Clock::now()) {}
        ~Scope() { t_.add(f_, msSince(t0_)); }
        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
    private:
        LoadTiming& t_;
        Field f_;
        Clock::time_point t0_;
    };

    // Any thread.
    juce::var var() const
    {
        Last l;
        juce::int64 n = 0;
        {
            std::lock_guard<std::mutex> lk(m_);
            l = last_;
            n = loads_;
        }
        auto* o = new juce::DynamicObject();
        double msg = 0.0;
        for (double v : l.ms) msg += v;
        o->setProperty("loads", n);
        o->setProperty("parse_ms", l.ms[Parse]);
        o->setProperty("prep_ms", l.ms[Prep]);
        o->setProperty("opens_ms", l.ms[Opens]);
        o->setProperty("open_count", l.openCount);
        o->setProperty("open_max_ms", l.openMaxMs);
        o->setProperty("seq_ms", l.ms[Seq]);
        o->setProperty("swap_ms", l.ms[Swap]);
        o->setProperty("ui_ms", l.ms[Ui]);
        o->setProperty("msg_ms", msg);
        o->setProperty("total_ms", l.totalMs);
        return juce::var(o);
    }

private:
    struct Last
    {
        double ms[kNumFields] = {};
        int openCount = 0;
        double openMaxMs = 0.0;
        double totalMs = 0.0;
    };
    Last cur_;                  // message thread only
    Clock::time_point t0_{};    // message thread only
    bool active_ = false;       // message thread only
    mutable std::mutex m_;
    Last last_;                 // guarded by m_
    juce::int64 loads_ = 0;     // guarded by m_
};
