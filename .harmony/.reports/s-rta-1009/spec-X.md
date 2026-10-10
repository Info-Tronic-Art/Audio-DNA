# SPEC X -- The loose lists (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-X.md, ruling rule-X.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM C1
TITLE: Show start: his stopped/paused sentence against his show-start sentence
STATUS: SETTLED
HIS: L30, L12
RULE: Several clips are started together by a click on a column trigger: "One is to click the column trigger, and that plays them at the same time." and "The only way to do this is with a column trigger. Moving on." (L30). No paused start is built: nothing makes a fired clip stand ready on its first frame until play. What he calls "a waste of time" in L30 is another way, which he describes there: "to trigger all the clips, press, pause, then drag each play head to the beginning manually"; nothing is built to make that way easier. With the tempo stopped, the click is the new 1 and the clips of the column start together: "When stopped, user’s click on play or any clip is the new 1." (L12); that a click on a column trigger counts as such a click is INFERRED (it fires clips). With the tempo running or paused each clip of the column follows its own rule (C22); what a fire during a pause does is C2.
CHANGED: Question 172: no letter taken; his words name the column trigger alone. Option A (pause first; each fired clip shows on its first frame and waits; play is the 1) is not built. What option B says of a stopped tempo (the first clip fired starts the beat and plays) is his L12. Reading R140's "load up the clips = firing them while the beat does not run, so that they stand ready" falls away. Mended by this ruling: the paper tied "a waste of time" to the pause-first way of option A; L30 says it of firing, pausing and dragging each play head back.
TODAY: facts-takes-bpm-fire.md 5.7-5.11 as cited on the page (not re-read): the app has no stopped or paused beat; the tempo bar's stop, pause and play are new work.
@@END

@@ITEM C2
TITLE: Paused fire: his R111 sentence against 136 b
STATUS: SETTLED with two lines for him (X-1, X-2) -- AMENDED after page 2 (C2 1 by X)
HIS: L31, L12, L13, L17, L11
RULE: With the tempo paused: (1) a BPM-mode clip that he fires waits for the 1 and then starts ("There is nothing in the layer. The tempo is paused. I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires.", L31; "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one.", L12). (2) The fire itself sets the beat clock going again from the position it was holding at, as a press on tempo play does: "that press starts the clock from when the user clicks play or the clip" and "the user started the clock from the position it was holding at" (L13); his Arena does the same (L17). That every fire does this, whatever the clip's mode, is X-1. (3) Until that 1 the layer keeps what it had: nothing on an empty layer, or the clip that was there, which plays on (the default of question 173, taken by L1; L31 fits it and does not say that the waiting clip stands on its first frame). (4) A clip that was playing when pause was pressed stays shown, held on its frame, and plays on from there when the clock runs again: "it will display the paused clip and when I press tempo play it will play from where it was paused and the clock will continue from where it was paused as well" (L31). (5) Pause holds "the beat clock, the clips and everything that it controls with BPM" (L13). A clip that is not in BPM mode is not held and starts on his press when fired: his earlier 136 b and "If there are clips that are not BPM based, then they play just as they were and are unaffected" (binding-decisions.md:918-920); the second sentence of L11 fits it (INFERRED: it names a layer); X-2. With the tempo stopped a fire plays at once and is the new 1 (L12; "The only difference is if I was stopped, it would play the moment I triggered it and I would be setting a new 1 for the tempo.", L31). So 136 b stands, and his earlier "When paused, it will not play but will still display." holds for the clip that was playing when pause was pressed (L31 opens "let me clarify this"). A click on an empty cell is not such a fire: it never starts the beat. During a pause the layer's BPM-mode clip stays, held, and the layer goes empty on the first 1 after the tempo runs again (item 248). [page 2, X: item 248 accepted as written]
CHANGED: Question 174's default A (no video fired during a pause moves, whatever its mode) is replaced by his words. Reading R139 (c) and (d) (a fired clip shows and stands on its first frame during a pause; only play runs the beat on) are replaced: a fired BPM-mode clip waits without being shown (question 173 A, INFERRED for the pause), and the fire itself sets the clock going (L13). Mended by this ruling: "shows nothing" stood in front of the L31 quote as if it were his word, and the restart of the clock was held as an open edge although L13 says it.
TODAY: facts-takes-bpm-fire.md 5.9, S8 as cited on the page (not re-read): no paused beat exists in the app.
@@END

@@ITEM C3
TITLE: Preset button: his name answer against the nameless Resolume P
STATUS: SETTLED
HIS: L52
RULE: The effect's header line carries, on the same line as the effect's name, the name of the preset the effect is set to. When the preset is changed, the name is taken away and a Save button stands in its place: "I would like to have the presets name in the same header line as the effects name. If the preset gets changed then remove the preset name and replace it with save button" (L52). Resolume's nameless P is not copied. "Changed" is read as in the reading on the small Save button, which he did not name and which stands: the effect's sliders no longer match its preset. The Save button goes away when they match again, and the name is then back in its place (INFERRED from that reading with L52; X-26). An effect that never had a preset reads "Default" and shows the Save button once it leaves its defaults (the same reading, part b). While the Save button stands in the name's place the list of the effect's presets stays reachable (X-26). How the app knows which preset to name is ruled in the presets topic.
CHANGED: Question 178: none of A, B, C as written. A's "the name stays after you change a slider" is replaced (the name goes, the Save button comes); A's "The effect remembers which preset it carries" is not touched by his words; B and C (a "P" without a name) fall away. Added by this ruling: what "changed" means, that the name comes back, and that the list stays reachable (all three were unnamed in the paper).
TODAY: facts-presets-delta.md U1, S1 as cited on the page (not re-read).
@@END

@@ITEM C4
TITLE: His 133 (a change is not shown) against the small Save button
STATUS: SETTLED
HIS: L52
RULE: A changed preset IS shown: the preset's name leaves the header line and a Save button takes its place (L52). His earlier "no need to show that it was changed" (133) is replaced by this later word of his for the header line. When the Save button shows and when it goes follows the reading on the small Save button, parts (a) and (b), which he did not name and which stand: it shows the moment the effect's sliders no longer match its preset, also on an effect that reads "Default", and it goes away when they match again (see C3).
CHANGED: Reading R127 (c) ("the preset's name itself still stays after a change and gets no mark") is replaced by L52. R127 (a), (b), (d) and (e) (when the button shows and goes; the name window, the next free name, an existing name asks first) are not touched and stand.
TODAY: not checked
@@END

@@ITEM C5
TITLE: Action off goes back (159 b) against last-on wins (160 a)
STATUS: SETTLED with one question for him (X-27)
HIS: L66, L64, L67, L70
RULE: He named the reading and corrected only its cases 2 and 4 (L66), so case 1 stands: while another action that is still on moves the same slider, that one has the slider; the slider goes back to where it was before any action only when the last such action is off. Going back is a glide: "whenever an action is switched off and it goes back, it follows the master action fade back time" (L67), over the time set by "the Global glide slider", "from instant to four seconds max" (L64). A start glides too wherever it would make a jump: when an action starts, when one action takes a slider over from another or hands it back, and at a Resync, because he wants "a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (L70). That this fade and the glide back are ONE slider is INFERRED from "We will just keep the Global glide slider" (L64) and is asked (X-27).
CHANGED: Reading R129: case 1 stands; "a jump or a glide: question 213" is answered (a glide: L64, L67, and 213 b at L77). Added by this ruling from L70: the glide at a start, at a take-over and at a Resync; the paper called the glide time "the one global glide time" as if that were his word.
TODAY: facts-actions-open.md P1, P3 as cited on the page (not re-read).
@@END

@@ITEM C6
TITLE: A layer action under a global action: off or held back
STATUS: DROPPED the reading it was handled as stands (not named by him)
HIS: L1
RULE: While a global action is on, a layer's actions are held back, not switched off: their buttons stay on and they come back in step when the global action goes off. He went through the page and did not name this reading, while he corrected its neighbours (L64, L66, L67); nothing in his message says otherwise. A layer can be taken out of this by its own switch (L73, L7; see X-3).
CHANGED: nothing
TODAY: facts-actions-open.md P2, U6 as cited on the page (not re-read).
@@END

@@ITEM C7
TITLE: Hand on a slider an action moves: catches up against goes back
STATUS: SETTLED
HIS: L66, L64, L67
RULE: You can take hold of a slider that an action is moving; when you let go, "the slider moves back to the action position based on the global glide back setting I described earlier." (L66): it glides to where the action is by then, over the time of the Global glide slider ("from instant to four seconds max", L64). When the action goes off or has played once, the slider glides back to its position from before the action (L64, L67). The ignore lamp stays the way to keep your own value. That all these glides share ONE slider is X-27.
CHANGED: Reading R129 case 2: "when you let go it jumps to the action (158 A)" is replaced by a glide over the Global glide time. The accepted default 158 A (a jump) is replaced by his later words.
TODAY: not checked
@@END

@@ITEM C8
TITLE: His restore (said for routines) against 159 b
STATUS: SETTLED
HIS: L64, L67, L70
RULE: There is no separate restore switch. Each action has a loop toggle where it is placed: "Each action where it is placed, will have a loop toggle right there." (L64). An action that is not set to loop plays once and its controls then "go back to the position they were at before they played" (L64); an action switched off does the same (L67); both glide over the time of the Global glide slider. When an action starts, a control that would jump to the action's first value glides there instead: "a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (L70). One slider for both is X-27.
CHANGED: Reading R172: its last part stands (no restore switch); added by him: the loop toggle on every action and the Global glide slider (L64). Replaced by L70: "a control that was somewhere else jumps there on the 1" and "A global action is the exception" (every action's start glides where it would jump).
TODAY: facts-actions-open.md P15 as cited on the page (not re-read).
@@END

@@ITEM C9
TITLE: Clip and layer actions saved apart against global reaching all layers
STATUS: SETTLED with one question for him (X-3) -- AMENDED after page 2 (C9 1 by X)
HIS: L55, L73, L7
RULE: Both hold. When the rows picked are of more than one kind, "the app breaks them down into clean, concise actions that can be saved in the action save window" (L55): a clip's rows become that clip's action, one layer's rows a layer action, and rows of the Global tab or of more than one layer a global action. That rows of two layers become ONE global action (option A of question 179) is INFERRED: his line opens "179 a layer can have a global action bypass", which names no letter for certain; the actions topic carries it as its line D-8. A global action reaches every layer; a layer can be taken out of that: "a layer can have a global action bypass in the same way that I can have column trigger bypass" (L73). That bypass and the "ignore actions toggle on the layer" (L7) are ONE switch on the layer, of the same kind as Ignore Column Trigger: while it is on, global actions leave that layer alone; the layer's own actions and its clips' actions keep playing (item 225). [page 2, X: item 225 accepted as written]
CHANGED: Reading R133 (e) confirmed ("E yes, this is a good idea."); (d): the tab is called Global (L55). Question 179 A gains the per-layer bypass.
TODAY: facts-actions-open.md P14 as cited on the page (not re-read).
@@END

@@ITEM C10
TITLE: Review: a press stops, a click also jumps
STATUS: SETTLED he named the reading and left this part as written
HIS: L79
RULE: In Review, while it plays, the stop button, the spacebar or a mouse press inside a track stops playback and changes nothing else; a click in a track also moves picture and timeline to that moment; a drag scrubs both. His L79 adds only where the picture can go (to a monitor, or a draggable line between tracks and picture).
CHANGED: nothing in this part; R138 gains the two display choices of L79.
TODAY: not checked
@@END

@@ITEM C11
TITLE: Scrub by dragging against drawing in a track
STATUS: DROPPED question 187 keeps its default A (not named)
HIS: L1, L92, L86
RULE: Two tools in Review: the pointer (click = jump, drag = scrub) and the pencil (drag = draw; select a piece and move it); the pointer is on when the screen opens. His message speaks of both as existing: "the scrubbing and drawing with mouse" (L92). Sound is not scrubbed when the setting for it is off (L86).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM C12
TITLE: Actions made only in Review against copy, paste, delete
STATUS: SETTLED
HIS: L72, L68, L75
RULE: An action is made (recorded) only in Review: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72). Copy, cut, paste and delete of a finished action stay. A paste onto a clip that lacks a parameter or an effect: "that specific action is muted, and the small messages is displayed where the user needs to say OK they understand." (L68). Whether the whole pasted action is muted or only the part that does not fit is asked once, in the actions topic (its D-6).
CHANGED: Reading R162 stands; question 184 is answered by L68, to which his L75 points ("184 read my note on this above"); none of its letters.
TODAY: not checked
@@END

