# ATTACK PAPERS -- plan-notices (s-rta-1003b)

Seat papers as the workflow handed them to the architect: VERBATIM content, pretty-printed, one section per seat.
Raw JSON length as transcribed: 34970 characters (the dispatch said 34970). Seats returned: 4 of 4. The array is closed.

## Seat: stage-operator (7 attacks)

```json
{
  "seat": "stage-operator",
  "attacks": [
    {
      "id": "ST-1",
      "target": "N4 / Q1: a failed TAKE save defaults to 'nothing'",
      "claim": "The plain reading of his last ruling covers a take. A take that did not write take.json is the most expensive failed save he can have on stage, and under the default he sees nothing, only a status line that names an older take (or 'No take loaded') that he must notice.",
      "evidence": "Boris verbatim (binding-decisions.md:718-721): \"ok. the only fail message will be a failed save. remove all others\". The 'two boxes only' reading is Harmony's (backlog BF34: 'Harmony's reading, not his words'; 'edge, flagged not decided'). RecorderHost.cpp:383-406: disarm sets lastError_, returns ok=saved and clears recording_ either way. Plan N4 lines 253-255: after a failed Stop the line 'does NOT read Ready. Last take: <name>'. Plan T3 says the same.",
      "severity": "MUST",
      "proposed_change": "Flip the default. Wire the already-planned reportSaveFailed into perfStop and perfStopPlay when disarm returned not-ok for a recording take (title 'Save Take', the take folder path). The plan itself prices this at three lines, and a wrong 'nothing' is silent loss while a wrong 'box' is deletable. Keep routine refusals out: that is an in-memory refusal, not a disk write. Ask Q1 with this as the default."
    },
    {
      "id": "ST-2",
      "target": "N2 row B14, F8, T7, Q2: the no-device / no-input yellow line",
      "claim": "The plan removes the only persistent dead-input display against its own rule, and Q2 misstates what remains. His words name only the mic-FALLBACK note, not the no-input state.",
      "evidence": "N2's rule (plan 187-189): a text STAYS when it names the present condition and is recomputed from the model. refreshAudioDeviceNotice (MainComponent.cpp:3213-3231) is exactly that: state derived from getDeviceState(), shown until the device returns. The code comment :476-477 says it is a persistent notice 'not a file-label write (every other label write would hide it)'. Clip fires overwrite the label (:4926, :4932), so 'Mic: no wired mic' is gone at the next trigger. Q2 (plan 619-621) says 'while the app listens to the mic the top line still says Mic: no wired mic'. That is false after the first clip fire (T7 admits it). Verbatim record names only 'the yellow note on a microphone fallback' (binding-decisions.md:711), i.e. A13 MicReplaced.",
      "severity": "MUST",
      "proposed_change": "Make the default 'keep': leave the line for NoDevice/NoInput only (state, recomputed, no event wording), and drop only the MicReplaced branch (A13, his named example). Rewrite Q2 to say plainly that a dead input then shows nowhere after the first clip fire. S4 already isolates this hunk."
    },
    {
      "id": "ST-3",
      "target": "N4 / section 7 check 1 / F16: when the surviving Save-failed box can actually appear",
      "claim": "Harmony told Boris the box covers an unplugged drive or a moved folder. At the pin those cases raise NO box, and check 1 gives him no performable way to see one.",
      "evidence": "backlog:282 (what Boris was told): 'a save fails when the drive is unplugged / the disk is full / the folder moved / the file is locked ... the only sign'. MainComponent.cpp:3546-3568: if the show's parent folder is not a directory, saveComposition() calls saveCompositionAs() (a chooser), never the box. Same for saveDeck (:3759-3764). Plan F16 notes this, but section 7 check 1 says only 'a locked folder, or a full / read-only disk' and 'wrong: nothing happens'. Save As through a macOS chooser into a non-writable folder likely never reaches the box (INFERRED, not read).",
      "severity": "SHOULD",
      "proposed_change": "Rewrite check 1 as concrete steps: load a show from a folder, make that folder read-only in Finder (Get Info), press Save. Say in the plan and on his page that an unplugged drive or moved folder opens Save As instead, which is what his ruling now rests on. Do not claim the box covers it."
    },
    {
      "id": "ST-4",
      "target": "5.3 NR6-NR8 / NR15 / MU-N4: the gate on the one surviving alert",
      "claim": "The surviving box has a gate that cannot fail for the failure mode that matters, a guard that stops it ever showing in production.",
      "evidence": "Plan NR6 (520-521): 'P1: passes too in test mode (the box is suppressed there)'. NR7/NR8 assert the `save_failed` WITNESS, which is recorded where testMode_ suppresses the box. NT-L1 counts lines (exactly 1 box call), so an inverted or misplaced `!testMode_` guard inside reportSaveFailed passes it. MU-N4 mutates a caller, MU-N6 adds a second call; no mutant touches the guard. NR15 runs only if FN-1 (U1, ASSUMED, plan 130-131) says the box is a JUCE component; otherwise 'the real box rests on NT-L1 ... and Boris's check 1' (plan 548-549).",
      "severity": "MUST",
      "proposed_change": "Add MU-N8: invert or drop the testMode_ guard in reportSaveFailed; it must go RED by name. Make the witness record whether the box call was made, independent of testMode_, via an injectable `showBox` seam whose default wiring is the real call. Then NR7/NR8 fail when production would show nothing, even if FN-1 says native."
    },
    {
      "id": "ST-5",
      "target": "N4 'Saves other than a show or a deck' (inventory of saves)",
      "claim": "The save inventory is incomplete. Several silent file writes that Boris could call a failed save are not listed and have no default or delta.",
      "evidence": "MainComponent.cpp:6694 `composition_.saveToFile(compFile);` (Collect Files, result ignored). :2979-2990 Save Preset and :4399-4415 FX Save: savePreset false gives no label, no box (only success is handled). :7291 `bindingManager_.saveToFile(...)` ignored. N4 names only take, routine and settings (plan 253-260), and U3 (settings write) is still unread.",
      "severity": "SHOULD",
      "proposed_change": "Add these four to N4 with an explicit default and the one-line delta (reportSaveFailed caller). Because Boris's words are 'a failed save', the Save Preset, FX Save and Collect show-file writes are the closest siblings to B06/B08. Read AppSettings in S0 so U3 is not an assumption."
    },
    {
      "id": "ST-6",
      "target": "Section 7 checks 4 and 9 / V7: a Record that is refused",
      "claim": "The check list tests only the success path of Record, but the gate pins that a refused Record is invisible. Boris never sees that outcome before accepting the lane.",
      "evidence": "Plan 5.5 B3: 'V7 == V1's', and V7 'same picture as V1 -- that is the point'. Check 4 (plan 606-607) is press Record, press Stop only. Check 9 ('by feel, on stage') is not a performable check. RecorderHost.cpp:215/:229/:235 are the refusals; recording_ stays false, so after the press the panel is unchanged. T1 fixes the idle line to 'Ready. No take loaded.' but nothing shows the press was refused.",
      "severity": "SHOULD",
      "proposed_change": "Add a performable check: make a refusal happen (e.g. press Record with the takes folder made read-only), press Record, and say what he sees (nothing). Ask whether that is acceptable. Replace check 9 with that. This is the 'take not recording' case that costs the show, so he should rule on it explicitly instead of discovering it live."
    },
    {
      "id": "ST-7",
      "target": "5.3 DENY-ANY list",
      "claim": "DENY-ANY substring-matches every showing text in the app, including user content, so the gate can turn RED on a name rather than a notice.",
      "evidence": "Plan 506-509: DENY-ANY includes ' started', 'refused', 'is empty.', 'Last: ', 'unresolved:', 'skipped:' and is applied to 'every SHOWING text of GET /api/debug/ui_texts', which includes Buttons, deck and clip names, and preset names.",
      "severity": "NIT",
      "proposed_change": "Apply DENY-ANY only to the pinned notice widgets and Record/Pad/row-1 labels, or anchor the phrases ('^A take is already') so a fixture or user name cannot trip it."
    }
  ],
  "strongest_point": "The plan removes the dead-input display (ST-2) against its own rule, and Q2 tells Boris the top line 'still says Mic: no wired mic' when the next clip fire overwrites it. His verbatim words name only the mic-fallback note, so a persistent, model-derived NoInput/NoDevice state line is not ruled away by his words. Together with ST-1, the failed take save, the plan leaves him blind exactly where he named a moment that costs the show.",
  "citations_rechecked": true
}
```

