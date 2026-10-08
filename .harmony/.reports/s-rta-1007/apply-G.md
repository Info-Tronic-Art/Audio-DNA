# APPLY G -- Output screens (s-rta-1007)
Written 2026-10-07 22:41:18 by the architect seat for topic G. Read-only: nothing built, run, probed or launched; Audio-DNA and Resolume Arena not touched. Read whole: boris-msg-numbered.txt, slice-G.md, s-rta-1005/area-outputs.md (with its CORRECTIONS block). Read in part: binding-decisions.md 137-147, 155-163, 790-803, 830-839, 869-875, 1174-1175; slice-E.md (186, R138, R187); slice-F.md (R188); CLAUDE.md "Outputs". Labels: VERIFIED = read by me; INFERRED = reasoned.

## SUMMARY
- ONE thing changes, by his L96: a show remembers which outputs were on when it was saved, and opening it switches them on again by itself when they are plugged in. The page's line "The app never opens an output by itself" (R191 g) is gone.
- That line was never his word: it was a default of 2026-09-27 that shipped without an answer (area-outputs.md part 5, plan5 Q1 and Q3). So his words do not pull apart; an adopted default, CLAUDE.md "Outputs" and docs/claude/integration.md do, and they are rewritten when it is built.
- With his L94 (the app opens to the very last show) it follows that screens can come on at the start of the app. Shown to him as one line he can strike (G-3).
- The one real question (G-1): opening a show only ADDS the outputs it remembers and never switches off one that is on.
- Syphon: his L1 took default A of 200 ("off at every launch"); his earlier word makes Syphon an output, so L96 covers it. These two pull apart: asked (G-2).
- 199 stands as its default: a screen's eight settings belong to the screen on this computer, not to the show. His L88 leans the same way. The show holds WHICH outputs are on, never their numbers.
- R209 and R226 stand; the tab is named Global (L55). P27 is dropped by L8. D25 to D28 are still Harmony's own and are shown as lines.
- Nothing else of his message moves this topic. What the outputs show while the Review screen is open is topic E (186 b, L91; L79).