@@ITEM C13
TITLE: Length row of a BPM-mode clip: bars against beats
STATUS: SETTLED for the row; the first number of a new clip is asked in the clip topic (H-5) -- AMENDED after page 2 (C13 1 by H, C13 1 by X)
HIS: L99, L21
RULE: The row reads and steps in beats: "R218 do beats here. It was my mistake before" (L99). Plus and minus change the length by one beat; /2 and x2 halve and double it (his Arena does the same, L21). The length a NEW clip in BPM mode starts with is settled by his later answer "b" (BF258): its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed, and the row then reads that length in beats. What stood before that answer: [page 2, X: BF258 "b"] the reading's part on it (whole groups of 4 bars, the out point moved in; his rule "always work with multiples of 4", binding-decisions.md:812) was not touched by him, while his look in his Arena reports "a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode" (L21). A look is not a rule; the two were put to him as item 238, and he chose his own rule (BF258: "b"): whole groups of 4 bars, the clip out point moved in, exactly normal speed. The rule in full is item R218 (b). [page 2, H: BF258 "b"]
CHANGED: Reading R218 (a) is confirmed; his earlier "lets do bars here not beats" is withdrawn by him for this row. R218 (b) (the first guess) is neither confirmed nor replaced: his L21 look bears on it.
TODAY: Clip.h:36-41 by the cover-clip-transport seat as cited on the page (not re-read): every new clip starts at 4 beats.
@@END

@@ITEM C14
TITLE: His hand against the app's listening: what Manual means
STATUS: OPEN he asked a question back (L32); the answer is owed to him first -- AMENDED after page 2 (C14 1 by X, C14 2 by X)
HIS: L32, L14, L13
RULE: A correction he makes by hand while the app listens by itself never switches Manual on: the app stays in automatic mode. The tempo: a number he corrects stays close to his number, following only small drifts, until the music clearly changes tempo (a new track that is not beat-matched); then the app follows the music again (item 217). The 1: the app always tries to find the 1 by itself, and he corrects it with Resync when it sits wrong: "b. app should always try to find the 1 and I will correct if necessary" (BF246); "we should be able to resync the 1 if the detection is finidng the correct bpm but not the correct 1" (BF242). So in automatic mode the press that starts the beat is the 1 at that moment and no more: the app may place the 1 by itself afterwards (way b of item 218, as the tempo topic rules it). How long his Resync holds against the app's own search is not in his words: the tempo topic puts it to him, and nothing here settles it. He had asked: [page 2, X: item 217 accepted as written; BF246 "app should always try to find the 1 and I will correct"; BF242 "we should be able to resync the 1"] "is it possible to correct the app listening to the music if it is off by two BPM's and keep it on automatic mode, or will that correction necessitate going back to manual mode? This is a question for you." and "Same question if I correct, where the one is." (L32). The answer was given to him on page 2 (probably yes, not proven: it is tested on the DJ tracks he brings, and he is told before anything is built if a correction would need manual mode); with it he took the way in which a correction never switches Manual on. [page 2, X: item 217 accepted as written; BF246 "app should always try to find the 1"] Fixed already: /2, x2 and a tempo change never move the 1 (L13); the nudge amount is shown only in automatic mode (L14); a tap alone starts nothing (128, earlier).
CHANGED: Question 189: no letter taken; its default A (any hand correction switches Manual on) is NOT taken by L1, because he named the question and asked back.
TODAY: area-tempo.md U-HIS-4, O-17, O-20, M-6 as cited on the page (not re-read): with Manual off the app puts the beat back by itself and replaces a tapped tempo after about two seconds.
@@END

@@ITEM C15
TITLE: Show keeps the nudge amount against Resync sets it to 0
STATUS: STANDS the reading it was handled as (not named by him); L14 adds where the amount shows -- AMENDED after page 2 (C15 1 by X, C15 2 by X)
HIS: L14, L13
RULE: The show keeps its nudge amount and a Resync sets the amount to 0 (his earlier 63 and 49), so a nudge saved with a show counts only when that show is opened while the beat is running, and then lasts until his next Resync or his next start after a pause or a stop (item 219). His earlier answer that the show keeps the amount stands only in this narrowed sense; the narrowing is Harmony's reading of item 219 beside that answer, not his word (X-5; the tempo topic carries the same line) [page 2, X: item 219 accepted as written]. New from him: a nudge "moves the one a hair forward or back" (L13), and the amount is shown only in automatic mode: "We only need to display the nudge XMS in automatic mode. In manual mode, this is not necessary because the user will nudge it to move it to where they want." (L14). In manual mode no amount is shown (L14); that none is counted or kept there, so that the amount the show keeps is the automatic-mode one, is Harmony's reading (X-5). What "that nudge is history" (L13) means for the amount at a start from pause is carried by the tempo topic as a line of its own; it is settled by item 219: when he starts the beat again after a pause or a stop, the nudge is history and the nudge number shown in automatic mode goes back to 0; a Resync sets it to 0 as well. Pausing or stopping by itself does not change the number. [page 2, X: item 219 accepted as written]
CHANGED: Reading R177 (not named) stands; L14 adds that the amount is SHOWN only in automatic mode. Mended by this ruling: the paper wrote "in manual mode there is no amount to keep (L14)"; L14 speaks of the display only.
TODAY: area-tempo.md U-HIS-6 as cited on the page (not re-read).
@@END

@@ITEM C16
TITLE: Fire between two 1s: starts at once (123 b) against waits (154)
STATUS: SETTLED -- AMENDED after page 2 (C16 1 by X)
HIS: L12, L31, L11
RULE: A BPM-mode clip fired while the tempo runs or is paused waits for the 1: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one." (L12). A Resync press is the 1, so a waiting clip starts on it (INFERRED from "unless resync is clicked"). With the tempo stopped it plays at once and that press is the new 1 (L12, L31). A clip that is not in BPM mode starts on his press: that is the reading on his 154, which he did not name and which stands; the second sentence of L11 fits it (INFERRED: it names a layer; see X-2). 123 b no longer holds for firing a BPM-mode clip. One exception to the wait: a BPM-mode clip triggered just after the 1, within a tenth of a beat, still starts at once, in step with the music, and does not wait a whole bar for the next 1 (item 249). [page 2, X: item 249 accepted as written]
CHANGED: Reading R175 (1) confirmed by his new words.
TODAY: not checked
@@END

@@ITEM C17
TITLE: Cmd+Z never touches the layer strip against the old R113
STATUS: DROPPED the reading it was handled as stands (not named by him)
HIS: L1
RULE: His sentence wins: Cmd+Z undoes nothing of the layer strip (play, play reverse, pause, transparency, bypass, solo) and no fire; it undoes what is built or removed (a clip, a layer, a column, a deck, an effect, an action, a preset load). Nothing in his message of 2026-10-07 speaks of Undo.
CHANGED: nothing
TODAY: area-cue-layers.md O5, W4, W5 as cited on the page (MainComponent.cpp:723-768, 5029-5039; not re-read): the app still undoes a fire, B, S and X.
@@END

