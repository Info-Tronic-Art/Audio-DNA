# PAGE 2 ITEMS (s-rta-1007): the nine answers Boris is owed, the page items (ASK then LINE, numbered from 217), and one TRIAGE block per assumption of assume-all.md. Made by the ruling over all topics (ruling-page.md); parsed by lint_page.py and render_page.py.

## THE NINE ANSWERS

@@PAGE-ANSWER codec
ASKED: "Is there any reason for us to build our own codec that is optimized for our system" [L3]
TITLE: A codec of our own: no
TEXT: No. I would not build one, and I could not promise to make one reliable: it would play only in this app and need its own converter and long testing, with no outside player to check its files. What you want from DXV exists in a free format with the same purpose, HAP, and the app plays HAP files. Such formats store each picture whole, so a jump, backwards play or a random beat should need no catching up. Not measured yet: how long a jump waits on your ordinary clips, and how much bigger a fast copy is (my estimate: much bigger). So I measure first, on two of your clips, and then we decide.
ITEM: 239
@@END

@@PAGE-ANSWER lowres
ASKED: "How much comp resource would this take up?" [L6]
TITLE: The low-resolution show recording: light, but not yet measured
TEXT: It should be light, and I would build it; but nobody has measured it, so every number is an estimate. A film a quarter as wide and as high as the show (480 by 270 for full HD), 30 pictures a second: about 1 to 5 percent of the processor and 25 to 150 MB for each 10 minutes. Not known: whether fetching the small picture makes the live picture hitch; a test on your Mac with a heavy show comes before any promise. Pieces alone are not enough: a film cut off by a crash cannot be played, so each piece is also written so that a crash costs only its last seconds.
ITEM: 232
@@END

@@PAGE-ANSWER 189
ASKED: "keep it on automatic mode, or will that correction necessitate going back to manual mode?" and "Same question if I correct, where the one is." [L32]
TITLE: Correcting the listening without leaving automatic mode
TEXT: Yes, it can be built: you correct the tempo or the 1, and the app stays in automatic mode. Nothing switches to manual by itself. Whether it holds on your music is not measured: I can say only after a test on your three DJ tracks, which I do not have yet. If it fails, a correction needs manual mode, and I tell you before anything is built. The tempo: you set the number; the app keeps listening, but only close around your number, so it follows the music's slow changes and keeps the beats on the music. The 1: when you press Resync, that beat is the 1, and the app counts on from it.
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
TEXT: I wrote it, and I take it back. In my plan nothing plays a whole recording inside the live window, so I looked for another way to show a whole recorded night and wrote that you could save all of it as one action. It is not needed. A recording is played in Review; with your 186 b the outputs show what Review shows, and you hear its sound. Actions stay pieces that you cut from a recording.
ITEM: 229
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
TEXT: Yes, it is a good idea. The logic: such an effect (a strobe, a pulse) follows the beat clock, not the signals. So what holds the clock holds the effect: on pause it stands where it was and goes on in step when you press play; on stop it waits and starts on your new 1. That is your own rule: pause pauses everything the beat controls. A strobe that flashed on through a pause would be out of step afterwards. The Master Signal only turns signals down, so at 0 the strobe keeps its beat; to stop it, switch the effect off. One thing to know: a strobe paused in its dark moment stays dark until you press play.
ITEM: 243
@@END

@@PAGE-ANSWER review-name
ASKED: "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" [L114]
TITLE: A name for the review screen
TEXT: Review is a good name, and I keep writing it until you say otherwise. One risk: when you speak or dictate, Review and Preview differ by one sound, and the app has both. If that bothers you, my pick is Studio: musicians pair Live and Studio, and the studio is where you listen back to a recording and make things from it. The other candidate is Replay, your own earlier word ("the replay window"): clear, but close to play on the tempo bar.
ITEM: 245
@@END

@@PAGE-ANSWER 207-hold
ASKED: "make an argument for why we should have a key/pad hold setting" [L120]
TITLE: Why a pad could have a hold setting
TEXT: The argument: hold turns a pad into an instrument. With a band you play by hand: the flash, the logo or the effect is there exactly as long as you hold the pad, through a drum fill or a held chord, and gone when you let go. A DJ holds a pad through the build-up and lets go on the drop. With press only, each of these needs a second press that must also be on time; miss it and the flash stays on. It would be one switch per pad, set to press until you change it. One limit: on a BPM mode clip a short hold can end before the 1, and then nothing shows. I would add it.
ITEM: 246
@@END

