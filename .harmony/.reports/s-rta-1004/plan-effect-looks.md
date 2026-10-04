# PLAN -- lane "effect-looks": looks per effect, kept by the app for every show; a small menu on each effect

Author: architect (plan authoring, s-rta-1004). Position paper for the blind council; a ruling follows; Harmony decides.
Pin: `git -C /Users/boriskarpman/projects/RealTimeAudio rev-parse --short HEAD` printed 185147b and
`git status --short -- src tests docs CMakeLists.txt` printed nothing, so every code read is a plain file at 185147b.
Worktrees checked clean: lane/bf2 at 740b6d6, lane/bf2-keys at 9eab9bd. Nothing was built, run or launched.
Labels: VERIFIED = I read the line at 185147b. SHEET = a row of a fact sheet whose VERIFICATION section did not overturn it
(FEL = facts-effect-looks.md, FOS = facts-one-save.md, FS = facts-saves.md, all in .harmony/.reports/s-rta-1004/).
INFERRED / ASSUMED are written where they are used. All paths are under /Users/boriskarpman/projects/RealTimeAudio.

VERDICT IN ONE PARAGRAPH. A look is a small named file that holds ONE effect's slider values and its Dry / Wet, each value
filed under the parameter's name. Looks live one file per look, in one folder per effect, under ~/Library/Audio-DNA/Looks/,
written the moment the look is made; nothing is read at launch; a bad file hides only itself. Every effect header gains one
small button that shows which look the effect's settings ARE (derived from the values, never remembered) and opens the menu:
Default, his looks, New Look, Rename, Delete. Loading is one fenced write of one effect's values, one undo step of its own
(a new command that touches one slot, not the whole chain), never the layer strip, never a connection. The show file does not
change. The old Save, Load, FX Save and ten slots are removed by the one-save lane's stage S7, not here.

---------------------------------------------------------------------------------------------------------
## 1 GOAL
---------------------------------------------------------------------------------------------------------
Boris (binding-decisions.md, 2026-10-04, recorded 12:59:09): "every effect has many looks with specific parameter setups.
these are saved with the app. always." Boris (recorded 13:22:56): "86 default yes".
Harmony's reading R42, told to him and not corrected (boris-clarify-86.md): a look belongs to ONE effect; a small menu on each
effect picks a look or keeps the current settings as a new look; kept by the app for every show, the moment it is made.
Harmony constraint: a look names its effect and its parameters BY NAME; the store is never one read-modify-write file; a look
is on disk the moment it is made; loading is ONE step, whole or not at all, one undo step; it never changes the layer strip;
it never writes a row the engine owns except through the engine's own path; what a take sees is MEASURED; the old buttons are
removed exactly once.
Done means: section 5's unit rows green with their RED arms shown, Harmony's live rows G-EL-1..G-EL-11 run by her, the
visual gate passed on states V-EL-1..V-EL-17, and section 6 handed to Boris.

