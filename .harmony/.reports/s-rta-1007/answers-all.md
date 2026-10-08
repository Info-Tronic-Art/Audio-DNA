# THE ANSWERS TO THE QUESTIONS BORIS ASKED HARMONY (ruled @@ANSWER blocks of the topic papers, then the three answer papers' own sections; made by merge.py)

(topic A)
@@ANSWER 189
HIS: L32
ANSWER: Yes, it can be built: you correct the tempo or the "1" and the app stays automatic. Nothing switches to manual by itself. How well it holds on your music is not measured: I can only say after a test on your three DJ tracks, which I do not have yet. If it fails, a correction needs manual mode, and I say so before anything is built. Tempo: you set the number (tap, type, "+", "-"). The app keeps listening, but only close around your number: it follows the music's slow changes and keeps the beats on the music. "/2" and "x2" work the same way. The "1": when you press Resync, that beat is the "1"; the app counts on from it and does not move it by itself.
@@END

(topic D)
@@ANSWER R129-4
HIS: L66
ANSWER: I recommend: loading a preset never stops the action, and the preset is never lost. Example: an effect has sliders A, B and C, and an action is moving A. You load a preset. (1) B and C take the preset's values at once. (2) A keeps moving with the action. (3) The preset's value for A is kept underneath: when the action goes off, or has played once, A glides to the preset's value, not to the value from before the action. The other way, which my page described: the preset's value for A is thrown away, and A goes back to its old value when the action goes off. Tell me if you want that instead.
@@END

(topic E)
@@ANSWER R184
HIS: L87, L91
ANSWER: That sentence was mine, and it is withdrawn. I meant this: in my plan the button that plays a whole recording inside the live window goes away, so I looked for another way to show a whole recorded night and wrote that you could save all of it as one action and fire that. It is not needed. A recording is played in Review; with your "186 b" the outputs show what Review shows, and you hear its sound. Actions stay pieces that you cut from a recording and place on a clip, a layer or global; the save window warns you when a piece is not a multiple of 4 bars. If a recording should also play inside the live window, say so at my assumption about Record Over.
@@END

(topic E)
@@ANSWER lowres
HIS: L6
ANSWER: It should be light on the computer, and I would build it; but nobody has measured it yet, so every number here is an estimate. A film a quarter as wide and as high as the show (480 by 270 for full HD) at 30 pictures a second: about 1 to 5 percent of the processor and about 25 to 150 MB for each 10 minutes. Not known: whether fetching the small picture makes the live picture hitch. Only a test on your Mac with a heavy show tells; it comes before any promise. Pieces of 10 or 20 minutes are not enough alone: a film cut off by a crash cannot be played, so each piece must also be written so that a crash costs only its last seconds.
@@END

(topic F)
@@ANSWER R188-snapshot
HIS: L94
ANSWER: No, not for saving anything: the show holds everything, the layout too, and the app opens your last show. I recommend keeping Snapshot only as one small command that saves a still picture of the output as an image file, for a flyer, a post, or to remember a look. It is never part of a show, it changes nothing in one, and it gets no more work. If you never use it, say "drop snapshot" and the command leaves the app.
@@END

(topic H)
@@ANSWER codec
HIS: L3
ANSWER: No: do not build a codec of our own, and I could not promise to make one reliable. It would play only here and need its own converter and long testing, with no outside player to check its files. DXV stores every picture whole, so a jump, backwards play or a random beat needs no catching up. HAP, a free format with the same purpose, has existed since 2012, and HAP files already open here. Both are also made for the graphics card to do the unpacking; I have not shown that we can use that here. Not measured yet: how long a jump waits on your ordinary clips, how much a fast copy gains, and how much bigger it is (my estimate: much bigger). So: measure first, then decide.
@@END

(topic I)
@@ANSWER R221-b
HIS: L107, L13
ANSWER: Yes, it is a good idea. The logic: such an effect listens to the beat clock, not to the signals. So what holds the clock holds the effect: on pause it stands where it was and goes on in step when you press play; on stop it waits and starts on your new 1. That is your own rule: pause pauses everything the beat controls. A strobe that kept flashing through a pause would be out of step. What only turns the signals down does not touch it: with the Master Signal at 0 the sliders stand still and the strobe keeps its beat; to stop it, switch the effect off. One thing to know: a strobe paused in its dark moment stays dark until you press play.
@@END

(topic J)
@@ANSWER 207-hold
HIS: L120
ANSWER: Yes, I would add it: one switch per pad, press or hold, set to press until you change it. Hold turns a pad into an instrument. With a band you play by hand: the flash, the logo or the effect is there exactly as long as you hold the pad, through a drum fill or a held chord, and gone when you let go. A DJ holds a pad through the build-up and lets go on the drop. With press only, each of these needs a second press that must also be on time; miss it and the flash stays on. One limit: on a BPM mode clip a short hold can end before the 1, and then nothing shows. Say "207 hold" to add it; otherwise c stands.
@@END

(topic J)
@@ANSWER review-name
HIS: L114, L72
ANSWER: My pick: "Studio". Musicians already pair Live and Studio: live is the stage, the studio is where you listen back to what you recorded and make things from it, which is what this screen does: you watch the night again and record your actions there. And it is easy to tell apart when you speak or dictate: "Review" and the cue system's "Preview" differ by one sound and can be mixed up. Alternative 1: "Replay", your own earlier word ("the replay window"); clear, but close to "play" on the tempo bar, and it names only the watching. Alternative 2: "Review", your word now; short and true, if the nearness to "Preview" does not bother you. Answer with the one word you want; until then I write "Review".
@@END

