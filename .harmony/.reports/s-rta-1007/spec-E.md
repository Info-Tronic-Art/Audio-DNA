# SPEC E -- The review screen and recordings (s-rta-1007): the paper apply-E.md with the ruling rule-E.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 186
TITLE: The outputs while you review
STATUS: ANSWERED b
HIS: L91, L79
RULE: While the review screen is open, every connected output shows the review picture too: what plays, and what a click or a scrub lands on (his "186 b", the letter without a switch). When the review screen is closed the outputs show the live show again, with nothing playing (R165). There is no extra switch for it: an output is switched off the way any output is, and each output's own screen settings apply to the review picture as to any picture it shows (assumption E-1). The recording's sound is heard while the recording plays in real time (R138, L86); through which sound output: assumption E-33. If Record Over runs in the review screen (assumption E-2), the outputs show its picture as well. His L79 follows from it: with the picture on a monitor, the tracks can have the room on the main screen (R138).
CHANGED: The default A (outputs black) is replaced by B, his "186 b" (L91). The page's note under the question that "the recording's own picture and sound never reach the outputs" falls away with it. Ruling: three things the paper's RULE gave as settled (each output's own settings, the sound, what a Record Over shows) are no words of his; they now stand as assumptions E-1, E-33 and E-2.
TODAY: A replay drives the live layers and the live outputs; there is no separate review picture (FT section 1, S5; AR 1.10). A review renderer and its route to the outputs do not exist; both are to be built. Through which device a recording's sound is played now: not checked.
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
STATUS: ANSWERED in his words (Record Over stays; no letter)
HIS: L92, L84
RULE: Record Over stays. It is done with the MIDI controller and it plays in real time: the recording plays with its sound and its recorded moves, the picture shows what the moves do, and what he moves on the controller while it plays is recorded over the moves of those same controls. A control he moves supersedes its recorded track while he holds it and goes back to the recorded track as soon as it is let go (his earlier words, BD:340-341); what counts as "let go" for a knob on a controller: assumption E-32. It is a third way of changing a recording, apart from scrubbing and from drawing with the mouse (L92). A Record Over never changes the original: it is recorded separately from the original and kept beside it, with the same sound (L92; how many copies: assumption E-4). On which screen it runs: assumption E-2 (his answer names no letter, and letter A said the live window). Which controls can record over: assumption E-3. While a Record Show or a clip recording runs, Record Over is not available (R223 c).
CHANGED: No letter was given. That Record Over stays and that the old moves play under it are his: "yes we can record over using the midi controller" (L92) and "The knob will supersede whatever is happening, And will snap back to the recorded track as soon as it is let go." (BD:340-341). A's "kept as a new recording beside the first, with the same sound" is confirmed by "Record overs are recorded separately from the original" (L92). A's "reached from the list of recordings ... play in the live window" is neither confirmed nor replaced by his words: it is the ALT of assumption E-2. B (only the sound plays) and C (Record Over goes) are not taken.
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
RULE: The review screen is a screen of its own, not the live window: the picture at the top, one track per changed slider or button below it (L81). While it plays, the stop button, the spacebar, or a mouse press inside a track stops it and changes nothing else. A click in a track moves the picture and the timeline to that moment; a drag scrubs both together (with the pointer tool, item 187). It does not start playing again by itself: the spacebar or play does that, from where the playhead stands. The tempo row's buttons are not on this screen. From L79: the line between the tracks and the picture can be dragged, to make the picture as small or as large as the screen allows; and because the connected outputs show the review picture (186 b), he can watch it on a monitor and give the main screen to the tracks. Whether the picture on the main screen can be closed altogether is layout (L8). Sound: it plays while the recording plays in real time; whether it is also heard during a scrub is a setting (R183, L86).
CHANGED: Added: the draggable line between tracks and picture, and the picture watched on a monitor (L79). "monitor" is read as a connected output, which shows the review picture by his 186 b: INFERRED (assumption E-1). Replaced: "the recording's sound plays while it plays and is silent while you scrub" becomes a setting (L86). "one track per slider or button" now means per changed control (L81). The rest stands.
TODAY: None of it exists (AR 1.3, 1.10; FT section 1). No space-key handler exists in the app (AR correction 5).
@@END

@@ITEM R165
TITLE: Opening the review screen stops the live show
STATUS: STANDS
HIS: L72, L35, L91, L80
RULE: The review screen is for after the show. Opening it is like a press on stop for the live show: the clips leave the layers and the tempo stops; the app asks first if clips are playing. Closing it brings back the live window with the show as it was and nothing playing. Reviewing or mending a recording (and recording over it, if that runs here: assumption E-2) never changes a slider, a button or a clip of the show in the live window: the review works on the recording's own copy of the show (R173, L80), whichever show is open in the live window. The only thing that goes from the review screen into a show is an action he saves there (assumption E-11). While it is open the outputs show the review picture (186 b).
CHANGED: nothing; its pointer to question 186 is now answered b. His words fit it: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72) and "live and recording review mode" (L35). Ruling: said outright that the review uses the recording's own copy beside the live show (this reading, which he left as it was, with L80).
TODAY: A replay runs through the live layers, so review and live cannot run side by side (FT section 1, S5). Showing a recorded moment without touching the live show is not built and its cost is unmeasured (AR T3).
@@END

