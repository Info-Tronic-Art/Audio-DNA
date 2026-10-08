# SPEC B -- The cue system (s-rta-1007): the paper apply-B.md with the ruling rule-B.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 176
TITLE: A clip's name clicked while layers are cued
STATUS: ANSWERED a, with the return rule replaced by his R170 words
HIS: L37, L35, L16
RULE: The preview monitor has two modes, cue mode and preview mode, and one toggle button that switches between them (L35: "we will have a toggle between cue mode and preview mode"). A single click on a clip's name in the deck switches the monitor to preview mode at once, with no other press (L35: "The preview will just happen frictionlessly"): the clip takes the whole preview monitor, alone, outside the cued mix (the page's option A; L37: "176 a but read R170 for detail"). The cue buttons keep their state while the monitor is in preview mode, and a cue button pressed meanwhile still switches its layer in the cued mix (INFERRED). The cued mix is seen again only when he pushes the toggle button (L35: "to see the cue you need to push the toggle button", and again "to get back to queue you need to push the toggle button"). Firing a clip, pressing a cue button or clicking the same name again does not bring the cued mix back (INFERRED from his "you need to", said twice: it replaces the three ways back of option A's text). The name click also selects the clip and fills the Clip tab (L16), never fires it and never changes the output. When the previewed clip starts to move: R151, R179 and assumptions B-1 and B-2.
CHANGED: Option A's last sentence is replaced. The page said "The cued layers come back when you fire any clip, press a cue button, or click that name again"; his words say only the toggle button brings them back (L35). Added: the two modes and the toggle. Option B (the clip shown inside the cued mix) is not built. Assumption B-6 is dropped: his words settle it.
TODAY: No preview monitor, no modes: one panel that shows the composition canvas (CT section 1; AC 1a items 1-4). A name click selects and, for a picture or a generated clip, loads it into the main renderer's fallback slot, which reaches the outputs when nothing plays (CT section 2; src/MainComponent.cpp:693-716). All of it is new work.
@@END

@@ITEM 177
TITLE: Global effects in the preview: the master cue button
STATUS: ANSWERED in his own words (a switch between A and B)
HIS: L38, L36
RULE: A toggle button called "master cue" is added to the cue controls (L38: "add a global effects and actions toggle button called master cue"). Master cue OFF: the preview shows the cued layers with their own clip effects and layer effects, mixed, and the global effects are not on it (the page's option A). Master cue ON: the cued mix is also run through the global effects exactly as they are on the output at that moment, moved by whatever global actions are running, so the preview looks the way the mix would look on the output (the page's option B). His word "actions" is read as the global actions (INFERRED): a clip's own actions and a layer's actions show in the preview whatever master cue says (L36: "a previewed clip (clicking its name) shows the clip with all its actions"; R179 b, which he passed with "all good"). Master cue only decides what the preview shows: it never changes the output and never starts, stops or tries out an action (assumption B-4). It works in cue mode only; a clip or file shown alone in preview mode does not get the global effects (assumption B-5). The master fader and the global Transform (position, scale, rotation) are never applied to the preview, whatever master cue says (R149 g). Master cue opens off, is not saved with the show and is never recorded (assumption B-7); it can be put on a key or a pad (question 206 at its default). Global effects that remember earlier pictures (trails, echo) may look slightly different in the preview than in the output (D9, assumption B-11). Where the button sits: with the cue buttons under the preview monitor, laid out by Harmony (L8).
CHANGED: Neither letter was taken; both options stay, chosen live by the new button. Added: the button, its name, and the word "actions" (the page spoke of global effects only).
TODAY: The global effects run once per frame on the one canvas, with their own history key (CT section 5, the line on applyGlobalEffects, Renderer.cpp:714). No second picture exists (CT section 5, first line). A second run of the global stack for the preview needs its own history, or its trails are approximated (D9).
@@END

@@ITEM R141
TITLE: The preview monitor, cue buttons, and a transparency slider each
STATUS: CORRECTED one thing added (the slider); the fader sentence changes with it (how: assumption B-3)
HIS: L34, L111
RULE: A preview monitor sits just below the output monitor. Under it, one cue button per layer; any number of them can be on. Beside each cue button sits a transparency slider for that layer (L34: "next to each cue button for each layer, there's also a transparency slider so we can see what the transparency would be in the preview monitor"). The layers whose cue button is on show in the preview mixed together, stacked in layer order, each laid over the cued layers below it the way its own layer settings say (L34: "of course using the transparency setting in the layer settings and the layer stacked in their correct order"). His "the transparency setting in the layer settings" is read as the layer's blend mode, the setting that the layer's transparency slider works through (L111: "We are only going to use the transparency slider to control that layers blend mode"); the other reading, the layer's own transparency value, is the b) of assumption B-3. As assumed (B-3): the slider beside a cue button is a try-out for the preview only; it sets how strongly that layer shows in the preview; it starts at full; it never moves the layer's own transparency slider, and that slider never moves it. With or without the global effects and the global actions: the master cue button (177). Nothing done on the cue controls ever changes the output. This multi-layer cue is his own and not Resolume's (one monitor there shows one layer or one group). A click on a clip's name: 176, R150, R151, R179.
CHANGED: Added: the transparency slider beside each cue button (L34). Replaced: "except that each cued layer's own fader is ignored (R149 a)": the strength in the preview is set by the new slider; how that slider relates to the layer's own slider is assumption B-3, not his word. Replaced: "(with or without the global effects: question 177)" becomes the master cue button (L38). His "the transparency setting in the layer settings" is read as the layer's blend mode (INFERRED from L111, and from the page's own "with their blend modes, in layer order", which his sentence follows); the other reading is in B-3. The rest stands.
TODAY: No cue or preview control exists on the layer strip or the monitor (AC 1c item 16; CT section 4). A per-layer picture taken after the layer's effects and before its transparency and blend already exists for the Layer Router (CT section 5, saveLayerOutput, CompositorEngine.cpp:1150-1154): the cued mix can be blended from those copies without running a layer twice. A bypassed layer is skipped before it is drawn (AC 1d item 17), so it has no such copy yet.
@@END

@@ITEM R149
TITLE: A cued layer and its fader, bypass, solo, blend
STATUS: REPLACED in part (a) by L34 (how the new slider relates to the layer's own: assumption B-3) and in (g) by L38; (b) to (f) stand
HIS: L34, L38, L29, L51, L55
RULE: (a) A cued layer shows in the preview at the strength that the slider beside its cue button sets (L34). As assumed (B-3) that is so wherever the layer's own transparency slider is: with the layer's own slider all the way down and its cue slider up, the layer is fully seen in the preview and not at all in the output. (b) The preview shows the layer's clip with the clip's effects and the layer's effects. (c) The layer's B (bypass) and S (solo) buttons do not change what the preview shows; only the cue buttons do. A bypassed layer's clip plays on out of sight, so the preview of a cued bypassed layer moves (his Resolume look L29: "it plays out of sight"; evidence, the rule itself is in topic H). (d) The cued layers are mixed among themselves only, in layer order (L34): the lowest cued one is drawn as it is, over black, and the others are blended onto it with their own blend mode, so one cued layer alone shows its own picture whatever its blend mode. The cue slider of the lowest cued layer fades that layer against the black (assumption B-20). (e) A cued layer that is empty adds nothing. A cued layer that holds only effects acts on the cued layers below it and does nothing as the lowest cued one (L51, assumption B-13). (f) Cueing ahead of time: the layers' own sliders down, fire the clips or the column, switch the cue buttons on, look and try the mix with the cue sliders, then bring the layers' own sliders up. (g) The master fader and the global Transform (position, scale, rotation) are never on the preview. The global effects, with what the global actions do to them, are on it only while master cue is on (177). A layer's own Feedback is part of the layer and shows.
CHANGED: (a) replaced: the page said "at full strength wherever its fader is"; a slider beside the cue button now sets the strength (L34); that it starts at full and is independent of the layer's own slider is assumption B-3, not his word. (d) gains the cue slider of the lowest cued layer (B-20). (e) gains the effects-only layer (L51). (f) gains "try the mix with the cue sliders" and holds as written only under B-3. (g): "whatever you answer to 177" becomes the master cue button (L38); "the composition's" becomes "global" (L55). (b), (c) stand as shown; (c) and (d) are still Harmony's own, not named by him.
TODAY: A layer at transparency 0 is still decoded and given its effects; one of the Opaque kind draws black over everything below (AC 1d item 18; CT section 4). Bypass and solo skip the layer in the one real picture (AC 1d item 17). What a layer's type and blend menu become: topic I (his L111).
@@END

@@ITEM R150
TITLE: Name click selects and previews; thumbnail click fires
STATUS: STANDS confirmed by his Resolume look and extended by L35
HIS: L16, L35
RULE: A click on a clip's thumbnail fires it. A click on a clip's name selects the clip without firing it: the Clip tab shows its settings, so they can be changed before it is live, and the clip is shown in the preview monitor, which switches to preview mode (L16: "when you click Lipp's name, it plays in the preview and populates the clip tab"; L35). A name click only selects and previews: it never puts anything on the main picture or on an output.
CHANGED: nothing in the reading. Added from L35: the name click also switches the monitor to preview mode. "Lipp's" is read as "clip's" (INFERRED, a typing slip).
TODAY: Thumbnail = fire and name bar = select are built (AC 1b item 7; ClipCell.cpp:231-239). The name click's load into the fallback slot, which can reach the outputs when nothing plays, has to go (AC 1b items 8-9; AC section 3 O3; src/MainComponent.cpp:693-716).
@@END

@@ITEM R151
TITLE: How a clip previewed by its name plays
STATUS: REPLACED in part by L36 (starts on the 1, with its actions); the rest stands, the stopped or paused tempo as assumption B-2
HIS: L36, L35, L16, L12, L13
RULE: A clip previewed by its name plays in the preview monitor, it is not a still (L16), with its own effects and with all its own actions (L36), without its layer's effects (assumption B-5), without the layer's transparency and without the master, at the clip's own Opacity setting. Previewing never fires it and never changes the output. WHILE THE TEMPO RUNS it is triggered on the "1" (L36: "it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music"). As assumed (B-1, because L35 says "They will play right away"): the preview takes the clip over at the click, the clip stands on its first frame while it waits, and on the next "1" it starts from its beginning with its actions; a Resync while it waits is that "1" (INFERRED from L12, as topic A rules it for a fired clip). It loops; a BPM-mode clip follows the tempo there too. WHILE THE TEMPO IS PAUSED OR STOPPED no "1" is coming. As assumed (B-2): the clip still plays in the preview, with its actions, at the tempo number, starting at the click; a preview that is already playing when the tempo is paused or stopped plays on in the same way; previewing never starts or resumes the tempo; and when the tempo runs again the previewed clip is triggered again on the next "1", so that it is in time with the music (INFERRED from L36). A CLIP THAT IS PLAYING IN THE OUTPUT is not started again by a click on its name: the preview shows it at the same moment of its playback, with the same action values, and a previewed clip that he then fires is shown from then on as it plays in the output (assumption B-8). A second click on the name of the clip that is already previewed starts it again on the next "1" (assumption B-14). A cued layer (cue mode) shows what the layer itself holds, so a clip that stands still in the output stands still there too.
CHANGED: Replaced: "it starts from its beginning and loops" at the click becomes "starts from its beginning on the 1" (L36). Added: all its own actions play (L36; the page's R179 c had them left out). The page's sentence on the stopped or paused beat is kept as the assumed rule and is asked (B-2): he did not say "R151 still", but he read it before he wrote L36, which puts the start on a "1", and his pause holds "the clips and everything that it controls with BPM" (L13). Added by Harmony, not by his words: what happens when the tempo runs again, a Resync while a preview waits, a second click on the same name, a fire of the previewed clip.
TODAY: A video clip has an open, parked player, but no clip can be drawn outside a layer (CT section 5, the line "Can it draw a clip that is on no layer"; AC T3). One player per clip: a clip seen twice is the same playback (CT section 5, the video line). Clip actions are not built (AC section 2 item 7). A preview that plays while the show's beat stands (B-2) needs a beat of its own: not checked.
@@END

@@ITEM R152
TITLE: Cue controls belong to the night, not the show
STATUS: STANDS extended by Harmony to the new controls (assumption B-7)
HIS: none
RULE: The cue buttons are not saved with the show and the app opens with none on. The same holds for the cue transparency sliders, master cue (opens off) and the cue / preview toggle (opens in cue mode) (assumption B-7; where a cue slider starts: assumption B-3). With no cue button on, cue mode shows black; with nothing previewed yet, preview mode shows black (assumption B-19). Every one of these controls can be put on a key, a MIDI pad or a knob in the keyboard and MIDI mapping (question 206 at its default: every button and every slider can be mapped). A recording never keeps a press or a move of them, and an action never presses or moves them.
CHANGED: nothing in the reading. Added by Harmony, not by his words: the three new controls of L34, L35 and L38 follow the same rule (assumption B-7); black in both modes when there is nothing to show (B-19, which also carries the one variant of picture item P5 that differed in what the app does).
TODAY: None of these controls exists (CT section 4). The show file stores no cue state (not checked beyond the absence of any cue control).
@@END

@@ITEM R153
TITLE: The names: output monitor and preview monitor
STATUS: STANDS
HIS: L34, L35
RULE: The two monitors are called the output monitor and the preview monitor (his words: L34 "preview monitor", L35 "the preview window under the output window"). The preview monitor's two modes are called cue mode and preview mode (L35). The two small buttons "Preview" and "Output" at the top of the monitor panel go. Nothing of Resolume's monitor menu is copied (Selected Clip, Crossfader, Snapshot, Copy Image, Transform Widget): the cue buttons, their sliders, master cue and the cue / preview toggle take its place. There is no A / B crossfader and none is built.
CHANGED: nothing in the reading. Added: the mode names from L35. In L35 "queue" ("to get back to queue") is read as "cue" (INFERRED, a typing slip).
TODAY: The panel's "Preview" and "Output" buttons only change colour and no code reads them (CT section 1 and its correction; PreviewPanel.cpp:82-90). The model carries four unused crossfader fields (AC 1d item 24).
@@END

@@ITEM R170
TITLE: No undocking yet; files are previewed; cue and preview modes
STATUS: CORRECTED files are previewed in the first build; the toggle is added
HIS: L35, L11, L13
RULE: The preview monitor stays in the main window; undocking it or moving it to a second screen is not built yet (L35: "no need to undock and move just yet"). Besides the cued layers, the preview monitor previews (1) a clip whose name is single-clicked in the deck and (2) a file that is double-clicked in the files window (L35: "we also want to preview the clips double clicked from the files window and name clicked from a clip in the deck"). Either click takes the preview over at once, with no other press, and switches the monitor to preview mode (L35: "Double clicking or single clicking the name will take over the preview"). One toggle button switches between cue mode and preview mode; pushing it is the only way back to the cued mix (L35: "to get back to queue you need to push the toggle button"). Pushed again it returns to preview mode and shows the last previewed clip or file (assumption B-14). A file from the files window is not a clip yet: it plays right away from its start (L35: "They will play right away"), also while the tempo is paused or stopped (it is not in BPM mode and has no actions, so nothing of it waits for the tempo: INFERRED from L11 and L13), loops, alone, without effects or actions, shown whole and never stretched (assumption B-21). A double-click only previews: nothing goes into a deck and nothing reaches the output; which windows preview by a double-click is assumption B-9. When a deck clip previewed by its name starts to move: R151 and assumptions B-1 and B-2. His sentence "We will have various configurations other than live and recording review mode" announces more screen arrangements later; nothing is built from it in this topic (assumption B-17).
CHANGED: Replaced: "it shows cued layers and clips that are in the clip grid, not files in the Files tab" (files are in, L35). Confirmed: the monitor is in the main window only. Added: cue mode, preview mode, the toggle button, and that only the toggle returns to the cue.
TODAY: A double-click on a file in the Files tab loads it as a picture into the main renderer's fallback slot and sets the file label (src/ui/FilesBrowser.cpp:101-109; src/MainComponent.cpp:1506-1509); that slot is drawn on the real canvas when nothing plays (AC 1b item 9). That path has to go and a preview path has to be built.
@@END

@@ITEM R179
TITLE: The edges: waiting clip, actions in the preview, Cuepoints
STATUS: CORRECTED part (c) replaced; (a), (b), (d) stand ("all good")
HIS: L36, L56
RULE: (a) A cued layer shows in the preview what the layer itself holds: while a clip waits for its "1" on that layer, the preview shows what the layer shows in the output and not the waiting clip; to look at the waiting clip he clicks its name, which switches the monitor to preview mode. (b) A layer's actions show in the preview of that cued layer, because it is the same layer seen twice. (c) A clip previewed by its name is shown with all its own actions, and it is triggered on the "1" so that the actions play in time with the music (L36: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1"). His "all its actions" is read as the clip's own actions that are switched on (assumption B-18): switching one of them on or off, in its row below the clip (L56), shows in the preview by the same timing as in the show and is a real change to the clip. When no "1" is coming, and against L35's "They will play right away": R151 and assumptions B-1 and B-2. (d) The clip's eight jump points in the Clip tab keep their name "Cuepoints"; everywhere else "cue" means the cue controls under the preview monitor.
CHANGED: (c) replaced: the page said a clip that is not playing "is shown without its actions: they play when the clip plays". Added to (c): the start on the "1". (a), (b), (d) stand (L36: "all good except").
TODAY: Clip actions are not built (AC section 2 item 7: only his words). The word Cuepoints is on screen in the Clip tab (ClipInspector.cpp:566, taken from the page's label; not re-opened).
@@END

@@ITEM D7
TITLE: The preview never reaches an output, a recording or Syphon
STATUS: SETTLED for outputs and recordings; Syphon follows (assumption B-12)
HIS: L35, L88
RULE: The preview monitor is shown in the main window only. It is never sent to an output screen (L35: "no need to undock and move just yet"), never part of a recording (L88: "Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues. It must be the full screen composition.") and never sent to Syphon. A later build may put it on a second screen when he asks for that.
CHANGED: nothing. His "just yet" keeps a second screen for the preview open for later.
TODAY: Outputs, the recorder and Syphon all read the one composition canvas (CT section 1, the publishToOutputs line; AC 1a item 4). A preview drawn into its own target never enters that path.
@@END

@@ITEM D8
TITLE: The preview may be smaller and less smooth
STATUS: OPEN still Harmony's own (assumption B-10)
HIS: none
RULE: The preview picture is smaller than the output and may run less smoothly; the output's smoothness always comes first. Its cost is measured before it is built, and if it would cost the output anything he is told first how many frames per second the output would lose.
CHANGED: nothing; no word of his touches it. The new parts (a slider per cued layer, master cue, files previewed) add to what has to be measured.
TODAY: One picture per frame is drawn and nothing else; no measurement of the frame cost on his machine exists (CT section 5, the frame-budget line; CT "what I could not establish").
@@END

@@ITEM D9
TITLE: Same moment as the output; limits of the first build
STATUS: OPEN still Harmony's own (assumption B-11)
HIS: L126
RULE: A cued layer shows in the preview the same moment as the output: it is the same layer seen twice. A layer's Echo, Freeze or Feedback, and the global effects under master cue, may show a slightly different trail in the preview than in the output. One limit of the first build: a generated picture with a running state of its own (a simulation), or a MilkDrop clip, of the same kind as one that is playing shows the playing one's state in the preview, not a fresh run of its own; MilkDrop itself is not changed in this build (L126: "that's it for this upcoming build"). The build is held to it that a preview never disturbs the output.
CHANGED: nothing in the decision. Added: the global effects under master cue share the trail limit. His L126 keeps MilkDrop as it is for this build, so its limit stays.
TODAY: One history per layer, one player per clip, one generator per kind of generated picture, one MilkDrop instance (CT section 5; AC T1, T2). That a preview can be drawn without disturbing the output is INFERRED from the code, not shown (see NOT DONE).
@@END

@@ITEM P3
TITLE: Where the two monitors sit and how big
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Fixed by his words: the preview monitor is below the output monitor (BD:1093) and stays in the main window (L35).
CHANGED: The three drawn variants are not shown to him.
TODAY: One monitor, the left-most of four bottom panels, 22 percent of the width by default (CT section 1; AC 1a item 3).
@@END

@@ITEM P4
TITLE: The row of cue controls under the preview monitor
STATUS: DROPPED
HIS: L8, L34
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Fixed by his words: the controls are under the preview monitor (BD:1093), one cue button per layer with its transparency slider next to it (L34); master cue and the cue / preview toggle sit with them.
CHANGED: The three drawn variants (numbered, named, with a small picture) are not shown to him.
TODAY: No cue control exists (CT section 4).
@@END

@@ITEM P5
TITLE: The preview monitor's title line and its empty state
STATUS: OPEN one variant differs in what the app does (assumption B-19); the look is dropped (L8)
HIS: L8, L130
RULE: The title line is laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What the monitor shows when it has nothing to show is a function, not a look: with no cue button on, cue mode is black; with nothing previewed yet, preview mode is black (assumption B-19; the same as R152). The variant that showed the output, dimmed, when nothing is cued is not built unless he asks for it.
CHANGED: The three drawn variants are not shown to him. The one that differed in what the app does is put to him as one line (B-19), because he did not read this list (L130).
TODAY: not checked (no preview monitor exists).
@@END

## ASSUMPTIONS
@@ASSUME B-1
ABOUT: R151 R179 R170 176
TEXT: I assume a clip you preview by its name takes the preview over at once, stands on its first frame, and starts on the next 1 with its actions. A file from the files window plays at once.
WHY: L35 says previews "will play right away"; L36 says a name-previewed clip "is triggered on the 1". The two lines pull apart.
ALT: b) Every preview plays at once; only its actions wait for the next 1. c) Only a clip that has actions or is in BPM mode waits for the 1; any other plays at once.
IF-WRONG: STAGE he sees a previewed clip wait up to a bar, or start at once, against what he expects
ASK: YES his own two lines differ and it decides how the preview clock is built
@@END

@@ASSUME B-2
ABOUT: R151 R179
TEXT: I assume that while the tempo is paused or stopped, a clip you preview by its name still plays in the preview, with its actions, at the tempo number, starting at your click. Previewing never starts the tempo.
WHY: L36 puts the preview's start on the 1; paused or stopped, no 1 comes, and his pause holds "the clips and everything that it controls with BPM" (L13).
ALT: b) It waits on its first frame until the tempo runs again and reaches the 1, as a fired BPM clip waits while paused. c) Its picture plays, but its actions hold still until the tempo runs.
IF-WRONG: STAGE before a show starts he could not watch a previewed clip move; and the assumed way needs a beat of its own for the preview
ASK: YES the page's sentence predates L36 and his L13 pulls against it; he would see it; it decides whether the preview gets a beat of its own
@@END

@@ASSUME B-3
ABOUT: R141 R149 R152
TEXT: I assume the slider beside a cue button is a try-out for the preview only. It starts at full and never moves the layer's own transparency slider, and that slider never moves it.
WHY: L34 "using the transparency setting in the layer settings" can mean the layer's blend mode (L111) or its transparency value; the slider's tie to the layer's own is not said.
ALT: b) It starts at the layer's own transparency and follows it until you move it. c) It is the layer's own transparency slider shown a second time: moving it changes the output too.
IF-WRONG: STAGE with c) a move of it reaches the audience; with b) a cued layer whose own slider is down shows nothing until he raises it
ASK: YES his words can be read three ways, he would see the difference at the first cue, and it decides what the slider is
@@END

