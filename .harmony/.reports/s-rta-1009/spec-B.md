# SPEC B -- The cue system (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-B.md, ruling rule-B.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 220
TITLE: A clip previewed by its name starts at the click
STATUS: ANSWERED in his own words: he changes the item (neither the text nor its way b)
HIS: BF247 "Previewing a clip should not happen on the beat." and "we are cueing this way, then it should play in time"
RULE: A click on a clip's name in the deck shows that clip in the preview monitor at once and starts it at once, from its beginning, at the click. It never waits for the 1 or for any beat, whether the clip is in BPM mode or in Timeline mode and whether it has actions or not. A click on another name replaces the previewed clip at once, in the same way, as often and as fast as he clicks (BF247: "the user could go through any amount of previews as quick as they want"); no preview ever waits for the one before it to finish or to reach a beat. QUICK IS PART OF THE RULE, not only a start that does not wait for the beat (BF247: "It should just be quick"): the build is held to a start that he sees as immediate; the time from the click to the first picture of a previewed video is measured when the preview is built and he is told the number; a previewed clip is never opened in the output's drawing; and fast clicking through previews is one of the loads that are measured before the cue system is built (item 252). The previewed clip loops. A BPM mode clip plays in the preview at the speed that the tempo's BPM gives it, counted from the click and not lined up with the 1; a Timeline mode clip plays at its own speed. The clip's own actions that are switched on start with the clip, at the click, at the tempo's BPM, and are likewise not lined up with the 1 (assumption B3-3). In a preview the click stands in for the trigger: a play-once action of the clip plays once from the click and again only when the preview is started again (BF251: "that action plays again only when that clip is re-triggered"), a One Shot on one of its sliders runs once from the click, and an envelope that follows the playhead follows the preview's own playhead; what reads the beat clock itself -- a signal that runs on the beat such as an oscillator, an envelope that runs along the beat, an effect that reads the beat by itself -- follows the show's one beat clock in the preview as everywhere, and a signal of the sound follows the sound (assumption B3-6). A clip previewed alone is played by a player and a place in its playback that belong to the preview: previewing never moves the clip's own playhead, its play or pause state or any value that the output reads (item 252). Previewing never triggers the clip, never changes the output, and never starts, resumes or moves the tempo or the 1. Being in time with the beat belongs to triggering only (BF247: "only when they trigger and play should they be on time with the beat"): a clip that is triggered on a layer starts by the trigger rules as they stand in topic A, and a layer whose cue button is on shows in the preview what that layer holds, at the same moment as the layer itself, so a clip that was triggered on a layer and is looked at through the cue button plays in time with the music (BF247: "If the clip is loaded into the layer, and we are cueing this way, then it should play in time"; read as the triggered clip of a cued layer: assumption B3-4). Cueing a clip ahead of time therefore stays what @@ITEM R149 (f) says: the layer's own transparency slider down, trigger the clip, cue button on. A clip that is playing on a layer is never started again by a click on its name, a second click included: the preview shows it as the layer plays it, with @@ITEM R151 as it stands in that part (assumption B-8). A second click on the name of a previewed clip that is not playing on a layer starts the preview again at once (assumption B-14). A file double-clicked in the Files tab plays at once, with @@ITEM R170 as it stands. What a previewed clip is shown with (its own effects, its own Opacity, no layer effects, no master): with @@ITEM R151 as it stands.
CHANGED: The item as he read it ("waits for the next 1 before it starts, in BPM mode or not") and its way b ("Only a BPM-mode clip waits") are both rejected: nothing waits. Against the old blocks: R151's "WHILE THE TEMPO RUNS it is triggered on the 1" and the assumed first-frame wait (B-1), R179 (c) "it is triggered on the 1", and U18's first half are replaced (amendments). His words of 2026-10-07 (BF175) go the other way: see CONFLICTS. INFERRED: "loaded into the layer" and "cueing this way" are read as a clip triggered on a layer and seen through that layer's cue button (B3-4; his words of 2026-10-05, BF142 and BF143, describe cueing so). Not said by him and filled by Harmony: when the previewed clip's actions start (B3-3); what a play-once action, a One Shot, an envelope and the things that read the beat do in a preview (B3-6); what a second click on the same name does (B-14). Added by the ruling: "quick" as a requirement of the build, which is his word; the preview's own player, from the technical papers on item 252; and that a clip playing on a layer is never started again by a click on its name (B-8 before B-14).
TODAY: No preview monitor exists; a name click selects the clip and, for a picture or a generated clip, loads it into the main renderer's fallback slot, which reaches the outputs when nothing plays (old 176 and R150 TODAY lines; src/MainComponent.cpp:693-716, read, not run). No clip can be drawn outside a layer; one player per clip (facts-app-cue-today.md section 5). A video's wish to play and its playhead live in the clip model that the output reads: the render thread pushes clip.playing to the one player of the clip and writes the playhead and the play state back (src/render/ClipTransportSync.h:32-52, read, not run; check-252.md F2), so a clip previewed alone needs a player and a play state of the preview's own. Clip actions are not built. A preview that starts at the click needs a clock of its own for the previewed clip and its actions: not checked. How fast a clicked video shows its first picture: not measured.
@@END

@@ITEM 221
TITLE: A preview by name while the tempo is paused or stopped
STATUS: ACCEPTED as written; tested against BF247, which says the same for the running tempo, and against BF250, BF251 and BF271
HIS: box left empty = accepted as written
RULE: While the tempo is paused or stopped, a clip previewed by its name plays in the preview monitor exactly as @@ITEM 220 says for the running tempo: it starts at the click, from its beginning, loops, with its own actions that are switched on, a BPM mode clip and the actions at the tempo's BPM (the BPM the tempo bar shows). One rule therefore holds inside the preview for every state of the tempo. Previewing never starts the tempo, never resumes it and never sets or moves the 1. A preview that is already playing when the tempo is paused, stopped or started again plays on untouched, and nothing in the preview is triggered again when the tempo runs again (assumption B3-5). The way b of the item (the clip waits on its first frame until the tempo runs and reaches the 1) is not built. THE PREVIEW DIFFERS HERE FROM THE SHOW, ON PURPOSE: in the show an action that is switched on while the beat is stopped waits and moves nothing, the action of a Timeline mode clip starts on the next 1, and a One Shot runs from the next 1 after its clip is triggered (topics D and I rule those; page item 242); in a preview the click stands in for the trigger and the actions move from the click (assumptions B3-3 and B3-6). What stands still in a preview while the beat is paused or stopped is only what reads the beat clock itself: an effect that reads the beat by itself, a signal that runs on the beat and an envelope that runs along the beat (page item 243 as accepted; assumption B3-6). AFTER A TEMPO STOP the preview shows the clip's actions that are switched on at that moment, whatever topic D's rule for the stop leaves switched on (BF250: "tempo stop stops all actions, not just global"): if the stop switches the button of every clip's action off, as topic D reads it and asks him, a preview after a stop shows the clip without moving actions until he switches one on again, and an action he switches on then plays in the preview from the click, although in the show it waits for the beat.
CHANGED: nothing in the item: accepted as written. Against the old blocks: R151's last clause of this part ("when the tempo runs again the previewed clip is triggered again on the next 1") falls, because 220 (BF247) takes every start on the 1 out of the preview. Inside the preview the paused or stopped tempo is no longer an exception; against the show it now is one, on purpose, and the RULE says so. U18's "Open besides" is closed. A pull against his earlier words, named so that it can be said to him once: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." (BF167, 2026-10-07) against the accepted item, in which a BPM mode clip and its actions play in the preview through a pause; the accepted item is the newer and wins, and the pause still holds the clips on the layers. Not his words, filled by Harmony: what a play-once action, a One Shot and the things that read the beat do in a preview (B3-6); what a running preview does when the tempo is paused, stopped or started (B3-5).
TODAY: The app has no stopped or paused state of the beat (facts-app-cue-today.md line 79) and no preview monitor. Old R151 TODAY: a preview that plays while the show's beat stands needs a beat of its own; not checked.
@@END