## Seat: state-truth (8 attacks)

```json
{
  "seat": "state-truth",
  "attacks": [
    {
      "id": "ST-1",
      "target": "N3 T1 (Record idle state after a refused Record)",
      "claim": "T1 fixes only the status line. The Show in Finder button still derives from takeFolder, which arm() overwrites before its refusals, so after a refused Record it stays enabled with 'Shows the take folder in the Finder.' and opens a folder that was never created. NT-U6, NR9 and V7/B3 check only the status-line text, so this lie passes every gate.",
      "evidence": "src/ui/RecordPanelModel.h:205-209 (revealFolder = s.takeFolder when idle and nothing loaded; button enabled if non-empty); src/ui/RecordPanel.cpp:66-67 (revealToUser on it); src/recording/RecorderHost.cpp:185 (takeFolder_ written) before refusals at :215, :229, :235. Plan lines 211-216 touch only RecordPanelModel.h:274-275.",
      "severity": "MUST",
      "proposed_change": "Make T1 the rule for every consumer of takeFolder: revealFolder in the idle branch also reads lastSavedTakeFolder (disabled with 'No take yet.' when empty). Add an NT-U case (refused arm gives reveal disabled) and a B-bar in V7: the reveal button's enabled state equals V1's."
    },
    {
      "id": "ST-2",
      "target": "N3 T5 / T6 (file label after a composition load); gates NT-U9, NR3",
      "claim": "T5's premise is false. The label does not keep naming a clip of the replaced show: the swap already rewrites it from the NEW show. T5 then adds an event-time label write (the thing Boris banned) that overwrites that truthful clip name with 'No file loaded' or 'Mic: ...' while the preview shows the clip. The label has two owners (audio source and preview clip), and T6 collapses them.",
      "evidence": "MainComponent.cpp:3353 swapCompositionModel -> refreshUiAfterModelSwap (:3051, calls refreshPreviewFromShow at :3116) -> label set from the first playing image or source clip (:5165, :5173) or '' (:5184); only afterwards :3360 setFileLabel(done), which the plan rewrites into T6's text (plan lines 234-236). NR3 (plan 5.3) expects 'No file loaded' or 'Mic: ', so it passes only because T5 clobbers the preview name.",
      "severity": "MUST",
      "proposed_change": "Drop T5: delete :3360 and write nothing, so refreshPreviewFromShow's write stands. Re-derive NR3 as 'file_label equals the name of the first playing clip, or empty'. Re-check whether T4/T6 should also write over a preview-owned label."
    },
    {
      "id": "ST-3",
      "target": "N3 (missing rows): camera and image-folder failures; N2 row C03 'Camera: / Folder:' kept as state",
      "claim": "Two failure paths leave a state display showing the old state once their failure text goes, and N3 does not list them. Camera: the dropdown already shows the chosen camera and nothing reverts it, so after a failed open the selector says camera X while no camera is active. Folder: slideshowImages_ is cleared before the empty check, so the label keeps 'Folder: <old> (n images)' for a slideshow that no longer exists.",
      "evidence": "MainComponent.cpp:364-370 (onChange does not revert) and :4572-4576 (the only sign is 'Camera failed to open', B12); :4489 slideshowImages_.clear(), :4497-4501 'No images found' and return, label last set :4513-4514 for the old folder; advanceSlideshow returns at :4520.",
      "severity": "MUST",
      "proposed_change": "Add rows to N3: failed openCamera calls selector.setSelectedId(1, dontSendNotification) (state follows the model); the empty-folder branch must not leave the old 'Folder:' label, so write the T6 state text or restore the previous slideshow list. Add NR rows or lint-only coverage plus a Boris check."
    },
    {
      "id": "ST-4",
      "target": "N3 T4/T7, N2 row C02 ('Mic: ...' STAYS as state); B01 device-reapply failure",
      "claim": "C02 is kept under the rule 'recomputed from the model', but it is an event write: set at :300, :486, :538 and :5842 only. Worse, a failed reapply is skipped at :485 (error non-empty) and reached the label only through onError('Audio device: ...') (B01), which the plan removes. T4 covers only 'Failed to load'. After the lane, the label keeps 'Mic: <old input> @ rate' after the device failed to reapply, and the plan has no row for it.",
      "evidence": "AudioEngine.cpp:15-21 (onError only on error); MainComponent.cpp:470-474, :484-487 (write only if error.isEmpty()); audio/DeviceGuard.cpp:202-207 (reapply returns silently, no callback, on Reapply::None or a repeat scan seq); plan N2 rule lines 187-190 vs plan line 200.",
      "severity": "SHOULD",
      "proposed_change": "Say in N3 what the onError lambda writes for an 'Audio device:' error (the T6 state text, recomputed from getDeviceStatus). Reclassify C02 honestly as an event-written state text and add the B01 path to a probe row (probe-btguard has the lever) instead of calling it recomputed."
    },
    {
      "id": "ST-5",
      "target": "N3 T2 (status clause after the audio tap stopped)",
      "claim": "T2 swaps the false 'from live input' for ' without audio', but that wording means no audio was requested, and the take still holds the audio captured before the stop. The line trades one lie for another. NT-U3 pins the wording, so the gate would lock it in.",
      "evidence": "RecordPanelModel.h:157, :256-258 (' without audio' is the audioRequested == false branch; BG7 sets audio=false when there is no device, MainComponent.cpp:5879-5880); RecorderHost.cpp:108 (finalRef keeps the written audio segment); the tick edge at :202-208 only sets lastError_.",
      "severity": "SHOULD",
      "proposed_change": "After capture stops, drop the audio clause entirely ('Recording m:ss · n lanes · n moves'), which is true and adds no sentence; or ask Boris. Rewrite NT-U3 to pin 'no from-clause' instead of ' without audio'."
    },
    {
      "id": "ST-6",
      "target": "N4 saves other than a show or a deck; Boris: 'the only fail message will be a failed save'",
      "claim": "N4 lists takes, routines and settings but not four other saves that already fail silently: Save Preset, FX Save, Export Bindings, Collect Files' composition JSON. Save Preset's 'Saved: <file>' (A01) was the only difference between success and failure (no else branch). After removal a failed preset save and a good one look identical. Silent now, but the plan neither classifies nor asks.",
      "evidence": "MainComponent.cpp:2983-2989 (A01, success-only branch), :4397-4403 (FX Save, flash only on success), :6694 (saveToFile return ignored), :7291 (bindingManager_.saveToFile ignored). Plan N4 lines 251-267 and NT-L1's three-caller table name none of them.",
      "severity": "SHOULD",
      "proposed_change": "Add the four saves to N4 with an explicit default and extend Q1 to 'any save that fails'. If they get the box, they become callers of reportSaveFailed and NT-L1's pinned table must list them; otherwise say plainly that a failed preset save is now silent."
    },
    {
      "id": "ST-7",
      "target": "N4 take-save claim and N3 T3 ('Load Take... does not list it')",
      "claim": "The plan's account of what Boris sees after a failed Stop is wrong. A provisional take.json is written at arm and refreshed periodically, so a take whose final save failed still exists on disk as a loadable folder. Load Take is a folder chooser, not a list. Only the status line stops naming it, so Boris cannot tell it exists. The page's Q1 default is argued on a false description.",
      "evidence": "RecorderHost.cpp:278, :293 (provisional.save at arm), :540 (periodic snapshot.save), :383 (final save). RecordPanelModel.h:201 and RecordPanel.cpp:44-60 (Load Take = FileChooser over the takes root). Plan lines 253-255, 618.",
      "severity": "SHOULD",
      "proposed_change": "Correct N4 and Q1: 'a take that failed its final save may still be on disk from the periodic saves; the Record line will not name it.' Re-ask Q1 with the true description, and make NT-U6's third clause assert only lastSavedTakeFolder."
    },
    {
      "id": "ST-8",
      "target": "N2 row E07 (cyan 'Record Over starts a new take on top of this audio; the loaded take is kept.') and plan F12",
      "claim": "The plan says the Record button tooltip 'already says what Record Over does', so deleting the cyan line loses nothing. It does lose the only on-screen statement that the loaded take is kept, which is the reassurance a performer needs before pressing a button captioned 'Record Over'.",
      "evidence": "RecordPanelModel.h:175-176 (tooltip: 'Records a new take over this take's audio, from the current playback position.' no 'kept') vs :300-301 (the cyan text).",
      "severity": "NIT",
      "proposed_change": "Fold 'The loaded take is kept.' into the tooltip at :176 (no new widget, no event text), and correct F12."
    }
  ],
  "strongest_point": "ST-2 and ST-1 together. T5 rests on a false premise (refreshUiAfterModelSwap -> refreshPreviewFromShow already rewrites the label from the new show) and adds the event-time label write Boris banned, with NR3 passing only because of that clobber. T1 fixes the status line but leaves Show in Finder reading the same takeFolder that arm() overwrites before it refuses, so the 'truthful state' lane would ship a button that opens a folder that was never made.",
  "citations_rechecked": true
}
```

