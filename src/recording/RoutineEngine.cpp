#include "recording/RoutineEngine.h"
#include "model/Composition.h"
#include "model/Routine.h"
#include "analysis/FeatureSnapshot.h"
#include <juce_events/juce_events.h>
#include <algorithm>
#include <cmath>

static_assert(RoutineEngine::kBankSize == Composition::kRoutineBankSize,
              "the engine's bank is the composition's routine bank");
static_assert(static_cast<int>(RoutineSnap::Off)     == static_cast<int>(Clip::BeatSnapMode::Off)
           && static_cast<int>(RoutineSnap::Beat)    == static_cast<int>(Clip::BeatSnapMode::Beat)
           && static_cast<int>(RoutineSnap::Bar)     == static_cast<int>(Clip::BeatSnapMode::Bar)
           && static_cast<int>(RoutineSnap::TwoBar)  == static_cast<int>(Clip::BeatSnapMode::TwoBar)
           && static_cast<int>(RoutineSnap::FourBar) == static_cast<int>(Clip::BeatSnapMode::FourBar),
              "RoutineSnap mirrors Clip::BeatSnapMode value for value");

// Message-thread only -- the same guard idiom as RecorderHost.cpp: headless tests call the engine
// with no MessageManager running; whenever one IS running, the invariant is enforced.
#define ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD()                                  \
    jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr       \
            || juce::MessageManager::existsAndIsCurrentThread())

namespace
{
    RoutineSnap toSnap(Clip::BeatSnapMode m) { return static_cast<RoutineSnap>(m); }

    std::string counted(int n, const char* one, const char* many)
    {
        return std::to_string(n) + " " + (n == 1 ? one : many);
    }
}

// ---- SlotSink: forwards one routine's Player into `dispatch`, counts refusals, and arbitrates
// routine-vs-routine on a shared control (plan 4.3, D9): an ACCEPTED touch is a gesture begin and
// makes this routine the control's owner; a set from any other routine is then refused (counted as
// `yielded`), which marks that routine's Player displaced for the rest of ITS gesture (the built
// per-gesture TOUCH rule, Player.cpp). A routine that has been overtaken never releases the grip
// the later routine now holds. ManualWrite and Player are untouched. ----
struct RoutineEngine::SlotSink : Sink
{
    RoutineEngine& eng;
    int slot;
    int skipped = 0;
    int yielded = 0;
    SlotSink(RoutineEngine& e, int s) : eng(e), slot(s) {}

    bool fire(const Fired& f) override
    {
        // A refused restore entry is counted by Running::preambleRefused (firePreamble's return
        // value), never here as well -- the same rule as RecorderHost's HostSink.
        const bool isPreamble = f.p.origin == Origin::Preamble;
        const bool ok = eng.dispatch.fire && eng.dispatch.fire(f);
        if (!ok && !isPreamble)
            ++skipped;
        return ok;
    }

    bool touch(const ControlPath& key, const std::string& grip) override
    {
        if (!eng.dispatch.touch)
        {
            ++skipped;
            return false;
        }
        const bool ok = eng.dispatch.touch(key, grip);
        if (ok)
            eng.laneOwner_[key] = slot;   // a human refusal records nothing: the human displaced us (D8)
        return ok;
    }

    bool set(const ControlPath& key, float v) override
    {
        auto it = eng.laneOwner_.find(key);
        if (it != eng.laneOwner_.end() && it->second != slot)
        {
            ++yielded;   // once per gesture: the Player never calls set() again for a displaced gesture
            return false;
        }
        if (!eng.dispatch.set)
        {
            ++skipped;
            return false;
        }
        return eng.dispatch.set(key, v);
    }

    void release(const ControlPath& key) override
    {
        auto it = eng.laneOwner_.find(key);
        if (it != eng.laneOwner_.end() && it->second != slot)
            return;   // overtaken: the later routine's grip is not ours to close
        if (eng.dispatch.release)
            eng.dispatch.release(key);
    }
};

