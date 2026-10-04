# RULING: the NOTICES lane (plan-notices.md) -- architect ruling on the blind council's attacks

Architect ruling, s-rta-1003b (Opus at max effort). Harmony decides after this; this is her working document.
Pin: main 34179a2. Every src / tests / probe line below was read by me at that commit (`git show 34179a2:<path>`,
`git grep ... 34179a2`). Nothing was built, run or launched. JUCE's own source is not in git: it was read in the main
checkout's `build/_deps/juce-src` (JUCE 8.0.4, the tag CMakeLists.txt:45-46 pins) and is labelled so where used.
Plan: `.harmony/.reports/s-rta-1003b/plan-notices.md`. Papers (4 of 4 seats, 29 attacks, written first):
`.harmony/.reports/s-rta-1003b/attack-notices-papers.md`.
Attack ids: two seats both numbered their attacks ST-n. Here SO-ST-n = the stage-operator seat, TS-ST-n = the
state-truth seat, GA-n = gates, CO-n = completeness.
Labels: VERIFIED = I read the line at the pin. INFERRED / ASSUMED = said so at the claim.
Harmony constraint: a state display shows what IS and never what just happened.
Harmony constraint: Boris is quoted only verbatim; every reading of his words below is Harmony's or the architect's.

## 0 VERDICT

The plan NEEDS REVISION and then stands. It is the build base; the 24 amendments of section 3 override its body.
Of 29 attacks: 23 ACCEPT, 5 PARTIAL, 1 REJECT. No attack sinks the design (one function for the one box, the notify
seam kept as the log channel, probes moved before any text goes). What the seats found, in order of weight:

1. The moved probe waits were already true before the load had landed (GA-1, CO-1). VERIFIED: `finishStagedLoad`
   publishes `staged = 0` at src/MainComponent.cpp:3324, before the swap (:3353), the deck push (:3392) and the label
   (:3360, :3398). Fixed by a wait on a counter that only rises AFTER the model changed (`load.timing.loads`,
   src/core/LoadTiming.h:48-57, ended at :3402), then one message-thread barrier (AM-13).
2. One "truthful state" write of the plan was built on a false premise and was itself an event-time write (TS-ST-2):
   T5 is dropped. Two real lies the plan missed are fixed at their root, with no label write: the image folder that
   destroyed the running slideshow before it knew the new folder was empty, and Show in Finder after a refused Record
   (TS-ST-1, TS-ST-3, CO-3).
3. The proof of the ONE box Boris kept could not fail where it matters (SO-ST-4, GA-4, CO-6). The fact it waited on is
   settled by reading: the box is a JUCE component, not a native alert. The real box is now a row that always runs,
   with mutants on the guard and on each caller (AM-5).
4. Four live rows had no dependable RED arm (NR6 by its own text; NR10, NR12 and NR13 never put the Record tab on
   screen), no row reached the lie T1 fixes, and three rows asserted a REST answer the app does not give (GA-2,
   GA-3, CO-6, and my own finding X1: the /api/perf routes answer `ok` at once and drop the refusal,
   src/api/ApiServer.cpp:1682-1686).
5. WHICH failed saves show the box is not settled by Boris's words. His sentence is general; Harmony's recorded
   reading is "exactly the two boxes". Three seats attacked the narrow default (SO-ST-1, SO-ST-5, TS-ST-6, CO-2). I
   recommend the default flips to: no save's failure is quieter after this lane than it is today, and its one form is
   the box. That is 7 callers of one function instead of 3. It is a DEFAULT, Harmony's to adopt (H-1), Boris's to
   answer (Q1). The narrow form is specified too, so either answer is built from this ruling.

The one rejection: the wording " without audio" after the audio capture stopped (TS-ST-5) stays.
The device line (SO-ST-2) keeps the plan's default (it goes) but the question to Boris is rewritten, because the plan's
wording told him something that is false after the first clip fire.

Order: this lane starts after BOTH the sync dial (bf2) and the transport lane have merged (the dispatch names both as
ahead). The plan's fork F7 ("before transport") is overridden (AM-22). Every line number here is the pin's; S0
re-derives all of them on the real base.

## 1 FACTS RE-DERIVED (VERIFIED at 34179a2 unless labelled)

Loads and their witnesses
- R1 `finishStagedLoad` (src/MainComponent.cpp:3318-3407), in order: :3320 `staged_` moved out; :3323-3324 the label
  hold cleared and `publishLoadWitness()` (`stagedNow_` = 0, :3233-3238); :3326-3345 sequences opened; :3353 the model
  swap (composition) or :3388-3392 the deck push; :3395 grid rebuild; :3360 / :3398 the label; :3402
  `loadTiming_.end()`; :3403-3406 ticket, reset, `pumpLoadQueue()`. The pump can stage the next queued load inside the
  same message (:3429-3453), publishing `queued` one lower BEFORE the next `beginStagedOpen` publishes `staged = 1`.
- R2 `/api/state` "load" is read from atomics on any thread (:5514-5527). `timing.loads` is `LoadTiming::loads_`: it
  rises only in `end()` (src/core/LoadTiming.h:48-57) and is read under a mutex (:74-86). `begin()` runs at :3483
  (composition), :3652 (Load Deck), :3938 (Duplicate); `end()` at :3379 (the deck-id refusal) and :3402. So
  `timing.loads` rises once per staged load, AFTER that load changed the model. A load refused at validation calls
  `begin()` and never `end()` (:3657-3664): the counter does not move.
- R3 GET /api/debug/ui_text is answered ON the message thread (`callAsync` + a 2 s wait,
  src/api/ApiServer.cpp:2073-2111): a barrier. It needs no --test-mode (routes compiled under AUDIODNA_TEST_SERVER,
  :299-344).
- R4 `.harmony/RIG-RULES.md:22-23` forbids polling /api/composition while a staged load may land;
  `.harmony/probe-asan-live.sh:171-183` waits on the label and :174-177 records the ASan container-overflow that rule
  came from. Its three label waits: :218 "Loaded deck:", :236 "Duplicated deck:", :269 "Loaded: bf9b-check".

Saves
- R5 Save (:3546-3569): the box only when the show has a path whose parent is a directory AND `saveToFile` fails
  (:3548-3563); otherwise the Save As chooser (:3565-3568), no box. Save As: `saveCompositionTo` (:3571-3585) returns
  false silently; its chooser callback raises the box itself (:3609-3615). The test route calls `saveCompositionTo`
  directly (:2151) and reaches no box. Save Deck: the same fall-through to the chooser (:3757-3768); `writeDeckFile`
  raises the box (:3812-3827). All three box sites are skipped in `testMode_` (:3557, :3609, :3821).
- R6 Other writes: Save Preset handles only success (:2983-2989); FX Save the same, and its 300 ms flash is in the
  success branch (:4397-4415); Collect Files' show copy (:6694), Export Bindings (:7291) and Save Layout (:7332)
  ignore the result. All use `replaceWithText` (src/ui/PresetManager.cpp:156, src/model/Composition.h:1156,
  src/recording/Take.cpp:288-294, src/model/AppSettings.cpp:40).
- R7 A take: a provisional take.json is written at Record (src/recording/RecorderHost.cpp:278, :293) and again
  periodically (:537-541); the final save is :383; `disarm` returns `ok = saved` (:391) and clears `recording_`
  either way (:406). `perfStop` (:5912-5934) reports a not-ok disarm as "Could not stop the take: <error>" where the
  error may be empty (:392); `perfStopPlay` handles an overdub (:6003-6014); the natural end of a replay calls
  `perfStop` (:6041-6042). Saving a ROUTINE writes nothing to disk (:6219-6221).

The recorder and the Record tab
- R8 `arm`: "already recording" returns at :169-173, BEFORE `takeFolder_` is written (:185); and `perfRecord` returns
  before it even calls `arm` when a take is recording (:5849-5855). The refusals AFTER :185 are :215 (overdub audio not
  found), :229 (the audio folder), :235 (disk space). `recording_` becomes true only at :265. REST can reach :215:
  /api/perf/record takes `overdubAssetId` (src/api/ApiServer.cpp:1674; src/MainComponent.cpp:5887-5888).
- R9 The perf routes answer `{"ok":true}` at once and drop the funnel's string (src/api/ApiServer.cpp:1682-1686,
  :1699-1700, :1747-1748). A funnel-level refusal ("A take is already recording.", "No take is loaded...") reaches
  REST nowhere. The routine save and fire routes answer `ok` at once too (:2547-2551, :2573-2577), and the save
  route itself refuses a body without a range (:2530). The routine funnels do set `lastError`
  (src/MainComponent.cpp:6154, :6274, :6318; src/recording/RoutineEngine.cpp:686-697).
- R10 The panel: refreshed from the timer every 8th tick (:4315-4337); `setNotice` only stores
  (src/ui/RecordPanel.cpp:190-195); texts are applied at the refresh (:292-298); a tab switch refreshes at once
  (:210-215); Load Take is a folder chooser over the takes root (:44-60); Show in Finder reveals `revealFolder`
  (:65-67); the three lines are laid out at :371-373 and :405-407. `nowSeconds` has two readers only: the notice
  expiry (src/ui/RecordPanelModel.h:296) and the routine-notice expiry (RecordPanel.cpp:304).
- R11 The model (src/ui/RecordPanelModel.h): `revealFolder` reads `takeFolder` when idle and nothing is loaded
  (:205-209); the idle line reads `takeFolder` (:274-277); " without audio" is the audio-not-requested branch
  (:256-258); the warning ladder :279-289; the notice block :291-301; the Record Over tooltip (:175-176) does not say
  the loaded take is kept; the play-with-audio tooltip while playing is "Locked while the take replays." (:228).

The top row
- R12 `setFileLabel` (:3206-3211); 46 lines under src hold `setFileLabel(`, 3 hold `fileLabel_.setText`.
  A model swap ends in `refreshUiAfterModelSwap` (:3162) whose last call is `refreshPreviewFromShow` (:3116), which
  writes the label from the NEW show: the first playing image or source clip (:5165, :5173), else "" (:5184). Only
  after that does :3360 write "Loaded: <name>". Clip fires write the label (:4926, :4932). The "Mic: ..." text is
  written at :300, :486, :538, :5842 only. The audio error lambda :470-474; the re-apply write :484-487 runs only
  when the error is empty.
- R13 src/audio/AudioEngine.cpp: a re-apply calls `onError("Audio device: ...")` on an error and `onDevicesReapplied`
  always (:15-21); `loadFile` stops and drops the old source (:54-58) BEFORE it tries the new file (:60-66);
  `getDeviceStatus` reads the manager now (:133-143). `onDeviceStateChanged` (AudioEngine.h:52, called
  AudioEngine.cpp:184-185) has one assignee, src/MainComponent.cpp:481.
- R14 The device line: :476-481, :488, text :3213-3231 (recomputed from `getDeviceState()`), layout :2762-2767, test
  hook :2153-2155.
- R15 Image folder: the running list is cleared at :4489, before the empty test (:4497-4501); `advanceSlideshow`
  returns on an empty list (:4518-4521). Camera: `openCamera` closes the current camera first (:4559); a failed open
  writes the label (:4572-4576); success writes "Camera: <device>" (:4583); `closeCamera` writes no label
  (:4586-4594); the selector's `onChange` does not revert (:364-370).

Boxes
- R16 12 `showMessageBoxAsync` call sites, all in src/MainComponent.cpp (:3024, :3033, :3493, :3508, :3559, :3611,
  :3660, :3677, :3822, :4722, :4752, :4766), plus one comment line (src/ui/LookAndFeel.h:15). `showOkCancelBox`: :3172,
  src/ui/CompDecksBrowser.cpp:263, plus one comment (LookAndFeel.h:84). Lines under src holding any of `AlertWindow`,
  `MessageBox`, `DialogWindow`, `CallOutBox`, `BubbleMessage`: 57 (MainComponent.cpp 30, MainComponent.h 1,
  CompDecksBrowser.cpp 1, LookAndFeel.cpp 14, LookAndFeel.h 9, PreferencesDialog.cpp 1, PreferencesDialog.h 1).
