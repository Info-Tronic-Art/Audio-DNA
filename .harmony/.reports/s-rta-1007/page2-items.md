@@PAGE-ANSWER codec
ASKED: "Is there any reason for us to build our own codec that is optimized for our system" [L3]
TITLE: A codec of our own: no
TEXT: No. I would not build one, and I could not promise to make one reliable: it would play only in this app and need its own tools and long testing, and no other program could play or check its files. What you want from DXV exists in a free format with the same purpose, HAP; as far as I can see without running it, the app already plays HAP files. Such formats store each picture whole, so a jump, backwards play or a random beat should need no catching up. Not measured yet: how long a jump waits on your ordinary clips, and how much bigger a copy made in HAP is (my estimate: much bigger). So I measure first, on two of your clips, and then we decide.
ITEM: 239
@@END

@@PAGE-ANSWER lowres
ASKED: "How much comp resource would this take up?" [L6]
TITLE: The low-resolution show recording: light, but not yet measured
TEXT: It should be light, and I would build it; but nobody has measured it, so every number is an estimate. A film a quarter as wide and as high as the show (480 by 270 for full HD), 30 pictures a second: about 1 to 5 percent of the processor and 25 to 150 MB for each 10 minutes. Not known: whether fetching the small picture makes the live picture stutter; a test on your Mac with a heavy show comes before any promise. Each 10-minute piece is also saved so that a crash costs you only its last seconds.
ITEM: 231, 262
@@END

@@PAGE-ANSWER 189
ASKED: "keep it on automatic mode, or will that correction necessitate going back to manual mode?" and "Same question if I correct, where the one is." [L32]
TITLE: Correcting the listening without leaving automatic mode
TEXT: Probably yes, but not proven. The plan: you correct the tempo or the 1, and the app stays in automatic mode; nothing switches to manual by itself. Whether it holds on your music is not measured: I can say only after a test on the DJ tracks you are bringing. If it fails, a correction needs manual mode, and I tell you before anything is built. The tempo: you set the number; the app keeps listening, but only close around your number, so it follows the music's slow changes and keeps the beats on the music. The 1: when you press Resync, that beat is the 1, and the app counts on from it.
ITEM: 217, 218
@@END

@@PAGE-ANSWER R129-4
ASKED: "please explain in more detail about loading a preset onto an effect with an action playing" [L66]
TITLE: A preset loaded while an action plays
TEXT: My recommendation: loading a preset never stops the action, and the preset is never lost. Example: an effect has sliders A, B and C, and an action is moving A. You load a preset. B and C take the preset's values at once. A keeps moving with the action. The preset's value for A waits underneath: when the action goes off, or has played once, A glides to the preset's value, not to the value from before the action. The other way: the preset's value for A is thrown away.
ITEM: 228
@@END

@@PAGE-ANSWER R184
ASKED: "To play a whole recording as a performance you make one action from its start to its end." [L87]
TITLE: That sentence was mine, and I take it back
TEXT: I take it back. It meant that a whole recorded night could be saved as one long action. We do not need that: you watch a whole recording in Review, with its sound, and the outputs show what Review shows. Actions are only pieces you cut from a recording.
ITEM: none
@@END

@@PAGE-ANSWER R188-snapshot
ASKED: "Do we need snapshot?" [L94]
TITLE: Snapshot: not needed for saving
TEXT: No, not for saving anything: the show holds everything, the layout too, and the app opens your last show. I would keep Snapshot only as one small command that saves a still picture of the output as an image file, for a flyer, a post, or to remember a look. It is never part of a show and changes nothing in one.
ITEM: 236
@@END

@@PAGE-ANSWER R221-b
ASKED: "Is this a good idea, help me think through this logically" [L107]
TITLE: Effects that read the beat by themselves
TEXT: Yes, it is a good idea. The logic: such an effect (a strobe, a pulse) follows the beat clock, not the signals. So what holds the clock holds the effect: on pause it stands where it was and goes on in step when you press play; on stop it waits and starts on your new 1. That is your own rule: pause pauses everything the beat controls. The Master Signal only turns signals down, so at 0 the strobe keeps its beat; to stop it, switch the effect off. One thing to know: a strobe paused in its dark moment stays dark until you press play.
ITEM: 243
@@END

@@PAGE-ANSWER review-name
ASKED: "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" [L114]
TITLE: A name for the review screen
TEXT: Not clearly better. Review is a good name, and I keep writing it until you say otherwise. One risk: when you speak or dictate, Review and Preview differ by one sound, and the app has both. If that bothers you, my pick is Studio: musicians pair Live and Studio, and the studio is where you listen back to a recording and make things from it. The other candidate is Replay, your own earlier word ("the replay window"): clear, but close to play on the tempo bar.
ITEM: 245
@@END

@@PAGE-ANSWER 207-hold
ASKED: "make an argument for why we should have a key/pad hold setting" [L120]
TITLE: Why a pad could have a hold setting
TEXT: The argument: hold turns a pad into an instrument. With a band you play by hand: the flash, the logo or the effect is there exactly as long as you hold the pad, through a drum fill or a held chord, and gone when you let go. A DJ holds a pad through the build-up and lets go on the drop. With press only, each of these needs a second press that must also be on time; miss it and the flash stays on. It would be one switch per pad, set to press until you change it. One limit: on a BPM mode clip a short hold can end before the 1, and then nothing shows. I would add it; but your answer stands unless you say otherwise.
ITEM: 246
@@END

@@PAGE-ITEM 217
TOPIC: A
KIND: ASK
TEXT: I assume a tempo you correct by hand in automatic mode stays close to your number, following only small drifts, until the music clearly changes tempo (a new track that is not beat-matched); then the app follows the music again.
B: It stays exactly at your number until you change it yourself, whatever the music does.
C: none
FROM: A-18, X-4, 189, C14
@@END

@@PAGE-ITEM 218
TOPIC: A
KIND: ASK
TEXT: I assume in automatic mode the 1 is always yours (the press that starts the beat, or your Resync): the app never moves it by itself, so after a new track it may sit wrong until you press Resync.
B: The app also places the 1 by itself when it is sure, until your first Resync.
C: none
FROM: A-18, X-4, 189, R164
@@END

@@PAGE-ITEM 219
TOPIC: A
KIND: ASK
TEXT: I assume that when you start the beat again after a pause or a stop, the nudge is history: the nudge number shown in automatic mode goes back to 0.
B: The nudge number stays through pause, stop and play and keeps shifting the beat; only Resync sets it to 0.
C: none
FROM: A-8, R174, R177
@@END

@@PAGE-ITEM 220
TOPIC: B
KIND: ASK
TEXT: I assume a clip you preview by its name waits for the next 1 before it starts, in BPM mode or not, so that its actions play in time with the music.
B: Only a BPM-mode clip waits; any other clip starts at once, and its actions join on the next 1.
C: none
FROM: B-1, X-29, R151, R179
@@END

@@PAGE-ITEM 221
TOPIC: B
KIND: ASK
TEXT: I assume that while the tempo is paused or stopped, a clip you preview by its name still plays in the preview, with its actions, at the tempo's BPM, from your click. Previewing never starts the tempo.
B: It waits on its first frame until the tempo runs again and reaches the 1.
C: none
FROM: B-2, X-24, U18
@@END

