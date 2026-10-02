# PLAN bf7: bars wherever the app has beats (s-rta-1002b), FINAL

Repo /Users/boriskarpman/projects/RealTimeAudio, main eff2b1c (it contains 5e47d17; there is zero src/ or tests/ diff
between the two). Paths are relative to src/ unless they start with tests/, docs/ or .harmony/. The dispatch's recon
tables are not repeated here. Only the claims this design leans on were re-read.

## 1. GOAL
Every beat-count setting and every beat counter speaks bars (4/4 only: 1 bar = 4 beats) through ONE shared vocabulary.
The top counter reads bar.beat, with the bar wrapping 1-2-3-4-1. Stored values stay in beats. Only routines show bar
numbers above 4.
- Boris, 2026-10-02: "Add bars to all places we have beats".
- Boris, 2026-09-26 (kept, reconciled below): "we only need longer than 4 bar counts for routines and that should be
  displayed with the routine and nothing else. Top bar count should go 1-2-3-4-1 etc".

HOW BOTH RULINGS HOLD (no question to Boris needed):
- COUNTS are numbers that tick up while music plays.
  - The top counter becomes "bar.beat" (e.g. 3.2). Its bar digit is exactly today's 1-2-3-4-1 (barCount % 4 + 1). Its
    beat digit also runs 1-2-3-4-1.
  - The 2026-09-26 note was never confirmed: it could mean bars 1-4 or beats 1-4. bar.beat satisfies both readings.
  - No other counter shows a bar number above 4. Routine pads keep "5/8" (that is "displayed with the routine").
  - The bar digit restarts at a drop, at a breakdown exit and at Resync. That is the same edge the 2 Bar / 4 Bar
    quantize uses, so "1.1" on the counter is exactly where a 4-bar quantize fires.
- LENGTHS are settings: how long, or how often. They are not counts, and BF7 labels them in bars.
  - bf7 adds NO length above 4 bars.
  - Some lengths above 4 bars already exist (autopilot 32 beats, per-type cycles up to 64, content length up to 64,
    MilkDrop jukebox). They keep their values and are now labelled truthfully in bars.

## INVENTORY -> PROPOSAL (place | today | proposed | storage | UI change)
| # | Place | Today | Proposed (bars) | Storage change | UI change |
|---|---|---|---|---|---|
| 1 | TopBar counter (ui/TopBar.cpp:531-544) | "Bar 1".."Bar 4", "Bar -" | "1.1".."4.4", "-.-"; the bar wraps 1-4 from the last phrase restart | none | text only |
| 2 | TopBar Quantize (TopBar.cpp:176-189) | Off / Next Beat / Next Downbeat | Off / Beat / Bar / 2 Bars / 4 Bars | QuantizeMode appends 3, 4; old files unchanged | 2 new items; the combo follows the model |
| 3 | Clip "Beats/ Cycle" (beatDivision) | 1/4 Beat, 1/2 Beat, 1 Beat, 2 Beats, 4 Beats (1 Bar), 8 Beats (2 Bars), 16 Beats (4 Bars) | 1/4 Beat, 1/2 Beat, 1 Beat, 2 Beats, 1 Bar, 2 Bars, 4 Bars | none (float beats) | labels; row label becomes "Cycle" |
| 4 | Clip "Content Beats" (videoBeats) | number 1-64, snaps 1,2,4,8,16,32,64 | 1 Beat, 2 Beats, 1 Bar, 2 Bars, 4 Bars, 8 Bars, 16 Bars; typing "2 bars" works | none | box 34 -> 52 px; label becomes "Content" |
| 5 | Clip snap | Snap Off, Beat, Bar, 2 Bar, 4 Bar | Snap Off, Beat, Bar, 2 Bars, 4 Bars | none | 2 labels |
| 6 | Clip autopilot | Layer Determined, 1/2/4/8/16/32 Beats | Layer Determined, 1 Beat, 2 Beats, 1 Bar, 2 Bars, 4 Bars, 8 Bars | none (enum) | labels |
| 7 | Layer On-Beat count | 1/2/4/8/16/32 Beats | 1 Beat, 2 Beats, 1 Bar, 2 Bars, 4 Bars, 8 Bars | none (enum) | labels + tooltip |
| 8 | Composition per-type Opaque / Transparent / Effect | IncDec numbers 1-64 | same 1-beat steps; text reads "4 Bars" / "6 Beats"; typing "4 bars" works | none (int beats) | box 30 -> 52 px |
| 9 | Signal oscillator | 1/4 Beat .. 8 Beats (6 items) | ..., 1 Bar, 2 Bars, 4 Bars (NEW) | none (not persisted today, recon A39) | +1 item |
| 10 | Signal envelope | 1/2/4/8/16 Beats | 1 Beat, 2 Beats, 1 Bar, 2 Bars, 4 Bars | none | labels |
| 11 | Param BPM Sync menu (UniversalParamControl) + connection LFO (ConnPicker) | 4 shapes x 1/4 Beat .. 8 Beats | x 1/4 Beat .. 1 Bar, 2 Bars, 4 Bars (NEW) | none (cycleBeats float; 16 is already legal, ParamConnection.h:47) | +1 division per shape; button reads e.g. "Sine 1 Bar" |
| 12 | Connection Timeline envelope (fixed 4 beats) | no number shown | unchanged | none | none (bf6 / bf45 own it) |
| 13 | MilkDrop playlist timing | 4/8/16/32 beats | 1 Bar, 2 Bars, 4 Bars, 8 Bars | none | labels |
| 14 | MilkDrop jukebox timing | "4 beats".."32 beats", "30 sec", "60 sec" (really 4,8,16,32,32,64 BARS) | 4 Bars, 8 Bars, 16 Bars (default), 32 Bars, 64 Bars | not persisted | 5 items; switching unchanged |
| 15 | Routine pad Quantize submenu | Off, Beat, Bar, 2 Bar, 4 Bar | Off, Beat, Bar, 2 Bars, 4 Bars | none (strings "2bar" / "4bar" unchanged) | 2 labels |
| 16 | Routine pad progress "bar/barsTotal" | "5/8" | unchanged (the one live count above 4) | none | none |
| 17 | Record panel From bar / To bar; REST routine fromBar/toBar, quantize strings | already bars | unchanged | none | none |
| 18 | Other REST / OSC beat fields | none exist (recon C14, C16) | no new routes | n/a | n/a |
| 19 | Hidden or inert: beat-random selector (hidden, recon H1); AudioReadoutPanel (hidden, V15); ClipInspector Duration/"Beats" row (inert, V11) | n/a | not touched (findings) | n/a | n/a |
| 20 | Fade / transition times | seconds | unchanged (not beats) | n/a | n/a |
| 21 | Hits quantize (BORIS_DECISIONS.md:107-113) | spec only, no code (recon G5) | when built, it uses core/BeatLength.h | n/a | n/a |
| 22 | Ableton Link quantum (sync/LinkSync.h) | 4 beats | unchanged (= 1 bar in 4/4) | n/a | n/a |