## THE PAGE ITEMS: ASK

@@PAGE-ITEM 217
TOPIC: A
KIND: ASK
TEXT: I assume a tempo you correct by hand in automatic mode holds until the music clearly changes tempo (a new track that is not beat-matched); then the app follows the music again.
B: It holds until you change it yourself, whatever the music does.
C: none
FROM: A-18, X-4, 189, C14
@@END

@@PAGE-ITEM 218
TOPIC: A
KIND: ASK
TEXT: I assume in automatic mode the 1 is always yours: the press that starts the beat, or your Resync. The app keeps the beats on the music but never moves the 1 by itself.
B: The app also places the 1 by itself when it is sure, until your first Resync.
C: none
FROM: A-18, X-4, 189, R164
@@END

@@PAGE-ITEM 219
TOPIC: B
KIND: ASK
TEXT: I assume a clip you preview by its name starts as it would on the output: a BPM-mode clip on the next 1; any other clip at once, its actions joining on the next 1.
B: Every previewed clip waits for the next 1, in BPM mode or not.
C: none
FROM: B-1, X-29, R151, R179
@@END

@@PAGE-ITEM 220
TOPIC: B
KIND: ASK
TEXT: I assume that while the tempo is paused or stopped, a clip you preview by its name still plays in the preview, with its actions, at the tempo number, from your click. Previewing never starts the tempo.
B: It waits on its first frame until the tempo runs again and reaches the 1.
C: none
FROM: B-2, X-24, U18
@@END

@@PAGE-ITEM 221
TOPIC: B
KIND: ASK
TEXT: I assume master cue only shows in the preview what global does on the output at that moment: the global effects, moved by any global action that runs. It is not a way to try a global effect or action unseen.
B: Master cue also lets you switch a global effect or action on for the preview only, to try it first.
C: Master cue shows the whole output in the preview, as on a DJ mixer.
FROM: B-4, 177
@@END

@@PAGE-ITEM 222
TOPIC: C
KIND: ASK
TEXT: I assume that when a show and the computer hold two different versions of one preset, the newer one wins, name and values, without asking.
B: A small window asks which one to keep.
C: Both are kept, and the show's one gets a number added to its name.
FROM: C-3
@@END

@@PAGE-ITEM 223
TOPIC: C
KIND: ASK
TEXT: I assume a preset remembers an envelope you drew only by its name. Loaded in a show that has no envelope of that name, the slider takes the preset's value and nothing moves it.
B: The preset carries the envelope's shape and makes that envelope in a show that lacks it.
C: none
FROM: C-12, F-6
@@END

@@PAGE-ITEM 224
TOPIC: D
KIND: ASK
TEXT: I assume ONE slider, Global glide (instant to 4 seconds), times every glide of an action: its start, its switching off, going back after playing once, your hand letting go, and a Resync.
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
TEXT: I assume the two stops differ. Stop actions switches every action off, and the values glide back. The tempo stop leaves the action buttons on: the actions stand still and start again from their beginning when the beat next runs.
B: The tempo stop also switches every action off; you switch on again the ones you want.
C: Stop actions is a hold: actions stand still while it is lit and carry on when released.
FROM: D-3, 180, R224
@@END

@@PAGE-ITEM 227
TOPIC: D
KIND: ASK
TEXT: I assume a play-once action on a clip plays once each time that clip is fired, its button staying on; on a layer or on global it plays once, and its button then goes off by itself.
B: Every play-once action switches its own button off when it is done.
C: Its button stays on; it plays again only after you switch it off and on.
FROM: D-4
@@END

@@PAGE-ITEM 228
TOPIC: D
KIND: ASK
TEXT: I assume a preset loaded while an action plays is kept underneath, as my answer describes: when the action goes off, its sliders glide to the preset's values.
B: Those sliders go back to their values from before the action; the preset's values for them are lost.
C: none
FROM: D-5, R129
@@END