@@ASSUME B-4
ABOUT: 177
TEXT: I assume master cue on shows the cued layers through the global effects exactly as they are on the output at that moment, moved by any global action that is running. Master cue off leaves the global effects out.
WHY: L38 says only "a global effects and actions toggle button called master cue"; what the "actions" half does in a preview is not said.
ALT: b) With master cue on you can also try out a global action in the preview before the audience sees it. c) Master cue on shows the whole output in the preview, as on a DJ mixer.
IF-WRONG: REBUILD trying a global action in the preview alone needs a second, preview-only copy of the global effects and their actions (a clip's own actions already play for the preview alone, L36); and if b) is what he means, he would press a global action expecting to see it first
ASK: YES no word of his says what the actions half of the button does, and b) is a far bigger build
@@END

@@ASSUME B-5
ABOUT: 177 R151
TEXT: I assume master cue works in cue mode only. A clip or file shown alone in preview mode has its own effects and actions, never its layer's effects or the global ones.
WHY: L38 names no mode; L36 says a previewed clip shows "with all its actions" and names no layer or global part.
ALT: b) Master cue also puts the global effects on a clip shown alone. c) A clip shown alone also gets its layer's effects.
IF-WRONG: SMALL one more pass on the preview picture
ASK: LINE a real choice he would most likely wave through
@@END

