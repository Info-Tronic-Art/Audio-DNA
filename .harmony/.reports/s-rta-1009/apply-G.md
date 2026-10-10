# APPLY G -- Output screens (s-rta-1009, page 2)
Written 2026-10-09 18:46 EDT by the architect of topic G. Read, not run: nothing was built, launched or measured.

## SUMMARY
- One box of his lands here: 265, "b" (BF269). All Outputs Off now HOLDS: after it the app switches on no output by itself -- not when a show is opened, not when a screen is plugged in, not Syphon -- until he switches an output on himself.
- 237, 263 and 264 were left empty and stand as written; none of his 33 boxes says otherwise. Each now carries the hold of 265 as its one exception.
- Squared: a show still remembers its outputs and brings them on when it is opened (his L96 of 2026-10-07); opening a show never switches one off (237); the hold of 265 is the only time an opened show adds nothing.
- Restore Last Outputs is read as his own act: after All Outputs Off it brings back the outputs that were on, and ends the hold (assumption G3-1, a line he can strike).
- The one real doubt, put to him: the hold ends at a quit, so the next start of the app brings the last show's outputs on (G3-2, ASK YES).
- Old spec: R191 g3, g5, g6, g9 and g11, item 200 and R209 are amended (five of the seven are the hold; two are the name Studio).
- No answer is owed by this topic.

