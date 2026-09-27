#pragma once
#include "recording/Program.h"
#include "recording/Player.h"
#include "recording/RecorderClock.h"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

struct Composition;       // model/Composition.h pulls model/Clip.h (juce_graphics); this header
struct FeatureSnapshot;   // stays juce_core-only (a bank-view model test links juce_core alone)

// RoutineSnap -- the quantize grid a routine starts on. Value for value the same enumerators as
// Clip::BeatSnapMode (Off, Beat, Bar, TwoBar, FourBar); carried as its own uint8_t enum only so
// this header does not include model/Clip.h. RoutineEngine.cpp static_asserts the mapping, and
// callers convert with static_cast (MainComponent: quantizeModeToForcedSnap's result).
enum class RoutineSnap : uint8_t { Off, Beat, Bar, TwoBar, FourBar };

// RoutineEngine -- s167 D9 / s-rta-0926 routines slice 1 (plan-routines-s1-final.md section 4):
// runs every fired routine as its OWN Player on one shared beat clock (a RecorderClock that ticks
// from app start), starts it on the next bar (per-routine quantize; global Quantize overrides),
// fires its restore list through the SAME Player preamble -> dispatch path a take replay uses
// (s-rta-0926b plan3 C: the discrete half ON the boundary; the continuous half GLIDES over the
// last beat before it -- at the start, every loop return and a re-fire restart; a routine whose
// Routine::restoreStyle is Jump restores both halves in one call ON the boundary), loops or holds
// at its end, releases every grip on stop, and arbitrates two routines on
// one control by which gesture BEGAN later (D9 "the later begin wins for the rest of that
// gesture") inside its own sink. Message-thread only (same jassert idiom as RecorderHost);
// talks to the app ONLY through `dispatch`. Never includes MainComponent.h, src/ui/, src/render/.
class RoutineEngine
{
public:
    static constexpr int kBankSize = 8;   // == Composition::kRoutineBankSize (static_assert in .cpp)

    // The SAME lambdas RecorderHost::Dispatch carries (MainComponent copies them over): writes enter
    // the app's handlers as Origin::Replay, so nothing a routine does is captured into a take or
    // pushed onto undo (plan 4.4). Any slot left empty refuses (counted as `skipped`), never a stub.
    struct Dispatch
    {
        std::function<bool(const Fired&)> fire;
        std::function<bool(const ControlPath&, const std::string& grip)> touch;
        std::function<bool(const ControlPath&, float v)> set;   // v normalised [0,1]
        std::function<void(const ControlPath&)> release;
        std::function<void(const std::string&)> notify;          // one-line human-readable notices
        // s-rta-0926b plan3 C: the NORMALISED value the control shows right now (the connected twin
        // when it is driven, else the manual field) -- a restore glide's "from". An empty slot means
        // no glide: the continuous restore lands in one call at the boundary, as before.
        std::function<std::optional<float>(const ControlPath&)> read;
    };
    Dispatch dispatch;

    // plan3 C: a restore glides over kRestoreGlideBeats ending on its boundary (less when fired
    // later), never shorter than kRestoreGlideMinBeats (which then spills past the boundary).
    static constexpr double kRestoreGlideBeats = 1.0;
    static constexpr double kRestoreGlideMinBeats = 0.25;
    static constexpr int kBeatsPerBar = 4;

    struct Status
    {
        double clockBeat = 0.0;
        bool beatAvailable = false;
        int fires = 0;               // fire() calls accepted since app start (never reset)
        std::string lastError;       // the last refusal ("" after a successful fire/save)

        struct Slot
        {
            int slot = -1;
            std::string uuid, name;
            double lengthBeats = 0.0;
            bool loop = false, restoreState = true;
            std::string restoreStyle = "ease";   // "ease" | "jump" (Routine::restoreStyle)
            std::string quantize;    // "off" | "beat" | "bar" | "2bar" | "4bar"
            int lanes = 0, preambleEntries = 0;
            std::string state = "empty";   // "empty" | "idle" | "pending" | "running"
            double position = 0.0;         // routine beats since this cycle's start
            int cycle = 0, restarts = 0;
            uint32_t startedTotalBar = 0;
            int unresolved = 0, reboundByPosition = 0, reboundByName = 0;
            int preambleUnresolved = 0, preambleCount = 0, preambleFired = 0, preambleRefused = 0;
            int skipped = 0;         // discrete fires the app refused at fire time (a layer/clip gone)
            int yielded = 0;         // gestures displaced by ANOTHER routine's later begin (never silent)
            int glides = 0;          // plan3 C: restore glides started and not yet released
            // s-rta-0927 routine display (slice A): where a pending/running routine plays -- computed ONCE
            // at fire from its compiled program's RESOLVED targets (never ControlPath::layer), so a
            // rebound-by-name lane reports the layer it really drives. -1 / empty while idle.
            int deck = -1;                 // the deck its targets resolved on (the active deck at fire)
            std::vector<int> layers;       // sorted, deduplicated layer indices on `deck`
            bool touchesComp = false;      // a target at composition level (layer -1)
            bool restartPending = false;   // re-fired while running: restarts at the next boundary
            uint32_t fireSeq = 0;          // the fire order (the engine's `fires` count at this fire)
            std::string startsOn;          // pending or restartPending: "now" | "beat" | "bar" | "2bar" | "4bar" (effective grid)
        };
        Slot slots[kBankSize];

