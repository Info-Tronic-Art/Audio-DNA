# SPEC K -- Sources and the automatic features (s-rta-1007): the paper apply-K.md with the ruling rule-K.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 211
TITLE: Many pictures dropped on a cell: one clip, plus time per picture
STATUS: DEFAULT option A, by his first line
HIS: L1
RULE: Option A, the default he took: pictures dropped together on a cell make one clip, and one thing is new. In full: one picture dropped makes one clip; exactly two make two clips side by side; three or more make ONE clip that runs through them in the order of their file names, a number inside a name counted as a number. A folder itself is not dropped: its pictures are selected and dropped together, and that is how a folder of pictures gets into a cell once the Image Folder button is gone (R228). The clip has playback like a video: Timeline mode or BPM mode, Speed, pause, backwards, the loop menu and the start menu, as topic H rules them; it comes into the show in Timeline mode (topic H's H-15), where it runs at a set number of pictures per second. In BPM mode it has two ways, switched on the clip: (1) Spread: all its pictures are spread evenly over the clip's Beats, like a video; a clip has this until he switches. (2) Time per picture, new: every picture stays for the same whole number of beats and the picture changes on a beat; while it is on, the clip's Beats row counts the beats of ONE picture, its "-", "+", "/2" and "x2" work on that number, and the clip's whole length is that number times the number of pictures (K-7). In both ways the clip, being in BPM mode, starts on the "1" like every BPM-mode clip (L11, L12), and at its end it does what its loop menu says. Copy, paste and Option-drag work on it as on any clip (L4, L5). The separate row at the top (Image Folder, Beats per Image) goes (R228); the steady change of picture on the beat lives in the clip from then on.
CHANGED: nothing (he did not name it; "All defaults good except for these", L1). Tested against L21 and L99 (a BPM-mode clip counts in beats and has "-", "+", "/2", "x2"): that shapes how the new choice is set, which the page gave only as "a beat, a bar, 2 bars" (K-7). Added by the ruling, none of it a change of the default: the order of the pictures and the folder sentence (both are the default's "as today"), and the pointers to topic H for playback and for the end of the run.
TODAY: sheet TODAY 2, 3 and H12, H13: 1 picture = 1 clip, 2 = 2 clips, 3+ = one sequence cell (MainComponent.cpp:843-880, re-read by the ruling; its comment names a ruling of his of 2026-08-04 for the two-picture case, which is not in binding-decisions.md); the pictures of a sequence clip are sorted by file name in natural order (MainComponent.cpp:5633-5636 and ImageSequence.cpp:44-47, read by the ruling); a cell's drop test takes picture and video files only, so a folder is refused (ClipCell.cpp:311-323, read by the ruling); in BPM mode the sequence is spread over beatDivision beats (Clip.h:34-41 by the sheet); the per-picture rhythm exists only in the old top row (MainComponent.cpp:455-467 by the sheet) and feeds only the old picture slot. To change: a Time-per-picture switch on the sequence clip in BPM mode, and the Beats row counting one picture while it is on; the old row goes with R228.
@@END

@@ITEM 212
TITLE: Seven things that do nothing: later, all seven
STATUS: DEFAULT option A, by his first line
HIS: L1
RULE: Later, all seven. The coming builds do not touch them and they stay as they are, neither removed nor hidden nor built: (a) the camera (the Camera list in the top row and the Camera Input entry in the Sources tab); (b) Import ISF Shader; (c) the genre the app detects; (d) what happens on a detected drop or breakdown; (e) the smart autopilot; (f) the Autopilot block of the tab that is now called Global (L55); (g) the randomizer of effects. After the builds they are gone through one by one, each with its own questions to him; in the order of work that is after the six planned things of 215 (K-20). Which message boxes of these seven may still appear (the box after an ISF import, the camera's failure box) follows topic J's item 209, not this item. Nothing in his message asks for any of the seven sooner.
CHANGED: nothing. Tested against his whole message: no line names a camera, ISF, genre, a drop, a smart autopilot or a randomizer; L55 only gives the Composition tab its name Global; L126 keeps the one live reaction to a drop (MilkDrop's Jukebox) untouched. Added by the ruling: the pointer to item 209 for the message boxes; the order of work now points at K-20.
TODAY: sheet TODAY 5, 6 (camera), 12 (ISF phantom), 13, 14 (genre), 15 with C1, C2 (structure: only a log, plus the Jukebox and the ruled oscillator switch), 18 (smart autopilot never runs), 17 (the block is read by nothing), 19 (randomizer hidden, aimed at the old chain); H1-H6, H9; M6 (the ISF and camera message boxes). Earlier instruction binding-decisions.md 99 (Harmony's consequence text: wire dead UI up, do not hide it) agrees with "later, not removed".
@@END

@@ITEM 215
TITLE: Six things: all are planned
STATUS: ANSWERED in his own words: all six
HIS: L128, L110
RULE: His words: "215 plan all of these". All six go into the plan of the app, none is left as it is. That is read as: they get built (K-1), after the first builds and before the blend lists, which he puts last (L110: "this is built after all other parts are done. This is last, before the ui redesign which is the final change"); the order is Harmony's (K-20). None of the six is built before its own plan is clear. What each plan starts from: (a) OSC: every button and slider that can be put on a key or pad can also be driven by an OSC message; the app receives OSC and sends none (K-13); the addresses that say routine say action (topic J's reading on OSC); still to plan: fixed addresses or addresses that travel in the mapping files he wants (L116), the port and where it is set. (b) Ableton Link: first a look-up, not yet done, of whether his copy of the app can be built with Link and under which licence terms a closed app may carry it; with Link on, the tempo bar does what topic A's reading on Link says (play, pause and Resync greyed out, a fire never moves the beat, stop only clears the layers), and "/2", "x2" and the nudge act inside this app only (K-14); still to plan: a Link tempo outside his range of 22 to 480 (L14), and where the switch sits (L8). (c) Picking the audio input in the app: a list (Audio Input) that holds only wired inputs and the built-in microphone, never a Bluetooth device (his standing rule, binding-decisions.md 485-488); the computer remembers the pick, for every show (K-10); when the picked input is missing at the start or is pulled out, the app listens to the built-in microphone without a word (R204 b) and goes back to the picked input when it returns (K-22); still to plan: which channels of a many-channel input are heard. No message appears by itself (topic J's item 209). (d) An audio file that loops: an audio file played in the app starts again at its end; one switch, Loop File, turns that off, and it is on at every start of the app (K-21); the tempo bar never starts or stops the file (K-11); the file, the input mode and the gain are remembered nowhere, as he was shown in topic F's reading on what a show keeps. (e) A text source where words are typed: a new source, Text; the words are typed on its Clip tab and saved with the clip; its settings connect to signals like every slider (his rule, binding-decisions.md 106-109); the source that draws digits stays under a new name (K-12); still to plan: fonts, one line or several, how the words move. (f) Render: a film file made afterwards, in Review, from the recording that is open there (the original or a mend of it: L84), the whole recording, at the size of the full composition (L88, said of the recording), with the recording's sound (K-2). It is not made in real time, and it shows what Review shows of that recording, no more exactly than Review does (topic E's item on the review picture and its assumption E-38). What "All else is gone" (L114) takes away is asked once, in topic J (its J-1); Render follows that answer.
CHANGED: Option A (THE DEFAULT, "None of the six is built or changed now") is replaced by his words; option B (name the letters) is answered with all six. The lines of other readings that said "stays as it is" for these six fall with it: R204 a and c (this slice), the pointer in R201 d (this slice), and in other slices the readings on Link, on the Record tab's Render and on OSC. Changed by the ruling: Render no longer stands as a question against L114 (L128 names it; topic J's J-1 asks what L114 removes); the promise that a Render equals the night is taken out (nobody has measured it; topic E's E-38); the show remembers nothing of the audio setup (topic F's reading R188 e, which he read through L94); the picked input is kept by the computer (K-10).
TODAY: OSC input on UDP 8000 with 15 address patterns, one of them /audiodna/routine/{slot} (area-controls.md TODAY 25-28, by topic J's paper; not re-read). Link: his copy is built without it and the switch is dimmed; with Link on the app follows Link's tempo only, never its beat position (area-tempo.md part 1 point 10, M-3, by topic A's paper). Link's licence for a closed app: NOT looked up (no web tools in this run; a researcher's job; from memory, UNVERIFIED: Link is offered under the GPL or under a separate licence from Ableton). Audio input: no picker, follows the Mac's default or the built-in microphone (sheet TODAY 23, H14); nothing of the audio setup is saved (TODAY 24, H15). Audio file: no looping call in the code, plays once (sheet M3, INFERRED there). Text Animator draws random seven-segment digits (sheet C4, EmbeddedShaders.h:11944-11975 by the sheet). Render: a greyed "Render... (coming)" on the Record tab (area-recording.md AR 1.1, by topic E's ruling); his September words binding-decisions.md 220-224, and sound in the film file approved at 235 ("do this but triage the correct build order.").
@@END

@@ITEM R201
TITLE: Sources stay; symbols and pictures come in the UI step
STATUS: CORRECTED part b (L124) and the pointer in part d (L128); a and c stand
HIS: L124, L128, L12
RULE: (a) Stays: the Sources tab with its generated pictures and fractals, each with its own sliders; MilkDrop with its own browser; still pictures; videos. (b) His words: "we will need symbols and pictures that make sense for everything and this will be done in the UI step". Every thing on screen, a source's cell included, gets a symbol or picture that makes sense, and that work belongs to the UI step, not to the coming builds, which leave the cell of a source as it is. (c) Two clips of the same kind of generated picture keep their own slider values. The coming builds do not give each clip a running state of its own: a picture with a memory (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics) that plays on two layers at once goes on disturbing itself, and all MilkDrop clips go on showing one and the same preset (MilkDrop is not touched: L126). What that means for the preview monitor is topic B's (its assumption B-11). (d) The source that draws rows of digits stays; a source where words are typed is planned (215 e, L128; K-12). (e) As clips, a still, a generated picture and MilkDrop follow the rules of every clip: fired while the beat is stopped they start it (L12: "any clip"); they have no playback of their own, the tempo bar's pause does not freeze them, and fired during a pause they show and move (topic H's reading R193 b, shown to him and left standing; the rule behind it, that pause holds only what follows the beat, is asked once, in topic A: its A-3).
CHANGED: b: the page said "whether it gets a picture comes with the pictures"; his words move it, and symbols and pictures for everything, to the UI step. d: its pointer "a real text source: question 215 (e)" is answered: planned (L128). a and c: nothing in what they decide; c is reworded so that it says what the builds leave, not how the app is. Added by the ruling: part e, from L12 and from topic H's reading, in place of the two assumptions the paper asked.
TODAY: sheet TODAY 7, 8 (a source's cell: gradient, tag "SRC", the type name; ClipCell.cpp:27-47 by the sheet), 9 and T1 (one instance per source type, Renderer.cpp:1160-1171 by the sheet: two clips of one type share state and overwrite each other's values while drawn), 10 with C4 (digits), H11, H19, H20, H21; MilkDrop: one engine for the whole show, every MilkDrop cell a window onto it (milkdrop-current.md 1.1).
@@END

@@ITEM R211
TITLE: The ten numbered preset slots and three old buttons go
STATUS: STANDS
HIS: none
RULE: The ten numbered preset slots in the bar along the bottom of the window (each a number button with a drop-down) and the three small buttons Save, Load and FX Save in the top row go, as he ruled earlier (86). Nothing takes their place: a column is his scene, and each effect has its presets. The show's own Save, Open and Collect Media stay. The app's description drops the line about instant preset save and recall.
CHANGED: nothing. Tested against L48, L50, L52 (effect presets: they travel with the show and sit in the effect's header line) and L94 ("Do we need snapshot?", about the window layout): none speaks for a whole-set-up recall in place of the slots.
TODAY: sheet TODAY 20, part 2 items 5, 6, O3, H18: the slots load a file into the OLD effect chain, never into a clip (MainComponent.cpp:384-395 by the sheet); removal is on paper in stage S7 of the one-save plan, not merged; CLAUDE.md still lists "instant preset save/recall".
@@END

@@ITEM R228
TITLE: No clip plays: the output is black; the old top row goes
STATUS: STANDS with one doubt against L126 (K-3)
HIS: none
RULE: When no clip plays, the output is black, apart from an effect that holds or trails a picture after a stop (his 147 a; topic A's item on it) and apart from the camera of part b. (a) No loose picture reaches the output: Open Image, Image Folder with its list Beats per Image, a picture dropped on the window outside the clip grid, and a click on a preset in the MilkDrop tab (its "<", ">" and "?" buttons the same) do not put a picture on the output by themselves. The two buttons and the list go, together with Save, Load and FX Save (R211); a picture dropped outside the clip grid does nothing; a click in the MilkDrop tab changes the preset MilkDrop shows, and MilkDrop reaches the output only through a cell that plays. That last point is the one place where this reading meets L126 ("that's it for this upcoming build"): K-3, asked. A click on a clip's name or a double-click in the files window shows in the preview monitor, never on the output (L35; topic B). (b) The Camera list stays where it is and keeps showing its camera only while no clip plays, until the camera is decided (212 a: later).
CHANGED: nothing by a line of his that names it. L35 confirms the direction (a name click and a files double-click go to the preview). L126 is tested against the MilkDrop click in part a: kept as shown, with K-3 for him. Added by the ruling: that a click in the MilkDrop tab still changes the preset, that a picture dropped outside the grid does nothing, and the camera of part b named as the second exception to black, which the page's own part b implies.
TODAY: sheet TODAY 5, 28, O1, H1, H13: the Renderer draws "deck compositor > active source > loaded image" (Renderer.cpp:690-735 by the sheet), so the old single-picture slot (last opened image, image folder, a pick in the MilkDrop tab, the camera) shows whenever no layer plays; row 1 controls MainComponent.cpp:2749-2767 (by the page). A HAND pick in the MilkDrop tab (a row click, "<", ">", "?" and, by that paper's wording, the random start that Play makes) makes MilkDrop the loose picture, and the next fire or clear takes it away again; the Jukebox counts only while a MilkDrop picture is drawn (milkdrop-current.md 1.4 last point, 1.10 point 9, 2.4; the pick's call that makes MilkDrop the loose picture and the draw order were read by the ruling too: MainComponent.cpp:1724-1738, Renderer.cpp:688-720; how long the loose picture lasts is INFERRED in that paper; nothing run). The slot itself stays in the code: the Camera list of part b uses it, and so do the remote-control and test calls that load a loose picture (milkdrop-current.md 1.9; Pitfalls 28, 47): K-23.
@@END

@@ITEM R202
TITLE: MilkDrop: a document now, nothing changed, a session later
STATUS: CORRECTED parts c, d, e and the pointer to the labels (L126)
HIS: L126, L4, L5, L12, L13, L35
RULE: His words: "we will design a much smarter system for doing Milk drop and we will do that as a dedicated session where I will design the UI and how we will use it but not right now. I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build." So: (1) the coming build changes nothing of MilkDrop itself: its browser, the three ways of changing presets (the Jukebox, a clip's own playlist, by hand), Random and Bag, favourites, both Blend sliders, the reaction to a build-up, drop or breakdown while the Jukebox runs, and how a clip's playlist is saved all stay exactly as they are. (2) The one MilkDrop deliverable of the coming build is the document that says how MilkDrop works at the time of writing (milkdrop-current.md, written and re-checked in this session). (3) The smarter system is designed in a session of its own, where he designs the UI and how it is used; the document is that session's ground. Carried into that session and not built before it, because each stood in the reading he answered with "that's it": the playlist's Blend slider that does not fade (c), a switch for the change on a drop or breakdown (d), whether a clip's preset files travel with the show (e; until then they do not), and the labels of the two timing lists (R212). (4) A MilkDrop clip is still a clip in a cell, and what his words rule for any clip holds for it: copy, paste and Option-drag (L4, L5); fired while the beat is stopped it starts the beat (L12: "any clip"); the tempo bar's stop takes it off its layer like every clip (his answer to 126 and 127); a click on its name previews it (L35), within the limit topic B names (its B-11). None of that changes how MilkDrop makes or changes its picture. (5) What the tempo bar does to the beat reaches MilkDrop only through the beat it counts: while the beat is paused or stopped the Jukebox and a clip's playlist do not move on (topic A's reading on what stands still, shown to him and left standing; L13), and the picture goes on moving to the sound. The one open meeting point is a click in the MilkDrop tab while no clip plays: K-3.
CHANGED: c: "is made to fade" is removed from the coming build (no MilkDrop change, L126). d: "say so if you want a switch for it" is not answered now; it goes to the MilkDrop session. e: "say so if the preset files of your clips should travel with the show" likewise. The last sentence ("What changes in the labels of the Jukebox list: R212") is overtaken in time (see R212). a and b: nothing, they stay. Added by the ruling: part 4 with its lines (in place of an assumption the paper asked) and part 5 (how the new tempo bar reaches a MilkDrop that is not touched: INFERRED from L13 and topic A's reading).
TODAY: sheet TODAY 11 with C1, C3, M1, M2, M5, H10. The full account is milkdrop-current.md (written 2026-10-07 22:53, re-checked: check-milkdrop.md): one engine for the whole show (1.1); a cell's saved preset is not loaded when the cell is fired (1.10 point 1); the Jukebox and a cell's playlist change presets only while the beat runs, and the picture moves to the sound (1.6, 1.7); the playlist's Blend number is saved and not used (1.10 point 4); a cell stores full file locations (1.10 point 12).
@@END

@@ITEM R212
TITLE: MilkDrop's timing labels in bars: rule stands, work waits
STATUS: REPLACED in time only, by his L126
HIS: L126
RULE: His earlier rule stands as a rule: MilkDrop's timing lists read in bars and carry no seconds (his "bars", binding-decisions.md 675; "I only need beats", 649). By his L126 ("that's it for this upcoming build") the lists are not touched in the coming build: the pointer to this reading stood inside the MilkDrop reading he answered with those words. The relabelling (the Jukebox list reading 4, 8, 16, 32 and 64 bars; a clip's own playlist list reading 1, 2, 4 and 8 bars; the picture changing as often as before) is carried into the MilkDrop session and done with the smarter system. The playlist's triggers on the music's structure stay without a control until that session.
CHANGED: The whole reading was "Mine: the list reads what it does"; what it says is kept, WHEN it is done changes: not in the coming build (L126). INFERRED from "that's it"; he did not name R212 itself, and the consequence of a wrong reading is a few words in two lists. No line for him: the paper's assumption on the wait is dropped.
TODAY: The Jukebox list reads "4 beats", "8 beats", "16 beats", "32 beats", "30 sec", "60 sec" and waits 4, 8, 16, 32, 32 and 64 bars; the playlist list reads "4 beats" to "32 beats" and counts true beats (milkdrop-current.md 1.5; sheet C3: MilkDropBrowser.cpp:563-575, PresetSelector.cpp:169, MilkDropBrowser.cpp:611-615 by the sheet). The adopted bars lane (bf7) holds this relabel: it must be taken out of the lane's packet for the coming build.
@@END

@@ITEM R203
TITLE: The autopilot stays, beside actions
STATUS: STANDS
HIS: none
RULE: Stays: (a) a layer's autopilot: the next clip after a count, or at the end of a video; in order or at random; at the end of the row it starts again from the first clip; at random it may return to the clip it has just left. The row it walks is the row of the deck its playing clip came from, whatever deck the grid shows: looking through decks never changes what the autopilot brings next (his words of 2026-10-02: "I can switch between 20 decks looking for a clip and the playing will not be affected"). (b) A clip's own list of what comes next. (c) The count per kind of layer, for as long as layers have kinds: whether they keep them is asked in topic I (its I-1, after his L111), whose main reading is that kinds go and this count with them; nothing is taken out before his answer. The autopilot and actions are different tools and both stay. Where his new rules meet the autopilot: R213.
CHANGED: nothing replaced by a line of his. Added by the ruling: (c) is made to hang on topic I's I-1 (L111 took no letter of question 204, whose option B removes the count per kind of layer); the deck sentence, from his words of 2026-10-02 (binding-decisions.md 598-600). L98 agrees with the reading and decides nothing in it: "If the clip plays on autopilot and the next clip plays then the eject would be cancelled by the next clip appearing." Tested against L7, L72 and L73: none speaks of the autopilot.
TODAY: sheet TODAY 16, 17 with C5, M7 (wraps with the number of columns, skips empty cells, random may go back, nothing on a lone clip; Autopilot.cpp:243-282 by the sheet), H7; the Per-Type Autopilot of the Composition tab is wired and off by default (TODAY 17, Pitfall 23); one autopilot for the show advances a layer within the deck its playing clip came from, never the shown deck (docs/claude/effects.md:79, read by the ruling).
@@END

@@ITEM R213
TITLE: The autopilot under his new tempo rules
STATUS: REPLACED in one clause of part a by L102 (the start menu); the rest stands, carried by L11, L12, L13
HIS: L102, L11, L12, L13, L98
RULE: (a) A fire by the autopilot is a fire like his own. A video starts from its beginning unless the clip's start menu says otherwise (L102: "model these 2 little menu’s after resolume"; topic H's item 202). A clip in BPM mode waits for the "1"; a clip that is not in BPM mode comes in at once (L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"). The clip's actions that are switched on start with it, as at any fire; what a layer's switch against actions does to them is topic D's (its D-2, on L7 and L73). (b) The autopilot counts on the running beat: while the beat is paused or stopped it does not move on (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."), and it never starts a stopped beat: that takes a press of his own (L12; topic A's D1 and A-13). (c) Its counts read in bars, as every list of lengths (the reading on bars and beats, shown to him and left standing; his rule of 2026-10-03, binding-decisions.md 646-648; L99 is about the length row of a BPM-mode clip only, which that rule itself counts in beats). The counts on offer are six lengths: 1 beat, 2 beats, 1 bar, 2 bars, 4 bars and 8 bars; how the two that are shorter than a bar are written follows topic I (its I-20, on L83). (d) A BPM-mode clip the autopilot brings in starts on the next "1" and its count starts there, so a count shorter than a bar still changes on a "1"; until that "1" the layer shows what a fire of his own shows while it waits (topic A). (e) While the beat is paused or stopped the autopilot does nothing at all, End of Video included: a video that ends then stays on its last frame until the beat plays, and the next clip comes then; a clip set to Play Once and Eject waits the same way (K-16). When the autopilot's next clip appears, a pending Eject is cancelled (L98).
CHANGED: a: "a video starts from its beginning" gains "unless the clip's start menu says otherwise" (L102, his words elsewhere: every fire obeys the start menu that topic H builds). The rest: nothing by a line that names it; a, b, d are his own tempo words applied to a fire the app makes (L11, L12, L13: INFERRED, his lines speak of his own press). c: the paper's question whether the counts read in bars or beats is dropped (settled by the standing readings; L99 names the clip's row only). e: the meeting with Play Once and Eject during a pause is not in the page or in his words (K-16, internal).
TODAY: sheet TODAY 16 with C5, H8, T9: the on-beat autopilot advances only on beat crossings (Autopilot.cpp:74-76) and needs clip->playing (:100); End of Video runs regardless of the beat (:40); an autopilot fire resumes a clip where it was and brings it in at once (the page's reading, by the area sheets); counts read "1 Beat" to "32 Beats" (LayerInspector.cpp:43-137 by the sheet); an autopilot fire and a hand fire go through the same fire path, so the origin of a fire has to be told apart for part b (facts-takes-bpm-fire.md section 3 line 50, by topic A's paper). To change: the start menu and the wait for the "1" at an autopilot fire, the hold of End of Video while the beat does not run, the labels.
@@END

@@ITEM R204
TITLE: The sound the app listens to
STATUS: REPLACED in parts a and c (L128) and in the pointer of part d (L8); b stands
HIS: L128, L8
RULE: (a) The app listens to a wired input or the built-in microphone, never to a Bluetooth device (his standing rule). Until the planned pick is built it follows the Mac's own choice; once built, the input is picked from a list inside the app (215 c; K-10). (b) If a wired input is pulled out during a show, the app goes on listening to the built-in microphone, without a word. (c) An audio file can be played instead: it starts when he chooses it and is heard on the Mac's output; once the planned loop is built it starts again at its end (215 d; K-21). The tempo bar's play and pause never start or stop it, as he was shown; that its stop does not stop it either is Harmony's reading of L72, shown to him as a line (K-11). "Audio play / pause" in the keyboard and MIDI mapping starts and stops it. (d) What he sees of the sound stays: the waveform, the signal meters and the tempo. The band meters, the dB level, the onset flash, the genre line and the spectrum stay off screen; whether and where they come back is settled in the UI redesign (L8, L124). (e) What is remembered: the show holds nothing of the audio setup, and the input mode, the file and the gain are remembered nowhere, so the app opens on the microphone input, as he was shown in topic F's reading on what a show keeps; the one new thing, the picked input, is kept by the computer for every show (K-10).
CHANGED: a: "there is no list of inputs in the app" is replaced once 215 c is built (L128). c: "plays once" is replaced once 215 d is built (L128). d: "is for the pictures" now reads: in the UI redesign (L8). b: nothing. The stop: the page spoke of play and pause only; L72 ("The tempo stop button stops all actions as well as everything else.") is read as everything the stop already takes (the clips, the beat, what the beat drives), not the audio file: K-11. Added by the ruling: part e, so that this reading and topic F's say the same.
TODAY: sheet TODAY 22-26, M3, M4, M8, H14-H16: an input list with "Mic Input" and "Audio File" only; the device is the macOS default if the Bluetooth guard allows it, else the built-in one (AudioEngine.cpp:24-26 by the sheet); the file plays through the app's output too; no audio field is saved in the show or the settings; the readout panel is hidden for good (MainComponent.cpp:2801-2802 by the sheet). Topic F's reading R188 (e) as shown to him: the audio input and its gain are held nowhere, the app opens on the microphone input.
@@END

@@ITEM D36
TITLE: Autopilot moves on from a still, source or MilkDrop
STATUS: OPEN still Harmony's own; no words of his name it; shown to him as K-9 (L130)
HIS: none
RULE: Best reading, shown to him as K-9: the autopilot moves on from a still, a generated picture or MilkDrop the same as from a video. Such a clip has no end, so on a layer set to change at the end of a video it is counted on the beat instead; when the count is full the next clip comes. That holds every time: when the clip is fired a second time, and when it is fired again after the tempo bar's stop took every clip off. The autopilot never stays stuck on such a clip, and the same holds for any other clip without playback, an effects-only clip included. These clips have no pause of their own (topic H's reading R193 b); a video or a picture sequence he has paused by hand is not moved on by the count until he plays it again, as the standing autopilot has it (R203). This is a mend of the autopilot, not a change of MilkDrop (L126).
CHANGED: The page's line said "also after you paused it": these clips get no pause of their own (topic H's reading R193 b, shown to him), so the words are read for the tempo bar's pause, after which the count simply runs on (R213 b). The paper's second rule (a clip paused by hand is not moved on) is the standing autopilot and left K-9.
TODAY: sheet TODAY 29, T4: a Source clip is set playing only on its first fire (Layer.h:555-556: auto-play only while not hasBeenTriggered, never reset), and the on-beat autopilot needs clip->playing (Autopilot.cpp:100), so after a pause or stop the autopilot skips it for good. Lines read by the sheet; the chain INFERRED there. A clip without playback falls through to beat counting (Autopilot.cpp:40, 103-107 by the sheet's C5); a MilkDrop cell has no play head: End of Video cannot end it, the clip autopilot can still count beats and move on (milkdrop-current.md 1.7).
@@END

@@ITEM P35
TITLE: A generated picture's cell
STATUS: DROPPED
HIS: L8, L124
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His L124 says the same of this very thing: "we will need symbols and pictures that make sense for everything and this will be done in the UI step". The three variants (a tile with a tag and the name, a still taken once, a small live picture) differ only in how the cell looks; what a small live picture costs (every source drawn once more per frame) is weighed in the UI step, before one is offered.
CHANGED: The item was held back to be drawn in three variants; nothing is drawn now. Added by the ruling: the cost sentence.
TODAY: sheet TODAY 8: gradient tile, tag "SRC", the type name (ClipCell.cpp:27-47 by the sheet). A live picture would run every source once more per frame (sheet T1).
@@END

## ASSUMPTIONS
@@ASSUME K-1
ABOUT: 215
TEXT: I assume "plan all of these" means all six get built, not only designed on paper: OSC, Link, picking the audio input, an audio file that loops, a text source, Render.
WHY: L128 says plan; whether planned means built is not said.
ALT: b) The six are only designed on paper for now, and each is built when you ask for it.
IF-WRONG: SMALL work is planned and built that he did not want yet; nothing has to be rebuilt
ASK: LINE the one guess in his word "plan" that decides how much is built
@@END

@@ASSUME K-2
ABOUT: 215
TEXT: I assume Render is one button in Review that makes a film file of the recording you have open: the whole recording, at full composition size, with its sound.
WHY: L128 plans Render; what it makes is in no line since September (binding-decisions.md 220-224). L114 "All else is gone" is read as about the recordings' names.
ALT: b) Render makes a film only of the stretch between the in and out points. c) Render is gone with "All else is gone"; the Video box of Record Show is the only way to a film.
IF-WRONG: SMALL a planned thing, not built yet; the ground it needs (Review draws a recording afresh) is laid for Review anyway
ASK: LINE he answered 215 with Render in the list (L128), so it is no question; the line shows what Render is taken to be, and what L114 removes is asked once, in topic J (J-1)
@@END

@@ASSUME K-3
ABOUT: R228 R202
TEXT: I assume MilkDrop reaches the output only through a cell that plays. A click on a preset in the MilkDrop tab changes the preset, but does not put MilkDrop on the output while no clip plays.
WHY: L126 says a document "and that's it for this upcoming build"; the black-output rule, which he read and did not name, changes this one MilkDrop behaviour.
ALT: b) MilkDrop is left fully alone: a preset clicked in its tab shows on the output whenever no clip plays.
IF-WRONG: STAGE he clicks a preset with no clip playing and the audience sees black, or sees MilkDrop
ASK: YES seen on the output; L126 and a reading he left standing meet here, and it is about how he puts MilkDrop on screen
@@END

@@ASSUME K-7
ABOUT: 211
TEXT: I assume a clip of many pictures gets, in BPM mode, a time per picture: a number of beats set like a clip's Beats (-, +, /2, x2). Until you switch it on, its pictures are spread over its length.
WHY: The default said "a beat, a bar, 2 bars"; L21 and L99 count a BPM-mode clip in beats; how it is set and its first setting are in no line.
ALT: b) A list beside the Beats row instead: 1 beat, 2 beats, 1 bar, 2 bars, 4 bars. c) A new clip of many pictures starts with one picture per bar.
IF-WRONG: SMALL one row of the Clip tab and a first setting
ASK: LINE a small choice of Harmony's in a row he reads in beats; he can strike it
@@END

@@ASSUME K-9
ABOUT: D36
TEXT: I assume the autopilot moves on from a still, a generated picture or MilkDrop when its count is full, the same as from a video, also after a stop or a second fire. It never stays stuck on one.
WHY: He did not read the list of things decided without asking and asks to be shown them (L130); no line of his names this.
ALT: b) A still, a generated picture or MilkDrop stays until you change it by hand, whatever the autopilot is set to.
IF-WRONG: STAGE a layer on autopilot that stays on one picture, or leaves one he wanted to keep
ASK: LINE a mend he would most likely wave through; shown because of L130
@@END

@@ASSUME K-10
ABOUT: 215 R204
TEXT: I assume the computer remembers which audio input you picked, for every show; a show does not hold it.
WHY: L128 plans the pick, not where it is remembered. A line he read (L94) keeps the audio input out of the show; L96 has the show remember its outputs.
ALT: b) The show remembers the pick too, the way it remembers its outputs. c) Nothing remembers it: at every start the app follows the Mac's own choice until you pick.
IF-WRONG: STAGE the app could listen to the wrong input after a show is opened, until he picks again
ASK: LINE a real choice of where one setting lives; his lines lean both ways (L94, L96), so he sees it and can strike it
@@END

