# seat2 cover-outputs (blind seat, round 2) -- written 2026-10-05 16:44:14
Read-only. Nothing built, run, probed or launched. Lane worktrees not read. Sheet: area-outputs.md (parts 3, 4, 5, CORRECTIONS incl. W1-W3, M1-M7). List: boris-all-items.json (final; Q172-215, R127-R213, decided 1-28, design 1-31). Abbreviations: BD binding-decisions.md, BL backlog, Rn reading, Qn question.
Source lines I re-read myself (VERIFIED): SharedFrameSet.h:7-11 (outputs keep the last frame when the context dies); MainComponent.cpp:2715-2726 (big signal bar hides the preview panel); MenuBarModel.cpp:134-150 (Output menu items; Syphon Output a tick item, not in the display list); OutputManager.cpp:100, 125-135, 178 (closeAll never re-saves; forgetSaved on open = W1 is TRUE); SyphonOutput.h:59-62 (enabled_ false at boot; MainComponent.cpp:7318-7323 "Default OFF each boot"); Renderer.cpp:2482-2511 (transform skipped at default; clear to opaque black); CompositionInspector.cpp:158-190 (Anchor writes X only, Y = 0; Resolution applies at once, no recording / output check anywhere in the file or CanvasSizeCombo.h); Composition.h:95, 753, 756, 770-774 (master opacity, canvas size, transform all saved with the show); PreviewPanel.cpp:76-82 (tab only recolours); boris-clarify-51.md:11-22 (R45-R56 as told).

