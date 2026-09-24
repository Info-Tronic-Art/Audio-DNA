// test_record_panel_model -- s-rta-0924b step 4 (Lane S4-M): the Record panel's click-through
// matrix (plan-step4-record-panel.md 4.3), one TEST_CASE per row, plus formatClock, notice expiry,
// the forced-off "Play with audio", the overdub flag and the warning-line precedence. Pure: the
// model derives everything from RecorderHost::Status, so no GUI, no host, no audio device.
#include <catch2/catch_test_macros.hpp>
#include "ui/RecordPanelModel.h"
#include <cmath>
#include <limits>

namespace
{
    const juce::String kDot = juce::String::fromUTF8(" \xc2\xb7 ");     // " · "
    const juce::String kDash = juce::String::fromUTF8(" \xe2\x80\x94 "); // " — "

    const std::string kLastTake = "/Users/test/Documents/Audio-DNA/Takes/last.adna-take";
    const std::string kLoaded   = "/Users/test/Documents/Audio-DNA/Takes/show.adna-take";

    RecordPanelInputs inputs(double now = 100.0)
    {
        RecordPanelInputs in;
        in.nowSeconds = now;
        return in;
    }

    RecorderHost::Status loadedStatus(const std::string& audioStatus)
    {
        RecorderHost::Status s;
        s.loadedTakeFolder = kLoaded;
        s.loadedRecordedAt = "2026-09-24T10:00:00.000Z";
        s.loadedDuration = 83.4;
        s.loadedLanes = 3;
        s.audioStatus = audioStatus;
        if (audioStatus == "Resolved" || audioStatus == "ResolvedUnverified")
            s.loadedAssetId = "asset-1";
        return s;
    }

    RecorderHost::Status recordingStatus(bool audio, uint64_t framesWritten, double t)
    {
        RecorderHost::Status s;
        s.recording = true;
        s.takeFolder = "/Users/test/Documents/Audio-DNA/Takes/new.adna-take";
        s.assetId = audio ? "asset-new" : "";
        s.audioMode = "input";
        s.framesWritten = framesWritten;
        s.t = t;
        s.lanes = 2;
        s.points = 3;
        s.gestures = 1;
        return s;
    }

    void playing(RecorderHost::Status& s, bool withAudio)
    {
        s.playing = true;
        s.playMode = withAudio ? "withAudio" : "wallClock";
        s.positionSeconds = 12.3;
        s.lengthSeconds = 45.6;
        s.unresolved = 2;
    }

    // Reveal is the only button whose enabled state is "revealFolder non-empty" -- check both agree.
    void checkReveal(const RecordPanelView& v, const std::string& folder)
    {
        CHECK(v.revealFolder == juce::String(folder));
        CHECK(v.reveal.enabled == !folder.empty());
    }
}

// ---- the matrix ----

TEST_CASE("RecordPanelModel row 1 -- idle, nothing loaded, nothing recorded", "[recordpanel][model]")
{
    const auto v = deriveRecordPanelView(RecorderHost::Status{}, inputs());
    CHECK(v.record.text == "Record Take");
    CHECK(v.record.enabled);
    CHECK(v.record.tone == RecordPanelView::Tone::Neutral);
    CHECK(v.play.text == "Play Take");
    CHECK_FALSE(v.play.enabled);
    CHECK(v.play.tooltip == "Load a take first.");
    CHECK(v.load.text == "Load Take...");
    CHECK(v.load.enabled);
    CHECK(v.reveal.text == "Show in Finder");
    checkReveal(v, "");
    CHECK(v.repair.text == "Repair Audio");
    CHECK_FALSE(v.repair.enabled);
    CHECK(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.playWithAudioValue);
    CHECK(v.nameEnabled);
    CHECK_FALSE(v.recordSendsOverdub);
    CHECK(v.statusText == "Ready. No take loaded.");
    CHECK(v.statusTone == RecordPanelView::Tone::Neutral);
    CHECK(v.warningText.isEmpty());
    CHECK(v.noticeText.isEmpty());
}

TEST_CASE("RecordPanelModel row 2 -- idle, last take recorded, nothing loaded", "[recordpanel][model]")
{
    RecorderHost::Status s;
    s.takeFolder = kLastTake;
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.enabled);
    CHECK_FALSE(v.play.enabled);
    CHECK(v.load.enabled);
    checkReveal(v, kLastTake);
    CHECK_FALSE(v.repair.enabled);
    CHECK(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK(v.nameEnabled);
    CHECK(v.statusText == "Ready. Last take: last");
}

