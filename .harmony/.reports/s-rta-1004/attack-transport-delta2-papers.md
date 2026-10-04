# Attack papers -- lane transport-delta2 (blind council, 4 of 4 seats)

Written by the Architect (ruling role) FIRST, before any ruling, from the dispatch text.
Content is the seats' own JSON, pretty-printed, one section per seat; nothing added, nothing removed.
Raw JSON as received: 39807 characters (the dispatch stated 39807); parses as a closed JSON array of 4 seats.
Attack ids: VI-1, VI-2, VI-3, VI-4, VI-5, VI-6, VI-7, VI-8, GA-1, GA-2, GA-3, GA-4, GA-5, GA-6, GA-7, GA-8, ST-1, ST-2, ST-3, ST-4, ST-5, ST-6, ST-7, SC-1, SC-2, SC-3, SC-4, SC-5, SC-6, SC-7, SC-8 (31 attacks).
The ruling on these papers: .harmony/.reports/s-rta-1004/ruling-transport-delta2.md

## Seat: video-clock

```json
{
  "seat": "video-clock",
  "attacks": [
    {
      "id": "VI-1",
      "target": "TD3 fit (out point moved in) x TD6 lock x S4c TL-U47 / TL-U43",
      "claim": "The fit makes outPoint < 1 the default for any clip over 8 s. At that setting the real transport never runs backwards inside in..out. The plan's reversed and ping-pong lock rows pass only on the chain's fake player and never touch the real players.",
      "evidence": "ClipTransportSync.h:56-71: writeBack tests only `ph >= outPoint` and then `seekTo(inPoint)`. It has no in-point test for a backward clock. VideoPlayer.cpp:442-450: reverse Loop wraps to the file END (duration + fmod), and writeBack then sees ph >= out and seeks to inPoint. Reverse therefore runs from in down to 0 and wraps again. A PingPong forward leg is cut at out by that same seek, so it never turns. SeqVram.h:158-165 documents it: 'reverse wraps to the end (an active out point: to in)', and 'Loop and PingPong -- syncMedia seeks, it never reflects there'. Plan TD5 table says 'reversed: at its end', but Layer.h:553 sets the tail playhead to inPoint, and no changed line is named for it. Plan 744-745: TL-U47 uses fakes. No live row plays reverse.",
      "severity": "MUST",
      "proposed_change": "Add to S2 or S4c: an in-point test for the backward clock in writeBack, and a ping-pong reflection at in/out. The reversed fire starts at out (Layer.h tail). Prove it with a real-player case and a live row (reverse and ping-pong, fitted clip with out < 1, in-point never undershot, one seek per wrap). Re-register TL-U47 on the real players."
    },
    {
      "id": "VI-2",
      "target": "TD6 slide arithmetic, D-6, check 14, question 33",
      "claim": "The stated slide times leave out the tempos where the wait is worst. The app's tempo floor is 60 BPM, where the plan's own 34.1-beat worst case lasts 34 s. Check 14 tells Boris 'within 10 to 20 seconds', which is false at 90 BPM (23 s) and at 60 BPM.",
      "evidence": "BPMTracker.h:41-42 kMinBPM = 60, kMaxBPM = 200. Plan 428-436: worst = 34.1 beats, and the table lists only 90, 120 and 174. Plan 815 (TR20) runs the driver at 60. Plan 940-942, check 14: '10 to 20 seconds'. boris-clarify-26-37.md:43: B was offered as 'up to about 15 seconds'. boris-feedback-backlog.md:418: the earlier option was 'a few seconds'. The plan's own D-6 and K-12 name 17 / 23 s but not 60 BPM. The 34.1-beat figure itself re-derives correctly.",
      "severity": "SHOULD",
      "proposed_change": "Add 60 BPM (34 s worst, 17 s typical) to the table, D-6, K-12 and O1's sentence. Check 14's expected time becomes 'up to half a minute at slow tempos'. Offer Boris the jump fallback (TD6 F39) as a setting now, not only after a failed measurement."
    },
    {
      "id": "VI-3",
      "target": "TD7 decision table O1..O7, X1..X8",
      "claim": "The decision table is not exhaustive. Several outcomes of M3 (Auto) match no row, so the 'first row that matches' rule leaves them unruled. X1 also counts slips only through c, which cannot see the realign steps that Auto makes on every confident beat.",
      "evidence": "Plan 498-504. O1 needs every bar met. O2 and O3 need X1 to fail. O4 and O5 test X2, X3 and X4 'in M1 / M2' only. So 'X1 met, but X2 or X3 fails in M3 (Auto)' has no row, and neither does 'X5 alone' beyond a sentence at 506. c = (totalBeatCount - beatInBar) mod 4 (plan 471). BPMTracker.cpp:235-236 realigns on every confident detected beat, and :253-258 realigns without touching beatInBar. From the first half of a beat the phase steps back by up to 0.5 beat while c stays constant, so X1 can pass while e jumps. A phase-jitter failure is then sent to O4, whose jump fallback does not remove jitter.",
      "severity": "SHOULD",
      "proposed_change": "Add a catch-all row (any bar failing in M3 only: STOP AND ASK, with the log) and an O8 'phase jitter' row, for example smoothing e in the lock. Add the per-hop step of barPhase against time to the sample's derived columns as X1c, so the first-half realign is counted as a slip."
    },
    {
      "id": "VI-4",
      "target": "TD8 item 1, S4t, M5, S4c re-run",
      "claim": "M5 (Taps, Resync) is measured and ruled on a tracker that has the Tap bar-slip bug. The fix, S4t, comes later, after the nudge lane. The re-run after S4c is only M2 and M4, so the lock's Tap behaviour is never measured on the final tracker.",
      "evidence": "Plan 519-525: a Tap in the second half of a beat completes the beat but leaves beatInBar. BPMTracker.cpp:253-258 confirms realignPhaseToZero does not touch beatInBar. Plan 481 puts M5 in SM, before S4t. Plan 652 and 899 re-run M2 and M4 only. S4t is proved only by unit case TL-U70 (plan 752). The 'detection path advances beatInBar in the same hop' claim is INFERRED (plan 523-525). Every Tap before S4t makes the lock slide 1 beat off the displayed '1' (a 34-beat slide), which X5 'settled by 40 beats' hides.",
      "severity": "SHOULD",
      "proposed_change": "Make S4t part of SM's instrument build, or re-run M5 after S4t as a pre-registered step with X5 unchanged. Make the O-row verdict conditional on that re-run. Add a live row: eight Taps in Manual leave the clip within 0.10 beat of the wheel's '1'."
    },
    {
      "id": "VI-5",
      "target": "TD9 Random and BeatLoopr (TL-U72, TL-U73, TR28, TR29)",
      "claim": "Random's BPM Sync rule contradicts its own option list. Nothing stops Random jumps, BeatLoopr loops or Catch Up seeks on a paused, held or Speed-0 clip. The decoder's seek cadence at 1/16 bar is never measured.",
      "evidence": "Plan 546-550: both numbers come from {1/16, 1/8, 1/4, 1/2, 1, 2, 4} bars, yet 'its length is a whole number of BEATS'. A Distance of 1/16 or 1/8 bar is 0.25 or 0.5 beat, so a whole-beat jump is 0 beats or breaks the invariant. Paused is handled only for the lock (plan 389-393) and for Eject (TL-U71). TL-U72 to TL-U74 and TR28 have no paused, held or Speed-0 case, so a jump or Catch Up seek while paused would contradict 'stays, paused'. At 174 BPM 1/16 bar = 86 ms (about 5 frames) per seek. X8 measures ten seeks on a '1' only (plan 484), TL-U73 uses a fake, and TR28 is Timeline at 1 s.",
      "severity": "SHOULD",
      "proposed_change": "Clamp Distance to a whole number of beats (the list starts at 1/4 bar), or define what a sub-beat Distance does. State that Random, BeatLoopr and Catch Up are inert while paused, held or at Speed 0, and add the cases and a mutant. Add a live row for Random at the shortest Interval on the long-GOP fixture, with a held-frame count."
    },
    {
      "id": "VI-6",
      "target": "TD3 fitBars / kFitTolSeconds 0.05 (F35)",
      "claim": "The fit has a cliff just under each multiple of 8 s. A file that reads 0.06 s short of 16 s gets 4 bars and loses half of itself. The tolerance is a fixed 0.05 s, about 1.2 frames at 24 fps, and the plan admits it never sampled real durations.",
      "evidence": "Plan 273: 4 x floor((L + 0.05) / 8). 15.98 s gives 8 bars, but 15.94 s gives 4 bars with out moved in to 8.0 s (plan 297-303 shows the pattern). F35 itself says 'INFERRED that such lengths occur (container durations were not sampled)' (plan 309-310). The plan's example of a '16 s file as its container reports it' is one point, not the spread. A trailing partial GOP or an audio-longer container can be several frames off. Boris's rule is 'move the outpoint in' only for the uneven part ('multiples of four' with 'uneven' moved).",
      "severity": "SHOULD",
      "proposed_change": "Set the tolerance from the file: max(0.05 s, 2 frame durations, 0.5 % of L). Run S0's duration sample before S4a, and add TL-U33 rows at 15.94, 15.9 and 24 - 0.1 s. Show Boris the result as a number, not as lost video."
    },
    {
      "id": "VI-7",
      "target": "TD4 speed list x8 / x16, M7 / X6, TR27",
      "claim": "The x8 and x16 measurement covers forward play only. The Speed row, the strip's fourth button and BPM Sync all stay live in reverse and ping-pong, where Pitfall 62's GOP cache is the decoder path. Nothing measures that path at 16x, and 'Speed 0 against the decoder' rests on reading alone.",
      "evidence": "Plan 483: 'M7 speed steps x8 and x16, both fixtures, 60 s each' names no direction, loop style or in/out trim. TR27 (plan 839-840) steps speed forward only. VideoPlayer.cpp:505-506 and :1005 treat speed 0 as 'no next frame' (frameMsEff = 1e9), which only a read supports. At 16x, 60 fps content advances 0.27 s per frame, and the lock wraps a 4-bar clip every beat, a seek per beat (plan 352-354). The trimmed-loop wrap in ClipTransportSync.h:69 is a seek on every cycle. The only x16 bar is X6 content rate within 3 % and peak frame <= 50 ms.",
      "severity": "SHOULD",
      "proposed_change": "Extend M7 and X6 to reverse, ping-pong, and a trimmed 4-bar loop at x16 (seek per wrap), plus Speed 0 held 60 s then resumed. Add TR27 sub-rows for reverse and 0, with a held-frame counter and a decoder-stall counter. If x16 reverse fails, O6 stops the list at the highest clean step."
    },
    {
      "id": "VI-8",
      "target": "TD2 / TD5 pause surfaces, W10 / W18, C13",
      "claim": "A paused layer and a Speed-0 clip are told apart only by which button is lit. The plan's pause rule does not state that the player's position cannot be changed by another writer while paused. The play-back write-back (`playheadPosition.store(ph)`) and the loop seek still run while paused.",
      "evidence": "Plan 389-392: while paused, writeBack skips only the play-state compare-exchange and the OneShot stop. ClipTransportSync.h:56-71 shows the out-point test and `seekTo(inPoint)` run for every non-OneShot mode regardless of intent. A paused clip at ph >= out (for example a Hold clip paused on its last frame after the fit moved the out point in) is then seeked to inPoint by writeBack. 'It stays, paused' (Boris, binding-decisions) is contradicted. TL-U62 covers only a OneShot at its last frame, not a Loop clip at or past out. W18 / C13 check lit states, not position.",
      "severity": "SHOULD",
      "proposed_change": "Gate the whole out-point branch of writeBack on `!(held || paused)`, not only the CAS and the stop. Add a TL-U62 sub-case: a Loop clip paused at ph >= out keeps its playhead through ten syncs. Add a TR25 read of equal positions 0.3 s apart on a fitted clip."
    }
  ],
  "strongest_point": "VI-1. The plan's central trick is to fit a clip by moving its out point in, so most files end up with outPoint < 1. The real transport cannot play backwards inside in..out. ClipTransportSync.h:56-71 and VideoPlayer.cpp:442-450 let a reversed clip run below its in point to 0 and then jump. A ping-pong never turns, because the out-point seek fires before the player's own reflection. The plan's reversed and ping-pong lock rows (TL-U47, TL-U43) run only on fake players, and no live row plays a reversed fitted clip. As written, the plan would ship a reverse button and a ping-pong style that break exactly on the clips this lane trims.",
  "citations_rechecked": true
}
```

