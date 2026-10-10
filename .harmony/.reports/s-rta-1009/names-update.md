# NAMES.md update from names3-all.md (s-rta-1009)

File changed: /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md (375 -> 388 lines). Only NAMES.md and this paper were written. All 41 blocks done. 35 distinct names (Studio x3, Review x3, Render x2, Record Show x2 are one row each). NAMES.md has no count of names in its header, so none was corrected. The dated line is in the header, line 5.

## CHANGES

automatic mode | CHANGED | meaning adds the "1"; source adds BF246 (his words)
preview mode | CHANGED | now plays from the click, never waits for the beat (BF247)
preview monitor | CHANGED | adds his two variants "preview/cue monitor", "preview-cue screen"; BF248, BF267 inferred
cueing | ADDED | section 3 after cue mode; meaning is Harmony's reading (B3-4)
master cue | CHANGED | meaning widened to effects plus what global actions do; waits on item 222
output monitor | CHANGED | adds his variant "output preview screen" (BF267, inferred)
Studio (topic D) | RENAMED from Review | one row, section 7; "Show Recording Review" listed in Replaces
Global glide | CHANGED | "one slider is assumed" note removed; only-one is his (item 224)
Ignore Global Actions | CHANGED | layer's own and clips' actions keep playing; "open until you say" removed
play-once action | ADDED | section 6 after Loop (action); name is Harmony's pick, clip behaviour his (BF251)
Stop actions | CHANGED | adds: tempo stop does the same; darkening of clip buttons still asked
Ignore Actions | CHANGED | "(and on a layer)" removed; layer switch is Ignore Global Actions
Studio (topic E) | RENAMED from Review | same row as the topic D block; no contradiction
Studio picture | RENAMED from Review picture | picture made fresh from the recording; outputs show it too
low-resolution show recording | CHANGED | 10-minute pieces, no sound, next to every recording, reference only
Render | CHANGED | "in full quality", in Studio, his "HD render"; "my pick" removed
Snapshot | CHANGED | now a save of the whole show plus output picture; old still-picture meaning has no name
layer strip | CHANGED | his word (BF256); meaning rewritten from block; "my pick" removed
layout | CHANGED | Review -> Studio; wording of the block taken
All Outputs Off | CHANGED | adds: nothing comes on by itself afterwards (his way b, item 265)
Restore Last Outputs | CHANGED | adds: also after All Outputs Off and after a quit; source R191 i, BF216, BF269
codec test | ADDED | section 7 (last row); Harmony's pick from BF240; section is a judgment call
Mask | ADDED | section 1 after Keying; his words BF260
Moving mask | ADDED | section 1 after Mask; what makes it move is Harmony's reading
Keying | ADDED | section 1 after Blend mode; replaces retired row "K slider, Keying", which is deleted
Envelope on the beat | ADDED | section 5 after Envelope; name is Harmony's pick
Envelope on the playhead | ADDED | section 5 after Envelope on the beat; name is Harmony's pick
Sync | ADDED | section 4 after Dry / Wet; kept apart from the old sync dial (Delay) and BPM Sync
Common macro | ADDED | section 5 after macro knob; his "maybe"
Falloff | CHANGED | goes up at once, comes down slowly, instant to 2 seconds (item 268)
Studio (topic J) | RENAMED from Review | same row as the topic D and E blocks
Endless | ADDED | section 9 after the mapping screen row; Harmony's pick from BF270
Review (topic J) | ADDED (section 11, retired word) | "Use: Studio"; old row of section 7 became the Studio row
Record Show | CHANGED | adds "whole performance" and "looked at afterwards in Studio"
track | CHANGED | one row of Studio: one recorded button or slider
Show Recordings | CHANGED | opened in Studio; source now cites L6
audio file | ALREADY RIGHT | block equals the existing row; no edit
Review (topic K) | same as the topic J block | one row in section 11
Render (topic K) | same as the topic E block | one row in section 7; the two blocks agree
Review (topic X) | same as the topic J block | its extra sentence (old rules C10..U11, X-12, X-14 mean Studio) is in the row
Record Show (topic X) | same as the topic J block | quote "we will record to clip or record show" added to the source

Counts by distinct row: CHANGED 20, RENAMED 2 (Studio, Studio picture), ADDED 12 (11 new rows plus the retired-word row Review), ALREADY RIGHT 1. Total 35. The blocks that are repeats (6 blocks) fall on rows already counted.

## OTHER ROWS TOUCHED