@@PAGE-ITEM 222
TOPIC: B
KIND: ASK
TEXT: I assume master cue only shows in the preview what global does on the output at that moment: the global effects, moved by any global action that runs. It is not a way to try a global effect or action unseen.
B: Master cue also lets you switch a global effect or action on for the preview only, to try it first.
C: Master cue shows the whole output in the preview, as on a DJ mixer.
FROM: B-4, 177
@@END

@@PAGE-ITEM 223
TOPIC: C
KIND: ASK
TEXT: I assume that when a show and the computer hold two different versions of one preset (you changed it on one computer and not on the other), the newer one wins, without asking.
B: A small window asks which one to keep.
C: The show's version always wins, as with the keyboard and MIDI mapping.
FROM: C-3
@@END

@@PAGE-ITEM 224
TOPIC: D
KIND: ASK
TEXT: I assume ONE slider, Global glide (instant to 4 seconds), sets how long every smooth change of an action takes: when it starts, goes off, has played once, takes over from another action, or after a Resync.
B: Two sliders: one for how actions start, one for how values go back.
C: none
FROM: D-1, X-27, 183, 213, R147
@@END

@@PAGE-ITEM 225
TOPIC: D
KIND: ASK
TEXT: I assume a layer gets ONE new switch, like Ignore Column Trigger: while it is on, global actions leave that layer alone. The layer's own actions and its clips' actions keep playing.
B: One switch, and it makes the layer ignore every action, its own and its clips' too.
C: Two switches: one against global actions, one against all actions.
FROM: D-2, X-3, G5, 179
@@END

@@PAGE-ITEM 226
TOPIC: D
KIND: ASK
TEXT: I assume the tempo stop also switches every layer and global action off, as Stop actions does; you switch on again the ones you want.
B: Their buttons stay on: the actions stand still and start again from their beginning when the beat next runs.
C: none
FROM: D-3, 180, R224
@@END

@@PAGE-ITEM 227
TOPIC: D
KIND: ASK
TEXT: I assume a play-once action on a clip plays once every time that clip is triggered. On a layer or on global it plays once, and its button then goes off by itself.
B: On a clip too, its button goes off after one play; you switch it on again yourself.
C: Its button always stays on; it plays again only after you switch it off and on.
FROM: D-4
@@END

@@PAGE-ITEM 228
TOPIC: D
KIND: ASK
TEXT: I assume that when you load a preset onto an effect while an action is moving one of its sliders, that slider keeps moving with the action, and when the action goes off it glides to the preset's value.
B: It glides back to the value it had before the action; the preset's value for it is lost.
C: none
FROM: D-5, R129
@@END

@@PAGE-ITEM 229
TOPIC: E
KIND: ASK
TEXT: I assume Record Over is a button in Review: the show recording plays in real time with its sound, and what you move on the MIDI controller is recorded over it.
B: Record Over runs in the live window, through your live layers.
C: none
FROM: E-2, X-8, 195
@@END

@@PAGE-ITEM 230
TOPIC: E
KIND: ASK
TEXT: I assume an action you save in Review goes into the show the recording was made in, onto the same clip, wherever that clip is by now. If the clip is gone, the app tells you and nothing is saved.
B: If the clip is gone, the action is kept and you paste it onto a clip you choose.
C: It goes into whatever show is open in the live window.
FROM: E-11
@@END

@@PAGE-ITEM 231
TOPIC: E
KIND: ASK
TEXT: I assume, once a test shows it is light, the low-resolution show recording runs with every record show, with sound, unless you untick it.
B: It runs only when you tick it.
C: It has no sound.
FROM: E-21, G4
@@END

@@PAGE-ITEM 232
TOPIC: E
KIND: ASK
TEXT: I assume record to clip runs on to the end of the bar in which you press stop and keeps every bar it recorded (5 bars stay 5); you trim it yourself.
B: It runs on until a group of 4 bars is full.
C: It stops at the end of the bar, but the clip loops only whole groups of 4 bars (5 recorded bars loop as 4); the rest stays in the file.
FROM: E-6
@@END

@@PAGE-ITEM 233
TOPIC: F
KIND: ASK
TEXT: I assume you select an empty cell by clicking where a clip's name would be, at the bottom of the cell, and can then paste into it. A click anywhere else on it triggers it: the layer goes empty.
B: A plain click on an empty cell both selects it and triggers it (the layer goes empty); you then paste.
C: none
FROM: F-1, G2
@@END

@@PAGE-ITEM 234
TOPIC: F
KIND: ASK
TEXT: I assume pasting over, cutting or deleting a clip that is playing does not change the picture: it plays on until you trigger something else on that layer.
B: The layer switches at once to the pasted clip, and goes empty at once on a cut or a delete.
C: none
FROM: F-3, G2, G3
@@END

@@PAGE-ITEM 235
TOPIC: F
KIND: ASK
TEXT: I assume "delete all the old show files" means every show saved so far and the app's safety copies of them. I list them first; after your yes they go to the Trash. Saved decks, recordings, presets and settings stay.
B: The old saved decks and the old recordings go too.
C: Nothing is deleted; the new app just does not open old shows.
FROM: F-13, D-7, R143, D13
@@END

@@PAGE-ITEM 236
TOPIC: F
KIND: ASK
TEXT: I assume Snapshot stays as one small command that saves a still picture of the output to a file, and gets no more work.
B: Snapshot leaves the app.
C: none
FROM: F-16, R188
@@END

@@PAGE-ITEM 237
TOPIC: G
KIND: ASK
TEXT: I assume opening a show never switches off an output that is already on; it only adds the ones the show remembers.
B: Opening a show sets the outputs exactly as it was saved: every other output goes off.
C: none
FROM: G-1, R191
@@END

@@PAGE-ITEM 238
TOPIC: H
KIND: ASK
TEXT: I assume a new clip in BPM mode gets a number of beats (4, 8, 16, 32 ...) that plays the whole clip at about its normal speed, and its end is never cut.
B: Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed.
C: none
FROM: H-5, R218, C13
@@END

@@PAGE-ITEM 239
TOPIC: H
KIND: ASK
TEXT: I assume we build no codec. First I measure, on two of your clips, how long a clip waits when it jumps; only if you would notice the wait do I add a command that makes HAP copies. Ordinary files keep playing.
B: Plan the command that makes HAP copies now, without timing first.
C: No such command: you make HAP copies yourself with tools you own.
FROM: H-14, G1
@@END

@@PAGE-ITEM 240
TOPIC: I
KIND: ASK
TEXT: I assume every layer works the same: it is blended in by its blend mode and its transparency slider, the bottom one too. There are no mask layers.
B: Mask layers stay; only the keying list and its slider go.
C: none
FROM: I-1, 204
@@END

@@PAGE-ITEM 241
TOPIC: I
KIND: ASK
TEXT: I assume an envelope you draw always runs on the beat, never along the clip's playhead.
B: An envelope can also follow the playhead: its drawn shape is stretched over the clip from start to end.
C: none
FROM: I-6, 205
@@END

