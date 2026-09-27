#pragma once
#include "model/ControlPath.h"
#include "model/Clip.h"
#include "recording/Lane.h"
#include <juce_core/juce_core.h>
#include <map>
#include <string>
#include <vector>

// Routine -- s167 D9: a composition-owned, beat-native lane set + the state it
// needs first (D4 step 2). Cut from a take by sliceRoutine
// (src/recording/RoutineSlice.h), compiled for the Beat clock by
// compileRoutine (src/recording/Program.h), saved inside the composition JSON
// (Composition::routines / routineBank). Header-only like Composition.h/Deck.h.
// Slice 1 (s-rta-0926, plan-routines-s1-final.md 3.1-3.2).
struct Routine
{
    std::string uuid;                 // juce::Uuid().toString(); identity; the bank references it
    std::string name;                 // whole words, user-typed; the caller supplies "Routine N" when empty
    double lengthBeats = 16.0;        // whole bars by default (D4 step 5); the Player's end (Program::length)
    Clip::BeatSnapMode quantize = Clip::BeatSnapMode::Bar;   // ruling 27; global Quantize overrides when on
    bool loop = false;                // ruling 22: once (default) / loop
    bool restoreState = true;         // ruling 26: restore (default) / "start from now"
    // s-rta-0926b (Boris: "we should have controls for jump or ease in each"): HOW the restore reaches the
    // recorded start look -- at the start, every loop return and a re-fire restart. Ease (default) glides
    // over the last beat onto the boundary (RoutineEngine, plan3 C); Jump restores in one call ON it.
    enum class RestoreStyle { Ease, Jump };
    RestoreStyle restoreStyle = RestoreStyle::Ease;
    bool deckRelative = true;         // D2/D9: keys resolve on the ACTIVE deck at fire time; layer = recorded (ruling 17)

    // Where it came from (display + re-slice later; never identity).
    struct Source { std::string takeFolder; double fromBeat = 0.0, toBeat = 0.0; };
    Source source;

    // D4 step 2 restore list, explicit: "make the world look the way it did for the things I am
    // about to touch". Discrete entries carry v or action (the same vocabulary dispatch.fire
    // accepts); continuous entries carry a NORMALISED value (what Sink::set takes). ORDER IS
    // LOAD-BEARING (R5): the slicer emits layer flags, then activeClip, then clip play/pause;
    // Player::firePreamble fires all discrete before all continuous.
    struct PreambleEntry
    {
        ControlPath key;              // deckRelative = true, positional layer/col/fx/param + names
        bool continuous = false;
        int v = 0;                    // discrete: state value (activeClip column, 0/1 flag)
        std::string action;           // discrete, action-valued controls ("resume" | "pause")
        float norm = 0.0f;            // continuous: [0,1]

        juce::var toVar() const
        {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("key", key.toVar());
            if (continuous)
            {
                obj->setProperty("norm", static_cast<double>(norm));
            }
            else
            {
                obj->setProperty("v", v);
                if (!action.empty())
                    obj->setProperty("action", juce::String(action));
            }
            return juce::var(obj);
        }

        static PreambleEntry fromVar(const juce::var& v)
        {
            PreambleEntry e;
            if (auto* obj = v.getDynamicObject())
            {
                e.key = ControlPath::fromVar(obj->getProperty("key"));
                e.continuous = obj->hasProperty("norm");
                if (e.continuous)
                    e.norm = static_cast<float>(static_cast<double>(obj->getProperty("norm")));
                if (obj->hasProperty("v"))
                    e.v = static_cast<int>(obj->getProperty("v"));
                if (obj->hasProperty("action"))
                    e.action = obj->getProperty("action").toString().toStdString();
            }
            return e;
        }
    };
    std::vector<PreambleEntry> preamble;

    std::map<ControlPath, Lane> lanes;   // x / beat in ROUTINE beats (0 = the fire boundary); Gesture::stamps EMPTY

    // Enums are strings (D12): "off" | "beat" | "bar" | "2bar" | "4bar".
    static const char* quantizeToString(Clip::BeatSnapMode m)
    {
        switch (m)
        {
            case Clip::BeatSnapMode::Off:     return "off";
            case Clip::BeatSnapMode::Beat:    return "beat";
            case Clip::BeatSnapMode::Bar:     return "bar";
            case Clip::BeatSnapMode::TwoBar:  return "2bar";
            case Clip::BeatSnapMode::FourBar: return "4bar";
        }
        return "bar";
    }

