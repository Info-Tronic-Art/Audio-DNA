# PLAN -- lane "looks-answers": DELTA on ruling-effect-looks.md for Boris's answers to questions 101-104 and 106

Author: architect (plan authoring; a blind council attacks this, an architect ruling follows, Harmony decides).
Pins: `git -C /Users/boriskarpman/projects/RealTimeAudio rev-parse --short HEAD` printed 185147b and `git status --short -- src
tests docs CMakeLists.txt` printed nothing, so every code read below is a plain file at 185147b. The two stopped worktrees
(lane/bf2 740b6d6, lane/bf2-keys 9eab9bd) were NOT read: nothing of this lane comes from them (section 9). Nothing was built, run
or launched. Nothing under ~/Library was read or listed.
Labels: VERIFIED = I read the line at 185147b (or in JUCE under build/_deps/juce-src). SHEET = a fact-sheet row its own
VERIFICATION section confirmed. INFERRED / ASSUMED are written where used. Short names: RU = ruling-effect-looks.md (RU:n = its
line n; AM-n, LK-n, ST-n, LC-n, LM-n, GL-n, V-n, HD-n, MU-EL-n are ITS ids); FEL / FOS = facts-effect-looks.md / facts-one-save.md;
BD:n = binding-decisions.md line n; BFB:n = boris-feedback-backlog.md line n. New ids of this delta: DA-n (delta amendment),
MU-LA-n (mutant), CE-n (engine unit row), Q131..Q133. Paths are under /Users/boriskarpman/projects/RealTimeAudio.
Precedence: Boris's verbatim words > Harmony's adoption (plan-effect-looks.md:619-667) > this delta (once ruled) > RU > the plan body.
Where this delta is silent, RU stands word for word.

---------------------------------------------------------------------------------------------------------
## 1 GOAL
---------------------------------------------------------------------------------------------------------
Boris answered (BFB:654-659, BD:923-933). Four of the five answers change what RU ruled:
- 102 -- Boris: "102 B". The option he took reads, as asked: "The look also remembers which signal drives each slider, and
  loading it plugs them in again (a later build)." (boris-clarify-101-106.md:14). This overrules RU's first ruled point (RU:25,
  RU:34-40).
- 103 -- Boris: "For a default behavior I will want to create a look 3 when I change look 2 and save a differnt version of look 2.
  look 2 remains unchanged but I need to have a way to save look 2 if I tweak it a little."
- 104 -- Boris: "open a name box but with default name look x that can easily be changed"
- 101 -- Boris: "I will build them later myself, but when the app is finished"
- 106 -- Boris: "delete them. this is a new build"
This delta says, for each: what a look holds now, what loading does to the signals on an effect, the file format, the menu and
the name box, and lists every stage, case, lint, live row, visual state and Boris check of RU that changes (LA5). Nothing of the
looks lane is built, so no look file exists on any disk today (plan-effect-looks.md:652, "NOT STARTED").

IN ONE PARAGRAPH. A look now holds one effect's values, its Dry / Wet AND the connection on each of its sliders (which signal,
its range, invert and the other saved shaping, on or off). Loading a look makes the effect what the look says: a slider the look
wires is plugged that way; a slider the look holds without a signal is unplugged (question 131, default A); a connection that is
already the same is not touched, so its hand, its smoothing and its phase carry on. A signal the show does not have: the wire is
stored as the look says and SLEEPS (the slider shows the look's own value) until a signal of that name exists -- one line in the
connection engine. New Look opens a name box holding the next free "Look N", selected. The menu gains `Save over "<look>"` for
the look last loaded or made on that effect; it swaps a whole, read-back-verified file in. Three stages stay three (S1, S2, S3),
plus one small independent engine stage (SE). S1 can still start first and is still headless, but NOT unchanged.

---------------------------------------------------------------------------------------------------------
## 2 ESTABLISHED FACTS (VERIFIED or SHEET only)
---------------------------------------------------------------------------------------------------------
The connection (VERIFIED)
- C1 One `ParamConnection` per slider, stored inside the thing it controls: `source` (ConnSource), `shape` (ConnShape), `enabled`
  (bool), plus two runtime members `grip` and `state` that are never saved (src/connect/ParamConnection.h:95-140).
- C2 `ConnSource::Kind` is None, Signal, Macro, Lfo, Envelope, ClipPosition (ParamConnection.h:34). A Signal is named by
  `signalName`, "the PERSISTENT key (SignalRegistry ids are minted per process and never saved)" (:37-38). A Macro by
  `macroIndex` 0..7 (:39). Lfo carries shape, cycleBeats, phaseOffset, pulseWidth (:41-50); Envelope a curve of points, a clock and
  cycleBeats (:52-63); ClipPosition nothing.
- C3 `ConnShape`: outMin / outMax (the range), inverted, playback, loop, curve, inMin / inMax, smoothingMs,
  resetPhaseOnStructural (ParamConnection.h:66-88).
- C4 Copy-ASSIGNING a connection copies source, shape, enabled and resets `grip` and `state` (ParamConnection.h:158-166); a copy
  constructor carries only the three saved members (:156-157); a move keeps everything (:167-168). `state` holds the smoothing
  memory, the hand-back glide, the Sample-and-Hold memory, the once-pin and the name->id cache (:130-139).
- C5 On an effect: `EffectSlot::paramConns` (parallel to `paramValues`), `dryWetConn`, and the live twins `paramLive`,
  `dryWetLive` (src/model/Clip.h:73-76). The renderer reads `effParam` / `effDryWet`, i.e. the twin when it is a number, else the
  base value (Clip.h:109-113).
- C6 The show's JSON shape of one connection is `ConnSerialization::toVar`: {"src": {...}, "shape": {...}, "enabled"}
  (src/connect/ConnSerialization.cpp:95-160; the header comment at ConnSerialization.h:26). `fromVar` starts from a fresh default,
  loads an unknown source kind as None and counts it in `*unknownKindCount` (ConnSerialization.cpp:162-175, :247-255). The show
  writes only connected entries, keyed by parameter INDEX "p" (Clip.cpp:113-125; FEL verification row 4, SHEET).
- C7 The connection engine runs on the MESSAGE thread (`jassert` at src/connect/ConnectionEngine.cpp:294), called from
  MainComponent.cpp:4296-4298 with a Context of signals, macros, the snapshot, dt and now (ConnectionEngine.h:34-50); the signal
  registry's comment names that caller as running at 120 Hz (src/signal/SignalRegistry.cpp:168-169). For each effect slot it
  evaluates every connected, enabled connection and stores the result (or NaN) in the twin; a connection that is NOT connected is
  skipped and its twin is NOT touched (ConnectionEngine.cpp:250-289, the `continue` at :265-266).
- C8 Who reads or writes `paramConns` / `dryWetConn` in src (grep, whole tree): the engine, the three serializers (Clip.cpp,
  Layer.cpp, model/Composition.h), connect/ManualWrite.cpp:54 / :67, and ui/EffectStackView.cpp:369 / :423-425 (binding a row's
  control). Nothing under src/render names them. The renderer takes effect vectors by const reference and never copies a slot
  (render/CompositorEngine.h:168, :385, :451).
- C9 A parameter row's control keeps RAW POINTERS to its connection and twin (`bindConnection(&fx.paramConns[p], &fx.paramLive[p])`,
  EffectStackView.cpp:369, :423-425) and CACHES the source's name, range and invert when bound
  (ui/UniversalParamControl.cpp:126-146). The signal picker writes `conn_->source` directly, on the message thread, with no
  fence and no undo step (UniversalParamControl.cpp:656-668).
- C10 `disconnect(ParamConnection&, LiveValue*)` resets source, state and grip and stores NaN in the twin; it leaves `shape` as
  it was (src/connect/ManualWrite.cpp:205-212).
- C11 A Signal connection is resolved by NAME every tick (`resolveSignalId`, ConnectionEngine.cpp:21-43). When no signal has
  that name the id is 0, `getCachedValue(0)` returns 0.0 (SignalRegistry.cpp:181-189), and the value is shaped and published
  like any other (ConnectionEngine.cpp:106-110, :158): the slider is driven to the bottom of the connection's range.
- C12 The registry builds 32 signals with fixed names at start (Volume, Sub Bass, Bass, Mid, Air, Tempo, Beat Position, Hit, 21
  hidden audio signals, Mod 1, Mod 2, Clip Position; SignalRegistry.cpp:7-86); others arrive through `addSignal` (:88-99).
  User signals and the eight Link macros have no serializer (FOS M-6, facts-one-save.md:244, SHEET). A macro index outside 0..7
  evaluates to 0 (ConnectionEngine.cpp:112-118).