@@ITEM 222
TITLE: What master cue does and where it is shown
STATUS: OPEN he asks back what and where the master cue is
HIS: BF248 "what and where is the master cue?" and "where would the master cue be displayed?"
RULE: Waits on his answer: the reading put to him again (assumption B3-1) is that master cue is one toggle button that belongs with the layers' cue buttons, the controls under the preview monitor. It has no picture and no monitor of its own: it changes what the preview monitor shows in cue mode. Off: the preview shows the cued layers mixed, each with its clip's effects and its layer's effects, without the global effects. On: the same mix is also run through the global effects as they are on the output, moved by whatever global actions are running, so the preview shows what the mix would look like on the output. It only decides what the preview shows: it never changes the output and never starts, stops or tries out a global effect or a global action. Everything else with @@ITEM 177 as it stands, all of it waiting on the same answer. Fixed by his own words and not waiting: the layers have a cue button each, which shows the layer in the preview monitor (BF248: "The layers have a cue button to display in the preview/cue monitor"), and the cue controls are under the preview monitor (2026-10-05: "we will have the controls under the preview monitor").
CHANGED: Nothing is decided: he chose no way and asks what the thing is. The item's text and its ways b and c stand as the three readings. His "the preview/cue monitor" is his name for the preview monitor (noted under NAMES).
TODAY: No cue button, no master cue and no preview monitor exist (old 177 and R141 TODAY lines; area-cue-layers.md 1c item 16). That is why he cannot find the cue button on screen.
@@END

@@ITEM 251
TITLE: The cue transparency slider is a try-out for the preview
STATUS: ACCEPTED as written; no box of his says otherwise
HIS: box left empty = accepted as written
RULE: The transparency slider beside a layer's cue button sets how strongly that layer shows in the preview monitor and nothing else. It starts at full. It never moves the layer's own transparency slider, and the layer's own slider never moves it; a layer whose own slider is all the way down and whose cue slider is up is fully seen in the preview and not at all in the output. The way b of the item (it starts at the layer's own transparency) is not built. The rest with @@ITEM R141 and @@ITEM R149 (a), (d) and (f) as they stand; that it is not saved with the show, never recorded and can be put on a key, a pad or a knob: with @@ITEM R152 as it stands.
CHANGED: nothing: accepted as written. The old assumption B-3 is now his accepted rule; the old RULE lines of R141 and R149 already say the same and are not amended.
TODAY: No cue control exists on the layer strip or the monitor (old R141 TODAY; area-cue-layers.md 1c item 16).
@@END

