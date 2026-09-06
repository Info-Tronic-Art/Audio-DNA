#pragma once
#include <juce_core/juce_core.h>
#include <string>
#include <map>
#include <cstdint>

// PerfState -- D4: "checkpoint 0", the full performance state at record
// start (and a second checkpoint at stop, lint-only). Generalises G21's
// LayerRuntimeSnapshot across the whole composition so a lane's own
// silence ("unchanged") can be told apart from "the take never touched
// this control at all" -- what no lane can tell you is the value of a
// control the performance never touched.
//
// Deliberately its OWN plain-typed shape, not a reference into
// Composition/Deck/Layer/Clip: a checkpoint is captured once and stored in
// the take, so it must survive independently of whatever the live model
// does afterward. Capturing FROM a live Composition is step 3's job
// (MainComponent wiring); this packet only needs the shape and its
// round-trip.
struct PerfState
{
    // Only NON-DEFAULT clip state is worth keeping (D4): effect manual
    // values, scalars, playing/playhead. Keyed by column within the owning
    // LayerRuntime.
    struct ClipRuntime
    {
        std::string clip;                       // name at capture (re-target aid, never identity)
        bool playing = false;
        double playheadPosition = 0.0;
        std::map<int, float> effectParams;       // fx slot index -> manual value
        std::map<std::string, float> scalars;    // ScalarDef::key -> value

        juce::var toVar() const;
        static ClipRuntime fromVar(const juce::var& v);
    };

    // Generalises G21's LayerRuntimeSnapshot {activeClipColumn,
    // previousClipColumn, crossfadeProgress, pendingTriggerColumn,
    // pendingTriggerSnapOverride} plus the flags + opacity + layer-effect
    // manual values D4 asks for.
    struct LayerRuntime
    {
        std::string layer;
        int activeClipColumn = -1;
        int previousClipColumn = -1;
        float crossfadeProgress = 1.0f;
        int pendingTriggerColumn = -1;
        int pendingTriggerSnapOverride = 0;
        float opacity = 1.0f;
        bool visible = true;
        bool bypassed = false;
        bool solo = false;
        bool muted = false;
        bool autopilotEnabled = false;
        std::map<int, float> effectParams;       // per-layer effect manual values, non-default
        std::map<int, ClipRuntime> clips;         // column -> non-default clip state

        juce::var toVar() const;
        static LayerRuntime fromVar(const juce::var& v);
    };

    struct DeckRuntime
    {
        std::string deck;
        std::map<int, LayerRuntime> layers;

        juce::var toVar() const;
        static DeckRuntime fromVar(const juce::var& v);
    };

    int activeDeckIndex = -1;
    int quantizeMode = 0;
    float bpm = 0.0f;
    std::string audioAction;      // "play" | "pause" | "stop" | "" (never set)
    std::map<int, DeckRuntime> decks;

    juce::var toVar() const;
    static PerfState fromVar(const juce::var& v);
};