@@PAGE-ITEM 229
TOPIC: E
KIND: ASK
TEXT: I assume a recording plays only in Review, never inside the live window. Record Over is a button in Review: the recording plays in real time, and what you move on the controller is recorded over it.
B: Record Over runs in the live window, through your live layers, and a whole recording can be played there too.
C: none
FROM: E-2, X-8, 195, R184
@@END

@@PAGE-ITEM 230
TOPIC: E
KIND: ASK
TEXT: I assume a clip made with record to clip is a full picture, exactly as on the output: where no layer shows, it is black.
B: Its empty parts stay see-through, so layers under it show through; likely price, not measured: bigger files and a heavier recording.
C: none
FROM: E-9, D22
@@END

@@PAGE-ITEM 231
TOPIC: E
KIND: ASK
TEXT: I assume an action you save in Review goes into the show the recording was made in, onto the same clip, wherever that clip is by now. If the clip is gone, the app tells you and nothing is saved.
B: If the clip is gone, the action is kept and you paste it onto a clip you choose.
C: It goes into whatever show is open in the live window.
FROM: E-11
@@END

@@PAGE-ITEM 232
TOPIC: E
KIND: ASK
TEXT: I assume, once a test shows it is light, the low-resolution recording is made with every record show unless you untick it: a quarter as wide and as high as the show, with sound, in 10-minute pieces.
B: A quarter of the area instead (half as wide and as high): sharper, about four times the size.
C: Only when you tick it; or 20-minute pieces; or no sound: say which.
FROM: E-21, G4
@@END

@@PAGE-ITEM 233
TOPIC: F
KIND: ASK
TEXT: I assume you select an empty cell by clicking where a clip's name would be, at the bottom of the cell, and can then paste into it. A click anywhere else on it triggers it: the layer goes empty.
B: An empty cell cannot be selected: you paste into it with a right-click and Paste.
C: none
FROM: F-1, G2
@@END

@@PAGE-ITEM 234
TOPIC: F
KIND: ASK
TEXT: I assume pasting over, cutting or deleting a clip that is playing does not change the picture: it plays on until you fire something else on that layer.
B: The layer switches at once to the pasted clip, and goes empty at once on a cut or a delete.
C: none
FROM: F-3, G2, G3
@@END

@@PAGE-ITEM 235
TOPIC: F
KIND: ASK
TEXT: I assume "delete all the old show files" means every show saved so far and the app's backup copies of them. I list them first; after your yes they go to the Trash. Deck files, recordings, presets and settings stay.
B: The old deck files and the old recordings go too.
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
TEXT: I assume opening a show switches on the outputs it was saved with, if they are plugged in, and never switches off an output that is already on.
B: Opening a show sets the outputs exactly as it was saved: every other output goes off.
C: none
FROM: G-1, R191
@@END

@@PAGE-ITEM 238
TOPIC: H
KIND: ASK
TEXT: I assume a new clip in BPM mode gets its first number of beats as in your Resolume: 4, 8, 16, 32 and so on, whichever plays the whole clip nearest to its normal speed. No out point is moved.
B: Your rule of 4 October: the largest whole group of 4 bars that fits, the out point moved in to match, exact normal speed.
C: none
FROM: H-5, R218, C13
@@END

@@PAGE-ITEM 239
TOPIC: H
KIND: ASK
TEXT: I assume we build no codec. I measure first, on two of your clips, how long a jump waits; only if the wait shows do I add a command that makes fast copies. Ordinary files keep playing.
B: Plan the command that makes fast copies now, without measuring first.
C: No such command: you make fast copies yourself with tools you own.
FROM: H-14, G1, E-27
@@END

@@PAGE-ITEM 240
TOPIC: I
KIND: ASK
TEXT: I assume layers do not come in kinds: every layer, the bottom one too, is blended in by its blend mode and its transparency slider. There are no mask layers, and the autopilot counts no kinds of layer.
B: A layer keeps a kind, picked in the Layer tab: replaces, blends, effects only, mask.
C: none
FROM: I-1, 204
@@END

@@PAGE-ITEM 241
TOPIC: I
KIND: ASK
TEXT: I assume a slider set to Timeline moves straight from its low end to its high end as the clip plays through. A shape you draw is an envelope, and an envelope always runs on the beat, never along the clip.
B: An envelope can also follow the playhead: its drawn shape is stretched over the clip from start to end.
C: none
FROM: I-6, 205
@@END

