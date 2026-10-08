# SLICE A -- Firing clips and the tempo row
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
172 173 174 175 189 190 191 R128 R168 R139 R140 R142 R163 R169 R164 R174 R175 R176 R214 R215 R216 R177 R178 R205 R206 D1 D2 D3 D4 D5 D6 P1 P2

## QUESTIONS (7)
### Question 172 -- How a show starts: pause, load the clips, play   [his lines: L30]
About: R111 and the show start
Situation: A show is about to start. The beat is stopped and the layers are empty. You want three BPM-mode clips, one on each of three layers, to start together on the first beat of the music. A click on a column does that in any case. This is about the other way, your "load up the clips and press start".
- A (THE DEFAULT) [your words + mine]: You press pause first: the beat is paused at its start. Each clip you fire now shows on its first frame and waits. You press play on the first beat of the music: that press is the "1" and all three start.  -- why the default: [your words + mine] It is your two sentences read together: stopped, a fire starts the beat; paused, a clip you fire shows and does not play. So "load up the clips and press start" is the paused way. Mine: that pause can be pressed on a stopped beat and holds it at its start; the app has no stopped or paused beat today.
- B: There is no paused start: from stop, the first clip you fire starts the beat and plays. "load up the clips" then means filling the clip grid, and "press start" starts the beat with nothing playing yet.
Why it was asked: It decides how every show of yours begins, and the tempo row is built on it. Whether a clip that is not in BPM mode would also wait under A is question 174. Either way you can also start with the faders down and bring them up (R149).
His earlier words it rested on: "R111 if I fire a clip while the beat is stopped, it starts playing the beat. When paused, it will not play but will still display." / "The way to set up / cue up for a show start is to load up the clips and press start, or click on a column to trigger all of them at the same time and cue them ahead of time via the cue system which I am about to describe."
Source (Harmony-side): BD:1091, BD:1092; facts-takes-bpm-fire.md 5.7-5.11

### Question 173 -- What the layer shows while a BPM-mode clip waits   [NOT NAMED by him]
About: 154: what the layer shows while a clip waits
Situation: The beat is running. A clip plays on layer 2. Half-way through a bar you fire another clip of layer 2 that is in BPM mode.
- A (THE DEFAULT) [Resolume says so]: The old clip plays on until the "1"; then the new one takes over and starts. On an empty layer nothing shows until the "1". The new clip's cell shows that it is waiting.  -- why the default: [Resolume says so] As in Resolume when a clip waits for the bar (its manual: "at the start of the next bar, the output will change to play the new clip"), and your own word for it is quantization.
- B: The new clip shows at once, standing on its first frame, and starts moving on the "1". The old clip is gone at your press.
Why it was asked: With B the audience sees a frozen picture for up to a bar each time you fire a BPM-mode clip; with A they keep seeing the old clip (or nothing) until the "1".
His earlier words it rested on: "154 a clip that has BPM mode enabled will start playing on the next 1 if it is triggered in the middle of a bar. This is automatic quantization that is implied when the clip has BPM mode."
Source (Harmony-side): BD:1081; facts-resolume-emulate.md C1.5 (VERIFIED), C1.6; facts-takes-bpm-fire.md section 3 and U1

### Question 174 -- A video not in BPM mode fired while paused   [his lines: L31]
About: R111 (paused) against 136 b
Situation: The beat is paused. You fire a video or a picture sequence that is NOT in BPM mode.
- A (THE DEFAULT) [your words]: It shows on its first frame and plays when the beat plays: while the beat is paused, no video or picture sequence you fire moves, whatever its mode. Your sentence as written. (Pictures without playback, such as MilkDrop: R193 b.)  -- why the default: [your words] Closest to your words: your reply to R111 names no mode ("When paused, it will not play but will still display."), and it took the place of a text of mine that said such a clip plays at once.
- B: It plays at once. Pause holds only the clips in BPM mode (your 136 b): a clip that is not in BPM mode and was already playing plays on through the pause anyway.
Why it was asked: Your answer 136 b means: a pause holds the BPM-mode clips, and a clip that is not in BPM mode plays on. With A, such a clip plays on if it was already playing but waits if you fire it during the pause. With B you cannot stand it ready for a show start (question 172).
His earlier words it rested on: "R111 if I fire a clip while the beat is stopped, it starts playing the beat. When paused, it will not play but will still display." / "136 b"
Source (Harmony-side): BD:1091 against BD:1009-1010; boris-clarify-150-plus.md:92 (R111 as shown); facts-takes-bpm-fire.md 5.9, S8; facts-actions-open.md correction C5