struct RoutineEngine::Running
{
    int slot = -1;
    std::string uuid, name;
    std::shared_ptr<const Program> program;
    std::unique_ptr<Player> player;
    std::unique_ptr<SlotSink> sink;
    bool pending = true;
    RoutineSnap ownSnap = RoutineSnap::Bar;   // the routine's own quantize; a forced (global) one wins
    bool restore = true, loop = false;
    double lengthBeats = 0.0;
    double startBeat = 0.0;                    // clock beat of this cycle's routine beat 0
    double position = 0.0;
    uint32_t startedTotalBar = 0;
    int cycle = 0, restarts = 0;
    bool restartRequested = false;
    int preambleFired = 0, preambleRefused = 0;
    bool done = false;                         // finished inside tick(); compacted AFTER the loop (F5)
};

RoutineEngine::RoutineEngine()
{
    for (int i = 0; i < kBankSize; ++i)
        bank_[i].slot = i;
    publishStatus();
}

// Deliberately does NOT stop the players: dispatch's lambdas point into the app, which may be half
// torn down by now. The app calls stopAll() itself before shutdown (plan 4.3).
RoutineEngine::~RoutineEngine() = default;

void RoutineEngine::notify(const std::string& msg) const
{
    if (dispatch.notify)
        dispatch.notify(msg);
}

RoutineSnap RoutineEngine::effectiveSnap(RoutineSnap forced, RoutineSnap own) const
{
    return forced != RoutineSnap::Off ? forced : own;
}

// Plan 4.2 step 3 / F4: the EDGE is totalBarCount's rising edge (never rewound); the 2 Bar / 4 Bar
// PARITY is barCount's -- the counter Layer::processPendingTrigger consults for a quantized clip.
bool RoutineEngine::dueNow(RoutineSnap mode, bool beatEdge, bool barEdge) const
{
    switch (mode)
    {
        case RoutineSnap::Off:     return true;
        case RoutineSnap::Beat:    return beatEdge;
        case RoutineSnap::Bar:     return barEdge;
        case RoutineSnap::TwoBar:  return barEdge && (lastBarCount_ % 2) == 0;
        case RoutineSnap::FourBar: return barEdge && (lastBarCount_ % 4) == 0;
    }
    return barEdge;
}

void RoutineEngine::releaseOwnership(int slot)
{
    for (auto it = laneOwner_.begin(); it != laneOwner_.end();)
        it = (it->second == slot) ? laneOwner_.erase(it) : std::next(it);
}

void RoutineEngine::startNow(Running& r)
{
    r.startBeat = clock_.now().beat;
    r.startedTotalBar = lastTotalBar_;
    r.position = 0.0;
    r.cycle = 1;
    r.pending = false;
    r.restartRequested = false;
    r.player->start(0.0);

    int refused = 0;
    if (r.restore)
    {
        refused = r.player->firePreamble(*r.sink);
        r.preambleFired += r.program->report.preambleCount - refused;
        r.preambleRefused += refused;
    }
    r.player->advanceTo(0.0, *r.sink);

    const auto& report = r.program->report;
    std::string msg = "Routine " + r.name + " started on bar " + std::to_string(r.startedTotalBar);
    if (!report.preambleUnresolved.empty())
        msg += "; " + counted(static_cast<int>(report.preambleUnresolved.size()), "setting", "settings")
             + " could not be restored (a layer or clip no longer exists)";
    if (refused > 0)
        msg += "; " + counted(refused, "control you are holding was", "controls you are holding were") + " left alone";
    if (!report.unresolved.empty())
        msg += "; " + counted(static_cast<int>(report.unresolved.size()), "timeline points", "timelines point")
             + " at a layer or clip that no longer exists";
    notify(msg);
}

