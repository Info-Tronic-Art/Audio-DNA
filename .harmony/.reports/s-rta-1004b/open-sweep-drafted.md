# open-sweep-drafted -- sweep "drafted" (written 2026-10-04, session s-rta-1004b close)
Read-only sweep. Labels: VERIFIED = I read it at the file:line named; INFERRED = my reading, said so.
Paths: BD = /Users/boriskarpman/projects/RealTimeAudio/.harmony/binding-decisions.md; BL = .../.harmony/boris-feedback-backlog.md;
C135 = .../.harmony/.reports/s-rta-1004b/boris-clarify-135-143.md; C144 = .../s-rta-1004b/boris-clarify-144-147.md;
RN2 = .../s-rta-1004b/ruling-nudge-row2.md; RL2 = .../s-rta-1004b/ruling-looks-answers2.md; other files by name under .harmony/.reports/s-rta-1004[b]/.
No Boris message of the session carries an answer to any item in PART 1 (checked: BD and BL hold no "150" or "151" as an answer; boris-msg-raw-5.txt is the whole last message: "144 a / 145 b / 146 b / 147 a" + the close instruction).

SUMMARY
- Numbered questions with NO answer: 73 (held back), 150, 151 (neither shown to him yet). Four more stand at their default without his naming them: 30, 34, 37, 38.
- Readings not yet told to him: nudge-row2 R103..R114 (twelve; to be shown as R109..R120; R112 = R118 is VOID by "145 b"), looks-answers2 section 6 (20 checks) and section 7's "readings these rules rest on" (from R121).
- Rules resting on a reading he did not correct: 13 items in PART 3 (heaviest: R87/R100 "the clip's Snap goes", R94 "the pads go", R88 "a preset unplugs signals"; R16 and R79 already resolved by his later answers).
- Requests still open: RQ-0 (three tracks; he said another session, from a DJ), RQ-3 (partly met). PART 5 adds 8 other open things (a dropped draft question, the actions + review-screen design, R75/R76 talk, pending checks).
- Items listed: 63 (3 questions + 4 default-stood + 12 readings + 1 readings paragraph + 20 looks-answers2 checks + 13 rests-on + 2 requests + 8 other); INFERRED count.
- Verdict on every other number from 26 to 147: answered or withdrawn (table in PART 1d).

=====================================================================================================
PART 1  NUMBERED QUESTIONS FOR BORIS WITH NO ANSWER
=====================================================================================================

--- 1a. Q73 -- HELD BACK until a measurement called FM-8 --------------------------------------------
shown to him: NO (never put to him; "HELD BACK (asked only if FM-8 finds a BPM-synced clip in a saved show)").
Source of the text as drafted: .harmony/.reports/s-rta-1004/boris-clarify-71-74.md:24-29 (same words in ruling-transport-delta2.md:751-756). VERIFIED. Verbatim:
## HELD BACK (asked only if FM-8 finds a BPM-synced clip in a saved show)
73. Your saved shows that have BPM-synced clips:
    A (default) When you open one, each such clip gets its Bars number and its end moves in to whole groups of 4 bars.
      The saved file changes only when you save.
    B They open as plain speed clips (a video looks as it did; an image sequence keeps the speed it had at 120 BPM);
      you switch the ones you want to BPM Sync yourself.
Why it is held, in one plain line: it only matters if one of his saved shows holds a clip set to BPM Sync, and FM-8 (a read-only search of his show files for `"transportMode": 1`) has not been run; ruling-transport-delta2.md:522-523 says "None: question 73 is not asked and default A is built." (VERIFIED, ruling-transport-delta2.md:522-523; "73 still held back (FM-8)": boris-clarify-121-124.md:24; s-rta-1004-work.md row 15:35:01 "Every question 26-111 is now answered or withdrawn, except 73".)
My own cheap check (NOT FM-8 itself; the copy of his one show in .harmony/.reports/s-rta-1004/boris-show-backup/test with harry.json, same sha256 as his file at 20:51 per s-rta-1004b-work.md row 20:51:53): 15 clips carry "transportMode": 0 and none carries 1 -> INFERRED: FM-8 will find none, Q73 will not be asked, default A stands. Caveat: `1` = BPM Sync is the ruling's own wording, I did not re-derive the enum; and he may keep other show files elsewhere (compositions/ holds exactly one file per the baseline in s-rta-1004b-work.md row 20:51:53).

--- 1b. Q150 and Q151 (ruling-looks-answers2.md section 7; the ruling is returned, NOT YET ADOPTED) -----
shown to him: NO ("Neither question was shown to him before; these are their words", RL2:863). Source RL2:861-882 (VERIFIED). Copied character for character, RL2 lines 865-872:
150. You add an effect: its small button reads "Default". You move one of its sliders. You have never given this effect a
     preset.
     A (default) It keeps reading "Default" -- the same way "Preset 2" stays after you change it.
     B It reads "Presets", dim, as soon as it is no longer at its defaults, and "Default" again when you bring the sliders
       back.
151. MilkDrop's list also calls its items "presets" ("Search presets...", "12 presets"). Now effects have presets too.
     A (default) Leave MilkDrop's words as they are: they are inside the tab named MilkDrop.
     B Make them say "MilkDrop presets" in full, everywhere in that tab.
What each answer changes (RL2:873-882, verbatim, so the page can say it in his terms):
What changes with each answer (so that either is a small change)
- 150 B: `buttonState` loses its case (3) -- one line. LM-3 takes its dormant name (5.2); V-3 shows "Presets", dim; check 1
  says so; GL-12 takes its dormant last clause (5.5). GL-10 and GL-11 do not change (GL-11 (d)'s effect is AT its
  defaults). No stored name, no format, no route changes.
- 151 B: SIX strings, not the plan's five (SF-9): src/ui/MilkDropBrowser.cpp:20 ("No presets loaded."), :101, :122, :283,
  :456 and src/ui/MilkDropBrowser.h:120. V-24 is captured again. They are not this lane's files today: one row added to
  S3, or a small packet of its own.
NOT ASKED, and why. The plan's 152 (what the other effects read after a rename): ruled in RB-3 from his 133 and told to him
in check 19; HD-26 holds the way back to a question. Every stored name, the four cases of the button, the name in the show,
the tick, the routes, the fence: technical, ruled here.
Note on Q150's default: RL2 deliberately gives it the OPPOSITE default to the plan (HD-29, RL2:913; "Question 150 goes to him with the default this ruling gives it, the opposite of the plan's"). The plan's own draft is plan-looks-answers2.md:672 ("new numbers 150..152"); Q152 is WITHDRAWN (ruled in RB-3 from his 133, told to him in check 19) and 153 is not used (RL2:861). VERIFIED.
Also: "Nothing here is shown to him before the visual gate has passed" (RL2:779) applies to section 6, not to 150/151.

