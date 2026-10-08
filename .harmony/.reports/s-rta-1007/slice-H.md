# SLICE H -- How a clip plays
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
201 202 R192 R217 R218 R193 R194 R219 P28 P29 G1

## GENERAL POINTS OF HIS MESSAGE THAT LAND IN THIS TOPIC (new; they answer no numbered item)
### G1 (L3): a codec of our own, like DXV 3.0 (he asks; a researcher answers the question, you file where it lands)

## QUESTIONS (2)
### Question 201 -- Where a Random jump lands   [his lines: L101]
About: Random: where the clip lands
Situation: A clip in BPM mode has its loop menu on Random, as in your Resolume picture, with its two numbers Interval and Distance. Every so often the clip jumps to another place in itself.
- A (THE DEFAULT) [your words]: Every jump lands on a "1" of the clip: the first beat of one of its bars. When it jumps is set by Interval and how far by Distance, both in beats, as in your picture.  -- why the default: [your words] Your sentence about that picture, in which "1" is your word.
- B: Every jump lands on a whole beat of the clip, any beat, up to Distance beats away. Resolume's older manual says: "the clip will randomly jump to a random beat and continue playing from there".
Why it was asked: With A the clip always comes in on the first beat of one of its bars; with B it can come in on its beat 2, 3 or 4, still in time. You also said to mimic Resolume's transport control exactly: one look at Random in your Arena shows which of the two it does (R206). Your "random 1's" points to A, your "mimic exactly" to B.
His earlier words it rested on: "this is essentially jumping to random 1's on the beat" / "mimic exactly how resolume is doing"
Source (Harmony-side): BD:1000-1002, BD:757-758; area-clip-transport.md U-H11 (HIS, leans A; the adopted transport ruling jumps whole beats, any beat); what Resolume's Random does on a bar line is UNKNOWN (cheapest: his Arena); facts-resolume-transport.md:48, 65 (the quoted sentence of the older manual; by the facts seat)

### Question 202 -- The small menu: where a fired clip starts   [his lines: L102]
About: 42 against r87: the small menu that says where a fired clip starts
Situation: In your Resolume picture of the clip panel there are two small menus at the right: the loop menu (R217) and one that decides where a clip starts when you fire it again (from its start, carrying on from where it was, or carrying on as if it had never stopped). You said two things about it. First you took the default of question 42 ("42 default"): every fire starts the clip from its beginning, and no menu to carry on is built. Later, of that picture: model ours on it. Today ours shows this menu (Restart, Continue, Relative) and it does nothing.
- A (THE DEFAULT) [your words]: No such menu: every fire starts the clip from its beginning. The menu that is on screen today goes.  -- why the default: [your words] You took the default of question 42, which says exactly this, and the day before you answered "restart" to the same point; your later sentence is about the panel as a whole and names no menu.
- B: Ours has the menu with the three entries, as in your picture, and "from the start" is set on every clip until you change it.
Why it was asked: With B a fire no longer always means "from the beginning" (R193 c, R128 d), and every rule on this page that says a fired clip starts from its beginning gets an exception. With A our panel differs from your picture by one small menu.
His earlier words it rested on: "42 default" / "r87 this is the resolume bpm clip menu, lets model ours based on this:"
Source (Harmony-side): BD:844-845 against BD:1025-1027; src/ui/ClipInspector.cpp:69-73 (read by me: three entries, no handler); facts-resolume-transport.md:55 (the manual calls the three "from the start / pick-up / relative pick-up"); area-clip-transport.md part 1 item 4

## READINGS (6)
### R192 (about: Random and Beat Repeat)   [NOT NAMED by him]
As in Resolume, from your pictures (you asked me to mimic its transport control exactly). (a) Beat Repeat's buttons read Off, 4, 2, 1, 1/2, 1/4, 1/8, 1/16, 1/3, 1/6 and "Catch up", and the numbers count beats, as Resolume's do. This replaces the list I had planned (1/16 to 4, counted in bars, without 1/3 and 1/6). (b) Random shows Interval and Distance, also counted in beats. Where a Random jump lands: question 201. (c) Mine, as I understand Resolume's own description (not tried): Beat Repeat is a thing of the moment: it repeats from where the clip is, it goes off when the clip is fired again or leaves the layer, and it is not saved.
(label: "As in Resolume (his pictures VERIFIED by the area sheet; that the numbers are beats, and what Beat Repeat does on a fire: from the earlier fact sheet, a reply of its team); where a Random jump lands is his to say: question 201")
(source: "area-clip-transport.md U-H11, U-H12, U-H21, O13; BD:757-760, 947, 1000-1002")

