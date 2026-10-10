# PAGE 3 -- the list, round 1 (s-rta-1009)

Made by the page ruling across topics A to K from assume3-all.md, answers3-all.md and his 33 boxes. Nothing built, run or measured. Provisional ids P1 ... ; a script gives the final numbers from 274.

## THE ANSWERS (what he asked or ordered in this round)

@@PAGE-ANSWER 222
ASKED: "what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?"
TITLE: Master cue: what it is and where
TEXT: Master cue is one on / off button. I put it with the layers' cue buttons, under the preview monitor, and it has no picture of its own: it changes what the preview monitor shows. The name and the button are yours, from 7 October: "177 add a global effects and actions toggle button called master cue". What it does is my guess. Off: the preview monitor shows the layers you cue, mixed, without the global effects. On: the same mix goes through the global effects, as the global actions move them, so you see it as it would look on the output. None of the cue buttons is built yet, which is why you cannot find one in the app.
ITEM: P5
@@END

@@PAGE-ANSWER 250
ASKED: "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?"
TITLE: What "shifting the beat" meant
TEXT: Yes: I meant sliding all the beats, the 1 with them, a little forward or backwards in time, so that a beat falls exactly where you tap. It is like a nudge: never more than half a beat, and it never makes another beat the 1. The beat has three things: how fast it runs (the tempo number), where the beats fall in time, and which beat is the 1. Tapping always sets how fast. The only open point is whether tapping also moves where the beats fall. I assumed it does not: you place the beats with Resync and the nudge, and in automatic mode the app is meant to keep them on the music by itself (not measured yet).
ITEM: P2
@@END

@@PAGE-ANSWER 252
ASKED: "What do you mean by come first?" and "should all run at the same fps, no?"
TITLE: The same fps everywhere, and "come first"
TEXT: Yes: they should all run at the same fps, and I will build it so. The output monitor and the preview monitor are drawn together, picture by picture, so the two cannot run apart; the output screens show the same pictures. You are right about smaller and less smooth: I take both back. A layer you cue should look exactly as on the output, trails included. By "come first" I meant two things. A preview never changes what the audience sees. And if the computer cannot keep up, the preview stops on a still marked "paused" while the output carries on; a short hitch may come before that. Nothing of this is measured: I measure it with you on a heavy show of yours before the preview is built.
ITEM: P6
@@END

@@PAGE-ANSWER 239
ASKED: "Why would we make HAP copies at all?" and "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"
TITLE: HAP copies, and your codec test
TEXT: Only if your clips are seen to wait; if nothing waits, never. Most ordinary video files store a few pictures whole and the others only as changes from the picture before. So on a jump, on backwards play or on a random beat the app must first rebuild the picture, which can show as a short wait. A HAP file stores every picture whole, as Resolume's DXV does, so nothing is rebuilt; its price is a much bigger file (my estimate). And yes to your test: once the app is built I run it on this Mac with your own clips, playing and recording, and you get one short list of the codecs that work well here. It also shows whether your clips wait at all. No numbers before that.
ITEM: P22
@@END

@@PAGE-ANSWER lowres
ASKED: "Maybe something close to the output monitor resolution? 30 fps is the fastest or even 15 could be ok."
TITLE: The low-resolution show recording: size and rate
TEXT: Yes to the test, the rate and the purpose. Its codec comes out of your test of codecs after the build. It runs at 30 pictures a second, and at 15 if the test shows that 30 is heavy. A very HD film is made afterwards with Render in Studio. The size: I take "the output monitor" to be the one inside the app, not the projector, so the film stays small. I start at a quarter as wide and as high as the show (480 by 270 for full HD) and make it larger if that is too small to judge by. None of this is measured yet: if a test shows that it makes the live picture stutter, I tell you before anything is decided.
ITEM: P13, P14
@@END

