# RULING bf7: bars wherever the app has beats (s-rta-1002b)

Plan (unchanged on disk): .harmony/.reports/s-rta-1002b/plan-bf7.md. Seat papers (verbatim): .harmony/.reports/s-rta-1002b/attack-bf7-papers.md.
Base read: main c6720a0 working tree. Paths are relative to src/ unless they start with tests/, docs/, .harmony/ or build/.

What this ruling verified (cited at each use):
- source reads of TopBar.cpp/.h, TopBarModel.h, MainComponent.cpp/.h, ClipInspector.cpp, CompositionInspector.cpp, RoutineDeckView.h,
  model/Autopilot.cpp, model/Layer.h, model/Clip.h, render/Renderer.cpp, MilkDropBrowser.cpp, sources/PresetSelector.h,
  connect/ConnSerialization.cpp and UniversalParamControl.cpp;
- the vendored JUCE (build/_deps/juce-src) for the slider and label edit paths;
- a scratch run (outside the repo) of the plan's lint regexes and of the narrowed ones over every string literal in src/;
- a sweep of all 110 string literals in src/ that contain the word beat or beats.

## 0. VERDICT
ready_to_build: YES, with 18 amendments (AM1-AM18). The plan's core stands: one vocabulary, plus a relabel of every length to bars.
Three things change materially:
- (1) The top-bar counter is NOT changed.
  - "bar.beat" was Harmony's gloss, not Boris's words, and the counter already counts bars.
  - Boris gets a question (Q1); the default keeps the counter as it is.
- (2) The global Quantize gets only two changes: the relabel ("Next Bar") and the fix that makes the combo follow the model.
  - Its new 2-bar and 4-bar values become Boris question Q2 (default: not now).
  - That also removes the 8-second wait and the probe that could not fail.
- (3) Typing a length follows the screen.
  - A typed plain number uses the unit the box shows.
  - The per-type +/- buttons walk 1 Beat, 2 Beats, then whole bars.
Also:
- A missed live control ("Beats per Image") joins the lane.
- The dead "Beats/Duration" row in the clip panel is removed.
- The lint is rebuilt so it can actually go green.
- The visual gate gains a measured text-width test (G7) and named critic seats.

## 1. ATTACK DISPOSITIONS

### UX seat (bf7-ux-attack)

UX-A1 [MUST] ACCEPT -> AM1.
- Boris's words are not "bar.beat":
  - backlog:30 heads the item list "Harmony's reading — consequence text, not Boris's words".
  - Boris's BF7 words are only backlog:23, "Add bars to all places we have beats".
- The 09-26 ruling is still open: BORIS_DECISIONS.md:334-335 says "interpretation being confirmed with Boris".
- The counter already counts bars 1-4:
  - TopBarModel.h:13-16 returns "Bar " + (barCount % 4 + 1).
  - It is drawn at TopBar.cpp:531-544 in 11 pt, inside a 44 px box (TopBar.cpp:578), right after the 26 px beat wheel.
- So the top bar already has bars where it has beats. bar.beat adds a beat NUMBER that nobody asked for, on a display whose
  ruling is unconfirmed.

UX-A2 [MUST] ACCEPT -> AM2.
- Typing replaces the unit. When a slider's text box is edited, JUCE selects the whole text
  (build/_deps/juce-src/modules/juce_gui_basics/widgets/juce_Label.cpp:243), so the first key replaces "4 Bars", unit and all.
- The committed text then goes through getValueFromText and snapValue (juce_Slider.cpp:447).
- Every typed box defaults to a whole bar:
  - Content: 4 beats (the ClipInspector.cpp:237-266 block, setValue/setDefaultValue 4.0);
  - Opaque 16, Transparent 8, Effect 4 (CompositionInspector.cpp:105-115).
- So under plan F7 ("bare = beats"), a plain number contradicts the screen in every default state.

UX-A3 [MUST] ACCEPT -> AM3.
- The boxes are IncDec, range 1..64, step 1 (CompositionInspector.cpp:93-103).
- JUCE's +/- buttons call incrementOrDecrement(+/-interval), which calls owner.snapValue(getValue() +/- interval, notDragging)
  (juce_Slider.cpp:410-414, :631).
- snapValue is virtual (juce_Slider.h:852). So a ladder that steps in bars is a one-method override.

UX-A4 [MUST] ACCEPT -> AM4.
- The row is dead in every mode:
  - durationSlider_ has no onValueChange and no model write (ClipInspector.cpp:89-98).
  - Its "/2" and "x2" buttons have no onClick: they appear only at :102-103 (setup) and :632-634 (bounds).
  - Nothing else in src/ or tests/ references the three members (grep).
- Its painted label reads "Beats" in BPM-sync mode (:525-528).

UX-A5 [MUST] ACCEPT in part -> AM5 (gate G7), AM8, finding H1.
- (1) Measured text width: ACCEPT. Gate G7 is a pixel measurement with a negative control.
- (2) Pending marker: no UI file reads the pending trigger.
  - A grep of src/ui for pendingTriggerColumn, isPending and PendingTrigger finds zero hits. The state lives only in
    Layer.h:49-50 and :415-416.
  - AM8 removes the new 2-bar and 4-bar global values, so bf7 creates no new wait: the global combo's longest wait stays one bar.
  - The 8 s waits that exist today come from the per-clip "4 Bar" snap, which bf7 leaves unchanged.
  - So this is a FINDING (H1), and a mandatory item of the Q2 follow-up (AM16). It is not a bf7 item.
- (3) Counter legibility: moot, because AM1 leaves the counter unchanged.

UX-A6 [SHOULD] ACCEPT, with the names corrected -> AM6.
- The seat's proposed names are swapped relative to the code.
- beatDivision is how long one pass of the clip takes:
  - "BPM Sync: adjust speed so video loops in beatDivision beats" (render/Renderer.cpp:1816-1822);
  - "cycle through images over beatDivision beats" (:1882-1892).
- videoBeats is the content's own length at normal speed (model/Clip.h:39-41).
- So beatDivision = "Loop length" and videoBeats = "Content length".

UX-A7 [SHOULD] ACCEPT in part -> AM7, Q3.
- ACCEPT the tooltip. Every autopilot length counts from the clip's start: clip->beatsPlayed accumulates per clip
  (model/Autopilot.cpp:97-98) and is zeroed at activation (model/Layer.h:540).
