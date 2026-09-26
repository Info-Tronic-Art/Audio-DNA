#include "recording/Program.h"
#include "model/Composition.h"
#include "connect/ScalarParams.h"
#include <algorithm>
#include <cmath>

namespace
{
    // D2's tri-state resolution outcome for ONE level of a ControlPath
    // (deck, layer, col, fx, ...). Combined across levels (worst wins:
    // Missing > NameOnly > PositionOnly > ExactMatch) to get the whole
    // lane's CompileReport bucket.
    enum class LevelOutcome { ExactMatch, PositionOnly, NameOnly, Missing };

    LevelOutcome combine(LevelOutcome a, LevelOutcome b)
    {
        if (a == LevelOutcome::Missing || b == LevelOutcome::Missing) return LevelOutcome::Missing;
        if (a == LevelOutcome::NameOnly || b == LevelOutcome::NameOnly) return LevelOutcome::NameOnly;
        if (a == LevelOutcome::PositionOnly || b == LevelOutcome::PositionOnly) return LevelOutcome::PositionOnly;
        return LevelOutcome::ExactMatch;
    }

    struct LevelResult { LevelOutcome outcome; int index; };

    LevelResult resolveDeck(const Composition& comp, const ControlPath& key)
    {
        if (key.deckRelative)
        {
            const int idx = comp.activeDeckIndex;
            const bool valid = idx >= 0 && idx < static_cast<int>(comp.decks.size());
            return { valid ? LevelOutcome::ExactMatch : LevelOutcome::Missing, valid ? idx : -1 };
        }

        const int idx = key.deck;
        if (idx >= 0 && idx < static_cast<int>(comp.decks.size()))
        {
            const bool nameMatches = comp.decks[static_cast<size_t>(idx)].name == key.deckName;
            return { nameMatches ? LevelOutcome::ExactMatch : LevelOutcome::PositionOnly, idx };
        }
        if (!key.deckName.empty())
            for (size_t i = 0; i < comp.decks.size(); ++i)
                if (comp.decks[i].name == key.deckName)
                    return { LevelOutcome::NameOnly, static_cast<int>(i) };
        return { LevelOutcome::Missing, -1 };
    }

    LevelResult resolveLayer(const Deck& deck, const ControlPath& key)
    {
        const int idx = key.layer;
        if (idx >= 0 && idx < static_cast<int>(deck.layers.size()))
        {
            const bool nameMatches = deck.layers[static_cast<size_t>(idx)].name == key.layerName;
            return { nameMatches ? LevelOutcome::ExactMatch : LevelOutcome::PositionOnly, idx };
        }
        if (!key.layerName.empty())
            for (size_t i = 0; i < deck.layers.size(); ++i)
                if (deck.layers[i].name == key.layerName)
                    return { LevelOutcome::NameOnly, static_cast<int>(i) };
        return { LevelOutcome::Missing, -1 };
    }

    LevelResult resolveCol(const Layer& layer, const ControlPath& key)
    {
        const int idx = key.col;
        if (idx >= 0 && idx < static_cast<int>(layer.clips.size()) && layer.clips[static_cast<size_t>(idx)].has_value())
        {
            const bool nameMatches = layer.clips[static_cast<size_t>(idx)]->name == key.clipName;
            return { nameMatches ? LevelOutcome::ExactMatch : LevelOutcome::PositionOnly, idx };
        }
        if (!key.clipName.empty())
            for (size_t i = 0; i < layer.clips.size(); ++i)
                if (layer.clips[i].has_value() && layer.clips[i]->name == key.clipName)
                    return { LevelOutcome::NameOnly, static_cast<int>(i) };
        return { LevelOutcome::Missing, -1 };
    }

    LevelResult resolveFx(const std::vector<Clip::EffectSlot>& fxList, const ControlPath& key)
    {
        const int idx = key.fx;
        if (idx >= 0 && idx < static_cast<int>(fxList.size()))
        {
            const bool nameMatches = fxList[static_cast<size_t>(idx)].effectName == key.fxName;
            return { nameMatches ? LevelOutcome::ExactMatch : LevelOutcome::PositionOnly, idx };
        }
        if (!key.fxName.empty())
            for (size_t i = 0; i < fxList.size(); ++i)
                if (fxList[i].effectName == key.fxName)
                    return { LevelOutcome::NameOnly, static_cast<int>(i) };
        return { LevelOutcome::Missing, -1 };
    }

