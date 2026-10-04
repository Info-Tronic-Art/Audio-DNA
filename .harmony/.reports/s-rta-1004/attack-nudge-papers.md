# ATTACK PAPERS -- blind council on plan-nudge.md (lane "nudge", the beat nudge; s-rta-1004, 2026-10-04)

Verbatim seat papers, 4 of 4 seats returned. Source: the workflow task text handed to the architect (one JSON array;
the dispatch states 39421 characters; as transcribed here: 39421 characters; the array is closed).
Pretty-printed only (json.dumps indent=2, ensure_ascii=False); no word was changed. Seats were blind to each other.
Plan attacked: .harmony/.reports/s-rta-1004/plan-nudge.md. Ruling: .harmony/.reports/s-rta-1004/ruling-nudge.md.

| seat | attacks | ids | citations rechecked by the seat |
|---|---|---|---|
| beat-clock | 7 | BE-1 (MUST), BE-2 (MUST), BE-3 (SHOULD), BE-4 (SHOULD), BE-5 (SHOULD), BE-6 (NIT), BE-7 (NIT) | yes |
| gates | 8 | GA-1 (MUST), GA-2 (MUST), GA-3 (MUST), GA-4 (MUST), GA-5 (SHOULD), GA-6 (SHOULD), GA-7 (SHOULD), GA-8 (SHOULD) | yes |
| stage-hands | 8 | ST-1 (SHOULD), ST-2 (SHOULD), ST-3 (SHOULD), ST-4 (SHOULD), ST-5 (SHOULD), ST-6 (SHOULD), ST-7 (NIT), ST-8 (NIT) | NO |
| scope | 8 | SC-1 (SHOULD), SC-2 (SHOULD), SC-3 (SHOULD), SC-4 (SHOULD), SC-5 (SHOULD), SC-6 (NIT), SC-7 (MUST), SC-8 (SHOULD) | yes |

Total: 31 attacks.

## SEAT: beat-clock

