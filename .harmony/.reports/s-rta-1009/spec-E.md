# SPEC E -- Studio and the recordings (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-E.md, ruling rule-E.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
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
RULE: An action saved in Studio goes into the show the show recording was made in, as that show is by then, onto the same clip, wherever that clip sits by then (another cell, another deck). If that clip is no longer in that show, the app says so and nothing is saved. It never goes into whatever other show is open in the live window: if another show is open there, the app says which show to open before the action can be saved. An action that belongs to a layer or to Global goes to that layer or to Global of the same show; if that layer is no longer in that show, the app says so and nothing is saved. The action is written to the show file at the next Save, like any other change: with @@ITEM R173 as it stands (as amended). How "the same clip" is recognised after it moved is one rule with the actions topic.
CHANGED: nothing: accepted as written (old assumption E-11 is closed by it). INFERRED, not in the item he read: the extension to a layer's and Global's actions and the case of a layer that is gone (the item names only a clip); "the app says which show to open before the action can be saved", carried from old item R173, where it stood under old assumption E-11 (the item's way c, whatever show is open, was not taken).
TODAY: A recording names no show and carries none; a replay is matched against whatever show is open (old item R173 TODAY: FT section 1, S6; AR 1.5, H24).
@@END

@@ITEM 231
TITLE: The low-resolution show recording runs by default, without sound
STATUS: ANSWERED in his words
HIS: BF253 "It defaults to running next to the show recording so that we can see if there is a mistake in the parameter recording. It has no sound."; BF241 "This is just for reference."
RULE: The low-resolution show recording runs by default next to the show recording: whenever a record show runs it is written too, unless it has been switched off. It has no sound, whatever the Audio box says. It is there so that a mistake in the parameter recording can be seen: it is a film of the picture as it really was during the show, to hold against the picture that Studio makes fresh from the recorded moves (with @@ITEM D16 as amended). It is just for reference and is never the film to keep or share: a film in full quality is made afterwards with Render in Studio (BF241). It shows the full picture, never one output screen's corrected picture, and is written in pieces so that a crash costs only the last seconds: with @@ITEM G4 as it stands (as amended). It keeps running through a tempo pause and a tempo stop, as the show recording does (item 259). Harmony's own, not his words: (1) it is switched off and on with one more box of Record Show, ticked in a new app, and the app remembers the tick like the other boxes (how it is switched is layout, laid out where it fits); (2) every piece is kept with its show recording and carries the time of the show recording at which it begins, which Studio needs if it is to play the film in step with its own picture (assumption E-23); (3) a mend and a Record Over write no film of their own and share the film of their original, as they share its sound; (4) whether it is written when the Parameters box is not ticked: assumption E3-4; (5) the test on his Mac with a heavy show comes before it is left switched on by default: if it costs the output its smoothness he is told before anything is decided, and the output comes first (item 261). Its size, its rate and its codec: item 262.
CHANGED: The page's "with sound" is replaced by his "It has no sound." (the page's way c); "unless you untick it" stays as his "It defaults to running". New in his words: what it is for ("so that we can see if there is a mistake in the parameter recording"). INFERRED: "the parameter recording" is read as the recorded moves, the Parameters box of Record Show. The page's condition "once a test shows it is light" is not repeated by him: it is kept as Harmony's safety from old item G4, which he read, and listed with the other four points that are Harmony's own and not his words (the box and its remembered tick; the pieces that carry their time; the film that a mend and a Record Over share; the Parameters box).
TODAY: Nothing of it exists. The only film recorder writes one full-size file without sound and without pieces, and its index at the end, so a crash leaves it unplayable (old item G4 TODAY: VideoRecorder.cpp, check-lowres facts 6-8; read, not run).
@@END

@@ITEM 232
TITLE: Record to clip ends at the bar; loops groups of 4
STATUS: ANSWERED c
HIS: BF254 "c"
RULE: Record to clip stops recording at the end of the bar in which the press that stops the recording falls; it does not run on until a group of 4 bars is full. The file holds every bar that was recorded. The clip that lands loops only whole groups of 4 bars: the app sets its clip out point at the end of the last full group of 4 bars (5 recorded bars loop as 4; 9 as 8; 12 stay 12), and the rest stays in the file behind the clip out point; nothing is taken out of the file. It is one kind of cut with the cut of a clip that is brought in (page item 238, way b, topic H); how far the clip out point can be dragged out again is the rule for every clip's out point (topic H). The clip is a BPM-mode clip, and its number of beats is that of the looped part. Its bars are the bars the beat clock counted while it was recorded, at whatever tempo the clock had, and also when the 1 was moved meanwhile (a Resync, or the app placing the 1): they are never worked out afterwards from the file's length at some other tempo. A picture that was lost because the computer could not keep up (item 261) is filled with the picture before it, so the file keeps its length and its bars stay in time. A recording is never shorter than 4 bars (a stopping press before the end of bar 4 runs on to the bar line that closes bar 4), starts on the 1, and lands waiting in its cell: with @@ITEM R186 as it stands (as amended). The tempo bar's stop is not this press: a tempo stop, a quit, a full disk, or the stopping press while the tempo is paused end the recording at once and keep it (old assumptions E-7 and E-30).
CHANGED: The item's main text ("keeps every bar it recorded (5 bars stay 5); you trim it yourself") is replaced by its way c. Old item R186 (c) said that no out point is moved by itself: amended. It agrees with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (binding-decisions.md:812) and with his answer b to item 238 (BF258) for a clip brought in. Kept from the old rule he read, since way c does not speak of it: never shorter than 4 bars (R186 g). INFERRED, internal: that "loops only whole groups of 4 bars" is built as the clip out point; that the rest can be reached again by dragging the clip out point (topic H rules how far); how the bars are counted, also under a moved 1 (BF246 "app should always try to find the 1 and I will correct if necessary"); that a lost picture is filled; that "stop" in the item is the press that stops the recording and not the tempo bar's stop.
TODAY: Record to clip does not exist in the app (old item R186 TODAY: AR 1.18).
@@END

@@ITEM 256
TITLE: A recorded clip is exactly the output, as one layer
STATUS: ANSWERED in his words
HIS: BF268 "This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like."
RULE: A clip made with record to clip is exactly the output recorded as one layer: whatever the output is displaying is what the clip looks like. It is the full picture of all layers mixed together with everything Global does to it, the global effects included; where no layer shows it is black, as on the output. It has no see-through parts. His "as one layer" means that the whole output lands as one clip that plays on one layer; it is not the recording of one chosen layer, which stays removed (with @@ITEM R186 (e) and @@ITEM 216 as they stand). Played afterwards it is a clip like any other ("The recording will always play like a regular clip once it is recorded.", binding-decisions.md:627): it covers the layers under it except through its layer's own blend mode and transparency slider, and the effects that are on when it plays, its layer's and the global ones, act on it as on any clip, also where the same global effects are already in its picture. "The output" is the picture as the output monitor shows it, never one output screen's own Delay, Brightness, Contrast or colour: with @@ITEM R187 (d) as it stands. Its file type needs no see-through channel and is picked by the test of codecs after the build (topic H).
CHANGED: The item's main text is confirmed in his own words; way b (see-through parts) is not taken. Old assumptions E-9 and E-8 are closed by it; old items R186 (a), (e) and D22 are amended. INFERRED: "as one layer" is read as the whole output landing as one clip on one layer, not as the recording of one chosen layer (his "I don't think there's any reason for a one layer recording.", BF211, binding-decisions.md:1170, stands); "the output" is read as the picture and not as one output screen, from his earlier "not the screen output as screens can be modified" (binding-decisions.md:1170). That the effects which are on when it plays act on it again is his "always play like a regular clip" (binding-decisions.md:627), not a new rule and not a question.
TODAY: Record to clip does not exist in the app; the film recorder reads the composition's picture back from the graphics card (old item D20 TODAY: AR H29).
@@END

