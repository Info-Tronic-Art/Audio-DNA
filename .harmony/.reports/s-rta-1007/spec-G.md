# SPEC G -- Output screens (s-rta-1007): the paper apply-G.md with the ruling rule-G.md laid over it by merge.py. This file wins over both.

## ITEMS
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
STATUS: DEFAULT option A, by his first line; its sentence on the launch follows his L96 (by his L9)
HIS: L1, L96, L9 (L94 for the start of the app)
RULE: Syphon is one more line in the Outputs list, with its own tick. It goes off with All Outputs Off and with Cmd+Shift+Esc. Other programs see "Audio-DNA" as a Syphon source only while the line is ticked, never from the mere start of the app. "Restore Last Outputs" brings it back together with the screens. It has the same eight settings as a screen. Syphon is an output (his earlier word), so the show remembers it as it remembers a screen (R191 g): it comes on by itself when a show that was saved with Syphon on is opened, also at the start of the app, and otherwise it stays off until he ticks it (assumption G-2; the other reading keeps Syphon off at every start and at every opening of a show until he ticks it). No exception like the main display's is needed, because Syphon covers nothing. Harmony's layout, not a word of his (L8): the number on the Outputs button counts every ticked line, Syphon included, and Syphon's Device row reads "Syphon".
CHANGED: option A's sentence "It is off at every launch" is replaced: off unless the show that opens was saved with Syphon on. This is not his word against his word: that sentence repeated, for Syphon, the rule the page then had for screens (R191 g, "The app never opens an output by itself"); he corrected that rule where it stood (L96), and his L9 says an explanation given once answers the repeats; his earlier "syphon is an output and treated with same output settings as a screen" (binding-decisions.md:834) makes Syphon one of the outputs. That L96 covers Syphon stays INFERRED (he wrote of outputs "connected"), so it is shown to him as a line (G-2). The rest of option A stands. Taken out of the rule as his: "It counts in the number the Outputs button shows" (not in option A; now marked as layout).
TODAY: "Syphon Output" is a tick item of the Output menu outside the per-display list, not counted by "Outputs: N", not switched off by All Outputs Off, off at every launch and not remembered (area-outputs.md part 1 item 15, H5); the Syphon server is created at start whether ticked or not, so other programs list "Audio-DNA" from launch (item 17, H6: INFERRED there, not run). All of that changes.
@@END

