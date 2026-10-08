# APPLY B -- The cue system (s-rta-1007)
Written 2026-10-07 22:43:11 by the architect seat for topic B. Read-only: nothing built, run, probed or launched; no commit.
Sources read whole: boris-msg-numbered.txt (L0-L135), slice-B.md, facts-app-cue-today.md (CT), area-cue-layers.md (AC), facts-resolume-emulate.md part B and C (RE); on demand: binding-decisions.md 1092-1093 and 1119-1136 (BD), slice-A.md (173, 174, R139, R168, R206), slice-J.md (206), src/ui/FilesBrowser.cpp:101-109, src/MainComponent.cpp:1506-1509.

## SUMMARY
- The preview monitor now has TWO MODES, cue mode (the cued layers, mixed) and preview mode (one clip or file, alone), and one toggle button between them (L35). A name click or a files-window double-click takes the monitor over at once; ONLY the toggle brings the cue mix back. This replaces the page's three ways back (fire, cue button, the name again) under 176 A (L37).
- Files double-clicked in the files window ARE previewed in the first build (L35); the page had left them out (R170). Undocking the monitor is not built yet (L35).
- Beside each layer's cue button sits a transparency slider that acts on the preview only (L34). It replaces the page's "a cued layer shows at full strength wherever its fader is" (R149 a): the cue slider sets the strength, the layer's own blend setting and the layer order are used.
- A new toggle button, "master cue", decides whether the preview shows the global effects and actions (L38). Question 177 is answered by a switch, not by a letter.
- A clip previewed by its name shows WITH all its actions and is triggered on the "1" (L36). This replaces R179 c (shown without actions) and R151's "starts from its beginning" at the click.
- One real clash inside his message: L35 "They will play right away" against L36 "it is triggered on the 1". Asked as B-1.
- His Resolume looks: L16 confirms that a name click plays the clip in the preview and fills the clip tab; L29 (a bypassed layer plays on out of sight) agrees with a cued layer showing its clip whatever the layer's B button says.
- 16 items; 2 assumptions to ask (B-1, B-4), 7 as strike-through lines, 7 internal.

