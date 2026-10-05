# PLAN -- lane "transport-answers": the transport lane, DELTA ON BORIS'S ANSWERS to questions 71, 72, 74 (s-rta-1004)

Role: architect, plan authoring. Read-only. Nothing built, nothing run, no app or probe launched.
Pin: main 185147b (`rev-parse --short HEAD` printed 185147b; `status --short -- src tests docs CMakeLists.txt` printed
nothing), so every src / docs line below is a plain-file read at the pin. Worktrees bf2 (740b6d6) and bf2keys (9eab9bd):
heads checked, clean, NOT read for code (nothing of them is a part of this lane, section 9).
Short names: RU:n = ruling-transport-delta2.md line n. PL:n = plan-transport-delta2.md line n. BD:n =
.harmony/binding-decisions.md line n. RT:n =
.harmony/.reports/s-rta-1003b/ruling-transport.md line n. RA-n, V-n, TL-U.., TR.., W.., C.., D-n, O-n, X-n, M-n, MU-n = the ruling's own ids.
Labels: VERIFIED = read by me in the named file at the pin; RULING-VERIFIED = a V-row of the ruling that I did not
re-read; INFERRED = reasoned from verified lines, not run; ASSUMED = neither.
Boris is quoted only verbatim and only from BD. Amendments of this delta are TA-1 .. TA-12; each names the ruling line it
replaces. EVERYTHING THE RULING SAYS THAT THIS FILE DOES NOT NAME STANDS AS WRITTEN.
Precedence asked for: Boris's words > the two adoption blocks (PL:1039-1089) > this delta (once ruled and adopted) >
ruling-transport-delta2.md > plan-transport-delta2.md > the chain.
A correction to the dispatch: questions 47, 48, 49, 50 are not open -- all four are answered (BD:864-871, :879-881;
RU V23). Nothing in this delta leans on them.

## 1 GOAL

