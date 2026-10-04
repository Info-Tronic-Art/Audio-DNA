# Plan: the NOTICES lane -- remove every on-screen event text and failure text, app-wide, except a failed save

Architect plan, s-rta-1003b. Pin: main 34179a2 (every file:line below was read at that commit with `git show` /
`git grep`; nothing was built, run or launched). Labels: VERIFIED = read at the pin; INFERRED / ASSUMED = said so where used.
Row ids (A01.., B01.., C.., E.., M..) are those of `.harmony/.reports/s-rta-1003/facts-notices.md`.

## 1 GOAL

Boris, verbatim (`.harmony/binding-decisions.md` "No what happened texts", 2026-10-03 15:30:49):
"We don't need any text indicating what has happened or what has happened. That is something that happens online and is
not necessary in this application. It is extra overhead and bloat. Please remove it cleanly and completely."

Boris, verbatim (15:42:01, asked whether texts that were in the app before that day go too, failure reports kept until
he had seen a list): "remove the list entirely and cleanly"

Boris, verbatim (20:58:42, a question and a lean, not a ruling): "how would a save fail? not sure we need that"

Boris, verbatim (20:59:50, asked "keep the two \"Save failed\" boxes as the one exception, or remove them too?"):
"ok. the only fail message will be a failed save. remove all others"

What the lane delivers:
- The app shows what IS, never what just happened. Every row of tables A and B goes, except B06 and B08 (the two
  "Save failed" boxes).
- No state display lies afterwards (N3): three small model facts are added so that the lines that stay read the truth.
- Every probe and test that leaned on a removed text stands on a model fact FIRST (stage S1), before any text goes.
- One pre-registered gate list (section 5) proves it; a deny-by-default lint keeps new texts out (N7).

Harmony constraint: a state display shows what IS; it never says what just happened.
Harmony constraint: Boris is quoted only verbatim; every reading of his words below is Harmony's or the architect's.

How the architect reads "the list" (a reading, not his words): the list he ruled removed without review is tables A and
B of the inventory. So a row that sits IN table A or B is settled by his words even where the inventory flags it EDGE
(A06, A23, B14, B19, B21): it goes. What his words do NOT settle is (i) texts the inventory filed as state (class C)
that the completeness critic moved to A / B, (ii) saves other than a show or a deck, (iii) what a state line must read
once its event twin is gone. Those are N2-N4 and section 8.

NOT in this lane: an "unsaved changes" mark; any new state widget; the Edit menu (BF33); the colour-only flashes
(E09: Sync, FX Save, Resync buttons -- colour, not text, not in his words); tooltips, menu items, captions, empty-state
texts, decision dialogs (classes D, E); the REST / OSC return values and `lastError` fields; the app log.

## 2 ESTABLISHED FACTS (verified at 34179a2)

Text sinks and their writers
- F1 `fileLabel_` has ONE setter, `MainComponent::setFileLabel` (src/MainComponent.cpp:3206-3211); 46 lines under src
  hold `setFileLabel(` (definition + declaration src/MainComponent.h:194 included); `fileLabel_.setText` appears 3
  times (:3210 the setter, :3288 the "Loading" hold, :3422 the cancel release).
- F2 `setFileLabel` diverts into `staged_->label` while a load is staged (:3208-3209); `stagedload::LabelHold`,
  `loadingLabel`, `doneLabel`, `labelAfterCancel`, `queueFullLabel`, `sourceGoneLabel` live in src/core/StagedLoad.h:58-117.
- F3 `audioDeviceNotice_` (src/MainComponent.h:349): set up :478-481, text chosen in `refreshAudioDeviceNotice`
  (:3213-3231: NoDevice / NoInput / MicReplaced), laid out :2762-2766 (it takes row-1 width from `fileLabel_`), read by
  the test hook `onDebugAudioNotice` (:2153-2155) -> `audio_notice` in GET /api/debug/ui_text (src/api/ApiServer.cpp:2105).
- F4 The Record tab's notice has three writers of `RecordPanel::setNotice`: the recorder notify lambda
  (src/MainComponent.cpp:2033-2042), the routine notify lambda (:2065-2069) and -- NOT in the inventory -- a direct call
  at :1953-1956 ("A recorded clip change aims at a deck that was removed -- skipped.", guarded by
  `skippedDeckIdsNotified_`, src/MainComponent.h:679), plus `RecordPanel::runAction` (src/ui/RecordPanel.cpp:217-228).
  Each lambda / the direct call writes std::cerr first (:2034, :2066, :1954).
- F5 28 lines under src hold `dispatch.notify(`: MainComponent.cpp (21: :4808, :5048, :5676, :5722, :5853, :5904, :5921,
  :5931, :5943, :5959, :5990, :6013, :6054, :6068, :6156, :6247, :6276, :6301, :6320, :6324, :6458),
  recording/RecorderHost.cpp (:294, :370, :388, :447, :489, :541), recording/RoutineEngine.cpp (:218 through `notify`,
  callers :469, :688, :758). The funnels also RETURN their refusal strings to REST / OSC / bindings (e.g. :5854, :5905).
- F6 Record tab widgets: `statusLabel_`, `warningLabel_` (yellow), `noticeLabel_` (cyan), `takesRootLabel_`,
  `routineNoticeLabel_` (cyan) -- setup src/ui/RecordPanel.cpp:105-119, :171-173; layout :371-373 and :407; texts applied
  :292-298 and :305. The panel is refreshed from MainComponent's timer (src/MainComponent.cpp:4336).
- F7 The status line, the warning ladder and the notice rule are the pure function `deriveRecordPanelView`
  (src/ui/RecordPanelModel.h:140-304): status :236-277, warning :279-289, notice :291-301.
