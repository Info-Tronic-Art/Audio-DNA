# RULING looks-answers -- architect ruling on the blind council's attacks on plan-looks-answers.md (lane "looks-answers": the per-effect looks lane, DELTA on Boris's answers to questions 101-104 and 106; s-rta-1004)

Author: architect (ruling; run on opus at max effort, as the dispatch states). Harmony decides after this; the ruling is her
working document.
Pins: `git -C /Users/boriskarpman/projects/RealTimeAudio rev-parse --short HEAD` printed 185147b and
`git status --short -- src tests docs CMakeLists.txt` printed nothing, so every code read below is a plain file at 185147b. The two
stopped worktrees were checked (lane/bf2 740b6d6, lane/bf2-keys 9eab9bd, both clean) and NOT read further: nothing of this lane
comes from them. Nothing was built, run or launched. Nothing under ~/Library was read or listed.
Seat papers, verbatim and whole: .harmony/.reports/s-rta-1004/attack-looks-answers-papers.md (3 seats, 23 attacks, 27419
characters, a closed JSON array, parsed).
Labels: VERIFIED = I read the line at 185147b (or in JUCE under build/_deps/juce-src/modules). SHEET = a fact-sheet row its own
VERIFICATION section confirmed. INFERRED / ASSUMED are written where used.
Short names: PL:n = plan-looks-answers.md line n; RU = ruling-effect-looks.md (RU:n its line n; AM-n, LK-n, ST-n, LC-n, LM-n,
GL-n, V-n, HD-n, MU-EL-n are ITS ids); FOS = facts-one-save.md; BD:n = binding-decisions.md line n; BFB:n =
boris-feedback-backlog.md line n. Paths are under /Users/boriskarpman/projects/RealTimeAudio.
THREE NAME SPACES, kept apart in this document: the PLAN's delta amendments are written "PL DA-n"; the SEATS' attacks are written
"A:DA-n", "A:GA-n", "A:ST-n"; RU's store test rows keep "ST-n". This ruling's amendments are "RA-n".
Precedence: Boris's verbatim words > Harmony's adoption (plan-effect-looks.md:619-667) > this ruling (once adopted) > the plan
looks-answers > RU > the first plan's body. Where this ruling and the plan looks-answers are both silent, RU stands word for word.
Boris is quoted only verbatim, from binding-decisions.md and boris-feedback-backlog.md.

---------------------------------------------------------------------------------------------------------
## 0 VERDICT
---------------------------------------------------------------------------------------------------------
NEEDS REVISION. The delta's core is sound and is kept: a look carries the connection on each slider, a load makes the effect
what the look says and leaves an equal connection alone, New Look opens a name box, Save over swaps a whole verified file in.
17 amendments (section 3) override the plan body. 23 attacks ruled: 16 ACCEPT, 7 PARTIAL, 0 REJECT. Both MUST attacks
found real holes (Save over could name the wrong look; his own default, "look 2 remains unchanged", had no machine row);
both are closed. Five errors of the plan that no seat named were found on the way and are corrected (E1-E5, section 1).

RULED FIRST, four things.

(1) WHAT A LOOK HOLDS NOW, AND WHAT LOADING ONE DOES TO EVERY SIGNAL ALREADY PLUGGED INTO THAT EFFECT.
A look holds, for ONE effect: each slider's value, Dry / Wet, and for each slider and for Dry / Wet the connection plugged
into it -- which source (a signal by name, a macro by number, an LFO, a timeline, a clip position), its range, invert, the
other saved shaping, and on or off. Never bypass, never the effect's own `enabled`, never a hand on a slider, never the
engine's memory. Boris: "102 B"; the option as asked reads "The look also remembers which signal drives each slider, and
loading it plugs them in again (a later build)." (boris-clarify-101-106.md:14).
Loading a look makes the effect what the look says, slider by slider:
- the look has a signal on the slider and the effect has the SAME one (same source, range, invert, shaping, on / off): nothing
  is touched. The hand on it, its smoothing and its memory carry on.
- the look has a signal on the slider and the effect has none, or another: the look's signal is plugged in, fresh.
- the look was made with NO signal on that slider and the effect has one: the signal is UNPLUGGED (the default; question 131).
- "Default": every slider goes to its default value, Dry / Wet to 1.0, and every signal on that effect is UNPLUGGED (the same
  default; question 131). "Default" is the effect as it is when it is first added, and a new effect has no signals.
- a slider the look does not know (added in a later version), and every look written in RU's first format (no signals in the
  file): the signal on it is left exactly as it is.
One Cmd+Z puts back values AND signals as they were. The option he took says "plugs them in again" and is silent on a slider the
look holds without a signal: that is why 131 is a question. The default is "unplug" for one reason that does not depend on taste: a
signal on a slider overrides the look's value on that slider, so a look that cannot say "no signal here" cannot bring back its
own picture on an effect that has signals. Both behaviours are built and unit-tested; his answer changes ONE constant (RA-10).

(2) A LOOK THAT NAMES A SIGNAL THE CURRENT SHOW DOES NOT HAVE.
The look loads WHOLE: every value lands, and the connection is stored on the slider exactly as the look says. A Signal
connection whose name no signal has drives NOTHING: the slider shows and renders the look's own value, and its row shows the
signal's name -- a state, not a message. The moment a signal of that name exists, it drives. No text, no window, no count.
That is one line in the connection engine (stage SE). FACT, corrected against the plan: at 185147b nothing in the app can make
or remove a signal (`addSignal` has no caller), so from inside the app this state cannot arise today; it is reachable only by
a look file written by hand or by another build, or after a built-in signal is renamed. So: the rule stays (Harmony constraint:
a stated rule, never a half-plugged effect), a pinned list of the 32 built-in names turns red on a rename (CE-4), and nothing
about it is shown to Boris as something he can try (RA-6).