## ITEMS
@@ITEM 199
TITLE: Where a screen's Delay is remembered
STATUS: DEFAULT option A, by his first line
HIS: L1 (L88 leans the same way)
RULE: A screen's eight settings (Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue) belong to that screen on this computer. When the Mac recognises a projector that was set before, it comes up with the numbers it had last: 40 ms set for one room is 40 ms again at the next venue, and he changes it there for the new room. The settings are not saved in a show and there is no list of rooms; opening or saving any show leaves them as they are. A screen's settings can be changed only while that screen is plugged in. Syphon's settings are kept the same way, on this computer. The show remembers only WHICH outputs are on (R191 g, L96), never their settings.
CHANGED: nothing. Tested against his whole message: L96 names which outputs are connected, not their settings; L88 "screens can be modified to fit a projectors color and timing issues" treats the settings as the projector's, which is option A.
TODAY: no setting of any kind exists per output and there is no settings window (area-outputs.md part 1 item 7). The ruled store is a new settings.json key "outputSettings", on paper only (part 2 (h)). To build: the eight settings, the store, the recognition of a screen (part 4 T3).
@@END
@@ITEM 200
TITLE: Syphon as an output
STATUS: DEFAULT option A, by his first line; its "off at every launch" is narrowed by L96 with L94
HIS: L1, L96, L94
RULE: Syphon is one more line in the Outputs list, with its own tick. It counts in the number the Outputs button shows. It goes off with All Outputs Off and with Cmd+Shift+Esc. Other programs see "Audio-DNA" as a Syphon source only while the line is ticked, never from the mere start of the app. "Restore Last Outputs" brings it back together with the screens. It has the same eight settings as a screen (its Device row reads Syphon). Because Syphon is an output, the show remembers it as it remembers a screen (R191 g): Syphon is off when the app starts, unless the show that opens was saved with Syphon on, and then it comes on by itself (assumption G-2; the other reading is that Syphon stays off at every start and at every opening of a show until he ticks it).
CHANGED: option A's sentence "It is off at every launch" is narrowed: off unless the opened show was saved with it on (INFERRED from L96, L94 and his earlier "syphon is an output and treated with same output settings as a screen", binding-decisions.md:834). The rest of option A stands.
TODAY: "Syphon Output" is a tick item of the Output menu outside the per-display list, not counted by "Outputs: N", not switched off by All Outputs Off, off at every launch and not remembered (area-outputs.md part 1 item 15, H5); the Syphon server is created at start whether ticked or not, so other programs list "Audio-DNA" from launch (item 17, H6: INFERRED there, not run). All of that changes.
@@END
@@ITEM R191
TITLE: Output screens: what stands, and what a show remembers
STATUS: CORRECTED part g replaced by his L96; a to f, h and i stand
HIS: L96 (with L94; L88 for e; L8 for the last sentence)
RULE: (a) Each output screen has eight settings: Device, Delay (0 to 100 ms), Opacity, Brightness, Contrast, Red, Green, Blue; Syphon has the same. (b) Device names the screen and is not a menu; screens are switched on and off in the Outputs list. (c) These settings cannot be put on a key, a knob or a pad, are not recorded and are not Undo steps. (d) A screen's Opacity fades that screen to black; Syphon's Opacity fades the Syphon picture to black too; the master opacity fades the whole show; both stay. (e) The monitors inside the app (the output monitor and the preview monitor) show the picture as it is now, without any screen's Delay or colour; a screen's Delay and colour are judged on that screen. Recordings are the same plain picture (L88). (f) A screen shows the whole composition, fitted with bars, never stretched; no screen shows only a part of it. (g) NEW, his L96: a show remembers which outputs were on at the moment it was saved. When that show is opened, every remembered output that is plugged in switches on by itself, without a click. A remembered screen that is not plugged in is passed over without a message, and it switches on when it is plugged in while that show is open (G-6). Opening a show only adds outputs: it never switches off an output that is on, and a show saved with no output on leaves the outputs as they are (G-1). The same happens at the start of the app, because the app opens to the very last show (L94; G-3). The output on the main display, which covers the app's own window, is the one exception: it does not come on by itself (G-4). A show brings back only the very screens it was saved with, as the Mac reports them; on another computer or with another projector nothing comes on by itself (G-5). The show takes the set of outputs at each Save; switching an output on or off does not by itself make the show count as changed (G-6). The show holds only on or off, never a screen's settings (199). Kept from the old part g: a projector that is unplugged and plugged in again comes back by itself; plain Esc never closes an output; an output window never takes the keyboard. A launch made by a test or by a tool never opens an output by itself (G-9). (h) The Delay's number can also be typed and has plus and minus for one millisecond at a time. (i) "Restore Last Outputs" stays as the computer's own memory, whatever show is open: it brings back the outputs that were last on on this computer, also after All Outputs Off or Cmd+Shift+Esc, and also after a quit and a new start; a screen switched off by its own line is forgotten there. Where a screen's settings are opened is laid out by Harmony (L8, P27).
CHANGED: part g: the page said "The app never opens an output by itself: Restore Last Outputs brings yours back with one click"; REPLACED by L96 (a show remembers its outputs and opens them). The rest of g (replug, plain Esc) stands. Last sentence "Where a screen's settings are opened comes as a picture": REPLACED by L8. Parts a, b, c, d, e, f, h, i: nothing (he named R191 and changed only this). Everything in g after the first sentence is Harmony's reading and is carried by G-1, G-3, G-4, G-5, G-6.
TODAY: outputs are machine state, never show state: a show never opens or remembers an output; the wanted set is in settings.json "outputs"; only Output > Restore Last Outputs opens the saved set (area-outputs.md part 1 items 10, 12; CLAUDE.md "Outputs"; docs/claude/integration.md:27, 29 as cited there). After All Outputs Off the screens opened this session leave the file, so Restore has nothing to open (CORRECTIONS W1, M1: INFERRED from code, not run). No Delay, no settings (items 6, 7). To change: a new field in the show file for the set of outputs that are on; opening the set on load and at launch; a guard for test launches (binding-decisions.md:137-147 screen-safety law); CLAUDE.md "Outputs", Pitfall 40 text and integration.md rewritten.
@@END
@@ITEM R209
TITLE: Outputs keep going whatever the main window shows
STATUS: STANDS
HIS: none (not named; L35 leans the same way)
RULE: The outputs, Syphon and a recording that is running keep going, always. Nothing done in the main window can freeze or stop the audience's picture: opening the big signal bar, changing to another arrangement of the window, hiding a monitor, minimising the window or hiding the app. The picture is made whether or not any monitor inside the app is shown (G-10).
CHANGED: nothing. Tested against his whole message: L35 "We will have various configurations other than live and recording review mode" means the main window will often not show the monitor, which makes this rule necessary; no word of his asks for a frozen picture.
TODAY: the monitor panel's GL context hosts the render loop; when the panel is hidden (big signal bar open) or the app is minimised, every output holds its last frame and Syphon and the recorder stop getting frames (area-outputs.md part 1 item 13, H7, T1, T6: seen in code, not run). To change: the render loop must live apart from the panel.
@@END
@@ITEM R226
TITLE: Picture size, the Transform and the output window
STATUS: STANDS only the tab's name changes (L55)
HIS: none (not named; L55 for the tab's name)
RULE: (a) The picture size is chosen in the Resolution list in the Global tab: 1920x1080, 1280x720, 2560x1440, 3840x2160, a portrait size, a square size and a 4:3 size; a size cannot be typed. (b) The Transform section of the same tab (Position X, Position Y, Scale, Rotation, Anchor) moves, scales and turns the whole picture after every effect and before the master opacity, so the monitors, every screen, Syphon, recordings and snapshots show it; what the picture no longer covers is black. (c) The Anchor has two sliders, sideways and up-down, as Position has. (d) An output is always a borderless window that fills one display; there is no output window that can be moved or sized. (e) Keys: Cmd+Shift+Esc all outputs off, Cmd+F the main display's output, Cmd+` brings the app over an output.
CHANGED: only a name: the page's "Composition tab" reads "Global tab" (L55 "lets move to global"; question 210 default A: INFERRED that L55 names this tab too). Parts a to e: nothing.
TODAY: Resolution drop-down in the inspector's Composition tab, section "Output Settings" (area-outputs.md part 1 item 25); five Transform controls, the one Anchor slider writes X only and forces Y to 0 (items 27-29, CORRECTIONS W2); outputs borderless, one per display (item 4); keys (item 8). To change: the Anchor's up-down slider; the tab's name.
@@END
@@ITEM D25
TITLE: Rotating and scaling the whole picture
STATUS: OPEN still Harmony's own; shown to him as a line (G-7)
HIS: none
RULE: Rotating the whole composition turns the picture rigidly, without stretching or shearing it, on a picture of any shape. The lowest Scale gives a very small picture; it never gives a huge one.
CHANGED: nothing; no word of his touches it.
TODAY: the transform rotates in unit space with no aspect term, so a wide picture shears; Scale at 0 is a 1000 times zoom (area-outputs.md CORRECTIONS M3: read in EmbeddedShaders.h:136-160 there, not run).
@@END
@@ITEM D26
TITLE: Changing the picture size while recording or showing
STATUS: OPEN still Harmony's own; shown to him as a line (G-8)
HIS: none
RULE: The Resolution cannot be changed while a recording runs. Opening a show of another picture size while a recording runs asks first, in the one window that says the recording will be stopped and kept (R187 b). With a screen or Syphon on, a new picture size is allowed at any time, and the screens may flash for about half a second.
CHANGED: nothing; no word of his touches it. L88 is about when a recording starts and ends, not about the picture size.
TODAY: no guard exists; the Resolution list can be changed at any time (area-outputs.md CORRECTIONS M2; slice-G D26 cites src/ui/CompositionInspector.cpp:181-189); the flash is docs/claude/rendering.md:162 as cited there. What a running recording does on a size change: not checked.
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
STATUS: OPEN in part: its first sentence is settled by 199's default (L1); the rest is Harmony's own (G-12)
HIS: L1 (199 default A); L88 leans the same way
RULE: A screen's settings are tied to the screen as the Mac reports it, never to its size or its place. A different projector on the same socket comes up with default settings. Two projectors of the same model that the Mac cannot tell apart share one set of settings, and the settings place says so. Raising a screen's Delay holds its picture still for the added time; lowering it skips ahead; the screen never goes black for it. The same recognition of a screen is used for the outputs a show remembers (R191 g, G-5).
CHANGED: nothing replaced. Settled by his words: "tied to the screen" (199 A, L1). Still Harmony's: the different projector, the twins, hold and skip.
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
TEXT: I assume each show remembers which outputs were on when you saved it. Opening it switches those on if they are plugged in. It never switches off an output that is already on.
WHY: L96 says the show remembers connected outputs; it does not say what happens to an output that is on when a show saved without it is opened.
ALT: b) Opening a show sets the outputs exactly as saved and switches the others off. c) The computer remembers the outputs for every show alike, as Resolume does, and a show holds none.
IF-WRONG: STAGE a show opened during a set could black out a projector, or leave one on that he expected off
ASK: YES no word of his settles it; the audience would see it; it is about what the app does
@@END
@@ASSUME G-2
ABOUT: 200, R191
TEXT: I assume Syphon is remembered by the show like a screen: it comes on by itself when you open a show that was saved with Syphon on. Otherwise it is off when the app starts.
WHY: His L1 took the default "off at every launch"; his earlier word calls Syphon an output, and L96 with L94 brings outputs back when the last show opens.
ALT: b) Syphon is never switched on by a show: it stays off until you tick it or use Restore Last Outputs.
IF-WRONG: SMALL another program gets or misses the picture until he ticks one line
ASK: YES two statements of his pull apart (L1 on 200 against L96); see CONFLICTS
@@END
@@ASSUME G-3
ABOUT: R191
TEXT: I assume the same happens when you start the app: it opens your last show, so the outputs that show was saved with come on without a click.
WHY: L94 (the app opens to the very last show) and L96 together give this; he did not say it in one sentence, and the page had told him the opposite.
ALT: b) At the start of the app no output comes on; they come on only when you open a show yourself.
IF-WRONG: SMALL seen at the first start, one switch to change
ASK: LINE follows from two lines of his; he can strike it
@@END
@@ASSUME G-4
ABOUT: R191, R226
TEXT: I assume the output on the laptop's own screen never comes on by itself, because it would cover the app. You switch it on with Cmd+F.
WHY: L96 speaks of outputs that are connected; the main display's output is not a connected screen, and nothing of his names it.
ALT: b) It comes back like any other output when the show was saved with it on.
IF-WRONG: SMALL the app would start hidden behind its own picture, or he presses one key
ASK: LINE a choice of Harmony's he would most likely wave through
@@END
@@ASSUME G-5
ABOUT: R191, D28
TEXT: I assume a show brings back only the very screens it was saved with. With another projector, or on another computer, nothing comes on by itself and you tick the screen once.
WHY: L96 covers only the case "open the show back up with the outputs connected"; a different screen in the same place is not said.
ALT: b) The picture goes to whatever outside screen is plugged in when the saved one is missing.
IF-WRONG: SMALL one click at set-up in a new venue
ASK: LINE the careful choice: the picture never lands on a screen he did not pick
@@END
@@ASSUME G-6
ABOUT: R191
TEXT: I assume the show takes the outputs that are on at the moment you save. Switching an output on or off does not make the app ask you to save. A remembered screen plugged in later comes on then.
WHY: L96 says "if I save a show with the outputs connected"; when the set is taken and what a late plug-in does are not said.
ALT: b) Every switch of an output counts as a change to the show and is asked about at closing. c) A screen plugged in after the show opened waits for a click.
IF-WRONG: SMALL
ASK: LINE a real choice, cheap to change
@@END
@@ASSUME G-7
ABOUT: D25
TEXT: I assume rotating the whole picture turns it without stretching it, and the lowest Scale gives a very small picture.
WHY: Decided without asking; he did not read that list (L130) and no word of his touches the Transform.
ALT: none
IF-WRONG: SMALL
ASK: LINE a mend he would most likely wave through
@@END
@@ASSUME G-8
ABOUT: D26
TEXT: I assume the picture size cannot be changed while a recording runs. With screens on it can be changed, and they may flash for about half a second.
WHY: Decided without asking; he did not read that list (L130) and no word of his touches it.
ALT: b) With a screen on, the app asks before it changes the picture size. c) The size is locked while any output is on.
IF-WRONG: STAGE the audience would see a short flash if he changes the size during a show
ASK: LINE he chooses the moment himself; he can strike it
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

