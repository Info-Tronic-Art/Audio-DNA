# RULING bf2 DELTA -- architect ruling on the blind council's attacks (lane bf2, s-rta-1003)

Plan attacked: .harmony/.reports/s-rta-1003/plan-bf2-delta.md (669 lines, read in full).
Seat papers, verbatim: .harmony/.reports/s-rta-1003/attack-bf2-delta-papers.md (3 seats, 29 attacks).
Read-only work: nothing was built, run, launched or probed. The only computation is python3 statistics on per-marker
errors already saved by the S3 builder (S3LOG below).
Paths: W: = .claude/worktrees/bf2 (lane/bf2, head 4a1f240). R: = .claude/worktrees/recon (a7491d4, the app after the
deck change). P: = .harmony/.reports/s-rta-1002b (plan-bf2.md, ruling-bf2.md). JUCE: = build/_deps/juce-src (8.0.4,
CMakeLists.txt:46). S3LOG = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/
73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf2-S3.
Labels: VERIFIED (read at the cited line, or computed from a saved file), INFERRED (reasoned from verified lines),
ASSUMED (not checked). "D" = the dial value in ms.

## 0 VERDICT

SOUND WITH AMENDMENTS. The plan's direction stands: finish S3, build S6, put the control in the top bar, carry the
venue in the composition. Two of its choices are reversed and nine gate rows are re-stated.

 1. Boris answered the plan's two open questions "yes" about 80 seconds after the plan was written (F1). S6 and the
    music-beat wheel are no longer conditional. This also settles the one conflict between seats: SO-1 ("default NO
    until he answers") is overtaken, T4 ("he already answered") is right.
 2. REVERSED: the origin cap (B4 i). It does not stop anything running backwards. It turns a 0.3-beat step back into
    a 3.7-beat leap forward for every 2 / 4 / 8-bar shape and leaves them a bar ahead until Resync (F15). Appendix A's
    rule is built instead (D5). The runner-up (keep the cap, corrected disclosure) is named with its switch condition.
 3. REVERSED: the CallOutBox. It is modal and takes the keyboard (F22). The panel becomes a plain child that takes
    neither (D16).
 4. Gates that could not fail or could not be read, re-stated: G6 (blind to LATE, never engaged EARLY, read a field
    that does not exist), R12 (compared a function with itself), R7b (i) (true on any tree), R5's hold clause (passes
    with zero qualifying hops), R9 (its bar ignored that the POST returns before the request is stamped), R11 (drove
    only Save As), G7 (four states no driver could reach), R7's teeth claim, R7b (ii)'s bounds.
 5. Behaviour holes closed: plain Save and Collect media wrote no sync key (D13); a take armed after a Tap at a dial
    above about 250 ms started from a stale snapshot (D9); the wheel answered a Tap D ms late at a LATE dial, against
    his "yes" (D12); a typed number was lost unless Enter was pressed (D17); the SYNC button overlapped the tracker
    word by 1 px (D18); a deck load could have re-tuned the dial (D14).

29 attacks ruled: 19 ACCEPT, 9 PARTIAL, 1 REJECT. 22 amendments (D1-D22). Stages: M0, S3f, S4, S6, S5a, S5b.
Ready to build: yes. Nothing waits for Boris: two questions, each with a default.

## 1 FACTS RE-DERIVED (every seat citation re-read; the plan's too where a ruling rests on it)

Boris
 F1  Q9 and Q10 are answered. binding-decisions.md:658 (recorded 2026-10-03 14:03:17), :669-670: "Q9 the beat wheel
     stays where he tapped, only the picture is shifted - Boris: "yes". Q10 opening a composition loads its saved venue
     and sync value - Boris: "yes"". His message verbatim: boris-feedback-backlog.md:181-196 ("9 yes", "10 yes"); the
     questions as asked: :212-214. The plan file's mtime is 14:01:57 (`stat`). VERIFIED.
 F2  Page question 16's default was "where you pressed" (P:boris-checks.html:48); the ruling made S6 conditional on
     that question (P:ruling-bf2.md:279-302, :378-380). VERIFIED. His own paragraph, same message as "All the other
     defaults are good." (boris-feedback-backlog.md:103, :133): "The user will try to have the beat matched exactly to
     the music so the user can see it pulsing exactly to the time of the music." "The visuals should be on the delay.
     The music is in time and the visuals should be delayed or a little ahead depending on how the system is wired."
     VERIFIED.

G6 (T1, A3)
 F3  The DSP-load timer starts at W:src/analysis/AnalysisThread.cpp:211 and stops at :469 (EMA at :473-474). The
     drain, the slew and the due gate are in serviceOnce, :124-162, outside that window. BeatLead::apply is called at
     :453-457, inside it. VERIFIED. (The seat wrote "run()"; it is serviceOnce. Immaterial.)
 F4  BeatLead does nothing when the tempo is 0: deltaT = 0 (W:src/analysis/BeatLead.cpp:146-148) and an identity
     return (:156-165). VERIFIED.
 F5  GET /api/status carries fps, frameTimeMs, masterLevel, masterSignal, activeDeck, renderOnsetPulses and the bpm
     fields (W:src/api/ApiServer.cpp:373-392). No analysis load. No "frameMs". The only reader of getCpuLoad() is
     W:src/MainComponent.cpp:4176 -> TopBar::setDspLoad (:4188). AUDIODNA_SYNC_TEST is read in TEST-SERVER builds only
     (W:src/sync/SyncOffsetController.h:24, .cpp:9-20). VERIFIED.
 F6  G6 has never been run at any stage, and no script exists (W:.harmony/.reports/s-rta-1002b/bf2.md:361, :611, :821,
     :1045). VERIFIED.
 F7  The per-hop witness entry is written inside processHop after the load is computed, TEST-SERVER builds only
     (W:src/analysis/AnalysisThread.cpp:526-571); elapsedUs (:469) is in scope there. VERIFIED.

