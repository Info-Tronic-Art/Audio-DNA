# SEAT cover-actions (round 2) -- completeness of the ONE list for the area "actions (which replace routines)"
Written 2026-10-05 16:46:16 EDT
Read-only seat. Nothing built, run, probed, launched, committed. No lane worktree read. Code at main f51f32d (VERIFIED: git log -1).
Labels: VERIFIED = I read it (file:line or quoted words). INFERRED = my reading, from what is said. UNKNOWN = cheapest test named.
Short names: LIST = boris-all-items.json (final list). SHEET = area-actions.md (with its CORRECTIONS). BD = binding-decisions.md. "Qn" = question n, "Rn" = reading, "decided n" / "design n" = the n-th line of those lists in the JSON (1-based, as printed by me).

## VERDICT: FAIL (one MUST: Q180 contradicts R161 / R129(3) for a clip's own actions at a stop; 11 SHOULD; the rest NIT. Coverage itself is good: no point is wholly without a home).
Coverage: 65 points of the SHEET checked (H1-H23, part 5 = 15 points, O1-O20, M1-M7). Every point has a home in the LIST except three that are only half covered (M4 reorder, H23 default ticks, O2 bank + bands) -- they get a NEW line below. W1-W8 (the adversarial corrections) were applied before judging (W1 Loop/Once settled by 139 a; W2 undo rule already his; W3 4/4 only; W8 saved BPM is technical).

---------------------------------------------------------------------
## 1. COVERAGE TABLE -- the point -> the item that covers it
"Covers" = his answer (or his silence on the reading) lets a builder go on without guessing. PART = covers only part (the gap is named and has a fix in section 4).