    struct WholeResult { LevelOutcome outcome; ResolvedTarget target; std::string reason; };

    // The one place a ControlPath's coordinates are trusted (D2). See
    // Program.h's header comment for row 1's disclosed depth limits.
    WholeResult resolveKey(const Composition& comp, const ControlPath& key)
    {
        ResolvedTarget target;

        if (key.scope == ControlPath::Scope::Comp)
            return { LevelOutcome::ExactMatch, target, "" };   // one Composition -- always addressable

        if (key.scope == ControlPath::Scope::Macro || key.scope == ControlPath::Scope::Routine)
            return { LevelOutcome::Missing, target, "scope not resolved in row 1 (macro/routine capture is LATER)" };

        // Clip and Layer scopes both need deck -> layer.
        auto deckR = resolveDeck(comp, key);
        target.deck = deckR.index;
        if (deckR.outcome == LevelOutcome::Missing)
            return { LevelOutcome::Missing, target, "deck not found" };
        const Deck& deck = comp.decks[static_cast<size_t>(deckR.index)];

        auto layerR = resolveLayer(deck, key);
        target.layer = layerR.index;
        LevelOutcome overall = combine(deckR.outcome, layerR.outcome);
        if (layerR.outcome == LevelOutcome::Missing)
            return { LevelOutcome::Missing, target, "layer not found" };
        const Layer& layer = deck.layers[static_cast<size_t>(layerR.index)];

        const Clip* clip = nullptr;
        if (key.scope == ControlPath::Scope::Clip)
        {
            auto colR = resolveCol(layer, key);
            target.col = colR.index;
            overall = combine(overall, colR.outcome);
            if (colR.outcome == LevelOutcome::Missing)
                return { LevelOutcome::Missing, target, "clip not found" };
            clip = &layer.clips[static_cast<size_t>(colR.index)].value();
        }

        if (key.fx >= 0 || !key.fxName.empty())
        {
            const std::vector<Clip::EffectSlot>& fxList = clip ? clip->effects : layer.layerEffects;
            auto fxR = resolveFx(fxList, key);
            target.fx = fxR.index;
            overall = combine(overall, fxR.outcome);
            if (fxR.outcome == LevelOutcome::Missing)
                return { LevelOutcome::Missing, target, "effect slot not found" };

            if (key.control == "param")
            {
                const auto& slot = fxList[static_cast<size_t>(fxR.index)];
                const bool paramValid = key.param >= 0 && key.param < static_cast<int>(slot.paramValues.size());
                if (!paramValid)
                    return { LevelOutcome::Missing, target, "param index out of range (row 1: index-only, no name fallback)" };
                target.param = key.param;
            }
        }

        return { overall, target, "" };
    }

    // ==== s-rta-0925 plan section 3.2: D4's preamble ("restore checkpoint 0, then play") ====

    ControlPath compControlPath(const std::string& control)
    {
        ControlPath p;
        p.scope = ControlPath::Scope::Comp;
        p.control = control;
        return p;
    }

    // A Missing outcome is reported by the CALLER (it needs a specific reason, e.g. "layer not
    // found" vs "clip not found") -- this only folds a non-Missing, non-exact outcome into the
    // existing rebind buckets, tagged so a reader can tell a preamble rebind from a lane rebind.
    void addPreambleRebind(Program& out, LevelOutcome outcome, const ControlPath& key, const std::string& what)
    {
        if (outcome == LevelOutcome::PositionOnly)
            out.report.reboundByPosition.push_back({ key, "preamble: " + what + " rebound by position" });
        else if (outcome == LevelOutcome::NameOnly)
            out.report.reboundByName.push_back({ key, "preamble: " + what + " rebound by name" });
    }