@@ITEM R173
TITLE: A show recording saves its own show file
STATUS: CORRECTED
HIS: L80, L89
RULE: Every show recording saves with it a show file that holds the clips exactly as they are for that show: the recorded show file (L80, L89). The review screen opens a recording from its recorded show file, so the recording opens correctly whatever has been done to the show since, also when the show of that name has been changed. It opens that copy for the review only: the show in the live window is not replaced, and comes back as it was when the review screen is closed (R165). When the show of that name has changed since, the app tells the user which clips have been moved or are missing; read as: a clip that now sits in another cell of that show, a clip that is no longer in it, and a clip whose file is not found on the disk (INFERRED from L80). When the recorded show file is taken and what the review takes from it: assumption E-34; whether it holds the media: assumption E-12. What is removed or added during a recording is kept in it (R223 d). An action saved in the review screen goes into the show (his One Save: it is written to the show file at the next Save, and the quit window asks about it like any other change). Into which show and onto which clip: assumption E-11; under its reading the action goes into the show the recording was made in, and if another show is open in the live window the app says which show to open before the action can be saved.
CHANGED: Replaced: "it is reviewed with that show open (if another show is open, the app says which one to open)" and "The review uses the show as it is now" -- by L80: "the recording also saves a show file with the clips exactly as they are for the show" and "the recording still opens it up correctly and indicates this to the user that these clips have been moved or missing". Replaced: "a row of a clip or an effect that you have since removed is marked missing and cannot be ticked" -- its track is there and plays in the review; what saving does for it is E-11. The last sentence (the action goes into that show, One Save) stands. Ruling: added that the copy is opened beside the live show (R165 with L80), what "moved or missing" is compared with (INFERRED), the case of another show being open (part of E-11) and the pointer to E-34.
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
RULE: A mend (a shape drawn over a curve, a piece moved, a Quantize) is never written into the original recording. The mend is another recording that is saved, parallel to the original (L84): it stands beside the original in the list of recordings and shares its sound and its recorded show file. The original stays exactly as it was recorded and can always be opened again. The mended recording is saved by the app as he works, with no Save press, and is there when the review screen is closed and opened again. When the mended copy is made, how many there are and what a mend of a mended recording does: assumption E-4. An action saved earlier from a recording does not change when that recording is mended afterwards.
CHANGED: Replaced: "a mend ... changes the recording", "The night as it was recorded is always kept underneath" and the button "Back to as recorded" -- by L84: "the mend is another recording that is saved, parallel to the original." (he took the page's own closing offer). The page's "it is kept when you close the review screen" and the sentence about earlier saved actions stand. Ruling: that it is saved without a Save press is the page's "kept when you close", which he did not change; his own word is "saved" (L84), so the when is carried by E-4.
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
STATUS: CORRECTED (added to; g replaced; a to f stand)
HIS: L86, L81, L114, L71
RULE: The three boxes of Record Show stand as shown: (a) Parameters: the moves of the controls; it gives the review screen its tracks and its picture, which is made fresh from the moves and the recorded show file. (b) Audio: the night's sound; it gives the review screen its sound and its audio track. (c) Video: a film file of the composition to keep or share; the review screen does not use it. (d) The boxes sit beside the Record Show button; in a new app Parameters and Audio are ticked and Video is not; the app remembers the ticks; with no box ticked the button is greyed. (e) With Audio and Video both ticked the film has the night's sound; a Video-only recording is a film file and is not in the list of recordings. (f) The Output menu's own film recording goes: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone." (L114). New from L86: each recorded track, button or slider, is shown in a small row that can be dragged taller, with its keyframes and slider positions. A button's track is a horizontal line that can only be in two positions, 0 and 100, unless the control has more than two positions; then its line has one level for each position. A setting decides whether the sound is heard while scrubbing; with it off the sound plays only when the recording plays in real time. How the setting starts: assumption E-19. (g) A slider that a signal was moving has a track that shows what it did that night, in the colour of signal moves (L71, D18); that its moves are stored as values, so that this holds also when Audio was not ticked, is assumption E-24. A small film of the night, if he wants one (G4), is one more box: assumption E-21.
CHANGED: Added: the track's look and the draggable row height, the two-position button line, the scrub sound setting (L86); "one level for each position" is INFERRED from "unless this is something with more than one position". (a) "its rows" now means tracks of changed controls only (L81). (g) "Until it is measured I cannot promise what a slider that the music was moving shows when Audio is not ticked" is replaced: that the track is there is his (L71: "recorded off of signals"); how the moves are kept is Harmony's (E-24). (a) to (f) stand.
TODAY: One tick "Record audio", on at every launch and not remembered; Parameters always on; the film is a separate Output menu command with no sound (AR 1.2, 1.13, H9, H10). The film may play at the wrong speed (AR 1.14, T2: inferred, not run).
@@END

@@ITEM R184
TITLE: The list of recordings and the Record tab
STATUS: CORRECTED (one sentence of d withdrawn; names; Render planned)
HIS: L87, L91, L114, L128, L92
RULE: (a) The place where a recording is watched, mended and cut into actions is one screen, the review screen. (b) Recordings are found in a list: name, date, length, the show each was made in. A double-click opens one in the review screen; a recording can be renamed and shown in Finder. Mended and recorded-over recordings stand in the list beside their original (R222, 195). (c) The app never deletes a recording; it is removed in Finder. (d) The page's sentence "To play a whole recording as a performance you make one action from its start to its end." is withdrawn (his L87; the answer R184 explains it). A recording is played in the review screen, and the outputs show it (186 b). That nothing plays a recording inside the live window, and that Record Over is a button of the review screen, is assumption E-2. Render, making a film from a recording afterwards, is planned: "215 plan all of these" (L128). (e) A recording's sound that was damaged by a crash is repaired by itself when the list opens. (f) Beside the list there are: the Record Show button, which reads "Stop Recording" while a recording runs; the box for the recording's name; the boxes of R183. The word "take" is not on screen (Harmony's reading of "All else is gone", L114; the list of names owns the wording).
CHANGED: (d) the sentence he asks about in L87 is withdrawn. Whether a recording can still be played inside the live window is not settled by his words and is asked with the place of Record Over (E-2). (d) "Whether you still want Render: question 215 (f)" is answered: planned (L128). (f) the button's name follows L114 ("record show"). (d) "Record Over: question 195" is answered by L92 as far as it stays; its place is E-2. The rest stands.
TODAY: Record tab: "Record Take" / "Stop Recording" / "Record Over", "Play Take" / "Stop Playback", "Load Take...", "Show in Finder", "Repair Audio", "Record audio", "Play with audio", the name box, "Take (timelines and audio)", "Render... (coming)", "Takes: <folder>", "Save Routine" (AR 1.1, 1.2). No list of recordings (AR 1.3, H5). No key or pad starts a recording (AR 1.4).
@@END