## ITEMS
@@ITEM 176
TITLE: A clip's name clicked while layers are cued
STATUS: ANSWERED a, with the return rule replaced by his R170 words
HIS: L37, L35, L16
RULE: The preview monitor has two modes, cue mode and preview mode, and one toggle button that switches between them (L35: "we will have a toggle between cue mode and preview mode"). A single click on a clip's name in the deck switches the monitor to preview mode at once, with no other press (L35: "The preview will just happen frictionlessly"): the clip takes the whole preview monitor, alone, outside the cued mix (the page's option A; L37: "176 a but read R170 for detail"). The cue buttons keep their state while the monitor is in preview mode, and the cued mix is seen again only when he pushes the toggle button (L35: "to see the cue you need to push the toggle button"). Firing a clip, pressing a cue button or clicking the same name again does not bring the cued mix back (assumption B-6). The name click also selects the clip and fills the Clip tab (L16), never fires it and never changes the output. When the previewed clip starts to move: R179 and assumption B-1.
CHANGED: Option A's last sentence is replaced. The page said "The cued layers come back when you fire any clip, press a cue button, or click that name again"; his words say only the toggle button brings them back (L35). Added: the two modes and the toggle. Option B (the clip shown inside the cued mix) is not built.
TODAY: No preview monitor, no modes: one panel that shows the composition canvas (CT section 1; AC 1a items 1-4). A name click selects and, for a picture or a generated clip, loads it into the main renderer's fallback slot, which reaches the outputs when nothing plays (CT section 2; src/MainComponent.cpp:693-716). All of it is new work.
@@END
@@ITEM 177
TITLE: Global effects in the preview: the master cue button
STATUS: ANSWERED in his own words (a switch between A and B)
HIS: L38
RULE: A toggle button called "master cue" is added to the cue controls (L38: "add a global effects and actions toggle button called master cue"). Master cue OFF: the preview shows the cued layers with their own clip effects and layer effects, mixed, and the global tab's effects are not on it (the page's option A). Master cue ON: the cued mix is also run through the global tab's effects, as those effects stand in the output at that moment, including whatever the running global actions are doing to them, so the preview looks the way the mix would look on the output (the page's option B). Master cue never changes the output and never starts or stops an action (assumption B-4). It acts on the cued mix only; a clip or file shown alone in preview mode does not get the global effects (assumption B-5). The master fader and the composition's position, scale and rotation are never applied to the preview, whatever master cue says (R149 g). Master cue opens off, is not saved with the show, is never recorded and can be put on a key or a pad (assumption B-7). Global effects that remember earlier pictures (trails, echo) may look slightly different in the preview than in the output (D9, assumption B-11). Where the button sits: with the cue buttons under the preview monitor, laid out by Harmony (L8).
CHANGED: Neither letter was taken; both options stay, chosen live by the new button. Added: the button, its name, and the word "actions" (the page spoke of global effects only).
TODAY: The global effects run once per frame on the one canvas, with their own history key (CT section 5, the line on applyGlobalEffects, Renderer.cpp:714). No second picture exists (CT section 5, first line). A second run of the global stack for the preview needs its own history, or its trails are approximated (D9).
@@END
@@ITEM R141
TITLE: The preview monitor, cue buttons, and a transparency slider each
STATUS: CORRECTED one thing added (the slider); the fader sentence changes with it
HIS: L34
RULE: A preview monitor sits just below the output monitor. Under it, one cue button per layer; any number of them can be on. Beside each cue button sits a transparency slider for that layer (L34: "next to each cue button for each layer, there's also a transparency slider so we can see what the transparency would be in the preview monitor"). The layers whose cue button is on show in the preview mixed together, each at the strength of its own cue slider, each with the blend setting of its layer settings, stacked in layer order (L34: "of course using the transparency setting in the layer settings and the layer stacked in their correct order"). The cue slider acts on the preview only: it never moves the layer's real transparency and the layer's real transparency never moves it; it starts at full (assumption B-3). With or without the global effects and actions: the master cue button (177). Nothing done on the cue controls ever changes the output. This multi-layer cue is his own and not Resolume's (one monitor there shows one layer or one group). A click on a clip's name: 176, R150, R151, R179.
CHANGED: Added: the transparency slider beside each cue button (L34). Replaced: "except that each cued layer's own fader is ignored (R149 a)" becomes "each cued layer shows at the strength of its cue slider". Replaced: "(with or without the global effects: question 177)" becomes the master cue button (L38). The rest stands.
TODAY: No cue or preview control exists on the layer strip or the monitor (AC 1c item 16; CT section 4). A per-layer picture taken after the layer's effects and before its transparency and blend already exists for the Layer Router (CT section 5, saveLayerOutput, CompositorEngine.cpp:1150-1154): the cued mix can be blended from those copies without running a layer twice. A bypassed layer is skipped before it is drawn (AC 1d item 17), so it has no such copy yet.
@@END
@@ITEM R149
TITLE: A cued layer and its fader, bypass, solo, blend
STATUS: REPLACED in parts (a), (f) and (g) by L34 and L38; (b) to (e) stand
HIS: L34, L38, L29, L51
RULE: (a) A cued layer shows in the preview at the strength of its cue slider, wherever the layer's real transparency slider is: with the real slider all the way down and the cue slider up, the layer is fully seen in the preview and not at all in the output (L34). (b) The preview shows the layer's clip with the clip's effects and the layer's effects. (c) The layer's B (bypass) and S (solo) buttons do not change what the preview shows; only the cue buttons do. A bypassed layer's clip plays on out of sight, so the preview of a cued bypassed layer moves (his Resolume look L29: "it plays out of sight"; evidence, the rule itself is in topic H). (d) The cued layers are mixed among themselves only, in layer order (L34), the lowest cued one drawn as it is and the others blended onto it with their own blend setting, so one cued layer alone shows its own picture. (e) A cued layer that is empty adds nothing. A cued layer that holds only effects acts on the cued layers below it and does nothing as the lowest cued one (L51, assumption B-13). (f) Cueing ahead of time: real sliders down, fire the clips or the column, switch the cue buttons on, look and try the mix with the cue sliders, then bring the real sliders up. (g) The master fader and the composition's position, scale and rotation are never on the preview. The global effects and actions are on it only while master cue is on (177). A layer's own Feedback is part of the layer and shows.
CHANGED: (a) replaced: the page said "at full strength wherever its fader is"; the cue slider now sets the strength (L34). (f) gains "try the mix with the cue sliders". (g) replaced: "whatever you answer to 177" becomes the master cue button (L38). (e) gains the effects-only layer (L51). (b), (c), (d) stand as shown; (c) and (d) are still Harmony's own, not named by him.
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
STATUS: REPLACED in part by L36 (starts on the 1, with its actions); the rest stands
HIS: L36, L35, L16
RULE: A clip previewed by its name plays in the preview monitor, it is not a still (L16), with its own effects and with all its own actions (L36), without its layer's effects, without the layer's transparency and without the master, at the clip's own Opacity setting. Previewing never fires it and never changes the output. While the beat runs it is triggered on the "1": the preview takes the clip over at the click, the clip stands on its first frame, and on the next "1" it starts from its beginning with its actions, so that they play in time with the music (L36: "it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music"; the clash with L35 is assumption B-1). It loops; a BPM-mode clip follows the tempo there too. A clip that is playing in the output at that moment is not started again: the preview shows it at the same moment of its playback (assumption B-8). While the beat is stopped or paused, a clip previewed by its name still plays in the preview, at the tempo number, starting at the click, and the preview never starts the tempo (assumption B-2). A cued layer (cue mode) shows what the layer itself holds, so a clip that stands still in the output stands still there too.
CHANGED: Replaced: "it starts from its beginning and loops" at the click becomes "starts from its beginning on the 1" (L36). Added: all its own actions play (L36; the page's R179 c had them left out). The sentences on the stopped or paused beat stand as shown (he did not say "R151 still"), now as assumption B-2 because L36 puts every preview start on a "1".
TODAY: A video clip has an open, parked player, but no clip can be drawn outside a layer (CT section 5, the line "Can it draw a clip that is on no layer"; AC T3). One player per clip: a clip seen twice is the same playback (CT section 5, the video line). Clip actions are not built (AC section 2 item 7).
@@END
@@ITEM R152
TITLE: Cue controls belong to the night, not the show
STATUS: STANDS extended by Harmony to the new controls (assumption B-7)
HIS: none
RULE: The cue buttons are not saved with the show and the app opens with none on. The same holds for the cue transparency sliders (they open at full), master cue (opens off) and the cue / preview toggle (opens in cue mode). With no cue button on, cue mode shows black; with nothing previewed yet, preview mode shows black. Every one of these controls can be put on a key, a MIDI pad or a knob in the keyboard and MIDI mapping (question 206 at its default: every button and every slider can be mapped). A recording never keeps a press or a move of them, and an action never presses or moves them.
CHANGED: nothing in the reading. Added by Harmony, not by his words: the three new controls of L34, L35 and L38 follow the same rule (assumption B-7).
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
HIS: L35
RULE: The preview monitor stays in the main window; undocking it or moving it to a second screen is not built yet (L35: "no need to undock and move just yet"). Besides the cued layers, the preview monitor previews (1) a clip whose name is single-clicked in the deck and (2) a file that is double-clicked in the files window (L35: "we also want to preview the clips double clicked from the files window and name clicked from a clip in the deck"). Either click takes the preview over at once, with no other press, and switches the monitor to preview mode (L35: "Double clicking or single clicking the name will take over the preview"). One toggle button switches between cue mode and preview mode; pushing it is the only way back to the cued mix (L35: "to get back to queue you need to push the toggle button"). Pushed again it returns to preview mode and shows the last previewed clip or file (assumption B-14). A file from the files window is not a clip yet: it plays right away from its start (L35: "They will play right away"), loops, alone, without effects or actions, and it never reaches the output (assumption B-9). A double-click previews only; it loads nothing into a deck. When a deck clip previewed by its name starts to move: R151 and assumption B-1. His sentence "We will have various configurations other than live and recording review mode" announces more screen arrangements later; nothing is built from it in this topic (assumption B-17).
CHANGED: Replaced: "it shows cued layers and clips that are in the clip grid, not files in the Files tab" (files are in, L35). Confirmed: the monitor is in the main window only. Added: cue mode, preview mode, the toggle button, and that only the toggle returns to the cue.
TODAY: A double-click on a file in the Files tab loads it as a picture into the main renderer's fallback slot and sets the file label (src/ui/FilesBrowser.cpp:101-109; src/MainComponent.cpp:1506-1509); that slot is drawn on the real canvas when nothing plays (AC 1b item 9). That path has to go and a preview path has to be built.
@@END
@@ITEM R179
TITLE: The edges: waiting clip, actions in the preview, Cuepoints
STATUS: CORRECTED part (c) replaced; (a), (b), (d) stand ("all good")
HIS: L36
RULE: (a) A cued layer shows in the preview what the layer itself holds: while a clip waits for its "1" on that layer, the preview shows what the layer shows in the output and not the waiting clip; to look at the waiting clip he clicks its name, which switches the monitor to preview mode. (b) A layer's actions show in the preview of that cued layer, because it is the same layer seen twice. (c) A clip previewed by its name is shown with all its own actions, and it is triggered on the "1" so that the actions play in time with the music (L36: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1"). "All its actions" are the clip's own actions that are switched on; switching one on or off in the Clip tab shows in the preview and is a real change to the clip (assumption B-18). (d) The clip's eight jump points in the Clip tab keep their name "Cuepoints"; everywhere else "cue" means the cue controls under the preview monitor.
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
HIS: L125
RULE: A cued layer shows in the preview the same moment as the output: it is the same layer seen twice. A layer's Echo, Freeze or Feedback, and the global effects under master cue, may show a slightly different trail in the preview than in the output. One limit of the first build: a generated picture with a running state of its own (a simulation), or a MilkDrop clip, of the same kind as one that is playing shows the playing one's state in the preview; MilkDrop itself is not changed in this build (L125). The build is held to it that a preview never disturbs the output.
CHANGED: nothing in the decision. Added: the global effects under master cue share the trail limit. His L125 keeps MilkDrop as it is for this build, so its limit stays.
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
STATUS: DROPPED the look; what it shows when empty is R152 (black)
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The one variant that differed in what the app does (showing the output, dimmed, when nothing is cued) is not built: with nothing cued, or nothing previewed, the monitor shows black (R152, which stands).
CHANGED: The three drawn variants are not shown to him.
TODAY: not checked (no preview monitor exists).
@@END

## ASSUMPTIONS
@@ASSUME B-1
ABOUT: R151 R179 R170 176
TEXT: I assume a clip you preview by its name takes the preview over at once, stands on its first frame, and starts on the next 1 with its actions. A file from the files window plays at once.
WHY: L35 says previews "will play right away"; L36 says a name-previewed clip "is triggered on the 1". The two lines pull apart.
ALT: b) Every preview plays at once and its actions fall in with the beat. c) Only a clip that has actions or is in BPM mode waits for the 1; any other plays at once.
IF-WRONG: STAGE he sees a previewed clip wait up to a bar, or start at once, against what he expects
ASK: YES his own two lines differ and it decides how the preview clock is built
@@END
@@ASSUME B-2
ABOUT: R151
TEXT: I assume that while the tempo is stopped or paused, a clip you preview by its name still plays in the preview, at the tempo number, starting at your click. Previewing never starts the tempo.
WHY: L36 puts the start on the 1; with the tempo stopped or paused no 1 comes (L12, L13, L31). The page said this and he did not correct it.
ALT: b) It stands on its first frame until the tempo runs and reaches the 1.
IF-WRONG: STAGE before a show starts he could not watch a previewed clip move
ASK: LINE it was on the page with a way to say no, and he let it stand
@@END
@@ASSUME B-3
ABOUT: R141 R149
TEXT: I assume the slider beside a cue button changes the preview only. It starts at full, never moves the layer's real transparency, and the preview mixes each cued layer with the blend set in its layer settings.
WHY: L34 "using the transparency setting in the layer settings" can mean the layer's blend setting (L111) or its transparency value.
ALT: b) The cue slider starts at the layer's real transparency value. c) It is a second handle of the layer's real transparency and moves the output too.
IF-WRONG: SMALL the start value or a link is a small change
ASK: LINE with b) a layer with its real slider down would not show when cued, which defeats cueing ahead
@@END
@@ASSUME B-4
ABOUT: 177
TEXT: I assume master cue on means: the preview also goes through the global effects as they stand in the output at that moment, running global actions included. It never starts or tries out an action in the preview alone.
WHY: L38 says only "a global effects and actions toggle button called master cue"; what "actions" does in a preview is not said.
ALT: b) With master cue on, a global action can be tried out in the preview before it reaches the output. c) Master cue shows the whole output in the preview, as on a DJ mixer. d) It also switches the clip and layer actions off in the preview.
IF-WRONG: REBUILD trying an action out in the preview alone needs a second copy of the show's state
ASK: YES no word of his says what the actions half of the button does, and b) is a far bigger build
@@END
@@ASSUME B-5
ABOUT: 177 R151 R149
TEXT: I assume master cue acts on the cued layers only. A clip shown alone in preview mode has its own effects and actions, never its layer's or the global ones. The master fader is never applied to the preview.
WHY: L38 names no mode; L36 says a previewed clip shows "with all its actions" and names no layer or global part.
ALT: b) Master cue also puts the global effects on a clip shown alone in preview mode. c) A clip shown alone also gets its layer's effects.
IF-WRONG: SMALL one more pass on the preview picture
ASK: LINE a real choice he would most likely wave through
@@END
@@ASSUME B-6
ABOUT: 176 R170
TEXT: I assume that in preview mode, firing a clip or pressing a cue button does not switch the monitor back: only the toggle button shows the cued layers again. A cue button pressed meanwhile still switches its layer.
WHY: L35 says twice that the toggle is needed to get back; the page's option A (L37 "176 a") listed three other ways back.
ALT: b) Pressing a cue button also switches the monitor back to cue mode.
IF-WRONG: SMALL one condition
ASK: NO his words in L35 settle it; L37 itself points to them
@@END
@@ASSUME B-7
ABOUT: R152 R141 177 R170
TEXT: I assume the cue sliders, master cue and the cue / preview toggle belong to the night like the cue buttons: not saved with the show, never recorded, never moved by an action. The app opens in cue mode, master cue off, sliders full.
WHY: L34, L35 and L38 add three controls and say nothing on saving, recording or their state at opening.
ALT: b) They are saved with the show and come back as they were left.
IF-WRONG: SMALL a few saved fields
ASK: LINE it extends a rule he read and let stand to controls he added later
@@END
@@ASSUME B-8
ABOUT: R151
TEXT: I assume a clip that is already playing in the output, when you click its name, shows in the preview exactly as it is playing: it is not started again on the 1.
WHY: L36 says a previewed clip "is triggered on the 1" and does not name a clip that is already playing.
ALT: b) The preview shows a second, separate playback of the clip from its beginning.
IF-WRONG: SMALL seen in the preview only
ASK: NO it was on the page and he let it stand; a restart would disturb the output
@@END
@@ASSUME B-9
ABOUT: R170
TEXT: I assume only the files window previews by a double-click: videos, pictures and picture sequences, played from the start, looping, without effects. Sources, MilkDrop presets and effects are not previewed this way.
WHY: L35 names "the clips double clicked from the files window" only; Resolume also previews sources and effects this way.
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
TEXT: I assume two limits in the first build: trails (Echo, Freeze, Feedback) may look slightly different in the preview, and a simulation or MilkDrop clip shows in the preview the state of the one of its kind that is playing.
WHY: No word of his covers it; L125 keeps MilkDrop as it is for this build; he did not read this point (L130).
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
ABOUT: R149
TEXT: I assume a cued layer that holds only effects acts, in the preview, on the cued layers below it; as the lowest cued layer it does nothing.
WHY: L51 gives this rule for the output; applied to the cued mix by his L9.
ALT: b) In the preview it acts on every layer below it, cued or not.
IF-WRONG: SMALL
ASK: NO his own rule, carried over
@@END
@@ASSUME B-14
ABOUT: R170 176
TEXT: I assume a previewed clip or file stays in preview mode until another one is previewed. It plays on out of sight in cue mode, and the toggle shows it again.
WHY: L35 says how a preview starts and how to get back to the cue, not how long a preview lasts.
ALT: b) Going back to cue mode ends the preview; preview mode is then black until the next click.
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
TEXT: I assume "all its actions" means the clip's own actions that are switched on. Switching one on or off in the Clip tab shows in the preview at once and is a real change to the clip.
WHY: L36 says "with all its actions"; whether actions that are switched off also play is not said.
ALT: b) Every action stored on the clip plays in the preview, switched on or not.
IF-WRONG: SMALL
ASK: NO follows the rule that an action plays only while its button is on
@@END