### Part 4, HIS points
| Point | Covered by | Judgement |
|---|---|---|
| H1 what an action does to its sliders at start | R172 (jump to the action's value on the "1", no separate restore switch; "say so if you want a glide") + Q213 (the off side) | COVERED. Reading, not question; his older "restore" (BD:435) and "snap back" (BD:339-345) vs "smooth transition back." (BD:254) are quoted in Q213's his_words. R172 itself does not quote them (NIT 4.N1). |
| H2 which per-routine settings survive (Loop/Once, Quantize 2/4 bar, Restore, Ease/Jump, Rename) | Loop/Once: settled by 139 a (W1) and R158 ("...and loops"); Quantize: R128(b) + R158 ("one timing rule for every action"); Restore / Ease-Jump: R172; Rename: R145 | COVERED but implicit: no sentence says "an action's menu has none of today's five settings". NIT 4.N2. |
| H3 action and a signal on one slider | Q181 (A action wins, signal back when off; B signal keeps) + R195(b),(c) + R181(e) (an action recorded from a signal-driven slider replays it without the music) | COVERED by a question. Two flaws: Q181 A says "goes back to the signal" but Q213 A makes every off a jump, against today's 120 ms glide and ruling 7 (SHOULD S2); Q181 B borrows "the light of R130" whose meaning is another action (SHOULD S8). |
| H4 clip replaced while it fades out | R181(d) (old clip keeps the look until its fade ends; then back) + R161 | COVERED (reading). R129(3) says "back to before" without the timing (NIT 4.N3). |
| H5 when a layer's / global action ends; when it starts if switched on by hand | R158(c) (next "1" from its beginning, also a clip's action switched on while the clip plays); R181(a) (runs until switched off); R161 (layer X) | COVERED. |
| H6 Resync, nudge, tempo change against a running action | R144 (speed), R181(b) (a Resync moves the "1" and the action with it; sliders may hop), R177 (nudge moves the "1" for everything on the beat) | COVERED by readings. (INFERRED: the nudge case follows from R177; no sentence says "a nudge moves a running action"; he would correct R181(b) if wrong.) |
| H7 clip paused by hand / reached its end / beat stopped | R158(e),(f); R181(c); Q180 and R163 for stop | COVERED, except Q180's clip case (MUST M1). |
| H8 the rest positions that disagree (159 b / 160 a / 158 A / 156 A) | R129 (three cases), R130, R160, Q213; conflicts 5, 7, 6 quoted | COVERED. R129(2) throws his hand value away twice; it is among the "look first" readings. |
| H9 what an action can move (drop-downs, speed, in/out, macros) | Q193 (A: every slider, button, drop-down, Speed; not the clip's play / pause / backwards, not in / out points) + decided 16 | COVERED. NIT 4.N7: colour fields and text boxes not named. |
| H10 keys and pads for action buttons | R199(a),(c),(d) (key belongs to the action, not its place; held key fires once; "Stop actions"), Q207 (press or hold; hold-to-run), 166 A (default) | COVERED. The three targeting modes of today's clip pads (position / this item / selected) are not mentioned for actions; R199(a) says "belongs to that action" which settles it (INFERRED). |
| H11 two controls that do the same job (layer action's clip fires vs Autopilot; global action vs column click; macro / preset) | R203 (autopilot stays, "different tools"), R213 (autopilot's fire = a fire like yours), Q194 (hand fire vs layer action) | PART. Two actions of one layer that both fire clips, and a global action's fire against your column click, are not said (SHOULD S9). A preset load or macro against an action: not said (SHOULD S11). |
| H12 the word "ignore": layer's "Ignore Column Trigger" vs the new lamp | R181(g) (a layer set to Ignore Column Trigger is skipped by a global action's clip fires) | COVERED in behaviour. The two lamps / one word is not told to him (NIT 4.N5). |
| H13 layer bypassed / solo / hidden | R181(h) (run on unseen, in step on return) | COVERED. |
| H14 what he sees of a waiting / running / failed action | design 10, 13, 15 (button looks, a control while an action moves it, idle / waiting / missing-part) + R197(a) (hint text "ROUTINE" becomes "Action" because every "routine" you can read changes) | COVERED as pictures. |
| H15 names | R197 (a)-(i), R148, R120, R200(c) | COVERED. Default name "Action 1", "take" -> "recording" are stated there. |
| H16 select / copy / paste / cut across levels / undo | R145, R162, R181(f), Q184, Q208, R198(b) | COVERED. (Layer-action paste: R181(f) says the clip fires stay behind under 182 A, come along under 182 B.) |
| H17 undo of on / off, delete, paste | R198(a),(b) (on / off and ignore lamp are NOT undo steps; delete, cut, paste are) | COVERED. W2 applied: his own rule BD:688-689, BD:764-766. |
| H18 what a saved show remembers | R132, R163, R188(a) | COVERED. |
| H19 the saved BPM | decided 11 (W8: technical) | COVERED. |
| H20 one or several global actions; layer rows when layers change | Q179, R133(d),(e), decided 13 (follows its layer on re-order), decided 10 (delete) | PART: a global action's rows for a DELETED layer are not said (SHOULD S12). |
| H21 transition slider: place, unit, effect on a clip fire | Q183 (seconds 0-4 / beats), design 14, R160(d) (a clip fire cuts at once) | COVERED. VERIFIED today: the layer's F slider is 0 to 4, step 0.1, default 0.3 (LayerStrip.cpp:479-481), so Q183 A's "like the F slider" is true. |
| H22 hand on a MIDI / OSC control | R199(e) (yours while you turn and 250 ms after, then the action takes it; the slider jumps to the knob at your next turn) | COVERED by a reading flagged "Mine"; R199 is on the "look first" list. VERIFIED 250 ms: Composition.h:147. |
| H23 review-screen points that make an action | Q188, R137, R167, R208, Q193 | PART: which rows are ticked when the screen opens is not said (SHOULD S10). |