@@ITEM 257
TITLE: What Record Over takes
STATUS: ACCEPTED
HIS: box left empty = accepted as written; from other boxes: BF270 "We need to work with both and know how they both work so they both work smoothly."; BF262 "b"
RULE: Record Over takes everything in the keyboard and MIDI mapping that moves a control of the show: knobs, faders, pads and keys, clip triggers too. What is moved with the mouse on screen is not recorded over. What is recorded is what the control does (its values, its presses), never the knob's own messages: so a knob that sends its position and an endless knob that sends steps record over alike (BF270). A knob or fader supersedes its recorded track while it is held and gives it back when it is let go (with @@ITEM 195 as it stands; when a knob counts as let go: assumption E-32). A clip triggered during a Record Over is added to the triggers already recorded (assumption E3-5). The mapping that works in Studio is the one in the show recording's own show file; an entry that belongs to a cell (topic J's rule for a knob on a clip's slider, BF262) finds its cell in the deck that the show recording has on screen at that moment. Studio's own keys (the spacebar, Cmd+Z: @@ITEM R185 (a) as it stands) keep their Studio meaning during a Record Over. Entries of the mapping that move no control of the show (the tempo bar's, the recording buttons) do nothing while Studio is open (assumption E3-7).
CHANGED: nothing in what he accepted (old assumption E-3 is closed by it). Added from his words in other boxes: both kinds of knob (BF270); the knob that belongs to a cell (BF262, way b of item 244, topic J's rule). INFERRED, internal, and carried by assumptions: that a trigger is added to the recorded ones (E3-5; it was in the old assumption's text and not in the item he read); whose mapping works in Studio, and that the tempo bar's entries and the recording buttons do nothing there (E3-7: "everything" in the item is read as everything that moves a control of the show, with two rules he read, Studio's own keys in old item R185 a and "The tempo row's buttons are not on this screen" in old item R138); when a knob counts as let go (E-32).
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
HIS: box left empty = accepted as written; from another box: BF271 "Stop removes all clips from all layers so it would stop."
RULE: Record show starts at the press and stops at the press; it does not wait for the 1 and does not end on a bar line. It runs on through a tempo pause and a tempo stop: the moves, the sound and the low-resolution show recording keep being recorded, and the presses on the tempo bar are recorded with them (with @@ITEM R185 (b) as it stands). Through a stop it records what the stop leaves: the layers empty, the actions stopped and, where the sound was an audio file that the stop stopped (topic K's rule from BF271), silence until the file is started again; the recording's sound is always what the app hears (with @@ITEM D19 as it stands). Only record to clip waits for the 1 and ends on a bar line.
CHANGED: nothing: accepted as written (old assumption E-10 is closed by it). Added as a consequence of his words in another box (BF271, and BF250 for the actions, as topics A, D and K apply them): what a show recording holds from a tempo stop on.
TODAY: Tempo commands are recorded as events; a tempo-row stop does not exist yet (old item R185 TODAY: AR 1.8, H26).
@@END

@@ITEM 260
TITLE: What is added or taken out while recording
STATUS: ACCEPTED
HIS: box left empty = accepted as written; from another box: BF256 "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play."
RULE: Anything added while a record show runs (an effect, a clip, a layer) is recorded from the moment it is added: the adding is recorded, the thing is put into the recorded show file, and Studio shows it from that moment. Its controls are treated like every other control: one that is changed during the recording gets a track, flat at its default before the adding; one that never changes gets none (with @@ITEM R137 as it stands). Anything taken out while recording (an effect, a clip, a layer) is recorded at that moment: the removal is recorded, the thing is kept in the recorded show file, and from the removal on all its settings are at their defaults and its tracks are flat at default; Studio shows it working up to the removal and gone after it. A clip that is deleted or pasted over while it plays leaves its layer at that moment (BF256), and Studio shows the layer as the output had it; a paste over a clip is a taking out and an adding at one moment. With @@ITEM R223 (d) as it stands (as amended). An action cut from a clip that was added during the recording is saved by the rule of item 230: it is saved if that clip is in the show by then.
CHANGED: nothing: accepted as written (old assumptions E-17 and E-35 are closed by it). Corrected against the paper: only the controls of an added thing that change get tracks (his "we are not recording anything that does not move.", BF204, binding-decisions.md:1163). Added from his words in another box (BF256): what a delete or a paste over a playing clip is in a recording. INFERRED, internal: a track of an added thing is flat at its default before the adding (the mirror of his rule for a removal).
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
STATUS: OPEN for its size (his question in another box, BF241); the 10-minute pieces are accepted
HIS: box left empty = accepted as written; BF241 "Maybe something close to the output monitor resolution? 30 fps is the fastest or even 15 could be ok. This is just for reference."
RULE: Waits on his answer for the size: the low-resolution show recording is a quarter as wide and as high as the show (480 by 270 for a full HD show), as the item he left says; his "Maybe something close to the output monitor resolution?" is read as the output monitor inside the app, so that the film stays small, and not as an output screen's own size, which would make it a full-size film (assumption E3-1). Settled: it is written in 10-minute pieces, each a file that plays by itself and is written so that a crash costs only its last seconds (with @@ITEM G4 as it stands, as amended). Settled by his words: it runs at 30 pictures a second at the most, and 15 is good enough for him (BF241); it is built for 30, and the test on his Mac after the build decides whether it stays at 30 or goes down to 15; no rate above 30 and none below 15 without his word. Its codec is picked by the test of codecs after the build (topic H owns that test). It has no sound (item 231).
CHANGED: He left the box empty (a quarter; 10-minute pieces) and, in the box under the answer on this recording, suggests a size with "Maybe" and a question mark and gives a rate. The size is put to him once more as one line (E3-1): "the output monitor" can name the monitor inside the app (his name for it) or a connected screen, and his next sentences ("This is just for reference. If we want a very hd recording, we can make an HD render from the Recording Review") speak for a small film. The rate is new and is his (the item named none; Harmony's answer said 30): no question is left on it, the pick inside his range follows the test.
TODAY: Nothing of it exists (old item G4 TODAY). The output monitor inside the app has no fixed size: its panel letter-boxes the canvas and follows the window's layout (CLAUDE.md, UI Patterns, "Preview/Output panel never reshapes the picture"; read, not run), so no number is taken from it.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 186
TITLE: The outputs while you review
STATUS: ANSWERED b -- AMENDED after page 2 (186 1 by E)
HIS: L91, L79
RULE: While the review screen is open, every connected output shows the review picture too: what plays, and what a click or a scrub lands on (his "186 b", the letter without a switch). When the review screen is closed the outputs show the live show again, with nothing playing (R165). There is no extra switch for it: an output is switched off the way any output is, and each output's own screen settings apply to the review picture as to any picture it shows (assumption E-1). The recording's sound is heard while the recording plays in real time (R138, L86); through which sound output: assumption E-33. Record Over runs in Studio, and the outputs show its picture as well. [page 2, E: item 229 accepted as written] His L79 follows from it: with the picture on a monitor, the tracks can have the room on the main screen (R138).
CHANGED: The default A (outputs black) is replaced by B, his "186 b" (L91). The page's note under the question that "the recording's own picture and sound never reach the outputs" falls away with it. Ruling: three things the paper's RULE gave as settled (each output's own settings, the sound, what a Record Over shows) are no words of his; they now stand as assumptions E-1, E-33 and E-2.
TODAY: A replay drives the live layers and the live outputs; there is no separate review picture (FT section 1, S5; AR 1.10). A review renderer and its route to the outputs do not exist; both are to be built. Through which device a recording's sound is played now: not checked.
@@END

@@ITEM 187
TITLE: Scrub or draw inside a track
STATUS: DEFAULT A
HIS: L1, L92
RULE: Two tools, picked by two buttons or by holding a key. The pointer: a click in a track jumps the picture and the timeline to that moment, a drag scrubs both. The pencil: a drag draws a new shape over a track's curve; a piece can be selected and moved left or right, snapping to the grid. The pointer is on when the screen opens. Besides these two mouse tools there is Record Over, which changes tracks in real time from the MIDI controller (item 195); his L92 keeps the three apart: "This record over mode is different than the scrubbing and drawing with mouse as this plays in real time". Whatever the pencil changes is a mend and goes into the parallel recording (R222).
CHANGED: nothing in the default; L92 confirms that both scrubbing and drawing with the mouse exist.
TODAY: No track, timeline, scrub or curve editing exists anywhere in the app (FT section 1, the Record tab line and the line after it; AR 1.3).
@@END

@@ITEM 188
TITLE: Which rows the review screen shows
STATUS: REPLACED by L81
HIS: L81
RULE: The review screen has a track only for a button or slider that changed during the recording. Such a track runs from the beginning of the recording to its end and is a flat line at the value the control had until the moment it was first used. A button or slider that was not changed during the show and was at its default the entire time gets no track. Clips, layers and global that had no changed control have no tracks at all. A control that never moved but stood away from its default: assumption E-13. The tracks are folded under the names of their clip, layer and effect, or under global.
CHANGED: The default A ("Every control of those has a row, moved or not") and B (every control of the whole show) are both replaced by his L81: "On any button or slider that was not changed during the show and was at default the entire time, does not get a row in the recording."
TODAY: A recording keeps ten kinds of event and the values that arrive through keys, pads, MIDI, OSC and REST; a slider moved with the mouse, the layer strip's B and S, and a preset load are not kept (AR 1.8; FT section 2). No rows exist. The recorder has to reach every control (AR T4).
@@END

@@ITEM 195
TITLE: Record Over with the MIDI controller, kept separately
STATUS: ANSWERED in his words (Record Over stays; no letter) -- AMENDED after page 2 (195 1 by E, 195 2 by E)
HIS: L92, L84
RULE: Record Over stays. It is done with the MIDI controller and it plays in real time: the recording plays with its sound and its recorded moves, the picture shows what the moves do, and what he moves on the controller while it plays is recorded over the moves of those same controls. A control he moves supersedes its recorded track while he holds it and goes back to the recorded track as soon as it is let go (his earlier words, BD:340-341); what counts as "let go" for a knob on a controller: assumption E-32. It is a third way of changing a recording, apart from scrubbing and from drawing with the mouse (L92). A Record Over never changes the original: it is recorded separately from the original and kept beside it, with the same sound (L92; how many copies: assumption E-4). It runs in Studio: Record Over is a button there, the show recording plays in real time with its sound, and what is moved on the MIDI controller is recorded over it. It never runs in the live window or through the live layers. [page 2, E: item 229 accepted as written] Everything in the keyboard and MIDI mapping can record over, clip triggers too; what is moved with the mouse is not recorded over (page item 257). How a clip trigger joins the recorded ones: assumption E3-5. [page 2, E: item 257 accepted as written] While a Record Show or a clip recording runs, Record Over is not available (R223 c).
CHANGED: No letter was given. That Record Over stays and that the old moves play under it are his: "yes we can record over using the midi controller" (L92) and "The knob will supersede whatever is happening, And will snap back to the recorded track as soon as it is let go." (BD:340-341). A's "kept as a new recording beside the first, with the same sound" is confirmed by "Record overs are recorded separately from the original" (L92). A's "reached from the list of recordings ... play in the live window" is neither confirmed nor replaced by his words: it is the ALT of assumption E-2. B (only the sound plays) and C (Record Over goes) are not taken.
TODAY: "Record Over" shows only while a recording replays with its sound, records a new take onto the same sound from the current position, and runs through the live layers (AR 1.1, H8, O2). It takes keys, pads, MIDI, OSC and REST values, not the mouse (AR 1.8). Its only door, "Play Take", goes.
@@END

@@ITEM 216
TITLE: The fader in a one-layer recording
STATUS: REPLACED by L88 (the question falls away)
HIS: L88
RULE: There is no one-layer recording, so the question does not arise. A clip recording holds the full composition, and every layer is in it at the strength its fader has at each moment: a fade performed while recording is in the clip.
CHANGED: Default A and option B both fall away: "I don't think there's any reason for a one layer recording." (L88).
TODAY: Record to clip does not exist in the app (AR 1.18). The old plan had a Whole / Layer switch (AR O13, H18): the Layer half is not to be built.
@@END

@@ITEM R138
TITLE: The review screen: stop, jump, scrub, and where its picture goes
STATUS: CORRECTED (added to; the sound sentence replaced)
HIS: L79, L86, L91
RULE: The review screen is a screen of its own, not the live window: the picture at the top, one track per changed slider or button below it (L81). While it plays, the stop button, the spacebar, or a mouse press inside a track stops it and changes nothing else. A click in a track moves the picture and the timeline to that moment; a drag scrubs both together (with the pointer tool, item 187). It does not start playing again by itself: the spacebar or play does that, from where the playhead stands. The tempo row's buttons are not on this screen. From L79: the line between the tracks and the picture can be dragged, to make the picture as small or as large as the screen allows; and because the connected outputs show the review picture (186 b), he can watch it on a monitor and give the main screen to the tracks. Whether the picture on the main screen can be closed altogether is layout (L8). Sound: it plays while the recording plays in real time; whether it is also heard during a scrub is a setting (R183, L86).
CHANGED: Added: the draggable line between tracks and picture, and the picture watched on a monitor (L79). "monitor" is read as a connected output, which shows the review picture by his 186 b: INFERRED (assumption E-1). Replaced: "the recording's sound plays while it plays and is silent while you scrub" becomes a setting (L86). "one track per slider or button" now means per changed control (L81). The rest stands.
TODAY: None of it exists (AR 1.3, 1.10; FT section 1). No space-key handler exists in the app (AR correction 5).
@@END

@@ITEM R165
TITLE: Opening the review screen stops the live show
STATUS: STANDS -- AMENDED after page 2 (R165 1 by E, R165 2 by E)
HIS: L72, L35, L91, L80
RULE: The review screen is for after the show. Opening it stops everything that plays in the live show, as a press on the tempo stop does: the clips leave every layer and the beat stops (BF271), the actions stop (BF250), and an audio file that the app plays stops (BF271); the app asks first if clips are playing. Unlike the tempo stop it changes no button of the show: every action button stays as he set it, whatever a tempo stop does to the action buttons (page item 226, topic D) (assumption E3-6). Closing it brings back the live window with the show as it was and nothing playing; nothing starts by itself. So the only sound the app plays while Studio is open is the show recording's. [page 2, E: BF271 "Stop removes all clips from all layers so it would stop."; BF250 "tempo stop stops all actions, not just global"] Reviewing or mending a recording (and recording over it, which runs here) [page 2, E: item 229 accepted as written] never changes a slider, a button or a clip of the show in the live window: the review works on the recording's own copy of the show (R173, L80), whichever show is open in the live window. The only thing that goes from the review screen into a show is an action he saves there (assumption E-11). While it is open the outputs show the review picture (186 b).
CHANGED: nothing; its pointer to question 186 is now answered b. His words fit it: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72) and "live and recording review mode" (L35). Ruling: said outright that the review uses the recording's own copy beside the live show (this reading, which he left as it was, with L80).
TODAY: A replay runs through the live layers, so review and live cannot run side by side (FT section 1, S5). Showing a recorded moment without touching the live show is not built and its cost is unmeasured (AR T3).
@@END

@@ITEM R173
TITLE: A show recording saves its own show file
STATUS: CORRECTED -- AMENDED after page 2 (R173 1 by E)
HIS: L80, L89
RULE: Every show recording saves with it a show file that holds the clips exactly as they are for that show: the recorded show file (L80, L89). The review screen opens a recording from its recorded show file, so the recording opens correctly whatever has been done to the show since, also when the show of that name has been changed. It opens that copy for the review only: the show in the live window is not replaced, and comes back as it was when the review screen is closed (R165). When the show of that name has changed since, the app tells the user which clips have been moved or are missing; read as: a clip that now sits in another cell of that show, a clip that is no longer in it, and a clip whose file is not found on the disk (INFERRED from L80). When the recorded show file is taken and what the review takes from it: assumption E-34; whether it holds the media: assumption E-12. What is removed or added during a recording is kept in it (R223 d). An action saved in the review screen goes into the show (his One Save: it is written to the show file at the next Save, and the quit window asks about it like any other change). The action goes into the show the recording was made in, as that show is by then, onto the same clip wherever that clip is by then; if the clip is gone from that show, the app says so and nothing is saved (page item 230). If another show is open in the live window, the app says which show to open before the action can be saved. [page 2, E: item 230 accepted as written]
CHANGED: Replaced: "it is reviewed with that show open (if another show is open, the app says which one to open)" and "The review uses the show as it is now" -- by L80: "the recording also saves a show file with the clips exactly as they are for the show" and "the recording still opens it up correctly and indicates this to the user that these clips have been moved or missing". Replaced: "a row of a clip or an effect that you have since removed is marked missing and cannot be ticked" -- its track is there and plays in the review; what saving does for it is E-11. The last sentence (the action goes into that show, One Save) stands. Ruling: added that the copy is opened beside the live show (R165 with L80), what "moved or missing" is compared with (INFERRED), the case of another show being open (part of E-11) and the pointer to E-34.
TODAY: A recording names no show and carries none; a replay is matched against whatever show is open (FT section 1, S6; AR 1.5, H24). The recording's file needs a new shape (AR T6) and the verified writer (AR 1.22).
@@END

@@ITEM R137
TITLE: Only what changed gets a track
STATUS: CORRECTED (a and c replaced)
HIS: L81
RULE: (a) Nothing that does not move is recorded as a track. A button or slider that was changed at any moment of the recording gets a track from the beginning of the recording to its end; before its first use the track is a flat line at the value it had. A button or slider that was not changed during the show and was at its default the entire time gets no track. (b) A preset loaded during the recording shows as a jump at one moment on the track of each slider it changed; before the jump each track shows the value it had. (c) falls away: a control that never moved has no track (the case of one standing away from its default: assumption E-13). (d) An action made from there stores the numbers and never the preset's name: changing the preset later does not change the action. (e) Selecting the name of a clip, a layer or an effect selects all its tracks.
CHANGED: (a) "a row for every slider and every button that can be adjusted, moved or not" and (c) "the row of a control that never moved is a flat line at its value" are replaced by L81: "we are not recording anything that does not move." (b), (d), (e) stand. His own earlier sentence, "there is a row in the recording for every parameter and button that can be adjusted" (BD:1089), is replaced by his newer one.
TODAY: See item 188: no rows; the recorder reaches only part of the controls (AR 1.8, T4; FT section 2).
@@END

@@ITEM R135
TITLE: In and Out sit on grid lines; 1 beat is the smallest
STATUS: CORRECTED (the list shortened)
HIS: L82, L83
RULE: In and Out can sit only on grid lines. The grid's size can be changed; the snap cannot be switched off. The grid sizes for In and Out are 4 bars, 2 bars, 1 bar, 2 beats and 1 beat; 1 beat is the smallest. Quantize has its own spacing list (R166), which goes finer: assumption E-14.
CHANGED: "half a beat and a quarter beat" are removed from the list: "1beat is the smallest" (L82). The rest of the list stands as shown ("half a bar" is written here as 2 beats, his word in L83).
TODAY: No grid exists; an action's forerunner is cut by typed bar numbers (AR 1.2).
@@END

@@ITEM R166
TITLE: Quantize asks for its grid spacing
STATUS: CORRECTED
HIS: L83, L89, L84
RULE: Quantize is a control of the review screen. When a Quantize is done the user selects its grid spacing: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat or 1/4 beat. It moves button presses and clip fires onto the nearest line of that spacing; it does not reshape a slider's curve. What it works on: assumption E-15. A Quantize is a mend: it goes into the parallel recording and never into the original (R222). A Quantize is an Undo step.
CHANGED: "In and Out use the same grid" is replaced: Quantize has a spacing setting of its own with his list (L83). The rest stands.
TODAY: Quantize is a live setting in the top bar (Off / Next Beat / Next Downbeat; FT SURPRISES S2); its removal there is planned and not built (AR part 2, R-8). No review Quantize exists.
@@END

@@ITEM R222
TITLE: A mend is another recording, parallel to the original
STATUS: CORRECTED
HIS: L84, L92
RULE: A mend (a shape drawn over a curve, a piece moved, a Quantize) is never written into the original recording. The mend is another recording that is saved, parallel to the original (L84): it stands beside the original in the list of recordings and shares its sound and its recorded show file. The original stays exactly as it was recorded and can always be opened again. The mended recording is saved by the app as he works, with no Save press, and is there when the review screen is closed and opened again. When the mended copy is made, how many there are and what a mend of a mended recording does: assumption E-4. An action saved earlier from a recording does not change when that recording is mended afterwards.
CHANGED: Replaced: "a mend ... changes the recording", "The night as it was recorded is always kept underneath" and the button "Back to as recorded" -- by L84: "the mend is another recording that is saved, parallel to the original." (he took the page's own closing offer). The page's "it is kept when you close the review screen" and the sentence about earlier saved actions stand. Ruling: that it is saved without a Save press is the page's "kept when you close", which he did not change; his own word is "saved" (L84), so the when is carried by E-4.
TODAY: No mending exists. Several recordings can already share one stored sound (AR 1.6; BD:452-458).
@@END

@@ITEM R167
TITLE: An action never records a deck switch or the tempo
STATUS: STANDS
HIS: L85, L89
RULE: An action never switches decks and never touches the tempo: the night's tempo changes and deck switches cannot be selected into an action. The night's tempo is recorded and shown in the review screen, and its bars are the grid. Whether deck switches are shown at all: assumption E-16.
CHANGED: nothing; his "R167 action should not record switching decks" (L85) says the same for decks, and "Actions will not be made with a tempo change." (L89) for the tempo.
TODAY: A recording keeps deck switches and tempo commands as events (AR 1.8; FT section 1).
@@END

@@ITEM R183
TITLE: The three boxes; how a track looks; sound while scrubbing
STATUS: CORRECTED (added to; g replaced; a to f stand) -- AMENDED after page 2 (R183 1 by E, R183 2 by E)
HIS: L86, L81, L114, L71
RULE: The three boxes of Record Show stand as shown: (a) Parameters: the moves of the controls; it gives the review screen its tracks and its picture, which is made fresh from the moves and the recorded show file. (b) Audio: the night's sound; it gives the review screen its sound and its audio track. (c) Video: a film file of the composition to keep or share; the review screen does not use it. (d) The boxes sit beside the Record Show button; in a new app Parameters, Audio and the low-resolution show recording are ticked and Video is not; the app remembers the ticks; with none of Parameters, Audio and Video ticked the button is greyed, because the low-resolution show recording runs only next to a show recording (BF253) [page 2, E: BF253 "It defaults to running next to the show recording"]. (e) With Audio and Video both ticked the film has the night's sound; a Video-only recording is a film file and is not in the list of recordings. (f) The Output menu's own film recording goes: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone." (L114). New from L86: each recorded track, button or slider, is shown in a small row that can be dragged taller, with its keyframes and slider positions. A button's track is a horizontal line that can only be in two positions, 0 and 100, unless the control has more than two positions; then its line has one level for each position. A setting decides whether the sound is heard while scrubbing; with it off the sound plays only when the recording plays in real time. How the setting starts: assumption E-19. (g) A slider that a signal was moving has a track that shows what it did that night, in the colour of signal moves (L71, D18); that its moves are stored as values, so that this holds also when Audio was not ticked, is assumption E-24. The low-resolution show recording (G4) is one more box, ticked in a new app and remembered like the others: it runs by default next to the show recording and has no sound, also when Audio is ticked (BF253). [page 2, E: BF253 "It defaults to running next to the show recording"]
CHANGED: Added: the track's look and the draggable row height, the two-position button line, the scrub sound setting (L86); "one level for each position" is INFERRED from "unless this is something with more than one position". (a) "its rows" now means tracks of changed controls only (L81). (g) "Until it is measured I cannot promise what a slider that the music was moving shows when Audio is not ticked" is replaced: that the track is there is his (L71: "recorded off of signals"); how the moves are kept is Harmony's (E-24). (a) to (f) stand.
TODAY: One tick "Record audio", on at every launch and not remembered; Parameters always on; the film is a separate Output menu command with no sound (AR 1.2, 1.13, H9, H10). The film may play at the wrong speed (AR 1.14, T2: inferred, not run).
@@END

@@ITEM R184
TITLE: The list of recordings and the Record tab
STATUS: CORRECTED (one sentence of d withdrawn; names; Render planned) -- AMENDED after page 2 (R184 1 by E, R184 2 by E, R184 3 by E)
HIS: L87, L91, L114, L128, L92
RULE: (a) The place where a recording is watched, mended and cut into actions is one screen, the review screen. (b) Recordings are found in a list: name, date, length, the show each was made in. A double-click opens one in the review screen; a recording can be renamed and shown in Finder. Mended and recorded-over recordings stand in the list beside their original (R222, 195). (c) The app never deletes a recording; it is removed in Finder. (d) The page's sentence "To play a whole recording as a performance you make one action from its start to its end." is withdrawn (his L87; the answer R184 explains it). A recording is played in the review screen, and the outputs show it (186 b). Nothing plays a show recording inside the live window, and Record Over is a button of Studio (page item 229). [page 2, E: item 229 accepted as written] Render, making a film from a show recording afterwards in Studio, is planned ("215 plan all of these", L128) and is the way to a film in full quality: "If we want a very hd recording, we can make an HD render from the Recording Review" (BF241). The low-resolution show recording is only for reference and is never that film. What Render makes is ruled with item 215 (f); whether the Video box of Record Show stays beside it: assumption E3-3. [page 2, E: BF241 "we can make an HD render from the Recording Review"] (e) A recording's sound that was damaged by a crash is repaired by itself when the list opens. (f) Beside the list there are: the Record Show button, which reads "Stop Recording" while a recording runs; the box for the recording's name; the boxes of R183. The word "take" is not on screen (Harmony's reading of "All else is gone", L114; the list of names owns the wording). (g) Record Show starts and stops at the press, without waiting for the 1 or a bar line, and runs on through a tempo pause or stop (page item 259). [page 2, E: item 259 accepted as written]
CHANGED: (d) the sentence he asks about in L87 is withdrawn. Whether a recording can still be played inside the live window is not settled by his words and is asked with the place of Record Over (E-2). (d) "Whether you still want Render: question 215 (f)" is answered: planned (L128). (f) the button's name follows L114 ("record show"). (d) "Record Over: question 195" is answered by L92 as far as it stays; its place is E-2. The rest stands.
TODAY: Record tab: "Record Take" / "Stop Recording" / "Record Over", "Play Take" / "Stop Playback", "Load Take...", "Show in Finder", "Repair Audio", "Record audio", "Play with audio", the name box, "Take (timelines and audio)", "Render... (coming)", "Takes: <folder>", "Save Routine" (AR 1.1, 1.2). No list of recordings (AR 1.3, H5). No key or pad starts a recording (AR 1.4).
@@END

@@ITEM R185
TITLE: The review screen: keys, Undo and the tempo row
STATUS: REPLACED in part (a: the controller in the review; c is marked as inferred) -- AMENDED after page 2 (R185 1 by E)
HIS: L92, L89, L72
RULE: (a) While the review screen is open the keyboard is its own: the spacebar stops and plays (his words for the stop, BD:1090); Cmd+Z undoes the last change made there (a mended curve, a Quantize, an In or an Out). The keys and pads of the keyboard and MIDI mapping fire nothing into the live show, which is stopped. The controller works in Studio: while a recording only plays [page 2, E: item 229 accepted as written], a knob or fader he moves supersedes its control for as long as he holds it and goes back to the recorded track when he lets go, and the recording is not changed (his words on play mode, BD:340-342; what "let go" is for a knob: assumption E-32); during a Record Over what he moves is recorded over (195). (b) A show recording keeps the presses on the tempo row (play, pause, stop, Tap, Resync, the nudge, a tempo change), so the review picture goes empty where stop was pressed that night; they are shown and cannot be selected into an action. (c) The grid follows the night's tempo as it changed (INFERRED from L89).
CHANGED: (a) "the keys and pads of the keyboard and MIDI mapping fire nothing" is replaced for the controller: during a Record Over by L92 ("yes we can record over using the midi controller"), and while a recording only plays by his earlier words: "It could be in record over mode or it could just be in play mode and play mode. The timeline doesn't change and then record mode. The timeline is updated based on the knob movement." (BD:341-343). (b) stands. (c) stands; L89 says "When tempo changes, record that as it will happen often."; that the grid follows the recorded tempo is INFERRED from it, not his word.
TODAY: No space-key handler; Cmd+Z undoes the last fire app-wide (AR correction 5, M8). Tempo commands are recorded; a tempo-row stop does not exist yet (AR 1.8, H26; FT SURPRISES S4).
@@END

@@ITEM R186
TITLE: Record to Clip: always in time, always the full composition
STATUS: CORRECTED (d and e replaced; c follows L88 to the letter; the main reading of the empty parts in a turns round and is asked: page item 230) -- AMENDED after page 2 (R186 1 by E, R186 2 by E, R186 3 by E, R186 4 by E, R186 5 by E, R186 6 by E)
HIS: L88, L114
RULE: (a) A clip recording has no time limit except the disk; while it records its picture is seen in the cell it lands in. Its empty parts: it is a full picture exactly as on the output, global effects included, with no see-through parts ("This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like.", BF268) [page 2, E: BF268 "This is exactly the output recorded as one layer."], black where no layer shows (L88: "It must be the full screen composition"; his earlier "The recording should look exactly like the output.", binding-decisions.md:626); his earlier "yes" to see-through parts was given for a single-layer recording (binding-decisions.md:671), which L88 removes; keeping them see-through was put to him as way b of page item 256 and is not taken [page 2, E: BF268 "Whatever the output is displaying is what this should look like."]. (b) It lands in the next empty cell from the left on the top layer of the deck on screen, without that cell being selected. (c) It is always recorded in time: it starts on the "1" (the next bar line after the REC press) and it ends on a bar line: after the stop press it keeps recording until the end of the bar (L88: "keep recording till it is on an even grid line like the end of the bar"). The clip that lands is a BPM-mode clip with its beats and tempo set, whose file holds every recorded bar and whose out point the app sets at the end of the last full group of 4 bars, so that the clip loops only whole groups of 4 bars (5 recorded bars loop as 4); the rest stays in the file. He trims it, or brings the rest back in, later with its in and out points [page 2, E: BF254 "c"] (L88: "It can be trimmed later if necessary."). The recording never runs on until a group of 4 bars is full: it stops at the end of the bar (page item 232, way c). [page 2, E: BF254 "c"] (d) Nothing is recorded while the tempo is paused: a clip recording never starts or runs on a standing picture, and there is no clip recording that is out of time. A press on record to clip while the tempo is paused or stopped waits: the recording starts on the first 1 once the tempo runs (page item 258). [page 2, E: item 258 accepted as written] What a pause or stop does to a recording that runs: E-7. (e) There is no one-layer recording and no choice between whole and layer. A clip recording is the output of the layers, the full composition, and never the picture of one output screen, because a screen can be altered to fit a projector's colour and timing. The full composition is whatever the output is displaying: all layers mixed together with everything Global does to the picture, the global effects included (BF268). [page 2, E: BF268 "Whatever the output is displaying is what this should look like."] (f) A recorded clip has no sound. (g) A clip recording is never shorter than 4 bars: a stop press before the end of bar 4 ends it on the bar line that closes bar 4. (h) When it lands it waits in its cell, ready to be fired; it does not start playing by itself. (i) It lands in the deck that is on screen at the REC press; if the top layer has no empty cell a new column is added to that deck.
CHANGED: (d) "With the beat stopped or paused, REC starts and ends on your press and the clip is not in BPM mode" is replaced by L88: "it is impossible to record something paused. Anything is recorded it is recorded in time on the one and ending on a even amount. If we push stop recording, keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (e) the one-layer recording, its selected layer and its blend sentence are removed by L88: "I don't think there's any reason for a one layer recording. Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues. It must be the full screen composition." (c) the end is now his words (end of the bar). (a), (b), (f), (g), (h), (i) stand; his earlier "whole ouput of select layer" (BD:574) and the "yes" to a single-layer recording's see-through parts (BD:671) are overtaken for the layer half. BY THE RULING OVER ALL TOPICS (2026-10-08): (c) the out point is no longer moved in to the last full group of 4 bars by itself: L88 is read to the letter (end of the bar, trimmed later) and shown to him as page item 260; (a) the main reading of the empty parts turns round to "as on the output" and is asked as page item 230, because the only "yes" of his to see-through parts was for a single-layer recording.
TODAY: Record to clip does not exist in the app; only an old plan does, with a 64-bar automatic end, black for see-through parts, a text-only display and a Whole / Layer switch, all overtaken (AR 1.18, O3-O6, O13, H12-H18, H31). A tempo pause / stop does not exist yet (FT SURPRISES S4).
@@END

@@ITEM R187
TITLE: Recordings: quitting, opening a show, the disk, which picture
STATUS: STANDS (d is now his own words)
HIS: L88
RULE: (a) Quitting while a recording runs stops it and keeps it, with no extra window. (b) Opening another show or pressing New while a recording runs: the app asks once, in one window, saying that the recording will be stopped and kept, with the same three choices as at quit; then the show opens. (c) Record does not start when the disk is nearly full and shows "not enough HDD space to record"; if the disk runs low during a recording the recording ends by itself and keeps what is written. The amounts of free space are measured before it is built. Whether the app says that a recording was ended by the disk belongs to question 209 (another topic). (d) Recordings, films, the low-resolution recording and snapshots show the composition and never one screen's Delay, Brightness, Contrast or colour.
CHANGED: nothing. (d) is confirmed by L88: "Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues."
TODAY: The sound's disk floor (2 GB) is checked at the start only; the film has no disk check; nothing stops a recording when another show is opened (AR 1.7, 1.13, H20, M4, M5, T5).
@@END

@@ITEM R223
TITLE: The review screen and recordings: the edges
STATUS: CORRECTED (a added to; d and e replaced) -- AMENDED after page 2 (R223 1 by E)
HIS: L89, L80
RULE: (a) When the review screen opens nothing is selected and Save is greyed until something is. The tracks are greyed out and the selected area is white. The selected area is the selected tracks, between the In and Out points on the timeline; selecting a name selects all its tracks. Where In and Out stand when a recording opens: assumption E-36. (b) When playback reaches the end of the recording it stops on the last moment and stays there; play starts again from the beginning; it never carries on into the live show (assumption E-37: his earlier words on the end of a replay say otherwise). (c) While a recording runs, the way into the review screen and Record Over are greyed. (d) When something is removed during a recording, the removal is recorded and the thing is kept in the recorded show file; from the moment of its removal all its settings are at their defaults and its tracks show that (L89). So the review shows it working up to the removal and gone after it. The rule holds for anything taken out while recording, an effect, a clip or a layer. Anything added while recording is recorded from the moment it is added: the adding is recorded and the thing is put into the recorded show file, and those of its controls that change get tracks like every other control, flat at default before the adding (R137, L81), so that Studio shows the night as it was (page item 260). [page 2, E: item 260 accepted as written; BF204 "we are not recording anything that does not move."] (e) Tempo changes are recorded, as they happen often, and are shown. An action is not made with a tempo change: it never holds the tempo and never changes it. An action cut from a stretch in which the tempo moved: assumption E-18. A recording always has a tempo (D21), so its grid always shows bars.
CHANGED: (a) added: the greyed tracks and the white selected area, and what the selected area is (L89). (b) stands as shown; the page called it an edge no word of his covers, but his earlier words on the old replay do ("It should keep playing at with the current parameters at the end.", BD:503): carried by E-37. (d) "an effect you remove while recording keeps its rows, which move nothing once it is gone" and its closing question are replaced by L89: "If it is removed, record the removal and keep it in the recorded show file, but default all its settings once removed." INFERRED: in "Very little chance a recording will be removed" the word "recording" is read as the thing removed while recording (E-17). The page's "an effect you add while recording has no rows and is not added again in the review" is not touched by his words; E-35 proposes the opposite and shows it to him. (e) "takes the tempo at its In point" is replaced by L89: "When tempo changes, record that as it will happen often. Actions will not be made with a tempo change."; the sentence on 175 B falls away (175 takes its default A, L1). (c) stands.
TODAY: A replay holds the last look in the live window until "Stop Playback" (AR 1.11). Adding or removing an effect is not a recorded event (AR 1.8). A recording started before a tempo is known cannot be cut (AR 1.9, H25).
@@END

@@ITEM R208
TITLE: The units a track shows
STATUS: STANDS
HIS: L86
RULE: A track of the review screen shows the number that its slider shows: Position, Scale, Rotation and Anchor read 0 to 1 with 0.50 in the middle, as their sliders do, so that a track and its control always agree. A button's track runs between 0 and 100, the bottom and the top of its row (L86).
CHANGED: nothing; he did not ask for degrees or radians. L86 gives the button track its 0 and 100.
TODAY: The sliders read 0 to 1 with two decimals; rotation is kept in degrees underneath (as cited on the page: src/ui/UniversalParamControl.cpp:198, src/ui/CompositionInspector.cpp:163, 169; not re-read).
@@END

@@ITEM D16
TITLE: The review picture is made fresh from the recording
STATUS: SETTLED (L80 for the show it is made with; the rest by R183 a, read and not corrected) -- AMENDED after page 2 (D16 1 by E)
HIS: L80, L84, L92, L6
RULE: The picture in the review screen is made fresh each time from the recorded moves and the recorded show file; it is not a film of the night. So a mend or a Record Over changes the picture that is seen. The clips' files must still be on the disk; a clip whose file is not found is named as such when the recording opens. What the app picks by chance during the night: assumption E-38. Under a fast scrub a video's picture may lag a moment: assumption E-28. The low-resolution show recording (G4) is the film of the night that this picture is held against, so that a mistake in the parameter recording can be seen (BF253); how Studio shows the two together: assumption E-23. [page 2, E: BF253 "so that we can see if there is a mistake in the parameter recording"]
CHANGED: "with the show it was recorded in" becomes the recorded show file (L80). Ruling: the paper's sentence that the low-resolution recording "is the film to check this picture against" is cut back to what L6 says ("to use as a double check"); what chance does to a picture made fresh is named (E-38); the scrub remark points at E-28.
TODAY: A recording stores no picture; the only renderer is the live one, and it can only play a recording from its start (FT section 1, S5; AR T3).
@@END

@@ITEM D17
TITLE: What a recording is able to keep
STATUS: REPLACED in part by L81
HIS: L81, L86, L76, L80
RULE: A recording can keep any button, slider, drop-down list or play / direction button of the clip, layer and global tabs and of the layer strip, however it is moved, the mouse included; but it keeps a track only for those that changed during the night (L81). Where every control stood when Record Show was pressed is in the recorded show file (L80; assumption E-34), so the review picture is right from its first moment.
CHANGED: "will keep every slider and button ... from its first moment" is replaced: "we are not recording anything that does not move." (L81). That any kind of control can be recorded is his: "Each track recorded, button or slider" (L86) and "the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes" (L76). Ruling: that the recorded show file is taken at the Record Show press is Harmony's reading of L80 and is carried by E-34.
TODAY: More than half of a clip's controls and all mouse moves cannot be recorded (AR 1.8, T4; FT section 2). This is the largest piece of the build.
@@END

@@ITEM D18
TITLE: Moves by hand, by a signal, by an action: all recorded
STATUS: SETTLED
HIS: L71, L66
RULE: A recording keeps what a control did whoever moved it. Moves recorded off signals are shown in the review screen in a different colour, so that the user sees it before creating actions; an action can be made from them and is treated like a regular action. When an action was playing during the recording, its moving parameters are shown as tracks in a slightly different colour and the action's name is shown in the hierarchy; a new action made from them holds the moving parameters themselves and never an action inside an action. So there are three colours of moves: by hand, by a signal, by an action. The moves a signal made are kept as values (assumption E-24).
CHANGED: "with a mark that a signal drove it" becomes his different colour (L71: "we need to have a different color for the actions that were recorded off of signals"); the moves of a playing action get their own colour and the action's name (L66: "those parameters will be slightly different colored and the name of the action shown somehow in the hierarchy").
TODAY: Nothing that a fired routine or a signal does to a control is recorded (AR M1, 1.8).
@@END

@@ITEM D19
TITLE: The sound of a recording when the music is a file
STATUS: SETTLED by his earlier words (BD:283-286)
HIS: none in this message (his earlier words, BD:283-286)
RULE: A show recording with Audio ticked keeps the sound the app heard, whatever its source: the microphone, the sound card or an audio file played in the app. That the sound stays in time with the recorded moves is tested when it is built.
CHANGED: nothing in the rule. Ruling: it is no longer Harmony's own. His words: "it was either the Audio from the microphone or from the sound card or from whatever. It gets recorded Along with the log and we will test it to make sure it stays in time." (BD:284-286). Assumption E-25 is dropped.
TODAY: Recording arms on the file source or the live input (AR M6); not checked by a run.
@@END

@@ITEM D20
TITLE: Several recordings at the same time
STATUS: OPEN (Harmony's own; assumption E-26)
HIS: L6
RULE: Record Show with any of its boxes, the low-resolution recording and a clip recording can run at the same time. If the computer cannot keep up, the smoothness of the output comes first and a recording loses frames. It is measured before it is built.
CHANGED: The low-resolution recording of L6 joins the list of things that can run together.
TODAY: No measurement exists; each film reads the composition's picture back from the graphics card (AR H29, T2).
@@END

@@ITEM D21
TITLE: A recording always has a tempo
STATUS: SETTLED by the default of question 175 (L1); the topic-A paper owns 175
HIS: L1, L89
RULE: The tempo number is never empty (175 A), so a show recording always has a tempo: its grid always shows bars and any part of it can become an action.
CHANGED: The condition "If you answer 175 A" is met by his L1 ("All defaults good except for these"); 175 is not named by him.
TODAY: A recording started while no tempo is known cannot be cut (AR 1.9, H25).
@@END

@@ITEM D22
TITLE: The file types of a film and of a recorded clip
STATUS: OPEN (that the app picks is his word, BD:650-651; which types is Harmony's: assumption E-27; it waits for E-9 and for the answer to his codec question L3) -- AMENDED after page 2 (D22 1 by E, D22 2 by E)
HIS: L3, L6
RULE: The app picks the file types itself (his words: "We should pick the most optimal Kodex to use and not worry about the other ones." and "I want codecs that decode easily and play well", BD:650-651): a film and the pieces of the small film open in ordinary players; a recorded clip is in the type that plays back most lightly in the app and carries no see-through parts, being exactly the output's full picture (BF268) [page 2, E: BF268 "This is exactly the output recorded as one layer."]. A film plays at the right speed whatever the output's frame rate. Which types exactly, for a film, for a recorded clip and for the low-resolution show recording, is decided by the test of codecs after the build: which codecs encode and decode well on his Mac (BF240, BF241). The topic on how a clip plays owns that test. [page 2, E: BF241 "we should test which codecs work best."; BF240 "can we test encoding and decoding of different codec"]
CHANGED: The see-through half ("a recorded clip keeps its see-through parts") now hangs on assumption E-9, since the one-layer recording it was ruled for is gone (L88). Ruling: the first sentence is marked as his own words (BD:650-651), so only the choice of the types is still open.
TODAY: The film is H.264 only, without sound, and probably plays in slow motion (AR 1.13, 1.14, 1.17, T1, T2: inferred, not run).
@@END

@@ITEM P17
TITLE: The review screen's layout
STATUS: DROPPED
HIS: L8, L79
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His function for it is in R138: the line between picture and tracks can be dragged, and the picture can go to a monitor (L79).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P18
TITLE: The review screen's left side: names, hierarchy, folding
STATUS: DROPPED
HIS: L8, L81, L66
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Only changed controls have tracks (L81), under clip, layer and global; an action that played is named in the hierarchy (L66).
CHANGED: The variants' "mark on the ones that moved" is no longer needed: everything listed moved.
TODAY: not checked
@@END

@@ITEM P19
TITLE: The review screen's tools
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The tools themselves: pointer and pencil (187), In and Out and the grid size (R135), Quantize with its spacing (R166), Record Over (195), the scrub sound setting (R183).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P20
TITLE: Picking tracks and saving
STATUS: DROPPED (the look is his: L89)
HIS: L8, L89
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His words give the look meanwhile: the tracks are greyed out and the selected area is white (R223 a).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P21
TITLE: How the review screen is opened and closed
STATUS: DROPPED
HIS: L8, L35
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). It is built as one of the main window's configurations, beside live: "We will have various configurations other than live and recording review mode" and "no need to undock and move just yet" (L35). Opening it stops the live show either way (R165).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P22
TITLE: The window that saves an action
STATUS: DROPPED (what it does is the topic-D paper's)
HIS: L8, L55, L65
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What the window does (the app breaks a mixed selection into clean actions, L55; a warning when an action is not a multiple of 4 bars, L65) is ruled in the actions topic.
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P23
TITLE: The REC button and the sign that a recording runs
STATUS: DROPPED (its Whole / Layer choice is removed by L88)
HIS: L8, L88
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). REC has no Whole / Layer choice (L88). A running recording is prominently displayed, his earlier word (BD:625).
CHANGED: The Whole / Layer choice is removed: "I don't think there's any reason for a one layer recording." (L88).
TODAY: not checked
@@END

@@ITEM P24
TITLE: The list of recordings and the three boxes
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM P25
TITLE: What the audio track shows
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Meanwhile it is the waveform across the top with the common grid of the night's bars over it (assumption E-31).
CHANGED: nothing
TODAY: A recording can carry marks for drum hits, but no control sets it (AR M7).
@@END

@@ITEM G4
TITLE: A small film of the show in pieces, as a double check
STATUS: ANSWERED (his cost question is answered in the answer lowres; whether and how it is built is his choice: assumption E-21) -- AMENDED after page 2 (G4 1 by E, G4 2 by E, G4 3 by E)
HIS: L6, L88
RULE: With Record Show the app writes a small film of the show without sound, the low-resolution show recording, unless it is switched off: as a double check (L6), and so that a mistake in the parameter recording can be seen (BF253). [page 2, E: BF253 "It defaults to running next to the show recording so that we can see if there is a mistake in the parameter recording."] It shows the full composition, never one screen's corrected picture (L88). It is written in pieces of 10 minutes, each a small file [page 2, E: item 262 accepted as written] that plays by itself, so that if something happens most of the recording is not lost. Pieces alone do not do that: a film that is cut off by a crash cannot be played, so each piece is also written in a way that a crash costs only its last seconds; a forced quit in the middle of a piece is the test, and nothing is promised before it. How the matter stands now: his question on the cost has its answer (the answer lowres): it should be light on the computer, every number in it is an estimate, and the one thing nobody knows is whether fetching the small picture makes the live picture hitch. That is measured on his Mac with a heavy show before the film is left switched on; if it costs the output its smoothness he is told, and the output comes first (assumption E-26). He wants it: it runs by default next to every show recording and can be switched off; it has no sound, whatever the Audio box says; it is there so that a mistake in the parameter recording can be seen (BF253), and it is just for reference: a film in full quality is made with Render in Studio (BF241). Its size waits on his answer: a quarter as wide and as high as the show, with his "something close to the output monitor resolution" read as the output monitor inside the app and not as an output screen's own size (assumption E3-1). It runs at 30 pictures a second at the most, and at 15 if the test after the build shows that 30 is heavy; both are fine with him (BF241). Its codec is picked by the test of codecs after the build (BF241; the topic on how a clip plays owns that test). [page 2, E: BF253 "It has no sound."; BF241 "This is just for reference."; BF241 "30 fps is the fastest or even 15 could be ok."] Where the pieces are kept and watched: assumption E-23.
CHANGED: New; it answers no item of the page. His words: "I think it might be good to record a very low resolution show recording if that is possible and to record it in 10 or 20 minute chunks so that they are small and if something happens most of the recording is not lost. Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check. How much comp resource would this take up?" (L6). Ruling: the paper's "without a gap", "numbered in order" and "built only if the researcher's answer shows ..." were no words of his and are taken out or carried by E-21 and E-23; the cost question is answered from answer-lowres-rec.md as corrected by check-lowres.md (the ranges are widened: the re-check found the encoder about twice as costly and its files about twice as large as the answer paper assumed; nothing was run by either).
TODAY: The only film recorder writes one full-size file with its index at the end, so a crash leaves the file unplayable: no pieces, no fragments (VideoRecorder.cpp, check-lowres fact 8). It cuts a corner out of the picture instead of shrinking it (VideoRecorder.cpp:117-130) and has no steady picture rate, so the file would play at the wrong speed (inferred, not run; check-lowres facts 6, 7). It carries no sound (VideoRecorder.h:34) and uses the software encoder only (check-lowres facts 3, 4). Nothing of the small film exists and no measurement exists.
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME E3-1
ABOUT: 262, 231, G4, answer lowres
TEXT: I assume you mean the output monitor inside the app, not an output screen such as the projector: so the low-resolution show recording stays small, a quarter as wide and as high as the show (480 by 270 for full HD).
WHY: He left 262 empty (a quarter) and asks in BF241 "Maybe something close to the output monitor resolution?"; the name fits the monitor in the app or a connected screen; his next sentences in the same box ("This is just for reference. If we want a very hd recording, we can make an HD render from the Recording Review") speak for a small film. The monitor in the app has no fixed size, so no number is read from it.
ALT: b) Larger: half as wide and as high as the show (960 by 540 for full HD). c) You mean the output screen's own size: then it is a full-size film, heavier, and no longer small.
IF-WRONG: SMALL one number; the full-size way is the heavy one and is not taken without his word.
ASK: LINE his question in the box must get this line, as the choice under the answer on this recording; but by his own "This is just for reference." a wrong size is never seen on stage and is changed in a moment.
@@END