@@PAGE-ITEM 242
TOPIC: I
KIND: ASK
TEXT: I assume a One Shot on a slider runs once from the next 1 after its clip is triggered; on a layer's slider after every clip triggered on that layer; on a global slider when you switch it on.
B: It starts the moment its clip starts, off the beat too.
C: It runs only when you press a start button on that slider.
FROM: I-9, R221
@@END

@@PAGE-ITEM 243
TOPIC: I
KIND: ASK
TEXT: I assume effects that read the beat by themselves (a strobe, a pulse) hold still while the beat is paused or stopped.
B: They keep pulsing through a pause.
C: none
FROM: I-16, R221, R214
@@END

@@PAGE-ITEM 244
TOPIC: J
KIND: ASK
TEXT: I assume a MIDI knob you map to a clip's slider stays with that clip, wherever the clip goes; a copy of the clip has no knob. A MIDI knob on a layer's or a global slider stays there.
B: The knob belongs to the cell: it moves that slider of whatever clip sits there in the deck on screen.
C: The knob moves that slider of whatever clip is playing on that layer.
FROM: J-8, 206
@@END

@@PAGE-ITEM 245
TOPIC: J
KIND: ASK
TEXT: I assume the screen where you watch a recording and make actions keeps the name Review, as you wrote, until you pick another.
B: It is called Studio.
C: It is called Replay.
FROM: R197, R224 (the answer review-name)
@@END

@@PAGE-ITEM 246
TOPIC: J
KIND: ASK
TEXT: I assume your answer stands: no hold setting. A pad or a key only presses.
B: Every pad and key gets one switch, press or hold, set to press until you change it, so nothing changes for a pad you leave alone.
C: none
FROM: 207 (the answer 207-hold), D34
@@END

@@PAGE-ITEM 247
TOPIC: K
KIND: ASK
TEXT: I assume nothing of MilkDrop changes in the coming build: with no clip playing, a preset you click in the MilkDrop tab still shows on the output.
B: With no clip playing the output stays black: a clicked preset changes the preset but does not show; MilkDrop reaches the output only through a clip that plays.
C: none
FROM: K-3, R228, R202
@@END

@@PAGE-ITEM 248
TOPIC: A
KIND: LINE
TEXT: I assume a click on an empty cell never starts the beat: in a pause the layer's BPM-mode clip stays until the 1 after play.
B: The click sets the beat going, as a click on a clip does.
C: In a pause the layer goes empty at once.
FROM: A-6
@@END

@@PAGE-ITEM 249
TOPIC: A
KIND: LINE
TEXT: I assume a BPM-mode clip triggered just after the 1 (within a tenth of a beat) still starts at once, in step with the music.
B: It waits a whole bar for the next 1.
C: none
FROM: A-15, D4
@@END

@@PAGE-ITEM 250
TOPIC: A
KIND: LINE
TEXT: I assume tapping changes only the tempo number, from the second tap on; it never shifts the beat or the 1.
B: Each tap also pulls the beat onto your tap, but never moves the 1.
C: none
FROM: A-2, X-10, 190, R206
@@END

@@PAGE-ITEM 251
TOPIC: B
KIND: LINE
TEXT: I assume the transparency slider beside a cue button is a try-out for the preview only: it starts at full and never moves the layer's own slider.
B: It starts at the layer's own transparency.
C: none
FROM: B-3, R141
@@END

@@PAGE-ITEM 252
TOPIC: B
KIND: LINE
TEXT: I assume the output always comes first: the preview may be smaller, less smooth and, in the first build, show trails (Echo, Freeze, Feedback) and MilkDrop a little differently.
B: none
C: none
FROM: B-10, B-11, X-11, D8, D9
@@END

@@PAGE-ITEM 253
TOPIC: C
KIND: LINE
TEXT: I assume only files and deck clips are previewed: a double-click on an effect shows its properties but no picture, and a source is not previewed.
B: Effects and sources are previewed by a double-click too, as in Resolume.
C: none
FROM: C-9, B-9
@@END

@@PAGE-ITEM 254
TOPIC: D
KIND: LINE
TEXT: I assume a clip that an action triggered stays on its layer when the action ends; only sliders, buttons and menus go back.
B: The layer goes back to the clip that played before the action.
C: none
FROM: D-26
@@END

@@PAGE-ITEM 255
TOPIC: D
KIND: LINE
TEXT: I assume a control's ignore actions toggle, switched on while an action moves it, sends the control gliding back to where it was before the action.
B: The control stays where the action has it at that moment, so you can catch a value.
C: none
FROM: D-11, R131
@@END

@@PAGE-ITEM 256
TOPIC: E
KIND: LINE
TEXT: I assume a clip made with record to clip is the output's full picture, global effects included: black where no layer shows, hiding layers under it.
B: Its empty parts stay see-through, so layers under it show through; likely price, not measured: bigger files and a heavier recording.
C: none
FROM: E-9, E-8, D22
@@END

@@PAGE-ITEM 257
TOPIC: E
KIND: LINE
TEXT: I assume Record Over takes everything in the keyboard and MIDI mapping, clip triggers too; what you move with the mouse is not recorded over.
B: Only knobs and faders record over.
C: none
FROM: E-3
@@END

@@PAGE-ITEM 258
TOPIC: E
KIND: LINE
TEXT: I assume a press on record to clip while the tempo is paused or stopped waits: the recording starts on the first 1 once the tempo runs.
B: The button does nothing until the tempo plays.
C: none
FROM: E-5
@@END

@@PAGE-ITEM 259
TOPIC: E
KIND: LINE
TEXT: I assume record show starts and stops at your press and runs on through a tempo pause or stop; only record to clip waits for the 1.
B: Record show also starts on the next 1 and ends on a bar line.
C: none
FROM: E-10
@@END

@@PAGE-ITEM 260
TOPIC: E
KIND: LINE
TEXT: I assume anything you add or take out while recording (an effect, a clip, a layer) is recorded at that moment; what you took out then stays flat at default.
B: What you add while recording is not recorded: it has no tracks and is not there in Review.
C: none
FROM: E-17, E-35
@@END

@@PAGE-ITEM 261
TOPIC: E
KIND: LINE
TEXT: I assume record show, record to clip and the low-resolution show recording can run together; if the computer cannot keep up, a recording loses pictures, never the output.
B: A second recording is refused while one runs.
C: none
FROM: E-26, D20
@@END

@@PAGE-ITEM 262
TOPIC: E
KIND: LINE
TEXT: I assume the low-resolution show recording is a quarter as wide and as high as the show (480 by 270 for full HD), in 10-minute pieces.
B: An eighth as wide and as high (240 by 135), or 20-minute pieces: say which.
C: none
FROM: E-21, G4
@@END

@@PAGE-ITEM 263
TOPIC: G
KIND: LINE
TEXT: I assume a show remembers Syphon like a screen: a show saved with Syphon on switches it on again when it is opened.
B: Syphon stays off until you tick it.
C: none
FROM: G-2, X-7
@@END

@@PAGE-ITEM 264
TOPIC: G
KIND: LINE
TEXT: I assume a show brings back only the screens it remembers: with a different projector nothing comes on by itself, and you tick it once.
B: The picture goes to whatever output screen is plugged in.
C: none
FROM: G-5, D28
@@END