- R17 FN-1 SETTLED BY READING (JUCE 8.0.4 in build/_deps/juce-src; not in git): `showMessageBoxAsync` calls
  `showAsync` (modules/juce_gui_basics/windows/juce_AlertWindow.cpp:638-651), which goes native only when
  `isUsingNativeAlertWindows()` (:625-631); that returns `useNativeAlertWindows` on macOS
  (lookandfeel/juce_LookAndFeel.cpp:192-198), default false (juce_LookAndFeel.h:295); no line under src sets it (grep:
  0). The non-native path puts a JUCE component into modal state (detail/juce_AlertWindowHelpers.h:51-52). So the box
  is a JUCE component that `ModalComponentManager` lists and can cancel. tests/test_lookandfeel_square.cpp:117-126
  already builds that very alert ("Save Deck" / "Save failed: x.deck.json") through the app LookAndFeel.
- R18 FN-2 SETTLED: no route selects a browser tab (grep of src/api: only the `inspector_tab` readout).

Pads, MIDI Learn, tests, probes
- R19 The pad "!" is painted (src/ui/RoutinePad.cpp:109-117) from `pad.warning` (src/ui/RoutineDeckView.h:168); the
  tooltip prefix is `warningText` (:126-136, :206); both read the slot's LAST-RUN counts
  (src/recording/RoutineEngine.cpp:820-829, :888-896). `kWarning` has one user (RoutinePad.h:60, RoutinePad.cpp:113).
  MIDI Learn's "Last: ..." is painted (src/ui/MidiLearnOverlay.cpp:101-104).
- R20 ctest names are TEST_CASE names (`catch_discover_tests`, tests/CMakeLists.txt:24 ff.). The eight cases the plan
  retires exist under exactly those names (tests/test_staged_load.cpp:70, :79, :85;
  tests/test_record_panel_model.cpp:463, :499, :514, :610; tests/test_routine_deck_view.cpp:200). Row 9 of the model
  test holds more than CHECK lines on the removed fields: :313-327 builds notice inputs and reads `noticeLive`. The
  helper `inputs(double now)` (:18-23) sets `nowSeconds`. Row 2 (:98-112) pins `takeFolder` for both the idle line and
  Show in Finder.
- R21 probe-async-load.py: a3's cancel clause already compares with the label read before the load (:770-777); a4(f1)
  expects "Loading ..." (:840-841); a4(f2) expects the label after the cancel to be the TRIGGER's text, top.png
  (:846-857); a5 :879-892, a5b :920-941, a6 :957-973 wait with `wait_until(num_decks() == n)` while the load is
  staged (`num_decks()` is GET /api/composition, :322-326, :368-369). probe-btguard.sh reads `audio_notice` at :223, :240, :307, :314, :338, :354, :402, :413 (texts :150-151); it
  has no lever that makes a re-apply FAIL (grep).
- R22 Already gone at the pin: the Remove Deck sentence, the Undo Remove button and its hide call, the deck load
  notice (`finishStagedLoad` only logs `migrationNote`, :3355-3357).
- R23 Stale after the removal: src/recording/RecorderHost.cpp:285-290 and RecorderHost.h:72, :75-79 (they cite the
  notice key and the "notice INVARIANT"), src/recording/RoutineEngine.h:48, the TEST_CASE name at
  tests/test_lookandfeel_square.cpp:96-98 ("the 12 showMessageBoxAsync calls"), `.harmony/APP-INVENTORY.md` :85, :109,
  :232, :357.
- R24 `PresetManager::LoadStats` (src/ui/PresetManager.h:41-50): `resolvedByKey`, `resolvedByName`, `legacyIndex`
  have no reader under src today; `dropped`, `mappingsTotal`, `droppedDescriptions` are read at src/MainComponent.cpp
  :397, :3022-3029, :4433; `legacyFile` at :3031 and in tests/test_preset_manager.cpp:259, :285.

