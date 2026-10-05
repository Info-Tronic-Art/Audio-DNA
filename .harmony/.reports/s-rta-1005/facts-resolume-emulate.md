# Facts sheet: how Resolume Arena does it (for Boris's 'please emulate this')
Written: 2026-10-05 14:12:42 EDT by Researcher (s-rta-1005). Nothing built. No design, no recommendation, no question for Boris.
Tool note: firecrawl search errored (empty); exa fetch/search used instead.
Labels: VERIFIED = I read it (URL + words, or the picture file). INFERRED = from what is named. UNKNOWN = cheapest check named.
Version: Resolume's manual pages list v7 .. v7.25 (VERIFIED, https://resolume.com/support/en/clips, accessed 2026-10-05). The pages read are the current 7.x text unless a line says otherwise.
Source tiers: OFFICIAL = resolume.com/support pages, release notes, Resolume's own video. FORUM-STAFF = forum reply by Resolume team (named where the page showed a name). COMMUNITY = other forum users / third-party sites.

## 0. WHICH ARENA BORIS HAS
0.1 VERIFIED: His tempo-bar picture (.harmony/.reports/s-rta-1004b/boris-images/resolume-tempo-bar.png) shows, left to right: a round four-part button (Ableton Link), PLAY (lit green), PAUSE, STOP, "BPM 256", minus, plus, two nudge buttons, "/2", "x2", TAP, RESYNC.
0.2 VERIFIED: A Stop button on the tempo bar was added in 7.22: "There is now also a stop button on the global transport" (OFFICIAL, https://www.resolume.com/blog/27125, accessed 2026-10-05).
0.3 INFERRED (from 0.1 + 0.2): his Arena is 7.22 or newer. Exact number UNKNOWN; cheapest: one click of his, Arena > About.

## A. EFFECT PRESETS
A1. Where the control sits
A1.1 VERIFIED: "use the Preset drop down above the effect parameters and select the Save option" (OFFICIAL, https://resolume.com/support/en/effects, accessed 2026-10-05).
A1.2 VERIFIED: effects go onto "the Composition, Group, Layer or Clips panel" (same page), so the same effect box, with the same preset control, appears in all of them. (The manual does not say the box differs by panel.)
A1.3 VERIFIED (his picture resolume-effect-panel-presets-menu.png): the open menu has a grey heading "Arena Presets", then Default, Blue, Green, Red, a divider, then "Manage...", "Save". No "Save As". No tick or dot beside any entry (his monitor menu, by contrast, marks the chosen item with a green dot).
A1.4 VERIFIED (same picture): the effect box is headed "Add Subtract"; its header row ends with three small buttons: "B", "P" with a small arrow, "X". The row "Blend Mode  Add" ends with a second "P" with a small arrow. Parameter rows R -100 %, G -100 %, B 0 %.
A1.5 VERIFIED: B = bypass, X = remove: "All effects can be temporarily bypassed (B toggle) or removed (X button)" (OFFICIAL v4 manual https://resolume.com/download/Resolume-4-Manual-English.pdf) and "You can also animate the effect bypass switch. When you right-click on the B icon..." (OFFICIAL effects page).
A1.6 INFERRED: the "P" in the effect's header row is the effect-preset menu (the one in his picture). From: effects page says "Preset drop down above the effect parameters"; Wire manual says "hit the P-icon next to the cogwheel... save, load and manage presets" (https://resolume.com/support/en/wire-user-interface). The picture does not show which P the open menu hangs from.
A1.7 INFERRED: the second "P" in the Blend Mode row is a preset menu for that one parameter, not the effect. From: "Palette ... access them via the little P dropdown on the far right" (OFFICIAL https://resolume.com/support/en/parameters); envelope "little P" (https://resolume.com/support/en/envelopes); parameter presets at the cogwheel menu bottom (https://resolume.com/support/en/parameter-animation). Whether a Blend Mode "P" exists in the manual: UNKNOWN; cheapest: Boris clicks that P once and sends a picture.
A1.8 VERIFIED (older, v4 manual): the preset menu then also did "rename presets, delete a preset or reset the effect settings to their defaults" (OFFICIAL v4 manual). Today's menu shows "Default" and "Manage...", which INFERRED carry the reset and rename/delete.

A2. What "Save" does
A2.1 VERIFIED: "select the Save option. The Manage Presets window opens, you can enter a name and hit return to save." (OFFICIAL effects page). So Save opens the Manage Presets window, with a name entry; Return saves.
A2.2 UNKNOWN: what text the name field starts with (empty / "Untitled" / the loaded preset's name). A weak hint: a user saw presets auto-named "Untitled 1", "Untitled 2" after a rename bug (COMMUNITY, https://resolume.com/forum/viewtopic.php?t=18989). Cheapest: Boris opens Save on any effect and sends a picture of the window.
A2.3 UNKNOWN for 7.x: saving under a name that already exists. Older evidence (v4 manual, OFFICIAL): "If you make any changes to a preset, you will need to use the Preset dropdown Save option to save the changes" (so Save over a loaded preset updated it), and that menu had "Save As...". Today's menu has "Save" only (A1.3). Cheapest: Boris tries it on a throw-away preset.
A2.4 INFERRED: the 7.x "Save" is both save and save-as through the name window (from A2.1 + A1.3: no separate "Save As").

A3. "Manage..."
A3.1 INFERRED: "Manage..." opens the same Manage Presets window as A2.1, where presets are saved, renamed and deleted. From: envelope presets: "You can delete or rename presets by pressing Manage..." (OFFICIAL https://resolume.com/support/en/envelopes); Wire: "preset manager. Here you can save, rename and delete your presets" (OFFICIAL Wire page); v4 manual (A1.8). The effects page itself does not describe Manage... for effects.
A3.2 UNKNOWN: the exact look of the window (list, rename by double-click, delete key). Cheapest: Boris opens it, one picture.

A4. Which preset is loaded, and "changed"
A4.1 VERIFIED: "The effect doesn't remember which preset was used to get its current settings. It doesn't even care if the octopus did it, or you did it yourself by setting everything by hand. So, when you update a preset, other effects that were set to this preset ... don't get updated" (OFFICIAL effects page, "octopus" paragraph).
A4.2 VERIFIED: same stance in a team reply: "The effect has no concept of which preset is applied to it... The parameter values are then stored and recalled via the composition, the preset itself is no longer used" (FORUM-STAFF-style reply, name not shown in the page extract, 2015, https://resolume.com/forum/viewtopic.php?t=11938; the same message says they had talked about a 'Save and Update' system). Dated 2015: the 2026 effects page repeats it, so still true as documented.
A4.3 VERIFIED (his picture): the effect box shows the effect name ("Add Subtract"), not a preset name; no tick in the menu; the values shown (R -100 %, G -100 %) look like the preset "Blue" (INFERRED reading of the numbers).
A4.4 INFERRED: so there is NO loaded-preset name, NO tick in the menu and NO "changed" mark in Resolume. (Follows from A4.1 + A4.3; I found no manual line that says "there is no changed mark".)
A4.5 VERIFIED: a user can rename an effect instance: "right-clicking the name of the effect and then selecting Rename" (OFFICIAL effects page). That is the only way a name beside the effect changes.

A5. The Effects tab
A5.1 VERIFIED (his picture resolume-effects-tab-presets.png): heading "VIDEO EFFECTS"; "Acuarela" (no arrow); "Add Subtract" with a down-arrow (unfolded) and, indented under it, "Blue" (highlighted), "Green", "Red".
A5.2 VERIFIED: "The new preset will now appear in the effects list, under the name of the effect itself." (OFFICIAL effects page). An effect with no presets has no fold arrow in his picture (INFERRED from "Acuarela" having none).
A5.3 INFERRED: the arrow folds/unfolds the effect's presets (shown unfolded in the picture; the manual has no sentence on the fold). Cheapest: one click of his.
A5.4 VERIFIED: "To use a preset, drag it onto the Composition, Layer or Clip in the same way as you would for the effect itself." (OFFICIAL effects page).
A5.5 INFERRED: the effect arrives in the panel under the EFFECT's name, with all its parameters set to the preset's values (from A5.4, the octopus paragraph, and his picture A4.3). Not stated in the manual in one sentence.
A5.6 INFERRED: dragging the effect itself puts the effect in with its default values (the "Default" entry in the menu). Not stated as such.
A5.7 VERIFIED: drag targets also include "directly drop them onto a clip or layer", and a quick search: "Double click on the panel header, empty space in the panel or clip handle" (OFFICIAL effects page). "Hold down ALT while dropping an effect to apply it with its opacity turned all the way down."
A5.8 VERIFIED: "To preview an effect, double click its name in the effects panel. It will show in the Preview Output, applied over whatever is playing in the composition." (OFFICIAL effects page). Double-click on a PRESET row: UNKNOWN (cheapest: one double-click of his).
A5.9 VERIFIED: "You can favorite your effects ... click the heart icon" (OFFICIAL effects page): a favorites tab exists beside the list.

A6. On disk, and what a preset holds
A6.1 VERIFIED: user presets live in the user's Documents under "Presets: user presets for sources, effects, GUI and the advanced output" (OFFICIAL https://resolume.com/support/en/7/directory-list, accessed 2026-10-05). That page names the folder "Documents/Resolume Avenue/" for both editions.
A6.2 VERIFIED (COMMUNITY): one file per preset, XML, in "\Documents\Resolume Arena\Presets\Video Effects\(Effect name)" (user, https://resolume.com/forum/viewtopic.php?t=18989, Windows). A Mac user: ".../Documents/Resolume Arena/Presets/Sources/..." for source presets (https://resolume.com/forum/viewtopic.php?t=21347). Mac effects path therefore INFERRED to be ~/Documents/Resolume Arena/Presets/Video Effects/<effect>/<name>.xml.
A6.3 VERIFIED: "Each preset contains all the settings for an effect" (OFFICIAL effects page).
A6.4 VERIFIED (forum test, https://resolume.com/forum/viewtopic.php?t=18989; replier's name not in the extract): "when I make a parameter animated to timeline, paused, loop, store the preset1 ... load the previous preset1, my param goes back to paused and loop properly" - so a preset keeps a parameter's animation mode (timeline / loop / paused).
A6.5 VERIFIED: a release-note fix title: "Dashboard Link positions not saved properly in Effect Preset" (OFFICIAL, #13766, https://resolume.com/blog/19640) - so an effect preset is meant to hold the effect's Dashboard-link positions.
A6.6 VERIFIED: Copy Effects / Paste Effects: "All settings, presets and animations will be copied along" (OFFICIAL effects page).
A6.7 INFERRED: audio-analysis (FFT) and BPM-sync settings on a parameter are saved inside the preset along with the animation mode (A6.4). The parameter-presets text lists "playback mode, loopback mode, timeline mode, duration and envelope, start settings and animation settings. In/Out points are not saved." (OFFICIAL, https://resolume.com/support/en/parameter-animation) but that is the PER-PARAMETER preset, not the effect preset. Whether effect presets include FFT links: UNKNOWN; cheapest: Boris saves an effect with an FFT-driven slider and re-loads it.

## B. MONITORS AND PREVIEW
B1. The monitors and the menu
B1.1 VERIFIED (his pictures): top box titled "Composition Monitor", caption "Composition - 1280x720". Lower box titled "Clip Monitor", caption "IntoTheGlow - 1280x720", with a "Fit" dropdown, a hand icon and a cogwheel at its bottom right (the top box shows only a cogwheel).
B1.2 VERIFIED (resolume-monitors-menu.jpg): menu, from top: "Monitor": Composition, Preview, Selected Clip (green dot = current, here Selected Clip). "Layers": Selected Layer, Layer 3, Layer 2, Layer 1. "Crossfader": Result, Bus A, Bus B. Then Snapshot, Copy Image. Then Transform Widget (green dot = on). Then two small swatches (one checkered, one black).
B1.3 VERIFIED: the cogwheel at the monitor's bottom right opens it: "Press the cogwheel at the bottom-right of a monitor to change what you want to monitor" (OFFICIAL https://resolume.com/support/en/layouts).
B1.4 VERIFIED (OFFICIAL video, Resolume VJ Software, "Resolume Arena Tutorial - Monitors", https://www.youtube.com/watch?v=KecYCWg5RyY, transcript read 2026-10-05): "the composition monitor shows the final output the result of all layers mixed together."
B1.5 VERIFIED (same video): "Preview": by default the preview monitor has a blocky (checkered) background, the composition monitor black; Preview shows a clip you chose by its handle ("I don't click on the thumbnail but instead on the handle below the thumbnail ... preview the clip before I actually trigger it"), also an effect, a source, or a file from the Files panel double-clicked, "outside of the composition".
B1.6 VERIFIED (same video): "Selected Clip / Selected Layer / Selected Group ... allow you to view the selected clip layer or group ... different from the layer or group monitors as they are locked to a specific layer or group."
B1.7 VERIFIED (same video): "Layer": "selecting layer will allow you to preview a specific layer ... useful when you are using different blend modes on each layer or you need to isolate a certain layer"; "Groups are similar".
B1.8 VERIFIED (same video): Crossfader "Result" = "the mix between A and B ... all layers that are not assigned to the crossfader are excluded from this monitor"; Bus A / Bus B = the A side / B side of the crossfader. Also OFFICIAL layouts page: layers inside groups are not part of the global crossfade and not visible in these.
B1.9 VERIFIED (same video): "Snapshot" = takes a snapshot of the monitor and adds it to an empty clip as a still (stored in its own folder); "Copy Image" = puts the monitor's picture on the clipboard; "Transform Widget" = toggles the move/scale/rotate widget for that monitor only (on in the preview monitor by default, off in the composition monitor); background = black or transparent (checker) per monitor (the two swatches, INFERRED to be that choice).
B1.10 VERIFIED: OFFICIAL clips page: "Copy Image ... will copy the current frame of that monitor to your clipboard".
B1.11 VERIFIED: more monitors: "right click on an existing monitor and press duplicate" (OFFICIAL layouts page). So one monitor shows ONE thing; several monitors side by side is how Resolume shows several.
B1.12 VERIFIED: the Composition button (top left): "will let you preview the composition, even when it's faded down" (OFFICIAL https://resolume.com/support/en/composition).
B1.13 INFERRED: In his picture the lower box is in "Selected Clip" mode (green dot) and its title is "Clip Monitor"; "Preview" is a separate menu entry. Resolume's own words for the lower box are "preview monitor" in the 2024 video and "Clip Monitor" on his screen (whether the title follows the mode: UNKNOWN).

B2. Click the NAME against click the THUMBNAIL
B2.1 VERIFIED: "Triggering a clip is as simple as clicking its thumbnail." (OFFICIAL clips page).
B2.2 VERIFIED: "You can select a clip, rather than trigger it, by clicking the name handle underneath the thumbnail." Reason given: "Want to change a setting before a clip is live?" (OFFICIAL clips page).
B2.3 VERIFIED: name-handle click shows the clip in the preview monitor without triggering (OFFICIAL video, B1.5).
B2.4 INFERRED: the previewed clip PLAYS in the monitor (not a still). From: old manual "take a peek at a layer or clip without actually playing it out" (OFFICIAL v4 manual) and a user: "the clip will playback in the preview monitor if I preview it" (COMMUNITY, Arena 4.1.6, https://resolume.com/forum/viewtopic.php?t=10495). No official sentence "it plays and loops". Cheapest: Boris clicks a name on a video and watches.
B2.5 INFERRED: it has its own transport: selecting the clip fills the clip panel with that clip's Transport (B2.2 reason + the Transport section of the clip panel, https://resolume.com/support/video). On Beat Snap: "This also works with beat snap, but only if the clip is not being previewed" (COMMUNITY, https://resolume.com/forum/viewtopic.php?t=18032) - hints a previewed clip is in a state of its own.
B2.6 VERIFIED (older, v4 manual, OFFICIAL): "When previewing, the volume and opacity parameters of the layer or clip are ignored."
B2.7 INFERRED: it shows the clip's OWN effects (COMMUNITY: "If you apply an FX to the Clip being previewed, it will show up in the preview window", https://resolume.com/forum/viewtopic.php?t=14340, 2017). Layer/composition effects on a clip preview: UNKNOWN. Cheapest: Boris tests with a layer effect on.
B2.8 VERIFIED: previewing never touches the output: "try things out before sending them out to the main audio and video outputs ... without actually playing it out" (OFFICIAL v4 manual); "preview the clip before I actually trigger it" (OFFICIAL video).
B2.9 VERIFIED (FORUM-STAFF-style reply, 2018, https://resolume.com/forum/viewtopic.php?t=15834): "Clip preview is cleared when a new clip is launched manually ... Auto auto pilot doesn't clear preview any more, Layer, group and composition previews are not cleared." A preference "Update clip panels on external triggers" exists (OFFICIAL https://www.resolume.com/support/en/preferences) so MIDI/OSC triggers need not move the clip panel.
B2.10 VERIFIED: a file can be previewed too: "double click on a file in the Files panel to preview it in your preview monitor" (OFFICIAL clips page).

B3. A LAYER in a monitor
B3.1 VERIFIED: a layer monitor works with the layer faded down: "trigger it in a layer with the opacity turned down and then preview the layer. This is similar to how you would preview a clip looks with layer effects applied" (team reply, 2015, https://resolume.com/forum/viewtopic.php?t=11797); "The only way to preview an effect on a clip, layer or the comp itself, is to fade it out, apply the effect and preview it" (Joris, Resolume team, 2017, https://resolume.com/forum/viewtopic.php?t=14340).
B3.2 INFERRED: so a layer monitor is "before the fader" (layer opacity does not dim it) and includes the layer's own effects (B3.1 "with layer effects applied"). It is not stated as a rule in the manual.
B3.3 UNKNOWN: whether bypass (B) on the layer blanks its monitor. Cheapest: Boris presses B on a playing layer with a Layer monitor open.
B3.4 INFERRED: a layer monitor does NOT include composition effects (composition effects are "applied to the final output, after the layers have been mixed together", OFFICIAL effects page). Not stated for monitors; cheapest: Boris puts a composition effect on and looks at a Layer monitor.
B3.5 VERIFIED: one monitor = one layer, OR one group (the group monitor shows that group): B1.7, B1.11. Showing SEVERAL chosen layers mixed in ONE monitor: no menu entry for it. The nearest ways are (a) a Group monitor (the layers inside the group, mixed: INFERRED from what a group is), (b) Crossfader Result (layers assigned to bus A and B, mixed; B1.8), (c) several monitors side by side (B1.11).
B3.6 VERIFIED (team reply, 2015, https://resolume.com/forum/viewtopic.php?t=12328): "Previewing several clips or layers together is currently not possible in Resolume." Older answer (2015; and a 2014 reply: "We are researching how you could elegantly preview multiple layers together", https://resolume.com/forum/viewtopic.php?t=11069). The 7.x monitor menu has added groups and crossfader modes since; no 7.x statement found that a free multi-pick exists.
B3.7 UNKNOWN: whether a Group monitor shows the group before its own opacity/bypass. Cheapest: one test of his.
B3.8 VERIFIED: Solo is an OUTPUT thing: "The S will Solo the layer, hiding everything but the current layer" (OFFICIAL layers page); it is not in the monitor menu (B1.2).

B4. Preview on a second screen
B4.1 VERIFIED: "To undock a panel from the main interface, right click on the panel and select undock. This will create a new window which you can drag onto your secondary ... screen." Works for all panels (OFFICIAL layouts page; OFFICIAL video: "this works for all panels not just for monitors"). So a monitor (incl. Preview) can sit on a second screen as a window.
B4.2 VERIFIED: layouts are saved as XML presets (OFFICIAL layouts page).
B4.3 UNKNOWN: whether the Advanced Output (the real output) can be fed from Preview. The monitor menu lists "Advanced Output Screens" as something you can WATCH (OFFICIAL layouts page); no line says Preview can be OUTPUT. Cheapest: Boris looks in Advanced Output for a source list.
B4.4 VERIFIED (third-party): a stock layout "dual display sends the preview to a second monitor" (COMMUNITY, https://vjacademy.info/resolume-composition-layout; not Resolume's own page).

B5. Cost or limit of previewing
B5.1 UNKNOWN: no Resolume statement of a cost (GPU, frame rate) for previewing found. Cheapest: Boris watches the FPS readout (his picture shows "FPS 110.2", VERIFIED) with and without a Preview open.
B5.2 VERIFIED limits: previews ignore volume/opacity (B2.6, v4); clip preview is cleared on a manual trigger (B2.9); layers inside groups are absent from crossfader monitors (B1.8); no multi-layer pick (B3.6).

## C. BPM SYNC AND WHEN A CLIP STARTS
C1. Beat Snap
C1.1 VERIFIED: where set: "Beat Snap option for the whole Composition through the Composition > Beat Snap menu"; "for an individual clip ... Clip > Beat Snap"; a clip may say "Composition determined" (OFFICIAL clips page). Shift-select several clips and change them together (same page).
C1.2 VERIFIED: since 7.22 columns also have Beat Snap: "columns now also have beat snap settings, just like clips" (OFFICIAL https://www.resolume.com/blog/27125).
C1.3 UNKNOWN: a Beat Snap setting on a LAYER. The 7.x manual lists composition, clip and (7.22) column only; layer not named. Cheapest: Boris right-clicks a layer name.
C1.4 VERIFIED: values: "wait until the next beat, the next bar, in 2 bars, in 4 bars and so on" plus "none" (OFFICIAL clips page; "Composition > Beat Snap > None" in a team reply, 2017, https://resolume.com/forum/viewtopic.php?t=14485). A user sets "the BEAT SNAP of the composition to 1/2nd" (COMMUNITY https://zunayed.com/outlines-in-bpm-tempo-resolume-vj-tips/), so sub-beat steps exist. Full list: UNKNOWN; cheapest: Boris opens Composition > Beat Snap, one picture.
C1.5 VERIFIED: the wait: "Try clicking another thumbnail on the same layer as the one that is already playing. You will see that, at the start of the next bar, the output will change to play the new clip." (OFFICIAL, https://resolume.com/support/en/quickstart-tutorial). So the previous clip keeps playing until the snap point; the new one is not shown before.
C1.6 VERIFIED: waiting indicator: "the horizontal bar ... What you're seeing is the beat snap bar. The beat snap bar waits for the next whole musical bar" (team reply, 2017, https://resolume.com/forum/viewtopic.php?t=14485). In 7.22: "countdown dials on the clip thumbnails ... dials turn into countdown pies when in BPM sync mode ... These pies also replace the bars that were previously used to indicate beat snapping" (OFFICIAL https://www.resolume.com/blog/27125). A transition shows a similar "T in a circle" spinner that can be mistaken for beat snap (team reply, 2024, https://resolume.com/forum/viewtopic.php?t=27430).
C1.7 VERIFIED: if the clock is not running a beat-snapped clip never starts: "any clips set to beat snap will forever wait for the next beat, and never actually trigger" (Joris, Resolume team, 2011, https://resolume.com/forum/viewtopic.php?t=7886; old; the 7.22 global clock changed pause/stop - see C3).
C1.8 VERIFIED (COMMUNITY, 2018, https://resolume.com/forum/viewtopic.php?t=15925): triggering an EMPTY slot after a beat-snapped clip also waits for the snap; layer eject (X) is immediate.
C1.9 UNKNOWN: whether a layer with a waiting clip shows anything other than the thumbnail's pie. Cheapest: Boris triggers a clip mid-bar and sends a picture.

C2. Is a snap implied by BPM Sync?
C2.1 VERIFIED: NO - they are two settings. "Enable Beat-snap for all clips (not only the ones in bpm-sync mode)" (OFFICIAL Resolume 3 release notes, https://www.resolume.com/blog/4490, #463), so Beat Snap works for every transport mode, and is its own setting.
C2.2 VERIFIED: with Beat Snap "none" a clip starts at once: "By setting the beat snap to 'none' I can successfully trigger the clip" (COMMUNITY user, https://resolume.com/forum/viewtopic.php?t=9812); "Turn off beat snap" as the way to start a BPM-synced clip immediately (COMMUNITY, https://resolume.com/forum/viewtopic.php?t=22637).
C2.3 VERIFIED: a file with audio gets BOTH defaults together: "By adding audio to your clips, the resulting AV clip is automatically set to BPM sync mode, and also to a 1 bar beat snap" (Joris, Resolume team, 2011, https://resolume.com/forum/viewtopic.php?t=7886) - a default pairing, not an implied rule.
C2.4 VERIFIED: the example composition is shipped with Beat Snap on a bar: "these clips are set up to be synchronised to the BPM ... so the clip may not start playing instantly - it will wait for the start of the next bar. ... if you want to launch clips instantly, you can set them up to do that" (OFFICIAL quickstart). And a new composition "will put the comp Beat Snap to 1 bar" (COMMUNITY test, https://resolume.com/forum/viewtopic.php?t=15925, Res 5/6).
C2.5 VERIFIED: what BPM Sync does to speed: "all clips that have their transport mode set to BPM Sync will play at a speed that synchronises them with that BPM"; the clip spans a number of beats (default guessed to the nearest power of two); Speed modifier quantised to multiples of 2 (OFFICIAL, https://resolume.com/support/video).
C2.6 VERIFIED: where a clip starts when triggered: "Playmode Away ... The first option plays the clip from the start. This is the default"; "pick-up" starts "from wherever it was when it was last played" (OFFICIAL, https://resolume.com/support/video).
C2.7 INFERRED: a BPM Sync clip with Beat Snap none starts from its first frame at the click, so in general NOT in phase with the bar. From C2.6 + the quickstart line: messing with a clip makes it "out of phase. You can resynchronise it by clicking the clip thumbnail again - it will start again at the start of the next bar" (OFFICIAL quickstart). Against it: a COMMUNITY user asked for "start immediately but still be in time" and accepted "Turn off beat snap" (C2.2); that exchange does not say the picture was in phase. Cheapest: Boris triggers a BPM Sync clip mid-bar with snap none and watches the playhead against the bar.
C2.8 VERIFIED: "Resync" makes "everything in Resolume that is set to BPM Sync ... jump back to the first beat of the first bar of the first phrase" (OFFICIAL https://resolume.com/support/en/bpm).

C3. The tempo bar
C3.1 VERIFIED: Tap: "clicking the 'Tap' button a few times to set the tempo ... in time with the beat" (OFFICIAL bpm page). Resync: C2.8, "click the 'Resync' button on the first beat of the new phrase". A tempo can also be typed by clicking the BPM value.
C3.2 VERIFIED: nudge: "'Nudge Up' and 'Nudge Down' ... temporarily speed the tempo up or down while you have the button pressed, and set it back ... when you let go" (OFFICIAL bpm page).
C3.3 VERIFIED: 7.22 global clock: "The BPM clock can now act as a global clock that can be paused and stopped. This means that when the clock is paused, all transports, including timeline animations, will be paused." New compositions default to "BPM Clock and Timeline"; old ones are "BPM Clock Only" (only BPM-synced things follow) - set in the Composition menu, Global Transport (OFFICIAL, https://www.resolume.com/blog/27125).
C3.4 VERIFIED: Stop: "this stops and rewinds all timeline and BPM animations and ejects all playing clips" (same note).
C3.5 VERIFIED: Resync with the new option: "when resyncing your composition timelines will also be reset previously only BPM sync parameters would be reset" (OFFICIAL 7.22 video transcript, https://www.youtube.com/watch?v=-tnKnEPgg0Q).
C3.6 VERIFIED: BPM Sync clips while the tempo is paused: they pause (7.22 note, C3.3: "all transports ... paused"). Older: "BPM Pause stops video clips in BPM sync mode but not audio clips" (OFFICIAL Resolume 3 fix #412, https://www.resolume.com/blog/4490).
C3.7 VERIFIED: with Ableton Link on, Pause and Resync are greyed out "because it would manipulate the beat grid, and pause playback across other applications" (team reply, 2023, https://resolume.com/forum/viewtopic.php?t=23012).
C3.8 UNKNOWN: triggering a clip while the tempo is PAUSED: does it show (a still frame) and wait to move; and while STOPPED: does it start the clock, wait, or start nothing. I found no Resolume text. The only related line: a beat-snapped clip waits for a clock that is not running (C1.7, 2011). Boris's own rule (binding-decisions.md line 1091: "if I fire a clip while the beat is stopped, it starts playing the beat. When paused, it will not play but will still display.") has no Resolume source I could find. Cheapest: Boris fires a clip in Arena with the tempo bar on pause, then on stop, and reports what he sees for a beat-snap-none clip and a one-bar clip.

C4. Triggering a column
C4.1 VERIFIED: "You can trigger multiple clips at the same time by triggering the column" (OFFICIAL clips page).
C4.2 VERIFIED (7.22+): "Selecting a column no longer triggers that column; instead, you use the column play button to trigger columns"; the button shows play when the column has clips and a stop icon when empty ("when you trigger that column, all playing clips will stop") (OFFICIAL https://www.resolume.com/blog/27125). Older versions: "trigger found at the top of the column" (OFFICIAL v4 manual).
C4.3 VERIFIED: columns have Beat Snap settings since 7.22 (C1.2). What a column snap does to the clips inside (all together at the snap point, or each by its own clip snap): UNKNOWN. Cheapest: Boris sets a column snap and a different clip snap and fires.
C4.4 VERIFIED: layers/clips can be exempt: "Ignore Column Trigger ... the clip or layer ... doesn't get triggered" (OFFICIAL clips + layers pages).
C4.5 VERIFIED: "Next and previous column" buttons exist and (7.22) skip empty columns (OFFICIAL 7.22 note).

## D. BRIEF CONTEXT
D1 VERIFIED: Resolume has no recorded-performance timeline of parameter moves in what I read. Its "Record" records the OUTPUT as a video file: "The record function records (parts) of the composition output or advanced output to disk" (OFFICIAL https://resolume.com/support/en/recording); old manual: "meant to quickly record ... content that has a few effects or modulations applied, so that for instance you do not have to continuously keep moving a fader" (https://resolume.com/support/en/6/recording).
D2 VERIFIED: nearest thing = per-parameter animation: every parameter has a cogwheel menu with Timeline, BPM Sync, Clip Position, Audio Analysis (FFT), Crossfader Phase, and an Envelope (keyframe curve with "P" presets); animation can be saved in a parameter preset (OFFICIAL https://resolume.com/support/en/parameter-animation and /envelopes). You draw the curve; it is not recorded from moves.
D3 VERIFIED: "Parameter start settings": an animation starts at composition load, clip trigger or column trigger, or manually (same page); BPM Phase Lock can be off (same page).
D4 INFERRED: I found no "ignore" / "bypass automation" switch on a parameter in the manual. A parameter's animation is turned off by choosing another mode in its cogwheel menu (INFERRED from the menu listing). Not found does not mean absent; cheapest: Boris opens a cogwheel and sends a picture.
D5 VERIFIED: every parameter's input can be MIDI/OSC/mouse: "there's no difference between controlling Resolume via MIDI, OSC or the mouse"; an envelope is applied "to the input of your parameter ... even parameters controlled via MIDI or OSC" (OFFICIAL parameter-animation and envelopes pages).

## SURPRISES (things that cut against what the questions assume)
S1. No preset name shown, no tick, no "changed" mark in Resolume (A4). Boris's "the name stays" plus a Save button on change (binding-decisions 150) is NOT how Resolume does it.
S2. Resolume's Save opens a Manage Presets name window; his picture has no "Save As" (A1.3, A2.1).
S3. Resolume presets are one-shot value setters; updating a preset changes no placed effect (A4.1-A4.2).
S4. The lower monitor is titled "Clip Monitor" on his screen and is in "Selected Clip" mode; "Preview" and "Layers" are other menu entries (B1.13).
S5. One Resolume monitor shows ONE layer or group; no free multi-layer cue mix exists (B3.5-B3.6). "Resolume calls it the layer monitor" is only part right: Layer N entries are locked-layer monitors.
S6. Beat Snap and BPM Sync are separate settings; Resolume has no implied snap (C2.1). A BPM Sync clip with snap none starts at once, probably from its first frame (C2.7).
S7. Pre-7.22, a beat-snapped clip waits forever if the clock is not running; his Arena has the 7.22 global clock whose Stop also ejects every playing clip (C1.7, C3.4).
S8. Resolume has no performance-recording timeline (D1); only output video record and per-parameter animation.

## WHAT I COULD NOT ESTABLISH
U1. Exact Arena version of Boris (0.3).
U2. Name-field starting text and existing-name behaviour of Save (A2.2-A2.3), and the look of the Manage window (A3.2).
U3. What the Blend Mode row's "P" holds (A1.7).
U4. Whether a double-click on a preset row previews it (A5.8); whether effect presets hold FFT links (A6.7).
U5. Whether a previewed clip plays/loops, shows layer or composition effects (B2.4, B2.7).
U6. Layer monitor under bypass; composition effects in a layer monitor; group monitor before the group fader (B3.3, B3.4, B3.7).
U7. Whether Preview can feed an output screen; any cost of previewing (B4.3, B5.1).
U8. Beat Snap on a layer; the full snap list; the layer-strip look while waiting (C1.3, C1.4, C1.9).
U9. Phase of a BPM Sync clip started with snap none (C2.7).
U10. Triggering while paused / stopped in 7.22+ (C3.8) - no Resolume text found; his R111 words have no Resolume source I could find.
U11. What a column's Beat Snap does to the clips inside (C4.3).
U12. Any ignore/bypass-automation switch on a parameter (D4).

## CORRECTIONS (adversarial re-read)
Started 2026-10-05 14:13:13 EDT (in progress).
Completed 2026-10-05 14:16:26 EDT by Researcher (adversarial re-read). Re-fetched ~45 pages/pictures; tool note: WebFetch got 403 on forum, exa used; one exa search hit its rate limit.
Format: sheet line as written -> what is true -> source. Verdict: SOUND_WITH_CORRECTIONS. No Resolume-fact line was found contradicted by its own source except the three marked WRONG; the rest are re-labels (VERIFIED -> INFERRED) or context.
CHECKED OK (source says it): A1.1, A1.2, A1.3, A1.4 (pictures re-read), A2.1, A3.1 (envelopes + Wire pages), A4.1, A4.2 (Joris 2015, 11938), A4.5, A5.1-A5.9, A6.3, A6.5 (#13766 is in 7.1.1/6.1.5 notes), A6.6, A1.8 + A2.3 (v4 PDF: "Save As...", "Save option to save the changes", rename/delete/reset), B1.1-B1.12 (video transcript + pictures), B2.1-B2.3, B2.5, B2.6, B2.8, B3.6 (Joris 2015, 12328), B3.8, C1.1-C1.3, C1.5-C1.7, C2.1, C2.3 (Joris 2011), C2.5, C2.6, C2.8, C3.1-C3.5, C3.7, C4.1, C4.2, C4.4, 0.2.
WRONG (source does not say it):
W1. C3.6 "Older: 'BPM Pause stops video clips in BPM sync mode but not audio clips' (OFFICIAL Resolume 3 fix #412)" -> the ticket is #297; #412 is an unrelated mask-loading crash fix. Wording is right, the number is wrong. -> https://www.resolume.com/blog/4490 (accessed 2026-10-05)
W2. S1 "Boris's 'the name stays' plus a Save button on change (binding-decisions 150)" -> "the name stays" is Harmony's bullet heading, not Boris's words. His words (line 1080): "150 a is good, but as soon as the preset is changed, we have a little button to save this preset, ..." S1 is also a comparison of his wish against Resolume, i.e. a judgement, not a fact. -> /Users/boriskarpman/projects/RealTimeAudio/.harmony/binding-decisions.md:1080
W3. C1.8 "triggering an EMPTY slot after a beat-snapped clip also waits for the snap" (labelled COMMUNITY) -> it is Joris and Zoltan (Team Resolume), 2018, and the rule given is narrower: an empty slot beat-snaps with the COMPOSITION Beat Snap, not the previous clip's; layer eject is immediate. -> https://resolume.com/forum/viewtopic.php?t=15925
RE-LABELLED VERIFIED -> INFERRED (source is not Resolume's manual / staff / own video):
R1. 0.1 "(Ableton Link)" for the round four-part button -> the picture shows only an icon; the Link page does not describe it. Reading is INFERRED. -> https://resolume.com/support/en/link
R2. D1 "VERIFIED: Resolume has no recorded-performance timeline ... in what I read" -> an absence cannot be VERIFIED. The recording page confirms only what Record records (composition/advanced output to disk). Absence = INFERRED. -> https://resolume.com/support/en/recording
R3. A6.2 (XML file per preset, Windows/Mac paths) -> community forum users, not Resolume's manual; INFERRED. A6.1: directory-list names "Resolume Avenue" and says to read it as Arena for Arena. -> https://resolume.com/support/en/7/directory-list
R4. A6.4 (preset keeps animation mode) -> a forum test by Zoltan (Team Resolume rank) on Windows, 2019 (Add Subtract example); not manual; INFERRED. -> https://resolume.com/forum/viewtopic.php?t=18989
R5. B4.4 (stock layout "dual display" on a second monitor) -> third-party vjacademy.info; INFERRED (not re-fetched). C1.4 half-beat snap "1/2nd" -> third-party blog zunayed.com; INFERRED (not re-fetched). Manual text lists beat, bar, 2 bars, 4 bars "and so on"; "none" is from Joris 2017 (14485).
R6. C2.2 (snap none = clip starts at once) -> quotes are two users; the only staff words are Joris 2012 "Try setting it to 1 bar (or even none)" and 2017 "Composition > Beat Snap > None" as the way to turn snap off. Starts-at-once is INFERRED from them. -> https://resolume.com/forum/viewtopic.php?t=9812
CONTEXT / PRECISION (not false, but a planner could misread):
P1. B3.1 first quote (2015, Joris, 11797) -> it is about previewing a Layer Router / effect clip ("trigger it in a layer with the opacity turned down and then preview the layer"), not a plain layer; the 2017 Joris line (14340) is the general one. B3.2 stays INFERRED. -> https://resolume.com/forum/viewtopic.php?t=11797
P2. B2.9 "A preference 'Update clip panels on external triggers' exists (OFFICIAL preferences page)" -> the page says only "Clip Panel: choose whether or not Resolume should update the clip panel when a clip is triggered"; the quoted name is from Zoltan's 2018 post. -> https://www.resolume.com/support/en/preferences
P3. C3.3 names the old option "BPM Clock Only" -> the 7.22 note itself says both "BPM Sync only" and "BPM Clock Only"; the menu label is UNKNOWN. Stop behaviour in the old mode is not stated. -> https://www.resolume.com/blog/27125
P4. C4.3 / U11 "UNKNOWN what a column snap does" -> Resolume's own 7.22 video says "columns now have beat snap, this allows you to trigger them in sync with the BPM": the column trigger itself snaps; only the per-clip interplay stays UNKNOWN. -> https://www.youtube.com/watch?v=-tnKnEPgg0Q
P5. C2.4 second half "a new composition puts Beat Snap to 1 bar (COMMUNITY test)" -> said by Joris (team) 2018, for Res 5/6; no 7.x text. -> 15925
NOT-A-FACT CHECK: no design or recommendation found. Mild flags: the SURPRISES block (S1, S4, S5) argues against the owner's wording (judgement); the sheet says "no question for Boris" yet U1-U12 / "Cheapest: Boris ..." lines are asks Harmony may turn into questions.