--- 1c. Questions that stand at their DEFAULT because he never named them (told to him as defaults; INFERRED consent) ---
shown to him: YES (each was asked; he did not name it). Not "open" strictly, but never confirmed in words. Texts as asked: boris-clarify-26-37.md.
- Q30 (boris-clarify-26-37.md:26-29) BPM box on the clip: A (default) as Resolume: no BPM box. Stands: BD:828; BL:473; told again as R20, boris-clarify-42-44.md:33.
- Q34 (boris-clarify-26-37.md:45-47) a failed Save: A (default) the "Save failed" box still shows. Stands: BD:828; BL:473; R20.
- Q37 (boris-clarify-26-37.md:58-61) the "not enough disk space to record" sign: A (default) a line in the Record tab. Stands: BD:828; BL:473; R20.
- Q38 (boris-clarify-38-41.md:8-10) where a screen's Delay is remembered: A (default) with the screen, whatever show is open. Stands: BD:839; BL:491; told as R23 (boris-clarify-45-46.md:19).
Check twice: his 21:03:09 line "All defaults good except for these:" sits above the pasted list of 125-134 (BL:716-718), so I read it as covering that list, not 30/34/37/38 (INFERRED). BL:175 BF30 "All the other defaults are good" is about the Oct 2 page (older questions).

--- 1d. Every other number checked twice (asked -> answered, with the line that says so) ---------------
VERIFIED at the ANSWERS blocks / BL headings. 26 27 28 31 33 35 36 answered 2026-10-04 12:21:01 (boris-clarify-26-37.md:82); 29 -> re-asked 42 = default (boris-clarify-42-44.md:42); 32 -> re-asked 43 -> 47 = default A (boris-clarify-47-49.md:34); 39 40 41 answered (boris-clarify-38-41.md:39); 44 -> 48 = B; 45 answered, 46 = A (boris-clarify-45-46.md:27); 47 = A, 48 = B, 49 = A; 50 changed to B + computer (boris-clarify-50.md:13-14); 51 = A "51 default is good" (boris-clarify-51.md:25); 61 = B, 62 = his row, 63 = A (boris-clarify-61-63.md:27); 71 = B, 72 = his words, 74 = B (boris-clarify-71-74.md:40); 80 = A, 81 = both, 82 = A (boris-clarify-80-82.md:32-33); 83 answered 12:56:43 (boris-clarify-83.md:15); 84 and 85 answered (boris-clarify-84.md:19, -85.md:13); 86 = A (boris-clarify-86.md:21); 91 = A, 93 = B, 94 = A (boris-clarify-91-94.md:35); 101-104, 106 answered (boris-clarify-101-106.md:36); 111 = A (boris-clarify-111.md:22); 121 = A, 122 = A, 123 = B, 124 = A (boris-clarify-121-124.md:30); 125 = A, 126 = A, 127 and 128 his own words (-> 135/136), 129 = B (boris-clarify-125-129.md:60-67); 131 = A (his two cases, R88), 132 = A, 133 = no letter (name stays, no mark), 134 = B done (boris-clarify-131-134.md:40-45); 135 = A, 136 = B, 137 = reading R96, 138 = no letter (read as A) + rename look -> preset, 139 = A, 140 = A + "catches up", 141 = A, 142 = A + "a layers actions can trigger clips", 143 = A (C135:69-78); 144 = A, 145 = B, 146 = B, 147 = A (C144:22).
Numbers never used: 52-59 (ruling-outputs.md:816 "52-59 are not used"), 75-79, 92, 95-99, 105 (withdrawn), 107-110, 112-120, 130, 148, 149, 152 (withdrawn), 153. Next free question: 154 (C144:2). INFERRED where I did not open the file (112-120, 130, 107-110): the board and C144:2 say only "next free: 154".
Older items: "PRESET MIGRATION -- STILL OPEN" (BD:260, BD:312) is CLOSED as moot by questions-for-boris-s168.md:136 ("there is nothing to migrate") -- VERIFIED. The honesty batch / the rack (BD:168-173) is overruled by BD:99-100 ("DO NOT HIDE ... WIRE IT UP") -- VERIFIED headings only.

=====================================================================================================
PART 2  READINGS NOT YET TOLD TO HIM  (shown to him: NO for every line below)
=====================================================================================================
Numbering rule (VERIFIED, C144:2 and the adoption block at plan-nudge-row2.md:821): Harmony's own R103-R108 were already told to him under those numbers (22:27:57, 22:32:45; C135:101-114). The ruling's twelve readings carry the same numbers R103..R114 inside RN2, so they are told to him as R109..R120 in the same order, and are cited as "nudge-row2 R103" etc. Next free reading after that: R121. Mapping (INFERRED arithmetic, +6 each):
 nudge-row2 R103 -> R109   R104 -> R110   R105 -> R111   R106 -> R112   R107 -> R113   R108 -> R114
 nudge-row2 R109 -> R115   R110 -> R116   R111 -> R117   R112 -> R118 (VOID)   R113 -> R119   R114 -> R120
The ruling's R112 (-> R118) is VOID: he answered "145 b" (BD 22:40:14; BL:877+; C144:22); the ruling's own text for 145 B is "the text 'nudge +12 ms' sits BETWEEN the two nudge buttons", which the ruling's R112 contradicts ("right after RESYNC"). plan-nudge-row2.md:821 says "R112 -> R118 is void by 145 B". VERIFIED. The other eleven are unaffected by it (INFERRED).
Related effect of "146 b" (Bar text out): the ruling's reading R79 (tempo-row, "Bar 1..4 text left of the circle") is superseded by his answer (INFERRED from BF123, BL:894 region; the ruling's R79 line: RN2:860).

