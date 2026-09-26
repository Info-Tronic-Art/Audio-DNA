#include "recording/RoutineSlice.h"
#include "connect/ScalarParams.h"
#include "effects/EffectLibrary.h"
#include <algorithm>
#include <cmath>
#include <set>
#include <tuple>

namespace
{
    constexpr double kBeatsPerBar = 4.0;

    enum class Source { Lanes, Checkpoint, Defaults, Unknown };

    void count(SliceResult& out, Source s)
    {
        switch (s)
        {
            case Source::Lanes:      ++out.preambleFromLanes; break;
            case Source::Checkpoint: ++out.preambleFromCheckpoint; break;
            case Source::Defaults:   ++out.preambleFromDefaults; break;
            case Source::Unknown:    ++out.preambleUnknown; break;
        }
    }

    // Plan 3.3 step 2: a routine is hands on layers/clips/comp scalars, not the transport. Tempo,
    // audio transport, deck switches and the global quantize setting are dropped by name, and so
    // is every macro/routine lane (macro capture is LATER). Additive later.
    bool isTransportLane(const ControlPath& key)
    {
        if (key.scope == ControlPath::Scope::Macro || key.scope == ControlPath::Scope::Routine)
            return true;
        if (key.scope == ControlPath::Scope::Comp)
            return key.control == "tempo" || key.control == "audio"
                || key.control == "activeDeck" || key.control == "quantize";
        return false;
    }

    std::string laneName(const ControlPath& key)
    {
        switch (key.scope)
        {
            case ControlPath::Scope::Comp:    return key.control;
            case ControlPath::Scope::Macro:   return "macro " + std::to_string(key.macro + 1);
            case ControlPath::Scope::Routine: return "routine";
            case ControlPath::Scope::Clip:
            case ControlPath::Scope::Layer:   break;
        }
        return std::string(ControlPath::scopeToString(key.scope)) + " " + key.control;
    }

    // Plan 3.3 step 1 (D4 step 4): a routine lives on the beat grid, so a stretch recorded while the
    // tempo was unknown (bpm 0: TempoMap::beatAt freezes there) cannot be cut into one.
    std::string checkMetered(const TempoMap& tempo, double fromBeat, double toBeat)
    {
        if (tempo.a.empty())
            return "this take has no beat grid (no tempo was recorded)";
        static const std::string unmetered = "this stretch has no beat; the tempo was unknown while it was recorded";
        const TempoAnchor* bracketing = nullptr;
        for (const auto& anchor : tempo.a)
            if (anchor.beat <= fromBeat)
                bracketing = &anchor;
        if (bracketing != nullptr && bracketing->bpm <= 0.0f)
            return unmetered;
        for (const auto& anchor : tempo.a)
            if (anchor.beat >= fromBeat && anchor.beat <= toBeat && anchor.bpm <= 0.0f)
                return unmetered;
        return {};
    }

    // Checkpoint 0 is looked up POSITIONALLY by the lane key's recorded deck/layer: it was captured
    // against the same composition the lane was recorded on (plan 3.3 step 4).
    const PerfState::LayerRuntime* checkpointLayer(const PerfState& cp0, const ControlPath& key)
    {
        auto deckIt = cp0.decks.find(key.deck);
        if (deckIt == cp0.decks.end())
            return nullptr;
        auto layerIt = deckIt->second.layers.find(key.layer);
        return layerIt == deckIt->second.layers.end() ? nullptr : &layerIt->second;
    }

    const PerfState::ClipRuntime* checkpointClip(const PerfState::LayerRuntime& lr, int col)
    {
        auto it = lr.clips.find(col);
        return it == lr.clips.end() ? nullptr : &it->second;
    }

    // Absent from checkpoint 0 = it was at its DEFAULT at Record (PerfStateCapture keeps
    // non-default values only) -> the effect's registered default.
    Source libraryDefault(const EffectLibrary& library, const ControlPath& key, float& norm)
    {
        const auto* def = library.getEffectDef(juce::String(key.fxName));
        if (def == nullptr || key.param < 0 || key.param >= static_cast<int>(def->params.size()))
            return Source::Unknown;
        norm = def->params[static_cast<size_t>(key.param)].defaultValue;
        return Source::Defaults;
    }

    void setPlayState(Routine::PreambleEntry& e, bool playing)
    {
        e.v = playing ? 1 : 0;
        e.action = playing ? "resume" : "pause";
    }