@@ASSUME B-7
ABOUT: R152 R141 177 R170
TEXT: I assume the cue sliders, master cue and the cue / preview toggle are like the cue buttons: not saved with the show, never recorded, never moved by an action. The app opens in cue mode, master cue off.
WHY: L34, L35 and L38 add three controls and say nothing on saving, recording or their state at opening.
ALT: b) They are saved with the show and come back as they were left.
IF-WRONG: SMALL a few saved fields
ASK: LINE it extends a rule he read and let stand to controls he added later
@@END

@@ASSUME B-8
ABOUT: R151
TEXT: I assume a clip already playing in the output shows in the preview exactly as it plays there when you click its name: it is not started again. A previewed clip that you then fire is shown the same way.
WHY: L36 says a previewed clip "is triggered on the 1" and does not name a clip that is already playing, or one that is fired while previewed.
ALT: b) The preview shows a second, separate playback of the clip from its beginning.
IF-WRONG: SMALL seen in the preview only
ASK: NO the first sentence was on the page and he let it stand; a restart would disturb the output
@@END

@@ASSUME B-9
ABOUT: R170
TEXT: I assume a double-click in the files window only previews the file, from its start, looping, without effects: nothing goes into a deck or onto the output. Sources and MilkDrop presets are not previewed by a double-click.
WHY: L35 names "the clips double clicked from the files window" only; what else a double-click does, and other windows, are not said.
ALT: b) A double-click in the Sources tab and the MilkDrop browser previews too.
IF-WRONG: SMALL one more double-click handler per window, apart from MilkDrop
ASK: LINE a real choice he would most likely wave through
@@END