@@ASSUME K-11
ABOUT: R204 215
TEXT: I assume the tempo bar never starts or stops an audio file the app plays: on pause and on stop the music plays on. Only its own play / pause control stops it.
WHY: The page said this of play and pause and he left it; of stop it said nothing, and L72 has stop stop "all actions as well as everything else".
ALT: b) The tempo bar's stop also stops the audio file.
IF-WRONG: STAGE the music stops, or plays on, when he presses stop; one small change to undo
ASK: LINE his earlier answers list what stop takes (the clips, the beat) and the file is not among them; L72 can be read wider, so he sees it
@@END

@@ASSUME K-12
ABOUT: 215 R201
TEXT: I assume a new source called Text: you type the words on its Clip tab and set font, size, colour and place like any source. The source that draws digits stays and is renamed Digit Rain.
WHY: L128 plans a text source; no line says where words are typed, what can be set, or what becomes of the digit source.
ALT: b) The digit source is turned into the text source. c) The words are the clip's name.
IF-WRONG: SMALL one new source and a name
ASK: LINE details come back to him when this part is planned
@@END

@@ASSUME K-13
ABOUT: 215
TEXT: I assume planning OSC (messages from a controller program) means: every button and slider you can put on a key or pad can also be driven by such a message. The app only receives them; it sends none.
WHY: L128 plans OSC; no line says which things it reaches or whether the app answers.
ALT: b) Only the OSC messages the app already takes are kept, with routine renamed to action. c) The app also sends OSC, so that a controller program shows the state.
IF-WRONG: SMALL addresses can be added later
ASK: LINE low stakes; the same point as topic J's J-10, asked once, here
@@END

