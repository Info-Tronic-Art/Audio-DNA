# s-rta-0924b Step 4 -- Record panel: ONE consolidated fix plan (Architect)

Author: Architect (Fable), 2026-09-25. Inputs: the five critic seats (`critic-*.md` in this folder), the 10
live crops + `sheet.png`, the step-4 plan (`../plan-step4-record-panel.md` sections 4.3, 5), and the sources
read this session: `src/ui/RecordPanel.{h,cpp}`, `src/ui/RecordPanelModel.h`, `src/recording/RecorderHost.{h,cpp}`,
`src/recording/RecorderClock.{h,cpp}`, `src/recording/Program.cpp`, `src/MainComponent.cpp` (perf* funnel,
notify fan-out, tick), `tests/test_recorder_host.cpp`, `tests/test_record_panel_model.cpp`,
`.harmony/probe-step3.sh`, `/tmp/p3.log` (the last probe run's output). Labels: VERIFIED = read on disk at the
cited line this session; INFERRED = derived from cited code; ASSUMED = stated so the builder checks it.
Read-only pass: no source edited, app not launched.

---

## VERDICT

**The panel's shape holds; two DATA defects under it must be fixed at the source, and one of them is worse
than any seat reported.** The model/view/funnel split, the 12-row matrix, the two flipping buttons, the
tones and the uniform dimming are all confirmed in source and on screen (every seat's PASS-WITH-FIXES rests
on that). But:

1. The recording clock never restarts per take (VERIFIED, F1). Besides "Recording 0:15" one second after
   arming, it inflates the SAVED `take.meta.duration` (the "Loaded: s4gate -- 0:26" line is the app's uptime
   at Stop, the take was ~11 s), and -- not reported by any seat -- **delays every wall-clock replay by the
   app's uptime at arm**: `/tmp/p3.log` from the 63/0 probe run shows the same take replaying with audio at
   t~2/5/8/11 s and on the wall clock at t~14/17/20 s, a constant 12 s offset the probe's 45 s poll budget
   swallowed (VERIFIED, section F1).
2. The Playing readout's length comes from the last compiled control event, so an audio-only take reads
   "/ 0:00" (VERIFIED, F2).

Fix now: F1-F4 (MUST) and F5-F8 (SHOULD, each a few lines, all inside the step-4 files). Six items deferred
with a line each. Seven product calls listed for Boris with a recommended default; none blocks. Nothing is
patched in `RecordPanelModel.h` to hide either data defect: the model is a pure function of `Status`
(`RecordPanelModel.h:7-13`) and cannot know the arm time or the audio length -- the truth is fixed where it
is minted, and the model then prints it unchanged.

---

## 0. SEAT RECONCILIATION (dedupe, conflicts, dispositions)

| # | Finding | Seats (severity) | Verified? | Disposition |
|---|---|---|---|---|
| 1 | Recording clock is app-relative, not take-relative | interaction-logic MUST, logic-source MUST; Harmony obs. (2) | VERIFIED + a worse consequence found (wall-clock replay delay) | **F1 MUST** |
| 2 | Playing length is 0:00 for a take without control events | interaction-logic MUST, logic-source MUST, ux MUST; Harmony obs. (1) | VERIFIED | **F2 MUST**. Conflict resolved: fix in `RecorderHost::publishStatus()` (interaction-logic/ux), NOT in `Program::compile` (logic-source's first option) -- reasons in F2 |
| 3 | Notices show REST jargon ("perf/play failed: ...") | ux MUST; Harmony obs. (3) | VERIFIED; plus the plan's "Saved: <take>" notice (plan 4.5 B1 #6) was never implemented -- `perfStop()` returns silently on success (`MainComponent.cpp:5127-5138`) | **F3 MUST** |
| 4 | Name placeholder truncated "Name (blank = date and t..." | visual-design MUST, ux SHOULD, graphic SHOULD; Harmony obs. (4) | VERIFIED (geometry in F4) | **F4 MUST** (CLAUDE.md "always display whole words") |
| 5 | "Record Over" has no on-screen explanation before pressing | ux MUST | VERIFIED (tooltip only, `RecordPanelModel.h:123-124`) | **F5 SHOULD-now** (2 lines + test); wording = Boris B1 |
| 6 | Overdub "double-stop trap": both Stop buttons enabled, asymmetric | interaction-logic MUST | **Premise FALSE**: row 10's Stop Playback is DISABLED (`RecordPanelModel.h:134-135` `canStop = !(recording && overdub)`; `crop-overdub.png` shows it dimmed; row-10 test pins it, `test_record_panel_model.cpp:326-327`). The residual is true: Stop Recording during an overdub leaves the replay running (`perfStop` -> `disarm` never touches `playing_`, `RecorderHost.cpp:258-352`) and the tooltip does not say so | **F6 SHOULD-now** (tooltip disclosure); behaviour itself = Boris B6 |
| 7 | "Armed, waiting for audio..." hides a concurrent playback (row 12 during the 0-frame window) | interaction-logic SHOULD | VERIFIED (`RecordPanelModel.h:177-178` vs `:195-196`) | **F7 SHOULD-now** (1 line + test) |
| 8 | Stop Recording text fails AA contrast (2.9:1) | graphic SHOULD | VERIFIED by computation (F8) | **F8 SHOULD-now** (1 token) |
| 9 | Dimmed buttons give no on-screen reason | ux SHOULD | true, by design | **D1 deferred** |
| 10 | Browser tab "Comp/Decks" clipped to "Comp/Deck" | visual-design SHOULD | VERIFIED (`BrowserPanel.cpp:74` `tabWidth = width / 6`) | **D2 deferred** -- outside the panel; separate micro-lane |
| 11 | Error notices styled like routine notices | graphic NICE | true | **D3 deferred** |
| 12 | Row A/B button widths differ | graphic NICE | true | **D4 deferred** |
| 13 | Funnel + notify fan-out verified clean | logic-source NICE | VERIFIED (`MainComponent.cpp:2012-2018`, `:5074-5225`) | no action; it means F1/F2 land inside the host with no re-entrancy |
| 14 | Colour tokens / uniform dimming verified | visual-design NICE | VERIFIED | no action |
| 15 | Ruling-1 check passed live (stop_play during overdub stops the recording, selector back to Mic) | Harmony obs. (5) | consistent with `RecorderHost::stopPlayback` `:665-680` and `perfStopPlay` `:5180-5209` | no action |

---

## 1. FIX NOW

### F1 -- The recording clock restarts at every arm (MUST)

**Files.** `src/recording/RecorderHost.cpp` (`arm()`, `tick()`); `tests/test_recorder_host.cpp` (+1 case);
`.harmony/probe-step3.sh` (+1 assertion -- Harmony's file, see "Gate"). No header change, no new API.

**Root cause (VERIFIED).** `RecorderHost` owns ONE `RecorderClock clock_` for its whole life
(`RecorderHost.h:249`). `RecorderClock::tick()` anchors `startWall_` and latches `haveTicked_` on the
FIRST call it ever receives (`RecorderClock.cpp:13-24`); afterwards `t = wallNow - startWall_` (`:27`).
`RecorderHost::tick()` calls `clock_.tick(...)` unconditionally at `RecorderHost.cpp:392`, BEFORE the
`if (recording_)` guard at `:395`, and `MainComponent::tickFeaturePipeline` calls `recorderHost_.tick`
every 120 Hz tick from app start (`MainComponent.cpp:3353-3361`). `arm()` (`RecorderHost.cpp:143-256`)
resets every other per-take field (`:160-171`) but never the clock, and `RecorderClock` exposes no reset.
So `t` is seconds since the app's first tick, not since Record. The clock's own contract says "One instance
per take being recorded" (`RecorderClock.h:20`) and "the FIRST tick() call establishes t = 0 (recording
start)" (`:37-39`) -- the host violates it.

**Consequences.**
- (a) VERIFIED on screen: `crop-armed-or-recording.png` "Recording 0:15" one second after the arm POST;
  `crop-overdub.png` "Recording over s4gate 0:33" -- the overdub's clock started at 33 s of app life.
- (b) VERIFIED in code: `take.meta.duration = clock_.now().t` at Stop (`RecorderHost.cpp:316`) and in every
  periodic save (`:464`) -- so `Loaded: s4gate -- 0:26` (`crop-loaded-audio-ready.png`) is app uptime at
  Stop for a take that ran from ~15 s to ~26 s (about 11 s of audio).
- (c) VERIFIED in the probe log: every discrete point's `s.t` is minted from this clock
  (`PerformanceRecorder.cpp:31,49,80`; markers `RecorderHost.cpp:564-568`); `Program::compile` uses `s.t`
  VERBATIM as the event time under `DriveClock::Wall` (`Program.cpp:162`); wall-clock replay runs
  `pos = wallNow - playStartWall_` from 0 (`RecorderHost.cpp:479`). Hence a take armed U seconds after
  launch replays every event U seconds late on the wall clock. `/tmp/p3.log` (last 63/0 run of
  `probe-step3.sh`): `replay(withAudio): t~2s/5s/8s/11s` vs `replay(wallClock): t~14s/17s/20s` -- same
  3 s spacing, +12 s offset (the sample-domain replay is immune because `sample` stamps are absolute
  delivered-sample counts and `pos = playFirstSample_ + transportFrames`, `:477`). The probe still said
  PASS because before Play `status().length` is 0 (`:738-741`) so `REPLAY_BUDGET` falls back to 45 s
  (`probe-step3.sh:602-603`) and only the ORDER of the sequence is checked (`:660-663`). For a performer
  ten minutes into a show, wall-clock replay would appear to do nothing.
- (d) INFERRED: `lastCheckpointT_ = 0.0` at arm (`:234`) with `now.t` = uptime makes the "periodic" save
  fire on the first tick after arm whenever the app has been up >= 60 s (`:453-454`) -- harmless, wasteful.
- (e) INFERRED: the model's armed warning `armed && s.t > kArmedWarnSeconds` (`RecordPanelModel.h:215`) can
  flash "No audio is arriving" on the very first tick after arm (t already > 2). Both (d) and (e) vanish
  with F1.

**Change (two hunks, `RecorderHost.cpp`).**
```cpp
// arm(): after `lastError_.clear();` (:171), before `recorder_.start(comp, clock_, takeFolder_);` (:232)
    // One clock per take (RecorderClock.h:20): the first tick after this arm is t = 0 and writes the
    // "start" anchor at that tick's sample. The recorder takes the clock BY REFERENCE at start(), so
    // re-creating it in place here keeps the same address.
    clock_ = RecorderClock{};

// tick(): move `clock_.tick(snap, wallNow, sampleForClock);` (:392) to be the FIRST statement inside
// `if (recording_)` (:395). Nothing reads the clock while idle (publishStatus guards on recording_ :712,
// marker/capture/onHuman* return early :509/:523/:531/:547, disarm requires recording_ :262), and an
// idle clock only grows its TempoMap with "bpm"/"periodic" anchors for hours (RecorderClock.cpp:75-86).
```
`RecorderClock` is default-constructible and assignable (plain members + `TempoMap` = a `std::vector`,
`TempoMap.h:27-30`) -- VERIFIED. `PerformanceRecorder::start` stores `&clock` (`PerformanceRecorder.h:28,75`,
`.cpp:18`) -- the object's address is unchanged by assignment -- VERIFIED. The 1-tick (<= 8 ms) window
between `arm()` and the first tick now stamps `{t 0, sample 0}` instead of a stale pre-arm reading; accepted
(a capture in that window was already wrong, differently).

**Fail-first test** (append to `tests/test_recorder_host.cpp`; idiom of the `[host][capture]` case
`:446-483` and the `[host][replay]` case `:548-596`; helpers `makeSnap/layerKey/makeComposition/
FakeDispatch/TempDir` exist there):
```cpp
TEST_CASE("RecorderHost clock -- t restarts at every arm; a wall-clock replay is not delayed by the app's uptime", "[host][clock]")
{
    TempDir storeRoot("clock_store");
    TempDir takeFolder("clock_take");
    AudioStore store(storeRoot.dir);
    RecorderHost host(store);
    Composition comp = makeComposition();
    FakeDispatch fake;
    fake.wire(host, &comp);
    AudioTap dummyTap;

    RecorderHost::ArmOptions opts;
    opts.takeFolder = takeFolder.dir;
    opts.audio = false;   // 5.5 -- no tap; the clock is the subject
    opts.appVersion = "test";

    // The app has been ticking for 100 s before the performer presses Record.
    for (int i = 0; i <= 100; ++i)
        host.tick(makeSnap(), static_cast<double>(i), 0, dummyTap, std::nullopt, 48000.0);

    REQUIRE(host.arm(comp, dummyTap, opts).ok);
    host.tick(makeSnap(), 100.0, 0, dummyTap, std::nullopt, 48000.0);   // first tick after arm
    CHECK(host.status().t == Approx(0.0));                              // RED today: 100.0

    host.tick(makeSnap(), 100.5, 0, dummyTap, std::nullopt, 48000.0);
    const ControlPath key = layerKey(0, "activeClip");
    DiscretePoint p; p.origin = Origin::Human; p.v = 1;
    host.capture(key, std::move(p));

    const auto stop = host.disarm(comp, dummyTap);
    REQUIRE(stop.ok);
    CHECK(stop.duration == Approx(0.5));                                // RED today: 100.5

    LoadStats stats;
    auto saved = Take::load(takeFolder.dir, stats);
    REQUIRE(saved.has_value());
    REQUIRE(saved->lanes.count(key) == 1);
    CHECK(saved->lanes.at(key).points[0].s.t == Approx(0.5));           // RED today: 100.5
    CHECK(saved->meta.duration == Approx(0.5));

    SECTION("a second take restarts at 0 too -- the first-tick latch must not survive a take")
    {
        TempDir second("clock_take2");
        opts.takeFolder = second.dir;
        host.tick(makeSnap(), 150.0, 0, dummyTap, std::nullopt, 48000.0);   // idle ticks between takes
        REQUIRE(host.arm(comp, dummyTap, opts).ok);
        host.tick(makeSnap(), 200.0, 0, dummyTap, std::nullopt, 48000.0);
        CHECK(host.status().t == Approx(0.0));                          // RED today: 200.0
        host.disarm(comp, dummyTap);
    }

    SECTION("wall-clock replay fires the point 0.5 s after Play, not 100.5 s after")
    {
        REQUIRE(host.load(takeFolder.dir).ok);
        const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        REQUIRE(host.play(RecorderHost::PlayMode::WallClock, comp).ok);
        host.tick(makeSnap(), w0 + 0.6, 0, dummyTap, std::nullopt, 48000.0);
        CHECK(fake.fired.size() == 1);                                   // RED today: 0 (the point sits at t = 100.5)
        host.stopPlay();
    }
}
```
Existing cases stay green: every one of them ticks at `0.0` right AFTER arm (`:221, :277, :333, :384, :464,
:645, :737` ...) so their t values are unchanged; the `[host][periodic]` case (`:193-235`, save at 65 s,
`meta.duration == 65`) is unchanged because its first post-arm tick is at 0.0.

**Probe pin (Harmony, `.harmony/probe-step3.sh`, after the wall-clock loop `:645-663`).** Record `ELAPSED`
at the first `activeClipColumn` change AFTER the pre-play leftover in both replay loops; require
`|wall - withAudio| <= 1.5` s. Today: 14 vs 2 -> RED; after F1 -> GREEN. Without this pin the 45 s budget
makes the regression invisible again.

**Data note.** Takes recorded before F1 keep their inflated `meta.duration` and their offset `t` stamps on
disk (`~/Documents/Audio-DNA/Takes/s4gate.adna-take`, `s4over.adna-take`, and every `step3gate*` take).
Delete the gate takes before the re-shoot (never the store's audio -- ruling 28 deletes nothing
automatically; orphan audio is harmless). Not a source change.

### F2 -- The Playing length is the take's real length (MUST)

**Files.** `src/recording/RecorderHost.cpp` `publishStatus()` (`:751-767`) + the field comment
`RecorderHost.h:224-226`; `tests/test_recorder_host.cpp` (+2 sections in the `[host][status][seconds]`
case `:1560-1605`).

**Root cause (VERIFIED).** `s.length = program_->length` (`RecorderHost.cpp:741`); `Program::compile` seeds
`maxAt = 0.0` and raises it only per discrete point / gesture end (`Program.cpp:187, 212, 248, 273`) --
never from `Take::Meta::duration` or the audio. `lengthSeconds` is a pure conversion of `length`
(`RecorderHost.cpp:760, 766`). The gate take had 0 lanes, so `length == 0` -> "Playing 0:02 / 0:00"
(`crop-playing-with-audio.png`), and `crop-overdub.png` "Playing 0:04 / 0:00". The correct facts already
exist: the asset's frame count/rate (`loadedAudio_.asset.frames/.rate`, `AudioStore.h:26-27`, published at
`:638-640`) and `loadedTake_->meta.duration` (`:776`).

**Why the host and not `Program::compile`** (logic-source's first option): `Program::length` has a defined
meaning -- the last compiled event in the drive-clock domain (plan G8) -- that `Status::length` publishes on
REST unchanged (D12 "ADD, never REDEFINE"); `Program.*` is outside every step-4 lane (plan 4.7 "NOT touched");
and for WithAudio the true end is the AUDIO's end (the transport stops there), which the compiler does not
know. The panel-facing `lengthSeconds` was added by this very step and is simply wrong -- fixing its rule
now, before Boris sees it, is not a redefinition.

**Rule.** WithAudio: `lengthSeconds = max(convert(length), asset.frames / asset.rate)` -- the replay ends
where the audio ends, never earlier than the last compiled event. WallClock: `lengthSeconds =
max(length, meta.duration)` -- the recorded duration (arm -> stop, correct after F1), never earlier than the
last event; a take saved only provisionally (crash, `duration 0`) still shows its last event.

**Change (`RecorderHost.cpp:754-767`).**
```cpp
        if (playMode_ == PlayMode::WithAudio)
        {
            if (playAssetRate_ > 0.0)
            {
                const double first = static_cast<double>(playFirstSample_);
                s.positionSeconds = std::max(0.0, s.position - first) / playAssetRate_;
                // The replay ends where the audio ends: the asset's length IS the length, never less than
                // the last compiled event (an audio-only take has no events, so `length` alone reads 0).
                s.lengthSeconds = std::max(std::max(0.0, s.length - first),
                                           static_cast<double>(loadedAudio_.asset.frames)) / playAssetRate_;
            }
        }
        else
        {
            s.positionSeconds = s.position;
            // Wall clock: the recorded duration (arm -> stop), never less than the last compiled event.
            s.lengthSeconds = std::max(s.length, loadedTake_ ? loadedTake_->meta.duration : 0.0);
        }
```
Header comment (`RecorderHost.h:224-226`): "positionSeconds: the playback position in seconds regardless of
DriveClock. lengthSeconds: the take's length in seconds -- with audio, the audio's length; otherwise the
recorded duration -- never less than the last compiled event. position/length stay in the drive-clock domain."

**Existing expectations unchanged.** WithAudio section (`:1572-1588`): `max(1.0, 10240/48000 = 0.213) = 1.0`
(the asset is `makeFinalizedAsset(store, 48000.0, 20)` = 20 x 512 frames, `:1451-1475`). WallClock section
(`:1590-1604`): `max(1.0, 0) = 1.0` (`makeTwoPointTake` leaves `meta.duration` 0, `:1477-1490`).

**Fail-first sections** (append inside the same `TEST_CASE`, `:1605`):
```cpp
    SECTION("WithAudio -- an audio-only take (no lanes) is as long as its audio")
    {
        TempDir folder("seconds_audio_only");
        Take take;
        take.audio = AudioStore::referencing(asset, 96000);   // 10 240 frames @ 48 kHz = 0.2133 s
        REQUIRE(take.save(folder.dir));
        REQUIRE(host.load(folder.dir).ok);
        REQUIRE(host.play(RecorderHost::PlayMode::WithAudio, comp).ok);
        host.tick(makeSnap(), 0.0, 0, dummyTap, std::optional<int64_t>(4800), 48000.0);
        const auto st = host.status();
        CHECK(st.positionSeconds == Approx(0.1));
        CHECK(st.lengthSeconds == Approx(10240.0 / 48000.0));   // RED today: 0.0 ("Playing 0:02 / 0:00")
        host.stopPlay();
    }

    SECTION("WallClock -- a take with no lanes is as long as its recorded duration")
    {
        TempDir folder("seconds_wall_only");
        Take take;
        take.meta.duration = 7.5;
        REQUIRE(take.save(folder.dir));
        REQUIRE(host.load(folder.dir).ok);
        const double w0 = juce::Time::getMillisecondCounterHiRes() / 1000.0;
        REQUIRE(host.play(RecorderHost::PlayMode::WallClock, comp).ok);
        host.tick(makeSnap(), w0 + 0.5, 0, dummyTap, std::nullopt, 48000.0);
        CHECK(host.status().lengthSeconds == Approx(7.5));      // RED today: 0.0
        host.stopPlay();
    }
```
ASSUMED (builder checks once): `Take::load` accepts a take with zero lanes and `play()` compiles it (`play()`
has no non-empty guard, `RecorderHost.cpp:600-650`). If `load` refuses an empty take, add one lane with a
single point at `sample 96000` / `t 0.0` -- `length` then converts to 0 and both sections still bite.

### F3 -- Notices in plain words, and a "Saved" notice at Stop (MUST)

**Files.** `src/MainComponent.cpp` `perfRecord/perfStop/perfLoad/perfPlay/perfStopPlay/perfRepair`
(`:5074-5225`); `src/recording/RecorderHost.cpp` three refusal strings (`:188, :202, :208`).

**Root cause (VERIFIED).** The funnel hard-codes "perf/..." strings and hands them unmodified to
`dispatch.notify` (`MainComponent.cpp:5078, 5117, 5132, 5145, 5159, 5192-5193, 5219`), which prints
`[Recorder] <msg>` to stderr and stores the same text as the panel's notice (`:2012-2018`;
`RecordPanel.cpp:136-140`, rendered at `:228`). Seen live in `crop-notice-no-take-loaded.png`,
`crop-after-stopplay-during-overdub.png`, `crop-idle-restored.png`. `perfStop()` notifies nothing on
success (`:5127-5138`) although plan 4.5 B1 #6 specified "Saved: <take>" -- `crop-idle-after-stop.png` shows
no notice at all. Safe to change: no ctest pins any of these strings (grep over `tests/*.cpp`:
`test_record_panel_model.cpp:407` uses "perf/play failed: no take loaded" only as an opaque input for the
expiry test); REST responses never carry them (the `onPerf*` lambdas discard the return, `:2039-2044`);
`probe-step3.sh` greps stderr only for the retired analysis warning (`:242`).

**Replacements (whole words, sentences; `<reason>` = the host's text).**

| Site | Today | New |
|---|---|---|
| `:5078` | `perf/record refused: already recording` | `A take is already recording.` |
| `:5117` | `perf/record failed: <reason>` | `Could not start the take: <reason>` |
| `:5132` | `perf/stop: <reason>` | `Nothing is recording.` when `<reason> == "not recording"`, else `Could not stop the take: <reason>` |
| `:5137` (new, success path) | (silent) | notify `Saved: <name>` where `<name> = result.takeFolder.getFileNameWithoutExtension()` (return stays `""`) |
| `:5145` | `perf/load failed: <reason>` | `Could not load the take: <reason>` |
| `:5159` | `perf/play failed: <reason>` | `No take is loaded. Use Load Take... first.` when `<reason> == "no take loaded"` (the only refusal the panel can reach -- Play is disabled otherwise), else `Could not play: <reason>` |
| `:5192` | `perf/stop_play: the recording over the take was stopped too (its clock is the replayed audio)` | `Stopped the playback and the take recorded over it. Saved: <name>` (`stopped.overdub.takeFolder`) |
| `:5193` | `perf/stop_play: stopping the recording over the take failed: <e>` | `Stopped the playback, but the take recorded over it could not be saved: <e>` |
| `:5219` | `perf/repair: <err>` | `Could not repair the audio: <err>` |
| `RecorderHost.cpp:188` | `overdub asset not found: <id>` | `the stored audio for this take was not found` |
| `RecorderHost.cpp:202` | `could not begin audio asset (store root unwritable?)` | `the audio folder could not be written (Documents/Audio-DNA/Audio)` |
| `RecorderHost.cpp:208` | `AudioTap failed to start (disk / free-space?)` | `audio recording could not start; the disk needs at least 2 GB free` |

Kept as they are (already plain and composed after a lead-in): `already recording`, `not recording`,
`no take loaded`, `already playing`, `audio not resolved: <reason>`, `Take::load`'s refusal reasons (spec 6:
verbatim). The stderr line keeps its `[Recorder] ` prefix, so engineers lose nothing.

**Verification.** No logic -> no unit test; `grep -n '"perf/' src/MainComponent.cpp` -> 0 hits; the
re-shoot's four notice states read the new sentences (Gate, below).

### F4 -- The name placeholder is shown whole (MUST)

**File.** `src/ui/RecordPanel.cpp` `resized()` (`:258-263`).

**Root cause (VERIFIED).** Placeholder "Name (blank = date and time)" (`:92`). Row C hands the editor what
remains after two fixed toggles: panel ~428 px logical (the crops are 864 px wide at 2x and the tab bar spans
them) - 8 (`reduced(4)`) - 130 - 110 - 4 = 176 px (`:259-263`); the visible fragment "Name (blank = date and
t" (22 chars) spans ~165 px in the crops, so the 28-char string needs ~210 px. Every one of the 10 crops
shows the ellipsis.

**Change.** Row C = the name alone (left, width `min(rowC.getWidth(), 260)`), then a new Row C2 = the two
switches (`recordAudioToggle_` 130 px, 6 px gap, `playWithAudioToggle_` 150 px, left-aligned):
```cpp
    // Row C: the take name -- its placeholder is a whole sentence, so it gets the width.
    auto rowC = area.removeFromTop(kControlHeight);
    nameEditor_.setBounds(rowC.removeFromLeft(std::min(rowC.getWidth(), 260)).reduced(1, 2));
    area.removeFromTop(kRowSpacing);
    // Row C2: the two switches.
    auto rowC2 = area.removeFromTop(kControlHeight);
    recordAudioToggle_.setBounds(rowC2.removeFromLeft(130));
    rowC2.removeFromLeft(6);
    playWithAudioToggle_.setBounds(rowC2.removeFromLeft(150));
```
Height is free: the panel gets the whole browser column (`BrowserPanel.cpp:83` `recordPanel_.setBounds(area)`)
and lays out top-down, the crops show empty space below Row D. Component ids/AX titles unchanged; no model
or test change. Verify on the re-shoot: the full placeholder in every state.

### F5 -- "Record Over" is explained before it is pressed (SHOULD-now)

**Files.** `src/ui/RecordPanelModel.h` notice block (`:224-226`); `tests/test_record_panel_model.cpp`
row 9 (`:285-307`, +2 checks) and row 10 (`:309-337`, +1).

**Why here and not the warning line.** The cyan notice line is the panel's informational channel and is
empty in row 9 unless a refusal just happened; the yellow line already carries "Replaying with the take's
audio..." and a third sentence would crowd 420 px at 10 pt (the current 76-char warning spans ~300 px in
the crops, so ~105 chars fit; a 75-char hint fits alone, not appended).

**Change.**
```cpp
    // ---- notice line: a refusal/notify for kNoticeSeconds; otherwise, while a take replays with its
    // audio, say what the relabelled Record button will do BEFORE it is pressed (ux critic, step 4).
    if (in.notice.isNotEmpty() && in.noticeAtSeconds >= 0.0 && in.nowSeconds - in.noticeAtSeconds <= kNoticeSeconds)
        v.noticeText = in.notice;
    else if (withAudio && !recording)
        v.noticeText = "Record Over starts a new take on top of this audio; the loaded take is kept.";
```
**Test (RED first: `noticeText` is empty today).** Row 9: `CHECK(v.noticeText == "Record Over starts a new
take on top of this audio; the loaded take is kept.");` and, with `in.notice = "x"; in.noticeAtSeconds =
100.0;`, `CHECK(... .noticeText == "x")` (a fresh notice wins). Row 10: `CHECK(v.noticeText.isEmpty());`
(already recording). The expiry case (`:404-417`) uses an idle `Status{}` -> unaffected. Wording = Boris B1.

### F6 -- Stop Recording during an overdub says the replay keeps playing (SHOULD-now)

**Files.** `src/ui/RecordPanelModel.h:119-120`; `tests/test_record_panel_model.cpp` row 10 (+1 check).

**Fact (VERIFIED).** Row 10 offers only Stop Recording (Stop Playback disabled, `:134-135`). Pressing it
runs `perfStop()` -> `disarm()`, which finalizes and saves the overdub and never touches `playing_`
(`RecorderHost.cpp:258-352`, `MainComponent.cpp:5127-5138`): the replay continues and the panel lands in
row 9 ("Record Over" available again -- the ruling-28 loop: redo, stop, redo). Coherent, not a dead end,
but undisclosed: the tooltip says only "Stops this take. It is saved automatically." (`:120`).

**Change.** `if (recording) v.record = { "Stop Recording", true, (overdub && playing) ? "Stops this take. It
is saved automatically. The replay keeps playing." : "Stops this take. It is saved automatically.",
Tone::Recording };` **Test.** Row 10: `CHECK(v.record.tooltip == "Stops this take. It is saved
automatically. The replay keeps playing.");` (RED first). Rows 7 and 11 keep the short text (row 11 is an
overdub with no replay). Whether Stop Recording should ALSO stop the replay is Boris B6 (default: no).

### F7 -- "Armed" also shows a concurrent playback (SHOULD-now)

**Files.** `src/ui/RecordPanelModel.h:177-178`; `tests/test_record_panel_model.cpp` row 6 (`:191-213`, +1 section).

**Change.** In the `if (armed)` branch, after `v.statusText = "Armed, waiting for audio...";` add
`if (playing) v.statusText << dash() << "playing " << formatClock(s.positionSeconds);` -- the mirror of
the plain-recording branch (`:195-196`). **Test.** New `SECTION("armed while a wall-clock replay runs
(REST-only row 12 during the 0-frame window)")`: `auto s = recordingStatus(true, 0, 1.0); playing(s, false);`
-> `statusText == "Armed, waiting for audio..." + kDash + "playing 0:12"`. RED first.

### F8 -- Stop Recording text passes AA contrast (SHOULD-now)

**File.** `src/ui/RecordPanel.cpp:178` -> `juce::Colours::black`.

**Numbers (WCAG 2.x relative luminance, computed).** Today: `kTextPrimary` #e0e0e0 on `kMeterRed` #ff1744
= 2.9:1 -- fails AA (4.5:1); the button font is 14 px regular (`LookAndFeel.cpp:83`), so the large-text
3:1 relief does not apply. `kBackground` #1a1a2e on red = 4.43:1 (just short). Black on red = 5.46:1
(passes). For reference the green button is `kBackground` on `kMeterGreen` #00e676 = 10.2:1 (fine; switching
it to black too, 11.6:1, would make one rule "dark text on a bright fill" -- optional). The red status label
on the panel's #1a1a1a is 4.5:1 -- passes, leave it. Colour tokens themselves are shared with the TopBar and
are NOT changed (Boris B4 if he prefers a darker red).