    // One deck+layer, resolved ONCE (D2 tri-state) so every row emitted under it shares a single
    // rebind/unresolved Issue rather than one per row (plan section 3.2's emission-order table).
    struct ResolvedLayerCtx
    {
        ControlPath layerKey;   // scope=Layer; positional deck/layer + the names captured at record time
        ResolvedTarget target;  // .deck / .layer filled; .col/.fx/.param stay -1
        const Layer* layer = nullptr;
        const PerfState::LayerRuntime* rt = nullptr;
    };

    std::vector<ResolvedLayerCtx> resolvePreambleLayers(const PerfState& cp0, const Composition& comp, Program& out)
    {
        std::vector<ResolvedLayerCtx> ctxs;
        for (const auto& [deckIdx, deckRT] : cp0.decks)
        {
            ControlPath deckProbe;
            deckProbe.scope = ControlPath::Scope::Layer;
            deckProbe.deck = deckIdx;
            deckProbe.deckName = deckRT.deck;
            auto deckR = resolveDeck(comp, deckProbe);
            if (deckR.outcome == LevelOutcome::Missing)
            {
                out.report.preambleUnresolved.push_back({ deckProbe, "deck not found" });
                continue;
            }
            const Deck& deck = comp.decks[static_cast<size_t>(deckR.index)];

            for (const auto& [layerIdx, layerRT] : deckRT.layers)
            {
                ControlPath layerKey;
                layerKey.scope = ControlPath::Scope::Layer;
                layerKey.deck = deckIdx;
                layerKey.deckName = deckRT.deck;
                layerKey.layer = layerIdx;
                layerKey.layerName = layerRT.layer;

                auto layerR = resolveLayer(deck, layerKey);
                const LevelOutcome overall = combine(deckR.outcome, layerR.outcome);
                if (layerR.outcome == LevelOutcome::Missing)
                {
                    out.report.preambleUnresolved.push_back({ layerKey, "layer not found" });
                    continue;
                }
                addPreambleRebind(out, overall, layerKey, "layer");

                ResolvedTarget target;
                target.deck = deckR.index;
                target.layer = layerR.index;
                ctxs.push_back(ResolvedLayerCtx{ layerKey, target,
                    &deck.layers[static_cast<size_t>(layerR.index)], &layerRT });
            }
        }
        return ctxs;
    }