@@ITEM R191
TITLE: Output screens: what stands, and what a show remembers
STATUS: CORRECTED part g replaced by his L96; part e reworded after L35 and L88; a to d, f, h and i stand
HIS: L96 (with L94 for the start of the app; L35 and L88 for e; L8 for the last sentence)
RULE: (a) Each output screen has eight settings: Device, Delay (0 to 100 ms), Opacity, Brightness, Contrast, Red, Green, Blue; Syphon has the same. Where each one starts and how far it runs is as in his Resolume's Screen window (his picture; the numbers are in Harmony's notes). (b) Device names the screen and is not a menu; screens are switched on and off in the Outputs list. (c) These settings cannot be put on a key, a knob or a pad, are not recorded and are not Undo steps. (d) A screen's Opacity fades that screen to black; Syphon's Opacity fades the Syphon picture to black too; the master opacity fades the whole show; both stay. (e) No monitor inside the app carries a screen's Delay or colour: the output monitor shows the composition as it is at that moment, and the preview monitor shows the cue or the previewed clip (topic B), also without any screen's Delay or colour. A screen's Delay and colour are judged on that screen, not on the laptop. Recordings are the plain composition too, never one screen's picture (L88). (f) A screen shows the whole composition, fitted with bars, never stretched; no screen shows only a part of it. (g) NEW, his L96: a show remembers its outputs. (g1) A show remembers an output from the moment that output is on while the show is open: one he switches on, one that Restore Last Outputs brings back, and one that was already on when the show was opened or made; screens and Syphon alike (Syphon: G-2). What the show remembers is written into the show file at each Save (his L96: "if I save a show with the outputs connected"). It holds only which outputs, never a screen's settings (199). (g2) The show forgets an output only when he switches it off by its own line in the Outputs list (G-6). Unplugging the screen, All Outputs Off, Cmd+Shift+Esc, and a Save made while the screen is unplugged or off do not make the show forget it. A screen that is not plugged in has no line, so it stays remembered until it is there and he switches it off. (g3) When the show is opened, every remembered output that is plugged in switches on by itself, without a click and without a message. Taking a deck out of another show is not opening that show and touches no output. (g4) Opening a show only adds outputs: it never switches off an output that is on, and a show that remembers none leaves the outputs as they are (G-1). (g5) The same happens at the start of the app, because the app opens to the very last show (L94; G-3). (g6) A remembered screen that is not plugged in is passed over without a message (G-5); it switches on by itself at the moment it is plugged in while that show is open (G-14), unless All Outputs Off was used since (g9). (g7) The output on the main display, which would cover the app's own window, is the one exception: it is not taken by a show and never comes on by itself (G-4). (g8) A show brings back only the very screens it remembers, recognised as the Mac reports them (D28); on another computer or with another projector nothing comes on by itself (G-5). (g9) All Outputs Off and Cmd+Shift+Esc switch every output off for now: after them a plug alone brings nothing back, "Restore Last Outputs" brings back the outputs that were on, and opening a show brings that show's outputs on again (G-15). (g10) Switching an output on or off does not by itself mark the show as changed (G-16). (g11) Opening a recording in Review, which opens the show file saved with that recording, never switches an output on or off (G-17). (g12) A start of the app made by a test or a tool never switches an output on (G-9). Kept from the old part g: an output that was on and whose projector is unplugged comes back by itself when the projector is plugged in again; plain Esc never closes an output. The app's standing rule, not a word of his and not in the reading shown to him: an output never takes the keyboard. (h) The Delay's number can also be typed and has plus and minus for one millisecond at a time. (i) "Restore Last Outputs" stays as the computer's own memory, whatever show is open: it switches on the outputs that were last on on this computer, also after All Outputs Off or Cmd+Shift+Esc, and also after a quit and a new start; a screen he switched off by its own line is forgotten there too. Where a screen's settings are opened is laid out by Harmony (L8, P27).
CHANGED: part g: the page's first sentence there, "The app never opens an output by itself" with one click on Restore Last Outputs as the only way back, is REPLACED by L96 (a show remembers its outputs and brings them on). The rest of g (replug, plain Esc) stands. Part e: reworded only. The page's "The monitors inside the app always show the picture as it is now" was written before the preview monitor showed a cue or a previewed clip (L35); what stands is that no monitor carries a screen's Delay or colour; for recordings his L88 now says so himself. Last sentence "Where a screen's settings are opened comes as a picture": REPLACED by L8. Parts a, b, c, d, f, h, i: nothing (he named R191 and changed only this); the pointer to his Resolume picture in part a adds no rule. Everything in g beyond "a show remembers the outputs it was saved with and brings them on when it is opened" is Harmony's reading and is carried by G-1, G-3, G-4, G-5, G-6, G-9, G-14, G-15, G-16 and G-17. "connected" in L96 is read as switched on at the save and plugged in at the opening (INFERRED). "An output never takes the keyboard" came from the project's own rules (Pitfall 40), not from the reading: marked so.
TODAY: outputs are machine state, never show state: a show never opens or remembers an output; the wanted set is in settings.json "outputs"; only Output > Restore Last Outputs opens the saved set (area-outputs.md part 1 items 10, 12; CLAUDE.md "Outputs"; docs/claude/integration.md:27, 29 as cited there). After All Outputs Off the screens opened this session leave the file, so Restore has nothing to open (CORRECTIONS W1, M1: INFERRED from code, not run). All Outputs Off cancels pending replug reopens (part 1 item 11). No Delay, no settings (items 6, 7). A screen is known only by six numbers of place and size (item 7, T3). The builder's numbers for part a: his picture shows Delay 0 ms, Opacity 100 %, and Brightness, Contrast, Red, Green, Blue at 0 with each slider at its middle (boris-feedback-backlog.md:437, Harmony's description of the picture); the adopted ruling took Opacity 0 to 100 % and -1.00 to 1.00 for the five (area-outputs.md part 2 (h)); not re-checked against his Resolume. To change: a new field in the show file for the outputs a show remembers, with the forget-only-by-its-own-line test (the file's shape and version change: Pitfall 68; no old show files are carried over, his L69); bringing the set on at load and at launch; one recognition of a screen for the settings and for the show's set (T3); a guard for test launches (binding-decisions.md:137-147 screen-safety law); CLAUDE.md "Outputs", Pitfall 40 text and integration.md rewritten.
@@END

@@ITEM R209
TITLE: Outputs keep going whatever the main window shows
STATUS: STANDS
HIS: none (not named; L35 leans the same way; L91 for what the outputs show while Review is open)
RULE: The outputs, Syphon and a recording that is running keep going, always. Nothing done in the main window can freeze or stop the audience's picture: opening the big signal bar, changing to another arrangement of the window, hiding a monitor, minimising the window, hiding the app, or bringing another program to the front. The picture is made whether or not any monitor inside the app is shown (G-10). While Review is open the outputs are not frozen either: they show the review picture (186 b, L91; topic E rules it).
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

## ASSUMPTIONS
@@ASSUME G-1
ABOUT: R191
TEXT: I assume each show remembers which outputs were on when you saved it, and opening it switches those on if they are plugged in. Opening a show never switches off an output that is already on.
WHY: L96: the show remembers the outputs. It does not say what happens to an output that is on when a show saved without it is opened.
ALT: b) Opening a show sets the outputs exactly as it was saved and switches the others off. c) The computer remembers the outputs, the same for every show, and a show holds none.
IF-WRONG: STAGE a show opened during a set would leave a projector on that he expected to go off; built the other way it would cut one off
ASK: YES no word of his settles what opening a show does to an output that is on; the audience would see it; it is about what the app does
@@END