@@ITEM C18
TITLE: "Nothing else" against messages for a paste and a recording's show
STATUS: SETTLED one line for him (X-6) -- AMENDED after page 2 (C18 1 by X)
HIS: L122, L68, L65, L80
RULE: "209 already dicsussed above" (L122): the places where he himself now asks the app to say something are allowed, on top of the four that stood (a failed save; a clip recording that was not saved; a show recording that was not saved; not enough disk space to record). The three he named: after a paste that lacks a parameter or an effect, a small message "where the user needs to say OK they understand" (L68); "show a warning in save screen if the action is not a multiple of 4 bars" (L65); a recording opened against a changed show "indicates this to the user that these clips have been moved or missing" (L80). One more message comes from his answers to page 2: when he saves an action in Studio and its clip is gone, the app tells him and nothing is saved (item 230; the recordings topic rules the neighbouring cases). And one more question: when he opens a show and that show and the computer hold two different versions of one preset, a small window asks which one to keep (BF249, way b of item 223). Like the other questions the app asks before something is replaced or closed (the quit window, saving over a name, importing a mapping file that replaces the show's mapping: item 269) it is not a message and was never counted in this list; it is named here because it comes at the opening of a show, without a command of his that names presets. No other message appears by itself. [page 2, X: BF249 "b"; item 230 accepted as written]
CHANGED: Question 209: answered in words; it comes out as A plus his three places; B and C fall away.
TODAY: area-recording.md H21, M2 as cited on the page (not re-read).
@@END

@@ITEM C19
TITLE: Syphon as an output against a switch of its own
STATUS: DEFAULT question 200 keeps its default A (not named); one line for him on the new edge (X-7) -- AMENDED after page 2 (C19 1 by X)
HIS: L1, L96
RULE: Syphon is one more line in the Outputs list with its own tick and the same eight settings as a screen; all outputs off switches it off too; other programs see the app's Syphon picture only while it is ticked; Restore Last Outputs brings it back with the screens. New from L96: "if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." So a show remembers its outputs, and Syphon with them: a show saved with Syphon on switches it on again when that show is opened (item 263). As a line of the Outputs list Syphon follows the two other rules for outputs: opening a show never switches it off when it is already on (item 237), and after All Outputs Off it stays off, also when a show is opened afterwards, until he switches an output on himself (BF269, way b of item 265). [page 2, X: item 263 accepted as written; item 237 accepted as written; BF269 "b"]
CHANGED: Question 200 A: only the sentence "It is off at every launch" is touched, by L96 for screens and, by X-7, for Syphon. "Restore Last Outputs brings it back with the screens" stands.
TODAY: area-outputs.md H5, H6, M7 as cited on the page (not re-read): Syphon has a switch of its own outside the list and survives all outputs off.
@@END

@@ITEM C20
TITLE: Redo to the same audio against an action without audio
STATUS: SETTLED except the screen it runs on, which is one question for him (X-8) -- AMENDED after page 2 (C20 1 by X)
HIS: L92
RULE: Both hold, for two different things. A recording keeps its sound and can be recorded over: "195 yes we can record over using the midi controller. This record over mode is different than the scrubbing and drawing with mouse as this plays in real time. Record overs are recorded separately from the original" (L92). The recording plays with its sound and its recorded moves, and a control he moves takes over from its recorded track while he moves it: his earlier words "The knob will supersede whatever is happening, And will snap back to the recorded track as soon as it is let go." (binding-decisions.md:340-341). An action made from a recording carries no sound (his earlier words, unchanged). Record Over is a button in Studio: the show recording plays there in real time with its sound, and what he moves on the MIDI controller is recorded over it (item 229); it does not run in the live window through the live layers. It takes everything in the keyboard and MIDI mapping, clip triggers too; what he moves with the mouse is not recorded over (item 257). [page 2, X: item 229 accepted as written; item 257 accepted as written]
CHANGED: Question 195: C (Record Over goes) falls away; B (only the sound plays, from nothing) falls away by his earlier words on the knob and the recorded track; A's content holds except its place: "reached from the list of recordings" and "in the live window" are neither confirmed nor replaced by L92 (X-8).
TODAY: src/ui/RecordPanelModel.h:155, 173-177 as cited on the page (not re-read): Record Over shows only while a recording replays with its sound.
@@END

@@ITEM C21
TITLE: Recorded clip: next empty cell on top layer or nearest open cell
STATUS: SETTLED he named the reading and left this part as written
HIS: L88
RULE: A clip recording lands in the next empty cell from the left on the top layer of the deck on screen, without being selected; the two sentences of his are read as the same cell. His L88 corrects only parts D and E of that reading.
CHANGED: nothing in part (b).
TODAY: not checked
@@END

@@ITEM C22
TITLE: A column fires all together against a BPM clip waits
STATUS: DEFAULT question 191 keeps its default A (not named); the empty cells follow his L11 -- AMENDED after page 2 (C22 1 by X)
HIS: L1, L11, L12, L19, L30
RULE: Each clip of a clicked column follows its own rule: a clip that is not in BPM mode starts on the click (the default's own sentence; the second sentence of L11 fits it, INFERRED), and a BPM-mode clip starts on the next 1 (L12). An empty cell of the column clears its layer, as in his Arena (L19), except on a layer set to ignore the column trigger; it clears on the 1 when the clip playing there is in BPM mode ("the empty cell triggers on the 1 if the clip playing is in bpm mode, same as a new clip in bpm mode", L11) and at once otherwise. That L11, said of an empty cell, holds inside a column trigger too is INFERRED by his L9 (the tempo topic carries it). With the tempo stopped the click is the new 1 and everything starts together, which is his show start (L30, L12). With the tempo paused the click is a fire like any other (C2, X-1). Two edges from page 2: a BPM-mode clip of the column that is triggered within a tenth of a beat after the 1 starts at once and does not wait a whole bar (item 249; C16); and the empty cells of a column never start a stopped beat and never end a pause, only the column's clips do (item 248, as the tempo topic rules it; C2). [page 2, X: item 249 accepted as written; item 248 accepted as written]
CHANGED: nothing in 191 A; the empty-cell part comes from L11 (it replaces reading R168 b, where an empty cell never waited, and the moment in reading R205).
TODAY: area-cue-layers.md H12 as cited on the page (not re-read).
@@END

@@ITEM C23
TITLE: Random: his random 1s against mimic Resolume exactly
STATUS: OPEN his answer reads two ways: one question for him (X-28); the best reading is any beat marker -- AMENDED after page 2 (C23 1 by X)
HIS: L101, L20, L21
RULE: A Random jump lands on one of the clip's markers: "201 each of the markers on a clip are bpm lines and the random lands on one of them" (L101). Best reading: the markers are beat markers, one per beat, as he saw in his Arena ("When it changes, it jumps to random beat markers on the clip.", L20; "Bpm mode the clip’s timeline is divided into beat markers.", L21), so a jump can land on any beat of the clip between its in and out points, not only on the first beat of a bar. When it jumps and how far are set by Interval and Distance, both counted in beats, as in his Resolume picture (the reading on Random, not named by him, stands). The other reading: by his earlier rule a clip's timeline shows bar lines only ("Timeline only shows bars.", binding-decisions.md:825); then "the markers on a clip" are bar lines and every jump lands on the first beat of a bar, his earlier "random 1's". The first reading holds and nothing waits on a reply: the question was not put to him, because his own words settle it ("201 each of the markers on a clip are bpm lines and the random lands on one of them", BF219, with his look L20 and L21): one marker per beat, and a jump lands on any of them. His earlier "Timeline only shows bars" gives way for a clip's own timeline. The clip topic's item on Random carries the rule. [page 2, X: BF219 "201 each of the markers on a clip are bpm lines and the random lands on one of them"]
CHANGED: Question 201: answered in his words, without a letter. Under the best reading they fit B (any beat marker); under the other they fit the default A (the first beat of a bar). Mended by this ruling: the paper had it as B flatly and called the two statements reconciled; his earlier "Timeline only shows bars" and the unnamed reading that repeats it (R193 h) were not weighed, and Interval and Distance were left out.
TODAY: area-clip-transport.md U-H11 as cited on the page (not re-read).
@@END

@@ITEM C24
TITLE: Failed save only, against the two recording messages
STATUS: SETTLED same as C18
HIS: L122, L68, L65, L80
RULE: The later sentence adds to the earlier one, and his words of L65, L68 and L80 add three more places; the full list is in C18. Nothing else appears by itself.
CHANGED: Question 209's situation text (four messages) becomes four plus the three places he named.
TODAY: not checked
@@END

@@ITEM C25
TITLE: 42 default (every fire from the start) against the second small menu
STATUS: SETTLED -- AMENDED after page 2 (C25 1 by X)
HIS: L102
RULE: Both small menus of his Resolume picture are built as Resolume has them: the loop menu, and the start menu, which says where a fired clip starts (from the start, pick-up, relative pick-up). His words: "202 model these 2 little menu’s after resolume" (L102). So a fire no longer always means from the beginning: it follows that clip's start menu. Every clip is set to "from the start" until he changes it: that is the text of the option his words fit (option B of question 202), and his earlier "restart" and "42 default" said the same for every fire. One case stands outside the start menu: triggering the clip that is already playing on its layer always restarts it from its start, even when its start menu is set to continue where it left off (item 266). For every other trigger the start menu holds as above. A video he brings in starts in Timeline mode (item 267). [page 2, X: item 266 accepted as written; item 267 accepted as written]
CHANGED: Question 202: B in effect, not the default A; "42 default" (no such menu) is replaced by his later words and now holds as the menu's first setting. Every rule of the page that says "a fired clip starts from its beginning" now reads "starts where its start menu says". Mended by this ruling: the first setting was held as an open line (X-9); option B's own text settles it.
TODAY: src/ui/ClipInspector.cpp:69-73 as cited on the page (not re-read): the menu shows Restart, Continue, Relative and has no handler.
@@END

@@ITEM C26
TITLE: Paused fire against clips that are not BPM based
STATUS: SETTLED one line for him (X-2); one part is his own question (L107) -- AMENDED after page 2 (C26 1 by X)
HIS: L13, L31, L11, L107
RULE: Pause holds what the beat clock controls and nothing else: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." (L13). A clip that is not in BPM mode is not held: one that is playing plays on through the pause, and one that is fired starts on his press (his earlier 136 b and "If there are clips that are not BPM based, then they play just as they were and are unaffected", binding-decisions.md:918-920; the second sentence of L11 fits it, INFERRED; that "the clips" of L13 are the BPM-mode clips is X-2; whether such a fire also sets the beat clock going is X-1). A picture without playback (a still, a generated picture, MilkDrop, the camera, an effects-only clip) is not held either and shows and moves when fired during a pause: the reading that says so was not named by him and stands (X-2). Signals that run on the beat, and the sliders they drive, stand still during the pause on every kind of clip (L13). An effect that reads the beat by itself (a strobe, a pulse) holds still too while the beat is paused or stopped: on pause it stands where it was and goes on in step at play; after a stop it starts on his new 1 (item 243, accepted after the answer he was given). Under that answer he wrote: "For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects" (BF244; "the sink" read as the sync, INFERRED). These words are read as coming on top of the hold, not in its place (INFERRED: he left the box of item 243 empty). What his "adjust the sink" asks for, with its other readings, and whether the common macro is built now, is the effects topic's to rule and to put to him. [page 2, X: item 243 accepted as written; BF244 "we should just be able to adjust the sink"]
CHANGED: nothing in reading R193 (b); question 174's default A is replaced (see C2). Mended by this ruling: the paper's last sentence had "sliders and effects that the beat drives" stand still as a fact; the effects half is his open question (L107).
TODAY: not checked
@@END

@@ITEM N1
TITLE: Resolume's Save and Manage windows; Save onto an existing name
STATUS: DROPPED he looked (L22); the look of ours waits for the UI redesign (L8)
HIS: L22, L8
RULE: Manage opens a small window with the names of all of the effect's presets; a name can be changed and a preset can be deleted (his look: "G manages a small window with the names of all of the presets and the names can be changed or the presets can be deleted.", L22). Saving under a name that exists asks first and then saves over it: that is his own earlier rule (103, 132) and needs no look at Resolume. How the windows look is laid out by Harmony and settled in the UI redesign (L8).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM N2
TITLE: Does Resolume's previewed clip play
STATUS: DROPPED he looked (L16) -- AMENDED after page 2 (N2 1 by X)
HIS: L16, L36
RULE: In his Arena a click on a clip's name plays the clip in the preview and fills the clip tab ("A when you click Lipp's name, it plays in the preview and populates the clip tab", L16; "Lipp's" read as clip's, INFERRED). His rule for the app is now the same as his Arena's: a preview starts at once and never on the beat: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat." (BF247). It replaces his earlier L36 (a previewed clip is triggered on the 1). In time with the beat is only a clip that is triggered. His last sentence, "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" (BF247), is read in the cue topic as a clip triggered on a layer and seen through that layer's cue button (INFERRED; the cue topic shows him that reading as one line). [page 2, X: BF247 "Previewing a clip should not happen on the beat."] Whether Resolume loops it or shows layer effects no longer matters.
CHANGED: nothing here; reading R151 is corrected by L36 in the cue topic.
TODAY: not checked
@@END

@@ITEM N3
TITLE: His Arena on a fire with the tempo paused or stopped
STATUS: DROPPED he looked (L17)
HIS: L17, L12, L13, L31
RULE: In his Arena a fire while the tempo is paused or stopped switches tempo play on ("B tempo paused or stopped, when you trigger a clip tempo play his activated", L17; "his" read as is, INFERRED). His own rule for the app is in L12, L13 and L31 (see C2); the one edge his words leave is X-1.
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM N4
TITLE: Cost of a second picture per frame for the preview monitor
STATUS: OPEN technical: a measurement before the cue part is planned -- AMENDED after page 2 (N4 1 by X)
HIS: L34, L35
RULE: The output is the more important picture ("Of course the output is more important", BF267). Waits on his answer for the rest: he asks "What do you mean by come first?" and whether the output monitor and the preview monitor under it "should all run at the same fps, no?" (BF267; his "the output preview screen" is read as the output monitor and his "preview-cure screen" as the preview monitor, INFERRED). The reading put to him again, as the cue topic rules it: both monitors are shown from the same frame and always run at the same frames per second, and the preview is never drawn at a lower rate on purpose; what the preview needs of its own is drawn at the full size of the picture, and a smaller size is chosen only if the measurement shows that the full size does not fit; "comes first" means that nothing done for the preview ever changes what the output shows, and that the preview gives way when the computer cannot keep up. Either way [page 2, X: BF267 "should all run at the same fps, no?"] its cost is measured before the cue part is planned, and he is told first if the output would lose frames. His new words add work to that picture (a transparency slider beside each cue button, L34; files previewed from the files window and a cue / preview toggle, L35), so the measurement covers a mix of cued layers with their effects and a single previewed file.
CHANGED: nothing
TODAY: facts-app-cue-today.md sections 1 and 5 as cited on the page (not re-read): one picture per frame, no measurement exists. Cheapest: an offscreen second composite at reduced size timed in the render loop when the cue lane is planned (a build step; not now).
@@END

@@ITEM N5
TITLE: Can Review draw its picture without touching the live show
STATUS: OPEN technical (X-12, X-13); his L80 changes the premise -- AMENDED after page 2 (N5 1 by X)
HIS: L80, L72, L91
RULE: Review never changes a slider, a button or a clip of the show that is open in the live window, and that show is back as it was when Review is closed (the reading on opening Review, not named by him, stands). His L80 gives the recording a show file of its own ("the recording also saves a show file with the clips exactly as they are for the show"), so Review plays the recording with that saved copy, not with the open show; with 186 b (L91) the outputs show the Review picture too. Whether the copy can be drawn beside the open show is the technical point, checked before Review is planned (X-12). What he sees: nothing more than that reading already says, and no extra save question (X-13). One thing does go from Studio into a show: an action he saves there. It goes into the show the recording was made in, onto the same clip, wherever that clip is by now, and if the clip is gone the app tells him and nothing is saved (item 230). When that show is the one open in the live window, the saved action is a change to it like any other, written to the show file at his next Save. Playing a recording, mending it and recording over it never change the open show. What opening Studio stops in the live show, and what happens when another show is open there, is ruled in the recordings topic. [page 2, X: item 230 accepted as written]
CHANGED: Reading R173 ("the review uses the show as it is now") is replaced by L80; reading R165's aim stands. Mended by this ruling: the paper's X-13 added a save question at the opening of Review that the standing reading does not have; it is now the other way of X-13.
TODAY: facts-takes-bpm-fire.md section 1, S5 as cited on the page (not re-read): a replay drives the live layers. Cheapest: an architect reads docs/claude/pitfalls.md entries 37, 52, 55, 58 and the capture and load code.
@@END

@@ITEM N6
TITLE: How the music's part of a recording is kept
STATUS: OPEN technical (X-14); his words lean to stored values
HIS: L71, L81, L86
RULE: A slider that a signal moved gets its own track in the recording like any other moved control, in a different colour ("we need to have a different color for the actions that were recorded off of signals", L71), and an action can be made from it "and treated like a regular action" (L71). That needs the values each such slider took to be in the recording itself, so a recording shows them with or without its sound. Only what moved gets a track (L81).
CHANGED: Reading R183 (g) ("I cannot promise what a slider that the music was moving shows when Audio is not ticked") falls away if X-14 holds.
TODAY: facts-actions-open.md section 4 item 13 as cited on the page (not re-read); no measurement of the cost of storing every signal-driven value per frame. Cheapest: count signal-driven sliders in a full show and size the stream on paper, then measure when the recorder is extended.
@@END

@@ITEM N7
TITLE: How long the app takes to find the tempo after launch
STATUS: DROPPED question 175 keeps its default A, so nothing waits on this
HIS: L1, L14
RULE: The tempo number is never empty: the app opens at the tempo used last (120 the very first time) and a first fire starts the beat at that number; the music, a tap or a typed number then corrects it. The tempo runs from 22 to 480, by hand always; "/2" and "x2" work to both limits, by hand and on the listening clock: "The /2 and x2 manual work to both limit as well as the listening clock." (L14; read as: to both limits in manual mode, and on the listening clock as well, INFERRED). The listening is given the same open range for now: "If having such an open range is distorting the listening clock, then we will change that later but manually it will always go from 22 to 480." (L14). How long the listening takes to lock is no longer needed to size anything.
CHANGED: nothing in question 175. Added by this ruling: the second and third sentence of L14 (the limits of "/2" and "x2"; the listening clock's range), which the paper left out.
TODAY: facts-app-cue-today.md section 6 as cited on the page (BPMTracker.h:36, 216, 219; not re-read): no tempo until enough music is heard.
@@END

@@ITEM N8
TITLE: Is the Link switch dimmed in his copy
STATUS: DROPPED Link is to be planned (L128), so its present state only goes into Harmony's notes
HIS: L128
RULE: Ableton Link is planned with the other five things of question 215: "215 plan all of these" (L128). What the tempo row does while Link is on is the reading on Link that he did not name (play, pause and Resync greyed; stop still clears the layers).
CHANGED: Reading R178 ("nothing new is built for it") is replaced by L128.
TODAY: area-tempo.md part 1 line 10, M-3 as cited on the page (not re-read): the switch is dimmed because his build is made without Link.
@@END

@@ITEM N9
TITLE: Does Resolume's Tap leave the start of the bar alone
STATUS: DROPPED he looked (L18); one line for him (X-10) -- AMENDED after page 2 (N9 1 by X, N9 2 by X)
HIS: L18, L13
RULE: In his Arena "the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing" (L18). For the app, question 190 keeps its default A (a tap sets the tempo only; the 1 is set by him with Resync, and in automatic mode the app also places it by itself when it is sure (BF246, way b of item 218) [page 2, X: BF246 "b. app should always try to find the 1 and I will correct if necessary"]), which his own words back: "/2 and x2, tempo change do not move the 1" (L13). From his look one thing is taken over: the tempo follows from the second tap on (X-10). Waits on his answer for the one point that was put to him as item 250: he asks back "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?" (BF266; "the beep" read as the beat, INFERRED). The reading put to him again, from the tempo topic: tapping changes only how fast the beat runs, from the second tap on; it does not slide the beats onto his taps and never makes another beat the 1. Not waiting: a tap alone starts nothing, and a tap never changes which beat is the 1. [page 2, X: BF266 "What does shifting the beep mean to you?"]
CHANGED: Question 190 A's reason ("Resolume, my guess") is replaced by his own words (L13 and his earlier "If I tapped the tempo again, to set the tempo, the time does not change."); decided line D3 (the average of the last eight taps) must still react from the second tap.
TODAY: area-tempo.md M-1, M-2 as cited on the page (TopBar.cpp:55-77; BPMTracker.cpp:253-258; not re-read): from the second tap every tap also pulls the beat.
@@END

@@ITEM N10
TITLE: Arena's quit question and its deck list
STATUS: DROPPED his own rules stand; what Arena does is no longer needed
HIS: L1, L94
RULE: The quit window comes at every quit (his 93 b); deck tabs keep the order in which the decks were made (the reading on menus and files of shows and decks, not named). New from L94: "When you open the application, it opens to the very last show."
CHANGED: Reading R189 (h) ("The app opens with a new, empty show") is replaced by L94.
TODAY: not checked
@@END

@@ITEM N11
TITLE: Freeze or Echo picture on a preset load or a bypass
STATUS: OPEN technical (X-15)
HIS: L106
RULE: Loading a preset on Freeze or Echo, or switching one off and on with B, keeps the picture it holds; removing the effect clears it. He named that reading (L106-L108) and did not touch this part (d), so the aim stands.
CHANGED: nothing
TODAY: not checked (UNKNOWN on the page). Cheapest: read how the per-chain history is keyed and cleared (docs/claude/pitfalls.md entry 35, EffectChain and CompositorEngine) when the presets build is planned.
@@END

@@ITEM N12
TITLE: Which pad controllers he plays with
STATUS: DROPPED question 214 keeps its default A (not named)
HIS: L1
RULE: Nothing new for pad lights for now: the new buttons (actions, cue, the tempo row) get no lights until he names a controller. His message names none ("the midi controller", L92).
CHANGED: nothing
TODAY: src/midi/MidiOutputHandler.h:10, 19-20 as cited on the page (not re-read): lights are made for Launchpad X and Mini MK3.
@@END

@@ITEM N13
TITLE: The counts of the blend, keying and transition lists
STATUS: OPEN technical (X-16); it waits for the last build (L110) -- AMENDED after page 2 (N13 1 by X)
HIS: L110, L111
RULE: The lists are worked on last: "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change" (L110). Before that build an architect reads both count tables and names one. The Keying list stays and is counted with the blend and transition lists: "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder." (BF260); it replaces his earlier L111 (remove the keying and its slider). Masks and moving masks are added as well: "We will also add masks and moving masks with which are alpha channels" (BF260); what a mask is and when each of these is built is ruled in the effects topic. [page 2, X: BF260 "All the blend modes and keying stay"]
CHANGED: Question 203: C, not the default A; the Keying list of A is replaced by L111.
TODAY: boris-keying.html against rulings-keying.md, area-cue-layers.md T11 as cited on the page (not re-read): the two differ by one or two entries.
@@END

@@ITEM N14
TITLE: Does a Resolume column click clear a layer with an empty slot
STATUS: DROPPED he looked (L19)
HIS: L19, L11, L9
RULE: In his Arena it does ("D layer goes empty when a column is triggered with an empty", L19). In the app a click on a column trigger clears every layer whose cell in that column is empty, except a layer set to ignore the column trigger (the reading on a column click, not named by him, stands). When: on the 1 if the clip playing on that layer is in BPM mode, at once otherwise (L11, said of an empty cell; that it holds inside a column trigger is INFERRED by his L9).
CHANGED: Reading R205 stands in what happens; its moment follows L11 (the page had "at once").
TODAY: area-cue-layers.md M3 as cited on the page (Layer.h:402-403; not re-read): the app clears such a layer at once.
@@END

@@ITEM N15
TITLE: Resolume's Random: any beat or the first beat of a bar
STATUS: DROPPED he looked (L20); what the app does is C23, with one question for him (X-28)
HIS: L20, L101
RULE: In his Arena Random "jumps to random beat markers on the clip" (L20). What the app does is C23: best reading, any beat marker of the clip; the other reading, the first beat of a bar, is asked once (X-28).
CHANGED: nothing beyond C23.
TODAY: not checked
@@END

@@ITEM N16
TITLE: What Resolume titles the tab that lists the effects
STATUS: DROPPED the name is his own word; it goes on the list of names (L114)
HIS: L41, L114, L40
RULE: The browser tab that lists the effects reads "Effects": the reading that proposed it was not named by him and stands, and his own word for that tab is "effect tab" (L41: "if you double click an effect in effect tab"; earlier "the effects tab", binding-decisions.md:1089). It is entered in the one list of names he asks for ("We need a solid list with what we call everything.", L114). The effects part of a clip's, a layer's or the Global tab is another thing and gets no bar and no title: "then there is no header bar for any effects" (L40).
CHANGED: nothing. Mended by this ruling: the paper cited L40 for the tab's name; in L40 "the effects tab for a clip layer or Global" is the effects part of those tabs, not the browser tab.
TODAY: facts-presets-delta.md 1.5 as cited on the page (BrowserPanel.h:48; not re-read): the tab reads "FX".
@@END

@@ITEM N17
TITLE: What "Bag" does in the MilkDrop browser
STATUS: DROPPED looked at in the source; it goes into the MilkDrop document (L126)
HIS: L126
RULE: Nothing of MilkDrop is changed in the coming build; Harmony writes "a dedicated document with how milk drop functions currently and that's it for this upcoming build" (L126), and Bag is described there as it really behaves.
CHANGED: Reading R202 (b) ("Random and Bag, MilkDrop's two ways of shuffling") is to be worded truthfully in that document.
TODAY: src/sources/PresetSelector.h:51-54 (read by me): the comment says Bag is not a true no-repeat-until-exhausted and behaves like Random; the two lists offer "Bag" at src/ui/MilkDropBrowser.cpp:545 and :605. INFERRED from the comment, not run.
@@END

@@ITEM N18
TITLE: Pause pressed on a stopped beat
STATUS: OPEN Harmony's own, not asked (X-17); the architect delta on the tempo bar is still owed -- AMENDED after page 2 (N18 1 by X)
HIS: L30, L12, L13, L31
RULE: The show start does not need this press (L30), but the press exists and must do something defined. His words give the neighbours: stopped, a play or a fire is the new 1 (L12); paused, the clock holds its position (L13, L31). Harmony's reading (X-17): pause pressed on a stopped tempo does nothing; stop pressed while paused stops and clears as any stop; play while the tempo runs changes nothing. A stop pressed while the tempo is already stopped changes nothing for the beat and the layers, which are stopped and empty; it still does what every tempo stop does to whatever is on at that moment: the actions he switched on since ("tempo stop stops all actions, not just global", BF250) and an audio file the app plays ("Stop removes all clips from all layers so it would stop.", BF271), as the actions topic and the sources topic rule them. [page 2, X: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers so it would stop."] The adopted tempo-bar text gets a delta that defines these four presses.
CHANGED: Decided line D6 ("pause pressed on a stopped beat holds it at its start") loses its reason (question 172 A is not taken).
TODAY: area-tempo.md U-TECH-4 as cited on the page (not re-read): the adopted tempo-row text does not define these presses. Cheapest: an architect reads .harmony/.reports/s-rta-1004b/ruling-nudge-row2.md section 1.
@@END

@@ITEM N19
TITLE: A bypassed layer's clip: stands still or plays on
STATUS: DROPPED he looked (L29); the rule stands
HIS: L29
RULE: A clip on a layer that is bypassed, or hidden by another layer's solo, plays on out of sight, so the layer comes back further on. His Arena does the same: "H it plays out of sight" (L29).
CHANGED: nothing
TODAY: CompositorEngine.cpp:1079-1080 as cited on the page (not re-read): the layer is skipped, so its clip appears to stand still; that is what changes.
@@END

@@ITEM N20
TITLE: Does the layer strip's pause button pause the clip
STATUS: OPEN technical (X-18)
HIS: none
RULE: A clip's pause is one thing shown in two places: the pause on the layer strip and the pause in the Clip tab are the same button of the clip that plays (the reading on how a clip plays, not named, stands).
CHANGED: nothing
TODAY: src/ui/LayerStrip.cpp:376-381 (read by me): the strip's "||" sets the playing clip's playing flag to false and calls onTransportPause; so the code does write the pause, against the handoff's "does nothing". What shows on screen is not run. Cheapest: checked when the transport part is built.
@@END

@@ITEM N21
TITLE: Filed fault: Return after a cell click clears the top layer
STATUS: OPEN technical (X-19)
HIS: none
RULE: The Return key never clears a layer; if the fault is still there it is mended with the keyboard and MIDI mapping build. It is not a question for him.
CHANGED: nothing
TODAY: not checked: the lane board was not found in two greps; .harmony/HANDOFF-ARCHIVE.md:36 still lists it as HIGH. Cheapest: the lane board row 11, or read the key handler in src/MainComponent.cpp.
@@END

@@ITEM N22
TITLE: The real top of the Speed range
STATUS: OPEN technical (X-20)
HIS: L21
RULE: Speed runs to 10 when a clip is not in BPM mode (his 74 b); the real top the video player can hold is measured before the transport part is built, and he is told first if it is lower. In timeline mode Speed and Duration do not change each other's setting; each makes the video move slower or faster (his look, L21).
CHANGED: nothing
TODAY: not checked (the strip's forward button caps speed at 4.0: src/ui/LayerStrip.cpp:391, read by me). Cheapest: a timed playback run of one of his files at 4, 6, 8, 10 when the transport part is built.
@@END

@@ITEM N23
TITLE: Beat-reading effect on a held beat; held picture on preset load
STATUS: OPEN the product half is his question (L107); the other half is technical (X-15) -- AMENDED after page 2 (N23 1 by X)
HIS: L107, L13
RULE: Two parts. (1) An effect that reads the beat by itself (a strobe, a pulse) holds still while the tempo is paused or stopped (item 243, accepted after the answer to his question of L107; his own L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."). Under that answer he wrote that for these effects "we should just be able to adjust the sink, and maybe we could use a common macro that we could set later" (BF244; "the sink" read as the sync, INFERRED). These words are read as coming on top of the hold, not in its place (INFERRED: he left the box of item 243 empty); what the sync control is and whether the common macro is built now is the effects topic's to rule and to put to him. [page 2, X: item 243 accepted as written; BF244 "maybe we could use a common macro"] (2) Loading a preset on Freeze or Echo keeps the picture it holds (N11).
CHANGED: Reading R221 (b) is not confirmed: he asks about it.
TODAY: not checked (UNKNOWN on the page): what such an effect does on a held beat, and what the code does with a held picture at a preset load. Cheapest: read which shaders take u_beatPhase directly (src/render/EmbeddedShaders.h) and what feeds that uniform.
@@END

@@ITEM N24
TITLE: Does the film of the Video box play at the right speed
STATUS: OPEN technical (X-21) -- AMENDED after page 2 (N24 1 by X)
HIS: L6, L88
RULE: Every film the app writes plays at the right speed in an ordinary player. This covers the low-resolution show recording in 10-minute pieces (L6; item 262), the clip made with record to clip, which is exactly the output recorded as one layer (L88; BF268), and the HD render that can be made from Studio ("we can make an HD render from the Recording Review", BF241). Which codec each of these films uses follows the test of codecs he asks for after the build (BF240, BF241), which the clip topic owns. [page 2, X: item 262 accepted as written; BF268 "exactly the output recorded as one layer"; BF241 "we can make an HD render from the Recording Review"]
CHANGED: nothing
TODAY: VideoRecorder.cpp:102-141, 385 by the cover-recording seat as cited on the page (not re-read): INFERRED that the film may play at the wrong speed. Cheapest: when that part is built, record ten seconds and compare the file's length with the clock.
@@END

@@ITEM N25
TITLE: What each entry of MilkDrop's Jukebox list really waits
STATUS: OPEN technical (X-22): it goes into the MilkDrop document (L126)
HIS: L126
RULE: MilkDrop is not changed in the coming build; its document says what each Jukebox entry really waits. The relabelling of the list waits for his dedicated MilkDrop session (L126).
CHANGED: Reading R212 (the list is relabelled in bars) is not built in the coming build: L126 ("that's it for this upcoming build").
TODAY: src/sources/PresetSelector.cpp:79-83, 169 by two seats as cited on the page (4, 8, 16, 32, 32, 64 bars; not re-read, not run). Cheapest: re-read those lines while writing the document.
@@END

@@ITEM N26
TITLE: The title "Mapping Editor" in the code
STATUS: DROPPED looked at: it cannot be reached on screen
HIS: L114
RULE: "Mapping" on screen means the keyboard and MIDI mapping only; no screen titled Mapping Editor is shown. The name does not go on his list of names (L114).
CHANGED: nothing
TODAY: src/ui/MappingEditor.cpp:100 (read by me: the title text "Mapping Editor"); .harmony/APP-INVENTORY.md:104 (read by me: reached only from the effects rack's map buttons, UNREACHABLE because the rack is hidden).
@@END

@@ITEM U1
TITLE: Texts of the page that no seat had checked
STATUS: DROPPED each of them is tested against his words in this session's topic papers
HIS: L1, L98, L99, L106, L84, L89, L72, L118, L102, L92, L112
RULE: Of those texts he named and answered the readings on the loop menu, the length row, the signal edges, the mend, the Review edges, the action edges and the keys, and questions 202, 195 and 205. The others (not named) stand or keep their default by L1 and are tested against his whole message in their topic papers. Question 216 falls away: there is no one-layer recording (L88).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM U2
TITLE: His show-start sentence read as pause, fire, play
STATUS: SETTLED
HIS: L30
RULE: That reading is withdrawn. His words: "The only way to do this is with a column trigger. Moving on." (L30). What pause does on a stopped tempo is N18.
CHANGED: Question 172 A and reading R140 (see C1).
TODAY: not checked
@@END

@@ITEM U3
TITLE: Paused: clips with playback only, or generated pictures too
STATUS: STANDS the reading on pictures without playback (not named by him); his sentence itself is clarified by L31
HIS: L31, L13
RULE: His earlier sentence "When paused, it will not play but will still display." is clarified by him: it is about the clip that was playing when pause was pressed ("If I have a click that is playing, and I pause the tempo it then it will display the paused clip", L31; "click" read as clip, INFERRED). Pause holds "the beat clock, the clips and everything that it controls with BPM" (L13). A generated picture, MilkDrop, a still, the camera and an effects-only clip are not held and keep moving: the reading that says so was not named by him and stands; L13 does not speak of them. A clip that is not in BPM mode is not held either (his earlier 136 b). Both are shown to him as one line (X-2).
CHANGED: nothing in reading R193 (b). Mended by this ruling: the paper called this settled by L13 and L11; neither line names a generated picture.
TODAY: not checked
@@END

@@ITEM U4
TITLE: Previewing files that are not in the clip grid yet
STATUS: SETTLED that files are previewed; when a preview starts is one question for him (X-29) -- AMENDED after page 2 (U4 1 by X)
HIS: L35, L36
RULE: Files are previewed from the files window too: "we also want to preview the clips double clicked from the files window and name clicked from a clip in the deck. They will play right away and we will have a toggle between cue mode and preview mode." (L35). A double-click on a file or a click on a clip's name takes over the preview monitor; a toggle button brings the cue back ("to see the cue you need to push the toggle button", L35). No undocking of the monitor yet ("no need to undock and move just yet", L35). WHEN a preview starts is settled by his newest words: at once, for a file from the files window and for a clip previewed by its name alike, in BPM mode or not: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat." (BF247). His L35 ("They will play right away") stands; his L36 (a clip previewed by its name "is triggered on the 1") is replaced. Only files and deck clips are previewed: a double-click on an effect shows its properties but no picture, and a source is not previewed (item 253). [page 2, X: BF247 "Previewing a clip should not happen on the beat."; item 253 accepted as written]
CHANGED: Reading R170: "it shows cued layers and clips that are in the clip grid, not files in the Files tab" is replaced; previewing a file joins the first build. Added by this ruling: the pull between L35 and L36, which the paper did not name.
TODAY: not checked
@@END

@@ITEM U5
TITLE: Saving an action without its ignored controls
STATUS: DEFAULT question 185 keeps its default A (not named); Harmony's reading of one sentence is X-23, not asked
HIS: L1, L72
RULE: The action's right-click menu has "Save without ignored controls", which makes a NEW action that moves only the controls not set to ignore, named after the old one with a number; the old action stays. He approved the reading that lists this entry among what can be done to a saved action: "R224 yes. I like that." (L72). His sentence in the same line, "We will only allow users to record actions in the recording review screen, not modify during a show.", is read as: an action is made only in Review and a saved action is never opened and changed; this entry records nothing and changes no saved action, so it stays in the menu wherever the action's button is (X-23).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM U6
TITLE: Overridden actions: switched off or held back
STATUS: DROPPED both readings stand (not named by him)
HIS: L1
RULE: An action that a later action or a global action overrides stays switched on, carries the light that says something else holds a part of it, and takes its sliders back when the other goes off. He went through the page, corrected the readings on either side (L66, L67) and named neither of these two.
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM U7
TITLE: Random 1s against mimic exactly
STATUS: OPEN his answer reads two ways: see C23 (X-28)
HIS: L101, L20
RULE: A Random jump lands on one of the clip's markers (L101). Best reading: any beat marker, as in his Arena (L20); the other reading, bar lines only and so his earlier "random 1's", is asked once (C23, X-28).
CHANGED: Question 201 (see C23).
TODAY: not checked
@@END

@@ITEM U8
TITLE: A new show takes the last show's keyboard and MIDI mapping
STATUS: SETTLED he named the reading and left this part as written
HIS: L94, L116
RULE: Every show brings its own keyboard and MIDI mapping; a new show starts with the mapping of the show saved last, by itself. He named that reading (L94) and changed only the layout, the opening of the last show and the snapshot. Mapping files stay as well: "R199 we want mapping files" (L116).
CHANGED: Reading R188 (c) stands; reading R199 (f) ("Export and Import files go") is replaced by L116.
TODAY: not checked
@@END

@@ITEM U9
TITLE: The Save button's rule under the nameless-P option
STATUS: DROPPED that option was not chosen (L52)
HIS: L52
RULE: The effect remembers its preset and shows its name; the Save button replaces the name when the preset is changed (see C3). The rule invented for the nameless P is not needed.
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM U10
TITLE: Cueing ahead and a preview of a whole unfired column
STATUS: DROPPED the reading stands (not named); no column preview is in his words
HIS: L1, L34, L35
RULE: Cueing ahead is done with the cue buttons under the preview monitor; a preview of a whole column that is not fired is not built. New from him: a transparency slider beside each cue button (L34) and the cue / preview toggle (L35), both handled in the cue topic.
CHANGED: Reading R149 (a) ("a cued layer shows at full strength wherever its fader is") is touched by L34 (the slider beside the cue button), in the cue topic's paper.
TODAY: not checked
@@END

@@ITEM U11
TITLE: Three unmeasured things a reading's promise rests on
STATUS: OPEN technical: the same three as N4, N5, N6
HIS: L80, L71
RULE: See N4 (cost of the preview picture), N5 (Review beside the open show; L80 gives the recording its own show file) and N6 (the music's part of a recording; L71 leans to stored values). None is his to answer.
CHANGED: nothing
TODAY: not checked; none measured.
@@END

@@ITEM U12
TITLE: Readings that rested on consent by silence
STATUS: DROPPED he has now read the page: two of the four he named and corrected; two stand except where his words elsewhere touch them
HIS: L1, L14, L96, L94, L102, L101
RULE: The tempo-bar reading is corrected by L14 (22 to 480; the nudge amount shown only in automatic mode; one circle = one bar). The outputs reading is corrected by L96 (a show remembers its connected outputs). The readings on opening and saving a show and on how a clip plays were not named and stand, except where his words elsewhere say otherwise: "the app opens with a new, empty show" is replaced by L94; "every fire of a video starts it from its beginning" now follows the clip's start menu (L102, C25); "the clip's timeline shows bars only" is weighed against L101 in one question (C23, X-28).
CHANGED: R176 (d), (e); R191 (g), (i); R189 (h); R193 (c) and, pending his reply, (h): each in its topic paper.
TODAY: not checked
@@END

@@ITEM U13
TITLE: Signals and macros saved with the show
STATUS: SETTLED he named the reading and left this part as written -- AMENDED after page 2 (U13 1 by X)
HIS: L94, L104
RULE: The show keeps his own signals (their shapes and lengths, his envelopes) and the macro knobs of clip, layer and global with their names and what they drive. There can be any number of envelopes, one per slider or shared by many (L104), and they are kept with the show the same way (INFERRED from the reading he left standing). Envelopes are of two kinds, and the show keeps both the same way: one that runs along the beat, "which is like a signal, but personalized for that clip", and one that follows the playhead, "so we can draw a envelope that happens over the course of the playing of that clip in timeline mode" (BF261). That the show keeps both kinds is INFERRED from the rule above. Two things his sentence leaves open are ruled in the effects topic and not here: whether an envelope made on a clip belongs to that clip alone ("personalized for that clip", beside "shared by many" above), and which clips offer the playhead kind (only a clip in Timeline mode, as his words name it, or a clip in BPM mode too). [page 2, X: BF261 "There should be two different types of envelopes that could work."]
CHANGED: Reading R188 (b) stands.
TODAY: area-effects-signals.md E15, E16 as cited on the page (not re-read): signals and macros are lost when a show is re-opened.
@@END

@@ITEM U14
TITLE: The seven things of question 212, parked not answered
STATUS: DROPPED question 212 keeps its default A (not named)
HIS: L1, L126
RULE: Later, all seven: the coming builds do not touch them; after the builds they are gone through one by one with their own questions. MilkDrop likewise gets its own session (L126). They do not stand in the way of "all is clear".
CHANGED: nothing
TODAY: area-sources-auto.md H1-H6, H9 as cited on the page (not re-read).
@@END

@@ITEM U15
TITLE: Questions he might not answer in ten seconds
STATUS: DROPPED he answered three and took the default of three
HIS: L110, L128, L92, L1
RULE: 203: "203 c" (L110). 215: "215 plan all of these" (L128). 195: record over stays (L92). 212, 185 and 199 were not named and keep their defaults (L1).
CHANGED: nothing
TODAY: not checked
@@END

@@ITEM U16
TITLE: The one-layer recording
STATUS: SETTLED -- AMENDED after page 2 (U16 1 by X)
HIS: L88
RULE: There is no one-layer recording: "E) I don't think there's any reason for a one layer recording. Recording will be the output of the layers but not the screen output as screens can be modified to fit a projectors color and timing issues. It must be the full screen composition." (L88). A clip recording is the whole composition, without any one screen's delay or colour. Question 216 (the fader in a one-layer recording) falls away. In his newest words: "This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like." (BF268): the full picture with the global effects, black where no layer shows, with no see-through parts (read from "Whatever the output is displaying": the output shows black there; INFERRED). His two uses of "one layer" are two things: the one-layer recording above is the recording of one chosen layer, which stays out; "recorded as one layer" here means that the whole output lands as one clip that plays on one layer. "The output" there is read as the composition's picture, as the sentence before this one says, not one screen's own delay or colour (INFERRED; the recordings topic carries it). [page 2, X: BF268 "This is exactly the output recorded as one layer."]
CHANGED: Reading R186 (e) is removed; the Whole / Layer choice at REC goes.
TODAY: not checked
@@END

@@ITEM U17
TITLE: App opens with the beat stopped; black output when no clip plays
STATUS: DROPPED both readings stand (not named by him) -- AMENDED after page 2 (U17 1 by X)
HIS: L1, L94, L12
RULE: The app opens to the very last show (L94) with the tempo stopped and waiting; the first play or fire is the 1 (L12). With no clip playing the output is black, with one exception in the coming build: a preset he picks by hand in the MilkDrop tab shows MilkDrop on the output, because nothing of MilkDrop changes until he has designed its new system ("Keep Milk drop as it is. When I have time while you are building, I will design a whole system for Milk drop", BF265, said on item 247); when that picture goes again is ruled in the sources topic; [page 2, X: BF265 "Keep Milk drop as it is."] the old top row's loose picture and its buttons go.
CHANGED: nothing in R163 and R228; L94 adds which show is open at launch.
TODAY: src/MainComponent.cpp:2749-2767 as cited on the page (not re-read).
@@END

@@ITEM U18
TITLE: A previewed clip while the beat is stopped; a bypassed layer's clip
STATUS: OPEN the first half is put to him (X-24, X-29); the second half is closed (L29) -- AMENDED after page 2 (U18 1 by B)
HIS: L36, L35, L29
RULE: A bypassed layer's clip plays on out of sight (see N19). A clip previewed by its name starts in the preview at the click, never on the 1, with all its actions that are switched on (BF247: "Previewing a clip should not happen on the beat"); only a triggered clip is in time with the beat. The same holds while the tempo is stopped or paused (item 221, accepted as written): it plays in the preview at once, at the tempo's BPM, with its actions, and previewing never starts the tempo. The full rule: R151 in the cue topic. [page 2, B: BF247 "Previewing a clip should not happen on the beat."; item 221 accepted as written]
CHANGED: Reading R151: "it starts from its beginning" at once is replaced by "triggered on the 1" (L36); reading R179 (c) ("shown without its actions") is replaced by L36. R151's sentence on a stopped or paused beat was not named by him; it is asked because L36 changes what it stood on.
TODAY: not checked
@@END

@@ITEM U19
TITLE: Deck files going; a mend changing the recording itself
STATUS: SETTLED -- AMENDED after page 2 (U19 1 by X)
HIS: L84, L1, L69
RULE: A mend never changes the recording it is made on: "the mend is another recording that is saved, parallel to the original." (L84). "Save Deck", "Save Deck As..." and "Load Deck..." go, and deck files saved earlier can no longer be opened (that reading was not named by him and stands). Nothing is carried over from old files: "delete all the old show files and start from scratch." (L69). "All the old show files" means every show saved so far and the app's safety copies of them: Harmony lists them first, and after his yes they go to the Trash. Saved decks, recordings, presets and settings are not deleted (item 235); the old deck files stay on the disk although the app no longer opens them. [page 2, X: item 235 accepted as written]
CHANGED: The line of reading R222 that a mend changes the recording itself (with a button that brings back the night as recorded) is replaced by L84. Mended by this ruling: the paper stretched L69 over the old deck files; L69 names show files.
TODAY: not checked
@@END

@@ITEM U20
TITLE: Areas not swept: Preferences, the Audio Store, the remote-control port
STATUS: OPEN technical (X-25)
HIS: L114
RULE: Harmony goes through the three by itself before the plan is called clear, enters what they are called in the one list of names (L114), and asks him only where one of them needs a decision of his.
CHANGED: nothing
TODAY: not checked. Cheapest: read src/ui/PreferencesDialog.cpp, docs/claude/recording.md (Audio Store) and docs/claude/integration.md (the REST commands).
@@END

@@ITEM U21
TITLE: Record over: old moves playing under him, or from nothing
STATUS: SETTLED that the old moves play; the screen it runs on is one question for him (X-8) -- AMENDED after page 2 (U21 1 by X)
HIS: L92
RULE: Record Over stays, runs in real time from the MIDI controller, and is kept separately from the original (L92). The recording plays with its sound and its old moves, and a control he moves takes over from its recorded track: his earlier words "The knob will supersede whatever is happening, And will snap back to the recorded track as soon as it is let go. It could be in record over mode" (binding-decisions.md:340-342). So he does not start from nothing. It runs in Studio, as a button there, beside scrubbing and drawing: the show recording plays in real time with its sound (item 229). It takes everything in the keyboard and MIDI mapping, clip triggers too; what he moves with the mouse is not recorded over (item 257). [page 2, X: item 229 accepted as written; item 257 accepted as written]
CHANGED: Question 195: C falls away (L92); B (from nothing) falls away by his earlier words; A holds except its place, the live window, which L92 neither confirms nor replaces.
TODAY: src/record/RecorderHost.cpp:209-219, 669, 818 as cited on the page (not re-read).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME X3-3
ABOUT: 241, U13
TEXT: I assume the show keeps both kinds of envelope, the one along the beat and the one along the clip's playhead, with the slider or sliders each one drives.
WHY: BF261 adds the playhead kind; that a show keeps envelopes rests on a reading he left standing, said before there were two kinds.
ALT: none
IF-WRONG: SMALL an envelope missing after a show is opened again.
ASK: NO it follows from the standing rule that the show keeps his envelopes.
@@END

@@ASSUME X-5
ABOUT: 219, C15
TEXT: I assume the nudge amount is counted only in automatic mode; in manual mode a nudge just moves the 1. A show still keeps the amount, but it counts only when the show is opened while the beat runs.
WHY: Item 219 (accepted) sets the amount to 0 at every start after a pause or a stop; his earlier "good (this is just nudge amount)" (binding-decisions.md:916) kept it in the show; together they leave the kept amount almost no life. L14 shows the amount only in automatic mode and does not say whether one is counted in manual mode.
ALT: b) A show no longer keeps a nudge amount at all. c) The amount is counted in both modes and only hidden in manual mode.
IF-WRONG: SMALL a number on the tempo bar after a show is opened.
ASK: NO bookkeeping of one number; what he sees at a start is settled by item 219; the twin of the tempo topic's line on what a show keeps of the nudge.
@@END

@@ASSUME X-12
ABOUT: N5, U11
TEXT: I assume Review can play a recording without changing a slider, a button or a clip of the show you had open; that is checked in the code before Review is planned.
WHY: Not shown to be possible yet; L80 gives the recording a show file of its own, which changes how it is done.
ALT: none
IF-WRONG: REBUILD Review's whole way of drawing its picture.
ASK: NO technical: an architect's read of the capture and load code.
@@END

@@ASSUME X-13
ABOUT: N5, 230
TEXT: I assume opening a recording in Studio never replaces the show you have open and changes none of its settings: Studio plays the recording with the show file saved with it. Only an action you save in Studio goes into your show.
WHY: L80 says the recording saves and opens its own show file; what becomes of the open show meanwhile is not said. Item 230 (accepted) puts a saved action into the show the recording was made in, so "never changes" needed that one exception.
ALT: b) Opening a recording first asks to save your open show, as opening another show does.
IF-WRONG: SMALL one question window more or fewer.
ASK: NO it repeats a reading he left standing, with the exception he accepted in item 230; how it is done is technical, and what opening Studio stops is the recordings topic's line.
@@END

@@ASSUME X-14
ABOUT: N6, U11
TEXT: I assume a recording keeps the values of every slider that a signal moved, as its own track, so Review shows them the same with or without the recording's sound.
WHY: L71 asks for such tracks in their own colour and for actions made from them; how they are kept is technical.
ALT: b) The recording's sound is analysed again each time; without sound those tracks are empty.
IF-WRONG: REBUILD the recorder's stream.
ASK: NO technical: sized on paper, then measured.
@@END

@@ASSUME X-15
ABOUT: N11, N23
TEXT: I assume loading a preset on Freeze or Echo, or switching one off and on, keeps the picture it holds, and removing the effect clears it; I check in the code what has to change for that.
WHY: He named that reading (L106) and left part (d) as written; what the code does is unknown.
ALT: none
IF-WRONG: SMALL a held picture cleared at a preset load.
ASK: NO technical: a read of the history code.
@@END

@@ASSUME X-16
ABOUT: 240, N13
TEXT: I assume nothing of the blend, keying and transition lists changes until that part is built, and that every blend mode and every keying entry stays. Before that I settle the small difference between my two count tables.
WHY: BF260 keeps them ("All the blend modes and keying stay, and they will be built when there is time."), against his earlier L111; L110 puts the lists last; the two count tables differ by one or two. BF260 does not name the transition list, so nothing is said here of its entries.
ALT: none
IF-WRONG: SMALL one or two list entries.
ASK: NO technical: an architect reads both tables before that part is planned; the order of work is held by the effects topic, internal there too.
@@END

@@ASSUME X-17
ABOUT: N18
TEXT: I assume pause does nothing while the tempo is stopped, stop works from pause as from running, and play does nothing while the tempo already runs. A stop pressed again while stopped still stops the actions you switched on since, and an audio file.
WHY: L30 drops the pause-first show start that needed this press; no word of his defines the press itself. BF250 ("tempo stop stops all actions, not just global") and BF271 ("Stop removes all clips from all layers so it would stop.") give a stop work to do also when the tempo is already stopped; they name no exception.
ALT: b) Pause on a stopped tempo arms it: clips you trigger then wait, and play is the 1. c) A stop pressed while the tempo is already stopped does nothing at all.
IF-WRONG: SMALL one button press with or without an effect.
ASK: NO presses at the edge; the stop follows his own words for every stop, and the other way of pause is the paused start that his L30 did not take; the tempo topic holds the twin.
@@END