```json
{
  "seat": "beat-clock",
  "attacks": [
    {
      "id": "BE-1",
      "target": "NB1 rule 3 (beatInBar'/bar counts/downbeat' = S.beatInBar + w) and R2 'the nudge adds no new glitch'",
      "claim": "In Auto, when the tracker's phase wraps a few hops BEFORE the onset is scored, count and beatInBar are not advanced together, yet rule 3 adds the same phase-derived carry w to both. Result: the shifted beatInBar' dips by one for the lag hops (b, b-1, b, b+1), barCount' steps back (bw=-1), and downbeatDetected' goes true,false,true: a downbeat edge fired twice and a bar count running backwards. The tracker's own beatInBar never dips. The guards cover only totalBeatCount and totalBarCount.",
      "evidence": "BPMTracker.cpp:225-231 (count +1 at the phase wrap); :380-382 (beatInBar advances only in scoreBeat, on the onset hop); :253-258 (first-half realign adds no count); :343 (scoreBeat gated on beat detected). Plan lines 156-161 (rule 3), 164-174 (guards name only counts), 686-688 (R2). T-N5 (l.522-525) and T-N4 (Manual only) never assert beatInBar, barCount or downbeat edges. Worked case: D=+30 ms at 120 BPM, wrap-hop phase 0.02 gives x=-0.043, w=-1, so beatInBar'=b-1 while the tracker still holds b.",
      "severity": "MUST",
      "proposed_change": "Give BeatShift its own shifted beatInBar'/barCount' state that steps +1 exactly when count' steps +1 (re-seated from S only at a hop where S's count and beatInBar agree), instead of S.beatInBar + w. Extend guard 4 to beatInBar, barCount and the downbeat level. Add to T-N5: on the late-onset sequence, beatInBar' changes only on count' edges, never decreases except 3 to 0, and rising downbeat edges equal the tracker's bar count. That is the 'equal counts' test R2 promises but T-N5 does not contain."
    },
    {
      "id": "BE-2",
      "target": "NB1 rule 4 hold + NB4 Resync snap vs Pitfall 48 (trackerRequestSeq echo)",
      "claim": "After a Resync with D<0, the snap to 0 puts count' below F, so the hold republishes the old nine beat fields. The same snapshot still carries trackerRequestSeq = the resync's sequence number, because the seq is not one of the nine fields and is not held. A snapshot then claims a request its beat fields do not reflect. The take arm starts on 'first snapshot whose seq reaches the posted one' and reads startBeatInBar from it, so Record after Resync gets the wrong bar position. The hold lasts |D| ms, longer than a 120 Hz tick for any |D| above about 10 ms. R6 notes the half-second freeze but not this break. L9 only tests +40, the sign that never holds.",
      "evidence": "Pitfall 48 (docs/claude/pitfalls.md:105: 'a snapshot never claims a request it has not read'; RecorderHost takes meta.startBeatInBar from that snapshot). AnalysisThread.cpp:213 writes trackerRequestSeq at stage 5, outside BeatShift. Plan 164-168 (hold), 328-330 (snap on Resync), 700-703 (R6), 619-622 (L9 +40 only). Also: the resync branch (MainComponent.cpp:5783-5788) and the nudge-word store are two unordered writes, so a hop can latch the new seq and still read the old D.",
      "severity": "MUST",
      "proposed_change": "During a hold, publish the previous hop's trackerRequestSeq (or exempt the hop that applied a Resync or Tap from the hold, treating it as a declared discontinuity). In the resync branch, store the nudge word BEFORE requestResync, and have BeatShift load the word after the seq latch. Add an L9 arm at D=-250: the first poll with seq >= posted shows beatInBar 0 and beatPhase < 0.2."
    },
    {
      "id": "BE-3",
      "target": "Gates L3 / L4 / L2 / L9 (all Manual) and T-N4",
      "claim": "Every live row and the bar-line unit case run Manual, the one regime where the tracker's wrap drives count and beatInBar together (the predicted-beat regime). Auto, where F4 says they decouple, is covered only by T-N5, which checks counts only. L3 tests only 'Quantize Next Beat'. A Bar, 2 Bar, 4 Bar or Next Downbeat quantised fire is never checked on the shifted beat, although Boris's rule is that a quantised fire lands on the shifted beat. A bar-snapped fire fires on the count edge when beatInBar==0 and barCount parity holds. In Auto, whether that fires on the right beat changes as D crosses the onset lag, because the count' edge and the beatInBar' update move relative to each other.",
      "evidence": "Layer.h:470-478 (Bar/TwoBar/FourBar read beatInBar and barCount at the count edge); Autopilot.cpp:~76-91 (consume count, then processPendingTrigger(beatInBar, barCount)); BPMTracker.cpp:99-104 (Manual drives the beat from the predicted wrap); plan 595-601 (L3 Next Beat only), 602-605 (L4 Manual), 522-525. Boris (binding-decisions.md, 2026-10-04 12:31:21): 'a quantised fire lands on the (shifted) beat'.",
      "severity": "SHOULD",
      "proposed_change": "Add a unit case driving a real tracker in Auto with onsets 2 hops after the wrap, D in {0, +3, +30, +250, -30}, with a Bar-snap pending trigger. Assert the trigger fires only when beatInBar'==0 at a count' edge, and that its landing beat is the same for D=0 and D=+3. Add an L3 arm with Quantize Next Downbeat and a clip set to Bar snap."
    },
    {
      "id": "BE-4",
      "target": "NB2 take recorder: 'nudge' anchor, hold as 'a flat stretch'",
      "claim": "During a hold, raw = totalBeatCount + beatPhase stays flat, so current_.beat is flat. The only anchor written is at the moment appliedMs changes, which is the START of the hold. TempoMap::beatAt then assumes the beat advances at the anchor's bpm across the whole hold, until the next 'periodic' anchor up to 32 beats later. The take's beat to sample map is wrong by up to |D| ms of beats. No anchor is written at hold end, and raw is not less than lastRaw, so the 'reset' arm does not fire either. Separately, a held key moves appliedMs about every third hop, so one heap-allocating anchor (std::string why) per change goes into the take's JSON.",
      "evidence": "RecorderClock.cpp:56-64 (reset only when raw < lastRaw), :67-80 (anchors on bpm change or 32 beats); TempoMap.h:9-11 and :15-21 (beat(t) linear at the anchor's bpm; anchors carry std::string why); plan 245-248, 412 ('a hold is a flat stretch'), T-R1 (565) tests only 0 then 5.",
      "severity": "SHOULD",
      "proposed_change": "Write a 'nudge' anchor on the first tick where the published fields leave a hold (a BeatShift flag, or the tick sees raw flat then moving). Rate-limit nudge anchors to one per N ms of applied change. Extend T-R1: a hold of 20 hops, then assert beatAt(t_end) matches current_.beat."
    },
    {
      "id": "BE-5",
      "target": "NB1 rule 1 glide vs Boris 'not the tempo'; the text shows the TARGET",
      "claim": "A typed or pasted +/-500 glides for 2 s with the beat running at 0.75x or 1.25x. Every edge reader (quantised fire, autopilot, slideshow, routines) sees beats at 125% or 75% of the tempo for 2 s, while the top bar shows the target number, not the applied one, for the whole glide. Boris's words: 'A nudge just moves the placement of the downbeat in time not the tempo.' B3 tells him a typed value slides in 'about half a second', true only for 120. Fires that land mid-glide land on an intermediate nudge that the label never showed.",
      "evidence": "Plan lines 148-153 (glide 0.25 ms per ms; typed 500 takes 2 s at 0.75x / 1.25x), 268-270 (text shows the target, never the gliding value), 636 (B3). Boris verbatim, boris-feedback-backlog.md (BF55 block, 'You are exactly right...').",
      "severity": "SHOULD",
      "proposed_change": "Either state in B3 and the manual that a large typed jump runs the beat at 125% or 75% for up to 2 s. Or raise the snap threshold: snap when |D - Da| exceeds a bound such as 50 ms, accepting one hold or cap step in place of a long tempo distortion. Add a V-state that captures the label mid-glide."
    },
    {
      "id": "BE-6",
      "target": "NB1 rule 4 'NONE SKIPPED' cap and hold bound ('at most as long as the step back')",
      "claim": "A tempo step moves delta by D x (change in BPM) / 60000 beats at once, so a typed or REST BPM jump while D is large holds the beat for that whole step. 120 to 200 with D=+500 is a 0.67-beat hold, about 0.4 s of frozen phase, count and bar fields. The plan's bound 'under half a beat for a realign' and its T-N7 case (120 to 126, a hold of at most 0.06 beat) do not cover it. T-N6 tests the opposite sign only for Tap 60 to 200 with D=-500, not the held case.",
      "evidence": "Plan lines 164-173 (guard 4), 190-193 (tempo change), 526-531 (T-N6, T-N7 use 120 to 126 and 60/200 Tap). BPMTracker.cpp:586-605 (followExternalTempo changes lockedBPM at the hop start with no phase change).",
      "severity": "NIT",
      "proposed_change": "Add a T-N7 arm for a typed 120 to 200 with D=+500 that prints the hold length. State the bound as |D| x |change in bpm| / 60000 beats in NB1 and in B-row notes."
    },
    {
      "id": "BE-7",
      "target": "Section 7 'still open' list and NB4 / NB6 (Q49, Q50)",
      "claim": "The plan lists 49 and 50 as open and builds 'default A' for 49. Boris has answered both. 49: 'default', so the number reads 0 after Resync (the plan's choice is therefore settled, not a default). 50: first 'default', then changed to 'saved with the show AND kept on the computer for other shows'. NB6 says the nudge bindings follow whatever 50 decides and that no line changes. The two new binding targets will now be saved with the show as well as the computer, which touches the show's JSON and the new Composition key.",
      "evidence": "binding-decisions.md:864-871 ('49 default' -> reads 0; '50 default') and :878-881 ('50 change answer to B and also saved on the computer...', question 83 open). Plan lines 666-667, 385-386.",
      "severity": "NIT",
      "proposed_change": "Mark 49 as settled (BF63) and drop the 'one line changes if he answers B' wording. Re-read NB6 against 50-B: say where the BeatNudge bindings are written, and that appending BeatNudge last keeps both saved copies loadable."
    }
  ],
  "strongest_point": "BE-1. The plan's central promise is 'one shifted beat, fields consistent, no beat twice, no count backwards'. But rule 3 adds one phase-derived carry w to beatInBar and the bar counts, and the tracker in Auto advances count at the phase wrap and beatInBar only later, on the onset. So at the first-half-realign hop the shifted beatInBar', barCount' and downbeat level dip and re-fire, which the un-nudged tracker never does. The guards and the T-N5 / T-N4 / L4 gates only check counts, and only in the regime where they cannot fail.",
  "citations_rechecked": true
}
```

