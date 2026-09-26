#pragma once
#include "model/Routine.h"
#include "recording/Take.h"
#include <optional>
#include <string>
#include <vector>

class EffectLibrary;

// RoutineSlice -- s167 D4 slice(): cut a take's lanes into a Routine (s-rta-0926 routines
// slice 1, plan-routines-s1-final.md 3.3). The cut is beat-native and relative: every kept
// point/breakpoint is rebased so beat `fromBeat` of the take is beat 0 of the routine, wall and
// sample stamps are dropped (a routine plays on the Beat clock only), a gesture straddling either
// edge gets a synthesized breakpoint there, and the state every kept control needs FIRST is
// written out as an explicit restore list (Routine::preamble): the last lane value before the
// cut, else checkpoint 0, else the control's default. Pure: no Composition, no app state.

struct SliceRequest
{
    double fromBeat = 0.0, toBeat = 0.0;   // take beats, [fromBeat, toBeat)
    std::string name;                     // routine name as typed (the caller supplies "Routine N" when empty)
    bool wholeBars = true;                // round the length UP to whole bars (D4 step 5)
    std::string takeFolder;               // Routine::source.takeFolder (display / re-slice only)
};

struct SliceResult
{
    std::optional<Routine> routine;
    std::string error;                    // non-empty => refused, routine empty
    std::vector<std::string> droppedLanes;   // transport-class lanes, by name
    int preambleFromLanes = 0, preambleFromCheckpoint = 0, preambleFromDefaults = 0, preambleUnknown = 0;
};

SliceResult sliceRoutine(const Take& take, const SliceRequest& req, const EffectLibrary& library);

// Take beat of the start of 1-based bar `bar`, given Meta::startBeatInBar: bar 1 is the first FULL
// bar after Record (the partial bar before it is bar 0). Unknown (-1) counts bars from the take's
// start.
double takeBeatOfBar(const Take& take, int bar);