@@ITEM R185
TITLE: The review screen: keys, Undo and the tempo row
STATUS: REPLACED in part (a: the controller in the review; c is marked as inferred)
HIS: L92, L89, L72
RULE: (a) While the review screen is open the keyboard is its own: the spacebar stops and plays (his words for the stop, BD:1090); Cmd+Z undoes the last change made there (a mended curve, a Quantize, an In or an Out). The keys and pads of the keyboard and MIDI mapping fire nothing into the live show, which is stopped. If the controller works in the review screen (assumption E-2): while a recording only plays, a knob or fader he moves supersedes its control for as long as he holds it and goes back to the recorded track when he lets go, and the recording is not changed (his words on play mode, BD:340-342; what "let go" is for a knob: assumption E-32); during a Record Over what he moves is recorded over (195). (b) A show recording keeps the presses on the tempo row (play, pause, stop, Tap, Resync, the nudge, a tempo change), so the review picture goes empty where stop was pressed that night; they are shown and cannot be selected into an action. (c) The grid follows the night's tempo as it changed (INFERRED from L89).
CHANGED: (a) "the keys and pads of the keyboard and MIDI mapping fire nothing" is replaced for the controller: during a Record Over by L92 ("yes we can record over using the midi controller"), and while a recording only plays by his earlier words: "It could be in record over mode or it could just be in play mode and play mode. The timeline doesn't change and then record mode. The timeline is updated based on the knob movement." (BD:341-343). (b) stands. (c) stands; L89 says "When tempo changes, record that as it will happen often."; that the grid follows the recorded tempo is INFERRED from it, not his word.
TODAY: No space-key handler; Cmd+Z undoes the last fire app-wide (AR correction 5, M8). Tempo commands are recorded; a tempo-row stop does not exist yet (AR 1.8, H26; FT SURPRISES S4).
@@END

@@ITEM R186
TITLE: Record to Clip: always in time, always the full composition
STATUS: CORRECTED (d and e replaced; c follows L88 to the letter; the main reading of the empty parts in a turns round and is asked: page item 230)
HIS: L88, L114
RULE: (a) A clip recording has no time limit except the disk; while it records its picture is seen in the cell it lands in. Its empty parts: the main reading is a full picture exactly as on the output, black where no layer shows (L88: "It must be the full screen composition"; his earlier "The recording should look exactly like the output.", binding-decisions.md:626); his earlier "yes" to see-through parts was given for a single-layer recording (binding-decisions.md:671), which L88 removes; keeping them see-through is way b of page item 230, which asks him (assumption E-9). (b) It lands in the next empty cell from the left on the top layer of the deck on screen, without that cell being selected. (c) It is always recorded in time: it starts on the "1" (the next bar line after the REC press) and it ends on a bar line: after the stop press it keeps recording until the end of the bar (L88: "keep recording till it is on an even grid line like the end of the bar"). The clip that lands is a BPM-mode clip with its beats and tempo set, as long as what was recorded: every recorded bar is kept and no out point is moved by itself (5 bars stay 5). He trims it later with its in and out points (L88: "It can be trimmed later if necessary."). The other reading of "a even amount", that the recording runs on until a group of 4 bars is full, is way b of page item 260 (assumption E-6). (d) Nothing is recorded while the tempo is paused: a clip recording never starts or runs on a standing picture, and there is no clip recording that is out of time. What the REC press does while the tempo is paused or stopped: assumption E-5 (page item 259). What a pause or stop does to a recording that runs: E-7. (e) There is no one-layer recording and no choice between whole and layer. A clip recording is the output of the layers, the full composition, and never the picture of one output screen, because a screen can be altered to fit a projector's colour and timing. What the full composition includes: assumption E-8. (f) A recorded clip has no sound. (g) A clip recording is never shorter than 4 bars: a stop press before the end of bar 4 ends it on the bar line that closes bar 4. (h) When it lands it waits in its cell, ready to be fired; it does not start playing by itself. (i) It lands in the deck that is on screen at the REC press; if the top layer has no empty cell a new column is added to that deck.
CHANGED: (d) "With the beat stopped or paused, REC starts and ends on your press and the clip is not in BPM mode" is replaced by L88: "it is impossible to record something paused. Anything is recorded it is recorded in time on the one and ending on a even amount. If we push stop recording, keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (e) the one-layer recording, its selected layer and its blend sentence are removed by L88: "I don't think there's any reason for a one layer recording. Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues. It must be the full screen composition." (c) the end is now his words (end of the bar). (a), (b), (f), (g), (h), (i) stand; his earlier "whole ouput of select layer" (BD:574) and the "yes" to a single-layer recording's see-through parts (BD:671) are overtaken for the layer half. BY THE RULING OVER ALL TOPICS (2026-10-08): (c) the out point is no longer moved in to the last full group of 4 bars by itself: L88 is read to the letter (end of the bar, trimmed later) and shown to him as page item 260; (a) the main reading of the empty parts turns round to "as on the output" and is asked as page item 230, because the only "yes" of his to see-through parts was for a single-layer recording.
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
RULE: (a) When the review screen opens nothing is selected and Save is greyed until something is. The tracks are greyed out and the selected area is white. The selected area is the selected tracks, between the In and Out points on the timeline; selecting a name selects all its tracks. Where In and Out stand when a recording opens: assumption E-36. (b) When playback reaches the end of the recording it stops on the last moment and stays there; play starts again from the beginning; it never carries on into the live show (assumption E-37: his earlier words on the end of a replay say otherwise). (c) While a recording runs, the way into the review screen and Record Over are greyed. (d) When something is removed during a recording, the removal is recorded and the thing is kept in the recorded show file; from the moment of its removal all its settings are at their defaults and its tracks show that (L89). So the review shows it working up to the removal and gone after it. "It" is read as an effect, and the rule is taken to hold for a clip or a layer too: assumption E-17. What is added during a recording: assumption E-35. (e) Tempo changes are recorded, as they happen often, and are shown. An action is not made with a tempo change: it never holds the tempo and never changes it. An action cut from a stretch in which the tempo moved: assumption E-18. A recording always has a tempo (D21), so its grid always shows bars.
CHANGED: (a) added: the greyed tracks and the white selected area, and what the selected area is (L89). (b) stands as shown; the page called it an edge no word of his covers, but his earlier words on the old replay do ("It should keep playing at with the current parameters at the end.", BD:503): carried by E-37. (d) "an effect you remove while recording keeps its rows, which move nothing once it is gone" and its closing question are replaced by L89: "If it is removed, record the removal and keep it in the recorded show file, but default all its settings once removed." INFERRED: in "Very little chance a recording will be removed" the word "recording" is read as the thing removed while recording (E-17). The page's "an effect you add while recording has no rows and is not added again in the review" is not touched by his words; E-35 proposes the opposite and shows it to him. (e) "takes the tempo at its In point" is replaced by L89: "When tempo changes, record that as it will happen often. Actions will not be made with a tempo change."; the sentence on 175 B falls away (175 takes its default A, L1). (c) stands.
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
RULE: The picture in the review screen is made fresh each time from the recorded moves and the recorded show file; it is not a film of the night. So a mend or a Record Over changes the picture that is seen. The clips' files must still be on the disk; a clip whose file is not found is named as such when the recording opens. What the app picks by chance during the night: assumption E-38. Under a fast scrub a video's picture may lag a moment: assumption E-28. The small film of G4, if he wants it, is a film of the night to hold this picture against.
CHANGED: "with the show it was recorded in" becomes the recorded show file (L80). Ruling: the paper's sentence that the low-resolution recording "is the film to check this picture against" is cut back to what L6 says ("to use as a double check"); what chance does to a picture made fresh is named (E-38); the scrub remark points at E-28.
TODAY: A recording stores no picture; the only renderer is the live one, and it can only play a recording from its start (FT section 1, S5; AR T3).
@@END

