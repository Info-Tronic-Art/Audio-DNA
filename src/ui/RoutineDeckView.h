#pragma once
#include "recording/RoutineEngine.h"
#include "ui/RoutineBankModel.h"   // kRoutineBeatsPerBar
#include <juce_core/juce_core.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <vector>

// RoutineDeckView -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 3.5; design
// routine-ux/design-final.md 2.3). A PURE function from RoutineEngine::Status to everything the deck's
// ROUTINES row (eight RoutinePads + the corner note) and the layer strips' routine bands show, plus the
// pad settings menu's items. juce_core only (RoutineEngine.h is juce_core-only by rule), so it is pinned
// by a Catch2 test that links juce_core alone (tests/test_routine_deck_view.cpp).
//
// A pad has exactly ONE press action, Fire (restart while playing, a no-op while waiting) -- there is
// no press-action field to get wrong. A routine leaves by a band's x (the whole routine), the layer X,
// its own end, the pad menu's "Remove from layers", or Stop (routines only).
struct RoutineDeckView
{
    enum class State { Empty, Idle, Waiting, Playing };

    struct Pad
    {
        int number = 0;                 // 1-based: what keys / MIDI / OSC / REST fire
        juce::String name;
        State state = State::Empty;
        bool onShownDeck = true;        // false: it plays on another deck (the pad paints at 50 %)
        bool loop = false, restoreFirst = true, startEase = true;   // the menu's ticks
        juce::String quantize;          // "off" | "beat" | "bar" | "2bar" | "4bar"
        float progress01 = 0.0f;        // Playing: position / length
        int bar = 0, barsTotal = 0;     // Playing: "5/8"
        bool restartPending = false;    // Playing, pressed again: restarts from the top on its next line
        bool warning = false;           // something could not be restored / played: the red "!"
        juce::String tooltip;
    };
    Pad pads[RoutineEngine::kBankSize];

    struct Band
    {
        int slot = -1;
        juce::String name;
        State state = State::Idle;      // Waiting or Playing
        float progress01 = 0.0f;
    };
    // The SHOWN deck's layer index -> every band on it, newest fire first (bandsToDraw trims to two).
    std::map<int, std::vector<Band>> bandsByLayer;

    juce::String cornerNote;            // after "ROUTINES": "· Save one in the Record tab" / "· Drop on Deck 2" / ""

    enum class PadMenu : int
    {
        Loop = 1, Once, RestoreFirst, StartFromNow, StartEase, StartJump,
        QuantizeOff, QuantizeBeat, QuantizeBar, QuantizeTwoBar, QuantizeFourBar,
        Rename, RemoveFromLayers, DeleteRoutine
    };
    // One menu row. `inQuantizeSubmenu` rows go into the "Quantize" submenu, which sits where its first
    // row appears; `separatorBefore` on that first row puts the separator before the submenu.
    struct MenuItem
    {
        PadMenu id;
        juce::String label;
        bool enabled = true, ticked = false, separatorBefore = false, inQuantizeSubmenu = false;
        bool destructive = false;       // irreversible: DeckView paints it in the app's warning red
    };
};

// What a settings row writes through perfRoutineSet (fields left empty are not changed).
struct RoutineSettingsChange
{
    std::optional<bool> loop, restoreState;
    juce::String restoreStyle, quantize;
};

namespace routine_deck_view_detail
{
    inline juce::String dot() { return juce::String::fromUTF8("\xc2\xb7"); }   // the corner note's "·"

    inline RoutineDeckView::State stateOf(const std::string& s)
    {
        if (s == "idle")    return RoutineDeckView::State::Idle;
        if (s == "pending") return RoutineDeckView::State::Waiting;
        if (s == "running") return RoutineDeckView::State::Playing;
        return RoutineDeckView::State::Empty;
    }

    inline juce::String padName(const RoutineEngine::Status::Slot& s, int i)
    {
        return s.name.empty() ? "Routine " + juce::String(i + 1) : juce::String(s.name);
    }

    inline juce::String counted(int n, const char* one, const char* many)
    {
        return juce::String(n) + " " + (n == 1 ? one : many);
    }

    inline juce::String nameAt(const std::vector<juce::String>& names, int idx, const char* fallback)
    {
        if (idx >= 0 && idx < static_cast<int>(names.size()) && names[static_cast<size_t>(idx)].isNotEmpty())
            return names[static_cast<size_t>(idx)];
        return juce::String(fallback) + " " + juce::String(idx + 1);
    }