@@PAGE-ITEM 265
TOPIC: G
KIND: LINE
TEXT: I assume All Outputs Off is for the moment only: a show you open afterwards brings its outputs on again.
B: After All Outputs Off nothing comes on until you switch an output on yourself.
C: none
FROM: G-15
@@END

@@PAGE-ITEM 266
TOPIC: H
KIND: LINE
TEXT: I assume triggering the clip that is already playing always restarts it, even when it is set to continue where it left off.
B: Set to continue, it plays on undisturbed when you trigger it again.
C: none
FROM: H-3
@@END

@@PAGE-ITEM 267
TOPIC: H
KIND: LINE
TEXT: I assume a video you bring in starts in Timeline mode; you switch the clips you want on the beat to BPM mode yourself.
B: Every new video starts in BPM mode.
C: One setting says which mode new clips start in.
FROM: H-15
@@END

@@PAGE-ITEM 268
TOPIC: I
KIND: LINE
TEXT: I assume Falloff means: the value jumps up with the signal at once and comes down slowly, from instant to 2 seconds.
B: It smooths the way up and the way down alike.
C: none
FROM: I-13
@@END

@@PAGE-ITEM 269
TOPIC: J
KIND: LINE
TEXT: I assume a mapping file holds the whole keyboard and MIDI mapping of the open show; importing one replaces that show's mapping, after asking.
B: An import adds to the mapping and overwrites only the keys that clash.
C: none
FROM: J-2
@@END

@@PAGE-ITEM 270
TOPIC: J
KIND: LINE
TEXT: I assume MIDI knobs must send their position: an endless knob that only sends steps will not work. Which MIDI controller do you use?
B: Knobs that send steps must work too.
C: none
FROM: J-4, 207, D33
@@END

@@PAGE-ITEM 271
TOPIC: J
KIND: LINE
TEXT: I assume switching an output screen on or off cannot be put on a key or a pad, so that no stray hit blanks a projector.
B: It can go on a key or a pad like any button.
C: none
FROM: J-18
@@END

@@PAGE-ITEM 272
TOPIC: K
KIND: LINE
TEXT: I assume the computer remembers which audio input you picked, for every show; a show does not hold it.
B: The show remembers it too, as it remembers its outputs.
C: none
FROM: K-10
@@END

@@PAGE-ITEM 273
TOPIC: K
KIND: LINE
TEXT: I assume the tempo bar's stop does not stop an audio file the app plays: the music plays on.
B: The stop also stops the audio file.
C: none
FROM: K-11
@@END

@@TRIAGE A-1
TO: SETTLED
WHY: L13 and L17: a click on the clip starts the held clock, as play does; any clip.
@@END

@@TRIAGE A-2
TO: 250
WHY: His look L18 reads both ways and the default for 190 rested on a guess: shown as one line.
@@END

@@TRIAGE A-3
TO: SETTLED
WHY: L13 with his earlier answer to 111: clips not BPM based play on and are unaffected.
@@END

@@TRIAGE A-4
TO: SETTLED
WHY: L11: what is not in BPM mode plays instantly when triggered.
@@END

@@TRIAGE A-5
TO: INTERNAL
WHY: Follows "the resolume way" (L11): the X is immediate there; one button, cheap to turn.
@@END

@@TRIAGE A-6
TO: 248
WHY: L11 says "same as a new clip", L12 names only play or a clip: shown as one line.
@@END

@@TRIAGE A-7
TO: SETTLED
WHY: L12 ("unless resync is clicked") with his earlier 128: Resync is the 1 and starts the beat.
@@END

@@TRIAGE A-8
TO: 219
WHY: L13 "that nudge is history" against his earlier 129 b; he wrote "If you don’t understand ask me."
@@END

@@TRIAGE A-9
TO: INTERNAL
WHY: Bookkeeping of a number; what he sees is settled by L14.
@@END

@@TRIAGE A-10
TO: SETTLED
WHY: L14: the open range for the listening clock too, narrowed later if it distorts.
@@END

@@TRIAGE A-11
TO: INTERNAL
WHY: Technical edge of the range.
@@END

@@TRIAGE A-12
TO: INTERNAL
WHY: Belongs to the Link plan, shown to him when Link is planned (item 282).
@@END

@@TRIAGE A-13
TO: SETTLED
WHY: L12: "user’s click on play or any clip is the new 1"; L13: the pause holds all the BPM controls, the autopilot too.
@@END

@@TRIAGE A-14
TO: INTERNAL
WHY: Technical: a tap count and a gap, tuned at the build.
@@END

@@TRIAGE A-15
TO: 249
WHY: Decided without asking (L130); seen on stage if wrong; most likely waved through.
@@END

@@TRIAGE A-16
TO: INTERNAL
WHY: Presses with no effect; the one other way is the paused start his L30 turned down.
@@END

@@TRIAGE A-18
TO: 217
WHY: No words settle how long a correction holds or who places the 1; asked as 217 and 218.
@@END

@@TRIAGE A-19
TO: SETTLED
WHY: L11 applied through L9; L19 shows a column trigger empties a layer.
@@END

@@TRIAGE A-20
TO: INTERNAL
WHY: A starting mode; one click either way.
@@END

@@TRIAGE A-21
TO: SETTLED
WHY: His earlier 147 a keeps held and trailing pictures after a stop; L72 is about actions and clips.
@@END

@@TRIAGE A-22
TO: INTERNAL
WHY: A word on the tempo bar: a display, laid out by Harmony (L8).
@@END

@@TRIAGE B-1
TO: 220
WHY: L35 and L36 pull apart; the default is L36 to the letter (triggered on the 1); asked.
@@END

@@TRIAGE B-2
TO: 221
WHY: No words cover a preview while no 1 is coming; seen before every show; needs its own beat.
@@END

@@TRIAGE B-3
TO: 251
WHY: L34 "what the transparency would be" reads as a try-out; its start value is shown as a line.
@@END

@@TRIAGE B-4
TO: 222
WHY: L38 does not say what the actions half does; the other way is a far bigger build.
@@END

@@TRIAGE B-5
TO: INTERNAL
WHY: As in Resolume, a clip shown alone has its own effects only; small, cheap to turn.
@@END

@@TRIAGE B-7
TO: INTERNAL
WHY: Extends a rule he read and let stand (cue controls are not saved) to the new cue controls.
@@END

@@TRIAGE B-8
TO: INTERNAL
WHY: On the page and left standing; a restart would disturb the output.
@@END

@@TRIAGE B-9
TO: MERGED 253
WHY: The same point as C-9: what a double-click previews.
@@END

@@TRIAGE B-10
TO: 252
WHY: Decided without asking (L130); he would see a smaller, less smooth preview.
@@END

@@TRIAGE B-11
TO: MERGED 252
WHY: One line on what the preview may show differently; second copies of a simulation are not built.
@@END

@@TRIAGE B-12
TO: SETTLED
WHY: L35 and L88 keep the preview off screens and recordings; Syphon follows as an output.
@@END

@@TRIAGE B-13
TO: SETTLED
WHY: L51 carried to the cued mix by L9; the edge of a cell previewed alone is cheap.
@@END

@@TRIAGE B-14
TO: INTERNAL
WHY: Nothing of it reaches the output; small.
@@END