@@ITEM D17
TITLE: What a recording is able to keep
STATUS: REPLACED in part by L81
HIS: L81, L86, L76, L80
RULE: A recording can keep any button, slider, drop-down list or play / direction button of the clip, layer and global tabs and of the layer strip, however it is moved, the mouse included; but it keeps a track only for those that changed during the night (L81). Where every control stood when Record Show was pressed is in the recorded show file (L80; assumption E-34), so the review picture is right from its first moment.
CHANGED: "will keep every slider and button ... from its first moment" is replaced: "we are not recording anything that does not move." (L81). That any kind of control can be recorded is his: "Each track recorded, button or slider" (L86) and "the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes" (L76). Ruling: that the recorded show file is taken at the Record Show press is Harmony's reading of L80 and is carried by E-34.
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
STATUS: SETTLED by his earlier words (BD:283-286)
HIS: none in this message (his earlier words, BD:283-286)
RULE: A show recording with Audio ticked keeps the sound the app heard, whatever its source: the microphone, the sound card or an audio file played in the app. That the sound stays in time with the recorded moves is tested when it is built.
CHANGED: nothing in the rule. Ruling: it is no longer Harmony's own. His words: "it was either the Audio from the microphone or from the sound card or from whatever. It gets recorded Along with the log and we will test it to make sure it stays in time." (BD:284-286). Assumption E-25 is dropped.
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
STATUS: OPEN (that the app picks is his word, BD:650-651; which types is Harmony's: assumption E-27; it waits for E-9 and for the answer to his codec question L3)
HIS: L3, L6
RULE: The app picks the file types itself (his words: "We should pick the most optimal Kodex to use and not worry about the other ones." and "I want codecs that decode easily and play well", BD:650-651): a film and the pieces of the small film open in ordinary players; a recorded clip is in the type that plays back most lightly in the app and, if assumption E-9 holds, carries its see-through parts. A film plays at the right speed whatever the output's frame rate. Which types exactly is decided after his codec question (L3) is answered, which another paper owns.
CHANGED: The see-through half ("a recorded clip keeps its see-through parts") now hangs on assumption E-9, since the one-layer recording it was ruled for is gone (L88). Ruling: the first sentence is marked as his own words (BD:650-651), so only the choice of the types is still open.
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
TITLE: A small film of the show in pieces, as a double check
STATUS: ANSWERED (his cost question is answered in the answer lowres; whether and how it is built is his choice: assumption E-21)
HIS: L6, L88
RULE: With Record Show the app can also write a very small film of the show as a double check (L6). It shows the full composition, never one screen's corrected picture (L88). It is written in pieces of 10 or 20 minutes, each a small file that plays by itself, so that if something happens most of the recording is not lost. Pieces alone do not do that: a film that is cut off by a crash cannot be played, so each piece is also written in a way that a crash costs only its last seconds; a forced quit in the middle of a piece is the test, and nothing is promised before it. How the matter stands now: his question on the cost has its answer (the answer lowres): it should be light on the computer, every number in it is an estimate, and the one thing nobody knows is whether fetching the small picture makes the live picture hitch. That is measured on his Mac with a heavy show before the film is left switched on; if it costs the output its smoothness he is told, and the output comes first (assumption E-26). Whether he wants it, its size, its piece length, its sound (in it only when the Audio box is ticked, as for the full-size film, R183 e) and how it is switched on: assumption E-21 (his choice). Where the pieces are kept and watched: assumption E-23.
CHANGED: New; it answers no item of the page. His words: "I think it might be good to record a very low resolution show recording if that is possible and to record it in 10 or 20 minute chunks so that they are small and if something happens most of the recording is not lost. Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check. How much comp resource would this take up?" (L6). Ruling: the paper's "without a gap", "numbered in order" and "built only if the researcher's answer shows ..." were no words of his and are taken out or carried by E-21 and E-23; the cost question is answered from answer-lowres-rec.md as corrected by check-lowres.md (the ranges are widened: the re-check found the encoder about twice as costly and its files about twice as large as the answer paper assumed; nothing was run by either).
TODAY: The only film recorder writes one full-size file with its index at the end, so a crash leaves the file unplayable: no pieces, no fragments (VideoRecorder.cpp, check-lowres fact 8). It cuts a corner out of the picture instead of shrinking it (VideoRecorder.cpp:117-130) and has no steady picture rate, so the file would play at the wrong speed (inferred, not run; check-lowres facts 6, 7). It carries no sound (VideoRecorder.h:34) and uses the software encoder only (check-lowres facts 3, 4). Nothing of the small film exists and no measurement exists.
@@END

## ASSUMPTIONS
@@ASSUME E-1
ABOUT: 186, R138
TEXT: I assume every connected output shows the Review picture whenever Review is open, each with its own screen settings. There is no extra switch: to keep it off a projector you switch that output off as usual.
WHY: L91 "186 b" is the letter without a switch (letter c had one); L79 "use can chose to output the recording to monitor" could also be read as a choice.
ALT: b) A switch in Review, "Show on outputs", as in letter c of the question.
IF-WRONG: SMALL a switch can be added later.
ASK: LINE his "186 b" picked the letter without the switch, and an output can be switched off as always.
@@END