- C13 A row shows a connection's source as text: the signal's NAME for a Signal, "Macro N", the LFO's shape and division,
  "Timeline", "Clip Position" (src/connect/ConnPicker.cpp:110-131; shown by `bindConnection`, UniversalParamControl.cpp:136-137).
  It is derived from the connection alone, whether or not such a signal exists.
What RU already established and this delta leans on (RU section 1; each VERIFIED there)
- U1 / U3 / U4 (RU:137-150): `EffectStackCmd` assigns the whole vector; the fence blocks the message thread until the GL thread
  finishes its frame; `fence_hold_frames` / `fence_black_frames` are in GET /api/state; effect values are plain floats.
- T1 (RU:153-155): a take records a continuous move only through `manualWrite` with Origin::Human; it stores a state at Record
  and at Stop that holds effect VALUES. A take's stored state is keyed slot * 100 + parameter and holds values
  (src/recording/PerfState.h:20-33, VERIFIED); `grep paramConns|ParamConnection` under src/recording finds one comment and no code.
- J1 / J3 (RU:160-164): `moveFileTo` deletes the target first; `FileOutputStream::flush` fsyncs.
JUCE (VERIFIED in build/_deps/juce-src)
- J5 `File::replaceFileIn(target)`: when the target exists it calls `replaceInternal(target)` and never deletes the target first
  (modules/juce_core/files/juce_File.cpp:323-336); `replaceInternal` is `moveInternal` in native/juce_SharedCode_posix.h:434-437.
- J6 `AlertWindow::getButton(int)` / `getButton(const String&)` exist (juce_gui_basics/windows/juce_AlertWindow.h:111, :118).
  A disabled Button ignores its keyboard shortcut: `keyStateChangedCallback` returns false first when `! isEnabled()`
  (juce_gui_basics/buttons/juce_Button.cpp:641-644).
- J7 The app's window idiom: `new juce::AlertWindow`, `addTextEditor("name", old)`, two buttons with Return and Esc as their
  shortcuts, `enterModalState(true, callback, true)` (MainComponent.cpp:6337-6352, `renameRoutine`).
The record (VERIFIED by reading)
- B1 His five answers, verbatim: BFB:654-659 and BD:923-933. Harmony's consequences: plan-effect-looks.md:654-667.
- B2 The questions as asked: boris-clarify-101-106.md:7-27. RU's own "what changes with each answer": RU:718-724.
- B3 His two old presets are gone: "the two files moved to the Trash" (boris-clarify-101-106.md:36; plan-effect-looks.md:663).
- B4 The dispatch names questions 47-50 as open. RU:76-77 records them as answered (BD:865-880) and this lane touches none of
  them either way: no line of this delta changes with any of those answers.

---------------------------------------------------------------------------------------------------------
## 3 ITEMS
---------------------------------------------------------------------------------------------------------

### LA1 SIGNALS IN A LOOK (102 B)

VERIFIED: C1..C13. A "signal on a slider" is a `ParamConnection` in `EffectSlot::paramConns[p]` or `dryWetConn`.

