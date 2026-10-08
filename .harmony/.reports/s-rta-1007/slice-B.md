# SLICE B -- The cue system
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
176 177 R141 R149 R150 R151 R152 R153 R170 R179 D7 D8 D9 P3 P4 P5

## QUESTIONS (2)
### Question 176 -- A clip's name clicked while layers are cued   [his lines: L37]
About: cue system: a clip's name
Situation: Layers 1 and 2 are cued and show mixed in the preview monitor. You click the name of a clip of layer 3, to look at it before you fire it.
- A (THE DEFAULT) [Resolume says so + mine]: The clip takes the whole preview monitor, alone. The cued layers come back when you fire any clip, press a cue button, or click that name again.  -- why the default: [Resolume says so + mine] As in Resolume: a previewed clip is shown outside the composition (its own video says so), and the clip preview is cleared when a clip is fired by hand (a reply of its team, 2018). Coming back by a cue button or by the name is mine.
- B: The clip is shown inside the cued mix, as if it were already playing on layer 3: you see how it will look together with layers 1 and 2. It leaves the preview as in A.
Why it was asked: A shows the clip by itself. B shows the mix you are about to make, which is your DJ picture, but it is not what Resolume does.
His earlier words it rested on: "Also, when a user clicks on the bottom of a clip, where its name is, it is displayed in the preview monitor. I want this behavior as well to preview clips before adding them to the layer strip." / "exactly how a DJ mixer allows the user to cue and listen to how the tracks would be mixed together before they mix them"
Source (Harmony-side): BD:1093; facts-resolume-emulate.md B1.5 (VERIFIED), B2.9 (team reply 2018)