@@ASSUME E-2
ABOUT: 195, R184, R185, R165, 186
TEXT: I assume a recording plays only in Review, never inside the live window. Record Over is a button in Review: the recording plays in real time and what you move on the controller is recorded over it.
WHY: L92 does not say on which screen; letter A of question 195 said the live window and he named no letter; his 170 words call Review "the replay window" (BD:1090).
ALT: b) Record Over runs in the live window: the recording plays through your live layers while you redo the moves, and a whole recording can be played there too, with live clips on top.
IF-WRONG: REBUILD it decides whether Review takes the MIDI controller, and whether the live window can still play a recording.
ASK: YES no word of his places it, and it shapes how Review and the live window are built.
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
TEXT: I assume all your mends of one recording go into one mended copy beside the original, saved by itself as you work. Each Record Over makes one more copy. Every copy is complete and shares the original's sound.
WHY: L84 "parallel to the original" and L92 "recorded separately" do not say how many copies, when one is saved, or whether a copy is whole.
ALT: b) Every time you mend and close, one more copy is made. c) Only the changes are kept, as a layer over the original that you can switch off.
IF-WRONG: SMALL the list of recordings would be grouped differently.
ASK: LINE a choice of mine on how the list fills up.
@@END

@@ASSUME E-5
ABOUT: R186
TEXT: I assume that if you press REC while the tempo is paused or stopped, the clip recording waits and starts on the first "1" once the tempo runs again, the way a BPM clip waits for the "1".
WHY: L88 "it is impossible to record something paused" does not say what the press does then; L12 has a new BPM clip wait: "it waits for the one".
ALT: b) REC is greyed and does nothing until the tempo plays. c) From stop, the REC press itself starts the tempo and is the new "1".
IF-WRONG: STAGE you press REC before a song and get a recording you did not expect, or none.
ASK: LINE it follows his own rule for a BPM clip (L12), and nothing is lost while it waits.
@@END

@@ASSUME E-6
ABOUT: R186
TEXT: I assume a clip recording stops at the end of the bar in which you press stop. Its out point is then set at the last full group of 4 bars; the bars beyond it stay in the file.
WHY: L88: "ending on a even amount", "an even grid line like the end of the bar", "It can be trimmed later"; older: "always work with multiples of 4" (BD:812).
ALT: b) It keeps recording until a group of 4 bars is full, so nothing is cut off. c) It ends at the bar line and the clip keeps every bar, 5 bars stay 5; you trim it yourself.
IF-WRONG: STAGE the clip loops shorter or longer than he meant.
ASK: YES "even amount" can be read three ways, and it decides how long every recorded clip loops.
@@END

@@ASSUME E-7
ABOUT: R186
TEXT: I assume a tempo pause holds a clip recording: nothing is recorded while paused, and it carries on in time when you press play. A tempo stop ends the recording and keeps what was recorded.
WHY: L88 rules out recording a paused picture; L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM"; L72: stop stops "everything else".
ALT: b) A pause also ends the recording, at its last full bar.
IF-WRONG: STAGE a recording that ends, or goes on, when he did not mean it.
ASK: LINE the hold follows his L13 words, and neither way loses what was recorded.
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
TEXT: I assume a recorded clip keeps the empty parts of the picture see-through, so the layers under it show when it plays. The likely price, not measured: larger files and more work for the computer while recording.
WHY: His "yes" to see-through parts (BD:671) was for a one-layer recording, which L88 removes; "The recording should look exactly like the output" (BD:626) would give black.
ALT: b) Empty parts are black, exactly as on the output: smaller files and a lighter recording.
IF-WRONG: REBUILD it decides the file type of every recorded clip.
ASK: YES his own change left it open, and the file type is costly to change later.
@@END