@@PAGE-ITEM 242
TOPIC: I
KIND: ASK
TEXT: I assume a One Shot on a slider runs once from the next 1 after its clip is fired: a clip's slider with that clip, a layer's slider with every clip fired on that layer, a global slider when switched on.
B: It starts the moment its clip starts, off the beat too.
C: It runs only when you press a start button on that slider.
FROM: I-9, R221
@@END

@@PAGE-ITEM 243
TOPIC: I
KIND: ASK
TEXT: I assume effects that read the beat by themselves (a strobe, a pulse) hold still while the beat is paused or stopped, and keep pulsing when the Master Signal is at 0.
B: They keep pulsing through a pause.
C: The Master Signal at 0 calms them too.
FROM: I-16, R221, R214
@@END

@@PAGE-ITEM 244
TOPIC: J
KIND: ASK
TEXT: I assume a knob you map to a clip's slider stays with that clip, wherever the clip goes; a copy of the clip has no knob. A knob on a layer's or a global slider stays there.
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
TEXT: I assume your answer stands after my argument: a pad or a key only presses, and there is no hold setting.
B: Add it: one switch per pad, press or hold, set to press until you change it.
C: none
FROM: 207 (the answer 207-hold), D34
@@END

@@PAGE-ITEM 247
TOPIC: K
KIND: ASK
TEXT: I assume MilkDrop reaches the output only through a clip that plays. With no clip playing, a click on a preset in the MilkDrop tab changes the preset, and the output stays black.
B: MilkDrop is left fully alone: a preset clicked in its tab shows on the output whenever no clip plays.
C: none
FROM: K-3, R228, R202
@@END

## THE PAGE ITEMS: LINE

@@PAGE-ITEM 248
TOPIC: A
KIND: LINE
TEXT: I assume a click on an empty cell never starts a stopped beat and never ends a pause.
B: During a pause it sets the beat going, as a click on a clip does.
C: none
FROM: A-6
@@END

@@PAGE-ITEM 249
TOPIC: A
KIND: LINE
TEXT: I assume a BPM-mode clip fired a hair after the 1 (within a tenth of a beat) starts at once, in time, not a bar later.
B: It waits for the next 1.
C: none
FROM: A-15, D4
@@END

@@PAGE-ITEM 250
TOPIC: B
KIND: LINE
TEXT: I assume the slider beside a cue button is a try-out for the preview only: it starts at full and never moves the layer's own slider.
B: It starts at the layer's own transparency.
C: none
FROM: B-3, R141
@@END

@@PAGE-ITEM 251
TOPIC: B
KIND: LINE
TEXT: I assume the preview picture may be smaller and less smooth than the output; the output always comes first.
B: none
C: none
FROM: B-10, X-11, D8
@@END

@@PAGE-ITEM 252
TOPIC: B
KIND: LINE
TEXT: I assume two first-build limits in the preview: trails (Echo, Freeze, Feedback) may look different, and a simulation or MilkDrop clip may move exactly like one already playing.
B: none
C: none
FROM: B-11, D9
@@END

@@PAGE-ITEM 253
TOPIC: C
KIND: LINE
TEXT: I assume a show file carries all your presets, and opening it on another computer adds the ones missing there for good, without asking.
B: Only the presets that the show uses travel with it.
C: none
FROM: C-2
@@END

@@PAGE-ITEM 254
TOPIC: C
KIND: LINE
TEXT: I assume only files and deck clips are previewed: a double-click on an effect shows its properties but no picture, and a source is not previewed.
B: Effects and sources are previewed by a double-click too, as in Resolume.
C: none
FROM: C-9, B-9
@@END

@@PAGE-ITEM 255
TOPIC: D
KIND: LINE
TEXT: I assume the whole pasted action stays muted while the clip lacks anything it moves, and wakes up when you add what is missing.
B: Only the missing part is muted; the rest of the action plays.
C: none
FROM: D-6, 184, R162
@@END

@@PAGE-ITEM 256
TOPIC: D
KIND: LINE
TEXT: I assume a layer set to Ignore Column Trigger is also skipped by the clip fires of a global action, while its sliders still follow that action.
B: Ignore Column Trigger blocks column clicks only.
C: none
FROM: D-9
@@END