@@ASSUME K-14
ABOUT: 215
TEXT: I assume that with Link on, the tempo and the 1 come from the programs you are linked to, and your /2, x2 and nudge change only this app's picture, never what the others share.
WHY: L13 and L14 rule /2, x2 and the nudge for the app's own clock, not under Link. The reading on Link he left greys out only play, pause and Resync.
ALT: b) With Link on, /2, x2 and the nudge are greyed out too. c) They change the shared tempo for everybody.
IF-WRONG: STAGE the picture runs at another rate than the band's clock
ASK: LINE Link is planned, not built; shown so that he can strike it
@@END

@@ASSUME K-16
ABOUT: R213
TEXT: I assume during a tempo pause a clip whose turn is over waits on its last frame, and the autopilot brings the next clip when the tempo plays again, also if it is set to Play Once and Eject.
WHY: L98 lets the autopilot's next clip cancel the Eject; the reading he left has the autopilot do nothing during a pause; the two together are in no line.
ALT: b) During a pause the Eject wins and the layer goes empty.
IF-WRONG: STAGE a layer that goes empty during a pause; rare
ASK: NO three rare things at once (a tempo pause, the autopilot, Play Once and Eject); it follows from L98 and the standing reading, and either way is a small change
@@END

@@ASSUME K-19
ABOUT: R202
TEXT: I assume the MilkDrop document is one file kept with the project's planning papers and is the starting paper of the MilkDrop session.
WHY: L126 asks for "a dedicated document"; where it is kept is not said.
ALT: none
IF-WRONG: SMALL a file's place
ASK: NO internal
@@END