@@ITEM 252
TITLE: The preview against the output: frames per second, size, limits
STATUS: OPEN he asks back what "come first" means and whether all run at the same fps; the technical call is his to Harmony and is ruled here, then put to him again
HIS: BF267 "What do you mean by come first?" and "should all run at the same fps, no?"
RULE: Waits on his answer: the reading put to him again (assumption B3-2), which is also Harmony's ruling on the technical call he handed over (BF267: "You understand how the system needs to be built better than me."), is this. (1) ONE RATE. The picture for the output and the picture for the preview monitor are made together, once for every frame, and the output monitor and the preview monitor are shown from the same frame: the two always run at the same frames per second (BF267: "should all run at the same fps, no?": yes), and the preview is never drawn at a lower rate on purpose. Every output screen shows those same pictures of the output. (2) THE OUTPUT IS THE MORE IMPORTANT PICTURE (BF267: "Of course the output is more important"); "comes first" means exactly three things. First: nothing that is done for the preview ever changes what the output shows -- no clip's place in its playback, no fade, no simulation and no trail of the output is moved, stepped or overwritten for a preview; the first test of the build is that the output with the preview busy equals, picture for picture, the output with the preview off. Second: inside a frame the output's work, and its share of the video work, is done before the preview's. Third: the preview gives way -- when frames run late for a sustained time while the preview is doing work of its own (a clip or a file previewed alone, the global effects under master cue, a layer that is drawn for the cue only), that work stops, the preview monitor holds its last picture under a plain mark that says it is paused, and the output goes on; the preview comes back when the load has stayed low or when he previews something else, and it must not flicker on and off. The plain cue of layers that are playing, shown from the output's own pictures of them, adds next to no work (an estimate, not measured) and is not stopped. No promise is made that the output never loses a frame before the preview has given way: a short hitch can come first. Nothing that the preview costs may ever switch off or change anything on the output. (3) WHAT THE PREVIEW SHOWS IS TRUE. A cued layer is shown from the one picture that is made of that layer in the frame, so its picture in the preview is exactly that layer's picture in the output, its Echo, Freeze or Feedback trail included; how strongly it shows in the cue is set by its cue transparency slider alone (item 251). A cued layer that the output is not drawing (bypassed, or silenced by another layer's solo: @@ITEM R149 c) is drawn once in the frame, for the cue alone, from the layer's own playback and with the layer's own trail memory: it is the same layer, not a copy. What only the preview needs beyond that -- a clip or a file previewed alone, the global effects under master cue, the effects of a cued layer that holds only effects -- is drawn for the preview alone and keeps a trail memory of the preview's own, never the output's; a clip or a file previewed alone also has a player and a place in its playback of the preview's own. All of it is drawn at the full size of the picture, so that every effect looks as it will on the output; a smaller size is chosen only if the measurement shows that the full size does not fit, it is then one fixed size, and he is told first. Nothing is drawn for the preview that the preview monitor is not showing at that moment. (4) TWO LIMITS OF THE FIRST BUILD (assumption B-11): a MilkDrop clip, or a generated picture that keeps a running state of its own (a simulation), that is previewed by its name while another of its kind is playing shows the playing one's picture under a plain mark that says it is not a run of its own; the one that is playing is never stepped, loaded or changed for the preview; MilkDrop itself stays as it is (BF265: "Keep Milk drop as it is"). (5) MEASURED FIRST. Nothing of this is measured. Before the cue system is built its cost is measured on a heavy show of his, with @@ITEM D8 as amended: if the preview would cost the output anything he is told the number and asked before anything is built. How the two marks look is laid out by Harmony.
CHANGED: The item as he read it is withdrawn in all its parts: "the output always comes first", "smaller", "less smooth" and its line on trails and MilkDrop shown "a little differently" are replaced by the reading above. His own half-sentence "Of course the output is more important" is kept as the order of importance, and his leaning ("should all run at the same fps, no?") is the default. INFERRED: "the preview-cure screen" is read as the preview-cue screen, his name for the preview monitor; "the output preview screen which we have" is read as the output monitor. Harmony's ruling and not his words (he handed the technical call over): what "comes first" means, the preview that gives way and its mark, the full size, the preview's own player and trail memory. The design is ruled from answer-252.md with four corrections from check-252.md: no guarantee is claimed; a clip previewed alone never uses the clip's own play state; the same-rate claim is made for the two monitors only; the size is ruled after the measurement. The MilkDrop limit was in the item and the simulation limit in old D9; his box names neither: they stay assumed, as a line of their own (B-11); the trail limit is gone for cued layers.
TODAY: One picture is drawn per frame, offscreen, and shown in the monitor panel and on the outputs (facts-app-cue-today.md line 17; Pitfall 37). No second picture exists; the compositor cannot draw a subset of layers or a clip that is on no layer, and it cannot be run twice in a frame without advancing fades, video and simulations twice (facts-app-cue-today.md section 5). A per-layer picture is kept after the layer's effects and before keying, transparency and blend, for the Layer Router (src/render/CompositorEngine.cpp:1150-1154). All GL drawing runs on one shared thread, so a late frame delays the output windows too; an output window shows the newest finished picture once per refresh of its own display, swap interval 0 (src/ui/OutputWindow.cpp:12-15). After 30 frames in a row over 12 ms the app switches off the last enabled effect of the legacy effect chain (src/render/Renderer.cpp:839-858; Renderer.h:555-556): the preview's time must be kept out of that. That timer is CPU submit time only (Renderer.cpp:826-830), so the give-way needs a trigger of its own, on GPU time or on a missed refresh: effort not known. A video's wish to play and its playhead live in the clip model, one player per clip (src/render/ClipTransportSync.h:32-52). One MilkDrop engine and one instance per kind of simulation (as answer-252.md cites and check-252.md confirms; not re-read). Which screen paces new pictures (the one the app window is on) is INFERRED, not run. The output screens are fed by the monitor panel's drawing: with that panel hidden the outputs freeze (check-252.md F5; not re-read); old R209 forbids that, and the preview monitor must not be given the same fate. The stated budget is 16.67 ms per frame at 60 fps; the frame cost on his machine is not measured. All read, not run: the source lines named here were re-read for this ruling, the rest stands on the two technical papers.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 176
TITLE: A clip's name clicked while layers are cued
STATUS: ANSWERED a, with the return rule replaced by his R170 words -- AMENDED after page 2 (176 1 by B)
HIS: L37, L35, L16
RULE: The preview monitor has two modes, cue mode and preview mode, and one toggle button that switches between them (L35: "we will have a toggle between cue mode and preview mode"). A single click on a clip's name in the deck switches the monitor to preview mode at once, with no other press (L35: "The preview will just happen frictionlessly"): the clip takes the whole preview monitor, alone, outside the cued mix (the page's option A; L37: "176 a but read R170 for detail"). The cue buttons keep their state while the monitor is in preview mode, and a cue button pressed meanwhile still switches its layer in the cued mix (INFERRED). The cued mix is seen again only when he pushes the toggle button (L35: "to see the cue you need to push the toggle button", and again "to get back to queue you need to push the toggle button"). Firing a clip, pressing a cue button or clicking the same name again does not bring the cued mix back (INFERRED from his "you need to", said twice: it replaces the three ways back of option A's text). The name click also selects the clip and fills the Clip tab (L16), never fires it and never changes the output. The previewed clip starts to move at the click, whatever the tempo does (BF247; item 221): R151. [page 2, B: BF247 "Previewing a clip should not happen on the beat."]
CHANGED: Option A's last sentence is replaced. The page said "The cued layers come back when you fire any clip, press a cue button, or click that name again"; his words say only the toggle button brings them back (L35). Added: the two modes and the toggle. Option B (the clip shown inside the cued mix) is not built. Assumption B-6 is dropped: his words settle it.
TODAY: No preview monitor, no modes: one panel that shows the composition canvas (CT section 1; AC 1a items 1-4). A name click selects and, for a picture or a generated clip, loads it into the main renderer's fallback slot, which reaches the outputs when nothing plays (CT section 2; src/MainComponent.cpp:693-716). All of it is new work.
@@END

@@ITEM 177
TITLE: Global effects in the preview: the master cue button
STATUS: ANSWERED in his own words (a switch between A and B) -- AMENDED after page 2 (177 1 by B, 177 2 by B)
HIS: L38, L36
RULE: A toggle button called "master cue" is added to the cue controls (L38: "add a global effects and actions toggle button called master cue"). Master cue OFF: the preview shows the cued layers with their own clip effects and layer effects, mixed, and the global effects are not on it (the page's option A). Master cue ON: the cued mix is also run through the global effects exactly as they are on the output at that moment, moved by whatever global actions are running, so the preview looks the way the mix would look on the output (the page's option B). His word "actions" is read as the global actions (INFERRED): a clip's own actions and a layer's actions show in the preview whatever master cue says (L36: "a previewed clip (clicking its name) shows the clip with all its actions"; R179 b, which he passed with "all good"). Master cue only decides what the preview shows: it never changes the output and never starts, stops or tries out an action (assumption B-4). It works in cue mode only; a clip or file shown alone in preview mode does not get the global effects (assumption B-5). The master fader and the global Transform (position, scale, rotation) are never applied to the preview, whatever master cue says (R149 g). Master cue opens off, is not saved with the show and is never recorded (assumption B-7); it can be put on a key or a pad (question 206 at its default). Under master cue the global effects run a second time, on the cued mix, with a trail memory of the preview's own: a global effect that remembers earlier pictures (a trail, an echo) shows in the preview the trail of the cued mix, never the output's, and that trail starts empty when master cue is switched on (item 252 and D9 as amended; this waits on his answers to items 222 and 252). [page 2, B: BF267 "You understand how the system needs to be built better than me."] Where the button sits: with the cue buttons under the preview monitor, laid out by Harmony (L8). The whole of this rule waits on his answer to item 222 (BF248: "what and where is the master cue?"): his are the name and the button (L38); what the button does when on and off is Harmony's reading, put to him again as assumption B3-1. Nothing of master cue is built before he answers. [page 2, B: BF248 "what and where is the master cue?"]
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
STATUS: REPLACED in part (a) by L34 (how the new slider relates to the layer's own: assumption B-3) and in (g) by L38; (b) to (f) stand -- AMENDED after page 2 (R149 1 by B, R149 2 by B)
HIS: L34, L38, L29, L51, L55
RULE: (a) A cued layer shows in the preview at the strength that the slider beside its cue button sets (L34). As assumed (B-3) that is so wherever the layer's own transparency slider is: with the layer's own slider all the way down and its cue slider up, the layer is fully seen in the preview and not at all in the output. (b) The preview shows the layer's clip with the clip's effects and the layer's effects. (c) The layer's B (bypass) and S (solo) buttons do not change what the preview shows; only the cue buttons do. A bypassed layer's clip plays on out of sight, so the preview of a cued bypassed layer moves (his Resolume look L29: "it plays out of sight"; evidence, the rule itself is in topic H). (d) The cued layers are mixed among themselves only, in layer order (L34): the lowest cued one is drawn as it is, over black, and the others are blended onto it with their own blend mode, so one cued layer alone shows its own picture whatever its blend mode. The cue slider of the lowest cued layer fades that layer against the black (assumption B-20). (e) A cued layer that is empty adds nothing. A cued layer that holds only effects acts on the cued layers below it and does nothing as the lowest cued one (L51, assumption B-13). (f) Cueing ahead of time: the layers' own sliders down, fire the clips or the column, switch the cue buttons on, look and try the mix with the cue sliders, then bring the layers' own sliders up. (g) The master fader and the global Transform (position, scale, rotation) are never on the preview. The global effects, with what the global actions do to them, are on it only while master cue is on (177). A layer's own Feedback is part of the layer and shows. A clip triggered on a layer in the way of (f) starts by the trigger rules as they stand in topic A, so through the cue button it is seen in time with the music (BF247: "If the clip is loaded into the layer, and we are cueing this way, then it should play in time"; read as the triggered clip of a cued layer: assumption B3-4). Nothing in cue mode has a start of its own. [page 2, B: BF247 "we are cueing this way, then it should play in time"] Part (d) holds for everything by which a layer is laid over the layers below it: his newest words keep keying beside the blend modes (BF260: "All the blend modes and keying stay"), so a cued layer that lies over other cued layers is keyed and blended onto them exactly as the output keys and blends it, with whatever of the blend and keying entries is built by then (topic I, page item 240); the lowest cued layer stays as part (d) says. What a mask does in the cue (BF260: "We will also add masks and moving masks") waits on topic I's rule for masks. [page 2, B: BF260 "All the blend modes and keying stay"]
CHANGED: (a) replaced: the page said "at full strength wherever its fader is"; a slider beside the cue button now sets the strength (L34); that it starts at full and is independent of the layer's own slider is assumption B-3, not his word. (d) gains the cue slider of the lowest cued layer (B-20). (e) gains the effects-only layer (L51). (f) gains "try the mix with the cue sliders" and holds as written only under B-3. (g): "whatever you answer to 177" becomes the master cue button (L38); "the composition's" becomes "global" (L55). (b), (c) stand as shown; (c) and (d) are still Harmony's own, not named by him.
TODAY: A layer at transparency 0 is still decoded and given its effects; one of the Opaque kind draws black over everything below (AC 1d item 18; CT section 4). Bypass and solo skip the layer in the one real picture (AC 1d item 17). What a layer's type and blend menu become: topic I (his L111).
@@END

@@ITEM R150
TITLE: Name click selects and previews; thumbnail click fires
STATUS: STANDS confirmed by his Resolume look and extended by L35 -- AMENDED after page 2 (R150 1 by B)
HIS: L16, L35
RULE: A click on a clip's thumbnail fires it. A click on a clip's name selects the clip without firing it: the Clip tab shows its settings, so they can be changed before it is live, and the clip is shown in the preview monitor, which switches to preview mode (L16: "when you click Lipp's name, it plays in the preview and populates the clip tab"; L35). A name click only selects and previews: it never puts anything on the main picture or on an output. In the deck only a click on a clip's name feeds the preview monitor. A click anywhere else on a cell, which triggers the cell and by his newest words also selects it (BF255: "you can select the cell which triggers it and selects it"; topic F rules it under page item 233), does not change the preview monitor, and neither does a trigger by a key, a pad, a column trigger or an action. A click on the name place of an empty cell selects the cell and leaves the preview monitor as it is, its mode and what it shows (assumption B3-7). [page 2, B: BF255 "you can select the cell which triggers it and selects it"]
CHANGED: nothing in the reading. Added from L35: the name click also switches the monitor to preview mode. "Lipp's" is read as "clip's" (INFERRED, a typing slip).
TODAY: Thumbnail = fire and name bar = select are built (AC 1b item 7; ClipCell.cpp:231-239). The name click's load into the fallback slot, which can reach the outputs when nothing plays, has to go (AC 1b items 8-9; AC section 3 O3; src/MainComponent.cpp:693-716).
@@END

@@ITEM R151
TITLE: How a clip previewed by its name plays
STATUS: REPLACED in part by L36 (starts on the 1, with its actions); the rest stands, the stopped or paused tempo as assumption B-2 -- AMENDED after page 2 (R151 1 by B, R151 2 by B, R151 3 by B)
HIS: L36, L35, L16, L12, L13
RULE: A clip previewed by its name plays in the preview monitor, it is not a still (L16), with its own effects and with all its own actions (L36), without its layer's effects (assumption B-5), without the layer's transparency and without the master, at the clip's own Opacity setting. Previewing never fires it and never changes the output. IT STARTS AT THE CLICK, never on the "1" and never on any beat (BF247: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat"): the preview takes the clip over at the click and the clip starts from its beginning at once, in BPM mode or in Timeline mode, with actions or without. A click on another name replaces it at once, as often and as fast as he clicks. It loops. A BPM mode clip plays at the speed the tempo's BPM gives it, counted from the click and not lined up with the "1". Its own actions that are switched on start with it at the click, at the tempo's BPM, not lined up with the "1" (assumption B3-3). The click stands in for the trigger: a play-once action plays once from the click and a One Shot runs once from the click; a signal that runs on the beat, an envelope that runs along the beat and an effect that reads the beat by itself follow the show's one beat clock in the preview as everywhere (assumption B3-6). A Resync or a new "1" does nothing to a preview; a change of the tempo's BPM changes the speed of a BPM mode preview and of its actions. A clip previewed alone is played by a player and a place in its playback that belong to the preview, never the clip's own, which the output reads (item 252). [page 2, B: BF247 "Previewing a clip should not happen on the beat."] WHILE THE TEMPO IS PAUSED OR STOPPED the same rule holds (item 221, accepted as written): the clip plays in the preview, with its actions, at the tempo's BPM, starting at the click. Previewing never starts or resumes the tempo. A preview that is already playing when the tempo is paused, stopped or started again plays on untouched, and nothing in the preview is triggered again when the tempo runs again (assumption B3-5). [page 2, B: item 221 accepted as written; BF247 "only when they trigger and play should they be on time"] A CLIP THAT IS PLAYING IN THE OUTPUT is not started again by a click on its name: the preview shows it at the same moment of its playback, with the same action values, and a previewed clip that he then fires is shown from then on as it plays in the output (assumption B-8). A second click on the name of the clip that is already previewed starts it again at once (assumption B-14); if that clip is playing on a layer, no click on its name starts anything again, the second one included, and the part above on a clip that is playing in the output holds (assumption B-8). [page 2, B: BF247 "It should just be quick"] A cued layer (cue mode) shows what the layer itself holds, so a clip that stands still in the output stands still there too.
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
STATUS: CORRECTED files are previewed in the first build; the toggle is added -- AMENDED after page 2 (R170 1 by B)
HIS: L35, L11, L13
RULE: The preview monitor stays in the main window; undocking it or moving it to a second screen is not built yet (L35: "no need to undock and move just yet"). Besides the cued layers, the preview monitor previews (1) a clip whose name is single-clicked in the deck and (2) a file that is double-clicked in the files window (L35: "we also want to preview the clips double clicked from the files window and name clicked from a clip in the deck"). Either click takes the preview over at once, with no other press, and switches the monitor to preview mode (L35: "Double clicking or single clicking the name will take over the preview"). One toggle button switches between cue mode and preview mode; pushing it is the only way back to the cued mix (L35: "to get back to queue you need to push the toggle button"). Pushed again it returns to preview mode and shows the last previewed clip or file (assumption B-14). A file from the files window is not a clip yet: it plays right away from its start (L35: "They will play right away"), also while the tempo is paused or stopped (it is not in BPM mode and has no actions, so nothing of it waits for the tempo: INFERRED from L11 and L13), loops, alone, without effects or actions, shown whole and never stretched (assumption B-21). A double-click only previews: nothing goes into a deck and nothing reaches the output; which windows preview by a double-click is assumption B-9. A deck clip previewed by its name starts at the click too, whatever the tempo does (BF247; item 221): R151. A file and a deck clip therefore start alike; they differ only in that the deck clip has its effects and its actions. [page 2, B: BF247 "Previewing a clip should not happen on the beat."] His sentence "We will have various configurations other than live and recording review mode" announces more screen arrangements later; nothing is built from it in this topic (assumption B-17).
CHANGED: Replaced: "it shows cued layers and clips that are in the clip grid, not files in the Files tab" (files are in, L35). Confirmed: the monitor is in the main window only. Added: cue mode, preview mode, the toggle button, and that only the toggle returns to the cue.
TODAY: A double-click on a file in the Files tab loads it as a picture into the main renderer's fallback slot and sets the file label (src/ui/FilesBrowser.cpp:101-109; src/MainComponent.cpp:1506-1509); that slot is drawn on the real canvas when nothing plays (AC 1b item 9). That path has to go and a preview path has to be built.
@@END

@@ITEM R179
TITLE: The edges: waiting clip, actions in the preview, Cuepoints
STATUS: CORRECTED part (c) replaced; (a), (b), (d) stand ("all good") -- AMENDED after page 2 (R179 1 by B, R179 2 by B)
HIS: L36, L56
RULE: (a) A cued layer shows in the preview what the layer itself holds: while a clip waits for its "1" on that layer, the preview shows what the layer shows in the output and not the waiting clip; to look at the waiting clip he clicks its name, which switches the monitor to preview mode. (b) A layer's actions show in the preview of that cued layer, because it is the same layer seen twice. (c) A clip previewed by its name is shown with all its own actions, and it starts at the click, never on the "1" (BF247: "Previewing a clip should not happen on the beat"); the first half of his L36 stands ("a previewed clip (clicking its name) shows the clip with all its actions"), and the actions start with the clip at the click, at the tempo's BPM, not lined up with the "1" (assumption B3-3). [page 2, B: BF247 "Previewing a clip should not happen on the beat."] His "all its actions" is read as the clip's own actions that are switched on (assumption B-18): switching one of them on or off, in its row below the clip (L56), shows in the preview at once and is a real change to the clip (assumption B-18: "at once" is not his word; it follows from the start at the click, assumption B3-3). The state of the tempo changes nothing in a preview by name: R151. [page 2, B: BF247 "Previewing a clip should not happen on the beat."; item 221 accepted as written] (d) The clip's eight jump points in the Clip tab keep their name "Cuepoints"; everywhere else "cue" means the cue controls under the preview monitor.
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
STATUS: OPEN still Harmony's own (assumption B-10) -- AMENDED after page 2 (D8 1 by B)
HIS: none
RULE: Waits on his answer to item 252 (BF267). The reading put to him (assumption B3-2): the picture for the output and the picture for the preview monitor are made together, once for every frame, so the output monitor and the preview monitor always run at the same frames per second, and the preview is never drawn at a lower rate on purpose; every output screen shows the same pictures of the output. What only the preview needs is drawn at the full size of the picture; a smaller size is chosen only if the measurement shows that the full size does not fit, and he is told first. The output is the more important picture (BF267: "Of course the output is more important"): when the computer cannot keep up, the preview gives way and holds a marked still, as item 252 says, and no promise is made that the output never loses a frame before that. [page 2, B: BF267 "should all run at the same fps, no?"] Its cost is measured before it is built, and if it would cost the output anything he is told first how many frames per second the output would lose.
CHANGED: nothing; no word of his touches it. The new parts (a slider per cued layer, master cue, files previewed) add to what has to be measured.
TODAY: One picture per frame is drawn and nothing else; no measurement of the frame cost on his machine exists (CT section 5, the frame-budget line; CT "what I could not establish").
@@END

@@ITEM D9
TITLE: Same moment as the output; limits of the first build
STATUS: OPEN still Harmony's own (assumption B-11) -- AMENDED after page 2 (D9 1 by B, D9 2 by B)
HIS: L126
RULE: A cued layer shows in the preview the same moment as the output: it is the same layer seen twice. Waits on his answer to item 252 (BF267); the reading put to him (assumption B3-2): a cued layer is shown from the one picture that is made of that layer in the frame, so its Echo, Freeze or Feedback trail in the preview is the output's own trail. A cued layer that the output is not drawing (R149 c) is drawn once, for the cue alone, with the layer's own playback and the layer's own trail memory: it is the same layer, not a copy. What the preview has to draw for itself beyond that (the global effects under master cue, the effects of a cued layer that holds only effects, a clip or a file previewed alone) keeps a trail memory of the preview's own and never touches the output's: its trail is the true trail of the picture that the preview shows, and it starts empty when that preview starts. [page 2, B: BF267 "Playing something smaller or less smooth may create problems. What do you think?"] One limit of the first build: a generated picture with a running state of its own (a simulation), or a MilkDrop clip, of the same kind as one that is playing shows the playing one's picture in the preview, under a plain mark that says it is not a run of its own (assumption B-11); the one that is playing is never stepped, loaded or changed for the preview; [page 2, B: BF265 "Keep Milk drop as it is"] MilkDrop itself is not changed in this build (L126: "that's it for this upcoming build"). The build is held to it that a preview never disturbs the output.
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

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME B3-1
ABOUT: 222 177
TEXT: I assume master cue is one more cue button, for global. On: the preview monitor shows the cued layers through the global effects, moved by the global actions, as the output would. Off: without them. It never changes the output.
WHY: BF248 asks "what and where is the master cue?"; his own line of 2026-10-07 names the button and not what it does.
ALT: b) On, it also lets you switch a global effect or action on for the preview monitor only, to try it first. c) On, the preview monitor shows the whole output, as on a DJ mixer.
IF-WRONG: REBUILD way b needs a second, preview-only copy of the global effects and their actions; and he would press it expecting something else
ASK: YES he asked back; no word of his says what the button does; way b is a far bigger build
@@END

