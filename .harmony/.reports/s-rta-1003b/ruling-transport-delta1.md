# ruling-transport-delta1 -- architect ruling on the blind council's attacks on plan-transport-delta1.md (s-rta-1003b)

Author: architect (opus, max effort). Read-only. Source read ONLY at main 34179a2 (`git show 34179a2:<path>`,
`git grep ... 34179a2`). Nothing built, run, launched or probed. Nothing under `.claude/worktrees/` read.
Labels: VERIFIED = read at 34179a2 at the cited line. INFERRED = reasoned from lines read, not run. ASSUMED = not read.
Boris is quoted only verbatim, from .harmony/binding-decisions.md (:658-660, :684, :688, :725-744) and
.harmony/boris-feedback-backlog.md (:223-225, :288-309). "Harmony constraint:" marks what Harmony requires.
"Reading:" marks an interpretation of his words.
The plan ruled on: .harmony/.reports/s-rta-1003b/plan-transport-delta1.md ("the delta plan").
The papers: .harmony/.reports/s-rta-1003b/attack-transport-delta1-papers.md (3 seats, 22 attacks, whole: the JSON
line is 27994 characters and ends with a closed array).
The base: .harmony/.reports/s-rta-1003b/ruling-transport.md (AM-1 .. AM-30) + the HARMONY ADOPTION at the end of
plan-transport.md (H-1 .. H-5).

HOW THIS FILE IS READ. A builder builds from plan-transport.md's body + ruling-transport.md + THIS file; where they
differ this file wins over the ruling, the ruling over the plan body. The delta plan is NOT read by a builder: every
part of it that stands is restated here in full (sections 3-7). Its ids AM-31 .. AM-36 are replaced by DA-1 .. DA-10.

## 0 VERDICT

THE DELTA PLAN'S SHAPE HOLDS; BUILDABLE from plan-transport.md's body + ruling-transport.md + this file. As written
it would have shipped defects on stage, and four of its gates could not fail. What the council changed most:
- The beat lock, as defaulted, changed the speed of the MOST COMMON clip on every loop (ST-1). Boris: "default all
  bpms to 120" -- so Beats is 2 x the length and almost never whole. A loop that is not a whole number of beats has no
  beat to return to. Default now: the lock runs only for a whole number of beats (within 0.05); any other clip plays
  at the steady tempo rate, as the base built it, and its lines are drawn where its beats are. Which of three ways he
  wants is question Q-D1, first on his page (DA-1).
- The lock's reach is 6 % of the speed, not 10 % (ST-2, ST-3): the room sees the picture, only the operator sees the
  lines. Worst pull-in 9.1 beats instead of 6.8.
- Undo could still change what the strip shows: the "FX" tile and the name of a playing clip that has no media hang
  on its effect list (UN-1, GA-3). The landing rule and EffectStackCmd leave that case (DA-5).
- The base's sentence "it cannot shift a row" is false once an Undo keeps a live layer (UN-2): the pin's Load Deck
  Redo appends its layers at the end and would put that deck's rows, and every later step, one row off. Rule: erase
  by layer id, re-insert at the recorded index (DA-6). A restored layer now really comes back as it was: the three
  commands capture it when they erase it, not when they were first run (UN-3).
- A trimmed loop loses 1 to 2 frames at every wrap (UN-5 and a line the seat did not read: a video's seek call does
  not advance). The lock's unit cases now run the real write-back against a player that wraps the way the real one
  does.
- Gates: TR17 released the clip already on the beat (GA-2); UC16 asserted something no test target can call (GA-5);
  B10 contradicted V14 (ST-4); nothing fed the lock a real hop staircase (GA-4). All four re-registered.
Two findings of my own while re-deriving: the delta plan's "beats 16.016: the trim never exceeds 1 %" is wrong by
its own law (it reaches 2.3 %); and base AM-22's pre-decided slew of the rate (2 % per second) would defeat the lock
for many seconds after a tapped tempo change -- its consequence becomes STOP and report (DA-1).
22 attacks: 11 ACCEPT, 11 PARTIAL, 0 REJECT. 10 delta amendments (DA-1 .. DA-10).
Counts after this ruling: 77 unit cases + 6 lints (+ 1 if S2b); live rows `EXPECTED_ROWS=23`; bars B1-B12; states
V1-V14; measurements M1-M5 and M4b; 39 Boris checks; 6 questions (Q-D1 .. Q-D6); 4 new Harmony decisions (H-D1 .. H-D4).
Nothing waits on Boris: every question has a default. One fact needs a run before S4b starts (FM-5).

## 1 FACTS RE-DERIVED (at 34179a2; every line below read by me)

The layer strip
- RD-1 VERIFIED the strip's thumbnail is the "FX" tile exactly when `clip->hasEffects() && !clip->hasMedia()`
  (src/ui/LayerStrip.cpp:1028, the tile :1044-1063); its clip name is `clip->name` for a source and for a media-less
  clip with effects, the media file's name for a clip with media, empty otherwise (:1067-1097). `hasEffects()` =
  `!effects.empty()` (src/model/Clip.h:249; hasMedia :247, isPlayable :248).
- RD-2 VERIFIED EffectStackCmd with a Clip scope resolves `&clip->effects` by (deckIndex, layerIndex, column) and
  assigns the whole vector on Undo and Redo (src/core/EffectCommands.h:35-43, :99-109). `EffectScope::clip(` occurs
  at src/MainComponent.cpp:701, :1169, :1296, :1372, :1597, :5385. INFERRED reachable: a playing clip with no media
  and one effect; remove that effect in the Clip tab (a step); Cmd+Z puts it back -- the strip's tile and name come
  back with it. Redo takes them away again.
- RD-3 VERIFIED the V fader shows `eff(LayerScalar::Opacity)` when the opacity is connected, else `opacity`
  (LayerStrip.cpp:815-827), and takes the routine cue colour while a lane-rank hand grips it (:829-839). INFERRED: no
  Command class writes `scalarConns` of a layer that exists (the class list of the delta plan's D3; the bodies of
  SetColumnCountCmd and RemoveColumnCmd were not read).
- RD-4 VERIFIED the S fader writes the playing clip's `speed` = 4 x its value (LayerStrip.cpp:404-413) and the timer
  reads it back (:841-852); the forward button writes `reverse = false`, `playing = true`, `speed = min(speed x 2, 4)`
  (:388-393); back writes `reverse = true` (:370-375). The renderer applies `reverse` in both modes
  (src/render/Renderer.cpp:1647-1648, :1707-1708); a BPM-synced VIDEO ignores `clip->speed` (:1657-1667); a
  BPM-synced SEQUENCE still takes it at the pin (:1705-1706) and stops doing so with base TL-U19.