### Question 177 -- Global effects in the preview   [his lines: L38]
About: cue system: global effects
Situation: You have effects on in the global tab, and you cue two layers.
- A (THE DEFAULT) [Resolume, my guess]: The preview shows the cued layers with their own clip effects and layer effects, mixed. The global tab's effects are not on it: you see the layers before the global effects are added.  -- why the default: [Resolume, my guess] A DJ's cue is taken before the master; Resolume's layer monitor also appears to leave the composition's effects out (worked out; not stated in its manual).
- B: The preview runs the cued mix through the global tab's effects too, so that it looks the way it would on the output.
Why it was asked: With A the preview can look different from what the audience will get when your global effects are strong. With B the preview costs more to draw, and effects that remember earlier pictures (trails, echo) can only be approximated in it (my reading of the code, not tested; the same holds for a layer's own Echo or Feedback under A).
His earlier words it rested on: "exactly how a DJ mixer allows the user to cue and listen to how the tracks would be mixed together before they mix them"
Source (Harmony-side): BD:1093; facts-resolume-emulate.md B3.4 (INFERRED); facts-app-cue-today.md section 5

## READINGS (8)
### R141 (about: cue system)   [his lines: L34]
A preview monitor sits just below the output monitor. Under it, one cue button per layer; any number of them can be on. The layers switched on there show in the preview mixed together, with their blend modes, in layer order, as they would mix in the output, except that each cued layer's own fader is ignored (R149 a) (with or without the global effects: question 177). Nothing you do there ever changes the output. This part is yours and not Resolume's: in its monitor menu you pick one layer or one group at a time, or the A or the B side of its crossfader (your picture: its green dot sits on one entry, "Selected Clip"; the second dot is the switch of the Transform Widget); a Resolume reply of 2015 says previewing several layers together is not possible, and I found nothing newer. A click on a clip's name: R150 and R151.
(why_revised: "Adds that several cue buttons can be on at once and that the output is never touched. The picture has two green dots; the reading now says which is which. Says that a cued layer ignores its fader, and names the crossfader entries of Resolume's menu.")
(label: "his words (cue system); the Resolume facts VERIFIED (his picture; team reply 2015)")
(source: "BD:1093; facts-resolume-emulate.md B1.2, B3.5, B3.6")
(picture: "resolume-monitors-menu.jpg")

### R149 (about: cue system: the fader)   [NOT NAMED by him]
(a) A cued layer shows in the preview at full strength wherever its fader is, the way a DJ hears a cued track with the channel fader down (Resolume's layer monitor does the same: worked out from two replies of its team, not stated in its manual). (b) You see the layer's clip with the clip's effects and the layer's effects. Mine: (c) the layer's B (bypass) and S (solo) buttons do not change what the preview shows, only the cue buttons do (what Resolume does there is not known); (d) the cued layers are mixed among themselves only, the lowest one drawn as it is and the others blended onto it, so one cued layer alone shows its own picture whatever its blend mode; (e) a cued layer that is empty adds nothing. (f) This is how you cue ahead of time: faders down, fire the clips or the column, cue the layers, look, then bring the faders up. (g) Mine: the master fader and the composition's position, scale and rotation are never on the preview, whatever you answer to 177; a layer's own Feedback is part of the layer and shows.
(label: "INFERRED from his DJ words; the Resolume part INFERRED (team replies 2015 and 2017); bypass in Resolume UNKNOWN")
(source: "BD:1093; facts-resolume-emulate.md B3.1, B3.2, B3.3; facts-app-cue-today.md section 4 (today a layer at fader 0 is still decoded and given its effects, and an Opaque one draws black: seen in how the app is built, not run)")
(star: true)

### R150 (about: cue system: name and thumbnail)   [NOT NAMED by him]
As in Resolume (its manual says so): a click on a clip's name selects the clip without firing it, so the Clip tab shows its settings and you can change them before it is live; a click on its thumbnail fires it. The app already works this way today. New: the name click also shows the clip in the preview monitor (as Resolume's own video shows). One thing goes: today a name click on a picture or a procedural clip can also put it on the main picture, and so on the outputs, while nothing is playing (seen in how the app is built, not seen running). A name click will only select and preview.
(label: "As in Resolume (VERIFIED: manual and Resolume's own video); today's behaviour VERIFIED in the code, its visible effect INFERRED")
(source: "facts-resolume-emulate.md B2.1, B2.2, B2.3; facts-app-cue-today.md section 2 (ClipCell.cpp:233-238; MainComponent.cpp:693-716)")

### R151 (about: cue system: a previewed clip)   [NOT NAMED by him]
As in Resolume (worked out from its old manual and from its users; its present manual has no sentence on it): a clip you preview by its name plays in the preview monitor, it is not a still, with its own effects, without the layer's fader and without the master, at the clip's own Opacity setting; previewing never fires it and never changes the output. Mine: it starts from its beginning and loops, and a BPM-mode clip follows the tempo there too; a clip that is playing in the output at that moment shows in the preview at the same moment of its playback, because it is the same clip seen twice. Mine: a clip you preview by its name plays in the preview also while the beat is stopped or paused, at the tempo number, the way a DJ hears the cued track while the room hears nothing; a cued layer shows what the layer itself holds, so a clip that stands still in the output stands still there too. Say "R151 still" if a previewed clip should stand on its first frame while the beat does not run.
(label: "As in Resolume, INFERRED there (old manual and users); the sentences after Mine: INFERRED, the last one from today's code (one player per clip)")
(source: "facts-resolume-emulate.md B2.4, B2.6, B2.7, B2.8; facts-app-cue-today.md section 5")
(star: true)

### R152 (about: cue system: the buttons)   [NOT NAMED by him]
Mine, no word of yours covers it: the cue buttons belong to the night, not to the show. They are not saved, and the app opens with none on. With none on and no clip previewed, the preview monitor shows black. A cue button can be put on a key or a MIDI pad in the keyboard and MIDI mapping: you chose that for action buttons (166 A), and I assume the same for cue buttons. A recording never keeps a cue press, and an action never presses one.
(label: "INFERRED (no word of his covers it; 166 A is about action buttons)")
(source: "BD:1093; boris-clarify-150-plus.md:64-67")

### R153 (about: cue system: the names)   [NOT NAMED by him]
Names: in your Resolume pictures the two monitors are titled "Composition Monitor" and "Clip Monitor"; "Preview" is one entry in a monitor's menu (I take it to be the lower one's: its green dot is on "Selected Clip"). In your app they are the output monitor and the preview monitor, your words. Today's panel has two small buttons at its top, "Preview" and "Output", that only change colour and show the same picture: they go. Mine: nothing else of Resolume's monitor menu (Selected Clip, Crossfader, Snapshot, Copy Image, Transform Widget) is copied; your cue buttons take its place. There is no A / B crossfader in the app and none is built.
(label: "the titles VERIFIED (his pictures); which monitor owns the menu INFERRED; today's two buttons VERIFIED in the code; what is not copied INFERRED from his words")
(source: "facts-app-cue-today.md section 1 (PreviewPanel.cpp:82-90); facts-resolume-emulate.md B1.1, B1.2, B1.13; BD:1093")
(picture: "resolume-monitors.jpg")

