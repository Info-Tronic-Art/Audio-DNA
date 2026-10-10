# SPEC K -- Sources and the automatic features (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-K.md, ruling rule-K.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 247
TITLE: MilkDrop stays exactly as it is; a clicked preset still shows
STATUS: ANSWERED in his own words: the item as written, not way b
HIS: BF265 "Keep Milk drop as it is. When I have time while you are building, I will design a whole system for Milk drop"
RULE: Nothing of MilkDrop changes in the coming build (BF265: "Keep Milk drop as it is."), and that includes how it reaches the output: while no clip plays on any layer, a preset picked by hand in the MilkDrop tab (a click on its row; the tab's "<", ">" and "?" buttons the same) changes the preset and shows MilkDrop on the output. Way b of the item (the output stays black; MilkDrop only through a clip that plays) is not built. The build leaves alone when that picture comes and when it goes; once a clip plays, the clip is what the output shows. Two places where the new tempo bar meets that picture are not in his words; they are Harmony's reading and are built so until he says otherwise: (1) the tempo stop takes the picture away with everything else, and the output is black until he picks a preset again or triggers a clip (K3-3); (2) a triggered BPM-mode clip that waits for the "1" does not take the picture away while it waits: MilkDrop keeps showing and gives way when the clip starts (K3-6). The rule that the output is black when no clip plays (@@ITEM R228 as amended) holds for every other loose picture: Open Image, Image Folder and a picture dropped outside the clip grid no longer reach the output. Everything else of MilkDrop stays with @@ITEM R202 as amended, parts 1, 2, 4 and 5: the browser, the Jukebox, a clip's own playlist, picking by hand, Random and Bag, favourites, both Blend sliders, the reaction to a build-up, drop or breakdown, and how a playlist is saved; a MilkDrop clip is a clip like any other for copy, paste, Option-drag, the start of a stopped beat and the tempo stop; the Jukebox and a clip's playlist do not move on while the beat is paused or stopped. What a show keeps of MilkDrop stays as it is too, so a snapshot (topic F's item 236) holds no more of MilkDrop than a show does: whatever it brings back of what was playing (topic F's question), it does not bring back the preset that was showing (K3-8). The new MilkDrop system is designed by him, when he has time while the build runs (BF265: "I will design a whole system for Milk drop"); nothing of MilkDrop is planned or built before his design arrives (K3-4).
CHANGED: Against the item as he read it: nothing in what it decides; way b is rejected. Against the old blocks: R228 a said a click in the MilkDrop tab does not put MilkDrop on the output: that clause falls (amended); R202's "one open meeting point" is closed (amended). New in his words: who designs the smarter system and when (R202 part 3 amended). Old assumption K-3 is closed by this answer. Harmony's own, each flagged: the stop (K3-3), the waiting clip (K3-6), the snapshot (K3-8), nothing planned before his design (K3-4). By the ruling: the sentence that told as a rule how the app behaves before the build (the picture leaves "when a clip is next triggered, or a layer or the deck is cleared": INFERRED in the MilkDrop document, not run) is moved to the notes below; the waiting clip and the snapshot are added; HIS quotes both of his sentences.
TODAY: A hand pick calls loadPreset, makes MilkDrop the loose "active source" and clears the loose still (MainComponent.cpp:1724-1739, re-read by the ruling; Renderer.cpp:688-720 draws "deck compositor > active source > loaded image", by docs/claude/milkdrop.md line 147; read, not run). The loose MilkDrop picture lasts until the next fire of a cell or column or a clear of a layer or the deck, and a pick made while layers play only changes the preset (milkdrop.md line 46: INFERRED there). Read by the ruling, not run: refreshPreviewFromShow (MainComponent.cpp:5209-5241) points the loose picture at a playing still or source clip and clears it when none is playing; by its own comment it runs after a layer's clear and after every column fire (where it is called at a single cell's trigger was not traced). So for K3-3: a tempo stop built as a clear of every layer takes the picture away by that same path. For K3-6 (INFERRED): that refresh at a trigger would clear the picture at once, before a waiting clip has started; it has to run when the clip starts. For K3-8: which preset is showing is not saved with a show (milkdrop.md 1.8), and a cell's saved preset is not loaded when the cell is triggered (milkdrop.md 1.10 point 1). To change for this item: nothing else; the removal of the old top row (R228, R211) must leave the active-source path and the MilkDrop pick alone.
@@END

@@ITEM 272
TITLE: The computer remembers the picked audio input, not the show
STATUS: ACCEPTED as written
HIS: box left empty = accepted as written
RULE: Once the list Audio Input is built (@@ITEM 215 part c as it stands), the input he picks there is remembered by the computer and holds for every show: opening, saving or switching a show never changes which input the app listens to, and a show file holds nothing of it. At the start of the app the remembered input is used. A show taken to another computer listens to whatever is picked on that computer. A Bluetooth device is never offered and never opened (his standing rule; @@ITEM R204 part a as it stands). What the app does when the picked input is missing at the start or is pulled out is @@ITEM 215 part c and @@ITEM R204 part b as they stand: it listens to the built-in microphone, without a word. That it goes back to the picked input when that input returns is Harmony's own assumption (K-22), never shown to him and not part of what he accepted here.
CHANGED: nothing: accepted as written. Tested against his 33 boxes: BF243 (a snapshot saves "the show exactly where it is with all the settings") is about what a snapshot holds and is topic F's, whose ruling leaves the picked audio input out of a snapshot; no box says a show should hold the audio input. The old assumption K-10 had a third way (nothing remembers it), which the page did not show him; it falls with his acceptance. By the ruling: the return to the picked input is no longer written as part of the accepted text (K-22).
TODAY: No picker in the app; the device is the Mac's default if the Bluetooth guard allows it, else the built-in one (AudioEngine.cpp:24-26 by the fact sheet, TODAY 23, H14); no audio field is saved in the show or the settings (TODAY 24, H15). To change: with 215 c, one entry in the computer's settings for the picked input.
@@END