@@TRIAGE B-17
TO: INTERNAL
WHY: L35 announces more configurations and describes none; nothing is built from it now.
@@END

@@TRIAGE B-18
TO: INTERNAL
WHY: Follows the rule that an action plays only while its button is on.
@@END

@@TRIAGE B-19
TO: INTERNAL
WHY: The page said black and he let it stand; a look (L8).
@@END

@@TRIAGE B-20
TO: INTERNAL
WHY: A detail of drawing the preview.
@@END

@@TRIAGE B-21
TO: INTERNAL
WHY: How a previewed file is fitted is a look (L8).
@@END

@@TRIAGE C-2
TO: SETTLED
WHY: L48: presets "should travel with the show file"; L50: "kept with the app and the show file".
@@END

@@TRIAGE C-3
TO: 223
WHY: No words say which version counts; a wrong guess replaces his preset without a word.
@@END

@@TRIAGE C-4
TO: INTERNAL
WHY: Deleted stays deleted; anything else would be a fault he would report.
@@END

@@TRIAGE C-5
TO: INTERNAL
WHY: An edge of the header rule of L52; cheap to turn, revisited in the UI redesign.
@@END

@@TRIAGE C-6
TO: INTERNAL
WHY: Not a real choice: without it no other preset could be picked; its place is layout (L8).
@@END

@@TRIAGE C-7
TO: INTERNAL
WHY: An edge of the header rule of L52 with L53; one test, cheap to turn.
@@END

@@TRIAGE C-8
TO: INTERNAL
WHY: By L9 his 192 b (L53) extends to Default, a preset with no signal; one switch to turn.
@@END

@@TRIAGE C-9
TO: 253
WHY: L41 asks for the properties; L35 lists files and clips for the preview; shown as one line.
@@END

@@TRIAGE C-11
TO: INTERNAL
WHY: Follows L106: gain, falloff and one shot / looping sit on the slider, so the effect's preset holds them.
@@END

@@TRIAGE C-12
TO: SETTLED
WHY: L51 "R180 yes": a preset remembers a signal by its name only; a show without that name plugs nothing in.
@@END

@@TRIAGE C-13
TO: INTERNAL
WHY: Follows L111 and L51; nothing new is built for it.
@@END

@@TRIAGE C-14
TO: INTERNAL
WHY: Taken from his own picture of the window.
@@END

@@TRIAGE C-15
TO: INTERNAL
WHY: Technical: the moment of a file write.
@@END

@@TRIAGE C-16
TO: INTERNAL
WHY: Small rules of the build, each easy to turn.
@@END

@@TRIAGE D-1
TO: 224
WHY: L64, L66 and L67 are one slider by his own words; L70's fade control may be a second.
@@END

@@TRIAGE D-2
TO: 225
WHY: L7 and L73: one switch or two, and its reach, differ on stage; topics read it two ways.
@@END

@@TRIAGE D-3
TO: 226
WHY: L72 against the default he took for 180; the default follows L72; asked.
@@END

@@TRIAGE D-4
TO: 227
WHY: L64 gives play once; what its button does after, and when it plays again, is not said.
@@END

@@TRIAGE D-5
TO: 228
WHY: He asked (L66); the choice follows the answer R129-4.
@@END

@@TRIAGE D-6
TO: SETTLED
WHY: L68: "that specific action is muted": the whole action; that it wakes when the part is added is internal.
@@END

@@TRIAGE D-7
TO: MERGED 235
WHY: The same question as F-13: which old files go to the Trash (L69).
@@END

@@TRIAGE D-8
TO: SETTLED
WHY: L73: the bypass he gives a layer only has a job if the pick is one global action.
@@END

@@TRIAGE D-9
TO: SETTLED
WHY: L71 names the reading and changes only the colours; its part on Ignore Column Trigger stands (L1).
@@END

@@TRIAGE D-10
TO: SETTLED
WHY: L74 (182 b: the deck that is shown, other pictures after a deck switch) with L11 and L19.
@@END

@@TRIAGE D-11
TO: 255
WHY: L67 is about an action switched off; the toggle switched on under a running action is not said.
@@END

@@TRIAGE D-13
TO: INTERNAL
WHY: L70 names starts, stops and a Resync; what was recorded plays as recorded.
@@END

@@TRIAGE D-14
TO: INTERNAL
WHY: Follows L70 with the rule that every action starts on a 1; revisited with his answer to 224.
@@END

@@TRIAGE D-15
TO: INTERNAL
WHY: A first state that follows his earlier answer that actions loop.
@@END

@@TRIAGE D-16
TO: SETTLED
WHY: L65: "show a warning"; a warning is not a refusal.
@@END

@@TRIAGE D-17
TO: INTERNAL
WHY: Follows his One Save rule; he sets the slider himself.
@@END

@@TRIAGE D-18
TO: INTERNAL
WHY: His paste rule (L68) one case further; follows his answer on item 255.
@@END

@@TRIAGE D-19
TO: INTERNAL
WHY: Any other behaviour would be a fault he would report.
@@END

@@TRIAGE D-20
TO: INTERNAL
WHY: Technical: stored inside the action, shown nowhere.
@@END

@@TRIAGE D-21
TO: INTERNAL
WHY: Follows the default he took that a copied clip keeps its actions.
@@END

@@TRIAGE D-22
TO: INTERNAL
WHY: Internal logic: otherwise an action could never un-pause its clip.
@@END

@@TRIAGE D-23
TO: INTERNAL
WHY: A detail of the save window in Review (L55): he sees the list of actions before anything is saved.
@@END

@@TRIAGE D-24
TO: INTERNAL
WHY: Technical edge: a column the deck on screen does not have.
@@END

@@TRIAGE D-25
TO: INTERNAL
WHY: Technical: the arithmetic of a glide.
@@END

@@TRIAGE D-26
TO: 254
WHY: L64 speaks of values going back; what becomes of a fired clip is seen on stage.
@@END

@@TRIAGE D-27
TO: INTERNAL
WHY: Follows L70: nothing an action moves may jump.
@@END

@@TRIAGE D-28
TO: INTERNAL
WHY: Internal logic: otherwise an action could switch itself off or lock itself out.
@@END

@@TRIAGE D-29
TO: INTERNAL
WHY: How every clipboard works; both ways to paste an action were on his page.
@@END

@@TRIAGE D-30
TO: INTERNAL
WHY: A look; the action buttons take their place (L8).
@@END

@@TRIAGE D-31
TO: INTERNAL
WHY: The natural result of the two states of L64.
@@END

@@TRIAGE E-1
TO: SETTLED
WHY: L91 (186 b): the letter without a switch; an output can be switched off as always.
@@END

@@TRIAGE E-2
TO: 229
WHY: No words place Record Over or a recording's playback; it shapes two screens.
@@END

@@TRIAGE E-3
TO: 257
WHY: L92 names the MIDI controller and sets the mouse apart; pads and keys are not named.
@@END

@@TRIAGE E-4
TO: INTERNAL
WHY: L84 and L92 do not say how many copies; a small choice about the list of recordings, changed later.
@@END

@@TRIAGE E-5
TO: 258
WHY: L88 does not say what the press does while paused or stopped; follows L12; shown as a line.
@@END