### Question 175 -- The tempo of the very first fire   [NOT NAMED by him]
About: R111: the tempo of the first fire
Situation: You have just opened the app. No music has played yet and you have not tapped or typed a tempo, so the app has no tempo number. You fire a clip.
- A (THE DEFAULT) [mine]: The tempo number is never empty: the app opens at the tempo you used last (120 the very first time). Your fire starts the beat at that number; the music, a tap or a typed number then corrects it.  -- why the default: [mine] Least surprise, and your own rule for a stopped beat: "the beat stops but tempo is not lost, just not playing".
- B: No tempo, no beat: the tempo row shows no number until the app finds the tempo in the music or you tap or type one. Until then a BPM-mode clip you fire shows on its first frame and waits; other clips play.
Why it was asked: It decides what the first press of every night does. Today the app has no tempo until it has heard enough music (seen in how the app is built).
His earlier words it rested on: "135 the beat stops but tempo is not lost, just not playing"
Source (Harmony-side): BD:1007; facts-app-cue-today.md section 6 (BPMTracker.h:36, 216, 219; a show stores no tempo: Composition.h:754-755)

### Question 189 -- Your hand against the app's listening (Manual)   [his lines: L32]
About: R76 and 128: your hand against the app's listening
Situation: The app is listening to the music (the tick box "Manual" is off) and shows 126. You know the track is 128: you tap it, or you press "+" twice, or you press Resync on the "1" you hear.
- A (THE DEFAULT) [your words + mine]: Your hand holds. Any of these switches Manual on: the number and the "1" stay where you put them until you switch Manual off again. This is what "switch Manual on" means: the app stops correcting you.  -- why the default: [your words + mine] Closest to your words: tapping sets the tempo, Resync is the "1". It is also the only way your press is still true ten seconds later.
- B: As today: Manual stays off. The app takes your tap or your "1" for a moment, and within about two seconds its own listening moves the number and the beat back if it hears something else.
- C: The tempo steps and a typed number switch Manual on; Tap and Resync do not: they help the app's listening for a moment, and it carries on.
Why it was asked: Today, with Manual off, the app puts the beat back on every beat it is sure of and replaces a tapped tempo after about two seconds (seen in how the app is built). You asked what "switch Manual on" means; this is the place where it matters. What the number does while the beat is stopped or paused: R214.
His earlier words it rested on: "R76 please explain wha tit means to switch manual on" / "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button push" / "ok what is your question about r76 having to do with manaul?"
Source (Harmony-side): area-tempo.md U-HIS-4, O-17, O-20, M-6 (VERIFIED there: BPMTracker.cpp:99-104, 150-195, 233-235; MainComponent.cpp:5824-5851); BD:957-958, BD:966; BD:1037; src/ui/TopBar.h:150 (the control is a tick box "Manual"; there is no button "Auto")