## ITEMS
@@ITEM 237
TITLE: Opening a show only adds outputs, never switches one off
STATUS: ACCEPTED as written; tested against all 33 boxes, none says otherwise; the hold of 265 (BF269) is its one exception
HIS: box left empty = accepted as written; for the exception BF269 "b"
RULE: Opening a show never switches off an output that is on, whichever show it is and whatever that show remembers. It only adds: every output the opened show remembers and that is plugged in switches on, without a click. A show that remembers no output leaves the outputs exactly as they are. An output that was on when the show was opened stays on and is remembered by that show from then on, with @@ITEM R191 parts g1, g2 and g4 as they stand. The one exception to "adds": while All Outputs Off holds (item 265), an opened show adds nothing; it still switches nothing off, because nothing is on. The start of the app is the opening of the last show and follows the same rule (R191 g5).
CHANGED: nothing: accepted as written. The other way he read (the outputs set exactly as saved, every other output off) is rejected by the empty box. Old assumption G-1 is closed by it; R191 g4 already says the same and is not amended. New against the item as he read it: the exception for the hold, which comes from his "b" on 265 (BF269).
TODAY: a show never opens, closes or remembers an output; outputs are the computer's state (spec-G.md R191 TODAY; area-outputs.md part 1 items 10, 12; CLAUDE.md "Outputs"). To build: the show file's list of remembered outputs and the add-only switch-on at load (Pitfall 68: the file's shape and version change). Not re-checked in the program text by me.
@@END
@@ITEM 263
TITLE: A show remembers Syphon like a screen
STATUS: ACCEPTED as written; tested against all 33 boxes, none says otherwise; the hold of 265 (BF269) covers Syphon too
HIS: box left empty = accepted as written; for the hold BF269 "b"
RULE: Syphon is an output and a show remembers it exactly as it remembers a screen: a show saved with Syphon on switches Syphon on by itself when that show is opened, also when the app starts and opens the last show; a show that does not remember Syphon leaves Syphon as it is, on or off. The show forgets Syphon only when he unticks Syphon's own line in the Outputs list. Syphon goes off with All Outputs Off and with Cmd+Shift+Esc, and while that holds (item 265) no opened show switches Syphon on; ticking Syphon's line or Restore Last Outputs does. Everything else with @@ITEM 200 as it stands (one line of the Outputs list with its own tick; other programs see "Audio-DNA" only while it is ticked; the same eight settings as a screen, kept on the computer).
CHANGED: nothing: accepted as written. The other way he read (Syphon stays off until he ticks it) is rejected by the empty box. Old assumption G-2 and its twin X-7 are closed by it. New against the item as he read it: the hold of 265 reaches Syphon as it reaches a screen (BF269; Syphon is an output by his earlier word, binding-decisions.md, quoted in spec-G.md item 200).
TODAY: "Syphon Output" is a tick of the Output menu outside the per-display list, not switched off by All Outputs Off, off at every launch, not remembered by anything; the Syphon server is created at start whether ticked or not (spec-G.md item 200 TODAY; area-outputs.md part 1 items 15, 17, H5, H6: INFERRED there, not run). All of that changes.
@@END
@@ITEM 264
TITLE: A show brings back only the very screens it remembers
STATUS: ACCEPTED as written; tested against all 33 boxes, none says otherwise
HIS: box left empty = accepted as written
RULE: A show brings back only the very screens it remembers, each recognised as the Mac reports that screen, never by its socket, its place or its size. With a different projector in its place, or on another computer, nothing comes on by itself: the picture never goes to a screen he did not pick. He ticks the new screen once in the Outputs list; from then on the open show remembers it and writes it into the show file at the next Save. The screen that is missing stays remembered by the show, and comes on by itself when it is plugged in again while that show is open and All Outputs Off does not hold (item 265). No message is shown for a remembered screen that is missing (assumption G3-3). With @@ITEM R191 parts g6 and g8 and @@ITEM D28 as they stand.
CHANGED: nothing: accepted as written. The other way he read (the picture goes to whatever output screen is plugged in) is rejected by the empty box. Old assumption G-5 is closed by it. The item as he read it no longer carried G-5's words "no message is shown": that half stays Harmony's own (G3-3, not asked).
TODAY: an output is known only by six numbers of place and size; no display id or name is read (spec-G.md D28 TODAY; area-outputs.md part 1 item 7, T3). To build: one recognition of a screen, used for its settings and for the show's list. What his own screens report is measured after the build.
@@END
@@ITEM 265
TITLE: After All Outputs Off nothing comes on by itself
STATUS: ANSWERED way b
HIS: BF269 "b"
RULE: All Outputs Off, which is also Cmd+Shift+Esc, switches off every output, the screens and Syphon, and from that moment it HOLDS: the app switches on no output by itself. While it holds: (1) opening a show switches nothing on, whatever that show remembers, and whether it is another show or the same one opened again; (2) plugging in a screen switches nothing on, whether it was on before, is remembered by the open show, or is new; (3) Syphon stays off. The hold ends when he switches an output on himself: a tick on a screen's line or on Syphon's line in the Outputs list, Cmd+F for the main display's output, or Restore Last Outputs, which brings back the outputs that were on before All Outputs Off (G3-1). The act that ends the hold switches on only what it names: one tick brings on that one output and no other. After that everything works as it does without a hold: a show opened afterwards brings on the outputs it remembers and that are plugged in (R191 g3), and a screen the open show remembers comes on when it is plugged in afterwards (R191 g6). The hold changes nothing in what a show remembers: no show forgets an output because of All Outputs Off, and a Save made during the hold keeps the show's remembered outputs (R191 g2). The hold is kept neither in the show nor on the computer: it ends when the app is quit, so the next start of the app opens the last show and brings that show's outputs on (G3-2). Switching every output off one by one, each by its own line, is not All Outputs Off and starts no hold. A start of the app made by a test or a tool never switches an output on, hold or no hold.
CHANGED: way b replaces the item's own text ("for the moment only: a show you open afterwards brings its outputs on again"). Against the old blocks: R191 g9 said that opening a show after All Outputs Off brings that show's outputs on again (old assumption G-15); that half no longer holds and R191 g3, g5, g6, g9 and item 200 are amended. His "b" says only "nothing comes on until you switch an output on yourself"; three things in the RULE are Harmony's reading, not his word: that Restore Last Outputs counts as switching on himself and that one tick brings on only that output (G3-1), and that a quit ends the hold (G3-2). That a plug alone brings nothing back after All Outputs Off was Harmony's in the old spec; his "nothing comes on" now covers it.
TODAY: All Outputs Off closes every output window and cancels the pending reopen of unplugged screens (area-outputs.md part 1 item 11); Syphon is not touched by it (item 15, H5); the screens opened in the session leave the saved set, so Restore Last Outputs then has nothing to open (CORRECTIONS W1, M1: INFERRED from code there, not run). No show opens an output, so no hold exists. To build: the hold as one flag of the running app, set by All Outputs Off, cleared by any switch-on of his, read by the show-open and plug-in paths; Restore Last Outputs keeping its set through All Outputs Off; Syphon inside All Outputs Off. Not re-checked in the program text by me.
@@END

## AMENDMENTS
@@AMEND R191 1
OLD: (g3) When the show is opened, every remembered output that is plugged in switches on by itself, without a click and without a message.
NEW: (g3) When the show is opened, every remembered output that is plugged in switches on by itself, without a click and without a message, except while All Outputs Off holds (g9): then opening a show switches nothing on.
HIS: BF269 "b"
WHY: g3 let every opened show switch its outputs on; his way b of 265 stops that after All Outputs Off.
@@END
@@AMEND R191 2
OLD: (g5) The same happens at the start of the app, because the app opens to the very last show (L94; G-3).
NEW: (g5) The same happens at the start of the app, because the app opens to the very last show (L94; G-3), also when All Outputs Off was used before the app was quit: a quit ends its hold (assumption G3-2; the other reading keeps the hold over a quit, so that the start brings nothing on).
HIS: BF269 "b"
WHY: g5 did not say whether All Outputs Off reaches over a quit; his way b makes All Outputs Off hold, so its end has to be said.
@@END
@@AMEND R191 3
OLD: unless All Outputs Off was used since (g9)
NEW: unless All Outputs Off holds (g9)
HIS: BF269 "b"
WHY: "used since" had no end; the hold of way b ends when he switches an output on himself, and a screen plugged in after that comes on again.
@@END
@@AMEND R191 4
OLD: (g9) All Outputs Off and Cmd+Shift+Esc switch every output off for now: after them a plug alone brings nothing back, "Restore Last Outputs" brings back the outputs that were on, and opening a show brings that show's outputs on again (G-15).
NEW: (g9) His way b of item 265: All Outputs Off and Cmd+Shift+Esc switch every output off, Syphon included, and then hold: the app switches on no output by itself, not when a show is opened (another show or the same one again), not when a screen is plugged in, until he switches an output on himself. He does that with a tick on an output's own line, with Cmd+F for the main display's output, or with "Restore Last Outputs", which brings back the outputs that were on before All Outputs Off (assumption G3-1). The act that ends the hold switches on only what it names. After it a show that is opened brings its outputs on again (g3) and a remembered screen that is plugged in comes on again (g6). The hold makes no show forget an output (g2), is kept neither in the show nor on the computer, and ends when the app is quit (assumption G3-2). Switching every output off by its own line starts no hold.
HIS: BF269 "b"
WHY: g9 said, with old assumption G-15, that a show opened after All Outputs Off brings its outputs on again; he chose the other way.
@@END
@@AMEND R191 5
OLD: Opening a recording in Review
NEW: Opening a recording in Studio
HIS: BF245 "Let's go with studio. That's perfect."; BF263 "b"
WHY: The screen where a recording is watched was called Review; he named it Studio in this round.
@@END
@@AMEND 200 1
OLD: (assumption G-2; the other reading keeps Syphon off at every start and at every opening of a show until he ticks it)
NEW: (item 263, accepted as written). While All Outputs Off holds (R191 g9, his way b of item 265) no opened show switches Syphon on: Syphon comes on again by a tick on its own line or by "Restore Last Outputs"
HIS: BF269 "b"; item 263 accepted as written
WHY: The rule carried the other reading of G-2, which he has now turned down, and let every opened show switch Syphon on, which his way b of 265 stops after All Outputs Off.
@@END
@@AMEND R209 1
OLD: While Review is open the outputs are not frozen either
NEW: While Studio is open the outputs are not frozen either
HIS: BF245 "Let's go with studio. That's perfect."; BF263 "b"
WHY: The screen was called Review; he named it Studio in this round. What the outputs show while it is open stays topic E's.
@@END

## ASSUMPTIONS
@@ASSUME G3-1
ABOUT: 265; R191 (g9, i); 200
TEXT: I assume that after All Outputs Off, Restore Last Outputs still brings back the outputs that were on, because you press it yourself. Ticking one output brings on only that one.
WHY: BF269 "b" says nothing comes on until he switches an output on himself; whether Restore Last Outputs is such an act, and what else comes on with one tick, is not said.
ALT: b) Restore Last Outputs does nothing after All Outputs Off: you tick each output yourself. c) When you tick one output, the other outputs the open show remembers come on with it.
IF-WRONG: STAGE after a cut he presses Restore Last Outputs and either nothing or more than he expected comes on; a small change to undo
ASK: LINE Restore Last Outputs is a command only he can press, so it fits his "yourself"; he would most likely wave it through
@@END
@@ASSUME G3-2
ABOUT: 265; R191 (g5, g9)
TEXT: I assume All Outputs Off holds only until you quit the app: at the next start your last show brings its outputs on by themselves, as before.
WHY: BF269 "b" gives the hold no end but his own switch-on; his L96 of 2026-10-07 wants a saved show to bring its outputs back without connecting them again. A quit sits between the two.
ALT: b) It holds over a quit too: at the next start nothing comes on until you switch an output on yourself.
IF-WRONG: STAGE a projector comes on by itself at the start of the app after he had cut all outputs; or, built the other way, he must switch his outputs on by hand after every night that ended with All Outputs Off
ASK: YES no word of his says where the hold ends besides his own switch-on; he would see it at the start of the app before a show; it is about what the app does
@@END
@@ASSUME G3-3
ABOUT: 264; R191 (g6)
TEXT: I assume no message is shown when a screen that a show remembers is not plugged in: the show simply opens without it.
WHY: Item 264, which he accepted, says nothing comes on by itself; it does not say whether the app tells him a remembered screen is missing.
ALT: b) A short note names the remembered screen that was not found.
IF-WRONG: SMALL a note can be added later; he sees at once that the projector is dark
ASK: NO how a missing screen is told is a matter of look, left to the UI redesign
@@END
@@ASSUME G-17
ABOUT: R191 (g11)
TEXT: I assume opening a recording in Studio never switches an output on or off by itself, although the show file saved with that recording remembers the outputs of that night.
WHY: Unchanged in what it says; only the screen's name is new (BF245, BF263). His L96 lets an opened show switch outputs on; whether a recording's own show file counts is not said.
ALT: b) Opening a recording in Studio brings on the outputs that were on that night.
IF-WRONG: SMALL he ticks one output to watch a recording on the big screen
ASK: NO the careful choice at no cost; topic E owns Studio and what the outputs show while it is open
@@END