- REJECT relabelling to "every 16 beats from start". It reverses Boris's request, and "4 Bars" is still a true length.
- REJECT aligning to bar lines in this lane. That is a behaviour change, so it goes to Boris as Q3 (default: leave).

UX-A8 [SHOULD] ACCEPT -> AM8, Q2.
- The global combo gets the relabel to "Next Bar" and the follow-the-model fix, nothing more.
- The 2-bar and 4-bar extension is separable by the plan's own text (plan F4, R8).
- Keeping the word "Next" keeps the cue that this is a wait.
- The armed value is shown by the combo itself, and the combo is now truthful because it follows the model.

UX-A9 [SHOULD] ACCEPT (verified, plus a test) -> AM9.
- sourceName_ is rebuilt from the connection on every refresh: sourceName_ = describeSource(conn_->source)
  (ui/UniversalParamControl.cpp:136). Otherwise it is assigned only at :551, at :559 and in the menu handler.
- The 300+ ids exist only inside the popup.
- A connection saves numbers: cycleBeats is stored as a double (e.g. connect/ConnSerialization.cpp:125).
- So nothing saved or bound depends on a label or an id.
- The one thing that can break is the menu tick, which compares sourceName_ with the item's full name
  (UniversalParamControl.cpp:461). AM9 pins it.

UX-A10 [SHOULD] ACCEPT -> AM10.
- Today, ui/MilkDropBrowser.cpp:563-575 maps ids 1..6 to setTransitionBars {1,2,4,8,8,16}, with default id 3.
- sources/PresetSelector.h:66 sets transitionBars_ = 4, which means 16 bars because of the x4.

UX-A11 [NIT] PARTIAL -> AM11.
- "-.-": moot (AM1).
- Brittle regex (b): ACCEPT. The lookahead is redundant after \b.
- Scoping the lint to UI call sites: REJECT.
  - The scratch run shows the narrowed rules hit zero non-UI literals on main.
  - Call-site scoping would miss real hits that live in tables, not calls: RoutineDeckView.h:278-279 (a struct table) and
    ConnPicker.cpp:24-29 (an array).

### Gates seat (bf7-gates)

G-A1 [MUST] ACCEPT -> AM12.
- The control is live:
  - MainComponent.cpp:366 sets the label "Beats per Image".
  - :464-477 fills the items at runtime as juce::String(b) for b = 2..128. The default is id 4 (8 beats); slideshowBeats_
    is set at :475.
  - :2621-2622 lays it out, and it is not in the hide list at :2602-2612.
- Correction to the seat's FIX: the values are 2, 4, 8, 16, 32, 64 and 128 beats. So the items are "2 Beats", "1 Bar",
  "2 Bars", "4 Bars", "8 Bars", "16 Bars" and "32 Bars", not "1 Beat .. 4 Bars, 8 Bars".
