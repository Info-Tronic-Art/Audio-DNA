# SEAT PAPERS -- lane "effect-looks" (blind council; 4 of 4 seats returned)

Written verbatim by the architect before ruling (s-rta-1004). Source: the dispatch text, 38419 characters of JSON, parsed and pretty-printed; no word changed. The plan attacked: .harmony/.reports/s-rta-1004/plan-effect-looks.md. The ruling: .harmony/.reports/s-rta-1004/ruling-effect-looks.md.

Attack ids by seat: data-safety = DA-1, DA-2, DA-3, DA-4, DA-5, DA-6, DA-7, DA-8; gates = GA-1, GA-2, GA-3, GA-4, GA-5, GA-6, GA-7, GA-8; stage-hands = ST-1, ST-2, ST-3, ST-4, ST-5, ST-6, ST-7; scope (MINIMALIST) = SC-1, SC-2, SC-3, SC-4, SC-5, SC-6, SC-7, SC-8

---------------------------------------------------------------------------------------------------------
## SEAT: data-safety
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "data-safety",
  "attacks": [
    {
      "id": "DA-1",
      "target": "EL3 the button's derived name + the menu's Rename/Delete enabling",
      "claim": "A look can become impossible to rename or delete from the app. The button shows the FIRST matching look (Default first, then his by name), and Rename/Delete work only on the MATCHED look. So any look whose values equal Default or an earlier look is unreachable.",
      "evidence": "plan-effect-looks.md:225-227 (first look the slot matches; order Default then his looks by name) and :238-239 (Rename and Delete enabled only while one of HIS looks is the matched one). The plan's own flow makes this: press New Look twice with no slider move gives Look 1 and Look 2 with equal values, and Look 2 can never be the matched one. The same holds for New Look on an untouched effect, which equals Default. A look file with a value outside 0..1 (clamped by resolve, :154) may never match either. LM-2/LM-3 (:443-445) test only a single matched look.",
      "severity": "MUST",
      "proposed_change": "Make Rename/Delete act on a look chosen from the menu list itself (a per-item submenu, or rows that are ticked and acted on), not on the derived match. Add a test: two looks with equal values, and one equal to Default. Each must be deletable and renamable, and the unreachable case (an out-of-range value in a file) must be listed."
    },
    {
      "id": "DA-2",
      "target": "EL2 make() naming + the not-on-disk state",
      "claim": "In the failure state the plan designs for (look not written), two looks can end up with one name, and one is lost silently when the retry runs.",
      "evidence": "plan-effect-looks.md:183 ('The file's existence is the collision test') and :244 (smallest free number) against :189-192 (a look that could not be written stays in the menu, cache-only, retried later). A cache-only look has no file, so the next New Look sees 'Look 1' as free, and the cache holds two 'Look 1'. retryPending then writes both to Look 1.look.json (either one clobbers the other, or the second is refused and stays ringed forever). ST-2 (:418) tests only on-disk files and ST-5 (:421) only one failing write.",
      "severity": "MUST",
      "proposed_change": "Make the collision test the union of files on disk and cache entries. Add ST-5b: a read-only folder, New Look twice, give Look 1 and Look 2, then chmod, retry, and both files exist with their own values. RED arm: a mutant that tests only the disk."
    },
    {
      "id": "DA-3",
      "target": "5.6 gate rows G-EL-3 and G-EL-6",
      "claim": "Two live rows have a RED arm that cannot turn the bar red as written.",
      "evidence": "G-EL-3 (plan :469) names MU-EL-2 (resolve by index, :401-404) 'on a fixture whose slot order differs'. Slot order is the effect's position in a chain. MU-EL-2 matters only when the look's parameter order differs from the def's. A look made and loaded by the same build on the same effect has the same order, so index and name give identical values and the row passes under the mutant. G-EL-6 (:472) uses MU-EL-9 (listing stops at the first bad file). That fails only if the garbage file sorts before the good looks, and the plan does not say the fixture does.",
      "severity": "MUST",
      "proposed_change": "G-EL-3: the fixture pre-writes a look file whose params array is re-ordered and carries one extra and one missing entry, so MU-EL-2 gives different values. G-EL-6: name the garbage file so it sorts first, and print the listing order in the row."
    },
    {
      "id": "DA-4",
      "target": "EL2 THE WRITE (writeAtomic)",
      "claim": "The swap step is unnamed, and the obvious JUCE call is less safe than the replaceWithText the plan bans. The plan has no durability step, and a crash leaves a temp file that nothing sweeps.",
      "evidence": "plan :184-188 says 'move it over the target'. JUCE moveFileTo deletes the target first, then renames (build/_deps/juce-src/modules/juce_core/files/juce_File.cpp:300-315, delete at :311), so a crash between the two leaves NO look. replaceFileIn (:323-336) goes through replaceInternal for an existing target. LINT-EL-1 (:457) bans only replaceWithText. The read-back (:186) reads the page cache, not the platter, and there is no fsync, so an OS crash or power loss after the rename can leave a zero-length look that the reader then hides silently (:178). No rule deletes stale temp files after a crash; the plan deletes the temp only on a failed step.",
      "severity": "SHOULD",
      "proposed_change": "Name the primitive: replaceFileIn (or POSIX rename) for an existing target, and add a lint banning moveFileTo/deleteFile-then-write on a look path. fsync (F_FULLFSYNC) the temp before the swap. Sweep '*.tmp' leftovers in an effect's folder at its first read. Add ST-6b: a writer hook that fails after the temp is written, and the old look is intact."
    },
    {
      "id": "DA-5",
      "target": "EL2 file format + rename/remove by name",
      "claim": "A look has two identities, the file name and the inner 'name', with no rule for when they disagree. Rename and delete address a look by name and then derive the file name from it.",
      "evidence": "plan :164-183 (file = fileNameFor(name); 'name' inside the file; a file is refused only for format/effect/params faults, not a name mismatch) and :211 (rename(effect, from, to), remove(effect, name)). Cases: a Finder copy 'Look 1 copy.look.json' holding name 'Look 1' lists two 'Look 1' entries; Delete 'Look 1' then removes the wrong file or none. A Finder rename, a crash between rename's move and rewrite, and 'A/B' vs 'AB' (R6, :567) give the same split. The plan never says which write comes first in rename (new file then delete old, or move then rewrite). ST-7 (:423) tests only clean paths.",
      "severity": "SHOULD",
      "proposed_change": "Make the file the identity: list by file name, carry the path in the cache entry, and have rename/remove act on that path. Rename = write the new file, verify, then delete the old. Ignore the inner name when it disagrees, or refuse the file. Add ST-7b: a duplicate inner name, and an interrupted rename."
    },
    {
      "id": "DA-6",
      "target": "EL2 keying + EL1 'meaning' of a stored value",
      "claim": "A key by display name has no pin against a future rename, and the file records no version of the effect. A deleted or renamed effect's looks stay invisible and cannot be removed. A value whose meaning changed under the same uniform name loads silently as something else.",
      "evidence": "plan :170-173 (folder = display name; the shader key is recorded but 'this lane writes it and does not read it'), :202-203 (folder stays unlisted), LK-11 (:411) pins only that today's 135 names are distinct. Nothing pins the (display, shader) pairs, so a rename in a later PR orphans a folder and no test turns red. The Pitfall 74-style note (:371-374) does not tell the next builder about aliases. Pitfall 45 (docs/claude/pitfalls.md:99) shows the library removes and re-implements params (759 to 681 source params); a changed range under an unchanged uniform name passes resolve (:152-154). The format has only 'version' of the file (:175), not of the app or the effect.",
      "severity": "SHOULD",
      "proposed_change": "Add LK-13: a golden list of the 135 (display, shader) pairs, so any rename fails a test that says 'add an alias'. Add one key 'app' (build version) to the look file now (free), and one sentence in the pitfall text. Say plainly that orphaned looks are not deletable from the app, or list a folder whose effect is unknown, read-only."
    },
    {
      "id": "DA-7",
      "target": "EL6 the old Presets files",
      "claim": "The four old files in ~/Library/AudioDNA/Presets/ are named parameter setups Boris made, by effect and parameter name, which is the format looks use. The plan orphans them without ever reading what they hold. The only mitigation is a sentence to Boris.",
      "evidence": "plan :332 and section 6 item 9 (:521-522): they 'can no longer be opened from the app'. facts-effect-looks.md:125 (the legacy PresetManager JSON is the only by-name parameter-setup store, effect name + params [{name,value}]) and :233 (Presets/ holds 4 old files). Boris's words (binding-decisions.md:894): 'every effect has many looks with specific parameter setups. these are saved with the app. always.' The plan does not check whether these four hold setups for effects that now exist, so 'always' is broken for what he has today.",
      "severity": "SHOULD",
      "proposed_change": "Before S7 removes the picker, read the four files read-only and report the effects they name. Offer a one-shot import (one chain effect = one look, by name) run by Harmony or a builder, or keep the files reachable until Boris rules. Add to section 6 a row: 'your four old presets are in the Looks menus' or 'were judged empty'."
    },
    {
      "id": "DA-8",
      "target": "EL2 WHERE + section 6",
      "claim": "Boris is never told where his years of looks live, and the plan gives no way to carry or back them up. A different machine or a drive failure empties every menu, which is the opposite of 'always'.",
      "evidence": "plan :160-163 (~/Library/Audio-DNA/Looks/) and section 6 (:503-524): none of the ten items names the folder. The show carries no look names (:596, no change to the show file). Boris's own One Save list put 'an effects look' among things saved with the show (binding-decisions.md:873-876, then 'saved with the app' at :894), so he may assume a show copied to a laptop brings its looks. The plan has no 'Show Looks Folder' item or export. Settings (keys, MIDI) got 'import to another show' (:891).",
      "severity": "SHOULD",
      "proposed_change": "Add to section 6 one line naming the folder and saying a show on another machine does not bring its looks, and ask Boris as question 106 whether he wants a menu item that reveals the folder in Finder. The default is yes, since it is one line and no new file format."
    }
  ],
  "strongest_point": "The best structural choice is one file per look in one folder per effect, with refuse-the-whole-file reading. It removes the settings.json trap (AppSettings.cpp:19-40), so one bad file cannot empty the others. The weak points are in the layers above it. The derived button makes a look unreachable for rename and delete when its values equal an earlier one. The failed-write state it designs for can create two same-named looks that clobber each other. Two gate rows have RED arms that cannot fail.",
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
      "target": "5.1-5.4 RED arms 'tree before the change' (LK-8, LK-10, ST-2, ST-8, ST-10, LC-1, LC-8, LM-1, LM-5)",
      "claim": "Nine rows name no mutant. 'Tree before the change' means the file or function does not exist, so it fails at compile and shows nothing about whether the assertion can fail. LK-10 is also a function compared with itself.",
      "evidence": "plan-effect-looks.md:408, 410, 418, 424, 426, 431, 438, 443, 447. The preamble at :395-396 allows 'tree before' only 'where the file does not exist yet', yet LK-8 and LC-1 sit in files whose behaviour a mutant can break. LK-10 (:410) says 'defaultLook is the library defaults': defaultLook(def) is built from def.defaultValue and checked against def.defaultValue. facts-effect-looks.md:204 shows a new slot takes the registry defaults through the real add path, and the plan never checks that path.",
      "severity": "MUST",
      "proposed_change": "Give each row a named mutant: clamp removed (LK-8); Default built with Dry/Wet 0.5 (LK-10); free-number reuse broken (ST-2); remove deletes the folder (ST-8); load rewrites the file (ST-10); undo restores only params, not dryWet (LC-1); wrong text (LC-8); menu order swapped (LM-1); button shown for an unknown effect (LM-5). Rewrite LK-10 so it adds each of the 135 effects through the real add path (as tests/test_composition_tier_oracle.cpp does) and asserts looks::matches(def, freshSlot, defaultLook) is true."
    },
    {
      "id": "GA-2",
      "target": "G-EL-3 and G-EL-2 RED arms",
      "claim": "G-EL-3's RED arm cannot go red live. MU-EL-2 (resolve matches by index) behaves exactly like match-by-name whenever the effect's def is the same one that made the look, and one build has one def.",
      "evidence": "plan:469 'MU-EL-2 on a fixture whose slot order differs'. The slot's paramValues follow the library def's positional order (Clip.h:60-114 as cited at plan:37). A fixture cannot reorder an effect's parameters, and fx-index order in a chain is irrelevant to resolve. Only the unit rows LK-2..LK-4 can go red, because they supply a fake def. G-EL-2's RED arm (:468) is a control ('an empty folder lists none') that proves the probe can see absence; it is not a mutant of the code under test.",
      "severity": "MUST",
      "proposed_change": "Drop MU-EL-2 as G-EL-3's arm and say the live row guards a different defect: the same look landing on Clip, Layer and Global hosts, with a mutant that resolves the chain for the wrong scope (Clip only). For G-EL-2 use a mutant that keeps looks in the in-memory cache or the show, and run the relaunch against it."
    },
    {
      "id": "GA-3",
      "target": "G-EL-4 / G-EL-5 bars",
      "claim": "The picture bars are not tied to a pre-registered effect, look or baseline, and G-EL-5's undo count does not match its baseline.",
      "evidence": "plan:470-471. G-EL-4 loads a look on the clip effect AND on the layer effect. G-EL-5 then runs one /api/debug/undo, which reverts only the last load (the layer's). It still demands 'picture back within max(1.5, 4 x noise)'. The clip effect's look is still applied, so the picture cannot return to the pre-load baseline, and the baseline picture is never named. Which effect, which look values and which image give a change above the bar is left to the run. A temporal effect never returns, because history is kept (plan:286).",
      "severity": "SHOULD",
      "proposed_change": "Pre-register in the row: the effect (non-temporal, e.g. one that visibly changes a flat test card), the look's values, and the image. Do each load with its own undo, or undo twice. Compare against the picture taken before the first load. Print the measured difference, the noise and the bar on the pass line."
    },
    {
      "id": "GA-4",
      "target": "G-EL-9 / ST-9 'zero reads at launch'",
      "claim": "The gate counts only calls inside looksFor, and the fixture is under-specified. A directory scan anywhere else is invisible to the counter. Any stack on screen at launch makes the derived button read its effect's folder.",
      "evidence": "plan:425 and :475. MU-EL-12 'the constructor scans the root' is caught only if that scan bumps dirReads. EL3 (plan:225-227, :233) derives the button text from the values on every refresh(), which needs looksFor(effect) for each effect shown, so a launch whose default composition shows a stack (clip, layer or global effects) reads at launch. 'No clip selected' (:475) does not cover the layer and composition inspectors, which also embed stacks (F7, plan:54).",
      "severity": "SHOULD",
      "proposed_change": "Route every directory enumeration through one function that increments the counter, and add a lint that no findChildFiles / getChildFile enumeration exists outside it. Name the fixture, a composition with no effects anywhere. Add a second arm: show one effect, then assert dirReads is exactly 1."
    },
    {
      "id": "GA-5",
      "target": "Section 6 'what only Boris can check' items 3 and 6",
      "claim": "A machine could test two claims that are left to his eye. Item 3: 'flashes black for longer than a blink' and the layer restart. Item 6: the signal-driven slider keeps following its signal.",
      "evidence": "plan:509-511 and :516-517. G-EL-4 (:470) compares one before picture and one after picture, never the frames in between. LC-7 (:437) pins only the connection object and the live twin in a unit test. No live row proves the renderer still reads effParam for the wired parameter after a load. V-EL-16 (:498) is a screenshot that needs a human eye. Item 1's success-path claim 'no box, no message' is checked only on the failure path, by G-EL-7.",
      "severity": "SHOULD",
      "proposed_change": "Add G-EL-12: capture N consecutive frames through captureFrame around look_load and assert none is black (mean luma above a pre-registered floor). Add G-EL-13: wire a signal to one parameter, look_load, and assert the effective value reported by the debug route still equals the signal and not the look. Add a success-path windows-and-label check to G-EL-1. Leave Boris only the taste items 5, 7 and 8."
    },
    {
      "id": "GA-6",
      "target": "G-EL-10 and the effectLooksDir seam",
      "claim": "The seam that keeps tests off the real folder has no unit test and no mutant. The live row lists a folder Boris's own app writes to.",
      "evidence": "plan:476. Its RED arm is a 'selftest on a mutated copy that omits the variable check', which tests the probe script, not the app. Nothing mutates effectLooksDir (MU-EL-1..20 list, :401-453, has none) and nothing in tests/ calls the function (S2 owns only test_effect_look_cmd.cpp). The listing 'names, sizes, mtimes of ~/Library/Audio-DNA/' includes settings.json and the MilkDrop favourites (plan:96, F21). Boris's running app rewrites those, so the row fails by chance, or invites someone to exclude them. RIG-RULES says Boris's app and Resolume are his.",
      "severity": "SHOULD",
      "proposed_change": "Add a ctest on effectLooksDir(true): with the variable unset it returns a path under the temp dir, never under ~/Library; with a relative value it falls back to the scratch folder. Add mutant MU-EL-21, 'the seam returns the real folder in test mode', which must turn that ctest red. Restrict G-EL-10's listing to Looks/ only, so Boris's other files cannot matter."
    },
    {
      "id": "GA-7",
      "target": "G-EL-11, LK-10/LK-11 pins, LINT-EL-1",
      "claim": "The count bar has no number, and a pin and a lint pattern will go stale or misfire.",
      "evidence": "plan:477 'ctest = baseline + the new cases' states neither baseline nor delta (12+10+8+11 = 41 if one TEST_CASE per row). APP-INVENTORY.md:31 pins 1249 and the work log, .harmony/s-rta-1004-work.md line 12, item (14), records 'ctest -N 1252 vs 1249 unchecked', so a baseline pin is already off. LK-10 and LK-11 (plan:410-411) hard-code 'the 135', which any added effect from another lane breaks. LINT-EL-1 (:456) greps the word 'Looks', which the button text and reserved name also contain.",
      "severity": "SHOULD",
      "proposed_change": "Run ctest -N before S1 and write the baseline N and the expected N+41 into the row. Assert LK-10/LK-11 against registry size() and keep the literal 135 in one place. Make LINT-EL-1 grep the path literal 'Audio-DNA/Looks' or the folder constant. State how APP-INVENTORY's count line is updated in S3."
    },
    {
      "id": "GA-8",
      "target": "LM-11 / LM-8 layout and mouse gates",
      "claim": "LM-11 checks overlap among four widgets only. The collapsed value text, which the plan leaves in place, is not in it.",
      "evidence": "plan:453. EffectStackView.cpp:51-58 draws the collapsed first value right-aligned to headerBounds.withTrimmedRight(6), about 22 px wide at 11 pt, so it starts near width-30. The new button's right edge is width-28 (plan:219-220). They can overlap by a few pixels, and the fold arrow at width-16 sits under X (:60-81). The plan defers this as SF-3 (:597), but V-EL-1, the folded state, depends on it. LM-8's 'right press does not fold' is a model-level call, not a real MouseEvent routed through EffectStackView::mouseDown (:449-464, where the x > 28 test has no button check).",
      "severity": "NIT",
      "proposed_change": "Add the collapsed-value text rectangle and the fold-arrow box to LM-11's non-overlap set, measured with Font::getStringWidth. Build LM-8 from a juce::MouseEvent with the popup flag fed to mouseDown directly, with no synthetic OS input."
    }
  ],
  "strongest_point": "GA-2 together with GA-1. Several RED arms cannot make their gate fail: nine rows have only 'tree before the change', and G-EL-3's live mutant MU-EL-2 behaves identically to the correct code whenever the def is the same one that made the look. These are the 'a test that cannot fail' cases this seat is built to find.",
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
      "target": "EL3 the new LooksButton (a juce::Button subclass) plus the Rename window and Delete confirm; no keyboard-focus rule anywhere in the plan",
      "claim": "After Boris clicks the Looks button, JUCE leaves keyboard focus on it, and the next Return reopens the menu instead of reaching his key bindings. The plan never homes focus after the menu, Rename or Delete confirm close.",
      "evidence": "juce_Button.cpp:89 (every Button calls setWantsKeyboardFocus(true)) and :665-673 (Button::keyPressed on Return calls triggerClick and returns true, so the key never reaches MainComponent's binding listener). pitfalls.md:140 (Pitfall 65 part 2): JUCE parks focus on the first focusable descendant, and 'the next Return then clears the layer / fires the column', so every close must grabKeyboardFocus home. grep of plan-effect-looks.md for 'focus' prints nothing. No row G-EL-*, LM-* or V-EL-* covers it.",
      "severity": "MUST",
      "proposed_change": "Make LooksButton setWantsKeyboardFocus(false) and setMouseClickGrabsKeyboardFocus(false). Call MainComponent's grabKeyboardFocus (the onRenameClosed idiom, MainComponent.cpp:1398-1401) in the callbacks of the menu, Rename and Delete confirm. Add a unit row: after a look pick, a Return reaches the binding manager. Add a look_ui row reading the focused component's name."
    },
    {
      "id": "ST-2",
      "target": "EL3 menu: Rename and Delete 'enabled only while one of HIS looks is the matched one'; button text = first match in order Default, then names",
      "claim": "Boris can rename or delete a look only after loading it onto the effect. Loading a look he only wants to delete changes the live picture and pushes an undo step. A look whose values equal Default or an earlier-named look can never be matched, so it can never be renamed or deleted. 'New Look' on unchanged or already-matching values leaves the button reading 'Default' or the earlier name, not the new look, which contradicts the plan's 'becomes the matched look'.",
      "evidence": "Plan lines 225-227 and 236-239: matched = first of Default then his looks by name; Rename and Delete act on the matched look. Plan line 245: New Look 'becomes the matched look'. §6 item 1 only tests New Look after sliders were moved. LM-2 and ST-2 never test duplicate values.",
      "severity": "SHOULD",
      "proposed_change": "Give Rename and Delete a per-look path that needs no load, for example a submenu per look, or the menu's rows targeting the clicked look. Tie-break the button to the most recently made or loaded look kept in the view, as a pure hint verified against the values. Add a unit row: New Look on default values reads 'Look 1'."
    },
    {
      "id": "ST-3",
      "target": "EL4 'one fenced write'; R4; §5.6 gate rows",
      "claim": "Every look load wraps a two-float-array write in withDeckDetached. A fenced frame holds the canvas, and the recorder, Syphon and capture get no frame. Mid-set, each look click freezes all layers, not only that effect. Slider writes are unfenced today. No gate bounds the hold.",
      "evidence": "pitfalls.md:119 (Pitfall 55): a fenced deck-less frame re-presents canvasFBO_ untouched, with 'no clear, no composite, no capture answered, no recorder / Syphon frame'. fence_black_frames 'must stay 0 in a running app'. Plan lines 281-286 and 561-563 accept 'one fenced frame' per load. EffectStackView.cpp:356-360 and 405-411 write unfenced. grep of the plan for fence_hold or fence_black: none. G-EL-4 compares two stills only.",
      "severity": "SHOULD",
      "proposed_change": "Add G-EL-4b: read fence_black_frames (bar 0) and fence_hold_frames (report) around 20 rapid look_loads with a recording running. Reconsider the fence for the pure-float write, which has the same race as a slider. If kept, state the recorder and Syphon dropped-frame cost to Harmony."
    },
    {
      "id": "ST-4",
      "target": "EL4 Cmd+Z paragraph 'After an undo the rows fold ... not mended here'",
      "claim": "Cmd+Z right after a look collapses every effect row in all three stacks and rebuilds the deck grid and clip panel. The effect Boris is tuning vanishes into a folded header, and the clip inspector is re-pointed. Undo of the one feature built around fast A/B is the least live-friendly path, and the plan leaves it as is.",
      "evidence": "MainComponent.cpp:5350-5402 refreshAfterUndoRedo: syncAfterModelChange(Grid), clip inspector setClip, repointLayerInspector, rebuildCompositionEffects. EffectStackView.cpp:256-289 rebuildRows folds all rows (plan F10, F17). Plan lines 291-292. LC-4 asserts only model equality, not row fold state.",
      "severity": "SHOULD",
      "proposed_change": "In refreshAfterUndoRedo, when the processed command is an EffectLookCmd, call its own refresh and skip the grid sync and re-point. Or have rebuildRows keep expanded flags keyed by effect index. Add a unit row: Cmd+Z after a look leaves the row unfolded."
    },
    {
      "id": "ST-5",
      "target": "EL1 'wired parameter keeps following its signal; the look's value is stored underneath' and the derived button text",
      "claim": "The plan does not say whether capture and matches read paramValues (the base) or effParam (what the slider shows). New Look on a wired slider stores a number Boris never saw. If matches used effParam, a signal-driven parameter would keep the button at dim 'Looks' forever. After loading onto a wired slider, nothing on screen shows the look took.",
      "evidence": "EffectStackView.cpp:~214-216 refresh pushes fx.effParam(p), the live twin, to the sliders. Plan line 149 declares looks::matches(def, slot, look) with no source named. LM-3 and LK-9 use no connection. V-EL-16 is a capture of a loaded wired slider with no button-state assertion. No row pins a connected-parameter match.",
      "severity": "SHOULD",
      "proposed_change": "State that capture and matches read paramValues and dryWet only (never the live twins). Add unit row LM-12: a connected parameter whose live value drifts keeps the button on its look. Add V-EL-16 manifest fields for the button text and the base versus live value."
    },
    {
      "id": "ST-6",
      "target": "EL6 'His four files in ~/Library/AudioDNA/Presets/ become unreachable'; §6 item 9",
      "claim": "The 'look he made last week' is exactly those four files, and the plan strands them with no carry-over and no on-screen sign of where Save's job went. Boris loses his setups the moment S7 lands and has nothing in the app to point him at the Looks button.",
      "evidence": "facts-one-save.md Q6 (lines 80-86): Presets/ has 4 entries, each a snapshot of the legacy chain, and the old Save loads into the legacy chain only. Plan lines 332 and 521-522 only say they 'can no longer be opened'. Boris, BF72 (boris-feedback-backlog.md 'Boris's answer to question 86'): 'The old look buttons go now'. HD-1 runs S7 before this lane's S3, so there is a gap with neither.",
      "severity": "SHOULD",
      "proposed_change": "Make S7 land only after S3, or ship a one-time user-run conversion that turns each preset's effects into per-effect look files by name. Without it, tell Boris plainly now, as a question with a default, that his four setups are not carried over."
    },
    {
      "id": "ST-7",
      "target": "§5.6 live rows G-EL-1..10 and the look_ui route (menu, rename, delete, expand, fold, dismiss)",
      "claim": "No live row exercises picking an item from the real menu. All live rows call the data routes, and look_ui can only open the menu. The two MainComponent wiring lines (New Look, Load, Rename, Delete from the row's callback with the right scope and fx index) are covered only by unit tests with hooks. A wrong effect index or scope in that wiring passes every gate and is found by Boris.",
      "evidence": "Plan line 345: look_ui actions have no 'pick'. Plan lines 348-350 and 463-477: every G-EL row drives look_make or look_load, not the menu callback. LM-6 and LM-7 (lines 448-449) are unit tests with a stub host. Plan line 368 gives MainComponent.cpp 'two wiring lines' in S3.",
      "severity": "SHOULD",
      "proposed_change": "Add a look_ui action pick:<menu row> that invokes the real PopupMenu result callback through the same function a click calls. Add a row: pick New Look on a layer-scope effect, then GET looks shows the new file and the values of that effect only."
    }
  ],
  "strongest_point": "The plan is strongest on data safety. One file per look, a read-back before the swap, a one-slot EffectLookCmd that never touches connections or the layer strip, and a button text derived from the values so it cannot go stale. It is weakest where Boris's hands are: keyboard focus after the button and dialogs, loading a look just to delete it, and the undo that folds the rows.",
  "citations_rechecked": true
}
```

---------------------------------------------------------------------------------------------------------
## SEAT: scope (MINIMALIST)
---------------------------------------------------------------------------------------------------------

```json
{
  "seat": "scope (MINIMALIST)",
  "attacks": [
    {
      "id": "SC-1",
      "target": "EL2 'A look that could not be written' (ring, retryPending, quit-time retry, writer hook, onDisk field)",
      "claim": "A whole state machine is paid for a full or locked disk the plan itself says Boris 'should never see'. Cut it: make() returns false, no look is created, the menu and button do not change.",
      "evidence": "Plan :189-193 (ring, retry at each store operation and in the destructor), :519 ('you should never see the ring unless the disk is full or the folder is locked'). It drags in ST-5, MU-EL-10, LM-4, G-EL-7 (chmod rows, Quartz window counts), V-EL-6, HD-2 and the onDisk field of GET /api/debug/looks. The retry-at-quit still loses the look on a crash or a still-locked folder, so the ring cannot honestly promise 'always'.",
      "severity": "SHOULD",
      "proposed_change": "EffectLookStore::make returns nullptr on a failed write; the New Look click then visibly does nothing (no text, so no messages-lane conflict). Delete retryPending, onDisk, the ring in LooksButton and the menu model, ST-5, G-EL-7, V-EL-6, HD-2. Lost: a look on an unwritable disk is unusable this run instead of usable-until-quit. One line (the nullptr branch) changes if Boris wants it back."
    },
    {
      "id": "SC-2",
      "target": "EL2 THE WRITE (temp file + byte read-back + move) and the file format's unused fields",
      "claim": "A fresh per-look file needs no temp-and-swap and no read-back: the plan's own rule already hides a half-written file ('refused WHOLE, not listed'). The 'shader' key is written and never read, and version above 1 / unknown-key keeping pay for a future that nothing uses.",
      "evidence": "Plan :172 ('writes it and does not read it for matching'), :177 (unknown keys KEPT on rename), :178-180 (version rule), :184-188 (read-back compare), :178 (refuse-whole rule), ST-6 / MU-EL-11 / ST-7 / ST-10. The only reason for temp-and-swap is F24 (replaceWithText), which the plan already avoids by checking stream status after write and flush. Make never overwrites (file existence is the collision test, :183).",
      "severity": "SHOULD",
      "proposed_change": "Write the new file through a FileOutputStream on a deleted-first path (Pitfall 46), check status after flush, delete the file on failure. Drop the 'shader' key, the alias-table sentence, the version clause, unknown-key preservation, ST-6, ST-10 and MU-EL-11. Must keep: the bad-file-hides-itself rule (ST-3)."
    },
    {
      "id": "SC-3",
      "target": "EL3 right-click on a header opens the menu and stops folding (HD-5, LM-8, MU-EL-19)",
      "claim": "A second entry to the same menu that changes today's fold behaviour. Boris asked for no such thing, and the plan lists the cost itself (R7: 'he loses a habit').",
      "evidence": "EffectStackView.cpp:449-464 folds on any button when x > 28 (checked); plan :252-253, R7 :571, HD-5 :614. Nothing in Boris's 2026-10-04 records mentions the header click. The button is already one click from the menu.",
      "severity": "SHOULD",
      "proposed_change": "Delete the right-click path: no isPopupMenu test in mouseDown, no LM-8, MU-EL-19, R7 or HD-5. Lost: a second way to open the menu. The fold behaviour stays exactly as shipped, and a regression nobody asked for is not paid for."
    },
    {
      "id": "SC-4",
      "target": "EL3 derived button text 'which look the effect IS' (LooksButton paint state, matches(), 0.0005 tolerance, refresh hook)",
      "claim": "The largest recurring cost in the UI: every refresh (about 10 Hz, every effect row) must load the effect's folder and compare against every look, and the button needs the Pitfall 59 compare-what-you-paint machinery. Boris never asked for a name on the header. A static 'Looks' button with the tick computed only when the menu opens gives the same menu at no per-frame cost.",
      "evidence": "Plan :222-234 (state derived each refresh), LK-9 / MU-EL-6, LM-3 / MU-EL-17, LM-10 / MU-EL-20 (100 refreshes, 0 repaints), V-EL-1..3. R1 :548-553 names the weakness: after one slider move the name is gone anyway. F10: refresh() runs at about 10 Hz (comment at MainComponent.cpp:3056, per the plan). The binding records (84, 86) say only 'many looks', 'saved with the app', 'a small menu on each effect' (R42).",
      "severity": "SHOULD",
      "proposed_change": "LooksButton is a plain triangle button; looksmenu::build computes matches() once per menu open and ticks the match. Drop setState, the refresh() hook, LM-3, LM-10, MU-EL-17/20, V-EL-1..3 and V-EL-11's text-or-triangle mode. Lost: seeing the current look without opening the menu. If Boris wants that, it is one setState call in refresh() later, with no data change."
    },
    {
      "id": "SC-5",
      "target": "EL1 name matching: uniform first, then label, then keep current (two keys stored per parameter)",
      "claim": "Each stored parameter carries both a uniform and a label so a re-label or uniform rename in a later build still lands. Both are shipped by the same app, and the plan's own T6 pin makes uniforms unique and stable. Match on the uniform alone.",
      "evidence": "Plan :152-155 (two-step fallback), :175-176 (params array of {uniform, label, value}), LK-5, MU-EL-3. F4: tests/test_preset_manager.cpp:292 'T6' pins unique param and uniform names. CLAUDE.md Shader Rule 7 fixes the uniform naming convention. Pitfall 45's param lint ties each registered param to its shader read, so a uniform rename would also be a shader edit.",
      "severity": "NIT",
      "proposed_change": "Store {uniform, value}; resolve by uniform only. Drop the label field, LK-5's second half and MU-EL-3. Lost: a look survives a label-only rename. Keep the by-NAME rule, which is the real requirement."
    },
    {
      "id": "SC-6",
      "target": "EL7/5.6 live rows G-EL-1, 6, 7, 9 and G-EL-8; question 105; the test routes",
      "claim": "Live rows duplicate unit rows with the same RED arm, and G-EL-8 measures something the lane does not act on. Each live row costs a test-server route, a probe stage and a Harmony run on Boris's machine. Question 105 (are looks played back by a take) is not Boris's: the lane keeps the answer-A behaviour, and a mouse slider already behaves that way.",
      "evidence": "G-EL-1 repeats ST-1 (MU-EL-8), G-EL-6 repeats ST-3 (MU-EL-9), G-EL-7 repeats ST-5 (MU-EL-10), G-EL-9 repeats ST-9 (MU-EL-12): plan :467, 472, 473, 475 against :417, :419, :421, :425. G-EL-8 :480-488: the predicted outcome is 'nothing', and a differing outcome only STOPs a lane whose default is already 'not played back' (Q105 A, :541-543). F12: slider writes are unrecorded today. The plan itself says the record is a separate lane (SF-2 :303).",
      "severity": "SHOULD",
      "proposed_change": "Keep live G-EL-2 (relaunch, different show: Boris's actual 'always'), G-EL-3, G-EL-4/5 (strip unchanged, picture changed, undo) and G-EL-10 (real folder untouched). Move 1, 6, 7, 9 to unit-only. Drop G-EL-8 and Q105 and say 'recording looks is its own lane', as SF-2 already does. Fewer routes: look_make and look_load plus GET /api/debug/looks."
    },
    {
      "id": "SC-7",
      "target": "EL3 Rename + auto-named 'Look N' + question 104; and the NEW Look 'no box' reason",
      "claim": "The plan avoids a name box because 'a text box takes the keys in the middle of a show', then ships a Rename window and a Delete confirm, which take the keys the same way. The argument holds for neither or both. Boris asked only to 'pick a look, or keep the current settings as a new look' (R42): no Rename.",
      "evidence": "Plan :244-249 (no box at creation; the Rename AlertWindow idiom from MainComponent.cpp:6336-6342, which I checked), :250 (Delete confirm). R42 in boris-clarify-86.md has no rename. Costs: ST-7, the rename route, look_ui rename, V-EL-7, unknown-key preservation, the reserved / clash / length checks in two places.",
      "severity": "SHOULD",
      "proposed_change": "Ask 104 with B as the cheaper reading: New Look opens the one existing AlertWindow idiom for a name. Delete Rename (item, route, window, ST-7, V-EL-7). A typo is fixed by delete and remake. Lost: renaming in place. If auto-names are kept (default A), the plan should drop 'no box' as a reason, since it applies to Rename too."
    },
    {
      "id": "SC-8",
      "target": "Section 4 stage order and the visual gate (S1 -> S2 -> S3 strictly in order; 17 states, five critics; EffectLookOps.h)",
      "claim": "S1 (store) and the Cmd in S2 do not depend on each other, yet the lane is strictly serial with 'nothing live' until S2, and other lanes wait behind it. The visual gate has 17 states for one button, most of them edge cases. EffectLookOps.h exists to give tests a 'no host' direct-write path that the app never uses.",
      "evidence": "Plan :358-369: S1 'nothing live', and its pinned review gates S2. EffectLookCmd takes looks::Values from EffectLook.h only (:271), not the store. V-EL-12 (40-character name), 13 (bypassed header), 14 (two instances), 15 (thirty looks), 17 (two window sizes) are layout cases already covered by LM-11 (widths 400..160) and LM-6. :277-279 loadInto 'with no host writes directly under the fence'; MainComponent always supplies the host.",
      "severity": "NIT",
      "proposed_change": "Build EffectLook.h + EffectLookCmd (pure model) as S1 and the store as S2 on a side branch, so S3 (menu) can start as soon as both are reviewed. Fold Ops into the Cmd. Cut the visual gate to V-EL-1,2,4,5,7,8,9,10,11,16 and run three critics. Pitfall NN, docs and test routes stay in the stage that changes them."
    }
  ],
  "strongest_point": "The disk-failure ring and retry machinery (SC-1) and the always-on derived button label (SC-4) are the two biggest mechanisms paid for something Boris never asked for. The plan itself says Boris should 'never see the ring' (:519) and names the label's weakness as its own strongest risk (R1). Cutting both removes about a third of the unit rows, three live rows and four visual states. It costs nothing in the data path. The one thing that must NOT be cut is EL2's core: one file per look, one folder per effect, each written whole when the look is made, values stored by name, and no shared read-modify-write file. AppSettings.cpp:19-40 shows the trap it avoids: an unreadable file reads as empty and the next update drops the other keys. That store shape is what Boris's 'saved with the app. always.' actually requires, and it is cheap. Also keep the one-slot fenced EffectLookCmd, because a whole-chain EffectStackCmd would clear every connection's grip (ParamConnection.h:147-160, as the plan claims).",
  "citations_rechecked": true
}
```