@@ITEM 273
TITLE: Tempo stop stops the audio file; tempo pause does not
STATUS: ANSWERED in his own words: way b for the stop, and a rule for the pause
HIS: BF271 "Stop removes all clips from all layers so it would stop." and "A pause would not pause it unless it is connected to the BPM."
RULE: (1) The tempo stop stops an audio file the app plays: with the clips leaving every layer and the beat stopping, the music stops. (2) The tempo pause does not pause the audio file: the music plays on through a pause. His exception, "unless it is connected to the BPM", needs something that connects an audio file to the BPM; the build gives the audio file no such thing (it is the sound the app listens to, not a clip on a layer, and it has no BPM mode), so in the build the pause never pauses it. What he means by "connected to the BPM" is put to him (K3-1). (3) Tempo play does not start the audio file, as he was shown and left standing in @@ITEM R204 part c; his new words do not speak of play. (4) Not in his words, built so until he says otherwise (K3-2): after a tempo stop the file is back at its beginning and stays silent until he starts it with Audio play / pause; a clip triggered while the beat is stopped starts the beat but not the file; a file set to Loop File does not start again by itself after a tempo stop. (5) Audio play / pause keeps starting and pausing the file at any time, whatever the tempo bar does. What the stop does to clips and layers is topic A's; this item rules only the audio file.
CHANGED: The item as he read it ("the music plays on") is replaced by his words: the stop stops the file. The pause half is new: the page asked only about the stop; it agrees with what he was shown before (the tempo bar's pause never stops the file) and adds an exception the old spec did not have. Against the old blocks: K-11 is answered against its text; R204 c and 215 d amended. INFERRED: "it" in both of his sentences is the audio file of the item he was answering.
TODAY: The file plays through a juce::AudioTransportSource in AudioEngine (AudioEngine.cpp:52-72, read not run); nothing on the tempo row touches it; no looping call, it plays once (fact sheet M3, INFERRED there). The tempo stop of the new tempo bar is not built. To change: the tempo stop calls the audio file's stop and sets its position to the start (if K3-2 holds); the pause path must not touch it.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 211
TITLE: Many pictures dropped on a cell: one clip, plus time per picture
STATUS: DEFAULT option A, by his first line -- AMENDED after page 2 (211 1 by K)
HIS: L1
RULE: Option A, the default he took: pictures dropped together on a cell make one clip, and one thing is new. In full: one picture dropped makes one clip; exactly two make two clips side by side; three or more make ONE clip that runs through them in the order of their file names, a number inside a name counted as a number. A folder itself is not dropped: its pictures are selected and dropped together, and that is how a folder of pictures gets into a cell once the Image Folder button is gone (R228). The clip has playback like a video: Timeline mode or BPM mode, Speed, pause, backwards, the loop menu and the start menu, as topic H rules them; it comes into the show in Timeline mode (topic H's H-15), where it runs at a set number of pictures per second. In BPM mode it has two ways, switched on the clip: (1) Spread: all its pictures are spread evenly over the clip's Beats; a clip has this until he switches. When the clip gets its first Beats (topic H's item 238, point 10) they are whole groups of 4 bars, and no picture is cut off its end: the cut that his answer to item 238 gives a new BPM-mode clip (BF258: "b") is read as a video's, because a clip of many pictures has no normal speed to keep. That reading is Harmony's, not his word, and is put to him as one line (K3-5). [page 2, K: BF258 "b"] (2) Time per picture, new: every picture stays for the same whole number of beats and the picture changes on a beat; while it is on, the clip's Beats row counts the beats of ONE picture, its "-", "+", "/2" and "x2" work on that number, and the clip's whole length is that number times the number of pictures (K-7). In both ways the clip, being in BPM mode, starts on the "1" like every BPM-mode clip (L11, L12), and at its end it does what its loop menu says. Copy, paste and Option-drag work on it as on any clip (L4, L5). The separate row at the top (Image Folder, Beats per Image) goes (R228); the steady change of picture on the beat lives in the clip from then on.
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
STATUS: ANSWERED in his own words: all six -- AMENDED after page 2 (215 1 by K, 215 2 by K, 215 3 by K, 215 4 by K)
HIS: L128, L110
RULE: His words: "215 plan all of these". All six go into the plan of the app, none is left as it is. That is read as: they get built (K-1), after the first builds and before the blend lists, which he puts last (L110: "this is built after all other parts are done. This is last, before the ui redesign which is the final change"); the order is Harmony's (K-20). None of the six is built before its own plan is clear. What each plan starts from: (a) OSC: every button and slider that can be put on a key or pad can also be driven by an OSC message; the app receives OSC and sends none (K-13); the addresses that say routine say action (topic J's reading on OSC); still to plan: fixed addresses or addresses that travel in the mapping files he wants (L116), the port and where it is set. (b) Ableton Link: first a look-up, not yet done, of whether his copy of the app can be built with Link and under which licence terms a closed app may carry it; with Link on, the tempo bar does what topic A's reading on Link says (play, pause and Resync greyed out, a fire never moves the beat, the tempo stop still takes the clips off the layers and leaves the beat running; by his words of this round it also stops every action (BF250; topic D's item 226) and the audio file (BF271; item 273), read as holding with Link on too, because his sentences name no exception: INFERRED, and settled with Link's plan (K-14)) [page 2, K: BF271 "Stop removes all clips from all layers so it would stop."; BF250 "tempo stop stops all actions, not just global"], and "/2", "x2" and the nudge act inside this app only (K-14); still to plan: a Link tempo outside his range of 22 to 480 (L14), and where the switch sits (L8). (c) Picking the audio input in the app: a list (Audio Input) that holds only wired inputs and the built-in microphone, never a Bluetooth device (his standing rule, binding-decisions.md 485-488); the computer remembers the pick, for every show (K-10); when the picked input is missing at the start or is pulled out, the app listens to the built-in microphone without a word (R204 b) and goes back to the picked input when it returns (K-22); still to plan: which channels of a many-channel input are heard. No message appears by itself (topic J's item 209). (d) An audio file that loops: an audio file played in the app starts again at its end; one switch, Loop File, turns that off, and it is on at every start of the app (K-21); the tempo stop stops the file, the tempo pause never pauses it and tempo play never starts it (R204 part c as amended; item 273); a looping file that the tempo stop has stopped does not start again by itself (K3-2) [page 2, K: BF271 "Stop removes all clips from all layers so it would stop."]; the file, the input mode and the gain are remembered nowhere, as he was shown in topic F's reading on what a show keeps. (e) A text source where words are typed: a new source, Text; the words are typed on its Clip tab and saved with the clip; its settings connect to signals like every slider (his rule, binding-decisions.md 106-109); the source that draws digits stays under a new name (K-12); still to plan: fonts, one line or several, how the words move. (f) Render: the film in full quality, which he calls "an HD render" (BF241), made afterwards, in Studio (the screen that was called Review; every "Review" in this part reads Studio), from the recording that is open there [page 2, K: BF241 "we can make an HD render from the Recording Review"; BF245 "Let's go with studio."] (the original or a mend of it: L84), the whole recording, at the size of the full composition (L88, said of the recording), with the recording's sound (K-2). It is not made in real time, and it shows what Review shows of that recording, no more exactly than Review does (topic E's item on the review picture and its assumption E-38). Render no longer waits on what "All else is gone" (L114) takes away: he names it himself (BF241: "we can make an HD render from the Recording Review"). What is left of that question is whether the Video box of Record Show stays beside Render: topic E's line (its E3-3). [page 2, K: BF241 "If we want a very hd recording, we can make an HD render from the Recording Review"]
CHANGED: Option A (THE DEFAULT, "None of the six is built or changed now") is replaced by his words; option B (name the letters) is answered with all six. The lines of other readings that said "stays as it is" for these six fall with it: R204 a and c (this slice), the pointer in R201 d (this slice), and in other slices the readings on Link, on the Record tab's Render and on OSC. Changed by the ruling: Render no longer stands as a question against L114 (L128 names it; topic J's J-1 asks what L114 removes); the promise that a Render equals the night is taken out (nobody has measured it; topic E's E-38); the show remembers nothing of the audio setup (topic F's reading R188 e, which he read through L94); the picked input is kept by the computer (K-10).
TODAY: OSC input on UDP 8000 with 15 address patterns, one of them /audiodna/routine/{slot} (area-controls.md TODAY 25-28, by topic J's paper; not re-read). Link: his copy is built without it and the switch is dimmed; with Link on the app follows Link's tempo only, never its beat position (area-tempo.md part 1 point 10, M-3, by topic A's paper). Link's licence for a closed app: NOT looked up (no web tools in this run; a researcher's job; from memory, UNVERIFIED: Link is offered under the GPL or under a separate licence from Ableton). Audio input: no picker, follows the Mac's default or the built-in microphone (sheet TODAY 23, H14); nothing of the audio setup is saved (TODAY 24, H15). Audio file: no looping call in the code, plays once (sheet M3, INFERRED there). Text Animator draws random seven-segment digits (sheet C4, EmbeddedShaders.h:11944-11975 by the sheet). Render: a greyed "Render... (coming)" on the Record tab (area-recording.md AR 1.1, by topic E's ruling); his September words binding-decisions.md 220-224, and sound in the film file approved at 235 ("do this but triage the correct build order.").
@@END

@@ITEM R201
TITLE: Sources stay; symbols and pictures come in the UI step
STATUS: CORRECTED part b (L124) and the pointer in part d (L128); a and c stand -- AMENDED after page 2 (R201 1 by K)
HIS: L124, L128, L12
RULE: (a) Stays: the Sources tab with its generated pictures and fractals, each with its own sliders; MilkDrop with its own browser; still pictures; videos. (b) His words: "we will need symbols and pictures that make sense for everything and this will be done in the UI step". Every thing on screen, a source's cell included, gets a symbol or picture that makes sense, and that work belongs to the UI step, not to the coming builds, which leave the cell of a source as it is. (c) Two clips of the same kind of generated picture keep their own slider values. The coming builds do not give each clip a running state of its own: a picture with a memory (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics) that plays on two layers at once goes on disturbing itself, and all MilkDrop clips go on showing one and the same preset (MilkDrop is not touched: L126). What that means for the preview monitor is topic B's (its assumption B-11). (d) The source that draws rows of digits stays; a source where words are typed is planned (215 e, L128; K-12). (e) As clips, a still, a generated picture and MilkDrop follow the rules of every clip: fired while the beat is stopped they start it (L12: "any clip"); they have no playback of their own, the tempo bar's pause does not freeze them, and fired during a pause they show and move (topic H's reading R193 b, shown to him and left standing; the rule behind it, that pause holds only what follows the beat, is asked once, in topic A: its A-3). (f) A generated picture that reads the beat by itself takes the beat from the beat clock: while the beat is paused the part of the picture that the beat moves holds where it was, and while the beat is stopped it stands on the "1", as everything that follows the beat does (topic A's item R214, which he left standing; his rule: "Pause pauses, the beat clock, the clips and everything that it controls with BPM.") and as effects that read the beat do (page item 243, accepted as written). The rest of the picture goes on moving, as part e says, and what the music itself drives in it keeps moving. MilkDrop is not such a picture: it moves to the sound (R202 part 5). He named effects, not generated pictures: this is Harmony's reading of his rule (K3-9). A Sync of its own for such a picture, as those effects get one (BF244; topic I), is not in his words and is not built. [page 2, K: item 243 accepted as written; BF167 "Pause pauses, the beat clock, the clips and everything that it controls with BPM."]
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
STATUS: STANDS with one doubt against L126 (K-3) -- AMENDED after page 2 (R228 1 by K, R228 2 by K, R228 3 by K, R228 4 by K)
HIS: none
RULE: When no clip plays, the output is black, apart from an effect that holds or trails a picture after a stop (his 147 a; topic A's item on it) and apart from the camera of part b, and apart from MilkDrop that a pick by hand in the MilkDrop tab has put on the output (part a). [page 2, K: BF265 "Keep Milk drop as it is."] (a) No loose picture reaches the output, except MilkDrop by a pick in its tab: [page 2, K: BF265 "Keep Milk drop as it is."] Open Image, Image Folder with its list Beats per Image, and a picture dropped on the window outside the clip grid do not put a picture on the output by themselves. [page 2, K: BF265 "Keep Milk drop as it is."] The two buttons and the list go, together with Save, Load and FX Save (R211); a picture dropped outside the clip grid does nothing; MilkDrop is the one loose picture that stays, by his words "Keep Milk drop as it is": a preset picked by hand in the MilkDrop tab (a click on its row, "<", ">" or "?") changes the preset and, while no clip plays, shows MilkDrop on the output; the build leaves alone when that picture comes and when it goes. Harmony's reading, not his words: the tempo stop takes it away (K3-3), and a triggered BPM-mode clip that waits for the "1" takes its place only when it starts (K3-6). The whole rule is item 247. [page 2, K: BF265 "Keep Milk drop as it is."] A click on a clip's name or a double-click in the files window shows in the preview monitor, never on the output (L35; topic B). (b) The Camera list stays where it is and keeps showing its camera only while no clip plays, until the camera is decided (212 a: later).
CHANGED: nothing by a line of his that names it. L35 confirms the direction (a name click and a files double-click go to the preview). L126 is tested against the MilkDrop click in part a: kept as shown, with K-3 for him. Added by the ruling: that a click in the MilkDrop tab still changes the preset, that a picture dropped outside the grid does nothing, and the camera of part b named as the second exception to black, which the page's own part b implies.
TODAY: sheet TODAY 5, 28, O1, H1, H13: the Renderer draws "deck compositor > active source > loaded image" (Renderer.cpp:690-735 by the sheet), so the old single-picture slot (last opened image, image folder, a pick in the MilkDrop tab, the camera) shows whenever no layer plays; row 1 controls MainComponent.cpp:2749-2767 (by the page). A HAND pick in the MilkDrop tab (a row click, "<", ">", "?" and, by that paper's wording, the random start that Play makes) makes MilkDrop the loose picture, and the next fire or clear takes it away again; the Jukebox counts only while a MilkDrop picture is drawn (milkdrop-current.md 1.4 last point, 1.10 point 9, 2.4; the pick's call that makes MilkDrop the loose picture and the draw order were read by the ruling too: MainComponent.cpp:1724-1738, Renderer.cpp:688-720; how long the loose picture lasts is INFERRED in that paper; nothing run). The slot itself stays in the code: the Camera list of part b uses it, and so do the remote-control and test calls that load a loose picture (milkdrop-current.md 1.9; Pitfalls 28, 47): K-23.
@@END

@@ITEM R202
TITLE: MilkDrop: a document now, nothing changed, a session later
STATUS: CORRECTED parts c, d, e and the pointer to the labels (L126) -- AMENDED after page 2 (R202 1 by K, R202 2 by K)
HIS: L126, L4, L5, L12, L13, L35
RULE: His words: "we will design a much smarter system for doing Milk drop and we will do that as a dedicated session where I will design the UI and how we will use it but not right now. I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build." So: (1) the coming build changes nothing of MilkDrop itself: its browser, the three ways of changing presets (the Jukebox, a clip's own playlist, by hand), Random and Bag, favourites, both Blend sliders, the reaction to a build-up, drop or breakdown while the Jukebox runs, and how a clip's playlist is saved all stay exactly as they are. (2) The one MilkDrop deliverable of the coming build is the document that says how MilkDrop works at the time of writing (milkdrop-current.md, written and re-checked in this session). (3) The smarter system is designed by him, when he has time while the build runs: the UI and how it is used are his; the document is the ground for it. "That session" and "the MilkDrop session", here and in R212, mean the work that begins when his design arrives; nothing of MilkDrop is planned or built before it (K3-4). [page 2, K: BF265 "When I have time while you are building, I will design"] Carried into that session and not built before it, because each stood in the reading he answered with "that's it": the playlist's Blend slider that does not fade (c), a switch for the change on a drop or breakdown (d), whether a clip's preset files travel with the show (e; until then they do not), and the labels of the two timing lists (R212). (4) A MilkDrop clip is still a clip in a cell, and what his words rule for any clip holds for it: copy, paste and Option-drag (L4, L5); fired while the beat is stopped it starts the beat (L12: "any clip"); the tempo bar's stop takes it off its layer like every clip (his answer to 126 and 127); a click on its name previews it (L35), within the limit topic B names (its B-11). None of that changes how MilkDrop makes or changes its picture. (5) What the tempo bar does to the beat reaches MilkDrop only through the beat it counts: while the beat is paused or stopped the Jukebox and a clip's playlist do not move on (topic A's reading on what stands still, shown to him and left standing; L13), and the picture goes on moving to the sound. A click in the MilkDrop tab while no clip plays keeps showing MilkDrop on the output (item 247; R228 part a). Whether the tempo stop takes that picture away is K3-3. [page 2, K: BF265 "Keep Milk drop as it is."]
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
STATUS: STANDS -- AMENDED after page 2 (R203 1 by K)
HIS: none
RULE: Stays: (a) a layer's autopilot: the next clip after a count, or at the end of a video; in order or at random; at the end of the row it starts again from the first clip; at random it may return to the clip it has just left. The row it walks is the row of the deck its playing clip came from, whatever deck the grid shows: looking through decks never changes what the autopilot brings next (his words of 2026-10-02: "I can switch between 20 decks looking for a clip and the playing will not be affected"). (b) A clip's own list of what comes next. (c) The count per kind of layer, for as long as layers have kinds: whether they keep them was put to him as item 240 of page 2, and his answer names no kind of layer and takes nothing out: the blend modes and the keying stay and masks are added (BF260). So nothing is taken out for a kind, and this count stays as it is. What a layer's kind is from now on -- what a mask is, and whether a layer gets a control for its kind -- is topic I's, with its item 240. [page 2, K: BF260 "All the blend modes and keying stay, and they will be built when there is time."] The autopilot and actions are different tools and both stay. Where his new rules meet the autopilot: R213.
CHANGED: nothing replaced by a line of his. Added by the ruling: (c) is made to hang on topic I's I-1 (L111 took no letter of question 204, whose option B removes the count per kind of layer); the deck sentence, from his words of 2026-10-02 (binding-decisions.md 598-600). L98 agrees with the reading and decides nothing in it: "If the clip plays on autopilot and the next clip plays then the eject would be cancelled by the next clip appearing." Tested against L7, L72 and L73: none speaks of the autopilot.
TODAY: sheet TODAY 16, 17 with C5, M7 (wraps with the number of columns, skips empty cells, random may go back, nothing on a lone clip; Autopilot.cpp:243-282 by the sheet), H7; the Per-Type Autopilot of the Composition tab is wired and off by default (TODAY 17, Pitfall 23); one autopilot for the show advances a layer within the deck its playing clip came from, never the shown deck (docs/claude/effects.md:79, read by the ruling).
@@END

@@ITEM R213
TITLE: The autopilot under his new tempo rules
STATUS: REPLACED in one clause of part a by L102 (the start menu); the rest stands, carried by L11, L12, L13 -- AMENDED after page 2 (R213 1 by K)
HIS: L102, L11, L12, L13, L98
RULE: (a) A fire by the autopilot is a fire like his own. A video starts from its beginning unless the clip's start menu says otherwise (L102: "model these 2 little menu’s after resolume"; topic H's item 202). A clip in BPM mode waits for the "1"; a clip that is not in BPM mode comes in at once (L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"). The clip's actions that are switched on start with it, as at any fire; what a layer's switch against actions does to them is topic D's (its D-2, on L7 and L73). (b) The autopilot counts on the running beat: while the beat is paused or stopped it does not move on (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."), and it never starts a stopped beat: that takes a press of his own (L12; topic A's D1 and A-13). (c) Its counts read in bars, as every list of lengths (the reading on bars and beats, shown to him and left standing; his rule of 2026-10-03, binding-decisions.md 646-648; L99 is about the length row of a BPM-mode clip only, which that rule itself counts in beats). The counts on offer are six lengths: 1 beat, 2 beats, 1 bar, 2 bars, 4 bars and 8 bars; how the two that are shorter than a bar are written follows topic I (its I-20, on L83). (d) A BPM-mode clip the autopilot brings in starts on the next "1" and its count starts there, so a count shorter than a bar still changes on a "1"; until that "1" the layer shows what a fire of his own shows while it waits (topic A). (e) While the beat is paused or stopped the autopilot does nothing at all, End of Video included: a video that ends then stays on its last frame until the beat plays, and the next clip comes then; a clip set to Play Once and Eject waits the same way (K-16). When the autopilot's next clip appears, a pending Eject is cancelled (L98). (f) The autopilot moves on only from a clip that is on its layer; it never brings a clip onto an empty layer. After the tempo stop every layer is empty (BF271: "Stop removes all clips from all layers"), and a layer on autopilot stays empty, also when the beat runs again, until a clip is triggered there, by him or by an action of his; from that clip the autopilot counts on as before. The same holds for a layer that he emptied with its Clear or by triggering an empty cell, and for a layer whose playing clip was deleted or pasted over (BF256: "removes it from the layer strip, and it does not play"; topic F's item 234). Harmony's reading, not his words (K3-7). [page 2, K: BF271 "Stop removes all clips from all layers so it would stop."; BF256 "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does no]
CHANGED: a: "a video starts from its beginning" gains "unless the clip's start menu says otherwise" (L102, his words elsewhere: every fire obeys the start menu that topic H builds). The rest: nothing by a line that names it; a, b, d are his own tempo words applied to a fire the app makes (L11, L12, L13: INFERRED, his lines speak of his own press). c: the paper's question whether the counts read in bars or beats is dropped (settled by the standing readings; L99 names the clip's row only). e: the meeting with Play Once and Eject during a pause is not in the page or in his words (K-16, internal).
TODAY: sheet TODAY 16 with C5, H8, T9: the on-beat autopilot advances only on beat crossings (Autopilot.cpp:74-76) and needs clip->playing (:100); End of Video runs regardless of the beat (:40); an autopilot fire resumes a clip where it was and brings it in at once (the page's reading, by the area sheets); counts read "1 Beat" to "32 Beats" (LayerInspector.cpp:43-137 by the sheet); an autopilot fire and a hand fire go through the same fire path, so the origin of a fire has to be told apart for part b (facts-takes-bpm-fire.md section 3 line 50, by topic A's paper). To change: the start menu and the wait for the "1" at an autopilot fire, the hold of End of Video while the beat does not run, the labels.
@@END

@@ITEM R204
TITLE: The sound the app listens to
STATUS: REPLACED in parts a and c (L128) and in the pointer of part d (L8); b stands -- AMENDED after page 2 (R204 1 by K)
HIS: L128, L8
RULE: (a) The app listens to a wired input or the built-in microphone, never to a Bluetooth device (his standing rule). Until the planned pick is built it follows the Mac's own choice; once built, the input is picked from a list inside the app (215 c; K-10). (b) If a wired input is pulled out during a show, the app goes on listening to the built-in microphone, without a word. (c) An audio file can be played instead: it starts when he chooses it and is heard on the Mac's output; once the planned loop is built it starts again at its end (215 d; K-21). The tempo stop stops it, with everything else the stop takes (his words: "Stop removes all clips from all layers so it would stop."). The tempo pause does not pause it ("A pause would not pause it unless it is connected to the BPM."); nothing in the build connects an audio file to the BPM, so the pause never pauses it (K3-1). Tempo play does not start it, as he was shown. After a tempo stop the file is back at its beginning and silent until he starts it himself (K3-2). [page 2, K: BF271 "Stop removes all clips from all layers so it would stop."] "Audio play / pause" in the keyboard and MIDI mapping starts and stops it. (d) What he sees of the sound stays: the waveform, the signal meters and the tempo. The band meters, the dB level, the onset flash, the genre line and the spectrum stay off screen; whether and where they come back is settled in the UI redesign (L8, L124). (e) What is remembered: the show holds nothing of the audio setup, and the input mode, the file and the gain are remembered nowhere, so the app opens on the microphone input, as he was shown in topic F's reading on what a show keeps; the one new thing, the picked input, is kept by the computer for every show (K-10).
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

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME K3-1
ABOUT: 273; R204, 215
TEXT: I assume an audio file the app plays is the music the app listens to, with its own play control: not a clip on a layer, and nothing connects it to the BPM. So tempo stop stops it, and tempo pause never pauses it.
WHY: BF271 reasons from "all clips from all layers" and says "unless it is connected to the BPM"; what would connect an audio file to the BPM is in no line. In his mouth the phrase means what follows the beat clock ("everything that is connected to BPM shifts forward or back. I mean everything.", binding-decisions.md:859-860); for a clip that would be BPM mode (INFERRED). He also writes "audio clips" for the files he will bring (BF272). So he may see an audio file as a clip, while the build has it as the sound source only.
ALT: b) An audio file can also be a clip in a cell, with Timeline mode and BPM mode like a video: tempo stop takes it off with the other clips, and in BPM mode tempo pause pauses it. c) It is not a clip, but it gets one switch that connects it to the BPM: with it on, tempo pause pauses the music and tempo play carries it on.
IF-WRONG: REBUILD audio clips in cells would be a new kind of clip (how it plays, how it is stopped, saved and recorded), not a small change; and on stage the music would pause, or play on, against what he expects
ASK: YES no words of his settle it; it is about what the app does; way b is a large piece of work he may be taking for granted
@@END

@@ASSUME K3-2
ABOUT: 273; R204, 215
TEXT: I assume that after tempo stop the audio file is back at its beginning and stays silent until you start it with its own play control; tempo play or a triggered clip does not start it.
WHY: BF271 says the stop stops it; where it stands afterwards and what starts it again is in no line.
ALT: b) The audio file starts again together with the beat: on tempo play or on the first clip you trigger. c) It keeps its place and goes on from there when you start it.
IF-WRONG: STAGE silence, or music, where he expects the other after a stop; a small change to undo
ASK: LINE a real choice of Harmony's that follows his "it would stop"; he can strike it
@@END

@@ASSUME K3-3
ABOUT: 247; R228, R202
TEXT: I assume tempo stop also takes away a MilkDrop picture you put on the output by clicking a preset: the output is black until you click a preset again or trigger a clip.
WHY: BF265 keeps MilkDrop as it is and the tempo stop is new, so no line of his names this meeting. His own words carry the reading: BF271 "Stop removes all clips from all layers so it would stop." (he reasons that the stop takes everything, the audio file too) and "The tempo stop button stops all actions as well as everything else." (BF196, binding-decisions.md:1155).
ALT: b) Tempo stop removes only clips: MilkDrop that you clicked in its tab stays on the output.
IF-WRONG: STAGE with way b he presses stop and MilkDrop is still on the output; with the text it is gone when he wanted it kept, and one click brings it back
ASK: NO his words on the stop carry it ("everything else"), and the other way is the one that would hurt on stage; if topic A's line on the stop is printed, six words there cover it
@@END

@@ASSUME K3-4
ABOUT: 247; R202, R212
TEXT: I assume your MilkDrop design comes to me whenever you have it, and until then nothing of MilkDrop is planned, changed or built, its labels included.
WHY: BF265: "When I have time while you are building, I will design a whole system for Milk drop"; when and how it is handed over is not said.
ALT: b) Harmony drafts a proposal for the new MilkDrop system for him to correct.
IF-WRONG: SMALL a draft he did not ask for, or a wait
ASK: NO the order of work; his words already say who designs it
@@END

@@ASSUME K3-5
ABOUT: 211; page item 238
TEXT: I assume a clip of many pictures never loses a picture in BPM mode: all its pictures are spread over whole groups of 4 bars, or each stays a set number of beats, and none is cut off the end.
WHY: With "b" (BF258) he chose the page's way b for item 238, which cuts a new BPM-mode clip's end so that it is whole groups of 4 bars and "plays at exactly its normal speed" (the page's words); a clip of many pictures has no normal speed, and his words do not name it. Default A of question 211, which he took (BF154: "All defaults good except for these."), keeps such a clip spreading all its pictures over its length. Topic H's ruling builds on this reading (its item 238, point 10) and leaves the asking to this line.
ALT: b) A clip of many pictures is cut like a video: the pictures past the last whole group of 4 bars are not shown.
IF-WRONG: STAGE pictures missing from the end of a slideshow, or a slideshow that is not cut where he wanted it cut; small to change
ASK: LINE it is where his answer to item 238 meets the many-picture clip; topic H leaves it to this line, and he would most likely wave it through
@@END

@@ASSUME K3-6
ABOUT: 247; R228
TEXT: I assume a MilkDrop picture you put on the output by a click stays until a clip really starts: while a triggered clip in BPM mode waits for the 1, MilkDrop keeps showing, and the clip takes its place on the 1.
WHY: BF265 keeps MilkDrop as it is; the wait for the "1" is new, so no line of his names this moment. The reading follows the default he took for a waiting clip (item 173 of topic A; BF154: "All defaults good except for these."): what was showing goes on until the "1".
ALT: b) The MilkDrop picture goes at the trigger: the output is black until the 1.
IF-WRONG: STAGE up to a bar of black before the first clip comes in
ASK: NO it follows the default he took for a waiting clip; the other way is a gap of black that nobody would choose
@@END

@@ASSUME K3-7
ABOUT: R213, R203
TEXT: I assume the autopilot never brings a clip onto an empty layer: after tempo stop, or after you cleared a layer or deleted its playing clip, a layer on autopilot stays empty until you trigger a clip there.
WHY: BF271 "Stop removes all clips from all layers so it would stop." empties every layer; what a layer on autopilot does then is in no line, and topic A's paper left it to this topic. The reading he left standing in round 1 has the autopilot bring "the next clip", which needs a clip on the layer; BF256 has a deleted or pasted-over clip leave its layer.
ALT: b) When the beat runs again, a layer on autopilot brings a clip by itself.
IF-WRONG: STAGE clips would come on by themselves after a stop, or a layer he expected to fill stays empty until he triggers it
ASK: NO it follows his own sentence on the stop and the autopilot he left standing; a layer that fills by itself after a stop would go against "Stop removes all clips"
@@END