(3) "SAVE OVER": HOW THE FILE IS REPLACED SAFELY.
This sentence REPLACES RU:45-47 ("No look file is ever read-modify-written") and goes into the pitfall text:
"A look file is never edited in place. It changes in exactly one way: Save over writes a complete new file beside it, reads
that file back through the listing's own reader, swaps it in with ONE rename in the same folder, and reads the look back from
its own name. If any step fails the look on disk is the old one, whole, and the menu still lists the old one. An unreadable
file is still never rewritten and never deleted; Save over is offered only for a look that lists and that this build wrote or
can rewrite without loss (its version is not above this build's)."
The plan's "at every instant ... whole old or whole new" is struck: JUCE's call is a rename with a copy as fall-back, and the
honest claim is the one above, pinned by a new row that makes the swap itself fail (ST-16; RA-9).

(4) HOW THE MENU TELLS "NEW LOOK" FROM "SAVE OVER LOOK 2".
Two items, in this order, under his looks: "New Look", then `Save over "Look 2"`. New Look opens the name box holding the next
free "Look N" and NEVER touches an existing file -- Boris: "For a default behavior I will want to create a look 3 when I change
look 2 and save a differnt version of look 2. look 2 remains unchanged but I need to have a way to save look 2 if I tweak it a
little." Save over carries the NAME of the look it will replace in its own text; that look is the one last loaded or made on
that effect (a runtime name on the effect; a look the values merely match is taken only for an effect that has none, and
only when its row is built -- RA-1). With no such look the
item reads "Save over" and is greyed; it never disappears. Its confirm window names the look again, and in that window ONLY A
CLICK on "Save Over" saves: Return and Esc both cancel (RA-2), because the item sits next to New Look, whose box teaches
"click, Return".

THE REST, IN ONE PARAGRAPH. The name box asks the file system whether a name is taken, at every keystroke and once more at
the accept, and the view gives the field the keyboard and selects its text itself (RA-3). Look files are read strictly: a name
this build does not know inside a connection refuses the whole look instead of quietly meaning something else, and an older
build never saves over a newer build's look (RA-4). The format is pinned by one hand-written file (RA-5). The after-write hook
is no longer an assumption -- the engine uses the tick's time step only for smoothing, where a step of zero is safe -- and it
gets a live RED arm: the load's own answer carries the live values (RA-7). The wired fence measurement uses two wired looks,
written out (RA-12). Stages: S1a, S1b, SE, S2, S3, VG; 81 new unit cases; ten live rows; 22 visual states.

HARMONY'S SIX CONSTRAINTS, AND WHERE EACH IS MET
- Harmony constraint: a look that names a signal the show does not have must still load -- whole or nothing for the slider
  values, a stated rule for the connection, never a half-plugged effect, never a text. -> (2); RA-6; CE-1, CE-3; GL-9 step (c).
- Harmony constraint: loading a look onto a playing layer never makes the picture black or stutter; measured, not assumed
  away. -> PL DA-5 rules 1-3 and RA-7 (the hook); GL-4, both arms; GL-9 step (b).
- Harmony constraint: say exactly what "Default" and a look made with no signals do to signals that are plugged in, and ask
  Boris only if his words do not settle it. -> (1); RA-10; question 131 (his words do not settle it).
- Harmony constraint: look files written by the ruling's format stay readable. -> PL DA-2's first-format rule; LK-17; GL-2.
- Harmony constraint: Save over is a verified write, never a file left half-written; say what replaces "no file is ever
  rewritten". -> (3); RA-9; ST-13..ST-17; GL-10.
- Harmony constraint: the name box takes the keyboard while it is open and hands it back; the looks button itself still never
  does. -> RA-3; F16; LM-8, LM-21; section 6 items 1, 3 and 12.

The dispatch names questions 47-50 as open. The record shows them answered (BD:864-881) and this lane touches none of them: no
line of this ruling changes with any of those answers. "Routines" become "actions" (Boris: "I want to change what we are
calling routines to actions. Easier to remember") is its own lane; this lane writes neither word on screen.

---------------------------------------------------------------------------------------------------------
## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
---------------------------------------------------------------------------------------------------------
The connection and its file shape
- F1 Copy-ASSIGNING a `ParamConnection` copies source, shape, enabled and resets `grip` and `state`
  (src/connect/ParamConnection.h:158-166); the copy constructor carries only the three saved members (:156-157); a move keeps
  everything (:167-168). VERIFIED. PL C4 holds.
- F2 `ConnSerialization::fromVar` is lenient where a name is not known: an unknown curve name gives 0, Linear
  (src/connect/ConnSerialization.cpp:87-93); an unknown playback gives Forward (:56-61); an unknown point interpolation gives
  Linear (:74-79); any envelope clock but "clipPosition" gives Beats (:207-208). A macro's index is
  `static_cast<int>(srcObj->getProperty("index"))` (:188): a missing key is a void var, which converts to 0 -- Macro 1
  (INFERRED from juce::var's conversion; not run). Only an unknown source kind (:247-255) and an unknown LFO shape (:193-196)
  are counted. The keys written: "src", "shape" (min, max, invert, playback, loop, curve, inMin, inMax, smoothMs,
  resetPhaseOnStructural), "enabled" (:95-160). VERIFIED. A:DA-3's and A:DA-4's citations hold.
- F3 The show writes an effect's connections keyed by parameter INDEX "p" (src/model/Clip.cpp:113-125). A look files them by
  parameter name. VERIFIED.
The engine
- F4 The Signal case (src/connect/ConnectionEngine.cpp:106-110) resolves by name every tick (:21-43); with no such signal the
  id is 0 and `getCachedValue(0)` returns 0.0 (src/signal/SignalRegistry.cpp:181-189; ids start at 1, SignalRegistry.h:51), so
  the slider is driven to the bottom of the connection's range. `evaluate` is ONE function for every target: macros (:327-328,
  a NaN falls back to `manualValue`), composition / layer / clip scalars (:245-246), effect slots (:272-273, :284-285), source
  parameters (:372-373). VERIFIED. A:GA-6's reach claim holds.
- F5 The tick's time step `dt` is used in exactly one place, the smoothing (:159). `applySmoothing` with a NaN memory returns
  the value and stores it; with a number and dt = 0 it returns the memory unchanged (src/connect/ConnectionShaper.cpp:93-106).
  Grip expiry and the hand-back glide use `ctx.now`, not dt (:80-84, :164-181). VERIFIED. So evaluating one slot with dt = 0 is
  safe: PL DA-5's ASSUMED piece is settled by reading.
- F6 The regular tick is `MainComponent::tickFeaturePipeline` (src/MainComponent.cpp:4220), on a 120 Hz message-thread timer
  (src/MainComponent.h:371, :378). It reads the snapshot (:4222), evaluates the registry (:4229), reads the Master Signal depth
  (:4239), takes `now = connNow()` (:4260), computes dt from `lastConnTick_` (:4292-4295), builds the Context (:4296-4297) and
  ticks (:4298). Every input of a Context is a member or a call available on the message thread outside the tick. VERIFIED.
- F7 No clip clock is passed anywhere: `tick` gives nullptr for global (:337), layer (:343-345) and clip (:353-355) targets,
  and a Clip Position source and an Envelope on the clip-position clock then evaluate at position 0.0 (:141, :149). The
  picker's "Timeline" is an Envelope on the BEAT clock, a 4-beat ramp (src/connect/ConnPicker.cpp:90-100): it moves. The
  picker offers "Clip Position" (src/ui/UniversalParamControl.cpp:511, :559-561; ConnPicker.cpp:86-88): that wire holds still
  on every owner today. VERIFIED. A:ST-6 is right for Clip Position and wrong for the picker's Timeline.
- F8 `grep -rn "addSignal\|removeSignal"` over src, tests, docs/claude and CLAUDE.md prints only the two declarations
  (SignalRegistry.h:24-25) and the two definitions (SignalRegistry.cpp:88, :101). NOTHING calls either. `initDefaults`
  registers 32 fixed names: 8 visible audio signals (:22-29), 21 hidden ones (:39-61), "Mod 1", "Mod 2" (:65, :70) and
  "Clip Position" (:77). VERIFIED. A:DA-5 and A:ST-1 hold; PL:163-164 ("a user signal") and PL R2 are wrong. FOS M-6
  (facts-one-save.md:244, SHEET) says user signals have no serializer; it never says one can be made.
- F9 "Mod 1" is an OscillatorSignal and "Mod 2" an EnvelopeSignal; src/ui/SignalInspector.cpp and src/api/ApiServer.cpp name
  those types, and neither is saved (FOS M-6). INFERRED (the setters were not read): what Mod 1 and Mod 2 DO can be changed
  per run and is not in a show or a look, exactly as for the eight macros.
The row on screen
- F10 `UniversalParamControl::bindConnection` first RELEASES a live grip of the connection it was bound to
  (UniversalParamControl.cpp:126-129), then caches the source's text, range and invert (:134-144). The picker assigns a whole
  fresh source (:664; ConnPicker.cpp:56-108) or calls `disconnect` (:660; src/connect/ManualWrite.cpp:205-212, which stores
  NaN in the twin and leaves `shape`). VERIFIED. PL DA-6's "re-binds the controls ... (`bindConnection` again)" would let go of
  a hand on a connection the load did not change, against PL DA-5 rule 1 (RA-17).
- F11 `EffectScope` has four fields: kind, deckIndex, layerIndex, column (src/core/EffectScope.h:16-23). The view keeps
  `scope_` (src/ui/EffectStackView.h:146). `refresh()` pushes `effParam` / `effDryWet` to the row and ends with an
  unconditional repaint (src/ui/EffectStackView.cpp:188-219); rows bind once, when built (:369, :423-425). VERIFIED.
- F12 `EffectSlot` (src/model/Clip.h:60-114): `paramConns`, `paramLive`, `dryWetConn`, `dryWetLive` (:73-76),
  `resizeParams` (:83-87), `effParam` / `effDryWet` (:109-113). No positional brace initialisation of an EffectSlot exists in
  src or tests (two greps, nothing printed). Under src/render effect vectors appear only as const references
  (render/CompositorEngine.h:168, :451; CompositorEngine.cpp:413, :1274). A connection already holds a std::string
  (ParamConnection.h:37) that the picker writes unfenced on the message thread (UniversalParamControl.cpp:664). VERIFIED. So a
  string member `lookName`, last in the struct and written only on the message thread, adds no new class of hazard; PL R4's
  NOT VERIFIED is settled.
JUCE (VERIFIED as source in build/_deps/juce-src/modules; nothing was run)
- F13 `File::replaceFileIn` (juce_core/files/juce_File.cpp:323-336) calls `replaceInternal`, which on macOS is
  `moveInternal` (juce_core/native/juce_SharedCode_posix.h:434-437; no macOS override: `replaceInternal` is defined only there
  and in juce_Files_windows.cpp:372). `moveInternal` tries `rename` first (:417); if that fails it tries `copyInternal(dest)`,
  then deletes the source, and if THAT fails it deletes the destination (:423-429). On macOS `copyInternal` is
  NSFileManager's copyItemAtPath (juce_core/native/juce_Files_mac.mm:44-55) -- INFERRED from Apple's documented behaviour, not
  readable here and not run, to refuse an existing destination. On Linux it deletes the destination first
  (juce_core/native/juce_CommonFile_linux.cpp:38-58). So PL R7 is settled (a rename, no macOS override), PL DA-10's "at every
  instant" is more than the call promises, and A:GA-4's "a non-atomic overwrite" is right for Linux and not shown for macOS.
- F14 Keys in an AlertWindow. `AlertWindow::keyPressed` gives a key to the button registered for it with `triggerClick()` and
  no enabled test (juce_gui_basics/windows/juce_AlertWindow.cpp:549-558); `triggerClick` posts a command that
  `Button::handleCommandMessage` runs only when `isEnabled()` (juce_gui_basics/buttons/juce_Button.cpp:390-399).
  `Button::keyStateChangedCallback` also returns first when disabled (:641-644). So a greyed button does nothing on Return by
  BOTH paths: PL J6 holds, with A:GA-8's citation as the one a Return in the window takes.
- F15 A FOCUSED button clicks itself on Return whatever its shortcuts are (`Button::keyPressed`, juce_Button.cpp:665-671).
  `AlertWindow::addButton` makes every button want the keyboard with explicit focus order 1
  (juce_AlertWindow.cpp:133-134) and lays buttons out in the order added (:477-490, INFERRED from the loop).
  `addTextEditor` sets select-all-when-focused and does not let the field swallow Return or Esc (:194-195). The window itself
  wants the keyboard only when it has no children (:530). The default focus is the first component in focus order that wants
  it (juce_gui_basics/keyboard/juce_KeyboardFocusTraverser.cpp:41-44, :74-81); the order puts explicit orders first and looks
  only at visible, enabled children (juce_gui_basics/detail/juce_FocusHelpers.h:42-77). So BY SOURCE an AlertWindow hands the
  keyboard to its first enabled BUTTON, not to its text field. What happens on screen was not run (section 8, facts to
  measure). Two consequences: the name box must give its field the keyboard itself (RA-3), and in a confirm window the FIRST
  button answers Return even with no shortcut (RA-2).
- F16 Keys under a modal window: `ComponentPeer::getTargetForKeyPress` sends a key to the modal component when the focused one
  is blocked by it (juce_gui_basics/windows/juce_ComponentPeer.cpp:175-187), and src holds no native key monitor (grep
  addLocalMonitor / addGlobalMonitor / CGEventTap: nothing). So no key typed while the box is open reaches a clip trigger
  through JUCE's dispatch: PL DA-12's INFERRED is VERIFIED as source. Key RELEASES are still polled by
  `MainComponent::keyStateChanged` (MainComponent.cpp:4120-4145), as PL:318-320 says.
- F17 The app's window idiom: `renameRoutine` (MainComponent.cpp:6328-6353); `deleteRoutine` binds Return to "Delete"
  (:6367-6368). VERIFIED. A:ST-7's citation holds.
The tests the seats named
- F18 tests/test_layer_strip_follows_model.cpp:113-129 wires the unregistered name "rms" and then sets the twin BY HAND; it
  never ticks the engine. tests/test_show_model.cpp:1341-1359 builds a show for a conversion test. Neither calls `evaluate`:
  the engine line changes neither (VERIFIED for the lines read; INFERRED for the rest of the second file). The test_connection
  target already links ConnectionEngine.cpp, ConnSerialization.cpp and SignalRegistry.cpp (tests/CMakeLists.txt:603-610) and
  has 40 cases.
The record
- F19 His five answers: BD:922-933, BFB:652-672. The option he took for 102, as asked: boris-clarify-101-106.md:14. What was
  done at his word and what was left alone: .harmony/s-rta-1004-work.md:43 -- the two presets moved to the Trash; "LEFT ALONE,
  not named by him": Presets/fast_saves (9 entries, dated 2026-03-14), Presets/"test 1.deck.json", the four *.deck.json in
  Decks. RU's "four old files" were the four entries of the Presets folder (plan-effect-looks.md:632-636). A:DA-7 holds.
- F20 Return in another window he was asked about -- Boris: "93 b" (BD:938): Return presses "Save & Quit", the lit button, and
  Esc cancels. Used in RA-2 only as a sign that he expects Return on the SAFE button.
Errors of the plan that no seat named (each ruled in section 3)
- E1 PL:510 predicts for mutant MU-LA-12 "undo gave 0 connections back". With undo restoring values but not connections, the
  slot keeps Ghost's ONE connection: the FAIL line reads 1 where 2 are expected (RA-16).
- E2 PL:201 says GL-4's second arm uses "two WIRED looks"; PL:506 runs "Wired" and "Default" in turn (RA-12).
- E3 PL:216-217 re-binds the row through `bindConnection`, which releases grips (F10; RA-17).
- E4 PL:300 and PL:520 lean on the field having the keyboard as the box opens; by source the first button has it (F15; RA-3).
- E5 PL:550-551 tells Boris that after a Save over "Look 2 now holds the change, on every effect". A look is copied into an
  effect when loaded; an effect that had the old Look 2 keeps its settings (RA-14).

---------------------------------------------------------------------------------------------------------
## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it; RA-n = the amendment in section 3)
---------------------------------------------------------------------------------------------------------
| id | sev | verdict | decided by | what changes |
|---|---|---|---|---|
| A:DA-1 | MUST | ACCEPT | PL:249-251: writer (3) sets the loaded look to whatever the button derives, at every refresh; with two equal looks that is the earlier name, and Save over has no undo (PL:279-285). | RA-1: writer (3) acts only on an effect with NO loaded look, and only when its row is built; the loaded look wins a tie for the button and the tick. LM-22; GL-10 with a tie and its own mutant (MU-LA-32). |
| A:DA-2 | SHOULD | PARTIAL | PL:308-309 guards the name box by "the row's index still holds that effect" and gives the confirm no guard; F11: the scope has four fields and the view can be re-pointed while a window is open. | RA-2: one guard for the box and the confirm (the view's vector, the scope's four fields, the index, the effect's name; for Save over also that the look still lists and differs). Rejected: capturing at the menu pick -- the window says "the settings on now", and both windows keep one rule: the settings at the accept. |
| A:DA-3 | SHOULD | PARTIAL | F2: unknown names load as Linear / Forward / Beats and a missing macro index as Macro 1, uncounted; PL:276 lets Save over write the file fresh. | RA-4: a connection is read strictly (every name must survive a write-back; index and name must be present); Save over refuses a look whose version is above this build's. Rejected: reading a newer file "as values only" (that is half a look) and scanning for unknown keys (a build that adds a key raises the version: the pitfall text says so). |
| A:DA-4 | SHOULD | ACCEPT | PL:417: LK-16 is a round trip through one code; F2: the keys are plain strings another lane is editing (PL:260). | RA-5: LK-23, a hand-written literal file that must load to exact fields; MU-LA-33. |
| A:DA-5 | SHOULD | ACCEPT | F8: nothing calls `addSignal`. | RA-6: the premise is re-worded; the engine line stays; CE-4 pins the 32 names; Boris check 6 (c) is replaced. |
| A:DA-6 | SHOULD | PARTIAL | PL:296-311 gives the verdict "the effect's file names" while RU reads a folder once per run (RU:273-274). | RA-3: the verdict asks the file system (`exists` on the built path) as well as the listed names, at every keystroke and at the accept. Rejected: keeping the box open when `make` fails -- a failed make closes the box and the button keeps reading "Looks" (RU AM-5's state); `make` itself never overwrites. |
| A:DA-7 | SHOULD | ACCEPT | F19: the work log names what was left alone; PL:558-559 dropped the sentence that told him. | RA-15: item 14 says what stays on his disk; the count is corrected; question 134. |
| A:DA-8 | SHOULD | PARTIAL | His answer is "102 B" (BD:926); the option as asked says only that loading "plugs them in again" and is silent on a slider the look holds without a signal: it is HIS to answer. | RA-10: question 131 is asked with the adoption, with the stage risk in its text; the default stays "unplug" (section 0 (1), section 10 R1); both policies are built and tested, so his answer is one constant. Reconciled with A:ST-2. |
| A:GA-1 | MUST | ACCEPT | PL:507, :511, :457: no row loads a look, tweaks, makes a new one and then reads the loaded look's file. | RA-11: GL-10 step (b); LM-16 gains the clause; MU-LA-30. |
| A:GA-2 | SHOULD | ACCEPT | PL:510: GL-9 (b) is a second HTTP call racing a 120 Hz timer (F6); its RED arms do not touch the hook. | RA-7: `look_load`'s own answer carries the live values, read in the callback that ran the command; MU-LA-27 is the live RED arm. F5 settles the ASSUMED piece. |
| A:GA-3 | SHOULD | ACCEPT | A same-size vector copy-assign keeps its buffer (INFERRED from the standard library's rule; not run); the ruling does not lean on it. | RA-8: MU-LA-16 is pinned to the move form; LC-12 also compares `data()`; each mutant's row is named. |
| A:GA-4 | SHOULD | ACCEPT | F13: rename first, a copy as fall-back; ST-13's two arms fail before the swap (PL:431). | RA-9: the sentence is re-worded; ST-16 makes the swap itself fail through a seam; any failure after the read-back re-reads the look from disk. |
| A:GA-5 | SHOULD | ACCEPT | PL:576 names three places for answer 131 B; PL:440 and PL:510 also encode answer A. | RA-10: LC-9 is re-worded so that it does not depend on the answer; GL-9's bar is written for BOTH answers; the full list is in section 7. |
| A:GA-6 | SHOULD | ACCEPT | F4: one `evaluate` for every target. | RA-6: CE-3 covers a layer scalar, a source parameter and a macro. No flag: one rule for every slider is the smaller and the safer change. F18: the two fixture tests the seat named do not run the engine and do not change. |
| A:GA-7 | SHOULD | ACCEPT | PL:506 against PL:487-489 (a fresh folder per row). | RA-12: the two wired looks are written out; RU's thresholds are restated for the arm. |
| A:GA-8 | NIT | ACCEPT | F14, F15. | RA-13: PL J6's citation is corrected; `look_ui` gains `return`, which hands a Return to the open window through the peer's own key entry (no OS input); GL-5 and GL-10 use it; a Boris check is added. |
| A:ST-1 | SHOULD | ACCEPT | F8. | RA-6: said plainly that no in-app path makes a signal; the engine line stays as the stated rule; V-23 and check 6 (c) are dropped; "Ghost" stays as the machine proof of Harmony's constraint. |
| A:ST-2 | SHOULD | PARTIAL | As A:DA-8. | RA-10. Rejected: dropping the file's "signals" flag -- under the default it is what tells a first-format file from a look made with no signals, and RU-format files must stay readable. |
| A:ST-3 | SHOULD | ACCEPT | PL:249-251 and RU:110 (the refresh runs about 10 times a second): a slider dragged to an end stop sits on another look's value for many refreshes. | RA-1 (the same rule as A:DA-1); LM-22 carries "a tweak that passes through another look's settings leaves the loaded look". |
| A:ST-4 | SHOULD | ACCEPT | As A:GA-2. | RA-7. |
| A:ST-5 | SHOULD | PARTIAL | What the button shows after a tweak is his eye (RU section 6 item 5 already asks it). | RA-14: question 133 stays, default A, and its text says what A hides. Rejected as the default: the name with a mark -- it lives in the run only (PL DA-9, HD-14), so it would vanish when a show is re-opened. No tooltip. Item 8b's wrong sentence is corrected (E5). |
| A:ST-6 | SHOULD | PARTIAL | F7. | RA-6: the sentence is corrected. Rejected: leaving those kinds out of a look -- a look must equal the effect it was made from, or that effect never matches its own look. The still Clip Position wire is older than the lane: section 9. |
| A:ST-7 | SHOULD | ACCEPT | F17, F15: the model binds Return to the destructive button, and a focused first button answers Return even without a shortcut. | RA-2: in the Save Over window only a click saves; Return and Esc cancel; LM-18 clause, GL-10 step, MU-LA-36. |
Reconciled conflicts: A:DA-1 against A:ST-3 on WHEN writer (3) may act -- A:DA-1 allows it once "the named look no longer
matches", which is exactly the tweak A:ST-3 shows to be unsafe; A:ST-3's stricter condition is ruled and tightened once more: a match is taken only when a row is
built, never by the refresh timer (RA-1). A:DA-8 and A:ST-2
against the plan on question 131's default -- ruled once, in RA-10. A:DA-5 and A:ST-1 agree; A:GA-2 and A:ST-4 agree.

---------------------------------------------------------------------------------------------------------
## 3 AMENDMENTS (numbered RA-n; each OVERRIDES the plan looks-answers where they differ; "PL DA-n" is the plan's own item)
---------------------------------------------------------------------------------------------------------
RA-1 THE LOADED LOOK IS SET BY WHAT HE DID, NEVER BY WHAT THE VALUES HAPPEN TO MATCH (PL DA-9, DA-8; A:DA-1, A:ST-3).
- `EffectSlot::lookName` stands as PL DA-9 defines it: a runtime string, the LAST member, message thread only, never saved,
  copied with the slot (F12).
- The loaded look of a slot = the look of that name, if the store lists one for this effect. Else the slot has NO loaded look.
- Writers (1) and (2) stand: `EffectLookCmd` (execute and redo set it, undo puts the earlier name back, "Default" sets it
  empty); New Look and Save over, after the store has verified the file.
- Writer (3) leaves `refresh()` altogether: no timer writes `lookName`. It runs ONCE PER ROW, when the view builds the row
  (`rebuildRows`): if the slot has no loaded look and `firstMatch` names one of his looks, `lookName` becomes that name. So an
  effect that came from a saved show and still IS "Look 2" has Look 2 as its loaded look when its inspector first shows it,
  and a slider dragged through another look's settings changes nothing. A slot that HAS a loaded look is never re-pointed by
  a match -- whether or not the settings still match it, and whatever else they match.
- `looks::shownMatch(def, slot, looks, loadedName)`: the loaded look when it lists AND matches the settings; else `firstMatch`.
  The button's text and the menu's one tick use `shownMatch`. This replaces "the first look the settings match" in AM-6, AM-7
  and LM-1. The button still shows a name only while the effect IS that look: MU-EL-17 stays a mutant.
- `Save over` names the loaded look and nothing else (MU-LA-22 stays a mutant).
- On screen, said to the critics: an effect dragged exactly onto Look 1's settings while Look 2 is loaded reads "Look 1" on
  the button and `Save over "Look 2"` in the menu. Both are true; the item and its window name the look that will be replaced.
- A rename of the loaded look from this row carries the name. A delete, or a rename from another row, leaves the slot with no
  loaded look; rule (3) then takes the look it matches, if any, the next time its row is built.
Rows: LM-22 (new), LC-13 (stands), GL-10 steps (c) and (d). Mutant MU-LA-32.

RA-2 THE SAVE OVER WINDOW, AND ONE GUARD FOR BOTH WINDOWS (PL DA-8, DA-12; A:ST-7, A:DA-2).
- The window: title "Save Over Look"; text `Replace look "<name>" of <effect> with the settings on now? This cannot be undone.`;
  buttons in this order: "Save Over" (result 1; NO key; `setWantsKeyboardFocus(false)` on it) and "Cancel" (result 0; Esc AND
  Return). So the keyboard rests on Cancel, Return and Esc cancel by both key paths (F14, F15), and only a click on
  "Save Over" saves. For its keys this is NOT "the Delete Look model": that model's Return is the destructive button (F17).
- Why here and not for Delete Look: Save over sits directly under New Look, whose box teaches "click, Return"; Delete is
  reached through its own red list. RU's Delete Look window is not changed by this ruling (HD-19).
- THE GUARD, one for the name box (New Look and Rename) and for the Save Over window. The modal callback holds: a SafePointer
  to the view; the vector the view showed; the scope's four fields (F11); the effect's index; the effect's name; for Rename and
  Save over, the look's name. It writes only when the view still shows that vector with that scope, that index still holds
  that effect name, and -- Save over -- the look still lists, this build may rewrite it (RA-4) and the settings differ from
  it. If anything fails: nothing is written, nothing is shown, the window is gone.
- WHAT is saved, both windows: the effect's settings at the accept ("the settings on now"), captured after the guard passed.
Rows: LM-18 (extended), LM-23 (new), GL-10 step (c). Mutants MU-LA-36, MU-LA-31.

RA-3 THE NAME BOX: THE VERDICT ASKS THE FILE SYSTEM; THE VIEW GIVES THE FIELD THE KEYBOARD (PL DA-12; A:DA-6; E4).
- `EffectLookStore::pathTaken(effect, name)`: `juce::File::exists()` on `<folder>/<name>.look.json`. Not a directory read
  (AM-13 stands: `listFolder` is still the only enumeration) and never the cached list alone: a file that cannot be read, a
  file put there after the folder was read, and a name that differs only in letter case or in Unicode composition are all
  answered by the file system that would have to hold the new file.
- `looks::nameVerdict(trimmedName, listedNames, pathTaken, ownName)`, pure: Empty; Reserved (`isLegalName` fails); Taken when
  a listed name equals it without letter case OR `pathTaken` is true -- unless the name is the look's own name in any letter
  case (Rename); else Ok. `isLegalName` also reserves "Save over" in any letter case (PL:242).
- `EffectLookStore::nextName(effect)`: "Look N" for the smallest N whose name is not Taken by that same test.
- The view computes the verdict at every change of the field and once more at the accept. `make(effect, name, look)` still
  refuses a path that exists (AM-4 step 3): it can never overwrite.
- A `make` that fails (a full disk, a locked folder) closes the box and makes nothing; the button keeps reading "Looks"
  (AM-5). The box is not kept open: the button is the state.
- Opening: after `enterModalState` the view calls, on the field, `grabKeyboardFocus()` and `selectAll()`. Both are explicit:
  by source the window would hand the keyboard to its first button (F15). Return typed in the field still reaches the first
  button's shortcut, because the field does not swallow it (F15).
- The field's `onTextChange` sets the first button's enabled state and, for Taken, the field's text colour (the warning
  colour). No text is added anywhere.
- Titles and buttons as PL DA-12: "New Look" / "Save Look" (Return) / "Cancel" (Esc); "Rename Look" / "Rename" / "Cancel".
Rows: LK-22 (re-written), ST-2 (re-written), LM-16, LM-17, GL-5.

RA-4 LOOK FILES ARE READ STRICTLY; AN OLDER BUILD NEVER SAVES OVER A NEWER LOOK (PL DA-2, DA-10; A:DA-3).
- `looks::connVarReadable(v)` (in EffectLook.h) runs on every "conn" and "dryWetConn" BEFORE `ConnSerialization::fromVar`. It
  is true only when: v is an object; "src" is an object whose "kind" is none, signal, macro, lfo, envelope or clipPosition; a
  signal has a non-empty text "name"; a macro has a numeric "index" from 0 to 7; every number is finite; and every NAME the
  var holds -- "kind", an LFO's "shape", an envelope's "clock", each point's "interp", the shape's "playback" and "curve" --
  is written back unchanged by `toVar(fromVar(v))`. A name this build does not know fails that last test, and no second copy
  of the name tables is needed. A "conn" of kind none reads as no connection.
- A look with one unreadable connection is refused whole: not listed, not touched (PL DA-2's list, with these clauses added).
- "version" is written as 2 and still never gates READING. It gates REWRITING: `EffectLookStore::canReplace(effect, name)` is
  false for a look whose file says a version above 2, `replace` refuses it, and the menu shows `Save over "<look>"` greyed for
  it. So an older and a newer build still share the folder without damaging each other's files (RU:51-52 stays true).
- The pitfall text gains: "a build that adds a key to a look file raises its version".
- `ConnSerialization.*` is NOT edited by this lane (the one-save lane is editing the serializers, PL:260).
Rows: LK-20 (extended), ST-17 (new), LM-18. Mutants MU-LA-34, MU-LA-35.

RA-5 THE FORMAT IS PINNED BY ONE HAND-WRITTEN FILE (A:DA-4).
LK-23: a look file's text, written by hand as a literal inside tests/test_effect_look.cpp (never produced by `toVar`; a
comment above it says a change to it needs a ruling), with one connection of each source kind, their ranges, invert, curve,
playback, smoothing and enabled, and one connection that holds only its source. It must load to the exact field values, the
last one with the default shape. RED arm MU-LA-33: a key renamed in ConnSerialization.cpp, applied and reverted the way
MU-EL-28 is on EffectLibrary.cpp. Under it LK-16's round trip stays green and LK-23 does not: that is the point of the row.

RA-6 A SIGNAL THE SHOW DOES NOT HAVE: THE RULE STAYS, THE STORY AROUND IT IS CORRECTED (PL DA-4, R2; A:DA-5, A:ST-1,
A:GA-6, A:ST-6).
- PL:163-164 is replaced by: "What can be missing: a Signal named by a name no registered signal has. At 185147b no code
  path makes or removes a signal (F8), so every name the picker can write exists in every show. A missing name can come only
  from a look file written by hand or by another build, or after a built-in signal is renamed. A macro number 0..7 always
  exists. An LFO and the picker's Timeline run on the beat clock. A Clip Position source is evaluated at position 0 on every
  owner today (F7): a look stores it and replays it exactly as the show does -- it holds still."
- The engine line of PL DA-4 stands: in the Signal case of `evaluate`, id 0 returns NaN. It is one rule for every slider in
  the app; CE-3 pins the three other kinds of target. No flag is added.
- CE-4 pins the 32 built-in names: a rename turns it red before it ships.
- Nothing about it is told to Boris as something to try: check 6 (c) is replaced (section 6). V-23 is dropped: he cannot
  reach that state. PL HD-17 is withdrawn. PL R2 is replaced by section 10 R4.
- The look "Ghost" and GL-9 step (c) stay: they are the machine proof of Harmony's constraint (a look that names a signal the
  show does not have loads whole and no slider is half-plugged).
- A look names "Macro 3", "Mod 1" or "Mod 2"; what those DO in a show is not in the look (F9, FOS M-6). Told to him.

RA-7 THE AFTER-WRITE HOOK IS PROVED LIVE (PL DA-5 rule 4, R3; A:GA-2, A:ST-4).
- Settled by reading (F5, F6): the Context can be built outside the regular tick and dt = 0 is safe. The host's hook builds
  it from the registry, the macro bank, a snapshot read the way MainComponent.cpp:4222 reads it, dt 0, `now = connNow()`, the
  composition's `gripHoldMs` and `handBackGlideMs`, and the depth read as :4239 reads it; then it calls
  `ConnectionEngine::tickSlot` on that ONE slot. The hook never writes `lastConnTick_`.
- POST /api/debug/look_load answers with `live` (per parameter: a number, or null for NaN) and `dryWetLive`, read on the
  message thread in the same callback that ran the command, after it returned. No timer can run in between: the read is
  deterministic. The route takes the shape of /api/debug/ui_text (src/api/ApiServer.cpp:2073-2096: the handler posts one
  callback and waits up to 2 s for its answer), not the fire-and-forget shape of /api/debug/undo (:2367-2379).
- GL-9 step (b) reads that answer. MU-LA-27 (the host passes no hook) is its live RED arm: the twins are still NaN.
- LC-11 stays the unit proof of where the hook runs, CE-2 of what `tickSlot` does.
- PL HD-13 stays as the fall-back, for a blocker a builder finds that this reading did not.

RA-8 LC-12 AND ITS MUTANT (A:GA-3).
LC-12 compares `data()` of both arrays as well as each element's address. MU-LA-16 is pinned to the move form (a fresh vector
moved in), which changes the buffer and turns LC-12 red. The copy-assign-the-whole-vector form is forbidden by LINT-EL-3 and
is, in behaviour, MU-LA-13 (every connection is assigned): its row is LC-10.

RA-9 THE VERIFIED REPLACE (PL DA-10; A:GA-4).
- The sentence: section 0 (3).
- `EffectLookStore::replace(effect, name, look)`: (1) the look lists and `canReplace` is true, else nothing; (2) delete a
  left-over `.<name>.look.json.new`, then write that file fresh (Pitfall 46); (3) check the stream after `write` and after
  `flush()`; (4) read the new file back through the listing's reader and require the captured look (values bit for bit,
  connections `sameConn`); (5) `temp.replaceFileIn(target)` and require true; (6) read the TARGET back the same way;
  (7) only then update the list entry.
- A failure in 2-4: delete the new file, change nothing. A failure in 5 or 6: delete the new file if it is still there,
  RE-READ that look from disk into the list (the menu then shows what the disk holds, whatever it is), report failure.
- What is VERIFIED and what is not: the swap is a rename inside one folder (F13). If the rename fails JUCE tries a copy; on
  macOS that copy is INFERRED to refuse an existing target, so the old file stays. ST-16 makes the swap fail and requires the
  old bytes: it is the behavioural check on the machine the tests run on.
- The seam: `EffectLookStore::beforeSwapForTest`, a `std::function<void()>` that is empty outside tests. ST-16 uses it to
  make the folder read-only between steps 4 and 5.
- The ruling for the other outcome: if ST-16 cannot be made green with `replaceFileIn` (the old bytes change when the swap
  fails), step 5 calls the system's rename itself and treats any failure as "nothing changed"; LINT-EL-1 is re-worded to
  that one call, and Harmony is told in the stage report. No look file is ever replaced by a copy.
- LINT-EL-1 as PL:478-481: `replaceFileIn` exactly once, inside `replace`.
- A failed Save over shows nothing; the button keeps reading "Looks" (the state). PL DA-11 (no undo of a Save over) stands.

RA-10 WHAT UNPLUGS: ONE CONSTANT, BOTH POLICIES BUILT (PL DA-3, Q131; A:DA-8, A:ST-2, A:GA-5).
- `looks::resolve(def, slot, look, UnwiredEntry policy)`, with `enum class UnwiredEntry { Unplug, Keep }`. The policy decides
  two rows of the table below and nothing else. Unplug: no connection is wanted. Keep: the slot's present connection is
  wanted, so nothing is assigned.
- `looks::kUnwiredEntry` is the ONE constant every caller passes (the view, the routes). Default: Unplug. Answer 131 B
  changes that one line.
- `looks::matches` is the SAME under both policies and is strict: values within 0.0005 and, for a look that speaks about
  signals, every connection `sameConn` -- an unwired entry matches only an unplugged slider, and "Default" matches only an
  effect with no signal. Reason: New Look is offered only when nothing matches, so a lenient match would grey New Look on
  "Look 2 plus a signal" and he could never keep that as a look. A first-format look still ignores wiring (LK-17).
- The table, final:
  | the look ... | the value | the connection after the load |
  |---|---|---|
  | has an entry with a "conn" | the look's | the look's (untouched when it is `sameConn`) |
  | has an entry without "conn" (the look speaks about signals) | the look's | Unplug: none. Keep: as it is |
  | has no entry for this parameter | kept | as it is |
  | is in the first format (no "signals") | the look's | as it is |
  | is "Default" | the def's default; Dry / Wet 1.0 | Unplug: none, every slider and Dry / Wet. Keep: as it is |
- LK-18 pins Unplug and LK-24 pins Keep; LC-9 is worded for the command, not for the policy; GL-9's bar is pre-registered
  for both answers (section 5.6).
- Harmony asks question 131 WITH the adoption (HD-18). Nothing waits: S1a builds both policies either way.

RA-11 HIS DEFAULT HAS A MACHINE ROW (PL LA2; A:GA-1).
GL-10 step (b): load "Off", change it, New Look, accept -- "Off"'s bytes and modification time unchanged, the new file holds
the change, the loaded look is the new one, Save over is greyed. LM-16 gains "a look loaded before stays byte for byte and
the new look becomes the loaded one". MU-LA-30: New Look calls `replace` on the loaded look.

RA-12 GL-4'S WIRED ARM (PL DA-5, section 5.6; A:GA-7; E2).
Two WIRED looks, hand-written into the arm's own fresh folder (their content: section 5.6). "Wired" and "Wired B" hold the
same two sources on swapped sliders, so every load replaces two connections: the costly case, and the same under either
answer to 131. The arm alternates them 20 times on show C's plain Ripple. RU's thresholds are restated in the arm's own table.

RA-13 THE KEY PATH (PL J6, section 5.6; A:GA-8).
- PL J6's second sentence is replaced by F14.
- `look_ui` actions: `type:<text>` (sets the open box's field and runs its change handler); `accept` (`triggerClick()` on the
  open window's FIRST button: JUCE's own enabled test decides); `accept:<text>`; `cancel` (the same on its last button);
  `return` (NEW: `handleKeyPress` of a Return on the open window's own peer -- the entry JUCE's native layer calls; inside
  the app, no OS event, so the SCREEN-SAFETY LAW's "no synthetic input" is kept). VERIFIED:
  `ComponentPeer::handleKeyPress(const KeyPress&)` is public (juce_gui_basics/windows/juce_ComponentPeer.h:374) and walks
  from the key's target up through each parent's `keyPressed` (juce_ComponentPeer.cpp:196-221).
- GL-5: Return on a taken name leaves the box open. GL-10: Return in the Save Over window does not save.
- In the background rig the app is not the front app, so nothing may hold the keyboard and `return` reaches the window
  itself (F16): that is the window's path (F14), not the focused-button path (F15). RA-2 is built so that BOTH paths cancel;
  the focused path is Boris's check (section 6 item 11).
- NO UNIT TEST CREATES A JUCE WINDOW. The view opens the name box and the Save Over window -- and RU's Rename window and
  Delete confirm -- through ONE replaceable function it holds (`EffectStackView::lookWindowOpener`, a `std::function` whose
  default builds the `juce::AlertWindow` from the pure model in EffectLooksMenu.h). The unit rows install a recorder that
  keeps the model and lets the test accept or cancel: ctest never puts a window on his screen. The real windows are
  exercised live (GL-5, GL-10) and in the visual gate.

RA-14 THE BUTTON AFTER A TWEAK (PL DA-9, Q133, section 6 item 8b; A:ST-5; E5).
- Question 133 stands with default A; its text says what A hides (section 7).
- Item 8b's sentence is replaced by: "Look 2 now holds the change wherever you load it from now on. An effect that already
  had the old Look 2 keeps its settings; its button reads 'Looks'."

RA-15 HIS OLD FILES (PL LA4, section 6 item 14; A:DA-7).
Item 14 says what was deleted at his word and what is still on his disk and no longer opened by the app. Whether the nine
quick FX saves go too is question 134 (default: they stay). Nothing is deleted without his word. The four deck files in Decks
belong to the one-save lane (HD-20).

RA-16 CORRECTIONS TO THE PLAN'S ROWS (E1; PL section 5).
- GL-9's RED arm for MU-LA-12: the FAIL line names the first undo -- fx 1 is not Wired again. Under answer 131 A it then
  holds ONE connection where two are expected (the plan predicted 0); under B the count stays 2, which is why the check
  reads `matched`, under both answers.
- The fixtures "Off", "Far", "Wired", "Wired B" and "Ghost" are written in the CURRENT format ("version": 2,
  "signals": true). "Hand" (GL-2) stays in RU's first format: GL-2 is the live proof, and LK-17 the unit proof, of Harmony's
  constraint that files of the ruling's format stay readable.
- The command's apply function first calls `resizeParams(paramValues.size())` inside the fence (a no-op in production, F12),
  so the three arrays have one length before it writes.
- Counts: 81 new unit cases (24 + 17 + 13 + 23 + 4); ten live rows; 22 visual states.

RA-17 THE ROW AFTER A LOAD KEEPS HANDS ON (PL DA-6 last bullet; E3).
The view does not call `bindConnection` after a load. `UniversalParamControl::refreshConnectionDisplay()` -- NEW; the display
half of `bindConnection` (UniversalParamControl.cpp:134-145: source text, range, invert, repaint) -- is called on that row's
controls. A grip on a connection the load did not change is still held afterwards. `bindConnection` itself is unchanged.
Rows: LM-20 (extended). Mutant MU-LA-39.

THREADS, said once. Everything this delta adds runs on the MESSAGE thread: the store, the box, the windows, the command, the
hook, `tickSlot`, `pathTaken`'s file query. Nothing is added to the audio callback or to the analysis thread. The render
thread never waits: the fence blocks the message thread until the GL thread has finished its frame (RU AM-9), never the other
way round. No lock and no new mutex (LINT-EL-4). Copying a connection allocates on the message thread, inside the fence;
GL-4's second arm measures what that costs.
TEXTS, said once. The delta's only new words on screen: the menu items `Save over "<look>"` and "Save over"; the box's titles
"New Look" and "Rename Look" and its buttons "Save Look", "Rename", "Cancel"; the window "Save Over Look" with its one
sentence and its buttons "Save Over" and "Cancel". Each is shown only by his own menu choice; none announces an event or a
failure (LINT-EL-6). The lane still adds no slider, and its menu is still opened with `showMenuAsync` on the top-level parent
(LINT-EL-2).

What of the plan stands untouched: PL DA-1 (what a look holds), PL DA-2's three additive keys and the first-format rule,
`sameConn` (PL:156-158), PL DA-5 rules 1-3, PL DA-6 except its last bullet, PL DA-7 (a take), PL DA-8's menu order and
texts, PL DA-11 (no undo of a Save over), PL DA-12's box except as RA-3 says, PL LA4 (101, 106), PL section 9 (not in the lane).

---------------------------------------------------------------------------------------------------------
## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
---------------------------------------------------------------------------------------------------------
One lane (`lane/effect-looks`, one worktree), as RU section 4. Each stage is ONE builder context, ends with the full ctest
green, shows its own unit rows RED first (the named mutant) and then GREEN in its report, and is reviewed pinned before the
next starts. A builder never launches the app, never runs a live row, never gives a gate verdict. Docs move in the stage that
changes the behaviour. This table REPLACES RU's stage table and PL section 4's.

| stage | owns (files) | proves (unit, by the builder) | Harmony runs herself, and when |
|---|---|---|---|
| S1a the look (headless) | NEW src/effects/EffectLook.h (the look with a connection per entry; `capture`, `resolve` with the policy, `kUnwiredEntry`, `matches`, `firstMatch`, `shownMatch`, `sameConn`, `connVarReadable`, `isLegalName`, `nameVerdict`, `toVar` / `fromVar`); NEW tests/test_effect_look.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp). Touched ONLY as mutants, reverted: src/effects/EffectLibrary.cpp (MU-EL-28), src/connect/ConnSerialization.cpp (MU-LA-33). | LK-12 first, then LK-1..LK-24; LINT-EL-4; their mutants RED | BEFORE S1a: `ctest -N` on the lane's base (N0 of GL-8). After S1a: nothing live; the pinned review. |
| S1b the store (headless; after S1a) | NEW src/effects/EffectLookStore.h / .cpp (`listFolder`, `looksFor`, `make(effect, name, look)`, `nextName`, `pathTaken`, `rename`, `remove`, `replace`, `canReplace`, `beforeSwapForTest`, `defaultRoot`, `testModeRoot`); CMakeLists.txt (one source); NEW tests/test_look_store.cpp; tests/CMakeLists.txt (one target, which also links src/connect/ConnSerialization.cpp) | ST-1..ST-17; LINT-EL-1, LINT-EL-4; their mutants RED | Nothing live; the pinned review. |
| SE the wire that drives nothing, and `tickSlot` (shares no file with S1a or S1b: any order before S2; side by side only in a worktree of its own) | src/connect/ConnectionEngine.h / .cpp (the one line of RA-6; `static void tickSlot(Clip::EffectSlot& fx, const Context& ctx)`, the body of the per-slot loop at ConnectionEngine.cpp:253-288 made public); tests/test_connection.cpp (four cases appended); docs/claude/rendering.md (the Mapping System section gains two sentences: a Signal connection whose name no signal has drives nothing; `tickSlot`). Touched ONLY as a mutant, reverted: src/signal/SignalRegistry.cpp (MU-LA-29). | CE-1..CE-4; their mutants RED | After the pinned review: the regression probes of RU (.harmony/probe-effects-parity.sh, .harmony/probe-deck-path.sh, the gate A set), GREEN only. SE's live proof is GL-9, after S2. |
| S2 the load, undo, the host, the data routes, the probe (after S1b AND SE) | NEW src/core/EffectLookCmd.h (values, Dry / Wet, connections, the loaded-look name; one apply function obeying PL DA-5 rules 1-4; the after-write hook); src/model/Clip.h (ONE member, `lookName`, last in `EffectSlot`); src/MainComponent.h / .cpp (the store member, `effectLooksDir`, `performLookLoad` with the hook of RA-7, the route callbacks); src/api/ApiServer.h / .cpp (inside the test-only block: GET /api/debug/looks with `conn`, `live`, `dryWetConn`, `dryWetLive`, `loaded`; POST look_make with "name"; POST look_load whose answer carries `live`; NEW POST look_replace); NEW tests/test_effect_look_cmd.cpp; tests/CMakeLists.txt; NEW .harmony/probe-effect-looks.sh with its fixtures (shows A, B, C; the looks of section 5.6) and a selftest (rows GL-1, 2, 3, 4, 6, 7, 9; quits only its own pid); docs: docs/claude/effects.md (new section "Looks per effect"), docs/claude/pitfalls.md (entry NN, text below), docs/claude/testing-eyes.md (the routes), docs/claude/architecture.md (the new files) | LC-1..LC-13, LINT-EL-3, LINT-EL-5, LINT-EL-6, LINT-EL-7; their mutants RED | After the pinned review: GL-1, GL-2, GL-3, GL-4 (both arms), GL-6, GL-7, GL-9, each with its RED arm. GL-4 and GL-6 are measurements with decision tables: a STOP outcome stops the lane before S3. |
| S3 the menu, the name box, Save over | NEW src/ui/LooksButton.h; NEW src/ui/EffectLooksMenu.h (the pure models: the menu with the Save over item, the box, the Save Over window's keys; `LookNameFilter`); src/ui/EffectStackView.h / .cpp (as RU, plus the name box for New Look and Rename, the Save Over window, the guard of RA-2, writer (3) of RA-1 in `rebuildRows`, the one window opener of RA-13, the display refresh of RA-17); src/ui/UniversalParamControl.h / .cpp (ONE method, `refreshConnectionDisplay`); src/ui/InspectorPanel.h / .cpp and the three inspector headers (forwarders only); src/MainComponent.cpp (the wiring lines, the `look_ui` callback); src/api/ApiServer.h / .cpp (`look_ui` with type, accept, accept:<text>, cancel, return; GET looks gains `menu.saveOver`, `box`, `confirm`); NEW tests/test_effect_looks_menu.cpp; tests/CMakeLists.txt; .harmony/probe-effect-looks.sh (rows GL-5, GL-10); docs: CLAUDE.md (the capability paragraph, UI Patterns "Looks button", the pitfall index), docs/claude/effects.md (the menu, the box, Save over), .harmony/APP-INVENTORY.md | LM-1..LM-23, LINT-EL-2; their mutants RED | GL-5 and GL-10 with their RED arms; GL-8; the regression probes; then GL-1, GL-2, GL-3, GL-4, GL-7, GL-9 once more at the lane's final head (GREEN only). |
| VG the visual gate | a capture builder (window-id captures only, driven through `look_ui`; 22 states, each with a manifest of model facts), then FIVE critic seats, as RU | -- | The verdict. A fix round goes back to S3's files. Boris sees nothing of this lane before it passes. |

What Harmony runs herself, in order: (0) with the adoption: questions 131-134 to Boris, and the `ctest -N` baseline; (1) after
SE's review: the regression probes; (2) after S2's review: the seven live rows above; (3) after S3's review: GL-5, GL-10, GL-8,
the regression probes and the re-run; (4) the visual gate; (5) the merge sequence of RIG-RULES B (RED on the pre-merge copy,
merge, build, ctest, GREEN). Every live row takes the live lock; none runs while another lane's perf A/B is in progress.
Order: (one-save S7, in its own lane, any time) S1a -> S1b, and SE any time before S2 -> S2 -> S3 -> VG -> merge.
S1a may start first once this ruling is adopted. It does not start unchanged: the format, `make`'s signature and the row
lists are this ruling's. The S1a and S1b packets are written from RU sections 4 and 5 with THIS section 5 laid over them.
Why S1 is cut in two: 41 unit rows and about 40 mutants are more than one builder context should carry, and the store
needs the look, never the other way round (HD-16).

Pitfall NN (Harmony assigns the number, HD-4), text for S2 -- it REPLACES RU:449-458:
"A look is one effect's BASE values (`paramValues`, `dryWet`) AND the connection on each slider and on Dry / Wet (source,
shape, enabled -- never a grip, engine state or a live twin), by NAME -- uniform first, then label -- in one file per look:
`~/Library/Audio-DNA/Looks/<effect display name>/<look name>.look.json`; the file name is the look's name. A file without
"signals": true holds values only and touches no connection. A look file is never edited in place: make writes a new file and
reads it back before listing it; rename renames the file; delete removes it; Save over writes a complete new file beside the
old one, reads it back, swaps it in with one rename and reads the look back from its own name -- if a step fails the old look
stays, whole. The store never deletes or rewrites a file it could not read, never rewrites a look whose version is above its
own, and reads an effect's folder the first time that effect's looks are asked for. A build that adds a key to a look file
raises its version. Every look in a menu is a file on disk. A load is `EffectLookCmd` -- one slot's values, Dry / Wet and
connections inside one fence: a connection equal to the wanted one is NEVER re-assigned (assignment clears its grip and
memory), a changed or unplugged one has its live twin cleared, and the slot is ticked once before the fence ends; never
`bypassed`, never a layer field. A Signal connection whose name no signal has drives nothing: the slider rests on its own
value. The button's text is derived from values and wiring; the loaded look (`EffectSlot::lookName`: runtime, message thread,
never saved) is set by a load, New Look or Save over, and by a match only when a row is built for a slot that has none. The button never takes the
keyboard. Renaming an effect's display name orphans its Looks folder and every saved show that names it (LK-13); renaming a
built-in signal stops every look and show that names it from being driven (CE-4). Changing what a uniform's 0..1 means
changes every saved look and show: give the parameter a new uniform name instead."