@@ASSUME B3-2
ABOUT: 252 D8 D9 177
TEXT: I assume the output monitor and the preview monitor always run at the same fps, and a cued layer looks exactly as on the output. If the computer cannot keep up, the preview pauses, marked, and the output keeps going.
WHY: BF267 asks "should all run at the same fps, no?" and "What do you think?" and hands the build to Harmony; the design is ruled from answer-252.md with the corrections of check-252.md; nothing is measured.
ALT: b) The preview is always drawn smaller, so it rarely has to pause; price: a few effects look slightly different in it than on the output.
IF-WRONG: STAGE a preview that costs the output frames is seen by the audience; a preview that looks different from the output misleads him
ASK: YES he asked back (BF267); this is the technical call he handed over, ruled and put to him again
@@END

@@ASSUME B3-3
ABOUT: 220 221 R151 R179
TEXT: I assume a clip you preview by its name plays with its actions from the moment you click: they run at the tempo's BPM but are not lined up with the 1. Only a triggered clip is on the beat.
WHY: BF247 takes the wait for the beat out of previewing and does not name the actions; on 2026-10-07 he wanted them "playing in time with the music".
ALT: b) The clip's picture starts at once, and its actions join on the next 1. c) A clip previewed by its name is shown without its actions.
IF-WRONG: SMALL seen in the preview only; it changes when the preview's actions start, not how they are built
ASK: LINE his "only when they trigger and play" all but says it, and he accepted the same for the paused tempo (221)
@@END