@@ASSUME G-2
ABOUT: 200, R191
TEXT: I assume the show remembers Syphon like a screen: it comes on by itself when a show saved with Syphon on is opened, also at the start of the app. Otherwise it stays off until you tick it.
WHY: The default he took (L1) says off at every launch; that sentence repeats the old rule for screens, which his L96 replaced (L9), and he calls Syphon an output.
ALT: b) Syphon is never switched on by a show: it stays off until you tick it or use Restore Last Outputs.
IF-WRONG: SMALL another program gets or misses the picture until he ticks one line
ASK: LINE his L96, his L9 and his earlier word that Syphon is an output settle it by inference; a wrong guess costs one tick; the same assumption is X-7
@@END

@@ASSUME G-3
ABOUT: R191
TEXT: I assume that when you start the app it opens your last show, and the outputs that show remembers come on by themselves, without a click, if they are plugged in.
WHY: L94 (the app opens to the very last show) with L96 gives this; he did not say it in one sentence, and the page had told him the opposite.
ALT: b) At the start of the app no output comes on; they come on only when you open a show yourself.
IF-WRONG: STAGE a projector would come on by itself at the start of the app; mild, because a show opens with empty layers (question 196 default A)
ASK: LINE it follows from two lines of his (L94, L96); shown because the page had told him the opposite; the same assumption is F-12
@@END

@@ASSUME G-4
ABOUT: R191, R226
TEXT: I assume the output on your main display, the one the app itself is on, never comes on by itself, because it would cover the app. You switch it on with Cmd+F.
WHY: L96 speaks of outputs that are connected; the output on the main display is not a connected screen, and nothing of his names it.
ALT: b) It comes back like any other output when the show was saved with it on.
IF-WRONG: SMALL the app would open hidden behind its own picture, or he presses one key
ASK: LINE a choice of Harmony's that departs from the letter of L96 for one output; he would most likely wave it through
@@END

@@ASSUME G-5
ABOUT: R191, D28
TEXT: I assume a show brings back only the very screens it remembers. With another projector, or on another computer, nothing comes on by itself, no message is shown, and you tick the screen once.
WHY: L96 covers only the same outputs, connected again; a different screen in their place, and whether the app says that one is missing, are not said.
ALT: b) When the saved screen is missing, the picture goes to whatever outside screen is plugged in. c) A short note says which remembered screen was not found.
IF-WRONG: SMALL one click at set-up in a new venue
ASK: LINE the careful choice: the picture never lands on a screen he did not pick
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

@@ASSUME G-14
ABOUT: R191
TEXT: I assume a screen that the open show remembers comes on by itself at the moment you plug it in, also when it was not plugged in when the show was opened.
WHY: L96: he does not want to connect them again; the reading he named keeps that a replugged projector comes back by itself. A screen plugged in later is not named.
ALT: b) A screen plugged in after the show was opened waits for a click.
IF-WRONG: SMALL one click
ASK: NO it follows from L96: a click after plugging in would be connecting them again
@@END

@@ASSUME G-15
ABOUT: R191, 200
TEXT: I assume All Outputs Off only switches the outputs off for now: Restore Last Outputs brings back the ones that were on, and opening a show brings that show's outputs on again.
WHY: L96 lets a show switch outputs on; what that does after All Outputs Off, and which outputs Restore Last Outputs then brings back, is not said.
ALT: b) After All Outputs Off nothing comes on by itself, also not when a show is opened, until you switch an output on yourself or press Restore Last Outputs.
IF-WRONG: STAGE a projector he has cut would come on again when he opens a show; mild, because a show opens with empty layers (question 196 default A)
ASK: LINE his L96 read to the letter gives this; the other way is a guard he may want, and he can strike the line. That a plug alone brings nothing back after All Outputs Off is kept from the app as built (area-outputs.md part 1 item 11); it is not his word
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
ABOUT: R191
TEXT: I assume opening a recording in Review never switches an output on or off by itself, although the show file saved with that recording remembers the outputs of that night.
WHY: L96 makes an opened show switch its outputs on; L80 gives every recording a show file of its own. Whether opening that one counts is not said.
ALT: b) Opening a recording in Review brings on the outputs that were on that night.
IF-WRONG: SMALL he ticks one output to watch a recording on the big screen
ASK: NO the careful choice at no cost; topic E owns Review and asks about its outputs (E-1)
@@END

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

