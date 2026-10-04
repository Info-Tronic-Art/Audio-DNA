# Attack papers -- lane one-save (blind council, 4 of 4 seats returned)

Written by the Architect BEFORE ruling, verbatim from the dispatch (the seats JSON, 40377 characters, a closed JSON array).
Pretty-printed only (indentation and line breaks added; no word, id or number changed). One section per seat.
The plan attacked: .harmony/.reports/s-rta-1004/plan-one-save.md. The ruling: .harmony/.reports/s-rta-1004/ruling-one-save.md.

Seats: data-safety (7 attacks), gates (8 attacks), stage-hands (8 attacks), scope (8 attacks)

## Seat: data-safety

Attack ids: DA-1 [MUST], DA-2 [SHOULD], DA-3 [SHOULD], DA-4 [SHOULD], DA-5 [SHOULD], DA-6 [SHOULD], DA-7 [NIT]

```json
{
  "seat": "data-safety",
  "attacks": [
    {
      "id": "DA-1",
      "target": "E1 + OS1-c writeShow + OS5 Save & Quit + section 5 (no full-disk row)",
      "claim": "The plan's safety premise is false: it says a failed write leaves the old file in place, but JUCE replaceWithText ignores the temp-file write result. On a full disk it swaps a truncated file over the show and returns true. Nothing in the plan catches this.",
      "evidence": "Plan lines 35-37 (E1). juce_File.cpp:798-803: replaceWithText calls appendText, ignores the result, and returns overwriteTargetFileWithTemporary(). facts-saves.md line 12 (item 4) says the same: 'a full disk can report success', UNKNOWN-NEEDS-A-RUN. loadFromFile on a truncated file returns false (Composition.h:1159-1168), so the show is unopenable. Plan lines 433-436: Saving + SaveDone(Saved) goes straight to DoQuit, and 'Saved' is that bool. The backup copy, the settings.json copy and AppSettings.update use the same primitives. No SF, SB or OS-L row covers a short write; OS-L14 only covers a backups-is-a-file failure.",
      "severity": "MUST",
      "proposed_change": "writeShow stops using replaceWithText. It writes the temp file through a FileOutputStream, checks every write and flush, and reads the temp file back to confirm its size equals the JSON length and that it parses. Only then does it swap. Do the same for the backup copy: compare size and hash before the show write starts. Add unit row SB-7 (a failing stream returns false and the target bytes are unchanged). Add a live row on a small disk-image volume, with Harmony running it. Correct E1."
    },
    {
      "id": "DA-2",
      "target": "OS1-e backup rule (+ SB-6, line 621)",
      "claim": "The only protection is a once-ever copy of an old-shape or later-version target. From then on every Save of a version-2 show overwrites the only good copy with no previous generation. A target that is damaged, or does not parse as a show, is overwritten with no backup at all. The plan lists a rolling backup as out of the lane (line 824), so this lane's first new write path is also its first chance to lose a whole show.",
      "evidence": "Plan lines 146-153: backup only when the target parses to an object with a decks array and version != 2. SB-6 (line 621): a target that is missing or is not a show needs no backup. Line 824: rolling backup out of scope. Combined with DA-1, a damaged v2 show saved over via Save As Replace, or by plain Save after a short write, leaves nothing to go back to.",
      "severity": "SHOULD",
      "proposed_change": "Keep ONE previous generation: before the swap, rename the existing target to backups/<name>.prev.json if it is readable bytes, even when unparseable (a byte copy needs no parse). Overwrite only that one file. If a ruling keeps the lane minimal, at least drop the 'must parse' condition for the byte copy so a damaged target is still copied."
    },
    {
      "id": "DA-3",
      "target": "OS4 'Taking a deck' + OS6/S4b removal of ShowMigration::legacyRowSettings + OS-L12 / TD-4",
      "claim": "Taking a deck from an old-shape donor, which is Boris's own only show, silently gives it the first deck's layer settings, not its own. The one reader of per-deck row settings is deleted, and the gate checks only the clip count. The first Save of his show also drops Deck 2's differing layer settings for good. Only the byte backup keeps them, and he is never told.",
      "evidence": "ShowMigration.h:106-121: layer i takes the settings of the FIRST deck that has the row, and later decks' differing settings are 'dropped' with one log line. Plan lines 381-383 delete legacyRowSettings, the only code reading per-deck row settings. Plan lines 351-353 convert the donor in memory and read 'the donor's layers'. Plan line 703 (OS-L12): the pass condition is 'the deck's clip count' only, so a deck with the wrong opacity, blend or layer FX passes. Plan line 645 (TD-4) states the expectation, but no live row checks Deck 2's own opacity.",
      "severity": "SHOULD",
      "proposed_change": "For an old-shape donor, read rowSettings from the raw decks[i] rows (keep legacyRowSettings, or move its logic into deckFromShow). Add to OS-L12 and TD-4 a distinguishing fixture: Deck 2's row opacity or blend differs from Deck 1's, and the wide-deck case asserts Deck 2's values. Tell Boris once, in B-1, that the first Save keeps the old file as backups/*.v0.json because Deck 2's layer settings are merged."
    },
    {
      "id": "DA-4",
      "target": "OS4 list rule 'a file that is not a show is not listed' (lines 337-339) + R6",
      "claim": "A show file that cannot be summarised is hidden from the list entirely. A truncated or damaged show, or a later-version file, is a performer's only copy, and it vanishes from the one screen where he looks for his shows. He cannot see, Show in Finder, or delete it.",
      "evidence": "Plan lines 337-339: a file is shown only once its summary says isShow (an object with a decks array of at least one object). R6 accepts that a file 'could vanish'. Today every *.json is a row (facts-one-save Q1: 'EVERY *.json in the folder is a row'). Combined with DA-1, the truncated result of a failed Save disappears from view.",
      "severity": "SHOULD",
      "proposed_change": "List every *.json. A file whose summary is not a show gets a plain one-line row (name and date, no triangle, no deck rows, greyed) with the same right-click menu (Show in Finder, Delete...). Only a bare deck file or preset may be hidden, and only after its content is positively recognised as such. Cost: one more state in SL-2."
    },
    {
      "id": "DA-5",
      "target": "OS5 + G-OS5 / OS-L17..L20 / LINT-5",
      "claim": "No automated gate can fail if the production quit path is wired wrong. Every live row feeds quit_flow choices through a route. Test mode quits at once by design. The window itself is only reached by an inert route. So a systemRequestedQuit that calls quit() directly in a non-test run passes every unit and live row, and only Boris's B-6 by hand would catch it. That is the path where unsaved work is lost.",
      "evidence": "Plan lines 425-426: in test mode requestQuit() quits at once and shows no window. Lines 715-720: L17-L20 post to /api/debug/quit_flow, which hands the flow the choice. Line 730: 'a test-mode quit by pid shows no window' is the only check of the real route, and it is the bypass. LINT-5 (line 660) is a textual grep. QF-7 pins that test mode never asks.",
      "severity": "SHOULD",
      "proposed_change": "Add a test-server route POST /api/debug/system_quit that calls AudioDNAApplication::systemRequestedQuit() with a test flag 'ask as in production' and only records the resulting QuitFlow state ('asking') and the ShowWindow output, with no window and no quit. The row asserts state == asking and the pid alive. The RED arm is a mutant where systemRequestedQuit calls quit(). This runs inside the app's own process, with no synthetic input and no Output window."
    },
    {
      "id": "DA-6",
      "target": "OS2 'Open ... the computer's copy becomes that set too' (lines 187-188) + R3 + OS2 Import",
      "claim": "Opening a show written elsewhere replaces the live set, and the same moment overwrites the computer's remembered set. Bindings carry only channel, note and CC numbers and no device identity, so on a machine without that controller the pads go dead and the machine's own set is gone. The way back is a manual Import from the previous show, and only if that file still exists. No Save was made, so it contradicts Boris's rule that nothing is saved without a Save.",
      "evidence": "Binding.h has no device or port field (grep: only the 'Input Source' comment). Plan lines 187-188: the computer's copy becomes the opened set at Open, not only at Save. R3 (lines 787-790) concedes 'the one place this lane can lose something without a file on disk'. binding-decisions.md:772 and boris-feedback-backlog.md:389 (his 11:59 rule): 'if the user is changing things around settings, etc., and they do not save the composition, nothing is saved'.",
      "severity": "SHOULD",
      "proposed_change": "Take the plan's own fallback now. Write the computer's copy only at Save and Save As (delete the call in finishStagedLoad). The live set still switches at Open, and Import brings in a set. This is a one-line change and also removes OS-L1's side effect on the computer's copy. If Boris answers Q50 'remember always', the hook is the two overlay exits (MainComponent.cpp:1818, :1822), as the plan says."
    },
    {
      "id": "DA-7",
      "target": "OS1-a versionOf rule: 'key present -> trusted as is'",
      "claim": "The version reader trusts any present value, so a version of 0, a negative number, a string or a float on a file that has top-level layers sends a bf9b-shaped show through the legacy conversion. The plan reads shape from the version where the version cannot be right, and tests only the v2-without-layers case.",
      "evidence": "Plan lines 121-125: 'key present -> trusted as is'; from 2 on no presence test. SF-5 (line 608) covers only v2 without layers. Today's conversion (Composition.h:993-997, ShowMigration.h:18-22) merges all decks' layer settings into one stack and drops differing ones (ShowMigration.h:106-121).",
      "severity": "NIT",
      "proposed_change": "versionOf: trust the key only when it is an integer >= 2. Anything else (absent, 0, 1, negative, non-integer) falls back to the existing layers-presence test. Add SF-10 for a file with version 0, 'x' or 2.5 together with layers."
    }
  ],
  "strongest_point": "DA-1. The plan's whole data-safety story rests on E1: 'a failed write leaves the old file in place.' JUCE's replaceWithText (juce_File.cpp:798-803) ignores the temp-file write result and swaps in whatever it wrote. A Save on a nearly full disk can therefore truncate the show, report success, and unlock Save & Quit's DoQuit. The facts sheet the plan cites (facts-saves.md:12) already says so. Yet the plan adds more bytes to every Save (keys, layout, the settings.json copy, the backup copy) and has no unit row or live row for a short write.",
  "citations_rechecked": true
}
```