Boris, after the ruling was adopted (BD:901-907): "71 b"; "new clip plays, the old clip is permanently paused and if comp
is saved, it is saved as paused. the clip is now paused until the user changes that setting."; "74 b". Three choices of
the ruling fall: the slide (RA-2's "met" row, RA-9, D-6), pause on the layer (RA-7, PL F38, D-7), Timeline Speed 0 .. 4
(PL F37, D-15). This file is the DELTA: the replacement text for every stage, case, gate row, visual state and Boris
check those answers touch, and one line for everything else. Done = a builder of S2, SM-b, S3, S4a, S4c, S5a, S5b or S6
can write his packet from the ruling plus this file with no design choice left to him.
In one paragraph:
- TA1. A BPM-synced clip runs at exactly the tempo's rate. While it is within a tenth of a beat of the bar it is HELD
  there by itself (a correction that never lasts longer than a tenth of a beat of music; no seek). When it is further
  off it PLAYS ON at exactly the rate and is sought ONCE, on the next "1", to the nearest place the bar gives. No speed
  law that runs for seconds exists any more. His reliability question becomes "how often does the picture cut": SM-a
  stays the first stage and gains one number, NC (the cuts a clip would have shown on each track), with N1's bar.
- TA2. `paused` is a saved field of the CLIP. Only his pause and play buttons change it. A fire never changes it. Cmd+Z
  never changes it. A take or a routine never changes it. A paused clip shows a pause mark in its grid cell.
- TA3. Timeline Speed runs 0 .. 10 on a two-piece law: the lower half of the travel is 0 .. 2 (as today's lower half),
  the upper half 2 .. 10. x10 is measured on the player (three arms) beside X6 before the top is promised.

## 2 ESTABLISHED FACTS (only verified lines; INFERRED / ASSUMED are labelled where they are used, in section 3)

His words (BD; the text after "->" in that file is Harmony's):
- B1 BD:902 "71 b". The question as asked (boris-clarify-71-74.md:12): "B No slide: the clip plays on from where it is
  and cuts once, on the next "1", into time."
- B2 BD:904-905 "new clip plays, the old clip is permanently paused and if comp is saved, it is saved as paused. the
  clip is now paused until the user changes that setting."
- B3 BD:907 "74 b". As asked (boris-clarify-71-74.md:20-22): "Speed in Timeline mode goes from 0 to 4 today. Resolume's
  goes to about 10." / "B Go to 10."
- B4 BD:763 "9 b" (question 9 B as recorded: the app keeps nudging it back by itself -- Harmony's text).
- B5 BD:748 "when you pause a clip and then fire it, it stays, paused"; BD:806 "stays in layer strip paused".
- B6 BD:764-766 "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo,
  etc. if it is  the layer strip, cmd-z does not affect it. If a clip is triggered and plays, it is not affected."
- B7 BD:844 "42 default" (a fired clip starts at its beginning, every time -- Harmony's text).
- B8 BD:855-856 "If I tapped the tempo again, to set the tempo, the time does not change. If I press re-sync, then it
  does re-sync and that changes by how far off the beat we are."
- B9 BD:859-861 "If we are shifted forward or back, everything that is connected to BPM shifts forward or back. I mean
  everything. If the user twist the knob in real time, or triggers a clip, that is not affected unless it's set to be
  quantized".
- B10 BD:918-920 "just the bpm timer. If most of the show is set up to BPM, and the BPM goes stop, the BPM goes to zero
  nothing moves. If there are clips that are not BPM based, then they play just as they were and are unaffected".
  The question as asked (boris-clarify-111.md:9-11), default A: "The beat. Pause holds the beat where it is, and
  everything that follows the beat waits with it. Play lets it run again. Stop puts it back on the "1" and holds it
  there."
- B11 BD:774 "Nothing else." (which failures show a message); BD:777 "display not enough HDD space to record."
The adoption (PL:1075-1089): (1) no slide is built, S4c builds the one-cut path as the design, "The 6 % slide law, its
pull-in bar X4 and the "eases onto the beat" text are void", SM-a still runs first, N1's bar and O3's STOP-AND-ASK
stand; (2) F38 and R16's second sentence are overruled, C is taken plus "persists until he changes it and is written
to the show file"; (3) the range goes to 10. This delta is owed before the packets of S2, SM-b and S4c.
The pin, read by me (VERIFIED):
- E1 src/model/Clip.h:237-240: `playing`, `playheadPosition`, `beatsPlayed`, `hasBeenTriggered` are Relaxed runtime
  fields. No field `paused` exists.
- E2 src/model/Clip.cpp:65-71 writes `transportMode`, `loopMode`, `speed`, `reverse`, `inPoint`, `outPoint`; :213-226
  loads them, `speed` with NO clamp (:217-218). A grep of Clip.cpp for `playing` and for `paused` finds nothing: no
  play or pause state is in a show file today.
- E3 src/model/Layer.h:548-558 `applyActivationTail`: playhead to the in point, beat count 0, and `playing = true` only
  on a NEW activation of a clip with `hasBeenTriggered` false.
- E4 src/render/ClipTransportSync.h:33-41 `pushIntent` reads `clip.playing` once; :45-73 `writeBack` stores the playhead,
  compare-exchanges `playing`, and holds the range branch RA-4 removes.
- E5 src/render/Renderer.cpp:1632-1638: a video player is looked up by CLIP id (`videoPlayers_.find(clip->id)`); the
  sequence branch likewise (:1694-1697). A clip keeps its own player and clock while another clip plays on its layer.
- E6 Renderer.cpp:1657-1670: BPM Sync pushes `videoBeats / beatDivision` from its own `featureBus_.read()`; Timeline
  pushes `effectiveClipSpeed(clip->speed, masterSpeedVal, false)`; :1705 the sequence the same. Renderer.h:290-293:
  `effectiveClipSpeed` = speed x master speed when not synced. src/media/VideoPlayer.h:81: `setSpeed` stores, no clamp.
- E7 src/media/VideoPlayer.cpp:354-359 `seekTo` is a request (two atomic stores); :361-375 the frame that consumes it
  sets the clock to the target and does not advance (RU V3). :395-448 `advanceTransport`: the clock moves by
  dt x speed x direction; the comment after :448 says a negative speed "runs the clock down too" and a change of
  direction is a generation bump. :498-506: `contentFrameSec = frameDur / |speed|`, "speed 0: no next frame".
- E8 src/ui/ClipInspector.cpp:78-80: the Speed slider is a ResettableSlider-style setup with range 0 .. 4, step 0.01,
  default 1; :106-113 the two speed buttons halve (floor 0.01) and double (cap 4). :37-53: back / pause / play write
  `clip_->reverse` and `clip_->playing`.
- E9 src/ui/LayerStrip.cpp:366-392: four buttons; pause writes `clip->playing = false`; the fourth (">|") writes
  `reverse = false`, `playing = true` and doubles the speed up to 4. :396-411: the S fader runs 0 .. 1, default 0.25,
  speed = value x 4. :721 and :841-849: it is set from `clip->speed / 4` on refresh and on the timer.
- E10 src/ui/ClipCell.cpp:151-166 (border: selected white, active cyan), :190-197 (an "L" in the top-right corner for a
  locked clip), :204-215 (a red "!" in the top-left for a missing file). src/ui/DeckView.cpp:311: the grid's setters
  are compare-before-set.
- E11 A grep of src/api/ApiServer.cpp for `speed` finds nothing: no REST route reads or writes a clip's speed at the
  pin (RU V7: `/api/set_clip_param` takes only `fitMode`).
- E12 docs/claude/pitfalls.md entry 62 (read whole): reverse video comes from a decode-thread GOP cache; a direction
  change is a generation bump; "a GOP that does not fit is a sliding window, never a stall or a black frame (the reader
  holds, `video_late_frames`)".
From the fact sheets and the plan (each row's VERIFICATION did not overturn it):
- R1 facts-resolume-transport.md Q2 and VERIFICATION line 120: Resolume's Timeline Speed slider is non-linear, "you have
  more precision in the values between 0 and 2", "When you go towards 10, it ramps up more quickly" -- CONFIRMED. The
  curve itself and the minus / plus step are NOT DOCUMENTED (PL:74-78, R2).
- R2 PL:104-105 (P3, his screenshots 1 and 8): in Timeline the thumb of "1" sits at about 25 % of the travel.
- R3 facts-resolume-transport.md VERIFICATION refutation 3 (RU V18): in Resolume pause is the clip's own.
RULING-VERIFIED rows this delta rests on: V3 (a seek costs its frame), V6 (which files ctest links; the real players
are linked headless), V9 (test mode has no tracker), V10 (60 .. 200 BPM), V12 (the count and the bar's beat move on two
events in Auto), V13 (a Tap), V14 (the nudge is applied once, before the snapshot is published), V26 (ping-pong leg).
NOT VERIFIED BY ME: section "not_verified" at the end of 9.

## 3 ITEMS

The real-time rules hold in every item: nothing touches the audio callback or the analysis thread (S4t is the ruling's,
unchanged); the hold, the cut, the "1" detector and the anchor are plain GL-thread arithmetic and one GL-thread record;
`paused` and its place are `Relaxed<T>` model fields (Pitfall 63); the render thread never waits; NO new mutex.

### TA1 NO SLIDE (71 B)

VERIFIED: B1, B4, B7, B8, B9, B10, E6, E7; RU V3, V12, V13, V14.
THE LAW (amendment TA-1; replaces PL:415-445 D2-8's trim and its fallback text, RA-2's "met" row, RA-9 whole).
e = the clip's place minus the music's, in beats, wrapped to half the lock period -- `barLockErrorBeats`, unchanged
(PL:417-418; `lockPeriodBeats` unchanged, RA-12, RA-8). The music's bar is read through the ANCHORED reader, always
(TA-3). Three bands, decided every frame from this frame's e alone (no stored offset, as the chain's law):
- IN TIME, |e| <= `kGridDeadbandBeats` = 0.05: the speed pushed is EXACTLY the rate (`barSyncRate x syncSpeed`).
- HELD, 0.05 < |e| <= `kOutOfTimeBeats` = 0.10: this frame's advance is corrected so that the clip lands on the grid:
  the speed pushed is the rate x `gridHoldFactor`, the factor = 1 - e / (the frame's length in beats), never below 0
  and never above 2. A clip 0.10 beat ahead stands for 0.10 beat of music (50 ms at 120, 100 ms at 60); one 0.10 beat
  behind runs double for that long. Never a seek, never backwards (E7: a negative speed is a generation bump).
- OUT OF TIME, |e| > 0.10: the clip PLAYS ON at exactly the rate. On the frame in which the music's "1" is crossed
  ONE `seekTo` is issued, to the nearest place the bar gives (`catchUpTarget`, RA-8), one render frame ahead (the frame
  a seek costs, E7). Nothing else is done until that "1".
"OUT OF TIME" is therefore: more than a tenth of a beat off the bar -- the number the ruling already calls "in time"
in X2, TR20 .. TR22 and PL:443.
Pure functions, all in src/render/ClipBarGrid.h (built at SM-b):
- `float gridHoldFactor(double errorBeats, double frameBeats) noexcept;` (replaces `barLockTrim`; the constants
  `kBarLockTauBeats` and `kBarLockMaxTrim` go; `kBarLockDeadbandBeats` 0.03 becomes `kGridDeadbandBeats` 0.05).
- `bool outOfTime(double errorBeats) noexcept;`
- `bool barOneCrossed(const BarAnchor& before, const BarAnchor& now, uint32_t countBefore, uint32_t countNow, bool
  resyncLanded) noexcept;` -- true when (count - anchor) div 4 rose between two frames with the same anchor, or when a
  Resync landed in this frame's snapshot (TA-2). It is worked out from the COUNT, never from a phase wrap (Pitfall 42).
- `double catchUpTarget(...)` -- the ruling's (RA-8), built here at SM-b instead of S4e; S4e's Catch Up calls the same.
In src/render/ClipTransportSync.h (S4c): `TempoView` gains `double frameSeconds; bool barOneCrossed; bool beatRunning;`.
`transportSpeed` multiplies by the hold factor (RA-5's "x (1 + trim) from S4c on" reads "x the hold factor from S4c
on"). `template <class Player> bool applyCut(const Clip&, Player&, const Intent&, const TempoView&) noexcept;` issues
the one seek and returns whether it did; `syncMedia` calls it once per branch, after `setRange` and the restart, before
the advance. `Renderer` keeps, beside the anchor, last frame's count: ONE record for the show, updated once per frame
where `frameSnap_` is read (RA-3's shape, Pitfall 38), never per clip.
OFF (the speed is exactly the rate, no seek is asked): the ruling's list (PL:423 -- not wanted, held, paused, tempo 0,
tracker not LOCKED, length not known, speed step 0), a BeatLoopr loop that is on (RA-8), a seek still pending, and
`beatRunning` false (below).

WHY 0.05 AND NOT THE OLD 0.03 (INFERRED, arithmetic): the bar position is published once per 512-sample hop, 10.7 ms =
0.0356 beat at 200 BPM (RU V10). A correction that lands the clip exactly on the grid at one instant of that staircase
leaves the measured e anywhere inside one hop afterwards; a deadband narrower than one hop would correct again and
again above 168 BPM. 0.05 is wider than the largest hop. TL-U45c (unchanged text) is the guard: if it fails at 0.05 the
builder STOPS and reports; he does not tune.

WHAT EACH EVENT DOES (amendment TA-2; replaces PL:513-517's "SLIDES, never jumps" and D-14):

| event | cut? | when |
|---|---|---|
| a fire that lands on the "1" (Snap Bar, 2 Bar, 4 Bar: RU V17) | no | -- |
| a fire off the "1" (Snap off or Beat) | yes, if more than 0.10 beat off | it starts at its beginning (B7), plays on, ONE cut on the next "1" (at most 4 beats later) |
| a playhead drop (the release of a drag or a press) | yes, if more than 0.10 beat off | plays on from the drop; ONE cut on the next "1" |
| a Tap | only if the beat it sets is more than 0.10 beat from the old one | ONE cut on the next "1"; later Taps of the same run move the beat by less and are held (M5 measures: at most 2 cuts over eight Taps) |
| a Resync | yes, if off | AT ONCE: the press is the "1" (question 121, default A) |
| a tempo value, BPM minus / plus, /2, *2 | no | the rate follows at once; the phase is continuous (PL:514, R10) |
| a nudge of up to 0.10 beat (50 ms at 120) | no | held: the clip is on the SHIFTED beat within 0.10 beat of music (B9) |
| a nudge amount that jumps by more (a show opened with its own amount) | yes | ONE cut on the next "1" |
| Auto moves its own "1" (the anchor changes, RA-3) | yes | 8 beats later the anchor follows; ONE cut on the next "1" after that |
| Auto realigns the beat by more than 0.10 beat (after a breakdown) | yes | ONE cut on the next "1" |
| Auto's ordinary small realigns, a wobbling tempo | no | held |
| BeatLoopr Off without Catch Up; Speed stepped up from 0; +4 bars, x2, /2, a moved out point | yes, if off | ONE cut on the next "1" |
| the BPM timer paused or stopped (B10) | no | the clip STANDS on its frame (speed 0); play button stays lit; its pause is not touched |
| the BPM timer started again | after a pause: no; after a stop: as a Resync | the start after a stop is a "1" |

THE BPM TIMER (B10; the tempo row's delta is not written yet, plan-nudge.md:818-820). This lane's contract is ONE input,
`TempoView::beatRunning`. S4c builds it as constant true. When the nudge lane's tempo row lands, ONE line changes: the
line in `Renderer` that fills it from the snapshot. If that lane publishes a tempo of 0 instead, the ruling's own OFF
row (tempo 0 -> rate 0) already stands the clip still and no line changes. Either way: a BPM-synced clip stands, a
Timeline clip is untouched (`transportSpeed`'s Timeline branch never reads `TempoView`), `Clip::paused` is not written.

WHAT KEEPS A CLIP IN TIME BETWEEN CUTS (fork F-A1).
- CHOSEN: the hold above -- "a position worked out from the beat", applied only inside a tenth of a beat.
- RUNNER-UP: the rate follows the tempo and nothing else; a cut whenever the drift passes 0.10 beat (the ruling's
  fallback as PL:441-445 wrote it). It loses on B4: the rate is the tracker's tempo NUMBER, the beat is its PHASE, and
  in Auto the phase is corrected by detections the number does not carry. A tempo read 0.1 % off drifts 0.10 beat in
  100 beats: a cut about every 50 s at 120, twelve in ten minutes (INFERRED; D1 below measures it on his tracks). A
  picture that cuts every minute is not what B4 asks for. In Manual the runner-up would do (two clocks a few
  parts in a hundred thousand apart: a cut in tens of minutes, INFERRED) -- but one law for both modes is smaller than two.
- NOT CHOSEN EITHER: the position set from the beat on every frame with no band. It would follow every move of the beat
  at once -- a Tap, a moved "1" -- which is a cut NOT on the "1", against B1, and it would copy the hop staircase into
  the picture (RU's TL-U45c exists because of that).
THE CUT'S TARGET (fork F-A2). CHOSEN: the nearest place in time (never more than half the lock period away: 2 beats).
RUNNER-UP: after a FIRE, the clip's beginning. Loses as a rule: it makes a fire and a drop two laws, and "plays on from
where it is and cuts once ... into time" (B1) names no beginning. It is his to overturn: question 123.
THE READER (fork F-A3; amendment TA-3; replaces RA-2's rows O1 and O2 and RA-3's "S4c's verdict row says which reader
feeds it"). CHOSEN: the anchored reader, always. RUNNER-UP: the raw reader when R1 is at most 1 %. Under the slide a
one-beat misread for a fraction of a beat pulled the speed by 0.06 % (RU:319); under the cut the same misread across a
"1" is a visible jump of a beat and a second jump back a bar later. 1 % of reads is not zero. The cost of the anchor is
the ruling's own: when Auto moves its "1", a clip follows two bars later. R1 is still measured and reported (it is the
first number for side finding SF-2); it decides nothing in this lane.

HOW MANY CUTS A TEN-MINUTE TRACK MAY SHOW (amendment TA-4; adds to RA-1's SM-a, RA-2's X1).
SM-a is unchanged as a run (three of his tracks, Auto, ten minutes each, production mode, `/api/bpm` at 50 Hz, no
product code, FIRST). Its offline script gains a virtual 4-bar clip under the law above, started in time at the
anchor's start, and prints per track:
- N1 (unchanged; bar: at most 1 per 10 minutes on each track; over = O3, STOP AND ASK).
- NC = the seeks that clip would have been given. BAR: at most 1 per 10 minutes on each track. Over = O3, with the
  number in the sentence. NC is never below N1, so this bar is the tighter one; N1's is not loosened.
- D1 (INFO, no bar) = the cuts the RUNNER-UP of F-A1 would have shown. H1 (INFO) = the share of reads in which the
  hold factor is not 1. They say how much the hold does; they decide nothing.
- R1, X7: measured as the ruling says. X7's bar and row O7 stand.
The script's self-test gains three synthetic logs with their mutated copies (instrument first, RIG-RULES A2): a beat
step of 0.20 beat -> NC 1; a step of 0.08 -> NC 0; the bar's beat moved by one for 3 beats and back -> NC 0, N1 0.
SM-a is run ONCE MORE, same bars, on the first tree that carries both this lane and the nudge lane (that lane re-seats
the published bar fields, plan-nudge.md:777-779): a row outside the pin, run by Harmony for whichever lane merges second.
WHAT HE SEES AT A CUT: the picture jumps once, by at most half a bar of the clip, on the "1"; from that frame the
playhead meets a bar line each time the circle shows "1". Between cuts: nothing -- no speeding up, no slowing down
that lasts longer than a tenth of a beat.

THE INSTRUMENT (amendment TA-5; amends RA-1's SM-b, RA-2's bars). STILL NEEDED, unchanged: ClipBarGrid.h, BarLockProbe.h
and its ring, the four routes, the two shown-frame accessors and the seek-pending flag, `.harmony/probe-barlock.*`,
TL-L6, G-P1, the production-mode recipe, the 22 drops, every run M1 .. M8. CHANGED: the probe's branch calls
`gridHoldFactor`, `outOfTime`, `barOneCrossed`, `catchUpTarget` and issues the seek itself; a sample gains two fields
(the seek issued this frame, its target); `reader` stays a parameter of the route and defaults to anchored.

| id | ruling | now |
|---|---|---|
| N1 | at most 1 per 10 min per track | STANDS |
| R1 | at most 1 % for the raw reader; selects O1 / O2 | MEASURED AND REPORTED; selects nothing (TA-3) |
| NC | -- | NEW: at most 1 per 10 min per track (SM-a); confirmed on M3: the real clip's seeks on each track are at most 1 |
| X7 | tempo wobble <= 0.5 % per 60 s | STANDS, with O7 |
| X2 | from 36 beats after the fire, abs(e) <= 0.10 in >= 95 % of frames (M1, M2: >= 99 %); the fifth-minute clause | The same shares and the same clause, FROM 1 BEAT AFTER THE FIRE (M1, M2, M3 fire on a "1"). Tightened. |
| X3 | content rate per 2 s window within 2 % in >= 90 % of windows | STANDS (it now judges the hold) |
| X4 | the slide's pull-in within 34 beats | VOID with the slide. Replaced by X4c. |
| X4c | -- | NEW (M4, 22 drops): no seek between the drop and the next "1"; before it the speed pushed is exactly the rate; EXACTLY ONE seek, issued within 2 render frames of the "1"; the shown frame is within one content frame of the target within 3 render frames; from a quarter of a beat after it abs(e) <= 0.10 in every frame of the next 16 beats, and no second seek in them; 22 of 22 |
| X5 | no position step; Taps; Resync as X4; tempo value; M6 | RE-STATED (M5 on the tree that has S4t; M6): over the eight Taps at most 2 seeks, each on a "1", c unchanged, abs(e) <= 0.10 from 5 beats after the last Tap. The Resync: one seek within 3 render frames of the frame it lands in (default A of 121), abs(e) <= 0.10 from a quarter of a beat later. The tempo value 120 -> 128 -> 120: no seek, abs(e) <= 0.15 throughout. M6 (a 30 ms step, twice): no seek, abs(e) <= 0.10 within half a beat. |
| X6 | x8, x16 in four arms | STANDS. X6t added (TA3). |
| X8 | ten seeks on a "1": shown within 3 render frames, 10 of 10 | STANDS. It is now THE player bar of the design. |

THE DECISION TABLE (amendment TA-6; replaces RU:335-350). Two verdicts as before: one tracker row (O7, O3 or "tracker
met", the first that matches) and one player row (O6 beside the others; then O5, O8, O9 or "met").

| row | when | what is built | Harmony tells Boris |
|---|---|---|---|
| O7 | as the ruling | as the ruling | as the ruling |
| O3 | N1 above 1 OR NC above 1 on any track | STOP AND ASK. The named fallback stands (period 1 beat in Auto, the bar in Manual and after a Resync), built only on his word | "In Auto, on <track>, the picture of a synced clip would have cut N times in ten minutes because the app moved its '1'. Either clips stay on the beat in Auto and on the bar when you tap and Resync, or we fix the '1' first. Which?" |
| O1 | -- | FOLDED INTO "tracker met" (TA-3) | -- |
| O2 -> "tracker met" | N1 and NC met | S4c on the anchored reader | "When the app itself moves its '1' in Auto, a clip follows two bars later, with one cut on a '1'." |
| O6 | as the ruling | as the ruling | as the ruling |
| O6t | X6t fails at 10 in any arm | TA3 | TA3 |
| O5 | X8 fails, or X4c fails | STOP AND ASK; S4c is not built; the lane goes on without it | "One clean cut does not work on this player yet. A clip that is out of time keeps the tempo but stays off the bar until you fire it again (as Resolume), until we fix the player. OK for now?" |
| O4 | -- | VOID (its fallback is the design) | -- |
| O8 | M1, M2, M4 met; X2 or X3 fails in M3 only | as the ruling (STOP and report; the named candidates stand: a wider deadband in Auto; e averaged over one beat) | as the ruling |
| O9 | as the ruling | as the ruling | as the ruling |
| met | X2, X3, X4c, X5, X8 met | S4c as TA-1 | "Yes: measured over ten minutes on three tracks and at three tempos. A clip that is out of time plays on and cuts once, on the next '1', into time, and stays there." |

RA-11's last rule stands: if no row allows S4c by the time S6 is done, merge 2 goes without it and D-6, checks 13 .. 16
and TR20 .. TR22 wait.

Changes by file (no stage is re-designed by a builder): src/render/ClipBarGrid.h (SM-b: the four functions, the two
constants); src/render/BarLockProbe.h and the probe branch of Renderer.cpp (SM-b: the seek, the two sample fields);
src/render/ClipTransportSync.h (S4c: `TempoView`'s three fields, `transportSpeed`'s factor, `applyCut`); Renderer.h/.cpp
(S4c: last frame's count beside the anchor; the two `applyCut` calls; the one line that fills `beatRunning`);
.harmony/probe-bar-one.py (SM-a: NC, D1, H1); .harmony/probe-barlock.py (the re-stated bars).
RED-first cases and mutants: section 5.1 (TL-U41, U43 .. U49, U49j, U83, U88; MU-71, MU-72, MU-73, MU-76, MU-100,
MU-105, MU-106, MU-107). Gate rows: TR20, TR21, TR22, TR23, TR24, TR29, TR31 (c) in 5.3.

### TA2 PAUSE IS THE CLIP'S, AND SAVED (72)

VERIFIED: B2, B5, B6, E1, E2, E3, E4, E5, E8, E9, E10; R3.
THE FIELDS (amendment TA-7; replaces RA-7's first three bullets and its last, PL:366-400, PL F32, PL F38, RA-15's
"The layer's X ends its pause", DA-5's and DA-6's additions in RU:86-87).
- `RelaxedBool Clip::paused = false;` and `RelaxedDouble Clip::pausedAt = -1.0;` (the frame it is paused on, as a
  place 0 .. 1; -1 = none). Both are shared model fields: `Relaxed<T>`, a static_assert pin, one TSan case (Pitfall 63).
  `Layer::paused` is never created.
- THE SHOW KEYS: `"paused": true` and `"pausedAt": <number>`, written ONLY when the clip is paused. Absent = not paused:
  an old show opens with nothing paused, and a show with nothing paused is byte-for-byte what it was (no new key).
  Loaded: `paused` from the key; `pausedAt` from its key, clamped into [inPoint, outPoint]; `playheadPosition` is set to
  it so the strip and the Clip tab show the place before the clip is ever drawn.
- `Clip::playing` keeps the ruling's one meaning (this clip is running; false = it ended or was cleared).
  `hasBeenTriggered` is deleted, as the ruling says (AM-3; Pitfalls 2 and 7 are re-written at S6 to this rule).
- The render sync never reads a model field for it (RA-7's second bullet STANDS word for word): `pushIntent(clip,
  player, bool paused)`, `Intent { wanted, held, paused }`. src/model/TransportPause.h keeps its three functions with
  the ruling's signatures; their bodies are answer C plus his sentence:
  `pausedFor(layer, clip)` = the clip's `paused`. `setPaused(layer, clip, on)`: on -> `paused` true and `pausedAt` =
  the clip's present playhead; off -> `paused` false and `pausedAt` -1. `fireRestarts(layer, fired, sameClip)` = the
  fired clip is not paused.
WHAT SETS AND CLEARS IT. Only these, all on the message thread, all through `setPaused`:
- the strip's pause (sets) and its play, play-backwards and fourth button (clear) -- they act on the layer's PLAYING
  clip (`Composition::playing(i)`, Pitfall 67);
- the Clip tab's pause (sets) and its play and play-backwards (clear) -- they act on the SHOWN clip, whether or not a
  layer plays it; the Clip tab's pause is never greyed (PL F32 is overruled);
- a key or a pad bound to one of those buttons (it is his hand);
- the test route `POST /api/debug/clip_pause {layer, column, paused}` (test-server build; it replaces the plan's
  `/api/debug/layer_pause`).
NOTHING ELSE writes it: no fire, no column fire, no clear of a layer, no Eject, no autopilot, no take, no routine, no
OSC, no production REST route, no Undo, no Redo. Lint TL-L4 gains: "`paused` and `pausedAt` are assigned only in
TransportPause.h, in Clip.cpp's loader and in the landing rule of TA-8; `setPaused(` is called only from the sites
S0's table lists, count pinned".
WHAT EACH ACTION DOES (replaces the table of PL:376-386):

| action | the fired / acted clip is not paused | it is paused |
|---|---|---|
| fire the clip the layer plays | restarts at its beginning (reversed: its end), plays | NOTHING moves: it stays on its frame; no stamp is minted (B5) |
| fire another clip on that layer | it starts at its beginning and PLAYS, whatever the first clip's pause (B2); the first clip keeps `paused` and its frame | it becomes the layer's clip and SHOWS ITS PAUSED FRAME, not playing; no stamp; `playing` true, the beat count 0 |
| a column fire | each layer as the two rows above | each paused clip of the column appears on its frame; the others start |
| a queued (quantised) fire | on its beat, then as above | the same, on its beat |
| autopilot, a routine, a take replay, REST, OSC, MIDI fire | as above (the same two handlers, lint TL-L4) | as above: it appears paused |
| pause (strip or Clip tab) | `paused` true, `pausedAt` = its playhead | -- |
| play / play backwards | sets the direction | sets the direction, clears `paused`: it runs on from its frame |
| the layer's X | clears the layer | clears the layer; THE CLIP STAYS PAUSED (B2: "until the user changes that setting"); fired again it appears on its frame |
| a drag or a press inside in..out | the ruling's scrub and hold | the scrub moves the frame it is paused on: `scrubClip` also writes `pausedAt`; it stays paused |
| Play Once and Eject | ejects at its end | never ends (TL-U71, unchanged) |

THE TAIL (amends PL:394-396). For a fire where `fireRestarts` is true: the ruling's tail (playhead to the start,
`playing` true, the stamp). Where it is false: `playing` true and the beat count 0 only -- no playhead write, no stamp.
`triggerWillAutoPlay` reads: the target exists, is not playing, and is not paused.
THE FRAME (fork F-P1). The player belongs to the clip (E5), so inside a session a paused clip's own player still stands
on its frame when the clip is fired again. That is not enough after a show is opened, after the player was re-opened, or
when the media was not open yet: so the model is the authority while a clip is paused. In ClipTransportSync.h:
`template <class Player> void holdPausedPlace(const Clip&, Player&, const Intent&) noexcept;` -- when the intent is
paused and `pausedAt` is 0 or more and the player's place differs from it by more than half a frame, `seekTo(pausedAt)`.
Called once per branch of `syncMedia`, after `setRange` and before the advance. A paused player never moves its clock
(TL-U68), so this asks for at most one seek per (re)opened player. `writeBack` is not changed for it.
RUNNER-UP: keep the place only in `playheadPosition`. Loses: `writeBack` stores the PLAYER's place on every sync (E4),
and a player that is not open yet answers 0 -- the saved place would be overwritten before any seek could land.
SECOND RUNNER-UP: save only `paused`; a re-opened show's paused clip shows its first frame. It is the smaller change
and it is offered to him as question 122's B (then the key `pausedAt` is not written: one line of Clip.cpp).
UNDO (amendment TA-8; replaces RU:86 "the strip fields gain the layer's pause", RU:87 "`restoredForUndo` also returns
the layer not paused", PL:575's "layer's `paused`", TL-UC17's added clause). `paused` is now a saved field, so every
value copy of a Clip carries it -- a command's snapshot included. Rule: an Undo or a Redo that lands on a cell holding
the SAME clip id leaves `paused` and `pausedAt` as the cell has them, whether the cell is live or idle (the ruling's
landing cases 3 AND 4, RT:440-447): `TransportState` gains the two fields and the idle landing copies them from the
cell's present clip. A cell that was empty (Undo of a delete, of a removed layer) gets the clip as it was removed,
pause included -- that brings a thing back, it does not change a setting. Duplicate Deck and a Duplicate of a clip copy
the pause (it is a setting, like Speed).
TAKES AND ROUTINES. `paused` and `pausedAt` are not part of PerfState: a take neither records nor restores them, a
routine's restore and replay never write them. A take or a routine that fires a paused clip gets the paused clip: it
appears on its frame (the table's fifth row). S0 already owes "whether PerfState's restore or a routine writes
`playing` directly" (PL:620); the answer cannot touch the pause, which no longer lives in `playing`.
THE GRID CELL (a STATE display; B11 is not touched). Every cell whose clip is paused shows a pause mark -- playing on a
layer or not, in whatever deck is shown. `void ClipCell::setPaused(bool)` (compare-before-set, as its other setters,
E10); DeckView's refresh sets it from the clip. The mark is DRAWN (two filled bars on a dark backing, never a text
glyph: Pitfall 6), in the lower-left corner of the thumbnail above the name bar (top-left is the missing-file "!",
top-right the lock "L", E10). No timer is added: the cell repaints only when the bit changes (Pitfalls 57, 59), and a
refresh decodes nothing (Pitfall 51). Its look goes through VG-1 (states P4, P5).
A layer's strip shows the playing clip's pause (pause lit); the Clip tab the shown clip's.
Changes by file: model/Clip.h (the two fields; `TransportState`), model/Clip.cpp (the two keys), model/Layer.h (the
tail takes `fireRestarts`), model/TransportPause.h (new), the landing rule's file (S0 names it), render/
ClipTransportSync.h (`Intent`, `pushIntent`, `holdPausedPlace`), Renderer.cpp and the compositor call sites (the bool;
the call), ui/LayerStrip.cpp and ui/ClipInspector.cpp (the buttons; lit state from the model on the timer, Pitfalls
41, 59), ui/ClipCell.h/.cpp, ui/DeckView.cpp, binding/BindingManager.*, MainComponent.cpp (`triggerWillAutoPlay`),
api/ApiServer.cpp (`paused`, `pausedAt` per CLIP in `/api/composition`; the test route; `transport_buttons`).
RED-first cases and mutants: 5.1 (TL-U60 .. U64, U69, U80, U81, U82, U84, U86; MU-60, MU-61, MU-62, MU-98, MU-101,
MU-102, MU-103, MU-109, MU-110). Gate rows: TR25, VG-1.

### TA3 SPEED TO 10 (74 B)

VERIFIED: B3, E2, E6, E7, E8, E9, E11, E12; R1, R2.
THE RANGE AND THE LAW (amendment TA-9; replaces PL:355-357 and F37, RA-15's "double up to 4 in Timeline", D-15).
- `Clip::speed` in Timeline: 0 .. `kTimelineSpeedMax` = 10. Default 1.
- The slider's law (fork F-S1). What Resolume documents (R1): non-linear, finer between 0 and 2, quicker towards 10.
  What it does not: the curve. What his screenshots show (R2): "1" at a quarter of the travel. CHOSEN: two straight
  pieces -- the lower half of the travel is 0 .. 2, the upper half 2 .. 10. So "1" sits at a quarter (R2), "2" at the
  middle, and every speed up to 2 sits exactly where today's 0 .. 4 fader has it (E9: value x 4). RUNNER-UP: JUCE's
  skew with its midpoint at 2 (one call, no mapping of our own). Loses: it puts "1" at 37 % of the travel, against his
  screenshots, and moves every position he knows from today's fader. THIRD: linear 0 .. 10 -- loses to R1 (a tenth of
  the travel for everything below 1).
- Pure, in ClipBarGrid.h (SM-b): `float timelineSpeedFromTravel(double travel) noexcept;` `double
  travelFromTimelineSpeed(float speed) noexcept;` `float clampTimelineSpeed(float speed) noexcept;` (into 0 .. 10; not
  a number -> 1).
- The Clip tab's Speed slider (S5a): a `ResettableSlider` over the VALUE 0 .. 10 with that law as its mapping (a
  `NormalisableRange` built from the two functions), step 0.01, `setDefaultValue(1.0)`; the number box shows the
  value. Minus / plus: 0.1 (the ruling's step; Resolume's is not documented, request RQ-3 stands), stopping at 0 and 10.
- The strip's S fader in Timeline (S5b): still a `ResettableSlider` over the travel 0 .. 1, default 0.25; speed =
  `timelineSpeedFromTravel(value)`; shown = `travelFromTimelineSpeed(speed)` (E9's three sites). The fourth button
  doubles up to 10 (1, 2, 4, 8, 10) and, as today, plays forward (so it clears a pause, TA2).
- In BPM Sync nothing changes: nine steps (PL:342-350).
- A SPEED SAVED IN AN OLD SHOW: the key is `speed`, loaded as it is (E2). Every old value is a speed 0 .. 4 and is
  inside the new range: it plays exactly as it did. The thumb stands where it did for a speed up to 2 and lower than
  before above 2 (4 was the top; it is now at 62.5 % of the travel). No migration, no new key. The loader is not
  changed (a hand-edited value outside 0 .. 10 plays as it does today; the first touch of a control clamps it).
- REST: `/api/set_clip_param` gains `speed` (S4a; the ruling's list had `transportMode`, `clipBars`, `syncSpeed`,
  PL:336-337): the value passes `clampTimelineSpeed`; the reply carries the stored value. `/api/composition` reports
  `speed` per clip. No other route writes it (E11).
- The master speed still multiplies a Timeline clip's speed (E6) and is not clamped with it: 10 is the clip's own top.
WHAT x10 ASKS OF THE PLAYER (INFERRED from E7, E12; not run). Forward: the clock passes ten content frames per frame of
real time; the decoder cannot produce 300 frames a second of 1080p, so the ring's pick shows what it has and the
writer re-seeks when the clock leaves its look-ahead -- late frames and holds, more on a long-GOP file. Reversed: the
GOP cache is consumed ten times as fast; a 250-frame GOP lasts 0.8 s, and "a GOP that does not fit is a sliding window
... (the reader holds, `video_late_frames`)" (E12). Ping-pong: a turn is a generation bump (E7), every few seconds on a
short clip. A sequence only skips indices. None of this is settled by reading: it is measured.
THE MEASUREMENT ROW BESIDE X6 (amendment TA-10; adds to RA-1's M7 and RA-12).
- M7t: Timeline mode through the SAME probe branch and log, the clip's `speed` 10 (set through the probe's record, as
  M7 sets the step), both fixtures, 60 s each, three arms: forward Loop on the ramp trimmed to out 0.8, reversed Loop,
  Ping Pong. It runs with M7, on the tree that has SR.
- X6t = X6's own numbers: content rate within 3 % of 10 over 4 s windows; peak frame time <= 50 ms. The player's late
  and hold counters are printed per arm (reported, no bar), as RA-12 says for M7.
- Row O6t: X6t fails in any arm -> the top is the highest of 10, 8, 4 that met its bar in every arm (8 is judged by
  M7's x8 arms; 4 is today's and is not re-judged). It is data (`kTimelineSpeedMax`); S4a, S5a and S5b do not wait.
  Harmony tells Boris: "The player does not play cleanly at ten times speed. The slider stops at N for now. Leave it
  there, or fix the player first?" If X6t is met: nothing to tell.
RED-first cases and mutants: 5.1 (TL-U85, TL-U87, TL-U54; MU-104, MU-108). Gate rows: TR32, VG-2 states W23, W24, bar C17.

### TA4 THE CHANGE LIST

"Old" is quoted in short from the ruling (RU) or, where the ruling left the plan's text standing, from the plan (PL).
"New" is the replacement, or the place in this file that holds it. A row not in these tables STANDS.

A. Amendments RA-1 .. RA-17: 5 STAND (RA-6, RA-10, RA-11, RA-14, RA-17), 9 AMENDED, 2 REPLACED, 1 VOID.

| id | verdict | old | new |
|---|---|---|---|
| RA-1 | AMENDED | SM-a prints N1, R1, X7; the probe's branch calls `barLockTrim`; M7 has four arms | SM-a also prints NC, D1, H1 and is run once more with the nudge lane (TA-4); the branch calls the functions of TA-1 and issues the seek, the sample gains two fields (TA-5); M7t is added and the route's record gains `timeline` (TA-10) |
| RA-2 | REPLACED | X1 .. X8 and the table O1 .. O9 | the bar table of TA-5 and the decision table of TA-6 |
| RA-3 | AMENDED | "S4c's verdict row says which reader feeds it"; TL-U49a | the anchored reader, always (TA-3); `barOneCrossed` beside it; TL-U49a stands |
| RA-4 | AMENDED | TL-U46 "within 0.03 beat ... the trim is never below -0.06 nor above 0.06" | TL-U46 as 5.1; everything else of RA-4 stands |
| RA-5 | AMENDED | "x (1 + trim) from S4c on"; `TempoView { bpm; showBarBeats; clockLocked; }` | "x the hold factor from S4c on"; `TempoView` gains `frameSeconds`, `barOneCrossed`, `beatRunning` (TA-1) |
| RA-7 | REPLACED | `RelaxedBool Layer::paused`; answers A / B / C; "The layer's X ends its pause" | TA-7, TA-8. Its second bullet (the sync takes a bool) and its fourth (the fit carries the playhead through `scrubClip`) STAND; the fourth gains "and `pausedAt` with it" |
| RA-8 | AMENDED | "Off without Catch Up: the clip plays on from where it is and the lock, if one is built, slides it"; `catchUpTarget` built at S4e | "... plays on from where it is and, if it is out of time, cuts once on the next '1'"; `catchUpTarget` is built at SM-b and S4e calls it |
| RA-9 | VOID | the slide's numbers, said truthfully | nothing: no slide exists. Its last sentence ("If he answers 71 with B, S4c builds the jump when X8 is met") is what TA-6's "met" and O5 rows now say |
| RA-12 | AMENDED | X6 has four arms | and X6t has three (TA-10) |
| RA-13 | AMENDED | TR21 (b), TR25 | 5.3 |
| RA-15 | AMENDED | "The layer's X ends its pause"; the fourth button "double up to 4 in Timeline" | the X leaves the clip's pause alone (TA-7); "double up to 10" (TA-9) |
| RA-16 | AMENDED | TR24 as the plan ("within 8 beats abs(e) <= 0.10") | TR24 as 5.3; the rest stands (the clip reads the shifted beat with no line changed) |

B. Sections 0.1 .. 0.3 of the ruling. 0.1: 37 of its 40 rows stand; AM-3's row reads "the pause survives a fire (TA-7)";
DA-1's row reads "the hold and the one cut (TA-1), measured first"; DA-5's and DA-6's additions are struck (TA-8).
0.2 (the first Bars number) STANDS whole. 0.3: its "FIRST" and "SECOND" stand; "THE ANSWERS" is TA-6's table.

C. Stages: 5 STAND (S4t, SR, S2b, S3h, S4d), 12 CHANGE -- section 4.

D. Unit cases (103 in the ruling): 85 STAND, 18 are RE-STATED (TL-U36's clause, U41, U43, U44, U45, U45b, U46, U47, U48,
U49, U54, U60, U61, U62, U63, U64, U69, TL-UC17), TL-U49j becomes unconditional, 9 are NEW (TL-U80 .. U88). Lints: 8
stand (TL-L4 gains a clause), 1 new (TL-L8). Texts: 5.1.

E. Live rows (30 at merge 2): 23 STAND, 6 RE-STATED (TR20, TR21, TR22, TR23, TR25, TR29), TR31 loses one number in (c),
1 NEW (TR32); outside the pin TR24 is re-stated. Texts: 5.3.

F. Visual states and bars: VG-1 P2's text and two new states (P4, P5); VG-2 W18's text, two new states (W23, W24), C13
re-stated, C17 new; every other state and C1 .. C12, C14 .. C16 STAND. Texts: 5.5.

G. Differences and Boris checks: D-6, D-7, D-14, D-15 are re-written, D-16 is new, 11 stand; of checks 1 .. 23, 14
stand, 9 are re-written (4, 5, 6, 7, 12, 14, 15, 16, 19), 4 are new (24 .. 27). Texts: section 6.

H. Questions: 71, 72, 74 are ANSWERED; 73 stays held back behind FM-8; RQ-0 .. RQ-3 stand. New: 121, 122, 123.

I. Harmony's decisions H-T1 .. H-T9: H-T7 is CLOSED (he answered). H-T1 is settled with the nudge adoption's H-10
(section 4, S4t). The other seven stand.

J. Risks K-1 .. K-11: K-4 and K-5 are CLOSED (their questions are answered); K-9 now holds always, not only under O2;
the other eight stand. New ones: section 8.

## 4 STAGES + ORDER (the ruling's section 4 stands; a stage named here is changed ONLY as written here)

ONE lane, based on main. One builder context per stage. Every stage: its cases written first and shown RED by id on the
stage's base, then GREEN at its head. A builder never runs a live row, never launches the app, never gives a verdict.

| key | change | owns (added to the ruling's list) | proves (added or re-stated) |
|---|---|---|---|
| S0 | ADDS to its table | -- | the file that holds the landing rule and its idle case (TA-8); every call site that will call `setPaused` (count for TL-L4); which snapshot field changes when a Resync lands (candidate `resyncBarOrigin`; `trackerRequestSeq` also changes on a Tap); whether `/api/inject_features` writes `totalBeatCount` and `trackerRequestSeq` (the anchored reader in test mode needs both; if not, SM-b's tempo route gains them); every site that scales or clamps `Clip::speed` by 4 (VERIFIED so far: E8, E9; bindings and OSC not read); the REST route that saves a show to a named path, if any (TR25 f); the dump route that reports a grid cell's state; whether a player is ever closed while its clip stays in the grid |
| SM-a | ADDS | .harmony/probe-bar-one.py | NC, D1, H1 and the three synthetic logs with their mutated copies (TA-4). THEN HARMONY: three tracks; N1, NC, R1, X7; the tracker row |
| S4t | STANDS | -- | ORDER with the nudge lane (its adoption's H-10 asks this delta to rule): one builder at a time in BPMTracker.cpp. If the nudge lane's S1 is built when this lane reaches S4t, S4t is built on top of it; if not, S4t is built here on main and the nudge lane's S1 takes it in as its step 0. TL-U70 is registered once, by whoever builds S4t. M5 runs on a tree that has S4t. |
| SR | STANDS | -- | -- |
| SM-b | CHANGED | the same files; ClipBarGrid.h holds the functions of TA-1 and TA-9 and `catchUpTarget` | TL-U32, U33, U34, U40, U41 (re-stated), U42, U43 (re-stated), U49a, U49j, U83, U85; TL-L6. THEN HARMONY: M1 .. M8 and M7t; X2, X3, X4c, X5, X6, X6t, X8; the player row |
| S1 | ONE CLAUSE STRUCK | -- | TL-UC17 without "the layer's pause"; the strip's field list and `restoredForUndo` gain nothing (TA-8) |
| S2 | RE-STATED (TA-7, TA-8) | + model/Clip.cpp, the landing rule's file, ui/ClipCell.h/.cpp, ui/DeckView.cpp; the test route is `clip_pause` | the chain's 15 S2 cases + TL-U60 .. U64, U69 (re-stated), U80, U81, U82, U84; TL-L1 (S2 form), TL-L1b, TL-L4 (with its clause); G-U4. Then Harmony: FM-1 |
| S2b | STANDS | -- | -- |
| S3 | ONE CASE ADDED | -- | TL-U86 |
| S3h | STANDS | -- | -- |
| MERGE 1 | CHANGED | Harmony | probe-transport at `EXPECTED_ROWS=18` (unchanged count; TR25 as 5.3); VG-1 with P4 and P5 |
| S4a | ADDS | ApiServer.cpp: `speed` in `set_clip_param` and in `/api/composition` | the route calls `clampTimelineSpeed` (TL-U85 is its teeth; TL-L7 gains "`speed =` under src/api appears once, with `clampTimelineSpeed(` on that line") |
| S4d | STANDS | -- | -- |
| S4e | ONE CLAUSE | -- | TL-U76 unchanged; RA-8's sentence as TA4 A; `catchUpTarget` is called, not written |
| S5a | CHANGED | -- | + TL-U87; the three buttons write the SHOWN clip through `setPaused`; pause is never greyed |
| S5b | CHANGED | -- | TL-U54 (re-stated). Ends at VG-2 |
| S4c | RE-STATED (TA-1 .. TA-3) | the ruling's files | TL-U44 .. U49 (re-stated), TL-U36's clause, U88; TL-L5, TL-L8; probe-video whole. Built when the tracker row is "tracker met" and the player row is "met" (O6 and O6t hold nothing back). Then Harmony: M2 at one tempo and M4 again on the lane build, same bars |
| S6 | CHANGED | -- | probe-transport at 31 rows; docs as the ruling plus: Pitfalls 2 and 7 re-written to the pause rule, performance-controls.md (pause is the clip's and is saved; the grid mark), rendering.md (the hold and the cut; Timeline Speed's law), CLAUDE.md "Transport state" paragraph |
| MERGE 2 | CHANGED | Harmony | `EXPECTED_ROWS=31` |

ORDER (unchanged in shape): S0 -> SM-a (Harmony) -> S4t -> SR -> SM-b (then Harmony: M1 .. M8, M7t, the verdict) -> S1
-> S2 (-> S2b) -> S3 -> S3h -> MERGE 1 -> S4a -> S4d -> S4e -> S5a -> S5b (VG-2) -> S4c -> S6 -> MERGE 2.
WHAT WAITS ON WHAT:
- this delta, ruled and adopted: the packets of SM-b, S2, S3, S4a, S5a, S5b, S4c, S6. NOT S0, SM-a's run, S4t, SR, S1.
- SM-a's run: three tracks from Boris (RQ-0; H-T2: none = BLOCKED). SM-a's script change: this delta.
- S4t: the nudge lane's S1 only in the sense above (never two builders in one file).
- SM-b: SR (M7 and M7t need the range), S4t (M5).
- S4c: the two verdict rows; S4a; S4e is not needed (`catchUpTarget` is SM-b's).
- S5a, S5b: S4a. They do not wait for X6t (the top is data).
- Questions 121 .. 123: nothing waits; the defaults are built; each answer's other branch is one line (section 7).
WHAT HARMONY RUNS HERSELF: everything the ruling lists (RU:569-572), with M7t beside M7, SM-a's second run with the
nudge lane, and her six mutants unchanged in number (MU-71 keeps its place, re-aimed in 5.1).

## 5 TESTS + GATE ROWS (pre-registered here; the ruling's section 5 stands except where this section speaks)

Harmony constraint (unchanged): RED arm = the frozen pre-lane main app for a live row, the stage's base for a unit
case; a flake verdict needs >= 5 runs per arm; a bar is met or reported, never loosened; a bar inside 4 x the measured
noise is BLOCKED until she waives it in writing. Three numbers here differ from the ruling's and none is a loosened bar:
0.05 beat in TL-U43, U45b, U46, U49 is the NEW law's deadband where 0.03 was the void law's (the live and measured bar
"in time" stays 0.10 everywhere); X2's start moves from 36 beats to 1 beat (tighter); TR31 (c)'s from 36 to 8 (tighter).

### 5.1 Unit cases (each a TEST_CASE whose name begins with its id; RED arm: the stage's base, by id, in the builder's log)

SM-b (RED arm: ClipBarGrid.h as a stub whose functions return 0 / false / 1):
- TL-U41 "grid hold: the factor is 1 inside 0.05 beat; between 0.05 and 0.10 it is 1 minus the error over the frame's
  beats, never below 0 and never above 2; beyond 0.10 it is 1"
- TL-U43 "a clip started 1.99 beats off the bar plays at exactly the rate until the next 1, is sought once there, and is
  within 0.05 beat (+ 1e-6) from the frame the seek lands on; a clip 0.08 beat off is never sought and is within 0.05
  beat inside 0.10 beat of music; the position steps nowhere but at that one seek"
- TL-U49j "the cut: asked only on the frame the bar's 1 is crossed and only when the clip is more than 0.10 beat off
  the anchored bar; the target is the nearest place the bar gives, one frame ahead; one seek and none on the frames
  after it; none while a seek is pending; a one-beat flip of the raw bar that lasts a fraction of a beat across a 1
  asks for none"
- TL-U83 "the 1 is crossed when the count passes a multiple of four from the anchor: a phase that wraps back without
  the count crosses nothing; a count that jumps two beats over a 1 crosses once; a Resync is a 1"
- TL-U85 "Timeline speed law: travel 0 is 0, a quarter is 1, a half is 2, three quarters is 6, the top is 10; the way
  back gives the same travel within 1e-6 for 0, 0.5, 1, 2, 4, 10; the clamp gives 0 for -1, 10 for 12, 1 for not a
  number"
S2 (RED arm: S2's base; the fakes are the chain's):
- TL-U60 "a fire of a paused clip that its layer plays changes nothing: no stamp, the playhead, the pause and its place
  stay"
- TL-U61 "another clip fired on a layer whose clip is paused: its stamp is minted, it starts at its beginning, the
  intent pushed is running; the first clip is still paused, on the same place"
- TL-U62 "a pause is never written back as ended: ten syncs of a paused clip leave its play state true, a OneShot at
  its last frame is not stopped, and play carries on from the same frame; a Loop clip paused beyond a moved out point
  keeps its playhead through ten syncs"
- TL-U63 [tsan] "a message-thread pause against the GL-side read of the clip's pause and of its place"
- TL-U64 "a column fire: a paused clip in the column becomes its layer's clip on its paused place, not running; the
  other layers start; clearing a layer leaves its clip paused"
- TL-U69 "strip and Clip tab: pause is lit while the clip is paused, play when it is not and its direction is forward,
  back when backward (the model's `reverse`, never the ping-pong leg); the Clip tab's pause is enabled for a clip no
  layer plays; each is set only when it changes"
- TL-U80 "the show file: a paused clip writes paused and its place; a clip that is not paused writes neither key and its
  text is what the stage's base writes; a file without the keys loads not paused; a place outside in..out loads on the
  nearer point; a loaded paused clip's playhead is its place"
- TL-U81 "an Undo and a Redo that land on a cell holding the same clip leave its pause and its place as the cell has
  them, on a live cell and on an idle one; an Undo that brings back a removed clip brings its pause; a duplicate copies it"
- TL-U82 "grid cell: the pause mark is painted exactly when the clip is paused, on a cell that plays and on one that
  does not; setting the same state again asks for no repaint"
- TL-U84 "a paused clip under a player that stands elsewhere (a fresh one at 0) is sought to its place once and never
  again; on the REAL VideoPlayer and the REAL ImageSequence the shown place is the paused place after the sync; a clip
  that is not paused is never sought by it"
S3: TL-U86 "a drag inside in..out on a paused clip moves the place it is paused on and leaves it paused; a press outside
does nothing".
S4c (the fakes are the chain's: a player whose seek is a request and whose wrap keeps the overshoot; the real
`pushIntent`, `transportSpeed`, `applyCut`, `writeBack` in the renderer's order):
- TL-U36's S4c clause reads "; in the held band it is that times the hold factor; in time, out of time or off it is
  exactly that".
- TL-U44 "off -- the speed pushed is exactly the rate and no seek is asked: a paused clip, a held clip, a hold cleared
  between pushIntent and the push, a tempo of 0, a tracker that is not locked, a length not known, speed step 0, a
  BeatLoopr loop that is on, a seek still pending; a tracker state that flips every frame asks for no seek off a 1"
- TL-U45 "a tempo change from 120 to 90 on a clip in time: no seek; the error stays inside 0.05 beat" | TL-U45b "a
  realign of 2 beats: no position step before the next 1; exactly one seek there; within 0.05 beat (+ 1e-6) after it"
  | TL-U45c UNCHANGED TEXT (it now judges the hold's deadband).
- TL-U46 "a trimmed loop, in 0.2, out 0.7, 4 bars, at 200 BPM through the real sync and a player that wraps inside its
  range: no seek is asked; from the second wrap on the error is within 0.05 beat (+ 1e-6); the factor is never below
  0 nor above 2"
- TL-U47 (REAL players, RA-4) "a reversed 4-bar clip meets its lines on the bar; a ping-pong stays in time through both
  turns with no seek; on the frame a clip is reversed the error is taken with the new direction; a reversed clip out
  of time is sought once, on the next 1, to the mirrored place"
- TL-U48 "after a drop the clip plays on from the drop point at exactly the rate: no seek before the next 1, a quarter
  of a beat later the position is the drop point plus a quarter of a beat; on the next 1 exactly one seek"
- TL-U49 "plus 4 bars, x2, /2, a moved out point and a speed step on a clip in time: no seek on that frame; if the
  change leaves it more than 0.10 beat off, exactly one seek on the next 1 and within 0.05 beat (+ 1e-6) after it"
- TL-U88 "the beat not running: the speed pushed is 0, no seek is asked, the clip's play state stays true and its pause
  is not written; running again it goes on from that frame; a Timeline clip's speed is the same either way"
S5a: TL-U87 "Clip tab, Timeline: the Speed slider runs 0 to 10 with 1 at a quarter and 2 at half of its travel; its
default is 1 and a right-click gives it; minus and plus step 0.1 and stop at 0 and at 10".
S5b: TL-U54 "layer strip: in BPM Sync the S fader has nine detents and the forward button steps one up; in Timeline it
runs 0 to 10 on the Timeline law with its default at a quarter of the travel, and the forward button doubles up to 10;
the lit state of back, pause and play follows the playing clip's pause and direction and is set only when it changes".
S1: TL-UC17 keeps the ruling's field list WITHOUT "the layer's pause".
LINTS. TL-L4 gains the clause of TA-7. TL-L7 gains the clause of S4a (section 4). TL-L8 (S4c; new): "`barLockTrim`,
`kBarLockMaxTrim` and `kBarLockTauBeats` appear nowhere under src or tests; in src/render `seekTo(` is called only
inside `applyCut`, `holdPausedPlace`, the Catch Up site and the Random site, count pinned by S0; `applyCut(` is called
exactly twice, both in `syncMedia`". RED arm: the builder shows once that a second `applyCut(` call turns it RED.
COUNTS. Cases: the ruling's 103 + TL-U49j + TL-U80 .. U88 = 113. Lints: 9.
MUTANTS (each alone on the lane head turns the named case RED). Re-aimed: MU-71 the cut's target is mirrored (the sign
of the error flipped) -> TL-U49j, TL-U43 (TR20, TR22). MU-72 no deadband -> TL-U41, TL-U45c. MU-73 the hold factor is
not limited (it goes below 0) -> TL-U41, TL-U46. MU-76 the cut is issued at once, not on the 1 -> TL-U48, TL-U49j
(TR20 ii, TR22). MU-60 a fire lifts the fired clip's pause -> TL-U60, TL-U64 (TR25 a, c, e). MU-74, MU-75, MU-61,
MU-62, MU-98 stand. VOID: MU-92 (its rule is gone: another clip plays). NEW: MU-100 the hold is off (the factor is
always 1) -> TL-U41, TL-U43's second clause (TR24). MU-101 the pause is the layer's (another clip fired there waits)
-> TL-U61 (TR25 b). MU-102 the pause is not written to the show -> TL-U80 (TR25 f). MU-103 an Undo lands the
snapshot's pause -> TL-U81 (TR25 h). MU-104 the Timeline law is linear 0 .. 10 -> TL-U85, TL-U87 (C17). MU-105 the cut
asks again on the frames after it -> TL-U49j (TR20 iv). MU-106 the cut reads the raw bar -> TL-U49j's last clause.
MU-107 `beatRunning` is ignored -> TL-U88. MU-108 the REST clamp stays at 4 -> TL-U85 (TR32). MU-109 the cell's mark
follows `playing` -> TL-U82 (VG-1 P4). MU-110 a paused clip under a fresh player shows its first frame -> TL-U84
(TR25 f).

### 5.2 Unit gates
- G-U1: the ruling's command and string, with N >= 121 (122 with S2b). Harmony constraint (unchanged): N is S0's
  recount of the list, never lower than 121; at merge 1 the pin is the count of the stages built by then.
- G-U3: the standing mutants + MU-60 .. MU-110 without MU-92. G-U4: `EXPECTED_TSAN_CASES` unchanged (TL-U63 is the same
  case on the clip's two fields). G-U1b, G-U2, G-U5, G-N1, G-P1: as the ruling.

### 5.3 Live rows -- `.harmony/probe-transport.sh`; final line `PROBE-TRANSPORT GREEN`; pins `EXPECTED_ROWS=18` (merge 1), `=31` (merge 2)
Launch rules, fixtures, the driver, "frame code", tolerance and flake rule: the ruling's. The driver posts the beat
count with the bar's beat (S0); every row that fires waits 12 beats after the driver starts (the anchor settles in 8).
"A step" = a read that differs from rate x dt by more than 3 frames. "The driver's 1" = the instant its bar position
passes 0.
- TR20 the_cut_on_the_one (was bar_lock_pulls_in). Driver at 60; the fitted ramp (rate 0.5); Quantize off; fire at
  driver bar position [1.20, 1.30]. Bars: (i) a read within 0.5 s of the fire has abs(e) >= 1.0 beat; (ii) from the fire
  to the driver's next 1: no step, and the content rate is in [0.48, 0.52]; (iii) exactly ONE step in the 0.3 s after
  that 1; (iv) from 0.5 s after it, five reads 0.5 s apart each have abs(e) <= 0.10, and no step in the next 16 beats;
  (v) a capture 1 s after the cut: its frame code is within 2 of playhead x 300. RED arm (frozen app, TR9's
  composition): (iii) fails -- no step -- and (iv) fails, abs(e) about 1.25 (INFERRED; if it passes the row is a
  guard). Teeth: MU-71 (iv), MU-74 (iv: it rests a beat off the bar), MU-76 (ii), MU-105 (iv).
- TR21 tempo_and_a_moved_beat. From TR20's state. (a) the driver steps to 90, phase continuous: no step in the next 8
  beats, every read has abs(e) <= 0.15, after them <= 0.10; the rate over 2 s is in [0.72, 0.78]. (b) the driver moves
  its phase by +0.4 beat (count and bar's beat unchanged): no step before its next 1; exactly one within 0.3 s after
  it; then abs(e) <= 0.10. (c) the driver moves its bar's beat by one (count unchanged): no step for 8 beats; then
  exactly one within 0.3 s after the next 1; then abs(e) <= 0.10. (d) the driver posts a Resync (S0's field) at bar
  position about 2: one step within 0.3 s, then abs(e) <= 0.10. RED arm: (a)'s rate stays about 1.0. Teeth: MU-76 (b),
  MU-106 (c: it cuts at once), MU-94 (c).
- TR22 after_a_drop (was bar_lock_after_a_drop). TR20's fixture, fired on a 1, left 16 beats. Strip scrub down 0.35;
  four reads over 1 s within one frame of 0.35; up at a driver bar position that leaves the clip about 1 beat ahead.
  Bars: (i) a read within 0.3 s of the up has abs(e) >= 0.8; (ii) no step until the driver's next 1 and the read just
  before it is the drop point plus rate x the time since the up, within 0.02; (iii) exactly one step within 0.3 s
  after that 1; (iv) five reads 0.5 s apart from 0.5 s later: abs(e) <= 0.10. GUARD (new routes). Teeth: MU-76 (ii),
  MU-71 (iv).
- TR23 lines_on_the_bar: the ruling's row; "wait 30 beats" reads "wait until 4 beats after the driver's first 1 after
  the fire".
- TR25 paused_fire (merge 1). A and B are two ramps of one row, told apart by pixels. (a) fire A; 1 s; pause it (test
  route); read p0 and a frame code f0; fire A; at 0.5 s the playhead is within one frame of p0, A's `paused` is true,
  the frame code is within 1 of f0; a second capture 0.5 s later has the same code. (b) fire B: two reads 0.3 s apart
  differ by >= 0.01, two captures 0.5 s apart differ by >= 5 frames, B's `paused` is false, A's is true. (c) fire A:
  at 0.5 s its playhead is within one frame of p0 and the frame code within 1 of f0. (d) play: two reads 0.3 s apart
  differ by >= 0.01; A's `paused` is false. (e) a second layer plays; pause A at p1; `trigger_column` on A's column: A's
  two reads 0.3 s apart are equal and within one frame of p1; the other layer's read is within 0.06 of its in point.
  (f) with A paused at p1: save the show to a scratch path (S0's route; if there is none this arm loads a hand-written
  file carrying the two keys and TL-U80 alone proves the write), load it: `/api/composition` reports A paused and no
  other clip paused; the file holds `"paused": true` exactly once; fire A: the playhead is within one frame of p1 and a
  capture's frame code within 1 of p1 x 300. (g) load a pre-lane fixture show: no clip reports paused. (h) with A
  paused, rename A (a command), `POST /api/debug/undo`: A's `paused` is still true and its place unchanged. GUARD (the
  field is new); RED arm of (a): the frozen app, reported. Teeth: MU-60 (a, c, e), MU-61 (a), MU-101 (b), MU-102 (f),
  MU-110 (f), MU-103 (h).
- TR29 beatloopr: the ruling's row, with "locked" read as "in time"; its last sentence reads "Again without Catch Up:
  no step at Off; at most one step, within 0.3 s after the driver's next 1; then abs(e) <= 0.10".
- TR31 (c): "from 36 beats on" reads "from 8 beats on".
- TR32 timeline_speed_ten (NEW; merge 2). The 10 s ramp, Timeline, in 0, out 0.8. `set_clip_param speed 10`: the reply
  and `/api/composition` say 10; the content rate over 2 s is in [9.5, 10.5]; every read is in [0, 0.8 + 1/300].
  `speed 12`: stored 10. `speed -1`: stored 0 and two reads 0.5 s apart are equal. Then `speed 1`: the rate over 2 s is
  in [0.97, 1.03]. The strip dump's S fader reads travel 1.0 at 10 and 0.25 at 1 (+/- 0.005). GUARD (the key is new).
  Teeth: MU-108 (stored 4), MU-104 (the dump's travel).
- (outside the pin) TR24 lock_follows_the_nudge. From TR20's state (driver at 60), through the nudge lane's own route:
  (a) +90 ms (0.09 beat): no step; within 0.5 s abs(e) <= 0.05 against the SHIFTED bar. (b) a further +300 ms: no step
  before the driver's next 1, exactly one after it, then abs(e) <= 0.10. If the nudge's range cannot reach (b), that arm
  is dropped and reported. Teeth: MU-100 (a: e stays 0.09). Run by whichever lane merges second. With the tempo row:
  TR24b the beat stopped: a synced clip's two reads 1 s apart are equal and its `paused` is false, a Timeline clip's
  differ; started again after a pause: no step.
- Paths with no live row and what proves them: as the ruling, with "the lock" read as "the hold and the cut"; the cut on
  a sequence, reversed and ping-pong -- TL-U47; the saved pause's write -- TL-U80.

### 5.4 Re-runs and idle paint: as the ruling. Added: probe-idle-paint with a paused clip in the grid and nothing
playing -- no repaint of the grid (TL-U82 is the unit gate).

### 5.5 VISUAL WORK GATES (a capture builder, then five critic seats, then Harmony; Boris sees nothing before a gate passes)
- VG-1 (before merge 1). P1, P3: as the ruling. P2 reads "the same clip paused (Clip tab, strip, and its grid cell)".
  NEW P4: the grid with three clips in view -- one paused and playing on its layer, one paused that no layer plays,
  one not paused. NEW P5: the paused clip that no layer plays, selected: the Clip tab shows it. Bars (from
  `transport_buttons`, `/api/composition` and S0's grid dump): P2 -- pause lit, play not lit, the CLIP's `paused` true,
  the cell's mark on; P4 -- the mark on exactly the two paused cells; P5 -- the Clip tab's pause lit AND enabled; no
  widget of the section moved against the pre-lane capture (+/- 1 px); the mark overlaps neither the "!" nor the "L"
  corner. GUARDS (new routes); teeth: MU-98 (P2), MU-109 (P4). The seats get: his three pause sentences (B2, B5), the
  five states, and these questions -- can a paused clip be told from one that is not at a glance across the grid, on
  a dark and on a bright thumbnail? can the mark be mistaken for the lock or the missing-file sign? does P5 look like a
  clip that is playing?
- VG-2 (after S5b). The ruling's states stand; W18 reads "a paused clip (Clip tab + strip + cell)". NEW W23: Timeline,
  Speed 10 (panel and strip). NEW W24: Timeline, Speed 2 (both). C13 reads: "W18: pause lit and play not lit on both
  surfaces, the clip's `paused` true, the cell's mark on; W19: backwards lit on both". NEW C17: "W1: the Speed thumb
  at 1/4 of its travel; W24: at 1/2, text 2; W23: at the top, text 10, plus disabled; the strip's S fader at the same
  three travels; all +/- 2 px". Teeth: MU-104 -> C17. The packet's D list is D-1 .. D-16 as section 6 has them.

### 5.6 Facts Harmony must measure (none can be settled by reading)
N1, NC, R1, X7, D1, H1 (SM-a; and once more with the nudge lane). X2, X3, X4c, X5, X6, X6t, X8 (SM-b) with TA-6's
table. The noise of e (5 runs of a clip in time) and of the REST latency (20 calls). FM-1, FM-2, FM-7, FM-8: the
ruling's. After S4c: M2 at one tempo and M4 again on the lane build, same bars; not met: STOP and report.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; on his real screens and by his ear)

WHERE HIS RULING DIFFERS FROM RESOLUME. D-1 .. D-5, D-8 .. D-13 stand. Re-written or new:
- D-6 A clip that is out of time is brought back by ONE cut on the next "1" -- after a drop, a Tap that moves the beat,
  and after every fire that is not on the "1" (with Snap on Bar a fire lands on the "1" and nothing cuts). Resolume
  leaves it off the bar until it is fired again.
- D-7 Pause is the clip's own, as in Resolume -- and here it is also SAVED with the show, and a paused clip carries a
  pause mark in its cell.
- D-14 A Resync cuts synced clips into time at once, each to the nearest place on its own bars. Resolume's Resync jumps
  them to their first beat. (Question 121.)
- D-15 Ours, not from Resolume: a press inside the bar moves the playhead there. Timeline Speed goes to 10 as
  Resolume's does; its curve is ours (the lower half of the slider is 0 to 2), because Resolume's is not published.
- D-16 BeatLoopr Off without Catch Up: the clip plays on from where it is and cuts into time on the next "1". In
  Resolume it stays where it is.
CHECKS. The chain's checks and the ruling's 1 .. 3, 8 .. 11, 13, 17, 18, 20 .. 23 stand. Re-written:
- 4. Pause a clip, then fire it. -> It stays on its frame, paused, and its cell shows the pause mark. Press play: it
  runs on from there and the mark goes. Wrong: it jumps to its start, it starts playing, or no mark.
- 5. Fire a BPM-synced clip in the middle of a bar with Snap off. -> It starts at its beginning, plays, and on the next
  "1" it cuts once; from then on its bar lines meet the "1". With Snap on Bar it starts on the "1" and nothing cuts.
  Wrong: no cut and it stays off; more than one cut; a cut that is not on the "1". Say if the cut should take it back
  to its beginning instead (question 123).
- 6. Pause a clip, then fire ANOTHER clip on that layer. -> The other clip plays. The first one keeps its mark. Fire
  the first again: it is back on the frame where you paused it, still paused. Clear the layer with X and fire it once
  more: the same. Wrong: the other clip waits; the first one plays or shows its first frame.
- 7. Pause a clip and fire its whole column. -> It appears on its paused frame; the other layers start.
- 12. Step Speed through its nine values in BPM Sync (panel, then the S fader). -> as the ruling. THEN in Timeline:
  drag Speed from 0 to 10 on a clip you know, forwards and backwards, on the panel and on the S fader. -> 1 sits at a
  quarter, 2 in the middle, 10 at the top; the two controls move together; right-click gives 1. Wrong: stutter, a
  frozen picture or black at the top of the range; a jump of speed in the middle of the slider.
- 14. Drop the playhead in the middle of a bar. -> It plays on from there and cuts once on the next "1". Press Resync
  in mid-bar. -> One cut, at the press. Wrong: a slow drift back, more than one cut, or it never comes back.
- 15. Tap a new tempo while a synced clip plays. -> It takes the tempo at once. If your taps moved the beat it cuts
  once on a "1" (at most twice while you tap). Wrong: a cut on every tap; it ends up off the "1".
- 16. In Auto, on your own music, leave a synced clip alone for ten minutes. -> Its lines stay on the "1" and it does
  not cut. Wrong: every so often it cuts (that is the app moving its own "1" -- tell me the track).
- 19. BeatLoopr: press "1/2". -> Half a bar repeats from where the playhead was. Off with Catch Up lit: it cuts at once
  to where it would have been. Without Catch Up: it carries on from there and cuts on the next "1".
NEW:
- 24. Pause two clips -- one that is playing, one in a deck you are not showing. Save the show. Quit. Open it. -> Both
  cells carry the pause mark. Fire each: it appears on the frame where you paused it, not playing. Press play: it runs
  and the mark goes; save again and it opens playing. Wrong: a mark is missing, a clip plays, or it shows another frame.
- 25. Open a show you saved before today. -> No clip is paused; every clip plays at the speed it had.
- 26. Pause a clip, change something else (rename a clip, move an effect), press Cmd+Z several times. -> The pause
  mark never changes.
- 27. (when the tempo row is built) Press stop on the beat. -> Every BPM-synced clip stands still, its play button
  still lit and no pause mark; clips in Timeline keep playing. Press play: they run again.

## 7 QUESTIONS FOR BORIS (new numbers 121 .. 123; 124 is not used; each has a default A; nothing waits)

121. You press Resync while BPM-synced clips are out of time.
     A (default) They cut into time at once: your press is the "1".
     B They play on and cut on the next "1", one bar after your press.
122. You save a show with a paused clip and open it another day.
     A (default) The clip is paused on the same frame as when you saved.
     B The clip is paused on its first frame.
123. You fire a BPM-synced clip between two "1"s. On the next "1" it cuts into time. Where to?
     A (default) To the nearest place that is in time. If you fired it early in the bar that is inside its second bar.
     B Always back to its beginning, so it starts over on the "1".
What each B changes: 121 -- one argument of `barOneCrossed` (a Resync is not a "1"), TL-U83's last clause, X5's and
TR21 (d)'s Resync sentence, D-14. 122 -- one line of Clip.cpp (the key `pausedAt` is not written), TL-U80, TR25 (f)'s
frame clause. 123 -- `applyCut`'s target for a clip that has not been cut since its stamp was minted, one clause in
TL-U49j, TR20 (iv)'s e (it is the same) and a frame-code clause there.
Still held back, unchanged: 73 (FM-8). Requests RQ-0 .. RQ-3 stand; RQ-0 (three tracks) is the one the lane's first
run needs.

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)

- K-A1 THE STRONGEST, against this delta: THE HOLD IS A SLIDE UNDER ANOTHER NAME. The adoption voided "the 6 % slide
  law"; this file puts a speed correction back and calls it a hold. Why it stands: what he turned down in 71 was a
  picture that runs visibly fast or slow for 17 to 34 seconds. The hold cannot last longer than a tenth of a beat (50
  ms at 120) and works only inside a tenth of a beat; everything larger is his cut. Without it "9 b" has to be kept
  by cuts alone, and in Auto that is a cut every time the tempo number and the beat disagree by a tenth of a beat (D1).
  It is the first of the two ways the dispatch names ("a position worked out from the beat"). If the council or Harmony
  reads the adoption as forbidding any correction, the runner-up of F-A1 is one constant away (`kGridDeadbandBeats` =
  `kOutOfTimeBeats`: no held band) and D1 then becomes the number with NC's bar. Cheapest refuting test: SM-a's D1 and
  H1 on one track -- if D1 is at most 1 the hold buys nothing and the runner-up is the smaller change.
- K-A2 A CUT IS VISIBLE AND THE APP DECIDES WHEN. Under the slide a wrong "1" cost a slow drift; now it costs a jump in
  front of an audience. The anchor (8 beats) and the count-based "1" are the guard; NC measures it on his tracks before
  S4c is built; O3 stops the lane. Cheapest test: SM-a, one track.
- K-A3 The hold pushes a speed that changes for a few frames -- the chain's unproven FM-5 (a speed that changes every
  frame) in a milder form. X3 on M1 .. M3 is the proof; TL-U45c pins "exactly the rate in at least 95 % of frames".
- K-A4 0.05 is chosen by arithmetic on the hop (INFERRED), not measured; in Auto the beat's own jitter may be wider.
  Then the hold works on most frames and X3 fails in M3 only: row O8 (named candidate: a wider deadband in Auto).
- K-A5 The cut is ONE seek, and a seek on a long-GOP file may hold a frame. X8 (3 render frames, 10 of 10) is the only
  proof and O5 the way out. Cheapest test: M8 on the long-GOP fixture.
- K-A6 A Resync as a "1" (121 A) cuts every synced clip at the press, in one frame: N seeks at once. On a deck of
  several videos that is the upload budget's worst frame (Pitfall 60). M5 has one clip. Named addition, Harmony's to
  run: M5's Resync with three synced clips; peak frame time <= 50 ms (X6's number); over -> the cuts are spread over
  the following frames, one per frame (reported, an architect line decides).
- K-P1 `paused` is now SAVED, so it travels with every copy of a clip: a show, a duplicate, a command. TA-8's landing
  rule is the only thing between that and "Cmd+Z changed my pause". Cheapest test: TL-U81 on an idle cell (MU-103).
- K-P2 A clip paused weeks ago in a show he re-opens does not play when fired, and the only sign is the mark. That is
  his rule ("the clip is now paused until the user changes that setting"), and why the mark must pass VG-1 on bright
  and dark thumbnails. If the seats cannot tell a paused cell at a glance the gate fails; it is not waived.
- K-P3 A routine or a take recorded while a clip ran will, replayed over a show where that clip is now paused, show a
  still frame. That is the rule stated (nothing but his buttons changes the pause); it is in check 6's spirit but he
  has not been shown it with a routine. Say so when the lane is presented.
- K-P4 `holdPausedPlace` seeks a paused player that stands elsewhere. If some path moves a paused player's clock every
  frame (none is known: TL-U68), it would seek every frame. TL-U84's "never again" clause is the guard.
- K-S1 x10 on real 1080p files will very likely show late frames forward and holds in reverse (INFERRED, E12). The
  range is his ruling; the top is data and O6t tells him the truth. Cheapest test: M7t's forward arm, 60 s.
- K-S2 The two-piece law has a corner at 2: a fader moved at constant speed changes its rate of change there. It is the
  price of keeping "1" at a quarter. Check 12 asks him; the runner-up (skew) is one function body.
- K-O1 This delta leans on rows of the ruling I did not re-read (labelled RULING-VERIFIED) and on a tempo-row delta
  that does not exist yet (B10). `beatRunning` is built as a constant until it does.
- The ruling's K-1 .. K-3, K-6 .. K-8, K-10, K-11 stand.

## 9 WHAT IS NOT IN THIS LANE

- The ruling's section 9 and the plan's section 9 stand, with two lines REMOVED from "not in this lane": "Timeline
  Speed above 4 (question 74)" and "a per-clip held frame beside the layer's pause (ST-1)" -- both are now built, the
  second as the clip's own pause.
- NOT built here: the slide in any form (a speed law that runs longer than a tenth of a beat); a setting that chooses
  between slide and cut; a Resync that restarts clips at their first beat; a cut back to the clip's beginning after a
  fire (unless 123 B); the tempo row's play / pause / stop (the nudge lane's; this lane builds only the input it will
  feed); a production REST or OSC route for a clip's pause; recording a pause in a take; a pause filter or a "show
  paused clips" list; "play all" / "pause all"; a migration of old speeds; a clamp in the loader; Resolume's exact
  speed curve (not published); x10 in BPM Sync (its list stays nine steps); any text on screen that announces a cut,
  a pause or a failed seek (B11) -- the pause mark, a lit button and a greyed "plus" are states.
- A tracker fix so that the app's "1" moves less (SF-2) is its own plan; this lane measures (N1, NC) and reads around it.
- THE STOPPED SYNC-DIAL BRANCHES (rulings-bf2.md H-17: superseded). lane/bf2 at 740b6d6: NOTHING is carried into this
  lane; everything of it is dropped for this lane. lane/bf2-keys at 9eab9bd: NOTHING is carried into this lane; its key
  and pad work is re-typed by the NUDGE lane (plan-nudge.md:802-803), not here. I checked both heads and did not read
  either tree for code.
- not_verified (what I could not establish by reading, each with who settles it): how often the app's "1" moves and
  how far its tempo number and its phase disagree on real music (SM-a: N1, NC, D1); whether the hold is invisible (X3,
  check 13); the seek's latency on a "1" (X8); x10 on the decoder in each direction (X6t); which snapshot field marks a
  Resync and whether the test-mode route can post the count (S0); the file and function of the Undo landing rule's idle
  case (S0; I read its text in the chain's ruling, not the code -- the lane is not built); whether PerfState or a
  routine writes `playing` (S0, the ruling's own item); whether a player is ever closed while its clip stays in the
  grid (S0); bindings, OSC and routines that scale `Clip::speed` by 4 (S0); a REST route that saves a show to a path
  (S0); the nudge's range and its tempo-row delta (not written); Resolume's speed curve and its minus / plus step (not
  documented; RQ-3). The ruling's V-rows are taken as verified by the ruling, not re-read by me, except E1 .. E12.

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 15:36:21, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-transport-answers.md (status DONE; 16 attacks ruled: 11 ACCEPT, 5 PARTIAL, 0 REJECT; 14 amendments
TB-1..TB-14, each OVERRIDES this delta plan's body). It is a DELTA on ruling-transport-delta2.md: precedence for the transport
lane is now Boris's verbatim words > the adoption blocks at the end of plan-transport-delta2.md > this adoption >
ruling-transport-answers.md > ruling-transport-delta2.md > ruling-transport-delta1.md > ruling-transport.md > the plans.
Workflow run wf_19467261-08a (draft: architect opus high; seats gates 8 attacks / 2 MUST, stage-hands 8 / 1 -- papers whole
(18,130 characters) in attack-transport-answers-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict (whole), stage list and decisions, and its section 7
(questions) in full. NOT read by me: the other sections -- the builders' and reviewers' spec; gate strings only from section 5.
WHAT THE COUNCIL CAUGHT: the delta's draft kept a "held" band -- a 0..2x speed correction near the grid -- which is a slide
under another name; Boris answered "71 b" (no slide). STRUCK (TB): a synced clip always runs at exactly the tempo's rate;
more than 0.10 beat off it plays on and is cut ONCE on the next "1"; after his own hand on the beat (nudge, Tap, Resync, a
tempo) the next "1" seats every clip more than one frame off. And the Undo gate for "Cmd+Z never touches a clip's pause" was
ordered so that its mutant passed: re-ordered.
HIS RELIABILITY QUESTION is now "how often does the picture CUT": SM-a stays FIRST; N1 keeps its bar and STOP-AND-ASK; NC (the
cuts a clip would show) is new with the same bar (at most 1 per 10 minutes per track); over it on drift = row O3d, asked of
him with the measured numbers.
HARMONY'S DECISIONS: H-A1..H-A12 at their defaults. Notes -- H-A4 with the tempo-row adoption's HR-1: ONE builder at a time
in BPMTracker.cpp, order nudge S1, S1r, then S4t unless the transport lane reaches S4t first (then the nudge stages go on
top); H-A8: takes and routines keep recording and restoring play / pause, now writing the clip's pause -- told to Boris
(his check 29); H-A12: questions 121-124 are asked at this close; unanswered when a packet needs one: A is built.
"Routines" in this ruling's text are Boris's "actions" (BF80): the naming lane re-words; no builder invents wording.
STAGES of the lane now: S0, SM-a, S4t, SR, SM-b, S1, S2, (S2b), S3, S3h, MERGE 1, S4a, S4d, S4e, S5a, S5b, S4c, S6, MERGE 2, LM-0.
Questions 121-124: boris-clarify-121-124.md. 73 still held back (FM-8). RQ-0 (three real tracks) is needed by SM-a.
NOT STARTED in this session: nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWERS (2026-10-04 15:59:53)
Boris, verbatim (binding-decisions.md, "Boris's answers to questions 121-124; "Beat Repeat""): "121 a", "122 a", "123 b",
"124 a", "also, lets change name to beat repeat. beat loopr is resolumes original name". 121 A, 122 A and 124 A are the
ruling's defaults. 123 B: the ruling's section 7 names the change ("applyCut's target for a clip that has not been cut since
its stamp was minted, one clause in TL-U49j, a frame-code clause in TR20") -- the S4c packet carries this block and those
three names; no architect delta is needed. NAMING: every on-screen and documented "BeatLoopr" of this lane reads
"Beat Repeat" (stage S4e's row, the panel's S5a states, section 6's checks, the docs); code identifiers are the builder's.
H-A12 is closed: no question of this lane is open except the held-back 73 (FM-8).

## HARMONY ADOPTION, UPDATE ON BORIS'S MESSAGE (2026-10-04 21:04:04, session s-rta-1004b)
Boris, verbatim (binding-decisions.md, the section headed "2026-10-04 (s-rta-1004b)", recorded 2026-10-04 21:03:09; his whole message: boris-feedback-backlog.md, same stamp): "here is the beat repeat screenshot: [Image #5]"; "A screenshot of a clip in BPM Sync with the loop
menu on Random, showing Interval and Distance. this is essentially jumping to random 1's on the beat: [Image #6] [Image
#7]"; "one click on time jumps 1 bpm, duration in timeline mode moves up 0.1"; and, on reading R81 of the tempo row,
"Quantize ... This is not for live usage. We should remove it from the top bar" (quoted whole in binding-decisions.md).
Consequences, binding for every packet of the transport lane:
(1) RESOLUME FACTS that were requested, now on disk (.harmony/.reports/s-rta-1004b/boris-images/; each looked at by me):
    resolume-beat-repeat.png -- the panel "BeatLoopr": ONE row of buttons "Off 4 2 1 1/2 1/4 1/8 1/16 1/3 1/6" ("Off" lit)
    and a button "Catch up". resolume-random-1.png, resolume-random-2.png -- Transport in "BPM Sync" with the play-mode menu
    on Random: rows Speed "1/4" ("-" "+" and a slider), Interval "1" ("-" "+" slider), Distance "2" ("-" "+" slider), Beats
    "16" ("-" "+" "/2" "x2"); buttons back, pause, play. The packets of S4e (Beat Repeat) and S5a (the panel) carry this
    block and the three files; where facts-resolume-transport.md says NOT DOCUMENTED for these rows, the pictures win.
(2) Step sizes: Duration's "+" in Timeline mode adds 0.1 (his words). Which row "time" is ("jumps 1 bpm") is asked:
    question 137; unanswered = the other steps stay as ruled.
(3) QUANTIZE IS NOT A LIVE CONTROL. Reading R86 (told to him): every fire starts at once on the press; nothing waits for a
    beat or bar line; a BPM-synced clip is put in time by the one cut on the next "1" ("71 b", "123 b"). Every row of this
    lane's chain that makes a fire wait for a line is re-read: a fact sheet first, then an architect delta, BEFORE the
    packet of any stage it names. By the stage list S0, SM-a, S4t and SR do not rest on Quantize: unchanged -- NOT VERIFIED
    beyond the stage list; the fact sheet says.
(4) The tempo row's stop now takes every clip off the layer strips and its pause stops the clips (his answer to 126 / 127;
    questions 135, 136 open). Whether the row's pause writes the clip's OWN saved pause (this lane's TransportPause.h, his
    answer 72) is ruled by the tempo-row delta "nudge-row2" with this lane in view -- no builder decides it.