---

## 2. DEFERRED (one line each; none blocks the re-shoot)

- **D1** Dimmed buttons' reasons on screen (ux SHOULD): the status line already names the dominant reason in
  every idle state ("Ready. No take loaded." explains Play/Show in Finder/Repair), and a caption per button
  is clutter in a 428 px column -- rewrite-era UI (ruling 5).
- **D2** "Comp/Decks" tab clipped (visual-design SHOULD): `BrowserPanel.cpp:74` gives each of 6 tabs
  `width / 6` = 71 px; the 14 px label needs ~78 px. Outside the Record panel; separate micro-lane: size
  tabs by `Font(14).getStringWidth(text) + 16` and share the remainder, or Boris renames (B7).
- **D3** Error vs routine notice styling (graphic NICE): one channel today (`noticeLabel_`); needs the funnel
  to classify refusal vs information -- later, with D1.
- **D4** Row A 120/120 vs Row B 100/110/100 button widths (graphic NICE): cosmetic; fold into any later
  layout pass.
- **D5** (found here) `RecordPanel::runAction` re-applies `lastStatus_` from the last 4 Hz refresh after a
  click (`RecordPanel.cpp:155-166`), so a pressed button can show its old label for up to 250 ms; harmless,
  fix later by handing the panel a fresh `status()` after the funnel returns.