@@NAME Restore Last Outputs
MEANS: The command that switches on again the outputs that were last on on this computer, whatever show is open.
SOURCE: the on-screen name now, kept (R191 i stands)
@@END

@@NAME All Outputs Off
MEANS: The command, also Cmd+Shift+Esc, that switches off every output, Syphon included.
SOURCE: the on-screen name now, kept; Syphon included by question 200 default A (L1)
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

## CONFLICTS (from the paper, unruled)
- His L1 on question 200 against his L96. L1: "All defaults good except for these." takes default A of 200, whose text says of Syphon "It is off at every launch". L96: "I expect the show to remember the outputs connected and not need to connect them again", with L94 "When you open the application, it opens to the very last show." and his earlier "syphon is an output and treated with same output settings as a screen" (binding-decisions.md:834). Asked as G-2.
- His L96 against an ADOPTED default, not against a word of his. L96: "if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." The project rule (CLAUDE.md, "Outputs"): "the app never opens an output by itself (only Output > Restore Last Outputs does". No quoted word of his in binding-decisions.md says "never opens" (grep: no hit); the rule is a default of 2026-09-27 that shipped without his answer (area-outputs.md part 5, plan5 Q1 and Q3). L96 wins; CLAUDE.md "Outputs" and docs/claude/integration.md "Output windows" are rewritten when it is built. The open edges are G-1, G-3, G-4, G-5, G-6.
- His L96 against the standing screen-safety law (binding-decisions.md:137-147: "never let an automated gate open fullscreen unattended"; that text is Harmony's law, not a quotation of his). Squared by G-9: a start made by a test or a tool never opens an output by itself.

## NOT DONE / UNSURE (from the paper, unruled)
- What "connected" means in L96 is read as "switched on" at the save and "plugged in" at the opening (INFERRED). If he means only "plugged in" both times, the result is the same for a screen that was on; a screen that was plugged in but off at the save would then also come on. Cheapest: his answer to G-1.
- The exact words he saw on 2026-09-27 for "should the app turn the remembered screens back on by itself" are not on file (area-outputs.md part 5). Not needed any more: L96 answers it.
- What the outputs show while the Review screen is open (186 b, L91) and sending the recording's picture to a monitor (L79) belong to topic E; this paper only assumes that R209's "keep going, always" is not broken by it.
- binding-decisions.md:940 ("51 default is good", the 0 to 100 ms range) was not opened by me; taken from area-outputs.md part 6 item 1 and its CORRECTIONS check (1).
- TODAY lines come from the area sheet; I opened no source file myself.

## FOR THE PAGE RULING (from the ruling)
- L96 puts "which outputs are on" into the show: R188 a (topic F) gains that line; a screen's eight settings stay on the computer (199 A by L1; R188 d, named at L94 and left).
- Ask once: G-3 = F-12 (apply-F has it as YES; ruled LINE here, it follows from L94 with L96). G-2 = X-7 (LINE in both now).
- The one question of this topic is G-1: opening a show never switches off an output that is on (the other way: exactly as saved). The lines that matter most after it: G-6 (a save at home does not forget the projector), G-15 and G-3.
- The paper's CONFLICTS bullet 1 (L1 on question 200 against L96) is resolved by his L9, not open: "Asked as G-2" there now means a line he can strike.
- What a projector shows by itself is decided in three places: G-15 (All Outputs Off is for now; a show opened after it brings its outputs on), E-1 (outputs during Review) and G-17 (opening a recording never switches an output on). Rule them as one.
- Names: "output screen" = the full-screen display; "output monitor" (his L35 "output window") = the monitor inside the app (topic B). His "output settings" = the eight per output; the Global tab's section "Output Settings" (Resolution) collides and needs another name in the list (L114).
- Topic J's rule makes every button mappable except the output settings: say there whether All Outputs Off, Restore Last Outputs and an output's tick can go on a key or a pad.
- Topic B: whether the global Transform shows on the preview monitor (master cue, L38); R226 b no longer claims it.
- If his answer on Snapshot (L94, topic F) is "drop", the word "snapshots" in R226 b and R187 d falls away; nothing else here leans on it. If the low-resolution show recording of L6 runs all night, D26 locks the picture size all night.
- At the build: CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md say the app never opens an output by itself; they are rewritten, and a start made by a test or a tool never opens one (G-9, screen-safety law).