## QUESTIONS BACK
None: his message asks no question of his own in this topic. (L94 "Do we need snapshot?" belongs to topic F.)

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
SOURCE: his words L55 "lets move to global"; the on-screen name now is "Composition", which is replaced by Global
@@END

## CONFLICTS
- His L1 on question 200 against his L96. L1: "All defaults good except for these." takes default A of 200, whose text says of Syphon "It is off at every launch". L96: "I expect the show to remember the outputs connected and not need to connect them again", with L94 "When you open the application, it opens to the very last show." and his earlier "syphon is an output and treated with same output settings as a screen" (binding-decisions.md:834). Asked as G-2.
- His L96 against an ADOPTED default, not against a word of his. L96: "if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." The project rule (CLAUDE.md, "Outputs"): "the app never opens an output by itself (only Output > Restore Last Outputs does". No quoted word of his in binding-decisions.md says "never opens" (grep: no hit); the rule is a default of 2026-09-27 that shipped without his answer (area-outputs.md part 5, plan5 Q1 and Q3). L96 wins; CLAUDE.md "Outputs" and docs/claude/integration.md "Output windows" are rewritten when it is built. The open edges are G-1, G-3, G-4, G-5, G-6.
- His L96 against the standing screen-safety law (binding-decisions.md:137-147: "never let an automated gate open fullscreen unattended"; that text is Harmony's law, not a quotation of his). Squared by G-9: a start made by a test or a tool never opens an output by itself.

## NOT DONE / UNSURE
- What "connected" means in L96 is read as "switched on" at the save and "plugged in" at the opening (INFERRED). If he means only "plugged in" both times, the result is the same for a screen that was on; a screen that was plugged in but off at the save would then also come on. Cheapest: his answer to G-1.
- The exact words he saw on 2026-09-27 for "should the app turn the remembered screens back on by itself" are not on file (area-outputs.md part 5). Not needed any more: L96 answers it.
- What the outputs show while the Review screen is open (186 b, L91) and sending the recording's picture to a monitor (L79) belong to topic E; this paper only assumes that R209's "keep going, always" is not broken by it.
- binding-decisions.md:940 ("51 default is good", the 0 to 100 ms range) was not opened by me; taken from area-outputs.md part 6 item 1 and its CORRECTIONS check (1).
- TODAY lines come from the area sheet; I opened no source file myself.
