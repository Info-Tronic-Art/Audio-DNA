# RULING effect-looks -- architect ruling on the blind council's attacks on plan-effect-looks.md (lane "effect-looks": looks per effect, kept by the app for every show; s-rta-1004)

Author: architect (ruling; run on opus at max effort, as the dispatch states). Harmony decides after this; the ruling is her
working document.
Pins: `git -C /Users/boriskarpman/projects/RealTimeAudio rev-parse --short HEAD` printed 185147b and
`git status --short -- src tests docs CMakeLists.txt` printed nothing, so every code read below is a plain file at 185147b. The two
stopped worktrees (lane/bf2 740b6d6, lane/bf2-keys 9eab9bd) were not read: nothing in this lane comes from them (PL:117-119, F29).
Nothing was built, run or launched. Nothing under ~/Library was read or listed.
Seat papers, verbatim and whole: .harmony/.reports/s-rta-1004/attack-effect-looks-papers.md (4 seats, 31 attacks, 38419 characters,
a closed JSON array, parsed).
Labels: VERIFIED = I read the line at 185147b (or in JUCE under build/_deps/juce-src). SHEET = a fact-sheet row its own VERIFICATION
section confirmed. INFERRED / ASSUMED are written where used. Short names: PL:n = plan-effect-looks.md line n; FEL / FOS / FS =
facts-effect-looks.md / facts-one-save.md / facts-saves.md; BD:n = binding-decisions.md line n. Paths are under
/Users/boriskarpman/projects/RealTimeAudio. Boris is quoted only verbatim, from binding-decisions.md and boris-feedback-backlog.md.

---------------------------------------------------------------------------------------------------------
## 0 VERDICT
---------------------------------------------------------------------------------------------------------
NEEDS REVISION. The core of the plan is sound and is kept: a look is one effect's values by name, one file per look, and a load is
one small command of its own. 26 amendments (section 3) override the plan body. 31 attacks ruled: 14 ACCEPT, 15 PARTIAL, 2 REJECT.

RULED FIRST, three things.

(1) WHAT ONE LOOK HOLDS: one effect's parameter VALUES and its Dry / Wet. Each value is filed under the parameter's uniform name and
its label. Nothing else: not bypass, not `enabled`, not the signal connections on its parameters.
- Values and Dry / Wet, because Boris said (recorded 2026-10-04 12:59:09): "every effect has many looks with specific parameter
  setups. these are saved with the app. always." Dry / Wet is the first slider row of every unfolded effect, built like the others
  (EffectStackView.cpp:347-352, VERIFIED): to his eye it is one of the effect's parameters.
- The BASE values (`paramValues`, `dryWet`), never the engine's live twins: those two fields are what the on-screen sliders write
  (EffectStackView.cpp:358, :409, VERIFIED) and what the show saves. A parameter driven by a signal keeps following its signal
  after a load; the look's value sits underneath (Pitfall 33: the renderer reads `effParam`, CompositorEngine.cpp:581, :596).
- Not bypass: it is a performing switch with a second writer that a take records (FEL verification C3 a, SHEET). A look that could
  switch an effect off would make "load a look" take a picture away.
- Not the signal connections, for three facts: his words say "parameter setups" and the reading told to him (R42,
  boris-clarify-86.md) says "keep the current settings as a new look"; a load would replace wiring on a playing layer, and assigning
  a connection clears its grip and its engine memory (connect/ParamConnection.h:150-160, VERIFIED), a visible jump; user signals
  and the macro bank are saved nowhere (FOS M-6, SHEET), so a look kept for every show could carry a wire to a signal that show
  does not have, and no text may say so. This is the strongest counterargument to the ruling (section 10, R1). It is put to Boris
  as question 102 with this ruling as its default. The file format is by name and ignores keys it does not know, so "also the
  signals" would be one added optional key in a later build, never a second format.

(2) WHERE THE LOOKS LIVE, AND HOW THE STORE SURVIVES.
`~/Library/Audio-DNA/Looks/<the effect's display name>/<the look's name>.look.json`: one file per look, one folder per effect.
The FILE NAME is the look's name (amendment 2).
- An unreadable file: refused whole, not listed, never rewritten, never deleted; every other look still lists. No look file is
  ever read-modify-written: making a look writes one new file, renaming renames the file without changing a byte, deleting removes
  one file. So one bad read cannot drop anything (the settings.json trap, FS trap 8 / AppSettings.cpp:19-40 SHEET, cannot occur).
- A renamed parameter: every value carries the uniform name AND the label. A value lands by uniform; else by label; else the
  effect keeps its current value. A name the effect does not have is ignored. None is an error and none shows a text.
- A parameter added, removed or re-ordered: the same rule (by name, never by position).
- Two versions of the app: unknown keys are ignored; "version" is written and never gates reading; and because no build ever
  rewrites a look file, an older and a newer build can share the folder without damaging each other's files.
- A renamed or removed EFFECT: its folder stays on disk, unlisted and untouched. Test LK-13 (a pinned list of effect names) turns
  red on the rename before it ships.