@@ASSUME X-18
ABOUT: N20
TEXT: I assume the pause on the layer strip and the pause in the Clip tab are one and the same pause of the clip that plays.
WHY: The handoff and the code disagree on what the strip's button does now; no word of his is needed.
ALT: none
IF-WRONG: SMALL a button that has to be mended.
ASK: NO technical: checked when the transport part is built.
@@END

@@ASSUME X-19
ABOUT: N21
TEXT: I assume the Return key never clears a layer; if that old fault is still in the app it is mended with the keyboard and MIDI mapping.
WHY: A filed fault whose state I could not look up; not a product question.
ALT: none
IF-WRONG: STAGE a layer cleared by a stray Return.
ASK: NO technical: a look at the lane board and the key handler.
@@END

@@ASSUME X-20
ABOUT: N22
TEXT: I assume Speed runs to 10 for a clip that is not in BPM mode; the real top is measured before it is built and I tell you first if it is lower.
WHY: His 74 b names 10; what the video player can hold is not measured.
ALT: none
IF-WRONG: SMALL a lower top of one slider.
ASK: NO technical: a measurement.
@@END

@@ASSUME X-21
ABOUT: N24
TEXT: I assume every film the app writes plays at the right speed in an ordinary player; that is checked with a ten-second recording when that part is built.
WHY: A suspected fault seen in the code, not run; L6 and L88 keep film recording in the plan.
ALT: none
IF-WRONG: STAGE a recording that plays too fast or too slow.
ASK: NO technical: a test when built.
@@END