@@ASSUME K-20
ABOUT: 215 212
TEXT: I assume the six planned things are built after the first builds (firing and the tempo bar, the cue system, presets, actions, Review) and before the blend lists; the seven idle things are gone through after that.
WHY: L128 gives no order; L110 puts only the blend lists last, before the UI redesign; the default of 212 says "After the builds we go through them one by one".
ALT: b) Some of the six belong inside the first builds.
IF-WRONG: SMALL the order of work changes, nothing is rebuilt
ASK: NO the order of work is Harmony's to set
@@END

@@ASSUME K-21
ABOUT: 215 R204
TEXT: I assume a looping audio file gets one switch, Loop File, that is on at every start of the app: the file starts again at its end until you switch it off.
WHY: L128 plans the looping audio file; a switch and its first setting are in no line. That nothing of the audio setup is remembered he read and left (L94).
ALT: b) The file always loops, no switch. c) The switch is off at first.
IF-WRONG: SMALL a switch and its first setting
ASK: NO a small default he can flip on screen
@@END

@@ASSUME K-22
ABOUT: 215 R204
TEXT: I assume that when the audio input you picked comes back after it was pulled out, the app goes back to it by itself, without a word.
WHY: The reading he left covers the pull-out (the built-in microphone, without a word); the return is in no line.
ALT: b) The app stays on the built-in microphone until you pick again.
IF-WRONG: STAGE rare: a cable put back in during a show
ASK: NO completes the pull-out rule he left standing; a rare moment, a small change either way
@@END

