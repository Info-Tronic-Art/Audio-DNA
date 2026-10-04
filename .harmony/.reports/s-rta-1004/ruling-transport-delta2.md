# ruling-transport-delta2 -- architect ruling on the blind council's attacks on plan-transport-delta2.md (s-rta-1004)

Role: architect, ruling (opus, max effort; Fable is out of usage). Read-only. Nothing built, nothing run, no app launched.
Pin: main 185147b; `git status --short -- src tests docs CMakeLists.txt` printed nothing, so every src / tests / docs line
below is a plain-file read at the pin. Worktrees bf2 (740b6d6) and bf2keys (9eab9bd) are clean and were not read for code.
Papers (4 of 4 seats, 31 attacks, 39807 characters, parsed whole):
.harmony/.reports/s-rta-1004/attack-transport-delta2-papers.md
Precedence after adoption: this ruling > plan-transport-delta2.md > ruling-transport-delta1.md > ruling-transport.md >
plan-transport.md.
Short names: PL:n = plan-transport-delta2.md line n. RT = ruling-transport.md, RD = ruling-transport-delta1.md (both in
.harmony/.reports/s-rta-1003b/). BD:n = .harmony/binding-decisions.md line n. BL:n = .harmony/boris-feedback-backlog.md
line n.
Labels: VERIFIED = read by me in the named file; INFERRED = reasoned from verified lines, not run; ASSUMED = neither.
Boris is quoted only verbatim and only from BD / BL. Amendments here are RA-1 .. RA-17; each OVERRIDES the plan body.

## 0 VERDICT

The plan NEEDS REVISION before a builder starts; its shape stands (his rulings are built his way, the lock is measured
before it is promised). 31 attacks ruled: 17 ACCEPT, 11 PARTIAL, 3 REJECT. 17 amendments.
- ONE FINDING NO SEAT MADE changes the first stage (V9). A `--test-mode` instance never starts the analysis thread
  (src/MainComponent.cpp:1828-1837). The plan's instrument is switched on only with `--test-mode` (PL:456-457) and its runs
  M2, M3, M5 are "her own test-mode instance ... audio from a FILE, which the app analyses" (PL:472-481). In that instance
  there is no tracker: no Auto, no Manual free-run, no Tap, no Resync. The run that answers his question (M3 / X1) could not
  have been made. RA-1 re-builds the measurement on the rig's own pattern (production mode, a music file through
  `/api/perf/record`, a poller on `/api/bpm`: .harmony/probe-manual-bpm.sh:12-16). The scope seat's SC-1 asked for that
  poller on other grounds; it is now the first stage.
- The four MUSTs are upheld. VI-1: a reversed clip leaves in..out and a ping-pong never turns inside it (V1, V2); the chain
  knew and left it out (RT:1205, RD:448-449); his fit makes a trimmed range the normal case, so it is fixed here (RA-4, new
  stage SR). GA-1: TR7c presses outside in..out, where his rule now says nothing happens (V15; RA-13). GA-2: three cases
  assert code no ctest target links (V6; RA-5: pure functions and a lint). GA-3: no row reads the picture (RA-13).
- The lock stays his answer B ("b can you program this reliably or should we change the plan?", BD:817), measured first.
  The decision table had holes (VI-3, GA-5) and one bar that would have misfired: in Auto the beat COUNT and the bar's
  BEAT move on two different events (V12), so the raw bar position reads one beat off between them. RA-2 restates the bars
  on what the lock would read, RA-3 specifies the anchored reader, and the table now ends in a catch-all row.
- He was told "up to about 15 seconds" (boris-clarify-26-37.md:43). The law gives 17 s at 120, 23 s at 90, 34 s at 60, and
  it runs after EVERY fire that is not on the "1", not only after a drop (ST-2, ST-3, VI-2). Question 71 gives him the true
  numbers before S4c is built; the gate's number and the sentence he is told are now the same number (34 beats).
- Order (RA-10, RA-11): S0 -> SM-a (Harmony, no product code) -> S4t -> SR -> SM-b -> [Harmony: M1 .. M8, the verdict]
  -> S1 -> S2 -> S3 -> S3h -> MERGE 1 -> S4a -> S4d -> S4e -> S5a -> S5b -> visual gate -> S4c -> S6 -> MERGE 2.
  Only S4c waits for the verdict. If the verdict is a STOP row, merge 2 goes without the lock (a BPM-synced clip keeps the
  tempo and stays where it is dropped, as Resolume) and the lock follows as its own merge.
- Nothing here loosens a pre-registered bar. Three are TIGHTENED before any run: X4 from 40 to 34 beats, X2's start from
  44 to 36 beats, TL-U46's "never beyond 0.16" to "within 0.03" (the wrap no longer loses frames, RA-4).

### 0.1 RECONCILIATION -- one verdict for each of the 40 amendments of the chain

"Line" = the words of his that decide the row (BD line). "none" = he named nothing there; the row rides on "All defaults
good except for these:" (BD:747) or on a default he did not name (said in the row). 16 STAND, 19 AMENDED, 5 DROPPED.

| id | verdict | his line | what replaces it / what stays |
|---|---|---|---|
| AM-1 queue-only transitions never run the tail | STANDS | "42 default" (BD:844) | unchanged |
| AM-2 the restart stamp | STANDS | "42 default" | unchanged; a fire that does not restart (RA-7) mints no stamp |
| AM-3 "every fire plays" | AMENDED | "when you pause a clip and then fire it, it stays, paused" (BD:748); "stays in layer strip paused" (BD:806) | the pause survives a fire (RA-7); `hasBeenTriggered` is still deleted; FM-2 still reported |
| AM-4 a held key fires once | STANDS | none | unchanged |
| AM-5 a fire on a layer that is not drawn | STANDS | none | unchanged |
| AM-6 the landing rule, `TransportState` | AMENDED | "lets do bars here not beats. I know I said beats before but lets do bars" (BD:761) | the rule stands; the field list loses `clipBpm`, gains `clipBars`, `syncSpeed`, `timelineRate`, `randomInterval`, `randomDistance` |
| AM-7 a refused Redo | STANDS | none | unchanged |
| AM-8 one "live" test | STANDS | none | unchanged |
| AM-9 a loaded deck's layers stay | STANDS | none (question 8 kept its default, BD:789) | unchanged |
| AM-10 Q3 widened | DROPPED | "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo, etc." (BD:764) | stays replaced by DA-5 / DA-7 |
| AM-11 the drag, `scrubClip` | AMENDED | "You can click outside the timeline and In-N-Out points, but that won't do anything." (BD:754) | a press outside in..out is ignored (PL D2-7); the rest stands |
| AM-12 the hold record, `Intent` | AMENDED | "2 b" (BD:750) | built (DA-3); `Intent` gains `paused` (RA-7) |
| AM-13 a drop past the out marker | AMENDED | "if you drag the play head, it cannot be dragged outside of the in and out points" (BD:753) | stands for a drag that began inside; its clause "(so is a trimmed PingPong, R9)" is struck: a trimmed ping-pong now turns (RA-4) |
| AM-14 `clipBpm` | DROPPED | none of his names it: question 30 kept its default "no BPM box" (BD:828, Harmony's record line) | `Relaxed<int> Clip::clipBars`; its static_assert pin and TSan case take AM-14's place |
| AM-15 S4a additive; old fields leave with the widgets | AMENDED | "mimic exactly how resolume is doing" (BD:757) | same shape; its test rows re-aimed at `clipBars` |
| AM-16 old shows | AMENDED | "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples." (BD:812) | RA-14; question 73 |
| AM-17 the widgets | AMENDED | "I want you to study these images and mimic exactly how resolume is doing. It's transport control." (BD:757) | PL D2-1: no BPM row, no Reverse text button, the "do" route's new actions |
| AM-18 REST fields, beat lines, the inert combo | AMENDED | "Timeline only shows bars." (BD:825); "42 default" | REST `clipBars`, `syncSpeed`; bar lines; the combo's removal is his now |
| AM-19 unit gates by name | STANDS | none | unchanged |
| AM-20 live rows | AMENDED | none (follows the rows above) | section 5.3 here |
| AM-21 the stale frame, S2b | STANDS | none | unchanged |
| AM-22 rate, wobble, drift | AMENDED | "9 b" (BD:763); "yes but this will be bars now" (BD:785) | the tempo store stands; FM-3 = X7, FM-4 = X2's clause, both measured before the lock is built |
| AM-23 no steady beat: a re-fire restarts at once | STANDS | none | unchanged |
| AM-24 S0's list | AMENDED | none | section 4, S0 |
| AM-25 T6 corrections | AMENDED | none | k10 stands; AM-15's rows as re-aimed |
| AM-26 the inert Duration row is removed | AMENDED | "mimic exactly how resolume is doing" (BD:757; reading R9 told to him, boris-clarify-26-37.md:72) | the dead widgets go; the row is REBUILT live: "Duration" in Timeline, "Bars" in BPM Sync |
| AM-27 no event text, no failure text | STANDS | "Nothing else." (BD:774); "display not enough HDD space to record." (BD:777) | this lane adds none of the allowed texts; a greyed control, a number, "--" are states |
| AM-28 the visual gate | AMENDED | none | five seats stay (RA-17); states and bars as section 5.5 |
| AM-29 S1 may start before the sync-dial lane merges | DROPPED | "replace our sync with this" (BD:795) | the dial does not merge (H-17); the base is main |
| AM-30 what no trim may cut | STANDS | "42 default" | unchanged |
| DA-1 the beat lock | AMENDED | "yes but this will be bars now" (BD:785); "b can you program this reliably or should we change the plan?" (BD:817) | the same law aimed at the bar; measured first (RA-1 .. RA-3); `beatLockApplies` and Q-D1 go |
| DA-2 beat lines | AMENDED | "Timeline only shows bars. The only place we see beats is in the circle with 4 positions in top bar that shows the 4 beats repeating." (BD:825) | one line per bar; the thinning rule kept |
| DA-3 the hold | STANDS | "2 b" (BD:750) | unchanged |
| DA-4 the range is the timeline | AMENDED | "You can click outside the timeline and In-N-Out points, but that won't do anything." (BD:754) | its bullet "a press ... in the dark part ... lands on the nearer point" is replaced; its last bullet "NOT in this lane: reverse PLAY leaving a trimmed range" is REVERSED by RA-4 |
| DA-5 the strip rule | STANDS | "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo, etc." (BD:764) | the strip fields gain the layer's pause |
| DA-6 layers that come and go | STANDS | "default is ok. If a layer strip is deleted, user can ctrl-z to get it back, but clip is not playing" (BD:767) | `restoredForUndo` also returns the layer not paused |
| DA-7 a layer move ends the history | STANDS | none (question 22 kept its default, BD:789) | unchanged; S1m stays unbuilt |
| DA-8 units: beats | DROPPED | "lets do bars here not beats. I know I said beats before but lets do bars" (BD:761) | the row is "Bars" |
| DA-9 the S fader is disabled in BPM Sync | DROPPED | "doubles and halves and then smaller fractions to higher multiples, just like resolume does" (BD:787) | nine steps (PL D2-4) |
| DA-10 labels and ids | AMENDED | none of his; Q-D1 is closed by "we are going to use whole bars instead, so this doesn't happen" (BD:778) | Q-D1 .. Q-D6 closed as PL:165 says |

The gate rows and stages of the chain: PL TD1 C and D stand as written EXCEPT: TR7c is AMENDED, not STANDING (RA-13);
TL-U46 and TL-U47 are restated (RA-4); TL-U36, TL-U37, TL-U71 are re-aimed at pure functions (RA-5); MU-80's live arm
goes (RA-13); the stage table is section 4 here.

### 0.2 THE FIRST BARS NUMBER AND THE OUT POINT, for a clip of any length

L = the marked length in seconds (in point to out point, on the file). One bar at 120 BPM lasts 2.0 s, so 4 bars last 8.0 s.
tol = the larger of 0.05 s and two frames of the clip's own frame rate (RA-6).
1. Bars = 4 x floor((L + tol) / 8), never below 4 (and never above 4096). Always the constant 120, never the tempo of
the moment.
2. L shorter than 8 s - tol: the out point is NOT moved; the whole clip is played over 4 bars, slower than it was shot
   ("a short clip can have 4 bars", BD:846; "47 default", BD:865). Speed makes it fast.
3. Otherwise the out point moves IN to in point + Bars x 2.0 s -- never out, never past where it already is
   ("If it is uneven, then move the outpoint in to keep those multiples", BD:812).
4. The speed the picture then has = L' x tempo / (240 x Bars), L' = the marked length after step 3, times the Speed step.

| clip (in 0, out at the end) | Bars | out point | speed at 120 | at 90 | at 174 |
|---|---|---|---|---|---|
| 2 s | 4, whole clip | unchanged (2.0 s) | 0.25 | 0.1875 | 0.3625 |
| 7.3 s | 4, whole clip | unchanged (7.3 s) | 0.9125 | 0.684 | 1.323 |
| 9 s | 4 | moved in to 8.0 s (0.889 of the file) | 1.0 | 0.75 | 1.45 |
| 45 s | 20 | moved in to 40.0 s (0.889 of the file) | 1.0 | 0.75 | 1.45 |

With the number unchanged, pulling the out point in afterwards slows the picture and pulling it out speeds it up
("within that same amount of bars, it goes through less video, appearing to play slower", BD:780). The two other examples
that matter to the tolerance: 15.98 s -> 8 bars, out unchanged, 0.999; 15.90 s at 30 fps -> 4 bars, out at 8.0 s (FM-7
decides
whether that cliff is ever met by a real file, RA-6).

### 0.3 THE MEASUREMENT THAT ANSWERS "can you program this reliably or should we change the plan?"

What can make it unreliable is not the arithmetic: it is whether the app's own "1" stays put against time. A lock that
follows the bar follows every move of that "1", and each move is a slide of up to 34 beats for every synced clip.
- FIRST (SM-a; Harmony; no product code): three music tracks, Auto, ten minutes each, the app in production mode with the
  file as its sound source, `/api/bpm` polled at 50 Hz. Read: N1 = how often the "1" moves and stays moved; R1 = the share
  of reads in which the raw bar position disagrees with the settled one; X7 = the tempo's wobble.
- SECOND (SM-b; Harmony; a test-only instrument): the lock's law on a real player -- exact clock, Manual, drops, Taps and
  Resync, x8 and x16, and the fallback's own seek.
- THE ANSWERS, pre-registered (section 3, RA-2):
  everything met -> "Yes." The slide is built, on the raw bar (O1) or on the anchored bar (O2: clips follow a moved "1"
  two bars later).
  the "1" moves more than once in ten minutes on a track (O3) -> STOP AND ASK. Fallback named: in Auto the clip is held on
  the BEAT (at most half a beat, 4.6 s at 120), on the BAR when he taps and Resyncs. He would see: lines on the beat
  always, on the "1" only after a Resync.
  the slide itself is not smooth or not accurate on an exact clock (O4) -> the fallback is ONE CUT on the next "1": the
  clip plays on from where it is and jumps once into time. If that seek is not clean either (O5) -> STOP AND ASK; until
  then a clip keeps the tempo and stays where it is dropped until it is fired again, as Resolume.
  x16 or x8 does not play cleanly (O6) -> the Speed list stops at the highest clean step; he is told and asked.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on)

