#include "recording/Program.h"
#include "model/Composition.h"
#include <algorithm>

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
}

std::shared_ptr<const Program> compile(const Take& take, const Composition& comp,
                                        DriveClock clock, std::optional<Range> range)
{
    auto program = std::make_shared<Program>();
    program->clock = clock;

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
    // is FOR ONE DRIVE CLOCK (D5), so continuous curves are converted once
    // here via the take's own TempoMap -- Player then just evaluates
    // curve.eval(pos) with pos already in the program's domain, no
    // per-tick tempo lookups.
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

            for (const auto& g : lane.gestures)
            {
                if (g.curve.pts.empty()) continue;

                ContLane::G cg;
                cg.grip = g.grip;
                for (const auto& bp : g.curve.pts)
                {
                    Breakpoint converted = bp;
                    converted.x = convertBeatX(bp.x);
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
