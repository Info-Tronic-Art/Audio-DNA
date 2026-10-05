# SEAT 2 paper: cover-cue-layers (round 2) -- blind seat on the ONE list for Boris
Written 2026-10-05 16:46:42 by seat "cover-cue-layers". Read-only: nothing built, run, probed, launched; Audio-DNA and Resolume Arena not touched; no lane worktree read; other seats' papers not read.
Sources: boris-all-items.json (questions 172-215, readings R127-R213, decided 1-28, design 1-31, intro 1-14, notes), area-cue-layers.md (parts 3, 4, 5, CORRECTIONS), BD / BL quotes, source lines named below.
Labels: VERIFIED = I opened it (file:line or quoted words). INFERRED = from what is named. UNKNOWN = cheapest way named.
House-keeping disclosure: my first python dump wrote helper text files (intro.txt, conflicts.txt, questions.txt, readings.txt, seeds.txt, area_points.txt, told_in_chat.txt, design_page.txt, decided_not_asked.txt, still_unsure.txt, not_established.txt, notes_for_harmony.txt, arena_checks.txt, mended_while_carrying.txt) into the shared scratchpad directory, overwriting any file of those names. No script there refers to them (grep of *.py / *.js: none), so I think nothing broke; Harmony may want to re-generate them if a seat reads them.