--- 2a. The ruling's twelve readings, VERBATIM from RN2:833-852 (the ruling's own numbers R103..R114; each is told as +6) ---
Header of the block, RN2:831-832: "READINGS told with them (he corrects only what is wrong; these REPLACE PL2's R103-R110; next free reading R115, next free question 148):"
 R103 Stop also stops the routines that are playing or waiting on the layers, as a layer's X does.
 R104 A recording you replay is not stopped, paused or started by the row: it follows its own sound and can put clips
      back; a Resync inside it does not start a stopped or paused beat.
 R105 A BPM Sync clip you fire while the beat is paused or stopped appears on its first frame and starts moving when
      the beat runs (from stop: on your play press, the "1"). A clip that is not in BPM Sync plays at once.
 R106 Stop is a cut: the clips go at once, like a layer's X. No fade.
 R107 Stop is not an Undo step. Until the Undo rule for the layer strip is built, Cmd+Z after a stop still undoes your
      last fire, as it does today, and that can put a clip back.
 R108 The row's pause and stop never change a clip's own pause: a clip you paused yourself stays paused, a clip that
      was playing plays when you fire it again, and nothing about the row's pause or stop is saved in the show.
 R109 Resync while the beat is stopped or paused: your press is the "1", the beat runs from it, the nudge reads 0.
      Tapping a tempo while it is stopped changes the number and starts nothing. (replaces R84)
 R110 An old show's Quantize setting is dropped the next time it is saved. An old recording made with Quantize on
      replays each press at the moment you pressed, not on the line it landed on then.
 R111 Routine pads keep their own start setting until actions replace them: by default a pad waits for the next bar. A
      pad you press while the beat is stopped waits, and starts one bar after you press play.
 R112 The text "nudge +12 ms" sits right after RESYNC, so your thirteen cells stay side by side as in your picture.
 R113 The cells are as tall as the top bar is today -- a little smaller than Resolume's.
 R114 In the key and pad list the old "Stop" reads "Stop routines" and the old "Play / Pause" reads "Audio play /
      pause", so neither can be taken for the row's "Beat stop" or "Beat play".
Also in the same block, RN2:853 (VERBATIM): "NOT told, because he can neither see nor lose it: the clips' Snap values stay in the show file, unread (NC-7)."
Quotable caution: RN2:831 says these REPLACE "PL2's R103-R110" (plan-nudge-row2's own draft readings): the plan's are NOT to be shown. VERIFIED.

--- 2b. ruling-looks-answers2 section 7: its readings paragraph, VERBATIM RL2:883-886 (to be shown from R121 on) ---
Readings these rules rest on. R88 and R95 stand by INFERRED consent (told to him, not corrected): if he corrects R88, one
constant and GL-9's dormant line (section 0 (4)); if he corrects R95, 138 B becomes a stage of its own and nothing here is
undone. Two rules rest on MY reading of his 133, which was never told to him as a reading: question 150's default (he is
asked outright) and RB-3 (he is told in check 19; if he corrects it, the walk is limited to the renaming row).

(That paragraph names no R-number of its own; the two readings it rests on are already told: R88 and R95, C135:46-49 and C135:80-82. What is NEW in it and never told to him as a reading: "RB-3 rests on MY reading of his 133" -- the ruling's rule for what the other effects read after a preset is renamed -- told to him only through section 6 check 19 below.)

--- 2c. ruling-looks-answers2 section 6 "WHAT ONLY BORIS CAN CHECK", VERBATIM RL2:777-858 (20 numbered checks; per RL2:779 "Nothing here is shown to him before the visual gate has passed") ---
Harmony asked for these "to be shown from R121 on": INFERRED numbering = one reading per check, check 1 = R121 ... check 20 = R140; the numbering is Harmony's call, not in the file. The checks describe a build that does not exist yet. Note the ruling's own header: "this list REPLACES PL2 section 6 and RA section 6".
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; this list REPLACES PL2 section 6 and RA section 6)
---------------------------------------------------------------------------------------------------------
On his own screens, in his own show. Nothing here is shown to him before the visual gate has passed.
1. Put an effect on a clip: its small button reads "Default". Move a slider: it still reads "Default" (question 150). Press
   it and choose New Preset -> a small box opens holding "Preset 1", already selected, and you can type at once. Press
   Return -> the button reads "Preset 1". Or type a name first, then Return -> the button reads that name. Esc -> no preset
   -> wrong: you must click the box before you can type; typing adds to "Preset 1" instead of replacing it; a preset
   appears after Esc; the word "look" anywhere.
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Preset 1" is there;
   choosing it sets the sliders as you left them -> wrong: it is missing, or it is there only in the first show.
3. While the box is open, type letters that are your clip keys -> they go into the box and no clip fires. After Return or Esc
   your keys launch clips again at once -> wrong: a clip fires while you type, or the keys are dead after the box closes.
4. With a clip playing on that layer, load a preset -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move. For <the number Harmony measured> frame(s) the screens hold the picture
   they had, as they do when you add an effect; a video recording and Syphon get no picture for those frames -- also as
   today when you add an effect. The same with a preset that plugs signals in -> wrong: the layer restarts, the picture goes
   black, anything in the strip changes, or a hitch you can see in a recording.
5. Press Cmd+Z -> the effect returns to how it was just before the preset, sliders AND signals, and the button reads what it
   read before; a slider you moved on ANOTHER effect meanwhile stays where you put it. (A slider you moved or a signal you
   plugged on the SAME effect after the preset goes back too.) -> wrong: the other effect's slider jumps back; the effect
   folds shut; the sliders come back but the signals do not.
6. THE NAME STAYS. Load "Preset 2", then move its sliders as far as you like -> the button still reads "Preset 2", in the
   same colour, with no mark. Open the menu: "Preset 2" is still ticked; click it -> the effect is back on Preset 2 -- its
   sliders, and its signals too: a signal you plugged in since, which Preset 2 does not hold, is unplugged. Cmd+Z takes all
   of that back. Save the show, quit, open it again -> the button still reads "Preset 2" -> wrong: the name dims, turns into
   "Presets", gets a star or a dot; the name is gone after you re-open the show; the name changes by itself while you drag
   a slider.
7. Signals. (a) Plug the bass into a slider, set its range, make a preset. Put the same effect on another clip and load the
   preset -> the slider there follows the bass with the same range, from the first frame. (b) Load that preset again on the
   first effect -> nothing jumps, and a slider you are holding stays in your hand. (c) Load "Default", or a preset you made
   with no signal on a slider, or click the preset the effect already carries -> the signals that preset does not hold are
   unplugged. Cmd+Z brings sliders and signals back in one step. That is the only way back, and only until you close the
   show: wiring you want to keep, keep as a preset first. (d) To change what signals a preset holds: load it on an effect
   in your show, change the signals there, then `Save over "<its name>"`. There is no other place to edit a preset.
   (e) A preset remembers "Macro 3", "Mod 1" or "Mod 2" by name, not what they are doing in this show. (f) A slider driven
   by "Clip Position" holds still today, with or without a preset: that is older than this build and is on the list ->
   wrong: a slider pinned at one end after a load; a signal left on after Default; an undo that brings back sliders but
   not signals.
8. Rename opens the same box, holding the preset's name, selected. Delete: a list in red; pick one; a window asks. You never
   have to load a preset to rename or delete it -> wrong: a preset you cannot rename or delete; a name that changes by
   itself.