@@TRIAGE E-6
TO: 232
WHY: "even amount" (L88) has three readings and sets how long every recorded clip loops: asked.
@@END

@@TRIAGE E-7
TO: INTERNAL
WHY: Follows L88 (nothing paused is recorded) and L72; neither way loses what was recorded.
@@END

@@TRIAGE E-8
TO: MERGED 256
WHY: L88 and his earlier "look exactly like the output" carry it; item 230 shows it ("global effects included").
@@END

@@TRIAGE E-9
TO: 256
WHY: His earlier yes was for a one-layer recording, which L88 removes; it decides the file type.
@@END

@@TRIAGE E-10
TO: 259
WHY: L88 was said of record to clip; the reading that cannot lose any of the night.
@@END

@@TRIAGE E-11
TO: 230
WHY: No words say where an action saved in Review lands; it is what Review is for.
@@END

@@TRIAGE E-12
TO: INTERNAL
WHY: Technical; his "moved or missing" (L80) expects a file that points at the media.
@@END

@@TRIAGE E-13
TO: INTERNAL
WHY: Not settled: L81's first sentence says no row for what never moves; small; its value is in the show recording.
@@END

@@TRIAGE E-14
TO: SETTLED
WHY: L82 (the grid) and L83 (the Quantize list), each said on its own reading.
@@END

@@TRIAGE E-15
TO: INTERNAL
WHY: Follows his selected area (L89) and a part of the reading he left; a mend can be undone.
@@END

@@TRIAGE E-16
TO: INTERNAL
WHY: A deck switch never changes the picture; whether its mark is drawn is a look (L8).
@@END

@@TRIAGE E-17
TO: 260
WHY: L89 "a recording will be removed" is read as the thing removed; his to strike.
@@END

@@TRIAGE E-18
TO: INTERNAL
WHY: Not settled by L89; cutting stays allowed, harmless if wrong; a refusal is cheap to add; seen only in Review.
@@END

@@TRIAGE E-19
TO: INTERNAL
WHY: A setting's first state; L86 says scrubbed sound will not sound good.
@@END

@@TRIAGE E-21
TO: 231
WHY: He asked the cost in order to decide (L6); whether, how big and how long is his.
@@END

@@TRIAGE E-23
TO: INTERNAL
WHY: Technical: where the pieces are kept and how they are written.
@@END

@@TRIAGE E-24
TO: INTERNAL
WHY: Technical: how a signal's moves are kept in a recording.
@@END

@@TRIAGE E-26
TO: 261
WHY: Decided without asking (L130); no words rank the recordings against the output.
@@END

@@TRIAGE E-27
TO: INTERNAL
WHY: The app picks the file types itself, by his earlier words; the answer on the codec carries the rest.
@@END

@@TRIAGE E-28
TO: INTERNAL
WHY: A technical limit of a fast scrub.
@@END

@@TRIAGE E-30
TO: INTERNAL
WHY: A technical edge that follows the answer to item 260; a recording that is cut short is always kept.
@@END

@@TRIAGE E-31
TO: INTERNAL
WHY: A look, settled in the UI redesign (L8).
@@END

@@TRIAGE E-32
TO: INTERNAL
WHY: One number with the mapping rule he read and left: a quarter of a second, tuned at the build.
@@END

@@TRIAGE E-33
TO: INTERNAL
WHY: Technical: the Mac's own sound output.
@@END

@@TRIAGE E-34
TO: INTERNAL
WHY: Technical; only this reading lets Review start as the night started (L80).
@@END

@@TRIAGE E-35
TO: MERGED 260
WHY: Adding and taking out while recording are one line; "not recorded" is its way b.
@@END

@@TRIAGE E-36
TO: INTERNAL
WHY: A starting position he moves anyway.
@@END

@@TRIAGE E-37
TO: SETTLED
WHY: L89 names the reading and leaves this part: it stops on its last moment and stays (L1).
@@END

@@TRIAGE E-38
TO: INTERNAL
WHY: Technical; nothing is promised before it is measured.
@@END

@@TRIAGE F-1
TO: 233
WHY: L4 wants an empty cell selected, L11 makes a click on it trigger: one click cannot do both.
@@END

@@TRIAGE F-2
TO: INTERNAL
WHY: The plain meaning of copy, carried by defaults he took (L1).
@@END

@@TRIAGE F-3
TO: 234
WHY: No words cover an edit to a playing cell; the audience would see it.
@@END

@@TRIAGE F-5
TO: INTERNAL
WHY: The usual way of a grid; small to change.
@@END

@@TRIAGE F-6
TO: INTERNAL
WHY: Technical; a pasted clip follows the by-name rule he accepted (L51): a missing signal leaves the value.
@@END

@@TRIAGE F-8
TO: SETTLED
WHY: L4 ("sell with something else in it") and L5 read together: the paste replaces.
@@END

@@TRIAGE F-9
TO: INTERNAL
WHY: A mouse gesture for the UI redesign (L8); copy and paste reach every deck.
@@END

@@TRIAGE F-10
TO: INTERNAL
WHY: Where one small block is stored; either reading of "comp" keeps the layout in the show (L94).
@@END

@@TRIAGE F-11
TO: INTERNAL
WHY: Edges of a clear instruction (L94); the silent fallback follows his rule on messages.
@@END

@@TRIAGE F-12
TO: SETTLED
WHY: L94 and L96, as G-3.
@@END

@@TRIAGE F-13
TO: 235
WHY: L69 names no files and no moment; a deletion of his own files is asked.
@@END

@@TRIAGE F-14
TO: INTERNAL
WHY: Technical: the file's version and the checked save.
@@END

@@TRIAGE F-15
TO: INTERNAL
WHY: His own rule for deck files, carried to the show's file.
@@END

@@TRIAGE F-16
TO: 236
WHY: He asked (L94); the choice follows the answer R188-snapshot.
@@END

@@TRIAGE F-17
TO: INTERNAL
WHY: The offer is the default he took (L1); the order and the number are technical.
@@END

@@TRIAGE F-18
TO: INTERNAL
WHY: Follows his One Save rule: what the message adds is saved with the show.
@@END

@@TRIAGE F-19
TO: INTERNAL
WHY: A name on screen: it goes into the list of names he asked for (L114).
@@END

@@TRIAGE F-20
TO: INTERNAL
WHY: Where layouts are stored; how configurations are chosen is for the UI redesign (L8).
@@END

@@TRIAGE F-21
TO: INTERNAL
WHY: Technical: the usual way of an Option-drag on a Mac.
@@END

@@TRIAGE F-22
TO: INTERNAL
WHY: Technical; it follows a reading he let stand.
@@END

@@TRIAGE F-23
TO: INTERNAL
WHY: How Harmony's own test starts behave.
@@END

@@TRIAGE G-1
TO: 237
WHY: L96 does not say what opening a show does to an output that is on; the audience sees it.
@@END

@@TRIAGE G-2
TO: 263
WHY: L96 with L9 against the default he took for 200; one tick either way; his to strike.
@@END

@@TRIAGE G-3
TO: SETTLED
WHY: L94: the app opens the very last show; L96: the show remembers the outputs connected.
@@END

