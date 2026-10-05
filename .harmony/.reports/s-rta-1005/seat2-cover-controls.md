# SEAT 2 -- cover-controls (round 2): keyboard and MIDI mapping, MIDI pad lights, OSC, remote port, menus, tooltips, manual
Stamp: 2026-10-05 16:49:33 EDT
Read-only sweep. Nothing built, run, launched, committed. Labels: VERIFIED = read (file:line / words); INFERRED = says from what; UNKNOWN = cheapest check.
Short names: R = /Users/boriskarpman/projects/RealTimeAudio; S = R/src; ALL = R/.harmony/.reports/s-rta-1005/boris-all-items.json; SHEET = area-controls.md of that folder; BD / BL = binding-decisions / boris-feedback-backlog.

VERDICT: PASS_WITH_NITS. No MUST. Of the 27 HIS points + 9 overtaken + 11 technical + 9 corrections (about 70 checked with the part-5/6 lines), all but a handful have a home; 7 have none (below). 14 SHOULD, rest NIT.

## A. What I re-checked in source (load-bearing, before leaning on the sheet)
- VERIFIED S/ui/MenuBarModel.cpp:9-10 nine menus "Audio-DNA","Composition","Deck","Layer","Column","Clip","Output","Shortcuts","View"; :153-162 entries "Edit Keyboard Shortcuts...", "Edit MIDI Mappings...", "Stop All", "Export Bindings...", "Import Bindings...". R148(a) quotes all of them correctly.
- VERIFIED BindingOverlay.cpp:66-68 title "Keyboard Binding Mode — Click a target, then press a key"; MidiLearnOverlay.cpp:94-96 "MIDI Learn Mode — Click a target, then send MIDI"; MainComponent.cpp:7342 / 7357 FileChooser titles "Export Bindings" / "Import Bindings". R148(a) correct.
- VERIFIED MappingEditor.cpp:100 title "Mapping Editor"; APP-INVENTORY.md:104 says it is UNREACHABLE in v2 (rack hidden) -> R148's "whether it still shows: not checked" can read "it cannot be reached today (inventory)". INFERRED from the inventory only.
- VERIFIED MainComponent.cpp:7767-7771 velocityToOpacity writes the CLIP's opacity (Q207's wording right); Binding.h:82.
- VERIFIED Composition.h:147 gripHoldMs = 250 (R199 e "a quarter of a second" right).
- VERIFIED BindingManager.cpp:250-253 fromVar() clears all bindings first (R199 label "an import can erase all" right).
- VERIFIED BL:85 his words "What does this mean: Momentary pad released before its quantized beat: now cancels" -- Q207 quotes it exactly.
- VERIFIED BD:699, 712-713, 1079, 1087, 1093, 1095, 1099 match the quotations the items use (the text after "->" is Harmony's).
- VERIFIED MidiOutputHandler.h:10, 19, 68 and .cpp:153-161: the pad lights are laid out and coloured for Launchpad X / Mini MK3; a comment names APC40 only as an example. Q214's "a Launchpad or an APC" is NOT verified (see F9).
- VERIFIED ApiServer.cpp:350-354 five /api/routine/* routes and OscHandler.cpp:193-198 /audiodna/routine/{slot}: R200(c) "their routine addresses are renamed" is true for both.
- VERIFIED words he reads: grep of ALL for "binding", "overlay", "key / pad list", "key and pad list", "MIDI learn": only R148 (quoting today's strings, as it must) and Q203's unrelated "Overlay" blend. The term rule of BD:1099 is kept. "the mapping" short for the full name is used in Q207 A, Q214 B, R199 (f) (NIT 1).

## B. Coverage table (sheet point -> item that covers it, or NONE)
Covered = his answer (or silence on that reading) lets a builder go on without guessing. P = partly (the gap is in a finding).
| Point | Home | Verdict |
|---|---|---|
| U1 what can go on a key / pad / knob | Q206 (default A "every button, every slider, as Resolume says") | covered; F5 adds the ignore lamp |
| M1 output settings off keys | R191(c) + Q206 A ("only the output screens' settings stay out") | covered |
| M6 six handled-but-unlisted targets | Q206 A names Record, Snapshot; "every button" / B "Buttons" take layer play, effect bypass; faders in B | covered (C leaves them out "as today": fine) |
| U2 how the screen looks | design 30 (3 variants) | covered as picture |
| U3 the name on screen | R148 (+ R197 g, i), design 29 | covered; F11 (Export/Import files) |
| M7 editing during a show | design 30 shows the look only; behaviour of the live show while it is up not stated | NONE (F3) |
| U4 rules when you map | R199(b) (move + shows from where + every entry removable) | P (several keys per target: F11) |
| U5 hold / velocity / endless / "this clip" settings | Q207 (+ Q197 C) | covered; the third mode ("the clip playing on a layer") is not named: NIT 5 |
| U6 hold shorter than the wait for the "1" | Q207 A + R174 last line + WHY quoting BF12 | covered |
| U7 a held key | R199(c) | covered |
| U8 key on a clip after a deck switch | Q197 (A as today / B / C) + decided 28 | covered; layer buttons on re-order: NIT 6 |
| U9 what a key attaches to after copy / delete / "+" | R199(a) (+ decided 13, 14) | covered |
| U10 Cmd+C/V/X and Delete | R145 + Q208 | P (R145(d) vs Q208 A; Delete on a clip: F1) |
| U11 what the Edit menu holds | Q208 | covered; F12 (Undo names what it undoes) |
| U12 pad lights | Q214 (+ R199 end) | P (APC claim; lag; waiting not lit: F8, F9) |
| U13 what is saved with "key and MIDI settings" | R188(c) mapping; R200(a) tooltip switch | P: pad-light device and tooltip switch not on R188's list (F4) |
| U14 stop things | R120 (told), R199(d), R197(i), Q180, R204(c) "Audio play / pause" | covered |
| U15 who owns the keyboard (review screen) | R185(a) | covered |
| U16 knob vs action | R199(e), R129 case 2, Q213 | covered; knob vs signal-driven slider, endless start: F6, F7 |
| U17 tooltips | R200(a) | covered (mine; the key in the text: NIT 8) |
| U18 the manual | R200(b) | covered (format: NIT 9) |
| U19 OSC and remote port | R200(c) + Q215(a) | covered; wording clash F10 |
| U20 the word "cue" | R179(d) + R197(f) | covered |
| M2 controllers plugged in late | R199(g) | covered |
| M3 two controllers, same note | R199(g) | covered |
| M4 mapping change = Undo step? | R199(h) + R198(a) | covered |
| M5 keys while a text box is open | R199(i) | covered; Return and known fault X1: F13 |
| M8/K11 pad-light lag, waiting clip lit as loaded | none | NONE (F8) |
| O1 key/MIDI settings: show + computer + import | R188(c), R199(f) | covered |
| O2 sync / nudge on a key | R199(c) "nudge and tempo keys repeat"; Q206 A "the tempo row"; (R27 told earlier) | covered |
| O3 "key and pad list" -> "keyboard and MIDI mapping" | R148 + design 29 | covered |
| O4 routine pads / "Routine N" boxes / OSC routine | decided 12, R197(a), R200(c) | covered |
| O5 Stop / Play-Pause names | R120 (told), R199(d), R204(c) | covered |
| O6 Cmd+Z and fires | R198 | covered |
| O7 held pad and the wait (BF12 owed) | Q207 | covered |
| O8 old BORIS_DECISIONS.md lines (MIDI/OSC indicator on column, "Timed cues = Hit") | none shown to him | architects' record; VERIFIED the file still says line 26 "Timed cues = Hit (not ... cue)" and line 28 cue points stay for clip markers (consistent with R179 d): NIT 10 |
| O9 doc counts | n/a | architects |
| Part 5: drafted 180, 181, 182, R145, R148, R152 | now Q180, Q181, Q182 (the sheet's C2 renumbering), R145, R148, R152 | covered |
| Part 5: 166 A "architect's guess" | R152, R199(a), Q206 A, intro tag rule | covered |
| Part 5: BF12 explanation owed | Q207 WHY | covered |
| Part 6 (10 settled lines) | R148/R188/R198/R191(g)/Q208 agree with all ten; none contradicted | covered |
| Ignore lamp itself on a key or pad (part 2, no word) | Q206 A "every button" does not name it; B/C exclude it | NONE (F5) |
| Delete / Space / Esc / Cmd keys vs mapped keys (part 1 items 4-5) | R145(b) gives Delete to the marked action, R185 gives Space to review; nothing says what a mapped Delete does | NONE (F2) |
| K1 endless knob starts at 0.5 (first turn jumps) | none | NONE (F7) |
| K3 / K8 release of a held pad not recorded | none | NONE (F7) |
| K10 / X1 Return after a click clears a layer | none (status UNKNOWN; cheapest: read board.md row 11 / LayerStrip focus) | NONE (F13) |
| K2, K4, K5, K6, K7, K9 | architects; nothing he would meet except K4 on a 8x8 pad (none) | covered by "decided" silence; NIT 11 |
| C1 pad lights every 8th tick of 30 Hz (~267 ms) | not on page | = F8 |
| C2-C9 | sheet's own corrections; ALL uses the corrected numbers (Q180-182, 184-185) | covered |

## C. Contradictions with what the app does today (checked every J item and the items of this area)
None is a MUST. Q206 situation, Q207 (four hidden settings, velocity -> clip opacity), Q208 (Cmd+X clears, no copy / paste), R145(d), R148(a), R198 (a fire is an Undo step today), R200 (no manual, no Help menu), R204(c) all agree with SHEET part 1 and the source lines above. Two softer ones: Q214 says an APC is lit (UNVERIFIED, F9); R197(g)/R148 promise that plugging a signal into a slider is never called mapping on screen while a "Mapping Editor" title exists in source (unreachable per inventory: NIT 2).

## D. FINDINGS (exact wording to use)
F1 SHOULD Q208 + R145(d): R145(d) says "With clip cells selected, Cmd+X clears those clips, as it does today ... none is added by this", Q208 A turns Cmd+X into a cut. A reader who says "readings ok" and "208 a" has said two things. Also Q208 does not say what Delete does to selected clips.
 FIX: R145(d) ends "...as it does today (unless you answer 208 A: then Cmd+X on clips is a cut that can be pasted, and clips get copy and paste)." Q208 A add: "The Delete key empties the selected cells, as Cmd+X does today, so clearing stays one key."
F2 SHOULD NEW reading (J): keys the app keeps. Today no key held with Cmd can be mapped (MainComponent.cpp:4158), while Space and Delete can; R145(b) makes Delete a command and R185 gives Space to the review screen.
 NEW R-reading: "Mine, no word of yours covers it. (a) A key held with Cmd belongs to the app (Save, Open, Undo, Cut, Copy, Paste, the output keys) and cannot be mapped, as today. (b) Esc and the Delete key also belong to the app: Esc leaves the keyboard and MIDI mapping, Delete removes what is marked (R145). Every other key, the spacebar included, can be mapped; while the review screen is open the spacebar is its own (R185). Tell me if you have a pad or key plan that needs Delete."
F3 SHOULD R199 add (j) (M7): nothing says what the live show does while you are mapping. Today every key and pad is swallowed while either screen is open (BindingManager.cpp:54-59; the outputs keep running: INFERRED, nothing pauses the render).
 FIX (add to R199): "(j) As today, while you set up an entry the show and the outputs go on, but no key or pad fires anything until you close the screen: it is for setting up, not for playing. If the picture you choose for the screen is the real window with controls tinted (design 30), tell me and keys stay live there."
F4 SHOULD R188(d): the computer's list omits what U13 asked: the pad-light controller (chosen by hand every launch: Preferences > MIDI > "MIDI Output:", PreferencesDialog.cpp:119-141, not saved: AppSettings.h:18-19) and the tooltip switch that R200(a) says is remembered.
 FIX: R188(d) add ", the controller whose pad lights you chose, and the switch for tooltips".
F5 SHOULD Q206 A: the ignore lamps (BD:1056-1058 "every slider, button, everything") are not in A's list, and B / C leave them out.
 FIX: Q206 A: "...(a clip, a column, an action, a cue button, an ignore lamp, the tempo row, Record, Snapshot)..."; B and C: add "the ignore lamps stay on the mouse".
F6 SHOULD R199(e): the knob rule names an action only. A slider a signal drives meets the same knob (Q181 A: "the action wins as your hand does while you hold a slider").
 FIX: R199(e) "...a slider that an action or a signal moves is yours while you turn the knob and for a quarter of a second after, then the action or the signal takes it back (158 A)..." (INFERRED: today's grip hold of 250 ms applies to every write).
F7 SHOULD decided-not-asked, two lines (K1, K3/K8, both quiet failures on stage): (a) "An endless knob starts from where the slider stands, so the first turn does not make it jump (today it starts from the middle and jumps)." VERIFIED BindingManager.cpp:173 per SHEET K1. (b) "A pad you hold: its release is kept in a recording like its press (today the release is not kept: a held pad's clip would never let go when the recording is played)." VERIFIED per SHEET TODAY 18 / FQ:95.
F8 SHOULD decided-not-asked (C1, M8/K11): "Pad lights follow a press up to a quarter of a second late (today the app asks every 267 ms) and a clip waiting for the "1" is lit like a loaded clip, not as waiting; both are mended when pad lights are built." VERIFIED MainComponent.cpp:439, 4372, 4416; MidiOutputHandler.cpp:64-77 per the sheet. And Q214 A: add "(a press shows on the pad a quarter second late today, and a clip waiting for the "1" is not lit as waiting)".
F9 SHOULD Q214 situation: "Today the app can light the pads of a Launchpad or an APC" -- only Launchpad X / Mini MK3 is verified (MidiOutputHandler.h:19, 68; .cpp:153-161). FIX: "Today the app lights pads laid out for a Launchpad X or Mini MK3: the grid shows the clips of the deck on screen. Another controller gets the same notes and may show wrong cells or colours. I do not know which controller you play with."
F10 SHOULD Q215(a) vs R200(c): Q215 A says "none of the six is built or changed", R200(c) renames the routine addresses of OSC and the remote port (VERIFIED ApiServer.cpp:350-354, OscHandler.cpp:193). Also "remote control from another program" names two things (OSC; the app's own remote-control port that test tools use).
 FIX: Q215(a): "(a) OSC, messages from a controller program (its routine address becomes an action address, R200 c; nothing else changes)". R200(c): "OSC (messages from a controller program) and the app's own remote-control port (which test tools use) stay as they are..."
F11 SHOULD R199(b)/(f) and design 29: (1) nothing says a button may have a key AND a pad (today allowed, the screen shows only the first, BindingOverlay.cpp:258-302). (2) design 29 variant 1 keeps "Export Mapping..." / "Import Mapping..." files, but your answers 81, 83, 85 are about importing from a show (BD:879-897) and no word of yours asks for files.
 FIX: R199(b) add "A button can have a key and a pad both; the screen lists every one of them." R199(f) add "Today's Export Bindings and Import Bindings files go: you import from another show." design 29 variant 1: drop "Export Mapping..." and name the import "Import Mapping from Show...".
F12 SHOULD Q208 A: his own words are that "Undo Remove" is read in the Edit menu (BD:699, 712-713); today the item names what it undoes ("Undo Remove Deck", MenuBarModel.cpp:32-68). FIX: Q208 A add "Undo and Redo name what they undo, as today ("Undo Remove Deck")."
F13 SHOULD decided-not-asked (K10): R199(i) says Return gives the keyboard back, while the filed bug X1 (board row 11, HIGH: after a click on a clip cell, Return clears the top layer) is still on the board; whether bf9b fixed it is UNKNOWN (cheapest: read board.md row 11 / LayerStrip focus, or ask Harmony). If still open: "A button you clicked never takes Return or Space as a press; the fault where Return clears a layer after a click is mended in the mapping build."
F14 SHOULD R152 / R145 / R185 / R198 are "Mine" and seen on stage but are not in the intro's list of fourteen (intro line 5 lists R149, R160, R168, ... R199 ... R213). FIX: add R145, R152, R185 and R198 to that list (and say "eighteen").
NITs
N1 NIT "the mapping" in Q207 A, Q214 B, R199(f), design 29 variants: R148(c) says it is always written in full -> "the keyboard and MIDI mapping" (or "your mapping" in the singular entry sense "an entry of the keyboard and MIDI mapping").
N2 NIT R148 label "whether it still shows on screen: not checked" -> "it cannot be reached today (inventory, APP-INVENTORY.md:104)"; R197(g) then needs no promise about a title nobody sees.
N3 NIT R200(a): "the switch for tooltips is remembered" and tooltips in Preferences "Show Tooltips:" resets to ON each launch today (VERIFIED by sheet 27): say "today it resets at every launch".
N4 NIT R199(d): "Stop actions switches every action off at once": add "(their sliders go back, as R129)" so it is not a second meaning of "off".
N5 NIT Q207: the third targeting mode (the clip playing on a layer; docs wrongly say "the UI selection") is not offered; add to decided: "A third way of aiming a pad, 'the clip playing on that layer', stays in the app unseen."
N6 NIT R199(a): a key on a layer's own button (Bypass, Solo, Mute, X) stays on that place in the stack when layers are re-ordered, unlike an action (decided 13) -- INFERRED from today's layer-index targets; say so in one line.
N7 NIT Q207 WHY: "hold" on a column or deck pad is not mentioned; say "Hold applies to clips and action buttons only".
N8 NIT R200(a): tooltips of shortcuts show their key ("Cmd+Shift+Esc", as the Outputs button does today, TopBar.cpp:276).
N9 NIT R200(b): the manual has no stated form (a page in the app, a file beside it); say "its form comes with the pictures".
N10 NIT the old notes file R/BORIS_DECISIONS.md (header 2026-05-22) still says "MIDI/OSC indicator always visible on column" (line 84) and AUTO / OVERRIDE: list under decided-not-asked "Old notes of May that say otherwise are void where this page differs."
N11 NIT K4: pad-light notes collide from the 11th column; a Launchpad is 8x8 so invisible; architects.