VERDICT: FAIL (one MUST: his answer 147 a is on no part of the list and is not today's behaviour). After that one is fixed: PASS_WITH_NITS.
Points checked: 68 (H1-H22, M1-M8, A1-A5, O1-O10, T1-T12, W1-W11), plus about 30 items of the list that say what the app does today.
Points with no home: 3 (H10's bypass half, H17's blend-default half, and W3 = answer 147 a).

====================================================================
## 1 COVERAGE TABLE  (point -> item that covers it; "covered" = his answer, or his silence on a reading, lets a builder go on without guessing)
====================================================================
Key: Q = question, R = reading, DEC = "decided, not asked" line (numbered in list order 1-28), DES = design-page item (1-31), N = a new item proposed in section 3.

### Part 4, the cue system
| Point | Home | Verdict |
| H1 clicked clip vs cued layers | Q176 (A: alone; cued mix returns on any fire, a cue button, or the same name) + R150 + R152 | COVERED |
| H2 what the cued mix contains | R141 (mixed, blend modes, layer order), R149 (a,b,d,e: before fader, clip + layer effects), Q177 (global effects) | PARTIAL: master fader and the composition's position / scale / rotation are named nowhere; "full strength" vs a clip's own Opacity not said -> F5 |
| H3 bypassed / solo-hidden / fader 0 / empty / waiting clip in a cue | fader 0: R149(a); B and S: R149(c); empty: R149(e); waiting: R179(a) | COVERED for the picture; what Bypass does to the clip's playing is H10 -> F3 |
| H4 empty monitor, saved, undone, recorded | R152 (black, not saved, opens empty, never recorded, no action presses it), R198(a) (cue not an Undo step), DES 4 | COVERED |
| H5 a name-clicked clip: effects | R151 (own effects, plays, full strength), R179(c) (no actions) | PARTIAL (same as H2: opacity) -> F5 |
| H6 does a previewed clip move; phase | R151 (plays from its beginning, loops, BPM mode follows tempo, same moment if it plays in the output) | COVERED; phase is moot because every fire restarts the clip (BD:844). NIT: beat stopped / paused in the preview not said |
| H7 name click still selects | R150 (selects, does not fire; thumbnail fires; name click also previews) | COVERED (W7: the sheet contradicts itself; R150 is right and says "the app already works this way": VERIFIED ClipCell.cpp:231-239) |
| H8 where / titles / tabs / second screen | R153 (names "output monitor", "preview monitor"; the "Preview" / "Output" buttons go), R170 (main window only; second screen and Files tab not in first build), DES 2, DES 4 | COVERED |
| H9 where the cue buttons are | R141 (one per layer), DES 3, R152 (key / pad allowed) | COVERED (NIT: DES 3 does not show a show with 12 layers or a folded layer) |
| H10 cue / Solo / Bypass / fader 0 / visible / mute | Solo, B as output controls: R194(d); B and S do not change the cue: R149(c); visible / mute: notes for Harmony only | PARTIAL: what B does to a clip's playing (stops today, INFERRED) is NONE -> F3 (Q216) |
| H11 the word "cue" | R179(d), R197(f), R153 | COVERED (NIT: the chartreuse routine cue colour, W10) |
| H12 set-up without the audience seeing; column; layer meanwhile | Q172, R140, R149(f) (faders down), Q191, Q173, R168 | COVERED |

### Part 4, the layer strip and the menus
| H13 layer kind | Q204 (+ M4) | COVERED, wording to mend -> F11 |
| H14 menus: keying / blend / transition | Q203, DES 27, KP page | PARTIAL: his own two sentences ("right slider", "transparency slider for keying") not carried -> F10 |
| H15 strip transport buttons | R193(d) (same buttons as the Clip tab's), DES 26 (the three buttons may leave) | COVERED |
| H16 strip S vs Clip tab Speed | R193(e) | COVERED |
| H17 fresh layer defaults, F unit, Cut | F: R194(a) (0 = cut, seconds); Cut: Q203 A (true cut); blend default Add: NONE | PARTIAL -> F4 (Q217) |
| H18 A/B crossfader | R153 (Crossfader entry of the menu not copied) | COVERED (NIT: say "no crossfader is built") |
| H19 X and Undo | R198(a,c), R194(e) | COVERED (W5: settled by his own sentence) |
| H20 how much the strip may grow | DES 26, DES 12, DES 9 | COVERED (as pictures; W8: layer-action buttons on the strip are his words, DES 9 / 26 treat them so) |
| H21 mute / visible | notes for Harmony ("not offered in the keyboard and MIDI mapping") | COVERED as a note; nothing he would notice. NIT: W9, the note must not say they vanish: mute is saved, recorded and in the remote-control API |
| H22 two paused states | Q174 (+R193(b)) | COVERED (W6: Q174 quotes R111 itself and asks once: right) |

### Corrections block, MISSED
| M1 several cued layers or one | R141 ("any number of them can be on", said to be his and not Resolume's) | COVERED |
| M2 how long a name preview stays | Q176 A (until any fire, a cue button, or the same name again) | COVERED for A; NIT: Q176 B gives no return rule |
| M3 a column click clears layers with an empty cell | R205 (+ R168, Q191) | COVERED but the answer word is ambiguous -> F2 |
| M4 layer kind's other jobs (Per-Type Autopilot) | Q204 B names the autopilot | COVERED |
| M5 two blend lists | Q203 / DES 27 do not mention the Layer tab's list | PARTIAL -> F10 |
| M6 Layer Router Source Layer | DEC 25 | COVERED |
| M7 third clip while a fade runs | R194(b) | COVERED |
| M8 Solo on an empty layer | R194(c) | COVERED (VERIFIED CompositorEngine.cpp:1050-1068: an empty soloed layer leaves no active layer, compositeShow returns 0) |

### Part 5
| A1 keying page | Q203 | PARTIAL -> F10 |
| A2 what a layer shows while a BPM clip waits | Q173, R168(d) | COVERED |
| A3 where the ignore lamp goes | DES 12, DES 26 | COVERED |
| A4 R139-R141 never shown | now R139, R140, R141 on the page | COVERED |
| A5 which Resolume item is "the preview monitor" | R153 takes it to be the lower panel; R206(a) tests the name click, not this | COVERED as a reading he can correct. NIT: R153's bracket "its green dot is on 'Selected Clip'" reads as if it were the Preview entry; his picture (BL:925) has the dot on "Selected Clip" in the group "Monitor: Composition, Preview, Selected Clip" |

### Part 3, OVERTAKEN
| O1 two vs three opacity levels | no item; settled BD:277-281, built; interplay with the cue -> F5 | COVERED (needs no question) |
| O2 two meanings of cue | R179(d), R197(f) | COVERED |
| O3 name click showing on the output | R150 ("A name click will only select and preview") | COVERED |
| O4 panel called Preview | R153 | COVERED |
| O5 Undo and the strip | R198(c), R194(e) | COVERED |
| O6 fire while stopped / paused | R139, R175, Q174 | COVERED |
| O7 TopBar buttons | R193(d), earlier told R85 | COVERED |
| O8 a clip that leaves a layer resumes | R193(c) ("every fire starts from its beginning") | COVERED |
| O9 deck badge | none needed (built, BD:618-620) | COVERED |
| O10 docs call the keying slider working | Q203 | COVERED (doc fix after) |

### Technical points T1-T12 (architects): T1, T2 -> DEC 7, DEC 8; T3-T6 no word to him needed; T7 -> DEC 25; T8, T10 -> Q203; T9 -> DEC 16 (wording, F7); T11, T12 -> R194 (NIT: four numbers for an untouched F, not three: the Layer tab shows 0.0, LayerInspector.cpp:941 per W11).

### Corrections W1-W11 (errors inside the sheet): W1, W2 quote Harmony's consequence text as his: the list's quotes (his_words) are his own; I found no "->" text presented as his in Q172-Q215 of my area. W3 = F1. W4: R198(c) says today the app undoes a fire, B, S and X: agrees. W5-W8: see above. W9 -> H21 NIT. W10 -> NIT. W11 -> NIT.

====================================================================
## 2 THE LIST'S CLAIMS ABOUT TODAY, IN MY AREA, AGAINST THE SHEET AND THE SOURCE
====================================================================
VERIFIED here (I opened the line): PreviewPanel.h:69-70 the two buttons "Preview" and "Output" (R153); PreviewPanel.cpp:82-87 setActiveTab only recolours and repaints (R153 "show the same picture": INFERRED, CT:109); ClipCell.cpp:231-239 thumbnail fires / name selects (R150); LayerStrip.cpp:365-392 "<" "||" ">" ">|" write the clip's own flags (R193(d) consistent; HANDOFF.md:120 "do nothing" is wrong); LayerStrip.cpp:14-24 menu words "Compositing", "Alpha", "Add" (Q204 says "Add": right); LayerStrip.cpp:1099-1121 V menu = 13 keying entries then the mix modes (Q203); LayerStrip.cpp:705-712 a fade < 0 is shown as 0.3 and Layer.h:361-374 a fire with speed <= 0 makes the fade complete at once, i.e. a cut (R194(a) "shows 0.3 and cuts anyway": VERIFIED by reading, not run); Layer.h (blendMode = Additive; type = Opaque; transitionSpeed = -1) and Composition.h:476-483 makeLayer default Transparent (Q204, new item Q217); Layer.h:402-403 an empty cell clears the layer (R205: right); CompositorEngine.cpp:1038-1068 + Renderer.cpp:693-716 no active layer -> compositeShow returns 0 and the global effects are skipped (F1); CompositorEngine.cpp:1074-1081 B and Solo skip a layer before anything else (Q216 premise); Renderer.cpp:1288-1289 Layer Router: [0,1] x 9 = index (DEC 25 "one of the first ten places": right); MainComponent.cpp:5165, 5182 string "Trigger Column" (the one Undo entry R198(c) says is mended). No grep hit for any setter of Layer::type in ui / MainComponent / api (Q204 "only by editing the show file": VERIFIED by grep).
INFERRED only (labelled so in the sheet, and in the list they read as fact): Q204 "with its fader at 0 the screen shows black" (it is layer 1 that is black; Opaque draws over cleared black; code read, not run) -> F11; R194(a) "cuts anyway"; R150 "a name click ... can also put it on the main picture" (the list says "read in the code, not seen running": fine).
NO CONTRADICTION found between any list item and part 1 / the corrections, except F1 (the list says nothing; the app differs from his 147 a) and the small mismatches listed as NITs.

====================================================================
## 3 FINDINGS  (exact wording to add)
====================================================================
### MUST
F1. NEW reading R214 [topic A, next to R176] -- his answer 147 a is lost (W3).
BD:1072 "147 a" answered: "Stop takes every clip off. Some effects hold or trail a picture by themselves: Freeze holds one; Echo and feedback leave a trail. A (default) They keep doing what they do: a trail fades out; a frozen picture stays until you switch that effect off." (boris-clarify-144-147.md:13-16). It is on no item of the list. The app today does not do it: VERIFIED by reading, CompositorEngine.cpp:1040-1068 (no layer with a playing clip -> compositeShow returns 0) and Renderer.cpp:699-716 (global effects run only when sourceTexture != 0): with every clip gone the effect chain is not run and the screen is the fallback image or black at once (not run; INFERRED that nothing trails).
Exact reading: "R214 After a stop. You answered 147 a: when stop takes every clip off, an effect that holds or trails a picture goes on doing so: an Echo or a feedback trail fades out by itself, and a frozen picture stays until you switch that effect off. Today it is not so: with no clip playing the global effects do not run and the screen goes dark at once (read in the code, not seen running). It will be built with the stop (the tempo row). Say \"R214 no\" if you want the screen to go dark at once."

### SHOULD
F2. R205, last sentence -- the answer word is ambiguous.
"Say \"R205 keep\"" can mean keep today's rule or keep the layer playing; a wrong reading blanks a layer mid-show or does not. Replace the last sentence with: "Say \"R205 leave\" if a click on a column should leave a layer playing when its cell in that column is empty."

F3. NEW question 216 [B or A] -- what B (bypass) does to the playing clip (H10; sheet INFERRED, CompositorEngine.cpp:1074-1081).
Premise: today a bypassed layer is skipped before its clip is advanced, so its video stands still (INFERRED, not run). That sits against DEC 8 ("a cued layer shows the same moment as the output"), R149(c) (a cue ignores B), and R181(h) (a bypassed layer's actions run on unseen). Exact question: "216 [B] A video plays on layer 2. You press B (bypass) on that layer, wait ten seconds, and press B again. A (default) The video went on playing out of sight: when B comes off you see it ten seconds further on, as if the layer had never been off. A cue on the layer shows that same moment. [mine: it is how a bypassed layer's actions already run, R181 h] B The video stood still when you pressed B and goes on from that frame when B comes off. This is what the app does today (read in the code, not run). Why it matters: it is what the audience sees the moment a layer comes back, and whether the cue of a bypassed layer shows a moving or a standing picture." Resolume's behaviour is UNKNOWN (RE:B3.3): one B press in Arena settles it; add it to R206 if wanted.

F4. NEW question 217 [I, next to 203/204] -- a new layer's blend (H17; VERIFIED Layer.h blendMode = Additive, Composition.h makeLayer = Transparent, menu word "Add", LayerStrip.cpp:20).
Exact: "217 [I] You add layer 2 above layer 1 and fire a clip on it. Layer 2's Blend reads \"Add\": black in its picture lets layer 1 show through and light parts add up toward white, so it glows over layer 1 and never covers it. (Layer 1 itself ignores its Blend: question 204.) A (default) Layer 2 starts on Add, as today. [as today] B Layer 2 starts on Alpha, the plain blend: its picture covers layer 1 except where the picture is see-through. Why it matters: it is the first look of every layer you add; after question 203 Add and Alpha both stay in the list."

F5. R149 and R151 (and Q177's why-line) -- what the preview leaves out is only half said (H2, H5, O1).
R149(a) says "full strength wherever its fader is", R151 "at full strength", Q177 only the global tab. Not said: the master fader, the composition's position / scale / rotation, a clip's own Opacity, the layer's Feedback. Add to R149: "(g) Mine: the master fader and the composition's position, scale and rotation (Composition tab) are never on the preview, whatever you answer to 177; the layer's own Feedback is part of the layer and shows." Replace R151's "at full strength" with "without the layer's fader and without the master, but at the clip's own Opacity setting (your opacity rule: a clip's Opacity is a ceiling)".

F6. DEC 8 (and Q177's why-line) -- trailing effects in the preview (T1).
Q177 says trails "can only be approximated" under B only, but A also shows a layer's own Echo / Freeze / Feedback, which need a past picture the layer shares with the output. Add to DEC 8: "A layer's Echo, Freeze or Feedback may show a slightly different trail in the preview than in the output, because the preview is a second, smaller picture; the output is never changed by it."

F7. DEC 16 -- what a recording keeps (T9).
It says "every slider and button of the clip, layer and global tabs"; the layer strip's own controls (opacity, B, S, X, the play buttons, F) are not in a tab, and his words are "every parameter and button that can be adjusted" (BD:1089). Replace the phrase with: "every slider and button of the clip, layer and global tabs and of the layer strip (opacity, B, S, the play buttons, the fade time)". (X is an action on the layer, not a row; the cue buttons are not kept: R152.)

F8. INTRO5 -- the list of "fourteen readings ... that is mine" is incomplete in my area.
R141 (any number of cue buttons), R152 (black preview, nothing saved), R179 (edges of the cue), R194 (a, b, c: F default, third fire, empty Solo) and R193(e, f) hold stage-visible "Mine" lines and are not in it. Replace "Fourteen readings" by "Nineteen readings" and add "R141, R152, R179, R193, R194" to the list (re-count by script: other areas may add more).

F9. INTRO3 -- compound tags.
Six defaults carry "your words + mine" or "Resolume says so + mine" (Q172, Q176, Q187, Q189, Q193, Q209). INTRO3 says "all defaults good" accepts every [mine]; it is not said that a "+ mine" tag counts. Add: "A tag that ends \"+ mine\" counts as [mine]: part of that default is my own."

F10. Q203 -- two of his own sentences are not carried and one list is left out (A1, H14, M5).
(a) BD:654-657 "Should we use the transparency slider to do the work for keying elements?" is dropped from his_words; the page KP answered it (leave V as the fade; give the key its own amount). Add to Q203's why: "You also asked whether the transparency slider should do the keying: I say no. V stays the fade of the whole layer, and the key gets its own amount in K, connected to the Luma Key." (b) KP asks "confirm F is the slider you meant by 'right slider'". Add to the situation: "I take your \"right slider in layer strip\" to be F, the fade-time slider and its list of 55 transitions: say if you meant another." (c) One DEC line: "The Layer tab's Video Blend Mode list and the strip's V list become one list (today the Layer tab shows 25 names and a blank for the others: area sheet M5)."

F11. Q204 -- code names and an inferred "black".
"Opaque, Transparent, FX Only or Mask" are never written on any screen (no setter, no label: grep). Say what they do, and say layer 1 itself is black at fader 0 (INFERRED). Replace the situation's second and third sentences with: "With its fader at 0, layer 1 itself shows black (read in the code, not seen). The reason is a hidden setting, the layer's kind: layer 1 REPLACES what is below it and ignores Blend and Keying; every layer you add BLENDS over what is below; two more kinds, effects only and mask, exist and are not used by your show." Option A: "...a visible drop-down in the Layer tab (replaces / blends / effects only / mask)".

### NIT (paper only)
N1 Q176 B gives no return rule: add "It leaves when you fire any clip, press a cue button or click the name again, as in A."
N2 R151 does not say what a previewed BPM-mode clip does while the beat is paused or stopped: add "while the beat is paused or stopped it stands on its first frame, as in the output (R139)".
N3 R150: Cmd / Shift on a name selects several clips; say the preview shows the one clicked last.
N4 DES 3: add a variant or a line for 12 layers and for a folded layer (22 px row, DeckView.h:359).
N5 DES 3 / R179(d): a lit cue button must not use the chartreuse routine cue colour (kRoutineCue, LayerStrip.cpp:130, 829-835); W10.
N6 R153: the bracket "its green dot is on 'Selected Clip'" reads as if "Preview" were that entry; write: "its menu lists Composition, Preview and Selected Clip, the dot on Selected Clip".
N7 R153 / H18: add "There is no A / B crossfader in the app today and none is built."
N8 Q174 A "whatever its mode" vs R193(b) (a picture, a source, MilkDrop, the camera have no playback): write "whatever its mode (a clip that has playback)".
N9 R194(a): the Layer tab shows an untouched fade as 0.0, the strip 0.3, a fire cuts, the clock's own fallback is 0.5 (four numbers, W11); say "today the strip shows 0.3 s and a fire cuts (the Layer tab shows 0)". Label "cuts" as read in the code, not run.
N10 DEC 8 vs T2: DEC 8 promises the glitch stays in the preview; T2 says the naive result is a glitch on the output. Keep the promise only as the architects' aim ("is built so that").
N11 Notes for Harmony on mute / visible: correct as written ("mute changes nothing in the picture") but add W9: mute is saved (Layer.cpp:36, 142), recorded (RoutineSlice.cpp:132) and shown by the remote-control API, so it is hidden from the mapping, not removed.
N12 Q203 states the page's counts (20 / 55 / 43 / 5) as fact; T11 says the ruling file has 19 / 55 / 43 / 6 and calls Chroma Key suspect. The why-line already says "a later check differs by one or two": fine.
N13 R193 (d): the hidden fourth strip button ">|" (doubles the clip's speed up to 4, LayerStrip.cpp:390) is not named; DES 26 says "the three play buttons". Add "(a fourth, hidden one goes too)".