@@PAGE-ANSWER 246-hold
ASKED: "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers."
TITLE: The hold setting: what people do
TEXT: Yes, I think it is worth doing, in its smallest form. DJs hold buttons as a matter of course: AlphaTheta (Pioneer) calls it Gate Cue, where a hot cue plays only while the pad is held, and Denon calls it Momentary. For bands, the big lighting desks (grandMA, ChamSys, Avolites) have a Flash or Bump button that is on only while held, used for hits, strobes and blinders; your Resolume calls it Piano and offers it per clip and per shortcut. DJ / producers have it in their music tools (Gate in Ableton and Traktor), but the few who described their own visuals let Ableton trigger them; I found none who holds a pad for visuals, which does not prove they would not. Nobody has numbers on how many use it. I recommend one press-or-hold switch per pad and key, set to press. [Resolume](https://resolume.com/support/en/keyboard-shortcuts) [AlphaTheta](https://downloads.support.alphatheta.com/manuals/all-in-one-dj-systems/XDJ-AZ/html/en/000COV_en/Using_the_Performance_Pads/Using_the_Performance_Pads.htm) [ChamSys](https://docs.chamsys.co.uk/magicq/1.9.9.x/manual/playback.html)
ITEM: P26
@@END

@@PAGE-ANSWER general
ASKED: "are mp3 and m4a files ok or do prefer a certain format?"
TITLE: MP3 and M4A are fine
TEXT: Yes, MP3 and M4A are both fine: send the files as you get them, and do not convert anything yourself. If the DJ can give you WAV or AIFF just as easily, take those: they are the safest. One honest note: I have not played an MP3 or an M4A in the app yet. If one does not open, I make a WAV copy of it myself and your files stay untouched. I take these to be the tracks for measuring how well the app finds the tempo and the 1 on your music, before anything about automatic mode is promised. Say so if you meant them for something else.
ITEM: none
@@END

## THE ITEMS (by topic A to K; inside a topic the ASK items first)

### Topic A -- Triggering clips and the tempo bar

@@PAGE-ITEM P1
TOPIC: A
KIND: ASK
TEXT: I assume in automatic mode the app places the 1 whenever it is sure, so the press that starts the beat is the 1 only until then. Your Resync holds: until the music clearly changes (a new track, not beat-matched) or you pause or stop.
B: After your first Resync the 1 is yours for the rest of the night: the app never places it again.
C: The app may move the 1 whenever it is sure, in the middle of a track too, also after your Resync.
FROM: A3-1 (carries his newest words on the start press: "app should always try to find the 1", before "click on play or any clip is the new 1")
@@END

@@PAGE-ITEM P2
TOPIC: A
KIND: ASK
TEXT: I assume tapping only sets how fast the beat runs (the tempo number): it does not move the 1, or any other beat, forward or backwards in time. Resync and the nudge do that.
B: Each tap also moves the beats, the 1 with them, a little forward or backwards, so that a beat falls on your tap; which beat is the 1 never changes.
C: none
FROM: A-2; again: 250
@@END

### Topic B -- The cue system

@@PAGE-ITEM P5
TOPIC: B
KIND: ASK
TEXT: I assume the master cue button, switched on, shows the layers you cue in the preview monitor with the global effects on them, moved by any global action that runs, as on the output; switched off, without them. It never changes the output.
B: It also lets you switch a global effect or action on for the preview monitor only, to try it before the audience sees it.
C: Switched on, the preview monitor shows the whole output, as on a DJ mixer.
FROM: B3-1; again: 222
@@END

@@PAGE-ITEM P6
TOPIC: B
KIND: ASK
TEXT: I assume the output monitor and the preview monitor run at the same fps, and a layer you cue looks exactly as on the output. If the computer cannot keep up, the preview stops on a still picture marked "paused" and the output keeps going.
B: The preview is always drawn smaller, so it rarely has to stop; price: a few effects look slightly different in it than on the output.
C: none
FROM: B3-2; again: 252
@@END

@@PAGE-ITEM P7
TOPIC: B
KIND: LINE
TEXT: I assume a previewed clip's actions start with it at your click: they run at the tempo's BPM but are not lined up with the 1.
B: The picture starts at once, and its actions join on the next 1.
C: none
FROM: B3-3 (his newest words, BF247, take the wait for the beat out of previewing; before, BF175: "it is triggered on the 1")
@@END

@@PAGE-ITEM P8
TOPIC: B
KIND: LINE
TEXT: I assume "loaded into the layer" and "cueing" mean a clip you triggered on a layer, seen through that layer's cue button: it is in time because you triggered it.
B: With a layer's cue button on, a click on a clip's name in that layer puts the clip into the cue on the next 1, without it reaching the output.
C: none
FROM: B3-4
@@END

### Topic C -- Presets

@@PAGE-ITEM P9
TOPIC: C
KIND: LINE
TEXT: I assume the window about two versions of a preset comes only for a show from another computer; for an older show saved here, this computer's version stays without asking.
B: The window asks every time the two differ, also for a show saved on this computer.
C: none
FROM: C3-1
@@END

### Topic D -- Actions in a show

@@PAGE-ITEM P10
TOPIC: D
KIND: ASK
TEXT: I assume the tempo stop and the Stop actions button stop every action. Layer and global action buttons go off; a clip's action buttons stay as set, so its actions play again when that clip is next triggered. Which is it: this, b or c?
B: Every action's button goes off, also on every clip of the show, playing or not; after a stop you switch on again the ones you want.
C: At the tempo stop no button goes off: every action waits and starts again from its beginning when the beat next runs.
FROM: D3-1 (ends in a question, because his words pull two ways; the page ruling made his own words on a clip's action button the text and the topic ruling's widest reading way b)
@@END

@@PAGE-ITEM P11
TOPIC: D
KIND: ASK
TEXT: I assume a layer's Clear button (the X) takes the clip off and also stops that layer's own actions: their buttons go off, as at the tempo stop. A clip that ejects, or an empty cell you trigger, leaves them on.
B: Clear takes only the clip off: the layer's own actions keep playing, and one that triggers clips puts a clip on that layer again at its next trigger.
C: none
FROM: D3-5
@@END

@@PAGE-ITEM P12
TOPIC: D
KIND: LINE
TEXT: I assume a clip's action that plays once, switched on while its clip is playing, waits for that clip's next trigger; switching it off and on does not play it.
B: Switching it on while its clip plays also plays it once, from the next 1.
C: none
FROM: D3-3
@@END

### Topic E -- Studio and the recordings

@@PAGE-ITEM P13
TOPIC: E
KIND: ASK
TEXT: I assume Studio can play the low-resolution show recording together with its own picture, both always at the same moment of the show, also while you scrub, so that a mistake in the parameter recording shows at once.
B: Studio does not show it: the pieces are plain film files that you open in any player and compare by eye.
C: none
FROM: E-23
@@END

@@PAGE-ITEM P14
TOPIC: E
KIND: LINE
TEXT: I assume "the output monitor" means the monitor inside the app, not the projector: the low-resolution show recording stays small, about a quarter as wide and high as the show.
B: You mean the projector's own size: then it is a full-size film, heavier, and no longer small.
C: none
FROM: E3-1; again: 262
@@END

@@PAGE-ITEM P15
TOPIC: E
KIND: LINE
TEXT: I assume opening Studio stops everything, like the tempo stop, but switches no action's button off: when you close Studio the show is as it was, with nothing playing.
B: Opening Studio switches the buttons of layer actions and global actions off, as the tempo stop does.
C: none
FROM: E3-6
@@END

### Topic F -- The show file, decks and saving

@@PAGE-ITEM P16
TOPIC: F
KIND: ASK
TEXT: I assume a snapshot, saved apart from the show, keeps a picture of the output and what was playing. Opening one replaces the open show (after asking to save it) and brings the clips back on their layers where they stood, with the tempo paused.
B: It opens like any show, with the layers empty; you trigger the clips yourself.
C: It never replaces the open show: you only look at its picture and take clips or decks from it.
FROM: F3-6 (carries his newest words on a save apart from the show's Save: BF243, before "There's no reason to save them separately")
@@END

@@PAGE-ITEM P17
TOPIC: F
KIND: ASK
TEXT: I assume a click on a clip, anywhere but its name, triggers it and also selects it, and a key or a pad on its cell does the same. So Delete then takes the clip you just triggered off its layer, without asking.
B: A key or a pad only triggers: what is selected stays as it was.
C: Only an empty cell is selected by the click that triggers it; a clip is selected by a click on its name only.
FROM: F3-1, J3-10 (and the internal J-19: Delete never asks first)
@@END

@@PAGE-ITEM P18
TOPIC: F
KIND: LINE
TEXT: I assume a playing clip leaves its layer at once when you paste over, cut or delete it, and also when you remove its column or its deck.
B: A clip whose column or deck is removed plays on until you trigger something else on its layer.
C: none
FROM: F3-3, F3-4
@@END

@@PAGE-ITEM P19
TOPIC: F
KIND: LINE
TEXT: I assume a snapshot does not bring back the MilkDrop preset that was showing, as MilkDrop stays as it is; its picture shows how the moment looked.
B: A snapshot remembers the MilkDrop preset and loads it when it is opened: a small change to MilkDrop in the coming build (an estimate).
C: none
FROM: K3-8
@@END

### Topic G -- Output screens

@@PAGE-ITEM P20
TOPIC: G
KIND: ASK
TEXT: I assume All Outputs Off lasts until you switch an output on yourself, over a quit too: the next start opens your last show with no output on. Once you switch one on, or press Restore Last Outputs, shows bring their outputs on again.
B: It ends when you quit: the next start of the app brings your last show's outputs on by themselves.
C: Each output stays off until you switch that very output on yourself, whatever show you open.
FROM: G3-2, G3-1 (carries his newest words: "b" on All Outputs Off, before "I expect the show to remember the outputs connected and not need to connect them again.")
@@END

### Topic H -- How a clip plays

@@PAGE-ITEM P21
TOPIC: H
KIND: ASK
TEXT: I assume a clip's end is cut the first time you switch it to BPM mode: up to 8 seconds are lost, so a loop made without a seam can jump there. In Timeline mode it stays cut until you drag its out point out.
B: The cut holds only in BPM mode: Timeline mode always plays the whole clip.
C: none
FROM: H3-1
@@END

@@PAGE-ITEM P22
TOPIC: H
KIND: LINE
TEXT: I assume we make no HAP copies and build no command for them; I ask again only if your test of codecs after the build shows that your clips wait.
B: Never, whatever the test shows: your clips play as they are.
C: none
FROM: H3-2; again: 239
@@END

### Topic I -- Effects, signals and what moves a slider by itself

@@PAGE-ITEM P23
TOPIC: I
KIND: ASK
TEXT: I assume your "adjust the sink" means the sync: a strobe, a pulse and effects like them run locked to the beat, and each gets a Sync slider that sets how fast it pulses: every bar, every beat, every half beat and so on.
B: Sync is a switch on each of them: on, its speed is locked to the beat; off, it runs free at the speed you set.
C: Sync shifts the effect earlier or later against the beat, so its flash lands where you want it (how you used the word sync before).
FROM: I3-6
@@END

@@PAGE-ITEM P24
TOPIC: I
KIND: ASK
TEXT: I assume an envelope you draw for a clip is that clip's own: saved and copied with the clip, and only that clip's sliders and buttons use it. A preset keeps the slider's value but does not carry the envelope to another clip.
B: A preset carries the envelope's shape too: loaded on another clip, it makes a copy of the envelope there and plugs it in.
C: Every envelope is in one list for the whole show, and any clip, layer or Global slider can use any of them.
FROM: I3-4, C3-6
@@END

@@PAGE-ITEM P25
TOPIC: I
KIND: LINE
TEXT: I assume your newest words hold: all the blend modes and the keying, with its slider, stay and are built later (before: remove the keying and its slider).
B: The keying list stays, but its slider goes.
C: none
FROM: newest words: BF260 against his "204 I want to remove the keying and slider." of 2026-10-07 (binding-decisions.md:1185); asked for by the rulings of topics I and X
@@END

@@PAGE-ITEM P31
TOPIC: I
KIND: LINE
TEXT: I assume masks are built last, before the UI redesign, with the blend modes and the keying; what a mask does I ask you when that part is planned.
B: Masks are built in the coming build; I then ask you what a mask does on the next page, before anything is built.
C: none
FROM: I3-2
@@END

### Topic J -- The keyboard and MIDI mapping, menus and messages

@@PAGE-ITEM P26
TOPIC: J
KIND: ASK
TEXT: I assume your answer stands until you say otherwise: no hold setting. A pad or a key only presses, and letting go does nothing.
B: Every pad and key gets one switch, press or hold, set to press: on hold, what it switches is on only while you hold it (my recommendation).
C: The same switch, but only for an effect's button and an action; a clip's pad always only presses.
FROM: J3-hold; again: 246
@@END

@@PAGE-ITEM P27
TOPIC: J
KIND: ASK
TEXT: I assume that when a slider stands somewhere else than its knob (an action, a preset or another clip in the cell moved it), a knob that sends its position makes it jump to the knob at your next turn. An endless knob never jumps.
B: No jump: the knob does nothing until it passes the slider's value, and the slider follows it from there.
C: No jump: the slider moves from where it stands, faster or slower than the knob, until the two meet.
FROM: J3-8 (carries his newest words on knobs: both kinds work, BF270, before "207 c")
@@END

@@PAGE-ITEM P28
TOPIC: J
KIND: LINE
TEXT: I assume after a deck switch a knob on a clip's slider moves the clip in that cell of the new deck, not the old deck's clip that still plays.
B: While a clip from that cell of another deck is still playing, the knob stays with it.
C: none
FROM: J3-2
@@END

### Topic K -- Sources and the automatic features

@@PAGE-ITEM P29
TOPIC: K
KIND: ASK
TEXT: I assume an audio file the app plays is the music the app listens to, with its own play control: not a clip on a layer, and nothing connects it to the BPM. So tempo stop stops it, and pausing the tempo never pauses it.
B: An audio file can also be a clip in a cell, with Timeline mode and BPM mode like a video; in BPM mode a pause of the tempo pauses it.
C: It is not a clip, but one switch connects it to the BPM: then pausing the tempo pauses the music, and tempo play carries it on.
FROM: K3-1
@@END

@@PAGE-ITEM P30
TOPIC: K
KIND: LINE
TEXT: I assume after tempo stop the audio file is back at its beginning and silent until you start it yourself; tempo play or a triggered clip does not start it.
B: The audio file starts again together with the beat: on tempo play or on the first clip you trigger.
C: none
FROM: K3-2
@@END

## THE TRIAGE (one block for every assumption of assume3-all.md whose ASK is YES or LINE: 41)

@@TRIAGE A3-1
TO: P1
WHY: His letter "b" and his own word "always" (BF246) pull two ways on who holds the 1 after a Resync, he sees it in every set, and the item also tells him once that the start press is the 1 only until the app is sure.
@@END

@@TRIAGE A-2
TO: P2
WHY: He asked back (BF266), so the item of page 2 returns under a new id in his own words ("moving the 1 forward or backwards in time"), with the answer on "shifting the beat" above it.
@@END

@@TRIAGE A3-2
TO: SETTLED
WHY: His own rules give the cut its kind, so nothing is left to ask: "124 a" (binding-decisions.md:946: a clip out of time carries on and cuts into time on the next 1) and "121 a" (binding-decisions.md:943: his own Resync cuts at once).
@@END

@@TRIAGE A3-3
TO: SETTLED
WHY: The first page said it to him in words among the readings that stand (the page's words: "stop is a cut, the clips go at once, no fade"), inside the reading he named and corrected in one other point only: "R147 one small correction I do want to have a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (BF194, binding-decisions.md:1153).
@@END

@@TRIAGE B3-1
TO: P5
WHY: He asked what and where master cue is (BF248); no word of his says what the button does, and way b is a far bigger build.
@@END

@@TRIAGE B3-2
TO: P6
WHY: He asked back and handed over the technical call (BF267); the item says what is built (same fps, a cued layer exactly as on the output) and the one thing he would see, the preview stopping when the computer cannot keep up.
@@END

@@TRIAGE B3-3
TO: P7
WHY: BF247 takes the wait for the beat out of previewing and does not name the actions; the line also tells him once that his newest words go against "it is triggered on the 1" (BF175).
@@END

@@TRIAGE B3-4
TO: P8
WHY: It reads his newest sentence (BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time") as nothing new to build, and the other reading is a large addition, so he sees the reading.
@@END

@@TRIAGE B-11
TO: INTERNAL
WHY: A narrow technical limit of the first build, seen in the preview only and marked there; its MilkDrop half follows his "Keep Milk drop as it is." (BF265), and the other ways are a look or a second MilkDrop he has not asked for.
@@END

@@TRIAGE C3-6
TO: P24
WHY: Whether a preset carries a clip's envelope is one decision with who owns an envelope (I3-4): asked once, in the topic that owns the envelope, with "the preset carries the shape" as way b.
@@END

@@TRIAGE C3-1
TO: P9
WHY: His "b" (BF249) was given for a preset changed on another computer; that no window comes for an older show of this same computer narrows his answer, so he gets one line.
@@END

@@TRIAGE C3-2
TO: SETTLED
WHY: His "b" (BF249) is the page's way "A small window asks which one to keep": one of the two is kept, so the other goes; that the show opens without waiting for the window is a technical order of two steps.
@@END

@@TRIAGE D3-1
TO: P10
WHY: BF250 "tempo stop stops all actions, not just global" gives the two stops their reach, not what they do to a clip's buttons, and what he has read and written pulls two ways (BF251 "It's button stays on"; the first page's reading that Stop actions switches every action off), so the item ends in a question that silence cannot accept, with his own words on a clip's button as the text and the topic ruling's widest reading as way b.
@@END

@@TRIAGE D3-5
TO: P11
WHY: No word of his says what the Clear button does to a layer's own actions; the first page and his "143 a" point two ways, and the audience sees a cleared layer fill again or stay empty.
@@END

@@TRIAGE D3-2
TO: SETTLED
WHY: BF251 "neither. It's button stays on and that action plays again only when that clip is re-triggered" rejects the two other ways and speaks of "that clip" only, so the sentence he read for a layer or global (plays once, its button then goes off by itself) stands.
@@END

@@TRIAGE D3-3
TO: P12
WHY: His "only" (BF251) is taken one step further, to a first switching-on under a playing clip that his sentence does not name; he would see the one-time move come at the next bar or not at all.
@@END

@@TRIAGE E-23
TO: P13
WHY: BF253 gives the low-resolution show recording its purpose ("so that we can see if there is a mistake in the parameter recording"), and whether Studio serves that purpose is a function he would look for there.
@@END

@@TRIAGE E3-1
TO: P14
WHY: His box asks "Maybe something close to the output monitor resolution?" (BF241) while he left the quarter of item 262 standing; the line under the answer says which monitor is meant, and a wrong size is one number.
@@END

@@TRIAGE E3-3
TO: SETTLED
WHY: His answer "48 b" (binding-decisions.md:867) gave Record Show its three boxes, Video among them, and BF241 names Render as a further way to a full-quality film without taking the box away; nothing is removed, so nothing is asked again.
@@END

@@TRIAGE E3-6
TO: P15
WHY: Once a tempo stop switches action buttons off, the two sentences of the reading he left standing (opening Studio "is like a press on stop"; closing it brings back "the show as it was") cannot both hold, and he would see the difference each time he comes back from Studio.
@@END

@@TRIAGE F3-6
TO: P16
WHY: BF243 ("a save of the show exactly where it is", "save that to look out later") does not say whether what plays is kept or what opening a snapshot does to the open show, and the three ways are three different builds.
@@END

@@TRIAGE F3-1
TO: P17
WHY: BF255 "you can select the cell which triggers it and selects it" was written under an item about empty cells; applied to a clip it gives the Delete key a new reach that is seen on stage, so it is a card, with the key and the pad (J3-10) in the same sentence.
@@END

@@TRIAGE F3-3
TO: P18
WHY: BF256 "removes it from the layer strip, and it does not play" settles that the clip leaves and that the pasted clip waits; the moment ("at once", also under a clip in BPM mode) is said in the one line that also carries the column and the deck.
@@END

@@TRIAGE F3-4
TO: P18
WHY: It carries his words on a deleted clip (BF256) over to a removed column and a removed deck, against two defaults he had let stand, so he gets the line and can strike it.
@@END

@@TRIAGE G3-2
TO: P20
WHY: His newest "b" (BF269) and his earlier "I expect the show to remember the outputs connected and not need to connect them again." (BF216) pull apart exactly at the quit, and he sees it at set-up before a show.
@@END

@@TRIAGE G3-1
TO: P20
WHY: When All Outputs Off is over is one decision with whether it lasts over a quit: both come from his "b" (BF269), so the stricter reading (each output until that very one is switched on) is way c of the same card.
@@END

@@TRIAGE H3-2
TO: P22
WHY: He asked back (BF259), the answer above says why a copy could ever be needed, and the default costs him nothing now (no copies, no command), so it is one line he can strike and not a card.
@@END

@@TRIAGE H3-1
TO: P21
WHY: BF258 "b" picks the cut but not its moment, and what the cut costs (up to 8 seconds of a clip's end; a jump in a loop made without a seam) never stood on his page; it is seen on stage, so it is a card.
@@END

@@TRIAGE I3-2
TO: P31
WHY: BF260 names masks without saying what a mask does; masks are built at the last stage and nothing of them is built before his answer, so the page tells him in one line that the question comes when that part is planned.
@@END

@@TRIAGE I3-6
TO: P23
WHY: BF244 "adjust the sink" is a dictated word that can be read three ways, and it decides a control he uses live in the coming build.
@@END

@@TRIAGE I3-4
TO: P24
WHY: BF261 "personalized for that clip" is read as: the envelope is the clip's own; it is one decision with what a preset carries of it (C3-6), so both are one card.
@@END

@@TRIAGE I3-5
TO: SETTLED
WHY: His own words name the place: BF261 "so we can draw a envelope that happens over the course of the playing of that clip in timeline mode"; offering it on a clip in BPM mode too is Harmony's advice, small to add later, and not put to him.
@@END

@@TRIAGE I3-7
TO: SETTLED
WHY: His own words make it a maybe for later: BF244 "maybe we could use a common macro that we could set later"; each such effect's Sync can be turned by a macro knob of its own clip, layer or Global, and one knob across the whole show is added when he asks.
@@END

@@TRIAGE J3-8
TO: P27
WHY: BF270 "so they both work smoothly" can be read as no jump, the jump was only in a reading he left standing, and the audience sees a slider jump.
@@END

@@TRIAGE J3-hold
TO: P26
WHY: He ordered the research (BF264) and has not taken back his "207 c"; his answer is the text and the research's recommendation is way b, with the answer on the hold setting above it.
@@END

@@TRIAGE J3-2
TO: P28
WHY: It is the wording of the way he chose (BF262 "b": the deck on screen), but beside BF256 ("If a clip is playing, and I change the deck, that does not change the clip") it is the consequence most likely to surprise him on stage.
@@END

@@TRIAGE J3-10
TO: P17
WHY: Whether a key or a pad that triggers a cell also selects it is one decision with the click that triggers and selects (F3-1): it is the clause "a key or a pad on its cell does the same", with "a key or a pad only triggers" as way b.
@@END

@@TRIAGE K3-1
TO: P29
WHY: BF271 "A pause would not pause it unless it is connected to the BPM." does not say what would connect an audio file to the BPM, he writes "audio clips" (BF272), and an audio file as a clip in a cell would be a new kind of clip.
@@END

@@TRIAGE K3-2
TO: P30
WHY: BF271 says the stop stops the audio file ("so it would stop"), not where it stands afterwards or what starts it again; silence or music after a stop is heard by the room.
@@END

@@TRIAGE K3-5
TO: SETTLED
WHY: Default A of question 211, which he took with "All defaults good except for these." (BF154, binding-decisions.md:1105), keeps a clip of many pictures spreading all its pictures; his "b" on the cut (BF258) chose a clip that plays at its normal speed, which such a clip has not.
@@END

@@TRIAGE K3-8
TO: P19
WHY: His two answers of this round meet here and neither names the other (BF243 "exactly where it is"; BF265 "Keep Milk drop as it is."); it is a limit of the snapshot he should know before he relies on it.
@@END

## TO SAY IN CHAT
1. Not measured, and measured before anything about automatic mode is promised: how well the app finds the tempo and the 1 on his music (items 217 and 218 of page 2; P1). It is measured on the audio he brings after this round (BF272); he is told before anything is built if a correction needs manual mode or the app's 1 cannot be trusted.
2. Waits for after the build, by his own words (BF240 "after we build the app"; BF241 "we should test which codecs work best"): the test of codecs -- whether his clips wait at a jump, which codecs his clips are in (nobody has looked), whether DXV files open, the codec of the low-resolution show recording, 30 or 15 pictures a second.
3. Not measured, owed before the cue is built (it needs his app running with a heavy show and nothing new built, so it is scheduled with him): the time per picture of his heaviest show, the cost of master cue and of one previewed clip, the time from a click on a name to the first picture; and whether the low-resolution show recording makes the live picture stutter (his "It defaults to running", BF253, is kept under that test, and the answer on that recording says so).
4. Not tried: an MP3 or an M4A in the app (read, not run). The answer says so; open one of each when the build hold ends.
5. P10 is the one item that ends in a question, so his page does not take silence for it. The page ruling also turned topic D's default: the text is "a clip's action buttons stay as set" (BF251 "It's button stays on"; his words of 2026-10-04, binding-decisions.md:980-981), and topic D's widest reading is way b. One edit block swaps them if Harmony rules otherwise.
6. Found by the page ruling in the first page's own text (.harmony/.reports/s-rta-1005/boris-clarify-all.md:290): "stop is a cut, the clips go at once, no fade" stood there in R147, the reading he named and corrected in one other point (BF194). So A3-3 is SETTLED and not on the page; topic A's ruling had taken it for Harmony's pick.
7. Not put to him again, by the page ruling: the nudge that goes to 0 at every start (item 219: he accepted it with the old way shown as way b, and "that nudge is history" is his own, BF167) and the preview that plays on through a tempo pause (item 221, accepted). The rulings of topics A and X asked for one line on the nudge; its price, if it should be said in chat: a nudge saved with a show is gone at his first press.
8. Kept off the page or put off: what a mask does (P31 tells him it is asked when masks are planned); the preview's two limits (B-11, internal, marked on screen); which MIDI controller he uses is still unknown, and how endless knobs send their steps is a Researcher task before the mapping is built (BF270); still owed by Harmony before "all clear", none of it for him: the Preferences window, the store of recorded sound and the remote-control port (topic X's note).

## COUNTS
- answers: 7 (his two boxes on HAP copies and on the test of codecs are ONE answer, under the key 239; the lint notes the missing key "codec" as merged)
- ASK items: 16 (one of them, P10, ends in a question he must answer)
- LINE items: 13
- triage blocks: 41 of 41 (32 point to an item; 8 SETTLED by words of his: A3-2, A3-3, C3-2, D3-2, E3-3, I3-5, I3-7, K3-5; 1 INTERNAL: B-11)
- the provisional ids run P1 to P31 without P3 and P4: both were written and then withdrawn by the page ruling (chat lines 6 and 7); 29 items in all
- two items stand without a triage block on purpose: P25 (his newest words on the keying against his "204" of 2026-10-07) is the one "newest words" line; every other clash of his newest words with earlier ones is carried inside the item that owns it (P1 the start press, P16 the save apart from the show, P18 the edit of a playing clip, P20 All Outputs Off, P27 both kinds of knob)
- his reading time, my estimate: about 20 minutes (the lint counts about 2,930 words: 15 minutes at 200 words a minute, 23 at 130), a few minutes more where he types an answer
- nothing was built, run, launched or measured for this list; every fact about the app in it is read, not run
Written (system clock): Fri Oct  9 20:40:52 EDT 2026