@@ASSUME E-10
ABOUT: R186, R184, R185
TEXT: I assume starting on the "1" and ending on a bar line is for Record to Clip only. Record Show starts and stops when you press and keeps running through a tempo pause or stop, as the music does.
WHY: L88 answers the clip reading but says "Anything is recorded it is recorded in time on the one"; the night's sound does not pause with the tempo.
ALT: b) Record Show also waits for the next "1", ends on a bar line, and holds while the tempo is paused.
IF-WRONG: SMALL up to one bar at each end of a show recording.
ASK: LINE the reading that cannot lose any of the night; he can strike it in a second.
@@END

@@ASSUME E-11
ABOUT: R173, R165
TEXT: I assume an action saved in Review goes into the show the recording was made in, as that show is now: onto the same clip wherever it moved. If the clip is gone, the app says so and saves nothing.
WHY: L80 has the recording open from its own show file and name moved or missing clips; not where a saved action lands, nor what if another show is open.
ALT: b) The action is saved anyway and waits; you paste it onto the clip you choose. c) It goes into whatever show is open in the live window.
IF-WRONG: REBUILD where actions are kept and how a clip is recognised depend on it.
ASK: YES no word of his covers it, and it is what Review is for.
@@END

@@ASSUME E-12
ABOUT: R173, D16
TEXT: I assume the show file that a recording saves holds every clip's settings and where its files are on the disk, not copies of the picture and video files themselves.
WHY: L80 "a show file with the clips exactly as they are", and "moved or missing", fit a file that points at the media.
ALT: b) The recording also copies the media files, so that it opens even when they are deleted; it would be very large.
IF-WRONG: SMALL copying media could be added later as a command.
ASK: NO technical; his "moved or missing" (L80) already expects a file that points at the media.
@@END

@@ASSUME E-13
ABOUT: R137, 188
TEXT: I assume a control that never moved but stood away from its default the whole night also gets a track: a flat line at that value.
WHY: L81 records nothing "that does not move", yet drops only what "was at default the entire time"; an action holds "all parameters that are changed from it’s defaults" (L76).
ALT: b) No track: only controls that moved during the recording get one.
IF-WRONG: SMALL more or fewer tracks in the list.
ASK: LINE his sentence can be read both ways; the cost is small.
@@END

@@ASSUME E-14
ABOUT: R135, R166
TEXT: I assume "1 beat is the smallest" is for the grid and the In and Out points, and Quantize has its own spacing list that goes down to a quarter beat.
WHY: L82 "1beat is the smallest" answers the grid; L83 gives Quantize its own list ending "1 beat, 1/2 beat, 1/4 beat". Both lines hold only as two lists.
ALT: b) Nothing in Review goes finer than 1 beat, and the Quantize list ends at 1 beat.
IF-WRONG: SMALL a list of sizes.
ASK: LINE the one reading that keeps both of his lines.
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
WHY: L85 "action should not record switching decks" is about actions; the reading he read says the deck changes are shown, and he did not change that.
ALT: b) Deck switches are not recorded at all and the review screen shows none.
IF-WRONG: SMALL one track more or less.
ASK: NO a deck switch never changes the picture; whether its mark is drawn is a look (L8).
@@END

@@ASSUME E-17
ABOUT: R223
TEXT: I assume your removal rule holds for anything taken out while you record, an effect, a clip or a layer: the removal is recorded, it stays in the recorded show file, and its tracks sit at default from then on.
WHY: L89: "Very little chance a recording will be removed. If it is removed, record the removal"; "a recording" is read as the thing removed while recording.
ALT: b) You meant only an effect. c) You meant something else by "a recording".
IF-WRONG: SMALL one rule for a rare case.
ASK: LINE a typing slip read kindly; he can strike it.
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
ASK: NO a setting's first state; he said it "won't sound good if it's scrubbed" (L86).
@@END

@@ASSUME E-21
ABOUT: G4, R183
TEXT: I assume you want it built like this: with every Record Show, unless you untick it, a small film with the night's sound, a quarter as wide and as high as the show, in 10-minute pieces.
WHY: L6 offers "10 or 20 minute chunks" and "1/4 or 1/8 the size"; "the size" can mean each side or the area; sound and its switch are not named.
ALT: b) A quarter of the area (960 by 540 for a full HD show): sharper, about four times the size and the work (estimate). c) An eighth as wide and as high (240 by 135). d) 20-minute pieces. e) No sound in it. f) Not at all.
IF-WRONG: STAGE it would run during every show you record, and whether it costs the picture its smoothness is not measured yet.
ASK: YES he asked the cost in order to decide; whether it runs by itself and at which size is his.
@@END

@@ASSUME E-23
ABOUT: G4
TEXT: I assume the pieces are plain film files kept in the show recording's folder and watched in Finder or any player; Review does not play them. Each piece is written so that a crash should cost only its last seconds.
WHY: L6 says "to use as a double check" and not where it is watched; the crash safety is a requirement, tested by a forced quit before it is promised.
ALT: b) Review can show the small film beside its own picture for comparing.
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

@@ASSUME E-26
ABOUT: D20, G4
TEXT: I assume Record Show, a clip recording and the small film can all run together; if the computer cannot keep up, the output stays smooth and a recording loses pictures instead.
WHY: L6 adds one more recording; no word of his says which gives way.
ALT: b) The app refuses to start a second recording while one is running.
IF-WRONG: STAGE only if the output stuttered; this choice is the one that protects it.
ASK: LINE no word of his ranks them; the output first is my rule for a live show, and he can strike it.
@@END

@@ASSUME E-27
ABOUT: D22
TEXT: I assume the app picks the file types itself: films that open in ordinary players, and for a recorded clip the type that plays back most lightly in the app.
WHY: His words give the rule ("pick the most optimal Kodex", "codecs that decode easily and play well", BD:650-651); which types waits for his codec question (L3) and the see-through choice.
ALT: b) A codec of our own for recorded clips, as his L3 asks about.
IF-WRONG: REBUILD only for recorded clips already on disk.
ASK: NO the answer to his own codec question carries it to him.
@@END