@@ASSUME E3-3
ABOUT: R183, R184, 215
TEXT: I assume the Video box of Record Show stays, as you answered before: ticked, it writes a full-size film while the show runs. Render in Studio makes such a film afterwards.
WHY: His answer "48 b" (binding-decisions.md:867) gave Record Show three boxes, Video among them; BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" names Render as a way to a full-quality film and does not say that the box goes.
ALT: b) The Video box goes: during a show only the low-resolution show recording is filmed, and a full-quality film is always made afterwards with Render.
IF-WRONG: SMALL one box more or less; way b spares the heaviest of the recordings during a show.
ASK: LINE his earlier answer is the text; his new words may make the box unneeded, and he can strike it in a second. Print it once: topic J's J-1 is the same question with the opposite text.
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

@@ASSUME E3-6
ABOUT: R165, 226
TEXT: I assume opening Studio stops everything that plays (the clips leave the layers, the tempo, the actions and an audio file stop) but leaves your action buttons as you set them: when you close Studio the show is as it was, with nothing playing.
WHY: Old item R165, which he read and left, says both that opening it "is like a press on stop" and that closing it brings back "the show as it was"; under page item 226 (BF250 "tempo stop stops all actions, not just global") a tempo stop may switch every action button off, so both cannot hold. The sentence on closing is kept; topic D's ruling advises the same.
ALT: b) Opening Studio is a tempo stop in everything: the action buttons go off as at any stop, and you switch on again the ones you want.
IF-WRONG: SMALL what the opening does to the buttons can be changed without touching Studio.
ASK: LINE two sentences he read pull two ways once a stop switches buttons off, and he would see the difference each time he comes back from Studio; it hangs on his answer to topic D's question on the stop and falls away if no button goes off at a stop.
@@END

