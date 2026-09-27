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

    // plan3 C: the routine beat until which the recording's own hand is on `key` -- the latest end of a
    // gesture on it that is under way at `pos` or begins before `endPos` (0 when there is none). A
    // gesture that begins at or after `endPos` is never reached before the boundary, so it never delays
    // a glide.
    double busyUntil(const Program& prog, const ControlPath& key, double pos, double endPos)
    {
        double busy = 0.0;
        for (const auto& lane : prog.continuous)
            if (lane.key == key)
                for (const auto& g : lane.gestures)
                    if (g.x0 < endPos && g.x1 > pos)
                        busy = std::max(busy, g.x1);
        return busy;
    }

    // s-rta-0927 routine display: where a compiled routine plays -- its RESOLVED targets (a rebound-by-name
    // lane sits on the layer it really drives, never ControlPath::layer). deck = the first target's deck;
    // layers sorted and deduplicated; touchesComp = any composition-level target (layer -1).
    struct Footprint { int deck = -1; std::vector<int> layers; bool touchesComp = false; };

    Footprint footprint(const Program& prog)
    {
        Footprint fp;
        const auto add = [&fp](const ResolvedTarget& t) {
            if (fp.deck < 0 && t.deck >= 0)
                fp.deck = t.deck;
            if (t.layer >= 0)
                fp.layers.push_back(t.layer);
            else
                fp.touchesComp = true;
        };
        for (const auto& f : prog.preamble)            add(f.target);
        for (const auto& ps : prog.preambleContinuous) add(ps.target);
        for (const auto& f : prog.discrete)            add(f.target);
        for (const auto& lane : prog.continuous)       add(lane.target);
        std::sort(fp.layers.begin(), fp.layers.end());
        fp.layers.erase(std::unique(fp.layers.begin(), fp.layers.end()), fp.layers.end());
        return fp;
    }

    const char* snapWord(RoutineSnap m)
    {
        switch (m)
        {
            case RoutineSnap::Off:     return "now";
            case RoutineSnap::Beat:    return "beat";
            case RoutineSnap::Bar:     return "bar";
            case RoutineSnap::TwoBar:  return "2bar";
            case RoutineSnap::FourBar: return "4bar";
        }
        return "bar";
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

// ---- plan3 C (s-rta-0926b): one continuous restore entry on its way from what the control shows to
// the recorded start value -- this routine's own lane-rank hand on that knob: touch at t0, one set per
// tick on a straight line, release at t1 (routine-clock beats). It writes through the routine's SlotSink,
// so routine-vs-routine arbitration and "a human hand wins" apply unchanged. ----
struct RoutineEngine::Glide
{
    ControlPath key;
    float from = 0.0f, to = 0.0f, last = 0.0f;
    double t0 = 0.0, t1 = 0.0;
    bool started = false;   // the touch was accepted; `from` is read at the touch
    bool landed = false;    // arrived while still pending: holds `to` until the start, then lets go
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
    bool jump = false;                         // Routine::RestoreStyle::Jump: never glide -- the restore lands in one call ON its boundary
    double lengthBeats = 0.0;
    double startBeat = 0.0;                    // clock beat of this cycle's routine beat 0
    double position = 0.0;
    uint32_t startedTotalBar = 0;
    int cycle = 0, restarts = 0;
    bool restartRequested = false;
    int preambleFired = 0, preambleRefused = 0;
    bool done = false;                         // finished inside tick(); compacted AFTER the loop (F5)
    std::vector<Glide> glides;                 // plan3 C: at most one per control
    int glideCycle = 0;                        // the cycle whose loop return has been scheduled
    int glideRefused = 0;                      // this restore's glides a human hand refused (the start notice)
    bool glideScheduled = false;               // the continuous half of the NEXT restore belongs to `glides`
    // s-rta-0927 routine display: the footprint, computed ONCE at fire (never per tick), and the fire order.
    int deck = -1;
    std::vector<int> layers;
    bool touchesComp = false;
    uint32_t fireSeq = 0;
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

// plan3 C: how many beats until the boundary `mode` starts on, predicted from the last snapshot (the
// start itself is still the EVENT -- dueNow). Bar: the rest of this bar; 2 Bar / 4 Bar: plus the whole
// bars until the edge that makes barCount % N == 0 (the parity dueNow consults); Off: now.
double RoutineEngine::beatsUntilBoundary(RoutineSnap mode) const
{
    const double toBar = static_cast<double>(kBeatsPerBar) - static_cast<double>(lastBeatInBar_)
                       - static_cast<double>(lastBeatPhase_);
    switch (mode)
    {
        case RoutineSnap::Off:     return 0.0;
        case RoutineSnap::Beat:    return 1.0 - static_cast<double>(lastBeatPhase_);
        case RoutineSnap::Bar:     return toBar;
        case RoutineSnap::TwoBar:  return toBar + kBeatsPerBar * ((2 - (lastBarCount_ + 1) % 2) % 2);
        case RoutineSnap::FourBar: return toBar + kBeatsPerBar * ((4 - (lastBarCount_ + 1) % 4) % 4);
    }
    return toBar;
}

// plan3 C, the one scheduling rule (absolute routine-clock beats): every continuous restore entry glides
// over one beat that ENDS on `boundary`, never before now nor before the recording's own hand lets go of
// that knob (`keyFreeFrom`); a shorter window when there is less than a beat left, and a quarter-beat
// floor that spills past the boundary when there is less than that. One glide per control: an unstarted
// one is re-timed, a started one re-aimed from where it is (never a jump). The caller decides that the
// restore is on and a beat is available.
void RoutineEngine::scheduleGlides(Running& r, double boundary,
                                   const std::function<double(const ControlPath&)>& keyFreeFrom)
{
    if (!dispatch.read)
        return;   // no "from": the continuous restore lands in one call at the boundary, as before
    const double now = clock_.now().beat;
    for (const auto& ps : r.program->preambleContinuous)
    {
        const double earliest = std::max(now, keyFreeFrom(ps.key));
        const double len = std::clamp(boundary - earliest, kRestoreGlideMinBeats, kRestoreGlideBeats);
        const double t1 = std::max(boundary, earliest + len);
        const double t0 = t1 - len;

        auto it = std::find_if(r.glides.begin(), r.glides.end(), [&](const Glide& g) { return g.key == ps.key; });
        if (it == r.glides.end())
        {
            Glide g;
            g.key = ps.key;
            g.to = ps.v;
            g.t0 = t0;
            g.t1 = t1;
            r.glides.push_back(std::move(g));
        }
        else if (it->to == ps.v && std::abs(it->t1 - t1) < 1.0 / 32.0)
        {
            // already on its way to the same place, landing at the same time
        }
        else if (!it->started)
        {
            it->to = ps.v;
            it->t0 = t0;
            it->t1 = t1;
        }
        else
        {
            it->from = it->last;
            it->to = ps.v;
            it->t0 = now;
            it->t1 = std::max(t1, now + kRestoreGlideMinBeats);
            it->landed = false;
        }
    }
    r.glideRefused = 0;
    r.glideScheduled = true;
}

// plan3 C: one pass per tick over a routine's glides. Runs AFTER the Player's advanceTo in every tick
// path, so the tick the recording's own hand takes a knob (Player::holds), the glide is dropped before it
// writes -- without a release: equal rank, the release would close the Player's grip.
void RoutineEngine::stepGlides(Running& r)
{
    const double beat = clock_.now().beat;
    for (size_t i = 0; i < r.glides.size();)
    {
        Glide& g = r.glides[i];
        const auto drop = [&r, i] { r.glides.erase(r.glides.begin() + static_cast<std::ptrdiff_t>(i)); };

        if (!g.started && beat < g.t0)
        {
            ++i;
            continue;
        }
        if (r.player->holds(g.key))
        {
            drop();   // the cancel rule: the recording moves this knob now
            continue;
        }
        if (!g.started)
        {
            if (!r.sink->touch(g.key, "held"))
            {
                ++r.glideRefused;   // a human holds it: left alone (the start notice counts it)
                ++r.preambleRefused;
                drop();
                continue;
            }
            ++r.preambleFired;
            g.from = dispatch.read ? dispatch.read(g.key).value_or(g.to) : g.to;   // where it is NOW
            g.started = true;
        }
        if (g.landed)
        {
            if (!r.pending)
            {
                r.sink->release(g.key);   // the start came: let go
                drop();
                continue;
            }
            ++i;
            continue;
        }

        const double u = std::clamp((beat - g.t0) / (g.t1 - g.t0), 0.0, 1.0);
        g.last = g.from + static_cast<float>(u) * (g.to - g.from);
        if (!r.sink->set(g.key, g.last))
        {
            // Another routine took the knob (its later touch; SlotSink counted `yielded`), or a human cut in
            // (this routine still owns it): either way no release -- the knob is someone else's now.
            const auto owner = laneOwner_.find(g.key);
            if (owner != laneOwner_.end() && owner->second == r.slot)
            {
                ++r.glideRefused;
                ++r.preambleRefused;
            }
            drop();
            continue;
        }
        if (u >= 1.0)
        {
            if (r.pending)
            {
                g.landed = true;   // the edge comes late: hold the start value until it does
            }
            else
            {
                r.sink->release(g.key);
                drop();
                continue;
            }
        }
        ++i;
    }
}

// plan3 C: every non-restoring end (once-end, loop or restore switched off at the end, stop, stop-all)
// lets go of the glides in flight where they are -- never a leaked grip (critic LP-2).
void RoutineEngine::releaseGlides(Running& r)
{
    for (const auto& g : r.glides)
        if (g.started)
            r.sink->release(g.key);   // a no-op when another routine has since taken the knob
    r.glides.clear();
    r.glideScheduled = false;
}

void RoutineEngine::startNow(Running& r)
{
    r.startBeat = clock_.now().beat;
    r.startedTotalBar = lastTotalBar_;
    r.position = 0.0;
    r.cycle = 1;
    r.glideCycle = 0;
    r.pending = false;
    r.restartRequested = false;
    r.player->start(0.0);

    const bool glidePath = r.glideScheduled;
    r.glideScheduled = false;
    int refused = 0;
    if (r.restore)
    {
        // The discrete half (clips, flags, play/pause) fires ON the boundary, before the continuous half lands.
        const int discreteRefused = r.player->firePreambleDiscrete(*r.sink);
        r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
        r.preambleRefused += discreteRefused;
        refused = discreteRefused;
        if (glidePath)
        {
            const double now = r.startBeat;
            for (auto& g : r.glides)
            {
                if (!g.started && g.t0 > now)
                {
                    g.t0 = now;   // the edge beat its prediction: a quarter-beat spill, never a snap
                    g.t1 = now + kRestoreGlideMinBeats;
                }
            }
            stepGlides(r);   // due glides land and let go here; spills go on
            refused += r.glideRefused;
        }
        else
        {
            // No `read` wired or no beat when it was fired: the continuous restore lands in one call, as before.
            const int continuousRefused = r.player->firePreambleContinuous(*r.sink);
            r.preambleFired += static_cast<int>(r.program->preambleContinuous.size()) - continuousRefused;
            r.preambleRefused += continuousRefused;
            refused += continuousRefused;
        }
    }
    else
    {
        releaseGlides(r);
    }
    r.glideRefused = 0;
    r.player->advanceTo(0.0, *r.sink);

    // The start notice carries no bar number (plan3 B.2 (3): the TopBar's count is 1-2-3-4).
    const auto& report = r.program->report;
    std::string msg = "Routine " + r.name + " started";
    if (!report.preambleUnresolved.empty())
        msg += "; " + counted(static_cast<int>(report.preambleUnresolved.size()), "setting", "settings")
             + " could not be restored (a layer or clip no longer exists)";
    if (refused > 0)
        msg += "; " + counted(refused, "control you are holding was", "controls you are holding were") + " left alone";
    if (!report.unresolved.empty())
        msg += "; " + counted(static_cast<int>(report.unresolved.size()), "timeline points", "timelines point")
             + " at a layer or clip that no longer exists";
    notify(msg + ".");
}

// s-rta-0927 fix round: a WAITING routine re-reads its Quantize, Loop, Restore first / Start from now and Start:
// Ease / Jump from the live routine on every tick (the re-fire-while-running re-sync in fire(), for the wait), so
// a pad-menu or REST edit made before the start reaches THIS start -- the menu's tick, the pad's "Starting on ..."
// tooltip and what happens on the boundary always agree. The restore glides follow the style: switched off (Jump,
// or Start from now) they let go where they are; switched on (Ease) they are scheduled for the boundary now
// ahead, by the one rule fire() uses; a Quantize change re-times them.
void RoutineEngine::resyncPending(Running& r, const Composition& comp, RoutineSnap forcedSnap)
{
    const Routine* live = comp.routineInSlot(r.slot);
    if (live == nullptr || live->uuid != r.uuid)
        return;
    const bool easedBefore = r.restore && !r.jump;
    const RoutineSnap snapBefore = r.ownSnap;
    r.ownSnap = toSnap(live->quantize);
    r.loop = live->loop;
    r.restore = live->restoreState;
    r.jump = live->restoreStyle == Routine::RestoreStyle::Jump;
    const bool eased = r.restore && !r.jump;
    if (easedBefore && !eased)
    {
        releaseGlides(r);
    }
    else if (eased && beatAvailable_ && (!easedBefore || r.ownSnap != snapBefore))
    {
        const double now = clock_.now().beat;
        scheduleGlides(r, now + beatsUntilBoundary(effectiveSnap(forcedSnap, r.ownSnap)),
                       [now](const ControlPath&) { return now; });
    }
}

void RoutineEngine::tick(const FeatureSnapshot& snap, double wallNow, const Composition& comp,
                         RoutineSnap forcedSnap, bool beatAvailable)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

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
    lastBeatInBar_ = snap.beatInBar;
    haveTicked_ = true;
    beatAvailable_ = beatAvailable;

    // Step 3: an index loop with NO erase inside it (F5); finished routines are marked `done`.
    for (size_t i = 0; i < running_.size(); ++i)
    {
        Running& r = running_[i];
        if (r.pending)
            resyncPending(r, comp, forcedSnap);   // s-rta-0927 fix round: a menu edit made while waiting counts
        const RoutineSnap mode = effectiveSnap(forcedSnap, r.ownSnap);

        if (r.pending)
        {
            if (dueNow(mode, beatEdge, barEdge))
                startNow(r);
            else
                stepGlides(r);   // plan3 C: the restore glides in over the last beat of the wait
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

            // loop / restore / restore-style edits made while running land at the end (plan 5.1 `set`).
            if (const Routine* live = comp.routineInSlot(r.slot); live != nullptr && live->uuid == r.uuid)
            {
                r.loop = live->loop;
                r.restore = live->restoreState;
                r.jump = live->restoreStyle == Routine::RestoreStyle::Jump;
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
                    // plan3 C: the continuous half glided back over the cycle's last beat; a pending restart
                    // keeps its own glides (scheduled for ITS boundary) for its start.
                    const bool glidePath = r.glideScheduled;
                    if (!r.restartRequested)
                        r.glideScheduled = false;
                    const int discreteRefused = r.player->firePreambleDiscrete(*r.sink);
                    r.preambleFired += static_cast<int>(r.program->preamble.size()) - discreteRefused;
                    r.preambleRefused += discreteRefused;
                    if (!glidePath)
                    {
                        const int continuousRefused = r.player->firePreambleContinuous(*r.sink);
                        r.preambleFired += static_cast<int>(r.program->preambleContinuous.size()) - continuousRefused;
                        r.preambleRefused += continuousRefused;
                    }
                    r.player->advanceTo(pos, *r.sink);
                    stepGlides(r);   // due return glides land here (their t1 is this new startBeat); spills go on
                }
                else
                {
                    releaseGlides(r);   // restore switched off inside the last beat: let go where they are
                    r.player->advanceTo(pos, *r.sink);
                }
                r.position = pos;
            }
            else
            {
                // Once: the look HOLDS (manual values stay; a connected control glides back to its
                // signal -- D8 hand-back). plan3 C: a return glide in flight (loop switched off inside
                // the last beat) is let go where it is -- never a leaked grip.
                releaseGlides(r);
                releaseOwnership(r.slot);
                r.position = r.lengthBeats;
                r.done = true;
                rememberRun(r);   // s-rta-0927: the idle pad keeps this run's "!"
            }
            continue;
        }

        r.player->advanceTo(pos, *r.sink);
        r.position = pos;

        // plan3 C: once per cycle, when its last beat begins, the loop return is scheduled against the LIVE
        // loop / restore settings (the lookup the end uses). A requested restart owns the next restore.
        // A Jump routine schedules nothing: its return lands in one call ON the loop point (s-rta-0926b).
        if (r.glideCycle != r.cycle && pos >= r.lengthBeats - kRestoreGlideBeats)
        {
            r.glideCycle = r.cycle;
            bool loop = r.loop, restore = r.restore, jump = r.jump;
            if (const Routine* live = comp.routineInSlot(r.slot); live != nullptr && live->uuid == r.uuid)
            {
                loop = live->loop;
                restore = live->restoreState;
                jump = live->restoreStyle == Routine::RestoreStyle::Jump;
            }
            if (loop && restore && !jump && beatAvailable_ && !r.restartRequested && r.lengthBeats > 0.0)
            {
                const double startBeat = r.startBeat, length = r.lengthBeats;
                scheduleGlides(r, startBeat + length, [&r, startBeat, length, pos](const ControlPath& key) {
                    return startBeat + std::min(busyUntil(*r.program, key, pos, length), length);
                });
            }
        }
        stepGlides(r);   // after advanceTo: a same-tick Player touch cancels the glide before it writes
    }
    std::erase_if(running_, [](const Running& r) { return r.done; });

    refreshBank(comp);
    publishStatus();
}