### R217 (about: the loop menu of a clip)   [his lines: L98]
As in Resolume (its manual, and your picture of its panel): the loop menu of a clip reads Loop; Ping Pong (forwards, then backwards, again and again); Random (the clip jumps by itself: R192, question 201); Play Once and Eject (it plays once, then the clip leaves its layer and the layer is empty); Play Once and Hold (it plays once and stays on its last frame). Today the list reads "Loop", "Ping Pong", "One Shot". Eject is the one to look at: on stage, a clip that ends leaves its layer empty. Mine: when the clip's own choice of what comes next (R203 b) or the layer's autopilot brings a next clip at that moment, the next clip wins, and Eject empties the layer only when nothing follows. Say so if Eject should always win.
(label: "As in Resolume (VERIFIED by the earlier fact sheet from its manual pages; its lines on Play Once and Eject, Play Once and Hold and the five entries re-read by me); today's list VERIFIED in the code (src/ui/ClipInspector.cpp:56-58); who wins at the end of a clip: INFERRED, no word of his")
(source: ".harmony/.reports/s-rta-1004/facts-resolume-transport.md:47-51, 139; area-clip-transport.md U-H18, part 2 (D-12); BD:757-760, 1025-1027")
(picture: "../s-rta-1004b/boris-images/resolume-bpm-sync-panel.png")

### R218 (about: 137 and your bars: the length of a BPM-mode clip)   [his lines: L99]
Three sentences of yours are about this row, and I read the latest as the rule. (a) The row reads "Beats", as in your Resolume picture, and "+" and "-" change the length by 1 beat (your 137, said after "lets do bars here not beats"); "/2" and "x2" halve and double it. So a clip of 16 beats becomes 17 with one press, and a clip of 17 beats drifts against the bars each time it loops. (b) Your rule of multiples of 4 holds for the app's first guess: a new clip gets a whole number of bars in multiples of 4, and if the file is uneven the out point is moved in (today every new clip starts at 4 beats). Say "R218 bars" if the row should read and step in bars after all.
(label: "his words (137; bars not beats; multiples of 4; model ours on the Resolume panel); that the later sentence replaces the earlier one for this row: INFERRED from the times (11:59, 12:21, 21:33 of 2026-10-04); the row of his picture reads Beats with - + /2 x2 (looked at); today's 4 beats: by the cover-clip-transport seat (Clip.h:36-41)")
(source: "BD:761-762, 812-814, 865-866, 1011-1013, 1025-1027; area-tempo.md U-HIS-10, O-12; area-clip-transport.md U-H10, O7")
(picture: "../s-rta-1004b/boris-images/resolume-bpm-sync-panel.png")

### R193 (about: how a clip plays: the rest)   [NOT NAMED by him]
Standing, or mine where no word of yours exists; say so if one is wrong. (a) Only a video and a picture sequence have playback (BPM mode or not, speed, pause, backwards, the loop styles). (b) Mine: a still picture, a generated source, MilkDrop, the camera and an effects-only clip have none: they show on your press and run on their own clock; the tempo row's pause never freezes them, and fired during a pause they show and move. Your sentence for a pause ("it will not play but will still display") names no kind of clip; I take it for clips that have playback, because you also wrote that what is not BPM based plays "just as they were". Say "R193 b freeze" if a generated picture or MilkDrop that you fire during a pause should stand still until play. (c) Every fire of a video starts it from its beginning (your "restart"; the small menu that could change this: question 202); mine: a clip you set to play backwards keeps its direction and starts from its end. (d) Mine: a clip's pause is one thing shown in two places: the Clip tab's buttons and the layer strip's "<", "||", ">" are the same buttons of the clip that plays. (e) The same for speed: the strip's S slider and the Clip tab's Speed are one control with one range (to 10 when the clip is not in BPM mode: the real top is measured before it is built, and if it is lower I tell you first; the halves and doubles in BPM mode). (f) Mine: the master Speed changes only clips that are not in BPM mode. (g) As in Resolume when the clip is not in BPM mode (its manual, as my notes report it): pulling the out point in makes the clip shorter at the same speed and the Duration follows; typing a Duration changes the Speed. In BPM mode it is your own rule: pulling the out point in keeps the same number of bars, so the clip goes through less video and looks slower. (h) The clip's timeline shows bars only.
(label: "his words (restart; 74 b; bars only on the timeline); which clips have playback VERIFIED by the area sheet (Clip.h:248); the Resolume rule for the out point: its manual, by the earlier fact sheet; the rest INFERRED")
(source: "area-clip-transport.md U-H8, U-H19, U-H20, U-H24, U-H27; area-cue-layers.md H15, H16; area-sources-auto.md H19, H20; BD:668, 787-788, 825-827, 907, 1009")
(star: true)

