#include "connect/ConnectionEngine.h"
#include "connect/ConnectionShaper.h"
#include "connect/ScalarParams.h"
#include "model/Composition.h"
#include "routing/MacroBank.h"
#include "signal/SignalRegistry.h"
#include "signal/Signal.h"
#include "analysis/FeatureSnapshot.h"
#include <juce_events/juce_events.h>   // MessageManager (message-thread jassert, matches S166-L1 precedent)
#include <cmath>
#include <limits>

namespace
{
    constexpr float kNan = std::numeric_limits<float>::quiet_NaN();

    // name->id cache, re-validated every call (ParamConnection::State::
    // cachedSignalId) so a rename or reorder in the registry is noticed
    // without paying a linear scan every tick in the common case.
    uint32_t resolveSignalId(const ConnSource& source, ParamConnection::State& state,
                             const SignalRegistry& signals)
    {
        if (state.cachedSignalId != 0)
        {
            if (const Signal* s = signals.getSignal(state.cachedSignalId))
                if (s->getName() == source.signalName)
                    return state.cachedSignalId;
        }
        for (int i = 0; i < signals.getNumSignals(); ++i)
        {
            if (const Signal* s = signals.getSignalAt(i))
            {
                if (s->getName() == source.signalName)
                {
                    state.cachedSignalId = s->getId();
                    return state.cachedSignalId;
                }
            }
        }
        state.cachedSignalId = 0;
        return 0;
    }

    // "once" (loop==false) gating: pins the cycle start the first time a
    // phase-based source is evaluated (s166 spec section 2.4's "on enable/
    // connect otherwise" -- clip-trigger re-pinning is Lane 5's job), then
    // clamps the returned "cycles elapsed" so playbackXform holds at the
    // end-of-cycle value forever after. The tiny epsilon keeps the held
    // phase just BEFORE the wrap point (frac(1.0) == 0.0 would otherwise
    // snap a held SawUp/Lfo back to its start value instead of its end).
    float gateOnce(float rawCycles, ConnShape::Playback pb, bool loop, ParamConnection::State& state)
    {
        if (loop)
        {
            state.onceStartBeats = -1.0;
            return rawCycles;
        }
        if (state.onceStartBeats < 0.0)
            state.onceStartBeats = static_cast<double>(rawCycles);

        float elapsed = rawCycles - static_cast<float>(state.onceStartBeats);
        float maxCycles = (pb == ConnShape::Playback::PingPong) ? 2.0f : 1.0f;
        if (elapsed >= maxCycles)
            elapsed = maxCycles - 1e-5f;
        return elapsed;
    }
}