- F8 The idle status reads `s.takeFolder` (:274-275 "Ready. Last take: <name>"). `RecorderHost::arm` writes
  `takeFolder_` at src/recording/RecorderHost.cpp:185, BEFORE its three refusals (:215, :229, :235), and
  `publishStatus` copies it (:890). So after a refused Record the idle line names a take that never started
  (the model's own comment says so: src/ui/RecordPanelModel.h:24-26).
- F9 `recording_` becomes true only after the refusals (:265); a refused arm never reads "Recording".
- F10 A mid-take fault does not end the take: the rate change (:439-448) and the tap self-stop (:484-490) only set
  `lastError_` and notify; `recording_` stays true. The status line's audio clause is derived from `assetId` and
  `overdub` only (src/ui/RecordPanelModel.h:157, :256-259), so it keeps reading "from live input" after the tap stopped.
- F11 `disarm` returns `ok = saved` (:383-391) and clears `recording_` (:406) whether or not take.json was written.
- F12 The Record button's tooltip in the replay-with-audio state already says what Record Over does
  (src/ui/RecordPanelModel.h:175-176); the cyan advance line (:300-301, E07) repeats it.
- F13 Routine pad: `pad.warning` and the tooltip prefix come from the slot's LAST-RUN record
  (src/ui/RoutineDeckView.h:126-136, :168, :206; src/recording/RoutineEngine.cpp:826-828 written at a run, :893-895
  published for an idle slot); the red "!" is painted at src/ui/RoutinePad.cpp:109-117 and is in the paint key (:13).
- F14 MIDI Learn: `lastMidiMessage_` (src/ui/MidiLearnOverlay.h:55; set :205, :252; cleared :23; painted :102-103).

Alerts
- F15 13 lines under src hold `showMessageBoxAsync`: 12 call sites, all in MainComponent.cpp -- :3024 (B04), :3033 (A05), :3493 and
  :3508 (B05), :3559 and :3611 (B06), :3660 and :3677 (B07), :3822 (B08), :4722 and :4752 (B13), :4766 (A12) --
  and one comment (src/ui/LookAndFeel.h:15, re-worded by S2 so that NT-L1 can count whole lines). `showOkCancelBox`: :3172 (D01), src/ui/CompDecksBrowser.cpp:263
  (D04). Hand-built `juce::AlertWindow`: :3838 (Rename Deck), :6337 (Rename Routine), :6364 (Delete Routine).
  `DialogWindow`: Preferences only (src/ui/PreferencesDialog.h:8).
- F16 THE TWO ROWS THAT STAY, exactly:
  - B06 "Save Composition" / "Save failed: <full path>", WarningIcon. Raised (a) by Save when the show has a path whose
    parent folder exists and `composition_.saveToFile` returns false (:3548-3563); (b) by Save As when
    `saveCompositionTo` returns false (:3609-3615). Both are skipped when `testMode_` (:3557, :3609).
    When the parent folder does NOT exist (a drive that was unplugged), Save opens the Save As chooser instead
    (:3565-3568) -- no box.
  - B08 "Save Deck" / "Save failed: <full path>", WarningIcon. Raised by `writeDeckFile` when `replaceWithText` fails
    (:3814-3826; callers :3765 Save Deck, :3798 Save Deck As). Skipped when `testMode_` (:3821).
  - The test route POST /api/debug/save_composition calls `saveCompositionTo` directly (:2151) and so reaches NO box
    today, in either mode.
- F17 `testMode_` suppresses B05, B06, B07, B08 (:3492, :3507, :3557, :3609, :3659, :3676, :3821); it does NOT gate
  A05, A12, B04, B13.

REST and probes
- F18 GET /api/debug/ui_text answers `file_label`, `audio_notice`, `inspected_layer`, `inspected_clip`,
  `inspector_tab`, on the message thread (src/api/ApiServer.cpp:2073-2111). The /api/debug routes are compiled under
  `AUDIODNA_TEST_SERVER` and need no --test-mode (:299-344; probe-btguard.sh:9 and probe-async-load.sh:16 run
  production mode).
- F19 /api/state "load" carries `staged`, `queued`, `staged_players` and the opener counters from atomics, any thread
  (src/MainComponent.cpp:3233-3238, :5514-5527). GET /api/debug/audio_devices carries `state`
  (ok / no-input / mic-replaced / no-device) and `lost_input` (src/audio/AudioEngine.cpp:202-217).
- F20 /api/perf/status carries `lastError`, `lastFinalizeError`, `finalizeErrors`, `takeFolder`
  (src/MainComponent.cpp:6074-6092); /api/routine/status carries `lastError` (:6386).
- F21 Probes that read an on-screen text (grep over .harmony/*.sh, *.py at the pin): probe-asan-live.sh (waits
  :218 "Loaded deck:", :236 "Duplicated deck:", :269 "Loaded: bf9b-check"; helper :163-198), probe-async-load.py
  (label clauses :718-729, :770-777, :814-856, :880-892, :924-938, :958-973, :1033-1039), probe-btguard.sh
  (`audio_notice` clauses :223, :240, :307, :314, :338, :354, :402, :413). probe-boxes.py and probe-ui-files-rename.sh
  read NO label text (their only hits are deck names "Loaded" and other /api/debug routes).
- F22 Tests that pin a removed text or its machinery: tests/test_staged_load.cpp:70-100, :151-152;
  tests/test_record_panel_model.cpp (rows :94-95, :111, :209-212, :255, :280-290, :313-327, :361, :385, :443-444,
  :457-458; cases at :463, :499, :514, :610); tests/test_routine_deck_view.cpp:69, :200-226;
  tests/test_routine_pad_paint_key.cpp:70. Tests on the notify SEAM (not the widget): tests/test_recorder_host.cpp,
  tests/test_routine_engine.cpp, tests/test_tempo_start.cpp.
- F23 A lint already forbids the deck-change identifiers in src (tests/test_render_thread_lint.cpp:444-451:
  undoHint, "Undo Remove", loadNotice, "Removed deck", ...).
- F24 `AudioEngine::loadFile` drops the old source BEFORE it tries the new file (src/audio/AudioEngine.cpp:54-65); its
  callers write `currentAudioFile_` and the label only on success (src/MainComponent.cpp:316-320). `AudioEngine.h` has
  no "is a file loaded" accessor (grep: only `hasAudioDevice`, :37).
- F25 `Composition::routineLoadNote` (src/model/Composition.h:81) has no reader in src; `migrationNote` is logged only
  (src/MainComponent.cpp:3355-3357).

Not established (used below with its label)
- U1 ASSUMED: whether `AlertWindow::showMessageBoxAsync` shows a JUCE component or a native macOS alert in this build
  (no `isUsingNativeAlertWindows` override exists in src; JUCE's default was not read). Settled by reading in S0 (FN-1).
- U2 INFERRED: the sync dial (bf2) and the transport lane add no text sink (their rulings say so; their trees were not read).
- U3 INFERRED: a failed write of settings.json shows nothing today (no alert / label site names it; AppSettings not read).

## 3 ITEMS

### N1 The list, re-verified at 34179a2

Already gone (the deck change removed them; lint F23 pins it): A10 (Remove Deck sentence), A11 + M02 (Undo Remove
button and its hide call), the deck load notice, the hint case of tests/test_deck_tab_row.cpp, the "<pad> on <deck>"
half of C08 (src/ui/RoutineDeckView.h:226-229).
New since the inventory: N-NEW1 src/MainComponent.cpp:1953-1956 (class B, F4).
Changed: B08 is now `testMode_`-gated (:3821). Every MainComponent line moved (+9 .. +60).

Still there (id | where at the pin | sink | read by REST / test / probe):
- A01 | :2988 "Saved: <file>" | file label | -
- A02 | :4402 "Saved: FX_Save_<n>.json" (flash :4410-4414 stays, E09) | file label | -
- A03 | :3014 "Loaded: <preset>" | file label | -
- A04 + B03 | :395-397 and :4431-4433 "Slot N: <preset>[ (check mappings)]" | file label | -
- A05 | :3031-3040 "Legacy Preset" box | alert | -
- A06 | :3286-3288 "Loading <name>..." (+ release :3419-3423, divert :3208-3209) | file label + LabelHold |
  test_staged_load :70-100; probe-async-load a4, a5, a6
- A07 | :3346, :3360, :3398 "Loaded: / Loaded deck: / Duplicated deck:" | file label | test_staged_load :74-76;
  probe-async-load a2, a5, a5b, a6, a9; probe-asan-live L1, L4, L6
- A08 | :3553, :3581 "Saved: <show file>" | file label | -
- A09 | :3816 "Saved deck: <name>" | file label | -
- A12 | :4766-4771 "ISF Import Successful" box | alert | -
- A13 | :3220-3222 mic-replaced sentence | `audioDeviceNotice_` | ui_text `audio_notice`; probe-btguard
- A14-A20, B15, B16, B18 | :5851-6068, :6153-6324, :4808, :5048, :5676, :5722, :6458 | notify -> `noticeLabel_` |
  funnels' return values to REST stay
- A21, A22 | src/recording/RoutineEngine.cpp:460-469, :758 | notify -> `noticeLabel_` | test_routine_engine:143 (seam)
- A23 | src/ui/MidiLearnOverlay.cpp:102-103 (+ :23, :205, :252) | painted | -
- B01, B02 | src/audio/AudioEngine.cpp:18, :64 -> lambda src/MainComponent.cpp:470-474 | file label | -
- B04 + M01 | :3022-3030; src/ui/PresetManager.cpp:400, :428 (`droppedDescriptions`) | alert |
  tests/test_preset_manager.cpp (2 lines read `droppedDescriptions`)
- B05 | :3492-3496, :3507-3511 | alert | LoadTicket Failed stays (REST)
- B06 | :3557-3563, :3609-3615 | alert | STAYS (F16)
- B07 | :3659-3663, :3676-3680 | alert | -
- B08 | :3821-3825 | alert | STAYS (F16)
- B09 | :3895 | file label | - (test mode records the path instead, :3884-3889)
- B10 | :3636, :3920, :3450 | file label | test_staged_load :151-152
- B11 | :4499 | file label | -      - B12 | :4574 | file label | -
- B13 | :4722-4725, :4752-4757 | alert | -
- B14 | :3218-3219 | `audioDeviceNotice_` | ui_text `audio_notice`; probe-btguard B3, B3b, C2 and the K rows
- B17 | src/recording/RecorderHost.cpp:294, :370, :388, :447, :489, :541 | notify -> `noticeLabel_`; `lastError_`
  -> `warningLabel_` | test_recorder_host (seam, status), /api/perf/status
- B19 | src/ui/RecordPanelModel.h:279-289 | `warningLabel_` | test_record_panel_model :209-212, :313, :610-635
- B20 | src/ui/RecordPanel.cpp:163-173, :301-327, :407 | `routineNoticeLabel_` (+ `noticeLabel_` via notify) |
  /api/routine/status lastError stays
- B21 | src/ui/RoutinePad.cpp:109-117; src/ui/RoutineDeckView.h:126-136, :168, :206 | painted + tooltip |
  test_routine_deck_view :200-226; test_routine_pad_paint_key :70
- N-NEW1 | :1953-1956 | `noticeLabel_` (direct) | -
- M03 (no text exists), M04 (test-only overlay): unchanged, nothing to do.

### N2 The edge rows

Rule used (the architect's, from his two sentences): a text goes when it is on screen BECAUSE something occurred or
failed (it is written at the occurrence, or reports an outcome or a count of outcomes). A text stays when it names the
present condition of something and is recomputed from the model.

| Row | Decision | Why | Runner-up and why it loses |
|---|---|---|---|
| B14 "No audio device found - plug one in..." / "No wired mic found - plug one in..." | GOES with A13: the whole yellow line `audioDeviceNotice_` | It is in the list; it is the yellow line; in Mic mode the same condition is already a state text in the file label ("Mic: no wired mic (Bluetooth is never used)", src/audio/AudioEngine.cpp:137-141, C02) | Keep the line for the two no-device states as a "state". Loses: it reports that the app failed to find a device and tells him what to do -- a failure message, and his last ruling leaves exactly one. Asked as Q2 (default: goes) because with it gone a dead input in File mode has no words anywhere. |
| B19 line 3 "Replaying with the take's audio. Live input is paused until Stop Playback." | GOES (the yellow line goes whole) | In the list. The mode stays visible as state: the status line "Playing m:ss / m:ss", the ticked "Play with audio" switch, the source selector on File, the button reading "Record Over" | Move the sentence into the status line. Loses: a new sentence nobody asked for; four state displays already show the mode. |
| B19 lines 1, 2, 4, 5 (lastError, "No audio is arriving...", refused moves, knob moves) | GO | Failure reports and outcome counts. Line 2's state twin stays: "Armed, waiting for audio..." | - |
| B21 red "!" + tooltip prefix | GOES | It reports the LAST RUN (F13), not a present condition: it stays lit after the show is repaired | Keep as state like the clip cell's red "!" (C07). Loses: C07 is recomputed while the file is missing; this is not. |
| A06 "Loading <name>..." | GOES, with the LabelHold machinery | Progress text: on screen because a load began | Keep as "state: a load is staged". Loses: in the list; the live show keeps playing during the load, nothing on screen is waiting. On Boris's page as a check. |
| A23 "Last: Note 60 vel=..." in MIDI Learn | GOES; "Press Escape to exit" stays (E08) | In the list; it echoes an event | Keep as a live input monitor. Loses by his words; asked as Q3 (default: goes) because it is the only sign in Learn mode that the controller is heard. |
| A04 "Slot N: <preset>" | GOES | Echo of a recall | - |
| C02 "Mic: <input> @ <rate>Hz", the re-apply write (:484-487) | STAYS | State of the open input; the write keeps it true after a device change | - |
| C03 "MilkDrop: ..", "Camera: ..", "Folder: .. (n images)" | STAY | Name the active source | Move "Folder" to A. Loses: it is the only place the slideshow folder is named. |
| C04 "Ready. Last take: <name>" | STAYS, re-based on a saved-take fact (N3) | With every "Saved" text gone it is the only place that tells a saved take from a failed one | Remove it (the critic's class A). Loses: then a take that saved and a take that did not look the same. Q4a, default: stays. |
| C04 outcome counts: "unresolved: n", "moved: n", "skipped: n" (:117-124), "N audio gaps" (:260-261) | GO | The same reports as B19 / B21, in the status line (the critic's 7.2.1: decide together with B21) | Keep as live readouts. Loses: keeping them while the pad's "!" goes rules the same fact two ways. Q4b, default: go. "lanes" / "moves" (:127-130) STAY: the size of the take being recorded. |
| E07 "Record Over starts a new take on top of this audio; the loaded take is kept." | GOES with `noticeLabel_` | Not an event text, but its only widget is the notice line and the Record button's tooltip already says it (F12) | Keep `noticeLabel_` for this one sentence. Loses: a whole line of the panel for a duplicate of a tooltip. In the visual gate's questions. |
| E09 colour flashes | STAY | Colour, not text; not in his words | - |

### N3 State displays stay truthful

Each line: what would lie -> the truthful state -> the model fact it reads -> the change.

T1 Record status after a REFUSED Record (F8). Lie today and after: "Ready. Last take: <the take that never started>".
  Truth: the last take that was actually written, else "Ready. No take loaded."
  Fact: new `RecorderHost::Status::lastSavedTakeFolder`, set in `disarm` only when `take.save` returned true
  (src/recording/RecorderHost.cpp:383), never by `arm`; published by `publishStatus`; added to /api/perf/status.
  Change: src/ui/RecordPanelModel.h:274-275 reads it instead of `takeFolder`. This is also what he sees when a take
  save fails (N4): the line does not name the take.
T2 Record status after the audio capture STOPPED mid-take (F10: "the Record panel must not say Recording after a
  failed take"). The take itself goes on (moves are still recorded), so "Recording m:ss" is true; the lie is the
  clause "from live input" / "from audio file".
  Truth: the existing state wording " without audio" (src/ui/RecordPanelModel.h:258) while audio is not being
  captured NOW.
  Fact: new `Status::audioCapturing` = the tap was started for this take and `tap.isRunning()` at the last tick
  (the value `tick` already reads, src/recording/RecorderHost.cpp:483); added to /api/perf/status.
  Change: the clause is "from ..." only when `audioRequested && audioCapturing`; the "Armed, waiting for audio..."
  branch is unchanged (frames == 0). No new sentence, no colour change.
T3 Record status after a FAILED Stop (take.json not written, F11): `recording` is false, T1's fact was not set ->
  the idle line reads the previous saved take or "Ready. No take loaded." True by T1.
T4 File label after a FAILED audio file load (F24): today "Failed to load: x" (B02); after the removal it would keep
  naming the previous file although the engine has dropped it.
  Truth: "No file loaded" (the text of :221), or the "Mic: ..." state in Mic mode.
  Fact: new `bool AudioEngine::hasFileLoaded() const` (`readerSource_ != nullptr`, message thread).
  Change: the `onError` lambda (:470-474) logs the message (`logLine`) and, on the message thread, clears
  `currentAudioFile_` when the engine has no file and writes the audio-source state text (T6's function).
T5 File label after a composition LOAD: today "Loaded: <name>" overwrites whatever was there; after the removal the
  label would keep naming a clip of the show that was just replaced.
  Truth: the audio-source state. Change: `setFileLabel(done)` at :3360 becomes a write of T6's text. Load Deck and
  Duplicate Deck (:3398) write nothing: what the label names is still there.
T6 One pure function, `fileLabelAudioSourceText(micMode, deviceStatus, fileLoaded, fileName)` -> "Mic: <status>" /
  "<file name>" / "No file loaded" (new header src/ui/FileLabelText.h, the project's pure-model pattern:
  ClipMediaText.h, StagedLoad.h). Used by T4 and T5 only; the nine existing audio-source writes are NOT re-routed.
T7 Audio line with no device: Mic mode -> the file label reads the C02 no-mic state at the moment of the change
  (:484-487, unchanged). File mode -> nothing says it (meters flat, BPM "---"); that is Q2.
  Known and unchanged: any later label write (a clip fire) replaces the "Mic: ..." text; a persistent audio-source
  display would be a new state widget -- NOT in this lane.
T8 Routine pad after an incomplete run: the "!" goes (N2). Nothing lies: the pad shows name, number, state,
  progress. No replacement.
T9 Cyan / yellow Record lines: gone; the rows below move up (N5). Nothing blank is left.
T10 Slot dropdowns, deck tabs, browser lists: already model-driven; the removed echoes were duplicates
  (INFERRED for the slot dropdown's shown name: `populateSlotMenu` was not read; S0 confirms).

### N4 Saves other than a show or a deck (default: no message)

- A take (B17, B18). What he sees when it fails, under the default: after Stop the status line does NOT read
  "Ready. Last take: <that name>" (T1 / T3), and Load Take... does not list it. During the take nothing shows a
  failed periodic save; the log line and /api/perf/status `lastError` keep it.
- A routine (B20). Saving a routine writes nothing to disk: it adds the routine to the show in memory
  (src/MainComponent.cpp:6219-6221); it is on disk when the show is saved. "Could not save the routine" is a REFUSAL
  (no take loaded, bank full, no bars). What he sees: the pad row does not change -- no new named pad.
  /api/routine/status `lastError` keeps the text.
- Settings / venue list: no on-screen text exists today (U3); nothing to remove, nothing added.
- THE DELTA, should Boris answer the other way (built so nothing is redone): S2 makes ONE function,
  `MainComponent::reportSaveFailed(const juce::String& title, const juce::String& what)`, the only place that shows
  a box. Under the default it has three callers (Save, Save As, Save Deck). "A failed take save counts": one more
  caller in `perfStop` and one in `perfStopPlay`, on the branch where `disarm` returned not-ok for a take that was
  recording (title "Save Take", the take folder's path). "A refused routine save counts": one more caller in
  `perfRoutineSave`'s `refuse` (title "Save Routine", the refusal sentence). The lint NT-L1 pins the caller list, so
  either answer is a three-line change plus one table row. No widget is kept "just in case".

### N5 Cleanly and completely

Design choice (fork F1): the `dispatch.notify` SEAM stays as the app-log channel; only its screen sink goes. The two
lambdas keep their std::cerr line and nothing else. Why: the funnels' sentences are also their REST / OSC return
values and their log lines, both of which stay; 3 test files stand on the seam (F22). Runner-up: delete the seam and
every sentence. Loses: it removes the log twin, rewrites about 30 call sites inside functions the transport lane
edits, and retires seam tests for no on-screen gain.

Log twins (fork F2): where a removed FAILURE text had no log line (B02, B04 + M01, B05, B07, B09, B10, B11, B12, B13)
the removed line is replaced by one `logLine(...)` with the same words. Event texts (table A) get none. Also one
`logLine(composition_.routineLoadNote)` next to :3356-3357 (the handoff's ledger; log only). Runner-up: remove bare.
Loses: a failure would then leave no trace anywhere for support.

By file (no code; S = stage of section 4)
- src/MainComponent.cpp / .h [S2]: remove the event writes A01-A04, A08, A09, B03, B09-B12 (lines of N1); `setFileLabel`
  loses its divert branch; :3286-3289 and :3419-3423 go; `staged_->label` goes; :3360 becomes T5; :3398 goes; the
  `onError` lambda becomes T4; alerts A05, A12, B04, B05, B07, B13 go (the `LoadTicket::Outcome::Failed` finishes and
  the early returns stay); B06 / B08 become calls of `reportSaveFailed` (the `!testMode_` test moves inside it; the
  Save As failure is reported inside `saveCompositionTo`, so the test route reaches it too); the browser refreshes
  next to the removed "Saved" writes stay.
- src/core/StagedLoad.h [S2]: `loadingLabel`, `doneLabel`, `labelAfterCancel`, `queueFullLabel`, `sourceGoneLabel`,
  `LabelHold` and the comment block :58, :75, :79-81 go. `Admit::Refuse` keeps refusing.
- src/ui/PresetManager.cpp / .h [S2]: `droppedDescriptions` stays as the log's source (one `logLine` per entry in
  `loadPreset`'s caller); `dropped`, `mappingsTotal`, `legacyFile` stay (tests read them).
- src/audio/AudioEngine.h / .cpp [S2]: `hasFileLoaded()` added; `onError` texts unchanged (they are log text now).
- src/ui/FileLabelText.h [S2, new]: T6 and the two box titles / the "Save failed: " text as constants.
- src/MainComponent.cpp [S3]: the two notify lambdas lose `setNotice` (:2035-2041, :2067-2068); :1953-1956 keeps its
  std::cerr line, loses `setNotice`; `skippedDeckIdsNotified_` stays (it limits the log to once per deck).
- src/ui/RecordPanel.h / .cpp [S3]: `warningLabel_`, `noticeLabel_`, `routineNoticeLabel_`, `notice_`, `noticeAt_`,
  `noticeKey_`, `routineNotice_`, `routineNoticeAt_`, `setNotice`, `forgetNotice`, `applyRoutineNotice`,
  `runRoutineAction` go; `runAction` keeps "run, then refresh at once" and drops the returned string; the Save
  Routine click keeps clearing the name field on success; layout :371-373 keeps the status line only, :405-407 go;
  header comment :16 re-worded.
- src/ui/RecordPanelModel.h [S3]: `RecordPanelNoticeKey`, `noticeKeyOf`, `RecordPanelInputs::notice /
  noticeAtSeconds / noticeKey`, `RecordPanelView::warningText / noticeText / noticeLive`, `kNoticeSeconds`,
  `kArmedWarnSeconds`, the warning ladder :279-289, the notice block :291-301, the comment :15-27 go;
  `playingReadout` loses its three counts (:120-123); the gaps clause :260-261 goes; T1, T2.
- src/recording/RecorderHost.h / .cpp [S3]: T1 and T2's two Status fields; nothing else (lastError, the notify calls,
  the counters all stay: REST and log).
- src/ui/RoutinePad.h / .cpp, src/ui/RoutineDeckView.h [S3]: `warning` (spec, paint key, paint :109-117, the x = 20
  shift), `Pad::warning`, `warningText`, the prefix at :206 go. The slot counters in `RoutineEngine::Status` stay (REST).
- src/ui/MidiLearnOverlay.h / .cpp [S3]: `lastMidiMessage_`, its two `callAsync` writes and the " | Last: " append go.
- src/MainComponent.cpp / .h, src/api/ApiServer.h / .cpp [S4]: `audioDeviceNotice_`, `refreshAudioDeviceNotice`,
  the `onDeviceStateChanged` hook's use (:481, :488), layout :2762-2766 (`fileLabel_` takes the row),
  `onDebugAudioNotice`, the `audio_notice` JSON field and Box member. `AudioEngine::DeviceState`, `lostInput`,
  `/api/debug/audio_devices` stay (state; tests/test_audio_engine_devices.cpp AE3 stays).
- What the freed space becomes (VISUAL change): Record tab -- the two 14-px lines under the status line and the one
  under Save Routine are removed and the rows below move up by exactly their height; no placeholder. Row 1 -- the
  file label always has the full row width.

Tests retired BY NAME (no other test disappears)
- tests/test_staged_load.cpp: "the label texts are today's three texts"; "a cancel restores the label only while it
  still shows the loading text"; "AL5: while held the label keeps the loading text and a cancel shows the latest
  writer's text". In "AL7: a queued Duplicate finds its source deck by id, or is skipped" the two label CHECKs
  (:151-152) go; the case stays.
- tests/test_record_panel_model.cpp: "RecordPanelModel notice -- a notice raised while playing is dropped at the
  finish; one ..." (:463); "RecordPanelModel notice -- shown for kNoticeSeconds, then expires" (:499);
  "RecordPanelModel notice -- shown only while the recorder is still in the situation it was raised in" (:514);
  "RecordPanelModel warning precedence -- lastError beats every other warning" (:610). Rows 1, 6, 7, 8, 9, 10, 11,
  13, 14 lose only their `warningText` / `noticeText` CHECKs; row 2 (:98-111) is re-pinned on `lastSavedTakeFolder`;
  row 8's SECTION "moved and skipped counts are appended when non-zero" becomes NT-U4.
- tests/test_routine_deck_view.cpp: "RoutineDeckView warning: the red ! and a tooltip composed from the counts"
  (:200) becomes NT-U8; the CHECK at :69 goes. tests/test_routine_pad_paint_key.cpp: the line :70 goes; the case stays.
- 8 cases retired in all (3 + 4 + 1). The seam tests (F22) are untouched.

### N6 Probes and tests that wait on a text -- moved FIRST (stage S1, before any text is removed)

Harmony constraint: each moved row is run on the frozen pre-lane app P0 -- GREEN there in its moved form -- and its
registered RED arm is re-run and is still RED for the row's remaining clauses. A moved row with no RED arm other than
the label is listed as such in the lane report with a one-off builder demonstration (the driver POST commented out ->
the row fails).

- .harmony/probe-asan-live.sh, the three waits (L1 :218, L4 :236, L6 :269) and `settle`'s `done_label` (:171-183):
  the wait becomes (1) one GET /api/debug/ui_text right after the command (a message-thread barrier: the command's
  message has run), then (2) poll /api/state `load.staged == 0 and load.queued == 0` at 0.25 s (atomics; never
  /api/composition while a load may land -- RIG-RULES A), same limits (15 s; 40 s for L6), same ASan / health breaks.
  `label()` goes. The VALID clauses (layers, numDecks, inspected_layer) are unchanged and are the row's teeth.
  RED arm: the wiring mutant, "RED (step L1)" (Harmony's, on the ASan build).
- .harmony/probe-async-load.py: completion clauses a2(c), a5(a) second half, a5b, a6(b), a9 -> `load.staged == 0`
  after the barrier AND the model fact already read in the same row (composition name / numDecks / deck names from
  one GET /api/composition after the settle). "Loading <name>..." clauses a4(f1), a5(a) first half, a6(b) ->
  `load.staged == 1` at the barrier. Cancel clauses a3(c), a4(f2) -> "file_label after the cancel == file_label
  before the load" (true on P0 through the hold, true after the lane because a load never writes the label).
  `ui_text()` stays as the barrier; the header text :19-71 is re-worded.
- .harmony/probe-btguard.sh: every `audio_notice ==` clause (:223, :240, :307, :314, :338, :354, :402, :413) ->
  `state` of GET /api/debug/audio_devices ("no-input", "no-device", "ok"), which each row already fetches as `d`.
  B3b's sentence becomes "a label-writing action leaves the state no-input and the file label on 'Mic:'".
  `NOTICE_IN` / `NOTICE_DEV` (:150-151) go in S4, when two rows B3n / C2n are added: `audio_notice` is absent from
  ui_text. `NOMIC` / `mic()` clauses on `file_label` stay (C02 state).
- probe-boxes.py, probe-ui-files-rename.sh: no change (F21).
- Unit tests: the retirements of N5 happen in S2 / S3 with their widgets, not in S1; S1 retires nothing.

### N7 Keeping them out

A lint IS possible for the four routes by which every text of tables A and B reached the screen. It is
deny-by-default on those routes (a new NAME does not walk past: anything not on the allowlist fails). It cannot
classify a sentence. All cases live in tests/test_render_thread_lint.cpp next to F23's case, comments included
(whole lines), over src/**/*.h, *.cpp, *.mm.

- NT-L1 boxes. Lines holding `showMessageBoxAsync`, `showMessageBox(`, `NativeMessageBox`, `BubbleMessageComponent`,
  `CallOutBox`: exactly 1 in src, inside `MainComponent::reportSaveFailed`. `reportSaveFailed(` call lines: exactly
  the pinned table (3 rows: function + title). `showOkCancelBox`, `new juce::AlertWindow`, `DialogWindow`: exactly
  the pinned table (D01, D04, Rename Deck, Rename Routine, Delete Routine, Preferences).
  Catches: any new box or dialog through a JUCE API. Cannot catch: a hand-made overlay Component.
- NT-L2 the file label. `fileLabel_.setText` exactly 1 (the setter). For every `setFileLabel(` call (argument read
  to its closing parenthesis): its string literals are in the allowlist { "", "No file loaded", "Mic: ",
  "MilkDrop: ", "Camera: ", "Folder: ", " (", " images)", " frames)" } and its identifiers are in a pinned list
  (getFileName, getDeviceStatus, sourceType, name, fileLabelAudioSourceText, ... -- S0 writes the list from the
  surviving calls). Catches: a new literal, a new helper, a new variable fed to the label. Cannot catch: an event
  string smuggled through an allowlisted identifier.
- NT-L3 the notify sinks. The bodies of the two `dispatch.notify = [this]` lambdas hold `std::cerr` and no other
  statement; no line in src holds `setNotice`, `noticeLabel`, `warningLabel_`, `routineNotice`, `kNoticeSeconds`,
  `noticeKeyOf`, `warningText`, `lastMidiMessage_`, `LabelHold`, `loadingLabel`, `doneLabel`, `queueFullLabel`,
  `sourceGoneLabel`. The identifier half proves the removal complete; it is NOT a keep-out gate (a new name passes).
  The lambda half is: a notify sink that reaches a widget again fails.
- NT-L4 the device line: no line in src holds `audioDeviceNotice`, `audio_notice`, `onDebugAudioNotice`,
  "plug one in". (Completeness only.)
- NT-L5 the tripwire (fork F6, default IN): a pinned per-file count of text-widget members (`juce::Label`,
  `juce::TextEditor` declared as members under src/ui and src/MainComponent.h). A new text widget fails the lint
  until its file's row is raised in the same commit, with a one-word class in the row's comment (state / caption /
  help). Catches: "a new yellow label". Cannot catch: an event sentence written into an EXISTING label, a text
  painted in a `paint()` body, a button whose caption changes at an event. It is a tripwire for the reviewer's eye,
  not a classifier, and every UI lane pays one table edit for it. Runner-up: no NT-L5. Loses narrowly: without it
  the most likely new offender (one more Label fed at an event) passes every gate.
- What no lint covers is covered by rule + review: one line in CLAUDE.md's UI Patterns (section 6) and a standing
  question in every reviewer packet ("does this diff put a sentence on screen because something happened or failed?").

### Fork index (id | choice | runner-up)
- F1 notify seam | stays as the app-log channel, screen sink removed | delete the seam and every sentence
- F2 log twins | one logLine where a removed FAILURE text had none | remove bare
- F3 "Ready. Last take" | stays, re-based on a saved-take fact (T1) | remove the branch
- F4 file label after a composition load | the audio-source state text (T5) | leave the label untouched
- F5 the save box | one function `reportSaveFailed` + a witness; the real box by NR15 only if FN-1 allows | a production-mode row that always raises the box
- F6 keep-out tripwire NT-L5 | in | lints NT-L1..L4 only
- F7 order | after the sync dial, before the transport lane | after the transport lane (the handoff order)
- F8 the no-device yellow line (B14) | goes with A13 (Q2) | keep it for the two no-device states
- F9 outcome counts in the Record status line | go with the pad "!" (Q4b) | keep as live readouts
- F10 the cyan advance line (E07) | goes with `noticeLabel_`; the tooltip carries it | keep the label for that one sentence
- F11 the RED arm of live rows | P1 = pre-removal app + test routes | P0 with 404-as-RED

## 4 STAGES + ORDER (N9)

One builder context per stage. Every stage: its cases are written first and shown RED by name on the stage's base
(a case that never failed is struck and reported), then GREEN at its head. No stage adds an on-screen sentence.

- S0 RE-BASE NOTES (no code). On the lane's base (main after the lanes ahead of it have merged) write into the lane
  report: the file:line of every row of N1 and every site of N5; the counts of `setFileLabel(`, `fileLabel_.setText`,
  `setNotice(`, `showMessageBoxAsync`, `showOkCancelBox`, `dispatch.notify(` (pin: 46 / 3 / 6 / 13 / 3 / 28); any text
  sink the sync dial or the transport lane added (U2); `populateSlotMenu` (T10); the test binaries' paths; the list
  NT-L2 needs. FN-1 (by READING build/_deps JUCE source, nothing is run): does `showMessageBoxAsync` make a JUCE
  component that `ModalComponentManager` lists and can cancel, or a native alert? FN-2: is there already a route that
  selects a browser tab? Harmony constraint: freeze P0, a copy of the pre-lane main app bundle (never re-signed).
  Proves: the table, FN-1, FN-2.
- S1 WITNESSES + PROBE MOVES. Nothing on screen changes. (a) N6: the three probes moved; (b) test routes (compiled
  under AUDIODNA_TEST_SERVER only): GET /api/debug/ui_texts -- on the message thread, every Label / TextEditor /
  Button under every top-level window: component path, id, class, text, showing, bounds, text colour; `modals`
  (count and titles) when FN-1 allows; POST /api/debug/browser_tab {name} unless FN-2 found one; POST
  /api/debug/save_deck {deck, path} (calls `writeDeckFile`); POST /api/debug/dismiss_modals when FN-1 allows;
  (c) `.harmony/probe-notices.sh / .py / .json` with every row of 5.3. Files: the three probes, src/api/ApiServer.*,
  MainComponent.cpp (the route wiring block only), docs/claude/testing-eyes.md.
  Proves: the moved probes GREEN on P0 and their RED arms still RED; probe-notices RED on P1 in exactly the rows 5.3
  names. P1 = the app at S1's head: P0 plus test routes (the six counts of S0 unchanged -- shown in the report).
  Harmony freezes P1 as the RED arm of every live row.
- S2 TOP ROW AND BOXES. N5's [S2] list; N3 T4, T5, T6; `reportSaveFailed` with its witness (count, last title, last
  text, in ui_texts as `save_failed`). Files: MainComponent.*, core/StagedLoad.h, ui/PresetManager.*,
  audio/AudioEngine.*, ui/FileLabelText.h, ui/LookAndFeel.h (one comment), tests.
  Proves: NT-U9, NT-U10; lints NT-L1, NT-L2; the staged-load binary whole; rows NR1-NR8.
- S3 RECORD TAB, ROUTINE PADS, MIDI LEARN. N5's [S3] list; N3 T1, T2. Files: MainComponent.cpp (two lambdas and
  :1953-1956), ui/RecordPanel.*, ui/RecordPanelModel.h, recording/RecorderHost.*, ui/RoutinePad.*,
  ui/RoutineDeckView.h, ui/MidiLearnOverlay.*, tests.
  Proves: NT-U1 .. NT-U8; lint NT-L3; the record-panel, recorder-host, routine binaries whole; rows NR9-NR13.
  The stage ends at the VISUAL WORK GATE (5.5), not at a commit.
