#pragma once
#include "recording/RecorderHost.h"   // juce_core-only header (see its A7 note)
#include <juce_core/juce_core.h>

// RecordPanelModel -- s-rta-0924b step 4 (Lane S4-M). A PURE function from the recorder's published
// RecorderHost::Status (plus the few inputs the panel itself owns) to everything the Record panel
// shows: each button's text / enabled / tooltip / tone, the two toggles, the status, warning and
// notice lines. No juce_gui_basics here -- the click-through matrix is pinned by a Catch2 table test
// (tests/test_record_panel_model.cpp) that links juce_core only, and this is the piece meant to
// outlive the UI rewrite (ruling 5). Every state is derived from Status, never from panel memory,
// so a shadow "am I recording" bool (the old G25 class) cannot exist.

struct RecordPanelInputs
{
    double nowSeconds = 0.0;          // wall clock (Time::getMillisecondCounterHiRes()/1000)
    bool   recordAudio = true;        // ruling 19's switch, user-owned
    bool   playWithAudio = true;      // user-owned preference; forced off by the model when audio is not ready
    juce::String notice;              // last refusal/notify text
    double noticeAtSeconds = -1.0;    // when it was set (-1 = never); expires after kNoticeSeconds
};

struct RecordPanelView
{
    enum class Tone { Neutral, Recording, Playing, Warning };
    struct Button { juce::String text; bool enabled = false; juce::String tooltip; Tone tone = Tone::Neutral; };
    Button record, play, load, reveal, repair;
    bool recordAudioEnabled = true, playWithAudioEnabled = false, playWithAudioValue = false, nameEnabled = true;
    juce::String recordAudioTooltip, playWithAudioTooltip;
    bool recordSendsOverdub = false;  // what pressing Record would request right now
    juce::String statusText; Tone statusTone = Tone::Neutral;
    juce::String warningText;         // "" when none
    juce::String noticeText;          // "" when none/expired
    juce::String revealFolder;        // "" => Show in Finder disabled
};

inline constexpr double kNoticeSeconds = 10.0;
inline constexpr double kArmedWarnSeconds = 2.0;

// "m:ss", "h:mm:ss" past one hour; negative/NaN -> "0:00". Whole seconds, no tenths (plan 5 #10).
inline juce::String formatClock(double seconds)
{
    (void) seconds;
    return {};   // FAIL-FIRST STUB
}

inline RecordPanelView deriveRecordPanelView(const RecorderHost::Status& s, const RecordPanelInputs& in)
{
    (void) s; (void) in;
    return RecordPanelView{};   // FAIL-FIRST STUB
}