void RoutineEngine::tick(const FeatureSnapshot& snap, double wallNow, const Composition& comp,
                         RoutineSnap forcedSnap, bool beatAvailable)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();

    // Plan 4.2 step 1: the `sample` argument is unused here; the TempoMap it grows is bounded and
    // never read.
    clock_.tick(snap, wallNow, 0);
    const double beat = clock_.now().beat;

    // Step 2: edges, then the trackers. The Beat edge is the TRACKER's beat -- its beatPhase
    // sawtooth wrap, the rule a Beat-quantized clip uses (Autopilot.cpp) -- never a whole beat of
    // clock_, whose zero sits wherever the tracker was when the clock started or re-locked
    // (s-rta-0926 routine-grid).
    const bool barEdge = haveTicked_ && snap.totalBarCount != lastTotalBar_;
    const bool beatEdge = haveTicked_ && snap.beatPhase < lastBeatPhase_ - 0.5f;
    lastTotalBar_ = snap.totalBarCount;
    lastBarCount_ = snap.barCount;
    lastBeatPhase_ = snap.beatPhase;
    haveTicked_ = true;
    beatAvailable_ = beatAvailable;

    // Step 3: an index loop with NO erase inside it (F5); finished routines are marked `done`.
    for (size_t i = 0; i < running_.size(); ++i)
    {
        Running& r = running_[i];
        const RoutineSnap mode = effectiveSnap(forcedSnap, r.ownSnap);

        if (r.pending)
        {
            if (dueNow(mode, beatEdge, barEdge))
                startNow(r);
            continue;
        }

        if (r.restartRequested && dueNow(mode, beatEdge, barEdge))
        {
            // Retrigger = restart at the next boundary (D9, clip retrigger semantics).
            r.player->stop(*r.sink);
            ++r.restarts;
            startNow(r);
            continue;
        }

        double pos = beat - r.startBeat;
        if (pos >= r.lengthBeats)
        {
            // Everything due up to the end first, so a point just before it is never skipped.
            r.player->advanceTo(r.lengthBeats, *r.sink);

            // loop / restore edits made while running land at the end (plan 5.1 `set`).
            if (const Routine* live = comp.routineInSlot(r.slot); live != nullptr && live->uuid == r.uuid)
            {
                r.loop = live->loop;
                r.restore = live->restoreState;
            }

            r.player->stop(*r.sink);   // every grip released (R9)
            if (r.loop && r.lengthBeats > 0.0)
            {
                r.startBeat += r.lengthBeats;   // exact, no drift
                pos -= r.lengthBeats;
                ++r.cycle;
                r.player->start(0.0);
                if (r.restore)   // D9: a loop restart re-fires the restore
                {
                    const int refused = r.player->firePreamble(*r.sink);
                    r.preambleFired += r.program->report.preambleCount - refused;
                    r.preambleRefused += refused;
                }
                r.player->advanceTo(pos, *r.sink);
                r.position = pos;
            }
            else
            {
                // Once: the look HOLDS (manual values stay; a connected control glides back to its
                // signal -- D8 hand-back).
                releaseOwnership(r.slot);
                r.position = r.lengthBeats;
                r.done = true;
            }
            continue;
        }

        r.player->advanceTo(pos, *r.sink);
        r.position = pos;
    }
    std::erase_if(running_, [](const Running& r) { return r.done; });

    refreshBank(comp);
    publishStatus();
}

std::string RoutineEngine::fire(const Composition& comp, int slot, RoutineSnap forcedSnap, bool beatAvailable)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();

    refreshBank(comp);
    auto refuse = [this](const std::string& msg) {
        lastError_ = msg;
        notify(msg);
        publishStatus();
        return msg;
    };
    if (slot < 0 || slot >= kBankSize)
        return refuse("There is no routine pad " + std::to_string(slot + 1)
                      + "; the pads are 1 to " + std::to_string(kBankSize) + ".");
    const Routine* routine = comp.routineInSlot(slot);
    if (routine == nullptr)
        return refuse("Routine pad " + std::to_string(slot + 1) + " is empty.");

    for (auto& r : running_)
    {
        if (r.slot != slot)
            continue;
        // Fire while running = restart at the next boundary; fire while pending = no-op.
        if (!r.pending)
        {
            r.restartRequested = true;
            r.ownSnap = toSnap(routine->quantize);
            r.loop = routine->loop;
            r.restore = routine->restoreState;
        }
        lastError_.clear();
        publishStatus();
        return {};
    }

    Running r;
    r.slot = slot;
    r.uuid = routine->uuid;
    r.name = routine->name.empty() ? "Routine " + std::to_string(slot + 1) : routine->name;
    r.program = compileRoutine(*routine, comp);   // targets resolved once, on the active deck (D2)
    r.player = std::make_unique<Player>(r.program);
    r.sink = std::make_unique<SlotSink>(*this, slot);
    r.ownSnap = toSnap(routine->quantize);
    r.restore = routine->restoreState;
    r.loop = routine->loop;
    r.lengthBeats = r.program->length;
    running_.push_back(std::move(r));
    ++fires_;
    lastError_.clear();

    Running& added = running_.back();
    if (!beatAvailable)
    {
        // A start that waits for a bar that may never come is worse than an immediate one (the
        // same honesty as quantizeModeToForcedSnap for a clip trigger).
        notify("Routine " + added.name + ": no beat yet -- the routine starts now.");
        startNow(added);
    }
    else if (effectiveSnap(forcedSnap, added.ownSnap) == RoutineSnap::Off)
    {
        startNow(added);
    }
    publishStatus();
    return {};
}

