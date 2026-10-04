# ruling-transport-answers -- architect ruling on the blind council's attacks on plan-transport-answers.md (lane "transport-answers", s-rta-1004)

Role: architect, ruling (opus, max effort; Fable is out of usage). Read-only. Nothing built, nothing run, no app or probe
launched. Pin: main 185147b (`rev-parse --short HEAD` printed 185147b; `status --short -- src tests docs CMakeLists.txt`
printed nothing), so every src / tests / docs line below is a plain-file read at the pin. Worktrees bf2 (740b6d6) and
bf2keys (9eab9bd): heads checked, clean, not read for code (nothing of them is a part of this lane).
Papers (2 of 2 seats, 16 attacks, 18130 characters, array closed, parsed whole):
.harmony/.reports/s-rta-1004/attack-transport-answers-papers.md
Short names: PA:n = plan-transport-answers.md line n. RU:n = ruling-transport-delta2.md line n. PL:n =
plan-transport-delta2.md line n. RT:n = .harmony/.reports/s-rta-1003b/ruling-transport.md line n. BD:n =
.harmony/binding-decisions.md line n. BL:n = .harmony/boris-feedback-backlog.md line n. RN:n = ruling-nudge.md line n.
NR:n = plan-nudge-row.md line n (a PLAN under attack, not a ruling; said wherever it is leaned on).
Labels: VERIFIED = read by me in the named file; COMPUTED = arithmetic on verified numbers; INFERRED = reasoned from
verified lines, not run; ASSUMED = neither.
Boris is quoted only verbatim and only from BD / BL. Amendments here are TB-1 .. TB-14; each OVERRIDES the plan body.
Precedence after adoption: Boris's words > the two adoption blocks (PL:1039-1089) > this ruling > plan-transport-answers.md
> ruling-transport-delta2.md > plan-transport-delta2.md > the chain. EVERYTHING THE PLAN SAYS THAT THIS FILE DOES NOT NAME
STANDS AS THE PLAN WROTE IT, and everything of the ruling that neither file names stands as the ruling wrote it.

## 0 VERDICT

THE RULES, FIRST.

1. "IN TIME", WITH NO SLIDE. A BPM-synced clip is always pushed EXACTLY the tempo's rate. No speed law of any length is
   built: the plan's "held" band (PA:125-128) is STRUCK from the build.
   - In time = its bar lines are within a tenth of a beat of the music's "1" (0.10 beat: 50 ms at 120). Nothing is done.
   - Out of time = further off than that. The clip plays on at exactly the rate and is CUT ONCE, on the frame the
     music's next "1" is crossed, to the nearest place that is in time (never more than half a bar of the clip away).
   - HIS HAND: on the first "1" after he moves the beat or the tempo himself (a nudge, a Tap, a Resync, a tempo value,
     BPM minus / plus, /2, x2, the beat timer's start), every synced clip that is more than one picture frame off (16 ms
     of music) is cut onto the moved beat. A Resync is itself a "1": the cut is at the press (question 121).
   - HOW OFTEN IT CAN CUT: never more than once per "1" per clip (once per bar: every 2 s at 120), and only when one
     of these happened: a fire that is not on the "1"; a playhead drop; play after his pause; his hand on the beat (above);
     BeatLoopr switched off without Catch Up, a Speed stepped up from 0, a changed Bars number or out point; the app
     moving its own "1" in Auto; drift (the tempo the app reads and its beat coming a tenth of a beat apart). The last
     two are the ones nobody can know by reading: SM-a measures them on three of his tracks BEFORE any product code
     (N1, NC), each with the bar "at most 1 in ten minutes on each track", and over the bar is STOP AND ASK.
   - WHAT HE SEES WHEN THE APP'S "1" MOVES IN AUTO: the circle in the top bar shows the new "1" at once; a synced clip
     does nothing for 8 beats (it waits to see whether the "1" stays moved); then, on the next "1", every synced clip
     jumps once, by one or two beats, and is in time again. If the "1" moves back inside those 8 beats, nothing happens.
2. THE PAUSE. It lives on the CLIP: `Clip::paused` and the frame it is paused on, `Clip::pausedAt`. Both are written to
   the show file only while the clip is paused, so an old show opens with nothing paused and a show with nothing paused
   is byte for byte what it was.
   - What writes it: his pause and play buttons (the strip's act on the clip the layer plays, the Clip tab's on the clip
     it shows), a key or a pad bound to them, the top bar's Play and Pause for as long as they exist -- and a take or a
     routine putting back and replaying the play / pause it RECORDED, exactly as it does today (finding 1 below).
   - What never writes it: a fire, a column fire, the layer's X, Eject, autopilot, Cmd+Z, Redo, the beat timer.
   - What the buttons show: the strip shows the layer's playing clip (pause lit while it is paused; play lit when it is
     not and runs forward; back lit when it is not and runs backward); the Clip tab shows the same for the clip it
     shows, and its pause is never greyed; every paused clip carries a drawn pause mark in its grid cell.