9. New Preset is grey when the effect already is exactly one of your presets, or Default: there is nothing new to keep.
   `Save over "Preset 2"` is live only after you loaded or made Preset 2 on that effect and then changed something.
10. Load "Preset 2", change a slider, open the menu. New Preset -> the box says "Preset 3": Return keeps the change as
   Preset 3 and Preset 2 is as it was. Or `Save over "Preset 2"` -> a window asks -> click Save Over: Preset 2 now holds the
   change wherever you load it from now on. The button does not change when you save over; to see that it saved, open the
   menu again: `Save over "Preset 2"` is grey. An effect that already had the old Preset 2 keeps its settings and still
   reads "Preset 2"; clicking Preset 2 in its menu gives it the new settings. There is no undo for a Save over or a New
   Preset: Cmd+Z after either takes back the last time you LOADED a preset on that effect, not the save -- what you saved
   stays in the preset -> wrong: Preset 2 changed after New Preset; Save over names another preset; Preset 2 is missing or
   half-changed after a Save over.
11. In the Save Over window press Return -> the window closes and NOTHING was saved; only a click on "Save Over" saves. In
   the name box, type a name you already have -> the name turns red and the button goes grey; press Return -> nothing happens
   and the box stays -> wrong: Return saves over a preset; a taken name closes the box.
12. After the menu, the box or one of the windows closes, your keys work as before (Return, your clip keys) -> wrong: Return
   opens the menu again, or presses a button.
13. Record a take, load a preset in the middle, play the take back -> this build does NOT play the preset change back, and a
   take never stores which signals are plugged in. The same is true today of an effect slider you move with the mouse.
   Recording these is its own build, and it is on the list.
14. Where your presets live: on this computer, in the folder Library / Audio-DNA / Effect Presets inside your home folder,
   one folder per effect, one small file per preset. A show you open on another computer still looks the same -- each effect
   keeps its settings and its signals in the show -- but the menus there list only that computer's presets, and a button
   whose preset is not on that computer reads "Presets", dim. Copy that folder across and the names are back. To back your
   presets up, copy that folder.
15. If the disk is full or that folder is locked, New Preset and Save over do nothing and the old preset is untouched. No
   message. After New Preset the button does not take the new name. After Save over the button reads the same either way:
   open the menu -- `Save over "..."` is still live when it did not save.
16. By eye on your own screens: the button's size; the dim "Presets" (you will see it only for a preset this computer does
   not have); the menu's length with many presets; the red Delete list; the name box and the red name when a name is taken;
   the Save Over window.
17. The old buttons (they go with the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone.
   Your two old presets and your nine quick FX saves were moved to the Trash at your word. Still on your disk, and no longer
   opened by the app: one deck file ("test 1.deck.json") in Library / AudioDNA / Presets.
18. Two kinds of preset on one screen: open the MilkDrop tab on the left and an effect's Presets menu on the right. The
   MilkDrop list says "presets" too ("Search presets...") -> tell us if you ever read one as the other (question 151).
19. Rename a preset that several effects carry -> every effect in the open show that carried it reads the new name, whether
   you had changed it or not. Delete it -> those effects read "Default" and keep their settings. One thing to know: a show
   that was NOT open at that moment still holds the old name. Its effects read "Presets", dim, until you load a preset on
   them -- and if you ever make a new preset under that old name, they will read it as theirs -> wrong: after a rename an
   effect of the open show reads "Presets".
20. Not changed by this build: the Record tab still says "holding the last look". There "look" means the picture your take
   left on screen, not an effect preset; that screen is being rebuilt with the recording review.

=====================================================================================================
PART 3  RULES THAT REST ON A READING HE DID NOT CORRECT  ("INFERRED consent": told to him, he did not name it)
=====================================================================================================
Each item: reading number | the text AS TOLD to him (verbatim copy, file:line) | what is built on it | where the ruling says so | what changes if he corrects it. "shown to him: YES" for all (they were told with a question or an answer); "corrected: NO" unless said. Count: 13 items.

--- RO-1  R87 / R100: the clip's own Snap goes  (the one the sweep was told to look for) ---
Told (C135:45 and C135:91-93), VERBATIM:
 R87 Today each clip also has its own beat-snap setting. It goes too.
 R100 A clip's BPM panel is modelled on your picture of Resolume's Transport in BPM Sync: the timeline, back / pause / play,
     the loop menu, the play-out menu, Speed, Beats with "-" "+" "/2" "x2". It has no snap setting, so R87 stands: the
     clip's own Snap goes.
What he actually said (BL:793, VERBATIM): "r87 this is the resolume bpm clip menu, lets model ours based on this: [Image #9]" -- a picture with no snap row; he did NOT say "snap goes" in words.
Built on it: quantize-out lane (QO-2: the clip's Snap box leaves; the clip half of `Layer::triggerClip`'s gate comes back if he corrects); the clip's two saved keys "beatSnap" / "beatSnapMode" stay in the file, read by nothing (NC-7, HB-5: removed by ONE short stage only after he confirms R100 in words). His show: 15 clips each carry "beatSnapMode": 0 (my own read of the backup copy, VERIFIED).
Sources: RN2:855-856 ("R87 / R100 (the clip's Snap goes) -> QO-2's combo and the clip half of `Layer::triggerClip`'s gate come back; the values are still in his files (NC-7). R100 was told with his LAST message: he has not yet had a turn to correct it."), RN2:279 (ST-5: "the clip's Snap rests on a reading he has not confirmed in words ... REJECTED: asking him again"), RN2:873-876 (HB-5), RN2:917 (SF-C8: "'Not corrected' starts to mean something only after his next message"), plan-nudge-row2.md:247 and :742-743. VERIFIED.
Since R100 was told he sent three more messages (22:27:57, 22:32:45, 22:40:14: BL:845, BL:869, BL:877); none names R87 or R100 (VERIFIED by reading them). So "his next message" has now passed without a correction (INFERRED: consent by silence is still not his word).

--- RO-2  R101: the thirteen cells, the caption "BPM", upper-case TAP / RESYNC ---
Told (C135:94-96), VERBATIM:
 R101 The top row is laid out like your picture of Resolume's bar: square cells side by side in this order -- beat circle,
     play, pause, stop, "BPM" with the number, "-", "+", nudge back, nudge forward, "/2", "x2", TAP, RESYNC -- as far as it
     fits beside what else is in the top bar. The three old buttons are gone.
Built on it: `rowOrder()` / `rowTexts()` of the tempo row (RN2:857; plan-nudge-row2.md:743-744; plan-nudge-row2.md:343). His 145 B has since moved one name (the text between the two nudge buttons) -- that part is his answer, not a reading.

--- RO-3  R94: the routine pads, the bank of 8 and the name bands go (replaced by actions) ---
Told (C135:59-60), VERBATIM:
 R94 This replaces today's routines: the eight pads above the column numbers, the bank of 8 and the name bands on the
     layers go.
Built on it: RN2 reading R111 (nudge-row2; to be R117): pads keep their own start setting until actions replace them; NC-18: PL2's draft question 147 (the pad's Quantize menu) was NOT asked (text in PART 5, D-1); "if the pads stay, PL2's draft question on the pad's Quantize menu is asked under a new number" (RN2:858-859). Also HR-9 PARKED for the actions lane (RN2:891). Not named by him: BL:813 "R86, R88, R90, R91, R92, R93, R94 stand as told (INFERRED consent)". His own related words are R83 (BL:728, VERBATIM): "There will be a little area above each clip where there will be toggle buttons for each action that was recorded for that clip." -- that is about actions, not an explicit "the pads go".