- **D6** (found here) Playback never ends by itself: wall-clock replay keeps counting past the length
  (`Player` has no end notion -- grep `length|finished` in `Player.{h,cpp}` = 0) and with-audio replay sits
  at the audio's end until Stop Playback. Boris B5.

---

## 3. BORIS'S CALLS (product/taste; recommended default in brackets -- nothing waits on these)

- **B1** The relabelled button: keep "Record Over" or call it "Overdub" (a real word musicians know)? And
  the hint sentence of F5. [Keep "Record Over"; hint as written in F5.]
- **B2** The "Saved" notice: take name or full folder path? [Name only -- "Saved: s4gate"; the folder is
  already on the "Takes: ~/Documents/Audio-DNA/Takes" caption and under Show in Finder.]
- **B3** Name field width after F4: full row or capped? [Capped at 260 px, left-aligned.]
- **B4** Stop Recording colours: black text on the shared red, or a darker red with the light text?
  [Black text; the red token stays shared with the TopBar.]
- **B5** Should a replay stop by itself at the end of the take (and the input come back)? [Not in this
  step; a later lane -- it changes when the live input returns during a set.]
- **B6** Stop Recording during an overdub: keep the replay running (so Record Over can be pressed again)
  or stop everything like Stop Playback does? [Keep it running; F6 says so in the tooltip.]