@@ASSUME E-28
ABOUT: D16
TEXT: I assume that under a fast scrub a video clip's picture in the review screen may lag a moment behind the mouse.
WHY: A technical limit of drawing the picture fresh; no word of his.
ALT: none
IF-WRONG: SMALL
ASK: NO technical.
@@END

@@ASSUME E-30
ABOUT: R187, R186
TEXT: I assume a clip recording that is cut short (a quit, a full disk, a tempo stop) ends at once and is always kept: its out point at the last full group of 4 bars, a shorter one kept whole.
WHY: L88's "keep recording till it is on an even grid line" cannot hold at a quit, a full disk or a tempo stop; under 4 bars is not covered.
ALT: b) With fewer than 4 bars recorded nothing is kept, and the app says so.
IF-WRONG: SMALL a rare edge.
ASK: NO a technical edge; the default never throws a recording away.
@@END

@@ASSUME E-31
ABOUT: P25
TEXT: I assume the audio track shows the waveform with the night's bar lines over it, and no marks for single drum hits.
WHY: His reference words put "the audio track across the top" and a grid over all (BD:1062-1064); nothing more.
ALT: b) A mark at every drum hit the app heard.
IF-WRONG: SMALL
ASK: NO a look, settled in the UI redesign (L8).
@@END

@@ASSUME E-32
ABOUT: 195, R185
TEXT: I assume a knob on the controller counts as let go once it has stood still for about one beat: then the recorded moves take over again, as you said for a control you let go.
WHY: His words: "will snap back to the recorded track as soon as it is let go" (BD:340-341); a controller's knob cannot tell the app that a hand has left it.
ALT: b) Once you move a control in Record Over it stays yours until you stop Record Over.
IF-WRONG: SMALL one rule, but it decides how Record Over feels.
ASK: LINE his words give the rule, not the moment a knob counts as let go.
@@END

@@ASSUME E-33
ABOUT: 186, R138
TEXT: I assume the recording's sound in Review plays through the Mac's own sound output, the one chosen in the Mac's sound settings.
WHY: No word of his names where Review's sound comes out; "186 b" (L91) names the picture.
ALT: b) A sound output chosen inside the app.
IF-WRONG: SMALL a setting could be added.
ASK: NO technical.
@@END

@@ASSUME E-34
ABOUT: R173, D17, R165
TEXT: I assume the show file a recording saves is taken the moment you press Record Show, with every change you had made, saved or not. Review takes the clips and settings from it and leaves your outputs as connected.
WHY: L80 "with the clips exactly as they are for the show" does not name the moment; a show also remembers its connected outputs (L96), which Review must not change.
ALT: b) It is a copy of the show file as it was last saved.
IF-WRONG: SMALL Review would start from another look than the night had.
ASK: NO technical; only this reading lets Review start as the night started.
@@END

@@ASSUME E-35
ABOUT: R223
TEXT: I assume an effect or a clip you add while recording is recorded as well, from the moment you add it, so that Review shows the night as it was.
WHY: L89 rules only on a removal; the page's line that an added effect gets no tracks was not changed by him.
ALT: b) A thing added while recording is not recorded: it has no tracks and Review does not show it.
IF-WRONG: SMALL where something was added, Review would differ from the night.
ASK: LINE the mirror of his removal rule; the page had said the opposite.
@@END

@@ASSUME E-36
ABOUT: R223, R135
TEXT: I assume that when a recording opens in Review, In sits on its first bar line and Out on its last, so the whole recording lies between them until you move them.
WHY: L89 defines the selected area by In and Out; no word says where they stand at first.
ALT: b) There is no In and no Out until you set them.
IF-WRONG: SMALL a starting position.
ASK: NO a starting position that he moves anyway.
@@END

@@ASSUME E-37
ABOUT: R223
TEXT: I assume that when a recording reaches its end in Review, the picture stops on its last moment and stays; play starts it again from the beginning.
WHY: His older words: "It should keep playing at with the current parameters at the end." (BD:503); the page's newer line said it stops, and he left it.
ALT: b) At the end the clips keep running with the last settings until you press stop.
IF-WRONG: STAGE with the outputs showing Review, an audience sees how a recording ends.
ASK: LINE his older words and the page's line differ; he can strike it.
@@END

@@ASSUME E-38
ABOUT: D16, D18
TEXT: I assume what the app picks by chance while you record (a Random jump, an autopilot pick) is written into the recording, so that Review repeats it; where that cannot be done, Review may differ a little from the night.
WHY: The Review picture is made fresh from the recording (L80, L84); no word of his covers what the app decides by chance.
ALT: b) Chance is not recorded: Review picks again.
IF-WRONG: SMALL the small film of the night shows the difference.
ASK: NO technical; nothing is promised here before it is measured.
@@END

## QUESTIONS BACK
@@ANSWER R184
HIS: L87, L91
ANSWER: That sentence was mine, and it is withdrawn. I meant this: in my plan the button that plays a whole recording inside the live window goes away, so I looked for another way to show a whole recorded night and wrote that you could save all of it as one action and fire that. It is not needed. A recording is played in Review; with your "186 b" the outputs show what Review shows, and you hear its sound. Actions stay pieces that you cut from a recording and place on a clip, a layer or global; the save window warns you when a piece is not a multiple of 4 bars. If a recording should also play inside the live window, say so at my assumption about Record Over.
@@END