    // A pending restart's line (s-rta-0927 fix round): "Restarting from the top on the next bar."
    inline juce::String restartText(const std::string& startsOn)
    {
        if (startsOn == "now")  return "Restarting from the top now.";
        if (startsOn == "beat") return "Restarting from the top on the next beat.";
        if (startsOn == "2bar") return "Restarting from the top on the next two-bar line.";
        if (startsOn == "4bar") return "Restarting from the top on the next four-bar line.";
        return "Restarting from the top on the next bar.";
    }

    inline juce::String startsOnText(const std::string& startsOn)
    {
        if (startsOn == "now")  return "Starting now.";
        if (startsOn == "beat") return "Starting on the next beat.";
        if (startsOn == "2bar") return "Starting on the next two-bar line.";
        if (startsOn == "4bar") return "Starting on the next four-bar line.";
        return "Starting on the next bar.";
    }

    // "2 settings could not be restored; 1 timeline points at a layer or clip that no longer exists; 3 clip
    // changes could not be made. " -- only the non-zero clauses; "" when there is nothing to say.
    inline juce::String warningText(const RoutineEngine::Status::Slot& s)
    {
        juce::StringArray parts;
        if (s.preambleUnresolved > 0)
            parts.add(counted(s.preambleUnresolved, "setting", "settings") + " could not be restored");
        if (s.unresolved > 0)
            parts.add(counted(s.unresolved, "timeline points", "timelines point") + " at a layer or clip that no longer exists");
        if (s.skipped > 0)
            parts.add(counted(s.skipped, "clip change", "clip changes") + " could not be made");
        return parts.isEmpty() ? juce::String() : parts.joinIntoString("; ") + ". ";
    }
}

inline RoutineDeckView deriveRoutineDeckView(const RoutineEngine::Status& status, int shownDeck,
                                             const std::vector<juce::String>& deckNames,
                                             const std::vector<juce::String>& shownDeckLayerNames)
{
    using namespace routine_deck_view_detail;
    using State = RoutineDeckView::State;
    RoutineDeckView v;
    bool allEmpty = true;

    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
    {
        const auto& s = status.slots[i];
        auto& pad = v.pads[i];
        pad.number = i + 1;
        pad.state = stateOf(s.state);
        if (pad.state == State::Empty)
        {
            pad.tooltip = "No routine is saved on this pad yet. Save one in the Record tab.";
            continue;
        }
        allEmpty = false;
        pad.name = padName(s, i);
        pad.onShownDeck = s.deck < 0 || s.deck == shownDeck;
        pad.loop = s.loop;
        pad.restoreFirst = s.restoreState;
        pad.startEase = s.restoreStyle != "jump";
        pad.quantize = juce::String(s.quantize);
        pad.warning = s.unresolved + s.preambleUnresolved + s.skipped > 0;

        juce::String tip;
        switch (pad.state)
        {
            case State::Idle:
                tip = "Press to play. Right-click for settings.";
                break;
            case State::Waiting:
                tip = startsOnText(s.startsOn);
                break;
            case State::Playing:
            {
                if (s.lengthBeats > 0.0)
                {
                    pad.progress01 = static_cast<float>(std::clamp(s.position / s.lengthBeats, 0.0, 1.0));
                    pad.barsTotal = static_cast<int>(std::ceil(s.lengthBeats / kRoutineBeatsPerBar));
                    pad.bar = std::clamp(static_cast<int>(std::floor(s.position / kRoutineBeatsPerBar)) + 1, 1,
                                         std::max(1, pad.barsTotal));
                }
                pad.restartPending = s.restartPending;
                if (pad.restartPending)
                {
                    tip = restartText(s.startsOn);
                }
                else if (!pad.onShownDeck)
                {
                    tip = "Playing on " + nameAt(deckNames, s.deck, "Deck") + ". Switch decks to see its layers.";
                }
                else
                {
                    juce::StringArray names;
                    for (int l : s.layers)
                        names.add(nameAt(shownDeckLayerNames, l, "Layer"));
                    tip = (names.isEmpty() ? juce::String("Playing.") : "Playing on " + names.joinIntoString(", ") + ".")
                        + " Press to restart from the top.";
                }
                break;
            }
            case State::Empty:
                break;
        }
        pad.tooltip = warningText(s) + tip;
    }

    // Bands: every waiting / playing routine on the SHOWN deck, one per layer it plays on, newest fire first.
    std::vector<int> live;
    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
    {
        const auto st = v.pads[i].state;
        if ((st == State::Waiting || st == State::Playing) && status.slots[i].deck >= 0
            && status.slots[i].deck == shownDeck)
            live.push_back(i);
    }
    std::stable_sort(live.begin(), live.end(), [&status](int a, int b) {
        return status.slots[a].fireSeq > status.slots[b].fireSeq;
    });
    for (int i : live)
        for (int l : status.slots[i].layers)
            v.bandsByLayer[l].push_back({ i, v.pads[i].name, v.pads[i].state,
                                          v.pads[i].state == State::Playing ? v.pads[i].progress01 : 0.0f });

    // Corner note.
    if (allEmpty)
    {
        v.cornerNote = dot() + " Save one in the Record tab";
    }
    else
    {
        std::map<int, juce::StringArray> offDeck;   // deck index -> names, in pad order
        for (int i = 0; i < RoutineEngine::kBankSize; ++i)
        {
            const auto st = v.pads[i].state;
            const int deck = status.slots[i].deck;
            if ((st == State::Waiting || st == State::Playing) && deck >= 0 && deck != shownDeck)
                offDeck[deck].add(v.pads[i].name);
        }
        juce::StringArray groups;
        for (const auto& [deck, names] : offDeck)
            groups.add(names.joinIntoString(", ") + " on " + nameAt(deckNames, deck, "Deck"));
        if (!groups.isEmpty())
            v.cornerNote = dot() + " " + groups.joinIntoString("; ");
    }
    return v;
}