@@ASSUME K-23
ABOUT: R228
TEXT: I assume only what you reach from the screen stops feeding the output (the two buttons, a picture dropped outside the grid, a click in the MilkDrop tab); the remote-control and test calls that load a loose picture keep working.
WHY: The reading names four feeds he can reach from the screen; the calls that test tools use for the same slot are in no line.
ALT: b) The slot goes altogether and the test tools are rebuilt on clips.
IF-WRONG: SMALL the test tools would need rework; nothing he sees
ASK: NO technical; the remote-control port is not his concern (topic J)
@@END

## QUESTIONS BACK
## NAMES
@@NAME MilkDrop
MEANS: The music-visualizer source with its own browser, Jukebox and playlists; untouched in the coming build.
SOURCE: the on-screen name now, kept; he types "Milk drop" (L126)
@@END

@@NAME Jukebox
MEANS: MilkDrop's own way of changing presets by itself after a count.
SOURCE: the on-screen name now, kept until the MilkDrop session (L126)
@@END

@@NAME Autopilot
MEANS: The tool that brings a layer's next clip by itself after a count or at the end of a video; a different tool from actions.
SOURCE: the on-screen name now, kept; he uses it in L98 ("plays on autopilot")
@@END

@@NAME Global
MEANS: The level above the layers (its tab holds the unused Autopilot block of 212 f); replaces the word Composition.
SOURCE: his words L55 ("lets move to global")
@@END

