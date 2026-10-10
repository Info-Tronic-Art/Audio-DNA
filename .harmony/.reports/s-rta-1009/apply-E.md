# APPLY E -- Studio and the recordings (s-rta-1009, page 2)
## SUMMARY
- The low-resolution show recording is wanted and has a purpose now: it runs by default next to every show recording, WITHOUT sound, "so that we can see if there is a mistake in the parameter recording" (BF253); it is "just for reference" (BF241).
- Its size is open again: he accepted a quarter (item 262, box empty) and asks in another box "Maybe something close to the output monitor resolution?" (BF241). Asked once (E3-1). Its rate (30 or 15 pictures a second) and its codec are tested after the build (BF241, BF240; the codec test is topic H's).
- A film in full quality is made afterwards with Render in Studio (BF241); Render was already planned (old item 215 f, topic K). Whether the Video box of Record Show stays beside it: E3-3.
- Record to clip: way c (BF254): it stops at the end of the bar; the clip loops only whole groups of 4 bars; the rest stays in the file.
- A clip made with record to clip is "exactly the output recorded as one layer" (BF268): a full picture, no see-through parts, global effects included.
- Accepted as written: Record Over is a button in Studio and takes everything in the keyboard and MIDI mapping (229, 257); a saved action goes into the show the recording was made in (230); record to clip waits for the 1 (258); record show runs through pause and stop (259); adding and taking out are recorded (260); the recordings run together, the output first (261).
- What Studio does with the low-resolution show recording (shows it beside its own picture, or leaves it to a film player) is not in his words: E-23, asked.
## ITEMS
@@ITEM 229
TITLE: Record Over is a button in Studio
STATUS: ACCEPTED
HIS: box left empty = accepted as written; the screen's name: BF245 "Let's go with studio. That's perfect."
RULE: Record Over is a button in Studio. A press plays the show recording that is open there in real time, with its sound and its recorded moves; the Studio picture and the outputs show what the moves do; what is moved on the MIDI controller while it plays is recorded over the moves of those same controls. It never runs in the live window and never through the live layers, and a show recording is played only in Studio, never inside the live window. A control that is moved supersedes its recorded track while it is held and goes back to the recorded track when it is let go; the Record Over is kept separately from the original, beside it, with the same sound; it is not available while a record show or a record to clip runs: with @@ITEM 195 as it stands (as amended below). Which controls record over: item 257.
CHANGED: nothing: accepted as written. The item's word "Review" is Studio now (BF245, BF263). That a show recording plays only in Studio is the other half of old assumption E-2 and of Harmony's answer on page 2 ("you watch a whole recording in Review"), whose box he left empty: INFERRED as accepted with it.
TODAY: "Record Over" shows only while a recording replays with its sound and runs through the live layers; its only door is "Play Take" (old item 195 TODAY: AR 1.1, H8, O2). No Studio screen exists. Both are to be built.
@@END
@@ITEM 230
TITLE: Where an action saved in Studio goes
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: An action saved in Studio goes into the show the show recording was made in, as that show is by then, onto the same clip, wherever that clip sits by then (another cell, another deck). If that clip is no longer in that show, the app says so and nothing is saved. It never goes into whatever other show is open in the live window: if another show is open there, the app says which show to open before the action can be saved. An action that belongs to a layer or to Global goes to that layer or to Global of the same show. The action is written to the show file at the next Save, like any other change: with @@ITEM R173 as it stands (as amended below). How "the same clip" is recognised after it moved is one rule with the actions topic.
CHANGED: nothing: accepted as written (old assumption E-11 is closed by it). "onto the same clip" is extended to a layer's and Global's actions: INFERRED from the item, which names only a clip.
TODAY: A recording names no show and carries none; a replay is matched against whatever show is open (old item R173 TODAY: FT section 1, S6; AR 1.5, H24).
@@END
@@ITEM 231
TITLE: The low-resolution show recording runs by default, without sound
STATUS: ANSWERED in his words
HIS: BF253 "It defaults to running next to the show recording"; BF253 "It has no sound."; BF241 "This is just for reference."
RULE: The low-resolution show recording runs by default next to the show recording: with every record show it is on unless it has been switched off; it is one more box of Record Show, ticked in a new app, and the app remembers the tick like the other boxes. It has no sound, also when the Audio box is ticked. Its purpose is to show whether there is a mistake in the parameter recording: it is a film of the picture as it was during the show, to hold against the picture that Studio makes fresh from the recorded moves. It is just for reference and is never the film to keep or share; a film in full quality is made afterwards with Render in Studio (BF241). So that it can serve its purpose, every piece of it is kept with its show recording and carries the time of the show recording at which it begins, so that the same moment can be found in the film and in Studio. How Studio shows the two together: assumption E-23. Whether it runs when the Parameters box is not ticked: assumption E3-4. It shows the full picture, never one output screen's corrected picture, and is written in pieces so that a crash costs only the last seconds: with @@ITEM G4 as it stands (as amended below). The test on his Mac with a heavy show comes before it is left switched on by default: if that test shows that it costs the output its smoothness, he is told before anything is decided, and the output comes first (item 261). Its size: item 262. Its rate (30 or 15 pictures a second) and its codec: tested after the build (BF241).
CHANGED: The page's "with sound" is replaced by his "It has no sound." (the page's way c); "unless you untick it" stays as his "It defaults to running". New in his words: what it is for ("so that we can see if there is a mistake in the parameter recording"). The page's condition "once a test shows it is light" is not repeated by him and is kept as Harmony's safety from old item G4, which he read. "the parameter recording" is read as the Parameters box of Record Show, the recorded moves: INFERRED.
TODAY: Nothing of it exists. The only film recorder writes one full-size file without sound and without pieces, and its index at the end, so a crash leaves it unplayable (old item G4 TODAY: VideoRecorder.cpp, check-lowres facts 6-8; read, not run).
@@END
@@ITEM 232
TITLE: Record to clip ends at the bar; loops groups of 4
STATUS: ANSWERED c
HIS: BF254 "c"
RULE: Record to clip stops recording at the end of the bar in which stop is pressed; it does not run on until a group of 4 bars is full. The file holds every bar that was recorded. The clip that lands loops only whole groups of 4 bars: the app sets its out point at the end of the last full group of 4 bars (5 recorded bars loop as 4; 9 as 8), and the rest stays in the file, behind the out point, where the clip's out point can be dragged out to it again later. The clip is a BPM-mode clip whose number of beats is that of the looped part. A recording is never shorter than 4 bars, starts on the 1, and lands waiting in its cell: with @@ITEM R186 as it stands (as amended below).
CHANGED: The item's main text ("keeps every bar it recorded (5 bars stay 5); you trim it yourself") is replaced by its way c. Old item R186 (c) said that no out point is moved by itself: amended. It agrees with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (binding-decisions.md:812) and with his answer b to item 238 (BF258) for a clip brought in.
TODAY: Record to clip does not exist in the app (old item R186 TODAY: AR 1.18).
@@END
@@ITEM 256
TITLE: A recorded clip is exactly the output, as one layer
STATUS: ANSWERED in his words
HIS: BF268 "This is exactly the output recorded as one layer."; BF268 "Whatever the output is displaying is what this should look like."
RULE: A clip made with record to clip is exactly the output recorded as one layer: whatever the output is displaying is what the clip looks like. It is the full picture of all layers mixed together with everything Global does to it, the global effects included; where no layer shows it is black, as on the output. It has no see-through parts: played on a layer it is a full picture, and the layers under it are seen only through that layer's own blend mode and transparency slider. "The output" is the picture as the output monitor shows it, never one output screen's own Delay, Brightness, Contrast or colour: with @@ITEM R187 (d) as it stands. Its file type needs no see-through channel and is picked by the test of codecs after the build (topic H).
CHANGED: The item's main text is confirmed in his own words; way b (see-through parts) is not taken. Old assumptions E-9 and E-8 are closed by it; old items R186 (a), (e) and D22 are amended. "the output" is read as the picture and not as one output screen: INFERRED, from his earlier "not the screen output as screens can be modified" (binding-decisions.md:1170).
TODAY: Record to clip does not exist in the app; the film recorder reads the composition's picture back from the graphics card (old item D20 TODAY: AR H29).
@@END
@@ITEM 257
TITLE: What Record Over takes
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Record Over takes everything in the keyboard and MIDI mapping: knobs, faders, pads and keys, clip triggers too. What is moved with the mouse on screen is not recorded over. A knob or fader supersedes its recorded track while it is held (with @@ITEM 195 as it stands). A clip triggered during a Record Over is added to the triggers already recorded (assumption E3-5).
CHANGED: nothing: accepted as written (old assumption E-3 is closed by it). That a trigger is added to the recorded ones, and does not replace them, was in the old assumption's text and not in the item he read: carried as E3-5.
TODAY: Record Over takes keys, pads, MIDI, OSC and REST values, not the mouse (old item 195 TODAY: AR 1.8).
@@END
@@ITEM 258
TITLE: Record to clip pressed while the tempo stands
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A press on record to clip while the tempo is paused or stopped waits: the button shows that it waits, nothing is recorded, and the recording starts on the first 1 once the tempo runs. The press never starts the tempo by itself. A second press while it waits takes the wait back.
CHANGED: nothing: accepted as written (old assumption E-5 is closed by it). "shows that it waits" and "a second press takes the wait back" are not in the item: INFERRED, internal.
TODAY: Record to clip does not exist; a tempo pause / stop does not exist yet (old item R186 TODAY: FT SURPRISES S4).
@@END
@@ITEM 259
TITLE: Record show starts and stops at the press
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Record show starts at the press and stops at the press; it does not wait for the 1 and does not end on a bar line. It runs on through a tempo pause and a tempo stop: the moves, the sound and the low-resolution show recording keep being recorded, and the presses on the tempo bar are recorded with them (with @@ITEM R185 (b) as it stands). Only record to clip waits for the 1 and ends on a bar line.
CHANGED: nothing: accepted as written (old assumption E-10 is closed by it).
TODAY: Tempo commands are recorded as events; a tempo-row stop does not exist yet (old item R185 TODAY: AR 1.8, H26).
@@END
@@ITEM 260
TITLE: What is added or taken out while recording
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Anything added while a record show runs (an effect, a clip, a layer) is recorded from the moment it is added: it is put into the recorded show file, its controls get tracks from then on, and Studio shows it from that moment. Anything taken out while recording (an effect, a clip, a layer) is recorded at that moment: the removal is recorded, the thing is kept in the recorded show file, and from the removal on all its settings are at their defaults and its tracks are flat at default; Studio shows it working up to the removal and gone after it. With @@ITEM R223 (d) as it stands (as amended below).
CHANGED: nothing: accepted as written (old assumptions E-17 and E-35 are closed by it).
TODAY: Adding or removing an effect is not a recorded event (old item R223 TODAY: AR 1.8).
@@END
@@ITEM 261
TITLE: The recordings run together; the output comes first
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Record show with any of its boxes, record to clip and the low-resolution show recording can run at the same time; a second recording is never refused because one runs. If the computer cannot keep up, the output keeps its smoothness and a recording loses pictures. With @@ITEM D20 as it stands; the load is measured on his Mac before it is promised.
CHANGED: nothing: accepted as written (old assumption E-26 is closed by it). His box BF241 ("30 fps is the fastest or even 15 could be ok") touches the rate of the low-resolution show recording, not this rule.
TODAY: No measurement exists; each film reads the composition's picture back from the graphics card (old item D20 TODAY: AR H29, T2).
@@END
@@ITEM 262
TITLE: Size and pieces of the low-resolution show recording
STATUS: OPEN for its size (he asks in another box); the 10-minute pieces are accepted
HIS: box left empty = accepted as written; BF241 "Maybe something close to the output monitor resolution?"; BF241 "30 fps is the fastest or even 15 could be ok."
RULE: Waits on his answer: the low-resolution show recording is a quarter as wide and as high as the show (480 by 270 for a full HD show), which is read as close to the size at which the output monitor shows the picture inside the app; the other reading of his question, the size of the output screen itself, would make it a full-size film (assumption E3-1). Settled: it is written in 10-minute pieces, each a file that plays by itself and is written so that a crash costs only its last seconds. Its rate is at most 30 pictures a second and may be 15: the test after the build decides (assumption E3-2). Its codec is picked by the same test of codecs (topic H). It has no sound (item 231).
CHANGED: He left the box empty (a quarter; 10-minute pieces) and in the box under the answer on this recording suggests a size with "Maybe" and a question mark, and a rate: his newer words do not settle the size, so it is put to him once more (E3-1); the rate is new (the item named none; Harmony's answer said 30). "the output monitor" can name the monitor inside the app (his name for it) or a connected screen: both readings are in E3-1.
TODAY: Nothing of it exists (old item G4 TODAY). The size at which the output monitor is drawn inside the app: not checked.
@@END
## AMENDMENTS
@@AMEND 195 1
OLD: On which screen it runs: assumption E-2 (his answer names no letter, and letter A said the live window).
NEW: It runs in Studio: Record Over is a button there, the show recording plays in real time with its sound, and what is moved on the MIDI controller is recorded over it. It never runs in the live window or through the live layers.
HIS: item 229 accepted as written
WHY: The screen was left to an assumption; he accepted that Record Over is a button in Studio.
@@END
@@AMEND 195 2
OLD: Which controls can record over: assumption E-3.
NEW: Everything in the keyboard and MIDI mapping can record over, clip triggers too; what is moved with the mouse is not recorded over (page item 257). How a clip trigger joins the recorded ones: assumption E3-5.
HIS: item 257 accepted as written
WHY: Which controls record over was left to an assumption; he accepted the whole keyboard and MIDI mapping, without the mouse.
@@END
@@AMEND R184 1
OLD: That nothing plays a recording inside the live window, and that Record Over is a button of the review screen, is assumption E-2.
NEW: Nothing plays a show recording inside the live window, and Record Over is a button of Studio (page item 229).
HIS: item 229 accepted as written
WHY: Both halves hung on an assumption that he has now accepted.
@@END
@@AMEND R184 2
OLD: Render, making a film from a recording afterwards, is planned: "215 plan all of these" (L128).
NEW: Render, making a film from a show recording afterwards in Studio, is planned ("215 plan all of these", L128) and is the way to a film in full quality: "If we want a very hd recording, we can make an HD render from the Recording Review" (BF241). The low-resolution show recording is only for reference and is never that film. What Render makes is ruled with item 215 (f); whether the Video box of Record Show stays beside it: assumption E3-3.
HIS: BF241 "we can make an HD render from the Recording Review"
WHY: Render stood only as planned; his new words give it its job, the film in full quality, against the low-resolution show recording.
@@END
@@AMEND R184 3
OLD: APPEND
NEW: (g) Record Show starts and stops at the press, without waiting for the 1 or a bar line, and runs on through a tempo pause or stop (page item 259).
HIS: item 259 accepted as written
WHY: When Record Show starts and ends was held only by an assumption and stood in no rule.
@@END
@@AMEND R185 1
OLD: If the controller works in the review screen (assumption E-2): while a recording only plays
NEW: The controller works in Studio: while a recording only plays
HIS: item 229 accepted as written
WHY: The sentence was conditional on an assumption that he has now accepted.
@@END
@@AMEND R165 1
OLD: (and recording over it, if that runs here: assumption E-2)
NEW: (and recording over it, which runs here)
HIS: item 229 accepted as written
WHY: Conditional on an assumption that he has now accepted.
@@END
@@AMEND 186 1
OLD: If Record Over runs in the review screen (assumption E-2), the outputs show its picture as well.
NEW: Record Over runs in Studio, and the outputs show its picture as well.
HIS: item 229 accepted as written
WHY: Conditional on an assumption that he has now accepted.
@@END
@@AMEND R173 1
OLD: Into which show and onto which clip: assumption E-11; under its reading the action goes into the show the recording was made in, and if another show is open in the live window the app says which show to open before the action can be saved.
NEW: The action goes into the show the recording was made in, as that show is by then, onto the same clip wherever that clip is by then; if the clip is gone from that show, the app says so and nothing is saved (page item 230). If another show is open in the live window, the app says which show to open before the action can be saved.
HIS: item 230 accepted as written
WHY: Where a saved action lands was an assumption; he accepted it, and the case of a clip that is gone is now ruled.
@@END
@@AMEND R186 1
OLD: the main reading is a full picture exactly as on the output
NEW: it is a full picture exactly as on the output, global effects included, with no see-through parts ("This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like.", BF268)
HIS: BF268 "This is exactly the output recorded as one layer."
WHY: It stood as a main reading waiting for his answer; he has answered it in his own words.
@@END
@@AMEND R186 2
OLD: keeping them see-through is way b of page item 230, which asks him (assumption E-9)
NEW: keeping them see-through was put to him as way b of page item 256 and is not taken
HIS: BF268 "Whatever the output is displaying is what this should look like."
WHY: The see-through way was still open; his words take the full picture.
@@END
@@AMEND R186 3
OLD: as long as what was recorded: every recorded bar is kept and no out point is moved by itself (5 bars stay 5). He trims it later with its in and out points
NEW: whose file holds every recorded bar and whose out point the app sets at the end of the last full group of 4 bars, so that the clip loops only whole groups of 4 bars (5 recorded bars loop as 4); the rest stays in the file. He trims it, or brings the rest back in, later with its in and out points
HIS: BF254 "c"
WHY: The rule kept every bar in the loop and moved no out point; he chose the way in which the clip loops only whole groups of 4 bars.
@@END
@@AMEND R186 4
OLD: The other reading of "a even amount", that the recording runs on until a group of 4 bars is full, is way b of page item 260 (assumption E-6).
NEW: The recording never runs on until a group of 4 bars is full: it stops at the end of the bar (page item 232, way c).
HIS: BF254 "c"
WHY: Running on to a full group of 4 bars was still an open way; his letter c closes it.
@@END
@@AMEND R186 5
OLD: What the REC press does while the tempo is paused or stopped: assumption E-5 (page item 259).
NEW: A press on record to clip while the tempo is paused or stopped waits: the recording starts on the first 1 once the tempo runs (page item 258).
HIS: item 258 accepted as written
WHY: The press under a standing tempo was an assumption; he accepted that it waits.
@@END
@@AMEND R186 6
OLD: What the full composition includes: assumption E-8.
NEW: The full composition is whatever the output is displaying: all layers mixed together with everything Global does to the picture, the global effects included (BF268).
HIS: BF268 "Whatever the output is displaying is what this should look like."
WHY: What the full composition includes was an assumption; his words say it is exactly the output.
@@END
@@AMEND R223 1
OLD: "It" is read as an effect, and the rule is taken to hold for a clip or a layer too: assumption E-17. What is added during a recording: assumption E-35.
NEW: The rule holds for anything taken out while recording, an effect, a clip or a layer. Anything added while recording is recorded from the moment it is added: it is put into the recorded show file and its controls get tracks from then on, so that Studio shows the night as it was (page item 260).
HIS: item 260 accepted as written
WHY: Both halves were assumptions; he accepted one rule for what is added and what is taken out.
@@END
@@AMEND R183 1
OLD: A small film of the night, if he wants one (G4), is one more box: assumption E-21.
NEW: The low-resolution show recording (G4) is one more box, ticked in a new app and remembered like the others: it runs by default next to the show recording and has no sound, also when Audio is ticked (BF253).
HIS: BF253 "It defaults to running next to the show recording"
WHY: Whether he wants it, and with or without sound, was his choice to make; he made it.
@@END
@@AMEND G4 1
OLD: It is written in pieces of 10 or 20 minutes, each a small file
NEW: It is written in pieces of 10 minutes, each a small file
HIS: item 262 accepted as written
WHY: The piece length was open between 10 and 20 minutes; he left the 10-minute pieces as written.
@@END
@@AMEND G4 2
OLD: Whether he wants it, its size, its piece length, its sound (in it only when the Audio box is ticked, as for the full-size film, R183 e) and how it is switched on: assumption E-21 (his choice).
NEW: He wants it: it runs by default next to every show recording and can be switched off; it has no sound, whatever the Audio box says; it is there so that a mistake in the parameter recording can be seen (BF253), and it is just for reference: a film in full quality is made with Render in Studio (BF241). Its size waits on his answer (a quarter as wide and as high as the show, read as close to the output monitor's size: assumption E3-1). Its rate, 30 pictures a second at the most or 15 (assumption E3-2), and its codec are settled by the test after the build (BF241; the test of codecs is the topic on how a clip plays).
HIS: BF253 "It has no sound."; BF241 "This is just for reference."
WHY: All of it hung on one assumption put to him; he answered the default and the sound, gave its purpose, and asked back about the size.
@@END
@@AMEND D16 1
OLD: The small film of G4, if he wants it, is a film of the night to hold this picture against.
NEW: The low-resolution show recording (G4) is the film of the night that this picture is held against, so that a mistake in the parameter recording can be seen (BF253); how Studio shows the two together: assumption E-23.
HIS: BF253 "so that we can see if there is a mistake in the parameter recording"
WHY: The film stood as a maybe with a general purpose; his words make it wanted and name what it checks.
@@END
@@AMEND D22 1
OLD: and, if assumption E-9 holds, carries its see-through parts
NEW: and carries no see-through parts, being exactly the output's full picture (BF268)
HIS: BF268 "This is exactly the output recorded as one layer."
WHY: The file type was to carry see-through parts if he wanted them; he took the full picture.
@@END
@@AMEND D22 2
OLD: Which types exactly is decided after his codec question (L3) is answered, which another paper owns.
NEW: Which types exactly, for a film, for a recorded clip and for the low-resolution show recording, is decided by the test of codecs after the build: which codecs encode and decode well on his Mac (BF240, BF241). The topic on how a clip plays owns that test.
HIS: BF241 "we should test which codecs work best."; BF240 "can we test encoding and decoding of different codec"
WHY: The choice waited for the answer to his codec question; he now wants the codecs tested after the build, and that test decides.
@@END
## ASSUMPTIONS
@@ASSUME E3-1
ABOUT: 262, 231, G4, answer lowres
TEXT: I assume "the output monitor" is the one inside the app, so the low-resolution show recording stays a quarter as wide and as high as the show (480 by 270 for full HD), which is close to it.
WHY: He left 262 empty (a quarter) and asks in BF241 "Maybe something close to the output monitor resolution?"; the name fits the monitor in the app or a connected screen.
ALT: b) It is exactly as large as the output monitor is drawn in the app. c) You meant the projector's or screen's own size: then it is a full-size film, heavier, and no longer small.
IF-WRONG: REBUILD the size sets the load of every recorded show and what the film is good for.
ASK: YES his two boxes differ, his new words end in a question mark, and way c is another recording altogether.
@@END
@@ASSUME E3-2
ABOUT: 262, 231, 261, G4
TEXT: I assume the low-resolution show recording runs at 30 pictures a second, and at 15 only if the test on your Mac shows that 30 costs the output its smoothness.
WHY: BF241: "30 fps is the fastest or even 15 could be ok" gives the range and not which; nothing is measured.
ALT: b) Always 15 pictures a second: lighter, and enough for a reference.
IF-WRONG: SMALL one number, changed in a moment.
ASK: LINE his words give the range; the pick inside it follows the test.
@@END
@@ASSUME E-23
ABOUT: 231, G4, D16
TEXT: I assume Studio can show the low-resolution show recording beside its own picture, both at the same moment while you play or scrub, so that a mistake in the recorded moves is seen at once.
WHY: BF253 says what it is for ("so that we can see if there is a mistake in the parameter recording"), not how the two are compared.
ALT: b) The pieces are plain film files that you open in any player and compare by eye; Studio does not show them.
IF-WRONG: REBUILD a second picture in Studio that follows the playhead is a piece of work of its own; without it the check is done by hand.
ASK: YES his new words give the film its purpose, and no word says whether Studio shows it.
@@END
@@ASSUME E3-3
ABOUT: R183, R184, 215
TEXT: I assume the Video box of Record Show stays: a full-size film written during the show when you tick it. Render in Studio makes the same kind of film afterwards.
WHY: BF241: "If we want a very hd recording, we can make an HD render" names Render as the way to a full-quality film; the Video box does the same during the show.
ALT: b) The Video box goes: during a show only the low-resolution show recording is written, and a full-quality film is always made afterwards with Render.
IF-WRONG: SMALL one box more or less; way b spares building the full-size film during a show.
ASK: LINE he read the three boxes and left them; his new words may make one of them unneeded.
@@END
@@ASSUME E3-4
ABOUT: 231, R183
TEXT: I assume the low-resolution show recording is written with every record show for which its box is ticked, also when the Parameters box is not.
WHY: BF253 "next to the show recording"; its purpose is to check the parameter recording, yet his first words (BF161) also wanted it against a lost recording.
ALT: b) It is written only when the Parameters box is ticked.
IF-WRONG: SMALL a rare combination of boxes.
ASK: NO an edge of the boxes; neither way loses anything he ticked.
@@END
@@ASSUME E3-5
ABOUT: 257, 195
TEXT: I assume a clip you trigger during a Record Over is added to the clip triggers already recorded; the recorded ones still play.
WHY: Item 257 (accepted) says clip triggers record over too, not whether they join or replace the recorded ones.
ALT: b) From your first trigger on a layer, that layer's recorded triggers are dropped until you stop Record Over.
IF-WRONG: SMALL seen only in Studio, and the original recording is never changed.
ASK: NO it was in the text of the assumption behind item 257; a Record Over can be done again.
@@END
@@DROP ASSUME E-27
WHY: BF241 ("we should test which codecs work best") and BF240 (a test of encoding and decoding of different codecs after the build) settle how the file types are picked: by that test, which topic H owns; its way b, a codec of our own, was answered no on page 2, and BF268 removes the see-through choice it also waited for. Old item D22 is amended.
@@END
## QUESTIONS BACK
@@ANSWER lowres
HIS: BF241 "Maybe something close to the output monitor resolution?"
ANSWER: Yes, if you mean the output monitor inside the app: a film about that large is small enough, and I take it to be near the quarter size (480 by 270 for a full HD show). If you mean the projector's own size, it is no longer a small film; I ask you once which you mean. What will be built: it runs by default next to every show recording, without sound, in 10-minute pieces, just for reference. What is tested after the build, with the codec test you asked for: which codec, and whether 30 pictures a second stays light or 15 is needed. A very HD film is made afterwards with Render in Studio.
@@END
## NAMES
@@NAME Studio
MEANS: The screen where a show recording is watched, mended, recorded over and cut into actions.
SOURCE: his words (BF245: "Let's go with studio. That's perfect."; BF263: "b"); replaces the name Review in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md; the keyboard-and-menus topic owns the name block, this paper uses it
@@END
@@NAME Studio picture
MEANS: The area of Studio that shows the picture made fresh from the show recording; the outputs show it too.
SOURCE: Harmony's pick; replaces the name Review picture in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END
@@NAME low-resolution show recording
MEANS: The small film of the show, without sound, written in 10-minute pieces next to every show recording, to see whether there is a mistake in the parameter recording; just for reference.
SOURCE: his words (BF161 "a very low resolution show recording"; BF253; BF241); the row in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md says "pieces of 10 or 20 minutes, as a double check" and is to be brought up to date
@@END
@@NAME Render
MEANS: Making a film file in full quality afterwards from a show recording, in Studio; his "HD render".
SOURCE: his words (BF241: "we can make an HD render from the Recording Review"); the row in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md says "my pick until you name it": he has now used the word
@@END
## REACHES OTHER TOPICS
- K (old item 215 f, assumption K-2): BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" confirms Render as a function of Studio and gives it its job, the film in full quality. It is not a new function: 215 (f) already rules it (the whole recording, at the full size of the show, with its sound, not in real time). "very hd" is read as that full size and no larger. K's paper applies it to 215 (f); this paper amends only R184 and asks about the Video box (E3-3).
- H (the test of codecs after the build, BF240): BF241 "we should test which codecs work best" puts the codec of the low-resolution show recording, and with D22 the file types of a film and of a recorded clip, into that test. BF268 adds one fact for it: a recorded clip needs no see-through channel. BF241 also puts a second question to a test after the build, 30 or 15 pictures a second, which is a load test of this topic (E3-2) and can be run in the same session.
- H (item 238, BF258 "b"): a clip brought in in BPM mode is cut in to whole groups of 4 bars; BF254 "c" gives a recorded clip the same rule (loops whole groups of 4 bars). One rule in both papers.
- J (the name, item 245, BF245, BF263): every old RULE of spec-E says "the review screen" or "Review"; it reads Studio from now on. Not amended line by line here; one global replacement when the specs are merged. "Review picture" becomes Studio picture (Harmony's pick, above).
- A (BF271 "Stop removes all clips from all layers", box 273): with it a tempo stop leaves nothing to record for record to clip; old assumption E-7 (a tempo stop ends a clip recording and keeps it; a pause holds it) is not contradicted and stays as it is. If A's paper rules stop or pause otherwise, E-7 and item 259 need a second look.
- D (item 230): how "the same clip" is recognised after it moved, and where an action is stored, must be one rule in both papers (unchanged from the old spec).
- F (BF243, the snapshot as "a save of the show exactly where it is"): the recorded show file (old item R173, assumption E-34) is the same kind of save taken at the record show press; F's paper may want one mechanism for both. Nothing changes here.
- Applied here from another topic's box: BF245 / BF263 (Studio); BF240 (the test of codecs, into D22); BF258 (item 238 b, as agreement with 232 c).
## CONFLICTS
- The size of the low-resolution show recording. New, BF241: "Maybe something close to the output monitor resolution?" against item 262, whose box he left empty in the same sitting = accepted as written: "a quarter as wide and as high as the show (480 by 270 for full HD)", and against his earlier "Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check." (binding-decisions.md:1112, BF161). Not picked: assumption E3-1, asked.
- The sound of the low-resolution show recording. New, BF253: "It has no sound." against the page's own text for item 231 ("with sound") and the old ruling in G4 (sound when the Audio box is ticked). No earlier word of his said it has sound; his newest words win; no question.
- Where a recorded clip ends. New, BF254: "c" (loops only whole groups of 4 bars; the rest stays in the file) against the ruling over all topics of 2026-10-08 that read his "keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (binding-decisions.md:1170) to the letter (5 bars stay 5). His letter wins, and it agrees with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (binding-decisions.md:812). No question.
- See-through parts of a recorded clip. New, BF268: "This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like." against his earlier "yes" to see-through parts, given for a one-layer recording (binding-decisions.md:671 as cited by old item R186), and in line with "The recording should look exactly like the output." (binding-decisions.md:626). The newer words win; no question.
- The purpose of the low-resolution show recording. New, BF253: "so that we can see if there is a mistake in the parameter recording" beside his earlier "so that they are small and if something happens most of the recording is not lost" (binding-decisions.md:1112). Read as two purposes that both hold (the pieces stay); no question.
## NOT DONE / UNSURE
- What "the output monitor resolution" names (BF241): NAMES.md gives "output monitor" as the monitor inside the app (his words of 2026-10-05), and his R138 line uses "monitor" for a connected screen (binding-decisions.md:1161). Not settled by reading; E3-1 asks. The size at which the output monitor is drawn in the app was not read from the program text, so the @@ANSWER says "I take it to be near" and claims no number for it. Cheapest: his answer to E3-1; and one look at the monitor panel's size before the page is printed, so that the sentence can be firmer or cut.
- "the output" in BF268 is read as the picture (the output monitor), not one output screen with its own corrections; his earlier words carry that reading (binding-decisions.md:1170). Not asked.
- "the parameter recording" in BF253 is read as the recorded moves (the Parameters box). Not asked.
- Whether the test "light enough" still gates the default: he wrote "It defaults to running" without the page's condition. Kept as Harmony's safety from G4 (he is told before anything is decided if the test shows a stutter). If the page ruling wants it said to him, it is one line.
- Render's details (whole recording or the stretch between In and Out; its size) are K's assumption K-2; nothing new is assumed here beyond E3-3.
- R183 (d) "with no box ticked the button is greyed": whether the low-resolution box alone counts as a ticked box was not ruled (an edge; E3-4 covers its neighbour).
- Old assumptions tested and left untouched: E-4, E-7, E-12, E-13, E-15, E-16, E-18, E-19, E-24, E-28, E-30 (agrees with 232 c now), E-31, E-32, E-33, E-34, E-36, E-38. Changed: E-23. Dropped: E-27.
- TODAY lines are taken from the old spec's TODAY lines; nothing was re-derived from the program text and nothing was run.
- Written 2026-10-09 18:49:14 EDT.