### Question 190 -- Tap: the tempo only, or the beat too   [NOT NAMED by him]
About: Tap: the tempo only, or the beat too
Situation: The beat runs. You tap along with the music for a few beats to set the tempo.
- A (THE DEFAULT) [Resolume, my guess]: Tap sets only the tempo. Where the "1" sits does not move; you place it with Resync.  -- why the default: [Resolume, my guess] Resolume's manual gives Tap the tempo and Resync the first beat of the bar; it does not say in so many words that Tap leaves the beat alone (one tap in your Arena shows it: R206). It also fits your words: a tap starts nothing, Resync is the "1".
- B: As today: from the second tap on, every tap also pulls the beat onto your tap, but not the "1": the lit quarter of the circle can then disagree with your tap.
- C: Tap sets the tempo, and the first tap of a run is also the "1", as a Resync would be. While the beat is stopped a tap still starts nothing.
Why it was asked: On an earlier page I told you that tapping puts the beat on your taps, as today, and you did not correct it; Resolume's manual does not say that a tap moves the beat, so I take it that it does not (one tap in your Arena shows it: R206). With A you need one Resync press after you tap in a new track.
His earlier words it rested on: "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button push" / "If I tapped the tempo again, to set the tempo, the time does not change. If I press re-sync, then it does re-sync and that changes by how far off the beat we are."
Source (Harmony-side): area-tempo.md U-HIS-5, M-1, M-2 (VERIFIED there: TopBar.cpp:55-77; BPMTracker.cpp:253-258, 566-578); facts-resolume-emulate.md C3.1; BD:855-857, 957-958

### Question 191 -- A column that holds both kinds of clips   [NOT NAMED by him]
About: R128: a column that holds both kinds of clips
Situation: The beat runs. Half-way through a bar you click a column that holds two BPM-mode clips and one clip that is not in BPM mode.
- A (THE DEFAULT) [your words]: Each clip follows its own rule (R128): the clip that is not in BPM mode starts on your click, the two BPM-mode clips start on the next "1". For up to a bar the column is only partly there.  -- why the default: [your words] It is your 154 applied to each clip, and a clip that is not in BPM mode waits nowhere else.
- B: The whole column waits: if any clip in it is in BPM mode, all of them start together on the next "1".
Why it was asked: Your show-start sentence says a column triggers "all of them at the same time". From stop that is true either way, because your click is the "1". This is the case where it is not. In Resolume 7.22 a column has a Beat Snap setting of its own; how it meets a clip's own setting is not documented.
His earlier words it rested on: "The way to set up / cue up for a show start is to load up the clips and press start, or click on a column to trigger all of them at the same time and cue them ahead of time via the cue system which I am about to describe." / "154 a clip that has BPM mode enabled will start playing on the next 1 if it is triggered in the middle of a bar. This is automatic quantization that is implied when the clip has BPM mode."
Source (Harmony-side): area-clip-transport.md U-H4; area-cue-layers.md H12; BD:1081, BD:1092

## READINGS (18)
### R128 (about: 154)   [NOT NAMED by him]
(a) A clip in BPM mode that you fire in the middle of a bar starts playing on the next "1" (the first beat of the next bar), by itself. A clip that is not in BPM mode starts on your press. (b) Once this is built there is no Snap box on a clip and no Quantize setting for firing clips live (the review screen has its own Quantize, R166). (c) In Resolume the wait is a separate setting, Beat Snap; in your app BPM mode itself does it: your words. Mine: (d) fire the clip that is already playing and it plays on, then starts again from its beginning on the next "1"; (e) a click on a column starts its BPM-mode clips on the next "1" and the others on your click. What the layer shows while the clip waits: question 173.
(why_revised: "Adds the re-fire and the column (mine); the open point became question 173.")
(label: "his words (154); the re-fire and the column INFERRED; the Resolume fact VERIFIED (Beat Snap is a setting of its own)")
(source: "BD:1081; facts-resolume-emulate.md C2.1, C1.1; facts-takes-bpm-fire.md 5.1-5.5; today's Snap and Quantize controls: section 3 of the same sheet (TopBar.cpp:172-187, ClipInspector.cpp:162-175)")
(star: true)