    // Discrete restore value from checkpoint 0 (plan 3.3 step 4).
    Source checkpointDiscrete(const PerfState& cp0, const ControlPath& key, Routine::PreambleEntry& e)
    {
        if (key.scope != ControlPath::Scope::Layer && key.scope != ControlPath::Scope::Clip)
            return Source::Unknown;
        const auto* lr = checkpointLayer(cp0, key);
        if (lr == nullptr)
            return Source::Unknown;

        if (key.scope == ControlPath::Scope::Layer)
        {
            if (key.control == "activeClip") { e.v = lr->activeClipColumn; return Source::Checkpoint; }
            if (key.control == "visible")    { e.v = lr->visible ? 1 : 0; return Source::Checkpoint; }
            if (key.control == "bypass")     { e.v = lr->bypassed ? 1 : 0; return Source::Checkpoint; }
            if (key.control == "solo")       { e.v = lr->solo ? 1 : 0; return Source::Checkpoint; }
            if (key.control == "mute")       { e.v = lr->muted ? 1 : 0; return Source::Checkpoint; }
            if (key.control == "autopilot")  { e.v = lr->autopilotEnabled ? 1 : 0; return Source::Checkpoint; }
            return Source::Unknown;
        }

        if (key.control == "playing")
        {
            // Absent = default = paused (PerfStateCapture stores a clip only when non-default).
            const auto* cr = checkpointClip(*lr, key.col);
            setPlayState(e, cr != nullptr && cr->playing);
            return Source::Checkpoint;
        }
        return Source::Unknown;   // e.g. an effect slot's bypass: not in PerfState
    }

    // Continuous restore value (normalised) from checkpoint 0, else the control's default (plan 3.3
    // step 5). Composition scalars / global effects, clip source params, dry/wet and speed are not
    // in PerfState -> Unknown (counted, no entry).
    Source checkpointContinuous(const PerfState& cp0, const ControlPath& key, const EffectLibrary& library,
                                float& norm)
    {
        if (key.scope != ControlPath::Scope::Layer && key.scope != ControlPath::Scope::Clip)
            return Source::Unknown;
        const auto* lr = checkpointLayer(cp0, key);
        if (lr == nullptr)
            return Source::Unknown;

        if (key.scope == ControlPath::Scope::Layer)
        {
            if (key.control == "scalar" && key.fx < 0 && key.scalar == "opacity")
            {
                norm = layerScalarDefs()[static_cast<size_t>(LayerScalar::Opacity)].toNorm(lr->opacity);
                return Source::Checkpoint;
            }
            if (key.control == "param" && key.fx >= 0 && key.param >= 0)
            {
                auto it = lr->effectParams.find(PerfState::fxParamKey(key.fx, key.param));
                if (it != lr->effectParams.end()) { norm = it->second; return Source::Checkpoint; }
                return libraryDefault(library, key, norm);
            }
            return Source::Unknown;   // other layer scalars are never captured into PerfState
        }

        const auto* cr = checkpointClip(*lr, key.col);
        if (key.control == "scalar" && key.fx < 0)
        {
            for (const auto& def : clipScalarDefs())
            {
                if (key.scalar != def.key)
                    continue;
                if (cr != nullptr)
                {
                    auto it = cr->scalars.find(key.scalar);
                    if (it != cr->scalars.end()) { norm = it->second; return Source::Checkpoint; }
                }
                norm = def.defaultNorm;
                return Source::Defaults;
            }
            return Source::Unknown;
        }
        if (key.control == "param" && key.fx >= 0 && key.param >= 0)
        {
            if (cr != nullptr)
            {
                auto it = cr->effectParams.find(PerfState::fxParamKey(key.fx, key.param));
                if (it != cr->effectParams.end()) { norm = it->second; return Source::Checkpoint; }
            }
            return libraryDefault(library, key, norm);
        }
        return Source::Unknown;
    }

    // Interpolation of the segment that contains `x` (the one leaving the last breakpoint at or
    // before x), so a synthesized breakpoint continues the recorded shape.
    Breakpoint::Interp interpAt(const std::vector<Breakpoint>& pts, double x)
    {
        Breakpoint::Interp interp = Breakpoint::Interp::Linear;
        for (const auto& bp : pts)
            if (bp.x <= x)
                interp = bp.interp;
        return interp;
    }