- **B7** The "Comp/Decks" tab: rename ("Decks") or resize the tabs (D2)? [Resize; no rename.]

---

## 4. LANES, ORDER, GATE

| Lane | Owns (nothing else) | Fixes | Fail-first anchor |
|---|---|---|---|
| **H** host | `src/recording/RecorderHost.{h,cpp}`, `tests/test_recorder_host.cpp` | F1, F2, F3 (3 host strings) | `[host][clock]` case; 2 `[host][status][seconds]` sections |
| **M** model | `src/ui/RecordPanelModel.h`, `tests/test_record_panel_model.cpp` | F5, F6, F7 | row 9/10/6 checks |
| **V** view | `src/ui/RecordPanel.cpp` | F4, F8 | re-shoot only |
| **B** app | `src/MainComponent.cpp` | F3 (funnel strings + "Saved") | re-shoot only; `grep '"perf/'` = 0 |

Disjoint files. Order: H first (its two tests are the gate's teeth), then M, V, B in parallel (M/V/B do not
depend on H's code). Each lane: own `-B build-<lane>`, run its ctest target, `git show --stat HEAD`.

**Gate (Harmony; screen-safety law and plan section 6 script unchanged).**
1. `cmake --build build-s4gate` clean; `ctest` -- expect +1 case (H), +2 sections (H), +4 checks (M), all green
   on the fixed tree and RED when any of F1/F2/F5/F6/F7 is reverted (fail-first evidence).
2. `bash .harmony/probe-step3.sh` from the main checkout: 63/0 preserved + the new wall-clock timing pin (F1)
   green; the log's `replay(wallClock): t~` lines must now match the `replay(withAudio)` lines within ~1 s.
3. Delete `~/Documents/Audio-DNA/Takes/s4gate.adna-take` and `s4over.adna-take` (pre-F1 durations).
4. Re-shoot the 10 states with the plan section 6 script. Pass criteria per crop:
   - armed-or-recording (1 s after arm): "Recording 0:01 from live input · 0 lanes · 0 moves" (not 0:15)
   - recording (+4 s): "Recording 0:05 ..."
   - idle-after-stop: "Ready. Last take: s4gate" + cyan "Saved: s4gate"
   - loaded-audio-ready: "Loaded: s4gate — 0:0x · 0 lanes · Audio: ready" with x = the seconds between arm and stop
   - playing-with-audio: "Playing 0:02 / 0:0x · unresolved: 0"; yellow replay line; cyan "Record Over starts a new take on top of this audio; the loaded take is kept."
   - overdub: "Recording over s4gate 0:0y · ... — Playing 0:04 / 0:0x · unresolved: 0" with y counting from 0 at the overdub's arm; Stop Playback dimmed; no cyan hint
   - after-stopplay-during-overdub: cyan "Stopped the playback and the take recorded over it. Saved: s4over"; Audio selector "Mic Input"
   - idle-restored: cyan "Nothing is recording."
   - notice-no-take-loaded: cyan "No take is loaded. Use Load Take... first."
   - every crop: "Name (blank = date and time)" whole on its own row, the two switches on the row below; Stop Recording label black on red; no "perf/" anywhere.
5. Interaction-Logic critic re-brief: rows 9/10/12 only (F5/F6/F7), plus the claim "row 10 has one enabled
   stop" as a stated fact to check, not to re-derive.

---

## 5. RISKS

- **R1 Gating `clock_.tick` on `recording_` (F1).** Nothing reads the clock while idle (cites in F1); if a
  future caller does, `now()` returns the previous take's last stamp -- acceptable, but the builder should
  keep the reset AND the gate together with the comment. Reset-only (no gate) is also correct; the gate only
  stops idle TempoMap growth.
- **R2 The arm-to-first-tick window (F1).** <= 8 ms at 120 Hz; a capture there stamps t 0 / sample 0. Not
  fixed here (would need a snapshot at arm, the same shape as the onset-baseline limitation already
  documented at `RecorderHost.h:308-320`).
- **R3 F2's `max()` rule.** A hand-built take whose events run past its audio reports the event end while
  the transport has stopped -- the S4-A meaning is kept for such takes; real takes cannot have it.
- **R4 Host refusal strings (F3).** No test or REST client pins them (VERIFIED by grep and by the discarded
  returns); the `[Recorder]` stderr prefix survives. If a hidden consumer exists, it is a script grepping
  stderr for "AudioTap failed" -- grep the repo once before landing (`grep -rn "AudioTap failed" .harmony/
  tests/ scripts/` -> expected 0).
- **R5 Layout (F4).** A different browser-column width in Boris's window can still clip; the 260 px cap and
  the switches on their own row leave ~160 px of slack at the crop's 428 px. Re-shoot confirms.
- **R6 The probe pin is Harmony's edit.** Without it, the wall-clock delay class stays invisible (45 s
  budget); the `[host][clock]` replay section covers it in ctest regardless.