## SEAT: gates

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "Row L3 (quantised vs by-hand fire) and NB2's claim that quantised fire lands on the shifted beat",
      "claim": "L3 passes with BeatShift absent, or with the fire reading the unshifted beat. It is a live row that never reaches the code under test.",
      "evidence": "Plan lines 595-601: (a) asserts 'totalBeatCount == the count at the request + 1 and beatPhase < 0.15', both read from /api/features, which is the same published bus the quantised fire reads (NB2 lines 238-241: 'Nothing to change in those files'). Any self-consistent reader of one bus passes at D=0, D=+250 or with BeatShift deleted. Its only RED arm is a SELFTEST swapping (a) and (b) data. L10 (line 623) mutates only the sign (M1), and M1 shifts the bus itself, so the fire stays consistent with it.",
      "severity": "MUST",
      "proposed_change": "Compare against something the shift cannot move. In Manual 120 the tracker line is known from L2's two bracketing 0 arms. Require (a)'s fire at +250 to land at tracker-phase 0.5 +/- 0.15 beat, and the same fires at 0 to land at about 0. Add a mutant app with BeatShift bypassed (D forced to 0 in the analysis step) and require L3 and L2 to FAIL on it."
    },
    {
      "id": "GA-2",
      "target": "T-N10 and rule 5 IDENTITY; Row L1 ('bit-identical at the default')",
      "claim": "The zero-identity gate compares the lane with itself, not with 185147b. The function it needs does not exist. The padding claim is unprovable by it.",
      "evidence": "Plan 547-550: lane run with the 'BeatShift call compiled out by the test's switch' vs the lane run with it in. Any other lane change (the stage-5 write, the new field at 332, the accessor) is in both arms. The new write of beatNudgeAppliedMs happens in both arms, so line 179's claim 'same bits the cleared snapshot holds today' cannot be decided by it (line 722 says 'T-N10 decides'). 'The function AnalysisThread calls before the publish' does not exist: the pipeline is inline in run(), AnalysisThread.cpp:62-348 (acquireWrite :127, publishWrite :348). S2's file list (lines 448-456) names no extraction, and the BeatLead processHop split is DROPPED (line 749). L1 (line 585) compares 'the same PASS lines' as main, which are strings, not bytes.",
      "severity": "MUST",
      "proposed_change": "Add a golden: CRC of all 384 bytes per hop for a fixed click input, recorded from a 185147b build and pinned in the test, compared to the lane build at D=0. Name the seam S2 must cut in run() (a per-hop function), or state how the thread is driven in the test. Make L1 compare a features dump against a 185147b dump."
    },
    {
      "id": "GA-3",
      "target": "Visual gate states V6, V10, V11, V12, V13 (plan 303-311) and F11 'VERIFIED' window size",
      "claim": "Five states need a mouse (hover, click, press-hold) or a window resize, and the plan names no machine route or TEMPORARY hook for them. F11's 1280x800 premise is also contradicted by the code.",
      "evidence": "RIG-RULES.md:53-54: 'no synthetic input (UI states via REST / composition files / TEMPORARY env-var hook ...)'. V12 (tooltips on hover), V13 (the '-' held, pressed look) and V6 (editor opened by a click) need pointer input. S3 (plan 458-460) adds only a 'nudge' key (painted text) to /api/debug/ui_text, with no tooltip, editor or pressed state. V10/V11 need a resize, and grep finds no route or env var for window size in src/api/ApiServer.cpp or src/. src/Main.cpp:64-70 ('Open maximized') calls setBounds(display->userArea) after the setSize(1280, 800) the plan cites at MainComponent.cpp:2438, so the window never opens at 1280. Plan 83-85 marks that 'VERIFIED'. The pre-registered MUST 'a missing tooltip' (line 627) cannot be measured.",
      "severity": "MUST",
      "proposed_change": "Either state the TEMPORARY hooks, reverted and rebuilt with a strings check = 0, as in the probe-ui-files-rename '--hook cell-tooltip' precedent (testing-eyes.md:28), for V6, V12 and V13, and for a window-width hook (V10/V11). Or move those states to Section 6 as Boris-only. Add tooltip strings to ui_text's 'nudge' output so a machine reads them. Correct F11."
    },
    {
      "id": "GA-4",
      "target": "T-N8 (mutant M9) and rule 4 hold bound; risk R6",
      "claim": "M9 (origin clamp removed) survives T-N8 as written unless the Resync lands in a window the test does not pin. The Resync-with-earlier-nudge hold has no pre-registered bar.",
      "evidence": "Plan 174: totalBarCount' = max(F.totalBarCount, target), and F.total is already the tracker's T except in the first half beat of a bar. At D=+250/120 BPM, 'Resync applied mid-bar' (line 532) gives origin = T and count' = T (no violation), so dropping the clamp (rule 3, line 162) is not RED. INFERRED from rules 3-4 and not run. The -250 arm: 'within 1 hop + the hold' (line 534) uses the hold as its own bound. R6 (lines 700-703) says the test 'prints the hold length', which is a measurement with no bar. At -500 that is up to 500 ms in which Boris's Resync shows no '1' (his: 'If I press re-sync, then it does re-sync').",
      "severity": "MUST",
      "proposed_change": "Pin the Resync hop: tracker beatInBar 0, phase < 0.5, D=+250, and assert the unclamped build gives origin > count' (a precondition check that proves the arm can go red). State the bar before the run: hold <= |D| ms + 1 hop, and the first 'beatInBar 0' hop <= |old D| + 1 hop after the Resync."
    },
    {
      "id": "GA-5",
      "target": "Rows L4, L2/L7/L9 text checks; Boris's answer 46 (the beat circle moves with the nudge); REST route",
      "claim": "L4's bar is unpassable as worded, and it has no live RED arm for its own claim. Nothing machine-checks the label or the wheel against the model, or the REST clamp.",
      "evidence": "L4 (lines 602-605) requires '0 violations' that ui_text's 'Bar N' follows barCount, but the text is refreshed by a 15 Hz timer (TopBar.cpp:289-311, 67 ms) while features are polled live, so about 7 polls per bar edge disagree unless a lag allowance is stated (none is). The only RED arm is a synthetic backward count. M3/M4 (bar fields not carried) are RED only at unit level, and L10 runs M1 only. No row reads ui_text 'nudge' against GET ms (S3, line 459, adds the key but T-U1/T-U4 are pure helpers), so Pitfall 41/59 wiring (key, REST or load changes the model, the label follows) is unmeasured. POST /api/beat_nudge {'delta'} clamp at +/-500 and 'non-number is 400' (line 421) have no row. L7 stops at +/-3.",
      "severity": "SHOULD",
      "proposed_change": "State the label lag allowance (>= 150 ms) in L4. Add assert ui_text.nudge == offBeatText(GET ms) within 200 ms after every L2/L7/L9 step. Add one REST arm: ms 499 then delta +5 gives 500, {'ms':'x'} gives 400. Run M4's mutant app live against L4."
    },
    {
      "id": "GA-6",
      "target": "G-N1 counts, mutant list M1-M18, T-N5 bar",
      "claim": "G-N1's count bar is stated by the run it grades. Two RED arms sit outside the mutant set the gate builds. T-N5's tolerance hides a skipped beat.",
      "evidence": "Line 580: N = 'the lane's ctest -N count' and 'N >= main's count + the new cases'. Main's count is not pinned (APP-INVENTORY.md:31 says 1249; H-17(d) leaves 1249-vs-1252 open) and the number of new ctest entries is not pre-registered (a T-id can be several Catch2 cases). The gate runs 'M1-M18' (line 581), but T-N13's 'Mutant: count' computed in uint32' (line 543) and T-N14's 'std::vector member' RED arm (545) are unnumbered, so the gate never builds them. T-N5 (lines 522-525) accepts 'exactly 200 +/- 1', so one skipped beat in 200 passes, and no per-hop 'count' - F.count in {0,1}' check exists outside T-N6.",
      "severity": "SHOULD",
      "proposed_change": "Pin N_main by running ctest -N on 185147b now, and pin the exact new-case count. Number the T-N13 and T-N14 mutants (M19, M20) into G-N1. Replace +/-1 in T-N5 with a per-hop increment in {0,1} plus an exact final count against the tracker's own."
    },
    {
      "id": "GA-7",
      "target": "Row L2 +/-1 ms arms and L10 expectation",
      "claim": "The +/-1 ms live arms are at the instrument's noise floor, and L10 demands FAIL on all six arms including them.",
      "evidence": "L2 (lines 586-593) fits to HTTP polls of pos, which is a staircase on a 10.67 ms hop grid, with bar |measured - D| <= 1.5 ms and VALID only if 0 arms agree to 1.0 ms. R4 (696) admits the arm 'may be limited by the instrument'. RIG-RULES.md:59 says 'a bar whose teeth equal the drift is INFO, not a gate'. The plan's answer is a synthetic self-test with 2 ms jitter, which does not show the live rig resolves 1 ms. If the +1/-1 arms go INVALID, L10's 'FAIL on all six arms' (line 623) is unreachable and the row blocks. 'A flake verdict needs >= 5 runs per arm' (RIG-RULES.md:59) is not applied to L2, L3 ('30 of 30') or L4.",
      "severity": "SHOULD",
      "proposed_change": "Declare the +/-1 arms INFO live (T-N2/T-N3 carry the 1 ms proof) and make L10 FAIL on the four large arms only. Or pre-register the measured poll-jitter floor from the self-test on the live rig. Add the >= 5-run rule for the 30-of-30 row."
    },
    {
      "id": "GA-8",
      "target": "Section 1 goal quote and NB6 'His words' (T-B3, T-B4, L8, M16)",
      "claim": "A quote about the old Sync dial is cited as Boris's words on the nudge, and three gates pin a CC refusal he never ruled for it.",
      "evidence": "Plan lines 20 and 359 quote 'Key and pad is fine, knobs can skip the sync'. The record has it as his answer to Q13 'Nudging Sync from a controller' (boris-feedback-backlog.md:388, 414; binding-decisions.md:769-770), about the sync dial that he replaced at 12:08-12:09 (rulings-bf2.md H-17). He was never asked about key, pad or knob for the nudge. The plan builds a new action and overlay targets with 'bindingIsLive' dropping every CC (NB6 lines 366-368), and gates T-B3, T-B4, L8 and M16 defend that as if ruled.",
      "severity": "SHOULD",
      "proposed_change": "Label the key/pad/no-knob rule INFERRED from his Q13 answer on the old Sync dial, not his words on the nudge. List it under the H-a..H-d defaults, or ask it as a question 63-style default. Keep the CC refusal behind one function so a different answer is a one-line change."
    }
  ],
  "strongest_point": "GA-1 and GA-2 together show that the two rows meant to prove the Harmony constraint, 'quantised fire lands on the shifted beat' and 'bit-identical at 0', cannot fail. L3 reads the same bus the code under test writes. T-N10 compares the new code with itself through a per-hop function that does not exist in run(). Both would pass if BeatShift were deleted or wrong in the way the constraint fears.",
  "citations_rechecked": true
}
```

## SEAT: stage-hands

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "NB2 / F8 reader list; gate rows L1-L10; B1",
      "claim": "A BPM-synced clip that is already playing is not a beat-clock reader today, so a nudge cannot move it. The plan says every reader sees one shifted beat and never lists it.",
      "evidence": "Boris, binding-decisions.md:859-860: 'everything that is connected to BPM shifts forward or back. I mean everything.' Harmony's reading of that (backlog:522) names 'BPM-synced clips'. Renderer.cpp:1659-1665: BPMSync video only sets speed = videoBeats / beatDivision and never reads beatPhase. Renderer.cpp:1720-1730: image sequences set fps from bpm and run freely. Plan F8 (lines 63-71) and NB2 never name either. Plan B1 itself calls 'only some things move' WRONG.",
      "severity": "SHOULD",
      "proposed_change": "Add a NB2 line naming clip playback in BPM-sync mode. A clip fired on the beat takes its phase from the published beat (MainComponent.cpp:4866-4885), but a running loop stays where it is. Say in the plan that moving a running loop depends on the transport lane's bar lock reading the shifted bus. Add a live row: nudge +250, then check that a synced clip's loop edge moves or that the plan's own text says it does not. Tell Boris, since his words say 'everything'."
    },
    {
      "id": "ST-2",
      "target": "NB1 rule 4 hold + NB4 Resync + R6 + T-N8",
      "claim": "After Resync from an 'earlier' nudge, the shown beat, bar and downbeat are frozen for up to |D| ms (500 ms at -500) before the '1' he pressed on appears. No bar caps that freeze.",
      "evidence": "R6 (plan lines 700-703) admits 'half a second of frozen beat phase'. The hold republishes F unchanged, so beatInBar and downbeat do not show the Resync state (rule 4, lines 165-168). T-N8 (lines 532-534) says 'within 1 hop + the hold' and sets no maximum. 'Printing' the hold is not a bar. Boris said 'If I press re-sync, then it does re-sync' (binding-decisions.md:855) and chose 49 default, 'Resync is a fresh start'.",
      "severity": "SHOULD",
      "proposed_change": "Give T-N8 and live row L9 a hard bar: the hold after a Resync is at most the old |D| plus one hop, asserted, not printed. Consider letting a Resync (the snap-sequence bump it already causes) skip the hold and publish the tracker's fields once, accepting one backward count that is flagged and tested. Otherwise state in B6 that at large negative nudges the 1 lands late."
    },
    {
      "id": "ST-3",
      "target": "NB5 load hook (snap=true) + R28 'saved with the show'",
      "claim": "Opening another show mid-set silently overwrites the room's number and snaps the beat by up to 500 ms, with no glide. The plan's own NB7 argument (the nudge is 'where he stands in a room on a night') says this is wrong.",
      "evidence": "Plan lines 346-350: the swap calls setBeatNudgeMs(..., snap=true), and rule 1 (line 150) says a snap applies at once. A show saved at home reads 0 and old shows read 0. R28 'every show remembers it's sync' (backlog:234, 258) was a dial-era answer: backlog:247 says 'a show saved at home resets the room to 0'. He was not asked it for the nudge, and the dial it applied to was replaced (backlog:439). NB7 refuses to replay an old room's correction onto a new room, yet NB5 does exactly that on every open.",
      "severity": "SHOULD",
      "proposed_change": "Ask Boris the new-numbered question 'opening a show: does its number replace the one you set in this room?', with the default he chose for the dial stated. Whatever he answers, make the load apply with snap=false (a glide) so the picture does not jump. Add the line that changes (the load hook's argument) to NB5. Add to L6 an assertion on the beat jump size after a load."
    },
    {
      "id": "ST-4",
      "target": "NB3 right-click = 0 (H-b) + typed-value editor",
      "claim": "A right-click anywhere on the small text silently zeros the room correction. It has no confirmation and no undo, and Boris never asked for it. The editable label also has no stated rule for returning keyboard focus to the performance keys.",
      "evidence": "Plan lines 277-279 and H-b (line 671): right-click on the text = 0, built by default. A number that returns to 0 silently is the failure Boris is guarding against. The existing BPM field (TopBar.cpp:149-153) never releases focus on Enter. While a text editor is focused, digit and letter keys go to it (INFERRED, JUCE behaviour, not run), so bound clip keys would type into the label instead of firing. The plan has no focus-return rule, and V6 only captures the editor open.",
      "severity": "SHOULD",
      "proposed_change": "Drop right-click-reset. Zero is reached by typing 0 or by Resync, both visible. Specify that Enter, Esc and focus loss all end the edit and hand keyboard focus back to the main component, and add a model-level or ui_text probe that the bound-key path is live again after an edit. Otherwise the VJ's keys die mid-set."
    },
    {
      "id": "ST-5",
      "target": "NB3 hold repeat + NB6 held key / pad (question 63 default A)",
      "claim": "The hold speeds are too slow for a live correction, and a pad is almost unusable. A button hold gives 20 ms per second and a key about 30 ms per second, so crossing +500 takes 17 to 25 seconds. A pad gives 1 ms per hit, so a 40 ms fix is 40 hits.",
      "evidence": "Plan lines 271-275: setRepeatSpeed 400 ms then every 50 ms. Lines 382-384: one pad hit = 1 ms; whether a held pad repeats is question 63, default A (no). Boris: 'Key and pad is fine' (backlog:388), which implies a pad should work. R7 (lines 704-706) already admits a UI-tick repeat is the fallback if OS key repeat is not delivered. Key auto-repeat reaching keyPressed is ASSUMED (F14).",
      "severity": "SHOULD",
      "proposed_change": "Build the repeat once, on the UI tick, for the key, the on-screen button and a held pad (the pad only needs the note-off already polled for Momentary bindings), so 63 A and 63 B differ by one flag. Add a modest acceleration (1 ms steps, ticks quicken after about 1 s) so +-500 is under about 5 s. Do not rely on OS key repeat. Add a model test of the repeat schedule."
    },
    {
      "id": "ST-6",
      "target": "NB1 sign rule / NB3 text / question 61",
      "claim": "'off beat by +12 ms' carries no direction word, and the plan has no single place where the sign flips. If Boris answers 61 B, the beat shift, the text, three tooltips, the binding step signs, the manual wording and the tests all change by hand.",
      "evidence": "Plan line 142 (D > 0 = later), the label (lines 268-270), the tooltips (272-275), the binding step (362-365) and the manual step 4 (line 483) each encode the sign separately. The only constant named is kNudgeAfterTempo... for layout, not sign. The text reads like 'the picture is 12 ms off, ahead'. After he fixes an early picture with '+', it reads '+12' for a fix he made. The plan notes 61 B only in section 7.",
      "severity": "SHOULD",
      "proposed_change": "Name ONE constant (e.g. kPlusMeansLater) read by BeatShift's delta, offBeatText, the tooltips, targetNudgeStepMs's sign and the overlay labels. Make T-N1 and T-U1 run for both values so answer 61 B is a one-line change. Add the line that changes to section 7 as the plan does for 49."
    },
    {
      "id": "ST-7",
      "target": "NB3 15 Hz wheel / Boris's answer 46",
      "claim": "His answer 46 ('It moves with the nudge') is met only in principle. The circle's lit quarter is sampled at 15 Hz with up to 67 ms of jitter, larger than any nudge he makes, and no gate checks it follows.",
      "evidence": "Plan lines 294-299 and B10 (649-650): 'a few ms cannot be seen on it' and the timer is not raised. TopBar.cpp:297-311 is the 15 Hz timer. Plan rows L4 and V1-V17 read 'Bar N' and static captures, not the wheel. Pitfall 57 only forbids a timed repaint of the whole window, not one that fires on a change of the lit quarter. H-d files a faster wheel as an idea.",
      "severity": "NIT",
      "proposed_change": "Read the lit quarter and the nudge on a faster tick, but repaint only when the lit-quarter index changes (allowed by Pitfalls 57 and 59). Or state to Boris, in words next to the question 46 reading, that the circle is a coarse indicator and the number is the instrument. Add one probe row that the wheel's model beat equals the published beat after a +250 nudge."
    },
    {
      "id": "ST-8",
      "target": "NB4 / NB7 Resync clears the number; take recorder anchor",
      "claim": "A nudge change writes a recorder 'nudge' anchor on every hop where appliedMs differs, so every glide or held key adds an anchor per hop (about 94 per second) to the take. T-R1 tests only the two-tick case.",
      "evidence": "Plan lines 245-248 (anchor on a change of beatNudgeAppliedMs), rule 1 (lines 148-152: the applied value moves every hop during a glide). RecorderClock.cpp:73-77 and :95-98 show anchors are appended per event with no rate limit (tempo_.append at :7). A held key for 17 s or a typed 500 glide (about 190 hops) writes hundreds of anchors, while 'periodic' anchors are meant to be every 32 beats.",
      "severity": "NIT",
      "proposed_change": "Write the 'nudge' anchor only when the applied value has been steady, or when |change| exceeds a threshold, or once per glide end. Add a T-R1 case: a 2-second glide writes at most 2 anchors. Keep the anchor on a snap."
    }
  ],
  "strongest_point": "ST-1: Boris said 'everything that is connected to BPM shifts... I mean everything', and the plan's reader list omits BPM-synced clip playback. That playback runs on speed only (Renderer.cpp:1659-1665, 1720-1730). A nudge therefore moves the effects but leaves a running synced loop where it was, which is the 'only some things move' outcome the plan's own B1 calls WRONG.",
  "citations_rechecked": false
}
```