## QUESTIONS BACK
(none owed by this topic)

## NAMES
@@NAME All Outputs Off
MEANS: The command, also Cmd+Shift+Esc, that switches off every output, Syphon included, and after which nothing comes on by itself until he switches an output on himself.
SOURCE: the on-screen name, kept; what follows it: his way b of item 265 (BF269); refines the row "All Outputs Off" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END
@@NAME Restore Last Outputs
MEANS: The command that switches on again the outputs that were last on on this computer, whatever show is open, also after All Outputs Off.
SOURCE: the on-screen name, kept; "also after All Outputs Off" is Harmony's reading of BF269 (assumption G3-1), not his word
@@END

## REACHES OTHER TOPICS
- F (R188 a, the show file; R189, opening a show): opening a show after All Outputs Off switches no output on (BF269); the show's remembered outputs are loaded and kept all the same. The hold is not a field of the show file.
- F (old assumption F-12, the twin of G-3: outputs at the start of the app): stands, with one new edge that this paper asks once for both topics (G3-2: a quit ends the hold).
- X (old assumption X-7, the twin of G-2; old item C19): closed by item 263, accepted as written; the hold of 265 reaches Syphon too (BF269).
- J (item 271, accepted as written: switching an output screen on or off cannot go on a key or a pad): it fits 265 b. "Switch an output on yourself" therefore means the Outputs list, Cmd+F and Restore Last Outputs, never a pad. Whether All Outputs Off and Restore Last Outputs can go on a pad is J's (old J-18 named both; item 271 as he read it names only the on and off of an output screen).
- E (item 186 b, the outputs while Studio is open; old E-1): closing or opening Studio during a hold must not switch an output on; G-17 says so for opening a recording. E rules what an output that is on shows.
- E (item 231, BF253 "It defaults to running next to the show recording"): applied here without an amendment: the low-resolution show recording runs beside record show, so D26 (no change of picture size while a recording runs) locks no longer than record show does. The worry in the old ruling notes (a lock all night) falls away.
- F (item 236 and the answer on Snapshot, BF243 "with all the settings and the output and everything", BF257): if a Snapshot is a save of the show, F must say whether opening one counts as opening a show for the outputs (then 237 and 265 apply as written) and whether it still holds a still picture; R226 b's word "snapshots" (the Transform shows in them) waits on that and is not amended here.
- B (item 252, BF267, the monitors at one frame rate): not applied here; R209 (the outputs keep going whatever the main window shows) is untouched by it.