TEST_CASE("RecordPanelModel row 3 -- idle, loaded, audio ready", "[recordpanel][model]")
{
    auto s = loadedStatus("Resolved");
    s.takeFolder = kLastTake;   // loaded wins over the last recorded take
    auto in = inputs();
    SECTION("user value on")
    {
        const auto v = deriveRecordPanelView(s, in);
        CHECK(v.record.text == "Record Take");
        CHECK(v.record.enabled);
        CHECK(v.play.text == "Play Take");
        CHECK(v.play.enabled);
        CHECK(v.play.tooltip == "Replays the loaded take.");
        CHECK(v.load.enabled);
        checkReveal(v, kLoaded);
        CHECK_FALSE(v.repair.enabled);
        CHECK(v.recordAudioEnabled);
        CHECK(v.playWithAudioEnabled);
        CHECK(v.playWithAudioValue);
        CHECK(v.nameEnabled);
        CHECK(v.statusText == "Loaded: show" + kDash + "1:23" + kDot + "3 lanes" + kDot + "Audio: ready");
        CHECK(v.statusTone == RecordPanelView::Tone::Neutral);
    }
    SECTION("user value off is respected")
    {
        in.playWithAudio = false;
        const auto v = deriveRecordPanelView(s, in);
        CHECK(v.playWithAudioEnabled);
        CHECK_FALSE(v.playWithAudioValue);
    }
    SECTION("unverified audio is ready too")
    {
        const auto v = deriveRecordPanelView(loadedStatus("ResolvedUnverified"), in);
        CHECK(v.playWithAudioEnabled);
        CHECK(v.statusText.endsWith("Audio: ready (unverified)"));
    }
}

TEST_CASE("RecordPanelModel row 4 -- idle, loaded, audio incomplete", "[recordpanel][model]")
{
    const auto v = deriveRecordPanelView(loadedStatus("Incomplete"), inputs());
    CHECK(v.record.enabled);
    CHECK(v.play.enabled);            // wall clock only
    CHECK(v.load.enabled);
    checkReveal(v, kLoaded);
    CHECK(v.repair.enabled);
    CHECK(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.playWithAudioValue); // forced off although the user value is on
    CHECK(v.nameEnabled);
    CHECK(v.statusText.endsWith("Audio: incomplete, repair available"));
}

TEST_CASE("RecordPanelModel row 5 -- idle, loaded, audio not usable", "[recordpanel][model]")
{
    struct Row { const char* status; const char* word; };
    const Row rows[] = {
        { "Missing", "missing" }, { "Mismatch", "does not match" }, { "Legacy", "old format, record again" },
        { "MultiSegment", "unsupported" }, { "NoAudio", "none" },
    };
    for (const auto& r : rows)
    {
        INFO(r.status);
        const auto v = deriveRecordPanelView(loadedStatus(r.status), inputs());
        CHECK(v.record.enabled);
        CHECK(v.play.enabled);         // wall clock
        CHECK(v.load.enabled);
        checkReveal(v, kLoaded);
        CHECK_FALSE(v.repair.enabled);
        CHECK(v.recordAudioEnabled);
        CHECK_FALSE(v.playWithAudioEnabled);
        CHECK_FALSE(v.playWithAudioValue);
        CHECK(v.nameEnabled);
        CHECK(v.statusText.endsWith(juce::String("Audio: ") + r.word));
    }
}

TEST_CASE("RecordPanelModel row 6 -- armed (audio requested, no frames yet)", "[recordpanel][model]")
{
    const auto s = recordingStatus(true, 0, 0.5);
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Stop Recording");
    CHECK(v.record.enabled);
    CHECK(v.record.tone == RecordPanelView::Tone::Recording);
    CHECK(v.play.text == "Play Take");
    CHECK_FALSE(v.play.enabled);
    CHECK(v.play.tooltip == "Stop the recording first.");
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, s.takeFolder);      // the provisional folder exists
    CHECK_FALSE(v.repair.enabled);
    CHECK_FALSE(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.nameEnabled);
    CHECK(v.statusText == "Armed, waiting for audio...");
    CHECK(v.statusTone == RecordPanelView::Tone::Recording);
    CHECK(v.warningText.isEmpty());   // not yet past kArmedWarnSeconds

    const auto late = deriveRecordPanelView(recordingStatus(true, 0, 2.5), inputs());
    CHECK(late.warningText == "No audio is arriving. Check the input source.");
}