@@NAME Review
MEANS: The screen where a show recording is looked at and where Render will sit.
SOURCE: his words L114 ("Show Recording Review should be called Review for short")
@@END

@@NAME Render
MEANS: Making a film file afterwards from a show recording.
SOURCE: the on-screen name now ("Render... (coming)"), kept; Harmony's pick until he names it; BD:220-224 "offline render" is Harmony's wording of his September answer
@@END

@@NAME Time per picture
MEANS: In a many-picture clip in BPM mode, how long each picture stays.
SOURCE: Harmony's pick (the default of 211 said "how long each picture stays")
@@END

@@NAME Text
MEANS: The planned source where he types the words that are shown.
SOURCE: Harmony's pick (215 e, L128)
@@END

@@NAME Digit Rain
MEANS: The source that draws rows of random digits.
SOURCE: Harmony's pick; the on-screen name now is "Text Animator", which is replaced by this once the Text source exists (K-12)
@@END

@@NAME Audio Input
MEANS: The planned list in the app from which the wired input or the built-in microphone is picked.
SOURCE: Harmony's pick (215 c, L128)
@@END

@@NAME Link
MEANS: Ableton Link: sharing tempo and the 1 with other programs on the network.
SOURCE: the on-screen name now, kept (215 b, L128)
@@END

