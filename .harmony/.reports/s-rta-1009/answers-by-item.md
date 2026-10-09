# Boris's answers to page 2, box by box (made by wf/file_answers.py from the byte-exact file; never edit by hand)
Each block: the box, the line of his file, the page's item as he saw it, HIS WORDS verbatim. A box that is not listed was left empty = accepted as written.

@@BOX answer codec
BF: BF240
FILE-LINE: 8
HIS-WORDS: after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?
@@END

@@BOX answer lowres
BF: BF241
FILE-LINE: 11
HIS-WORDS: we should test which codecs work best. Maybe something close to the output monitor resolution? 30 fps is the fastest or even 15 could be ok. This is just for reference. If we want a very hd recording, we can make an HD render from the Recording Review
@@END

@@BOX answer 189
BF: BF242
FILE-LINE: 14
HIS-WORDS: I imagine we will be fine tuning this beat detection till it's perfect and we should be able to resync the 1 if the detection is finidng the correct bpm but not the correct 1
@@END

@@BOX answer R188-snapshot
BF: BF243
FILE-LINE: 17
HIS-WORDS: Perhaps a good snapshot is a save of the show exactly where it is with all the settings and the output and everything so if there is a cool inspiring moment, that happened, the user can push a shortcut button and save that to look out later
@@END

@@BOX answer R221-b
BF: BF244
FILE-LINE: 20
HIS-WORDS: For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects
@@END

@@BOX answer review-name
BF: BF245
FILE-LINE: 23
HIS-WORDS: Let's go with studio. That's perfect.
@@END

@@BOX 218
BF: BF246
FILE-LINE: 26
PAGE-ITEM: [218] I assume in automatic mode the 1 is always yours (the press that starts the beat, or your Resync): the app never moves it by itself, so after a new track it may sit wrong until you press Resync.
PAGE-WAY-b: The app also places the 1 by itself when it is sure, until your first Resync.
HIS-WORDS: b. app should always try to find the 1 and I will correct if necessary
@@END

@@BOX 220
BF: BF247
FILE-LINE: 29
PAGE-ITEM: [220] I assume a clip you preview by its name waits for the next 1 before it starts, in BPM mode or not, so that its actions play in time with the music.
PAGE-WAY-b: Only a BPM-mode clip waits; any other clip starts at once, and its actions join on the next 1.
HIS-WORDS: I think I want to change this. Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat. If the clip is loaded into the layer, and we are cueing this way, then it should play in time
@@END

@@BOX 222
BF: BF248
FILE-LINE: 32
PAGE-ITEM: [222] I assume master cue only shows in the preview what global does on the output at that moment: the global effects, moved by any global action that runs. It is not a way to try a global effect or action unseen.
PAGE-WAY-b: Master cue also lets you switch a global effect or action on for the preview only, to try it first.
PAGE-WAY-c: Master cue shows the whole output in the preview, as on a DJ mixer.
HIS-WORDS: what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?
@@END

@@BOX 223
BF: BF249
FILE-LINE: 35
PAGE-ITEM: [223] I assume that when a show and the computer hold two different versions of one preset (you changed it on one computer and not on the other), the newer one wins, without asking.
PAGE-WAY-b: A small window asks which one to keep.
PAGE-WAY-c: The show's version always wins, as with the keyboard and MIDI mapping.
HIS-WORDS: b
@@END

@@BOX 226
BF: BF250
FILE-LINE: 38
PAGE-ITEM: [226] I assume the tempo stop also switches every layer and global action off, as Stop actions does; you switch on again the ones you want.
PAGE-WAY-b: Their buttons stay on: the actions stand still and start again from their beginning when the beat next runs.
HIS-WORDS: tempo stop stops all actions, not just global
@@END

@@BOX 227
BF: BF251
FILE-LINE: 41
PAGE-ITEM: [227] I assume a play-once action on a clip plays once every time that clip is triggered. On a layer or on global it plays once, and its button then goes off by itself.
PAGE-WAY-b: On a clip too, its button goes off after one play; you switch it on again yourself.
PAGE-WAY-c: Its button always stays on; it plays again only after you switch it off and on.
HIS-WORDS: neither. It's button stays on and that action plays again only when that clip is re-triggered
@@END