@@ASSUME E3-7
ABOUT: 257, 195, R185
TEXT: I assume Record Over works with the keyboard and MIDI mapping that the show recording's own show file holds, and that the tempo bar's keys and pads and the recording buttons do nothing while Studio is open: the recorded tempo stays.
WHY: Item 257 (accepted) says "everything in the keyboard and MIDI mapping"; no word says whose mapping works in Studio, which works on the recording's own copy of the show (old items R165, R173); old item R138, which he read, says "The tempo row's buttons are not on this screen", and the recorded tempo is the grid of the recording (old items R167, R185 b).
ALT: b) The mapping of the show that is open in the live window is used. c) The tempo bar's keys and pads record over too.
IF-WRONG: SMALL which mapping is read is one choice, seen only in Studio.
ASK: NO technical; it follows the rule that Studio works on the recording's own copy of the show.
@@END

@@ASSUME E-4
ABOUT: 195, R222
TEXT: I assume all your mends of one recording go into one mended copy beside the original, saved by itself as you work. Each Record Over makes one more copy. Every copy is complete and shares the original's sound.
WHY: L84 "parallel to the original" and L92 "recorded separately" do not say how many copies, when one is saved, or whether a copy is whole.
ALT: b) Every time you mend and close, one more copy is made. c) Only the changes are kept, as a layer over the original that you can switch off.
IF-WRONG: SMALL the list of recordings would be grouped differently.
ASK: LINE a choice of mine on how the list fills up.
@@END

