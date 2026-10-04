# PLAN -- lane "one-save": one Save; the show holds its decks, its key and MIDI settings, its window layout and its routines

Author: architect (s-rta-1004), read-only. Pin checked first: `git -C /Users/boriskarpman/projects/RealTimeAudio rev-parse --short HEAD`
= 185147b; `git status --short -- src tests docs CMakeLists.txt` printed nothing, so every src / tests / docs read is a plain
file at 185147b. Worktrees checked clean: lane/bf2 = 740b6d6, lane/bf2-keys = 9eab9bd. Nothing was built, run or launched.
Labels: VERIFIED = a line I read at 185147b, or a fact-sheet row its VERIFICATION section did not overturn (FO = facts-one-save.md,
FO-V<n> = row n of its VERIFICATION table, FO-M<n> = its "Missing" list, FS = facts-saves.md). INFERRED / ASSUMED are marked where used.
Boris is quoted only verbatim, only from .harmony/binding-decisions.md and .harmony/boris-feedback-backlog.md.

## 1 GOAL

Boris: "All of these things should be saved when a show is saved. There's no reason to save them separately:" (2026-10-04 12:50:58).
Afterwards there is ONE save: Save / Save As of the show. The show file holds, besides what it holds today (decks, clips, layer
settings, effects, routines and the pad bank), a VERSION, the key and MIDI settings, and the window layout. The separate saves go
with their menu items, buttons, code, tests and docs: Save Deck, Save Deck As, Load Deck..., the Decks section of the library,
Export Bindings, Import Bindings, Save Layout, Load Layout. The list of shows opens each show to its decks (Boris: "A deck will go
into all of the decks that are available and they are the decks that are available with a show like this is how resolume does it:")
and a deck is taken into the current show from there. Keys and MIDI (Boris: "83 they are in a show and can be imported to another
show"; "yes but you import the setup from the most recent show"; "with show and app. these stay. if a user starts another show file
from scratch, they can import the settings from another show file."): opening a show switches to its set, the computer keeps the
set of the most recent show for the next launch and for new shows, and "Import Settings from Show..." reads a set out of a show
file he picks. Quitting asks first (Boris: "when the user quits, we need to have a secondary window open to say this, which is what
resolume does. Look at the image."). Collect Media and Snapshot stay commands of their own (Boris: "82 default"; reading R36 told to
him). The effects looks are not this lane (Boris: "every effect has many looks with specific parameter setups. these are saved with
the app. always."), beyond his answer to question 86 (OS6.4).
Harmony constraint: nothing in this lane may make a file he has unreadable; the first Save of an old-shape show must leave a way
back; a show whose keys or layout block is absent, old or unreadable still OPENS; applying a show's keys never leaves a held key
stuck or a learn screen half open; a command that goes takes its menu item, button, code, tests and docs in the same stage; test
mode never shows the quit window.

## 2 ESTABLISHED FACTS (VERIFIED only)

Show file
- E1 The show is one JSON object written by `Composition::toVar` (src/model/Composition.h:735-868) through `saveToFile` =
  `file.replaceWithText(JSON)` (:1153-1157). JUCE's `File::replaceWithText` writes a hidden temporary file and then
  `overwriteTargetFileWithTemporary()` (build/_deps/juce-src/modules/juce_core/files/juce_File.cpp:798-803): a failed write
  leaves the old file in place.
- E2 No version key. The shape is told by key presence: `ShowMigration::isLegacyShow` = "no top-level `layers` array"
  (src/model/ShowMigration.h:18-22), used by `fromVar` (Composition.h:993-997). Unknown keys are ignored on load and dropped on the
  next save (FO Q3).
- E3 `loadFromFile` sets `filePath` to the file read (Composition.h:1159-1168); `saveComposition()` writes to `filePath` with no
  backup and no prompt (src/MainComponent.cpp:3546-3569). So a plain Save of an old-shape show overwrites the original in the new
  shape (FO-M3).
- E4 Three src callers write a show: `saveComposition` (MainComponent.cpp:3551), `saveCompositionTo` (:3574; the Save As chooser
  and the test route `/api/debug/save_composition`, :2151), and Collect Media (`composition_.saveToFile(compFile)`, :6692-6694).
- E5 Dead keys (FO Q3 lines 43-50, re-confirmed FO VERIFICATION "Also re-checked"): `outputDisplay`, `crossfaderPhase`,
  `crossfaderBlendMode`, `crossfaderBehaviour`, `crossfaderCurve`, `genrePresetNames`, `smartAutopilotEnabled` have no reference
  outside Composition.h; `bpmMultiplier` is set by ui/TopBar.cpp:403 and read by nothing; `autopilotDirection / DurationMode /
  ClipLoops / Loop / MasterLayer` are set and shown only by ui/CompositionInspector.cpp; `autoPresetOnGenre`,
  `genreDeckAssignment`, `structuralSceneEnabled` are read by callbacks that only log.
- E6 `GET /api/composition` serves `toVar()` on the HTTP thread, unlocked (RIG-RULES A, SF-12).
- E7 Boris's disk (FO line 5 + its VERIFICATION header): ONE show, "test with harry.json", OLD shape (no top-level "layers"; decks
  "Deck 1" id 0 and "Deck 2" id 100); `Decks/` holds 4 v1 `*.deck.json` ("type":"deck", no "layers") and NO v2 deck file; no
  `bindings/` and no `layouts/` folder; `settings.json` holds only "outputs"; `FX Saves/` empty; `Presets/` 4 entries. A copy of
  his show is in .harmony/.reports/s-rta-1004/boris-show-backup/ (boris-clarify-86.md R44).
- E8 The v1 deck files are already neither listed nor loadable (ui/CompDecksBrowser.h:43-49; CompDecksBrowser.cpp:297-320;
  MainComponent.cpp:3657-3665 "not a deck file").

Keys and MIDI
- E9 One `BindingManager bindingManager_`, empty at launch; `addBinding` is called only by the two overlays (FO Q4, FO-V13). No UI
  removes one binding or clears all (FO-M5).
- E10 Format: `{"bindings":[22 fields each],"version":1}`, ids not written, enums as ints, `Action` APPEND ONLY
  (src/binding/BindingManager.cpp:214-248; src/binding/Binding.h:24-47). `fromVar` clears the list FIRST, reads every field
  unguarded, mints ids from `nextId_`, clears `relativeCCValues_` (:250-292); a JSON without a "bindings" array erases every
  binding and `loadFromFile` still returns true (:300-308).
- E11 Export / Import Bindings: MainComponent.cpp:7279-7309, menu ui/MenuBarModel.cpp:160-161. Neither exits a learn mode nor
  refreshes anything (FO-V16).
- E12 A held momentary press is tracked in `momentaryRefs_`, keyed by the binding id (MainComponent.cpp:7674-7685, :7721, :7758);
  the map is never cleared by a show swap (grep: those five lines are its only uses). The overlays cache `targets_` when a mode
  is entered (:7510-7550); `exitAllBindingModes()` closes both (:7554-7560). Cmd-modified keys never reach bindings (:4101).
- E13 Machine settings: `AppSettings` (src/model/AppSettings.h:18-19 two keys, `update` is read-modify-write and keeps unknown
  keys); a test-mode app never touches the real file (`appSettingsFile`, MainComponent.cpp:83-100, env AUDIODNA_SETTINGS_FILE).
- E14 No serializer exists for the eight Link macros (`MacroBank::Macro` = name "Link N", `manualValue`, links;
  src/routing/MacroBank.h:30-40, :62) or for user signals, while saved connections and `Action::AdjustMacro` name them (FO-M6).

Layout
- E15 A layout is `{"deckDividerY": int, "vDividerFrac": [3 floats]}` (MainComponent.cpp:7326-7332), applied only by Load Layout
  (:7336-7362) and Reset Layout (:7363-7370: -1, 0.22, 0.50, 0.75), each ending in `resized()`. `deckDividerY_` is clamped inside
  `resized()` (:2806-2811); `vDividerFrac_` is used raw (:2836-2858). A drag clamps each fraction between its neighbours with
  `kMinPanelWidth` (120 px, MainComponent.h:516) of gap (:7436-7451).
- E16 `resized()` places existing children only; it does not create or destroy a component, take keyboard focus, or touch
  OutputManager (FO Q5 last bullet, VERIFIED for :2740-2905; I read :2700-2745: the signal-bar branch hides panels and returns).

The list of shows, decks
- E17 `CompDecksBrowser`: two sections; a show row = any `*.json` of `~/Library/AudioDNA/compositions` (no content check), name +
  date, sorted by path; left click = `onCompositionLoad` -> `confirmReplaceShow` -> `loadComposition`; right click = Open / Show in
  Finder / Delete...; deck rows = files of `decks/` passing `isV2DeckFile` (a full parse on the message thread per refresh); no
  drag, no double-click, no hover (ui/CompDecksBrowser.cpp, whole file read; FO Q1, FO-V1..V5).
- E18 No reader takes names out of a show without a full load (FO-V6). A deck's name, id and numColumns sit in `decks[i]` of the
  show JSON in BOTH shapes; the canvas is `outputWidth` / `outputHeight` (FO Q1 line 20).
- E19 Load Deck: `appendDeckFromFile` -> `stageDeckAppend` parses on the message thread (MainComponent.cpp:3654), shape check =
  top-level "layers" only (:3657), `Deck::fromVar`, `ShowMigration::legacyRowSettings`, `validateDeck`, `remintClipIds`,
  `reconcileSourceParams`, name = file base name, `sourceFile = file` (:3700-3701), then `beginStagedOpen` -> `finishStagedLoad` ->
  ONE `InsertDeckCmd` "Load Deck" (:3373-3400). Queue: `stagedload::Queued {kind, file, ..., name}` (src/core/StagedLoad.h:148-154).
- E20 `InsertDeckCmd` (src/core/DeckCommands.h:777-832): fresh deck id via `appendDeck`; for every row index >= the show's layer
  count it adds a shared layer, taking `rowSettings[r]` when present (with a fresh layer id); Undo removes the deck and those
  layers. Clip ids are re-minted from `s_nextClipId`; a deck file carries no routine and `InsertDeckCmd` touches none (FO Q2, FO-V11, V12).
- E21 A bf9b-shape show passes Load Deck's shape check and becomes an empty deck named after the file with `sourceFile` = the show
  file, which Save Deck then overwrites with deck JSON (FO Q2(b), FO-V8, V9). An OLD-shape show is refused (FO W-1).
- E22 Save Deck / Save Deck As: MainComponent.cpp:3757-3827; menu MenuBarModel.cpp:86-89; tab menu and "+" menu
  src/ui/DeckTabRow.h:39-61; dispatch MainComponent.cpp:1384-1394; tooltip ui/DeckView.cpp:570-575 ("Save Deck As... adds it").
- E23 Probes that call `/api/debug/load_deck` with a deck-file fixture they write themselves: .harmony/probe-asan-live.sh:217,
  probe-async-load.py:957, probe-boxes.py:578.

Quit
- E24 Every quit path ends in `systemRequestedQuit()` -> `quit()`: the in-app Quit item (MainComponent.cpp:6606-6608), the window
  close button (src/Main.cpp:87-90), and macOS's own terminate (Cmd+Q, the app menu, logout / shutdown) through
  `applicationShouldTerminate:`, which calls `systemRequestedQuit()` and answers `NSTerminateCancel` when no stop message was
  sent (juce_events/native/juce_MessageManager_mac.mm:62-73). Nothing is asked; only a running take and an MP4 are finalised in
  `~MainComponent` (FO Q8).
- E25 No "changed since the last save" flag exists; Save does not clear undo and many edits are not undoable (FO Q8, FO-V "REFUTE 3").
- E26 `saveComposition()` / `saveCompositionAs()` are `void`; Save As runs in an async chooser callback; a cancelled chooser does
  nothing (MainComponent.cpp:3546-3617). `confirmReplaceShow` is an async OK / Cancel box used by New and the library row
  (:3168-3178); Composition > Open... and REST do not ask.

## 3 ITEMS

### OS1 THE SHOW FILE

Forks and choices
- OS1-a VERSION. Choice: a top-level integer `"version": 2`, written first by `toVar`. Reading: key present -> trusted as is; key
  absent -> the ONE grandfathered presence test that exists today (E2) names it 0 (old shape, converted by ShowMigration) or 1
  (bf9b shape); from 2 on, no reader ever looks at a key's presence to tell a shape. A file whose version is 2 or more never runs
  the legacy conversion, whatever keys it has. Runner-up: keep telling the shape by "layers" and add a version only "when
  needed" -- loses because the Harmony constraint forbids guessing the next shape, and E21 shows what one overloaded key cost.
  Rule for neighbours: a lane that adds an OPTIONAL top-level key read under `hasProperty` (the beat nudge's "beatNudgeMs")
  does not bump the version; the version changes only when an older reader would MISREAD the file.
- OS1-b NEW BLOCKS. `"keys"`: exactly `BindingManager::toVar()`'s object (`{"bindings":[...],"version":1}`), always written (an
  empty list is fine). `"layout"`: `{"deckDividerY": int, "vDividerFrac":[f,f,f]}`, always written. Choice of carrier: the two
  blocks are NOT model fields of the render-shared Composition and are NOT emitted by `toVar()` (E6: toVar runs on the HTTP
  thread). `saveToFile(file, const ShowExtras&)` (no default argument) adds them to the object; `loadFromFile` keeps the two raw
  vars and the file's version in message-thread-only members (`loadedExtras`, `loadedVersion`) that MainComponent takes and
  clears at the cut. Runner-up: typed `std::vector<Binding>` + layout fields inside Composition, written by toVar -- loses: it
  duplicates state owned by BindingManager / MainComponent, and puts a ref-counted var under an unlocked HTTP-thread reader.
- OS1-c ONE WRITER. `MainComponent::writeShow(const juce::File&) -> bool` is the only src caller of `Composition::saveToFile`:
  it snapshots the live keys and layout into a `ShowExtras`, runs the backup rule (OS1-e), writes, and on success stores the
  computer's copy of the keys (OS2-c). The three callers of E4 switch to it (Collect Media's show copy included, so a collected
  show carries its keys and layout).
- OS1-d DEAD KEYS. Dropped NOW (field, write and read; an old file that has them still loads, the keys are simply ignored):
  the seven with no reference outside Composition.h (E5) -- `outputDisplay`, the four `crossfader*`, `genrePresetNames`,
  `smartAutopilotEnabled`. plan-outputs.md:849 hands `outputDisplay`'s removal to this lane. NOT this lane: `bpmMultiplier` (the
  beat-nudge lane's question on the inert /4 /2 x1 x2 x4 buttons; plan-nudge.md:289 keeps the member); the five `autopilot*`
  keys (a control on screen sets them: removing a control is its own item, section 9 finding SF-2); `autoPresetOnGenre`,
  `genreDeckAssignment`, `structuralSceneEnabled` (deliberately inert, bf9b F15). Runner-up: drop nothing -- loses only on
  tidiness; if the council finds any reader, the seven stay and nothing else in the lane moves.
- OS1-e PROTECTION OF AN OLD-SHAPE SHOW (FO-M3). Choice: BEFORE any write over an existing target, `writeShow` reads the TARGET
  file from disk; if it parses to an object with a "decks" array and its version (OS1-a rule) is not 2, the file is copied byte
  for byte to `<target's folder>/backups/<base name>.v<N>.json` (N = 0, 1, or the later number). An existing backup with the same
  bytes is kept and nothing is copied; an existing one with other bytes is never overwritten (the copy takes the next free
  "<name> (2)" sibling). If the copy cannot be made, NOTHING is written and the save fails with today's "Save failed: <path>"
  box (the one failure box Boris kept: question 34's default). After one successful save the target is version 2, so no further
  backup is made. The rule is about the FILE, not about what is in memory, so it also covers Save As onto his old show, Collect
  Media's copy and the test route. The list of shows is not recursive (E17), so a backup is never listed as a show.
  Runner-up: send the first Save of an old-shape show to Save As -- loses: he can still pick the same name (the chooser's
  "Replace" is one click), it interrupts a Cmd+S on stage with a chooser, and it protects nothing on Save As.
  The same rule protects a show written by a LATER build (version 3+): it opens best-effort (unknown keys ignored, as today) and
  the first save over it keeps `<name>.v3.json`. Runner-up: refuse to open a later version -- loses: it needs a new failure text
  on screen (Boris 2026-10-04: "Nothing else."), and no such file exists until a later lane makes one and rules its own reader.
- OS1-f VERSION-LESS / OLD FILES ARE READ AS TODAY: same conversion, same one logged note (MainComponent.cpp:3355-3357). His show
  opens exactly as at 185147b; only a Save changes a file, and then with a backup.

Change by file / function
- NEW src/core/ShowFile.h (pure juce_core): `kShowVersion = 2`; `int versionOf(const juce::var&)` (present -> its int; absent ->
  0 or 1 by `ShowMigration::isLegacyShow`); `struct ShowExtras { juce::var keys, layout; }`; `juce::File backupFileFor(target, n)`;
  `enum class Backup { NotNeeded, Made, AlreadyThere, Failed }`; `Backup backupBeforeOverwrite(const juce::File& target)`.
- src/model/Composition.h: `toVar` writes "version" first and stops writing the seven keys; `fromVar` reads `versionOf` once,
  branches on it instead of calling `isLegacyShow` directly, stops reading the seven; the seven members go; `saveToFile(file,
  const ShowExtras&)`; `loadFromFile` fills `loadedVersion` / `loadedExtras` (raw "keys" and "layout" vars, void when absent);
  `ShowExtras takeLoadedExtras()`.
- src/MainComponent.cpp: `writeShow`; `saveComposition`, `saveCompositionTo`, Collect Media call it. (Keys / layout snapshot
  functions arrive in OS2 / OS3; in stage S1 `writeShow` passes empty extras and the blocks are written as empty objects.)
- src/ui/CompositionInspector.cpp and anything else naming a dropped member: none expected (E5); the builder greps first and
  STOPS with a report if a reader exists.
- tests: tests/test_composition.cpp "Composition full field" :386 (touches outputDisplay) and "Backward compatibility" :519 are
  edited for the dropped keys; tests/test_preset_manager.cpp :454 "Deck files never carry an output display" belongs to the v1
  deck pair and goes in S4b; tests/test_output_law.cpp:221-229 (forbids naming `outputDisplay`) stays green by construction.

RED-first tests (tests/test_show_file.cpp, tag [showfile]) and mutants -- strings in section 5.
Gate row: G-OS1 (section 5).

### OS2 KEYS AND MIDI

What he sees, case by case
- SAVE (Cmd+S, Save As, the library's Save Composition button, Save & Quit): the show file gets the live set in "keys"; the
  computer's copy (settings.json, new key `AppSettings::kKeys = "keys"`, the same object) is replaced by the same set. Nothing
  shows.
- OPEN of a show whose "keys" block holds at least one binding: at the cut (the moment the new show replaces the old one) the
  live set becomes the show's set, whole; the computer's copy becomes that set too. Nothing shows.
- OPEN of a show with NO "keys" block (his one show, every show saved before this lane), an EMPTY one, or an UNREADABLE one (not
  an object, no "bindings" array, inner version above 1): the show opens; the live keys stay exactly as they were; the
  computer's copy is not touched. Choice on "empty": an empty set is treated as "this show was saved before any key was set
  up". Runner-up: an empty set clears the live keys -- loses: no screen can clear bindings today (E9), so an empty set can only
  come from a show saved before any binding existed, and clearing would silently wipe a working controller setup.
- NEW: the live keys stay as they are at that moment. Choice. Runner-up: New re-reads the computer's copy (the set of the last
  show saved or opened), discarding key changes made since -- loses: it throws away the mapping he just made by pressing New,
  and where there are no unsaved key changes both rules give the same keys. (His words: "you import the setup from the most
  recent show".)
- LAUNCH: before any show is opened the app reads the computer's copy and makes it live. "Most recent" = the set that was live at
  the last Save of a show or the last Open that switched the set, whichever came later -- one copy, in settings.json, written at
  exactly those two moments. Choice of memory: a COPY of the set. Runner-up: remember the PATH of the most recent show and read
  its block at launch -- loses: the file can be moved, renamed, trashed or be his old-shape show with no block, and each of
  those would leave him with no keys at launch. Choice of moment: Save and Open only, never on each binding change, from his
  rule of 2026-10-04 11:59:06 ("if the user is changing things around settings, etc., and they do not save the composition,
  nothing is saved."). First launch after this lane on his machine: no "keys" in settings.json, his one show has no block ->
  empty, as today.
- IMPORT: Shortcuts > "Import Settings from Show..." opens a chooser in the compositions folder; the picked show's set REPLACES
  the live set, whole. The same command sits in a show row's right-click menu in the list ("Import Key and MIDI Settings"),
  enabled only when that show has at least one binding (a state, read from the list's summary, OS4). A picked file with no
  usable set changes nothing and shows nothing. An import is not a save: the computer's copy and the open show's file change
  only at the next Save. Choice replace over merge: today's Import replaces (E10); a merge needs a rule for two sets claiming
  one key and his words name none ("they can import the settings from another show file").
- EXPORT / IMPORT BINDINGS to their own file: go (menu items, handlers, `BindingManager::saveToFile` / `loadFromFile`). He has no
  such file (E7).
- OPEN QUESTION 50 as the task lists it: his words of 12:52:00 and 12:56:12 answer it (with the show AND on the computer). If he
  later wants the computer to remember every key change the moment it is made, the change is ONE call to the function that
  writes the computer's copy from the two overlays' exit hooks (MainComponent.cpp:1818, :1822); if he wants opening a show NOT
  to switch keys, it is the one apply call in `finishStagedLoad`.

Applying a set (one function, three callers: the cut of an Open, the launch, Import)
- `MainComponent::applyKeySet(const juce::var& block) -> bool`, message thread: (1) `BindingManager::parseSet(block)` -- a pure
  parse that touches nothing; no result or zero bindings -> return false, live keys untouched; (2) `exitAllBindingModes()` -- a
  learn or shortcut screen closes cleanly through its own exit, so its cached targets and capture state go with it; (3)
  `releaseAllMomentary()` -- every entry of `momentaryRefs_` is released through the existing `releaseMomentaryRefs(id)` and the
  map ends empty, so the clip a held key fired is let go BEFORE the ids change; (4) `bindingManager_.replaceAll(set)` -- new
  ids, relative-CC state cleared. A key or note released later finds no entry and does nothing (E12). A CC knob held in Relative
  mode restarts from 0.5 (accepted: E10, today's Import does the same).
- At an Open the order is: `exitAllBindingModes()` and `releaseAllMomentary()` BEFORE the model swap (they act on the old show;
  today a learn screen left open across an Open keeps targets of the old show -- E12 -- so this also mends that), then the swap,
  then `applyKeySet` and the layout (OS3) from `composition_.takeLoadedExtras()`. This holds for every route to a whole-show
  load: the list, Composition > Open..., REST `/api/load_composition`.
- Guard: `replaceAll` refuses (returns false, nothing changed) while BindingManager is inside a dispatch (a depth counter around
  the action callback, BindingManager.cpp:70 / :96 region); `applyKeySet` then re-posts itself once with `callAsync` through a
  `SafePointer`. No path reaches this today (INFERRED: every caller is a chooser callback, a staged-load completion or the
  constructor); the guard is for the next caller.
- Per-binding reading: unchanged field by field (E10), except that a binding whose `action` or `inputType` number is outside the
  known enum is dropped (a set written by a later build), and the rest of the set is kept.
- No allocation, lock or syscall is added on the audio callback, the analysis thread or the render thread: everything above is
  message-thread code; `momentaryRefs_` release uses the existing one-CAS-per-layer `releaseMomentary`.

Side question M-6 (the eight Link macros and the user signals have no serializer). Ruling: NOT built in this lane; filed as
finding SF-1 for Boris and as its own lane. Why: no fact sheet covers what a user signal is made of or how a macro's links are
rebuilt, and a serializer designed without reading that code is a guess; nothing in this lane makes it worse (the show's
saved connections name them today and will tomorrow). Runner-up: add `"macros"` / `"signals"` blocks here -- loses on the above
and on size. The block layout of OS1-b leaves room: two more optional top-level keys, no version bump.

Change by file / function
- src/binding/BindingManager.h/.cpp: `static std::optional<std::vector<Binding>> parseSet(const juce::var&)`; `bool
  replaceAll(std::vector<Binding>)`; `bool isDispatching() const`; `fromVar` becomes parse + replace (a void / broken var now
  leaves the list alone); `saveToFile` / `loadFromFile` go.
- src/model/AppSettings.h: `kKeys = "keys"`.
- src/MainComponent.cpp/.h: `applyKeySet`, `releaseAllMomentary`, `keysSnapshot()` (= `bindingManager_.toVar()`),
  `storeKeysOnComputer()`; the constructor reads `AppSettings(appSettingsFile(testMode_)).read(kKeys)` after the binding wiring
  (:1811-1826) and applies it; `finishStagedLoad`'s Composition branch (:3353-3363) gets the before / after calls; `writeShow`
  fills `ShowExtras::keys` and calls `storeKeysOnComputer()` on success; `case kShortcutsExportBindings` / `kShortcutsImportBindings`
  (:7279-7309) go; new `case kShortcutsImportFromShow`.
- src/ui/MenuBarModel.cpp/.h: the two items (:160-161) go; "Import Settings from Show..." takes their place.
- src/api/ApiServer.cpp (inside `#if AUDIODNA_TEST_SERVER`, answered on the message thread like `/api/debug/ui_text`):
  `GET /api/debug/keys`, `POST /api/debug/set_keys`, `POST /api/debug/import_keys`, `POST /api/debug/binding_mode`,
  `POST /api/debug/midi` (section 5 names their fields).
- tests: tests/test_routine_engine.cpp :758 (toVar / fromVar round trip, old file without targetRoutineSlot) stays green
  unchanged -- it is this lane's regression row for the per-binding format.
Gate row: G-OS2.

### OS3 THE WINDOW LAYOUT

- What is in it: exactly what a layout is today (E15): the deck divider and the three fractions. Window size and position, the
  browser tab, the inspector and output windows are not in it and are not added (INFERRED from his page's list item "the window
  layout" = what Save Layout saved; reading R34 was told to him).
- SAVE: `writeShow` puts the two live values in "layout".
- OPEN of a show that has a valid "layout": at the cut, after the swap, the values are set and `resized()` is called once.
- OPEN of a show with no "layout", or an invalid one: the window stays as it is. Valid means: an object; `vDividerFrac` an array
  of exactly three finite numbers, strictly increasing, each at least 0.05 above its left neighbour (0 for the first) and at
  most 0.95; `deckDividerY` an integer >= -1. Anything else skips the WHOLE block (never half of it). After that the three
  fractions are clamped with the drag's own rule (neighbour + `kMinPanelWidth / bottomAreaWidth_`, E15) against the window he
  has NOW, so an applied layout can only produce panel sizes a drag can produce; `deckDividerY` is clamped by `resized()` as
  today. This closes the sheet's unknown (a zero-width preview detaching the GL context, FO Q5) by construction.
- NEW: the window stays as it is (same rule as the keys; nothing of his is reset by New).
- RESET LAYOUT stays: it is a command, not a save, and it is his way back from a layout he does not like. Save Layout... and
  Load Layout... go (menu items MenuBarModel.cpp:167-168, handlers MainComponent.cpp:7313-7362). He has no layout file (E7).
- What applying touches while the show plays (E16): `resized()` moves existing panels; the preview's GL surface is resized as by
  a divider drag (the canvas is not: Pitfall 37); no component is made or destroyed, focus is not taken, output windows are not
  children of the main window and are not touched (Pitfall 40). One whole-window repaint at an Open, which already rebuilds the
  UI (Pitfall 57: a one-off, not a timer). INFERRED, not run: the signal-bar-expanded branch (:2700-2727) returns before the
  dividers are used, so a layout applied in that state takes effect when the bar is folded.
- Runner-up for the whole item: also apply the computer's last layout at launch -- loses: his words put the layout in the show
  only, and a launch has no show.

Change by file / function
- NEW src/ui/LayoutFit.h (pure): `struct WindowLayout { int deckDividerY; std::array<float,3> frac; }`;
  `std::optional<WindowLayout> parse(const juce::var&)`; `juce::var toVar(const WindowLayout&)`;
  `std::array<float,3> clampToDrag(std::array<float,3>, float minFrac)`.
- src/MainComponent.cpp/.h: `layoutSnapshot()`, `applyLayout(const juce::var&) -> bool`; `writeShow` fills `ShowExtras::layout`;
  `finishStagedLoad` calls `applyLayout`; the two cases go; `kViewResetLayout` unchanged.
- src/ui/MenuBarModel.cpp/.h: the two items and their ids go.
- src/api/ApiServer.cpp (test build): `GET /api/debug/layout`.
Gate row: G-OS3.

### OS4 DECKS THROUGH THE LIST OF SHOWS

The row layout (his screenshot resolume-show-decks-list.png, read: a header of two lines -- line 1 the show's name left and
"1280 x 720" right; line 2 a disclosure triangle and "4 Decks" left, "4 Oct 2026 11:54" right -- then one plain row per deck
name, indented)
- A SHOW HEADER is two lines, 40 px: line 1 = the file's base name (left, primary text) and the canvas "W x H" (right, secondary
  text); line 2 = a triangle (drawn as a path, not a glyph: Pitfall 6) + "N Decks" ("1 Deck" for one) left, the file's date right
  (the format the row shows today, CompDecksBrowser.cpp:291). Under an OPEN header: one 24 px row per deck, the deck's name,
  indented to the header's text column, in file order. Colours and fonts are Audio-DNA's own (reading R1's rule for copies of
  Resolume).
- The two section headers ("Compositions (n)", "Decks (n)") go: the list is only shows. The "Save Composition" button stays where
  it is (it saves the show). The empty state reads "No saved compositions" (a state, not an event).
- What opens / closes a header, and what a click does (Question 91, default A, built as A): a click anywhere on a header opens or
  closes its deck rows; a double-click on a header opens the show (through today's `confirmReplaceShow`, same text); a click on a
  deck row adds that deck to the current show as a new deck, undoable with Cmd+Z (the gesture a deck row has today, E17).
  Right-click on a header: "Open", "Import Key and MIDI Settings" (OS2), "Show in Finder", separator, "Delete..." (today's confirm
  and Trash). Right-click on a deck row: "Add to This Composition". Menus: `showMenuAsync` with
  `.withParentComponent(getTopLevelComponent())`, as today (:232-234). Every header starts closed; open / closed is kept per
  path for the session (not saved).
  Runner-up: keep "one click on the show's name opens the show" and put open / close on the triangle only -- loses: two different
  actions 20 px apart in one row, one of which replaces everything playing. Runner-up 2: drag a deck row onto the deck tab row
  -- not built: his words ask for where decks are found, not for a drag; a drag needs a DragAndDropContainer path and a drop
  target on the tab row, and adds nothing a click does not do.
- Sort order (Question 92, default A, built as A): by name, as today (FO-M4), so a show keeps its place in the list.
- No drag, no hover highlight, no selection (none today, E17); no slider anywhere in this lane.

How the names are read (off the message thread), cached, refreshed
- NEW src/core/ShowSummary.h (pure): `struct ShowSummary { bool isShow; int version; int canvasW, canvasH; std::vector<std::string>
  deckNames; int keyCount; }` and `ShowSummary summarize(const juce::var& root)`: `isShow` = the root is an object with a "decks"
  array of at least one object (the same floor `compload::validateComposition` uses to refuse "no decks"); names =
  `decks[i].name` (an empty name reads "Deck <i+1>"); canvas = outputWidth / outputHeight (absent or <= 0 -> 1920 x 1080, the
  loader's own default, Composition.h:746-751 region); `keyCount` = the size of `keys.bindings` when it is an array, else 0.
  It reads the same keys in both shapes (E18): no conversion, no model object.
- The list's refresh (message thread) only LISTS the folder (as today) and compares each file's (path, modification time, size)
  with a cache. For each new or changed file it posts ONE job to a single background thread owned by the browser (a
  `juce::ThreadPool` of one thread): read the file, `JSON::parse`, `summarize`. The job hands its result back with
  `MessageManager::callAsync` guarded by a `Component::SafePointer`; the message thread stores it in the cache and repaints the
  list. No mutex is added: a job owns everything it touches and the cache is message-thread-only. The browser's destructor
  removes the jobs and waits for the running one (bounded by one file read).
- A file is shown as a header only once its summary says `isShow`. A file that is not a show (a preset, a deck file, garbage) is
  not listed at all -- today it is listed and then fails to open (E17). While a summary is pending the file is simply not there
  yet (tens of milliseconds after launch; cached afterwards).
- Refresh moments: today's five call sites and Delete (FO-V5), plus when the Compositions tab becomes visible
  (`CompDecksBrowser::visibilityChanged`), so a show copied into the folder appears when he next looks at the tab.
- An OLD-SHAPE show in the list (his one show): listed like any other -- "test with harry", its canvas, "2 Decks", its date; open
  it and the rows read "Deck 1" and "Deck 2" (E7). Listing never writes or converts the file.

Taking a deck from another show's file
- Route: `MainComponent::appendDeckFromShow(const juce::File& show, int deckIndex, const std::string& deckName)`; it replaces
  `appendDeckFromFile` as the one entry of `stagedload::Kind::DeckAppend`. The queue entry (`stagedload::Queued`, E19) gains the
  deck index; behind a staged load it waits its turn exactly as today (AL7), and a whole-show load supersedes it (Pitfall 58).
- Staging (the same place and thread as today's `stageDeckAppend`, E19): parse the show into a PRIVATE donor `Composition`
  (`fromVar`: an old-shape donor is converted in memory by ShowMigration, never on disk); pick the deck: `decks[deckIndex]` when
  its name equals `deckName`, else the first deck with that name, else nothing happens (one log line; no box). Then
  NEW `compload::deckFromShow(Composition& donor, int index) -> std::optional<TakenDeck>` (src/core/CompositionLoad.h), where
  `TakenDeck = { Deck deck; std::vector<std::optional<Layer>> rowSettings; }`:
  - ROWS: the deck keeps its rows up to its LAST row that holds a clip (at least one row); the empty rows above that are cut. A
    donor deck is padded to the donor show's layer count (Composition.h `normalizeRows`), so without the cut a 2-row deck from a
    6-layer show would add four empty layers to his show.
  - LAYERS THE DECK NEEDS: `rowSettings[r]` = the donor show's shared layer r (name, opacity, blend, layer effects, connections).
    `InsertDeckCmd` uses an entry ONLY for a row that adds a layer to the current show (E20: the wide-deck rule) and gives it a
    fresh layer id; a row that lands on an existing layer keeps that layer's own settings (CLAUDE.md rule 15). A deck with fewer
    rows is padded.
  - then, as today: `validateDeck`, `remintClipIds` (fresh clip ids from `s_nextClipId`, Pitfall 36), `reconcileSourceParams`,
    `beginStagedOpen` (media opens off the message thread; a missing file marks the clip, nothing is relinked: media paths are
    the absolute paths the donor show holds), `finishStagedLoad` -> ONE `InsertDeckCmd` "Load Deck" (fresh deck id from
    `appendDeck`; refused with today's log line when the session has used every deck number).
  - The deck's name is its name in the donor show (not the file's name). Two decks with one name are allowed (true today).
- NOT taken: the donor's routines and pad bank (a routine names a deck by index + name inside its own show; E20), its global
  effects, its keys, its layout, its other decks, the settings of layers the current show already has. A connection on a taken
  clip that names a Link macro or a signal rides along as it is (SF-1).
- Undo: one step, exactly today's Load Deck undo (the deck goes; the layers it added go with it; test T6g / T6h / T6i stay).
  Undo history is not cleared.
- Taking a deck from the file of the show that is open now gives the deck as SAVED, as a new deck. Allowed.

What goes with it (same stage, S4b)
- Deck > "Load Deck...", "Save Deck", "Save Deck As..." (MenuBarModel.cpp:86-89 and their ids; MainComponent.cpp:6750-6756);
  the tab's right-click "Save Deck" / "Save Deck As..." and the "+" menu's "Load Deck..." (DeckTabRow.h:39-61;
  MainComponent.cpp:1388-1390). The "+" then has one meaning and adds a deck directly (no one-item menu); its tooltip becomes
  "New Deck". The tab tooltip loses its first line (DeckView.cpp:572-573) and reads "Double-click: rename" / "Right-click:
  Rename / Duplicate / Remove".
- `loadDeck`, `saveDeck`, `saveDeckAs`, `writeDeckFile`, `stageDeckAppend`, `appendDeckFromFile` (MainComponent.cpp:3625-3827
  minus `newDeck`), `Deck::sourceFile` (model/Deck.h:19; core/CompositionLoad.h:163), `CompDecksBrowser::isV2DeckFile`,
  `getDecksDir`, `onDeckLoad`, the Decks section, `ShowMigration::legacyRowSettings` (ShowMigration.h:198-217: it read rows of a
  DECK FILE; the take path reads the donor's layers instead), and the dead v1 pair `PresetManager::saveDeck / loadDeck /
  getDeckDirectory` (no caller, FO Q6 line 84).
- The hole of E21 closes by construction: no code reads a file as a deck by the "layers" key any more, nothing writes a deck
  file, and a taken deck remembers no file.
- Deck files on disk: never read, never listed, never deleted, never moved. His four v1 files are already unreadable (E8); he
  has no v2 deck file (E7). Harmony constraint (pre-merge check, row G-OS4-0): `ls` of his decks folder is repeated before the
  merge; if a v2 deck file has appeared since, the lane does not merge until Harmony has wrapped a COPY of it into a one-deck
  show file in the compositions folder (a file edit by hand, his original untouched).
- The test route `/api/debug/load_deck` keeps its name and takes `{"path": <show file>, "deck": <index, default 0>, "name":
  <optional>}`; the three probes of E23 write their fixture as a one-deck SHOW (`{"version":2,"layers":[...],"decks":[deck]}`)
  instead of a bare deck. `stagedload::doneLabel` and the staged-load tests are untouched.

States the visual gate must capture (OS4): V1 the list empty; V2 one show, closed; V3 the same show open with 2 decks; V4 three
shows, the middle one open, the others closed; V5 a show with 12 decks open (the list scrolls); V6 a 60-character show name and a
60-character deck name (elision; the canvas and the date still readable); V7 the old-shape fixture show open ("Deck 1" /
"Deck 2"); V8 the header's right-click menu with "Import Key and MIDI Settings" enabled; V9 the same menu on a show with no keys
(item greyed); V10 a deck row's right-click menu; V11 the Delete confirm; V12 the list at the browser's narrowest width; V13 the
Deck menu, the tab's right-click menu and the "+" after the removals; V14 the Shortcuts and View menus after the removals;
V15 the tab tooltip.

Change by file: src/ui/CompDecksBrowser.h/.cpp (rewritten list content, the reader, callbacks `onCompositionLoad`,
`onDeckTake(file, index, name)`, `onImportKeys(file)`, `onCompositionSave`); NEW src/ui/ShowListModel.h (pure: summaries + open
set -> rows, hit test, menu items); NEW src/core/ShowSummary.h; src/core/CompositionLoad.h; src/core/StagedLoad.h (one field);
src/MainComponent.cpp/.h; src/ui/DeckTabRow.h, DeckView.cpp, MenuBarModel.cpp/.h; src/model/Deck.h, ShowMigration.h;
src/ui/PresetManager.cpp/.h (the dead deck pair only); src/api/ApiServer.cpp (test routes); tests and probes per OS6.
Test-mode library folder: `CompDecksBrowser::getCompositionsDir()` answers `AUDIODNA_LIBRARY_DIR` (absolute path) in a
test-server build running --test-mode, else a scratch folder in the temp directory -- the pattern of `appSettingsFile` (E13) --
so no gate lists or writes his real folder.
Gate rows: G-OS4-0..G-OS4-3.

### OS5 THE QUIT WINDOW

- Text and buttons, as his screenshot (resolume-quit-dialog.png, read): window title "Quit!"; heading "Quit!"; message "Do you
  really want to quit?" / "All unsaved progress will be lost."; three buttons, left to right "Quit", "Cancel", "Save & Quit",
  the last one drawn as the default (accent). Esc = Cancel; Return = the default button (Question 93, default A = Save & Quit).
  No icon (Audio-DNA has no logo mark in the app; NoIcon as `confirmReplaceShow` uses). It is one of the texts he allows (the quit window).
- EVERY quit shows it (reading R18, told to him: "the quit window appears every time you quit"). Runner-up: only when something
  changed. Cost of that: a "changed" flag set on every path that changes the show -- undoable commands, the non-undoable writes
  (hand-moved values, Save Routine, key and MIDI learn, the dividers), REST, OSC, MIDI and bound keys, routine replays -- and
  cleared on Save, Open, New; one missed path quits without asking and loses work silently. No flag exists (E25) and his words
  do not ask for one: his screenshot's text is unconditional. Not built.
- Every path reaches it through ONE function: `AudioDNAApplication::systemRequestedQuit()` (E24) stops calling `quit()` and
  calls `MainComponent::requestQuit()`. That covers the in-app Quit item, the window's close button, Cmd+Q and the app menu's
  Quit, and a logout / shutdown (macOS is answered "cancel" and shows the window; after "Quit" the app ends -- as any app that
  asks). In test mode `requestQuit()` quits at once, no window (Harmony constraint; gates quit by pid).
- The flow (NEW pure src/core/QuitFlow.h: states Idle, Asking, Saving; inputs Request, ChoseQuit, ChoseCancel, ChoseSaveAndQuit,
  SaveDone(Saved | Cancelled | Failed); outputs ShowWindow, StartSave, DoQuit, Nothing):
  - Idle + Request -> Asking, ShowWindow. A second Request while Asking or Saving does nothing (the window comes to the front).
  - Asking + ChoseCancel -> Idle. Nothing stops, nothing is saved.
  - Asking + ChoseQuit -> DoQuit: `JUCEApplication::quit()`; the shutdown is today's (a running take and a video recording are
    finalised by `~MainComponent`, E24). Nothing else is written.
  - Asking + ChoseSaveAndQuit -> Saving, StartSave: `saveComposition(onDone)`. A show WITH a file is written to it (through
    `writeShow`: the backup rule applies). A show with NO file opens today's Save As chooser (reading R18, told to him).
  - Saving + SaveDone(Saved) -> DoQuit. Saving + SaveDone(Cancelled) (he closed the chooser) -> Idle: the app stays open, nothing
    shows. Saving + SaveDone(Failed) -> Idle: today's "Save failed: <path>" box shows and the app stays open.
- `saveComposition` / `saveCompositionAs` gain an optional completion `std::function<void(SaveResult)>` (Saved, Cancelled,
  Failed), called exactly once; every existing caller passes none and behaves as today.
- A take or a recording running: unchanged by this lane -- Quit and Save & Quit end them as a quit does today. The window does
  not stop them while it is open, and Cancel leaves them running.
- While the window is open the show keeps playing: the box is asynchronous (`enterModalState` with a callback, no modal loop, as
  `confirmReplaceShow`, MainComponent.cpp:3168-3178); the render, analysis and audio threads are not involved. A modal box holds
  the keyboard, so bound KEYS do not fire while it is open; MIDI still does; output windows never take the keyboard (Pitfall 40).
- New and Open keep their own confirm (`confirmReplaceShow`, E26) with its text unchanged; Composition > Open... still does not
  ask. Not asked for; not changed.
- The window: `juce::AlertWindow` built by hand (title, message, NoIcon, parent = the main component), `addButton` three times
  in the order above, shown with `enterModalState(true, callback, true)`; it draws with the app LookAndFeel like the other boxes.
  Runner-up: a custom DialogWindow laid out to the pixel of Resolume's (Quit far left, the other two right) -- held back: the
  visual gate's critics compare state Q1 with his screenshot; if they fail the layout, the fix round swaps the component and
  nothing else in the flow changes.
- States the visual gate must capture (OS5): Q1 the quit window over a playing show (the app's own window only, by window id;
  opened by the test route `POST /api/debug/quit_window`, whose buttons are inert); Q2 the same at the app's minimum window size.

Change by file: src/Main.cpp (`systemRequestedQuit`, `closeButtonPressed` unchanged); src/MainComponent.cpp/.h (`requestQuit`,
`showQuitWindow`, the completion on the two save functions); NEW src/core/QuitFlow.h; src/api/ApiServer.cpp (test routes
`POST /api/debug/quit_flow`, `GET /api/debug/quit_flow`, `POST /api/debug/quit_window`).
Gate row: G-OS5.

### OS6 WHAT GOES AND WHAT STAYS

6.1 Table 1 of facts-one-save, row by row (file:line as in the table, re-read where this plan changes the row)
| label as painted | verdict | what changes |
|---|---|---|
| New Composition (MenuBarModel.cpp:70) | STAYS | keys and layout stay as they are (OS2, OS3) |
| Open... / Cmd+O (:71) | CHANGES | the cut also applies the show's keys and layout |
| Save / Cmd+S (:73) | CHANGES | through `writeShow`: version, keys, layout, backup rule, the computer's copy of the keys |
| Save As... (:74) | CHANGES | same |
| Collect Media... (:76) | STAYS a command of its own | its show copy is written through `writeShow` (S1) |
| Relocate Missing Files..., Undo / Redo | STAY | -- |
| Save Composition button (CompDecksBrowser.cpp:177-181) | STAYS | -- |
| show row, left click (:103-106) | CHANGES | a click opens / closes the decks; a double-click opens the show (Q91) |
| show row, right click (:215-273) | CHANGES | + "Import Key and MIDI Settings" |
| deck row, left / right click (:133-136, :215-273) | GOES as a deck FILE row | replaced by the deck rows under a show |
| Deck > Load Deck... (MenuBarModel.cpp:86) | GOES | S4b |
| "+" tab: New Deck / Load Deck... (DeckView.cpp:549) | CHANGES | "+" adds a deck directly |
| Save Deck, Save Deck As... (MenuBarModel.cpp:88-89; tab menu) | GO | S4b |
| Duplicate Deck, Replace Content... | STAY | -- |
| row-1 Save, Load, FX Save; the ten slots | per question 86 (6.4) | -- |
| Edit Keyboard Shortcuts... / Edit MIDI Mappings... / Stop All (:155-158) | STAY | -- |
| Export Bindings..., Import Bindings... (:160-161) | GO | "Import Settings from Show..." takes their place (S2) |
| Save Layout..., Load Layout... (:167-168) | GO | S3 |
| Reset Layout (:169) | STAYS | -- |
| Save Routine button, Record Take / Load Take / Repair / Play Take | STAY | the routine is saved with the show, as today (R35) |
| Snapshot (:143), Start / Stop Recording | STAY | -- |
| Preferences..., Import ISF Shader..., Restore Last Outputs, Open Image / Image Folder | STAY | -- |
| Quit (:28) and the close button | CHANGES | the quit window (S5) |
| REST POST /api/load_composition | CHANGES | applies keys and layout at the cut, as Open |
| REST GET /api/composition | CHANGES | gains "version"; loses the seven dead keys; never carries "keys" / "layout" |
| REST /api/snapshot, /api/render_frame, /api/perf/*, /api/routine/save | STAY | -- |
| TEST /api/debug/load_deck | CHANGES | `{path: show file, deck, name}` |
| TEST /api/debug/duplicate_deck | STAYS | -- |
| TEST /api/debug/save_composition | CHANGES | `{path}` = Save As to the path (as today); `{}` = a plain Save |
| OSC /audiodna/snapshot; binding actions Snapshot / ToggleRecording / TriggerRoutine | STAY | -- |
| "NO route exists for" row | still true for decks, layouts, looks; keys get test-only routes | -- |
OSC: no route saves or loads a show, a deck, keys or a layout today (FO Table 1 last row); none is added.

6.2 Table 2: what turns red and what replaces it
| test / probe | turns red because | replaced by |
|---|---|---|
| tests/test_comp_decks_browser.cpp (4 cases, `isV2DeckFile`) | the function and the Decks section go (S4b) | tests/test_show_summary.cpp + test_show_list_model.cpp |
| tests/test_composition.cpp :386, :519 | the seven keys go (S1) | edited in place; new cases in test_show_file.cpp |
| tests/test_composition.cpp :877, :908, :1277-1279 | `saveToFile` takes `ShowExtras` | call sites pass `{}` |
| tests/test_show_model.cpp M3 :1431, M6 :1556 | they load a DECK FILE through `legacyRowSettings` | tests/test_take_deck.cpp TD-3, TD-4 (the same two outcomes, from a donor SHOW) |
| tests/test_show_model.cpp M2, M7, T6g/h/i, AS1, AS6; tests/test_undo_commands.cpp :1605, :1646 | must stay GREEN unchanged | (regression rows) |
| tests/test_deck_tab_row.cpp :52 | menu items change | the same case, new expected lists |
| tests/test_preset_manager.cpp :411 "Deck round-trip", :454 "Deck files never carry an output display" | the dead v1 deck pair goes (S4b) | none (dead code) |
| tests/test_routine_engine.cpp :758 | must stay GREEN unchanged | (regression row for the per-binding format) |
| tests/test_staged_load.cpp, test_load_ticket.cpp, test_media_opener.cpp, test_media_presence.cpp | must stay GREEN; `Queued` gains a field | -- |
| tests/test_app_settings.cpp | must stay GREEN; one new case for `kKeys` beside the others | -- |
| tests/test_lookandfeel_square.cpp | draws CompDecksBrowser rows | updated to the new row painter |
| probes probe-asan-live.sh L1, probe-async-load.py a6, probe-boxes.py K1a | deck-file fixtures | one-deck show fixtures (S4a edits the three scripts) |
| probe-boxes K7 (old show save + reload) | the saved file gains "version"/"keys"/"layout" and a backup appears beside it | K7's expectation updated in S1 |

6.3 Docs (each line moves in the stage that changes the behaviour): CLAUDE.md (the project-identity paragraph where it names
saves; "Deck tab row" under UI Patterns; a new pitfall index line "NN"); docs/claude/pitfalls.md (Pitfall NN, text in S1:
"the show file has a version; `Composition::saveToFile` is called only by `MainComponent::writeShow`; keys / layout blocks are
optional and a bad block is skipped; an older file is copied to backups/ before it is overwritten" -- Harmony assigns the
number); docs/claude/performance-controls.md :52 (old shows), :67 (the tab row); docs/claude/integration.md :26 (the
`outputDisplay` sentence), the REST table, the settings.json keys; docs/claude/architecture.md (the new files);
.harmony/APP-INVENTORY.md (menus, routes, counts).

6.4 The effects looks, by his answer to question 86 (.harmony/.reports/s-rta-1004/boris-clarify-86.md: "ANSWERS (none yet)")
- No answer: the lane touches none of the row-1 Save, Load, FX Save or the ten slots. Stage S7 does not start.
- A (remove now): stage S7 removes the three buttons, the ten slot pairs, `savePreset`, `loadPreset`, `fastSave`,
  `loadSlotPreset`, `populateSlotMenu`, `getFastSaveDir`, `PresetSlot`, `PresetManager` and tests/test_preset_manager.cpp
  (FO Q6 last bullet lists the lines); the legacy chain, `MappingEngine` and `EffectsRackPanel` stay; his `Presets/` (4 entries)
  and `FX Saves/` folders are left on disk untouched. Visual gate state V16: row 1 and the bottom bar after the removal.
- B (keep until the new looks exist): nothing; S7 is struck.
Either way this lane removes the DEAD v1 deck pair inside PresetManager (S4b), which no button reaches.

### OS7 PROOF (the full strings, bars and RED arms are section 5)
- Unit, RED first: the round trip of each block (test_show_file SF-1..SF-9, test_key_set KS-1..KS-8, test_layout_fit LF-1..LF-5);
  an old-shape file, a version-less bf9b file, a broken keys block, a show from a later version (SF-3..SF-7); the backup rule
  (test_show_backup SB-1..SB-6); the summary and the list rows (SS-1..SS-6, SL-1..SL-6); taking a deck (TD-1..TD-7); the quit
  flow (QF-1..QF-8); the "nothing dead left" lints (LINT-1..LINT-5).
- Live, through test-server routes, every row Harmony's (probe .harmony/probe-one-save.sh, written by the builder of each
  stage, never run by a builder): open a show and read the live keys (L1-L3 via `/api/load_composition` + `GET
  /api/debug/keys`); New (L4 via `POST /api/debug/new_composition`); launch (L5: `AUDIODNA_SETTINGS_FILE` pointing at a scratch
  settings.json that holds "keys"); Save (L6); a held key and an open learn screen (L7, L8 via `POST /api/debug/midi`,
  `/api/debug/binding_mode`, `/api/debug/import_keys`); layout (L9, L10 via `GET /api/debug/layout`); take a deck from another
  show and read ids and layers (L11, L12 via `/api/debug/load_deck`); the backup on the first Save of an old-shape show (L13,
  L14); the list (L15, L16 via `GET` / `POST /api/debug/show_list`); the quit window's logic with no synthetic input and no
  window (L17-L20 via `POST /api/debug/quit_flow {choice, path}`, which feeds the flow the choice a button would give and, for
  a show with no file, the path a chooser would give).
- Mutants: one named mutant per row (MU-OS-1..MU-OS-16, section 5).

### OS8 STAGES AND ORDER (detail in section 4)
S1 show file -> S2 keys -> S3 layout -> S4a take a deck (model + route + probes) -> S4b the list + the removals -> S5 quit
window -> VG visual gate -> S7 (only on 86 = A). One lane, one worktree, strictly in this order (every stage edits
MainComponent.cpp).

## 4 STAGES + ORDER

One lane (`lane/one-save`, one worktree), stages strictly in order; each stage is ONE builder context, ends with the full ctest
green and its own unit rows RED-then-GREEN shown in its report, and is reviewed pinned before the next starts. A builder never
launches the app, never runs a live row, never gives a gate verdict: every live row (L1-L20), every mutant arm on a live row,
the visual gate and every verdict are Harmony's. Docs move in the stage that changes the behaviour (OS6.3).

| stage | owns (files) | proves (unit, by the builder) | Harmony runs |
|---|---|---|---|
| S1 show file | NEW src/core/ShowFile.h; src/model/Composition.h; MainComponent.cpp (`writeShow`, the three callers); tests/test_show_file.cpp, test_show_backup.cpp, test_composition.cpp edits; docs: pitfalls NN, integration.md :26; probe-boxes K7 expectation; NEW .harmony/probe-one-save.sh (skeleton + L13, L14) | SF-1..SF-9, SB-1..SB-6, LINT-1; MU-OS-1..5 red in unit | G-OS1: L13, L14; probe-boxes K7 |
| S2 keys | src/binding/BindingManager.h/.cpp; src/model/AppSettings.h (one key); MainComponent.cpp/.h (apply, launch read, Import, the cut hooks; Export / Import cases out); ui/MenuBarModel.cpp/.h; api/ApiServer.cpp/.h (keys, set_keys, import_keys, binding_mode, midi, new_composition routes); tests/test_key_set.cpp, test_app_settings.cpp (+1); docs | KS-1..KS-8, LINT-2; MU-OS-6, 9 red in unit | G-OS2: L1-L8 with MU-OS-7, 8, 9, 10, 17 |
| S3 layout | NEW src/ui/LayoutFit.h; MainComponent.cpp/.h (snapshot, apply, Save / Load Layout cases out); ui/MenuBarModel.cpp/.h; api/ApiServer (layout route); tests/test_layout_fit.cpp; docs | LF-1..LF-5, LINT-3; MU-OS-11 red in unit | G-OS3: L9, L10 |
| S4a take a deck | src/core/CompositionLoad.h (`deckFromShow`); src/core/StagedLoad.h; MainComponent.cpp/.h (`appendDeckFromShow`, the queue, the test route's contract; the old deck-file entry still compiled and reachable from the UI until S4b); api/ApiServer (load_deck fields); tests/test_take_deck.cpp; the three probes of E23 (fixtures) | TD-1..TD-7; MU-OS-13, 14 red in unit | G-OS4-1: L11, L12; probe-asan-live L1, probe-async-load a6, probe-boxes K1a re-run green |
| S4b the list + the removals | NEW src/core/ShowSummary.h, src/ui/ShowListModel.h; src/ui/CompDecksBrowser.h/.cpp; MainComponent.cpp/.h (browser wiring; `loadDeck`, `saveDeck`, `saveDeckAs`, `writeDeckFile`, `stageDeckAppend`, `appendDeckFromFile` out); ui/DeckTabRow.h, DeckView.cpp, MenuBarModel.cpp/.h; model/Deck.h, ShowMigration.h; ui/PresetManager.cpp/.h (dead deck pair); api/ApiServer (show_list routes; AUDIODNA_LIBRARY_DIR); tests: test_show_summary.cpp, test_show_list_model.cpp, test_deck_tab_row.cpp, test_lookandfeel_square.cpp; test_comp_decks_browser.cpp, M3 / M6, two preset-manager cases removed; docs: CLAUDE.md, performance-controls.md :52 :67, APP-INVENTORY | SS-1..SS-6, SL-1..SL-6, LINT-4; MU-OS-12 red in unit | G-OS4-0 (his decks folder), G-OS4-2: L15, L16 with MU-OS-18 |
| S5 quit window | NEW src/core/QuitFlow.h; src/Main.cpp; MainComponent.cpp/.h (`requestQuit`, `showQuitWindow`, the save completion); api/ApiServer (quit routes); tests/test_quit_flow.cpp; docs | QF-1..QF-8, LINT-5; MU-OS-15, 16 red in unit | G-OS5: L17-L20 |
| VG visual gate | a capture builder (window-id captures only, states V1-V15, Q1, Q2 with a manifest of model facts per state), then five critic seats given his words verbatim, his two screenshots, and the list of texts that pre-date the lane | -- | the verdict; a fix round goes back to S4b / S5's files |
| S7 looks off (only if 86 = A) | MainComponent.cpp/.h (row 1, the slot bar), ui/PresetManager.*, CMakeLists entries, tests/test_preset_manager.cpp, docs | LINT-6 | state V16; row-1 layout capture |

Order against the lanes planned beside this one
- MESSAGES lane: the failure box that stays is a failed Save of the show -- "Save Composition" / "Save failed: <path>"
  (MainComponent.cpp:3557-3562, :3609-3615); this lane keeps both sites and routes a failed backup to the same box. The
  "Save Deck" box (:3822-3826) and the two "Load Deck" boxes (:3659-3663, :3676-3680) LEAVE with their functions in S4b: the
  messages lane must not plan on them. This lane adds no text that announces an event or a failure; the labels "Saved: <file>" /
  "Loaded deck: <name>" (`setFileLabel`) are the messages lane's to rule and are not touched here except that "Saved deck: ..."
  (:3814) goes with `writeDeckFile`.
- BEAT NUDGE lane: its number is a show field ("beatNudgeMs", plan-nudge.md:342, an optional top-level key beside
  `bpmMultiplier`): additive, no version bump (OS1-a), written by `toVar`, so it is saved by `writeShow` with no change here.
  Its binding action and step field (plan-nudge.md:573) live in `BindingManager::toVar / fromVar`, which this lane reuses
  whole: whichever lane merges second re-bases `parseSet` (a builder's step 0). `bpmMultiplier` stays untouched here.
- OUTPUT SETTINGS lane: per machine, never in the show (ruling told to Boris as R23). It adds a settings.json key and mends
  `AppSettings::update` (plan-outputs.md:356, :375); this lane adds the key `kKeys` to the same header -- two one-line edits of
  src/model/AppSettings.h; the second to merge re-bases. `outputDisplay` leaves the show HERE (plan-outputs.md:849).
- TRANSPORT lane: clip fields and the clip panel; it writes inside `Clip::toVar` only. No shared function with this lane except
  the version rule of OS1-a (an optional clip key needs no bump).
- No stage of this lane runs while another lane's perf A/B is in progress (RIG-RULES: commits fire the graph rebuild).

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened)

Fixtures (built in code by tests/ShowFixture.h helpers or written by the probe; none is a file of his):
F-A v2 show, 2 layers, deck "A1", keys = 3 bindings (Keyboard 81 TriggerClip L0 C0; MidiNote 36 ch 1 TriggerColumn 1; MidiCC 7
MasterOpacity), layout {300, [0.30, 0.55, 0.80]} | F-B v2 show, keys = 1 binding (MidiNote 41 TriggerColumn 1) | F-NOKEYS v2
show with neither block | F-EMPTY v2 show, keys.bindings = [] | F-BROKEN v2 show, "keys": "x", "layout":
{"vDividerFrac":[0,0,0]} | F-OLD old shape as his (no top-level "layers"; decks "Deck 1" id 0, "Deck 2" id 100; rows carry
layer settings and "persistent"; "globalTransitionSpeed") | F-V1 bf9b shape, no "version" | F-V3 "version": 3 plus an unknown
key "futureThing" | F-DONOR v2, 6 layers (layer 3 named "Donor L4", opacity 0.25), decks "Narrow" (clips in rows 0-1) and
"Wide" (clips in rows 0-3), 1 routine on pad 0, keys = 2 | F-HOST v2, 2 layers, 1 deck, 1 routine on pad 0 | F-MOM a set with
one Momentary MidiNote 40 TriggerClip L0 C0.

Unit rows (Catch2 case titles are these strings; RED arm = the tree before the stage -- the case does not compile or fails --
AND the named mutant)
- SF-1 "showfile: toVar writes version 2 first and saveToFile adds keys and layout" [showfile] -- MU-OS-1 (toVar omits
  "version").
- SF-2 "showfile: keys and layout round-trip through saveToFile / loadFromFile as loadedExtras, and toVar never carries them".
- SF-3 "showfile: an old-shape file reads as version 0 and converts exactly as before (decks, layers, the one note)" -- MU-OS-2
  (`versionOf` answers 1 for every version-less file).
- SF-4 "showfile: a version-less bf9b file reads as version 1 and loads unchanged".
- SF-5 "showfile: a version-2 file is never converted by key presence" (a v2 object without "layers" loads with default layers,
  no migration note) -- MU-OS-3 (`fromVar` calls `isLegacyShow` whatever the version).
- SF-6 "showfile: a broken keys or layout block still loads the show; loadedExtras carry the raw vars".
- SF-7 "showfile: a version-3 file loads best-effort, unknown keys ignored, loadedVersion 3".
- SF-8 "showfile: the seven dropped keys are neither written nor needed; a file that has them loads".
- SF-9 "showfile: old show -> save -> load -> save gives two equal files that carry version 2" (M2's twin with the version).
- SB-1 "showbackup: an old-shape target is copied byte for byte to backups/<name>.v0.json before the write" -- MU-OS-4
  (`backupBeforeOverwrite` always answers NotNeeded).
- SB-2 "showbackup: a version-1 target gives .v1.json, a version-3 target .v3.json; a version-2 target gives none".
- SB-3 "showbackup: a second save makes no second backup; an existing identical backup is kept".
- SB-4 "showbackup: when the copy cannot be made nothing is written and the target's bytes are unchanged" -- MU-OS-5 (a Failed
  backup is ignored).
- SB-5 "showbackup: an existing different backup is never overwritten (the copy takes the next free name)".
- SB-6 "showbackup: a target that is missing, or is not a show, needs no backup".
- KS-1 "keyset: parseSet reads a set field by field and touches no manager" [keyset].
- KS-2 "keyset: replaceAll swaps the whole list, mints new ids, clears relative-CC state".
- KS-3 "keyset: a void var, a string, an object without a bindings array, an inner version 2 -> no set; fromVar then leaves
  the list alone" -- MU-OS-6 (a missing array parses as an empty set and replaces).
- KS-4 "keyset: an empty set is no set".
- KS-5 "keyset: a binding with an unknown action or input type is dropped, the rest kept".
- KS-6 "keyset: replaceAll refuses while a dispatch is running and changes nothing".
- KS-7 "keyset: the apply order -- modes exited, held momentary refs released, then replaced" (a seam: `keyapply::run(hooks)`
  records the order of three callbacks) -- MU-OS-9 (the release step removed).
- KS-8 "keyset: toVar of a replaced set equals the set parsed (22 fields)".
- LF-1 "layoutfit: toVar / parse round-trip" [layoutfit]. LF-2 "layoutfit: clampToDrag gives what a drag can give". LF-3
  "layoutfit: zero, unordered, non-finite, two or four fractions, a string -> no layout" -- MU-OS-11 (validation removed).
  LF-4 "layoutfit: deckDividerY below -1 -> no layout". LF-5 "layoutfit: a valid block with extra keys parses".
- SS-1 "showsummary: deck names, count, canvas and key count from a version-2 show" [showsummary] -- MU-OS-12 (names read from
  the top-level "layers"). SS-2 "showsummary: the same from an old-shape show". SS-3 "showsummary: an empty deck name reads
  Deck <n>". SS-4 "showsummary: a deck file, a preset, an array, garbage -> not a show". SS-5 "showsummary: a missing canvas
  reads 1920 x 1080". SS-6 "showsummary: keyCount 0 for an absent, empty or broken keys block".
- SL-1 "showlist: rows = one header per show, sorted by name; deck rows only under an open header" [showlist]. SL-2 "showlist:
  a file whose summary is pending or not a show gives no row". SL-3 "showlist: hit test -- header, deck row, nothing". SL-4
  "showlist: header menu items; Import Key and MIDI Settings enabled only with keyCount > 0". SL-5 "showlist: open / closed is
  kept per path across a refresh; a vanished file drops its entry". SL-6 "showlist: 1 Deck / N Decks".
- TD-1 "takedeck: the deck named by (index, name) is taken; a moved deck is found by name; a missing one gives nothing"
  [takedeck]. TD-2 "takedeck: trailing empty rows are cut -- Narrow from a 6-layer donor has 2 rows" -- MU-OS-13 (no cut).
  TD-3 "takedeck: Wide into a 2-layer show adds 2 layers that carry the donor's layer 2 and 3 settings under fresh ids; the
  two existing layers keep their own" -- MU-OS-14 (rowSettings left empty). TD-4 "takedeck: a deck from an old-shape donor --
  the converted donor's layers give the settings". TD-5 "takedeck: fresh deck id, re-minted clip ids, the show's routines and
  bank unchanged, the donor var unchanged". TD-6 "takedeck: Undo removes the deck and the layers it added; Redo restores the
  same deck". TD-7 "takedeck: the deck keeps its own name, not the file's".
- QF-1 "quitflow: Request -> Asking + ShowWindow; a second Request does nothing" [quitflow]. QF-2 "quitflow: Cancel -> Idle,
  no quit". QF-3 "quitflow: Quit -> DoQuit". QF-4 "quitflow: Save & Quit then SaveDone(Cancelled) -> Idle, no quit" -- MU-OS-16.
  QF-5 "quitflow: Save & Quit then SaveDone(Failed) -> Idle, no quit" -- MU-OS-15. QF-6 "quitflow: Save & Quit then
  SaveDone(Saved) -> DoQuit". QF-7 "quitflow: test mode -> DoQuit at once, never ShowWindow". QF-8 "quitflow: the window's
  texts are the four strings" (Quit! / Do you really want to quit?\nAll unsaved progress will be lost. / Quit / Cancel / Save &
  Quit -- compared with constants in QuitFlow.h).
- LINT (tests/test_one_save_lint.cpp, source greps over src/, the pattern of tests/test_output_law.cpp) [onesavelint]:
  LINT-1 "`saveToFile(` on a Composition is called only inside MainComponent::writeShow". LINT-2 "src names no `Export
  Bindings`, `Import Bindings`, `kShortcutsExportBindings`, `kShortcutsImportBindings`". LINT-3 "src names no `Save Layout`,
  `Load Layout`, `kViewSaveLayout`, `kViewLoadLayout`". LINT-4 "src names no `Save Deck`, `Load Deck...`, `isV2DeckFile`,
  `getDecksDir`, `writeDeckFile`, `legacyRowSettings`, `sourceFile` in model/Deck.h". LINT-5 "`quit()` is called from
  Main.cpp / MainComponent only through the quit flow". LINT-6 (S7 only) "src names no `fastSave`, `presetSlots_`, `FX Save`".
  RED arm of each: the tree before its stage.

Live rows (probe .harmony/probe-one-save.sh; test-mode instance by `open -g`, own-pid quit by probe-quit-ours.sh, the live lock;
scratch `AUDIODNA_SETTINGS_FILE` and `AUDIODNA_LIBRARY_DIR`; never an Output window, no full-screen capture, no synthetic
input). A row prints exactly `PASS  OS-L<n>: <facts>` or `FAIL  OS-L<n>: <facts>`; a stage's gate is its rows all PASS on the
stage head AND FAIL on the RED arm named.
Routes used (all TEST-ONLY, message-thread answers): `GET /api/debug/keys` -> {count, bindings[], bindingMode, learnMode, held,
playing[{layer, deck, column}], computerCount, lastSave}; `POST /api/debug/set_keys {keys}`; `POST /api/debug/import_keys {path}`;
`POST /api/debug/binding_mode {mode: "keyboard"|"midi"|"off"}`; `POST /api/debug/midi {note, channel, on}`;
`POST /api/debug/new_composition`; `GET /api/debug/layout` -> {deckDividerY, vDividerFrac[3], previewW}; `POST
/api/debug/load_deck {path, deck, name}`; `POST /api/debug/save_composition {path}` or `{}`; `GET /api/debug/show_list` ->
{shows[{name, canvas, decks[], open, keyCount}], messageThreadParses}; `POST /api/debug/show_list {op: "refresh"|"toggle"|
"take"|"import", show, deck}`; `POST /api/debug/quit_flow {choice: "quit"|"cancel"|"save_quit", path?}`; `GET
/api/debug/quit_flow` -> {state, last}; `POST /api/debug/quit_window {show}`. Existing: `/api/load_composition`,
`/api/debug/ui_text`, `/api/debug/undo`, `/api/debug/deck_tabs`.
- OS-L1 load F-A -> keys.count == 3 and the three (inputType, key / note / cc, action) tuples equal F-A's. RED: MU-OS-7 (the
  apply call removed at the cut): count 0.
- OS-L2 then load F-NOKEYS, then F-EMPTY -> count stays 3, tuples unchanged, both shows loaded (deck_tabs names). RED: MU-OS-6.
- OS-L3 then load F-BROKEN -> the show is loaded (its deck name in deck_tabs), count 3, layout fractions unchanged from before
  the load. RED: MU-OS-6 / MU-OS-11.
- OS-L4 `new_composition` -> count 3, tuples unchanged; deck_tabs shows one deck.
- OS-L5 launch with a scratch settings.json holding "keys" = F-B's set -> the FIRST read gives count 1, note 41; launch with a
  settings.json that has only "outputs" -> count 0. RED: MU-OS-8 (the launch read removed).
- OS-L6 `set_keys` (4 bindings), `save_composition {path}` -> the file has "version" 2, keys.bindings length 4, a "layout"
  object; the scratch settings.json has "keys".bindings length 4 and its "outputs" key (seeded beforehand) is unchanged.
  RED: MU-OS-17 (`storeKeysOnComputer` not called).
- OS-L7 `set_keys` F-MOM; load a one-clip show state where L0 C0 exists; `midi {40, on}` -> playing[0].column == 0 and held ==
  1; `import_keys` F-B's file -> held == 0 and playing[0] is no longer (deck, 0); `midi {40, off}` -> nothing changes, app
  alive. RED: MU-OS-9: after the import held == 1 or playing[0].column == 0.
- OS-L8 `binding_mode keyboard`, load F-A -> bindingMode == false, learnMode == false, count 3; the same with "midi". RED:
  MU-OS-10 (the exit removed): bindingMode true.
- OS-L9 from Reset values, load F-A -> vDividerFrac == [0.30, 0.55, 0.80] each within 0.01 (or the drag clamp's value for the
  scratch window, computed by the probe from previewW's formula and printed), previewW >= 120; `/api/render_frame` decodes to
  a non-uniform picture after the load. RED: tree before S3 (fractions stay 0.22 / 0.50 / 0.75).
- OS-L10 load F-BROKEN (layout [0,0,0]) -> fractions unchanged, previewW >= 120. RED: MU-OS-11: previewW 0 or fractions 0.
- OS-L11 load F-HOST; `load_deck {F-DONOR, deck 1, "Wide"}` -> numDecks 2; the new deck's name "Wide", its id >= 100 and not
  the host deck's id; layers 4; layer 2 and 3 names / opacities are the donor's ("Donor L4", 0.25 on layer 3), layers 0 and 1
  unchanged; every clip id of the new deck >= 1000 and different from every host clip id; routines 1, bank pad 0 unchanged;
  sha256 of F-DONOR's file unchanged. Then `load_deck {F-DONOR, 0, "Narrow"}` -> numDecks 3, layers still 4. Then two `undo`
  -> numDecks 1, layers 2. RED: the tree before S4a (the route reads the show as a deck file: the new deck is named after the
  file and has no clips, E21) and MU-OS-13 (layers 6 after "Narrow").
- OS-L12 `load_deck {F-OLD, deck 1, "Deck 2"}` into F-HOST -> the deck is added with F-OLD's "Deck 2" clip count; sha256 of
  F-OLD's file unchanged; no file appears beside it. RED: the tree before S4a (refused: "not a deck file", numDecks unchanged).
- OS-L13 copy F-OLD to scratch/s/old.json, load it, `save_composition {}` -> scratch/s/backups/old.v0.json exists and its
  sha256 equals the original's; old.json now has "version" 2; `save_composition {}` again -> the backups folder still holds
  exactly one file. RED: the tree before S1 (no backups folder) and MU-OS-4.
- OS-L14 the same with a plain FILE named `backups` already in scratch/s2 -> old.json's sha256 is unchanged after
  `save_composition {}`; the save reports failed (`GET /api/debug/keys` field `lastSave` == "failed"). RED: MU-OS-5 (old.json rewritten).
- OS-L15 library dir holding F-A, F-OLD, a bare deck file, a preset-shaped JSON and garbage.json -> `show_list` (polled until
  pending == 0) names exactly two shows, sorted, F-OLD with decks ["Deck 1", "Deck 2"], F-A with ["A1"] and keyCount 3;
  `toggle` F-OLD -> open true; `take` F-OLD deck 1 -> numDecks + 1. RED: the tree before S4b (route absent) and MU-OS-12.
- OS-L16 with 40 show files of about 1 MB each in the library dir, `refresh` -> `messageThreadParses` == 0 after all
  summaries have arrived. RED: MU-OS-18 (the parse done in `refresh`): > 0.
- OS-L17 `quit_flow {cancel}` -> state "idle", pid alive after 3 s. OS-L18 `quit_flow {quit}` -> the pid exits within 10 s, no
  crash dialog (RIG-RULES count). OS-L19 show with no file, `quit_flow {save_quit, path: <unwritable>}` -> pid alive, state
  "idle", last "save_failed". RED: MU-OS-15 (the pid exits). OS-L20 `quit_flow {save_quit, path: scratch/q.json}` -> q.json
  exists with "version" 2 BEFORE the pid exits within 10 s. RED: the tree before S5 (route absent).

Gate rows (Harmony's; strings copied only from here)
- G-OS1 (after S1): ctest 100 %; `ctest -R "showfile|showbackup"` all pass; OS-L13, OS-L14 PASS; RED arms as named; probe-boxes
  K7 PASS.
- G-OS2 (after S2): ctest 100 %; OS-L1..L8 PASS; RED arms MU-OS-7, 8, 9, 10, 17 each FAIL its row; `strings` of the app holds no
  "Export Bindings" / "Import Bindings".
- G-OS3 (after S3): ctest 100 %; OS-L9, OS-L10 PASS with their RED arms; `strings` holds no "Save Layout" / "Load Layout".
- G-OS4-0 (before the merge): `ls` of his decks folder shows no file with a top-level "layers" key (else the hand-wrap of OS4).
- G-OS4-1 (after S4a): ctest 100 %; OS-L11, OS-L12 PASS with their RED arms; probe-asan-live L1, probe-async-load a6,
  probe-boxes K1a PASS with show fixtures.
- G-OS4-2 (after S4b): ctest 100 %; OS-L15, OS-L16 PASS with their RED arms; `strings` holds no "Save Deck" / "Load Deck...".
- G-OS5 (after S5): ctest 100 %; OS-L17..L20 PASS with their RED arms; a test-mode quit by pid shows no window (the probe's own
  quit, every run).
- G-OS-HIS (before the merge, read-only, on a COPY of his show made by Harmony in scratch): load the copy, read deck names
  "Deck 1" / "Deck 2", plain Save -> backups/test with harry.v0.json has the checksum of
  .harmony/.reports/s-rta-1004/boris-show-backup/test with harry.json; reload the saved copy -> same deck names and clip counts.
- G-OS-VG: the visual gate, states V1-V15, Q1, Q2 (V16 with S7).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

- B-1 His own show. Open "test with harry" from the list, press Cmd+S. Expect: it plays as before; in Finder, next to it, a
  folder "backups" holding "test with harry.v0.json" (the file as it was). Wrong: no backups folder; or the show looks different
  after re-opening.
- B-2 The list. Open the Compositions tab. Expect: his show's name with its canvas size on the right, "2 Decks" and its date
  under it; a click opens it to "Deck 1" and "Deck 2"; a click on "Deck 2" adds that deck as a new tab, and Cmd+Z takes it away
  again; a double-click on the show's name asks before opening it. Wrong: a click opens the show; a deck arrives empty; extra
  empty layers appear; what was playing stops when a deck is added.
- B-3 Keys travel with the show. With his controller: learn two pads in show A, save A. Start a new show: the pads still work.
  Open an older show that has other keys: the pads now do what THAT show says. Open "test with harry" (no keys in it): the pads
  keep doing what they did. Quit, start the app: the pads work before any show is opened. Wrong: pads dead after launch; pads
  wiped by opening a show that has none; a clip stuck on after opening a show while a pad was held.
- B-4 Import. In a new show: Shortcuts > Import Settings from Show..., pick show A. Expect: A's keys and MIDI are live. Wrong:
  nothing changes for a show that has keys; the learn screen stays half open.
- B-5 The window. Drag the dividers, save, open another show saved with other dividers, open the first again. Expect: the
  panels jump to each show's own arrangement and the preview keeps running. Wrong: a panel of zero width; the preview black
  after the jump; an output screen flickers (it must not change at all).
- B-6 Quit. Cmd+Q, the red close button, and the menu's Quit, each: the window of his screenshot. Cancel: nothing changes, the
  show keeps playing. Quit: the app closes. Save & Quit on a new show: it asks where to save, then closes; cancelling that
  chooser leaves the app open. Wrong: the app closes without asking; Save & Quit closes without having saved.
- B-7 What is gone. The Deck menu has no Load Deck / Save Deck / Save Deck As; "+" adds a deck at once; Shortcuts has no Export /
  Import Bindings; View has only Reset Layout. Wrong: any of them still there, or a menu item that does nothing.
- B-8 By ear and eye on his real screens: opening a show from the list while music plays -- the cut is as clean as before this
  lane (nothing this lane adds runs on the audio, analysis or render threads).

## 7 QUESTIONS FOR BORIS (new numbers; each has a default A; nothing waits)

91. In the list of shows, what does a click do?
    A (default) A click on a show opens or closes its list of decks. A double-click on a show opens the show (it asks first, as
      today). A click on a deck adds that deck to the show you are in.
    B A click on a show opens the show, as today; only the small triangle opens its list of decks.
92. The order of the shows in the list:
    A (default) By name, as today: a show stays in its place.
    B Newest first: the show you saved last is at the top.
93. In the quit window, the Return key:
    A (default) Return = "Save & Quit" (the highlighted button, as in Resolume's window). Esc = Cancel.
    B Return = Cancel. Saving and quitting always needs a click.

Findings told to him, not questions: SF-1 and SF-2 (section 9).

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)

- R1 THE STRONGEST: "opening a show switches the keys" can take his controller away in the middle of a night -- he opens an
  older show (or a REST client loads one) and the pads now do something else, silently. Why the choice stands: it is his answer
  to question 85 ("yes but you import the setup from the most recent show"), the absent / empty / broken rules keep his keys for every show that never had any (all shows that
  exist today), and Import brings a set back in one command. Cheapest refuting test: B-3 on his own controller; if it bites,
  the one apply call in `finishStagedLoad` is the line that changes.
- R2 "An empty set is no set" means a show can never mean "no keys at all". Stands because no screen can clear bindings today
  (E9). Refuting test: KS-4 documents it; if a "clear all keys" command is ever built, this rule is revisited there.
- R3 The computer's copy is written at Open: opening a friend's show replaces the remembered set; a set that was only ever in
  the computer's copy (learned, never saved in a show) is then gone. This is his rule ("if the user is changing things around settings, etc., and they do not save the
  composition, nothing is saved."), but it is the one place this lane can lose something without a file on disk. Cheapest test: OS-L1 + L5.
  If he objects: write the computer's copy only at Save (one line out of `finishStagedLoad`).
- R4 A later-version show opened best-effort may be misread (OS1-e). No such file can exist before a later lane; the backup
  rule keeps the bytes. Test: SF-7, SB-2.
- R5 The backup reads and parses the target at every Save (message thread). INFERRED small for his 49 KB show; a show of many
  megabytes doubles the Save's stall. Cheapest test: OS-L13 prints the Save's duration; if it exceeds one frame for a real show,
  remember the path this build last wrote and skip the read for it (a three-line change in `writeShow`).
- R6 The list hides files that are not shows (today they are listed). A file he expects could "vanish". On his disk the folder
  holds one show (E7). Test: OS-L15.
- R7 Removing the deck-file reader strands any v2 deck file made between now and the merge. Guard: G-OS4-0. Stays a risk for a
  file made on another machine; the hand-wrap (a one-deck show) is the way back.
- R8 Click vs double-click on a header: the first click of a double-click also toggles the header. The builder acts on
  mouse-up with the click count, and SL-3 pins the model; the visual gate cannot see it -- B-2 does.
- R9 `juce::AlertWindow` lays its three buttons in one centred row; his screenshot has "Quit" apart on the left. The critics may
  fail Q1; the fallback component is named in OS5.
- R10 The modal quit window holds the keyboard: bound keys are dead while it is open (MIDI is not). Resolume's window does the
  same (ASSUMED, not checked). Test: B-6 while a show plays.
- R11 A logout or shutdown is cancelled by the window (E24) until he answers. Accepted: it is what "ask first" means.
- R12 Layout numbers are absolute pixels for the deck divider: a show saved on a larger window is clamped on a smaller one, so
  the arrangement is "as close as fits", not identical. Test: OS-L9 prints both.
- R13 Not read by me, so INFERRED where used: `refreshUiAfterModelSwap`; tests/ShowFixture.h's helpers; the three
  probes' fixture writers; whether `ApiServer::handleLoadComposition`'s throwaway `Composition` (ApiServer.cpp:1135) needs any
  change for `loadedExtras` (it should not: it discards the object).
- R14 The dropped seven keys: E5 rests on greps by the sheet and its re-read, not on a build. The S1 builder greps again and the
  compiler is the proof; any reader found stops the drop, not the stage.

## 9 WHAT IS NOT IN THIS LANE

- SF-1 (finding for Boris, own lane): the eight Link macros (names, values, links) and user signals are not saved by anything,
  although saved connections and the AdjustMacro key action name them (E14). "One Save" is not whole until they are; needs its
  own fact sheet first.
- SF-2 (finding): the Composition inspector's five autopilot controls set values nothing reads (E5); the genre keys are inert.
  Their removal or wiring is a separate item. `bpmMultiplier` and the /4 /2 x1 x2 x4 buttons: the beat-nudge lane.
- SF-3 (finding, existing ledger debt): piano / momentary, relative CC, Selected / ThisItem and velocity-to-opacity cannot be
  set from any screen (FO-V14); after this lane they can exist only in a show file's "keys" block edited by hand.
- A "changed since the last save" flag, auto-save, a rolling backup at every Save, a reminder on New / Open.
- Per-effect looks (reading R42), and the old look buttons beyond question 86 (OS6.4).
- Collect Media's behaviour (it only writes its show copy through `writeShow`), Relocate, Snapshot, takes, video recording.
- Output screens and their settings (per machine: the outputs lane); the audio source choice, tempo state, the MIDI output
  device (FO-M6: not in the show; not asked).
- The event labels ("Saved: ...", "Loaded: ...") and every failure box other than the failed Save: the messages lane.
- Window size / position, the browser tab, the inspector state in the layout.
- Dragging a deck from the list; deleting a single deck inside another show's file; thumbnails in the list.
- A converter for v1 deck files (unreadable before this lane, E8) and any deletion of files on his disk.
- From the STOPPED sync-dial branches (H-17: kept as parts only; commit ids from plan-nudge.md:742-752 and facts-syncdial-parts.md; e15d1d0 and
  e0010e1 checked with `git log -1` in their worktrees, their diffs not re-read):
  - CARRIED by this lane: nothing. No code of lane/bf2 (740b6d6) or lane/bf2-keys (9eab9bd) is needed for the show file, the
    list or the quit window.
  - LEFT TO THE NUDGE LANE: lane/bf2-keys e15d1d0 (the nudge key action, its step field and their `toVar` / `fromVar` lines) --
    it lands in the same `BindingManager` serializer this lane reuses whole, so it is saved in the show's "keys" block with no
    change here.
  - DROPPED WITH THE DIAL, and relevant here only as things this lane must NOT re-create: lane/bf2 e0010e1 (`SyncVenues`,
    `SyncOffsetController`, `AppSettings::kSync`, `/api/sync*`); the never-built S5a "sync" key in the composition ("every show
    remembers it's sync" as far as it named the dial -- superseded 2026-10-04 12:08:21). The show file gets no sync field.

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 14:19:12, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-one-save.md (status DONE; 31 attacks ruled: 23 ACCEPT, 7 PARTIAL, 1 REJECT;
all 8 MUSTs upheld; 17 amendments, each OVERRIDES this plan's body). Precedence for every stage, review and gate of lane
"one-save": Boris's verbatim words (binding-decisions.md, the 2026-10-04 sections) > this adoption > ruling-one-save.md > this
plan's body. Workflow run wf_c82b83d6-51f (plan: architect opus high; seats data-safety 7 attacks / 1 MUST, gates 8 / 3,
stage-hands 8 / 2, scope 8 / 2 -- papers whole (40,377 characters) in attack-one-save-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list, decisions, measurements, strongest
counter-argument, and its section 7 (questions and readings) in full. NOT read by me: sections 1-6 and 8-10 in the file -- the
builders' and reviewers' spec; a gate string is copied only from section 5.
WHAT THE RULING ADDS TO THE PLAN: every show write is verified by READ-BACK before the swap (JUCE's replaceWithText can swap in
a cut-off file and report success); any existing file that is not a version-2 show is copied to backups/ first, fail-closed;
a version counts only as an integer >= 2; the quit window is the app's own panel (a stock AlertWindow would have bound Return);
the settings.json mend, orphaned between two lanes, lands here.
BORIS'S ANSWER 86 = A ("86 default yes", 13:22:56) is already in the ruling: stage S7 removes the old look buttons.
HARMONY'S DECISIONS (the ruling's section 8):
H-1  default: two merges -- M1 = S1 alone once G-OS1, G-OS-HIS and G-OS4-0 pass (he is told about the "backups" folder then);
     M2 after the visual gate.
H-2  default: THIS lane's S7 removes the old look buttons; the effect-looks lane plans no removal and cites it. Reconciled
     with the effect-looks ruling when it lands (exactly one owner).
H-3  default: the list keeps today's order; not asked.
H-4  default: no previous copy at every Save of a version-2 show in this lane.
H-5  default: not built; M-2 decides.
H-6  CHANGED to my rule: lanes write "Pitfall NN"; I assign the number at each merge.
H-7  default: the settings.json mend lands in this lane's S1; the outputs lane builds none (its adoption's HD-5 agrees).
H-8  default: AppSettings.h -- the second lane to merge re-bases.
H-9  default: BindingManager's serializer -- the second of this lane and the nudge lane to merge re-bases parseSet as step 0.
H-10 default: a take that cannot be finalised during a quit shows nothing here; the messages lane rules it.
H-11 default: the RIG-RULES line (no gate shows the quit window; its picture is the headless tool's) is added at M2.
H-12 default: build while 91, 93, 94 are open, as A; each answer is the one-line change section 7 names.
H-13 default: Import from a show with no usable set changes nothing and shows nothing.
H-14 default: the conversion note on his show stays in the log; he is told once (reading R64).
FACTS I MEASURE: M-1 (a full disk on a 2 MB image), M-2 (the cost of a Save), M-3, M-4, as the ruling's section 8.
ORDER: S1 -> my rows -> MERGE 1 -> S7 -> S2 -> S3 -> S4a -> S4b -> S5 -> visual gate -> MERGE 2. NOT STARTED in this session:
nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWERS (2026-10-04 15:30:15)
Boris, verbatim (binding-decisions.md, "Boris's answers to questions 91, 93, 94"): "91 default"; "93 b"; "94 default".
91 A and 94 A are the ruling's defaults: nothing changes. 93 B: Return presses "Save & Quit" -- the ruling's section 7 names
the change ("one line in QuitPanel::keyPressed; QP-3, OS-L18's last step and MU-OS-36 flip with it"). No architect delta is
needed: the S5 packet carries this block and those three names. H-12 is closed (no question of this lane is open).
