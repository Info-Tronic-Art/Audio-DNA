#pragma once
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "model/ControlPath.h"
#include <optional>
#include <cstdint>

struct Composition;
class MacroBank;

// manualWrite's headless core (s-rta-0923 lane 3 plan section 3.1-3.2; R8:
// this lane owns manualWrite, the recorder only hooks it later). Lane C0
// (this file) declares the CONTRACT every parallel builder lane compiles
// against and ships every function as a stub (STATUS: every function
// below returns std::nullopt/false/no-op until Lane C1 implements
// ManualWrite.cpp for real). Lane C1 implements the bodies in
// src/connect/ManualWrite.cpp; nothing in this header changes then.

// The D8 ownership chain as a rank (s167 spec D8; binding-decisions ruling
// 18): human Held > human Decaying (MIDI/OSC/HTTP) > playing lane gesture
// (replay/routine) > connection > rest. Stored on ParamConnection::Grip::
// rank (critic amendment #3, src/connect/ParamConnection.h).
enum class Hand : uint8_t { None = 0, Lane = 1, HumanDecaying = 2, HumanHeld = 3 };

// Everything a manual writer needs about ONE continuous control, resolved
// from a ControlPath.
struct ControlRef
{
    ParamConnection* conn = nullptr;   // the grip lives here even when kind == None (arrays are dense)
    float*           manual = nullptr; // the raw model field (MODEL units for scalars, [0,1] otherwise)
    LiveValue*       live = nullptr;   // the twin (only needed by disconnect; may be null for macros)
    float (*toModel)(float norm) = nullptr;   // ScalarDef::toModel for scalars; identity for params/dryWet/macro
    float (*toNorm)(float model) = nullptr;   // ScalarDef::toNorm for scalars; identity for params/dryWet/macro
};

// ControlPath -> live model. POSITIONAL fields only (names are compile-time
// resolution, D2 -- Program::compile's job, not this function's). nullopt on
// any out-of-range/unknown control.
//   scope Comp,  control "scalar", scalar=<key>              -> Composition::scalarConns[CompScalar(key)]
//   scope Layer, deck,layer,       control "scalar"          -> Layer::scalarConns[...]
//   scope Clip,  deck,layer,col,   control "scalar"          -> Clip::scalarConns[...]
//   scope Clip|Layer|Comp, fx>=0,  control "param", param.i  -> EffectSlot::paramConns[i] / paramValues[i]
//   scope Clip|Layer|Comp, fx>=0,  control "dryWet"          -> EffectSlot::dryWetConn / dryWet
//   scope Clip,  fx<0,             control "param", param.i  -> Clip::sourceParams[i].conn / .value   (vocabulary
//                                                               EXTENSION, plan section 3.5)
//   scope Macro, macroScope 0,     control "macro", macro=i  -> globalMacros.getMacro(i).conn / .manualValue
//   control "speed" (Clip::speed) -> nullopt in this lane (Clip::speed has no ParamConnection; step-3 note)
std::optional<ControlRef> resolveControl(Composition& comp, MacroBank& globalMacros, const ControlPath& path);

// Is a grip currently in force? Held: always. Decaying: only until
// gripHoldMs after lastTouch. (The engine only expires Decaying grips for
// CONNECTED params inside evaluate(); an unconnected param's grip must
// expire by timestamp here or a lane would be refused forever after one
// MIDI turn.)
bool gripActive(const ParamConnection& c, double now, float gripHoldMs);

// Open/refresh a grip at rank `hand`. Returns false (nothing changed) if a
// HIGHER rank holds the control. kind: Held = the writer has a release
// event (slider, lane gesture); Decaying = it does not.
bool manualTouchCore(const ControlRef& r, Hand hand, ParamConnection::Grip::Kind kind, double now, float gripHoldMs);

// Touch (as above) + write. valueNorm is the writer's [0,1] value; scalars
// are stored through toModel. Returns false and writes NOTHING if refused.
// Equal rank: latest writer wins (refreshes the grip).
bool manualWriteCore(const ControlRef& r, float valueNorm, Hand hand, ParamConnection::Grip::Kind kind,
                     double now, float gripHoldMs);

// Close the grip if the holder's rank <= hand (a lane cannot release a
// human; a human releases anything).
void manualReleaseCore(const ControlRef& r, Hand hand);

// Disconnect helper the widget/picker uses (s166 section 4.3 "the model
// mutator stores NAN into the twin and releases the grip"): source.kind =
// None, state reset, live->v = NAN, grip cleared.
void disconnect(ParamConnection& c, LiveValue* live);