@@ASSUME B-10
ABOUT: D8
TEXT: I assume the preview picture may be smaller and less smooth than the output, and the output always comes first. If the preview would cost the output any frames, I tell you the number before it is built.
WHY: No word of his covers the cost of a second picture; he did not read this point (L130).
ALT: b) The preview must be as sharp and as smooth as the output.
IF-WRONG: STAGE a preview at full quality could cost the output frames
ASK: LINE he did not read it and asks to be shown what is still assumed (L130)
@@END

@@ASSUME B-11
ABOUT: D9 177
TEXT: I assume two limits in the first build. Trails (Echo, Freeze, Feedback) may look different in the preview. A previewed simulation or MilkDrop clip shows the motion of one of the same kind that is playing, not a fresh run.
WHY: No word of his covers it; L126 keeps MilkDrop as it is for this build; he did not read this point (L130).
ALT: b) Every previewed picture must be fully its own, which needs a second copy of each simulation and of MilkDrop.
IF-WRONG: REBUILD second copies of every stateful picture are a large addition
ASK: LINE he did not read it and asks to be shown what is still assumed (L130)
@@END

@@ASSUME B-12
ABOUT: D7
TEXT: I assume the preview monitor never reaches Syphon, an output screen or a recording.
WHY: L35 and L88 cover a second screen and recordings; Syphon is not named.
ALT: none
IF-WRONG: SMALL
ASK: NO it follows from his words on outputs and recordings
@@END

