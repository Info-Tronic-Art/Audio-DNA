# FACTS: Quantize choices and BARS / BEATS wording (s-rta-1003)

READ-ONLY fact sheet. No design. Source tree = recon checkout of a7491d4 (lane/bf9b).
Paths below starting `src/`, `tests/`, `docs/`, `.harmony/` are relative to
`/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon` ("recon"). Paths starting `.harmony/.reports/`
and `.harmony/boris-feedback-backlog.md` / `binding-decisions.md` are in the main repo.
Labels: VERIFIED = read in recon (file:line). INFERRED = reasoned. UNKNOWN-NEEDS-A-RUN = needs a run (cheapest test given).

Boris's words used (verbatim, backlog "Boris feedback of 2026-10-03", lines 111-113 and 129):
- "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6"
- "Let's get rid of beats and just have bars in most places unless beats are necessary. For setting a clips beats, these are done with beats and not bars. For most other things, we will use bars. For the circle at the top that counts off 1234 and then starts over, those are beats as well."
- "I have beats and seconds in the milkdrop editor. I only need beats."
- (BF15, same message, line 97) "If it is BPM synced, regardless of the length, it is synced to the current playing BPM and it has bars that the user can set. We automatically set bars for a cliff, but the user can change the bars in it by amount."

Scope fact (VERIFIED): bf7 is NOT in a7491d4. `grep -rn "BeatLength|beatlen" src tests` = no hit; `core/` has no BeatLength.h;
TopBar still reads "Next Downbeat"; ClipInspector still reads "Beats/ Cycle". So everything below is the PRE-bf7 app, and the
bf7 plan/ruling are un-built paper.

---------------------------------------------------------------------------------------------------------------------
## 1 QUESTIONS ANSWERED

### Q1. Top-bar Quantize menu, the enum, where the value is read; per-clip / per-routine snap enum

A1a. Menu entries today: three. VERIFIED `src/ui/TopBar.cpp:178-180`: "Off"(id 1), "Next Beat"(2), "Next Downbeat"(3).
Default id 1 (:181). onChange (:182-188): `mode = QuantizeMode(getSelectedId() - 1)`; writes `composition_.quantizeMode` and calls
`onQuantizeChanged`. Combo is 100 px wide (`TopBar.cpp:603`), label "Quantize:" 55 px (:602). No tooltip on the combo.
Nothing else is wired to `onQuantizeChanged` (`grep onQuantizeChanged src` = TopBar.h:29, TopBar.cpp:186-187 only).

A1b. Enum: VERIFIED `src/model/Composition.h:144-145`: `enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat }`, field default Off.
Saved as an int `Composition.h:742`; loaded with an UNGUARDED `static_cast` `Composition.h:880`. Reset to Off `Composition.h:230`.
Also carried as an int in the take recorder: `src/recording/PerfState.h:89`, `PerfState.cpp:192,225`, `PerfStateCapture.cpp:53`,
`Program.cpp:294`; replayed by `MainComponent.cpp:2003-2006` (`composition_.quantizeMode = static_cast<QuantizeMode>(f.p.v)`).

A1c. Where the value is read: ONE translator. VERIFIED `src/MainComponent.cpp:36-43` `quantizeModeToForcedSnap(mode, snap)`:
Off -> Off; tracker not LOCKED -> Off (an honest immediate trigger, :23-28 comment); NextBeat -> `BeatSnapMode::Beat`; ANYTHING ELSE
(incl. NextDownbeat) -> `BeatSnapMode::Bar`. Four callers, all VERIFIED: clip trigger `MainComponent.cpp:4746`, column trigger
`:4975`, routine engine tick `:4176` (cast to `RoutineSnap`), routine fire `:6156`.
So the top Quantize is "force this granularity on ONE queued trigger"; it also OVERRIDES a routine's own quantize
(`RoutineEngine::effectiveSnap`, `src/recording/RoutineEngine.cpp:221-224`).

A1d. The combo does NOT follow the model. VERIFIED: `grep quantizeSelector_ src` = TopBar.h:62,119 and TopBar.cpp:177-187,603 only
(no `setSelectedId` after the constructor). A loaded composition, a take/routine preamble replay (`MainComponent.cpp:2003`) or a
REST load changes the model and leaves the combo showing the old value (Pitfall 41 class). There is no REST route that sets
quantize: `grep -in quantize src/api/ApiServer.cpp` hits only the ROUTINE `quantize` option (:2285 `quantizeKnown`, :2324-2329, :2414-2420);
the only no-UI way to set the global Quantize is `POST /api/load_composition` with `"quantizeMode": N` in the file.

A1e. Per-clip snap granularity ("beat snap granularity", `docs/claude/performance-controls.md:69`): VERIFIED
`src/model/Clip.h:131-139` `enum class BeatSnapMode : uint8_t { Off, Beat, Bar, TwoBar, FourBar }` (comments: 1 beat / 4 beats / 8 beats /
16 beats), `Clip::beatSnapMode` default Off, legacy `bool beatSnap`. Persisted as int `src/model/Clip.cpp:72-73`, loaded :229-233
(legacy beatSnap true + Off -> Beat). UI: `src/ui/ClipInspector.cpp:162-175` combo "Snap Off","Beat","Bar","2 Bar","4 Bar"
(ids 1-5, `clip_->beatSnapMode = BeatSnapMode(sel-1)`), tooltip "Quantize clip trigger to next beat/bar boundary" (:168); model->combo at :1303.

A1f. Per-routine: VERIFIED `src/recording/RoutineEngine.h:20` `enum class RoutineSnap : uint8_t { Off, Beat, Bar, TwoBar, FourBar }`, value-for-value
the same as BeatSnapMode (static_asserts `RoutineEngine.cpp:12`). Routine stores a `Clip::BeatSnapMode quantize`; strings
"off|beat|bar|2bar|4bar" `src/model/Routine.h:88-106` (unknown -> Bar), REST `ApiServer.cpp:2285-2329,2414-2420` ("quantize must be off, beat, bar, 2bar or 4bar."),
pad menu `src/ui/RoutineDeckView.h:257-260` (Beat / Bar / "2 Bar" / "4 Bar"), toast text `MainComponent.cpp:6193-6200`,
start-line text `RoutineDeckView.h:109-121`. Engine uses `dueNow` (`RoutineEngine.cpp:229-240`) and `beatsUntilBoundary` (:245-256).

