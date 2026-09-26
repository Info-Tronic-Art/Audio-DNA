#include "connect/ManualWrite.h"
#include "model/Composition.h"
#include "routing/MacroBank.h"
#include <cmath>
#include <limits>

// Lane C1: manualWrite's headless core, real implementation (s-rta-0923
// lane 3 plan section 3.1-3.2, section 5 Lane C1). Replaces every stub body
// Lane C0 shipped.

namespace
{
    // Find the index of `key` in a scalar-family's descriptor table (linear
    // scan; N is at most 9 -- ScalarParams.h -- so this is cheap and never
    // hot-path: only fires on picker/inspector edits, not per render frame).
    template <size_t N>
    int findScalarIndex(const std::array<ScalarDef, N>& defs, const std::string& key)
    {
        for (size_t i = 0; i < N; ++i)
            if (key == defs[i].key)
                return static_cast<int>(i);
        return -1;
    }

    // One scalar family (Clip/Layer/Composition), resolved via the SAME
    // manualRef(Owner&, ScalarEnum) overload ConnectionEngine.cpp's
    // tickScalars uses -- "the only place that names the fields" (per the
    // model headers' own comment on manualRef).
    template <typename Owner, typename ScalarEnum, size_t N>
    std::optional<ControlRef> resolveScalar(Owner& owner, std::array<ParamConnection, N>& conns,
                                            std::array<LiveValue, N>& live,
                                            const std::array<ScalarDef, N>& defs,
                                            const std::string& key)
    {
        int idx = findScalarIndex<N>(defs, key);
        if (idx < 0)
            return std::nullopt;
        size_t i = static_cast<size_t>(idx);
        return ControlRef{ &conns[i], &manualRef(owner, static_cast<ScalarEnum>(idx)), &live[i],
                            defs[i].toModel, defs[i].toNorm };
    }

    // control == "param" / "dryWet" with fx >= 0, against one owner's
    // EffectSlot vector (Clip::effects, Layer::layerEffects,
    // Composition::globalEffects -- plan section 3.2's resolveControl table).
    std::optional<ControlRef> resolveEffectControl(std::vector<Clip::EffectSlot>& fxList,
                                                    const ControlPath& path)
    {
        if (path.fx < 0 || static_cast<size_t>(path.fx) >= fxList.size())
            return std::nullopt;
        Clip::EffectSlot& fx = fxList[static_cast<size_t>(path.fx)];

        if (path.control == "dryWet")
            return ControlRef{ &fx.dryWetConn, &fx.dryWet, &fx.dryWetLive,
                                ScalarMath::identity, ScalarMath::identity };

        if (path.control == "param")
        {
            if (path.param < 0)
                return std::nullopt;
            // Self-heal any size drift (same lazy resize ConnectionEngine.cpp's
            // tickEffectVector already performs -- s166 spec section 3.3).
            fx.resizeParams(fx.paramValues.size());
            if (static_cast<size_t>(path.param) >= fx.paramValues.size())
                return std::nullopt;
            size_t p = static_cast<size_t>(path.param);
            return ControlRef{ &fx.paramConns[p], &fx.paramValues[p], &fx.paramLive[p],
                                ScalarMath::identity, ScalarMath::identity };
        }
        return std::nullopt;
    }
}