3. SPEED TO 10. `Clip::speed` in Timeline runs 0 .. `kTimelineSpeedMax` (10). The slider's law is two straight pieces:
   the lower half of the travel is 0 .. 2 (1 at a quarter, exactly where today's fader has it), the upper half 2 .. the
   top. What the decoder is asked: at 10 the clock passes ten content frames in every rendered frame -- 300 frames a
   second of a 30 fps file -- forward, reversed and in ping-pong. No reading settles whether it can: X6t measures it in
   three arms at 10 and at 8 before the top is promised, the top is DATA (10, 8 or 4), and Speed times the master
   speed never exceeds 16, the highest it can reach today (4 x 4).

THE VERDICT. The plan NEEDS REVISION before a builder starts. Its pause design (TA2) and its speed law (TA3) stand with
corrections; its TA1 does not: it puts a speed correction back after he answered "71 b". 16 attacks ruled: 11 ACCEPT,
5 PARTIAL, 0 REJECT. 14 amendments.
- BOTH SEATS' STRONGEST POINTS ARE UPHELD. ST-1 / GA-7: the adoption says "NO SLIDE IS BUILT" and names the one-cut path
  "as THE design" (PL:1079-1080); that path is "no trim; ... ONE seek ... then the steady rate" (PL:442-444). The hold
  is the plan's own addition, justified by a number nobody has measured (F7). It is not built. Whether anything is
  needed between cuts is now a measured row (O3d), and the hold is its NAMED FALLBACK, built only on his word (TB-3).
  GA-1: the Undo arm of TR25 and his own check 26 could not fail (F12); both are re-ordered, in both directions (TB-9).
- THE TWO SEATS PULL APART ON ONE POINT AND ARE RECONCILED. ST-1 / GA-7 take the hold out; ST-7 wants a clip to follow a
  small nudge more finely than the hold did. Without the hold a running clip would ignore every nudge under a tenth of
  a beat (50 ms at 120, 100 ms at 60), against "I mean everything" (BD:860). The hand rule (rule 1, TB-2) answers both
  in his own words for an out-of-time clip: one cut, on the next "1".
- FINDING 1, NO SEAT'S (F14). A take and a routine RECORD AND RESTORE a clip's play / pause today (the "playing" lane,
  MainComponent.cpp:2004-2011 and :6541-6567; checkpoint 0 and a routine's restore, docs/claude/recording.md:11 and
  :46-47; his rulings BD:495 "yes" and BD:435 "restore"). The plan's "a take neither records nor restores them"
  (PA:339-342) would silently drop that. Ruled: they keep doing it; the thing they write becomes the clip's pause (TB-8).
- FINDING 2, NO SEAT'S (F10). The plan builds `beatRunning` in S4c. If no row allows S4c, "nothing moves" (BD:919) would
  have no home. It moves to S4a's speed function, and the line that fills it is pinned by a lint that arms itself when
  the nudge lane's files arrive (TB-6). The sentence "tempo 0 ... already stands the clip still" (PA:179-180) is struck:
  the nudge-row plan publishes a state, not a tempo of 0 (NR:211-213), and at the pin a tempo of 0 leaves the last speed.
- HIS RELIABILITY QUESTION ("b can you program this reliably or should we change the plan?", BD:817) is still answered
  by measurement, and SM-a is still the first stage. Without a slide the question is "how often does the picture cut":
  N1 (the app's "1" moves) keeps its bar and its STOP-AND-ASK; NC (every cut a clip would have shown) is new, with the
  same bar. Section 3, TB-3 and TB-4 say which parts and bars of the instrument are still needed and which are void.
- Nothing here loosens a pre-registered bar. No new mutex. Nothing touches the audio callback or the analysis thread
  beyond the ruling's own S4t. No on-screen text is added: the pause mark, a lit button, a greyed "plus" are states.
- OUT OF ORDER IN THE DISPATCH: it lists questions 47 .. 50 as open; all four are answered (F19). Nothing here leans
  on them.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on)

- F1 VERIFIED the pin and both worktree heads (header).
- F2 VERIFIED BD:902 "71 b"; BD:904-905 "new clip plays, the old clip is permanently paused and if comp is saved, it is
  saved as paused. the clip is now paused until the user changes that setting."; BD:907 "74 b". The questions as asked:
  boris-clarify-71-74.md:7-22 (71 B: "No slide: the clip plays on from where it is and cuts once, on the next "1", into
  time.").
- F3 VERIFIED BD:763 "9 b". The question as asked is on record at BL:411 ("... B The app keeps nudging it back onto the
  beat by itself."); those words are Harmony's question text, his word is "9 b".
- F4 VERIFIED PL:1079-1082 (adoption): "NO SLIDE IS BUILT. Stage S4c builds the ruling's one-cut path ... as THE design,
  not as a fallback. The 6 % slide law, its pull-in bar X4 and the "eases onto the beat" text are void." PL:441-445 (the
  path it names): "The jump: no trim; on the music's next "1" ... ONE seek ...; then the steady rate." The adopted
  design has no speed correction. The HELD band of PA:125-128 is an addition to it.
- F5 COMPUTED. One render frame in beats = tempo / 3600: 0.0167 at 60, 0.0333 at 120, 0.0556 at 200. The plan's factor
  (1 - e / frame, clamped 0 .. 2) is at its clamp from one frame of error up. So a clip that a STEP of the beat leaves
  0.10 beat ahead stands for 3 frames at 120 (50 ms) and 6 frames at 60 (100 ms); behind, it runs double for as long.
  A clip that DRIFTS into the band is corrected by one repeated or skipped frame (0.051 -> 0.018 at 120). The seats'
  "freeze for about 3 frames" is right for a step (a Tap, a nudge, a realign), not for drift.
- F6 VERIFIED src/media/VideoPlayer.cpp:354-359 (`seekTo` = two atomic stores), :366-386 (the frame that consumes it
  sets the clock, `jumped`, and `gen_.fetch_add(1)`): EVERY seek, however short, is a generation bump the decode thread
  must serve. :456-469: a speed of 0 "keeps the direction"; only a change of sign is a discontinuity. docs/claude/
  pitfalls.md:135 (Pitfall 62, rule 6): forward play takes cache hits "after ... a seek onto a resident frame".
  INFERRED: many short cuts cost the decoder more than held frames do; what one cut costs on screen is not settled by
  reading (X8, X8b).
- F7 VERIFIED src/analysis/BPMTracker.cpp:223 (the phase advances by hop / locked period: the beat runs off the same
  tempo number the clip's rate uses). With RU V12 (detections realign the phase), INFERRED: in Manual the clip and the
  beat differ only by two clocks; in Auto the beat is also moved by detections the number does not carry, so the clip
  drifts by the number's error. COMPUTED: an error of 0.1 % is 0.10 beat in 100 beats (50 s at 120, about 12 cuts in
  ten minutes); 0.2 % is one in 50 beats. How large the error is on his music is NOT KNOWN. It is NC (TB-3).
- F8 VERIFIED BD:859-861 "If we are shifted forward or back, everything that is connected to BPM shifts forward or back.
  I mean everything. ..."; BD:799 "... move the beat forward or back to get it to match the image exactly ...";
  RN:322-326 (the nudge lane publishes `float beatNudgeAppliedMs` at offset 332 and `uint8_t beatShiftState` at 336),
  RN:328-331 (the applied value GLIDES at a quarter of real time), RN:437-441 (a running synced clip does not move with
  the nudge; moving it is this lane's), RN:483 ("an injected snapshot is published as given"). COMPUTED: 0.05 beat is
  25 ms at 120 and 50 ms at 60; 0.10 beat is 50 ms and 100 ms. ST-7 holds, and holds harder once the hold is gone.
- F9 VERIFIED src/analysis/FeatureSnapshot.h:139-144: `trackerRequestSeq` reflects "every tempo / Tap / Resync /
  manual-mode request"; it is 0 "in test mode (no analysis thread)". It is the one published mark of his hand on the
  beat that exists on main.
- F10 VERIFIED src/render/Renderer.cpp:1657-1667 (video: `if (snap.bpm > 0.0f && clip->beatDivision > 0.0f)
  player->setSpeed(...)`, no else: at a tempo of 0 the last speed stays), :1705-1706 and :1718-1734 (a sequence: its
  own speed first, its fps from the tempo only above 0). GA-2's citation holds. src/analysis/BPMTracker.h:41-42 (60 ..
  200): a tempo set by hand is never 0; 0 is "no tempo yet". NR:190-217 and :258-271 (a plan): the beat timer is a
  tracker state published in two more bits of `beatShiftState`, read through `bool beatRunning(uint8_t)` in a new
  src/model/BeatTimer.h; "PUBLISH bpm = 0" is rejected there (NR:211-213) for this very pin fact; "RT's `TempoView`
  gains `bool running` ... Whichever of the two lanes merges SECOND makes this its step 0" (NR:269-271).
- F11 VERIFIED tests/CMakeLists.txt:43-44, :171: MainComponent.cpp and Renderer.cpp are linked into no ctest target.
  A unit case cannot call "the Renderer line" GA-2 asks for; a lint can read it.
- F12 VERIFIED src/core/ClipCommands.h:80-141: `SetClipCmd` holds `std::optional<Clip> before_, after_`; `undo()`
  applies `before_`; `apply` does `cell = *state` (:117). src/MainComponent.cpp:5306-5322: `snapshotCell` copies the
  whole Clip. RT:434-449: the landing rule and `TransportState`. So a snapshot taken AFTER the pause carries the pause,
  and landing it changes nothing. GA-1 holds.
- F13 VERIFIED src/api/ApiServer.cpp:328 and :2178-2197: `POST /api/debug/save_composition {"path"}` exists (an
  absolute file in an existing folder). :340 `/api/debug/undo`; :182 `/api/trigger_column`. `set_clip_param` takes only
  `fitMode` and is "Not undo-recorded" (its own comment). RT:1014-1017: the chain's TR13b makes an Undo step through
  "the Undo-step test route" (an effect added to the playing clip). No route renames a clip (a grep finds only
  `/api/debug/deck_rename`). So TR25 (f) has its route, and TR25 (h)'s command is TR13b's, not a rename.
- F14 VERIFIED MainComponent.cpp:6541-6567: `applyClipPlaying` -- the actions "play", "resume", "pause", "stop",
  "reverse" write `clip->playing` / `reverse` / the playhead, and every one that is not a replay is captured on the
  control "playing". :2004-2011: a replay applies them through the same function. :611-622: the top bar's Play and
  Pause call it for every layer's playing clip. :7891-7903: the `LayerTransport` pad. :5021-5035: `captureAutoPlay`
  ("resume" after a fire that auto-plays). src/recording/PerfState.h:38-42 and PerfState.cpp:58, :71 (`playing` per
  captured clip); RoutineSlice.cpp:110-141 (a routine's preamble: "resume" | "pause"). docs/claude/recording.md:11:
  a replay first restores "each captured clip's effect values/scalars/play-pause"; :46-47: a routine's restore fires
  "clips, flags, play/pause". BD:495 "yes" (replay restores the look at Record time -- Harmony's text); BD:435
  "restore". src/ui/LayerStrip.cpp:370-392 and src/ui/ClipInspector.cpp:37-53 write the clip directly and capture
  nothing. So: at the pin a take records and restores a clip's play / pause, and the plan's writer list (PA:290-301)
  misses the top bar, and names the pad without its function.
- F15 VERIFIED LayerStrip.cpp:391 (`clip->speed = std::min(clip->speed * 2.0f, 4.0f)`: 0 stays 0), :396-411 (the S fader:
  0 .. 1, default 0.25, speed = value x 4). ClipInspector.cpp:78-80 (range 0 .. 4, step 0.01, default 1), :106-113
  (halve, floor 0.01; double, cap 4). ST-6 holds; the dead button at 0 is a defect of main today.
- F16 VERIFIED src/render/Renderer.h:290-293 (`effectiveClipSpeed` = speed x master when not synced); src/ui/
  CompositionInspector.cpp:148 (master = value x 4); Renderer.cpp:1670 (pushed, no clamp); src/media/VideoPlayer.h:81.
  COMPUTED: the highest product at the pin is 4 x 4 = 16; with a top of 10 it is 40. ST-4 holds.
- F17 VERIFIED facts-resolume-transport.md:66 and :150 (the Catch Up sentence, CONFIRMED). RU:479-481 (RA-8, adopted):
  "Off without Catch Up: the clip plays on from where it is and the lock, if one is built, slides it." PL:935-936
  (check 19). So "it ends in time either way" was ruled before this delta; what 71 B changed is how soon. He has never
  been told it differs from Resolume: ST-3 is right about that.
- F18 COUNTED. The plan's own lists give 113 cases and 9 lints: 122. Its G-U1 says "N >= 121" (PA:594). Off by one.
- F19 VERIFIED BD:864-871, :879-881: questions 47, 48, 49, 50 are answered.
- F20 VERIFIED PL:417-418: e is "wrapped into [-period/2, period/2)". At exactly half the period it reads minus half.
- F21 VERIFIED, as the plan cites them: src/model/Clip.h:237-240; Clip.cpp:65-71 and :213-226 (no key for `playing` or
  `paused`; `speed` loaded with no clamp); src/model/Layer.h:548-558; src/render/ClipTransportSync.h:33-41, :45-73;
  Renderer.cpp:1632-1638 (a player is found by CLIP id); src/ui/ClipCell.cpp:190-197 ("L", top right), :204-215 ("!",
  top left).
NOT VERIFIED BY ME (a run, or a file I did not open; each has its owner): every measured number (Harmony); the read
site in PerfStateCapture.cpp and `Player::firePreambleDiscrete` (S0); whether `/api/inject_features` posts the beat
count and the request sequence (S0); src/ui/DeckView.cpp:311 (S0); which published field marks a Resync and a start
from stop (S0, and the later merge's step 0); the ruling's V-rows other than through the code lines named above.

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)

| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| GA-1 | MUST | ACCEPT | F12: `cell = *state` lands the snapshot; a snapshot taken after the pause holds the pause, so MU-103 passes the arm and check 26. Also F13: there is no clip-rename route; the Undo step is TR13b's. Re-ordered, in both directions (snapshot not paused over a paused cell; snapshot paused over a playing cell). | TB-9 |
| GA-2 | MUST | ACCEPT | F10: at the pin a tempo of 0 leaves the last speed; the nudge-row plan publishes a state, never a tempo of 0. The sentence is struck; tempo 0 is pinned as its own case; `beatRunning` moves to S4a. F11: "the Renderer line" cannot be a unit case, so it is a lint that arms itself when the nudge lane's files are on the tree. | TB-5, TB-6 |
| GA-3 | SHOULD | ACCEPT | PA:401-402 makes the top data; PA:643-647 writes 10 and a 5 % band where X6t has 3 %; a 0.8 loop at 10 wraps every 0.8 s. TR32 reads the top from a pin, takes X6t's band and window, and says how reads are unwrapped. | TB-10 |
| GA-4 | SHOULD | PARTIAL | PA:225 "issues the seek itself" breaks RA-1's own rule (the probe calls the product functions). ACCEPTED: the cut is one function built at SM-b and called by the probe; M5, M8b and M3 on one track run again on the lane build; the three-clip cut is a gate (X8b). REJECTED: all three tracks again -- the second M3 proves wiring, the first proved the law. | TB-1, TB-4 |
| GA-5 | SHOULD | ACCEPT | PA:565-566 is satisfied by a fixture that stays in time; PA:510-511 has no checkable meaning; PA:518-520 mixes speeds and travels. Re-written in 5.1. | 5.1 |
| GA-6 | SHOULD | ACCEPT | PA:259-260 lets merge 2 go without S4c; PA:481, :594-599 pin one count. F18: the pinned count was also off by one. A table, by what is on the tree. | TB-13 |
| GA-7 | SHOULD | ACCEPT | F4: the adopted design is "no trim". F7: the need is a number nobody has. The hold is not built; SM-a prints the number (NC) with a bar; over the bar he is asked, with the hold as the named fallback and its own number (NCH) beside it. | TB-1, TB-3 |
| GA-8 | NIT | ACCEPT | F13: the save route exists, so (f) has no fallback. TR24 and TR24b get an owner (Harmony, at the later of the two merges) and a place in that merge's pin. | TB-6, TB-9, TB-13 |
| ST-1 | MUST | ACCEPT | F2, F4, F5. He answered "71 b" to a question whose B reads "plays on from where it is and cuts once"; a clip that stands for up to 3 frames at 120 (6 at 60), or runs double as long, after a Tap, nudge or realign that lands in the band is a speed change he was not shown. Built only if measured and only on his word. | TB-1, TB-3 |
| ST-2 | SHOULD | ACCEPT | PA:160-175 has no row for his own play button on a synced clip; F20 settles the tie (minus half: the cut goes forward). Row, case fixture, D-6 and a check added. | TB-2, 5.1, 6 |
| ST-3 | SHOULD | PARTIAL | F17. ACCEPTED: he is asked (124) and the line that changes is named. REJECTED: Arena's behaviour as the default -- "9 b" (BD:763) and RA-8 as adopted already bring such a clip back; the default stays his own rule for an out-of-time clip. | TB-14 |
| ST-4 | SHOULD | ACCEPT | F16: 10 x 4 = 40 into a player measured to 16 at most. Speed x master stops at 16, the top it can reach today. Not a second measurement arm: at 40 nobody expects a pass. | TB-11 |
| ST-5 | SHOULD | ACCEPT | as GA-3; the law is written for any top and is today's fader at a top of 4. | TB-10 |
| ST-6 | SHOULD | PARTIAL | F15: 0 x 2 = 0, on main today and in the line S5b rewrites. ACCEPTED: at 0 the button gives 1; a case clause and a check line. REJECTED: a new sign for Speed 0 -- the number and the thumb are the state. | TB-12 |
| ST-7 | SHOULD | PARTIAL | F8, F9. ACCEPTED: a nudge must move a running synced clip, and the gate must show a small one doing it. REJECTED: a deadband in ms on a hold -- no hold is built. The clip follows by his own rule for an out-of-time clip: one cut on the next "1", for any move of his hand above one picture frame. | TB-2 |
| ST-8 | SHOULD | PARTIAL | F10. ACCEPTED: the "tempo 0" sentence is struck; the live row has an owner and a pin; check 27 says the circle shows the stopped beat. REJECTED: a test route that switches the beat off in a product that has no such control; the rule is a unit case, its wiring a self-arming lint, its live proof the later merge's row. | TB-6 |

## 3 AMENDMENTS (TB-1 .. TB-14; each OVERRIDES the plan body where they differ)

The real-time rules hold in every amendment: nothing touches the audio callback or the analysis thread (S4t is the
ruling's, unchanged); the cut, the "1" detector, the anchor and the hand record are plain GL-thread arithmetic and one
GL-thread record; `paused` and `pausedAt` are `Relaxed<T>` model fields (Pitfall 63); the render thread never waits; a
seek is a request (F6); NO new mutex. No amendment adds a slider, a menu or a text: the plan's Speed slider and S fader
stay `ResettableSlider`s with their defaults (PA:374-378), and nothing on screen announces a cut, a pause or a stop.

TB-1 THE LAW: THE RATE AND ONE CUT (ST-1, GA-7, GA-4). REPLACES PA:124-131 (the three bands), PA:135-136 and PA:143-144
(`gridHoldFactor`, "x the hold factor"), PA:152-156 ("WHY 0.05"), and fork F-A1's choice (PA:183-190: the runner-up is
built). The plan's replacement of PL:415-445, of RA-2's "met" row and of RA-9 STANDS; RA-5's "x (1 + trim) from S4c on"
is struck and nothing takes its place.
- e and its reader: as PA:121-123. The anchored reader, always (TA-3 STANDS).
- IN TIME, |e| <= `kOutOfTimeBeats` = 0.10: nothing is done. OUT OF TIME: the clip plays on; on the frame in which the
  music's "1" is crossed ONE `seekTo` is issued, to the nearest place the bar gives (`catchUpTarget`, RA-8), one render
  frame ahead. Decided every frame from this frame's e alone; no stored offset.
- The speed pushed in BPM Sync is `barSyncRate x syncSpeed`, exactly, in every frame of every state. S4c adds no term
  to `transportSpeed`.
- Pure, in src/render/ClipBarGrid.h, built at SM-b: `bool outOfTime(double errorBeats) noexcept;`
  `double seatFloorBeats(double bpm) noexcept;`
  `bool cutDue(double errorBeats, double bpm, bool barOneCrossed, bool handMoved) noexcept;`
  `bool barOneCrossed(...)` as PA:138-140; `double catchUpTarget(...)` (RA-8's, built here as the plan says).
  Constants `kOutOfTimeBeats` 0.10 and `kSeatFloorSeconds` 0.016. NOT built: `gridHoldFactor`, `kGridDeadbandBeats`.
  `barLockTrim`, `kBarLockTauBeats`, `kBarLockMaxTrim`, `kBarLockDeadbandBeats` go, as the plan says.
- THE CUT IS ONE FUNCTION, built at SM-b in src/render/ClipTransportSync.h, so that the probe measures the code that
  ships: `struct CutView { double clipBar; float syncSpeed; int periodBeats; bool backward; double showBarBeats; double
  bpm; double frameSeconds; bool barOneCrossed; bool handMoved; bool off; };`
  `template <class Player> bool applyCut(Player&, const CutView&) noexcept;` (true when it issued the seek).
  The probe's branch fills a `CutView` from its record and calls it (PA:225 "issues the seek itself" is struck). S4c
  adds, in the same header, `template <class Player> CutView cutViewFor(const Clip&, const Player&, const Intent&,
  const TempoView&) noexcept;` -- the view from the model, the OFF list of TB-5 included -- and each branch of
  `syncMedia` reads `applyCut(player, cutViewFor(...))` once, after `setRange` and the restart, before the advance,
  and decides nothing itself (Renderer.cpp is in no ctest target, F11). At S4c `TempoView` gains `double frameSeconds;
  bool barOneCrossed; bool handMoved;` (the plan's three at PA:142, with `beatRunning` moved to S4a by TB-6 and
  `handMoved` added). S4c swaps inputs and writes no law (RA-1's rule, kept).
- `Renderer` keeps beside the anchor: last frame's count and the hand record of TB-2. ONE record for the show, updated
  once per frame where `frameSnap_` is read (Pitfall 38's shape), never per clip; GL thread only.

TB-2 HIS HAND, AND WHAT EACH EVENT DOES (ST-7, ST-2). REPLACES the table of PA:160-175. AMENDS RA-16.
- `struct BeatHand { uint32_t requestSeqSeen; float nudgeSeen; bool pending; };`
  `bool noteBeatHand(BeatHand&, uint32_t trackerRequestSeq, float nudgeAppliedMs, bool barOneCrossed) noexcept;`
  (ClipBarGrid.h, SM-b; pure.) A changed request sequence or a changed applied nudge sets `pending`; the call returns
  `pending`; a frame in which the "1" was crossed clears it after that frame. On this lane's base the renderer passes
  `frameSnap_.trackerRequestSeq` (F9) and the literal `0.0f`; TB-6 says when the literal goes.
- `cutDue` = the "1" was crossed AND |e| is above 0.10 -- or, with the hand record pending, above `seatFloorBeats(bpm)`
  = 0.016 x bpm / 60 (0.016 beat at 60, 0.032 at 120, 0.053 at 200).
- WHY 16 ms (COMPUTED; the hop is CLAUDE.md's: 512 samples at 48 kHz = 10.7 ms): the bar position is published once
  per hop, so a clip seated from one snapshot and measured against another reads up to one hop off without having
  moved. 16 ms is a hop and a half, and one picture frame at 60 Hz. TL-U45c (5.1) is the guard: if it asks for a
  second seek the builder STOPS and reports; he does not tune. (Named candidate for the architect line: two hops,
  0.0213.)
- RA-16 gains: "A nudge is a move of his hand. On a tree that has the nudge lane's `beatNudgeAppliedMs` (F8) the hand
  record reads it (TB-6); no other line of this lane changes." TR24: 5.3.

| event | cut? | when |
|---|---|---|
| a fire that lands on the "1" (Snap Bar, 2 Bar, 4 Bar) | no | -- |
| a fire off the "1" (Snap off or Beat) | if more than 0.10 beat off | it starts at its beginning (BD:844), plays on; ONE cut on the next "1" (question 123: where to) |
| a playhead drop | if more than 0.10 off | plays on from the drop; ONE cut on the next "1" |
| play after his pause (strip or Clip tab) | if more than 0.10 off | runs on from its frame; ONE cut on the next "1" |
| a Tap | if more than one frame (16 ms) off the tapped beat | ONE cut on the next "1"; while he taps, at most one per "1" |
| a Resync | if more than one frame off | AT ONCE: the press is the "1" (question 121) |
| a tempo value, BPM minus / plus, /2, x2 | the rate follows at once, the phase is continuous (PL:514) | a clip more than one frame off is seated on the next "1"; a clip in time shows nothing |
| a nudge (on a tree with the nudge lane) | if more than one frame off the SHIFTED beat | ONE cut on the next "1"; if the beat was still gliding there (RN:328-331), once more on the "1" after |
| Auto moves its own "1" (the anchor changes, RA-3) | yes | 8 beats later the anchor follows; ONE cut on the next "1" after that |
| Auto realigns the beat by more than 0.10 beat | yes | ONE cut on the next "1" |
| Auto's small realigns, a wobbling tempo, drift | not until the clip is more than 0.10 off | then ONE cut on a "1" (how often: NC) |
| BeatLoopr Off without Catch Up; Speed stepped up from 0; +4 bars, x2, /2, a moved out point | if more than 0.10 off | ONE cut on the next "1" (BeatLoopr: question 124) |
| the beat timer paused or stopped (BD:918-920) | no | the clip STANDS (speed 0); its play button stays lit; its pause is not touched (TB-6) |
| the beat timer started again | after a pause: no. After a stop: his hand | after a stop: a cut at the start or on the next "1" (TB-6) |

TB-3 THE FIRST MEASUREMENT AND ITS TABLE (GA-7, ST-1). REPLACES PA:204-220 (TA-4) and PA:242-260 (TA-6's table).
- SM-a is unchanged as a run (three of his tracks, Auto, ten minutes each, production mode, `/api/bpm` at 50 Hz, no
  product code, FIRST). Its offline script runs a virtual 4-bar clip under THE LAW OF TB-1: started in time at the
  anchor's start, advanced by the polled tempo over the poller's own clock, the bar read through the anchor, a cut on a
  "1" beyond 0.10 beat, no hand moves. Per track it prints:
  N1 (unchanged). BAR: at most 1 per 10 minutes on each track.
  NC = the cuts that clip would have been given. BAR: at most 1 per 10 minutes on each track. Printed with it: how many
  were larger than half a beat (the "1" moved) and how many smaller (drift, realigns).
  NCH (INFO, no bar) = the cuts the same clip would have shown under the plan's held band (PA:124-131 as written).
  H1 (INFO) = the share of reads in which that band's factor is not 1. R1, X7: as the ruling; X7's bar and O7 stand.
- The script's self-test: the plan's three synthetic logs (PA:214-215) and a fourth -- a tempo number 0.2 % above the
  beat's own pace for 1200 beats at 120: NC between 22 and 24, NCH 0; its mutated copy (the clip advanced by the beat
  instead of the tempo number): NC 0. The instrument is shown to see drift before it is trusted (RIG-RULES A2).
- SM-a's second run, on the first tree that carries both this lane and the nudge lane: STANDS (PA:216-217); owner TB-6.
- THE TABLE. Two verdicts, as before: one tracker row (O7, O3, O3d or "tracker met", the first that matches) and one
  player row (O6 and O6t beside the others; then O5, O5b, O8, O9 or "met").

| row | when | what is built | Harmony tells Boris |
|---|---|---|---|
| O7 | as the ruling | as the ruling | as the ruling |
| O3 | N1 above 1 on any track | STOP AND ASK. The ruling's named fallback stands (period 1 beat in Auto, the bar in Manual and after a Resync), built only on his word | "In Auto, on <track>, the app moved its '1' N times in ten minutes. Each time every synced clip would jump by a beat or two. Either clips stay on the beat in Auto and on the bar when you tap and Resync, or we fix the '1' first. Which?" |
| O3d | N1 met on every track, NC above 1 on any | STOP AND ASK, for S4c only; every other stage goes on. Named fallback, built only on his word: THE HELD BAND (the appendix at the end of this section) | "In Auto, on <track>, a synced clip would have cut N times in ten minutes, each time by about a tenth of a beat, on a '1': the tempo the app reads and its beat drift apart. Three ways. A: leave it -- small cuts on the '1'. B: between cuts the app repeats or skips single frames so the clip never gets that far off; it would then have cut M times. C: I fix the tempo reading first. Which?" (N = NC, M = NCH. The cause in the sentence follows the split the script prints: if the cuts are larger than half a beat it says "by a beat or more, when the app re-places its beat", not "drift") |
| tracker met | N1 and NC met on every track | S4c as TB-1 | "When the app itself moves its '1' in Auto, a clip follows two bars later, with one cut on a '1'." |
| O6 | as the ruling | as the ruling | as the ruling |
| O6t | X6t fails at the top in any arm | TB-10 | PA:403-404's sentence |
| O5 | X8 fails, or X4c fails | STOP AND ASK; S4c is not built; the lane goes on without it | PA:253's sentence |
| O5b | X8, X4c met; X8b fails | STOP and report; S4c waits. Named candidate, an architect line decides: at most one cut is issued per rendered frame, in layer order, each target worked out for its own frame | (the report) |
| O4 | -- | VOID (its fallback is the design) | -- |
| O8 | M1, M2, M4 met; X2 fails in M3 only | STOP and report. Named candidates, nothing pre-decided: e averaged over one beat for the decision; a wider out-of-time band in Auto; the held band | as the ruling |
| O9 | as the ruling | as the ruling | as the ruling |
| met | X2, X2c, X3, X4c, X5, X8, X8b met | S4c as TB-1 | "Yes: measured over ten minutes on three tracks and at three tempos. A clip that is out of time plays on and cuts once, on the next '1', into time, and stays there." |

RA-11's last rule STANDS: if no row allows S4c by the time S6 is done, merge 2 goes without it and D-6, checks 13 .. 16
and TR20 .. TR22 wait.

TB-4 THE INSTRUMENT: WHAT IS STILL NEEDED AND WHAT IS VOID (GA-4, GA-5). REPLACES PA:222-241 (TA-5).
STILL NEEDED, unchanged: ClipBarGrid.h, BarLockProbe.h and its ring, the four routes, the two shown-frame accessors and
the seek-pending flag, `.harmony/probe-barlock.*`, TL-L6, G-P1, the production-mode recipe, the 22 drops, every run
M1 .. M8. CHANGED: the probe's branch calls `noteBeatHand` and `applyCut` (TB-1, TB-2); a sample gains three fields
(the seek issued this frame, its target, the hand record). VOID: `barLockTrim` and its three constants; bar X4; row
O4; the raw reader as a way to lock (the route keeps `reader` for R1's record and defaults to anchored); the hold.

| id | now |
|---|---|
| N1 | STANDS: at most 1 per 10 min per track |
| R1 | MEASURED AND REPORTED; selects nothing (TA-3) |
| NC | NEW: at most 1 per 10 min per track (SM-a); confirmed on M3: the real clip's seeks on each track are at most 1 |
| X7 | STANDS, with O7 |
| X2 | The ruling's shares and its fifth-minute clause, FROM 1 BEAT AFTER THE FIRE (M1, M2, M3 fire on a "1"). Tightened; as the plan |
| X2c | NEW (M1; each tempo of M2): fired on a "1" and left alone, the clip is given at most 1 seek over the whole run. It is "how often does the picture cut" on an exact and on a typed tempo. Over: row O9, and the report names what lost the time (the two clocks, a wrap, a long frame) |
| X3 | STANDS, as a guard: the speed pushed is exactly the rate, so only a window that holds a cut can leave the 2 %. It has its old teeth again if the held band is ever built |
| X4 | VOID. Replaced by X4c |
| X4c | STANDS as PA:237 wrote it (22 drops) |
| X5 | RE-STATED for TB-2 (M5 on a tree that has S4t; M6). Over the eight Taps: at most one seek per "1" crossed between the first Tap and the first "1" after the last, each issued on a "1", none later; c unchanged; abs(e) <= 0.10 from 5 beats after the last Tap. The Resync: one seek within 3 render frames of the frame it lands in; abs(e) <= 0.10 from a quarter of a beat later. The tempo value 120 -> 128 -> 120: no seek off a "1", at most one per "1", abs(e) <= 0.15 throughout. M6 (at 120, where 30 ms is 0.06 beat): the driver moves its phase by 30 ms, twice, WITH the request sequence raised: each time exactly one seek, on the next "1", then abs(e) <= 0.05; once more WITHOUT the sequence raised: no seek in the next 8 beats |
| X6 | STANDS. X6t beside it (TB-10) |
| X8 | STANDS. It is THE player bar of the design |
| X8b | NEW (M8b): three synced video clips on three layers, at least one on the long-GOP fixture; five Resyncs through the tempo route with all three out of time. Each clip's shown frame is within one content frame of its target within 3 render frames; peak frame time <= 50 ms in the second after each. 5 of 5 |

AFTER S4c, on the lane build, same bars: M2 at one tempo, M4, M5 whole, M8b, and M3 on the ONE track with the highest
NC. Not met: STOP and report. (The first M3 runs all three tracks through the same functions; the second proves the
wiring in Auto.)

TB-5 THE OFF LIST, AND A TEMPO OF 0 (GA-2). REPLACES PA:148-150 and the sentence at PA:179-180.
- OFF = no seek is asked: the clip is not wanted, held or paused; a tempo of 0; a tracker that is not LOCKED; a length
  not known; speed step 0; a BeatLoopr loop that is on; a seek still pending; the beat not running (TB-6). Question
  124's B adds one line (TB-14).
- A tempo of 0 is "no tempo yet", never his stop (F10). `barSyncRate` is 0 there, so the speed pushed is 0 and the clip
  stands, its play state true, until there is a tempo. It is pinned in TL-U36 (S4a) and RED on S4a's base, where the
  last speed stays. The plan's sentence that this "already stands the clip still and no line changes" for BD:918-920
  is STRUCK.

TB-6 THE BEAT TIMER (GA-2, ST-8, GA-8). REPLACES PA:177-181; MOVES TL-U88 to S4a; AMENDS RA-5's `TempoView`.
- `TempoView` gains `bool beatRunning` at S4a, not at S4c. `transportSpeed`: in BPM Sync, not running -> 0. The
  Timeline branch never reads it. `Clip::paused` is not written and the play state stays true. So the rule holds even
  if no row ever allows S4c.
- One name on both sides: this lane's field is `beatRunning`; NR:269's "bool running" reads `beatRunning`.
- TWO EXPRESSIONS in Renderer.cpp wait for the nudge lane: the value of `beatRunning` (the literal `true` on this
  lane's base) and the nudge argument of `noteBeatHand` (the literal `0.0f`). Lint TL-L9 arms itself: "in Renderer.cpp
  `beatRunning` is filled in exactly one place and `noteBeatHand(` is called exactly once (one record for the show:
  at SM-b inside the probe's null test, from S4c on where `frameSnap_` is read, the probe using its result); if
  src/model/BeatTimer.h exists, the fill calls `beatRunning(`; if FeatureSnapshot.h declares `beatNudgeAppliedMs`, the
  call passes it; a literal in either place is then RED". RED arm: the builder shows once, with a scratch BeatTimer.h,
  that the literal turns it RED.
- THE LATER MERGE'S STEP 0 (builder: of whichever lane merges second; gate: Harmony): the two expressions read the
  snapshot; that merge's S0 names the published mark of a start from stop; TR24 and TR24b join that merge's pin
  (TB-13); SM-a's second run.
- STARTED AGAIN. After a pause the beat and the clip both stood: no cut. After a stop the start is a move of his hand
  (TB-2): the cut is at the start if the snapshot marks it as it marks a Resync, else on the next "1".
- WHAT THIS LANE DELIVERS OF BD:918-920: the rule in the speed function, its case, and a lint that cannot be forgotten.
  WHAT IT CANNOT: a beat that stops. Main has no control for it, so nothing ships that ignores one.
- REJECTED (ST-8): a test route in this lane that switches `beatRunning` off -- a switch for a state the product
  cannot reach.

TB-7 WHO WRITES THE PAUSE (F14). REPLACES PA:290-301. The fields, the keys, the tail, `holdPausedPlace`, the grid cell
and the landing rule of the plan's TA2 (PA:273-289, :302-338, :343-355) STAND.
All on the message thread, all through src/model/TransportPause.h:
- the strip's pause (sets) and its play, play-backwards and fourth button (clear): the layer's PLAYING clip
  (`Composition::playing(i)`, Pitfall 67);
- the Clip tab's pause (sets) and its play and play-backwards (clear): the SHOWN clip; never greyed;
- `MainComponent::applyClipPlaying` -- the ONE entry of the top bar's Play and Pause (for as long as they exist: the
  nudge-row plan gives those widgets to the beat timer, NR:183-189), of the `LayerTransport` pad, and of a take's or a
  routine's replay (TB-8);
- the test route `POST /api/debug/clip_pause {layer, column, paused}`.
Beside the plan's three functions: `enum class PlayAction { Play, Resume, Pause, Stop, Reverse };`
`bool parsePlayAction(std::string_view, PlayAction&) noexcept;`
`void applyPlayAction(Layer&, Clip&, PlayAction) noexcept;`
-- Play: forward, not paused. Resume: not paused. Pause: paused on its present place. Stop: paused on its in point.
Reverse: the direction flips, the pause stays. None writes `Clip::playing`. `applyClipPlaying` parses, calls it,
refreshes and captures as it does today; the value it captures is "not paused".
NOTHING ELSE writes the pause: no fire, no column fire, no clear of a layer, no Eject, no autopilot, no OSC, no
production REST route, no Undo, no Redo, no beat timer.
Lint TL-L4's clause reads: "`paused` and `pausedAt` are assigned only in TransportPause.h, in Clip.cpp's loader and in
the landing rule; `setPaused(` and `applyPlayAction(` are called only from the sites S0's table lists, count pinned;
inside `applyClipPlaying` no clip field is assigned".

TB-8 TAKES AND ROUTINES (F14). REPLACES PA:339-342.
- THE RULE: a take and a routine record and put back a clip's play / pause exactly as they do at the pin; what they
  write is now the clip's pause. What a take holds at the pin (F14): the play / pause of the clip each layer plays when
  Record is pressed; every pause and play pressed through the top bar or a pad while it records; the start of a clip
  that a fire set running. Said for him: a replay puts back the play / pause the take holds. A clip that played when
  Record was pressed plays in the replay, also if he has paused it since; one that was paused then is paused. When the
  replay holds at its end the clips stay as the take left them, and a Save then saves that. What a take does not hold
  it does not touch: a clip he paused later, which the take neither started nor paused nor played, keeps his pause.
- The wire format does not change: the control "playing"; the actions "play", "resume", "pause", "stop", "reverse";
  `PerfState`'s `playing` per clip; a routine's "resume" | "pause". An old take replays as it did. `pausedAt` is NOT
  recorded: a clip a replay pauses stands on the frame it has at that instant (video playheads are "NOT restored",
  recording.md:11).
- S0 lists every read and write of a clip's `playing` under src/recording and at MainComponent.cpp's capture and
  replay sites and says for each which it means after S2: "running" (`Clip::playing`) or "not paused". The play / pause
  lane means "not paused". Checkpoint 0 keeps what it has at the pin -- one play / pause entry per layer, for the clip
  that layer plays at Record time (src/recording/Program.cpp:510-518) -- and its value means "not paused"; it gains no
  entry for any other clip. A site where the two cannot be told apart is a STOP for an architect line.
- IF HE SAYS a replay must never touch a pause: one branch at the top of `applyClipPlaying` (a replay's Play, Resume,
  Pause and Stop do nothing), TL-U90's last clause and check 29 change. Nothing else.
- NOT in this lane: capturing the strip's and the Clip tab's own buttons in a take (not captured at the pin, F14).

TB-9 THE UNDO GATE (GA-1, GA-8). REPLACES TR25 (f) and (h) (PA:633-639), TL-U81's text (PA:538-539) and check 26
(PA:724-725). The landing rule itself (TA-8, PA:331-338) STANDS.
- The Undo step is made by TR13b's route (an effect added to the clip, RT:1014-1017); there is no rename route (F13).
  That route addresses a cell by (layer, column), played or not: S1's builder builds it so, and S0 notes it.
- (h1) the step is made while A is NOT paused; A is paused at p1; undo; redo: after each, A's `paused` is true and its
  place within one frame of p1. Run on the live cell, then with A in a cell no layer plays.
- (h2) A is paused; the step is made; A is played (the test route); undo; redo: after each, A's `paused` is false and
  two reads 0.3 s apart differ by >= 0.01.
- (f) saves through `POST /api/debug/save_composition` (F13). The "hand-written file" fallback is struck; "exactly
  once" stands.
- MU-103 must turn (h1) and (h2) RED; the lane report shows both.

TB-10 THE TOP IS DATA EVERYWHERE (GA-3, ST-5). AMENDS PA:363-373 (TA-9) and PA:395-404 (TA-10).
- `float timelineSpeedFromTravel(double travel, float top = kTimelineSpeedMax) noexcept;`
  `double travelFromTimelineSpeed(float speed, float top = kTimelineSpeedMax) noexcept;`
  `float clampTimelineSpeed(float speed, float top = kTimelineSpeedMax) noexcept;`
  Up to half the travel: 4 x travel. Above: 2 + (top - 2) x (2 x travel - 1). With a top of 4 it is today's fader.
- Every case, row, state and check names "the top", never the number: TL-U85, TL-U87, TL-U54, TR32, C17, W23, MU-104,
  MU-108, check 12.
- M7t runs at 10 AND at 8, three arms each, both fixtures, 60 s each: O6t's "8" is judged by its own Timeline arms, not
  by M7's BPM Sync arms. X6t = X6's numbers at each speed.
- The live probe pins `EXPECTED_SPEED_TOP=10`. Harmony changes it only with row O6t's verdict in writing; VG-2's
  manifest carries the same number.

TB-11 SPEED TIMES THE MASTER SPEED NEVER EXCEEDS 16 (ST-4). REPLACES PA:388.
- `constexpr float kSpeedTimesMasterMax = 16.0f;` in ClipTransportSync.h. In `transportSpeed`'s Timeline branch the
  product speed x master is taken as the smaller of itself and 16, then multiplied by `timelineRate` as the ruling
  says (PL:235-237, :334). (A product below 0, reachable only from a hand-edited file, is pushed as today.) 16 is the
  highest that product can reach at the pin (4 x 4, F16) and the top step X6 measures: 74 B widens nothing. TL-U36
  gains the clause (S4a); check 12 gains a line. Not touched: the Duration rate and the BPM Sync rate, which are the
  ruling's and can ask the player for more (side finding SF-3).

TB-12 THE FOURTH STRIP BUTTON AT SPEED 0 (ST-6). AMENDS PA:378-379.
- Timeline: at a speed of 0 it gives 1 (the default); otherwise it doubles, up to the top. BPM Sync: one step up
  (0 -> 1/8), as the ruling. It plays forward and clears the pause, as his hand.
- REJECTED: a new sign for Speed 0. The number 0, the thumb at the bottom and the lit play button are the state.

TB-13 COUNTS AND PINS ARE A TABLE (GA-6, F18). REPLACES PA:579, PA:594-596, PA:599 and "EXPECTED_ROWS=31" at PA:481.
Section 5.2 and 5.3 carry the tables. Harmony constraint (unchanged): every number is S0's recount, never lower.

TB-14 BEATLOOPR OFF WITHOUT CATCH UP (ST-3). The plan's rule STANDS as the default (one cut on the next "1": his rule
for any clip that is out of time, and RA-8 as adopted, F17). He is ASKED, because he has never been told it differs
from Arena: question 124. 124 B is: `RelaxedBool Clip::offBarKept` (runtime, not saved; static_assert pin with the
others), set by Off without Catch Up, cleared by a fire, a drop, a Resync, Catch Up and a new loop; one line in TB-5's
list ("a clip BeatLoopr left off the bar"); TL-U44's clause; TR29's last sentence; D-16; check 19.

THE PLAN'S CHANGE LIST (PA:407-454) IS READ WITH THESE CORRECTIONS; a row not named here stands as the plan wrote it.
- A, RA-1: SM-a prints NC, NCH, H1 (no "D1"); the probe's branch calls `noteBeatHand` and `applyCut`; M7t runs at 10
  and at 8. RA-2: its bars are TB-4's table and its decision table TB-3's. RA-3: stands as the plan; the hand record
  sits beside the anchor. RA-4: TL-U46 as 5.1 here. RA-5: "x the hold factor" is struck; `TempoView` gains
  `beatRunning` at S4a and `frameSeconds`, `barOneCrossed`, `handMoved` at S4c. RA-12: X6t at 10 and at 8. RA-13 and
  RA-16: 5.3 here and TB-2. RA-15: the fourth button also gets its rule at 0 (TB-12).
- D (unit cases) and E (live rows): the tables of 5.2 and 5.3 here. F (visual): 5.5 here.
- G: D-6 is re-written here; of the checks, the plan's 12, 15, 16, 26, 27 are re-written here and 28 .. 31 are new.
- H: questions 121 .. 124 as section 7 here. J: section 10 here.

APPENDIX TO SECTION 3 -- THE HELD BAND. NOT BUILT. It is built only under row O3d, on his word "B", and then exactly
so, by no builder's design: the band and its factor as PA:124-131 wrote them; `gridHoldFactor` and `kGridDeadbandBeats`
0.05 as PA:135-136; the reasoning and the stop rule of PA:152-156; `transportSpeed` multiplies by the factor in BPM
Sync; cases TL-U41h (the text of PA:507-508), TL-U45h (the ruling's TL-U45c text, PL:738-740, judging the band) and
TL-U36's clause of PA:549-550; mutants MU-72, MU-73, MU-100 as PA:581-585. X3 then judges it: M1 .. M3 are run again.
The cut and the hand rule stay as TB-1 and TB-2 beside it.

## 4 FINAL STAGES + ORDER (the ruling's section 4 and the plan's section 4 stand; a stage named here is changed ONLY as written here)

ONE lane, based on main. One builder context per stage. Every stage: its cases written first and shown RED by id on the
stage's base (a case that never failed is struck and reported), then GREEN at its head. A builder never runs a live
row, never launches the app, never gives a verdict, never tunes a constant this file names.

| key | against the plan (PA:461-481) | owns (added to the plan's list) | proves (added or re-stated) |
|---|---|---|---|
| S0 | ADDS to its table | -- | the plan's additions (PA:463), plus: every read and write of a clip's `playing` under src/recording and at MainComponent.cpp's capture / replay sites, each marked "running" or "not paused" (TB-8); every caller of `applyClipPlaying` and whether any sends "stop" for a clip; the route TR13b uses to make an Undo step; whether `/api/inject_features` posts the beat count and the request sequence (TR21 e, f need both); which published field marks a Resync; the recount of both columns of TB-13's tables; that SM-a's poller stamps a read with its own monotonic clock at the reply |
| SM-a | CHANGED | .harmony/probe-bar-one.py | N1, NC (with its split), NCH, H1, R1, X7; the four synthetic logs and their mutated copies (TB-3). THEN HARMONY: three tracks; the tracker row |
| S4t | STANDS as PA:465 (one builder at a time in BPMTracker.cpp; TL-U70 registered once) | -- | -- |
| SR | STANDS | -- | -- |
| SM-b | CHANGED | ClipBarGrid.h holds the functions of TB-1, TB-2, TB-10 and `catchUpTarget`; ClipTransportSync.h holds `CutView` and `applyCut`; the probe's branch calls them | TL-U32, U33, U34, U40, U41, U42, U43, U45c, U49a, U49j, U83, U85, U89; TL-L6; TL-L8 and TL-L9 in their first form. THEN HARMONY: M1 .. M8, M7t (at 10 and at 8), M8b; X2, X2c, X3, X4c, X5, X6, X6t, X8, X8b; the player row |
| S1 | ONE CLAUSE STRUCK, as the plan | -- | TL-UC17 without "the layer's pause" |
| S2 | RE-STATED: the plan's (PA:469) with TB-7, TB-8, TB-9 | + MainComponent.cpp (`applyClipPlaying` calls `applyPlayAction`; the captured value), the capture read S0 names under src/recording | the chain's 15 S2 cases + TL-U60 .. U64, U69, U80, U81, U82, U84, U90; TL-L1 (S2 form), TL-L1b, TL-L4 (with TB-7's clause); G-U4. Then Harmony: FM-1 |
| S2b, S3, S3h | as the plan (S3 adds TL-U86) | -- | -- |
| MERGE 1 | as the plan | Harmony | probe-transport at `EXPECTED_ROWS=18` (TR25 as 5.3); VG-1 with P4 and P5 |
| S4a | ADDS | ApiServer.cpp (`speed`); ClipTransportSync.h (`beatRunning` in `TempoView` and in `transportSpeed`; `kSpeedTimesMasterMax`) | TL-U36 with its two new clauses, TL-U88; TL-L7 with the plan's clause; TL-L9 final |
| S4d | STANDS | -- | -- |
| S4e | ONE CLAUSE, as the plan. With 124 B: the flag of TB-14 | -- | TL-U76 unchanged |
| S5a | CHANGED, as the plan | -- | + TL-U87 (against the top) |
| S5b | CHANGED | -- | TL-U54 (TB-12). Ends at VG-2 |
| S4c | RE-STATED (TB-1, TB-2) | the ruling's files: the `CutView` from the model, the two `applyCut` calls, the hand record read in `syncMedia`'s frame | TL-U44, U45, U45b, U46, U47, U48, U49; TL-L5, TL-L8 final; probe-video whole. Built when the tracker row is "tracker met" (or row O3d has his answer) AND the player row is "met"; O6 and O6t hold nothing back. Then Harmony: M2 at one tempo, M4, M5, M8b and M3 on one track, on the lane build |
| S6 | CHANGED | -- | probe-transport at the row count of TB-13; the plan's docs (PA:480) plus: the hand rule, "a take and a routine write the clip's pause", the top of Speed x master, the count table |
| MERGE 2 | CHANGED | Harmony | `EXPECTED_ROWS` and N by TB-13's tables |
| LM-0 | NEW: step 0 of whichever lane merges second (this one or the nudge lane) | Renderer.cpp's two expressions (TB-6) | TL-L9 on the merged tree; TR24, TR24b; SM-a's second run (Harmony) |

ORDER (unchanged): S0 -> SM-a (Harmony) -> S4t -> SR -> SM-b (then Harmony: the runs, the verdict) -> S1 -> S2 (-> S2b)
-> S3 -> S3h -> MERGE 1 -> S4a -> S4d -> S4e -> S5a -> S5b (VG-2) -> S4c -> S6 -> MERGE 2.
WHAT WAITS ON WHAT: this ruling, adopted: SM-a's script, and the packets of SM-b, S2, S3, S4a, S5a, S5b, S4c, S6. NOT S0,
SM-a's recipe, S4t, SR, S1. SM-a's run: three tracks from Boris (RQ-0; none = BLOCKED). SM-b: SR and S4t. S4c: the two
verdict rows and S4a. S5a and S5b: S4a; they do not wait for X6t. Questions 121 .. 124: nothing waits.
WHAT HARMONY RUNS HERSELF, AND WHEN: SM-a right after S0; FM-2 on the frozen app before S2; M1 .. M8, M7t, M8b after
SM-b (S1 .. S3h are built meanwhile); FM-1 after S2; FM-7 and FM-8 before S4a; merge 1's gate and VG-1; VG-2 after S5b;
the five runs after S4c; every row of 5.2 .. 5.5; six mutants of her choice (always MU-4, MU-18, MU-21, MU-48, MU-71,
MU-89); at the later merge TR24, TR24b and SM-a again. Every production-mode run follows the ruling's H-T5 recipe (the
live lock, never while his app runs, `open -g`, a file source, own-pid quit) and the SCREEN-SAFETY LAW (.harmony/
HANDOFF.md:105): no gate opens an Output window, takes a full-screen capture or sends synthetic input.

## 5 TESTS + GATE ROWS (pre-registered here; the plan's section 5 stands except where this section speaks)

Harmony constraint (unchanged): RED arm = the frozen pre-lane main app for a live row, the stage's base for a unit case;
a flake verdict needs >= 5 runs per arm; a bar is met or reported, never loosened; a bar inside 4 x the measured noise
is BLOCKED until she waives it in writing. Nothing here is a loosened bar: no bar of this lane has been run yet, and
every number below that differs from the plan's belongs to a law that differs (no hold; the hand rule).

### 5.1 Unit cases (each a TEST_CASE whose name begins with its id; RED arm: the stage's base, by id, in the builder's log)

The plan's texts STAND for TL-U60 .. U64, U69, U80, U82, U83, U84, U86, U47, U48 (PA:509-568). Re-stated or new:
SM-b (RED arm: ClipBarGrid.h and `applyCut` as stubs that return 0 / false / 1):
- TL-U41 "in time and out of time: an error of 0.10 beat is in time and 0.1001 is not; the seat floor is 16 ms of music
  -- 0.016 beat at 60, 0.032 at 120, 0.0533 at 200 (each +/- 1e-6); a cut is due only on a crossed 1: beyond 0.10
  always, beyond the floor when the hand record is pending, never without the 1"
- TL-U43 "a clip started 1.99 beats off the bar is pushed exactly the rate on every frame, is sought once on the next
  1 and is within 0.01 beat (+ 1e-6) from the frame the seek lands on; a clip 0.08 beat off is never sought in 64
  beats, its speed is exactly the rate on every frame and its error is still 0.08 (+/- 0.001) at the end; the position
  steps nowhere but at the one seek"
- TL-U45c "a bar position published every 512 samples at 48 kHz and read at 60 frames a second, at 90, 120, 174 and
  200 BPM: a 4-bar clip is seated by one cut and then left 96 beats with the hand record set in every bar -- no second
  seek is asked, and the speed pushed is exactly the rate on every frame". If it fails at 0.016 the builder STOPS.
- TL-U49j "the cut: asked only on the frame the bar's 1 is crossed and only when the clip is more than 0.10 beat off
  the anchored bar -- with the hand record pending, more than the seat floor; the target is the nearest place the bar
  gives, one frame ahead; an error of exactly half the period reads minus half and the cut goes forward; one seek and
  none on the frames after it; none while a seek is pending; none while the view is off; a one-beat flip of the raw bar
  that lasts a fraction of a beat across a 1 asks for none"
- TL-U85 "Timeline speed law, for a top of 10, of 8 and of 4: travel 0 is 0, a quarter is 1, a half is 2, three
  quarters is half of (2 + the top), 1 is the top; speed -> travel -> speed returns 0, 0.5, 1, 2, 4 and the top within
  1e-6; travel -> speed -> travel returns 0, 0.25, 0.5, 0.75, 1 within 1e-6; the clamp gives 0 for -1, the top for the
  top + 2, 1 for not a number; with a top of 4 the law is speed = 4 x travel"
- TL-U89 (NEW) "the hand record: a changed request sequence sets it, a changed applied nudge sets it, neither leaves
  it; it stays set through the frame in which the 1 is crossed and is clear on the frame after; a Resync sets it and is
  a 1 in the same frame"
S2 (RED arm: S2's base):
- TL-U81 "an Undo and a Redo that land on a cell holding the same clip leave its pause and its place as the CELL has
  them when the command's snapshot says otherwise -- a snapshot not paused over a paused cell, and a snapshot paused
  over a cell that plays -- on a live cell and on an idle one; an Undo that brings back a removed clip brings its
  pause; a duplicate copies it"
- TL-U90 (NEW) "the play actions: pause sets the clip's pause and its place; play clears it and sets forward; resume
  clears it and leaves the direction; stop pauses it on its in point; reverse flips the direction and leaves the pause;
  none writes the play state; a word that is none of the five does nothing"
S4a (RED arm: S4a's base):
- TL-U36 gains "; at a tempo of 0 the speed pushed in BPM Sync is 0 and the play state stays true; in Timeline, Speed
  times the master above 16 counts as 16 (speed 10 with the master at 4 pushes 16 times the Duration rate)". The plan's S4c clause for it (PA:549-550) is
  STRUCK: S4c adds no term.
- TL-U88 (moved here from S4c) "the beat not running: in BPM Sync the speed pushed is 0, the clip's play state stays
  true and its pause is not written; running again it goes on from that frame; a Timeline clip's speed is the same
  either way"
S4c (the fakes are the chain's; the real `pushIntent`, `transportSpeed`, `applyCut`, `writeBack` in the renderer's order):
- TL-U44 "off -- no seek is asked and the speed pushed is the speed function's own (the rate; 0 at a tempo of 0 and
  while the beat is not running): a paused clip, a held clip, a hold cleared between pushIntent and the push, a tempo
  of 0, a tracker that is not locked, a length not known, speed step 0, a BeatLoopr loop that is on, a seek still
  pending, the beat not running; a tracker state that flips every frame asks for no seek off a 1"
- TL-U45 "a tempo change from 120 to 90 on a clip in time: no seek on that frame; the next frame's speed is the new
  rate; the error stays inside 0.05 beat up to the next 1" | TL-U45b "a realign of 2 beats: no position step before
  the next 1; exactly one seek there; within 0.01 beat (+ 1e-6) after it"
- TL-U46 "a trimmed loop, in 0.2, out 0.7, 4 bars, at 200 BPM through the real sync and a player that wraps inside its
  range, 64 beats: no seek is asked, the speed pushed is exactly the rate on every frame, and the error stays within
  0.03 beat (+ 1e-6)"
- TL-U49 "a change on a clip in time -- (a) plus 4 bars, (b) x2, (c) /2, (d) a moved out point, (e) a speed step, (f)
  Speed stepped up from 0 after 3 beats, (g) play after a pause of 3 beats -- each in its own fixture that FIRST asserts
  the change left the clip more than 0.10 beat off: no seek on that frame nor before the next 1; exactly one seek on
  the next 1; within 0.01 beat (+ 1e-6) after it. And one fixture whose change leaves it within 0.10: no seek in 16
  beats"
S5a: TL-U87 "Clip tab, Timeline: the Speed slider runs 0 to the top with 1 at a quarter and 2 at half of its travel; its
default is 1 and a right-click gives it; minus and plus step 0.1 and stop at 0 and at the top".
S5b: TL-U54 "layer strip: in BPM Sync the S fader has nine detents and the forward button steps one up; in Timeline it
runs 0 to the top on the Timeline law with its default at a quarter of the travel, and the forward button doubles up to
the top -- at 0 it gives 1; the lit state of back, pause and play follows the playing clip's pause and direction and is
set only when it changes".
LINTS. TL-L4: TB-7's clause. TL-L7: the plan's S4a clause. TL-L8 (first form at SM-b, final at S4c): "`barLockTrim`,
`kBarLockMaxTrim`, `kBarLockTauBeats` and `gridHoldFactor` appear nowhere under src or tests; in src/render `seekTo(` is
called only inside `applyCut`, `holdPausedPlace`, the Catch Up site and the Random site, count pinned by S0; `applyCut(`
is called once at SM-b (the probe's branch) and exactly three times at S4c (the probe's branch and twice in
`syncMedia`)". TL-L9 (first form at SM-b, final at S4a): TB-6's text. RED arms: the builder shows once that one more
`applyCut(` call turns TL-L8 RED and that a scratch BeatTimer.h turns TL-L9 RED.
MUTANTS (each alone on the lane head turns the named case RED). The plan's list (PA:580-591) stands except: MU-73 and
MU-100 are VOID (they return with the appendix); MU-72 is RE-AIMED "the seat floor is 0" -> TL-U41, TL-U45c; MU-103
is run against TB-9's order -> TL-U81 (TR25 h1, h2). NEW: MU-111 the hand rule is off (a cut is due only beyond 0.10)
-> TL-U41, TL-U49j (TR21 e). MU-112 the hand record never clears -> TL-U89, TL-U45c (TR21 f). MU-113 a replayed pause
writes the play state, not the pause -> TL-U90. MU-114 Speed x master is not limited -> TL-U36. MU-115 at 0
the forward button doubles -> TL-U54. MU-116 a tempo of 0 keeps the last speed -> TL-U36.

### 5.2 Unit gates
- G-U1: the ruling's command and string. N is S0's recount, never lower than this table:

| at merge 2 | cases | lints | N (with S2b) |
|---|---|---|---|
| S4c built | 115 (the ruling's 103 + TL-U49j + TL-U80 .. U90) | 10 (the ruling's 8 + TL-L8, TL-L9) | 125 (126) |
| S4c not built (without TL-U44, U45, U45b, U46, U47, U48, U49 and TL-L5) | 108 | 9 | 117 (118) |
| the held band built (row O3d, his "B"): + TL-U41h, TL-U45h | 117 | 10 | 127 (128) |

  At merge 1 the pin is the count of the stages built by then, stated in the lane report before the gate runs.
- G-U3: the standing mutants + MU-60 .. MU-116 without MU-92, MU-73, MU-100; when S4c is not built also without MU-75
  and MU-77 (their cases are S4c's). G-U4: `EXPECTED_TSAN_CASES` unchanged (TL-U63 is the same case on the clip's two
  fields). G-U1b, G-U2, G-U5, G-N1, G-P1: as the ruling.

### 5.3 Live rows -- `.harmony/probe-transport.sh`; final line `PROBE-TRANSPORT GREEN`
Launch rules, fixtures, the driver, "frame code", tolerance, the flake rule, "a step" and "the driver's 1": the ruling's
and the plan's (PA:600-603). The pin:

| at the gate, on the tree | S4c built | S4c not built |
|---|---|---|
| merge 1 | 18 | 18 |
| merge 2, neither nudge file | 31 | 28 (TR20, TR21, TR22 wait) |
| merge 2 or LM-0, `beatNudgeAppliedMs` only | 32 (+ TR24) | 28 |
| merge 2 or LM-0, the nudge lane with its tempo row | 33 (+ TR24, TR24b) | 29 (+ TR24b) |

The plan's texts STAND for TR20, TR22, TR29 and TR31 (c) (PA:604-610, :618-623, :640-642). Re-stated or new:
- TR21 tempo_and_a_moved_beat. (a) .. (d) as PA:611-617; in (a), (b), (c) the driver does NOT raise the request
  sequence. NEW (e) the driver moves its phase by +0.09 beat AND raises the request sequence: no step; from 0.5 s after
  its next 1, five reads 0.5 s apart each have abs(e) <= 0.04. NEW (f) the driver moves its phase by -0.09 beat and
  raises nothing: five reads over the next 8 beats each have abs(e) >= 0.05. RED arm of (a): the frozen app's rate stays
  about 1.0; of (e): the frozen app seats nothing (reported; if it passes, (e) is a guard); (f) is a GUARD whose teeth
  are MU-112. Teeth: MU-76 (b), MU-106 (c), MU-94 (c), MU-111 (e: e stays 0.09), MU-112 (f: it is seated).
- TR23 lines_on_the_bar: the ruling's row. With S4c built: fired at driver bar position [1.20, 1.30], then "wait until
  4 beats after the driver's first 1 after the fire". Without S4c: fired on a "1" (Snap Bar), wait 4 beats.
- TR25 paused_fire (merge 1): arms (a) .. (e) and (g) as PA:626-636; (f) and (h1), (h2) as TB-9. Teeth: MU-60 (a, c,
  e), MU-61 (a), MU-101 (b), MU-102 (f), MU-110 (f), MU-103 (h1, h2).
- TR29: as PA:640-641. Without S4c its last sentence reads "no step at Off" (the ruling's text).
- TR32 timeline_speed_top (merge 2). The 10 s ramp, Timeline, in 0, out 0.8; T = `EXPECTED_SPEED_TOP`. `set_clip_param
  speed T`: the reply and `/api/composition` say T; reads at 20 Hz for 4 s, UNWRAPPED (a read lower than the one before
  it adds the range's length, 0.8): the content rate is within 3 % of T; every read is in [0, 0.8 + 1/300]. `speed
  T + 2`: stored T. `speed -1`: stored 0 and two reads 0.5 s apart are equal. `speed 1`: the rate over 4 s is within
  3 % of 1. The strip dump's S fader reads travel 1.0 at T and 0.25 at 1 (+/- 0.005). GUARD (the key is new). Teeth:
  MU-108 (stored 4), MU-104 (the dump's travel). If row O6t lowered the top, T is the lowered number and the row passes
  at it; a build whose top is not T is RED.
- TR24 lock_follows_the_nudge (when `beatNudgeAppliedMs` is on the tree and S4c is built; owner: Harmony at LM-0 or at
  merge 2, whichever is later). From TR20's state (driver at 60). In test mode an injected snapshot is published as
  given (RN:483), so the DRIVER moves the beat and posts the applied nudge with it; the nudge route itself is the
  nudge lane's rows. (a) +80 ms (0.08 beat): no step; from 0.5 s after the driver's next 1, five reads 0.5 s apart each
  have abs(e) <= 0.04 against the SHIFTED bar. (b) a further +300 ms: no step before the driver's next 1, exactly one
  within 0.3 s after it, then abs(e) <= 0.10. GUARD (the field is new). Teeth: MU-111 (a: e stays 0.08).
- TR24b beat_stopped (when the tempo row's beat timer is on the tree; same owner; in test mode the driver holds its
  beat and posts the state with it -- that merge's S0 says through which key). The beat stopped: a synced clip's
  two reads 1 s apart are equal and its `paused` is false; a Timeline clip's two reads differ by >= 0.01. Started after
  a pause: no step in 8 beats. Started after a stop: at most one step, at the start or within 0.3 s after the driver's
  next 1; then abs(e) <= 0.10 (without S4c: this last clause waits). GUARD (the state is new). Teeth: MU-107 (the
  synced clip's reads differ).
- Paths with no live row and what proves them: as the plan (PA:654-655), plus: a replayed pause and a restored one --
  TL-U90 and TL-L4; the top of Speed x master and a tempo of 0 -- TL-U36 and TL-L7; the hand record's two inputs on a
  merged tree -- TL-L9.

### 5.4 Re-runs and idle paint: as the plan (PA:657-658).

### 5.5 VISUAL WORK GATES (a capture builder, then five critic seats, then Harmony; Boris sees nothing before a gate passes)
THIS RULING DRAWS NOTHING NEW: no widget, mark or text beyond the plan's. What he will see, and the states to capture:
- VG-1 (before merge 1), as PA:661-670: P1 a playing layer; P2 the same clip paused (Clip tab, strip and its grid
  cell); P3 playing backwards; P4 the grid with three clips in view (paused and playing on its layer; paused and on no
  layer; not paused); P5 the paused clip that no layer plays, selected. Bars and questions for the seats: the plan's.
- VG-2 (after S5b), as PA:671-675, with the top as data: W23 "Timeline, Speed at the top (panel and strip)"; C17 reads
  "W1: the Speed thumb at 1/4 of its travel; W24: at 1/2, text 2; W23: at the top, the text reads `EXPECTED_SPEED_TOP`,
  plus disabled; the strip's S fader at the same three travels; all +/- 2 px". Teeth: MU-104 -> C17. The packet's D
  list is D-1 .. D-16 as section 6 has them.

### 5.6 Facts Harmony must measure (none can be settled by reading)
N1, NC (with its split), NCH, H1, R1, X7 on three of his tracks (SM-a), and once more on the first tree that has both
lanes. X2, X2c, X3, X4c, X5, X6, X6t (at 10 and at 8), X8, X8b (SM-b), with TB-3's table. The noise of e (5 runs of a clip
in time) and of the REST latency (20 calls). FM-1, FM-2, FM-7, FM-8: the ruling's. After S4c, on the lane build: M2 at
one tempo, M4, M5, M8b, M3 on the track with the highest NC; not met: STOP and report. At the later merge: TR24, TR24b.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; on his real screens and by his ear)

WHERE HIS RULING DIFFERS FROM RESOLUME. D-1 .. D-5, D-8 .. D-13 stand (the ruling's). D-7, D-14, D-15, D-16 stand as
the plan wrote them (PA:688-695). Re-written:
- D-6 A clip that is out of time is brought back by ONE cut on the next "1" -- after a drop, after play following a
  pause, after a Tap or a nudge that moves the beat, and after every fire that is not on the "1" (with Snap on Bar a
  fire lands on the "1" and nothing cuts). Resolume leaves it off the bar until it is fired again.
CHECKS. The chain's checks, the ruling's 1 .. 3, 8 .. 11, 13, 17, 18, 20 .. 23 and the plan's 4, 5, 6, 7, 14, 19, 24,
25 (PA:697-723) stand. Re-written:
- 12. Step Speed through its nine values in BPM Sync (panel, then the S fader). -> as the ruling. THEN in Timeline: drag
  Speed from 0 to the top on a clip you know, forwards and backwards, on the panel and on the S fader. -> 1 sits at a
  quarter, 2 in the middle, 10 at the top (a lower number if I told you the player could not do 10); the two controls
  move together; right-click gives 1. With Speed at 0 press the fourth strip button. -> The clip runs at normal speed.
  With Speed at the top push the master Speed up. -> It gets no faster than 16 times. Wrong: stutter, a frozen picture
  or black at the top of the range; a jump of speed in the middle of the slider; a button that does nothing.
- 15. Tap a new tempo while a synced clip plays. -> It takes the tempo at once. While you tap it may cut by a hair on a
  "1", onto your beat; after your last tap, once more at most. Wrong: a cut that is not on a "1"; a cut on every tap;
  it ends up off the "1".
- 16. In Auto, on your own music, leave a synced clip alone for ten minutes. -> Its lines stay on the "1" and it cuts
  once at most. Wrong: it cuts every so often -- by a hair (the tempo reading drifts) or by a beat or two (the app
  moved its "1"). Tell me the track and which of the two.
- 26. Add an effect to a clip. Then pause the clip. Press Cmd+Z, then Cmd+Shift+Z. -> It stays paused on its frame and
  keeps its mark. Then the other way: pause it, add an effect, press play, press Cmd+Z. -> It keeps playing. Wrong:
  Cmd+Z un-pauses it, pauses it, or moves its frame.
- 27. (when the tempo row is built) Press stop on the beat. -> The circle stands on "1"; every BPM-synced clip stands
  still with its play button still lit and no pause mark -- the circle is what tells you the beat is stopped; clips
  in Timeline keep playing. Press play: they run again. After pause-then-play nothing cuts; after stop-then-play each
  synced clip cuts once into time.
NEW:
- 28. Pause a BPM-synced clip for a few beats, then press play. -> It runs on from its frame and cuts once, on the next
  "1", into time. Wrong: it stays off the "1"; more than one cut.
- 29. Record a take while a clip plays. Stop. Pause that clip. Replay the take. -> The clip plays, as it did in the
  take, and stays as the take left it. Fire a routine cut from that take: the same. If that is wrong for you, say so:
  the other rule is that a replay never touches a pause.
- 30. (on a build that has the nudge) Nudge the beat by 30 ms while a synced clip plays. -> On the next "1" the clip
  cuts by a hair onto the nudged beat. Wrong: the clip does not follow; it cuts on bars where you touched nothing.
- 31. Launch the app in Auto with no music and fire a BPM-synced clip. -> It stands on its first frame until the app
  has a tempo; then it runs. (Today it runs at its own speed.) Say if it should run as at 120 until then.

## 7 BORIS QUESTIONS (121 .. 124; each has a default A; nothing waits)

121. You press Resync while BPM-synced clips are out of time.
     A (default) They cut into time at once: your press is the "1".
     B They play on and cut on the next "1", one bar after your press.
122. You save a show with a paused clip and open it another day.
     A (default) The clip is paused on the same frame as when you saved.
     B The clip is paused on its first frame.
123. You fire a BPM-synced clip between two "1"s. On the next "1" it cuts into time. Where to?
     A (default) To its nearest bar line: back to its beginning if you fired late in the bar, forward to the start of
       its second bar if you fired early.
     B Always back to its beginning, so it starts over on the "1".
124. BeatLoopr: you switch the loop off and Catch Up is not lit.
     A (default) The clip carries on from where it is and cuts into time on the next "1", like any clip that is out
       of time.
     B It stays where it is, off the bar, until you fire it again or press Resync. (This is what Resolume does.)
What each B changes: 121 -- one argument of `barOneCrossed` (a Resync is not a "1"), TL-U83's and TL-U89's last
clauses, X5's and TR21 (d)'s Resync sentence, D-14. 122 -- one line of Clip.cpp (the key `pausedAt` is not written),
TL-U80, TR25 (f)'s frame clause. 123 -- `applyCut`'s target for a clip that has not been cut since its stamp was
minted, one clause in TL-U49j, a frame-code clause in TR20. 124 -- TB-14.
Still held back, unchanged: 73 (FM-8). Requests RQ-0 .. RQ-3 stand; RQ-0 (three tracks) is the one the first run needs.
NOT a numbered question, and why: what is built if a clip would cut too often (rows O3 and O3d) is asked only if the
measurement says so, with the measured numbers in the sentence; whether the picture cuts on small nudges, what a replay
does to a pause, the fourth button at 0 and a tempo of 0 are ruled here and shown to him (checks 30, 29, 12, 31).

## 8 HARMONY'S DECISIONS (each with a default; the builder builds the default unless told otherwise)

- H-A1 The held band. DEFAULT: not built. Built only under row O3d and on his word "B", exactly as the appendix has it.
- H-A2 If SM-a gives O3d. DEFAULT: the STOP concerns S4c only (as H-T8 for O3 and O7); SM-b is built and run under
  the law of TB-1 and the lane goes on to merge 1.
- H-A3 TR24, TR24b and SM-a's second run. DEFAULT: Harmony runs them at the later of the two lanes' merges, inside
  that merge's pin (5.3); that merge's step-0 builder changes the two expressions of TB-6.
- H-A4 S4t and the nudge lane's S1 (the nudge adoption's H-10 asks this delta to rule). DEFAULT: as PA:465 -- if the
  nudge lane's S1 is built or in flight when this lane reaches S4t, S4t is built on top of it; otherwise S4t is built
  here on main and the nudge lane's S1 takes main in as its step 0. Never two builders in BPMTracker.cpp. TL-U70 is
  registered once, by whoever builds S4t.
- H-A5 The seat floor. DEFAULT: 0.016 s. If TL-U45c fails at it: STOP and an architect line (candidate 0.0213).
- H-A6 The top of Speed x master. DEFAULT: 16. ALTERNATIVE: no top, and an M7t arm at 10 with the master at 4.
- H-A7 The fourth strip button at Speed 0. DEFAULT: it gives 1. ALTERNATIVE: 0.1, the plus step.
- H-A8 Takes and routines. DEFAULT: TB-8 (as the pin does today). ALTERNATIVE: a replay never touches a pause (one
  branch). Check 29 is where he sees it.
- H-A9 M3 after S4c. DEFAULT: one track, the one with the highest NC. ALTERNATIVE: all three (thirty minutes more of
  production-mode runs on his machine).
- H-A10 `EXPECTED_SPEED_TOP`. DEFAULT: 10. Changed only with row O6t's verdict in writing, in the probe and in VG-2's
  manifest together.
- H-A11 A tempo of 0. DEFAULT: a synced clip stands (TB-5). ALTERNATIVE, if check 31 is wrong for him: it runs at the
  rate of 120 until there is a tempo (one line in `transportSpeed`; TL-U36's clause changes).
- H-A12 Questions 121 .. 124 go on his page before the packets of SM-b (121, 123), S2 (122) and S4e (124) are written.
  Unanswered by then: A is built.
- The ruling's H-T1 .. H-T9 stand as the plan left them (H-T7 closed; H-T1 is H-A4).

## 9 SIDE FINDINGS

- SF-1 (F14) A take does not capture the strip's and the Clip tab's own play, pause and back buttons at the pin: they
  write the clip and call no capture; only the top bar and the pad go through `applyClipPlaying`. Not changed here;
  it belongs to the recording lane.
- SF-2 (F18) The plan's count was off by one (121 for 122). TB-13's tables replace it.
- SF-3 (F16) The ruling's own rates can ask the player for any speed. BPM Sync: markedSeconds x tempo / (240 x bars),
  times a step up to 16 -- a 300 s part in 4 bars at 120 is 37.5 before the step. Timeline: a typed Duration multiplies
  on top of Speed (PL:235-237) -- an 8 s clip typed to 0.1 s asks for 80. X6 measures x8 and x16 on the fitted ramp
  only. For Harmony: an INFO arm in M7, or a ceiling in its own ruling. Not built here.
- SF-4 (F19) The dispatch lists questions 47 .. 50 as open; all four are answered.
- SF-5 plan-nudge-row.md was written a minute after the plan under ruling called the tempo row's delta "not written
  yet" (PA:177). It is a plan under attack. This ruling leans on three things of it: the state is a byte, not a tempo
  of 0; the predicate's name; the "second lane's step 0" rule. If its ruling changes one, TL-L9's text follows;
  nothing else here does.
- SF-6 (RN:483) In test mode the nudge is not applied to an injected snapshot. Any live row of any lane that "nudges"
  in test mode must move the driver.
- SF-7 (F10) At the pin a BPM-synced clip with no tempo keeps its last speed (a video) or its own (a sequence).
- SF-8 (F6) Every seek is a generation bump. A design that corrects a clip by frequent short seeks pays the decoder
  each time; a factor between 0 and 2 on the speed does not. That is why the held band, not "more cuts", is the named
  fallback of row O3d.
- SF-9 (F13) The plan's TR25 (h) "rename A (a command)" names a route that does not exist.
- SF-10 (F15) The fourth strip button does nothing at Speed 0 on main today.
- SF-11 (F14) The top bar's Pause is a pause of every playing clip. After S2 it writes a SAVED setting on each of
  them. The tempo-row plan gives those buttons to the beat timer (NR:183-189); until then this is what they do.
- SF-12 (F14) A take holds a fired clip's play state only when the fire set it running; a re-fire of a running clip
  holds none. So a replay over a clip he paused later leaves it paused at that fire. The same at the pin; for the
  recording lane.
- NOT IN THIS LANE: the plan's section 9 stands, and "the slide in any form" there now covers the held band unless row
  O3d and his word bring it. Added: capturing the strip's buttons in a take; a ceiling on the BPM Sync rate; a sign
  for Speed 0; a test route for `beatRunning`; the nudge's controls and the tempo row (the nudge lane's). The stopped
  sync-dial branches: nothing is carried; both heads were checked and neither tree was read for code.

## 10 RISKS (the strongest counterargument first)

- K-B1 THE STRONGEST, against this ruling: TAKING THE HOLD OUT TRADES A FRAME THAT STANDS FOR A SEEK, AND SM-a WILL
  PROBABLY SAY SO. A beat tracker's tempo number is ordinarily a tenth of a percent or two off its own beat (F7), so
  in Auto NC will likely exceed its bar on drift alone; the lane then stops at O3d and he is asked about single frames
  he cannot judge in words -- when the plan's hold would simply have worked, and every cut is a decoder bump where a
  held frame is free (F6). Why it stands: his answer is "71 b" to "No slide: ... cuts once", the adoption names the
  one-cut path "as THE design", and the hold is a speed change he was never shown -- up to three frames at 120 and six
  at 60, standing or doubled, after any Tap, nudge or realign that lands in the band (F5). Nothing is lost by
  measuring first: SM-a runs before any product code and prints NC and NCH side by side, so if the hold is needed the
  sentence he gets carries two measured numbers, and the hold is specified to the signature in the appendix. Cheapest
  refuting test: SM-a on one track.
- K-B2 THE HAND RULE IS THIS RULING'S OWN. No seat proposed it by name and the plan did not have it. It adds one
  GL-thread record and a second threshold, and it can cut on every "1" while he taps, nudges or rides BPM plus / minus.
  On a long-GOP file each cut may show as a held frame. Why it stands: without it a running clip ignores every nudge
  under a tenth of a beat, against "I mean everything" (BD:860), or the held band comes back. Guards: TL-U45c (the
  floor), X5 and M6, X8, X8b; if X8 fails, row O5 stops S4c. Cheapest refuting test: M6.
- K-B3 TB-8 puts "restore" (BD:435; BD:495 "yes") above "the clip is now paused until the user changes that setting"
  (BD:905) where the two meet in a replay: a routine fired on stage un-pauses a clip he parked. It is what the app does
  today; check 29 shows it; the other rule is one branch (H-A8).
- K-B4 `beatRunning` and the nudge argument are literals until the other lane is on the tree, and TL-L9 arms itself on
  a file name and a field name the nudge lane may still change (SF-5). LM-0's S0 re-reads both names first.
- K-B5 X2's bar and the cut's threshold are the same 0.10: a drifting clip sits just beyond it for up to a bar before
  its cut. With NC at most 1 that is 4 beats in 1200. If row O3d is answered "A" with a high NC, X2's 95 % in M3 can
  fail for that reason alone: that is row O8, never a loosened bar.
- K-B6 A tempo of 0 now stands a synced clip where today it runs (F10, TB-5). Check 31 shows him; H-A11 is the way back.
- K-B7 The top of Speed x master is a limit with no sign: two faders pushed together stop having effect at 16.
  Check 12.
- K-B8 A global Pause from the top bar before a Save marks every playing clip paused in the show (SF-11). It is his
  rule on a button that exists today. Say so when the lane is presented.
- The plan's K-A2, K-A5, K-A6 (now a gate, X8b), K-P1, K-P2, K-P4, K-S1, K-S2 and K-O1 stand. K-A1, K-A3 and K-A4 are
  closed by TB-1 (they return with the appendix). K-P3 is replaced by K-B3.
- The ruling's K-1 .. K-3, K-6 .. K-8, K-10, K-11 stand.

STATUS: DONE
