# APPLY E -- The review screen and recordings (s-rta-1007)
Written 2026-10-07 22:45:41 EDT by the topic-E architect. Read-only work: nothing built, run or launched. His words are quoted verbatim from boris-msg-numbered.txt (L numbers) and binding-decisions.md (BD:line). Short names: AR = s-rta-1005/area-recording.md (part.item or H/T/M/O point), FT = s-rta-1005/facts-takes-bpm-fire.md.

## SUMMARY
1. There are two recordings and only two: Record Show and Record to Clip (L114). A one-layer clip recording is gone; a clip recording is always the full composition, never one screen's corrected picture (L88).
2. A clip recording is always in time: it starts on the "1" and ends on a bar line after the stop press; nothing is recorded while the tempo is paused (L88). The case "REC with the beat stopped = a free-running clip" is gone.
3. A show recording now carries its own show file ("the recorded show file", L80, L89): the review opens from it whatever happened to the show since, and tells which clips have moved or are missing.
4. Tracks exist only for controls that changed during the night (L81); this reverses the earlier "a row for every control" and answers question 188.
5. A mend is never written into the original: it is another recording, parallel to the original (L84). Record Over stays, done with the MIDI controller in real time, and is also kept separately (L92).
6. While he reviews, the outputs show the review picture too (L91, 186 b); the picture can go to a monitor or be resized by dragging the line between picture and tracks (L79).
7. Removing something during a recording is recorded and kept in the recorded show file (L89); tempo changes are recorded; moves made by signals and by playing actions are recorded and shown in their own colours (L71, L66).
8. In and Out: 1 beat is the smallest (L82). Quantize asks for its own spacing, 4 bars down to 1/4 beat (L83): the two lists differ (see CONFLICTS).
9. New: a very low-resolution recording of the show in 10 or 20 minute pieces as a double check (L6); filed as G4, its cost is a researcher's.
10. His question on the sentence about "one action from its start to its end" (L87): the sentence is withdrawn; 186 b makes it unnecessary (see QUESTIONS BACK).
11. Seven assumptions need his word (ASK: YES); the rest are lines he can strike.