- Independent sweep, done in this ruling: 110 literals in src/ contain beat or beats. Apart from this control, every one is
  one of:
  - an item the plan already covers;
  - an event word: "on the next beat", or LayerInspector.cpp:45-48 "on beat";
  - a feature or signal name: "Beat Position", "Beat In Bar", "Beat Phase", "Beat Ripple", "Beat Sensitivity";
  - a hidden v1 control: MainComponent.h:369 and .cpp:2610-2611, or the hidden AudioReadoutPanel (plan V15);
  - a REST or JSON enum string: ApiServer.cpp:2180, :2222, :2313, ConnSerialization.cpp:124, recording/*.
- The only other list built from numbers at runtime is beatCountSelector_, which is hidden.

G-A2 [MUST] ACCEPT (rules rebuilt), with one sub-part rejected -> AM11.
- A scratch run of the plan's rules on main shows they can never go green:
  - Rule (c) hits 22 literals. These include JSON and REST enum strings ("beats" at ConnSerialization.cpp:124; "beat" at
    ApiServer.cpp:2180, Lane.h:96, Routine.h:92 and TempoMap.cpp:8) and UI words the plan keeps ("Beat" at
    ClipInspector.cpp:164 and RoutineDeckView.h:276).
  - Rule (a) also flags the kept label "1/4 Beat" at 5 sites (ClipInspector.cpp:211, SignalInspector.cpp:41,
    UniversalParamControl.cpp:449 and :607, ConnPicker.cpp:24), because "\b4 Beat" matches inside "1/4 Beat".
- With the narrowed rules (AM11), main has exactly 36 hits (30 + 4 + 2, listed in G3). S1 and S2 convert every one of them.
- REJECT the seat's "allowlist non-empty by design": no legitimate literal matches the narrowed rules, so the allowlist is
  empty at merge, honestly.

G-A3 [MUST] ACCEPT. The arithmetic is right.
- On main, NextDownbeat maps to Bar (MainComponent.cpp:27-34). A fire at barCount%4==1 lands at barCount%4==2, which is
  even, within about 2 s. So P3 passes on main.
- The rig is described in the .harmony/probe-routines.sh header:
  - production mode, port 7070, "NO --test-mode" (the plan's "--test-mode" was also wrong);
  - a manual LOCKED tracker, where "Structural resets are suppressed ... barCount and totalBarCount advance together".
- AM8 makes this moot in bf7. The corrected probe is pre-registered for the Q2 follow-up (AM16).

G-A4 [MUST] ACCEPT.
- For the counter it is moot: AM1 deletes I2.
- The same wiring risk exists for the Quantize combo.
  - The sync function is tested directly by T7. This follows the precedent of tests/test_master_opacity_link.cpp:79, which
    calls bar.syncMasterFromComposition().
  - The timer wiring is proven live by G5: the combo reads the loaded file's value. TopBar inherits juce::Timer privately
    (TopBar.h:12), so a test cannot fire its timer.

G-A5 [SHOULD] ACCEPT -> AM13, T1.

G-A6 [SHOULD] ACCEPT -> AM14 (T5, T6, T7). One part is moot: "JSON back-compat of QuantizeMode 3/4", because AM8 leaves the
enum unchanged.

G-A7 [SHOULD] ACCEPT -> AM5 (G7) and AM15 (named critic seats; each Reads every PNG and gives a verdict per image).

G-A8 [SHOULD] ACCEPT -> AM1 and Q1. Harmony marks backlog:52-53 "beat counters read bar.beat" as its own consequence text (the
architect cannot edit the backlog).

G-A9 [NIT] ACCEPT -> AM7. Re-read done: Autopilot.cpp:97-98 and Layer.h:540 are verified.

### Tally (20 attacks)
- ACCEPTED in full: UX-A1, UX-A2, UX-A3, UX-A4, UX-A6, UX-A8, UX-A9, UX-A10; G-A1, G-A3, G-A4, G-A5, G-A6, G-A7, G-A8, G-A9.
- ACCEPTED in part: UX-A5, UX-A7, UX-A11, G-A2.
- REJECTED sub-parts:
  - UX-A7: relabelling as beats, and aligning to bar lines in this lane;
  - UX-A11: scoping the lint to call sites;
  - G-A2: "allowlist non-empty by design".

## ARCHITECT RULING (s-rta-1002b)

### A. AMENDMENTS (they override the plan body; where they conflict, the amendment wins)

AM1 — The top-bar counter is NOT changed (UX-A1, G-A8, G-A4).
- Delete I2 entirely. TopBarModel.h, TopBar.cpp paintBarPhraseDisplay and tests/test_topbar_model.cpp stay untouched.
- The counter keeps "Bar 1".."Bar 4" and "Bar -".
- Strike from the plan:
  - §1 "The top counter reads bar.beat", and the block "HOW BOTH RULINGS HOLD (no question to Boris needed)";
  - R1, the G5 TopBar counter frames, the "-.-" BLOCK rule, and the §8 line about "3.2".
- F1 CHOSEN becomes (b): keep "Bar N"; the wheel shows the beat.
- Inventory row 1 becomes "unchanged".
- Boris Q1 asks, with a mockup, whether he wants a beat number.
  - If he says yes, the follow-up passes the counter its values as a struct (G-A4) and reuses the plan's I2 grid-pin test.
- BORIS_DECISIONS: do NOT annotate the 2026-09-26 bar-counting bullet as resolved. It stays "being confirmed" until Q1 is
  answered.

AM2 — A typed length uses the unit the box shows (UX-A2, G-A5, G-A6).
- I1's parse becomes `std::optional<double> parse(std::string_view text, double bareUnitBeats)`.
  - It trims the text and ignores case.
  - The grammar is: a number, then optional spaces, then an optional unit.
    - The number is a decimal or a fraction a/b.
    - The unit is beat, beats, bar or bars. A bar unit multiplies by 4.
    - With no unit, the number is multiplied by bareUnitBeats.
  - It returns nullopt for:
    - empty or blank text;
    - a unit alone ("bars");
    - an unknown unit ("4 bat");
    - trailing junk;
    - zero or a negative number;
    - "1/0";
    - the text "nan" or "inf".
- Add `bool isWholeBars(double beats)`: true when beats >= 4 - 1e-4 and beats/4 is within 1e-4 of an integer. label() rule 2
  calls this same function.
- Each typed box sets:
  `valueFromTextFunction = [&s](const juce::String& t) { return beatlen::parse(t.toStdString(), beatlen::isWholeBars(s.getValue()) ? 4.0 : 1.0).value_or(s.getValue()); }`
  - So a plain number means the unit on screen at that moment.
  - Text that cannot be read changes nothing.
- Out-of-range input is clamped by Slider::setValue, and the box re-renders the clamped label. The clamp is therefore visible;
  T5 pins it.
- Each typed box gets the tooltip "Type 2 bars or 6 beats. A plain number uses the unit shown." AM6 puts the row's own meaning
  in front of it.
- Plan F7 is reversed: a bare number no longer always means beats.

AM3 — The per-type +/- buttons walk in bars (UX-A3). This replaces I7.
- Add a new header, ui/BarStepSlider.h: `class BarStepSlider : public ResettableSlider`.
- In CompositionInspector.h, the Opaque, Transparent and Effect members become BarStepSlider.
- Range 1..64 and interval 1 are unchanged; values are still stored as int beats.
- textFromValueFunction = beatlen::label. valueFromTextFunction follows AM2.
- The ladder L = {1, 2, 4, 8, 12, ..., 64}: 18 values, reading 1 Beat, 2 Beats, 1 Bar, 2 Bars, 3 Bars ... 16 Bars.
  - It is a pure function in core/BeatLength.h: `double stepLadder(double current, int dir, double lo, double hi)`.
  - dir +1 returns the smallest L value greater than current.
  - dir -1 returns the largest L value less than current.
  - The result is clamped to [lo, hi].
- Overrides:
  - getValueFromText(t): store lastTyped_ = the AM2 parse result, set typed_ = true, and return lastTyped_.
  - snapValue(v, mode):
    - If typed_ && v == lastTyped_: set typed_ = false and return v. Typed values are exact.
    - Otherwise set typed_ = false, then:
      - if mode == notDragging && v != getValue(): return stepLadder(getValue(), v > getValue() ? +1 : -1, getMinimum(), getMaximum());
      - else return v.
- The JUCE paths are verified in the vendored source: the +/- buttons at juce_Slider.cpp:410-414 and :631, typed text at :447.
- A value off the ladder (from a file or REST, e.g. 6) still reads truthfully as "6 Beats", because setValue never calls
  snapValue. From there "+" goes to 8 ("2 Bars") and "-" goes to 4 ("1 Bar").
- The text box grows from 30 px to the measured need. Start at 52 px; G7 pins the final width.
- The unused `label` argument of setupCycleSlider ("Opaque Beats" etc., CompositionInspector.cpp:105-115) is not touched. It is
  never shown, and the lint does not flag it.

AM4 — Remove the dead Duration/"Beats" row from ClipInspector (UX-A4). This is new item I13.
- Delete:
  - the members durationSlider_, durHalfBtn_ and durDoubleBtn_ (ClipInspector.h);
  - their constructor code (ClipInspector.cpp:89-98 and :102-103);
  - their layout block and its y advance (:628-638);
  - the painted "Beats"/"Duration" label and its ty advance (:525-528).
- The rows below move up by exactly one kRowHeight. Change paint() and resized() together.
- Any height total that counts this row loses one row. The builder greps kRowHeight sums and any preferred-height function.
- Precondition: the builder's grep shows no other reference to the three members in src/ or tests/. This ruling's grep found
  none.
- This overrides plan §4's "resized() layout is NOT touched". The ui lane builds first, and bf7 edits the rebased code.
- Note for Boris (not a question): "the Beats/Duration row in the clip panel never did anything; it is gone."

AM5 — The visual gate becomes mechanical (UX-A5, G-A7).
- Gate G7 (section D) is added.
- There is no bf7 pending-marker item (UX-A5(2)), because AM8 removes the only new long wait.
- The missing pending marker is finding H1, and it is a mandatory item of any Q2 follow-up.

AM6 — Row labels say what they measure, and each has a tooltip (UX-A6, G-A1). This replaces the label parts of I5 and I6.
- ClipInspector:
  - "Beats/ Cycle" (:204) becomes "Loop length".
    Tooltip: "How long one pass through the clip takes. The clip plays faster or slower to fit."
  - "Content Beats" (:237) becomes "Content length".
    Tooltip: "How long the clip's own content lasts at normal speed. Type 2 bars or 6 beats. A plain number uses the unit shown."
  - The autopilot length combo gets a new tooltip:
    "How long this clip plays before autopilot moves on, counted from when it starts."
- LayerInspector count combo tooltip, replacing "Number of beats before advancing" (:113):
  "How long each clip plays before the next one, counted from when the clip starts."
- CompositionInspector per-type boxes each get a tooltip:
  "How long each opaque / transparent / effect clip plays before autopilot changes it, counted from when the clip starts.
  Type 2 bars or 6 beats. A plain number uses the unit shown."
- MainComponent: "Beats per Image" becomes "Image every".
  Tooltip: "How long each image shows in a folder slideshow."
- If a label does not fit its column at its font (G7), widen that label column by the pixels needed and narrow its control by
  the same amount. Never abbreviate. G7 then pins both widths.

AM7 — Autopilot lengths stay in bars and say where they count from (UX-A7, G-A9).
- The tooltips follow AM6.
- No behaviour changes.
- Aligning changes to bar lines is Boris Q3 (default: leave).

AM8 — Global Quantize: relabel and follow the model only (UX-A8). This replaces I11.
- TopBar.cpp:178-180 items become "Off"[1], "Next Beat"[2], "Next Bar"[3]. onChange is unchanged.
- Add a PUBLIC TopBar::syncQuantizeFromComposition(), declared next to syncMasterFromComposition (TopBar.h:52).
  - Call it in timerCallback right after syncMasterFromComposition().
  - It computes id = int(composition_.quantizeMode) + 1.
  - If 1 <= id <= 3, id differs from getSelectedId(), and !isPopupActive() (juce_ComboBox.h:279), it calls
    setSelectedId(id, juce::dontSendNotification).
  - It never writes the model (Pitfall 41).
- bf7 does NOT:
  - change the QuantizeMode enum (Composition.h:117-118);
  - add forcedSnapFor;
  - touch MainComponent.cpp:27-34;
  - keep the I11 test_composition cases (deleted);
  - keep probe G4 (deleted).
- F4(i) moves to Boris Q2 (default: not now).
- Strike from the plan text: R4, and the global-wait parts of R8 and §8.
- The tsan-r5 note stays: if quantizeMode becomes Relaxed<T>, the sync line gains .load().

AM9 — The sync-list menu ticks stay consistent (UX-A9). In tests/test_conn_picker.cpp, in addition to I3's cases:
- For each of the 4 LFO shapes and each of the 7 kSyncCycleBeats entries:
  describeSource(LFO{shape, beats}) == <shape name> + " " + beatlen::label(beats).
  That is the exact string the BPM Sync menu compares at UniversalParamControl.cpp:461.
- A connection with cycleBeats 4, as main saves it, reads "Square 1 Bar".
- No persistence change (verified under UX-A9).

AM10 — Jukebox: the old-to-new table, with its default pinned (UX-A10, G-A6). This replaces I8's test text.

| old id | old label | real bars on main | new id | new label | real bars |
|---|---|---|---|---|---|
| 1 | 4 beats | 4 | 1 | 4 Bars | 4 |
| 2 | 8 beats | 8 | 2 | 8 Bars | 8 |
| 3 (default) | 16 beats | 16 | 3 (default) | 16 Bars | 16 |
| 4 | 32 beats | 32 | 4 | 32 Bars | 32 |
| 5 | 30 sec | 32 | merged into id 4 | 32 Bars | 32 |
| 6 | 60 sec | 64 | 5 | 64 Bars | 64 |

- The choices live once, in sources/PresetSelector.h:
  `static constexpr std::array<int,5> kJukeboxBarChoices{4,8,16,32,64}; static constexpr int kDefaultJukeboxChoice = 2;`
- MilkDropBrowser builds its items from kJukeboxBarChoices, using beatlen::label(bars * 4).
- transitionBars_ defaults to kJukeboxBarChoices[kDefaultJukeboxChoice], which is 16.
- PresetSelector.cpp compares barsSinceLastSwitch_ >= transitionBars_, as in the plan.
- The setting is not saved, so no file migration is needed (plan V7).
- Tests: T6.

AM11 — The label lint is rebuilt so it can go green (G-A2, UX-A11). This replaces I10's rules.
- Scope:
  - every string literal in src/**/*.{h,cpp,mm};
  - excluding // comments and block-comment lines;
  - skipping core/BeatLength.h;
  - NOT limited to particular call sites.