@@ASSUME B3-4
ABOUT: 220 R149 R179
TEXT: I assume "loaded into the layer" and "cueing" mean a clip you triggered on a layer, seen through that layer's cue button: it is in time because the trigger put it in time. Nothing new is built for it.
WHY: BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" names no click or button. His own words of 2026-10-05 describe cueing in this way: BF142 "click on a column to trigger all of them at the same time and cue them ahead of time via the cue system"; BF143 "exactly how a DJ mixer allows the user to cue and listen to how the tracks would be mixed together before they mix them".
ALT: b) With a layer's cue button on, a click on a clip's name in that layer puts the clip into the cue on the next 1, without it reaching the output.
IF-WRONG: REBUILD way b is a layer that plays one clip for the output and another for the cue: a large addition, though nothing built by this reading would have to be undone
ASK: LINE his words of 2026-10-05 (BF142, BF143) and BF248 "The layers have a cue button to display in the preview/cue monitor" describe cueing as triggered clips seen through the layers' cue buttons, and way b is a function he has never described; it is shown to him as a line because it reads his newest sentence
@@END

@@ASSUME B3-5
ABOUT: 220 221 R151 R152
TEXT: I assume the tempo bar's play, pause and stop never touch a clip you preview by its name: it plays on. The cue shows the layers as they are: standing still in a pause, empty after a stop.
WHY: Item 221 covers a preview started in a pause or a stop, not one already running; BF271 "Stop removes all clips from all layers" names layers only.
ALT: b) Tempo stop also empties the preview monitor.
IF-WRONG: SMALL seen in the preview only
ASK: NO follows from item 221 as accepted and from a cued layer being the same layer seen twice
@@END