@@ASSUME E-7
ABOUT: R186
TEXT: I assume a tempo pause holds a clip recording: nothing is recorded while paused, and it carries on in time when you press play. A tempo stop ends the recording and keeps what was recorded.
WHY: L88 rules out recording a paused picture; L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM"; L72: stop stops "everything else".
ALT: b) A pause also ends the recording, at its last full bar.
IF-WRONG: STAGE a recording that ends, or goes on, when he did not mean it.
ASK: LINE the hold follows his L13 words, and neither way loses what was recorded.
@@END

@@ASSUME E-12
ABOUT: R173, D16
TEXT: I assume the show file that a recording saves holds every clip's settings and where its files are on the disk, not copies of the picture and video files themselves.
WHY: L80 "a show file with the clips exactly as they are", and "moved or missing", fit a file that points at the media.
ALT: b) The recording also copies the media files, so that it opens even when they are deleted; it would be very large.
IF-WRONG: SMALL copying media could be added later as a command.
ASK: NO technical; his "moved or missing" (L80) already expects a file that points at the media.
@@END

@@ASSUME E-13
ABOUT: R137, 188
TEXT: I assume a control that never moved but stood away from its default the whole night also gets a track: a flat line at that value.
WHY: L81 records nothing "that does not move", yet drops only what "was at default the entire time"; an action holds "all parameters that are changed from it’s defaults" (L76).
ALT: b) No track: only controls that moved during the recording get one.
IF-WRONG: SMALL more or fewer tracks in the list.
ASK: LINE his sentence can be read both ways; the cost is small.
@@END

