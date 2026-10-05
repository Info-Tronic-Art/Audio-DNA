# PLAN looks-answers2 -- the per-effect looks lane, SECOND DELTA on Boris's answers (questions 131-134 and 138): a look is a "preset" everywhere, the effect's button keeps the preset's name after a change with no mark, a signal a preset unplugs comes back only by Cmd+Z (s-rta-1004b)

Author: architect (position paper; a blind council attacks it, an architect ruling follows, Harmony decides).
Pins: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt` printed nothing
(HEAD is 8b464a6, docs only), so every code read below is a plain file whose content is that of 7bc6df7 (= the code of 185147b
the two rulings cite). The two stopped worktrees were checked (lane/bf2 at 740b6d6, lane/bf2-keys at 9eab9bd, `status --short`
empty) and NOT read further: nothing of this lane comes from them. Nothing was built, run or launched. Nothing under ~/Library
was read or listed.
Labels: VERIFIED = I read the line myself at 7bc6df7. RULING = a fact the adopted rulings re-derived (their F-n rows); I did not
re-read it. INFERRED / ASSUMED are written where used.
Short names: RU = .harmony/.reports/s-rta-1004/ruling-effect-looks.md (AM-n, LK/ST/LC/LM-n, GL-n, V-n, HD-n, MU-EL-n, LINT-EL-n
are its ids); RA = .harmony/.reports/s-rta-1004/ruling-looks-answers.md (RA-n, MU-LA-n, CE-n are its ids; RA:n = its line n);
BD:n = .harmony/binding-decisions.md line n; OS = ruling-one-save.md. THIS paper's ids: P2-n (amendments), MU-P2-n (mutants),
LINT-P2-n (lints). Paths are under /Users/boriskarpman/projects/RealTimeAudio.
Precedence asked for: Boris's verbatim words > Harmony's adoption blocks (plan-effect-looks.md:619-669, plan-looks-answers.md:
642-700) > this delta once ruled and adopted > RA > RU > the plans. WHERE THIS DELTA IS SILENT, RA STANDS WORD FOR WORD WITH
THE WORDS OF SECTION 3 LB1 LAID OVER IT, and where RA is silent RU does, the same way.
THE WORD IN THIS PAPER: the thing is called "preset" from here on. "Look" appears only inside his verbatim quotes, inside the
OLD half of an old -> new pair, and in the ids and titles of the two rulings.

---------------------------------------------------------------------------------------------------------
## 1 GOAL
---------------------------------------------------------------------------------------------------------
One stage table and one list of rows from which a builder builds the per-effect presets lane without designing or wording
anything, after Boris's five answers:
- Boris: "138 look is an effect preset. change the name look to preset to avoid further confusion." -> LB1.
- Boris: "133 no need for any of that. It can be called look 2 but no need to show that it was changed. Effects are usually
  changed by the user." -> LB2.
- Boris: "131 is the signal plugged into a slider in a clip in the show, or is it a look which is an effect preset in the
  effect library that can also be connected to a signal. If this is a clip in the show, then ctrl-z brings it back. There is no
  other way. If it is a look, there is no way to remove the signal unless the user drops it into the show (clip, layer or
  global) and then adds a signal and saves that look." -> LB3 (read as A, reading R88).
- Boris: "132 default good" -> LB3. Boris: "134 delete them too" -> LB4.
The delta adds no feature. It changes: every word (LB1); the one text rule of the button, and with it where the loaded
preset's name is kept (LB2); nothing of 131 / 132 (LB3); three sentences of the docs and of his checks (LB4).
IN ONE SCREEN, what is ruled here:
(1) Everything he sees or reads says "preset". On disk: `~/Library/Audio-DNA/Effect Presets/<effect>/<name>.preset.json`,
    "format": "audio-dna-effect-preset". In code, routes and keys the thing is always an EFFECT preset (EffectPreset...,
    /api/debug/fx_preset..., "fxPreset"), never a bare "Preset...", because that word is taken twice (MilkDrop; the old
    PresetManager).
(2) The button reads the name of the preset last loaded, made or saved over on that effect -- whatever the sliders are now, in
    the normal colour, with no mark. An effect with no such preset reads the preset its settings equal ("Default" for a new
    effect), else "Presets", dim. The menu ticks the preset the button names; clicking it puts the effect back on it.
(3) Because the name is now remembered and no longer derived, it is SAVED WITH THE SHOW (one optional key per effect,
    "fxPreset"; RA's HD-14 is built, in S2). Otherwise nearly every name would vanish each time a show is re-opened.
(4) 131 A and 132 A are the rulings' defaults: no line of code changes. One sentence of his 131 points at presets in the
    effect list (138 B): not built, costed in one paragraph.
(5) Stages stay S1a, S1b, SE, S2, S3, VG. 83 new unit cases (81 + 2), eleven live rows (ten + 1), 24 visual states (22 + 2).
(6) Three questions for him, 150-152, each with a default; nothing waits.

---------------------------------------------------------------------------------------------------------
## 2 ESTABLISHED FACTS (only verified lines; each with file:line or the record's line)
---------------------------------------------------------------------------------------------------------
The record
- E1 His five answers: BD:988-996 (131-134) and BD:1014-1016 (138), VERIFIED. As asked: .harmony/.reports/s-rta-1004/
  boris-clarify-131-134.md (questions verbatim from RA section 7, his lines, Harmony's reading "131 A (reading R88; new
  question 138); 132 A; 133 not a letter (the name stays, no mark); 134 B (done 2026-10-04 21:01:09)") and
  .harmony/.reports/s-rta-1004b/boris-clarify-135-143.md (138 as asked; readings R88 and R95), VERIFIED.
- E2 R88, told and not corrected: "Question 131 is your first case ... the default A ... Your second case is as planned: a look
  is changed only by putting it on an effect in the show, changing that, and saving the look." R95, told with the answer to 138
  and not corrected at the time of writing: the "Presets" button, "Preset 2", 'Save over "Preset 2"'; a preset is "not dragged
  from the effect list". Both stand by INFERRED consent (section 3 says which lines move if he corrects one).
- E3 Earlier words that still bind (BD:925-933): 102 B; 103 -- Boris: "For a default behavior I will want to create a look 3
  when I change look 2 and save a differnt version of look 2. look 2 remains unchanged but I need to have a way to save look 2
  if I tweak it a little."; 104 -- Boris: "open a name box but with default name look x that can easily be changed". 138
  REPLACES the word "look" in them and nothing else (BD:1015-1016: "the rules themselves stand" is Harmony's consequence text).
- E4 One Save (BD:874) -- Boris: "All of these things should be saved when a show is saved. There's no reason to save them
  separately:" (said of his list of nine saves). Used in LB2 as a pointer, labelled INFERRED there.
- E5 134: the folder ~/Library/AudioDNA/Presets/fast_saves was moved to the Trash by Harmony 2026-10-04 21:01:09
  (.harmony/s-rta-1004b-work.md:8, VERIFIED as a record; the disk was not read). Left: "test 1.deck.json"
  (plan-looks-answers.md:678).
- E6 The adoption blocks (plan-effect-looks.md:619-669; plan-looks-answers.md:642-700) were read whole, VERIFIED. The newest
  (21:33:30) says: the WHOLE lane waits for this delta; "(1) may change a stored name".
The code today (all VERIFIED at 7bc6df7)
- E7 Nothing of the lane exists: `grep -rnE '\.preset\.json|Effect Presets|fxpreset' src tests docs/claude` prints nothing;
  `EffectStackView::refresh()` (src/ui/EffectStackView.cpp:188-219) sets the bypass colour and the slider values and repaints
  -- no preset button.
- E8 `Clip::EffectSlot` (src/model/Clip.h:60-114): effectName, paramValues, dryWet, enabled, bypassed, paramConns, paramLive,
  dryWetConn, dryWetLive, three helpers, effParam / effDryWet. No name member.
- E9 An effect slot is written and read in THREE places, each with its own loop: clip effects src/model/Clip.cpp:100-128
  (write; "name", "enabled", "bypassed", "dryWet", "params", "conns", "dryWetConn") and :286-306 (read); layer effects
  src/model/Layer.cpp:96-115 and :229-249; global effects src/model/Composition.h:817-836 and :1077-1097 (the Clip.cpp write
  loop was read whole; the other five places by their "bypassed" / "dryWetConn" lines). Keys are read with `hasProperty`
  (Clip.cpp:305).
- E10 "preset" is in a model key or member already: Layer.cpp:87 / :216-217 ("feedbackPreset", `feedback.presetName`);
  Clip.cpp:154 / :327 ("presetPlaylist"); Composition.h:786, :796 ("autoPresetOnGenre", "genrePresetNames");
  src/model/AppSettings.h:18 (`kMilkDropPresetDir`); src/sources/ProjectMPresetManager.cpp:105 ("userPresets").
- E11 Types with the word: `class PresetManager` (src/ui/PresetManager.h:33, the old one); `class ProjectMPresetManager`
  (src/sources/ProjectMPresetManager.h:9), held by MainComponent as `presetManager_` (src/MainComponent.h:315);
  `class PresetSelector` (src/sources/PresetSelector.h:11, MilkDrop's); `struct FeedbackPreset`
  (src/render/FeedbackProcessor.h:10); `struct Preset` (src/ui/CanvasSizeCombo.h:14, a canvas size).
- E12 "preset" ON SCREEN today, every place (grep of string literals over src; 20 files hold the word in a literal, the rest
  are keys, includes and stderr):
  MilkDrop -- src/ui/MilkDropBrowser.cpp:20-23 ("No presets loaded." and how to add them), :101 and :122 ("<n> presets"),
  :283 ("No recently used presets."), :456 ("Search presets..."); src/ui/MilkDropBrowser.h:120 ("Select presets, drag to
  cell"); src/ui/PreferencesDialog.h:74 ("MilkDrop Presets:"), PreferencesDialog.cpp:96 ("Path to .milk preset folder..."),
  :106 ("Select MilkDrop Preset Directory").
  Feedback -- src/ui/LayerInspector.cpp:408-415: a combo ("Custom" + the feedback presets' names) whose TOOLTIP is "Feedback
  preset"; no visible label holds the word.
  The old Save / Load -- src/MainComponent.cpp:2968 ("Save preset..."), :2996 ("Load preset..."), :3035-3036 ("Legacy
  Preset", "This preset was saved before ..."), inside `savePreset()` (:2965) and `loadPreset()` (:2993); the buttons
  themselves read "Save" and "Load" (src/MainComponent.h:346-347).
- E13 The MilkDrop list is one tab of the browser panel (src/ui/BrowserPanel.h:56 `milkDropTabBtn_{"MilkDrop"}`; one tab's
  content is visible at a time, BrowserPanel.cpp:149-154); the effect list is another tab ("FX", BrowserPanel.h:48).
- E14 "look" ON SCREEN today, in another meaning (the picture a take leaves): src/ui/RecordPanelModel.h:136, :244, :252, :263
  ("holding the last look"), :189 ("The look stays as it is."); src/MainComponent.cpp:5983 ("the look from when Record was
  pressed is restored first."), :6051 ("holding the last look.").
- E15 The app's own folder is spelled two ways today: "Audio-DNA" (src/MainComponent.cpp:75, :5829, :7237; settings.json per
  docs/claude/integration.md:29) and "AudioDNA" (src/ui/PresetManager.cpp:449, :458, :467; src/ui/CompDecksBrowser.cpp:325,
  :331; src/ui/FilesBrowser.cpp:620).
- E16 Test routes that exist: POST /api/debug/save_composition (src/api/ApiServer.cpp:328; its body was not read);
  POST /api/load_milkdrop_preset (src/test/TestServer.cpp:172).
The one-save lane
- E17 OS:514, its stage "S7 the old look buttons": row 1, the slot bar, "the seven functions", src/ui/PresetManager.h/.cpp
  deleted, tests/test_preset_manager.cpp deleted, docs; proved by LINT-6 (OS:640: "src names no `fastSave`, `presetSlots_`,
  `FX Save`, and no token `PresetManager`") and a grep of the token `PresetManager` "(not `ProjectMPresetManager`)".
  VERIFIED as text. NOT VERIFIED: that "the seven functions" include `savePreset()` and `loadPreset()` (E12) -- LINT-P2-2
  (section 5) is the check.
- E18 OS:513, its S1 edits src/model/Composition.h (version first; `saveToFile(file, extras)`). VERIFIED as text. No row of
  OS's stage table names src/model/Clip.cpp or Layer.cpp (grep, nothing).
From the rulings (RULING; not re-read by me)
- E19 RA F5 / F6 (the hook), F8 (no code makes a signal), F12 (`EffectSlot` is never brace-initialised; under src/render effect
  vectors are const references; a std::string member last in the struct, written on the message thread, adds no new hazard),
  F13-F16 (JUCE), RU's effect-header facts. Every rule of RA / RU that this delta leaves standing keeps resting on them.

---------------------------------------------------------------------------------------------------------
## 3 ITEMS
---------------------------------------------------------------------------------------------------------
### LB1 THE RENAME (138)
VERIFIED: E1 (his words), E7 (nothing built, nothing on his disk -- the second half is Harmony's statement in the dispatch and
in plan-looks-answers.md:692-693; his disk was not read), E10-E14 (where the word already lives).
Harmony constraint (the rename): every on-screen and documented "look" becomes "preset"; the list must be complete; every
stored name is ruled keep or change.

P2-1 THE STRINGS HE SEES (complete; old -> new; a text not listed here does not exist in the lane)
| where | old (RU / RA) | new |
|---|---|---|
| the button, no preset to name | "Looks" (secondary colour) | "Presets" (secondary colour) |
| the button, the built-in | "Default" | "Default" (stays) |
| the default name in the box | "Look N" ("Look 1", "Look 2", ...) | "Preset N" ("Preset 1", "Preset 2", ...) |
| menu | "Default" | "Default" (stays) |
| menu | "New Look" | "New Preset" |
| menu | `Save over "<look>"` | `Save over "<preset>"` (the form stays; the name inside is the preset's) |
| menu, nothing to save over | "Save over" (greyed) | "Save over" (stays) |
| menu | "Rename", "Delete" | "Rename", "Delete" (stay) |
| name box, new: title / buttons | "New Look" / "Save Look", "Cancel" | "New Preset" / "Save Preset", "Cancel" |
| name box, rename: title / buttons | "Rename Look" / "Rename", "Cancel" | "Rename Preset" / "Rename", "Cancel" |
| Save Over window: title | "Save Over Look" | "Save Over Preset" |
| Save Over window: sentence | `Replace look "<name>" of <effect> with the settings on now? This cannot be undone.` | `Replace preset "<name>" of <effect> with the settings on now? This cannot be undone.` |
| Save Over window: buttons | "Save Over", "Cancel" | stay |
| Delete confirm: title | "Delete Look" | "Delete Preset" |
| Delete confirm: sentence | `Delete look "<name>" of <effect>? This cannot be undone.` | `Delete preset "<name>" of <effect>? This cannot be undone.` |
| Delete confirm: buttons | "Delete", "Cancel" | stay |
| Edit menu (the command's description) | "Load Look '<look>' on '<effect>'" (shown as "Undo Load Look ...") | "Load Preset '<preset>' on '<effect>'" |
| tooltips | none (RA:246 "No tooltip") | none; none is added |
| names he may not give a preset (`isLegalName`) | Default, Looks, New Look, Save over, Rename, Delete | Default, Presets, New Preset, Save over, Rename, Delete |
| the folder he is told about (his check 14) | Library / Audio-DNA / Looks | Library / Audio-DNA / Effect Presets |
No other text is shown by the lane: no message, no count, no badge (LINT-EL-6 stands).

P2-2 THE STORED NAMES (each: keep or change, and why; nothing has shipped, nothing of the lane is on his disk)
| stored name | RU / RA | ruled | reason |
|---|---|---|---|
| root folder | `~/Library/Audio-DNA/Looks/` | CHANGE -> `~/Library/Audio-DNA/Effect Presets/` | He is told this folder and copies it by hand (check 14): it is on-screen text in the Finder. "Effect Presets", not "Presets": `~/Library/AudioDNA/Presets/` (E15, the old PresetManager's folder, still holding "test 1.deck.json") must not get a near-twin, and MilkDrop keeps a preset folder of its own. |
| per-effect folder | the effect's display name through `folderNameFor` | KEEP | no word in it |
| file extension | `.look.json` | CHANGE -> `.preset.json` | he sees the files in that folder; `.preset.json` exists nowhere in the app (E7); MilkDrop's files are `.milk` |
| Save over's new-file name | `.<name>.look.json.new` | CHANGE -> `.<name>.preset.json.new` | follows the extension; still starts with a dot and still does not end in the listed extension (ST-14) |
| "format" value | "audio-dna-look" | CHANGE -> "audio-dna-effect-preset" | documented in effects.md; a file with any other value is refused whole as today (LK-7). No reader for the old value: no such file exists. |
| "version" (2), "signals", "effect", "dryWet", "params", "uniform", "label", "value", "conn", "dryWetConn" | as RA | KEEP | none holds the word |
| the first format (no "signals": values only) | readable (LK-17, GL-2 "Hand") | KEEP readable, under the new extension and format value | the rule "a file that does not speak about signals touches no signal" is what a hand-written file relies on; removing it would be a change of behaviour, not of a word |
| the test-mode variable | AUDIODNA_LOOKS_DIR | CHANGE -> AUDIODNA_EFFECT_PRESETS_DIR | documented in docs/claude/testing-eyes.md |
| settings.json | no key | none | the lane still uses no settings key |
| the show | no key | NEW optional key "fxPreset" on an effect entry (LB2, P2-6) | -- |
| test routes (documented) | /api/debug/looks, look_make, look_load, look_replace, look_ui | CHANGE -> /api/debug/fx_presets, fx_preset_make, fx_preset_load, fx_preset_replace, fx_preset_ui | documented; "fx_" because /api/load_milkdrop_preset exists (E16) |
| route fields | `looks`, `menu.newLook`; request field "look" | CHANGE -> `presets`, `menu.newPreset`; request field "preset" | documented |
| route fields | `matched`, `loaded`, `menu.saveOver`, `menu.rename`, `menu.delete`, `button`, `box`, `confirm`, `live`, `dryWetLive`, `conn`, `folder`, `root`, `dirReads`, `focusHome` | KEEP | none holds the word |
| `fx_preset_ui` actions | pick:<menu item text>, rename:<name>, delete:<name>, renamelist, deletelist, menu, expand, fold, dismiss, type:, accept, accept:, cancel, return | KEEP; "pick:New Look" is now "pick:New Preset" because it carries the menu's own text | -- |
| ui_text keys | neither ruling defines a key that holds the word (GL-1 reads the existing file label) | none | -- |
| the probe | .harmony/probe-effect-looks.sh | CHANGE -> .harmony/probe-effect-presets.sh | named in docs and in every gate row |
| row, mutant and lint ids | LK-, ST-, LC-, LM-, CE-, GL-, V-, MU-EL-, MU-LA-, LINT-EL-, HD- | KEEP | ids, never on screen; they tie this paper to the two rulings |

P2-3 THE WORD IS TAKEN: THE NAMING RULE IN CODE (Harmony constraint: the word is taken)
- In code, in file names, in routes and in keys the thing is an EFFECT preset. Files (they are in the stage table, so they
  are ruled): src/effects/EffectPreset.h, src/effects/EffectPresetStore.h / .cpp, src/core/EffectPresetCmd.h,
  src/ui/EffectPresetsButton.h, src/ui/EffectPresetsMenu.h, tests/test_effect_preset.cpp,
  tests/test_effect_preset_store.cpp, tests/test_effect_preset_cmd.cpp, tests/test_effect_presets_menu.cpp. Catch tags:
  [fxpreset], [fxpresetstore], [fxpresetcmd], [fxpresetmenu].
- Identifiers that a pre-registered string names (ruled): `EffectSlot::fxPresetName` (was `lookName`; NOT `presetName`,
  which Layer.cpp already uses for the feedback preset, E10); `EffectPresetStore::presetsFor` (was `looksFor`; ST-9);
  `onPerformPreset` / PerformPreset (LM-7); `effectPresetsDir` (was `effectLooksDir`; LINT-EL-6). Every other identifier is
  the builder's (the papers write the namespace as `fxpreset::`), under two rules: no class, struct, file or namespace named
  bare `Preset...` and none named `PresetManager` -- that name was the old whole-chain saver's and is removed by one-save S7;
  it does not come back for a different thing.
- LINT-P2-1, LINT-P2-2, LINT-P2-3 (section 5) are the proof.

P2-4 WHERE AN EFFECT'S PRESETS AND THE OTHER PRESETS MEET ON SCREEN (VERIFIED: E12, E13)
| the two together | when | reads ambiguously? | ruled |
|---|---|---|---|
| the MilkDrop tab ("Search presets...", "<n> presets", "Select presets, drag to cell", "No recently used presets.", "No presets loaded.") and an effect's "Presets" button, its menu and its windows in the inspector | whenever the browser shows the MilkDrop tab and the inspector shows a clip, a layer or the composition with an effect | Each is bare "preset(s)", but each sits inside its own frame: the tab named "MilkDrop", and the header row of one named effect. The lane's two destroying windows name the effect in their sentence (`... of <effect>`). The name box ("New Preset" / "Save Preset") names no effect; it opens only from that effect's own menu, directly under his click. | No text of this lane changes. The MilkDrop tab's own words are a product choice: question 151 (default: leave them). |
| a MilkDrop clip that carries clip effects: each effect header shows "Presets" | his clip inspector on such a clip | No: the button is on the effect's row beside the effect's name; the MilkDrop preset is the clip's name / thumbnail, not a control in that row. | nothing |
| the layer inspector: the feedback combo (tooltip "Feedback preset"; the box shows "Custom" or a feedback preset's name, never the word) above the layer's effect stack with "Presets" buttons | every layer inspector | No: the only place the feedback one spells the word is its tooltip, and it is qualified. | nothing; V-11's frame includes the combo so the critics see both |
| Preferences: "MilkDrop Presets:" | the Preferences window (modal) | No: qualified. | nothing |
| the old "Save preset..." / "Load preset..." file windows and "Legacy Preset" (E12) | never, if one-save S7 has removed `savePreset()` / `loadPreset()` | YES if they survive: a bare "preset" that means a whole effect chain. | LINT-P2-2 on the lane's base: if it prints a line the lane does not start and Harmony is told -- the fix is the one-save lane's, not this lane's. |
| the Edit menu's "Undo Load Preset '<preset>' on '<effect>'" | after any load | No: it names the effect. No undo text for a MilkDrop preset exists (grep of "Load MilkDrop" / "Load Preset" literals: nothing). | nothing |
THE RULE FOR EVERY LATER TEXT, written into docs/claude/effects.md by S2: "On screen the bare word 'preset' belongs to an
effect, and only on that effect's own row, menu and windows. Every other preset is written with its first word: MilkDrop
preset, feedback preset."
NOT RENAMED, and said so: the record panel's "holding the last look" / "The look stays as it is." / "the look from when
Record was pressed is restored first." (E14). There "look" is the picture a take leaves, not an effect preset; "holding the
last preset" would be false. His words -- Boris: "change the name look to preset to avoid further confusion." -- are about
the effect preset (INFERRED from the sentence before it: "look is an effect preset"). Those files belong to the recording
lane, which his answers of 21:03:09 and 21:33:30 re-model; the finding is handed to Harmony for that lane (section 9).

P2-5 EVERY PRE-REGISTERED STRING OF BOTH RULINGS THAT HOLDS THE WORD is re-stated in section 5 with its new text: 48
unit-case names (one of them, LM-3, re-cut by LB2), two failure texts (LK-13, CE-4), the ST-9 info line, the gate lines
of GL-2, GL-3, GL-5, GL-6, GL-7, GL-9 (two) and GL-10, three lints (LINT-EL-1, -3, -7), the fixtures, the visual states. A unit
case's NAME is not on screen, but it is copied into packets and into GL-8's count, so each one is written out; no builder
substitutes a word himself. Mutant DESCRIPTIONS (RU 5.5, RA 5.5) read with the same words; the CHANGE each one makes is the
same, except MU-EL-17, which is retired (LB2).
FORKS.
- F-LB1-a the root folder: "Effect Presets" (chosen) vs "Presets". "Presets" loses on E15: a second `.../Presets` folder one
  hyphen away from the old one, and silent about which of the app's three kinds of preset it holds.
- F-LB1-b the extension: `.preset.json` (chosen) vs `.fxpreset.json`. The longer one loses: inside a folder named "Effect
  Presets / Ripple" the short one is already unambiguous, and he reads these names in the Finder.
- F-LB1-c ids and row prefixes: keep (chosen) vs re-letter (LK -> PK ...). Re-lettering loses: it would break every
  cross-reference into two adopted rulings for no text he or a builder sees.
- F-LB1-d test routes: rename with "fx_" (chosen) vs keep "look" routes as builder identifiers. Keeping loses: the routes are
  documented in testing-eyes.md, and Harmony's constraint covers documented words.
Cheapest refuting test for the whole item: LINT-P2-1 on the lane's final diff, RED on one seeded line.

### LB2 THE BUTTON AFTER A CHANGE (133)
VERIFIED: E1 (his words; not a letter), E8, E9 (three serializers), RA:257-276 (RA-1), RA:440-443 (RA-14), RA:809-811 (the
"133 B" lines), RU:324-331 (AM-7), RU:532 (LM-3) and RU:554 (MU-EL-17, "the button remembers the last loaded name").
Boris: "133 no need for any of that. It can be called look 2 but no need to show that it was changed. Effects are usually
changed by the user."

P2-6 THE RULE (replaces: AM-7's sentence "the name `firstMatch` gives, in the primary text colour; else "Looks" in the
secondary colour"; RA-1's two bullets on `shownMatch` driving the button and the tick and its bullet "On screen, said to the
critics ..."; RA-14 whole; RA section 7's "133 B" lines; LM-3 and MU-EL-17; RA R9)
- The loaded preset of a slot = the preset of this effect whose name is EXACTLY `fxPresetName`, if the store lists one. Else
  the slot has no loaded preset (RA-1's definition, unchanged).
- `fxpreset::buttonState(def, slot, presets) -> {text, dim}`, pure, in src/ui/EffectPresetsMenu.h:
  (1) the slot has a loaded preset -> its name, not dim -- whatever the values and the signals are now;
  (2) else `firstMatch` names something (Default, or one of his presets) -> that name, not dim;
  (3) else "Presets", dim.
- "No mark" is pinned: in case (1) the text, the colour and the triangle are the same whether the settings equal the preset
  or not. Dim is used for case (3) only.
- The menu's one tick is the item the button names (LM-1's words already say so); in case (3) nothing is ticked. Clicking the
  ticked preset after a change loads it again -- the effect is put back on the preset, one undo step (AM-6's rule "a pick
  that would change no value pushes no undo step" already separates the two cases).
- `shownMatch` stays, for ONE reader: the field `matched` of GET /api/debug/fx_presets, which GL-9 and GL-10 pre-register.
  It is no longer on screen.
- "Loaded" is written by, and only by (RA-1's writers, plus one): (1) `EffectPresetCmd` -- execute and redo set the name,
  undo puts the earlier one back, "Default" sets it empty (LC-13); (2) New Preset and Save over, after the store has verified
  the file; a Rename from this row carries the name; (3) once per row, when the view builds the row, for a slot with no
  loaded preset whose settings equal one of his presets; (4) NEW: the show's reader (P2-7). No timer writes it (MU-LA-32
  stays a mutant).
- What the menu offers after a change, unchanged from RA: "Default"; his presets, the loaded one ticked; "New Preset" -- live
  when the settings equal no preset, Default included; its box holds the next free "Preset N" (Boris, 103 and 104, E3);
  `Save over "<the loaded preset>"` -- live when the settings differ from it and this build may rewrite it; "Rename";
  "Delete".
- MAY AN EFFECT FAR FROM THE PRESET'S VALUES CARRY ITS NAME FOR EVER? Yes. That is his answer: the name says which preset was
  last put on this effect, not what the effect holds now. No distance ends it -- a threshold would be the mark he declined.
  It ends only by his own act: another preset or "Default" is loaded; New Preset; an undo across the load; or the preset no
  longer exists under that name (deleted, or renamed from another effect's row: question 152).
- AN EFFECT THAT WAS NEVER GIVEN A PRESET reads "Default" while it is at its defaults and "Presets", dim, once a slider
  moves (case 2 -> case 3), and "Default" again if it is brought back. "Default" is a description, not a name he gave: on a
  changed effect it would be false. This is the one place where the button still follows the values; it is his to overrule:
  question 150 (B = "Default" is kept like a name).

P2-7 THE NAME IS SAVED WITH THE SHOW (RA's HD-14 is BUILT, in S2; replaces RA-1's "never saved", RA section 4's "it adds ONE
runtime member ... and no key to the show", RU:461-462 "This lane adds NO key to the show and does not touch Clip.cpp,
Layer.cpp or Composition.h", LINT-EL-7 as worded, and the pitfall text's "(... runtime, message thread, never saved)")
- WHY, under his words. Before 133 the name was derived from the values, so a re-opened show showed it again by itself. Now
  it is remembered. He says: "Effects are usually changed by the user." So nearly every effect that carries a preset's name
  has been changed -- and after a re-open every one of them would read "Presets": the name that, in his words, "can be called look 2"
  would last until the next launch. RA said the same of its own option (RA:810-811: "the mark is gone after a show is
  re-opened unless HD-14 is built with it"; RA:824-825: "it is taken up at once"). And what an effect in a show is called is
  part of that show (INFERRED from E4; he did not say it of this name).
- The key: "fxPreset": "<name>", on the effect's own entry in the show, written by the three slot writers (E9) ONLY when the
  name is not empty, read by the three slot readers when present (`hasProperty`, the files' own idiom). A show with no loaded
  preset is byte for byte what it is without this lane -- so no baseline, checksum or fixture of the one-save lane moves.
  No show version is raised (INFERRED from the files' idiom that an absent optional key keeps the default; the one-save S1
  version rule was not read: section 8 R6).
- The name is kept in the slot and written back even when no preset of that name lists here (another computer; a deleted
  preset): it is SHOWN, ticked and offered for Save over only while one lists. A name read from a show is only ever COMPARED
  with listed names; it never builds a path (Save over resolves the file through the store's own list entry, RA-9 step 1).
- Threads: the member is a std::string written on the message thread; the show is written from MainComponent's save paths
  (src/MainComponent.cpp:3551, :3574, :6694 -- message thread, INFERRED: no thread is started around them) and a load builds
  a fresh composition (Pitfall 58). The renderer never reads it (LINT-EL-7). No lock, no new mutex.
- Rows: LC-14 (new), LM-24 (new), GL-11 (new). Mutants MU-P2-3, MU-P2-4, MU-P2-5, MU-P2-6.

FORKS.
- F-LB2-a what wins on the button: the loaded name first (chosen) vs "a match first, the loaded name when nothing matches"
  (RA's own 133 B line, RA:809). The runner-up loses three ways: the name would change by itself while a slider is dragged
  through another preset's settings (the refresh runs about 10 times a second -- the very case RA-1 was written against);
  RA R9's oddity stays (the button says one preset, Save over another); and his sentence is "It can be called look 2".
  Refuting test: LM-22 ("loading B makes the button ... read B") and GL-10 step (c).
- F-LB2-b across a re-open: the name is saved in the show (chosen) vs the run only. The runner-up is smaller (no serializer
  is touched) and loses on the paragraph above: with "usually changed", a name that a re-open forgets is a name he sees for
  one evening. Refuting test: GL-11's RED arm, which IS the runner-up's behaviour.
- F-LB2-c an effect that was never given a preset and is changed: "Presets", dim (chosen) vs it keeps "Default" (question 150
  B). B loses as the default because almost every effect in a show would then read "Default" while not being at its
  defaults; it is one clause away (what changes: LC-13's last clause -- loading Default sets the name "Default"; writer (3)
  also takes Default; LM-3's last clause; V-3).
- F-LB2-d a name no preset lists: kept in the show, not shown (chosen) vs shown anyway vs dropped at load. "Shown anyway"
  loses: the menu would have nothing to tick and Save over nothing to replace. "Dropped at load" loses: opening a show on a
  second computer and saving it there would strip every name for good.
- F-LB2-e when a preset is renamed from one effect's row, the other effects that carry the old name: left as they are
  (chosen; RA-1's rule) vs every effect of the open show is walked and re-named. The walk is a new write path over the whole
  show outside the undo stack; it is his to ask for: question 152.

### LB3 131 AS A, 132 A -- THE RULED DEFAULTS AGAINST HIS WORDS, LINE BY LINE
Harmony constraint: 131 is READ as A (R88, told to him and not corrected); B stays the one constant RA names.
| his words | the rulings | verdict |
|---|---|---|
| "131 is the signal plugged into a slider in a clip in the show, or is it a look which is an effect preset in the effect library that can also be connected to a signal." | Question 131 is about an effect in the show (a clip's, a layer's or a global one) onto which a preset is loaded: his first case. | He asks which; R88 answers. No rule. |
| "If this is a clip in the show, then ctrl-z brings it back." | RA-10: `kUnwiredEntry` = Unplug; one Cmd+Z puts values AND signals back (LC-9, LC-10, GL-9 steps d-e). | AGREES. "ctrl-z" is read as the app's Undo, Cmd+Z on his Mac (R88 says Cmd+Z; INFERRED). |
| "There is no other way." | Nothing else brings an unplugged signal back: no "keep signals" switch, no B policy on screen, no prompt. Redo, plugging it in again by hand, and loading a preset that holds it are not ways back, they are new acts. | AGREES. `UnwiredEntry::Keep` stays built and unit-tested (LK-24, MU-LA-37) as the one constant of Harmony's constraint; it is reachable from no screen. GL-9's "answer 131 B" line stays pre-registered and DORMANT. |
| "If it is a look, there is no way to remove the signal unless the user drops it into the show (clip, layer or global) and then adds a signal and saves that look." | A preset file changes in exactly one way: Save over, from an effect in the show (RA section 0 (3)); New Preset writes a new file from an effect in the show; Rename changes no byte; Delete removes the file. No preset editor, no route that edits a preset's signals without an effect (`fx_preset_replace` takes a slot). All three hosts are served (GL-2: 3/3). | AGREES for "only from an effect in the show" (R88's second sentence; INFERRED consent). |
| "... a look which is an effect preset in the effect library ..." and "drops it into the show" | No preset is listed in the effect list and none can be dragged: a preset is picked from the button of an effect already in the show. | THE ONE PLACE HIS TEXT AND THE RULINGS DIFFER. Asked as 138; no letter picked; read as A (R95). Not built. If he corrects R95: the paragraph below. |
| "132 default good" | RA-2: a window asks; only a click on "Save Over" saves; Return and Esc cancel. | AGREES. RA's "132 B" line and HD-15 (a hidden copy) are closed: not built. |
What a correction would move: if he corrects R88 to B, ONE line (`kUnwiredEntry`), and GL-9's dormant line becomes the live
one (RA:802-805). If he corrects R95, the lane as stated here still ships first and 138 B is its own stage.
Lines of the two rulings his 131 text makes stale (words only): RA section 6 item 7 (c) "(question 131)" and RA-10's last
bullet "Harmony asks question 131 WITH the adoption" -- the question is answered; RA:716 "If Boris answers 133 B: one more
state"; RA HD-18. No rule changes.
One thing he must be told, because "ctrl-z brings it back" is true only while that step is still in the undo history: after
the show is closed, the unplugged signal is gone unless it is kept in a preset (section 6 item 7).
138 B IN ONE PARAGRAPH (costed, not designed). Listing presets under each effect in the FX tab and dragging one onto a clip,
a layer or the global stack needs: the FX list to read the store for every effect it shows (today a folder is read the
first time that one effect's presets are asked for -- ST-9's "once" and `dirReads` would be re-cut); rows that open and
close under an effect in that list; a new drag description beside "fx:" that the four drop targets and the clip cells
accept (src/ui/EffectStackView.cpp:470, ClipCell.cpp:519, the three inspectors, LayerStrip.cpp:1143); "add the effect AND
load the preset" as ONE undo step, which is a new command; the list to follow New Preset, Rename, Delete and Save over;
and its own rows, live rows and visual states. About one more build stage and one more visual round. Nothing of it is in
this lane.

### LB4 134 -- DONE, NOTHING TO BUILD
VERIFIED: E5. Boris: "134 delete them too". Harmony moved ~/Library/AudioDNA/Presets/fast_saves to the Trash on
2026-10-04 21:01:09.
What must no longer be said, anywhere this lane writes:
- his check 17 (RA section 6) -- "Still on your disk ...: the nine quick FX saves and one deck file ... (question 134)" is
  replaced by section 6 item 17 below;
- RA-15 ("Whether the nine quick FX saves go too is question 134 (default: they stay)"), RA's question 134 and its "134 B"
  line, RA F19's "LEFT ALONE ...: Presets/fast_saves (9 entries ...)": closed, stale;
- no doc of this lane names fast_saves, "FX Save", the ten slots or "quick saves" as something that exists (the removal and
  its docs are one-save S7's; LINT-EL-5 stands).
Still his to hear, once: "test 1.deck.json" in Library / AudioDNA / Presets is still on the disk; RA's HD-20 hands it to the
one-save lane.

### LB5 STAGES -- which stage the rename and LB2 touch (the table is section 4)
| stage | the rename (LB1) | the button (LB2) |
|---|---|---|
| S1a | file names; "format" value; `isLegalName`'s words; LK names; LK-13's failure text | nothing (`shownMatch` stays; the button's rule lives in S3's file) |
| S1b | the root folder, the extension, the new-file name, `nextName` ("Preset N"), the test-mode variable; ST names | nothing |
| SE | CE-4's failure text only | nothing |
| S2 | routes and fields, the command's description, `effectPresetsDir`, the probe, the docs, the pitfall text | `fxPresetName` and the show key "fxPreset" (three writers, three readers); LC-14; the docs' sentence on the name |
| S3 | every on-screen text of P2-1; LM names | `buttonState`; LM-3 re-cut; LM-24; the tick |
| VG | every state is captured with the new words | V-3 and V-6 re-captured; V-23, V-24 new |
The fence with one-save S7: section 4.

### LB6 WHAT BORIS CHECKS, AND HIS QUESTIONS: sections 6 and 7.

---------------------------------------------------------------------------------------------------------
## 4 STAGES + ORDER (one builder context per stage; Harmony runs every live row and gives every gate verdict, never a builder)
---------------------------------------------------------------------------------------------------------
One lane, one worktree (the branch's name is Harmony's; `lane/effect-presets` is suggested). The rules of RA section 4 stand:
each stage is ONE builder context, ends with the full ctest green, shows its own unit rows RED first (the named mutant) and
then GREEN, and is reviewed pinned before the next starts; a builder never launches the app, never runs a live row, never
gives a verdict; docs move in the stage that changes the behaviour. This table REPLACES RA's stage table (RA:490-497). What
a cell does not say is as RA's cell says it, with LB1's names.

| stage | owns (files) | proves (unit, by the builder) | Harmony runs herself, and when |
|---|---|---|---|
| S1a the preset (headless) | NEW src/effects/EffectPreset.h (the preset with a connection per entry; `capture`, `resolve` with the policy, `kUnwiredEntry`, `matches`, `firstMatch`, `shownMatch`, `sameConn`, `connVarReadable`, `isLegalName`, `nameVerdict`, `toVar` / `fromVar`); NEW tests/test_effect_preset.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp). Touched ONLY as mutants, reverted: src/effects/EffectLibrary.cpp (MU-EL-28), src/connect/ConnSerialization.cpp (MU-LA-33). | LK-12 first, then LK-1..LK-24; LINT-EL-4; LINT-P2-1, LINT-P2-3 on the stage's diff; their mutants RED | BEFORE S1a, on the lane's base: LINT-P2-2 (the fence with one-save S7), and `ctest -N` (N0 of GL-8). After S1a: nothing live; the pinned review. |
| S1b the store (headless; after S1a) | NEW src/effects/EffectPresetStore.h / .cpp (`listFolder`, `presetsFor`, `make(effect, name, preset)`, `nextName`, `pathTaken`, `rename`, `remove`, `replace`, `canReplace`, `beforeSwapForTest`, `defaultRoot`, `testModeRoot`); CMakeLists.txt (one source); NEW tests/test_effect_preset_store.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp) | ST-1..ST-17; LINT-EL-1, LINT-EL-4; LINT-P2-1, LINT-P2-3; their mutants RED | Nothing live; the pinned review. |
| SE the wire that drives nothing, and `tickSlot` (as RA: shares no file with S1a or S1b; any order before S2) | src/connect/ConnectionEngine.h / .cpp; tests/test_connection.cpp (four cases appended); docs/claude/rendering.md (two sentences); as a mutant only: src/signal/SignalRegistry.cpp (MU-LA-29) | CE-1..CE-4; LINT-P2-1; their mutants RED | After the pinned review: the regression probes of RU, GREEN only. |
| S2 the load, undo, the host, the name in the show, the data routes, the probe (after S1b AND SE; and after one-save S1 has merged, see the fence) | NEW src/core/EffectPresetCmd.h; src/model/Clip.h (ONE member, `fxPresetName`, last in `EffectSlot`); src/model/Clip.cpp, src/model/Layer.cpp, src/model/Composition.h (in each: ONE write line and ONE read line for "fxPreset", inside the effect loops of E9, nothing else); src/MainComponent.h / .cpp (the store member, `effectPresetsDir`, `performPresetLoad` with the hook of RA-7, the route callbacks); src/api/ApiServer.h / .cpp (inside the test-only block: GET /api/debug/fx_presets, POST fx_preset_make, fx_preset_load, fx_preset_replace); NEW tests/test_effect_preset_cmd.cpp; tests/CMakeLists.txt; NEW .harmony/probe-effect-presets.sh with its fixtures and a selftest (rows GL-1, 2, 3, 4, 6, 7, 9); docs: docs/claude/effects.md (new section "Effect presets", with the rule of P2-4 and the sentence on the name of P2-6 / P2-7), docs/claude/pitfalls.md (entry NN, text below), docs/claude/testing-eyes.md (the routes, the variable), docs/claude/architecture.md (the new files; `EffectSlot::fxPresetName`; the show key) | LC-1..LC-14; LINT-EL-3, LINT-EL-5, LINT-EL-6, LINT-EL-7; LINT-P2-1, LINT-P2-3; their mutants RED | After the pinned review: GL-1, GL-2, GL-3, GL-4 (both arms), GL-6, GL-7, GL-9, each with its RED arm. GL-4 and GL-6 are measurements with decision tables: a STOP outcome stops the lane before S3. |
| S3 the menu, the name box, Save over, the button | NEW src/ui/EffectPresetsButton.h; NEW src/ui/EffectPresetsMenu.h (the pure models: the menu, the box, the Save Over window's keys, `buttonState`; the name filter); src/ui/EffectStackView.h / .cpp; src/ui/UniversalParamControl.h / .cpp (ONE method, `refreshConnectionDisplay`); src/ui/InspectorPanel.h / .cpp and the three inspector headers (forwarders only); src/MainComponent.cpp (the wiring lines, the `fx_preset_ui` callback); src/api/ApiServer.h / .cpp (`fx_preset_ui`; GET fx_presets gains `menu.saveOver`, `box`, `confirm`, `button`); NEW tests/test_effect_presets_menu.cpp; tests/CMakeLists.txt; .harmony/probe-effect-presets.sh (rows GL-5, GL-10, GL-11); docs: CLAUDE.md (the capability paragraph gains "effect presets (per effect, kept by the app for every show)"; UI Patterns gains "Effect Presets button"; the pitfall index gains NN), docs/claude/effects.md (the menu, the box, Save over, the button's rule), .harmony/APP-INVENTORY.md | LM-1..LM-24; LINT-EL-2; LINT-P2-1, LINT-P2-3; their mutants RED | GL-5, GL-10, GL-11 with their RED arms; GL-8; the regression probes; then GL-1, GL-2, GL-3, GL-4, GL-7, GL-9 once more at the lane's final head (GREEN only); LINT-P2-1 on the lane's whole diff. |
| VG the visual gate | a capture builder (window-id captures only, driven through `fx_preset_ui`; 24 states, each with a manifest of model facts AND of every text the state paints), then FIVE critic seats, as RU. The seats are given his 133 and 138 lines verbatim. | -- | The verdict; and that no manifest text holds "look" in any letter case. A fix round goes back to S3's files. Boris sees nothing of this lane before it passes. |

What Harmony runs herself, in order: (0) on the lane's base: LINT-P2-2 and the `ctest -N` baseline; questions 150-152 to
Boris with the adoption; (1) after SE's review: the regression probes; (2) after S2's review: the seven live rows; (3) after
S3's review: GL-5, GL-10, GL-11, GL-8, the regression probes, the re-run, LINT-P2-1 on the whole diff; (4) the visual gate;
(5) the merge sequence of RIG-RULES B. Every live row takes the live lock; none opens an Output window, captures a full
screen or sends synthetic input (the SCREEN-SAFETY LAW; RA's rig, unchanged); HD-22 stands (the name box and the Save Over
window of the TEST app show on his screen for seconds).
Order: one-save S7 merged (LINT-P2-2 green) -> S1a -> S1b, and SE any time before S2 -> [one-save S1 merged] -> S2 -> S3 -> VG
-> merge.

THE FENCE WITH THE ONE-SAVE LANE
- S7 first. This lane's base holds none of the old Save / Load / FX Save, the ten slots, `PresetManager` (E17). LINT-P2-2 is
  run by Harmony on the base before S1a's packet is cut; RED today (E12: src/MainComponent.cpp:2968, :2996, :3035;
  src/ui/PresetManager.h:33). If S7 merges and a line is still printed, the lane does not start: it is reported, and fixed in
  the one-save lane.
- This lane never brings the word back as a class: LINT-P2-3 on every stage's diff. One-save's LINT-6 (OS:640) then stays
  green after this lane merges -- its token test does not match `EffectPresetStore` or `EffectPresetsMenu` only if "token"
  there means the whole word `PresetManager`; that is how OS:640 states it (INFERRED from its text; the lint's script does
  not exist yet).
- S2 edits src/model/Composition.h, which one-save S1 is editing now (E18), and src/model/Clip.cpp and Layer.cpp, which no
  one-save stage names. S2's packet is cut after one-save S1 has merged; if another one-save stage is open on those files
  when S2 starts, whichever merges second re-bases as a builder's step 0 (RIG-RULES A2). S2's change there is six lines and
  writes nothing for an effect with no loaded preset, so no show fixture, checksum or baseline of that lane changes (LC-14
  pins it; GL-11 reads it live).
- CLAUDE.md: S7 strikes "instant preset save/recall"; S3 of this lane adds its own words. The strike must not be re-applied to
  S3's sentence at a re-base: said in S3's packet.
- LINT-EL-5 stands as worded (after S7 it is green by absence).

Pitfall NN (Harmony assigns the number), text for S2 -- it REPLACES RA:510-526:
"An effect preset is one effect's BASE values (`paramValues`, `dryWet`) AND the connection on each slider and on Dry / Wet
(source, shape, enabled -- never a grip, engine state or a live twin), by NAME -- uniform first, then label -- in one file per
preset: `~/Library/Audio-DNA/Effect Presets/<effect display name>/<preset name>.preset.json`; the file name is the preset's
name. A file without "signals": true holds values only and touches no connection. A preset file is never edited in place:
make writes a new file and reads it back before listing it; rename renames the file; delete removes it; Save over writes a
complete new file beside the old one, reads it back, swaps it in with one rename and reads the preset back from its own name
-- if a step fails the old preset stays, whole. The store never deletes or rewrites a file it could not read, never rewrites
a preset whose version is above its own, and reads an effect's folder the first time that effect's presets are asked for. A
build that adds a key to a preset file raises its version. Every preset in a menu is a file on disk. A load is
`EffectPresetCmd` -- one slot's values, Dry / Wet and connections inside one fence: a connection equal to the wanted one is
NEVER re-assigned (assignment clears its grip and memory), a changed or unplugged one has its live twin cleared, and the slot
is ticked once before the fence ends; never `bypassed`, never a layer field. A Signal connection whose name no signal has
drives nothing: the slider rests on its own value. THE BUTTON NAMES THE LOADED PRESET, NOT THE VALUES: `EffectSlot::
fxPresetName` (message thread; saved with the show as the optional key "fxPreset", written only when not empty) is set by a
load, New Preset, Save over, a rename from that row, the show's reader, and by a match only when a row is built for a slot
that has none -- never by a timer; the button reads that name whatever the sliders are, with no mark; with no loaded preset
it reads the preset the settings equal, else "Presets". The name is only ever compared with listed names, never made into a
path. The button never takes the keyboard. In code, files, routes and keys this is always an EFFECT preset (EffectPreset...,
fx_preset, fxPreset): MilkDrop has presets of its own and `PresetManager` was the removed whole-chain saver. Renaming an
effect's display name orphans its Effect Presets folder and every saved show that names it (LK-13); renaming a built-in
signal stops every preset and show that names it from being driven (CE-4). Changing what a uniform's 0..1 means changes
every saved preset and show: give the parameter a new uniform name instead."

Order against the other lanes: RA:528-534 and RU:466-470 stand, with the one-save bullet replaced by the fence above.

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; exact strings and bars; each with its RED arm; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------------
This section is LAID OVER RA section 5 (which is laid over RU section 5). A row that is not named here stands exactly as RA /
RU writes it -- and every such row holds no "look" (checked by grep over both rulings' row tables: 48 names hold the word
and all 48 are below). Every row is RED first. One TEST_CASE per row id, named exactly as written. Harmony copies gate
strings ONLY from this section for a row that is named here, else from RA section 5, else from RU section 5.
No bar is loosened by this delta: GL-10 gains a clause (twice) and GL-8's count grows by two, one row is re-cut because the
behaviour it pinned is the behaviour he declined (LM-3), one RED arm is retired with it (MU-EL-17).

5.1 THE 47 UNIT-CASE NAMES THAT ONLY CHANGE THEIR WORDS (file and tag per P2-3; the RED arm of each is unchanged)
| id | exact test name | RED arm |
|---|---|---|
| LK-1 | "LK-1 a preset round-trips by name: capture, toVar, fromVar, resolve give bit-equal values and Dry/Wet" | MU-EL-1 |
| LK-8 | "LK-8 values outside 0..1 are clamped, and a preset with such a value matches the slot it was loaded on" | MU-EL-25 |
| LK-9 | "LK-9 a slot matches a preset within 0.0005; one slider step (0.001) away it does not; a preset none of whose entries names a parameter matches nothing" | MU-EL-6 |
| LK-10 | "LK-10 a slot built the way the add sites build it matches the Default preset, for every registered effect" | MU-EL-26 |
| LK-13 | "LK-13 no effect name that has shipped is missing from the library" (a pinned list of today's display names must be a subset of the registered names; the failure text: "an effect was renamed or removed: its saved presets and every saved show that names it are orphaned -- restore the name, or add a folder alias before this ships") | MU-EL-28 |
| LK-14 | "LK-14 a connected parameter whose live twin drifts still matches its preset, and capture stores the base value" | MU-EL-24 |
| ST-3 | "ST-3 bad files beside good ones: the good ones list; the empty, the half-written, the wrong-effect and the binary file and a folder named like a preset are not listed, and their bytes are unchanged after make, rename and remove" (the fixture names bad files so that they sort both before and after the good ones, and the row prints the order read) | MU-EL-9 |
| ST-5 | "ST-5 a make that cannot write gives no preset: no new file, the list unchanged; a later make on a writable folder works" (a real read-only folder, and a writer hook) | MU-EL-10 |
| ST-8 | "ST-8 remove deletes exactly that preset's file; the effect's other presets and its folder stay" | MU-EL-33 |
| ST-9 | "ST-9 the constructor reads nothing; presetsFor reads a folder once for the whole run" (arm 1: dirReads is 0 after construction; arm 2: after presetsFor of one effect, called three times, it is exactly 1; prints "ST-9 INFO one read of 200 preset files took <ms> ms", reported, no bar) | MU-EL-12 |
| ST-11 | "ST-11 the test-mode root is never the real folder: an absolute AUDIODNA_EFFECT_PRESETS_DIR is used as given; unset, empty or relative gives a scratch folder under the temp directory" | MU-EL-21 |
| ST-12 | "ST-12 the file is the preset: two files with equal contents list as two presets under their file names; remove and rename act on the named file only" (fixture: "Preset 1", "Preset 1 copy", "Preset 10") | MU-EL-35 |
| LC-8 | "LC-8 the undo text is Load Preset '<preset>' on '<effect>'" | MU-EL-38 |
| LM-2 | "LM-2 Rename and Delete each list every one of his presets and never Default; two presets with equal values and a preset equal to Default can each be renamed and deleted; with no presets both are greyed" | MU-EL-41 |
| LM-5 | "LM-5 an effect the library does not know has no presets button" | MU-EL-43 |
| LM-7 | "LM-7 a pick pushes exactly one PerformPreset with the row's own scope and effect index and keeps the row unfolded; a pick that would change no value pushes none; with no host a pick writes nothing" | MU-EL-23, MU-EL-44 |
| LM-8 | "LM-8 the presets button never takes the keyboard: it does not want focus and a click does not grab it; every close of the menu, the Rename window and the Delete confirm calls the focus-home hook once" | MU-EL-19 |
| LM-9 | "LM-9 a drop still lands: fx:Echo,Ripple appends two effects with one PerformEdit; the presets button is not a drop target" | MU-EL-45 |
| LM-11 | "LM-11 at header widths 400, 300, 220 and 160 the B button, the name, the presets button and X do not overlap; under 56 px the button shows no text" | MU-EL-46 |
| LM-12 | "LM-12 a connected parameter whose live value drifts keeps the button on its preset" | MU-EL-24 |
| LM-15 | "LM-15 the name filter drops the characters a preset name may not hold and stops at 40" | MU-EL-48 |
| LK-15 | "LK-15 a preset name is legal when it is 1 to 40 characters, has none of the nine file characters or a control character, does not start with a dot, and is not Default, Presets, New Preset, Save over, Rename or Delete in any letter case" | MU-EL-29 |
| LK-16 | "LK-16 a preset with signals round-trips: a signal by name, a macro by index, an LFO, a timeline with its points and a clip position, each with range, invert, curve, smoothing and enabled, on parameters and on Dry/Wet" | MU-LA-1 |
| LK-18 | "LK-18 resolve with the Unplug policy: a wired entry gives the preset's connection; an unwired entry unplugs; a parameter the preset has no entry for keeps its value and its connection; Default unplugs every parameter and Dry/Wet" | MU-LA-3 |
| LK-22 | "LK-22 nameVerdict: empty and reserved are refused; a name is taken when a listed preset has it in any letter case or when its path exists; a preset's own name in another letter case is free for its own rename" | MU-LA-7, MU-LA-38 |
| LK-23 | "LK-23 a hand-written preset file of the current format loads to exact fields: a signal by name, a macro by index, an LFO, a timeline with its points and a clip position, each with its range, invert, curve, playback, smoothing and enabled, and a connection that holds only its source keeps the default shape" | MU-LA-33 |
| LK-24 | "LK-24 resolve with the Keep policy: a wired entry gives the preset's connection; an unwired entry and Default keep whatever is plugged; a slot with a signal the preset does not hold still does not match it" | MU-LA-37 |
| ST-2 | "ST-2 nextName gives Preset 1, Preset 2; a freed number is used again; a number taken by a listed preset in another letter case, by an unreadable file, or by a file that appeared after the folder was read is skipped; make under a given name makes that file, and under a taken name makes nothing and never overwrites" | MU-EL-30, MU-LA-8 |
| ST-10 | "ST-10 a file with unknown keys and a version above this build's lists, and no operation on another preset changes its bytes" | MU-EL-34 |
| ST-13 | "ST-13 replace swaps a whole verified file in: the preset then lists with the new values and connections; a second store on the folder reads the same; a replace that cannot write, or whose new file does not read back equal, leaves the old file's bytes unchanged" (a writer hook that truncates; a read-only folder) | MU-LA-9, MU-LA-26 |
| ST-14 | "ST-14 the new file's name is never listed: a left-over from a crash is not a preset, and the next replace of that preset succeeds and leaves none" | MU-LA-10 |
| ST-15 | "ST-15 replace touches one file: every other preset's bytes and modification time are unchanged; a name that does not list is not replaced and no file is made" | MU-LA-11 |
| ST-16 | "ST-16 a swap that fails leaves the old preset: with the folder made read-only between the read-back and the swap, replace reports failure, the old file's bytes are unchanged, the list still holds the old preset, and a later replace succeeds" (through `beforeSwapForTest`) | MU-LA-28 |
| ST-17 | "ST-17 replace refuses a preset whose file says a version above this build's: canReplace is false and its bytes are unchanged" | MU-LA-35 |
| LC-7 | "LC-7 a first-format preset on a connected parameter keeps its connection, its grip and its live twin; after one engine tick its effective value is the signal's and the preset's value is the base underneath" | MU-EL-37 |
| LC-9 | "LC-9 a preset with signals is one undo step: execute makes the slot's connections the resolved ones, undo puts the earlier connections back, redo applies again" | MU-LA-12 |
| LC-13 | "LC-13 the loaded-preset name follows the command: execute sets it, undo puts the earlier name back, redo sets it again; Default sets it empty" | MU-LA-17 |
| CE-4 | "CE-4 the registry's built-in signal names are the pinned 32" (the failure text: "a built-in signal was renamed or removed: every saved show and preset that names it stops being driven -- restore the name") | MU-LA-29 |
| LM-1 | "LM-1 the menu lists Default, his presets in natural order, New Preset, Save over, Rename, Delete; at most one item is ticked, the preset the button names" | MU-EL-40 |
| LM-4 | "LM-4 New Preset is enabled only when the settings match no preset, Default included; after the box is accepted the button reads the new preset's name; after Cancel, and after a make that fails, nothing changes and no file exists" | MU-EL-42 |
| LM-16 | "LM-16 New Preset opens the name box holding the next free Preset N with all of it selected; accepting it unchanged makes Preset N; accepting another name makes that preset; the preset holds the settings at the accept, not at the opening; a preset loaded before stays byte for byte and the new preset becomes the loaded one" | MU-LA-20, MU-LA-30 |
| LM-18 | "LM-18 Save over names the preset last loaded or made on this effect: greyed and bare with none, greyed while the settings equal it, greyed for a preset a newer build wrote, live after a tweak; in its window Save Over has no key and does not take the keyboard while Cancel has Esc and Return; confirming replaces that preset and the button reads its name; Cancel changes nothing" | MU-LA-22, MU-LA-36 |
| LM-19 | "LM-19 Rename uses the same box: it holds the present name selected, greys the same way, and a rename of the loaded preset keeps Save over pointing at it" | MU-LA-23 |
| LM-20 | "LM-20 after a load of a preset with signals the row's source buttons, ranges and invert marks show the new wiring, the row is neither rebuilt nor folded, and a grip on a connection the load did not change is still held" | MU-LA-24, MU-LA-39 |
| LM-21 | "LM-21 every close of the name box and of the Save Over window calls the focus-home hook once; while either is open the presets button still does not want the keyboard" | MU-LA-25 |
| LM-22 | "LM-22 the loaded preset is never moved by a match: with Preset A and Preset B equal, loading B makes the button and the tick read B and Save over name B; a tweak that passes through another preset's settings leaves the loaded preset; only an effect with no loaded preset takes the preset it matches, and only when its row is built" | MU-LA-32 |
| LM-23 | "LM-23 a name-box accept or a Save Over confirm that arrives after the view shows another chain, another effect at that index, or after the preset was deleted writes nothing" | MU-LA-31 |
Notes to three of them (no change of text): LK-23's hand-written literal holds "format": "audio-dna-effect-preset" and is
read from a file named `<name>.preset.json`, so a store that still expects the old value or extension fails it. LM-12's
fixture: the twin has drifted BEFORE the row is built, so that under MU-EL-24 the row-built match (writer 3) fails and the
button reads "Presets" -- with a name already loaded the mutant could not show. LM-18's "the button reads its name" and
LM-22's "the button ... read B" are now true by case (1) of P2-6.
Fixtures named in rows: "Look 1" -> "Preset 1" everywhere (ST-12: "Preset 1", "Preset 1 copy", "Preset 10").

5.2 THE ROWS LB2 RE-CUTS OR ADDS
| id | stage, file | exact test name | RED arm |
|---|---|---|---|
| LM-3 (re-cut; replaces RU:532) | S3, tests/test_effect_presets_menu.cpp | "LM-3 the button keeps the loaded preset's name: one slider step away, and with every slider far from its values, it reads that name and is not dim; an effect with no loaded preset reads the preset its settings equal and, one step away, Presets, dim; after Default is loaded and a slider moves it reads Presets, dim" | MU-P2-1, MU-P2-2 |
| LC-14 (new) | S2, tests/test_effect_preset_cmd.cpp | "LC-14 the loaded-preset name is saved with the show: a clip effect, a layer effect and a global effect each write fxPreset and read it back; an effect with no name writes no key; a show without the key loads with no name" | MU-P2-3, MU-P2-4, MU-P2-6 |
| LM-24 (new) | S3, tests/test_effect_presets_menu.cpp | "LM-24 a name that no preset of this effect lists names nothing: the button reads Presets, dim, nothing is ticked, Save over is bare and greyed, and the slot still holds the name" | MU-P2-5 |
Counts: 83 new unit cases = LK 24 + ST 17 + LC 14 + LM 24 + CE 4 (RA: 81; + LC-14, + LM-24; LM-3 is re-cut, not new).

5.3 THE MUTANTS
RU's MU-EL-1..48 and RA's MU-LA-1..39 stand with LB1's words, EXCEPT MU-EL-17 ("the button remembers the last loaded name"),
which is RETIRED: it is now the ruled behaviour. The delta's:
MU-P2-1 `buttonState` skips case (1): the text comes from the values alone (LM-3; GL-10). MU-P2-2 case (1) returns dim when
the settings differ from the loaded preset (LM-3). MU-P2-3 Clip.cpp's reader ignores "fxPreset" (LC-14; GL-11). MU-P2-4
Composition.h's writer writes "fxPreset" also for an empty name (LC-14). MU-P2-5 `buttonState` shows the stored name without
asking whether a preset lists (LM-24). MU-P2-6 Layer.cpp's writer omits "fxPreset" (LC-14).
Live mutant builds (extends HD-10): M-H = MU-P2-1 + MU-P2-3. They cannot mask each other: GL-10 is RED only on the line
"the button read Presets after the tweak" (MU-P2-3 does not touch a run without a re-open), GL-11 only on the line "loaded
none after a relaunch" (MU-P2-1 does not touch `loaded`).

5.4 THE LINTS (static; run by the builder, re-run by Harmony; each shown RED on a seeded line unless said)
- LINT-EL-1 (re-worded) "the Effect Presets folder is spelled once: `grep -rn 'getChildFile *(\"Effect Presets\")' src` prints
  exactly one line, in src/effects/EffectPresetStore.cpp; EffectPresetStore.cpp names no replaceWithText, replaceWithData,
  TemporaryFile; replaceFileIn appears exactly once, inside `replace`; findChildFiles / RangedDirectoryIterator /
  DirectoryIterator appear in the new files only inside `listFolder`".
- LINT-EL-3 (re-worded) "src/core/EffectPresetCmd.h names no bypassed, opacity, solo, mute, runtime( and no `.enabled` of a
  slot; it assigns no vector of connections or twins whole (no `paramConns =`, no `paramLive =`)".
- LINT-EL-6 stands, with `effectPresetsDir` in place of `effectLooksDir`.
- LINT-EL-7 (re-worded; replaces RA:637-638) "`grep -rn fxPresetName src/render` prints nothing; `grep -rn '\"fxPreset\"' src`
  prints exactly six lines: one write and one read in each of src/model/Clip.cpp, src/model/Layer.cpp,
  src/model/Composition.h".
- LINT-EL-2, -4, -5 stand.
- LINT-P2-1 (new; the rename's gate) "the diff against the lane's base adds no line that holds the whole word look or looks
  in any letter case -- code, comments, strings, test names, docs, the probe; and `grep -rnE 'Looks\"|\.look\.json|
  audio-dna-look|AUDIODNA_LOOKS_DIR|lookName|look_(make|load|ui|replace)|/api/debug/looks' src tests docs/claude CLAUDE.md
  .harmony/probe-effect-presets.sh` prints nothing". Run on each stage's diff and, by Harmony, on the lane's whole diff.
  Lines that exist today (E14, old comments) are not added lines and are not touched. Last clause: no NEW file of the lane
  (source, test, probe) holds the letters "look" in any letter case at all -- `grep -il look <the new files>` prints nothing.
- LINT-P2-2 (new; the fence; Harmony, on the lane's base, before S1a) "`grep -rnw PresetManager src` prints nothing, and
  `grep -nE '\"Save preset\.\.\.\"|\"Load preset\.\.\.\"|\"Legacy Preset\"' src/MainComponent.cpp` prints nothing". RED arm:
  the tree of 7bc6df7 (src/ui/PresetManager.h:33; src/MainComponent.cpp:2968, :2996, :3035).
- LINT-P2-3 (new) "the stage's diff adds no whole word PresetManager, no file whose name starts with Preset, and no
  declaration `class Preset`, `struct Preset` or `namespace preset`".

5.5 HARMONY'S LIVE ROWS (.harmony/probe-effect-presets.sh). The rig is RA 5.6's, unchanged but for names: a test-server
build; `open -g`; --test-mode; a FRESH AUDIODNA_EFFECT_PRESETS_DIR per row and per arm; the live lock; no Output window; no
full-screen capture; no synthetic input; HTTP with Connection: close; quits only its own pid. Routes: P2-2. Fixtures as
RA:656-668 with these changes only: every fixture file is `<name>.preset.json` with "format": "audio-dna-effect-preset";
"Look 1" is "Preset 1". Each row DOES what RA / RU says it does, through the renamed routes.
| row | bar (the exact line printed on pass) | RED arm (and its FAIL line) |
|---|---|---|
| GL-1 | stands: "GL-1 PASS Wobble on disk at make (segments 0.6100 rotation 0.2700); listed and loaded on a layer in another show after a relaunch; windows unchanged; file label unchanged" | MU-EL-8 |
| GL-2 | "GL-2 PASS 3/3 hosts hold the hand-written preset by name: intensity 0.1000 speed 0.9000 freq kept 0.5000 dryWet 0.4000" | MU-EL-2; MU-EL-22 (1/3 hosts) |
| GL-3 | "GL-3 PASS strip equal 5/5; clip preset changed <d> back <d>; layer preset changed <d> back <d>; other effect kept 0.6000; noise <n> bar <b>" | MU-EL-14; MU-EL-13 |
| GL-4 | both printed lines stand: "GL-4 MEASURED loads=20 hold=<n> per_load=<x.xx> black=<n> control=<n>/<n>" and "GL-4 MEASURED wired loads=20 hold=<n> per_load=<x.xx> black=<n> control=<n>/<n>"; both decision tables stand | the control arm of each table |
| GL-5 | "GL-5 PASS menu pick on layer fx 1: the box opened on Preset 1, selected; far greyed the button (taken) and Return left the box open; Mine stored 0.5500; Far set fx 1 to 0.8000; fx 0 kept 0.3000; focus home +3" | MU-EL-23 ("Mine stored 0.3000"); MU-LA-21 ("far did not grey the button") |
| GL-6 | "GL-6 MEASURED control lanes=<n> preset lanes=<n> stop-state=<equal|differs> connections=<n>"; the table stands | the control arm |
| GL-7 | "GL-7 PASS real Effect Presets folder unchanged (<absent or n files>); scratch folder named on stderr; the preset went there" -- the listing is of ~/Library/Audio-DNA/Effect Presets/ ONLY, before the first launch and after the last quit. (That the app spells no other folder is LINT-EL-1 and LINT-P2-1; the probe itself never spells the old name.) | the probe's selftest, as RU |
| GL-8 | "GL-8 PASS ctest <N> = baseline <N0> + 83 new cases, 0 failed" (83 = 24 + 17 + 14 + 24 + 4) | the lane's base: N0 is not N0 + 83 |
| GL-9 | LIVE (131 A): "GL-9 PASS wired preset: 2 connections stored; plugged on fx 1 with 2 live values in the load's own answer, matched Wired; Ghost rests at 0.3500 (live none), matched Ghost; undo gave 2 connections back, then 0; Default unplugged 2 on fx 0". DORMANT (only if he corrects R88 to B): "GL-9 PASS wired preset: 2 connections stored; plugged on fx 1 with 2 live values in the load's own answer, matched Wired; Ghost rests at 0.3500 (live none), speed kept its LFO, matched none; undo gave 2 connections back, then 0; Default kept 2 on fx 0" | MU-LA-12; MU-LA-18; MU-LA-27 -- FAIL lines as RA:678 |
| GL-10 (one clause added, twice) | "GL-10 PASS changed preset: after the tweak the button still read Off, not dim, and New Preset and Save over \"Off\" were offered; New Preset made Preset 1 (0.2500), left Off's bytes and time unchanged and became the loaded preset; with Off loaded and tweaked again the button read Off and the item still read Save over \"Off\"; Return did not save; Save Over replaced Off 0.0000 -> 0.2500, matched Off; no new-file left; Far and Preset 1 untouched; held after a relaunch" | MU-LA-26 ("after a relaunch 0.0000"); MU-LA-22 ("Save over not offered"); MU-LA-30 ("Off's bytes changed at New Preset"); MU-LA-32 ("the item read Save over \"Preset 1\""); MU-LA-36 ("Return saved"); MU-P2-1 ("the button read Presets after the tweak") |
| GL-11 (new) | "GL-11 PASS the name is the show's: Off loaded, amount moved to 0.2500, show saved; after a relaunch on the saved show loaded Off, the button read Off, not dim, matched none, Save over \"Off\" live; the saved show holds fxPreset once; a show saved with no loaded preset holds none" | MU-P2-3 ("loaded none after a relaunch"); the tree before S2's serializer lines prints the same FAIL |
What GL-10 reads, so the line cannot pass by accident (RA:680-685 stands, with `button` added): (a) `matched` "", `loaded`
"Off", `button` = {"Off", dim false}, `menu.newPreset` enabled, `menu.saveOver` = {`Save over "Off"`, enabled}. (b) `box` =
{open, "New Preset", "Preset 1"}; after accept `matched`, `loaded` and `button.text` "Preset 1", `menu.saveOver` =
{`Save over "Preset 1"`, greyed}. (c) after Off is loaded and tweaked to 0.25 the settings equal Preset 1: `matched`
"Preset 1", `loaded` "Off", `button.text` "Off" (under the old rule it would read "Preset 1"; under MU-P2-1 it does);
after `return` Off's bytes are unchanged; after `accept` `matched` is "Off". (d) amount 0.2500.
GL-11, step by step. Show B (a scratch copy) with "Off" pre-written; `fx_preset_ui expand` on the clip's Invert.
(a) fx_preset_load "Off"; POST /api/set_param Invert amount 0.25; GET fx_presets: `loaded` "Off", `matched` "", `button` =
{"Off", dim false}. (b) save the show to a scratch path the way the lane's probes save one (POST /api/debug/save_composition,
E16; its body as the one-save lane leaves it); count the lines of that file that hold "fxPreset": exactly 1, with "Off".
(c) quit own pid; relaunch on the saved file with the same scratch presets folder; expand; GET fx_presets: `loaded` "Off",
`button` = {"Off", dim false}, `matched` "", amount 0.2500, `menu.saveOver` = {`Save over "Off"`, enabled}. (d) relaunch on
the untouched show B; save it to a second scratch path; that file holds "fxPreset" 0 times.

5.6 THE VISUAL GATE: 24 STATES (RU's rules; every state is captured with the new words; each manifest also lists every text
the state paints, and Harmony checks that none holds "look")
Stand as RA / RU write them, with LB1's words: V-1 ("Default"), V-2 (a preset matched: its name), V-4 (the first menu:
Default ticked; New Preset, "Save over", Rename, Delete greyed), V-5 (three presets, one loaded and ticked: New Preset
greyed, `Save over "<that preset>"` greyed, Rename and Delete live), V-7, V-8, V-9 (the box for Rename), V-10, V-12, V-13,
V-14 (a 40-character preset name), V-15, V-16 (thirty presets), V-17, V-18, V-19 (the box for New Preset as it opens:
"Preset 3", selected), V-20, V-21 (the Save Over Preset window), V-22 (the loaded preset was deleted: the button reads
"Presets", dim; "Save over" bare and greyed).
Re-cut: V-3 an effect that was never given a preset, one slider moved: "Presets", dim (question 150 B would make it
"Default"). V-6 the menu of an effect whose loaded preset was changed: THAT PRESET STILL TICKED; New Preset live;
`Save over "Preset 2"` live. V-11 the layer inspector's stack, framed so that the feedback combo above it is in the picture
(P2-4).
New: V-23 an unfolded effect with "Preset 2" loaded and every slider far from it: the button reads "Preset 2", not dim
(manifest: `loaded` "Preset 2", `matched` "", the button's colour) -- captured beside V-2; the two buttons must look the
same, and the critics are told that this sameness is his answer -- Boris: "no need to show that it was changed". V-24 the
browser on its MilkDrop tab beside a clip inspector whose effect's Presets menu is open (P2-4's first row).
Struck from what the critics are told (RA:718): "the button can read "Look 1" while the menu says `Save over "Look 2"`" --
that state no longer exists. Added: his 133 and 138 lines verbatim; the rule of P2-4; "the dim 'Presets' means the effect
has no preset, never that a preset was changed".
NOT capturable in the rig: as RA:719-720.

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; this list REPLACES RA section 6)
---------------------------------------------------------------------------------------------------------
On his own screens, in his own show. Nothing here is shown to him before the visual gate has passed.
1. Put an effect on a clip: its small button reads "Default". Move a slider: it reads "Presets", dim (question 150). Press it
   and choose New Preset -> a small box opens holding "Preset 1", already selected, and you can type at once. Press Return ->
   the button reads "Preset 1". Or type a name first, then Return -> the button reads that name. Esc -> no preset -> wrong:
   you must click the box before you can type; typing adds to "Preset 1" instead of replacing it; a preset appears after Esc;
   the word "look" anywhere.
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Preset 1" is there;
   choosing it sets the sliders as you left them -> wrong: it is missing, or it is there only in the first show.
3. While the box is open, type letters that are your clip keys -> they go into the box and no clip fires. After Return or Esc
   your keys launch clips again at once -> wrong: a clip fires while you type, or the keys are dead after the box closes.
4. With a clip playing on that layer, load a preset -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move. For one frame the screens hold the picture they had, as they do when you
   add an effect. The same with a preset that plugs signals in -> wrong: the layer restarts, the picture goes black, or
   anything in the strip changes.
5. Press Cmd+Z -> the effect returns to how it was just before the preset, sliders AND signals, and the button reads what it
   read before; a slider you moved on ANOTHER effect meanwhile stays where you put it -> wrong: the other effect's slider
   jumps back; the effect folds shut; the sliders come back but the signals do not.
6. THE NAME STAYS. Load "Preset 2", then move its sliders as far as you like -> the button still reads "Preset 2", in the
   same colour, with no mark. Open the menu: "Preset 2" is still ticked; click it -> the effect is back on Preset 2 (Cmd+Z
   takes that back). Save the show, quit, open it again -> the button still reads "Preset 2" -> wrong: the name dims, turns
   into "Presets", gets a star or a dot; the name is gone after you re-open the show; the name changes by itself while you
   drag a slider.
7. Signals. (a) Plug the bass into a slider, set its range, make a preset. Put the same effect on another clip and load the
   preset -> the slider there follows the bass with the same range, from the first frame. (b) Load that preset again on the
   first effect -> nothing jumps, and a slider you are holding stays in your hand. (c) Load "Default", or a preset you made
   with no signal on a slider -> the signals on that effect are unplugged. Cmd+Z brings sliders and signals back in one
   step. That is the only way back, and only until you close the show: wiring you want to keep, keep as a preset first.
   (d) To change what signals a preset holds: load it on an effect in your show, change the signals there, then
   `Save over "<its name>"`. There is no other place to edit a preset. (e) A preset remembers "Macro 3", "Mod 1" or "Mod 2"
   by name, not what they are doing in this show. (f) A slider driven by "Clip Position" holds still today, with or without
   a preset: that is older than this build and is on the list -> wrong: a slider pinned at one end after a load; a signal
   left on after Default; an undo that brings back sliders but not signals.
8. Rename opens the same box, holding the preset's name, selected. Delete: a list in red; pick one; a window asks. You never
   have to load a preset to rename or delete it -> wrong: a preset you cannot rename or delete; a name that changes by
   itself.
9. New Preset is grey when the effect already is exactly one of your presets, or Default: there is nothing new to keep.
   `Save over "Preset 2"` is live only after you loaded or made Preset 2 on that effect and then changed something.
10. Load "Preset 2", change a slider, open the menu. New Preset -> the box says "Preset 3": Return keeps the change as
   Preset 3 and Preset 2 is as it was. Or `Save over "Preset 2"` -> a window asks -> click Save Over: Preset 2 now holds the
   change wherever you load it from now on. An effect that already had the old Preset 2 keeps its settings and still reads
   "Preset 2"; clicking Preset 2 in its menu gives it the new settings. There is no undo for a Save over -> wrong: Preset 2
   changed after New Preset; Save over names another preset; Preset 2 is missing or half-changed after a Save over.
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
   keeps its settings and its signals in the show -- but the Presets menus there list only that computer's presets, and a
   button whose preset is not on that computer reads "Presets". Copy that folder across and the names are back. To back your
   presets up, copy that folder.
15. If the disk is full or that folder is locked, New Preset and Save over do nothing: the button keeps reading what it
   read and the old preset is untouched. No message.
16. By eye on your own screens: the button's size, the dim "Presets", the menu's length with many presets, the red Delete
   list, the name box and the red name when a name is taken, the Save Over window.
17. The old buttons (they go with the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone.
   Your two old presets and your nine quick FX saves were moved to the Trash at your word. Still on your disk, and no longer
   opened by the app: one deck file ("test 1.deck.json") in Library / AudioDNA / Presets.
18. Two kinds of preset on one screen: open the MilkDrop tab on the left and an effect's Presets menu on the right. The
   MilkDrop list says "presets" too ("Search presets...") -> tell us if you ever read one as the other (question 151).
19. Rename a preset that several effects carry (question 152): the effect you renamed it from shows the new name; the others
   show it too while they still are exactly that preset; the ones you changed read "Presets" until you load a preset on them
   again.
20. Not changed by this build: the Record tab still says "holding the last look". There "look" means the picture your take
   left on screen, not an effect preset; that screen is being rebuilt with the recording review.

---------------------------------------------------------------------------------------------------------
## 7 QUESTIONS FOR BORIS (new numbers 150..152; each has a default A; nothing waits; 153 is not used)
---------------------------------------------------------------------------------------------------------
Only what is his. Ruled here and NOT asked: every stored name, the button's rule for a loaded preset (his 133), the name in
the show, the tick, the routes, the fence. Questions 131-134 and 138 are answered and are not re-worded.
150. An effect you never gave a preset reads "Default" on its small button. You move one of its sliders.
     A (default) It now reads "Presets", dim: it is no longer at its defaults and has no preset. It reads "Default" again if
       you bring the sliders back or load Default.
     B It keeps reading "Default", the way a preset's name stays.
151. MilkDrop's list also calls its items "presets" ("Search presets...", "12 presets"). Now effects have presets too.
     A (default) Leave MilkDrop's words as they are: they are inside the tab named MilkDrop.
     B Make them say "MilkDrop presets" in full, everywhere in that tab.
152. Several effects carry "Preset 2". You rename it to "Wobble" from one of them.
     A (default) That effect reads "Wobble". The others read "Wobble" too while they still are exactly that preset; the ones
       you changed read "Presets" until you load a preset on them again.
     B Every effect in the open show that carried "Preset 2" reads "Wobble".
What changes with each answer (so that either is a small change)
- 150 B: loading "Default" sets the name "Default" (LC-13's last clause becomes "Default sets it to Default"); writer (3)
  also takes a match with Default; LM-3's last clause reads "... it still reads Default"; V-3 shows "Default". No stored
  name, no format, no route changes. An effect in a show saved before the answer reads as under A until its row is built.
- 151 B: five strings in src/ui/MilkDropBrowser.cpp (:101, :122, :283, :456) and MilkDropBrowser.h (:120); V-24 is
  re-captured. Not this lane's files today: a row added to S3, or its own small packet.
- 152 B: the host walks the open show's effects of that kind after a Rename and carries the name (message thread, not an
  undo step); one unit row; one clause in LM-19. Shows that are not open keep the old name and read "Presets".
Readings these rules rest on (INFERRED consent; told to him, not corrected): R88 -- if corrected to B, one constant and
GL-9's dormant line (LB3). R95 -- if corrected, 138 B becomes its own stage (LB3's last paragraph); nothing here is undone.

---------------------------------------------------------------------------------------------------------
## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "Saving the name in the show (P2-7) is a feature, and Harmony's constraint says this delta
   adds none. His sentence is permissive -- 'It can be called look 2' -- so a name that is forgotten when the show is
   re-opened breaks no word of his; and the key puts six lines into serializers a lane beside this one is editing, which RA
   promised never to do." Why it still loses: (a) his reason is "Effects are usually changed by the user" -- so the changed
   effect is the NORMAL case, and with a run-only name the normal case loses its name at every launch: he would see his 133
   answer work for one evening; (b) RA already ruled the consequence for its own option ("taken up at once", RA:824-825);
   it is the option's known price, not a new idea; (c) the cost is bounded and measured -- six lines, written only for a
   named effect, so every existing show file and every one-save baseline is byte-equal (LC-14's "writes no key" clause,
   GL-11 step d). Cheapest refuting test: GL-11's RED arm is exactly the alternative; Harmony can look at it and decide.
   IF THE RULING CUTS P2-7: S2 loses the six serializer lines, LC-14 and GL-11 go, the count is 82, LINT-EL-7 returns to
   RA's wording with the new member name, check 6 loses "Save the show, quit, open it again", and LM-24 stays.
R2 "A name that no longer describes the effect misleads in a set: he reads 'Preset 2' and expects that picture." That is his
   answer, in his words. The guard he gets: the menu ticks it and one click puts the effect back on it. Refuting test: his
   check 6.
R3 "Default" is treated differently from a preset he made (it follows the values). One rule for both would be simpler to
   explain. It is his call: question 150; B is three clauses.
R4 A name is only text. Delete "A", make a new "A" for the same effect, and every effect that carried the old "A" reads "A"
   again and is offered `Save over "A"`. The guards are RA's: the window names the preset and the effect; Return and Esc
   cancel. Not closed further here: closing it needs an identity inside the file, which is a format change.
   Refuting test: LM-24 followed by a make of that name (a clause a ruling may add).
R5 An effect with no loaded preset that he brings exactly onto a preset by hand shows that name only while it stays equal --
   unless its row was built meanwhile, after which the name is kept (writer 3). The two differ by something he cannot see.
   Accepted as RA accepted its R12: reaching another preset's values by hand, on every slider within 0.0005, is in practice
   the Default case, which question 150 covers.
R6 NOT VERIFIED: whether the one-save lane pins the set of keys a show may hold, or rules that a new key raises the show's
   version (its S1 was not read past its stage row). If either is true S2 follows that rule (adds the key to the pin; raises
   nothing unless that lane says so) and says so in its report. Refuting test: one-save's own unit rows in S2's full ctest.
R7 The rename can leak. LINT-P2-1 reads added lines for the whole word and greps the known old names; a builder could still
   coin `lookStore`. Guard: the lint's last clause (no NEW file holds the letters "look" at all) and the review. On screen
   the guard is the visual gate's manifest check.
R8 Question 151 A leaves two bare "presets" on one screen. V-24 is the test; five critic seats and then he look at it.
R9 The Record tab keeps "look" in another meaning (E14). He may read it as a missed rename: check 20 tells him first.
R10 GL-11 saves through POST /api/debug/save_composition, whose body the one-save lane is changing now. The probe's save
   step is written when S3's packet is cut, against that lane's merged route.
R11 "Effect Presets" holds a space: every probe line quotes the path. ~/Library/Audio-DNA (this lane) and ~/Library/AudioDNA
   (the old folders) stay two folders (RA SF-9); nobody "corrects" one into the other.
R12 83 and 24 are derived numbers; a row cut or added by the ruling changes them, and S3's report restates the count.
R13 If he corrects R95, the lane's menu and store stand and 138 B is added beside them; what would be re-cut is ST-9's
   "once" (LB3). Nothing is built towards it.
RA's risks R1 (now closed by his answer), R2, R4-R8, R10, R11, R13-R16 stand. RA R9 is gone (P2-6). RA R3 is replaced by R2
and R4 here. RA R12 is replaced by R5 here.

---------------------------------------------------------------------------------------------------------
## 9 WHAT IS NOT IN THIS LANE
---------------------------------------------------------------------------------------------------------
- Presets listed in the effect list and dragged from it (138 B): costed in LB3, designed nowhere.
- Any change to MilkDrop's words (question 151), to the feedback preset combo, to Preferences.
- The record panel's "look" (E14) and everything else of the recording / actions re-model. Handed to Harmony as a finding
  for that lane.
- The removal of the old Save, Load, FX Save, the ten slots, `PresetManager` and its test (one-save S7). Anything about
  "test 1.deck.json" or the four deck files (RA HD-20, the one-save lane).
- A mark, a dim state, a star or a tooltip on a changed preset. A distance at which a name is dropped.
- Carrying a rename to other effects (question 152 B). An identity for a preset beyond its name (R4).
- A "keep signals" switch, a prompt before unplugging, any second way back for an unplugged signal (131). A hidden copy of
  a saved-over preset (RA HD-15: closed by "132 default good").
- A reader for files named `.look.json` or holding "audio-dna-look": none exists on any disk.
- Everything RA's "NOT IN THIS LANE" (RA:888-905) lists, unchanged, except its line "The loaded-look name in the show
  (HD-14). A mark on the button after a tweak (133 B)." -- the first is now IN the lane (P2-7), the second stays out.
- The stopped sync-dial branches: NOTHING is carried from lane/bf2 (740b6d6) or lane/bf2-keys (9eab9bd), and nothing of them
  is dropped by this lane; rulings-bf2.md H-17 already rules them superseded. Both worktrees were checked clean and not read.

THE LINES OF THE TWO RULINGS THIS DELTA REPLACES (complete; everything else stands, with LB1's words)
| ruling lines | what | replaced by |
|---|---|---|
| RU:43, :264, :287; RA:294, :301, :372-376 | the folder, the extension, "Look N", the new-file name | P2-2 |
| RU:267-268; RA:298-300 | the names a preset may not have | P2-1 |
| RU:277 | the "format" value | P2-2 |
| RU:316-319; RA:73-82 (the words), :279, :311, :472-476 | every on-screen text | P2-1 |
| RU:324-331 (AM-7: the file name, the button's text rule) | | P2-3, P2-6 |
| RA:257-276 (RA-1): the bullets on `shownMatch` driving the button and the tick, "On screen, said to the critics", and "never saved" | | P2-6, P2-7; the rest of RA-1 stands |
| RA:440-443 (RA-14), RA:445-448 (RA-15) | the button after a tweak; his old files | P2-6; LB4 |
| RU:359-382 (AM-11, AM-12); RA:642-655 | the routes, the variable, `effectLooksDir` | P2-2, P2-3 |
| RU:461-462; RA:529-531 | "no key to the show", "edits none of" the three model files | P2-7, the fence |
| RA:490-497, :499-503 | the stage table; what Harmony runs | section 4 |
| RA:510-526 (and RU:449-458 before it) | the pitfall text | section 4 |
| RU:482-544 and RA:547-602, the 48 names that hold the word; RU:532 (LM-3); RU:554 (MU-EL-17) | | 5.1, 5.2, 5.3 |
| RU:571-580; RA:631-638 (LINT-EL-1, -3, -6, -7) | | 5.4 |
| RU:604-611; RA:672-685 (the gate lines GL-2, 3, 5, 6, 7, 8, 9, 10 and GL-10's reads) | | 5.5 |
| RU:642-650; RA:707-718 | the visual states; what the critics are told | 5.6 |
| RA:723-779 (section 6, which had replaced RU's) | his checks | section 6 |
| RA:782-812 (section 7) | questions 131-134 and "what changes with each answer" | answered (E1); LB2, LB3, LB4 |
| RA HD-14, HD-15, HD-18; RA R3, R9, R12 | | P2-7; closed; closed; section 8 |

STATUS: DONE

## HARMONY: ADOPTION OWED (2026-10-04 22:43:31, session s-rta-1004b) -- NOT adopted yet
The ruling .harmony/.reports/s-rta-1004b/ruling-looks-answers2.md returned DONE at this session's end (run wf_fbb395b7-0a5: plan architect opus high; seats gates 8 attacks / 1 MUST, stage-hands 7 / 0, papers whole 19,583 characters in attack-looks-answers2-papers.md; ruling architect opus max: 15 attacks = 7 ACCEPT / 8 PARTIAL / 0 REJECT; 16 amendments RB-1..RB-16; verdict "NEEDS REVISION; the delta's core stands"). I read only its returned verdict, stage list, decisions and the two questions. It is NOT adopted: nothing of the presets lane can start before the one-save lane's M2 (its own HD-23), so no packet waits on it. FIRST ACT on this lane next session: read its section 7 in full, decide HD-23..HD-31 (HD-31 = whether the preset's name is saved with the show as the key "fxPreset": six lines in Clip.cpp, Layer.cpp and Composition.h that the earlier ruling promised not to touch; default keep), then append the adoption block here.
HEADLINES as returned: every on-screen "look" reads "preset"; stored names change (folder ~/Library/Audio-DNA/Effect Presets/, extension .preset.json, format "audio-dna-effect-preset"; code always EffectPreset..., never bare Preset... or PresetManager); the button keeps the carried preset's name with no mark; an effect never given a preset keeps reading "Default" after a tweak; a Rename or Delete reaches every carrier in the open show; 131 A and 132 A need no code. QUESTIONS for Boris: 150 and 151 (152 withdrawn, 153 not used) -- put on his page at this close.