### R168 (about: 154: while a clip waits)   [his lines: L11]
Mine, no word of yours covers it: (a) while a BPM-mode clip waits for the "1", a newer fire on the same layer takes its place (the last press wins). (b) The layer's X, or a click on an empty cell, ends the wait and clears the layer at once, in the middle of a bar too: an empty cell has no BPM mode, so it does not wait. (In Resolume an empty cell waits too; I do not copy that: it would leave a layer playing for up to a bar after you clear it.) (c) Press the waiting clip again and nothing changes: it keeps waiting for the same "1". (d) If the old clip plays on meanwhile (173 A), the layer's fade to the new clip starts on the "1".
(label: "INFERRED (no word of his covers it; in Resolume an empty slot waits with the composition's Beat Snap and the layer's X is immediate: a reply of its team, 2018)")
(source: "facts-resolume-emulate.md C1.8 with its correction W3; facts-app-cue-today.md section 2 (an empty cell clears the layer); BD:1081")
(star: true)

### R139 (about: R111)   [his lines: L12]
Your sentence has two halves. (a) STOPPED: you fire a clip and the beat starts ("it starts playing the beat"). Mine: (b) your press is the "1"; the clip plays from its beginning; any clip does this, in BPM mode or not, fired by a click, a key, a MIDI pad or a column; a click on an empty cell starts nothing; play and Resync still start the beat too. (c) PAUSED: a video or picture sequence you fire shows and does not play ("it will not play but will still display"). Mine: (d) it stands on its first frame; when you press play the beat runs on from where it was held, and a BPM-mode clip starts on the next "1" (your 154); when the beat was held at its start, your play press is that "1". A picture without playback (a still, a generated picture, MilkDrop): R193 b. This replaces the old R111. How this goes with your show start: question 172. While the beat runs, whether a waiting clip is seen: question 173. A clip that is not in BPM mode under pause: question 174. What tempo the very first fire runs at: question 175.
(why_revised: "Splits your sentence into its stopped half and its paused half, and marks what I added. Says which clips the paused half is about (those with playback); the others are in R193 b.")
(label: "his words (R111); the sentences after Mine: INFERRED")
(source: "BD:1091, BD:957-958; facts-takes-bpm-fire.md 5.7-5.10; facts-resolume-emulate.md C3.8 (Resolume has no text for this rule)")
(star: true)

### R140 (about: show start)   [NOT NAMED by him]
Your show-start sentence, as I read it: "press start" is the play button of the tempo row; "load up the clips" is firing them while the beat does not run, so that they stand ready; the column is the other way to start them all at once; "cue them ahead of time" is looking at them in the preview monitor first (R149 says how). Whether I read it right: question 172.
(why_revised: "Now says how I read each part of your sentence.")
(label: "his words (show start); how each part is read INFERRED")
(source: "BD:1092")

### R142 (about: R114)   [NOT NAMED by him]
As you said: a clip you paused is still paused after you save the show, quit and open it again. That pause is saved (it was already ruled so, your 122 a). Today it is not saved: after you re-open a show, the clip plays when you fire it.
(why_revised: "Now in your own words; the tempo row's part is in R163. Adds that the pause is not saved today.")
(label: "his words (R114, 122 a)")
(source: "BD:1094, BD:944")

### R163 (about: R114: the beat when the app or a show opens)   [NOT NAMED by him]
When the app starts, the beat is stopped and waits for you: until your first fire, play or Resync the circle stands still, and everything that follows the beat stands on the "1" (R214). On an earlier page I told you the opposite, that the app opens with the beat running, as it does today once it has found the tempo in the music; your "press start" made me change it. Say "R163 running" if the app should open with the beat running. Opening a show never changes the beat: stopped stays stopped, running keeps running, and the tempo number stays as it is. As today, a show holds no tempo number and nothing of the tempo row's play, pause or stop; of the tempo row it holds only the nudge amount (your 63; the "/2" and "x2" buttons: R215). The action buttons open as you saved them (R132), but no action plays until the beat runs (R158).
(label: "INFERRED from his show-start words; what a show holds today VERIFIED in the code (no tempo number; a saved multiplier that nothing reads: R215); the nudge amount: his words (63)")
(source: "BD:1092, BD:1094, BD:916; facts-app-cue-today.md section 6 (Composition.h:754-755)")
(star: true)