## 2. ESTABLISHED
VERIFIED (read this session):
- V1. ui/TopBarModel.h:11-16:
  - `kBarsPerCount = 4`.
  - `barReadoutText(hasBpm, barCount)` returns "Bar " + (barCount % 4 + 1), or "Bar -".
  - Drawn by TopBar::paintBarPhraseDisplay (TopBar.cpp:531-544) inside barPhraseBounds_, which is 44 px wide
    (TopBar.cpp:578).
- V2. The counter text is already repainted at 15 Hz:
  - TopBar.cpp:306 calls startTimerHz(15). TopBar.cpp:325-330 calls repaint(getWheelRepaintBounds()).
  - TopBar.h:68 defines that rect as beatWheelBounds_ UNION barPhraseBounds_.
  - TopBar.cpp:316-322 copies beatInBar and barCount from ONE snapshot.
  - So a per-beat text change adds no repaint (safe for Pitfall 57).
- V3. barCount is incremented at analysis/BPMTracker.cpp:521 (totalBarCount at :522). It is set to 0 at three places:
  - :509 when there is no lock;
  - :541 when the structural state changes into a drop or out of a breakdown, at whatever moment that happens, and not
    while predictedBeatRegime_;
  - :575 on Resync.
- V4. Global Quantize:
  - Combo at TopBar.cpp:176-189: "Off"[1], "Next Beat"[2], "Next Downbeat"[3]. onChange casts id-1.
  - Enum at model/Composition.h:117-118: `{ Off, NextBeat, NextDownbeat }`. Loaded at :451 with an unguarded
    static_cast.
  - NOTHING moves the combo when the model changes. grep of quantizeSelector_ finds only the constructor and :620.
  - The take/routine preamble replay writes the model (MainComponent.cpp:2014). This is a live Pitfall 41 case (I read
    docs/claude/pitfalls.md:91).
- V5. MainComponent.cpp:27-34, `quantizeModeToForcedSnap` (anonymous namespace):
  - Off -> Off; tracker not LOCKED -> Off; NextBeat -> Beat; anything else -> Bar.
  - Callers: :4090, :4653, :4862, :6107.
  - Composition.h:2 includes model/Deck.h.
- V6. A grep for numeric beat labels finds them ONLY at:
  - ClipInspector.cpp:146-151 and :211-217;
  - LayerInspector.cpp:106-111;
  - SignalInspector.cpp:41-46 and :87-91;
  - MilkDropBrowser.cpp:563-566 and :612-615;
  - UniversalParamControl.cpp:449 and :607;
  - connect/ConnPicker.cpp:24-29.
  The grid labels "2 Bar" / "4 Bar" are at ClipInspector.cpp:166-167 and RoutineDeckView.h:278-279.
