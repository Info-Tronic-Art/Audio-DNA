# PLAN bf2 DELTA -- the sync dial: the open items of S2 / S3, a new stage S6, stage S5 re-planned (s-rta-1003)

Author: architect (position; a blind council attacks it; an architect ruling follows; Harmony decides). Read-only work:
nothing was built, run, launched or probed. Labels: VERIFIED (read at the cited file:line or computed from a saved file
by the command named), INFERRED (reasoned from verified lines), ASSUMED (not checked; named where it is used).
Paths: W: = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2 (lane/bf2, head 4a1f240).
R: = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/recon (a7491d4, the app after the deck change).
P: = /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1002b (plan-bf2.md, ruling-bf2.md).
F: = /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-1003/facts-topbar-sync.md (the parallel fact
sheet; I re-read the lines it cites wherever a decision rests on them and say so).
S3LOG = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf2-S3
(the S3 builder's saved live runs).

## 1 GOAL

Get lane bf2 to a merge in which (a) every gate row is a test that can fail and whose bar its instrument can resolve,
(b) the dial means one thing in every mode -- the beat the user taps or the tracker hears is the MUSIC; the picture is
that beat moved by the dial -- and (c) the on-screen control lives in the top bar, is remembered with the composition,
and ships together with a Gain slider twice as long, behind one visual gate.

The six headline choices (each has its runner-up in section 3):
  1. R7 keeps its 1 ms bar but on a statistic the instrument can resolve: the trimmed MEAN pooled over 10 interleaved
     100 ms arms (not one median). R7b gains a second check that fails on the S2 behaviour.
  2. The replay-end decision, the origin cap and the "did not move back" reading are ACCEPTED as built, each with a
     written disclosure; one follow-up is filed (replay end).
  3. G6 gets a driver and becomes INFO plus a fixed tripwire; the real-time property at -500 is gated by R1a.
  4. NEW stage S6 "Taps go through the dial" is built (it was conditional), because without it the dial's effect on a
     tapped beat depends on the order of two button presses; the top-bar beat wheel is un-shifted from the snapshot.
  5. S5 = a "SYNC +42" button in the top bar's tempo slot that opens a drop-down panel (number box, - / +, the long
     bar, the venue picker); zero added bar width; Gain 70 -> 140 px where the window is wide enough.
  6. The composition file carries {"venue","ms"}; loading a file that has it sets the live dial (glide + notice);
     a file without it never touches the dial.

NOT in this lane: a top bar that works at 1280 (it does not today, fact E7); Gain persistence or a Gain REST route;
re-scaling the Gain slider; an "unsaved changes" marker; holding a replay's end by the dial amount (filed follow-up);
an instant wheel answer to a Tap at a LATE dial (section 9, K3); round knobs; any change to bf9b / bf7 files beyond the
hunks named in section 4.

## 2 ESTABLISHED FACTS (verified only)

Lane state
 E1  Head 4a1f240; S1a, S1b, S2 built, S3 PARTIAL; S4 and S5 not started (W:.harmony/.reports/s-rta-1002b/bf2.md:3-22,
     :65-70; `git -C W log --oneline -3`). The lane and a7491d4 and main 5abdf01 share merge-base af96de2
     (`git -C W merge-base HEAD a7491d4`, `... HEAD 5abdf01`).
 E2  Files changed on BOTH the lane and (main 5abdf01 or a7491d4) since af96de2 (`git diff --name-only`, `comm -12`):
     .harmony/APP-INVENTORY.md, .harmony/probe-tsan-unit.sh, CLAUDE.md, CMakeLists.txt, docs/claude/architecture.md,
     integration.md, pitfalls.md, recording.md, testing-eyes.md, src/api/ApiServer.cpp, ApiServer.h,
     src/MainComponent.cpp, MainComponent.h, src/render/Renderer.cpp, Renderer.h, tests/CMakeLists.txt,
     tests/test_recorder_host.cpp. NOT on both: src/analysis/AnalysisThread.* (lane only). Changed on the other side
     only, and touched by this plan's later stages: src/ui/TopBar.cpp, TopBar.h, src/model/Composition.h,
     src/binding/BindingTarget.h. A real merge dry run was not possible here (read-only fence).
 E3  CLAUDE.md sizes (`wc -c`): lane 23,947; a7491d4 24,264; main 5abdf01 23,962.

R7 / R7b (B1, B3)
 E4  The row as built: arms [0, 100, 0], 60 s click takes; per marker error = marker frame - nearest 0.5 s click;
     verdict on median(100) - mean(neighbouring 0 medians), bar 1.0 ms (W:.harmony/probe-sync.py:828-853, :862, :939-946).
 E5  Saved runs (S3LOG/live-*.log): S2 app: arm medians 30.67 / 130.67 / 30.67, means 31.83 / 130.92 / 32.01 (median
     difference +100.00, mean difference +99.0 ms). S3 app run 1: medians 30.67 / 32.00 / 30.67, means 31.39 / 31.98 /
     31.22. S3 app run 2: medians 30.67 / 29.33 / 30.67, means 31.13 / 30.66 / 30.69; lattice 64 samples (1.333 ms).
 E6  Computed from S3LOG/out-green2/run-*/probe-sync.json (python3 statistics on the saved per-marker errors; nothing
     re-run): per-arm SD 6.80 / 8.71 / 6.51 ms over 121 markers; each arm holds ONE marker about 60 ms late (4416 or
     4736 samples against a body of 1216-1664 at D = 0 and 1024-1984 at D = 100); the D = 100 arm sits on 16 lattice
     values, the D = 0 arms on 8 (+ strays). Bootstrap SD of the run's verdict statistic: mean 0.90 ms, median 0.99 ms,
     mean after dropping markers more than 12 ms from the arm median 0.72 ms. P(|statistic| > 1 ms) for ONE run of a
     correct app: mean 0.30, median 0.45, trimmed mean 0.22.
     CONSEQUENCE (INFERRED from E6): none of the builder's options (a) / (b) / (c) is a gate on one run -- a single
     [0, 100, 0] run cannot resolve 1 ms with ANY statistic.
 E7  R7b as built: the probe polls GET /api/perf/status "markers", fires POST /api/trigger_clip when it reaches 10 / 20
     / 30, and checks gesture sample - marker k sample in [0, 60 ms] (W:.harmony/probe-sync.py:882-893, :925-937). One
     variable, sampleForClock, stamps markers and gestures (W:src/recording/RecorderHost.cpp:462-480). The S2 app
     passes it with 0 / 0 / 0 samples (S3LOG/live-red-s2.log:50-52).

Replay end (B2)
 E8  With-audio replay: pos = endPos - llround(late x asset rate); the end edge compares endPos (the transport) with
     playEndPos_; on that edge the Player is advanced to playEndPos_ and stopped (W:src/recording/RecorderHost.cpp:564-596).
     Mutants M1 (no tail advance) and M2 (end edge on the shifted position) fail the unit cases
     (W:.harmony/.reports/s-rta-1002b/bf2.md:148-150).

BeatLead (B4)
 E9  At a non-Resync re-base the origin shift is capped so barsSinceResync() does not decrease; at a manual Resync
     originX_ = barX_ (W:src/analysis/BeatLead.cpp:184-212). Appendix A rule 2 says "O'' += the same" (P:plan-bf2.md:689-691);
     the ruling's AM-2 (v) says barsSinceResync() never decreases except at a Resync epoch (P:ruling-bf2.md:179); the
     adoption orders: adoption > ruling > plan body (P:plan-bf2.md:714).
 E10 barsSinceResync() is read by exactly three consumers, all as the bar term of a multi-bar fold:
     W:src/signal/OscillatorSignal.h:58, W:src/signal/EnvelopeSignal.h:55-60, W:src/connect/ConnectionEngine.cpp:98-102
     (`grep -rn barsSinceResync W:src`).
 E11 "Did not move back" as built: the antecedent is "the raw fold advanced >= aN - 1e-4" and the bar is "the target
     fold advanced >= 0.75 x aN - 1e-4 - max(0, aN - raw advance)" (W:.harmony/probe-sync.py:799-806, :816-817; unit:
     W:tests/test_beat_lead.cpp:295).
 E12 No perf A/B script exists (`ls W:.harmony | grep probe-sync` = probe-sync.py / .sh, probe-sync-venues.py / .sh).
     Env AUDIODNA_SYNC_TEST=<integer N> starts a session at N with persistence off (F: Q2, citing
     W:src/sync/SyncOffsetController.h:28-35; not re-read by me).

