# seat2 cover-sources-auto (round 2): sources, what a clip holds, the automatic features
Written: Mon Oct  5 16:51:59 EDT 2026 (read-only run; nothing built, run, launched, committed; no lane worktree opened)
Labels: VERIFIED = I read it (file:line / quoted words). INFERRED = from what is named. UNKNOWN = with the cheapest check.
Read: area-sources-auto.md (parts 1-6 + CORRECTIONS C1-C6, M1-M8), boris-all-items.json (all 44 questions, 87 readings, 28 decided, 31 design, conflicts, area_points), source files named below.

## VERDICT: FAIL (2 MUST, 9 SHOULD, 6 NIT). Every HIS point of the sheet has a home, but two homes decide against his own words or against each other, one reading is wrong about today, and four things on his stage have no home.

## A. COVERAGE TABLE (point -> covering item; C = covered, P = partial, NONE)
Covered means: his answer (or his silence on a reading) lets a builder go on without guessing.

### Part 4 HIS
| Point | Home | Verdict |
|---|---|---|
| H1 camera | Q212 (a) "later" (parked by his letter). BUT the top-row "Camera" list (the only working camera) is in the old row that Q211 A removes ("the old row at the top goes"), while Q212 A says the camera stays as is | P -> finding 1 (MUST) |
| H2 ISF | Q212 (b); the "Import Successful/Rejected" boxes: Q209 A (a window you opened may answer) | C |
| H3 genre | Q212 (c) | C (parked) |
| H4 structure scenes | Q212 (d); oscillator "switch" ruled and carried by R195 (b); Jukebox reaction R202 (d) | C (but R202 d inexact: finding 7) |
| H5 smart autopilot | Q212 (e) | C (parked) |
| H6 Composition-tab Autopilot block | Q212 (f) | C (parked) |
| H7 autopilot counts in bars | R196 + R213 (c) | C |
| H8 autopilot vs tempo row and "the 1" | R213 (a)(b) + decided 1 | P -> finding 8 (count shorter than a bar; End of Video during pause; where the count starts) |
| H9 random family | Q201 (loop mode, his words), R203 (a) autopilot random, R202 (b) MilkDrop Random/Bag, Q212 (g) effect randomizer | C |
| H10 MilkDrop modes/units/triggers | R202, R212, R196; Playlist triggers have no UI (C3) and are not named | P -> finding 6 (the clip's own Playlist list) |
| H11 preview of a source / MilkDrop; what a source cell shows | decided 8, R201 (b)(c), R150, R151 | P -> finding 4 (R201 c is wrong) and finding 10 (a design item for the source cell) |
| H12 slideshow = which thing | Q211 | P -> finding 5 |
| H13 many pictures dropped | Q211 (folder of twenty) | P -> finding 5 (no "one clip per picture" option; a folder cannot be dropped today) |
| H14 audio input choice | R204 (a), Q215 (c) | C |
| H15 show remembers audio setup | R188 (e) | C |
| H16 which audio facts he sees | R204 (d) | P -> finding 11 (hidden panel not named) |
| H17 word "preset" here | R197 (e) (Feedback list stays), R211, R202 (151 A) | C (wording NIT 3) |
| H18 replacement for instant recall | R211 ("a column is your scene") | C, but R211 states today wrongly: finding 2 (MUST) |
| H19 pause/stop/tempo for generated pictures | R193 (b) ("mine"), BD:1009 side | P -> finding 3 (MUST: two of his own statements, one chosen silently) |
| H20 firing a still/source while stopped | R139 (b) "any clip does this" (his literal words) | C |
| H21 text source | R201 (d), Q215 (e) | C |
### Part 4 TECHNICAL (T1-T10)
T1 one source instance per type: decided 8 + R201 (c) (c wrong, finding 4). T2 ISF persistence / T3 camera as clip / T5 two global chains / T6 dead keys / T10 auto-preset consumer: parked with Q212, architects. T4 autopilot stuck on a paused/stopped Source clip: NONE, a visible consequence -> finding 9. T7 docs disagree: not his. T8 audio persistence: R188 (e). T9 count origin: NONE -> finding 8.
### Part 3 OVERTAKEN
O1 row-1 controls vs "wire dead UI up": P -> finding 1 (Open Image, the window drop, the camera list, and what the output shows when nothing plays have no home). O2 PresetManager/auto-preset consumer: T10, architects (C). O3 "instant preset save/recall" in the app's own description: R211 last sentence (C). O4 MilkDrop beats vs bars: R212, R196 (C). O5 Composition-tab dead blocks: Q212 (f) (C).
### Part 5
172 (show start, relates H20/H8): C. 173 (BPM-mode fire mid-bar): C, R213 (a) extends it to autopilot. 174 (paused fire of a non-BPM clip): P, covers videos only -> finding 3. 175: C. 177/178: C.
### CORRECTIONS
C1 (drop acts on MilkDrop Jukebox + playlist OnDrop): R202 (d) says it (C; exactness finding 7). C2 (oscillator switch ruled): R195 (b) (C). C3 (playlist triggers have no UI; Jukebox labels are bars x 4): R212 for the Jukebox; the Playlist list not (finding 6). C4 (Text Animator = random digits): R201 (d) (C). C5/C6: citation/wording only, not his.
M1 drop and MilkDrop: R202 (d) (C). M2 blend in seconds: R202 (c), R196 (C). M3 audio file ends: R204 (c) + Q215 (d) (C). M4 wired input pulled: R204 (b) (C; one word NIT 5). M5 what travels with the show (MilkDrop favourites, playlists): favourites in R188 (d) (C); playlists NONE -> finding 9b/SHOULD "R214". M6 import/camera boxes: Q209 (C). M7 end of row / random repeat / lone clip: R203 (a) (C; "lone clip does nothing" not said, NIT 4). M8 audio file plays out loud: R204 (c) (C).
### Part 1 items compared with the list (today claims)
Contradictions: (1) R211 slots "at the top" (they are at the bottom): finding 2. (2) R201 (c): finding 4. (3) Q211 "drop a folder": finding 5. Others checked and true: Q212 (b), (c), (d) wording, (e), (f), (g); Q215 (c), (d) (no looping: git grep finds no looping code in src/audio or MainComponent.cpp, VERIFIED); R203 (a) wrap and "may return" (Autopilot.cpp:243-282 per sheet M7, INFERRED not re-read); R204 (a)(c); R212 counts (PresetSelector.cpp:169 `transitionBars_ * 4`, barsSinceLastSwitch_ counts bars at :82, VERIFIED); R213 (a) "resumes" (the picture's own player resumes, Layer.h:553 resets only the model playhead, MainComponent.cpp:5063-5075; VERIFIED, no flag).

## B. FINDINGS (exact new wording; new question numbers continue at 216, new readings at R214; none of them is a number he has seen)

### MUST
1. Q211 A / Q212 A / R150 / R211 / NEW 216 -- THE OLD ROW AND "WHAT THE OUTPUT SHOWS WHEN NOTHING PLAYS" HAVE NO HOME (O1, H1, H13).
VERIFIED: the top row holds "Open Image", "Open Folder", the "Beats per Image" list (2..128), the "Camera" list ("Cam: Off") and the three preset buttons (MainComponent.cpp:2749-2761, 350-369, 450-467). Camera code is compiled in on macOS (CMakeLists.txt:467, AUDIODNA_HAS_CAMERA=1). All of it feeds the old loose picture (Renderer fallback, FACP:33; INFERRED, not run). A picture dropped anywhere on the window outside a cell loads into the same loose picture (MainComponent.cpp:4217-4245 filesDropped -> previewPanel_.loadImage; VERIFIED), and a click on a MilkDrop preset in the MilkDrop tab sets it too (MainComponent.cpp:1724-1737; VERIFIED).
The list: Q211 A says "The old row at the top goes" (it names only Open Folder and the Beats list); Q212 A says the camera stays as it is; R211 removes the three buttons and ten slots; R150 removes the name click. Nothing says what happens to Open Image, to the camera list, to the picture dropped on the window, or what the output shows when no clip plays (e.g. after Stop, BD:1007). A builder following Q211 A deletes his only working camera path; one following Q212 A keeps a row that Q211 A deleted.
FIX, NEW question 216 [K], and Q211 A re-worded to name the same things:
"216 -- what the output shows when no clip plays. Situation: You press stop, or you have just opened the app: no clip plays on any layer. Today the output can still show a loose picture: the last one you chose with Open Image or Open Folder at the top, a picture dropped on the window outside the clip grid, a MilkDrop preset you clicked in the MilkDrop tab, or the camera chosen in the top row's Camera list (read in the code, not run). A [mine]: Black. Open Image, Open Folder, the Beats list and the loose picture go with the old row (211 A). The top row's Camera list stays as it is until you answer 212 (a), and shows only while nothing plays. B: The loose picture stays as it is today, and so does the old row (then 211 C). Why it matters: 211 A, 211 R and the cue system each take a piece of the old row away, and nothing says what an empty stage shows or what becomes of the camera."
And Q211 A: replace "The old row at the top goes." by "The old row's Open Folder button and its Beats list go (216 says what happens to the rest of that row)."

2. R211 -- WRONG ABOUT TODAY (contradiction with part 1 item 20).
VERIFIED: the ten numbered slots are at the BOTTOM of the window (MainComponent.cpp:2780-2797, removeFromBottom(28); buttons "1".."10", each with a "--" drop-down, :373-382); only "Save", "Load", "FX Save" are in the top row (:2763-2767). R211 says "The ten numbered preset slots at the top of the window, with their small buttons Save, Load and FX Save".
FIX, R211 first sentence: "The ten numbered preset slots along the bottom of the window (each a number button with a drop-down), and the three small buttons Save, Load and FX Save in the top row, go, as you ruled (86)."

3. R193 (b) / Q174 -- ONE OF HIS TWO STATEMENTS IS CHOSEN SILENTLY (H19, H20; rule f).
His words: "When paused, it will not play but will still display." (BD:1091, "a clip", no type) against "136 b" (BD:1009: pause holds only BPM-mode clips; "a clip that is not BPM-synced plays on"). Q174 asks this only for a video or sequence not in BPM mode. R193 (b) then rules, as "Mine" and "whatever you answer to 174", that a still, a generated picture, MilkDrop, the camera and an effects-only clip "show and move" when fired during a pause. That is the opposite of his literal sentence for exactly the pictures that have no playhead, and R193 is not in the intro's list of the fourteen readings to look at first. A fractal or Plasma moving while a video stands still is a stage-visible difference.
FIX: (i) Q174 situation: "The beat is paused. You fire a clip that is NOT in BPM mode (a video or a picture sequence), or a generated picture (Plasma, a fractal), MilkDrop, or a still picture." Q174 A: "It shows and stands still: a video on its first frame, a generated picture or MilkDrop as it was at your press, until the beat plays. This is your sentence as written. Something already playing plays on." Q174 B: "It shows and moves at once. Pause holds only the clips in BPM mode (your 136 b)." (ii) R193 (b): delete "and fired during a pause they show and move, whatever you answer to 174" and write "fired during a pause they follow your answer to 174". (iii) add R193 to the intro's "look at those first" list. Cost of freezing a generated picture is UNKNOWN (architects; the pictures that read time).

### SHOULD
4. R201 (c) -- OVERSTATES THE LIMIT.
VERIFIED: every draw passes the clip's own slider values (CompositorEngine.cpp:1442-1445 sourceRenderFn_(type, time, w, h, params)); only the generator and its output texture are one per kind (Renderer.cpp:1160-1171; CompositorEngine.cpp:1497-1520 comment). So two Plasma clips on two layers each show their own sliders. What is truly shared: the running state of a simulation (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics; INFERRED from Pitfall 22) and, INFERRED, the one MilkDrop picture.
FIX: "(c) Each kind of generated picture has one generator that all clips of that kind use in turn. Two Plasma clips each keep their own sliders, but a simulation (Reaction-Diffusion, Cellular Automata, Strange Attractor, Gravity Well, Fluid Dynamics) has one running state that its clips share, and every MilkDrop clip shows the same preset. That is today's limit, and it is why a preview of a source that is also playing shows the playing one (said under the cue system)." Also R150: "a click on its thumbnail fires it" -> add "(a generated picture has no thumbnail today, R201 b: its cell is clicked)".

5. Q211 -- THE SITUATION IS NOT TODAY'S; ONE OPTION IS NEW; ONE OPTION MISSING (H12, H13).
VERIFIED: a dropped FOLDER is not accepted: ClipCell::isInterestedInFileDrag accepts only files with a picture or video extension (ClipCell.cpp:311-323), MainComponent's too (:4204-4215); no handler expands a folder. The folder way today is only the old "Open Folder" button. Three or more pictures dropped together become one sequence (MainComponent.cpp:836-880; VERIFIED), exactly two become two clips (also today's rule, a "ruling of 2026-08-04" known only from code comments, not in BD).
Today's BPM mode spreads ALL pictures over a length (Clip.h:30-41); a time per picture ("a beat, a bar, 2 bars") is NEW, yet A begins "One clip, as today". The Q has no "one clip per picture" option (his Resolume habit is unwritten).
FIX: situation: "You select twenty pictures in Finder and drop them together onto a cell. Today three or more pictures become one clip that runs through them (exactly two become two clips; a folder cannot be dropped). The old Open Folder button at the top shows one picture every 2 to 128 beats, but only while no layer plays." A [mine]: "One clip, as today. New: in BPM mode you also set how long each picture stays (a beat, a bar, 2 bars). The old Open Folder button and its Beats list go." B: "One clip that spreads all its pictures over the clip's length (4, 8, 12 bars), like a video; no time per picture." C (new): "One clip per picture, side by side in the next cells, like videos; dropping a folder also becomes possible." Drop today's C ("Both stay") into 216 B. Why line unchanged plus: "It also decides whether two pictures stay two clips and three become one."

6. R212 / R196 -- THE CLIP'S OWN MILKDROP PLAYLIST LIST IS NOT NAMED (C3).
VERIFIED: the per-clip Playlist list reads "4 beats", "8 beats", "16 beats", "32 beats" (MilkDropBrowser.cpp:611-615) and counts real beats (Renderer.cpp:566-575, trigger Beats). R196 says "MilkDrop's list read in bars" and R212 treats only the Jukebox list, so a builder cannot tell whether the Playlist list changes. 
FIX, add to R212: "The list under a clip's own MilkDrop playlist reads 4, 8, 16 and 32 beats and does count beats; it will read 1, 2, 4 and 8 bars. The playlist's triggers on the music's structure (on the drop, on a breakdown, per phrase) exist only in the saved file and have no button: they stay without one unless you ask."

7. R202 (d) -- INEXACT ABOUT TODAY (C1).
VERIFIED PresetSelector.cpp:78-100, :76: any change of the structure state (not only drop and breakdown: also build-up and the return to normal) schedules a switch, only if at least 4 bars passed since the last switch, done on the next bar; with energy matching (default true, PresetSelector.h:64) the new preset is chosen to suit (intense on a drop, calm on a breakdown; INFERRED from :115-150).
FIX R202 (d): "One thing you may not know: while the Jukebox runs, a change in the music's structure (a build-up, a drop, a breakdown, back to normal) already changes the preset on the next bar, never sooner than four bars after the last change, and it picks an intense one on a drop and a calm one on a breakdown. That stays; say so if you want a switch for it." Also Q212 (d): "only MilkDrop's Jukebox reacts" -> "on screen only MilkDrop's Jukebox reacts (and the oscillator switch you ruled)".

8. R213 -- THREE GAPS (H8, T9).
(i) A count shorter than a bar: a BPM-mode clip the autopilot fires waits for the "1" (R213 a), so with a count of 1/4 or 1/2 bar the change still comes on the next "1". (ii) The "End of Video" trigger is not mentioned under pause: a non-BPM video that plays on through a pause (136 b) can reach its end; R213 (b) says "it does not move on" only for the count. (iii) Where the count starts for a BPM-mode clip that waited for the "1" (from the press or from the "1"): Autopilot.cpp:100-110 counts beats played since activation (VERIFIED); T9 is technical but he sees it as a clip staying a bar longer or shorter.
FIX, add to R213: "(d) Mine: a BPM-mode clip the autopilot brings in starts on the next "1", and its count starts there, so a count shorter than a bar still changes on a "1". (e) Mine: while the beat is paused or stopped the autopilot does nothing at all, 'End of Video' included; a video that ends then simply stays on its last frame until the beat plays." (If he prefers otherwise it is one word from him.)

9. DECIDED LINE MISSING (T4) and NEW R214 (M5).
(a) T4: the autopilot may never move on from a generated picture that was paused or stopped and fired again (Layer.h:553-556 `hasBeenTriggered` is never reset; Autopilot.cpp:100 needs `playing`; INFERRED for the chain, VERIFIED lines). Add under "decided, not asked": "The autopilot moves on from a generated picture, a still or MilkDrop the same as from any clip, also after you paused it, stopped everything, or fired it a second time. By: the architects (today it can stay stuck: Autopilot.cpp:100, Layer.h:553)."
(b) R214 (new reading, K): "A MilkDrop clip's list of presets is saved in the show as the places of the preset files on this computer (VERIFIED: Clip.h:201 "Full path to .milk file"; Clip.cpp:145-154). Collect Media copies pictures, videos and sequence pictures, but not these preset files (MainComponent.cpp:6709-6745, VERIFIED). Opened on another computer without those files, the clip stays but the preset does not load (INFERRED). MilkDrop's favourites stay on the computer (R188 d). Say so if the preset files of your clips should travel with the show."
   (Note, not his: Collect Media copies by file name and skips a name that exists, then re-points to it: two different pictures with the same name from two folders collapse into one; VERIFIED :6726-6744, architects.)

10. NEW design item (H11 c): "A generated picture's cell." Variants: (a) as today: coloured tile with the tag "SRC" and the name; (b) a still picture of the source taken once when the clip is made; (c) a small live picture (costly: every source runs once more per frame). Reason: R201 (b) leaves it as today, but Resolume's cells show their picture, and the cue system now makes the cell the thing you click to see it.

11. R204 (d) -- THE HIDDEN READOUTS ARE NOT NAMED (H16).
VERIFIED: the seven band meters, beat phase, dB, onset flash, genre and the spectrum are drawn by two panels that are hidden for good (MainComponent.cpp:2801-2802; APP-INVENTORY.md section 8). FIX R204 (d): "What you see of the sound stays: the waveform, the signal meters and the tempo. The band meters, the dB level, the onset flash, the genre line and the spectrum are not on screen today (their panel is switched off) and stay off; where they come back, if at all, is for the pictures."

### NIT (in the paper only)
N1. Q212 lead-in "do nothing you can see today" vs (d) "only MilkDrop's Jukebox reacts": say "do nothing, or very little, you can see".
N2. Q212 (a): add "(the camera you can choose in the top row's Camera list does show, but only when no clip plays)" -- ties to finding 1.
N3. R197 (e) "the layer's Feedback list of six looks": "look" is the word he replaced by "preset" (BD:1014); the on-screen tooltip is "Feedback preset" (LayerInspector.cpp:408-421). Write "six named settings (Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch)". R206's heading "seven looks in your Arena" has the same word in another sense.
N4. R203 (b) "a clip's own list of what comes next": it is one choice (nothing, next, previous, random, first, last or a named clip) with its own count (Clip.h:148-165). R203 (a): add "on a layer set to End of Video, a picture or a generated source, which has no end, changes after the layer's count instead (Autopilot.cpp:103-107, VERIFIED); a lone clip in a row stays".
N5. R204 (b) "the built-in microphone": the app takes the Mac's default if allowed, else the built-in (AudioEngine.cpp:23-26 comment; DeviceGuard); write "the Mac's own choice, usually the built-in microphone". Whether the app goes back to the wired input when it is plugged in again: UNKNOWN (cheapest: read AudioDeviceReconciler re-apply path).
N6. Q211 "only while no layer plays" and Q212 (a) "draws nothing" are INFERRED (FACP:33 "Not run"; Sources row, SourceRegistry has no "camera": createSource returns nullptr, SourceRegistry.cpp:1247-1253); say "as far as I read the code".

## C. THINGS CHECKED AND LEFT AS THEY ARE
- No text in my area uses "bindings", "overlay", "key / pad list" for his mapping list (scripted grep of all questions, readings, decided lines: only R148 quotes them as today's strings, correct and intended). R148 carries the on-screen titles with file:line (MenuBarModel.cpp:10, 155-161 etc.); not re-verified, not my area.
- Boris quotations in my area items (Q209, Q212 his_words empty on purpose; Q174, Q173, Q172 his_words) are the ones the sheet re-opened at BD lines; I did not re-open BD.
- decided 1, 6, 7, 8 agree with the sheet (autopilot never starts a stopped beat; preview never on an output; cost measured first; same-kind preview limit).
- Q212 A "later, all seven" is honest about BD:99 ("wire it up") in its why line; it leaves the seven parked by his letter, which is an answer. Genre / smart / structure / Composition-block / randomizer / ISF / camera each become a question set only after the first builds.
- Q215 (a)-(f) and R204 (c): audio file plays once and is heard out loud: VERIFIED no looping code; AudioEngine.cpp:68-76 per sheet (INFERRED, not re-read).
