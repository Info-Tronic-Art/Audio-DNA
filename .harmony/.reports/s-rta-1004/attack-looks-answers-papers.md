# SEAT PAPERS -- lane "looks-answers" (blind council; 3 of 3 seats returned)

Written verbatim by the architect before ruling (s-rta-1004). Source: the dispatch text, 27419 characters of JSON (the dispatch states 27419), parsed and pretty-printed; no word changed. The JSON ended with a closed array. The plan attacked: .harmony/.reports/s-rta-1004/plan-looks-answers.md. The ruling: .harmony/.reports/s-rta-1004/ruling-looks-answers.md.

NOTE ON IDS: the seats number their attacks DA-n / GA-n / ST-n. The plan ALSO uses DA-n for its delta amendments and RU uses ST-n for store test rows. In the ruling the seat attacks are written A:DA-n, A:GA-n, A:ST-n to keep them apart.

Attack ids by seat: data-safety = DA-1, DA-2, DA-3, DA-4, DA-5, DA-6, DA-7, DA-8; gates = GA-1, GA-2, GA-3, GA-4, GA-5, GA-6, GA-7, GA-8; stage-hands = ST-1, ST-2, ST-3, ST-4, ST-5, ST-6, ST-7

---------------------------------------------------------------------------------------------------------
## SEAT: data-safety
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "data-safety",
  "attacks": [
    {
      "id": "DA-1",
      "target": "DA-9 writer (3), lookName rewritten by refresh(); R11; LM-18; GL-10",
      "claim": "Save over can overwrite the WRONG look, with no undo. Writer (3) sets lookName to whatever the button derives, i.e. firstMatch, after every refresh. When two looks hold identical wiring and values (R11 calls this harmless), firstMatch is the earlier name, so loading Look 5 is silently overwritten by Look 2 as the 'loaded look'. Tweak, Save over 'Look 2', confirm: Look 2 is destroyed, not the look he loaded.",
      "evidence": "plan-looks-answers.md:248-251 (writer 3: 'whenever the derived name ... is one of his looks, lookName is set to it'); :612-613 (R11, ties 'harmless, said'); :284/:601 (no undo). MU-LA-22 (:476) forbids Save over naming firstMatch, yet writer 3 reintroduces it. LM-18 (:459) and GL-10 (:511) use two looks with different values (Off, Far), so no row exercises a tie.",
      "severity": "MUST",
      "proposed_change": "Writer (3) fires only when lookName is empty or the named look no longer matches the settings; it never replaces a loaded name that still matches. Add LM-18b: Look A and Look B with equal settings, load B, tweak, and Save over must name B. Add a GL-10 arm with a tie, with its own mutant."
    },
    {
      "id": "DA-2",
      "target": "DA-8 Save Over confirm + DA-12 guard",
      "claim": "The Save Over confirm has no stated re-check, and it is the one destructive, un-undoable write. The box's callback re-checks the row index (DA-12). The confirm's does not, and an index/name check cannot tell clip A's Ripple from clip B's Ripple. A MIDI pad or REST call that changes the selected clip while the modal is open makes 'the settings on now' another effect's. The text promises 'the settings on now' but the plan never says when they are captured.",
      "evidence": "plan-looks-answers.md:240-241 (confirm text/buttons only); :308-309 (guard only for the name box, 'LM-6's guard'); :279-285 (no undo); MIDI keeps firing while the box is open :314-315, so the selection can change.",
      "severity": "SHOULD",
      "proposed_change": "Capture the look from the slot when he picks the menu item, and write exactly that on 'Save Over'. Make the callback verify scope identity (layer/column/slot), the effect name, that the loaded look still lists, and that it is not already equal. If any check fails, write nothing. Pin it in LM-18."
    },
    {
      "id": "DA-3",
      "target": "DA-2 refusal list, 'version never gates reading', DA-10 'Save over drops unknown keys', R8",
      "claim": "The look reader is lenient exactly where a newer or hand-edited file would change meaning, and Save over then makes the damage permanent. fromVar maps an unknown curve, playback or interp name to Linear/Forward/Linear without counting it. A missing macro 'index' reads as 0 (Macro 1). Only an unknown source kind or LFO shape feeds unknownKindCount, the only gate DA-2 names. A file from a newer build is read by 'the same rules' and rewritten fresh, dropping its extra keys.",
      "evidence": "ConnSerialization.cpp:56-66, :74-80, :87-92 (silent defaults), :188 (static_cast<int> of a missing var is 0), :195-196 and :253-254 (the only increments). plan-looks-answers.md:131 ('version ... never gates reading'), :136-138 (refusals), :276 and :607 (R8: Save over drops unknown keys).",
      "severity": "SHOULD",
      "proposed_change": "Before fromVar, a look-file validator checks every enum name against the tables, requires macro 'index' and signal 'name' to be present, and rejects a version above 2 as values-only. The store's replace() refuses to overwrite a file with a version above 2 or with keys it does not know. Add LK-20 clauses for each case."
    },
    {
      "id": "DA-4",
      "target": "DA-2 'the show's own shape, one writer, one reader'; LK-16; S1 tests",
      "claim": "A look's on-disk format is now whatever ConnSerialization writes today, and nothing pins it. Looks live for years, and the plan itself says three serializers are being edited by the one-save lane. A later edit to a key name or a default in ConnSerialization passes every planned test, because LK-16 only round-trips with the same code, and strands or re-means every look on disk.",
      "evidence": "plan-looks-answers.md:128-131 (delegation to toVar), :417 (LK-16 is a round trip), :260 (serializers under edit by another lane). ConnSerialization.cpp:95-160 shows string keys 'src', 'shape', 'min', 'max', 'smoothMs', and defaults supplied by ConnShape{} (:164-169).",
      "severity": "SHOULD",
      "proposed_change": "Add LK-23: a hand-written look file, committed as a fixture with a literal 'conn' of each source kind. It must load to exact field values (range, invert, curve, smoothing) and must never be regenerated by the test. S1's packet says that a change to this fixture needs a ruling."
    },
    {
      "id": "DA-5",
      "target": "DA-4 sleeping wire premise, R2, section 6 item 6c, C12",
      "claim": "The premise that a Signal wire can name a signal the show does not have ('a user signal') is false at 185147b. Nothing calls SignalRegistry::addSignal, so no user can make a signal and the 32 built-ins always exist. Boris check 6(c) ('a signal you made yourself ... make a signal with that name') cannot be performed. R2's 'rare' argument is wrong, not just weak. The real hazard is the opposite: a later build renaming a built-in signal ('Air', 'Hit') silently sleeps every look keyed to that name, and no pinned list guards the second name space.",
      "evidence": "grep -rn addSignal src tests docs/claude CLAUDE.md prints only SignalRegistry.h:24 and SignalRegistry.cpp:88 (the definition). plan-looks-answers.md:163-164, :539-541, :592-593. RU LK-13 pins only effect names (ruling-effect-looks.md:53, :206).",
      "severity": "SHOULD",
      "proposed_change": "Reword DA-4 and R2: the wire can sleep only after a signal rename or a hand-edited file. Replace Boris check 6(c) with one he can do. Add a pinned list of the 32 built-in signal names (LK-13's twin) that goes red on a rename. Keep the engine line, which is still right for renames."
    },
    {
      "id": "DA-6",
      "target": "DA-12 nameVerdict / nextName inputs",
      "claim": "The box's verdict is computed from an undefined name set, so 'Taken' can pass as Ok and Return then silently makes nothing. The plan says nextName counts files on disk, including unreadable ones, but nameVerdict takes 'the effect's file names'. The store's list is cached per run, and a Finder-added file appears only at next launch. On APFS, names that differ only by Unicode normalisation are one file, which a case-insensitive string compare treats as free. The box then closes with no look, no file and no message, after he typed a name and pressed Return.",
      "evidence": "plan-looks-answers.md:296-311 ('the effect's file names', 'path must not exist', callback 'makes nothing'); ruling-effect-looks.md:273-274 (list read once per run, a file added by hand later appears at next launch); plan :300-301 (compare 'without letter case').",
      "severity": "SHOULD",
      "proposed_change": "The name set for the verdict is a fresh directory scan at the time of each check, including unreadable and dot-less files, compared with File::exists on the built path as well. If make still fails at accept, keep the box open with the taken state instead of closing it. Add an LM-17 clause: a file present on disk but absent from the list reads Taken."
    },
    {
      "id": "DA-7",
      "target": "LA4 / section 6 item 14: the old Presets files",
      "claim": "The re-write of item 14 tells Boris the old buttons go and that 'your two old presets were deleted', and drops the sentence that told him what stays. After one-save S7 removes the buttons, the other legacy files become unreachable with no word to him: Presets/fast_saves (9 entries, 2026-03-14), Presets/'test 1.deck.json', and the four Decks/*.deck.json. His 'delete them' named two presets only.",
      "evidence": "plan-looks-answers.md:333-334 ('Dropped: ... item 14's second sentence'), :558-559 (item 14 re-written); s-rta-1004-work.md:43 ('LEFT ALONE, not named by him: Presets/fast_saves (9 entries...) and Presets/\"test 1.deck.json\"; the four *.deck.json in Decks'); ruling-effect-looks.md:692 (the dropped sentence); binding-decisions.md 106 verbatim.",
      "severity": "SHOULD",
      "proposed_change": "Item 14 keeps a sentence listing what stays on disk and that the app will no longer open it, or asks him once. Nothing is deleted without his word. The 'four old files' count is corrected."
    },
    {
      "id": "DA-8",
      "target": "DA-3 / Q131 default A (unplug) and R1",
      "claim": "The default for the destructive reading is the wrong way round for a store skeptic. Wiring on an effect lives in no store: the signal picker writes unfenced and with no undo step, so wiring built by hand exists only in the open show. Loading Default, or any look made without wires, unplugs it, and the only way back is a Cmd+Z taken before any other undo step. Boris's words are 'plugs them in again'. Unplugging is Harmony's extension, shipped by default and offered live on stage.",
      "evidence": "plan-looks-answers.md:147-150 (unwired entry and Default unplug), :584-590 (R1 concedes 'a picture that stops moving'), :633 and :72-75 (picker writes unfenced, not undoable). binding-decisions.md 102: 'loading it plugs them in again'.",
      "severity": "SHOULD",
      "proposed_change": "Default Q131 to B (a look only plugs in; Default restores values and leaves wiring). Make 'unplug' an explicit second item or a later answer. This is one enum plus LK-18's clause 4, which the plan already says is a small change."
    }
  ],
  "strongest_point": "DA-1 is the strongest. DA-9's third lookName writer re-derives the 'loaded look' from firstMatch on every refresh. For two looks with identical settings, Save over then names the earlier look instead of the one he loaded, and the plan has no undo. Neither the unit rows nor GL-10 test a tie, and R11 calls ties harmless. That is a silent, permanent loss of a look he built.",
  "citations_rechecked": true
}
```

---------------------------------------------------------------------------------------------------------
## SEAT: gates
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "GL-5 / GL-10 / LM-16 / LM-4 (Boris's 103 default scenario)",
      "claim": "Boris's one default behaviour (load Look 2, tweak, New Look -> Look 3, Look 2 stays unchanged) has no machine row. Only the Boris-eye check 8b covers it.",
      "evidence": "Boris (103, quoted at plan:24-25): \"look 2 remains unchanged\". GL-5 (plan:507) opens New Look on fx 1 with no loaded look and never reads a loaded look's file. GL-10 (plan:511) tests Save over only. LM-16 (plan:457) asserts the new file exists, not that the loaded look's bytes and mtime are unchanged. ST-15 (plan:433) is about replace only. DA-9 writer (2) (plan:249) says New Look sets `lookName` to the new look, and no row pins that. A mutant where New Look writes into the loaded look's file or leaves `lookName` stale can go unseen by any machine gate.",
      "severity": "MUST",
      "proposed_change": "Add an arm to GL-5 or GL-10: load \"Off\", tweak, pick New Look, accept. Require Off's bytes and mtime unchanged, the new file holding the tweak, `loaded` = the new name, and Save over then greyed. Add a matching mutant (New Look calls replace on `lookName`) and extend LM-16 to compare the loaded look's file bytes."
    },
    {
      "id": "GA-2",
      "target": "GL-9 (b), LC-11, DA-5 rule 4, section 6 items 3 and 6a (\"from the first frame\")",
      "claim": "The after-write hook is the plan's own ASSUMED piece (plan:198), and no live row can go RED if it is missing or broken.",
      "evidence": "GL-9's RED arms are MU-LA-12 and MU-LA-18 only (plan:510). The hook mutants MU-LA-14 and MU-LA-15 are armed only by LC-11 (plan:442), which runs with a stub hook, so it cannot show that `tickSlot` with a Context built outside the tick yields live values. GL-9 (b) does a separate HTTP GET \"at once\" against a 120 Hz message-thread timer (MainComponent.h:378) that may run first. A tick landing in the gap makes a hook-less build pass sometimes, and the row is flaky in the other direction. Section 6 items 3 and 6a still promise \"from the first frame\".",
      "severity": "SHOULD",
      "proposed_change": "Have the look_load response read the twins in the same message-thread job right after the fence, before any timer can run. Make that the GL-9 (b) bar, and add MU-LA-27 (hook removed) as a live RED arm. If HD-13's fallback is taken, strike \"from the first frame\" from section 6."
    },
    {
      "id": "GA-3",
      "target": "LC-12 / MU-LA-16 (RED arm cannot go red)",
      "claim": "LC-12 compares element addresses, but the stated mutant (\"apply assigns the whole vector\") keeps those addresses when written as a copy-assign. LC-12 then stays green and MU-LA-16 is not RED under LC-12.",
      "evidence": "plan:443 (LC-12: \"the address of every connection and twin is the same before and after\") and plan:472-473 (MU-LA-16). std::vector copy-assignment of a same-size vector assigns elements in place and does not reallocate. The mutant would instead fail LC-10, because ParamConnection copy-assign resets grip and state (plan C4, ParamConnection.h:158-166). Only a move or swap form changes addresses. LINT-EL-3 (plan:482-483) is a text grep, not a behavioural RED.",
      "severity": "SHOULD",
      "proposed_change": "Pin MU-LA-16 to the move/fresh-vector form (`paramConns = std::move(tmp)`) so the buffer changes. Or have LC-12 also assert `data()` identity and capacity across the three operations. State which row each mutant goes RED under."
    },
    {
      "id": "GA-4",
      "target": "DA-10 NEW SENTENCE (\"at every instant ... whole old or whole new\") / ST-13 / R7",
      "claim": "The absolute atomicity sentence, headed for the pitfall text, is not what the JUCE call guarantees. ST-13 cannot exercise the failing step.",
      "evidence": "R7 says macOS behaviour is NOT VERIFIED (plan:605-607). I read it: `replaceInternal` is `moveInternal` (juce_SharedCode_posix.h:434-437). `moveInternal` tries rename, and when rename fails it falls back to `copyInternal(dest)` and then deletes the source (same file, ~415-432). That fallback is a non-atomic overwrite of the target. DA-10 says \"A failure in 5 leaves the old file\" (plan:273-274), which is false on the fallback path, and step 6 only reports it. ST-13's two arms (a truncating writer, a read-only folder) fail in steps 2-4, never in 5 (plan:431).",
      "severity": "SHOULD",
      "proposed_change": "Reword the sentence to \"one rename on the same volume; a failed rename can leave a half file, which the read-back detects\". Or call `::rename` directly and fail on error. Add a test seam that makes step 5 fail and assert the old bytes, or keep a `.old` copy until step 6 passes."
    },
    {
      "id": "GA-5",
      "target": "Q131 \"What changes with each answer\" (plan:576) vs LC-9, GL-9 (c)(f), LM/V rows",
      "claim": "The claim that answer 131 B is \"one enum value\" plus LK-18/LK-19 plus section 6 item 6(d) leaves stale pins in rows that encode answer A.",
      "evidence": "LC-9 (plan:440) says execute \"unplugs the rest\". The GL-9 bar (plan:510) says \"Default unplugged 2 on fx 0\" and \"undo gave 2 connections back, then 0\". Under B, loading Ghost on fx 1 keeps Wired's LFO on speed (fixture: Ghost has no speed connection, plan:500-501), so (c)(d)(e) and the \"0\" change. DA-5 rule 3 (\"an unplugged slider rests at once\") and the Default row of the DA-3 table (plan:150) are also unlisted. The 131 B bullet names only 3 items.",
      "severity": "SHOULD",
      "proposed_change": "List every row whose bar or fixture changes under 131 B: LC-9, GL-9 (c)(e)(f) and the bar string, DA-3's Default row. Or write GL-9's bar from the table in DA-3 so one table drives both. Re-count so 131 B really is small."
    },
    {
      "id": "GA-6",
      "target": "SE / CE-1 / DA-4 engine change (scope of the test)",
      "claim": "The one-line change at the Signal case of `evaluate` reaches every connected target (clip, layer and macro scalars, source params, macros), but CE-1 tests only an effect slot.",
      "evidence": "plan:449 (CE-1) and plan:170-172. The same `evaluate` feeds macros (ConnectionEngine.cpp:328, `isnan -> manualValue`), the source-param twins (:373) and the scalar twins (:246). A lost-signal connection on any of them changes from \"range bottom\" to \"rests on its own value\". test_layer_strip_follows_model.cpp:122 and test_show_model.cpp:1351 use the unregistered name \"rms\", so unknown names exist in fixtures. R2 (plan:591-593) argues rarity only from FOS M-6 (user signals). No row, regression probe or test pins the other targets.",
      "severity": "SHOULD",
      "proposed_change": "Extend CE-1 (or add CE-3) to cover a layer scalar, a macro connection and a source param with an unknown signal: NaN, twin NaN, manual value shown. Or scope the change to effect slots by passing a flag, so the lane does not silently change other surfaces. Say so in section 6 item 6c."
    },
    {
      "id": "GA-7",
      "target": "GL-4 second arm (\"Wired\" made in GL-9 (a), pre-written here)",
      "claim": "The GL-4 second arm depends on a look the rig cannot supply as written, and its content is not pre-registered.",
      "evidence": "plan:506: \"Wired made in GL-9 (a), pre-written here\". The rig runs every row with a FRESH AUDIODNA_LOOKS_DIR (plan:487-489), so GL-9's Wired file is not in GL-4's folder. \"Pre-written\" is the only repair, and then the bytes (connections, ranges, which parameters) are unstated. A bar run against a look whose wiring is not stated (an LFO or Bass wire that may sit still) does not show that wired loads cost nothing. The line also reuses RU's decision table \"to each arm by itself\" without stating the thresholds.",
      "severity": "SHOULD",
      "proposed_change": "Write the Wired fixture in the plan: Bass 0.20..0.80 on intensity, LFO Sine 1 Beat on speed. Say it is hand-written into GL-4's fresh folder. Restate the hold, black and per_load thresholds for the wired arm in the row, before the run."
    },
    {
      "id": "GA-8",
      "target": "GL-5 accept route / LM-17 / J6 (greyed button ignores Return)",
      "claim": "The only machine check that a taken name cannot be saved goes through the test route's own handler, so it never exercises JUCE's key path. The plan cites the wrong JUCE mechanism for it.",
      "evidence": "plan:496: the route `accept` runs \"the handler the button itself calls; a greyed button does nothing\". That is the model's own check. J6 (plan:99-100) cites `Button::keyStateChangedCallback`. A Return in an AlertWindow takes a different path: `AlertWindow::keyPressed` calls `b->triggerClick()` with no enabled check (juce_AlertWindow.cpp:549-558), and `Button::handleCommandMessage` then skips the click only because `isEnabled()` is false (juce_Button.cpp:390-395). The outcome is probably the same, but no gate pins that a real Return on the greyed button leaves the box open. Boris item 13 covers only the red colour.",
      "severity": "NIT",
      "proposed_change": "Correct J6's citation to `AlertWindow::keyPressed` -> `triggerClick` -> `handleCommandMessage`. Add a Boris check: with a taken name, Return does nothing and the box stays open. Or have LM-17 call `keyPressed(Return)` on a real AlertWindow in the unit test (no synthetic OS input)."
    }
  ],
  "strongest_point": "GA-1. Boris's single stated default (103: look 2 stays unchanged when he saves a changed version as look 3) is left to a Boris-eye check, and no machine row compares the loaded look's bytes after New Look. The remaining gates mostly prove neighbouring things: GL-5 starts from an effect with no loaded look, and GL-10 covers Save over only.",
  "citations_rechecked": true
}
```

