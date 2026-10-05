# ATTACK PAPERS -- lane "looks-answers2" (the per-effect looks lane, second delta on Boris's answers 131-134 and 138)

Filed by the architect BEFORE ruling, as received from the workflow harness. 2 of 2 seats returned.
The plan attacked: .harmony/.reports/s-rta-1004b/plan-looks-answers2.md
The ruling: .harmony/.reports/s-rta-1004b/ruling-looks-answers2.md
Source text: one JSON array of 19583 characters, closed (ends with "}]"), parsed without error. Below it is
pretty-printed (indent 2, key order kept, no character of any value changed), one section per seat.
Seats and attack counts: gates 8, stage-hands 7 (total 15).

---------------------------------------------------------------------------------------------------------
## SEAT: gates (8 attacks: GA-1, GA-2, GA-3, GA-4, GA-5, GA-6, GA-7, GA-8)
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "5.3 live mutant build M-H and GL-11 step (a)",
      "claim": "The plan says M-H = MU-P2-1 + MU-P2-3 cannot mask each other and that GL-11 goes RED 'only on the line loaded none after a relaunch'. That is false: GL-11 step (a) itself asserts `button` = {Off, dim false} right after the 0.25 tweak, which MU-P2-1 breaks first (button reads Presets, dim). The row fails in step (a) with a different line, and MU-P2-3's RED arm (the reader ignoring fxPreset, reached only in step (c) after the relaunch) is never shown.",
      "evidence": "plan-looks-answers2.md:517-519 ('GL-11 only on the line \"loaded none after a relaunch\" (MU-P2-1 does not touch `loaded`)') against :570-571 (step (a): `loaded` \"Off\", `matched` \"\", `button` = {\"Off\", dim false}) and :513 (MU-P2-1 'the text comes from the values alone'). Step (a) reads the button, not just `loaded`.",
      "severity": "MUST",
      "proposed_change": "Build MU-P2-3 alone for GL-11's RED arm (or let the probe run all steps and print every failing step, with the RED bar being the presence of the exact FAIL line 'loaded none after a relaunch'). Drop the 'cannot mask' sentence, or restrict M-H to GL-10 versus MU-P2-1."
    },
    {
      "id": "GA-2",
      "target": "LINT-EL-7 'exactly six lines' and S2's 'ONE read line' per file",
      "claim": "The pin is stale against the idiom the plan itself prescribes. P2-7 says the readers use `hasProperty` ('the files' own idiom'). That idiom spends TWO lines carrying the key literal: `if (fxObj->hasProperty(\"dryWetConn\"))` then `fromVar(..., fxObj->getProperty(\"dryWetConn\"))`. Following it gives 1 write + 2 read lines in each of 3 files = 9 lines, not 6. The lint is RED on correct code, or the builder contorts the reader into a one-liner to satisfy the count.",
      "evidence": "src/model/Clip.cpp:305-306, src/model/Layer.cpp:248-249, src/model/Composition.h:1096-1097 (each pair holds the literal twice). Plan :529-531 (LINT-EL-7 'exactly six lines: one write and one read in each'), :378 (S2 'ONE write line and ONE read line'), :282-283 (hasProperty idiom).",
      "severity": "SHOULD",
      "proposed_change": "Pin by file and by operation: `grep -c '\"fxPreset\"'` is exactly 3 per file in Clip.cpp, Layer.cpp and Composition.h (1 write + hasProperty + getProperty), 0 elsewhere in src. Or state the one-line read form so the count of 6 is reachable."
    },
    {
      "id": "GA-3",
      "target": "LINT-P2-1 last clause (`grep -il look` on every new file)",
      "claim": "As worded this cannot be GREEN for honest new files. 'look' as bare letters is inside LookAndFeel, AudioDNALookAndFeel, setLookAndFeel, lookup and looking, which a new UI header and its test must use (CLAUDE.md UI patterns; the existing tree has 441 AudioDNALookAndFeel and 77 lookup hits). S3's EffectPresetsButton.h / EffectPresetsMenu.h will be RED, so the builder either avoids the app's look-and-feel (against project pattern) or the bar is quietly loosened. The plan says a bar is 'met or reported, never loosened'.",
      "evidence": "plan :537-539 ('no NEW file ... holds the letters \"look\" in any letter case at all -- grep -il look prints nothing'). Count from `grep -rhoiE '\\w*look\\w*' src tests`: AudioDNALookAndFeel 441, LookAndFeel 129, lookup 77, setLookAndFeel 23, looking 19. The first clause already checks the whole word and the exact old identifiers.",
      "severity": "SHOULD",
      "proposed_change": "Replace the last clause with a word-level test, `grep -iwE 'looks?'` plus the identifier family `grep -iE 'lookName|looksFor|LooksButton|EffectLook|look_(make|load|ui|replace)|looksDir'` on the new files, and an explicit allow-list for LookAndFeel and lookup. Keep R7's concern (coined `lookStore`) in the review."
    },
    {
      "id": "GA-4",
      "target": "GL-8 count gate and the N0 baseline",
      "claim": "The bar proves arithmetic, not content, and its RED arm is vacuous. 'ctest N = N0 + 83' passes if a case is missing and an unrelated one added, or a row is renamed. The RED arm ('the lane's base: N0 is not N0 + 83') is true of any tree without the lane, so it shows nothing about the gate. N0 is also taken once on the base, yet the plan's own order and fence allow re-bases (one-save stages S2-S6 follow S7 and add tests), so N0 goes stale and S3's 'restate the count' does not say re-measure N0. The order line '[one-save S1 merged]' after 'S7 merged' is also vacuous because S1 precedes S7.",
      "evidence": "plan :559 (GL-8), :382-383 (N0 once, on the base), :388-389 (order), :400-404 (re-base rule), :741 (R12). ctest names are visible: tests/CMakeLists.txt uses catch_discover_tests (e.g. :24, :31). ruling-one-save.md:471 ('S1 -> M1 -> S7 -> S2 -> ... -> M2').",
      "severity": "SHOULD",
      "proposed_change": "GL-8 also runs `ctest -N` and checks that each of the 83 pre-registered names (5.1/5.2 plus the unchanged ids) appears exactly once, as a named list in the probe. Its RED arm deletes one case and renames one. N0 is re-measured on every re-base and at the final head's base, and the log prints both. Strike the vacuous S1 bracket."
    },
    {
      "id": "GA-5",
      "target": "P2-7 'byte for byte' claim, LC-14, GL-11 step (d)",
      "claim": "The claim that a show with no loaded preset saves byte for byte as before is gated only by the case where the probe's fixture happens not to trigger writer (3). Writer (3) latches a name on any slot whose settings equal one of his presets when its row is built (a build the view does on any rebuild), and the three writers then save it. So opening a show, viewing an effect that happens to equal a preset, and saving adds fxPreset keys. GL-11 (d) relaunches untouched show B and passes only because Invert's defaults equal no fixture preset. No row pins the writer-3-then-save path.",
      "evidence": "plan :255-256 (writer 3), :280-281 ('byte for byte'), :575 (GL-11 d), :506 (LC-14 pins only 'an effect with no name writes no key'), :493 (LM-22 covers writer 3 on the model, not the file).",
      "severity": "SHOULD",
      "proposed_change": "Add a GL-11 arm (e) where an effect's values equal a pre-written preset: expand its row, save, then count the fxPreset lines and record the ruled outcome (expected 1 if writer 3 stands), and word P2-7 'byte for byte' as 'when no row was built over a matching effect'. Or add a unit case on rebuild-then-save."
    },
    {
      "id": "GA-6",
      "target": "Section 6 checks 14 and 19 versus the rows (what only Boris can check)",
      "claim": "Two entries hide claims a machine could test with the existing routes. Check 19 (question 152 A: after a rename the other effects 'show it too while they still are exactly that preset; the ones you changed read Presets') has no unit or live row; LM-19 covers only the renaming row. Check 14 ('Copy that folder across and the names are back'; a missing preset reads Presets) is only partly covered by LM-24, with no copy-the-folder case. The rename-carries-the-name rule (P2-6 writer 2) is unpinned for other effects.",
      "evidence": "plan :665-667 (check 19), :651-655 (check 14), :490 (LM-19 text), :507 (LM-24), :683-686 (question 152). The data routes are in the plan (`fx_preset_ui rename:<name>`, GET fx_presets `loaded`/`button`).",
      "severity": "SHOULD",
      "proposed_change": "Add LM-25 (rename from row A: row A reads the new name; row B, still equal, reads it through firstMatch; row C, changed, reads Presets, dim; no slot name rewritten) and extend GL-11 with a copy-the-folder arm into a fresh AUDIODNA_EFFECT_PRESETS_DIR. Count becomes 84; GL-8 and R12 follow."
    },
    {
      "id": "GA-7",
      "target": "'No mark' pin (P2-6) and V-23",
      "claim": "The rule says the text, the colour AND the triangle are identical whether the settings equal the preset or not, but the only unit row (LM-3) tests the pure function `buttonState` -> {text, dim}. The triangle and any other paint input are outside it, so a mutant that changes a paint detail (a mode, a mark glyph) while buttonState stays the same passes. V-23 is judged by LLM critics who are told 'this sameness is his answer', a primed gate. The model manifest 'button = {text, dim, mode}' includes `mode`, which the new rule does not pin.",
      "evidence": "plan :245-246 (colour and triangle pinned), :241-244 and :505 (LM-3 reads text/dim), :589-592 (V-23 critics told the sameness is his answer), ruling-effect-looks.md:363 and :640 (button = {text, dim, mode}); LM-10 (RU:539) compares what is painted but is not re-tied to the loaded-name states.",
      "severity": "SHOULD",
      "proposed_change": "Add to LM-3 (or LM-26): the button's paint key (everything setState compares and paints: text, dim, mode, triangle) is equal for 'loaded name, values equal' and 'loaded name, values far', and unequal for a never-given-a-preset slot that is moved. Add a RED arm: a mutant that adds a mode or mark on a change. Do not tell the critics what the answer is."
    },
    {
      "id": "GA-8",
      "target": "5.6 V-24 (MilkDrop tab beside an open Presets menu)",
      "claim": "V-24 needs the browser's MilkDrop tab and an effect's Presets menu in one picture. The menu is a juce::PopupMenu, which is its own native window, and the visual gate is 'window-id captures only'. The plan does not say how one window-id capture contains both. The gate may pass with only a manifest, or the state cannot be captured, and P2-4's first row (the whole ambiguity argument) rests on it.",
      "evidence": "plan :592 (V-24), :380 (VG: window-id captures only); ruling-effect-looks.md:366-367 (the menu is the PopupMenu's callback), :653-654 ('NOT capturable without synthetic input': a sub-list, drop highlight, hover). The plan does not list V-24 under 'NOT capturable' (:596 points to RA:719-720).",
      "severity": "NIT",
      "proposed_change": "Say in V-24 how it is captured: two window-id captures (main window, menu window) composed offline, or the menu replaced by the unfolded row plus the manifest of menu items. Add V-24 to the 'not capturable' list if neither works. Never use a full-screen capture."
    }
  ],
  "strongest_point": "GA-1: the plan's live-mutant economy (M-H = MU-P2-1 + MU-P2-3) claims the two mutants cannot mask each other, but GL-11 step (a) reads `button` and MU-P2-1 breaks it before the relaunch step. So MU-P2-3's live RED arm, the one proving the show really carries the name across a re-open, is never shown on its own FAIL line. That claim, in the plan's own words, is the lane's main addition beyond Boris's words.",
  "citations_rechecked": true
}
```

---------------------------------------------------------------------------------------------------------
## SEAT: stage-hands (7 attacks: ST-1, ST-2, ST-3, ST-4, ST-5, ST-6, ST-7)
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "P2-6 / P2-7 with ST-2's nextName (plan:264, :308-310, :475, R4 :717-720)",
      "claim": "A rename frees a number that the name box then hands out again, and the remembered, saved name makes every old carrier claim the new preset. Rename \"Preset 2\" to \"Wobble\" from one effect. Other effects in the show still carry \"Preset 2\". The next New Preset box offers \"Preset 2\" (smallest free N). Make it, and every tweaked effect that carried the old one reads \"Preset 2\" again, ticked. Each is now offered Save over \"Preset 2\" against a different preset. R4 covers only delete-then-recreate by hand. With a persisted name, rename plus the default box makes this routine.",
      "evidence": "Plan:475 (ST-2: \"a freed number is used again\"); :257-259 (the box holds the next free \"Preset N\"); :264 and :308-310 (a rename from another effect's row leaves the other slots' names, question 152 default A); :717-720 (R4 \"Not closed further here\"). Boris sees: a button naming a preset that is not what the effect was loaded from, in last week's show.",
      "severity": "SHOULD",
      "proposed_change": "Make 152 B the default: Rename carries the new name to every slot of the open show that carried the old one. Or have nextName skip any name a slot of the open show still carries, and add a unit row for it. At minimum, extend R4 and check 19 to the rename path, with a row that renames and then makes."
    },
    {
      "id": "ST-2",
      "target": "P2-6 case (2)/(3), F-LB2-c, question 150 default A, V-3 (plan:265-268, :301-304, :585)",
      "claim": "The default for an effect never given a preset is a visible change-mark, the thing his 133 declined. Add an effect: it reads \"Default\". Touch one slider and the text flips to \"Presets\" AND dims. That is the commonest effect there is. His line is \"It can be called look 2 but no need to show that it was changed. Effects are usually changed by the user.\" The plan rejects 150 B because the effect \"would read Default while not being at its defaults\". That is exactly the case it accepts for \"Preset 2\". His own reason, that effects are usually changed, applies here too.",
      "evidence": "Plan:265-268 (case 2 to case 3 on one slider move); :301-304 (F-LB2-c); :594-595 (critics told \"the dim 'Presets' means ... no preset\"). Boris's words: BD:994-995 (133). A VJ mid-set sees the label flip and dim on every effect he touches.",
      "severity": "SHOULD",
      "proposed_change": "Make 150 B the default: an effect keeps reading \"Default\" until a preset is loaded or made, with no dim. Dim stays only for a name that no preset lists (LM-24). The cost is the three clauses the plan already lists at :688-690."
    },
    {
      "id": "ST-3",
      "target": "Section 6 items 10 and 15 against P2-6 case (1) (plan:638-642, :656-657, :245-247)",
      "claim": "Save over now gives no sign on the button that it worked or failed. The button reads \"Preset 2\" before, after success and after failure. Item 15 says that on a full disk or locked folder \"the button keeps reading what it read\". For Save over that is identical to success, and Save over is the one act with no undo and no message. The only sign is the menu item greying (LM-18), and Boris is not told to look there. New Preset still has a sign (the button takes the new name).",
      "evidence": "Plan:638-642 (item 10 says only \"click Save Over: Preset 2 now holds the change\"); :656-657 (item 15); :489 (LM-18: greyed while the settings equal it, live after a tweak); :245-247 (\"no mark\" pinned). Boris sees a window close and nothing else, and cannot tell a lost save from a saved one without opening the menu.",
      "severity": "SHOULD",
      "proposed_change": "Rewrite item 15 for Save over: \"after Save Over, open the menu: Save over is greyed if it saved, still live if it did not\". Add that menu state to V-21's manifest and to GL-10's failure arm. Add a live row where Save over fails (read-only folder) and the menu stays live."
    },
    {
      "id": "ST-4",
      "target": "LC-13 / writer (2) vs the Edit menu (plan:252-255, :484, :638-642, :615-617)",
      "claim": "Cmd+Z after New Preset or Save over undoes something else, and the plan never tells Boris. Names and files from New Preset and Save over are written outside the undo stack. The nearest undo step is the earlier preset load, and its undo restores the pre-load values and the earlier name. Sequence: load Preset 2, tweak, Save over, then Cmd+Z to take the save back. The effect jumps to its pre-load settings. The file stays overwritten, and the button name reverts. Item 10 says only \"There is no undo for a Save over\".",
      "evidence": "Plan:252-255 (writers; LC-13: \"undo puts the earlier one back\"); :484 (LC-13); :638-642 (item 10); :615-617 (item 5 describes Cmd+Z only after a load). RU AM-6: \"Making, renaming and deleting a look are not undo steps\". INFERRED: whether the slider tweak is itself a step was not read.",
      "severity": "SHOULD",
      "proposed_change": "Add to item 10: \"Cmd+Z after New Preset or Save Over takes back the last LOAD, not the save\". Pin it with a unit row: undo after make keeps the file and restores the pre-load values and name. Or push a no-op undo marker so Cmd+Z after a save does not reach the load."
    },
    {
      "id": "ST-5",
      "target": "Section 6 item 4 and GL-4's table (plan:611-614; RU:219, :621-623, :749-750, :818)",
      "claim": "Item 4 states as fact \"For one frame the screens hold the picture they had\". The pre-registered table allows up to 2 held frames per load and, above that, still says \"the fence stays\". Every outcome except black>0 keeps the fence, so the row cannot fail on a visible hitch. RU:219 also says each fenced frame gives the recorder and Syphon NO frame, and item 4 never tells Boris. A VJ feeding Syphon or recording mid-set sees a dropped frame on every preset load, at 20 loads with 100 ms between.",
      "evidence": "RU:621-623 (the table: hold above 40 gives \"fence stays for now\"); RU:749-750 (HD-8 default keep); RU:219; RU:818 (R4); plan:611-614 (item 4's wording; \"wrong\" lists restart and black only). Boris sees a stutter in the recorded or Syphon feed that nobody warned him about.",
      "severity": "SHOULD",
      "proposed_change": "Make item 4 say what GL-4 measured (the number, and that recording and Syphon skip those frames). Make \"hold above 40\" a STOP that Harmony rules before S3, not a pass. Add a check where Boris loads a preset while recording and watches the take."
    },
    {
      "id": "ST-6",
      "target": "P2-6 menu rule \"clicking the ticked preset puts the effect back\" and R2 (plan:247-249, :712-714; section 6 items 6 and 7c)",
      "claim": "The plan offers the ticked name as the guard against a drifted effect (\"one click puts the effect back on it\"). That click is a full load: it unplugs every signal he wired after loading, and Dry/Wet wiring too (RA-10 Unplug; LC-9). The same goes for the top \"Default\" item, one click from the button. Item 6 tells him the click is safe and takes only sliders back. Item 7c says Cmd+Z is the only way back for signals, but never links it to this click. A VJ who rewired live and clicks the name to \"reset the sliders\" loses his routing with no sign.",
      "evidence": "Plan:247-249 and :712-714 (R2: \"The guard he gets: the menu ticks it and one click puts the effect back\"); :618-620 (item 6 \"the effect is back on Preset 2 (Cmd+Z takes that back)\"); :625-627 (item 7c, undo only). Boris's words BD:988-993 (Cmd+Z, \"no other way\") mean this is allowed, but he was not told this click is the trigger.",
      "severity": "NIT",
      "proposed_change": "Item 6 should add \"this also unplugs any signal you added since, and Cmd+Z brings both back\". Add \"or click the ticked preset\" to item 7c's wrong-list."
    },
    {
      "id": "ST-7",
      "target": "LINT-P2-2 / the fence with one-save S7 (plan:539-541, :392-399; E17)",
      "claim": "The fence lint can stay RED after S7 for a reason unrelated to the old Save buttons. `grep -rnw PresetManager src` also hits comments and an include that S7's file list (PresetManager.h/.cpp, test) does not obviously remove, so the lane's start is gated on a scrub S7 has not been told to do. The lane then waits on a lint that is red for words, with the old buttons already gone and nothing replacing them on Boris's screen.",
      "evidence": "Verified at 7bc6df7: src/ui/CompDecksBrowser.h:38 and :45 (comments naming PresetManager and getDeckDirectory), CompDecksBrowser.cpp:304 (comment), src/test/TestServer.cpp:1232 (comment), src/MainComponent.h:15 (#include \"ui/PresetManager.h\"), MainComponent.cpp:387-388 (startup PresetManager::loadPreset). Plan:539-541 prints any line as a start-blocker.",
      "severity": "NIT",
      "proposed_change": "Give one-save S7 an explicit list: scrub those comment lines and the include, and decide the CompDecksBrowser dependency on getDeckDirectory (HD-20). Or reword LINT-P2-2 to code tokens (`PresetManager::`, the include) so comments cannot block the lane."
    }
  ],
  "strongest_point": "The strongest point is ST-1. The plan makes the loaded name sticky across tweaks and across sessions (it is saved in the show), and still has nextName reuse a freed number. So a plain Rename lets the next New Preset box silently re-attach every old carrier to a different preset, each with a live Save over \\\"<that name>\\\". Boris sees a button that names a preset the effect never came from, in a show he opens a week later. The plan's own R4 admits the identity gap and only guards it with a confirmation window.",
  "citations_rechecked": true
}
```

STATUS: DONE
