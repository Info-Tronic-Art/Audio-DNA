#pragma once
#include "connect/AutomationCurve.h"
#include <string>
#include <cstdint>
#include <cmath>

// The universal connection (s166 architecture,
// .harmony/specs/s166-universal-connection-architecture.md section 2.1):
// every slider in the app -- effect param, source param, macro knob, or a
// clip/layer/composition transform/opacity scalar -- is driven by exactly
// ONE of these, stored INSIDE the thing it controls (saved with it, copied
// with it, undone with it). Converged on the owner's rulings: Ruling A
// (latest-input-wins takeover, modelled by the Grip below -- there is no
// separate "mode/depth" concept); Ruling B (RANGE / INVERT / PLAYBACK are
// the only per-connection controls the owner asked for; curve / inMin/inMax
// / smoothingMs are the architect's additions, kept at neutral defaults so
// an unconfigured connection behaves exactly as the owner described it).
struct ConnSource
{
    // A recorded per-control performance take is NOT a connection Kind --
    // architect ruling, s167-l2 (retracts this lane's original packet,
    // which asked for a reserved Timeline/Lane seam here). A recorded lane
    // is a WRITER: it goes through the manual-write/grip path as one more
    // "hand" on the parameter, exactly like a human or a MIDI knob, because
    // a connection is exclusive (one owner) while several takes and a hand
    // can touch one knob in a minute; a lane is silent between gestures
    // where an owner is a total function; a lane's clock belongs to the
    // take that triggered it, not to the parameter; and half of all lanes
    // are buttons (clip hits, deck switches), which are not connectable at
    // all. No enum value is reserved for it. Forward-compatibility for any
    // future Kind is already unconditional: kinds serialize as strings, and
    // ConnSerialization loads any it does not recognize as None and reports
    // it (see ConnSerialization::fromVar) -- nothing further is needed here.
    enum class Kind : uint8_t { None, Signal, Macro, Lfo, Envelope, ClipPosition };
    Kind kind = Kind::None;

    std::string signalName;   // Kind::Signal -- the PERSISTENT key (SignalRegistry
                              // ids are minted per process and never saved)
    int macroIndex = -1;      // Kind::Macro (0..MacroBank::kNumMacros-1)

    struct Lfo
    {
        // "a frequency we generate ... timed to the bpm" (Ruling B). SawDown
        // is expressed as SawUp + Playback::Backward, not a separate shape.
        enum class Shape : uint8_t { Sine, SawUp, Triangle, Square, SampleHold };
        Shape shape = Shape::Sine;
        float cycleBeats = 1.0f;   // 0.25 .. 16; tempo-locked by definition
        float phaseOffset = 0.0f;  // [0,1)
        float pulseWidth = 0.5f;   // Square only
    } lfo;

    struct Envelope
    {
        // The hand-drawable per-control curve (owner D6). AutomationCurve is
        // the SAME type a future recorded performance take will use to hold
        // its captured shape (architect ruling, s167-l2: one drawn, one
        // captured, one struct/evaluator/editor) -- x runs 0..1 over
        // whichever domain `clock` selects below.
        AutomationCurve curve;
        enum class Clock : uint8_t { Beats, ClipPosition };
        Clock clock = Clock::Beats;
        float cycleBeats = 4.0f;
    } env;
};

struct ConnShape
{
    // ---- owner-ruled (Ruling B.3) ----
    float outMin = 0.0f, outMax = 1.0f;   // RANGE: which sub-range of the slider's travel the signal sweeps
    bool  inverted = false;               // INVERT
    enum class Playback : uint8_t { Forward, Backward, PingPong } playback = Playback::Forward;
    bool  loop = true;                    // false = play once per trigger, then hold the end value

    // ---- architect's additions; neutral defaults reproduce the owner's literal ask ----
    uint8_t curve = 0;                // MappingCurve enum value (Linear == 0) -- the repo's only
                                      // complete shaping math; needed for lossless v1 preset conversion.
    float inMin = 0.0f, inMax = 1.0f; // input normalization window (v1 conversion parity). Hidden.
    float smoothingMs = 0.0f;         // 0 = off (the owner's literal ruling; not owner-required otherwise)