## SEAT: scope

```json
{
  "seat": "scope",
  "attacks": [
    {
      "id": "SC-1",
      "target": "NB1 rule 1 (the glide Da, 0.25 ms per ms, snap sequence packed in the atomic word) with T-N11, M5, M7",
      "claim": "The glide is a mechanism bought only for a typed jump and a load; it also makes the beat run at 0.75x or 1.25x for 2 s, which sits badly with Boris's \"A nudge just moves the placement of the downbeat in time not the tempo.\" A 1 ms press needs no glide.",
      "evidence": "plan-nudge.md:148-152 (\"a typed 500 glides for 2 s with the beat running at 0.75x ... or 1.25x\"; a 1 ms press is \"fully applied at the next hop\"). Boris: binding-decisions.md:855-858 area, \"not the tempo\" (line 842-843 text). The snap sequence, the 16-bit packing, `stepApplied`, `appliedMs` and T-N11/M5/M7 (plan:537-541) exist only for the glide. The hold and the forward cap (plan:164-173) are needed anyway for a Tap or a first-half realign.",
      "severity": "SHOULD",
      "proposed_change": "Cut the glide. Apply D at the next hop (Da = D), drop the snap sequence (the word is just the signed ms), drop T-N11, M5, M7 and the B3 row. Keep `beatNudgeAppliedMs` as the probe's witness. Lost: a typed jump of several hundred ms freezes the beat for up to |D| ms (the existing hold) instead of sliding. If that is unacceptable, cut typed entry instead (SC-2); a 1 ms step never needs a glide."
    },
    {
      "id": "SC-2",
      "target": "NB3 typed value (editable label, parseOffBeat) and right-click = 0 (H-b)",
      "claim": "Boris asked for a nudge forward or back and the text \"off beat by [+/- X] ms\". A typed-number editor, a parser, a tolerant parse of \"off beat by -7 ms\", and right-click-resets are not in his words; Resync already sets 0.",
      "evidence": "Boris, binding-decisions.md:836-838 (\"nudge main bpm forward or back ... displayed as 'off beat by [+/- X] ms'\"). Plan:274-279 (editor, Enter commits, Esc cancels, right-click = 0), T-U2/M13 (plan:554-556), V6, H-b (plan:671: \"drop it if the visual gate's seats call it hidden behaviour\", i.e. the plan already doubts it). Resync = 0 is Boris's own rule (binding-decisions.md:868).",
      "severity": "SHOULD",
      "proposed_change": "Ship a read-only label plus the \"-\" and \"+\" buttons with repeat. Remove `parseOffBeat`, T-U2, M13, V6, the editor tooltip text and right-click. Lost: a big jump takes about 25 s of held \"+\" (20 ms per second, plan:271-273) or one Resync; if Boris wants it he says so. Do not carry H-b as a default-built hidden gesture."
    },
    {
      "id": "SC-3",
      "target": "S4 / NB6 (refusing a CC: bindingIsLive, learnMidiCC, titleText, selectAt, bindingTag, stateForTests, midi_learn + bind_overlay debug routes, the learn-title dash fix, the 15-label baseline)",
      "claim": "\"Knobs can skip the sync\" is permission to skip knobs, not an order to build a refusal path. Most of S4's size is the refusal machinery and H-16 clean-up that is not the nudge.",
      "evidence": "Boris, boris-feedback-backlog.md:388 (\"Key and pad is fine, knobs can skip the sync\", said of the dial, Q13; carrying it to the nudge is INFERRED). Plan:366-377, 388-400, 614-618, T-B3, T-B4, T-B6, M16, V14-V17, L8. A CC bound to a nudge target already yields delta 0 in `beatNudgeDeltaMs` (plan:364-365), so it is a harmless dead binding. The learn title's em dash (MidiLearnOverlay.cpp:95) and the 15 old labels are not caused by the nudge; H-17(e) (rulings-bf2.md) sent them to \"carry or drop\".",
      "severity": "SHOULD",
      "proposed_change": "Keep only Action::BeatNudge (appended last), `targetNudgeStepMs`, `beatNudgeDeltaMs`, the two overlay targets, the `handleBindingAction` case, T-B1, T-B2, T-B5/M17 and L7 (through a single `binding_action` debug route). Drop bindingIsLive, learnMidiCC, the midi_learn/bind_overlay routes, the dash fix, the ML-1 baseline, T-B3/B4/B6, M16, V14-V17, L8. Lost: a user can learn a knob onto \"Beat later\" and it silently does nothing. Add a one-line note to the manual instead."
    },
    {
      "id": "SC-4",
      "target": "NB2 RecorderClock \"nudge\" anchor (T-R1, M18)",
      "claim": "The anchor fires on every change of beatNudgeAppliedMs, which under the plan's own glide changes on every hop, and on every press of a held key. It is a file-size and write-rate defect, and the clock already absorbs the beat moving.",
      "evidence": "Plan:245-248 (anchor when `beatNudgeAppliedMs` changes), plan:148-152 (the applied value moves 2.667 ms per hop for up to 2 s, about 190 hops), plan:271-273 (a held button is 20 steps per second). RecorderClock.cpp:56-66 already absorbs a backward step (\"reset\" anchor) and :68-72 anchors on a bpm change; :78 re-anchors at most every 32 beats (RecorderClock.h:77). The plan cites 62-70 and 73-77; the real lines are 56-66 and 68-72.",
      "severity": "SHOULD",
      "proposed_change": "Either drop the anchor (error is at most |D| ms for up to 32 beats; the take stamps follow the nudged beat), or anchor on the TARGET changing (one per press), never on the gliding value. Drop T-R1/M18 if dropped. If SC-1 is taken, per-press anchors are acceptable and the file shrinks by one more touch point."
    },
    {
      "id": "SC-5",
      "target": "Section 4 order: S0 before S1; S3 and S4 after Harmony's S2 rows; other lanes waiting",
      "claim": "S0 is a read-only capture of 185147b and S1 is a pure class with unit tests. Neither depends on the other, yet the order chains them. S3 (top bar) needs only `setBeatNudgeMs`, not S2's live verdict. The plan's own disjoint-hunk rule means the clip-transport, messages and recording lanes need not wait on the whole lane.",
      "evidence": "Plan:440-447 (S0 \"no source change\"; S1 \"Nothing is wired\"), plan:490 (Order: S0 -> S1 -> S2 -> Harmony rows -> S3 -> S4 -> rows -> VG -> S5 -> merge), plan:500-502 (shared files, disjoint hunks, the second lane takes main in). S0 only feeds NB3's width numbers (plan:282-292), first used at S3.",
      "severity": "SHOULD",
      "proposed_change": "Run S0 and S1 in parallel (S0 captures main in a throwaway worktree). Let S5 docs start with S3. Let other lanes branch from main now and take this lane's merge as step 0 (the plan already prescribes that for the output-settings lane); the only real blocker is the one FeatureSnapshot field at offset 332. Lost: nothing but a stricter sequence."
    },
    {
      "id": "SC-6",
      "target": "Section 7 questions 63 (and the shape of 62)",
      "claim": "Question 63 (does a held pad repeat) is an implementation detail with a default of \"no\"; putting it to Boris spends his attention on something he did not raise. The plan itself says nothing waits on it.",
      "evidence": "Plan:663-665 (63 A default: one hit = 1 ms; B: a held pad keeps moving). The pad sends one note-on (plan:383-384), and B would need new repeat code (plan:704-705, R7). Boris named \"Key and pad is fine\" (boris-feedback-backlog.md:388) without any held-pad wish.",
      "severity": "NIT",
      "proposed_change": "Move 63 to the \"For Harmony\" list (H-e: held pad = one step, no repeat) and ask only 61 (which way is plus, his to say) and 62 (his UI). Lost: nothing; the answer is a one-line change later."
    },
    {
      "id": "SC-7",
      "target": "Gate row L5 (inject path is not shifted)",
      "claim": "L5 cannot fail. In test mode no analysis thread runs, so BeatShift cannot touch an injected snapshot; the row's only RED arm is a probe self-test, not the product.",
      "evidence": "Plan:75-77 (F10: test mode never starts the analysis thread; the TestServer writes the bus), plan:253-256 (\"in test mode there is no analysis thread, so BeatShift never runs\"), plan:606-607 (L5: POST ms 100, inject 0.25 / 2 / 7, read the same values; RED arm: \"SELFTEST expecting 0.05\"). Pitfall 68 text already states the rule (plan:476-478).",
      "severity": "MUST",
      "proposed_change": "Drop L5 and its test-mode `analysisRunning` field; keep the Pitfall 68 sentence. A row that passes by construction is a gate that cannot fail. If a check is wanted, make it a unit assertion that `AnalysisThread` is the only caller of `BeatShift::apply` (a grep, as T-N14 does)."
    },
    {
      "id": "SC-8",
      "target": "NB8 production route /api/beat_nudge (GET + POST ms/delta) and /api/features key",
      "claim": "A new production REST write surface is built for a feature where the plan refuses OSC because \"nobody asked\". The same reasoning applies to a public mutator; the rows need it only as a probe handle.",
      "evidence": "Plan:419-423 (production API beside set_bpm, POST {\"ms\"}/{\"delta\"}), against plan:672 (H-c: OSC \"NOT built (nobody asked)\") and plan:729 (\"OSC for the nudge\" out of lane). Boris's words (binding-decisions.md:836-838, 855-862) name no remote control of the nudge. Live rows L1-L4, L9 use it as the setter.",
      "severity": "SHOULD",
      "proposed_change": "Make the setter a TEST-SERVER debug route (like binding_action) and keep only the read-only `beatNudgeAppliedMs` in /api/features. Lost: an outside tool cannot set the nudge on a production build. If REST stays, drop the `delta` form."
    }
  ],
  "strongest_point": "The one thing that must NOT be cut is NB1's single publish-time step: BeatShift rewrites the nine beat fields on the analysis thread just before publishWrite() (AnalysisThread.cpp:348). Its guards (hold so no count runs backwards or beat fires twice, forward cap so none is skipped, bar counts carried with the beat) stay, and so does T-N10, the memcmp proof that a nudge of 0 leaves all 384 bytes identical. This is the only design that gives Boris's \"everything connected to BPM shifts ... I mean everything\" with one shifted beat for fifteen-plus readers (plan:63-71, 129-134). The Resync-reads-0 line (plan:326-330) is cheap and also stays. Everything the attacks cut sits around this core.",
  "citations_rechecked": true
}
```

