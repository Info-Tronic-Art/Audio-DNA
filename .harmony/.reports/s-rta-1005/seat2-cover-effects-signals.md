# SEAT cover-effects-signals (round 2) -- paper
Written: 2026-10-05 16:44:58 (read-only; nothing built, run, launched or committed; lane worktrees not opened)
Area: effects and the effect stack, the effects tab, presets, signals on sliders, macros, envelopes, the Timeline, the per-type autopilot.
Read: area-effects-signals.md (part 4 E1-E20, MISSED X1-X6, W1-W8, part 5, part 3 O1-O13), boris-all-items.json (questions 178, 181, 183, 184, 188, 192, 203-206, 210-213, 215; readings R127-R131, R136, R137, R148, R154-R157, R160, R171-R173, R180, R181, R188, R193, R195-R198, R203, R207, R208, R210-R213; decided 1-28; design 1-31; told-in-chat), his two Resolume pictures (looked at), BD:250-265, 589-595, 641-646, 960-966, 1087-1092.
Marks: VERIFIED = I read it (file:line / quoted words); INFERRED = from what is named; UNKNOWN = with cheapest check.

VERDICT: FAIL (2 MUST, 9 SHOULD, 5 NIT). The area is well covered: of 45 points, 34 are fully covered, 9 partly (a named gap each), 2 have no home (E20, and the Timeline's scope from his own words). Two things must be mended before the page goes to him: Q205 tells him a thing works that does not, and his own words on where the Timeline applies are lost.

======================================================================
## A. WHAT I RE-CHECKED IN THE SOURCE (five+ load-bearing lines of the sheet, and the list's today-claims)
======================================================================
V1 VERIFIED src/ui/BrowserPanel.h:48 the tab button reads "FX" (R207 right).
V2 VERIFIED src/ui/EffectStackView.cpp:449-462 any mouse button on a header at x>28 folds the row; resized() at :96-97 puts B at x=2 (LEFT of the name) and X at the right edge. His picture: fold arrow, name, then B, P, X on the RIGHT (looked at). So the B button moves; R136 does not say so (F3).
V3 VERIFIED src/signal/EnvelopeSignal.h:60-68 cyclePhase is already under 1 after fmod, so the One Shot clamp is inert: "Looping" / "One Shot" change nothing (R195 says "stays as it is": F7).
V4 VERIFIED src/connect/ConnectionEngine.cpp:327-345 passes nullptr as the clip clock at each site: "Clip Position" reads 0 today (Q205 option B calls it "a straight line along the clip": F1).
V5 VERIFIED src/ui/FXBrowser.cpp has no "preset" (grep count 0); FXBrowser has no double-click call (sheet W6, not re-read).
V6 VERIFIED src/model/Autopilot.cpp:326, 355, 417 every autopilot advance uses BeatSnapMode::Off (R213 a "brings it in at once" right); :74-76 it counts only beat crossings (R213 b right: no count on a stopped beat); :249-300 sequence wraps with %, random picks a candidate and can return to the previous clip (R203 a right).
V7 VERIFIED nothing calls setSmartRandomEnabled except the Renderer's own reader of an atomic nobody sets (Renderer.h:152, Renderer.cpp:511); smartAutopilotEnabled is saved only (Composition.h:192, 799, 983): Q212 (e), (f) right.
V8 VERIFIED src/ui/MenuBarModel.cpp:10, 155-161 menu "Shortcuts", entries as R148 (a) lists; BindingOverlay.cpp:68, MidiLearnOverlay.cpp:95 the two headings; MainComponent.cpp:7342, 7357 the file windows. R148 is the ONE reading the second message asked for and is right.
V9 VERIFIED src/ui/LayerInspector.cpp:408-416 the layer's Feedback list is Custom + six names and its tooltip reads "Feedback preset". The screen has no "look" anywhere there (F9).
V10 VERIFIED BD:591-594 and BD:645 his Timeline words (quoted in F2).

======================================================================
## B. COVER TABLE (every point -> its home)
======================================================================
Status: COVERED = his answer (or silence) lets a builder go on; PARTIAL = covered but a gap named in the finding; NONE = no home.
| Point | Home | Status |
|---|---|---|
| E1 does an effect show a preset name | Q178 (A/B/C, with his picture); W8 (his "preset name area") is in Q178 A's why | COVERED |
| E2 the small Save button: where / when / measured against what | R127 (a)-(e); where it sits: design 5; "after Save the effect carries the new name" is not said | PARTIAL (F4) |
| E3 what Save does; what Manage... holds | R127 (d)(e), R155 (a); the window's look: design 7 + R206 (g) | COVERED |
| E4 the effects tab with presets under each effect | R136 (c), R154, R180 (b); design 6; double-click on an EFFECT (Resolume previews it, FRE A5.8) is not said | PARTIAL (F10) |
| E5 what a preset holds; what loading does to a signal | Q192; R157; R156 (second P not copied) | COVERED |
| E6 four ways to put a repeating shape on a slider | Q205 (names the four), R195 (a)(d); R181 (e) for actions | COVERED |
| E7 where the Timeline curve is drawn; Clip Position and Timeline both stay | Q205 + design 28; "Clip Position" is mis-described | PARTIAL (F1) |
| E8 is a signal's motion part of a recording / an action | decided 17, R181 (e), R137 (b); his BD:962 sentence quoted in R181 (e) | COVERED |
| E9 a slider driven by a signal AND an action or the hand | Q181 (who wins), R129 (2), Q213 (jump or glide); the return of a SIGNAL-driven slider is not in Q213's picture | PARTIAL (F5) |
| E10 a lamp to switch a signal off | R195 (a)(b): Manual stays, the lamp is for actions only | COVERED (reading) |
| E11 beat-locked signals when the beat is paused / stopped / restarted | R210 (c); R193 (b); drop switch R210 (b); Resync W4 already built. Not said: "Once" start, the dead Looping / One Shot toggles, and an effect that reads the beat itself | PARTIAL (F7) |
| E12 sample envelopes: trim handles, position line | decided 26 | COVERED (NIT 2) |
| E13 beats or bars | R196 (W3 resolved) | COVERED |
| E14 names that collide | Q210, R148, R197 (a)-(i), R207; feedback list: R197 (e) says "six looks" | COVERED (F9) |
| E15 macros: banks, saved | R210 (a) three banks, R188 (b) saved; whether a clip bank is ONE bank or one per clip is not said | PARTIAL (F8) |
| E16 what of a signal is saved; can he make more | R188 (b), R195 (d) | COVERED |
| E17 autopilot in the new model | R203, R213, Q212 (e)(f); W1 (he has no words on it) is respected | COVERED |
| E18 how much of Resolume's effect panel besides presets | R195 (g) says re-order / copy / paste stay absent; decided 9 and R198 (b) speak as if re-ordering exists | PARTIAL (F6) |
| E19 preview an effect or preset before it enters the show | Q177, R149 (b), R151; double-click on an effect: not said | PARTIAL (F10) |
| E20 an effect added or removed during a take, a preset in a take | R137 (b) preset jump, R173 removed-since is marked missing, D16 every control from the first moment. Not said: an effect ADDED or REMOVED while recording | NONE (F11) |
| X1 no smoothing / curve on a signal-driven slider | R195 (e) | COVERED (reading) |
| X2 a preset stores only the signal's name | R180 (a), R188 (b) | COVERED |
| X3 the restart-on-drop switch has no control | R210 (b) | COVERED |
| X4 does Master Signal touch an action | R195 (b) | COVERED |
| X5 Freeze / Echo memory on preset load, B, remove | decided 27 | COVERED |
| X6 value read-out units | R195 (f), R208 | COVERED (NIT 1) |
| W2, W3, W4, W5, W8 (corrections) | as above: handles snap (D26), bars (R196), Resync / switch (R210), three macro banks (R210 a), preset name area (Q178) | COVERED |
| O1 133 vs 150 | R127 (c) | COVERED |
| O2 "look 2" button vs his picture | Q178 | COVERED |
| O3 menu of the earlier ruling vs his picture | R136 (b), R156 | COVERED |
| O4 presets not listed in the effects tab (R95) vs his picture | R180 (c), R136 (c) | COVERED |
| O5 "does not play the preset change back" vs 169 | R137 (b) | COVERED |
| O6 103 / 132 save-over vs one Save | R127 (e) | COVERED |
| O7 big envelope view | decided 26 (the plan's Q1 / Q2 never go to him) | COVERED |
| O8 beats in the envelope lengths | R196 | COVERED |
| O9 glide vs jump on hand-back | R195 (c), Q213 | PARTIAL (F5) |
| O10 a routine is a kind of signal | R143 (routines replaced by actions); nothing to ask | COVERED (plan, not his) |
| O11 drawn curve and captured lane are the same object | Q205 | COVERED |
| O12 RU: a take never stores which signals are plugged in; New Look opens no window | R127 (d), D17 | COVERED |
| O13 autopilot and the wait-for-the-line machinery | R213 (V6 confirms: autopilot fires with snap Off) | COVERED |
| Part 5: 131 | Q192 | COVERED |
| Part 5: 138 | R180 (c), R136 (c) | COVERED |
| Part 5: 151 | defaults line, R197 (e), R202 | COVERED |
| Part 5: R121 | told-in-chat R121 (points to 178 and 192) | COVERED |
| Part 5: Q25 / BF19 "where do I draw the timeline curve" | Q205 situation answers "nowhere today" | COVERED |
| N6 (part 6, BD:591-594, 645) the Timeline's scope: clip and layer, not composition; no Timeline on MilkDrop | Q205 says "along the clip" only | NONE (F2) |
| T1-T11 | architects; not checked here | n/a |

======================================================================
## C. FINDINGS (exact new wording)
======================================================================
F1 MUST Q205 (E7, 1.16, V4). Option B says "Clip Position" (a straight line along the clip) "stays". Today it reads 0 and does nothing, because the clip clock is not connected (ConnectionEngine.cpp, nullptr at every site). He would think a working thing stays.
FIX: add to the situation, after "...no curve to draw yet.": "The list also holds "Clip Position", a straight line from the start of the clip to its end; today it reads 0 and does nothing, because it is not connected to the clip yet." Option A, end: "..."Clip Position" stays and is connected." Option B: "No Timeline curve: an action is already a curve on a slider that plays with its clip. "Timeline" leaves the source list; "Clip Position" stays and is connected."

F2 MUST NEW reading (N6; his words BD:591-594 and BD:645 are in no item). Q205 speaks only of "the clip"; his rulings that it also works on a layer, is not needed on the composition, and that MilkDrop has no timeline are lost.
FIX, new reading (topic I): "Your words: "when I set any clip parameter to timeline it should be locked to the clips playhead, same with layer, not necessary for composition controls" and "For Milk drop there is no timeline, but there are effects." So: (a) "Timeline" is offered on a clip's sliders and on a layer's sliders, and not on the composition's. (b) On a layer it follows the clip that plays on that layer. (c) On a clip that has no length of its own (MilkDrop, a generated source, a still picture, the camera, an effects-only clip) the entry "Timeline" is greyed; every other signal still works there. (d) I read "no timeline" for MilkDrop as covering the stills and generated sources too; say so if one of them should have a Timeline."

F3 SHOULD R136 (b) and design 5 (V2). In his picture the header reads: fold arrow, name, then B, P, X at the right. In the app today B sits at the LEFT of the name. R136 says "as today" for the name and "P between B and X", so he is not told that B moves.
FIX: R136 (a) end: "... as today. One thing moves: today the small "B" sits at the left of the name; in your picture it sits at the right, with the "P" and the "X" beside it, and the fold arrow is at the left. I take it that B moves there."

F4 SHOULD R127 (d)(e). After Save, does the effect now carry (and, under 178 A, read) the new preset? With 178 A the button must read something after the name window; not said, and E2 asked "does it go away after saving".
FIX: add to R127 (d): "After Save the effect carries the new preset and reads its name, and the Save button goes away until a slider moves again." (under 178 B: "the button goes away only if the sliders then match a preset").

F5 SHOULD Q213 / Q181 (E9, O9, V-none). His "smooth transition back." (ruling 7) is for a slider a SIGNAL owns. Q213's picture is a fader and its default is a jump; Q181 A says "goes back to the signal" without saying jump or glide. For a signal-driven slider the two answers collide.
FIX: Q213 situation, add: "The same question for a slider a signal was moving: when the action goes off, does it jump back to the signal or glide, as your "smooth transition back" says for your hand?" Add an option line: "D: Sliders with a signal glide back to it (your "smooth transition back"); everything else as A." and say in the why: "A and D differ only on sliders a signal moves."

F6 SHOULD E18 / decided 9 / R195 (g) / R198 (b). Decided 9 says "when you re-order a clip's effects" and R198 (b) lists "moving ... an effect"; R195 (g) says effects cannot be re-ordered, and that it stays. He is told both. Effect order changes the picture and Resolume allows a drag.
FIX: (i) decided 9: "If effects can ever be re-ordered (question NEW below), an action follows its effect; if you remove one, ..." (ii) R198 (b): strike "moving ... an effect". (iii) NEW question (topic I): "You have Blur above Ripple on a clip and want Ripple first. Today an effect can only be added at the end, never moved; to change the order you remove and add again. A: As in Resolume: drag an effect's name row up or down. Default: it is what your Resolume does and what decided 9 already assumes. B: As today, no re-ordering. Why it matters: the order of effects changes the picture, and actions, recordings and keys find an effect by its place. Copy and paste of effects stay out either way."

F7 SHOULD NEW reading (E11 rest, V3). R195 says signals "stay as they are", but today the Signal tab's "Looping" and "One Shot" toggles do nothing, and nothing says when a "Once" envelope starts; R210 (c) does not say what an effect that reads the beat by itself does on a pause (his own ruling: such effects keep pulsing at Master Signal 0).
FIX, new reading (topic I): "Mine, two edges. (a) In the Signal tab an envelope has two toggles, "Looping" and "One Shot"; today they change nothing. Your rule is to make a dead control work: they will. A one-shot envelope starts on the next "1" (the default you took earlier), plays once and holds its last value. (b) An effect that reads the beat itself (a strobe, a pulse) follows the beat like the signals do: it holds still while the beat is paused or stopped. (c) Master Signal at 0 still leaves such effects pulsing while the beat runs, as you ruled. Say so if (b) is not what you want." (b) is INFERRED; today's behaviour on a pause is UNKNOWN (cheapest: one read of how the shaders get the beat phase on a pause).

F8 SHOULD R210 (a). "three banks, 8 knobs each" does not say whether a clip has ONE bank or each clip has its own (Resolume: per clip, per layer, one for the composition), nor which slider a plugged-in "Macro 3" follows in each tab.
FIX: "(a) Macros: three kinds of bank, 8 knobs each, as you ruled ("all. Global, layer, clip."): every clip has its own 8, every layer has its own 8, and there is one global 8. A slider in the Clip tab plugged into Macro 3 follows that clip's third knob, in the Layer tab that layer's, in the global tab the global one. Today there is one bank shown in all three tabs. Say so if a clip should share one bank."

F9 SHOULD R197 (e). "the layer's Feedback list of six looks stays as it is": on screen the list is Custom, Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch and its tooltip reads "Feedback preset" (LayerInspector.cpp:408-416); no "look". And his 138 turns every on-screen "look" into "preset"; "preset alone means an effect's preset".
FIX: "(e) "Preset" alone means an effect's preset; MilkDrop keeps its own "presets" (151 A). The layer's Feedback list (Custom, Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch; its hint reads "Feedback preset") keeps its entries; its hint will read "Feedback style" so that "preset" belongs to effects and MilkDrop only. Say "R197 e keep" if it should stay "Feedback preset"."

F10 SHOULD R154 (E4, E19). Resolume previews an EFFECT on a double-click (facts A5.8); in the app a double-click on an effect in the tab does nothing at all (FXBrowser never calls it). R154 speaks only of a preset's double-click, and no item says that an effect is not previewed before it enters the show.
FIX: R154, replace the "double-click" sentence: "Mine: a click on a preset in the tab only selects it for dragging; a double-click on a preset or on an effect does nothing in the first build (in Resolume a double-click on an effect shows it in its preview monitor; here the preview monitor shows clips only, R151, and effects are judged on the output)."

F11 SHOULD NEW reading (E20). D16 says a recording keeps every control "from its first moment", R173 says an effect removed AFTER is marked missing. Not said: an effect you add or remove WHILE recording; a row's identity is the effect's place, so a replay can only move effects that were there.
FIX, new reading (topic E): "Mine, no word of yours covers it. A recording has rows only for the effects that were in the show when you pressed Record. An effect you add while recording has no rows and is not added again on replay; an effect you remove while recording keeps its rows, which move nothing once it is gone. An action saved from such a recording plays on a clip that has the effect (question 184 says what happens when it does not). Say so if adding or removing an effect should itself be recorded."

NITs (paper only)
N1 R195 (f) says "Sliders read 0 to 1, without units"; R208 says "most sliders read 0 to 1; some read a percent or degrees"; his picture shows "100 %" and "0 °". Make one sentence: "Effect sliders read 0 to 1 (your picture shows % and degrees: not copied)". 
N2 decided 26 says "no moving position line in the Signal tab": his words (BD:641-644) refuse the bigger window, not the line; put the line in a reading so he can object.
N3 Q205 option A names the Signal tab as the place and design 28 asks "where": reword A "...you draw the curve in a small editor (in the Signal tab, as an envelope; or in a strip under the slider: design 28)".
N4 R136 (b) "the effect's preset button" and Q178 A "reads the preset's name, with a small arrow": say the button's place is design 5.
N5 R195 (d) "more can be made only for envelopes, when that part is built": say which build ("the next envelope work"), or drop the clause.

======================================================================
## D. TODAY-CLAIMS OF THE LIST IN THIS AREA, AGAINST THE SHEET
======================================================================
Right (VERIFIED against the sheet part 1 and the lines above): R207 (tab reads "FX"), R211 (ten slots, Save / Load / FX Save), R188 (b) (signals and macros not saved today), R195 (a)-(g) bar the inert toggles, R210 (a) one bank today and (b) the switch with no control, R203, R213, Q212 (e)(f)(g), Q192, Q178 (picture facts), R148 (every string).
Wrong or loose: Q205 option B ("Clip Position ... straight line": reads 0, F1); R136 (B's place, F3); R197 (e) ("looks": F9); R195 (g) vs decided 9 / R198 (b) (F6).
UNKNOWN left open: what an effect that reads the beat does on a pause (F7); whether the room is enough for the P menu and a Save button on a 24 px header (design 5 decides); Resolume's double-click on a preset (R154: honest).

Counts: points checked 45 (E1-E20, X1-X6, O1-O13, part 5 x5, plus N6 of part 6). Fully covered 34; partial 9 (E2, E4, E7, E9/O9, E11, E15, E18, E19); none 2 (E20, N6). Uncovered returned: 3 (E20, N6, and E11's sub-gap on Looping / One Shot / beat-reading effects).
