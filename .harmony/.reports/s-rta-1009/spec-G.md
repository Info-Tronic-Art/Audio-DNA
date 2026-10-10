# SPEC G -- Output screens (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-G.md, ruling rule-G.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 237
TITLE: Opening a show only adds outputs, never switches one off
STATUS: ACCEPTED as written; tested against all 33 boxes, none says otherwise; the hold of 265 (BF269) is its one exception
HIS: box left empty = accepted as written; for the exception BF269 "b"
RULE: Opening a show never switches off an output that is on, whichever show it is and whatever that show remembers. It only adds: every output the opened show remembers and that is plugged in switches on, without a click. A show that remembers no output leaves the outputs exactly as they are. An output that was on when the show was opened stays on and is remembered by that show from then on, with @@ITEM R191 parts g1, g2 and g4 as they stand. The output on the main display is the exception that R191 part g7 keeps: it stays on like any other when a show is opened, but no show takes it and no show brings it on. The one exception to "adds": while All Outputs Off holds (item 265), an opened show adds nothing; it still switches nothing off, because nothing is on. The start of the app is the opening of the last show and follows the same rule, the hold included (R191 g5 as amended; item 265).
CHANGED: nothing: accepted as written. The other way he read (the outputs set exactly as saved, every other output off) is rejected by the empty box. Old assumption G-1 is closed by it; R191 g4 already says the same and is not amended. New against the item as he read it: the exception for the hold, which comes from his "b" on 265 (BF269), also at the start of the app (assumption G3-2). Named by the ruling so that the rule is complete: the main display's output (R191 g7, Harmony's own under old assumption G-4, never shown to him).
TODAY: a show never opens, closes or remembers an output; outputs are the computer's state (spec-G.md R191 TODAY; area-outputs.md part 1 items 10, 12; CLAUDE.md "Outputs"). To build: the show file's list of remembered outputs and the add-only switch-on at load (Pitfall 68: the file's shape and version change). Read in the papers named, not in the program text; nothing run.
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
RULE: A show brings back only the very screens it remembers, each recognised as the Mac reports that screen, never by its socket, its place or its size. With a different projector in its place nothing comes on by itself: the picture never goes to a screen he did not pick. He ticks the new screen once in the Outputs list; from then on the open show remembers it and writes it into the show file at the next Save. The screen that is missing stays remembered by the show, and comes on by itself when it is plugged in again while that show is open and All Outputs Off does not hold (item 265). On another computer no screen comes on by itself either, also not the same projector, and he ticks it once there (assumption G3-4: Harmony's own, not in the item he read). No message is shown for a remembered screen that is missing (assumption G3-3). With @@ITEM R191 parts g6 and g8 as amended and @@ITEM D28 as it stands.
CHANGED: nothing in what he accepted. The other way he read (the picture goes to whatever output screen is plugged in) is rejected by the empty box. Old assumption G-5 is closed by it only as far as the item says it. Two halves of G-5 were not in the item as he read it and stay Harmony's own: "no message is shown" (G3-3, not asked) and "or on another computer" (G3-4, not asked; R191 g8 is amended to say so).
TODAY: an output is known only by six numbers of place and size; no display id or name is read (spec-G.md D28 TODAY; area-outputs.md part 1 item 7, T3). To build: one recognition of a screen, used for its settings and for the show's list; under G3-4 the show's list ties each remembered screen to the computer it was ticked on (under its way b the list holds screens alone). What his own screens report, and what a second Mac reports for the same projector, is measured after the build.
@@END

@@ITEM 265
TITLE: After All Outputs Off nothing comes on by itself
STATUS: ANSWERED way b
HIS: BF269 "b"
RULE: All Outputs Off, which is also Cmd+Shift+Esc, switches off every output, the screens and Syphon, and from that moment it HOLDS: the app switches on no output by itself. While it holds: (1) opening a show switches nothing on, whatever that show remembers, and whether it is another show or the same one opened again; (2) plugging in a screen switches nothing on, whether it was on before, is remembered by the open show, or is new; (3) Syphon stays off; (4) a start of the app switches nothing on: the hold is kept on the computer over a quit and a new start, so the last show then opens with no output on (assumption G3-2; the other reading ends the hold at a quit). The hold ends only when he switches an output on himself: a tick on a screen's line or on Syphon's line in the Outputs list, Cmd+F for the main display's output, or Restore Last Outputs, which brings back the outputs that were on before All Outputs Off, also after a quit and a new start (@@ITEM R191 part i as it stands). The first output he switches on ends the hold for every output (assumption G3-1; the other reading keeps each output off until he switches that very one on). The act that ends the hold switches on only what it names: one tick brings on that one output and no other. After that everything works as it does without a hold: a show opened afterwards brings on the outputs it remembers and that are plugged in (R191 g3), and a screen the open show remembers comes on when it is plugged in afterwards (R191 g6). The hold changes nothing in what a show remembers: no show forgets an output because of All Outputs Off, and a Save made during the hold keeps the show's remembered outputs (R191 g2). The hold is the computer's and never a part of a show file: a show carried to another computer brings no hold with it. Only the command starts a hold: it starts one also when no output was on at the press, and switching every output off one by one, each by its own line, starts none (assumption G3-5). A start of the app made by a test or a tool never switches an output on, hold or no hold (R191 g12).
CHANGED: way b replaces the item's own text ("for the moment only: a show you open afterwards brings its outputs on again"). Against the old blocks: R191 g9 said that opening a show after All Outputs Off brings that show's outputs on again (old assumption G-15); that no longer holds, and R191 g3, g5, g6, g9, the sentence kept from the old part g, and item 200 are amended. His "b" says only what the page's way b says: "After All Outputs Off nothing comes on until you switch an output on yourself." INFERRED, Harmony's reading and not his word: (1) the hold reaches over a quit and a new start, because the sentence he chose names no end but his own switch-on (G3-2, asked; the other way, a quit ends it, leans on his earlier BF216); (2) the first output he switches on ends the hold for every output, the letter of "an output" (G3-1, a line he can strike; the other way holds each output until he switches that very one on); (3) the act that ends it switches on only what it names; (4) only the command starts a hold, also when no output was on, and switching the outputs off one by one starts none (G3-5, not asked); (5) Cmd+F counts as switching an output on himself. That a plug alone brings nothing back after All Outputs Off was Harmony's in the old spec; his "nothing comes on" now covers it. NOT a reading of this round: that Restore Last Outputs brings the outputs back after All Outputs Off, also after a quit, is reading R191 part i of the first page (.harmony/.reports/s-rta-1005/boris-clarify-all.md:558), which he named (BF216) and left as it was; way b fits it, because he presses the command himself.
TODAY: All Outputs Off closes every output window and cancels the pending reopen of unplugged screens (area-outputs.md part 1 item 11); Syphon is not touched by it (item 15, H5); the screens opened in the session leave the saved set, so Restore Last Outputs then has nothing to open (CORRECTIONS W1, M1: INFERRED from code there, not run). No show opens an output and nothing comes on at the start of the app, so no hold exists. To build: the hold as one flag in the computer's settings (settings.json), written when All Outputs Off is pressed and cleared by any switch-on of his, read by the start of the app, the show-open path and the plug-in path before they switch anything on; Restore Last Outputs keeping its set through All Outputs Off and through a quit (R191 part i); Syphon inside All Outputs Off. Read in the papers named, not in the program text; nothing run.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 199
TITLE: Where a screen's Delay is remembered
STATUS: DEFAULT option A, by his first line
HIS: L1; reading R188 part d, which he named at L94 and left as it was
RULE: A screen's eight settings (Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue) belong to that screen on this computer. When the Mac recognises a projector that was set before, it comes up with the numbers it had last: 40 ms set for one room is 40 ms again at the next venue, and he changes it there for the new room. The settings are not saved in a show and there is no list of rooms; opening or saving any show leaves them as they are. A screen's settings can be changed only while that screen is plugged in. Syphon's settings are kept the same way, on this computer. The show remembers only WHICH outputs are on (R191 g, L96), never their settings.
CHANGED: nothing. Tested against his whole message and against his earlier words. (1) Reading R188 part d (topic F) told him "The computer holds, for every show: your presets; each output screen's settings"; he named R188 at L94, corrected other parts of it and left this one: the same as option A. (2) L96 names which outputs are connected, not their settings. (3) His earlier "every show remembers it's sync" (binding-decisions.md:696) leans to option C, but it was said of the sync dial, which he then replaced ("replace our sync with this", :795); where a screen's Delay is kept was put to him twice after that (question 38, default at :839; question 199, whose option C read "The Delay is saved with the show, like the nudge amount") and he took the default both times (L1). (4) L88 "screens can be modified to fit a projectors color and timing issues" is about what a recording holds: a weak lean only (INFERRED); it decides nothing here.
TODAY: no setting of any kind exists per output and there is no settings window (area-outputs.md part 1 item 7). The ruled store is a new settings.json key "outputSettings", on paper only (part 2 (h)). To build: the eight settings, the store, the recognition of a screen (part 4 T3).
@@END

@@ITEM 200
TITLE: Syphon as an output
STATUS: DEFAULT option A, by his first line; its sentence on the launch follows his L96 (by his L9) -- AMENDED after page 2 (200 1 by G)
HIS: L1, L96, L9 (L94 for the start of the app)
RULE: Syphon is one more line in the Outputs list, with its own tick. It goes off with All Outputs Off and with Cmd+Shift+Esc. Other programs see "Audio-DNA" as a Syphon source only while the line is ticked, never from the mere start of the app. "Restore Last Outputs" brings it back together with the screens. It has the same eight settings as a screen. Syphon is an output (his earlier word), so the show remembers it as it remembers a screen (R191 g): it comes on by itself when a show that was saved with Syphon on is opened, also at the start of the app, and a show that does not remember Syphon leaves Syphon as it is, on or off (items 263 and 237, accepted as written). While All Outputs Off holds (R191 g9, his way b of item 265) nothing switches Syphon on by itself, not an opened show and not a start of the app: Syphon comes on again by a tick on its own line or by "Restore Last Outputs" [page 2, G: BF269 "b"; items 263 and 237 accepted as written]. No exception like the main display's is needed, because Syphon covers nothing. Harmony's layout, not a word of his (L8): the number on the Outputs button counts every ticked line, Syphon included, and Syphon's Device row reads "Syphon".
CHANGED: option A's sentence "It is off at every launch" is replaced: off unless the show that opens was saved with Syphon on. This is not his word against his word: that sentence repeated, for Syphon, the rule the page then had for screens (R191 g, "The app never opens an output by itself"); he corrected that rule where it stood (L96), and his L9 says an explanation given once answers the repeats; his earlier "syphon is an output and treated with same output settings as a screen" (binding-decisions.md:834) makes Syphon one of the outputs. That L96 covers Syphon stays INFERRED (he wrote of outputs "connected"), so it is shown to him as a line (G-2). The rest of option A stands. Taken out of the rule as his: "It counts in the number the Outputs button shows" (not in option A; now marked as layout).
TODAY: "Syphon Output" is a tick item of the Output menu outside the per-display list, not counted by "Outputs: N", not switched off by All Outputs Off, off at every launch and not remembered (area-outputs.md part 1 item 15, H5); the Syphon server is created at start whether ticked or not, so other programs list "Audio-DNA" from launch (item 17, H6: INFERRED there, not run). All of that changes.
@@END

@@ITEM R191
TITLE: Output screens: what stands, and what a show remembers
STATUS: CORRECTED part g replaced by his L96; part e reworded after L35 and L88; a to d, f, h and i stand -- AMENDED after page 2 (R191 1 by G, R191 2 by G, R191 3 by G, R191 4 by G, R191 5 by G, R191 6 by G, R191 7 by G)
HIS: L96 (with L94 for the start of the app; L35 and L88 for e; L8 for the last sentence)
RULE: (a) Each output screen has eight settings: Device, Delay (0 to 100 ms), Opacity, Brightness, Contrast, Red, Green, Blue; Syphon has the same. Where each one starts and how far it runs is as in his Resolume's Screen window (his picture; the numbers are in Harmony's notes). (b) Device names the screen and is not a menu; screens are switched on and off in the Outputs list. (c) These settings cannot be put on a key, a knob or a pad, are not recorded and are not Undo steps. (d) A screen's Opacity fades that screen to black; Syphon's Opacity fades the Syphon picture to black too; the master opacity fades the whole show; both stay. (e) No monitor inside the app carries a screen's Delay or colour: the output monitor shows the composition as it is at that moment, and the preview monitor shows the cue or the previewed clip (topic B), also without any screen's Delay or colour. A screen's Delay and colour are judged on that screen, not on the laptop. Recordings are the plain composition too, never one screen's picture (L88). (f) A screen shows the whole composition, fitted with bars, never stretched; no screen shows only a part of it. (g) NEW, his L96: a show remembers its outputs. (g1) A show remembers an output from the moment that output is on while the show is open: one he switches on, one that Restore Last Outputs brings back, and one that was already on when the show was opened or made; screens and Syphon alike (Syphon: G-2). What the show remembers is written into the show file at each Save (his L96: "if I save a show with the outputs connected"). It holds only which outputs, never a screen's settings (199). (g2) The show forgets an output only when he switches it off by its own line in the Outputs list (G-6). Unplugging the screen, All Outputs Off, Cmd+Shift+Esc, and a Save made while the screen is unplugged or off do not make the show forget it. A screen that is not plugged in has no line, so it stays remembered until it is there and he switches it off. (g3) When the show is opened, every remembered output that is plugged in switches on by itself, without a click and without a message, except while All Outputs Off holds (g9): then opening a show switches nothing on. [page 2, G: BF269 "b"] Taking a deck out of another show is not opening that show and touches no output. (g4) Opening a show only adds outputs: it never switches off an output that is on, and a show that remembers none leaves the outputs as they are (G-1). (g5) The same happens at the start of the app, because the app opens to the very last show (L94; G-3), except while All Outputs Off holds (g9): the hold is kept on the computer over a quit, so a start of the app after All Outputs Off switches nothing on, and he brings his outputs back himself with one press on "Restore Last Outputs" or with a tick (assumption G3-2; the other reading ends the hold at a quit, so that the start brings the last show's outputs on). [page 2, G: BF269 "b"] (g6) A remembered screen that is not plugged in is passed over without a message (G-5); it switches on by itself at the moment it is plugged in while that show is open (G-14), unless All Outputs Off holds (g9) [page 2, G: BF269 "b"]. (g7) The output on the main display, which would cover the app's own window, is the one exception: it is not taken by a show and never comes on by itself (G-4). (g8) A show brings back only the very screens it remembers, recognised as the Mac reports them (D28); with another projector nothing comes on by itself (item 264, accepted as written); on another computer no screen comes on by itself either, also not the same projector, which is Harmony's own and was not in the item he read (assumption G3-4) [page 2, G: item 264 accepted as written]. (g9) His way b of item 265: All Outputs Off and Cmd+Shift+Esc switch every output off, Syphon included, and then hold: the app switches on no output by itself, not when a show is opened (another show or the same one again), not when a screen is plugged in, and not at a start of the app, because the hold is kept on the computer over a quit (assumption G3-2), until he switches an output on himself. He does that with a tick on an output's own line, with Cmd+F for the main display's output, or with "Restore Last Outputs", which brings back the outputs that were on before All Outputs Off (part i). The first output he switches on ends the hold for every output (assumption G3-1), and that act switches on only what it names. After it a show that is opened brings its outputs on again (g3) and a remembered screen that is plugged in comes on again (g6). The hold makes no show forget an output (g2) and is never a part of a show file. Only the command starts a hold, also when no output was on; switching every output off by its own line starts none (assumption G3-5). [page 2, G: BF269 "b"] (g10) Switching an output on or off does not by itself mark the show as changed (G-16). (g11) Opening a recording in Studio [page 2, G: BF245 "Let's go with studio. That's perfect."; BF263 "b"], which opens the show file saved with that recording, never switches an output on or off (G-17). (g12) A start of the app made by a test or a tool never switches an output on (G-9). Kept from the old part g: an output that was on and whose projector is unplugged comes back by itself when the projector is plugged in again, except while All Outputs Off holds (g9) [page 2, G: BF269 "b"]; plain Esc never closes an output. The app's standing rule, not a word of his and not in the reading shown to him: an output never takes the keyboard. (h) The Delay's number can also be typed and has plus and minus for one millisecond at a time. (i) "Restore Last Outputs" stays as the computer's own memory, whatever show is open: it switches on the outputs that were last on on this computer, also after All Outputs Off or Cmd+Shift+Esc, and also after a quit and a new start; a screen he switched off by its own line is forgotten there too. Where a screen's settings are opened is laid out by Harmony (L8, P27).
CHANGED: part g: the page's first sentence there, "The app never opens an output by itself" with one click on Restore Last Outputs as the only way back, is REPLACED by L96 (a show remembers its outputs and brings them on). The rest of g (replug, plain Esc) stands. Part e: reworded only. The page's "The monitors inside the app always show the picture as it is now" was written before the preview monitor showed a cue or a previewed clip (L35); what stands is that no monitor carries a screen's Delay or colour; for recordings his L88 now says so himself. Last sentence "Where a screen's settings are opened comes as a picture": REPLACED by L8. Parts a, b, c, d, f, h, i: nothing (he named R191 and changed only this); the pointer to his Resolume picture in part a adds no rule. Everything in g beyond "a show remembers the outputs it was saved with and brings them on when it is opened" is Harmony's reading and is carried by G-1, G-3, G-4, G-5, G-6, G-9, G-14, G-15, G-16 and G-17. "connected" in L96 is read as switched on at the save and plugged in at the opening (INFERRED). "An output never takes the keyboard" came from the project's own rules (Pitfall 40), not from the reading: marked so.
TODAY: outputs are machine state, never show state: a show never opens or remembers an output; the wanted set is in settings.json "outputs"; only Output > Restore Last Outputs opens the saved set (area-outputs.md part 1 items 10, 12; CLAUDE.md "Outputs"; docs/claude/integration.md:27, 29 as cited there). After All Outputs Off the screens opened this session leave the file, so Restore has nothing to open (CORRECTIONS W1, M1: INFERRED from code, not run). All Outputs Off cancels pending replug reopens (part 1 item 11). No Delay, no settings (items 6, 7). A screen is known only by six numbers of place and size (item 7, T3). The builder's numbers for part a: his picture shows Delay 0 ms, Opacity 100 %, and Brightness, Contrast, Red, Green, Blue at 0 with each slider at its middle (boris-feedback-backlog.md:437, Harmony's description of the picture); the adopted ruling took Opacity 0 to 100 % and -1.00 to 1.00 for the five (area-outputs.md part 2 (h)); not re-checked against his Resolume. To change: a new field in the show file for the outputs a show remembers, with the forget-only-by-its-own-line test (the file's shape and version change: Pitfall 68; no old show files are carried over, his L69); bringing the set on at load and at launch; one recognition of a screen for the settings and for the show's set (T3); a guard for test launches (binding-decisions.md:137-147 screen-safety law); CLAUDE.md "Outputs", Pitfall 40 text and integration.md rewritten.
@@END

@@ITEM R209
TITLE: Outputs keep going whatever the main window shows
STATUS: STANDS -- AMENDED after page 2 (R209 1 by G)
HIS: none (not named; L35 leans the same way; L91 for what the outputs show while Review is open)
RULE: The outputs, Syphon and a recording that is running keep going, always. Nothing done in the main window can freeze or stop the audience's picture: opening the big signal bar, changing to another arrangement of the window, hiding a monitor, minimising the window, hiding the app, or bringing another program to the front. The picture is made whether or not any monitor inside the app is shown (G-10). While Studio is open the outputs are not frozen either: they show the Studio picture [page 2, G: BF245 "Let's go with studio. That's perfect."; BF263 "b"] (186 b, L91; topic E rules it).
CHANGED: nothing. Tested against his whole message: L35 "We will have various configurations other than live and recording review mode" means the main window will often not show the monitor, which makes this rule necessary; no word of his asks for a frozen picture. Added for the builder: the pointer to 186 b (L91), and "another program in front" as one more case of "always" (INFERRED).
TODAY: the monitor panel's GL context hosts the render loop; when the panel is hidden (big signal bar open) or the app is minimised, every output holds its last frame and Syphon and the recorder stop getting frames (area-outputs.md part 1 item 13, H7, T1, T6: seen in code, not run). To change: the render loop must live apart from the panel. What hiding the app does to an output window: not checked.
@@END

@@ITEM R226
TITLE: Picture size, the Transform and the output screen
STATUS: STANDS only names change: the tab (210 default A by L1; L55) and "output screen" for "output window" (L35)
HIS: none for the reading itself (not named); L1 (question 210 default A) and L55 for the tab's name; L35 for "output window"
RULE: (a) The picture size is chosen in the Resolution list in the Global tab: 1920x1080, 1280x720, 2560x1440, 3840x2160, a portrait size, a square size and a 4:3 size; a size cannot be typed. (b) The Transform section of the same tab (Position X, Position Y, Scale, Rotation, Anchor) moves, scales and turns the whole picture after every effect and before the master opacity, so the output monitor, every screen, Syphon, recordings and snapshots show it; what the picture no longer covers is black. Whether the preview monitor shows it is topic B's (master cue, L38). (c) The Anchor has two sliders, sideways and up-down, as Position has. (d) An output screen always fills one whole display, without a border; an output is never a window on the desktop that can be moved or sized. (e) Keys: Cmd+Shift+Esc all outputs off, Cmd+F the main display's output, Cmd+` brings the app over an output.
CHANGED: names only. The page's "Composition tab" reads "Global tab": question 210's default A ("Global" for the tab), taken by L1, and his L55 "composition and global are interchangeable but lets move to global". The page's "output window" (title and part d) reads "output screen", because his L35 "the preview window under the output window" uses "output window" for the output monitor inside the app. In part b the page's "the monitor" is named as the output monitor; the page was written before the preview monitor had a picture of its own (INFERRED). Parts a to e: nothing else.
TODAY: Resolution drop-down in the inspector's Composition tab, section "Output Settings" (area-outputs.md part 1 item 25); five Transform controls, the one Anchor slider writes X only and forces Y to 0 (items 27-29, CORRECTIONS W2); outputs borderless, one per display (item 4); keys (item 8). To change: the Anchor's up-down slider; the tab's name; the section name "Output Settings" collides with his phrase for a screen's eight settings (see NAME output settings).
@@END

@@ITEM D25
TITLE: Rotating and scaling the whole picture
STATUS: OPEN still Harmony's own; a mend with no other way to mean it, kept in Harmony's notes and not put to him (G-7)
HIS: none
RULE: Rotating the whole composition turns the picture rigidly, without stretching or shearing it, on a picture of any shape. The lowest Scale gives a very small picture; it never gives a huge one.
CHANGED: nothing; no word of his touches it.
TODAY: the transform rotates in unit space with no aspect term, so a wide picture shears; Scale at 0 is a 1000 times zoom (area-outputs.md CORRECTIONS M3: read in EmbeddedShaders.h:136-160 there, not run).
@@END

@@ITEM D26
TITLE: Changing the picture size while recording or showing
STATUS: OPEN still Harmony's own; shown to him as a line (G-8)
HIS: none (L114 for the names of the two recordings)
RULE: The Resolution cannot be changed while a recording of either kind runs: record show or record to clip (his names, L114). Opening a show while a recording runs asks first, in the one window that says the recording will be stopped and kept, whatever that show's picture size (reading R187 b, topic E). With a screen or Syphon on and nothing recording, a new picture size is allowed at any time, by his hand in the Resolution list or with a show of another size that he opens; the screens may flash for a moment at the change (how long is measured at the build).
CHANGED: nothing; no word of his touches it. L88 is about when a recording starts and ends and what it holds, not about the picture size. The page's half about opening a show of another size is a repeat of reading R187 b, which stands in topic E for every show. "about half a second" is taken out of the rule: nobody has measured it for the new build.
TODAY: no guard exists; the Resolution list can be changed at any time (area-outputs.md CORRECTIONS M2; slice-G D26 cites src/ui/CompositionInspector.cpp:181-189); the flash is docs/claude/rendering.md:162 as cited there (a MilkDrop restart: black or a bright noise flash, normal again by about 0.6 s; MilkDrop itself is not worked on in the upcoming build, his L126, so that stays). What a running recording does on a size change: not checked.
@@END

@@ITEM D27
TITLE: The Delay moves in steps of one screen refresh
STATUS: OPEN still Harmony's own; a fact of the screen, not a choice (G-11)
HIS: none
RULE: A screen's Delay is set in whole milliseconds. A screen takes a new picture only once per refresh (about 17 ms on a 60 Hz projector, about 8 ms on a 120 Hz one), so a change of 1 ms can show no difference, and two screens with the same Delay can sit one refresh apart.
CHANGED: nothing; no word of his touches it.
TODAY: no Delay exists (area-outputs.md part 1 item 6); the real step on his Mac is not measured: the quiet run with Arena closed is still owed (T2, M6).
@@END

@@ITEM D28
TITLE: Which screen a set of settings belongs to
STATUS: OPEN in part: its first sentence is settled by 199's default (L1); the rest is Harmony's own (G-12 for the other projector and the twins, G-13 for hold and skip)
HIS: L1 (199 default A); reading R188 part d, named at L94 and left as it was
RULE: A screen's settings are tied to the screen as the Mac reports it, never to its size or its place. A different projector on the same socket comes up with default settings. Two projectors of the same model that the Mac cannot tell apart share one set of settings, and the settings place says so. Raising a screen's Delay holds its picture still for the added time; lowering it skips ahead; the screen never goes black for it. The same recognition of a screen is used for the outputs a show remembers (R191 g8, G-5).
CHANGED: nothing replaced. Settled by his words: "tied to the screen" (199 A, L1). Still Harmony's: the different projector and the twins (G-12); hold and skip (G-13, which the paper had left without an assumption).
TODAY: an output is known only by six numbers (place, size, scale, main or not); the framework gives no display id or name; the ruled UUID key is on paper (area-outputs.md part 1 item 7, T3). What his own screens report is measured after the build.
@@END

@@ITEM P27
TITLE: Where a screen's eight settings are opened
STATUS: DROPPED its two variants differ only in where the thing sits
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8)
CHANGED: the picture is not drawn. Harmony's layout until the redesign: a window "Output Screens", opened from the Output menu, listing every screen and Syphon with its eight settings; the Outputs list shows a screen's Delay beside its line.
TODAY: no settings window exists (area-outputs.md part 1 item 7); the adopted ruling's window "Output Screens" is on paper (part 2 (h), H17).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME G3-1
ABOUT: 265; R191 (g9)
TEXT: I assume All Outputs Off is over once you switch one output on yourself: only that one comes on, and from then on a show you open brings its outputs on again.
WHY: BF269 "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself.") can be read two ways: the first switch-on ends it for every output, which is the letter of "an output", or each output stays off until he switches that very one on. Not asked any more: that Restore Last Outputs still works after All Outputs Off is reading R191 part i of the first page, which he named (BF216) and left as it was.
ALT: b) Each output stays off until you switch that very output on yourself, whatever show you open.
IF-WRONG: STAGE he cuts two projectors, switches one on again and opens the next show: the other lights up by itself, or, built the other way, stays dark until he ticks it; a small change to undo
ASK: LINE the text follows the letter of the way he chose; the other way is a stricter guard he may want, and he can strike the line
@@END

@@ASSUME G3-2
ABOUT: 265; R191 (g5, g9, i)
TEXT: I assume All Outputs Off also lasts over a quit: if you quit after it, the next start of the app opens your last show with no output on, and one press on Restore Last Outputs brings your outputs back.
WHY: BF269 "b" gives All Outputs Off no end but his own switch-on, and a quit is not that: his answer, read to the letter, is the rule. Against it stands his earlier BF216, "I expect the show to remember the outputs connected and not need to connect them again.", with BF215, "When you open the application, it opens to the very last show." Which of the two he means at the start of the app is not said.
ALT: b) It ends when you quit: the next start of the app brings your last show's outputs on by themselves.
IF-WRONG: STAGE built as written and meant the other way, his outputs stay off at every start that follows a night ended with All Outputs Off, until he presses Restore Last Outputs; built the other way and meant as written, a projector he had cut lights up by itself at the start of the app
ASK: YES his newest word and his earlier one pull apart exactly here (BF269 against BF216) and no word of his names the quit; he sees it at set-up before a show; it is about what the app does
@@END

@@ASSUME G3-3
ABOUT: 264; R191 (g6)
TEXT: I assume no message is shown when a screen that a show remembers is not plugged in: the show simply opens without it.
WHY: Item 264, which he accepted, says nothing comes on by itself; it does not say whether the app tells him a remembered screen is missing.
ALT: b) A short note names the remembered screen that was not found.
IF-WRONG: SMALL a note can be added later; he sees at once that the projector is dark
ASK: NO how a missing screen is told is a matter of look, left to the UI redesign
@@END

@@ASSUME G3-4
ABOUT: 264; R191 (g8); D28
TEXT: I assume a show opened on another computer switches no screen on by itself, also not the same projector: you tick it once there.
WHY: Item 264, which he accepted, names only a different projector; "on another computer" came from old R191 g8 and old assumption G-5 and was not in the item he read. His BF216 speaks of opening the show back up with the outputs connected and names no computer. Whether a second Mac reports the same projector as the same screen is not known (D28: measured after the build). Syphon is no screen: item 263 as he accepted it names no computer, so Syphon comes on with its show on any computer.
ALT: b) On another computer the same projector comes on by itself too, when that computer reports it as the very screen the show remembers.
IF-WRONG: SMALL one tick at set-up on the second computer
ASK: NO rare, and it hangs on what a second Mac reports, which is measured after the build; the careful choice until then
@@END

@@ASSUME G3-5
ABOUT: 265; R191 (g9)
TEXT: I assume only the command All Outputs Off, or its key, makes the outputs stay off in this way, also when no output was on at the press. Switching every output off one by one, each by its own line, does not.
WHY: BF269 "b" names All Outputs Off and says nothing of its edges: a press while nothing is on, and all outputs switched off singly. An output switched off by its own line is forgotten by the show anyway (R191 g2), so no show brings it back.
ALT: b) Switching the last output off by its own line counts as All Outputs Off too. c) A press while no output is on changes nothing.
IF-WRONG: SMALL after a stray press he ticks an output or presses Restore Last Outputs once
ASK: NO two edges of his own answer with one sensible reading each; nothing for him to choose
@@END

@@ASSUME G-4
ABOUT: R191, R226
TEXT: I assume the output on your main display, the one the app itself is on, never comes on by itself, because it would cover the app. You switch it on with Cmd+F.
WHY: L96 speaks of outputs that are connected; the output on the main display is not a connected screen, and nothing of his names it.
ALT: b) It comes back like any other output when the show was saved with it on.
IF-WRONG: SMALL the app would open hidden behind its own picture, or he presses one key
ASK: LINE a choice of Harmony's that departs from the letter of L96 for one output; he would most likely wave it through
@@END

@@ASSUME G-6
ABOUT: R191
TEXT: I assume a show keeps remembering a screen that is unplugged when you save: after packing up or saving at home, it still knows your projector. It forgets a screen only when you switch it off by its own line.
WHY: L96 says "if I save a show with the outputs connected"; what a later save without the projector plugged in does is not said.
ALT: b) The show holds exactly the outputs that are on at the moment of each save: a save without the projector plugged in forgets it, and you tick it again at the venue.
IF-WRONG: SMALL he ticks the projector again at the venue, which is what L96 asks to be rid of
ASK: LINE a real choice of Harmony's beyond the letter of L96; cheap to change
@@END

@@ASSUME G-7
ABOUT: D25
TEXT: I assume rotating the whole picture turns it without stretching it, and the lowest Scale gives a very small picture.
WHY: Decided without asking; he did not read that list (L130) and no word of his touches the Transform.
ALT: none
IF-WRONG: SMALL
ASK: NO a mend with no other way to mean it; there is nothing for him to choose
@@END

@@ASSUME G-8
ABOUT: D26
TEXT: I assume the picture size cannot be changed while record show or record to clip is running. With screens on and nothing recording it can be changed, and the screens may flash for a moment at the change.
WHY: Decided without asking; he did not read that list (L130) and no word of his touches the picture size.
ALT: b) With a screen on, the app asks before it changes the picture size. c) The picture size is locked while any output is on.
IF-WRONG: SMALL the flash comes only when the picture size changes during a show, by his own hand or with a show he opens (its layers are empty then); a guard is a small change later
ASK: LINE he chooses the moment himself and would most likely wave it through; he can strike it
@@END

@@ASSUME G-9
ABOUT: R191
TEXT: I assume a start of the app made by a test or a tool never switches an output on, whatever the show holds. Only a start made by you does.
WHY: L96 makes outputs open by themselves; the standing screen-safety law (binding-decisions.md:137-147) forbids unattended output windows on his monitors.
ALT: none
IF-WRONG: SMALL
ASK: NO internal: how Harmony's own test runs behave
@@END

@@ASSUME G-10
ABOUT: R209
TEXT: I assume the picture is made apart from the monitors inside the app, so hiding or closing a monitor cannot stop an output, Syphon or a recording.
WHY: The reading promises "always"; how the build keeps that promise is not his matter (L35 leans the same way).
ALT: none
IF-WRONG: REBUILD it is the base the cue monitor and the Review screen stand on
ASK: NO technical: how it is built
@@END

@@ASSUME G-11
ABOUT: D27
TEXT: I assume a screen's Delay moves in steps of one screen refresh (about 17 ms at 60 Hz), although it is set in whole milliseconds.
WHY: A fact of how a screen works, told earlier as a reading; the exact step on his Mac is not measured yet.
ALT: none
IF-WRONG: SMALL
ASK: NO a fact, not a choice; goes into the manual
@@END

@@ASSUME G-12
ABOUT: D28, 199
TEXT: I assume two projectors of the same model that the Mac cannot tell apart share one set of settings, and a different projector on the same socket starts with default settings.
WHY: 199's default ties settings to the screen (L1); these two edge cases were decided without asking (L130).
ALT: b) Settings follow the socket, not the projector.
IF-WRONG: SMALL he sets a Delay once more
ASK: NO rare edge; what his own screens report is measured after the build
@@END

@@ASSUME G-13
ABOUT: D28
TEXT: I assume that when you raise a screen's Delay its picture holds still for the added time, and when you lower it the picture skips ahead. The screen never goes black for it.
WHY: Decided without asking; he did not read that list (L130), and no word of his says what a screen shows at the moment its Delay is changed.
ALT: b) The new Delay is taken only when you let go of the control. c) The screen goes black for a moment.
IF-WRONG: SMALL a hitch of at most a tenth of a second on that one screen, only at the moment he moves its Delay
ASK: LINE a choice of Harmony's that he would most likely wave through
@@END

@@ASSUME G-16
ABOUT: R191
TEXT: I assume switching an output on or off does not by itself make the app treat the show as changed: the show takes its outputs when you save it.
WHY: L96 names the save as the moment; whether an output switched on counts as an unsaved change is not said.
ALT: b) Every switch of an output counts as a change to the show.
IF-WRONG: SMALL
ASK: NO internal bookkeeping; the app asks at every quit and at every opening of another show anyway (apply-F.md, R189 a and d)
@@END

@@ASSUME G-17
ABOUT: R191 (g11)
TEXT: I assume opening a recording in Studio never switches an output on or off by itself, although the show file saved with that recording remembers the outputs of that night.
WHY: Unchanged in what it says; only the screen's name is new (BF245, BF263). His L96 lets an opened show switch outputs on; whether a recording's own show file counts is not said.
ALT: b) Opening a recording in Studio brings on the outputs that were on that night.
IF-WRONG: SMALL he ticks one output to watch a recording on the big screen
ASK: NO the careful choice at no cost; topic E owns Studio and what the outputs show while it is open
@@END

## CLOSED ASSUMPTIONS (one line each)
- G-1 -> page 2, item 237
- G-2 -> page 2, item 263
- G-3 -> SETTLED
- G-5 -> page 2, item 264
- G-14 -> SETTLED
- G-15 -> page 2, item 265

## QUESTIONS BACK
## NAMES
@@NAME Outputs
MEANS: The one list of everything the picture can be sent to: every connected screen and Syphon, each with a tick.
SOURCE: the on-screen name now (Output menu and the "Outputs" button), kept; that Syphon is a line of it: question 200 default A (L1)
@@END

@@NAME output
MEANS: One thing the picture is sent to: a screen or Syphon.
SOURCE: his words L96 "the outputs connected"; binding-decisions.md:834 "syphon is an output and treated with same output settings as a screen"
@@END

@@NAME Delay
MEANS: One of a screen's eight settings: how many milliseconds later (0 to 100) that screen shows the picture.
SOURCE: binding-decisions.md:792 "each output screen can be delayed and that is set on output display properties"
@@END

@@NAME Output Screens
MEANS: The place where each output's eight settings are opened.
SOURCE: Harmony's pick (the adopted ruling's window name; "Output Settings" already names the picture size); his own phrase is "output display properties" (binding-decisions.md:792); its place and look wait for the UI redesign (L8)
@@END

@@NAME Global tab
MEANS: The tab that holds the picture size (Resolution) and the Transform of the whole picture.
SOURCE: question 210 default A ("Global" for the tab), taken by his first line (L1); his words L55 "composition and global are interchangeable but lets move to global"; the on-screen name now is "Composition", which is replaced by Global
@@END

@@NAME output screen
MEANS: A connected display (a projector, a monitor, an LED wall) that shows the whole picture full-screen; one kind of output, Syphon being the other.
SOURCE: his words binding-decisions.md:792 "each output screen can be delayed"; used instead of "output window", which in his L35 ("the preview window under the output window") means the output monitor inside the app
@@END

@@NAME output settings
MEANS: The eight settings of one output: Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue.
SOURCE: his words binding-decisions.md:832 "yes add all of those output settings" and :834; the on-screen name now "Output Settings" is a section of the Global tab that holds the Resolution, which collides with it: that section is to be renamed in the list of names (L114)
@@END

@@NAME All Outputs Off
MEANS: The command, also Cmd+Shift+Esc, that switches off every output, Syphon included, and after which nothing comes on by itself until he switches an output on himself.
SOURCE: the on-screen name, kept; what follows it: his way b of item 265 (BF269); refines the row "All Outputs Off" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

@@NAME Restore Last Outputs
MEANS: The command that switches on again the outputs that were last on on this computer, whatever show is open, also after All Outputs Off and after a quit and a new start.
SOURCE: the on-screen name, kept; "also after All Outputs Off" and "after a quit" are reading R191 part i of the first page (.harmony/.reports/s-rta-1005/boris-clarify-all.md:558), which he named (BF216) and left as it was; his way b of item 265 (BF269) fits it, because he presses the command himself
@@END

## REACHES OTHER TOPICS (from the paper)
- F (R188 a, the show file; R189, opening a show): opening a show after All Outputs Off switches no output on (BF269); the show's remembered outputs are loaded and kept all the same. The hold is not a field of the show file.
- F (old assumption F-12, the twin of G-3: outputs at the start of the app): stands, with one new edge that this paper asks once for both topics (G3-2: a quit ends the hold).
- X (old assumption X-7, the twin of G-2; old item C19): closed by item 263, accepted as written; the hold of 265 reaches Syphon too (BF269).
- J (item 271, accepted as written: switching an output screen on or off cannot go on a key or a pad): it fits 265 b. "Switch an output on yourself" therefore means the Outputs list, Cmd+F and Restore Last Outputs, never a pad. Whether All Outputs Off and Restore Last Outputs can go on a pad is J's (old J-18 named both; item 271 as he read it names only the on and off of an output screen).
- E (item 186 b, the outputs while Studio is open; old E-1): closing or opening Studio during a hold must not switch an output on; G-17 says so for opening a recording. E rules what an output that is on shows.
- E (item 231, BF253 "It defaults to running next to the show recording"): applied here without an amendment: the low-resolution show recording runs beside record show, so D26 (no change of picture size while a recording runs) locks no longer than record show does. The worry in the old ruling notes (a lock all night) falls away.
- F (item 236 and the answer on Snapshot, BF243 "with all the settings and the output and everything", BF257): if a Snapshot is a save of the show, F must say whether opening one counts as opening a show for the outputs (then 237 and 265 apply as written) and whether it still holds a still picture; R226 b's word "snapshots" (the Transform shows in them) waits on that and is not amended here.
- B (item 252, BF267, the monitors at one frame rate): not applied here; R209 (the outputs keep going whatever the main window shows) is untouched by it.

## CONFLICTS (from the paper)
- Not his word against his word, but a narrowing he should hear once in one line. New, BF269 on item 265: "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself."). Earlier, binding-decisions.md:1175 (2026-10-07, BF216): "R191 if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." Squared: the show still remembers and brings its outputs on; only after All Outputs Off does he switch one on himself. The edge between the two is the quit (G3-2).
- His way b against the project's standing rule, which is no word of his: CLAUDE.md "Outputs" says "the app never opens an output by itself (only Output > Restore Last Outputs does". A grep for "never opens" in binding-decisions.md finds nothing, so no quoted word of his says it (the old spec found the same). His L96 replaced that rule on 2026-10-07; BF269 brings it back for one case only, the time after All Outputs Off. CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md are rewritten when it is built.

## NOT DONE / UNSURE (from the paper)
- Where the hold ends (G3-2) is the only thing here that needs his answer. Cheapest: one question, "After All Outputs Off and a quit, does the next start bring your show's outputs on?"
- A corner the hold makes easier to reach, left with old assumption G-6 (not shown to him, unchanged): a show forgets an output only when he switches it off by its own line; during a hold that output is already off, so to make the show forget it he must tick it on and off again. Cheapest: leave it; if it bothers him after the build, a "forget this screen" entry is a small addition.
- Whether All Outputs Off pressed when no output is on also starts a hold: written as yes (the command always holds); not asked, because the only effect is that a show opened next adds nothing until he ticks an output. Cheapest: fold into the question on G3-2 if he asks.
- TODAY lines are taken from spec-G.md and the fact sheet area-outputs.md; I opened no source file for this paper. Nothing was run.
- Old assumptions G-4, G-6, G-7, G-8, G-9, G-10, G-11, G-12, G-13 and G-16 were tested against the 33 boxes and the four items: untouched, nothing written. G-17 keeps its sense and gets the name Studio. No old assumption of the internal list is dropped.

## FOR THE PAGE RULING (from the ruling)
- MUST be put to him, the one question of this topic (ASK YES): G3-2. As ruled: All Outputs Off also lasts over a quit (the letter of his "b", BF269); its way b: it ends at the quit and the next start brings the last show's outputs on (his earlier BF216 leans there). It makes the one exception to what old assumptions G-3 and F-12 settled (the last show brings its outputs on at the start of the app): no other topic asks it.
- One line he can strike (ASK LINE), best right after G3-2 because both come from his "b" on item 265: G3-1. All Outputs Off is over once he switches one output on himself, for every output; its way b: each output stays off until he switches that very one on.
- Not shown (ASK NO): G3-3 (no message for a missing screen), G3-4 (another computer), G3-5 (what starts it), G-17 (opening a recording in Studio). The old internal ones (G-4, G-6 to G-13, G-16) are untouched by his 33 boxes.
- Newest against earlier, to be said to him once in one line, best as the lead-in of G3-2 and not a second time: BF269 "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself.") against BF216 "I expect the show to remember the outputs connected and not need to connect them again." In plain words: a show still remembers its outputs and brings them on when you open it; only after All Outputs Off do you bring them back yourself, with one press on Restore Last Outputs.
- NOT open, do not ask: that Restore Last Outputs works after All Outputs Off, also after a quit. Reading R191 part i of the first page said it; he named R191 and corrected only part g (H1).
- A word to keep off his page: "hold" is this topic's internal word for what All Outputs Off does afterwards. To him say that All Outputs Off lasts, or is over: "hold" is the pad setting of item 246.
- The paper's SUMMARY, REACHES, CONFLICTS and NOT DONE sections are carried over as written. Read three things in them with this ruling: where they say that a quit ends the hold (SUMMARY; REACHES, the bullet on F-12), that is now way b of G3-2, and the rule is that the hold is kept over a quit; where they call Restore Last Outputs after All Outputs Off an assumption (SUMMARY; the paper's G3-1), it is settled; and CONFLICTS bullet 1, which opens "Not his word against his word", is at the start of the app a real pull between BF269 and BF216.
- Depends on J: J3-9 (whether All Outputs Off and Restore Last Outputs can go on a key or a pad; item 271 names only an output screen's on and off). Whatever J rules, a press of his own counts as "yourself": Restore Last Outputs from a pad would end All Outputs Off like the menu command. Nothing else here changes.
- Depends on F: opening a snapshot counts as opening a show for the outputs (apply-F.md, item 236 part 4), so items 237, 264 and 265 apply as written, All Outputs Off included. F's R188 (a) says "which outputs were on when it was saved"; the rule is which outputs the show remembers (R191 g1, g2: a Save with the projector unplugged does not forget it): F should point here, not restate.
- Depends on E: opening or closing Studio, and opening a recording in it (G-17), switch no output on or off, also while All Outputs Off lasts; what an output that is on shows there is E's (186 b). E's reading of BF268 ("the output" is the picture, never one output screen's corrected picture) agrees with R191 part e.
- Agrees with X: X3-1 and the amended C19 (Syphon follows every rule for outputs); where All Outputs Off ends is ruled here (G3-1, G3-2), not there.
- At the build: the hold is one flag in the computer's settings, never in the show file; CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md are rewritten (the paper's CONFLICTS bullet 2). What a second Mac reports for the same projector is measured with D28's test (G3-4).