- V7. The jukebox labels are wrong by 4x:
  - MilkDropBrowser.cpp:563-575 maps the items to setTransitionBars({1,2,4,8,8,16}).
  - PresetSelector.cpp:79-83 counts BAR crossings. :169 switches when barsSinceLastSwitch_ >= transitionBars_ * 4.
  - getTransitionBars has no caller. Nothing persists the setting.
  - PresetSelector is default-constructible and has setPresetManager, setEnabled, setCycleMode and onAutoSwitch
    (PresetSelector.h:14-59).
  - ProjectMPresetManager is std-only and has scanDirectory (ProjectMPresetManager.h:26).
  - tests/test_projectm_preset_manager.cpp already links it.
- V8. The playlist timing chain is honest. Items at MilkDropBrowser.cpp:612-615 flow through getPlaylistTriggerBeats
  {4,8,16,32} (:967-973), then MainComponent.cpp:1388, then render/Renderer.cpp:611-615.
- V9. BPM-sync menu ids:
  - UniversalParamControl.cpp:446-452 builds ids as 300 + shape*6 + div.
  - The handler at :598-612 checks `>= 300 && < 324`, uses /6 and %6, and keeps its own copy of the labels.
  - The next id range starts at 400 (:472, :618).
  - ConnPicker.cpp:16 has kBpmDivisions with 6 entries and says it mirrors those labels "verbatim" (:12-15, :20-21).
  - The divIdx clamp uses .size() (:78-82). ConnPicker is std-only (ConnPicker.h:2-4).
- V10. SignalInspector:
  - Oscillator: list at :41-56, reverse map at :258-267.
  - Envelope: list at :87-101, reverse map at :280-287.
- V11. ClipInspector:
  - Label "Beats/ Cycle" is at :204; beatDivision items at :211-217.
  - The "Content Beats" block is at :237-266, with an editable 34 px text box.
  - durationSlider_ (:90-98) is INERT: it has no onValueChange and no model write. Its row label reads "Beats"
    (:525-526). This is a finding; bf7 does not touch it.
- V12. CompositionInspector setupCycleSlider (:93-103) builds an IncDec 1..64 slider with an editable 30 px box. The
  painted row labels are "Opaque" / "Transparent" (:237-238).
- V13. LayerInspector: items at :106-111, durToSel at :903-908.
- V14. JUCE 8 Slider has textFromValueFunction and valueFromTextFunction (build/_deps/juce-src/.../juce_Slider.h:626,
  :629). ResettableSlider overrides neither.
- V15. AudioReadoutPanel is hidden: MainComponent.cpp:2591 and :2667 call setVisible(false), and nothing calls
  setVisible(true). RecordPanel has no live beat display.
- V16. tests/test_topbar_model.cpp:7-18 pins "Bar N" and must change. test_conn_picker.cpp:93-99 ("Square 1 Beat")
  stays. No test asserts "2 Bar", "4 Bar" or "Next Downbeat".
- V17. Lint precedent: tests/test_log_line_lint.cpp (AUDIODNA_SRC_DIR at :23). There is one test exe per file
  (tests/CMakeLists.txt:1366-1392). core/ holds std-level headers (core/LogLine.h).

INFERRED (recon-cited; the builder re-reads these before the stage that leans on them):
- N1. Clip and routine engines already honour a FORCED TwoBar / FourBar:
  - model/Layer.h:437-470: FourBar fires when beatInBar==0 && barCount%4==0.
  - RoutineEngine.cpp:222-262. The packed override has room (Layer.h:140).
- N2. Autopilot counts beats from the clip's activation (Autopilot.cpp:97-98, Layer.h:540), not from bar lines.
- N3. 4/4 is hard-coded at about 10 sites and there is no meter field:
  - analysis/BPMTracker.h:61, RoutineSlice.cpp:12, RoutineEngine.h:60, RoutineBankModel.h:14, TopBarModel.h:11;
  - LinkSync quantum; OscillatorSignal.h:56-61; ConnectionShaper; Renderer.cpp:611-615; Layer.h:455-466.
- N4. Link is off in the default build. Its quantum is 4 beats = 1 bar. Only its BPM reaches the tracker, so bar lines
  always come from our own tracker.

## 3. DESIGN FORKS
F1. Counter vs the 2026-09-26 ruling.
- CHOSEN: (a) "bar.beat", with the bar wrapping at 4.
- (b) Keep "Bar N" and let the wheel show the beat. Loses: the beat never gets a number.
- (c) A running count such as "37.2". Breaks the 2026-09-26 ruling.
- (d) Ask Boris first. Unnecessary: (a) holds under both readings.

F2. Label style.
- CHOSEN: (a) a single bars-first label.
  - Whole bars read "1 Bar" / "N Bars".
  - Values under a bar read "1/4 Beat", "1/2 Beat", "1 Beat", "2 Beats".
  - Anything else reads in beats, e.g. "6 Beats".