@@BOX 228
BF: BF252
FILE-LINE: 44
PAGE-ITEM: [228] I assume that when you load a preset onto an effect while an action is moving one of its sliders, that slider keeps moving with the action, and when the action goes off it glides to the preset's value.
PAGE-WAY-b: It glides back to the value it had before the action; the preset's value for it is lost.
HIS-WORDS: b
@@END

@@BOX 231
BF: BF253
FILE-LINE: 47
PAGE-ITEM: [231] I assume, once a test shows it is light, the low-resolution show recording runs with every record show, with sound, unless you untick it.
PAGE-WAY-b: It runs only when you tick it.
PAGE-WAY-c: It has no sound.
HIS-WORDS: It defaults to running next to the show recording so that we can see if there is a mistake in the parameter recording. It has no sound.
@@END

@@BOX 232
BF: BF254
FILE-LINE: 50
PAGE-ITEM: [232] I assume record to clip runs on to the end of the bar in which you press stop and keeps every bar it recorded (5 bars stay 5); you trim it yourself.
PAGE-WAY-b: It runs on until a group of 4 bars is full.
PAGE-WAY-c: It stops at the end of the bar, but the clip loops only whole groups of 4 bars (5 recorded bars loop as 4); the rest stays in the file.
HIS-WORDS: c
@@END

@@BOX 233
BF: BF255
FILE-LINE: 53
PAGE-ITEM: [233] I assume you select an empty cell by clicking where a clip's name would be, at the bottom of the cell, and can then paste into it. A click anywhere else on it triggers it: the layer goes empty.
PAGE-WAY-b: A plain click on an empty cell both selects it and triggers it (the layer goes empty); you then paste.
HIS-WORDS: Both are correct. You can select the name without triggering it, or you can select the cell which triggers it and selects it.
@@END

@@BOX 234
BF: BF256
FILE-LINE: 56
PAGE-ITEM: [234] I assume pasting over, cutting or deleting a clip that is playing does not change the picture: it plays on until you trigger something else on that layer.
PAGE-WAY-b: The layer switches at once to the pasted clip, and goes empty at once on a cut or a delete.
HIS-WORDS: Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play. If a clip is playing, and I change the deck, that does not change the clip
@@END

@@BOX 236
BF: BF257
FILE-LINE: 59
PAGE-ITEM: [236] I assume Snapshot stays as one small command that saves a still picture of the output to a file, and gets no more work.
PAGE-WAY-b: Snapshot leaves the app.
HIS-WORDS: We keep the safe snapshot somewhere. Earlier in this document, I explained what a snapshot exactly is.
@@END

@@BOX 238
BF: BF258
FILE-LINE: 62
PAGE-ITEM: [238] I assume a new clip in BPM mode gets a number of beats (4, 8, 16, 32 ...) that plays the whole clip at about its normal speed, and its end is never cut.
PAGE-WAY-b: Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed.
HIS-WORDS: b
@@END

@@BOX 239
BF: BF259
FILE-LINE: 65
PAGE-ITEM: [239] I assume we build no codec. First I measure, on two of your clips, how long a clip waits when it jumps; only if you would notice the wait do I add a command that makes HAP copies. Ordinary files keep playing.
PAGE-WAY-b: Plan the command that makes HAP copies now, without timing first.
PAGE-WAY-c: No such command: you make HAP copies yourself with tools you own.
HIS-WORDS: Why would we make HAP copies at all?
@@END

@@BOX 240
BF: BF260
FILE-LINE: 68
PAGE-ITEM: [240] I assume every layer works the same: it is blended in by its blend mode and its transparency slider, the bottom one too. There are no mask layers.
PAGE-WAY-b: Mask layers stay; only the keying list and its slider go.
HIS-WORDS: All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder. We will also add masks and moving masks with which are alpha channels
@@END

@@BOX 241
BF: BF261
FILE-LINE: 71
PAGE-ITEM: [241] I assume an envelope you draw always runs on the beat, never along the clip's playhead.
PAGE-WAY-b: An envelope can also follow the playhead: its drawn shape is stretched over the clip from start to end.
HIS-WORDS: There should be two different types of envelopes that could work. One is along the beat, which is like a signal, but personalized for that clip, and another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode.
@@END