    // Plan 3.3 step 5 (D4 step 3): keep the breakpoints inside [fromBeat, toBeat), synthesize a first
    // one at fromBeat for a gesture straddling the start and a last one at toBeat for a gesture
    // straddling the end, rebase to routine beats, drop the stamps (Beat clock only), keep the grip.
    Gesture cutGesture(const Gesture& g, double fromBeat, double toBeat)
    {
        Gesture out = g;
        out.stamps.clear();
        out.curve.pts.clear();
        const auto& pts = g.curve.pts;

        const bool hasPointAtStart = std::any_of(pts.begin(), pts.end(),
                                                 [&](const Breakpoint& bp) { return bp.x == fromBeat; });
        if (pts.front().x < fromBeat && !hasPointAtStart)
        {
            Breakpoint b;
            b.x = fromBeat;
            b.y = g.curve.eval(fromBeat);
            b.interp = interpAt(pts, fromBeat);
            out.curve.pts.push_back(b);
        }
        for (const auto& bp : pts)
            if (bp.x >= fromBeat && bp.x < toBeat)
                out.curve.pts.push_back(bp);
        if (pts.back().x >= toBeat)
        {
            Breakpoint b;
            b.x = toBeat;
            b.y = g.curve.eval(toBeat);
            b.interp = interpAt(pts, toBeat);
            out.curve.pts.push_back(b);
        }
        for (auto& bp : out.curve.pts)
            bp.x -= fromBeat;
        return out;
    }

    // A clip's name as recorded anywhere in the take (lane keys first, then checkpoint 0), so a
    // synthesized play/pause entry carries the same name check D2 applies to a recorded one.
    std::string recordedClipName(const Take& take, int deck, int layer, int col)
    {
        for (const auto& [key, lane] : take.lanes)
            if (key.scope == ControlPath::Scope::Clip && key.deck == deck && key.layer == layer
                && key.col == col && !key.clipName.empty())
                return key.clipName;
        ControlPath probe; probe.deck = deck; probe.layer = layer;
        if (const auto* lr = checkpointLayer(take.checkpoint0, probe))
            if (const auto* cr = checkpointClip(*lr, col))
                return cr->clip;
        return {};
    }
}

