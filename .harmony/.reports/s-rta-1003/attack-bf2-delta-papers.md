# Blind council seat papers -- plan-bf2-delta.md (s-rta-1003)

Verbatim seat output (3 of 3 seats returned), pretty-printed, one section per seat.
Written by the architect BEFORE ruling; nothing below is edited, reordered or summarised.
Plan attacked: .harmony/.reports/s-rta-1003/plan-bf2-delta.md
Ruling: .harmony/.reports/s-rta-1003/ruling-bf2-delta.md

## Seat: timing (9 attacks; citations_rechecked=true)

```json
{
  "seat": "timing",
  "attacks": [
    {
      "id": "T1",
      "target": "B4 (iii) / G6 perf A/B and its TRIPWIRE (plan 263-281, 544-546)",
      "claim": "The G6 tripwire cannot fail at +500, and cannot fail at -500 as the driver is specified. The metric is blind to the LATE path and the driver never engages the EARLY path. The new bar (1.0 point or 1.0 ms) is also 3x the ruled 0.3 point. This registers a gate that cannot fail.",
      "evidence": "(1) The analysis cpuLoad is timed only inside processHop: AnalysisThread.cpp:211 sets pipelineStart and :469 reads elapsedUs. The delay-line drain, resampler pull, commitBack and frontDue gating are in run() at :124-162, outside that window. At +500 the dial's only cost is therefore invisible to cpuLoad. (2) BeatLead is inside the window (:455), but BeatLead.cpp:148 gives deltaT = bpm>0 ? lead*bpm/60 : 0 and :156 returns at once if not engaged and deltaT==0. The driver (plan 269-272) uses mic input and sets no tempo, so in a quiet room bpm is 0 and the -500 arm runs the identity path. (3) ruling-bf2.md:485 bar is max(0.3 pt, 2xSD). The plan (274-276) replaces it with 1.0 pt / 1.0 ms. cpuLoad is an EMA (AnalysisThread.cpp:473-474) of elapsed/hop x 100.",
      "severity": "MUST",
      "proposed_change": "Drive G6 from the click-track WAV (or manual 120 BPM via set_bpm) so BeatLead is engaged and the tracker locked, and use the same fixed input in every arm instead of the mic, which also cuts within-arm SD. Measure the delay line with a quantity that sees it (syncHopDelayMs_/iteration time from the witness, or R1a's iteration start-to-start at +500) instead of cpuLoad. Keep the ruled 0.3 pt as the INFO threshold and say plainly that the 1.0 tripwire only catches a gross regression. Do not call it a gate for the LATE path."
    },
    {
      "id": "T2",
      "target": "B3 / R7b check (ii) bar [-512, +2880] samples (plan 222-226, 519-522)",
      "claim": "The new absolute check has a one-lattice-step margin on its low side. A correct app also fails it whenever k lands on the arm's one late stray. Its upper bound also cannot see any gesture error under about 40 ms.",
      "evidence": "I recomputed from the saved S3 run S3LOG/out-green2/run-20261002-212823-74596/probe-sync.json (offsMs). B = first 0-arm median = 30.667 ms = 1472 samples. In the D=100 arm, e_k-B has min -448 samples (10 of 121 markers sit there; 12 more at -384), against the bar -512: margin 64 samples = one lattice step, and B itself moves one step between arms (medians 30.67/29.33/30.67). One marker per arm sits at +3264 > 2880 even with zero poll delay. The saved gestures were stamped in the marker's own tick (diff 0/0/0 and 0/512/0 in the two green runs), so delta is about 0. Three gestures x 1/121 gives about 2.5% spurious FAIL per run from the stray alone, plus the low-side tail. The S2 behaviour reads about +4800 (+100 ms), so a bar anywhere from 3,000 to 4,300 separates it, and the plan's 2,880 is needlessly tight.",
      "severity": "SHOULD",
      "proposed_change": "Widen to [-1,536, +3,840] samples (-32/+80 ms) expressed as ms x the take's segment rate, as the existing R7b does (probe-sync.py bar = 0.060*rate). Take B as the median of all 0 arms, not the first. Judge (ii) as 'at least 2 of 3 gestures inside' so one stray cannot fail a correct app. State that (ii) resolves only an error of 40 ms or more; R7 carries the 1 ms claim."
    },
    {
      "id": "T3",
      "target": "B1 chosen R7 statistic: teeth claim 'one-step regression fails about 9 times in 10' (plan 166-175)",
      "claim": "The claim is false by the plan's own known-bias term. A +1 lattice-step (1.333 ms) regression fails about 55% of runs at the bias the saved run shows. It fails only about 20% at the bias the plan itself expects (-0.5). A -1 step always fails. The gate is asymmetric and detects reliably only about 2 steps.",
      "evidence": "Plan 166-169 computes 9/10 with SE 0.23 and a zero-centred null. Plan 173-175 then says dbar is expected -0.2 to -0.5. I applied the plan's own trim (drop >1024 samples from the arm median) to S3LOG/out-green2: trimmed means 30.62/30.09/30.18, so dbar = -0.31 ms. A 2000-draw bootstrap per arm gave SD(dbar) 0.195. The value dbar = -0.31+1.333 = +1.02 fails P=0.55. At -0.5 bias, +0.83 fails about 20%. The sign is also not systematic: S3 run 1 gives mean-difference +0.67 (31.98 vs 31.39/31.22, plan E5) and run 2 gives -0.25.",
      "severity": "SHOULD",
      "proposed_change": "Strike the '9 in 10' sentence and the 'expected slightly negative' premise. Say the gate resolves a 2-step (2.67 ms) regression with near certainty and a 1-step one about half the time. Either keep 1.0 and accept that, or add a pre-registered 0-vs-0 control (the d between the two neighbouring 0 arms) and judge dbar minus the control. Do not re-centre by looking at the green runs. Also compute SE with the shared-neighbour covariance (each 0 arm feeds two d_i), not SD(d_i)/sqrt(10)."
    },
    {
      "id": "T4",
      "target": "Section 8.1 / 8.2, K1, S6 'Skipped if Boris answers no', R9 bracket (plan 319, 464, 490, 526, 622-633, 637-643)",
      "claim": "The plan treats Boris's answers on the wheel and on the composition venue as open. Both were already answered 'yes'. The conditional branches (S6 skipped, R9 'as ruled', 'put to Boris') are dead and the gate text still carries them.",
      "evidence": "binding-decisions.md:669-670: 'Q9 the beat wheel stays where he tapped, only the picture is shifted - Boris: \"yes\". Q10 opening a composition loads its saved venue and sync value - Boris: \"yes\"' (recorded 14:03:17). boris-feedback-backlog.md:209-212 repeats it ('lane/bf2's wheel follows the shifted beat today: must change'). plan-bf2-delta.md was written 14:01:57, about 80 s earlier. K1 ('Building S6 on a reading of his words') is therefore settled in its favour.",
      "severity": "SHOULD",
      "proposed_change": "Delete 8.1/8.2 and the 'if no' branches. Make S6, the un-shifted wheel and R9's D+-15 ms form unconditional. Cite Q9/Q10 as the authority. Keep 8.3/8.4 only if still open."
    },
    {
      "id": "T5",
      "target": "B6 (iii)(b) wheel + K3 (plan 360-361, 647-648)",
      "claim": "At a LATE dial the un-shifted wheel contradicts Boris's 'stays exactly where he tapped'. After a Tap the wheel keeps the old phase for D ms (up to 500) and then jumps. The plan files the fix as a follow-up, although the lane is being re-planned around the very sentence it breaks.",
      "evidence": "binding-decisions.md:669 (Q9 'yes'); boris-feedback-backlog.md:152-154 ('beat wheel unshifted'). Plan 360-361: 'at a LATE dial the wheel answers a Tap / Resync D ms after the press'. The cause is S6 (plan 324-326) applying the request at P+late, with the wheel reading the post-tap snapshot. Plan R12 gates only the steady state (20 pairs, +-0.05 beat) and the Resync at +-40 ms around k x 500, with no gate on the press-to-wheel latency.",
      "severity": "SHOULD",
      "proposed_change": "Give the TopBar the press time (it already receives Tap/Resync on the message thread) and let musicBeat() anchor a pending Tap at once: the wheel shows beat 0 at P, and the snapshot takes over when trackerRequestSeq catches up. If that is out of scope, add an R12 clause that bounds press-to-wheel latency (<= D+50 ms) and put the limit in the Boris page text next to the 'exactly where you tapped' wording, so the YES he gave is not silently narrowed."
    },
    {
      "id": "T6",
      "target": "B2 replay-end decision (A) (plan 188-208)",
      "claim": "The plan calls the end burst rare ('only for a take from another room', 'a 0-0.5 s tail'). It is the common case of the per-room dial that Boris asked for: record at home (dial 0), perform at the venue (+D). Every such with-audio replay and every routine slice bursts its last D ms of points onto the end instant. The claim 'nothing is lost' is untested for ordering and for routines.",
      "evidence": "RecorderHost.cpp:456-460 stamps minus the dial at the RECORD time. :578 shifts pos by the dial at the REPLAY time, so any D'>D_rec leaves stamps in (end-(D'-D_rec), end]. :583-584 fires them with advanceTo(playEndPos_) at the end edge, so they land D'-D_rec early, all at once. The routine feature (CLAUDE.md: 8-slot bank, loop/once, restore-then-replay on the next bar) replays takes through this same end edge. Binding words (backlog:10-11): 'adjust the delay ... for each room so we can set it per room'. The plan's tests (206-207) are the existing tail case plus M1/M2 only.",
      "severity": "SHOULD",
      "proposed_change": "Keep (A), but replace 'only a cross-room take' with 'any take replayed at a larger dial than it was recorded at (rehearse at 0, play at +150 is the normal case)'. Add a unit case: record at D_rec, replay at D'>D_rec, assert the burst fires in stamp order, its count equals the points in (end-(D'-D_rec), end], and a looping routine's restore-then-replay still starts clean. Add to 7.4 'record at 0, replay at +300 as a routine, once and looped'. If the burst is visible on a once-routine, take (B) using the wallNow already in the tick."
    },
    {
      "id": "T7",
      "target": "B4 (i) origin-cap acceptance and its disclosure (plan 233-255, 610-612, 662-663)",
      "claim": "The disclosure says multi-bar shapes 'sit one bar ahead' and Boris check 7.5 says wrong = 'stutters backwards'. What the cap actually does at the re-base hop is a FORWARD jump of the fold of (4 - delta) beats, 2.3-4 beats (delta <= 1.67), because the bar term is held while beatInBar steps back. Nothing in the gate set sees a forward jump.",
      "evidence": "BeatLead.cpp:205-209: the cap forces barsSinceResync back to 'keep', so the bar term does not drop. The re-based beatInBar/beatPhase still step back. OscillatorSignal.h:56-60 and EnvelopeSignal.h:55-59 fold 4*barsElapsed + beatInBar + beatPhase, so the fold moves +4 - delta where Appendix A would dip by delta. For a 4-bar shape (16 beats) that is a 15-25% cycle leap; a one-shot envelope can end early, and the leap can cross a cycle boundary. Probe 'did not move back' (probe-sync.py:799-817) checks only backward motion. T-L6 pins only barsSinceResync >= before (bf2.md:449-450, '2 >= 3').",
      "severity": "SHOULD",
      "proposed_change": "Rewrite the disclosure: 'at a capped re-base a 2/4/8-bar shape leaps forward by about a bar; Appendix A would retrace up to the lead'. Add one unit assertion bounding the fold step at a capped re-base (0 <= step <= 4 beats) and add to check 7.5 'Wrong also: a 4-bar shape jumps ahead when you tap'. Rule between cap and Appendix A on that basis, with the runner-up named."
    },
    {
      "id": "T8",
      "target": "B5 G1 [timing] (plan 283-296)",
      "claim": "One quiet pass is not evidence of margin. The case is a wall-clock case with a known thin bar and no serialization, and the plan's pass line is a single run.",
      "evidence": "test_analysis_sync_thread.cpp:664-693: pair bar mean(38)-mean(37) in [0.7,1.3]. The plan itself (293-295) records 0.649/0.678 under a 28% process, a miss of 0.02-0.05. tests/CMakeLists.txt:3488 registers the case through catch_discover_tests with no RUN_SERIAL (grep RUN_SERIAL tests/CMakeLists.txt returns nothing), so under ctest -j it competes with every other suite. The final G1 in the plan (505-510) is a whole-suite run.",
      "severity": "SHOULD",
      "proposed_change": "Register the [timing] case RUN_SERIAL (or RESOURCE_LOCK on the live/quiet resource) so a whole-suite G1 does not run it beside compilers. Require 5 consecutive quiet passes with the printed per-pair margin recorded. Any pair within 0.1 of a bound is reported to the architect rather than treated as green."
    },
    {
      "id": "T9",
      "target": "B1 / B3 constants in samples (plan 159, 224, 519-522)",
      "claim": "The trim (1,024), the R7b window (-512, +2,880) and B are hard-coded in samples at 48 kHz, and the 64-sample/1.333 ms lattice argument is stated for 'this rig' only. The rate domain is not stated (Pitfall 29: the take's segment rate is the device rate).",
      "evidence": "probe-sync.py:849 uses rate = seg.get('rate') and :930 uses bar = 0.060*rate, so the existing code is rate-aware. The plan converts to fixed samples. The lattice gcd(512, 24000 mod 512 = 448) = 64 is true only if the device block is 512 and the asset rate 48 kHz. At 44.1 kHz the click interval is 22,050 frames and the lattice differs.",
      "severity": "NIT",
      "proposed_change": "State all three constants as milliseconds x the segment rate (21.33 ms, -10.67/+60 ms or the widened values from T2). Record the device block and rate in the R7 INFO lines so the lattice is computed, not assumed."
    }
  ],
  "strongest_point": "G6 (T1) is the one gate that cannot fail as specified. The cpuLoad timer starts inside processHop (AnalysisThread.cpp:211), so the whole delay-line drain and gate (:124-162) is outside what it measures. BeatLead returns as identity at bpm 0 (BeatLead.cpp:148, :156), and the specified mic-in-a-quiet-room driver never engages it. The tripwire is also widened from the ruled 0.3 to 1.0 point. So neither the +500 arm nor the -500 arm can ever trip it. The plan's R7 rework (interleaved pooled trimmed mean plus a mutant-app RED) is a real improvement. But its '9 in 10' teeth claim is wrong by the plan's own bias term (about 55% at the observed -0.31 ms), and R7b(ii) as numbered has one lattice step of margin on a correct app.",
  "citations_rechecked": true
}
```