## QUESTIONS BACK
(none: this task names no question of his for topic B)

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
MEANS: The slider beside a layer's cue button that sets how strongly that layer shows in the preview only.
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

## CONFLICTS
- L35 "They will play right away and we will have a toggle between cue mode and preview mode" against L36 "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". Read as: the preview takes over at once, a deck clip starts on the next 1, a file plays at once. Asked as B-1 (ASK: YES).
- Not a clash, settled by himself: L37 "176 a but read R170 for detail" takes option A, whose text says the cued layers come back on a fire, a cue button or the name again; L35 "to see the cue you need to push the toggle button" replaces that return rule (B-6, ASK: NO).

## NOT DONE / UNSURE
- Whether a second picture can be drawn every frame without disturbing the output is INFERRED from the code only (CT section 5; AC T1-T6). Cheapest way to settle: when the build hold lifts, one measured spike before the cue lane is planned: blend the saved per-layer pictures into a small second target and read frame_time_ms with and without it. Not run here (nothing may be built or run).
- A cued layer that is bypassed has no picture of its own in the present draw order (AC 1d item 17, T6); R149 c needs it. How a bypassed layer keeps playing is topic H (his L29 look); the cue lane depends on that rule.
- A name-previewed clip that starts on the "1" with its actions needs the actions engine, which is not built (topic D). The cue lane can ship the picture first and the actions with topic D; the order is Harmony's to plan.
- Master cue ON needs a second run of the global effects with a history of their own, or approximated trails (B-11). Sized only after the spike above.
- "Various configurations other than live and recording review mode" (L35) is not turned into a rule here (B-17); it belongs with the review screen and the show's layout (L94).
- What Resolume does with a clip preview while its tempo is paused or stopped is not known (RE C3.8); it does not bind (his L36 already departs from Resolume).
- The assumption numbers B-15 and B-16 are not used (merged into B-5 and B-1).