@@ASSUME K3-8
ABOUT: 247; page item 236
TEXT: I assume a snapshot does not bring back the MilkDrop preset that was showing: MilkDrop stays as it is, and nothing remembers which preset was on. The picture kept with the snapshot shows how the moment looked.
WHY: BF243 has a snapshot save "the show exactly where it is with all the settings and the output and everything"; BF265 keeps MilkDrop as it is, and as it is no show keeps which preset is showing and a MilkDrop clip loads no preset of its own when it is triggered (docs/claude/milkdrop.md 1.8 and 1.10 point 1; read, not run). His two answers of this round meet here and neither names the other.
ALT: b) A snapshot remembers the preset that was showing and loads it when the snapshot is opened: likely a small change to MilkDrop in the coming build.
IF-WRONG: SMALL he opens the snapshot of a MilkDrop moment and sees another preset; the exception is small to add later
ASK: LINE a limit of the snapshot he should know before he relies on it; his own answer on MilkDrop is the text and the exception is way b; it borders topic F's snapshot question (F3-6) and can ride on it
@@END

@@ASSUME K3-9
ABOUT: R201; page item 243
TEXT: I assume a generated picture that reads the beat by itself does as effects that read the beat do: while the beat is paused or stopped, the part of it that pulses on the beat holds still, and the rest keeps moving.
WHY: Item 243, which he accepted, names effects (a strobe, a pulse); about 52 generated pictures name a beat uniform in the program text (topic I's ruling; read, not run), and no line of his names them. His rule carries the reading: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." (BF167); way b of item 243 (they keep pulsing through a pause) is the one he did not take. Whether such a picture gets a Sync like those effects (BF244) is not in his words; it waits on topic I's question about the Sync (its I3-6).
ALT: b) Generated pictures keep pulsing on the beat the app hears, also while the tempo is paused or stopped.
IF-WRONG: STAGE a picture that goes on pulsing during a pause, or holds still when he expected it to pulse; small to change
ASK: NO his rule on the pause and the item he accepted for effects carry it; it is the same answer as for effects, so there is nothing new for him to read
@@END