## 1. COVERAGE TABLE (point -> item that covers it; "partial" = a builder would still guess something)
| Point | Covered by | Verdict |
|---|---|---|
| H1 where Delay is remembered | Q199 | covered; gap: with A a screen that is not plugged in has no entry, so he cannot prepare a venue at home (sheet H1 edge) -> F11 |
| H2 type a Delay / +/- | R191(h) (typing only) | PARTIAL: his words (BD:578-580) also asked plus/minus; reading drops it silently -> F7 |
| H3 which picture a recording / snapshot shows | R187(d) (plain canvas, quotes his "exactly like the output") | covered |
| H4 Device menu / Syphon in it | R191(b), Q200 | covered (reading corrects his picture; labelled) |
| H5 panic key and Syphon | Q200 | covered; gap on launch state and Restore -> F3 |
| H6 Syphon visible while unticked | Q200 (bundled in A / B) | covered, bundled |
| H7 frozen picture | R209 | PARTIAL: names outputs only; Syphon and a running recording stop at the same moment -> F4 |
| H8 monitor delayed? | R191(e) | covered |
| H9 Delay vs nudge | R177 | covered; gap: "a little ahead" (O2) -> F8 |
| H10 screen opacity + master | R191(d) | covered (Syphon missing, see M5) |
| H11 Delay/colour on key or pad | R191(c), Q206 A "only the output screens' settings stay out" | covered, consistent |
| H12 names | R153, R197, R148 | PARTIAL: "Output Settings" (Composition tab, Resolution) vs the eight rows of the "Output Screens" window never told apart -> F12 (NIT-level) |
| H13 old name-click path | R150 | covered |
| H14 monitor undock / on an output | R170, decided 6 | covered |
| H15 outputs while reviewing | Q186 | covered; says "outputs" only, Syphon not named -> F13 (NIT) |
| H16 Snapshot / Copy Image / Transform Widget / Fit / caption | R153 | covered (Fit and the size caption not named: NIT) |
| H17 where a screen's settings open | design 25 | covered (picture) |
| H18 a part of the canvas on a screen | R191(f) | covered; reading does not say Resolume can (slices) -> NIT |
| M1 panic and Restore Last | R191(i) | PARTIAL: same session only, quit-after-panic not said -> F9 |
| M2 Resolution change while live | decided 24 | PARTIAL: reads as today's rule, is NEW; omits Syphon and "open a show of another size" -> F10 |
| M3 rotation shear, Scale 0, Anchor | decided 23 | PARTIAL: Anchor (X only, Y forced 0, does nothing alone) not told -> F6 |
| M4 judging colour from the laptop | R191(e) | covered |
| M5 Syphon opacity black vs see-through | R191(d) names only "that screen"; R52 told it for Syphon but the final list dropped it | NONE for Syphon -> F5 |
| M6 / T2 real frame step on his Mac | NONE (R45's "about 17 ms" told earlier, not carried; measured 93-99 per s on a 120 Hz screen) | NONE -> F1 |
| M7 Syphon not like Resolume's | Q200 | covered (no question needed) |
| T1 second picture for the monitor | decided 7, 8 | covered |
| T3 display identity, twins, replaced projector | NONE (R50 told earlier, not carried) | NONE -> F2 |
| T4 doc disagreements | architects | n/a (not his) |
| T5 transform defects | decided 23 (rotation, Scale only) | PARTIAL -> F6 (W2 correct: portrait claim of the sheet is false; Position is not a defect) |
| T6 Syphon with hidden panel | none | PARTIAL -> F4 |
| T7 / T8 | follow H13 / architects | n/a |
| plan5 Q1 launch | R191(g) | covered |
| plan5 Q2 Esc | R191(g) | covered |
| plan5 Q3 show remembers screens | R191(g) "never opens by itself", R188(d) | covered |
| plan5 Q5 movable output window | NONE | NONE -> F14 |
| plan5 Q6 replug | R191(g) | covered |
| plan5 Q7 frozen | R209 | = H7 |
| plan5 Q8 part of canvas | R191(f) | = H18 |
| Q38 | Q199 | = H1 |
| R21-R23 | R191(e), R187(d), Q199 | covered |
| R45 frame step | NONE | -> F1 |
| R46 Device text | R191(b) | covered |
| R47 list text "40 ms" | design 25 | covered |
| R48 never keyboard, drag | R191(h) partial | see H2 |
| R49 with the screen | Q199 | covered |
| R50 twins | NONE | -> F2 |
| R51 raise holds still, lower skips | NONE | -> F2 (same decided line) |
| R52 opacity black incl. Syphon | R191(d) screen only | -> F5 |
| R53 Syphon off at launch, one frame later | NONE in final list (Q200 A silent) | -> F3 |
| R54 colour rows -1..1, Resolume formulas unpublished | NONE (R191(a) names the rows, no range, no "ours are the usual ones") | -> F15 (NIT) |
| R55 no key / pad | R191(c) | covered |
| R56 video memory | NONE | NIT, architects (cost line only) |
| O1 signed range / typing | R191(a),(h) | covered |
| O2 "a little ahead" | NONE | -> F8 |
| O3 top-bar dial, show memory | R177, R188(a) (nudge amount in show), Q199 | covered; today no sync dial exists in TopBar.cpp (grep Sync: only Link, Resync) so nothing to remove |
| O4 R11 500 ms | R191(a) | covered |
| O5 Syphon delayed | R191(a), Q200 | covered |
| O6, O7 | internal | n/a |
| O8 per-room need | Q199 | covered |
| O9 recording exactly like the output | R187(d) | covered |
| O10 Device menu vs text | R191(b) | covered |
| W1 panic forgets opened screens | R191(i) "today it forgets them" -- true (OutputManager.cpp:100,125-135) | no contradiction |
| W2 / W3 | corrections only | n/a |
| Incomplete notes: R37, plan5 Q3 text, restore greyed | R191 | covered / NIT |
| Canvas + Resolution list today (sheet items 25, 27-30) | NONE as a "stays" reading; R188(a) lists "the canvas size" only | -> F6 / F16 |

## 2. TODAY-CLAIMS CHECKED (items of the list in my area vs sheet part 1 and corrections)
R153 two tab buttons only recolour: VERIFIED true. R150 name click puts Image/Source clip on the main picture when nothing plays: matches sheet item 23. R209 projector holds last picture when the signal bar opens: VERIFIED (MainComponent.cpp:2715-2726 + SharedFrameSet.h:7-11); "minimise": from the SharedFrameSet comment only. R191(i) "today it forgets them": true for screens opened this session (W1). Q200 B "all outputs off leaves it running": VERIFIED (closeAll has no Syphon call; grep of OutputManager.cpp for "syphon": no hit). Q200 B "other programs see Audio-DNA from launch": INFERRED (APP-INVENTORY.md:35 + SyphonOutput.mm), the list states it as fact -> label. decided 23 "today a wide picture shears": code read, visual not run; the line carries no "inferred" in its text (its by-field does). decided 24: no today-claim, but see F10. No CONTRADICTION (MUST) found.

## 3. FINDINGS (exact new wording)
F1 SHOULD NEW decided: "A screen's Delay is set in whole milliseconds, but a screen takes a new picture once per refresh of that screen (about 17 ms on a 60 Hz projector, about 8 ms on a 120 Hz one): a change of 1 ms can show no difference, and two screens with the same Delay can sit one step apart." Why: R45 told "about 17 ms" earlier, measured publish rate was 93-99 per s (a lower bound, Arena running); the final list carries neither. Label: INFERRED; the number owed to the re-run with Arena closed (that request to him can sit in R206 as an eighth look: "close Arena for a few minutes").
F2 SHOULD NEW decided: "A screen's settings are tied to the screen as the Mac reports it, never to its size or place. A different projector on the same socket comes up with default settings (Restore Last Outputs still reopens it). Two projectors of the same model that the Mac cannot tell apart share the socket's settings, and the Output Screens window says so. Raising a screen's Delay holds its picture still for the added time, lowering it skips ahead, never black." (R50, R51 told earlier, dropped; a VJ with two identical projectors would be surprised.)
F3 SHOULD Q200 option A, append: "Syphon is off at every launch; Restore Last Outputs brings it back along with the screens that were on." (default: R53 off at launch, told; Restore inclusion is Mine, since he calls Syphon an output.) Without it a builder guesses both.
F4 SHOULD R209, replace the first sentence: "Mine: the outputs, Syphon and a recording that is running keep going, always." and add "Today they stop at the same moment (the monitor panel hosts the picture; read in the code, not run), not only the projectors." Why: a recording that silently stops when he opens the big signal bar is a quiet loss.
F5 SHOULD R191(d), replace: "(d) A screen's Opacity fades that screen to black, and Syphon's Opacity fades the Syphon picture to black (a Syphon picture has no see-through part today, so another program cannot see what is behind it); the master opacity fades the whole show; both stay."
F6 SHOULD NEW reading "Stays as it is: the canvas and the Transform": "Stays as it is, apart from the two mends of the decided lines on rotation and Scale. (a) The picture size is the Resolution drop-down in the Composition tab, section Output Settings: 1920x1080, 1280x720, 2560x1440, 3840x2160, 1080x1920 (portrait), 1080x1080 (square), 1024x768 (4:3). A Custom line shows only while a show's size matches none of them; you cannot type a size. (b) The Transform section of the same tab (Position X, Position Y, Scale, Rotation, Anchor) moves, scales and turns the whole picture after every effect and before the master opacity, so the monitor, every screen, Syphon, recordings and snapshots show it. What the picture no longer covers is black, on Syphon too. (c) Today the one Anchor slider only moves the turning point sideways and does nothing alone; Mine: it gets an up-down partner, as Position has." Also add to decided 23: Anchor.
F7 SHOULD R191(h), replace: "(h) the Delay's number can also be typed and has plus and minus for one millisecond at a time, as you asked for the old sync dial (a slider cannot land on every millisecond)." Today's ruling says drag only (RO A-12); his BD:578-580 says typed + plus/minus.
F8 SHOULD add to R177: "A Delay only makes a screen later, as in Resolume's; a picture cannot be made earlier than the sound. Your earlier 'a little ahead' now exists only as the nudge, for what runs on the beat; anything driven by loudness cannot be moved earlier."
F9 SHOULD R191(i), replace: "(i) after "All Outputs Off" or Cmd+Shift+Esc, "Restore Last Outputs" brings back the screens that were on, also after you quit and start again; a screen you switch off yourself by its own line is forgotten (today the panic key makes the app forget them)."
F10 SHOULD decided 24, replace: "New rule (today nothing stops it): the Resolution cannot be changed while a recording runs, and opening a show of another picture size while a recording runs asks first (R187 b). With a screen or Syphon on, a new size is allowed and they may flash for about half a second." (Today the recorder keeps the size it started with; what it does when the size changes is UNKNOWN: cheapest, read the recorder.)
F11 SHOULD Q199 option A, append: "You can change a screen's settings only while it is plugged in." (so he knows he cannot prepare a venue at home; option B implies he can.)
F12 NIT/SHOULD R197, add (j): "\"Output Settings\" stays the name of the Composition tab's section with the Resolution; the window for the eight settings of a screen is \"Output Screens\"."
F13 NIT Q186: say "projectors and Syphon" for "outputs".
F14 SHOULD NEW reading: "An output is always a borderless window that fills one display (the main display too, Cmd+F); there is no output window you can move or size. Mine, no word of yours. Keys that stay: Cmd+Shift+Esc all off, Cmd+F the main display's output, Cmd+` brings the app over an output." (plan5 Q5 never answered; Cmd+F and Cmd+` appear nowhere in the list.)
F15 NIT R191(a): add "Brightness, Contrast, Red, Green and Blue run from -1.00 to 1.00 with 0 in the middle; Resolume does not publish its formulas, ours are the usual ones." (R54)
F16 NIT R188(a): add "the master opacity and the composition's Transform" to what a show holds (saved today: Composition.h:753, 770-774).
NITs: R191(f) add "(Resolume can show a part of the composition on a screen with slices; this build does not)"; R187(d)/R188(f) say "films" while R197(b) says "recording"; R153 does not name Fit and the size caption; Q200 B's "other programs see Audio-DNA from the start" needs the label INFERRED; the sheet's area_points map still says "questions 199 and 201" (201 is now Random; the outputs-while-hidden item is R209); R56 memory cost is only a cost line.

## 4. VERDICT
PASS_WITH_NITS leaning to a second round: no contradiction with today's code and nothing of his lost outright, but 12 SHOULDs would each cost him or a builder a second round (F1-F11, F14). Uncovered with no home at all: 6 (M5-Syphon, plan5 Q5, T2/R45, T3/R50-R51, R53 launch state, the canvas / Transform "stays" reading).