float ConnectionEngine::evaluate(ParamConnection& c, float manualNorm, const Context& ctx,
                                 const ClipClock* clock)
{
    if (!c.enabled || c.source.kind == ConnSource::Kind::None)
        return kNan;

    // Decaying grips expire on their own; Held grips never do (s166 spec
    // section 2.3).
    if (c.grip.kind == ParamConnection::Grip::Kind::Decaying
        && (ctx.now - c.grip.lastTouch) >= static_cast<double>(ctx.gripHoldMs) / 1000.0)
    {
        c.release(ctx.now);
    }

    if (c.grip.kind != ParamConnection::Grip::Kind::None)
    {
        // Gripped: publish nothing (the twin holds NaN -> the renderer falls
        // back to the manual field the human is actively writing), but keep
        // tracking the live manual value so the instant the grip lets go,
        // the hand-back glide starts from exactly where the human left it.
        c.state.handBackFrom = manualNorm;
        c.state.handBackStart = 0.0;   // glide not started yet
        return kNan;
    }

    float raw = 0.0f;
    float bn = ConnectionShaper::beatsNow(ctx.snap.beatPhase, ctx.snap.beatInBar, ctx.snap.barCount,
                                          ctx.snap.totalBarCount, c.shape.resetPhaseOnStructural);

    switch (c.source.kind)
    {
        case ConnSource::Kind::Signal:
        {
            uint32_t id = resolveSignalId(c.source, c.state, ctx.signals);
            raw = ctx.signals.getCachedValue(id);
            break;
        }
        case ConnSource::Kind::Macro:
        {
            int idx = c.source.macroIndex;
            raw = (idx >= 0 && idx < MacroBank::kNumMacros)
                ? ctx.macros.getMacro(idx).currentValue
                : 0.0f;
            break;
        }
        case ConnSource::Kind::Lfo:
        {
            float rawCycles = bn / std::max(c.source.lfo.cycleBeats, 1e-6f) + c.source.lfo.phaseOffset;
            rawCycles = gateOnce(rawCycles, c.shape.playback, c.shape.loop, c.state);
            float phase = ConnectionShaper::playbackXform(rawCycles, c.shape.playback);
            long cycleIdx = static_cast<long>(std::floor(rawCycles));
            raw = ConnectionShaper::lfoShapeValue(c.source.lfo.shape, phase, c.source.lfo.pulseWidth,
                                                  cycleIdx, c.state.shValue, c.state.shCycle);
            break;
        }
        case ConnSource::Kind::Envelope:
        {
            float pos;
            if (c.source.env.clock == ConnSource::Envelope::Clock::Beats)
            {
                float rawCycles = bn / std::max(c.source.env.cycleBeats, 1e-6f);
                rawCycles = gateOnce(rawCycles, c.shape.playback, c.shape.loop, c.state);
                pos = ConnectionShaper::playbackXform(rawCycles, c.shape.playback);
            }
            else
            {
                float clipPos = clock ? clock->position() : 0.0f;
                pos = ConnectionShaper::playbackXform(clipPos, c.shape.playback);
            }
            raw = c.source.env.curve.eval(static_cast<double>(pos));
            break;
        }
        case ConnSource::Kind::ClipPosition:
        {
            float clipPos = clock ? clock->position() : 0.0f;
            raw = ConnectionShaper::playbackXform(clipPos, c.shape.playback);
            break;
        }
        case ConnSource::Kind::None:
        default:
            return kNan;
    }

    float y = ConnectionShaper::shapeValue(c.shape, raw);
    y = ConnectionShaper::applySmoothing(y, c.shape.smoothingMs, ctx.dt, c.state.smooth);

    // Hand-back glide (owner D14): in progress if handBackFrom is non-NaN
    // (set above, the tick a grip was active; or the very last tick before
    // that, once the grip has since released/expired).
    if (!std::isnan(c.state.handBackFrom))
    {
        if (c.state.handBackStart == 0.0)
            c.state.handBackStart = ctx.now;   // first tick after release -- start the glide now

        double glideS = static_cast<double>(ctx.handBackGlideMs) / 1000.0;
        double elapsed = ctx.now - c.state.handBackStart;
        if (glideS <= 0.0 || elapsed >= glideS)
        {
            c.state.handBackFrom = kNan;
            c.state.handBackStart = 0.0;
        }
        else
        {
            float t = static_cast<float>(elapsed / glideS);
            y = c.state.handBackFrom + t * (y - c.state.handBackFrom);
        }
    }

    return y;
}

namespace
{
    // Ticks one scalar family (Clip/Layer/Composition), publishing shaped
    // values in MODEL units via ScalarDef::toModel. Owner/ScalarEnum are
    // resolved by the caller's explicit template arguments; manualRef(Owner&,
    // ScalarEnum) is found by ordinary overload resolution (Clip.h/Layer.h/
    // model/Composition.h each declare their own overload) -- "the only
    // place that names the fields" per the s166 spec's own comment on it.
    template <typename Owner, typename ScalarEnum, size_t N>
    void tickScalars(Owner& owner, std::array<ParamConnection, N>& conns,
                     std::array<LiveValue, N>& live, const std::array<ScalarDef, N>& defs,
                     const ConnectionEngine::Context& ctx, const ClipClock* clock)
    {
        for (size_t i = 0; i < N; ++i)
        {
            ParamConnection& c = conns[i];
            if (!c.isConnected() || !c.enabled)
                continue;
            ScalarEnum s = static_cast<ScalarEnum>(i);
            float manualNorm = defs[i].toNorm(manualRef(owner, s));
            float y = ConnectionEngine::evaluate(c, manualNorm, ctx, clock);
            if (!std::isnan(y))
                live[i].v.store(defs[i].toModel(y), std::memory_order_relaxed);
        }
    }