    // Unknown -> Bar (never silently Off: an unreadable setting must not make a routine
    // fire off the grid).
    static Clip::BeatSnapMode quantizeFromString(const juce::String& s)
    {
        if (s == "off")  return Clip::BeatSnapMode::Off;
        if (s == "beat") return Clip::BeatSnapMode::Beat;
        if (s == "2bar") return Clip::BeatSnapMode::TwoBar;
        if (s == "4bar") return Clip::BeatSnapMode::FourBar;
        return Clip::BeatSnapMode::Bar;
    }

    // "ease" | "jump". Unknown -- and a routine saved before the setting existed -- reads as Ease.
    static const char* restoreStyleToString(RestoreStyle s) { return s == RestoreStyle::Jump ? "jump" : "ease"; }
    static RestoreStyle restoreStyleFromString(const juce::String& s)
    {
        return s == "jump" ? RestoreStyle::Jump : RestoreStyle::Ease;
    }

    juce::var toVar() const
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("uuid", juce::String(uuid));
        obj->setProperty("name", juce::String(name));
        obj->setProperty("lengthBeats", lengthBeats);
        obj->setProperty("quantize", juce::String(quantizeToString(quantize)));
        obj->setProperty("loop", loop);
        obj->setProperty("restoreState", restoreState);
        obj->setProperty("restoreStyle", juce::String(restoreStyleToString(restoreStyle)));
        obj->setProperty("deckRelative", deckRelative);

        auto* srcObj = new juce::DynamicObject();
        srcObj->setProperty("takeFolder", juce::String(source.takeFolder));
        srcObj->setProperty("fromBeat", source.fromBeat);
        srcObj->setProperty("toBeat", source.toBeat);
        obj->setProperty("source", juce::var(srcObj));

        juce::Array<juce::var> pre;
        for (const auto& e : preamble)
            pre.add(e.toVar());
        obj->setProperty("preamble", pre);

        juce::Array<juce::var> laneArr;
        for (const auto& [key, lane] : lanes)
            laneArr.add(lane.toVar());
        obj->setProperty("lanes", laneArr);
        return juce::var(obj);
    }

    static Routine fromVar(const juce::var& v)
    {
        Routine r;
        auto* obj = v.getDynamicObject();
        if (!obj)
            return r;

        r.uuid = obj->getProperty("uuid").toString().toStdString();
        r.name = obj->getProperty("name").toString().toStdString();
        if (obj->hasProperty("lengthBeats"))
            r.lengthBeats = static_cast<double>(obj->getProperty("lengthBeats"));
        if (obj->hasProperty("quantize"))
            r.quantize = quantizeFromString(obj->getProperty("quantize").toString());
        if (obj->hasProperty("loop"))
            r.loop = static_cast<bool>(obj->getProperty("loop"));
        if (obj->hasProperty("restoreState"))
            r.restoreState = static_cast<bool>(obj->getProperty("restoreState"));
        if (obj->hasProperty("restoreStyle"))
            r.restoreStyle = restoreStyleFromString(obj->getProperty("restoreStyle").toString());
        if (obj->hasProperty("deckRelative"))
            r.deckRelative = static_cast<bool>(obj->getProperty("deckRelative"));

        if (auto* srcObj = obj->getProperty("source").getDynamicObject())
        {
            r.source.takeFolder = srcObj->getProperty("takeFolder").toString().toStdString();
            r.source.fromBeat = static_cast<double>(srcObj->getProperty("fromBeat"));
            r.source.toBeat = static_cast<double>(srcObj->getProperty("toBeat"));
        }

        if (auto* pre = obj->getProperty("preamble").getArray())
            for (const auto& ev : *pre)
                r.preamble.push_back(PreambleEntry::fromVar(ev));

        if (auto* laneArr = obj->getProperty("lanes").getArray())
            for (const auto& lv : *laneArr)
            {
                Lane lane = Lane::fromVar(lv);
                r.lanes[lane.key] = std::move(lane);
            }
        return r;
    }
};

// One pad of the routine bank: which routine (by uuid) sits on which slot.
struct RoutineSlot { int slot = -1; std::string uuid; };