@@PAGE-ITEM 257
TOPIC: D
KIND: LINE
TEXT: I assume a clip that an action fired stays on its layer when the action ends; only sliders, buttons and lists go back.
B: The layer goes back to the clip that played before the action.
C: none
FROM: D-26
@@END

@@PAGE-ITEM 258
TOPIC: E
KIND: LINE
TEXT: I assume Record Over takes everything in the keyboard and MIDI mapping, clip fires too; what you move with the mouse is not recorded over.
B: Only knobs and faders record over.
C: none
FROM: E-3
@@END

@@PAGE-ITEM 259
TOPIC: E
KIND: LINE
TEXT: I assume a press on record to clip while the tempo is paused or stopped waits: the recording starts on the first 1 once the tempo runs.
B: The button does nothing until the tempo plays.
C: none
FROM: E-5
@@END

@@PAGE-ITEM 260
TOPIC: E
KIND: LINE
TEXT: I assume a clip recording runs on to the end of the bar in which you press stop and keeps every bar (5 stay 5); you trim it yourself.
B: It runs on until a group of 4 bars is full.
C: none
FROM: E-6, E-30
@@END

@@PAGE-ITEM 261
TOPIC: E
KIND: LINE
TEXT: I assume record show starts and stops at your press and runs on through a tempo pause or stop; only record to clip waits for the 1.
B: none
C: none
FROM: E-10
@@END

@@PAGE-ITEM 262
TOPIC: E
KIND: LINE
TEXT: I assume your removal rule is for anything taken out while you record (an effect, a clip, a layer): the removal is recorded and its tracks then sit at default.
B: none
C: none
FROM: E-17
@@END

@@PAGE-ITEM 263
TOPIC: E
KIND: LINE
TEXT: I assume record show, record to clip and the low-resolution recording can run together; if the computer cannot keep up, the output stays smooth and a recording loses pictures.
B: A second recording is refused while one runs.
C: none
FROM: E-26, D20
@@END

@@PAGE-ITEM 264
TOPIC: E
KIND: LINE
TEXT: I assume an effect or a clip you add while recording is recorded too, from the moment you add it.
B: none
C: none
FROM: E-35
@@END

@@PAGE-ITEM 265
TOPIC: E
KIND: LINE
TEXT: I assume a recording that reaches its end in Review stops on its last picture and stays; play starts it again from the beginning.
B: The clips keep running with the last settings until you press stop.
C: none
FROM: E-37
@@END

@@PAGE-ITEM 266
TOPIC: G
KIND: LINE
TEXT: I assume that when you start the app and it opens your last show, that show's outputs come on by themselves if they are plugged in.
B: At the start of the app no output comes on by itself.
C: none
FROM: G-3, F-12
@@END

@@PAGE-ITEM 267
TOPIC: G
KIND: LINE
TEXT: I assume a show remembers Syphon like a screen: a show saved with Syphon on switches it on again when it is opened.
B: Syphon stays off until you tick it.
C: none
FROM: G-2, X-7
@@END

@@PAGE-ITEM 268
TOPIC: G
KIND: LINE
TEXT: I assume the output on your main display, the one the app itself is on, never comes on by itself, because it would cover the app.
B: none
C: none
FROM: G-4
@@END

@@PAGE-ITEM 269
TOPIC: G
KIND: LINE
TEXT: I assume a show brings back only the screens it remembers: with a different projector nothing comes on by itself, and you tick it once.
B: The picture goes to whatever outside screen is plugged in.
C: none
FROM: G-5, D28
@@END

@@PAGE-ITEM 270
TOPIC: G
KIND: LINE
TEXT: I assume All Outputs Off is for the moment only: a show you open afterwards brings its outputs on again.
B: After All Outputs Off nothing comes on until you switch an output on yourself.
C: none
FROM: G-15
@@END

@@PAGE-ITEM 271
TOPIC: H
KIND: LINE
TEXT: I assume a BPM-mode clip shows one marker per beat, and Random may land on any beat, not only on the 1 of a bar.
B: It shows bar lines only, and Random lands only on a 1.
C: none
FROM: H-1, X-28, C23
@@END