@@NAME Loop File
MEANS: The switch that makes an audio file played in the app start again when it reaches its end.
SOURCE: Harmony's pick (215 d, L128); named apart from an action's Loop toggle (topic D) and from the Loop entry of a clip's loop menu (topic H)
@@END

## CONFLICTS (from the paper, unruled)
- Render. L114: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone." against L128: "215 plan all of these" (215 f is Render) and his September words (BD:220-224): "It can be used to make a video recording without using much memory or resources". Read here as: L114 names the recordings, Render is not a recording. Asked: K-2.

## NOT DONE / UNSURE (from the paper, unruled)
- Timing only, not listed as a conflict: his earlier ruling that MilkDrop's lists read bars with no seconds (BD:675, BD:649) against L126 "that's it for this upcoming build". The rule is kept, the work waits (R212, K-4 as a line he can strike). If Harmony judges it a true conflict, K-4 becomes a question.
- Whether a running Jukebox alone (no MilkDrop clip on a layer) puts MilkDrop on the output while no clip plays: not checked. Cheapest: milkdrop-current.md of this run, or one read of the Renderer's "active source" branch (Renderer.cpp:690-735). It decides how wide K-3 is.
- Ableton Link in a closed app: Link's licence terms are an outside fact I cannot look up (no web tools). Cheapest: one Researcher lookup of Ableton's Link licence before the Link plan. Until then "his copy is built with Link" is ASSUMED possible.
- The TODAY lines for OSC, Link's dimmed switch and the greyed Render button are taken from the page's readings and CLAUDE.md, not re-read in source by me (the task: do not re-derive).
- What the tempo row does under Link belongs to topic A's reading on Link; this paper leans on apply-A for it and adds only K-14.
- What the preview monitor shows for a generated picture of a kind that is also live on a layer (one working copy per kind, R201 c) belongs to topic B; not settled here.
- Whether the bars lane's adopted packet (bf7) and the one-save stage S7 still match this paper (MilkDrop relabel out; old top row out) has to be re-read when the build is planned.
- The wording of BD:675 was not opened by me; it is cited through the fact sheet (O4, S3, checked there). K-2's September quote was read at BD:220-224.