TEST_CASE("RecordPanelModel row 7 -- recording (plain)", "[recordpanel][model]")
{
    SECTION("with audio")
    {
        const auto s = recordingStatus(true, 4800, 4.2);
        const auto v = deriveRecordPanelView(s, inputs());
        CHECK(v.record.text == "Stop Recording");
        CHECK(v.record.enabled);
        CHECK(v.record.tone == RecordPanelView::Tone::Recording);
        CHECK(v.record.tooltip == "Stops this take. It is saved automatically.");
        CHECK_FALSE(v.play.enabled);
        CHECK_FALSE(v.load.enabled);
        checkReveal(v, s.takeFolder);
        CHECK_FALSE(v.repair.enabled);
        CHECK_FALSE(v.recordAudioEnabled);
        CHECK_FALSE(v.playWithAudioEnabled);
        CHECK_FALSE(v.nameEnabled);
        CHECK(v.statusText == "Recording 0:04 from live input" + kDot + "2 lanes" + kDot + "4 moves");
        CHECK(v.statusTone == RecordPanelView::Tone::Recording);
    }
    SECTION("audio gaps and file mode are named")
    {
        auto s = recordingStatus(true, 4800, 4.2);
        s.audioMode = "file";
        s.gaps = 1;
        const auto v = deriveRecordPanelView(s, inputs());
        CHECK(v.statusText == "Recording 0:04 from audio file" + kDot + "2 lanes" + kDot + "4 moves" + kDot + "1 audio gap");
    }
    SECTION("without audio, never 'Armed'")
    {
        const auto s = recordingStatus(false, 0, 4.2);
        const auto v = deriveRecordPanelView(s, inputs());
        CHECK(v.statusText == "Recording 0:04 without audio" + kDot + "2 lanes" + kDot + "4 moves");
        CHECK(v.warningText.isEmpty());
    }
}

TEST_CASE("RecordPanelModel row 8 -- playing, wall clock", "[recordpanel][model]")
{
    auto s = loadedStatus("Missing");
    playing(s, false);
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Record Take");
    CHECK(v.record.enabled);
    CHECK_FALSE(v.recordSendsOverdub);
    CHECK(v.play.text == "Stop Playback");
    CHECK(v.play.enabled);
    CHECK(v.play.tone == RecordPanelView::Tone::Playing);
    CHECK(v.play.tooltip == "Stops the replay.");
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, kLoaded);
    CHECK_FALSE(v.repair.enabled);
    CHECK(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);  // locked while playing
    CHECK_FALSE(v.playWithAudioValue);    // shows the actual mode
    CHECK(v.nameEnabled);
    CHECK(v.statusText == "Playing 0:12 / 0:45" + kDot + "unresolved: 2");
    CHECK(v.statusTone == RecordPanelView::Tone::Playing);
    CHECK(v.warningText.isEmpty());

    SECTION("moved and skipped counts are appended when non-zero")
    {
        s.reboundByPosition = 1;
        s.reboundByName = 2;
        s.skipped = 4;
        const auto v2 = deriveRecordPanelView(s, inputs());
        CHECK(v2.statusText == "Playing 0:12 / 0:45" + kDot + "unresolved: 2" + kDot + "moved: 3" + kDot + "skipped: 4");
    }
}

TEST_CASE("RecordPanelModel row 9 -- playing with audio", "[recordpanel][model]")
{
    auto s = loadedStatus("Resolved");
    playing(s, true);
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Record Over");
    CHECK(v.record.enabled);
    CHECK(v.recordSendsOverdub);
    CHECK(v.record.tooltip == "Records a new take over this take's audio, from the current playback position.");
    CHECK(v.play.text == "Stop Playback");
    CHECK(v.play.enabled);
    CHECK(v.play.tone == RecordPanelView::Tone::Playing);
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, kLoaded);
    CHECK_FALSE(v.repair.enabled);
    CHECK_FALSE(v.recordAudioEnabled);
    CHECK(v.recordAudioTooltip == "Recording over a take always uses that take's audio.");
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK(v.playWithAudioValue);          // shows the actual mode
    CHECK(v.nameEnabled);
    CHECK(v.statusTone == RecordPanelView::Tone::Playing);
    CHECK(v.warningText == "Replaying with the take's audio. Live input is paused until Stop Playback.");
}