- V1 VERIFIED src/render/ClipTransportSync.h:56-71. `writeBack` tests only `clip.outPoint < 1.0f && ph >= outPoint`; a
  OneShot is stopped, every other style gets `player.seekTo(inPoint)`. There is no test at the in point and no test of the
  direction. The branch runs whatever the intent was (pushed false or true).
- V2 VERIFIED src/media/VideoPlayer.cpp:419-454 and src/media/ImageSequence.cpp:153-183. Both players handle only the
  file's two ends: `>= duration` and `< 0`. A reverse Loop wraps to `duration + fmod(...)` (the file END); a PingPong
  reflects only at 0 and at the duration. CONSEQUENCE (INFERRED from V1 + V2, not run): with out < 1 a reversed clip runs
  from in down to 0, wraps to the file end, is sought to in, and so plays only [0, in]; with in = 0 it stands at the start.
  A trimmed PingPong is sought to in on its forward leg and never turns. src/media/SeqVram.h:155-171 models exactly that
  ("the out point: seekTo(inPoint)", "reverse wraps to the end (an active out point: to in)"). The chain recorded it and
  left it out: RT:1205 (SF-5), RD:448-449. The seat's claim holds.
- V3 VERIFIED VideoPlayer.cpp:354-375. `seekTo` is a request; the frame that consumes it does NOT advance the clock. So
  every wrap that `writeBack` makes costs that frame and drops the overshoot (the chain's RD-12 "1 to 2 frames").
- V4 VERIFIED RT:834: the chain's TL-U4 is "a reversed clip restarts just inside its out-point" -- the player's `restart()`
  (AM-2). So VI-1's side remark "no changed line is named for a reversed fire" is answered by a case that STANDS; the
  model tail's write of `inPoint` (src/model/Layer.h:553) is what the strip shows for one frame only. His line for it:
  question 6 kept its default, "a clip set to play backwards starts from its end" (BL, record of 11:59:06; Harmony's text).
- V5 VERIFIED Clip.cpp:65-71, :213-226: `transportMode`, `loopMode`, `speed`, `reverse`, `inPoint`, `outPoint` are saved
  and loaded. A composition file can therefore put a reversed or ping-pong trimmed clip on the FROZEN app: TR31 has a
  RED arm.
- V6 VERIFIED tests/CMakeLists.txt:43-44 and :66-67 (MainComponent.cpp and Renderer.cpp are in no ctest target). Also
  VERIFIED: the REAL players are linked headless -- VideoPlayer.cpp in test_video_player_open (:2900-2902) and
  test_video_decode_trace (:2934-2936), ImageSequence.cpp at :2799-2802. A real-player case is possible.
- V7 VERIFIED src/api/ApiServer.cpp:729-733: `/api/set_clip_param` accepts ONLY `fitMode` at the pin. No REST route writes
  a clip's speed, direction or loop style on main.
- V8 VERIFIED ApiServer.cpp:836-860: `/api/bpm` answers from ONE `featureBus_.read()`: bpm, beatPhase, barPhase,
  beatInBar, barCount, downbeatDetected, totalBarCount, resyncBarOrigin, totalBeatCount. No `trackerState`: a grep of
  src/api and src/testing for it finds nothing. SC-1's claim holds for X1; X7 "while LOCKED" cannot be conditioned on the
  state from this route (a pass over ALL reads is sufficient; a fail is undecided until SM-b's log, RA-2).
- V9 VERIFIED MainComponent.cpp:1828-1837: "Start analysis (skip in test mode -- features are injected via HTTP)"; the
  analysis thread starts only `if (!testMode_)`; the snapshot's own comment says "in test mode (no analysis thread)"
  (src/analysis/FeatureSnapshot.h:139-144). The rig's own probes say the same and show the way: production mode,
  port 7070 (.harmony/probe-tempo-start.sh:38), a file through `POST /api/perf/record` with `audioFile` and `"audio":
  false` "plays ... into the analysis thread -- deterministic, no mic" (.harmony/probe-manual-bpm.sh:12-16), and "no
  REST/OSC route leaves manual mode" (:18-19; INFERRED by me from that header, the route bodies were not read).
  ALSO VERIFIED ApiServer.cpp:298-345: every `/api/debug/*` route is compiled under `#if AUDIODNA_TEST_SERVER` and "needs
  no --test-mode" (:299-301): a test-server build answers them in production mode. SM-b's routes follow that rule.
- V10 VERIFIED src/analysis/BPMTracker.h:41-42 (kMinBPM 60, kMaxBPM 200) and BPMTracker.cpp:613 (a typed or tapped tempo
  is folded into that range). 60 BPM is the slowest tempo the app runs at. VI-2's citation holds.
- V11 COMPUTED. Error 2 beats, 0.06 beat gained per beat down to 0.12, then a 2-beat time constant: to 0.03 beat
  (2 - 0.12) / 0.06 + 2 ln 4 = 31.3 + 2.8 = 34.1 beats; to 0.10 beat 31.3 + 2 ln 1.2 = 31.7 beats. From 1.8 beats: 28.4 beats
  to 0.10. From 1 beat: 17.4 beats to 0.03. In seconds, worst / typical: 60 BPM 34.1 / 17.4; 90 BPM 22.7 / 11.6; 120 BPM
  17.1 / 8.7; 174 BPM 11.8 / 6.0. The plan's 34.1 holds; its table lacks 60 BPM; its bars X2 (44 beats) and X4 (40 beats)
  are looser than the sentence in O1 (PL:487-498). ST-2 and VI-2 hold.
- V12 VERIFIED BPMTracker.cpp:225-231 (the count rises on the phase wrap), :234-236 and :253-258 (a confident detection
  realigns; from the second half of a beat it completes the beat), :343-346 and :377-382 (the bar's beat, `beatInBar`,
  rises in `scoreBeat` on a DETECTED beat of any confidence), :243-246 (a wrap moves `beatInBar` only in the predicted
  regime), :486-499 (bar position = `beatInBar` + phase). CONSEQUENCE (INFERRED, not run): with real onsets the count and
  the bar's beat move on two different events. Between them `beatInBar + beatPhase` reads one beat off, and
  c = (totalBeatCount - beatInBar) mod 4 flips for that gap, on most beats. So: (i) the seats are right that c does not
  see a pure phase step (a realign from the first half moves the phase and neither number); (ii) c also reports a "slip"
  on ordinary beats, so "at most 1 per 10 minutes, raw" (PL:486) would fail on any music for a reason that is not a moved
  "1"; (iii) a lock on the raw bar would see a one-beat error for those frames. X1 is re-defined (RA-2).
  ALSO VERIFIED :99-117, :134-142: the regime is decided PER HOP -- a hop in which aubio gives no tempo (`rawBpm <= 0`)
  with a locked tempo is "predicted", and a wrap in such a hop moves the bar's beat too. INFERRED: across one beat both
  rules can fire, or neither, and the "1" then moves for good. How often is exactly what N1 measures.
- V13 VERIFIED BPMTracker.cpp:580-584, :592-604, :606-622: a Tap is tempo + `realignPhaseToZero`; :253-258 never touches
  `beatInBar`; in the predicted regime only a WRAP moves it (:243-246, :260-277). INFERRED: a Tap in the second half of a
  beat completes the beat without a wrap, so the bar loses a beat. A Tap does not enter Manual (facts-beat-controls.md
  section 3 item 2). VI-4's citations hold.
- V14 VERIFIED plan-nudge.md:23, :213-222, :253-255, :217, :444-445: the nudge is applied ONCE, on the analysis thread,
  before the snapshot is published; a reader of FeatureSnapshot gets the shifted beat; the only BPMTracker file the
  nudge plan claims is BPMTracker.h ("`bool predictedBeatRegime() const` only"). That plan is not ruled yet (no
  ruling-nudge.md in the folder). SC-7's premise ("they own the beat clock and BPMTracker") does not hold as planned.
- V15 VERIFIED RT:987-991 and RD:904: TR7c presses at 0.95 with out 0.8 and at 0.05 with in 0.2. Both are presses outside
  in..out. Under PL:411 they do nothing: the OneShot bar ("`playing` false") fails, the Loop bar passes only by wrap
  timing, the "no read below 0.2" clause is empty, MU-10 cannot go RED. GA-1 holds.
- V16 VERIFIED (read by me) boris-resolume/resolume-transport-2.png and -7.png: TWO small menus right of the button row
  (a loop arrow; a bar-and-arrow). The plan's own P1 says so (PL:100). ST-7 holds.
- V17 VERIFIED src/ui/ClipInspector.cpp:162-167: Snap Off, Beat, Bar, 2 Bar, 4 Bar. Only Bar and above land on the "1".
- V18 VERIFIED boris-clarify-42-44.md:27-28: R16's second sentence ("A different clip fired on that paused layer shows its
  first frame and waits, paused") was TOLD to him as a reading ("he corrects only what is wrong"); his answers of
  12:31:04 came after it and do not mention it. It is not his word. R6 of the fact sheet (staff: pause is the clip's in
  Resolume, facts-resolume-transport.md VERIFICATION refutation 3) stands.
- V19 VERIFIED PL:546-550: Interval and Distance share {1/16 .. 4} bars, and a jump is "a whole number of BEATS". 1/16 bar
  is 0.25 beat. The rule and its list contradict each other. VI-5 holds. COMPUTED: 1/16 bar at 174 BPM = 86 ms.
- V20 VERIFIED src/ui/LayerStrip.cpp:368-392: four buttons; the fourth is captioned ">|" (the plan writes ">>"), doubles
  the speed up to 4. His list names three: "play, play reverse, pause" (BD:764). He asked for no removal.
- V21 VERIFIED docs/claude/pitfalls.md entries 52, 56, 60 (a capture is one frame under its own time override; the shown
  frame is the ring's pick, not the clock); RT:948-958 (TR2 .. TR4 already read frame codes from the app's own capture).
- V22 COMPUTED 4 x floor((15.94 + 0.05) / 8) = 4: a file 0.06 s short of 16 s gets 4 bars and its out point at 8.0 s.
  VI-6 holds.
- V23 VERIFIED BD:864-871, :879-881: questions 47, 48, 49, 50 are answered ("47 default", "48 b", "49 default", "50
  default", then 50 changed). The dispatch lists them as open; the plan's correction (PL:9-10) is right.
- V24 VERIFIED PL:110 says "D2-1 .. D2-14"; the body defines D2-1 .. D2-11 only (a grep for D2-12, D2-13, D2-14 finds
  the one line). No content is missing; the count is off.
- V25 VERIFIED Clip.h:237-240 (`playing`, `playheadPosition`, `beatsPlayed`, `hasBeenTriggered` are Relaxed runtime
  fields); src/ui/ClipInspector.cpp:37-53 and LayerStrip.cpp:370-385 (pause today writes the CLIP's `playing`).
- V26 VERIFIED VideoPlayer.cpp:411-414, :427-430, :445-448 and ImageSequence.cpp:142-146, :158-160, :175-177: in PingPong the
  direction is the leg XOR `reverse`, and each end SETS the leg to a constant. Renderer.cpp:1647-1648 and :1707-1708 push
  `reverse` only when the style is not PingPong. INFERRED (not run): a PingPong whose player still holds `reverse` (the
  clip was reversed before the style was chosen) runs to one end and bounces there, and the play-backwards button changes
  nothing on a Ping Pong clip. No seat raised it; it sits in the lines RA-4 rewrites.
NOT VERIFIED BY ME (each is a run or a file I did not open): every X bar; what `getDuration()` returns for real files;
the body of `/api/perf/record`; the GL-thread site for Eject; the nudge lane's final ruling.

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)

| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| VI-1 | MUST | ACCEPT | V1, V2, V3: the transport cannot run backwards inside in..out and a trimmed ping-pong never turns; RT:1205 and RD:448-449 left it out when a trimmed range was rare; his fit (BD:812) makes it normal. The side remark on the reversed fire is answered by TL-U4 (V4). Fixed in the players, not in `writeBack` (why: RA-4). | RA-4 |
| VI-2 | SHOULD | PARTIAL | V10, V11. ACCEPTED: 60 BPM in every table and sentence; check 14's wording. REJECTED: the jump "as a setting now" -- a setting nobody asked for; it is offered to him as question 71's B instead. | RA-9 |
| VI-3 | SHOULD | ACCEPT | PL:494-506: "X1 met, X2 or X3 fails in M3" matches no row. V12: c is blind to a phase step AND flips on ordinary beats. | RA-2, RA-3 |
| VI-4 | SHOULD | ACCEPT | V13; PL:481 puts M5 before S4t; PL:490 "settled by 40 beats" hides a 34-beat slide. S4t is built first, so M5 runs once, on the final tracker. | RA-2, RA-10 |
| VI-5 | SHOULD | ACCEPT | V19. Distance is whole beats; Random, BeatLoopr and Catch Up do nothing while the clip is not running; the seek cadence is reported. | RA-8 |
| VI-6 | SHOULD | PARTIAL | V22. ACCEPTED: the tolerance comes from the file (two frames), the duration sample is a pre-registered measurement, TL-U33 gains the rows. NOT pre-decided: the 0.5 % term -- a container's error is counted in frames; FM-7 decides. | RA-6 |
| VI-7 | SHOULD | ACCEPT | PL:483 names no direction or trim; VideoPlayer.cpp:505-506 (speed 0 = no next frame) is a read. M7 gains the arms; they run on a tree that has RA-4, or reverse on a trimmed clip is broken by V2, not by the speed. | RA-12 |
| VI-8 | SHOULD | ACCEPT | V1: the range branch runs for a paused clip. With RA-4 the branch is gone and a player that is not playing never moves its clock; RA-7 adds the rule for an out point moved past a paused playhead. | RA-4, RA-7 |
| GA-1 | MUST | ACCEPT | V15. | RA-13 |
| GA-2 | MUST | ACCEPT | V6; PL:332-336 puts the rate, the fit and its list in Renderer.cpp / MainComponent.cpp. | RA-5 |
| GA-3 | MUST | ACCEPT | V21: the model number is the clock, the picture is the ring's pick; TR2 .. TR4 show a machine can read it. Narrowed to STILL states and to loop bounds (a capture of a clip at x16 is one frame of it). Boris's checks stay as things he sees. | RA-13 |
| GA-4 | SHOULD | ACCEPT | PL:388-392 + PL:540-541: a paused layer's clip never ends, so the paused arm holds with or without the check. | RA-13 |
| GA-5 | SHOULD | ACCEPT | as VI-3. | RA-2, RA-3 |
| GA-6 | SHOULD | ACCEPT | PL:469 has no field for the shown frame; V21. | RA-2 |
| GA-7 | SHOULD | ACCEPT | PL:876-878 against PL:871-873 and PL:226; `/api/debug/undo` and `/api/trigger_column` exist at the pin (ApiServer.cpp:340, :182). | RA-13 |
| GA-8 | SHOULD | ACCEPT | PL:841-845 give no threshold; PL:684's own 4 x noise rule has no measurement behind it for these rows. | RA-13 |
| ST-1 | SHOULD | PARTIAL | V18. ACCEPTED: he is asked BEFORE S2 (question 72), not shown after. REJECTED: a per-clip held frame on top of the layer's pause -- two models at once. | RA-7 |
| ST-2 | SHOULD | ACCEPT | V11. One number: 34 beats. | RA-2, RA-9 |
| ST-3 | SHOULD | ACCEPT | PL:910, PL:926 name only a drop; PL:815 fires mid-bar on purpose; V17. He is told, and question 71 carries it. A fire that starts inside its first bar is not offered: "42 default" (BD:844). | RA-9 |
| ST-4 | SHOULD | PARTIAL | PL:552-555. ACCEPTED: the screenshot is asked NOW (RQ-1, RQ-2), the lists are data, the gate compares when it has arrived. REJECTED: a stage that cannot start until he answers. | RA-8 |
| ST-5 | SHOULD | PARTIAL | PL:289-290, PL:325-329. ACCEPTED: an old show's file is never changed by opening it; question 73 is on his page before S4a; the way back is shown to him. REJECTED: a saved "out point before the fit" -- a second out point nobody asked for; the default of 73 stays A (his "always", BD:812). | RA-14 |
| ST-6 | SHOULD | REJECT | BD:812 allows only multiples of 4. Half of 12 is 6. A "/2" that gives 8 is not a halving and "x2" would not undo it (16). Greyed is a state; minus gives 8. Shown in check 10 so he can say otherwise. | RA-15 |
| ST-7 | NIT | ACCEPT | V16. | RA-15 |
| SC-1 | SHOULD | PARTIAL | V8, V9. ACCEPTED: SM-a, the poller, first and with no product code -- it is also the only way the Auto run can be made. REJECTED: building SM-b "only if", and shortening M1 / M2 (X2's fifth-minute clause is the chain's FM-4). The probe calls the product functions, so S4c swaps its inputs and re-writes nothing. | RA-1 |
| SC-2 | SHOULD | PARTIAL | PL:660-664: no panel or strip case reads the lock. ACCEPTED: only S4c waits; two merges are the default. REJECTED: the measurement last -- Harmony constraint: it is the first stage. | RA-11 |
| SC-3 | SHOULD | PARTIAL | V18: the seat is right that the second sentence is not his. But the first-told reading stands until he says otherwise, and "stays in layer strip paused" (BD:806) is his later line. Not built his-Resolume way by default; offered as question 72's C; the sync takes the pause as a bool so C stays a contained change. | RA-7 |
| SC-4 | NIT | PARTIAL | ACCEPTED: old 71 ("/2" at 12), old 72 (bars, BD:825), old 74 (X ends the pause) are ruled here. REJECTED: removing the fourth strip button (V20: he asked for no removal). Kept: old 73 and 75, as 74 and 73 here. | RA-15 |
| SC-5 | SHOULD | PARTIAL | ACCEPTED: the screenshots are asked now; the lock is OFF inside a BeatLoopr loop. REJECTED: lock off under Random (one line keeps the clip on the beat for minutes); deferring Catch Up (it is half of the documented BeatLoopr, R8). With RA-11 neither stage waits for the verdict. | RA-8 |
| SC-6 | SHOULD | REJECT | Harmony constraint: five critic seats. And V6: for TR26, TR27, TR28, TR30 the unit twin cannot link the renderer or MainComponent, so the live row is the only proof of the wiring. | RA-17 |
| SC-7 | NIT | REJECT | V14: the nudge plan does not touch BPMTracker.cpp. The bar lock is the first reader that needs the bar's beat right after a Tap; the fix is built here, first. | RA-10 |
| SC-8 | SHOULD | PARTIAL | "9 b" (BD:763) has no exception for a fast clip, and a 1-beat loop that drifts off the beat is the most visible case. The period function stays. ACCEPTED: X6 failing no longer holds S4a (the list is data). | RA-12 |

## 3 AMENDMENTS (RA-1 .. RA-17; each OVERRIDES the plan body where they differ)

The real-time rules hold in every amendment: nothing touches the audio callback; S4t is one bookkeeping call on the
analysis thread with no allocation; the anchor, the players' range, the Random generator and the probe's write path are
plain GL-thread members or memory allocated once at start-up; the render thread never waits; NO new mutex.

RA-1 THE MEASUREMENT IS BUILT IN TWO PARTS (V8, V9, SC-1; replaces PL:456-484's switch and "THE RUNS").
- SM-a, THE "1" ITSELF. No product code. `.harmony/probe-bar-one.sh / .py` with a self-test on a synthetic log and two
  mutated copies (RIG-RULES A2: instrument first). The instance: PRODUCTION mode (no `--test-mode`), `open -g`, a normal
  cmake build of main at the pin (never a copied bundle, RIG-RULES B), under the live lock, never while Boris's own app
  runs, never an Output window, own-pid quit -- the recipe of .harmony/probe-manual-bpm.sh. Per track: a fresh launch
  (Auto: no `set_bpm`), `POST /api/perf/record` with `audioFile` and
  `"audio": false`, the take stopped at once and its folder deleted at teardown; `/api/bpm` polled at 50 Hz for 10 minutes.
  Three tracks: four-on-the-floor near 125, a breakbeat near 174 (or 87), one with a long breakdown; each file gives
  its 10 minutes in ONE pass (a DJ mix or a long track): the file's own end or loop point is never inside a run. Derived
  offline:
  u = totalBeatCount + beatPhase; c = (totalBeatCount - beatInBar) mod 4; the ANCHOR = the settled c (it starts 16 beats
  after bpm first reads above 0; a new value becomes the anchor when it has been read without a break for 8 beats of u);
  N1, R1, X7 as RA-2 defines them.
- SM-b, THE LAW ON A PLAYER. The plan's instrument, with four corrections. (1) Its switch is the test-server build plus
  the environment variable `AUDIODNA_BARLOCK_PROBE=1`, in production mode AND in test mode -- not `--test-mode`. (2) The
  probe's branch calls the product functions of ClipBarGrid.h (`barSyncRate`, `barLockErrorBeats`, `barLockTrim`, the two
  readers of RA-3); S4c swaps the branch's INPUTS (the probe's record -> the model's fields) and re-writes no law.
  (3) `POST /api/debug/barlock` gains `reader` = raw | anchored; `POST /api/debug/tempo` gains the action `auto` (no REST
  route leaves Manual, V9). (4) The sample gains the SHOWN frame's time (the ring pick's pts; a sequence's frame index)
  and a seek-pending flag: two read-only accessors on the players, GL thread, used by the probe only.
- WHERE EACH RUN LIVES. Test mode, the injected driver (an exact clock): M1, M4, M6, M7, M8. Production mode, the real
  tracker, a music file as SM-a loads it: M2 (Manual, `set_bpm` 90, 120, 174), M3 (Auto, SM-a's three tracks, the probe
  clip locked on the reader SM-a's result selects), M5 (Manual at 120: eight Taps at a 126 pace through the tempo route,
  a Resync in mid-bar, a tempo value 120 -> 128 -> 120). A clip is fired "on a 1" by its own Snap = Bar in the composition.
- M4 is 22 drops: 10 at 120 (five about 1 beat off, five about 1.8 beats off, both signs) and 4 each at 60, 90 and 174.
  (The plan listed 18 and judged "22 of 22".)
- M7 runs on a tree that has RA-4 (SR is built before SM-b): both fixtures, x8 and x16, 60 s each, in four arms: forward
  Loop on the fitted ramp (out 0.8: it wraps every 2 beats / every beat), reversed Loop, Ping Pong, and Speed 0 held 60 s
  then stepped to 1.
- Lint TL-L6 reads: "`BarLockProbe` is constructed only inside `#if AUDIODNA_TEST_SERVER` and only where the environment
  variable is checked; every use in Renderer.cpp is inside a null test; its write path holds no `new`, no container
  growth, no mutex, no file call". G-P1 stands (404 without the variable).

RA-2 THE BARS AND THE DECISION TABLE (VI-3, GA-5, GA-6, ST-2, VI-4, VI-7; replaces PL:485-507). Never loosened: met, or
reported.
- X1 the "1" against time (SM-a; confirmed on M3's log), per track, from the anchor's start:
  N1 = the number of anchor changes. Bar: at most 1 per 10 minutes on each track.
  R1 = the share of reads whose c is not the anchor. Bar FOR THE RAW READER: at most 1 %.
  (1 % is a number chosen by reading, INFERRED: at 1 % the flips pull the mean speed by 0.06 %.)
- X7 the tempo: over every 60 s window, (max - min) of bpm <= 0.5 % of the mean. SM-a judges it over all reads with bpm
  above 0 (a pass decides; a fail is undecided); M3's log, which has `trackerState`, decides a fail: LOCKED samples only.
- X2 holding (M1, M2, M3): from 36 beats after the fire, |e| <= 0.10 beat in >= 95 % of frames (M1, M2: >= 99 %); in M2's
  fifth minute 20 reads 3 s apart, >= 18 inside 0.10 beat.
- X3 smoothness (M1, M2, M3): content rate over each 2 s window against the expected rate: >= 90 % of windows within 2 %.
- X4 pull-in (M4): no playhead step back except across the loop point; the speed pushed within 6 % of the rate; the FIRST
  frame with |e| <= 0.10 beat comes within 34 beats of the seek and no later frame is beyond 0.10; in 22 of 22.
- X5 commands (M5 on the tree that has S4t; M6): no position step. After the eight Taps: c is what it was before them and
  |e| <= 0.10 from 12 beats after the last Tap. After the Resync: as X4 (34 beats). During and after the tempo value:
  |e| <= 0.15 throughout. M6 (a 30 ms step): |e| <= 0.10 within 8 beats.
- X6 x8, x16 (M7, each arm): content rate within 3 % of expected over 4 s windows; peak frame time <= 50 ms; in the
  Speed-0 arm the shown frame's time is equal over the 60 s and moves again within 0.5 s of the step to 1.
- X8 (M8): ten seeks issued on a "1"; from the log, the first frame whose SHOWN time is within one frame of the target
  comes within 3 render frames; 10 of 10. Read offline from the log, never from a capture (Pitfall 52).
- The lane report states the noise of e on a locked clip over 5 runs before any row is judged.
THE TABLE. Two verdicts are written: one tracker row (O7, O3, O2 or O1, the first that matches) and one player row (O6,
then O5, O4, O8, O9 or "met", the first that matches). O6 is written beside the others and holds nothing back. A
failing X5 is reported whichever row decides.

| row | when | what is built | Harmony tells Boris |
|---|---|---|---|
| O7 | X7 fails on M3's LOCKED samples | STOP and report; the lock is not built until the tracker is read | "The tempo the app reads wobbles more than the lock can take. I am looking at the tracker before building the lock." |
| O3 | N1 above 1 on any track | STOP AND ASK. Named fallback, built only on his word: period 1 beat in Auto, the bar in Manual and after a Resync | "In Auto the app moves its '1' too often for a clip to follow it. Either clips stay on the beat in Auto and on the bar when you tap and Resync, or we fix the '1' first. Which?" |
| O2 | N1 met, R1 above 1 % | S4c on the ANCHORED reader (RA-3) | "Yes, with one condition: when the app itself moves its '1' in Auto, clips follow two bars later instead of at once." |
| O1 | N1 met, R1 at most 1 % | S4c on the RAW reader | (nothing to add) |
| O6 | X6 fails at 16 (or at 8) in any arm | the Speed list is built up to the highest step that met X6 in every arm (data); S4a does not wait | "The player does not play cleanly at x16 (or x8). The list stops at xN for now. Leave it there, or fix the player first?" |
| O5 | X2, X3 or X4 fails on the exact or the Manual clock (M1, M2, M4) AND X8 fails | STOP AND ASK; S4c is not built; the lane goes on without the lock | "Neither the slide nor one clean cut works on this player yet. A dropped clip keeps the tempo but stays off the bar until you fire it again (as Resolume), until we fix the player. OK for now?" |
| O4 | X2, X3 or X4 fails on M1, M2 or M4, X8 met | the fallback: ONE jump on the next "1" (its cases TL-U49j and the re-stated TR21 / TR22 are registered BEFORE it is built) | "The slow slide was not smooth enough. Instead the clip plays on from where you drop it and cuts once, on the next '1', into time." |
| O8 | M1, M2, M4 met; X2 or X3 fails in M3 only | STOP and report; S4c waits. Named candidates, nothing pre-decided: a wider deadband in Auto; e averaged over one beat | "On a typed or tapped tempo the clip holds. On the app's own beat detection it trembles. I am fixing that before I promise it in Auto." |
| O9 | anything else that is not "all met": X5 alone, a BLOCKED bar, an invalid run | STOP and report with the log; an architect ruling decides; S4c waits | (the report) |
| met | X2 .. X5, X8 met | S4c as ruled: the slide | "Yes: measured over ten minutes at three tempos. A clip that is off the bar slides back onto it in at most 34 beats -- 17 seconds at 120, 23 at 90, 34 at 60 -- and stays there." |

RA-3 THE BAR'S READER (VI-3, GA-5; V12). Both readers are pure and live in src/render/ClipBarGrid.h from SM-b on:
- `double showBarBeatsRaw(int beatInBar, double beatPhase) noexcept;`
- `struct BarAnchor { int anchor; int candidate; double candidateSince; uint32_t requestSeqSeen; };`
  `double showBarBeatsAnchored(BarAnchor&, uint32_t totalBeatCount, int beatInBar, double beatPhase, uint32_t
  trackerRequestSeq) noexcept;`
  -- ((totalBeatCount - anchor) mod 4) + beatPhase. A new c becomes the candidate; it becomes the anchor when it has held
  for `kAnchorHoldBeats` = 8 beats of (totalBeatCount + beatPhase) -- the beat time is integrated, never a phase wrap
  (Pitfall 42). A changed `trackerRequestSeq` (a Tap, a Resync, a tempo or a mode command has landed in this snapshot:
  FeatureSnapshot.h:139-144) adopts the present c at once. One record for the show, updated ONCE per
  frame where the renderer reads `frameSnap_` (Pitfall 38's shape), never per clip; GL thread only; no shared field.
- `double Renderer::showBarBeats() const` returns this frame's value; S4c's verdict row says which reader feeds it.
- Case TL-U49a (registered unconditionally): "the anchored bar follows a moved 1 only after it has held 8 beats; a
  one-beat flip that lasts a fraction of a beat never moves it; a Resync is followed at once". Mutant MU-94.
- With O2 the wheel in the top bar (raw `beatInBar`) and the clip can disagree for 8 beats after the app moves its "1".
  Said to him in O2's sentence.

RA-4 THE RANGE BOTH WAYS -- the players own in..out (VI-1, VI-8; new stage SR; reverses RD:448-449 and RT:1205).
- Both players gain `void setRange(double inNorm, double outNorm) noexcept;` (GL thread; plain members). Their own
  boundary code (VideoPlayer.cpp:419-454, ImageSequence.cpp:153-183) uses the range's two ends in place of 0 and the
  duration: a Loop wraps inside the range and KEEPS the overshoot, forward to in and backward to out; a PingPong turns at
  out and at in, mirrored, with no position step; a OneShot stops on the out point's frame (reversed: on the in point's).
  A range narrower than one frame holds its first frame.
- `ClipTransportSync::writeBack` loses its range branch (ClipTransportSync.h:55-72) whole: it stores the playhead and
  compare-exchanges the play state, nothing else. A OneShot's end reaches the model through the player's own stop and
  that compare-exchange, as an untrimmed OneShot does today. `writeBack` never seeks.
- `struct PlayRange { float in; float out; };` `PlayRange playRange(const Clip&, double fileSeconds, double
  frameSeconds) noexcept;`
  in ClipTransportSync.h (in..out; from S4a on, the fitted out point of a clip not yet fitted, RA-5; from S4e on, a BeatLoopr
  loop while one is on). `syncMedia` calls `setRange` once per sync in each branch, FIRST: before a restart is applied and
  before the advance.
- src/media/SeqVram.h `step` and `atOneShotEnd` follow: a ranged PingPong reflects at the out frame and at the in frame, a
  ranged reverse Loop wraps to the out frame (:155-171 change; the look-ahead must plan along the path the player takes,
  Pitfall 54).
- THE LEG AND THE DIRECTION (V26). A turn FLIPS the ping-pong leg (today each end SETS it to a constant), and `syncMedia`
  pushes `reverse` in every loop style (the two guards at Renderer.cpp:1647 and :1707 go). So play-backwards and
  play-forwards act at once on a Ping Pong clip, and a player that still holds `reverse` from before cannot stick on an end.
- Unchanged: `seekTo` (a request), `restart()` and the stamp (AM-2; TL-U4), the hold, `scrubClip`'s clamp.
- So a player that is not playing never moves its clock, whatever the range (VI-8): TL-U68.
- WHY NOT THE SEAT'S WAY (a lower test and a turn inside `writeBack`): it keeps two boundary rules per player (the file's
  ends inside, the range's ends outside), makes every wrap and every turn a seek request that lands a frame late and
  drops the overshoot (V3), and at x16 overshoots the out point by a whole frame's advance (0.27 s of content) before it
  comes back. The chain kept away from the carry for scope ("the carry is the named candidate, nothing is pre-decided",
  RD DA-1); the fit makes the trimmed loop the normal clip and x16 wraps it every beat, so it is decided now.
- TL-U46 reads: "a trimmed loop, in 0.2, out 0.7, 4 bars, at 200 BPM through the real sync and a player that wraps
  inside its range: from the second wrap on the error is within 0.03 beat (+ 1e-6); the trim is never below -0.06 nor
  above 0.06 (+ 1e-6)". TL-U47 is registered on the REAL players (V6): the real VideoPlayer's clock and the real
  ImageSequence, driven headless through pushIntent, the speed, the advance and writeBack.
- STOP rule: if SR cannot keep `.harmony/probe-video.sh`, `.harmony/probe-media-open.sh` and `.harmony/probe-seq-vram.sh`
  GREEN, the builder stops and reports. Named fallback (an architect line decides, the same cases TL-U66 .. TL-U68): the
  range stays in `writeBack` with a lower test for a backward clock and `void turnPingPong(bool forward)` on both players.

RA-5 THE SPEED, THE FIT AND EJECT ARE PURE FUNCTIONS (GA-2; V6).
- In src/render/ClipTransportSync.h: `struct TempoView { double bpm; double showBarBeats; bool clockLocked; };`
  `template <class Player> float transportSpeed(const Clip&, const Player&, const Intent&, const TempoView&, float
  masterSpeed) noexcept;`
  -- the ONE speed either mode pushes. Timeline: speed x timelineRate, with the master speed. BPM Sync: the bar rate of the
  marked length (the clip's Bars, or its fit while `clipBars` is 0) x the speed step, x (1 + trim) from S4c on. Each of
  `syncMedia`'s two branches reads `setSpeed(ClipTransportSync::transportSpeed(...))` and computes no speed itself.
- In src/model/ClipBarFit.h (no JUCE beyond Clip.h): `bool applyBarFit(Clip&, double fileSeconds, double frameSeconds)
  noexcept;`
  (writes `clipBars` and `outPoint` once -- only while `clipBars` is 0 and the length is known; returns whether it wrote);
  `void refitBars(Clip&, double fileSeconds, double frameSeconds) noexcept;` (the right-click on Bars);
  `void setBars(Clip&, double typed) noexcept;` (through `snapBars`);
  `class PendingBarFits` (message thread; `add(clipId)`, `tick(lengthOf, clipOf)`, at most 256 ids).
  `void applyLegacyBpmKeys(Clip&, float videoBeats, float beatDivision) noexcept;` beside the loader in Clip.cpp.
- A clip in BPM Sync that is not fitted yet (`clipBars` 0, its length known on the GL thread) is PLAYED exactly as its fit:
  `transportSpeed` uses the fitted Bars and the fitted marked length, `playRange` returns the fitted out point. Nothing is
  written from the GL thread; the message thread's write is bookkeeping (the number, the pointer, the lines). This replaces
  PL K-9: no clip is played stretched, however late its fit is written. TL-U36's clause reads "a clip not fitted plays at
  the rate and inside the range of its fit, and the model is not written". A `PendingBarFits` id that falls off the list is
  fitted when the clip is next inspected, switched or fired.
- In ClipTransportSync.h: `bool ejectDue(Clip::LoopMode, bool playerStoppedItself, const Intent&) noexcept;` -- true only
  for Play Once and Eject, on the sync in which the player stopped itself, with an Intent that is running.
- TL-U36 calls `transportSpeed`; TL-U37 calls `applyBarFit`, `refitBars` and `PendingBarFits`; TL-U71 calls `ejectDue`;
  TL-U35a .. c call `applyLegacyBpmKeys`. Lint TL-L7: "in Renderer.cpp `setSpeed(` appears exactly twice inside
  `syncMedia`, each time with `ClipTransportSync::transportSpeed(` as its argument; `barSyncRate(`, `fitBars(` and
  `clipBars =` appear in no file under src/ui, src/api, and in neither Renderer.cpp nor MainComponent.cpp; the call
  sites of `applyBarFit(`, `refitBars(`, `setBars(` and `ejectDue(` are the ones S0's table lists, count pinned".
  Each of TL-U36, TL-U37, TL-U71 is shown RED by a mutant INSIDE the function (MU-64, MU-67, MU-69, MU-80).

RA-6 THE FIT'S TOLERANCE COMES FROM THE FILE (VI-6; V22).
- `int fitBars(double markedSeconds, double tolSeconds) noexcept;` `double fitTolerance(double frameSeconds) noexcept;`
  = max(`kFitTolSeconds` 0.05, 2 x frameSeconds); a frame rate that is not known gives 0.05. `fitOutPoint` takes the
  same tol.
- TL-U33 gains: "at 30 fps 15.94 s gives 8 bars, out unchanged; 15.90 s gives 4 bars, out at 8.0 s; at 24 fps 15.92 s
  gives 8; 23.9 s at 30 fps gives 8 bars, out at 16.0 s". Mutant MU-95 (a fixed 0.05) -> the 24 fps row.
- FM-7, measured by Harmony before S4a starts (read-only: the fixtures through `getDuration()`, and at least 20 of
  Boris's own clips by their container's duration): for each, the shortfall below the next multiple of 8 s when it is
  under 0.5 s. Outcome A, every shortfall is within two frames: the rule stands. Outcome B, some lie between two frames
  and 0.5 % of the length: `fitTolerance` gains the term 0.005 x L (one line; TL-U33 gains those rows; D-9's number
  changes). Outcome C, some lie beyond 0.5 %: the rule stands -- such a clip IS uneven by his rule -- and one of them is
  check 8's second clip.

RA-7 PAUSE (ST-1, SC-3, VI-8; amends PL TD5).
- Built by default as the plan: `RelaxedBool Layer::paused`. The basis is a reading told to him and not corrected (V18),
  not his word: question 72 is on his page BEFORE S2 starts and S2's builder reads the answer at stage start.
- The render sync never reads a model field for it: `pushIntent(clip, player, bool paused)`; `Intent { wanted, held,
  paused }`; "running" = wanted and not held and not paused. Where that bool comes from, who writes it and what the tail
  does on a fire are three small functions in one header, src/model/TransportPause.h: `bool pausedFor(const Layer&,
  const Clip&)`, `void setPaused(Layer&, Clip&, bool)`, `bool fireRestarts(const Layer&, const Clip& fired, bool
  sameClip)`. Answer A: as the plan. Answer B: `fireRestarts` clears the pause when another clip is fired (one line;
  TL-U61, TL-U64, TR25 (b) re-worded). Answer C: the bit is `RelaxedBool Clip::paused` (runtime), `fireRestarts` is
  false for a paused clip whichever cell it sits in, the three buttons of both surfaces write the clip's bit, and the
  Clip tab's pause is never greyed; TL-U60 .. TL-U64 are re-registered in the lane report before S2 starts.
- A fire of the active clip on a paused layer does nothing and mints no stamp; another clip (answer A) mints its stamp,
  is restarted and waits on its first frame: mutant MU-92 "a restart is not applied while paused" -> TL-U61 (TR25 b).
- When the fit moves the out point past the playhead, the fit's caller carries the playhead exactly as a marker drag does
  (DA-4's "Added 2"): it calls `scrubClip` with the clip's id and its present playhead, and the clamp lands it on the edge
  ("The play head can only go to the edges as they are defined.", BD:754). A paused clip stays paused. The SYNC itself
  never moves a clip that is not running (TL-U68).
- The layer's X ends its pause (the plan's question 74, ruled): an empty layer has no clip to hold, and a pause left on it
  would make the next fire wait for no visible reason. Shown in check 6.

RA-8 LOOP STYLES AND BEATLOOPR (VI-5, ST-4, SC-5; amends PL TD9).
- Three data lists in ClipBarGrid.h: `kRandomIntervalBars` {1/16, 1/8, 1/4, 1/2, 1, 2, 4}; `kRandomDistanceBars`
  {1/4, 1/2, 1, 2, 4} (whole beats only); `kBeatLooprOptions` {1/16, 1/8, 1/4, 1/2, 1, 2, 4}. Shown as fractions of a
  BAR ("Timeline only shows bars. The only place we see beats is in the circle", BD:825; the plan's question 72, ruled).
- Random in BPM Sync: one jump on each line of the Interval's grid in the music's beat time (totalBeatCount + beatPhase;
  it fires when the grid index passes the last one jumped on, so a small realign back cannot fire twice). The target is
  k whole beats away, k not 0, |k| beats at most the Distance, drawn evenly among the k that land inside the range;
  no k fits: no jump. The lock's period is 1 beat while Random is on (`int lockPeriodBeats(int bars, float syncSpeed,
  bool randomOn) noexcept;` TL-U42 gains "with Random on: 1"). In Timeline: seconds, as the plan.
- NOT RUNNING = INERT: no Random jump, no BeatLoopr wrap and no Catch Up seek in a sync whose Intent is not running, nor
  at Speed step 0. A BeatLoopr press on a paused clip arms the loop at the paused frame; Off on a clip that is not
  running is Off without Catch Up. Case TL-U79, mutant MU-93.
- BeatLoopr is a RANGE: while a loop is on, `playRange` returns [start, start + length) cut to in..out, and the player
  wraps inside it (RA-4); the length in clip units = loop bars x speed step / Bars of the marked part (pure,
  ClipBarGrid.h). The lock is OFF while a loop is on (TL-U44 gains the clause). Off without Catch Up: the clip plays on
  from where it is and the lock, if one is built, slides it. Off with Catch Up: ONE seek to
  `double catchUpTarget(...)` (pure, ClipBarGrid.h: the place the bar gives).
- RQ-1 and RQ-2 (section 7) go to him NOW. S4d and S4e do not wait for them; each list is one array; when a screenshot
  arrives the array, C15 and the one state's capture change, and the visual-design seat compares the row with it.
- INFO row TR28b reports Random at the shortest Interval on the long-GOP fixture (section 5.3).

RA-9 THE SLIDE, SAID TRUTHFULLY (VI-2, ST-2, ST-3; V11). The table of PL:432-436 gains its fourth row and every sentence
uses it: worst 34 beats = 34 s at 60 BPM, 23 s at 90, 17 s at 120, 12 s at 174; typical half of that. D-6, check 14,
O "met"'s sentence and K-12 read so. D-6 and check 5 say that EVERY fire off the "1" slides, not only a drop. Question
71 asks him, with the numbers, before S4c is built. The pass line of X4 and the sentence are one number (34 beats).
If he answers 71 with B, S4c builds the jump when X8 is met, whatever X2 .. X4 read; X8 not met: row O5.

RA-10 S4t IS BUILT HERE, AND FIRST (VI-4, SC-7; V13, V14). In the predicted regime a hard realign that completes a beat
(phase at or past one half) advances the bar's beat exactly as a predicted wrap does (`advancePredictedBeat`). A
confident detection, a Resync and `resetBeatPhase` are unchanged. One function, src/analysis/BPMTracker.cpp, analysis
thread, no allocation. The regime meant is the one THIS hop runs in: a request is applied before the hop's regime is set
(BPMTracker.cpp:79-104), so the builder decides the regime first or defers the advance (S0 names which). TL-U70 gains:
"with real onsets (not the predicted regime) a Tap leaves the bar's beat to the
detection". Built before SM-b, so M5 is measured once, on the final tracker.

RA-11 ORDER AND MERGES (SC-2, SC-1; replaces PL:506-507 and PL:628-629). Only S4c depends on the verdict. S4d, S4e,
S5a, S5b and the visual gate read `Renderer::showBarBeats()` and the grid, never the lock. Two merges are the default
(decision H-T4): MERGE 1 after S3h (the Tap fix, the range, undo-live, fires, pause, drag, hold); MERGE 2 the rest.
If no row allows S4c by the time S6 is done, merge 2 goes without the lock and D-6, checks 13 .. 16 and TR20 .. TR22 wait
for it. Merge 1 carries the instrument (test-server build and the environment variable only): G-P1 and TL-L6 are part
of its gate.

RA-12 x8 AND x16 (SC-8, VI-7). `lockPeriodBeats` stays (a 1-beat loop that drifts off the beat is the most visible case
of "9 b"). X6 has four arms (RA-1). X6 failing is row O6: the list is shortened as data; S4a is not held back.
Each arm also prints the player's own late and hold counters from `/api/state` (reported, no bar).

RA-13 GATE ROWS (GA-1, GA-3, GA-4, GA-7, GA-8): section 5.3 and 5.5 are the text. In short: TR7c is AMENDED into three
arms; the PICTURE is read by the app's own capture wherever the clip stands still, and against the loop's bounds where
it moves; TR30's paused arm is a guard and MU-80's teeth are TL-U71; C9 is a whitelist of strings per state; TR28 and
TR29 get numbers and a noise run; TR31 is new; `undo` joins TR26 and a column fire joins TR25, so checks 7 and 20 are
machine-proven and stay on his page only as things he is shown.

RA-14 OLD SHOWS AND THE WAY BACK (ST-5; amends PL D2-5).
- Opening a show never changes the file; the fit happens in memory and reaches the disk only when he saves.
- Question 73 keeps default A (his "always work with multiples of 4", BD:812). B is `applyLegacyBpmKeys`'s other branch:
  the clip opens in Timeline -- a video at speed = videoBeats / beatDivision (what it played at, Renderer.cpp:1657-1665),
  a sequence at the speed that gives one cycle per `beatDivision` beats at 120.
- FM-8, before the question is put on his page: how many of his saved shows hold a BPM-synced clip (a read-only search of
  the show files for `"transportMode": 1`). None: question 73 is not asked and default A is built.
- A switch back to Timeline keeps the moved out point (one pair of points per clip; no second, hidden out point). It is
  difference D-13 and check 8's last line.

RA-15 RULED HERE, NOT ASKED (ST-6, ST-7, SC-4). "/2" on Bars is greyed when half is not a multiple of 4 (12, 20, 28):
exact or not offered; C5 and TL-U51 stand. Random's and BeatLoopr's numbers are fractions of a bar. The layer's X ends
its pause. The strip's fourth button stays (he asked for no removal, V20): one Speed step up in BPM Sync, double up to 4
in Timeline. D-5 and check 2 say that Arena shows a SECOND small menu there and ours leaves it out by his "42 default";
the visual-design seat is told so and judges it as a chosen gap. The plan's control rules stand (PL:241-245: every slider
a ResettableSlider with its default; any new popup through `showMenuAsync` with the top-level parent).

RA-16 THE NUDGE (V14). The lock reads `frameSnap_`. As the nudge lane is planned, the snapshot it publishes IS the
shifted beat, so no line of this lane changes when it merges (the plan's "that one function body changes", PL:424-425,
is struck). TR24 stays the proof and is run by whichever lane merges second.

RA-17 THE GATE'S SIZE STAYS (SC-6). Five critic seats (Harmony constraint). Every live row stays: for the rate, the fit,
Random and Eject the unit twin cannot link the renderer or MainComponent (V6), so the row is the only proof of the wiring.
The seats judge what no number decides; they do not re-read C1 .. C16.

## 4 FINAL STAGES + ORDER (replaces PL section 4; one builder context per stage; Harmony runs every live row and measurement)

ONE lane, based on main. Every stage: its cases written first and shown RED by id on the stage's base (a case that never
failed is struck and reported), then GREEN at its head. A builder never runs a live row and never gives a verdict.

| key | scope | owns | proves |
|---|---|---|---|
| S0 | re-base notes, no code | -- | the plan's S0 table (PL:617-623), plus: the call sites that will call `applyBarFit` / `refitBars` / `setBars` / `ejectDue` (count for TL-L7); the cases of tests/test_clip_transport_sync.cpp and tests/test_seq_vram.cpp that lean on `writeBack`'s range branch; the body of `/api/perf/record`'s `audioFile` path; what the capture does to a paused player; the order of `applyTempoRequest` and the regime flag in `runPipeline` (S4t); the recount of the `^TL-` pin |
| SM-a | the "1" on real music: a poller, no product code | .harmony/probe-bar-one.sh / .py + self-test | the self-test and its two mutated copies. THEN HARMONY: three tracks, N1, R1, X7; the tracker row is written |
| S4t | a Tap keeps the bar (RA-10) | src/analysis/BPMTracker.cpp, its test | TL-U70 (RED arm: main) |
| SR | the players own in..out (RA-4) | src/media/VideoPlayer.h/.cpp, ImageSequence.h/.cpp, SeqVram.h, src/render/ClipTransportSync.h (the range branch leaves; `playRange`), Renderer.cpp (two `setRange` calls; the two `reverse` guards go), tests | TL-U66, TL-U67, TL-U68; the re-aimed cases of S0's list; the three media probes whole |
| SM-b | the instrument (RA-1) | src/render/ClipBarGrid.h (new, whole), BarLockProbe.h (new), Renderer.h/.cpp (the probe branch), the two shown-frame accessors, ApiServer.cpp (four routes), MainComponent.cpp (their wiring), tests/test_clip_bar_grid.cpp, the lint, .harmony/probe-barlock.* | TL-U32, TL-U33, TL-U34, TL-U40 .. TL-U43, TL-U49a, TL-L6; the probe's self-test. THEN HARMONY: M1 .. M8, X2 .. X8, the player row |
| S1 | undo-live and the strip rule | as the chain's S1 | the chain's 27 S1 cases; TL-L2; G-U5 |
| S2 | fires and pause (RA-7) | model/Clip.h, Layer.h, TransportPause.h (new), render/ClipTransportSync.h, the players (`restart`), Renderer.cpp and the compositor call sites (the bool), binding/BindingManager.*, ui/LayerStrip.cpp + ClipInspector.cpp (the three buttons only), MainComponent.cpp, ApiServer.cpp (`paused`, the pause test route, `GET /api/debug/transport_buttons?layer=N`: the lit state of back / pause / play on both surfaces), tests | the chain's 15 S2 cases (TL-U2 as PL amends it) + TL-U60 .. TL-U64, TL-U69; TL-L1 (S2 form), TL-L1b, TL-L4; G-U4. Then Harmony: FM-1 |
| S2b | only if FM-1 is over its bar | as the chain | TL-U6b |
| S3 | drag, with the press rule | as the chain's S3 | TL-U10, TL-U13 (with its clause), TL-U14, TL-U15, TL-U15b, TL-U65; TL-L3, TL-L1 final |
| S3h | hold | as the chain | TL-U11 .. TL-U12c; MU-H1 .. MU-H3 |
| -- | MERGE 1 (default; H-T4) | Harmony | 5.2 on the lane head; probe-transport at `EXPECTED_ROWS=18`; the small visual gate VG-1 |
| S4a | bars engine, additive (RA-5, RA-6, RA-14) | model/Clip.h/.cpp, model/ClipBarFit.h (new), render/ClipTransportSync.h (`transportSpeed`), Renderer.cpp (the two branches), MainComponent.cpp (the calls), ApiServer.cpp (REST fields), tests | TL-U35a .. c, TL-U36, TL-U36b, TL-U37, TL-U38, TL-U39; TL-L7; TL-UC3b's clause |
| S4d | loop styles: Random in both modes, Play Once and Eject / Hold | model/Clip.h/.cpp, render/ClipTransportSync.h, ClipBarGrid.h (the lists), the Eject site, tests | TL-U71 .. TL-U74, TL-U79 |
| S4e | BeatLoopr | model/Clip.h (`beatLoop`), render/ClipTransportSync.h (`playRange`), ClipBarGrid.h (`catchUpTarget`, the list), tests | TL-U75 .. TL-U78 |
| S5a | the panel | ui/ClipInspector.h/.cpp, the two Clip-tab test routes, the two old fields and their keys leave | TL-U50, TL-U51, TL-U52, TL-U18d |
| S5b | the strip | ui/LayerStrip.h/.cpp, ui/DeckView.*, the strip dump route | TL-U53, TL-U54. Ends at the VISUAL WORK GATE (5.5), not at a commit |
| S4c | the lock, as the two verdict rows say (any time after S4a once they are written) | render/ClipBarGrid.h, ClipTransportSync.h, Renderer.cpp (the probe's inputs become the model's; `showBarBeats()`), tests | TL-U44 .. TL-U49 (TL-U46, TL-U47 as RA-4), TL-U36's clause, TL-L5; probe-video whole. Then Harmony: M2 (one tempo) and M4 again on the lane build, same bars |
| S6 | probe and docs | .harmony/probe-transport.* (30 rows), docs as PL:665-672 plus the range rule and RA-10 | a full local run and the RED table |
| -- | MERGE 2 | Harmony | everything of section 5 |

What Harmony runs herself, and when: SM-a right after S0 (no builder is needed for the run); FM-2 on the frozen app before
S2; M1 .. M8 after SM-b (S1 .. S3h are built meanwhile); FM-1 after S2; FM-7 and FM-8 before S4a; merge 1's gate; the
visual gate after S5b; M2 / M4 again after S4c; every row of 5.2 .. 5.5; six mutants of her choice (always MU-4, MU-18,
MU-21, MU-48, MU-71, MU-89). S1m is not built.

## 5 TESTS + GATE ROWS (pre-registered here; the plan's section 5 stands except where this section speaks)

Harmony constraint (unchanged): RED arm = the frozen pre-lane main app; a flake verdict needs >= 5 runs per arm; a bar is
met or reported; a bar inside 4 x the measured noise is BLOCKED until she waives it in writing.

### 5.1 Unit cases added or re-stated (each a TEST_CASE whose name begins with its id; RED arm: the stage's base, by id)
- TL-U33, TL-U43, TL-U46, TL-U47, TL-U49a, TL-U70: as RA-6, below, RA-4, RA-4, RA-3, RA-10.
- TL-U43 "a clip started 1.99 beats off the bar is within 0.10 beat from 32 beats on and within 0.03 beat (+ 1e-6) from 35
  beats on; 1 beat off: within 0.03 from 18 beats on; its position never steps back; the speed stays within 6 % of the rate".
- TL-U66 (SR) "inside in 0.2 / out 0.7 on the REAL VideoPlayer and the REAL ImageSequence: a reversed Loop never leaves the
  range and wraps from in to out keeping the overshoot; a forward Loop wraps from out to in keeping it; a PingPong turns
  at out and at in with no position step; a PingPong reversed in mid-leg turns round at once and still turns at both
  ends; a OneShot stops on the out point's frame, reversed on the in point's; a range
  narrower than one frame holds its first frame". RED arm: the stage's base with `setRange` as an empty stub (the
  reversed read goes below 0.2).
- TL-U67 (SR) "SeqVram: a ranged PingPong reflects at the out frame and at the in frame; a ranged reverse Loop wraps to
  the out frame; the distances follow". RED arm: main.
- TL-U68 (SR) "a player whose intent is not running is never moved by the sync: ten syncs with the playhead beyond a
  moved out point leave it there". RED arm: the stage's base (the player is sought to the in point).
- TL-U62 (S2) gains "; a Loop clip paused beyond a moved out point keeps its playhead through ten syncs".
- TL-U73 (S4d) reads "Random in BPM Sync: a jump falls on the music's grid of the Interval and moves a whole number of
  beats, never 0, never more than the Distance; the error against the beat is the same before and after; a realign back
  across a grid line does not jump twice".
- TL-U79 (S4d) "a paused, a held and a Speed-0 clip: no Random jump, no BeatLoopr wrap, no Catch Up seek".
- TL-U69 (S2) "strip and Clip tab: pause is lit while the layer is paused, play when the clip's direction is forward,
  back when it is backward (the model's `reverse`, never the ping-pong leg); each is set only when it changes" (TL-U54
  keeps the S fader and the forward button).
- TL-L7 (S4a) as RA-5; RED arm: S4a's base, where `syncMedia` computes `videoBeats / beatDivision` itself. TL-L6 as RA-1.
- TL-U36, TL-U37, TL-U71, TL-U35a .. c: the plan's wording, called on the functions of RA-5.
Counts: the plan's 97 cases + TL-U49a, TL-U66, TL-U67, TL-U68, TL-U69, TL-U79 = 103; lints 8 (TL-L1, L1b, L2 .. L7).
New mutants (each alone on the lane head turns the named case RED): MU-89 the players ignore the range -> TL-U66 (TR31).
MU-90 a ping-pong turn drops the overshoot -> TL-U66. MU-91 SeqVram's ranged ping-pong goes to the in frame -> TL-U67.
MU-92 a restart is not applied while paused -> TL-U61 (TR25 b, the picture). MU-93 a Random jump on a paused clip ->
TL-U79. MU-94 the anchor follows at once -> TL-U49a. MU-95 the tolerance is a fixed 0.05 s -> TL-U33. MU-80's teeth are
TL-U71 only. MU-10's live arm is TR7c (b), (c); MU-63's is TR7c (a). MU-96 the mode switch pushes an Undo step that
puts the out point back -> TL-UC14 (TR26's undo clause). MU-97 Eject never clears the layer -> TL-U71 (TR30). MU-98 the
buttons' lit state
reads `Clip::playing` -> TL-U69 (VG-1, P2).

### 5.2 Unit gates
- G-U1 `ctest --test-dir build -R '^TL-' --no-tests=error --output-on-failure` -> `100% tests passed, 0 tests failed out
  of N`, N >= 111 (112 with S2b; one more with O4's TL-U49j), each id Passed. Harmony constraint: N is S0's recount of the
  list, never lower than 111. At merge 1 the pin is the count of the stages built by then, stated in the lane report
  before the gate runs.
- G-U1b, G-U2, G-U3 (the standing mutants + MU-60 .. MU-98), G-U4, G-U5, G-N1, G-P1: as the plan.

### 5.3 Live rows -- `.harmony/probe-transport.sh`; final line `PROBE-TRANSPORT GREEN`; pins `EXPECTED_ROWS=18` (merge 1), `=30` (merge 2)
Launch rules, fixtures, tolerance, driver and flake rule: the chain's and the plan's. "Frame code" = the frame number
decoded from the pixels of the app's own capture, as TR2 .. TR4 do (the ramp: 30 fps, 300 frames). The plan's rows TR9,
TR10, TR13b, TR20, TR22, TR23 stand as the plan wrote them. Changed or new:
- TR7c scrub_outside_and_past_the_markers (AMENDED; Loop ramp, in 0.2, out 0.8). (a) OUTSIDE: wait for a playhead read
  in [0.4, 0.6]; strip down 0.95, up; then down 0.05, up; then the same two presses on the Clip tab's bar: in the 0.5 s
  after each, `playing` stays true, every read is in [0.2, 0.8], and no step between two reads differs from rate x dt by
  more than 0.02. (b) PAST OUT: strip down 0.6, drag 0.95, up: within 0.5 s a read in [0.2, 0.3] and no read above
  0.8 + 1/300. OneShot variant: within 0.5 s `playing` false and the playhead <= 0.8 + 0.001. (c) BELOW IN: strip down
  0.4, drag 0.05, up: no read below 0.2 - 1/300. GUARD for the RED arm (routes new). Teeth: MU-63 turns (a) RED (a jump);
  MU-10 turns (b)'s OneShot variant and (c) RED.
- TR21 (b) reads: the driver adds 1.8 beats to its bar position; from 34 beats later |e| <= 0.10. The rest as the plan.
- TR25 paused_fire. (a) fire a ramp; 1 s; pause the layer; read p0 and a frame code f0; fire the same clip; at 0.5 s the
  playhead is within one frame of p0, `paused` true, the frame code within 1 of f0; a second capture 0.5 s later has the
  same frame code. (b) fire another ramp of that row (pixels tell the two apart): two reads 0.3 s apart are equal and
  within one frame of its in point; two captures 0.5 s apart show its first frame (code within 1 of in x 300), equal.
  (c) play: two reads 0.3 s apart differ by >= 0.01 and two captures 0.5 s apart differ by >= 5 frames. (d) a second
  layer plays; pause layer 0; `trigger_column`: layer 0's `paused` stays true and its two reads 0.3 s apart are equal;
  the other layer restarts (its read is within 0.06 of its in point). GUARD (the field is new); RED arm of (a): the
  frozen app, reported. Teeth: MU-60 (b, d), MU-61 (a), MU-92 (b: the picture is not the first frame).
- TR26 fit_on_switch: the plan's row, then `POST /api/debug/undo`: `clipBars` and `outPoint` unchanged; to Timeline:
  `outPoint` still in [0.799, 0.801]. Teeth: MU-64, MU-65, MU-96 (the undo clause).
- TR27 speed_steps: the plan's row; its 0 step reads: two reads 0.5 s apart equal, `playing` true, two captures 0.5 s
  apart with equal frame codes that are within 1 of playhead x 300; then step to 1: a read 0.5 s later has moved. Added
  arm, the same steps 2 and 1/2 with the clip reversed: the same rates by size and no read outside in..out. Teeth: MU-67.
- TR28 random_jumps: Timeline, Random, Interval 1 s, Distance 2 s, in 0.1, out 0.9, the generator seeded through the
  "do" route so that every jump is at least 0.05; 10 s of reads at 20 Hz. A jump = a step that differs from rate x dt by
  more than 0.02. Bars: at least 7 jumps; every read in [0.1, 0.9]; with the layer paused for 3 s: no jump. Teeth:
  MU-81, MU-93.
- TR28b (INFO, outside the pin) random_short_interval: BPM Sync, driver at 174, Random, Interval 1/16, the long-GOP
  fixture, 20 s: printed as MEASURE -- the share of intervals in which the SHOWN frame changed, and the longest run of
  held frames. One bar only: after Random is switched off a read and a capture 0.5 s later show the clip moving.
- TR29 beatloopr: driver at 60, locked; press option "1"; p = the loop's start as the "do" route's reply states it (not an
  earlier read). Every read of the next 3 bars is in [p, p + one bar's width + one frame + rate x the measured REST
  latency]; three captures in that time have frame codes inside the same span. Off with Catch Up: exactly one step (a
  step = more than 3 frames beyond rate x dt), then |e| <= 0.10 within 2 beats. Again without Catch Up: no step at Off.
  The REST latency and its noise are measured over 20 calls before the row is judged. GUARD. Teeth: MU-83.
- TR30 play_once_and_eject: out 0.3; fire: within 4 s the layer's `activeClip` is empty. Teeth: MU-97. Its paused arm
  ("still there after 4 s") is a GUARD with no teeth: a paused clip never ends.
- TR31 reverse_and_ping_pong_inside_the_range (NEW; composition files carry `reverse` and `loopMode`, V5). The 10 s
  ramp, Timeline, in 0.2, out 0.7. (a) reversed Loop: 40 reads over 4 s: every read in [0.2 - 1/300, 0.7]; the reads
  fall except across one wrap, after which the read is >= 0.65; three captures 0.3 s apart with no wrap between have
  falling frame codes inside [60, 210]. (b) Ping Pong: 12 s of reads: every read in [0.2 - 1/300, 0.7 + 1/300]; a
  maximum >= 0.69 and a minimum <= 0.21 are both seen; no step between two reads beyond 2 x rate x dt + 1/300.
  (c, merge 2 only) the fitted ramp in BPM Sync at driver 60, reversed: no read above 0.8; with S4c in the build,
  from 36 beats on five reads 0.5 s apart have |e| <= 0.10. RED arm (frozen app, the same composition files): (a) reads
  below 0.2; (b) a step of about 0.5 at the out point and no falling leg. Teeth: MU-89.
- (outside the pin) TR24 lock_follows_the_nudge: as the plan; TR19 INFO: as the chain.

### 5.4 Re-runs and idle paint: as the plan.

### 5.5 VISUAL WORK GATES (a capture builder, then five critic seats, then Harmony; Boris sees nothing before a gate passes)
- VG-1, before merge 1. States (PNG + dump, both window sizes): P1 a playing layer; P2 the same layer paused (Clip tab
  and strip); P3 playing backwards (both). Bars, read from `GET /api/debug/transport_buttons` (S2) and
  `/api/composition`: in P2 pause is lit, play is not and `paused` is true; in P3 back is lit; in P1 play is lit and
  pause is not; all on both surfaces; no widget of the section
  moved against the pre-lane capture (+/- 1 px). GUARDS (the route is new); teeth: MU-98 turns P2's bar RED. The seats
  get the three states, his two pause sentences and the list of what is in the section before the lane.
- VG-2, after S5b: the plan's states and bars C1 .. C16. States: W1 Timeline, default; W2 the readout showing the time left;
  W3 the mode menu open; W4 BPM Sync just fitted (10 s: Bars 4, out 0.8, 3 lines); W5 12 bars ("/2" greyed); W6 8 bars; W7
  the 2 s clip; W8 Speed 1/4; W9 Speed 16; W10 Speed 0; W11 length not known; W12 the out point pulled in on W6; W13 the
  narrowest inspector; W14 an image clip; W15 Random in Timeline; W16 Random in BPM Sync; W17 BeatLoopr on 1/2 with Catch
  Up; W18 a paused layer; W19 playing backwards; W20 the strip in BPM Sync; W21 the strip in Timeline; W22 the 128 s clip
  (64 bars) on both surfaces. Changes: C9 RE-STATED -- "the manifest holds, per state, the whitelist
  of every caption, label, tooltip and painted string of the section; a string outside its state's whitelist is RED; the
  union of the whitelists is pinned in TL-U50; across every 'do' action no string appears that is in no whitelist".
  C5 stands ("/2" greyed in W4 and W5). The packet gains: V16's fact (Arena's second small menu, left out by his "42
  default"), D-1 .. D-15, and RQ-1 / RQ-2's screenshots if they have arrived (the visual-design seat then compares W15 ..
  W17 with them).

### 5.6 Facts Harmony must measure (none can be settled by reading)
N1, R1, X7 (SM-a). X2 .. X6, X8 (SM-b), with RA-2's table. The noise of e (5 locked runs) and of the REST latency (20
calls). FM-1, FM-2 (the chain's). FM-7 (durations of real files, RA-6). FM-8 (BPM-synced clips in his saved shows,
RA-14). After S4c: M2 at one tempo and M4 again on the lane build, same bars; not met: STOP and report.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

WHERE HIS RULING DIFFERS FROM RESOLUME -- each is built HIS way and shown to him:
- D-1 The second row reads "Bars", not "Beats"; one bar = 4 beats.
- D-2 Bar counts are 4, 8, 12, 16 ...; Resolume picks 1, 2, 4, 8, 16 ... BEATS.
- D-3 On the switch to BPM Sync the app MOVES THE OUT POINT IN to the last whole group of 4 bars; Resolume keeps the whole
  clip and changes its speed.
- D-4 A clip shorter than 4 bars is fitted whole into 4 bars and plays slow; Speed makes it fast.
- D-5 Arena has TWO small menus right of the buttons; ours has one. The second (from the start / carry on / relative) is
  left out: every fire starts at the beginning.
- D-6 A clip that is off the bar SLIDES back by itself -- after a drop, a Resync, a Tap, and after EVERY fire that is not
  on the "1": up to 34 beats (17 s at 120, 23 s at 90, 34 s at 60), up to 6 % fast or slow meanwhile. He was told "up to
  about 15 seconds". Resolume leaves it off the bar until it is fired again. (Question 71.)
- D-7 Pause belongs to the layer: another clip fired on a paused layer waits on its first frame. In Resolume pause is the
  clip's own. (Question 72.)
- D-8 The bar shows one line per BAR; Resolume's ticks are per beat (INFERRED from his screenshot 2).
- D-9 A file within two frames of a whole group of 4 bars keeps its end and runs up to about 1 % slow instead of losing
  4 bars.
- D-10 Random's and BeatLoopr's numbers are fractions of a BAR; Arena's are beats, so the same numeral lasts four times
  as long here.
- D-11 "/2" on Bars is grey at 12, 20, 28 (half would not be a multiple of 4); in Arena "/2" always acts.
- D-12 Play Once and Eject empties the layer at the end (our reading of "eject").
- D-13 After a switch back to Timeline the out pointer stays where the fit put it.
- D-14 A Resync does not restart synced clips; they slide. Resolume's Resync jumps them to the first beat.
- D-15 Ours, not from Resolume: a press inside the bar moves the playhead there; Timeline Speed stops at 4 (question 74).
CHECKS. The chain's checks stand as PL:919 lists them. The plan's 1 .. 21 stand with these changes:
- 2 gains: "one small menu where Arena has two".
- 4, 6, 7, 12's "stands still", 20: the machine has proven the numbers and the picture (TR25, TR26, TR27). He is SHOWN them;
  wrong = it does not feel like "stays, paused" to him. 6 also shows: clear the paused layer with X, fire a clip -> it plays.
- 5 reads: Fire a BPM-synced clip in the middle of a bar with Snap off. -> It starts at its beginning and then slides
  onto the bar like a dropped clip. With Snap on Bar it starts on the "1" and nothing slides. Wrong: a jump.
- 8 gains a second clip (one that is a little short of a whole group, FM-7) and a last line: switch back to Timeline ->
  the out pointer stays moved; drag it to get the rest back.
- 10 reads: ... "/2" is grey at 4 and at 12. Say if it should go to the nearest allowed number instead.
- 14 reads: Drop the playhead in mid-bar; also press Resync in mid-bar. -> No jump; the lines are back on the "1" within
  about 17 seconds at 120 (longer at slow tempos: half a minute at 60). Wrong: a jump, it never comes back, or the wait is
  too long for you (then question 71's B is the other way).
- NEW 22. Set a clip with its out pointer pulled in to play backwards, then to Ping Pong. -> Backwards it stays between
  the two pointers and loops there; Ping Pong turns at each pointer. Wrong: it shows video outside the pointers, or it
  never turns. On a Ping Pong clip, play backwards turns it round at once.
- NEW 23. Tap the tempo eight times while a synced clip plays. -> The circle keeps counting 1-2-3-4 through your taps,
  no beat shown twice, and the clip's lines stay with the circle's "1". Wrong: the circle repeats a beat while you tap.

## 7 BORIS QUESTIONS (71 .. 74; each has a default A; 75 .. 79 are not used; nothing waits)

71. Question 33 told you a dropped clip slides back onto the bar in "up to about 15 seconds". Worked out properly it is
    longer: at most 17 seconds at 120 BPM, 23 at 90, 34 at 60 (about half of that on average), with the picture up to 6 %
    fast or slow while it slides. It also happens after every fire that is not on the "1" (with Snap on Bar a fire lands
    on the "1" and nothing slides).
    A (default: your answer 33 B) Keep the slide.
    B No slide: the clip plays on from where it is and cuts once, on the next "1", into time.
72. A layer is paused on a clip. You fire a DIFFERENT clip on that layer.
    A (default: what I told you I understood) The new clip shows its first frame and waits, paused, until you press play.
    B The new clip plays. The pause is over.
    C The new clip plays. The first clip keeps its pause: fire it again later and it is still paused on the frame where
      you left it. (This is what Resolume does.)
73. Your saved shows that have BPM-synced clips:
    A (default) When you open one, each such clip gets its Bars number and its end moves in to whole groups of 4 bars.
      The saved file changes only when you save.
    B They open as plain speed clips (a video looks as it did; an image sequence keeps the speed it had at 120 BPM);
      you switch the ones you want to BPM Sync yourself.
74. Speed in Timeline mode goes from 0 to 4 today. Resolume's goes to about 10.
    A (default) Keep 0 to 4.
    B Go to 10.
REQUESTS (not questions; a minute each in Arena; nothing waits on them):
RQ-1 One screenshot of the BeatLoopr row (a clip in BPM Sync), so its buttons can be copied.
RQ-2 One screenshot of a clip in BPM Sync with the loop menu on Random, showing Interval and Distance and the values
     their minus / plus step through.
RQ-3 (as the plan) click plus once on Speed and on Duration and send the numbers; Speed 2 in Timeline and the Duration it
     shows; a clip longer than a minute with the time shown both ways.

## 8 HARMONY'S DECISIONS (each with a default; the builder builds the default unless told otherwise)

- H-T1 S4t's owner. Default: built in this lane, before SM-b (RA-10). If the nudge RULING comes to claim BPMTracker.cpp,
  whichever lane builds it registers TL-U70 once and M5 is run on a tree that has it.
- H-T2 The three tracks of SM-a / M3. Default: she names them in the gate script before the run. None on the rig:
  BLOCKED, and Boris is asked for three tracks -- no tracker row without them.
- H-T3 W3 (the open mode menu) is DUMP-ONLY if S0 finds the capture cannot show a popup; it is then check 1.
- H-T4 Merges. Default: two (RA-11), with VG-1 before the first. One merge is the other way: VG-1 is then folded into VG-2.
- H-T5 The production-mode instance of SM-a, M2, M3, M5 runs on Boris's machine with his real settings file (test mode
  alone uses a scratch one, MainComponent.cpp:80-98). Default: exactly as .harmony/probe-manual-bpm.sh does it -- a
  normal cmake build (never a copied bundle), the live lock, never while his app runs, `open -g`, a file source (no
  microphone), no Output window, own-pid quit, the takes it made deleted. If S0 finds any step of that recipe touches a
  setting of his: STOP before the first run.
- H-T6 FM-7's files. Default: the fixtures and at least 20 clips from his own media folders, read-only (`ffprobe` or the
  app's own duration). FM-8: a read-only search of his show files.
- H-T7 Question 72 is on his page before S2 starts. Default if he has not answered by then: A is built (RA-7 keeps B and
  C small).
- H-T8 If SM-a gives O3 or O7: the STOP concerns S4c only. Default: SM-b is still built and run (its player bars are
  needed under every fallback), M3 then runs on the anchored reader for the record, and the lane goes on to merge 1.
- H-T9 The Pitfall numbers (the plan's three, plus "the players own in..out: one boundary rule per player, both
  directions; `writeBack` never seeks").

## 9 SIDE FINDINGS

- SF-1 The plan's first stage could not have run (V9). Any other plan that measures the tracker "in a test-mode
  instance" has the same hole; the nudge plan says it right (plan-nudge.md:253-255).
- SF-2 V12 is not this lane's alone: every reader of `beatInBar + beatPhase` or `barPhase` in Auto sees the one-beat
  flip between the count and the detection (shader uniforms, mappings, and the Bar / 2 Bar / 4 Bar release, which tests
  `beatInBar == 0` on a beat crossing, Layer.h:472-479). INFERRED, not run. SM-a's R1 is the first number for it. A fix
  belongs in the tracker (advance the bar's beat with the count), in its own plan; this lane reads around it (RA-3).
- SF-3 `/api/bpm` does not report `trackerState` (V8). One field would let any probe condition on LOCKED. Not built here.
- SF-4 No REST route leaves Manual (V9). SM-b's tempo route has `auto`; it is test-only.
- SF-5 The plan says "D2-1 .. D2-14" and defines eleven (V24); it writes ">>" for a button captioned ">|" (V20); it
  lists 18 drops and judges 22 (RA-1).
- SF-6 The dispatch lists questions 47 .. 50 as open; all four are answered (V23).
- SF-7 The tap code exists twice and taps 9 and later re-use the first eight times (facts-beat-controls.md section 3
  item 4). The nudge lane's or the top bar's; M5 sends its taps through the route, with the tempo computed by the probe.
- SF-8 Random at a short Interval on a long-GOP video cannot show a new picture per jump (86 ms at 174 BPM against a
  keyframe seek). TR28b reports it; the manual should say which files suit Random.
- SF-9 V26 is a defect of main today (a Ping Pong clip can stick on one end; play backwards does nothing on it). SR fixes
  it with the range; TL-U66's RED arm on main shows whether the inference holds.
- NOT IN THIS LANE: PL section 9 stands. Added: a tracker fix for V12 (SF-2); `trackerState` on `/api/bpm` (SF-3); a
  restore of the out point on a switch back to Timeline (RA-14); a per-clip held frame beside the layer's pause (ST-1);
  the jump as a user setting (VI-2); Timeline Speed above 4 (question 74); removing the strip's fourth button; a Resync
  that restarts synced clips (D-14). The stopped sync-dial branches: nothing is carried (PL section 9, H-17); I did not
  read them for code.

## 10 RISKS (the strongest counterargument first)

- K-1 THE STRONGEST, against this ruling: RA-4 is bigger than the seat asked for. It moves the range into the two
  players' clocks -- the code Pitfalls 56, 60, 62 and 64 guard -- in a lane that already has sixteen stages, when two
  more branches in `writeBack` would have made reverse stay inside the range. Why it still stands: the two-branch fix
  leaves every wrap and every turn a frame late with its overshoot dropped (V3), and his fit turns that from a rare case
  into every clip over 8 s, wrapped once a beat at x16; the lock would spend its 6 % making up frames the wrap threw
  away, and X6 could fail for that reason alone. The players already hold this exact boundary code for the file's ends
  (V2): the change is which two numbers it uses. It is gated by the three media probes whole, three real-player cases
  and a live row with a RED arm on the frozen app, and the smaller fix is the named fallback. Cheapest refuting test:
  TL-U66 on the real VideoPlayer, then probe-video.
- K-2 The lock follows whatever the app calls "1". In Auto that is a beat detector's output and V12 shows it is not one
  clean event. Resolume's way (stay where dropped) can never go wrong like that. Why the lock stays: it is his answer,
  twice (BD:763, BD:817), and nothing is promised before SM-a's three tracks; O2, O3, O7, O8 are the ways out. Cheapest
  test: SM-a on one track, ten minutes.
- K-3 SM-a and three of SM-b's runs are production-mode launches on the machine Boris works on, with his settings file
  (H-T5). The recipe is the rig's own and has run before; it is still the one place this lane touches his state.
- K-4 Question 72's C moves the pause from the layer to the clip after S2 is planned. RA-7 keeps it to three functions
  and one field, but TL-U60 .. TL-U64 and TR25 are then re-registered: an answer after S2 is built costs a fix round.
- K-5 Question 71's B replaces the slide with the jump, whose own fact (X8) is measured only on ten seeks. If he picks
  B and X8 fails, row O5 applies: no lock until the player's seek is fixed.
- K-6 Two merges mean two gates and a small visual gate for three states (H-T4). One merge is cheaper to gate and
  dearer to rebase under the nudge and outputs lanes (PL K-2).
- K-7 The tolerance is still a number chosen by reading until FM-7 is run; D-9's "about 1 %" moves with it.
- K-8 The fit trims the clips of his saved shows on open under default A (question 73). FM-8 says how many; check 21
  shows him one before the merge.
- K-9 The anchored reader (O2) is state on the render thread, the one thing the chain's lock avoided. It is one record
  for the show, integrates the beat time (Pitfall 42) and is pinned by TL-U49a; a Resync cuts through it at once.
- K-10 S4t changes the bar's beat after a Tap for EVERY reader (the wheel, Bar-quantised fires, phrases), not only for
  clips. That is the intent (they all lose the beat today, V13), but it is an analysis-thread change riding in a
  transport lane: TL-U70 and the whole tracker binaries are its gate, and check 23 shows him.
- K-11 The chain's risks about Undo (K1), the stale frame (AM-21) and the plan's K-5 .. K-8, K-10, K-11 stand unchanged;
  its K-9 (a clip played stretched until its fit is written) is removed by RA-5.

STATUS: DONE