@@ASSUME B3-6
ABOUT: 220 221 R151 R179
TEXT: I assume that in a preview your click stands in for the trigger: a play-once action and a One Shot run once from the click. What reads the beat itself (an oscillator, an envelope along the beat, a strobe) follows the show's beat there too.
WHY: BF251 "that action plays again only when that clip is re-triggered", page item 242 (a One Shot runs from the next 1 after its clip is triggered) and page item 243 are written for the show; none of them names a preview, and the papers of topics D and I leave the preview to this topic. BF247 takes every wait for the beat out of a preview.
ALT: b) A play-once action and a One Shot do not play in a preview at all. c) Everything in the preview counts from the click, the effects that read the beat too.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal: it carries the show's rules into the preview by BF247 and by the one beat clock of the show
@@END

@@ASSUME B3-7
ABOUT: R150 176
TEXT: I assume in the deck only a click on a clip's name feeds the preview. A click that triggers, a key, a pad, a column trigger or an action leaves it as it was, and so does a click on an empty cell's name place.
WHY: BF255 "you can select the cell which triggers it and selects it" makes the triggering click select too; his words tie the preview to the name click only (BF174 "name clicked from a clip in the deck"); an empty cell has nothing to preview.
ALT: b) A click on the name place of an empty cell turns preview mode black.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal; the half on the triggering click is put to him by the show-file topic as a line of its own
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

