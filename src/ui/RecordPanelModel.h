#pragma once
#include "recording/RecorderHost.h"   // juce_core-only header (see its A7 note)
#include <juce_core/juce_core.h>
#include <cmath>
#include <string>

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
    if (!std::isfinite(seconds) || seconds < 0.0)
        return "0:00";
    const auto total = static_cast<long long>(std::floor(seconds));
    const long long h = total / 3600, m = (total / 60) % 60, sec = total % 60;
    const auto two = [](long long v) { return juce::String(v).paddedLeft('0', 2); };
    if (h > 0)
        return juce::String(h) + ":" + two(m) + ":" + two(sec);
    return juce::String(m) + ":" + two(sec);
}

namespace recordpanel_detail
{
    // Separators (UTF-8, built with fromUTF8 -- juce::String's const char* constructor is ASCII-only).
    inline juce::String dot()  { return juce::String::fromUTF8(" \xc2\xb7 "); }       // " · "
    inline juce::String dash() { return juce::String::fromUTF8(" \xe2\x80\x94 "); }   // " — "

    inline juce::String count(long long n, const char* one, const char* many)
    {
        return juce::String(n) + " " + (n == 1 ? one : many);
    }

    // The take's name as the user typed it: the folder name without ".adna-take".
    inline juce::String takeName(const std::string& folder)
    {
        return juce::File(juce::String(folder)).getFileNameWithoutExtension();
    }

    inline juce::String audioWord(const std::string& status)
    {
        if (status == "Resolved")            return "ready";
        if (status == "ResolvedUnverified")  return "ready (unverified)";
        if (status == "Incomplete")          return "incomplete, repair available";
        if (status == "Missing")             return "missing";
        if (status == "Mismatch")            return "does not match";
        if (status == "Legacy")              return "old format, record again";
        if (status == "MultiSegment")        return "unsupported";
        return "none";
    }

    inline juce::String playingReadout(const RecorderHost::Status& s)
    {
        juce::String t = "Playing " + formatClock(s.positionSeconds) + " / " + formatClock(s.lengthSeconds)
                       + dot() + "unresolved: " + juce::String(s.unresolved);
        const int moved = s.reboundByPosition + s.reboundByName;
        if (moved > 0)     t << dot() << "moved: " << juce::String(moved);
        if (s.skipped > 0) t << dot() << "skipped: " << juce::String(s.skipped);
        return t;
    }

    inline juce::String recordingCounts(const RecorderHost::Status& s)
    {
        return dot() + count(s.lanes, "lane", "lanes") + dot() + count(s.points + s.gestures, "move", "moves");
    }
}