### R194 (about: the layer strip)   [NOT NAMED by him]
Mine, where no word of yours exists. (a) A new layer's fade (F) is 0, a cut; today a new layer shows 0.3 seconds on F and cuts anyway (seen in how the app is built, not run). F counts seconds. (b) Fire a third clip while a fade is running: the new fade starts from the picture as it is at that moment. (c) Solo on a layer that holds no clip does not darken the show (today it does). (d) B (bypass) and S (solo) stay controls of the output. Mine: a clip on a layer that is bypassed, or hidden by another layer's solo, plays on out of sight, so when the layer comes back you see it further on (today it appears to stand still: seen in how the app is built, not run; R206 h shows what Resolume does). Say "R194 d hold" if it should stand still and go on from that frame. (e) The X's tooltip, which promises that the clip comes back with Undo, is corrected (R198). (f) A new layer's Blend starts on "Add", as today: black in its picture lets the layer below show, and light parts add up toward white. Say "R194 f alpha" if a new layer should start on "Alpha", which covers what is below except where the picture is see-through. What a column click does to a layer whose cell is empty: R205.
(label: "today's behaviour seen in how the app is built by the area sheet (Layer.h:361-374, 402-403; LayerStrip.cpp:345-346; CompositorEngine.cpp:1043-1063), not run; the rest INFERRED; the column click is reading R205")
(source: "area-cue-layers.md H10, H17, H19, M3, M7, M8, W11")
(star: true)

### R219 (about: the thin playhead bar of the layer strip)   [NOT NAMED by him]
The thin bar under a layer's picture in the layer strip shows where the playing clip is. Today you can drag it and it springs back: the clip does not move (seen in how the app is built, not run). Mine: it becomes a real jump, with the rules you gave for the Clip tab's timeline: it never leaves the in and out points, a click outside does nothing, the picture waits while you hold the mouse still, and the clip plays on from where you let go, at the same speed.
(label: "today: the write VERIFIED by the cover-clip-transport seat (LayerStrip.cpp:979-989), the spring-back INFERRED there; the rules are his words for the Clip tab's timeline; that they carry over to the strip: INFERRED")
(source: "area-clip-transport.md part 1 items 18-19; BD:658-660, 725-730, 750-756")

## DECIDED WITHOUT ASKING (0) -- he did NOT read these: each is still Harmony's assumption until his words settle it
## COMES NEXT AS PICTURES (2) -- he did NOT read these; his new rule on layout is L8
### P28: The layer strip as a whole: today's buttons and sliders plus 8 action buttons and "+", the ignore lamps, and a pause mark that tells a clip you paused from one the tempo row holds. What gives way.
(variants that were to be drawn: a wider strip | taller rows, fewer of them on screen | the K slider and the three play buttons leave the strip)

### P29: The clip's Transport panel in the Clip tab, modelled on your Resolume picture: where Beat Repeat's row, Random's Interval and Distance, the Beats row and the eight Cuepoints sit.
(variants that were to be drawn: exactly as the picture, with Beat Repeat as a row of buttons under the timeline | Beat Repeat and Random folded behind the loop menu | the Cuepoints in a fold of their own)