Order against the lanes planned beside this one (RU:460-470 stands, plus)
- ONE-SAVE edits the three serializers. This lane edits none of src/model/Clip.cpp, Layer.cpp, model/Composition.h or
  src/connect/ConnSerialization.*; it adds ONE runtime member to src/model/Clip.h and no key to the show. LK-23 is the alarm
  if that lane renames a connection key; LINT-EL-7 proves `lookName` is never saved.
- SE edits src/connect/ConnectionEngine.cpp: any lane that touches the engine's tick (the nudge lane shifts the beat it
  reads) re-bases as a builder's step 0, whichever merges second.
- S3 edits src/ui/UniversalParamControl.*: one added method, no change to an existing one.

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; exact strings and bars; each row's RED arm; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------------
This section REPLACES PL section 5 and is laid over RU section 5: a row of RU that is not named here stands exactly as RU
writes it. Every row is RED first. One TEST_CASE per row id, named exactly as written (GL-8's count leans on it). A mutant is
applied to the working tree, shown RED, reverted, and the restore rebuild is checked (RIG-RULES A). Harmony copies gate strings
ONLY from this section and from RU section 5.

5.1 tests/test_effect_look.cpp, tag [looks] (S1a; 24 rows = RU's 15, two of them re-written, + 9)
| id | exact test name | RED arm |
|---|---|---|
| LK-6 (re-written) | "LK-6 resolve gives values, Dry/Wet and connections only: bypassed, enabled, the name and both live twins are untouched" | MU-EL-4 |
| LK-15 (re-written) | "LK-15 a look name is legal when it is 1 to 40 characters, has none of the nine file characters or a control character, does not start with a dot, and is not Default, Looks, New Look, Save over, Rename or Delete in any letter case" | MU-EL-29 |
| LK-16 | "LK-16 a look with signals round-trips: a signal by name, a macro by index, an LFO, a timeline with its points and a clip position, each with range, invert, curve, smoothing and enabled, on parameters and on Dry/Wet" | MU-LA-1 |
| LK-17 | "LK-17 a file in the first format holds values only: resolve keeps every connection and matches ignores them" | MU-LA-2 |
| LK-18 | "LK-18 resolve with the Unplug policy: a wired entry gives the look's connection; an unwired entry unplugs; a parameter the look has no entry for keeps its value and its connection; Default unplugs every parameter and Dry/Wet" | MU-LA-3 |
| LK-19 | "LK-19 matches with signals: equal values with another signal, range, invert or enabled do not match; two unplugged connections match whatever range was left behind" | MU-LA-4 |
| LK-20 | "LK-20 fromVar refuses the whole file for a connection it cannot read: conn not an object, an unknown source kind, an unknown LFO shape, curve, playback, clock or point interpolation name, a signal with no name, a macro with no index or an index outside 0..7, a number that is not finite" | MU-LA-5, MU-LA-34 |
| LK-21 | "LK-21 sameConn compares what is saved and nothing else: a grip and engine state never make two connections differ" | MU-LA-6 |
| LK-22 | "LK-22 nameVerdict: empty and reserved are refused; a name is taken when a listed look has it in any letter case or when its path exists; a look's own name in another letter case is free for its own rename" | MU-LA-7, MU-LA-38 |
| LK-23 | "LK-23 a hand-written look file of the current format loads to exact fields: a signal by name, a macro by index, an LFO, a timeline with its points and a clip position, each with its range, invert, curve, playback, smoothing and enabled, and a connection that holds only its source keeps the default shape" | MU-LA-33 |
| LK-24 | "LK-24 resolve with the Keep policy: a wired entry gives the look's connection; an unwired entry and Default keep whatever is plugged; a slot with a signal the look does not hold still does not match it" | MU-LA-37 |

5.2 tests/test_look_store.cpp, tag [lookstore] (S1b; 17 rows = RU's 12, two of them re-written, + 5; a temp folder per case)
| id | exact test name | RED arm |
|---|---|---|
| ST-2 (re-written) | "ST-2 nextName gives Look 1, Look 2; a freed number is used again; a number taken by a listed look in another letter case, by an unreadable file, or by a file that appeared after the folder was read is skipped; make under a given name makes that file, and under a taken name makes nothing and never overwrites" | MU-EL-30, MU-LA-8 |
| ST-10 (re-written) | "ST-10 a file with unknown keys and a version above this build's lists, and no operation on another look changes its bytes" | MU-EL-34 |
| ST-13 | "ST-13 replace swaps a whole verified file in: the look then lists with the new values and connections; a second store on the folder reads the same; a replace that cannot write, or whose new file does not read back equal, leaves the old file's bytes unchanged" (a writer hook that truncates; a read-only folder) | MU-LA-9, MU-LA-26 |
| ST-14 | "ST-14 the new file's name is never listed: a left-over from a crash is not a look, and the next replace of that look succeeds and leaves none" | MU-LA-10 |
| ST-15 | "ST-15 replace touches one file: every other look's bytes and modification time are unchanged; a name that does not list is not replaced and no file is made" | MU-LA-11 |
| ST-16 | "ST-16 a swap that fails leaves the old look: with the folder made read-only between the read-back and the swap, replace reports failure, the old file's bytes are unchanged, the list still holds the old look, and a later replace succeeds" (through `beforeSwapForTest`) | MU-LA-28 |
| ST-17 | "ST-17 replace refuses a look whose file says a version above this build's: canReplace is false and its bytes are unchanged" | MU-LA-35 |

5.3 tests/test_effect_look_cmd.cpp, tag [lookcmd] (S2; 13 rows = RU's 8, two of them re-written, + 5)
| id | exact test name | RED arm |
|---|---|---|
| LC-2 (re-written) | "LC-2 a load touches one slot: every other slot's values, connections, grips and engine states are bit-equal" | MU-EL-13 |
| LC-7 (re-written) | "LC-7 a first-format look on a connected parameter keeps its connection, its grip and its live twin; after one engine tick its effective value is the signal's and the look's value is the base underneath" | MU-EL-37 |
| LC-9 | "LC-9 a look with signals is one undo step: execute makes the slot's connections the resolved ones, undo puts the earlier connections back, redo applies again" | MU-LA-12 |
| LC-10 | "LC-10 a connection equal to the wanted one is not assigned: its grip, smoothing memory and once-pin survive execute, undo and redo; a different one is replaced and starts fresh" | MU-LA-13 |
| LC-11 | "LC-11 an unplugged or replaced parameter's live twin is cleared inside the fence, and the after-write hook runs once per execute, undo and redo, inside the fence, after the write, with that slot" | MU-LA-14, MU-LA-15 |
| LC-12 | "LC-12 the connection arrays are written in place: data() of both arrays and the address of every connection and twin are the same before and after execute, undo and redo" | MU-LA-16 |
| LC-13 | "LC-13 the loaded-look name follows the command: execute sets it, undo puts the earlier name back, redo sets it again; Default sets it empty" | MU-LA-17 |

5.3b tests/test_connection.cpp, four cases appended (SE)
| id | exact test name | RED arm |
|---|---|---|
| CE-1 | "CE-1 a signal connection whose name the registry does not hold drives nothing: evaluate gives NaN and the twin reads the manual value; once a signal of that name is added it drives at the next tick" | MU-LA-18 |
| CE-2 | "CE-2 tickSlot publishes one slot's twins, parameters and Dry/Wet, and leaves every other slot's twin and connection state unchanged" | MU-LA-19 |
| CE-3 | "CE-3 an unknown signal drives nothing on every kind of target: a layer scalar and a source parameter read their manual value through a NaN twin, and a macro keeps its manual value" | MU-LA-18 |
| CE-4 | "CE-4 the registry's built-in signal names are the pinned 32" (the failure text: "a built-in signal was renamed or removed: every saved show and look that names it stops being driven -- restore the name") | MU-LA-29 |

5.4 tests/test_effect_looks_menu.cpp, tag [looksmenu] (S3; 23 rows = RU's 15, two of them re-written, + 8; handlers are called
directly, never OS input; no test opens a window on the screen)
| id | exact test name | RED arm |
|---|---|---|
| LM-1 (re-written) | "LM-1 the menu lists Default, his looks in natural order, New Look, Save over, Rename, Delete; at most one item is ticked, the look the button names" | MU-EL-40 |
| LM-4 (re-written) | "LM-4 New Look is enabled only when the settings match no look, Default included; after the box is accepted the button reads the new look's name; after Cancel, and after a make that fails, nothing changes and no file exists" | MU-EL-42 |
| LM-16 | "LM-16 New Look opens the name box holding the next free Look N with all of it selected; accepting it unchanged makes Look N; accepting another name makes that look; the look holds the settings at the accept, not at the opening; a look loaded before stays byte for byte and the new look becomes the loaded one" | MU-LA-20, MU-LA-30 |
| LM-17 | "LM-17 the box's first button is greyed while the name is empty, reserved or taken, the taken name is drawn in the warning colour, and an accept in that state makes nothing and leaves the box open" | MU-LA-21 |
| LM-18 | "LM-18 Save over names the look last loaded or made on this effect: greyed and bare with none, greyed while the settings equal it, greyed for a look a newer build wrote, live after a tweak; in its window Save Over has no key and does not take the keyboard while Cancel has Esc and Return; confirming replaces that look and the button reads its name; Cancel changes nothing" | MU-LA-22, MU-LA-36 |
| LM-19 | "LM-19 Rename uses the same box: it holds the present name selected, greys the same way, and a rename of the loaded look keeps Save over pointing at it" | MU-LA-23 |
| LM-20 | "LM-20 after a load of a look with signals the row's source buttons, ranges and invert marks show the new wiring, the row is neither rebuilt nor folded, and a grip on a connection the load did not change is still held" | MU-LA-24, MU-LA-39 |
| LM-21 | "LM-21 every close of the name box and of the Save Over window calls the focus-home hook once; while either is open the looks button still does not want the keyboard" | MU-LA-25 |
| LM-22 | "LM-22 the loaded look is never moved by a match: with Look A and Look B equal, loading B makes the button and the tick read B and Save over name B; a tweak that passes through another look's settings leaves the loaded look; only an effect with no loaded look takes the look it matches, and only when its row is built" | MU-LA-32 |
| LM-23 | "LM-23 a name-box accept or a Save Over confirm that arrives after the view shows another chain, another effect at that index, or after the look was deleted writes nothing" | MU-LA-31 |
The window models (which button has which key, which wants the keyboard, what is greyed) are pure data in
src/ui/EffectLooksMenu.h and are what LM-17 and LM-18 assert; the view builds the JUCE windows from them. The JUCE key paths
themselves are exercised live (GL-5, GL-10) and by Boris (section 6 item 11). No unit row creates a JUCE window: the rows
install a recorder in place of the view's window opener (RA-13).

5.5 The mutants (the change; the rows it must turn red) and the lints
RU's MU-EL-1..MU-EL-48 stand as written; MU-EL-30 also arms the re-written ST-2. The delta's mutants:
MU-LA-1 `toVar` omits "conn" (LK-16). MU-LA-2 a file without "signals" is read as speaking about signals (LK-17). MU-LA-3
under Unplug an unwired entry keeps the connection (LK-18). MU-LA-4 `matches` ignores connections (LK-19). MU-LA-5 a
connection that is not an object loads as none (LK-20). MU-LA-6 `sameConn` compares the grip (LK-21). MU-LA-7 `nameVerdict`
compares the listed names with letter case (LK-22). MU-LA-8 `make` ignores the given name and uses `nextName` (ST-2).
MU-LA-9 `replace` writes straight onto the target (ST-13). MU-LA-10 the new file's name ends in ".look.json" (ST-14).
MU-LA-11 `replace` of a name that does not list makes the file (ST-15). MU-LA-12 undo restores values but not connections
(LC-9; GL-9). MU-LA-13 `apply` assigns every connection (LC-10). MU-LA-14 unplugging resets the source only and keeps the
twin (LC-11). MU-LA-15 the hook is called before the write (LC-11). MU-LA-16 `apply` moves a fresh vector in (LC-12).
MU-LA-17 undo leaves the loaded-look name (LC-13). MU-LA-18 the engine line of RA-6 removed (CE-1, CE-3; GL-9). MU-LA-19
`tickSlot` skips the Dry / Wet connection (CE-2). MU-LA-20 the box opens empty (LM-16). MU-LA-21 the verdict treats a taken
name as free (LM-17; GL-5). MU-LA-22 Save over names what `firstMatch` names (LM-18; GL-10). MU-LA-23 a rename does not carry
the loaded-look name (LM-19). MU-LA-24 no display refresh after a load (LM-20). MU-LA-25 the focus-home call removed from the
box's callback (LM-21). MU-LA-26 `replace` only updates the list (ST-13; GL-10). MU-LA-27 the host passes no after-write hook
(GL-9). MU-LA-28 `replace` ignores the swap's result and updates the list (ST-16). MU-LA-29 the registry registers "Air" as
"Air 2" (CE-4). MU-LA-30 New Look calls `replace` on the loaded look (LM-16; GL-10). MU-LA-31 the windows' guard compares the
index only (LM-23). MU-LA-32 `refresh()` sets the loaded look to the derived name whenever one matches (LM-22; GL-10).
MU-LA-33 ConnSerialization writes and reads "smoothingMs" in place of "smoothMs" (LK-23). MU-LA-34 the write-back name test
removed from `connVarReadable` (LK-20). MU-LA-35 the version test removed from `canReplace` (ST-17). MU-LA-36 the Save Over
button keeps Return as its key (LM-18; GL-10). MU-LA-37 under Keep an unwired entry unplugs (LK-24). MU-LA-38 `nameVerdict`
ignores the path test (LK-22). MU-LA-39 the display refresh goes through `bindConnection` (LM-20).
Lints (static; run by the builder, re-run by Harmony; each shown RED on a seeded line)
- LINT-EL-1 (re-worded) "the Looks folder is spelled once: `grep -rn 'getChildFile *(\"Looks\")' src` prints exactly one line, in
  src/effects/EffectLookStore.cpp; EffectLookStore.cpp names no replaceWithText, replaceWithData, TemporaryFile; replaceFileIn
  appears exactly once, inside `replace`; findChildFiles / RangedDirectoryIterator / DirectoryIterator appear in the new files
  only inside `listFolder`".
- LINT-EL-3 (re-worded) "src/core/EffectLookCmd.h names no bypassed, opacity, solo, mute, runtime( and no `.enabled` of a slot;
  it assigns no vector of connections or twins whole (no `paramConns =`, no `paramLive =`)".
- LINT-EL-7 (new) "`grep -rn lookName src/render src/model/Clip.cpp src/model/Layer.cpp src/model/Composition.h` prints
  nothing: the loaded-look name is never read by the renderer and never saved".
- LINT-EL-2, -4, -5, -6 stand. LINT-EL-6 holds with the name box and the Save Over window: both are built with
  `new juce::AlertWindow` and `enterModalState`, never a `show` call.

5.6 Harmony's live rows (.harmony/probe-effect-looks.sh). The rig is RU 5.6's, unchanged: a test-server build; `open -g`;
--test-mode; a FRESH AUDIODNA_LOOKS_DIR per row and per arm; the live lock; no Output window; no full-screen capture; no
synthetic input; HTTP with Connection: close; quits only its own pid. A mutant arm is a normal cmake build of the mutant.
Routes (amends AM-11; all inside `#if AUDIODNA_TEST_SERVER`)
- GET /api/debug/looks also returns, per `values` entry, `conn` (the connection as the show writes it, or null) and `live`
  (the twin: a number, or null when it is NaN); `dryWetConn`, `dryWetLive`; `loaded` (the loaded-look name or ""); and after
  S3 `menu.saveOver` = {text, enabled}, `box` = {open, title, text, selected, acceptEnabled, taken} and `confirm` = {open,
  title}. After S3 it first runs that stack view's `refresh()`, so a read never races the 10 Hz timer.
- POST /api/debug/look_make takes an optional "name" (absent = `nextName`). POST /api/debug/look_load answers with `live` and
  `dryWetLive` (RA-7). NEW POST /api/debug/look_replace {scope, layer, column, fx, look}.
- POST /api/debug/look_ui gains type:<text>, accept, accept:<text>, cancel, return (RA-13). A click and a window's close
  are delivered as queued messages, so the probe never sleeps blindly: a step that must close a window is followed by reads
  of GET looks until `box.open` / `confirm.open` is false (at most 1 s); a step that must NOT close it is followed by two
  reads 200 ms apart, both open.
Fixtures, pre-registered (the probe's header names the files; S2 commits them before Harmony's first run)
- Shows A and B: as RU 5.6. Show C: show B's picture, PLAYING, on layer 0 column 0, with clip effects Ripple (fx 0: intensity
  driven by the signal "Bass" with range 0.20..0.80; speed driven by an LFO, Sine, 1 Beat; freq 0.80, so that fx 0 is not
  Default and, after a Default that keeps signals, is not "Wired" either) and Ripple (fx 1, plain defaults).
- In RU's FIRST format (no "signals" key), hand-written: Ripple / "Hand", exactly as RU 5.6 writes it.
- In the CURRENT format ("version": 2, "signals": true), hand-written, each with Dry/Wet 1.0: Invert / "Off": amount 0.0, no
  connection.
  Hue Shift / "Far": amount 0.8, no connection. Ripple / "Wired": Dry/Wet 1.0; intensity 0.50 with a connection to the signal
  "Bass", range 0.20..0.80; speed 0.50 with an LFO, sine, 1 beat, range 0.00..1.00; freq 0.50, no connection.
  Ripple / "Wired B": the same two connections on swapped sliders (intensity: the LFO; speed: "Bass" 0.20..0.80); freq 0.50.
  Ripple / "Ghost": intensity 0.35 with a connection to the signal "No Such Signal", range 0.60..0.90; speed 0.50 and
  freq 0.50, no connection.
- A probe that needs "Wired" made by the app (GL-9 step (a)) makes it there; GL-4's arm uses the hand-written two.

| row | does | bar (the exact line printed on pass) | RED arm |
|---|---|---|---|
| GL-1 (re-written) | as RU, with look_make "name": "Wobble" | "GL-1 PASS Wobble on disk at make (segments 0.6100 rotation 0.2700); listed and loaded on a layer in another show after a relaunch; windows unchanged; file label unchanged" | MU-EL-8 |
| GL-2, GL-3, GL-7 | stand as RU writes them ("Hand" stays a first-format file: the live proof that such files stay readable) | as RU | as RU |
| GL-4 | RU's table and line stand for the first arm. SECOND ARM: the table below | the two tables' lines | the control arm of each table |
| GL-5 (re-written) | Show A with "Far" pre-written, after S3. look_ui on layer 0, fx 1 (the SECOND Hue Shift): pick:New Look; GET looks. type:far; GET looks. return; GET looks. accept:Mine; GET looks; read the file. pick:Far on fx 1; GET looks for fx 0 and fx 1; GET looks?stats=1. | "GL-5 PASS menu pick on layer fx 1: the box opened on Look 1, selected; far greyed the button (taken) and Return left the box open; Mine stored 0.5500; Far set fx 1 to 0.8000; fx 0 kept 0.3000; focus home +3" | MU-EL-23 ("Mine stored 0.3000"); MU-LA-21 ("far did not grey the button") |
| GL-6 (one read added) | RU's table stands, plus one row: keys named conn, conns or dryWetConn anywhere in take.json -- predicted 0; if not, STOP and report | "GL-6 MEASURED control lanes=<n> look lanes=<n> stop-state=<equal|differs> connections=<n>" | the control arm |
| GL-8 (re-written) | full ctest at the lane's final head | "GL-8 PASS ctest <N> = baseline <N0> + 81 new cases, 0 failed" (81 = 24 + 17 + 13 + 23 + 4) | the lane's base: N0 is not N0 + 81 |
| GL-9 (new) | Show C with "Ghost" pre-written. (a) look_make "Wired" on fx 0; read the file. (b) look_load "Wired" on fx 1 and read ITS OWN ANSWER; GET looks fx 1. (c) look_load "Ghost" on fx 1; GET looks. (d) /api/debug/undo; GET looks. (e) /api/debug/undo; GET looks. (f) look_load "Default" on fx 0; GET looks. | answer 131 A: "GL-9 PASS wired look: 2 connections stored; plugged on fx 1 with 2 live values in the load's own answer, matched Wired; Ghost rests at 0.3500 (live none), matched Ghost; undo gave 2 connections back, then 0; Default unplugged 2 on fx 0". Answer 131 B: "GL-9 PASS wired look: 2 connections stored; plugged on fx 1 with 2 live values in the load's own answer, matched Wired; Ghost rests at 0.3500 (live none), speed kept its LFO, matched none; undo gave 2 connections back, then 0; Default kept 2 on fx 0" | MU-LA-12 (the FAIL line names the first undo: fx 1 is not Wired again -- under answer A it holds 1 connection where 2 are expected); MU-LA-18 (the FAIL line names the Ghost check: a live value where none is expected -- 0.6000 at full signal depth); MU-LA-27 ("0 live values in the load's own answer") |
| GL-10 (new) | Show B with "Off" and "Far" pre-written, after S3; fx = the clip's Invert. (a) look_load "Off"; POST /api/set_param Invert amount 0.25; GET looks. (b) look_ui pick:New Look; GET looks; accept; GET looks; read "Look 1"'s file; compare Off's bytes and modification time. (c) look_load "Off"; set_param amount 0.25; GET looks. look_ui pick:Save over "Off"; GET looks; return; GET looks; read Off's file. look_ui pick:Save over "Off"; accept; GET looks; read Off's file; list the folder; compare Far's and Look 1's bytes and times. (d) Quit own pid, relaunch on show B; look_load "Off"; GET looks. | "GL-10 PASS changed look: after the tweak New Look and Save over \"Off\" were offered; New Look made Look 1 (0.2500), left Off's bytes and time unchanged and became the loaded look; with Off loaded and tweaked again the item still read Save over \"Off\"; Return did not save; Save Over replaced Off 0.0000 -> 0.2500, matched Off; no new-file left; Far and Look 1 untouched; held after a relaunch" | MU-LA-26 ("after a relaunch 0.0000"); MU-LA-22 ("Save over not offered"); MU-LA-30 ("Off's bytes changed at New Look"); MU-LA-32 ("the item read Save over \"Look 1\""); MU-LA-36 ("Return saved") |
What GL-10 reads at each step, so that the line cannot pass by accident: (a) `matched` "", `loaded` "Off", `menu.newLook`
enabled, `menu.saveOver` = {`Save over "Off"`, enabled}. (b) `box` = {open, "New Look", "Look 1"}; after accept `matched` and
`loaded` "Look 1", `menu.saveOver` = {`Save over "Look 1"`, greyed}. (c) the settings now equal Look 1: `matched` "Look 1",
`loaded` STILL "Off", `menu.saveOver` = {`Save over "Off"`, enabled}; after `return` the window is closed, Off's bytes are
unchanged and `matched` is still "Look 1"; after `accept` Off and Look 1 are equal and `matched` is "Off" -- the loaded look
wins the tie (RA-1). (d) amount 0.2500.
What GL-9 reads at each step: (a) the file "Wired.look.json": "signals": true, freq 0.8000, and two entries with a "conn"
(intensity: the signal "Bass", 0.20..0.80; speed: the LFO). (b) the load's own answer: `live` holds 2 numbers; then `matched` and `loaded`
"Wired". (c) intensity: `value` 0.3500, `conn` the signal "No Such Signal", `live` null; under answer A speed has no `conn`
and `matched` is "Ghost"; under answer B speed still has the LFO and `matched` is "". (d) after the first undo: `matched`
"Wired", 2 connections. (e) after the second: 0 connections, `matched` "Default". (f) on fx 0, under answer A: 0 connections,
`matched` "Default"; under answer B: 2 connections, `matched` "".

GL-4, SECOND ARM, pre-registered decision table (a measurement). Show C playing; "Wired" and "Wired B" hand-written into this
arm's own fresh folder. Read fence_hold_frames and fence_black_frames (GET /api/state). Control: 2 s with no load, read again.
Then 20 look_loads on fx 1 (the plain Ripple), "Wired" and "Wired B" in turn, 100 ms apart; after the last GET looks has
answered, read again.
| read | ruling |
|---|---|
| control: either counter moved with no load | the instrument is wrong: no conclusion, fix the probe |
| the 20 loads did not alternate `matched` between "Wired" and "Wired B" | the instrument is wrong: no conclusion |
| black moved by more than 0 | STOP and report: a fenced frame found no canvas. S3 does not start. |
| black 0, hold at most 40 (2 per load) | a load that replaces two signals costs what a plain load costs; section 6 item 4 stands as written |
| black 0, hold above 40 | the fence stays for now; Harmony decides HD-8 with the number; section 6 item 4 names the number |
Printed line: "GL-4 MEASURED wired loads=20 hold=<n> per_load=<x.xx> black=<n> control=<n>/<n>".
Regression, unchanged and green: .harmony/probe-effects-parity.sh, .harmony/probe-deck-path.sh, the gate A set.

5.7 The visual gate: 22 states (RU's rules: window-id captures only, each with a manifest of model facts; 1280 x 720 unless said)
Stand as RU writes them: V-1, V-2, V-3, V-7, V-8, V-10, V-11, V-12, V-13, V-14, V-15, V-16, V-18.
Re-captured with new content: V-4 the first menu he sees: Default ticked; New Look, "Save over", Rename, Delete greyed.
V-5 three looks, one loaded and ticked: New Look greyed, `Save over "<that look>"` greyed, Rename and Delete live. V-6 a loaded
look tweaked: nothing ticked, New Look live, `Save over "Look 2"` live. V-9 the name box for Rename: the present name,
selected. V-17 an effect after a WIRED look was loaded: two sliders show their sources (manifest: each connection, each twin).
New: V-19 the name box for New Look as it opens: "Look 3", selected. V-20 the box with a taken name: the first button greyed,
the name in the warning colour. V-21 the Save Over window. V-22 the menu for an effect whose loaded look was deleted:
"Save over" bare and greyed.
Dropped: the plan's V-23 (RA-6). If Boris answers 133 B: one more state, the button with its mark.
The critics are also told: the row's source button, its range sliders and its invert mark pre-date the lane; the button can
read "Look 1" while the menu says `Save over "Look 2"` (RA-1); a look that plugs signals in is still ONE click.
NOT capturable in the rig, said so: which control holds the keyboard as the box opens, and the caret (the rig runs the app in
the background); a sub-list hanging off its parent item; hover colours. Boris's items 1, 3 and 11 cover the first.

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; this list REPLACES RU section 6 and PL section 6)
---------------------------------------------------------------------------------------------------------
1. Put an effect on a clip: its small button reads "Default". Move a slider: it reads "Looks", dim. Press it and choose
   New Look -> a small box opens holding "Look 1", already selected, and you can type at once. Press Return -> the button reads
   "Look 1". Or type a name first, then Return -> the button reads that name. Esc -> no look -> wrong: you must click the box
   before you can type; typing adds to "Look 1" instead of replacing it; a look appears after Esc.
2. Quit, start the app again, open a different show, put the same effect on a LAYER, open its menu -> "Look 1" is there;
   choosing it sets the sliders as you left them -> wrong: the look is missing, or it is there only in the first show.
3. While the box is open, type letters that are your clip keys -> they go into the box and no clip fires. After Return or Esc
   your keys launch clips again at once -> wrong: a clip fires while you type, or the keys are dead after the box closes.
4. With a clip playing on that layer, load a look -> only that effect's picture changes, at once; play, pause, the fader,
   bypass and solo in the layer strip do not move. For one frame the screens hold the picture they had, as they do when you add
   an effect. The same with a look that plugs signals in: no black frame, no stutter, the layer does not restart -> wrong: the
   layer restarts, the picture goes black, or anything in the strip changes.
5. Press Cmd+Z -> the effect returns to how it was just before the look, sliders AND signals, and stays open; the strip does
   not move; a slider you moved on ANOTHER effect meanwhile stays where you put it. (A slider you moved or a signal you
   plugged on the SAME effect after the look goes back too.) -> wrong: the other effect's slider jumps back; the effect folds
   shut; the sliders come back but the signals do not.
6. The name on the button. After loading "Look 2", move one slider -> the button reads "Looks" (dim); move it back to the exact
   value -> the name returns (question 133).
7. Signals. (a) Plug the bass into a slider, set its range, make a look. Put the same effect on another clip and load the
   look -> the slider there follows the bass with the same range, from the first frame. (b) Load that look again on the first
   effect -> nothing jumps, and a slider you are holding stays in your hand. (c) Load "Default", or a look you made with no
   signals -> every signal on that effect is unplugged (question 131). Cmd+Z brings sliders and signals back in one step.
   (d) A look remembers "Macro 3", "Mod 1" or "Mod 2" by name, not what they are doing in this show. (e) A slider driven by
   "Clip Position" holds still today, with or without a look: that is older than this build and is on the list -> wrong: a
   slider pinned at one end after a load; a signal left on after Default; an undo that brings back sliders but not signals.
8. Rename opens the same box, holding the look's name, selected. Delete as before: a list in red; pick one; a window asks.
   You never have to load a look to rename or delete it -> wrong: a look you cannot rename or delete; a name that changes by
   itself.
9. New Look is grey when the effect already is one of your looks, or Default: there is nothing new to keep.
   `Save over "Look 2"` is live only after you loaded or made Look 2 on that effect and then changed something.
10. Load "Look 2", change a slider, open the menu. New Look -> the box says "Look 3": Return keeps the change as Look 3 and
   Look 2 is as it was. Or `Save over "Look 2"` -> a window asks -> click Save Over: Look 2 now holds the change wherever you
   load it from now on. An effect that already had the old Look 2 keeps its settings; its button reads "Looks". There is no
   undo for a Save over -> wrong: Look 2 changed after New Look; Save over names another look; Look 2 is missing or
   half-changed after a Save over.
11. In the Save Over window press Return -> the window closes and NOTHING was saved; only a click on "Save Over" saves. In the
   name box, type a name you already have -> the name turns red and the button goes grey; press Return -> nothing happens and
   the box stays -> wrong: Return saves over a look; a taken name closes the box.
12. After the menu, the box or one of the windows closes, your keys work as before (Return, your clip keys) -> wrong: Return
   opens the menu again, or presses a button.
13. Record a take, load a look in the middle, play the take back -> this build does NOT play the look change back, and a take
   never stores which signals are plugged in: playing it back restores the slider values and leaves the signals as they are
   at that moment. The same is true today of an effect slider you move with the mouse. Recording these is its own build, and
   it is on the list.
14. Where your looks live: on this computer, in the folder Library / Audio-DNA / Looks inside your home folder, one folder per
   effect, one small file per look. A show you open on another computer still looks the same -- each effect keeps its settings
   and its signals in the show -- but the Looks menus there list only that computer's looks. To take your looks along, or to
   back them up, copy that folder.
15. If the disk is full or that folder is locked, New Look and Save over do nothing: the button keeps reading "Looks" and the
   old look is untouched. No message.
16. By eye on your own screens: the button's size, the dim "Looks", the menu's length with many looks, the red Delete list,
   the name box and the red name when a name is taken, the Save Over window.
17. When the old buttons go (the one-save build): the small Save, Load, FX Save and the ten numbered slots are gone. Your two
   old presets were deleted at your word. Still on your disk, and no longer opened by the app: the nine quick FX saves and
   one deck file ("test 1.deck.json") in Library / AudioDNA / Presets (question 134).

---------------------------------------------------------------------------------------------------------
## 7 BORIS QUESTIONS (numbers 131..134; each has a default A; nothing waits)
---------------------------------------------------------------------------------------------------------
131. An effect has a signal plugged into a slider. You load a look that was made with NO signal on that slider, or you load
     "Default".
     A (default) The signal is unplugged: the effect becomes exactly the look. A signal you plugged in by hand and never
       kept in a look is gone; Cmd+Z brings it back.
     B The signal stays plugged in: a look only ever plugs signals in. A look made without signals then does not look the
       same on an effect that has signals, and the button does not show its name.
132. `Save over "Look 2"` replaces Look 2 for good; there is no undo for it.
     A (default) A small window asks first. Only a click on "Save Over" saves; Return and Esc cancel.
     B It saves at once, with no window.
133. You loaded "Look 2" and moved a slider. The small button on the effect:
     A (default) reads "Looks", dim -- it shows a name only while the effect is exactly that look. The menu still says
       `Save over "Look 2"`.
     B keeps reading "Look 2" with a mark that says it was changed.
134. Still on your disk from before: nine quick FX saves (the numbered slots). When the old buttons go, the app no longer
     opens them.
     A (default) Leave them on the disk.
     B Delete them too.
What changes with each answer (so that either is a small change)
- 131 B: ONE line, the constant `looks::kUnwiredEntry` (Unplug -> Keep). Both policies are already built and unit-tested
  (LK-18, LK-24) and `matches` is the same under both. Pre-registered text that changes: GL-9's bar (the B string in 5.6);
  RA-10's table rows 2 and 5 read their "Keep" half; section 6 item 7 (c); V-17's manifest. Rows that do NOT change: LC-9
  (worded for the command), GL-3, GL-4 (both arms), GL-10. PL DA-5 rule 3 then applies only to a replaced connection.
- 132 B: the Save Over window is not opened (one branch in the view); V-21 is dropped; the window clauses leave LM-18 and
  LM-21; GL-10's bar loses "Return did not save; " and its two window steps; HD-15's hidden copy is built with one new store
  row.
- 133 B: the button's text rule in `refresh()` reads the loaded look's name when nothing matches, with a mark; LM-3 and
  MU-EL-17 are re-cut; one more visual state. The loaded look lives in the run only, so the mark is gone after a show is
  re-opened unless HD-14 is built with it. No store or format change.
- 134 B: Harmony moves that folder to the Trash at his word, as for 106. No code.

---------------------------------------------------------------------------------------------------------
## 8 HARMONY'S DECISIONS (each has a default)
---------------------------------------------------------------------------------------------------------
RU's HD-1..HD-11 stand as adopted (plan-effect-looks.md:628-651), except: HD-2 is closed (done; 106 answered); HD-8 also
rules GL-4's second arm. The delta's decisions:
- HD-12 (changed again) S1a starts first once this ruling is adopted; SE runs any time before S2 (beside S1a only in a
  worktree of its own). Default: yes.
- HD-13 The after-write hook. Default: build it (F5 and F6 settle what the plan assumed). Only if a builder reports a blocker
  this reading did not find: drop the hook, accept at most one frame on the look's base value for a newly wired slider,
  re-word LC-11 and GL-9 step (b), and strike "from the first frame" in section 6 item 7 (a).
- HD-14 The loaded look is not saved in the show. Default: not in this lane; one optional key later, with the one-save lane.
  If Boris answers 133 B it is taken up at once, or the mark he asked for vanishes when a show is re-opened.
- HD-15 A kept hidden copy of a saved-over look (`.<name>.look.json.old`). Default: not built; built only if Boris answers
  132 B.
- HD-16 (changed) S1's size: 41 unit rows and about 40 mutants. Default: cut it at the file boundary into two builder
  contexts, S1a = src/effects/EffectLook.h + tests/test_effect_look.cpp (LK rows) and S1b = the store +
  tests/test_look_store.cpp (ST rows; after S1a). One context only if Harmony's packet sizing says it fits.
- HD-17 WITHDRAWN: no mark on a wire that drives nothing; V-23 is dropped (RA-6).
- HD-18 Questions 131-134 go to Boris WITH the adoption, 131 first. Default: yes. Nothing waits for an answer: 131 is one
  constant, 132 one branch, 133 one text rule, 134 no code.
- HD-19 RU's Delete Look window keeps its keys (Return = "Delete", as the Delete Routine window). Default: leave it. If
  Harmony wants ONE rule for both windows that destroy a look, it is RA-2's two lines in that window and one clause in LM-8.
- HD-20 The four deck files in Decks and "test 1.deck.json" in Presets (F19). Default: handed to the one-save lane, whose
  list of shows is the only way a deck is reached; nothing here.
- HD-21 A:GA-6's fork (a flag that limits the engine line to effect slots). Default: no flag, as ruled (RA-6).
- HD-22 The rig shows two more real windows of the TEST app on his screen for the seconds GL-5, GL-10 and four captures
  need: the name box and the Save Over window. Default: as RU already does for the Rename window and the Delete confirm;
  each row closes what it opened, and the probe's exit path quits its own pid.
- HD-10 (extended) four more mutant builds for the live rows, none of which can mask another: M-D = MU-LA-12 + MU-LA-26 +
  MU-LA-36 (GL-9 undo; GL-10 relaunch; GL-10 Return); M-E = MU-LA-18 + MU-LA-21 + MU-LA-22 (GL-9 Ghost; GL-5 taken; GL-10
  offer); M-F = MU-LA-27 + MU-LA-30 (GL-9 hook; GL-10 New Look); M-G = MU-LA-32 (GL-10 tie). The probe evaluates EVERY check
  of a row and prints one FAIL line per failed check; a row is RED only when a FAIL line names the check its own mutant
  predicts. GL-10's "Return did not save" check reads Off's bytes AND `matched`.
FACTS HARMONY MEASURES (who: Harmony, on her rig; each with its rule above)
- N0, the `ctest -N` count on the lane's base, before S1a (GL-8).
- GL-4, both arms: held and black frames for 20 plain loads and for 20 loads that replace two signals, on a playing layer.
- GL-6: what a take sees of a look load, with the count of connection keys in take.json (predicted 0).
- GL-9 step (b): the live values in the load's own answer; step (c): what a wire to an absent signal reads.
- GL-3's noise and bar; the header width from the visual gate's manifest (as RU).
NOT measurable on the rig (the app runs in the background there): which control holds the keyboard as the name box opens,
and what a real Return does in the Save Over window. Who: Boris, section 6 items 1, 3 and 11. The ruling for each outcome:
if he must click the box before typing, the fix round adds nothing new -- RA-3's explicit grab is then not taking, and the
builder finds why; if a real Return saves in the Save Over window, RA-2's focus line is not taking: same fix round, S3's files.

---------------------------------------------------------------------------------------------------------
## 9 SIDE FINDINGS (found, not fixed)
---------------------------------------------------------------------------------------------------------
- SF-1 By source, an AlertWindow hands the keyboard to its first enabled button, not to its text field (F15). If that is what
  happens on screen, then in today's Rename Routine window (MainComponent.cpp:6337-6352) and its siblings he must click the
  field before typing, and a Return at once presses "Rename" with the old name. NOT run. A foreground look settles it; the
  fix is RA-3's two calls in those windows. Not this lane.
- SF-2 A slider driven by "Clip Position" holds still on every owner: the picker offers it (UniversalParamControl.cpp:511)
  and the engine passes no clip clock (F7; the engine's own comment at ConnectionEngine.cpp:349-352 calls it another lane's
  job). The transport lane is its natural owner.
- SF-3 On Linux JUCE's fall-back copy deletes the target first (F13). The day a Linux build ships, `replace` calls rename
  itself. The same fall-back sits under JUCE's own safe-save calls, so under every save in the app.
- SF-4 SignalRegistry.h:14 says "Users can add/remove modulation signals"; nothing calls either function (F8). The "user
  signals" of FOS M-6 do not exist yet: for signals, a show has nothing to save except what Mod 1 and Mod 2 are set to.
- SF-5 What Mod 1, Mod 2 and the eight macros DO is saved nowhere (F9, FOS M-6): a re-opened show, and a look loaded in
  another show, name them and get whatever they are doing then. The one-save lane's question.
- SF-6 `bindConnection` lets go of a hand when it re-binds (F10): any later code that "refreshes a row" through it does too.
- SF-7 `ConnSerialization`'s silent defaults (F2) apply to SHOWS as well: an unknown curve or playback name in a show loads
  as Linear / Forward with no count. The one-save lane owns the serializers.
- SF-8 The picker's own writes stay unfenced and are not undo steps (UniversalParamControl.cpp:656-668). With this lane,
  New Look becomes the first way to keep hand-built wiring outside a show.
- SF-9 Two folders in his Library: AudioDNA (Presets, Decks; F19) and Audio-DNA (Looks; RU:43). Older than the lane; said so
  that nobody "corrects" one into the other.
- SF-10 A momentary key held while a modal window opens is released without the app seeing it (MainComponent.cpp:4120-4145).
  True of every window in the app today; filed, not fixed.
- SF-12 RU names no seam for its Rename window and its Delete confirm, so RU's LM-2 and LM-8 as written would have created
  real windows during ctest. RA-13's one window opener closes it for all four windows of the lane.
- SF-11 The plan's own RED prediction for MU-LA-12 was wrong (E1). Any gate string copied from the plan's section 5 inherits
  it: copy only from this ruling's section 5.

NOT IN THIS LANE
- "Routines" become "actions": its own lane. This lane writes neither word on screen; the Save Over window is modelled on
  the Delete Routine window's CODE, not its text.
- Looks that ship with the app (101 -- Boris: "I will build them later myself, but when the app is finished"). A conversion
  of old presets (106 -- Boris: "delete them. this is a new build"). The removal of the old Save, Load, FX Save and the ten
  slots (one-save S7). Deleting his old quick saves (question 134).
- Any edit to src/connect/ConnSerialization.*, to the show file, or to the three model serializers. A serializer for
  Mod 1, Mod 2 or the macro bank. A clip clock for Clip Position wires (SF-2).
- Connections on a source's parameters, on clip / layer / composition scalars, or on a macro. Looks for procedural sources.
  Bypass in a look. A look of several effects.
- The loaded-look name in the show (HD-14). A mark on the button after a tweak (133 B). A hidden copy of a saved-over look
  (HD-15). A mark on a wire that drives nothing (HD-17, withdrawn).
- Undo for making, renaming, deleting or saving over a look. Recording a look load, a mouse-moved slider or a wiring change
  into a take (HD-6's lane). Making the picker's own writes undo steps (SF-8).
- Delete Look's keys (HD-19). The keyboard focus of today's rename windows (SF-1). The momentary-key release (SF-10).
- Loading a look from a key, MIDI, OSC or production REST. A glide between two looks.
- The stopped sync-dial branches: nothing is carried from lane/bf2 (740b6d6) or lane/bf2-keys (9eab9bd) and nothing of them
  is dropped by this lane; rulings-bf2.md H-17 already rules them superseded.

---------------------------------------------------------------------------------------------------------
## 10 RISKS (the strongest counterargument first)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "Default, and a look made without signals, should NOT unplug what I plugged in." Two of the
   three seats argue it (A:DA-8, A:ST-2): the option he took says only "plugs them in again"; wiring built by hand lives in no store
   (the picker's writes are not even undo steps); and mid-set one click on "Default" leaves a picture that stops moving, with
   nothing on screen saying why. Why "unplug" still stands as the default: (a) a signal on a slider overrides the look's
   value on that slider, so under "keep" a look made without signals does not bring back its own picture on an effect that
   has signals -- the load would silently do less than he asked; (b) New Look is offered only when nothing matches, so the
   match must be exact, and under "keep" the button then reads "Looks" straight after a load: the only sign that a load
   worked is gone; (c) hand-set VALUES that are in no look are replaced by every load as well, and nobody calls that a loss:
   one Cmd+Z brings values and signals back together; (d) this lane gives hand-built wiring its first store -- New Look.
   What makes it cheap to be wrong: the question goes to him with the adoption, with the stage risk in its own words; both
   behaviours are built and tested; his answer is one constant. Cheapest refuting test: section 6 item 7 (c), in front of him.
R2 Two windows that destroy a look answer Return differently: Save Over cancels on Return (RA-2), Delete Look deletes on
   Return (RU). The reason is where Save over sits. HD-19 holds the one-line way to make them the same.
R3 The loaded look is a run-time name (RA-1). It is forgotten when a show is re-opened and the effect was tweaked: Save over
   is then greyed and bare until he loads a look. And it is copied with the slot, so an undo of an older step can bring an
   older name back. Guard: the item and the window always NAME the look that will be replaced.
R4 The engine line reaches every slider in the app, not only effects (F4): a connection to a signal name that no signal has
   rests on the slider's own value instead of the bottom of its range. From inside the app that state cannot arise today
   (F8); a show file edited by hand would look different. Refuting tests: CE-1, CE-3; GL-9's Ghost.
R5 The swap. That a failed rename leaves the old file rests, on macOS, on an INFERRED property of a system call (F13).
   ST-16 is the behavioural check; on Linux the fall-back is destructive (SF-3). The read-back after the swap cannot repair,
   only report: the list is re-read so the menu shows what the disk holds.
R6 Strict reading (RA-4): a look that a NEWER build wrote with a curve or playback this build does not know is not listed
   here. It is invisible in the older build, never damaged. Accepted: the alternative is a look that quietly means something
   else, and a Save over that makes it permanent.
R7 The name box's keyboard (RA-3) and the Save Over window's Return (RA-2) rest on explicit focus calls that the rig cannot
   observe (the app runs in the background). Only Boris's items 1, 3 and 11 see them.
R8 The rig opens real windows of the test app on his screen (HD-22). Each row closes its own; a probe that dies leaves one
   until its exit path quits its pid.
R9 The button can read "Look 1" while the menu says `Save over "Look 2"` (RA-1). True on both counts, and odd. The critics
   are told; answer 133 B removes it.
R10 Save over has no undo (PL DA-11). Guards: the window, Return cancels, the default path never destroys (New Look), and
   HD-15 if he wants no window.
R11 `pathTaken` is one file-system query per typed character, on the message thread (RA-3). On a local disk that is
   microseconds; a home folder on a slow network share would make typing in the box stutter. Accepted.
R12 A matched look becomes the loaded one only when a row is built (RA-1). An effect he brings onto a look's exact settings
   by hand, with no loaded look, shows that look's name on the button but offers a bare, greyed "Save over" until its row is
   built again (he selects the clip again) or he loads the look. Accepted: the safe side of a write that has no undo.
R13 S1 is large and is cut in two (HD-16). `sameConn`'s tolerance (0.000001) could call equal two ranges that differ by less than a slider
   step can express: accepted, below anything he can set (PL R10).
R14 81 is a derived number (GL-8). A row cut or added in a fix round changes it; S3's report restates the count.
R15 A take stores no wiring (PL DA-7): playing a take back restores values and leaves the signals as they are. GL-6 measures
   it with a stop rule; the gap goes to HD-6's lane.
R16 Merge conflicts in MainComponent.cpp, ApiServer.cpp, src/model/Clip.h, ConnectionEngine.cpp and tests/CMakeLists.txt
   with one-save, transport, nudge and outputs: a builder's step 0, never Harmony's (RIG-RULES A2).

STATUS: DONE