## Seat: stage-operator (10 attacks; citations_rechecked=true)

```json
{
  "seat": "stage-operator",
  "attacks": [
    {
      "id": "SO-1",
      "target": "B6 (iii) W2 / stage S6 / question 8.1 default YES (plan :297-352, :490, :622-628)",
      "claim": "The plan builds S6 and the un-shifted wheel by default, on Harmony's reading, although the ruling made S6 conditional on Boris answering 'later' and Boris then said the page defaults (Tap = 'where you pressed') are good. The default for 8.1 contradicts a default he blessed, and the build order does not wait for his answer.",
      "evidence": "ruling-bf2.md:279-286 AM-12: 'DEFAULT = plan F7(a): Tap / Resync land where you press, at any D' and 'CONDITIONAL stage S6 ... built ONLY if Boris answers Q6 later'; ruling :378-380 'S6 only on Q6 = later'. boris-checks.html Q16: 'Default: where you pressed.' Boris, verbatim: 'All the other defaults are good.' (backlog BF30 names sync Q11, Q12, Q13, Q15 and does NOT list Q16, but the blanket sentence covers it). Plan :490 'Skipped if Boris answers 8.1 no' (built unless he refuses); plan :622-628 default YES; plan K1 :637-643 concedes it 'changes a ruled, gated behaviour (R9)'. BF16(c) in the backlog says only 'confirm: beat wheel unshifted, output shifted' (a confirm-item, not an answer). The ruling's own SA4 derivation (ruling :43-46) says which tap placement is right 'depends on where Boris listens -- only he knows'.",
      "severity": "MUST",
      "proposed_change": "Invert the default: 8.1 defaults to NO (taps land where pressed, wheel stays the picture beat, R9 as ruled) until Boris answers; S6 and the wheel change are built only on an explicit 'yes'. Make 'S6 starts only after the 8.1 answer is recorded' a written precondition in section 4, not a refutation note in K1. If Harmony keeps YES, say in section 8 that this reverses his blessed default and why."
    },
    {
      "id": "SO-2",
      "target": "B6 (i) Gain 70 -> 140 (plan :394-398, :632-633, G7 :562-563, 7.8)",
      "claim": "Doubling Gain moves every control to its right by 70 px at his window size (Play/Pause/Stop, beat wheel, Tempo, Tap, Resync, Manual, Link, x/ buttons, Quantize). The plan never says so, has no check for it, and its only 'bounds equal main' bar is at 1280 where Gain does NOT change. Tap and Resync are the controls he hits blind mid-show.",
      "evidence": "R:src/ui/TopBar.cpp:538 inputGainSlider_ takes removeFromLeft(70); transport, wheel, tempo, Tap (:572) etc. all follow in the same removeFromLeft chain, so +70 on Gain is +70 on all of them. Plan G7 :562-563 'at 1280 every top-bar widget's bounds equal main's' (no 1728 equivalent); 7.8 (:618-619) only asks 'Nothing is cut off'; 8.4 (:632) offers 'twice as long, same scale' with no mention of the shift. The plan itself counts the same shift against the runner-up T2 (:379 'shifts Tap / Resync right by 86 px more').",
      "severity": "SHOULD",
      "proposed_change": "State the 70 px shift in 8.4 and 7.8 ('Tap, Resync, Manual and everything after Gain move right by 70 px') and have G7 record Tap/Resync x at 1728 before and after. If he does not want it, the alternative is to reclaim the 70 px inside the Gain group by taking it from the 90 px Audio combo or the 6 px gaps; that still shifts, so ask him."
    },
    {
      "id": "SO-3",
      "target": "Gain width rule gainSliderWidth (plan :394-398, K6 :653-656)",
      "claim": "The doubling is all-or-nothing and rests on an unmeasured label width. Any window narrower than about 1695 px (or any font/label change) silently leaves Gain at 70, so the one thing he asked for ('twice as long') is not delivered and nothing tells him why. 1728 as 'his' width is only a test-comment claim.",
      "evidence": "Plan :394-396 'usable >= 1031 + 70 + 64 + rightGroupWanted ... else 70'; 33 px margin at 1728 is INFERRED from E19, which itself says the 'Master Signal:' width is UNMEASURED. R:tests/test_master_signal_link.cpp:14-17 is the only source for 1728 being his width. R:src/ui/TopBar.cpp:605-630 shows the right group already shrinks when short, so a graded rule is possible. Boris verbatim: 'Gain slider in top bar could be twice as long.' (no window-size condition).",
      "severity": "SHOULD",
      "proposed_change": "Make the rule continuous: Gain = clamp(70 + spare, 70, 140) so a window 1 px short still gains almost all of it. Move the K6 measurement (real label width from the dump) to the FIRST S5b step with a hard stop, and ask Boris his window width in 7.8."
    },
    {
      "id": "SO-4",
      "target": "SyncPanel in a juce::CallOutBox (plan :402-408, 423, G7 interaction-logic)",
      "claim": "CallOutBox is modal, which is the exact reason the plan gives for rejecting AlertWindow ('modal and stops the show'). While the panel is open the keyboard clip launcher is dead and the first click outside only dismisses. The plan marks the CallOutBox API ASSUMED and its G7 checks only that the keyboard returns after closing.",
      "evidence": "JUCE source juce_CallOutBox.cpp:71 'callout.enterModalState (true, this)' and :140-157 inputAttemptWhenModal (a click outside dismisses instead of passing through); :188-192 Esc is handled by the box. Plan :403-404 rejects AlertWindow because it 'is modal and stops the show'; :405-408 ASSUMES CallOutBox 'adds an in-window child' and 'closes on a click outside or Esc'; CLAUDE.md: Preview/keyboard clip launcher is the performance surface. The JUCE file read is the copy in another session's scratchpad (juce-src); the lane's own JUCE version was not read.",
      "severity": "SHOULD",
      "proposed_change": "Decide up front (not as K7 fallback) to use a plain non-modal child component placed under the button, closed by a second click on SYNC, by Esc, or by a click elsewhere that is NOT swallowed, so clips can still be fired while it is open. Add a G7/R10 check: with the panel open, a key press fires the mapped clip."
    },
    {
      "id": "SO-5",
      "target": "SyncPanel value box commit rule (plan :417-418) vs Boris's words",
      "claim": "A typed number is lost unless he presses Enter: 'Esc or anything else reverts'. His stated flow is to enter the amount and then click plus or minus; clicking + takes focus from the box, reverts the typed text and then adds 1 to the OLD value. The test route {'type':'+42'} then {'click':'plus'} -> 43 (R10) types and applies in one step, so the gate cannot show this failure.",
      "evidence": "Plan :416-418 'Enter applies (clamped), Esc or anything else reverts'. Boris verbatim (binding-decisions.md:578-580): 'can enter in the amount then click plus or minus to fine tune'. R10 (plan :527-531): {'type':'+42'} -> targetMs 42, then {'click':'plus'} -> 43, which presupposes the type applied.",
      "severity": "SHOULD",
      "proposed_change": "Commit (clamped) on focus loss and on a +/- press, revert only on Esc or unparseable text. Add R10 steps that type '+42' WITHOUT Enter and then click +, expecting 43 (not old+1), and click outside, expecting 42 applied."
    },
    {
      "id": "SO-6",
      "target": "SYNC button 62 x 13, 9 pt, the only door (plan :387-393, G7 :560, K4 :649)",
      "claim": "A 13 px high, 9 pt bold button in a 34 px bar is a poor arm's-length target, and the measurable bar pins that size, so the gate cannot fail on it. 'SYNC 0' is deliberately dim, so at the one setting most rooms start on it is hard to read. The dump bar also contradicts the geometry: by the layout code the button overlaps the tracker-state label by 1 px.",
      "evidence": "Plan :387 'bounds (tempoLabel_.getRight() + 2, area top, 62, 13), 9 pt bold'; :392 'Colour kTextSecondary at 0'; G7 :560 'the button inside the tempo slot, 62 x 13, overlapping neither the BPM number nor the tracker-state label'; G7 :569-570 the target-size question is a critic opinion, waivable (':579 Any FAIL blocks unless Harmony records why it is taste-only'). R:src/ui/TopBar.cpp:531 reduced(4,2) -> area top y=2; :565 tempoLabel_ trimmed 6 -> y=8; :566-568 trackerStateLabel_ y = 8+6 = 14, h 14. Button y 2..15 overlaps 14..28 by 1 px, so the plan's own measurable bar fails as specified.",
      "severity": "SHOULD",
      "proposed_change": "Give the button a minimum hit rectangle (full slot height of 26 px or more, at least 40 x 24) and make that a numeric bar, not a critic question; use 11 pt; keep 'SYNC 0' at readable contrast. Fix the geometry (button height 12 or tracker label moved down 1 px) before it reaches the test."
    },
    {
      "id": "SO-7",
      "target": "Un-shifted wheel at LATE (plan K3 :647-648, 7.2 :602)",
      "claim": "The plan treats the wheel answering a Tap D ms late as a minor limit, but the LATE dial is the main case Boris describes and tapping is his feedback loop: at +300 he gets no sign of a tap for 0.3 s, at +500 for 0.5 s, and two taps inside that window show only after the window. The 'pending tap on the wheel' fix is filed as a follow-up.",
      "evidence": "Plan :360-361 and K3 :647 'answers a Tap D ms late ... up to 0.5 s'; S6 rule :325-327 a Tap applies at nowMs >= P + lateMs. Boris verbatim: 'The user of the application will tap tempo and keep the application in time with the music ... so the user can see it pulsing exactly to the time of the music' and 'The visuals should be on the delay.' The tap-feedback wheel is his instrument for matching taps to the music.",
      "severity": "SHOULD",
      "proposed_change": "Ship a pending-tap acknowledgement in S5a (a flash/tick on the wheel at the press, from the UI-side press time) so the tap is acknowledged at 0 ms. If it is not built, add a Boris check at +300/+500 in section 7 stating exactly this lag and make it a question, not a K-note."
    },
    {
      "id": "SO-8",
      "target": "Wheel gate R12 and the 'known limits' (plan :539-543, :428-429, :360-362)",
      "claim": "R12 gates a function, not what he sees. The dump computes the wheel from a fresh bus read, while the painted wheel is a 15 Hz copy (up to 67 ms, about 0.13 beat at 120 BPM, stale), and it runs in manual mode only. The limits text also misstates holds: in Auto with EARLY, BeatLead freezes the beat fields for up to a beat, and a wheel built from those fields plus a constant offset freezes with them, not 'below the 67 ms tick'.",
      "evidence": "Plan :428-429 '(the 15 Hz painted copy is too coarse to gate)'; R12 :539 'PRODUCTION mode ... manual 120 BPM'; tolerance +-40 ms / +-0.05 beat. R:src/ui/TopBar.cpp:289 startTimerHz(15) and :297-306 timerCallback copies the snapshot into displaySnap_ that paint uses. Plan :360-361 'during an EARLY slew or a hold it is off by the unslewed part (below the wheel's 67 ms tick)'. W:src/analysis/BeatLead.cpp:217-219 'HOLD while the led position is behind what was published ... republish F until the view catches up'; boris-checks.html Q16 note: with EARLY and Auto 'beat-synced motion pauses ... up to one beat if it misses a beat'. musicBeat() only has the led fields (no raw), so during a hold it cannot recover the music position.",
      "severity": "SHOULD",
      "proposed_change": "Gate the painted value: have the dump return displaySnap_ as painted (and its age) and bar the age (e.g. under 70 ms), plus a case at EARLY in Auto with a forced tracker correction showing the wheel's deviation, as an INFO or a bar. Correct the limits sentence. If Auto+EARLY stalls the wheel, say so in 7.3 or carry a held/raw-beat field (Rule 8) so the wheel stays on the music."
    },
    {
      "id": "SO-9",
      "target": "Composition load adopts the file's dial live (plan :451-464, 8.2, K5)",
      "claim": "A load mid-show, from an older composition saved in another room, silently re-tunes the live sync. The only sign is a notice in the single shared load-notice slot (overwritten by any later notice, click-to-hide) and a button that does not show the venue name, so 'Warehouse +42' and 'Home +42' look identical. Nothing in the UI shows when the file and the store disagree.",
      "evidence": "Plan :463-464 'loading a composition saved in another room moves the dial live. It is what he asked for' and :456-458 the notice is the visibility. R:src/MainComponent.cpp:3103-3112 showLoadNotice has ONE text/tooltip, replaced by any other call (same slot as bf9b load refusals); :490-495 it is hidden by a click. Plan :387-389 the button text is only 'SYNC +42' (venue only in the tooltip). Plan :613-615 7.6 expects 'a yellow note'. Boris verbatim: 'if they come back to that venue, they have the settings already' (a return-to-venue use, not a mid-show deck swap). The plan's ':472 deck append' path (appendDeckFromFile) is not stated to be excluded. Also 'present = !(Default and ms == 0)' (:447) means a file saved at 'Warehouse 0' carries the key, so a plain zero is not neutral either.",
      "severity": "SHOULD",
      "proposed_change": "Adopt on a whole-composition open only, never on appendDeck/duplicate. Show the venue name on the button when it is not 'Default' (e.g. 'SYNC Warehouse +42'). Make the sync change its own persistent line (or part of the SYNC button, e.g. a changed-by-load mark) so it cannot be overwritten by another load notice. Add an R11 case where a second load notice arrives together with a sync change."
    },
    {
      "id": "SO-10",
      "target": "Store vs file on a venue name clash: 'the FILE's value replaces the store's' (plan :453-460, R11(d))",
      "claim": "On a name clash the load permanently overwrites the venue store (which is persisted to settings.json 500 ms later), not just the live dial. The exact crash case Boris names (tuned at the venue, the app dies, reload the last save) loses tonight's tuning for good, and there is no unsaved-changes marker to warn him. The plan's own rejection of 'newest wins' admits the hole.",
      "evidence": "Plan :454-455 'it has it with a different ms -> the FILE's value replaces the store's, then select it'; :459-462 newest-wins rejected as 'unpredictable', 'The notice with the old number covers that case'; NOT-in-lane :34-36 lists 'an unsaved changes marker'; the store saves 500 ms after a change (E21, plan :136). R11(d) (:536-538) pins this behaviour. Boris verbatim: 'they will set up the venue and save it in case of a computer crash or something' (the file is the backup of the store, so overwriting the store with the older backup inverts the purpose).",
      "severity": "SHOULD",
      "proposed_change": "On a clash, apply the file's value to the live dial only and leave the store's venue unchanged until he saves (or, if the file value is applied, keep the previous value as 'Warehouse (was +55)' venue). At minimum make the old number reachable after the notice is gone (a venue the picker lists). Change R11(d) to match."
    }
  ],
  "strongest_point": "SO-1: the plan builds S6 and the un-shifted wheel by default (8.1 default YES, S6 skipped only if he says no), which reverses the ruling's condition (S6 only if Boris answers 'later') and the 'where you pressed' default he blessed with 'All the other defaults are good.' This changes a ruled, gated tracker path before a merge on Harmony's reading of his words, and the plan's own K1 concedes it. Making NO the default until he answers costs nothing, because S3f and S4 come first.",
  "citations_rechecked": true
}
```