@@ASSUME E-15
ABOUT: R166
TEXT: I assume Quantize works on the selected area: the button and clip-fire tracks you selected, between In and Out. It never reshapes a slider's curve.
WHY: L83 gives the spacing list, not what a Quantize covers; L89 defines the selected area.
ALT: b) It always works on every button and clip fire of the whole recording.
IF-WRONG: SMALL a mend can be undone.
ASK: LINE a choice of mine that follows his selected area.
@@END

@@ASSUME E-16
ABOUT: R167
TEXT: I assume the night's deck switches still show as marks in the review screen, and can never be put into an action.
WHY: L85 "action should not record switching decks" is about actions; the reading he read says the deck changes are shown, and he did not change that.
ALT: b) Deck switches are not recorded at all and the review screen shows none.
IF-WRONG: SMALL one track more or less.
ASK: NO a deck switch never changes the picture; whether its mark is drawn is a look (L8).
@@END

@@ASSUME E-18
ABOUT: R223, R167
TEXT: I assume "actions will not be made with a tempo change" means an action never holds or changes the tempo. You can still cut one from a stretch where the tempo moved; it keeps its bars and beats.
WHY: L89 can also mean that no action may be cut across a tempo change; in automatic mode the tempo moves a little all the time.
ALT: b) The app refuses to save an action whose In-to-Out stretch holds a tempo change.
IF-WRONG: SMALL a refusal could be added to the save window.
ASK: LINE the other reading would block most of a night recorded in automatic mode.
@@END