TEST_CASE("RecordPanelModel row 10 -- overdub recording while playing", "[recordpanel][model]")
{
    auto s = loadedStatus("Resolved");
    playing(s, true);
    s.recording = true;
    s.overdub = true;
    s.assetId = "asset-1";   // the loaded take's asset
    s.takeFolder = "/Users/test/Documents/Audio-DNA/Takes/over.adna-take";
    s.t = 5.0;
    s.lanes = 1;
    s.points = 2;
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Stop Recording");
    CHECK(v.record.enabled);
    CHECK(v.record.tone == RecordPanelView::Tone::Recording);
    CHECK_FALSE(v.recordSendsOverdub);
    CHECK(v.play.text == "Stop Playback");
    CHECK_FALSE(v.play.enabled);
    CHECK(v.play.tooltip == "Stop the recording first.");
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, s.takeFolder);
    CHECK_FALSE(v.repair.enabled);
    CHECK_FALSE(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.nameEnabled);
    CHECK(v.statusText == "Recording over show 0:05" + kDot + "1 lane" + kDot + "2 moves"
                          + kDash + "Playing 0:12 / 0:45" + kDot + "unresolved: 2");
    CHECK(v.statusTone == RecordPanelView::Tone::Recording);
}

TEST_CASE("RecordPanelModel row 11 -- overdub recording, no playback (REST-only)", "[recordpanel][model]")
{
    RecorderHost::Status s;
    s.recording = true;
    s.overdub = true;
    s.assetId = "asset-other";
    s.takeFolder = "/Users/test/Documents/Audio-DNA/Takes/over.adna-take";
    s.t = 5.0;
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Stop Recording");
    CHECK(v.record.enabled);
    CHECK(v.play.text == "Play Take");
    CHECK_FALSE(v.play.enabled);
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, s.takeFolder);
    CHECK_FALSE(v.repair.enabled);
    CHECK_FALSE(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.nameEnabled);
    CHECK(v.statusText == "Recording over stored audio 0:05" + kDot + "0 lanes" + kDot + "0 moves");
    CHECK(v.statusTone == RecordPanelView::Tone::Recording);
    CHECK(v.warningText.isEmpty());   // an overdub never waits for tap frames
}

TEST_CASE("RecordPanelModel row 12 -- plain recording while playing (REST-only)", "[recordpanel][model]")
{
    auto s = loadedStatus("Missing");
    playing(s, false);
    s.recording = true;
    s.assetId = "asset-new";
    s.framesWritten = 480;
    s.takeFolder = "/Users/test/Documents/Audio-DNA/Takes/new.adna-take";
    s.audioMode = "input";
    s.t = 3.0;
    s.lanes = 1;
    s.points = 1;
    const auto v = deriveRecordPanelView(s, inputs());
    CHECK(v.record.text == "Stop Recording");
    CHECK(v.record.enabled);
    CHECK(v.play.text == "Stop Playback");
    CHECK(v.play.enabled);
    CHECK_FALSE(v.load.enabled);
    checkReveal(v, s.takeFolder);
    CHECK_FALSE(v.repair.enabled);
    CHECK_FALSE(v.recordAudioEnabled);
    CHECK_FALSE(v.playWithAudioEnabled);
    CHECK_FALSE(v.nameEnabled);
    CHECK(v.statusText == "Recording 0:03 from live input" + kDot + "1 lane" + kDot + "1 move"
                          + kDash + "playing 0:12");
    CHECK(v.statusTone == RecordPanelView::Tone::Recording);
}

// ---- the rest of the contract ----

TEST_CASE("RecordPanelModel formatClock", "[recordpanel][model]")
{
    CHECK(formatClock(0.0) == "0:00");
    CHECK(formatClock(83.4) == "1:23");
    CHECK(formatClock(59.99) == "0:59");
    CHECK(formatClock(3661.0) == "1:01:01");
    CHECK(formatClock(-3.0) == "0:00");
    CHECK(formatClock(std::numeric_limits<double>::quiet_NaN()) == "0:00");
    CHECK(formatClock(std::numeric_limits<double>::infinity()) == "0:00");
}