### Part 4, MISSED (corrections block)
| Point | Covered by | Judgement |
|---|---|---|
| M1 hand-fired clip on a layer whose action fires clips | Q194 | COVERED; wording names only "a layer's action" (S9). |
| M2 firing the same clip again; clip looping at its own length | R181(b) (never restarts the action) + R128(d) | COVERED. |
| M3 an action while a recording is replayed | R165 + R185(a) (review screen = live show stopped, keys fire nothing), decided 15 | COVERED. |
| M4 key bound to "Action N" when the row changes; new action's place; reorder | R199(a) (key follows the action; delete frees it), R134 ("you can change that order"), R197(a) (names) | PART: how the order is changed, where a new one lands, whether buttons close up (SHOULD S7). |
| M5 things moving under a layer / global action | R125 in R147 (clip moved), decided 13 (layer re-ordered: follows the layer), decided 9 (effect removed), Q182 (cell clip swapped, deck switched) | COVERED. |
| M6 preview monitor and actions | R179(b),(c) | COVERED. |
| M7 an action's button while its clip is not playing | design 15; behaviour: R158 ("play when the clip plays") | COVERED as picture; NIT 4.N6 (one sentence: an action whose clip is not playing moves nothing). |

### Part 5 (already asked, still open)
| Point | Covered by | Judgement |
|---|---|---|
| 154 what the layer shows until the "1" | Q173, R168 | COVERED |
| 160 the light, first or later action | R130 (reading, "R130 off") + R160(a) | COVERED (S8 for the Q181 B use) |
| 163 what a global action holds | R133(d), Q179 | COVERED |
| 164 where 8 / 8 / 16 sit | design 9, 11, 26 | COVERED (NIT 4.N8: only variant 3 says where the global 16 sit) |
| 169 rows: moved only or every control | R137, Q188 | COVERED |
| 170 outputs while reviewing; scrub vs draw | Q186, Q187, R138 | COVERED |
| R109 stop | Q180 | COVERED but MUST M1 |
| R111 firing while stopped / paused | R139, Q172, Q174, R174 | COVERED |
| R114 / R116 / R120 / R121 | told_in_chat (R114, R116, R120, R121), R143, R120 | COVERED |
| sweep: transition slider place / unit / trigger | design 14, Q183, R160(d) | COVERED |
| sweep: default name, rename, reorder | R197(a), R145, R134 | PART (reorder: S7) |
| sweep: where composition (global) actions sit | design 9 variant 3 | COVERED as a picture |

### Part 3 OVERTAKEN
| O | Replaced by | Judgement |
|---|---|---|
| O1 name | R197(a), R200(c) | COVERED |
| O2 pads / bank / bands | decided 12 (pads), R147(a) ("when the routine pads go") | PART: the bank of eight, the name bands on a layer's picture, the "x" on a band and the reserved band colour are named nowhere (SHOULD S6). |
| O3 pad press = Fire, no stop | R145(a) (a plain click only switches on / off) | COVERED |
| O4 routine as signal | Q181 | COVERED |
| O5 start restore | R172 | COVERED |
| O6 start on next bar / per-routine quantize | R158, R128(b) | COVERED (NIT 4.N2) |
| O7 two on one control | R182 (correction of the old page), R129, R130 | COVERED |
| O8 hand release | R129(2), 158 A, Q213 | COVERED (S2) |
| O9 how many buttons | decided by his 164; R134, design 9 | COVERED |
| O10 ignore saved | R132 | COVERED |
| O11 what a clip action holds | R133(a) | COVERED |
| O12 global and layer | R160 | COVERED |
| O13 rows | R137, Q188 | COVERED |
| O14 drawing vs scrubbing | Q187 | COVERED |
| O15 messages | Q209 (both of his sentences quoted; conflicts 18, 24) | COVERED |
| O16 bar counter | R176(g) | COVERED |
| O17 which deck | Q182, decided 10, 14 | COVERED |
| O18 pad pressed while stopped | R158(d) | COVERED |
| O19 Record tab "Save Routine" row | R184(d) | COVERED (REST /api/routine/save: R200(c) renames it but does not say whether a save by bars through REST stays; technical, NIT 4.N9) |
| O20 old Stop | R120, R199(d), Q180 | COVERED; the on-screen side is open (SHOULD S5) |