R7 / R7b (T2, T3, T9, A9)
 F8  Recomputed from S3LOG/out-green2/run-20261002-212823-74596/probe-sync.json (the only saved run with per-marker
     data; green1 and the S2 run saved arm summaries only). Per arm (D = 0 / 100 / 0), 121 markers each: medians
     30.667 / 29.333 / 30.667 ms; means 31.129 / 30.656 / 30.689; SD 6.80 / 8.71 / 6.51. Dropping markers farther than
     21.33 ms from the arm median drops exactly 1 per arm (4,416 / 4,736 / 4,416 samples); trimmed means 30.622 /
     30.089 / 30.178, so the run's trimmed difference is -0.311 ms. Bootstrap, 4,000 draws: SD of one run's trimmed
     difference 0.61 ms; per-arm trimmed-mean SE 0.37 / 0.56 / 0.31; predicted SE of a 10 + 11-arm statistic 0.20 ms
     (group-mean form), 0.19 (the plan's formula). With SE 0.20: P(FAIL | +1 lattice step, 1.333 ms) = 0.95 at zero
     bias, 0.55 at -0.31, 0.21 at -0.5; P(FAIL | +2 steps) = 1.00; P(false FAIL) = 0.0004 at -0.31, 0.007 at -0.5.
     VERIFIED (computed). These SEs see within-arm noise only; arm-to-arm drift over 23 minutes is UNKNOWN.
 F9  The two S3 runs disagree in sign on the mean: +0.68 and -0.25 ms (W:bf2.md:14-15). VERIFIED.
 F10 R7b from the same file: gesture - marker k = 0 / 0 / 0 samples (green2), 512 / 0 / 0 (green1), 0 / 0 / 0 (the S2
     app). In the 100 arm, marker error - B (B = 1,472 samples, the first 0 arm's median = the pooled 0 arms' median):
     lowest -448 (10 markers), then -384 (12 markers); highest body value +512; one stray at +3,264. Each 0 arm has one
     stray at +2,944. VERIFIED.
 F11 The probe as built: rate from the take's segment (W:.harmony/probe-sync.py:833), R7b bar 0.060 x rate (:926),
     arms [0, 100, 0] (:864), verdict on medians (:943-946), the lattice computed by gcd (:953-959). VERIFIED (the
     seats' line numbers are a few off; the content matches).
 F12 R5's hold clause as built: antecedent raw advance >= aN - 1e-4 (:803), bar (:805), verdict "applied > 0 and not
     slow" (:816-818). The number of hops that met the antecedent is neither required nor printed. VERIFIED.

Recorder (T6, A2)
 F13 Replay: the late amount comes from the snapshot at the tick (W:src/recording/RecorderHost.cpp:456); pos = endPos -
     late (:578); the end edge compares endPos and fires the tail with advanceTo(playEndPos_) (:586-591). VERIFIED.
     Routines do NOT go through it: RoutineEngine drives its own Player on beat positions
     (W:src/recording/RoutineEngine.cpp:455, :582, :621-647) and contains no syncOffset, lateSeconds, WithAudio or
     transportFrames (grep = 0 lines). VERIFIED.
 F14 Take start: kStartWaitFallbackSeconds = 0.25 (W:src/recording/RecorderHost.h:285); startDue returns true on that
     timeout and the take starts from the latest snapshot (W:src/recording/RecorderHost.cpp:612-628); Record stores
     postedRequestSeq() (W:src/MainComponent.cpp:5796). VERIFIED.

Tracker requests (S6, T5, R9)
 F15 Origin cap, walked through W:src/analysis/BeatLead.cpp:184-212. Scenario: tracker at beat 3, phase 0.3; lead 0.8
     beat; the published view is therefore the NEXT bar, beat 0, phase 0.1, K bars since Resync. A Tap in the first
     half of the beat sets the tracker to beat 3, phase 0 (F16). The re-based view is beat 3, phase 0.8 of the EARLIER
     bar; one bar is absorbed (:193-198); the cap (:205-209) pulls the origin back so barsSinceResync() stays K. The
     fold every multi-bar shape uses, 4 x bars + beatInBar + beatPhase (W:src/signal/OscillatorSignal.h:56-61,
     W:src/signal/EnvelopeSignal.h:55-60, W:src/connect/ConnectionEngine.cpp:98-102), goes from 4K + 0.1 to 4K + 3.8:
     +3.7 beats. Without the cap (origin += absorbed): 4(K - 1) + 3.8, i.e. -0.3 beat, the tracker's own step.
     INFERRED from the code (a walk, not a run). Appendix A contradicts itself: rule 2 shifts the origin
     (P:plan-bf2.md:685-686) and its T-L3 line says barsSinceResync() never decreases (:702-703). Mutant A (no cap)
     fails only T-L6 "2 >= 3" (W:bf2.md:450-452). VERIFIED.
 F16 A Tap is one coalescing atomic word (W:src/analysis/BPMTracker.cpp:586-611), applied at the next runPipeline
     (:76-84): tempo set, phase to 0, totalBeatCount + 1 when the phase was >= 0.5, beatInBar NOT touched (:253-258,
     :613-631). A Resync is a counter (:565-569): beatInBar 0, barCount 0, origin = totalBarCount (:571-584). There is
     no request queue today. foldBPMToRange is a public static (W:src/analysis/BPMTracker.h:337, .cpp:281-291).
     VERIFIED.
 F17 Every human tempo command reaches applyTempoCommand on the message thread: the top bar (W:src/MainComponent.cpp:
     596-611), REST through callAsync (W:src/api/ApiServer.cpp:837-856), OSC through MessageLoopCallback
     (W:src/osc/OscHandler.h:31-32), bindings in handleBindingAction (:7584, :7767, :7775; MIDI marshalled at :480),
     Link in the timer (:4212), replay (:1990). VERIFIED for REST and OSC, INFERRED for the binding path.
     CONSEQUENCE: POST /api/resync returns BEFORE the request is stamped (fire-and-forget callAsync).

[timing] (T8)
 F18 Bars at W:tests/test_analysis_sync_thread.cpp:664-693; the per-pair numbers are INFO lines, shown only on a
     failure (:677). Registered by catch_discover_tests TEST_SPEC "~[tsan]" (W:tests/CMakeLists.txt:3488); no
     RUN_SERIAL, no RESOURCE_LOCK anywhere in that file (grep = 0). VERIFIED.

Top bar (SO-2, SO-3, SO-6, SO-8, A5, A6, A7)
 F19 R:src/ui/TopBar.cpp: area = bounds.reduced(4, 2) (:531); Gain label 30 + slider 70 (:537-538); everything after
     it follows in one removeFromLeft chain (:534-604); tempo number at y 8 (:565); tracker word at (right + 2, 14,
     60, 14) (:566-568), a 9 pt Label, hidden in Manual (:49-51, :104); the slot is 64 wide (:569); the right group is
     laid from the right and the Master Signal label is sized from its text (:606-630). The bar is 34 px high
     (R:src/MainComponent.cpp:2578); the layout tests use 40 (R:tests/test_master_signal_link.cpp:187, :300, :340,
     :373; R:tests/test_topbar_link_toggle.cpp:54, :103). Left run = 1,031 px, 1,095 with the Manual field (my own
     sum). 1728 is named as Boris's width only by a test comment (R:tests/test_master_signal_link.cpp:14-17).
     VERIFIED. A button at (right + 2, 2, 62, 13) covers rows 2-14; the tracker word starts at row 14: 1 px overlap.
     VERIFIED (arithmetic). The overlap is top-anchored, so it is the same at 34 and at 40.
 F20 The wheel: a 15 Hz timer (:289) copies six snapshot fields into displaySnap_ (:297-306); paint reads only that
     copy (:473-493, :525). VERIFIED.
 F21 bf7's delta re-labels the top bar's Quantize combo (.harmony/.reports/s-rta-1003/facts-quantize-units.md:118);
     board.md rows 2, 5 and 9 name the TopBar collision. VERIFIED.

Panel (SO-4, SO-5)
 F22 juce::CallOutBox::launchAsynchronously's holder calls enterModalState(true, ...) (JUCE:.../windows/
     juce_CallOutBox.cpp:66-73): modal, and it takes the keyboard focus. Esc dismisses (:188-197). A click outside:
     inputAttemptWhenModal leaves the modal state at once (:158-162) and Component::internalMouseDown then DELIVERS
     the click (JUCE:.../components/juce_Component.cpp:2175-2191); only a click on the launching area is swallowed
     (:142-157). VERIFIED. So the seat's "the first click outside only dismisses" is wrong; "modal" is right.
 F23 The keyboard launcher is MainComponent::keyPressed plus a KeyListener on itself (R:src/MainComponent.cpp:
     2308-2310, :3889, :4011-4016). VERIFIED. Whether keys still reach it while a modal child holds the focus would
     need a run; D16 makes the question moot.
 F24 The earlier plan's value box: "Enter applies (clamped), Esc reverts, anything else reverts"
     (P:plan-bf2.md:412-413). New / Rename venue use the AlertWindow + text editor idiom (:416). Boris: "can enter in
     the amount then click plus or minus to fine tune" (binding-decisions.md:578-579). VERIFIED.

Composition file (A1, A10, SO-9, SO-10)
 F25 Three places write the file, each by a direct composition_.saveToFile call: saveComposition()
     (R:src/MainComponent.cpp:3464-3469), saveCompositionTo() (:3490-3493; Save As and /api/debug/save_composition,
     :2146), Collect media (:6595). No other caller (grep). saveToFile = toVar() -> replaceWithText
     (R:src/model/Composition.h:1153-1157). VERIFIED.
 F26 finishStagedLoad serves Composition, DeckAppend and DeckDuplicate (R:src/MainComponent.cpp:3229, :3251, :3258,
     :3288). The Composition branch swaps the model and shows ONE notice built by LoadNotice::forLoad(migrationNote,
     routineLoadNote) (:3264-3273; R:src/ui/LoadNotice.h:21-34). showLoadNotice holds one text and one tooltip and any
     later call replaces them (:3104-3112); a click hides it (:495); a save clears it (:3471, :3500). GET
     /api/debug/ui_text returns the load notice (R:src/api/ApiServer.cpp:2061-2075). VERIFIED.
 F27 Controller: setMs, nudge, selectVenue, createVenue, renameVenue, removeVenue, Listener, 500 ms save delay
     (W:src/sync/SyncOffsetController.h:36-42, :63-68). REST: /api/sync, /api/sync/set, /api/sync/nudge,
     /api/sync/venue, /venue/rename, /venue/remove (W:src/api/ApiServer.cpp:346-351). VERIFIED.

## 2 ATTACK RULINGS

Verdicts: ACCEPT = the claim holds and the change is adopted in substance. PARTIAL = part of the claim or of the
change is declined (named). REJECT = the claim is false or overtaken. "->" names the amendment.

| id | sev | verdict | the line that decides it | -> |
|---|---|---|---|---|
| T1 | MUST | ACCEPT | F3 + F4: the load timer does not cover the delay line, and at tempo 0 BeatLead returns at once; the mic-only driver sets no tempo. The tripwire cannot trip at either sign. | D2 |
| T2 | SHOULD | ACCEPT | F10: a correct app's markers already reach -448 of a -512 bar, and check (i) itself allows the gesture to be 2,880 samples after its marker, so (ii)'s upper bound of 2,880 fails a run that (i) passes. | D4 |
| T3 | SHOULD | PARTIAL | F8: "9 in 10" holds only at zero bias; at the plan's own expected bias it is 0.21-0.55. Sentence struck, SE formula replaced. DECLINED: the 0-vs-0 control (its expectation is 0 by symmetry; it measures noise, which the arm-level SE already does). | D3 |
| T4 | SHOULD | ACCEPT | F1: both answers are "yes", dated 80 s after the plan. | D1 |
| T5 | SHOULD | ACCEPT | F1 + F16: he said yes to a wheel that "stays where he tapped"; a wheel that waits D ms is not that. A Tap's effect on the wheel is simple enough to show at the press (phase to 0, same segment). | D12 |
| T6 | SHOULD | PARTIAL | F13: the burst happens whenever a take is replayed WITH AUDIO at a larger dial than it was recorded at -- the normal cross-room case, not a rare one. Wording and a unit case adopted. DECLINED: the routine claim (routines never use that edge) and choice (B). | D7 |
| T7 | SHOULD | ACCEPT | F15: at a capped re-base the fold leaps +(4 - step) beats. Ruled between cap and Appendix A on that basis: Appendix A. | D5 |
| T8 | SHOULD | PARTIAL | F18: no serialization, margins hidden unless it fails. RUN_SERIAL, printed margins, 3 consecutive quiet passes adopted. DECLINED as a block: "within 0.1 of a bound" is recorded as a finding; the bar itself is not moved. | D8 |
| T9 | NIT | ACCEPT | F11: the code is already rate-aware; the plan's fixed sample counts are not. | D3, D4 |
| SO-1 | MUST | REJECT | F1: overtaken. Boris answered the exact question "yes" at 14:03:17. F2: his own paragraph in that message says the tapped beat is the music and "The visuals should be on the delay." -- the specific sentence outranks the blanket one. KEPT from the seat: the consequence is stated to him in plain words (D1). | D1 |
| SO-2 | SHOULD | ACCEPT | F19: one removeFromLeft chain; +70 on Gain is +70 on every control after it. Disclosed and recorded. | D19 |
| SO-3 | SHOULD | ACCEPT | F19: the right group's wanted width rests on a measured label; an all-or-nothing rule turns a 1 px shortfall into no change at all. | D19 |
| SO-4 | SHOULD | PARTIAL | F22: modal and focus-taking is VERIFIED; "a click outside is swallowed" is wrong in this JUCE. The remedy is adopted anyway: a non-modal child is correct by construction, a CallOutBox needs a run to know (F23). | D16 |
| SO-5 | SHOULD | ACCEPT | F24: "anything else reverts" loses his typed number when he clicks +, the flow he described. | D17 |
| SO-6 | SHOULD | PARTIAL | F19: 1 px overlap VERIFIED; a 62 x 13 only-door is thin. The whole slot becomes the hit area and that is a numeric bar. DECLINED: 11 pt (does not fit a 12 px row) and the contrast claim (kTextSecondary is the bar's own caption colour); both stay with the critic panel. | D18 |
| SO-7 | SHOULD | ACCEPT | Same defect as T5; the anchor at the press IS the acknowledgement. | D12 |
| SO-8 | SHOULD | PARTIAL | F20: the gate read a fresh copy, not the painted one. Painted values gated; the limits sentence corrected (a hold freezes the wheel). DECLINED: a raw-beat snapshot field (EARLY + Auto is the case he calls rare; disclosed instead) and an Auto arm (the wheel code has no mode branch). | D12 |
| SO-9 | SHOULD | PARTIAL | F26: one shared notice slot. Adoption on a composition open only, one composed notice with the sync sentence first, and a lasting "was" line adopted. DECLINED: the venue name on the button (62 px) and stopping the live adoption (Q10 "yes"). | D14, D15 |
| SO-10 | SHOULD | PARTIAL | Q10 "yes": the file's value is what loads. A store-wins rule would break his own use ("if they come back to that venue, they have the settings already") for anyone who tunes the "Default" venue. KEPT from the seat: the replaced number stays reachable after the notice is gone. | D15 |
| A1 | MUST | ACCEPT | F25: three writers, the plan stamps one. | D13 |
| A2 | MUST | ACCEPT | F14 + S6: a queued Tap is applied up to 500 ms later; the start wait gives up at 250 ms. | D9 |
| A3 | MUST | ACCEPT | F5: neither field exists under the plan's names; the env variable is TEST-SERVER only. | D2 |
| A4 | MUST | ACCEPT | Plan :425-428 has no pointer, dialog or Gain verb; C6, C10, C15 are unreachable. The "taste-only" waiver stays for critic opinions only. | D20 |
| A5 | MUST | ACCEPT | F20: the dump and the paint read different copies; a top bar that never copies the offset passes R12 as written. DECLINED detail: a PNG of the wheel (a still shows no timing). | D12 |
| A6 | SHOULD | ACCEPT | F19 + F21: 1,031 is a sum of literals another lane is about to change. | D19 |
| A7 | SHOULD | ACCEPT | F19: rows 2-14 against 14-28. (Its "40 hides it" point does not hold, F19; new tests use 34 anyway.) | D18 |
| A8 | SHOULD | ACCEPT | F12: the verdict needs only applied > 0. | D6 |
| A9 | SHOULD | ACCEPT | F8: the plan's SE formula is low by about 5 % here; (i) is true on any tree with one clock and has no live RED. | D3, D4 |
| A10 | SHOULD | PARTIAL | F26: adoption must sit inside the Composition branch; deck-load arm and a real old file adopted. DECLINED: "always write the key" -- every show saved at home would then reset the room's dial to 0 when opened at the venue. The limit is documented and asked (Q2). | D14 |

Reconciliations between seats
 - SO-1 vs T4: decided by F1 (a dated fact), for T4.
 - T1 vs A3 (drive G6 better vs cut its cpu half): both are met by measuring inside the existing probe run (D2).
 - T5 vs SO-7 (pending tap on the wheel vs a flash at the press): one mechanism, the anchor (D12).
 - SO-6 vs A7 (bigger target vs fix 1 px): the whole slot is the target and the text row is 12 px (D18).
 - SO-3 vs A6 (graded rule vs computed rule): one function, graded and computed from the layout (D19).
 - T3 vs A9 on the SE: same defect; the group-mean form answers both (D3).

## 3 AMENDMENTS (each OVERRIDES plan-bf2-delta.md, and through it ruling-bf2.md, where they differ)

D1  (T4; SO-1 overtaken) Boris has answered -- nothing in this lane is conditional on him any more
  - Plan questions 8.1 and 8.2 are CLOSED by F1. Deleted from the plan: :319-320 ("put to Boris ... If he answers
    no"), :464 ("question 8.2 offers the other behaviour"), :490 ("Skipped if Boris answers 8.1 no"), :492 ("The
    wheel part is skipped with S6"), R9's bracket at :526, :622-628, and risk K1. S6, the music-beat wheel and
    "a composition open adopts its saved sync" are unconditional.
  - Plan question 8.3 is closed too: "Sync lives in top bar" answers page question 14, whose other choice was "a
    small button in the top bar that opens it" (P:ruling-bf2.md:524-525). It becomes Boris check 6.8; the plan's T2
    (the number always visible) stays the named follow-up if he finds the button too small.
  - The consequence is told to him on the page (a disclosure, not a question): "With Sync at +100, a Tap or Resync
    reaches the picture 100 ms after you press. The circle shows it at once."

D2  (T1, A3) G6 is measured inside the existing probe run; no perf script
  - Plan B4 (iii)'s driver (:268-277) and its tripwire are deleted. .harmony/probe-sync-perf.sh is NOT written.
  - W:src/analysis/SyncWitness.h: the per-hop entry gains `float pipelineUs`; AnalysisThread::processHop stores the
    elapsedUs it already computes (F7); GET /api/debug/sync_witness returns it. TEST-SERVER builds only; a production
    build is byte-for-byte unaffected.
  - The gate is G6 in section 5: a FIXED budget bar (the documented "< 2 ms per hop") over windows the probe already
    holds, with the EARLY path proven engaged (leadApplied on the -500 window's hops). The A/B difference is INFO,
    with the ruled 0.3-point figure printed beside it; -500 is compared with a 0 step in the same tempo mode.
  - What guards LATE: R1a (ii) and (iii), which already cover R1's 500 arm, plus R4's settled -500 window (plan
    :278-280, kept). Stated plainly: G6 is not a gate for the delay line; R1a is.
  - Why not the ruled A/B as a gate. Harmony constraint (plan :263-265): perf verdicts only on a quiet machine,
    arms interleaved, >= 5 runs per arm, and a bar whose teeth equal the drift is INFO, not a gate. The ruled bar
    max(0.3 point, 2 x SD) grows with the drift.

D3  (T3, A9 (1), T9) R7: the statistic, its SE, and what it resolves
  - Arms [0, 100] x 10 + [0], one launch, 60 s takes (plan :158, kept).
  - Per arm: drop a marker farther than 21.333 ms x the take's segment rate from the arm's median (1,024 samples at
    48 kHz), count the drops, take the MEAN of the rest.
  - dbar = mean of the ten 100-arm values - mean of the eleven 0-arm values. SE = sqrt(sH^2 / 10 + sZ^2 / 11), with
    sH and sZ the sample SD of the arm values in each group. (Replaces d_i-based SE: neighbouring d_i share a 0 arm.
    With 0 arms at both ends this form also cancels a linear drift. The d_i stay as INFO.)
  - The bar stays 1.0 ms. Harmony constraint: a pre-registered bar is never loosened to make a run pass. The plan's
    runner-up (2.0 ms on 5 arms) is not taken.
  - Struck from the plan: "it fails about 9 times in 10" (:167) and "dbar is expected slightly negative" (:173-174).
    Replacement text: "With an SE near 0.20 ms (F8: within-arm noise of one saved run), an offset of 2.67 ms or more
    always fails; 1.33 ms fails between 2 times in 10 and 9 in 10 depending on a bias of -0.5 to 0. Apart from a
    1-sample rounding (pinned exactly by the unit cases), the smallest error the implementation can make is one
    120 Hz tick, 8.33 ms (the plan's INFERRED list, :168-172). The S2 behaviour reads +99 to +100 ms."
  - Validity and the one extension: section 5, R7. The trim, the 0.33 ms SE limit and the extension rule are fixed
    here, before any 21-arm run exists.
  - INFO lines add the device block size and rate (GET /api/debug/audio_devices) next to the computed lattice.
  - RED: as plan :182-185 (arithmetic on the saved S2 numbers; the mutant app).

D4  (T2, A9 (2), T9) R7b: one live check with a RED; the invariant becomes INFO
  - (i) "gesture stamp - marker k stamp in [0, 60 ms]" is printed as INFO, no verdict. It is true on any tree with
    one stamp clock and has no live RED. Its teeth stay in the unit case "markers + gesture + tempo anchor in one
    domain" with mutant M3 (plan :219-221).
  - (ii) is the live row. B = the median paired marker error over ALL 0 arms of the run. BAR: the MEDIAN of the three
    (g_k - B) lies in [-32 ms, +80 ms] x the take's segment rate (-1,536 .. +3,840 samples at 48 kHz). All three are
    printed.
  - Where the numbers come from (F10): a correct app's markers spread -448 .. +512 samples around B, and (i) allows
    the gesture up to 2,880 samples after its marker, so a correct run can reach 3,392. The S2 behaviour starts at
    4,800 - 448 = 4,352. The bar sits between them with 448 and 512 samples to spare; the low side has 1,088 to
    spare. The median of three keeps the one 60 ms-late marker each take holds (F8) from failing a correct app; the
    mutant puts all three outside.
  - (ii) resolves an error of about 40 ms or more. R7 carries the 1 ms claim.

D5  (T7) The origin follows the view: Appendix A's rule 2 as written; the cap is removed
  - W:src/analysis/BeatLead.cpp: in the non-Resync branch of a re-base the origin excess grows by the absorbed bars
    and nothing else (the cap at :206-209 goes). From then on originX == barX on every hop.
  - Replaced text. ruling-bf2 AM-2 (v) and Appendix A's "barsSinceResync() never decreases except at a Resync epoch"
    (P:plan-bf2.md:702-703, :707) become: "(v) on every hop BeatLead ran, published resyncBarOrigin - the tracker's
    resyncBarOrigin == the published bar excess, so barsSinceResync() is always the led view's own count; on a hop
    that is neither a re-base nor a Resync it never decreases." totalBeatCount and totalBarCount still never
    decrease, anywhere.
  - Tests (tests/test_beat_lead.cpp): T-L3 (v) and T-L6's last check (:681) re-stated to the text above. NEW T-L6b
    "a Tap that un-crosses a view bar line": the F15 scenario; asserts the published fold step equals the tracker's
    own step (-0.3 +- 1e-5 beat), barsSinceResync() dropped by exactly 1, totalBarCount unchanged, and once the view
    crosses the line again barsSinceResync() is back to K.
  - RED first, on the tree as built: T-L6b reads a step near +3.7. STOP RULE: if it does not, F15 is wrong -- stop,
    leave the cap, report to the architect.
  - Why Appendix A wins. (1) The cap does not keep anything from running backwards: beatInBar and beatPhase step
    back either way. It holds only the bar term, which turns the tracker's own 0.3-beat step into a 3.7-beat leap
    for every multi-bar shape. (2) It leaves those shapes one bar ahead of the bar 1 he set with Resync, after about
    3 taps in 100 at an EARLY dial (INFERRED estimate); at D = 0 a Tap never moves the bar. (3) Appendix A re-fires
    one beat start, as the D = 0 app does at a Tap in the first half of a beat; the lead decides which beat that is,
    so it can be a bar's first. The plan's reason for the cap ("the alternative re-fires cycle-start events") is
    therefore true of D = 0 as well.
  - RUNNER-UP: keep the cap; disclosure "at such a Tap a 2 / 4 / 8-bar shape leaps forward by up to a bar and stays
    a bar ahead until Resync"; a unit bound 0 <= fold step <= 4 beats. It wins only by the STOP RULE, or if Harmony
    will not touch BeatLead before this merge.
  - Plan B4 (i)'s "Resync heals" assertion is no longer needed (there is no excess to heal).

D6  (A8) "Did not move back" cannot pass empty
  - R5 and T-L3 (iv) keep the builder's reading (plan :256-259) and add: the number of hops that met the antecedent is
    printed; R5 FAILS when it is below 5,000 or below half of the hops BeatLead ran. T-L3 prints its own count and
    asserts at least half of its non-re-base hops qualify; if the fixed-seed run has fewer, the builder reports the
    number and stops (it is not lowered).

D7  (T6) Replay end: (A) stands, described truthfully
  - Plan :193-197 and :204 are replaced by: "Any take replayed WITH AUDIO at a larger dial than it was recorded at
    (record at 0 at home, play at +150 in the room -- the normal case) plays the moves of its last (replay dial -
    record dial) ms together, at the instant the audio ends, then holds. Nothing is lost. Routines are not affected:
    they do not replay on the audio transport (F13)."
  - One more unit case in test_recorder_host [host][sync]: a take recorded at 0 with three points in its last 100 ms,
    replayed with audio at +300: at the end edge exactly those three fire, in stamp order, then finished; a second
    end tick fires nothing. If the existing tail case already asserts order and count, the builder cites it instead.
  - FU-1 ("the replay end holds for the dial amount") stays filed.

D8  (T8) [timing]: serialized, margins visible, three quiet passes
  - W:tests/CMakeLists.txt: the [timing] case is discovered on its own with PROPERTIES RUN_SERIAL TRUE; the general
    discovery excludes it (TEST_SPEC "~[tsan]~[timing]").
  - The case prints, on every run, each arm's mean and sd and each pair's mean(38) - mean(37) (today these are shown
    only on a failure).
  - Harmony's quiet-machine run is three consecutive passes (command in section 5, G1). A pass with a pair outside
    [0.8, 1.2] or an arm sd above 1.0 ms is still a pass and is recorded as a finding for the architect. The bars in
    the test are not changed.

D9  (A2) A take armed after a Tap waits for the dial
  - RecorderHost::startDue gives up after kStartWaitFallbackSeconds PLUS the late seconds of the tick's snapshot (the
    value tick() already computes, W:RecorderHost.cpp:456), not after 0.25 s flat.
  - RED-first unit case, test_recorder_host [host][sync] "take start waits for a Tap in the dial": snapshots at +400
    carrying the old request sequence until wall 0.40 s, the new one from 0.41 s; Record at 0.05 s -> the clock
    starts on the 0.41 s tick with that snapshot's anchor. On the unchanged constant it starts at 0.30 s from the
    stale snapshot.

D10 (side finding SF1) R9's bar counts from the right moment
  - The POST returns before the request is stamped (F17), so "D +- 15 ms after the POST returned" would fail a
    correct app on its high side. The as-ruled row already allowed 50 ms for that hand-over. New bar at +100 and
    +253: [D - 15, D + 50] ms after the POST returned. It still fails on the tree before S6 (which reads <= 50 ms).

D11 (side finding SF2) S6: a tempo typed after a pending Tap wins the tempo
  - Value-only requests stay immediate (plan :328). A queued Tap therefore applies its REALIGN always, and its TEMPO
    only when no value-only tempo request posted after it has already been applied. Without this, at +300 a Tap
    followed 100 ms later by POST /api/set_bpm ends on the Tap's tempo -- the older request.
  - One more [dial-tap] unit case in test_bpm_stabilization: Tap 120 at t, set tempo 128 at t + 100 ms, dial +300 ->
    after t + 300 ms the tempo is 128 and the beat was realigned at t + 300 ms. RED on a queue that applies the
    Tap's tempo unconditionally.
  - The rest of plan B6 (iii)(a) stands, including the call-site audit as a STOP condition (F17 says it will pass).

D12 (T5, SO-7, SO-8, A5) The wheel: one painted state, the music beat, a Tap shown at the press, and a gate on it
  - R:src/ui/TopBarModel.h, pure functions:
      `struct WheelBeat { int beatInBar; float beatPhase; uint32_t barCount; };`
      `WheelBeat musicBeat(float beatInBar, float beatPhase, uint32_t barCount, float bpm, float syncOffsetMs) noexcept`
        (plan (b), unchanged: identity when bpm <= 0 or the offset is 0)
      `WheelBeat afterRealign(WheelBeat atPress, bool resync) noexcept`
        (Resync -> beat 0, phase 0, bar 0. Tap -> same beat, phase 0, same bar. Mirrors F16.)
      `WheelBeat advance(WheelBeat from, float bpm, double ms) noexcept`
  - TopBar keeps ONE `wheel_` (a WheelBeat, its source, the tick's clock value). timerCallback computes it; the wheel
    and "Bar n" paint from it and from nothing else.
      `void noteHumanBeatAnchor(uint32_t requestSeq, double pressMs, float bpm, bool resync)` (message thread)
    MainComponent::applyTempoCommand calls it for a HUMAN "tap" or "resync" when the applied offset is above 0, right
    after the post: requestSeq = the tracker's posted sequence, pressMs = the stamp the queued request carries, bpm =
    BPMTracker::foldBPMToRange(tapped tempo) for a Tap, 0 (= follow the snapshot's tempo) for a Resync.
    The anchor's starting beat = afterRealign(what the wheel shows at the press, resync).
    Each tick: while an anchor is live, the snapshot's trackerRequestSeq is still behind it (signed difference) and
    now < pressMs + late + 250 ms -> wheel_ = advance(anchor, tempo, now - pressMs), source "pending". Otherwise
    wheel_ = musicBeat(snapshot), source "snapshot", and the anchor is dropped.
    At a dial of 0 or EARLY no anchor is ever armed; at 0 the painted values are the snapshot's, bit for bit.
    No new timer, no wider repaint (Pitfalls 57, 59); no new snapshot field (Rule 8 untouched).
  - Known limits (replace plan :360-361), all three on the Boris page:
      (1) About 1 Tap in 20 at a LATE dial lands within a hop of a beat boundary; the lit segment can then step once
          when the tracker's own answer arrives D ms later (INFERRED).
      (2) With the dial EARLIER and BPM on Auto, the circle pauses with the picture while the tracker corrects itself
          (the ruled hold: usually a few ms, up to one beat). The wheel only has the led fields.
      (3) At an EARLY tempo step the circle is off by the part of the lead still gliding, for a few beats.
  - RED-first tests. tests/test_topbar_model.cpp: the plan's musicBeat cases plus afterRealign and advance.
    NEW tests/test_topbar_wheel.cpp, a real TopBar on a FeatureBus, through `void tickForTest(double nowMs)` and
    `wheelForTest()`: (i) a snapshot at +100 ms, 120 BPM -> the painted beat is the snapshot's + 0.2 (RED on a
    timerCallback that copies the raw fields -- the seat's mutant); (ii) an anchor for request 6 while the snapshot
    carries 5 -> the tick 50 ms after the press shows phase 0.1 from the anchor; (iii) the snapshot reaches 6 ->
    source "snapshot", position continuous within 0.03 beat; (iv) the anchor expires 250 ms after press + late;
    (v) offset 0 -> painted == snapshot, bit for bit; (vi) two anchors: the later one shows.
  - The dump: GET /api/debug/sync_ui_dump "wheel" = {beatInBar, beatPhase, bar, source, tickMs} exactly as last
    computed for paint, plus the app's nowMs, both on the witness's clock (the one processMs uses). Plan :428-429's
    "fresh bus read" is deleted.
  - Gate: section 5, R12. DECLINED: a wheel capture in G7 (a still picture shows no timing).

D13 (A1) One writer for the composition file
  - File key and model as plan :443-446, kept. present = !(venue is "Default" and ms == 0), kept (see D14).
  - NEW `bool MainComponent::writeCompositionFile(const juce::File&)`: copies the controller's venue and value into
    the model, then composition_.saveToFile. saveComposition(), saveCompositionTo() and Collect media call it.
    Nothing else calls composition_.saveToFile.
  - NEW tests/test_save_path_lint.cpp [lint]: src/MainComponent.cpp holds `composition_.saveToFile(` exactly once.
    RED today (three).
  - TEST-ONLY: POST /api/debug/save_composition accepts {"plain": true} and then runs saveComposition() -- the Cmd+S
    path -- on the file path already set.

D14 (A10, SO-9) Loading: a composition open only; one notice; a real old file
  - The adopt hook runs ONLY inside the Kind::Composition branch of finishStagedLoad, after the model swap and before
    the notice is built (F26). Load Deck and Duplicate Deck never read the file's sync.
  - LoadNotice::forLoad takes a third argument, the sync sentence. When it is not empty it LEADS the visible text
    and the other notes go to the tooltip: one notice per load, never a second showLoadNotice call. A unit test
    covers the eight combinations of the three notes.
  - The sentence, only when the live venue or value actually changed: "Sync: Warehouse +55 -> Warehouse +42 (saved in
    this composition)".
  - "Default at 0 writes no key" stays. Always writing the key would make every show saved at home reset the room's
    dial to 0 when opened at the venue. The limit is written in docs/claude/integration.md and asked as Q2.
  - The old-file fixture is a real one: tests/fixtures/composition_pre_sync.json, saved through
    /api/debug/save_composition by the merged tree BEFORE any S5a change. test_composition asserts it holds no "sync"
    property and loads with present == false; the staged-load test and R11 (b) use it.

D15 (SO-10) The file's value loads; the value it replaced stays reachable
  - "The file wins" stands (Q10 "yes"; plan :453-455).
  - NEW in SyncOffsetController: adoptFromComposition remembers what the dial was whenever it changes the venue or
    the number: `std::optional<Replaced> replacedByLoad() const` ({venue, ms}). The next setMs, nudge, selectVenue,
    createVenue, renameVenue or removeVenue clears it. In memory only, never saved.
  - Shown in three places: the load notice (D14); the panel caption, "Visuals 42 ms later -- was +55 before this
    composition was opened"; the SYNC button's tooltip.
  - Tests: test_sync_offset_controller (set by an adoption that changed something, not set by one that did not,
    cleared by each of the six operations); test_sync_panel (the caption).

D16 (SO-4) The panel is a plain child, not a CallOutBox
  - Plan :402-408 and K7 are replaced. NEW R:src/ui/SyncPopover.h/.cpp: the SyncPanel is added to MainComponent as an
    ordinary child, placed under the SYNC button and kept inside the window. No modal state. Opening it does NOT
    take the keyboard focus -- the keyboard keeps firing clips while it is open.
  - It closes on: a click on the SYNC button; a mouse-down anywhere outside the panel and the button (seen through a
    mouse listener on MainComponent; the click is not swallowed, it does what it normally does); Esc while the panel
    has the focus and no edit is open.
  - Only the value box can take the keyboard focus, and only when clicked. The panel's buttons, its bar and its
    venue button do not take it when clicked (the load notice label already works this way,
    R:src/MainComponent.cpp:493-494). On Enter, Esc or a commit the box hands the focus back to whatever had it
    before, else to MainComponent. The dump carries "modalComponents" (the count of modal components) and
    "focusOwner" (the focused component's name, empty when none). (The plan's "as bpmEditField_ hands it back today" has no code behind it
    that I could find in R:TopBar.cpp:96-153; the dump's focusOwner bar in R10 is the check.)
  - The venue menu keeps showMenuAsync + withParentComponent(top level). New / Rename venue keep the AlertWindow
    idiom (F24): modal for the seconds it takes to type a name, a setup action, existing app behaviour.
  - NEW tests/test_sync_popover.cpp (RED on a stub): opening twice makes one panel; an outside mouse-down closes it
    and reports "not consumed"; a mouse-down in the panel or in its own venue menu does not close it; not modal while
    open; a click on the button closes it.

D17 (SO-5) The number box keeps what he typed
  - Plan :417-418 "Esc or anything else reverts" is replaced: Enter commits (clamped). Losing the focus commits when
    the text parses and reverts when it does not. Esc reverts. A press of - or + first commits a pending edit, then
    steps.
  - test_sync_panel adds: text "+50" pending, then + -> 51; pending "abc", then + -> old value + 1; focus lost with
    "7" -> 7; Esc -> unchanged. The test route gains {"typeNoEnter": "<text>"}, {"blur": true}, {"key": "escape"}.

D18 (SO-6, A7) The SYNC button: the whole slot is the target
  - Bounds (tempoLabel_.getRight() + 2, 2, 62, 26). The text "SYNC +42" is drawn in the top 62 x 12 rows, 9 pt bold.
    Hover = the house hover over the whole 62 x 26.
  - The tracker-state label keeps its bounds (rows 14-28), is drawn in front of the button and lets mouse clicks
    through, so a click anywhere in the slot opens the panel.
  - tests/test_topbar_sync_button.cpp sets the bar 34 px high and asserts: bounds 62 x 26 inside the tempo slot; the
    text rows do not meet the BPM number or the tracker label; a click on the tracker word fires onSyncClicked;
    "SYNC -500" fits 62 px.
  - Plan K4's fallback (full height only in Manual mode) is superseded.

D19 (SO-2, SO-3, A6) Gain: graded, computed from the layout, the shift disclosed
  - `int TopBarModel::gainSliderWidth(int spare) noexcept` -> clamp(70 + spare, 70, 140).
    spare = (the width left after the left run is laid out with Gain at 70) - (the right group's wanted width) -
    (64 when the Manual field is not laid out, so toggling Manual never changes Gain).
    Both numbers come from the layout code itself: resized() lays the left run through one local function that takes
    the Gain width and returns what is left; the right group's wanted width is the sum of the widths it then uses.
    The literal 1,031 does not appear.
  - By my sum at 1728: spare about 111, Gain 140, in Auto and in Manual. INFERRED: the Master Signal label's width
    is measured only at run time.
  - Disclosed to Boris (check 6.8): at his window width every control from Play to Quantize sits further right by
    the amount Gain grew (70 px).
  - Tests: gainSliderWidth (spare -1 -> 70, 0 -> 70, 30 -> 100, 70 -> 140, 500 -> 140); a 1280 layout case asserting
    every existing widget's bounds equal a table recorded from the top bar before this stage (replaces G7's "bounds
    equal main's" dump comparison); a 1728 case in Auto and in Manual: Gain 140, the right group at its wanted
    widths.
  - Harmony constraint (for board.md): a later lane that changes a top-bar width re-runs G7's 1728 / 1280 measurable
    bars.

D20 (A4) G7: only states a driver can reach; opinions and measurements kept apart
  - Captures removed: C6 (hover), C10 (New / Rename dialogs: the existing AlertWindow idiom, not this lane's design),
    C15 (Gain at two values: its content is the width, which the dump gives).
  - C11 (a refusal in the caption) is reached by REST: POST /api/sync/venue with a duplicate name while the panel is
    open.
  - The interaction-logic list leaves the critic panel. Each item is a named unit test with a RED stub: hold-repeat
    of - / + (test_sync_panel), one panel instance and closing (test_sync_popover), the box's commit rules (D17), a
    REST change while the panel is open (R10), deleting the current venue and the glide (test_sync_offset_controller,
    built in S1b).
  - The critic panel judges pixels and the dump only. "Any FAIL blocks unless Harmony records why it is taste-only"
    applies to critic items. A measurable bar has no waiver.

D21 Stages, routes, order
  - GET /api/debug/sync_ui_dump is created in S5a with "wheel" and "replacedByLoad"; S5b adds the button, panel and
    Gain parts and POST /api/debug/sync_ui.
  - S3f and S4 touch neither the top bar nor the composition file. They may be built before M0 if main does not yet
    hold the deck change when the lane is free (board.md row 2 lists them in that order); M0 then re-runs G1, G2, G3
    on the merged tree. S6, S5a and S5b are built only after M0.
  - S6, S5a and S5b reach main together. S6 alone would ship a wheel that shows the delayed beat.

D22 Docs (plan section 6 stands, with these changes)
  - docs/claude/analysis.md "Sync offset": the origin follows the view (D5), not "capped"; taps go through the dial;
    a tempo typed after a pending Tap wins (D11).
  - docs/claude/recording.md: the replay end in D7's words; the take start wait (D9); the Tap / Resync stamp rule.
  - docs/claude/integration.md: the "sync" key, the single writer (D13), "Default at 0 writes no key", the load
    rules (D14), the replaced value (D15), the test routes.
  - docs/claude/performance-controls.md: the circle shows the music beat; a Tap shows at the press; the three limits.
  - CLAUDE.md: <= 350 bytes for this lane (one pitfall index line under the next free number at the merge, one UI
    Patterns line); the builder prints `wc -c CLAUDE.md`; above 25,000 is a STOP.

Unchanged from the plan (attacked or not): B1's interleaved 21-arm design and the mutant-app RED; B2 choice (A);
B3's choice to watch the recorder's own marker count; B4 (ii) the reading of "did not move back"; B5's commands; B6
(iii)(a) S6 as specified, with D9, D10, D11; B6 (i) the button-plus-panel layout, the panel's content and captions; B6
(ii) the key, the three adopt cases and "the file wins"; M0 as specified.

## 4 FINAL BUILD STAGES + ROAD TO MERGE

One builder context per stage; a reviewer pass after each; Harmony runs every live row, never a builder.

| key | scope | needs | exit gates |
|---|---|---|---|
| M0 | `git merge main` INTO lane/bf2 once the deck change is in main (no rebase). Hand-merges as plan :478-483. Pitfall NN -> the next free number. No behaviour change. | main holds the deck change | G1, G2, G3 |
| S3f | S3 finish + the S2 items: probe rows R7 (D3), R7b (D4), R5 count (D6), R1a over -500, G6 (D2: witness pipelineUs); the mutant-app RED; mutant M3; BeatLead origin (D5: T-L6b RED first, then the cap removed); replay-end unit case (D7); [timing] serialized with printed margins (D8); docs sentences. | may precede M0 (D21) | G1, G1-RED, G2, G3, G4 R1a, R4, R4b, R5, R7, R7b, G6 |
| S4 | Keys & MIDI: plan I9 as ruled; line references re-read on the tree it is built on. | may precede M0 | G1, G1-RED |
| S6 | Taps go through the dial: plan B6 (iii)(a) + D9 (take start) + D10 (R9 bar) + D11 (tempo order). Engine only. | M0, S3f | G1, G1-RED, G2, G3 (b), G4 R9, R3, R8 |
| S5a | Remembered with the composition + the wheel: the "sync" key, the single writer (D13), adopt on a composition open (D14), the replaced value (D15), the music-beat wheel with the press anchor and its dump (D12). No new visible widget. | M0, S6 | G1, G1-RED, R11, R12 |
| S5b | The control on screen: the SYNC button (D18), the non-modal panel (D16, D17), Gain (D19), the test routes, the captures. | S5a | G1, G1-RED, R10, G7 |

Reviews: after M0 a merge review (conflict hunks only). After S3f a gates review (can each row fail? RED evidence)
and a real-time read of the BeatLead diff. After S6 a real-time review (no allocation, no mutex, the sequence
contract, the recorder rules). After S5a a state review (every save path, every load kind, the adopt cases). After
S5b a UI correctness review, then G7's critic panel, then Harmony's gate list.

Then: `git merge main` once more if main moved; the FINAL gate list (section 5) on that tree; merge to main; the
Boris page (sections 6 and 7).

## 5 FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings ONLY from here)

Rig: as P:ruling-bf2.md:386-390, unchanged. Harmony constraint: an Audio-DNA the run did not start is Boris's --
never quit or touch it. Harmony constraint: a pre-registered bar is never loosened to make a run pass.

KEPT BY ID, text unchanged from P:ruling-bf2.md:392-482: G1-RED, G2, G3 (a) (b), G4 R0, R1, R2, R3, R4, R4b, R6, R8,
the DECISION RULE, G5. Every other row is given in full below and replaces the earlier text.

G1  ctest, default build (ADNA_SANITIZE empty): 0 failed, 0 skipped. Suites added to the ruling's list --
    S3f: test_beat_lead (T-L6b; T-L3 (v) and T-L6 re-stated; T-L3 (iv) qualifying count), test_recorder_host
    [host][sync] (the replay-end case), test_sync_witness (pipelineUs);
    S4: test_binding_sync_nudge;
    S6: test_bpm_stabilization [dial-tap] (incl. the tempo-order case), test_analysis_sync_thread [dial-tap],
    test_recorder_host [host][sync] (the two Tap-stamp cases and "take start waits for a Tap in the dial");
    S5a: test_composition (the sync key; composition_pre_sync.json), test_sync_offset_controller (adopt;
    replacedByLoad), test_topbar_model (musicBeat, afterRealign, advance), test_topbar_wheel, test_save_path_lint,
    the LoadNotice::forLoad combinations;
    S5b: test_sync_panel, test_sync_popover, test_topbar_sync_button, test_topbar_model (gainSliderWidth), the 1280
    and 1728 layout cases, the re-anchored top-bar layout tests.
    REMOVED from the ruling's list: test_timing_window_content, test_topbar_sync_indicator.
    [timing]: registered RUN_SERIAL. Harmony's quiet run (no compiler, no app, no commit in the last 2 minutes;
    `ps -Ao pcpu,comm -r | head -5` shows nothing above 20 %):
      ctest --test-dir /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf2/build-lane -R "\[timing\]" --repeat until-fail:3 -V
    Pass line: "100% tests passed, 0 tests failed out of 1" after three runs; the printed per-arm sd and per-pair
    differences are copied into the gate record. A [timing] failure: if the pre-run `ps` check recorded a process
    above 20 % CPU, stop it or wait it out and re-run once (the re-run stands); otherwise it blocks.
R1a (changed) as ruled (P:ruling-bf2.md:421-424), PLUS (ii) and (iii) evaluated over R4's settled -500 window.
R5  (changed; S2, re-run in S3f) EARLY, AUTO, click track: arms [0, -100, 0, -500, 0], 60 s each. Bars: R4's
    monotonicity, level-contract and CRC bars on every hop; onsets: |median lag shift vs the neighbouring 0 arms|
    <= 8 ms; holds, every hop: held => deficitBeats > 0 or a counter would decrease; not held => published fold ==
    target fold +- 1e-4 beat; on every non-re-base hop whose raw fold advanced >= aN - 1e-4 (a free-running hop) the
    target fold advanced >= 0.75 x aN - 1e-4, and the number of such hops is printed and is >= 5,000 and >= half of
    the hops BeatLead ran. INFO: held-hop fraction, longest held run (ms), number of runs.
    (Unit T-L3 (iv): the same with 1e-6; the count printed and >= half of its non-re-base hops.)
R7  (changed; S3f) take alignment. Click takes with onset markers (audio:true), 21 arms in one launch: [0, 100] x 10
    + [0], 60 s each. Per arm: paired marker errors (probe-step3's T2 math); a marker farther than 21.333 ms x the
    take's segment rate (1,024 samples at 48 kHz) from the arm's median is dropped and counted; the arm's value is
    the MEAN of the rest. dbar = mean of the ten 100-arm values - mean of the eleven 0-arm values. SE = sqrt(sH^2 /
    10 + sZ^2 / 11), sH and sZ the sample SD of the arm values in each group.
    VALID when every arm pairs >= 100 markers and drops <= 4.
    BAR: |dbar| <= 1.0 ms.
    ONE extension: when |dbar| is within 2 x SE of 1.0 (either side), or SE > 0.33 ms, the same 21 arms run once
    more (PROBESYNC_R7_EXTEND=1) and the pooled 42 arms decide by the same formulas. If the pooled SE is still above
    0.33 ms the row is INCONCLUSIVE: it blocks and goes to the architect. No other re-run except the DECISION RULE's
    `ps` case. A FAIL with |dbar| < 2.67 ms goes to the architect as a finding and is not re-run.
    RED on record: the S2 behaviour reads +99 to +100 ms (saved S2 run: medians 130.667 against 30.667). Live RED:
    the mutant app (a scratch copy with W:RecorderHost.cpp:456 lateSeconds = 0.0) run with PROBESYNC_R7_ARMS=0,100,0
    -> FAIL near +100; `grep -c MUTANT` on the real tree = 0 afterwards.
    INFO (no verdict): each arm's median, mean, trimmed mean and drops; each 100 arm minus the mean of its two
    neighbours; the lattice in samples; the device block size and rate.
R7b (changed; S3f) gestures land in show time. Inside R7's FIRST 100 arm; the probe fires POST /api/trigger_clip when
    the recorder's own marker count (GET /api/perf/status "markers") reaches k = 10, 20, 30. For each k: g_k =
    (gesture stamp - the take's first sample) - the click-grid position nearest marker k. B = the median paired marker
    error over ALL 0 arms of the run.
    BAR: 3 gestures fired and 3 found in the take; the MEDIAN of the three (g_k - B) lies in [-32 ms, +80 ms] x the
    take's segment rate (-1,536 .. +3,840 samples at 48 kHz). All three are printed.
    RED: the mutant app of R7 -> every g_k - B is above +4,300 samples.
    INFO (no verdict): gesture stamp - marker k stamp for each k (the earlier [0, 60 ms] window printed beside it).
R9  (changed; S6; BLOCKING) manual 120 BPM; POST /api/resync three times per value, >= 3 s apart; clocks aligned
    through the witness reply's nowMs. The first witness hop whose trackerRequestSeq exceeds its pre-POST value
    publishes the Resync (beatInBar 0, beatPhase 0, resyncBarOrigin == totalBarCount). At +100 and +253: its
    processMs lies in [D - 15, D + 50] ms after the POST returned. At 0 and at -100: <= 50 ms.
    RED: the lane tree before S6 reads <= 50 ms at +100 and +253.
R10 (changed; S5b; TEST mode, scratch settings) POST /api/debug/sync_ui and GET /api/debug/sync_ui_dump.
    {"open":true} -> the panel is open and inside the window, "modalComponents" is 0, "focusOwner" is what it was
    before; {"type":"+42"} (text + Enter) -> GET /api/sync targetMs 42 within 200 ms and the button reads "SYNC +42";
    {"click":"plus"} -> 43; {"click":"minus"} twice -> 41; {"typeNoEnter":"+50"} then {"click":"plus"} -> 51;
    {"typeNoEnter":"7"} then {"blur":true} -> 7; {"typeNoEnter":"9"} then {"key":"escape"} -> still 7;
    {"type":"abc"} -> still 7 and the box reads "+7 ms"; {"rightClick":"value"} -> 0 and the button reads "SYNC 0";
    POST /api/sync/set {"ms":-30} while the panel is open -> the box reads "-30 ms" within 200 ms (Pitfall 41);
    {"open":true} again -> still one panel; {"open":false} -> closed, "focusOwner" as before the first open.
R11 (new; S5a; TEST mode, scratch settings and scratch files; the load notice read through GET /api/debug/ui_text)
    (a) venue "Warehouse" at +42, POST /api/debug/save_composition {"path": P} -> the file's "sync" equals
        {"venue":"Warehouse","ms":42}.
    (a2) set +43, POST /api/debug/save_composition {"plain": true} -> the same file's "sync" has ms 43.
    (a3) on "Default" at 0, save -> the file has no "sync" key.
    (b) dial at "Warehouse" +42, load tests/fixtures/composition_pre_sync.json -> GET /api/sync unchanged (venue,
        targetMs); the load notice holds no "Sync:" sentence.
    (c) select "Default" 0, load the file of (a) -> venue "Warehouse", targetMs 42; the notice begins
        "Sync: Default 0 -> Warehouse +42".
    (d) set "Warehouse" to +55, load the file of (a) -> targetMs 42; the store's "Warehouse" is 42; the notice begins
        "Sync: Warehouse +55 -> Warehouse +42"; the dump's "replacedByLoad" equals {"venue":"Warehouse","ms":55};
        then POST /api/sync/nudge +1 -> "replacedByLoad" is gone.
    (e) a store without "Warehouse", load the file of (a) -> the venue is created at 42 and selected.
    (f) the real ~/Library/Audio-DNA/settings.json sha256 is unchanged around the run.
    (g) dial at "Warehouse" +55, POST /api/debug/load_deck with the file of (a), then POST /api/debug/duplicate_deck
        -> GET /api/sync unchanged after each.
R12 (new; S5a + S6; PRODUCTION mode, test-server build, manual 120 BPM). The dump's "wheel" is what the top bar last
    painted from, with that tick's clock value (D12).
    (a) settled, for D in {+200, -200, 0}: 20 dumps >= 100 ms apart. Each: (wheel position - the position published by
        the last witness hop processed at or before the wheel's tickMs) mod 4 beats = D x 120 / 60000 +- 0.05 beat
        (+0.40, -0.40, 0.00); source is "snapshot"; the dump's nowMs - tickMs <= 150 ms.
    (b) a press at +200: the probe waits until the wheel's phase is between 0.4 and 0.6, then POST /api/resync at T;
        dumps every 20 ms for 1.5 s. Every dump whose tickMs >= T + 80 ms: 4 x bar + beatInBar + beatPhase =
        (tickMs - T) x 2 / 1000 +- 0.10 beat. The published beat's Resync hop has processMs in [T + 185, T + 250].
    RED (unit, G1-RED): test_topbar_wheel (i) on a timerCallback that copies the raw fields, (ii) on a wheel with
    no anchor. What the live row reads on those stubs: (a) 0.00 instead of +-0.40; (b) the old beat for the first
    200 ms.
G6  (changed; S3f; inside the G4 probe run, quiet machine) per-hop analysis cost, from the witness's pipelineUs.
    Windows (each at least 150 hops): R1's settled 0 arms and its 500 arm; R4's settled -500 window and R4's settled
    0 step (both manual 120 BPM -- the baseline for -500 is the 0 step in the SAME tempo mode). Per window: mean,
    p99, max.
    BARS (block; fixed): (1) in the -500 window leadApplied is 1 on >= 99 % of the hops; (2) mean pipelineUs <= 2,000
    in each window (CLAUDE.md: the analysis budget is under 2 ms per hop).
    If a 0 window itself is above 2,000: bar (2) is replaced by mean(-500) <= mean(R4's 0 step) + 300 and
    mean(+500) <= mean(its two neighbouring R1 0 arms) + 300 (microseconds), the second valid only when those two 0
    arms differ by less than 100; otherwise INCONCLUSIVE -> repeat on a quiet machine. The over-budget baseline is
    reported to the architect either way.
    INFO: mean(-500) - mean(R4's 0 step) and mean(+500) - mean(R1's 0 arms) in microseconds and in DSP points
    (microseconds / 106.67), the earlier 0.3-point figure printed beside them; p99 and max; GET /api/status
    frameTimeMs once per window. The probe records the app-clock window of each R4 step for this.
    Not a gate for the delay line: R1a is.
G7  (changed; S5b) VISUAL WORK GATE, before Boris sees anything. TEST mode, scratch settings, states set by REST and
    POST /api/debug/sync_ui; the main window captured by Quartz window id at 1728 x 1000 and at 1280 x 720, cropped
    to the top bar (and to the panel when open); the dump read at each capture.
    States: C1 first launch, 1728: "SYNC 0" dim, Gain 140, Auto mode; C2 the same in Manual mode (BPM field shown,
    Gain still 140, Master Signal label whole); C3 +42 on "Warehouse": "SYNC +42" bright; C4 -30; C5 +500 and -500;
    C7 the panel open at 0 (caption "In step with the sound coming in"); C8 the panel at +42 and at -30 (bar filled
    from the centre, captions); C9 the venue menu open (3 venues, tick, Delete in warning red); C11 a refusal in the
    caption (POST /api/sync/venue with a duplicate name while the panel is open); C12 the panel open while REST
    changes the value; C13 just after loading a composition that changed the dial (button, load notice, the panel's
    "was" caption); C14 1280 x 720: Gain 70, the button present, the panel fully inside the window.
    (C6, C10 and C15 of the plan are removed, D20.)
    MEASURABLE BARS (dump; no waiver): at 1728 in Auto AND Manual every right-group widget has its wanted width and
    the Master Signal label is not cut; Gain is 140 at 1728 in Auto and in Manual and 70 at 1280; the SYNC button is
    62 x 26 inside the tempo slot; its text rows (62 x 12) meet neither the BPM number nor the tracker-state label;
    the tracker-state label's bounds equal the pre-stage ones; every text fits its bounds, "SYNC -500" included;
    panel widgets do not overlap and lie inside the panel; the panel lies inside the window at both sizes;
    "modalComponents" is 0 with the panel open; every state's text equals the model (dump against GET /api/sync),
    C12 and C13 included. INFO: Tap and Resync x at 1728 before and after the stage (expected +70).
    CRITIC PANEL in parallel (pixels and dump only), PASS / FAIL per item:
     visual-design: is "SYNC 0" clearly quieter than "SYNC +42" yet readable? does the word above the tracker state
      crowd the BPM number? is the doubled Gain balanced against its neighbours? palette colours only, no orange
      #ff4500, red only for Delete?
     UX: can a first-time user find the control? is the sign obvious without the caption? is one click to reach the
      number acceptable for a set-once control? does the load notice say what changed in plain words?
     graphic-design: grammar widths, 9 pt / 11 pt hierarchy, caption #888, panel alignment with the house inspector
      rows, no round control.
    A critic FAIL blocks unless Harmony records why it is taste-only. Then the page Boris opens (sections 6 and 7).

Stage -> gates: M0: G1, G2, G3. S3f: G1, G1-RED, G2, G3, G4 R1a, R4, R4b, R5, R7, R7b, G6. S4: G1, G1-RED.
    S6: G1, G1-RED, G2, G3 (b), G4 R9, R3, R8. S5a: G1, G1-RED, R11, R12. S5b: G1, G1-RED, R10, G7.
At the merge Harmony runs: G1, G2, G3, G4 (R0, R1, R1a, R2, R3, R4, R4b, R5, R6, R7, R7b, R8, R9, R12), G5, G6, R10,
    R11, G7 (already passed; re-shot only if S5b files changed since).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

 6.1 Tap along to a track with Sync at 0. -> The circle at the top pulses on your taps. Open SYNC and type +150. ->
     The circle stays exactly on the music; the picture's beat motion now lands a little after it. Wrong: the circle
     drifts off the music, or the picture does not move later.
 6.2 With Sync still at +150, tap the tempo again. -> The circle answers each tap at once, with no wait, and the
     picture stays that same bit behind. Wrong: the circle waits before it follows your tap; or after tapping the
     picture is suddenly back on the beat.
 6.3 Type -150. -> Beat-locked motion lands a little BEFORE the music; loudness-driven motion does not run early (it
     cannot). The circle stays on the music. With BPM on Auto the circle may pause for a moment, together with the
     picture, when the beat tracker corrects itself. Wrong: the circle runs ahead of the music.
 6.4 Record a short take with music at Sync 0; set Sync to +300 and replay it. -> Everything plays a touch late, as
     set; in the last third of a second the final moves arrive together as the music stops. Wrong: the replay never
     ends, or the whole take is out of step.
 6.5 At -300: press Resync on the one, start a 4-bar effect, then tap the tempo several times without pressing
     Resync. -> The 4-bar effect stays on the bar you set. Wrong: after tapping it sits on a different bar, or it
     jumps ahead when you tap.
 6.6 Click SYNC, make a venue "Test room", set +42, save the composition with Cmd+S, quit, reopen, load it. ->
     SYNC +42, venue "Test room". Set it to +55 without saving and load the composition again. -> It glides back to
     +42, a yellow note says it changed from +55, and the SYNC panel still says "was +55". Load an old composition.
     -> Sync does not move. Load a single deck from another show. -> Sync does not move.
 6.7 Gain: drag it in the part you use (below a quarter). -> Twice the room to move. Is that enough?
 6.8 Look at the top bar in your usual window size, Manual on and off. -> Nothing is cut off on the right. Play,
     Stop, the circle, Tap, Resync and the buttons after them sit a little further right than before (Gain took the
     room). SYNC is the small word above the tracker word next to the BPM; clicking anywhere in that little block
     opens it. Is it easy enough to hit?
 6.9 Open SYNC and leave it open. -> Your keyboard still fires clips. Click a clip. -> The panel closes and the clip
     fires. Wrong: keys do nothing while the panel is open, or the first click only closes the panel.
 6.10 In the SYNC box type 60 and click + without pressing Enter. -> 61. Wrong: your 60 is gone.

 Said on the page, no answer needed:
  - With Sync at +100, a Tap or Resync reaches the picture 100 ms after you press. The circle shows it at once.
  - About 1 tap in 20 at a later Sync, the lit quarter of the circle may step once a moment after the tap.
  - A take recorded at a smaller Sync than you replay it at plays its very last moves together as the music ends.
  - Opening a show switches Sync to the venue and number saved in it. The number it replaced stays written in the
    SYNC panel until you change Sync yourself.

## 7 BORIS QUESTIONS (each has a default; nothing waits)

 Q1 Gain: twice as long, same scale. DEFAULT: that. If you want more on top: stretch the low part so that 0 to 1
    takes half the slider.
 Q2 A show you save while Sync is on "Default" at 0 does not remember a Sync setting, so opening it later leaves Sync
    as it is in the room. (To make a show remember "this room needs 0", give the room a name first.) DEFAULT: yes.
    The other way: every show remembers its Sync, 0 included -- then opening a show you saved at home sets the
    room's Sync back to 0.

 Closed since the plan was written (not asked again): the plan's 8.1 and 8.2 (answered "yes", F1) and 8.3 ("Sync
 lives in top bar").

## 8 SIDE FINDINGS

 SF1 R9's "after the POST returned" started the clock too early: REST hands the Resync to the message thread and
     returns (F17). Fixed in D10. No seat raised it.
 SF2 S6's queue could apply an older Tap's tempo over a newer typed tempo. Fixed in D11. No seat raised it.
 SF3 Every arm of the saved run holds exactly one marker about 60 ms late (4,416 / 4,736 / 4,416 samples, F8): one
     per take, not noise. Cause not established. INFERRED candidate: the markers that pile up while the take's t = 0
     waits are stamped on the t = 0 tick (W:RecorderHost.cpp:521-525). The trim drops it. The S3f builder prints the
     index of each dropped marker; if it is not the take's first marker it is a finding.
 SF4 The cap came from a contradiction inside Appendix A itself (rule 2 against its own T-L3 line, F15). The builder
     flagged it correctly and stopped for a ruling.
 SF5 G6 has never been run at any stage (F6). The S1a and S2 merges went without it.
 SF6 The plan's own stage order (M0 first) and board.md row 2 (S3-finish and S4 first) disagree. D21 allows both.
 SF7 Plan E19's spare widths (167 / 103) differ from my sum (175 / 111 with a 78 px label). Immaterial under D19.
 SF8 Two seat slips, neither load-bearing: the timing seat's "run()" is serviceOnce; its probe line numbers are a few
     off. The stage-operator seat's claim that a click outside a CallOutBox is swallowed is wrong in JUCE 8.0.4 (F22).
     The gates seat's "a 40 px test hides the overlap" does not hold (F19).
 SF9 The first S3 run's saved JSON has no per-marker data (the field was added between runs). R7's verdict run must
     keep it; the probe does today (W:.harmony/probe-sync.py:906-908).
 SF10 Plan K10's audit will most likely pass: every caller is on the message thread (F17). It stays a STOP condition.
 SF11 The plan cites "as bpmEditField_ hands it back today" for the focus hand-back; R:src/ui/TopBar.cpp:96-153 shows
     no hand-back. D16 states the rule and R10 checks it.

## 9 RISKS OF THIS RULING (the strongest counterargument first)

 K1 (strongest) This ruling makes the lane bigger just before a merge Boris is waiting for. It re-opens built and
    gated S2 code (the origin), adds a press anchor to the wheel, builds a popover by hand instead of using JUCE's,
    and adds a memory to the controller. The plan's route -- accept the limits, disclose them, let Boris's checks
    decide -- ships sooner. Why it loses: each addition closes a contradiction with something already settled, not
    a matter of taste. The wheel "stays where he tapped" is his own "yes". A Tap never moves the bar at D = 0, so it
    must not at -300. A panel that is open must not take the keyboard from a performer. Disclosing a known
    contradiction to a non-technical user makes him the test. All four are small; three are off the real-time path;
    the fourth removes a four-line cap, RED first, with a STOP rule and a named runner-up. Cheapest refutation: T-L6b's
    RED run (minutes) -- if the as-built tree shows no leap, D5 falls back to the runner-up.
 K2 "The file wins" can still roll back tonight's tuning with an older save. D15 keeps the old number in sight; it
    does not prevent the roll-back. Refutation: Boris check 6.6.
 K3 The press anchor is a second source for the wheel for up to D ms. If its mirror of the tracker's rule is wrong
    the wheel jumps at the hand-over. Guards: test_topbar_wheel (iii) and R12 (b), which holds every dump through
    the hand-over to +-0.10 beat. Known residue: limit (1) of D12.
 K4 R7's expected SE (0.20 ms) comes from one saved run's within-arm noise. Drift between arms over 23 minutes is
    unknown. The 0.33 ms validity limit and the INCONCLUSIVE route are there for that; the first run's INFO lines
    show it.
 K5 G6's budget bar assumes the baseline is under 2 ms per hop on this machine. If it is not, the fallback bar is
    pre-stated; it catches only a gross regression.
 K6 R9's upper bound of D + 50 ms would hide a real lateness of up to about 35 ms in S6. The unit [dial-tap] case
    pins the exact call on a fake clock; R9 is the end-to-end check.
 K7 The popover's outside-click listener is hand-built. A missed case leaves the panel open (harmless, it is not
    modal) or closes it too eagerly (its own venue menu must count as inside; test_sync_popover has that case).
 K8 D11 changes which tempo wins in one ordering (Tap, then a typed tempo, inside the dial's delay). No flow that
    depends on the old order was found; if one exists it changes.
 K9 Q2's default means a room that needs exactly 0 must be given a name to be remembered. He may not expect that;
    it is why it is asked.
 K10 From the plan, still open: M0 was not dry-run (plan K8); the 1728 budget rests on a label measured only at run
    time (plan K6) -- under D19 a shortfall shows as a Gain below 140 and fails G7 visibly instead of silently.

FACTS HARMONY MUST MEASURE (a build, a run or a measurement decides; the ruling for each outcome)
 M1 Does the as-built tree leap in T-L6b? Test: the S3f builder's RED run of T-L6b before any BeatLead edit.
    Step near +3.7 beats -> D5 as written (cap removed). No leap -> STOP, the cap stays, runner-up of D5, report.
 M2 The baseline analysis cost. Test: mean pipelineUs in G6's 0 window (or, before any run, the profile dump total
    in a saved probe app log). <= 2,000 -> G6's bars as written. Above -> the pre-stated relative bar, and the
    over-budget baseline goes to the architect.
 M3 R7's real SE. Test: the first 21-arm run's SE line. <= 0.33 and |dbar| not within 2 x SE of 1.0 -> the verdict
    stands. Otherwise -> the one extension pass; pooled SE still above 0.33 -> INCONCLUSIVE, to the architect.
 M4 Gain at 1728. Test: the first S5b dump, before any panel work. 140 in Auto and Manual -> go on. Below 140 ->
    G7 fails; back to the architect (candidates: the Manual field into the tempo slot's lower half; a shorter gap).
 M5 The free-running hop count in R5. Test: the number R5 prints. >= 5,000 and >= half -> pass. Below -> the row
    fails; the number goes to the architect (the bar is not lowered by a builder).
 M6 [timing] margins. Test: three quiet passes with the printed sd and pair differences. All pass -> green; any
    pair outside [0.8, 1.2] or sd above 1.0 -> green, recorded as a finding.
 M7 Which marker is the one late marker per take (SF3). Test: the dropped marker's index in R7's INFO. The take's
    first -> noted, nothing to do. Any other -> a finding for the architect before the merge.

SUMMARY: 29 attacks ruled (19 ACCEPT, 9 PARTIAL, 1 REJECT); 22 amendments (D1-D22); stages M0, S3f, S4, S6, S5a, S5b;
2 Boris questions with defaults; 7 facts for Harmony to measure (M1-M7).

STATUS: DONE -- ruling-bf2-delta, 2026-10-03