[timing] (B5)
 E13 The case's bars: per arm sd <= 2.0 ms, observed >= 99 % of published; each 37 / 38 pair: mean(38) - mean(37) in
     [0.7, 1.3] ms; each 150 arm: shift in [148, 152] ms (W:tests/test_analysis_sync_thread.cpp:664-693). It is
     registered through catch_discover_tests with TEST_SPEC "~[tsan]" (W:tests/CMakeLists.txt:3488). Its two S3
     failures: sd 2.07 / 3.16 / 2.34 with two python3 at ~100 %; then pairs 0.649 / 0.678 with a 28 % process; it
     passed alone at 21:06 on the same analysis sources (W:.harmony/.reports/s-rta-1002b/bf2.md:30-34, :275-276, :25-26).

What the dial moves today, and the wheel (B6 iii)
 E14 LATE: audio hops wait in a delay line and are processed when now >= arrival + D
     (W:src/analysis/AnalysisThread.cpp:124-156), so every published field is D late. EARLY: BeatLead rewrites nine
     beat fields only (F: Q2 / 2c; W:src/analysis/BeatLead.cpp:176-212 read by me).
 E15 A Tap (setManualBPM), a Resync and a typed tempo are REQUESTS the tracker applies inside its next process() call
     (W:src/analysis/BPMTracker.cpp:76-98, :565, :586, :613; W:src/MainComponent.cpp:5663-5695). Gate R9 pins it: at
     +100 / +253 a Resync publishes within 50 ms of the POST (P:ruling-bf2.md:462-466).
 E16 A dial MOVE in manual mode does move the published beat: gate R3 requires the published beat to run at 0.70-0.80
     of tempo while the dial slews 0 -> 500 (P:ruling-bf2.md:440-444), and S1a passed R3.
     CONSEQUENCE (INFERRED from E15 + E16): at a LATE dial the tapped beat's picture is D late if the dial was moved
     AFTER the last Tap / Resync, and NOT late if the Tap / Resync came after the dial was set (a venue recalled at
     start-up, then tapping all night = the dial does nothing to the tapped beat). No run was made; R3 and R9 are the
     two measured halves.
 E17 The top-bar wheel reads the same FeatureBus snapshot as everything else: TopBar copies bpm, trackerState,
     beatInBar, barPhase, beatPhase, barCount at its timer tick and paints the wheel from beatInBar / beatPhase and
     "Bar n" from barCount (R:src/ui/TopBar.cpp:300-313, :458-500, :525). So today the wheel shows the PICTURE's beat.
     The snapshot carries the applied offset as syncOffsetMs (W:src/recording/RecorderHost.cpp:456 reads it).

Top bar and composition file (B6 i, ii)
 E18 TopBar::resized() is all fixed pixels: Gain label 30 + slider 70 (R:src/ui/TopBar.cpp:537-538); tempo number 50 +
     a reserved 64 px slot holding the tracker-state label 60 x 14 (:565-569), which is hidden in Manual mode (:104);
     the BPM edit field takes 60 + 4 after Link only in Manual mode (:582-586); left run = 1031 px (1095 Manual) by my
     own sum of :534-604; the right group is built with removeFromRight and shrinks when space runs out (:606-630).
     Window minimum 1280 x 720 (R:src/Main.cpp:53); the bar is 34 px high (R:src/MainComponent.cpp:2578). No "Fade:"
     control at a7491d4 (grep). The Gain slider: ResettableSlider, 0..4, step 0.01, default 1.0, no text box (:25-30).
 E19 F: 1d (arithmetic re-done by me for the left run; the right group's wanted width ~514 px rests on an UNMEASURED
     "Master Signal:" label width, 72-78 px): at 1728 wide about 167 px are spare (103 in Manual mode); at 1280 the
     right group is already about 281 px short and the Master faders get ~0 px. 1728 is named as Boris's maximized
     width only by a test comment (R:tests/test_master_signal_link.cpp:14-17, via F:).
 E20 No juce::CallOutBox anywhere in R:src (`grep -rln CallOutBox R:src` = 0 files). A "Tempo" caption is painted above
     the BPM number (R:src/ui/TopBar.cpp:442-446). A load notice label exists (R:src/MainComponent.cpp:490-495).
 E21 Composition files are plain JSON written by Composition::toVar / read by fromVar with hasProperty guards; no
     version key; no "sync" key; saveToFile / loadFromFile at R:src/model/Composition.h:1119-1135 (F: Q3; I did not
     re-read Composition.h). The venue store: SyncVenues {name, ms}, current pointer, in settings.json key "sync";
     SyncOffsetController has setMs / nudge / selectVenue / createVenue / renameVenue / removeVenue and saves 500 ms
     after a change (W:src/sync/SyncOffsetController.h:42, :63-68; F: Q2).

## 3 ITEMS

### B1 R7's bar cannot be met at its own resolution

Facts: E4, E5, E6. Why the metric was wrong for the instrument (exactly): a marker's stamp is the delivered counter
sampled at a 120 Hz tick, so every per-marker error is a multiple of 64 samples (1.333 ms) and spreads over 8 (D = 0)
or 16 (D = 100) lattice values; one median of 121 such values moves in 1.333 ms steps, above the 1 ms bar; and one
run's MEAN has a standard error of 0.7-0.9 ms (one 60 ms-late detection alone moves an arm mean by 0.5 ms), so the
mean cannot hold a 1 ms bar on one run either.

Alternatives:
 (a) mean within 1 ms, one run -- fails 30 % of correct runs (E6). Not a gate.
 (b) median within one lattice step (1.34 ms), one run -- the bar equals the instrument's own step; a one-step
     regression passes half the time; a correct app still fails ~8 % (E6: P(|.| > 2) = 0.08). Not a gate.
 (c) keep -- a coin flip (0.45).
 (d) CHOSEN: keep the number (1.0 ms: one dial step -- a take must not be mis-aligned by more than the dial can
     express), change the statistic and the sample size so the instrument resolves it.
 (e) RUNNER-UP: bar 2.0 ms on the pooled trimmed mean of 5 arms (12 min instead of 23).