@@ASSUME X-23
ABOUT: U5
TEXT: I assume "Save without ignored controls" stays in an action's right-click menu wherever the action's button is, and makes a new, trimmed action; it changes no saved action.
WHY: L72 says "not modify during a show"; the same line says "yes. I like that." to the list that holds this entry, and question 185 kept its default.
ALT: b) It is offered only in Review, when an action is saved there.
IF-WRONG: SMALL where one menu entry works.
ASK: NO his "yes. I like that." (L72) covers the list that holds this entry; the entry records nothing and changes no saved action.
@@END

@@ASSUME X-25
ABOUT: U20
TEXT: I assume the Preferences window, the store of recorded sound and the remote-control port stay as they are; I go through them myself and ask you only if one needs a decision of yours.
WHY: These three were not swept as areas of their own; L114 wants one list of every name.
ALT: none
IF-WRONG: SMALL a name or a setting found late.
ASK: NO technical: three reads by Harmony.
@@END

@@ASSUME X-26
ABOUT: C3, C4
TEXT: I assume the preset's name comes back in the effect's header when its settings match that preset again, and a small arrow beside the Save button still opens the list of presets.
WHY: L52 says when the name goes, not when it returns nor where the list is opened meanwhile; the page's line on the Save button going away stands.
ALT: b) Once changed, the header shows Save until you save or pick a preset, also when the sliders are put back.
IF-WRONG: SMALL a name or a Save button in one header.
ASK: NO it follows a reading he left standing with L52; the presets topic carries the same point (its C-5 and C-6).
@@END