## ITEMS
@@ITEM 186
TITLE: The outputs while you review
STATUS: ANSWERED b
HIS: L91, L79
RULE: While the review screen is open, every connected output shows the review picture too: what plays, what a click or a scrub lands on, and what a Record Over shows. Each output shows it with its own screen settings, like any other picture it shows. The recording's sound is heard while it plays. When the review screen is closed the outputs show the live show again (with nothing playing, as R165 says). His line L79 adds that the picture can be given to a monitor so that the tracks get the room on the main screen. Whether there is a switch for it: assumption E-1.
CHANGED: The default A (outputs black) is replaced by B, his "186 b" (L91). The page's note under the question that "the recording's own picture and sound never reach the outputs" falls away with it.
TODAY: A replay drives the live layers and the live outputs; there is no separate review picture (FT section 1, S5; AR 1.10). A review renderer and its route to the outputs do not exist; both are to be built.
@@END
@@ITEM 187
TITLE: Scrub or draw inside a track
STATUS: DEFAULT A
HIS: L1, L92
RULE: Two tools, picked by two buttons or by holding a key. The pointer: a click in a track jumps the picture and the timeline to that moment, a drag scrubs both. The pencil: a drag draws a new shape over a track's curve; a piece can be selected and moved left or right, snapping to the grid. The pointer is on when the screen opens. Besides these two mouse tools there is Record Over, which changes tracks in real time from the MIDI controller (item 195); his L92 keeps the three apart: "This record over mode is different than the scrubbing and drawing with mouse as this plays in real time". Whatever the pencil changes is a mend and goes into the parallel recording (R222).
CHANGED: nothing in the default; L92 confirms that both scrubbing and drawing with the mouse exist.
TODAY: No track, timeline, scrub or curve editing exists anywhere in the app (FT section 1, the Record tab line and the line after it; AR 1.3).
@@END
@@ITEM 188
TITLE: Which rows the review screen shows
STATUS: REPLACED by L81
HIS: L81
RULE: The review screen has a track only for a button or slider that changed during the recording. Such a track runs from the beginning of the recording to its end and is a flat line at the value the control had until the moment it was first used. A button or slider that was not changed during the show and was at its default the entire time gets no track. Clips, layers and global that had no changed control have no tracks at all. A control that never moved but stood away from its default: assumption E-13. The tracks are folded under the names of their clip, layer and effect, or under global.
CHANGED: The default A ("Every control of those has a row, moved or not") and B (every control of the whole show) are both replaced by his L81: "On any button or slider that was not changed during the show and was at default the entire time, does not get a row in the recording."
TODAY: A recording keeps ten kinds of event and the values that arrive through keys, pads, MIDI, OSC and REST; a slider moved with the mouse, the layer strip's B and S, and a preset load are not kept (AR 1.8; FT section 2). No rows exist. The recorder has to reach every control (AR T4).
@@END
@@ITEM 195
TITLE: Record Over with the MIDI controller, kept separately
STATUS: ANSWERED in his words (Record Over stays; nearest to A)
HIS: L92, L84
RULE: Record Over stays. It is done with the MIDI controller and it plays in real time: the recording plays with its sound and its recorded moves, the picture shows what the moves do, and what he moves on the controller while it plays is recorded over the moves of those same controls. It is a third way of changing a recording, apart from scrubbing and from drawing with the mouse. A Record Over never changes the original: it is recorded separately from the original and kept beside it, with the same sound. The outputs show the picture while it runs (186 b). Where it runs and how long a touched control stays his: assumption E-2. Which controls can record over: E-3. Whether the separate recording is complete or only the changes: E-4. While a Record Show or a clip recording runs, Record Over is not available (R223 c).
CHANGED: No letter was given. A's "reached from the list of recordings ... play in the live window" is not confirmed by his words and is replaced by the reading that Record Over is a mode of the review screen (E-2). A's "kept as a new recording beside the first, with the same sound" is confirmed by "Record overs are recorded separately from the original" (L92). B (only the sound plays) and C (Record Over goes) are not taken: "yes we can record over" (L92), and his earlier "The knob will supersede whatever is happening, And will snap back to the recorded track as soon as it is let go." (BD:340-341).
TODAY: "Record Over" shows only while a recording replays with its sound, records a new take onto the same sound from the current position, and runs through the live layers (AR 1.1, H8, O2). It takes keys, pads, MIDI, OSC and REST values, not the mouse (AR 1.8). Its only door, "Play Take", goes.
@@END
@@ITEM 216
TITLE: The fader in a one-layer recording
STATUS: REPLACED by L88 (the question falls away)
HIS: L88
RULE: There is no one-layer recording, so the question does not arise. A clip recording holds the full composition, and every layer is in it at the strength its fader has at each moment: a fade performed while recording is in the clip.
CHANGED: Default A and option B both fall away: "I don't think there's any reason for a one layer recording." (L88).
TODAY: Record to clip does not exist in the app (AR 1.18). The old plan had a Whole / Layer switch (AR O13, H18): the Layer half is not to be built.
@@END
@@ITEM R138
TITLE: The review screen: stop, jump, scrub, and where its picture goes
STATUS: CORRECTED (added to; the sound sentence replaced)
HIS: L79, L86, L91
RULE: The review screen is a screen of its own, not the live window: the picture at the top, one track per changed slider or button below it. While it plays, the stop button, the spacebar, or a mouse press inside a track stops it and changes nothing else. A click in a track moves the picture and the timeline to that moment; a drag scrubs both together. It does not start playing again by itself: the spacebar or play does that, from where the playhead stands. The tempo row's buttons are not on this screen. New from L79: the user can give the recording's picture to a monitor so that the tracks have the most room on the main screen, or drag the line between the tracks and the picture to make the picture as small or as large as he wants on one monitor. Sound: it plays while the recording plays in real time; whether it is also heard during a scrub is a setting (R183, L86).
CHANGED: Added: the picture on a monitor and the draggable line between tracks and picture (L79). Replaced: "the recording's sound plays while it plays and is silent while you scrub" becomes a setting (L86). "one track per slider or button" now means per changed control (L81). The rest stands.
TODAY: None of it exists (AR 1.3, 1.10; FT section 1). No space-key handler exists in the app (AR correction 5).
@@END
@@ITEM R165
TITLE: Opening the review screen stops the live show
STATUS: STANDS
HIS: L72, L35, L91
RULE: The review screen is for after the show. Opening it is like a press on stop for the live show: the clips leave the layers and the tempo stops; the app asks first if clips are playing. Closing it brings back the live window with the show as it was and nothing playing. Reviewing, mending or recording over a recording never changes a slider, a button or a clip of the show; the only thing that goes from the review screen into the show is an action he saves there. While it is open the outputs show the review picture (186 b).
CHANGED: nothing; its pointer to question 186 is now answered b. His words fit it: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72) and "live and recording review mode" (L35).
TODAY: A replay runs through the live layers, so review and live cannot run side by side (FT section 1, S5). Showing a recorded moment without touching the live show is not built and its cost is unmeasured (AR T3).
@@END
@@ITEM R173
TITLE: A show recording saves its own show file
STATUS: CORRECTED
HIS: L80, L89
RULE: Every show recording saves with it a show file that holds the clips exactly as they were for that show: the recorded show file. The review screen opens a recording from its recorded show file, so the recording opens correctly whatever has been done to the show since, and also when the show of that name has been changed. When the show has changed since, the app tells the user which clips have been moved or are missing. What is removed during a recording is kept in the recorded show file (R223 d). What the recorded show file holds of the media: assumption E-12. An action saved in the review screen goes into the show (his One Save: it is written to the show file at the next Save, and the quit window asks about it like any other change); onto which clip when the show has changed: assumption E-11.
CHANGED: Replaced: "it is reviewed with that show open (if another show is open, the app says which one to open)" and "The review uses the show as it is now" -- by L80: "the recording also saves a show file with the clips exactly as they are for the show" and "the recording still opens it up correctly and indicates this to the user that these clips have been moved or missing". Replaced: "a row of a clip or an effect that you have since removed is marked missing and cannot be ticked" -- its track is there and plays in the review; what saving does for it is E-11. The last sentence (the action goes into that show, One Save) stands.
TODAY: A recording names no show and carries none; a replay is matched against whatever show is open (FT section 1, S6; AR 1.5, H24). The recording's file needs a new shape (AR T6) and the verified writer (AR 1.22).
@@END
@@ITEM R137
TITLE: Only what changed gets a track
STATUS: CORRECTED (a and c replaced)
HIS: L81
RULE: (a) Nothing that does not move is recorded as a track. A button or slider that was changed at any moment of the recording gets a track from the beginning of the recording to its end; before its first use the track is a flat line at the value it had. A button or slider that was not changed during the show and was at its default the entire time gets no track. (b) A preset loaded during the recording shows as a jump at one moment on the track of each slider it changed; before the jump each track shows the value it had. (c) falls away: a control that never moved has no track (the case of one standing away from its default: assumption E-13). (d) An action made from there stores the numbers and never the preset's name: changing the preset later does not change the action. (e) Selecting the name of a clip, a layer or an effect selects all its tracks.
CHANGED: (a) "a row for every slider and every button that can be adjusted, moved or not" and (c) "the row of a control that never moved is a flat line at its value" are replaced by L81: "we are not recording anything that does not move." (b), (d), (e) stand. His own earlier sentence, "there is a row in the recording for every parameter and button that can be adjusted" (BD:1089), is replaced by his newer one.
TODAY: See item 188: no rows; the recorder reaches only part of the controls (AR 1.8, T4; FT section 2).
@@END
@@ITEM R135
TITLE: In and Out sit on grid lines; 1 beat is the smallest
STATUS: CORRECTED (the list shortened)
HIS: L82, L83
RULE: In and Out can sit only on grid lines. The grid's size can be changed; the snap cannot be switched off. The grid sizes for In and Out are 4 bars, 2 bars, 1 bar, 2 beats and 1 beat; 1 beat is the smallest. Quantize has its own spacing list (R166), which goes finer: assumption E-14.
CHANGED: "half a beat and a quarter beat" are removed from the list: "1beat is the smallest" (L82). The rest of the list stands as shown ("half a bar" is written here as 2 beats, his word in L83).
TODAY: No grid exists; an action's forerunner is cut by typed bar numbers (AR 1.2).
@@END
@@ITEM R166
TITLE: Quantize asks for its grid spacing
STATUS: CORRECTED
HIS: L83, L89, L84
RULE: Quantize is a control of the review screen. When a Quantize is done the user selects its grid spacing: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat or 1/4 beat. It moves button presses and clip fires onto the nearest line of that spacing; it does not reshape a slider's curve. What it works on: assumption E-15. A Quantize is a mend: it goes into the parallel recording and never into the original (R222). A Quantize is an Undo step.
CHANGED: "In and Out use the same grid" is replaced: Quantize has a spacing setting of its own with his list (L83). The rest stands.
TODAY: Quantize is a live setting in the top bar (Off / Next Beat / Next Downbeat; FT SURPRISES S2); its removal there is planned and not built (AR part 2, R-8). No review Quantize exists.
@@END
@@ITEM R222
TITLE: A mend is another recording, parallel to the original
STATUS: CORRECTED
HIS: L84, L92
RULE: A mend (a shape drawn over a curve, a piece moved, a Quantize) is never written into the original recording. The mend is another recording that is saved, parallel to the original: it stands beside the original in the list of recordings and shares its sound and its recorded show file. The original stays exactly as it was recorded and can always be opened again. The mended recording is kept when the review screen is closed. An action saved earlier from a recording does not change when that recording is mended afterwards. How many parallel recordings mending makes: assumption E-4.
CHANGED: Replaced: "a mend ... changes the recording", "The night as it was recorded is always kept underneath" and the button "Back to as recorded" -- by L84: "the mend is another recording that is saved, parallel to the original." (he took the page's own closing offer). The sentence about earlier saved actions stands.
TODAY: No mending exists. Several recordings can already share one stored sound (AR 1.6; BD:452-458).
@@END
@@ITEM R167
TITLE: An action never records a deck switch or the tempo
STATUS: STANDS
HIS: L85, L89
RULE: An action never switches decks and never touches the tempo: the night's tempo changes and deck switches cannot be selected into an action. The night's tempo is recorded and shown in the review screen, and its bars are the grid. Whether deck switches are shown at all: assumption E-16.
CHANGED: nothing; his "R167 action should not record switching decks" (L85) says the same for decks, and "Actions will not be made with a tempo change." (L89) for the tempo.
TODAY: A recording keeps deck switches and tempo commands as events (AR 1.8; FT section 1).
@@END
@@ITEM R183
TITLE: The three boxes; how a track looks; sound while scrubbing
STATUS: CORRECTED (added to; a to g stand)
HIS: L86, L81, L114
RULE: The three boxes of Record Show stand as shown: (a) Parameters: the moves of the controls; it gives the review screen its tracks and its picture, which is made fresh from the moves and the recorded show file. (b) Audio: the night's sound; it gives the review screen its sound and its audio track. (c) Video: a film file of the composition to keep or share; the review screen does not use it. (d) The boxes sit beside the Record Show button; in a new app Parameters and Audio are ticked and Video is not; the app remembers the ticks; with no box ticked the button is greyed. (e) With Audio and Video both ticked the film has the night's sound; a Video-only recording is a film file and is not in the list of recordings. (f) The Output menu's own film recording goes: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone." (L114). New from L86: each recorded track, button or slider, is shown in a small row that can be dragged taller, with its keyframes and slider positions. A button's track is a horizontal line that can only be in two positions, 0 and 100, unless the control has more than two positions; then its line has one level for each position. A setting decides whether the sound is heard while scrubbing; with it off the sound plays only when the recording plays in real time. How the setting starts: assumption E-19. (g) A slider that a signal was moving shows what it did that night, because its moves are recorded as values (D18).
CHANGED: Added: the track's look and the draggable row height, the two-position button line, the scrub sound setting (L86). (a) "its rows" now means tracks of changed controls only (L81). (g) "I cannot promise what a slider that the music was moving shows" is replaced by D18's rule (L71). (a) to (f) stand.
TODAY: One tick "Record audio", on at every launch and not remembered; Parameters always on; the film is a separate Output menu command with no sound (AR 1.2, 1.13, H9, H10). The film may play at the wrong speed (AR 1.14, T2: inferred, not run).
@@END
@@ITEM R184
TITLE: The list of recordings and the Record tab
STATUS: CORRECTED (one sentence of d withdrawn; names; Render planned)
HIS: L87, L91, L114, L128, L92
RULE: (a) The place where a recording is watched, mended, recorded over and cut into actions is one screen, the review screen. (b) Recordings are found in a list: name, date, length, the show each was made in. A double-click opens one in the review screen; a recording can be renamed and shown in Finder. Mended and recorded-over recordings stand in the list beside their original (R222, 195). (c) The app never deletes a recording; it is removed in Finder. (d) There is no separate button that plays a recording into the live show. To show a whole recording, it is opened in the review screen and played: the outputs show it (186 b). The sentence "To play a whole recording as a performance you make one action from its start to its end." is withdrawn (see the answer to his question; assumption E-20). Record Over is in the review screen (195). Render, making a film from a recording afterwards, is planned: "215 plan all of these" (L128). (e) A recording's sound that was damaged by a crash is repaired by itself when the list opens. (f) Beside the list there are: the Record Show button, which reads "Stop Recording" while a recording runs; the box for the recording's name; the three boxes of R183. The word "take" is not on screen anywhere.
CHANGED: (d) the sentence he asks about in L87 is withdrawn, because his 186 b gives a direct way to show a whole recording. (d) "Whether you still want Render: question 215 (f)" is answered: planned (L128). (f) the button's name follows L114 ("record show"). (d) "Record Over: question 195" is answered by L92. The rest stands.
TODAY: Record tab: "Record Take" / "Stop Recording" / "Record Over", "Play Take" / "Stop Playback", "Load Take...", "Show in Finder", "Repair Audio", "Record audio", "Play with audio", the name box, "Take (timelines and audio)", "Render... (coming)", "Takes: <folder>", "Save Routine" (AR 1.1, 1.2). No list of recordings (AR 1.3, H5). No key or pad starts a recording (AR 1.4).
@@END
@@ITEM R185
TITLE: The review screen: keys, Undo and the tempo row
STATUS: REPLACED in part (a: the MIDI controller is live during Record Over)
HIS: L92, L89, L72
RULE: (a) While the review screen is open the keyboard is its own: the spacebar stops and plays; Cmd+Z undoes the last change made there (a mended curve, a Quantize, an In or an Out). The keys and pads of the keyboard and MIDI mapping do not fire anything into the live show, which is stopped. During a Record Over the MIDI controller works on the recording that is playing: what is moved on it is heard and seen in the review picture and is recorded over (195). (b) A show recording keeps the presses on the tempo row (play, pause, stop, Tap, Resync, the nudge, a tempo change), so the review picture goes empty where stop was pressed that night; they are shown and cannot be selected into an action. (c) The grid follows the night's tempo as it changed: "When tempo changes, record that as it will happen often." (L89).
CHANGED: (a) "the keys and pads of the keyboard and MIDI mapping fire nothing" is replaced for Record Over by L92: "yes we can record over using the midi controller." (b) and (c) stand; (c) is now his word (L89).
TODAY: No space-key handler; Cmd+Z undoes the last fire app-wide (AR correction 5, M8). Tempo commands are recorded; a tempo-row stop does not exist yet (AR 1.8, H26; FT SURPRISES S4).
@@END
@@ITEM R186
TITLE: Record to Clip: always in time, always the full composition
STATUS: CORRECTED (d and e replaced; c sharpened)
HIS: L88, L114
RULE: (a) A clip recording has no time limit except the disk; while it records its picture is seen in the cell it lands in. See-through parts: assumption E-9. (b) It lands in the next empty cell from the left on the top layer of the deck on screen, without that cell being selected. (c) It is always recorded in time: it starts on the "1" (the next bar line after the REC press) and it ends on a bar line: after the stop press it keeps recording until the end of the bar. The clip that lands is a BPM-mode clip with its beats and tempo set. It can be trimmed later with its in and out points. Its out point is moved in to the last full group of 4 bars and the rest of the file is kept beyond it: assumption E-6 holds both readings of "a even amount". (d) Nothing is recorded while the tempo is paused: a clip recording never starts or runs on a standing picture, and there is no clip recording that is out of time. What the REC press does while the tempo is paused or stopped: assumption E-5. What a pause or stop does to a recording that runs: E-7. (e) There is no one-layer recording and no choice between whole and layer. A clip recording is the output of the layers, the full composition, and never the picture of one output screen, because a screen can be altered to fit a projector's colour and timing. What the full composition includes: assumption E-8. (f) A recorded clip has no sound. (g) A clip recording is never shorter than 4 bars: a stop press before the end of bar 4 ends it on the bar line that closes bar 4. (h) When it lands it waits in its cell, ready to be fired; it does not start playing by itself. (i) It lands in the deck that is on screen at the REC press; if the top layer has no empty cell a new column is added to that deck.
CHANGED: (d) "With the beat stopped or paused, REC starts and ends on your press and the clip is not in BPM mode" is replaced by L88: "it is impossible to record something paused. Anything is recorded it is recorded in time on the one and ending on a even amount. If we push stop recording, keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (e) the one-layer recording, its selected layer and its blend sentence are removed by L88: "I don't think there's any reason for a one layer recording. Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues. It must be the full screen composition." (c) the end is now his words (end of the bar). (a), (b), (f), (g), (h), (i) stand; his earlier "whole ouput of select layer" (BD:574) and the "yes" to a single-layer recording's see-through parts (BD:671) are overtaken for the layer half.
TODAY: Record to clip does not exist in the app; only an old plan does, with a 64-bar automatic end, black for see-through parts, a text-only display and a Whole / Layer switch, all overtaken (AR 1.18, O3-O6, O13, H12-H18, H31). A tempo pause / stop does not exist yet (FT SURPRISES S4).
@@END
@@ITEM R187
TITLE: Recordings: quitting, opening a show, the disk, which picture
STATUS: STANDS (d is now his own words)
HIS: L88
RULE: (a) Quitting while a recording runs stops it and keeps it, with no extra window. (b) Opening another show or pressing New while a recording runs: the app asks once, in one window, saying that the recording will be stopped and kept, with the same three choices as at quit; then the show opens. (c) Record does not start when the disk is nearly full and shows "not enough HDD space to record"; if the disk runs low during a recording the recording ends by itself and keeps what is written. The amounts of free space are measured before it is built. Whether the app says that a recording was ended by the disk belongs to question 209 (another topic). (d) Recordings, films, the low-resolution recording and snapshots show the composition and never one screen's Delay, Brightness, Contrast or colour.
CHANGED: nothing. (d) is confirmed by L88: "Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues."
TODAY: The sound's disk floor (2 GB) is checked at the start only; the film has no disk check; nothing stops a recording when another show is opened (AR 1.7, 1.13, H20, M4, M5, T5).
@@END
@@ITEM R223
TITLE: The review screen and recordings: the edges
STATUS: CORRECTED (a added to; d and e replaced)
HIS: L89, L80
RULE: (a) When the review screen opens nothing is selected and Save is greyed until something is. The tracks are greyed out and the selected area is white. The selected area is the selected tracks, between the In and Out points on the timeline; selecting a name selects all its tracks. (b) When playback reaches the end of the recording it stops on the last moment and stays there; play starts again from the beginning; it never carries on into the live show. (c) While a recording runs, the way into the review screen and Record Over are greyed. (d) When an effect is removed during a recording, the removal is recorded and the effect is kept in the recorded show file; from the moment of its removal all its settings are at their defaults and its tracks show that. So the review shows the effect working up to the removal and gone after it. What is added during a recording: assumption E-17. (e) Tempo changes are recorded, as they happen often, and are shown. An action is not made with a tempo change: it never holds the tempo and never changes it. An action cut from a stretch in which the tempo moved: assumption E-18. A recording always has a tempo (D21), so its grid always shows bars.
CHANGED: (a) added: the greyed tracks and the white selected area, and what the selected area is (L89). (d) "an effect you remove while recording keeps its rows, which move nothing once it is gone" and its closing question are replaced by L89: "If it is removed, record the removal and keep it in the recorded show file, but default all its settings once removed." INFERRED: in "Very little chance a recording will be removed" the word "recording" is read as the effect being recorded, because (d) is about effects (E-17). (e) "takes the tempo at its In point" is replaced by L89: "When tempo changes, record that as it will happen often. Actions will not be made with a tempo change."; the sentence on 175 B falls away (175 takes its default A, L1). (b), (c) stand.
TODAY: A replay holds the last look in the live window until "Stop Playback" (AR 1.11). Adding or removing an effect is not a recorded event (AR 1.8). A recording started before a tempo is known cannot be cut (AR 1.9, H25).
@@END
@@ITEM R208
TITLE: The units a track shows
STATUS: STANDS
HIS: L86
RULE: A track of the review screen shows the number that its slider shows: Position, Scale, Rotation and Anchor read 0 to 1 with 0.50 in the middle, as their sliders do, so that a track and its control always agree. A button's track runs between 0 and 100, the bottom and the top of its row (L86).
CHANGED: nothing; he did not ask for degrees or radians. L86 gives the button track its 0 and 100.
TODAY: The sliders read 0 to 1 with two decimals; rotation is kept in degrees underneath (as cited on the page: src/ui/UniversalParamControl.cpp:198, src/ui/CompositionInspector.cpp:163, 169; not re-read).
@@END
@@ITEM D16
TITLE: The review picture is made fresh from the recording
STATUS: SETTLED (L80 for the show it is made with; the rest by R183 a, read and not corrected)
HIS: L80, L84, L92, L6
RULE: The picture in the review screen is made fresh each time from the recorded moves and the recorded show file; it is not a film of the night. So a mend or a Record Over changes the picture that is seen. The clips' files must still be on the disk; a clip whose file has moved or is missing is named as such when the recording opens. The low-resolution recording (G4) is the film to check this picture against.
CHANGED: "with the show it was recorded in" becomes the recorded show file (L80). The remark about a video's picture lagging under a fast scrub is Harmony's own technical note (assumption E-28).
TODAY: A recording stores no picture; the only renderer is the live one, and it can only play a recording from its start (FT section 1, S5; AR T3).
@@END
@@ITEM D17
TITLE: What a recording is able to keep
STATUS: REPLACED in part by L81
HIS: L81, L86, L76, L80
RULE: A recording can keep any button, slider, drop-down list or play / direction button of the clip, layer and global tabs and of the layer strip, however it is moved, the mouse included; but it keeps a track only for those that changed during the night (L81). Where every control stood at the start is in the recorded show file (L80), so the review picture is right from its first moment.
CHANGED: "will keep every slider and button ... from its first moment" is replaced: "we are not recording anything that does not move." (L81). That any kind of control can be recorded is his: "Each track recorded, button or slider" (L86) and "the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes" (L76).
TODAY: More than half of a clip's controls and all mouse moves cannot be recorded (AR 1.8, T4; FT section 2). This is the largest piece of the build.
@@END
@@ITEM D18
TITLE: Moves by hand, by a signal, by an action: all recorded
STATUS: SETTLED
HIS: L71, L66
RULE: A recording keeps what a control did whoever moved it. Moves recorded off signals are shown in the review screen in a different colour, so that the user sees it before creating actions; an action can be made from them and is treated like a regular action. When an action was playing during the recording, its moving parameters are shown as tracks in a slightly different colour and the action's name is shown in the hierarchy; a new action made from them holds the moving parameters themselves and never an action inside an action. So there are three colours of moves: by hand, by a signal, by an action. The moves a signal made are kept as values (assumption E-24).
CHANGED: "with a mark that a signal drove it" becomes his different colour (L71: "we need to have a different color for the actions that were recorded off of signals"); the moves of a playing action get their own colour and the action's name (L66: "those parameters will be slightly different colored and the name of the action shown somehow in the hierarchy").
TODAY: Nothing that a fired routine or a signal does to a control is recorded (AR M1, 1.8).
@@END
@@ITEM D19
TITLE: The sound of a recording when the music is a file
STATUS: OPEN (Harmony's own; assumption E-25)
HIS: none
RULE: A show recording with Audio ticked keeps the sound the app heard, whatever its source: the microphone, the sound card or an audio file played in the app.
CHANGED: nothing
TODAY: Recording arms on the file source or the live input (AR M6); not checked by a run.
@@END
@@ITEM D20
TITLE: Several recordings at the same time
STATUS: OPEN (Harmony's own; assumption E-26)
HIS: L6
RULE: Record Show with any of its boxes, the low-resolution recording and a clip recording can run at the same time. If the computer cannot keep up, the smoothness of the output comes first and a recording loses frames. It is measured before it is built.
CHANGED: The low-resolution recording of L6 joins the list of things that can run together.
TODAY: No measurement exists; each film reads the composition's picture back from the graphics card (AR H29, T2).
@@END
@@ITEM D21
TITLE: A recording always has a tempo
STATUS: SETTLED by the default of question 175 (L1); the topic-A paper owns 175
HIS: L1, L89
RULE: The tempo number is never empty (175 A), so a show recording always has a tempo: its grid always shows bars and any part of it can become an action.
CHANGED: The condition "If you answer 175 A" is met by his L1 ("All defaults good except for these"); 175 is not named by him.
TODAY: A recording started while no tempo is known cannot be cut (AR 1.9, H25).
@@END
@@ITEM D22
TITLE: The file types of a film and of a recorded clip
STATUS: OPEN (Harmony's own; assumption E-27; waits for the answer to his codec question L3)
HIS: L3, L6
RULE: The app picks the file types itself: a film and the pieces of the low-resolution recording open in ordinary players; a recorded clip is in the type that plays back most lightly in the app. A film plays at the right speed whatever the output's frame rate. The choice is made after his question of L3 (a codec of our own) is answered, which is another topic's.
CHANGED: The see-through half ("a recorded clip keeps its see-through parts") now hangs on assumption E-9, since the one-layer recording it was ruled for is gone (L88).
TODAY: The film is H.264 only, without sound, and probably plays in slow motion (AR 1.13, 1.14, 1.17, T1, T2: inferred, not run).
@@END
@@ITEM P17
TITLE: The review screen's layout
STATUS: DROPPED
HIS: L8, L79
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His function for it is in R138: the line between picture and tracks can be dragged, and the picture can go to a monitor (L79).
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P18
TITLE: The review screen's left side: names, hierarchy, folding
STATUS: DROPPED
HIS: L8, L81, L66
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Only changed controls have tracks (L81), under clip, layer and global; an action that played is named in the hierarchy (L66).
CHANGED: The variants' "mark on the ones that moved" is no longer needed: everything listed moved.
TODAY: not checked
@@END
@@ITEM P19
TITLE: The review screen's tools
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The tools themselves: pointer and pencil (187), In and Out and the grid size (R135), Quantize with its spacing (R166), Record Over (195), the scrub sound setting (R183).
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P20
TITLE: Picking tracks and saving
STATUS: DROPPED (the look is his: L89)
HIS: L8, L89
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). His words give the look meanwhile: the tracks are greyed out and the selected area is white (R223 a).
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P21
TITLE: How the review screen is opened and closed
STATUS: DROPPED
HIS: L8, L35
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). It is built as one of the main window's configurations, beside live: "We will have various configurations other than live and recording review mode" and "no need to undock and move just yet" (L35). Opening it stops the live show either way (R165).
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P22
TITLE: The window that saves an action
STATUS: DROPPED (what it does is the topic-D paper's)
HIS: L8, L55, L65
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What the window does (the app breaks a mixed selection into clean actions, L55; a warning when an action is not a multiple of 4 bars, L65) is ruled in the actions topic.
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P23
TITLE: The REC button and the sign that a recording runs
STATUS: DROPPED (its Whole / Layer choice is removed by L88)
HIS: L8, L88
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). REC has no Whole / Layer choice (L88). A running recording is prominently displayed, his earlier word (BD:625).
CHANGED: The Whole / Layer choice is removed: "I don't think there's any reason for a one layer recording." (L88).
TODAY: not checked
@@END
@@ITEM P24
TITLE: The list of recordings and the three boxes
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8).
CHANGED: nothing
TODAY: not checked
@@END
@@ITEM P25
TITLE: What the audio track shows
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Meanwhile it is the waveform across the top with the common grid of the night's bars over it (assumption E-31).
CHANGED: nothing
TODAY: A recording can carry marks for drum hits, but no control sets it (AR M7).
@@END
@@ITEM G4
TITLE: A very low-resolution show recording in pieces
STATUS: ANSWERED (the function; its cost is a researcher's)
HIS: L6, L88
RULE: While a show is recorded the app can also write a very low-resolution film of the show as a double check. It shows the full composition (never one screen's corrected picture, L88), at about a quarter or an eighth of the size. It is written in pieces of 10 or 20 minutes: each piece is a small finished file by itself, so that if something happens most of the recording is not lost; only the piece that was being written can be hurt. The pieces follow each other without a gap and are numbered in order. They are kept with the show recording they belong to. It is built only if the researcher's answer shows that it does not cost the output its smoothness; the output comes first. How it is switched on and how it stands to the Video box: assumption E-21. Piece length and size: E-22. The rest of its edges: E-23.
CHANGED: New; it answers no item of the page. His words: "I think it might be good to record a very low resolution show recording if that is possible and to record it in 10 or 20 minute chunks so that they are small and if something happens most of the recording is not lost. Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check. How much comp resource would this take up?" (L6). The last sentence is his question; a researcher answers it.
TODAY: The only film recording writes one full-size file whose index is written at the end, so a crash leaves it unplayable; no pieces, no small size, no sound (AR 1.13, 1.14, T2: the crash part inferred, not run).
@@END

## ASSUMPTIONS
@@ASSUME E-1
ABOUT: 186, R138
TEXT: I assume that while the review screen is open every connected output shows the review picture, with no switch for it, and that the outputs go back to the live show when you close it.
WHY: L91 "186 b" reads as always; L79 "use can chose to output the recording to monitor" reads as a choice.
ALT: b) A switch in the review screen turns the picture on the outputs on and off. c) You pick the one monitor that shows it; the other outputs stay black.
IF-WRONG: STAGE a projector that is still connected shows your scrubbing and mending.
ASK: YES two lines of his pull apart and the audience could see the result.
@@END
@@ASSUME E-2
ABOUT: 195, R185, R184
TEXT: I assume Record Over is a button in the review screen: the recording plays in real time, and a control you move on the MIDI controller replaces its recorded moves from your first touch until you stop moving it.
WHY: L92 says it plays in real time, not where it runs or how long a touch lasts; his older words: "snap back to the recorded track as soon as it is let go" (BD:341).
ALT: b) Once touched, a control stays yours until you stop Record Over. c) Record Over runs in the live window with the live show, not in the review screen.
IF-WRONG: REBUILD it decides whether the review screen takes the MIDI controller at all.
ASK: YES no word of his places it, and it shapes the review screen's build.
@@END
@@ASSUME E-3
ABOUT: 195
TEXT: I assume Record Over takes everything in the keyboard and MIDI mapping: knobs, faders, pads and keys. A clip you fire is added to the fires already recorded. The mouse does not record over.
WHY: L92 names only "the midi controller" and sets the mouse apart; it does not say whether pads and keys that fire clips count.
ALT: b) Only knobs and faders record over; clip fires cannot be added. c) Sliders moved on screen with the mouse record over too.
IF-WRONG: SMALL one more kind of control to let in or keep out.
ASK: LINE a choice he will most likely wave through.
@@END
@@ASSUME E-4
ABOUT: 195, R222
TEXT: I assume a mend and a Record Over are each kept as a complete recording beside the original, sharing its sound. All mends of one recording go into one mended copy; each Record Over makes a new one.
WHY: L84 "parallel to the original" and L92 "recorded separately" do not say whole copy or changes only, nor how many copies.
ALT: b) Every time you mend and close, one more copy is made. c) Only the changes are kept, as a layer over the original that you can switch off.
IF-WRONG: SMALL the list of recordings would be grouped differently.
ASK: LINE a choice of mine on how the list fills up.
@@END
@@ASSUME E-5
ABOUT: R186
TEXT: I assume that with the tempo paused or stopped a press on REC arms it: the clip recording starts on the first "1" after the tempo plays again.
WHY: L88 "it is impossible to record something paused" does not say what the REC press does in that state.
ALT: b) REC is greyed and does nothing until the tempo plays. c) From stop, the REC press itself starts the tempo and is the new "1", as a clip click is.
IF-WRONG: STAGE you press REC before a song and get no recording, or one you did not expect.
ASK: YES he would meet it on stage and his words leave it open.
@@END
@@ASSUME E-6
ABOUT: R186
TEXT: I assume a clip recording ends at the end of the bar in which you press stop, and its out point is then set at the last full group of 4 bars; the bars beyond it stay in the file.
WHY: L88 says both "ending on a even amount" and "an even grid line like the end of the bar"; his older rule is "always work with multiples of 4" (BD:812).
ALT: b) It keeps recording until a group of 4 bars is full, so nothing is cut off. c) It ends at the bar line and the clip keeps every bar: 5 bars stay 5.
IF-WRONG: STAGE the clip loops shorter or longer than he meant, or records on for up to 3 bars.
ASK: YES "even amount" can be read two ways and it is the heart of his rule.
@@END
@@ASSUME E-7
ABOUT: R186
TEXT: I assume that pausing or stopping the tempo while a clip records ends the recording there, with its end set back to the last full group of 4 bars; with less than 4 bars recorded nothing is kept.
WHY: L88 rules out recording a paused picture but does not say what happens to a recording that is running.
ALT: b) The recording holds while the tempo is paused and carries on, in time, when it plays again.
IF-WRONG: SMALL one rule at the pause press.
ASK: LINE he can strike it if he wants the recording to hold.
@@END
@@ASSUME E-8
ABOUT: R186, 216, G4
TEXT: I assume a recording holds the whole composition as the output monitor shows it: all layers, the global effects and the master opacity; never one screen's own colour or delay.
WHY: L88 says "the output of the layers" and also "the full screen composition"; global effects are not named.
ALT: b) The layers mixed together, without the global effects and the master opacity, so that a recorded clip played back is not given the global effects twice.
IF-WRONG: STAGE a recorded clip would look different from what he saw while recording.
ASK: LINE his older words ("The recording should look exactly like the output", BD:626) lean this way.
@@END
@@ASSUME E-9
ABOUT: R186, D22
TEXT: I assume a recorded clip keeps the empty parts of the composition see-through, so the layers under it show through when it plays.
WHY: His "yes" to see-through parts (BD:671) was for a one-layer recording, which L88 removes; a full composition shows black where nothing is.
ALT: b) Empty parts are black, exactly as on the output; the files are much smaller and play back more lightly.
IF-WRONG: REBUILD it decides the file type of every recorded clip.
ASK: YES his own change left it open, and the file type is costly to change later.
@@END
@@ASSUME E-10
ABOUT: R186, R184
TEXT: I assume starting on the "1" and ending on a bar line is your rule for a clip recording. Record Show starts and stops the moment you press, so that nothing of the night is missed.
WHY: L88 answers the clip reading but says "Anything is recorded it is recorded in time on the one".
ALT: b) Record Show also waits for the next "1" and ends on a bar line.
IF-WRONG: SMALL up to one bar at each end of a show recording.
ASK: LINE he can strike it in a second.
@@END
@@ASSUME E-11
ABOUT: R173
TEXT: I assume an action saved in the review screen goes into the recording's show as it is now, onto the same clip wherever it has moved. For a clip that is gone, the app says so and saves nothing.
WHY: L80 has the recording open from its own show file and name moved or missing clips, but not where a saved action lands then.
ALT: b) The action is saved anyway and waits, and you paste it onto the clip you choose. c) It goes into whatever show is open in the live window.
IF-WRONG: REBUILD where actions are stored and how a clip is recognised depend on it.
ASK: YES no word of his covers it and it is the purpose of the review screen.
@@END
@@ASSUME E-12
ABOUT: R173, D16
TEXT: I assume the show file that a recording saves holds every clip's settings and where its files are on the disk, not copies of the picture and video files themselves.
WHY: L80 "a show file with the clips exactly as they are", and "moved or missing", fit a file that points at the media.
ALT: b) The recording also copies the media files, so that it opens even when they are deleted; it would be very large.
IF-WRONG: SMALL copying media could be added later as a command.
ASK: LINE a choice he will most likely wave through.
@@END
@@ASSUME E-13
ABOUT: R137, 188
TEXT: I assume a control that never moved but stood away from its default the whole night also gets a track: a flat line at that value.
WHY: L81 says nothing that does not move is recorded, yet drops only what "was not changed ... and was at default the entire time".
ALT: b) No track: only controls that moved during the recording get one.
IF-WRONG: SMALL more or fewer tracks in the list.
ASK: LINE his sentence can be read both ways; the cost is small.
@@END
@@ASSUME E-14
ABOUT: R135, R166
TEXT: I assume "1 beat is the smallest" is for the In and Out points, and Quantize has its own list that goes down to a quarter beat.
WHY: L82 "1beat is the smallest" against L83's list for Quantize: "1 beat, 1/2 beat, 1/4 beat".
ALT: b) Nothing in the review screen goes finer than 1 beat, and the Quantize list ends at 1 beat. c) The grid for In and Out may also go down to a quarter beat.
IF-WRONG: SMALL a list of sizes.
ASK: YES two lines of his pull apart; ten seconds settle it.
@@END
@@ASSUME E-15
ABOUT: R166
TEXT: I assume Quantize works on the selected area: the button and clip-fire tracks you selected, between In and Out. It never reshapes a slider's curve.
WHY: L83 gives the spacing list, not what a Quantize covers; L89 defines the selected area.
ALT: b) It always works on every button and clip fire of the whole recording.
IF-WRONG: SMALL a mend can be undone.
ASK: LINE a choice of mine that follows his selected area.
@@END
@@ASSUME E-16
ABOUT: R167
TEXT: I assume the night's deck switches still show as marks in the review screen, and can never be put into an action.
WHY: L85 "action should not record switching decks" may be about actions only, or about the show recording too.
ALT: b) Deck switches are not recorded at all and the review screen shows none.
IF-WRONG: SMALL one track more or less.
ASK: LINE either way an action holds no deck switch.
@@END
@@ASSUME E-17
ABOUT: R223
TEXT: I assume "a recording will be removed" means an effect removed while recording, and that an effect or a clip you add while recording is recorded in the same way as a removal.
WHY: L89 says "Very little chance a recording will be removed. If it is removed, record the removal"; adding is not named.
ALT: b) Only removals are recorded; a thing added while recording gets no tracks. c) You meant something other than an effect by "a recording".
IF-WRONG: SMALL the review picture would differ from the night where something was added.
ASK: LINE a typing slip read kindly, plus its mirror case.
@@END
@@ASSUME E-18
ABOUT: R223, R167
TEXT: I assume "actions will not be made with a tempo change" means an action never holds or changes the tempo. You can still cut one from a stretch where the tempo moved; it keeps its bars and beats.
WHY: L89 can also mean that no action may be cut across a tempo change; in automatic mode the tempo moves a little all the time.
ALT: b) The app refuses to save an action whose In-to-Out stretch holds a tempo change.
IF-WRONG: SMALL a refusal could be added to the save window.
ASK: LINE the other reading would block most of a night recorded in automatic mode.
@@END
@@ASSUME E-19
ABOUT: R183, R138
TEXT: I assume the scrub sound setting starts switched off: you hear the recording only while it plays in real time, until you switch scrub sound on.
WHY: L86 asks for "a setting to not scrub sound" and does not say how it starts.
ALT: b) It starts switched on, and you switch it off when it bothers you.
IF-WRONG: SMALL one tick.
ASK: LINE he said it "won't sound good if it's scrubbed".
@@END
@@ASSUME E-20
ABOUT: R184
TEXT: I assume the way to show a whole recording to an audience is to open it in the review screen and press play. There is no other button that plays a recording.
WHY: L87 asks about the withdrawn sentence; L91 puts the review picture on the outputs; no word of his asks for a recording played inside the live show.
ALT: b) A "play recording" in the live window as well, so that live clips and moves can go on top of it.
IF-WRONG: REBUILD playing a recording inside the live show is a second engine.
ASK: LINE it follows from his "186 b"; he can strike it.
@@END
@@ASSUME E-21
ABOUT: G4, R183
TEXT: I assume the low-resolution recording is one more box beside Record Show, ticked when the app is new: a small film with the night's sound, apart from the full-size Video box.
WHY: L6 describes the film, not how it is switched on or how it stands to the Video box.
ALT: b) It runs by itself with every Record Show, with no box. c) It takes the place of the Video box. d) It runs whenever the app is open, recording or not.
IF-WRONG: SMALL a box more or less.
ASK: LINE a layout-and-default choice; the cost answer may change it anyway.
@@END
@@ASSUME E-22
ABOUT: G4
TEXT: I assume pieces of 10 minutes, each a film that plays by itself in any player, at a quarter of the width and height: 480 by 270 for a full HD show.
WHY: L6 offers "10 or 20 minute chunks" and "1/4 or 1/8 the size".
ALT: b) 20-minute pieces. c) An eighth of the width and height: 240 by 135. d) A quarter of the area: 960 by 540.
IF-WRONG: SMALL two numbers.
ASK: LINE two numbers he offered himself; the researcher's cost answer may move them.
@@END
@@ASSUME E-23
ABOUT: G4
TEXT: I assume the pieces are plain film files kept in the show recording's folder and looked at in Finder; the review screen does not play them. A crash can hurt only the last piece.
WHY: L6 says "to use as a double check" and not where it is watched.
ALT: b) The review screen can show the small film beside its own picture for comparing.
IF-WRONG: SMALL a viewer could be added later.
ASK: NO technical; nothing he would meet on stage.
@@END
@@ASSUME E-24
ABOUT: D18, R183
TEXT: I assume the moves that a signal makes on a slider are stored as values in the recording, so its track shows them even when the Audio box was not ticked.
WHY: L71 says moves are "recorded off of signals"; how they are kept is not his concern.
ALT: b) They are worked out again from the recording's sound each time it is reviewed.
IF-WRONG: SMALL internal.
ASK: NO technical.
@@END
@@ASSUME E-25
ABOUT: D19
TEXT: I assume a show recording keeps the sound the app heard, whether it came from the microphone, the sound card or an audio file played in the app.
WHY: No line of this message speaks of it; his older words on the stored sound lean this way (area sheet M6).
ALT: b) For an audio file only a pointer to the file is kept.
IF-WRONG: SMALL internal.
ASK: NO technical.
@@END
@@ASSUME E-26
ABOUT: D20, G4
TEXT: I assume Record Show, a clip recording and the low-resolution recording can all run together; if the computer cannot keep up, the output stays smooth and a recording loses frames instead.
WHY: L6 adds one more recording; no word of his says which gives way.
ALT: b) The app refuses to start a second recording while one is running.
IF-WRONG: STAGE only if the output stuttered; this choice is the one that protects it.
ASK: LINE the output first is his standing priority.
@@END
@@ASSUME E-27
ABOUT: D22
TEXT: I assume the app picks the file types itself: films that open in ordinary players, and for a recorded clip the type that plays back most lightly in the app.
WHY: His L3 asks whether to build a codec of our own; until that is answered the choice stays mine.
ALT: b) A codec of our own for recorded clips, as his L3 asks about.
IF-WRONG: REBUILD only for recorded clips already on disk.
ASK: LINE it waits for the answer to his own codec question.
@@END
@@ASSUME E-28
ABOUT: D16
TEXT: I assume that under a fast scrub a video clip's picture in the review screen may lag a moment behind the mouse.
WHY: A technical limit of drawing the picture fresh; no word of his.
ALT: none
IF-WRONG: SMALL
ASK: NO technical.
@@END
@@ASSUME E-29
ABOUT: R185, R138
TEXT: I assume the spacebar stops and plays in the review screen, whatever it is mapped to in the live window.
WHY: His 170 words give the spacebar to stop in the review (BD:1090); L118 says "Spacebar is typically tap tempo".
ALT: b) In the review screen too the spacebar does what the mapping says, and stop has another key.
IF-WRONG: SMALL one key.
ASK: LINE two screens, two jobs for one key.
@@END
@@ASSUME E-30
ABOUT: R187, R186
TEXT: I assume that quitting, or a nearly full disk, ends a clip recording at once without waiting for the bar line; its out point is set back to the last full group of 4 bars.
WHY: L88's "keep recording till it is on an even grid line" cannot hold when the app closes or the disk is full.
ALT: none
IF-WRONG: SMALL
ASK: NO technical edge.
@@END
@@ASSUME E-31
ABOUT: P25
TEXT: I assume the audio track shows the waveform with the night's bar lines over it, and no marks for single drum hits.
WHY: His reference words put "the audio track across the top" and a grid over all (BD:1062-1064); nothing more.
ALT: b) A mark at every drum hit the app heard.
IF-WRONG: SMALL
ASK: NO a look, settled in the UI redesign (L8).
@@END