## THE ANSWER PAPER codec (answer-codec.md) -- its two sections for him, as written (the topic ruling may have replaced them: see the @@ANSWER block above with the key codec)
### ANSWER FOR BORIS
No. Building a codec of our own is not worth it, and I would not call it something I can do reliably. Resolume made DXV in 2009; HAP, a free open format with the same purpose, arrived in 2012. HAP stores every frame on its own, so jumping, playing backwards and landing on random beats are instant, and the graphics card can do the unpacking. It can also carry see-through video. The price: files several times bigger than ordinary video, and each file must be converted once. A codec of our own would play only in our app and would need new tools and long testing, for no gain over HAP. My recommendation: later add a "Convert for performance" command that makes HAP files; ordinary files keep playing. Reliability: our own codec, no. HAP, yes, because the format and its decoder already exist and can be checked against known pictures.
### QUESTION FOR HIM
Plan for a "Convert for performance" command that makes HAP files, with ordinary videos still playing as they do?
- DEFAULT: yes, plan it (nothing built until you say all is clear).
- No: leave video as it is.

### THE CHECKER'S REPLACEMENT
No. A codec of our own is not worth building, and I would not call it something I can do reliably. Resolume made DXV in 2009. HAP, a free open format with the same purpose, has existed since 2012. Every HAP picture is stored on its own, so jumps and random beats should be near-instant (not yet measured on your clips). It can carry see-through video, and it is built for the graphics card to do the unpacking, which I have not yet shown we can use. The price is much bigger files (my estimate) and a one-time conversion. A codec of our own would play only here and needs new tools and long testing. HAP, or a simple all-pictures-stored format, is reliable because both already exist and can be checked.
(117 words)

## THE ANSWER PAPER lowres (answer-lowres-rec.md) -- its two sections for him, as written (the topic ruling may have replaced them: see the @@ANSWER block above with the key lowres)
### ANSWER FOR BORIS
It costs very little. Every number below is an ESTIMATE, not a measurement.
- A quarter of each side (480 x 270) at 30 pictures a second: about 1-3% of the processor, under 1% of the graphics card, about 25-75 MB per 10 minutes.
- The big reading, a quarter of the area (960 x 540) at 60 a second: about 3-10% of the processor, about 120-380 MB per 10 minutes.
- An eighth is smaller still (240 x 135: 6-24 MB per 10 minutes).
Cutting into 10 or 20 minute pieces is not enough alone: a video file that is not closed properly cannot be played. It has to be written so a crash loses only the last few seconds. Then both ideas work together.
I recommend 480 x 270 at 30 a second. Only a test on your Mac with a show running gives a number I can promise.
Decided without asking you: this is a size choice inside the Video box.
### QUESTION FOR HIM
Should the small recording carry the sound?
- (default) No. The Audio box already keeps the sound at full quality, separately.
- Yes, add sound to the small file: about 10 MB more per 10 minutes.
- Both: a choice on the box.

### THE CHECKER'S REPLACEMENT
It should be light on the computer, but nothing is measured yet. Every number is an estimate.
- 480 x 270, 30 pictures a second: about 1-3% of the processor, 25-100 MB per 10 minutes.
- 960 x 540, 60 a second: about 3-12% of the processor, 120-400 MB per 10 minutes.
- 240 x 135: under 25 MB per 10 minutes.
Not known: whether the small picture causes a tiny hitch while you perform. Only a test on your Mac shows that.
Pieces of 10 or 20 minutes are not enough alone: a video cut off by a crash cannot be played, so each piece must also be written to lose only the last seconds.
I suggest 480 x 270.
Decided without asking you: it is a size choice in the Video box.
Question: did you mean a quarter of each side, or of the area?

## THE ANSWER PAPER tempo-auto (answer-tempo-auto.md) -- its two sections for him, as written (the topic ruling may have replaced them: see the @@ANSWER block above with the key 189)
### ANSWER FOR BORIS
Yes to both. Your correction can hold while the app stays on automatic; it does not have to go to manual. It needs a change to the listening, and I can only promise it after it is measured on your three tracks.

Tempo: your number becomes the centre. The app keeps listening, but only close around your number, and uses what it hears to keep the beat on the kick. "/2" and "x2" work the same way: the listening carries on at half or double.

The "1": your Resync says which beat is the 1. The app counts on from your 1, and the listening only keeps the edge of each beat on the kick; it stops guessing the 1 itself. The nudge moves it a hair.

I recommend exactly this. Manual stays as your own switch for when the listening is no use.
### QUESTION FOR HIM
Your corrected tempo is holding in automatic. The music then moves to a clearly different tempo (a new track that was not beat-matched). What should the app do?
- A (DEFAULT): it lets go of your number by itself and follows the music again; your "1" keeps counting until you press Resync.
- B: it keeps your number until you change it yourself.

### THE CHECKER'S REPLACEMENT
Yes, I can build it so your correction holds and the app stays on automatic. Whether it works on your music I can only say after I test it on your three DJ tracks, which I do not have yet.

Tempo: you set the number (tap, or type it). The app keeps listening but only accepts tempos very close to yours, so it follows a slow pitch change and ignores the rest. "/2" and "x2" work the same way.

The 1: when you press Resync, that beat is the 1, and the app counts on from it. It no longer guesses the 1 itself. The nudge moves it a hair.

Manual stays as your own switch.

Decided without asking you, say so if one is wrong: Tap, Resync and the tempo buttons will not switch Manual on by themselves.

(about 125 words)

Keep the paper's QUESTION FOR HIM as it is. It is one focused question, not a reading.

Written (system clock): see date line below.
Wed Oct  7 23:03:19 EDT 2026