### R170 (about: cue system: what Resolume can do and the first build does not)   [his lines: L35]
As in Resolume (its manual says both): a monitor can be undocked into a window of its own and dragged onto a second screen, and a file can be previewed from the Files tab by a double-click. Your words ask for a preview monitor below the output monitor, and for the click on a clip's name "to preview clips before adding them to the layer strip", which I read as: before firing them. So in the first build the preview monitor is in the main window only, and it shows cued layers and clips that are in the clip grid, not files in the Files tab. Say so if you want either of the two in the first build.
(label: "As in Resolume (VERIFIED: manual); what the first build leaves out is the architects' choice, INFERRED from his words")
(source: "facts-resolume-emulate.md B4.1, B2.10; BD:1093")

### R179 (about: cue system: the edges)   [his lines: L36]
Mine, no word of yours covers these. (a) A cued layer shows in the preview what the layer itself holds: while a clip waits for its "1" on that layer, the preview shows what the layer shows in the output (question 173) and not the waiting clip; to look at the waiting clip, click its name. (b) A layer's actions show in the preview of that cued layer, because it is the same layer seen twice. (c) A clip that is not playing and that you preview by its name is shown without its actions: they play when the clip plays. (d) The clip's eight jump points in the Clip tab keep their name "Cuepoints"; everywhere else "cue" means the cue buttons under the preview monitor.
(label: "INFERRED (no word of his covers them); the on-screen word Cuepoints VERIFIED by the area sheet (ClipInspector.cpp:566)")
(source: "area-cue-layers.md H3, H11; area-actions.md M6; area-clip-transport.md U-H13; area-controls.md U20")

## DECIDED WITHOUT ASKING (3) -- he did NOT read these: each is still Harmony's assumption until his words settle it
### D7: The preview monitor is never sent to an output, never recorded, and never reaches Syphon.
(decided by: His words ask for a monitor to look at before mixing (BD:1093); a second screen for it is reading R170.)

### D8: The preview picture is smaller than the output and may run less smoothly; the output's smoothness always comes first. Its cost is measured before it is built, and if it costs the output anything I tell you first, before it is built, how many frames per second the output would lose.
(decided by: Today the app draws one picture per frame and nothing else (facts-app-cue-today.md sections 1 and 5); no measurement exists yet.)

### D9: A cued layer shows in the preview the same moment as the output: it is the same layer seen twice. A layer's Echo, Freeze or Feedback may show a slightly different trail in the preview than in the output. One limit of the first build: a generated picture with a running state of its own (a simulation), or a MilkDrop clip, of the same kind as one that is playing shows the playing one's state in the preview. The build is held to it that a preview never disturbs the output; that is not yet shown to be possible.
(decided by: One state per layer, one player per clip and one generator per kind of generated picture today (facts-app-cue-today.md section 5; area-cue-layers.md T1, T2: previewing a kind that is on a layer may write into the live copy); INFERRED from the code, not run.)

## COMES NEXT AS PICTURES (3) -- he did NOT read these; his new rule on layout is L8
### P3: The two monitors: where the preview monitor goes below the output monitor, and how big each is (today the one monitor is the left-most of four panels at the bottom, about a fifth of the width).
(variants that were to be drawn: stacked in today's left column, both smaller | the left column made wider for the two | the preview monitor foldable, so the output monitor is big when you do not cue)

### P4: The row of cue buttons under the preview monitor.
(variants that were to be drawn: one numbered button per layer, lit when cued | one button per layer carrying the layer's name | the same, plus a small picture of each layer)

### P5: The preview monitor's title line, which says what it shows now, and what it shows when nothing is cued.
(variants that were to be drawn: "Preview: layers 1, 2" or "Preview: the clip's name"; black when nothing is cued | the same, with the words "nothing cued" on the black | the same, but it shows the output, dimmed, when nothing is cued)