### R169 (about: the tempo row with Ableton Link on)   [NOT NAMED by him]
As in Resolume (a reply of its team from 2023, not its manual): with Ableton Link switched on, the tempo row's pause and Resync are greyed out, because they would move the beat that the other programs share. Mine: play is greyed out too; a fire never starts or moves the beat, because the beat is Link's and always runs; stop still takes the clips off the layers and leaves the beat running. Link is off unless you switch it on.
(label: "As in Resolume (VERIFIED: team reply 2023); the sentences after Mine: INFERRED; what Resolume's stop does under Link is UNKNOWN")
(source: "facts-resolume-emulate.md C3.7; CLAUDE.md (Ableton Link: optional, off by default)")

### R164 (about: the DJ tracks)   [NOT NAMED by him]
(a) The three DJ tracks you are bringing are for measuring how well the app finds the "1" in real music. One part of the BPM-mode work waits for them, and so does how far you can rely on the "1" while the app listens by itself. (b) The "1" your clips wait for: while the app listens to the music (Manual off) it finds the "1" itself, it can be wrong, and it counts the bars afresh after a drop or a breakdown (seen in how the app is built); when you press Resync the "1" is yours and stays (question 189 A). The tracks measure how often the app finds it right.
(label: "his words (the tracks); what waits VERIFIED in the plans (by the fact sheet); how the app finds the 1 VERIFIED in the code by the facts seat (BPMTracker.h:21-24, 246-253; BPMTracker.cpp:509, 541)")
(source: "BD:1098, BD:1040; facts-takes-bpm-fire.md section 4")

### R174 (about: 154 and R111: the edges)   [his lines: L13]
Mine, the edges of your 154 and of R111 that no word of yours covers. (a) A clip you paused yourself does not wait: it shows at once on its paused frame, because nothing in it moves. (b) While a BPM-mode clip waits and you press Resync, your press is the "1" and the clip starts then; "/2", "x2", a tap or a typed tempo move the "1" it waits for; stop takes it away with everything else; pause holds the wait, and the clip starts on the first "1" after the beat plays again. (c) Every BPM-mode clip waits, a clip set to Play Once too: a clip you want to punch in on your press is not put in BPM mode (in Resolume the wait is a setting of its own; your 154 makes it part of BPM mode). So a hit that must land on your press cannot also follow the tempo; if you want both, say "R174 c split" and I will ask. (d) A press that starts a stopped beat (R139) leaves the nudge number as it is; only Resync sets it to 0 (your 129 b). The "1" a clip waits for is the nudged "1", like everything else on the beat (R177). A pad you hold and let go before the "1": question 207.
(label: "INFERRED (no word of his covers these edges); 121 a, 129 b and 154 are his")
(source: "area-clip-transport.md U-H2, U-H25, U-H26; area-tempo.md U-HIS-2; BD:943, BD:959, BD:1081")
(star: true)

### R175 (about: earlier words that a later word of yours replaced)   [NOT NAMED by him]
Four earlier things are replaced by later words of yours; I list them so that nobody builds the old one. (1) Your 123 b (a clip fired between two "1"s starts at once and is cut back to its beginning on the next "1") no longer holds for firing a BPM-mode clip: by your 154, said later, it waits and starts on the "1". 123 b and 71 b still hold for a clip that falls out of time while it plays, after a tempo change: it plays on and is cut once on the next "1"; a Resync cuts it at once (121 a). (2) Your 128 named Resync as what starts a stopped beat; by your later reply to R111 a clip you fire starts it too (R139). (3) On the tempo-row page I told you that the app opens with the beat running; R163 replaces that: the beat is stopped and waits for you. (4) Your words of 2026-10-03, that a clip is held to the line "if we are in Qantize mode", fell away when you took Quantize out of live use on 2026-10-04.
(label: "his words (123 b, 71 b, 121 a, 128, 154, the reply to R111); that the later one replaces the earlier one INFERRED from the dates")
(source: "area-clip-transport.md O1, O2, O3; area-tempo.md O-03, O-05; BD:630-635, 902, 943, 945, 957, 967-972, 1081, 1091")