inline RecordPanelView deriveRecordPanelView(const RecorderHost::Status& s, const RecordPanelInputs& in)
{
    using namespace recordpanel_detail;
    using Tone = RecordPanelView::Tone;

    // Every predicate comes from Status -- never from panel memory.
    const bool recording = s.recording;
    const bool overdub = s.overdub;
    const bool playing = s.playing;
    const bool withAudio = playing && s.playMode == "withAudio";
    const bool loaded = !s.loadedTakeFolder.empty();
    const bool audioRequested = recording && !s.assetId.empty() && !overdub;
    const bool armed = audioRequested && s.framesWritten == 0;
    const bool audioReady = s.audioStatus == "Resolved" || s.audioStatus == "ResolvedUnverified";
    const bool audioIncomplete = s.audioStatus == "Incomplete";
    const bool idle = !recording && !playing;

    RecordPanelView v;

    // ---- Record: "Record Take" <-> "Stop Recording"; "Record Over" while replaying with audio ----
    if (recording)
        v.record = { "Stop Recording", true, "Stops this take. It is saved automatically.", Tone::Recording };
    else if (withAudio)
    {
        v.record = { "Record Over", true,
                     "Records a new take over this take's audio, from the current playback position.", Tone::Neutral };
        v.recordSendsOverdub = true;
    }
    else
        v.record = { "Record Take", true, "Starts a new take. Audio is recorded too when Record audio is on.", Tone::Neutral };

    // ---- Play: "Play Take" <-> "Stop Playback" ----
    if (playing)
    {
        // An overdub's clock is the replayed audio: the panel stops the recording first (row 10).
        const bool canStop = !(recording && overdub);
        v.play = { "Stop Playback", canStop, canStop ? "Stops the replay." : "Stop the recording first.", Tone::Playing };
    }
    else if (recording)
        v.play = { "Play Take", false, "Stop the recording first.", Tone::Neutral };
    else if (!loaded)
        v.play = { "Play Take", false, "Load a take first.", Tone::Neutral };
    else
        v.play = { "Play Take", true, "Replays the loaded take.", Tone::Neutral };

    // ---- Load / Show in Finder / Repair ----
    v.load = { "Load Take...", idle,
               idle ? "Choose a take folder (.adna-take)."
                    : (recording ? "Stop the recording first." : "Stop the playback first."),
               Tone::Neutral };

    if (recording)             v.revealFolder = juce::String(s.takeFolder);
    else if (loaded)           v.revealFolder = juce::String(s.loadedTakeFolder);
    else                       v.revealFolder = juce::String(s.takeFolder);
    v.reveal = { "Show in Finder", v.revealFolder.isNotEmpty(),
                 v.revealFolder.isNotEmpty() ? "Shows the take folder in the Finder." : "No take yet.", Tone::Neutral };

    const bool canRepair = idle && loaded && audioIncomplete;
    v.repair = { "Repair Audio", canRepair,
                 canRepair ? "Rebuilds the audio record of a take that was cut off, for example by a crash."
                           : "Only needed when a loaded take's audio was cut off, for example by a crash.",
                 Tone::Neutral };

    // ---- toggles and name ----
    v.recordAudioEnabled = !recording && !withAudio;
    v.recordAudioTooltip = withAudio ? "Recording over a take always uses that take's audio."
                                     : "Records the sound the app is listening to, alongside the timelines.";
    v.playWithAudioEnabled = idle && loaded && audioReady;
    v.playWithAudioValue = playing ? withAudio : (v.playWithAudioEnabled && in.playWithAudio);
    v.playWithAudioTooltip = (loaded && !audioReady && !playing)
        ? "This take's audio is not available, so it replays without audio."
        : "Replays the take's own audio instead of the live input.";
    v.nameEnabled = !recording;

    // ---- status line ----
    if (recording)
    {
        v.statusTone = Tone::Recording;
        if (armed)
            v.statusText = "Armed, waiting for audio...";
        else if (overdub)
        {
            const juce::String over = (loaded && s.loadedAssetId == s.assetId) ? takeName(s.loadedTakeFolder)
                                                                              : juce::String("stored audio");
            v.statusText = "Recording over " + over + " " + formatClock(s.t) + recordingCounts(s);
            if (playing)
                v.statusText << dash() << playingReadout(s);
        }
        else
        {
            v.statusText = "Recording " + formatClock(s.t)
                         + (audioRequested ? (s.audioMode == "file" ? " from audio file" : " from live input")
                                           : " without audio")
                         + recordingCounts(s);
            if (s.gaps > 0)
                v.statusText << dot() << count(s.gaps, "audio gap", "audio gaps");
            if (playing)
                v.statusText << dash() << "playing " << formatClock(s.positionSeconds);
        }
    }
    else if (playing)
    {
        v.statusTone = Tone::Playing;
        v.statusText = playingReadout(s);
    }
    else if (loaded)
        v.statusText = "Loaded: " + takeName(s.loadedTakeFolder) + dash() + formatClock(s.loadedDuration)
                     + dot() + count(s.loadedLanes, "lane", "lanes") + dot() + "Audio: " + audioWord(s.audioStatus);
    else if (!s.takeFolder.empty())
        v.statusText = "Ready. Last take: " + takeName(s.takeFolder);
    else
        v.statusText = "Ready. No take loaded.";

    // ---- warning line (first match wins) ----
    if (recording && !s.lastError.empty())
        v.warningText = juce::String(s.lastError);
    else if (armed && s.t > kArmedWarnSeconds)
        v.warningText = "No audio is arriving. Check the input source.";
    else if (withAudio)
        v.warningText = "Replaying with the take's audio. Live input is paused until Stop Playback.";
    else if (recording && s.humanRefused > 0)
        v.warningText = count(s.humanRefused, "move was", "moves were") + " refused because a control was already held.";
    else if (playing && s.continuousUnavailable > 0)
        v.warningText = count(s.continuousUnavailable, "knob move", "knob moves") + " could not be replayed.";

    // ---- notice line ----
    if (in.notice.isNotEmpty() && in.noticeAtSeconds >= 0.0 && in.nowSeconds - in.noticeAtSeconds <= kNoticeSeconds)
        v.noticeText = in.notice;

    return v;
}