- (b) Dual labels such as "16 Beats (4 Bars)" (today's 3-item precedent). Loses: it doubles the width of narrow
  combos and keeps beats as the first-read unit. Q1 can flip this in one function.
- (c) A Beats|Bars toggle. Loses: it adds persisted state.
- (d) bar.beat notation for lengths. Loses: cryptic.

F3. Storage.
- CHOSEN: beats stay the stored unit everywhere; no JSON key changes.
- The one enum change: QuantizeMode APPENDS NextTwoBars=3 and NextFourBars=4.
- An older build reading 3 or 4 falls into V5's "anything else -> Bar" branch, which is a safe degrade.
- Runner-up: store bars. Loses: it means a migration for zero benefit.

F4. Range.
- CHOSEN: relabel everything, plus two extensions, each 4 bars or less:
  - (i) The global Quantize gains 2 Bars and 4 Bars. Clip snap and routines already have them.
  - (ii) The BPM-sync / LFO / oscillator lists gain 4 Bars. They stop at 2 bars today, while the envelope list and
    the counter's cycle are 4 bars.
- Runner-up: relabel only. Loses: the global Quantize would stay the one bar setting with no 2- or 4-bar values.
- Both extensions are separable (I3's 7th entry; I11) and can be dropped without touching the rest.

F5. Where the vocabulary lives.
- CHOSEN: core/BeatLength.h, using std::string.
- Runner-ups: put it in ui/ (connect/ would then need JUCE), or keep today's three hand-kept copies (V9).

F6. Jukebox.
- CHOSEN: the labels follow today's behaviour, and setTransitionBars takes real bars.
- Runner-up: make the behaviour follow the old labels. Loses: presets would switch 4x more often by default.

F7. Typed lengths.
- CHOSEN: a bare number means BEATS, as it did yesterday. "4 bars", "4 bar" and "4bars" mean bars.
- Runner-up: a bare number means whatever unit is shown. Loses: the same keys would mean lengths 4x apart.

F8. Quantize combo follows the model.
- CHOSEN: yes (Pitfall 41). With 4-bar quantize, a stale "Off" would hide waits of up to 8 s at 120 BPM.

F9. Time signature.
- CHOSEN: 4/4 only, and NO question to Boris. Another meter is not cheap: it needs the analysis downbeat model plus the
  N3 sites. The N3 list goes into the docs for any future meter lane.

## 4. ITEMS
I1. core/BeatLength.h (new; header-only; std-only; namespace beatlen)
- Contents:
  - `inline constexpr double kBeatsPerBar = 4.0;` (comment: 4/4 only, BF7).
  - `inline constexpr std::array<double,7> kSyncCycleBeats{0.25,0.5,1,2,4,8,16};`. This is the ONE list that the
    UniversalParamControl BPM Sync menu and ConnPicker index.
- `std::string label(double beats)`, rules applied in order:
  1. Not finite, or <= 0: return "".
  2. beats/4 is within 1e-4 of an integer n >= 1: "1 Bar" or "<n> Bars".
  3. Under 1 beat and on the quarter grid: "1/4 Beat", "1/2 Beat" or "3/4 Beat".
  4. An integer n: "1 Beat" or "<n> Beats".
  5. Otherwise, trimmed to 2 decimals plus " Beats".
- `std::optional<double> parse(std::string_view)`:
  - Trims and is case-insensitive.
  - Accepts a number, which may be a decimal or a fraction a/b, optionally followed by beat / beats / bar / bars.
  - A bare number means beats.
  - Returns nullopt for empty, unknown-unit, non-positive or non-finite input.
- RED-first test, tests/test_beat_length.cpp (new exe, links nothing else):
  - Label table: 0.25 -> "1/4 Beat", 0.5 -> "1/2 Beat", 1 -> "1 Beat", 2 -> "2 Beats", 3 -> "3 Beats", 4 -> "1 Bar",
    6 -> "6 Beats", 8 -> "2 Bars", 12 -> "3 Bars", 16 -> "4 Bars", 32 -> "8 Bars", 64 -> "16 Bars",
    1.5 -> "1.5 Beats", 4.00001f -> "1 Bar" (Clip::beatDivision is a float).
  - Parse table: "4 bars" -> 16, "1 Bar" -> 4, "6" -> 6, "6 beats" -> 6, "1/2" -> 0.5, "x" -> nullopt,
    "-2" -> nullopt.
  - Round trip: parse(label(v)) == v for every kSyncCycleBeats entry and for 1..64.
- How it is RED on main: the header is absent, so the test does not compile. Assertion-level REDs for the lane are in
  I2, I3, I8 and I10.
- GREEN: all cases pass.

I2. Top counter reads bar.beat
- ui/TopBarModel.h: replace barReadoutText with
  `barBeatReadoutText(bool hasBpm, uint16_t barCount, uint8_t beatInBar)`.
  - It returns "<barCount%4+1>.<beatInBar%4+1>", or "-.-" when there is no BPM.
  - Keep kBarsPerCount. The comment quotes both rulings.
- TopBar.cpp:538-542: pass displaySnap_.beatInBar.
- No layout or repaint change (V2, V1).
- Test: rewrite tests/test_topbar_model.cpp:
  - (0,0) -> "1.1"; (0,3) -> "1.4"; (3,3) -> "4.4"; (4,0) -> "1.1" (the wrap); (65535,1) -> "4.2"; no BPM -> "-.-".
  - Grid pin over barCount 0..15 and beat 0..3:
    - text == "1.1" exactly when beat==0 && barCount%4==0 (the FourBar rule, N1);
    - text is "1.1" or "3.1" exactly when beat==0 && barCount%2==0 (TwoBar).
- RED on main: compile failure, and the old "Bar N" asserts are replaced.
- GREEN: all cases pass.

I3. One sync-length list, plus "4 Bars" (connect/ConnPicker.cpp, ui/UniversalParamControl.cpp)
- ConnPicker.cpp:10-35:
  - kBpmDivisions becomes beatlen::kSyncCycleBeats (7 entries).
  - formatBeats becomes beatlen::label.
  - The clamp at :78-82 follows .size() automatically.
- UniversalParamControl.cpp, menu block :442-465:
  - Item texts come from beatlen::label(kSyncCycleBeats[d]).
  - Ids become 300 + shape*7 + d, i.e. 300..327 (400+ is untouched, V9).
- UniversalParamControl.cpp, handler :598-612:
  - Range becomes `< 300 + 4*7`; use /7 and %7.
  - sourceName_ = shape + " " + label.
  - Delete both local divisions[] arrays.
- Update the PickerChoice comment in UniversalParamControl.h.
- Tests to add in tests/test_conn_picker.cpp:
  - describeSource(LFO Square, 4 beats) == "Square 1 Bar". RED on main: it returns "Square 4 Beats".
  - BpmSync divIdx 6 -> cycleBeats 16. RED on main: the clamp gives 8.
  - describeSource(LFO, 16) == "Sine 4 Bars".
  - The existing "Square 1 Beat" stays.
- GREEN: all cases pass.

I4. SignalInspector
- Oscillator, :41-56:
  - Items come from label() over {0.25,0.5,1,2,4,8,16}; "4 Bars" is the new id 7.
  - durations[] gets 16.
  - Reverse map :258-267 gains `else if (bd <= 8.0f) sel = 6; else sel = 7;`.
- Envelope, :87-101: items come from label() over {1,2,4,8,16}. Values are unchanged.
- signal/OscillatorSignal.h:16: update the comment list.
- RED: I10.

I5. ClipInspector (constructor blocks only)
- :145-151 autopilot items: "Layer Determined" plus label() of {1,2,4,8,16,32}. Ids are unchanged.
- :204: the label becomes "Cycle".
- :211-217 beatDivision items: label() of {0.25,0.5,1,2,4,8,16}. Ids and values are unchanged.
- Content block :237-266:
  - Label: "Content".
  - textFromValueFunction = label.
  - valueFromTextFunction = parse, falling back to the slider's current value.
  - Text box 34 -> 52 px.
  - The snap list is unchanged.
- The inert Duration/"Beats" row (:90-98, :525-526) is NOT touched.
- RED: I10. The existing tests/test_clip_inspector_paint_key.cpp must stay green.

I6. LayerInspector :106-113
- Items: label() of {1,2,4,8,16,32}.
- Tooltip: "How long each clip plays before the next one".
- durToSel is unchanged.
- RED: I10.

I7. CompositionInspector setupCycleSlider (:93-103)
- Add textFromValueFunction = label and valueFromTextFunction = parse (fallback: the current value).
- Text box 30 -> 52 px.
- Range and step are unchanged: 1..64, 1-beat steps, still stored as int beats.

I8. MilkDrop
- Playlist (MilkDropBrowser.cpp:612-615): labels come from label() of {4,8,16,32}. getPlaylistTriggerBeats is
  unchanged.
- Jukebox (:563-575):
  - Items: "4 Bars"[1], "8 Bars"[2], "16 Bars"[3] (default, still id 3), "32 Bars"[4], "64 Bars"[5].
  - onChange uses bars[]={4,8,16,32,64}, idx < 5.
- PresetSelector:
  - PresetSelector.h:66: transitionBars_ = 16.
  - PresetSelector.cpp:169: `barsSinceLastSwitch_ >= transitionBars_` (drop the *4); fix the comment at :168.
- Every surviving choice switches exactly as today. "30 sec" (really 32 bars) merges into "32 Bars".
- Test: tests/test_preset_selector_bars.cpp (new exe).
  - Setup: a temp dir with 2 dummy presets (use the fixture approach of test_projectm_preset_manager.cpp) and
    ProjectMPresetManager::scanDirectory. PresetSelector with setPresetManager, setEnabled(true), Sequential mode and
    onAutoSwitch counting calls.
  - Input: a snapshot per frame with barPhase ramping 0 -> 0.95 and wrapping (one wrap = one crossing). The structural
    state stays constant.
  - setTransitionBars(4): exactly 1 switch, on the 4th crossing.
  - setTransitionBars(16): the first switch is on the 16th crossing.
  - RED on main: 0 switches after 4 crossings (main needs 16).
  - The builder first confirms that processFrame (:73-175) has no earlier path with constant structural input.
    kMinBarsBetweenSwitches only gates the structural path (:90).

I9. Grid labels
- ClipInspector.cpp:166-167: "2 Bars", "4 Bars".
- RoutineDeckView.h:278-279: "2 Bars", "4 Bars".
- Tooltips, the strings "2bar" / "4bar", REST and JSON are all unchanged.
- RED: I10.

I10. Label lint, tests/test_beat_label_lint.cpp (new; AUDIODNA_SRC_DIR)
- Scans src/**/*.{h,cpp,mm} line by line. It strips // comments and skips core/BeatLength.h.
- Inside string literals it flags:
  - (a) a whole-bar count written in beats: `\b(4|8|12|16|20|24|28|32|48|64)\s*[Bb]eats?\b`;
  - (b) a plural number with a singular bar: `\b([2-9]|[1-9]\d+)\s+Bar\b(?!s)`, so "2 Bar" is flagged and "2 Bars"
    is not;
  - (c) a literal that is just a unit used to build a number label at runtime: `"\s*[Bb]eats?"`.
- An explicit allowlist (file + substring + reason) exists and is EMPTY at merge.
- RED on main: it fails, listing ClipInspector.cpp:148-151, :166-167, :215-217; LayerInspector.cpp:108-111;
  SignalInspector.cpp:45-46, :89-91; MilkDropBrowser.cpp:563-566, :612-615; UniversalParamControl.cpp:449, :607;
  ConnPicker.cpp:28-29; RoutineDeckView.h:278-279. Keep that log in the lane report.
- GREEN: zero hits.
- Write it FIRST in Stage S2.

I11. Global Quantize gains bars, and its combo follows the model
- Composition.h:117: `enum class QuantizeMode : uint8_t { Off, NextBeat, NextDownbeat, NextTwoBars, NextFourBars };`
- Add next to it: `static Clip::BeatSnapMode forcedSnapFor(QuantizeMode, bool trackerLocked)`.
  - Off, or not locked -> Off; NextBeat -> Beat; NextTwoBars -> TwoBar; NextFourBars -> FourBar.
  - Anything else (NextDownbeat, or an unknown int) -> Bar.
- MainComponent.cpp:27-34: the body becomes
  `return Composition::forcedSnapFor(mode, snap.trackerState == BPMTracker::STATE_LOCKED);`. The signature and the 4
  callers are untouched.
- TopBar.cpp:177-181 items: "Off"[1], "Beat"[2], "Bar"[3], "2 Bars"[4], "4 Bars"[5]. onChange is unchanged.
- The combo is 100 px wide (:620), which fits.
- New private TopBar::syncQuantizeFromComposition(), called in timerCallback right after syncMasterFromComposition()
  (:332):
  - id = int(composition_.quantizeMode) + 1.
  - If id is in 1..5, differs from getSelectedId(), and !isPopupActive(): setSelectedId(id, dontSendNotification).
  - It never writes the model (the Pitfall 41 rule).
- Tests to add in tests/test_composition.cpp:
  - the forcedSnapFor table (including unknown 9 -> Bar);
  - JSON round trip of NextFourBars;
  - a file with no key loads Off (today no test pins this, recon D4).
- RED on main: compile failure. The behavioural RED is probe G4 on main.

BUILD STAGES (one builder context each, in dependency order):
- S1, "vocabulary + counter + sync list" (I1, I2, I3). SHIPS ALONE.
- S2, "every list and box" (I10 first, then I4-I9). Needs S1. Ships after S1.
- S3, "global Quantize bars" (I11, plus probe G4). Independent of S1/S2, SHIPS ALONE. It uses literal grid words, not
  label().
- Total size is small (about 300-400 lines). One builder may run S1 -> S2 -> S3 in sequence in one worktree.

FILES SHARED WITH OTHER LANES (bf7 edits only these blocks):
- ClipInspector.cpp constructor blocks :145-151, :161-167, :204-217, :237-266. The ui lane (BF3 codec row) builds
  before bf7. The resized() layout is NOT touched.
- LayerInspector.cpp :106-113 (string literals only). bf9b builds before bf7; deck/layer ownership is not touched.
- SignalInspector.cpp :41-56, :87-101, :258-267, :280-287. bf45 builds after bf7 and inherits beatlen.
- CompositionInspector.cpp :93-103.
- TopBar.cpp :176-189, timerCallback (one call), paintBarPhraseDisplay :531-544; TopBar.h one decl. bf2 adds the sync
  dial after bf7; bf7 does not touch resized().
- TopBarModel.h (whole file, 16 lines).
- MainComponent.cpp :27-34 only.
- Composition.h :117 plus one static next to it.
- ConnPicker.cpp :10-35 and UniversalParamControl.cpp :442-465, :598-612. bf6 (Timeline) builds before bf7 and may edit
  the adjacent Timeline branch (ConnPicker.cpp:89-100) and the same menu builder.
- MilkDropBrowser.cpp, PresetSelector.*, RoutineDeckView.h: no other lane.
- tests/CMakeLists.txt: append new exes at the end.
- tsan-r5 (after bf7) may turn quantizeMode into Relaxed<T>; the sync line then gains .load().
- bf1 / bf45 planners: use beatlen::label for any length list (the lint will reject "16 beats" literals).

## 5. GATES (Harmony runs these after each merge)
- G1. Release build: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)`. Zero new warnings in the touched
  files.
- G2. Tests:
  - New: test_beat_length, test_beat_label_lint, test_preset_selector_bars.
  - Changed: test_topbar_model, test_conn_picker, test_composition.
  - Then the full ctest -j.
  - Rule: 0 failures. A pre-existing flake is waived only if it fails identically on main eff2b1c.
- G3. The lint RED log from main is in the lane report, and the lint is green on the merge.
- G4. Probe row "quantize-bars" (S3). Copy the .harmony/probe-routines.sh header: manual LOCKED tracker at 120 BPM
  (beat 0.5 s, bar 2 s), --test-mode, Connection: close.
  - Fixture: a composition with quantizeMode 4, loaded by POST /api/load_composition. Layer 0 has two image clips.
  - Wait until GET /api/bpm reads barCount%4==1, so the next bar line is NOT a 4-bar line. Then fire column 1 through
    the clip-trigger route the existing probes use (the builder names it).
  - Poll the layer's active column and /api/bpm every 50 ms.
  - Pre-registered bars:
    - P1: the switch happens 5.5-8.3 s after the fire.
    - P2: the first poll after the switch reads barCount%4==0 and beatInBar==0.
    - P3: with quantizeMode 3, the switch is <= 4.3 s and barCount%2==0.
    - P4 (control arm): with Off, the switch is <= 0.3 s.
  - On main, P1 and P2 FAIL (a 4 falls back to Bar, so the switch is about 2 s later at barCount%4==2). That is the
    behavioural RED.
- G5. VISUAL WORK GATE (all S1/S2/S3 UI). Nothing is shown to Boris before the critics pass.
  - Headless tool, tests/tool_beat_labels_snapshot.cpp (pattern: tests/tool_routine_deck_snapshot.cpp). It renders to
    PNG, with the model in the target state:
    - ClipInspector with a BPM-sync clip: Cycle 1 Bar, Content 2 Bars, autopilot 4 Bars, snap 2 Bars;
    - LayerInspector: On Beat, 8 Bars;
    - CompositionInspector, per-type on: 4 Bars / 2 Bars / 1 Bar;
    - SignalInspector: oscillator 4 Bars, envelope 1 Bar;
    - the MilkDrop timing combos;
    - TopBar at (0,0), (3,3), (6,1) and no lock.
    It also writes a text dump of every changed combo's item list.
  - Live: ONE Audio-DNA (lock helper), launched with open -g ... --args --test-mode and loaded with the G4 fixture.
    Take main-window captures by Quartz window id only (never full screen, never an Output window), 0.5 s apart. They
    must show the counter advancing and the Quantize combo reading "4 Bars" straight from the file. On main it reads
    "Off", which is the RED of the sync.
  - Critic panel: visual-design, UX, graphic-design and logic critics, plus a dedicated interaction-logic critic for
    the combo-follows-model behaviour and the counter wrap/restart.
  - BLOCK rules (pre-registered):
    - any truncated label in a closed control;
    - any whole-bar value shown in beats;
    - any counter showing a bar above 4 outside the routine pads;
    - "-.-" missing when there is no lock.
  - After the critics pass: an artifact page Boris opens.
- G6. Perf: none required. There is no hot-path change, and V2 shows the counter rides the existing repaint. If
  Harmony wants it: topBarWheelRepaints per second, interleaved A/B, at least 5 runs per arm. INFO only.

## 6. DOCS
- docs/claude/performance-controls.md (beat snap / quantize / Live Performance Controls):
  - the global Quantize list and the follow-the-model rule;
  - the bar.beat counter and the reconciliation sentence;
  - phrase restarts (drop, breakdown exit, Resync) restart the bar digit, the same edge as 2/4-bar quantize;
  - Link quantum = 1 bar; bar lines always come from our tracker.
- docs/claude/effects.md "Autopilot System": lengths read in bars; per-type boxes accept "N bars"; a bare number is
  beats.
- docs/claude/architecture.md Naming Conventions:
  - core/BeatLength.h is the only source of beat/bar labels (test_beat_label_lint);
  - a "Time signature: 4/4 only" note with the N3 site list.
- The doc that covers the MilkDrop jukebox (grep docs/claude for "jukebox"): timing is in bars, and setTransitionBars
  takes bars.
- APP-INVENTORY.md: the changed controls' value lists.
- BORIS_DECISIONS.md: add "Bars wherever beats (BF7, 2026-10-02)" with the counts-vs-lengths reconciliation. Annotate
  the 2026-09-26 bar-counting bullet "resolved by BF7: the bar digit still runs 1-2-3-4-1".
- CLAUDE.md: NO change (0 bytes; the lint enforces the rule).
- New pitfalls: none.

## 7. RISKS (each with the cheapest discriminating test)
- R1. "3.2" could be read as a decimal. Test: G5 critics plus Boris. The fallback is a one-function change in
  TopBarModel.h.
- R2. Labels could truncate in narrow controls. Test: the G5 snapshot tool's BLOCK rule. The 52 px boxes are measured
  there.
- R3. Autopilot "N Bars" counts from the clip's start (N2), so it is bar-aligned only if the clip started on a bar.
  Test: Q2 to Boris. Behaviour is unchanged by bf7.
- R4. A global 4-bar wait can reach 8 s at 120 BPM. Test: G4 timing. The pending-clip indicator must be visible
  (interaction critic). The "not LOCKED -> immediate" rule stays (V5).
- R5. Merge conflicts in shared files. Mitigation: only the named blocks are edited; rebase onto ui, bf6 and bf9b
  before S1.
- R6. The lint will fail any future lane that writes "N beats". This is intended; tell the bf1 and bf45 planners.
- R7. Jukebox off-by-one. Test: the I8 test pins exact crossing counts (4 and 16).
- R8. STRONGEST COUNTERARGUMENT: "relabel only; leave the counter and Quantize alone; smallest diff". It loses because
  "every beat setting also offers bar values" then fails exactly at the global Quantize. Also, both extensions stay at
  4 bars or less, so the 2026-09-26 ruling holds under any reading. They are separable (I11; I3's 7th entry), so
  Harmony can cut them alone.

## 8. WHAT ONLY BORIS CAN CHECK (live rig)
- The counter "3.2" reads at a glance in a dark room. The bar digit snapping back to 1 at a drop feels right.
- "1 Bar / 2 Bars / 4 Bars" wording in every list (Q1).
- Global Quantize 2 Bars / 4 Bars: the wait after a press feels musical, not laggy.
- Jukebox and playlist pacing sound unchanged by ear.

## 9. QUESTIONS FOR BORIS (plain words; each has a default, so the build never blocks)
- Q1. Settings that are whole bars will read "1 Bar", "2 Bars", "4 Bars". Do you want just that, or both numbers,
  like "16 Beats (4 Bars)"? DEFAULT: just bars.
- Q2. Autopilot "change every 4 bars" counts from the moment the clip started. If a clip starts mid-bar, its changes
  land mid-bar too. Should autopilot changes always wait for the start of a bar? DEFAULT: leave it as is for now
  (clips fired with the Bar snap already start on the bar); it is a small follow-up if you want it.
- Q3. In the MilkDrop jukebox, "30 sec" and "60 sec" were never seconds: they changed presets every 32 and 64 bars.
  They will read "32 Bars" and "64 Bars". Do you also want real time-based choices? DEFAULT: no, bars only.
- Time signature: not asked. 4/4 only; other meters are not cheap (F9).

STATUS: FINAL

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 16:47:46) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-bf7.md IN FULL (AM1-AM18; stages S0 -> S1 -> S2 -> S3 -> S4, then G7,
   then G5; gates G1, G2, G3, G5, G6, G7; G4 deleted). Base = main at launch.
2. BORIS QUESTIONS — defaults until he answers: Q1 keep "Bar 1-4" + the beat wheel; Q2 no 2 / 4-bar top-bar Quantize now;
   Q3 autopilot keeps counting from the clip start; Q4 "N Bars" only; Q5 MilkDrop jukebox in bars only.
3. QUEUED: build after the in-flight lanes (mkvidx, ui, bf2) and bf10; it shares ClipInspector / LayerInspector /
   SignalInspector / TopBar with ui, bf2 (S5 later), bf45 and bf9b — rebase onto whatever main is at launch.
4. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b
   incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release.
   Visual gate (critic panel, four named critics reading every image) runs on Harmony's decoded captures before Boris
   sees anything. MERGE by Harmony.