## CLOSED ASSUMPTIONS (one line each)
- X-1 -> SETTLED
- X-2 -> SETTLED
- X-3 -> MERGED 225
- X-4 -> MERGED 217
- X-6 -> SETTLED
- X-7 -> MERGED 263
- X-8 -> MERGED 229
- X-10 -> MERGED 250
- X-11 -> MERGED 252
- X-22 -> CLOSED by page 2 (X): Settled twice over: he rules "Keep Milk drop as it is" (BF265, on item 247), and the MilkDrop document is written (docs/claude/milkdrop.md, read by me: its lines 62, 89 and 97 say what Bag does and what the Jukebox's timing list really counts), so nothing is left to assume.
- X-24 -> MERGED 221
- X-27 -> MERGED 224
- X-28 -> SETTLED
- X-29 -> MERGED 220

## QUESTIONS BACK
## NAMES
@@NAME Global
MEANS: The level above the layers and the tab that holds its effects and actions; the word that takes the place of "composition" for that level.
SOURCE: his words L55 ("composition and global are interchangeable but lets move to global as that is what musicians are more used to")
@@END

@@NAME Effects
MEANS: The browser tab that lists every effect and, under each effect, its presets.
SOURCE: his words L41 ("effect tab") and binding-decisions.md:1089 ("the effects tab"); the reading that proposed the word was not named by him and stands; the on-screen name now, "FX", is replaced by it
@@END

@@NAME keyboard and MIDI mapping
MEANS: Everything about putting a button on a key or a pad and a slider on a knob; on screen the word mapping means only this.
SOURCE: his words binding-decisions.md:1099 ("r120 we call it keyboard and midi 'mapping' lets use this term") and L116 ("we want mapping files")
@@END

@@NAME Global glide
MEANS: The one slider, from instant to four seconds, that times every glide of an action: going back, a hand letting go, and a start or a Resync that would make a jump.
SOURCE: his words L64 ("the Global glide slider"); his other words for it are L66 ("the global glide back setting"), L67 ("the master action fade back time") and L70 ("a Global fade control"); that they are one slider is X-27
@@END

@@NAME tempo bar
MEANS: The row with the beat circle, play, pause, stop, the tempo number and its buttons, the nudge, tap and Resync.
SOURCE: his words L13 ("the tempo bar"); it takes the place of "tempo row" in every text
@@END

@@NAME automatic mode
MEANS: The app listens to the music and finds the tempo by itself; its opposite is manual mode.
SOURCE: his words L14 ("automatic mode", "manual mode") and L32
@@END

@@NAME Record to Clip
MEANS: The recording of the full composition's picture into a video clip that lands in a cell.
SOURCE: his words L114 ("we will record to clip or record show.") and L88 ("It must be the full screen composition.")
@@END

@@NAME Record Over
MEANS: Recording new moves over a show recording in real time from the MIDI controller; kept separately from the original.
SOURCE: his words L92 ("record over") and binding-decisions.md:500 ("record over")
@@END

@@NAME start menu
MEANS: The second small menu of a clip, which says where a fired clip starts: from the start, pick-up or relative pick-up.
SOURCE: Harmony's pick (the clip topic uses the same name); his words L102 name the two menus only
@@END

@@NAME Review
MEANS: A word that is no longer used: his earlier name for the screen that is now called Studio (long form: Show Recording Review). Use: Studio. Every old block of this topic that still says "Review" outside a quote of his (the rules of C10, C11, C12, N5, U1, U5 and U11, the internal X-12 and X-14) means Studio.
SOURCE: his words BF245 ("Let's go with studio. That's perfect.") and BF263 ("b"); they replace his earlier words of L114, where he asked for a better name himself; the first two sentences are the keyboard topic's block word for word, and that topic owns the name
@@END

@@NAME Record Show
MEANS: The recording of a whole performance, which is looked at afterwards in Studio.
SOURCE: his words L114 ("we will record to clip or record show."); "Studio" by BF245; the same words as the keyboard topic's block, so that the two cannot drift
@@END

## REACHES OTHER TOPICS (from the paper)
- A (tempo bar): BF246 and BF242 with items 217, 219, 248 and 249 were applied here in C2, C14, C15, C16 and X-5. How long his Resync holds against the app's own search for the 1 ("always", BF246, against way b's "until your first Resync") is A's to rule; C14 only points there. N9 now waits on his reply to BF266 (item 250), which A puts to him again.
- A and D (tempo stop): BF271 "Stop removes all clips from all layers" and BF250 "tempo stop stops all actions, not just global" fit the X blocks that mention stop (N8, N18: "stop still clears the layers", "stops and clears as any stop"); neither block says what stop does to actions, and none was amended. A and D rule it.
- B (previews and cue): BF247 was applied here in N2, U4 and U18; item 221 in U18; item 253 in U4; BF267 in N4. Left to B: when a previewed clip's actions start while the tempo runs; what "loaded into the layer, and we are cueing this way" covers; the reply he is owed on "come first" and on the frame rate (item 252); what and where the master cue is (BF248, item 222) -- U10 does not speak of the master cue and was not amended.
- C (presets): BF249 (a small window asks which preset version to keep) was applied here only as one more place where the app speaks by itself (C18). BF252 (item 228 b: after an action the slider glides back to its value from before the action; the preset's value for it is lost) fits C5, C7 and C8 as they stand; no amendment.
- D (actions): items 224 and 225 close what C5, C7, C8 and C9 held open (one Global glide slider; one layer switch against global actions); only C9 was amended, the others already say it. BF251 (a play-once action on a clip keeps its button on and plays again only when the clip is re-triggered) does not touch a RULE here: C8 says what the controls do, not what the button does.
- E (recordings, Studio): items 229, 257 and 262 and BF268, BF241 were applied here in C20, U21, N24 and U16. BF241 ("Maybe something close to the output monitor resolution? 30 fps is the fastest or even 15 could be ok.") stands against the accepted item 262 (a quarter as wide and as high): E rules it; N24 names no size.
- F (show file, Snapshot): item 235 was applied in U19. BF243 and BF257 make Snapshot a save of the whole show at one moment; U8's RULE only mentions the snapshot in passing and was not amended. NAMES.md line 220 (Snapshot = a still picture) no longer holds: F's.
- G (outputs): item 263, item 237 and BF269 were applied to Syphon in C19 (X3-1). What ends the hold after All Outputs Off (Restore Last Outputs, a quit) is G's.
- H (how a clip plays, codecs): BF258 was applied in C13, items 266 and 267 in C25. Left to H: a clip shorter than 4 bars; what "continue" still means once a re-trigger of the playing clip always restarts; the codec test (BF240) that N24 points to; his question "Why would we make HAP copies at all?" (BF259).
- I (effects): BF260 was applied in N13 and X-16, BF261 in U13, item 243 and BF244 in C26 and N23. What a mask is, when keying is built, what the sync control and the common macro of the beat-reading effects are: I's.
- J (keyboard and MIDI mapping): the name Studio (BF245, BF263) is J's; here one @@NAME block. N12 (no pad lights until he names a controller) stands: BF270 names no controller, it asks for both kinds of knob. U8 stands with item 269.
- K (MilkDrop, audio): BF265 was applied in U17 and closes X-22. BF271's second sentence ("A pause would not pause it unless it is connected to the BPM") fits C26 as it stands (pause holds what the beat clock controls and nothing else).

## CONFLICTS (from the paper)
- Preview on the beat or at once. New, BF247: "I think I want to change this. Previewing a clip should not happen on the beat. It should just be quick" -- against his earlier "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music" (binding-decisions.md:1134). The new words win (N2, U4, U18).
- Keying. New, BF260: "All the blend modes and keying stay, and they will be built when there is time." -- against his earlier "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." (binding-decisions.md:1185). The new words win (N13, X-16).
- The nudge amount and the show. Item 219, which he accepted with its box empty (the nudge number goes back to 0 at a start after a pause or a stop), against his earlier "good (this is just nudge amount)" on a show keeping its nudge amount (binding-decisions.md:916). No new words of his: the accepted item wins, and the kept amount has almost no life left (C15, X-5; the tempo topic carries it).
- The output is black with no clip playing (a reading he left standing, U17) against "Keep Milk drop as it is." (BF265, on item 247, whose text says a clicked MilkDrop preset still shows on the output). His words win for the coming build (U17 amended).

## NOT DONE / UNSURE (from the paper)
- The name: "Review" still stands in the RULE lines of C10, C11, C12, C20, N5, N6, U5, U21 and in the assumptions X-12 and X-13. They are to be read as Studio; I wrote no amendment for the name alone. Cheapest: one replace pass by the merge script.
- C23, U7, N15, U12 (Random) still read OPEN in spec-X.md although the slice lists their assumption X-28 as SETTLED before page 2. Nothing on page 2 touches Random, so nothing is written here. Cheapest: the ruling checks that the clip topic's block carries the settled rule.
- C5, C7 and C8 still say "is asked (X-27)" and C6 says "see X-3"; items 224 and 225 close both, and the RULE texts already say what holds, so by the rule of this paper they are not amended.
- X-5 and the nudge: I followed the tempo topic's reading (a show's nudge counts only when the show opens onto a running beat). Whether a show should keep a nudge at all is its way b; not asked.
- C25: what "continue where it left off" still does once a re-trigger of the playing clip always restarts (item 266) is not said by the item; left to the clip topic.
- N4: the wording of the reading put to him again follows the cue topic's paper (apply-B.md, item 252), read by me, not ruled yet.
- Nothing about the app was measured or run for this paper; the one file of the app's documents I read is docs/claude/milkdrop.md (for X-22).
- written 2026-10-09 18:54:45 EDT