// At most two bands fit on a strip's picture: a third or more becomes "+N" on the second.
inline std::vector<RoutineDeckView::Band> bandsToDraw(const std::vector<RoutineDeckView::Band>& bands)
{
    if (bands.size() <= 2)
        return bands;
    std::vector<RoutineDeckView::Band> out(bands.begin(), bands.begin() + 2);
    out[1].name += " +" + juce::String(static_cast<int>(bands.size()) - 2);
    return out;
}

// The pad's right-click settings menu, in order. There is no "Stop" row anywhere.
inline std::vector<RoutineDeckView::MenuItem> padMenu(const RoutineDeckView::Pad& pad)
{
    using M = RoutineDeckView::PadMenu;
    using State = RoutineDeckView::State;
    const bool live = pad.state == State::Waiting || pad.state == State::Playing;
    return {
        { M::Loop,             "Loop",               true, pad.loop,             false, false },
        { M::Once,             "Once",               true, !pad.loop,            false, false },
        { M::RestoreFirst,     "Restore first",      true, pad.restoreFirst,     false, false },
        { M::StartFromNow,     "Start from now",     true, !pad.restoreFirst,    false, false },
        { M::StartEase,        "Start: Ease",        true, pad.startEase,        false, false },
        { M::StartJump,        "Start: Jump",        true, !pad.startEase,       false, false },
        { M::QuantizeOff,      "Off",                true, pad.quantize == "off",  true, true },
        { M::QuantizeBeat,     "Beat",               true, pad.quantize == "beat", false, true },
        { M::QuantizeBar,      "Bar",                true, pad.quantize == "bar",  false, true },
        { M::QuantizeTwoBar,   "2 Bar",              true, pad.quantize == "2bar", false, true },
        { M::QuantizeFourBar,  "4 Bar",              true, pad.quantize == "4bar", false, true },
        { M::Rename,           "Rename...",          true, false,                false, false },
        { M::RemoveFromLayers, "Remove from layers", live, false,                false, false },
        { M::DeleteRoutine,    "Delete routine",     true, false,                true,  false, true },
    };
}

// A settings row -> what it writes (Rename / Remove from layers / Delete routine write nothing here).
inline RoutineSettingsChange settingsChangeFor(RoutineDeckView::PadMenu id)
{
    using M = RoutineDeckView::PadMenu;
    RoutineSettingsChange c;
    switch (id)
    {
        case M::Loop:            c.loop = true; break;
        case M::Once:            c.loop = false; break;
        case M::RestoreFirst:    c.restoreState = true; break;
        case M::StartFromNow:    c.restoreState = false; break;
        case M::StartEase:       c.restoreStyle = "ease"; break;
        case M::StartJump:       c.restoreStyle = "jump"; break;
        case M::QuantizeOff:     c.quantize = "off"; break;
        case M::QuantizeBeat:    c.quantize = "beat"; break;
        case M::QuantizeBar:     c.quantize = "bar"; break;
        case M::QuantizeTwoBar:  c.quantize = "2bar"; break;
        case M::QuantizeFourBar: c.quantize = "4bar"; break;
        case M::Rename:
        case M::RemoveFromLayers:
        case M::DeleteRoutine:   break;
    }
    return c;
}