A1g. The pending override is stored in the Layer trigger tuple word: `src/model/Layer.h:112-114,123` shifts the snap enum into the top 4 bits
(`<< 28`); `Layer.h:181` `static_assert(FourBar < 16, "the snap override packs into 4 bits")`. Room: values 0-15 (5 used).

### Q2. The trigger queue: release boundary; what can it release on; what sub-beat would need

A2a. How a trigger is queued: VERIFIED `src/model/Layer.h:410-433` `triggerClip`: queue only when snap is enabled (forced snap != Off, or the
target's own beatSnapMode != Off) AND the target is not the already-active ref; a re-fire of the active clip is IMMEDIATE (restart),
never queued (:424). The queue entry = (pendingTriggerColumn, pendingDeckId, pendingTriggerSnapOverride) in the one CAS word.

A2b. How it is released: VERIFIED. Render thread, once per GL frame: `src/render/Renderer.cpp:508-513` calls `showAutopilot_.processFrame(*composition_, snap, ...)`.
`src/model/Autopilot.cpp:74-76`: `beats = beatCrossings_.consume(snapshot.totalBeatCount)`; if 0, return (NOTHING is released between beat edges).
`Autopilot.cpp:78-90`: for every layer with a pending column, `layer.processPendingTrigger(snapshot.beatInBar, snapshot.barCount, rows)`.
Pitfall 42 respected: the edge is the `totalBeatCount` DELTA (not the beatPhase wrap). `src/model/Layer.h:449-488` decides by mode:
Off/Beat -> every beat edge; Bar -> `beatInBar == 0`; TwoBar -> `beatInBar == 0 && barCount % 2 == 0`; FourBar -> `beatInBar == 0 && barCount % 4 == 0`.
A forced (global Quantize) override wins over the clip's own mode (:460-464); a clip with Off + legacy flag falls to Beat (:463-464).
barCount is the phrase-relative counter: set to 0 on no lock (`src/analysis/BPMTracker.cpp:509`), on drop entry / breakdown exit (:541), on Resync (:575);
`totalBarCount` is never rewound. So "2 Bar"/"4 Bar" parity is relative to the last phrase restart, not the song start (docs/claude/recording.md:82-83).
A stall: one `processPendingTrigger` call per frame with that frame's `beatInBar`; a bar edge inside a multi-beat gap is not seen; the trigger "slips to its next qualifying edge" (`Layer.h:394-396`).

A2c. Can it release on 1/2 bar, 1/4 bar, 1/8 bar, 1/16 bar today? Answer (in 4/4, 1 bar = 4 beats; `BPMTracker.h:61 kBeatsPerBar = 4`):
- 1/4 bar = 1 beat: YES, exists (`Beat`).
- 1 bar: YES (`Bar`). 2 bars, 4 bars: YES (`TwoBar`, `FourBar`) -- Boris's list does not name these two.
- 1/2 bar = 2 beats: NO. There is no enumerator and `processPendingTrigger` has no `beatInBar % 2` case. It would still be a WHOLE-BEAT edge (INFERRED: `beatInBar % 2 == 0` on the same beat edge).
- 1/8 bar = half a beat, 1/16 bar = quarter of a beat: NO, and not reachable by adding an enumerator alone, see A2d.
- Whole-beat edges only: VERIFIED (Autopilot.cpp:75-76 returns before any pending check unless the beat counter moved).

A2d. What sub-beat boundaries would need (facts, not design):
- A sub-beat edge counter. The only beat counters the render thread has are `totalBeatCount` (whole beats) and `beatPhase` [0,1) (`src/analysis/FeatureSnapshot.h:41,137`); continuous beat time is `totalBeatCount + beatPhase` ("continuous beat time with no wrap to detect", FeatureSnapshot.h:132-136). VERIFIED these exist; INFERRED that a half-/quarter-beat edge = a change of `floor(raw*2)` / `floor(raw*4)`, consumed once per frame like `OnsetPulse`/`beatCrossings_`.
- `processPendingTrigger` currently receives only `(beatInBar, barCount)`; a sub-beat grid would also need the sub-beat index inside the beat (INFERRED: signature/arg change; caller `Autopilot.cpp:85`).
- The early-return at `Autopilot.cpp:75-76` (`beats == 0`) sits BEFORE the pending loop, so pending triggers are only examined on whole-beat edges today (VERIFIED). Sub-beat release means the pending loop must run on a different edge (INFERRED).
- Timing resolution (INFERRED arithmetic, numbers from CLAUDE.md "4-Thread Model"): `beatPhase` is published once per analysis hop (512 samples @ 48 kHz = ~10.7 ms; `BPMTracker.cpp:223` `phase_ += hop/period`), the render frame is 16.67 ms. Worst-case release lateness ~ one hop + one frame (~27 ms). A quarter beat = 125 ms at 120 BPM, 83 ms at 180 BPM; so up to ~20-30 % of the cell at 180 BPM. UNKNOWN-NEEDS-A-RUN for real jitter (see section 4).
- Phase is not strictly monotone: a confident onset hard-realigns the phase to 0 (`BPMTracker.cpp:213-232,253-258`: +1 beat if phase >= 0.5, else restart). A sub-beat counter reading raw time must treat a backwards step as continuity (the same rule RecorderClock uses, Pitfall 42), or it double-fires/skips a sub-beat at a realign (INFERRED from the tracker code).
- `beatInBar` and `totalBeatCount` are written by different code paths: `beatInBar_` advances in `scoreBeat()` on an accepted real onset (`BPMTracker.cpp:380-382`) or in `advancePredictedBeat()` on the predicted wrap (:262-277); `totalBeatCount_` advances on phase wrap or a second-half realign (:226-232,255-256). The Bar test uses `beatInBar` read on the frame where `totalBeatCount` changed. INFERRED they agree to within a hop; UNKNOWN-NEEDS-A-RUN whether a real-audio bar release can land one beat off (manual/predicted regime is exact).
- A routine's quantize is a different clock: the RoutineEngine ticks on the message thread from a `RecorderClock` that integrates `totalBeatCount + beatPhase` (Pitfall 42, docs). `dueNow` takes `beatEdge`/`barEdge` flags (`RoutineEngine.cpp:229-240`) and `beatsUntilBoundary` already uses `lastBeatPhase_` (:245-256), so a sub-beat boundary for routines would use a continuous value that already exists (INFERRED).
- Packing: new snap enumerators fit the 4-bit field (Layer.h:181) up to 15; `RoutineSnap` and `Routine::quantizeToString/FromString`, `RoutineDeckView` menu, REST `quantizeKnown`, the `RoutineEngine.cpp:12` static_asserts and the `ClipInspector` combo are all enumerated by hand (VERIFIED locations above).
- The tracker-not-locked rule (`MainComponent.cpp:40`) turns ANY forced quantize into an immediate trigger until LOCKED (VERIFIED).

A2e. Boris's last entry "1/6": INFERRED reading = 1/16 (backlog BF20 OPEN-Q: "read as 1/16"). A literal 1/6 bar = 2/3 beat is a triplet and is not on any grid the app has. UNKNOWN: ask Boris. Boris did not list Off, 2 bars or 4 bars; today those exist (Off in the top menu; 2/4 bars in clip snap + routines).

### Q3. The adopted "bars everywhere" plan (bf7): items, and which conflict with today's words

Adopted: ruling-bf7 IN FULL (plan HARMONY ADOPTION, `.harmony/.reports/s-rta-1002b/plan-bf7.md:473-483`); Harmony defaults until Boris answers: Q1 keep "Bar 1-4" + the beat wheel; Q2 no 2/4-bar top-bar Quantize; Q3 autopilot counts from clip start; Q4 "N Bars" only; Q5 MilkDrop jukebox in bars only. Stages S0 -> S1 -> S2 -> S3 -> S4, then G7, then G5. Queued behind mkvidx/ui/bf2/bf10; NOT built (see scope fact).

Boris's earlier source for bf7: 2026-10-02 "Add bars to all places we have beats" (plan:11-12). Today's words (2026-10-03) add exceptions.

Status keys: OK = still consistent; CONFLICT = contradicts today's words; CHANGED = still wanted but content must change; OPEN = today's words do not decide it.

| Item | What it does (plan + ruling) | Status vs today | Why |
|---|---|---|---|
| I1 core/BeatLength.h (label/parse/isWholeBars/stepLadder, `kSyncCycleBeats`) | one shared bars-first vocabulary | CHANGED | its "whole bars print as Bars" rule must not apply to the clip-length rows and MilkDrop (beats stay beats there); needs a per-surface unit choice |
| I2 bar.beat counter | struck by AM1 | OK (stays struck) | Boris: the circle "counts off 1234 ... those are beats" = the beat wheel, which is already beats (A5-row TopBar below). Does not ask for "bar.beat". Ruling Q1 effectively answered "keep"; text "Bar N" next to it is not mentioned |
| I3 sync-length list + "4 Bars" (ConnPicker, UniversalParamControl BPM Sync) | shared list, labels "1 Bar" etc., +16-beat entry | OK-ish / OPEN | param LFO cycle length is "most other things" = bars; sub-bar entries stay "1/4 Beat" etc. (a beat is necessary there) |
| I4 SignalInspector oscillator/envelope | labels in bars, +"4 Bars" | OK (INFERRED: not "a clip's beats") | |
| I5 ClipInspector: autopilot lengths, "Beats/ Cycle"->"Loop length", "Content Beats"->"Content length", snap labels | bars labels | CONFLICT for beatDivision + videoBeats; OK for autopilot/snap | Boris: "For setting a clips beats, these are done with beats and not bars". beatDivision (loop length) and videoBeats (content length) are the clip's beats (`Clip.h:34-41`). Contradiction inside his own message: BF15 says a BPM-synced clip "has bars that the user can set". Backlog BF15/BF21 mark the unit OPEN-Q. UNKNOWN: ask |
| I6 LayerInspector On-Beat count | "1/2/4/8/16/32 Beats" -> bars | OK | "most other things" |
| I7->AM3 CompositionInspector per-type boxes -> BarStepSlider (+/- walk 1 Beat, 2 Beats, then bars) | bars | OK | |
| I8 MilkDrop playlist + jukebox (+AM10 table, `kJukeboxBarChoices`, drop the x4, default 16) | relabel to Bars | CONFLICT | Boris: "I have beats and seconds in the milkdrop editor. I only need beats." Plan makes it "Bars only" (Q5 default "no [seconds], bars only"). Needs BF28's OPEN-Q: beats vs bars |
| I9 grid labels "2 Bar"/"4 Bar" -> plural | wording | OK | still wanted if those entries survive |
| I10->AM11 label lint (flags `(4|8|12|16|...)\s*Beats?` etc., empty allowlist at merge) | enforces bars everywhere | CHANGED | the rule would reject "16 Beats" labels that Boris now wants for clip length and MilkDrop; allowlist/scoping needed |
| I11->AM8 TopBar Quantize: relabel to Off / Next Beat / Next Bar + combo follows the model; NO enum change, NO forcedSnapFor, 2/4-bar deferred to Boris Q2 | | CHANGED | Boris now names sub-bar values (1/2, 1/4, 1/8, 1/16) and a bar; the "follow the model" fix stays needed; AM8's "bf7 does NOT change the enum / MainComponent:27-34" no longer holds; 2/4 bars are not in his list (Q2 default "not now" is consistent) |
| I12 (AM12) slideshow "Beats per Image" -> "Image every" in bars | | OK | "most other things" |
| I13 (AM4) delete the dead Duration/"Beats" row in ClipInspector | | OK (independent) | VERIFIED still dead: `ClipInspector.cpp:89-98` (no onValueChange), label `:525-528` ("Beats"/"Duration"), layout `:628-638` |
| AM1 counter unchanged | | OK | matches "the circle ... those are beats" |
| AM2 typed lengths use the unit on screen | | OK where the box is in bars; moot where a box stays in beats | |
| AM6 row labels "Loop length"/"Content length"/"Image every" + tooltips | | CHANGED for the two clip-length rows if they stay beats | |
| AM10 jukebox old->new table | | CONFLICT | see I8 |
| Q4 "just bars, not both numbers" | | CHANGED | beats are now wanted in some rows |
| Q5 jukebox bars only | | CONFLICT | Boris: "I only need beats" |
| Plan row 20 (fade/transition times seconds) | unchanged | OK | not MilkDrop |
| Plan row 22 / Link quantum = 4 beats (= 1 bar) | unchanged | OK | |
| Plan row 16 routine pad "5/8" (bars) | unchanged | OK (INFERRED) | routines are "bars" in his wording elsewhere (RecordPanel From/To bar) |

### Q4. MilkDrop editor / jukebox: every control showing beats or seconds

The "editor" = `src/ui/MilkDropBrowser.cpp` (Jukebox / VJ / Playlist modes; layout :700-790). VERIFIED rows:

| # | Control | file:line | Label text | What it REALLY drives |
|---|---|---|---|---|
| M1 | `jukeboxTimingSelector_` | MilkDropBrowser.cpp:563-568 | "4 beats","8 beats","16 beats","32 beats","30 sec","60 sec"; default id 3 (:569) | onChange (:570-576) `bars[] = {1,2,4,8,8,16}` -> `PresetSelector::setTransitionBars(bars[idx])`. `PresetSelector.cpp:83` counts BAR crossings (`barPhase` wrap); `:169` switches when `barsSinceLastSwitch_ >= transitionBars_ * 4`. So the real intervals are 4, 8, 16, 32, 32, 64 BARS (= 16, 32, 64, 128, 128, 256 beats). The labels are wrong by 4x; "30 sec"/"60 sec" are 32/64 bars (at 120 BPM: 64 s/128 s), not seconds. Default id 3 "16 beats" = 16 bars (matches `PresetSelector.h:66 transitionBars_ = 4` -> x4 = 16 bars). `getTransitionBars` has no caller; not persisted |
| M2 | `jukeboxBlendSlider_` + `jukeboxBlendLabel_` "Blend:" | :580-597; header :116-117 | slider 0.5-5.0, default 2.0, text-box suffix "s" (:593) -> reads e.g. "2.0s" | `presetSelector_->setBlendSeconds` (:596) -> `ProjectMSource.cpp:272` `projectm_set_soft_cut_duration`. REAL SECONDS (crossfade length). This is the other "seconds" in the editor |
| M3 | `playlistTimingSelector_` | :612-616 | "4 beats","8 beats","16 beats","32 beats"; default id 2 (8) | `getPlaylistTriggerBeats()` {4,8,16,32} (:967-973) -> `MainComponent.cpp:1360` `clip.playlistTriggerBeats` -> `Renderer.cpp:570` compares against `presetBeatsPlayed` (the `totalBeatCount` delta, :553-558). Honest BEATS because `Clip::playlistTrigger` is never set by UI (grep: only Clip.cpp serialise/load) and defaults to `Beats` (`Clip.h:216-219`); the `Bars`(x4)/`Phrase`(x16) branches at Renderer.cpp:571-574 are unreachable from the UI |
| M4 | `playlistBlendSlider_` + `playlistBlendLabel_` "Blend:" | :620-634; header :123-124 | slider 0.3-3.0, default 1.5, suffix "s" | `getPlaylistBlendSeconds()` -> `clip.playlistBlendSeconds` (`MainComponent.cpp:1361`). VERIFIED `grep playlistBlendSeconds src`: written at MainComponent:1361, serialised Clip.cpp:158/348, declared Clip.h:222/305/368 -- NO READER in Renderer/ProjectMSource. The playlist Blend slider (in seconds) is decorative today |
| M5 | ProjectMSource "Speed" param (clip inspector, not the browser) | `src/sources/ProjectMSource.cpp:14,260-261` | normalised 0..1 slider | maps to `projectm_set_preset_duration(5 + (1-v)*55)` = 5-60 SECONDS preset dwell inside libprojectM; shown as a 0-1 number, not as seconds. Mentioned so a planner knows a seconds-valued quantity exists behind a plain slider |
| M6 | Header comment `MilkDropBrowser.h:115` "4/8/16/32 beats, 10/30/60 sec" | | comment only (stale) | |
| M7 | Playlist info label | `MilkDropBrowser.cpp:762-766` | "N selected - drag to cell" | no beats/seconds |

Facts a planner needs: the editor has two "beats" combos (M1 mislabelled, M3 honest) and two "seconds" sliders (M2 real, M4 decorative). Boris's "only beats" could mean any of: remove "30 sec/60 sec" (M1); also remove the two Blend sliders (M2/M4, real seconds); or relabel M1 in the unit it really counts (bars). UNKNOWN; BF28 already records "OPEN-Q: label in bars or beats". Making M1 honest in BEATS would need PresetSelector to count beats, not bars (`PresetSelector.cpp:78-83` uses the barPhase wrap, itself a wrap-reader of the Pitfall-42 kind: `barCrossing = barPhase < last && last > 0.5`) -- INFERRED. `PresetSelector` has no test (`ls tests | grep -i preset` = test_preset_manager.cpp, test_projectm_preset_manager.cpp only).

### Q5. Every UI label showing beat(s)/bar(s)

See section 2 TABLE A. Re-check of bf7's inventory: section 2 TABLE C (13 lines re-read at a7491d4, 8 hold, 5 moved).

---------------------------------------------------------------------------------------------------------------------
## 2 TABLES

### TABLE A: every user-visible label with beat(s) / bar(s) in src (VERIFIED by grep of all string literals in src/ui, MainComponent.*, then the non-UI dirs)

"Clip beats" column = Boris's "setting a clip's beats" candidate (INFERRED, ask). "Circle" = the top beat wheel.

| # | file:line | Label text | What it sets / shows | Clip beats? |
|---|---|---|---|---|
| A1 | src/ui/TopBar.cpp:178-180 | "Off", "Next Beat", "Next Downbeat" | global `Composition::quantizeMode` (Q1) | no |
| A2 | src/ui/TopBarModel.h:15 (drawn TopBar.cpp:514-526, 11 pt, 44 px box TopBar.cpp:561; wheel 26 px :557) | "Bar 1".."Bar 4", "Bar -" | read-only: `barCount % 4 + 1` (bar within the 4-bar group; restarts at drop/breakdown exit/Resync, BPMTracker.cpp:509,541,575) | no |
| A3 | src/ui/TopBar.cpp:458-512 (`paintBeatWheel`) | no text; 4 arc segments, segment `beatInBar` (0-3) lit, fades with `beatPhase` | read-only beat 1-2-3-4 | this IS the circle: beats already, no text |
| A4 | src/ui/TopBar.cpp:82 | tooltip "Reset beat phase to sync with the music" | Resync button | no |
| A5 | src/ui/ClipInspector.cpp:146-151 | "Layer Determined","1 Beat","2 Beats","4 Beats","8 Beats","16 Beats","32 Beats" | `Clip::autopilotDuration` enum (`Clip.h:156-160`, Beat1..Beat32) | maybe (how long the clip plays) |
| A6 | src/ui/ClipInspector.cpp:164-167 | "Snap Off","Beat","Bar","2 Bar","4 Bar" (tooltip :168) | `Clip::beatSnapMode` | no (trigger snap) |
| A7 | src/ui/ClipInspector.cpp:204 | label "Beats/ Cycle" | row label of A8 | YES |
| A8 | src/ui/ClipInspector.cpp:211-217 | "1/4 Beat","1/2 Beat","1 Beat","2 Beats","4 Beats (1 Bar)","8 Beats (2 Bars)","16 Beats (4 Bars)" | `Clip::beatDivision` (0.25..16 beats); BPM-Sync play-back length: `Renderer.cpp:1659-1665` speed = videoBeats/beatDivision; image seq `Renderer.cpp:1720-1730`. Shown only for a playable clip in BPM Sync mode (`ClipInspector.cpp:641-658`) | YES |
| A9 | src/ui/ClipInspector.cpp:237 | label "Content Beats" | row label of A10 | YES |
| A10 | src/ui/ClipInspector.cpp:243-266 | slider 1-64 step 1, editable 34 px text box, default 4, snaps to {1,2,4,8,16,32,64}, shows a bare number | `Clip::videoBeats` (the content's own length in beats; `Clip.h:38-41`); shown with A8 (`:660-667`) | YES |
| A11 | src/ui/ClipInspector.cpp:526 (painted) | "Beats" (BPM Sync) / "Duration" | row label for the DEAD durationSlider_ (`:89-98` no onValueChange; layout `:628-638`) | n/a (dead) |
| A12 | src/ui/ClipInspector.cpp:419 | "Bars" (fit combo item) | picture fit "bars" = letterbox bars, NOT music bars | no (false positive for a grep) |
| A13 | src/ui/LayerInspector.cpp:76 | "On Beat" (trigger mode, id 2) | layer autopilot trigger mode | no |
| A14 | src/ui/LayerInspector.cpp:106-111,113 | "1 Beat","2 Beats","4 Beats","8 Beats","16 Beats","32 Beats"; tooltip "Number of beats before advancing" | `Layer::defaultAutopilotDuration` (onChange :90-98; model->combo :898-899) | maybe |
| A15 | src/ui/LayerInspector.cpp:45,47,48 | tooltips "Autopilot: play previous/next/random clip on beat" | | no |
| A16 | src/ui/CompositionInspector.cpp:93-103 (box) + :105,110,115 | box shows a bare int 1-64 (IncDec, 30 px text box); the `label` args "Opaque Beats","Transparent Beats","Effect Beats" are UNUSED (never shown); painted row labels are "Opaque"/"Transparent" (plan V12, not re-read) | `perTypeAutopilot.opaqueCycleBeats` (16) / `transparentCycleBeats` (8) / `effectCycleBeats` (4) ints | no |
| A17 | src/ui/SignalInspector.cpp:41-46 | "1/4 Beat","1/2 Beat","1 Beat","2 Beats","4 Beats","8 Beats" | oscillator `setBeatDuration` {0.25..8}; handler :48-54 | no |
| A18 | src/ui/SignalInspector.cpp:87-91 | "1 Beat","2 Beats","4 Beats","8 Beats","16 Beats" | envelope `setBeatDuration` {1,2,4,8,16}; handler :92-98 | no |
| A19 | src/ui/UniversalParamControl.cpp:449 (menu build) and :607 (handler copy) | "1/4 Beat","1/2 Beat","1 Beat","2 Beats","4 Beats","8 Beats" x shapes Sine/Saw/Triangle/Square | BPM-Sync LFO `cycleBeats` on a connected param; ids 300+ | no |
| A20 | src/connect/ConnPicker.cpp:24-29,34 | "1/4 Beat" ... "8 Beats"; fallback `N + " Beat(s)"` | source-name text for an LFO (`describeSource`); mirrors A19 | no |
| A21 | src/MainComponent.cpp:357 | label "Beats per Image" | row label of A22 | no |
| A22 | src/MainComponent.cpp:455-468 | items are bare numbers 2,4,8,16,32,64,128 (`juce::String(b)`), default id 4 = 8 | `slideshowBeats_` (folder slideshow; laid out :2638-2639, 80 px label + 50 px box) | no |
| A23 | src/MainComponent.cpp:230, 239-249, 2627-2628 | "Random FX on Beat"; `beatRandomToggle_` "Beats" + `beatCountSelector_` bare "1,2,4,8,16,32" | beat-random FX; the toggle and selector are hidden (`setVisible(false)` :2627-2628) | hidden |
| A24 | src/ui/MilkDropBrowser.cpp:563-568 | "4 beats".."32 beats","30 sec","60 sec" | see Q4 M1 | no |
| A25 | src/ui/MilkDropBrowser.cpp:612-615 | "4 beats".."32 beats" | see Q4 M3 | no |
| A26 | src/ui/RoutineDeckView.h:257-260 | "Beat","Bar","2 Bar","4 Bar" (+ Off above) | routine pad Quantize submenu (`pad.quantize` strings "beat|bar|2bar|4bar") | no |
| A27 | src/ui/RoutineDeckView.h:109-121 | "Restarting from the top on the next beat." / "...two-bar line." / "...four-bar line." / "...bar." (same four for "Starting on...") | pad tooltips | no |
| A28 | src/ui/RoutineDeckView.h:33,184-186 and src/ui/RoutinePad.h:36 | pad shows "bar/barsTotal", e.g. "5/8" | progress through the routine in bars (the one live bar count above 4) | no |
| A29 | src/ui/RecordPanel.h:105-107, RecordPanel.cpp:136,147,156 | "From bar","To bar" + tooltips "...counted from 1" | save-routine bar range | no |
| A30 | src/MainComponent.cpp:6193-6200 | toast words "on the next beat/bar/two-bar line/four-bar line", "at once" | routine option toast | no |
| A31 | src/MainComponent.cpp:6146 | "; bars counted from the start of the take" | take message | no |
| A32 | src/ui/SignalStrip.cpp:196,199,299-309 | signal names "Beat Position"->"Beat", "Beat In Bar"->"Bar" | signal strip names (feature names, not settings) | no |
| A33 | src/ui/MappingEditor.cpp:27; src/signal/SignalRegistry.cpp:28,46,48; src/sources/ProjectMSource.cpp:13; src/effects/EffectLibrary.cpp:761 | "Beat Phase","Beat Position","Bar Position","Bar Count","Beat Sensitivity","Beat Ripple" | feature / param / effect names | no |
| A34 | src/ui/AudioReadoutPanel.cpp:310,336 | "Beat","Bar" | panel hidden (plan V15) | hidden |
| A35 | REST/JSON enum strings (not UI) | "beat","bar","2bar","4bar","beats" | ApiServer.cpp:2285-2329/2414-2420, Routine.h:88-106, RoutineEngine.cpp:86-91, ConnSerialization.cpp:124, Lane.h:96, TempoMap.cpp:8 | n/a |

### TABLE B: how the same quantities are stored (what a label change does NOT need to migrate)
- Clip length values: floats in beats (`beatDivision`, `videoBeats`), ints for per-type cycles, enums for autopilot durations (Beat1..Beat32), Clip.cpp serialises numbers.
- Quantize / snap: `QuantizeMode` and `BeatSnapMode` as ints (`Composition.h:742,880`; `Clip.cpp:73,230`), routine quantize as strings.

### TABLE C: re-check of bf7 plan/ruling line references against a7491d4 (13 re-read; plan base was main eff2b1c/c6720a0)
| Plan cite | Holds at a7491d4? | Actual |
|---|---|---|
| ClipInspector.cpp:146-151, 166-167, 204, 211-217, 237 | HOLD | same lines |
| LayerInspector.cpp:106-111, 113 | HOLD | same |
| SignalInspector.cpp:41-46, 87-91 | HOLD | same |
| MilkDropBrowser.cpp:563-566, 612-615 (+563-575 map) | HOLD | same |
| UniversalParamControl.cpp:449, 607 | HOLD | same |
| ConnPicker.cpp:24-29, 34 | HOLD | same |
| CompositionInspector.cpp:105-115 | HOLD | same |
| ClipInspector Duration row :89-98, :525-526 | HOLD (ruling cites :528; text at :526) | dead row still dead |
| TopBarModel.h:11-16 | HOLD | same |
| TopBar.cpp:176-189 | HOLD (items :178-180) | |
| TopBar.cpp:531-544 (counter paint) | MOVED | 514-526 |
| RoutineDeckView.h:278-279 ("2 Bar"/"4 Bar") | MOVED | 259-260 (and :109-121 text) |
| MainComponent.cpp:366 ("Beats per Image"), :464-477, :2621-2622, :27-34 | MOVED | :357, :455-468, :2638-2639, :36-43 |
| Composition.h:117-118 (QuantizeMode) | MOVED | :144 |
| Layer.h:437-470 / 457-479 (processPendingTrigger) | MOVED | :446-488 |
| Autopilot.cpp:97-98 (beatsPlayed) | MOVED | :108-109 |
=> label lines in the inspectors and MilkDrop are unchanged; the shifted ones are MainComponent / TopBar paint / RoutineDeckView / Composition / Layer (bf9b added deck code above them). Any builder must re-read before editing.

---------------------------------------------------------------------------------------------------------------------
## 3 WHAT EXISTS TODAY vs WHAT BORIS ASKED (plain gaps)

1. Quantize list. Today: Off / Next Beat / Next Downbeat (3 entries; "Downbeat" = a bar). Boris: 1 bar, 1/2 bar, 1/4 (bar), 1/8 (bar), "1/6" (= 1/16 per backlog OPEN-Q). Gap: the top menu has "1 bar" (as "Next Downbeat") and "1/4 bar" (as "Next Beat"); it has NO 1/2 bar, NO 1/8 bar, NO 1/16 bar; the labels do not use bar fractions. The trigger queue can only release on whole beats and whole-bar multiples (A2c). The menu also does not follow the model (A1d). The reading of "1/6" is not settled.
2. 2 bars / 4 bars: exist for clip snap and routines, not in the top menu and not in Boris's list. Off: exists, not in his list.
3. Clip snap (per clip) and routine quantize have their own 5-value lists; whether Boris wants his list there too is not stated (UNKNOWN). Same hand-kept enum copies (A1f) would all change together if one list is shared.
4. Bars vs beats wording. Today every length is in beats (A5, A8, A10, A14, A17-A22, A24-A25); only the routine pads, record panel and clip/routine snap speak bars. Boris: bars in most places, beats for "a clip's beats", beats for the top circle. Gap: the rows that count as "a clip's beats" are not named; strongest candidates are Loop length (beatDivision, A7-A8) and Content length (videoBeats, A9-A10); autopilot lengths (A5, A14, A16) are ambiguous. His own BF15 sentence says a BPM-synced clip "has bars that the user can set".
5. Top circle. Already beats (4 segments, no text). The text "Bar 1..4" beside it is a bar counter; Boris's sentence does not say whether it stays.
6. MilkDrop editor. Today: jukebox timing mislabelled in beats (really bars x4) plus "30 sec"/"60 sec" (really bars); playlist timing honestly beats; two Blend sliders in real seconds (jukebox real, playlist decorative). Boris: "I only need beats." Gap: nothing in the editor is a clean beats-only control yet; the jukebox engine counts bars.
7. bf7 un-built: BeatLength vocabulary, lint, BarStepSlider, relabels, the Quantize combo sync, the removed Duration row are all still to do, and the adopted ruling's choices on rows 3, 4, 13, 14 (clip length rows, MilkDrop) and Q4/Q5 contradict today's words.

---------------------------------------------------------------------------------------------------------------------
## 4 UNKNOWN-NEEDS-A-RUN

U1. Real release lateness for a sub-beat boundary (hop 10.7 ms + frame 16.7 ms). Cheapest test: production mode (port 7070), `POST /api/set_bpm {"bpm":120}` (manual LOCKED tracker, predicted phase, bar = 2.0 s), then sample `GET /api/bpm` (fields beatInBar, barCount, totalBeatCount, beatPhase; ApiServer.cpp:833-848) at 5-10 ms and log the (totalBeatCount+beatPhase) value at which a render-side change is visible; compare edge spacing to 125 ms / 250 ms. Note the render frame sees the snapshot only per frame, so the measurement is an upper bound for the poll path, not the render path; the render-path number needs a debug counter (not present).
U2. Whether a real-audio Bar/TwoBar/FourBar release can land one beat off because `beatInBar` and `totalBeatCount` advance on different code paths (BPMTracker.cpp:380 vs :226-232). Test: play a steady 4/4 loop (audio source = file), `POST /api/trigger_clip` with a Bar snap clip, poll `GET /api/bpm` for beatInBar at the moment the active column flips (layer state via `GET /api/composition`/`/api/state`); expect beatInBar == 0 at the flip. Manual/predicted regime (set_bpm) is exact and is the control arm.
U3. What Boris means by "1/6" (1/16 vs a 1/6 triplet) and by "a clip's beats" (which rows). Ask; no code answers it.
U4. Whether Boris wants "Off", "2 bars", "4 bars" kept in the top menu, and whether clip-snap and routine quantize get the new fractions.
U5. Whether the MilkDrop Blend sliders (seconds) are among "seconds" he wants gone.
U6. Whether a loaded composition's quantizeMode value outside 0-2 (e.g. from an older/newer file) is harmful: `Composition.h:880` unguarded cast; `quantizeModeToForcedSnap` maps unknown -> Bar. Cheapest test: load a composition with `"quantizeMode": 9` via `POST /api/load_composition`, trigger a clip with BPM locked, observe Bar behaviour. (Low risk; only matters when new enumerators are added/removed.)
U7. Perceived feel of a quarter-beat quantize at 150+ BPM (human check on a rig; cannot be decided from code).

---------------------------------------------------------------------------------------------------------------------
## 5 FILES A BUILDER WOULD TOUCH (paths only; recon-relative)

Quantize list + sub-beat release
- src/model/Composition.h (QuantizeMode :144, load :880, save :742)
- src/MainComponent.cpp (quantizeModeToForcedSnap :36-43; callers :4176,:4746,:4975,:6156; toast :6193-6200; replay :2003-2006)
- src/ui/TopBar.cpp, src/ui/TopBar.h (combo :177-188, :603; a model->combo sync in timerCallback ~:300-335)
- src/model/Clip.h (BeatSnapMode :131-139), src/model/Clip.cpp (:73,:229-233)
- src/model/Layer.h (processPendingTrigger :446-488; pack :112-123; static_assert :181)
- src/model/Autopilot.cpp, src/model/Autopilot.h (edge gate :74-90)
- src/recording/RoutineEngine.h/.cpp (RoutineSnap, dueNow, beatsUntilBoundary, static_asserts, :86-91 strings)
- src/model/Routine.h (:88-106), src/api/ApiServer.cpp (quantizeKnown :2285, :2324-2329, :2414-2420), src/api/ApiServer.h (:151,:158)
- src/ui/ClipInspector.cpp (:162-175, :1303), src/ui/RoutineDeckView.h (:109-121, :257-282)
- src/recording/PerfState.h/.cpp, PerfStateCapture.cpp, Program.cpp (quantizeMode int)
- src/analysis/FeatureSnapshot.h and BPMTracker.cpp only if the sub-beat edge is published by the tracker (INFERRED alternative to a render-side counter)

Bars/beats wording
- src/ui/ClipInspector.cpp (:146-151, :162-168, :204-217, :237-266, :519-528 paint, :628-638 layout, :1303-1321), src/ui/ClipInspector.h
- src/ui/LayerInspector.cpp (:106-113, :898-908)
- src/ui/CompositionInspector.cpp/.h (:93-115)
- src/ui/SignalInspector.cpp (:41-56, :87-101, reverse maps ~:258-287)
- src/ui/UniversalParamControl.cpp (:442-465, :598-612), src/ui/UniversalParamControl.h, src/connect/ConnPicker.cpp (:10-35)
- src/MainComponent.cpp (:357, :455-468, :2638-2639)
- src/ui/TopBarModel.h, src/ui/TopBar.cpp (:514-526) only if the "Bar N" text changes
- (new, per bf7 paper) src/core/BeatLength.h, src/ui/BarStepSlider.h

MilkDrop editor
- src/ui/MilkDropBrowser.cpp/.h (:563-597, :612-634, :700-790, :967-978; header comment :115,:122)
- src/sources/PresetSelector.h/.cpp (:66 default, :83, :169, setTransitionBars :36)
- src/sources/ProjectMSource.cpp (:260-272 Speed / soft-cut), src/MainComponent.cpp (:1360-1361), src/render/Renderer.cpp (:570-574 playlist x4/x16 branches), src/model/Clip.h/.cpp (:216-222, :156-158, :344-349)

Docs / inventory a change would touch: docs/claude/performance-controls.md (beat snap :69), docs/claude/recording.md (:36, :82-83), docs/claude/effects.md (Autopilot System), .harmony/APP-INVENTORY.md (TopBar row :56), BORIS_DECISIONS.md.

---------------------------------------------------------------------------------------------------------------------
## 6 EXISTING TESTS / PROBES and REST routes that can drive this area

Existing tests (names = files under tests/; VERIFIED present, grep for symbols):
- tests/test_layer_runtime.cpp (processPendingTrigger with forced Beat override :349-359; snap enum round trips :70-71,:181-182; TwoBar :369), tests/test_layer_runtime_race.cpp
- tests/test_routine_engine.cpp (TwoBar/FourBar start :729,:746; `beatsUntilBoundary` check table :1654-1697), tests/test_routine.cpp (quantize strings)
- tests/test_autopilot.cpp ([autopilot][stall] F3 cases :268-358; [autopilot][law] :373: no phase-wrap reader), tests/test_per_type_autopilot_layout.cpp
- tests/test_composition.cpp (QuantizeMode NextBeat round trip :158,:195; no NextDownbeat, no missing-key case)
- tests/test_topbar_model.cpp (pins "Bar N" wrap; would change only if the counter text changes)
- tests/test_clip_inspector_paint_key.cpp, tests/test_conn_picker.cpp (pins "Square 1 Beat" etc.), tests/test_bpm_stabilization.cpp ([bpm][beatcount]), tests/test_take.cpp ([recorderclock][stall]), tests/test_program_preamble.cpp, tests/test_show_model.cpp, tests/test_undo_commands.cpp, tests/ShowFixture.h
- NO test for: the TopBar Quantize combo (items or model sync), PresetSelector / the jukebox timing, MilkDropBrowser labels, SignalInspector lists, LayerInspector count combo, CompositionInspector per-type boxes' text, any sub-beat release.
- bf7 would add (paper only): test_beat_length, test_beat_label_lint, test_bar_step_slider, test_preset_selector_bars, test_topbar_quantize_sync, test_beat_label_widths.

Probes (shell, live app): .harmony/probe-routines.sh (production mode port 7070, no --test-mode; `POST /api/set_bpm {"bpm":120}` for a manual LOCKED tracker, bar = 2.0 s; fires `POST /api/trigger_clip`; covers ROUTINE quantize, not the global Quantize), .harmony/probe-beatclock.sh (beat clock under a message-thread stall; TEST-ONLY `POST /api/debug/stall_message_thread`), .harmony/probe-downbeat-level.sh. No probe sets or asserts the GLOBAL Quantize (only fixtures carry `"quantizeMode": 0`: make-bf9b-check.py:57, probe-vupload.py:473).

REST routes (production 7070, `docs/claude/integration.md`; route list from `src/api/ApiServer.cpp` registrations, :192 etc.) that drive this area without synthetic input:
- `POST /api/set_bpm {"bpm":120}` -> manual LOCKED tracker with predicted phase (exact grid; barCount and totalBarCount advance together). `POST /api/resync`.
- `GET /api/bpm` -> beatInBar, barCount, totalBarCount, totalBeatCount, beatPhase, trackerState (ApiServer.cpp:215, :833-848). Poll at 5-50 ms to timestamp edges.
- `POST /api/trigger_clip {"layer":L,"column":C}` / `POST /api/trigger_column` -> go through `quantizeModeToForcedSnap` (the same path as a click: `MainComponent.cpp:4746,4975`).
- `POST /api/load_composition` -> the ONLY way to set the global Quantize (file key `quantizeMode`) and per-clip `beatSnapMode` (Clip.cpp:73) without the UI; `GET /api/composition` reads clip fields back (beatDivision/videoBeats/beatSnapMode via Clip::toVar, Clip.cpp:72-73,190-192).
- `POST /api/routine/save|fire|stop|set|remove`, `GET /api/routine/status` (routine quantize strings "off|beat|bar|2bar|4bar").
- `GET /api/debug/ui_text` (reads UI text, e.g. combo contents; ApiServer.cpp:319,2066), `POST /api/debug/ui_test_menu` (:314), `GET /api/debug/ui_paint|ui_passes`.
- Test server 8080 (`--test-mode`, no analysis thread): `POST /api/inject_features` sets beatPhase, beatInBar, barCount, totalBarCount, totalBeatCount (TestServer.cpp:510-525; production variant ApiServer.cpp:961-983) -> the Autopilot beat edge fires from injected counters (Pitfall 42 note: move totalBeatCount with beatPhase). Use it to test a sub-beat edge counter deterministically without audio; `POST /api/render_frame` to step; `/api/state`.
- No route sets the top Quantize directly and none reads a pending (queued) trigger; a queued trigger is visible only via the layer tuple (no UI reads `pendingTriggerColumn`: `grep src/ui` = 0 hits, VERIFIED) -- the cheapest read-back is the active column in `GET /api/composition` after the expected edge.
