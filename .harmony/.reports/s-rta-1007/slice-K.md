# SLICE K -- Sources and the automatic features
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
211 212 215 R201 R211 R228 R202 R212 R203 R213 R204 D36 P35

## QUESTIONS (3)
### Question 211 -- Many pictures dropped: time per picture, spread, or one each   [NOT NAMED by him]
About: many pictures dropped on a cell
Situation: You select twenty pictures in Finder and drop them together onto a cell. Today three or more pictures become one clip that runs through them (exactly two become two clips; a folder itself cannot be dropped), and in BPM mode the clip spreads all its pictures over its length. A separate old row at the top (the button "Image Folder" with the list "Beats per Image", 2 to 128) shows one picture every so many beats; as far as I can see from how the app is built, only while no layer plays (not tried).
- A (THE DEFAULT) [mine]: One clip, as today, and new: in BPM mode you can also set how long each picture stays (a beat, a bar, 2 bars) instead of spreading them over the clip's length.  -- why the default: [mine] It keeps the one thing the old row can do and a clip cannot, a steady change of picture on the beat, and puts it inside your clip grid.
- B: One clip, as today and nothing new: its pictures are spread over the clip's length (4, 8, 12 bars), like a video.
- C: One clip per picture, side by side in the next empty cells, like videos.
Why it was asked: The app's own description promises folders for beat-synced slideshows; in your new clip panel a clip has a length, not a time per picture. It also decides whether three pictures stay one clip. What becomes of the old row at the top: R228.
Source (Harmony-side): area-clip-transport.md U-H14; area-sources-auto.md H12, H13, O1 (MainComponent.cpp:455-467, 843-880); src/MainComponent.h:355 ("Image Folder"), src/MainComponent.cpp:357 ("Beats per Image"): read by me; ClipCell.cpp:311-323 (no folder drop), Renderer.cpp:1718-1736 (spread over the length): by the seats