- Rules (std::regex ECMAScript):
  - (a') `(^|[^/0-9.])(4|8|12|16|20|24|28|32|48|64)\s*[Bb]eats?\b` catches a whole-bar length written in beats. It no longer
    fires inside "1/4 Beat".
  - (b') `\b([2-9]|[1-9][0-9]+)\s+Bar\b` catches a plural count with a singular "Bar". No lookahead is needed.
  - (c') catches a literal that is exactly `^\s+[Bb]eats?$`: a unit suffix glued onto a number at runtime.
- The lint test also runs these self-test cases against the three regexes (this ruling ran them in the scratch rig):
  - must NOT match: "1/4 Beat", "3/4 Beat", "2 Bars", "1 Bar", "Beat", "beats", "Next Beat", "1.5 Beats", "64 Bars";
  - MUST match: "16 Beats" (a'), "4 Beats (1 Bar)" (a'), "2 Bar" (b'), " Beats" (c').
- The allowlist mechanism (file + literal + reason) exists and is EMPTY at merge.
- The RED list on main is in G3.
- The lint is written first in S2.

AM12 — The slideshow selector speaks bars (G-A1). This is new item I12 and inventory row 23.
- MainComponent.cpp:
  - The label at :366 becomes "Image every", with the AM6 tooltip.
  - The items at :464-469 come from beatlen::label over {2,4,8,16,32,64,128}.
  - Unchanged: ids 1..7, default id 4 (8 beats, now shown as "2 Bars"), and the slideshowBeats_ values (:472-475).
- Widths at :2621-2622:
  - The selector (50 px today) widens to fit its widest item, as measured.
  - The label (80 px) stays if "Image every" fits.
- MainComponent cannot be built headless. G7 therefore measures a replica ComboBox with the same LookAndFeel, width and items.
  The live capture in G5 is the real check.

AM13 — The vocabulary gets an assertion-level RED (G-A5).
- S1's first commit adds core/BeatLength.h as stubs:
  - label returns "";
  - parse returns nullopt;
  - isWholeBars returns false;
  - stepLadder returns current.
- test_beat_length then compiles and FAILS on its assertions. The next commit implements the header.
- Round-trip claims are limited to the grid (T1). Nothing is claimed for off-grid fractions such as 1.333.

AM14 — Extra tests (G-A6):
- T5: typed boxes, through the real text path.
- T6: the jukebox default.
- T7: the Quantize combo follows the model. It calls the public sync function, following the precedent of
  tests/test_master_opacity_link.cpp:79.
- Harness precedent: tests/test_topbar_link_toggle.cpp:53 (`TopBar bar(bus, comp);`).

AM15 — The critic seats are named and must read the pictures (G-A7). See G5.

AM16 — Pre-registered for the Q2 follow-up only; nothing here is built in bf7 (G-A3). If Boris says yes to global 2-bar and
4-bar Quantize, that lane must use:
- Rig: the .harmony/probe-routines.sh mechanics.
  - production mode, port 7070, NO --test-mode;
  - POST /api/set_bpm 120 for a manual LOCKED tracker (one bar = 2.0 s).
- Precondition: barCount advances exactly once per 2.0 +/- 0.1 s for 3 bars before any fire. Otherwise the run is
  INCONCLUSIVE, which counts as a FAIL.
- Fire phase: always fire within 100 ms after an observed bar line.
- 4-bar arm:
  - Fire just after a line with barCount%4==1.
  - The switch must land at 6.0 s (-0.3 / +0.6).
  - The first poll after the switch must read barCount%4==0 and beatInBar==0.
- 2-bar arm: fire just after a line with barCount%4==0.
  - Control: Next Bar mode switches at 2.0 s (-0.3 / +0.6) with an odd barCount.
  - Test: 2-bar mode switches at 4.0 s (-0.3 / +0.6) with an even barCount.
  - On main this arm FAILS: main switches at about 2 s, with an odd barCount.
- Off arm: the switch happens within 0.3 s.
- That lane must also build the pending-cell marker (H1).

AM17 — The shared-files list is updated. Where it differs, it replaces the plan's.
- TopBar.cpp: only :178-180 and one call in timerCallback.
- TopBar.h: one public declaration.
- TopBarModel.h: untouched.
- MainComponent.cpp: only :366, :464-469 and :2621-2622. :27-34 stays untouched.
- Composition.h: untouched.
- ClipInspector.cpp: the plan's constructor blocks, plus AM4's paint and resized edits, plus any AM6 label-width change.
- ClipInspector.h: 3 member lines removed.
- CompositionInspector.h/.cpp: 3 member types, plus setupCycleSlider.
- New files: ui/BarStepSlider.h and core/BeatLength.h.
- As in the plan: PresetSelector.h/.cpp, MilkDropBrowser.cpp, RoutineDeckView.h, SignalInspector.cpp, LayerInspector.cpp,
  ConnPicker.cpp and UniversalParamControl.cpp.
- bf1 and bf45 planners: use beatlen::label for any length list. The lint rejects "16 Beats" literals and " Beats" suffixes.

AM18 — Docs. Where they differ, these replace plan §6.
- performance-controls.md:
  - Quantize reads Off / Next Beat / Next Bar and follows the model.
  - The top-bar counter is unchanged by BF7: BF7 puts lengths in bars and leaves counts alone.
  - The Link quantum is 1 bar.
- effects.md "Autopilot System":
  - Lengths read in bars and count from when the clip starts.
  - The per-type +/- buttons walk 1 Beat, 2 Beats, then whole bars.
  - Typing: a plain number uses the unit shown; "2 bars" and "6 beats" always work.
- architecture.md Naming Conventions:
  - core/BeatLength.h is the only source of beat and bar length labels (enforced by test_beat_label_lint).
  - Add the "4/4 only" note with the N3 site list.
- The jukebox doc: timing is in bars, and setTransitionBars takes bars.
- APP-INVENTORY.md: the changed value lists; "Loop length", "Content length" and "Image every"; the removed Duration row.
- BORIS_DECISIONS.md:
  - Add "Bars wherever beats (BF7, 2026-10-02): lengths read in bars; counts untouched".
  - Do NOT mark the 09-26 counter bullet resolved (AM1).
- CLAUDE.md: no change. No new pitfall.

### B. FINAL INVENTORY (only rows that differ from the plan)
- Row 1, TopBar counter: unchanged ("Bar 1".."Bar 4", "Bar -").
- Row 2, TopBar Quantize: Off / Next Beat / Next Bar. 3 items, enum unchanged. The combo follows the model.
- Row 3, Clip beatDivision: labels as in the plan; the row label becomes "Loop length", plus a tooltip.
- Row 4, Clip videoBeats: labels as in the plan; the row label becomes "Content length", plus a tooltip; typing per AM2.
- Row 6, Clip autopilot: labels as in the plan, plus a tooltip (AM6).
- Row 7, Layer On-Beat count: labels as in the plan; tooltip per AM6.
- Row 8, Composition per-type: BarStepSlider (AM3), typing per AM2, tooltips.
- Row 14, Jukebox: as in the AM10 table.
- Row 19:
  - the ClipInspector Duration/"Beats" row is REMOVED (AM4);
  - the hidden beat-random selector and AudioReadoutPanel stay unchanged (hidden, not touched).
- NEW row 23, slideshow selector (AM12):
  - today: "Beats per Image", with bare numbers 2..128;
  - after: "Image every", with 2 Beats, 1 Bar, 2 Bars, 4 Bars, 8 Bars, 16 Bars, 32 Bars;
  - storage: no change.

### C. FINAL BUILD STAGES
One builder works in one worktree, in this order. Each stage is its own set of commits, and ships alone once its gates pass.

- S0 "preflight" (no code):
  - Rebase onto ui, bf6 and bf9b.
  - Re-run the narrowed lint rules on the rebased base and record the RED list. It must equal G3's 36 literals, except for
    deltas caused by a lane merged before bf7; name each delta.
  - Grep that the AM4 members have no other references.
  - Check that the inspectors construct headless.
    - ClipInspector does (tests/test_clip_inspector_paint_key.cpp).
    - If CompositionInspector does not, run T5's per-type cases on a BarStepSlider configured by the same setup function the
      inspector calls.
- S1 "vocabulary + sync list":
  - I1 per AM2, AM3 (stepLadder) and AM13 (stub commit first);
  - I3, plus AM9.
  - Gates: G1; G2 (T1, T3); G3 partial (the ConnPicker and UniversalParamControl hits are gone).
- S2 "every list in bars" (needs S1):
  - I10 per AM11, written FIRST, with its RED log captured;
  - I4;
  - I5 lists, labels and tooltips (AM6);
  - I6 (AM6);
  - I8, plus AM10;
  - I9;
  - I12 (AM12);
  - I13 (AM4).
  - Gates: G1; G2 (T2, T6); G3 green at the end of S2.
- S3 "typed lengths" (needs S1):
  - the ClipInspector Content box per AM2;
  - the per-type BarStepSlider per AM3.
  - Gates: G1; G2 (T4, T5).
- S4 "Quantize relabel + follows the model" (independent of S1-S3):
  - I11 per AM8.
  - Gates: G1; G2 (T7).
- After S1-S4 are merged: G7, then G5. Nothing goes to Boris before G5 passes.

### D. FINAL CONSOLIDATED GATE LIST
Harmony copies gate strings only from this list. Every bar below is pre-registered.

G1 — Release build.
- `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` exits 0.
- Zero NEW warning lines in the touched files, compared with the pre-bf7 base's build log for the same files.

G2 — Tests.
- `ctest --test-dir build -j$(sysctl -n hw.ncpu) --output-on-failure` reports 0 failures.
  - A failure is waived only if the same test fails identically on the pre-bf7 base.
- The cases below must exist and pass.
  - Every case marked RED must fail on the pre-bf7 base, or, for a new header, against the AM13 stubs.
  - The builder pastes that failure into the lane report.

T1 — tests/test_beat_length.cpp (new exe; RED against the stubs):
- Label table:
  - 0.25 "1/4 Beat"; 0.5 "1/2 Beat"; 0.75 "3/4 Beat";
  - 1 "1 Beat"; 2 "2 Beats"; 3 "3 Beats";
  - 4 "1 Bar"; 6 "6 Beats"; 8 "2 Bars"; 12 "3 Bars"; 16 "4 Bars"; 32 "8 Bars"; 64 "16 Bars"; 128 "32 Bars";
  - 1.5 "1.5 Beats"; 4.00001f "1 Bar";
  - 0, -1, NaN and inf all give "".
- isWholeBars: 4, 8 and 64 are true; 2, 6 and 0 are false.
- parse(text, unit):
  - with unit 1: "4 bars" -> 16; "4 BARS" -> 16; "4 bars " -> 16; "4bars" -> 16; "2 bar" -> 8; "1 Bar" -> 4; "6" -> 6;
    "1/2" -> 0.5; "6 beats" -> 6;
  - with unit 4: "6" -> 24; "1/2" -> 2; "6 beats" -> 6;
  - nullopt for "x", "bars", "", "  ", "0", "-2", "1/0", "nan", "inf" and "4 bat".
- Round trip: parse(label(v), 1.0) == v for every v in {0.25, 0.5, 0.75, 1, 2, 4, 8, 16, 32, 64, 128} and for every integer
  1..64.
- stepLadder over [1, 64]:
  - The up-walk from 1 yields exactly 1 Beat, 2 Beats, 1 Bar, 2 Bars, 3 Bars ... 16 Bars (18 labels), then stays at 16 Bars.
  - The down-walk from 64 is the exact reverse, then stays at 1 Beat.
  - From 6: up gives 8, down gives 4. From 3: up gives 4, down gives 2.

T2 — tests/test_beat_label_lint.cpp (new exe):
- The AM11 self-test cases pass.
- RED on the base is exactly the G3 list.
- GREEN is 0 hits.

T3 — tests/test_conn_picker.cpp:
- describeSource(LFO Square, 4) == "Square 1 Bar". RED: today it returns "Square 4 Beats".
- BpmSync divIdx 6 gives cycleBeats 16. RED: today the clamp gives 8.
- describeSource(LFO Sine, 16) == "Sine 4 Bars".
- AM9's 4x7 tick-consistency table passes.
- The existing "Square 1 Beat" case is unchanged and passes.

T4 — tests/test_bar_step_slider.cpp (new exe; RED against a BarStepSlider stub with no overrides):
- Start a BarStepSlider at 4, and press the real "+" button by calling the onClick of the slider's child button:
  - "+" gives 8, text "2 Bars";
  - "-" gives 4, "1 Bar";
  - "-" again gives 2, "2 Beats".
- At 6 (set by setValue), the text reads "6 Beats". From 6, "+" gives 8 and "-" gives 4.
- Typing "6 beats" through the slider's text-box Label (setText(..., sendNotificationSync)) gives 6, "6 Beats". The typed value
  is kept exactly; it is not moved onto the ladder.

T5 — Typed boxes through the real text path (new cases in the tests/test_clip_inspector_paint_key.cpp harness, or a new exe).
- ClipInspector Content box:
  - At 4 ("1 Bar"), type "4": clip videoBeats 16, text "4 Bars". RED on the base: 4 and "4".
  - At 2 ("2 Beats"), type "1": 1, "1 Beat".
  - Type "3 bars": 8, "2 Bars" (the inspector's snap list).
  - Type "x": no change.
- Per-type box:
  - At 16 ("4 Bars"), type "32": the model field becomes 64 and the text reads "16 Bars" (a visible clamp).
  - Type "6 beats": 6, "6 Beats".
  - Type "": no change.

T6 — tests/test_preset_selector_bars.cpp (new exe; fixture as in plan I8):
- setTransitionBars(4): exactly 1 switch after 4 crossings, and it lands on the 4th. RED on the base: 0 switches.
- setTransitionBars(16): the first switch lands on the 16th crossing.
- A default-constructed PresetSelector: the first switch lands on the 16th crossing. This is a regression guard and is GREEN
  on the base too.
- kJukeboxBarChoices[kDefaultJukeboxChoice] == 16.
- The browser's item labels equal label(bars * 4) for every entry.
- First, the builder confirms that processFrame has no earlier switch path when the structural input is constant (plan I8).

T7 — tests/test_topbar_quantize_sync.cpp (new exe; harness of test_topbar_link_toggle.cpp):
- Build `TopBar bar(bus, comp)`. AFTER construction, set comp.quantizeMode = NextDownbeat and call
  bar.syncQuantizeFromComposition().
  - The quantize combo's selected id is 3, and its text is "Next Bar". RED on the base: the function is absent; the base combo
    reads id 1, "Off".
- Set Off and sync again: id 1.
- The item texts are exactly {"Off", "Next Beat", "Next Bar"}. RED on the base: "Next Downbeat".
- After the syncs, comp.quantizeMode still equals what the test last set. The sync never writes the model.
- The timer wiring itself is proven live in G5. TopBar's Timer base is private (TopBar.h:12).

G3 — Lint RED log in the lane report; lint green on the merge.
- RED on main c6720a0 (verified by this ruling's scratch run): 36 literals.
- (a') 30 hits:
  - ClipInspector.cpp:148-151: "4 Beats", "8 Beats", "16 Beats", "32 Beats".
  - ClipInspector.cpp:215-217: "4 Beats (1 Bar)", "8 Beats (2 Bars)", "16 Beats (4 Bars)".
  - LayerInspector.cpp:108-111: "4 Beats", "8 Beats", "16 Beats", "32 Beats".
  - MilkDropBrowser.cpp:563-566 and :612-615: "4 beats", "8 beats", "16 beats", "32 beats" (each range).
  - SignalInspector.cpp:45-46: "4 Beats", "8 Beats".
  - SignalInspector.cpp:89-91: "4 Beats", "8 Beats", "16 Beats".
  - UniversalParamControl.cpp:449 and :607: "4 Beats" and "8 Beats" on each line.
  - ConnPicker.cpp:28-29: "4 Beats", "8 Beats".
- (b') 4 hits: "2 Bar" and "4 Bar" at ClipInspector.cpp:166-167 and RoutineDeckView.h:278-279.
- (c') 2 hits: " Beat" and " Beats" at ConnPicker.cpp:34.
- GREEN: 0 hits, with the allowlist empty.

G4 — Deleted by AM8.
- The quantize probe is pre-registered for the Q2 follow-up (AM16).
- Harmony runs nothing under this id for bf7.

G5 — VISUAL WORK GATE. Nothing is shown to Boris before it passes.
- Headless tool tests/tool_beat_labels_snapshot.cpp (pattern: tests/tool_routine_deck_snapshot.cpp). It renders PNGs of:
  - ClipInspector with a BPM-sync clip: Loop length 1 Bar, Content length 2 Bars, autopilot 4 Bars, snap 2 Bars.
  - ClipInspector with a Timeline clip, to show the Duration row is gone in both modes.
  - LayerInspector: On Beat, 8 Bars.
  - CompositionInspector per-type: 4 Bars / 2 Bars / 1 Bar, plus one frame with Opaque at 6 Beats.
  - SignalInspector: oscillator 4 Bars, envelope 1 Bar.
  - MilkDrop: jukebox 16 Bars, playlist 4 Bars.
  - TopBar with Quantize at "Next Bar".
  The tool also writes a text dump of every changed combo's items and every AM6 tooltip.
- Live check:
  - Run ONE Audio-DNA under the live lock, launched with `open -g` per the probe-rig rules.
  - Load a composition fixture with quantizeMode 2 through POST /api/load_composition.
  - Take main-window captures by Quartz window id only: never full screen, never an Output window.
  - The captures must show:
    - the Quantize combo reading "Next Bar" straight from the file (RED on the base: "Off");
    - the slideshow row reading "Image every" and "2 Bars".
- Critic seats: 4 separate agents that Harmony dispatches.
  - The seats: bf7-critic-visual, bf7-critic-ux, bf7-critic-graphic, bf7-critic-logic. The logic seat covers
    combo-follows-model, the +/- ladder and the typed-unit rule.
  - Each seat must Read every decoded PNG path (Harmony lists them in the dispatch).
  - Each seat returns one verdict line per image: PASS, or BLOCK with a reason.
  - A seat reply without a per-image list is void, and that seat is re-run.
- BLOCK rules (pre-registered):
  1. any G7 failure, or any truncated, squashed or ellipsized text in a captured changed control;
  2. any whole-bar length shown in beats in a changed control (in the text dump: an item matching rule (a'), or "N Beats"
     with N % 4 == 0);
  3. any "2 Bar" / "4 Bar" singular-plural error;
  4. the Duration/"Beats" row visible in either ClipInspector mode;
  5. the live Quantize combo not showing the file's value;
  6. any beat-valued item shown as a bare number (e.g. the slideshow combo still reading "8");
  7. an AM6 row missing its tooltip in the dump;
  8. any label still reading "Beats/ Cycle", "Content Beats" or "Beats per Image".
- After all four seats PASS, Harmony builds an artifact page for Boris. It holds:
  - the PNGs;
  - the AM10 jukebox table;
  - the Q1 mockup ("Bar 3" vs "Bar 3 · Beat 2");
  - the removed-row note.

G6 — Perf: none. No hot path changes. The TopBar repaint is untouched because the counter is unchanged.

G7 — Measured text fit: tests/test_beat_label_widths.cpp (new exe, headless).
- Rule: for every changed control, and for every item or value it can show,
  juce::GlyphArrangement::getStringWidth(font, text) <= the width of the text area.
  - getStringWidth is verified at build/_deps/juce-src/modules/juce_graphics/fonts/juce_GlyphArrangement.h:329.
  - The text area is the control's child juce::Label bounds minus its border size, at the control's real size, with the font
    the LookAndFeel assigns.
  - For a painted label, the text area is the paint rect, with the paint font.
- Controls checked:
  - ClipInspector: the Loop length combo and its label; the Content length label; the Content box at "16 Bars" and at
    "2 Beats"; the autopilot combo; the snap combo.
  - LayerInspector: the count combo.
  - CompositionInspector: the three boxes at "16 Bars" and at "6 Beats".
  - SignalInspector: the oscillator and envelope combos.
  - MilkDropBrowser: the jukebox and playlist combos.
  - TopBar: the Quantize combo.
  - UniversalParamControl: the source button at its longest new name.
  - The slideshow selector and its label, as a replica (AM12).
- Negative control (required): a 40-character string in the Content box must FAIL the checker.
- The builder puts the full table (control, text, width, available) in the lane report.
- Bar: 0 failures, AND the negative control fails.
- Popup lists are not measured, because a JUCE popup sizes itself to its widest item. They are listed in the G5 dump.

### E. BORIS QUESTIONS (plain words; each has a default, so the build never waits)
- Q1. The top bar shows the beat as a 4-part wheel, and the bar as "Bar 1, Bar 2, Bar 3, Bar 4, Bar 1...". Is that the
  1-2-3-4-1 count you asked for on 26 September? Or do you also want the beat as a number, like "Bar 3 · Beat 2"? (The
  artifact page shows both.)
  DEFAULT: keep it as it is.
- Q2. The Quantize menu in the top bar will read Off / Next Beat / Next Bar. Clips and routines can already wait 2 or 4 bars.
  Should the top-bar Quantize also offer "Next 2 Bars" and "Next 4 Bars"? A 4-bar wait can be up to 8 seconds at 120 BPM. If
  you say yes, a waiting clip would also blink until it starts.
  DEFAULT: not now.
- Q3. Autopilot "every 4 bars" counts from the moment the clip started. If a clip starts mid-bar, its changes land mid-bar
  too. Should autopilot changes always wait for the start of a bar?
  DEFAULT: leave it as is. Clips fired with Bar snap already start on the bar.
- Q4. Lengths that are whole bars will read "1 Bar", "2 Bars", "4 Bars". Do you want just that, or both numbers, like
  "16 Beats (4 Bars)"?
  DEFAULT: just bars.
- Q5. In the MilkDrop jukebox, "30 sec" and "60 sec" were never seconds: they changed presets every 32 and 64 bars. They will
  now read "32 Bars" and "64 Bars". Do you also want choices in real seconds?
  DEFAULT: no, bars only.
- Not asked, just told:
  - The clip panel's "Beats/Duration" row never did anything, and it is removed.
  - When you type a length, a plain number uses the unit shown: "4" next to "2 Bars" means 4 bars. Typing "6 beats" or
    "2 bars" always works.
  - The time signature stays 4/4.

### F. RISKS
- R1. BarStepSlider depends on JUCE's internal call paths: snapValue is called from +/- and from typed text
  (juce_Slider.cpp:414, :447). A JUCE upgrade could move them. T4 drives the real buttons and the real text box, so a moved
  path turns T4 red.
- R2. "A plain number uses the unit shown" means the same keys give different lengths depending on what the box shows. This is
  accepted: the unit is on screen, highlighted, at the moment of typing (juce_Label.cpp:243). The plan's "always beats" rule
  contradicted the screen in every default state.
- R3. Removing the dead row moves the rows below it up by one row, and ClipInspector is shared with the ui lane, which builds
  first. Mitigations:
  - AM4 edits paint() and resized() together;
  - test_clip_inspector_paint_key stays green;
  - G5 renders both ClipInspector modes.
- R4. A label that does not fit forces a column-width change (AM6), which is a layout edit. G7 pins the widths. The builder
  never abbreviates.
- R5. Many lanes touch MainComponent.cpp. AM12 edits 3 small blocks, and S0 rebases first.
- R6. STRONGEST COUNTERARGUMENT to this ruling: "Dropping the 2/4-bar global Quantize leaves the global combo as the one
  quantize list without multi-bar values, out of step with clip snap and routines." It loses:
  - nobody asked for it: Boris's words are about labels (backlog:23);
  - it brings an 8-second wait with no feedback on the cell (H1);
  - its probe as written could not fail (G-A3);
  - "Next Bar" already gives the combo its bar value;
  - Q2 plus AM16 make it a ready follow-up if Boris wants it.
- R7. SECOND COUNTERARGUMENT: "The bar.beat counter was part of Harmony's default, which Boris accepted." It loses:
  - that text was Harmony's consequence text (backlog:30), not Boris's words;
  - the 09-26 bullet is still marked "being confirmed";
  - a wrong guess on a display bound by a ruling costs more than one question;
  - if Q1 comes back yes, it is a one-function follow-up.

### G. FINDINGS FOR HARMONY'S BACKLOG (not bf7 items)
- H1. A queued clip trigger is invisible.
  - No file in src/ui reads the Layer's pending trigger (Layer.h:49-50).
  - Today's per-clip "4 Bar" snap can wait about 8 s at 120 BPM with no feedback on the cell.
  - It is a mandatory item if Q2 is answered yes; otherwise it is worth its own small lane.
- H2. The global Quantize combo has never followed a loaded composition (plan V4).
  - AM8 fixes it in bf7 (S4).
  - It is recorded here because it is a Pitfall 41 case that predates BF7.
- H3. The unused `label` argument of CompositionInspector's setupCycleSlider ("Opaque Beats" etc., :105-115) is dead text. It
  is left alone because it is never shown.

STATUS: FINAL