@@ASSUME B-13
ABOUT: R149 176
TEXT: I assume a cued layer that holds only effects acts, in the preview, on the cued layers below it; as the lowest cued layer it does nothing. Previewed alone by its name, such a cell shows black.
WHY: L51 gives this rule for the output; applied to the cued mix by his L9. Alone, an effect has no picture to act on (176 a).
ALT: b) In the preview it acts on every layer below it, cued or not. c) Previewed by its name, it is shown acting on the cued layers.
IF-WRONG: SMALL
ASK: NO his own rule, carried over; the edge of a cell previewed alone is cheap to change
@@END

@@ASSUME B-14
ABOUT: R170 176 R151
TEXT: I assume a preview lasts until another clip or file is previewed. In cue mode it plays on out of sight, and the toggle shows it again. A second click on the same name restarts it on the next 1.
WHY: L35 says how a preview starts and how to get back to the cue, not how long a preview lasts or what a second click does.
ALT: b) Going back to cue mode ends the preview; preview mode is then black until the next click. c) A second click on the same name changes nothing.
IF-WRONG: SMALL
ASK: NO internal; nothing of it reaches the output
@@END

@@ASSUME B-17
ABOUT: R170
TEXT: I assume your words on "various configurations other than live and recording review mode" ask for nothing in the cue system now.
WHY: L35 announces them and describes none.
ALT: b) He expects a named set of screen arrangements in the first build.
IF-WRONG: SMALL belongs to the screens topic
ASK: NO carried to the topic of the review screen and the show file (L94)
@@END