    // S168: false (default) = ConnectionShaper::beatsNow folds this
    // connection's Lfo/Envelope(Beats) phase across FeatureSnapshot::
    // totalBarCount (never jumps backward on a structural reset); true =
    // the original S166-L1 behaviour, folding across FeatureSnapshot::
    // barCount (jumps on a real drop/breakdown transition). Same name/
    // default as OscillatorSignal::resetPhaseOnStructural_ and
    // EnvelopeSignal::resetPhaseOnStructural_ -- one design, three places.
    bool resetPhaseOnStructural = false;
};

// ParamConnection: the whole story for ONE slider. Exactly one per
// parameter (a field, not a list) -- enforced by the type living where it
// lives (EffectSlot::paramConns[p], SourceParam::conn, Clip/Layer/
// Composition::scalarConns[i], MacroBank::Macro::conn), never a table keyed
// by an id.
struct ParamConnection
{
    ConnSource source;
    ConnShape  shape;
    bool enabled = true;
    bool isConnected() const { return source.kind != ConnSource::Kind::None; }

    // ---- the grip (Ruling A): model-level "latest input wins, automatic
    // hand-back on release" -- see ConnectionEngine::evaluate for the full
    // state machine that reads/mutates this. ----
    struct Grip
    {
        // Held = an explicit release event exists (a dragged slider, a
        // touch surface) -- never expires on its own. Decaying = no release
        // event exists (MIDI CC, OSC, HTTP, note velocity) -- expires
        // `gripHoldMs` after the last refreshing write.
        enum class Kind : uint8_t { None, Held, Decaying };
        Kind kind = Kind::None;
        double lastTouch = 0.0;   // engine time, seconds -- Decaying's refresh stamp
    } grip;

    // Runtime state; lives INSIDE the connection (never a parallel vector,
    // matching the LiveValue placement rationale) so it copies/moves/undoes
    // with the connection. Not serialized.
    struct State
    {
        float smooth = NAN;            // EMA memory (only meaningful if smoothingMs > 0)
        float handBackFrom = NAN;      // NAN = not currently handing back
        double handBackStart = 0.0;    // 0.0 = hand-back glide not yet started
        float shValue = 0.0f;          // Sample & Hold memory
        int shCycle = -1;
        double onceStartBeats = -1.0;  // loop==false: the pinned cycle-start; -1 = not yet pinned
        uint32_t cachedSignalId = 0;   // name->id cache, re-validated every evaluate()
    };
    mutable State state;

    ParamConnection() = default;

    // Copying a connection (undo/redo's whole-vector before/after snapshot
    // swap via EffectStackCmd, a preset overwriting a live struct in place)
    // never carries a live human gesture or its per-tick timing bookkeeping
    // onto the destination -- "Undo/redo and preset load clear all grips"
    // (s166 spec section 2.3). A MOVE (std::vector reallocation/growth,
    // Clip::replaceContent's std::move(savedEffects)/std::move(scalarConns))
    // relocates the SAME live connection rather than forking a second one,
    // so it is left =default and preserves grip/state exactly -- this is
    // also why every special member below is spelled out explicitly: a
    // user-declared copy ctor/assignment silently suppresses the implicit
    // move ctor/assignment, which would make every std::vector<EffectSlot>
    // growth silently fall back to copying (clearing grips it must not).
    ParamConnection(const ParamConnection& o)
        : source(o.source), shape(o.shape), enabled(o.enabled) {}
    ParamConnection& operator=(const ParamConnection& o)
    {
        source = o.source;
        shape = o.shape;
        enabled = o.enabled;
        grip = Grip{};
        state = State{};
        return *this;
    }
    ParamConnection(ParamConnection&&) = default;
    ParamConnection& operator=(ParamConnection&&) = default;

    void gripHeld() { grip.kind = Grip::Kind::Held; }   // slider mouse-down / touch-begin -- always wins
    void gripTouch(double now)                          // a release-less write (MIDI/OSC/HTTP/velocity)
    {
        if (grip.kind == Grip::Kind::Held)
            return;   // a Held grip is not displaced by a Decaying one
        grip.kind = Grip::Kind::Decaying;
        grip.lastTouch = now;
    }
    void release(double /*now*/) { grip.kind = Grip::Kind::None; }   // slider mouse-up, or Decaying expiry
};