**Strongest counterargument, and why it loses.** "Patch the two readouts in the model -- print
`s.t - tAtArm` and fall back to `loadedDuration` -- a smaller diff, no host risk." It loses three ways: the
model is a pure function of `Status` and has no arm time; the clock defect corrupts the SAVED take
(`meta.duration`, every `t` stamp) and delays every wall-clock replay by the app's uptime -- VERIFIED in
the probe's own log -- so a view-side patch would hide a data bug behind a nicer number; and the host fix is
two lines plus a test, smaller than the patch would be.

---

## 6. SUMMARY (for Harmony)

1. Two data defects, fixed at the source: the per-take clock (F1) and the Playing length (F2). F1 also fixes
   wall-clock replay, which today fires every event as late as the app was old when Record was pressed.
2. Notices become sentences and Stop says "Saved: <name>" (F3); the placeholder gets its own row (F4).
3. Four cheap model/view fixes (F5-F8) with tests where logic exists. Six deferrals, seven Boris defaults.
4. One seat's MUST (overdub double-stop) was built on a false premise; its residual is a tooltip (F6).
5. Gate: ctest (+1 case, +2 sections, +4 checks, fail-first), probe 63/0 + a wall-clock timing pin, delete
   the pre-fix gate takes, re-shoot the 10 states against the listed readouts.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0924b/step4-visual/fix-plan.md
STATUS: COMPLETE