## QUESTIONS BACK
@@ANSWER R184
HIS: L87, L91
ANSWER: Forget that sentence; it no longer holds. What I meant: the old button that replayed a whole recording goes away, so the only way left to show a whole recorded night to an audience would have been to mark it from its first moment to its last, save that as one giant action, and fire it. Your answer "186 b" makes that detour pointless: the outputs show the review picture. So to play a whole recording, you open it in the review screen and press play; the audience sees it, with its sound. Actions stay what you described: pieces of a recording, in groups of 4 bars, that you place on a clip, a layer or global.
@@END

## NAMES
@@NAME Record Show
MEANS: The recording of the whole show: the moves of the controls, the night's sound and, if ticked, a film; it is what the review screen opens.
SOURCE: his words L114 ("we will record to clip or record show. That’s what the 2 recordings are called. All else is gone."); the on-screen name now, "Record Take", is replaced by it
@@END
@@NAME Record to Clip
MEANS: The recording of the full composition's picture into a video file that lands as a clip in the next empty cell of the top layer.
SOURCE: his words L114; "REC" is the short label used on the page for its button
@@END
@@NAME show recording
MEANS: One recording made with Record Show; the word "take" is not used on screen or in talk.
SOURCE: his words L6 ("show recording") and L114 ("Show Recording Review"); the on-screen name now, "Take", is replaced by it
@@END
@@NAME recorded show file
MEANS: The show file that a show recording saves with it, holding the clips exactly as they were, from which the review screen opens the recording.
SOURCE: his words L89 ("keep it in the recorded show file") and L80
@@END
@@NAME Record Over
MEANS: Recording new moves from the MIDI controller over a show recording while it plays in real time; kept separately from the original.
SOURCE: his words L92 ("record over"), and earlier BD:499-500
@@END
@@NAME mend
MEANS: A change made to a show recording with the mouse (a drawn shape, a moved piece, a Quantize), saved as another recording parallel to the original.
SOURCE: his words L84 ("the mend is another recording"); the word first stood on Harmony's page
@@END
@@NAME track
MEANS: One row of the review screen: what one button or slider did over the length of the recording.
SOURCE: his words L86 ("Each track recorded, button or slider") and L89
@@END
@@NAME selected area
MEANS: The selected tracks between the In and Out points; it is white while the other tracks are greyed out, and it is what an action is saved from.
SOURCE: his words L89
@@END
@@NAME grid spacing
MEANS: The setting a Quantize asks for: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat or 1/4 beat.
SOURCE: his words L83
@@END
@@NAME scrub sound
MEANS: The setting that decides whether the recording's sound is heard while scrubbing, or only while it plays in real time.
SOURCE: Harmony's pick, from his words L86 ("a setting to not scrub sound")
@@END
@@NAME low-resolution show recording
MEANS: The very small film of the show written in pieces of 10 or 20 minutes as a double check.
SOURCE: his words L6 ("a very low resolution show recording"); a shorter name is the topic-J paper's to propose
@@END
@@NAME the review screen
MEANS: The one screen where a show recording is watched, mended, recorded over and cut into actions; a placeholder until the topic-J paper answers his question about its name.
SOURCE: his words L72 ("the recording review screen") and L114 ("Show Recording Review should be called Review for short"); the final name is the topic-J paper's
@@END