    void buildPreamble(const PerfState& cp0, const Composition& comp, Program& out)
    {
        // R9: a v1/checkpoint-less take (PerfState never captured -- activeDeckIndex still at its
        // struct default and no decks at all) synthesizes NO preamble entries -- behaves exactly as
        // today. A real arm() always calls capturePerfState() (RecorderHost::arm), so this only
        // happens for a legacy/hand-built take with no checkpoint0 section.
        if (cp0.decks.empty() && cp0.activeDeckIndex < 0)
            return;

        // Row 1: comp/activeDeck.
        {
            const ControlPath key = compControlPath("activeDeck");
            const bool valid = cp0.activeDeckIndex >= 0 && cp0.activeDeckIndex < static_cast<int>(comp.decks.size());
            if (valid)
            {
                DiscretePoint p; p.v = cp0.activeDeckIndex; p.origin = Origin::Preamble;
                out.preamble.push_back(Fired{ 0.0, 0, key, ResolvedTarget{}, std::move(p) });
                out.report.preambleCount++;
            }
            else
            {
                out.report.preambleUnresolved.push_back({ key, "deck index out of range" });
            }
        }

        // Row 2: comp/quantize -- Comp scope is always addressable (resolveKey never fails it).
        {
            DiscretePoint p; p.v = cp0.quantizeMode; p.origin = Origin::Preamble;
            out.preamble.push_back(Fired{ 0.0, 0, compControlPath("quantize"), ResolvedTarget{}, std::move(p) });
            out.report.preambleCount++;
        }

        // Pass A (rows 3-6): resolve every deck/layer ONCE, then emit flags/opacity/layer-fx/activeClip.
        std::vector<ResolvedLayerCtx> ctxs = resolvePreambleLayers(cp0, comp, out);
        for (auto& ctx : ctxs)
        {
            const auto& rt = *ctx.rt;
            auto controlKey = [&](const std::string& control)
            { ControlPath k = ctx.layerKey; k.control = control; return k; };

            auto emitFlag = [&](const std::string& name, bool value)
            {
                DiscretePoint p; p.v = value ? 1 : 0; p.origin = Origin::Preamble;
                out.preamble.push_back(Fired{ 0.0, 0, controlKey(name), ctx.target, std::move(p) });
                out.report.preambleCount++;
            };
            emitFlag("visible", rt.visible);
            emitFlag("bypass", rt.bypassed);
            emitFlag("solo", rt.solo);
            emitFlag("mute", rt.muted);
            emitFlag("autopilot", rt.autopilotEnabled);

            {
                // LayerRuntime::opacity is always captured (not gated on non-default, unlike
                // ClipRuntime) and is ALWAYS emitted here too, even when it equals the scalar
                // default -- s-rta-0925 rr-fix: a checkpoint value matching the default is NOT a
                // no-op write in general, because the LIVE value at Play time may have since
                // diverged from default (a human/REST perturbation between Record and Play). The
                // live gate's own recipe hits exactly this: checkpoint opacity is 1.0 (the
                // default), the probe perturbs it to 0.9, and a skip-if-default check here left it
                // stuck at 0.9 forever (preambleFired counted the write as absent, not refused).
                const auto& def = layerScalarDefs()[static_cast<size_t>(LayerScalar::Opacity)];
                const float norm = def.toNorm(rt.opacity);
                ControlPath k = controlKey("scalar"); k.scalar = "opacity";
                out.preambleContinuous.push_back(PreambleSet{ k, ctx.target, norm });
                out.report.preambleCount++;
            }

            for (const auto& [fxKey, value] : rt.effectParams)
            {
                const int slot = PerfState::fxParamSlot(fxKey);
                const int param = PerfState::fxParamIndex(fxKey);
                // Plan section 3.2 row 5: "no name in PerfState" -- index-only bounds check, never
                // the tri-state name-fallback resolveFx() runs for a recorded lane's fx.
                if (slot < 0 || slot >= static_cast<int>(ctx.layer->layerEffects.size()))
                {
                    ControlPath k = controlKey("param"); k.fx = slot; k.param = param;
                    out.report.preambleUnresolved.push_back({ k, "effect slot not found" });
                    continue;
                }
                ControlPath k = controlKey("param"); k.fx = slot; k.param = param;
                ResolvedTarget t = ctx.target; t.fx = slot; t.param = param;
                out.preambleContinuous.push_back(PreambleSet{ k, t, value });
                out.report.preambleCount++;
            }

            {
                const int col = rt.activeClipColumn;
                const int numCols = static_cast<int>(ctx.layer->clips.size());
                if (col >= numCols)
                {
                    out.report.preambleUnresolved.push_back({ controlKey("activeClip"), "clip index out of range" });
                }
                else
                {
                    DiscretePoint p; p.v = col; p.origin = Origin::Preamble;
                    out.preamble.push_back(Fired{ 0.0, 0, controlKey("activeClip"), ctx.target, std::move(p) });
                    out.report.preambleCount++;
                }
            }
        }

        // Pass B (rows 7-8): per captured clip, then the active column's play/pause -- run as a
        // SEPARATE pass over every layer (not interleaved with pass A) so every layer's trigger
        // (row 6) precedes every layer's play/pause (row 8): R4 -- Layer::triggerClipImmediate
        // auto-plays a never-triggered clip, so the trigger must land first or the recorded
        // play/pause state would be immediately overridden by the auto-play.
        for (auto& ctx : ctxs)
        {
            const auto& rt = *ctx.rt;

            for (const auto& [col, clipRT] : rt.clips)
            {
                ControlPath colProbe; colProbe.col = col; colProbe.clipName = clipRT.clip;
                auto colR = resolveCol(*ctx.layer, colProbe);
                if (colR.outcome == LevelOutcome::Missing)
                {
                    ControlPath k = ctx.layerKey; k.scope = ControlPath::Scope::Clip;
                    k.col = col; k.clipName = clipRT.clip;
                    out.report.preambleUnresolved.push_back({ k, "clip not found" });
                    continue;
                }
                {
                    ControlPath rebindKey = ctx.layerKey; rebindKey.scope = ControlPath::Scope::Clip;
                    rebindKey.col = colR.index; rebindKey.clipName = clipRT.clip;
                    addPreambleRebind(out, colR.outcome, rebindKey, "clip");
                }

                ResolvedTarget clipTarget = ctx.target; clipTarget.col = colR.index;
                const Clip& liveClip = ctx.layer->clips[static_cast<size_t>(colR.index)].value();
                auto clipControlKey = [&](const std::string& control)
                {
                    ControlPath k = ctx.layerKey; k.scope = ControlPath::Scope::Clip;
                    k.col = colR.index; k.clipName = clipRT.clip; k.control = control; return k;
                };

                for (const auto& [fxKey, value] : clipRT.effectParams)
                {
                    const int slot = PerfState::fxParamSlot(fxKey);
                    const int param = PerfState::fxParamIndex(fxKey);
                    if (slot < 0 || slot >= static_cast<int>(liveClip.effects.size()))
                    {
                        ControlPath k = clipControlKey("param"); k.fx = slot; k.param = param;
                        out.report.preambleUnresolved.push_back({ k, "effect slot not found" });
                        continue;
                    }
                    ControlPath k = clipControlKey("param"); k.fx = slot; k.param = param;
                    ResolvedTarget t = clipTarget; t.fx = slot; t.param = param;
                    out.preambleContinuous.push_back(PreambleSet{ k, t, value });
                    out.report.preambleCount++;
                }

                for (const auto& [scalarKey, value] : clipRT.scalars)
                {
                    ControlPath k = clipControlKey("scalar"); k.scalar = scalarKey;
                    out.preambleContinuous.push_back(PreambleSet{ k, clipTarget, value });
                    out.report.preambleCount++;
                }
            }

            if (rt.activeClipColumn >= 0)
            {
                const auto it = rt.clips.find(rt.activeClipColumn);
                ControlPath colProbe; colProbe.col = rt.activeClipColumn;
                if (it != rt.clips.end()) colProbe.clipName = it->second.clip;
                auto colR = resolveCol(*ctx.layer, colProbe);
                if (colR.outcome != LevelOutcome::Missing)
                {
                    // D4 table row 8: "a clip that was playing is always non-default" (PerfStateCapture
                    // only stores a ClipRuntime for a clip that differs from default), so the ABSENCE
                    // of a ClipRuntime for the active column means it was paused -- never assume playing.
                    const bool playing = it != rt.clips.end() && it->second.playing;
                    ControlPath k = ctx.layerKey; k.scope = ControlPath::Scope::Clip;
                    k.col = colR.index; k.clipName = colProbe.clipName; k.control = "playing";
                    ResolvedTarget t = ctx.target; t.col = colR.index;
                    DiscretePoint p; p.action = playing ? "resume" : "pause"; p.origin = Origin::Preamble;
                    out.preamble.push_back(Fired{ 0.0, 0, k, t, std::move(p) });
                    out.report.preambleCount++;
                }
                // Missing: the active column no longer resolves at all -- already reported above if it
                // was ALSO a captured (non-default) clip; if it was never captured (a paused default
                // clip whose column has since been deleted), silently skip rather than double-report.
            }
        }
    }
}