@@ASSUME B-18
ABOUT: R179
TEXT: I assume "all its actions" means the clip's own actions that are switched on. Switching one on or off shows in the preview, by the same timing as in the show, and is a real change to the clip.
WHY: L36 says "with all its actions"; whether actions that are switched off also play is not said.
ALT: b) Every action stored on the clip plays in the preview, switched on or not.
IF-WRONG: SMALL
ASK: NO follows the rule that an action plays only while its button is on
@@END

@@ASSUME B-19
ABOUT: P5 R152
TEXT: I assume the preview monitor is black when it has nothing to show: no cue button on in cue mode, or nothing previewed yet in preview mode.
WHY: He did not read the picture list (L130); one of its variants differed in what the app does; the page's own line said black.
ALT: b) With no cue button on, it shows the output, dimmed.
IF-WRONG: SMALL one branch of the preview picture
ASK: LINE a function variant of a list he did not read (L130); the page said black and he let that stand
@@END

@@ASSUME B-20
ABOUT: R149 R141
TEXT: I assume the lowest cued layer is drawn over black, and its cue slider fades it against that black. One cued layer alone shows its own picture whatever its blend mode.
WHY: L34 adds a slider per cued layer; the page drew the lowest cued layer as it is and did not say what its slider fades it against.
ALT: b) The slider of the lowest cued layer does nothing.
IF-WRONG: SMALL
ASK: NO a detail of drawing the preview; the second sentence was on the page and stands
@@END

