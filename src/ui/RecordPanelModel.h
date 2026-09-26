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

// ---- the situation a notice belongs to (s-rta-0925 step-4 polish, fix-plan section 2 "stale notice") ----
// A notice is advice about the recorder's situation when it was raised ("No take is loaded...", "Saved: x").
// It is shown while that situation lasts and dropped when it changes -- whichever comes first with the
// kNoticeSeconds expiry. The key holds exactly the Status fields whose change is a row change in the
// click-through matrix (rows 1-2 vs 3-5: loadedTakeFolder; rows 6-12: recording/playing/overdub/playMode), and
// ONLY fields the host publishes synchronously inside the transition that writes them (RecorderHost::disarm,
// load, play, stopPlay, stopPlayback -- and arm, since this lane). INVARIANT for future host edits: a write to
// any of these fields must be followed by publishStatus() in the same function, or a notice raised inside that
// transition is keyed to the OLD situation and silently dropped at the next refresh.
// Deliberately NOT in the key: takeFolder (a refused arm has already overwritten it before refusing, and the
// next tick publishes it AFTER the refusal notice -- it would erase "Could not start the take" within 250 ms),
// audioStatus (repair re-resolves it with no transition of its own), framesWritten (armed -> recording is the
// same take), t / position / every counter (they change every tick).
struct RecordPanelNoticeKey
{
    bool recording = false, playing = false, overdub = false;
    std::string playMode, loadedTakeFolder;
    // s-rta-0925 end-of-replay: LAST member so the aggregate stays positional (see noticeKeyOf's own
    // callers -- only through this function, never a brace-init of the struct). A notice raised while
    // playing is dropped at the finish (a row change: playing/!finished -> playing/finished); the
    // finish notice survives until Stop Playback (playing flips) or kNoticeSeconds.
    bool finished = false;
    bool operator==(const RecordPanelNoticeKey&) const = default;
};

inline RecordPanelNoticeKey noticeKeyOf(const RecorderHost::Status& s)
{
    return { s.recording, s.playing, s.overdub, s.playMode, s.loadedTakeFolder, s.finished };
}

struct RecordPanelInputs
{
    double nowSeconds = 0.0;          // wall clock (Time::getMillisecondCounterHiRes()/1000)
    bool   recordAudio = true;        // ruling 19's switch, user-owned
    bool   playWithAudio = true;      // user-owned preference; forced off by the model when audio is not ready
    juce::String notice;              // last refusal/notify text
    double noticeAtSeconds = -1.0;    // when it was set (-1 = never); expires after kNoticeSeconds
    RecordPanelNoticeKey noticeKey;   // the situation the notice was raised in:
                                      // noticeKeyOf(status) read AFTER the event that produced it
};

struct RecordPanelView
{
    enum class Tone { Neutral, Recording, Playing, Warning };
    struct Button { juce::String text; bool enabled = false; juce::String tooltip; Tone tone = Tone::Neutral; };
    Button record, play, load, reveal, repair;
    bool recordAudioEnabled = true, playWithAudioEnabled = false, playWithAudioValue = false, nameEnabled = true;
    juce::String recordAudioTooltip, playWithAudioTooltip, nameTooltip;
    bool recordSendsOverdub = false;  // what pressing Record would request right now
    juce::String statusText; Tone statusTone = Tone::Neutral;
    juce::String warningText;         // "" when none
    juce::String noticeText;          // "" when none/expired
    bool noticeLive = false;          // the stored notice is shown this frame; false = expired or its
                                       // situation is over -- the panel forgets it
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