        struct LastSaved
        {
            int slot = -1;
            std::string uuid, name;
            int lanes = 0, preambleEntries = 0, preambleUnknown = 0;
            std::vector<std::string> dropped;
        };
        LastSaved lastSaved;
    };

    RoutineEngine();
    ~RoutineEngine();   // defined in the .cpp: Running/SlotSink are complete only there
    RoutineEngine(const RoutineEngine&) = delete;
    RoutineEngine& operator=(const RoutineEngine&) = delete;

    // 120 Hz, right after recorderHost_.tick and BEFORE connectionEngine_.tick (plan 4.5).
    // `comp` is read for the bank listing (and a running routine's loop/restore settings at its
    // next end); `forcedSnap` = the global Quantize override (Off when Quantize is off or the
    // tracker is not locked); `beatAvailable` = tracker locked and bpm > 0 (published only).
    void tick(const FeatureSnapshot& snap, double wallNow, const Composition& comp,
              RoutineSnap forcedSnap, bool beatAvailable);

    // Compiles NOW against `comp` (targets resolved once, D2), then queues it for its start
    // boundary. Returns "" or the refusal text (also sent through dispatch.notify). Starts at once
    // (inside this call) when the effective quantize is Off or `beatAvailable` is false (the latter
    // with the notice "no beat yet"). Fire while running = restart at the next boundary; fire while
    // pending = no-op.
    std::string fire(const Composition& comp, int slot, RoutineSnap forcedSnap, bool beatAvailable);
    void stop(int slot);   // releases every grip (Player::stop); idle at once
    // s-rta-0927: the layer X -- stop(slot) (whole routine) for every pending/running routine whose
    // footprint holds {deck, layer}. No-op when none does.
    void stopOnLayer(int deck, int layer);
    void stopAll();        // global Stop, composition load, shutdown

    Status status() const;                          // mutex-guarded copy (HTTP thread reads it)
    void setLastSaved(const Status::LastSaved& s);  // MainComponent's save funnel reports through these
    void setLastError(const std::string& e);

    // The save funnel's refusal text for a sliceRoutine() error, in whole words (carried concern
    // (b)): the unmetered refusal says WHY; anything else is prefixed. Pure; pinned by a ctest.
    static std::string saveRefusalText(const std::string& sliceError);

private:
    struct SlotSink;
    struct Running;
    struct Glide;

    void startNow(Running& r);
    bool dueNow(RoutineSnap mode, bool beatEdge, bool barEdge) const;
    double beatsUntilBoundary(RoutineSnap mode) const;
    void scheduleGlides(Running& r, double boundary, const std::function<double(const ControlPath&)>& keyFreeFrom);
    void stepGlides(Running& r);
    void releaseGlides(Running& r);
    void scheduleRestartGlides(Running& r, RoutineSnap forcedSnap);   // a restart's restore onto ITS boundary
    void resyncPending(Running& r, const Composition& comp, RoutineSnap forcedSnap);   // a waiting start or restart follows its settings
    RoutineSnap effectiveSnap(RoutineSnap forced, RoutineSnap own) const;
    void releaseOwnership(int slot);
    void notify(const std::string& msg) const;
    void refreshBank(const Composition& comp);
    void publishStatus();

    std::vector<Running> running_;   // at most one per slot; compacted AFTER each tick's loop (F5)

    // F1: the routine whose ACCEPTED touch on this control is the most recent (D9 "the later begin
    // wins for the rest of that gesture"). Kept after that routine's release too, so a routine whose
    // gesture began EARLIER stays displaced for its remainder even when the later gesture was a
    // one-call restore (touch -> set -> release); a new gesture always touches first and claims it.
    std::map<ControlPath, int> laneOwner_;

    RecorderClock clock_;            // the routine beat clock: ticks EVERY tick from app start
    bool haveTicked_ = false;
    uint32_t lastTotalBar_ = 0;
    uint32_t lastTotalBeatCount_ = 0;   // the Beat edge is its change (the tracker's beat, stall-proof)
    uint16_t lastBarCount_ = 0;
    float lastBeatPhase_ = 0.0f;     // the tracker's beatPhase last tick (the boundary PREDICTION only)
    uint8_t lastBeatInBar_ = 0;      // plan3 C: the tracker's beatInBar last tick (the Bar boundary prediction)
    bool beatAvailable_ = false;
    int fires_ = 0;
    std::string lastError_;
    Status::LastSaved lastSaved_;

    // The bank listing from the last tick()/fire() (names/settings from the composition), reused
    // by stop()/stopAll(), which do not receive the composition.
    Status::Slot bank_[kBankSize];

    // s-rta-0927: the global Quantize override as last seen by tick()/fire() -- a pending slot's
    // `startsOn` is the effective grid it will start on.
    RoutineSnap lastForcedSnap_ = RoutineSnap::Off;

    // s-rta-0927: a finished run's warning counters, kept for the idle pad of the SAME routine (uuid)
    // so its "!" outlives the run; stopAll() clears them.
    struct LastRun { std::string uuid; int unresolved = 0, preambleUnresolved = 0, skipped = 0; };
    LastRun lastRun_[kBankSize];
    void rememberRun(const Running& r);

    mutable std::mutex statusMutex_;
    Status published_;
};