## Seat: gates

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "TD1 C and 5.3: live row TR7c 'STANDS' against D2-7 and MU-10",
      "claim": "TR7c is kept unamended, but D2-7 makes its bars fail or pass vacuously on the lane build. The row's Loop and OneShot variants and MU-10's teeth all rest on a press outside in..out landing on a point.",
      "evidence": "Plan :186 and :805 list TR7c as STANDING. Plan :411 (D2-7): a press outside in..out 'does NOTHING: no scrub, no hold'. ruling-transport.md:987-992: TR7c is 'strip down 0.95' with out 0.8, then 'OneShot variant: playing false and the playhead <= 0.81'. A press that does nothing leaves a OneShot playing, so 'playing false' fails. The Loop bar '[0.2,0.3] within 0.5 s' then passes only by wrap timing. Delta1's added clause 'strip down 0.05, up: no read below 0.2' (ruling-transport-delta1.md:904) is vacuously true. MU-10 (no clamp) cannot go RED, because a drag that begins outside is ignored. MU-63 has no live arm.",
      "severity": "MUST",
      "proposed_change": "Move TR7c from STAND to AMENDED. Arm (a): a press outside in..out leaves the playhead, `playing` and the hold untouched, checked by two reads and an unchanged `playing`. Arm (b): the press begins inside and drags past out. The OneShot ends within 0.5 s with the playhead <= out + 1e-3, and MU-10 turns arm (b) RED. Give MU-63 a live arm through the strip route."
    },
    {
      "id": "GA-2",
      "target": "5.1 TL-U36, TL-U37, TL-U71 (and MU-64, MU-67, MU-80)",
      "claim": "These cases test code that lives in Renderer.cpp and MainComponent.cpp, and no ctest target links either file. They can only test a mirror of the product, and the plan names no pure function to hold the logic.",
      "evidence": "tests/CMakeLists.txt:43-44 and :66-67 say 'MainComponent.cpp and Renderer.cpp are not linked into any ctest target'. TD3 'Changes by file' puts 'the speed pushed = barSyncRate x syncSpeed' and the clipBars==0 fallback in Renderer.cpp. It puts the fit, the pending-fit list and the length provider in MainComponent.cpp (plan :332-336, :644-645). TL-U36 (:721), TL-U37 (:725) and TL-U71 (:754) assert exactly those behaviours. S4a has no lint like TL-L5 tying the Renderer expression to the case. The Eject site is 'S0 names' (:541). The chain met the same trap: delta1 GA-5, 'UC16 asserted something no test target can call'.",
      "severity": "MUST",
      "proposed_change": "Put the logic in pure, linkable functions. For example: `double barPushRate(const Clip&, double len, double bpm, ...)` in ClipBarGrid.h, `fitClipForBpmSync(Clip&, double len)` and a `PendingFits` class in src/model, and the Eject decision in ClipTransportSync.h. Make TL-U36, TL-U37 and TL-U71 call those functions. Add a lint pinning that Renderer.cpp and MainComponent.cpp call them exactly N times. Show each case RED with a mutant in the function."
    },
    {
      "id": "GA-3",
      "target": "5.3 TR25, TR27 and 5.5 C13, W18, W10; Boris checks 4, 6, 12",
      "claim": "Every claim about the picture is proven by model numbers (REST playhead, `paused`, `playing`). That includes 'stays on its frame, paused', 'shows its first frame and waits' and 'at 0 the picture stands'. No row reads pixels, and no C-bar does either. The stale-frame risk (AM-21) is exactly a model position that disagrees with what is shown.",
      "evidence": "TR25 (plan :832-835) reads only the playhead and `paused`. TR27's 0-step bar is 'two reads equal and `playing` true' (:839-840). C13 and C7 read lit and enabled state. Boris check 12 ('at 0 the picture stands still'), check 4 and check 6 are left to his eyes (:924-937). The frame-coded ramp exists and TR2-TR4 already compare pixel frame codes (ruling-transport.md:952-958), so a machine can read the picture. Pitfalls 53, 56 and 60 (the shown slot belongs to the reader, a hold is not pending) show that the model number and the shown frame can differ.",
      "severity": "MUST",
      "proposed_change": "Add the app's own capture of the frame code (as TR2-TR4 do) to TR25 a/b/c, to TR27's speed-0 arm and to speed 16, and to TR29's loop. In (a) and (b) require the frame code to equal the model's position within 1 frame, and equal across two captures 0.5 s apart. Teeth: MU-62 or a never-seek mutant turns it RED. Strike the machine-testable half of checks 4, 6 and 12 from Boris's list."
    },
    {
      "id": "GA-4",
      "target": "5.3 TR30 and MU-80",
      "claim": "The paused arm of TR30 cannot go red under MU-80, so the claimed 'Teeth: MU-80' is false for the live row. A paused layer never lets its clip end, so Eject's trigger cannot be reached.",
      "evidence": "TD9 (:540-541) fires Eject when `playing == false`, the stamp is seen and the layer is not paused. TD5 (:388-392) and TL-U62 (:711-712) say a paused layer never writes `playing` back as ended. A paused layer's clip therefore stays `playing` true, and TR30's 'the same on a paused layer: still there after 4 s' (:846-847) holds with or without the pause check in Eject. Pausing after the end cannot be staged, because Eject fires within a tick. Only TL-U71 (an assembled state) can catch MU-80.",
      "severity": "SHOULD",
      "proposed_change": "Drop 'Teeth: MU-80' from TR30 and say the paused arm is a guard. Or build the state: let the clip end with Eject suppressed by a test switch, pause, lift the switch, and read that the layer still holds the clip. Keep TL-U71 as the real catch, with MU-80 proven there."
    },
    {
      "id": "GA-5",
      "target": "TD7 decision table O1-O7 and the X bars",
      "claim": "The pre-registered table has outcomes that match no row, though 'the first row that matches decides'. Auto-mode failures of X2 or X3 are one such outcome. The clip-bar slips that X1 counts miss realigns of the beat phase, which move e just as much.",
      "evidence": "O4 and O5 require X2/X3/X4 to fail 'in M1 / M2' (:501-502). X2 and X3 are also scored in M3, Auto (:487-488). If M3 fails X2 or X3 while X1 and X1a pass, no row matches. X1 counts changes of c = (totalBeatCount - beatInBar) mod 4 (:470). In Auto the tracker hard-realigns the phase on every confident beat, which can add one to totalBeatCount without touching beatInBar (BPMTracker.cpp:234-236, :253-258; facts-beat-controls VERIFICATION A3, A10). That moves the shown bar position and slides the clip, yet c does not register it as a slip. X5 failing alone and X7 are the only other stops.",
      "severity": "SHOULD",
      "proposed_change": "Add rows. 'X2 or X3 fails in M3 only, X1 met' -> STOP AND ASK, with the fallback named (the lock reads the anchored bar plus a phase low-pass). Define X1 on e itself: count windows where |e| > 0.25 beat that are not explained by a command. Report c as diagnosis only."
    },
    {
      "id": "GA-6",
      "target": "TD7 M8 / X8 and the sample record",
      "claim": "X8, 'the new picture is up within 3 frames', which separates O4 from O5, cannot be derived from the instrument's own sample. The sample holds no field for what is shown.",
      "evidence": "The sample (plan :469) holds frame time, bpm, beatPhase, beatInBar, totalBeatCount, trackerState, trackerRequestSeq, resyncBarOrigin, the playhead before the advance, speed, trim, e and flags. M8 (:484) needs 'frames until the shown picture comes from the new place'. Pitfalls 56 and 60: the render thread picks the newest ring frame <= its clock and the shown slot stays the reader's, so the playhead is not the shown frame. Pitfall 52: a capture owns the time override, so using capture per frame would perturb the measurement.",
      "severity": "SHOULD",
      "proposed_change": "Add the shown frame's pts or index (the ring pick's result) and the pending-seek stamp to the probe sample. Define X8 from those fields, for example the first frame whose shown pts is within 1 frame of the target. Say the verdict is read offline from the log, not from capture."
    },
    {
      "id": "GA-7",
      "target": "5.5 C9 vs C3; Boris checks 7 and 20",
      "claim": "C9 contradicts the section's own states and cannot pass as written. And two Boris-only checks hide claims a machine can test.",
      "evidence": "C9 (:876-878): 'nothing in the section's captions, labels or tooltips changes across any do action except numbers and lit/greyed states'. The mode_bpm/mode_timeline action flips 'Duration' to 'Bars' and shows or hides the BeatLoopr row (C3, :871-873). loop_style=Random reveals 'Interval' and 'Distance' (:226). So C9 either fails by construction or is judged after the run. Check 20 (Cmd+Z leaves the fit, :951) can be run through `/api/debug/undo`, as TR11 does. Check 7 (a column fire on a paused layer, :929) has only TL-U64 and no live row, though `trigger_column` exists (TR2).",
      "severity": "SHOULD",
      "proposed_change": "Restate C9 as a finite whitelist of strings per state, with the union pinned. Any other string in any state is red. That also gives AM-27 ('no text announces an event') a machine test. Add `undo` after the switch to TR26, requiring clipBars and outPoint unchanged. Add a TR25(d): `trigger_column` with a paused layer leaves `paused` true and `playing` true on the other layers. Remove checks 7 and 20 from Boris's list."
    },
    {
      "id": "GA-8",
      "target": "5.3 TR28, TR29 bars",
      "claim": "Two new rows state bars that are undefined or unmeasurable. They risk being fixed, or flaking, only after the run.",
      "evidence": "TR28 (:841-842) counts 'a step between two reads that the rate does not explain' with no threshold. Random targets are 'at most Distance away' (:544), so a legal jump can be tiny, and '>= 7 of ~10' then depends on the seed. TR29 (:843-845) requires every read to lie inside [p, p + one bar + one frame], with p read BEFORE the press. At rate 0.5 and REST latency L, the loop start is already p + 0.5 L/10 s, so 50 ms is 0.0025 against one frame (1/300 = 0.0033). The plan's own rule is that a bar inside 4x the measured noise is BLOCKED (:684), but no noise measurement is scheduled for these rows. 'Off with Catch Up: exactly one step' has no definition of a step either.",
      "severity": "SHOULD",
      "proposed_change": "Fix numbers now. In TR28, a jump is |step - rate x dt| > 0.02 of the clip, and the test seeds the xorshift so that every jump is >= 0.05; require >= 7. In TR29, take p from the press reply or from the `beatLoop` stamp, not from an earlier read, and widen the upper bound by rate x measured latency. Define 'step' as > 3 frames beyond the rate. Register the noise-measurement run before the row is judged."
    }
  ],
  "strongest_point": "GA-2 and GA-1 together are the strongest. The plan's central new unit cases (the push rate, the fit and its pending list, Eject) assert behaviour inside Renderer.cpp and MainComponent.cpp, which tests/CMakeLists.txt:43-44 and :66-67 say no ctest target links, and it names no pure function to hold that logic. The live row TR7c is carried as 'STANDS' although its own D2-7 rule (a press outside does nothing) makes its bars fail or pass vacuously and leaves MU-10 unreachable. The counts, the worked examples and the slide arithmetic did hold when I re-derived them. The weakness is in what the gates actually exercise.",
  "citations_rechecked": true
}
```

## Seat: stage-hands

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "TD5 / D2-6 pause on the layer (F38; D-7; check 6)",
      "claim": "Boris pauses clip A, fires clip B on that layer, then fires A again. A shows its FIRST frame and waits; its paused position is gone. In Resolume pause is the clip's own setting, so A would still sit on its frame and B would play. Boris's words are about 'a clip' that is paused and then fired.",
      "evidence": "Plan 376-379: 'fire another clip of that row' on a paused layer shows its first frame and waits. Lines 369-373 concede pause is the clip's in Resolume (R6). Line 927 and K-3 (988) admit the rule rests on an INFERRED reading. F32 (238-240): the Clip tab's pause is greyed when no layer plays the clip, so 'pause a clip, then fire it' is impossible on a clip that is not already playing. Boris: 'when you pause a clip and then fire it, it stays, paused'.",
      "severity": "SHOULD",
      "proposed_change": "Keep the layer's pause, and also store a per-clip held frame, so a clip paused while active is fired again and shows that frame. Do not grey the Clip tab's pause when no layer plays the clip; let it set the clip's own held state. Ask Boris check 6 BEFORE S2 is built, not after."
    },
    {
      "id": "ST-2",
      "target": "TD7 decision table O1 and the pre-registered bars X2 / X4",
      "claim": "The sentence Boris is promised ('slides back onto the bar in at most 17 seconds at 120') is not what the gate measures. The bars allow 40 to 44 beats and a 0.10 beat tolerance. Boris would be told 17 s, and a 20 s slide would pass.",
      "evidence": "O1 (498): 'at most 17 seconds at 120'. X4 (489): '|e| <= 0.10 beat by 40 beats after the seek'. X2 (487): 'from 44 beats after the fire'. At 120 BPM 40 beats is 20 s and 44 beats is 22 s; at 90 BPM 40 beats is 26.7 s. The plan's own worst case is 34.1 beats, 22.7 s at 90 (TD6 table 432-436), to a 0.03 beat deadband. The promised figure is never a pass/fail line.",
      "severity": "SHOULD",
      "proposed_change": "Add a bar: the first frame with |e| <= 0.10 comes within 34 beats, in 22 of 22 drops at every tempo. Or word O1's sentence to the gate's real number ('about 20 seconds at 120, 27 at 90'). Either way the sentence and the bar must be the same number."
    },
    {
      "id": "ST-3",
      "target": "D-6 and checks 13 / 14: what Boris is told about slides",
      "claim": "The wait is told to him only for a DROP and a Resync. The ordinary action is a fire. A clip fired mid-bar always starts at second 0, which is rarely on the music's 1. Boris sees a speed wobble of up to 6 % lasting 9 to 17 s after most fires. The plan never says so.",
      "evidence": "D-6 (910) says only 'A clip dropped mid-bar SLIDES'. Check 5 (926) says only 'It starts over from its beginning'. The plan's own typical slide is 8.7 s at 120 and 11.6 s at 90 (429-436). TR20 (815-816) fires at bar position 1.2 to 1.3 on purpose. The Snap choices are Off, Beat, Bar, 2 Bar, 4 Bar (ClipInspector.cpp:163-167). Only Bar and above land on the 1, so with Snap Off or Beat every fire slides.",
      "severity": "SHOULD",
      "proposed_change": "Add to D-6 and check 5 that every unquantised fire in BPM Sync slides. Ask Boris whether a BPM Sync fire may start the clip at the bar position the music gives (one seek at fire, no later slide) as the alternative. Keep the slide as the default only after he has seen it on a fire."
    },
    {
      "id": "ST-4",
      "target": "TD9 BeatLoopr and Random option lists (S4d / S4e, K-10, RQ-1)",
      "claim": "Boris has Arena on the same desk. The plan builds BeatLoopr buttons and Random's Interval / Distance steps as bar fractions 1/16 .. 4 from two documentation sentences. It then asks for a screenshot afterwards. His buttons may not match Arena's, and nothing in the gate compares them.",
      "evidence": "Plan 552-555: the list is 'NOT DOCUMENTED', default is kBarFractions, RQ-1 is 'not a question'. K-10 (1004) admits 'built from two sentences'. C15 (882) checks only 'exactly one option lit'. Boris: 'mimic exactly how resolume is doing'.",
      "severity": "SHOULD",
      "proposed_change": "Make RQ-1 a stage precondition for S4e: the option array is not registered until his screenshot arrives (a minute in Arena), and the visual gate compares the row to that screenshot. Do the same for Random's Distance semantics and defaults. Build S4d with only the loop-menu names until then."
    },
    {
      "id": "ST-5",
      "target": "TD3 the fit (D2-3, D2-5), TR26, check 20, check 21, question 75 default A",
      "claim": "The out point is moved in silently, and there is no way back. Switch a 45 s clip to BPM Sync and then back to Timeline. It now ends at 40 s for good: Cmd+Z does not restore it, and dragging the pointer out by hand in BPM Sync speeds the clip up. Worse, opening a saved show applies the same trim to his old clips before he has answered 75.",
      "evidence": "Plan 289-290: a later switch keeps 'both points'. TR26 (837): 'to Timeline and back: both unchanged'. Check 20 (951): 'the out pointer stays moved'. D2-5 (325-329) and Q75 default A (970) fit every old BPM clip on open. Question 75 is unanswered, so the default is applied to his saved shows. K-4 (991) admits it trims files he never trimmed. The renderer already plays an unfitted clip without writing (289).",
      "severity": "SHOULD",
      "proposed_change": "Save the out point from before the fit, and give it back on a switch to Timeline when the out point still equals the fitted one. On open, play an old clip at its fit's rate but write the out point only on his first switch or on a Bars right-click. Make Q75's B the default until he answers."
    },
    {
      "id": "ST-6",
      "target": "TD3 THE ROW: '/2' greyed at 12, 20, 28 (question 71 default A, F43)",
      "claim": "'/2' is dead on every clip whose Bars is not a multiple of 8. A 30 s file gives 12 bars, which is common. Boris sees a greyed '/2' that looks broken, with no tooltip allowed to explain it. The plan's own typed rule says half of 12 (6) becomes 8, so the button could do the same.",
      "evidence": "Plan 312-314: '/2' enabled only when half is itself allowed. Line 277 and the TL-U33 text: typed 6 gives 8 (tie goes up), and a typed 13 gives 12. Question 71 B (958-959) offers '12 -> 4', which is the wrong halving. C5 (874) tests '/2 enabled false in W4 and W5', so the gate pins the grey state. Boris: 'doubles and halves ... just like resolume does'. In Arena '/2' always acts.",
      "severity": "SHOULD",
      "proposed_change": "Make '/2' = snapBars(n / 2) (12 -> 8, 20 -> 12, 28 -> 16) and grey it only at 4. Re-aim C5 and TL-U51 at that. Re-word question 71's B to 'the nearest allowed number' (8)."
    },
    {
      "id": "ST-7",
      "target": "TD2 panel item 4 / D-5: the second small menu",
      "claim": "The first screenshots Boris sent show TWO small icon menus at the right of the button row, loop style and a trigger-style icon. The plan draws one. Boris said 'mimic exactly', so he sees a gap beside Arena on the same desk. The plan lists it as a difference in D-5 without the screenshot fact.",
      "evidence": "resolume-transport-2.png and -7.png: two icons right of the buttons (loop arrow, and a bar-and-arrow icon). Plan 224-226: 'at the right ONE small menu'; P1 (98-101) says 'two small icon menus'. Boris's '42 default' only drops a per-clip carry-on menu.",
      "severity": "NIT",
      "proposed_change": "Say in D-5 and check 2 that Arena has a second menu there and ours omits it by his '42 default'. The critic seats' visual-design question should name it, so it is judged as a chosen gap and not as a missing part."
    }
  ],
  "strongest_point": "ST-2 and ST-3 together. The slide is the heart of Boris's reliability question ('can you program this reliably'), yet the promised sentence (17 s) is looser to verify than it sounds (the bars pass at 40 to 44 beats). Boris is also never told that every ordinary unquantised fire, not just a drop, runs 6 % off for about 9 s, because D-6 and check 5 name only drops and Resync. The plan's own TR20 fires mid-bar on purpose.",
  "citations_rechecked": true
}
```

