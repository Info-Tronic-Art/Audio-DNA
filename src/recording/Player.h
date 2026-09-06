#pragma once
#include "recording/Program.h"
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

// Sink -- s167 D5: what a Player dispatches into. `bool` returns are the
// D8 ownership-chain answer ("accepted?") -- a `false` from touch()/set()
// means a more deliberate hand (human Held/Decaying grip) already owns
// that control for this gesture; the Player marks itself displaced and
// keeps advancing silently rather than fighting for it (D8's TOUCH mode).
// MainComponent's Sink implementation (step 3) re-resolves ResolvedTarget
// into the live model and calls the SAME handlers the human path uses,
// tagged Origin::Replay/Routine.
struct Sink
{
    virtual ~Sink() = default;
    virtual bool fire(const Fired&) = 0;
    virtual bool touch(const ControlPath&, const std::string& grip) = 0;
    virtual bool set(const ControlPath&, float v) = 0;
    virtual void release(const ControlPath&) = 0;
};

// Player -- one per running thing: the set replay, or each routine
// (polyphony, D9). Owns a pinned, immutable Program; `advanceTo` fires due
// discrete points and drives every continuous lane's gesture synchronously
// -- nothing outlives the call (no pointer survives past it, R1).
class Player
{
public:
    enum class Override : uint8_t { Touch, Latch };

    explicit Player(std::shared_ptr<const Program> program);

    // Seats every cursor at `at` (0 = the take's start) without firing
    // anything before it -- a mid-take start only fires what is still due
    // FROM `at` forward, matching advanceTo's own "each event fires
    // exactly once, only once pos has reached its `at`" invariant.
    void start(double at = 0.0);

    // Fires everything newly due since the last call, and drives every
    // continuous lane's cursor up to `pos`. `pos` must be non-decreasing
    // and in the SAME domain as the compiled Program's DriveClock.
    void advanceTo(double pos, Sink& sink);

    // Releases every gesture this Player still holds (D5/R9: "a stopped
    // routine lets go of its hands") and stops firing discrete points.
    void stop(Sink& sink);

    // Edit-while-playing (D5): compile a new Program, then swap into it at
    // the next tick. Every cursor is re-seated at the current position; a
    // lane held by the OLD program with no gesture covering `pos` in the
    // NEW one is released immediately (an orphan). A lane still covered at
    // `pos` and already held keeps its grip without a fresh touch() (a
    // re-seat, not a re-trigger).
    void swap(std::shared_ptr<const Program> program, Sink& sink);

    // Override::Latch is accepted and stored but not yet wired into
    // advanceTo's dispatch this step (see the builder report's
    // DEVIATIONS) -- Touch (D8's default) is what row 1 implements and
    // tests.
    void setOverride(Override o) { override_ = o; }

    void reenable(const ControlPath& key);
    void reenableAll();

    double position() const { return pos_; }
    bool running() const { return running_; }
    const CompileReport& report() const { return prog_->report; }

private:
    struct LaneCursor
    {
        size_t gestureIndex = 0;
        bool inGesture = false;
        bool displaced = false;   // TOUCH refusal: writes refused for THIS gesture only (D8)
    };

    std::shared_ptr<const Program> prog_;
    size_t nextDiscrete_ = 0;
    double pos_ = 0.0;
    bool running_ = false;
    Override override_ = Override::Touch;
    std::vector<LaneCursor> cursors_;
};