    void tickEffectVector(std::vector<Clip::EffectSlot>& effects, const ConnectionEngine::Context& ctx,
                          const ClipClock* clock)
    {
        for (auto& fx : effects)
        {
            // Self-heal any size drift between paramValues and the parallel
            // connection/live arrays (s166 spec section 3.3). MainComponent.
            // cpp's three push_back(defaultValue) sizing sites are
            // deliberately NOT touched by this lane (FENCE DEVIATION -- see
            // s167-l2 report); this lazy resize is what keeps that safe.
            fx.resizeParams(fx.paramValues.size());

            for (size_t i = 0; i < fx.paramConns.size(); ++i)
            {
                ParamConnection& c = fx.paramConns[i];
                if (!c.isConnected() || !c.enabled)
                    continue;
                float y = ConnectionEngine::evaluate(c, fx.paramValues[i], ctx, clock);
                if (!std::isnan(y))
                    fx.paramLive[i].v.store(y, std::memory_order_relaxed);
            }

            if (fx.dryWetConn.isConnected() && fx.dryWetConn.enabled)
            {
                float y = ConnectionEngine::evaluate(fx.dryWetConn, fx.dryWet, ctx, clock);
                if (!std::isnan(y))
                    fx.dryWetLive.v.store(y, std::memory_order_relaxed);
            }
        }
    }
}

void ConnectionEngine::tick(Composition& comp, const Context& ctx)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    // ORDERING FACT for whoever adds a recorded-performance player later: a
    // performance take is a WRITER (see ParamConnection.h's Kind-level
    // comment), so its per-tick `advanceTo` call must run BEFORE this
    // tick() -- grip state has to already be current (Held/Decaying/None)
    // by the time evaluate() below decides whether to publish, or a
    // gesture's first tick renders one frame late. Nothing in this lane
    // calls such a player; this is a placement constraint for that future
    // caller, not a TODO for this lane.

    // Macros tick first -- they are sources for parameters (s166 spec
    // section 2.2). ctx.macros is caller-supplied (not reached through
    // `comp`): the owner's multi-bank amendment means this engine must not
    // assume a single global bank lives on the Composition -- see the
    // s167-l2 report.
    for (int i = 0; i < MacroBank::kNumMacros; ++i)
    {
        MacroBank::Macro& macro = ctx.macros.getMacro(i);
        if (!macro.conn.isConnected() || !macro.conn.enabled)
            continue;
        float y = evaluate(macro.conn, macro.manualValue, ctx, nullptr);
        if (!std::isnan(y))
            macro.currentValue = y;
    }

    tickScalars<Composition, CompScalar>(comp, comp.scalarConns, comp.scalarLive, compScalarDefs(),
                                         ctx, nullptr);
    tickEffectVector(comp.globalEffects, ctx, nullptr);

    for (auto& deck : comp.decks)
    {
        for (auto& layer : deck.layers)
        {
            tickScalars<Layer, LayerScalar>(layer, layer.scalarConns, layer.scalarLive,
                                            layerScalarDefs(), ctx, nullptr);
            tickEffectVector(layer.layerEffects, ctx, nullptr);

            for (auto& clipOpt : layer.clips)
            {
                if (!clipOpt.has_value())
                    continue;
                Clip& clip = *clipOpt;

                // ClipPosition/Envelope(Clock::ClipPosition) wiring for a
                // real per-clip playhead is Lane 5's job (s166 spec section
                // 5, L5); this lane passes nullptr everywhere it walks the
                // model, which evaluate() treats as "position 0.0".
                tickScalars<Clip, ClipScalar>(clip, clip.scalarConns, clip.scalarLive,
                                              clipScalarDefs(), ctx, nullptr);
                tickEffectVector(clip.effects, ctx, nullptr);

                bool publishedAny = false;
                for (auto& sp : clip.sourceParams)
                {
                    if (!sp.conn.isConnected() || !sp.conn.enabled)
                        continue;
                    float y = evaluate(sp.conn, sp.value, ctx, nullptr);
                    if (!std::isnan(y))
                    {
                        sp.live.v.store(y, std::memory_order_relaxed);
                        publishedAny = true;
                    }
                }
                if (publishedAny && onSourceParamsPublished)
                    onSourceParamsPublished(&clip);
            }
        }
    }
}