## CONFLICTS
- Not his word against his word, but a narrowing he should hear once in one line. New, BF269 on item 265: "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself."). Earlier, binding-decisions.md:1175 (2026-10-07, BF216): "R191 if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." Squared: the show still remembers and brings its outputs on; only after All Outputs Off does he switch one on himself. The edge between the two is the quit (G3-2).
- His way b against the project's standing rule, which is no word of his: CLAUDE.md "Outputs" says "the app never opens an output by itself (only Output > Restore Last Outputs does". A grep for "never opens" in binding-decisions.md finds nothing, so no quoted word of his says it (the old spec found the same). His L96 replaced that rule on 2026-10-07; BF269 brings it back for one case only, the time after All Outputs Off. CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md are rewritten when it is built.

## NOT DONE / UNSURE
- Where the hold ends (G3-2) is the only thing here that needs his answer. Cheapest: one question, "After All Outputs Off and a quit, does the next start bring your show's outputs on?"
- A corner the hold makes easier to reach, left with old assumption G-6 (not shown to him, unchanged): a show forgets an output only when he switches it off by its own line; during a hold that output is already off, so to make the show forget it he must tick it on and off again. Cheapest: leave it; if it bothers him after the build, a "forget this screen" entry is a small addition.
- Whether All Outputs Off pressed when no output is on also starts a hold: written as yes (the command always holds); not asked, because the only effect is that a show opened next adds nothing until he ticks an output. Cheapest: fold into the question on G3-2 if he asks.
- TODAY lines are taken from spec-G.md and the fact sheet area-outputs.md; I opened no source file for this paper. Nothing was run.
- Old assumptions G-4, G-6, G-7, G-8, G-9, G-10, G-11, G-12, G-13 and G-16 were tested against the 33 boxes and the four items: untouched, nothing written. G-17 keeps its sense and gets the name Studio. No old assumption of the internal list is dropped.