@@PAGE-ITEM 272
TOPIC: H
KIND: LINE
TEXT: I assume firing the clip that is already playing always restarts it, even when it is set to continue where it left off.
B: Set to continue, it plays on undisturbed when you fire it again.
C: none
FROM: H-3
@@END

@@PAGE-ITEM 273
TOPIC: H
KIND: LINE
TEXT: I assume a video you bring in starts in Timeline mode; you switch the clips you want on the beat to BPM mode yourself.
B: Every new video starts in BPM mode.
C: One setting says which mode new clips start in.
FROM: H-15
@@END

@@PAGE-ITEM 274
TOPIC: I
KIND: LINE
TEXT: I assume that after a One Shot has run, the slider stays on the signal's last value; it does not glide back as an action does.
B: It glides back to where you had it by hand.
C: none
FROM: I-10
@@END

@@PAGE-ITEM 275
TOPIC: I
KIND: LINE
TEXT: I assume Falloff means: the value jumps up with the signal at once and comes down slowly, from instant to 2 seconds.
B: It smooths the way up and the way down alike.
C: none
FROM: I-13
@@END

@@PAGE-ITEM 276
TOPIC: J
KIND: LINE
TEXT: I assume "All else is gone" is about names only: record show keeps its three boxes (parameters, audio, a full-size film), and Render stays planned.
B: The full-size film box goes too.
C: none
FROM: J-1, K-2
@@END

@@PAGE-ITEM 277
TOPIC: J
KIND: LINE
TEXT: I assume a mapping file holds the whole keyboard and MIDI mapping of the open show; importing one replaces that show's mapping, after asking.
B: An import adds to the mapping and overwrites only the keys that clash.
C: none
FROM: J-2
@@END

@@PAGE-ITEM 278
TOPIC: J
KIND: LINE
TEXT: I assume the spacebar is tap tempo in the live window (you can map it to something else), and in Review it stops and starts the playback.
B: It is tap tempo in Review as well.
C: none
FROM: J-3, R227
@@END

@@PAGE-ITEM 279
TOPIC: J
KIND: LINE
TEXT: I assume knobs must send their position: one that only sends steps will not work, and how hard you hit a pad changes nothing. Which controller do you use?
B: Knobs that send steps must work too.
C: none
FROM: J-4, 207, D33
@@END

@@PAGE-ITEM 280
TOPIC: J
KIND: LINE
TEXT: I assume these names: a clip plays in BPM mode or Timeline mode, and the signal that follows a layer's playhead reads Timeline in a slider's list.
B: That signal reads Playhead, so that Timeline is only the clip's mode.
C: none
FROM: J-7, I-7, 205
@@END

@@PAGE-ITEM 281
TOPIC: J
KIND: LINE
TEXT: I assume switching an output screen on or off cannot be put on a key or a pad, so that no stray hit blanks a projector.
B: It can go on a key or a pad like any button.
C: none
FROM: J-18
@@END

@@PAGE-ITEM 282
TOPIC: K
KIND: LINE
TEXT: I assume "plan all of these" means all six get built, not only designed: OSC, Link (if its licence allows), picking the audio input, a looping audio file, a text source, Render.
B: They are only designed on paper until you ask for each.
C: none
FROM: K-1, 215
@@END

@@PAGE-ITEM 283
TOPIC: K
KIND: LINE
TEXT: I assume the computer remembers which audio input you picked, for every show; a show does not hold it.
B: The show remembers it too, as it remembers its outputs.
C: none
FROM: K-10
@@END

@@PAGE-ITEM 284
TOPIC: K
KIND: LINE
TEXT: I assume the tempo bar never stops an audio file the app plays: on pause and on stop the music plays on.
B: The tempo bar's stop also stops the audio file.
C: none
FROM: K-11
@@END

## TRIAGE: one block per assumption of assume-all.md