- S4 THE DEVICE LINE. N5's [S4] list; probe-btguard's two absent rows. Separate and last of the removals so that an
  answer to Q2 can still change its form (the Q2 delta: keep the widget for NoDevice / NoInput only, drop the
  MicReplaced branch; then NT-L4 is not built and the btguard clauses of N6 keep reading `audio_notice`).
  Proves: lint NT-L4; probe-btguard whole (production mode, `audio_deny`, no hardware); row NR14.
- S5 TRIPWIRE, DOCS, FULL RUN. NT-L5; section 6's docs; Boris's page (sections 7, 8); the lane report (the six counts
  before / after, the retired cases by name, the RED table).
  Proves: a full probe-notices run on the lane build and the RED table on P1.

ORDER. Recommended: this lane merges AFTER the sync dial (bf2) and BEFORE the transport lane's S1.
- Why after bf2: bf2 edits TopBar, ApiServer and MainComponent's provider block; S1 and S4 here edit the ApiServer
  route table and row 1. One re-base instead of two.
- Why before transport: this lane is removal-only and, with fork F1, touches NO line inside the trigger handlers,
  `pushCommands`, UndoService, ClipInspector or LayerStrip (the `dispatch.notify(...)` calls at :4808, :5048, :5676,
  :5722 stay as they are). The transport lane's probe (17 rows) and its visual gate are then written on an app that has
  no label to lean on, its G-N1 guard starts from the smaller counts, and probe-asan-live -- which the transport
  lane re-runs -- no longer waits on a text.