    // s-rta-0925 end-of-replay (Boris ruling 2026-09-25 "hold, don't stop"): the take's real end has
    // been reached; the last look stays exactly as it is (Stop Playback is the only exit).
    inline juce::String finishedReadout(const RecorderHost::Status& s)
    {
        return "Finished " + takeName(s.loadedTakeFolder) + dash() + "holding the last look";
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
    // s-rta-0925 end-of-replay (Boris ruling 2026-09-25 "hold, don't stop"): `finished` is a
    // sub-state of `playing` (the host keeps `playing` true at the end, section 1). `withAudio`
    // excludes it: once finished, Record Over / the "Live input is paused" warning / the F5 hint
    // all go away by derivation (the take's audio has stopped driving anything -- the input is
    // already back, or on its way back).
    const bool finished = playing && s.finished;
    const bool withAudio = playing && !finished && s.playMode == "withAudio";
    const bool loaded = !s.loadedTakeFolder.empty();
    const bool audioRequested = recording && !s.assetId.empty() && !overdub;
    const bool armed = audioRequested && s.framesWritten == 0;
    const bool audioReady = s.audioStatus == "Resolved" || s.audioStatus == "ResolvedUnverified";
    const bool audioIncomplete = s.audioStatus == "Incomplete";
    const bool idle = !recording && !playing;

    RecordPanelView v;

    // ---- Record: "Record Take" <-> "Stop Recording"; "Record Over" while replaying with audio ----
    if (recording)
        // Fix plan F6: during an overdub with a replay (row 10) Stop Recording saves the take but leaves
        // the replay running -- the tooltip says so.
        v.record = { "Stop Recording", true,
                     (overdub && playing) ? "Stops this take. It is saved automatically. The replay keeps playing."
                                          : "Stops this take. It is saved automatically.",
                     Tone::Recording };
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
        v.play = { "Stop Playback", canStop,
                   !canStop ? "Stop the recording first."
                            : (finished ? "Ends the replay. The look stays as it is." : "Stops the replay."),
                   Tone::Playing };
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

    // ---- toggles and name: a locked control says why in its tooltip (s-rta-0925 D1, the tooltip version --
    // JUCE shows tooltips on disabled components; the on-screen caption stays deferred, fix-plan D1) ----
    v.recordAudioEnabled = !recording && !withAudio;
    v.recordAudioTooltip = recording  ? "Locked while a take is recording."
                         : withAudio  ? "Recording over a take always uses that take's audio."
                                      : "Records the sound the app is listening to, alongside the timelines.";
    v.playWithAudioEnabled = idle && loaded && audioReady;
    // s-rta-0925: keep showing the ACTUAL mode while finished -- `withAudio` above now excludes
    // `finished`, so this reads playMode directly rather than through that predicate.
    v.playWithAudioValue = playing ? (s.playMode == "withAudio") : (v.playWithAudioEnabled && in.playWithAudio);
    if (recording)                       v.playWithAudioTooltip = "Stop the recording first.";
    else if (playing)                    v.playWithAudioTooltip = finished ? "Stop the playback first." : "Locked while the take replays.";
    else if (loaded && !audioReady)      v.playWithAudioTooltip = "This take's audio is not available, so it replays without audio.";
    else if (!loaded)                    v.playWithAudioTooltip = "Load a take first.";
    else                                 v.playWithAudioTooltip = "Replays the take's own audio instead of the live input.";
    v.nameEnabled = !recording;
    v.nameTooltip = recording ? "Locked while a take is recording. It names the next take."
                              : "The name of the next take. Leave it blank to name it by date and time.";

    // ---- status line ----
    if (recording)
    {
        v.statusTone = Tone::Recording;
        if (armed)
        {
            v.statusText = "Armed, waiting for audio...";
            if (playing)   // fix plan F7: the mirror of the plain-recording branch below
                v.statusText << dash() << (finished ? "holding the last look" : "playing " + formatClock(s.positionSeconds));
        }
        else if (overdub)
        {
            const juce::String over = (loaded && s.loadedAssetId == s.assetId) ? takeName(s.loadedTakeFolder)
                                                                              : juce::String("stored audio");
            v.statusText = "Recording over " + over + " " + formatClock(s.t) + recordingCounts(s);
            if (playing)
                v.statusText << dash() << (finished ? juce::String("holding the last look") : playingReadout(s));
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
                v.statusText << dash() << (finished ? "holding the last look" : "playing " + formatClock(s.positionSeconds));
        }
    }
    else if (playing)
    {
        if (finished) { v.statusTone = Tone::Neutral; v.statusText = finishedReadout(s); }
        else          { v.statusTone = Tone::Playing; v.statusText = playingReadout(s); }
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

    // ---- notice line: a refusal/notify for kNoticeSeconds AND only while the recorder is still in the
    // situation it was raised in (s-rta-0925: "No take is loaded" must not outlive the Load or the Record that
    // answered it; "Saved: x" must not survive into the next take); otherwise, while a take replays with its
    // audio, say what the relabelled Record button will do BEFORE it is pressed (fix plan F5).
    v.noticeLive = in.notice.isNotEmpty() && in.noticeAtSeconds >= 0.0
                && in.nowSeconds - in.noticeAtSeconds <= kNoticeSeconds
                && noticeKeyOf(s) == in.noticeKey;
    if (v.noticeLive)
        v.noticeText = in.notice;
    else if (withAudio && !recording)
        v.noticeText = "Record Over starts a new take on top of this audio; the loaded take is kept.";

    return v;
}