## CONFLICTS
- The smallest step. L82: "R135 1beat is the smallest" against L83: "the grid spacing setting: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat". Read as two lists, In and Out against Quantize: assumption E-14 (ASK: YES).
- The outputs during a review. L91: "186 b" (the outputs show the review picture, always) against L79: "use can chose to output the recording to monitor to maximize space on the screen" (a choice). Assumption E-1 (ASK: YES).
- The end of a clip recording, inside L88: "ending on a even amount" against "keep recording till it is on an even grid line like the end of the bar"; with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (BD:812-813). Assumption E-6 (ASK: YES).
- See-through parts. His "yes" that "a single-layer recording keeps its see-through parts" (BD:671) against L88: "I don't think there's any reason for a one layer recording" and "It must be the full screen composition". The newer line wins for the layer; what is left open is assumption E-9 (ASK: YES).
- Which controls get a track, inside L81: "we are not recording anything that does not move" against "was not changed during the show and was at default the entire time, does not get a row" (a control that stood away from its default is not covered). Assumption E-13 (LINE). His earlier "there is a row in the recording for every parameter and button that can be adjusted" (BD:1089) is replaced by L81; no question.
- The spacebar. His 170 words "presses the spacebar, it will stop playing" (BD:1090, the review screen) against L118: "Spacebar is typically tap tempo" (the live window). Two screens: assumption E-29 (LINE); the topic-J paper owns the live key.
- Replaced by his newer words, no question: "whole ouput of select layer" (BD:574) by L88; "It should keep playing at with the current parameters at the end." (BD:503, the end of a replay) by the review screen's own rule that playback stops on the last moment (R223 b, shown to him and not corrected).

## NOT DONE / UNSURE
- The cost of the low-resolution show recording (his question in L6) is not answered here: a researcher's measurement decides whether G4 is built and with which numbers (E-22).
- The screen's name (L72, L114) is the topic-J paper's; "the review screen" is used meanwhile.
- Whether the app tells him that a recording was ended by a full disk, or was saved without sound, hangs on question 209, where his "209 already dicsussed above" (L122) is applied by the paper that owns 209; R187 c leans on it.
- D21 leans on question 175 keeping its default A in the topic-A paper; if that paper finds words of his against it, R223 e and D21 need a second look.
- Where show recordings, their recorded show files and recorded clips are kept on the disk, and whether they travel with a show to another computer, is the topic-F paper's (R188); his L48 and L50 speak of presets only. Cheapest: one line on his next page if that paper leaves it open.
- That a recorded moment can be drawn fresh without touching the live show, and how fast a jump is, is unmeasured (AR T3; FT "WHAT I COULD NOT ESTABLISH" 6). It needs a measurement in the build session, before R165 and D16 are promised.
- "Today" lines were taken from the two fact sheets and not re-derived from the source, as the task allows.