---------------------------------------------------------------------
## 2. WHAT THE LIST SAYS THE APP DOES TODAY -- compared with the SHEET's part 1 and re-read in the source
Re-checked at HEAD f51f32d by me (more than five load-bearing lines):
 (1) VERIFIED MenuBarModel.cpp:10 menu "Shortcuts"; :155-161 "Edit Keyboard Shortcuts...", "Edit MIDI Mappings...", "Stop All", "Export Bindings...", "Import Bindings..."; MainComponent.cpp:7333-7335 "Stop All" = exitAllBindingModes() (R197(i) "only closes the mapping screens": TRUE). BindingOverlay.cpp:68 and MidiLearnOverlay.cpp:95 hold the two screen headings exactly as R148(a) quotes them; MainComponent.cpp:7342, :7357 the two file-window titles.
 (2) VERIFIED R120: MainComponent.cpp:7641-7645 the learn targets "Play / Pause" (GlobalPlayPause -> applyAudioTransport, :7910-7913, the audio file) and "Stop" (GlobalStop -> routineEngine_.stopAll(), :7914-7919, "routines only"). TRUE.
 (3) VERIFIED R145(d) and "no copy / paste of clips today": MainComponent.cpp:4127-4135 Cmd+X = kClipClear; MenuBarModel.h:79-83 kClipCut / kClipCopy / kClipPaste exist only as enum values (grep of src: nothing else).
 (4) VERIFIED R198(c) "today the app still undoes a fire, B, S and X": TriggerClipCmd (MainComponent.cpp:5036, :5162) and ToggleLayerFlagCmd (:753-764).
 (5) VERIFIED R184(d) strings: RecordPanel.h:76-78 "Record Take", "Play Take", "Load Take..."; RecordPanel.cpp:98 "Render... (coming)", :123 "Save Routine". ("greyed" for Render: INFERRED, I did not read the enable flag.)
 (6) VERIFIED Q182 / decided 9 / decided 13: Routine.h:29 `deckRelative = true` (keys resolve on the ACTIVE deck at fire time; layer = recorded); Routine.h:42 positional keys. Q182's "Today's routines fire the cell of the deck that is shown" TRUE.
 (7) VERIFIED R199(e) / H22: Composition.h:147-148 gripHoldMs 250, handBackGlideMs 120.
 (8) VERIFIED R131 / R181(g): Layer.h:210 ignoreColumnTrigger; LayerInspector.cpp:155-160.
 (9) VERIFIED TopBar.cpp:39 `stopButton_.setTooltip("Stop all routines")`: today's top-bar Stop stops routines only; R176(a) says "Stop: one press; every clip leaves every layer; the beat stops". R176 sits under "These stand from earlier pages", but the TODAY behaviour of that button (routines only) is not said anywhere in the list: for the tempo-row seat; noted in S5 because it is what makes an on-screen "all actions off" disappear.
 (10) VERIFIED ClipInspector.cpp:357-361 the clip has its own blend drop-down ("Layer Determined / Normal / Additive..."): Q193's "drop-down lists (blend, ...)" in the Clip tab is TRUE.
 (11) VERIFIED RoutineEngine.cpp:458-460, :758 and MainComponent.cpp:2065-2069: today a routine start gives a notice line in the Record tab ("Routine X started", "no beat yet -- the routine starts now"). Nothing in the list tells him these lines go with routines; Q209's "four messages" will be true only because routines go. NIT 4.N10.
Contradictions with the SHEET's part 1 or corrections: NONE found in the list's own "today" lines. The one list item that sits against a verified fact is NOT a "today" line: Q213 A (jump back at once) sits against today's 120 ms hand-back glide for a slider with a signal (S2).

---------------------------------------------------------------------
## 3. HIS-OWN-WORDS CHECK
VERIFIED by a normalised substring test against BD / BL / boris-msg-raw-1..3: every string in his_words of Q179-Q185, Q193, Q194, Q213 is found. The quotes inside R130, R133, R145, R146, R160, R161 are found (truncations only). The text of the quoted entry "Save without ignored controls" (Q185, R162) is Harmony's label, not his: it is not marked as such (NIT 4.N11).
Vocabulary rule (no code names, file names, stage ids, "quantize-out", "delta"; first-message term "keyboard and MIDI mapping"): no violation in Q179-Q185, Q193, Q194, Q213, R129-R134, R143-R146, R158-R162, R172, R181, R182; the words "binding", "shortcut", "MIDI learn" occur only in R148 and R197(i) as today's titles (allowed). Option length: no option of Q179-Q185, Q193, Q194, Q213 over 45 words (script count).
The 14 "look first" readings of the intro (R129, R131, R149, R160, R168, R172, R174, R181, R186, R189, R199, R205, R210, R213) leave out stage-visible "Mine" readings of this area: see S3.

