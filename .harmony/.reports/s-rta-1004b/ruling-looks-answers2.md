# RULING looks-answers2 -- architect ruling on the blind council's attacks on plan-looks-answers2.md (lane "looks-answers2": the per-effect looks lane, SECOND DELTA on Boris's answers to questions 131-134 and 138; s-rta-1004b)

Author: architect (ruling; run on opus at max effort, as the dispatch states). Harmony decides after this; the ruling is her
working document.
Pins: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt` printed nothing
(HEAD is 8b464a6, docs only), so every code read below is a plain file whose content is that of 7bc6df7 (= the code of 185147b
that RA and RU cite). JUCE was read as source under build/_deps/juce-src/modules. The two stopped worktrees (lane/bf2,
lane/bf2-keys) were NOT opened: nothing of this lane comes from them. Nothing was built, run or launched. Nothing under
~/Library was read or listed.
Seat papers, verbatim and whole: .harmony/.reports/s-rta-1004b/attack-looks-answers2-papers.md (2 seats, 15 attacks, 19583
characters, a closed JSON array, parsed; the file's two blocks read back equal to the array).
Labels: VERIFIED = I read the line myself (at 7bc6df7, in JUCE's source, or in the record). INFERRED / ASSUMED are written
where used.
Short names: PL2 = .harmony/.reports/s-rta-1004b/plan-looks-answers2.md (PL2:n = its line n; P2-n its amendments; MU-P2-1..6
and LINT-P2-1..3 are its ids); RA = .harmony/.reports/s-rta-1004/ruling-looks-answers.md; RU = .../ruling-effect-looks.md;
OS = .../ruling-one-save.md; BD:n = .harmony/binding-decisions.md line n; C131 = .harmony/.reports/s-rta-1004/
boris-clarify-131-134.md; C135 = .harmony/.reports/s-rta-1004b/boris-clarify-135-143.md. Paths are under
/Users/boriskarpman/projects/RealTimeAudio.
NAME SPACES, kept apart: the seats' attacks are written "A:GA-n" and "A:ST-n" (a bare "ST-n" is a store test row, as in RU and
RA). This ruling's amendments are "RB-n", its facts "FB-n", its mutants MU-P2-7..15 (continuing the plan's), its decisions
HD-23.. (continuing RA's).
Precedence asked for: Boris's verbatim words > Harmony's adoption blocks (plan-effect-looks.md:619-669; plan-looks-answers.md:
642-700) > this ruling once adopted > PL2 > RA > RU > the plans. WHERE THIS RULING IS SILENT, PL2 STANDS WORD FOR WORD; where
PL2 is silent RA does, with the words of section 0 (1) laid over it; where RA is silent RU does, the same way.
Boris is quoted only verbatim, from binding-decisions.md and boris-feedback-backlog.md. His answers to this lane are in the
sections recorded 21:03:09 and 21:33:30 (BD:950-1041). TWO LATER SECTIONS were recorded while this ruling was being written
(22:27:57 and 22:32:45, BD:1043-1066); both were read before it was closed: they are about actions and the recording review
screen, name no preset, correct neither reading R88 nor R95, and change no rule here. What they mean for this lane: SF-10.
THE WORD: the thing is a "preset". "Look" appears only inside his quotes, inside the OLD half of an old -> new pair, in the ids
and titles of the earlier papers, and where the record panel's OTHER "look" is named.

---------------------------------------------------------------------------------------------------------
## 0 VERDICT
---------------------------------------------------------------------------------------------------------
NEEDS REVISION. The delta's core is sound and is kept: the rename's two tables are complete (checked line by line and, for
the unit-case names, by machine); the button keeps the name of the preset last put on the effect, with no mark; that name is
saved with the show. 16 amendments (section 3) override the plan body. 15 attacks ruled: 7 ACCEPT, 8 PARTIAL, 0 REJECT. The
one MUST (A:GA-1) is right about the plan's sentence and is closed with separate mutant builds. The two seats pulled apart on
a rename (A:ST-1: carry the new name; A:GA-6: pin the plan's rule) -- ruled once: the name is carried (RB-3). Seven errors of
the plan that no seat named were found on the way and are corrected (E1-E7, section 1).

RULED FIRST, four things.

(1) THE RENAME: THE COMPLETE OLD -> NEW TABLE, AND EVERY STORED NAME'S FATE.
Boris: "138 look is an effect preset. change the name look to preset to avoid further confusion."
WHAT THE LATER LINE REPLACES IN HIS OWN EARLIER ONES: the word, and nothing else. 84 -- Boris: "every effect has many looks
with specific parameter setups. these are saved with the app. always." (BD:894-895) -- and 101-104 (BD:925-932) keep their
rules; where they say "look" the app now says "preset". No other earlier rule of his is replaced by the five answers: 133
replaces RA's default for the button (not a word of his); 134 ends what Harmony had left alone on his disk.
PL2's tables P2-1 and P2-2 are CONFIRMED. Every line was re-read against RU and RA; three lines are added (marked +).

(1a) ON SCREEN (complete; a text that is not in this table is not shown by the lane)
| where | old (RU / RA) | new |
|---|---|---|
| the button: the effect carries a preset | that look's name | that preset's name |
| the button: no preset was ever put on the effect | "Default" at its defaults, else "Looks" (secondary colour) | "Default" (RB-1; question 150) |
| the button: the show names a preset this computer does not have | "Looks" (secondary colour) | "Presets" (secondary colour) |
| + the button under 56 px wide | the triangle alone | the triangle alone (stays) |
| the name the box offers | "Look N" ("Look 1", "Look 2", ...) | "Preset N" ("Preset 1", "Preset 2", ...) |
| the menu, top to bottom | "Default" / his looks / "New Look" / `Save over "<look>"` (or "Save over", greyed) / "Rename" / "Delete" | "Default" / his presets / "New Preset" / `Save over "<preset>"` (or "Save over", greyed) / "Rename" / "Delete" |
| + the Rename list and the Delete list | his looks' names, nothing else | his presets' names, nothing else |
| name box, new | title "New Look"; buttons "Save Look", "Cancel" | title "New Preset"; buttons "Save Preset", "Cancel" |
| name box, rename | title "Rename Look"; buttons "Rename", "Cancel" | title "Rename Preset"; buttons "Rename", "Cancel" |
| Save Over window | title "Save Over Look"; `Replace look "<name>" of <effect> with the settings on now? This cannot be undone.`; buttons "Save Over", "Cancel" | title "Save Over Preset"; `Replace preset "<name>" of <effect> with the settings on now? This cannot be undone.`; buttons stay |
| Delete confirm | title "Delete Look"; `Delete look "<name>" of <effect>? This cannot be undone.`; buttons "Delete", "Cancel" | title "Delete Preset"; `Delete preset "<name>" of <effect>? This cannot be undone.`; buttons stay |
| Edit menu | "Undo Load Look '<look>' on '<effect>'" (and its Redo) | "Undo Load Preset '<preset>' on '<effect>'" (and its Redo) |
| tooltips | none | none; none is added |
| names he may not give (`isLegalName`) | Default, Looks, New Look, Save over, Rename, Delete | Default, Presets, New Preset, Save over, Rename, Delete (any letter case) |
| the folder he is told about | Library / Audio-DNA / Looks | Library / Audio-DNA / Effect Presets |
No other text is shown: no message, no count, no badge (LINT-EL-6 stands).

(1b) DOCUMENTED
| where | old | new |
|---|---|---|
| docs/claude/effects.md, the lane's section | "Looks per effect" | "Effect presets" |
| CLAUDE.md, the capability paragraph | "looks per effect, kept by the app" | "effect presets (per effect, kept by the app for every show)" |
| CLAUDE.md, UI Patterns | "Looks button" | "Effect Presets button" |
| CLAUDE.md's pitfall index and docs/claude/pitfalls.md entry NN | RA:510-526 | PL2:410-431 with the three sentences of RB-16 |
| docs/claude/testing-eyes.md | the look routes; AUDIODNA_LOOKS_DIR | the fx_preset routes; AUDIODNA_EFFECT_PRESETS_DIR |
| docs/claude/architecture.md | EffectLook...; `lookName` | EffectPreset...; `EffectSlot::fxPresetName`; the show key "fxPreset" |
| .harmony/APP-INVENTORY.md | "the Looks surface" | "the Effect Presets surface" |
| his checks | RA section 6 | section 6 of THIS ruling |
| questions 131-134 and 138 | as asked | NOT re-worded: a question he was shown keeps its words in the record |

(1c) PRE-REGISTERED STRINGS THAT CARRY THE WORD, and where each is written out
- Unit cases: 46 test names hold the word and 2 more hold it only in their failure text (LK-13, CE-4): 48 rows of the 81.
  PL2 5.1 writes 47 of them out. I compared each with RU / RA by machine (FB-18): every one is the old name with the word
  swapped and nothing else, except ST-11 (it also carries the variable's new name). The 48th is LM-3. Three rows are re-cut
  HERE, for reasons that are not the rename (LM-3, ST-11, ST-13): section 5.2.
- Gate lines GL-2, GL-3, GL-5, GL-6, GL-7, GL-9 (both forms), GL-10 and their FAIL lines ("Off's bytes changed at New
  Preset"; `the item read Save over "Preset 1"`): PL2 5.5, confirmed. GL-8 and GL-11 are re-cut here; GL-12 is new here.
- Decision-table prose the plan left as "stands" (E6): RU:634 "the two looked effects" -> "the two effects a preset was
  loaded on"; RU:634 "a look load IS recorded as moves" -> "a preset load IS recorded as moves"; RU:635 "(the looks'
  values)" -> "(the presets' values)"; RU:615 and RA:695 "20 look_loads" -> "20 fx_preset_loads".
- Fixtures: "Look 1" -> "Preset 1" (ST-12: "Preset 1", "Preset 1 copy", "Preset 10"); every fixture file is
  `<name>.preset.json` with "format": "audio-dna-effect-preset".
- ui_text keys: neither ruling defines one that holds the word (GL-1 reads the existing file label). None is added.
- The 24 visual states: section 5.6.

(1d) STORED NAMES (each: keep or change, and why). Nothing has shipped; nothing of the lane is on his disk (the second half
is Harmony's statement in the dispatch; his disk was not read).
| stored name | RU / RA | ruled | reason |
|---|---|---|---|
| root folder | `~/Library/Audio-DNA/Looks/` | CHANGE -> `~/Library/Audio-DNA/Effect Presets/` | He is told this folder and copies it by hand: it is on-screen text in the Finder. Not "Presets": `~/Library/AudioDNA/Presets/` is the removed saver's folder (src/ui/PresetManager.cpp:449) and would sit one hyphen away. |
| per-effect folder | `folderNameFor(effect)` | KEEP | no word in it |
| file extension | `.look.json` | CHANGE -> `.preset.json` | he sees the files; the extension exists nowhere in the app today (FB-8); MilkDrop's files are `.milk` |
| Save over's new-file name | `.<name>.look.json.new` | CHANGE -> `.<name>.preset.json.new` | follows the extension; still starts with a dot, still does not end in the listed extension (ST-14) |
| "format" value | "audio-dna-look" | CHANGE -> "audio-dna-effect-preset" | documented; no reader for the old value: no such file exists |
| "version" (2), "signals", "effect", "dryWet", "params", "uniform", "label", "value", "conn", "dryWetConn" | as RA | KEEP | none holds the word |
| the first format (no "signals": values only) | readable | KEEP readable, under the new extension and format value | a behaviour, not a word (LK-17, GL-2 "Hand") |
| the test-mode variable | AUDIODNA_LOOKS_DIR | CHANGE -> AUDIODNA_EFFECT_PRESETS_DIR | documented |
| + the test-mode scratch folder | "a scratch folder under the temp directory" | KEEP the place; NEW: it is a new folder at every launch (RB-5) | a left-over preset there could put a name into another lane's saved show |
| settings.json | no key | none | the lane still uses no settings key |
| the show | no key | NEW optional key "fxPreset" on an effect's entry, written only when not empty (P2-7, RB-4) | bare "preset" is taken inside the same files ("presetPlaylist", src/model/Clip.cpp:154; "feedbackPreset", src/model/Layer.cpp:87) |
| test routes | /api/debug/looks, look_make, look_load, look_replace, look_ui | CHANGE -> /api/debug/fx_presets, fx_preset_make, fx_preset_load, fx_preset_replace, fx_preset_ui | documented; "fx_" because /api/load_milkdrop_preset exists (src/test/TestServer.cpp:172) |
| route fields | `looks`, `menu.newLook`; request field "look" | CHANGE -> `presets`, `menu.newPreset`; request field "preset" | documented |
| route fields | `matched`, `loaded`, `menu.saveOver`, `menu.rename`, `menu.delete`, `button`, `box`, `confirm`, `live`, `dryWetLive`, `conn`, `folder`, `root`, `dirReads`, `focusHome` | KEEP | none holds the word |
| `fx_preset_ui` actions | pick:<menu item text>, rename:<name>, delete:<name>, renamelist, deletelist, menu, expand, fold, dismiss, type:, accept, accept:, cancel, return | KEEP ("pick:New Preset" carries the menu's own text); + NEW action `browser:MilkDrop` (RB-12) | -- |
| the probe | .harmony/probe-effect-looks.sh | CHANGE -> .harmony/probe-effect-presets.sh | named in docs and in every gate row |
| row, mutant, lint and decision ids | LK-, ST-, LC-, LM-, CE-, GL-, V-, MU-EL-, MU-LA-, LINT-EL-, HD- | KEEP | never on screen; they tie the papers together |

(1e) IN CODE (the builder's, under P2-3's rule, confirmed): the thing is always an EFFECT preset. Files:
src/effects/EffectPreset.h, src/effects/EffectPresetStore.h / .cpp, src/core/EffectPresetCmd.h, src/ui/EffectPresetsButton.h,
src/ui/EffectPresetsMenu.h, tests/test_effect_preset.cpp, tests/test_effect_preset_store.cpp, tests/test_effect_preset_cmd.cpp,
tests/test_effect_presets_menu.cpp; Catch tags [fxpreset], [fxpresetstore], [fxpresetcmd], [fxpresetmenu]. Four identifiers
are ruled because a pre-registered string names them: `EffectSlot::fxPresetName`, `EffectPresetStore::presetsFor`,
`onPerformPreset` / PerformPreset, `effectPresetsDir`. No class, struct, file or namespace is named bare `Preset...`, and the
letters `PresetManager` come back in no new identifier (RB-10): that was the removed whole-chain saver.

(2) THE BUTTON AFTER A CHANGE (133: the name stays, no mark).
Boris: "133 no need for any of that. It can be called look 2 but no need to show that it was changed. Effects are usually
changed by the user." "Any of that" was BOTH letters as asked (C131:14-17): A "reads "Looks", dim -- it shows a name only
while the effect is exactly that look" and B "keeps reading "Look 2" with a mark that says it was changed".
THE RULE (RB-1). The button reads the first of these that applies:
  1. the effect CARRIES a preset (the one last loaded, made or saved over on it) and this computer's folder lists it:
     that preset's name, normal colour;
  2. else its settings equal a preset (Default, or one of his): that name, normal colour;
  3. else it carries no name: "Default", normal colour;
  4. else (it carries a name that no preset here lists): "Presets", dim.
What he asked, answered one by one:
- AFTER A SLIDER MOVES: nothing on the button changes -- the same name, the same colour, the same triangle, however far the
  sliders go. No distance ends it. An effect that was never given a preset keeps reading "Default" the same way (A:ST-2;
  the plan had "Presets", dim, here: that is now question 150's B).
- AFTER A SHOW IS RE-OPENED: the same name. The carried name is saved with the show (the optional key "fxPreset", P2-7).
  On a computer whose folder does not hold that preset: "Presets", dim; with the folder copied across: the name again.
- AFTER "DEFAULT": the button reads "Default" and the carried name is emptied (LC-13). It keeps reading "Default" when a
  slider then moves.
- AFTER ANOTHER PRESET IS LOADED: that preset's name. After New Preset: the new preset's name. After Save over: the same
  name -- the button gives no sign that it saved; the menu item does (RB-6).
- WHAT THE MENU'S "SAVE OVER" NAMES: the preset the effect carries, and nothing else -- the name the button shows in case 1:
  `Save over "<that name>"`, live when the settings differ from it and this build may rewrite it. In cases 2, 3 and 4 the
  item is "Save over", bare and greyed. The one tick in the menu is the item the button names (case 3: "Default"; case 4:
  nothing). Clicking the ticked preset after a change loads it again: one undo step.
- A RENAME OR A DELETE: every effect in the OPEN show that carried the preset reads the new name -- or, after a delete,
  carries none and reads "Default" (or the preset its settings equal, if any) (RB-3; the plan left the changed ones
  reading "Presets", dim).

(3) WHERE AN EFFECT'S PRESETS AND MILKDROP'S PRESETS MEET ON SCREEN. P2-4's table is confirmed place by place (FB-14):
| the two together | when | ambiguous? | ruled |
|---|---|---|---|
| The browser's MilkDrop tab -- "Search presets...", "No presets loaded." with its four lines of help, "No recently used presets.", "Select presets, drag to cell", and "<n> presets" in the small picture under the pointer while a section or a selection is dragged -- beside an effect's button, menu ("New Preset", `Save over "..."`) and windows in the inspector | whenever the browser shows that tab and the inspector shows a clip, a layer or the composition with an effect | Each is a bare "preset(s)", and each sits in its own frame: the tab named "MilkDrop", and the header row of one named effect. The two destroying windows name the effect in their sentence. The name box names no effect, but opens only from that effect's own menu, under his click. | No text of this lane changes. MilkDrop's own words are a product choice: question 151 (default: leave them). State V-24 puts both in one picture for the critics. |
| a MilkDrop clip that carries clip effects | his clip inspector on such a clip | No: the button is on the effect's own row beside the effect's name; the MilkDrop preset is the clip's name ("MilkDrop Playlist (n)", or the file's name), never a control in that row. | nothing |
| the layer inspector: the feedback combo above the layer's effect stack | every layer inspector | No: the combo shows "Custom" or a feedback preset's own name; the word appears only in its tooltip, qualified ("Feedback preset"). | nothing; V-11's frame includes the combo |
| Preferences: "MilkDrop Presets:", "Path to .milk preset folder...", "Select MilkDrop Preset Directory" | the Preferences window | No: qualified, or inside the MilkDrop row. | nothing |
| the old "Save preset..." / "Load preset..." file windows and "Legacy Preset" | never, once one-save S7 is in: both functions are named in its removal (E1) | They WOULD be: a bare "preset" that means a whole effect chain. | LINT-P2-2 on the lane's base (RB-10). |
| the Edit menu's "Undo Load Preset '<preset>' on '<effect>'" | after any load | No: it names the effect; no other undo text holds the word. | nothing |
THE RULE FOR EVERY LATER TEXT (into docs/claude/effects.md, S2): "On screen the bare word 'preset' belongs to an effect, and
only on that effect's own row, menu and windows. Every other preset is written with its first word: MilkDrop preset,
feedback preset."
NOT RENAMED, and said so: the record panel's "holding the last look", "The look stays as it is." and "the look from when
Record was pressed is restored first." (FB-15). There "look" is the picture a take leaves; "holding the last preset" would be
false. His sentence is about the effect preset (INFERRED from the words before it: "look is an effect preset"). Handed to
the recording lane (section 9, SF-3) and told to him (check 20).

(4) 131, 132, 134 AND 138: NO RULE MOVES.
- 131 is READ as A (Harmony's reading R88, C135:46-49, told to him and not corrected -- INFERRED consent). Boris: "If this is
  a clip in the show, then ctrl-z brings it back. There is no other way." `kUnwiredEntry` stays Unplug; one Cmd+Z puts values
  AND signals back; nothing else does. `UnwiredEntry::Keep` stays built and unit-tested (LK-24) as the ONE constant, reachable
  from no screen; GL-9's second line stays pre-registered and DORMANT. If he corrects R88: that one line, and GL-9's dormant
  line becomes the live one.
- 132 -- Boris: "132 default good": the Save Over window as RA-2 rules it. 134 -- Boris: "134 delete them too": done by
  Harmony 2026-10-04 21:01:09; no code (PL2 LB4 confirmed).
- 138's letters were not picked: READ as A (reading R95, C135:80-82, INFERRED consent): a preset is picked from the button of
  an effect that is in the show; none is listed in, or dragged from, the effect list. WHAT B WOULD COST, in one paragraph,
  nothing designed: the FX list would have to read the store for every effect it shows (today a folder is read the first
  time that one effect's presets are asked for: ST-9's "once" and `dirReads` are re-cut); rows that open and close under an
  effect in that list; a new drag description beside "fx:" (made at src/ui/FXBrowser.cpp:239) that six accept sites learn
  (src/ui/EffectStackView.cpp:470, ClipCell.cpp:519, ClipInspector.cpp:1525, LayerInspector.cpp:1023,
  CompositionInspector.cpp:583, LayerStrip.cpp:1143); "add the effect AND load the preset" as ONE undo step, a new command;
  the list following New Preset, Rename, Delete and Save over; its own unit rows, live rows and visual states. About one more
  build stage and one more visual round. If he corrects R95 the lane as ruled here still ships first.

THE REST, IN ONE PARAGRAPH. The name in the show stays (P2-7), with its "byte for byte" claim cut to what is true and a live
arm for the case the seat found (RB-4). The app's own Rename and Delete no longer leave false names behind in the open show
(RB-3). "No mark" is pinned where it is painted, and by pixels at the visual gate (RB-2). Save over's outcome is read from
the menu item, and the store's failure path gets the missing list clause (RB-6). Undo across a New Preset or a Save over is
pinned and told to him (RB-7). The live mutant builds are split so that every FAIL line has one possible cause (RB-8). GL-8
compares names, not a number (RB-9). Four lints are re-worded so that correct code can pass them (RB-10). Stages stay S1a,
S1b, SE, S2, S3, VG; 87 new unit cases (PL2's 83 + 4); twelve live rows (PL2's eleven + GL-12); 24 visual states (four re-cut);
two questions for him (150, 151), each with a default; nothing waits.

HARMONY'S CONSTRAINTS, AND WHERE EACH IS MET
- Harmony constraint (looks answers 2): a DELTA that names every line it replaces and adds no feature. -> PL2:762-782 names
  the lines; section 3 ends with the lines this ruling adds to that list. What is new beyond words is each tied to his 133:
  the name in the show (RA's own HD-14, "taken up at once"), and the carry of RB-3 (section 10, R1 argues it).
- Harmony constraint (the rename): the complete list, old and new side by side; every stored name ruled. -> (1a)-(1e).
- Harmony constraint (the word is taken): MilkDrop's presets found in every place; `PresetManager` does not come back. ->
  (3), FB-14; RB-10 (LINT-P2-2, LINT-P2-3).
- Harmony constraint: 131 READ as A, B the one constant; 138 READ as A, B costed and not designed. -> (4).
- No text announces an event or a failure. -> LINT-EL-6 stands; RB-6 uses a STATE (a menu item's own enabled state).
- Everything he will see goes through the visual gate first. -> section 5.6 names the 24 states.

---------------------------------------------------------------------------------------------------------
## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
---------------------------------------------------------------------------------------------------------
The record
- FB-1 His five answers: BD:988-996 (131-134) and BD:1014-1016 (138). As asked: 131-134 at C131:5-21, his lines C131:41-44;
  138 at C135:15-20, his line C135:72. Harmony's readings R88 (C135:46-49) and R95 (C135:80-82) were told to him; no later
  line of his corrects either. VERIFIED.
- FB-2 Question 133 as asked had two letters (C131:14-17). His answer takes neither and declines both ("no need for any of
  that"). A's text is "it shows a name only while the effect is exactly that look". The plan's rule for an effect that was
  never given a preset (PL2:265-268: "Default", then "Presets", dim, once a slider moves) is that same behaviour. VERIFIED.
  A:ST-2's reading of his words holds.
- FB-3 Earlier words that still bind (BD:926-932): 102 B; 103; 104 -- Boris: "open a name box but with default name look x
  that can easily be changed". 138 replaces the word in them and nothing else. One Save (BD:874) -- Boris: "All of these
  things should be saved when a show is saved. There's no reason to save them separately:" -- said of his list of nine saves;
  it is used for P2-7 only as a pointer (INFERRED there).
- FB-4 Harmony's adoption blocks, read whole (plan-effect-looks.md:619-669; plan-looks-answers.md:642-700): HD-8 "default:
  keep the fence; tell Boris the number if GL-4 reads above 2 held frames"; HD-10 "a row is RED only when its FAIL line names
  the check its own mutant predicts"; the block of 21:33:30: "The WHOLE lane waits for the architect delta" and "(1) may
  change a stored name". .harmony/RIG-RULES.md section A: "critics get Boris's binding words verbatim AND the list of
  pre-existing texts that are outside the lane". VERIFIED.
The one-save lane
- FB-5 OS A-14 (OS:469-474): "S1 -> MERGE M1 -> S7 -> S2 -> S3 -> S4a -> S4b -> S5 -> VG -> MERGE M2" and "S7 and the deck
  removals change what he sees: they wait for the visual gate." (also H-1, OS:841). So S7 reaches main only with M2: after
  every other one-save stage and that lane's visual gate. VERIFIED as text.
- FB-6 OS A-11 (OS:422-431) names what S7 removes -- "`savePreset`, `loadPreset`, `fastSave`, `loadSlotPreset`,
  `populateSlotMenu`, `getFastSaveDir`, `PresetSlot`, `presetSlots_`, `fastSaveCounter_`; src/ui/PresetManager.h/.cpp WHOLE"
  -- and says "Comments that name the class (six files) are re-worded." LINT-6 (OS:640-641): "src names no `fastSave`,
  `presetSlots_`, `FX Save`, and no token `PresetManager`" (a token = not preceded by a letter). VERIFIED as text.
  My own grep at 7bc6df7 (`grep -rnw PresetManager src`): the class's two files; code at src/MainComponent.h:15 and
  src/MainComponent.cpp:387-388, :2969-3009, :4386-4424; and COMMENTS in SEVEN files, not six: src/ui/CompDecksBrowser.h:38
  and :45, src/ui/CompDecksBrowser.cpp:304, src/test/TestServer.cpp:1232, src/render/Renderer.h:627,
  src/connect/ConnSerialization.h:18-19, src/connect/ConnSerialization.cpp:5, src/effects/EffectChain.h:132.
  `MainComponent::savePreset` starts at :2965 and `loadPreset` at :2993 (the next function starts at :3051); the three old
  strings (:2968, :2996, :3035-3036) are inside them. VERIFIED.
- FB-7 OS:297: "an optional top-level key read under `hasProperty` does not bump the version." No row of OS pins the set of
  keys an effect's entry may hold (grep of OS for "keys", "byte" and "checksum": the checksums are of HIS files, read-only).
  VERIFIED as text; that the rule also covers a key inside an effect's entry is INFERRED.
The code
- FB-8 Nothing of the lane exists: `grep -rnE '\.preset\.json|Effect Presets|fxpreset|fxPreset|EffectPreset' src tests
  docs/claude CLAUDE.md` prints nothing, and the plan's own old-name pattern (PL2:534-535) prints nothing today. VERIFIED.
- FB-9 An effect slot is written and read in three places. The readers guard an optional key in TWO lines --
  `if (fxObj->hasProperty("dryWetConn"))` and, under it, the read (src/model/Clip.cpp:305-306, src/model/Layer.cpp:248-249,
  src/model/Composition.h:1096-1097); "dryWet" the same (Clip.cpp:287-288). A text key is read in ONE line with no guard
  (Clip.cpp:284: `getProperty("name").toString().toStdString()`). A writer puts an optional key under an `if`
  (Clip.cpp:122-125). VERIFIED. A:GA-2's citation holds: in the files' guarded idiom the key's literal sits on three lines
  per file, not two.
- FB-10 `Clip::EffectSlot` (src/model/Clip.h:60-114) has no name member but `effectName`. `Composition::forEachClip`
  (src/model/Composition.h:384-400) visits every clip of every live deck and then of every retired deck; a layer's effects
  are `layerEffects` (src/model/Layer.h:310), the global ones `globalEffects` (Composition.h:69). The three slot writers are
  reached through `saveToFile`, called in src only at src/MainComponent.cpp:3551, :3574 and :6694 (a deck writer at :3814
  goes with one-save S4b); WHICH THREAD those run on was not read. A take stores its state through its own serializer
  (src/recording/PerfState.cpp:54-151), not through the slot writers (INFERRED from a grep of `toVar` under src/recording).
  VERIFIED for the lines named.
- FB-11 JUCE reads a std::string as UTF-8 (juce_core/text/juce_String.cpp:386 with :156-161) and writes one as UTF-8
  (:2114-2117): a preset name with any letter survives the show's round trip. VERIFIED as source.
- FB-12 The letters "look" in today's tree (`grep -rhoiE '\w*look\w*' src tests`, counted): AudioDNALookAndFeel 441,
  LookAndFeel 129, lookup 77, setLookAndFeel 23, looking 19 -- A:GA-3's numbers exactly. src/ui/EffectStackView.cpp takes its
  colours from `AudioDNALookAndFeel` (:43-61, :295). A new header that paints in the app's colours holds the letters. VERIFIED.
- FB-13 tests/CMakeLists.txt calls `catch_discover_tests` 131 times, a few with TEST_PREFIX (:877, :3516): `ctest -N` prints
  every TEST_CASE by its name. VERIFIED. A:GA-4's citation holds.
- FB-14 "preset" ON SCREEN today, every place (a grep of every string literal in src that holds the letters: 61 hits, each
  read). MilkDrop -- src/ui/MilkDropBrowser.cpp:20-24 ("No presets loaded." and four lines on how to add "MilkDrop presets"),
  :101 and :122 ("<n> presets", drawn into a 120 x 24 picture for a drag: :93-101, :117-122), :283 ("No recently used
  presets."), :456 ("Search presets..."); src/ui/MilkDropBrowser.h:120 ("Select presets, drag to cell");
  src/ui/PreferencesDialog.h:74 ("MilkDrop Presets:"), src/ui/PreferencesDialog.cpp:96 and :106. Feedback --
  src/ui/LayerInspector.cpp:408-415 (a combo of names; tooltip "Feedback preset"). The old Save / Load --
  src/MainComponent.cpp:2968, :2996, :3035-3036. NOT the word: a MilkDrop playlist clip is named "MilkDrop Playlist (n)"
  (MainComponent.cpp:1311) and the file label reads "MilkDrop: <name>" (:1737). The MilkDrop list is one tab of the browser
  (src/ui/BrowserPanel.h:56; one tab's content is visible at a time, src/ui/BrowserPanel.cpp:149-154); the effect list is the
  tab "FX" (BrowserPanel.h:48). VERIFIED. PL2's E12 and E13 hold.
- FB-15 "look" ON SCREEN today in another meaning: src/ui/RecordPanelModel.h:136, :244, :252, :263 ("holding the last look"),
  :189 ("The look stays as it is."); src/MainComponent.cpp:5983, :6051. VERIFIED.
- FB-16 A menu opened with a parent component is a CHILD of that component and is drawn inside the main window
  (juce_gui_basics/menus/juce_PopupMenu.cpp:361-364); only without one is it a desktop window of its own (:378).
  docs/claude/pitfalls.md:123 calls "a PopupMenu with `withParentComponent`" an in-peer overlay, and LINT-EL-2 makes the
  lane's menu one. VERIFIED. A:GA-8's premise ("its own native window") is wrong for this app. The name box, the Save Over
  window and the Delete confirm ARE windows of their own (`juce::AlertWindow`).
- FB-17 No route shows a browser tab: `BrowserPanel::setActiveTab` (src/ui/BrowserPanel.h:40) is called only by the tab
  buttons (BrowserPanel.cpp:14). VERIFIED (grep over src). So V-24 as the plan writes it has no way to reach its state.
- FB-18 The row tables, by machine: RU:479-545 and RA:544-603 give 81 rows (LK 24, ST 17, LC 13, LM 23, CE 4); 46 names
  hold the word; PL2:446-508 restates 50 rows, none still holds it, and all but LM-3 and ST-11 equal a pure word swap.
  VERIFIED (a script over the three papers; nothing of the tree was run).
- FB-19 A slider move is not an undo step: the row's slider calls `onParamChanged` (src/ui/EffectStackView.cpp:405-410);
  `onPerformEdit` is called only for Bypass / Enable (:316-318), Remove (:341-343) and Add (:541-546). VERIFIED. A:ST-4's
  sequence holds: after load, tweak, Save over, the newest undo step is the LOAD.
- FB-20 The fence: docs/claude/pitfalls.md:119 -- a fenced frame "re-presents `canvasFBO_` untouched -- no clear, no
  composite, no capture answered, no recorder / Syphon frame -- counted in `fence_hold_frames`"; src/render/Renderer.cpp:
  408-419 ("the fence lasts 1-2 frames"); the counters are served at src/api/ApiServer.cpp:1550-1551. VERIFIED. A:ST-5's
  evidence holds: "one frame" is not established, and a recording and Syphon get no frame while a fence holds.
- FB-21 `EffectStackView::refresh()` (src/ui/EffectStackView.cpp:188-219) has no preset button today. Rows are built in
  `rebuildRows` (:256), called from `setEffects` (:134), after a remove (:337) and after a drop (:535). RA-1's writer (3)
  runs there: at every re-point of the inspector, not once per run. VERIFIED.
- FB-22 The rulings' own rules this ruling leans on, read: `firstMatch` tries Default first (RU:258); "A pick that would
  change no value pushes no undo step" (RU:311); making, renaming and deleting are not undo steps (RU:322); the button's
  `setState` "compares what it paints" (RU:328) and the route's `button` = {text, dim, mode} (RU:363), `mode` being the
  under-56-px form (RU:327); RA-1 (RA:257-276); RA-3's "the button is the state" and RA-9's last line (RA:304-305, RA:388);
  ST-13 (RA:564); GL-8 (RU:611, RA:677); GL-4's two tables (RU:614-624, RA:693-704). VERIFIED.
- FB-23 POST /api/debug/save_composition exists (src/api/ApiServer.cpp:328; handler :2178); one-save S1 changes its body
  (OS:513). VERIFIED as text.
- FB-24 The fixture effects' defaults (src/effects/EffectLibrary.cpp): Invert amount 0.70 (:154-156) -- the very value show B
  gives its clip's Invert (RU:590); Ripple intensity 0.40, freq 0.50, speed 0.50 (:40-44); Hue Shift amount 0.30 (:65-67).
  `firstMatch` tries Default first (RU:258) and writer (3) takes only one of HIS presets (RA:264). So an arm that wants
  writer (3) to act must first move the effect off its defaults (GL-11 step e). POST /api/set_param exists
  (src/api/ApiServer.cpp:187). VERIFIED.

Errors of the plan that no seat named (each ruled in section 3)
- E1 PL2:117-118 leaves NOT VERIFIED whether S7's functions include `savePreset()` and `loadPreset()`. Both are named (FB-6).
- E2 PL2:388-389 and :392 start the lane at "one-save S7 merged". By FB-5 that is one-save's M2: the whole lane, not one
  stage. The fence's paragraph on a still-open one-save stage (PL2:400-404) is moot on that base (RB-15).
- E3 PL2 R6 (PL2:725-727) leaves the show's version NOT VERIFIED. OS:297 settles it: no raise (FB-7).
- E4 PL2:615-617 (his check 5) drops RA's sentence "(A slider you moved or a signal you plugged on the SAME effect after
  the look goes back too.)" (RA:738-739). It is restored: without it A:ST-4's surprise is twice as large.
- E5 PL2's table of replaced lines (PL2:762-782) misses three sentences that 133 makes false: RA:304-305 ("the button keeps
  reading "Looks" ... the button is the state"), RA:388 ("A failed Save over shows nothing; the button keeps reading
  "Looks" (the state)") and the reason in RU:227 ("its NOT changing is the only sign a write failed"). RB-6.
- E6 PL2:557 says GL-6's "table stands"; its prose holds the old word in three places (section 0 (1c)).
- E7 PL2:497-498 says that under MU-EL-24 LM-12's button "reads "Presets"". Under RB-1 it reads "Default" (no name, no
  match). The row is RED either way; the note is corrected (5.2).

---------------------------------------------------------------------------------------------------------
## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it; RB-n = the amendment in section 3)
---------------------------------------------------------------------------------------------------------
| id | sev | verdict | decided by | what changes |
|---|---|---|---|---|
| A:GA-1 | MUST | ACCEPT | PL2:570-571: GL-11 step (a) asserts `button` = {"Off", dim false}; MU-P2-1 (PL2:513) makes that read "Presets", dim. So under the build M-H GL-11 does NOT fail "only on the line loaded none after a relaunch" (PL2:517-519): the sentence is false. (The seat's "never shown" overstates: by HD-10 the probe prints every failed check, so the relaunch line would still print.) | RB-8: the "cannot mask" sentence is struck; MU-P2-1 gets a build to itself; in every shared build each row's FAIL line has ONE mutant that can cause it, and the ruling says why. |
| A:GA-2 | SHOULD | ACCEPT | FB-9: the files' own guarded read is two lines, so "exactly six lines" fails correct code. | RB-10: LINT-EL-7 counts by operation and by file, never by line. S2's cell says "one write and one read of the key". |
| A:GA-3 | SHOULD | ACCEPT | FB-12: `grep -il look` cannot print nothing for a header that uses the app's colours. | RB-10: LINT-P2-1 is a per-file count after two allowed families are removed. R7's worry (a coined `lookStore`) is caught by the same count. |
| A:GA-4 | SHOULD | ACCEPT | RU:611, RA:677, PL2:559: the bar is one number, and its RED arm ("the lane's base: N0 is not N0 + 83") is true of every tree. FB-13: the names are there to compare. FB-5: the order's "[one-save S1 merged]" is vacuous. | RB-9: GL-8 compares the SET of names; its RED arm is a doctored listing; the baseline is re-taken at every re-base. RB-15 strikes the bracket. |
| A:GA-5 | SHOULD | ACCEPT | PL2:254-256 (writer 3) with FB-21 and PL2:279-281: a row built over an effect that equals one of his presets takes its name, and the next Save writes it. "Byte for byte" is false for that show. GL-11 (d) passes only because its fixture equals nothing. | RB-4: the claim is cut to what is true; GL-11 (d) builds the row, (e) is new and has its own mutant. RB-5 closes the half that could reach another lane's probes. |
| A:GA-6 | SHOULD | PARTIAL | PL2:665-667 tells him a behaviour no row pins: right. But that behaviour is re-ruled (A:ST-1, FB-2): the row must pin what is ruled, not what the plan said. Check 14's claim has unit parts (LM-24, LC-14) and no end-to-end row: right. | LM-25 is added and pins RB-3; with it LC-16 and the live row GL-12. GL-11 (f) is the folder-absent, folder-back arm. Rejected: LM-25 as the seat words it ("row C, changed, reads Presets, dim"). |
| A:GA-7 | SHOULD | PARTIAL | PL2:245-246 pins "the text, the colour and the triangle", and LM-3 (PL2:505) tests a pure function of two fields; RU:328: the button repaints on what it PAINTS. | RB-2: LM-3 is asserted on the built row's painted key; MU-P2-8; V-23 is compared with V-2 by pixels, by Harmony. Rejected: keeping his words from the critics -- the rig rule gives them his binding words verbatim (FB-4). What they are no longer given is the plan's gloss ("this sameness is his answer"). |
| A:GA-8 | NIT | PARTIAL | FB-16: the premise is wrong -- the menu is drawn inside the main window, so ONE window-id capture holds the tab, the inspector and the menu. FB-17: but nothing can show the MilkDrop tab without a click. | RB-12: V-24 is one capture of the main window; `fx_preset_ui` gains `browser:MilkDrop`; a stated fall-back if the menu does not stay open in the background rig. |
| A:ST-1 | SHOULD | PARTIAL | PL2:475 ("a freed number is used again") with PL2:263-264, :308-310 (the other carriers keep the old name) and P2-7 (it is saved): the path is real, and by his 104 (FB-3) changing a default name is the EXPECTED act. | RB-3: a Rename and a Delete reach every carrier in the open show, so none is left holding a freed name. Not taken: making `nextName` skip carried names (unneeded once RB-3 holds; ST-2 stands as PL2 5.1 writes it); the plan's question 152 (withdrawn, section 7). What is left -- shows that were not open, copies in the undo history, a name re-made by hand -- is named (R4); its full closure is HD-24. |
| A:ST-2 | SHOULD | ACCEPT | FB-2: the plan's default for an effect that never had a preset IS the behaviour of 133's letter A, which he declined in so many words. | RB-1: case 3; question 150 with its letters swapped; LM-3; V-3; check 1; MU-P2-7. |
| A:ST-3 | SHOULD | PARTIAL | PL2:245-247 with RA:388 (E5): under 133 the button reads the same before a Save over, after it, and after one that failed. The state that remains is the menu item (LM-18). And ST-13's failure arm does not assert the LIST (RA:564): a store that updates its list before it writes would grey the item over a preset that was never saved. | RB-6: ST-13 gains the list clause; LM-26 is new; MU-P2-12; checks 10 and 15 tell him where to look. Rejected: a live arm with a locked folder -- the view calls the store itself, so there is no app wiring that a unit row cannot see (RU's test for what earns a live row, RU:241-242). |
| A:ST-4 | SHOULD | PARTIAL | FB-19 with RU:322: after load, tweak, Save over, Cmd+Z takes back the LOAD; the file stays. | RB-7: LC-15 and MU-P2-11 pin it; one lint clause; check 10's sentence; E4's sentence is restored. Rejected: an undo marker that swallows one Cmd+Z -- a keypress that does nothing is a new behaviour, and nothing is lost without it: what he saved is in the preset. |
| A:ST-5 | SHOULD | PARTIAL | FB-20; and HD-8 as adopted already says "tell Boris the number" (FB-4). | RB-13: check 4 carries the measured number and names recording and Syphon; HD-8 is decided before S3's packet when either arm reads above 40. Rejected: re-cutting the table's rows -- they are adopted, they are a measurement and not a pass bar, and none of his five answers touches them. |
| A:ST-6 | NIT | ACCEPT | PL2:247-249 with RA-10 (Unplug): a click on the ticked preset is a full load. | RB-14: checks 6 and 7 (c) say it. |
| A:ST-7 | NIT | PARTIAL | FB-6: OS A-11 orders the comments re-worded and LINT-6 forbids the token anywhere in src, so S7 cannot pass its own lint with one comment left: "a scrub S7 has not been told to do" is wrong. Right in one detail: A-11 counts six comment files and there are seven. | RB-10: LINT-P2-2 uses OS's own token test, so a green S7 is a green fence by construction. SF-1 hands the seven files and their lines to the one-save lane. Rejected: loosening the lint to code tokens. |
Reconciled conflicts. A:ST-1 (carry the new name everywhere) against A:GA-6 (pin "the ones you changed read Presets"): one
rule, RB-3, and A:GA-6's row pins it. A:ST-2 against the plan's F-LB2-c: his own words decide (FB-2); the plan's reading
becomes question 150's B. A:GA-7 ("do not tell the critics") against the rig rule: the critics keep his words and lose the
gloss; the proof of sameness moves to a pixel comparison. A:ST-3 and A:GA-5 / A:GA-6 each ask for a live arm: two are
taken (GL-11 e and f, GL-12: wiring and files a unit row cannot see) and one is refused (a locked folder: no wiring).

---------------------------------------------------------------------------------------------------------
## 3 AMENDMENTS (numbered RB-n; each OVERRIDES the plan looks-answers2 where they differ; "P2-n" is the plan's own item)
---------------------------------------------------------------------------------------------------------
RB-1 THE BUTTON'S RULE HAS FOUR CASES (A:ST-2). Replaces P2-6's cases (PL2:241-244), its paragraph "AN EFFECT THAT WAS NEVER
GIVEN A PRESET" (PL2:265-268) and F-LB2-c (PL2:301-304).
- `fxpreset::buttonState(def, slot, presets) -> {text, dim}`, pure, in src/ui/EffectPresetsMenu.h. The first that applies:
  (1) the slot has a loaded preset (a preset of this effect is listed under EXACTLY `fxPresetName`): its name, not dim;
  (2) else `firstMatch` names something (Default, or one of his presets): that name, not dim;
  (3) else `fxPresetName` is empty: "Default", not dim;
  (4) else: "Presets", dim.
- Dim, and the bare word "Presets", are case (4) only: a name that no preset of this effect lists on this computer.
- The menu's one tick is the item the button names: case (3) ticks "Default"; case (4) ticks nothing.
- Why case (3). His 133 declined, in so many words, a button that "shows a name only while the effect is exactly that"
  (FB-2); the plan kept exactly that for the effect that never had a preset -- the commonest effect there is, since he has
  no presets yet (101 -- Boris: "I will build them later myself, but when the app is finished", BD:925). His reason --
  Boris: "Effects are usually changed by the user." -- does not stop at effects that were given a preset.
- Nothing stores the word "Default": loading "Default" still empties the name (LC-13 stands), and an effect in a show saved
  before the lane has no name and reads "Default". No migration, no new writer.
- Question 150 asks it, with this as the default A. His B (the plan's rule) removes case (3): one line; LM-3's dormant
  form; V-3; check 1; GL-12's dormant last clause (section 7).
- The rest of P2-6 stands: `shownMatch` for the route's `matched` only; the writers (writer (2) as RB-3 changes it); what
  the menu offers after a change; "may an effect far from the preset's values carry its name for ever? Yes."
Rows: LM-3 (re-cut), LM-24, GL-10, GL-11. Mutants MU-P2-1, MU-P2-2, MU-P2-5, MU-P2-7.

RB-2 "NO MARK" IS PINNED WHERE IT IS PAINTED (A:GA-7).
- `EffectPresetsButton` paints from ONE value, its painted key {text, dim, mode}; `mode` is the under-56-px form and depends
  on the button's width alone (RU:327). The row hands the button `buttonState(...)` and nothing else of the slot.
  `EffectPresetsButton::paintedKey()` returns what `setState` compares (RU:328's rule; LM-10 stands).
- LM-3 is asserted through a BUILT ROW (the stack view linked as LM-10 and LM-11 link it), on `paintedKey()`: it is equal
  for "carried, settings equal the preset" and "carried, every slider far". Mutant MU-P2-8.
- V-23 is captured with the same effect, the same preset, the same window size and position as V-2. Each manifest gives the
  button's rectangle in window pixels and whether the pointer was inside the test window's frame. Harmony compares the two
  rectangles pixel by pixel. Bar: equal. A pair in which either capture had the pointer inside the window is captured
  again, never compared. Unequal with the pointer outside: FAIL of "no mark"; a fix round on S3's files.
- The critics get his 133 and 138 lines verbatim (the rig rule, FB-4) and the rule of section 0 (3). STRUCK from what they
  are told: "the two buttons must look the same, and ... this sameness is his answer" (PL2:590-591). The sameness is
  Harmony's comparison, not a critic's opinion.

RB-3 A RENAME AND A DELETE REACH EVERY CARRIER IN THE OPEN SHOW (A:ST-1, A:GA-6). Replaces P2-6's writer (2) clause "a Rename
from this row carries the name" (PL2:253-254), "renamed from another effect's row: question 152" (PL2:263-264), F-LB2-e
(PL2:308-310), question 152, the plan's check 19, PL2 R4's "Not closed further here", and RA-1's last bullet (RA:274-275).
- THE RULE. After the store renamed a preset of effect E from A to B, every slot of the open show whose effect is E and
  whose name is exactly A gets the name B. After the store removed preset A of E, every such slot gets the empty name.
  "Every slot" = the clip effects of every clip of every deck, retired decks included (`Composition::forEachClip`), every
  layer's effects, and the global effects (FB-10).
- THE PARTS. `fxpreset::carryNameIn(std::vector<Clip::EffectSlot>&, effect, from, to) -> int` and
  `fxpreset::carryName(Composition&, effect, from, to) -> int` (the number of slots changed), in src/core/EffectPresetCmd.h
  (S2). The view fires `onPresetNameChanged(effect, from, to)` ONCE after a rename or a remove that the store reports done
  (`to` empty for a remove) and writes no name itself; the hook is forwarded by InspectorPanel and the three inspectors the
  way `onPerformPreset` is; the host calls `carryName` on the composition and then `refresh()` on the stack views that are
  showing. With no host wired nothing is carried (as with a pick: LM-7).
- Not an undo step (RU:322 stands: renaming and deleting change the app's store, not the show). Message thread. No fence:
  the renderer never reads the member (LINT-EL-7) and under src/render effect vectors are only const references (RA F12).
  No lock, no new mutex.
- WHY A DELETE EMPTIES THE NAME instead of leaving it. Under RB-1 an effect with no preset reads "Default", and a carrier
  whose preset he just deleted has none. A preset made later under the freed name cannot claim it. And dim "Presets" then
  means ONE thing on his screen: this show names a preset that this computer's folder does not hold.
- WHY THIS IS RULED AND NOT ASKED. Under the plan's rule, after a rename the carriers he had NOT changed read the new name
  (they still equal it) and the carriers he HAD changed read "Presets", dim: the app would show which effects were changed
  -- Boris: "no need to show that it was changed." -- and would show as missing a preset that its own Rename had just
  renamed. By his 104 (FB-3) changing a default name is what the default names are FOR. This reading of his 133 is mine
  (INFERRED). If he corrects it, the host's walk is limited to the renaming row: one line; LC-16 and LM-25 are re-cut and
  GL-12's line re-written.
- WHAT IT DOES NOT REACH, said plainly: (a) a show that was not open -- its effects keep the old name and read "Presets",
  dim, until a preset is loaded on them (or the old name again, if a preset is ever made under it); (b) a copy of a slot
  inside the undo history -- an undo of an older step can bring an old name back (RA R3 said this of names already);
  (c) a name typed again by hand after a delete. All three have one closure, an identity inside the preset file: HD-24,
  not in this lane.
- `nextName` is NOT changed: ST-2 stands as PL2 5.1 writes it ("a freed number is used again"). With this rule no slot of
  the open show is left holding a freed name.
- LM-19 stands as PL2 5.1 writes it; its fixture installs the hook (the test's host calls `carryNameIn` on its vector).
Rows: LC-16 (new), LM-25 (new), GL-12 (new), LM-19. Mutants MU-P2-9, MU-P2-10.

RB-4 THE NAME IN THE SHOW: P2-7 STANDS, WITH THREE CORRECTIONS (A:GA-5; E3).
- PL2:280-281 ("byte for byte") is replaced by: "A show none of whose effects carries a name is written exactly as it is
  without this lane. An effect gets a name only through the writers of P2-6. One of them needs no act of his on that
  effect: writer (3) -- a row built over an effect whose settings equal one of his presets takes that preset's name
  (FB-21) -- and the next Save writes it. That is correct: the file then says what the button says." GL-11 (d) and (e) pin
  both sides.
- No baseline, checksum or fixture of another lane moves -- for this reason, not the plan's: a test-mode launch that is
  given no presets folder starts with an EMPTY one (RB-5), so writer (3) finds nothing to match.
- PL2:287-289 (the thread sentence) is replaced by: "The member is read by the three slot writers and written by the three
  slot readers exactly where `effectName` and the connections' own strings are; whatever thread rule holds for those lines
  holds for this one, and no new one is needed (which thread runs `saveToFile` was not read: FB-10). A load builds a fresh
  composition (Pitfall 58). The renderer never reads it (LINT-EL-7). No lock, no new mutex."
- No show version is raised (FB-7; INFERRED for a key inside an effect's entry). If one-save's S1 as merged pins an effect
  entry's keys, S2 adds the key to that pin and says so in its report.
- The three model files cannot see the store, so the name is written back whatever lists there (F-LB2-d) BY CONSTRUCTION:
  LINT-EL-7's last clause (RB-10).
- LC-14's fixture uses one name with a space and one with a letter outside ASCII (FB-11). A note, not a new bar.
Rows: LC-14 and LM-24 as PL2 5.2 writes them; GL-11 as re-cut in 5.5.

RB-5 THE TEST-MODE SCRATCH FOLDER IS NEW AT EVERY LAUNCH (the other half of A:GA-5; amends RU AM-12, RU:377-383, and ST-11).
- `EffectPresetStore::testModeRoot(envValue, launchTag)`: an absolute path is used as given; anything else gives a folder
  under the temp directory whose name ends in `launchTag`. `effectPresetsDir` passes a tag made of the process id and the
  launch time in milliseconds. Never under the user's Library (unchanged). Created only by the first successful `make`
  (unchanged).
- Why. GL-7's own arm makes a preset in that folder (RU:610). With one fixed folder every later test-mode launch of every
  lane would list it; writer (3) could take its name for an effect that equals it; a probe that then saves a show would
  find a key it did not expect.
- ST-11 is re-cut (5.2). Mutant MU-P2-13. GL-7's line stands: the folder is still named on stderr.

RB-6 SAVE OVER'S ONLY SIGN IS THE MENU ITEM (A:ST-3; E5). Replaces RA:304-305, RA:388, and the reason in RU:227.
- The sentence: "After New Preset the button takes the new preset's name; when the make fails it does not. After a Save
  over the button reads what it read -- on success and on failure alike. The state that tells them apart is the menu's own
  item: `Save over "<name>"` is greyed when the settings equal the listed preset (it saved) and live when they differ (it
  did not). Nothing else is shown."
- For that state to be true the store's list must never run ahead of the disk: ST-13's failure arm also asserts the LIST
  (5.2). Mutant MU-P2-12.
- LM-26 (new) pins the view's side, with the real store on a folder that cannot be written.
- V-5 and V-6 already show the item greyed and live; their manifests carry `menu.saveOver` (5.6).
- No message, no window, no live arm (section 2, A:ST-3).

RB-7 UNDO ACROSS A NEW PRESET OR A SAVE OVER (A:ST-4; E4).
- The rule, unchanged and now said: New Preset and Save over are not undo steps (RU:322; RA's PL DA-11). The undo step
  before them is the load. Cmd+Z after either takes back the LOAD: the effect's values, connections and name become what
  they were before it -- whatever name the slot was given since -- and no file changes. Redo puts the loaded preset's
  values and name back. What he kept is in the preset: loading it brings it back.
- LC-15 (new). Mutant MU-P2-11. LINT-EL-3 gains one clause: src/core/EffectPresetCmd.h includes no store header and names
  no `EffectPresetStore`.
- Check 5 regains RA's sentence (E4); check 10 gains one (section 6).
- No undo marker (section 2, A:ST-4).

RB-8 THE LIVE MUTANT BUILDS (A:GA-1). Extends HD-10 once more; replaces PL2:517-519.
- STRUCK: "They cannot mask each other ... (MU-P2-1 does not touch `loaded`)".
- M-H = MU-P2-1 ALONE. Row GL-10, FAIL line "the button read Presets after the tweak". It shares a build with nothing,
  because it also turns the `button` checks of GL-11 and GL-12 red.
- M-I = MU-P2-3 + MU-P2-10. Row GL-11, FAIL line "loaded none after a relaunch"; row GL-12, FAIL line "0/3 hosts carry
  Tide". MU-P2-3 acts only on a name read from a show's file: GL-12's show holds none. MU-P2-10 acts only at a rename or a
  delete: GL-11 does neither.
- M-J = MU-P2-14 + MU-P2-5. Row GL-11 step (e), FAIL line "no name taken when the row was built"; row GL-11 step (f), FAIL
  line "without the folder the button read Off". MU-P2-14 acts only on a slot with no name: step (f)'s slot has one.
  MU-P2-5 acts only on a name that no preset lists: in step (e) every name lists.
- HD-10's rule holds for all three: the probe evaluates EVERY check of a row and prints one FAIL line per failed check; a
  row is RED only when the line its own mutant predicts is printed. Said so that nobody reads it as a second cause: under
  M-I GL-11 prints several FAIL lines -- every check of steps (c) and (f) that needs the name read from the show's file --
  all of them from MU-P2-3; and under M-J step (e) prints a second one (P3 holds no fxPreset), from MU-P2-14.
- RA's M-D..M-G and RU's M-A..M-C stand, with the new words.

RB-9 GL-8 COMPARES NAMES, NOT A NUMBER (A:GA-4). Replaces RU:611, RA:677 and PL2:559.
- The names file: .harmony/probe-effect-presets.names, 87 lines, one pre-registered test name per line, written by HARMONY
  from the rulings before S1a's packet is cut (HD-28). Packets copy names from it; a builder never types one from memory.
- The baseline: the sorted list of names `ctest -N` prints on the lane's base, kept as a file. Taken again, on the new
  base, at every re-base.
- The bar and its RED arm: section 5.5.

RB-10 FOUR LINTS ARE RE-WORDED SO THAT CORRECT CODE CAN PASS THEM (A:GA-2, A:GA-3, A:ST-7). Replaces PL2:529-531 and
PL2:533-543. Their exact text: section 5.4. In one line each:
- LINT-EL-7 counts the key's write and read by operation and by file; a guard line is free; the model files name no
  `EffectPreset`.
- LINT-P2-1 is a per-file COUNT of the letters, after the `LookAndFeel` identifiers and `lookup` have been taken out: it
  may not rise in a file the lane touches and is 0 in a file the lane adds.
- LINT-P2-2 uses the one-save lane's own token test (FB-6): a green S7 is a green fence by construction.
- LINT-P2-3 forbids the letters `PresetManager` in any new identifier, not only as a whole word.

RB-11 GL-11 GAINS TWO ARMS AND GL-12 IS NEW (A:GA-5, A:GA-6, A:ST-1). Section 5.5. GL-11 (d) now builds the row; (e) is the
row built over an effect that equals a preset; (f) is the folder absent and then back. GL-12 drives a rename from the clip's
row, a rename from the layer's row and a delete from the global stack, and reads all three hosts each time.

RB-12 FOUR VISUAL STATES ARE RE-CUT, AND ONE TEST ACTION IS ADDED (A:GA-8, A:GA-7, A:ST-2). Section 5.6.
- V-3 shows "Default" (RB-1). V-22 is reached by a show that names a preset the folder does not hold (after RB-3 a delete
  in the open show no longer leaves that state). V-23: RB-2.
- V-24 is ONE window-id capture of the main window: the menu is drawn inside it (FB-16). `fx_preset_ui` gains the action
  `browser:MilkDrop`, which calls `BrowserPanel::setActiveTab` (FB-17); test-only, like the route.

RB-13 THE FENCE'S COST IS TOLD AS MEASURED (A:ST-5).
- GL-4's two tables stand as RU and RA write them, with the words of section 0 (1c).
- Check 4 no longer states "one frame" as a fact: Harmony fills in the per-load number GL-4 measured, for both arms, when
  she writes his page; and the check says that a video recording and Syphon get no picture for those frames (FB-20).
- HD-8 is DECIDED before S3's packet is cut whenever either arm reads above 40 held frames for its 20 loads (HD-27). S3
  does not start on an open HD-8.

RB-14 WHAT HE IS TOLD (A:ST-3, A:ST-4, A:ST-5, A:ST-6; E4). Section 6 of this ruling REPLACES PL2 section 6.

RB-15 THE FENCE WITH THE ONE-SAVE LANE, CORRECTED (A:GA-4, A:ST-7; E1, E2, E3). Replaces PL2:388-389 and PL2:392-404.
- The fence is a property of the lane's BASE, not an event: the base passes LINT-P2-2. By FB-5 the first main that can is
  main after one-save's M2. The order reads: a base that passes LINT-P2-2 -> S1a -> S1b, and SE any time before S2 -> S2
  -> S3 -> VG -> merge. The bracket "[one-save S1 merged]" is struck.
- On that base no one-save stage is open. PL2:400-404's re-base rule stands for any OTHER lane that edits the three model
  files (the transport lane saves a clip's pause; the nudge lane its amount): whichever merges second re-bases as a
  builder's step 0 (RIG-RULES A2). S2's change there is one write and one read per file.
- At every re-base Harmony takes two things again, on the new base: LINT-P2-2 and GL-8's baseline.
- E1: the old file windows are in S7's removal by name; LINT-P2-2's second clause is a re-check, not a hope. E3: no raise.
- LINT-P2-3 keeps one-save's LINT-6 green after this lane merges, whichever way "token" is read. CLAUDE.md: PL2:405-406
  stands. HD-23 holds the one alternative (the headless stages earlier).

RB-16 CORRECTIONS AND COUNTS (E5, E6, E7).
- The pitfall text (PL2:410-431) stands with ONE sentence replaced and ONE added. Replaced, PL2:422-426 ("THE BUTTON NAMES
  THE LOADED PRESET ... else "Presets"."), by: "THE BUTTON NAMES THE PRESET THE EFFECT CARRIES, NOT THE VALUES:
  `EffectSlot::fxPresetName` (message thread; saved with the show as the optional key "fxPreset", written only when not
  empty) is set by a load, New Preset, Save over, the show's reader, by a match only when a row is built for a slot that
  has none, and by the host after a Rename or a Delete of that preset -- the new name, or none, on EVERY slot of the open
  show that held the old one (`fxpreset::carryName`) -- never by a timer. The button reads that name whatever the sliders
  are, with no mark; a slot with no name reads the preset its settings equal, else "Default"; a slot whose name no preset
  here lists reads "Presets", dim. After a Save over the button does not change: the menu item's own state says whether it
  saved." Added at the end: "In test mode with no AUDIODNA_EFFECT_PRESETS_DIR the store's folder is new at every launch."
- GL-6's table prose: section 0 (1c).
- LM-12's note (PL2:497-498): under MU-EL-24 the button reads "Default", not the preset's name.
- Counts: 87 new unit cases = LK 24 + ST 17 + LC 16 + LM 26 + CE 4 (PL2: 83; + LC-15, LC-16, LM-25, LM-26). Twelve live
  rows (GL-1..GL-12). 24 visual states. Mutants MU-P2-1..MU-P2-15. Three live mutant builds of this delta (M-H, M-I, M-J).
- Questions: 150 (letters swapped) and 151 (as PL2). 152 is withdrawn. 153 is not used.

THREADS, said once. Everything this ruling adds runs on the MESSAGE thread: the walk of RB-3, the hook, the button's key,
the test action. Nothing is added to the audio callback or to the analysis thread. The render thread never waits and never
reads the name. No lock and no new mutex (LINT-EL-4 stands).
TEXTS, said once. This ruling adds NO word on screen beyond section 0 (1a). It adds no slider. The lane's menu is still
opened with `showMenuAsync` on the top-level parent (LINT-EL-2).

THE LINES OF THE EARLIER PAPERS THAT THIS RULING REPLACES, beyond PL2:762-782's own table (which stands)
| lines | what | replaced by |
|---|---|---|
| RA:274-275 | RA-1's last bullet: a rename from this row carries; a delete or a rename from another row leaves the slot | RB-3 |
| RA:304-305, RA:388; the reason in RU:227 | "the button keeps reading "Looks" ... the button is the state" | RB-6 |
| RU:377-383 (AM-12), RU:511 (ST-11) | the test-mode scratch folder | RB-5 |
| RA:564 (ST-13) | the failure arm | RB-6, 5.2 |
| RU:611, RA:677 (GL-8) | the count gate | RB-9, 5.5 |
| RA:842-846 (HD-10 as extended) | the live mutant builds | RB-8 (extended, not replaced) |
| RA:714-715 (V-22) | how the state is reached | RB-12 |
| RA:738-739 | the sentence the plan dropped | restored (check 5) |
| PL2:241-244, :253-254, :263-268, :301-304, :308-310 | P2-6's cases, writer (2), question 152's fork | RB-1, RB-3 |
| PL2:280-281, :287-289 | "byte for byte"; the thread sentence | RB-4 |
| PL2:373-380, :382-389, :392-404 | the stage table; what Harmony runs; the order; the fence | section 4, RB-15 |
| PL2:422-426 | one sentence of the pitfall text | RB-16 |
| PL2:497-498, :505, :508 | LM-12's note; LM-3; the count | 5.2, RB-16 |
| PL2:517-519 | the live mutant build M-H | RB-8 |
| PL2:529-531, :533-543 | LINT-EL-7, LINT-P2-1, -2, -3 | 5.4 |
| PL2:559, :562, :569-575 | GL-8; GL-11 and its steps | 5.5 |
| PL2:585, :589-595 | V-3, V-23, V-24; what the critics are told | 5.6 |
| PL2:599-669 | his checks | section 6 |
| PL2:676-694 | questions 150 and 152, and what changes with each | section 7 |
| PL2 R4, R5, R6 (PL2:717-727); PL2:753 | the risks RB-3 and FB-7 re-cut; "Carrying a rename to other effects" | section 10; now IN the lane |
Everything else of PL2 stands: LB1's tables and forks, P2-3, P2-4, P2-5, P2-7 but for RB-4, LB3, LB4, PL2 5.1 but for the
rows named in 5.2 here, the 5.5 rows not re-cut here, PL2's risks R1-R3 and R7-R13, and its "NOT IN THIS LANE" but for the
one line above.

---------------------------------------------------------------------------------------------------------
## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row and gives every gate verdict)
---------------------------------------------------------------------------------------------------------
One lane, one worktree (the branch's name is Harmony's; `lane/effect-presets` is suggested). The rules of RA section 4
stand: each stage is ONE builder context, ends with the full ctest green, shows its own unit rows RED first (the named
mutant) and then GREEN, and is reviewed pinned before the next starts; a builder never launches the app, never runs a live
row, never gives a verdict; docs move in the stage that changes the behaviour. This table REPLACES PL2:373-380.

| stage | owns (files) | proves (unit, by the builder) | Harmony runs herself, and when |
|---|---|---|---|
| S1a the preset (headless) | NEW src/effects/EffectPreset.h (the preset with a connection per entry; `capture`, `resolve` with the policy, `kUnwiredEntry`, `matches`, `firstMatch`, `shownMatch`, `sameConn`, `connVarReadable`, `isLegalName`, `nameVerdict`, `toVar` / `fromVar`); NEW tests/test_effect_preset.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp). Touched ONLY as mutants, reverted: src/effects/EffectLibrary.cpp (MU-EL-28), src/connect/ConnSerialization.cpp (MU-LA-33). | LK-12 first, then LK-1..LK-24; LINT-EL-4; LINT-P2-1 and LINT-P2-3 on the stage's diff; their mutants RED | BEFORE S1a, on the lane's base: LINT-P2-2; GL-8's baseline listing; the names file (87 lines). After S1a: nothing live; the pinned review. |
| S1b the store (headless; after S1a) | NEW src/effects/EffectPresetStore.h / .cpp (`listFolder`, `presetsFor`, `make(effect, name, preset)`, `nextName`, `pathTaken`, `rename`, `remove`, `replace`, `canReplace`, `beforeSwapForTest`, `defaultRoot`, `testModeRoot(envValue, launchTag)`); CMakeLists.txt (one source); NEW tests/test_effect_preset_store.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp) | ST-1..ST-17, with ST-11 and ST-13 as 5.2 re-cuts them; LINT-EL-1, LINT-EL-4; LINT-P2-1, LINT-P2-3; their mutants RED (MU-P2-12 and MU-P2-13 among them) | Nothing live; the pinned review. |
| SE the wire that drives nothing, and `tickSlot` (as RA: shares no file with S1a or S1b; any order before S2) | src/connect/ConnectionEngine.h / .cpp; tests/test_connection.cpp (four cases appended); docs/claude/rendering.md (two sentences); as a mutant only: src/signal/SignalRegistry.cpp (MU-LA-29) | CE-1..CE-4; LINT-P2-1; their mutants RED | After the pinned review: the regression probes of RU, GREEN only. |
| S2 the load, undo, the host, the name in the show, the carry, the data routes, the probe (after S1b AND SE) | NEW src/core/EffectPresetCmd.h (the command; `fxpreset::carryNameIn`, `fxpreset::carryName`); src/model/Clip.h (ONE member, `fxPresetName`, last in `EffectSlot`); src/model/Clip.cpp, src/model/Layer.cpp, src/model/Composition.h (in each: ONE write and ONE read of "fxPreset" inside the effect loops, the read guarded as the lines beside it are; nothing else); src/MainComponent.h / .cpp (the store member, `effectPresetsDir` with its launch tag, `performPresetLoad` with the hook of RA-7, the route callbacks); src/api/ApiServer.h / .cpp (inside the test-only block: GET /api/debug/fx_presets, POST fx_preset_make, fx_preset_load, fx_preset_replace); NEW tests/test_effect_preset_cmd.cpp; tests/CMakeLists.txt; NEW .harmony/probe-effect-presets.sh with its fixtures and a selftest (rows GL-1, 2, 3, 4, 6, 7, 9); docs: docs/claude/effects.md (new section "Effect presets", with the rule of section 0 (3) and the button's four cases), docs/claude/pitfalls.md (entry NN: PL2:410-431 with RB-16), docs/claude/testing-eyes.md (the routes, the variable), docs/claude/architecture.md (the new files; `EffectSlot::fxPresetName`; the show key) | LC-1..LC-16; LINT-EL-3, LINT-EL-5, LINT-EL-6, LINT-EL-7; LINT-P2-1, LINT-P2-3; their mutants RED (MU-P2-3, -4, -6, -9, -11 among them) | After the pinned review: GL-1, GL-2, GL-3, GL-4 (both arms), GL-6, GL-7, GL-9, each with its RED arm. GL-4 and GL-6 have decision tables: a STOP outcome stops the lane before S3; a GL-4 arm above 40 waits for HD-8 (RB-13). |
| S3 the menu, the name box, Save over, the button, the hook | NEW src/ui/EffectPresetsButton.h (with `paintedKey`); NEW src/ui/EffectPresetsMenu.h (the pure models: the menu, the box, the Save Over window's keys, `buttonState` with its four cases; the name filter); src/ui/EffectStackView.h / .cpp (as RA's S3 cell, plus `onPresetNameChanged`); src/ui/UniversalParamControl.h / .cpp (ONE method, `refreshConnectionDisplay`); src/ui/InspectorPanel.h / .cpp and the three inspector headers (forwarders only); src/MainComponent.cpp (the wiring lines, the host's carry, the `fx_preset_ui` callback with `browser:MilkDrop`); src/api/ApiServer.h / .cpp (`fx_preset_ui`; GET fx_presets gains `menu.saveOver`, `box`, `confirm`, `button`); NEW tests/test_effect_presets_menu.cpp; tests/CMakeLists.txt; .harmony/probe-effect-presets.sh (rows GL-5, GL-10, GL-11, GL-12); docs: CLAUDE.md (the capability paragraph gains "effect presets (per effect, kept by the app for every show)"; UI Patterns gains "Effect Presets button"; the pitfall index gains NN), docs/claude/effects.md (the menu, the box, Save over, the button's rule, what a Rename and a Delete do to the open show), .harmony/APP-INVENTORY.md | LM-1..LM-26; LINT-EL-2; LINT-P2-1, LINT-P2-3; their mutants RED (MU-P2-1, -2, -5, -7, -8, -10, -14, -15 among them) | GL-5, GL-10, GL-11, GL-12 with their RED arms (the builds M-H, M-I, M-J and RA's M-D..M-G); GL-8; the regression probes; then GL-1, GL-2, GL-3, GL-4, GL-7, GL-9 once more at the lane's final head (GREEN only); LINT-P2-1 on the lane's whole diff. |
| VG the visual gate | a capture builder (window-id captures only, driven through `fx_preset_ui`; 24 states; each manifest holds the model facts, EVERY text the state paints, the button's rectangle in window pixels, and whether the pointer was inside the test window), then FIVE critic seats given his 133 and 138 lines verbatim, the rule of section 0 (3), and the list of things that pre-date the lane (RU 5.7's list, plus the MilkDrop tab's and the Record tab's own words) | -- | The verdict; the V-2 / V-23 pixel comparison (RB-2); that no manifest text holds the old word. A fix round goes back to S3's files. Boris sees nothing of this lane before it passes. |

What Harmony runs herself, in order: (0) on the lane's base: LINT-P2-2, GL-8's baseline, the names file; questions 150 and
151 to Boris with the adoption; (1) after SE's review: the regression probes; (2) after S2's review: the seven live rows;
if a GL-4 arm reads above 40, HD-8 before S3's packet; (3) after S3's review: GL-5, GL-10, GL-11, GL-12, GL-8, the
regression probes, the re-run, LINT-P2-1 on the whole diff; (4) the visual gate; (5) the merge sequence of RIG-RULES B.
Every live row takes the live lock; none opens an Output window, captures a full screen or sends synthetic input (the
SCREEN-SAFETY LAW; RA's rig, unchanged). HD-22 stands and widens by one row: GL-12 shows the name box and the Delete
confirm of the TEST app on his screen for seconds.
Order: a base that passes LINT-P2-2 -> S1a -> S1b, and SE any time before S2 -> S2 -> S3 -> VG -> merge (RB-15).
Order against the other lanes: RA:528-534 and RU:466-470 stand, with the one-save bullet replaced by RB-15.

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; exact strings and bars; each with its RED arm; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------------
This section is LAID OVER PL2 section 5, which is laid over RA section 5, which is laid over RU section 5. WHERE A STRING IS
COPIED FROM: a row named in this section -- from here; else from PL2 section 5; else from RA section 5; else from RU
section 5. Every row is RED first. One TEST_CASE per row id, named exactly as written (GL-8 reads the names).
No bar is loosened by this ruling. GL-8 is tightened (names, not a number). GL-11 gains clauses. ST-11 and ST-13 gain a
clause each. LM-3 is re-cut once more: the half of it that the plan pinned for an effect without a preset is the behaviour
he declined (FB-2).

5.1 THE UNIT-CASE NAMES THAT ONLY CHANGE THEIR WORDS. PL2 5.1 (PL2:446-494) stands for 45 of its 47 rows, exactly as it
writes them (checked by machine, FB-18). ST-11 and ST-13 are re-cut in 5.2. Notes, no change of text:
- LM-12: under MU-EL-24 the row-built match fails and the button reads "Default" where the row expects the preset's name
  (E7).
- LM-19: the fixture installs the name-changed hook (RB-3); without a host nothing is carried.
- LM-22: its last clause ("only an effect with no loaded preset takes the preset it matches, and only when its row is
  built") is also armed by MU-P2-14.
- LM-18's "the button reads its name" and LM-22's "the button ... read B" are true by case (1) of RB-1.
- LK-23's literal and every fixture: `<name>.preset.json`, "format": "audio-dna-effect-preset".

5.2 THE ROWS THIS RULING RE-CUTS OR ADDS (and the two PL2 rows that stand, for the count)
| id | stage, file | exact test name | RED arm |
|---|---|---|---|
| LM-3 (re-cut; replaces PL2:505 and RU:532) | S3, tests/test_effect_presets_menu.cpp | "LM-3 the button keeps its name: with a preset loaded, one slider step away and with every slider far from its values, the row's button paints the same text, colour and triangle as when the settings equal the preset; an effect that was never given a preset reads Default at its defaults, one step away and far from them, and is never dim; after Default is loaded and a slider moves it still reads Default" (asserted on `paintedKey()` of a built row, RB-2) | MU-P2-1, MU-P2-2, MU-P2-7, MU-P2-8 |
| ST-11 (re-cut; replaces PL2:458) | S1b, tests/test_effect_preset_store.cpp | "ST-11 the test-mode root is never the real folder: an absolute AUDIODNA_EFFECT_PRESETS_DIR is used as given; unset, empty or relative gives a scratch folder under the temp directory whose name carries the launch tag, so two launches never share one" | MU-EL-21, MU-P2-13 |
| ST-13 (re-cut; replaces PL2:477) | S1b, tests/test_effect_preset_store.cpp | "ST-13 replace swaps a whole verified file in: the preset then lists with the new values and connections; a second store on the folder reads the same; a replace that cannot write, or whose new file does not read back equal, leaves the old file's bytes unchanged and the list still holding the old preset" (a writer hook that truncates; a read-only folder) | MU-LA-9, MU-LA-26, MU-P2-12 |
| LC-14 (stands, PL2:506) | S2, tests/test_effect_preset_cmd.cpp | as PL2 writes it (fixture note: one name with a space, one with a letter outside ASCII) | MU-P2-3, MU-P2-4, MU-P2-6 |
| LC-15 (new) | S2, tests/test_effect_preset_cmd.cpp | "LC-15 undo takes back the load whatever the slot was named since: after a load, with the slot's name set to another preset's the way New Preset and Save over set it, undo gives the values, connections and name from before the load, and redo gives the loaded preset's again" | MU-P2-11 |
| LC-16 (new) | S2, tests/test_effect_preset_cmd.cpp | "LC-16 a name is carried through the whole show: carryName changes it on every slot of that effect that held the old name -- a clip effect on every deck, a retired deck's clip, a layer effect and a global effect -- returns how many it changed, and leaves a slot of another effect that holds the same name, a slot with another name and every value and connection untouched; an empty new name empties it" | MU-P2-9 |
| LM-24 (stands, PL2:507) | S3, tests/test_effect_presets_menu.cpp | as PL2 writes it (fixture note: the slot's settings equal neither Default nor a listed preset) | MU-P2-5, MU-P2-15 |
| LM-25 (new) | S3, tests/test_effect_presets_menu.cpp | "LM-25 a rename and a delete are handed to the host once: after a rename the store did, the name-changed hook is called exactly once with the effect, the old name and the new one; after a delete, once with an empty new name; after one that failed or was cancelled, never; with the hook carrying, the renaming row, a second row still equal to the preset and a third row that was changed all read the new name, not dim, and after the delete all three hold no name" | MU-P2-10 |
| LM-26 (new) | S3, tests/test_effect_presets_menu.cpp | "LM-26 a Save Over that cannot write changes nothing: the old preset's bytes and its list entry stay, the Save over item still names that preset and is live, the button reads the same name, and a later Save Over on a writable folder works" (the real store on a real read-only folder; the window through the recorder of RA-13) | MU-P2-12 |
DORMANT, used only if he answers 150 B: "LM-3 the button keeps the loaded preset's name: with a preset loaded, one slider
step away and with every slider far from its values, the row's button paints the same text, colour and triangle as when
the settings equal the preset; an effect with no loaded preset reads the preset its settings equal and, one step away,
Presets, dim; after Default is loaded and a slider moves it reads Presets, dim" (RED arms MU-P2-1, MU-P2-2, MU-P2-8).
Counts: 87 new unit cases = LK 24 + ST 17 + LC 16 + LM 26 + CE 4.

5.3 THE MUTANTS
RU's MU-EL-1..48 and RA's MU-LA-1..39 stand with the new words, EXCEPT MU-EL-17, which stays RETIRED (PL2:511-512).
PL2's MU-P2-1..MU-P2-6 stand as PL2:513-516 writes them. This ruling's:
MU-P2-7 `buttonState` has no case (3): a slot with no name whose settings match nothing reads "Presets", dim (LM-3).
MU-P2-8 the row hands the button a second input, and the triangle is drawn in the secondary colour when the settings
differ from the loaded preset; `buttonState` itself stays right (LM-3's painted-key clause).
MU-P2-9 `carryName` walks the clip effects only (LC-16).
MU-P2-10 the view never fires the name-changed hook (LM-25; GL-12).
MU-P2-11 undo puts the earlier name back only while the slot still holds the name the command set (LC-15).
MU-P2-12 `replace` updates its list entry before it writes (ST-13; LM-26).
MU-P2-13 `testModeRoot` ignores the launch tag: one fixed scratch folder (ST-11).
MU-P2-14 writer (3) is removed from `rebuildRows` (LM-22; GL-11 step e).
MU-P2-15 the row empties a name that no preset lists (LM-24).
The live mutant builds of this delta: M-H, M-I, M-J (RB-8).

5.4 THE LINTS (static; run by the builder, re-run by Harmony; each shown RED on a seeded line unless said)
- LINT-EL-7 (re-worded; replaces PL2:529-531) "`grep -rn fxPresetName src/render` prints nothing;
  `grep -rl '\"fxPreset\"' src` prints exactly src/model/Clip.cpp, src/model/Layer.cpp and src/model/Composition.h; in each
  of those three `grep -c 'setProperty *(\"fxPreset\"'` is 1 and `grep -c 'getProperty *(\"fxPreset\")'` is 1;
  `grep -n EffectPreset src/model/Clip.h src/model/Clip.cpp src/model/Layer.cpp src/model/Composition.h` prints nothing".
  A guard line (`hasProperty`) is free. RED arms: a second write seeded in one of the three; the literal seeded in a fourth
  file.
- LINT-P2-1 (re-worded; replaces PL2:533-538). Two parts; each grep pattern below is ONE line.
  (a) this command prints nothing, run over src tests docs/claude CLAUDE.md .harmony/probe-effect-presets.sh:
  `grep -rnE 'Looks\"|\.look\.json|audio-dna-look|LOOKS_DIR|lookName|looksFor|LooksButton|EffectLook|LookNameFilter|PerformLook|LooksUi|lookWindowOpener|look_(make|load|ui|replace)|/api/debug/looks|probe-effect-looks'`
  (b) "for every file the lane's diff touches COUNT at the head is not above COUNT at the base, and for every file the
  lane adds COUNT is 0 -- where COUNT(file) is how many times the four letters l-o-o-k occur, in any letter case, after
  every identifier that holds `LookAndFeel` and every `lookup` / `lookups` has been taken out of the text". One way to
  compute COUNT:
  `sed -E 's/[A-Za-z_0-9]*LookAndFeel[A-Za-z_0-9]*//g; s/[Ll]ookups?//g' FILE | grep -oi look | wc -l`
  Run on each stage's diff and, by Harmony, on the lane's whole diff. (a) prints nothing on today's tree (run by me).
  RED arms: `lookName` seeded in src for (a); a comment "the look store" seeded in a new file for (b).
- LINT-P2-2 (re-worded; replaces PL2:539-541; the fence; Harmony, on the lane's base before S1a's packet and again at every
  re-base). Both commands print nothing; each pattern is ONE line:
  `grep -rnE '(^|[^A-Za-z])PresetManager' src`
  `grep -nE '\"Save preset\.\.\.\"|\"Load preset\.\.\.\"|\"Legacy Preset\"' src/MainComponent.cpp`
  The first is the one-save lane's own token test (OS:640-641). RED arm: the tree of 7bc6df7 (38 lines for the first, run by
  me; three for the second).
- LINT-P2-3 (re-worded; replaces PL2:542-543) "the stage's diff adds no line that holds the letters PresetManager outside
  `ProjectMPresetManager`, no file whose name starts with Preset, and no declaration `class Preset`, `struct Preset` or
  `namespace preset`".
- LINT-EL-3 (PL2:526-527) gains: "and it includes no store header and names no `EffectPresetStore`" (RB-7).
- LINT-EL-1, -2, -4, -5, -6 stand as PL2 5.4 writes or keeps them.

5.5 HARMONY'S LIVE ROWS (.harmony/probe-effect-presets.sh). The rig is PL2 5.5's, unchanged: a test-server build;
`open -g`; --test-mode; a FRESH AUDIODNA_EFFECT_PRESETS_DIR per row and per arm; the live lock; no Output window; no
full-screen capture; no synthetic input; HTTP with Connection: close; quits only its own pid. GL-1, GL-2, GL-3, GL-4 (both
arms), GL-5, GL-6, GL-7, GL-9 (both forms) and GL-10 stand as PL2:552-561 writes them, GL-10 with PL2:563-568's reads.
| row | bar (the exact line printed on pass) | RED arm (and its FAIL line) |
|---|---|---|
| GL-8 (re-cut; replaces RU:611, RA:677, PL2:559) | "GL-8 PASS ctest <N> = baseline <N0> + 87 new cases: each of the 87 pre-registered names once, no baseline name missing, none unlisted, 0 failed" (87 = 24 + 17 + 16 + 26 + 4) | the comparer's selftest, three doctored listings, one at a time: a pre-registered name deleted -> "GL-8 FAIL missing: <name>"; a name added that is in neither list -> "GL-8 FAIL not pre-registered: <name>"; a baseline name deleted -> "GL-8 FAIL baseline name missing: <name>" |
| GL-11 (re-cut; replaces PL2:562) | "GL-11 PASS the name is the show's: Off loaded, amount moved to 0.2500, show saved; after a relaunch on the saved show loaded Off, the button read Off, not dim, matched none, Save over \"Off\" live; the saved show holds fxPreset once; a show saved after its row was built on an effect that equals no preset of his holds none; one saved after its row was built on an effect that equals Same holds fxPreset \"Same\" once; without the folder the button read Presets, dim, and the show saved there still holds fxPreset \"Off\"; with the folder back the button read Off" | MU-P2-3 ("loaded none after a relaunch"); MU-P2-14 ("no name taken when the row was built"); MU-P2-5 ("without the folder the button read Off") |
| GL-12 (new) | "GL-12 PASS a rename and a delete reach every carrier: Wave loaded on 3 hosts, one of them changed; renamed to Tide from the clip: 3/3 hosts carry Tide and read it, not dim; renamed to Surf from the layer: 3/3; deleted from the global stack: 0/3 carry a name and 3/3 read Default; the folder holds no Ripple preset" | MU-P2-10 ("0/3 hosts carry Tide") |
GL-8, how. The comparer reads three lists: the baseline (names `ctest -N` printed on the lane's base), the 87 names of
.harmony/probe-effect-presets.names, and the names `ctest -N` prints at the head. PASS needs: head = baseline + the 87 as
SETS, each of the 87 exactly once, and a full ctest with 0 failed. <N0> and <N> are the sizes of the first and the third.
The baseline is the one taken on the lane's CURRENT base (RB-9).
GL-11, step by step. Show B (a scratch copy). Folder F1 holds Invert / "Off" (amount 0.0, no connection). Folder F2 holds
"Off" and Invert / "Same" (amount 0.40, no connection, the current format). Folder F0 is empty. "expand" below is
`fx_preset_ui expand` on the clip's Invert; the route re-points that stack view at the chain, so the rows are built anew,
before it unfolds the row.
(a) launch on show B with F1; expand; fx_preset_load "Off"; POST /api/set_param Invert amount 0.25; GET fx_presets:
    `loaded` "Off", `matched` "", `button` = {"Off", dim false}.
(b) save the show to scratch path P1 (POST /api/debug/save_composition, as the one-save lane leaves that route); count the
    lines of P1 that hold "fxPreset": exactly 1, with "Off".
(c) quit own pid; relaunch on P1 with F1; expand; GET: `loaded` "Off", `button` = {"Off", dim false}, `matched` "", amount
    0.2500, `menu.saveOver` = {`Save over "Off"`, enabled}.
(d) relaunch on the untouched show B with F1; expand (the row is built; Invert is at its default, 0.70, FB-24); GET:
    `loaded` "", `button` = {"Default", dim false}; save to P2; P2 holds "fxPreset" 0 times.
(e) relaunch on the untouched show B with F2; FIRST POST /api/set_param Invert amount 0.40, THEN expand (the row is built
    over an effect that equals "Same" and is not at its defaults); GET: `loaded` "Same", `button` = {"Same", dim false};
    save to P3; P3 holds "fxPreset" on exactly 1 line, with "Same".
(f) relaunch on P1 with F0; expand; GET: `loaded` "", `button` = {"Presets", dim true}, `menu.saveOver` = {"Save over",
    greyed}; save to P4; P4 holds "fxPreset" on exactly 1 line, with "Off". Relaunch on P4 with F1; expand; GET: `loaded`
    "Off", `button` = {"Off", dim false}.
GL-12, step by step. Show A, after S3. The folder holds Ripple / "Wave": the current format, Dry/Wet 1.0, intensity 0.40,
speed 0.60, freq 0.70, no connection (not Ripple's defaults, FB-24). The three hosts: the clip's Ripple (layer 0, column 0,
fx 1), layer 0's Ripple (fx 2), the global Ripple (fx 0).
(a) fx_preset_load "Wave" on all three; POST /api/set_param on the clip's Ripple, intensity 0.45 (the changed carrier);
    `fx_preset_ui expand` on each; GET fx_presets on each: `loaded` "Wave" three times.
THE TWO READS PER HOST, used in (b), (c) and (d): FIRST a GET fx_presets with no UI call before it (it answers from the
model: read `loaded`); THEN `fx_preset_ui expand` on that host and a second GET (the row is on screen: read `button`).
"Carry" is counted on the first read, "read" on the second. The order matters: an `expand` builds the rows, and a row built
over an effect that equals the renamed preset would take its name by writer (3) and hide a missing carry.
(b) `fx_preset_ui rename:Wave` on the CLIP's Ripple; `accept:Tide`; reads until `box.open` is false; the two reads on each
    host: `loaded` "Tide" three times, `button` = {"Tide", dim false} three times; the folder holds Tide.preset.json and
    no Wave.preset.json.
(c) `fx_preset_ui rename:Tide` on the LAYER's Ripple; `accept:Surf`; the two reads on each host: "Surf", three times each.
(d) `fx_preset_ui delete:Surf` on the GLOBAL Ripple; `accept`; reads until `confirm.open` is false; the two reads on each
    host: `loaded` "" three times, `button` = {"Default", dim false} three times; the Ripple folder holds no file that
    ends in .preset.json.
Under MU-P2-10 the first reads of (b) give `loaded` "" three times: "0/3 hosts carry Tide". DORMANT, used only if he
answers 150 B: the same PASS line with "3/3 read Presets, dim" in place of "3/3 read Default".

5.6 THE VISUAL GATE: 24 STATES (RU's rules; window-id captures only; each manifest lists the model facts, every text the
state paints, the button's rectangle and whether the pointer was inside the window; Harmony checks that no text holds the
old word)
Stand as PL2:579-584 writes them: V-1, V-2, V-4, V-5, V-7, V-8, V-9, V-10, V-12, V-13, V-14, V-15, V-16, V-17, V-18, V-19,
V-20, V-21. V-6 and V-11 stand as PL2:586-588 re-cuts them; the manifests of V-5 and V-6 carry `menu.saveOver` (RB-6).
Re-cut here:
- V-3 an effect that was never given a preset, one slider moved: the button reads "Default", normal colour (150 B would
  make it "Presets", dim).
- V-22 a show that names a preset this folder does not hold (fixture: a show whose effect entry has "fxPreset": "Gone"):
  the button reads "Presets", dim; in its menu nothing is ticked and "Save over" is bare and greyed.
- V-23 the effect and the preset of V-2 with every slider far from the preset: the button reads the preset's name, normal
  colour (manifest: `loaded` the name, `matched` ""). Compared with V-2 by pixels (RB-2).
- V-24 ONE capture of the main window: the browser on its MilkDrop tab (through `browser:MilkDrop`), the clip inspector on
  a clip with an effect, that effect's Presets menu open. The manifest lists every text of the tab that holds the word.
  FALL-BACK, used only if the capture builder reports that the menu does not stay open in the background rig for this
  state: two captures of the same launch -- the main window with the tab and the unfolded effect row, and state V-6 --
  with the menu's items in the manifest, and the critics are told it is two pictures.
What the critics are told, beyond RU's list: his 133 and 138 lines verbatim; the rule of section 0 (3); "the dim
'Presets' means this show names a preset that is not in this computer's folder"; that the MilkDrop tab's and the Record
tab's own words pre-date the lane. STRUCK: RA:718's "the button can read "Look 1" while the menu says ..." (that state no
longer exists) and PL2:590-591's sentence on sameness (RB-2).
NOT capturable in the rig: as RA:719-720 (which control holds the keyboard, the caret, a sub-list hanging off its parent
item, hover colours).

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; this list REPLACES PL2 section 6 and RA section 6)
---------------------------------------------------------------------------------------------------------
On his own screens, in his own show. Nothing here is shown to him before the visual gate has passed.
1. Put an effect on a clip: its small button reads "Default". Move a slider: it still reads "Default" (question 150). Press
   it and choose New Preset -> a small box opens holding "Preset 1", already selected, and you can type at once. Press
   Return -> the button reads "Preset 1". Or type a name first, then Return -> the button reads that name. Esc -> no preset
   -> wrong: you must click the box before you can type; typing adds to "Preset 1" instead of replacing it; a preset
   appears after Esc; the word "look" anywhere.
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Preset 1" is there;
   choosing it sets the sliders as you left them -> wrong: it is missing, or it is there only in the first show.
3. While the box is open, type letters that are your clip keys -> they go into the box and no clip fires. After Return or Esc
   your keys launch clips again at once -> wrong: a clip fires while you type, or the keys are dead after the box closes.
4. With a clip playing on that layer, load a preset -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move. For <the number Harmony measured> frame(s) the screens hold the picture
   they had, as they do when you add an effect; a video recording and Syphon get no picture for those frames -- also as
   today when you add an effect. The same with a preset that plugs signals in -> wrong: the layer restarts, the picture goes
   black, anything in the strip changes, or a hitch you can see in a recording.
5. Press Cmd+Z -> the effect returns to how it was just before the preset, sliders AND signals, and the button reads what it
   read before; a slider you moved on ANOTHER effect meanwhile stays where you put it. (A slider you moved or a signal you
   plugged on the SAME effect after the preset goes back too.) -> wrong: the other effect's slider jumps back; the effect
   folds shut; the sliders come back but the signals do not.
6. THE NAME STAYS. Load "Preset 2", then move its sliders as far as you like -> the button still reads "Preset 2", in the
   same colour, with no mark. Open the menu: "Preset 2" is still ticked; click it -> the effect is back on Preset 2 -- its
   sliders, and its signals too: a signal you plugged in since, which Preset 2 does not hold, is unplugged. Cmd+Z takes all
   of that back. Save the show, quit, open it again -> the button still reads "Preset 2" -> wrong: the name dims, turns into
   "Presets", gets a star or a dot; the name is gone after you re-open the show; the name changes by itself while you drag
   a slider.
7. Signals. (a) Plug the bass into a slider, set its range, make a preset. Put the same effect on another clip and load the
   preset -> the slider there follows the bass with the same range, from the first frame. (b) Load that preset again on the
   first effect -> nothing jumps, and a slider you are holding stays in your hand. (c) Load "Default", or a preset you made
   with no signal on a slider, or click the preset the effect already carries -> the signals that preset does not hold are
   unplugged. Cmd+Z brings sliders and signals back in one step. That is the only way back, and only until you close the
   show: wiring you want to keep, keep as a preset first. (d) To change what signals a preset holds: load it on an effect
   in your show, change the signals there, then `Save over "<its name>"`. There is no other place to edit a preset.
   (e) A preset remembers "Macro 3", "Mod 1" or "Mod 2" by name, not what they are doing in this show. (f) A slider driven
   by "Clip Position" holds still today, with or without a preset: that is older than this build and is on the list ->
   wrong: a slider pinned at one end after a load; a signal left on after Default; an undo that brings back sliders but
   not signals.
8. Rename opens the same box, holding the preset's name, selected. Delete: a list in red; pick one; a window asks. You never
   have to load a preset to rename or delete it -> wrong: a preset you cannot rename or delete; a name that changes by
   itself.
9. New Preset is grey when the effect already is exactly one of your presets, or Default: there is nothing new to keep.
   `Save over "Preset 2"` is live only after you loaded or made Preset 2 on that effect and then changed something.
10. Load "Preset 2", change a slider, open the menu. New Preset -> the box says "Preset 3": Return keeps the change as
   Preset 3 and Preset 2 is as it was. Or `Save over "Preset 2"` -> a window asks -> click Save Over: Preset 2 now holds the
   change wherever you load it from now on. The button does not change when you save over; to see that it saved, open the
   menu again: `Save over "Preset 2"` is grey. An effect that already had the old Preset 2 keeps its settings and still
   reads "Preset 2"; clicking Preset 2 in its menu gives it the new settings. There is no undo for a Save over or a New
   Preset: Cmd+Z after either takes back the last time you LOADED a preset on that effect, not the save -- what you saved
   stays in the preset -> wrong: Preset 2 changed after New Preset; Save over names another preset; Preset 2 is missing or
   half-changed after a Save over.
11. In the Save Over window press Return -> the window closes and NOTHING was saved; only a click on "Save Over" saves. In
   the name box, type a name you already have -> the name turns red and the button goes grey; press Return -> nothing happens
   and the box stays -> wrong: Return saves over a preset; a taken name closes the box.
12. After the menu, the box or one of the windows closes, your keys work as before (Return, your clip keys) -> wrong: Return
   opens the menu again, or presses a button.
13. Record a take, load a preset in the middle, play the take back -> this build does NOT play the preset change back, and a
   take never stores which signals are plugged in. The same is true today of an effect slider you move with the mouse.
   Recording these is its own build, and it is on the list.
14. Where your presets live: on this computer, in the folder Library / Audio-DNA / Effect Presets inside your home folder,
   one folder per effect, one small file per preset. A show you open on another computer still looks the same -- each effect
   keeps its settings and its signals in the show -- but the menus there list only that computer's presets, and a button
   whose preset is not on that computer reads "Presets", dim. Copy that folder across and the names are back. To back your
   presets up, copy that folder.
15. If the disk is full or that folder is locked, New Preset and Save over do nothing and the old preset is untouched. No
   message. After New Preset the button does not take the new name. After Save over the button reads the same either way:
   open the menu -- `Save over "..."` is still live when it did not save.
16. By eye on your own screens: the button's size; the dim "Presets" (you will see it only for a preset this computer does
   not have); the menu's length with many presets; the red Delete list; the name box and the red name when a name is taken;
   the Save Over window.
17. The old buttons (they go with the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone.
   Your two old presets and your nine quick FX saves were moved to the Trash at your word. Still on your disk, and no longer
   opened by the app: one deck file ("test 1.deck.json") in Library / AudioDNA / Presets.
18. Two kinds of preset on one screen: open the MilkDrop tab on the left and an effect's Presets menu on the right. The
   MilkDrop list says "presets" too ("Search presets...") -> tell us if you ever read one as the other (question 151).
19. Rename a preset that several effects carry -> every effect in the open show that carried it reads the new name, whether
   you had changed it or not. Delete it -> those effects read "Default" and keep their settings. One thing to know: a show
   that was NOT open at that moment still holds the old name. Its effects read "Presets", dim, until you load a preset on
   them -- and if you ever make a new preset under that old name, they will read it as theirs -> wrong: after a rename an
   effect of the open show reads "Presets".
20. Not changed by this build: the Record tab still says "holding the last look". There "look" means the picture your take
   left on screen, not an effect preset; that screen is being rebuilt with the recording review.

---------------------------------------------------------------------------------------------------------
## 7 BORIS QUESTIONS (numbers 150 and 151; each has a default A; nothing waits; 152 is withdrawn; 153 is not used)
---------------------------------------------------------------------------------------------------------
Only what is his. Neither question was shown to him before; these are their words. Questions 131-134 and 138 are answered
and are not re-worded.
150. You add an effect: its small button reads "Default". You move one of its sliders. You have never given this effect a
     preset.
     A (default) It keeps reading "Default" -- the same way "Preset 2" stays after you change it.
     B It reads "Presets", dim, as soon as it is no longer at its defaults, and "Default" again when you bring the sliders
       back.
151. MilkDrop's list also calls its items "presets" ("Search presets...", "12 presets"). Now effects have presets too.
     A (default) Leave MilkDrop's words as they are: they are inside the tab named MilkDrop.
     B Make them say "MilkDrop presets" in full, everywhere in that tab.
What changes with each answer (so that either is a small change)
- 150 B: `buttonState` loses its case (3) -- one line. LM-3 takes its dormant name (5.2); V-3 shows "Presets", dim; check 1
  says so; GL-12 takes its dormant last clause (5.5). GL-10 and GL-11 do not change (GL-11 (d)'s effect is AT its
  defaults). No stored name, no format, no route changes.
- 151 B: SIX strings, not the plan's five (SF-9): src/ui/MilkDropBrowser.cpp:20 ("No presets loaded."), :101, :122, :283,
  :456 and src/ui/MilkDropBrowser.h:120. V-24 is captured again. They are not this lane's files today: one row added to
  S3, or a small packet of its own.
NOT ASKED, and why. The plan's 152 (what the other effects read after a rename): ruled in RB-3 from his 133 and told to him
in check 19; HD-26 holds the way back to a question. Every stored name, the four cases of the button, the name in the show,
the tick, the routes, the fence: technical, ruled here.
Readings these rules rest on. R88 and R95 stand by INFERRED consent (told to him, not corrected): if he corrects R88, one
constant and GL-9's dormant line (section 0 (4)); if he corrects R95, 138 B becomes a stage of its own and nothing here is
undone. Two rules rest on MY reading of his 133, which was never told to him as a reading: question 150's default (he is
asked outright) and RB-3 (he is told in check 19; if he corrects it, the walk is limited to the renaming row).

---------------------------------------------------------------------------------------------------------
## 8 HARMONY'S DECISIONS (each has a default)
---------------------------------------------------------------------------------------------------------
RU's HD-1..HD-11 and RA's HD-12..HD-22 stand as adopted, except: HD-14 is BUILT (P2-7; HD-31 holds the cut); HD-15 and
HD-18 are closed by his answers (PL2); HD-8 gets its moment (HD-27); HD-10 is extended (RB-8: M-H, M-I, M-J); HD-22 widens
by one row (GL-12 shows the name box and the Delete confirm of the TEST app on his screen for seconds). This ruling's:
- HD-23 The lane's base. Default: the first main that passes LINT-P2-2 -- by OS A-14 that is main after one-save's M2
  (FB-5). The one alternative: S1a, S1b and SE are headless and share no file with a one-save stage but the two CMake
  lists (append); they could start on main after M1, with LINT-P2-2 run before S2's packet instead of before S1a's. Her
  dispatch says the lane starts after S7; the default keeps that.
- HD-24 An identity for a preset beyond its name. Default: NOT in this lane. What it is: an "id" minted by `make`, kept by
  Rename and by Save over, stored beside the name in the slot and in the show; a slot's preset is then the one with its
  id. What it closes: all three leftovers of RB-3 (R4) -- and the walk of RB-3 would not be needed. What it costs: one key
  in the file and one in the show, rules for a file without an id and for two files with one id (a Finder copy), about
  six unit rows, and an attack round of its own. Deferring loses nothing but a version step: it is additive (a file
  without an id falls back to its name), and once presets exist on his disk it raises the file's version to 3.
- HD-25 A pick of a preset the settings already equal. Today's rule (RU:311) pushes no undo step for it, so it does not
  move the name either: with two equal presets he clicks the other one and nothing changes (GL-10 step (c) passes through
  that state). Default: leave it. To mend it: "a pick that changes no value, no connection AND no name pushes no step" --
  one clause in LM-7, one in LC-13.
- HD-26 RB-3 is built without asking him. Default: yes. If she would rather ask, PL2:683-686 words the question; its B is
  what RB-3 rules, and GL-12's RED arm is a picture of its A.
- HD-27 HD-8 is decided before S3's packet is cut whenever a GL-4 arm reads above 40 held frames. Default: yes (RB-13).
- HD-28 The names file for GL-8 (.harmony/probe-effect-presets.names, 87 lines) is Harmony's, written from the rulings
  before S1a's packet; a builder's own list is never the reference. Default: yes.
- HD-29 Question 150 goes to him with the default this ruling gives it, the opposite of the plan's. Default: yes, as
  section 7 words it, with the adoption.
- HD-30 The per-launch scratch folder (RB-5). Default: build it. The alternative is a rig rule -- every probe of every lane
  passes a fresh AUDIODNA_EFFECT_PRESETS_DIR, for ever -- which one forgotten probe breaks.
- HD-31 If she cuts P2-7 after all (the name is not saved): S2 loses the three model files; LC-14 and GL-11 go; LINT-EL-7
  returns to RA's wording with the new member name; 86 unit cases, eleven live rows; check 6 loses its last sentence and
  check 14 its sentence on "Presets", dim; V-22's fixture cannot exist and the state is dropped. Default: keep P2-7.
FACTS HARMONY MUST MEASURE (who: Harmony, on her rig; each with its ruling)
- GL-8's baseline listing on the lane's base, and again at every re-base (RB-9, RB-15).
- GL-4, both arms: the held frames per load. The number goes into check 4. An arm above 40: HD-8 before S3. Black above 0:
  STOP, as RU rules.
- GL-6; GL-9 steps (b) and (c); GL-3's noise and bar; the header width from the visual gate's manifest (as RA).
- NEW: V-2 against V-23, the button's rectangle, by pixels. Equal: "no mark" holds. Unequal with the pointer outside the
  window: FAIL, a fix round on S3's files. The pointer inside the window in either capture: capture again.
- NEW: whether the open menu stays open in the background rig with the browser on its MilkDrop tab (V-24). Yes: one
  capture. No: the two-picture fall-back of 5.6, said to the critics.
- NEW: the shape of POST /api/debug/save_composition once one-save S1 is in (FB-23): GL-11's four save steps are written
  against the merged route.
NOT measurable on the rig (the app runs in the background there): which control holds the keyboard as the name box opens,
and what a real Return does in the Save Over window. Who: Boris, checks 1, 3 and 11 (as RA).

---------------------------------------------------------------------------------------------------------
## 9 SIDE FINDINGS (found, not fixed)
---------------------------------------------------------------------------------------------------------
- SF-1 OS A-11 says the comments that name `PresetManager` sit in six files (OS:431). The grep finds seven (FB-6, with
  lines). LINT-6 catches the seventh either way. For the one-save lane's S7 packet.
- SF-2 The dead click (HD-25): with two equal presets, a click on the one the effect does not carry changes nothing.
- SF-3 The record panel's "look" (FB-15) is another meaning and is not renamed here. For the recording lane, which his
  words of 21:03:09 and 21:33:30 re-model.
- SF-4 MilkDrop's empty-list text names a developer's folder on screen ("resources/projectm_presets/",
  src/ui/MilkDropBrowser.cpp:23). Not this lane.
- SF-5 A video recording and Syphon get no frame while a fence holds (FB-20). True today of every fenced act (adding an
  effect, dropping a clip). The outputs lane (a Delay per screen; Syphon as an output) should know.
- SF-6 No route shows a browser tab (FB-17). One-save adds a "reveal" for the Compositions tab; this lane adds
  `browser:MilkDrop` to its own UI route. One general test route would serve every capture builder.
- SF-7 A name in the undo history: `EffectStackCmd` keeps whole copies of the slots (the comment at
  src/model/Clip.h:68-72), so the undo of an add, a remove or a bypass restores the names those copies held (RB-3 (b)).
- SF-8 Invert's default amount, 0.70, is the value RU's fixture show B gives its clip's Invert (FB-24). A row that needs
  that effect "not at its defaults" must move it first: GL-3, GL-10 and GL-11 (a)-(c) do; GL-11 (e) is written to.
- SF-9 The plan counts five MilkDrop strings for 151 B; the first line of the empty-list text makes six (section 7).
- SF-10 HIS WORDS OF 22:27:57, which arrived while this ruling was being written, reach this lane's new button. Boris: "the
  clip actions control clip sliders and buttons within the clip tab which include any effects," and Boris: "Also, every
  slider, button, everything needs an ignore actions toggle so the user can turn off action control of something during a
  show if they want to keep the action but need to cancel something live." An effect's presets button is a button inside
  those tabs. This lane builds neither an action that presses it nor an "ignore actions" switch on it, and a preset load
  is still not something a take records (RU HD-6; GL-6 measures it; check 13 tells him). For the actions lane to rule:
  whether loading a preset is a press an action can make, and where that button's switch sits in a header row that LM-11
  lays out for four things (B, the name, the presets button, X). No rule of this ruling changes.

NOT IN THIS LANE
- PL2's list (PL2:746-760) stands, but for one line: "Carrying a rename to other effects (question 152 B)" is now IN the
  lane (RB-3).
- An identity for a preset beyond its name (HD-24). Mending the dead click (HD-25).
- Any change to the fence or to the rows of GL-4's tables. A sign, a message or a mark after a Save over. An undo marker.
  A `nextName` that looks at the show. A general browser-tab route (SF-6).
- Presets listed in the effect list and dragged from it (138 B): costed in section 0 (4), designed nowhere.
- Any change to MilkDrop's words (question 151), to the feedback combo, to Preferences, to the Record tab's "look".
- An action that loads a preset, and an "ignore actions" switch on the presets button (SF-10: the actions lane).
- The stopped sync-dial branches: NOTHING is carried from lane/bf2 (740b6d6) or lane/bf2-keys (9eab9bd) and nothing of
  them is dropped by this lane; rulings-bf2.md H-17 already rules them superseded. Neither worktree was opened.

---------------------------------------------------------------------------------------------------------
## 10 RISKS (the strongest counterargument first)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT, against RB-3: "A:ST-1 is a SHOULD, and the plan had already filed the gap as its R4. This
   ruling answers with a new write path over the whole show, outside the undo stack, in a delta that was to add no feature
   -- and it is half a fix: a show that was not open and a copy in the undo history still keep the old name. Do it properly
   (an identity in the file, HD-24) or leave it alone (R4). The middle is the worst of both." Why the middle still stands:
   (a) the false state is made by the app's own Rename, on the show in front of him, and by his 104 changing a default name
   is the expected act -- under the plan's rule EVERY rename dims every carrier he had changed, which is the changed /
   unchanged signal his 133 declined; that is not R4's rare "made again by hand"; (b) the cost is one pure function over
   three kinds of effect list, on a member only the message thread touches and the renderer never reads: two unit rows and
   one live row; (c) an identity is a format, more rules and a wart of its own (a Finder copy has its twin's id) that no
   seat has attacked, and deferring it loses nothing, because it is additive; (d) what is left is told to him in one
   sentence (check 19). Cheapest refuting test: GL-12's RED arm IS the plan's rule -- "0/3 hosts carry Tide" is what he
   would see. Harmony can look at it and decide (HD-26).
R2 Question 150's default now rests on my reading of his 133 (FB-2), against the plan's reading ("Default is a
   description") and although R95 told him of "the "Presets" button": under A he will see the word "Presets" on that button
   only for a preset this computer does not have, and "Default" will sit on effects that are far from their defaults. Why
   it stands: the letter he declined says "it shows a name only while the effect is exactly that"; one rule for every name
   is the one he can predict; and B is one line away with every string pre-registered. Cheapest refuting test: check 1.
R3 The name in the show (P2-7) is the one thing RA promised this lane would not do; PL2's R1 argues it and stands. With
   RB-4 a show's file can change because he LOOKED at an effect (writer 3). The file then says what the button reads, and
   GL-11 (e) pins it. If Harmony cuts P2-7, HD-31 says what goes.
R4 A name is only text. RB-3 closes the open show. Still open: a show that was not open at a rename or a delete; a copy of
   a slot in the undo history; a name he types again after a delete. In all three an effect can read, and offer to save
   over, a preset it never came from. Guards: the Save Over window names the preset and the effect, and only a click saves
   (RA-2). Full closure: HD-24.
R5 An effect with no name that he brings exactly onto a preset by hand reads that preset's name only while it stays equal
   -- unless its row was built meanwhile (PL2 R5, RA R12) -- and then reads "Default" at the next tweak. The one place
   left where the button follows the values; reaching a preset by hand on every slider within 0.0005 is rare.
R6 V-23's pixel bar can fail for a reason that is not a mark. The manifest's pointer flag and the capture-again rule cover
   the pointer; anything else is reported, never waved through.
R7 GL-8's names file is one more thing to keep true: a row re-cut in a fix round changes one of its lines. Harmony owns it
   (HD-28); S3's report restates the count.
R8 LINT-P2-1 counts letters: a builder can still write another word for the old one, and it cannot see a text built at run
   time. On screen the guard is the visual gate's manifest, which lists every text a state paints.
R9 The lane waits for ALL of one-save (FB-5), not for one stage. If that lane's visual gate drags, this lane does. HD-23
   holds the way to start the headless stages earlier.
R10 GL-11 has six launches and GL-12 opens two windows of the test app on his screen (HD-22). Each row closes what it
   opened; the probe's exit path quits its own pid.
R11 `fx_preset_ui expand` is ruled to re-point the stack view, so that rows are built (GL-11 d, e, f lean on it). If the
   route as built only unfolds, step (e) fails on "no name taken when the row was built": the row reports it and the fix
   is in the route, never in the bar.
R12 Three more live mutant builds (M-H, M-I, M-J). Each is a normal cmake build of the mutant; RIG-RULES A's check after a
   mutant applies to each.
R13 PL2's risks R1-R3 and R7-R13 stand; its R4, R5 and R6 are replaced by R4, R5 and FB-7 here. RA's risks stand as
   PL2:740-741 says.

STATUS: DONE
