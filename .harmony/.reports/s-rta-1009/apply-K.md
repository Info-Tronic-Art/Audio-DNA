# APPLY K -- Sources and the automatic features (s-rta-1009, page 2)
## SUMMARY
- MilkDrop (247, BF265): "Keep Milk drop as it is." The one MilkDrop change the old spec carried falls: a preset clicked in the MilkDrop tab while no clip plays keeps showing on the output (R228 a and the last sentence of R202 amended).
- MilkDrop's new system: he designs it himself, on his own time, while the build runs (BF265); it is no longer a session to be scheduled (R202 part 3 amended).
- The audio file (273, BF271): the tempo stop stops it; the tempo pause does not pause it. The page's assumption (the music plays on) is replaced; R204 c and 215 d amended.
- "unless it is connected to the BPM" is left open by his words, and his reasoning from "all clips" suggests he may see the audio file as a clip: ONE question for him (K3-1).
- The audio input (272): accepted as written; the computer remembers the pick, a show does not.
- Render (215 f): his BF241 confirms an HD render made from the review screen, which is called Studio now (BF245); K-2 reworded, its way "Render is gone" removed.
- His question on file types (BF272): answered from the program text, read not run.
## ITEMS
@@ITEM 247
TITLE: MilkDrop stays exactly as it is; a clicked preset still shows
STATUS: ANSWERED in his own words: the item as written, not way b
HIS: BF265 "Keep Milk drop as it is."
RULE: Nothing of MilkDrop changes in the coming build, and that includes how it reaches the output. A preset picked by hand in the MilkDrop tab (a click on its row, and the tab's "<", ">" and "?" buttons) changes the preset and, while no clip plays on any layer, shows MilkDrop on the output; while clips play, the pick only changes the preset. MilkDrop put on the output this way leaves it as before the build: when a clip is next triggered, or a layer or the deck is cleared. The rule that the output is black when no clip plays (@@ITEM R228 as amended below) holds for every other loose picture: Open Image, Image Folder and a picture dropped outside the clip grid still go. Everything else of MilkDrop stays with @@ITEM R202 as it stands, parts 1, 2, 4 and 5: the browser, the Jukebox, a clip's own playlist, picking by hand, Random and Bag, favourites, both Blend sliders, the reaction to a build-up, drop or breakdown, how a playlist is saved; a MilkDrop clip is a clip like any other for copy, paste, Option-drag, the start of a stopped beat and the tempo stop; the Jukebox and a clip's playlist do not move on while the beat is paused or stopped. The new MilkDrop system is designed by him, when he has time while the build runs; nothing of MilkDrop is planned or built before his design arrives (K3-4). Whether the tempo stop also takes away MilkDrop that was put on the output by a click is not in his words: built as "it does" until he says otherwise (K3-3).
CHANGED: Against the item as he read it: nothing in what it decides; way b (black output, MilkDrop only through a clip that plays) is rejected. Against the old blocks: R228 a said a click in the MilkDrop tab does not put MilkDrop on the output; that clause falls (amended). R202's "one open meeting point" is closed (amended). New in his words: who designs the smarter system and when ("When I have time while you are building, I will design a whole system for Milk drop"): R202 part 3 amended. Old assumption K-3 is closed by this answer.
TODAY: A hand pick calls loadPreset, makes MilkDrop the loose "active source" and clears the loose still (MainComponent.cpp:1724-1739; Renderer.cpp:688-720 draws "deck compositor > active source > loaded image"; both by docs/claude/milkdrop.md line 147, read not run). The loose MilkDrop picture lasts until the next fire of a cell or column or a clear of a layer or the deck (milkdrop.md line 46: INFERRED there). To change for this item: nothing; the removal of the old top row (R228, R211) must leave the "active source" path and the MilkDrop pick alone. The new tempo stop does not exist yet: whether it clears the active source is K3-3.
@@END
@@ITEM 272
TITLE: The computer remembers the picked audio input, not the show
STATUS: ACCEPTED as written
HIS: box left empty = accepted as written
RULE: Once the list Audio Input is built (@@ITEM 215 part c as it stands), the input he picks there is remembered by the computer and holds for every show: opening, saving or switching a show never changes which input the app listens to, and a show file holds nothing of it. At the start of the app the remembered input is used; if it is missing, the app listens to the built-in microphone without a word and goes back to the picked input when it returns (@@ITEM R204 parts a, b and e as they stand; K-22). A Bluetooth device is never offered and never opened. A show taken to another computer listens to whatever is picked on that computer.
CHANGED: nothing: accepted as written. Tested against his 33 boxes: BF243 (a snapshot saves "the show exactly where it is with all the settings") is about what a snapshot holds and is topic F's; no box says a show should hold the audio input. The old assumption K-10 had a third way (nothing remembers it), which the page did not show him; it falls with his acceptance.
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
## AMENDMENTS
@@AMEND R228 1
OLD: and apart from the camera of part b.
NEW: apart from the camera of part b, and apart from MilkDrop when a preset is picked by hand in the MilkDrop tab while no clip plays (part a).
HIS: BF265 "Keep Milk drop as it is."
WHY: The rule named two exceptions to the black output; his words keep MilkDrop's hand pick on the output, which is a third.
@@END
@@AMEND R228 2
OLD: a picture dropped on the window outside the clip grid, and a click on a preset in the MilkDrop tab (its "<", ">" and "?" buttons the same) do not put a picture on the output by themselves.
NEW: and a picture dropped on the window outside the clip grid do not put a picture on the output by themselves.
HIS: BF265 "Keep Milk drop as it is."
WHY: The click in the MilkDrop tab was listed among the loose pictures that no longer reach the output; he keeps MilkDrop as it is, so it leaves that list.
@@END
@@AMEND R228 3
OLD: a click in the MilkDrop tab changes the preset MilkDrop shows, and MilkDrop reaches the output only through a cell that plays. That last point is the one place where this reading meets L126 ("that's it for this upcoming build"): K-3, asked.
NEW: MilkDrop is the one loose picture that stays, by his words "Keep Milk drop as it is": a preset picked by hand in the MilkDrop tab (a click on its row, "<", ">" or "?") changes the preset and, while no clip plays, shows MilkDrop on the output; it leaves the output when a clip is next triggered or a layer or the deck is cleared, exactly as before the build (item 247). Whether the tempo stop takes it away too is K3-3.
HIS: BF265 "Keep Milk drop as it is."
WHY: The reading let MilkDrop reach the output only through a playing cell and asked him (K-3); he answered that MilkDrop stays as it is.
@@END
@@AMEND R202 1
OLD: (3) The smarter system is designed in a session of its own, where he designs the UI and how it is used; the document is that session's ground.
NEW: (3) The smarter system is designed by him, when he has time while the build runs: the UI and how it is used are his; the document is the ground for it. "That session" and "the MilkDrop session", here and in R212, mean the work that begins when his design arrives; nothing of MilkDrop is planned or built before it (K3-4).
HIS: BF265 "When I have time while you are building, I will design"
WHY: It spoke of a dedicated session still to come; his newest words say he designs the system himself while the build runs.
@@END
@@AMEND R202 2
OLD: The one open meeting point is a click in the MilkDrop tab while no clip plays: K-3.
NEW: A click in the MilkDrop tab while no clip plays keeps showing MilkDrop on the output (item 247; R228 part a). Whether the tempo stop takes that picture away is K3-3.
HIS: BF265 "Keep Milk drop as it is."
WHY: The meeting point was open and asked as K-3; his answer closes it.
@@END
@@AMEND R204 1
OLD: The tempo bar's play and pause never start or stop it, as he was shown; that its stop does not stop it either is Harmony's reading of L72, shown to him as a line (K-11).
NEW: The tempo stop stops it, with everything else the stop takes (his words: "Stop removes all clips from all layers so it would stop."). The tempo pause does not pause it ("A pause would not pause it unless it is connected to the BPM."); nothing in the build connects an audio file to the BPM, so the pause never pauses it (K3-1). Tempo play does not start it, as he was shown. After a tempo stop the file is back at its beginning and silent until he starts it himself (K3-2).
HIS: BF271 "Stop removes all clips from all layers so it would stop."
WHY: The reading had the stop leave the audio file playing (Harmony's reading of L72, shown as a line); he answered that the stop stops it.
@@END
@@AMEND 215 1
OLD: the tempo bar never starts or stops the file (K-11)
NEW: the tempo stop stops the file, the tempo pause never pauses it and tempo play never starts it (R204 part c as amended; item 273); a looping file that the tempo stop has stopped does not start again by itself (K3-2)
HIS: BF271 "Stop removes all clips from all layers so it would stop."
WHY: Part d said the tempo bar never stops the file; his answer to 273 has the stop stop it.
@@END
@@AMEND 215 2
OLD: (f) Render: a film file made afterwards, in Review, from the recording that is open there
NEW: (f) Render: an HD film file made afterwards, in Studio (the screen that was called Review; every "Review" in this part reads Studio), from the recording that is open there
HIS: BF241 "we can make an HD render from the Recording Review"; BF245 "Let's go with studio."
WHY: Part f placed Render in Review and K-2 still asked whether Render exists at all; he now names the HD render from that screen himself and has renamed the screen Studio.
@@END
## ASSUMPTIONS
@@ASSUME K3-1
ABOUT: 273; R204, 215
TEXT: I assume the audio file stays a thing of its own, not a clip on a layer, and nothing connects it to the BPM: tempo stop stops it, and tempo pause never pauses it.
WHY: BF271 reasons from "all clips from all layers" and says "unless it is connected to the BPM"; what would connect an audio file to the BPM is not said.
ALT: b) An audio file can also sit in a cell as a clip, with Timeline mode and BPM mode like a video; in BPM mode the tempo pause pauses it. c) The audio file gets one switch that ties it to the tempo bar: with it on, the tempo pause pauses the music too.
IF-WRONG: REBUILD audio clips in cells would be a new kind of clip, not a small change; and on stage the music would pause or play on against what he expects
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
WHY: BF265 keeps MilkDrop as it is; BF271 has the stop remove "all clips"; MilkDrop shown by a click is not a clip, and the tempo stop is new.
ALT: b) Tempo stop removes only clips: MilkDrop that you clicked in its tab stays on the output.
IF-WRONG: STAGE he presses stop and MilkDrop is still on the output, or it is gone when he wanted it kept
ASK: LINE the stop is his way to black; he would most likely wave it through
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
TEXT: I assume a clip made of many pictures never loses a picture in BPM mode: its pictures are spread over its beats, or each stays a set number of beats, and none is cut off the end.
WHY: BF258 ("b" to item 238) cuts a new BPM-mode clip's end to whole groups of 4 bars; a clip of many pictures has no normal speed, and his words do not name it.
ALT: b) A clip of many pictures is cut like a video: the pictures past the last whole group of 4 bars are not shown.
IF-WRONG: STAGE pictures missing from a slideshow, or a slideshow whose length is not whole groups of 4 bars
ASK: LINE topic H owns item 238; this is its meeting with the many-picture clip, one line he can strike
@@END
@@ASSUME K-2
ABOUT: 215
TEXT: I assume Render is one button in Studio that makes an HD film file of the recording you have open: the whole recording, at full composition size, with its sound.
WHY: BF241 names "an HD render from the Recording Review"; whether it takes the whole recording or a stretch, and whether it has sound, is in no line (BF253 takes the sound only from the low-resolution recording).
ALT: b) Render makes a film only of the stretch between the in and out points. c) The film has no sound.
IF-WRONG: SMALL a planned thing, not built yet
ASK: LINE he names the HD render himself (BF241); the line shows what it is taken to make
@@END
@@ASSUME K-23
ABOUT: R228
TEXT: I assume only the two picture buttons and a picture dropped outside the grid stop feeding the output; the remote-control and test calls that load a loose picture keep working, and so does a click in the MilkDrop tab.
WHY: BF265 keeps the MilkDrop click; the calls the test tools use for the same slot are in no line.
ALT: b) The slot goes altogether except for MilkDrop, and the test tools are rebuilt on clips.
IF-WRONG: SMALL the test tools would need rework; nothing he sees
ASK: NO technical; the remote-control port is not his concern
@@END
@@DROP ASSUME K-19
WHY: The MilkDrop document is written and has its place (docs/claude/milkdrop.md, with a trigger row in CLAUDE.md); page 2 part 4 told him so and BF265 ("I will design a whole system for Milk drop") builds on it without asking for anything else.
@@END
## QUESTIONS BACK
@@ANSWER general
HIS: BF272 "after this round of questions, I will give you 3 10 min audio clips and a longer set to look at. are mp3 and m4a files ok or do prefer a certain format?"
ANSWER: Both are fine. As I read the program, the app's own file window offers MP3, WAV, AIFF, FLAC and OGG; M4A is not on its list, so I make a WAV copy of an M4A with a tool that is on this Mac. Send what you have. They are for testing whether your corrections hold in automatic mode.
@@END
## NAMES
@@NAME Studio
MEANS: The screen where a show recording is watched, mended, recorded over and cut into actions, and where Render sits.
SOURCE: his words (BF245: "Let's go with studio."; BF263: "b"); replaces the name Review in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md (topic J owns the name; used here for 215 part f)
@@END
@@NAME audio file
MEANS: A sound file played in the app as the sound the app listens to; it is not a clip on a layer.
SOURCE: the name in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, kept; he writes "audio clips" for the files he will bring (BF272), which is why K3-1 asks whether he sees it as a clip
@@END
## REACHES OTHER TOPICS
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
## CONFLICTS
- MilkDrop's click: new, BF265 "Keep Milk drop as it is." against the reading R228 a that he left standing on 2026-10-07 (MilkDrop reaches the output only through a cell that plays; binding-decisions.md line 1199, "THE READINGS HE DID NOT NAME ... stand as shown"). His newest words win; it was asked as item 247 and is answered, so there is nothing to put to him again.
- Who designs the new MilkDrop and when: new, BF265 "When I have time while you are building, I will design a whole system for Milk drop" against binding-decisions.md line 1194: "we will do that as a dedicated session where I will design the UI and how we will use it but not right now". A shift of when, not of what: he designs it either way. Newest words win (R202 part 3 amended).
- The audio file and the stop: no conflict with words of his. L72 (binding-decisions.md line 1155: "The tempo stop button stops all actions as well as everything else.") agrees with BF271; what falls is Harmony's narrower reading of L72 (old K-11).
## NOT DONE / UNSURE
- Which audio file types open: READ, NOT RUN. The file window's list is "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg" (MainComponent.cpp:310 and :547); the reader is JUCE's registerBasicFormats (AudioEngine.cpp:8, :60), which on a Mac also registers Apple's Core Audio reader (juce_AudioFormatManager.cpp:76-78 in build/_deps). INFERRED and not claimed to him: that reader would open an M4A, and also the MP3 if JUCE's own MP3 reader is off; no JUCE_USE_MP3AUDIOFORMAT line was found in CMakeLists.txt. docs/claude/architecture.md:169 lists WAV/AIFF/FLAC/MP3/OGG. Cheapest check when the build hold ends: open one MP3 and one M4A through the Audio File entry. Until then an M4A is converted with /opt/homebrew/bin/ffmpeg (present on this Mac, seen by ls; not run).
- "connected to the BPM" (BF271): not settled; K3-1 asks. If he means way b (an audio file as a clip in a cell), that is a new kind of clip and topics H and A must rule its playback.
- Whether the new tempo stop clears MilkDrop's loose picture (K3-3) depends on how topic A's stop is built (a clear of every layer refreshes the loose picture away, docs/claude/milkdrop.md line 46: INFERRED there). Cheapest: decide it in the stop's plan and say it on his page as one line.
- The old assumptions K-7, K-9, K-12, K-13, K-14, K-16, K-20, K-21, K-22 were tested against his 33 boxes and left untouched: no box of his speaks of the time per picture, the autopilot's count on a still, the Text source, OSC, Link, Play Once and Eject in a pause, the order of the six planned things, Loop File or the return of a pulled-out input. BF270 (endless knobs) is MIDI, not OSC; BF246 (the app tries to find the 1) is automatic mode without Link.
- A clip deleted or pasted over while the autopilot holds it (BF256: it "does not play"): the layer goes empty and the autopilot has nothing to move on from until he triggers a clip. Read from R203 and D36 as they stand, not ruled; topic F owns the delete.
- TODAY lines are taken from the old spec and the fact sheets except the audio file path and the MilkDrop pick, which were read in the docs and source named above. Nothing was run.