## Seat: scope (MINIMALIST)

```json
{
  "seat": "scope (MINIMALIST)",
  "attacks": [
    {
      "id": "SC-1",
      "target": "TD7 / stage SM: the instrument (BarLockProbe ring, Renderer probe branch, 4 test routes, M1..M8)",
      "claim": "The first and biggest stage pays for a player-side probe to answer a question that needs none. X1 (how often Auto moves the '1') and X7 (tempo wobble) are properties of the tracker, readable from the existing route. The probe branch re-writes S4c's rate push and S4c then deletes it.",
      "evidence": "ApiServer.cpp:836-860 (handleGetBpm) already publishes bpm, beatPhase, beatInBar, totalBeatCount, totalBarCount, resyncBarOrigin in one coherent snapshot; TD7 derives c = (totalBeatCount - beatInBar) mod 4 from just those fields (plan l.468-471). Plan l.461-466 adds a 131072-sample ring, a Renderer branch pushing barSyncRate x step x (1+trim), 4 routes; l.651 'The probe branch of SM is DELETED here'. M1-M3 alone are 60 min of runs (l.475-478), re-run after S4c (l.652).",
      "severity": "SHOULD",
      "proposed_change": "Split SM. SM-a: a poller on /api/bpm (about 20 Hz) for X1 and X7 on M3's three tracks. No product code, no new route, answers O1/O2/O3 before any lock exists. SM-b (ring, probe, X2-X4, X6, X8) is built only if SM-a does not hit O3. Shorten M1/M2 to 3 min (deterministic driver, free-running Manual bar); keep 10 min only for M3/X1. Lost: one stage of time-to-first-number."
    },
    {
      "id": "SC-2",
      "target": "Section 4 order: 'S4c, S4d, S4e, S5a, S5b, S6 do not start before the verdict' (l.506-507, l.628-629); H-T4 default 'one merge'",
      "claim": "Everything Boris can see waits on a measurement of the lock, though only S4c depends on it. The panel, bar lines, Speed steps, fit, Timeline-mode Random, Eject and BeatLoopr never read the lock's result. The clip-transport lane is also the gate for other lanes (messages removal, recording), and it merges as one 14-stage block by default.",
      "evidence": "Plan l.628-629 and l.506-507 gate S5a/S5b/S6 on the SM verdict. S5a's own proves-list (TL-U50..52, U18d, l.660-661) and S5b's (TL-U53/54) have no lock case; lines are barLinesOf, not the lock (l.322-324). S4a's merge waits on M7/X6 (l.645-646), also a player question. Only S4c lists the lock cases TL-U44..49 (l.647-652). H-T4 default one merge (l.680).",
      "severity": "SHOULD",
      "proposed_change": "Order: S0, S1, S2, S3, S3h, S4a, S5a, S5b, S4d(Timeline part + Eject), SM-a. S4c alone waits on the verdict. Make H-T4's two merges the default: (1) undo/fires/pause/drag/hold, then (2) bars engine + panel + strip with the lock OFF (rate exact). The lock lands third. Lost: the visual gate runs before the lock exists, so W-states carry no slide; the lock gets its own two live rows."
    },
    {
      "id": "SC-3",
      "target": "TD5 / F38 / D2-6: pause is the LAYER's (Layer::paused, Intent::paused, 9-row table, D-7, check 6, Q74)",
      "claim": "The layer-level pause is built to satisfy a sentence Boris never wrote. His two sentences (same clip fired while paused stays paused; stays paused in the layer strip) hold with a clip-level paused flag, which is what Resolume does and what he said to mimic exactly. 'Another clip on a paused layer shows its first frame and waits' is Harmony's INFERRED reading R16.",
      "evidence": "binding-decisions.md:748 ('when you pause a clip and then fire it, it stays, paused'), :757 ('mimic exactly'), :806-807 (stays in layer strip paused; 'INFERRED; told to him as a reading'). Plan l.363-373 (F38), l.84-86 R6 (staff: pause is the clip's), l.988-990 K-3 admits the inferred basis, l.913 D-7 deviates from Resolume, l.966-968 Q74, l.385 'clear lifts the pause' rule, l.926-928 check 6.",
      "severity": "SHOULD",
      "proposed_change": "F38's runner-up: Relaxed<bool> on the Clip (runtime, not saved), set by the three buttons, read by pushIntent. A fire of another clip plays it, as Resolume. This drops the layer-pause lift-rules, D-7, Q74, check 6, TL-U61/U64 re-wording and the 'clear lifts pause' row, and shrinks TL-U60..64. Lost: the plan's own claim that a OneShot-ended clip and a paused clip are told apart still holds because paused is its own bit. Resolume's way ends up the shipped behaviour."
    },
    {
      "id": "SC-4",
      "target": "Section 7 questions 71, 72, 74, 76 put to Boris",
      "claim": "Four of six new questions are not his to answer: his own words or the plan's rules already decide them. 72 is contradicted by his words. 71 follows from his multiples-of-4 rule. 74 is a corner case. 76 contradicts 'mimic exactly' when answered by default.",
      "evidence": "binding-decisions.md:825-826 ('Timeline only shows bars. The only place we see beats is in the circle'): Q72 B (beats) cannot be what he wants, and plan l.564-565 F40 itself loses to that sentence. Q71: plan l.312-314 ('/2' enabled only when half is allowed, from his 'always work with multiples of 4', :812-814). Q74 (l.966-968) is an edge of the layer-pause table in SC-3. Q76: P1 (plan l.98-101) shows three buttons; l.972-974 keeps a fourth by default.",
      "severity": "NIT",
      "proposed_change": "Decide 71, 72, 74 in the ruling with a one-line reason; do not ask them. Keep 75 (his saved shows, out points move) and, if SC-3 is refused, 73. Fold 76 into the panel's 'mimic exactly' check and remove the fourth strip button unless he objects, since it is the only strip control with no Resolume twin. Lost: nothing he said."
    },
    {
      "id": "SC-5",
      "target": "TD9 / S4d+S4e: Random and BeatLoopr in BPM Sync, built from two sentences of documentation",
      "claim": "Boris's 'build random and beatloopr' is honoured, but the plan builds the BPM Sync halves on guesses (option list NOT DOCUMENTED, Distance meaning NOT DOCUMENTED) and adds machinery only they need. That machinery is beat-grid jump targets, a 1-beat lock period while Random runs, a 64-bit loop word with a press stamp, and a Catch Up seek ('the only seek the lock family makes'). One screenshot (RQ-1) from him would replace the guessed list, so the list may be built twice.",
      "evidence": "Plan l.544-563 (Random/BeatLoopr design), l.551-552 and l.554-556 ('Resolume's own list is NOT DOCUMENTED: request RQ-1'), l.1004 K-10, l.975-977 RQ-1. Tests TL-U71..78, TR28..30, MU-81..84 all ride on it. Lock period of 1 beat while Random is on (l.549-550) widens lockPeriodBeats (l.352-354).",
      "severity": "SHOULD",
      "proposed_change": "Ask for RQ-1 now. Build Random in Timeline (seconds, all documented) and Eject/Hold first. Build BPM-Sync Random and BeatLoopr after the screenshot arrives, lock simply OFF while either is active (no period-1 branch). Defer Catch Up's seek to the same step. Lost: Random and BeatLoopr in BPM Sync ship a stage later, and the lock is off while a loop is running (it re-slides on release, as the plan already says for Off)."
    },
    {
      "id": "SC-6",
      "target": "Section 5 gate size: 97 cases, MU-60..MU-88 (29 new mutants), 22 visual states x 5 critic seats, 29 live rows, M1..M8 twice",
      "claim": "The gate set is sized for a new engine, but much of it checks values the dumps already prove mechanically. The five critic seats re-read C1-C16, which are exact dump comparisons. Most live rows are 'GUARD (new routes)' that duplicate a unit case.",
      "evidence": "Plan l.775 (97 cases + 7 lints), l.776-789 (MU-60..MU-88), l.862-868 (W1..W22), l.869-884 (C1-C16 all mechanical, 'read from the two dump routes'), l.885-895 (five seats), l.806-847 (TR rows; TR26, TR27, TR28, TR30 restate TL-U37/U38/U72/U71), l.652 (M2/M4 re-run).",
      "severity": "SHOULD",
      "proposed_change": "Cut the critics to two (visual-design against his four screenshots, logic for the 'numbers pass but tell something false' states). Keep the mechanical C-bars. Drop live rows TR26/TR27/TR28/TR30 and their mutants unless the unit twin cannot see the render thread. Keep TR20-TR23 (lock/lines) and TR25 (pause). Lost: independent UX/graphic-design seat opinions, which Boris's own eyes replace at checks 1-21."
    },
    {
      "id": "SC-7",
      "target": "S4t (Tap keeps the bar) and H-T1: an analysis-thread change inside the clip lane",
      "claim": "S4t edits BPMTracker's realign path, a file the nudge lane may also claim. The plan makes it a lane stage that cannot start until another lane merges, yet it blocks nothing in this lane. It still moves the gate counts (G-U1 N >= 104/105/103).",
      "evidence": "Plan l.593-596 (waits for nudge lane), l.653-654 (S4t), l.793-794 (N variants), l.752-753 TL-U70. Code: BPMTracker.cpp realignPhaseToZero (253-258 region) bumps totalBeatCount_ only; Auto-detect advance of beatInBar_ is INFERRED (plan l.524-525, NOT VERIFIED l.1030-1032).",
      "severity": "NIT",
      "proposed_change": "Hand S4t to the nudge lane outright (they own the beat clock and BPMTracker), drop it and TL-U70 from this lane's count, and keep only SM-a's c column as the read-out of whether the bar slips. Lost: the Tap fix is no longer verified in this lane's gate. It is verified in the nudge lane's gate."
    },
    {
      "id": "SC-8",
      "target": "TD4 / F42: lockPeriodBeats = gcd(loop beats, 4) and x8/x16 lock",
      "claim": "The sub-bar lock periods exist only for speed 8 and 16 (and 12 bars at x8). They add a function, cases and measurement, and make O6 a hard STOP that blocks S4a's merge, to align a clip that loops every 1-2 beats. The list top is his, but locking it is not what he asked for.",
      "evidence": "Plan l.350-354 (period 4, 2, 1; examples: 4 bars at 8 = 2 beats, at 16 = 1), l.359-360 (x8/x16 measured in M7), l.645-646 and l.502 (O6: S4a waits, STOP AND ASK), TL-U42 (l.700-701). At speeds 0.. 4 with bars multiple of 4, B = 4*bars/syncSpeed is always a multiple of 4, so gcd = 4 and the plain bar lock already works.",
      "severity": "SHOULD",
      "proposed_change": "Lock only for syncSpeed <= 4 (period = the bar, always). At 8 and 16 the lock is off and the clip free-runs at its exact rate (still tempo-correct, may sit off the '1' by up to a beat). Drop lockPeriodBeats, the period-1/2 cases, MU-74's second arm, and make M7/X6 a report, not a merge gate. Lost: at x8/x16 the loop boundaries are not pulled onto the beat."
    }
  ],
  "strongest_point": "The one thing that must NOT be cut is the pre-lock measurement of how often Auto moves the '1' (X1) with its pre-registered decision table (O1-O3), together with the single-reader rule (showBarBeats() fed from frameSnap_). The risk is quiet: the stateless lock follows whatever the beat detector calls '1' (E10: beatInBar advances on detected beats, may re-lock every 16 beats), so each move sends every synced clip 6% off speed for up to 17-23 s. That is Boris's own question ('can you program this reliably') and a failure he would only see by ear, minutes in. The plan's structure is right to put this first. My attacks (SC-1, SC-2) say it can be answered by polling /api/bpm and need not hold the panel and strip hostage.",
  "citations_rechecked": true
}
```