## Seat: gates-scope (10 attacks; citations_rechecked=true)

```json
{
  "seat": "gates-scope",
  "attacks": [
    {
      "id": "A1",
      "target": "B6 (ii) Persistence, 'Saving' (plan :447) and gate R11 (:532-538)",
      "claim": "The composition carries the venue only on Save As. Plain Save (Cmd+S) and Collect-media write the file without the stamp, so a pro who re-saves loses or keeps a stale 'sync' key. R11 drives only the Save-As path, so it passes while that defect ships.",
      "evidence": "Plan :447 puts the stamp in 'MainComponent::saveCompositionTo, before toVar'. In R:src/MainComponent.cpp, saveComposition() calls composition_.saveToFile(composition_.filePath) directly at :3466-3469; saveCompositionTo is a different function (:3490-3493). Collect-media calls composition_.saveToFile(compFile) at :6595. R11(a) uses POST /api/debug/save_composition, wired to saveCompositionTo (R:src/MainComponent.cpp:2146). Boris, BF16(a): 'saved as part of the composition (venue recall after a crash / return visit)'.",
      "severity": "MUST",
      "proposed_change": "Stamp in ONE place that every save path calls (a Composition-side provider hook invoked inside saveToFile, or a stampSyncForSave() called by all three sites). Add an R11 arm that saves through the plain-Save path (a TEST route that calls saveComposition() on a loaded composition: dial changed, Save, the file's key follows the dial). Add a unit test with a stub that stamps only in saveCompositionTo; it must go RED."
    },
    {
      "id": "A2",
      "target": "S6 'Taps go through the dial' (plan :322-352) and the claim 'Pitfall 48's contract is unchanged' (:333-335)",
      "claim": "Once a Tap or Resync is applied only at P+D, a take armed shortly after a Tap at a LATE dial waits for a snapshot that arrives later than the recorder's 250 ms start-wait fallback. For D in about 250..500 ms the take starts from the stale snapshot with a stderr line, and the start anchor can be wrong. No S6 test, gate or R9 arm covers it.",
      "evidence": "R-side lane code W:src/recording/RecorderHost.h:285 has kStartWaitFallbackSeconds = 0.25. W:src/recording/RecorderHost.cpp:612-628 (startDue) returns true on that timeout 'the take starts from the latest snapshot'. The plan's own text says the seq becomes true 'up to D later' (:334-335). The S6 test list (:346-351) has no take-start case. Tapping and then pressing Record is the workflow Boris describes ('tap tempo and keep the application in time').",
      "severity": "MUST",
      "proposed_change": "Make the fallback max(0.25, lateMs + margin), or have startDue wait on the tracker's pending-request count. Add to the S6 tests a [host][sync] case: Tap at +400, Record at +50 ms, start anchor equals the Tap's beat, no fallback fired. Add it to R9's neighbours or to a live arm."
    },
    {
      "id": "A3",
      "target": "B4 (iii) G6 driver and final gate G6 (plan :263-281, :544-546)",
      "claim": "The tripwire reads two numbers from GET /api/status that do not exist under those names. There is no analysis cpuLoad in /api/status, and the render time is frameTimeMs, not frameMs. Half the tripwire is unreadable, so the gate is INCONCLUSIVE or waived by construction. The plan flags this ASSUMED but still registers it as a blocking gate.",
      "evidence": "W:src/api/ApiServer.cpp:373-380 (handleStatus) sets only fps, frameTimeMs, masterLevel, masterSignal, activeDeck, renderOnsetPulses, and the bpm/phase fields. grep for cpuLoad/getCpuLoad shows the only consumer is MainComponent.cpp:4176 -> TopBar::setDspLoad. Plan :276 'ASSUMED: /api/status exposes both numbers under those names'. Also the driver says 'production app' with AUDIODNA_SYNC_TEST (:269-270), but W:src/sync/SyncOffsetController.h:24 reads that variable in TEST-SERVER builds only.",
      "severity": "MUST",
      "proposed_change": "Verify the field names first. Either add a TEST-only analysis-load field to /api/status (not production) or drop the cpuLoad half. Keep frameTimeMs and the fixed R1a (ii)/(iii) window over R4's -500 arm as the real-time guard. State the build mode explicitly (test-server, preview attached). CUT lost: the cross-arm cpu comparison, which was INFO anyway. KEEP: R1a over the -500 window."
    },
    {
      "id": "A4",
      "target": "G7 visual gate states and critic questions (plan :547-579) versus the test routes (:425-429)",
      "claim": "Several G7 states and interaction-logic items cannot be reached by the allowed drivers, and the gate carries a waiver, so it cannot fail. Unreachable: C6 hover, C10 New-venue/Rename dialogs, C11 duplicate-name refusal, C15 Gain at 1.0 and 0.25. The panel closing returns the keyboard to the clip launcher, hold-repeat of - / +, and 'loading a composition while open' have no driver and are not visible in a PNG either.",
      "evidence": "Plan :425-428 lists only verbs open / type / click plus|minus / rightClick value / menu open. Nothing moves the pointer or answers a dialog. No Gain route exists (grep 'inputGain' in api/ finds none; plan :34 forbids a Gain REST route). Plan :579 'Any FAIL blocks unless Harmony records why it is taste-only'. Plan :574-578 asks LLM critics to judge items no capture shows.",
      "severity": "MUST",
      "proposed_change": "Cut the unreachable captures: C6, C10, C11, C15 become dump assertions (hover colour via a unit test, dialog text via test_sync_panel, Gain range/default/width via the dump) or one explicit driver verb each (e.g. 'dialog':'accept:Name'). Move the interaction items to named unit tests with a RED stub (hold-repeat timer, focus handback as in bpmEditField_, single panel instance). Restrict the critic panel to pixels. Delete the 'taste-only' waiver for measurable bars. LOST: the picture of hover and dialogs; Boris checks 7.7/7.8 still cover feel."
    },
    {
      "id": "A5",
      "target": "S6(b) wheel / gate R12 (plan :353-364, :539-543) and absence of a wheel state in G7",
      "claim": "R12's first half is tautological and the painted wheel is never gated. The dump computes the wheel from a fresh bus read through the same musicBeat() the paint uses, so wheel minus published equals D x bpm/60000 by definition of the function. A mutant where TopBar::timerCallback forgets to copy syncOffsetMs, or paints the unshifted beat, passes R12, passes the unit tests, and passes G7 (no wheel capture in C1-C15). The wheel is what Boris sees every second.",
      "evidence": "Plan :428-429 'wheel ... computed at the request from a fresh bus read ... (the 15 Hz painted copy is too coarse to gate)'; :540 R12 pairs. R:src/ui/TopBar.cpp:300-313 shows the paint path reads a displaySnap_ copy set in timerCallback; the plan adds syncOffsetMs to that copy (:357). G7 states C1-C15 (:550-558) show no wheel.",
      "severity": "MUST",
      "proposed_change": "Make the dump also return the values paintBeatWheel/Bar-n actually used at the last paint (the displaySnap_ fields plus the painted segment). Gate: painted wheel within one 67 ms tick (0.13 beat at 120) of the fresh one, at D = +200/-200/0 where the shift (0.40 beat) is 3x larger than the tolerance. Add a G7 capture of the wheel at +200 settled. RED: stub timerCallback that skips the offset."
    },
    {
      "id": "A6",
      "target": "B7 stage order / SyncPanel + Gain rule (plan :394-399, :473-498); board.md rows 2, 5, 9",
      "claim": "S5b is built on a top bar that bf7 and ui-polish are still going to edit, and its Gain rule hard-codes today's left-run total. The constant 1031 is the sum of fixed widths including the Quantize label and 100 px combo. bf7's delta renames and may widen that combo, and 'Bar N' text may change. A later width change silently flips Gain from 140 to 70, and the re-anchored 1728/1280 tests and 'bounds equal main's' bar would be recorded against a TopBar that moves.",
      "evidence": "Plan :395 'usable >= 1031 + 70 + 64 + rightGroupWanted'; R:src/ui/TopBar.cpp:534-604 (the sum, Quantize 55 + 100 at :602-603). board.md rows 2/5/9: TopBar 'collides with bf7, ui-polish'; facts-quantize-units.md:118 (bf7 delta changes the Quantize combo/labels) and :161 ('Bar N' text A2). The plan's stage list (:473-498) orders S5 only against M0, never against bf7/ui-polish.",
      "severity": "SHOULD",
      "proposed_change": "Compute the Gain width from the same layout code (one function that returns the left-run and right-group widths, used by resized() and by gainSliderWidth) instead of a literal. Put S5b after any TopBar-touching lane that merges first, or state in B7 that the later lane re-runs G7 C1/C2. Make the gate 'Gain is 140 at 1728 in Auto and Manual' the pinned constant, not 1031."
    },
    {
      "id": "A7",
      "target": "SYNC button geometry (plan :387, :393) versus its own G7 bar (:560-561)",
      "claim": "As specified, the button overlaps the tracker-state label by 1 px, so the plan fails its own 'overlapping neither the BPM number nor the tracker-state label' bar. The plan also says that label 'keeps its bounds'.",
      "evidence": "R:src/ui/TopBar.cpp:531 area = getLocalBounds().reduced(4, 2), so top = 2; the button is at 'area top', 13 high, so y 2..15. Tempo label y = 2 + 6 = 8 (:564); tracker label y = tempoLabel_.getY() + 6 = 14, 14 high (:565-569), so y 14..28. Rows 14 overlap. Existing layout tests use a 40 px bar (test_master_signal_link.cpp:300, test_topbar_link_toggle.cpp:54) while the real bar is 34 (R:src/MainComponent.cpp:2578), so a test written at 40 can pass where the 34 px window fails.",
      "severity": "SHOULD",
      "proposed_change": "Fix the numbers before the builder starts: button height 12 at y 2, or the label moved down 1 px (then it no longer 'keeps its bounds'). Make the new button tests set the bar to 34 px high. K4's 13 px target is also thin; keep the Manual-mode fallback."
    },
    {
      "id": "A8",
      "target": "B4 (ii) 'did not move back' reading, R5 changed clause (plan :256-262, :513-515)",
      "claim": "The accepted reading narrows R5's antecedent to hops where the raw fold advanced at least aN, and the probe passes whenever any hop ran BeatLead, even if zero hops met the antecedent. A run with no qualifying hops passes vacuously, and a mutant that lets the target move back on hops outside the antecedent is never tested.",
      "evidence": "W:.harmony/probe-sync.py:805-810 (antecedent 'raw_adv >= aN - 1e-4') and :822-823: the verdict is 'applied > 0 and not slow'. No count of hops that met the antecedent is required or printed. Plan :256-262 accepts the narrowing as the registered text.",
      "severity": "SHOULD",
      "proposed_change": "Count hops meeting the antecedent; require that count >= 90% of the free-running hops (print it as a number). A pass with fewer than N qualifying hops is FAIL. Cheap: two lines in the probe, no run needed to register."
    },
    {
      "id": "A9",
      "target": "B1 R7 statistic (plan :159-164) and R7b(i) (:217-221)",
      "claim": "(1) SE = SD(d_i)/sqrt(10) treats the ten d_i as independent, but adjacent d_i share a neighbouring 0 arm (each 0 arm feeds two d_i). That understates the SE and so the 'within 2 x SE' extension trigger. (2) R7b(i) is declared to pass on the S2 behaviour 'by construction' and is registered as a live verdict; it adds no evidence at the merge.",
      "evidence": "Plan :160-161 d_i definition: 100-arm trimmed mean minus the mean of its two neighbouring 0 arms, arms [0,100,0,100,...,0]. Plan :217-219 '(i) INVARIANT ... passes on the S2 app by construction'. The trim cut 1,024 samples (:159) is chosen after seeing the saved data (E6).",
      "severity": "SHOULD",
      "proposed_change": "Compute dbar as mean(100 arms) minus mean(0 arms) with a Welch/SE from the two groups (or a bootstrap over arms); state the 1,024 trim as chosen before the new run. Demote R7b(i) from the live verdict to INFO (its teeth are the unit mutant M3) and keep (ii) as the live row. LOST: a duplicate live guard against a second clock, which the unit case already pins."
    },
    {
      "id": "A10",
      "target": "B6 (ii) hook scope and 'present' rule (plan :447-458)",
      "claim": "Two gaps. (a) The hook sits in finishStagedLoad, which is shared by Composition, DeckAppend and DeckDuplicate; the plan does not say it is gated to Kind::Composition, and R11 has no 'Load Deck of a file carrying sync' arm. (b) present = !(Default and 0) means a user who deliberately returned to Default 0 saves no key, so loading that file does not restore 0, which conflicts with 'come back to that venue and have the settings'. Adopt also rewrites the persisted venue store on a mere load.",
      "evidence": "R:src/MainComponent.cpp:3229 (finishStagedLoad) branches on s->kind at :3251, :3258, :3288. Plan :447-448 (present rule), :451-455 (adopt replaces the store's value), R11 :532-538 (arms a-f, none for deck load). Old-file test is only 'an old file leaves a +42 dial at +42' (:468-469), built from an absent key rather than a real file saved by a7491d4.",
      "severity": "SHOULD",
      "proposed_change": "Run adoption only under Kind::Composition; add R11 arm (g): Load Deck of a file with a sync key leaves the dial unchanged. Write the key whenever the user has ever touched the dial this session (or always write it); at minimum document the Default/0 limit. Commit a real pre-lane composition file as the old-file fixture for test_composition and the staged-load test."
    }
  ],
  "strongest_point": "Strongest: A1, the persistence stamp. Boris asked for the venue to ride with the composition 'in case of a computer crash', and the plan stamps it only in saveCompositionTo. Plain Save (R:src/MainComponent.cpp:3466-3469) and Collect-media (:6595) write the file through composition_.saveToFile with no stamp. Gate R11 drives only /api/debug/save_composition, which is the Save-As path, so the gate goes green while the common pro workflow (open the file, tune, Cmd+S) loses the venue. Cuts I would make and what each loses. (1) G6's cross-arm cpu comparison: loses a perf regression signal at +/-500, which R1a over the -500 window still gives. (2) G7's C6 hover, C10 dialogs, C11 refusal and C15 Gain captures: loses pictures of states, which unit tests and Boris checks 7.7/7.8 still cover. (3) R7b(i) as a live verdict: loses a duplicate guard against a second clock, which unit mutant M3 pins. The ONE thing that must not be cut is R11(b) plus the staged-load 'file without the key never touches the dial' test, ideally on a real old composition file. Loading any of Boris's existing shows must never move a venue's dial, and that failure is silent and costly in a venue. Second keeper: R7's mutant-app RED, the only live proof that S3's stamps do anything.",
  "citations_rechecked": true
}
```