SliceResult sliceRoutine(const Take& take, const SliceRequest& req, const EffectLibrary& library)
{
    SliceResult out;
    const double fromBeat = req.fromBeat;
    const double toBeat = req.toBeat;

    // Step 1: refusals.
    if (!(toBeat > fromBeat))
    {
        out.error = "the end of the range must come after its start";
        return out;
    }
    if (auto err = checkMetered(take.tempo, fromBeat, toBeat); !err.empty())
    {
        out.error = err;
        return out;
    }

    Routine routine;
    routine.uuid = juce::Uuid().toString().toStdString();
    routine.name = req.name;
    routine.source = { req.takeFolder, fromBeat, toBeat };

    // Step 6: length, rounded UP to whole bars by default (D4 step 5).
    double length = toBeat - fromBeat;
    if (req.wholeBars)
        length = std::ceil(length / kBeatsPerBar - 1e-9) * kBeatsPerBar;
    routine.lengthBeats = length;

    // Restore list, in the load-bearing order (3.1 / R5): layer flags (and any other discrete
    // control), then activeClip, then clip play/pause; continuous entries last (firePreamble fires
    // all discrete before all continuous anyway).
    std::vector<Routine::PreambleEntry> flags, activeClips, playing, continuous;
    std::set<std::tuple<int, int, int>> clipsWithPlayingLane;   // (deck, layer, col)
    std::vector<ControlPath> restoredActiveClips;              // activeClip entries with column >= 0

    for (const auto& [recordedKey, lane] : take.lanes)
    {
        if (lane.kind == Lane::Kind::Opaque)
            continue;   // unparseable -- never dispatched (D12 rule 3)
        if (isTransportLane(recordedKey))
        {
            out.droppedLanes.push_back(laneName(recordedKey));
            continue;
        }

        // Step 3: keys resolve on the ACTIVE deck at fire time (positional deck index kept for
        // display). Comp keys carry no deck at all.
        ControlPath key = recordedKey;
        if (key.scope == ControlPath::Scope::Layer || key.scope == ControlPath::Scope::Clip)
            key.deckRelative = true;

        Lane cut;
        cut.key = key;
        cut.kind = lane.kind;

        if (lane.kind == Lane::Kind::Discrete)
        {
            // Step 4: keep [fromBeat, toBeat), rebase, drop wall/sample stamps (seq stays: it is the
            // (at, seq) tie-break).
            const DiscretePoint* before = nullptr;
            for (const auto& p : lane.points)
            {
                if (p.beat < fromBeat) { before = &p; continue; }
                if (p.beat >= toBeat) continue;
                DiscretePoint q = p;
                q.beat -= fromBeat;
                q.s.t = 0.0;
                q.s.sample = 0;
                cut.points.push_back(std::move(q));
            }

            Routine::PreambleEntry e;
            e.key = key;
            Source src = Source::Lanes;
            if (before != nullptr)
            {
                e.v = before->v;
                e.action = before->action;
                if (key.control == "playing")
                    setPlayState(e, before->v != 0);   // restore the STATE the point left, never a toggle
            }
            else
            {
                src = checkpointDiscrete(take.checkpoint0, recordedKey, e);
            }
            count(out, src);

            if (src != Source::Unknown)
            {
                if (key.control == "activeClip" && key.scope == ControlPath::Scope::Layer)
                {
                    activeClips.push_back(e);
                    if (e.v >= 0)
                    {
                        ControlPath clipKey = key;   // layer key + the restored column
                        clipKey.col = e.v;
                        restoredActiveClips.push_back(clipKey);
                    }
                }
                else if (key.control == "playing" && key.scope == ControlPath::Scope::Clip)
                {
                    playing.push_back(e);
                    clipsWithPlayingLane.insert({ key.deck, key.layer, key.col });
                }
                else
                {
                    flags.push_back(e);
                }
            }
        }
        else
        {
            // Step 5: keep gestures overlapping [fromBeat, toBeat).
            bool covered = false;
            const Gesture* lastBefore = nullptr;
            for (const auto& g : lane.gestures)
            {
                if (g.curve.pts.empty())
                    continue;
                const double x0 = g.curve.pts.front().x;
                const double x1 = g.curve.pts.back().x;
                if (x1 <= fromBeat)
                {
                    if (lastBefore == nullptr || x1 >= lastBefore->curve.pts.back().x)
                        lastBefore = &g;
                    continue;
                }
                if (x0 >= toBeat)
                    continue;
                if (x0 <= fromBeat)
                    covered = true;   // its (synthesized) begin at x = 0 IS the state
                cut.gestures.push_back(cutGesture(g, fromBeat, toBeat));
            }

            if (!covered)
            {
                Routine::PreambleEntry e;
                e.key = key;
                e.continuous = true;
                Source src = Source::Lanes;
                if (lastBefore != nullptr)
                    e.norm = lastBefore->curve.pts.back().y;
                else
                    src = checkpointContinuous(take.checkpoint0, recordedKey, library, e.norm);
                count(out, src);
                if (src != Source::Unknown)
                    continuous.push_back(e);
            }
        }

        // A control with no movement inside the cut gets no timeline (its restore entry above
        // still puts it back).
        if (!cut.points.empty() || !cut.gestures.empty())
            routine.lanes[key] = std::move(cut);
    }

    // Step 4 (R5): every restored activeClip column also gets its clip's play/pause state, AFTER
    // the trigger -- the trigger auto-plays a never-triggered clip (Layer.h). Same rule as a
    // recorded `playing` lane: that lane's own entry when one exists, else checkpoint 0.
    for (const auto& layerKey : restoredActiveClips)
    {
        const int col = layerKey.col;
        if (clipsWithPlayingLane.count({ layerKey.deck, layerKey.layer, col }) > 0)
            continue;
        Routine::PreambleEntry e;
        e.key = layerKey;
        e.key.scope = ControlPath::Scope::Clip;
        e.key.col = col;
        e.key.control = "playing";
        e.key.clipName = recordedClipName(take, layerKey.deck, layerKey.layer, col);
        ControlPath recorded = e.key;
        recorded.deckRelative = false;
        const Source src = checkpointDiscrete(take.checkpoint0, recorded, e);
        count(out, src);
        if (src != Source::Unknown)
            playing.push_back(e);
    }

    // Step 7.
    for (auto* group : { &flags, &activeClips, &playing, &continuous })
        routine.preamble.insert(routine.preamble.end(), group->begin(), group->end());

    out.routine = std::move(routine);
    return out;
}

double takeBeatOfBar(const Take& take, int bar)
{
    const double start = take.meta.startBeatInBar >= 0.0 ? take.meta.startBeatInBar : 0.0;
    return (bar - 1) * kBeatsPerBar + std::fmod(kBeatsPerBar - start, kBeatsPerBar);
}