@@ASSUME E-19
ABOUT: R183, R138
TEXT: I assume the scrub sound setting starts switched off: you hear the recording only while it plays in real time, until you switch scrub sound on.
WHY: L86 asks for "a setting to not scrub sound" and does not say how it starts.
ALT: b) It starts switched on, and you switch it off when it bothers you.
IF-WRONG: SMALL one tick.
ASK: NO a setting's first state; he said it "won't sound good if it's scrubbed" (L86).
@@END

@@ASSUME E-23
ABOUT: 231, G4, D16
TEXT: I assume Studio can play the low-resolution show recording together with its own picture, both always at the same moment of the show, also while you scrub, so that a mistake in the parameter recording shows at once.
WHY: BF253 says what it is for ("so that we can see if there is a mistake in the parameter recording"), not how the two are compared; no word of his says whether Studio shows the film.
ALT: b) Studio does not show it: the pieces are plain film files that you open in any player and compare by eye.
IF-WRONG: REBUILD a second picture in Studio that follows the playhead is a piece of work of its own; without it the check is done by hand, piece by piece.
ASK: YES his new words give the film its purpose, and whether Studio serves that purpose is a function he would look for there.
@@END

@@ASSUME E-24
ABOUT: D18, R183
TEXT: I assume the moves that a signal makes on a slider are stored as values in the recording, so its track shows them even when the Audio box was not ticked.
WHY: L71 says moves are "recorded off of signals"; how they are kept is not his concern.
ALT: b) They are worked out again from the recording's sound each time it is reviewed.
IF-WRONG: SMALL internal.
ASK: NO technical.
@@END

@@ASSUME E-28
ABOUT: D16
TEXT: I assume that under a fast scrub a video clip's picture in the review screen may lag a moment behind the mouse.
WHY: A technical limit of drawing the picture fresh; no word of his.
ALT: none
IF-WRONG: SMALL
ASK: NO technical.
@@END

@@ASSUME E-30
ABOUT: R187, R186, 232
TEXT: I assume a record to clip cut short (a quit, a full disk, a tempo stop, its own stop pressed in a tempo pause) ends at once and is kept: out point at the last full group of 4 bars, a shorter one whole.
WHY: L88's "keep recording till it is on an even grid line" cannot hold at a quit, a full disk or a tempo stop, nor while a tempo pause holds the recording (old assumption E-7): the bar does not end. Under 4 bars is not covered. It agrees with item 232, way c (BF254).
ALT: b) With fewer than 4 bars recorded nothing is kept, and the app says so.
IF-WRONG: SMALL a rare edge.
ASK: NO a technical edge; the default never throws a recording away.
@@END

@@ASSUME E-31
ABOUT: P25
TEXT: I assume the audio track shows the waveform with the night's bar lines over it, and no marks for single drum hits.
WHY: His reference words put "the audio track across the top" and a grid over all (BD:1062-1064); nothing more.
ALT: b) A mark at every drum hit the app heard.
IF-WRONG: SMALL
ASK: NO a look, settled in the UI redesign (L8).
@@END

@@ASSUME E-32
ABOUT: 195, R185, 257
TEXT: I assume a knob on the controller counts as let go once it has stood still for a quarter of a second, as everywhere else in the app, whether it sends its position or is endless; then the recorded moves take over again.
WHY: His words: "will snap back to the recorded track as soon as it is let go" (binding-decisions.md:340-341); a knob cannot tell the app that a hand has left it. The mapping rule he read gives a quarter of a second (old item R199 e), and BF270 wants both kinds of knob to work; "about one beat", this assumption's earlier number, is given up so that there is one number.
ALT: b) Once you move a control in Record Over it stays yours until you stop Record Over.
IF-WRONG: SMALL one number, tuned at the build.
ASK: NO one number with the mapping rule he read and left.
@@END

@@ASSUME E-33
ABOUT: 186, R138
TEXT: I assume the recording's sound in Review plays through the Mac's own sound output, the one chosen in the Mac's sound settings.
WHY: No word of his names where Review's sound comes out; "186 b" (L91) names the picture.
ALT: b) A sound output chosen inside the app.
IF-WRONG: SMALL a setting could be added.
ASK: NO technical.
@@END

@@ASSUME E-34
ABOUT: R173, D17, R165
TEXT: I assume the show file a recording saves is taken the moment you press Record Show, with every change you had made, saved or not. Review takes the clips and settings from it and leaves your outputs as connected.
WHY: L80 "with the clips exactly as they are for the show" does not name the moment; a show also remembers its connected outputs (L96), which Review must not change.
ALT: b) It is a copy of the show file as it was last saved.
IF-WRONG: SMALL Review would start from another look than the night had.
ASK: NO technical; only this reading lets Review start as the night started.
@@END

@@ASSUME E-36
ABOUT: R223, R135
TEXT: I assume that when a recording opens in Review, In sits on its first bar line and Out on its last, so the whole recording lies between them until you move them.
WHY: L89 defines the selected area by In and Out; no word says where they stand at first.
ALT: b) There is no In and no Out until you set them.
IF-WRONG: SMALL a starting position.
ASK: NO a starting position that he moves anyway.
@@END

@@ASSUME E-38
ABOUT: D16, D18
TEXT: I assume what the app picks by chance while you record (a Random jump, an autopilot pick) is written into the recording, so that Review repeats it; where that cannot be done, Review may differ a little from the night.
WHY: The Review picture is made fresh from the recording (L80, L84); no word of his covers what the app decides by chance.
ALT: b) Chance is not recorded: Review picks again.
IF-WRONG: SMALL the small film of the night shows the difference.
ASK: NO technical; nothing is promised here before it is measured.
@@END

## CLOSED ASSUMPTIONS (one line each)
- E-1 -> SETTLED
- E-2 -> page 2, item 229
- E-3 -> page 2, item 257
- E-5 -> page 2, item 258
- E-6 -> page 2, item 232
- E-8 -> MERGED 256
- E-9 -> page 2, item 256
- E-10 -> page 2, item 259
- E-11 -> page 2, item 230
- E-14 -> SETTLED
- E-17 -> page 2, item 260
- E-21 -> page 2, item 231
- E-26 -> page 2, item 261
- E-27 -> CLOSED by page 2 (E): BF241 ("we should test which codecs work best") and BF240 (a test of encoding and decoding of different codecs after the build) settle how the file types are picked: by that test, which topic H owns; its way b, a codec of our own, was answered no on page 2, and BF268 removes the see-through choice it also waited for. Old item D22 is amended.
- E-35 -> MERGED 260
- E-37 -> SETTLED

## QUESTIONS BACK
@@ANSWER lowres
HIS: BF241 "Maybe something close to the output monitor resolution?"
ANSWER: Yes to the test, the rate and the purpose. The codec is picked by the test of codecs after the build. It runs at 30 pictures a second at the most, and at 15 if the test shows that 30 is heavy. It is for reference only, without sound, in 10-minute pieces, next to every show recording; a very HD film is made afterwards with Render in Studio. The size: I take the output monitor to be the one inside the app, not the projector, so the film stays small. I start at a quarter as wide and as high as the show (480 by 270 for full HD) and make it larger if that proves too small to judge by. If you meant the projector's own size, say so: that is a full-size film and no longer light. None of this is measured yet.
@@END

## NAMES
@@NAME Record Show
MEANS: The recording of the whole show: the moves of the controls, the night's sound and, if ticked, a film; it is what the review screen opens.
SOURCE: his words L114 ("we will record to clip or record show. That’s what the 2 recordings are called. All else is gone."); the on-screen name now, "Record Take", is replaced by it
@@END

@@NAME Record to Clip
MEANS: The recording of the full composition's picture into a video file that lands as a clip in the next empty cell of the top layer.
SOURCE: his words L114; "REC" is the short label used on the page for its button
@@END

@@NAME show recording
MEANS: One recording made with Record Show; the word "take" is not used on screen or in talk.
SOURCE: his words L6 ("show recording") and L114 ("Show Recording Review"); the on-screen name now, "Take", is replaced by it
@@END

@@NAME recorded show file
MEANS: The show file that a show recording saves with it, holding the clips exactly as they were, from which the review screen opens the recording.
SOURCE: his words L89 ("keep it in the recorded show file") and L80
@@END

@@NAME Record Over
MEANS: Recording new moves from the MIDI controller over a show recording while it plays in real time; kept separately from the original.
SOURCE: his words L92 ("record over"), and earlier BD:499-500
@@END

@@NAME mend
MEANS: A change made to a show recording with the mouse (a drawn shape, a moved piece, a Quantize), saved as another recording parallel to the original.
SOURCE: his words L84 ("the mend is another recording"); the word first stood on Harmony's page
@@END

@@NAME track
MEANS: One row of the review screen: what one button or slider did over the length of the recording.
SOURCE: his words L86 ("Each track recorded, button or slider") and L89
@@END

@@NAME selected area
MEANS: The selected tracks between the In and Out points; it is white while the other tracks are greyed out, and it is what an action is saved from.
SOURCE: his words L89
@@END