@@ASSUME B-21
ABOUT: R170
TEXT: I assume a file shown in preview mode is shown whole, never stretched, and that the preview goes black when the clip or file it shows is removed.
WHY: L35 asks for files in the preview; a file is not a clip yet and has no fit setting; a removal is not named.
ALT: b) A previewed file fills the monitor, cropped.
IF-WRONG: SMALL
ASK: NO how a previewed file is fitted is a look (L8); the removal case is internal
@@END

## QUESTIONS BACK
## NAMES
@@NAME preview monitor
MEANS: The second monitor, below the output monitor, that shows cued layers or one previewed clip or file and never reaches the audience.
SOURCE: his words L34, L35 (and binding-decisions.md 1093)
@@END

@@NAME output monitor
MEANS: The monitor that shows the composition as the outputs get it.
SOURCE: his words binding-decisions.md 1093 ("resolume has a output monitor"); L35 "the output window"; the on-screen name now is a panel with two buttons "Preview" and "Output", which is replaced by this name
@@END

@@NAME cue button
MEANS: One button per layer under the preview monitor; on = that layer is in the cued mix of the preview.
SOURCE: his words L34 ("each cue button for each layer")
@@END

@@NAME cue transparency slider
MEANS: The slider beside a layer's cue button that sets how strongly that layer shows in the preview only; "cue slider" for short.
SOURCE: Harmony's pick, from his words L34 ("a transparency slider")
@@END

@@NAME cue mode
MEANS: The state of the preview monitor in which it shows the cued layers mixed.
SOURCE: his words L35
@@END

@@NAME preview mode
MEANS: The state of the preview monitor in which it shows one clip or one file, alone.
SOURCE: his words L35
@@END

@@NAME cue / preview toggle
MEANS: The one button that switches the preview monitor between cue mode and preview mode.
SOURCE: Harmony's pick, from his words L35 ("a toggle between cue mode and preview mode", "the toggle button")
@@END

@@NAME master cue
MEANS: The toggle button that puts the global effects and actions on the preview, or leaves them off.
SOURCE: his words L38
@@END

