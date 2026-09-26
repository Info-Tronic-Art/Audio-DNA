#pragma once
#include "model/ControlPath.h"
#include "recording/Take.h"
#include <juce_core/juce_core.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <optional>

struct Composition;

// Program.h -- s167 D5: the COMPILED, IMMUTABLE schedule for one drive
// clock. `Program::compile` is the ONLY place a ControlPath's coordinates
// are trusted (D2's resolution policy); everything downstream (Player,
// Sink) works from validated indices, never a name lookup, never a
// pointer into a live model (R1).

enum class DriveClock : uint8_t { Wall, Beat, Sample };

// Range -- reserved for a partial compile (a routine's slice, LATER/D9).
// Row 1 supports it as a simple clip-to-[from,to) filter with no preamble
// synthesis; full slice() semantics (D4) land with routines.
struct Range { double from = 0.0, to = 0.0; };

// ResolvedTarget -- validated COORDINATES only, never a pointer (R1). -1
// means "not applicable to this control", not "unresolved" (that is a
// CompileReport bucket, not a field here).
struct ResolvedTarget
{
    int deck = -1;
    int layer = -1;
    int col = -1;
    int fx = -1;
    int param = -1;
    int macro = -1;
};

struct Fired
{
    double at = 0.0;
    uint64_t seq = 0;
    ControlPath key;
    ResolvedTarget target;
    DiscretePoint p;   // BY VALUE (R1) -- never a pointer into Take::lanes
};

struct ContLane
{
    ControlPath key;
    ResolvedTarget target;
    struct G { double x0 = 0.0, x1 = 0.0; AutomationCurve curve; std::string grip; };
    std::vector<G> gestures;
};

// s-rta-0925 (plan section 3.2): a continuous restore entry (D4 preamble
// for a continuous control) -- fired as touch("held") -> set(v) -> release
// at Player::firePreamble, never by advanceTo. `v` is NORMALISED [0,1],
// exactly what Sink::set takes.
struct PreambleSet
{
    ControlPath key;
    ResolvedTarget target;
    float v = 0.0f;
};

struct Issue
{
    ControlPath key;
    std::string reason;
};

// D2's tri-state resolution outcome, bucketed for the compile-time report
// ("There is no path on which a lane silently does nothing" -- G5's raw
// handlers would; a compiled Program never has that path).
struct CompileReport
{
    std::vector<Issue> unresolved;
    std::vector<Issue> reboundByPosition;
    std::vector<Issue> reboundByName;
    std::vector<Issue> invalid;   // gestures whose x had to be reconstructed
                                   // from the tempo map -- no parallel stamps
    std::map<std::string, int> unknown;          // e.g. "kind:temporal-blend" -> count
    std::vector<std::string> missingRoutines;     // reserved (D9, LATER); always empty in row 1
    int resolvedCount = 0;

    // s-rta-0925 (D4 preamble, plan section 3.2): a deck/layer/clip/effect
    // slot the checkpoint0 preamble could not resolve at all (Missing, D2) --
    // counted and named, never silently dropped, same policy as `unresolved`
    // above but kept SEPARATE because the preamble is not a recorded lane
    // and Boris's ruling requires it be said in one plain notice line
    // (RecorderHost::Status::preambleUnresolved). A rebind (position/name)
    // for a preamble entry is folded into the existing `reboundByPosition`/
    // `reboundByName` buckets above, tagged with a "preamble: " reason.
    std::vector<Issue> preambleUnresolved;
    int preambleCount = 0;   // discrete + continuous preamble entries actually emitted
};

struct Program
{
    DriveClock clock = DriveClock::Wall;
    double length = 0.0;
    bool loop = false;
    std::vector<Fired> preamble;               // s-rta-0925: D4's discrete preamble entries, at=0/seq=0
    std::vector<PreambleSet> preambleContinuous;   // s-rta-0925: D4's continuous preamble entries
    std::vector<Fired> discrete;    // k-way merged, ordered by (at, seq) -- D3
    std::vector<ContLane> continuous;
    CompileReport report;
};

// compile -- once per load/play (D2). Resolves every lane's ControlPath
// against `comp` (D2's three-way policy: resolved / rebound-by-position /
// rebound-by-name / unresolved -- unresolved lanes are compiled OUT, never
// silently dropped without a trace), picks each point's `at` from whichever
// stamp field matches `clock` (t / beat / sample -- already captured by
// RecorderClock on every point, D1), and k-way merges every discrete lane
// by (at, seq).
//
// Row 1 scope (disclosed, see the builder report): full resolution depth
// for Comp/Layer/Clip scopes (deck -> layer -> col, each independently
// position/name checked); an `fx` sub-level resolves the same way within
// the resolved object's effect stack. `param`/`macro` resolve by INDEX
// only (bounds-checked) -- name-fallback search via EffectLibrary is not
// implemented this step (see the report's DEVIATIONS). Scope::Routine
// always reports unresolved (routines are D9, LATER).
//
// Continuous breakpoints: each one's x comes from its own gesture's
// parallel Stamp (Lane.h Gesture::stamps, D1/D7) via the same per-clock
// pick discrete points use -- the take's TempoMap is only the reported
// fallback (CompileReport::invalid) for a gesture with no parallel stamps.
std::shared_ptr<const Program> compile(const Take& take, const Composition& comp,
                                        DriveClock clock, std::optional<Range> range = {});