--- RO-4  R86, R90, R91, R92, R93: the actions model (all "not named by him", BL:813) ---
Told (C135:43-44, :52-58), VERBATIM:
 R86 With Quantize out of the top bar, every clip you fire starts at once on your press; nothing waits for a beat or
     bar line. A BPM-synced clip is put in time by the one cut on the next "1" (your "71 b" and "123 b").
 R90 An action is a recorded movement of sliders (a clip's, its effects', a layer's, or the whole composition's). It is
     kept in beats together with the tempo it was recorded at, so at another tempo it still lands on the beat.
 R91 Every action belongs to one clip, one layer, or the composition. Its owner shows one small on / off button per
     action. On = it plays whenever its owner plays; a clip's actions start with the clip, in time with it.
 R92 You record a show (a take, with its sound). In the review screen you pick a stretch of the take and make it an
     action. From then on the action does not need the sound.
 R93 Quantize lives in the review screen only: it moves recorded button presses onto the exact beat or bar.
Built on it: nothing yet built; the whole ACTIONS lane (2f) and the RECORDING REVIEW SCREEN lane (2g) are to be planned on them (board.md "2f / 2g"; s-rta-1004b-work.md row 22:27:57 "START HERE"). R86 and R97 are said to need no consent because his own words carry them (RN2:861). R89 was told as "I have a new idea ..." and he then described the review screen (BL:800-801), which Harmony read as accepting it.
If he corrects: the actions design page (not authored; see PART 5, D-3).

--- RO-5  R88: loading a preset that has no signal on a slider UNPLUGS the signal ---
Told (C135:46-49), VERBATIM:
 R88 Question 131 is your first case: an effect on a clip (or a layer, or global) in the show. Your rule for it ("ctrl-z
     brings it back. There is no other way.") is the default A: loading a look that has no signal on that slider unplugs
     the signal; Cmd+Z brings it back. Your second case is as planned: a look is changed only by putting it on an effect
     in the show, changing that, and saving the look.
Built on it: `looks::kUnwiredEntry` = Unplug (the default A of the old question 131); `UnwiredEntry::Keep` stays built and unit-tested (LK-24), reachable from no screen; GL-9's second line is DORMANT (RL2:176-181, VERIFIED: "131 is READ as A (Harmony's reading R88, C135:46-49, told to him and not corrected -- INFERRED consent)... If he corrects R88: that one line, and GL-9's dormant line becomes the live one."). Harmony's own doubt in HANDOFF.md:110: "Whether Boris wants looks to UNPLUG signals (question 131's default) -- the critics called it the riskiest default." Check 7 (c) of RL2 (PART 2c) is where he would see it.
His own words (verbatim, BL:730): "If this is a clip in the show, then ctrl-z brings it back. There is no other way. If it is a look, there is no way to remove the signal unless the user drops it into the show (clip, layer or global) and then adds a signal and saves that look."