The chosen row (replaces R7's bar; the take math E4 is unchanged):
  Arms [0, 100, 0, 100, ... , 0] = 10 arms at 100 interleaved with 11 at 0, one app launch, 60 s takes (about 23 min).
  Per arm: drop a marker whose error is more than 1,024 samples (21.33 ms at 48 kHz) from the arm's median, count the
  drops, take the MEAN of the rest ("trimmed mean"). Per 100-arm i: d_i = its trimmed mean - the mean of the trimmed
  means of its two neighbouring 0 arms. Statistic: dbar = mean of the ten d_i; SE = SD(d_i) / sqrt(10).
  BAR: |dbar| <= 1.0 ms; every arm pairs >= 100 markers; every arm drops <= 4 markers.
  ONE pre-registered extension: if |dbar| lies within 2 x SE of 1.0 (either side), run the same 21 arms once more and
  judge the pooled 20 d_i; that verdict stands. No other re-run except the ruling's `ps` rule.
  INFO lines (no verdict): each d_i, SE, the per-arm medians, the lattice.
Why it still has teeth: the S2 behaviour reads dbar = +99 to +100 ms (E5) -- 99 times the bar. A one-lattice-step
(1.333 ms) regression: with the expected SE of about 0.23 ms (0.72 / sqrt(10), E6) it fails about 9 times in 10; a
two-step one always. Does one step matter? No code path makes a 64-sample error: 64 is gcd(512-sample device block,
24,000-sample click interval mod 512) on THIS rig, not a unit of the implementation. The implementation's error units
are 1 sample (rounding; pinned exactly by the unit cases, "1000000 == 995200"), one tick (8.33 ms), one device block or
hop (10.67 ms), a rate-domain slip (8.8 ms at D = 100 on a 44.1 kHz device; unit case +253 @ 44.1 kHz) and the whole
offset (100 ms) -- all far outside 1 ms. (INFERRED list.)
Known systematic term (INFERRED, builder's note bf2.md:110-111): the real wait is D plus the loop's wake, so dbar is
expected slightly negative (-0.2 to -0.5 ms). If dbar fails the bar with |dbar| < 2.67 ms that is a FINDING to route
to the architect, not a flake to re-run.
Why the runner-up loses: 2.0 ms is safer against a false FAIL (margin 5 SE) and cheaper, but it changes the
pre-registered number after two failing runs, which Harmony's constraint forbids unless the number itself was wrong;
only the statistic and the sample size were wrong. If the council finds the 1 ms figure has no requirement behind it
(the dial-step argument is mine), (e) is the fallback, stated here in advance.
Change by file: W:.harmony/probe-sync.py row_r7 (arms default, trim, d_i, dbar, SE, the extension flag
PROBESYNC_R7_EXTEND=1 that appends a second pass and pools), probe-sync.sh header text. No src change.
RED first: (1) recompute from the saved S2-app numbers (E5): dbar = +99.0 -> FAIL (arithmetic, no run). (2) LIVE RED
for the new row AND for B3: a MUTANT app -- the lane tree copied to a scratch build dir with one line changed
(RecorderHost.cpp:456 lateSeconds = 0.0, i.e. the S2 behaviour), run with PROBESYNC_R7_ARMS=0,100,0: R7 FAIL near
+100, R7b(ii) FAIL. `grep -c MUTANT` on the real tree = 0 afterwards; the copy is deleted.
Gate: section 5, R7.

### B2 The replay-end decision

Facts: E8. With a LATER dial D, a replay WITH audio shows each recorded move D after its audio, like the live picture.
The audio file ends; the last D of show time has no transport left to run on.
What Boris would see:
 (A) as built (ACCEPT): the picture follows the take D late all the way; at the instant the music file ends, any moves
     recorded in the take's last D ms (they exist only when the take was recorded at a SMALLER dial value than the
     replay's) happen all at once, up to D early, and the take then holds its last state. With a take recorded and
     replayed at the same dial value nothing is early. Worst case 0.5 s, only at the very end, only for a take from
     another room.
 (B) hold the end D ms after the transport stops: those last moves play out in time, and the take (and a looping
     routine's restart) ends D ms after the audio does.
 (C) end edge on the shifted position (mutant M2): the replay never finishes. Not an option.
Choice: (A), with follow-up FU-1 filed ("replay end holds for the dial amount"). Why (B) loses NOW: it needs a clock
that keeps running after the transport stopped (wall-time extrapolation from the tick that saw the stop), and it moves
the end edge that routines use to restart on the bar (CLAUDE.md "restore-then-replay on the next bar") -- a second
timing change in the routine path for a 0-0.5 s tail that only a cross-room take can show. Nothing is lost under (A):
the points fire, the final state is right.
Change: none in src. Docs: recording.md "Takes under the sync dial" gains two sentences (what (A) does; FU-1).
Tests: exist (the tail case, M1, M2). Gate: G1 test_recorder_host [host][sync] (unchanged).
Cheapest refutation: Boris check 7.4 (a take recorded at 0, replayed at +300, watch its last half second).

### B3 R7b cannot fail on the S2 app

Facts: E7. R7b compares two stamps taken from one variable, so it is true in any tree that keeps one clock: it pins an
INVARIANT ("gestures and markers share one domain"), it is not evidence that S3 did anything. The builder's choice to
watch the recorder's own marker count (not the bus count) is ACCEPTED: it guarantees the gesture is made after marker k
was stamped, which the bus count (up to one 8.3 ms tick ahead) does not.
Alternatives: (a) keep and label it "invariant"; (b) CHOSEN: keep it AND add a check with a RED; (c) drop it.
Re-registered row R7b, two checks, both inside R7's first 100 arm:
 (i)  INVARIANT (as built): gesture stamp - stamp of marker k in [0, 2,880] samples, k = 10, 20, 30. Declared: passes
      on the S2 app by construction; its teeth are the unit case "markers + gesture + tempo anchor in one domain" under
      a mutant that shifts markers only (S3-finish builder adds mutant M3 to the report: expected 100 ms = 4,800
      samples > 2,880 -> FAIL).
 (ii) NEW, absolute: g_k = (gesture stamp - the take's first sample) - the click-grid position nearest to marker k
      (same grid math as t2_errors). BAR: g_k - B in [-512, +2,880] samples, where B = the median marker error of the
      FIRST 0 arm (about 30.67 ms = 1,472 samples in E5). On the S2 behaviour g_k - B is about +4,800 samples
      (+100 ms) -> FAIL by 40 ms; on the S3 app the saved runs give 0 or 512 samples plus the marker's own lattice
      offset (INFERRED from E5 / E7; the saved JSON lacks the take's first sample, so this is not recomputed).
Why (a) loses: a row that cannot fail on the base tells Harmony nothing at the merge. Why (c) loses: the invariant is
cheap and guards a real future mistake (a second clock for gestures).
Change: W:.harmony/probe-sync.py row_r7 (g_k, B). RED: the mutant app of B1. Gate: section 5, R7b.

### B4 The S2 items

(i) The origin cap. Facts: E9, E10. It triggers only at EARLY, at a re-base that is not a Resync and that un-crosses a
bar line the led view had already crossed (a Tap landing within the lead after a bar line; tempo going to 0).
 What the user gets:
  cap (as built):   nothing ever runs backwards; multi-bar shapes (2- / 4- / 8-bar oscillators and envelopes) sit one
                    bar ahead of the tracker's own bar count until the next Resync, which re-anchors them.
  Appendix A:       the bar count those shapes fold on drops by one at that re-base: a multi-bar shape retraces up to
                    the lead (<= 1.67 beats) and anything that fires at the start of its cycle fires twice; afterwards
                    it agrees with the tracker.
  hold (third way): the beat fields freeze until the view catches up -- up to the lead after a TAP, i.e. the tapped
                    beat does not land where it was pressed. Rejected: it breaks the Tap.
 Choice: ACCEPT the cap. Reasons: (1) precedence -- the ruling's AM-2 (v) overrides the plan's Appendix A (E9), and the
 builder followed it; (2) "which bar is bar 1" is set by Resync in this app at any dial value (EnvelopeSignal.h:52-54
 says so), and the cap's cost ends exactly at the next Resync (BeatLead.cpp:199-200); (3) the alternative re-fires
 cycle-start events, the same class of phantom event AM-2 exists to prevent. Runner-up: Appendix A + (v) relaxed at
 re-bases; it loses on (3), and it wins only if Boris reports 4-bar shapes "on the wrong bar" after tapping at an
 EARLY dial without pressing Resync (check 7.5).
 Change: no code. Appendix A is amended in words (docs/claude/analysis.md "Sync offset", one paragraph: the origin
 excess, capped; healed by Resync). Test: S3-finish builder confirms a unit assertion pins "after the next Resync
 barsSinceResync() == the view's bars since it"; test_beat_lead.cpp:676 pins the origin excess at a re-base; if the
 Resync-heals assertion is missing it is added to T-L4 (RED on a mutant that drops `originX_ = barX_`).
 Honest cost not in the builder's note (INFERRED): several such Taps in a row at a large EARLY value each add a bar,
 so after a tapping run the multi-bar phase is arbitrary until Resync.

(ii) "Did not move back". Facts: E11. The ruling's words are undefined for float phases and for a realign that nets a
 small forward step. The builder's reading -- "the raw fold advanced at least its free-run step aN (less 1e-6 in unit,
 1e-4 live)" -- is ACCEPTED and becomes the registered text (section 5, R5 and T-L3 (iv)). It narrows the antecedent to
 free-running hops, which is the case the clause exists for (no hold outlasts its catch-up). Runner-up: test every
 non-re-base hop with "target advance >= raw advance - 0.25 x aN"; stronger, but I have not proven it holds across the
 auto regime's count lag, and a false RED there would cost a round. Not required.

(iii) G6 perf A/B at 0 vs -500. Facts: E12. Harmony constraint: perf verdicts only on a quiet machine, arms interleaved
 launch by launch, >= 5 runs per arm, and a bar whose teeth equal the drift is INFO, not a gate.
 The ruled bar, max(0.3 points, 2 x within-arm SD), grows with the drift, so by that constraint it is INFO.
 Choice: specify the driver; the A/B delta is INFO; add a FIXED tripwire far above drift; gate the real-time property
 at -500 where an instrument with fixed teeth already exists (R1a).
 Driver (NEW W:.harmony/probe-sync-perf.sh, written by the S3-finish builder, RUN by Harmony only): per run -- `ps`
 check (no process above 20 % CPU; else wait), launch the production app with `open -g --env AUDIODNA_SYNC_TEST=<D>`
 (session starts at D, persistence off, E12), mic input, POST /api/render_frame once (preview attached), 10 s warm-up,
 then GET /api/status once a second for 20 s -> run mean of analysis cpuLoad and of render frameMs; graceful quit.
 Launch order 0, -500, +500 repeated 5 times (15 launches, about 12 min). settings.json sha256 before and after.
 Output: per arm mean and SD over its 5 runs, deltas vs the 0 arm.
 Verdict: INFO table, plus TRIPWIRE (blocks): an arm's mean cpuLoad more than 1.0 percentage point above the 0 arm's,
 or its mean frameMs more than 1.0 ms above -- valid only when 2 x the within-arm SD is below that figure; otherwise
 INCONCLUSIVE = wait for a quiet machine and repeat. ASSUMED: /api/status exposes both numbers under those names (the
 ruling's G6 says so; not re-read).
 NEW fixed gate: R1a (ii) and (iii) are also evaluated over R4's settled -500 window (the witness is already running
 there): iteration start-to-start <= 10.7 ms on >= 99.5 % of iterations. Change: probe-sync.py passes R4's window to
 row_r1a (it already takes a window list, :983-993).
 Runner-up: keep G6 as a gate with the ruled bar -- loses to Harmony's constraint above.

### B5 G1 [timing]

No ruling: both failures have a recorded cause, the analysis sources did not change in S3, and the case passed alone at
21:06 (E13). Harmony's re-run, on a quiet machine (no compiler, no app, no commit in the last 2 minutes -- a commit in
a worktree starts the graphify rebuild at ~100 % CPU for about a minute, bf2.md:88-90):
   ps -Ao pcpu,comm -r | head -5        (nothing above 20 %)
   ctest --test-dir /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/build-lane -R "\[timing\]" --output-on-failure
 Pass line: "100% tests passed, 0 tests failed out of 1". (build-lane is the lane's build dir per bf2.md:30, :274;
 ASSUMED still present and built at 4a1f240 -- if not, build the one target first.) After the merge-in (M0) the same
 command runs again in the lane's rebuilt tree as part of G1.
 Not a defect, but record it (INFERRED): the second failure missed the 37 / 38 bar by 0.02-0.05 ms under a 28 % process;
 the bar's margin is thin. If a QUIET run fails, the ruling's rule stands (it blocks) and the finding goes to the
 architect with the printed per-arm sd and "longest poll gap" -- the bar is not re-thresholded by a builder.

### B6 Stage S5 re-planned (and what Boris's answers do to the engine)

(iii) FIRST, because it decides the rest: which beat is "the music"

Boris: "The user of the application will tap tempo and keep the application in time with the music." "The visuals
should be on the delay. The music is in time and the visuals should be delayed or a little ahead depending on how the
system is wired." "All the other defaults are good."
What the lane does today: E14-E17. Three things follow.
 1. The wheel shows the picture's beat (E17), not the beat he tapped.
 2. At an EARLY dial a Tap is led like any beat: picture = tap - |D|. This already matches his words.
 3. At a LATE dial a Tap / Resync lands at the press (E15): picture = tap, the dial adds nothing -- unless the dial was
    moved after the tap (E16). The dial's effect depends on the order of two actions. With a venue recalled at
    start-up (page question 12's default) the order is always "dial first, then tap", so in the workflow he describes
    the LATE dial would not move the tapped beat at all. His message's last sentence (the page-16 default "where you
    pressed") and his own description disagree if "the beat" means the picture; they agree if it means the wheel.
Alternatives:
 W1 leave it (wheel = picture; taps at the press). Cheapest; contradicts his description; keeps the order dependence.
 W2 CHOSEN: one rule in every mode -- MUSIC beat = what the tracker hears or the user taps; PICTURE beat = music beat
    moved by the dial; the wheel shows the music beat. Needs (a) the conditional stage S6 of the ruling ("Taps go
    through the dial") and (b) an un-shifted wheel.
 W3 un-shift the wheel only. Not coherent: after "dial, then tap" the snapshot is not shifted (E15), so a wheel that
    adds the offset back would run D ahead of his tap.
Choice W2, put to Boris as question 8.1 with default YES (Harmony's reading). If he answers no: S6 and the wheel change
are both dropped (W1), R9 stays as ruled, nothing else in this plan changes.

 (a) S6 "Taps go through the dial" (engine). The ruling's outline (P:ruling-bf2.md:285-301) with these decisions:
   - WHEN a request is due: a human realigning request (Tap = setManualBPM, Resync) carries its press time P (the delay
     line's clock, juce::Time::getMillisecondCounterHiRes) and is applied in the first tracker process() call made at
     nowMs >= P + lateMs, lateMs = max(applied offset, 0) at that call. At a dial of 0 or EARLY this is "the next
     process() call", bit-identical to today (this is why I replace the outline's "first hop whose arrival stamp >= P",
     which would add up to one hop at D = 0 and could move the R8 regression probes).
   - Value-only requests (typed BPM, set_bpm, Link, manual / auto) and every Origin::Replay request stay immediate.
   - Mechanism: a fixed single-producer queue of 16 {kind, bpm, pressMs, seq} inside BPMTracker (no allocation, no
     mutex; producers are message-thread callers only -- REST / OSC / MIDI handlers already marshal to it, ASSUMED;
     the builder verifies each call site of applyTempoCommand and reports any off-message-thread caller as a STOP).
     Signature: `void BPMTracker::setProcessClock(double nowMs, double lateMs) noexcept` (analysis thread, called by
     AnalysisThread::processHop before process()). appliedRequestSeq() advances only past APPLIED requests, so
     Pitfall 48's contract is unchanged: "everything sent before X" = snapshot trackerRequestSeq >= postedRequestSeq()
     read at X -- it just becomes true up to D later. Queue full: the oldest applies at once, counted (expected 0).
   - MainComponent::applyTempoCommand's resync branch stops zeroing beatCounter_ / beatCrossings_ at the press; it
     stores the posted seq and the zeroing runs at the first feature-pipeline tick whose snapshot shows
     trackerRequestSeq >= it (ruling LA2). At D <= 0 that is the next tick.
   - Take recorder rule (the addendum the ruling asked for): a HUMAN Tap / Resync point is stamped at the show time at
     which it took effect = its press stamp + the late amount, in the take's own unit; in the sample domain that is the
     raw delivered counter at the press (the audio he heard). Every other point keeps S3's rule. Check of the rule: a
     with-audio replay at a dial D' fires the point when the transport reaches stamp + D' and applies it at once
     (Origin::Replay), so the picture's beat lands D' after the audio he tapped to -- right in any room.
   - Docs: analysis.md "human anchors" rewritten; the ruling's AM-12 (c) sentence is replaced.
   - RED-first tests: test_bpm_stabilization [dial-tap] (not applied before P + late; applied at the first call at or
     after it; two taps inside the window both apply, in order; late = 0 and late < 0 -> applied in the very next call,
     identical counts to the base; value-only immediate; Replay immediate; seq contract; queue-full path);
     test_analysis_sync_thread [dial-tap] (serviceOnceForTest at D = 100: a Resync posted at t publishes beatInBar 0 /
     beatPhase 0 at >= t + 100 and < t + 100 + 2 hops); test_recorder_host [host][sync] + 2 cases (a human tap at +100,
     delivered 1,000,000 -> stored 1,000,000, a gesture in the same tick -> 995,200; with-audio replay at +100 fires
     the tap when the transport reaches 1,004,800). Each RED on a stub that applies at once / stamps like S3.
   - Gate: R9 flips (section 5).
 (b) The wheel. The music beat is the picture beat with the applied offset taken back out. Pure function in
   R:src/ui/TopBarModel.h (new): `WheelBeat musicBeat(float beatInBar, float beatPhase, uint32_t barCount, float bpm,
   float syncOffsetMs) noexcept` -> position = beatInBar + beatPhase + syncOffsetMs x bpm / 60000, re-folded into
   beatInBar (0-3), beatPhase and a barCount moved by the whole bars crossed (either direction). bpm <= 0 or offset 0
   -> identity (bit-identical inputs out). TopBar::timerCallback copies syncOffsetMs too and paints the wheel and
   "Bar n" from musicBeat(...). Nothing else reads it; no new snapshot field (Rule 8 untouched); no new timer
   (Pitfall 57); it is a display of one coherent snapshot, not a clock or an edge (Pitfall 42 not engaged).
   Known limits (disclosed, section 9): at a LATE dial the wheel answers a Tap / Resync D ms after the press (then sits
   exactly on it); during an EARLY slew or a hold it is off by the unslewed part (below the wheel's 67 ms tick).
   RED: tests/test_topbar_model.cpp new cases -- 0 -> identity; +100 ms at 120 BPM -> +0.2 beat; beat 3 phase 0.9 ->
   beat 0 phase 0.1 and "Bar" +1; -500 at 120 -> -1 beat, beat 0 -> beat 3 and "Bar" -1 (wraps 1 -> 4); bpm 0 ->
   identity. RED on a stub returning its input. Live: R12 (section 5).

(i) The control in the top bar

Boris: "Sync lives in top bar". (The page's question was "Where should it live?" with "The other choice is a small
button in the top bar that opens it.") "-500 to +500 in 1 ms steps plus can enter in the amount then click plus or
minus to fine tune." "Gain slider in top bar could be twice as long."
The space (E18, E19): at his 1728-wide window about 167 px are spare (103 in Manual mode -- the mode a tapping user is
in); doubling Gain takes 70; that leaves 97 / 33 px. A number box with - / + and a label needs about 86-120 px; a long
bar another 120+. At 1280 the bar is already 281 px short.
Alternatives:
 T1 CHOSEN: a "SYNC +42" button inside the existing 64 px tempo slot (zero added width, the note of page question 12
    and the control's door in one) that opens a drop-down panel holding the whole control.
 T2 RUNNER-UP: number box and - / + always visible in the left run (two stacked rows, 86 px), venue picker and bar in
    the drop-down; needs the BPM edit field moved into the tempo slot to fit in Manual mode, leaves ~11 px of margin
    at 1728 on an unmeasured label width, and shifts Tap / Resync right by 86 px more.
 T3 the panel in the BPM tab with a top-bar button that reveals it: no new popup, but it is the page default he did
    not choose.
Why T2 loses: it spends the bar's last pixels on a control he sets once per room, on a margin nobody has measured, and
moves a second existing control; T1 costs nothing and is the option the page offered him. If Boris wants the number
always in view (question 8.3), T2 is specified enough above to be built as a follow-up.
Parts (T1):
 TopBar (R:src/ui/TopBar.h/.cpp)
  - `syncButton_`: a flat text button, bounds (tempoLabel_.getRight() + 2, area top, 62, 13), 9 pt bold, text
    "SYNC 0" / "SYNC +42" / "SYNC -30"; ALWAYS visible (it is the only door). Colour kTextSecondary at 0,
    kTextPrimary otherwise; hover = the house button hover; never orange #ff4500, never red (AM-14's rule). Tooltip:
    "Sync: visuals 42 ms later -- venue 'Warehouse'. Click to change." (0: "Sync: in step -- venue ...").
    `void setSyncDisplay(int targetMs, const juce::String& venue)` (message thread), called from the controller's
    change notification (no polling; the painted text is compared, repaint only on change -- Pitfalls 57, 59).
    `std::function<void()> onSyncClicked`. The tracker-state label below it keeps its bounds.
  - Gain: `inputGainSlider_` width = TopBarModel::gainSliderWidth(usableWidth, rightGroupWanted) -> 140 when
    usable >= 1031 + 70 + 64 + rightGroupWanted (the Manual-mode field always counted, so toggling Manual never
    changes the Gain width), else 70. At 1728 -> 140 with about 33 px to spare (INFERRED, E19); at 1280 -> 70, i.e.
    today's layout pixel for pixel (no regression where the bar already overflows). Range, step, default, right-click
    reset unchanged (ResettableSlider, setDefaultValue(1.0) already set).
  - Existing layout tests re-anchored (tests/test_master_signal_link.cpp 1728 / 1280 cases, test_topbar_link_toggle,
    test_master_opacity_link, test_topbar_model).
 SyncPanel (NEW R:src/ui/SyncPanel.h/.cpp = plan I11, re-homed; I10 the TimingWindow slot is DROPPED)
  - Shown in a juce::CallOutBox launched from MainComponent with the main component as parent, pointing at
    syncButton_. A new pattern here (E20); the existing ones do not fit: a PopupMenu closes on the first click and
    cannot hold a text box plus a bar; an AlertWindow is modal and stops the show; the BPM tab was declined.
    ASSUMED (JUCE API from memory -- the builder confirms in the JUCE headers of the build's _deps before coding):
    CallOutBox::launchAsynchronously(std::unique_ptr<Component>, Rectangle<int>, Component* parent) adds an in-window
    child (no native window, so Pitfall 40 and the "menus dismiss on app switch" rule are not engaged) and closes on a
    click outside or Esc.
  - Content 360 x 96, house grammar (label column 50):
      Venue   [ Warehouse            v ]      one button; menu (showMenuAsync + withParentComponent(top level)): the
                                              venues (tick = current) | New venue... | Rename venue... | Delete
                                              venue... (warning red, confirm, greyed with one venue)
      Offset  [-] [ +42 ms ] [+]  [==========|==========]     value box 56, - / + 22 each, bar the rest (about 190)
              Visuals 42 ms later                                caption, #888
  - Behaviour = plan I11 unchanged: the bar is a ResettableSlider, LinearHorizontal, -500..+500, interval 1,
    setDefaultValue(0), no text box, bipolar fill from the centre (LookAndFeel "bipolar" property),
    setSliderSnapsToMousePosition(false); value box: "42", "+42", "-30", "42 ms" + Enter applies (clamped), Esc or
    anything else reverts; right-click on the box or the bar -> 0; - / + step 1 ms and repeat while held; captions
    0 -> "In step with the sound coming in", +N -> "Visuals N ms later", -N -> "Beats N ms earlier (loudness can't
    run early)". "New venue" starts from the current setting (page question 15). The value box takes focus when the
    panel opens; when the panel closes, focus goes back exactly as bpmEditField_ hands it back today (the keyboard clip
    launcher must not lose its keys).
  - The panel holds no truth: it registers as SyncOffsetController::Listener while it exists and re-pulls every widget
    on each notification, skipping the bar while it is dragged (Pitfall 41); it repaints only on change.
 TEST-ONLY routes (ApiServer, appended): POST /api/debug/sync_ui {"open":true|false | "type":"+42" | "click":"plus" |
  "click":"minus" | "rightClick":"value" | "menu":"open"}; GET /api/debug/sync_ui_dump -> the button's text, colour
  and bounds, the tracker-state label bounds, the panel's open flag and every widget's bounds and text, the Gain
  slider bounds, the right group's bounds vs wanted widths, wheel {beatInBar, beatPhase, bar, nowMs} computed at the
  request from a fresh bus read through the same musicBeat() the paint uses (the 15 Hz painted copy is too coarse to gate).
 RED tests: tests/test_sync_panel.cpp (plan I11's list, unchanged); tests/test_topbar_sync_button.cpp (replaces
  test_topbar_sync_indicator: "SYNC 0" dim and visible at 0; "SYNC +42" / "SYNC -30" bright; tooltip names the venue;
  setSyncDisplay with the same values does not repaint; click fires onSyncClicked; bounds inside the tempo slot and
  not intersecting the BPM number or the tracker-state label; Pitfall 34: setVisible(true) explicitly);
  tests/test_topbar_model.cpp gainSliderWidth cases (1712 -> 140, 1264 -> 70, the threshold itself, Manual does not
  change it). Each RED on a stub first.

(ii) Persistence: the composition carries the venue

Boris: "For sync control we should have a way to remember it as part of a composition save. I imagine if a user is
doing this professionally, they will set up the venue and save it in case of a computer crash or something and if they
come back to that venue, they have the settings already."
Facts: E21. The app's venue store stays (it is what brings the app back on the last venue, page question 12).
 File key: top-level "sync": {"venue": "Warehouse", "ms": 42}. Written by Composition::toVar when the model's
  syncPresent flag is set; read by fromVar behind hasProperty (ms clamped to -500..+500, name cut to 40 characters;
  a malformed value = absent). Model: `struct Composition::SyncSetting { bool present = false; juce::String venue;
  int ms = 0; }`.
 Saving (MainComponent::saveCompositionTo, before toVar): present = !(current venue is "Default" and ms == 0); venue
  and ms copied from the controller. So a show saved at home on the untouched default carries no key.
 Old files and New: no key -> present false -> the live dial is NOT touched (a fresh Composition's defaults are never
  applied: the adopt hook runs only when present).
 Loading a file WITH the key (hook in finishStagedLoad after the model swap, message thread): NEW
  `void SyncOffsetController::adoptFromComposition(const juce::String& venue, int ms)`:
    the store has no venue of that name -> create it with ms and select it;
    it has it with the same ms           -> select it;
    it has it with a different ms        -> the FILE's value replaces the store's, then select it.
  The dial glides (page question 13); the SYNC button shows the new value; the load notice says, only when the live
  value or venue actually changed, "Sync: Warehouse +55 -> Warehouse +42 (saved in this composition)" (old and new, so
  a value lost to an older file can be typed back). With persistence off (test env) the adoption is in memory only.
 What wins when the same venue name holds a different value in the store: the file (Harmony's default as put to
  Boris). RUNNER-UP: newest wins (a changedAt stamp on each venue and in the file). It protects one case -- he tuned
  tonight, did not save, the app crashed, he reloads an older save -- but makes loading unpredictable ("sometimes the
  file's number loads, sometimes not"); the notice with the old number covers that case with a rule he can predict.
 The hazard that remains, stated plainly: loading a composition saved in another room moves the dial live. It is what
  he asked for; the glide, the notice and the SYNC button make it visible; question 8.2 offers the other behaviour.
 RED tests: tests/test_composition.cpp (round trip of the key; absent -> present false; malformed -> absent; clamp);
  tests/test_sync_offset_controller.cpp adoptFromComposition (the three cases; a Listener notification each time; with
  persist on the store's JSON equals the expected text; no call when present is false -- tested at the hook through a
  seam `applyCompositionSync(const Composition&)`); the staged-load test file gains "an old file leaves a +42 dial at
  +42". Live: R11 (section 5).

(iv) The visual work gate for S5: section 5, G7 (states, measurable bars, critic questions).

## 4 BUILD STAGES + ROAD TO MERGE (B7)

Order (one builder context per stage; a reviewer pass after each; Harmony runs the live rows, never a builder):

 M0  MERGE-IN, after the deck change is in main. `git merge main` INTO lane/bf2 (no rebase: 24 commits of evidence keep
     their hashes). Expected hand-merges (E2): tests/CMakeLists.txt and CMakeLists.txt (both sides append),
     src/MainComponent.cpp / .h and src/api/ApiServer.cpp / .h (the lane's hunks are appended routes, members and one
     provider block), src/render/Renderer.*, tests/test_recorder_host.cpp, docs (architecture, integration, pitfalls,
     recording, testing-eyes), APP-INVENTORY.md, CLAUDE.md, probe-tsan-unit.sh (the [tsan] case count). TopBar.*,
     Composition.h, BindingTarget.h come from main untouched (the lane never edited them). "Pitfall NN" -> 68 in every
     file that says NN (grep "Pitfall NN" must return 0). No behaviour change. Exit: G1 (whole suite), G2, G3.
     Why first (runner-up: finish S3 / S4 on the old base, merge last): every remaining stage edits a colliding file,
     S5 can only be built on the post-deck-change top bar, and Harmony's merge gates must run on the merged tree
     anyway -- merging once, first, means every later RED / GREEN is on the code that ships.
 S3f "S3 finish": B1 + B3 probe rows and the mutant-app RED; mutant M3; B4 (i) Resync-heals assertion if missing;
     B4 (iii) probe-sync-perf.sh and R1a over the -500 window; docs sentences of B2 / B4. No src change expected.
 S4  "Keys & MIDI": plan I9 as ruled; line references re-read on the merged tree (BindingTarget.h changed on main).
 S6  "Taps go through the dial": B6 (iii)(a). Skipped if Boris answers 8.1 "no".
 S5a "Remembered with the composition + the wheel": B6 (ii) and (iii)(b); no visible widget; unit + R11 + R12.
     (The wheel part is skipped with S6.)
 S5b "The control on screen": B6 (i) -- TopBar button, SyncPanel, Gain width, test routes, captures. Ends at G7.
 Reviews: after M0 a merge review (conflict hunks only); after S3f a gates review (can each row fail? RED evidence);
     after S6 a real-time review (no allocation, no mutex, seq contract, recorder rule); after S5a a state review
     (load / save / adopt paths); after S5b a UI correctness review; then G7's critic panel; then Harmony's gate list.
 Then: `git merge main` once more if main moved, the FINAL gate list (section 5) on that tree, merge to main.
 S5 before or after the merge-in: AFTER (M0 first) -- the top bar it lands on exists only there.

## 5 HARMONY'S GATE LIST (final; supersedes the ruling's list where a row is restated; rows not restated are kept by id)

Rig: as P:ruling-bf2.md:386-390, unchanged. Harmony constraint: an Audio-DNA the run did not start is Boris's -- never
quit or touch it. Kept by id, text unchanged: G1-RED, G2, G3 (a) (b), G4 R0, R1, R2, R3, R4, R4b, R6, R8, G5.

G1  ctest, default build: 0 failed, 0 skipped. Suites added to the ruling's list -- S3f: test_beat_lead (T-L4 Resync
    heals); S4: test_binding_sync_nudge; S6: test_bpm_stabilization [dial-tap], test_analysis_sync_thread [dial-tap],
    test_recorder_host [host][sync] (7 cases); S5a: test_composition (sync key), test_sync_offset_controller (adopt),
    test_topbar_model (musicBeat); S5b: test_sync_panel, test_topbar_sync_button, test_topbar_model (gainSliderWidth),
    the re-anchored top-bar layout tests. REMOVED from the ruling's list: test_timing_window_content,
    test_topbar_sync_indicator. [timing]: command and pass line in B5; the ruling's re-run rule unchanged.
R1a (changed) as ruled, PLUS (ii) and (iii) over R4's settled -500 window.
R5  (changed clause) "...; on every non-re-base hop whose raw fold advanced >= aN - 1e-4 (a free-running hop), the
    target fold advanced >= 0.75 x aN - 1e-4. ..." (unit T-L3 (iv): the same with 1e-6). The rest of R5 as ruled.
R7  (changed, S3) take alignment: click takes with onset markers (audio:true), arms [0, 100] x 10 + [0], one launch.
    Per arm: markers farther than 1,024 samples from the arm median dropped and counted; trimmed mean. d_i = 100-arm
    trimmed mean - mean of its two neighbours'. BAR: |mean of the ten d_i| <= 1.0 ms; every arm >= 100 paired markers
    and <= 4 dropped. If the result is within 2 x SE of 1.0: one more identical pass, the pooled 20 decide. RED on
    record: the S2 behaviour reads +99 to +100 ms. A failure below 2.67 ms goes to the architect.
R7b (changed, S3) inside R7's first 100 arm, k = 10, 20, 30, trigger = the recorder's own marker count:
    (i) INVARIANT gesture stamp - marker k stamp in [0, 2,880] samples; (ii) (gesture stamp - grid position of click
    k) - (median marker error of the first 0 arm) in [-512, +2,880] samples. RED for (ii): the mutant app, about
    +4,800.
R9  (changed, S6; BLOCKING) at +100 and +253, manual 120 BPM, POST /api/resync three times per value, >= 3 s apart:
    the first witness hop whose trackerRequestSeq exceeds its pre-POST value publishes the Resync (beatInBar 0,
    beatPhase 0, resyncBarOrigin == totalBarCount) and its processMs is D +- 15 ms after the POST returned. At 0 and
    at -100: <= 50 ms (as ruled). [If Boris answers 8.1 "no": R9 as ruled, at every value.]
R10 (changed, S5b, TEST mode, scratch settings) POST /api/debug/sync_ui {"open":true} -> dump shows the panel open,
    inside the window, the value box focused; {"type":"+42"} -> GET /api/sync targetMs 42 within 200 ms and the
    button reads "SYNC +42"; {"click":"plus"} -> 43; {"click":"minus"} twice -> 41; {"type":"abc"} -> still 41, the
    value box reads "+41 ms"; {"rightClick":"value"} -> 0 and the button reads "SYNC 0"; POST /api/sync/set {"ms":-30}
    while the panel is open -> the panel's box reads "-30 ms" within 200 ms (Pitfall 41); {"open":false} -> closed.
R11 (new, S5a, TEST mode, scratch settings and scratch files) (a) set venue "Warehouse" +42, POST
    /api/debug/save_composition -> the file's "sync" equals {"venue":"Warehouse","ms":42}; on "Default" at 0 -> the
    file has no "sync" key. (b) dial at +42, load a file WITHOUT the key -> GET /api/sync unchanged (venue, targetMs).
    (c) select "Default" 0, load the file of (a) -> venue "Warehouse", targetMs 42. (d) set "Warehouse" to +55, load
    the file of (a) -> targetMs 42, the store's "Warehouse" is 42, the load notice text names +55 and +42. (e) a store
    without "Warehouse" -> created at 42 and selected. (f) the real ~/Library/Audio-DNA/settings.json sha256
    unchanged around the run.
R12 (new, S5a + S6, PRODUCTION mode, test-server build, manual 120 BPM) for D in {+200, -200, 0}: set, wait settled,
    sample 20 pairs {sync_ui_dump wheel (fresh read, see B6 (i) test routes), witness published beat nearest its nowMs}: wheel position - published
    position (mod 4 beats) = D x 120 / 60000 beats +- 0.05 (i.e. +0.40, -0.40, 0). Then at +200: POST /api/resync;
    1 s later the wheel's beat-0 crossings sit within +-40 ms of (POST time + k x 500 ms), while the published ones
    sit 200 +- 15 ms later.
G6  (changed) .harmony/probe-sync-perf.sh as specified in B4 (iii): launch-by-launch, order 0, -500, +500 x 5, quiet
    machine. INFO table. TRIPWIRE (blocks): an arm more than 1.0 point of cpuLoad or 1.0 ms of frameMs above the 0
    arm, when 2 x within-arm SD is below that; otherwise INCONCLUSIVE -> repeat quiet.
G7  (changed, S5b) VISUAL WORK GATE, before Boris sees anything. TEST mode, scratch settings, states set by REST and
    /api/debug/sync_ui; the main window captured by Quartz window id at 1728 x 1000 and at 1280 x 720, cropped to the
    top bar (and to the panel when open); the dump read at each capture.
    States: C1 first launch, 1728: "SYNC 0" dim, Gain 140 px, Auto mode; C2 the same in Manual mode (BPM field
    shown; Gain still 140; Master Signal label whole); C3 +42 on "Warehouse": "SYNC +42" bright; C4 -30; C5 +500 and
    -500 (widest text fits 62 px); C6 hover on the button; C7 the panel open at 0 (box focused, caption "In step
    with the sound coming in"); C8 the panel at +42 and at -30 (bar fill from the centre, captions); C9 the venue
    menu open (3 venues, tick, Delete in warning red); C10 New venue and Rename dialogs; C11 a refusal (duplicate
    name) in the caption line; C12 the panel open while REST changes the value; C13 just after loading a composition
    that changed the dial (button + load notice); C14 1280 x 720: Gain 70, today's layout otherwise, the button
    present, the panel fully inside the window; C15 the Gain slider at 1.0 and at 0.25 (1728) next to a capture of
    today's main for the same two values.
    Measurable bars (dump): at 1728 in Auto AND Manual every right-group widget has its wanted width and the Master
    Signal label is not cut; Gain 140 (1728) / 70 (1280); the button inside the tempo slot, 62 x 13, overlapping
    neither the BPM number nor the tracker-state label; every text fits its bounds; panel widgets do not overlap and
    lie inside the panel; the panel lies inside the window at both sizes; at 1280 every top-bar widget's bounds equal
    main's (dump of main recorded first) except the added button.
    Critic panel in parallel, each given the PNGs, this section, B6 (i), BORIS_DECISIONS.md grammar / rejected list;
    PASS / FAIL per item:
     visual-design: is "SYNC 0" clearly quieter than "SYNC +42" yet readable? does a 13 px button above the tracker
      state crowd the BPM number? is the doubled Gain visually balanced against its neighbours? colours from the
      palette only, no orange #ff4500, red only for Delete?
     UX: can a first-time user find the control (is a 62 x 13 target enough for the only door)? is the sign obvious
      without the caption (is the tooltip enough)? is one click to reach the number acceptable for a set-once control?
      does the load notice say what changed in plain words?
     graphic-design: grammar widths, 9 pt / 11 pt hierarchy, caption #888, panel alignment with the house inspector
      rows, no round control.
     logic: every state's text equals the model (dump vs GET /api/sync) incl. C12 and C13; 1280 unchanged.
     interaction-logic (stateful control): type -> Enter / Esc / garbage; - / + hold-repeat; right-click resets on the
      box and the bar but not on - / +; a drag does not fight a REST change (C12); the panel closing returns the
      keyboard to the clip launcher; opening the panel twice never makes two; deleting the current venue; a venue
      switch glides; loading a composition while the panel is open.
    Any FAIL blocks unless Harmony records why it is taste-only. Then the page Boris opens (section 7).
Stage -> gates: M0: G1, G2, G3. S3f: G1, G1-RED, G4 R7, R7b, R1a (-500), R5, G6. S4: G1, G1-RED. S6: G1, G1-RED, G2,
    G3 (b), G4 R9, R3, R8. S5a: G1, G1-RED, R11, R12. S5b: G1, G1-RED, R10, G7.
At the merge Harmony runs: G1, G2, G3, G4 (R0, R1, R1a, R2, R3, R4, R4b, R5, R6, R7, R7b, R8, R9, R12), G5, G6, R10,
    R11, G7 (already passed; re-shot only if S5b files changed since).

## 6 DOCS (CLAUDE.md stays <= 25,000 bytes)

 CLAUDE.md: after M0 measure `wc -c` (E3: 24,264 at a7491d4 before main's lines are merged in -- the room left is
  about 600 bytes, INFERRED). Budget for this lane: <= 350 bytes total = one Pitfall 68 index line + one UI Patterns
  line ("Sync: the top-bar SYNC button and its panel follow the controller; a composition carries the venue --
  docs/claude/integration.md"). The builder prints `wc -c CLAUDE.md` in its report; above 25,000 is a STOP.
 docs/claude/pitfalls.md: Pitfall 68 (was NN). docs/claude/analysis.md "Sync offset": the music / picture rule, taps
  go through the dial, the origin cap in words, the "free-running hop" wording. docs/claude/recording.md: the replay
  end (B2) and the Tap / Resync stamp rule. docs/claude/integration.md: the composition "sync" key, load / save
  rules, the test routes. docs/claude/rendering.md or performance-controls.md: the wheel shows the music beat.
  .harmony/APP-INVENTORY.md: the top-bar surface row, the Gain width rule.

## 7 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

 7.1 Tap along to a track with Sync at 0. -> The top circle pulses on your taps. Now open SYNC and type +150. -> The
     circle keeps pulsing exactly on the music; the picture's beat motion now lands a little after it. Wrong: the
     circle drifts off the music, or the picture does not move later.
 7.2 With Sync still at +150, tap the tempo again. -> The circle is back on your taps within about a sixth of a second
     and the picture stays that same bit behind. Wrong: after tapping, the picture is suddenly back on the beat (the
     dial "stopped working"), or the circle sits late for good.
 7.3 Type -150. -> Beat-locked motion lands a little BEFORE the music; loudness-driven motion does not run early (it
     cannot). The circle stays on the music. Wrong: the circle runs ahead of the music.
 7.4 Record a short take with music at Sync 0; set Sync to +300 and replay it. -> Everything plays a touch late, as
     set; in the last third of a second the final moves may arrive together as the music stops. Wrong: the replay
     never ends, or the whole take is out of step.
 7.5 At -300, tap the tempo several times, do NOT press Resync, and watch a 4-bar effect. -> It may restart on a
     different bar than before; press Resync on the one and it is right. Wrong: it stutters backwards or double-fires
     at each tap. Tell us if "one bar off until Resync" bothers you.
 7.6 Click SYNC, make a venue "Test room", set +42, save the composition, quit, reopen, load it. -> SYNC +42, venue
     "Test room". Set it to +55 without saving, load the composition again. -> It glides back to +42 and a yellow note
     says it changed from +55. Load an old composition. -> Sync does not move.
 7.7 Gain: drag it in the part you use (below a quarter). -> Twice the room to move. Is that enough?
 7.8 Look at the top bar in your usual window size, Manual on and off. -> Nothing is cut off on the right; SYNC sits
     above the small tracker word next to the BPM. Is it easy enough to hit?

## 8 QUESTIONS FOR BORIS (each has a default; nothing waits)

 8.1 When you tap the tempo, the circle at the top stays exactly on your taps -- on the music you hear -- and the Sync
     number moves only the picture, later or earlier than that. DEFAULT: yes. (This also means: with Sync at +100, a
     Tap or Resync shows up in the picture 100 ms after you press. Earlier we said "where you pressed" -- that now
     describes the circle, not the picture.)
 8.2 When you open a composition that was saved with a Sync setting, the app switches to that venue and number (it
     glides there and a note tells you what changed). DEFAULT: yes. The other way: keep what you have now and just
     offer the saved one with one click.
 8.3 Sync is a small "SYNC +42" button next to the BPM; clicking it drops down the number box, the - / + buttons, the
     long bar and the venue list. DEFAULT: that. The other way: keep the number and - / + always visible in the top
     bar (it pushes Tap and the buttons after it further right and leaves almost no spare room).
 8.4 Gain: twice as long, same scale. DEFAULT: that. The other way, on top: stretch the low part so 0 to 1 takes half
     the slider.

## 9 RISKS (strongest counterargument first; the cheapest refuting test for each choice)

 K1 (strongest) Building S6 on a reading of his words. He wrote that all other defaults are good, and the page default
    for Tap was "where you pressed"; S6 changes a ruled, gated behaviour (R9) and touches the tracker's request path
    one stage before a merge. Why the position stands: E15 + E16 show the as-built LATE dial is order-dependent, which
    neither answer to the page question describes, and his own paragraph describes W2. Cheapest refutation: his answer
    to 8.1 (one sentence) BEFORE the S6 builder starts -- S3f and S4 come first, so nothing waits; or, without him,
    R3-then-R9 on the current lane app in one session (dial 0 -> +200 in manual, read the beat's shift; then Resync,
    read it again): if the shift survives the Resync, E16's consequence is wrong and S6 is unnecessary.
 K2 R7 at 1.0 ms may fail for a real 0.3-0.5 ms systematic term plus noise (B1). The extension rule and the "finding,
    not flake" route handle it; the fallback (2.0 ms, 5 arms) is pre-stated. Refutation of the SE estimate: the INFO
    lines of the first real run (SD of the ten d_i); if it exceeds 1.0 ms the design is under-powered and goes back.
 K3 The un-shifted wheel answers a Tap D ms late at a LATE dial (up to 0.5 s at the extreme; one or two wheel ticks at
    typical room values). Check 7.2. If it bothers him: a follow-up shows a pending Tap on the wheel at once.
 K4 The SYNC button is a 62 x 13 target and the only door. G7's UX critic judges it; fallback inside the same slot:
    in Manual mode (tracker word hidden) the button takes the slot's full height. Refutation: check 7.8.
 K5 "The file wins" can overwrite tonight's tuning with an older save (B6 ii). The notice shows the old number.
    Refutation: his answer to 8.2; R11 (d) pins whichever is built.
 K6 The 1728 budget rests on an unmeasured label width (E19): if the right group wants more than ~547 px the Gain
    rule yields 70 at 1728 and Boris gets no longer slider. Cheapest test: G7 C1 / C2's dump -- run it FIRST in S5b,
    before any panel work; if Gain is 70 at 1728, stop and return to the architect (candidates: the Manual field into
    the tempo slot's lower half, or a shorter right-group gap).
 K7 CallOutBox is new here and its API is recalled, not read (B6 i). Cheapest test: the builder reads the JUCE header
    in the build's _deps first; if it creates a native window or cannot take a parent, fall back to a plain child
    component of MainComponent placed under the button and closed on outside mouse-down.
 K8 The merge-in (M0) was not dry-run (fence). If conflicts are larger than E2 suggests, M0 reports before resolving
    anything in MainComponent.cpp beyond the lane's own hunks.
 K9 Origin cap accepted (B4 i): several Taps at a large EARLY value leave multi-bar shapes on an arbitrary bar until
    Resync. Check 7.5.
 K10 S6's queue producers are assumed to be message-thread only; an off-thread caller would make it multi-producer.
    The builder's call-site audit is a STOP condition, not a note.

STATUS: DONE -- plan-bf2-delta, 2026-10-03; items B1-B7; stages M0, S3f, S4, S6, S5a, S5b; 4 Boris questions with
defaults; 10 forks for the council.

## HARMONY ADOPTION (s-rta-1003, 2026-10-03 14:49:54) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1003/ruling-bf2-delta.md IN FULL — amendments D1..D22; stages M0 (merge main, with the
   deck change, INTO lane/bf2) / S3f / S4 / S6 / S5a / S5b; section 5's gate list (the only source of gate strings);
   sections 6-7. S6 (taps go through the dial) and the music-beat wheel are UNCONDITIONAL: Boris answered "yes" to "the beat
   wheel stays exactly where you tapped; only the picture is shifted" and "yes" to "opening a composition loads its saved
   venue and sync value" (binding-decisions.md, 2026-10-03, answers 9 and 10).
2. BORIS QUESTIONS — defaults until he answers: Gain = twice as long, same scale; a show saved while Sync is on "Default" at
   0 does not remember a Sync setting.
3. ORDER (Harmony): the deck change (bf9b) merges first. S3f and S4 may precede M0 by the ruling, but NO bf2 builder starts
   while bf9b's FIX stages build: (a) S3f's timing rows (R7 21 arms, G6, [timing]) need a quiet machine that a second
   compiling lane denies; (b) the s-rta-1002b session hit a usage-limit stop with parallel opus lanes, and the bf9b merge is
   the critical path today. Facts M1-M7 are measured by the stage the ruling names, M2 / M3 / M6 by Harmony in a quiet window.
4. PITFALL: lanes write "Pitfall NN"; Harmony assigns 68 at the merge. CLAUDE.md stays <= 25,000 bytes (and T26 STEP 1 will
   cut it to <= 8 KB after the bf9b merge: docs for this lane go to docs/claude/*.md, not CLAUDE.md).
5. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — the lock helper, own-pid quits only, no Output window, no
   synthetic input. S5b is a visual deliverable: critic panel (visual-design, UX, graphic-design, logic, interaction-logic)
   on decoded captures before Boris sees it. MERGE by Harmony.
6. OVERRIDE (2026-10-03 14:55:32; Boris, verbatim: "every show remembers it's sync"): EVERY save writes the sync key — the venue name and
   the value, including "Default" at 0 — and opening such a file sets the dial to it (the SYNC note shows the change). The
   ruling's rule "a show saved while Sync is on Default at 0 carries no key" is withdrawn. A file WITHOUT the key (an old
   show) still never touches the dial. Gain: "yes" = twice as long, same scale (no low-end stretch).
7. OVERRIDE (2026-10-03 15:30:49; Boris, verbatim: "We don't need any text indicating what has happened or what has happened. That is
   something that happens online and is not necessary in this application. It is extra overhead and bloat. Please remove it
   cleanly and completely."): NO notice text when a composition open changes the dial (the ruling's "notice shows old and
   new" is withdrawn, together with any other event text S5a / S5b would add). The SYNC button always shows the CURRENT
   value: that is a state display and stays. The ruling's "replaced value kept reachable" stays only as a control (not a
   message), or is dropped if it needs a text: the S5a builder reports which.