Not verified by me (carried with its label): `populateSlotMenu`'s shown name (plan T10, INFERRED); the callers of
`AppSettings::save`; each of the 30 token lines of MainComponent.cpp one by one; the member list of
src/ui/RecordPanel.h; whether the modal box holds the keyboard (INFERRED from JUCE's modal state). S0 reads them.

## 2 ATTACK RULINGS (one row per attack; "decides" = the line I read that settles it)

| Attack | Verdict | Decides | What changes |
|---|---|---|---|
| SO-ST-1 a failed TAKE save defaults to nothing | ACCEPT (as a default: H-1, Q1) | `.harmony/binding-decisions.md:720` (his sentence names "a failed save", no kind); src/recording/RecorderHost.cpp:383-391 (`ok = saved`); src/MainComponent.cpp:5915-5922 | AM-1: `perfStop` and `perfStopPlay` call the box for a take whose final save failed. A refused Save Routine stays out (no disk write, :6219-6221). |
| SO-ST-2 the no-device yellow line | PARTIAL | :3213-3231 (recomputed state by mechanism); :4926, :4932 (a clip fire overwrites "Mic: ..."); backlog:269 (the mic-fallback note was Harmony's example in her question, not his words) | AM-3: default stays "goes"; the plan's reason and Q2 are rewritten; S4 stays separable. Not accepted: flipping the default. |
| SO-ST-3 when the box can appear | ACCEPT | :3548-3549, :3565-3568 (Save falls to the chooser when the parent folder is gone); :3762-3767 (deck) | AM-4: check 1 as concrete steps; the page says an unplugged drive opens Save As, not the box. |
| SO-ST-4 the gate on the surviving box | PARTIAL | R17 (JUCE component); :3557, :3609, :3821 (`testMode_` skips) | AM-5: the real-box row always runs; mutants on the guard; NT-L1 pins the guard line. Not accepted: an injectable `showBox` seam (any seam keeps one line that only a production run proves; NR15 is that run). |
| SO-ST-5 the save inventory is incomplete | ACCEPT | :2983-2989, :4397-4415, :6694, :7291, :7332 | AM-1 lists every save with its default. |
| SO-ST-6 a refused Record is never shown to Boris | ACCEPT | RecorderHost.cpp:226-231 with src/recording/AudioStore.cpp:109-124 (a read-only audio folder refuses the start) | AM-21: a performable check and question Q5. |
| SO-ST-7 DENY-ANY matches user content | ACCEPT | plan 5.3 (fragments " started", "refused", "is empty.") | AM-16: whole phrases, anchored where short; a fixture self-check. |
| TS-ST-1 Show in Finder after a refused Record | ACCEPT | src/ui/RecordPanelModel.h:205-209; RecorderHost.cpp:185 before :215 / :229 / :235 | AM-6: `revealFolder` reads the saved-take fact when idle; cases, mutant, bar. |
| TS-ST-2 T5's premise is false | ACCEPT | :3162 -> :3116 -> :5165 / :5173 / :5184, all before :3360 | AM-7: T5 dropped; :3360 deleted, nothing written; NR3 re-derived. |
| TS-ST-3 camera and image folder | ACCEPT | :4489 before :4497; :364-370; :4572-4576 | AM-9: the folder keeps its running list; the camera selector goes back to Off. |
| TS-ST-4 "Mic: ..." is an event write; a failed re-apply | ACCEPT | :484-487 (`error.isEmpty()`); src/audio/AudioEngine.cpp:15-21 | AM-10: the re-apply write runs after every re-apply; C02 named honestly; no live lever (R21), said so. |
| TS-ST-5 " without audio" is a second lie | REJECT | RecordPanelModel.h:256-258: the clause is present tense ("Recording ... from live input" = capturing now) | AM-11: T2 stands. Dropping the clause would make a take that stopped capturing read like one that says nothing about audio, with the Record audio switch still on and no yellow line left to say otherwise. |
| TS-ST-6 four other silent saves | ACCEPT | as SO-ST-5 | AM-1. |
| TS-ST-7 a failed take is still on disk; Load Take is a chooser | ACCEPT | RecorderHost.cpp:278, :293, :537-541; src/ui/RecordPanel.cpp:44-60 | AM-2: N4, T3 and Q1 say what is true. |
| TS-ST-8 "the loaded take is kept" is lost | ACCEPT | RecordPanelModel.h:175-176 against :300-301 | AM-12: the sentence joins the Record Over tooltip; plan F12 corrected. |
| GA-1 `staged == 0` is true at the START of the landing | ACCEPT | :3320-3324 against :3353 / :3392 / :3402 | AM-13: wait on `load.timing.loads`, then one barrier. Stronger than the seat's fix: it is exact with queued loads too. |
| GA-2 absence rows pass before a text could be drawn | ACCEPT | :4315-4337; RecordPanel.cpp:190-195, :292-298 | AM-14: Record tab shown and checked; the dump runs the panel's refresh first. |
| GA-3 no row reaches the lie T1 fixes | ACCEPT | RecorderHost.cpp:169-173 before :185; src/MainComponent.cpp:5849-5855 | AM-6: row NR9b and state V7 use a refusal AFTER :185. The seat's ":200" is :215 at the pin; the REST lever is `overdubAssetId` (src/api/ApiServer.cpp:1674), no file needs deleting. |
| GA-4 two of three callers have no row | ACCEPT | :3548-3563, :3609-3615, :2151 | AM-5: POST /api/debug/save (never opens a chooser), row NR7b, mutants MU-N10 and MU-N11, "+1 exactly". |
| GA-5 NT-L1 is an allowlist of names | ACCEPT | R16 (57 token lines) | AM-15: a per-file table of token lines. |
| GA-6 painted texts in DENY-ANY; bar B6 is vacuous | ACCEPT | src/ui/MidiLearnOverlay.cpp:101-104; RoutineEngine.cpp:820-829 | AM-16: painted texts are lint-only; B6 fires a routine first and needs > 0 on P1, or is struck. |
| GA-7 row 9 loses more than CHECK lines | ACCEPT | tests/test_record_panel_model.cpp:313-327, :18-23 | AM-17. |
| GA-8 "< 46" passes for any decrease | ACCEPT | 46 lines at the pin, 16 of them named | AM-18: exactly base - 16. |
| CO-1 the moved waits read mid-swap | ACCEPT | as GA-1 | AM-13. |
| CO-2 Save Preset and FX Save | ACCEPT (as a default: H-1, Q1) | :2987-2989, :4401-4403 (the removed text is the only sign of success) | AM-1 rows 4 and 5. |
| CO-3 camera and folder leave the label lying | PARTIAL | :4489; :4559; :4586-4594 (`closeCamera` writes no label today) | AM-9: the folder fix is the seat's own. Camera: the selector reverts; NOT accepted: writing the audio-source text at a camera failure (an unrelated text written at an event). The label that keeps "Camera: <name>" after the camera closes is the same on a plain Off today: SF-1. |
| CO-4 E07 and B19 line 3 | PARTIAL | RecordPanelModel.h:176, :228, :285, :301 | AM-12: E07's missing half moves to the tooltip. NOT moved: B19 line 3 (it is in table B; the mode is shown by the status line, the ticked switch, the source selector and the button). |
| CO-5 dead members and stale text | PARTIAL | R10, R13, R19, R23, R24 | AM-19: (a), (b), (d) accepted, plus `kWarning` and a log twin for B03. (c): the `LoadStats` fields stay, with the reason. |
| CO-6 NR6 cannot fail; NR15 is conditional | ACCEPT | R17 | AM-5, AM-23: NR15 always runs; NR6 becomes a production-mode row with a real RED arm. |

Seats in conflict, and how each was settled
- "Keep a signal" (stage) against "remove completely" (completeness): no attack pair asked for opposite things on
  the same text. Where a signal is kept or added it is a failed save (AM-1) or a state fact (AM-6, AM-8, AM-10).
  Where his words do not settle it, it is a question with a default: Q1 (which saves), Q2 (the device line), Q3,
  Q4, Q5.
- Camera (TS-ST-3 wants the selector reverted, CO-3 wants a label write): the selector is the camera's state display;
  the label is not. AM-9.
- The save box's proof (SO-ST-4 a seam, GA-4 a route and mutants, CO-6 an unconditional row): one design, AM-5.

Found by the architect while re-deriving (no seat raised them)
- X1 The /api/perf routes answer `ok` at once and drop the refusal (R9): three plan rows assert a REST answer the app
  does not give. AM-14.
- X2 The plan's moved cancel clause is wrong for a4(f2): that row expects the trigger's text, not the text before the
  load (R21). AM-20.
- X3 B03 (" (check mappings)") is a failure text with no log twin in the plan's list. AM-19(f).
- X4 `RoutinePad::kWarning` is dead once the "!" goes. AM-19(e).
- X5 NR6 as planned has no RED arm at all (the plan says so itself). AM-5.
- X6 The dispatch puts the transport lane ahead of this one; the plan recommended the opposite. AM-22.

## 3 AMENDMENTS (24; each OVERRIDES the plan body where they differ)

AM-1 WHICH FAILED SAVES SHOW THE BOX (plan N4 is replaced; a DEFAULT: H-1, Q1).
Boris, verbatim: "ok. the only fail message will be a failed save. remove all others"
His words do not say which saves. Harmony's recorded reading is the two boxes of a show and a deck. The architect's
rule for the default: no save's failure is quieter after this lane than it is at the pin, and the one visible form of
a failed save is the box. One function shows it:
`void MainComponent::reportSaveFailed(const juce::String& title, const juce::String& path)`.
Its callers, the pinned table of NT-L1 (function | title | when):
1. `saveComposition` | "Save Composition" | plain Save and `saveToFile` returned false (today's :3557-3563).
2. `saveCompositionTo` | "Save Composition" | Save As and the test route, `saveToFile` returned false (:3574-3575).
   The chooser callback (:3609-3615) raises nothing itself any more.
3. `writeDeckFile` | "Save Deck" | `replaceWithText` returned false (:3821-3825).
4. `savePreset`, its chooser callback | "Save Preset" | `PresetManager::savePreset` returned false (:2983).
5. `fastSave` | "FX Save" | `PresetManager::savePreset` returned false (:4397).
6. `perfStop` | "Save Take" | `disarm` returned not-ok and its error is not "not recording" (:5915-5919); the path is
   the take folder (`result.takeFolder`).
7. `perfStopPlay` | "Save Take" | an overdub was stopped and its save failed (:6004-6010); the path is
   `stopped.overdub.takeFolder`.
Text, every row: "Save failed: " + the full path.
- Why rows 6-7: a take's failed save has a failure message today (B17, B18) and is the most expensive failed save on
  stage. A wrong box can be deleted in one table row; a wrong silence loses a recording nobody knows is lost.
- Why rows 4-5: today a failed preset save differs from a good one ONLY by the missing "Saved: ..." text (:2987-2989,
  :4401-4403). The lane removes that text. Without rows 4-5 the lane itself would make that failure invisible.
- Never a caller: the save at Record and the periodic saves (RecorderHost.cpp:293, :540). One box per take, at the
  moment it is lost; a periodic box would repeat during a take. They stay in the log and in `lastError`.
- Not a save: a refused Save Routine (no take loaded, bank full, no bars). Nothing is written to disk
  (:6219-6221). Nothing shows; the pad row does not change. `/api/routine/status` keeps `lastError`.
- Silent at the pin and silent after (nothing removed, nothing added; listed on Boris's page): Collect Files' copy of
  the show (:6694), Export Bindings (:7291), Save Layout (:7332), settings.json, favourites, a snapshot or a video
  recording that does not start (inventory M03).
- THE NARROW FORM, if Harmony keeps her recorded reading or Boris answers so: rows 1-3 only. Rows 4-7 are not built;
  NT-L1's table has 3 rows; NT-U10 pins 2 titles; NR16 runs in its "silent" form; MU-N13 is dropped. Nothing else
  in this ruling changes.
- Stages: rows 1-5 in S2, rows 6-7 in S3.

AM-2 WHAT A FAILED TAKE SAVE LEAVES (plan N4 bullet 1, T3, Q1). The plan's "Load Take... does not list it" is
struck: Load Take is a folder chooser (src/ui/RecordPanel.cpp:44-60). True text: a take whose last save failed may
still be on disk as a partial take, from the save at Record or the last periodic save (RecorderHost.cpp:293, :540),
and Load Take... can open that folder. The Record line does not name it (AM-6).

AM-3 THE DEVICE LINE (plan N2 row B14, T7, Q2). The default stays: the whole yellow line goes, in S4.
- The plan's reason is replaced. By mechanism the line is a state display: it is recomputed from `getDeviceState()`
  and stays until the device is back (:3213-3231). The plan's own N2 rule would keep it. It goes for another reason,
  and that reason is a reading: it is the yellow line, it is worded as a failure and an instruction ("... found -
  plug one in"), and his last sentence leaves one failure message. His words do not name it: the microphone-fallback
  note was Harmony's example in her question, and his answer was "remove the list entirely and cleanly".
- Why the default is not flipped: each time a text was proposed to stay, he ruled it out ("yes remove the visible
  line. not needed"; "remove the list entirely and cleanly"; "remove all others"). A dead input still shows as state:
  flat meters, BPM "---".
- T7 is corrected: once a clip is fired, the top line no longer says "Mic: no wired mic" (:4926, :4932). After that,
  nothing in words says the input is dead, in either source mode.
- Q2 is rewritten (section 8). The plan's Q2 delta stays buildable (keep the widget for NoDevice / NoInput, drop
  MicReplaced): S4 is its own stage for that reason.

AM-4 WHEN THE BOX CAN APPEAR (plan section 7 check 1, and what Boris was told). An unplugged drive or a folder that
is gone does NOT raise the box: Save then opens the Save As window (:3565-3568; a deck: :3766-3767). The box needs a
folder that exists and cannot be written, or a full disk. Check 1 becomes concrete steps (section 7). The page says
the Save As behaviour in plain words. Not read: how the macOS Save window itself behaves for a folder that cannot be
written (ASSUMED: it refuses before the app is asked).

AM-5 THE BOX AND ITS PROOF (plan N5 "[S2]", 5.1 mutants, 5.3 NR6-NR8, NR15, fork F5).
- `reportSaveFailed` does two things: (1) always: the witness (count + 1, last title, last text; carried by
  GET /api/debug/ui_texts as `save_failed`) and one `logLine`; (2) unless `testMode_`: the one `showMessageBoxAsync`
  (WarningIcon, the title, "Save failed: " + path). No other place under src shows a message box.
- Test routes added in S1 (test-server builds only): GET /api/debug/ui_texts with `modals` (the titles of the
  components `ModalComponentManager` lists; always present -- R17); POST /api/debug/dismiss_modals;
  POST /api/debug/save_deck {deck, path} (calls `writeDeckFile`); POST /api/debug/save (calls `saveComposition()`
  ONLY when the show has a path whose parent is a directory; otherwise it answers `{"ok":false,"reason":"no path"}`
  and calls nothing -- a probe never opens a chooser); POST /api/debug/browser_tab {name} (R18).
- Rows: NR7 and NR8 also assert `modals == []` (test mode shows no box) and "count rises by exactly 1". NR7b is the
  plain Save branch. NR15 always runs (production mode: the real box is listed, then dismissed). NR6 becomes a
  production-mode row (L: no box; P1: the "Load Deck" box) -- as planned it could not fail. NR16 is the take.
- Mutants: MU-N8 the `testMode_` guard inverted; MU-N9 the box call removed, witness kept; MU-N10 plain Save's
  failure branch drops the call; MU-N11 the Save As callback reports again; MU-N13 `perfStop` drops the call.
- NT-L1 pins the guard too (AM-15 c), so MU-N8 has a RED arm even on a day NR15 cannot be run.
- Not built: an injectable `showBox` seam. Any seam keeps one line that tells test mode from production, and only
  a production run proves that line. NR15 is that run.
- Screen: NR6 and NR15 put a real box of the probe's own app on screen for under a second. FM-5 decides whether they
  run while Boris works (section 5.6; H-6).

AM-6 T1 COVERS SHOW IN FINDER; THE REFUSED-RECORD ROW (plan N3 T1, NT-U1, NT-U2, NT-U6, NR9, V7, B3).
- `RecorderHost::Status::lastSavedTakeFolder` as the plan: set in `disarm` only when `take.save` returned true, never
  by `arm`; published; added to /api/perf/status.
- When idle and nothing is loaded, BOTH readers use it: the status line (RecordPanelModel.h:274-277) and
  `revealFolder` (:205-209). Empty: "Ready. No take loaded." and Show in Finder disabled with "No take yet.". While
  recording, `revealFolder` stays `takeFolder`; with a take loaded, `loadedTakeFolder` -- unchanged.
- NT-U6's refused arm is the :215 refusal (an overdub id the store does not have) and also asserts that `takeFolder`
  WAS written by that refused arm -- the fact the fix exists for.
- Live: row NR9b (5.3). State V7 uses NR9b's driver. Bar B3 adds the Show in Finder button.
- The plan's NR9 stays as the "already recording" row but proves nothing about T1 (R8); its wording says so.

AM-7 T5 IS DROPPED (plan N3 T5, fork F4, NR3). src/MainComponent.cpp:3360 is deleted and nothing is written in its
place: the swap's own refresh has already written the label from the new show (R12). :3398 is deleted as planned.
After a composition load the label reads what `refreshPreviewFromShow` derives: the first playing image clip's file
name or source clip's type, bottom layer first, else "". NR3 asserts exactly that.

AM-8 T4 IS NARROWED (plan N3 T4, T6). The failed audio-file load.
- `bool AudioEngine::hasFileLoaded() const` as the plan. `fileLabelAudioSourceText(...)` (new header
  src/ui/FileLabelText.h) stays, with this one caller, plus the box titles and "Save failed: " as constants.
- The error lambda (:470-474), on the message thread: (1) logs the message; (2) if the engine has no file loaded and
  `currentAudioFile_` names a file: clears `currentAudioFile_`, and -- only if the file label's text is that file's
  name -- writes `fileLabelAudioSourceText(...)`. Nothing else. A device error writes nothing here (AM-10).
- Why the test on the label: the label has several owners and the last writer wins (audio source, fired clip,
  folder, camera, MilkDrop). The write is owed only when the label is naming the file that is gone. A true clip name
  is left alone; a label change at every failed load would be a failure notice in state clothing.
- Proof with a RED arm: row NR17.
- Runner-up, not built (H-4): reorder `AudioEngine::loadFile` so a failed load keeps the old file (three lines
  moved; no label write, no accessor, no new function). Smaller on screen, but it changes what he hears and what a
  replay with audio does when its wav cannot be read (:5962-5973), and it needs its own engine test.

AM-9 IMAGE FOLDER AND CAMERA (new rows of N3; plan rows B11, B12, C03).
- Folder: the chooser callback (:4484-4515) builds the list in a local and replaces `slideshowImages_`, the index
  and the counters only when that local is not empty. A folder with no images changes nothing: the running slideshow
  goes on and "Folder: <old> (n images)" stays true. One `logLine` (the log twin of B11). No label write.
- Camera: the failed-open branch (:4572-4576) sets the selector to "Off" (id 1, no notification) and logs (the twin
  of B12). No label write.
- Left as it is today (SF-1): after the camera closes, by Off or by a failed open, the label can still read
  "Camera: <name>" -- `closeCamera` writes no label (:4586-4594).
- No live row can drive either (a chooser; a camera). Covered by reading in review and Boris's check 11.

AM-10 THE FAILED DEVICE RE-APPLY (plan N2 row C02, T4, T7; inventory B01). `onDevicesReapplied` (:484-487) writes
"Mic: " + `getDeviceStatus()` after EVERY re-apply while the app listens to the mic, failed or not: the
`error.isEmpty()` test goes and the comment at :482-483 is re-worded. `getDeviceStatus` reads the device manager at
that moment (AudioEngine.cpp:133-143), so the text is what IS. C02 is named in the table for what it is: a state text
written at audio-source events, not a recomputed display. No probe can make a re-apply fail (R21): this hunk is
covered by reading and by NT-L2's allowlist, and the lane report says so.

AM-11 T2 STANDS (TS-ST-5 rejected). After the audio capture stopped mid-take the clause reads " without audio"
(RecordPanelModel.h:258) while `audioCapturing` is false. The clause is present tense: it says where the take's audio
is coming from NOW. What was captured before the stop stays in the take and is reported when the take is loaded
("Audio: ..."). NT-U3 is unchanged. docs/claude/recording.md gets that one sentence (AM-24).

AM-12 TOOLTIPS (plan N2 rows E07 and B19 line 3; fact F12).
- The Record Over tooltip (RecordPanelModel.h:175-176) becomes "Records a new take over this take's audio, from the
  current playback position. The loaded take is kept." New case NT-U11; row 9's CHECK at
  tests/test_record_panel_model.cpp:300 takes the new text. Plan F12 ("the tooltip already says it") was half true.
- B19 line 3 ("... Live input is paused until Stop Playback.") is NOT moved anywhere. It is in table B; the mode
  stays on screen as state (the plan's N2 row lists the four displays).

AM-13 THE SETTLE PROTOCOL (plan N6, every moved wait; R1, R2, R3).
For a load expected to LAND (k loads issued by the row):
1. Before the command: read `L0 = load.timing.loads` from /api/state.
2. POST the command(s); then one GET /api/debug/ui_text (the command's message has run).
3. Poll /api/state at 0.25 s until `load.timing.loads >= L0 + k` AND `load.staged == 0` AND `load.queued == 0`.
   Limits unchanged (15 s; 40 s for probe-asan-live L6), with the probe's ASan and health breaks.
4. One more GET /api/debug/ui_text (the landing's message runs on after `end()` -- ticket, reset, pump -- and has now
   returned).
5. Only now read /api/composition, once. The row's VALID clauses are judged on it. A later re-read is allowed only as
   the probe already does (probe-asan-live's 1 s steps), never inside steps 2-4.
For a load expected to be REFUSED before staging (not a deck file; the 9th queued request): step 2, step 4, then
`staged == 0`, `queued == 0` and `timing.loads == L0`.
For a load expected to be CANCELLED: the row's existing cancel clause (the ticket's answer), then step 4.
- Why it is sound: `timing.loads` rises at :3402, after the swap (:3353) or the deck push (:3392); step 4 waits out
  the rest of that message; with k counted, a queued load is never taken for the last one.
- What the plan had: the barrier, then `staged == 0 and queued == 0` -- true from :3324 on, that is, during the swap.
- One helper, used by every moved wait: probe-asan-live L1, L4, L6 (k = 1 each); probe-async-load's moved completion
  clauses; probe-notices NR1-NR4. `label()` and `done_label` go.
- It needs no app change: every witness exists at the pin, so the moved probes are GREEN on P0 as the plan requires.
- RED arm: FM-1 (section 5.6) and the probe mutant MU-P1 (the helper without step 3's `loads` clause and without
  step 4).

AM-14 RECORD-TAB ROWS (plan 5.3 NR9-NR13; R9, R10).
- Precondition of NR9, NR9b, NR10, NR11, NR12, NR13, NR16: POST /api/debug/browser_tab {"name":"Record"}; the dump
  must hold the status label with `showing == true`. If not, the row is RED "Record tab not showing".
- GET /api/debug/ui_texts runs the Record panel's own refresh first (the call the 4 Hz timer makes, :4335-4337), then
  reads the widgets. A dump after the driver's barrier therefore shows what the next timer refresh would show: no
  sleep, no race with the 4 Hz tick; on P1 the sentence is there (it was stored a moment ago; it lives 10 s).
- Each row: driver POST -> GET /api/debug/ui_text (barrier) -> GET /api/debug/ui_texts.
- The perf routes never answer a refusal (R9). The plan's "the second answer carries 'A take is already recording.'"
  (NR9) and "the REST answer carries the refusal" (NR10, NR13) are struck. The rows assert status fields (5.3). Plan
  fact F5's "(the funnels) RETURN their refusal strings to REST" is corrected: they return them to their caller; the
  perf routes drop them, and so do the routine save and fire routes (R9). NR13's body gives a bar range, or the
  route refuses it itself and the funnel is never reached.

AM-15 NT-L1, THE BOX LINT, BY TOKEN (plan N7).
(a) For each file under src, the number of lines (comments included) holding any of `AlertWindow`, `MessageBox`,
    `DialogWindow`, `CallOutBox`, `BubbleMessage`, `showYesNo`, `showNativeDialog` equals a pinned row. At the pin: 57
    lines in 7 files (R16). The lane's table is S0's table on the base, minus the removed alert sites, plus
    `reportSaveFailed`; each row names the dialogs it holds in a word each.
(b) `showMessageBoxAsync`: exactly 1 line under src, inside `MainComponent::reportSaveFailed`.
(c) The non-blank line before it is the `testMode_` test (`if (!testMode_)`).
(d) `reportSaveFailed(` call lines: exactly the caller table of AM-1 (function and title).
(e) No line under src holds `setUsingNativeAlertWindows` (R17 depends on it).
The plan's claim "deny-by-default ... any new box or dialog through a JUCE API" is re-worded: deny-by-token. A
hand-made overlay Component, or a JUCE name that is not on the token list, passes.

AM-16 THE DENY LISTS, PAINTED TEXTS, BAR B6 (plan 5.3, 5.5).
- DENY-LABEL (the file label only; "begins with"): "Saved", "Loaded", "Loading ", "Duplicated deck", "Slot ", "Too
  many loads", "Duplicate skipped", "Show in Finder", "No images found", "Camera failed", "Audio device:", "Failed to
  load".
- DENY-ANY (every showing Label, TextEditor and Button text; "contains", except the two marked whole-text):
  "plug one in", "lost - now listening", "ould not ", "A take is already recording", "Nothing is recording", "No take
  is loaded", "deck unresolved", "refused because", "audio stop refused", "Removed the routine", "Saved routine",
  "Saved:", "holding the last look.", "restored first", "No audio is arriving", "Replaying with", "Record Over
  starts", "routine bank is full", "no beat yet", "(check mappings)", "unresolved: ", "moved: ", "skipped: ", "audio
  gap"; whole-text patterns: `^Routine .+ started`, `^Routine pad [0-9]+ is empty\.$`.
- Painted texts are not in any dump: MIDI Learn's "Last: " and the pad's "!" are struck from DENY-ANY and are
  "lint only" (NT-L3).
- User content: the probe uses only fixtures it makes. Before any row it checks that none of its fixture names
  (show, decks, clips, takes, routines) matches a deny entry. A match is INVALID (the probe's fault), never RED.
- Bar B6 is rewritten: fire a routine that has a timeline pointing at a layer that no longer exists, wait for the
  run, then count the pixels of 0xffcc3333 inside every pad rect: 0 on L, and > 0 on P1 under the same driver. If
  the capture builder cannot make such a run by REST (FM-6), B6 is STRUCK and reported; the "!" then rests on NT-U8
  and NT-L3.

AM-17 TESTS BY NAME (plan N5 "Tests retired BY NAME", G-U2).
- Retired: the plan's eight TEST_CASEs (R20). No other name disappears.
- Changed and staying: rows 1, 6, 7, 8, 10, 11, 13, 14 of tests/test_record_panel_model.cpp lose their
  `warningText` / `noticeText` CHECKs. Row 9 loses :313-327 whole (the warning CHECK, the notice CHECK and the two
  notice scenarios). Row 2 is re-pinned on `lastSavedTakeFolder`, for the line and for Show in Finder. The helper
  `inputs(double now)` (:18-23) loses `nowSeconds`. Row 8's SECTION "moved and skipped counts are appended when
  non-zero" is deleted (NT-U4 replaces it). tests/test_staged_load.cpp:151-152, tests/test_routine_deck_view.cpp:69,
  tests/test_routine_pad_paint_key.cpp:70 as the plan.
- Renamed (the only one): tests/test_lookandfeel_square.cpp:96-98, "... (the 12 showMessageBoxAsync calls with no
  associated component, ..." becomes "... (the Save failed box: showMessageBoxAsync with no associated component,
  ...". The old name is absent from `ctest -N`, the new one present.
- G-U2: N on L = N on the base - 8 + 16 (NT-U1 .. NT-U11 and NT-L1 .. NT-L5), plus any test S0 adds, by name.
  SECTIONs are not ctest names: the name diff is about TEST_CASEs only.

AM-18 G-U6's FIRST COUNT IS EXACT (plan 5.2). `setFileLabel(` lines on L = (S0's count on the base) - 16. The 16, at
the pin: :395, :2988, :3014, :3360, :3398, :3450, :3553, :3581, :3636, :3816, :3895, :3920, :4402, :4431, :4499, :4574.
:472 stays one line (AM-8). No amendment adds a line. At the pin: 46 -> 30.

AM-19 CLEAN-UP, ADDED TO THE PLAN'S LISTS (plan N5 "By file").
(a) [S3] `nowSeconds`: `RecordPanelInputs::nowSeconds`, the parameter of `RecordPanel::refresh` and `applyView`, and
    the second argument at src/MainComponent.cpp:4336-4337 and src/ui/RecordPanel.cpp:175, :214, :227.
(b) [S4] `AudioEngine::onDeviceStateChanged` (AudioEngine.h:52, AudioEngine.cpp:184-185): its one assignee goes, so
    it goes. `publishDeviceStatus()` stays.
(c) `PresetManager::LoadStats`: every field stays. It is the loader's report (three of its fields have no reader
    under src already, R24). `dropped`, `mappingsTotal` and `droppedDescriptions` are read by B04's log twin;
    `legacyFile` is how tests/test_preset_manager.cpp:259, :285 pin the old-file path, which stays.
(d) Comments re-worded: src/recording/RecorderHost.cpp:285-290, RecorderHost.h:72 and :75-79,
    src/recording/RoutineEngine.h:48 (notify is the app-log channel; no notice key, no "notice INVARIANT").
(e) [S3] `RoutinePad::kWarning` (src/ui/RoutinePad.h:60) goes with the "!".
(f) [S2] A log twin for B03: each slot path (:395-397, :4431-4433) logs one line when a recall dropped mappings.
(g) Tokens. NT-L3 adds: `RecordPanelNoticeKey`; `nowSeconds` (in the three RecordPanel files only); `kWarning` (in
    the RoutinePad files only); "notice INVARIANT"; and five alert titles that have no log twin and no other use:
    "Legacy Preset", "Some Mappings Dropped", "ISF Import Successful", "Import Failed", "ISF Import Rejected".
    NOT tokens: "Open Composition" and "Load Deck" (a chooser title and an undo name use them, :3183, :3372); the
    failure sentences themselves (their log twins keep the same words). NT-L4 adds `onDeviceStateChanged`.

AM-20 probe-async-load.py, THE MOVED CLAUSES (plan N6; R21).
- a4(f2)'s end clause is NOT changed: after a trigger and a cancel it expects the trigger's text (top.png, :846-857).
  That is a state text; it is true on P0 (through the hold) and after the lane (the trigger writes the label
  itself). The plan's "file_label after the cancel == file_label before the load" is right for a3(c) only (:770-777).
- a4(f1) and the "mid" halves of a4(f2), a5(a), a6(b): `load.staged == 1` in an /api/state read taken right after
  that same barrier.
- Completion clauses a2(c), a5(a), a5b, a6(b), a9: AM-13, then the row's model fact.
- The rows' timing polls (`wait_until(num_decks() == n)`, :881, :926, :959) are not this lane's: SF-2.

AM-21 A REFUSED RECORD (plan section 7 checks 4 and 9, 5.5). Check 9 ("by feel") is replaced by a performable check
(section 7, check 5) and question Q5. Nothing new is built: after a refused Record the button still reads "Record
Take" and the line still reads "Ready. ..." -- that unchanged state is all he gets, and he is asked whether it is
enough.

AM-22 ORDER (plan section 4 "ORDER", fork F7). The lane starts after the sync dial (bf2) AND the transport lane
have merged. S0 re-derives every file:line, the six counts, the token table and R1 / R2 on that base. The transport
lane's count guard G-N1 is relative to its own base and is not a test in the tree (ruling-transport.md:937-941): it
does not pin this lane's counts. S1 alone (witness routes and probe moves, nothing on screen) MAY merge earlier:
H-3, default no.

AM-23 SETTLED FACTS (plan section 2 "Not established", S0, NR15, risk R4). U1 is VERIFIED (R17): the box is a JUCE
component. FN-1 and FN-2 leave S0's list; S0 only re-checks that CMakeLists.txt still pins JUCE 8.0.4 and that no
line sets native alerts. `modals` and `dismiss_modals` are unconditional. NR15 always runs. U3: no alert site and no
label line names the settings file (all 12 alert sites and 46 label lines are in this ruling's table), so a failed
settings write shows nothing today -- VERIFIED by that enumeration; the write's callers were not read.

AM-24 DOCS (plan section 6, additions). `.harmony/APP-INVENTORY.md` :109 and :357 (the ISF alert rows) with :85 and
:232. docs/claude/recording.md "Surfaces": the audio clause is present tense (" without audio" = no audio is being
captured now). docs/claude/testing-eyes.md: the new routes of AM-5, `save_failed`, `modals`, and the settle protocol
of AM-13 in three lines. `.harmony/RIG-RULES.md` section A, the /api/composition rule: "wait on
`load.timing.loads`, then one /api/debug/ui_text, then read once" replaces "wait on /api/debug/ui_text". The CLAUDE.md
line of the plan names `reportSaveFailed` and its caller table.

## 4 FINAL BUILD STAGES + ORDER

One builder context per stage. Every stage: its cases are written first and shown RED by name on the stage's base (a
case that never failed is struck and reported), then GREEN at its head. No stage adds an on-screen sentence.
The lane starts after bf2 and the transport lane have merged (AM-22). The two lanes are never merged into each other
mid-flight.

- S0 RE-BASE NOTES (no code). On the base, into the lane report: the file:line of every row of section 6 and of every
  site of plan N5 and AM-19; the six counts (pin: `setFileLabel(` 46, `fileLabel_.setText` 3, `setNotice(` 6,
  `showMessageBoxAsync` 13, `showOkCancelBox` 3, `dispatch.notify(` 28); the token table of AM-15(a) (pin: 57 lines,
  7 files); the 16 label lines of AM-18; R1 and R2 re-read -- if `loadTiming_.end()` no longer follows the model
  change, STOP and report (AM-13 rests on it); what every REST route used by a row of 5.3 answers (AM-14); any text
  sink bf2 or the transport lane added; `populateSlotMenu` (plan T10); the Record tab's name in the browser panel;
  the identifier list NT-L2 needs; the test binaries' paths; CMakeLists still pins JUCE 8.0.4 and no line sets native
  alerts. Harmony constraint: freeze P0, a copy of the pre-lane main app bundle (never re-signed).
  Proves: the tables.
- S1 WITNESSES + PROBE MOVES. Nothing on screen changes. (a) The settle helper (AM-13) and the three probes moved
  (plan N6 with AM-20). (b) The test routes of AM-5, with GET /api/debug/ui_texts as AM-14 specifies (every Label,
  TextEditor and Button under every top-level window: component path, id, class, text, showing, enabled, bounds, text
  colour; `modals`; later `save_failed`). (c) `.harmony/probe-notices.sh / .py / .json` with every row of 5.3.
  Files: the three probes, src/api/ApiServer.*, src/MainComponent.cpp (the route wiring block only),
  docs/claude/testing-eyes.md.
  Proves: the moved probes GREEN on P0 and their registered RED arms still RED; MU-P1 and FM-1; probe-notices RED on
  P1 in exactly the rows 5.3 names. P1 = the app at S1's head (P0 plus test routes; the six counts unchanged, shown
  in the report). Harmony freezes P1 as the RED arm of every live row.
- S2 TOP ROW AND BOXES. Plan N5's [S2] list, with: AM-7 (T5 dropped), AM-8 (the audio-file failure), AM-9 (folder,
  camera), AM-10 (the re-apply write), AM-1 rows 1-5 and AM-5 (`reportSaveFailed`, its witness), the plan's log twins
  and AM-19(f), the `routineLoadNote` log line.
  Files: src/MainComponent.*, src/core/StagedLoad.h, src/ui/PresetManager.* (the log's source only),
  src/audio/AudioEngine.*, src/ui/FileLabelText.h (new), src/ui/LookAndFeel.h (one comment), tests.
  Proves: NT-U9, NT-U10; NT-L1 (5 caller rows), NT-L2; G-U6's label count; the staged-load binary whole; rows NR1-NR8
  with NR7b, NR15, NR17; mutants MU-N2, MU-N4, MU-N6, MU-N8, MU-N9, MU-N10, MU-N11, MU-N14.
- S3 RECORD TAB, ROUTINE PADS, MIDI LEARN. Plan N5's [S3] list, with: AM-6 (the saved-take fact, both readers), plan
  T2, AM-12 (the tooltip), AM-19 (a), (d), (e), AM-1 rows 6-7.
  Files: src/MainComponent.cpp (the two notify lambdas, :1953-1956, `perfStop`, `perfStopPlay`, the refresh call),
  src/ui/RecordPanel.*, src/ui/RecordPanelModel.h, src/recording/RecorderHost.*, src/recording/RoutineEngine.h (one
  comment), src/ui/RoutinePad.*, src/ui/RoutineDeckView.h, src/ui/MidiLearnOverlay.*, tests.
  Proves: NT-U1 .. NT-U8, NT-U11; NT-L3; NT-L1 raised to 7 caller rows; the record-panel, recorder-host and routine
  binaries whole; rows NR9b, NR9, NR10 .. NR13, NR16; mutants MU-N1, MU-N3, MU-N5, MU-N12, MU-N13.
  The stage ends at the VISUAL WORK GATE (5.5), not at a commit.
- S4 THE DEVICE LINE. Plan N5's [S4] list and AM-19(b); probe-btguard's two absent rows. Separate and last of the
  removals so that an answer to Q2 can still change its form (the plan's Q2 delta).
  Proves: NT-L4; probe-btguard whole (production mode, `audio_deny`, no hardware); row NR14.
- S5 TRIPWIRE, DOCS, FULL RUN. NT-L5; the docs of plan section 6 and AM-24; Boris's page (sections 7 and 8 here);
  the lane report (the six counts before and after, the retired and renamed cases by name, the RED table, what is
  covered by reading only).
  Proves: a full probe-notices run on the lane build and the RED table on P1; G-U2's arithmetic; MU-N7.

## 5 FINAL CONSOLIDATED GATE LIST (pre-registered; the full list; Harmony copies strings only from here)

Arms: P0 = frozen pre-lane main app. P1 = S1's head (P0 + test routes). L = the lane head.
A pre-registered bar is never loosened: it is met, or it is reported as not met.

### 5.1 Unit cases (each a TEST_CASE whose name begins with the id; RED arm = the stage base, by name)
- NT-U1 RecordPanelModel: idle, `takeFolder` set, `lastSavedTakeFolder` empty -> the line is "Ready. No take
  loaded."; Show in Finder is disabled, `revealFolder` is empty, its tooltip is "No take yet.".
- NT-U2 RecordPanelModel: idle, `lastSavedTakeFolder` = ".../jam.adna-take" -> "Ready. Last take: jam"; Show in
  Finder enabled on that folder.
- NT-U3 RecordPanelModel: recording, frames written, `audioCapturing` false -> the line holds " without audio" and
  neither "from live input" nor "from audio file"; `audioCapturing` true -> "from live input".
- NT-U4 RecordPanelModel: playing with unresolved 2, moved 1, skipped 3 reads exactly "Playing 0:10 / 1:00".
- NT-U5 RecordPanelModel: recording with gaps 2 holds no "gap".
- NT-U6 RecorderHost: after an arm refused at the missing-overdub-audio refusal, `takeFolder` IS the refused folder
  and `lastSavedTakeFolder` is empty; a disarm that wrote take.json sets it; a disarm whose take folder cannot be
  written returns not-ok and leaves it unchanged.
- NT-U7 RecorderHost: `audioCapturing` is true while the tap runs and false from the tick that sees it stopped.
- NT-U8 RoutineDeckView: a slot with unresolved 2, preambleUnresolved 1, skipped 3 -> the pad's tooltip equals the
  tooltip of the same slot with zeros.
- NT-U9 fileLabelAudioSourceText: mic -> "Mic: <status>"; file loaded -> its name; no file -> "No file loaded".
- NT-U10 the save-failed strings: titles "Save Composition", "Save Deck", "Save Preset", "FX Save", "Save Take"; text
  "Save failed: " + path. (Narrow form of AM-1: the first two titles.)
- NT-U11 RecordPanelModel: replaying with audio, not recording -> the Record button's tooltip is "Records a new take
  over this take's audio, from the current playback position. The loaded take is kept."
- NT-L1 boxes (AM-15 a-e). NT-L2 the file label: `fileLabel_.setText` exactly 1 (the setter); for every
  `setFileLabel(` call, its string literals are in { "", "No file loaded", "Mic: ", "MilkDrop: ", "Camera: ",
  "Folder: ", " (", " images)", " frames)" } and its identifiers are in S0's pinned list (which holds
  `fileLabelAudioSourceText`). NT-L3 the notify sinks: the bodies of the two `dispatch.notify = [this]` lambdas hold
  `std::cerr` and no other statement; no line under src holds `setNotice`, `noticeLabel`, `warningLabel_`,
  `routineNotice`, `kNoticeSeconds`, `kArmedWarnSeconds`, `noticeKeyOf`, `warningText`, `lastMidiMessage_`,
  `LabelHold`, `loadingLabel`, `doneLabel`, `queueFullLabel`, `sourceGoneLabel`, or a token of AM-19(g).
  NT-L4 the device line: no line under src holds `audioDeviceNotice`, `audio_notice`, `onDebugAudioNotice`,
  "plug one in", `onDeviceStateChanged` (not built under Q2's other answer). NT-L5 the tripwire: the pinned per-file
  count of `juce::Label` and `juce::TextEditor` members under src/ui and src/MainComponent.h (plan N7).
  All five live in tests/test_render_thread_lint.cpp, tags [lint][notices], comments counted.
- Mutants (the builder shows each RED once, by the named case or row, then restores; the restore rebuild is checked
  per RIG-RULES A):
  MU-N1 `setNotice` back in a notify lambda -> NT-L3.
  MU-N2 `setFileLabel("Saved: " + ...)` back -> NT-L2 and G-U6.
  MU-N3 the idle line reads `takeFolder` again -> NT-U1.
  MU-N4 `writeDeckFile` no longer calls `reportSaveFailed` -> NT-L1 and NR8.
  MU-N5 the audio clause ignores `audioCapturing` -> NT-U3.
  MU-N6 a second `showMessageBoxAsync` -> NT-L1.
  MU-N7 a new `juce::Label` member in RecordPanel.h -> NT-L5.
  MU-N8 the `testMode_` guard in `reportSaveFailed` inverted -> NT-L1 (c), NR7 (`modals` not empty in test mode),
        NR15 (`modals` empty in production).
  MU-N9 the box call removed from `reportSaveFailed`, the witness kept -> NT-L1 (b) and NR15.
  MU-N10 `saveComposition`'s failure branch drops the call -> NT-L1 (d) and NR7b.
  MU-N11 the Save As chooser callback calls `reportSaveFailed` again -> NT-L1 (d). (No live arm: a chooser.)
  MU-N12 `revealFolder` reads `takeFolder` again when idle -> NT-U1.
  MU-N13 `perfStop` drops the call -> NT-L1 (d) and NR16. (Not built in the narrow form.)
  MU-N14 the audio-error lambda no longer writes the label -> NR17.
  MU-P1 (a probe mutant) the settle helper without step 3's `loads` clause and without step 4 -> FM-1's reading.

### 5.2 Unit gates (Harmony, on L)
- G-U1 build rc 0.
- G-U2 ctest serial: "100% tests passed, 0 tests failed out of N"; N counted by Harmony on L and equal to (N on the
  base) - 8 + 16, plus any test S0 adds, by name. The 8 retired names are absent from `ctest -N`; the one renamed
  case is present under its new name; no other name of the base list is absent (diff of the two `ctest -N` lists).
- G-U3 each NT case RED by name on its stage base (the builder's log), GREEN on L.
- G-U4 mutants MU-N1 .. MU-N14 RED by the named case or row (MU-N13 not in the narrow form); MU-P1 per FM-1.
- G-U5 the TSan unit gate and the ASan unit gate as registered (5 / 5, 0 warnings; "PROBE-ASAN-UNIT GREEN").
- G-U6 the six counts on L: `setFileLabel(` = (S0's base count) - 16 (pin: 46 -> 30); `fileLabel_.setText` = 1;
  `setNotice(` = 0; `showMessageBoxAsync` = 1; `showOkCancelBox` = 3; `dispatch.notify(` = (S0's base count),
  unchanged (pin: 28).

### 5.3 Live rows -- `.harmony/probe-notices.sh`
open -g, the live lock, quits only its own pid, no Output window, no synthetic input, no screen capture. Final line:
"PROBE-NOTICES GREEN (19 rows)" or "PROBE-NOTICES RED (<row>)". The probe REFUSES to start when an Audio-DNA it did not
start is running. Its takes, folders and files carry the prefix "probe-notices-" and are removed by its trap (folders
it made read-only are made writable first); the takes root is Boris's.
Deny lists: AM-16. Applied to `file_label` (DENY-LABEL) and to every showing text of GET /api/debug/ui_texts
(DENY-ANY). Fixture self-check first (a match is INVALID).
Shared forms: "settle" = AM-13. "barrier" = one GET /api/debug/ui_text. "dump" = GET /api/debug/ui_texts.
"Record tab" = the precondition of AM-14.
Test mode unless said. Each row: driver -> GREEN on L -> RED on P1.
- NR1 POST /api/debug/load_deck (a valid deck), settle (k = 1) -> numDecks + 1; `file_label` equals its value before;
  no deny hit -> P1: `file_label` "Loaded deck: <name>".
- NR2 POST /api/debug/duplicate_deck, settle (k = 1) -> numDecks + 1; `file_label` unchanged; no deny hit -> P1:
  "Duplicated deck: <name>".
- NR3 a composition load (probe-async-load's driver; a fixture with nothing playing), settle (k = 1) -> the show's
  name in /api/composition; every layer's active clip empty; `file_label` == "" -> P1: "Loaded: <name>".
- NR4 a load with one video clip (probe-async-load's fixture): right after the barrier, `load.staged == 1` and
  `file_label` equals its value before the load; then settle -> P1: "Loading <name>...".
- NR5 POST /api/debug/save_composition into a scratch folder -> the file exists; `file_label` unchanged;
  `save_failed.count` unchanged; `modals == []` -> P1: "Saved: <file>".
- NR6 (production mode) POST /api/debug/load_deck with a file that is not a deck; barrier -> numDecks unchanged;
  `load.timing.loads` unchanged; `file_label` unchanged; `modals == []` -> P1: `modals == ["Load Deck"]` (then POST
  /api/debug/dismiss_modals).
- NR7 failed Save As: a scratch folder made read-only (chmod 555); POST /api/debug/save_composition into it ->
  `save_failed` = { count + 1 exactly, title "Save Composition", text "Save failed: <path>" }; `modals == []`; no
  file; /api/composition's name unchanged -> P1: `save_failed` absent.
- NR7b failed plain Save: POST /api/debug/save_composition into a writable scratch folder (the show now has that
  path); chmod 555 that folder; POST /api/debug/save -> `save_failed` count + 1 exactly, title "Save Composition",
  text "Save failed: <that path>"; `modals == []`; the file's bytes unchanged -> P1: `save_failed` absent.
- NR8 failed deck save: POST /api/debug/save_deck into the read-only folder -> count + 1 exactly, title "Save Deck",
  text "Save failed: <path>"; `modals == []` -> P1: `save_failed` absent.
- NR9b refused Record (fresh app, Record tab): POST /api/perf/record {"name":"probe-notices-refused",
  "overdubAssetId":"probe-notices-no-such-asset","audio":false}; barrier; poll /api/perf/status (<= 1 s) until
  `takeFolder` ends with "probe-notices-refused.adna-take" (the refused arm did write it); dump -> `recording` false;
  `lastSavedTakeFolder` ""; the status label == "Ready. No take loaded."; the Show in Finder button not enabled; no
  deny hit -> P1: the status label == "Ready. Last take: probe-notices-refused" and a showing label holds "Could not
  start the take".
- NR9 already recording (Record tab): POST /api/perf/record {"name":"probe-notices-a","audio":false}; barrier; POST
  /api/perf/record {"name":"probe-notices-b","audio":false}; barrier; dump -> `recording` true; `takeFolder` still
  ends with "probe-notices-a.adna-take"; no deny hit. Then POST /api/perf/stop; barrier; dump -> the status label ==
  "Ready. Last take: probe-notices-a"; `lastSavedTakeFolder` ends with "probe-notices-a.adna-take"; Show in Finder
  enabled; no deny hit -> P1: a showing label reads "A take is already recording.", then "Saved: probe-notices-a".
  (This row does not test T1: R8.)
- NR10 (Record tab) POST /api/perf/play with nothing loaded; barrier; dump -> `playing` false; no deny hit -> P1:
  "No take is loaded. Use Load Take... first." showing.
- NR11 (Record tab) the widget census: no showing component whose text colour is kMeterYellow or kAccentCyan among
  the panel's labels; the panel's labels are exactly the pinned list (the status line, "Takes: ...", the captions);
  the status line's bottom to the format row's top == the panel's row spacing; nothing below the Save Routine row
  -> P1: three more labels, two of them between the status line and the format row.
- NR12 (Record tab) POST /api/routine/fire on an empty pad; barrier; dump -> /api/routine/status `lastError`
  "Routine pad <n> is empty."; no deny hit -> P1: that sentence showing.
- NR13 (Record tab) POST /api/routine/save {"fromBar":1,"toBar":2} with no take loaded; barrier; dump -> /api/routine/status `lastError`
  holds "No take is loaded"; no deny hit -> P1: the sentence showing.
- NR14 (production mode, probe-btguard's `audio_deny` lever) input denied -> /api/debug/audio_devices `state`
  "no-input"; ui_text has no `audio_notice` key; no deny hit; `file_label` == "Mic: no wired mic (Bluetooth is never
  used)" in Mic mode -> P1: `audio_notice` == the NoInput sentence. (Under Q2's other answer: `audio_notice` == that
  sentence on L too, and the row checks that the mic-replaced text is gone from src by NT-L3's token "lost - now".)
- NR15 (production mode) the real box: NR7's driver -> `modals == ["Save Composition"]` and `save_failed` count + 1
  -> POST /api/debug/dismiss_modals -> `modals == []`. Then the Quartz count of Audio-DNA windows equals the count
  before the row -> P1: `modals == []` after the driver (the route reaches no box at P1). The probe's trap posts
  dismiss_modals before it quits its app.
- NR16 failed take save (Record tab): the probe makes the folder "<takes root>/probe-notices-ro.adna-take" and makes
  it read-only; POST /api/perf/record {"name":"probe-notices-ro","audio":false}; barrier; POST /api/perf/stop;
  barrier; dump -> `recording` false; `save_failed` count + 1 exactly, title "Save Take", text "Save failed: <that
  folder>"; `modals == []`; the status label does not hold "probe-notices-ro"; `lastSavedTakeFolder` unchanged; no
  deny hit -> P1: `save_failed` absent and a showing label holds "Could not stop the take".
  (Narrow form of AM-1: `save_failed` count unchanged on L; the rest as written.)
- NR17 failed audio-file load (last test-mode row): the probe writes a 0.2 s wav "probe-notices-tone.wav" and a text
  file "probe-notices-not-audio.txt". POST /api/perf/record {"name":"probe-notices-t1","audio":false,
  "audioFile":<the wav>}; barrier; POST /api/perf/stop; barrier -> `file_label` == "probe-notices-tone.wav". POST
  /api/perf/record {"name":"probe-notices-t2","audio":false,"audioFile":<the text file>}; barrier; POST
  /api/perf/stop; barrier; barrier -> `file_label` == "No file loaded" -> P1: `file_label` == "Failed to load:
  probe-notices-not-audio.txt".
- Not drivable by REST, covered by lint and reading only (reported as such in the lane report): the preset Save /
  Load / slot texts and the two preset callers of the box (AM-1 rows 4-5), `perfStopPlay`'s caller (row 7), the Save
  As chooser callback, ISF import, the image folder, the camera, Show in Finder on a missing clip file, the 9th
  queued load, the failed device re-apply (AM-10), MIDI Learn's "Last:", the pad's "!" outside bar B6.

### 5.4 Re-runs (Harmony)
- probe-asan-live: "PROBE-ASAN-LIVE GREEN (7 steps, ...)" on L's ASan build and "RED (step L1)" on the wiring mutant.
- probe-async-load and probe-btguard whole on L, their RED tables unchanged in the rows plan N6 and AM-20 did not
  touch.
- probe-boxes (63 / 0 / 1 BLOCKED, rc 3) and probe-deck-tabs unchanged.
- idle-paint: the registered bars (the lane removes three 4 Hz label writes and adds none).

### 5.5 VISUAL WORK GATE (stage S3 for the Record tab; S4 for row 1)
States (set by REST / test routes; captured by Quartz window id only; a manifest carries the model facts and the dump
of each):
- V1 Record tab, idle, nothing recorded. V2 idle after a take ("Ready. Last take: <name>"). V3 recording with audio.
- V4 recording, capture stopped (POST /api/debug/audio_stop mid-take if FM-7 says it stops the tap; else omitted and
  said so). V5 a take loaded. V6 playing with audio (the state that showed the yellow and the cyan line).
- V7 after a REFUSED Record: NR9b's driver on a fresh app. V8 row 1 in the Ok state. V9 row 1 with the input denied,
  Mic mode.
Bars from the dump (pass / fail, no judgement):
- B1 in V1-V7 the Record tab has no showing text widget beyond the pinned list; none is yellow or cyan.
- B2 the status line's bottom to the format row's top == the row spacing in every state (no two-line hole); the Save
  Routine row's bottom is the panel's last occupied y.
- B3 the status line in V1 "Ready. No take loaded.", in V2 "Ready. Last take: <name>", in V7 == V1's; the Show in
  Finder button's enabled state in V7 == V1's (disabled).
- B4 V8, V9: the file label's right edge == row 1's right edge; no yellow label in row 1.
- B5 every text of every dump passes DENY-ANY; `file_label` passes DENY-LABEL.
- B6 as AM-16 (a fired routine with an unresolved timeline: 0 pixels of 0xffcc3333 in the pad rects on L, > 0 on P1;
  else STRUCK and reported).
Questions for the five critic seats (each gets Boris's four sentences verbatim, and the list of texts that are
outside the lane: tooltips, captions, the status line's state forms, "Takes: <path>", the Save failed box):
- visual-design: with the two lines gone, does the Record tab read as one block, or does the status line now float?
- UX: in V7 (a refused Record) can the user tell, from what IS on screen, that no take is being recorded, with no
  sentence telling him? In V4, does " without audio" read as "not recording audio now"?
- graphic-design: is any gap, misalignment or orphaned caption left where a line was removed (Record tab, row 1)?
- logic: name any text in any dump that is on screen because something happened or failed. Name any state text that
  is false for the model facts in its manifest row (the status line, Show in Finder's enabled state, the top line).
- interaction-logic: for each button of the Record tab, is its result visible as a state change (button caption,
  status line, pad row) in the same refresh, now that no sentence confirms it? Name each button whose press can
  leave the screen exactly as it was.

### 5.6 FACTS HARMONY MUST MEASURE (only a run can establish them; the ruling for each outcome)
- FM-1 Is the plan's wait really early? On P0's ASan build, during probe-asan-live L6's load, sample /api/state every
  5 ms: is there a sample with `staged == 0`, `queued == 0` and `timing.loads == L0`? Yes -> MU-P1's RED arm is
  proven by a run. Not seen in 5 runs -> reported "not reproduced by a run; the order is proven by the code
  (:3324 against :3402)". AM-13 stands either way.
- FM-2 Does a read-only take folder make the final take save fail here (NR16's lever)? On P1: NR16's driver, then
  /api/perf/status `lastError` == "could not save take.json at stop". No -> the lever becomes a take name whose
  folder path is a FILE the probe made; the row is otherwise unchanged.
- FM-3 Does chmod 555 on the folder make a save of an existing show file fail (NR7b's lever)? On P1: NR7b's driver,
  the file's bytes and mtime unchanged. No -> the lever becomes `chflags uchg` on the file; unchanged otherwise.
- FM-5 Does the JUCE box of an app launched with open -g take the keyboard or come in front of Boris's work? One run
  of NR6's driver on P1 in production mode with the frontmost app read before and after. Stays behind -> NR6 and
  NR15 run as registered. Comes forward -> they are run only at a time Harmony has told him, or are reported NOT RUN
  with the box resting on NT-L1 (b), (c) and MU-N8 / MU-N9's lint arms (H-6).
- FM-6 Can REST make a routine run that leaves an unresolved timeline (bar B6)? Yes -> B6 as written. No -> struck.
- FM-7 Does POST /api/debug/audio_stop stop the audio tap of a recording take in the capture build (state V4)? Yes
  -> V4 is captured. No -> V4 is omitted and said so; NT-U3 and NT-U7 carry T2.
(There is no FM-4: Boris's check 1 uses the same permission NR7 proves; its Finder step is INFERRED equal to
chmod and labelled so in section 7.)

## 6 THE FINAL TABLE (every on-screen text of classes A and B at 34179a2; one line each)

Lines are src/MainComponent.cpp unless a file is named. "log stays" = the notify seam's std::cerr line; "log twin" =
one `logLine` with the same words (plan fork F2). Defaults of H-1 and Q2 are assumed; their other answers are named.

Class A (event texts)
- A01 | :2988 | "Saved: <file>.json" (Save Preset) | GOES. A preset save that FAILS: the box (AM-1 row 4).
- A02 | :4402 | "Saved: FX_Save_<n>.json" | GOES. The 300 ms button flash stays (colour, not text). A failed FX Save:
  the box (row 5).
- A03 | :3014 | "Loaded: <preset>" | GOES.
- A04 | :395-397, :4431-4433 | "Slot N: <preset>" | GOES.
- A05 | :3031-3040 | box "Legacy Preset" | GOES (an event text: no log twin).
- A06 | :3286-3288; text src/core/StagedLoad.h:59 | "Loading <name>..." | GOES, with the label hold
  (StagedLoad.h:79-117, :3208-3209, :3419-3423).
- A07 | :3360, :3398; text StagedLoad.h:60-69 | "Loaded: <name>" / "Loaded deck: <name>" / "Duplicated deck: <name>"
  | GOES. After a composition load the top line is what the swap's own refresh wrote (AM-7); after Load Deck or
  Duplicate it is unchanged.
- A08 | :3553, :3581 | "Saved: <show file>" | GOES.
- A09 | :3816 | "Saved deck: <name>" | GOES.
- A10 | not in the tree at the pin | "Removed deck ..." | ALREADY GONE.
- A11 | not in the tree at the pin | the "Undo Remove ..." button | ALREADY GONE.
- A12 | :4766-4771 | box "ISF Import Successful" | GOES.
- A13 | :3220-3222 | "Mic "<lost>" lost - now listening on "<open>"." | GOES (S4). While the app listens to the mic,
  the top line names the mic now in use (the re-apply write, AM-10).
- A14 | :5926-5932 | "Saved: <take>" | GOES from the screen (log stays). BECOMES A STATE DISPLAY: the idle line
  "Ready. Last take: <name>", reading `RecorderHost::Status::lastSavedTakeFolder` (AM-6).
- A15 | :5978-5991 | "Playing <take>: the look from when Record was pressed is restored first." and its counts | GOES
  (log stays).
- A16 | :6006-6013 | "Stopped the playback and the take recorded over it. Saved: <take>" | GOES (log stays).
- A17 | :6048-6055 | "Finished <take>: holding the last look." (+ "Listening to the live input again.") | GOES (log
  stays). Its state twin stays: the status line "Finished <take> -- holding the last look"
  (src/ui/RecordPanelModel.h:134-137, :268).
- A18 | :6233-6247 | "Saved routine <name> to pad <n>: ..." | GOES (log stays). The pad shows the name and number.
- A19 | :6290-6308 | "Routine <name>: loops ..., starts ..." | GOES (log stays).
- A20 | :6323-6324 | "Removed the routine on pad <n>." | GOES (log stays).
- A21 | src/recording/RoutineEngine.cpp:460-469 | "Routine <name> started ..." | GOES (log stays).
- A22 | src/recording/RoutineEngine.cpp:758 | "Routine <name>: no beat yet -- the routine starts now." | GOES (log
  stays).
- A23 | src/ui/MidiLearnOverlay.cpp:101-104 | "... | Last: Note 60 vel=..." | GOES (Q3). "Press Escape to exit"
  stays.

Class B (failure texts)
- B01 | src/audio/AudioEngine.cpp:17-18 -> :470-474 | "Audio device: <error>" | GOES (logged). While the app listens
  to the mic the top line BECOMES "Mic: <status now>" after every re-apply, reading `getDeviceStatus()` (AM-10).
- B02 | src/audio/AudioEngine.cpp:63-64 -> :470-474 | "Failed to load: <file>" | GOES (logged). The top line BECOMES
  "No file loaded" (or "Mic: ...") only when it was naming the file the engine dropped, reading
  `AudioEngine::hasFileLoaded()` (AM-8).
- B03 | :397, :4433 | " (check mappings)" | GOES (log twin, AM-19 f).
- B04 + M01 | :3022-3030; src/ui/PresetManager.cpp:400, :428 | box "Some Mappings Dropped" | GOES (log twin).
- B05 | :3492-3496, :3507-3511 | box "Open Composition": could not read / <reason> | GOES (log twin; the load ticket
  still answers Failed to REST).
- B06 | :3557-3563, :3609-3615 | box "Save Composition" / "Save failed: <path>" | STAYS, through `reportSaveFailed`:
  his words ("the only fail message will be a failed save").
- B07 | :3659-3663, :3676-3680 | box "Load Deck": not a deck file / <reason> | GOES (log twin).
- B08 | :3821-3825 | box "Save Deck" / "Save failed: <path>" | STAYS, through `reportSaveFailed`.
- B09 | :3895 | "Show in Finder: not found - <path>" | GOES (log twin).
- B10 | :3636, :3920, :3450; texts StagedLoad.h:76-77 | "Too many loads waiting: ..." / "Duplicate skipped: ..." |
  GOES (log twins).
- B11 | :4499 | "No images found in folder" | GOES (log twin). The running slideshow is kept (AM-9).
- B12 | :4574 | "Camera failed to open" | GOES (log twin). The camera selector goes back to "Off" (AM-9).
- B13 | :4722-4725, :4752-4757 | boxes "Import Failed" / "ISF Import Rejected" | GOES (log twins).
- B14 | :3218-3219 | "No audio device found - plug one in. ..." / "No wired mic found - plug one in. ..." | GOES (S4)
  -- a reading, not his words: Q2, H-2. Other answer: STAYS as the one state line for "no device / no wired mic".
- B15 | :5851-5853, :5902-5904, :5917-5921, :5941-5943, :5955-5959, :6066-6068 | "A take is already recording." /
  "Could not start the take: ..." / "Nothing is recording." / "Could not stop the take: ..." / "Could not load the
  take: ..." / "No take is loaded. Use Load Take... first." / "Could not play: ..." / "Could not repair the audio:
  ..." | GOES from the screen (log stays). What remains is state: the Record button's caption and the status line.
  A take that could not be SAVED at Stop: the box (AM-1 row 6).
- B16 | :4808, :5048, :5676, :5722, :6458 | "...: deck unresolved" / "audio stop refused: ..." | GOES from the screen
  (log stays).
- B17 | src/recording/RecorderHost.cpp:294, :370, :388, :447, :489, :541 | the recorder's fault texts | GOES from the
  screen (log and `lastError` stay). The final save (:388): the box (AM-1 row 6). The audio capture that stopped:
  the status line BECOMES "... without audio ...", reading `Status::audioCapturing` (plan T2, AM-11).
- B18 | :5929-5930, :6009 | ", but its audio had a problem: ..." / "... could not be saved: ..." | GOES from the
  screen (log stays). The overdub that could not be saved: the box (AM-1 row 7).
- B19 | src/ui/RecordPanelModel.h:279-289 | the yellow line's five texts | GOES. The state twin that stays: "Armed,
  waiting for audio...".
- B20 | src/ui/RecordPanel.cpp:163-173, :301-327, :407 | the Save Routine refusal line | GOES. `/api/routine/status`
  keeps `lastError`.
- B21 | src/ui/RoutinePad.cpp:109-117; src/ui/RoutineDeckView.h:126-136, :168, :206 | the red "!" and the tooltip
  prefix | GOES.
- N-NEW1 (not in the inventory) | :1953-1956 | "A recorded clip change aims at a deck that was removed -- skipped." |
  GOES from the screen (its std::cerr line stays).
Narrow form of AM-1: the "the box" halves of A01, A02, B15, B17 and B18 read "nothing" instead.

Outside A and B, touched because of them
- C02 "Mic: <input> @ <rate>Hz" (:300, :486, :538, :5842): STAYS. A state text written at audio-source events, true
  when written; a later owner of the label (a clip fire) replaces it.
- C03 "MilkDrop: ...", "Camera: ...", "Folder: ... (n images)": STAY.
- C04 "Ready. Last take: <name>": STAYS, re-based on the saved-take fact (Q4a). The counts "unresolved: / moved: /
  skipped:" (RecordPanelModel.h:117-124) and "N audio gaps" (:260-261): GO (Q4b). "lanes" and "moves": STAY.
- E07 "Record Over starts a new take on top of this audio; the loaded take is kept." (:300-301): the line GOES with
  its widget; "The loaded take is kept." joins the Record Over tooltip (AM-12).
- E09 the colour flashes: STAY.

## 7 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

1. A save that cannot be written. Open a show that lives in a folder. In Finder select that FOLDER, File > Get
   Info, Sharing & Permissions, set your own name to "Read only" (INFERRED: the same permission the gate uses). In
   the app press Save -> ONE box: "Save failed: <path>" -> wrong: nothing happens, or any other box. Set the folder
   back to "Read & Write". The same with Save Deck for a deck that has a library file.
   Know this: if the drive is unplugged or the folder is gone, Save opens the Save As window instead of the box. That
   is how the app works today and it does not change.
2. Open a show, save it, load a deck, duplicate a deck, recall a preset slot, press FX Save -> the top text line
   never says Saved / Loaded / Loading / Slot -> wrong: any such word.
3. Open a file that is not a show -> nothing changes, no box -> wrong: a box.
4. Record tab: press Record, press Stop -> the line reads "Ready. Last take: <the name>"; no yellow or blue sentence
   at any moment; no empty gap where the lines were -> wrong: a sentence, or a gap.
5. A Record that cannot start. In Finder set Documents/Audio-DNA/Audio to "Read only". With "Record audio" on, press
   Record -> the button stays "Record Take", the line does not change, nothing else appears, and Show in Finder does
   not open a folder that was never made -> wrong: the line names a take, or a sentence appears. Set the folder back.
   (This is Q5: say if an unchanged button is not enough of a sign.)
6. A take that cannot be saved. Set Documents/Audio-DNA/Takes to "Read only". Press Record, then Stop -> the box
   "Save failed: <the take's folder>"; the line does not name that take -> wrong: no box (unless you answer Q1 the
   other way), or the line names it. Set the folder back.
7. Unplug the wired mic while the app listens to it -> no yellow sentence; the top line names the mic now in use or
   says there is no wired mic -- until you fire a clip, which writes the clip's name there -> wrong: a yellow line.
   (This is Q2.)
8. Fire a routine whose layer you deleted -> the pad plays what it can; no red "!" -> wrong: a red mark.
9. MIDI Learn: move a knob -> only "Press Escape to exit" under the title -> wrong: "Last: ...". (Q3.)
10. Open a big show with videos -> the old show keeps playing until the new one appears; no "Loading..." text.
11. While a picture folder is running as a slideshow, choose another folder that has no pictures -> the slideshow
    keeps going as before; no text -> wrong: the pictures stop changing.
12. Play a take with its audio, hold the mouse over "Record Over" -> the tip ends with "The loaded take is kept.";
    no yellow or blue line under the status line -> wrong: either line is back.
13. By feel, after a rehearsal: is there a moment where you pressed something and could not tell whether it worked?
    Name the button. (The critics' list of buttons whose press can leave the screen unchanged comes with the page.)

## 8 BORIS QUESTIONS (plain words; each has a default; nothing waits -- the default is built)

- Q1 (his page's question 12, asked again with the true facts). You said: "ok. the only fail message will be a failed
  save. remove all others". Which saves does that cover? Default: every save you press -- Save show, Save Deck, Stop
  Recording (the take), Save preset, FX Save. If the file could not be written you get the one box, "Save failed:
  <path>". The other answer: only Save show and Save Deck; then a recording, a preset or an FX Save that could not
  be written shows nothing. To know: a recording whose last save failed may still be on disk as a partial take (the
  app saves while it records) and Load Take can open it; the Record line will not name it.
- Q1b. "Save Routine" that is refused (no take loaded, the pads are full, the take has no bars): default nothing --
  no new pad appears. It writes no file, so it is not a failed save. Keep it silent?
- Q2. When no microphone or sound device is found, the yellow line "No wired mic found - plug one in" goes with the
  rest. Default: it goes. Then a dead sound input has no words on screen once you have fired a clip -- you see flat
  meters and "---" for the tempo. Keep that one line as the sign for "no sound input", or not?
- Q3. In MIDI Learn the small text "Last: Note 60 ..." shows the last message your controller sent. Default: it goes.
  Keep it as a "the controller is alive" monitor, or not?
- Q4. Record tab, the one line that stays: (a) when idle it reads "Ready. Last take: <name>" -- default: it stays,
  and only for a take that was really saved; (b) while a take plays it also counted "unresolved / moved / skipped"
  and "audio gaps" -- default: those counts go, with the red "!" on a routine pad.
- Q5. A Record that cannot start (the audio folder cannot be written, or the disk has under 2 GB free): default
  nothing -- the button simply stays "Record Take". Is that enough, or do you want a sign for "it did not start"?

## 9 HARMONY'S DECISIONS (each with a default)

- H-1 Which failed saves show the box until Boris answers Q1. Default (the architect's recommendation): AM-1's seven
  callers. Other: the narrow form, three callers -- Harmony's recorded reading (backlog BF34). Switching later costs
  one table (NT-L1 d), four call lines, NT-U10, NR16's form and MU-N13. To weigh: the box is modal. While it is
  open the keyboard goes to the box until Return is pressed (INFERRED from JUCE's modal state, R17; not read
  further) -- true today for all 12 boxes, and after the lane only for a failed save. The page's Q1 should say it.
- H-2 The device line until Boris answers Q2. Default: it goes (S4 as planned). Other: keep the widget for the two
  no-device states and drop the mic-replaced branch; then NT-L4 is not built, probe-btguard's clauses keep reading
  `audio_notice`, and the token "lost - now listening" joins NT-L3.
- H-3 S1 early. Default: no -- the whole lane after bf2 and the transport lane. Other: S1 alone merges before the
  transport lane starts (it changes nothing on screen and takes the three label waits out of probe-asan-live, which
  the transport lane re-runs). Cost: its own S0 on the earlier base and one more re-base of S2-S5.
- H-4 The failed audio-file load. Default: AM-8, a label write only when the label names the file that is gone.
  Other: reorder `AudioEngine::loadFile` so a failed load keeps the old file -- no label write at all, but it changes
  what he hears and needs an engine test.
- H-5 The camera's name in the top line after the camera closes (SF-1). Default: left as it is today. Other: one
  more label write when the camera closes -- a new state write nobody asked for.
- H-6 Rows NR6 and NR15 put a real box of the probe's own app on screen for under a second. Default: decided by
  FM-5 (run if the box stays behind his work; else run at a told time, or report NOT RUN and rest on NT-L1 b / c and
  the lint arms of MU-N8 / MU-N9).
- H-7 probe-async-load's timing polls (SF-2). Default: untouched by this lane. Other: S1 moves them to
  `load.timing.loads` -- it ends the unlocked read during a load but changes what a registered timing bar measures
  (the end of the landing, not the first moment two decks can be read), so the bar must be re-registered, not reused.
- H-8 The two tripwires (NT-L1 a, NT-L5): every later UI lane pays one table edit each. Default: both in (plan fork
  F6). Other: NT-L1 (b)-(e) only and no NT-L5; then a new Label fed at an event passes every gate.
- H-9 The renamed test (AM-17). Default: renamed and listed. Other: the stale "12" stays in its name.
- H-10 JUCE's source was read outside git (R17) although the dispatch says "source only at the pin". Default: accept
  it as VERIFIED (the tag is pinned in CMakeLists.txt at the pin and the tree says 8.0.4); S0 re-checks. Other: treat
  FN-1 as open until S0; nothing else in this ruling would change except NR15's "always runs".

## 10 SIDE FINDINGS (outside this lane)

- SF-1 The top line keeps "Camera: <name>" after the camera is closed, by Off or by a failed open: `closeCamera`
  writes no label (src/MainComponent.cpp:4586-4594). The top line is one widget with five owners (audio source, fired
  clip, folder, camera, MilkDrop) and the last writer wins; a truthful display per source is a new state widget
  (plan T7), not this lane.
- SF-2 probe-async-load a5, a5b and a6 wait with `wait_until(num_decks() == n)` at 0.02 s steps while a load is
  staged (:881, :926, :959). `num_decks()` is GET /api/composition (:322-326, :368-369): it is the unlocked read
  RIG-RULES A forbids (the ledger's SF-12). Not introduced here. A fix: time the landing on `load.timing.loads`.
- SF-3 After this lane a refusal at funnel level ("A take is already recording.", "No take is loaded...") leaves no
  trace a machine client can read: the perf routes drop it (R9) and `lastError` is not set for it. The log has it.
  A `lastRefusal` field in /api/perf/status would close that; not this lane.
- SF-4 Writes whose result the app ignores today: Collect Files' copy of the show (:6694), Export Bindings (:7291),
  Save Layout (:7332). Failed saves that are silent before and after the lane; on Boris's page under Q1.
- SF-5 A take whose final save failed leaves its audio in the store and maybe a partial take.json (R7): nothing
  cleans it up or marks it.
- SF-6 `AudioEngine::loadFile` drops the playing file before it knows the new one can be read
  (src/audio/AudioEngine.cpp:54-66): a failed load silences the music. H-4's other answer would fix it.
- SF-7 The inventory (facts-notices.md section 1, fact 2) and plan fact F5 say the funnels' return values reach
  REST. For the perf routes they do not (R9). Correct the inventory when it is next touched.
- SF-8 `.harmony/APP-INVENTORY.md` :109 and :357 cite the ISF alerts at ":2418" and ":2426-2454"; at the pin they are
  at :4722-4771. Stale before this lane.
- SF-9 The transport ruling's guard G-N1 counts five strings relative to its own base
  (ruling-transport.md:937-941). It is not a test in the tree, so this lane does not break it. If the transport lane
  left it as a pinned lint after all, S0 finds it and S2 / S3 re-pin it.
- SF-10 Two handoff-ledger lines close with this lane: "probe-asan-live waits on three event texts" (AM-13) and
  "Composition::routineLoadNote has no reader and no log line" (the plan's log line, S2).

## 11 RISKS (the strongest counterargument first)

- K1 THE STRONGEST: this ruling WIDENS the one exception Boris granted. He was asked about two boxes and said "ok."
  Harmony recorded "exactly these two". On the architect's reading of the rest of his sentence the default now has
  five kinds of save and seven callers, two of which (Save Preset, FX Save) never had a failure message. If he meant
  the two boxes, the lane ships boxes he did not ask for, and a box is modal: it takes the keyboard until Return.
  His own lean was against the box at all ("how would a save fail? not sure we need that").
  Why the default stands, as a default only: his sentence names no kind of save; a take that silently fails to save
  is the costliest outcome on the list and repeats for every later take with the same cause; the two preset rows
  exist because the lane itself would otherwise erase the only sign of a failed preset save; and the whole
  difference is one pinned table. It is Harmony's to adopt (H-1) and his to answer (Q1). Cheapest refuting test:
  his answer to Q1.
- K2 "Truthful state" is where new text comes back in (the plan's own R1). After this ruling T5 -- the one write
  that was an announcement -- is gone. What remains: T1 (a re-based line and button), T2 (a clause), AM-8 (a label
  write only when the label names a file that is gone), AM-10 (the mic status after a failed re-apply), and five
  words in a tooltip (AM-12). Each stops a display from saying something false and is still true a minute later with
  nothing else happening. If Harmony disagrees: T1 falls back to "Ready. No take loaded." always (Q4a's other
  answer); AM-8 to no write (the top line may then name a file that is gone); AM-12 to no tooltip change. T2 and
  AM-10 cannot be dropped without leaving a false line.
- K3 The settle protocol rests on where `loadTiming_.end()` sits (after the model change, :3402). Two lanes merge
  before this one. S0 re-reads it and STOPS if it moved; FM-1 measures it.
- K4 The production-mode rows put a real box on Boris's screen for a moment (NR6 on P1, NR15 on L). FM-5 and H-6.
- K5 Covered by reading and lint only, with no failing run behind them: the two preset callers of the box, the
  overdub caller, the Save As chooser callback, the image-folder and camera fixes, the failed re-apply write. A
  mistake there passes every gate except NT-L1's table, NT-L2's allowlist and review. The lane report lists them.
- K6 The device line's default may be wrong the other way (SO-ST-2's case): with it gone, a dead sound input has no
  words once a clip is fired. Q2 says so plainly; S4 is one separable stage.
- K7 NT-L1's token table and NT-L5 are tripwires, not classifiers: a builder can raise a row to make a new dialog or
  label pass. It is a diff a reviewer sees; it is not proof (plan R5).
- K8 Line numbers are the pin's. On the real base every one moves; S0's table is the only one a builder uses.
- K9 Not verified by me (section 1, last paragraph): `populateSlotMenu`, the settings write's
  callers, the member list of RecordPanel.h, the modal box's hold on the keyboard, the Finder
  step of check 1. Each carries its label where it is used.
- K10 The notify seam keeps about 28 calls whose sentences read like screen text and now reach only the log (plan
  fork F1). A later builder may wire a sink back. NT-L3's lambda half is the guard.

## 12 HOW A BUILDER READS THE PLAN

- The plan body is the base. Where this ruling and the plan disagree, the ruling wins. Where the ruling is silent,
  the plan stands.
- Built from the plan body as written: section 1; facts F1-F4, F6-F11, F13-F25 as S0 re-finds them; N1 (the list);
  N2 rows A06, A23, A04, C03, C04, E09, B21 and B19 lines 1, 2, 4, 5; N3 T2, T3, T6, T8, T9, T10; N5's two design
  choices (the notify seam stays as the log channel; log twins) and its by-file lists; N6's probe-btguard paragraph;
  N7's NT-L2, NT-L3, NT-L4, NT-L5; forks F1, F2, F3, F6, F8, F9, F10, F11; section 6 (docs).
- Built from the plan body AS AMENDED: fact F5 (AM-14) and F12 (AM-12); N2 rows B14 (AM-3), C02 (AM-10), E07 and
  B19 line 3 (AM-12); N3 T1 (AM-6), T4 (AM-8), T7 (AM-3); N5's lists (AM-19); N5's retired tests (AM-17); N6's
  probe-asan-live and probe-async-load paragraphs (AM-13, AM-20); section 6 (AM-24).
- Built from this ruling ONLY (the plan's text is replaced): N3 T5 (dropped, AM-7); the folder and camera rows
  (AM-9); N4 whole (AM-1, AM-2); N7's NT-L1 (AM-15); section 4, stages and order (section 4 here); section 5 whole
  (section 5 here -- gate strings are copied only from it); section 7 (section 7 here); section 8 (section 8 here);
  forks F4, F5, F7.
- Defaults a builder builds without asking: H-1's seven callers, H-2's "the line goes", H-4's AM-8, H-5's "left".
  Harmony's adoption text may change any of them before a stage starts; the stage prompt then says which.
- A builder never adds a caller of `reportSaveFailed` beyond the table, never adds an on-screen sentence, and never
  loosens a row: a row that cannot be met is reported as not met.
- A line that neither the plan nor this ruling covers: stop and report it. Do not invent a text for it.

STATUS: DONE