std::string RoutineEngine::fire(const Composition& comp, int slot, RoutineSnap forcedSnap, bool beatAvailable)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    lastForcedSnap_ = forcedSnap;   // s-rta-0927: a pending slot's `startsOn`

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
            const bool newRequest = !r.restartRequested;
            r.restartRequested = true;
            r.ownSnap = toSnap(routine->quantize);
            r.loop = routine->loop;
            r.restore = routine->restoreState;
            r.jump = routine->restoreStyle == Routine::RestoreStyle::Jump;
            // plan3 C: the restart's restore glides onto ITS boundary by the same rule, never touching a
            // knob while the recording's own hand is on it before that boundary. Jump: it lands in one call
            // on the restart's boundary; a return glide still in flight from an Ease setting lets go now.
            if (newRequest && r.jump)
            {
                releaseGlides(r);
            }
            else if (newRequest && r.restore && beatAvailable)
            {
                const double boundary = clock_.now().beat + beatsUntilBoundary(effectiveSnap(forcedSnap, r.ownSnap));
                const double startBeat = r.startBeat, endPos = boundary - r.startBeat, pos = r.position;
                scheduleGlides(r, boundary, [&r, startBeat, endPos, pos](const ControlPath& key) {
                    return startBeat + std::min(busyUntil(*r.program, key, pos, endPos), endPos);
                });
            }
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
    r.jump = routine->restoreStyle == Routine::RestoreStyle::Jump;
    r.lengthBeats = r.program->length;
    {
        auto fp = footprint(*r.program);   // s-rta-0927: where it plays, once
        r.deck = fp.deck;
        r.layers = std::move(fp.layers);
        r.touchesComp = fp.touchesComp;
    }
    running_.push_back(std::move(r));
    ++fires_;
    running_.back().fireSeq = static_cast<uint32_t>(fires_);
    lastError_.clear();

    Running& added = running_.back();
    if (!beatAvailable)
    {
        // A start that waits for a bar that may never come is worse than an immediate one (the
        // same honesty as quantizeModeToForcedSnap for a clip trigger). plan3 C: no glide either --
        // the clock is frozen without a beat, so a beat-long window would never end.
        notify("Routine " + added.name + ": no beat yet -- the routine starts now.");
        startNow(added);
    }
    else
    {
        // plan3 C: the restore glides over the last beat before the boundary it will start on; with
        // Quantize Off the boundary is now (a quarter-beat spill that begins inside this call). Jump: no
        // glide -- the whole restore lands in one call ON the boundary (startNow's one-call path).
        const RoutineSnap mode = effectiveSnap(forcedSnap, added.ownSnap);
        const double now = clock_.now().beat;
        if (added.restore && !added.jump)
            scheduleGlides(added, now + beatsUntilBoundary(mode), [now](const ControlPath&) { return now; });
        if (mode == RoutineSnap::Off)
            startNow(added);
    }
    publishStatus();
    return {};
}