---------------------------------------------------------------------------------------------------------
## 2 ESTABLISHED FACTS (VERIFIED or SHEET only)
---------------------------------------------------------------------------------------------------------
The effect instance
- F1 One struct, `Clip::EffectSlot`: effectName, paramValues, dryWet, enabled, bypassed, paramConns, paramLive, dryWetConn,
  dryWetLive; helpers addParam, resizeParams, effParam, effDryWet. src/model/Clip.h:60-114 (VERIFIED). No id, no label, no
  look name (SHEET FEL 1b, re-read #1).
- F2 Exactly three owners: `Clip::effects` (Clip.h:115), `Layer::layerEffects` (Layer.h:310), `Composition::globalEffects`
  (Composition.h:69) (SHEET FEL 1a, re-read #2). Addressed by `EffectScope` and `resolveEffectVector`
  (core/EffectScope.h:18-35, core/EffectCommands.h:19-50) (SHEET FEL 1a).
- F3 `effectName` is the DISPLAY name; `getEffectDef` is an exact match on it (SHEET FEL 1b, re-read MISSING 5). The show
  stores params as a bare positional array, connections sparse by index (SHEET FEL 1b, re-read #3, #4).
- F4 The library gives `ParamDef { name, uniformName, defaultValue }` and `EffectDef { name, category, shaderName, params,
  temporal }`; no range, no look list. src/effects/EffectLibrary.h:17-34 (VERIFIED). 135 registerEffect calls (SHEET FEL
  re-read, re-counted). Built-in names / shader names / param + uniform names are unique: tests/test_preset_manager.cpp:292
  "T6" (VERIFIED the test exists by name; its body not re-read).
- F5 Three EffectLibrary instances; the stacks use MainComponent's `effectLibrary_` (MainComponent.cpp:1411 VERIFIED;
  SHEET FEL re-read #23). An ISF import registers into the renderer's library only (SHEET FEL 1c).
- F6 A slot whose effect has no def shows "Param n" rows and is skipped by the renderer (EffectStackView.cpp:392-398
  VERIFIED; render skip SHEET FEL re-read MISSING 5).
The row on screen
- F7 All three inspectors embed the same `EffectStackView`; they forward `setEffectPerformEdit` / `setEffectFenceHook`
  through InspectorPanel.cpp:145-156 (VERIFIED); MainComponent wires both at MainComponent.cpp:1421-1438 (VERIFIED).
- F8 Header: 26 px (`kHeaderHeight`, EffectStackView.h:151); "B" at x=2 w=24, "X" at width-26 w=24
  (EffectStackView.cpp:97-98); the name is drawn from x=30 to the right edge (:43-48); the collapsed first value and the
  fold arrow are painted under the X button's rectangle (:51-81) (VERIFIED).
- F9 `mouseDown` folds on ANY button when x > 28 inside a header; there is no button test (EffectStackView.cpp:449-464)
  (VERIFIED). No PopupMenu exists in the stack or the three inspectors (SHEET FEL re-read #7).
- F10 `rebuildRows` forgets every bound connection first and rebuilds every row folded (EffectStackView.cpp:256-289)
  (VERIFIED). `refresh()` pushes `effParam` / `effDryWet` to the controls and repaints (:188-219); it is called by the three
  inspectors' own refresh (LayerInspector.cpp:809, CompositionInspector.cpp:461, ClipInspector.cpp:1005) from
  `inspectorPanel_->refresh()` at about 10 Hz (MainComponent.cpp:3056 comment) (VERIFIED the call sites; the rate INFERRED
  from the comment).
- F11 Drops: interested iff the description starts with "fx:"; a comma list appends each effect under ONE fence and one
  `onPerformEdit` (EffectStackView.cpp:468-549) (VERIFIED). The stack is not a drag source (SHEET FEL Q3). Each inspector
  paints the cyan panel highlight; the inner stack highlight is purple (SHEET FEL re-read #20).
- F12 A slider writes `slot.paramValues[p]` / `slot.dryWet` directly and unfenced (EffectStackView.cpp:356-360, 405-411);
  `onParamChanged` and its siblings are never assigned (SHEET FEL re-read #15). A slider drag sets a Held grip on the bound
  connection; +/- and right-click reset touch it (UniversalParamControl.cpp:27-60 VERIFIED).
Engine, undo, takes
- F13 The renderer reads `effParam(i)` / `effDryWet()`; the engine writes only the live twins (Pitfall 33,
  docs/claude/pitfalls.md entry 33 VERIFIED). Effect params and dryWet are plain floats, not atomic (ManualWrite.h:26-29
  VERIFIED).
- F14 `EffectStackCmd` stores whole-vector before / after; execute and undo assign the whole vector under the fence
  (core/EffectCommands.h:83-119 VERIFIED). `UndoManager::perform` calls `execute()` (core/UndoManager.cpp:12-15 VERIFIED),
  so the bypass button's edit is applied twice (once by the view, once by the command).
- F15 Copy-ASSIGNING a `ParamConnection` clears its grip and its engine state (connect/ParamConnection.h:147-160 VERIFIED):
  a whole-vector assignment resets every connection in the chain, not only the edited slot's.
- F16 A parameter, Dry / Wet or connection change is not undoable; add / remove / the on-screen bypass are (SHEET FEL re-read
  #15, #16, C3). A structural undo restores whole slots and so overwrites later slider edits (SHEET FEL re-read C3 b).
- F17 Every undo / redo call site runs `refreshAfterUndoRedo`, which re-points all three stacks (rows fold)
  (MainComponent.cpp:5350-5402 VERIFIED).
- F18 The fence: `UndoService::withDeckDetached` detaches the deck, drains one GL frame, runs the mutation while the GL
  thread keeps rendering; a fenced frame holds the canvas (Pitfall 55, pitfalls.md entry 55 VERIFIED first 900 characters).
- F19 A take records a continuous move only through `MainComponent::manualWrite` with `Origin::Human`
  (MainComponent.cpp:2090-2100, 4195-4202 VERIFIED). The callers are REST / OSC / MIDI / the layer strip
  (MainComponent.cpp:1907-1926, 2331-2365, 7713-7928 VERIFIED); the effect stack's sliders are not among them (F12).
  A take also stores a state at Record and at Stop: `checkpoint0` and `checkpointEnd` = `capturePerfState()`
  (recording/RecorderHost.cpp:269-270, 381 VERIFIED), which holds non-default effect values by position
  (recording/PerfStateCapture.cpp:78, 114 VERIFIED).
- F20 `resolveControl` resolves an effect param / dryWet for scopes Clip, Layer and Comp (connect/ManualWrite.h:57-66
  VERIFIED, the comment block).
Stores and folders
- F21 Two spellings under ~/Library. Hyphen `Audio-DNA`: AppSettings (model/AppSettings.cpp:5-8 VERIFIED), the MilkDrop
  favourites file (MainComponent.cpp:72-76 VERIFIED), bindings export, layouts, and Documents/Audio-DNA. No hyphen
  `AudioDNA`: compositions, decks, Presets, FX Saves, browser_favorites.txt (SHEET FEL re-read #12).
- F22 `AppSettings::update` is read-modify-write; an unreadable file reads as empty and the next update drops the other
  keys (model/AppSettings.cpp:19-40 VERIFIED).
- F23 The test-mode seam: `appSettingsFile(testMode)` uses AUDIODNA_SETTINGS_FILE or a scratch file, never the real one
  (MainComponent.cpp:83-101 VERIFIED).
- F24 `juce::File::replaceWithText` swaps a temp file in but ignores a failed write of the temp file (SHEET FS "Where the
  sweeps disagreed" 4: INFERRED there from reading JUCE, UNKNOWN-NEEDS-A-RUN). This plan treats it as untrusted.
- F25 No store holds a per-effect look today (SHEET FEL Q5 + re-read C2).
The old buttons
- F26 The small Save / Load, FX Save and the ten slots snapshot the hidden legacy chain; their code, tests and the files a
  removal touches are listed in SHEET FOS Q6 (re-read: CONFIRMED). Row 1 lays them out at MainComponent.cpp:2756-2761 and the
  slot bar at :2776-2791 (VERIFIED). The legacy chain still renders after the deck compositor (SHEET FOS re-read).
- F27 plan-one-save.md already holds the removal as its stage S7 "looks off (only if 86 = A)" with LINT-6 and visual state
  V16 (plan-one-save.md:522-528, :568, :661 VERIFIED). Question 86 is now answered A (section 1).
Test routes
- F28 Production REST writes only a CLIP effect of the SHOWN deck; no route writes a layer or global effect (SHEET FEL
  re-read #18, MISSING 4). Test-only routes live in api/ApiServer.cpp inside `#if AUDIODNA_TEST_SERVER` (:299-344 VERIFIED):
  among them /api/debug/undo, /api/debug/inspect_clip, /api/debug/ui_test_menu, /api/debug/ui_text (the file label's text
  only, :317-319 comment).
- F29 The stopped branches do not touch EffectStackView, EffectCommands.h, src/effects, UniversalParamControl,
  InspectorPanel.cpp, PresetManager.cpp or Clip.h: `git diff --name-only 185147b...<head>` over those paths printed nothing
  for 740b6d6 and for 9eab9bd (VERIFIED).

---------------------------------------------------------------------------------------------------------
## 3 ITEMS
---------------------------------------------------------------------------------------------------------

### EL1 WHAT A LOOK HOLDS
VERIFIED: F1, F3, F4, F13, F15.
Forks: (a) values only; (b) values + Dry / Wet; (c) values + Dry / Wet + connections; (d) (c) + bypass.
CHOICE: (b). A look holds, for ONE effect: its name, the effect's display name, the effect's shader key, Dry / Wet, and one
entry per parameter {uniform name, label, value}. Nothing else.
- Not `bypassed`, not `enabled`: bypass is a performing switch with a second writer that takes record (SHEET FEL re-read
  C3 a); a look that could switch an effect off would make "load a look" remove a picture.
- Not connections. Runner-up (c) loses for three reasons that are facts, not taste: a connection names a signal by name or
  a macro by index, and neither the signals nor the macro bank are saved anywhere (SHEET FOS re-read M-6), so a look kept
  "for every show" would carry wires to things another show does not have; loading would replace wiring on a playing layer;
  and assigning a connection clears its grip and engine state (F15), a visible hiccup. (a) loses because Dry / Wet is the
  first row of every unfolded effect (EffectStackView.cpp:347-373) and is part of what he sets up.
What Boris gets when he loads a look onto an effect that has a signal wired to a parameter: the wired parameter keeps
following its signal; the look's value is stored underneath and is what he sees the moment he unplugs the signal; every
other slider and Dry / Wet take the look at once. No grip is opened (question 102 offers the other behaviour; the one place
that changes is `EffectLookCmd::apply`, one loop that would touch each connected parameter's connection).
The "Default" look: yes. It is the library's defaults plus Dry / Wet 1.0, built from the def at menu time, never a file,
first in every menu, cannot be renamed or deleted. It is also the "reset the whole effect" that does not exist today
(SHEET FEL Q6).
Does the app SHIP looks: default NO, only "Default" and the ones he makes (question 101). If he says yes, the one place
that changes is `EffectLookStore::looksFor`, which would also list a read-only folder inside the app bundle; the file
format is the same.
Change by file: NEW src/effects/EffectLook.h (header-only, pure juce_core + the model): `struct EffectLook`,
`looks::Values { std::vector<float> params; float dryWet; }`, `looks::capture(def, slot, name)`,
`looks::resolve(def, slot, look) -> Values` (what the slot would hold after a load), `looks::matches(def, slot, look)`,
`looks::defaultLook(def)`, `looks::toVar`, `looks::fromVar -> std::optional<EffectLook>`, `looks::folderNameFor(effect)`,
`looks::fileNameFor(lookName)`.
Rules of `resolve` (the Harmony constraint, as code rules): for each def parameter i below `slot.paramValues.size()`: the
look's entry with the same uniform name gives the value; else the entry with the same label; else the slot's CURRENT value
is kept. An entry that matches no def parameter is ignored. Values are clamped to [0, 1]. Dry / Wet likewise. None of this
is an error and none shows a text.
Tests: LK-1..LK-12 (section 5). Mutants MU-EL-1..7.

### EL2 THE STORE
VERIFIED: F21-F25, F3, F4.
WHERE: `~/Library/Audio-DNA/Looks/` (the hyphen spelling). Why this one: it is the folder of the things the app keeps by
itself (settings.json, the MilkDrop favourites: F21), it already has a test-mode seam to copy (F23), and the no-hyphen
folder holds what he saves by hand plus the old `Presets` and `FX Saves` that are leaving. Runner-up `~/Library/AudioDNA/`
loses on exactly that: a new "Looks" beside the dead "Presets".
SHAPE: one FILE PER LOOK, one FOLDER PER EFFECT: `Looks/<effect folder>/<look file>.look.json`.
- Forks: one store file; one file per effect; one file per look. One store file is forbidden by the Harmony constraint (F22
  is the trap). One file per effect is the runner-up: it is still read-modify-write at the size of an effect, so one bad
  read of that file followed by "New Look" would drop that effect's other looks unless a move-aside rule is added. One file
  per look has NO read-modify-write at all: making a look creates one new file, deleting removes one, a bad file can only
  hide itself.
- Keyed by the effect's DISPLAY name (the name the slot and the show carry, F3), made into a legal folder name by
  `looks::folderNameFor` (`juce::File::createLegalFileName`). The file also records the shader key, so a later version that
  renames an effect can map old folders with a one-line alias table; this lane writes it and does not read it for matching.
  A file whose "effect" is not exactly the effect asked for is not listed (guards two effects whose legal folder names
  collide; LK-11 proves no two of the 135 do).
- The format (JSON object, names not positions): "format": "audio-dna-look"; "version": 1; "effect": display name;
  "shader": shader key; "name": the look's name; "dryWet": number; "params": array of {"uniform", "label", "value"}.
  Unknown keys are ignored on read and KEPT on a rename (rename edits the parsed object and writes it back).
  A file is refused WHOLE (not listed, not touched) when: it does not parse, the root is not an object, "format" differs,
  "effect" differs, "params" is not an array, or any "value" / "dryWet" is not a finite number. A "version" above 1 is read
  by the same rules.
- The look's name: unique per effect, compared case-insensitively on the legal file name (APFS default is
  case-insensitive, SHEET FEL section 3 item 16); "Default" and "Looks" are reserved; empty or longer than 40 characters is
  refused. The file's existence is the collision test.
THE WRITE: `EffectLookStore::writeAtomic(file, text)`: create the folder; write a sibling temp file through a
`juce::FileOutputStream` opened on a path that was deleted first (Pitfall 46); check the stream status after write and
flush; READ THE TEMP FILE BACK and compare it byte for byte with the text; only then move it over the target. Any failed
step deletes the temp file and returns false. `replaceWithText` is not used (F24). A full disk therefore gives false, never
a truncated look.
A LOOK THAT COULD NOT BE WRITTEN, as a state (no text, no box): the look still exists for this run -- it is in the menu and
can be loaded -- and it carries a hollow ring before its name in the menu and on the effect's button while it is the
matched look. The store retries every not-on-disk look at each later store operation and once when the store is destroyed
at quit. The ring goes when the file is on disk. Nothing else is shown. (Whether the quit window names it is Harmony's
decision HD-2, default no: it is not one of the five things his rule allows.)
READ: nothing at launch. `looksFor(effect)` reads that ONE effect's folder the first time an inspector shows that effect or
its looks are asked for, on the message thread, and caches the parsed looks for the run; the app is the only writer, so the
cache is refreshed only by its own operations. 135 effects times many looks costs nothing until an effect is shown.
INFERRED: a folder of some dozens of files of a few hundred bytes reads in a few milliseconds; the render thread and the
audio callback never wait on the message thread (CLAUDE.md, the 4-thread model), so the worst case is a short UI pause the
first time an effect with very many looks is shown. Not gated by a bar; ST-9 / G-EL-9 gate "zero reads at launch".
THREADS: message thread only, like AppSettings (F22's header says the same). No lock, no new mutex, nothing on the audio
callback, the analysis thread or the render thread.
An effect renamed or removed in a later version: its folder stays on disk, untouched and unlisted; nothing deletes a look
file except his own Delete.
Test mode: NEW free function `effectLooksDir(bool testMode)` beside `appSettingsFile` (MainComponent.cpp:83-101): in a
test-server build running --test-mode, AUDIODNA_LOOKS_DIR when absolute, else a scratch folder in the temp directory; never
the real folder. One stderr line names the folder, as the settings seam does.
Change by file: NEW src/effects/EffectLookStore.h / .cpp: `explicit EffectLookStore(juce::File root)`,
`const std::vector<EffectLook>& looksFor(const juce::String& effect)`,
`const EffectLook* make(const EffectLibrary::EffectDef&, const Clip::EffectSlot&)` (names it "Look N", N the smallest free
number), `bool rename(effect, from, to)`, `bool remove(effect, name)`, `void retryPending()`, `int dirReads() const` (test
counter), a settable writer hook for the tests. MainComponent.h: one member; MainComponent.cpp: the seam function and the
construction. CMakeLists.txt: the new source.
Tests: ST-1..ST-10. Mutants MU-EL-8..12.

### EL3 THE MENU ON AN EFFECT
VERIFIED: F7-F11.
WHERE: one new button per effect header, `looksBtn`, immediately left of "X", in EffectStackView -- so it is in every
inspector that shows a stack (clip, layer, composition) with no per-inspector code beyond two forwarders.
- Layout (EffectStackView::resized): width `lookW = jlimit(28, 110, headerWidth - 58 - 96)`; bounds
  (headerWidth - 28 - lookW, y + 3, lookW, 20). The effect name's draw rectangle is trimmed so it ends 4 px before the
  button (the one changed line in `paint`, :46-48). Under 56 px the button shows only its triangle (and the ring).
  The collapsed first value and the fold arrow already sit under "X" (F8): NOT moved here (side finding SF-3).
- The button is a small `juce::Button` subclass (NEW src/ui/LooksButton.h) that paints: an optional hollow ring, the text,
  a down triangle drawn as a path (no unicode glyph, Pitfall 6). Its text is a STATE derived from the effect's values each
  refresh: the name of the first look that the slot `matches` (order: Default, then his looks by name), in the primary
  text colour; else the word "Looks" in the secondary colour. So "which look is loaded" = the name; "changed since" = the
  name is gone. No memory of "the last look loaded" is kept anywhere.
  Why derived and not a field: fork (i) a `lookName` on the slot, saved in the show; fork (ii) row memory; fork (iii)
  derived. (ii) dies at every rebuild (F10). (i) is the runner-up: it could show "Look 2, changed", but it changes the show
  format in three serializers that two other lanes are editing, and a rename or delete would leave stale names in every
  slot of every saved show. (iii) cannot be stale: undo, a routine, REST or a slider all change the values and the button
  follows; setting the sliders back brings the name back.
- Pitfall 57 / 59: the button's state is set from `EffectStackView::refresh()`; `LooksButton::setState` compares what it
  PAINTS (text, colour, ring, text-or-triangle mode) and repaints only on a change. No new timer.
THE MENU (built by a pure function, NEW src/ui/EffectLooksMenu.h: `looksmenu::build(def, slot, looks) -> Model`), top to
bottom: "Default"; separator; his looks in natural name order (a not-on-disk look in the secondary colour with the ring);
separator; "New Look"; "Rename '<matched>'..."; "Delete '<matched>'" (warning red through `addColouredItem`, the
"Delete routine" precedent, docs/claude/recording.md "Surfaces"). The matched look is ticked. Rename and Delete are
enabled only while one of HIS looks is the matched one (never for Default). With no looks of his the list part is empty.
Shown with `showMenuAsync` and `.withParentComponent(getTopLevelComponent())` and the button as target (CLAUDE.md PopupMenu
rule; the per-parameter picker at UniversalParamControl.cpp:387-413 is the precedent).
- A menu answer arrives later: the callback holds a SafePointer to the view, the row's effect index and the effect's name,
  and does nothing unless that index still holds that effect (LM-6).
- New Look: one click, no box. The look is named by itself, "Look 1", "Look 2", ... (the smallest free number), written at
  once, and becomes the matched look. Why no name box at creation: the app is a keyboard clip launcher; a text box takes the
  keys in the middle of a show. (Question 104 offers the box.)
- Rename: the AlertWindow idiom of `renameRoutine` (MainComponent.cpp:6336-6342), opened by the user's own menu choice; a
  refused name (empty, clash, reserved, too long) changes nothing and shows nothing. Not an in-place editor over a
  rebuilding row, so Pitfall 65 is not engaged; that is why the runner-up (a name box inside the header) loses.
- Delete: a confirm (the "Delete routine" precedent), then the file goes; the effect's sliders do not move; the button
  falls back to "Looks" (or to another look with the same values).
- Right-click on a header opens the same menu and no longer folds; a left click folds as today (F9; the deck tab rule
  "right-click = its menu" is the precedent; Harmony's decision HD-5). One test in `mouseDown`: `event.mods.isPopupMenu()`.
- An effect the library does not know (F6) and an ISF import (F5) get no button.
- Drag and drop (Pitfall 16, F11): the button is a plain child like "B" and "X"; it is not a drop target, so a drop over it
  reaches EffectStackView; the comma list and the single undo entry are untouched (LM-9).
Every state the visual gate must capture: V-EL-1..V-EL-17 (section 5).
Change by file: EffectStackView.h / .cpp (EffectRow gains `looksBtn`; `setLookStore`; `onPerformLook`; the menu open
function; the three menu actions; `resized`, the name rectangle in `paint`, `refresh`, `rebuildRows`, `mouseDown`);
InspectorPanel.h / .cpp, ClipInspector.h, LayerInspector.h, CompositionInspector.h (two forwarders each, the
`setEffectPerformEdit` pattern); MainComponent.cpp (two wiring lines beside :1421-1438).
Tests: LM-1..LM-11. Mutants MU-EL-17..20.

### EL4 LOADING A LOOK
VERIFIED: F12-F20.
WHAT IT WRITES: for ONE slot, `paramValues[i]` for each parameter `resolve` gives, and `dryWet`. Never `paramLive` /
`dryWetLive` (the engine's rows, Pitfall 33), never a connection, a grip, `bypassed`, `enabled`, the effect name, the
vector's size, or anything of the layer. These are the same two fields the on-screen sliders write (F12), so the renderer
picks them up through `effParam` exactly as after a slider move.
THROUGH WHICH PATH: one new command, NEW src/core/EffectLookCmd.h:
`EffectLookCmd(CompositionResolver, DeckFenceHook, EffectScope, int fxIndex, std::string effectName, looks::Values before,
looks::Values after, std::function<void()> refresh, std::string description)`. `execute` / `undo` resolve the chain by
coordinate (as EffectStackCmd does), check that index `fxIndex` still holds `effectName` with the same parameter count,
and then write the values inside ONE fence. A failed check writes nothing. The look's values come from the store's cache,
already validated when listed, so a load never reads the disk and cannot stop half way: WHOLE OR NOT AT ALL, also for the
render thread, which sees the old values or the new ones and never a mix (F18).
The shared funnel, NEW src/core/EffectLookOps.h: `looks::loadInto(chain, fxIndex, def, look, scope, fence, push) -> bool`
computes before / after and hands them to `push` (the host builds the command and `undoManager_.perform` runs it once);
with no host it writes directly under the fence. The menu and the test route both call it. Unlike the bypass button
(F14) the view does not also write by itself: one write per load.
- Fork: reuse `EffectStackCmd` (whole chain before / after). It is the runner-up and it loses on F15 and F16: assigning
  the whole chain clears every grip and connection state in the chain on a playing layer, undo would also take back slider
  moves made on OTHER effects of that chain after the load, and every row would fold. `EffectLookCmd` touches one slot's
  floats.
WHILE THE LAYER PLAYS: the picture takes the look on the next rendered frame; one fenced frame holds the canvas, the same
cost as one click on "B" today (F18, F14). No glide, no transition. Temporal effects keep their history (it is keyed by the
layer, Pitfall 35, and nothing structural changes). The layer strip is not touched.
CMD+Z: one step, "Undo Load Look '<look>' on '<effect>'" in the Edit menu (the existing label mechanism). Undo puts back
that one effect's values and Dry / Wet as they were just before the load; redo sets the look again. It never changes
anything in the layer strip -- Boris: "cmd z Does not change anything in the layer strip which is by default live based" --
because the command has no code path to a layer field (LC-4, LINT-EL-3). After an undo the rows fold, as after every undo
today (F17): not mended here.
The undo rule, ruled against the facts: a look load IS undoable, because add / remove / bypass are (F16) and a look can
overwrite eight sliders at once. Slider edits stay not undoable, and a STRUCTURAL undo still restores whole slots (F16):
this lane does NOT mend that and does NOT make it worse -- its own command is one slot wide. The mend (parameter edits as
commands, or structural commands that keep values) belongs to the undo-live lane; this lane leaves `EffectStackCmd`
untouched for it.
Making, renaming and deleting a look are not undo steps: they change the app's store, not the show. Delete has a confirm.
A TAKE BEING RECORDED: by the code (F19) a look load is seen by a take exactly as a mouse move of an effect slider is seen
today: NOT as a move -- no lane, so playing the take back does not replay it -- and the values appear in the state the
take stores at Stop. This is INFERRED from the wiring and is MEASURED by Harmony's row G-EL-8 with a pre-registered table;
it is not assumed. Question 105 asks Boris whether that is what he wants; if he wants it played back, the change is its
own lane (side finding SF-2), because the mouse sliders have the same gap.
A ROUTINE OR A TAKE BEING REPLAYED: the look opens no grip, so a routine's move that is in progress on one of the effect's
sliders keeps that slider (its next point overwrites the look's value); every slider the routine is not moving takes the
look and keeps it (LC-7).
A LOOK MADE ON A CLIP, LOADED ON A LAYER: the store is keyed by the effect's name only, so the same looks are offered for
that effect on a clip, on a layer, as a global effect, in any deck and any show (F2: one struct for all three).
Two instances of one effect in a chain each have their own button; a load acts on the row it was opened from.
Tests: LC-1..LC-8. Mutants MU-EL-13..16.

### EL5 SOURCES
Out of this lane: Boris spoke of effects. One line for later: a source parameter already carries its name and uniform in
the show (`Clip::SourceParam`, Clip.h:44-57), so the same file format and store serve it under a key such as
"source:<id>", plus one button in `ClipInspector::buildSourceParamControls` and a one-clip command.

### EL6 THE OLD BUTTONS GO
VERIFIED: F26, F27.
WHICH LANE: the one-save lane, its stage S7, removes the small Save and Load, FX Save, the ten slots, `PresetManager` and
tests/test_preset_manager.cpp. This lane touches none of them. Why: S7 already exists with its lint row and its visual
state (F27); the one-save lane owns row 1 and already edits ui/PresetManager.* in its S4b; two lanes editing the same rows
would be the double removal the Harmony constraint forbids. Runner-up (a stage 0 of THIS lane) loses on those shared files.
What this lane does instead: nothing in row 1, nothing in the slot bar, nothing in PresetManager. Three notes for Harmony:
- HD-1 His answer says now. S7 depends on nothing in S1-S5 of one-save; it can run FIRST in that lane. Default: yes.
- HD-3 S7 deletes tests/test_preset_manager.cpp whole; its T6 / T7 pin the LIBRARY (unique names, registerDynamic), not the
  looks (SHEET FEL Table 2). This lane's LK-11 re-pins the part it leans on, so it does not depend on S7's choice; the
  one-save ruling should still move T6 / T7 rather than drop them.
- CLAUDE.md's "instant preset save/recall" names the feature that goes: S7 strikes it; this lane only ADDS its own words.
SIDE FINDING, not a change: the hidden legacy chain those buttons wrote into still renders after the deck compositor and is
still driven by randomise, OSC and REST (F26). Not asked for removal. Recommendation: its own small lane after S7, since
after S7 nothing on screen can load anything into it.
His four files in ~/Library/AudioDNA/Presets/ become unreachable when S7 lands (section 6 says so to him).

### EL7 PROOF
Unit rows, live rows, the take measurement and the mutants are pre-registered in section 5. The shape:
- Unit: the store's round trip by name (LK-1, ST-1); a parameter added, removed, re-ordered, re-labelled (LK-2..LK-5); an
  unreadable file beside good ones (ST-3); load = whole or nothing (LC-5, LK-7); the undo rule (LC-1..LC-4).
- Live rows, all Harmony's, through TEST-ONLY routes because production REST reaches only clip effects of the shown deck
  (F28). New routes in the `#if AUDIODNA_TEST_SERVER` block of api/ApiServer.cpp, each marshalled to the message thread and
  calling the same funnel as the menu:
  GET /api/debug/looks (query: scope, layer, column, fx; or stats=1) -> the effect, the button's painted state, the list
  with onDisk and ticked, the folder, `dirReads`;
  POST /api/debug/look_make {scope, layer, column, fx}; POST /api/debug/look_load {scope, layer, column, fx, look};
  POST /api/debug/look_rename {effect, from, to}; POST /api/debug/look_delete {effect, look};
  POST /api/debug/look_ui {scope, layer, column, fx, action} with action = menu | rename | delete | expand | fold | dismiss
  (opens the real menu or window through the same function a click calls: for the capture builder; no synthetic input).
  Values are read back through the existing GET /api/composition (read once, after a message-thread route answered:
  RIG-RULES A, SF-12) and mapped to names through GET /api/effects. Fixtures put effects on a layer and as global effects
  through /api/load_composition.
- The measurement of what a take sees: G-EL-8.

### EL8 STAGES AND ORDER
Section 4.

---------------------------------------------------------------------------------------------------------
## 4 STAGES + ORDER
---------------------------------------------------------------------------------------------------------
One lane (`lane/effect-looks`, one worktree), stages strictly in order; each stage is ONE builder context, ends with the
full ctest green and its own unit rows shown RED first (the named mutant, or the tree before the change) and then GREEN in
its report, and is reviewed pinned before the next starts. A builder never launches the app, never runs a live row, never
gives a gate verdict: every live row, every mutant arm of a live row, the visual gate and every verdict are Harmony's. Docs
move in the stage that changes the behaviour.

| stage | owns (files) | proves (unit, by the builder) | Harmony runs |
|---|---|---|---|
| S1 the look and the store | NEW src/effects/EffectLook.h, src/effects/EffectLookStore.h / .cpp; CMakeLists.txt (one source); NEW tests/test_effect_look.cpp, tests/test_look_store.cpp; tests/CMakeLists.txt (two targets) | LK-1..LK-12, ST-1..ST-10, LINT-EL-1, LINT-EL-4; MU-EL-1..12 red | nothing live (no app code path yet); the pinned review |
| S2 the load, undo, the host, the test routes | NEW src/core/EffectLookCmd.h, src/core/EffectLookOps.h; src/MainComponent.h (the store member, four funnel declarations), src/MainComponent.cpp (`effectLooksDir` beside :83-101, the store's construction, the push hook beside :1421-1438, the route callbacks); src/api/ApiServer.h / .cpp (the five data routes inside the test-server block); NEW tests/test_effect_look_cmd.cpp; tests/CMakeLists.txt; NEW .harmony/probe-effect-looks.sh (rows G-EL-1..3, 5..10, a selftest on mutated copies, quits only its own pid); docs: docs/claude/effects.md (new section "Looks per effect": format, folder, rules), docs/claude/pitfalls.md (new entry NN, text below), docs/claude/testing-eyes.md or integration.md (the test routes), docs/claude/architecture.md (the new files) | LC-1..LC-8, LINT-EL-3; MU-EL-13..16 red | G-EL-1, 2, 3, 5, 6, 7, 8, 9, 10 with their RED arms |
| S3 the menu | NEW src/ui/LooksButton.h, src/ui/EffectLooksMenu.h; src/ui/EffectStackView.h / .cpp; src/ui/InspectorPanel.h / .cpp; src/ui/ClipInspector.h, LayerInspector.h, CompositionInspector.h (forwarders only); src/MainComponent.cpp (two wiring lines); src/api/ApiServer.cpp (`look_ui`); NEW tests/test_effect_looks_menu.cpp; tests/CMakeLists.txt; docs: CLAUDE.md (the capability paragraph gains "looks per effect, kept by the app"; UI Patterns gains "Looks button"; the pitfall index gains NN), docs/claude/effects.md (the menu), .harmony/APP-INVENTORY.md | LM-1..LM-11, LINT-EL-2; MU-EL-17..20 red | G-EL-4, G-EL-11, the regression probes |
| VG the visual gate | a capture builder (window-id captures only, through `look_ui`; states V-EL-1..17 with a manifest of model facts per state), then five critic seats given his two quotes verbatim, reading R42, and the list of texts that pre-date the lane | -- | the verdict; a fix round goes back to S3's files |

Pitfall NN (Harmony assigns the number; 68 stays free, rulings-bf2.md H-17 c), text for S2: "A look is one effect's values
by NAME in one file per look; the store reads nothing at launch and never rewrites a file it could not read; a load is
`EffectLookCmd` -- one slot's `paramValues` and `dryWet` inside one fence, never a connection, a live twin or a layer field;
the button's text is derived from the values, never remembered; never use `replaceWithText` for a look."

Order against the lanes planned beside this one
- ONE-SAVE: owns the removal of the old buttons (its S7; HD-1 asks that it run first), row 1, ui/PresetManager.*, the show
  file. This lane adds NO key to the show and does not touch Clip.cpp, Layer.cpp or Composition.h. Shared files, different
  regions: src/MainComponent.cpp (this lane: the seam near :83-101, the wiring near :1421-1438, four funnel bodies),
  src/api/ApiServer.cpp (append inside the test-server block), tests/CMakeLists.txt (append). Whichever merges second
  re-bases as a builder's step 0. The quit window (one-save S5) is not touched: the store's last retry runs in its own
  destructor, not in the quit flow.
- TRANSPORT and its Undo rules (not built): this lane adds one Command class and does not edit UndoManager, Command.h or
  EffectCommands.h. When the live-layer filter is built, `EffectLookCmd` is on the "never the strip" side by construction
  (LINT-EL-3): one line in that lane's classification. Transport rebuilds the clip panel in ClipInspector.cpp; this lane
  edits only two forwarders in ClipInspector.h.
- MESSAGES: this lane adds no text that announces an event or a failure and calls no `setFileLabel`. Its two windows
  (Rename, the Delete confirm) open only on his own menu choice. The messages lane has nothing to remove here.
- OUTPUT SETTINGS / NUDGE: they add keys to settings.json; this lane does not use settings.json at all.
- No stage runs while another lane's perf A/B is in progress (RIG-RULES A: commits start the graph rebuild).

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------------
Every row is RED first. "RED arm" names the mutant (MU-EL-n, applied to a copy, reverted, the restore rebuild checked per
RIG-RULES A "after a mutant") or says "tree before the change" where the file does not exist yet.

5.1 tests/test_effect_look.cpp, tag [looks]
| id | exact test name | RED arm |
|---|---|---|
| LK-1 | "LK-1 a look round-trips by name: capture, toVar, fromVar, resolve give bit-equal values and Dry/Wet" | MU-EL-1: `toVar` writes "params" as a bare array of numbers |
| LK-2 | "LK-2 a parameter added in a later version keeps the effect's current value" | MU-EL-2: `resolve` matches by index |
| LK-3 | "LK-3 a parameter re-ordered in a later version lands by name" | MU-EL-2 |
| LK-4 | "LK-4 a parameter removed in a later version is ignored and the rest lands" | MU-EL-2 |
| LK-5 | "LK-5 a re-labelled parameter lands by its uniform; a renamed uniform lands by its label" | MU-EL-3: the label fallback removed |
| LK-6 | "LK-6 resolve gives values and Dry/Wet only: bypassed, enabled, the name, both connection arrays are untouched" | MU-EL-4: the look carries and applies `bypassed` |
| LK-7 | "LK-7 fromVar refuses the whole file: not an object, wrong format, wrong effect, params not an array, a text value, NaN, infinity" | MU-EL-5: a non-finite value is accepted |
| LK-8 | "LK-8 values outside 0..1 are clamped" | tree before the change |
| LK-9 | "LK-9 a slot matches a look within 0.0005; one slider step (0.001) away it does not" | MU-EL-6: the tolerance is 0.01 |
| LK-10 | "LK-10 the Default look is the library's defaults and Dry/Wet 1.0 for every one of the 135 effects" | tree before the change |
| LK-11 | "LK-11 the 135 effect names give 135 distinct legal folder names, compared without case; every def's uniforms and labels are distinct" | MU-EL-7: `folderNameFor` keeps only the first 4 characters |
| LK-12 | "LK-12 a float survives the JSON text bit for bit (0.12345f, 1/3, every 0.001 step)" | a mutant that writes 3 decimal places |

5.2 tests/test_look_store.cpp, tag [lookstore] (a temp folder per case)
| id | exact test name | RED arm |
|---|---|---|
| ST-1 | "ST-1 make puts one file on disk before it returns; a second store on the same folder lists it" | MU-EL-8: `make` only fills the cache |
| ST-2 | "ST-2 make names by itself: Look 1, Look 2; a freed number is used again; an existing file is never overwritten" | tree before the change |
| ST-3 | "ST-3 a bad file beside good ones: the good ones list; the empty, the half-written, the wrong-effect and the binary file are not listed and their bytes are unchanged after make, rename and remove" | MU-EL-9: the listing stops at the first bad file |
| ST-4 | "ST-4 an operation on one effect reads and writes no other effect's folder" | a mutant whose `looksFor` scans the root |
| ST-5 | "ST-5 a write that fails leaves no file and no temp file; the look lists with onDisk false; retryPending writes it once the folder can be written" (the writer hook, and a real read-only folder) | MU-EL-10: `onDisk` is always true |
| ST-6 | "ST-6 a short write is caught: the temp file is read back before the swap" | MU-EL-11: the read-back removed |
| ST-7 | "ST-7 rename moves the file, changes the name inside and keeps unknown keys; an empty, a clashing (without case), a reserved or a 41-character name is refused and nothing changes" | a mutant that rewrites the file from the struct (drops unknown keys) |
| ST-8 | "ST-8 remove deletes exactly one file; the effect's other looks stay" | tree before the change |
| ST-9 | "ST-9 the constructor reads nothing: dirReads is 0 until looksFor is called, then 1 per effect for the whole run" | MU-EL-12: the constructor scans the root |
| ST-10 | "ST-10 a file of a later version with unknown keys lists and is not rewritten by a load" | tree before the change |

5.3 tests/test_effect_look_cmd.cpp, tag [lookcmd]
| id | exact test name | RED arm |
|---|---|---|
| LC-1 | "LC-1 a load is one undo step for Global, Layer and Clip: execute sets, undo restores bit-equal, redo sets again" | tree before the change |
| LC-2 | "LC-2 a load touches one slot: every other slot's values, connections, grips and engine states are bit-equal, and so are this slot's connections and grips" | MU-EL-13: the load is pushed as a whole-chain `EffectStackCmd` |
| LC-3 | "LC-3 a slider moved on another effect after the load survives the undo of the load" | MU-EL-13 |
| LC-4 | "LC-4 nothing of the layer strip changes across execute, undo, redo: opacity, bypass, solo, mute, the trigger word, the playing clip, its play state" | MU-EL-14: `apply` also stores the layer's opacity |
| LC-5 | "LC-5 whole or not at all: a stale coordinate, another effect at that index, or another parameter count writes nothing" | MU-EL-15: the effect-name check removed |
| LC-6 | "LC-6 the fence runs once per execute, undo, redo and the write happens inside it" | MU-EL-16: the write is done before the fence call |
| LC-7 | "LC-7 a connected parameter keeps its connection and its live twin; a lane-rank grip on a parameter still accepts the lane's next write" | a mutant that touches each connection's grip |
| LC-8 | "LC-8 the undo text is Load Look '<look>' on '<effect>'" | tree before the change |

5.4 tests/test_effect_looks_menu.cpp, tag [looksmenu] (links the stack view as tests/test_effect_stack_binding.cpp does)
| id | exact test name | RED arm |
|---|---|---|
| LM-1 | "LM-1 the menu lists Default, his looks in natural order, New Look, Rename, Delete; the matched look is ticked" | tree before the change |
| LM-2 | "LM-2 Rename and Delete are enabled only while one of his looks is matched, never for Default" | a mutant that enables them always |
| LM-3 | "LM-3 the button shows the matched look's name; one slider step away it shows Looks; back on the value it shows the name" | MU-EL-17: the button remembers the last loaded name |
| LM-4 | "LM-4 a not-on-disk look carries the ring in the menu model and on the button" | MU-EL-10 |
| LM-5 | "LM-5 an effect the library does not know has no looks button" | tree before the change |
| LM-6 | "LM-6 a menu answer that arrives after the row was rebuilt onto another effect does nothing" | MU-EL-18: the re-check removed |
| LM-7 | "LM-7 a load from the menu keeps the row unfolded and pushes one PerformLook; with no host it writes inside the fence hook" | a mutant that calls rebuildRows after a load |
| LM-8 | "LM-8 a right press on a header asks for the looks menu and does not fold; a left press folds" | MU-EL-19: the button test removed (the tree before the change is also RED) |
| LM-9 | "LM-9 a drop still lands: fx:Echo,Ripple appends two effects with one PerformEdit; the looks button is not a drop target" | a mutant that makes LooksButton a DragAndDropTarget |
| LM-10 | "LM-10 the button repaints only when what it paints changes: 100 refreshes with unchanged values ask for 0 repaints" | MU-EL-20: `setState` always repaints |
| LM-11 | "LM-11 at header widths 400, 300, 220 and 160 the B button, the name, the looks button and X do not overlap; under 56 px the button shows no text" | a mutant with a fixed 110 px button |

5.5 Lint rows (static; run by the builder, re-run by Harmony), each RED on a seeded line
- LINT-EL-1 "src names the Looks folder only in src/effects/EffectLookStore.cpp and the seam in MainComponent.cpp;
  EffectLookStore.cpp names no replaceWithText".
- LINT-EL-2 "EffectStackView.cpp opens menus only with showMenuAsync and withParentComponent".
- LINT-EL-3 "src/core/EffectLookCmd.h names no paramLive, dryWetLive, paramConns, dryWetConn, runtime(, bypassed, enabled".
- LINT-EL-4 "the new files name no std::mutex, lock_guard, unique_lock; src/audio and the analysis thread do not include
  them".

5.6 Harmony's live rows (.harmony/probe-effect-looks.sh; a test-server build, --test-mode, AUDIODNA_LOOKS_DIR = a scratch
folder, launched with open -g, no Output window, no full-screen capture, no synthetic input, quits only its own pid)
| row | does | bar (exact string printed on pass) | RED arm |
|---|---|---|---|
| G-EL-1 | look_make on a clip effect | "G-EL-1 PASS onDisk=true file exists and parses before the route answered" | MU-EL-8 |
| G-EL-2 | quit own pid; relaunch on a DIFFERENT show fixture; GET looks for that effect | "G-EL-2 PASS the look is listed in another show after a relaunch" | the same relaunch on an empty looks folder lists none |
| G-EL-3 | the look made on a clip, loaded on the same effect on a layer and as a global effect | "G-EL-3 PASS 3/3 hosts hold the look's values by name" (each value within 0.0005) | MU-EL-2 on a fixture whose slot order differs |
| G-EL-4 | a clip playing on a layer; look_load on its effect and on the layer's effect | "G-EL-4 PASS strip equal (playing clip, play state, opacity, bypass, solo, mute) before = after; picture changed" where "changed" = mean absolute difference of two `captureFrame` pictures above max(1.5, 4 x noise), noise = two captures of the unchanged state | MU-EL-14 |
| G-EL-5 | after G-EL-4: set another effect's slider (production /api/set_clip_param), then /api/debug/undo | "G-EL-5 PASS undo restored the looked effect; the other effect kept its later value; strip equal; picture back within max(1.5, 4 x noise)" | MU-EL-13 |
| G-EL-6 | a garbage file put in the effect's folder before launch; then make, rename, delete of other looks | "G-EL-6 PASS good looks listed; garbage sha unchanged" | MU-EL-9 |
| G-EL-7 | the looks folder read-only (chmod 555 on the scratch folder); look_make; then chmod 755 and one more store operation | "G-EL-7 PASS onDisk=false ring=true windows unchanged file label unchanged; after chmod onDisk=true ring=false" (windows counted with Quartz; the label through /api/debug/ui_text) | MU-EL-10 |
| G-EL-8 | THE TAKE MEASUREMENT, table below | the table's line | the instrument's own control arm |
| G-EL-9 | launch on a folder holding 135 x 20 look files, no clip selected | "G-EL-9 PASS dirReads=0 after launch" | MU-EL-12 |
| G-EL-10 | a listing (names, sizes, mtimes) of the real ~/Library/Audio-DNA/ before and after the whole probe; one launch without AUDIODNA_LOOKS_DIR | "G-EL-10 PASS real folder unchanged; scratch folder named on stderr" | the probe's selftest on a mutated copy that omits the variable check |
| G-EL-11 | full ctest | "G-EL-11 PASS ctest = baseline + the new cases, 0 failed" | -- |
Regression (must stay green, unchanged): .harmony/probe-effects-parity.sh, .harmony/probe-deck-path.sh, the gate A set.

G-EL-8, pre-registered decision table (a measurement, not a pass / fail of the code). Arm a take the way the take probes
do; during it: (1) look_load on a clip effect, (2) look_load on a layer effect, (3) the control: one production
/api/set_clip_param on a third parameter. Stop. Read take.json once.
| read | predicted (F19) | if it differs |
|---|---|---|
| lanes naming the control parameter | 1 or more (the instrument sees human writes) | the instrument is wrong: STOP, fix the probe, no conclusion |
| lanes naming any parameter or Dry / Wet of the two looked effects | 0 | STOP and report: a look load IS recorded as a move; EL4's take paragraph and question 105 are re-written from the measurement |
| the state stored at Stop: the looked clip effect's non-default values | the look's values | STOP and report |
Printed line: "G-EL-8 MEASURED control lanes=<n> look lanes=<n> stop-state=<equal|differs>".

5.7 The visual gate: states to capture (window-id captures only; each with its manifest of model facts)
V-EL-1 clip inspector, an effect folded, never changed: the button reads "Default". V-EL-2 unfolded, one of his looks
matched. V-EL-3 one slider moved: the button reads "Looks", dim. V-EL-4 the menu open: Default, three looks, the matched
one ticked, New Look, Rename, Delete in red. V-EL-5 the menu open with no look matched: Rename and Delete greyed.
V-EL-6 a not-on-disk look: the ring on the button and in the menu. V-EL-7 the Rename window. V-EL-8 the Delete confirm.
V-EL-9 the layer inspector's stack. V-EL-10 the composition inspector's global effects. V-EL-11 the inspector at its
narrowest: triangle only, the effect name with an ellipsis. V-EL-12 a 40-character look name: on the button and in the
menu. V-EL-13 a bypassed effect's header. V-EL-14 two instances of one effect in a chain, on different looks.
V-EL-15 thirty looks: the menu scrolls. V-EL-16 an effect with a signal on one parameter, a look loaded. V-EL-17 the
window at 1280 x 720 and at Boris's own size.
NOT capturable without synthetic input, said so: the drop highlight over a stack with the new buttons (unit LM-9 only).

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)
---------------------------------------------------------------------------------------------------------
1. Put an effect on a clip, move its sliders, press the small button on the effect's header, choose New Look -> the button
   now reads "Look 1" -> wrong: a box asks for something, a message appears, or the button still reads "Looks".
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Look 1" is there;
   choosing it sets the sliders as you left them -> wrong: the look is missing, or it is there only in the first show.
3. With a clip playing on that layer, load a look -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move -> wrong: the layer restarts, flashes black for longer than a blink, or
   anything in the strip changes.
4. Press Cmd+Z -> the effect returns to how it was just before the look; the strip does not move; a slider you moved on
   another effect meanwhile stays where you put it -> wrong: the other effect's slider jumps back too.
5. Move one slider after loading a look -> the button reads "Looks" (dim); move it back to the exact value -> the name
   returns. Tell us if you would rather see the name stay with a mark (it is a choice we ruled, not a law).
6. Load a look on an effect that has a signal driving one slider -> that slider keeps following the signal, the others
   take the look (question 102).
7. Rename a look, delete a look -> the menu follows; deleting never moves a slider.
8. By eye on your own screens: the button's size, the dim "Looks", the ring (you should never see the ring unless the disk
   is full or the folder is locked), the menu's length with many looks.
9. When the old buttons go (the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone, and
   the four old files in ~/Library/AudioDNA/Presets/ can no longer be opened from the app. They stay on the disk, untouched.
10. Record a take, load a look in the middle, play the take back -> today's build does NOT play the look change back, the
    same as a slider you move with the mouse (question 105).

---------------------------------------------------------------------------------------------------------
## 7 QUESTIONS FOR BORIS (new numbers; each has a default A; nothing waits)
---------------------------------------------------------------------------------------------------------
101. "every effect has many looks": should the app come with looks already made for its effects?
     A (default) No. Each effect starts with "Default" and the looks you make.
     B Yes: the app also brings its own looks for each effect (they would have to be designed; a later build).
102. You load a look onto an effect that has a signal driving one of its sliders.
     A (default) The signal keeps driving that slider. The other sliders take the look.
     B The look's value shows on that slider for a moment, then the signal takes it back (as when you nudge it by hand).
103. You have loaded "Look 2" and changed it. Do you want to save the change back into "Look 2"?
     A (default) No. New settings are always kept as a new look; you can delete the old one.
     B Yes: the menu also offers "Save over Look 2".
104. When you make a new look:
     A (default) It names itself ("Look 1", "Look 2") with one click, and you rename it later if you want.
     B A name box opens every time.
105. You load a look while a take is being recorded. When the take is played back:
     A (default) The look change is not played back -- the same as a slider you move with the mouse today.
     B It is played back, and so are the sliders you move with the mouse (its own build).

---------------------------------------------------------------------------------------------------------
## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "the button that forgets". Deriving the button's text from the values means that after
   one slider move the name of the look he loaded is gone; a performer may want "Look 2, changed". Why the choice still
   stands: remembering needs a field on the slot saved in the show (three serializers that two other lanes are editing) and
   leaves stale names in every saved show after a rename or a delete; the derived state cannot lie. Cheapest refuting test:
   V-EL-3 in front of Boris (section 6 item 5). If he wants the mark, the change is one optional key "look" on the slot
   plus a compare -- a small follow-up, and the show format's first change from this feature.
R2 "values without connections is half a look". A look made on a slider that a signal drives stores the value underneath,
   which he may never have looked at. Cheapest test: section 6 item 6 and question 102. Mitigation in place: a look never
   unplugs or re-wires anything, so the worst case is a value that does nothing until the signal is removed.
R3 The take: F19 predicts a look load is invisible to a take's playback. If G-EL-8 measures otherwise the plan's take
   paragraph is wrong; the row stops the lane's S2 gate and reports. If it measures as predicted, question 105 may turn
   that into a build of its own.
R4 The fence on a playing layer: each load holds the canvas for one fenced frame (F18), as each "B" click does today.
   Loading looks rapidly in a row holds that many frames. Cheapest test: G-EL-4's picture pair plus Boris's eye (section 6
   item 3). Unfenced would let the render thread see a half-applied look and is the same data race the sliders already
   have (F13): not chosen.
R5 Many files: one file per look could reach thousands of small files. The read is per effect and lazy (ST-9, G-EL-9); the
   first show of an effect with very many looks pauses the UI briefly (INFERRED, not gated). Cheapest test: time
   `looksFor` on 200 files in ST-9's fixture and print it (reported, no bar).
R6 A name that is legal for him and not for the disk: `createLegalFileName` drops characters, so "A/B" and "AB" clash.
   The second is refused silently (ST-7). He sees a rename that does not take. Accepted: no text is allowed; the window
   simply stays as it was. If the critics find that mute, the alternative is to keep the window open with the field
   selected -- a state, not a message.
R7 Right-click on a header stops folding (HD-5). If he used it, he loses a habit. Cheapest test: section 6; the revert is
   one line.
R8 Float equality: the button's match uses 0.0005, half a slider step (the slider's step is 0.001,
   UniversalParamControl.cpp:12 per SHEET FEL 1c). A value set by REST or a routine between steps can sit within the
   tolerance of a look and show its name. Accepted: it is within what the slider itself can show.
R9 JUCE's JSON number text is ASSUMED to carry a float exactly; LK-12 is the refuting test and runs first in S1.
R10 `EffectLookCmd` and a not-yet-built undo filter: if the undo-live lane filters commands by scope, a Layer-scope look
   load on a playing layer could be classed "live" and refused. The plan's position: it is an effect edit, like the
   effect add that Boris ruled IS undone while the layer plays (binding-decisions.md, question 23's default, SHEET FEL
   Q7). That lane's ruling must name the command.
R11 Two lanes in MainComponent.cpp and ApiServer.cpp: merge conflicts are a builder's step 0 (RIG-RULES A2), not Harmony's.

---------------------------------------------------------------------------------------------------------
## 9 WHAT IS NOT IN THIS LANE
---------------------------------------------------------------------------------------------------------
- The removal of the small Save, Load, FX Save, the ten slots, PresetManager and its tests: the one-save lane, stage S7
  (EL6). The hidden legacy effect chain: stays; side finding SF-1 with a recommendation (its own lane after S7).
- Looks for procedural sources (EL5). Looks that carry connections, bypass, or a chain of several effects.
- Looks that ship with the app (question 101 B), "Save over" (103 B), a name box at creation (104 B).
- Recording a look load -- and the mouse moves of effect sliders -- into a take (question 105 B; SF-2: `onParamChanged` and
  its siblings are declared and never assigned, F12, so a take does not record an inspector slider today).
- Parameter edits as undo steps, and the structural undo that overwrites later slider edits (F16): the undo-live lane.
  Rows that stay unfolded after an undo (F17).
- Loading a look from a key, MIDI, OSC or production REST; copy / paste of an effect; re-ordering effects; a glide between
  two looks; randomise on an effect.
- A look name remembered on the slot or in the show (R1). Any change to the show file.
- SF-3 the collapsed first value and the fold arrow painted under the X button (F8); the purple inner drop highlight
  (F11); the stale comments at Clip.h:62 and EffectCommands.h (global effects "not read on the GL side");
  docs/claude/architecture.md's "UndoManager DEAD" line. Reported, not changed.
- ISF-imported effects (F5).
- From the stopped sync-dial branches: NOTHING is carried into this lane and nothing of them is dropped by it. lane/bf2
  (740b6d6) and lane/bf2-keys (9eab9bd) do not touch any file this lane owns (F29). Their carry-or-drop items (the
  music-beat wheel, the Gain 140 px, the learn title, the docs fixes; rulings-bf2.md H-17 e) belong to the recon of the
  plan that replaces the dial, each with its commit there; Pitfall 68 stays free.

- The questions still open with Boris (47 the bar counts, 48 the Record boxes, 49 the nudge number after a Resync, 50 key and
  MIDI settings between launches): this lane touches none of them; no line of this plan changes with any answer.

HARMONY'S DECISIONS asked by this plan (each has a default)
- HD-1 The old buttons are removed by one-save S7, run first in that lane. Default: yes.
- HD-2 The quit window does not name a look that is not on disk. Default: it does not.
- HD-3 One-save S7 moves T6 / T7 of tests/test_preset_manager.cpp instead of deleting them. Default: move.
- HD-4 The number of the new pitfall. Default: the next free number after 67 that is not 68.
- HD-5 A right-click on an effect header opens the looks menu and no longer folds. Default: yes.

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 14:33:44, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-effect-looks.md (status DONE; 31 attacks ruled: 14 ACCEPT, 15 PARTIAL,
2 REJECT; 26 amendments, each OVERRIDES this plan's body). Precedence for every stage, review and gate of lane "effect-looks":
Boris's verbatim words (binding-decisions.md, the 2026-10-04 sections) > this adoption > ruling-effect-looks.md > this plan's
body. Workflow run wf_ede110cb-9eb (plan: architect opus high; seats data-safety 8 attacks / 3 MUST, gates 8 / 2, stage-hands
7 / 1, scope 8 / 0 -- papers whole (38,419 characters) in attack-effect-looks-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list, decisions, measurements, strongest
counter-argument, and its section 7 (questions) in full. NOT read by me: sections 1-6 and 8-10 in the file -- the builders'
and reviewers' spec; a gate string is copied only from section 5.
HARMONY'S DECISIONS (the ruling's section 8):
HD-1  RECONCILED: the old Save, Load, FX Save and the ten slots are removed by the ONE-SAVE lane's stage S7 (its ruling,
      adopted today, names S7 and says the effect-looks lane plans no removal). ONE owner. This lane touches none of them
      (LINT-EL-5).
HD-2  DONE by me, read-only, 2026-10-04 14:33:44: ~/Library/AudioDNA/Presets/ holds a folder "fast_saves", "test 1.deck.json" (a deck file,
      47,710 B, not a preset) and TWO presets in the version-1 format: "prestest 2.json" (4 effects, 3 enabled: Ripple, Hue
      Shift, Vignette; 5 mappings, all enabled) and "test 1.json" (4 effects, all enabled: Ripple, Hue Shift, RGB Split,
      Vignette; 5 mappings, all enabled). They hold setups that differ from the defaults (enabled effects), so question 106
      IS asked, with the two names. (Counted by a JSON read; the parameter values were not compared with the library.)
HD-3  default: S7 moves T6 / T7 of tests/test_preset_manager.cpp into a library test (told to the one-save lane's S7 packet).
HD-4  my rule: lanes write "Pitfall NN"; I assign the number at each merge.
HD-5  withdrawn by the ruling (no right-click path).
HD-6  default: the take gap -- mouse-moved effect sliders and look loads are NOT recorded as moves by a take, against his "record
      all the parameter movements" (2026-10-04 12:31:04) -- is told to Boris now, filed as its own lane, nothing asked.
HD-7  default: leave the folded first value and arrow under X; named to the critics.
HD-8  default: keep the fence; tell Boris the number if GL-4 reads above 2 held frames.
HD-9  default: accept AM-10's two side effects.
HD-10 default: three mutant builds as paired; a row is RED only when its FAIL line names the check its own mutant predicts.
HD-11 default: filed in the ledger ("B" and "X" park the keyboard; Return re-toggles bypass).
HD-12 CHANGED: S1 is NOT started in this session (no build stage is); it is free to start first next session -- it shares no
      file with other lanes except the two CMake lists.
FACTS I MEASURE: GL-6 (what a take sees of a look load), GL-4 (the fence's cost), the ctest -N baseline, GL-3's noise, the
header width from the visual gate's manifest -- as the ruling's section 8.
ORDER: (one-save S7 first, in its own lane) S1 -> S2 -> S3 -> visual gate (18 states, five critic seats) -> merge.
NOT STARTED in this session: nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWERS (2026-10-04 15:08:28)
Boris, verbatim (binding-decisions.md, ""Routines" become "actions"; Boris's answers to questions 101-104, 106", 15:07:44).
His words outrank the ruling. Consequences, binding for every packet of this lane:
(1) 101: nothing ships now (the ruling's default). He makes the looks himself when the app is finished: no stage for it.
(2) 102 B: A LOOK ALSO CARRIES THE SIGNALS THAT DRIVE ITS SLIDERS, and loading plugs them in again. This OVERRULES the
    ruling's first ruled point ("no signal connections"), which the ruling itself called "a later build with its own plan".
(3) 103: a changed look is a NEW look by default (the loaded one stays as it was); "save over" the loaded look is ALSO
    offered. The ruling's "no file is ever rewritten" no longer holds as written.
(4) 104: making a look opens a NAME BOX already holding "Look X". (The ruling's one-click self-naming is replaced.)
(5) 106: his two old presets were deleted at his word (moved to the Trash by me, 15:07:44); no conversion stage; HD-2 closed.
OWED BEFORE any packet of this lane is written, S1 included (the file format changes with (2) and (3)): an architect delta
on these answers (opus max) attacked by three blind seats (data-safety, gates, stage-hands) -- launched now as lane
"looks-answers" (plan-looks-answers.md / ruling-looks-answers.md). The one-save lane's S7 (the old buttons go) is NOT touched
by this and may still run first.

## POINTER (2026-10-04 16:09:01): Boris's answers 101-104, 106 are re-stated in plan-looks-answers.md / ruling-looks-answers.md (ADOPTED); they amend this plan's stages (S1 is cut into S1a + S1b; SE is new).
