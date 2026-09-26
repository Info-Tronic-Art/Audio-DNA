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
    // continuous lane's cursor up to `pos`, in the SAME domain as the
    // compiled Program's DriveClock. A `pos` smaller than the current
    // position is a defined SEEK (R8/addendum item 2): `advanceTo` calls
    // `seek(pos, sink)` first (a backwards jump is never a stall), then
    // runs its normal forward pass from the re-seated cursors -- no
    // epsilon, any decrease triggers it. A caller that wants a deliberate
    // FORWARD jump WITHOUT catch-up (D3's exactly-once semantics fire
    // every skipped discrete event as a burst otherwise) MUST call
    // `seek()` explicitly first.
    void advanceTo(double pos, Sink& sink);

    // Re-seats every cursor at `pos` without firing anything and without
    // synthesizing state for what `pos` skipped past (same contract as a
    // mid-take `start()`, see below) -- running only (no-op otherwise).
    // A gesture a lane is currently `inGesture` for keeps its grip/
    // displaced flags if it still covers `pos` (`g.x0 <= pos < g.x1`); the
    // next `advanceTo` re-evaluates the curve at the new `pos` with no
    // fresh touch(). Otherwise the gesture is released (unless already
    // displaced) and the cursor resets. `nextDiscrete_` becomes the first
    // event with `at >= pos`, so an event exactly AT the seek target
    // re-fires on the next `advanceTo(pos')` with `pos' >= at`; a seek
    // landing inside an earlier gesture re-touches it on the next
    // `advanceTo`, exactly as `start()` does.
    void seek(double pos, Sink& sink);

    // s-rta-0925 (D4/D9): fires every preamble entry exactly once per call, in Program order --
    // discrete via sink.fire(), continuous via touch("held") -> set(v) -> release (release only
    // when BOTH touch and set were accepted; a refused touch skips set() entirely, mirroring
    // advanceTo's own `displaced` rule -- a refused touch/set leaves the human's grip alone).
    // Never called by advanceTo/seek/swap; the owner calls it right after start(0). Carries no
    // cursor state of its own, so a routine's loop restart (a fresh start(0) + firePreamble) always
    // re-fires the whole preamble (D9). Returns the number of refused entries (discrete fire()
    // refusals + continuous touch/set refusals).
    int firePreamble(Sink& sink);

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

    // D8's LATCH ("your value holds until the lane's next gesture / re-enable") is
    // LATER (D14). Requesting it today is REFUSED, not silently mapped to Touch:
    // returns false, logs, leaves the mode unchanged. Touch (D8's default) is
    // what row 1 implements and tests.
    [[nodiscard]] bool setOverride(Override o);
    Override overrideMode() const { return override_; }

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