@@ASSUME K-2
ABOUT: 215
TEXT: I assume Render is one button in Studio that makes a film file of the recording you have open: the whole recording, at the size of the full composition, with the sound that was recorded with it.
WHY: BF241 names "an HD render from the Recording Review", so Render stands on his word and the old way "Render is gone" falls; "HD" is read as the full composition size. Sound in the film is his September yes ("do this but triage the correct build order.", binding-decisions.md:235), and item 229, which he accepted, plays the show recording with its sound; BF253 "It has no sound." speaks of the low-resolution show recording only. Still open: the whole recording or a stretch of it.
ALT: b) Render makes a film only of the stretch between the In and Out points.
IF-WRONG: SMALL a planned thing, not built yet
ASK: NO what Render makes returns to him with Render's plan (item 215: none of the six is built before its own plan is clear); nothing of it is in the coming build
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
TEXT: I assume only the two picture buttons and a picture dropped outside the grid stop feeding the output; the remote-control and test calls that load a loose picture keep working, and so does a click in the MilkDrop tab.
WHY: BF265 keeps the MilkDrop click; the calls the test tools use for the same slot are in no line.
ALT: b) The slot goes altogether except for MilkDrop, and the test tools are rebuilt on clips.
IF-WRONG: SMALL the test tools would need rework; nothing he sees
ASK: NO technical; the remote-control port is not his concern
@@END