--- RO-6  R95: a preset is picked from the button on an effect in the show; it is NOT dragged from the effect list ---
Told (C135:80-82 -- as printed in the ANSWERS block), VERBATIM:
 R95 "Look" becomes "preset" everywhere you see it: the "Presets" button on an effect, "Preset 2", 'Save over "Preset 2"'.
     A preset is still picked from that button on an effect that is in your show; it is not dragged from the effect list
     (138's default A, which you did not change).
Built on it: question 138 read as A; RL2:183-191 (VERIFIED, "138's letters were not picked: READ as A ... INFERRED consent"); B would cost "about one more" stage of its own (RL2:184-191). His words (BL:786): "138 look is an effect preset. change the name look to preset to avoid further confusion." -- no letter.

--- RO-7  RB-3: what the other effects read after a preset is RENAMED (Harmony's own reading of his 133; never told as a reading) ---
RL2:885-886 (VERBATIM, within the paragraph copied in PART 2b): "Two rules rest on MY reading of his 133, which was never told to him as a reading: question 150's default (he is asked outright) and RB-3 (he is told in check 19; if he corrects it, the walk is limited to the renaming row)." His 133 (BL:732, verbatim): "133 no need for any of that. It can be called look 2 but no need to show that it was changed. Effects are usually changed by the user." Harmony's draft question 152 is withdrawn but its text is kept in PART 5 (D-2); HD-26 (RL2:908): "RB-3 is built without asking him. Default: yes. If she would rather ask, PL2:683-686 words the question". shown to him: only through check 19 once the build exists (NO today).

--- RO-8  Tempo-row readings R74, R77, R78, R79, R80, R82, R84 (BL:746: "stand as told (INFERRED consent)") ---
Told with questions 125-129 (boris-clarify-125-129.md:32-47), VERBATIM (R75, R76, R81, R83 were answered by him in words and are NOT in this item):
 R74 Pause never touches the nudge number.
 R77 The tempo you set by hand can go from 30 to 400; what the app hears by itself stays between 60 and 200. Above
     about 200, beat-driven looks flash fast (400 is almost 7 times a second).
 R78 "/2" and "x2" never move the "1": the beat just runs half or twice as fast.
 R79 The "Bar 1..4" text sits just left of the circle; the LOCKED word sits beside Manual.
 R80 The nudge buttons read "<<" (back = later = minus) and ">>" (forward = earlier = plus), because "-" and "+" are
     now the tempo's.
 R82 Opening a show, or New, never starts or stops the BPM timer. The app always launches with it running.
 R84 Resync while stopped or paused: the beat goes to the "1" and the nudge reads 0, as after any Resync.
Built on it: the tempo row (30..400 by hand, 60..200 heard; "/2" "x2" never move the "1"; nudge buttons "<<" ">>"; opening a show never starts or stops the beat; Resync while stopped). Changes by his later answers: R79 -> his 146 B (Bar text goes); R84 -> nudge-row2 R109 (to be R115); R80's "<<" / ">>" read against his picture (his 145 B puts the text "nudge X ms" between the two nudge buttons). R76 (switch Manual on): he asked what it means; "answered in chat" (BL:726, BL:753, BL:767; BL:800) -- he then asked again what the question about R76 was (BL:780); no record that he accepted the explanation: see D-4.

--- RO-9  Outputs readings R45..R56 (and R11, void) ---
plan-outputs.md:903 (VERBATIM): "Readings R45..R56 were told to him with the question and not corrected." Text, boris-clarify-51.md:11-22 VERBATIM:
- R45 The picture moves one frame at a time (about 17 ms); the Delay is set in whole milliseconds.
- R46 The Device row names the screen. It is not a menu; screens are switched on and off in the Outputs list, as today.
- R47 A screen you have set shows it in the Outputs list, beside its name: "40 ms", or "adjusted" for the colour rows.
- R48 The window ("Output Screens") never takes the keyboard: your keys keep launching clips while it is open. Values are set by dragging; a right-click puts a row back.
- R49 A screen's settings are remembered with the screen on this computer, whatever show is open. Cmd+Z does not touch them.
- R50 Two projectors of the same model with no serial number cannot be told apart by the Mac: their settings follow the socket, and the window says so.
- R51 Raising a Delay holds that screen's picture still for the time you added, then it carries on; lowering it skips ahead. Never black.
- R52 Opacity fades to black, on a screen and on Syphon.
- R53 Syphon is switched on and off where it is today, and is off at every launch; its settings are in the window. With a Syphon setting changed, Syphon runs one frame later.
- R54 Brightness, Contrast, Red, Green, Blue run from -1.00 to 1.00 with 0 in the middle. Resolume does not publish its formulas; ours are the usual ones.
- R55 These settings cannot be put on a key, a knob or a MIDI pad.
- R56 Video memory while any Delay is on: about 111 MB at 1920x1080, 443 MB at 3840x2160 -- the same for one screen or five.
Built on it: the whole Output Screens window (outputs lane S1 merged-gated at aa7bc8e; S2.. not started). plan-outputs.md:793 says the 0..500 range (R11) rested on "was told 0..500 and did not correct it" -- since replaced by his "51 default is good" (0..100 ms), so R11 no longer rests on anything.

--- RO-10  Nudge readings R25-R28 (and R24 which he confirmed) ---
ruling-nudge.md:153-158 (VERBATIM): "READINGS told to him about the nudge and not corrected (.harmony/.reports/s-rta-1004/boris-clarify-45-46.md:20-24): R25 two controls and the text, always shown; R26 "1 ms per press, a held key keeps moving, a number can be typed, -500 to +500"; R27 "Earlier / later can be put on a key or a pad (no knobs), as he ruled for Sync"; R28 "The number is remembered with the show"." Also ruling-nudge.md:201 (GA-8) and :420-424 (A16: the refusal of knobs "stays behind ONE predicate, `bool bindingIsLive(const Binding&)`"), plan-nudge.md:340 ("R28 told to him and not corrected: with the show"). Texts boris-clarify-45-46.md:20-24 (R24 line :20 confirmed by him: "You are exactly right"). Partly overtaken by his row (steps in whole numbers: 125) but R27 (no knobs) and R28 (in the show) still stand on consent only.

--- RO-11  Effect-presets readings (the old "look" readings R42, and the old file's R74-R77) ---
ruling-effect-looks.md:87 and plan-effect-looks.md:24 (R42, VERBATIM there): "Harmony's reading R42, told to him and not corrected (boris-clarify-86.md): a look belongs to ONE effect: a small menu on each effect -- ...". Texts: boris-clarify-86.md:15-18 and boris-clarify-101-106.md:30-33 (the file's own R74..R77, which overlap tempo-row R74..R77 in number: cite as "effect-looks R74"). Built on it: the preset store, the button on each effect, "a look holds that effect's slider values and its Dry / Wet. Not its bypass" (effect-looks R76). The old R77 (a take does not record mouse-moved effect sliders or a look being loaded) is FILED AS ITS OWN JOB, not yet planned (HANDOFF.md:95-103 "FILED DEBT", which lists "a take records neither mouse-moved effect sliders nor a look load"; RL2 check 13 tells him again). Texts boris-clarify-86.md:15-18 and boris-clarify-101-106.md:30-33 copied verbatim in D-5.

--- RO-12  One-save readings R62-R68 (and R33-R37, R38-R41, R57-R61, R69-R73) ---
HANDOFF.md:89-91 (VERBATIM): "Told to him as readings and not corrected (INFERRED consent, each in a boris-clarify file): my R1-R77 and the tempo-row ruling's own R74-R84 (the numbers overlap: cite those as "tempo-row R74"). One I got wrong and corrected to him (R41: his old deck files are NOT loadable); one I withdrew (R39)." The heaviest, VERBATIM boris-clarify-91-94.md:26-32:
- R62 Until the per-effect looks exist, no effects look can be saved or loaded: the old buttons go first.
- R63 The first Save of an old show keeps the file as it was in a folder called "backups" next to it.
- R64 In his old show each deck had its own layer settings; a show now has one set of layers, and Deck 1's win where they differ (in his file: the third layer's blend and keying, which Deck 2 does not use).
- R65 The computer remembers the keys at each Save of a show; a key change he does not save is gone at quit.
- R66 The list shows every file of the folder; one that is not a show is grey.
- R67 His four old files in Decks stay where they are and cannot be opened.
- R68 The quit window appears on every quit (never during my tests). "Save & Quit" on a show with no file asks where to save, and quits only once it is saved; a save that fails does not quit.
Built on them: the one-save lane (MERGE 1 done: "version": 2 written first, ONE verified writer, copy to backups/ before any overwrite = R63; S2..S7 not yet built: the list of shows, quit window, keys in the show). R64 (Deck 1's layer settings win) and R67 (his four old deck files stay and cannot be opened) change what he sees in his own show: his four deck files are unchanged on disk (gate G-OS4-0, s-rta-1004b-work.md row 22:43:31).

--- RO-13  R16 (pause is the layer's) -- RESOLVED, listed so it is not lost ---
ruling-transport-delta2.md:446-447 (VERBATIM): "Built by default as the plan: `RelaxedBool Layer::paused`. The basis is a reading told to him and not corrected (V18), not his word: question 72 is on his page BEFORE S2 starts". He then answered 72 in his own words (BD / BL:618-631: the pause is the CLIP's, kept until he changes it, saved with the show), so this reading no longer carries a rule.

=====================================================================================================
PART 4  REQUESTS TO HIM THAT ARE STILL OPEN  (shown to him: YES for each)
=====================================================================================================
--- RQ-0  Three tracks you would really play, about 10 minutes each ---
Text as asked (boris-clarify-71-74.md:32, VERBATIM): "RQ-0 Three tracks you would really play, about 10 minutes each: tell me where the files are."
His own message of 21:03:09 (BL:736, VERBATIM within it) asked "explain better what you need here: Three tracks you would really play, about 10 minutes each -- tell me where the files are. The first transport measurement (how often the picture would cut) cannot run without them."). Explained to him in chat (BF105, BL:775: "answered in chat 2026-10-04 (s-rta-1004b)"; board.md row 21:11:14 "explained to Boris again").
His answer, VERBATIM (BL:782, boris-msg-raw-2.txt line 3): "I will get you the 3 audio tracks in another session. need to get them from a dj"
Status: OPEN (BL:802: "RQ-0 stays open; the transport lane's SM-a stays BLOCKED"). BF115 [INFO] BL:843. Blocks: the transport lane's SM-a (the first tracker measurement; "no tracker row without them", ruling-transport-delta2.md:770-771 H-T2) and everything that waits on its measurement (stage S4t etc. per board.md row 3 "SM-a (BLOCKED: three tracks)"). Hardware fact: none of the three is on this rig (H-T2: "None on the rig: BLOCKED, and Boris is asked for three tracks"). VERIFIED sources: BL:782, boris-clarify-71-74.md:32, board.md:27+ rows 21:11:14 and 21:33:30.

--- RQ-3  Click plus once on Speed and on Duration and send the numbers ---
Text as asked (boris-clarify-71-74.md:37-38, VERBATIM): "RQ-3 Click plus once on Speed and on Duration and send the numbers; Speed 2 in Timeline and the Duration it shows; a clip longer than a minute with the time shown both ways."
Partly met: his line "one click on time jumps 1 bpm, duration in timeline mode moves up 0.1" (BL:739 and BL:761; BD 21:03:09; question 137 then asked which row "time" is, answered 21:33:30: "the plus moves the main bpm 1 bpm number regardless of bpm or timeline mode. the plus moves 1 beat in the clip that is in bpm mode"). NOT supplied by him: Speed's step in Timeline mode, "Speed 2 in Timeline and the Duration it shows", "a clip longer than a minute with the time shown both ways". plan-transport-answers.md:376 (VERBATIM): "Minus / plus: 0.1 (the ruling's step; Resolume's is not documented, request RQ-3 stands)" -- so Speed's step is built as 0.1 by default until he sends numbers. Status: OPEN, nothing waits (the request says "nothing waits on them"). INFERRED that he considers it settled; not confirmed.

--- RQ-1 / RQ-2 (of the transport file) and RQ-1 (of the actions file) -- MET, listed so they are not re-asked ---
- boris-clarify-71-74.md RQ-1 (BeatLoopr row) and RQ-2 (Random menu): MET by his three pictures of 21:03:09 (BL:741-746 and BF106 at BL:776): resolume-beat-repeat.png, resolume-random-1.png, resolume-random-2.png.
- C135:61-63 RQ-1 (the review-screen picture): MET 22:32:45 (BD:1066; BL:869-873): boris-images/review-screen-reference.png.
- Harmony's own question to him ("what is the recording review screen that you are looking for?" BL:780) was answered in chat (BL:801).

--- Anything else asked of him? ---
Checked: C135 (RQ-1 only), C144 (none), the board and the work log. No other request addressed to him. VERIFIED.

=====================================================================================================
PART 5  OTHER THINGS THAT ARE OPEN WITH HIM OR THAT ONLY HE CAN SETTLE (not numbered questions)
=====================================================================================================
Each: id | verbatim text | source | shown to him.

--- D-1  A DRAFT question that was dropped, to be asked under a NEW number only if the pads stay (RN2 NC-18) ---
shown to him: NO. Source: plan-nudge-row2.md:720-723 (the plan's own draft 147; the ruling re-assigned the number 147 to the stop/Freeze question, RN2:500-503; RN2:858-859: "if the pads stay, PL2's draft question on the pad's Quantize menu is asked under a new number"). VERBATIM:
147. Each routine pad has its own "Quantize" in its right-click menu and waits for the next bar by default. Quantize
     has left the top bar and the clips. Until actions replace the pads:
     A (default) Leave the pads as they are.
     B Take that menu out now; a pad starts the instant you press it.
Status: NOT a live question: the pads are to be replaced by actions (R94, PART 3 RO-3), and the engine work belongs to the actions lane (plan-nudge-row2.md:728-731: "it goes to the actions lane's architect, not to a builder here").

--- D-2  A DRAFT question withdrawn but with a standing way back (looks-answers2 HD-26) ---
shown to him: NO (it is told to him only as check 19 of RL2 section 6, once a build exists). Source plan-looks-answers2.md:683-686; RL2:908-910 (HD-26: "RB-3 is built without asking him. Default: yes. If she would rather ask, PL2:683-686 words the question; its B is what RB-3 rules"). VERBATIM:
152. Several effects carry "Preset 2". You rename it to "Wobble" from one of them.
     A (default) That effect reads "Wobble". The others read "Wobble" too while they still are exactly that preset; the ones
       you changed read "Presets" until you load a preset on them again.
     B Every effect in the open show that carried "Preset 2" reads "Wobble".

--- D-3  The ACTIONS design and the RECORDING REVIEW SCREEN: not yet designed, no page for him yet ---
shown to him: NO (not authored). Source: s-rta-1004b-work.md row 22:27:57 (VERBATIM): "The DESIGN page for actions + the review screen is NOT started this session (off-ramp passed ...): it is the next session's START HERE, long task, with BF96, BF98-BF100, BF111, BF114, BF116-BF119, readings R89-R94, R98-R107 and the two fact sheets as its inputs." Board.md rows 2f / 2g (21:11:14, 21:33:30). His own caution (BF114 at BL:842, VERBATIM): "We need to really think about this to make sure it works very well and there's no confusion so in the display where all of the parameter changes are displayed, they are displayed in a very smart hierarchy so depending on what they are, they could be put into the right hierarchy, meaning clip, layer or Global." Open design questions that the sources raise and that are NOT yet written as numbered questions to him (INFERRED from the sources; none is a file-line quote): where composition-level action toggles live ("We will also find a place to have composition level actions", BL:728 verbatim: "We will also find a place to have composition level actions, but they will simply be toggle buttons as well."); the "transition" slider's range and where it sits (R106); whether the "ignore actions" switch is a UI row on every control (R107). VERIFIED that the first is his own words; the other two are Harmony's readings R106/R107 (told, not yet corrected).

--- D-4  R75 "we are calling routines actions. We need to discuss this" and R76 ---
Source BL:725-726 (his words, VERBATIM): "R75 we are calling routines actions. We need to discuss this. ..." and "R76 please explain wha tit means to switch manual on". Status: BF96 "[NEW, actions -- TO DISCUSS]" and BF97 "[ASK] What 'switch Manual on' means (R76) -- answered in chat 2026-10-04 (s-rta-1004b)" (BL:767). He then wrote "ok what is your question about r76 having to do with manaul?" (BL:780); Harmony: "he took R76 for a question. It is a statement of what the app will do; nothing is asked. Answered in chat." (BL:800). The chat answers are not in the files I read: INFERRED that the explanation was given; there is NO record that he accepted it. R76 text as told (boris-clarify-125-129.md:35, VERBATIM): "R76 With the app listening, "-", "+", "/2", "x2" or a typed tempo switch Manual on."

--- D-5  The effect-preset readings, VERBATIM (promised in RO-11) ---
boris-clarify-86.md:15-18:
## MY READINGS (told to him; he corrects only what is wrong)
- R42 A look belongs to ONE effect: a small menu on each effect -- pick a look, or keep the current settings as a new look. Kept by the app for every show, the moment it is made.
- R43 CORRECTS R41: his four deck files are in an older format that today's app already does not list or load; his one show is in the older shape and opens through a conversion.
- R44 His one show ("test with harry"): the first plain Save after opening it rewrites the file in the new shape, with no backup. A copy was made inside the project (.harmony/.reports/s-rta-1004/boris-show-backup/, same checksum) without touching his file.
boris-clarify-101-106.md:30-33 (called R74..R77 there; cite as effect-looks R74.. -- the numbers overlap tempo-row R74..R77):
- R74 One small button on each effect, next to its X: it shows the name of the look that is on, or "Looks". Its menu: Default, your looks, New Look, Rename, Delete.
- R75 A look is kept on disk the moment you make it; every show sees it. Loading one is one Cmd+Z step. It never touches the layer strip.
- R76 A look holds that effect's slider values and its Dry / Wet. Not its bypass.
- R77 FOUND, told to him: a take does not record effect sliders moved with the mouse today, nor a look being loaded -- against his "record all the parameter movements". Filed as its own job.
Note: "look" is renamed "preset" everywhere on screen by his 138 (BL:786); these were told to him as "look".

--- D-6  Three things inside the NUDGE-ROW2 "what only Boris can check" list that are really questions to him (RN2:769-803; shown to him: NO; they describe a build not yet made) ---
VERBATIM RN2:792-800 (B-6, B-7, B-9):
B-6  At arm's length on your screen, beside your picture of Resolume's bar: is the row in your order; are the cells
     the shape you meant, and big enough (they are as tall as today's top bar, a little smaller than Resolume's); can
     you see at a glance which of play / pause / stop is on; can you tell play ">" from nudge forward ">>"; is "nudge
     +12 ms" readable right after RESYNC (question 145); is "Bar 2" left of the circle wanted (question 146)?
B-7  Make the window narrow -> the row never loses a cell; FPS / DSP go first, then the LOCKED word, then Master
     Signal, then Master. Is that the right order to give up?
B-9  (a confirmation; a machine row proves it) Record a take while you pause, play and stop the beat, and press
     Resync; replay it -> the replay does not pause, stop or start your beat. A take you replay after pressing stop
     keeps replaying and puts its clips back (reading R104): tell us if stop should end a replay too.
Whole of RN2 section 6 (B-1..B-10 as replaced) is RN2:769-803; the other rulings' section 6 lists are at: ruling-one-save.md:777, ruling-outputs.md:768, ruling-nudge.md:697, ruling-nudge-row.md:677, ruling-transport-delta2.md:695, ruling-transport-answers.md:667, ruling-effect-looks.md:657, ruling-looks-answers.md:723 (the last two REPLACED by RL2 section 6). All are "Nothing here is shown to him before the visual gate has passed"-type lists: pending until each lane's visual gate. I did not copy them (outside the ask).

--- D-7  Decisions that are Harmony's but that the sources say are decided "with his check" or may go to him ---
All VERBATIM-pointers, shown to him: NO.
- HB-6 top bar height (RN2:877-879): "DEFAULT 34: cells 30. ALTERNATIVE 40: cells 36 -- `kTopBarHeight`, `kRowCell` and T-RB4's sums re-stated by the architect; every panel below moves 6 px; other lanes' baselines change. Decide after the visual gate, with his B-6."
- HB-9 question 147 (RN2:884-885): asked with 144-146 and answered A; "FM-B4 is still measured and reported" (plan-nudge-row2.md:820). Closed except the measurement.
- RL2 HD-24 an identity for a preset beyond its name (RL2:898-903, default NOT in this lane; "once presets exist on his disk it raises the file's version to 3"), HD-25 a pick of an equal preset pushes no undo step (default: leave it), HD-26 (D-2), HD-31 (RL2:917-920).
- Harmony's own recorded doubts (HANDOFF.md:110-112, VERBATIM): "Whether Boris wants looks to UNPLUG signals (question 131's default) -- the critics called it the riskiest default." / "Whether "delete them. this is a new build" (106) was meant wider than the two files I named: I kept it narrow." / ""Routines" -> "actions": "action" is already a word in the code and maybe on screen (binding actions): unchecked." (his 134 "delete them too" reads as the nine FX Saves only; Harmony moved exactly those: BL:774 BF104).

--- D-8  Not Boris's, but must not be lost (owed by Harmony; listed because they gate what he will be asked) ---
- ADOPTION of ruling-looks-answers2.md is OWED (not adopted: board.md row 22:36:19 says "NOT YET ADOPTED" for nudge-row2 only; the work log row 22:43:31 "ADOPTION OWED, noted at the end of plan-looks-answers2.md; questions 150, 151 go on his page").
- ADOPTION of nudge-row2 DONE 22:40:14 (plan-nudge-row2.md:814-826; HB-1..HB-11 at line 820); its R109..R120 readings are told on "the page boris-open.html of this session's close" (plan-nudge-row2.md:821): so they are NOT told yet.
- STOP-N1 and STOP-N2 (rulings-nudge.md:13 and :22; plan-nudge-row2.md:822): architect line owed before S1r and S2 packets.
- An architect delta on the transport lane (Quantize rows of S1, S2, S4c, S6; the pictures; step sizes) is NOT launched (s-rta-1004b-work.md row 21:38:02; plan-transport-answers.md:891-892).
- FM-8 (Q73) not run; FM-7 (real file durations) etc. are measurements, not questions.