## Seat: gates

Attack ids: GA-1 [MUST], GA-2 [MUST], GA-3 [SHOULD], GA-4 [MUST], GA-5 [SHOULD], GA-6 [SHOULD], GA-7 [SHOULD], GA-8 [SHOULD]

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "OS-L4 (New keeps live keys) and its place in G-OS2",
      "claim": "OS-L4 cannot fail against the rule the plan rejected. Its sequence (L1-L3 then new_composition) leaves live keys equal to the computer's copy, so 'New re-reads the computer's copy' and 'New keeps live keys' both give count 3. OS-L4 also names no RED arm and no mutant. Every other 'RED: the tree before S2' arm fails because the new debug route is absent (404), not because the logic is wrong.",
      "evidence": "Plan :194-197 ('where there are no unsaved key changes both rules give the same keys'); :682 (OS-L4 has no RED text); :723 (G-OS2 lists only MU-OS-7, 8, 9, 10, 17). By :677-680 the apply at Open also writes the computer's copy, so after L1 live == computer's copy.",
      "severity": "MUST",
      "proposed_change": "Make OS-L4 do set_keys (a 4th, unsaved binding) BEFORE new_composition, then assert count 4 with the tuples kept. Name a mutant 'New applies the computer's copy' that must fail it. State that a RED arm on a new route counts only when the route exists on the RED tree (use mutants, not 'tree before')."
    },
    {
      "id": "GA-2",
      "target": "VG states V8-V15 and Q1/Q2 vs the screen-safety law",
      "claim": "Most visual states cannot be reached with window-id captures and no synthetic input. V8-V10 are popup menus, V11 is a confirm box, V13-V14 are the native menu bar (Deck, Shortcuts, View) and the tab/'+' menus, and V15 is a hover tooltip. The plan names a route only for the quit window. No route opens a header menu, a Delete confirm, a menu-bar menu or a tooltip. Opening them takes a click, an AppleScript System Events click, or a hover, which is synthetic input. A native mac menu bar is also not a capturable window.",
      "evidence": "Plan :394-400 (V1-V15); :451-452 (capture 'by window id', route only for quit_window); :668-676 (the route list has no menu opener); :567 ('window-id captures only'). The law (HANDOFF.md:105-184, 'no synthetic input') forbids the alternative.",
      "severity": "MUST",
      "proposed_change": "Either add test-only routes that open each popup or confirm in the app's own window (the pattern of quit_window), or drop V8-V15 from the visual gate and prove them by a menu-model dump route (item names, enabled flags) plus tests/test_deck_tab_row.cpp-style cases. Say which in the plan."
    },
    {
      "id": "GA-3",
      "target": "OS-L9 / OS-L10 and the layout claim (OS3, B-5)",
      "claim": "OS-L9 passes when the layout code never ran in the part that matters. Its 'preview keeps running' check is /api/render_frame returning a non-uniform picture. The canvas renders offscreen at the composition's size, never from a Component, so it is independent of panel widths. It cannot see a zero-width preview detaching the GL context, the very unknown OS3 claims to close. The fraction bar has an escape ('or the drag clamp's value ... computed by the probe from previewW's formula'), so the expected value comes from the thing measured. deckDividerY is never asserted. No live row saves a layout and reloads it, so writeShow filling 'layout' from stale or default values survives (OS-L6 checks only that the object exists).",
      "evidence": "Plan :693-695, :696, :685-686; CLAUDE.md Pitfall 37 ('never from a Component'); plan :275-276 ('closes ... by construction').",
      "severity": "SHOULD",
      "proposed_change": "State a fixed scratch window size in the probe and a fixed expected fraction list, computed in the probe from the plan's own formula BEFORE the run. Assert deckDividerY. Add a live save-then-load round trip that sets the dividers, saves, loads another show, loads the first, and compares all four values. Treat render_frame as an irrelevant check and move 'preview not black' to B-5 or a preview-panel readback route."
    },
    {
      "id": "GA-4",
      "target": "G-OS5 / OS-L17-L20 (quit window)",
      "claim": "The rows prove the flow object, not that any real quit path asks. (a) Quit is reached through quit_flow, which feeds choices and skips Main.cpp's systemRequestedQuit -> requestQuit. QF-7 says test mode quits at once with no Asking state, so the plan does not say how the route gets the flow into Asking. (b) L17 (cancel, pid alive 3 s) and L18 (quit exits) have no mutant. (c) L20 ('q.json exists BEFORE the pid exits') cannot observe order: the file exists either way after exit, and a save-after-quit mutant still writes it. (d) 'Every quit path shows it' (close button, Cmd+Q) rests on LINT-5, a grep, plus B-6.",
      "evidence": "Plan :423-426, :653-654 (QF-7), :715-718, :730; Main.cpp:34-36 and :87-90 are the only quit sites. OS-L19 is the only row with a RED mutant (MU-OS-15).",
      "severity": "MUST",
      "proposed_change": "Name mutants for L17 (Cancel calls quit) and L20 (DoQuit issued before StartSave completes). Have the route record a monotonic sequence number for 'write finished' and 'quit issued' and assert the order. Add a test-build-only switch that lets requestQuit() run the real path with window creation allowed. State in the plan how quit_flow enters Asking in test mode."
    },
    {
      "id": "GA-5",
      "target": "OS-L15 / OS-L16 (list reading off the message thread)",
      "claim": "The gate polls a field the route does not return. OS-L15 says 'polled until pending == 0', but the GET /api/debug/show_list shape is {shows[...], messageThreadParses}, with no 'pending'. OS-L16's bar, messageThreadParses == 0, is a counter incremented by the code under test, so any parse on the message thread that bypasses it (a summarize call elsewhere, isV2DeckFile-style) goes uncounted. Nobody measures the claim itself, a message-thread stall with 40 MB of shows. The 'refresh moments' include visibilityChanged, which no row exercises.",
      "evidence": "Plan :672-673 (route shape), :710-714 (OS-L15, OS-L16), :340-341 (visibilityChanged refresh).",
      "severity": "SHOULD",
      "proposed_change": "Add `pending` to the route. Add a message-thread heartbeat check, with its bar stated before the run: max gap between ui_text round trips during the refresh stays below a fixed ms figure. Keep the counter as a second witness only. Add a mutant that parses inside visibilityChanged."
    },
    {
      "id": "GA-6",
      "target": "OS6.2 Table 2 (stale pins) and OS1 test list",
      "claim": "Table 2 misses tests that break when Deck::sourceFile goes in S4b, and it mislabels one it lists. tests/test_composition.cpp sets and checks `src.sourceFile` / `copy.sourceFile == juce::File()` in the duplicateDeck case. tests/test_undo_commands.cpp's hand-written Deck operator== compares `a.sourceFile == b.sourceFile` and its comment lists sourceFile as a covered field. Neither appears in Table 2. The plan also says test_composition.cpp:386 'touches outputDisplay', but that case touches crossfader*, genrePresetNames and smartAutopilotEnabled, and the file has no outputDisplay at all. Table 2 was not built from a grep of tests/.",
      "evidence": "tests/test_composition.cpp:1203,1220; tests/test_undo_commands.cpp:234-245; tests/test_composition.cpp:386-458 (grep outputDisplay -> 0 hits); plan :174-175, :498-511.",
      "severity": "SHOULD",
      "proposed_change": "Add both sourceFile sites to Table 2 and to S4b's file list, and fix the :386 description. Add a gate step 'grep tests/ for every removed member name (sourceFile, legacyRowSettings, isV2DeckFile, saveToFile with one argument) is empty', so the table is checked rather than trusted."
    },
    {
      "id": "GA-7",
      "target": "KS-4 / OS-L2 (F-EMPTY), mutant numbering, G-OS-HIS",
      "claim": "(a) 'An empty set is no set' (R2) is the choice that guards his controller, yet no mutant targets it. MU-OS-6 is 'a missing array parses as an empty set', and F-EMPTY has the array present, so a mutant 'empty set replaces' survives OS-L2 and KS-4. (b) The plan promises 'MU-OS-1..MU-OS-16', but rows cite MU-OS-17 and 18, and no list defines them in one place; stage rows say 'MU-OS-6, 9 red in unit' while live rows rely on them as build mutants. (c) G-OS-HIS compares the saved backup's checksum with the older boris-show-backup copy, not with the copy Harmony takes at gate time. It then checks only deck names and clip counts, which cannot show 'opens exactly as at 185147b' (layers, effects, routines).",
      "evidence": "Plan :545, :624-625, :679, :717, :732-734.",
      "severity": "SHOULD",
      "proposed_change": "Add MU-OS-19 'empty bindings array replaces' with KS-4 and OS-L2 as its RED rows. Give one table that defines every mutant (id, edit, row that must fail). In G-OS-HIS, hash the scratch copy at copy time. Also diff GET /api/composition from the 185147b app and the new app on that copy, ignoring 'version' and the seven dropped keys."
    },
    {
      "id": "GA-8",
      "target": "Section 6 'what only Boris can check' (B-2, B-3, B-7)",
      "claim": "Several entries hide claims a machine can test with routes the plan already has. B-7 ('no Load Deck / Save Deck, '+' adds a deck at once, a menu item that does nothing') is a menu-model assertion plus the deck_tabs route. B-2 ('what was playing stops when a deck is added') is readable from the keys route's playing[], but OS-L11 never asserts that layers keep playing. B-3 'pads dead after launch' is a dispatch claim that OS-L5 reads only as a binding count, never as a note that fires after launch.",
      "evidence": "Plan :739-760 (B-2, B-3, B-7); :697-704 (OS-L11 fields: no playing check); :683-684 (OS-L5 reads count and note only); :668 (playing[] exists).",
      "severity": "SHOULD",
      "proposed_change": "Add playing[] before/after to OS-L11 and OS-L12. In OS-L5, send a midi note after launch and assert the action fired. Add a menu-model dump row for B-7. Leave in section 6 only what needs eyes or ears on his real screens and controller."
    }
  ],
  "strongest_point": "OS-L4 is a gate that cannot fail. Its sequence leaves live keys equal to the computer's copy, so it cannot tell the chosen rule 'New keeps the live keys' from the rejected rule 'New re-reads the computer's copy'. The plan's own text admits the two rules agree there. The RED arms of the new-route rows mostly fail because the route does not exist yet, which says nothing about the logic. The visual gate (V8-V15) and the quit rows have the same weakness. V8-V15 need menus opened with no route to open them. The quit rows never reach the real quit entry.",
  "citations_rechecked": true
}
```

## Seat: stage-hands

Attack ids: ST-1 [MUST], ST-2 [MUST], ST-3 [SHOULD], ST-4 [SHOULD], ST-5 [SHOULD], ST-6 [SHOULD], ST-7 [SHOULD], ST-8 [SHOULD]

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "OS6.4 / OS8 stage S7 (the old Save, Load, FX Save and ten slots)",
      "claim": "The plan treats question 86 as unanswered and strikes stage S7, but Boris answered it. The plan would ship the old look buttons he told Harmony to remove now. This contradicts his words.",
      "evidence": "plan-one-save.md:522 reads 'boris-clarify-86.md: \"ANSWERS (none yet)\"'; :524-525 'No answer: ... Stage S7 does not start'; :568 'S7 ... (only if 86 = A)'; :478 row-1 buttons 'per question 86'. Boris's record: binding-decisions.md:898-899 '(86; recorded 2026-10-04 13:22:56) \"86 default yes\" -> the small Save and Load, FX Save and the ten slots are removed now'. boris-clarify-86.md:20-21 'ANSWERS 86 = default (A)'; boris-feedback-backlog.md:612-615.",
      "severity": "MUST",
      "proposed_change": "Rewrite OS6.4 and the OS8 table: S7 is mandatory and sits before the visual gate (LINT-6, state V16, row-1 capture, the tests/probes it turns red listed in OS6.2). Delete the 'No answer / B' branches. Add the S7 deletions to the gate rows, since G-OS4-2 and G-OS5 check only 'Save Deck' and 'Load Deck...'. Boris loses the look save buttons until the per-effect looks exist, so that gap should be stated to him."
    },
    {
      "id": "ST-2",
      "target": "OS5 quit window: hand-built AlertWindow, QuitFlow, G-OS5 rows L17-L20, QF-8",
      "claim": "The window the plan builds will not do what the plan says. Return does nothing, and Esc maps to an unspecified return value. No gate row goes through the window, so a wrong button wiring (Esc or a button quitting, Save & Quit not saving) can only be caught by Boris. Save & Quit that quits without saving is a failure he named.",
      "evidence": "plan:446-447 'juce::AlertWindow built by hand ... addButton three times' with no shortcuts and no return values; :432 'Esc = Cancel; Return = the default button'; :441-442 claims it works 'as confirmReplaceShow', yet confirmReplaceShow uses showOkCancelBox (MainComponent.cpp:3172), which goes through createAlertWindow and adds the Return/Esc shortcuts (build/_deps/juce-src/modules/juce_gui_basics/lookandfeel/juce_LookAndFeel_V2.cpp:405-425). In a bare AlertWindow, Return fires only when exactly 1 button exists and Esc does exitModalState(0) (juce_AlertWindow.cpp:560-570). If Quit is given value 0, Esc quits. Gates: QF-8 compares string constants (:654); L17-L20 feed choices straight into the flow (:542-544); the quit_window buttons are 'inert' (:452); G-OS5 'a test-mode quit ... shows no window' (:730).",
      "severity": "MUST",
      "proposed_change": "Fix the contract in the plan: Cancel = 0, Quit = 1, Save & Quit = 2; explicit KeyPress shortcuts; a pure QuitFlow::choiceFromReturn(int) with row QF-9 (0 -> Cancel, never Quit); mutant 'Esc/0 maps to Quit'. Add a test-server route that triggerClick()s each real button of the real window and records the choice, with the quit action stubbed. Do this without OS-level input. Add a Return/Esc row to B-6."
    },
    {
      "id": "ST-3",
      "target": "Q93 default A: Return = Save & Quit",
      "claim": "If Return really maps to Save & Quit, an accidental Cmd+Q in the middle of a set becomes one stray Return that writes over the show file and ends the app. Boris's hands are on a keyboard launcher, and the window is meant to protect against exactly this accident.",
      "evidence": "plan:431-434 'Return = Save & Quit' and :433-436 Save & Quit writes the show to its file through writeShow (with no backup for a v2 file, per the OS1-e rule at :146-153, which backs up only a non-v2 target) and then DoQuit. Q93 B is only offered as an alternative (:772-774). Pitfall 63/67-style keys are live performance keys (CLAUDE.md pitfall index 67: 'the shown deck is the grid').",
      "severity": "SHOULD",
      "proposed_change": "Make the default Return = Cancel (Q93 B) or give the window no Return binding; keep Esc = Cancel. If Boris wants Resolume's look, the accent colour can still sit on Save & Quit. Ask Q93 with this consequence written out, not as a default he can wave through."
    },
    {
      "id": "ST-4",
      "target": "OS5 QuitFlow Saving state and 'second Request does nothing'",
      "claim": "The flow assumes the Save As chooser's completion fires exactly once. It is not guaranteed. If it never fires, the flow is stuck in Saving and every later Cmd+Q or close-button press is swallowed, so the quit window never appears again and the app can only be force-quit.",
      "evidence": "plan:429 'A second Request while Asking or Saving does nothing'; :437-438 completion 'called exactly once'. All file choosers share ONE member, assigned at 17 sites (grep -c 'fileChooser_ = std::make_unique' MainComponent.cpp = 17; saveCompositionAs at :3585). JUCE's FileChooser destructor drops the callback without calling it (juce_FileChooser.cpp:130-133 'asyncCallback = nullptr'). A chooser replaced or destroyed while Saving therefore never reports SaveDone. QF rows (:650-655) test only the pure flow.",
      "severity": "SHOULD",
      "proposed_change": "Give Saving an exit. A Request while Saving checks that the save chooser is still alive; if not, the state falls back to Asking. Give the Save & Quit chooser its own member, or own it in the flow. Add row QF-9 'Saving + Request with no live chooser -> Asking + ShowWindow' and a mutant that drops the completion."
    },
    {
      "id": "ST-5",
      "target": "OS4 deck row single click = add deck",
      "claim": "One accidental click on a deck name in the list adds a deck AND makes it the shown deck, so Boris's ByPosition keys immediately fire the new deck's clips. Opening a show needs a double-click but this does not, and the rows now sit under every open header.",
      "evidence": "plan:313-315 'a click on a deck row adds that deck to the current show'. InsertDeckCmd sets the shown deck (DeckCommands.h:834 'comp->activeDeckIndex = addedIndex_'; its comment :811 'append under a fresh id, capture, show'). ByPosition bindings resolve against the shown deck (MainComponent.cpp:7690-7693 '-1 = the shown deck, ByPosition's box'; :7742-7744 velocity path uses getActiveDeck()). The plan's own wording at :317-318 calls an unsafe click asymmetry a loss for show rows but accepts it for deck rows.",
      "severity": "SHOULD",
      "proposed_change": "Take a deck on double-click of its row, matching the show header's double-click, or have the added deck NOT become the shown deck when added from the list. A single click does nothing, or only highlights if the list ever gains selection. Change Q91 and rows SL-3/B-2 to match. If the keys-follow-the-grid effect is kept, say so in B-2 so Boris checks it."
    },
    {
      "id": "ST-6",
      "target": "G-OS4-2 row OS-L16 / mutant MU-OS-18 (list reads off the message thread)",
      "claim": "The row proves the list does not freeze the window with a self-reported counter, which cannot see a parse hidden anywhere else and cannot see a stall. What Boris would feel is the Compositions tab or the set stuttering, and no row measures that.",
      "evidence": "plan:713-714 'OS-L16 ... messageThreadParses == 0'; the counter is the route's own field (:673 'messageThreadParses'); the mutant flips the counter's own call site (:714 'MU-OS-18 (the parse done in refresh): > 0'). The plan adds other message-thread reads: Import enablement from the summary (:309), the Import chooser's parse (:208), deck staging 'parse the show' (:349-350).",
      "severity": "SHOULD",
      "proposed_change": "Add a measured arm: during a refresh over 40 x ~1 MB shows, poll a cheap message-thread route (e.g. /api/debug/ui_text) and require max latency under a bar fixed in advance (e.g. 50 ms); the mutant is the parse in refresh(). Keep the counter as a second check only. Count parses in the shared summarize() entry so that a parse anywhere on the message thread shows up."
    },
    {
      "id": "ST-7",
      "target": "OS4 'What goes with it' / G-OS4-0: Boris's four saved deck files",
      "claim": "When Save Deck, Load Deck and the Decks section go, the four decks Boris actually saved stay stranded with nothing telling him. The 'find last month's deck' case is his own disk. The plan only guards against v2 deck files appearing, and never mentions the four he has.",
      "evidence": "facts-one-save.md:5 and :204 'Decks/ holds 4 legacy v1 files *.deck.json ... no v2 deck file'; plan:386-389 deck files 'never read, never listed, never deleted' and the hand-wrap applies only if 'a v2 deck file has appeared'; :832 'A converter for v1 deck files ... not in this lane'. Boris wrote 'A deck will go into all of the decks that are available...' (binding-decisions.md:884) and listed 'A deck - Save Deck' among his saves (boris-feedback-backlog.md:551-553), so he may believe those decks are reachable.",
      "severity": "SHOULD",
      "proposed_change": "Add a finding to section 9 and tell Boris plainly that his four v1 deck files cannot be opened (already true at 185147b). Offer Harmony a hand-wrap of copies into one-deck shows in the compositions folder before the merge, the plan's own mechanism, with his originals untouched. Make G-OS4-0 list the four files by name and checksum."
    },
    {
      "id": "ST-8",
      "target": "OS5 test route POST /api/debug/quit_window and visual states Q1/Q2",
      "claim": "The visual gate opens a real modal window in a test-mode app on the machine Boris is using. A modal AlertWindow activates the app and takes keyboard focus, so his typing could land in a test instance's Quit window. A failed probe could leave the window open. Neither case is covered by the screen-safety rules the plan cites.",
      "evidence": "plan:451-452 'the quit window over a playing show ... opened by POST /api/debug/quit_window'; :441-447 'enterModalState(true, callback, true)' with a modal that 'holds the keyboard'. HANDOFF.md SCREEN-SAFETY LAW (:105-117) and the end-of-batch window count (:185-187 '0 Audio-DNA ... windows after every batch') exist because a gate left a window on his displays. RIG-RULES.md has no rule on focus or activation (grep 'focus|activate' finds only the 'open -g' launch line, :59). 'Inert' buttons do not stop focus capture.",
      "severity": "SHOULD",
      "proposed_change": "Capture Q1/Q2 without showing a window: build the same component and use Component::createComponentSnapshot (or an offscreen render) to a PNG, and compare that with his screenshot. If a real window must open, the route must be test-mode only, close itself on a short timer even on error, and the probe must assert the window count is 0 after each use (the existing Quartz count), with a RIG-RULES line saying so."
    }
  ],
  "strongest_point": "The plan asks the quit window to do the job Boris fears most, Save & Quit that quits without saving or Esc that quits. It builds that window as a bare JUCE AlertWindow where Return does nothing and Esc returns 0 (juce_AlertWindow.cpp:560-570). Every live row (L17-L20) bypasses the real window and only feeds the flow its choice, so nothing proves the buttons are wired. Separately, the plan still carries Boris's question 86 as unanswered (plan:522), although he answered \"86 default yes\" at 13:22:56 (binding-decisions.md:898), so the old Save, Load, FX Save and ten slots would survive.",
  "citations_rechecked": true
}
```