@@NAME files window
MEANS: The list of files on disk, where a double-click previews a file.
SOURCE: his words L35; the on-screen name now is the "Files" tab of the browser panel (kept or replaced: for the list of names, L114)
@@END

@@NAME Cuepoints
MEANS: A clip's eight jump points in the Clip tab; the only other use of the word cue on screen.
SOURCE: the on-screen name now, kept (reading R179 d, "all good" L36)
@@END

@@NAME transparency
MEANS: His word for how strongly a layer shows (the layer's own slider, and the cue slider).
SOURCE: his words L34, L111; the on-screen name now is the strip's "V" slider (opacity), which is replaced by his word in the list of names (L114)
@@END

@@NAME the cue
MEANS: The mix of the layers whose cue button is on, as cue mode shows it; the papers also say "the cued mix".
SOURCE: his words L35 ("to see the cue you need to push the toggle button")
@@END

## CONFLICTS (from the paper, unruled)
- L35 "They will play right away and we will have a toggle between cue mode and preview mode" against L36 "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". Read as: the preview takes over at once, a deck clip starts on the next 1, a file plays at once. Asked as B-1 (ASK: YES).
- Not a clash, settled by himself: L37 "176 a but read R170 for detail" takes option A, whose text says the cued layers come back on a fire, a cue button or the name again; L35 "to see the cue you need to push the toggle button" replaces that return rule (B-6, ASK: NO).

## NOT DONE / UNSURE (from the paper, unruled)
- Whether a second picture can be drawn every frame without disturbing the output is INFERRED from the code only (CT section 5; AC T1-T6). Cheapest way to settle: when the build hold lifts, one measured spike before the cue lane is planned: blend the saved per-layer pictures into a small second target and read frame_time_ms with and without it. Not run here (nothing may be built or run).
- A cued layer that is bypassed has no picture of its own in the present draw order (AC 1d item 17, T6); R149 c needs it. How a bypassed layer keeps playing is topic H (his L29 look); the cue lane depends on that rule.
- A name-previewed clip that starts on the "1" with its actions needs the actions engine, which is not built (topic D). The cue lane can ship the picture first and the actions with topic D; the order is Harmony's to plan.
- Master cue ON needs a second run of the global effects with a history of their own, or approximated trails (B-11). Sized only after the spike above.
- "Various configurations other than live and recording review mode" (L35) is not turned into a rule here (B-17); it belongs with the review screen and the show's layout (L94).
- What Resolume does with a clip preview while its tempo is paused or stopped is not known (RE C3.8); it does not bind (his L36 already departs from Resolume).
- The assumption numbers B-15 and B-16 are not used (merged into B-5 and B-1).

## FOR THE PAGE RULING (from the ruling)
- Four ASK items: B-1 (L35 "They will play right away" against L36 "it is triggered on the 1"), B-2 (a preview while the tempo is paused or stopped), B-3 (the slider beside a cue button: L34 read three ways; it leans on topic I's reading of L111), B-4 (what the "actions" half of master cue does, L38).
- B-2 is the same question as X-24 (ASK YES there too; its ALT c, that previewing starts the tempo, is shut out here by the TEXT's last sentence): one item. Its default is an exception to topic D's timing rule (no action plays while the beat is stopped; all hold while paused) and needs a beat of its own for the preview. B-2 falls if he answers B-1 with b).
- More to merge: B-10 = X-11 (the cost of the preview picture: LINE here, NO there); B-7 overlaps F-18 (cue controls not saved) and J-15 (never undone); B-9 borders C-9 (the Effects tab's double-click, L41) and K-3 (a click in the MilkDrop tab); B-11 answers topic K's open note on a generated picture of a kind that is also live.
- Names: "master cue" here, "Master Cue" in topic J: one spelling in the list of names (his typing, L38: "master cue"). "the cue" is his word (L35) for what the papers call the cued mix. "cue transparency slider" and "cue / preview toggle" are Harmony's picks.
- A name click both selects and previews (L16, L35): selecting a clip to copy it (L4, topic F) also takes over the preview monitor. Which of several selected clips is previewed is not ruled here (best guess: the last one clicked).
- Leans on other topics: R149 c needs topic H's rule that a bypassed layer plays on out of sight (L29); L36 needs topic D's engine to play a clip's own actions for the preview alone; B-8 answers apply-D's open note (a clip previewed and playing is one clip seen twice, with the same action values).
- The merge keeps the paper's CONFLICTS section unruled. Missing there: L34 "the transparency setting in the layer settings" read two ways (B-3); L36 against the page's standing sentence on a stopped or paused tempo and his L13 (B-2). It still names B-6, which is dropped (settled by L35).
- The lists he did not read (L130): D7 is settled by L35 and L88; D8 and D9 are shown as lines B-10 and B-11; the one function variant of P5 as line B-19; P3 and P4 are layout (L8).