## FOR THE PAGE RULING (from the ruling)
- Topic X puts NOTHING of its own to him: after this ruling its five written assumptions are all NO (X-5, X-13, X-16, X-17 re-written here; X3-3 as the paper has it), with the ten old internal ones untouched (X-12, X-14, X-15, X-18 to X-21, X-23, X-25, X-26); X-22, X3-1 and X3-2 are dropped. YES 0, LINE 0.
- What X's rules lean on is asked by the owners, once each; in order of weight for this topic: (1) who holds the 1 after a start press and after a Resync (tempo topic; C14, and every X rule that says a start press "is the new 1": C1, C2, C16, C22, N18, U17); (2) what the tempo stop does to action buttons (actions topic; N18, X-17); (3) how long All Outputs Off lasts (output-screens topic; C19); (4) what "adjust the sink" asks for (effects topic; C26, N23); (5) the tap (tempo topic; N9); (6) the preview's fps and size (cue topic; N4).
- Newest words against earlier ones, each to be said to him once, by its owner and not from X: a preview at once (BF247) against "triggered on the 1" (BF175): cue topic. Keying stays (BF260) against "remove the keying and slider" (BF226): effects topic. The nudge goes to 0 at every start (item 219, accepted with an empty box) against his "129 b" and "good (this is just nudge amount)": tempo topic, one line. All Outputs Off holds (BF269) against "I expect the show to remember the outputs connected" (BF216): output-screens topic. The app places the 1 (BF246) against "click on play or any clip is the new 1" (BF166): tempo topic. A clicked MilkDrop preset shows with no clip playing (BF265) against the black output he had left standing: sources topic.
- The paper's CONFLICTS section is carried over as it is and lacks the fourth and fifth of these (checker F7); they are not X's to add.
- Written while the effects ruling had its verdicts but not yet its blocks: C26, N23, N13, U13 and X-16 agree with those verdicts (the sync is asked as "adjust the sink"; the order of work is internal there; the two gaps of BF261 are its two lines for him). If its blocks say more, these five X blocks are read with them. U17 follows the sources ruling's item 247 as it stood when this ruling was written.
- MERGE, dry run made for this ruling: all 30 amendments of X apply. U18 is now amended by the cue topic alone and the shared sentence of C13 by the clip topic alone; X keeps C13 1 (another sentence). The one amendment that dry run could not apply was topic I's AMEND R214 1, which the effects ruling says it drops (its H1).
- MERGE: after all amendments "Review" still stands, outside quotes of his, in seven RULE lines of spec-X (C10, C11, C12, N5, U1, U5, U11: counted by a script; the paper's list named C20, N6 and U21, which do not carry it, and missed U1 and U11) and in the internal X-12 and X-14; the new @@NAME Review says how they are read. One replace pass at the merge would be cleaner; no amendment was written for the name alone.
- C18 (messages) now counts as the keyboard topic's item 209 does after its ruling: eight messages, and the questions before a replacement apart. If item 209 changes again, C18 follows it.
- N18 and X-17 now give a stop pressed on a stopped tempo its work (BF250, BF271). The tempo topic's internal A-16 still says "a second stop" changes nothing: it should follow, or say why not.
- C23 (Random) is closed in X by BF219, as the ruling that made page 2 settled it; U7, N15 and U12 still say "asked once" and point to C23, which now says settled. The clip topic's item 201 carries the rule and still names H-1 as asked: the same stale pointer there.
- N12 stands: asked on page 2 which MIDI controller he uses (item 270), he named none (BF270); pad lights for the new buttons keep waiting on that. One line for the keyboard topic if the next page has room.
- Still owed by Harmony before the plan is called clear, none of it for him: the sweep of the Preferences window, the store of recorded sound and the remote-control port (U20, X-25).
- Nothing in X is measured: N4, N5, N6, N20, N21, N22, N24 stay Harmony's own looks and measurements.
- Ruled 2026-10-09 19:56:23 EDT by the ruling seat of topic X. Read-only; nothing built, run or committed.