## Seat: scope

Attack ids: SC-1 [MUST], SC-2 [MUST], SC-3 [SHOULD], SC-4 [SHOULD], SC-5 [SHOULD], SC-6 [SHOULD], SC-7 [NIT], SC-8 [NIT]

```json
{
  "seat": "scope",
  "attacks": [
    {
      "id": "SC-1",
      "target": "OS6.4 / S7 (plan lines 522-529, 568, OS8 line 548)",
      "claim": "The plan is stale on question 86 and, as written, would ship buttons Boris ordered removed. It treats 86 as 'ANSWERS (none yet)', so 'S7 does not start'. S7 is also the last stage, behind the visual gate.",
      "evidence": "plan-one-save.md:522 and :523-524 ('No answer: ... S7 does not start'). binding-decisions.md:898: 'Boris: \"86 default yes\" -> the small Save and Load, FX Save and the ten slots are removed now'. boris-clarify-86.md:20-21 holds 'ANSWERS ... 86 = default (A)'. boris-feedback-backlog.md:612-ff, BF72: 'The old look buttons go now'.",
      "severity": "MUST",
      "proposed_change": "Make S7 unconditional and delete the B branch and the 'no answer' branch. Move it before S4b. PresetManager then goes whole, so the dead v1 deck pair needs no separate surgery and test_preset_manager.cpp is deleted instead of having two cases edited. Keep V16 in the visual gate. Lost: nothing Boris wants. His Presets/ (4 entries) and FX Saves/ stay on disk as the plan already says."
    },
    {
      "id": "SC-2",
      "target": "G-OS5 / OS-L17, OS-L18 / quit_flow test routes (plan lines 715-718, 730, 426)",
      "claim": "Two of the four live quit rows cannot fail, and the live rows never touch the real quit wiring. L17 feeds 'cancel' through a test route and expects 'idle'. L18 expects a pid to exit on 'quit'. G-OS5 claims all four rows have 'their RED arms', but L17 and L18 name none; MU-OS-15 and MU-OS-16 belong to L19 and QF-4. The route injects the choice into QuitFlow, so Main.cpp's systemRequestedQuit -> requestQuit hop (the only line that decides whether Cmd+Q asks) is covered only by the LINT-5 grep and by Boris at B-6.",
      "evidence": "plan-one-save.md:715-718: 'OS-L17 `quit_flow {cancel}` -> state \"idle\"... OS-L18 ... pid exits within 10 s' with RED only on L19/L20. :730 'OS-L17..L20 PASS with their RED arms'. :426 test mode quits at once with no window, so cancel from Idle is Idle in any build. Main.cpp:84-90 is the close-button path.",
      "severity": "MUST",
      "proposed_change": "Drop L17 and L18 (QF-2/QF-3 already cover the pure flow; every probe already does an own-pid quit). Keep L19 and L20, which have RED arms. Add one RED-armed row for the real wiring: a test route that calls MainComponent::requestQuit() in a non-test-mode flag and reads state 'asking' with no window. Reword G-OS5 to name only rows that have an arm."
    },
    {
      "id": "SC-3",
      "target": "OS1-d dead keys (plan lines 139-145, R14 :812, SF-8)",
      "claim": "Dropping seven composition keys is not a request from Boris and is not needed for one Save. It adds a field, write and read edit across Composition.h, two test edits and MU/SF-8 rows. The plan admits the premise 'rests on greps ... not on a build' and that the drop stops if a reader exists. It protects nothing and adds a regression surface to the version-writing stage.",
      "evidence": "plan-one-save.md:139-145 (runner-up 'drop nothing -- loses only on tidiness'); :812 R14; :174-176 edits test_composition.cpp :386 and :519. Boris's 'One Save' text (binding-decisions.md:874) asks for nothing here. Only outputDisplay has a hand-off (plan-outputs.md:849).",
      "severity": "SHOULD",
      "proposed_change": "Remove OS1-d, SF-8 and MU-OS for it from S1. Keep only the removal of outputDisplay, because the outputs lane hands it over. Old files carrying the keys still load. Lost: six dead keys stay written until a tidy lane. No behaviour changes."
    },
    {
      "id": "SC-4",
      "target": "OS4 list of shows: header right-click 'Import Key and MIDI Settings' and keyCount (plan lines 206-209, 313, 325-329, SS-6, SL-4, V8, V9)",
      "claim": "Import has two entry points for one command. The menu item 'Import Settings from Show...' (chooser) is what Boris's words need. The right-click copy forces ShowSummary.keyCount, an enabled/greyed state, SS-6, SL-4, two visual states (V8, V9) and a route field. The double-click-opens-show gesture also creates the R8 first-click-toggles-twice ambiguity, which the plan admits the visual gate cannot see.",
      "evidence": "plan-one-save.md:206-209 and :313 (two Import sites); :325-329 (keyCount in the summary); :800-801 R8 ('the visual gate cannot see it -- B-2 does'). Boris: 'they can import the settings from another show file' (binding-decisions.md:887), a single command with no stated place.",
      "severity": "SHOULD",
      "proposed_change": "Keep only the Shortcuts menu Import. Remove the right-click Import item, keyCount, SS-6, SL-4's enabled-state case, V8/V9 and the keyCount field in show_list. Make header click = open/close and keep 'Open' in the right-click menu, which E17 shows already exists, so no double-click timing is needed. Lost: one-click Import from the list, and the double-click open."
    },
    {
      "id": "SC-5",
      "target": "Section 7 questions 92 and 93 (plan lines 769-774)",
      "claim": "Two of the three new questions are not Boris's to answer and add work to a non-technical user who asked for simpler. 92 (list order) asks whether to change something nobody asked to change: sorting is 'as today' and B is a new sort feature. 93 (the Return key) is fixed by his own screenshot, where the highlighted button is Save & Quit, and the plan already defaults to it.",
      "evidence": "plan-one-save.md:769-774 ('A (default) By name, as today' / 'Return = Save & Quit (the highlighted button, as in Resolume's window)'); :321 'built as A'; FO-M4 says today's order is by path. Boris's quit instruction: 'Look at the image' (binding-decisions.md:822).",
      "severity": "SHOULD",
      "proposed_change": "Strike 92 and 93 from section 7 and state them as Harmony's decisions: order by name as today, Return = the highlighted button of his screenshot. Keep only 91, which decides whether a click replaces what is playing."
    },
    {
      "id": "SC-6",
      "target": "OS8 / section 4: one lane, one worktree, strictly serial, one merge after the visual gate",
      "claim": "The order makes the lanes behind this one wait on pieces that are independent of them. S5 (quit window) needs only S1's writeShow. S7 is independent. S3 (layout) is independent of S2 (keys). The messages lane waits on S4b's removal of the Save Deck / Load Deck boxes, but the visual gate (V1-V15, Q1, Q2, five critics) sits after S5 and gates the single merge. The version and backup rule that protects his only old-shape show cannot reach main until the list restyle is critic-approved.",
      "evidence": "plan-one-save.md:547-550 ('One lane ... strictly in this order'); :559-568 (VG after S5, S7 last); :571-576 (messages lane must not plan on boxes that leave in S4b). E7: his one show is old-shape and the first Save rewrites it.",
      "severity": "SHOULD",
      "proposed_change": "Keep one worktree but declare two merge points. M1 = S1 + S7 + the deck-menu removals, with a G-OS-HIS gate. M2 = S2, S3, S4a/b, S5 with the visual gate. This frees the messages lane after M1, and Boris gets the backup rule before the list restyle. Lost: one extra merge gate run."
    },
    {
      "id": "SC-7",
      "target": "OS2 applyKeySet guard and per-binding enum filter (plan lines 231-236; KS-5, KS-6)",
      "claim": "Two mechanisms are paid for code paths the plan says do not exist. The 'dispatching depth counter + isDispatching + re-post via callAsync' guard is for a caller that 'No path reaches today (INFERRED)'. The unknown-enum drop is for a set 'written by a later build', and no later build exists. Together they touch BindingManager's per-key dispatch loop, which iterates bindings_ calling a callback, and add two unit cases.",
      "evidence": "plan-one-save.md:231-234 ('No path reaches this today (INFERRED ...); the guard is for the next caller'); :235-236 (later build); :626-628 KS-5, KS-6; BindingManager.cpp:60-100 shows the synchronous actionCallback_ loop the counter would wrap.",
      "severity": "NIT",
      "proposed_change": "Drop the depth counter, the isDispatching accessor and the re-post, and drop the enum filter (a bad enum already reads as today). Keep parseSet's no-set rules, exitAllBindingModes, releaseAllMomentary and the order they run in (KS-7), which fix real held-key hazards (E12). Add one comment in applyKeySet saying it is message-thread only and must not be called from an action callback."
    },
    {
      "id": "SC-8",
      "target": "OS1-e later-version branch and F-V3 fixtures (plan lines 156-158, SF-7, SB-2, R4)",
      "claim": "The 'show from a later version' protection (version 3+ opens best-effort, backs up as .v3.json) is built for a file the plan says cannot exist until another lane makes one. It adds a fixture (F-V3), a unit case (SF-7), part of SB-2 and a risk entry, and widens what the version test means.",
      "evidence": "plan-one-save.md:156-158 ('no such file exists until a later lane makes one'); :611 SF-7; :616 SB-2; :791-792 R4.",
      "severity": "NIT",
      "proposed_change": "Keep the rule as the single comparison 'version != 2 -> back up before overwriting'. That is one line and already covers v3. Drop F-V3, SF-7 and the v3 half of SB-2, and let the lane that creates version 3 pin its own reader. Lost: nothing now."
    }
  ],
  "strongest_point": "OS1-e, the backup before the first overwrite of an old-shape show, must not be cut. It is written to fail closed: if the copy cannot be made, nothing is written. It also covers Save As and Collect Media's copy, and it is gated by OS-L13, OS-L14 and G-OS-HIS. Boris's one real show is old-shape (E7), and the first Cmd+S rewrites it with no backup (E3). Boris never asked for the extra machinery, but this is the one place the lane could destroy his work, and a rule about the file on disk is cheaper than any in-memory 'changed' flag.",
  "citations_rechecked": true
}
```