@@ANSWER lowres
HIS: L6
ANSWER: It should be light on the computer, and I would build it; but nobody has measured it yet, so every number here is an estimate. A film a quarter as wide and as high as the show (480 by 270 for full HD) at 30 pictures a second: about 1 to 5 percent of the processor and about 25 to 150 MB for each 10 minutes. Not known: whether fetching the small picture makes the live picture hitch. Only a test on your Mac with a heavy show tells; it comes before any promise. Pieces of 10 or 20 minutes are not enough alone: a film cut off by a crash cannot be played, so each piece must also be written so that a crash costs only its last seconds.
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
MEANS: The one screen where a show recording is watched, mended, recorded over and cut into actions; this paper's working word for the screen he calls Review.
SOURCE: his words L114 ("Show Recording Review should be called Review for short is a good name"): the name on screen and in talk is Review, and the keyboard-and-menus topic owns its name block and his question for a better name; "the recording review screen" is his L72
@@END

## CONFLICTS (from the paper, unruled)
- The smallest step. L82: "R135 1beat is the smallest" against L83: "the grid spacing setting: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat". Read as two lists, In and Out against Quantize: assumption E-14 (ASK: YES).
- The outputs during a review. L91: "186 b" (the outputs show the review picture, always) against L79: "use can chose to output the recording to monitor to maximize space on the screen" (a choice). Assumption E-1 (ASK: YES).
- The end of a clip recording, inside L88: "ending on a even amount" against "keep recording till it is on an even grid line like the end of the bar"; with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (BD:812-813). Assumption E-6 (ASK: YES).
- See-through parts. His "yes" that "a single-layer recording keeps its see-through parts" (BD:671) against L88: "I don't think there's any reason for a one layer recording" and "It must be the full screen composition". The newer line wins for the layer; what is left open is assumption E-9 (ASK: YES).
- Which controls get a track, inside L81: "we are not recording anything that does not move" against "was not changed during the show and was at default the entire time, does not get a row" (a control that stood away from its default is not covered). Assumption E-13 (LINE). His earlier "there is a row in the recording for every parameter and button that can be adjusted" (BD:1089) is replaced by L81; no question.
- The spacebar. His 170 words "presses the spacebar, it will stop playing" (BD:1090, the review screen) against L118: "Spacebar is typically tap tempo" (the live window). Two screens: assumption E-29 (LINE); the topic-J paper owns the live key.
- Replaced by his newer words, no question: "whole ouput of select layer" (BD:574) by L88; "It should keep playing at with the current parameters at the end." (BD:503, the end of a replay) by the review screen's own rule that playback stops on the last moment (R223 b, shown to him and not corrected).

## NOT DONE / UNSURE (from the paper, unruled)
- The cost of the low-resolution show recording (his question in L6) is not answered here: a researcher's measurement decides whether G4 is built and with which numbers (E-22).
- The screen's name (L72, L114) is the topic-J paper's; "the review screen" is used meanwhile.
- Whether the app tells him that a recording was ended by a full disk, or was saved without sound, hangs on question 209, where his "209 already dicsussed above" (L122) is applied by the paper that owns 209; R187 c leans on it.
- D21 leans on question 175 keeping its default A in the topic-A paper; if that paper finds words of his against it, R223 e and D21 need a second look.
- Where show recordings, their recorded show files and recorded clips are kept on the disk, and whether they travel with a show to another computer, is the topic-F paper's (R188); his L48 and L50 speak of presets only. Cheapest: one line on his next page if that paper leaves it open.
- That a recorded moment can be drawn fresh without touching the live show, and how fast a jump is, is unmeasured (AR T3; FT "WHAT I COULD NOT ESTABLISH" 6). It needs a measurement in the build session, before R165 and D16 are promised.
- "Today" lines were taken from the two fact sheets and not re-derived from the source, as the task allows.

## FOR THE PAGE RULING (from the ruling)
- FIVE QUESTIONS FOR HIM from this topic (ASK: YES): E-2 where a recording plays and where Record Over runs; E-6 where a clip recording ends and where its out point lands; E-9 see-through parts in a recorded clip; E-11 into which show and onto which clip an action saved in Review goes; E-21 the small film (his choice after the answer lowres).
- The paper's CONFLICTS and NOT DONE sections are carried over unruled: their "(ASK: YES)" for E-1 and E-14 and their pointers to E-22 and E-29 are overtaken. E-1, E-5 and E-14 are LINE now; E-20, E-22, E-25, E-29 are dropped.
- E-2 reaches three topics: if he picks its ALT, the live window keeps playing a recording, R165 / R184 d / R185 a change, and the keyboard and MIDI mapping must reach either Review (TEXT) or only the live window (ALT).
- Tempo pause and stop reach recordings: E-7 (a pause holds a clip recording, a stop ends it) and E-10 (Record Show runs on through pause and stop) rest on L13 and L72 as the tempo topic reads them; if that topic reads them otherwise, both move.
- The spacebar in Review stops and plays (BD:1090): asked once, by the keyboard topic's J-3. Old recordings: owned by the actions topic (its R143 and D-7); nothing older than the new build opens in Review.
- The small film: print ONE answer (lowres) and ONE assumption (E-21). The answer paper's own "decided without asking: a size choice in the Video box" and its sound question are replaced by E-21 (its own box, with sound); do not print both.
- E-9 and the codec answer belong together: if recorded clips keep see-through parts, their file type must carry them (the codec answer names HAP as able to); D22 and E-27 wait on both.
- E-11 is half the actions topic's: where an action is stored and how "the same clip" is recognised after it moved (L80: "moved or missing") must be one rule in both papers.
- "Review equals the night" is an aim, not a promise: drawing a recorded moment beside the live show is unmeasured, and chance (E-38) and signal moves without the Audio box (E-24) are notes for the build, not lines for him.
- Names fixed here: Record Show, Record to Clip (L114), recorded show file (L89), track, selected area (L89), mend (L84), Record Over (L92); texts for him say "Review" (L114). The small film has no short name yet.