@@NAME grid spacing
MEANS: The setting a Quantize asks for: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat or 1/4 beat.
SOURCE: his words L83
@@END

@@NAME scrub sound
MEANS: The setting that decides whether the recording's sound is heard while scrubbing, or only while it plays in real time.
SOURCE: Harmony's pick, from his words L86 ("a setting to not scrub sound")
@@END

@@NAME the review screen
MEANS: The one screen where a show recording is watched, mended, recorded over and cut into actions; this paper's working word for the screen he calls Review.
SOURCE: his words L114 ("Show Recording Review should be called Review for short is a good name"): the name on screen and in talk is Review, and the keyboard-and-menus topic owns its name block and his question for a better name; "the recording review screen" is his L72
@@END

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

## REACHES OTHER TOPICS (from the paper)
- K (old item 215 f, assumption K-2): BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" confirms Render as a function of Studio and gives it its job, the film in full quality. It is not a new function: 215 (f) already rules it (the whole recording, at the full size of the show, with its sound, not in real time). "very hd" is read as that full size and no larger. K's paper applies it to 215 (f); this paper amends only R184 and asks about the Video box (E3-3).
- H (the test of codecs after the build, BF240): BF241 "we should test which codecs work best" puts the codec of the low-resolution show recording, and with D22 the file types of a film and of a recorded clip, into that test. BF268 adds one fact for it: a recorded clip needs no see-through channel. BF241 also puts a second question to a test after the build, 30 or 15 pictures a second, which is a load test of this topic (E3-2) and can be run in the same session.
- H (item 238, BF258 "b"): a clip brought in in BPM mode is cut in to whole groups of 4 bars; BF254 "c" gives a recorded clip the same rule (loops whole groups of 4 bars). One rule in both papers.
- J (the name, item 245, BF245, BF263): every old RULE of spec-E says "the review screen" or "Review"; it reads Studio from now on. Not amended line by line here; one global replacement when the specs are merged. "Review picture" becomes Studio picture (Harmony's pick, above).
- A (BF271 "Stop removes all clips from all layers", box 273): with it a tempo stop leaves nothing to record for record to clip; old assumption E-7 (a tempo stop ends a clip recording and keeps it; a pause holds it) is not contradicted and stays as it is. If A's paper rules stop or pause otherwise, E-7 and item 259 need a second look.
- D (item 230): how "the same clip" is recognised after it moved, and where an action is stored, must be one rule in both papers (unchanged from the old spec).
- F (BF243, the snapshot as "a save of the show exactly where it is"): the recorded show file (old item R173, assumption E-34) is the same kind of save taken at the record show press; F's paper may want one mechanism for both. Nothing changes here.
- Applied here from another topic's box: BF245 / BF263 (Studio); BF240 (the test of codecs, into D22); BF258 (item 238 b, as agreement with 232 c).

## CONFLICTS (from the paper)
- The size of the low-resolution show recording. New, BF241: "Maybe something close to the output monitor resolution?" against item 262, whose box he left empty in the same sitting = accepted as written: "a quarter as wide and as high as the show (480 by 270 for full HD)", and against his earlier "Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check." (binding-decisions.md:1112, BF161). Not picked: assumption E3-1, asked.
- The sound of the low-resolution show recording. New, BF253: "It has no sound." against the page's own text for item 231 ("with sound") and the old ruling in G4 (sound when the Audio box is ticked). No earlier word of his said it has sound; his newest words win; no question.
- Where a recorded clip ends. New, BF254: "c" (loops only whole groups of 4 bars; the rest stays in the file) against the ruling over all topics of 2026-10-08 that read his "keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (binding-decisions.md:1170) to the letter (5 bars stay 5). His letter wins, and it agrees with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (binding-decisions.md:812). No question.
- See-through parts of a recorded clip. New, BF268: "This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like." against his earlier "yes" to see-through parts, given for a one-layer recording (binding-decisions.md:671 as cited by old item R186), and in line with "The recording should look exactly like the output." (binding-decisions.md:626). The newer words win; no question.
- The purpose of the low-resolution show recording. New, BF253: "so that we can see if there is a mistake in the parameter recording" beside his earlier "so that they are small and if something happens most of the recording is not lost" (binding-decisions.md:1112). Read as two purposes that both hold (the pieces stay); no question.

## NOT DONE / UNSURE (from the paper)
- What "the output monitor resolution" names (BF241): NAMES.md gives "output monitor" as the monitor inside the app (his words of 2026-10-05), and his R138 line uses "monitor" for a connected screen (binding-decisions.md:1161). Not settled by reading; E3-1 asks. The size at which the output monitor is drawn in the app was not read from the program text, so the @@ANSWER says "I take it to be near" and claims no number for it. Cheapest: his answer to E3-1; and one look at the monitor panel's size before the page is printed, so that the sentence can be firmer or cut.
- "the output" in BF268 is read as the picture (the output monitor), not one output screen with its own corrections; his earlier words carry that reading (binding-decisions.md:1170). Not asked.
- "the parameter recording" in BF253 is read as the recorded moves (the Parameters box). Not asked.
- Whether the test "light enough" still gates the default: he wrote "It defaults to running" without the page's condition. Kept as Harmony's safety from G4 (he is told before anything is decided if the test shows a stutter). If the page ruling wants it said to him, it is one line.
- Render's details (whole recording or the stretch between In and Out; its size) are K's assumption K-2; nothing new is assumed here beyond E3-3.
- R183 (d) "with no box ticked the button is greyed": whether the low-resolution box alone counts as a ticked box was not ruled (an edge; E3-4 covers its neighbour).
- Old assumptions tested and left untouched: E-4, E-7, E-12, E-13, E-15, E-16, E-18, E-19, E-24, E-28, E-30 (agrees with 232 c now), E-31, E-32, E-33, E-34, E-36, E-38. Changed: E-23. Dropped: E-27.
- TODAY lines are taken from the old spec's TODAY lines; nothing was re-derived from the program text and nothing was run.
- Written 2026-10-09 18:49:14 EDT.

## FOR THE PAGE RULING (from the ruling)
- MUST be put to him, in this order of weight: (1) E-23 (YES): does Studio play the low-resolution show recording together with its own picture; his BF253 gives the film this purpose and no word says how the two are compared. (2) E3-1 (LINE): what "the output monitor" names, and with it the size; print it as the choice under the answer lowres (as 262 stood under that answer on page 2); item 262 stays OPEN on it. (3) E3-3 (LINE): the Video box of Record Show. (4) E3-6 (LINE): opening Studio leaves the action buttons as he set them.
- E3-1 is a LINE and not a YES although his box asks back: by the three tests a wrong size is one number and is never seen on stage (his own "This is just for reference.", BF241). It must still be printed; do not let it fall with the internal ones.
- E3-3 and topic J's J-1 are ONE question with opposite texts: print E3-3 only. His explicit answer is "48 b" (binding-decisions.md:867: three boxes, Video among them), so "the box stays" is the text and "the box goes" is way b; J-1 as written would flip an answer he gave. Topic J's ruling should drop J-1, or the page ruling drops it.
- Not for him (ASK NO): E3-4, E3-5, E3-7, and the old E-30 and E-32, both written again; E3-2 is dropped (his BF241 settles the range of the rate). The other old internal assumptions of this topic are untouched.
- Newest words against earlier ones: nothing of this topic has to be said to him again as a conflict. BF268 (no see-through parts) goes only against a "yes" he gave for the one-layer recording, which he removed himself; BF254 "c" goes against no word of his (it agrees with his "always work with multiples of 4", binding-decisions.md:812); BF253 "It has no sound." goes only against the page's own text. The size (BF241 against the quarter he left in 262 and his earlier "1/4 or 1/8 the size") is the line E3-1.
- Depends on topic A: what the tempo stop and the tempo pause do. AMEND R165 2, item 259, the last sentence of item 232 and the old E-7 and E-30 lean on A's reading that the stop takes every clip off every layer at once and that a pause holds what follows the beat; if A's ruling changes either, re-read them. Item 232 also leans on A for when the 1 can move (BF246).
- Depends on topic D: E3-6 hangs on D's question on what a tempo stop leaves of the action buttons (D3-1): print E3-6 beside it or fold it into it; if no button goes off at a stop, E3-6 falls away. Topic D's ruling advises the text E3-6 has. How "the same clip" is recognised is one rule with item 230.
- Depends on topic K: the audio file under a stop (the silence in item 259; AMEND R165 2); Render (old item 215 f): if he takes way b of E3-3, Render is the only way to a full-quality film and moves up in K's order.
- Depends on topic H: the test of codecs (the codec of the low-resolution show recording and of a recorded clip; the 30-or-15 test can run in the same session); ONE cut rule for items 232 and 238 (multiples of 4 bars, the clip out point moved in, nothing taken out of the file); how far a clip out point can be dragged out again.
- Depends on topic J: the name Studio in every old rule of this topic (one replacement at the merge; "Studio picture" is Harmony's pick); the knob that belongs to a cell (BF262) and both kinds of knob (BF270) inside Record Over (item 257, E3-7, E-32). If J's research gives pads a hold setting, a show recording keeps presses and releases like any button (old item D17); nothing changes here.
- Topic F: its ruling in progress keeps a picture in the Snapshot and lets old item R187 (d) of this topic stand for it (rule-F.md, its F15), so nothing is amended here; it also has Snapshot do nothing while Studio is on screen (its F3-13), which agrees with item 257 and E3-7. F may still want one mechanism for a Snapshot and for the recorded show file (old item R173, old assumption E-34).
- If he answers E3-1 with c (an output screen's own size), items 231 and 262, old item G4 and E3-3 are re-read together: the film is then no longer small, and the Video box is its twin. If he answers E-23 with b, points (2) and (3) of "Harmony's own" in item 231 fall away without harm.
- For the build session, not for him: the test on his Mac with a heavy show still gates "on by default" (item 231, point 5). He wrote "It defaults to running" without the page's condition; if the test fails he is told before anything is decided.
- For Harmony, on the workflow and not on the topic: the lint reads the whole file, but the merge takes a ruling's blocks only between "## REPLACEMENT BLOCKS" and the next heading; a ruling that appends blocks after "## FOR THE PAGE RULING" passes the lint and loses every block at the merge. Read-only check at 2026-10-09 19:32:21 EDT: the eight ruling files then present (A to H) kept all their blocks inside the section; compare each topic's "replaced / new / dropped" in the merge report with its lint line before the spec is laid over.

