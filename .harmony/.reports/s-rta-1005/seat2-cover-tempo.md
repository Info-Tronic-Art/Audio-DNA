# seat2 cover-tempo (blind seat, round 2): completeness of the tempo area on the ONE list

VERDICT: PASS_WITH_NITS is NOT earned; verdict FAIL (fixable by edits listed below): 3 MUST, 7 SHOULD, 8 NIT. Most of the area sheet's HIS points have a home; the holes are (1) a "/2 x2 setting saved in the show" that the adopted row ruling deletes, (2) the tempo number while the beat is stopped (the list and the adopted ruling disagree), (3) a question text that contradicts R193(b).
Labels: VERIFIED = I opened it (file:line or quoted words). INFERRED = reasoned from lines I read. UNKNOWN = not established.
Read-only; nothing built, run, launched, committed. No lane worktree read. List = boris-all-items.json (44 questions 172-215, 87 readings R127-R213, 28 decided lines, 31 design items, 24 conflicts, 14 intro lines).

## A. What I re-derived (not leaned on)
V1 VERIFIED TopBar.h:150 `juce::ToggleButton manualModeBtn_{"Manual"}`; TopBar.cpp:97-119: there is a tick box "Manual", no control called "Auto". Q189 says "press Auto again".
V2 VERIFIED MainComponent.cpp:606-628: ">" "||" play/pause every shared layer's playing clip; "[]" = routineEngine_.stopAll() only. 7909-7918: key target "Play / Pause" = audio file transport; "Stop" = routines only. (area sheet lines 2, 19 hold.)
V3 VERIFIED MenuBarModel.cpp:153-158 ("Edit Keyboard Shortcuts...", "Edit MIDI Mappings...", "Stop All"); BindingOverlay.cpp:66-68 ("Keyboard Binding Mode — Click a target, then press a key"; targets "Tap Tempo", "Resync", "Play / Pause", "Stop" at MainComponent.cpp:7633-7644); MidiLearnOverlay.cpp:95; MainComponent.cpp:7333-7335 ("Stop All" only calls exitAllBindingModes), 7338-7341 ("Export Bindings"). R148 and R197(i) are TRUE to the source. His "keyboard and MIDI mapping" instruction: R148 is the one reading, intro line 1 quotes "ask me questions about anything that is unclear when you are ready". My scan of every text field (outside quoted source strings) for binding / overlay / key list / pad list / shortcut / MIDI learn: no violation ("Overlay" in Q203 is a blend name).
V4 VERIFIED MilkDropBrowser.cpp:563-575 + :597 (Jukebox list "4 beats".."60 sec", bars[] = {1,2,4,8,8,16}); PresetSelector.cpp:78-85 and 168-170 (a switch after transitionBars_*4 counted BAR crossings) -> 4, 8, 16, 32, 32, 64 bars: R212 is right (the two area sheets' disagreement is settled for the Jukebox). MilkDropBrowser.cpp:612-615 playlist list "4 beats".."32 beats"; playlistBlendSlider_ has no onValueChange; the value goes to Clip::playlistBlendSeconds (MainComponent.cpp:1354) which nothing else reads (grep over src): decorative, VERIFIED.
V5 VERIFIED MainComponent.cpp:357 "Beats per Image" (slideshow row); Q211 A removes that row (covers it).
V6 VERIFIED .harmony/.reports/s-rta-1004/ruling-nudge-row.md:460 (S3r: Composition.h bpmMultiplier "the member, its reset and its save line removed; the load line reads and drops the key"; "the five-button code ... removed"), :71-85 (/2 = half the tempo, x2 = double, never move the "1"; hand tempo 30..400; Tap/REST/OSC/Link keep the 60..200 fold).
V7 VERIFIED .harmony/.reports/s-rta-1004b/ruling-nudge-row2.md:34-48 (table: tempo number "live, never 0" in STOP and PAUSE; beat-driven effects, signals, mappings "stand on the 1" (stop) / "stand where they were" (pause); autopilot waits), :245 (seven target titles "Beat play", "Beat pause", "Beat stop", "Tempo -", "Tempo +", "Tempo /2", "Tempo x2").
V8 VERIFIED boris-feedback-backlog.md:800 and binding-decisions.md:1037: his SECOND line on Manual, "ok what is your question about r76 having to do with manaul?" (the area sheet says no line of his follows R76; one does). Q189 quotes only the first line.
V9 VERIFIED boris-clarify-125-129.md:32-43 (tempo-row R74-R84 as told).

## B. Point-by-point table (area-tempo.md parts 4, 5, 3 and CORRECTIONS)
Cover rule: covered only if his answer (or silence) lets a builder go on without guessing.

### Part 4 HIS
| point | home | verdict |
|---|---|---|
| U-HIS-1 wait between press and "1" | Q173 (+R168, R128, DES 1 for the cell's look) | COVERED |
| U-HIS-2 fire while stopped: which "1", which tempo, which fires | R139 (b), R174(d), Q175, DEC 1 (who starts it); the "nudge keeps its number" in R174(d) | COVERED (nudge on the "1" a fire makes: NIT-6) |
| U-HIS-3 paused, fired clip not in BPM mode | Q174 | COVERED but Q174 A text is wrong (MUST-3) |
| U-HIS-4 Auto vs Tap / Resync / steps | Q189 (+Q190) | COVERED; text says "press Auto again" (SHOULD-4) |
| U-HIS-5 Tap moves the beat or not | Q190, DEC 3 (rolling last eight) | COVERED (Q190 C vs R176 f: NIT-1) |
| U-HIS-6 nudge vs Delay, Resync zeroing a saved nudge, step | R177, R176(e), CONF 15 | COVERED |
| U-HIS-7 Link | R178, R169, Q215(b), DES: none needed | COVERED (Link off leaves Manual: M-3, NIT-4) |
| U-HIS-8 tempo range, greying | R176(d) (reading) | COVERED as a reading; but R163/R188 contradict it (MUST-1) |
| U-HIS-9 global pause-all gone | R176(b) | COVERED |
| U-HIS-10 clip length bars vs beats | Q202 | COVERED (his_words lack 47 / 812-814: NIT-5) |
| U-HIS-11 which places say beats / bars | R196 (+R212, Q211, R213(c)) | COVERED |
| U-HIS-12 MilkDrop timing labels | R212 (right, V4), R202(c) | COVERED for the Jukebox; playlist list and playlist Blend missing (SHOULD-6) |
| U-HIS-13 Stop vs Ignore Column layer | R176(a) | COVERED (INFERRED, flagged) |
| U-HIS-14 tempo row in a recording | R185(b), R167 | COVERED |
| U-HIS-15 Quantize in the review screen | R166, R135 | COVERED |
| U-HIS-16 "press start" | Q172, R140 | COVERED |
| U-HIS-17 row Play and an audio file | R204(c) | COVERED |
| U-HIS-18 names on screen | R148, R197, R120 chat block | COVERED (R120 block says two new entries; ruling has seven: NIT-2) |
### Part 4 TECHNICAL (homes only where he would notice)
| U-TECH-1 BPM-mode wait mechanism | architects; his side Q173 | n/a |
| U-TECH-2 fire starts a stopped beat | DEC 1, R139 | COVERED |
| U-TECH-3 nudge S1 STOPs (STOP-N2: in Auto a small later nudge holds one hop on 199 of 200 beats) | none on the list; architects | NIT-8 (a measured stutter he could see) |
| U-TECH-4 transitions (pause while stopped, stop while paused, second stop) | Q172 A states pause-from-stop; the rest are no-ops | NIT-7 (one decided line) |
| U-TECH-5 launch with no tempo | Q175, R163 | COVERED (but see SHOULD-2) |
| U-TECH-6 recorder vs timer | R185(b) | COVERED |
| U-TECH-7 stale docs | none (not his) | n/a |
| U-TECH-8 old shows / takes | R143, R116 chat block | COVERED |
| U-TECH-9 Link under the row | R169 | COVERED |
### Part 5
| 154 residual | Q173 | COVERED |
| R139 / R140 / R141 / R142 / R144 | R139, R140, R141, R142, R144 | COVERED |
| R114 | chat block R114, R142, R163 | COVERED |
| R76 | Q189 (explains "Manual" in its own words) | COVERED |
| R74 (pause never touches nudge) | his 129 b covers stop and play; pause only in the row ruling | partial, NIT-3 |
| R77 | R176(d) | COVERED |
| R78 ("/2" "x2" never move the "1") | R174(b) says they move the "1" a WAITING clip waits for; nothing says they leave the beat's own "1" | partial, goes into new R215 |
| R79 | void by 146 b; R176(g) | COVERED |
| R80 (glyph meaning "<<" later, ">>" earlier) | R176(e) gives the sign (plus = earlier) not the glyphs | NONE -> new design item |
| R82 (launch starts Running) | R175(3), R163 | replaced by an INFERRED reading: SHOULD-2 |
| R84, R115 | R147 | COVERED |
| R109 | Q180 | COVERED |
| R110, R112, R113, R119 | R147 | COVERED |
| R120 | chat block R120, R148 | COVERED (NIT-2) |
### Part 3
O-01 R176(a)(b) COVERED. O-02 Q174. O-03 R175(2),(3). O-04 pads go (DEC 12), R139. O-05 R175(4). O-06 R128(b), R166. O-07 DEC 12, R158. O-08 R120 chat block, R199(d), Q180. O-09 his "46 default" (answered; R177 says "everything that runs on the beat"; NIT-3b). O-10 R176(e). O-11 **NONE** (the five buttons /4 /2 x1 x2 x4 are never mentioned on the list; see MUST-1). O-12 Q202. O-13 R193(h). O-14 R175(1). O-15 R212, R196. O-16 R176(g). O-17 Q189. O-18 not his. O-19 R177 + R174(d) (NIT-6). O-20 Q189.
### CORRECTIONS
C-1, C-2 (misattributions) n/a for him; I checked that no list item quotes Harmony's consequence text as his in this area. C-3 (REST / OSC / Link also switch the tracker's Manual on) not his; NIT-4. C-4 handled by R176(b).
M-1 Q190 + DEC 3. M-2 Q190 B text. M-3 R178/R169/Q215 (NIT-4). M-4 Q175 (+R139) COVERED. M-5 R213, R158, DEC 1 COVERED. M-6 **NONE** (Q189's why-line says "the same answer holds for the number you see while stopped or paused" but its situation is a hand press; the untouched-Auto case is not asked) -> MUST-2. M-7 R128(a) + DEC 5 COVERED (the detector's "1" is a guess in Auto: only Q189 A makes a Resync stick). M-8 R144, R159, R135 COVERED.

Counts: 81 sheet points checked (18 HIS + 9 TECH + 22 part 5 + 20 part 3 + 4 C + 8 M); NONE = 3 (O-11, M-6, R80); partial = 4 (R74, R78, U-HIS-12 playlist, U-TECH-4).

## C. FINDINGS (exact fixes)

### MUST
MUST-1 (R163, R188(a), and NEW R215). R163 says "It does hold its /2 and x2 setting (today already)"; R188(a) lists "the /2 x2 setting" as held by the show. TODAY the five buttons "/4" "/2" "x1" "x2" "x4" only write a number that the show saves and nothing reads (TopBar.cpp:403, 425-426; Composition.h:754, 891; sheet line 11): they change no tempo. The adopted row ruling removes the saved number and makes "/2" and "x2" one-press halve/double of the tempo (V6). He is told a working saved setting exists, and never told the five buttons go.
FIX (a) R163: replace the sentence by "It holds its nudge amount (your 63) and nothing else of the tempo row (R215)." (b) R188(a): delete "and the /2 x2 setting". (c) NEW R215 [A], label "your picture (the row has only /2 and x2) + INFERRED from the row ruling; today VERIFIED": "Today the five buttons beside the tempo, '/4' '/2' 'x1' 'x2' 'x4', change nothing at all: the app keeps which one you pressed and nothing uses it. In your Resolume picture the row has only '/2' and 'x2', so the five go and the two stay. '/2' halves the tempo number and 'x2' doubles it, once per press; neither moves the '1'; '/2' is greyed below 60 and 'x2' above 200 (R176 d). Nothing of them is saved with the show. A tap and the app's own listening stay between 60 and 200; a number you set by hand does not. Going back from Manual to listening folds a number outside 60 to 200 on the first beat." (also covers R74 and R78).

MUST-2 (NEW Q216; R176(a)). R176(a) says "the beat stops and the tempo number stays"; the adopted ruling says the number is "live" in stop and pause (V7); Q189's why-line only half-covers it. With the app listening and the beat stopped between two songs, whether the number follows the new song decides the tempo at which the next fire starts the beat. A builder would guess.
FIX: NEW Q216 [A] "While the beat is stopped or paused: does the tempo number follow the music?" Situation: "The app is listening (Manual is off). You press stop between two songs; the next song is faster than the number on the row. You fire a clip." A (default): "The number keeps following the music while the beat is stopped or paused, so your fire starts the beat at the new song's tempo. [your words + mine] Your 'tempo is not lost' says it is kept, not frozen; a held number would start the new song wrong." B: "The number holds where you stopped it until you play, tap or type; the music's tempo is ignored while the beat does not run." Why: "It decides at what speed the first fire of the next song starts. With Manual on, the number always holds." Reword R176(a) "the tempo number stays" to "the tempo number is not lost (what it does while the app is listening: question 216)". Add 216 to topic A in intro line 11 and the "first builds" line 9.

MUST-3 (Q174 A). Q174 A says "while the beat is paused nothing you fire moves, whatever its mode". R193(b) says a still picture, generated source, MilkDrop, camera and effects-only clip "fired during a pause show and move, whatever you answer to 174". The list contradicts itself; he may choose A believing sources freeze.
FIX Q174 A text: "It shows, standing on its first frame, and plays when the beat plays: while the beat is paused no video or picture sequence you fire moves, whatever its mode. (A still picture, a generated source, MilkDrop and the camera have no playback and keep moving: R193 b.) This is your sentence as written." Same limit in R139(c): "a clip you fire shows and does not play" -> "a video or picture sequence you fire shows ...".

### SHOULD
SHOULD-1 (NEW design item, topic A). The tempo row itself has no design item: where "Manual", "Link", the word SEARCHING / LOCKING / LOCKED and the BPM number's editing sit among the thirteen cells of his picture (his r85: "use this similar layout ... where it fits"; row ruling puts them to the right, widths COMPUTED not measured, ruling-nudge-row2.md section 0); which glyphs the two nudge buttons wear ("<<" ">>" told as R80 and never corrected; his picture shows arrows onto a bar); how the circle and the three cells look stopped / paused / running. FIX: add design item "The tempo row": variants (1) the thirteen cells of your picture, then Manual, Link and the state word to the right of RESYNC; (2) the state word under the tempo number, Manual and Link as two small tick boxes at the row's right end; (3) your picture alone, Manual and Link in a small menu. Plus a line in the list "comes next as pictures": nudge glyphs "<< >>" or your picture's arrows; the three cells lit stopped / paused / running.

SHOULD-2 (NEW Q217; R163, R175(3)). R82 (told, "the app always launches with the beat running") is replaced by R163 ("When the app starts, the beat is stopped") on INFERRED grounds only, and R163 does not say what it costs: at launch, with music playing and nothing pressed, everything that follows the beat (signals on the beat, beat-driven effects, autopilot count, MilkDrop Jukebox count) stands on the "1" (V7); today it runs once the app has locked. FIX: NEW Q217 [A] "The beat when the app opens". Situation: "You open the app, the music is playing, you have pressed nothing." A (default): "The beat is stopped and waits for your fire, play or Resync; the circle stands still and everything that follows the beat stands on the '1' until then. [your words + mine] Your 'load up the clips and press start'." B: "As today: the app finds the tempo in the music and the beat runs by itself; stop and pause are there when you want to hold it." Why: "It decides whether beat-driven looks move before your first press of the night. Question 175 only matters under A." Star R163.

SHOULD-3 (NEW R214). Nothing on the list says in his words what stands still when the beat stops or pauses, beyond the circle, BPM-mode clips (R176) and actions (R158) and autopilot (R213(b)); oscillators and beat-driven sliders / effects (V7) are not named. FIX: NEW R214 [A], label "INFERRED from the row's ruling; today there is no stop": "Stop: the circle stands on the '1' and every clip leaves. Everything that follows the beat stands on the '1' until the beat runs again: signals that run on the beat (oscillators), sliders and effects driven by the beat, the autopilot's count, MilkDrop's Jukebox count. Pause: the same things stand where they were, and a clip that is not in BPM mode goes on. In both, what the music itself drives (loudness, bass, tones) keeps moving."

SHOULD-4 (Q189 A, B, C, situation). The control is a tick box "Manual" (V1); there is no "Auto" button. FIX: situation "(Manual is off)" instead of "(Auto)"; A: "...stay where you put them until you switch Manual off again."; add to his_words: "ok what is your question about r76 having to do with manaul?" (V8) so he sees both his lines.

SHOULD-5 (intro line 5 "Look at those first"). R128, R139, R163 and R212 hold a "Mine:" or INFERRED part he would see on stage (any clip starts a stopped beat; launch stopped; re-fire restarts on the "1"; the Jukebox labels) but carry no star and are not in the list of fourteen. FIX: add R128, R139, R163, R212 (and new R214, R215) to the stars and to the sentence ("Eighteen ... : R128, R129, R131, R139, R149, R160, R163, R168, R172, R174, R181, R186, R189, R199, R205, R210, R212, R213, R214, R215"; count it by script).

SHOULD-6 (R212 / R202(c)). U-HIS-12 asked whether the Blend sliders are among his "seconds". R202(c) says "The Blend slider stays in seconds" without telling him the clip playlist's Blend slider does nothing today (V4), and R212 covers only the Jukebox list, not the clip playlist list ("4 beats" to "32 beats", real beats, V4) that R196 also sends to bars. FIX: R212 add: "The list in a clip's own playlist reads '4 beats' to '32 beats' and those are real beats; they read '1 bar', '2 bars', '4 bars', '8 bars' (4 beats = 1 bar). Mine." R202(c) add: "The Blend slider of a clip's playlist does nothing today (the Jukebox's does fade); it is made to fade, or hidden: say which." (or add the playlist Blend as an eighth item of Q212).