@@TRIAGE A-1
TO: SETTLED
WHY: L13 and L17: a click on the clip starts the held clock, as play does; any clip.
@@END
@@TRIAGE A-2
TO: SETTLED
WHY: L1 (default 190 A) and L13: a tempo change does not move the 1; his earlier words agree.
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
TO: INTERNAL
WHY: A display number; his earlier 129 b keeps it, L13 speaks of the nudge's effect, not the amount.
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
WHY: L12 names his click on play or a clip; the autopilot part stood in a reading he read.
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
TO: 219
WHY: L35 "play right away" against L36 "triggered on the 1": asked once, as one start rule.
@@END
@@TRIAGE B-2
TO: 220
WHY: No words cover a preview while no 1 is coming; seen before every show; needs its own beat.
@@END
@@TRIAGE B-3
TO: 250
WHY: L34 "what the transparency would be" reads as a try-out; its start value is shown as a line.
@@END
@@TRIAGE B-4
TO: 221
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
TO: MERGED 254
WHY: The same point as C-9: what a double-click previews.
@@END
@@TRIAGE B-10
TO: 251
WHY: Decided without asking (L130); he would see a smaller, less smooth preview.
@@END
@@TRIAGE B-11
TO: 252
WHY: Decided without asking (L130); two limits he would see; full copies are a large addition.
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
TO: 253
WHY: L48 and L50 lean to all presets in both places; shown because a show carries them all.
@@END
@@TRIAGE C-3
TO: 222
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
TO: SETTLED
WHY: L53 (192 b): question 192 put loading Default under the same letters (L9).
@@END
@@TRIAGE C-9
TO: 254
WHY: L41 asks for the properties; L35 lists files and clips for the preview; shown as one line.
@@END
@@TRIAGE C-11
TO: INTERNAL
WHY: Follows L106: a preset holds what the slider holds for its signal; no real other way.
@@END
@@TRIAGE C-12
TO: 223
WHY: No words cover a preset whose envelope the show lacks; it changes what a preset file holds.
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
WHY: L72 can be read against the default he took for 180; one question for both stops.
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
TO: 255
WHY: L68 says "that specific action is muted": read as the whole action; shown as a line.
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
TO: 256
WHY: Shown and not corrected, but L73 adds a switch of its own; his to strike.
@@END
@@TRIAGE D-10
TO: SETTLED
WHY: L74 (182 b: the deck that is shown, other pictures after a deck switch) with L11 and L19.
@@END
@@TRIAGE D-11
TO: SETTLED
WHY: L67, said on this very reading: a control an action lets go goes back in the fade back time.
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
WHY: He sees and can change the result in the action save window before saving (L55).
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
TO: 257
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
TO: 258
WHY: L92 names the MIDI controller and sets the mouse apart; pads and keys are not named.
@@END
@@TRIAGE E-4
TO: INTERNAL
WHY: How the list of recordings fills up; L84 and L92 give the rule (kept beside the original).
@@END
@@TRIAGE E-5
TO: 259
WHY: L88 does not say what the press does while paused or stopped; follows L12; shown as a line.
@@END
@@TRIAGE E-6
TO: 260
WHY: L88 read to the letter (end of the bar, trimmed later); the 4-bar group is shown as b.
@@END
@@TRIAGE E-7
TO: INTERNAL
WHY: Follows L88 (nothing paused is recorded) and L72; neither way loses what was recorded.
@@END
@@TRIAGE E-8
TO: SETTLED
WHY: L88: "It must be the full screen composition", not one screen's output; his earlier words agree.
@@END
@@TRIAGE E-9
TO: 230
WHY: His earlier yes was for a one-layer recording, which L88 removes; it decides the file type.
@@END
@@TRIAGE E-10
TO: 261
WHY: L88 was said of record to clip; the reading that cannot lose any of the night.
@@END
@@TRIAGE E-11
TO: 231
WHY: No words say where an action saved in Review lands; it is what Review is for.
@@END
@@TRIAGE E-12
TO: INTERNAL
WHY: Technical; his "moved or missing" (L80) expects a file that points at the media.
@@END
@@TRIAGE E-13
TO: SETTLED
WHY: L81: only what was at default the entire time gets no row; L76 agrees.
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
TO: 262
WHY: L89 "a recording will be removed" is read as the thing removed; his to strike.
@@END
@@TRIAGE E-18
TO: SETTLED
WHY: L89 with the reading he left: an action never holds or changes the tempo.
@@END
@@TRIAGE E-19
TO: INTERNAL
WHY: A setting's first state; L86 says scrubbed sound will not sound good.
@@END
@@TRIAGE E-21
TO: 232
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
TO: 263
WHY: Decided without asking (L130); no words rank the recordings against the output.
@@END
@@TRIAGE E-27
TO: MERGED 239
WHY: The file types wait on the codec choice, which item 239 carries.
@@END
@@TRIAGE E-28
TO: INTERNAL
WHY: A technical limit of a fast scrub.
@@END
@@TRIAGE E-30
TO: MERGED 260
WHY: Follows item 260: a recording that is cut short is kept whole.
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
TO: 264
WHY: The mirror of his removal rule (L89); the page had said the opposite, so it is shown.
@@END
@@TRIAGE E-36
TO: INTERNAL
WHY: A starting position he moves anyway.
@@END
@@TRIAGE E-37
TO: 265
WHY: His older words and the page's line differ; an audience can see how a recording ends.
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
TO: MERGED 223
WHY: Its one open half, a signal the show lacks, is the point of C-12; the rest is technical.
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
TO: MERGED 266
WHY: The same point as G-3: outputs at the start of the app.
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
TO: 267
WHY: L96 with L9 against the default he took for 200; one tick either way; his to strike.
@@END
@@TRIAGE G-3
TO: 266
WHY: Follows L94 with L96; shown because the last page told him the opposite.
@@END
@@TRIAGE G-4
TO: 268
WHY: Departs from the letter of L96 for one output; most likely waved through.
@@END
@@TRIAGE G-5
TO: 269
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
TO: 270
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
TO: 271
WHY: L101, L20 and L21 say beat markers; shown once because his earlier words said bars only.
@@END
@@TRIAGE H-2
TO: INTERNAL
WHY: Follows Resolume's manual, his model (L102); he can see it in his Arena.
@@END
@@TRIAGE H-3
TO: 272
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
TO: 273
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
TO: MERGED 280
WHY: One decision on the word Timeline, with J-7.
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
TO: 274
WHY: The page's text stands; shown because his rule for actions (L64) points the other way.
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
TO: 275
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
WHY: Wording of lists, after his own list of L83; it goes into the list of names.
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
TO: 276
WHY: L114 "All else is gone" follows a sentence on names; L6, L92 and L128 keep other things.
@@END
@@TRIAGE J-2
TO: 277
WHY: L116 says files exist, not what one holds or whether an import replaces.
@@END
@@TRIAGE J-3
TO: 278
WHY: L118 against his earlier words on the spacebar in the replay window; most likely both.
@@END
@@TRIAGE J-4
TO: 279
WHY: Two hidden consequences of his 207 c (L120); the controller's name settles both.
@@END
@@TRIAGE J-5
TO: SETTLED
WHY: L122 ("already dicsussed above") points at L65, L68 and L80, with his earlier list.
@@END
@@TRIAGE J-6
TO: INTERNAL
WHY: The safe way for the stage; how a message leaves is a look (L8).
@@END
@@TRIAGE J-7
TO: 280
WHY: A name he reads all night; it changes a line of the page that he read and left.
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
TO: 281
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
TO: 282
WHY: L128 says plan; whether planned means built decides how much is built.
@@END
@@TRIAGE K-2
TO: MERGED 276
WHY: What L114 removes is shown once, in item 276; Render's details return with its plan.
@@END
@@TRIAGE K-3
TO: 247
WHY: L126 and a reading he left standing meet here; seen on the output.
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
TO: 283
WHY: L94 and L96 lean two ways on where the picked input is remembered.
@@END
@@TRIAGE K-11
TO: 284
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
TO: MERGED 267
WHY: The same line as G-2.
@@END
@@TRIAGE X-8
TO: MERGED 229
WHY: The same question as E-2.
@@END
@@TRIAGE X-10
TO: SETTLED
WHY: L1 (default 190 A) and L13, as A-2.
@@END
@@TRIAGE X-11
TO: MERGED 251
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
TO: MERGED 220
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
TO: MERGED 271
WHY: The same point as H-1.
@@END
@@TRIAGE X-29
TO: MERGED 219
WHY: The same question as B-1.
@@END

Written (system clock): see the date line below.
Thu Oct  8 00:52:18 EDT 2026
STATUS: DONE
