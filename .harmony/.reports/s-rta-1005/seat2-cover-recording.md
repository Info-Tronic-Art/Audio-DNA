# SEAT 2 - cover-recording, round 2 (blind seat)
Written 2026-10-05 16:45:34 by the cover-recording seat. Read-only; nothing built, run or launched; no worktree read; only this file written.
Method: the area sheet area-recording.md (parts 3, 4, 5, the CORRECTIONS block with its MISSED points M1-M8) against boris-all-items.json (questions 172-215, readings R127-R213, decided_not_asked 1-28, design_page 1-31, conflicts, area_points). Labels: VERIFIED = I read the line (file:line, or the item's words in the json); INFERRED = said from what; UNKNOWN = said how to find out.
Re-checked in source for this paper (VERIFIED by me): RecordPanelModel.h:155-215 (Record Over appears ONLY while a take with audio is replaying, tooltip "Records a new take over this take's audio, from the current playback position."); RecordPanel.cpp:70-120 (Record audio on at every launch, Play with audio, name box "Name (blank = date and time)", "Take (timelines and audio)", "Render... (coming)" greyed, "Save Routine"); MenuBarModel.cpp:143-146 ("Snapshot", "Start Recording", "Stop Recording"); MainComponent.cpp:7289-7320 (the film: H.264, 30 fps, canvas size, Recordings folder, no audio, no disk check); RecorderHost.cpp:126, 209-219, 332-335, 669 (an overdub: clock = the replayed asset, the old take's moves are replayed and not re-recorded), :786 (a take is compiled against the live show: it names none); docs/claude/recording.md:13 and :27 (an unmetered start refuses a stretch before a tempo is known); ManualWrite.cpp:68-76 (clip speed has no path); binding-decisions.md:353-356 (ruling 19 = "default on", the "not remembered" part was the panel plan's own choice), :394-402 (his "draw rather than drag"), :868-878 (the one-Save list incl. "a snapshot"), :1086-1092.

## VERDICT: FAIL (2 MUST, 10 SHOULD, 8 NIT). The list covers almost every HIS point of the area sheet; what is missing is concentrated in four places: Record Over against the removal of Play Take, what a mend does to a recording, how a one-layer recording is chosen and how a clip recording lands, and the Record-tab leftovers.

## A. COVERAGE TABLE - part 4 (HIS points H1-H31)
Covered = his answer (or silence on the reading) lets a builder go on without guessing.
| Point | Covered by | Status |
|---|---|---|
| H1 what "the video" is | decided 15 + R183(a)(c) (picture made fresh from the moves, Video box not used) | covered. NIT: decided 15 has no tag; it is mine (INFERRED), his words lean to a film ("the video will jump to that frame"); say so in the line |
| H2 live show/outputs while reviewing; where the show is on leaving | R165 + Q186 | covered for opening, outputs and closing. NOT covered: what happens at the END of the playback (old rule: hold the last look live, "Stop Playback" exits) -> F3 |
| H3 sound while reviewing, scrub | R138 (Mine: sound while it plays, silent on scrub) + R183(b) | covered |
| H4 one screen or two; opened/closed | R184(a) + design 20 (+ R165) | covered |
| H5 list of recordings; which Record-tab controls survive | R184(b)(c)(d)(e), design 23, Q215(f) | PARTLY: R184(d) names four things that go; "Play with audio", the name box, "Stop Playback", the drop-down "Take (timelines and audio)", "Show in Finder", "Takes: <folder>" are named nowhere (grep of the whole list: 0 hits for "Play with audio", "Record audio", "Takes:", "Stop Playback", "Name (blank") -> F6 |
| H6 gestures in a track; the four stops | R138 + Q187 + R185(a) + R165 | covered (spacebar: no handler in src today, so R185 is a new rule; sheet correction 5) |
| H7 which rows, folded | Q188 + R137 + decided 16, design 17 | covered |
| H8 Record Over / re-do loop | Q195 | covered as a question BUT its text misdescribes today and its trigger is removed by R184(d) -> F1 (MUST) |
| H9 three boxes: where, defaults, remembered, combinations | R183(d)(e) + design 23 | covered; NIT: Parameters off + Audio on (a sound-only recording): has it an entry in the list? only "Video only" is answered |
| H10 menu items Start/Stop Recording, key action | R183(f), Q206(A "Record") | covered |
| H11 key/pad for Record, Snapshot, REC | Q206 | covered (NIT: answers B and C do not say whether Record/Snapshot/REC are in; A names "Record, Snapshot" but not the clip REC) |
| H12 REC waits for the bar | R186(c) | covered |
| H13 length vs multiples of 4 | R186(c) (out point moved in to the last full group of 4) | PARTLY: a clip of 1-3 bars (REC on bar lines makes them) is not covered -> F5 |
| H14 landed clip BPM Sync or Timeline | R186(c)(d) | covered |
| H15 REC while beat stopped/paused | R186(d) + decided 1 | covered |
| H16 "prominently displayed" | design 22 | covered (a picture item, correct) |
| H17 live cell picture vs preview monitor | R186(a), his Q3 "yes", decided 6 | covered |
| H18 what a one-layer recording holds; how the layer is chosen | R186(e) (before fader and blend) | PARTLY: how the layer is chosen is nowhere; (e) sits against his words "exactly as it's displayed" and the list itself flags it (still_unsure 15) -> F4 |
| H19 "exactly like the output": which | R187(d) | covered |
| H20 disk rules | R187(c) + Q209 | covered, one hole: does the stop-by-itself show the disk sentence? -> F9 |
| H21 where a "not saved" message appears, which failures | Q209 + design 31 + conflicts 17/23 | covered; a film (Video box) that fails is not named -> F9 |
| H22 "a snapshot" in the one-Save list | R188(f) | PARTLY: R188(f) says snapshots are files of their own but never says what a snapshot is (a PNG of the output picture, not the whole screen) nor that it makes no sign on screen -> F7 |
| H23 where recordings live; Collect Media | R188(f) | covered |
| H24 what a take is tied to | R173 | covered. NIT: a recording made in a show never saved has no name to remember |
| H25 no tempo / tempo changes inside a recording | decided 21 (only "if you answer 175 A"), R185(c) | PARTLY: 175 B has no fallback; an action cut over a tempo change has no rule -> F11 |
| H26 tempo row in a recording | R185(b), R167, R138 | covered |
| H27 words: take / recording / review / routine | R197(a)(b), R184 | covered. NIT: "Recordings" is also the folder that holds the Video-box films (today's folder name), but the list of recordings excludes films (R183e) |
| H28 Render | Q215(f), R184(d) | covered (BD:220-224 cited in Q215's source) |
| H29 several recordings at once | decided 20 | covered. NIT: "you are told what is given up" reads as a message by the app against Q209; say "I tell you before it is built" |
| H30 a whole take as an action | R184(d) | covered |
| H31 where the clip lands | R186(b), conflict 20 | PARTLY: not said whether the clip plays by itself when it lands (plan: it waits, "ready to trigger"), which deck (at the press or at the end), and what a full row does (plan: a new column for the whole deck) -> F5 |

## B. COVERAGE TABLE - the MISSED points M1-M8 of the CORRECTIONS block
| Point | Covered by | Status |
|---|---|---|
| M1 actions during a recording / review | decided 17 (a recording keeps what every control did, whoever moved it), R181(e), R185(a) (keys fire nothing in review) | covered. NIT: say in decided 17 whether the on/off press of an action is itself a row (the sheet's choice a/b/c) |
| M2 a recording saved but flawed | Q209 option C | covered |
| M3 sound in film and clip | R183(e), R186(f) | covered. NIT: neither says today's film has no sound (VERIFIED: MainComponent.cpp:7298-7310, no audio stream) |
| M4 quit while recording | R187(a) | covered. NIT: "no extra window" collides with the quit window of an unsaved show (R-quit); say which wins |
| M5 open/replace a show during a recording | R187(b) | covered for open and New; NOT for deleting a deck/clip or opening the review screen while a recording runs -> F10 |
| M6 source = Audio File | decided 19 | covered |
| M7 onset/beat marks on the audio track | area_points says "dropped: ... part of the review screen's design item" | NONE: design 16-19 never name what the audio track shows -> F12 |
| M8 undo and spacebar in the review screen | R185(a) | covered for undo of edits; the question of what an edit IS (kept or dropped on closing) is F2 |

## C. COVERAGE TABLE - part 3 OVERTAKEN (O1-O13) and part 5
| Point | Covered by | Status |
|---|---|---|
| O1 old replay model | R165, R138, Q186; hold-at-end gap | partly (F3) |
| O2 audio-as-anchor vs "audio is gone" | Q195, 171 A (take and sound stay), conflict 19 | covered (F1 for the text) |
| O3 automatic end | R186(a) | covered |
| O4 see-through | R186(a) | covered |
| O5 text-only REC display | R186(a), design 22 | covered |
| O6 Quantize decides the clip start | R128(b), R186(c), R175 | covered |
| O7 notices lane vs his messages | Q209, conflict 23 | covered; NIT: no line tells him the Record tab's status, yellow and cyan lines go (today VERIFIED RecordPanel.cpp:105-115) |
| O8 routines section, Save Routine row | decided 12, R184(d), R197(a), R143 | covered |
| O9 two boxes vs three | R183 | covered |
| O10 landing cell | R186(b) | covered (F5 for details) |
| O11 old recordings | told_in_chat R116, R143, decided 12 | covered |
| O12 events first, video later | R183(c), Q215(f) | covered |
| O13 REC button's home | design 22, R153 | covered |
| part 5: 154 | R128, Q173 | covered |
| part 5: 170 | R138 (told now), Q186, Q187 | covered |
| part 5: 36 (unsaved show) | R188(f), R173 | covered (NIT as H24) |
| part 5: 37 and 34 | design 31; defaults stand | covered. NIT: say that 34 (the "Save failed" box) stands, so design 31 does not seem to reopen it |
| part 5: 35, ruling-notices Q1b-Q5 | Q209, conflict 17/23 | covered |

## D. TECHNICAL points T1-T11 (architects): decided lines owed only where he would notice
| T1 codec/container | NONE: no decided line (grep codec, H.264, mp4, GB in the whole list: 0 hits). He said "pick the most optimal codec ... I want codecs that decode easily and play well" (BD:650-651), and "a lot of hard drive space" (BD:851) | F8 |
| T2 film speed/size today | NONE (INFERRED today: the film may play too fast/slow: stamps 1/30 s on every render frame, VideoRecorder.cpp:102-141, 385; not run) | F8 |
| T3 picture at any moment | not_established 4 + decided 15 | covered (UNKNOWN stays flagged) |
| T4 what the recorder can address | decided 16, Q193 | covered |
| T5 disk behaviour during recording | R187(c) says amounts are the architects' | covered for him; F8 adds the size consequence |
| T6-T11 | architects / process | no line owed |

## E. THE LIST'S "TODAY" CLAIMS in this area, against the sheet's part 1 and my re-reads
- R165, Q186 "today a recording is played back through the live layers/outputs": VERIFIED right (RecorderHost.cpp:786-838 via the sheet).
- R173 "today a recording names no show": VERIFIED (RecorderHost.cpp:786 compiles against the live show).
- R183(f), R184(d), R148/R197 strings "Start Recording", "Stop Recording", "Play Take", "Load Take...", "Save Routine", "Render... (coming)": VERIFIED (MenuBarModel.cpp:145-146; RecordPanel.cpp:98, 123).
- decided 16 (more than half of a clip's controls cannot be kept): consistent with ManualWrite.cpp:68-76 (speed has no path) and FT section 2; INFERRED for the exact fraction.
- Q193 "today a recording cannot keep Speed, the blend list, the fade": Speed VERIFIED; blend and fade INFERRED from the sheet (T4).
- Q195 "it is built and works today" and its description of Record Over: CONTRADICTS the code (see F1): MUST.
- decided 21 "today a recording started without a tempo cannot be cut": OVERSTATED: only a stretch before a tempo is known is refused (recording.md:13, :27). NIT.
- decided 24 "The Resolution cannot be changed while a recording runs": no such guard in src (grep videoRecorder_/recorderHost_ near resolution/canvas: nothing). It is a new rule, not today. NIT: say "will not".
- Q209 "(today the recording ones are a line in the Record tab ...)": today only a take's faults show there; the film and snapshot are silent (VERIFIED MainComponent.cpp:7289-7320 no message). NIT.
- R183(d) "the app remembers your ticks": today "Record audio" is NOT remembered (RecordPanel.cpp:78-80); that came from the panel plan, not from him (his ruling 19 says "default on"). Not a contradiction; NIT: say "today it is back on at every launch".

## F. FINDINGS (exact new wording)
F1 MUST - Q195 and R184(d). Q195's A says the recording's "sound plays as the music"; today Record Over (VERIFIED RecordPanelModel.h:174-178, RecorderHost.cpp:209-219, :669) appears only while a recording with sound is playing, starts from the place you are at, and the old moves go on playing in the live window while your new ones are recorded. R184(d) removes "Play Take", the only way to reach it, and R165 stops the live show whenever the review screen is open. A builder cannot place the button. NEW wording of Q195:
 Situation: "You have a recording of a night, with its sound. Today you play it and, at a moment you choose, press Record Over: the old moves keep playing in your live window and your new moves are recorded on top, to the same sound, as a new recording beside the first."
 A (default, [your words] September: "we could reuse the same audio that's locked to the files so the user can redo it"): "Record Over stays. In the list of recordings each recording has a Record Over button: it plays the recording's sound and its old moves in the live window, from the start or from a place you pick, and what you move is kept as a new recording beside the first, with the same sound. It is not part of the review screen."
 B: "Record Over goes. You mend a recording in the review screen (167 A), or you record a new night with new sound."
 Why: "Without Record Over there is one way less to re-do a night; with it, the list needs one more button."
F2 MUST - NEW question 216 (E): "what a mend does to a recording". Situation: "In the review screen you draw a new shape over a slider's curve, move a piece, or use Quantize on the button presses, then you close the screen." A (default, [your words + mine]: you called Quantize "cleaning up the recording afterwards"): "The recording itself is changed and kept when you close the screen. A button 'Back to as recorded' brings the night as it was back (it is always kept, your 'never delete')." B: "Nothing of a mend is kept in the recording; a mend only counts in the actions you save from it, and closing the screen drops it." C: "A mend is saved as a new recording beside the first ('name 2'); the first never changes." Why: "It decides whether the list of recordings fills with copies (C), whether the night can be spoiled by a slip (A is safe only with the button), or whether a mend can only ever live inside an action (B). decided 18 says the original is kept but not what you press to get there." Also add to decided 18 the sentence "The recording as it was made is the one thing the 'Back to as recorded' button restores" only if A is answered.
F3 SHOULD - NEW reading (E): "Mine: when the review plays to the end of the recording it stops there on the last moment and stays there; play starts again from the beginning. Playing never carries on into the live show. This replaces today's rule that a replay at its end holds the last look in the live window until you press Stop Playback." (today VERIFIED RM:11, RecordPanelModel.h:188 tooltip "Ends the replay. The look stays as it is.")
F4 SHOULD - NEW question 217 (E), the one-layer recording. Situation: "You press REC with 'Layer' to record one layer, then play the clip it makes on another layer whose fader is at 50 percent and whose blend is Add." A (default, [your words] BD:626-627 "it should look exactly as it's displayed"): "The recording is the layer exactly as it shows in the output, its fader, blend and keying included. Played on another layer those are applied a second time, so it looks darker or different." B: "The recording is the layer's clip and effects only, without its fader and blend; the layer you play it on gives it its own. It does not look like the output it was recorded from." Add a line: which layer: "the layer you have selected in the layer strip" (a) or "the layers switched on under the preview monitor" (b) or "you pick it in a menu next to REC" (c). Why: "It decides whether a recorded layer looks like what you saw or like a clean building block." R186(e) then reads "(e) A one-layer recording holds the layer chosen by question 217."
F5 SHOULD - R186 gets three more sentences (Mine, INFERRED; he corrects if wrong): "(g) A clip recording is never shorter than 4 bars: if you press REC the second time before the end of bar 4, it ends on the '1' that closes bar 4. (h) When it lands it waits in its cell, ready for you to fire; it does not start playing by itself. (i) It lands in the deck on screen at the moment you press REC; if the top layer's row has no empty cell, a new column is added to the deck for all its layers."
F6 SHOULD - R184 gets (f): "On the Record tab these stay: the Record button (it reads 'Record'; 'Stop Recording' black on red while it runs), the name box ('Name (blank = date and time)') and the three boxes of R183. These go with 'Play Take': the tick 'Play with audio' (the review screen plays the sound by itself, R138), 'Stop Playback', the drop-down 'Take (timelines and audio)', the label 'Takes: <folder>'; 'Show in Finder' and 'Repair Audio' move to the list (R184 b, e)."
F7 SHOULD - R188(f) gets after "snapshots": "Snapshot: your One-Save list says 'a snapshot - a still picture of the screen'. I read it as: Snapshot stays its own command and is not kept inside the show. It saves a picture file (PNG) of what the output monitor shows, today under Output > Snapshot into the Snapshots folder, and nothing appears on screen when it is taken or fails. Tell me if you meant that the show keeps its snapshots."
F8 SHOULD - NEW decided line (E): "The app chooses the file types for the film and for a recorded clip itself, for easy playback (your 'I want codecs that decode easily and play well'): the film opens in ordinary players, the recorded clip keeps its see-through parts. Consequence you would notice: both are big, a recorded clip of a one-hour show runs to tens of gigabytes at full HD (INFERRED arithmetic, about 23 GB an hour); and today's film may play at the wrong speed (INFERRED, not run), which is checked before the Video box is built." by: BD:650-651, facts-record-codec.md section 3, VideoRecorder.cpp:102-141, 385.
F9 SHOULD - Q209 option A, last sentence of the first list: replace "Those four stay the only messages that appear by themselves." with "Those four stay the only messages that appear by themselves: a failed save; a clip recording that was not saved; a show recording that was not saved (the film of the Video box counts, and so does a recording that ends by itself because the disk ran low); and 'not enough HDD space to record' when you press Record." Trim the situation's first sentence (the paste of an action belongs to question 184).
F10 SHOULD - NEW reading (E): "Mine: while a recording runs, the way into the list's review screen and Record Over are greyed (opening the review screen stops the live show, R165). If you delete the deck or the clip that a clip recording is going to land in, the recording still lands in the deck on screen. Opening another show or New: R187(b)."
F11 SHOULD - NEW reading (E), tempo: "Mine: an action cut from a stretch of a recording in which the tempo changed takes the tempo at its In point and plays at that tempo. With 175 B, a recording made before any tempo was known shows seconds on its grid and cannot be cut into an action until you type a tempo for it." (decided 21 stays as it is for 175 A.)
F12 SHOULD - design 16 gets a second line: "what the audio track shows": variants "the waveform alone", "the waveform with a mark on every bar line of the night's tempo", "the waveform with a mark at every drum hit the app heard" (the last needs the recording to keep them: onset marks are an option of the recorder today, set by no screen).
NITs (all in the tables above): decided 15 tag; R183(d) today wording; a sound-only recording's entry; Q206 B/C and the clip REC; a show never saved (R173); decided 17 action press; film sound today; quit window vs R187(a); "Recordings" name collision (film folder vs list); decided 20 "told"; decided 21 and decided 24 today wording; Q209 today line; no line on the Record tab status/warning/notice lines going; 34 stands.