### Question 212 -- Seven things that do nothing today   [NOT NAMED by him]
About: what is on screen or promised and does nothing today
Situation: Seven things are on screen or promised and do nothing, or very little, that you can see today: (a) the camera (the "Camera" list in the top row works, but shows only while no clip plays; the "Camera Input" entry in the Sources tab draws nothing that I can find in the code, not tried); (b) Import ISF Shader (it says "Import Successful" and the shader never shows); (c) the genre the app detects; (d) what happens on a detected drop or breakdown (on screen only MilkDrop's Jukebox reacts, and the oscillator switch you ruled); (e) the "smart" autopilot; (f) the Composition tab's own Autopilot block; (g) the randomizer of effects.
- A (THE DEFAULT) [mine]: Later, all seven: the coming builds do not touch them and they stay as they are. After the builds we go through them one by one, each with its own questions.  -- why the default: [mine] The first builds stand on the tempo row, presets, the cue system, actions and the review screen; none of the seven is needed for them.
- B: Take all seven off the screen now, so that nothing on screen does nothing; each comes back when it is built.
- C: You sort them by letter, for example "212 c: a and b now, hide e". What you do not name is later.
Why it was asked: In September you told me not to hide or delete a dead control but to build it; for the ten slots you said delete. Each of the seven first needs your word on what it should do, and that is seven more sets of questions. "Later" is a full answer: the seven then do not stand in the way of "all is clear".
Source (Harmony-side): area-sources-auto.md H1-H6, H9, C1, C3, O1, O5; area-effects-signals.md E17, 1.23; BD:99-104

### Question 215 -- Six things that stay unless you use them   [his lines: L128]
About: six things that stay as they are unless you use them
Situation: Six things I plan to leave exactly as they are, because you have not asked for them lately: (a) OSC, messages from a controller program (only its routine address becomes an action address, R200 c); (b) Ableton Link; (c) picking the audio input inside the app (today it follows the Mac's own choice); (d) an audio file that loops (today, as far as the code shows, it plays once and stops); (e) a text source where you type words (today's Text Animator draws random digits); (f) Render: making a film from a recording afterwards, which you asked for in September (the Video box of your 48 b records the film live instead).
- A (THE DEFAULT) [as today]: None of the six is built or changed now.  -- why the default: [as today] None of them is needed for the builds that come first.
- B: You name the letters you use on stage or want, for example "215 b: a and f".
Why it was asked: Each of the six would otherwise be a "say so" at the end of a long reading, where it is easy to pass over (R178, R184, R200, R201, R204).
Source (Harmony-side): area-tempo.md U-HIS-7, M-3; area-controls.md U19; area-sources-auto.md H14-H16, H18, M3, M4; area-recording.md H5 (Render asked in September: BD:220-224 by the area sheet)

## READINGS (8)
### R201 (about: sources: stays as it is)   [his lines: L124]
Stays as it is unless you say otherwise. (a) The Sources tab with its generated pictures and fractals, each with its own sliders; MilkDrop with its own browser; still pictures; videos. (b) A source's cell shows no picture of the source (a coloured tile with the tag "SRC" and its name; whether it gets a picture comes with the pictures). (c) Today there is one working copy of each kind of generated picture. Two clips of the same kind keep their own slider values, but they share one running state: a picture with a memory (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics) cannot play on two layers at once without disturbing itself, and every MilkDrop clip shows the same preset (seen in how the app is built, not tried). What that means for a preview is said under the cue system. (d) The source called Text Animator draws rows of random digits; there is nowhere to type words (a real text source: question 215 (e)). What goes from the top and the bottom of the window: R211 and R228.
(label: "today's facts VERIFIED by the area sheet (one generator per kind of source, each draw with the clip's own sliders: Renderer.cpp:1160-1171, CompositorEngine.cpp:1442-1445, by two seats; the digit rain: EmbeddedShaders.h:11944-11975); the slots are reading R211")
(source: "area-sources-auto.md H11, H18, H21, C4, O3, T1; BD:898-899")

### R211 (about: the ten numbered preset slots)   [NOT NAMED by him]
The ten numbered preset slots in the bar along the bottom of the window (each a number button with a drop-down), and the three small buttons "Save", "Load" and "FX Save" in the top row, go, as you ruled (86). Nothing takes their place: a column is your scene, and each effect has its presets. Your show's own Save, Open and Collect Media stay. The app's own description still lists instant preset save and recall; that line goes with the slots.
(label: "his answer 86 (\"86 default yes\"); that nothing replaces the slots INFERRED; where they sit VERIFIED in the code by me (src/MainComponent.cpp:2784: a 28-point bar taken from the bottom; :2763-2767: the three buttons in row 1)")
(source: "BD:898-899; area-sources-auto.md C4, O3, T1")

### R228 (about: what the output shows when no clip plays; the old top row)   [NOT NAMED by him]
Mine: when no clip plays, the output is black (apart from what your 147 a keeps: R216). Today it can still show a loose picture, left from the app's first version (seen in how the app is built, not run): the last picture chosen with "Open Image" or "Image Folder" in the top row, a picture dropped on the window outside the clip grid, a preset clicked in the MilkDrop tab, or the camera picked in the top row's "Camera" list. (a) "Open Image", "Image Folder" with its list "Beats per Image", the drop outside the grid and the MilkDrop click stop feeding the output; the two buttons and the list go, with "Save", "Load" and "FX Save" (R211). (b) The "Camera" list stays where it is and keeps showing only while no clip plays, until you say what the camera should become (question 212 a). Say "R228 keep" if the loose picture and its buttons should stay as today.
(label: "INFERRED (his stop takes every clip off; the name click that reaches the same picture already goes: R150); the row's controls VERIFIED in the code by me (src/MainComponent.cpp:2749-2767; src/MainComponent.h:354-355; \"Beats per Image\" :357, \"Cam: Off\" :362); what feeds the loose picture: by two seats (MainComponent.cpp:1724-1737, 4217-4245), not run")
(source: "area-sources-auto.md O1, H1, H13; BD:952-953, 1007, 1072")
(star: true)

### R202 (about: MilkDrop: stays, with truthful labels)   [his lines: L126]
Stays as it is unless you say otherwise. (a) The three ways of changing MilkDrop's presets: the Jukebox, a clip's own playlist, by hand. (b) Random and Bag (MilkDrop's two ways of shuffling) and favourites. (c) The Blend slider stays in seconds; the Blend slider of a clip's own playlist does nothing today (the Jukebox's does fade) and is made to fade. (d) One thing you may not know: while the Jukebox runs, a change in the music's structure (a build-up, a drop, a breakdown, back to normal) already changes the preset on the next bar, never sooner than four bars after the last change, and it picks an intense preset on a drop and a calm one on a breakdown. That stays; say so if you want a switch for it. (e) A MilkDrop clip's list of presets is saved in the show as the places of the preset files on this computer, and Collect Media does not copy those files: on another computer the clip stays but its presets do not load (seen in how the app is built, not tried). Say so if the preset files of your clips should travel with the show. What changes in the labels of the Jukebox list: R212.
(label: "his words (151 A); the drop: VERIFIED by the area sheet (PresetSelector.cpp:86-92); the labels are reading R212")
(source: "area-tempo.md U-HIS-12; area-sources-auto.md H10, C1, C3, M1, M2, O4; BD:645, 649, 675, 1078")

### R212 (about: the labels of MilkDrop's Jukebox list)   [NOT NAMED by him]
Your words: bars, and no seconds. Today the Jukebox list reads "4 beats", "8 beats", "16 beats", "32 beats", "30 sec", "60 sec", and by the code (read, not run) it changes after 4, 8, 16, 32, 32 and 64 bars. Mine: the list reads what it does (4, 8, 16, 32 and 64 bars). The picture changes as often as today; only the labels change. The list under a clip's own MilkDrop playlist reads "4 beats", "8 beats", "16 beats" and "32 beats" and does count beats; it will read 1, 2, 4 and 8 bars. The playlist's triggers on the music's structure (on the drop, on a breakdown, per phrase) exist only in the saved file and have no control: they stay without one unless you ask.
(label: "his words (bars; no seconds); the list's texts VERIFIED (src/ui/MilkDropBrowser.cpp:563-575); what each entry really waits: seen in how the app is built and re-derived by two seats (src/sources/PresetSelector.cpp:79-83, 169), not run; the playlist list: src/ui/MilkDropBrowser.cpp:611-615, by the seats")
(source: "area-tempo.md U-HIS-12; area-sources-auto.md H10, C1, M1, M2; BD:645, 649")

### R203 (about: the autopilot: stays, and how it meets the tempo row)   [NOT NAMED by him]
Stays as it is unless you say otherwise: (a) a layer's autopilot (the next clip after a count, or at the end of a video; in order or at random; at the end of the row it starts again from the first clip; at random it may return to the clip it has just left); (b) a clip's own list of what comes next; (c) the count per kind of layer. These are different tools from actions, and both stay. What changes where your new rules meet the autopilot: R213.
(label: "today's behaviour VERIFIED by the area sheets (Autopilot.cpp:74-76, 243-282, 326, 355, 417); no word of his names the autopilot; what changes is reading R213")
(source: "area-sources-auto.md H7, H8, M7, W1; area-clip-transport.md U-H18, U-H22; area-effects-signals.md E17, O13; area-actions.md H11; area-tempo.md M-5")

### R213 (about: the autopilot under your new rules)   [NOT NAMED by him]
Mine, where your new rules meet the autopilot. (a) An autopilot's fire is a fire like yours: a video starts from its beginning and a BPM-mode clip starts on the "1". Today the autopilot resumes a clip where it was and brings it in at once. (b) The autopilot counts on the running beat: while the beat is paused or stopped it does not move on, and it never starts a stopped beat. (c) Its counts read in bars (R196). (d) A BPM-mode clip the autopilot brings in starts on the next "1", and its count starts there, so a count shorter than a bar still changes on a "1". (e) While the beat is paused or stopped the autopilot does nothing at all, "End of Video" included: a video that ends then stays on its last frame until the beat plays.
(label: "today's behaviour VERIFIED by the area sheets (Autopilot.cpp:74-76, 243-282, 326, 355, 417); no word of his names the autopilot; all three INFERRED from 154, his restart and the decided line on who starts a stopped beat")
(source: "area-sources-auto.md H7, H8, M7, W1; area-clip-transport.md U-H18, U-H22; area-tempo.md M-5")
(star: true)

### R204 (about: the sound the app listens to)   [NOT NAMED by him]
Stays as it is unless you say otherwise. (a) The app listens to a wired input or the built-in microphone, never to a Bluetooth device (your rule); it follows the Mac's own choice of input, and there is no list of inputs in the app. (b) If a wired input is pulled out during a show, the app goes on listening to the built-in microphone, without a word. (c) An audio file can be played instead: it starts when you choose it, plays once and is heard on the Mac's output (seen in how the app is built, not tried); the tempo row's play and pause never start or stop it ("Audio play / pause" in the keyboard and MIDI mapping does). (d) What you see of the sound stays: the waveform, the signal meters and the tempo. The band meters, the dB level, the onset flash, the genre line and the spectrum are not on screen today (their panel is switched off) and stay off; whether and where they come back is for the pictures. Picking the input in the app, or a file that loops: question 215 (c) and (d).
(label: "his rule (no Bluetooth; a wired input or the built-in microphone); today's behaviour VERIFIED by the area sheet, that the file plays once INFERRED there (no looping call in the code)")
(source: "area-sources-auto.md H14, H15, H16, M3, M4, M8; area-tempo.md U-HIS-17; BD:485-488")

## DECIDED WITHOUT ASKING (1) -- he did NOT read these: each is still Harmony's assumption until his words settle it
### D36: The autopilot moves on from a generated picture, a still or MilkDrop the same as from any clip, also after you paused it, stopped everything or fired it a second time (today it can stay stuck there).
(decided by: area-sources-auto.md T4 (Layer.h:553-556, Autopilot.cpp:100: lines read there; the chain INFERRED).)

## COMES NEXT AS PICTURES (1) -- he did NOT read these; his new rule on layout is L8
### P35: A generated picture's cell.
(variants that were to be drawn: as today: a coloured tile with the tag "SRC" and the name | a still picture of the source, taken once when the clip is made | a small live picture (costly: every source runs once more per frame))