---------------------------------------------------------------------------------------------------------
## SEAT: stage-hands
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "DA-4 sleeping wire, stage SE, fixture Ghost, V-23, section 6 item 6(c), R2",
      "claim": "The scenario that justifies the engine change, a stage, a fixture and a visual state cannot happen in the app: Boris cannot make a user signal. His check 6(c) cannot be performed, and R2's claim that a lost-signal wire is rare is really 'never'.",
      "evidence": "grep -rn 'addSignal' src tests finds only the definition (src/signal/SignalRegistry.cpp:88) and the declaration (SignalRegistry.h:24). Nothing calls it. The registry's only non-fixed signals are built in (Mod 1, Mod 2, SignalRegistry.cpp:65,70). Plan 6(c) (lines 539-541): 'Load a look that names a signal you made yourself in another show ... make a signal with that name and it starts following'. Boris has no way to do either. facts-one-save M-6 says user signals have no serializer, but never says they can be created.",
      "severity": "SHOULD",
      "proposed_change": "Re-word 6(c) to what Boris can do, or drop it. Say in DA-4 and R2 that no in-app path makes a user signal, so the sleeping wire is only reachable by a hand-edited look file. Either demote SE to an optional guard or keep the one NaN line, but stop claiming a visible behaviour change in shows. Drop V-23 or mark it a fixture-only state."
    },
    {
      "id": "ST-2",
      "target": "DA-3 / Q131 default A: Default and any look without a wire unplug every signal",
      "claim": "Mid-set, one click on a look strips every wire Boris plugged in by hand, and the picture stops moving. Nothing on screen announces it, and the only recovery is Cmd+Z. His option 102 B only says loading 'plugs them in again'. Plugging in is all he asked for.",
      "evidence": "Boris's option as asked (boris-clarify-101-106.md:14): 'The look also remembers which signal drives each slider, and loading it plugs them in again'. DA-3 table rows 'entry without conn: unplugged' and 'Default: unplugged, every slider' (plan lines 147,150). DA-1 line 124-125: every look made by this build has hasSignals = true, so a look saved on an unwired effect always strips wires when loaded. R1 (line 584-590) names the harm and keeps A anyway. The plan's own reason for A is that the button must mean 'the effect is exactly this look'. That is a matching convenience, not Boris's words.",
      "severity": "SHOULD",
      "proposed_change": "Make B the default: loading unplugs nothing. hasSignals becomes 'at least one wired entry'. A look with no wired entry is a first-format look, so matches ignores wiring for it. This removes the 'signals' flag's false case and one table row. Ask Q131 with the A/B labels as written, and have Harmony state the stage risk to him in the question."
    },
    {
      "id": "ST-3",
      "target": "DA-9 writer (3): refresh() sets lookName whenever the derived button name is one of his looks",
      "claim": "Save over can silently retarget to the wrong look. Boris loads Look 2 and tweaks it. If any intermediate state, such as a slider dragged past a value, equals another look, the button derives 'Look 1' for one refresh. lookName becomes Look 1 and stays. The menu then offers Save over \"Look 1\" and Look 2 is no longer offered.",
      "evidence": "Plan DA-9 lines 249-251: '(3) the view's refresh(): whenever the derived name on the button is one of his looks, lookName is set to it'. The stated purpose is only the first show of an effect that came from a saved show. refresh() runs about 10 times a second (ruling-effect-looks.md:110). Look 1 and Look 2 differing in one slider is normal. Save over has no undo (DA-11), so one confirm click is the only guard.",
      "severity": "SHOULD",
      "proposed_change": "Restrict writer (3) to when lookName is empty or names no listed look. Add a row to LM-18 or LC-13: 'a transient match with another look does not move lookName'. Add the matching mutant."
    },
    {
      "id": "ST-4",
      "target": "GL-9 (b) and DA-5 rule 4: the after-write hook 'no frame on the base value'",
      "claim": "The live row cannot fail for the hook. After a load answers over HTTP, the regular 120 Hz tick has almost certainly already filled the twins, so '2 live values at once' passes with no hook at all. Its two listed RED arms (MU-LA-12, MU-LA-18) are about undo and the Ghost wire, not the hook. Only the unit row LC-11 can catch a removed hook, and live the one-frame black/base risk goes unmeasured.",
      "evidence": "Plan line 198 and the GL-9 row (line 510): 'GET looks fx 1 at once'; RED arms 'MU-LA-12 ... MU-LA-18'. MU-LA-14 and MU-LA-15 appear only under LC-11. The regular tick is the 120 Hz message-thread path (src/signal/SignalRegistry.cpp:168-169; MainComponent.cpp:4296-4298), well inside one HTTP round trip.",
      "severity": "SHOULD",
      "proposed_change": "Either have the look_load handler return the twins read inside the same message-thread callback, straight after the fence, so the read precedes any regular tick, and add a mutant for 'hook removed' with that row's RED arm. Or say plainly that the hook is unit-gated only, and drop 'at once' from the pass line."
    },
    {
      "id": "ST-5",
      "target": "DA-9 / Q133 default A and DA-8: the button after a tweak",
      "claim": "Boris loads Look 2, tweaks, and the small button reads 'Looks' (dim). It names no look, so nothing on screen says which look he is working from. The only place the name appears is inside the menu (Save over \"Look 2\"). This is the exact moment his answer 103 is about. After a Save over, every other effect still holding the old Look 2 values also drops to 'Looks'.",
      "evidence": "Plan lines 254-256 and 571-573 (Q133 A: 'reads Looks, dim -- it shows a name only while the effect is exactly that look'). Boris's words (BFB:654-659): 'I need to have a way to save look 2 if I tweak it a little'. His item 8b depends on seeing 'Look 2' to know what Save over will replace.",
      "severity": "SHOULD",
      "proposed_change": "Make Q133 B the default: the button keeps the loaded look's name with one quiet mark while the values differ. If the council keeps A, put the loaded look's name in the button tooltip so the state is visible without opening the menu. That is not an event text."
    },
    {
      "id": "ST-6",
      "target": "DA-4 sentence 'LFO, Timeline and Clip Position need nothing outside the connection', plus LK-16",
      "claim": "The plan's sentence is false. The engine passes no clip clock anywhere, so a captured Clip Position or Timeline wire evaluates at position 0.0 on every owner. A look carrying one loads a slider pinned at the curve's start. That is the half-plugged effect DA-4 claims to rule out. LK-16 round-trips these kinds as if they drive something.",
      "evidence": "src/connect/ConnectionEngine.cpp:337,345,355 pass nullptr as the clock for global, layer and clip effects. Lines 141 and 149 read 'clock ? clock->position() : 0.0f'. The code comment at :349-353 calls real per-clip playhead wiring 'Lane 5's job'. Plan lines 163-164 claim the opposite.",
      "severity": "SHOULD",
      "proposed_change": "Correct the sentence. Say Clip Position and Timeline wires are stored and replayed as the engine evaluates them today (position 0). Add this to Boris's check 6 so he knows what he will see. Either exclude those two kinds from capture, or keep them and give LK-16 no claim that they move."
    },
    {
      "id": "ST-7",
      "target": "DA-8 Save Over confirm: 'the Delete Look model'",
      "claim": "The Save Over window copies the Delete window's keys, so Return is the destructive button. Save over is irreversible and has no undo, so a reflex Return, or a Return meant for the New Look box, overwrites a look Boris made last week. The plan never says which button owns Return.",
      "evidence": "Plan line 240-243: buttons 'Save Over' and 'Cancel' (the Delete Look model, AM-6). The model it copies binds Return to the destructive button (src/MainComponent.cpp:6365-6368: addButton(\"Delete\", 1, returnKey), addButton(\"Cancel\", 0, escapeKey)). DA-11 (lines 279-285) admits there is no undo.",
      "severity": "SHOULD",
      "proposed_change": "In DA-8 say Esc cancels and Return does not confirm Save Over. Either bind Return to Cancel, or give Save Over no Return shortcut. Add the key binding to LM-18 so a test pins it."
    }
  ],
  "strongest_point": "Q131 default A: a VJ mid-set clicks Default or an older look and every signal he wired by hand is unplugged, which leaves the picture dead with no sign of why. Boris's chosen option only said loading 'plugs them in again'. The plan's own R1 admits the harm and keeps the default anyway.",
  "citations_rechecked": true
}
```