SHOULD-7 (R174(d), R177). Not said whether the "1" a BPM-mode clip waits for is the nudged "1" (his words BD:859-862 "triggers a clip, that is not affected" were written before 154 made the wait implicit). FIX: R174(d) append "Mine: the '1' a clip waits for is the nudged '1', like everything else on the beat (R177)."

### NIT (stay in the paper)
NIT-1 Q190 C vs R176(f): C ("the first tap of a run is also the '1', as a Resync would be") contradicts "a tap never starts the beat" when stopped: add "(a tap still starts nothing while the beat is stopped)".
NIT-2 R120 chat block says "two new entries" ("Beat stop", "Beat play"); the ruling has seven new titles (V7); say "new entries for the tempo row ('Beat play', 'Beat pause', 'Beat stop', 'Tempo -', 'Tempo +', 'Tempo /2', 'Tempo x2')". DES 29 should carry them.
NIT-3 R176(e): add "pause never touches the nudge" (R74) and "the circle moves with the nudge (your 46)".
NIT-4 Link: R169 omits Tap, "-", "+", "/2", "x2" and the nudge under Link (ruling greys the five hand-tempo buttons, NR:83); a Link switch turned off leaves the tracker on Manual (MainComponent.cpp:593 vs 5838 per the sheet, not re-read). Moot while Link is dimmed (R178); revisit only if Q215(b) names Link.
NIT-5 Q202 his_words: add "always work with multiples of 4. If it is uneven, then move the outpoint in" (BD:812-814) and "47 default" (BD:865-866).
NIT-6 DEC 4 and Q173/Q191 do not name the song-structure detector's bar-counter restart at a drop (sheet line 26): in listening mode "the 1" can move at a drop; only Resync fixes it (Q189 A).
NIT-7 Add one decided line: "Pause on a stopped beat holds it at its start; stop on a paused beat stops it; play while it runs, or a second stop, changes nothing."
NIT-8 Nudge S1's STOP-N2 (in listening mode a small later nudge holds the beat one hop on 199 of 200 beats, MEASURED by the builder, RNS:13-14): a visible stutter risk; architects, but list it as a decided line with its consequence if it is not fixed before S2.

## D. Today-claims on the list in this area, compared with sheet part 1 and corrections
OK: Q173 (old clip stays today: FT:46 per sheet), Q175 premise (line 4), Q189 why (lines 5, 8), Q190 B (line 7, M-2), R148 / R197(i) (V3), R204(c) (line 19), R212 (V4), R176 today-parts. CONTRADICTIONS: MUST-1 (R163, R188a: "/2 x2 setting today already"), MUST-3 (Q174 A vs R193 b). Not a contradiction but a trap: R82 vs R163 (SHOULD-2).

## E. UNKNOWN
- Whether the tracker's tempo updates are applied while the beat is held (Q216 asks; cheapest: ask the architect, or read the nudge lane's state table, which I did not read).
- Whether R185(b) (a recording keeps play / pause / stop) conflicts with NA-4 "the timer is not in a take" (the list chose R185(b); NR:346-350 is not re-read by me).
- Which picture of his shows Random with Interval and Distance (Q201 cites "your picture"; the Resolume BPM-panel picture of 21:33 has none: BL:797); outside my area, flag for the clip-transport seat.
Mon Oct  5 16:47:47 EDT 2026