@@BOX 244
BF: BF262
FILE-LINE: 74
PAGE-ITEM: [244] I assume a MIDI knob you map to a clip's slider stays with that clip, wherever the clip goes; a copy of the clip has no knob. A MIDI knob on a layer's or a global slider stays there.
PAGE-WAY-b: The knob belongs to the cell: it moves that slider of whatever clip sits there in the deck on screen.
PAGE-WAY-c: The knob moves that slider of whatever clip is playing on that layer.
HIS-WORDS: b
@@END

@@BOX 245
BF: BF263
FILE-LINE: 77
PAGE-ITEM: [245] I assume the screen where you watch a recording and make actions keeps the name Review, as you wrote, until you pick another.
PAGE-WAY-b: It is called Studio.
PAGE-WAY-c: It is called Replay.
HIS-WORDS: b
@@END

@@BOX 246
BF: BF264
FILE-LINE: 80
PAGE-ITEM: [246] I assume your answer stands: no hold setting. A pad or a key only presses.
PAGE-WAY-b: Every pad and key gets one switch, press or hold, set to press until you change it, so nothing changes for a pad you leave alone.
HIS-WORDS: I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers.
@@END

@@BOX 247
BF: BF265
FILE-LINE: 83
PAGE-ITEM: [247] I assume nothing of MilkDrop changes in the coming build: with no clip playing, a preset you click in the MilkDrop tab still shows on the output.
PAGE-WAY-b: With no clip playing the output stays black: a clicked preset changes the preset but does not show; MilkDrop reaches the output only through a clip that plays.
HIS-WORDS: Keep Milk drop as it is. When I have time while you are building, I will design a whole system for Milk drop
@@END

@@BOX 250
BF: BF266
FILE-LINE: 86
PAGE-ITEM: [250] I assume tapping changes only the tempo number, from the second tap on; it never shifts the beat or the 1.
PAGE-WAY-b: Each tap also pulls the beat onto your tap, but never moves the 1.
HIS-WORDS: What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?
@@END

@@BOX 252
BF: BF267
FILE-LINE: 89
PAGE-ITEM: [252] I assume the output always comes first: the preview may be smaller, less smooth and, in the first build, show trails (Echo, Freeze, Feedback) and MilkDrop a little differently.
HIS-WORDS: What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?
@@END

@@BOX 256
BF: BF268
FILE-LINE: 92
PAGE-ITEM: [256] I assume a clip made with record to clip is the output's full picture, global effects included: black where no layer shows, hiding layers under it.
PAGE-WAY-b: Its empty parts stay see-through, so layers under it show through; likely price, not measured: bigger files and a heavier recording.
HIS-WORDS: This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like.
@@END

@@BOX 265
BF: BF269
FILE-LINE: 95
PAGE-ITEM: [265] I assume All Outputs Off is for the moment only: a show you open afterwards brings its outputs on again.
PAGE-WAY-b: After All Outputs Off nothing comes on until you switch an output on yourself.
HIS-WORDS: b
@@END

@@BOX 270
BF: BF270
FILE-LINE: 98
PAGE-ITEM: [270] I assume MIDI knobs must send their position: an endless knob that only sends steps will not work. Which MIDI controller do you use?
PAGE-WAY-b: Knobs that send steps must work too.
HIS-WORDS: b due to what we're doing endless will be better, but we can assume that people will use one or the other. We need to work with both and know how they both work so they both work smoothly. If we have to, we could ask the user to set a toggle if it's an endless encoder.
@@END

@@BOX 273
BF: BF271
FILE-LINE: 101
PAGE-ITEM: [273] I assume the tempo bar's stop does not stop an audio file the app plays: the music plays on.
PAGE-WAY-b: The stop also stops the audio file.
HIS-WORDS: Stop removes all clips from all layers so it would stop. A pause would not pause it unless it is connected to the BPM.
@@END

@@BOX general
BF: BF272
FILE-LINE: 104
HIS-WORDS: after this round of questions, I will give you 3 10 min audio clips and a longer set to look at. are mp3 and m4a files ok or do prefer a certain format?
@@END

@@EMPTY
ITEMS-ACCEPTED-AS-WRITTEN: 217, 219, 221, 224, 225, 229, 230, 235, 237, 242, 243, 248, 249, 251, 253, 254, 255, 257, 258, 259, 260, 261, 262, 263, 264, 266, 267, 268, 269, 271, 272
@@END