@@TRIAGE G-4
TO: INTERNAL
WHY: L96 names "the outputs connected"; the main display is not one, and its output would cover the app.
@@END

@@TRIAGE G-5
TO: 264
WHY: L96 covers the same outputs only; another screen in their place is not said.
@@END

@@TRIAGE G-6
TO: INTERNAL
WHY: Follows the purpose of L96: a save at home must not forget the projector.
@@END

@@TRIAGE G-7
TO: INTERNAL
WHY: A mend with no other way to mean it.
@@END

@@TRIAGE G-8
TO: INTERNAL
WHY: A film cannot change its size midway; the flash is a technical fact, a guard is cheap.
@@END

@@TRIAGE G-9
TO: INTERNAL
WHY: How Harmony's own test starts behave.
@@END

@@TRIAGE G-10
TO: INTERNAL
WHY: Technical: how it is built.
@@END

@@TRIAGE G-11
TO: INTERNAL
WHY: A fact of how a screen works, not a choice.
@@END

@@TRIAGE G-12
TO: INTERNAL
WHY: A rare edge; measured on his own screens after the build.
@@END

@@TRIAGE G-13
TO: INTERNAL
WHY: A hitch of a tenth of a second on one screen, only while he moves its Delay.
@@END

@@TRIAGE G-14
TO: SETTLED
WHY: L96: he does not want to connect them again; a click after plugging in would be that.
@@END

@@TRIAGE G-15
TO: 265
WHY: L96 read to the letter; the other way is a guard he may want.
@@END

@@TRIAGE G-16
TO: INTERNAL
WHY: Bookkeeping; the app asks at every quit anyway.
@@END

@@TRIAGE G-17
TO: INTERNAL
WHY: The careful choice at no cost; an output is ticked as always.
@@END

@@TRIAGE H-1
TO: SETTLED
WHY: L101, L20, L21: one marker per beat, and Random lands on a beat marker.
@@END

@@TRIAGE H-2
TO: INTERNAL
WHY: Follows Resolume's manual, his model (L102); he can see it in his Arena.
@@END

@@TRIAGE H-3
TO: 266
WHY: L102 names no edge; his earlier words restart a playing column; seen on stage.
@@END

@@TRIAGE H-4
TO: INTERNAL
WHY: An edge of a menu that stands on "from the start" until he changes it.
@@END

@@TRIAGE H-5
TO: 238
WHY: L99 and L21 against his rule of 4 October; it touches every clip he brings in.
@@END

@@TRIAGE H-6
TO: INTERNAL
WHY: An edge he will rarely meet.
@@END

@@TRIAGE H-7
TO: INTERNAL
WHY: Arithmetic of the Duration row; the out point part stood on his page.
@@END

@@TRIAGE H-9
TO: INTERNAL
WHY: Follows Resolume's manual, his model (L102).
@@END

@@TRIAGE H-10
TO: INTERNAL
WHY: Internal; follows his answer to item 238.
@@END

@@TRIAGE H-11
TO: INTERNAL
WHY: L102 makes Resolume the model; one look in his Arena checks the third entry.
@@END

@@TRIAGE H-12
TO: INTERNAL
WHY: Technical: the size of one corrective jump.
@@END

@@TRIAGE H-13
TO: INTERNAL
WHY: Arithmetic of a row he asked to be as Resolume's (L21).
@@END

@@TRIAGE H-14
TO: 239
WHY: He asked (L3); a new command and bigger files are his to choose.
@@END

@@TRIAGE H-15
TO: 267
WHY: No words say which mode a new clip has; it decides the hand work of a show.
@@END

@@TRIAGE I-1
TO: 240
WHY: L111 names no kind of layer; it decides what a layer can be and what is taken out.
@@END

@@TRIAGE I-2
TO: SETTLED
WHY: L111 ("that layers blend mode") with L110: the list stays, the slider sets its strength.
@@END

@@TRIAGE I-3
TO: INTERNAL
WHY: The order of work between L110 and L111; nothing on stage hangs on it.
@@END