(line numbers are after all edits)
- 5 (header): the dated line 2026-10-09 (s-rta-1009) added; it also says the Re-check notes at the end keep the names of 2026-10-08.
- 23 live window: "as against Review" -> "as against Studio". His quote "live and recording review mode" untouched.
- 67 clip in point, clip out point: "Review's In and Out points" -> "Studio's".
- 99 timeline (lower case): "in Review" -> "in Studio"; the long form "Review timeline" -> "Studio timeline".
- 118 Quantize: "used only in Review" -> "Studio". His quote "recording review screen" untouched.
- 202 action save window: "an action in Review" -> "in Studio".
- 204 section 7 heading: "Recording and the review screen" -> "Recording and Studio" (lower-case, missed by the first grep, found by a case-blind one).
- 221 In and Out points: "timeline of Review" -> "of Studio".
- 227 recorded show file: "from which Review opens" -> "Studio opens".
- 238 Output menu: "...and Snapshot" -> "...and the old still-picture command (no name ruled for it yet; Snapshot now means something else)". Descriptive words, not a new name.
- 304 Load Take..., Play Take (section 11): "played in Review", Replaces "Review" -> Studio.
- 307 replay window and 308 recording review screen / mode, review screen (section 11): "Use: Review" -> "Use: Studio"; Replaces -> Studio; "(long form: Show Recording Review)" dropped there (it stays in the Studio and Review rows).
- 312 display screen (in Studio) (section 11): "in Review" -> "in Studio"; "Use: Studio picture". Row 313 "Review picture" added as a retired word.
- 324 top-bar Quantize box, Snap box (section 11): "lives in Review" -> "Studio"; Replaces "Quantize (Studio)".
- 353 Notes, Open names: "Review picture" -> "Studio picture".
- 354 Notes: the line "Review: his L114 asks for a better name ... keep Review until he says" was now false; replaced by "Studio: his words BF245 and BF263 (2026-10-09) took the name; it replaced Review, which is in section 11."
- Deleted: section 11 row "K slider, Keying (modes and slider)" (replaced by the Keying row in section 1, as the block orders).

## VERIFY

`grep -n "Review"` gives 11 lines: 5 213 216 229 306 313 354 374 375 379 382.
- 5: the dated line, "Review -> Studio" (a "was called Review" note).
- 213: Studio row, Replaces cell "Review (long form Show Recording Review)" (was-called note).
- 216: Studio picture row, Replaces cell "Review picture" (was-called note).
- 229: Render row, inside his quote "...from the Recording Review" (his words, untouched).
- 306: the retired-word row Review (section 11).
- 313: the retired-word row Review picture (section 11).
- 354: Notes line "it replaced Review" (was-called note).
- 374, 375, 379, 382: Re-check (s-rta-1007) history bullets dated 2026-10-08. They keep that day's names on purpose; the dated line 5 says so. Not rewritten, because they record what was checked then.

`grep -c "Studio"` = 25 lines.
`wc -l` = 388 (was 375).
Table check: every table row still has exactly 4 cells (awk found none with another count).
Row check by `grep -F "| <name> |"` at line start, line numbers: automatic mode 90, preview mode 131, preview monitor 125, cueing 130, master cue 133, output monitor 124, Studio 213, Global glide 197, Ignore Global Actions 200, play-once action 196, Stop actions 198, Ignore Actions 199, Studio picture 216, low-resolution show recording 228, Render 229, Snapshot 230, layer strip 32, layout 22, All Outputs Off 247, Restore Last Outputs 246, codec test 231, Mask 61, Moving mask 62, Keying 60, Envelope on the beat 168, Envelope on the playhead 169, Sync 145, Common macro 173, Falloff 179, Endless 259, Review 306 (section 11), Record Show 209, track 217, Show Recordings 211, audio file 290. Each of the 35 distinct names has exactly one name row (a plain count shows 2 to 4 for some names only because the name also sits in other rows' Replaces cell).

## UNSURE / NOT DONE

1. The old meaning of Snapshot (one still picture of the output as an image file) has no name now. The Output menu row (238) says "the old still-picture command (no name ruled for it yet)". Needs a ruling. Also open: whether that menu item stays at all.
2. Studio picture: the old row said it "can be sent to an output monitor or resized by dragging the line between it and the tracks". The block's meaning ("made fresh from the show recording; the outputs show it too") replaced it, so the drag-to-resize clause is gone. Restore it if still true.
3. Keying: Boris's L111 ("I want to remove the keying and slider") is reversed by BF260. I deleted the retired row; the Keying row does not mention the earlier removal. The on-screen letter "K slider" is now neither a retired word nor ruled; the Keying row says "its keying slider".
4. Transparency slider row (still says "The one slider of a layer that sets how strongly it shows") sits next to Keying and its keying slider; not touched, not contradicted, but a reader may ask which slider is "the one".
5. codec test: placed in section 7 (its last row). It is about playing and recording video, so section 7 or section 10 would both fit. Judgment call.
6. The Notes line "Open names (my pick, not yet his)" is not a full index, so I changed only "Review picture" -> "Studio picture" and did not add the new Harmony-pick names (Envelope on the beat, Envelope on the playhead, play-once action, codec test, Endless). Ignore Global Actions was already in that line.
7. The Review row (section 11) carries the topic X sentence about old papers (rules C10, C11, C12, N5, U1, U5, U11, X-12, X-14 mean Studio). It is about other papers, not Boris's vocabulary. Cut it if unwanted.
8. audio file: block says he writes "audio clips" (BF272), which is not a new name; no row for it. Whether an audio file can also be a clip is asked (K3-1).
9. The Re-check (s-rta-1007) bullets still say "Review" and "Review picture" (history of 2026-10-08, left as written; the dated header line explains it).
10. Source cells cite "BF" numbers and "item" numbers (BF240-BF272 are the 2026-10-09 answers); the dates of BF161 and BF216 are not stated in the blocks, so BF161 is filed under the 2026-10-07 date of its twin quote at L6, and BF216 is left undated.
11. Master cue meaning and "Ignore Global Actions" being one thing with L7/L73 stay as the blocks give them; no block contradicts another block or an existing row.