void RoutineEngine::stop(int slot)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    for (auto& r : running_)
        if (r.slot == slot)
            r.player->stop(*r.sink);   // every grip released (R9)
    releaseOwnership(slot);
    std::erase_if(running_, [slot](const Running& r) { return r.slot == slot; });
    publishStatus();
}

void RoutineEngine::stopAll()
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    for (auto& r : running_)
        r.player->stop(*r.sink);
    laneOwner_.clear();
    running_.clear();
    publishStatus();
}

void RoutineEngine::setLastSaved(const Status::LastSaved& s)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastSaved_ = s;
    lastError_.clear();
    publishStatus();
}

void RoutineEngine::setLastError(const std::string& e)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastError_ = e;
    publishStatus();
}

std::string RoutineEngine::saveRefusalText(const std::string& sliceError)
{
    // Keyed on sliceRoutine's unmetered refusal ("... the tempo was unknown while it was recorded",
    // RoutineSlice.cpp checkMetered); the ctest [routine][save] pins the pair.
    if (sliceError.find("tempo was unknown") != std::string::npos)
        return "This part of the take was recorded before the tempo was known, so it has no bars to cut.";
    return "Could not save the routine: " + sliceError + ".";
}

void RoutineEngine::refreshBank(const Composition& comp)
{
    for (int i = 0; i < kBankSize; ++i)
    {
        Status::Slot s;
        s.slot = i;
        if (const Routine* r = comp.routineInSlot(i))
        {
            s.uuid = r->uuid;
            s.name = r->name;
            s.lengthBeats = r->lengthBeats;
            s.loop = r->loop;
            s.restoreState = r->restoreState;
            s.quantize = Routine::quantizeToString(r->quantize);
            s.lanes = static_cast<int>(r->lanes.size());
            s.preambleEntries = static_cast<int>(r->preamble.size());
            s.state = "idle";
        }
        bank_[i] = std::move(s);
    }
}

void RoutineEngine::publishStatus()
{
    Status s;
    s.clockBeat = clock_.now().beat;
    s.beatAvailable = beatAvailable_;
    s.fires = fires_;
    s.lastError = lastError_;
    s.lastSaved = lastSaved_;
    for (int i = 0; i < kBankSize; ++i)
        s.slots[i] = bank_[i];

    for (const auto& r : running_)
    {
        if (r.done || r.slot < 0 || r.slot >= kBankSize)
            continue;
        auto& sl = s.slots[r.slot];
        const auto& report = r.program->report;
        sl.state = r.pending ? "pending" : "running";
        sl.position = r.pending ? 0.0 : r.position;
        sl.cycle = r.cycle;
        sl.restarts = r.restarts;
        sl.startedTotalBar = r.startedTotalBar;
        sl.unresolved = static_cast<int>(report.unresolved.size());
        sl.reboundByPosition = static_cast<int>(report.reboundByPosition.size());
        sl.reboundByName = static_cast<int>(report.reboundByName.size());
        sl.preambleUnresolved = static_cast<int>(report.preambleUnresolved.size());
        sl.preambleCount = report.preambleCount;
        sl.preambleFired = r.preambleFired;
        sl.preambleRefused = r.preambleRefused;
        sl.skipped = r.sink->skipped;
        sl.yielded = r.sink->yielded;
    }

    std::lock_guard<std::mutex> lock(statusMutex_);
    published_ = std::move(s);
}

RoutineEngine::Status RoutineEngine::status() const
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    return published_;
}