## CLOSED ASSUMPTIONS (one line each)
- K-1 -> SETTLED
- K-3 -> page 2, item 247
- K-10 -> page 2, item 272
- K-11 -> page 2, item 273
- K-19 -> CLOSED by page 2 (K): The MilkDrop document is written and has its place (docs/claude/milkdrop.md, with a trigger row in CLAUDE.md); page 2 part 4 told him so and BF265 ("I will design a whole system for Milk drop") builds on it without asking for anything else.

## QUESTIONS BACK
@@ANSWER general
HIS: BF272 "after this round of questions, I will give you 3 10 min audio clips and a longer set to look at. are mp3 and m4a files ok or do prefer a certain format?"
ANSWER: Yes, MP3 and M4A are both fine: send the files as you get them. I need no special format. If the DJ can give you WAV or AIFF just as easily, take those: they are the safest. Do not convert anything yourself. One honest note: I have not played an MP3 or an M4A in the app yet. If one does not open, I make a WAV copy of it myself, with a tool that is already on this Mac, and your files stay untouched. I take these to be the tracks I asked you for, about ten minutes each, plus one longer set: I use them to measure how well the app finds the tempo and the 1 on your music, before anything about automatic mode is promised. Say so if you meant them for something else.
@@END

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

@@NAME audio file
MEANS: A sound file played in the app as the sound source.
SOURCE: the name in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, kept as it is; he writes "audio clips" for the files he will bring (BF272), which is not a new name; whether an audio file can also be a clip is asked (K3-1)
@@END