@@ASSUME B-11
ABOUT: 252 D9
TEXT: I assume two limits in the first build: a MilkDrop clip, or a generated picture that runs by itself, previewed while another of its kind plays, shows the playing one's picture, marked, not a run of its own.
WHY: BF267 asks about "come first", size and fps and does not name MilkDrop; BF265 "Keep Milk drop as it is" keeps the one MilkDrop; the trail limit of the old text is gone, because a cued layer is shown from the output's own picture of it.
ALT: b) It shows no picture, only a note that it cannot be previewed while the other plays. c) Every previewed picture is fully its own, which needs a second MilkDrop and a second copy of each such generated picture.
IF-WRONG: REBUILD way c is a second MilkDrop and second copies of every picture that keeps a state of its own; the difference between the text and way b is small
ASK: LINE a narrow limit he would most likely wave through; the MilkDrop half follows his own words (BF265)
@@END

@@ASSUME B-14
ABOUT: R170 176 R151 220 252
TEXT: I assume a preview lasts until another clip or file is previewed. While the preview monitor is in cue mode it is not drawn; the toggle shows it again, from its beginning. A second click on the same name restarts it at once.
WHY: BF247 says a preview is quick and never on the beat; how long a preview lasts and what a second click does are still not said. Changed by the ruling: a preview that nobody sees is not drawn, because its work would be taken from the output (BF267 "Of course the output is more important"; item 252).
ALT: b) In cue mode it plays on out of sight, and the toggle shows it where it has got to. c) A second click on the same name changes nothing.
IF-WRONG: SMALL seen in the preview only
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
ABOUT: R179 220
TEXT: I assume "all its actions" means the clip's own actions that are switched on. Switching one on or off while the clip is previewed shows in the preview at once and is a real change to the clip.
WHY: His words say "shows the clip with all its actions" (BF175) and not whether actions that are switched off play too. When a switch made during a preview shows is not said by him anywhere: "at once" follows the start at the click (B3-3); BF247 speaks only of when a preview starts.
ALT: b) Every action stored on the clip plays in the preview, switched on or not. c) A switch shows in the preview only from the next 1, as in the show.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal: it follows the rule that an action plays only while its button is on, and the timing follows the line on when a preview's actions start
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

## CLOSED ASSUMPTIONS (one line each)
- B-1 -> page 2, item 220
- B-2 -> page 2, item 221
- B-3 -> page 2, item 251
- B-4 -> page 2, item 222
- B-9 -> MERGED 253
- B-10 -> page 2, item 252
- B-12 -> SETTLED
- B-13 -> SETTLED

## QUESTIONS BACK
@@ANSWER 222
HIS: BF248 "what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?"
ANSWER: Master cue is one on/off button that sits with the layers' cue buttons under the preview monitor. It has no picture of its own: it changes what the preview monitor shows. The name and the button are yours, from 7 October: "177 add a global effects and actions toggle button called master cue". What it does is my guess: off, the preview monitor shows the cued layers mixed, without the global effects; on, the same mix goes through the global effects, as the global actions move them, as it would look on the output. No cue button is built yet, which is why you cannot find one in the app.
@@END

@@ANSWER 252
HIS: BF267 "What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?"
ANSWER: Yes: they should all run at the same fps, and I will build it so. The output and the preview are drawn together, frame by frame, so the two monitors cannot run apart; the output screens show the same pictures. You are right about smaller and less smooth: I take both back. A layer you cue should look exactly as on the output, trails included. By "come first" I mean two things. A preview never changes what the audience sees. And if the computer cannot keep up, the preview stops on a still marked "paused" while the output carries on; a short hitch may come before that. Nothing is measured yet; I test it on a heavy show before building.
@@END

## NAMES
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

@@NAME cue / preview toggle
MEANS: The one button that switches the preview monitor between cue mode and preview mode.
SOURCE: Harmony's pick, from his words L35 ("a toggle between cue mode and preview mode", "the toggle button")
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

@@NAME preview mode
MEANS: The state of the preview monitor in which it shows one clip or one file, alone, playing from the moment it was clicked and never waiting for the beat.
SOURCE: his words BF247 ("Previewing a clip should not happen on the beat"); replaces the meaning "alone, playing in time with the music" of the row preview mode in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

@@NAME preview monitor
MEANS: The monitor below the output monitor that shows the cue or one previewed clip or file; he also calls it "the preview/cue monitor" and "the preview-cue screen", which are the same thing.
SOURCE: his words BF248 ("the preview/cue monitor") and BF267 ("the preview-cure screen", read as preview-cue: INFERRED); the name preview monitor in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md stays, his two variants are added to its row
@@END

@@NAME cueing
MEANS: Looking, in the preview monitor, at a layer whose cue button is on: the clip triggered on that layer is seen as the layer plays it, in time with the music.
SOURCE: his words BF247 ("we are cueing this way"); the meaning is Harmony's reading (assumption B3-4)
@@END

@@NAME master cue
MEANS: The toggle button, with the layers' cue buttons, that puts the global effects and what the global actions do to them on the preview monitor, or leaves them off; the meaning waits on his answer to item 222.
SOURCE: his words of 2026-10-07 ("called master cue") for the name; the meaning is Harmony's reading, asked again after BF248
@@END

@@NAME output monitor
MEANS: The monitor that shows the picture as the outputs get it; he also calls it "the output preview screen", which is the same thing.
SOURCE: his words of 2026-10-05, BF143 ("resolume has a output monitor"), and BF267 ("the output preview screen which we have", read as the output monitor: INFERRED); the name output monitor in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md stays, his variant is added to its row
@@END