---------------------------------------------------------------------
## 4. FINDINGS (exact wording to add or change)

### MUST
M1. Q180 (A) contradicts R161 and R129(3) for a CLIP's own actions at a Stop.
 What is wrong: Q180 puts "Two clip actions and a layer's action" under one Stop. A says every action stops with the beat, buttons stay ON, "sliders stay where they stood". But Stop makes the clips leave the layers, and R161 says "a layer's X takes its clip off, and the clip's actions end with it", R129(3) says a clip's action that ends because its clip stops is "back to before". So A promises a clip's sliders stay at the action's values while R161/R129 send them back; C is in fact the R161 behaviour. He cannot tell what he is choosing; a builder gets two rules for one event.
 Fix, exact wording. Q180 situation: "A layer's action and a global action are switched on and playing, and so is a clip's action. You press stop on the tempo row: the clips leave the layers and the beat stops. (A clip's own actions end with their clip, as R161 says: their sliders go back to where they were before, at once. The question is about the layer's and the global actions.)" A: "The layer's and global actions stop with the beat and their buttons stay ON. Their sliders stay where they stood (a fader that an action had at 0 stays at 0). When you fire a clip or press play, they start again from their beginning on the "1"." B: "Stop switches the layer's and the global actions OFF, and their sliders go back to where they were before (your 159 b). You switch on again the ones you want." C: "Their buttons stay ON as in A, but their sliders go back to where they were before the actions (as in 159 b) until the beat starts again." Why line unchanged. Also in R161 add the sentence: "The same happens at a stop: every clip leaves, so every clip's actions end (question 180 is about the layer's and the global ones)."

### SHOULD
S2. Q213 A and Q181 A: a slider with a signal.
 What is wrong: Q213 A makes every switch-off a jump ("the way a slider jumps to its action", 158 A). For a slider that has a signal plugged in, "back to before" is "back to the signal", and today that return glides (VERIFIED Composition.h:148 handBackGlideMs 120 ms), by his ruling 7 "smooth transition back." (BD:254-256: "snap-to is not the behaviour he wants"). Q181 A says "goes back to the signal" without a speed; so a builder reads A of Q213 as a snap on the music.
 Fix: Q213 A, add: "On a slider that has a signal plugged in, the return to the signal glides as it does today when your hand lets go (about a tenth of a second); only sliders without a signal jump." Q181 A, replace "goes back to the signal" with "glides back to the signal, as it does today when your hand lets go of it ("smooth transition back.")".