## FOR THE PAGE RULING (from the ruling)
- ONE QUESTION from this topic (K-3, YES): may a click in the MilkDrop tab still put MilkDrop on the output while no clip plays. R228, which he left standing, takes it away; L126 says nothing of MilkDrop changes. It borders topic B's B-9.
- K-2 is a LINE now, not a question: L128 names Render itself; what "All else is gone" (L114) removes is asked once, in topic J's J-1, and Render follows it. The paper's CONFLICTS bullet on Render is copied unruled and is overtaken; so are its NOT DONE bullets 1, 2, 6 and 8 (K-4 dropped; the MilkDrop document answers the Jukebox point; topic B's B-11; binding-decisions.md 675 opened).
- AUDIO INPUT, with topic F: R188 (e), which he read (L94), stays whole: the show holds nothing of the audio setup. K-10 (LINE) has the COMPUTER keep the picked input, one new entry for R188 d's list; "the show too" (L96) is its ALT b. F's flag "One of the two must give" is answered so.
- ASKED ONCE ELSEWHERE, dropped or pointed here: pause and clips not in BPM mode = A-3 (K-6 dropped); who starts a stopped beat = A-13 (R213 b says the same and stood in the part he read); under-a-bar wording = I-20 (K-8 dropped, R213 c follows it); layer kinds = I-1 (R203 c hangs on it); the layer's switch against actions = D-2 (R213 a follows it); chance picks = E-38 (K-18 dropped). K-13 = topic J's J-10: asked here.
- THE AUTOPILOT KEEPS TO THE DECK ITS CLIP CAME FROM (R203; his words, binding-decisions.md 598-600: "the playing will not be affected"). Topic D's D-10 lets a running action's next fire follow the deck on screen (182 b): the same words of his bear on it.
- L102 (the start menu) is taken into R213 a. L126 reaches the adopted bars lane: the MilkDrop relabel (R212) comes out of that lane's packet; the tempo bar still holds MilkDrop's preset counting through the beat (R202 part 5, with topic A's R214 a).
- THE SIX PLANNED THINGS: K-1 (LINE) asks only whether planned means built. Link's licence for a closed app is NOT looked up (no web tools here; a researcher's job) and decides whether Link can be built at all: his page must not promise Link before that.
- FOR HIS PAGE from this topic, by weight: K-3 (ask); lines K-1, K-2, K-11 (does the tempo bar's stop stop an audio file: L72), K-10, K-9 (D36, shown because of L130), K-7, K-12, K-14, K-13. Internal: K-16, K-19, K-20, K-21, K-22, K-23.
- NAMES: the replaced blocks say "tempo bar" (his L13; topic A), the blocks left as they are still say "tempo row". "Loop File" is new and kept apart from topic D's "Loop" and topic H's loop menu. Text, Digit Rain, Time per picture and Audio Input are Harmony's picks.
- THE MILKDROP DOCUMENT exists and is re-checked (milkdrop-current.md, check-milkdrop.md); its part 1 is written for him. His page should say so in one line.

