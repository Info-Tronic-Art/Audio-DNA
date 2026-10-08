# APPLY K -- Sources and the automatic features (s-rta-1007)
Written: Wed Oct  7 22:50:47 EDT 2026 (read-only run; nothing built, run, launched or committed)
Labels: his words are quoted verbatim with their L number (boris-msg-numbered.txt); BD = .harmony/binding-decisions.md; "sheet" = .harmony/.reports/s-rta-1005/area-sources-auto.md (its CORRECTIONS block wins). What the app does now stands ONLY in the TODAY lines.

## SUMMARY
- 215: "plan all of these" (L128). All six are planned: OSC, Ableton Link, picking the audio input in the app, an audio file that loops, a text source where words are typed, Render. The page's default (none is built or changed) is replaced. The ORDER is Harmony's assumption (K-1).
- R202: nothing of MilkDrop is changed in the coming build (L126); its only deliverable is the document on how MilkDrop works now (milkdrop-current.md). The smarter MilkDrop system is designed with him in a session of its own. The reading's own small mends (the playlist Blend that does not fade; whether preset files travel) wait for that session.
- R212 (the Jukebox labels in bars) is overtaken in TIME by L126: his rule "bars, no seconds" stands and is carried into the MilkDrop session (K-4, a line he can strike).
- R201: symbols and pictures for everything are done in the UI step (L124); so P35 (the look of a source's cell) is dropped as a question.
- R204: parts a (no list of inputs) and c (the file plays once) change once the planned things are built (L128); the rest stands.
- 211, 212, R211, R228, R203, R213 stand as shown; R213 is now carried by his tempo words (L11, L12, L13) and R217 (L98).
- Two real questions for him: Render against "All else is gone" (K-2), and whether the MilkDrop tab may still put a picture on the output while no clip plays (K-3).
- D36 is still Harmony's own (he did not read it): shown as a line (K-9).

## ITEMS
@@ITEM 211
TITLE: Many pictures dropped on a cell: one clip, plus time per picture
STATUS: DEFAULT option A, by his first line
HIS: L1
RULE: Pictures dropped together onto one cell: one picture makes one clip; exactly two make two clips side by side; three or more make ONE clip that runs through them (order: K-15); a folder itself is not dropped (the page's "one clip", unchanged). Outside BPM mode the clip runs at a set number of pictures per second. In BPM mode the clip has two ways, picked on the clip: (1) Spread: all its pictures are spread evenly over the clip's length, like a video; (2) Time per picture, new: every picture stays for a set musical length and the picture changes on the beat, so the clip's length follows from the number of pictures times that length (unit and first setting: K-7). Being in BPM mode, the clip starts on the "1" like every BPM clip (L11, L12). Copy, paste and Option-drag work on it as on any clip (L4, L5). The separate row at the top (Image Folder, Beats per Image) goes (R228); the steady change of picture on the beat lives in the clip from then on.
CHANGED: nothing (he did not name it; "All defaults good except for these", L1). Tested against L21 and L99 (a BPM clip counts in beats): that only sets the UNIT of the new choice, which the page gave as "a beat, a bar, 2 bars" (K-7).
TODAY: sheet TODAY 2, 3 and H12, H13: 1 picture = 1 clip, 2 = 2 clips, 3+ = one sequence cell (MainComponent.cpp:843-880 by the sheet); in BPM mode the sequence is spread over beatDivision beats (Clip.h:34-41 by the sheet); the per-picture rhythm exists only in the old top row (MainComponent.cpp:455-467 by the sheet) and feeds only the old picture slot. To change: a per-picture length on the sequence clip in BPM mode; the old row goes with R228.
@@END
@@ITEM 212
TITLE: Seven things that do nothing: later, all seven
STATUS: DEFAULT option A, by his first line
HIS: L1
RULE: Later, all seven. The coming builds do not touch them and they stay on screen as they are, neither removed nor hidden nor built: (a) the camera (the Camera list in the top row and the Camera Input entry in the Sources tab); (b) Import ISF Shader; (c) the genre the app detects; (d) what happens on a detected drop or breakdown; (e) the smart autopilot; (f) the Autopilot block of the tab that is now called Global (L55); (g) the randomizer of effects. After the builds they are gone through one by one, each with its own questions to him; in the order of work they come after the six planned things of 215 (K-1). Nothing in his message asks for any of the seven sooner.
CHANGED: nothing. Tested against his whole message: no line names a camera, ISF, genre, a drop, a smart autopilot or a randomizer; L55 only gives the Composition tab its name Global; L126 keeps the one live reaction to a drop (MilkDrop's Jukebox) untouched.
TODAY: sheet TODAY 5, 6 (camera), 12 (ISF phantom), 13, 14 (genre), 15 with C1, C2 (structure: only a log, plus the Jukebox and the ruled oscillator switch), 18 (smart autopilot never runs), 17 (the block is read by nothing), 19 (randomizer hidden, aimed at the old chain); H1-H6, H9. Earlier instruction BD:99 (Harmony's consequence text: wire dead UI up, do not hide it) agrees with "later, not removed".
@@END
@@ITEM 215
TITLE: Six things: all are planned
STATUS: ANSWERED in his own words: all six
HIS: L128
RULE: His words: "215 plan all of these". All six are planned, none is left as it is. Before each can be built, its plan has to settle the following. (a) OSC: the list of addresses (assumed: one for everything a key or pad can be mapped to, K-13); that the routine address is renamed to an action address; whether OSC follows the mapping files he wants (L116) or keeps a fixed address list; the port and where it is set; receive only or also send. (b) Ableton Link: that his copy of the app is built with Link at all, and under which licence terms a closed app may ship it (UNKNOWN, see NOT DONE); what the tempo row does while Link is on (the rule of topic A's reading on Link, re-read against L12, L13, L14); what /2, x2 and nudge do under Link (K-14); what happens when the Link tempo leaves his range of 22 to 480 (L14); where the switch sits (L8). (c) Picking the audio input in the app: a list that holds only wired inputs and the built-in microphone, never Bluetooth (his standing rule, BD:485-488); where the pick is remembered; what the app listens to when the picked input is missing at start or pulled out, and when it returns (K-10); which channels of a many-channel input are heard; no message appears by itself (question 209). (d) An audio file that loops: a Loop switch and its first setting; that the tempo row never stops the file (K-11); whether the show remembers the file and the switch (with c). (e) A text source where words are typed: where the words are typed, which fonts, size, colour, place, one line or several, how it moves, that its sliders connect to signals like every slider (BD:107-109), that the words are saved with the clip, and what becomes of the source that draws digits (K-12). (f) Render: a film made afterwards from a show recording, in Review: which recording (the original or its saved mend, L84), the whole or the stretch between in and out, size (the full composition, L88), file kind, with the recording's sound, not in real time; and the technical ground that a recording replays to the same picture every time (K-18). Render against his "All else is gone": K-2. The ORDER of the six against the other builds is not in his words: K-1.
CHANGED: Option A (THE DEFAULT, "None of the six is built or changed now") is replaced by his words; option B (name the letters) is answered with all six. The lines of other readings that said "stays as it is" for these six fall with it: R204 a and c (this slice), the last sentence of R201 d (this slice), and in other slices the readings on Link, on the Record tab's greyed Render and on OSC.
TODAY: OSC input exists (CLAUDE.md "OSC input (UDP 8000)"; not checked further). Link: his copy is built without it and the switch is dimmed (the page's reading on Link; not checked in source). Audio input: no picker, follows the Mac's default or the built-in microphone (sheet TODAY 23, H14); nothing of the audio setup is saved (TODAY 24, H15). Audio file: no looping call in the code, plays once (sheet M3, INFERRED there). Text Animator draws random seven-segment digits (sheet C4, EmbeddedShaders.h:11944-11975 by the sheet). Render: a greyed "Render... (coming)" on the Record tab (the page's reading on the Record tab; not checked); his September words BD:220-224.
@@END
@@ITEM R201
TITLE: Sources stay; symbols and pictures come in the UI step
STATUS: CORRECTED part b only
HIS: L124, L128
RULE: (a) Stays: the Sources tab with its generated pictures and fractals, each with its own sliders; MilkDrop with its own browser; still pictures; videos. (b) His words: "we will need symbols and pictures that make sense for everything and this will be done in the UI step". Every thing on screen, a source's cell included, gets a symbol or picture that makes sense, and that work belongs to the UI step, not to the coming builds; until then a source's cell is a coloured tile with a tag and its name. (c) There is one working copy of each kind of generated picture: two clips of the same kind keep their own slider values and share one running state, so a picture with a memory (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics) on two layers at once disturbs itself, and every MilkDrop clip shows the same preset. The coming builds leave that as it is; what it means for the preview monitor is ruled under the cue system (topic B). (d) The source that draws rows of digits stays; a source where words are typed is planned (215 e, L128). As clips, a generated picture, a still and MilkDrop follow the rules of every clip (K-6, K-17).
CHANGED: b: the page said "whether it gets a picture comes with the pictures"; his words move it, and symbols and pictures for everything, to the UI step. d: its pointer "a real text source: question 215 (e)" is now answered: planned (L128). a and c: nothing.
TODAY: sheet TODAY 7, 8 (a source's cell: gradient, tag "SRC", the name; ClipCell.cpp:27-47 by the sheet), 9 and T1 (one instance per source type, Renderer.cpp:1160-1171 by the sheet), 10 with C4 (digits), H11, H21.
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
RULE: When no clip plays, the output is black, apart from an effect that holds or trails a picture after a stop (his 147 a). (a) Open Image, Image Folder with its list Beats per Image, a picture dropped on the window outside the clip grid, and a click on a preset in the MilkDrop tab no longer put a picture on the output by themselves; the two buttons and the list go, together with Save, Load and FX Save. A click on a clip's name or a double-click in the files window shows in the preview monitor, never on the output (L35). (b) The Camera list stays where it is and keeps showing its camera only while no clip plays, until the camera is decided (212 a: later). The MilkDrop part of (a) is the one place where this reading meets L126 ("that's it for this upcoming build"): K-3.
CHANGED: nothing by a line of his that names it. L35 confirms the direction (a name click and a files double-click go to the preview). L126 is tested against the MilkDrop click in part a: kept as shown, with K-3 for him.
TODAY: sheet TODAY 5, 28, O1, H1, H13: the Renderer draws "deck compositor > active source > loaded image" (Renderer.cpp:690-735 by the sheet), so the old single-picture slot (last opened image, image folder, a MilkDrop tab click, the camera) shows whenever no layer plays; row 1 controls MainComponent.cpp:2749-2767 (by the page). Not run. Whether a running Jukebox alone feeds the output while no clip plays: not checked (see NOT DONE).
@@END
@@ITEM R202
TITLE: MilkDrop: a document now, nothing changed, a session later
STATUS: CORRECTED parts c, d, e and the pointer to the labels
HIS: L126
RULE: His words: "we will design a much smarter system for doing Milk drop and we will do that as a dedicated session where I will design the UI and how we will use it but not right now. I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build." So: (1) the coming build changes nothing of MilkDrop: its browser, the three ways of changing presets (the Jukebox, a clip's own playlist, by hand), Random and Bag, favourites, both Blend sliders, the reaction to a build-up, drop or breakdown while the Jukebox runs, and how a clip's playlist is saved all stay exactly as they are. (2) The one MilkDrop deliverable of the coming build is a document that says how MilkDrop works at the time of writing (milkdrop-current.md, written in this run). (3) The smarter system is designed in a session of its own, where he designs the UI and how it is used; the document is that session's ground. Carried into that session, not built before it: the playlist's Blend slider that does not fade, a switch for the change on a drop or breakdown, whether a clip's preset files travel with the show (K-5; his L48 on effect presets leans to yes), and the labels of the two timing lists (R212, K-4). (4) A MilkDrop clip is still a clip: the rules of every clip hold for it (K-17).
CHANGED: c: "is made to fade" is removed from the coming build (no MilkDrop change, L126). d: "say so if you want a switch for it" is not answered now; it goes to the MilkDrop session. e: "say so if the preset files of your clips should travel with the show" likewise. The last sentence ("What changes in the labels of the Jukebox list: R212") is overtaken in time (see R212). a and b: nothing, they stay.
TODAY: sheet TODAY 11 with C1, C3 (three play modes; the Jukebox changes preset on the next bar after a structural change, PresetSelector.cpp:86-92 by the sheet; structure triggers of a playlist have no control), M1, M2 (Blend in seconds), M5 (favourites by name in a file of this computer; a playlist entry stores a path), H10. The full account is the other agent's milkdrop-current.md (not read by me).
@@END
@@ITEM R212
TITLE: MilkDrop's timing labels in bars: rule stands, work waits
STATUS: REPLACED in time only, by his L126
HIS: L126
RULE: His earlier rule stands as a rule: MilkDrop's timing lists read in bars and carry no seconds. By his L126 ("that's it for this upcoming build") the lists are not touched in the coming build; the relabelling (the Jukebox list reading 4, 8, 16, 32 and 64 bars; a clip's own playlist list reading 1, 2, 4 and 8 bars; the picture changing as often as before) is carried into the MilkDrop session and done with the smarter system. The playlist's triggers on the music's structure stay without a control until that session. He can strike the wait: K-4.
CHANGED: The whole reading was "Mine: the list reads what it does"; what it says is kept, WHEN it is done changes: not in the coming build (L126). INFERRED from "that's it"; he did not name R212 itself.
TODAY: sheet C3: the Jukebox list reads "4 beats", "8 beats", "16 beats", "32 beats", "30 sec", "60 sec" while the engine counts bars and multiplies by 4 (MilkDropBrowser.cpp:563-575, PresetSelector.cpp:169 by the sheet), so "30 sec" and "32 beats" are the same wait; the playlist list reads beats and counts beats (MilkDropBrowser.cpp:611-615). His earlier words: BD:675, BD:649 (sheet O4, S3). The adopted bars lane (bf7) holds this relabel: it must be taken out of the lane's packet for the coming build.
@@END
@@ITEM R203
TITLE: The autopilot stays, beside actions
STATUS: STANDS
HIS: L98
RULE: Stays: (a) a layer's autopilot: the next clip after a count, or at the end of a video; in order or at random; at the end of the row it starts again from the first clip; at random it may return to the clip it has just left; (b) a clip's own list of what comes next; (c) the count per kind of layer. The autopilot and actions are different tools and both stay. His L98 takes the autopilot as given: "If the clip plays on autopilot and the next clip plays then the eject would be cancelled by the next clip appearing." Where his new rules meet the autopilot: R213.
CHANGED: nothing. Tested against L7 (a layer's ignore-actions and ignore-column toggles do not concern the autopilot), L72 (the stop button "stops all actions as well as everything else": the autopilot's count stands with the beat, R213 b) and L98 (the next clip of the autopilot wins over Eject, as the page had it).
TODAY: sheet TODAY 16, 17 with C5, M7 (wraps with the number of columns, skips empty cells, random may go back, nothing on a lone clip; Autopilot.cpp:243-282 by the sheet), H7.
@@END
@@ITEM R213
TITLE: The autopilot under his new tempo rules
STATUS: STANDS and now carried by L11, L12, L13
HIS: L11, L12, L13, L98
RULE: (a) A fire by the autopilot is a fire like his own: a video starts from its beginning; a clip in BPM mode waits for the "1"; a clip that is not in BPM mode comes in at once (L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"). The clip's actions start with it, unless the layer ignores actions (L7). (b) The autopilot counts on the running beat: while the beat is paused or stopped it does not move on (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM"), and it never starts a stopped beat: only his click on play or on a clip does (L12). (c) Its counts read in bars (K-8). (d) A BPM-mode clip the autopilot brings in starts on the next "1" and its count starts there, so a count shorter than a bar still changes on a "1". (e) While the beat is paused or stopped the autopilot does nothing at all, End of Video included: a video that ends then stays on its last frame until the beat plays, and the next clip comes then (with Play Once and Eject: K-16). When the autopilot's next clip appears, a pending Eject is cancelled (L98).
CHANGED: nothing by a line that names it. a, b, d are now his own tempo words applied to a fire the app makes (L11, L12, L13: INFERRED, his lines speak of his own click). e: the meeting with Play Once and Eject during a pause is not in the page or in his words (K-16).
TODAY: sheet TODAY 16 with C5, H8, T9: the on-beat autopilot advances only on beat crossings (Autopilot.cpp:74-76) and needs clip->playing (:100); End of Video runs regardless of the beat (:40); an autopilot fire resumes a clip where it was and brings it in at once (the page's reading, by the area sheets); counts read "1 Beat" to "32 Beats" (LayerInspector.cpp:43-137 by the sheet). To change: restart on fire, the wait for the "1", the hold of End of Video while the beat does not run, the labels.
@@END
@@ITEM R204
TITLE: The sound the app listens to
STATUS: REPLACED in parts a and c only (L128); the rest stands
HIS: L128
RULE: (a) The app listens to a wired input or the built-in microphone, never to a Bluetooth device (his standing rule). Until the planned pick is built it follows the Mac's own choice; once built, the input is picked from a list inside the app (215 c, K-10). (b) If a wired input is pulled out during a show, the app goes on listening to the built-in microphone, without a word. (c) An audio file can be played instead: it starts when he chooses it and is heard on the Mac's output; once the planned loop is built it can loop (215 d, K-11); the tempo row's play, pause and stop never start or stop it (K-11); "Audio play / pause" in the keyboard and MIDI mapping does. (d) What he sees of the sound stays: the waveform, the signal meters and the tempo. The band meters, the dB level, the onset flash, the genre line and the spectrum stay off screen; whether and where they come back is settled in the UI redesign (L8, L124).
CHANGED: a: "there is no list of inputs in the app" is replaced once 215 c is built (L128). c: "plays once" is replaced once 215 d is built (L128). d: "is for the pictures" now reads: in the UI redesign (L8). b: nothing. Tested against L72 ("The tempo stop button stops all actions as well as everything else"): read as everything the stop already takes (clips, the beat, what the beat drives), not the music file: K-11.
TODAY: sheet TODAY 22-26, M3, M4, M8, H14-H16: an input list with "Mic Input" and "Audio File" only; the device is the macOS default if the Bluetooth guard allows it, else the built-in one (AudioEngine.cpp:24-26 by the sheet); the file plays through the app's output too; no audio field is saved in the show or the settings; the readout panel is hidden for good (MainComponent.cpp:2801-2802 by the sheet).
@@END
@@ITEM D36
TITLE: Autopilot moves on from a still, source or MilkDrop
STATUS: OPEN still Harmony's own; no words of his name it
HIS: none
RULE: Best reading, shown to him as K-9: on a still, a generated picture or MilkDrop the autopilot counts exactly as it does on a video set to count on the beat, and moves on when the count is full; it does so again after the clip was fired a second time, and after a stop of everything followed by a new fire. It never gets stuck on such a clip. A clip he has paused by hand is not moved on until he plays it again, whatever kind of clip it is. This is a mend of the autopilot, not a change of MilkDrop (L126).
CHANGED: The page's line said "also after you paused it": read here as "after the pause is over" (a paused clip of any kind is not moved on); the other reading is in K-9.
TODAY: sheet TODAY 29, T4: a Source clip is set playing only on its first fire (Layer.h:555-556: auto-play only while not hasBeenTriggered, never reset), and the on-beat autopilot needs clip->playing (Autopilot.cpp:100), so after a pause or stop the autopilot skips it for good. Lines read by the sheet; the chain INFERRED there.
@@END
@@ITEM P35
TITLE: A generated picture's cell
STATUS: DROPPED
HIS: L8, L124
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His L124 says the same of this very thing: "we will need symbols and pictures that make sense for everything and this will be done in the UI step". The three variants (a tile with a tag and the name, a still taken once, a small live picture) differ only in how the cell looks.
CHANGED: The item was held back to be drawn in three variants; nothing is drawn now.
TODAY: sheet TODAY 8: gradient tile, tag "SRC", the type name (ClipCell.cpp:27-47 by the sheet). A live picture would run every source once more per frame (sheet T1).
@@END

## ASSUMPTIONS
@@ASSUME K-1
ABOUT: 215 212
TEXT: I assume "plan all of these" means all six get built, in this order: after the first builds (firing and the tempo row, the cue system, presets, actions, Review), before the blend lists and the UI redesign.
WHY: L128 says plan, not when, nor whether plan means build; L110 names only what is last.
ALT: b) The six are only designed on paper for now and built when you ask. c) Some of the six belong inside the first builds.
IF-WRONG: SMALL the order of work changes, nothing is rebuilt
ASK: LINE a choice of order he can strike
@@END
@@ASSUME K-2
ABOUT: 215
TEXT: I assume Render stays planned: in Review one button makes a film file from a show recording, with its sound and your later edits, at full composition size. "All else is gone" was only about what the recordings are called.
WHY: L114 "All else is gone" stands against L128, which plans Render; what Render makes is in no line of his since September (BD:220-224).
ALT: b) Render is gone with the rest; record show with its Video box is the only way to a film. c) Render makes a film only of the stretch between the in and out points.
IF-WRONG: REBUILD Render needs every recording to replay to the same picture; that ground is laid in the recording build
ASK: YES his own two lines pull apart
@@END
@@ASSUME K-3
ABOUT: R228 R202
TEXT: I assume that when no clip plays the output is black for MilkDrop too: a click on a preset in the MilkDrop tab no longer puts MilkDrop on the output by itself. Nothing else of MilkDrop changes.
WHY: L126 says a document "and that's it for this upcoming build"; the black-output rule he did not name would change this one MilkDrop behaviour.
ALT: b) MilkDrop is left fully alone: a preset clicked in its tab still shows on the output while no clip plays, also after a stop.
IF-WRONG: STAGE the audience sees MilkDrop, or black, after a stop
ASK: YES seen on the output; his L126 and the unnamed rule meet here
@@END
@@ASSUME K-4
ABOUT: R212 R202
TEXT: I assume even the small label fix on MilkDrop's two timing lists (bars instead of beats and seconds, as you ruled) waits for the MilkDrop session, and so does the playlist's Blend slider that does not fade.
WHY: L126 "that's it for this upcoming build" against his earlier ruling that the lists read bars (BD:675).
ALT: b) The labels are corrected in the coming build, because it is only text and you already ruled it. c) Labels and the Blend mend both in the coming build.
IF-WRONG: SMALL a few words in two lists
ASK: LINE he can strike the wait
@@END
@@ASSUME K-5
ABOUT: R202
TEXT: I assume the preset files of a MilkDrop clip do not travel with the show yet; whether they do is settled in the MilkDrop session.
WHY: L126 defers MilkDrop; L48 says presets "should travel with the show file", said of effect presets.
ALT: b) They travel with the show from the coming build on, like effect presets.
IF-WRONG: STAGE on another computer a MilkDrop clip's own presets do not load
ASK: LINE only matters when a show moves to another computer
@@END
@@ASSUME K-6
ABOUT: R201 R202 R213
TEXT: I assume a generated picture, a still and MilkDrop keep moving while the tempo is paused, because they are not in BPM mode; and firing one while the tempo is stopped starts the beat, like any clip.
WHY: L13 "Pause pauses, the beat clock, the clips and everything that it controls with BPM" can be read as all clips; L12 says "any clip".
ALT: b) Pause freezes every picture, generated ones and MilkDrop included. c) Only a clip in BPM mode starts a stopped beat.
IF-WRONG: STAGE a frozen or a moving picture during a pause
ASK: LINE leans on the tempo row topic, which asks the same for videos
@@END
@@ASSUME K-7
ABOUT: 211
TEXT: I assume the time per picture is picked in beats (1/2, 1, 2, 4, 8, 16), the clip's length then follows from it, and newly dropped pictures start spread over the clip's length until you switch.
WHY: The default said "a beat, a bar, 2 bars"; L21 and L99 count a BPM clip in beats; no line gives the list or the first setting.
ALT: b) The list reads in bars. c) A new clip of many pictures starts with one picture per bar.
IF-WRONG: SMALL a list and a first setting
ASK: LINE a small choice he can strike
@@END
@@ASSUME K-8
ABOUT: R213 R203
TEXT: I assume the autopilot's count reads in bars (1/4, 1/2, 1, 2, 4, 8 bars); only a BPM clip's own length reads in beats.
WHY: L99 "do beats here. It was my mistake before" is about the clip's length; whether it also loosens "bars in most places" for the autopilot is not said.
ALT: b) The autopilot's count reads in beats (1, 2, 4, 8, 16, 32 beats).
IF-WRONG: SMALL labels of one list
ASK: LINE he reads this label on stage
@@END
@@ASSUME K-9
ABOUT: D36
TEXT: I assume the autopilot counts on a still, a generated picture or MilkDrop exactly as on a video and never gets stuck there; and a clip you paused by hand is not moved on until you play it again.
WHY: He did not read the list of things decided without asking (L130); no line of his names this.
ALT: b) The autopilot moves on even from a clip you paused by hand.
IF-WRONG: STAGE a layer that stays on one picture, or leaves a clip he paused
ASK: LINE a mend he would most likely wave through
@@END
@@ASSUME K-10
ABOUT: 215 R204
TEXT: I assume the audio input is picked from a list of the wired inputs and the built-in microphone, the show remembers the pick, and if it is missing the app listens to the built-in microphone until it returns.
WHY: L128 plans the pick; L96 says a show remembers its outputs, nothing says so for the input.
ALT: b) The computer remembers the pick, not the show. c) If the picked input is missing the app hears nothing until it returns.
IF-WRONG: STAGE the app listens to the wrong input at a show
ASK: LINE settled in the plan of this part; he can strike it at once
@@END
@@ASSUME K-11
ABOUT: 215 R204
TEXT: I assume an audio file gets a Loop switch, on at first, and that the tempo row's stop and pause never stop the file: the music plays on.
WHY: L128 plans the loop without a switch or a first setting; L72 says stop "stops all actions as well as everything else".
ALT: b) The file always loops, no switch. c) The tempo row's stop also stops the file.
IF-WRONG: SMALL a switch and one rule of the stop button
ASK: LINE he can strike it
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
TEXT: I assume OSC gets an address for everything a key or pad can be mapped to, and that the app only receives OSC and does not send it.
WHY: L128 plans OSC; no line says which things it reaches or whether the app answers.
ALT: b) OSC keeps a short fixed list of addresses. c) The app also sends OSC, so a controller program shows the state.
IF-WRONG: SMALL addresses can be added later
ASK: LINE he can strike it
@@END
@@ASSUME K-14
ABOUT: 215
TEXT: I assume that with Link switched on the tempo and the 1 come from Link, and your /2, x2 and nudge still work but only inside this app, never changing what the other programs share.
WHY: L13 and L14 rule /2, x2 and nudge for the app's own clock; nothing says what they do under Link.
ALT: b) With Link on, /2, x2 and nudge are greyed out like pause and Resync. c) They change the shared tempo for everybody.
IF-WRONG: STAGE the picture runs at another rate than the band's clock
ASK: LINE settled when Link is planned; shown so he can strike it
@@END
@@ASSUME K-15
ABOUT: 211
TEXT: I assume pictures dropped together run in the order of their file names, and that two pictures dropped together stay two clips side by side.
WHY: The default says "one clip"; the order of the pictures is in no line.
ALT: b) The order is the order in which they were selected.
IF-WRONG: SMALL a sort rule
ASK: NO internal; the page showed him the two-picture rule
@@END
@@ASSUME K-16
ABOUT: R213
TEXT: I assume that while the tempo is paused a clip whose turn is over waits on its last frame, and the autopilot brings the next clip when the tempo plays again, also when that clip is set to Play Once and Eject.
WHY: L98 lets the autopilot's next clip cancel the Eject; no line covers a paused tempo, when the autopilot is asleep.
ALT: b) During a pause the Eject wins and the layer goes empty.
IF-WRONG: STAGE a layer that goes empty during a pause; rare
ASK: LINE a rare case, one line to strike
@@END
@@ASSUME K-17
ABOUT: R202 R201
TEXT: I assume the new rules for every clip (firing, stop, cue and preview, copy and paste, actions on its effects) also hold for a MilkDrop clip; only MilkDrop's own browser, Jukebox and playlists are left alone.
WHY: L126 "that's it for this upcoming build" could also be read as: a MilkDrop clip is exempt from every new rule.
ALT: b) A MilkDrop clip behaves in every way as before the build.
IF-WRONG: STAGE a MilkDrop clip that fires, stops or previews unlike all others
ASK: LINE he would most likely wave it through
@@END
@@ASSUME K-18
ABOUT: 215
TEXT: I assume a show recording stores every pick the app makes by chance (the autopilot's random clip, a random jump in a clip), so a Render shows the same picture as the night did.
WHY: L128 plans Render; that a recording must replay to the same picture is its technical ground, in no line of his.
ALT: none
IF-WRONG: REBUILD the recording's content would have to be widened after the fact
ASK: NO technical; belongs to the recording plan (topic E)
@@END
@@ASSUME K-19
ABOUT: R202
TEXT: I assume the MilkDrop document is one file kept with the project's planning papers and is the starting paper of the MilkDrop session.
WHY: L126 asks for "a dedicated document"; where it is kept is not said.
ALT: none
IF-WRONG: SMALL a file's place
ASK: NO internal
@@END

## QUESTIONS BACK
(none: this topic names no question of his to answer)

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
@@NAME record show
MEANS: One of the two recordings; the one a Render is made from.
SOURCE: his words L114 ("we will record to clip or record show")
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

## CONFLICTS
- Render. L114: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone." against L128: "215 plan all of these" (215 f is Render) and his September words (BD:220-224): "It can be used to make a video recording without using much memory or resources". Read here as: L114 names the recordings, Render is not a recording. Asked: K-2.

## NOT DONE / UNSURE
- Timing only, not listed as a conflict: his earlier ruling that MilkDrop's lists read bars with no seconds (BD:675, BD:649) against L126 "that's it for this upcoming build". The rule is kept, the work waits (R212, K-4 as a line he can strike). If Harmony judges it a true conflict, K-4 becomes a question.
- Whether a running Jukebox alone (no MilkDrop clip on a layer) puts MilkDrop on the output while no clip plays: not checked. Cheapest: milkdrop-current.md of this run, or one read of the Renderer's "active source" branch (Renderer.cpp:690-735). It decides how wide K-3 is.
- Ableton Link in a closed app: Link's licence terms are an outside fact I cannot look up (no web tools). Cheapest: one Researcher lookup of Ableton's Link licence before the Link plan. Until then "his copy is built with Link" is ASSUMED possible.
- The TODAY lines for OSC, Link's dimmed switch and the greyed Render button are taken from the page's readings and CLAUDE.md, not re-read in source by me (the task: do not re-derive).
- What the tempo row does under Link belongs to topic A's reading on Link; this paper leans on apply-A for it and adds only K-14.
- What the preview monitor shows for a generated picture of a kind that is also live on a layer (one working copy per kind, R201 c) belongs to topic B; not settled here.
- Whether the bars lane's adopted packet (bf7) and the one-save stage S7 still match this paper (MilkDrop relabel out; old top row out) has to be re-read when the build is planned.
- The wording of BD:675 was not opened by me; it is cited through the fact sheet (O4, S3, checked there). K-2's September quote was read at BD:220-224.