- "always": New Look writes the file, reads it back through the same reader the menu uses, and only then lists it. Every look in
  a menu is a file on disk. A write that fails makes no look at all (the plan's "not on disk" ring and retries are cut).

(3) WHO REMOVES THE OLD SAVE, LOAD, FX SAVE AND THE TEN SLOTS: the one-save lane, its stage S7 (plan-one-save.md:568), exactly
once. This lane touches nothing of row 1, the slot bar, ui/PresetManager.* or tests/test_preset_manager.cpp; lint LINT-EL-5 proves
it on this lane's own diff. Why that lane: the removal is already specified there with its lint (LINT-6, plan-one-save.md:661)
and its visual state (V16, :527); that lane also edits ui/PresetManager.* and two cases of tests/test_preset_manager.cpp in its
S4b (:506, :565), so two lanes on those files would be the double removal. What this lane does about it: nothing in code; two
decisions for Harmony (HD-1: S7 is cut loose and runs first, because Boris answered "86 default yes" to "Remove the old Save,
Load, FX Save and the ten slots now."; HD-2: his four old preset files are looked at, read-only, before S7 merges).
Harmony must reconcile this with ruling-one-save.md, which was still a skeleton (STATUS: PARTIAL) when this was written.

THE REST, IN ONE PARAGRAPH. The menu is re-cut so that every look can be renamed or deleted without loading it (Rename and Delete
become lists of his looks), and New Look is offered only when the effect's settings are not already a look. The button keeps its
derived name: under his no-event-text rule it is the only sign that New Look worked. The button never takes the keyboard, and every
close of the menu and its two windows hands the keyboard back. A load stays one fenced write, one undo step, never the layer
strip; what the fence costs and what a take sees are measured by Harmony with decision tables, not assumed. Cmd+Z after a look no
longer folds the effect. The right-click on a header, the ring, the retry machinery, the temp-file swap, the "shader" key, the
separate Ops header and four of the plan's live rows are cut. Two plan facts were wrong and are corrected: the REST route that
writes an effect parameter is POST /api/set_param, not /api/set_clip_param; GET /api/effects lists the hidden legacy chain.

The questions named OPEN in the dispatch (47, 48, 49, 50) are answered in the record (BD:865-880) and this lane touches none of
them: no line of this ruling changes with any of those answers.

---------------------------------------------------------------------------------------------------------
## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
---------------------------------------------------------------------------------------------------------
Boris's words (the only quotes this ruling uses)
- W1 BD:894-895, recorded 2026-10-04 12:59:09: "every effect has many looks with specific parameter setups. these are saved with
  the app. always."
- W2 BD:898, recorded 2026-10-04 13:22:56: "86 default yes". Question 86 as asked (boris-clarify-86.md): "A (default) Remove the
  old Save, Load, FX Save and the ten slots now. Looks per effect come as their own build. B Keep the old buttons until the new
  looks exist." Reading R42, told to him and not corrected: "A look belongs to ONE effect: a small menu on each effect -- pick a
  look, or keep the current settings as a new look. Kept by the app for every show, the moment it is made." (Harmony's words.)
- W3 BD:764-766, recorded 2026-10-04 11:59:06: "cmd-z does not affect anything in layer strip: play, play reverse, pause,
  transparency, bypass, solo, etc. if it is  the layer strip, cmd-z does not affect it. If a clip is triggered and plays, it is
  not affected." (The plan's shorter quote, "cmd z Does not change anything in the layer strip which is by default live based",
  is verbatim too: BD:736, 2026-10-03.)
- W4 BD:771-774 (which failures show a message): "... If the user is recording a clip live, and that is not saved in a message
  should show. If the user is recording the show, that is not saved, and that should be shown. Nothing else."
- W5 BD:849-853 (what a recording holds): "... We can record the output to parameters which will record the audio file and record
  all the parameter movements which saves on hard drive space. ..." and BD:819-821: "we can only record the show which is recording
  all the parameters and the actual audio file, or recording a clip ...".
- W6 the dispatch names questions 47-50 as open; the record holds answers to all four (BD:865-880). This lane touches none.

The effect row (src/ui/EffectStackView.cpp / .h, all VERIFIED)
- E1 Header: `kHeaderHeight` 26 (h:151). "B" at (2, y+3, 24, 20); "X" at (width-26, y+3, 24, 20) (cpp:97-98). The name is drawn
  from x = 30 to the right edge (cpp:46-48). Folded, the first value is drawn right-aligned ending at width-6, from
  `paramValues[0]` (cpp:51-58), and the fold arrow is centred at width-16 (cpp:60-81): both lie under X's rectangle
  (width-26 .. width-2). The arrow is fully hidden today; the value shows as a sliver about 4 px wide left of X (GA-8's numbers
  are right).
- E2 `mouseDown` folds on any mouse button at x > 28 inside a header; there is no button test (cpp:449-464).
- E3 `setEffects` always calls `rebuildRows` (cpp:130-135); `rebuildRows` forgets every bound connection first and builds every
  row with `expanded = false` (cpp:256-289).
- E4 `refresh()` pushes `effDryWet()` / `effParam(p)` -- the LIVE twins -- to the controls and ends with an unconditional
  `repaint()` of the whole stack (cpp:188-219). It runs about 10 times a second while an inspector shows the stack
  (ClipInspector.cpp:1005; rate from the comment at MainComponent.cpp:3056, INFERRED). Consequence: a child button of the stack is
  painted on every refresh whatever its own change test says (bears on SC-4, LM-10).
- E5 The sliders write `slot.dryWet` (cpp:358) and `slot.paramValues[p]` (cpp:409) directly, unfenced. Range 0..1, step 0.001
  (UniversalParamControl.cpp:12). The Dry / Wet row is built first, default 1.0 (cpp:347-352).
- E6 The "B" click fences TWICE: the view toggles inside `runFenced` (cpp:308-311), then `onPerformEdit` builds an
  `EffectStackCmd` (MainComponent.cpp:1421-1433) and `UndoManager::perform` calls `execute()` (core/UndoManager.cpp:12-15), a
  second fenced whole-vector assignment (core/EffectCommands.h:93, :103-108). PL:285 ("the same cost as one click on B") is
  therefore wrong in the plan's disfavour: a look load is ONE fence.
- E7 Keyboard focus: every `juce::Button` wants keyboard focus (juce_Button.cpp:89) and consumes Return as a click
  (juce_Button.cpp:665-673). EffectStackView.cpp sets no focus rule on "B" or "X" (grep: no `setWantsKeyboardFocus` in the file).
  The app's precedent for a new small button is ClipInspector.cpp:445: `revealBtn_.setWantsKeyboardFocus(false)` with the comment
  "a click never parks the keyboard here (a later Return would reveal again)". Pitfall 65 (2) (docs/claude/pitfalls.md:140): JUCE
  parks focus on the first focusable descendant, "the next Return then clears the layer / fires the column", so every close calls
  `onRenameClosed` -> `grabKeyboardFocus()` (MainComponent.cpp:1399-1402). The plan has no focus rule (ST-1 is right).
- E8 The slot (src/model/Clip.h:60-114): effectName (the display name; the comment at :62 is stale), paramValues, dryWet, enabled,
  bypassed, paramConns, paramLive, dryWetConn, dryWetLive; `effParam` / `effDryWet` read the twins. `addParam` is the only append.

The library (VERIFIED unless said)
- L1 `ParamDef { name, uniformName, defaultValue }`, `EffectDef { name, category, shaderName, params, temporal }`
  (src/effects/EffectLibrary.h:17-31); `getEffectDef` is an exact match on the display name (EffectLibrary.cpp:873-881).
- L2 `grep -c 'registerEffect({'` = 135. A regex over EffectLibrary.cpp (mine, today; INFERRED-by-regex): 135 names, all distinct
  without case, none contains \ / : * ? " < > |; 333 parameters; no effect has two equal uniforms or two equal labels.
- L3 `juce::File::createLegalFileName` removes the characters " # @ , ; : < > * ^ | ? \ / (juce_File.cpp:855-857). A look named
  "Look #2" or "Warm, soft" would be silently changed by it (bears on DA-5, R6: amendment 2 does not use it for look names).

Undo and the fence (VERIFIED)
- U1 `EffectStackCmd` assigns the whole vector on execute and undo (core/EffectCommands.h:83-119). Copy-assigning a
  `ParamConnection` resets `grip` and `state` (connect/ParamConnection.h:150-160). History cap `kMaxHistory = 100`
  (core/UndoManager.h:53); no merging unless a command asks (core/Command.h).
- U2 Cmd+Z (MainComponent.cpp:4050-4063) -> `refreshAfterUndoRedo` (:5350-5402): grid sync, `ClipInspector::setClip`
  (ClipInspector.cpp:812-832 -> `setEffects` -> `rebuildRows`), `repointLayerInspector`, `rebuildCompositionEffects`. Every row of
  every shown stack comes back folded (ST-4 is right).
- U3 The fence, `UndoService::withDeckDetached` (core/UndoService.cpp:32-135): detach the deck, block the message thread until the
  GL thread finishes its in-flight frame (`executeOnGLThread(..., true)`), run the mutation, restore; afterwards `onFencedEdit`
  runs `clearClipInspectorIfUnowned` only (MainComponent.cpp:1806-1809): no row rebuild. A fenced deck-less frame re-presents the
  canvas: "no clear, no composite, no capture answered, no recorder / Syphon frame -- counted in `fence_hold_frames`;
  `fence_black_frames` ... must stay 0 in a running app" (Pitfall 55, pitfalls.md:119). Both counters are in GET /api/state
  (src/api/ApiServer.cpp:1436, :1550-1551). The plan has no row that reads them (ST-3 is right).
- U4 The renderer reads `slot.effParam(p)` and `slot.effDryWet()` (render/CompositorEngine.cpp:581, :596); effect params and
  dryWet are plain floats, not atomic (connect/ManualWrite.h:26-29).

Takes (VERIFIED)
- T1 A take records a continuous move only through `MainComponent::manualWrite` with `Origin::Human` (MainComponent.cpp:2090-2100,
  :4195-4202). The stack's sliders do not call it (E5). A take stores a state at Record and at Stop
  (recording/RecorderHost.cpp:269-270, :380-381) that holds non-default effect values (recording/PerfStateCapture.cpp:78, :114).
- T2 `takesRoot()` is ~/Documents/Audio-DNA/Takes with no test seam (MainComponent.cpp:5828-5832); the take probes write there
  with a probe-chosen name and `"audio": False` (.harmony/probe-routines.sh:95, :293).

JUCE file primitives (VERIFIED in build/_deps/juce-src/modules/juce_core)
- J1 `File::moveFileTo` deletes the target first, then renames (files/juce_File.cpp:300-315); `deleteFile` of a path that does
  not exist returns true (native/juce_SharedCode_posix.h:401-413), so a move onto a FREE name has no delete window.
- J2 `File::replaceWithText` ignores the result of writing its temp file (juce_File.cpp:798-803): FS item 4 holds.
- J3 `FileOutputStream::flushInternal` calls `fsync` (juce_SharedCode_posix.h:547-551): DA-4's "there is no fsync" is wrong for a
  write that checks the stream after `flush()`, which is what PL:185-186 specifies. No F_FULLFSYNC anywhere: true.
- J4 A `FileOutputStream` opens an existing file at its END (Pitfall 46, pitfalls.md:101).

Routes (src/api/ApiServer.cpp, VERIFIED)
- A1 The test-only block is :299-345 (`#if AUDIODNA_TEST_SERVER`): /api/debug/undo (:340), /api/debug/inspect_clip (:344; its
  handler :2455-2472 posts to the message thread and answers at once), /api/debug/ui_text (:319; its handler :2073-2090 waits on
  the message thread, so a read issued after a posted write sees it).
- A2 PLAN FACT WRONG: POST /api/set_clip_param writes clip FIELDS, "today: fitMode only" (:191-192, handler :712). The REST write
  of a clip effect's parameter is POST /api/set_param {layer, column, effect, param, value} (:187-189, :601-675): first slot of
  that name on the shown deck, through `manualWrite`. PL:471 (G-EL-5) and PL:481-482 (G-EL-8's control) name the wrong route.
- A3 PLAN FACT WRONG: GET /api/effects lists the hidden LEGACY chain (`handleListEffects`, :1253-1282), not the library. PL:347-348
  maps values to names through it. Amendment 11 has the new debug route return values by name itself.

Tests and counts (VERIFIED)
- X1 tests/test_effect_stack_binding.cpp builds an `EffectStackView` headless under `ScopedJuceInitialiser_GUI` and delivers
  mouse events by calling the handler directly (:1-45; its link list tests/CMakeLists.txt:2196-2233). tests/
  test_composition_tier_oracle.cpp mirrors the add recipe in the test (:56-70). T6 is at tests/test_preset_manager.cpp:291.
- X2 .harmony/APP-INVENTORY.md:32 pins "1249 unit tests" (GA-7 cites :31; off by one). The work log names 1252 once; not re-read.

The old buttons and the one-save lane (VERIFIED)
- O1 Row 1 lays out the three buttons at MainComponent.cpp:2756-2761 and the slot bar at :2776-2791.
- O2 plan-one-save.md: S7 "looks off (only if 86 = A)" (:568) is LAST in its order (:549); its OS6.4 still says "ANSWERS (none
  yet)" (:522) though 86 is answered (W2); LINT-6 (:661); V16 (:527, :735); S4b edits ui/PresetManager.* and removes two cases of
  tests/test_preset_manager.cpp (:506, :565). ruling-one-save.md was a skeleton, STATUS: PARTIAL, when this ruling was written.
- O3 His four files in ~/Library/AudioDNA/Presets/: 4 entries by `ls` (FOS Q6, SHEET). What they HOLD has been read by nobody; the
  sheets describe the writer's format only (the whole legacy chain by name, plus mappings: FOS Q6). Not read here either.

Seat citations that did not hold (the rest held)
- DA-4 "no fsync": refuted by J3. SC-4 "every refresh must load the effect's folder": the plan caches a folder for the run
  (PL:194-196); the cost per refresh is float compares only. ST-6 quotes "The old look buttons go now" as Boris's: those are
  Harmony's BF72 words (boris-feedback-backlog.md, the BF72 line); his are W2. GA-7's line number is off by one (X2).

---------------------------------------------------------------------------------------------------------
## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it; AM-n = the amendment in section 3)
---------------------------------------------------------------------------------------------------------
| id | sev | verdict | ruling | decided by |
|---|---|---|---|---|
| DA-1 | MUST | ACCEPT | A look equal to Default or to an earlier-named look could never be renamed or deleted. Rename and Delete become lists of ALL his looks and act on the look chosen there; New Look is offered only when the settings match no look; "matches" means "loading it would change nothing", so a file with an out-of-range value matches once loaded. AM-6, AM-1. | PL:225-227 with PL:238-239: the first match takes the name, and only the matched look had Rename / Delete. |
| DA-2 | MUST | ACCEPT (the state is removed) | The collision is real: a cache-only look has no file, so "Look 1" is free twice. Repaired at the root: there is no not-on-disk look any more (SC-1), so the file's existence is a complete collision test again. AM-5. | PL:183 against PL:189-192. |
| DA-3 | MUST | ACCEPT | G-EL-3's mutant could not turn it red; the garbage file must sort first. The live row uses a hand-written look whose entries are re-ordered, with one extra and one missing; the bad-file case is a unit row whose fixture puts bad files first and last. AM-14, AM-15. | A look made and loaded by one build has the def's own order (E8, L1): index and name agree, so MU-EL-2 is invisible. |
| DA-4 | SHOULD | PARTIAL | Right that `moveFileTo` deletes its target first (J1) and that the plan named no primitive. But this lane never writes over an existing look, so there is nothing to swap: make writes a new file on a free name, rename moves the file to a free name (no delete window, J1), delete removes one file. No temp file, so nothing to sweep. `flush()` already fsyncs (J3); F_FULLFSYNC is refused: no save in the app does it, and it would stall the message thread that also serves his keys and MIDI. The swap primitive is named for the one future case that needs it (question 103 B). AM-4. | juce_File.cpp:300-315; juce_SharedCode_posix.h:401-413, :547-551. |
| DA-5 | SHOULD | ACCEPT | Two identities with no rule. The file name becomes the only one: no "name" key inside the file; the list entry carries its file; rename and delete act on that file; rename never rewrites content. A Finder copy is simply another look. `createLegalFileName` is not used for look names (L3). AM-2. | PL:164-183 with PL:211; juce_File.cpp:855-857. |
| DA-6 | SHOULD | PARTIAL | Accepted: a pinned list of effect display names (LK-13) that turns red when one is renamed or removed; two sentences in the pitfall text (a rename orphans the folder; a changed meaning needs a new uniform name); said plainly that orphaned looks stay on disk and are not reachable from the app. Refused: an "app" key and the (display, shader) pairs: nothing reads them, and the identity of an effect is its display name (L1). The "shader" key goes too (SC-2). AM-17, AM-3. | EffectLibrary.cpp:873-881: the display name is the only key the app resolves an effect by. |
| DA-7 | SHOULD | PARTIAL | Nobody has read what his four old files hold (O3). Harmony reads them read-only before S7 merges and files what they hold; if any holds a setup that differs from the defaults, question 106 is asked. No conversion is built and no stage waits: the files are never deleted, so a later conversion loses nothing. AM-18, HD-2. | W2: he chose "Remove ... now" over "Keep the old buttons until the new looks exist". |
| DA-8 | SHOULD | PARTIAL | He must be told where his looks live and that another computer has its own. Section 6 says both. No "Show Looks Folder" item and no question: it is one more item on a menu he uses mid-show, it sends him to the Finder, and it is one line to add the day he asks. AM-19. | Smallest change; W1 ("saved with the app") is his own rule and its consequence is told, not asked. |
| GA-1 | MUST | ACCEPT | Nine rows get a named mutant each. LK-10 was a function compared with itself: it now builds the slot the way the add sites do, and LM-14 drops every registered effect through the real `itemDropped`. AM-14. | PL:410: `defaultLook(def)` checked against `def.defaultValue`. |
| GA-2 | MUST | ACCEPT | As DA-3 for G-EL-3, plus a second arm for the three hosts (a command that resolves every scope as Clip). G-EL-2's control arm proves only that the probe can see absence: its RED arm becomes MU-EL-8 (make only fills the cache). AM-14, AM-15. | PL:468-469. |
| GA-3 | SHOULD | ACCEPT | The picture bars had no pre-registered effect, values or baseline, and one undo cannot take back two loads. Row GL-3 names the effects and values, undoes each load by itself, compares against the picture taken before it, and prints the numbers. AM-15. | PL:470-471. |
| GA-4 | SHOULD | PARTIAL | Accepted: one function enumerates a folder, the counter lives there, a lint forbids any other enumeration, and the unit row has a second arm (show one effect: exactly 1). Not taken: a re-fixtured live "zero reads at launch" row. Reading one small folder the first time an effect is shown is the design; "nothing at launch" was the plan's own nicety, not a requirement. AM-13. | E4: the derived button asks for the shown effects' looks on the first refresh. |
| GA-5 | SHOULD | PARTIAL | Accepted: the fence is measured live by its own counters (GL-4); the signal-driven parameter is machine-tested as a unit row that runs one engine tick (LC-7); the success path gets a windows-and-label check (GL-1). Refused: "N consecutive frames through captureFrame": a capture is its own render and owns the time override (Pitfall 52), it does not sample the frames the screens got; the counters do. AM-15. | pitfalls.md:119 (the counters); CLAUDE.md pitfall 52. |
| GA-6 | SHOULD | ACCEPT | The seam's logic moves into the store as a pure function with a unit row and a mutant that is never launched (it would write into his real folder). The live row lists the Looks folder only. AM-12, AM-15. | PL:476 lists a folder his own app writes (settings.json). |
| GA-7 | SHOULD | ACCEPT | Harmony measures the baseline with `ctest -N` before S1; the delta is exact (50, one TEST_CASE per row id); no test hard-codes 135; the folder lint greps the path call, not the word; S3 updates the inventory's count line. AM-21. | APP-INVENTORY.md:32. |
| GA-8 | NIT | PARTIAL | The numbers are right (E1). But the value and the arrow are not laid out by this lane and already sit under X; adding them to LM-11 would force this lane to move them into view, a visible change nobody asked for. They are named to the critics as pre-dating the lane, and HD-7 covers a fix round. The LM-8 half is moot: the right-click path is cut (SC-3). AM-25. | EffectStackView.cpp:51-58, :97-98. |
| ST-1 | MUST | ACCEPT | The button never wants focus and a click on it never grabs focus; the menu's close, the Rename window's close and the Delete confirm's close each hand the keyboard to MainComponent, as the deck rename does. Unit row LM-8; a counter in the debug route as the live witness (a background test app is never the key window, so "which component has focus" cannot be read live). AM-7, AM-8. | juce_Button.cpp:89, :665-673; ClipInspector.cpp:445; pitfalls.md:140. |
| ST-2 | SHOULD | PARTIAL | Accepted with DA-1: rename and delete without loading; "New Look" on settings that already are a look is not offered, so a new look is always the one the button then shows. Refused: a "most recently used" hint kept in the view; it is row memory that dies at every rebuild (E3), and the cases it mends no longer arise from the app's own menu. AM-6. | PL:228-232 (why the state is derived). |
| ST-3 | SHOULD | PARTIAL | Accepted: the cost is measured (GL-4, a decision table) and stated: one fence per load, each fenced frame gives the screens the previous picture and gives the recorder and Syphon no frame. The fence stays: Harmony constraint: "Loading a look is ONE step, whole or not at all"; unfenced, a frame could be drawn with half a look, and on a feedback effect that frame stays in the trail. AM-9, HD-8. | pitfalls.md:119; core/UndoService.cpp:32-135. |
| ST-4 | SHOULD | PARTIAL | Real and worth mending inside this lane's own file: `setEffects` keeps a row's fold state when it is handed the SAME chain and that index still holds the same effect. Refused: a special case in `refreshAfterUndoRedo` or a new flag on `Command`: those are the undo-live lane's files. AM-10. | MainComponent.cpp:5350-5402; EffectStackView.cpp:130-135, :289. |
| ST-5 | SHOULD | ACCEPT | Stated: capture, resolve and matches read and write `paramValues` and `dryWet` only. Rows LK-14 and LM-12; the manifest of state V-17 carries the button text and the base and live values. AM-1. | EffectStackView.cpp:209, :215 (what the sliders SHOW is the live twin). |
| ST-6 | SHOULD | PARTIAL | "S7 only after S3" is refused by his own answer (W2: he was offered exactly that as B and took A). The rest is DA-7's ruling. The seat's quote "The old look buttons go now" is Harmony's text, not his. AM-18. | boris-clarify-86.md, question 86 as asked. |
| ST-7 | SHOULD | ACCEPT | One live row drives the real menu handler: `look_ui` gains "pick", which calls the function the PopupMenu's callback calls. Row GL-5 picks on the SECOND of two instances of one effect on a layer, so a wrong index or scope fails. The data routes for rename and delete are cut instead. AM-11, AM-15. | PL:345 (no pick), PL:368 (the wiring is two lines nobody gates). |
| SC-1 | SHOULD | ACCEPT | A look that exists only until quit is what "always" forbids, and its ring is a state he was never told about. A failed write makes no look: the button does not change, which is how he sees it. Ring, retry, `onDisk`, the quit-time retry, HD-2 (old), G-EL-7 and V-EL-6 go. AM-5. | Harmony constraint: "'always' means a look is on disk the moment it is made". |
| SC-2 | SHOULD | PARTIAL | Accepted: no temp-and-swap (nothing is ever overwritten); no "shader" key; rename no longer rewrites, so unknown-key keeping needs no code. Kept: a read-back, as a parse through the listing's own reader (it is what makes "in the menu = readable on disk" true, and it catches the append trap, J4); "version": 1 stays (one key, lets a later reader branch). AM-3, AM-4. | PL:183 (make never overwrites); pitfalls.md:101. |
| SC-3 | SHOULD | ACCEPT | Not asked for, changes a shipped habit, and the button is one click away. `mouseDown` is not edited. LM-8 (old), MU-EL-19 (old), R7 and HD-5 go. AM-6. | E2; nothing in W1 / W2 / R42 names the header click. |
| SC-4 | SHOULD | REJECT | The name on the button stays. Under W4 no text may say "look saved": the button changing to "Look 1" is the only sign New Look worked, and its NOT changing is the only sign a write failed (SC-1). The cost claim does not hold: the folder is read once per run, a refresh costs a few hundred float compares, and the stack already repaints itself on every refresh (E4). The seat's fallback (tick only in the menu) stays one line away if Boris finds the header crowded (section 6, item 5). | BD:771-774 ("Nothing else."); EffectStackView.cpp:218. |
| SC-5 | NIT | REJECT | Both names stay. Harmony constraint: the store must survive "a renamed parameter"; with the uniform alone a renamed uniform loses its value silently. The label is one short string per value. LK-11 re-pins that both are unique per effect, since T6 leaves with the old buttons. | The dispatch's rule-first clause; tests/test_preset_manager.cpp:291. |
| SC-6 | SHOULD | PARTIAL | Accepted: old G-EL-1 folds into the relaunch row; old 6, 7, 9 become unit rows; three data routes and one UI route instead of six; question 105 is not asked. Refused: dropping the take measurement. Harmony constraint: "what a take being recorded sees of it is MEASURED, not assumed". 105 is not asked because W5 already says a take records "all the parameter movements": the gap is reported up (HD-6), not put to him as a choice. AM-15, AM-20. | BD:849-851; the dispatch's constraint. |
| SC-7 | SHOULD | PARTIAL | The inconsistency is real and the sentence is re-worded: New Look never FORCES a text box (it may be pressed mid-show); Rename is his own unhurried choice. Rename stays: with self-given names and no rename the looks are "Look 1 .. Look 40" for ever. Question 104 stays as written. AM-6, AM-24. | PL:244-249. |
| SC-8 | NIT | PARTIAL | Accepted: `EffectLookOps.h` is cut (the app always has a host; with none a pick writes nothing); the visual states go from 17 to 18: the ring state goes, the bypassed header is merged into another state, and three are added by the amendments (the first menu he will see, the Rename list, the Delete list). Refused: three critics (Harmony constraint: five critic seats) and a side branch for a parallel stage (every stage edits tests/CMakeLists.txt in one worktree; RIG-RULES A2 gives a parallel stage its own worktree and a merge step, which costs more than the stage it saves). AM-9, AM-16. | RIG-RULES.md A2 "TWO BUILDERS NEVER SHARE A WORKTREE". |

Where seats conflicted, and how it was settled
- DA-2 (mend the not-on-disk state) against SC-1 (remove it): SC-1. A state that can lose a look at quit cannot be mended into
  "always".
- DA-4 (a stronger swap, a full flush, a sweep) against SC-2 (no temp, no read-back): neither whole. With file-name identity no
  look file is ever overwritten, so the swap, the temp file and the sweep have nothing to do; the read-back survives as a parse.
- DA-1 / ST-2 (reach every look) against SC-7 (cut Rename) and SC-4 (cut the name): the menu is re-cut so both hold: every look
  reachable, Rename kept, the name kept.
- DA-6 (more keys) against SC-2 (fewer keys): the format loses "shader" and gains nothing; the protection DA-6 wants is a test.
- GA-5 (more live rows) against SC-6 (fewer): a live row stays only where a unit row cannot see it: the app's own wiring
  (GL-1, GL-2, GL-5), the render thread (GL-3, GL-4), the recorder (GL-6) and his real folder (GL-7).
- DA-3 and GA-2 offered two different repairs of G-EL-3: both are taken (two RED arms).

---------------------------------------------------------------------------------------------------------
## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
---------------------------------------------------------------------------------------------------------
AM-1 WHAT A LOOK HOLDS, EXACTLY (EL1; ST-5, DA-1).
- For ONE effect: the effect's display name, Dry / Wet, and one entry per parameter {uniform, label, value}.
- `looks::capture(def, slot)` reads `slot.paramValues[i]` and `slot.dryWet` and nothing else.
- `looks::resolve(def, slot, look) -> Values`: for each def parameter i below `slot.paramValues.size()`: the look's entry with
  the same uniform; else the entry with the same label; else the slot's current value. Clamped to [0, 1]. Dry / Wet likewise. An
  entry that names no parameter of the def is ignored.
- `looks::matches(def, slot, look)`: true exactly when at least one entry of the look lands on a parameter of the def (a look
  from a file; Default is exempt) AND every
  value `resolve` gives is within 0.0005 of the slot's present base value, Dry / Wet included. In words: loading it would change
  nothing. It never reads `effParam` / `effDryWet`.
- `looks::firstMatch(def, slot, looks)`: Default first, then his looks in natural name order; the first that matches.
- The Default look: built from the def (each `defaultValue`, Dry / Wet 1.0); never a file; cannot be renamed or deleted.
- On a parameter a signal drives: the signal keeps driving; the look's value is the base underneath and shows when the signal
  is unplugged. No grip is opened, no connection is touched.

AM-2 THE FILE IS THE LOOK (EL2; DA-5).
- Path: `<root>/<looks::folderNameFor(effect)>/<look name>.look.json`. `folderNameFor` replaces each of \ / : * ? " < > | and
  each control character by "_" (no registered effect name holds one today, L2; LK-11 pins it).
- The look's name is the file's name without ".look.json". Nothing inside the file names the look.
- `looks::isLegalName(name)`, after trimming: 1 to 40 characters; none of \ / : * ? " < > | and no control character; does not
  start with "."; is not "Default", "Looks", "New Look", "Rename" or "Delete" in any letter case (the menu's own words). `juce::File::createLegalFileName` is not used: it would
  silently drop # @ , ; ^ from a name (L3).
- A list entry is {name, file, the parsed look}. `rename(effect, from, to)`: `isLegalName(to)`; the target file does not exist,
  or is the same file (a change of letter case alone is allowed); then `file.moveFileTo(target)` -- a free name, so no delete
  window (J1). The file's bytes do not change. `remove(effect, name)` deletes that entry's file and nothing else.
- A file copied or renamed in the Finder is simply a look under its file name. A folder is read the first time that effect's
  looks are asked for in a run; a file added by hand later appears at the next launch.

AM-3 THE FORMAT (EL2; SC-2, SC-5, DA-6).
- A JSON object: "format": "audio-dna-look"; "version": 1; "effect": the display name; "dryWet": number; "params": an array of
  {"uniform", "label", "value"}. No "name", no "shader", no "app". Unknown keys are ignored. A "version" above 1 is read by the
  same rules.
- Refused whole (not listed, not touched): it does not parse; the root is not an object; "format" differs; "effect" is not
  exactly the effect asked for; "params" is not an array; an entry is not an object; a "value" or "dryWet" is not a finite number.
  A folder, a file whose name does not end in ".look.json", and a file whose name starts with "." are skipped.
- A value is written so that a float survives bit for bit: LK-12 runs FIRST in S1. If JUCE's JSON writer does not do it, the
  store formats the number itself with 9 significant digits; the file is plain JSON either way.

AM-4 THE WRITE (EL2 "THE WRITE"; DA-4, SC-2).
- `make`: (1) the name is "Look N", N the smallest number whose file does not exist; (2) create the effect's folder; (3) refuse
  if the path exists; (4) write through a `juce::FileOutputStream` on that new path and check the stream after `write` and after
  `flush()` (which fsyncs, J3); (5) read the file back through the same function the listing uses and require the same values
  bit for bit; (6) only then add it to the list and return it. Any failed step deletes the file this call created and returns
  nullptr.
- Not used for a look anywhere: `replaceWithText`, `replaceWithData`, `TemporaryFile`, `replaceFileIn`. No F_FULLFSYNC.
- If Boris answers 103 = B ("Save over"): that is the one operation that writes over an existing look. It then writes a sibling
  temp file and swaps it in with `juce::File::replaceFileIn`, never `moveFileTo` (J1). Not built now: one new store function.
- A crash in the middle of step 4 leaves a file that does not parse: hidden by AM-3, never deleted by the app, its name not
  given out again (section 10, R5).

AM-5 NO LOOK THAT IS NOT ON DISK (EL2; SC-1, DA-2).
PL:189-193 is struck: no ring, no `retryPending`, no `onDisk`, no retry at quit, no writer state. A `make` that fails returns
nullptr; the menu action then does nothing and shows nothing; the button does not change. Struck with it: the plan's ST-5, LM-4,
G-EL-7, V-EL-6, HD-2 and MU-EL-10 in their old meaning. The new ST-5 pins the failure.

AM-6 THE MENU (EL3; DA-1, ST-2, SC-3, SC-7). Top to bottom:
   "Default" / separator / his looks in natural name order / separator / "New Look" / "Rename" / "Delete".
- At most one item is ticked: the one `firstMatch` names.
- "New Look" is enabled only when `firstMatch` finds nothing: the settings are not already a look, Default included. So a new
  look is always the one the button then names.
- "Rename" opens a list of his looks; choosing one opens the Rename window for THAT look. "Delete" opens a list of his looks,
  each in warning red through `addColouredItem` (the precedent: ui/DeckView.cpp:945); choosing one opens the confirm for THAT
  look. Both are enabled when he has at least one look, never list Default, and need no load.
- Clicking a look loads it (AM-9). A pick that would change no value pushes no undo step.
- Shown with `showMenuAsync`, `.withParentComponent(getTopLevelComponent())`, the button as target. The lane adds no slider.
- The callback holds a SafePointer to the view, the row's effect index, the effect's name and the files the menu was built
  from; it does nothing unless that index still holds that effect (LM-6).
- No right-click path. `EffectStackView::mouseDown` is not edited: a right-click on a header folds, as it does today (SC-3).
- The texts, exact. Button: the matched look's name, or "Looks". Menu: "Default", "New Look", "Rename", "Delete". Rename window:
  title "Rename Look", one text field holding the present name, buttons "Rename" and "Cancel". Delete confirm: title
  "Delete Look", text `Delete look "<name>" of <effect>? This cannot be undone.`, buttons "Delete" and "Cancel" (the model is the
  Delete Routine window, MainComponent.cpp:6364-6365). Edit menu: "Undo Load Look '<look>' on '<effect>'".
- New Look names the look by itself and opens no window, because it may be pressed in the middle of a show. Rename is his own
  unhurried choice. (This replaces the reason at PL:245-246.)
- Making, renaming and deleting a look are not undo steps: they change the app's store, not the show.

AM-7 THE BUTTON (EL3; ST-1; SC-4 rejected).
`LooksButton` (NEW src/ui/LooksButton.h) calls `setWantsKeyboardFocus(false)` and `setMouseClickGrabsKeyboardFocus(false)` in its
constructor. It paints its text and a down triangle drawn as a path. Its state is set from `EffectStackView::refresh()`: the
name `firstMatch` gives, in the primary text colour; else "Looks" in the secondary colour; under 56 px wide, the triangle alone.
`setState` compares what it paints and asks for a repaint only on a change (LM-10). That row pins that the button adds no repaint
request of its own; it does not make the window paint less, because the stack repaints itself on every refresh (E4). No ring.
Layout as PL:219-221. An effect the library does not know gets no button. After `make`, `rename` and `remove` the view calls
its own `refresh()`, so the button follows at once.

AM-8 THE KEYBOARD GOES HOME (ST-1).
`EffectStackView::onLooksUiClosed` (a `std::function<void()>`), forwarded by InspectorPanel and the three inspectors in the
`setEffectPerformEdit` pattern (InspectorPanel.cpp:145-150), wired in MainComponent to the two lines `onRenameClosed` runs
(MainComponent.cpp:1399-1402: a counter, then `grabKeyboardFocus()`). Fired once at every close of the menu (a pick or a
dismiss), of the Rename window and of the Delete confirm. The counter is `focusHome` in GET /api/debug/looks?stats=1.

AM-9 THE LOAD (EL4; SC-8, ST-3).
- The view computes `before` (the slot's `paramValues` and `dryWet`) and `after = looks::resolve(def, slot, look)` and calls
  `onPerformLook(scope_, fxIndex, effectName, before, after, description)`. MainComponent builds ONE `EffectLookCmd`
  (PL:270-276) and `pushCommands` runs it once. The view itself writes nothing; with no host wired a pick writes nothing.
  `src/core/EffectLookOps.h` is not created.
- One fence per execute, undo and redo: ONE, where a "B" click costs two (E6). What a fence costs is measured by GL-4 and told
  to Boris in section 6.
- `EffectLookCmd` writes `paramValues[i]` and `dryWet` of one slot, after checking that index `fxIndex` still holds
  `effectName` with the same parameter count; else it writes nothing. No merging: every load is its own undo step.
- Threads: the store, the menu, the two windows and the command run on the message thread only. No lock and no new mutex
  (LINT-EL-4). Nothing is added to the audio callback or the analysis thread. The render thread never waits: the fence blocks
  the MESSAGE thread until the GL thread has finished its frame (U3), never the other way round.

AM-10 CMD+Z KEEPS THE EFFECT OPEN (EL4; ST-4).
`EffectStackView::setEffects(effects, scope)`: when `effects` is the vector the view already shows and `scope` has the same four
fields, each rebuilt row whose index held the SAME effect name before keeps its `expanded` flag. In every other case rows fold
as today. `rebuildRows` called by the view's own delete and drop is unchanged. Not edited: core/UndoManager.*, core/Command.h,
`MainComponent::refreshAfterUndoRedo`. Two side effects, both wanted: undo of an add, a remove or a bypass keeps the rows that
still exist as they were; a second click on the selected clip no longer folds its effects (HD-9).

AM-11 THE TEST ROUTES (EL7; ST-7, SC-6; A2, A3). Four routes inside `#if AUDIODNA_TEST_SERVER` (ApiServer.cpp:299-345):
- GET /api/debug/looks ?scope=clip|layer|global &layer &column &fx -- answered FROM the message thread (the ui_text pattern,
  ApiServer.cpp:2073-2090), so a GET sent after a POST sees it. It returns: `effect`; `values` = [{uniform, label, value}] and
  `dryWet` (the BASE fields); `matched` (the name `firstMatch` gives, or ""); `looks` = [{name, ticked}]; `menu` = {newLook,
  rename, delete} (enabled or not); `folder`; and, once S3 has landed and that row is on screen, `button` = {text, dim, mode}.
  With ?stats=1: `root`, `dirReads`, `focusHome`.
- POST /api/debug/look_make {scope, layer, column, fx}. POST /api/debug/look_load {scope, layer, column, fx, look}; the
  look "Default" names the built-in one.
- POST /api/debug/look_ui {scope, layer, column, fx, action} (S3), action = menu | pick:<menu item text> | rename:<look> |
  delete:<look> | renamelist | deletelist | expand | fold | dismiss. It first shows the inspector tab that holds that stack.
  "pick" calls the function the PopupMenu's own callback calls. "rename" / "delete" open the window for that look. "renamelist" /
  "deletelist" show that list by itself: JUCE opens a sub-list only on hover or an arrow key, and neither may be faked.
- Cut: /api/debug/look_rename and /api/debug/look_delete. Probes and the capture builder pre-write look files into the scratch
  folder before launch.
- Probes read effect values through GET /api/debug/looks, never GET /api/effects (A3). The strip is read through
  GET /api/composition, once, after a message-thread route answered (RIG-RULES A). "Another effect's slider" and the take's
  control arm use POST /api/set_param {layer, column, effect, param, value} (A2).

AM-12 THE TEST-MODE FOLDER (EL2 "Test mode"; GA-6).
`EffectLookStore::defaultRoot()` -- the real folder, and the ONE place in src that spells "Looks" as a path -- and
`EffectLookStore::testModeRoot(const juce::String& envValue)` -- an absolute path as given; anything else gives a scratch
folder under the temp directory, never under the user's Library -- are static functions of the store, unit-tested (ST-11).
`effectLooksDir(bool testMode)` in MainComponent.cpp is the thin wrapper beside `appSettingsFile` (:83-101): in a test-server
build running --test-mode it reads AUDIODNA_LOOKS_DIR and calls `testModeRoot`; it prints one stderr line naming the folder.
The real folder is created by the first successful `make`, never at launch.

AM-13 ONE PLACE READS A FOLDER (EL2 "READ"; GA-4).
`EffectLookStore::listFolder(effect)` is the only code that enumerates a directory; `dirReads` counts there; `looksFor(effect)`
calls it once per effect per run; the constructor reads nothing. LINT-EL-1 forbids any other enumeration in the new files. The
plan's G-EL-9 is cut. ST-9 has two arms and prints the time of one read of a 200-file folder (reported, no bar).

AM-14 RED ARMS (section 5; GA-1, GA-2, DA-3). Every unit row has a named mutant; "the tree before the change" is used for no
unit row. LK-10 is rewritten and LM-14 added. Mutants may be applied in batches of up to six when they sit in different
functions; the stage report quotes each row's own failing assertion; the restore rebuild is checked per RIG-RULES A.

AM-15 THE LIVE ROWS (section 5.6 replaces PL:463-488). Eight rows, GL-1..GL-8. From the plan: G-EL-1 + G-EL-2 -> GL-1; G-EL-3 ->
GL-2; G-EL-4 + G-EL-5 -> GL-3; GL-4 is new (the fence); GL-5 is new (the real menu handler); G-EL-8 -> GL-6; G-EL-10 -> GL-7;
G-EL-11 -> GL-8; G-EL-6, G-EL-7, G-EL-9 -> the unit rows ST-3, ST-5, ST-9. Gate strings are copied only from section 5.6.

AM-16 THE VISUAL GATE (section 5.7 replaces PL:490-500): 18 states, V-1..V-18; five critic seats.

AM-17 A RENAMED EFFECT, A CHANGED MEANING (DA-6). LK-13 pins the display names. The pitfall text (section 4) carries two
sentences for the next builder. Looks of an effect the library no longer knows stay on disk, unlisted; the app cannot reach them.

AM-18 HIS FOUR OLD FILES (EL6; DA-7, ST-6). HD-2. No conversion is built in this lane; PL:332 stays true and section 6 says it.

AM-19 HE IS TOLD WHERE THE LOOKS LIVE (DA-8): section 6, item 11. No menu item, no question.

AM-20 TAKES (EL4 "A TAKE BEING RECORDED"; SC-6). PL:299-303 stays as the PREDICTION; GL-6 measures it; question 105 is withdrawn
(W5 already says what a take records); the gap goes up as HD-6.

AM-21 COUNTS (GA-7). No test names the number 135: LK-10, LK-11 and LM-14 run over every registered effect. GL-8's baseline is
measured by Harmony before S1. S3 updates APP-INVENTORY.md: the unit-test count (:32), the route count, the Looks surface.

AM-22 THE OLD BUTTONS (EL6): section 0 (3). Lint LINT-EL-5 runs on this lane's diff. HD-1, HD-3.

AM-23 QUESTION 102 IS RE-CUT (section 7): it asks what a look holds about signals. The behaviour PL:532-534 asked about is
ruled, not asked: the signal keeps driving (AM-1).

AM-24 THE RENAME WINDOW (EL3; R6, SC-7). The `renameRoutine` idiom (MainComponent.cpp:6328-6350): a `juce::AlertWindow`, opened
by the view. Its field carries an input filter (`LookNameFilter`, in src/ui/EffectLooksMenu.h) that stops at 40 characters and
drops the characters a look name may not hold, so they cannot be typed: a state, not a message. A name that is still refused
(empty, already taken, or one of the menu's own words) changes nothing and shows nothing; the window closes.

AM-25 WHAT THIS LANE DOES NOT LAY OUT (GA-8). LM-11 checks B, the name, the looks button and X. The folded first value and the
fold arrow under X (E1) are not moved; the capture manifest lists them among the things that pre-date the lane. HD-7.

AM-26 NO NEW TEXT THAT ANNOUNCES ANYTHING (W4). Lint LINT-EL-6 on the lane's diff. The lane's only windows are the two above,
built with `new juce::AlertWindow` and `enterModalState` as `renameRoutine` is, opened only by his own menu choice.

---------------------------------------------------------------------------------------------------------
## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
---------------------------------------------------------------------------------------------------------
One lane (`lane/effect-looks`, one worktree), three build stages strictly in order, then the visual gate. Each stage is ONE
builder context, ends with the full ctest green, shows its own unit rows RED first (the named mutant) and then GREEN in its
report, and is reviewed pinned before the next starts. A builder never launches the app, never runs a live row, never gives a
gate verdict. Docs move in the stage that changes the behaviour.

| stage | owns (files) | proves (unit, by the builder) | Harmony runs herself, and when |
|---|---|---|---|
| S1 the look and the store | NEW src/effects/EffectLook.h; NEW src/effects/EffectLookStore.h / .cpp; CMakeLists.txt (one source); NEW tests/test_effect_look.cpp; NEW tests/test_look_store.cpp; tests/CMakeLists.txt (two targets). src/effects/EffectLibrary.cpp is touched only as mutant MU-EL-28, reverted. | LK-12 first, then LK-1..LK-15, ST-1..ST-12, LINT-EL-1, LINT-EL-4; their mutants RED | BEFORE S1: `ctest -N` on the lane's base, the baseline N0 of GL-8. After S1: nothing live (no app code path yet); the pinned review. |
| S2 the load, undo, the host, the data routes, the probe | NEW src/core/EffectLookCmd.h; src/MainComponent.h (the store member, `performLookLoad`, the three route callbacks); src/MainComponent.cpp (`effectLooksDir` beside :83-101, the store's construction, `performLookLoad`, the route callbacks); src/api/ApiServer.h / .cpp (GET /api/debug/looks, POST look_make, POST look_load, inside :299-345); NEW tests/test_effect_look_cmd.cpp; tests/CMakeLists.txt; NEW .harmony/probe-effect-looks.sh with its fixtures and a selftest (rows GL-1, 2, 3, 4, 6, 7; quits only its own pid); docs: docs/claude/effects.md (new section "Looks per effect": what a look holds, the folder, the format, the rules), docs/claude/pitfalls.md (entry NN, text below), docs/claude/testing-eyes.md (the routes), docs/claude/architecture.md (the new files) | LC-1..LC-8, LINT-EL-3, LINT-EL-5, LINT-EL-6; their mutants RED | After the pinned review: GL-1, GL-2, GL-3, GL-4, GL-6, GL-7, each with its RED arm. GL-4 and GL-6 are measurements with decision tables: a STOP outcome stops the lane before S3. |
| S3 the menu | NEW src/ui/LooksButton.h; NEW src/ui/EffectLooksMenu.h (the pure menu model and `LookNameFilter`); src/ui/EffectStackView.h / .cpp (the row's button, `setLookStore`, `onPerformLook`, `onLooksUiClosed`, the menu, the Rename window, the Delete confirm, `resized`, the name rectangle in `paint`, `refresh`, `rebuildRows`, `setEffects` per AM-10; NOT `mouseDown`); src/ui/InspectorPanel.h / .cpp; src/ui/ClipInspector.h, LayerInspector.h, CompositionInspector.h (forwarders only); src/MainComponent.cpp (three wiring lines beside :1421-1438, the `look_ui` callback); src/api/ApiServer.h / .cpp (`look_ui`); NEW tests/test_effect_looks_menu.cpp; tests/CMakeLists.txt; .harmony/probe-effect-looks.sh (row GL-5); docs: CLAUDE.md (the capability paragraph gains "looks per effect, kept by the app"; UI Patterns gains "Looks button"; the pitfall index gains NN), docs/claude/effects.md (the menu), .harmony/APP-INVENTORY.md (AM-21) | LM-1..LM-15, LINT-EL-2; their mutants RED | GL-5 with its RED arm; GL-8; the regression probes; then GL-1, GL-2, GL-3, GL-4, GL-7 once more at the lane's final head (GREEN only). |
| VG the visual gate | a capture builder (window-id captures only, driven through `look_ui`; states V-1..V-18, each with a manifest of model facts), then FIVE critic seats given W1 and W2 verbatim, reading R42 marked as Harmony's words, and the list of things that pre-date the lane (section 5.7) | -- | The verdict. A fix round goes back to S3's files. Boris sees nothing of this lane before it passes. |

What Harmony runs herself, in order: (0) the `ctest -N` baseline, and HD-2's read-only look at his four old files (any time
before one-save S7 merges); (1) after S2's review, the six live rows above; (2) after S3's review, GL-5, GL-8, the regression
probes and the re-run; (3) the visual gate; (4) the merge sequence of RIG-RULES B (RED on the pre-merge copy, merge, build,
ctest, GREEN). Every live row takes the live lock; none runs while another lane's perf A/B is in progress.

Pitfall NN (Harmony assigns the number, HD-4), text for S2:
"A look is one effect's BASE values (`paramValues`, `dryWet`) by NAME -- uniform first, then label -- in one file per look:
`~/Library/Audio-DNA/Looks/<effect display name>/<look name>.look.json`; the file name is the look's name. The store never
rewrites a look file (make writes a new file and reads it back before listing it; rename renames the file; delete removes it),
never deletes or rewrites a file it could not read, and reads an effect's folder the first time that effect's looks are asked
for. Every look in a menu is a file on disk: a write that fails makes no look. A load is `EffectLookCmd` -- one slot's
`paramValues` and `dryWet` inside one fence, never a connection, a live twin, `bypassed` or a layer field. The button's text is
derived from the values, never remembered, and the button never takes the keyboard. Renaming an effect's display name orphans
its Looks folder and every saved show that names it: `tests/test_effect_look.cpp` LK-13 fails first. Changing what a uniform's
0..1 means changes every saved look and show: give the parameter a new uniform name instead."

Order against the lanes planned beside this one
- ONE-SAVE owns the removal of the old buttons (its S7), row 1, ui/PresetManager.* and the show file. This lane adds NO key to
  the show and does not touch Clip.cpp, Layer.cpp or Composition.h. Shared files, different regions: src/MainComponent.cpp / .h,
  src/api/ApiServer.cpp / .h (this lane appends inside the test-only block), tests/CMakeLists.txt (append), CLAUDE.md (S7 strikes
  "instant preset save/recall"; this lane's S3 adds its own words), .harmony/APP-INVENTORY.md. Whichever merges second re-bases
  as a builder's step 0. Either order of S7 and this lane is safe: neither reads the other's code.
- TRANSPORT and its Undo rules (not built): this lane adds one Command class and does not edit core/UndoManager.*, core/Command.h
  or core/EffectCommands.h. That lane's ruling must name `EffectLookCmd` on the "never the layer strip" side; LINT-EL-3 and LC-4
  are the proof. Transport rebuilds the clip panel in ClipInspector.cpp; this lane edits ClipInspector.h only (forwarders).
- MESSAGES: this lane adds no text that announces an event or a failure (LINT-EL-6). Nothing for that lane to remove here.
- OUTPUT SETTINGS / NUDGE: they add keys to settings.json; this lane does not use settings.json.

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; exact strings and bars; each row's RED arm; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------------
This section REPLACES the plan's section 5. Every row is RED first. One TEST_CASE per row id, named exactly as written (the
count in GL-8 leans on it). A mutant is applied to the working tree, shown RED, reverted, and the restore rebuild is checked
(RIG-RULES A, "AFTER A MUTANT"). The mutants are listed once, in 5.5.

5.1 tests/test_effect_look.cpp, tag [looks] (S1; 15 rows)
| id | exact test name | RED arm |
|---|---|---|
| LK-1 | "LK-1 a look round-trips by name: capture, toVar, fromVar, resolve give bit-equal values and Dry/Wet" | MU-EL-1 |
| LK-2 | "LK-2 a parameter added in a later version keeps the effect's current value" | MU-EL-2 |
| LK-3 | "LK-3 a parameter re-ordered in a later version lands by name" | MU-EL-2 |
| LK-4 | "LK-4 a parameter removed in a later version is ignored and the rest lands" | MU-EL-2 |
| LK-5 | "LK-5 a re-labelled parameter lands by its uniform; a renamed uniform lands by its label" | MU-EL-3 |
| LK-6 | "LK-6 resolve gives values and Dry/Wet only: bypassed, enabled, the name, both connection arrays and both live twins are untouched" | MU-EL-4 |
| LK-7 | "LK-7 fromVar refuses the whole file: not an object, wrong format, wrong effect, params not an array, an entry that is not an object, a text value, NaN, infinity" | MU-EL-5 |
| LK-8 | "LK-8 values outside 0..1 are clamped, and a look with such a value matches the slot it was loaded on" | MU-EL-25 |
| LK-9 | "LK-9 a slot matches a look within 0.0005; one slider step (0.001) away it does not; a look none of whose entries names a parameter matches nothing" | MU-EL-6 |
| LK-10 | "LK-10 a slot built the way the add sites build it matches the Default look, for every registered effect" | MU-EL-26 |
| LK-11 | "LK-11 every registered effect name is its own folder name and no two are equal without case; every def's uniforms and labels are distinct" | MU-EL-7 |
| LK-12 | "LK-12 a float survives the JSON text bit for bit (0.12345f, 1/3, every 0.001 step)" | MU-EL-27 |
| LK-13 | "LK-13 no effect name that has shipped is missing from the library" (a pinned list of today's display names must be a subset of the registered names; the failure text: "an effect was renamed or removed: its saved looks and every saved show that names it are orphaned -- restore the name, or add a folder alias before this ships") | MU-EL-28 |
| LK-14 | "LK-14 a connected parameter whose live twin drifts still matches its look, and capture stores the base value" | MU-EL-24 |
| LK-15 | "LK-15 a look name is legal when it is 1 to 40 characters, has none of the nine file characters or a control character, does not start with a dot, and is not Default, Looks, New Look, Rename or Delete in any letter case" | MU-EL-29 |

5.2 tests/test_look_store.cpp, tag [lookstore] (S1; 12 rows; a temp folder per case)
| id | exact test name | RED arm |
|---|---|---|
| ST-1 | "ST-1 make puts one file on disk before it returns; a second store on the same folder lists it with bit-equal values" | MU-EL-8 |
| ST-2 | "ST-2 make names by itself: Look 1, Look 2; a freed number is used again; an existing file is never overwritten" | MU-EL-30 |
| ST-3 | "ST-3 bad files beside good ones: the good ones list; the empty, the half-written, the wrong-effect and the binary file and a folder named like a look are not listed, and their bytes are unchanged after make, rename and remove" (the fixture names bad files so that they sort both before and after the good ones, and the row prints the order read) | MU-EL-9 |
| ST-4 | "ST-4 an operation on one effect reads and writes no other effect's folder" | MU-EL-31 |
| ST-5 | "ST-5 a make that cannot write gives no look: no new file, the list unchanged; a later make on a writable folder works" (a real read-only folder, and a writer hook) | MU-EL-10 |
| ST-6 | "ST-6 a written file is read back through the listing's own reader before it is listed; a truncated write leaves no file" | MU-EL-11 |
| ST-7 | "ST-7 rename renames the file and changes no byte of it; an empty, a taken (without case), a reserved, an illegal or a 41-character name is refused and nothing changes; a change of letter case alone is allowed" | MU-EL-32 |
| ST-8 | "ST-8 remove deletes exactly that look's file; the effect's other looks and its folder stay" | MU-EL-33 |
| ST-9 | "ST-9 the constructor reads nothing; looksFor reads a folder once for the whole run" (arm 1: dirReads is 0 after construction; arm 2: after looksFor of one effect, called three times, it is exactly 1; prints "ST-9 INFO one read of 200 look files took <ms> ms", reported, no bar) | MU-EL-12 |
| ST-10 | "ST-10 a file with unknown keys and a version above 1 lists, and no operation on another look changes its bytes" | MU-EL-34 |
| ST-11 | "ST-11 the test-mode root is never the real folder: an absolute AUDIODNA_LOOKS_DIR is used as given; unset, empty or relative gives a scratch folder under the temp directory" | MU-EL-21 |
| ST-12 | "ST-12 the file is the look: two files with equal contents list as two looks under their file names; remove and rename act on the named file only" (fixture: "Look 1", "Look 1 copy", "Look 10") | MU-EL-35 |

5.3 tests/test_effect_look_cmd.cpp, tag [lookcmd] (S2; 8 rows)
| id | exact test name | RED arm |
|---|---|---|
| LC-1 | "LC-1 a load is one undo step for Global, Layer and Clip: execute sets, undo restores bit-equal, redo sets again" | MU-EL-36, MU-EL-22 |
| LC-2 | "LC-2 a load touches one slot: every other slot's values, connections, grips and engine states are bit-equal, and so are this slot's connections and grips" | MU-EL-13 |
| LC-3 | "LC-3 a slider moved on another effect after the load survives the undo of the load" | MU-EL-13 |
| LC-4 | "LC-4 nothing of the layer strip changes across execute, undo, redo: opacity, bypass, solo, mute, the trigger word, the playing clip, its play state" | MU-EL-14 |
| LC-5 | "LC-5 whole or not at all: a stale coordinate, another effect at that index, or another parameter count writes nothing" | MU-EL-15 |
| LC-6 | "LC-6 the fence runs once per execute, undo, redo and the write happens inside it; UndoManager::perform of the command fences once" | MU-EL-16 |
| LC-7 | "LC-7 a connected parameter keeps its connection, its grip and its live twin; after one engine tick its effective value is the signal's and the look's value is the base underneath" | MU-EL-37 |
| LC-8 | "LC-8 the undo text is Load Look '<look>' on '<effect>'" | MU-EL-38 |

5.4 tests/test_effect_looks_menu.cpp, tag [looksmenu] (S3; 15 rows; links the stack view as tests/test_effect_stack_binding.cpp
does, tests/CMakeLists.txt:2196-2233; handlers are called directly, never synthetic OS input)
| id | exact test name | RED arm |
|---|---|---|
| LM-1 | "LM-1 the menu lists Default, his looks in natural order, New Look, Rename, Delete; at most one item is ticked, the first look the settings match" | MU-EL-40 |
| LM-2 | "LM-2 Rename and Delete each list every one of his looks and never Default; two looks with equal values and a look equal to Default can each be renamed and deleted; with no looks both are greyed" | MU-EL-41 |
| LM-3 | "LM-3 the button shows the matched look's name; one slider step away it shows Looks; back on the value it shows the name" | MU-EL-17 |
| LM-4 | "LM-4 New Look is enabled only when the settings match no look, Default included; after New Look the button reads the new look's name; a make that fails changes nothing" | MU-EL-42 |
| LM-5 | "LM-5 an effect the library does not know has no looks button" | MU-EL-43 |
| LM-6 | "LM-6 a menu answer that arrives after the row was rebuilt onto another effect does nothing" | MU-EL-18 |
| LM-7 | "LM-7 a pick pushes exactly one PerformLook with the row's own scope and effect index and keeps the row unfolded; a pick that would change no value pushes none; with no host a pick writes nothing" | MU-EL-23, MU-EL-44 |
| LM-8 | "LM-8 the looks button never takes the keyboard: it does not want focus and a click does not grab it; every close of the menu, the Rename window and the Delete confirm calls the focus-home hook once" | MU-EL-19 |
| LM-9 | "LM-9 a drop still lands: fx:Echo,Ripple appends two effects with one PerformEdit; the looks button is not a drop target" | MU-EL-45 |
| LM-10 | "LM-10 the button asks for a repaint only when what it paints changes: 100 refreshes with unchanged values ask for none" | MU-EL-20 |
| LM-11 | "LM-11 at header widths 400, 300, 220 and 160 the B button, the name, the looks button and X do not overlap; under 56 px the button shows no text" | MU-EL-46 |
| LM-12 | "LM-12 a connected parameter whose live value drifts keeps the button on its look" | MU-EL-24 |
| LM-13 | "LM-13 a re-point to the same chain keeps each row's fold state where the effect at that index is unchanged; another chain, or another effect at that index, folds" | MU-EL-47 |
| LM-14 | "LM-14 every registered effect dropped on a stack reads Default on its button" (through `EffectStackView::itemDropped`) | MU-EL-26 |
| LM-15 | "LM-15 the name filter drops the characters a look name may not hold and stops at 40" | MU-EL-48 |

5.5 The mutants (the change; the rows it must turn red) and the lints
MU-EL-1 `toVar` writes "params" as a bare array of numbers (LK-1). MU-EL-2 `resolve` matches by index (LK-2, LK-3, LK-4; GL-2).
MU-EL-3 the label fallback removed (LK-5). MU-EL-4 the look carries and applies `bypassed` (LK-6). MU-EL-5 a non-finite value is
accepted (LK-7). MU-EL-6 the tolerance is 0.01 (LK-9). MU-EL-7 `folderNameFor` keeps only the first 4 characters (LK-11).
MU-EL-8 `make` only fills the list, no file (ST-1; GL-1). MU-EL-9 the listing stops at the first bad file (ST-3). MU-EL-10 a
failed write still lists the look (ST-5). MU-EL-11 the read-back removed (ST-6). MU-EL-12 the constructor scans the root (ST-9).
MU-EL-13 the load is pushed as a whole-chain `EffectStackCmd` (LC-2, LC-3; GL-3). MU-EL-14 `apply` also writes the layer's
opacity (LC-4; GL-3). MU-EL-15 the effect-name check removed (LC-5). MU-EL-16 the write is done before the fence call (LC-6).
MU-EL-17 the button remembers the last loaded name (LM-3). MU-EL-18 the menu's re-check removed (LM-6). MU-EL-19 the focus-home
call removed from the menu's callback (LM-8). MU-EL-20 `setState` always repaints (LM-10). MU-EL-21 `testModeRoot` returns the
real folder (ST-11) -- UNIT ONLY: an app built with it is never launched, it would write into his real folder. MU-EL-22 the
command resolves every scope as Clip (LC-1; GL-2). MU-EL-23 the view passes effect index 0 (LM-7; GL-5). MU-EL-24 capture /
matches read `effParam` (LK-14, LM-12). MU-EL-25 the clamp removed (LK-8). MU-EL-26 Default is built with Dry/Wet 0.5 (LK-10,
LM-14). MU-EL-27 `toVar` writes 3 decimal places (LK-12). MU-EL-28 the library registers "Invert" as "Invert 2" (LK-13).
MU-EL-29 the reserved-name test is case-sensitive (LK-15). MU-EL-30 `make` writes over an existing file (ST-2). MU-EL-31
`looksFor` scans the root (ST-4). MU-EL-32 `rename` rewrites the file from the parsed look (ST-7). MU-EL-33 `remove` deletes
the folder (ST-8). MU-EL-34 the listing rewrites a file it read (ST-10). MU-EL-35 `remove` matches the name by prefix (ST-12).
MU-EL-36 undo restores the parameters but not Dry/Wet (LC-1). MU-EL-37 `apply` touches each connection's grip (LC-7). MU-EL-38
the undo text omits the effect name (LC-8). MU-EL-40 New Look is listed above the looks (LM-1). MU-EL-41 Rename and Delete act
on the ticked look only (LM-2). MU-EL-42 New Look is always enabled (LM-4). MU-EL-43 the button is built for every row (LM-5).
MU-EL-44 the view calls `rebuildRows` after a load (LM-7). MU-EL-45 `LooksButton` is a `DragAndDropTarget` (LM-9). MU-EL-46 a
fixed 110 px button (LM-11). MU-EL-47 the fold state is kept by index alone (LM-13). MU-EL-48 the filter's length cap removed
(LM-15). (39 is unused.)

Lints (static; run by the builder, re-run by Harmony; each shown RED on a seeded line)
- LINT-EL-1 "the Looks folder is spelled once: `grep -rn 'getChildFile *(\"Looks\")' src` prints exactly one line, in
  src/effects/EffectLookStore.cpp; EffectLookStore.cpp names no replaceWithText, replaceWithData, replaceFileIn, TemporaryFile;
  findChildFiles / RangedDirectoryIterator / DirectoryIterator appear in the new files only inside `listFolder`".
- LINT-EL-2 "EffectStackView.cpp opens menus only with showMenuAsync and withParentComponent".
- LINT-EL-3 "src/core/EffectLookCmd.h names no paramLive, dryWetLive, paramConns, dryWetConn, runtime(, bypassed, enabled, opacity".
- LINT-EL-4 "the new files name no std::mutex, lock_guard, unique_lock; nothing under src/audio or src/analysis includes them".
- LINT-EL-5 "the lane's diff against its base touches no file named ui/PresetManager.* or tests/test_preset_manager.cpp and no
  line naming savePresetButton_, loadPresetButton_, fastSaveButton_, presetSlots_, fastSave, loadSlotPreset".
- LINT-EL-6 "the lane's diff adds no setFileLabel, showMessageBox, showMessageBoxAsync, NativeMessageBox or AlertWindow::show
  call, and no std::cerr outside `effectLooksDir`".

5.6 Harmony's live rows (.harmony/probe-effect-looks.sh). Rig, every row: a test-server build; `open -g <App> --args
--test-mode`; AUDIODNA_LOOKS_DIR = a FRESH scratch folder per row, passed the way AUDIODNA_SETTINGS_FILE is passed today; the
live lock; no Output window; no full-screen capture; no synthetic input; HTTP with Connection: close; quits only its own pid
(.harmony/probe-quit-ours.sh). A mutant arm is a normal cmake build of the mutant (never a copied or re-signed bundle).

The fixtures, pre-registered (the probe's header names the files; S2 commits them before Harmony's first run)
- Show A: layer 0, column 0 holds one still picture (an image file; no video, no source); its clip effects are Kaleidoscope and
  Ripple; layer 0's effects are Hue Shift (amount 0.30), Hue Shift (amount 0.55) and Ripple; one global effect, Ripple.
- Show B: layer 0, column 0 holds the same still picture, PLAYING; its clip effects are Invert (amount 0.70) and Pixelate (size
  0.30); layer 0's effect is Hue Shift (amount 0.30); layer 1's effect is Kaleidoscope. No time-driven effect, so two captures
  of an unchanged state differ only by noise.
- Hand-written looks the probe puts in the scratch folder before a launch: Ripple / "Hand": Dry/Wet 0.4 and three entries in
  THIS order: speed 0.9; an entry named u_ripple_gone / "gone" 0.77; intensity 0.1 (no entry for freq). Invert / "Off": amount
  0.0. Hue Shift / "Far": amount 0.8.
- A picture difference is the mean absolute difference over all pixels and the three colour channels, 0..255, of two
  `POST /api/render_frame` captures, pixel-decoded. noise = that difference between two captures of an unchanged state.
  bar = max(1.5, 4 x noise).
- "strip" = for layer 0: the playing clip (deck id and column), its play state, opacity, bypass, solo, mute -- read through
  GET /api/composition once, after a GET /api/debug/looks has answered.

| row | does | bar (the exact line printed on pass) | RED arm |
|---|---|---|---|
| GL-1 | Show A. Set the clip's Kaleidoscope to segments 0.61, rotation 0.27 (POST /api/set_param); look_make on it; GET looks; read the file. Quit own pid. Relaunch on show B; GET looks for layer 1's Kaleidoscope; look_load "Look 1" there. | "GL-1 PASS Look 1 on disk at make (segments 0.6100 rotation 0.2700); listed and loaded on a layer in another show after a relaunch; windows unchanged; file label unchanged" (windows counted with Quartz for the own pid before and after look_make; the label through /api/debug/ui_text) | MU-EL-8 |
| GL-2 | Show A with "Hand" pre-written. look_load "Hand" on the clip's Ripple, on layer 0's Ripple and on the global Ripple; GET looks after each. | "GL-2 PASS 3/3 hosts hold the hand-written look by name: intensity 0.1000 speed 0.9000 freq kept 0.5000 dryWet 0.4000" (each within 0.0005) | MU-EL-2 (the names land on the wrong sliders); MU-EL-22 (1/3 hosts) |
| GL-3 | Show B with "Off" and "Far" pre-written. Strip S0 and two pictures (noise, bar). (a) look_load "Off" on the clip's Invert: strip, picture. (b) /api/debug/undo: strip, picture. (c) look_load "Off" again; POST /api/set_param Pixelate size 0.6; /api/debug/undo: values, strip. (d) a picture; look_load "Far" on layer 0's Hue Shift: strip, picture. (e) /api/debug/undo: strip, picture. | "GL-3 PASS strip equal 5/5; clip look changed <d> back <d>; layer look changed <d> back <d>; other effect kept 0.6000; noise <n> bar <b>" (changed = above the bar against the picture taken just before that load; back = at or below the bar against that same picture) | MU-EL-14 (strip); MU-EL-13 (the other effect falls back to 0.3000) |
| GL-4 | THE FENCE MEASUREMENT, table below | the table's line | the control arm in the table |
| GL-5 | Show A with "Far" pre-written, after S3. look_ui on layer 0, fx 1 (the SECOND Hue Shift): pick:New Look; GET looks; read the file. Then pick:Far on fx 1; GET looks for fx 0 and fx 1; GET looks?stats=1. | "GL-5 PASS menu pick on layer fx 1: New Look stored 0.5500; Far set fx 1 to 0.8000; fx 0 kept 0.3000; focus home +2" | MU-EL-23 (New Look stores 0.3000) |
| GL-6 | THE TAKE MEASUREMENT, table below | the table's line | the control arm in the table |
| GL-7 | A listing (names, sizes, modification times; "absent" if it does not exist) of ~/Library/Audio-DNA/Looks/ ONLY, before the probe's first launch and after its last quit; plus one launch in test mode WITHOUT AUDIODNA_LOOKS_DIR and one look_make in it. | "GL-7 PASS real Looks folder unchanged (<absent or n files>); scratch folder named on stderr; the look went there" | the probe's selftest: the comparer is fed two listings that differ by one file and must print FAIL. MU-EL-21 is proved in the unit row ST-11 and is never launched. |
| GL-8 | full ctest at the lane's final head | "GL-8 PASS ctest <N> = baseline <N0> + 50 new cases, 0 failed" (N0 measured by Harmony with `ctest -N` on the lane's base before S1; 50 = 15 + 12 + 8 + 15) | the lane's base: N0 is not N0 + 50 |
Regression, unchanged and green: .harmony/probe-effects-parity.sh, .harmony/probe-deck-path.sh, the gate A set.

GL-4, pre-registered decision table (a measurement). Show B playing, "Off" pre-written. Read fence_hold_frames and
fence_black_frames (GET /api/state). Control: 2 s with no load, read again. Then 20 look_loads on the clip's Invert, "Off" and
"Default" in turn, 100 ms apart; after the last GET looks has answered, read again.
| read | ruling |
|---|---|
| control: either counter moved with no load | the instrument is wrong: no conclusion, fix the probe |
| the 20 loads did not alternate `matched` between "Off" and "Default" | the instrument is wrong: no conclusion |
| black moved by more than 0 | STOP and report: a fenced frame found no canvas (Pitfall 55 says this stays 0). S3 does not start. |
| black 0, hold at most 40 (2 per load) | the fence stays as ruled; section 6 item 3 stands as written |
| black 0, hold above 40 | the fence stays for now; Harmony decides HD-8 with the number; section 6 item 3 names the number |
Printed line: "GL-4 MEASURED loads=20 hold=<n> per_load=<x.xx> black=<n> control=<n>/<n>".

GL-6, pre-registered decision table (a measurement, not a verdict on the code). Show B, "Off" and "Far" pre-written. Arm a take
as .harmony/probe-routines.sh does (POST /api/perf/record with a probe-chosen name and "audio": false; the take folder is under
~/Documents/Audio-DNA/Takes, T2, and the probe prints its path and leaves it). During it, 1 s apart: (1) look_load "Off" on the
clip's Invert; (2) look_load "Far" on layer 0's Hue Shift; (3) the control: POST /api/set_param Pixelate size 0.6.
POST /api/perf/stop. Read take.json once.
| read | predicted (T1) | if it differs |
|---|---|---|
| lanes that name the control parameter | 1 or more | the instrument is wrong: STOP, fix the probe, no conclusion |
| lanes that name any parameter or Dry / Wet of the two looked effects | 0 | STOP and report: a look load IS recorded as moves. AM-20's paragraph and section 6 item 10 are re-written from the measurement, and Harmony checks what the take does on playback before S3 starts. |
| the state stored at Stop: the clip's Invert amount, the layer's Hue Shift amount | 0.0 and 0.8 (the looks' values) | STOP and report: the take's end state does not hold what was on screen. |
Printed line: "GL-6 MEASURED control lanes=<n> look lanes=<n> stop-state=<equal|differs>".
If it measures as predicted: section 6 item 10 is told to Boris as written and HD-6 files the gap.

5.7 The visual gate: 18 states (window-id captures only; each with a manifest of model facts: the effect, its base values, the
matched look, the button's text / dim / mode, the header's and the button's width in px, the menu model). Every state at
1280 x 720 unless said.
V-1 clip inspector, an effect just added, folded: the button reads "Default". V-2 unfolded, one of his looks matched: its name.
V-3 one slider moved: "Looks", dim. V-4 the menu on a fresh effect, no looks of his: Default ticked; New Look, Rename, Delete
greyed (the first menu he will see). V-5 the menu with three looks, one ticked: New Look greyed; Rename and Delete live.
V-6 the menu with nothing matched: nothing ticked, New Look live. V-7 the Rename list by itself. V-8 the Delete list by itself,
in red. V-9 the Rename window. V-10 the Delete confirm. V-11 the layer inspector's stack. V-12 the composition inspector's
global effects. V-13 the inspector at its narrowest: the triangle alone, the effect name with an ellipsis. V-14 a 40-character
look name, on the button and in the menu. V-15 two instances of one effect on different looks, the second bypassed.
V-16 thirty looks: the menu. V-17 an effect with a signal on one parameter, a look loaded (manifest: the button's text, that
parameter's base value and live value). V-18 V-1 and V-5 again at 1920 x 1080.
The critics are told these pre-date the lane: "B" and "X", the header's colours, the sliver of the folded first value left of
X and the hidden fold arrow (E1), the purple drop highlight inside a stack, the parameter rows, the inspector's tabs.
NOT capturable without synthetic input, said so: a sub-list hanging off its parent item (V-7 and V-8 show the list alone); the
drop highlight over a stack (unit LM-9 only); hover colours.

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; replaces the plan's section 6)
---------------------------------------------------------------------------------------------------------
1. Put an effect on a clip. Its header has a small button that reads "Default". Move a slider: it reads "Looks", dim. Press it
   and choose New Look -> the button reads "Look 1" -> wrong: a box asks for something, a message appears, or the button still
   reads "Looks".
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Look 1" is there;
   choosing it sets the sliders as you left them -> wrong: the look is missing, or it is there only in the first show.
3. With a clip playing on that layer, load a look -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move. For one frame the screens hold the picture they had, as they do when you add
   an effect -> wrong: the layer restarts, the picture goes black, or anything in the strip changes.
4. Press Cmd+Z -> the effect returns to how it was just before the look and stays open; the strip does not move; a slider you
   moved on ANOTHER effect meanwhile stays where you put it. (A slider you moved on the SAME effect after the look goes back
   too: undo puts that effect back to before the look.) -> wrong: the other effect's slider jumps back, or the effect folds shut.
5. The name on the button. After loading "Look 2", move one slider -> the button reads "Looks" (dim); move it back to the exact
   value -> the name returns. Tell us if you would rather the name stayed with a mark, or if you want no name there at all:
   both are small changes.
6. Load a look on an effect that has a signal driving one slider -> that slider keeps following the signal; the others take the
   look (question 102).
7. Open the menu and choose Rename -> a list of your looks; pick one -> a small window; type a name, press Return. Choose
   Delete -> a list in red; pick one -> a window asks; Delete. You never have to load a look to rename or delete it, and deleting
   never moves a slider -> wrong: a look you cannot rename or delete; a name that changes by itself.
8. New Look is grey when the effect's settings already are one of your looks, or Default: there is nothing new to keep. Tell us
   if that puzzles you.
9. After the menu or one of its two windows closes, your keys work as before (Return, your clip keys) -> wrong: Return opens the
   menu again, or presses a button.
10. Record a take, load a look in the middle, play the take back -> this build does NOT play the look change back. The same is
    true today of an effect slider you move with the mouse. You said a take records "all the parameter movements": recording
    these is its own build, and it is on the list.
11. Where your looks live: on this computer, in the folder Library / Audio-DNA / Looks inside your home folder, one folder per
    effect, one small file per look. A show you open on another computer still looks the same -- each effect keeps its settings
    in the show -- but the Looks menus there list only that computer's looks. To take your looks along, or to back them up,
    copy that folder.
12. If the disk is full or that folder is locked, New Look does nothing: the button keeps reading "Looks". No message.
13. By eye on your own screens: the button's size, the dim "Looks", the menu's length with many looks, the red Delete list.
14. When the old buttons go (the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone, and the
    four old files in ~/Library/AudioDNA/Presets/ can no longer be opened from the app. They stay on the disk, untouched.

---------------------------------------------------------------------------------------------------------
## 7 BORIS QUESTIONS (numbers 101..109; each has a default A; nothing waits)
---------------------------------------------------------------------------------------------------------
101. "every effect has many looks": should the app come with looks already made for its effects?
     A (default) No. Each effect starts with "Default" and the looks you make.
     B Yes: the app also brings its own looks for each effect (they have to be designed; a later build).
102. You have a signal driving one of an effect's sliders, and you make a look of that effect.
     A (default) The look keeps the slider values and Dry / Wet. Signals stay as they are on each effect: loading a look never
       plugs or unplugs one.
     B The look also remembers which signal drives each slider, and loading it plugs them in again (a later build).
103. You have loaded "Look 2" and changed it. Do you want to save the change back into "Look 2"?
     A (default) No. New settings are always kept as a new look; you can delete the old one.
     B Yes: the menu also offers "Save over Look 2".
104. When you make a new look:
     A (default) It names itself ("Look 1", "Look 2") with one click, and you rename it later if you want.
     B A name box opens every time.
105. WITHDRAWN, not asked. The plan asked whether a look change is played back by a take. His words already say what a take
     records (W5); section 6 item 10 tells him what this build does; HD-6 files the gap.
106. Asked ONLY if HD-2 finds, in his four old preset files, a setup that differs from the defaults. The text then:
     Your four old effect presets (their names) hold settings for (the effects). The old Load button that opened them is going.
     A (default) Leave them. They stay on the disk; the app no longer opens them.
     B Turn them into looks once: each effect in a preset becomes a look of that effect, named after the preset (a small build).
107-109 are not used.

What changes with each answer (so that either is a small change)
- 101 B: `EffectLookStore::looksFor` also lists a read-only folder inside the app bundle; same format; the looks themselves
  have to be designed.
- 102 B: one optional key per "params" entry (the connection, in the show's own JSON shape) and `EffectLookCmd` grows to carry
  it. Old look files keep working unchanged. A later build with its own plan: it re-opens "never a connection".
- 103 B: one menu item and one store function that swaps a temp file in with `replaceFileIn` (AM-4).
- 104 B: the New Look action opens the Rename window first; one branch in the view. Rename stays either way.

---------------------------------------------------------------------------------------------------------
## 8 HARMONY'S DECISIONS (each has a default)
---------------------------------------------------------------------------------------------------------
- HD-1 The old Save, Load, FX Save and the ten slots are removed by the one-save lane's S7, exactly once, and S7 is cut loose:
  it runs and merges FIRST, before one-save S1-S5 and before this lane's S3, because Boris took "Remove ... now" (W2).
  One-save S4b then has nothing left to do in ui/PresetManager.* and tests/test_preset_manager.cpp, and S7's one visual state
  (V16) goes through the visual gate before he sees it. Default: yes. If ruling-one-save.md names another owner, Harmony picks
  ONE. If she moves the removal into this lane, it
  becomes a stage S0 with the one-save plan's S7 scope, its LINT-6 and its V16, and LINT-EL-5 is dropped.
- HD-2 Before S7 merges, Harmony reads his four old preset files, read-only: their names and dates, and per file which effects
  are enabled and which parameters differ from the library's defaults. She files what they hold. Default: yes. If nothing
  differs from the defaults, section 6 item 14 gains "they hold no settings of yours". If something does, question 106 is
  asked; nothing waits for the answer, because the files are never deleted.
- HD-3 S7 moves T6 / T7 of tests/test_preset_manager.cpp into a library test instead of deleting them. Default: move. This
  lane does not depend on it: LK-11 re-pins what it leans on.
- HD-4 The number of the new pitfall. Default: the next free number at merge that is not 68.
- HD-5 WITHDRAWN: the right-click path is not built (SC-3).
- HD-6 The take gap: effect sliders moved with the mouse, and look loads, are not recorded as moves (T1), against W5.
  Default: tell Boris as section 6 item 10 does, file it in the handoff ledger as its own lane, ask nothing now. If GL-6
  measures otherwise, its table rules.
- HD-7 The folded first value and the fold arrow drawn under X (E1). Default: leave them and name them to the critics. If a
  critic still fails the lane on the sliver, the fix round deletes the folded-value draw (EffectStackView.cpp:50-58) and
  nothing else.
- HD-8 If GL-4 measures more than 2 held frames per load. Default: keep the fence and tell Boris the number. The alternative
  is one line (the command writes without the fence, as a slider does) and gives up "never half a look in a frame".
- HD-9 AM-10's two side effects (undo of an add, remove or bypass keeps the surviving rows as they were; a second click on the
  selected clip no longer folds its effects). Default: accept.
- HD-10 Live mutant arms may share a build where they cannot mask each other. Default: three builds: M-A = MU-EL-2 + MU-EL-14
  (GL-2 by name; GL-3 strip), M-B = MU-EL-8 + MU-EL-22 (GL-1; GL-2 hosts), M-C = MU-EL-13 + MU-EL-23 (GL-3 undo; GL-5). A row
  counts as RED only when its FAIL line names the check its own mutant predicts: GL-1 "no file at make"; GL-2 the wrong
  sliders (M-A) or "1/3 hosts" (M-B); GL-3 "strip differs" (M-A) or "other effect 0.3000" (M-C); GL-5 "New Look stored 0.3000".
- HD-11 "B" and "X" park the keyboard today (E7, SF-4). Default: file it in the ledger; not changed in this lane.
- HD-12 Order. S1 shares no file with any other lane except CMakeLists.txt and tests/CMakeLists.txt (append). S2 and S3 edit
  MainComponent.cpp and ApiServer.cpp: whichever lane merges second re-bases as a builder's step 0. Default: start S1 now.

---------------------------------------------------------------------------------------------------------
## 9 SIDE FINDINGS (found, not fixed)
---------------------------------------------------------------------------------------------------------
- SF-1 The hidden legacy chain still renders after the deck compositor and is still driven by randomise, OSC and REST (FOS Q6,
  SHEET). After S7 nothing on screen loads anything into it. Recommendation: its own small lane after S7.
- SF-2 An effect slider moved in an inspector is not recorded by a take (T1, E5): `onParamChanged` and its siblings are
  declared and never assigned (EffectStackView.h:81-85). Against W5. HD-6.
- SF-3 The folded first value and the fold arrow are drawn under X (E1): the arrow is invisible today, so a row shows no sign
  of being folded or open except its sliders.
- SF-4 "B" and "X" park the keyboard (E7): after a click on "B", Return toggles bypass again, and each toggle is an undo step.
- SF-5 `EffectStackView::refresh()` ends with an unconditional `repaint()`, about 10 times a second while an inspector shows a
  stack (cpp:218): the Pitfall 57 / 59 class, older than this lane.
- SF-6 A "B" click fences twice (E6): the view's own fenced toggle, then the command's execute through `perform`.
- SF-7 Stale words: Clip.h:62 ("Registry name (e.g., "ripple")"); EffectLibrary.h:11 and :35 ("17 effects");
  core/EffectCommands.h's class comment that global effects are not read on the GL side (FEL section 3 item 11, SHEET);
  docs/claude/architecture.md "UndoManager DEAD" (FEL section 3 item 12, SHEET).
- SF-8 GET /api/effects lists the legacy chain (A3): a probe that maps names through it leans on the hidden chain.
- SF-9 The plan named POST /api/set_clip_param for an effect parameter (A2). Any gate string copied from the plan's section 5
  inherits the wrong route: copy only from this ruling's section 5.
- SF-10 `takesRoot()` has no test-mode seam (T2): every take probe writes into ~/Documents/Audio-DNA/Takes; GL-6 does too.
- SF-11 plan-one-save.md:522 still says question 86 has no answer (it has: W2), and its S7 is last in its order (:549), which
  does not fit "now".
- SF-12 The undo history holds 100 steps (U1); every look load is one of them.
- SF-13 `juce::File::createLegalFileName` drops # @ , ; ^ (L3): any code that uses it on a name the user typed changes the
  name silently.

NOT IN THIS LANE
- The removal of the small Save, Load, FX Save, the ten slots, PresetManager and its tests (one-save S7). The hidden legacy
  chain (SF-1). A conversion of his four old presets (106 B).
- Looks for procedural sources (the plan's EL5 stays as a note). Looks that carry connections, bypass or several effects
  (102 B). Looks that ship with the app (101 B). "Save over" (103 B). A name box at creation (104 B).
- Recording a look load, or a mouse-moved effect slider, into a take (HD-6). Parameter edits as undo steps, and the structural
  undo that overwrites later slider edits (the undo-live lane).
- Loading a look from a key, MIDI, OSC or production REST. Copy and paste of an effect. Re-ordering effects. A glide between
  two looks. Randomise on an effect. A "Show Looks Folder" item.
- A look name remembered on the slot or in the show. Any change to the show file.
- "B" and "X" focus (HD-11); the folded value and arrow under X (HD-7); ISF-imported effects (no button).
- The stopped sync-dial branches: nothing is carried from them and nothing of them is dropped by this lane (PL F29).

---------------------------------------------------------------------------------------------------------
## 10 RISKS (the strongest counterargument first)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "a look without its signals is half a look". Audio-DNA is audio-reactive: the wire from the
   bass to "Amount", with its range, is much of how an effect looks in motion. A look that brings only slider positions makes
   him plug every signal in again on every clip. Why the ruling still stands: his words are "parameter setups" and he was told
   "keep the current settings"; a look with wires would replace wiring on a playing layer and reset each wire's grip and memory
   (ParamConnection.h:150-160); a wire names a signal by name or a macro by index, and neither user signals nor the macro bank
   are saved anywhere (FOS M-6), so a look kept for every show could point at nothing, with no text allowed to say so. What
   makes it cheap to be wrong: question 102 asks him, and B is one added optional key; the looks he has made by then keep
   working. Cheapest refuting test: section 6 item 6, in front of him.
R2 "The button that forgets", and its mirror, SC-4's "no name at all". After one slider move the name of the look he loaded is
   gone. The ruling keeps the derived name because it cannot be stale and because, under W4, it is the only sign that New Look
   worked. If he wants the name to stay with a mark, that is one optional key on the slot, saved in the show: the show format's
   first change from this feature, and a follow-up. Section 6 item 5 asks his eye.
R3 Two rulings, one removal. ruling-one-save.md was a skeleton when this was written. If it also gives the removal away,
   nobody removes the buttons; if both took it, twice. HD-1 is Harmony's to settle; LINT-EL-5 here and LINT-6 there are the
   machine check that it happened once.
R4 The fence. Every load holds the screens for the frames GL-4 counts (one or two are expected) and a recording misses them.
   Fast picking of looks in a set holds that many frames. HD-8 holds the alternative.
R5 The direct write. A crash during the write leaves a file that does not parse: hidden, never deleted, its name not given out
   again, and a rename onto it refused without a word. Accepted: a temp-and-swap leaves an equally invisible temp file and
   needs a sweep. `flush()` fsyncs (J3), yet after a power cut seconds after New Look the look can still be missing; no save
   in the app is stronger than this.
R6 New Look greyed while the settings already are a look may puzzle him (section 6 item 8). Allowing twins is one line.
R7 A refused name is refused without a word (AM-24): the window closes and the old name stays. The filter removes the common
   case. No text is allowed (W4).
R8 The undo history holds 100 steps: fifty look picks push fifty older steps out. Accepted: no merging, so that he can step
   back through looks one at a time.
R9 Matching by name cannot see a changed MEANING under an unchanged uniform name (DA-6): the value loads and looks different.
   The same is already true of every saved show. The pitfall text tells the next builder to rename the uniform.
R10 JUCE's JSON number text is ASSUMED to carry a float exactly. LK-12 is the refuting test and runs first in S1; AM-3 rules
   both outcomes.
R11 AM-10 changes what a second click on the selected clip does (HD-9).
R12 Undo of a look load also takes back sliders he moved on THAT effect after the load (one slot wide; section 6 item 4 says
   so). Other effects are untouched (LC-3, GL-3).
R13 Looks live per computer (section 6 item 11). A drive failure or a new machine empties every menu; the shows still look the
   same.
R14 The take: T1 predicts that a look load is invisible to a take's playback. GL-6 measures it with a stop rule.
R15 A look file added in the Finder while the app runs appears only at the next launch (AM-2).
R16 The Rename and Delete lists cannot be captured hanging off their parent item (5.7): only Boris's eye sees them as he will
   use them.
R17 Merge conflicts in MainComponent.cpp, ApiServer.cpp and tests/CMakeLists.txt with one-save, transport, nudge and outputs:
   a builder's step 0, never Harmony's (RIG-RULES A2).

STATUS: DONE