## REACHES OTHER TOPICS (from the paper)
- X (U18, X-24, X-29): U18 is amended here because item 221 is made from it. The old assumptions X-24 and X-29 are settled by BF247 and by item 221 as accepted: the paper of topic X should drop both. X-11 is the same matter as item 252 and stays open with it.
- D (clip actions): BF247 means a clip's own actions can play for the preview alone from the moment of a click, at the tempo's BPM, not lined up with the 1 (assumption B3-3). The actions engine has to allow a start that is not on the show's beat for the preview. BF250 ("tempo stop stops all actions, not just global") is D's: whether tempo stop switches a clip's action buttons off decides what a preview by name shows after a stop; this paper only says the preview shows the actions that are switched on.
- A (tempo bar and triggering): BF247 "only when they trigger and play should they be on time with the beat" confirms A's trigger rules and changes none; previewing never starts the tempo (item 221) stands beside A's rule that a trigger while stopped starts the beat. BF271 "Stop removes all clips from all layers" (A's rule) is applied here only as: after a stop the cue shows empty layers (B3-5).
- K (MilkDrop): BF265 "Keep Milk drop as it is" is applied here to the limit of the first build (B-11): no second MilkDrop for the preview.
- I (blend modes, keying, masks): BF260 says the blend modes and keying stay and masks come later. The cue mixes the cued layers by their blend modes (R141, R149 d) and that stands; what a mask layer does inside the cue is not ruled here and waits for I's rule on masks.
- C (item 253, accepted): only files and deck clips are previewed; nothing here contradicts it.
- F (BF256: "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play"): a previewed clip that is deleted leaves the preview black (old B-21, untouched); F owns the rule for the layer.
- J (names): the row "preview mode" of NAMES.md changes its meaning (see NAMES); the row "preview a clip" already says "plays at once" and stands. The word Studio replaces Review in every text for him; the old assumption B-17 keeps "recording review mode" only inside his quote.

## CONFLICTS (from the paper)
- A clip previewed by its name: NEW, BF247: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat." EARLIER, binding-decisions.md line 1134 (R179; BF175): "R179 all good except a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". The newest words win: a preview starts at the click; its actions come with it and are not lined up with the 1 (B3-3 is the one line that says so to him). His still earlier line agrees with the new one: binding-decisions.md line 1133 (R170; BF174): "They will play right away".
- Not a clash, said once more so that it is clear: BF248 asks "what and where is the master cue?" about a button he named himself, binding-decisions.md line 1136 (177; BF177): "177 add a global effects and actions toggle button called master cue". He is not withdrawing it; he asks what Harmony made of it.

## NOT DONE / UNSURE (from the paper)
- Item 252: this paper's answer is a draft from the fact sheets (read, not run). Whether a second picture every frame leaves the output's frame rate alone is NOT measured; the cheapest way to settle it is the spike the old spec already names (blend the saved per-layer pictures into a small second target and read the frame time with and without it), when the build hold lifts. The final text of @@ANSWER 252 and of B3-2 is settled with answer-252.md, which was not there when this was written.
- "Drawn at the size it is shown" is safe for cued layers (they are the output's own layer pictures made smaller: INFERRED from CompositorEngine.cpp:1150-1154, read, not run). For a clip previewed by its name, which is drawn only for the preview, effects that count in pixels (a blur, a pixelate) could look slightly different at a smaller size: ESTIMATE, not checked; way b of B3-2 (draw at full size) is the cure and costs more. The technical paper should rule this.
- A preview that starts at the click needs a clock of its own for the previewed clip and its actions (one player per clip; a clip seen twice is the same playback: facts-app-cue-today.md section 5). How a previewed video starts "at once" depends on how fast its first frame is ready: not measured; the rule only forbids waiting for the beat.
- B3-4: if he strikes the line and means way b (a clip put into the cue of a layer without reaching the output), the cue system grows by a second playback per layer: to be planned only on his word.
- What tempo stop does to a clip's action buttons (BF250, topic D) decides what a preview by name shows after a stop; settled by D's paper, then one sentence in R151.
- A mask layer inside the cue (BF260): waits for topic I.
- No old assumption of this topic is dropped: B-5, B-7, B-8, B-17, B-19, B-20 and B-21 are untouched by his 33 boxes; B-14 and B-18 change and keep their ids.
- Written 2026-10-09 18:47:48 EDT by the architect of topic B; read-only, nothing built, run or measured.

## FOR THE PAGE RULING (from the ruling)
- MUST be put to him, heaviest first: (1) B3-2 with @@ANSWER 252 -- he asked, and it is the technical call he handed over. The text for him is @@ANSWER 252 of this ruling, NOT the "ANSWER FOR BORIS" paragraph of answer-252.md, which check-252.md found to promise too much; the merge appends that paper's RECOMMENDATION as written, and its points 2 and 6 stand corrected by ITEM 252 here (a player of the preview's own for a clip previewed alone; no guarantee). (2) B3-1 with @@ANSWER 222 -- he asked what and where master cue is; old 177 and everything about master cue waits on it.
- Lines he can strike (3): B3-4 first -- it reads his newest sentence (BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time") as "nothing new is built": show it, do not merge it away. Then B3-3 (a preview's actions run from the click, not lined up with the 1). Then B-11 (two narrow limits: a MilkDrop clip or a simulation previewed while another of its kind plays).
- Internal, not for him (5): B3-5, B3-6, B3-7, B-14, B-18.
- Newest against earlier, to be said to him once: BF247 "Previewing a clip should not happen on the beat." against BF175 "it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". The newest wins; B3-3 is the line that tells him what it does to the actions.
- Newest against earlier, to be said to him once: item 221, accepted by his empty box (a preview by name plays on through a tempo pause, a BPM mode clip and its actions too) against BF167 "Pause pauses, the beat clock, the clips and everything that it controls with BPM." The accepted item wins; the pause still holds the clips on the layers.
- Not a clash: BF248 asks about a button he named himself (BF177 "177 add a global effects and actions toggle button called master cue"); @@ANSWER 222 says so.
- Depends on topic D: (1) D's question whether tempo stop switches the button of every clip's action off (BF250 "tempo stop stops all actions, not just global"): if yes, a preview after a stop shows clips without moving actions until he switches one on again (ITEM 221); worth one clause in D's question. (2) D's engine must play a clip's actions for the preview alone, from a click, off the show's beat (B3-3, B3-6); in the show D's rules stand (an action waits for the beat).
- Depends on topics F, I, K, A: F's line on the triggering click ("the preview does not change", a LINE in F's paper) is the twin of the internal B3-7: ask it once, in F. I: keying and masks in the cue (AMEND R149 2) follow I's rule for page item 240; what a One Shot and an envelope do in a preview is ruled here (B3-6), as I's paper asked. K: a MilkDrop clip previewed by its name leans on the one MilkDrop (BF265 "Keep Milk drop as it is"); its preset-load hitch is not measured. A: the trigger rules decide when a cued clip starts (B3-4).
- U18 is amended here AND in apply-X with the same OLD text; the merge can apply only one. This ruling keeps B's (item 221 is made from U18; the rule in it is the cue topic's). If rule-X keeps X's too, B's is applied first and X's shows in the merge report as "NOT applied": that is the wanted outcome, and nothing has to be done by hand.
- Owed before the cue build, Harmony's to schedule with him (answer-252.md, UNKNOWN UNTIL MEASURED 1 to 5; none needs anything built): the frame time of his heaviest show, the cost of master cue, the cost of one previewed clip, the MilkDrop preset-load hitch, which screen paces the show. Added here: the time from a name click to the first picture of a video (his word "quick", BF247).
- The two marks (a paused preview; "not a run of its own") are looks, laid out by Harmony (BF163); not asked.