@@NAME Review
MEANS: A word that is no longer used: his earlier name for the screen that is now called Studio (long form: Show Recording Review). Use: Studio.
SOURCE: his words BF245 ("Let's go with studio. That's perfect.") and BF263 ("b"); the same MEANS as topic J's block, which owns the name; it replaces this topic's old row ("... where Render will sit")
@@END

@@NAME Render
MEANS: Making a film file in full quality afterwards from a show recording, in Studio; his "HD render".
SOURCE: his words (BF241: "we can make an HD render from the Recording Review"); until now the name was Harmony's pick; the same MEANS as topic E's block, on purpose
@@END

## REACHES OTHER TOPICS (from the paper)
- A (R176 a, R214, R216): BF271 "Stop removes all clips from all layers" is A's rule for the stop; applied here only to the audio file (item 273). A's paper says the same of K (apply-A.md, its bullet on item 273).
- A (R176 a, R216): if K3-3 holds, the tempo stop also takes away MilkDrop that was put on the output by a click in the MilkDrop tab; if he strikes it, A's "with no clip playing the output is black" gains MilkDrop as an exception after a stop.
- A (the pause, old A-3): BF271 "A pause would not pause it unless it is connected to the BPM" restates his rule of 2026-10-07 that a pause holds what the BPM controls; it supports the reading that a clip not in BPM mode plays on through a pause. Not ruled here.
- E (page item 259, record show runs on through a tempo stop; R165, opening Studio is like a press on stop): the audio file stops at a tempo stop, so a show recording that runs on records silence from there if the audio file was its sound; and opening Studio stops the audio file if it stops the show like a press on stop. E rules both.
- E (the old reading on Render): 215 part f now reads "an HD film file ... in Studio" (BF241, BF245); E's own Render reading should say the same. K-2 no longer offers "Render is gone".
- H (page item 238, BF258 "b"): whether the cut to whole groups of 4 bars applies to a clip of many pictures (old item 211); K reads it as no (K3-5).
- B (page item 252, old B-11): BF265 "Keep Milk drop as it is" also means no second MilkDrop for the preview; B's paper already applies it.
- F (BF243, the snapshot that saves "all the settings"): item 272 keeps the audio input out of the show; whether a snapshot holds it is F's, and K's reading is that it does not.
- J (NAMES.md): the row Review is replaced by Studio (BF245, BF263); the rows Audio Input, audio file, Loop File and Audio play / pause stand. Applied here from J's box: the name Studio.
- D (BF251, a play-once action plays again "only when that clip is re-triggered"): a trigger by the autopilot is a trigger like his own (R213 a), so it plays the clip's play-once actions again. No change to R213; D rules the action.