std::shared_ptr<const Program> compile(const Take& take, const Composition& comp,
                                        DriveClock clock, std::optional<Range> range)
{
    auto program = std::make_shared<Program>();
    program->clock = clock;

    // s-rta-0925 (D4): checkpoint0 -> the preamble, fired at Play before the lane loop below runs.
    // Unconditional on `range` -- a routine's slice preamble is LATER (D9); row 1's range support
    // (Program.h's own comment) is a simple clip filter with no preamble synthesis of its own, so
    // this always uses the WHOLE checkpoint regardless.
    buildPreamble(take.checkpoint0, comp, *program);

    auto pickAt = [&](const Stamp& s, double beat) -> double
    {
        switch (clock)
        {
            case DriveClock::Wall:   return s.t;
            case DriveClock::Beat:   return beat;
            case DriveClock::Sample: return static_cast<double>(s.sample);
        }
        return s.t;
    };

    // A gesture's curve is stored beat-native (D7); the compiled Program
    // is FOR ONE DRIVE CLOCK (D5). Each breakpoint's x normally comes from
    // its own parallel Stamp via `pickAt` above (exact per-point t/sample,
    // D1) -- this tempo-map conversion is only the FALLBACK path for a
    // gesture with no parallel stamps (see the `exact` check below), so
    // Player still just evaluates curve.eval(pos) with pos already in the
    // program's domain, no per-tick tempo lookups.
    auto convertBeatX = [&](double beatX) -> double
    {
        switch (clock)
        {
            case DriveClock::Beat:   return beatX;
            case DriveClock::Wall:   return take.tempo.tAt(beatX);
            case DriveClock::Sample: return static_cast<double>(take.tempo.sampleAt(take.tempo.tAt(beatX)));
        }
        return beatX;
    };

    double maxAt = 0.0;

    for (const auto& [key, lane] : take.lanes)
    {
        if (lane.kind == Lane::Kind::Opaque)
            continue;   // unparseable -- never dispatched, never silently misfires (D12 rule 3)

        auto res = resolveKey(comp, key);
        const Issue issue{ key, res.reason };
        switch (res.outcome)
        {
            case LevelOutcome::ExactMatch:   program->report.resolvedCount++; break;
            case LevelOutcome::PositionOnly: program->report.reboundByPosition.push_back(issue); break;
            case LevelOutcome::NameOnly:     program->report.reboundByName.push_back(issue); break;
            case LevelOutcome::Missing:      program->report.unresolved.push_back(issue); break;
        }
        if (res.outcome == LevelOutcome::Missing)
            continue;   // compiled OUT, but already counted+listed above (D2 policy 3)

        if (lane.kind == Lane::Kind::Discrete)
        {
            for (const auto& p : lane.points)
            {
                const double at = pickAt(p.s, p.beat);
                if (range && (at < range->from || at >= range->to)) continue;
                maxAt = std::max(maxAt, at);
                program->discrete.push_back(Fired{ at, p.s.seq, key, res.target, p });
            }
        }
        else
        {
            ContLane* target = nullptr;
            for (auto& cl : program->continuous)
                if (cl.key == key) { target = &cl; break; }

            int stampMismatches = 0;
            for (const auto& g : lane.gestures)
            {
                if (g.curve.pts.empty()) continue;

                // D1/D7: every breakpoint carries its own exact {t,sample}
                // reading (Lane.h Gesture::stamps, size == pts.size(),
                // enforced by PerformanceRecorder::set/release). Use it. The
                // tempo map is a FALLBACK for a gesture with no parallel
                // stamps (hand-built, or an edited take whose editor dropped
                // them) -- reported, never silent.
                const bool exact = g.stamps.size() == g.curve.pts.size();
                if (!exact) ++stampMismatches;

                ContLane::G cg;
                cg.grip = g.grip;
                for (size_t i = 0; i < g.curve.pts.size(); ++i)
                {
                    Breakpoint converted = g.curve.pts[i];
                    converted.x = exact ? pickAt(g.stamps[i], g.curve.pts[i].x)
                                        : convertBeatX(g.curve.pts[i].x);
                    cg.curve.pts.push_back(converted);
                }
                cg.x0 = cg.curve.pts.front().x;
                cg.x1 = cg.curve.pts.back().x;
                if (range && (cg.x1 < range->from || cg.x0 >= range->to)) continue;
                maxAt = std::max(maxAt, cg.x1);

                if (!target)
                {
                    program->continuous.push_back(ContLane{ key, res.target, {} });
                    target = &program->continuous.back();
                }
                target->gestures.push_back(std::move(cg));
            }
            if (stampMismatches > 0)
                program->report.invalid.push_back({ key, std::to_string(stampMismatches)
                    + " gesture(s) without parallel stamps: x reconstructed from the tempo map (inexact)" });
        }
    }

    std::sort(program->discrete.begin(), program->discrete.end(),
        [](const Fired& a, const Fired& b) {
            if (a.at != b.at) return a.at < b.at;
            return a.seq < b.seq;
        });

    for (auto& cl : program->continuous)
        std::sort(cl.gestures.begin(), cl.gestures.end(),
            [](const ContLane::G& a, const ContLane::G& b) { return a.x0 < b.x0; });

    program->length = maxAt;
    return program;
}