TEST_CASE("RecordPanelModel notice -- shown for kNoticeSeconds, then expires", "[recordpanel][model]")
{
    auto in = inputs(100.0);
    in.notice = "perf/play failed: no take loaded";
    in.noticeAtSeconds = 95.0;
    CHECK(deriveRecordPanelView(RecorderHost::Status{}, in).noticeText == in.notice);
    in.nowSeconds = 95.0 + kNoticeSeconds;
    CHECK(deriveRecordPanelView(RecorderHost::Status{}, in).noticeText == in.notice);
    in.nowSeconds = 95.0 + kNoticeSeconds + 0.5;
    CHECK(deriveRecordPanelView(RecorderHost::Status{}, in).noticeText.isEmpty());
    in.noticeAtSeconds = -1.0;   // never set
    in.nowSeconds = 95.0;
    CHECK(deriveRecordPanelView(RecorderHost::Status{}, in).noticeText.isEmpty());
}

TEST_CASE("RecordPanelModel play with audio is forced off whenever the audio is not ready", "[recordpanel][model]")
{
    auto in = inputs();
    in.playWithAudio = true;
    CHECK_FALSE(deriveRecordPanelView(RecorderHost::Status{}, in).playWithAudioValue);        // nothing loaded
    CHECK_FALSE(deriveRecordPanelView(loadedStatus("Incomplete"), in).playWithAudioValue);
    CHECK_FALSE(deriveRecordPanelView(loadedStatus("Missing"), in).playWithAudioValue);
    CHECK(deriveRecordPanelView(loadedStatus("Resolved"), in).playWithAudioValue);
}

TEST_CASE("RecordPanelModel recordSendsOverdub only while playing with audio and not recording", "[recordpanel][model]")
{
    int overdubRows = 0;
    auto check = [&](const RecorderHost::Status& s) { if (deriveRecordPanelView(s, inputs()).recordSendsOverdub) ++overdubRows; };

    check(RecorderHost::Status{});
    check(loadedStatus("Resolved"));
    check(loadedStatus("Incomplete"));
    check(recordingStatus(true, 0, 0.0));
    check(recordingStatus(true, 10, 1.0));
    auto wall = loadedStatus("Resolved"); playing(wall, false); check(wall);
    auto over = loadedStatus("Resolved"); playing(over, true); over.recording = true; over.overdub = true; check(over);
    CHECK(overdubRows == 0);

    auto withAudio = loadedStatus("Resolved"); playing(withAudio, true);
    CHECK(deriveRecordPanelView(withAudio, inputs()).recordSendsOverdub);
}

TEST_CASE("RecordPanelModel warning precedence -- lastError beats every other warning", "[recordpanel][model]")
{
    auto s = recordingStatus(true, 0, 5.0);   // armed and late -> would warn "No audio is arriving"
    s.humanRefused = 3;
    s.lastError = "audio tap self-stopped mid-take (device rate/channel change?) -- sample stamps after this point are unreliable";
    CHECK(deriveRecordPanelView(s, inputs()).warningText == juce::String(s.lastError));

    s.lastError.clear();
    CHECK(deriveRecordPanelView(s, inputs()).warningText == "No audio is arriving. Check the input source.");

    s.framesWritten = 100;   // no longer armed
    CHECK(deriveRecordPanelView(s, inputs()).warningText == "3 moves were refused because a control was already held.");

    // Replay-with-audio note beats the replay's own continuous count; lastError beats it while recording.
    auto p = loadedStatus("Resolved");
    playing(p, true);
    p.continuousUnavailable = 2;
    CHECK(deriveRecordPanelView(p, inputs()).warningText
          == "Replaying with the take's audio. Live input is paused until Stop Playback.");
    playing(p, false);
    CHECK(deriveRecordPanelView(p, inputs()).warningText == "2 knob moves could not be replayed.");

    // A stale lastError from the previous take is not shown once recording has stopped.
    RecorderHost::Status idle;
    idle.lastError = "device sample rate changed mid-take: 48000 -> 44100 Hz";
    CHECK(deriveRecordPanelView(idle, inputs()).warningText.isEmpty());
}