- What the transport lane's S0 re-reads if this lane merges first: nothing new in kind -- its S0 list already
  re-reads MainComponent's handlers, the route table and the lint counts; G-N1's base numbers are taken at its base.
- The strongest case for the other order (transport first, the handoff's order): Boris is waiting for the fire and
  Undo behaviour, not for the removal; the transport ruling is pinned to 34179a2 and every merge before it grows its
  S0; this lane carries a visual gate and four open questions, any of which can hold a merge for a day. If Harmony
  wants the features first, this lane loses nothing: S0 here re-reads the same table on the later base, and S1 (no
  on-screen change, fixes the asan-live wait) can still merge ahead of transport on its own.
- Either way the two lanes are never merged into each other mid-flight (G-N1 and the six counts are base-relative).

## 5 HARMONY'S GATE LIST (N8; pre-registered -- Harmony copies strings only from here)

Arms: P0 = frozen pre-lane main app. P1 = S1's head (P0 + test routes). L = the lane head.

### 5.1 Unit cases (each a TEST_CASE whose name begins with the id)
- NT-U1 RecordPanelModel: idle with `takeFolder` set and `lastSavedTakeFolder` empty reads "Ready. No take loaded."
- NT-U2 RecordPanelModel: idle with `lastSavedTakeFolder` = ".../jam.adna-take" reads "Ready. Last take: jam".
- NT-U3 RecordPanelModel: recording, frames written, `audioCapturing` false -> the line holds " without audio" and
  neither "from live input" nor "from audio file"; `audioCapturing` true -> "from live input".
- NT-U4 RecordPanelModel: playing with unresolved 2, moved 1, skipped 3 reads exactly "Playing 0:10 / 1:00".
- NT-U5 RecordPanelModel: recording with gaps 2 holds no "gap".
- NT-U6 RecorderHost: `lastSavedTakeFolder` stays empty after a refused arm, is set by a disarm that wrote
  take.json, and is unchanged by a disarm whose take folder cannot be written.
- NT-U7 RecorderHost: `audioCapturing` is true while the tap runs and false from the tick that sees it stopped.
- NT-U8 RoutineDeckView: a slot with unresolved 2, preambleUnresolved 1, skipped 3 -> the pad's tooltip equals the
  tooltip of the same slot with zeros.
- NT-U9 fileLabelAudioSourceText: mic -> "Mic: <status>"; file loaded -> its name; no file -> "No file loaded".
- NT-U10 the save-failed strings: titles "Save Composition", "Save Deck"; text "Save failed: " + path.
- Lints NT-L1 .. NT-L5 (N7), each named "NT-Ln ...", tags [lint][notices].
- Mutants (builder shows each RED once, by case name, then restores -- restore rebuild checked per RIG-RULES A):
  MU-N1 `setNotice` back in a notify lambda -> NT-L3. MU-N2 `setFileLabel("Saved: " + ...)` back -> NT-L2.
  MU-N3 the idle line reads `takeFolder` again -> NT-U1. MU-N4 `writeDeckFile` no longer calls `reportSaveFailed`
  -> NT-L1 and NR8. MU-N5 the clause ignores `audioCapturing` -> NT-U3. MU-N6 a second `showMessageBoxAsync`
  -> NT-L1. MU-N7 a new `juce::Label` member in RecordPanel.h -> NT-L5.

### 5.2 Unit gates (Harmony, on L)
- G-U1 build rc 0. G-U2 ctest serial: "100% tests passed, 0 tests failed out of N", N counted by Harmony on L and
  equal to (N on the lane base) - 8 + (the cases added: 10 + 5, plus any S0 adds); the 8 retired names of N5 are
  absent from `ctest -N`, and no other name of the base list is absent (diff of the two `ctest -N` lists).
- G-U3 each NT case RED by name on the stage base (builder's log), GREEN on L. G-U4 the seven mutants RED by name.
- G-U5 TSan unit and ASan unit gates as registered (5 / 5, 0 warnings; "PROBE-ASAN-UNIT GREEN").
- G-U6 the six counts on L: `setFileLabel(` < 46 (the number is written by S2 and pinned by NT-L2);
  `fileLabel_.setText` = 1; `setNotice(` = 0; `showMessageBoxAsync` = 1; `showOkCancelBox` = 3; `dispatch.notify(` = 28.

### 5.3 Live rows -- `.harmony/probe-notices.sh` (open -g, the live lock, quits only its own pid, no Output window,
no synthetic input, no screen capture). Final line: "PROBE-NOTICES GREEN (14 rows)" -- "(15 rows)" when NR15 runs -- or "PROBE-NOTICES RED (<row>)".
Takes and files the probe makes carry the prefix "probe-notices-" and are deleted by its trap (the takes root is Boris's).
Two deny lists, applied to every SHOWING text of GET /api/debug/ui_texts and to `file_label`:
- DENY-LABEL (file label only): begins with "Saved", "Loaded", "Loading ", "Duplicated deck", "Slot ", "Too many
  loads", "Duplicate skipped", "Show in Finder", "No images found", "Camera failed", "Audio device:", "Failed to load".
- DENY-ANY (every widget): "plug one in", "lost - now listening", "ould not", "already recording", "Nothing is
  recording", "No take is loaded", "deck unresolved", "refused", " started", "Removed the routine", "Saved routine",
  "Saved:", "holding the last look.", "restored first", "No audio is arriving", "Replaying with", "Record Over
  starts", "is empty.", "bank is full", "Last: ", "unresolved:", "skipped:", "audio gap".
Rows (test mode unless said; each: driver -> GREEN on L -> RED on P1):
- NR1 POST /api/debug/load_deck (a valid deck) -> numDecks + 1 after the settle of N6; `file_label` equals its value
  before; no DENY hit -> P1: `file_label` "Loaded deck: <name>".
- NR2 POST /api/debug/duplicate_deck -> same -> P1: "Duplicated deck: <name>".
- NR3 a composition load (probe-async-load's driver) -> the show's name in /api/composition; `file_label` is
  "No file loaded" or begins "Mic: " (the audio-source state, T5) -> P1: "Loaded: <name>".
- NR4 a load with one video clip (probe-async-load's fixture) -> at the barrier `load.staged == 1` and `file_label`
  not DENY -> P1: "Loading <name>...".
- NR5 POST /api/debug/save_composition into a scratch folder -> the file exists; `file_label` unchanged;
  `save_failed.count` 0 -> P1: "Saved: <file>".
- NR6 POST /api/debug/load_deck with a file that is not a deck -> numDecks unchanged; `file_label` unchanged; no
  DENY hit; `modals` 0 -> P1: passes too in test mode (the box is suppressed there). Its teeth are NT-L1 and NR15.
- NR7 failed show save: a scratch folder made read-only (chmod 555, restored and deleted by the probe's trap) ->
  POST /api/debug/save_composition into it -> `save_failed` = { count 1, title "Save Composition", text
  "Save failed: <path>" }; no file; /api/composition's name unchanged -> P1: `save_failed` absent.
- NR8 failed deck save: POST /api/debug/save_deck into the same folder -> count 2, title "Save Deck", text
  "Save failed: <path>" -> P1: `save_failed` absent.
- NR9 Record tab shown (browser_tab); POST /api/perf/record twice -> the second answer carries "A take is already
  recording." (REST keeps it); the dump's Record tab holds no DENY hit; then stop -> status label == "Ready. Last
  take: <name>", /api/perf/status `lastSavedTakeFolder` ends with <name> -> P1: a showing label reads "A take is
  already recording.", then "Saved: <name>".
- NR10 POST /api/perf/play with nothing loaded -> the REST answer carries the refusal; no DENY hit -> P1: "No take
  is loaded. Use Load Take... first." showing.
- NR11 the widget census of the Record tab: no component whose text colour is kMeterYellow; exactly the pinned list
  of labels (status, "Takes: ...", the captions); status-line bottom to the next row's top == the panel's row
  spacing -> P1: three more labels, two of them between the status line and the next row.
- NR12 POST /api/routine/fire on an empty pad -> /api/routine/status `lastError` "Routine pad <n> is empty."; no
  DENY hit -> P1: that sentence showing.
- NR13 POST /api/routine/save with no take loaded -> REST refusal; no DENY hit -> P1: the sentence showing.
- NR14 (production mode, with probe-btguard's `audio_deny` lever) input denied -> /api/debug/audio_devices `state`
  "no-input"; ui_text has no `audio_notice` key; no DENY hit; `file_label` == "Mic: no wired mic (Bluetooth is never
  used)" in Mic mode -> P1: `audio_notice` == the NoInput sentence.
- NR15 (production mode; RUN ONLY IF FN-1 says the box is a JUCE component) the real box: NR7's driver ->
  `modals` == ["Save Composition"] -> POST /api/debug/dismiss_modals -> `modals` 0; NR6's driver -> `modals` 0.
  Then the Quartz count of Audio-DNA windows equals the count before the row. P1: NR6's driver -> `modals` ==
  ["Load Deck"], dismissed by the same route. Screen safety: the app is launched with open -g; the box belongs to
  the probe's own pid and dies with it; the probe's trap posts dismiss_modals before it quits; no box of Boris's own
  app is ever touched (the probe REFUSES when an Audio-DNA it did not start is running).
  If FN-1 says native: NR15 is NOT RUN and is reported as such; the real box then rests on NT-L1 (one call, guarded
  only by `testMode_`), NR7 / NR8 (the call is reached) and Boris's check 1.
- Not drivable by REST, covered by lint only (reported as such): preset Save / Load / slots / FX Save (A01-A05,
  B03, B04), ISF import (A12, B13), image folder, camera, Show in Finder, the 9th queued load, the Save Routine
  button's own line, MIDI Learn's "Last:".

### 5.4 Re-runs (Harmony): probe-asan-live "PROBE-ASAN-LIVE GREEN (7 steps, ...)" on L's ASan build and "RED (step L1)"
on the wiring mutant; probe-async-load and probe-btguard whole on L, with their RED tables unchanged in the rows N6
did not touch; probe-boxes (63 / 0 / 1 BLOCKED, rc 3) and probe-deck-tabs unchanged; idle-paint: the registered bars
(the lane removes one 4 Hz label write and adds none).

### 5.5 VISUAL WORK GATE (stage S3 for the Record tab; S4 for row 1)
States (set by REST / test routes; captured by Quartz window id only; a manifest carries the model facts and the
ui_texts dump of each):
- V1 Record tab, idle, nothing recorded. V2 idle after a take ("Ready. Last take: <name>"). V3 recording with audio.
  V4 recording, capture stopped (the test seam of NT-U7's fact through a TEST-ONLY route, or omitted and said so).
  V5 a take loaded. V6 playing with audio (the state that used to show the yellow and the cyan line).
  V7 after a refused Record (same picture as V1 -- that is the point). V8 row 1 in the Ok state. V9 row 1 with the
  input denied, Mic mode.
Bars from the dump (pass / fail, no judgement):
- B1 in V1-V7 the Record tab has no showing text widget beyond the pinned list; none is yellow or cyan.
- B2 status-line bottom to the format row's top == the row spacing in every state (no two-line hole);
  the Save Routine row's bottom is the panel's last occupied y (no empty line under it).
- B3 the status line's text in V1 "Ready. No take loaded.", V2 "Ready. Last take: <name>", V7 == V1's.
- B4 V8, V9: `fileLabel_`'s right edge == row 1's right edge; no yellow label in row 1.
- B5 every text of every dump passes DENY-ANY; `file_label` passes DENY-LABEL.
- B6 the pads row: no pad paints "!" (pixel census of the kWarning colour inside every pad rect == 0) in a show
  whose routine has unresolved timelines.
Questions for the five critic seats (each gets Boris's four sentences verbatim, and the list of texts that are
outside the lane: tooltips, captions, the status line's state forms, "Takes: <path>"):
- visual-design: with the two lines gone, does the Record tab read as one block, or does the status line now float?
- UX: in V7 (a refused Record) and after a failed Stop, can the user tell from what IS on screen that no take is
  being / was recorded, without any sentence telling him?
- graphic-design: is any gap, misalignment or orphaned caption left where a line was removed (Record tab, row 1)?
- logic: name any text in any dump that is on screen because something happened or failed. Name any state text
  that is false for the model facts in its manifest row.
- interaction-logic: for each button of the Record tab, is its result visible as a state change (button caption,
  status line, pad row) in the same refresh, now that no sentence confirms it?

## 6 DOCS (stage S5 unless said)
- docs/claude/recording.md: :34 ("said in the notice") and :95 (the red "!") re-worded; "Surfaces": the Record tab
  has one status line, no warning / notice lines; a pad shows no last-run mark; the notify seam is the app log.
- docs/claude/rendering.md:101 (Pitfall 58): the label hold and "Loading <name>..." removed from the text.
- docs/claude/pitfalls.md:133 (Pitfall 61): the yellow notice removed from the entry (S4).
- docs/claude/testing-eyes.md:19 and docs/claude/integration.md: `audio_notice` out; `ui_texts`, `browser_tab`,
  `save_deck`, `dismiss_modals`, `save_failed`, `lastSavedTakeFolder`, `audioCapturing` in (S1 / S3 / S4).
- .harmony/APP-INVENTORY.md:85 and :232. BORIS_DECISIONS.md: one "Built (notices)" sentence with his four sentences.
- CLAUDE.md, UI Patterns, one line (the file is near its byte cap; Harmony assigns the Pitfall number, the lane
  writes "NN"): "No event or failure text: nothing on screen says what just happened or what failed; the one
  exception is the Save failed box (`reportSaveFailed`); a state line reads the model (Pitfall NN, lints NT-L1..L5)."
- .harmony/HANDOFF.md ledger: the asan-live line and the routineLoadNote line closed.

## 7 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)
1. Save a show into a folder you cannot write to (a locked folder, or a full / read-only disk) -> ONE box: "Save
   failed: <path>" -> wrong: nothing happens, or any other box. Same for Save Deck.
2. Open a show, save it, load a deck, duplicate a deck, recall a preset slot, press FX Save -> the top text line
   never says Saved / Loaded / Loading -> wrong: any such word.
3. Open a file that is not a show -> nothing changes, no box -> wrong: a box.
4. Record tab: press Record, press Stop -> the line reads "Ready. Last take: <the name>"; no yellow or blue
   sentence at any moment -> wrong: a sentence appears, or a gap where the lines were.
5. Unplug the wired mic while listening to it -> no yellow sentence; the top line names the mic now in use (or says
   there is no wired mic) -> wrong: a yellow line. (This is Q2: say if a dead input with no words is not acceptable.)
6. Fire a routine whose layer you deleted -> the pad plays what it can; no red "!" -> wrong: a red mark.
7. MIDI Learn: move a knob -> only "Press Escape to exit" under the title -> wrong: "Last: ...". (Q3.)
8. Open a big show with videos -> the old show keeps playing until the new one appears; no "Loading..." text.
9. By feel, on stage: is anything you relied on to know "it worked" now missing?

## 8 QUESTIONS FOR BORIS (plain words; nothing waits -- the default is built)
- Q1 (his page's question 12). A recording that could not be saved, or a routine that could not be made: a box like
  "Save failed", or nothing? Default: nothing (your words named a failed save of a show or a deck). You would see it
  only because the Record line does not name the take. Changing it later is a three-line change (N4).
- Q2. When no microphone or sound device is found, the yellow line "No wired mic found - plug one in" goes with the
  rest. Default: it goes; while the app listens to the mic the top line still says "Mic: no wired mic". If you play
  from a file there are then no words at all for a missing sound device. Keep that one line, or not?
- Q3. In MIDI Learn the small text "Last: Note 60 ..." shows the last message your controller sent. Default: it goes.
  Keep it as a "the controller is alive" monitor, or not?
- Q4. Record tab, the one line that stays: (a) when idle it reads "Ready. Last take: <name>" -- default: it stays,
  and only for a take that was really saved; (b) while a take plays it also counted "unresolved / moved / skipped"
  and "audio gaps" -- default: those counts go, with the red "!" on a routine pad.

## 9 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
- R1 THE STRONGEST: "truthful state" is where new text comes back in. T1 keeps "Last take: <name>", T2 changes a
  clause at a failure, T4 / T5 write the label at an event (a failed load, a finished load). A council can fairly
  say each is an announcement wearing a state's clothes, and that the clean reading of his words is to remove and
  add nothing. Why the plan stands: each of the four is the smallest write that stops a line from saying something
  FALSE (F8, F10, F24), each reads a model fact every refresh or names what is loaded now, none adds a widget, a
  colour or a sentence that did not exist. Cheapest refuting test: for each of T1-T5, ask "is the text still true
  one minute later with nothing else happening?" -- an announcement fails that, these do not. If the ruling
  disagrees, T1 falls back to "Ready. No take loaded." always (Q4a's other answer), T5 to "no write"; T2 and T4
  cannot be dropped without leaving a false line.
- R2 Over-removal by the "in the list" reading (A06, A23, B14, B19 line 3, B21): his words settle them only if "the
  list" is tables A + B. Refuting test: Q2, Q3 and checks 5-8; each is one separable hunk (S4 whole; one paint
  line; one model line).
- R3 A failure with no trace on screen (inventory R2-R6): a take that does not start (disk full), a deck file that is
  not a deck, a camera that does not open. That is the ruling; the log twins (F2) and REST `lastError` are the only
  trace. Refuting test: check 9.
- R4 The real save box is proven end to end only if FN-1 lets NR15 run. Otherwise one line (`if (!testMode_)` + the
  call) rests on a lint and on Boris's check 1. Cheapest test: FN-1 is a read of two JUCE functions in S0.
- R5 NT-L2's identifier allowlist can be widened by a builder to make a new write pass. It is a diff a reviewer
  sees; it is not proof. NT-L5 is a tripwire, not a classifier (N7 says what each cannot catch).
- R6 Probe moves weaken a probe: a wait on `load.staged == 0` returns for a load that FAILED as well as one that
  landed. The VALID clauses (numDecks, layers, names) are the teeth and are unchanged; N6's RED / GREEN on P0 is
  the test. /api/state must be the one polled (atomics), never /api/composition during a load.
- R7 `reportSaveFailed` inside `saveCompositionTo` changes who can raise the box: the test route now does, in
  production mode. Wanted (NR15), but a probe that saves into a bad path in production mode would put a box on
  screen: probe-boxes' K7 save runs in test mode (INFERRED from its header: "/api/debug/* ... test-mode"); S0
  confirms no production-mode probe posts save_composition.
- R8 The Record tab's rows move up: a VISUAL change with Boris's eye as the last judge (check 4); the pads row
  loses the 16-px shift of the number when "!" was drawn (paint key changes: the pad-paint tests are re-run whole).
- R9 Order: if Harmony runs transport first, S0 here must re-derive every line; the plan's line numbers are the
  pin's and are not to be trusted on a later base.
- R10 Not verified by this plan (read in S0, labelled in section 2): U1 (native box), U2 (bf2 / transport trees),
  U3 (settings write), `populateSlotMenu`, the driver probe-async-load uses for a composition load, whether the
  Record panel applies its view while its tab is hidden.

STATUS: DONE


## HARMONY ADOPTION (s-rta-1003b, 2026-10-03 23:41:08) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1003b/ruling-notices.md IN FULL (STATUS DONE: 29 attacks ruled, 24 amendments). Stages
   S0 (re-base notes; freeze P0), S1 (witness routes + probe moves; nothing on screen changes), S2 (top row and boxes;
   reportSaveFailed), S3 (Record tab, routine pads, MIDI Learn; VISUAL WORK GATE), S4 (the device line), S5 (tripwire lint,
   docs, Boris page, lane report). Its section 5 is the ONLY source of gate strings (19 live rows); section 6 is the final
   GOES / STAYS / BECOMES A STATE table.
2. ORDER: after the sync dial (bf2) AND the transport lane have merged (the ruling; H-3 default: S1 is not built early).
3. HARMONY'S DECISIONS (the ruling's section 9), defaults unless said:
   H-1 Until Boris answers question 12: the architect's recommended default -- EVERY save he presses shows the box when
       it fails (seven callers: show x2, deck, preset, FX Save, the take at Stop, the overdub at Stop Playback). This
       REPLACES Harmony's first reading of BF34 (only the show and deck alerts) as the default: his words are "the only
       fail message will be a failed save", which does not narrow "save", and the broad form loses no work silently. The
       narrow three-caller form is specified in the ruling and is a small switch if he answers B.
   H-2 the device line goes (S4) until he answers question 16. H-3 no early S1. H-4 the label write only when the label
       names the file the engine dropped. H-5 the camera label left as today. H-6 by fact FM-5; a real box is never put in
       front of Boris's work: if FM-5 shows it comes forward, the two rows are NOT RUN and reported as such, resting on
       their lint arms. H-7 untouched. H-8 both tripwires in. H-9 rename. H-10 accepted; S0 re-checks.
4. BORIS QUESTIONS: on his page as 12 (which saves), 15 (refused Save Routine), 16 (the no-mic line), 17 (MIDI Learn's
   "Last:" text), 18 (the Record tab's remaining line), 19 (Record cannot start). Each keeps its default until he answers.
5. FACTS FM-1..FM-6 are Harmony's to measure on the running app at the stage the ruling names.
6. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP -- the lock helper, own-pid quits, no Output window, no
   synthetic input; S3 is a visual deliverable: five critic seats on decoded captures before he sees it. MERGE by Harmony.