S3. Intro line 5: "Fourteen readings hold something that you would see on stage and that is mine" is false for this area.
 What is wrong: R158 (c)-(f) (an action you switch on waits for the next "1"; no action plays while the beat is stopped; every action holds while the beat is paused; a clip you paused stops its actions), R159 (an action not a whole number of bars drifts), R161 (X and clip replacement end a clip's actions), R145 (right-click marks an action; Cmd not Ctrl; Delete asks nothing), R130 (the light), R133(e), R134 all carry "Mine:" parts he would see on stage and are not on the list. He may skim only the fourteen and write "readings ok".
 Fix: in the sentence change "Fourteen" to "Twenty-one" and add to the list: "R130, R133, R134, R145, R158, R159, R161" (the list then reads: R129, R130, R131, R133, R134, R145, R149, R158, R159, R160, R161, R168, R172, R174, R181, R186, R189, R199, R205, R210, R213). (Count 21; recount after other seats' edits.)

S5. NEW question 216 -- "all actions off" from the screen (O20; today's top-bar Stop = "Stop all routines", TopBar.cpp:39).
 What is wrong: today the top bar has a button that stops every routine. After Q176/Q180 the tempo-row Stop stops the beat and clips; R120/R199(d) keep "Stop actions" only as an entry of the keyboard and MIDI mapping. Nothing says that the one on-screen way to take everything off the sliders disappears, and he never answered whether he wants it. It is a stage panic control: what he does on stage, costly to find out in a club.
 Fix, exact question (topic D, after 213): Situation: "Three actions are on and the sliders are doing something you do not like in the middle of a song. You want every action off at once." A (default, [as today's keys]): "Only by the key or pad called "Stop actions" in the keyboard and MIDI mapping. No button on screen; the top bar's Stop belongs to the beat." B: "Also a button on screen in the global tab, "All actions off"; it switches every action off and the sliders go back (your 159 b)." C: "Also on the top bar, next to Stop." Why: "Today the top bar's Stop button takes every routine off; with the tempo row it takes the beat and the clips instead. A is what you can do with a controller; B and C add a button you can reach with the mouse." Add to the design page: "where 'All actions off' sits, if B or C".

S6. decided 12 and O2: the bank of eight and the bands are not named.
 What is wrong: decided 12 says only "the eight routine pads above the columns go". Today a routine also has: a bank of eight slots in the show file; a 16 px band with its name over the picture of every layer it drives, with an "x" that takes it off the layer; a knob it holds shows a reserved yellow-green colour with the hint "ROUTINE" (RoutineDeckView.h; UniversalParamControl.cpp:237; LookAndFeel.h:36 kRoutineCue). They go by his consent on R94 (a reading of 2026-10-04), which this list never shows him again.
 Fix, decided 12 replace with: "No carrying-over is built for old routines or old recordings. What goes when actions arrive: the row of eight routine pads above the columns, the bank of eight in the show, the name bands that sit on a layer's picture while a routine plays (with their "x"), and the "ROUTINE" colour and hint on a knob. What stands in their place is the action buttons (design 9-15); to take a layer's actions off there is the layer's "X" (R161)."

S7. R134 / M4: the order of the buttons.
 What is wrong: R134 says "you can change that order" and R199(a) frees a key when an action is deleted, but nothing says how the order is changed, where a new action lands, or whether a deleted one leaves a gap.
 Fix, add to R134 after "you can change that order": "Mine: you change it by dragging a button along its row; a new action goes to the end of its row; when you delete one, the buttons after it close up (a key or pad you had put on an action stays with that action, R199)."

S8. R130 / Q181 B: the light's meaning.
 What is wrong: R130 defines the light as "another action holds something of mine". Q181 B uses "the light of R130" for "an action never moves a slider that has a signal": the light would then mean a signal holds it. One light, two meanings, and R160's third (a global holds the layer's).
 Fix: Q181 B, replace "the action's button shows the light of R130" by "the action's button shows the same light as when another action holds one of its sliders (R130)". And add to R130 at the end: "The light means: something else holds a part of this action: another action (R130), a global action (R160), or a signal (question 181 B)."

S9. Q194 and a NEW reading: whose fire, and two firing actions.
 What is wrong: Q194 names only "a layer's action"; a global action fires clips too (R133(d)) and you can also click a column. And the list does not say what happens when two actions of one layer both fire clips (a layer's and a global one, or two layer actions on the same layer): 160 a is about a slider.
 Fix: Q194 situation: "A layer's action (or a global action) is on and fires that layer's clips in a rhythm...". New reading (D): "Mine: two actions that both fire clips on the same layer: a global action wins over a layer's (R160); of two actions of one layer the one switched on last fires, as with sliders (your 160 a), and the earlier one carries the light of R130 and fires again when the later one is off."

S10. NEW reading -- the review screen opens with nothing ticked (H23).
 Fix, exact reading (D or E): "Mine: when the review screen opens, nothing is ticked: you pick the rows or the names (a name ticks all its rows, R137). Save is greyed until something is ticked." He corrects it only if wrong.

S11. NEW reading -- a preset or a macro against an action (H8/H11).
 What is wrong: a preset load, a macro knob or a Resolume-style drop writes many sliders at once; with an action on the same sliders the list does not say who wins. INFERRED from R129(2): it is a hand-like write and the action takes the slider back at once, so the preset's look is lost. For macros UNKNOWN whether the engine counts them as a hand or a signal (cheapest: read ManualWrite.h origins; I did not).
 Fix, exact reading: "Mine: loading a preset onto an effect, or turning a macro knob, counts as your hand (R129): a slider that an action is moving jumps to the action as soon as the load is done. To keep the preset's look, switch the action off or light the ignore lamp (R131)."

S12. decided 13: a layer is deleted that a global action drives.
 Fix, replace decided 13 with: "An action follows its layer when you re-order the layers; it does not stay on the place. If a layer is deleted, a global action's rows for that layer go quiet and the rest of it plays on, as for a removed effect (decided 9)."

S13. NEW reading -- a saved action cannot be edited.
 What is wrong: his words give make (review screen), copy, cut, paste, delete, rename, and "save without ignored controls". Nothing says whether an action can be opened and changed (a value, a length, a row added). He cannot see this gap; a builder would invent an editor or none.
 Fix, exact reading (D): "Mine: a saved action cannot be opened and changed. To change one you make a new one from the recording in the review screen and delete the old one; the things you can do to a saved action are Rename, Copy, Cut, Paste, Delete and "Save without ignored controls". Say so if you want to open an action and change it."

### NIT (stay in this paper)
N1. R172: quote his two older words ("restore", BD:435; "smooth transition back.", BD:254) next to "Say so if you want a glide", as Q213 does.
N2. R158 or R145: one sentence that an action no longer has today's five settings (Loop / Once, Quantize, Restore first / Start from now, Ease / Jump): "An action has no settings of its own besides its name and its on / off button."
N3. R129(3): add "(when it ends because another clip replaced it, the old clip keeps its look until its fade-out is over: R181(d))".
N4. R172 header says "restore was ruled for routines on 2026-09-05" good; add the date "BD:435" as source.
N5. R131: add "(e2) The layer's own switch "Ignore Column Trigger" stays as it is and is a different thing: it keeps a layer out of column clicks and of a global action's clip fires (R181(g))."
N6. R158: add "(g) An action whose clip is not playing moves nothing; its button may be on."
N7. Q193 A: add "colour fields" after "drop-down list" (chroma-key colours are in the Clip tab; the list is silent on them).
N8. design 9: variants 1 and 2 do not say where the 16 global buttons sit; add "and the 16 global ones in the global tab" to both.
N9. R200(c): add "A save of an action by REST stays out: an action is made in the review screen only (your 141 a)." (INFERRED; technical.)
N10. decided (new line): "The notice lines that routines write in the Record tab today ("Routine X started") go with routines."
N11. Q185 / R162: mark "Save without ignored controls" as "my wording for the menu entry".
N12. Q182 situation and R181(f): say "a layer's or a global action". Q182 B also drops "clips it fires on another layer after a move": a clip dragged to another layer is not covered (M5): add to Q182 A "a clip you move to another layer is no longer fired by it".

---------------------------------------------------------------------
## 5. UNKNOWN (cheapest test)
U-a. Whether a macro knob write counts as a hand or a signal in the write chain (S11): read src/connect/ManualWrite.h origins and the macro path; not done (no time-box in this seat; read-only reading is allowed, I stopped at the list's coverage).
U-b. Whether the "Render... (coming)" item is greyed (R184(d)): RecordPanel.cpp:98, read the enable flag.
U-c. Everything the SHEET lists as UNKNOWN U1-U8 stays UNKNOWN; none changes a coverage verdict here.

## 6. NOTE ON MY OWN SLIP
At the start I dumped list items to q.txt, r.txt, decided_not_asked.json, design_page.json, conflicts.json, intro.json, area_points.json, told_in_chat.json, not_established.json, still_unsure.json, notes_for_harmony.json, mended_while_carrying.json, seeds.json in the shared scratchpad directory (/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/218b7246-9cc5-4850-8e9c-f9bc9435ceec/scratchpad/) and so OVERWROTE files of those names that were there (the ones with the same names as earlier dumps, e.g. q.txt, r.txt, *.json). They were dumps of the same list (a .txt twin of equal size exists for the json ones), so I believe nothing was lost; Harmony should not trust q.txt / r.txt there as the earlier version. Everything else I wrote went to the subfolder seat2act or this paper.