## CONFLICTS (from the paper)
- MilkDrop's click: new, BF265 "Keep Milk drop as it is." against the reading R228 a that he left standing on 2026-10-07 (MilkDrop reaches the output only through a cell that plays; binding-decisions.md line 1199, "THE READINGS HE DID NOT NAME ... stand as shown"). His newest words win; it was asked as item 247 and is answered, so there is nothing to put to him again.
- Who designs the new MilkDrop and when: new, BF265 "When I have time while you are building, I will design a whole system for Milk drop" against binding-decisions.md line 1194: "we will do that as a dedicated session where I will design the UI and how we will use it but not right now". A shift of when, not of what: he designs it either way. Newest words win (R202 part 3 amended).
- The audio file and the stop: no conflict with words of his. L72 (binding-decisions.md line 1155: "The tempo stop button stops all actions as well as everything else.") agrees with BF271; what falls is Harmony's narrower reading of L72 (old K-11).

## NOT DONE / UNSURE (from the paper)
- Which audio file types open: READ, NOT RUN. The file window's list is "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg" (MainComponent.cpp:310 and :547); the reader is JUCE's registerBasicFormats (AudioEngine.cpp:8, :60), which on a Mac also registers Apple's Core Audio reader (juce_AudioFormatManager.cpp:76-78 in build/_deps). INFERRED and not claimed to him: that reader would open an M4A, and also the MP3 if JUCE's own MP3 reader is off; no JUCE_USE_MP3AUDIOFORMAT line was found in CMakeLists.txt. docs/claude/architecture.md:169 lists WAV/AIFF/FLAC/MP3/OGG. Cheapest check when the build hold ends: open one MP3 and one M4A through the Audio File entry. Until then an M4A is converted with /opt/homebrew/bin/ffmpeg (present on this Mac, seen by ls; not run).
- "connected to the BPM" (BF271): not settled; K3-1 asks. If he means way b (an audio file as a clip in a cell), that is a new kind of clip and topics H and A must rule its playback.
- Whether the new tempo stop clears MilkDrop's loose picture (K3-3) depends on how topic A's stop is built (a clear of every layer refreshes the loose picture away, docs/claude/milkdrop.md line 46: INFERRED there). Cheapest: decide it in the stop's plan and say it on his page as one line.
- The old assumptions K-7, K-9, K-12, K-13, K-14, K-16, K-20, K-21, K-22 were tested against his 33 boxes and left untouched: no box of his speaks of the time per picture, the autopilot's count on a still, the Text source, OSC, Link, Play Once and Eject in a pause, the order of the six planned things, Loop File or the return of a pulled-out input. BF270 (endless knobs) is MIDI, not OSC; BF246 (the app tries to find the 1) is automatic mode without Link.
- A clip deleted or pasted over while the autopilot holds it (BF256: it "does not play"): the layer goes empty and the autopilot has nothing to move on from until he triggers a clip. Read from R203 and D36 as they stand, not ruled; topic F owns the delete.
- TODAY lines are taken from the old spec and the fact sheets except the audio file path and the MilkDrop pick, which were read in the docs and source named above. Nothing was run.