DA-1 WHAT A LOOK HOLDS NOW (replaces AM-1's first three bullets and RU:25).
- For ONE effect: the display name, Dry / Wet, one entry per parameter {uniform, label, value, and the connection on it if one is
  plugged}, and the connection on Dry / Wet if one is plugged. A connection = source + shape + enabled, exactly the three members
  a copy carries (C4). Never grip, never engine state, never a twin. Still never bypass, never the slot's `enabled`.
- Captured: the connections on THIS effect's parameters and on its Dry / Wet. Not captured: connections on a source's
  parameters, on clip / layer / composition scalars, on a macro; the macro bank; the definition of a user signal.
- `looks::capture(def, slot)` reads `paramValues[i]`, `dryWet`, `paramConns[i]`, `dryWetConn`. A look made by this build always
  "speaks about signals" (`hasSignals` = true), also when no slider is wired.
- In memory the look keeps each connection as a `ParamConnection` VALUE (a copy: C4 guarantees it carries no grip and no state).

DA-2 THE FORMAT (amends AM-3; RU's "one optional key per params entry", RU:721).
- Three additive keys, nothing else changes: top level `"signals": true`; per "params" entry an optional `"conn"`; top level an
  optional `"dryWetConn"`. `"conn"` and `"dryWetConn"` hold exactly what `ConnSerialization::toVar` writes (C6): the show's own
  shape, one writer, one reader. "version" is written as 2 and still never gates reading.
- A file WITHOUT `"signals": true` is a look in RU's first format: it holds values only. It lists, loads its values, and touches
  no connection (RU's AM-1 last bullet applies to it, word for word). This is what keeps "look files written by the ruling's
  format stay readable" true; no such file exists today (section 1), the rule costs one `if`.
- A file WITH `"signals": true`: an entry with no "conn" means "this slider has no signal" (not "say nothing").
- Refused whole (added to AM-3's list): "conn" or "dryWetConn" present and not an object; a source kind this build does not know
  (`unknownKindCount` > 0, C6); kind signal with an empty name; kind macro with an index outside 0..7; a number in the connection
  that is not finite. Never half a look: a look whose wiring cannot be read is not listed, and its file is not touched.
- Fork: the connection inside the params entry (CHOSEN) / a separate top-level "conns" array keyed by uniform. The runner-up
  loses: two lists that must agree by name is one more way for a hand-edited file to be half valid; RU already promised this key.

DA-3 WHAT LOADING DOES TO SIGNALS (replaces AM-1's last bullet for looks that speak about signals).
`looks::resolve(def, slot, look)` now gives, per def parameter and for Dry / Wet, a value AND a wanted connection:
| the look ... | the value | the connection after the load |
|---|---|---|
| has an entry with a "conn" | the look's | the look's connection |
| has an entry without "conn" (look speaks about signals) | the look's | unplugged (Q131 default A) |
| has no entry for this parameter (added later, renamed away) | kept | kept as it is |
| is in the first format (no "signals") | the look's | kept as it is |
| is "Default" | the def's default; Dry / Wet 1.0 | unplugged, every slider and Dry / Wet (Q131 default A) |
- "Default" and a look made from an effect with no signals do the same thing to wiring: after the load no slider of that effect is
  driven. Reason: his option reads "remembers which signal drives each slider"; remembering that none does is the same memory, and
  it is the only rule under which the name on the button means "the effect is exactly this look". His words do not settle the
  other reading (a look only ADDS wires), so it is asked: Q131. The line that changes with answer B: one enum in `resolve`
  (`UnwiredEntry::Unplug` -> `Keep`), and unit row LK-18's second clause.
- `looks::sameConn(a, b)`: both unconnected -> the same, whatever shape was left behind (C10 leaves a range behind). Else: kind,
  enabled, every ConnShape field (C3) and the kind's own fields (C2) equal; floats within 0.000001 (a JSON round trip, RU R10).
  It never reads grip or state.
- `looks::matches` (AM-1) gains: for a look that speaks about signals, every parameter the look lands on, and Dry / Wet, must
  also be `sameConn` to what the slot has. A first-format look and RU's value rule are unchanged. `firstMatch` order unchanged.

DA-4 A SIGNAL THE SHOW DOES NOT HAVE (Harmony constraint: whole or nothing, a stated rule, no text).
- What can be missing: only a Signal named by a name no registered signal has (a user signal: C12). A built-in name always
  exists; a macro index 0..7 always exists; LFO, Timeline and Clip Position need nothing outside the connection.
- Today such a connection pins the slider to the bottom of its range (C11). That is the "half-plugged effect".
- RULE: the load writes the connection exactly as the look says -- the effect IS the look, whole -- and the ENGINE treats a Signal
  connection whose name resolves to no signal as not driving: `evaluate` returns NaN, the twin reads NaN, the slider shows and
  renders its base value, which is the look's own value. The row shows the signal's name (C13): a state, not a message. When a
  signal of that name exists the wire drives from the next tick (C11 re-resolves every tick). No text, no window, no count.
- The change: one line in the Signal case of `ConnectionEngine::evaluate` (ConnectionEngine.cpp:106-110): id 0 returns NaN.
  It also changes what a SHOW does today with a connection to a signal it lost (the slider rests on its own value instead of
  the range's bottom). That is wanted, is said to Boris (section 6 item 6c), and is its own stage SE with its own review.
- Forks: (a) store as written, the wire sleeps (CHOSEN); (b) leave that slider unplugged and plug the rest; (c) plug none of the
  look's signals if one is missing. (b) loses: the effect then differs from the look it was just given -- the button reads
  "Looks" straight after loading "Look 2", and the look code would need the signal registry to know (S1 stops being headless).
  (c) loses: one absent user signal silently strips every wire, the worst half-load. (a) costs one engine line and keeps the look
  code free of the registry.
- Macros: the index is plugged as written. What macro 3 DOES in this show is not in the look (C12): told to Boris, section 6.

DA-5 WHAT A LOAD COSTS ON A PLAYING LAYER (the plan's own reason against connections: measured, not assumed away).
- Threads: a connection is read and written on the message thread only (C7, C8, C9). So wiring adds nothing the render thread
  reads; the render thread still never waits; no new lock, no new mutex. The command writes values and connections in its ONE
  fence (AM-9), because values are plain floats the renderer reads (U4).
- Rule 1, EQUAL IS UNTOUCHED: for each parameter and Dry / Wet, when `sameConn(present, wanted)` the connection is not assigned:
  its grip, smoothing memory, Sample-and-Hold memory, once-pin and id cache carry on (C4 is why this must be explicit). Loading
  the same look twice, or a look that shares a wire with what is on, moves nothing on that wire.
- Rule 2, A DIFFERENT WIRE STARTS FRESH: assigned in place (element by element, so the row's raw pointers stay good, C9; the
  vectors are never resized or re-assigned), which resets grip and state (C4): a hand on that slider is let go, smoothing starts
  from the first value, a play-once connection plays again from its start. This is the visible cost and it is the point of the
  load: that slider now follows another signal.
- Rule 3, AN UNPLUGGED SLIDER RESTS AT ONCE: unplugging goes through `disconnect` (C10), which stores NaN in the twin; without it
  the twin would keep the last driven value forever (C7 skips unconnected entries). A replaced wire's twin is set to NaN the same
  way, so no frame shows the OLD signal's value under the new look.
- Rule 4, NO FRAME ON THE BASE VALUE: after the write and still inside the fence the command calls a hook the host gives it;
  the host evaluates that ONE slot's connections once (`ConnectionEngine::tickSlot`, new, a public wrapper of the existing
  per-slot loop at ConnectionEngine.cpp:250-289, with the Context the regular tick builds at MainComponent.cpp:4296). So the
  first frame after the fence already shows the driven values. Without the hook a newly wired slider would show the look's base
  value until the next regular tick (C7: up to 1/120 s, at most one frame). ASSUMED, to be shown by LC-11 and GL-9: that the
  Context can be built outside the regular tick with dt = 0. If it cannot, the hook is dropped, the one frame is accepted and
  Harmony is told (HD-13).
- What is measured, by Harmony: GL-4 gains a second arm with two WIRED looks (same table, same bars: black 0; hold at most 2 per
  load); GL-9 reads, straight after a wired load has answered, that each newly wired slider's twin is already a number.
- Allocation: a connection holds a string and a vector of points; copying them allocates on the message thread, inside the
  fence. Nothing on the audio callback or the analysis thread changes.

DA-6 `EffectLookCmd` WITH CONNECTIONS (amends AM-9; one undo step).
- `before` and `after` are each {values, Dry / Wet, the connection per parameter, the Dry / Wet connection, the loaded-look name
  (DA-9)}. `after` = `resolve`; `before` = a copy of the slot's present ones. Execute and redo apply `after`, undo applies
  `before`, through ONE apply function that obeys DA-5 rules 1-4. Whole or not at all: the checks of AM-9 (index, effect name,
  parameter count) run before the first write.
- One Cmd+Z puts back values AND wiring as they were. A wire he re-plugged by hand on that effect AFTER the load goes back too
  (as a slider moved on the same effect does, RU section 6 item 4): undo puts that effect back to before the look.
- "A pick that would change no value pushes no undo step" (AM-6) becomes "... that would change no value and no connection".
- LINT-EL-3 is re-worded (section 5.5). `src/core/EffectLookCmd.h` now names `paramConns`, `dryWetConn`, the twins (only to
  clear them) and `disconnect`; it still names no `bypassed`, no layer field.
- After a load the view re-binds the controls of that ONE row (`bindConnection` again with the same pointers, C9), so the source
  buttons, ranges and invert marks show the new wiring without the row being rebuilt or folded (LM-20).

DA-7 WHAT A TAKE SEES (amends AM-20).
- PREDICTION (T1): nothing of the wiring. A signal moving a slider is never a recorded move; the state stored at Record and at
  Stop holds base values, not connections. So a take played back restores the looks' VALUES and leaves the wiring as it finds it.
  GL-6 gains one read ("take.json names no connection", predicted) and stays a measurement with its stop rule. The gap goes to
  HD-6's own lane, with the mouse-moved sliders and the look loads already filed there.

RED-first tests and mutants: LK-16..LK-21, LC-9..LC-12, CE-1, CE-2, LM-20 (section 5). Gate rows: GL-4 (second arm), GL-9.

### LA2 A CHANGED LOOK (103)

VERIFIED: J1, J3, J5; RU AM-4 ("If Boris answers 103 = B ... writes a sibling temp file and swaps it in with
`juce::File::replaceFileIn`, never `moveFileTo`"), RU:293-294.

DA-8 THE MENU (replaces AM-6's first line and its "New Look names the look by itself" bullet). Top to bottom:
   "Default" / separator / his looks in natural name order / separator / "New Look" / `Save over "<look>"` / "Rename" / "Delete".
- "New Look" comes first because it is his default ("For a default behavior I will want to create a look 3 when I change look
  2"). Enabled as ruled: only when the settings (values and wiring) are not already a look, Default included. It opens the name
  box (LA3).
- `Save over "Look 2"`: the look named is THE LOADED LOOK (DA-9). Enabled when there is one and the settings differ from it
  (`!matches`). With no loaded look the item reads "Save over" and is greyed; it never disappears (a menu whose items come and go
  moves the others under his finger).
- Texts, exact, added to AM-6's list: `Save over "<look>"` and "Save over". The confirm (Q132 default A): title "Save Over
  Look", text `Replace look "<name>" of <effect> with the settings on now? This cannot be undone.`, buttons "Save Over" and
  "Cancel" (the Delete Look model, AM-6). `isLegalName` (AM-2) also reserves "Save over" in any letter case.

DA-9 WHICH LOOK IS "THE LOADED LOOK" (the button's name is derived; after a tweak the values match nothing).
- A new RUNTIME field on the slot, `EffectSlot::lookName` (a string, LAST member of the struct, never saved in the show, copied
  with the slot): the name of the look last loaded or made on this effect instance. Message thread only (the renderer never
  copies a slot and reads slots by const reference, C8).
- Three writers: (1) `EffectLookCmd` (execute / redo set the loaded look's name, undo puts the earlier name back; "Default" sets
  it empty); (2) New Look and Save over, after the store has verified the file; (3) the view's `refresh()`: whenever the derived
  name on the button is one of his looks, `lookName` is set to it -- so an effect that came from a saved show and still IS
  "Look 2" has Look 2 as its loaded look the first time its inspector shows it.
- The loaded look = `lookName`, if the store lists a look of that name for this effect; else none. A rename of that look from
  this row carries the name along; a delete leaves none.
- The BUTTON is unchanged: its text stays derived from values and wiring (AM-7; MU-EL-17 stays a mutant). After a tweak it reads
  "Looks" (dim) and the menu's `Save over "Look 2"` names what he tweaked. Whether the button should keep the name with a mark
  is his eye: Q133.
- Forks: (a) the runtime field (CHOSEN); (b) "Save over" as a LIST of all his looks, like Rename and Delete; (c) the name saved
  in the show. (b) loses: he was asked about "Save over Look 2" by name; a list makes him find the look he tweaked among thirty,
  and a wrong pick destroys another look with no undo. (c) loses for now: it is the show file's first change from this feature
  and three serializers the one-save lane is editing (RU R2); the cost of not doing it is narrow (an effect tweaked, the show
  saved and re-opened: no loaded look until he loads one) and it is one optional key later (HD-14).

DA-10 THE VERIFIED REPLACE (replaces RU:45-47 "No look file is ever read-modify-written" and AM-4's 103 bullet).
- NEW SENTENCE for RU section 0 (2) and the pitfall text: "A look file is never edited in place. It changes in exactly one way:
  Save over writes a complete new file beside it, reads that file back through the listing's own reader, and swaps it in with one
  rename; at every instant the look's name holds the whole old look or the whole new one. An unreadable file is still never
  rewritten and never deleted, and Save over is offered only for a look that lists."
- `EffectLookStore::replace(effect, name, look)`: (1) the look is listed, else nothing; (2) write `.<name>.look.json.new` in the
  same folder (a fixed name per look: starts with a dot, does not end in ".look.json", so AM-3 skips it; a left-over from a
  crash is overwritten by the next Save over of that look -- delete it first, then open the stream, Pitfall 46 / J4); (3) check
  the stream after `write` and after `flush()` (J3); (4) read the temp file back through the listing's reader and require the
  captured look, bit for bit, connections `sameConn`; (5) `temp.replaceFileIn(target)` (J5), never `moveFileTo` (J1); (6) read
  the target back the same way; (7) only then update the list entry. A failure in 2-4 deletes the temp file and changes nothing.
  A failure in 5 leaves the old file. Step 6 failing is reported to the caller as failed; the list is re-read from disk.
- A failed Save over shows nothing and changes nothing; the button keeps reading "Looks" (the state). No text (AM-26).
- What Save over drops: keys this build does not know inside that one file (the file is written fresh). Said in risks.
- LINT-EL-1 changes: `replaceFileIn` is allowed in exactly one function, `replace`; the other three words stay forbidden.

DA-11 UNDO OF A SAVE OVER: THERE IS NONE.
Making, renaming, deleting and saving over a look change the app's store, not the show (AM-6 last bullet stands). Cmd+Z steps
through SHOW edits; a store change in that list would be undone long after, in another show, on an effect that may by then hold something else
(RU AM-6: "they change the app's store, not the show"). What protects him instead: the confirm (Q132 default A), the
verified swap, and that the DEFAULT path never destroys anything (New Look). If he answers Q132 = B (no confirm), the cheap
guard is one kept hidden copy per saved-over look (`.<name>.look.json.old`, copied before step 5): HD-15, not built by default.

RED-first tests and mutants: ST-13..ST-15, LM-18, LC-13 (section 5). Gate row: GL-10.

### LA3 THE NAME BOX (104)

VERIFIED: J6, J7; RU AM-24 (the Rename window and `LookNameFilter`), AM-8 (the keyboard goes home), AM-7 (the button never
takes the keyboard).

DA-12 ONE BOX FOR NEW LOOK AND RENAME (replaces AM-6 "New Look names the look by itself and opens no window", AM-4 step 1, and
AM-24's last sentence).
- Built in the `renameRoutine` idiom (J7) by the view: a `juce::AlertWindow`, one text field "name" with `LookNameFilter`,
  two buttons. New Look: title "New Look", the field holds `nextName` = "Look N" (N the smallest number such that no file
  "Look N.look.json" exists in that effect's folder, compared without letter case, unreadable files included), buttons
  "Save Look" (Return) and "Cancel" (Esc). Rename: title "Rename Look", the field holds the present name, buttons "Rename" and
  "Cancel".
- On opening, the field has the keyboard and ALL its text is selected: typing replaces "Look 3"; Return at once keeps "Look 3".
- Return (or the first button): the look is made under the trimmed name. Esc (or Cancel, or the window closed any other way):
  no look, no file, nothing changes.
- An empty, reserved or taken name is a STATE, never a message: `looks::nameVerdict(name, the effect's file names, own name)`
  gives Ok / Empty / Reserved / Taken (illegal characters and a 41st character cannot be typed: the filter). While the verdict
  is not Ok the first button is greyed -- a greyed button ignores Return (J6), so the box stays open -- and for Taken the field's
  text is drawn in the warning colour. The box opens on a free name by construction, so Return straight away always works.
  The modal callback checks the verdict once more and makes nothing if it is not Ok (the last guard; unreachable by hand).
- WHAT is saved: the effect's settings and wiring at the moment he presses Return, not at the moment the box opened; the
  callback first checks that the row's index still holds that effect (LM-6's guard), else nothing is made.
- `EffectLookStore::make(effect, name, look)` takes the name: `isLegalName`, the path must not exist (never an overwrite), then
  AM-4 steps 2-6 unchanged. `EffectLookStore::nextName(effect)` is the pure "Look N" rule.
- The keyboard. While the box is open it is a modal window with its own text field: every key he types goes to the field, and
  the main window's key handlers are reached only through the focused component's own chain (INFERRED from JUCE's focus model
  and the two rename windows that already work this way, MainComponent.cpp:6337-6352; pinned by LM-21 and checked by Boris,
  section 6 item 2). MIDI pads keep firing clips: he is not typing on them. Every close of the box -- Return, Esc, the button,
  the confirm of Save over -- fires `onLooksUiClosed` exactly once (AM-8), which gives the keyboard back to the main window.
  The looks BUTTON still never takes the keyboard (AM-7, LM-8 unchanged).
- Known and not changed here: a momentary key held down while a modal window opens is released without the app seeing it
  (MainComponent.cpp:4120-4141 polls releases only while the main window has the keyboard). True of every window in the app
  today; filed, not fixed.
- Fork: a `juce::AlertWindow` (CHOSEN) / an editor drawn in place on the effect's header (the deck-tab rename, Pitfall 65).
  The runner-up loses: the header is 26 px with four things in it, a rebuilt row would take the editor away (Pitfall 65), and
  Boris asked for "a name box".

RED-first tests and mutants: LK-22, ST-2 (re-written), LM-4 (re-written), LM-16, LM-17, LM-19, LM-21. Gate row: GL-5 (re-written).

### LA4 NOTHING SHIPS (101), HIS OLD PRESETS ARE GONE (106)

- 101 -- Boris: "I will build them later myself, but when the app is finished". No looks ship, no stage, no bundle folder. RU's
  line "101 B: `looksFor` also lists a read-only folder inside the app bundle" (RU:719-720) stays as the note for that later day
  and is not built. Nothing in RU changes.
- 106 -- Boris: "delete them. this is a new build". No conversion stage. Dropped from RU: HD-2 (closed: done, B3), AM-18
  (closed), question 106's B path, and RU section 6 item 14's second sentence (it spoke of four old files that "stay on the disk").

### LA5 THE CHANGE LIST (RU ids; "stands" = word for word)

| RU item | old | new |
|---|---|---|
| section 0 (1), RU:24-25, :34-40 | "Nothing else: ... not the signal connections" | DA-1: values, Dry / Wet and the connection on each slider and on Dry / Wet. Still not bypass, not `enabled`. |
| section 0 (2), RU:45-47 | "No look file is ever read-modify-written" | DA-10's NEW SENTENCE. |
| section 0 "the rest", RU:67-69 | "New Look is offered only when ... not already a look" | stands; "settings" now means values and wiring. |
| AM-1 | values only; "No grip is opened, no connection is touched" | DA-1, DA-3; the old last bullet holds only for a first-format look. |
| AM-3 | version 1; keys format, version, effect, dryWet, params | DA-2: + "signals", "conn", "dryWetConn"; version 2; five more refusals. |
| AM-4 | step 1 names the look itself; the 103 bullet "Not built now" | `make(effect, name, look)`; `nextName`; `replace` is built (DA-10). Steps 2-6 stand. |
| AM-5 | -- | stands (a failed make or replace makes or changes nothing). |
| AM-6 | five menu words; New Look opens no window | DA-8: six items; DA-12: New Look opens the box. The Rename / Delete lists, the red list, the SafePointer rule, no right-click: stand. |
| AM-7, AM-8 | -- | stand. `onLooksUiClosed` also fires for the name box and the Save Over confirm. |
| AM-9 | writes `paramValues[i]` and `dryWet` | DA-6: also connections and `lookName`, same single fence, plus the after-write hook. |
| AM-10, AM-12, AM-13, AM-17, AM-19, AM-21, AM-22, AM-25, AM-26 | -- | stand (AM-21's counts: section 5 here). |
| AM-11 | routes | section 5.6 here: `look_make` takes "name"; new `look_replace`; `look_ui` gains accept / cancel; GET looks gains `conns`, `live`, `loaded`, `menu.saveOver`, `box`. |
| AM-14 | -- | stands; the new mutants are MU-LA-1..MU-LA-26. |
| AM-15 | eight live rows | ten: GL-1, GL-4, GL-5, GL-6, GL-8 re-written; GL-9, GL-10 new; GL-2, GL-3, GL-7 stand. |
| AM-16 | 18 visual states | 23 (section 5.7 here). |
| AM-18, HD-2 | his four old files | closed (LA4). |
| AM-20 | what a take sees | DA-7 added. |
| AM-23 | question 102 re-cut | answered: B. |
| AM-24 | Rename window; "changes nothing ... the window closes" | DA-12: the same box as New Look; a refused name greys the button and the box stays open. |
| LK-6, LK-15; ST-2, ST-7; LC-2, LC-7; LM-1, LM-4 | -- | re-written (section 5). |
| MU-EL-1..MU-EL-48 | -- | stand as written; MU-EL-30 also arms the re-written ST-2. |
| LINT-EL-1, LINT-EL-3 | -- | re-worded (section 5.5). LINT-EL-2, -4, -5, -6 stand. |
| section 6 items 1, 3, 6, 7, 8, 10, 12, 14 | -- | re-written (section 6 here); 2, 4, 5, 9, 11, 13 stand. |
| section 7 | 101-104, 106 | answered; new Q131..Q133. |
| HD-6 | the take gap | + wiring is not in a take (DA-7). |
| R1, R5, R6, R7 of RU section 10 | -- | R1 is moot (he chose signals); R5 now also covers the temp file; R6 stands; R7 replaced by DA-12's state. |
| the pitfall text (RU:449-458) | "never rewrites a look file"; "never a connection" | DA-10's sentence; "values, Dry / Wet and the connections, by name; an equal connection is never re-assigned; a Signal wire whose name no signal has sleeps". |
| S1 | 27 rows; values only | 37 rows; + connections in the look, `make` with a name, `nextName`, `nameVerdict`, `replace`. Still headless, still shares only the two CMake lists. NOT unchanged. |
| S2 | 8 rows | 13 rows; + connections in the command, the hook, `look_replace`. |
| S3 | 15 rows | 21 rows; + the name box, Save over, the re-bind. |
| -- | -- | NEW stage SE (the sleeping wire and `tickSlot`): 2 rows, 2 files. |
What stands untouched: where the looks live and the file-is-the-look rule (AM-2), the read rules for bad files, by-name landing,
the test-mode folder, the one place that reads a folder, the button, the keyboard rule, Cmd+Z keeps the effect open, who removes
the old buttons (one-save S7; LINT-EL-5), no new text that announces anything.

---------------------------------------------------------------------------------------------------------
## 4 STAGES + ORDER (one builder context per stage; Harmony runs every live row and gives every gate verdict)
---------------------------------------------------------------------------------------------------------
One lane (`lane/effect-looks`, one worktree), as RU section 4. Each stage is ONE builder context, ends with the full ctest
green, shows its own unit rows RED first (the named mutant) then GREEN, and is reviewed pinned before the next starts. A builder
never launches the app, never runs a live row, never gives a gate verdict. RU's stage table stands except as amended here.

| stage | owns (files) -- additions to RU's table in CAPITALS | proves (unit, by the builder) | Harmony runs herself, and when |
|---|---|---|---|
| S1 the look and the store | as RU: NEW src/effects/EffectLook.h; NEW src/effects/EffectLookStore.h / .cpp; CMakeLists.txt; NEW tests/test_effect_look.cpp; NEW tests/test_look_store.cpp; tests/CMakeLists.txt (two targets, EACH NOW ALSO LINKS src/connect/ConnSerialization.cpp). EffectLook.h gains the connection per entry, `sameConn`, `nameVerdict`; the store gains `nextName`, `make(effect, name, look)`, `replace` | LK-12 first, then LK-1..LK-22, ST-1..ST-15, LINT-EL-1, LINT-EL-4; their mutants RED | BEFORE S1: `ctest -N` on the lane's base (N0 of GL-8). After S1: nothing live; the pinned review. |
| SE the sleeping wire (NEW; shares no file with S1; either order, or side by side) | src/connect/ConnectionEngine.h / .cpp (the one line of DA-4; `tickSlot`); tests/test_connection.cpp (two cases appended); docs/claude/rendering.md (the Mapping System section gains two sentences: a Signal connection whose name no signal has drives nothing; `tickSlot`) | CE-1, CE-2; their mutants RED | After the pinned review: the regression probes of RU (.harmony/probe-effects-parity.sh, .harmony/probe-deck-path.sh, the gate A set), GREEN only. SE's own live proof is GL-9 (d), run after S2. |
| S2 the load, undo, the host, the data routes, the probe (after S1 AND SE) | as RU, plus: src/model/Clip.h (ONE member, `lookName`, last in `EffectSlot`); src/core/EffectLookCmd.h carries connections and the name and takes the after-write hook; MainComponent.cpp `performLookLoad` builds the hook (`tickSlot` with the regular tick's Context); ApiServer: GET looks gains `conns`, `live`, `loaded`; `look_make` takes "name"; NEW `look_replace`; the probe gains fixture show C and the look "Ghost" and rows GL-9 and GL-4's second arm; docs as RU with DA-10's sentence and the pitfall text of LA5 | LC-1..LC-13, LINT-EL-3 (re-worded), LINT-EL-5, LINT-EL-6; their mutants RED | After the pinned review: GL-1, GL-2, GL-3, GL-4 (both arms), GL-6, GL-7, GL-9, each with its RED arm. GL-4 and GL-6 are measurements with decision tables: a STOP outcome stops the lane before S3. |
| S3 the menu, the name box, Save over | as RU, plus in src/ui/EffectLooksMenu.h the pure model of the box (title, text, verdict -> greyed, taken) and of the Save over item; in src/ui/EffectStackView.h / .cpp the name box (New Look and Rename), the Save Over confirm, the row re-bind after a load, the `lookName` writer in `refresh()`; ApiServer `look_ui` gains accept / accept:<text> / type:<text> / cancel and GET looks gains `menu.saveOver` and `box`; the probe gains GL-5 (re-written) and GL-10 | LM-1..LM-21, LINT-EL-2; their mutants RED | GL-5, GL-10 with their RED arms; GL-8; the regression probes; then GL-1, GL-2, GL-3, GL-4, GL-7, GL-9 once more at the lane's final head (GREEN only). |
| VG the visual gate | a capture builder, then five critic seats, as RU; 23 states (section 5.7) | -- | The verdict. A fix round goes back to S3's files. Boris sees nothing of this lane before it passes. |

Order: (one-save S7, in its own lane, any time) S1 and SE -> S2 -> S3 -> VG -> merge (RIG-RULES B).
CAN S1 START FIRST, UNCHANGED? It can start first; it cannot start unchanged. It is still headless and still shares no file with
any other lane except the two CMake lists, but its format, `make`'s signature and ten of its rows are new (LA5). S1's packet is
written from RU section 4 / 5 WITH this delta's section 5 laid over it, after this delta is ruled.
Harmony's decisions added by this delta (each has a default)
- HD-12 (changed again): S1 starts first once this delta is ruled and adopted. SE may run beside it.
- HD-13 If the after-write hook cannot be built (DA-5 rule 4, ASSUMED): default, drop the hook, accept at most one frame on the
  look's base value for a newly wired slider, re-word LC-11 and GL-9 (b) to say so, tell Boris in section 6 item 6.
- HD-14 The loaded look is not saved in the show (DA-9). Default: not in this lane; one optional key later, with the one-save lane.
- HD-15 A kept hidden copy of a saved-over look. Default: not built; built only if Boris answers Q132 = B.
- HD-16 If S1's context runs long, split it at the file boundary: S1a = EffectLook.h + tests/test_effect_look.cpp, S1b = the
  store + tests/test_look_store.cpp. Default: one stage.
- HD-17 A mark on a sleeping wire's row (DA-4). Default: none beyond the signal's name, which the row already shows; named to
  the critics at V-23.
- HD-10 (extended): two more paired mutant builds for the live rows: M-D = MU-LA-12 + MU-LA-26 (GL-9 undo; GL-10), M-E =
  MU-LA-18 + MU-LA-21 + MU-LA-22 (GL-9 Ghost; GL-5 taken; GL-10 item). A row is RED only when its FAIL line names the check its
  own mutant predicts.

---------------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; exact strings and bars; each with its RED arm; a bar is met or reported)
---------------------------------------------------------------------------------------------------------
RU section 5 stands except for the rows below. One TEST_CASE per id, named exactly as written. Every row is RED first.

5.1 tests/test_effect_look.cpp, tag [looks] (S1; 22 rows = RU's 15, two re-written, + 7)
| id | exact test name | RED arm |
|---|---|---|
| LK-6 (re-written) | "LK-6 resolve gives values, Dry/Wet and connections only: bypassed, enabled, the name and both live twins are untouched" | MU-EL-4 |
| LK-15 (re-written) | "LK-15 a look name is legal when it is 1 to 40 characters, has none of the nine file characters or a control character, does not start with a dot, and is not Default, Looks, New Look, Save over, Rename or Delete in any letter case" | MU-EL-29 |
| LK-16 | "LK-16 a look with signals round-trips: a signal by name, a macro by index, an LFO, a timeline with its points and a clip position, each with range, invert, curve, smoothing and enabled, on parameters and on Dry/Wet" | MU-LA-1 |
| LK-17 | "LK-17 a file in the first format holds values only: resolve keeps every connection and matches ignores them" | MU-LA-2 |
| LK-18 | "LK-18 resolve with signals: a wired entry gives the look's connection; an unwired entry unplugs; a parameter the look has no entry for keeps its value and its connection; Default unplugs every parameter and Dry/Wet" | MU-LA-3 |
| LK-19 | "LK-19 matches with signals: equal values with another signal, range, invert or enabled do not match; two unplugged connections match whatever range was left behind" | MU-LA-4 |
| LK-20 | "LK-20 fromVar refuses the whole file for a connection it cannot read: conn not an object, an unknown source kind, a signal with no name, a macro index outside 0..7, a number that is not finite" | MU-LA-5 |
| LK-21 | "LK-21 sameConn compares what is saved and nothing else: a grip and engine state never make two connections differ" | MU-LA-6 |
| LK-22 | "LK-22 nameVerdict: empty, reserved, taken without letter case, free; a look's own name in another letter case is free for its own rename" | MU-LA-7 |
If Boris answers Q131 = B: LK-18's second and fourth clauses read "an unwired entry keeps the connection" / "Default keeps every
connection", and LK-19 gains "a slider the look leaves unwired matches whatever drives it". No other row changes.

5.2 tests/test_look_store.cpp, tag [lookstore] (S1; 15 rows = RU's 12, one re-written, + 3)
| id | exact test name | RED arm |
|---|---|---|
| ST-2 (re-written) | "ST-2 nextName gives Look 1, Look 2; a freed number is used again; a number taken in another letter case or by an unreadable file is skipped; make under a given name makes that file, and under a taken name makes nothing and never overwrites" | MU-EL-30, MU-LA-8 |
| ST-13 | "ST-13 replace swaps a whole verified file in: the look then lists with the new values and connections; a second store on the folder reads the same; a replace that cannot write, or whose new file does not read back equal, leaves the old file's bytes unchanged" (a writer hook that truncates; a read-only folder) | MU-LA-9, MU-LA-26 |
| ST-14 | "ST-14 the new file's name is never listed: a left-over from a crash is not a look, and the next replace of that look succeeds and leaves none" | MU-LA-10 |
| ST-15 | "ST-15 replace touches one file: every other look's bytes and modification time are unchanged; a name that does not list is not replaced and no file is made" | MU-LA-11 |

5.3 tests/test_effect_look_cmd.cpp, tag [lookcmd] (S2; 13 rows = RU's 8, two re-written, + 5)
| id | exact test name | RED arm |
|---|---|---|
| LC-2 (re-written) | "LC-2 a load touches one slot: every other slot's values, connections, grips and engine states are bit-equal" | MU-EL-13 |
| LC-7 (re-written) | "LC-7 a first-format look on a connected parameter keeps its connection, its grip and its live twin; after one engine tick its effective value is the signal's and the look's value is the base underneath" | MU-EL-37 |
| LC-9 | "LC-9 a look with signals is one undo step: execute plugs the look's connections and unplugs the rest, undo puts the earlier connections back, redo plugs again" | MU-LA-12 |
| LC-10 | "LC-10 a connection equal to the wanted one is not assigned: its grip, smoothing memory and once-pin survive execute, undo and redo; a different one is replaced and starts fresh" | MU-LA-13 |
| LC-11 | "LC-11 an unplugged or replaced parameter's live twin is cleared inside the fence, and the after-write hook runs once per execute, undo and redo, inside the fence, after the write" | MU-LA-14, MU-LA-15 |
| LC-12 | "LC-12 the connection arrays are written in place: the address of every connection and twin is the same before and after execute, undo and redo" | MU-LA-16 |
| LC-13 | "LC-13 the loaded-look name follows the command: execute sets it, undo puts the earlier name back, redo sets it again; Default sets it empty" | MU-LA-17 |

5.3b tests/test_connection.cpp, two cases appended (SE)
| id | exact test name | RED arm |
|---|---|---|
| CE-1 | "CE-1 a signal connection whose name the registry does not hold drives nothing: evaluate gives NaN and the twin reads the manual value; once a signal of that name is added it drives at the next tick" | MU-LA-18 |
| CE-2 | "CE-2 tickSlot publishes one slot's twins and leaves every other slot's twin and connection state unchanged" | MU-LA-19 |

5.4 tests/test_effect_looks_menu.cpp, tag [looksmenu] (S3; 21 rows = RU's 15, two re-written, + 6)
| id | exact test name | RED arm |
|---|---|---|
| LM-1 (re-written) | "LM-1 the menu lists Default, his looks in natural order, New Look, Save over, Rename, Delete; at most one item is ticked, the first look the settings and wiring match" | MU-EL-40 |
| LM-4 (re-written) | "LM-4 New Look is enabled only when the settings match no look, Default included; after the box is accepted the button reads the new look's name; after Cancel, and after a make that fails, nothing changes and no file exists" | MU-EL-42 |
| LM-16 | "LM-16 New Look opens the name box holding the next free Look N with all of it selected; accepting it unchanged makes Look N; accepting another name makes that look; the look holds the settings at the accept, not at the opening" | MU-LA-20 |
| LM-17 | "LM-17 the box's first button is greyed while the name is empty, reserved or taken, the taken name is drawn in the warning colour, and an accept in that state makes nothing and leaves the box open" | MU-LA-21 |
| LM-18 | "LM-18 Save over names the look last loaded or made on this effect: greyed and bare with none, greyed while the settings equal it, live after a tweak; confirming replaces that look and the button reads its name; Cancel changes nothing" | MU-LA-22 |
| LM-19 | "LM-19 Rename uses the same box: it holds the present name selected, greys the same way, and a rename of the loaded look keeps Save over pointing at it" | MU-LA-23 |
| LM-20 | "LM-20 after a load of a look with signals the row's source buttons, ranges and invert marks show the new wiring and the row is neither rebuilt nor folded" | MU-LA-24 |
| LM-21 | "LM-21 every close of the name box and of the Save Over confirm calls the focus-home hook once; while either is open the looks button still does not want the keyboard" | MU-LA-25 |

5.5 The new mutants and the lints
MU-LA-1 `toVar` omits "conn" (LK-16). MU-LA-2 a file without "signals" is read as speaking about signals (LK-17). MU-LA-3 an
unwired entry keeps the connection (LK-18). MU-LA-4 `matches` ignores connections (LK-19). MU-LA-5 a connection that cannot be
read loads as none (LK-20). MU-LA-6 `sameConn` compares the grip (LK-21). MU-LA-7 `nameVerdict` compares with letter case
(LK-22). MU-LA-8 `make` ignores the given name and uses `nextName` (ST-2). MU-LA-9 `replace` writes straight onto the target
(ST-13). MU-LA-10 the new file's name ends in ".look.json" (ST-14). MU-LA-11 `replace` of a name that does not list makes the
file (ST-15). MU-LA-12 undo restores values but not connections (LC-9; GL-9). MU-LA-13 `apply` assigns every connection
(LC-10). MU-LA-14 unplugging resets the source only and keeps the twin (LC-11). MU-LA-15 the hook is called before the write
(LC-11). MU-LA-16 `apply` assigns the whole vector (LC-12). MU-LA-17 undo leaves the loaded-look name (LC-13). MU-LA-18 the
engine line of DA-4 removed (CE-1; GL-9). MU-LA-19 `tickSlot` ticks every slot of the vector (CE-2). MU-LA-20 the box opens
empty (LM-16). MU-LA-21 the verdict treats a taken name as free (LM-17; GL-5). MU-LA-22 Save over names what `firstMatch`
names (LM-18; GL-10). MU-LA-23 a rename does not carry the loaded-look name (LM-19). MU-LA-24 no re-bind after a load (LM-20).
MU-LA-25 the focus-home call removed from the box's callback (LM-21). MU-LA-26 `replace` only updates the list (ST-13; GL-10).
RU's mutants MU-EL-1..MU-EL-48 stand as written.
- LINT-EL-1 (re-worded) "the Looks folder is spelled once: `grep -rn 'getChildFile *(\"Looks\")' src` prints exactly one line, in
  src/effects/EffectLookStore.cpp; EffectLookStore.cpp names no replaceWithText, replaceWithData, TemporaryFile; replaceFileIn
  appears exactly once, inside `replace`; findChildFiles / RangedDirectoryIterator / DirectoryIterator appear in the new files
  only inside `listFolder`".
- LINT-EL-3 (re-worded) "src/core/EffectLookCmd.h names no bypassed, opacity, solo, mute, runtime( and no `.enabled` of a slot;
  it assigns no vector of connections or twins whole (no `paramConns =`, no `paramLive =`)".
- LINT-EL-2, -4, -5, -6 stand. LINT-EL-6 still holds with the name box: it is built with `new juce::AlertWindow` and
  `enterModalState`, not with a `show` call.

5.6 Harmony's live rows (.harmony/probe-effect-looks.sh; the rig of RU 5.6, unchanged: test-server build, `open -g`,
--test-mode, a FRESH AUDIODNA_LOOKS_DIR per row, the live lock, no Output window, no full-screen capture, no synthetic input,
quits only its own pid).
Routes (amends AM-11; all inside `#if AUDIODNA_TEST_SERVER`):
- GET /api/debug/looks also returns, per `values` entry, `conn` (the connection as the show writes it, or null) and `live` (the
  twin: a number, or null when it is NaN); `dryWetConn`, `dryWetLive`; `loaded` (the loaded-look name or ""); and after S3
  `menu.saveOver` = {text, enabled} and `box` = {open, title, text, acceptEnabled, taken}.
- POST /api/debug/look_make takes an optional "name" (absent = `nextName`). NEW POST /api/debug/look_replace {scope, layer,
  column, fx, look}.
- POST /api/debug/look_ui gains: type:<text> (sets the open box's field), accept (the open window's first button, through the
  handler the button itself calls; a greyed button does nothing), accept:<text> (type, then accept), cancel.
Fixtures added: Show C = show B's picture, PLAYING, on layer 0 column 0, with clip effects Ripple (fx 0: intensity driven by the
signal "Bass" with range 0.20..0.80; speed driven by an LFO, Sine, 1 Beat) and Ripple (fx 1, plain defaults). Hand-written look
Ripple / "Ghost": "signals": true; intensity 0.35 with a connection to the signal "No Such Signal", range 0.60..0.90; speed and
freq at their defaults with no connection.

| row | does | bar (the exact line printed on pass) | RED arm |
|---|---|---|---|
| GL-1 (re-written) | as RU, with look_make "name": "Wobble" | "GL-1 PASS Wobble on disk at make (segments 0.6100 rotation 0.2700); listed and loaded on a layer in another show after a relaunch; windows unchanged; file label unchanged" | MU-EL-8 |
| GL-4 (second arm added) | RU's table and line stand. Second arm on show C: the same control; then 20 look_loads on fx 1, "Wired" and "Default" in turn, 100 ms apart ("Wired" made in GL-9 (a), pre-written here) | second line: "GL-4 MEASURED wired loads=20 hold=<n> per_load=<x.xx> black=<n> control=<n>/<n>"; RU's decision table applies to each arm by itself | the control arm |
| GL-5 (re-written) | Show A with "Far" pre-written, after S3. look_ui on layer 0, fx 1: pick:New Look; GET looks (the box). type:far; GET looks. accept:Mine; GET looks; read the file. pick:Far on fx 1; GET looks for fx 0 and fx 1; GET looks?stats=1. | "GL-5 PASS menu pick on layer fx 1: the box opened on Look 1, selected; far greyed the button (taken); Mine stored 0.5500; Far set fx 1 to 0.8000; fx 0 kept 0.3000; focus home +3" | MU-EL-23 (Mine stored 0.3000); MU-LA-21 (far did not grey the button) |
| GL-6 (one read added) | RU's table stands, plus one row: keys named conn, conns or dryWetConn anywhere in take.json -- predicted 0; if not, STOP and report | "GL-6 MEASURED control lanes=<n> look lanes=<n> stop-state=<equal|differs> connections=<n>" | the control arm |
| GL-8 (re-written) | full ctest at the lane's final head | "GL-8 PASS ctest <N> = baseline <N0> + 73 new cases, 0 failed" (73 = 22 + 15 + 13 + 21 + 2) | the lane's base |
| GL-9 (new) | Show C with "Ghost" pre-written. (a) look_make "Wired" on fx 0; read the file. (b) look_load "Wired" on fx 1; GET looks fx 1 at once. (c) look_load "Ghost" on fx 1; GET looks. (d) /api/debug/undo; GET looks. (e) /api/debug/undo; GET looks. (f) look_load "Default" on fx 0; GET looks. | "GL-9 PASS wired look: 2 connections stored; plugged on fx 1 with 2 live values at once, matched Wired; Ghost sleeps at 0.3500 (live none), matched Ghost; undo gave 2 connections back, then 0; Default unplugged 2 on fx 0" | MU-LA-12 ("undo gave 0 connections back"); MU-LA-18 ("Ghost live 0.6000") |
| GL-10 (new) | Show B with "Off" and "Far" pre-written, after S3. look_load "Off" on the clip's Invert; POST /api/set_param Invert amount 0.25; GET looks (matched, loaded, menu.saveOver). look_ui pick:Save over "Off"; accept; GET looks; read Off's file; list the folder; compare Far's bytes and time. Quit own pid, relaunch on show B, GET looks. | "GL-10 PASS save over: offered as Save over \"Off\" after the tweak; Off replaced 0.0000 -> 0.2500 after the confirm; matched Off; no new-file left; Far untouched; held after a relaunch" | MU-LA-26 ("after a relaunch 0.0000"); MU-LA-22 ("Save over not offered") |
GL-2, GL-3, GL-7 stand as written.

5.7 The visual gate: 23 states (RU's rules: window-id captures only, each with a manifest; 1280 x 720 unless said)
Stand: V-1, V-2, V-3, V-7, V-8, V-10..V-16, V-18. Re-captured with new content:
V-4 the first menu he sees: Default ticked; New Look, Save over, Rename, Delete greyed. V-5 three looks, one ticked: New Look
greyed, "Save over" greyed, Rename and Delete live. V-6 a loaded look tweaked: nothing ticked, New Look live,
`Save over "Look 2"` live. V-9 the name box for Rename: the present name, selected. V-17 an effect after a WIRED look was loaded:
two sliders show their sources (manifest: each connection, each twin).
New: V-19 the name box for New Look as it opens: "Look 3", selected. V-20 the box with a taken name: the first button greyed,
the name in the warning colour. V-21 the Save Over confirm. V-22 the menu for an effect whose loaded look was deleted: "Save
over" bare and greyed. V-23 a sleeping wire: the row shows the absent signal's name, the slider rests on the look's value
(manifest: the connection, the twin = none).
The critics are also told: the row's source button, its range sliders and its invert mark pre-date the lane.

---------------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; items of RU section 6 that change, and new ones)
---------------------------------------------------------------------------------------------------------
1. (re-written) Put an effect on a clip: its small button reads "Default". Move a slider: it reads "Looks", dim. Press it and
   choose New Look -> a small box opens holding "Look 1", already selected. Press Return -> the button reads "Look 1". Or type
   a name first, then Return -> the button reads that name. Esc -> no look -> wrong: the box opens empty, typing adds to
   "Look 1" instead of replacing it, or a look appears after Esc.
2. (new) While the box is open, type letters that are your clip keys -> they go into the box and no clip fires. After Return or
   Esc your keys launch clips again at once -> wrong: a clip fires while you type, or the keys are dead after the box closes.
3. (stands, one sentence added) ... load a look on a playing layer -> only that effect's picture changes, at once. Now also
   with a look that plugs signals in: no black frame, no stutter, the layer does not restart.
6. (re-written) Signals. (a) Wire the bass to a slider, set its range, make a look. Put the same effect on another clip and
   load the look -> the slider there follows the bass with the same range, from the first frame. (b) Load that look again on
   the first effect -> nothing jumps. (c) Load a look that names a signal you made yourself in another show -> the slider rests
   on the look's value and its row shows the signal's name; make a signal with that name and it starts following. The same now
   happens in a SHOW whose signal is gone: the slider rests on its own value instead of dropping to the bottom of its range.
   (d) Load "Default", or a look you made with no signals -> every slider of that effect is unplugged (question 131). Cmd+Z
   brings values and signals back in one step -> wrong: a slider pinned at one end, a signal left on after Default, an undo
   that brings back values but not signals.
   Macros: a look remembers "Macro 3", not what Macro 3 is doing in this show.
7. (re-written) Rename opens the same box, holding the look's name, selected. Delete as before.
8. (re-written) New Look is grey when the effect already is one of your looks, or Default. `Save over "Look 2"` is live only
   after you loaded or made Look 2 on that effect and then changed something.
8b. (new) Load "Look 2", change a slider, open the menu. New Look -> the box says "Look 3": Return keeps the change as Look 3
   and Look 2 is as it was. Or `Save over "Look 2"` -> a window asks -> Save Over: Look 2 now holds the change, on every effect
   and in every show from now on. There is no undo for that -> wrong: Look 2 changed after New Look; Save over names another
   look; Look 2 is missing or half-changed after a Save over.
10. (one sentence added) A take does not play back a look change, and it never stores which signals are plugged in: playing a
   take back restores the slider values and leaves the signals as they are at that moment.
12. (re-written) If the disk is full or the folder is locked, New Look and Save over do nothing: the button keeps reading
   "Looks" and the old look is untouched. No message.
13. (extended) By eye on your own screens: also the name box and the red name when a name is taken.
14. (re-written) When the old buttons go (the one-save build): the small Save, Load, FX Save and the ten numbered slots are
   gone. Your two old presets were deleted at your word.
Items 2 (of RU: quit, another show, the same effect on a layer), 4, 5, 9, 11 stand.

---------------------------------------------------------------------------------------------------------
## 7 QUESTIONS FOR BORIS (numbers 131..134; each has a default A; nothing waits)
---------------------------------------------------------------------------------------------------------
131. An effect has a signal on one slider. You load a look that was made with NO signal on that slider (or you load "Default").
     A (default) The signal is unplugged: the effect becomes exactly what the look was. Cmd+Z brings it back.
     B The signal stays. A look only ever plugs signals in; it never takes one away.
132. `Save over "Look 2"` replaces Look 2 for good; there is no undo for it.
     A (default) A small window asks first ("Save Over" / "Cancel").
     B It saves at once, with no window.
133. You loaded "Look 2" and moved a slider. The small button on the effect:
     A (default) reads "Looks", dim -- it shows a name only while the effect is exactly that look.
     B keeps reading "Look 2" with a mark that says it was changed.
134 is not used.
What changes with each answer (so that either is a small change)
- 131 B: one enum value in `looks::resolve`, LK-18 / LK-19 as written under 5.1, section 6 item 6 (d). No file format change.
- 132 B: the confirm window is not opened (one branch in the view); V-21 is dropped; HD-15's hidden copy is built.
- 133 B: the button's text rule in `refresh()` reads the loaded-look name when nothing matches; LM-3 and MU-EL-17 are re-cut;
  one more visual state. No store or format change.

---------------------------------------------------------------------------------------------------------
## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
---------------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "Default unplugs my signals" (DA-3, Q131 default A). He wires the bass to three sliders,
   clicks Default to get the sliders back to the middle, and his wiring is gone; the same with any look he made before he
   wired. On stage that is a picture that stops moving. Why A still stands as the default: his option's words are "remembers
   which signal drives each slider", and a look that cannot say "none" can never take him from a wired look back to a plain
   one; under B the name on the button no longer says what is on (a look "matches" while a signal it never held drives the
   effect); and one Cmd+Z brings values and wiring back. Why it is cheap to be wrong: one enum, no format change, the looks he
   has made keep working (Q131). Cheapest refuting test: section 6 item 6 (d), in front of him.
R2 The engine line changes shows, not only looks (DA-4): a connection to a lost signal now rests on the slider's own value
   instead of the range's bottom. A show he has tuned BY EYE around that pinned value would look different. Evidence it is
   rare: only user signals can be lost (C12), and none is saved by any show today (FOS M-6). Refuting test: CE-1; GL-9's Ghost.
   If the council rejects the engine change, fork (b) of DA-4 is the fallback and costs the registry in `matches`.
R3 The after-write hook (DA-5 rule 4) is ASSUMED buildable: it needs the regular tick's Context outside the regular tick, and a
   smoothing step with dt = 0. Refuting test: LC-11 and GL-9 (b) ("2 live values at once"). HD-13 holds the fallback.
R4 `lookName` is a new member of a model struct the render thread reads (DA-9). It is safe only while the renderer never copies
   a slot and never reads the member (C8: true at 185147b by grep). A later lane that snapshots slots on the GL thread would
   race a string. Mitigation: the member's comment says "message thread only"; LC-13 pins the writers. NOT VERIFIED: that no
   place builds an `EffectSlot` by positional brace initialisation (the member goes last so that such a place still compiles).
R5 Save over has no undo (DA-11). A wrong click after the confirm loses the old look for good. Guards: the confirm, the default
   path (New Look) never destroys, HD-15. Refuting test for the write itself: ST-13 (a truncating writer leaves the old bytes).
R6 The loaded look is forgotten when a show is re-opened and the effect was tweaked (DA-9): Save over is then greyed and he
   must make a new look or load one first. Accepted for this lane (HD-14).
R7 `replaceFileIn` is read as one rename on this platform from juce_SharedCode_posix.h:434-437 (J5). NOT VERIFIED: that no
   macOS-specific override replaces it with a non-atomic copy. Builder's step 0 in S1 reads it; ST-13 is the behavioural check.
R8 Save over writes the file fresh: a key a NEWER build put in that look is dropped. Today there is one build and no file.
R9 A greyed button that ignores Return (J6) is the only thing between a taken name and a silent no-op. It is VERIFIED in JUCE's
   source, pinned by LM-17, and the callback's own check makes nothing either way.
R10 `sameConn`'s float tolerance (0.000001) could call two hand-set ranges equal that differ by less than a slider step can
   express (0.001). Accepted: below anything he can set.
R11 Two looks with equal values and different wiring now list as different looks; a first-format look and a signals look with
   the same values both match a plain effect and the first in name order names the button. Harmless, said.
R12 S1 grows from 27 to 37 rows in one builder context. HD-16 holds the split.
R13 A momentary key held while the box opens (DA-12): pre-dates the lane, filed.

---------------------------------------------------------------------------------------------------------
## 9 WHAT IS NOT IN THIS LANE
---------------------------------------------------------------------------------------------------------
- "Routines" become "actions" -- Boris: "I want to change what we are calling routines to actions. Easier to remember". Its own
  lane (every word he sees, the pads, the bands, the docs). This lane writes neither word on screen: the Delete Look and Save
  Over windows are modelled on the Delete Routine window's CODE, not its text.
- Looks that ship with the app (101: later, by him). A conversion of old presets (106: deleted). The removal of the old Save,
  Load, FX Save and the ten slots (one-save S7, unchanged by this delta).
- Saving user signals or the macro bank (FOS M-6): a look names a signal or a macro; it never carries one. A serializer for
  them belongs to the one-save lane's question of what a show holds.
- Connections on a source's parameters, on clip / layer / composition scalars, or on a macro. Looks for procedural sources.
  Bypass in a look. A look of several effects.
- The loaded-look name in the show file (HD-14). A mark on the button after a tweak (Q133 B). A hidden copy of a saved-over look
  (HD-15). A mark on a sleeping wire (HD-17).
- Undo for making, renaming, deleting or saving over a look. Recording a look load, a mouse-moved slider or a wiring change
  into a take (HD-6's lane).
- The signal picker's own writes: plugging a signal by hand stays unfenced and not undoable, as today (C9).
- Loading a look from a key, MIDI, OSC or production REST. A glide between two looks.
- The momentary-key release under a modal window (R13). The stale comment at Clip.h:62.
- The stopped sync-dial branches: nothing is carried from lane/bf2 (740b6d6) or lane/bf2-keys (9eab9bd) and nothing of them is
  dropped by this lane; rulings-bf2.md H-17 already rules them superseded. They were not read for this delta.

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 16:09:01, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-looks-answers.md (status DONE; 23 attacks ruled: 16 ACCEPT, 7 PARTIAL, 0 REJECT; 17 amendments, each
OVERRIDES this delta plan's body). It is a DELTA on ruling-effect-looks.md: precedence for the effect-looks lane is now Boris's
verbatim words > the adoption blocks at the end of plan-effect-looks.md > this adoption > ruling-looks-answers.md >
ruling-effect-looks.md > the plans. Workflow run wf_7f96cbdb-3ec (draft: architect opus high; seats data-safety 8 attacks /
1 MUST, gates 8 / 1, stage-hands 7 / 0 -- papers whole (27,419 characters) in attack-looks-answers-papers.md; ruling:
architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list and decisions, and its section 7 (questions) in
full. NOT read by me: the other sections -- the builders' and reviewers' spec; gate strings only from section 5.
RULED: a look holds one effect's values, Dry / Wet and the connection on each slider; loading leaves an equal connection
untouched and plugs a different one fresh; a slider the look holds without a signal, and "Default", are unplugged by default
(question 131: both behaviours are built, his answer is one constant); a look naming a signal the show lacks loads whole and
that wire drives nothing (one engine line, stage SE); Save over = a complete new file, read back, swapped in with one rename,
read back again; the loaded look is set by his acts, never by a refresh timer; the name box holds "Look N", selected.
HARMONY'S DECISIONS: HD-12..HD-22 at their defaults (HD-17 withdrawn by the ruling). HD-20: the four deck files in Decks and
"test 1.deck.json" in Presets are the one-save lane's to speak about, nothing here. HD-22 (the name box and the Save Over
window of the TEST app show on his screen for seconds during three live rows): accepted, said to him in the handoff; those
rows run only when no Audio-DNA of his is open (the lock helper already refuses).
STAGES of the lane now: (one-save S7 first, in its own lane) S1a, S1b, SE, S2, S3, VG.
Questions 131-134: boris-clarify-131-134.md (asked at the close; each has a default; nothing waits).
NOT STARTED in this session: nothing of this lane is built.
