#pragma once
#include "connect/ParamConnection.h"
#include <functional>
#include <cstdint>

class SignalRegistry;
class MacroBank;
struct FeatureSnapshot;
struct Composition;
struct Clip;

// ClipClock: the narrow interface a real per-clip playhead will implement
// against (Clip::playheadPosition is a GL-written `mutable double` -- s166
// spec section 8 names two acceptable read strategies, both Lane 5's call).
// L2 only needs the SHAPE of this seam so ConnSource::Kind::ClipPosition and
// Envelope::Clock::ClipPosition are evaluable and unit-testable today;
// nothing in this lane constructs one against a live Clip, and
// ConnectionEngine::tick passes nullptr for it everywhere it walks the
// model (Lane 5 wires the real thing).
struct ClipClock
{
    virtual ~ClipClock() = default;
    virtual float position() const = 0;   // [0,1] normalized playhead
};

// The one evaluator: walks a Composition's connections once per tick and
// publishes into LiveValue twins (s166 spec sections 4.3 and 1). Message-
// thread confined, matching every other piece of this design (SignalRegistry,
// MacroBank, ParamConnection's grip writers).
class ConnectionEngine
{
public:
    struct Context
    {
        const SignalRegistry& signals;
        MacroBank& macros;              // NOT owned by Composition -- see s167-l2 report re:
                                        // the owner's multi-bank amendment; the caller decides
                                        // which bank(s) to pass, this engine assumes nothing.
        const FeatureSnapshot& snap;
        float dt = 0.0f;                // measured tick delta, clamped [0, 0.05] by the caller
        double now = 0.0;               // engine time, seconds
        float gripHoldMs = 250.0f;      // Decaying grip expiry (owner D15)
        float handBackGlideMs = 120.0f; // hand-back glide duration; 0 = snap (owner D14)
    };

    // Walks macros -> comp.scalarConns -> comp.globalEffects -> per deck:
    // layer scalars + layerEffects -> per clip cell: scalars,
    // effects[].paramConns/dryWetConn, sourceParams[].conn (s166 spec
    // section 4.3). Message thread only (jassert). No allocation beyond the
    // occasional EffectSlot::resizeParams self-heal (s166 spec section 3.3).
    void tick(Composition& comp, const Context& ctx);

    // Pure(ish) per-connection pipeline: source value -> shape -> grip
    // takeover / hand-back glide. Non-static-adjacent state (grip, smoother
    // memory, once-mode pin, S&H memory) all lives in `c` itself, so this is
    // fully unit-testable against a bare ParamConnection.
    //
    // Returns NAN when nothing should be published this tick (disabled,
    // disconnected, or gripped) -- callers store the result straight into a
    // LiveValue twin. NAN propagates through every ScalarDef::toModel()
    // formula in this codebase (all pure arithmetic), so no special-casing
    // is needed at scalar publish sites either.
    //
    // manualNorm is the manual field's value in the SAME normalized
    // [0,1]-ish space this function shapes into: 1:1 for effect/source/macro
    // params (already stored normalized by convention), ScalarDef::toNorm
    // (manual) for a scalar target.
    static float evaluate(ParamConnection& c, float manualNorm, const Context& ctx,
                          const ClipClock* clock);

    // Fired after a clip's sourceParams live twins are published this tick,
    // so the renderer can re-run its existing per-source uniform-upload hook
    // (s166 spec section 4.3; wired to Renderer::updateActiveSourceParams in
    // Lane 3 -- not called from anywhere in this lane).
    std::function<void(const Clip*)> onSourceParamsPublished;
};