## Seat: gates (8 attacks)

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "N6 moved waits (probe-asan-live L1/L4/L6 and probe-async-load completion clauses) -- `load.staged == 0` as the completion test",
      "claim": "`staged == 0` goes true at the START of the landing, not the end, so the moved wait is already true before the thing it waits for. The old wait (label holds 'Loaded...') was step 7, the LAST landing step. The probe's reason to exist is to never read the model while a landing mutates it.",
      "evidence": "34179a2:src/MainComponent.cpp:3318-3326 `finishStagedLoad`: `auto s = std::move(staged_); ... s->label.clear(); publishLoadWitness();` (stagedNow_ = 0, :3233-3238) runs BEFORE the swap (step 6, ~:3350) and the label (step 7, :3360). Probe-asan-live.sh:175-178 records an ASan overflow from reading /api/composition while a Duplicate Deck push_back ran. Plan N6 line 'poll /api/state load.staged == 0 and load.queued == 0' has no second barrier.",
      "severity": "MUST",
      "proposed_change": "Wait = poll `staged==0 and queued==0`, THEN a second GET /api/debug/ui_text (message-thread barrier: the landing is one synchronous message-thread run, so the barrier returns only after step 7), THEN read /api/composition. Add a RED arm: on P0, poll only staged==0 with a heavy swap and show the facts read mid-landing."
    },
    {
      "id": "GA-2",
      "target": "5.3 rows NR9-NR13 (and NR10/NR12/NR13 specifically) -- the 'no DENY hit' clauses",
      "claim": "These absence checks can pass on L before the situation could ever have drawn text, and on P1 may fail to go RED. The notice is only applied to the label at the 4 Hz panel refresh, and the dump counts only SHOWING widgets. NR10, NR12 and NR13 never put the Record tab on screen and never wait a refresh.",
      "evidence": "34179a2:src/MainComponent.cpp:4316-4336 (refresh inside `++uiUpdateCounter_ >= 8`, ~267 ms); src/ui/RecordPanel.cpp:297-298 (setText at refresh only). Plan 5.3 lines 503-504 ('every SHOWING text'); NR9 alone says 'Record tab shown (browser_tab)'; NR10/NR12/NR13 name no browser_tab. Plan R10 admits 'whether the Record panel applies its view while its tab is hidden' is unread. kNoticeSeconds = 10 (RecordPanelModel.h:72), so a late dump also lets the P1 text expire.",
      "severity": "MUST",
      "proposed_change": "Every NR9-NR13 row: POST browser_tab Record, then wait >= 2 refresh periods (>= 0.6 s, but < 10 s) AFTER the driver POST, then dump. State the shared precondition once and add a precondition clause 'Record tab showing==true' that fails the row if false. On P1 require the sentence to appear within that same window."
    },
    {
      "id": "GA-3",
      "target": "N3 T1 / NR9 / V7 / bar B3 -- the 'refused Record leaves the idle line truthful' proof",
      "claim": "No live row or visual state reaches the lie T1 fixes. NR9's refusal is 'already recording', which returns BEFORE `takeFolder_` is written. So P1 reads the same as L and the row has no RED arm for T1. V7 ('after a refused Record', B3: V7 == V1) names no refusal; with 'already recording' the recorder is still Recording, so 'same picture as V1' is false or vacuous.",
      "evidence": "34179a2:src/recording/RecorderHost.cpp:169-172 (`if (recording_) { res.error = \"already recording\"; return res; }`) precedes :185 `takeFolder_ = opts.takeFolder;`. Only the later refusals (:200 stored audio not found, :215, :229, :235, per plan F8) follow it. Plan 5.3 NR9 line 527-530, 5.5 V7 line 565 and B3 line 571.",
      "severity": "MUST",
      "proposed_change": "Name the refusal V7 and a live row NR9b use: a refusal after :185 that REST can drive, e.g. Record Over on a loaded take whose audio .wav the probe deleted (:200 'stored audio not found'). Assert the idle line 'Ready. No take loaded.' and a `lastSavedTakeFolder` that is absent or unchanged. Show it RED on P1 (reads 'Ready. Last take: <new folder>'). Otherwise say plainly that T1 rests on NT-U1/U6 only."
    },
    {
      "id": "GA-4",
      "target": "5.3 NR7/NR8, mutant MU-N4, lint NT-L1 -- proof of the ONE message Boris kept",
      "claim": "Two of the three `reportSaveFailed` callers have no live row and no mutant: plain Save (existing path) and the Save As chooser callback. NR7 reaches only `saveCompositionTo` through the test route; NR8 reaches `writeDeckFile`. MU-N4 mutates only `writeDeckFile`. A Save branch that calls the box on the success branch, drops the call, or double-reports (the callback today still tests `!saveCompositionTo && !testMode_`) passes every gate except a line-pinned NT-L1 table. If FN-1 says 'native', `!testMode_` inside the function is also unproven.",
      "evidence": "34179a2:src/MainComponent.cpp:3548-3563 (Save: `composition_.saveToFile(filePath)` then the box), :3609-3615 (chooser callback calls the box itself), :3822 (writeDeckFile). The test route is `[this](juce::File f){ saveCompositionTo(f); }` (:2151), so it never executes :3548-3563. Plan N4 lines 261-267 and 5.1 MU-N4.",
      "severity": "MUST",
      "proposed_change": "Add a test route POST /api/debug/save (calls `saveComposition()`) and row NR7b: save_composition to a scratch path, chmod 555 the folder, POST /api/debug/save -> `save_failed` count +1, title 'Save Composition'. Add mutants MU-N8 (Save branch drops the call) and MU-N9 (chooser callback reports twice -> count 2). Also assert in NR7 that count rises by exactly 1 (no double report)."
    },
    {
      "id": "GA-5",
      "target": "N7 lint NT-L1 -- 'deny-by-default', 'any new box or dialog through a JUCE API'",
      "claim": "NT-L1 is an allowlist of API NAMES, so a new name walks past it. It does not catch `juce::AlertWindow::showAsync`, `showYesNoCancelBox`, `showNativeDialogBox`, `AlertWindow::LaunchOptions`, `std::make_unique<juce::AlertWindow>` or a stack `juce::AlertWindow w(`. The plan counts `showMessageBoxAsync`, `showMessageBox(`, `NativeMessageBox`, `showOkCancelBox` and `new juce::AlertWindow` only. 'Deny by default' is false for the box route, and that is the route a new failure text would use.",
      "evidence": "34179a2 src AlertWindow sites: showMessageBoxAsync x12, `showOkCancelBox` (:3172, CompDecksBrowser.cpp:263), `new juce::AlertWindow` (:3838, :6337, :6364). Plan N7 lines 363-372 pins those tokens; no line mentions the bare token `AlertWindow`.",
      "severity": "SHOULD",
      "proposed_change": "Pin by the bare tokens `AlertWindow`, `MessageBox`, `NativeMessageBox`, `showYesNo`, `showNativeDialog`, `LaunchOptions`, `DialogWindow`, `CallOutBox`, `BubbleMessage`: each line must be on a pinned (file, count) table, with LookAndFeel.h entries named. Reword the plan's claim to 'deny-by-token'."
    },
    {
      "id": "GA-6",
      "target": "5.3 DENY-ANY / S1 `ui_texts` dump / 5.5 bar B6",
      "claim": "The dump reads only Label / TextEditor / Button widgets, but DENY-ANY lists texts that are painted (MIDI Learn 'Last: ', the pad '!'). Those can never match, so those entries are decorative. B6 (pad '!' pixel census == 0) is also vacuous: the '!' comes from the slot's LAST-RUN record, not from the show, and the bar says only 'a show whose routine has unresolved timelines' without a fire.",
      "evidence": "Plan N5 S1(b) line 424-425 (dump = Label/TextEditor/Button) vs DENY-ANY 'Last: ' (line 508-509); 34179a2:src/ui/MidiLearnOverlay.cpp:102-103 paints it; src/ui/RoutineDeckView.h:126-136, :168, :206 and plan F13 (warning from `RoutineEngine::Status::Slot` last run, RoutineEngine.cpp:826-828). The plan itself lists MIDI Learn 'Last:' as 'covered by lint only' (line 550-552).",
      "severity": "SHOULD",
      "proposed_change": "Strike painted-only strings from DENY-ANY or mark them 'lint only'. Rewrite B6: fire the routine (fixture with an unresolved timeline) via /api/routine/fire, wait for the run, THEN count; require the same census to be > 0 on P1 or the bar is struck."
    },
    {
      "id": "GA-7",
      "target": "N5 'Tests retired BY NAME' and G-U2 count arithmetic",
      "claim": "The retire list is accurate on names (all 8 verified). 'Rows lose only their warningText/noticeText CHECKs' is wrong for row 9 and the notice-key scenarios. Row 9 holds a whole scenario that uses `in.notice`, `noticeKeyOf` and `noticeLive` and cannot compile once those fields go. Catch2 SECTIONs are not ctest units, so NT-U4 'becomes' from a SECTION cannot be shown by `ctest -N`.",
      "evidence": "34179a2:tests/test_record_panel_model.cpp:316-327 (inside TEST_CASE 'row 9 -- playing with audio', :292) uses `in.noticeKey = noticeKeyOf(s)` and `.noticeLive`. Plan N5 lines 327-329 and G-U2 line 492-495.",
      "severity": "NIT",
      "proposed_change": "List the :316-327 block as removed from row 9 by name. State that NT-U4/U8 conversions are not visible to the `ctest -N` diff; the diff rule applies to TEST_CASE names only."
    },
    {
      "id": "GA-8",
      "target": "5.2 G-U6 first count line",
      "claim": "`setFileLabel(` < 46 passes for any decrease, including a lane that removes one write and keeps an event text. The exact pin is deferred to 'the number is written by S2'.",
      "evidence": "Plan 5.2 line 497; N7 NT-L2 line 373-378 (the real pin, with an allowlist).",
      "severity": "NIT",
      "proposed_change": "Pre-register the exact post-lane count now (the builder's S0 list), or drop the inequality and cite NT-L2 as the gate."
    }
  ],
  "strongest_point": "The moved asan-live and async-load waits on `load.staged == 0` are already true at the start of the landing. finishStagedLoad (34179a2:src/MainComponent.cpp:3318-3326) publishes staged=0 before the swap and before step 7. The old label wait could only be satisfied after the landing finished. So the probe whose whole purpose is to avoid reading the model mid-landing (the ASan overflow in probe-asan-live.sh:175-178) loses its tooth in the wait the plan moves FIRST. A second message-thread barrier after staged==0 fixes it.",
  "citations_rechecked": true
}
```

## Seat: completeness (6 attacks)

```json
{
  "seat": "completeness",
  "attacks": [
    {
      "id": "CO-1",
      "target": "N6 probe moves (probe-asan-live L1/L4/L6, probe-async-load a2/a5/a5b/a6/a9): the wait becomes 'barrier GET, then poll /api/state load.staged==0 and load.queued==0'",
      "claim": "The replacement completion signal fires BEFORE the load lands, so the moved rows can read /api/composition while the swap runs. The label they replace was written AFTER the swap. This is the SF-12 hazard that RIG-RULES A forbids.",
      "evidence": "At 34179a2 MainComponent.cpp finishStagedLoad(): :3318 moves staged_ out, :3324 publishLoadWitness() stores staged=0 (stagedNow_, :3233-3238). The model swap is :3353 (composition_ = std::move). The deck push is ~:3392. setFileLabel(done) comes last, at :3360 and :3398. The plan's barrier (ui_text) runs BEFORE the poll, not after. The plan's R6 only worries that a failed load also returns. probe-asan-live.sh's own settle() docstring records the container-overflow incident.",
      "severity": "MUST",
      "proposed_change": "After the staged==0 poll, issue ONE MORE GET /api/debug/ui_text. It runs on the message thread, so it returns only after finishStagedLoad has finished the swap. Or publish a post-swap witness (a 'load.finished' counter written at the end of finishStagedLoad) and wait on that. Add a RED arm: the poll without the second barrier must be shown to read mid-swap, or the row is marked unproven."
    },
    {
      "id": "CO-2",
      "target": "N4 'Saves other than a show or a deck' + Q1 + NT-L1 (reportSaveFailed has 3 callers)",
      "claim": "The plan's list of other saves omits the preset Save and FX Save. They are the only other saves whose success AND failure are visible today, and the plan removes the success text while keeping no failure signal. Afterwards a failed preset save looks exactly like a successful one. Boris's rule is 'the only fail message will be a failed save', unqualified.",
      "evidence": "MainComponent.cpp:2983-2989 (savePreset): PresetManager::savePreset returns bool (PresetManager.h:53); the success branch writes 'Saved: ...' (A01); failure does nothing. :4397-4403 (fastSave): same, and the failure also skips the flash. N4 names only takes, routines, and settings. Q1 asks only about takes and routines. Boris, verbatim: 'ok. the only fail message will be a failed save. remove all others'.",
      "severity": "MUST",
      "proposed_change": "Add Save Preset and FX Save to N4 and Q1 explicitly. Default to the literal reading: both call reportSaveFailed (titles 'Save Preset' / 'Save FX', text 'Save failed: <path>'), raising NT-L1's pinned caller table from 3 to 5. Or have Harmony say plainly that the narrow reading is hers and put these two on Boris's page. Do not leave them unmentioned."
    },
    {
      "id": "CO-3",
      "target": "N3 'No state display lies afterwards' / R1's test; N1 rows B11 and B12",
      "claim": "Two failure branches clear or replace the real state before failing. Removing their text leaves the file label naming something that is gone, the same lie class the plan fixes for the audio file (T4). The plan fixes T4 only and misses these two. The inventory's claim that on 'No images' 'the slideshow state is unchanged' is false.",
      "evidence": "MainComponent.cpp:4489 slideshowImages_.clear() runs BEFORE the isEmpty check; :4499 is the removed 'No images found in folder', then return. advanceSlideshow returns on an empty list, so the slideshow stops while the label still reads 'Folder: <old> (n images)' (:4513). openCamera(): :4559 closeCamera() runs first; :4574 'Camera failed to open' is the removed text. The camera is now off while the label still names the previous source.",
      "severity": "MUST",
      "proposed_change": "Add T-rows. Folder: build the list locally and replace slideshowImages_ only when non-empty. Then nothing was lost and no label write is needed. Camera: on failure write the audio-source state text from T6 (no new sentence). Extend R1's 'still true a minute later' test to every surviving label write after a failure branch, and add an NR row for the folder case."
    },
    {
      "id": "CO-4",
      "target": "N2 rows E07 and B19 line 3 (removed; neither is on Boris's question list Q1-Q4)",
      "claim": "Two sentences that describe a present mode or an advance warning, not an event or failure, are removed on an 'it is in the list' reading. One carries information no surviving display carries. E07 is class E (kept) in the inventory and is not within his words.",
      "evidence": "RecordPanelModel.h:301 'Record Over starts a new take on top of this audio; the loaded take is kept.' vs the button tooltip :176 'Records a new take over this take's audio, from the current playback position.' The tooltip lacks 'the loaded take is kept', so F12's 'already says it' is only half true. :285 'Live input is paused until Stop Playback.' appears nowhere else; :228 reads only 'Locked while the take replays.' R2 names Q2/Q3 and checks 5-8 as the refuting tests; none covers these two.",
      "severity": "SHOULD",
      "proposed_change": "Keep the facts as tooltips (class E, no widget, no layout slot). Append 'The loaded take is kept.' to the Record Over tooltip (:176). Change the play-with-audio tooltip while playing (:228) to 'Locked while the take replays. Live input is paused until Stop Playback.' Add both to Boris's page as a single yes/no, or reword NT-U cases to pin the tooltips."
    },
    {
      "id": "CO-5",
      "target": "N5 'By file' removal lists (S3, S4) and S2 PresetManager: dead members, timers and stale text left behind",
      "claim": "'Cleanly and completely' is not met: several members become reader-less and are not on any removal list or NT-L3/NT-L4 token list.",
      "evidence": "(a) RecordPanelInputs::nowSeconds (RecordPanelModel.h:47), RecordPanel::refresh/applyView(nowSeconds) (RecordPanel.cpp:203-207, :256-259) and the tests' in.nowSeconds (test_record_panel_model.cpp:21) are read only by the notice expiry (:296) and applyRoutineNotice (RecordPanel.cpp:304), both removed. (b) AudioEngine::onDeviceStateChanged (AudioEngine.h:52, called AudioEngine.cpp:184-185): the plan removes its only assignee (MainComponent.cpp:481) but not the hook. (c) PresetManager LoadStats legacyFile / dropped / mappingsTotal are kept only for tests once A05/B03/B04 go (PresetManager.cpp:193, :294). (d) Stale comments: RecorderHost.cpp:285-288, RecorderHost.h:72-77 (cite RecordPanelNoticeKey and the 'notice INVARIANT'), RoutineEngine.h:48, tests/test_lookandfeel_square.cpp:97 ('the 12 showMessageBoxAsync calls'), APP-INVENTORY.md:109 and :357 (the ISF alert rows; the plan edits only :85 and :232).",
      "severity": "SHOULD",
      "proposed_change": "Add (a)-(d) to the S3/S4/S2 lists. Add 'nowSeconds', 'onDeviceStateChanged', 'RecordPanelNoticeKey' and 'ISF Import Successful' to the NT-L3/L4 token lists. Either say why the LoadStats fields stay (a stated reason, not 'tests read them'), or drop them."
    },
    {
      "id": "CO-6",
      "target": "5.3 rows NR6 / NR15 and the R4 / U1 / FN-1 'not established' item",
      "claim": "The one box that survives is proven in production only conditionally, and NR6 is a registered row that cannot fail. FN-1 is already answerable from the repo, so NR15 should not be conditional.",
      "evidence": "Plan NR6: 'P1: passes too in test mode' and 'Its teeth are NT-L1 and NR15'; NR15 is 'RUN ONLY IF FN-1 says the box is a JUCE component'. NR7/NR8 run in test mode, where the box is suppressed (F16/F17), so a mutant that never raises the real box passes them. NT-L1 is a count lint only. tests/test_lookandfeel_square.cpp:119 builds the owner-less alert through the default LookAndFeel's createAlertWindow and snapshots it; LookAndFeel.h:14-15 says showMessageBoxAsync alerts take the app LookAndFeel. That points to a JUCE component, not a native alert (INFERRED, not run). R4 and R10 leave this as 'ASSUMED'.",
      "severity": "SHOULD",
      "proposed_change": "Make NR15 unconditional once S0 reads the JUCE helper (a single read, no run). Add a named mutant, 'reportSaveFailed's testMode_ guard inverted or the box call removed', that must turn NR15 RED. Drop NR6 as a counted gate row, or mark it 'informational' and keep the 14-row count honest."
    }
  ],
  "strongest_point": "CO-1. The plan replaces the probes' 'Loaded' label wait with load.staged==0. finishStagedLoad publishes staged=0 at MainComponent.cpp:3324, before the swap at :3353 and the deck push, while the label was written after them (:3360, :3398). The moved rows would read /api/composition mid-swap, the SF-12 hazard RIG-RULES A forbids. The probe's own docstring records the container-overflow it caused. The fix is cheap: a second message-thread barrier after the poll.",
  "citations_rechecked": true
}
```

STATUS: DONE (4 seats, verbatim)