### R176 (about: the tempo row: what stands from earlier pages)   [his lines: L14]
These stand from earlier pages: you answered them, or I told them to you and you did not correct them. Say so if one is wrong. (a) Stop: one press; every clip leaves every layer; the beat stops and the tempo number is not lost (R214 d). Mine, never shown to you: a layer set to Ignore Column Trigger is cleared too, so a logo you keep on such a layer goes with a stop. Say "R176 a keep" if Stop should leave those layers alone. (b) Pause holds the beat and the BPM-mode clips; no button pauses every clip at once any more (you chose 136 b, not the option where every clip holds), and each clip keeps its own pause. Today the top bar's "||" pauses every playing clip; it goes with the three old buttons, and the tempo row's play, pause and stop take their place (your r85). (c) "-" and "+" step the tempo by 1 BPM, in whole numbers. (d) A tempo set by hand can run from 30 to 400; "/2" is greyed below 60 and "x2" above 200. (e) The nudge: plus moves the beat earlier; the text reads "nudge X ms"; one press is 1 ms, a held press repeats, a number can be typed; it runs from -500 to +500; pause never touches it. (f) A tap never starts the beat. (g) The words "Bar 1" to "Bar 4" are gone; the circle alone shows the beat.
(label: "his words (125, 126 and 127, 135, 136 b, 61 b, 128, 144 a, 146 b); the range 30 to 400, the greying and the nudge's step are readings told to him and not corrected (INFERRED consent); the Ignore Column layer INFERRED from his word all")
(source: "area-tempo.md part 6 (S-1 to S-9), U-HIS-8, U-HIS-9, U-HIS-13, C-4; BD:911, 951-953, 957-959, 1007, 1009, 1069-1071")
(star: true)

### R214 (about: what stands still while the beat does not run; the tempo number meanwhile)   [NOT NAMED by him]
Mine, from the tempo row as it is planned (today the app has no stop and no pause of the beat). (a) Stop: the circle stands on the "1" and every clip leaves. Everything that follows the beat stands on the "1" until the beat runs again: signals that run on the beat (oscillators, envelopes), sliders and effects driven by the beat, actions, the autopilot's count, MilkDrop's Jukebox count. (b) Pause: the same things stand where they were; a clip that is not in BPM mode plays on (your 136 b). (c) In both, what the music itself drives (loudness, bass, tones) keeps moving. (d) The tempo number: while the app is listening (Manual off) it keeps following the music, also while the beat is stopped or paused, so your first fire of the next song starts the beat at that song's tempo; with Manual on it holds what you set. Say "R214 d hold" if the number should freeze while the beat does not run.
(label: "INFERRED from the table of the adopted tempo-row ruling (the tempo number live in stop and pause; beat-driven things stand), read by the cover-tempo seat, not by me; his 135 (tempo is not lost) and 136 b; today no stop or pause of the beat exists")
(source: ".harmony/.reports/s-rta-1004b/ruling-nudge-row2.md:34-48 (by the seat); area-tempo.md M-6, U-TECH-4; BD:1007, BD:1009")
(star: true)

### R215 (about: 62: the buttons beside the tempo number)   [NOT NAMED by him]
Your row (your 62) has only "/2" and "x2". Today there are five buttons beside the tempo, "/4" "/2" "x1" "x2" "x4", and they change nothing: the app keeps which one you pressed, the show even saves it, and nothing uses it (seen in how the app is built). So the five go and two come: "/2" halves the tempo number and "x2" doubles it, once per press, as in Resolume's row; neither moves the "1" (told to you earlier and not corrected); "/2" is greyed below 60 and "x2" above 200 (R176 d). Nothing of them is saved with a show.
(label: "his words (62: the list of the row); today VERIFIED in the code (src/ui/TopBar.cpp:403 writes the value, src/model/Composition.h:754 saves it, no reader in src: grep); once per press and never the 1: the adopted tempo-row ruling, told to him as a reading and not corrected (INFERRED consent)")
(source: "BD:913-915; src/ui/TopBar.cpp:403, 425-426; src/model/Composition.h:151, 754, 891; .harmony/.reports/s-rta-1004/ruling-nudge-row.md:71-85, 460 (by the cover-tempo seat)")