void RoutineEngine::stop(int slot)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    for (auto& r : running_)
    {
        if (r.slot != slot)
            continue;
        releaseGlides(r);          // plan3 C: a glide lets go where it has the knob (R9)
        r.player->stop(*r.sink);   // every grip released (R9)
        rememberRun(r);            // s-rta-0927: the idle pad keeps this run's "!"
    }
    releaseOwnership(slot);
    std::erase_if(running_, [slot](const Running& r) { return r.slot == slot; });
    publishStatus();
}

void RoutineEngine::stopAll()
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    for (auto& r : running_)
    {
        releaseGlides(r);
        r.player->stop(*r.sink);
    }
    laneOwner_.clear();
    running_.clear();
    for (auto& lr : lastRun_)
        lr = LastRun{};   // s-rta-0927: a composition load / Stop / shutdown starts every pad clean
    publishStatus();
}

void RoutineEngine::stopOnLayer(int deck, int layer)
{
    ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD();
    std::vector<int> slots;
    for (const auto& r : running_)
        if (r.deck == deck && std::find(r.layers.begin(), r.layers.end(), layer) != r.layers.end())
            slots.push_back(r.slot);
    for (int slot : slots)
        stop(slot);   // whole routine, every grip released; publishes
}

void RoutineEngine::rememberRun(const Running& r)
{
    if (r.slot < 0 || r.slot >= kBankSize)
        return;
    const auto& report = r.program->report;
    auto& lr = lastRun_[r.slot];
    lr.uuid = r.uuid;
    lr.unresolved = static_cast<int>(report.unresolved.size());
    lr.preambleUnresolved = static_cast<int>(report.preambleUnresolved.size());
    lr.skipped = r.sink->skipped;
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
            s.restoreStyle = Routine::restoreStyleToString(r->restoreStyle);
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
    {
        s.slots[i] = bank_[i];
        // s-rta-0927: an idle pad of the routine that last ran here keeps that run's warning counters.
        const auto& lr = lastRun_[i];
        if (!lr.uuid.empty() && lr.uuid == s.slots[i].uuid)
        {
            s.slots[i].unresolved = lr.unresolved;
            s.slots[i].preambleUnresolved = lr.preambleUnresolved;
            s.slots[i].skipped = lr.skipped;
        }
    }

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
        sl.glides = static_cast<int>(std::count_if(r.glides.begin(), r.glides.end(),
                                                   [](const Glide& g) { return g.started; }));
        sl.deck = r.deck;
        sl.layers = r.layers;
        sl.touchesComp = r.touchesComp;
        sl.restartPending = r.restartRequested;
        sl.fireSeq = r.fireSeq;
        sl.startsOn = r.pending ? snapWord(effectiveSnap(lastForcedSnap_, r.ownSnap)) : "";
    }

    std::lock_guard<std::mutex> lock(statusMutex_);
    published_ = std::move(s);
}

RoutineEngine::Status RoutineEngine::status() const
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    return published_;
}