## FOR THE PAGE RULING (from the ruling)
- ONE QUESTION from this topic: K3-1 (YES), what "connected to the BPM" means for an audio file. Print it right after ANSWER general: his "audio clips" (BF272) and his "all clips from all layers" (BF271) point to its way b, an audio file as a clip in a cell. If he takes b it is a new kind of clip: topics H, A, F and E must then rule how it plays, stops, is saved and is recorded, and item 273 parts 2 to 5 are written again.
- LINES, by weight: K3-2 (after tempo stop the audio file is back at its beginning and silent until he starts it; print it under K3-1, same subject). K3-5 (no picture of a many-picture clip is cut in BPM mode; topic H's item 238 point 10 hangs on it: asked once, here). K3-8 (a snapshot does not bring back the MilkDrop preset that was showing; it can ride as one clause on topic F's snapshot question F3-6).
- NOT FOR HIM (NO): K3-3 (the tempo stop takes a clicked MilkDrop picture away; carried by BF196 and BF271; if topic A's line A3-3 is printed, add "MilkDrop you clicked goes too"), K3-4, K3-6, K3-7, K3-9, K-2, K-23. Old internal ones untouched: K-7, K-9, K-12, K-13, K-14, K-16, K-20, K-21, K-22.
- ANSWER general is owed and must be printed: he asked. It says yes to MP3 and M4A and says plainly that neither was tried in the app.
- NEWEST WORDS AGAINST EARLIER ONES, one line to say back, no decision needed: he now designs the new MilkDrop himself, while the build runs (BF265), where he had said "we will do that as a dedicated session where I will design the UI and how we will use it but not right now" (BF235). Until his design arrives nothing of MilkDrop is planned, changed or built, its labels in bars included (R212; K3-4).
- OVERTURNED READING, nothing to put to him again: R228 part a (a click in the MilkDrop tab no longer reaches the output) was a reading he left standing; BF265 answers it as item 247.
- DEPENDS ON A: the stop (R176 a as amended there) is the ground of item 273, K3-3 and K3-7. Topic X's AMEND N18 1 (a second stop still stops the audio file) leans on item 273.
- DEPENDS ON E: item 259 and AMEND R165 2 lean on item 273 and K3-2 (silence after a stop; opening Studio stops the audio file). E3-3 (the Video box) decides how early Render must come in this topic's order of work (K-20). E's NAME Render and this topic's are word for word the same.
- DEPENDS ON F: item 236 leaves the picked audio input out of a snapshot, as item 272 has it. K3-8 borders F3-6: if he answers F3-6 with way b or c (a snapshot does not bring the clips back), K3-8 falls away.
- DEPENDS ON H AND I: H's item 238 point 10 follows K3-5. AMEND R203 1 says only that the autopilot's count per kind of layer stays; it was written before topic I's ruling was finished (its paper and its checker both read BF260 so). If that ruling takes a layer's kinds out after all, R203 part c follows it. Topic I's ruling (its H2) leaves the generated pictures that read the beat to this topic: AMEND R201 1 and K3-9 (NO) hold their beat part still in a pause or a stop, as item 243 does for effects; if his answer to I3-6 gives effects a Sync, whether these pictures get one too is a new question, not asked now.
- NOT MEASURED, and not said to him as fact: whether an MP3 or an M4A opens in the app. Read, not run: the file window lists wav, aiff, aif, mp3, flac and ogg (MainComponent.cpp:310, :547); JUCE's own MP3 reader is off by default and not switched on in CMakeLists.txt, so on a Mac an MP3 or M4A would go through Apple's reader (juce_AudioFormatManager.cpp, registerBasicFormats). Cheapest check when the build hold ends: open one of each. ffmpeg and afconvert are on this Mac (seen by ls, not run).
- MERGE: this topic amends R228 (1 to 4), 215 (1 to 4), R202 (1, 2), R204, 211 and R203, and R213 and R201 (both appended). No other topic's paper or ruling amends these items (grep over all papers and the rulings written so far; topic I's ruling was not finished). Every OLD text was found exactly once in its old RULE line.