### R216 (about: 147 a: effects that hold or trail a picture after a stop)   [NOT NAMED by him]
You answered 147 a: when stop takes every clip off, an effect that holds or trails a picture goes on doing what it does: an Echo or feedback trail fades out by itself, and a frozen picture stays until you switch that effect off. Today it is not so: with no clip playing the global effects do not run and the screen goes dark at once (seen in how the app is built, not run). It is built with the stop. Apart from that, with no clip playing the output is black (R228).
(label: "his answer 147 a (the wording is the question's as asked); today's behaviour VERIFIED in the code, not run (CompositorEngine.cpp:1064-1065: no layer with a playing clip gives no picture; Renderer.cpp:709-712: the global effects run only on a picture)")
(source: "BD:1072; .harmony/.reports/s-rta-1004b/boris-clarify-144-147.md:13-16; area-cue-layers.md W3")

### R177 (about: the nudge and a screen's Delay)   [NOT NAMED by him]
Two controls bring picture and sound together, and they do different jobs. A screen's Delay (0 to 100 ms, one per output screen) makes everything on that screen later: it is for the room, the distance from the speakers to where you stand. The nudge moves where the "1" sits, earlier or later, for everything that runs on the beat: it is for a beat that sits a little off the music. Your show keeps its nudge amount (your 63), and a Resync sets the nudge to 0 (your 49). Together that means: a nudge saved with a show lasts until your first Resync of the night. Say so if a saved nudge should come back after a Resync. A Delay can only make a screen later, as in Resolume; your earlier "a little ahead" now exists only as the nudge, for what runs on the beat. What follows the loudness of the music cannot be moved earlier.
(label: "his words (63, 49, 51, and the purpose of the Delay); how the two rules meet INFERRED")
(source: "area-tempo.md U-HIS-6; area-outputs.md H9; BD:802-803, 866-868, 916-917, 940")

### R178 (about: Ableton Link)   [NOT NAMED by him]
Ableton Link: in the app as it is built for you today the Link switch is dimmed (your copy is built without Link), and you have never mentioned it. It stays that way and nothing new is built for it. If it is switched on one day, R169 says what the tempo row does. Whether you use Link: question 215 (b).
(label: "INFERRED from the area sheet (the switch is dimmed in his build: read there in the build's settings, not by me); no word of his names Link")
(source: "area-tempo.md part 1 line 10, U-HIS-7, M-3")

### R205 (about: a column click and the empty cells in it)   [NOT NAMED by him]
A click on a column also clears every layer whose cell in that column is empty; only a layer set to Ignore Column Trigger is left alone. A click on one empty cell does the same for its layer (R168). The app works this way today, and you have never been shown it. I believe Resolume does the same (not checked: one column click in your Arena shows it, R206). Say "R205 leave" if a click on a column should leave a layer playing when its cell in that column is empty.
(label: "today's behaviour seen in how the app is built by the area sheet (Layer.h:402-403; Composition.h:447-448), not run; Resolume: an empty slot waits for the composition's Beat Snap and the layer's X is immediate (team reply 2018), what its column click does with an empty slot is UNKNOWN; no word of his covers it")
(source: "area-cue-layers.md M3 (HIS; live risk: a blank layer in mid-show); facts-resolume-emulate.md C4.4, correction W3; BD:1092")
(star: true)