@@TRIAGE I-4
TO: INTERNAL
WHY: Nothing he asked for is taken away (L111 names the layer's keying only).
@@END

@@TRIAGE I-5
TO: INTERNAL
WHY: It only promises a later question (L110).
@@END

@@TRIAGE I-6
TO: 241
WHY: L112 against his earlier "build." for a drawn Timeline curve; a tool of its own.
@@END

@@TRIAGE I-7
TO: SETTLED
WHY: L112: what follows the layer's playhead is called Timeline, so Clip Position takes that name.
@@END

@@TRIAGE I-8
TO: INTERNAL
WHY: Internal detail: the two ends, and a fade between clips.
@@END

@@TRIAGE I-9
TO: 242
WHY: L106 does not say what starts a One Shot on a slider; seen on stage.
@@END

@@TRIAGE I-10
TO: SETTLED
WHY: L106 names the reading and moves only where the controls sit; "holds its last value" stands (L1).
@@END

@@TRIAGE I-11
TO: INTERNAL
WHY: A control is left out only where it has nothing to do.
@@END

@@TRIAGE I-12
TO: INTERNAL
WHY: A number that is tuned at the build.
@@END

@@TRIAGE I-13
TO: 268
WHY: L106 says "falloff (smoothing)"; the way up is not said; seen as a sharp or lazy pulse.
@@END

@@TRIAGE I-14
TO: SETTLED
WHY: L106 describes a button that is on or off above a threshold.
@@END

@@TRIAGE I-15
TO: INTERNAL
WHY: L108 gives the line; what it shows on a music signal is a look (L8).
@@END

@@TRIAGE I-16
TO: 243
WHY: He asked whether it is a good idea (L107); the choice follows the answer R221-b.
@@END

@@TRIAGE I-17
TO: INTERNAL
WHY: Layout and names (L8).
@@END

@@TRIAGE I-18
TO: INTERNAL
WHY: The standing half of the reading; L64 gives the Global glide to actions.
@@END

@@TRIAGE I-19
TO: INTERNAL
WHY: Technical: an effect keeps its identity when it is moved (L104).
@@END

@@TRIAGE I-20
TO: INTERNAL
WHY: Wording of lists only, after his own list of L83; Harmony adds it to the list of names.
@@END

@@TRIAGE I-21
TO: INTERNAL
WHY: A mend: it follows its layer, as an action does; anything else would be a fault.
@@END

@@TRIAGE I-22
TO: INTERNAL
WHY: Small: where the bar lines of a sample are counted from.
@@END

@@TRIAGE I-23
TO: INTERNAL
WHY: Consistency of one setting.
@@END

@@TRIAGE I-24
TO: INTERNAL
WHY: Technical.
@@END

@@TRIAGE J-1
TO: INTERNAL
WHY: L114 is read as names ("what the 2 recordings are called"); nothing is taken away; a film box is small to remove.
@@END

@@TRIAGE J-2
TO: 269
WHY: L116 says files exist, not what one holds or whether an import replaces.
@@END

@@TRIAGE J-3
TO: SETTLED
WHY: L118: tap tempo, mappable ("R227 all good"); in Review his own words: the spacebar stops the playing.
@@END

@@TRIAGE J-4
TO: 270
WHY: Two hidden consequences of his 207 c (L120); the controller's name settles both.
@@END

@@TRIAGE J-5
TO: SETTLED
WHY: L122 with L65, L68, L80: strictly the messages he named and a failed save; the "may tell you" clause is struck.
@@END

@@TRIAGE J-6
TO: INTERNAL
WHY: The safe way for the stage; how a message leaves is a look (L8).
@@END

@@TRIAGE J-7
TO: SETTLED
WHY: L112: the playhead link "should be called as such"; L21 "timeline mode" and L11 "BPM mode" are his.
@@END

@@TRIAGE J-8
TO: 244
WHY: No words say what a knob on a clip's slider follows; his two rules point two ways.
@@END

@@TRIAGE J-12
TO: INTERNAL
WHY: Follows 206 A and 207 c; small to widen later.
@@END

@@TRIAGE J-13
TO: INTERNAL
WHY: An edge he would not notice.
@@END

@@TRIAGE J-14
TO: INTERNAL
WHY: Deferred with 214 by the default he took.
@@END

@@TRIAGE J-15
TO: INTERNAL
WHY: His Undo rule applied to the new controls.
@@END

@@TRIAGE J-16
TO: INTERNAL
WHY: List housekeeping; nothing fires either way.
@@END

@@TRIAGE J-17
TO: INTERNAL
WHY: Layout and wording (L8).
@@END

@@TRIAGE J-18
TO: 271
WHY: Option A of 206 does not say it; the safe way for the stage is the reading.
@@END

@@TRIAGE J-19
TO: INTERNAL
WHY: The Mac's own habits.
@@END

@@TRIAGE J-20
TO: INTERNAL
WHY: A held /2 would run the tempo to its limit; nobody wants that.
@@END

@@TRIAGE J-21
TO: INTERNAL
WHY: An edge of a rule he gave.
@@END

@@TRIAGE J-22
TO: INTERNAL
WHY: The plain meaning of his words "not saved".
@@END

@@TRIAGE J-23
TO: INTERNAL
WHY: Not offered on the page; small to add.
@@END

@@TRIAGE K-1
TO: SETTLED
WHY: L128: "plan all of these": all six are in the plan; each is built only after his go (L1).
@@END

@@TRIAGE K-2
TO: INTERNAL
WHY: Render is in the plan (L128); what it makes returns with its plan.
@@END

@@TRIAGE K-3
TO: 247
WHY: L126 against a reading he left; the default follows L126 (nothing of MilkDrop changes); asked.
@@END

@@TRIAGE K-7
TO: INTERNAL
WHY: One row of the Clip tab under a default he took; it returns when that part is planned.
@@END

@@TRIAGE K-9
TO: INTERNAL
WHY: A mend: the autopilot's job is to move on; no real other way.
@@END

@@TRIAGE K-10
TO: 272
WHY: L94 and L96 lean two ways on where the picked input is remembered.
@@END

@@TRIAGE K-11
TO: 273
WHY: L72 "everything else" can be read wider than his earlier list of what stop takes.
@@END

@@TRIAGE K-12
TO: INTERNAL
WHY: Details of a planned thing; they come back to him when it is planned.
@@END

@@TRIAGE K-13
TO: INTERNAL
WHY: Details of a planned thing; addresses can be added later.
@@END

@@TRIAGE K-14
TO: INTERNAL
WHY: Link is planned, not built; shown to him with its plan and its licence check.
@@END

@@TRIAGE K-16
TO: INTERNAL
WHY: Three rare things at once; follows L98 and the standing reading.
@@END

@@TRIAGE K-19
TO: INTERNAL
WHY: A file's place.
@@END

@@TRIAGE K-20
TO: INTERNAL
WHY: The order of work is Harmony's to set (L110 fixes only the last part).
@@END

@@TRIAGE K-21
TO: INTERNAL
WHY: A small default he can flip on screen.
@@END

@@TRIAGE K-22
TO: INTERNAL
WHY: Completes the pull-out rule he left standing; rare.
@@END

@@TRIAGE K-23
TO: INTERNAL
WHY: Technical: the test tools.
@@END

@@TRIAGE X-1
TO: SETTLED
WHY: L13 and L17, as A-1.
@@END

@@TRIAGE X-2
TO: SETTLED
WHY: L13 with his earlier answer to 111, as A-3: what is not BPM based is unaffected.
@@END

@@TRIAGE X-3
TO: MERGED 225
WHY: The same question as D-2.
@@END

@@TRIAGE X-4
TO: MERGED 217
WHY: The same question as A-18 (items 217 and 218).
@@END

@@TRIAGE X-5
TO: INTERNAL
WHY: Bookkeeping; what he sees is settled by L14; the twin of A-9.
@@END

@@TRIAGE X-6
TO: SETTLED
WHY: L122 with L65, L68 and L80, as J-5.
@@END

@@TRIAGE X-7
TO: MERGED 263
WHY: The same line as G-2.
@@END

@@TRIAGE X-8
TO: MERGED 229
WHY: The same question as E-2.
@@END

@@TRIAGE X-10
TO: MERGED 250
WHY: The same point as A-2 (tap): one line.
@@END

@@TRIAGE X-11
TO: MERGED 252
WHY: The same point as B-10.
@@END

@@TRIAGE X-12
TO: INTERNAL
WHY: Technical: an architect's read before Review is planned.
@@END

@@TRIAGE X-13
TO: INTERNAL
WHY: Repeats a reading he left standing; how it is done is technical.
@@END

@@TRIAGE X-14
TO: INTERNAL
WHY: Technical, the twin of E-24.
@@END

@@TRIAGE X-15
TO: INTERNAL
WHY: Technical: a read of the code.
@@END

@@TRIAGE X-16
TO: INTERNAL
WHY: Technical: two count tables read before the last part is planned.
@@END

@@TRIAGE X-17
TO: INTERNAL
WHY: Presses with no effect, the twin of A-16.
@@END

@@TRIAGE X-18
TO: INTERNAL
WHY: Technical: checked when the transport part is built.
@@END

@@TRIAGE X-19
TO: INTERNAL
WHY: Technical: an old fault to look up.
@@END

@@TRIAGE X-20
TO: INTERNAL
WHY: Technical: a measurement.
@@END

@@TRIAGE X-21
TO: INTERNAL
WHY: Technical: a test when built.
@@END

@@TRIAGE X-22
TO: INTERNAL
WHY: Technical: Harmony reads the code for the MilkDrop document.
@@END

@@TRIAGE X-23
TO: INTERNAL
WHY: His "yes. I like that." (L72) covers the list that holds this entry.
@@END

@@TRIAGE X-24
TO: MERGED 221
WHY: The same question as B-2.
@@END

@@TRIAGE X-25
TO: INTERNAL
WHY: Technical: three reads by Harmony.
@@END

@@TRIAGE X-26
TO: INTERNAL
WHY: Follows L52 and a reading he left; the twin of C-5 and C-6.
@@END

@@TRIAGE X-27
TO: MERGED 224
WHY: The same question as D-1.
@@END

@@TRIAGE X-28
TO: SETTLED
WHY: L101, L20, L21, as H-1.
@@END

@@TRIAGE X-29
TO: MERGED 220
WHY: The same question as B-1.
@@END