std::optional<ControlRef> resolveControl(Composition& comp, MacroBank& globalMacros, const ControlPath& path)
{
    // control:"speed" (Clip::speed) has no ParamConnection in this lane --
    // plan section 3.5, named so it cannot surprise anyone.
    if (path.control == "speed")
        return std::nullopt;

    if (path.scope == ControlPath::Scope::Macro)
    {
        if (path.control != "macro" || path.macroScope != 0)
            return std::nullopt;
        if (path.macro < 0 || path.macro >= MacroBank::kNumMacros)
            return std::nullopt;
        MacroBank::Macro& m = globalMacros.getMacro(path.macro);
        return ControlRef{ &m.conn, &m.manualValue, nullptr, ScalarMath::identity, ScalarMath::identity };
    }

    if (path.scope == ControlPath::Scope::Comp)
    {
        if (path.fx >= 0)
            return resolveEffectControl(comp.globalEffects, path);
        if (path.control == "scalar")
            return resolveScalar<Composition, CompScalar>(comp, comp.scalarConns, comp.scalarLive,
                                                           compScalarDefs(), path.scalar);
        return std::nullopt;
    }

    if (path.scope == ControlPath::Scope::Layer)
    {
        if (path.deck < 0 || static_cast<size_t>(path.deck) >= comp.decks.size())
            return std::nullopt;
        Layer* layer = comp.decks[static_cast<size_t>(path.deck)].getLayer(path.layer);
        if (!layer)
            return std::nullopt;
        if (path.fx >= 0)
            return resolveEffectControl(layer->layerEffects, path);
        if (path.control == "scalar")
            return resolveScalar<Layer, LayerScalar>(*layer, layer->scalarConns, layer->scalarLive,
                                                      layerScalarDefs(), path.scalar);
        return std::nullopt;
    }

    if (path.scope == ControlPath::Scope::Clip)
    {
        if (path.deck < 0 || static_cast<size_t>(path.deck) >= comp.decks.size())
            return std::nullopt;
        Clip* clip = comp.decks[static_cast<size_t>(path.deck)].getClip(path.layer, path.col);
        if (!clip)
            return std::nullopt;

        if (path.fx >= 0)
            return resolveEffectControl(clip->effects, path);

        // Vocabulary extension (plan section 3.5): control:"param" with
        // fx.i == -1 on a Clip scope = Clip::sourceParams[param.i] (the
        // second existing working path -- procedural source params).
        if (path.control == "param" && path.fx < 0)
        {
            if (path.param < 0 || static_cast<size_t>(path.param) >= clip->sourceParams.size())
                return std::nullopt;
            Clip::SourceParam& sp = clip->sourceParams[static_cast<size_t>(path.param)];
            return ControlRef{ &sp.conn, &sp.value, &sp.live, ScalarMath::identity, ScalarMath::identity };
        }

        if (path.control == "scalar")
            return resolveScalar<Clip, ClipScalar>(*clip, clip->scalarConns, clip->scalarLive,
                                                    clipScalarDefs(), path.scalar);
        return std::nullopt;
    }

    return std::nullopt;
}

bool gripActive(const ParamConnection& c, double now, float gripHoldMs)
{
    if (c.grip.kind == ParamConnection::Grip::Kind::None)
        return false;
    if (c.grip.kind == ParamConnection::Grip::Kind::Held)
        return true;
    // Decaying: active until gripHoldMs after the last refreshing write (the
    // engine only expires a Decaying grip for CONNECTED params inside
    // evaluate() -- an unconnected param's grip must expire by timestamp
    // here, or a lane would be refused forever after one MIDI turn).
    return (now - c.grip.lastTouch) < static_cast<double>(gripHoldMs) / 1000.0;
}

bool manualTouchCore(const ControlRef& r, Hand hand, ParamConnection::Grip::Kind kind, double now, float gripHoldMs)
{
    if (!r.conn)
        return false;

    ParamConnection::Grip& grip = r.conn->grip;
    bool active = gripActive(*r.conn, now, gripHoldMs);

    // Refused only if a STRICTLY higher rank currently, actively, holds the
    // grip -- equal rank means "latest writer wins" (refreshes the grip); an
    // expired/inactive grip never blocks a lower rank from taking over.
    if (active && grip.rank > static_cast<uint8_t>(hand))
        return false;

    grip.kind = kind;
    grip.lastTouch = now;
    grip.rank = static_cast<uint8_t>(hand);
    return true;
}

bool manualWriteCore(const ControlRef& r, float valueNorm, Hand hand, ParamConnection::Grip::Kind kind,
                     double now, float gripHoldMs)
{
    if (!r.conn || !r.manual)
        return false;
    if (!manualTouchCore(r, hand, kind, now, gripHoldMs))
        return false;
    *r.manual = r.toModel ? r.toModel(valueNorm) : valueNorm;
    return true;
}

void manualReleaseCore(const ControlRef& r, Hand hand)
{
    if (!r.conn)
        return;
    ParamConnection::Grip& grip = r.conn->grip;
    // Close the grip only if the holder's rank <= the releaser's hand -- a
    // lane cannot release a human; a human releases anything (including its
    // own equal-rank hand, or a lower one).
    if (grip.rank <= static_cast<uint8_t>(hand))
    {
        grip.kind = ParamConnection::Grip::Kind::None;
        grip.rank = 0;
    }
}

void disconnect(ParamConnection& c, LiveValue* live)
{
    c.source = ConnSource{};
    c.state = ParamConnection::State{};
    c.grip = ParamConnection::Grip{};
    if (live)
        live->v.store(std::numeric_limits<float>::quiet_NaN(), std::memory_order_relaxed);
}