### R206 (about: eight looks in your Arena)   [his lines: L15, L16, L17, L18, L19, L20, L21, L22, L29]
Not something I understood, but eight looks that only your Arena can give me. Each takes a minute, none holds up this page, and each settles a guess of mine. (a) Click a clip's name: does the clip play and loop in the lower monitor, or stand still? (R151) (b) Fire a clip with the tempo bar paused, and again with it stopped: what happens? (R139, question 172) (c) Tap a tempo while a clip plays: does the beat jump to your tap, or does only the number change? (question 190) (d) Click a column in which one layer's cell is empty while that layer plays: does that layer go empty? (R205) (e) Random on a clip in BPM Sync: does a jump land only on the first beat of a bar, or on any beat? (question 201) (f) On a clip in BPM Sync, click "+" on Speed and on Duration and send me the numbers; in Timeline mode set Speed 2 and send the Duration it shows. (g) One picture: the "Manage..." window of an effect's presets, open (R155). (h) Press B (bypass) on a layer while its video plays, wait a few seconds, press B again: did the video play on out of sight, or did it stand still? (R194 d)
(label: "UNKNOWN, each one (the cheapest way to find out is his own Arena); (f) was asked on 2026-10-04 and half answered")
(source: "the list arena_checks of this file; facts-resolume-emulate.md B2.4-B2.8, C3.8, C3.1, A3.1; area-clip-transport.md part 5 (RQ-0, RQ-3), U-H11; area-cue-layers.md A5, M3")

## DECIDED WITHOUT ASKING (6) -- he did NOT read these: each is still Harmony's assumption until his words settle it
### D1: Only you start a stopped beat: your own fire (from the mouse, a key, a MIDI pad or another controller), play, or Resync. The autopilot does not, and a recording played in the review screen does not touch the live beat at all.
(decided by: The adopted tempo-row ruling for replayed commands (facts-takes-bpm-fire.md 5.8, 5.10); the autopilot steps on beats, and a stopped beat has none.)

### D2: If you answer 175 A, the app remembers one number between sessions: the tempo you used last.
(decided by: Nothing stores a tempo today, neither the show nor the app's settings (facts-app-cue-today.md section 6).)

### D3: A long run of taps keeps following you: the tempo is the average of your last eight taps (today it stops changing after the eighth tap).
(decided by: area-tempo.md M-1 (TopBar.cpp:55-77, VERIFIED there); the product half is question 190.)

### D4: A press within a tenth of a beat of the "1" counts as on the "1": a BPM-mode clip fired there starts at once and does not wait a whole bar.
(decided by: The tolerance ruled earlier for a clip being in time (not built: main has no bar lock); INFERRED that it carries over to a press (area-clip-transport.md U-H2, TB-1; facts-takes-bpm-fire.md section 4).)

### D5: The app counts every bar as four beats, everywhere: the "1" is the first of four.
(decided by: VERIFIED by the area sheet: src/analysis/BPMTracker.h:61 (four beats to the bar on both sides; area-actions.md W3). His clips are in multiples of 4 (BD:812-814).)

### D6: Pause pressed on a stopped beat holds it at its start and leaves the layers as they are; play from there is beat 1. Stop on a paused beat stops it. Play while the beat runs, and a second stop, change nothing.
(decided by: No ruling defines these presses yet (area-tempo.md U-TECH-4: UNKNOWN in the adopted tempo-row text); question 172 A needs the first one; an architect delta on the tempo row is owed.)

## COMES NEXT AS PICTURES (2) -- he did NOT read these; his new rule on layout is L8
### P1: A clip's cell while it waits for the "1" or is shown in the preview.
(variants that were to be drawn: a small countdown pie on the thumbnail, as Resolume has | a bar that fills along the name | a blinking border; a second border colour for the previewed clip)

### P2: The tempo row: where "Manual", "Link", the word that says whether the app has found the tempo, and the typing of the tempo number sit among the cells of your Resolume picture; which signs the two nudge buttons wear; how the circle and the play, pause and stop cells look while the beat is stopped, paused and running.
(variants that were to be drawn: the cells of your picture, then Manual, Link and the state word to the right of RESYNC | the state word under the tempo number, Manual and Link as two small tick boxes at the right end of the row | your picture alone; Manual and Link in a small menu)