- RD-5 VERIFIED the transport rect is as wide as the strip's left column: `transportBounds_ = (0, transportBarY,
  leftColW, transportBarH)` (LayerStrip.cpp:669). Its width in pixels at the two window sizes was not derived (S0).
  `TransportView` holds `showsClip`, `inPoint`, `outPoint`, `playheadX` (src/ui/LayerStrip.h:58-64).

Layers that come and go
- RD-6 VERIFIED `Composition::insertLayer(at, layer)` inserts at `at` and adds a row at `at` to every live and
  retired deck (src/model/Composition.h:480-495); `insertLayerWithRows` (:499-516); `eraseLayer(index)` (:519-533);
  `moveLayer` (:537-557); `makeLayer` mints the id (:470-477).
- RD-7 VERIFIED AddLayerCmd: the first execute appends and captures `added_` (src/core/DeckCommands.h:499-501); Redo
  re-inserts THAT capture at `addedIndex_` (:495); Undo erases `addedIndex_` (:506-512). RemoveLayerCmd: `removed_`
  is the constructor's copy (:541-547), taken by the handler before the command exists, always of the LAST layer
  (MainComponent.cpp:6838-6845); execute re-snapshots `rows_` but never `removed_` (:560-569); Undo inserts at
  `layerIndex_` (:588). InsertDeckCmd: the first execute appends its layers and captures them (:819-831); Redo
  re-adds `addedLayers_` at the END (:800-801) and then the deck value captured on the first execute (:802); Undo
  erases `getNumLayers() - 1` k times (:863-864).
- RD-8 INFERRED from RD-6, RD-7, base AM-7 and AM-9 (not run). (a) 4 layers; Remove Layer; Add Layer L (index 3); L
  plays; Undo (spent: L stays); Undo -> the removed layer returns at index 3 and L is at 4. L's row NUMBER changes;
  the order of the layers that were there does not; every older step still finds its row. (b) 3 layers; Load Deck
  adds X1, X2 (3, 4); drop Y on row 3; Add Layer L (5); a clip on L plays; Undo x 4 leaves L (kept) at 3; Redo of
  the Load Deck with the pin's append puts X1, X2 at 4, 5: the deck's own rows 3 and 4, and Y's step, are one row
  off (row 3 is L). Re-inserted at their RECORDED indices (3, 4), L is at 5 again and every step lands on the layer
  it was recorded on.
- RD-9 VERIFIED `UndoManager::clear()` empties the history and calls `onHistoryChanged` (src/core/UndoManager.cpp:
  108-113); `undo()` and `redo()` move the index unconditionally (:48-72). UndoManager.cpp is compiled into the undo
  test targets (tests/CMakeLists.txt:366, :571, :618, :1287, :1427, :3310, :3353); "MainComponent.cpp is not linked
  into any ctest target here" (:66-67).
- RD-10 INFERRED (members and class comments in src/core; not every body read): after the base's and this file's
  deletions, 12 Command classes hold a row number or rows by value -- SetClipCmd, SwapClipsCmd, ToggleClipLockCmd,
  ClearLayerClipsCmd (DeckCommands.h:367), SetColumnCountCmd, RemoveColumnCmd, AddLayerCmd (:524), RemoveLayerCmd
  (:607-609), AddDeckCmd (:743), InsertDeckCmd (:890-893), RemoveDeckCmd (:1001), EffectStackCmd (its scope). Three
  hold a whole Deck by value. The delta plan's "about ten" is 12.

The clocks, the wrap, the snapshot
- RD-11 VERIFIED VideoPlayer: a consumed seek REPLACES that call's advance (`if (seekRequested_) {...} else
  advanceTransport(dt)`, src/media/VideoPlayer.cpp:366-377); a whole-file Loop wrap keeps the overshoot (fmod,
  :423-426); the direction of an advance comes from `reverse_`, `loopMode_`, `pingPongForward_` at its top
  (:410-414); `reverseNow_` is set AFTER the clock moved (:456-469); the playhead is `currentTime_ / duration_`
  (:472-473). ImageSequence: a consumed seek is followed by the advance in the same call (src/media/
  ImageSequence.cpp:122-148); its playhead is continuous, `currentTime_ / dur` (:193-194); `getDuration()` = files /
  fps (:98-103).
- RD-12 VERIFIED `writeBack`: when `outPoint < 1` and the playhead is at or past it, a clip that is not a OneShot
  gets `seekTo(inPoint)` and the model playhead `inPoint` (src/render/ClipTransportSync.h:56-72). INFERRED with
  RD-11: every wrap of a TRIMMED loop drops the overshoot, and for a video one frame more (the seek's call does not
  advance): 1 to 2 frames of clip time for a video, up to 1 for a sequence. One frame is tempo / 60 / fps show
  beats: 0.033 at 120 BPM and 60 fps, 0.056 at 200. An untrimmed loop loses nothing.
- RD-13 VERIFIED the frame's one snapshot: `frameSnap_ = featureBus_.read()` (Renderer.cpp:402), `const
  FeatureSnapshot& snap = frameSnap_` (:406), `showAutopilot_.processFrame(*composition_, snap, &apReport)` (:513);
  the release consumes the `totalBeatCount` delta (src/model/Autopilot.cpp:72-76). The delta plan's D16 inference
  holds at the pin. `syncMedia` does its own reads at :1660 and :1721. (INFERRED: :406 and :513 are in one function;
  the lines between were not all read.)
- RD-14 VERIFIED the sequence branch holds `imageSeqMutex_` from Renderer.cpp:1693 to the end of the branch, the
  upload included (about :1761); `getImageSequence` takes the same mutex (:1602-1607); `getVideoPlayer` takes
  `videoPlayerMutex_` for the lookup only and records the message thread's wait (:1584-1591). The base's length
  provider is "wired in MainComponent to the clip's open player" (plan-transport.md:383-385).
- RD-15 VERIFIED the base already moves a sequence's BPM rate into its speed: "Rate in BPM Sync, video and sequence
  alike: rate = show BPM / clipBpm. Sequence fps stays `sequenceFps`; its speed is the rate"
  (plan-transport.md:340-341; base TL-U19). At the pin the rate is in the fps (Renderer.cpp:1718-1731).

Arithmetic (INFERRED: by hand from the law of DA-1; not run)
- RD-16 The law: trim = 0 inside 0.03 beat, else clamp(-e / 2, +-c). From an error E the clip is back inside the
  deadband after (E - 2c) / c + 2 ln(2c / 0.03) beats (first term 0 when E <= 2c). c = 0.10: 6.8 beats from half a
  beat (the delta plan's number). c = 0.06: 9.1 from half a beat, 4.9 from a quarter. c = 0.05: 10.4. c = 0.03: 16.
- RD-17 At a wrap of a loop of B beats the error steps by the distance of B from a whole number. B = 14.6 under the
  delta plan's default: 0.4 after every wrap; at 10 % that is about 6 beats of every 14.6 at a changed speed, on
  every pass (its own text, :246-247). B = 16.016: the error rests at the deadband's edge and steps to 0.046, so the
  trim reaches 2.3 % -- not "under 1 %" (delta plan :246, TL-U25b).
- RD-18 "x2 from a locked clip stays locked" (delta plan :243) is false: locked means frac(x) = phase; after x2 the
  clip beat is 2x and frac(2x) = frac(2 x phase), equal only when the phase is 0. /2 is the same in reverse.
- RD-19 The lock's unstable point: at |e| = 0.5 the wrap flips the trim's sign. Under phase noise n the sign flips
  frame by frame until the clip is n / 2 away, then one side wins. A pull-in that starts there is the worst case
  and has no fixed direction.
- RD-20 TR17 as the delta plan wrote it: a drop at 0.35 of 10 beats is clip beat 3.5; released at driver phase
  0.45-0.55 the error is 0.5 - phase, within +-0.05, and inside the 0.03 deadband for 60 % of that window: the lock
  has nothing to do and a flipped sign changes nothing there.
- RD-21 The hop staircase: the render thread reads a phase that is 0 to one hop old (one hop = 10.67 ms = 0.021 beat
  at 120 BPM, 0.030 at 168, 0.036 at 200), so the lock sees e + s, s in [0, hop). It stops trimming once the true
  error is in [-0.03, 0.03 - hop]: a zone 0.06 - hop wide, not empty below 337 BPM.
- RD-22 Base AM-22's follow-up (a rate slewed at 2 % per second if FM-3 is over its bar) against the lock: a tapped
  change 120 -> 90 leaves the rate up to 33 % fast and takes more than 12 s to arrive; the lock's reach is 6 %; the
  clip slips beat after beat for those seconds. The two cannot both be built.

Boris, re-read (the only source of his words)
- RD-23 "default all bpms to 120" (binding-decisions.md:684) answered a proposal that would have picked a beat count
  of 4, 8, 16, 32 or 64 (boris-feedback-backlog.md:220); Harmony's note of the reading he then confirmed names the
  consequence -- her words, not his: "2 beats per second, possibly not a whole number" (:222-225). "9 b" answers Q9 as asked: "A BPM-synced clip left running for many
  minutes ... B The app keeps nudging it back onto the beat by itself." (:304). The beat-lines sentence (:298). The
  drag: "I wanted to keep playing at the same speed, but play from wherever I drop the play head."
  (binding-decisions.md:658-660). Cmd+Z (:736-738). He ruled nothing about a loop that is not a whole number of
  beats, about easing after a drop, or about the strip's S fader in beats mode.
Carried from the delta plan's fact list (a builder does not read that file)
- RD-24 VERIFIED the strip's controls: X `clearBtn_` -> `onClearClip` (LayerStrip.cpp:341-343) and its tooltip
  (:345-346); B writes `layer_->bypassed` (:347-352); S writes `layer_->solo` (:353-358); the four transport
  buttons write the PLAYING clip's `reverse`, `playing`, `speed` (:370-393); the S fader (:404-413); the K fader
  writes `keyThreshold`, the V fader `opacity`, the V dropdown `keyingMode` or `blendMode`, the F fader
  `transitionSpeed`, the F dropdown `transitionMode` (:414-502). Painted states: the playing clip's thumbnail
  (:1018-1065), the routine bands, the transport rect with the in..out region and the playhead from
  `transportViewOf` (:533-563, :742-757; a press or drag there scrubs, :958-989), the layer name (:565-576), the
  clip name (:1067-1097), the selection outline (:591-596), the fold state (`bandsShown`, LayerStrip.h:124), and
  the ORDER of the strips = the order of `Composition::layers`. The strip shows nothing of a LAYER's effect stack.
- RD-25 VERIFIED ToggleLayerFlagCmd writes `bypassed`, `solo` or `folded` (DeckCommands.h:435-469) and is pushed
  from MainComponent.cpp:749-767 (bypass, solo: the strip) and :6901-6923 (fold: the menu); MoveLayerCmd
  (:619-647) from :6924-6965 (the menu, on the selected layer). `onLayerDragReorder`, `onBlendModeChanged`,
  `onFoldToggle` are declared (LayerStrip.h:94, :99, :100) and assigned nowhere in src (grep at the pin: a strip
  drag does not reorder layers). The Layer tab writes opacity, blend, fade time, key threshold and the layer name straight
  into the model with no Undo step (src/ui/LayerInspector.cpp:150, :178, :169, :324, :339, :22). The history is
  already cleared after a composition load, for the reason DA-7 gives (MainComponent.cpp:3017-3018, :3157-3160).
  Tests that name the two classes: tests/test_undo_commands.cpp:1362, :1394, :1472, :1508-1514, :2549-2557 (the
  random walk's flag step), :2604; tests/test_show_model.cpp:865-882. The Clip tab's own playhead writes:
  src/ui/ClipInspector.cpp:1372 (press) and :1396 (drag), each before `onCuepointJump`; its in and out drags write
  only the point (:1388, :1392).
- RD-26 VERIFIED the beat clock: `bpm` "0 if unknown", `beatPhase` "[0, 1) smooth sawtooth ramp between beats",
  `trackerState` (src/analysis/FeatureSnapshot.h:40-42; LOCKED = 2, :148); `totalBeatCount` and its contract --
  a tempo VALUE changes only the rate, a realign (Tap / Resync / confident detection) moves the phase (:128-137);
  `trackerRequestSeq` (:144; Pitfall 48); Pitfall 42 forbids reading the `beatPhase` WRAP as a clock or an edge.
  Test mode has no analysis thread; `/api/inject_features` sets the snapshot (src/api/ApiServer.cpp:237).
NOT read: BPMTracker.cpp (whether a typed, tapped or Link tempo reports LOCKED and keeps the phase moving: S0);
SetColumnCountCmd and RemoveColumnCmd bodies; the sync-dial lane (bf2); JUCE's source.

## 2 ATTACK RULINGS

| id | sev | verdict | the line that decides it | amendment |
|---|---|---|---|---|
| ST-1 | MUST | ACCEPT | RD-17, RD-23: at his 120 the Beats number is almost never whole, and the delta plan's default then changes the picture's speed on every pass of the most common clip. The room sees the speed; only the operator sees the lines. Default flipped: no trim for a loop that is not a whole number of beats; the lines are drawn either way; check 30 says so; rounding is his third choice. | DA-1, Q-D1 |
| GA-1 | MUST | PARTIAL | Accepted: "lock off" for 2.5 / 6.5 / 14.6 is not his rule and may not read as one -- TL-U25's clause is labelled "Harmony default; Q-D1 open" and Q-D1 leads his page; S4c's builder reads his answer at stage start. Rejected: the seat's default (lock every clip by rounding the target). A 14.6-beat loop cannot keep "BPM 120" AND cross all its lines on beats: a rounded target moves the lines off the Beats number or the BPM box off 120 (RD-23). Reconciled with ST-1: the seats want opposite defaults; the one that changes nothing in the picture wins until he answers. | DA-1, Q-D1 |
| ST-2 | SHOULD | PARTIAL | RD-23: "9 b" answered drift, not a drop; easing after a drop is a reading. Accepted: labelled a reading, asked as Q-D5, and the reach drops from 10 % to 6 %. Rejected: a second, smaller limit after a drop -- the lock is stateless and cannot tell a drop from a fire, and a stored "dropped" flag is the state fork F20 rejected. | DA-1, Q-D5 |
| ST-3 | SHOULD | PARTIAL | RD-19: the steps exist (lock on / off; the sign flip at half a beat) and no gate bounded them. Accepted: every step is at most 6 % of the speed, pinned by a flicker clause in TL-U25; live rows start a quarter of a beat off, where the direction is fixed. Rejected for now: the slew -- one more state per player with its own reset rules, against a step in SPEED with no step in position. It is pre-specified as the follow-up if FM-5 or check 26 shows a visible step. | DA-1 |
| ST-4 | SHOULD | PARTIAL | Delta plan :284 against :663-668: B10 demands lines in a state where the strip draws none. Accepted: B10 and V14 re-registered; S0 reports the strip's widths. Remedy differs from the seat's: lines that would be closer than 3 px THIN to every 2nd, 4th, 8th beat instead of vanishing -- beats mode stays visible on a long clip and every drawn line is still crossed on a beat. No mode mark is added (a new glyph with no word of his). | DA-2, H-D2 |
| ST-5 | SHOULD | PARTIAL | RD-4. Accepted: in BPM Sync the S fader is disabled and the forward button no longer doubles a speed nothing plays (today it would surface when the clip is switched back). Rejected: "the transport buttons do nothing" -- back, pause and play act in both modes; and the fader as a multiplier of the lock (base TL-U19: clip speed is not applied in BPM Sync; his "Their speeds are just controlled differently"). | DA-9, Q-D6 |
| ST-6 | SHOULD | PARTIAL | Same as GA-6. Accepted: Q-D2 and Q-D3 go on his page with his sentence beside each, before S1's code is merged. Rejected: flipping the default to "keep the history" -- RD-10: 12 commands hold row numbers, 3 hold whole decks; and a menu item that "shows why it is empty" is a text announcing an event (AM-27). | DA-7, Q-D2, Q-D3 |
| ST-7 | SHOULD | PARTIAL | RD-15: the seat read the pin, where the rate is in the fps; base S4a moves it into the speed and keeps the fps at `sequenceFps`, so the rate and the trim sit in one term, the speed, and the loop's duration (frames / fps) is untouched. Accepted: lint TL-L5 pins `setFps(` in `syncMedia`; TL-U19's name says "video and sequence". Rejected: a case on the real ImageSequence -- its playhead is continuous (RD-11), so the fake is faithful, and the real one needs image files and the decoder. | section 5 |
| ST-8 | NIT | ACCEPT | RD-18; and the delta plan used D1-D4 for facts and for questions. The sentence is struck; the questions are Q-D1 .. Q-D6; this file's facts are RD-n. | DA-1, DA-10 |
| UN-1 | MUST | ACCEPT | RD-1, RD-2: reachable, and it breaks his "Does not change anything in the layer strip". Same finding as GA-3. A step that would change whether a PLAYING media-less clip has effects is left. | DA-5 |
| UN-2 | MUST | PARTIAL | RD-7, RD-8. Accepted: base AM-7's "it cannot shift a row" is false after a kept layer, and the pin's append in InsertDeckCmd's Redo would misplace rows. Fixed by addressing: erase by id, re-insert at the recorded index (TL-UC20). Rejected: clearing the history after a spent Undo -- it throws away every older step where a rule keeps them right; and "the restored layer may not move the kept one" -- the restored layer must return to its recorded row or the older steps miss it; position is the ORDER of the layers that stay (GA-7). | DA-6 |
| UN-3 | SHOULD | ACCEPT | RD-7: the Layer value is captured at the first run, so Undo then Redo of Add Layer returns a pristine layer, and a second Undo of Remove Layer returns old settings. Each command captures the layer when it erases it. | DA-6 |
| UN-4 | SHOULD | ACCEPT | Base RE-2 / AM-12: one read of the hold per sync. The lock takes `wanted` and `held` only from that sync's Intent. (The effect of a second read is nil -- a paused player ignores its speed -- but one read is the rule, and it costs a parameter.) | DA-1 |
| UN-5 | SHOULD | ACCEPT | RD-11, RD-12, and worse than the seat read: a video loses the seek's frame as well. The fake player wraps and seeks as the real video does and the cases run the real `writeBack`; TL-U25c pins the bounded outcome at 200 BPM, out 0.7. The loss itself is not removed here (DA-1 says why). | DA-1, section 5 |
| UN-6 | NIT | ACCEPT | RD-11: `reverseNow_` is one frame late at a direction change. `movingBackward()` is the direction the NEXT advance takes, from the same expression the advance uses. | DA-1 |
| UN-7 | SHOULD | PARTIAL | RD-14. Accepted: the length provider never takes `imageSeqMutex_` -- a sequence's length is frames / `sequenceFps`, from the Clip's own fields (the base's own definition); a video's is one lookup under the lookup mutex; while unknown the strip asks at most twice a second. Rejected: a GL-written Relaxed length per clip -- the GL thread syncs only DRAWN clips (base R7), so the Clip tab would show no length for an idle clip; and it is a new shared field. | DA-2 |
| GA-2 | MUST | ACCEPT | RD-20. TR17 releases a quarter of a beat off and asserts the error is there before it asserts it is gone; TR18 and TR15 name their fire phase. Not the seat's half beat: RD-19. | section 5 |
| GA-3 | MUST | ACCEPT | Same as UN-1, plus RD-3: the effective opacity the V fader shows joins the field list and the test's StripState. | DA-5 |
| GA-4 | SHOULD | PARTIAL | RD-21. Accepted: TL-U24d feeds the staircase at 90, 120, 168 and 200 BPM; every "inside 0.03 beat" bar reads "<= 0.03 + 1e-6"; FM-5 reads `playheadPosition`, not pixels. Rejected: FM-5 at 0.5 % as a bar -- a slide after a realign is over it by design; the 0.5 % share is reported as M4b. | section 5 |
| GA-5 | SHOULD | ACCEPT | RD-9. The move and the clear are ONE function in a header the undo tests link; TL-UC22 is its behaviour case; MU-40 turns that case RED, not a lint. | DA-7 |
| GA-6 | SHOULD | PARTIAL | Same as ST-6. The "measurement" the seat asks for is done by reading: RD-10. The default stays; it is flagged on his page; S1m stays pre-specified. "Not undoable but keep the history valid" does not exist without S1m: the older steps name rows by number. | DA-7, Q-D3 |
| GA-7 | SHOULD | ACCEPT | A row number is not a strip field; the order of the strips is. StripState's position is the order of the layer ids that exist before and after the step. Q-D2 is on his page with the others. | DA-5, DA-6 |

Seats reconciled, not relayed: ST-1 against GA-1 (the default for a loop that is not whole: opposite proposals, one
question); UN-1 / GA-3 (one finding); ST-6 / GA-6 (the history after a layer move); UN-2 / GA-7 (what "position"
means); ST-2 / ST-3 (both are answered by the smaller reach, neither by new state); GA-4 / UN-5 (what the lock's
unit cases must feed it).
22 rows: ST 8, UN 7, GA 7. 11 ACCEPT, 11 PARTIAL, 0 REJECT.
The seats' three strongest points, weighed. Stage operator (ST-1): accepted whole -- it is the change that matters
most. Undo and threads (UN-2): the facts hold and the base ruling was wrong on that sentence; the remedy is
addressing, not a second history clear. Gates (GA-2): accepted; its runner-up (GA-1) is right that the default is
not his word and wrong about which default.

## 3 DELTA AMENDMENTS (each says what of ruling-transport.md it REPLACES or ADDS TO; full text)

DA-1 .. DA-10 override ruling-transport.md where they differ, and replace the delta plan's AM-31 .. AM-36 whole.
Forks after this file (id | choice | runner-up):
F20 | beat-phase lock modulo one beat, stateless | anchored position lock with a stored offset
F21 | bounded speed trim (6 %, deadband 0.03 beat, 2-beat time constant) | seek to the aligned position
F22 | a loop that is not a whole number of beats: no lock, the steady rate (Harmony default; Q-D1) | pull it back on every pass; or round the app's own Beats number
F23 | beat lines in the Clip tab AND the layer strip | Clip tab only
F24 | the bar keeps the whole file, the playhead is clamped to in..out | the bar rescaled so in..out fills it
F25 | a point dragged past the playhead carries it | the playhead left until it wraps
F26 | a layer move is no step and ends the Undo history | keep the history, re-number every step (S1m)
F27 | a restored layer returns as it was when erased, empty tuple, solo off | solo restored as it was
F28 | lines closer than 3 px thin to every 2nd, 4th, 8th beat | none when too dense, plus a mode mark
F29 | kept layers: erase by id, re-insert at the recorded index | clear the history after a spent Undo
F30 | in BPM Sync the strip's S fader is disabled | the fader scales the beats
Base forks: F6 = the hold (AM-12's form). F9 = the beat lock (DA-1). F13 widened (DA-5). All others as the ruling
left them.

### DA-1 BEAT LOCK (his "9 b" and the beat-lines sentence)

REPLACES: plan fork F9's "rate only" and the plan's sentence "The POSITION is integrated from the rate; nothing is
derived from beatPhase or totalBeatCount"; AM-22's first line "F9 stands", the CONSEQUENCE in its FM-3 bullet, and
its FM-4 bullet; 5.6's FM-3 consequence and FM-4. ADDS to AM-12 (the Intent is the lock's input). CLOSES base Q9.
BORIS. "9 b" (to: "B The app keeps nudging it back onto the beat by itself."). And: "when the Video is in beats
rather than adjust by speed mode, the beats are shown with lines in the play head area whereas if it was speed
control, it's just the basic play head and with beats control there are lines for each beat in the play head area
and the play head moves past them on time".

WHAT IS LOCKED TO WHAT
- The clip's beat coordinate: x = (playhead - inPoint) / (outPoint - inPoint) x beats, beats = L x clipBpm / 60 as
  in the base (L = the in..out length in seconds). A beat line sits at every whole x.
- The show's beat phase: the frame snapshot's `beatPhase` -- the fractional part of `totalBeatCount + beatPhase`.
- The error e = wrap(frac(s x x) - beatPhase) into [-0.5, 0.5); s = +1 when the next advance runs forward, -1 when
  it runs backward. e > 0: the clip is ahead. e < 0: the clip is behind.
- It is a lock modulo ONE beat with no memory: it does not say WHICH line is on which beat and does not put the
  loop's start on a bar. It compares two phases; it never detects a wrap, never counts a beat, keeps no baseline
  (Pitfall 42 cannot bite). Reading: "lines for each beat in the play head area" and "the play head moves past
  them on time" ask for exactly this; "bars will confuse this" speaks against a bar lock.

THE LAW: a bounded rate trim, never a seek
- speed pushed = rate x (1 + trim); rate = the base's `bpmSyncRate(show BPM, clipBpm)`.
- trim = 0 while |e| <= kBeatLockDeadband (0.03 beat); else clamp(-e / kBeatLockTauBeats, -kBeatLockMaxTrim,
  +kBeatLockMaxTrim), with kBeatLockTauBeats = 2.0 and kBeatLockMaxTrim = 0.06.
- So: near the lock the error shrinks with a time constant of 2 show beats; far from it by 0.06 beat per beat.
  From half a beat off: 9.1 beats (4.6 s at 120). From a quarter of a beat: 4.9 beats (RD-16). One clip beat passes
  per show beat at trim 0, so the law is the same at every tempo.
- Why 6 % and not the delta plan's 10 % (ST-2, ST-3; a number chosen by reading -- INFERRED, FM-5 and checks 26-28
  judge it): the reach is the size of every speed step the lock can make (on, off, the sign flip half a beat away,
  RD-19) and of every slide. The ROOM sees the picture's speed; only the operator sees how many beats a pull-in
  takes. 6 % costs 2.3 beats on the worst pull-in and under one beat on a typical one.
- Why the deadband (INFERRED; TL-U24d pins it): the phase the render thread reads is up to one analysis hop old
  (RD-21). A lock with no deadband would move the speed on that noise in every frame; with 0.03 it stops trimming
  once the clip sits inside the zone of RD-21, at every tempo up to 337 BPM.
- The trim is a pure function of this frame's position and this frame's snapshot. NO state: no field on Clip or
  Layer, none on the player, none in the renderer beyond the base's tempo store, no slew. No new shared field
  (Pitfall 63). It calls `setSpeed` only: no `seekTo`, no `restart`, no write of `playing` or `playheadPosition`.
- The slew ST-3 asked for is NOT built. If FM-5 or check 26 shows a visible step in speed, it is the pre-specified
  follow-up: `float beatTrimSlew(float target, float last, double dtSeconds)` (at most 0.2 per second), one
  GL-thread float per player, reset by a restart and by a hold. Its own unit case then.

WHEN THE LOCK IS OFF (speed pushed = the rate, exactly)
- this sync's Intent is not `wanted`, or it is `held` (DA-3); the show `bpm` is 0 or `trackerState` is not LOCKED;
  the length is not known (0); `beatLockApplies(beats)` is false. (Timeline mode never reaches the lock.)
- `beatLockApplies(beats)` = round(beats) >= 1 and |beats - round(beats)| <= kBeatLockWholeTol (0.05).
  HARMONY DEFAULT; Q-D1 OPEN -- not Boris's word. A loop that is not a whole number of beats has no beat to come
  back to: every pass starts frac(beats) later against the beat. It plays at the rate -- as the base built every
  BPM-synced clip -- and its lines pass off the beat. Its lines are drawn all the same (DA-2).
  16.016 (a 29.97 fps file): locked; the error rests at the deadband's edge and each wrap steps it to 0.046, a trim
  of at most 2.3 % for about a beat (RD-17). 14.6, 6.5, 2.5: not locked. /2 on an odd number of beats (15 -> 7.5)
  lets the lock go; x2 brings it back.
- THE OTHER TWO WAYS, pre-specified for his answer to Q-D1 (S4c's builder reads the answer at stage start):
  (B) "pull it back on every pass": `beatLockApplies` reads "the distance of beats from a whole number <=
  kBeatLockMaxTrim x beats / 2". 14.6 then locks and runs up to 6 % slow for about 7 of its 14.6 beats on every
  pass; 6.5 and 2.5 still do not lock. TL-U25 and TL-U25b take the wording given in 5.1; check 30 the wording given
  in section 6.
  (C) "round the app's own number": when a clip is switched to BPM Sync for the first time its clipBpm is set so
  that beats = max(1, round(2 x L)); the BPM box then shows a number near 120 instead of 120. The rule above stays.
  One line in S4b's first-switch path, one clause in TL-U20, V2's expectation changes. It contradicts the letter of
  "default all bpms to 120", so only he can choose it.
- If S0 finds that LOCKED with a `beatPhase` that stands still is reachable (test mode after `set_bpm`; perhaps a
  typed tempo): one more off-condition, "the beat time `totalBeatCount + beatPhase` did not advance since this
  player's last sync" -- one GL-thread double per player, the only state this amendment allows, and only then;
  TL-U25 gains the clause. If S0 finds that a typed or tapped tempo does not report LOCKED: the tracker condition
  becomes "`bpm` > 0 and the beat time advanced since this player's last sync" (the same double). S0 says which;
  S4c writes it into its report.

ONE SNAPSHOT, ONE INTENT, ONE DIRECTION
- The lock reads `frameSnap_`, the snapshot the quantized release got in that frame (RD-13). The two
  `featureBus_.read()` in `syncMedia` go. Else a clip released on a beat can start with an error the release did not
  see. S0 re-reads this on the merged main, and whether the sync-dial lane leads the phase the release uses: the
  lock uses the same phase as the release, whatever that is by then.
- `wanted` and `held` come ONLY from the Intent `pushIntent` returned in this sync (AM-12's rule, RE-2). The BPM Sync
  branch reads neither `clip->playing` nor the hold record.
- s comes from `movingBackward()`: the direction the NEXT advance will take, computed from the same expression the
  advance uses (RD-11: `reverse_`, `loopMode_`, `pingPongForward_`) -- never from `reverseNow_`, which is one frame
  late when a clip is reversed.

SIGNATURES
New header src/render/ClipBeatGrid.h -- pure; includes no player and no JUCE; used by the renderer, the Clip tab,
the layer strip and the tests:
- `double clipBeatAt(double playhead, float inPoint, float outPoint, double beats) noexcept;`
- `struct BeatLines { int count; int stride; double firstNorm; double stepNorm; };`
- `BeatLines beatLinesOf(float inPoint, float outPoint, double beats, double barWidthPx) noexcept;` (DA-2)
- `double beatLockError(double clipBeat, double showBeatPhase, bool backward) noexcept;`
- `float beatLockTrim(double error) noexcept;`
- `bool beatLockApplies(double beats) noexcept;`
- constants `kBeatLockDeadband = 0.03`, `kBeatLockTauBeats = 2.0`, `kBeatLockMaxTrim = 0.06`,
  `kBeatLockWholeTol = 0.05`, `kBeatLineMinPx = 3.0`.
In src/render/ClipTransportSync.h, templated on the player like `pushIntent` and `writeBack`:
- `template <class Player> float bpmSyncSpeed(const Clip& clip, const Player& player, const Intent& intent,
  float rate, double showBeatPhase, bool beatClockLocked);` -- the speed to push in BPM Sync: the rate, or the rate
  x (1 + trim). It reads the player's position (before the advance), its duration and `movingBackward()`.
- The player concept gains `double getDuration() const; bool movingBackward() const;`.
In src/media: `bool VideoPlayer::movingBackward() const noexcept;` and the same on ImageSequence (GL thread only).
Each of the renderer's two BPM Sync branches reads `setSpeed(ClipTransportSync::bpmSyncSpeed(...))`; nothing else
in `syncMedia` computes a BPM Sync speed. The unit cases drive pushIntent -> bpmSyncSpeed -> setSpeed -> advance ->
writeBack: the renderer's own order, with the real `writeBack`.

HOW IT LIVES WITH HIS OTHER RULINGS
- A fire is never moved to fit the beat ("restart"): the clip starts at its start (reversed: at its end) at once,
  or on the bar under Quantize. Fired BETWEEN beats with Quantize off, a clip of a whole number of beats then
  slides onto the NEAREST beat. The base's removal of the per-clip beat-phase seek stands.
- Under Quantize the clip starts on the frame of the beat: its first error is under one frame.
- The playhead drag: "I wanted to keep playing at the same speed, but play from wherever I drop the play head." The
  lock never seeks, so a clip plays on from the drop point, always. In speed mode nothing else happens. In beats
  mode the lock then eases it onto the nearest beat: at most half a beat, at most 6 % off its speed. READING, not
  his word: "9 b" answered a question about drift, and "moves past them on time" does not mention a drop. Q-D5 asks
  him; check 27 shows it to him.
- The hold (DA-3): a held clip is not trimmed; at Up it plays on from the held frame, then slides.
- A tempo change (typed, a tapped value, Link, REST): only the rate changes; the phase is continuous. The lock reads
  only the snapshot, so it follows a command one hop late (Pitfall 48); no code of this lane acts on "the tempo I
  just sent". The rate must follow the tempo AT ONCE: base AM-22's pre-decided slew of the rate is withdrawn
  (RD-22). If FM-3 is over its bar: STOP and report; no change is pre-decided.
- A Resync, a Tap, a confident detection: the phase is realigned. The clip does NOT jump: it sees a new error of at
  most half a beat and slides. A Resync does not restart a clip (NOT in this lane).
- Reverse: s = -1; the same lines are crossed on the beat. Reversing a clip in mid-loop mirrors its phase: an
  error of up to half a beat, and a slide. PingPong: s follows the leg; at a turn with a whole number of beats the
  phase is continuous.
- x2, /2, a typed BPM or Beats, a moved in or out point: x changes scale, the error steps by up to half a beat, and
  the clip slides -- or the lock lets go, or takes hold, as the number stops or starts being whole. Nothing seeks.
  (Struck: the delta plan's "x2 from a locked clip stays locked", RD-18.)
- A TRIMMED loop (out point below 1): every wrap loses 1 to 2 frames (RD-12). The lock pulls them back: after each
  wrap a trim of a few per cent for one to three beats, and the clip rests up to 0.03 beat behind the beat.
  NOT removed in this lane: carrying the overshoot through the wrap changes `writeBack`'s seek for every trimmed
  loop in both modes, and the lost frame of the seek is inside `VideoPlayer::advanceFrame` (Pitfalls 56, 62). If
  TL-U25c or FM-4 cannot be met: STOP and report; the carry is the named candidate, nothing is pre-decided.
- A layer that is not drawn (bypassed, hidden, muted by solo): `syncMedia` does not run (base R7); when it is drawn
  again the clip slides.
- AM-22's store (the last tempo read while LOCKED) stands. TL-U19's name gains its clause (5.1).

Tests: 5.1 (S4c). Live rows TR15 .. TR18. Facts FM-4, FM-5.

### DA-2 BEAT LINES (BF35)

REPLACES AM-18's second bullet ("The bar's beat lines ...") and the beat-line clause of TL-U17's name.
- Both surfaces draw `beatLinesOf(inPoint, outPoint, beats, barWidthPx)`: only in BPM Sync, only when the length is
  known, only when beats >= 2. `barWidthPx` = the width in pixels of the WHOLE bar on that surface (the range
  in..out covers (outPoint - inPoint) x barWidthPx of it). In Timeline mode neither surface draws a line.
- One line per whole beat b, 1 <= b < beats - 0.001, at inPoint + (outPoint - inPoint) x b / beats.
- THINNING (F28): when two neighbouring lines would be closer than kBeatLineMinPx (3 px), `stride` = the smallest of
  1, 2, 4, 8, 16, ... whose spacing (outPoint - inPoint) x barWidthPx x stride / beats is at least 3 px; the lines
  are at b = stride, 2 x stride, ... < beats - 0.001. `count` is 0 when no such line exists. The base's "none above
  64 beats" is gone: a long clip in beats mode still shows lines, and the playhead still meets every drawn line on
  a beat.
- One weight and one colour for every line: no heavier line every fourth beat, no label on a thinned line; nothing
  marks a bar on a clip (DA-8).
- The lines are drawn whether or not the lock applies (DA-1): they are where the clip's beats are.
- LINES AND PLAYBACK CANNOT DISAGREE: line b is, by definition, where `clipBeatAt` returns b, and the lock's error
  is computed from `clipBeatAt`. TL-U21 pins the identity.
- Clip tab: the loop of base R29 fed from `beatLinesOf`. Nothing else of the bar's painting changes. AM-18's
  `beatLineCount(beats)` is `beatLinesOf(...).count`.
- Layer strip: `TransportView` gains `int beatLines; int firstLineX; float lineStepPx` -- what paint() draws, in
  pixels, so the change test compares what is painted (Pitfall 59) -- and
  `transportViewOf(const Clip*, juce::Rectangle<int> bounds, double mediaSeconds)`. The lines change only when the
  beats, the in point, the out point or the rect change, so the strip repaints no more often than today.
- THE LENGTH. `std::function<double(const Clip&)> LayerStrip::mediaSeconds`, forwarded by DeckView, assigned in
  MainComponent to the base's length provider. The provider, for both surfaces: a SEQUENCE's length is
  `sequenceFiles.size() / sequenceFps`, from the Clip's own fields -- no lookup, never `getImageSequence`, so the
  message thread never waits on `imageSeqMutex_` (RD-14); a VIDEO's is `getVideoPlayer(id)->getDuration()`, one
  lookup under the lookup mutex; 0 = not known.
  The strip asks it in `refresh()`, when the layer's playing clip changes, and -- while the answer is 0 -- at most
  on every 15th timer tick (twice a second). Never in paint().
- Test routes (test mode): `GET /api/debug/strip_transport_ui?layer=N` -> the transport rect, the in and out x, the
  playhead x, the line xs, the stride, and the S fader's `enabled` (DA-9). `GET /api/debug/clip_transport_ui` also
  reports the bar's line xs, its stride and its playhead x.

Tests: TL-U21 (S4a), TL-U30 (S4b). Live: TR18. Bars B10, B11; states V12 .. V14.

### DA-3 THE HOLD IS BUILT (his "2 b")

REPLACES the words "ONLY if Boris answers Q2" in AM-12 and "(only if ...)" in section 4's S3h, in 5.1's
"Conditional stages" (for S3h), in G-U1, G-U4 and in 5.3's TR8. CLOSES base Q2. Retires risk K2.
BORIS. "2 b" (to: "B The picture waits on that frame until you let go.").
- S3h is a normal stage, after S3. AM-12's record, `Intent`, its end conditions and its cases stand word for word.
- AM-11's fourth bullet reads: "While the mouse is down the picture waits on the frame under it. No seek on
  mouseUp: after a drag the clip plays on from the last position."
- The hold and the lock (DA-1): while a clip is held the lock is off. At Up the clip plays on from the held frame --
  no seek, no jump -- and a BPM-synced clip of a whole number of beats then slides onto the nearest beat. A
  speed-mode clip is not trimmed at all.
- The hold and the clamp (DA-4): a clip held past the out marker waits just inside it; at Up a Loop starts over and
  a OneShot ends (AM-13).
- A marker drag is not a hold (DA-4).
Gates, all unconditional: TL-U11, TL-U11b, TL-U11c [tsan], TL-U12, TL-U12b, TL-U12c; MU-H1 .. MU-H3; live row TR8.

### DA-4 THE RANGE IS THE TIMELINE (his answer to 10)

ADDS to AM-11 and AM-13. CLOSES base Q10 (AM-13's outcome is his rule's outcome: the end marker is the end of the
timeline).
BORIS. "If we set the inpoint and endpoint on the timeline of the clip, I should not be able to drag outside of the
points. It should be as if that is the extent of the timeline unless I let go and drag one of the in or out points."
- During a drag, both surfaces: MET by AM-11 -- every press and every drag event goes through `scrubClip`, which
  clamps to [inPoint, outPoint - 1e-4] before it seeks and before it writes the model.
- Added 1: src/ui/ClipInspector.cpp has NO direct playhead write (at the pin it writes `clip_->playheadPosition`
  itself, unclamped, before the callback -- base R15: :1368-1375, :1395-1399). Its handlers only call the scrub
  callback. Lint TL-L3 (5.1).
- Added 2: in the Clip tab's in and out drags, after the point is written: when the clip's playhead is outside
  [inPoint, outPoint - 1e-4] the handler calls the scrub callback with the clip's id and its current playhead; the
  clamp lands it on the moved point. No hold is taken for a marker drag: from the in point the clip plays on
  inside the range; at the out point a Loop starts over and a OneShot ends, inside the range.
- A press or a drag in the dark part of either bar lands on the nearer point. Nothing is written on screen.
- The bar keeps showing the whole file with its in..out region (F24); it is not rescaled -- on a rescaled bar both
  points sit on its ends and cannot be dragged outward ("unless I let go and drag one of the in or out points").
  Reading.
- NOT in this lane: reverse PLAY leaving a trimmed range (base SF-5); a point set by anything but the Clip tab's
  drag.
Tests: TL-U15 stands; TL-U15b (5.1). Mutant MU-42 no carry -> TL-U15b (live: TR7d). MU-10 (no clamp) stays.
Live: TR7c gains a clause; TR7d (5.3).

### DA-5 THE STRIP RULE (his answer to 3)

REPLACES AM-10 whole and the rows of section 10's table named below. ADDS to AM-6 (outcomes 1 and 3) and to F13.
CLOSES base Q3.
BORIS. "cmd z Does not change anything in the layer strip which is by default live based". It widens his rule of
the afternoon: "Let's not allow control Z to change anything that is live in the layer strip. It changes anything
else".
- THE RULE (reading, by FIELD, not by widget). No Undo and no Redo writes a strip field of a layer that exists both
  before and after the step.
- STRIP FIELDS. Of the LAYER: name, bypass, solo, fold, opacity and what the V fader shows of it (the connected
  value, RD-3), key threshold, keying mode, blend mode, fade time, fade mode, its place in the ORDER of the layers
  that exist before and after the step (not its row number: DA-6), and its tuple (what it plays, what fades out,
  what is queued). Of the clip it PLAYS: which media, its name, whether it shows as an FX clip (no media and at
  least one effect, RD-1), and its whole transport (AM-6's `TransportState`).
- EDITS THAT STOP BEING UNDO STEPS

| edit | today | after |
|---|---|---|
| a fire, a layer's X | a step | no step (base AM-6 / TL-UC1) |
| Bypass, Solo (strip buttons) | ToggleLayerFlagCmd (MainComponent.cpp:749-767) | no step |
| Fold (menu) | ToggleLayerFlagCmd (MainComponent.cpp:6901-6923) | no step |
| Move Layer Up / Down (menu) | MoveLayerCmd (MainComponent.cpp:6924-6965) | no step; the Undo history ends there (DA-7) |
| opacity, key, blend, keying, fade time, fade mode, layer name | no step | no step -- unchanged |
| the strip's transport buttons, S fader, playhead, in / out points | no step | no step -- unchanged |

  `ToggleLayerFlagCmd` and `MoveLayerCmd` are deleted with their tests (RD-25:
  tests/test_undo_commands.cpp:1362, :1394 deleted; :1472, :1508-1514, :2604 lose their lines; the random walk's
  flag step :2549-2557 becomes a direct write with no step; tests/test_show_model.cpp:865-882 drives
  `Composition::moveLayer` directly). G-U2's "cases deleted" counts them. The three flag handlers keep their live
  write and their refresh and push nothing. `Command::affectsLayerOrder` and what only MoveLayerCmd fed
  (CompositeCommand.h:50-53, UndoManager.cpp:96-106, the parameter of `refreshAfterUndoRedo(bool)`) go with it if
  the builder's grep finds no other producer.
- AM-6, AMENDED. Outcome 1 reads: "The cell is live and the state is empty, another clip id, the same id with OTHER
  media, or the same id with no media on either side where exactly one of the two has effects: LeftLive. Nothing is
  written; no media hook; no dispose." Outcome 3 reads: "The cell is live, same id, same media: the state lands and
  the live clip's transport AND its name are put back -- AppliedKeepingTransport." `Clip::TransportState` is
  unchanged; `landClipInCell` keeps the name beside it.
- EffectStackCmd STAYS a step in all three scopes. Reading: a layer's and a clip's effect list is shown in the Layer
  tab and the Clip tab, not in the strip, so it is "anything else" -- also when the effect was dropped on the strip
  (Q-D4). ONE exception, Clip scope: when the cell is live, the clip has no media, and the saved list is empty
  while the clip has effects, or the reverse, nothing is written and the step is spent -- the strip's FX tile and
  the clip's name hang on exactly that (RD-1, RD-2).
- SetClipCmd, SwapClipsCmd, ClearLayerClipsCmd: AM-6 as amended. AddDeckCmd, InsertDeckCmd, RemoveDeckCmd: no tuple
  write except the base's one (AM-8). ToggleClipLockCmd, RenameDeckCmd, SetColumnCountCmd, RemoveColumnCmd: no
  strip field; as the base rules them.
- THE SAME FIELD EDITED ELSEWHERE. The rule is about the field. Opacity, blend, key threshold, fade time and the
  layer name set in the Layer tab are not Undo steps today (RD-25) and never become one in this lane; the same holds
  for a write by REST, MIDI, OSC, a routine or a take. No later lane may add an Undo command that writes a strip
  field (lint TL-L2; docs).
- A WHOLE LAYER MAY STILL COME AND GO WITH CMD+Z. He kept, by "all defaults": a removed layer comes back with
  Cmd+Z, not playing (base Q5); a loaded deck's added layers stay when its clip plays (base Q8). Reading: adding
  and removing a whole layer is an edit of the show; the rule protects what an EXISTING strip shows. Q-D2 asks him.
  How such a layer is addressed, captured and restored: DA-6.
- THE X BUTTON'S TOOLTIP promises "the clip comes back with Undo" (LayerStrip.cpp:345-346); after the base that is
  false. It reads: "Clear this layer's clip. Also stops every routine playing on this layer. This cannot be
  undone." -- a standing description, not an event text (AM-27). S1.
- F13 reads: "live" = AM-6's three slots and the clip's media, name, FX look and transport; "strip field" = the
  list above, live or not.
- SECTION 10's TABLE, the rows that change (every other row stands):

| command | Undo | Redo |
|---|---|---|
| ToggleLayerFlagCmd, MoveLayerCmd | deleted: bypass, solo, fold and a layer move push no step; a layer move clears the history (DA-7) | -- |
| SetClipCmd, SwapClipsCmd, ClearLayerClipsCmd | as section 10, with AM-6's outcomes 1 and 3 as amended here | same |
| EffectStackCmd | as today; left on a live clip with no media when it would change whether the clip has effects | same |
| AddLayerCmd | a live layer stays and the step is spent; an idle one is captured, then erased by id | re-inserts what the Undo erased, at its recorded index, through `restoredForUndo` (DA-6) |
| RemoveLayerCmd | the layer returns at its recorded index through `restoredForUndo`: its settings as when it was erased, an empty tuple, solo off | REFUSED over a live layer (AM-7); else it captures the layer and erases it by id |
| InsertDeckCmd | as section 10; the layers it erases are captured first and erased by id | as section 10; the layers it re-inserts go to their recorded indices through `restoredForUndo` |
| RenameDeckCmd, ToggleClipLockCmd | as today | as today |

Tests: TL-UC16 (renamed), TL-UC17, TL-UC19, TL-UC21; lint TL-L2 (5.1).

### DA-6 LAYERS THAT COME AND GO: addressing, capture, restore

REPLACES two bullets of AM-7: "The Undo direction needs no refusal: every collision there is ... and the other
Undos only put things back." and "AddLayerCmd and InsertDeckCmd find their added layers by layer ID (the plan said
so for InsertDeckCmd only)." ADDS to AM-9 and to section 10's rows for AddLayerCmd, RemoveLayerCmd, InsertDeckCmd
(DA-5's table). AM-7's refused Redo, its `lastRunRefused`, and every other bullet stand.
- WHY. A spent Undo keeps a live layer (section 10: AddLayerCmd; AM-9 keeps ALL of a Load Deck's layers). The stack
  is then taller than every older step was recorded on. The base said this "cannot shift a row"; it can (RD-8).
- ADDRESSING (F29).
  1. On their first execute AddLayerCmd and InsertDeckCmd append at the end, and Remove Layer takes the last layer
     -- as today.
  2. Each of the three commands records, for every layer it adds or removes, the layer's id and the index it had.
  3. ERASE is by id: AddLayerCmd's Undo, InsertDeckCmd's Undo and RemoveLayerCmd's Redo find the layer by its id,
     wherever it is. An id that is not in the stack erases nothing.
  4. RE-INSERT is at the RECORDED index (clamped to the count), never at the end: AddLayerCmd's Redo,
     InsertDeckCmd's Redo, RemoveLayerCmd's Undo.
  5. A layer move ends the history (DA-7), so a recorded index never goes stale by a move.
- WHAT FOLLOWS (INFERRED, RD-8; pinned by TL-UC20): whenever a step's Undo or Redo runs, every row it names by
  number holds the layer it held when the step was recorded; kept layers sit above the rows the older steps know.
  A kept layer's row NUMBER can change when a layer below it comes or goes. The ORDER of the layers that stay never
  changes, and the picture does not change: a layer that comes back plays nothing.
- "POSITION" in the strip rule and in the tests = the order of the layer ids that exist before and after the step.
- CAPTURE. Each of the three commands takes its copy of a Layer at the moment it ERASES it -- AddLayerCmd in
  undo(); InsertDeckCmd in undo(), for each layer it erases; RemoveLayerCmd in execute(), on every run -- not on
  its first run and not in the menu handler (RD-7). So a layer comes back with the name, bypass, fold, opacity,
  key, keying, blend, fade time, fade mode and effects it had when it went.
- RESTORE. `Layer restoredForUndo(Layer captured)` (core/DeckCommands.h): the captured value with an EMPTY tuple
  (base F15) and SOLO OFF. Used by the three re-insert paths. Solo is the one setting dropped because it is the one
  that reaches OTHER layers: a restored layer that solos would black out every layer that plays (F27; runner-up:
  restore solo as it was -- loses: Cmd+Z would change the live picture, the thing both of his rules forbid).
- AM-9 stands and is his now (DA-10).
- Not changed: the rows of a layer that AddLayerCmd or InsertDeckCmd erases are empty when that step is reached
  (every clip a later step put there was taken out by that step's Undo; if a clip there plays, the layer is live
  and stays). S0 lists every path that puts a clip into a cell without an Undo step; if one exists, the two
  commands snapshot and restore those rows the way RemoveLayerCmd does (DeckCommands.h:560-568, :588), and S1's
  report says so.

Tests: TL-UC18, TL-UC20 (5.1). Mutants MU-38, MU-48, MU-50.

### DA-7 A LAYER MOVE ENDS THE UNDO HISTORY

REPLACES section 10's MoveLayerCmd cell and AM-10's sentences on layer order (AM-10 is replaced whole by DA-5).
- WHY. The older steps name rows by number (base R17; 12 command classes, RD-10). Today a move is a step, so an Undo
  unwinds it before it reaches them. Once a move is no step, an older Undo would land on another layer's row. The
  tree already ends the history for this reason after a composition load (RD-25: MainComponent.cpp:3017-3018,
  and :3157-3160 "a stale command left alive could re-resolve, by coordinate, a valid-but-WRONG cell").
- ONE function, in a header the undo tests link (core/DeckCommands.h):
  `bool moveLayerEndingHistory(Composition& show, UndoManager& history, const DeckFenceHook& fence, int from, int to);`
  -- the fenced `Composition::moveLayer`; when the move happened, `history.clear()`; it returns whether it moved. A
  refused move (same index, out of range) leaves the history as it is.
- Both menu handlers (Move Layer Up, Move Layer Down) call it, and nothing else moves a layer: outside
  src/model/Composition.h, `moveLayer(` occurs in src only inside that function (lint TL-L2). There is no
  `MainComponent::moveLayerNoUndo`.
- Silent (AM-27): `clear()` already tells the menu (`onHistoryChanged`, RD-9); the Undo and Redo items are disabled
  after a move -- a state, not an event text. (INFERRED from RD-9 and base R39: the item's state is `canUndo()` /
  `canRedo()`; the hook's body was not read. Check 36 shows it.)
- HARMONY DEFAULT; Q-D3 OPEN. It narrows his "It changes anything else" for every edit made before a move. Why it
  holds for now: a move is a set-up action reached only from the menu (the strip's drag reorder is assigned nowhere
  at the pin, LayerStrip.h:100); the other way re-numbers 12 command classes, three of which hold whole decks,
  inside the lane's most intricate stage (base K1).
- S1m (only if he answers Q-D3 "keep the history"): `virtual void Command::layerMoved(int from, int to)` called on
  every step in the history, each command re-numbering its rows and the rows of a deck it holds by value;
  `moveLayerEndingHistory` calls it instead of clearing. Its own small plan first; its cases are not registered
  here.
- REJECTED: keep the step and refuse or spend its Undo. A refused step blocks every Cmd+Z behind it for good (the
  rule is unconditional, so it never clears); a spent one shifts the rows under older steps. REJECTED (ST-6): a
  menu item that says why it is empty -- a text that announces an event (AM-27).

Tests: TL-UC22; TL-UC16's third seed; lint TL-L2 (5.1). Mutant MU-40. Live: TR19 (INFO).

### DA-8 UNITS (his answer to 4)

ADDS to AM-17, AM-18 and bar B3.
BORIS. "For setting the clip beat marks, the clips are set with beats not bars. x2 or /2 are beat changes. bars
will confuse this. A bar is for beats and is used in places where longer durations make sense and beats are used
where exaggerations makes sense. We should be very clear where we are using beats and bars, but they are
essentially the same thing like feet and inches".
- The clip's length row is captioned exactly "Beats"; the tempo row exactly "BPM". "/2" and "x2" sit on the Beats
  row and change the beats (AM-17; base Q7 closed by "all defaults").
- Tooltips (standing text): Beats box "Length of the loop in beats"; BPM box "The clip's own tempo, in beats per
  minute"; "/2" "Half as many beats: the clip plays twice as fast"; "x2" "Twice as many beats: the clip plays half
  as fast".
- No caption, tooltip or painted string of the BPM row, the Beats row or the two bars holds the word "bar" in any
  form (B3). The beat lines are beat lines, all of one weight; nothing marks a bar on a clip, and a thinned line
  carries no label (DA-2).
- F8 stands (the two boxes are two views of one number). Harmony's reading of his answer: he picked no letter, and
  nothing he wrote asks for the other way.
- The per-clip Snap setting and the Quantize menu name bars today; they launch a clip, they do not set it; outside
  this lane.
NOTE FOR THE BARS LANE (bf7) -- not built here:
1. A clip is never set in bars. No bars-lane widget on a clip's rows may take or show bars.
2. Every number box, menu or readout that shows a musical length carries its unit in its caption or beside the
   number ("beats" or "bars"). A caption that names no unit ("Loop length", "Content length" -- ruling-bf7 AM6,
   already withdrawn by H-1) does not meet his rule.
3. Beats and bars are one measure at two scales (1 bar = 4 beats at the pin, FeatureSnapshot.h:45-46): a place that
   shows bars may say so once; it must not show a bare number that could be either.
4. The bars lane lists every place that shows a musical length with its unit, as a table, before it builds.

### DA-9 THE STRIP'S SPEED FADER IN BEATS MODE

NEW (ST-5). ADDS to AM-17 and to 5.5.
BORIS. "either BPM synced throughout the whole thing, or just playing with a speed control"; "Their speeds are just
controlled differently." Reading: in beats mode the speed control does not apply. He did not say what the strip's S
fader does then: Q-D6.
- In BPM Sync the strip's S fader is DISABLED and keeps showing the clip's stored speed; in Timeline it works as
  today. The state follows the layer's playing clip from the strip's timer (`syncFromModel`) and is set only when
  it changes (Pitfalls 41, 57, 59).
- In BPM Sync the forward button writes `reverse = false` and `playing = true` and leaves `speed` alone (today it
  doubles a speed nothing plays, which would surface when the clip is switched back). Back, pause and play are
  unchanged in both modes.
- No text. A disabled control is a state (AM-27).
Tests: TL-U31 (5.1). Bar B12 (5.5). Check 34.

### DA-10 LABELS AND IDS (no behaviour)

- AM-3: F1 ("every fire plays") is Boris's now -- base Q1's default, kept by "all defaults" (which default that is:
  Harmony's reading, boris-feedback-backlog.md:300). The narrow-rule fallback is dead. FM-2 is still measured and
  reported; nothing depends on it. T6's cells that read "Harmony default; Q1 open" read "Boris: all defaults (Q1)".
- AM-9: no longer ASSUMED (base Q8's default kept).
- Base Q1-Q10: all closed -- Q2 "2 b"; Q3 and Q10 by his own rules; Q9 "9 b"; Q4 he ruled the unit and F8 stands as
  Harmony's reading; Q1, Q5, Q6, Q7, Q8 "all defaults". "12 b" belongs to the notices lane and is OPEN there;
  nothing here depends on it.
- This delta's questions are Q-D1 .. Q-D6 (section 7). The delta plan's "D1-D4" named both facts and questions and
  is not used anywhere.
- Section 9 of the base: K2 (no hold) and K5 (rate, not position) are retired; K1 grows (DA-5, DA-6).
- Untouched by this delta: AM-1, AM-2, AM-4, AM-5, AM-8, AM-14 .. AM-16, AM-19 .. AM-21, AM-23 .. AM-30;
  H-1 .. H-5.

WHICH OF ruling-transport's PARTS CHANGE (the map)

| base | what happens to it | by |
|---|---|---|
| AM-3 | relabelled (his default); fallback dead | DA-10 |
| AM-6 | outcomes 1 and 3 amended | DA-5 |
| AM-7 | two bullets replaced; the refused Redo stands | DA-6 |
| AM-9 | stands; no longer ASSUMED; re-inserts at recorded indices | DA-6, DA-10 |
| AM-10 | REPLACED whole | DA-5, DA-7 |
| AM-11 | its fourth bullet replaced; the Clip tab's direct write struck | DA-3, DA-4 |
| AM-12 | "ONLY if Boris answers Q2" struck; the Intent also feeds the lock | DA-3, DA-1 |
| AM-13 | stands; added to | DA-4 |
| AM-17 | stands; added to | DA-8, DA-9 |
| AM-18 | its second bullet (the beat lines) replaced; the rest stands | DA-2 |
| AM-22 | "F9 stands", the FM-3 consequence and the FM-4 bullet replaced; the store stands | DA-1 |
| section 4 | replaced by section 4 here | section 4 |
| section 5 | pins and rows as section 5 here; every row not named there stands | section 5 |
| sections 6, 7 | replaced by sections 6 and 7 here | sections 6, 7 |
| section 10 | table rows replaced (DA-5); a stage row for S4c: plan parts none; with DA-1; 5.1 S4c; facts RD-11 .. RD-13, RD-16 .. RD-22 | DA-5, section 4 |

## 4 FINAL STAGES (the whole list, in order; replaces ruling-transport section 4's stage list)

ONE lane (base F17), started after the sync-dial lane (bf2) has merged (H-3 as adopted: S1 does not start early).
One builder context per stage, in this order. Every stage: its cases are written first and shown RED by id on the
stage's base (a case that never failed is struck and reported), then GREEN at its head.

- S0 RE-BASE NOTES (no code). The base's list (ruling-transport section 4, S0, whole), plus for this delta:
  - where the frame's snapshot is read and what the release is given (RD-13); whether the sync-dial lane leads the
    phase the release uses;
  - whether a typed, a tapped and a Link tempo report LOCKED and keep `beatPhase` moving; what `trackerState` and
    `beatPhase` read in test mode after `set_bpm` with no injected beats (DA-1's two pre-specified conditions: S0
    says which applies, if any);
  - the direction expression of both players and the sequence's leg (RD-11);
  - every push site of ToggleLayerFlagCmd and MoveLayerCmd on the merged main, every producer of
    `affectsLayerOrder`, and whether `onLayerDragReorder` is still unassigned;
  - every path that puts a clip into a cell without an Undo step (DA-6);
  - the Command classes that hold a row number or rows by value, re-counted (RD-10);
  - whether `/api/bpm` reports `beatPhase` (FM-4: if it does not, S4c adds the field -- a field a gate row reads,
    AM-18's test); whether a REST route can run a menu command (TR19);
  - the width in pixels of the strip's transport rect and of the Clip tab's bar at both window sizes, and for each
    the number of beats from which the lines thin (DA-2);
  - Boris's answers to Q-D1 .. Q-D6, as far as he has given them.
  Harmony constraint: freeze the RED arm -- a copy of the pre-lane main app bundle; FM-2 on it before S2 (base).
  Proves: the table.
- S1 UNDO-LIVE + THE STRIP RULE. The base's S1 (plan T5 with AM-6 .. AM-9) with DA-5, DA-6, DA-7, DA-10; AM-10 is
  gone. Files added to the base's list: MainComponent.cpp (the three flag handlers, the two move handlers, the
  layer-order plumbing), core/DeckCommands.h (the two classes deleted; `restoredForUndo`; `moveLayerEndingHistory`;
  addressing and capture in AddLayerCmd, RemoveLayerCmd, InsertDeckCmd), core/EffectCommands.h (the live clip with
  no media), core/Command.h, core/UndoManager.*, core/CompositeCommand.h (`affectsLayerOrder` leaves),
  ui/LayerStrip.cpp (the X tooltip), the tests DA-5 names. TL-UC16, TL-UC14 and TL-UC17 are written FIRST.
  Proves: the base's 21 S1 cases (TL-UC16 under its new name) + TL-UC17 .. TL-UC22 (27), the stamp and clip-BPM
  clauses left to S2 and S4a as the base rules; lint TL-L2 as widened; the undo and show binaries whole; G-U5.
- S2 RESTART. As the base (AM-1 .. AM-5; AM-3 in its F1 form, now his: DA-10). Then Harmony measures FM-1.
- S2b (only if FM-1 is over its bar). As the base (AM-21; TL-U6b; the two probes whole; FM-1 again).
- S3 DRAG. AM-11 (with DA-3's bullet), AM-13, DA-4. Files as the base. Proves: TL-U10, TL-U13, TL-U14, TL-U15,
  TL-U15b; lint TL-L3 as widened; lint TL-L1 in its final form.
- S3h HOLD (built; after S3). AM-12, DA-3. Files: render/ClipTransportSync.h, MainComponent.cpp, ui/LayerStrip.*,
  ui/ClipInspector.*, tests. Proves: TL-U11, TL-U11b, TL-U11c, TL-U12, TL-U12b, TL-U12c; MU-H1 .. MU-H3; G-U4 at 7.
- S4a BPM ENGINE, additive. As the base (AM-14, AM-15, AM-16, AM-18's REST, AM-22's store); the beats math and
  `beatLinesOf` live in the new src/render/ClipBeatGrid.h (DA-2). Proves: the base's 7 cases (TL-U17 under its
  shortened name) + TL-U21 (8); TL-UC3b gains its clip-BPM clause; G-U4 at 8. S4a's report states with file:line
  that a sequence's BPM Sync rate is pushed through `setSpeed` and its fps is `sequenceFps` (RD-15).
- S4c BEAT LOCK (new; after S3h and S4a, before S4b). DA-1. Files: render/ClipBeatGrid.h,
  render/ClipTransportSync.h (`bpmSyncSpeed`), render/Renderer.cpp (`syncMedia`'s two BPM Sync branches; the two
  `featureBus_.read()` go), media/VideoPlayer.h/.cpp and media/ImageSequence.h/.cpp (`movingBackward`), tests.
  Proves: TL-U22, TL-U23, TL-U24, TL-U24b, TL-U24c, TL-U24d, TL-U25, TL-U25b, TL-U25c, TL-U26, TL-U27, TL-U28 (12);
  TL-U19 gains its clause (the base's clause rule: the same TEST_CASE, shown RED by id in S4c's log); lint TL-L5;
  `.harmony/probe-video.sh` whole (`PROBE-VIDEO GREEN`: a speed that changes every frame under the reverse GOP
  cache, Pitfall 62). Then Harmony measures FM-5 -- before S4b starts.
- S4b BPM WIDGETS + BEAT LINES + THE STRIP. The base's S4b (AM-15's test rows, AM-17, AM-18, AM-26, AM-27, AM-28)
  with DA-2, DA-8, DA-9. Files added: ui/LayerStrip.*, ui/DeckView.* (the forward), MainComponent.cpp (the length
  provider's sequence branch), the strip dump route. Proves: TL-U18d, TL-U20, TL-U30, TL-U31; lint TL-L5's last
  clause. The stage ends at the VISUAL WORK GATE (5.5), not at a commit.
- S5 PROBE + DOCS. `.harmony/probe-transport.sh/.py/.json` (23 rows, 6 measurements); the docs of the base plus:
  the strip rule, layer addressing and the history's end at a layer move (docs/claude/performance-controls.md); the
  beat lock, ClipBeatGrid.h and the trimmed loop's lost frames (docs/claude/rendering.md); two new Pitfalls -- "a
  BPM-synced clip is trimmed, never sought; lines and lock come from ClipBeatGrid.h; the lock takes its Intent,
  its snapshot and its direction from the sync it runs in" and "no Undo command writes a strip field; a layer is
  erased by id and re-inserted at its recorded index; a layer move ends the history" (Harmony assigns the
  numbers); Boris's page (sections 6 and 7 here, with the base's notes N1 and N2); the lane report (G-N1's ten
  numbers; the default of Q-D1 signed, H-D1).
  Proves: a full local run on the lane build and the RED table.
- S1m (only if Boris answers Q-D3 "keep the history"): DA-7. Its own small plan first; its cases are not
  registered here.

What the other lanes get, added to the base's list: bars lane -- DA-8's note; Timeline lane -- ClipBeatGrid.h is the
only source of a clip's beat positions, and the hold is AM-12's record (built); Edit menu -- the Undo and Redo items
are disabled after a layer move.
Section 10's stage table: the S3h row loses "only if Q2 is answered with the hold"; S1's "with" reads "AM-6 ..
AM-9, AM-27, DA-5, DA-6, DA-7, DA-10"; S3's "AM-11, AM-13, DA-3, DA-4"; S4a's gains "DA-2 (the grid)"; S4b's gains
"DA-2, DA-8, DA-9"; a new row: | S4c | none | DA-1; 5.1 S4c; 5.3 TR15 .. TR18 | RD-11 .. RD-13, RD-16 .. RD-22 |.

## 5 GATE ROWS changed or added (and the rows of ruling-transport section 5 they replace)

Harmony constraint (base, unchanged): RED arm = the frozen pre-lane main app; a flake verdict needs >= 5 runs per
arm; no on-screen text announces an event or a failure; a pre-registered bar is never loosened: it is met, or
reported; a row whose bar sits inside 4 x the measured noise is BLOCKED until Harmony waives it in writing.
Every bar below is registered HERE, before any stage is built and before any run was judged against the delta
plan's text. Where a number differs from the delta plan's, the law changed (the reach is 6 %, the default for a
loop that is not whole) or the delta plan's number was wrong (RD-17, RD-20). Every row of section 5 that is not
named here stands as ruling-transport wrote it.

### 5.1 Unit cases -- REPLACES the heading of ruling-transport 5.1 and ADDS the cases below

The heading reads: "Unit cases (each a TEST_CASE whose name begins with the id; 77 cases + 6 lint cases;
conditional stage S2b: + 1)". 49 of the base + 6 of S3h (now unconditional) + 22 new. By stage: S1 27, S2 15,
S3 5, S3h 6, S4a 8, S4c 12, S4b 4.

S1. REPLACES TL-UC16's name: TL-UC16 "200 random Undo and Redo steps with fires between them, three seeds: unique
clip ids across live and retired decks, rows equal layers, every live ref resolves to a clip in a shown column, no
step changed a tuple, no step changed a strip field of a layer that exists before and after it" (S2 adds: every
fire's stamp is newer than every stamp before it). Its random actions gain the edits that push no step (bypass,
solo, fold, an opacity and a blend write, the transport buttons) between the Undo and Redo steps; its third seed
moves a layer twice through `moveLayerEndingHistory` and asserts after each move that the history is empty. Strip
fields are compared through a TEST-side helper `StripState stripStateOf(const Composition&, uint32_t layerId)`
(tests only; no src type): the fields of DA-5, the position as the order of the layer ids that stay.
New in S1:
- TL-UC17 "no command's Undo or Redo changes a strip field of a layer that exists before and after the step: name,
  bypass, solo, fold, opacity and its shown value, key, keying, blend, fade time, fade mode, order among the layers
  that stay, tuple, the playing clip's media, name, FX look and transport (every Command class)" -- the table of
  TL-UC14, with the strip fields.
- TL-UC18 "Undo of Remove Layer brings the layer back with the settings it had when it was removed, an empty tuple
  and solo off; settings changed after an Add Layer survive its Undo and Redo; settings changed between two
  removals survive the second Undo; Redo of Load Deck re-adds its layers the same way; no other layer's strip field
  changes"
- TL-UC19 "a same-clip landing on a live cell keeps the live clip's name"
- TL-UC20 "after an Undo that kept a live layer, every older Undo and every Redo lands on the layer it was recorded
  on: Remove then Add; Load Deck then Add; Add then Add -- rows equal layers, every clip id in one cell, the order
  of the layers that stay never changes"
- TL-UC21 "a playing clip with no media: Undo and Redo of removing its last effect, and of adding its first, are
  left; the same steps on an idle clip land; an effect edit that keeps it an FX clip lands"
- TL-UC22 "a layer move reorders the stack and empties the history: no Undo, no Redo; a refused move leaves the
  history"

S3. New: TL-U15b "a point dragged past the playhead carries it: the in point lands the playhead on it; the out
point puts it at the end of the range, and two frames later a Loop is at its start and a OneShot has ended; after
every marker drag event the playhead is inside in..out".

S3h. The six cases of the base's "Conditional stages", unconditional, names unchanged.

S4a. REPLACES TL-U17's name: it ends "... a trimmed clip uses its in..out length" (its beat-line clause moves to
TL-U21). New: TL-U21 "beat grid: line b sits where the clip beat is b; on a wide bar 19 lines for 20 beats, 14 for
14.6, 1 for 2, none below 2; lines closer than 3 px thin to every 2nd, 4th, 8th beat (64 beats on a 150 px bar:
every 2nd beat, 31 lines); a trimmed clip's lines lie inside in..out".

S4c. TL-U19's name gains: "...; in BPM Sync, video and sequence, the speed pushed is the rate times one plus the
trim; with the lock off it is exactly the rate". New:
- TL-U22 "beat lock error: clip beat 3.25 against show phase 0 is +0.25; 3.75 is -0.25; running backward mirrors
  it; the error is never outside [-0.5, 0.5)"
- TL-U23 "beat lock trim: zero inside 0.03 beat; minus half the error outside it; never beyond 0.06 either way"
- TL-U24 "a clip started half a beat off is within 0.03 beat (+ 1e-6) from 12 beats on; its position never steps
  back; the speed pushed stays within 6 % of the rate"
- TL-U24b "a tempo change from 120 to 90 on a locked clip: the error stays inside 0.05 beat"
- TL-U24c "a realign of half a beat (Resync, Tap): no position step; within 0.03 beat (+ 1e-6) from 12 beats on"
- TL-U24d "a show phase published every 512 samples at 48 kHz and read at 60 frames a second, at 90, 120, 168 and
  200 BPM, on a locked 16-beat clip over 96 beats: in the last 64 the speed pushed is exactly the rate in at least
  95 % of the frames"
- TL-U25 "the lock is off and the speed pushed is exactly the rate: a paused clip, a held clip, a hold cleared
  between pushIntent and the speed push, a tempo of 0, a tracker that is not locked, a length that is not known,
  beats 14.6, 6.5 and 2.5; a tracker state that flips every frame keeps the speed within 6 % of the rate and never
  steps the position"
- TL-U25b "beats 16.016 in a Loop started on the beat, over 8 wraps: the lock applies, the trim never exceeds
  2.5 %, and before every wrap the error is within 0.03 beat (+ 1e-6); beats 16.04 locks; beats 16.06 does not"
- TL-U25c "a trimmed loop started on the beat, in 0.2, out 0.7, 8 beats, at 200 BPM and 60 frames a second,
  through the real write-back and a player that seeks as the video does: from the second wrap on the error is
  within 0.03 beat (+ 1e-6) in the last beat before every wrap and never beyond 0.16, and the trim is never below
  0 nor above 0.06 (+ 1e-6)"
- TL-U26 "a reversed 16-beat clip crosses its lines on the beat; a ping-pong keeps the lock through both turns with
  no position step; on the frame a clip is reversed the error is taken with the new direction"
- TL-U27 "x2 and /2 on a locked clip: no position step; within 0.03 beat (+ 1e-6) from 12 beats on"
- TL-U28 "after a drop the clip plays on from the drop point: no seek is requested by the lock and a quarter of a
  beat later the position is the drop point plus a quarter of a beat, within 6 %"
The three beats numbers in TL-U25 are HARMONY'S DEFAULT (Q-D1 open), not Boris's word, and the lane report says
so. If he answers Q-D1 with B: TL-U25's clause reads "beats 6.5 and 2.5" and TL-U25b gains "beats 14.6 in a Loop:
from the second wrap on the error is within 0.03 beat (+ 1e-6) in the last beat before every wrap". With C: no
change here.
THE FAKES (binding for every S4c case). The fake player keeps a clock as the real video does: `seekTo` is a
request; the advance that consumes it sets the clock and does NOT advance in that call; a whole-range Loop wrap
keeps the overshoot; `movingBackward()` is the direction the next advance takes (RD-11). Every case calls the real
`pushIntent`, `bpmSyncSpeed` and `writeBack`, in the renderer's order. The fake beat clock is exact unless the case
says otherwise (TL-U24d). TL-U25's sub-cases start the clip a quarter of a beat off, so a lock that wrongly runs
moves the speed. Where the bars come from (INFERRED, RD-16, RD-17, RD-21): TL-U24 9.1 beats against 12;
TL-U25b 2.3 % against 2.5 %; TL-U25c 0.03 + 2 frames = 0.141 beat against 0.16 and 3.1 beats of pull-in inside an
8-beat loop; TL-U24d no trim at all once settled. A case that cannot meet its bar: STOP and report.

S4b. New:
- TL-U30 "layer strip: in BPM Sync the transport view carries the grid's lines and the rect repaints only when a
  line or the playhead moves a pixel; in Timeline it carries none; lines closer than 3 px thin and never vanish;
  the length is asked in refresh, when the playing clip changes, and at most on every 15th tick while it is 0"
- TL-U31 "layer strip: in BPM Sync the S fader is disabled and a forward press leaves the clip's speed; in Timeline
  the fader is enabled and a forward press doubles it; the enabled state is set only when it changes"

Lints (6, tests/test_render_thread_lint.cpp). TL-L1, TL-L1b, TL-L4 as the base. REPLACED or new:
- TL-L2: the base's text, then: "; `ToggleLayerFlagCmd`, `MoveLayerCmd`, `affectsLayerOrder` and `moveLayerNoUndo`
  nowhere in src; in src/core/*Commands.h no `bypassed =`, no `folded =`, no `opacity =`, no `blendMode =`, no
  `keyingMode =`, no `keyThreshold =`, no `transitionSpeed =`, no `transitionMode =`, and `solo =` only as `solo =
  false` inside restoredForUndo; outside src/model/Composition.h `moveLayer(` occurs in src only inside
  moveLayerEndingHistory, which holds `history.clear()`".
- TL-L3: "`onScrub` is assigned in src/MainComponent.cpp and forwarded in src/ui/DeckView.cpp;
  src/ui/LayerStrip.cpp and src/ui/ClipInspector.cpp have no `playheadPosition =`".
- TL-L5 (new, S4c): "src/render/ClipBeatGrid.h includes no player header and holds no `seekTo`, `restart(`,
  `setPlaying` or `playheadPosition`; `beatLockTrim(` occurs in src only in ClipBeatGrid.h and ClipTransportSync.h;
  in src/render/Renderer.cpp `bpmSyncSpeed(` occurs exactly twice, each inside a `setSpeed(` argument; between
  `GLuint Renderer::syncMedia(` and the next function there is no `featureBus_.read()`, `setFps(` occurs once, as
  `setFps(clip->sequenceFps)`, and no BPM Sync branch holds `playing.load` or a call of the hold record's reader
  (S3h's function; S4c's log names it); in src/media `movingBackward` reads no `reverseNow_`". S4b adds the clause
  "; the length provider in src/MainComponent.cpp holds no `getImageSequence(`" (the base's clause rule: the same
  TEST_CASE, shown RED by id in S4b's log).

Mutants (G-U3; each applied alone to the lane head turns the named case RED):
MU-30 the trim's sign flipped -> TL-U23, TL-U24 (live: TR15, TR17, TR18). MU-31 no deadband -> TL-U23, TL-U24d.
MU-32 no limit -> TL-U23, TL-U24. MU-33 `backward` ignored -> TL-U22, TL-U26. MU-34 the lock runs for a held or
paused clip -> TL-U25. MU-35 the lines use the whole file, not in..out -> TL-U21 (visual gate: B10 in V8; live:
TR18). MU-36 a seek to the aligned position after a release -> TL-U28 (live: TR17). MU-37 `beatLockApplies` always
true -> TL-U25. MU-38 a restore keeps solo -> TL-UC18. MU-39 outcome 3 drops the name -> TL-UC19, TL-UC17. MU-40
`moveLayerEndingHistory` without the clear -> TL-UC22, TL-UC16. MU-41 a command that writes `bypassed` on Undo ->
TL-UC17, TL-L2. MU-42 no marker carry -> TL-U15b (live: TR7d). MU-43 the lock does its own `featureBus_.read()` ->
TL-L5. MU-44 `bpmSyncSpeed` reads `clip.playing` or the hold record itself -> TL-U25 (the cleared-hold clause),
TL-L5. MU-45 `movingBackward()` returns `reverseNow_` -> TL-L5. MU-46 the lines never thin -> TL-U21, TL-U30.
MU-47 the S fader stays enabled in BPM Sync -> TL-U31 (visual gate: B12). MU-48 InsertDeckCmd's Redo appends its
layers at the end -> TL-UC20. MU-49 EffectStackCmd lands on a live clip with no media -> TL-UC21, TL-UC17. MU-50
AddLayerCmd's Redo re-inserts its first capture -> TL-UC18.

### 5.2 Unit gates -- REPLACES the pins of G-U1, G-U1b, G-U2, G-U3, G-U4

- G-U1 `ctest --test-dir build -R '^TL-' --no-tests=error --output-on-failure` -> `100% tests passed, 0 tests failed
  out of N` with N >= 83 (84 if S2b is built), AND each of the 83 ids appears as Passed. RED arm: the per-stage RED
  log, by id (S2's log also lists TL-UC2b and TL-UC16; S4a's TL-UC3b; S4c's TL-U19; S4b's TL-L5).
- G-U1b adds any new binary (the beat grid's) to the list run whole by path. GUARD, no RED arm.
- G-U2 as the base; "cases deleted" also counts the ToggleLayerFlagCmd and MoveLayerCmd cases (DA-5).
- G-U3 adds MU-30 .. MU-50 and makes MU-H1 .. MU-H3 unconditional. Harmony re-runs six: always MU-4, MU-18, MU-21,
  MU-30, MU-48, and one of her choice.
- G-U4 `.harmony/probe-tsan-unit.sh` -> exit 0 with `EXPECTED_TSAN_CASES` raised from 5 to 8 (TL-U8, TL-U11c,
  TL-U19b). By stage: 6 at S2's head, 7 at S3h's, 8 at S4a's. RED: as the base, and the hold record declared as a
  plain `uint64_t` makes TL-U11c report a race. This delta adds no shared field and so no TSan case.
- G-U5, G-N1: as the base. (The X tooltip and DA-8's four tooltips are `setTooltip` texts; they are not among
  G-N1's five strings and they announce no event.)

### 5.3 Live rows -- `.harmony/probe-transport.sh`; REPLACES the pin: `EXPECTED_ROWS=23`

The base's 17 rows stand with the changes and additions below (17 + TR8 + TR7d + TR15 .. TR18). Launch rules,
fixtures, tolerance and the flake rule as the base.
THE BEAT DRIVER (TR15 .. TR18): the probe posts `/api/inject_features` at >= 50 Hz with `bpm`, `trackerState` 2 and
`beatPhase` + `totalBeatCount` taken from its own monotonic clock (the driver of probe-boxes k5 and of TR6). Show
tempo 60 unless a row says otherwise (one post is then at most 0.02 beat old). "At driver phase [a, b]" = the probe
sends the request while its own clock's phase is in that window. The probe computes e = wrap(frac(x) - driver
phase at the reply), x from the reply's `playheadPosition`, `inPoint`, `outPoint`, `clipBpm` and the fixture's
length (on the frozen arm, which reports none of the last three: the fixture's own numbers). The lane report states
the noise of e on a locked clip over 5 runs (M5).
Every row starts its pull-in a QUARTER of a beat off, not half: half a beat is the law's unstable point (RD-19),
where the direction is not fixed; the half-beat case is the unit cases' (TL-U24, TL-U24c).
- TR7c gains: "strip down 0.05 (before the in point 0.2), up: no read is below 0.2". GUARD; teeth MU-10.
- TR7d marker_carries_the_playhead. In 0 / out 1, a Loop ramp. Wait for a playhead read in [0.3, 0.4]. Clip tab:
  press the in marker's tab, drag to the x of 0.6, up. Bar: `inPoint` in [0.59, 0.61] and a playhead read within
  0.2 s of the up is >= 0.59. GUARD: no RED arm (the route is new). Teeth: MU-42 -- the read is below 0.45.
- TR8 scrub_hold. As ruling-transport 5.3 wrote it; no longer conditional.
- TR15 beat_lock_pulls_in. BPM-synced ramp, in 0 / out 1, `clipBpm` 60 (10 beats), Quantize off; fire at driver
  phase [0.20, 0.30]. Bars: (i) a read within 0.5 s of the fire has |e| >= 0.12 (the fire did not move the clip
  onto the beat); (ii) no playhead read is below the one before it except across the loop's wrap, and the content
  rate over every 2 s window (from the playhead reads) is in [0.92, 1.08]; (iii) from 12 beats after the fire,
  five reads 0.5 s apart each have |e| <= 0.08. RED arm (frozen app, TR9's old-key composition, rate 1.0): (iii) fails with |e| about 0.25
  (INFERRED; if the RED arm passes, the row is reported as a guard and MU-30 is its teeth).
- TR16 beat_lock_tempo_and_realign. From TR15's locked state. (a) the driver steps to 90 with its phase continuous
  and posts at >= 75 Hz from then on: every read of the next 8 beats has |e| <= 0.12, after them <= 0.08, and the
  rate over 2 s is in [1.44, 1.56]. (b) the driver adds 0.3 beat to its phase in one step: no read is below the
  one before it except across the wrap; the rate over every 2 s window stays in [1.38, 1.62]; from 12 beats later
  |e| <= 0.08. RED arm: (a)'s rate stays about 1.0.
- TR17 beat_lock_after_a_drop. TR15's fixture, fired again and left 12 beats at 60. `/api/debug/strip_scrub` down
  0.35; four reads over 1 s within one frame of 0.35; up at driver phase [0.20, 0.30] (the clip is then a quarter of
  a beat ahead). Bars: (i) the read 0.5 s after the up is in [0.39, 0.41]; (ii) a read within 0.3 s of the up has
  |e| >= 0.12; (iii) from 12 beats after the up, five reads 0.5 s apart each have |e| <= 0.08. GUARD: no RED arm
  (the route is new). Teeth: MU-36 -- (i) and (ii) fail (the read is near 0.375); MU-30 -- (iii) fails.
- TR18 lines_on_the_beat. Ramp with in 0.2, out 0.7, `clipBpm` 96 (8 beats, 7 lines), inspected in the Clip tab;
  fired at driver phase [0.20, 0.30]; the probe then waits 12 beats. Bars: (i) on both dumps
  (`/api/debug/clip_transport_ui`, `/api/debug/strip_transport_ui?layer=0`) the line xs equal `beatLinesOf` of the
  model at that surface's bar width, +/- 1 px; (ii) at 8 instants where the driver's phase is in [0, 0.02] the
  playhead x on each surface is within max(3 px, 0.15 of one beat's width) of the x of a whole clip beat, computed
  from that dump's in and out x (the in marker counts). GUARD: no RED arm (the routes are new). Teeth: MU-35 turns
  (i) RED; MU-30 turns (ii) RED (the playhead sits half a beat off).
- TR9 runs with no beat driver. What the lock does then is S0's finding (DA-1); the row's bars do not change.
- (INFO, outside EXPECTED_ROWS, only if S0 finds a REST route that runs a menu command) TR19
  move_layer_ends_history: drop a file on an idle cell; Move Layer Up; `POST /api/debug/undo`: the cell still holds
  the file and the layer order is the moved one.
- Paths with no live row, and what proves them, added to the base's list: bypass, solo and fold push no step -- the
  class is deleted, lint TL-L2; a restored layer's state -- TL-UC18; kept layers -- TL-UC20; the effects-only clip
  -- TL-UC21; a layer move -- TL-UC22; a loop that is not a whole number of beats -- TL-U25; a trimmed loop at a
  high tempo -- TL-U25c; the hop staircase -- TL-U24d; the lock on a sequence, a reversed and a ping-pong clip --
  TL-U26; the S fader -- TL-U31 and B12.
- Measurements, printed as `MEASURE <name> <value>`, never part of the verdict line: M1 (FM-1), M2 (FM-3), M3
  (FM-4), M4 (FM-5), M4b (FM-5's share of windows within 0.5 %), M5 (the noise of e).

### 5.4 Re-runs and idle paint -- ADDS one line

- `.harmony/probe-idle-paint.sh` with a BPM-synced clip playing in a strip -> its existing bars (INFO unless it
  regresses; no RED arm); TL-U30 is the gate for "the lines add no repaint".

### 5.5 VISUAL WORK GATE -- ADDS states V12-V14 and bars B10-B12; REPLACES "Pass = B1-B9"

- States added: V12 the layer strip, BPM Sync, 8 beats (7 lines), the S fader disabled. V13 the same clip in
  Timeline mode, strip and Clip tab (no line; the S fader enabled). V14 a 64-beat clip in BPM Sync, strip and Clip
  tab (thinned wherever one line per beat would be under 3 px). Every dump of a BPM Sync state carries the line xs
  and the stride of both surfaces.
- B3 gains: no caption, tooltip or painted string of the BPM row, the Beats row or either bar holds "bar" in any
  letter case; the four tooltips are DA-8's strings exactly.
- B10 in every BPM Sync state whose length is known the line xs of each surface equal `beatLinesOf` of the model at
  that surface's bar width, +/- 1 px; neighbouring lines are at least 3 px apart; all lines of a surface have one
  colour and one width; in V14 each surface draws at least one line.
- B11 no line on either surface in V1, V6, V10, V13.
- B12 the strip's S fader is `enabled == false` in V12 and V14 and `true` in V13.
- Pass = B1-B12 and no seat raising a MUST. B10-B12 are GUARDS (new routes); teeth on the lane build, shown once in
  S4b's log: MU-35 turns B10 RED in V8; MU-47 turns B12 RED in V12.
- The critics' packet (AM-28) gains: the strip's controls that exist before the lane and are outside it (RD-24),
  and Boris's sentences of DA-1, DA-8 and DA-9, verbatim.
- Seats, added questions. visual-design: on both surfaces, at both window sizes, are the beat lines told apart from
  the in and out markers and from the playhead without colour alone? Does V14's thinned comb read as lines or as
  noise? Is the disabled S fader told apart from an enabled one without colour alone? UX: can beats mode be told
  from speed mode from the bar alone, also on a long clip? Does anything in the two rows or the two bars suggest
  bars? Is there a control in the strip that looks usable and does nothing in beats mode? interaction-logic (given
  TR18's numbers): does the playhead meet a line when the beat falls; after x2 are there twice the lines, or the
  same lines thinned?

### 5.6 Facts Harmony must measure -- REPLACES FM-3's consequence and FM-4; ADDS FM-5

- FM-3 (the tempo's wobble): the measurement and its bar as the base. Met: the raw read stays. Not met: STOP and
  report -- a slew of the rate would defeat the beat lock (RD-22); no change is pre-decided.
- FM-4 is a bar now (lane build). A music file; a BPM-synced ramp with a whole number of beats (in 0 / out 1,
  `clipBpm` 96: 16 beats) fired on a downbeat with Quantize on; in the fifth minute 20 reads 3 s apart of the clip's e against the
  beat phase `/api/bpm` reports: |e| <= 0.1 beat in at least 18. Not met: STOP and report -- no change is
  pre-decided (the trimmed loop's carry and the three constants are the named candidates).
- FM-5 the trim on real music (lane build; after S4c, before S4b starts). The same clip for 60 s while the tracker
  reports locked; `playheadPosition` read at 20 Hz (never pixels: one frame of a 30 fps ramp is 1.7 % of a 2 s
  window); the content rate over each 2 s window against (the mean show BPM of that window) / clip BPM. Bar: at
  least 90 % of the windows within 2 %. Not met: STOP and report -- the three constants and DA-1's slew are the
  named candidates; which moves is decided on the numbers. Reported with it, no bar: M4b, the share of windows
  within 0.5 %.
- FM-1, FM-2: as the base (FM-2 is reported; nothing depends on it any more, DA-10).

## 6 WHAT ONLY BORIS CAN CHECK (the whole list; do -> expect -> what wrong looks like)

REPLACES ruling-transport section 6. Checks 1-6, 8, 9, 11-19, 21-23 are the base's (their "(question n)" notes are
dropped: those questions are closed); 7, 10 and 20 are replaced; 24-39 are new.
1. Fire a video, let it run, fire another clip on the same layer, fire the first again. -> It starts from its
   beginning. Wrong: it carries on from where it was.
2. Fire a column, wait, fire the same column again. -> Every video in it starts over together. Wrong: some keep
   running.
3. Play music until the beat counter at the top runs steadily. Set Quantize to "Next Downbeat". Pick a column where
   every layer has a clip. Fire it, then fire it again in the middle of a bar. -> Nothing moves until the next "1",
   then they all start over together. With "Next Beat" they start over on the next beat. Wrong: they jump at once,
   or never. (With no steady beat the app cannot wait for a bar, so the videos start over at once. That is expected.)
4. Pause a clip, then fire it. -> It plays from its beginning. Wrong: it sits on its first frame.
5. Clear a layer with its X, then fire the same clip again. -> It plays from its beginning. Wrong: it sits frozen.
6. Let a One Shot clip run to its end, then fire it again. -> It plays again. Wrong: nothing moves.
7. Grab the playhead in the layer strip, and then in the Clip tab; drag; let go. -> The picture follows your hand
   and the clip plays on from where you let go. If you keep the mouse down and hold still, the picture waits on
   that frame until you let go. Wrong: it springs back, or it slides away under a resting hand.
8. Do the same on a reversed clip and a ping-pong clip. -> It carries on in the direction it had. Wrong: it turns
   round.
9. In the Clip tab, press at the very left or right end of the bar, below the small marker tab. -> The playhead
   goes there; the marker does not move. Drag the marker tab itself. -> The marker moves.
10. On a clip with a trimmed start and end, drag the playhead past either marker, in the strip and in the Clip tab.
    -> It stops at the marker and goes no further. Let go at the end marker: the loop starts over. Wrong: the
    playhead goes into the dark part.
11. Set a clip to BPM Sync. -> BPM shows 120 and Beats shows the clip's length in beats at 120. Press x2: both
    numbers double and the clip takes twice as long (it looks slower); /2 brings it back. The Reverse button is
    still there. Tap a faster tempo: the clip speeds up with it. Wrong: the speed does not follow your taps, or the
    numbers disagree with what you see.
12. Type a number into Beats (for example 16). -> BPM changes to match and the clip loops in exactly 16 beats.
13. Play a BPM-synced clip and a speed clip side by side and change the tempo. -> Only the BPM-synced one changes.
14. Open one of your saved shows that has BPM-synced clips. -> At a tempo of 120 they move as they did before. At
    another tempo a BPM-synced VIDEO now follows the tempo (before, it ignored it). Image sequences behave as before.
15. Fire a clip, then press Cmd+Z. -> The clip keeps playing; Cmd+Z undid your last edit instead (or nothing).
    Wrong: the clip stops or the old one comes back.
16. Drag an effect onto the cell that is playing, switch that clip's Reverse on, press Cmd+Z. -> The effect goes;
    the clip keeps running backwards. Wrong: the direction flips back, or the clip jumps.
17. Drop a new file onto the cell that is playing, press Cmd+Z. -> The picture does not change. Do the same on a
    cell that is not playing. -> The old file comes back.
18. Load a deck that adds layers, fire one of its clips, press Cmd+Z. -> The deck leaves the tab row, the clip keeps
    playing, the added layers stay. Wrong: the picture goes away.
19. Remove a layer by mistake, press Cmd+Z. -> The layer and its clips are back, not playing until you fire one.
20. Bypass a layer, solo another, move its V fader, then press Cmd+Z a few times. -> None of the three changes
    back; Cmd+Z undoes your last edit outside the layer strip instead. Wrong: the bypass or the solo flips back.
21. Fire a clip on a bypassed layer, wait, switch the bypass off. -> The clip starts from its beginning when it
    appears.
22. Hold down a key that fires a clip or a column. -> It fires once; the video plays on while you hold.
    Wrong: it keeps jumping back to its start.
23. Remove a layer, press Cmd+Z (it is back, playing nothing), fire a clip on it, press Cmd+Shift+Z. -> Nothing
    happens: the layer stays, the clip keeps playing, and nothing is written on screen. Once that layer plays
    nothing, Cmd+Shift+Z removes it. Wrong: the layer goes away while it plays, or a message appears.
24. Drag the in marker to the right, past the playhead. -> The playhead rides along on the marker.
25. Set a video to BPM Sync. -> Lines appear in its playhead bar, in the Clip tab and in the layer strip, one per
    beat. Set it back to speed. -> The lines are gone.
26. Give a BPM-synced clip a whole number of beats (type 16 into Beats). With music playing and the beat steady,
    watch it. -> The playhead crosses a line on every beat, for as long as you leave it. Wrong: it crosses between
    the beats and stays that way, or you can see the picture speed up and slow down.
27. Drag that clip's playhead somewhere and let go between two beats. -> It plays on from where you let go, with no
    jump, and within a few seconds it is crossing the lines on the beat again (question Q-D5).
28. With Quantize off, fire that clip between two beats. -> It starts at once from its beginning and eases onto the
    beat within a few seconds. Wrong: it starts part-way in, or it never lines up.
29. Press Resync, or tap a new tempo, while it plays. -> The clip does not jump; it eases onto the new beat.
30. Type 14.6 into Beats. -> The clip plays at a steady speed and its lines pass off the beat (question Q-D1). Type
    15: within a few seconds the lines are on the beat again.
    (If he answered Q-D1 with B, the check reads: "-> Each time the loop comes round the clip runs a little slow
    for a few beats, then sits on the beat. Type 15 and it runs evenly.")
31. Press /2 on a clip of 15 beats (it shows 7.5). -> As in 30: steady, off the beat. Press x2: back on the beat.
32. Trim the end of a BPM-synced clip, give it a whole number of beats, watch the loop come round. -> It stays on
    the beat; right after each loop point the playhead may be a hair late for a beat or two. Wrong: it drifts
    further off with every loop.
33. Put a long clip (a minute or more) in BPM Sync. -> The lines are still there, one for every 2nd, 4th or 8th
    beat, never packed tighter than you can see.
34. In the layer strip, look at the S fader of a layer whose clip is in BPM Sync. -> It is greyed; on a speed clip
    it works (question Q-D6).
35. Remove a layer that had bypass, solo and a lowered fader; press Cmd+Z. -> It is back in its old row with its
    clips, its bypass and its fader as they were, playing nothing, and its solo is off. A layer you added in
    between comes after it and keeps playing.
36. Move a layer up, then press Cmd+Z. -> Nothing happens; the Undo item in the menu is grey (question Q-D3).
37. Add an effect to a layer, press Cmd+Z. -> The effect goes (question Q-D4).
38. Play a clip that is only effects (no picture file). In the Clip tab remove its last effect, then press Cmd+Z.
    -> Nothing comes back while that layer plays it: the strip stays as it is. Wrong: the FX tile pops back.
39. Add a layer, pull its fader down, press Cmd+Z, then Cmd+Shift+Z. -> The layer goes and comes back with the
    fader where you left it (question Q-D2).

## 7 BORIS QUESTIONS that remain (each has a default; nothing waits)

REPLACES ruling-transport section 7's ten questions (all closed: DA-10). Notes N1 and N2 of the base stay on his
page. On his page each question carries his own sentence beside it (H-D3).
- Q-D1 A clip in beats mode whose Beats number is not whole (the app's own number usually is not: a 7.3-second
  clip shows 14.6).
  A (default) It plays at a steady speed; its lines pass off the beat until you give it a whole number of beats.
  B The app pulls it back onto the beat every time the loop comes round, so on every loop it runs a little slow or
  fast for a few beats.
  C The app rounds its own number to a whole beat when you switch a clip to beats -- the BPM box then shows a
  number near 120 instead of 120.
  Beside it: "default all bpms to 120"; "9 b".
- Q-D2 You add a layer and press Cmd+Z: the empty layer goes away again (default). Or should Cmd+Z never take a
  layer row away? Beside it: "cmd z Does not change anything in the layer strip which is by default live based".
- Q-D3 After you move a layer up or down, Cmd+Z can no longer step back to edits you made before the move
  (default). Or should those older steps stay reachable (more work; a later stage)? Beside it: "It changes anything
  else".
- Q-D4 An effect you put on a layer or a clip: Cmd+Z takes it off again, also while the layer plays (default). Or
  should Cmd+Z leave effects alone while the layer plays?
- Q-D5 You drop the playhead of a clip in beats mode between two beats: it plays on from there and eases onto the
  beat within a few seconds, running up to 6 % off its speed meanwhile (default). Or should it keep its exact speed
  and stay off the beat until you fire it again? Beside it: "I wanted to keep playing at the same speed, but play
  from wherever I drop the play head."; "9 b".
- Q-D6 In beats mode the S fader in the layer strip is greyed (default). Or should it do something there -- for
  example double and halve the beats?
The other ways, as far as they are pre-specified: Q-D1 B and C -- DA-1. Q-D2 "never" -- AddLayerCmd pushes no step
and InsertDeckCmd's Undo always keeps its layers (small, local). Q-D3 -- S1m (DA-7). Q-D4 "leave them" --
EffectStackCmd is left on a live layer or cell in every scope (the test of DA-5's exception, without its two
conditions). Q-D5 "keep the speed" -- the lock is off for a clip from a drop until its next fire: one runtime
stamp on the Clip (`Relaxed<uint32_t>`, the clip's restart stamp at the drop), a static_assert pin, one more TSan
case, one clause in TL-U28; its own small plan first. Q-D6 -- not pre-specified: it needs his words.

## 8 HARMONY'S DECISIONS

H-1 .. H-5 of the adoption stand as she took them (H-3: S1 does not start before bf2 has merged). New, each with a
default; the builder builds the default unless told otherwise:
- H-D1 before S4c: the default of Q-D1 -- a loop that is not a whole number of beats is not locked -- is Harmony's
  to sign in the lane report, as AM-3's default was; it is not Boris's word. The other default (B, the delta
  plan's) is one predicate away (DA-1). Default: A.
- H-D2 before S4b: thinned beat lines (DA-2). Default: thin by powers of two. The other way (the stage-operator
  seat's): draw none when one line per beat would be under 3 px -- then B10 loses its last clause, V14 asserts no
  line on a surface that is too narrow, TL-U21 and TL-U30 lose their thinning clauses, MU-46 goes, check 33 goes,
  and a long clip in beats mode looks like a speed clip in the strip.
- H-D3 Boris's page: Q-D1 .. Q-D6, each with his sentence beside it, and checks 24-39. Default: on his page at the
  next page build, and before S1's code is merged; S4c's builder reads his answers to Q-D1 and Q-D5 at stage start.
  Nothing waits on them.
- H-D4 FM-5 is run right after S4c and read before S4b starts. Default: yes. Over its bar: STOP, as 5.6 says.

## 9 RISKS (the strongest counterargument first; the cheapest refuting test)

- KR-1 THE STRONGEST. The default leaves the clip Boris will try FIRST unlocked. He answered "9 b" and wrote "the
  play head moves past them on time"; at his own 120 nearly every clip has a Beats number that is
  not whole, so on that clip he gets neither until he types a whole number -- and the gates seat called exactly
  this a contradiction of his words (GA-1). Why the ruling still chooses it: a loop of 14.6 beats cannot keep its
  BPM at 120, keep its speed steady AND cross its lines on the beat -- one of the three has to give, and only he
  can say which. Of the three, the steady speed is the one the ROOM never sees; the pulse changes the picture on
  every pass of every default clip; rounding rewrites a number he ruled. The default is one predicate, Q-D1 is the
  first question on his page, check 30 shows it to him, and S4c is five stages away. If he says B or C the change
  is pre-specified (DA-1). Cheapest refutation: his answer; check 30.
- KR-2 The lock changes a clip's speed by itself, on numbers chosen by reading -- the delta plan's own strongest
  risk, still true at 6 %. On real music the tracker realigns its phase ("confident detection"); every realign is
  a slide, and a tracker that realigns often makes a BPM-synced clip breathe by up to 6 %. Cheapest refutation:
  FM-5, one minute of real music, before S4b starts. Unit side: TL-U24d; if FM-5 fails, TL-U24 again with a
  recorded phase trace.
- KR-3 The 6 % is MY number, put in place of the plan's 10 %, also by reading. It makes a pull-in slower (up to 9
  beats). If checks 27-28 read as "too slow", the constant moves toward 0.10 -- and then TL-U23, TL-U24, TL-U25,
  TL-U25c, TL-U28, TR15, TR16, TR17 are re-registered BEFORE that build, never after a run.
- KR-4 A trimmed loop loses 1 to 2 frames at every wrap and the lock hides it by running a few per cent fast for a
  beat or two after each loop point; such a clip rests about 0.03 beat behind the beat. If Boris trims most of his
  loops he will see the playhead a hair late at every loop point (check 32). The cure is in the players' advance,
  the riskiest files of the app; it is named, not built. Refutation: TL-U25c, FM-4.
- KR-5 Kept layers (DA-6). The rule "erase by id, re-insert at the recorded index" is derived by hand over three
  sequences (RD-8); it is not proved for every interleaving. Refutation: TL-UC20 and the 200-step walk, written
  before any command changes. If the walk cannot be made GREEN inside S1 the fallback is the seat's: an Undo that
  keeps a layer ends the history -- and Boris is told.
- KR-6 The lock is modulo one beat (F20). After a drop, a Resync or an unquantized fire the loop's first beat can
  sit on beat 2, 3 or 4 of the bar, and stays there. His words ask for lines on beats, not for the loop on the bar.
  Refutation: checks 26-28.
- KR-7 A layer move ends the Undo history, silently (F26). "It changes anything else" is narrowed for every edit
  before a move. Q-D3, check 36; S1m is named.
- KR-8 The strip rule's edges are readings he has not seen: effects stay undoable (Q-D4) -- except on a playing
  clip that is only effects, where Cmd+Z cannot bring back an effect he removed by mistake (check 38); a whole
  layer may still go with Cmd+Z (Q-D2); a restored layer comes back with its solo off.
- KR-9 A LOCKED tracker whose phase stands still (test mode after `set_bpm`; perhaps a typed tempo) would make the
  lock chase a fixed phase and swing the speed by 6 % each way. Not read (BPMTracker). S0 settles it before S4c;
  the off-condition is pre-specified. TR9 would show it: its rate bar is 3 %.
- KR-10 The lock follows the snapshot, which is up to a hop old, and the picture reaches the screen a frame or more
  later: the clip sits a constant few milliseconds behind the sound. That offset is the sync-dial lane's subject;
  this lane promises only the phase the quantized release uses (S0).
- KR-11 Withdrawing AM-22's slew leaves "FM-3 over its bar" with no pre-decided cure. Accepted: a wobbling tempo
  with the lock is a new problem and deserves its own look; the deadband already absorbs a wobble of the phase.
- KR-12 Thinned lines. Every 4th beat is a bar in 4/4, and he wrote "bars will confuse this". They carry no label
  and no extra weight; H-D2 holds the other way; the visual gate's seats judge V14.
- KR-13 The greyed S fader is a reading (Q-D6). It takes one control away from a clip in beats mode.
- KR-14 The beat-driver rows run at 60 BPM; TR16 at 90 needs posts at 75 Hz. If the route cannot take it the row is
  BLOCKED, not loosened; TL-U24 .. TL-U28 remain the proof.
- KR-15 The hold freezes the output for as long as he holds (his choice, "2 b"). Base RE-2 and RE-3 live here;
  AM-12's cases are the guard. A speed that changes every frame under reverse play is new load on the GOP cache
  (Pitfall 62): probe-video is run whole at S4c.
- KR-16 Line numbers are those of 34179a2; the sync-dial lane moves them. S0 exists for that. The facts
  this file carries from the delta plan were re-read at the pin (RD-24 .. RD-26).

NOT IN THIS LANE: a Resync that restarts clips; a lock to the bar; removing the trimmed loop's lost frames; the
slew of the trim; the re-numbering of history after a layer move (S1m); reverse play leaving a trimmed range (base
SF-5); rescaling the bar to in..out; wiring the strip's drag reorder; greying a refused Redo (Edit-menu lane); any
bars-lane label; "12 b" and every failure text (notices lane); the Snap and Quantize menus.

SIDE FINDINGS (outside this lane; for Harmony)
- SF-D1 INFERRED (InsertDeckCmd read at DeckCommands.h:832-833 and :802; AddDeckCmd only by its lines :696 and
  :705-712): both re-insert the Deck value they captured on their first execute: every tweak to that deck's clips that is no
  Undo step (in and out points, transport, clip BPM) is lost when an idle deck is undone and redone. The same
  class as UN-3, but the deck is idle, so no strip field is touched; the sibling of base SF-7. Not fixed here.
- SF-D2 Learnings for Harmony to log (a project-repo boot has no log-event fence): (a) a gate row that proves "X
  eases onto Y" must first assert that X starts away from Y; (b) a lock that promises "on the beat" needs the real
  wrap in its unit cases -- a fake that wraps ideally hid 1 to 2 frames per loop; (c) when a ruling says "cannot
  shift a row", walk a KEPT element through the Undo and the Redo of an older structural step; (d) a stateless
  phase lock has an unstable point half a period away -- start a live row a quarter away.

22 of 22 attacks ruled: 11 ACCEPT, 11 PARTIAL, 0 REJECT. 10 delta amendments. Sections 0-9 complete.
BUILDABLE from plan-transport.md's body + ruling-transport.md + this file, this file overriding.

STATUS: DONE
