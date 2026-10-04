# PLAN bf2 STOPS -- the stop items left open after S3f / S4 / the fix round (P1..P7)
Architect position, s-rta-1003b, 2026-10-03. Lane read ONLY at 68abc16 (git objects). Nothing was built, run or
launched; python arithmetic was done on saved logs and on two saved takes of EARLIER builds (labelled where used).
Path shorthands: W: = the lane tree at 68abc16; SP: = <scratchpad> (/private/tmp/claude-501/-Users-boriskarpman-
projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad); RD: = .harmony/.reports/s-rta-1003/
ruling-bf2-delta.md. Labels: VERIFIED (read at the pin or computed from a saved file), INFERRED, ASSUMED.

## 1 GOAL
Rule the six open items of H-12 so the remaining stages (S6, S5a, S5b) can start. Answer in one paragraph:
- P1: MIDI learn does NOT bind a CC to a Sync target (keys and notes only); no dead binding can exist. Runner-up: a
  CC press edge = one step. One question for Boris (a knob for Sync? default no).
- P2: the saved logs point at the PROBE's assumption (the click file starts at the take's first sample), not at the
  take: the app arms the tap, writes a file, and only then starts the transport, so the file's first frame lands 0
  or 1 device blocks into the take. INFERRED, strong. It is settled by ONE probe-only experiment (measure the click
  grid's origin in the take's own audio); a decision table says what each outcome means. Nothing is fixed before it.
- P3: it is not a late marker. It is an EXTRA marker: the onset detector fires twice on click 69 of this click file
  (the second 5-7 blocks, 53-75 ms, after the first). VERIFIED by arithmetic on the saved runs and on two saved takes of earlier
  builds (the oldest of 2026-09-25): pre-existing, not the dial, not a save. One confirming arm rides in P2's run.
- P4: the low slope is a REGIME of a whole run (the 37 arms carry ~0.45 ms more wake overhead, the 38 arms ~0.22),
  not noise. The case stays a gate, quiet machine only, bar untouched; a 10-run experiment says what triggers it.
- P5: no route fires a binding action at the pin; one TEST-SERVER-only route + live row R13 drive the real handler.
- P6: remove the dead branch in this lane (it is this lane's orphan and no live gate has been run on it yet).
- P7: order = D0 (probe instrument) -> Harmony's diagnosis run RD -> S4b (keys fix, R13, BeatLead clean-up, R7
  re-stated) -> S6 -> S5a -> S5b. S6 does not start before RD's outcome is known.

## 2 ESTABLISHED FACTS (only what was read at 68abc16 or computed from a saved file)
Keys
 F1  `ccMode` appears only in W:src/binding/Binding.h (the field, default Absolute) and W:src/binding/
     BindingManager.cpp:173/177 (+ toVar / fromVar). The MIDI-learn CC capture, W:src/ui/MidiLearnOverlay.cpp:244-289,
     builds its Binding at :271-282 and never sets ccMode; before that it REMOVES every existing binding on that CC.
 F2  W:src/binding/BindingManager.cpp:168-177: for SyncNudge an Absolute CC returns false (nothing fires); a
     Relative CC hands `value - 64` as a raw tick count, no accumulator.
 F3  W:src/binding/Binding.h `syncNudgeDeltaMs`: MidiCC -> ticks x |step|; key / note -> step when value > 0.
 F4  The learn overlay's prompt while a target waits is the fixed string "Send a MIDI note or CC..." and the
     readout "Last: <message>" (W:src/ui/MidiLearnOverlay.cpp:93-103). Both exist before this lane.
 F5  W:src/MainComponent.cpp:7939-7944 is the handler case (2 lines); :2323 and :2442 are the REST / OSC nudge
     callbacks. No file under W:src/api calls processKeyDown / processMidi* / handleBindingAction (grep: only the
     constructor reference, ApiServer.h:402). TEST-SERVER-only routes that click things exist
     (W:src/api/ApiServer.cpp:343 /api/debug/tab_click; the block at :368-373 holds the sync debug routes).
The take start and the stamps
 F6  POST /api/perf/record with "audioFile" runs, on the message thread, in this order (W:src/MainComponent.cpp):
     :5910 AudioEngine::loadFile (stops the transport, position 0, new reader: W:src/audio/AudioEngine.cpp:52-73);
     :5946 RecorderHost::arm -> AudioTap::start, whose last act is `armed_.store(true)` (W:src/recording/
     AudioTap.cpp:146) -> then, still inside arm, publishStatus and the provisional take.json save
     (W:src/recording/RecorderHost.cpp:278-294, disk I/O); :5955 applyAudioTransport("play") ->
     transportSource_.start() (AudioEngine.cpp:75-78).
 F7  The tap takes its first sample on the first audio callback after the arm: `firstSample_ = deliveredBefore`
     (W:src/recording/AudioTap.cpp:179-183; the callback: W:src/audio/CombinedCallback.h:138, :165-167). So every
     callback that falls between F6's arm and F6's play puts one device block of pre-start audio at the head of
     the asset, and the file's frame 0 lands at asset frame N x block (N = 0, 1, ...). INFERRED from F6 + F7; the
     length of that window is not measured.
 F8  A marker's stamp is the delivered-sample counter of the message-thread tick that saw the onset count rise
     (RecorderHost.cpp:456-462, :651-661, :737-752). The probe's error is (stamp - firstSample) - the nearest
     multiple of the click interval, i.e. it ASSUMES the file's frame 0 is asset frame 0
     (W:.harmony/probe-sync.py:888-913, t2_errors).
 F9  Periodic take save: every 60.0 s of take time (W:src/recording/RecorderHost.h:280; RecorderHost.cpp:537-553);
     the early tempo save fires once, on the first metered tick. Neither is near 35 s of a 61 s take.
 F10 The analysis loop at D = 0 polls with sleep(1) when nothing is queued and processes a drained hop in the same
     pass (W:src/analysis/AnalysisThread.cpp:79-96, :124-160). The onset detector's minimum inter-onset interval
     is 50 ms (W:src/analysis/OnsetDetector.cpp:21). D9 edits RecorderHost::startDue (RD: D9), not F6's order.
Saved runs (SP:bf2-S3f/out-green, out-green2, out-red-r7, SP:bf2-R1/out-live; each probe-sync.json rows.R7.arms[k])
 F11 14 takes (zero and 100 arms). Every firstSample is a multiple of 512. In every zero arm the errors are a
     sawtooth of 8 values 64 samples apart: 1216..1664 (normal) or 1728..2176 (shifted), the SAME sequence
     ("first8" 1536,1600,1664,1216,... against 2048,2112,2176,1728,...): every marker + 512, constant inside a
     take. 5-12 markers per take sit one block above the main cluster in BOTH states (the tick's own jitter).
 F12 Which takes are shifted: out-green arm 2 only (arms 0, 1 normal); out-red-r7 (the mutant app) ALL THREE, from
     its first take (its 100 arm reads 6336..7296 = the normal 100-arm range 1024..1984 + 4800 + 512);
     out-green2 none of 5; out-live none of 3. 4 shifted of 14. Transitions seen: normal->normal 7,
     normal->shifted 1, shifted->shifted 2, shifted->normal 0. The one change inside a launch sits at a take
     boundary that is also a dial change (100 -> 0); out-green2 crosses the same dial change twice with no change;
     out-red-r7 is shifted before any dial change. Asset length (2927104 / 2927616 / 2928128 frames) does not
     follow the state.
 F13 The 60 s click file holds 120 clicks (/tmp/click_probesync_60s.wav = 11,520,044 bytes = 60 s; one click per
     24,000 frames, W:.harmony/gen-click-wav.py). Every take has 121 onset markers, 0 dupes, 0 off-grid. In every
     take the sawtooth runs idx 68, 69 on phase, idx 70 = the outlier (4416 / 4736 / 4928 / 4224 / 10048), and
     idx 71 carries the value idx 70 should have had: the sequence after idx 70 is ONE STEP BEHIND. So raw
     index 70 is a 121st marker between two clicks, not click 70's marker arriving late. In the zero arms it sits
     2560 or 3072 samples (5 or 6 blocks, 53 / 64 ms) after marker 69.
Saved takes of EARLIER builds (not the pin; ~/Documents/Audio-DNA/Takes + Audio; click onsets found in the asset WAV
at half full scale, each marker paired to the click before it)
 F14 step3gate1 (2026-10-02, 61.9 s): 124 clicks in the asset, 125 markers; the first click sits at asset frame 16
     (= the generator's threshold crossing of a click that starts at frame 0: origin 0); marker minus click =
     1198..1650 (= F11's normal state, minus those 16); exactly one click has two markers: click 69 at 34.5 s
     (+1329 and +4913).
 F15 step3long (2026-09-25, 1211.7 s): 2424 clicks, 2436 markers, no click without a marker; 12 clicks have two
     markers, the second 2560 to 3584 samples (5 to 7 blocks) after the first: clicks at 34.5, 163.0, 419.0, 634.5, 639.0, 710.5,
     754.5, 758.5, 822.5, 1003.0, 1023.0, 1150.5 s. Origin 0 again. No 60 s rhythm.
[timing]
 F16 The case (W:tests/test_analysis_sync_thread.cpp:553-690) reads lag = the poller's wall clock when it first
     sees a published hop minus the feeder's wall clock when that hop was pushed (both juce HiRes ms); the poller
     spins in 50 us yields (:525-530); bars: pair difference in [0.7, 1.3], per-arm sd <= 2.0.
 F17 Per-arm overhead (mean lag - D), saved logs. LOW runs (SP:bf2-M0/g1-LastTest.log:27116-27125, timing-run3.log,
     timing-run4.log, ctest-timing-rerun.log): 37 arms 1.50-1.63, 38 arms 1.20-1.36, 0 arms 1.16-1.22, 150 arms
     1.30-1.46; sd 0.49-0.61. NORMAL runs (SP:bf2-M0/timing-run5.log:116-125; the S3f run in the lane report,
     bf2-delta.md:593-596, Stremio at 27.2 %): 37 arms 1.07-1.15, 38 arms 1.05-1.14, 0 arms 0.96-1.01, 150 arms
     1.06-1.12; sd 0.37-0.48 (one 0 arm 0.67 beside a 10 ms poll gap). Inside a run the three 37 arms agree to
     0.1 ms and the three 38 arms to 0.14 ms.
BeatLead
 F18 `Flags::resyncs`, `lastResyncs_`, `originX_`, `Diag::originX`: W:src/analysis/BeatLead.cpp:156, :194-199, :202,
     :218, :220, :229; BeatLead.h:35, :63, :105, :108; the argument at W:src/analysis/AnalysisThread.cpp:457; tests
     W:tests/test_beat_lead.cpp:138, :247, :260, :266-267, :341, :691. SyncHopEntry carries beatX / barX, no
     originX (W:src/analysis/SyncWitness.h). BPMTracker::appliedResyncs() has no other src reader (grep).

## 3 ITEMS

### P1 THE DEAD KNOB
What each control sends, and what the engine's Relative decoding (F2: ticks = value - 64) makes of it:
| control | sends | as built | (a) learn = Relative | (b) no CC on a Sync target | (d) Absolute press edge |
|---|---|---|---|---|---|
| endless encoder, "64 +- n" coding | 65 / 63 per detent | bound, dead | +-1 ms per detent: right | cannot be bound | one step, then nothing until it turns back |
| endless encoder, two's-complement (1 / 127) or sign-bit coding | 1.. / 127.. | bound, dead | 1 -> -63 ms, 127 -> +63 ms: wrong way, at the clamp in 8 detents | cannot be bound | wrong |
| plain knob / fader | its position 0..127 on every move | bound, dead | every message adds position - 64: the dial is slammed to +-500 by one sweep | cannot be bound | +1 ms each time it crosses the middle upward |
| CC button | 127 on press, 0 on release | bound, dead | +63 then -64 ms | cannot be bound | one step per press: right |
| key, MIDI note / pad | press, release | works | works | works | works |
The app cannot tell these apart from the one message MIDI learn sees. (INFERRED from F2 / F3; the codings are
general MIDI practice, ASSUMED -- no controller of Boris's was read.)
FORKS
 (a) learn makes the binding Relative on a Sync target. Right for one coding of one control; for a knob, a fader or
     a CC button the first touch throws the venue's sync by tens of ms or to the clamp. The review's "costs nothing
     real" does not hold: S4 ignored the Absolute CC precisely so a touch cannot jump the dial. REJECTED.
 (b) keys and notes only; a CC is never bound to a Sync target. CHOSEN.
 (c) a general on-screen switch for a CC binding's mode. The only honest way to give encoders to every action, and
     a new control in the learn or shortcuts screen (design, captures, visual gate). NOT this lane. It is what a
     "yes" to the Boris question buys.
 (d) Absolute CC = a button (step on the rising edge through 64). RUNNER-UP: it makes CC buttons work and nothing
     jumps, but a knob then nudges once per crossing and an encoder works once and stalls: bound controls whose
     behaviour cannot be read off the label, plus new per-CC edge state, for a control nobody asked for.
Harmony constraint: no bound control that silently does nothing; no on-screen text that announces an event or a
failure. Under (b) no such binding can exist, and nothing is announced: the learn overlay STAYS in its waiting
state (the target keeps its highlight and shows no binding), and its prompt names what the target takes.
CHANGE (stage S4b)
 - W:src/binding/Binding.h: a pure `inline bool bindingIsLive(const Binding&) noexcept` -- false exactly for
   action SyncNudge + inputType MidiCC + ccMode != Relative.
 - W:src/binding/BindingManager.{h,cpp}: `bool learnMidiCC(const Binding& candidate)` -- if the candidate is not
   live: change NOTHING, return false; else remove the bindings on that CC (the loop now in the overlay), add,
   return true. addBinding refuses a non-live binding (returns 0); fromVar adds through the same check, so a
   bindings file's Absolute-CC Sync entry is not loaded. The Relative arm of F2 stays (ruled I9, unit-tested,
   reachable from a prepared file only); its `return false` for Absolute stays as the last guard.
 - W:src/ui/MidiLearnOverlay.cpp CC block: build the Binding, call learnMidiCC, and leave the waiting state only
   when it returned true. Prompt while waiting on a Sync target: "Send a MIDI note..." (the existing prompt minus
   "or CC"); every other target keeps "Send a MIDI note or CC...". The "Last: ..." readout is untouched.
 - Docs: W:docs/claude/performance-controls.md:41 and APP-INVENTORY: keys and notes nudge; MIDI learn does not bind
   a CC to a Sync target; a Relative CC entry works from a prepared bindings file only.
RED FIRST (W:tests/test_binding_sync_nudge.cpp, on stubs: bindingIsLive returning true, learnMidiCC = remove + add)
 T1 bindingIsLive: Absolute CC Sync false; Relative CC Sync, key Sync, note Sync, Absolute CC MasterOpacity true.
 T2 learnMidiCC refused: a manager holding CC 21 -> MasterOpacity; learn CC 21 onto Sync +1 returns false, the
    count is unchanged and CC 21 still drives MasterOpacity.
 T3 learnMidiCC accepted: CC 21 onto MasterOpacity replaces the old CC 21 binding (count 1, new target).
 T4 fromVar: a file with one Absolute-CC Sync entry and one key Sync entry loads 1; with "ccMode": 1 it loads 2
    and the encoder case of the existing end-to-end test still nudges.
 The existing "Absolute CC = no push" assertions are re-stated on T2's shape (the binding cannot be added).
MUTANTS: bindingIsLive always true -> T1, T2, T4 RED; learnMidiCC removing before the check -> T2 RED.
GATE: G1 (+ these cases); G7 gains the learn-overlay state of section 5.

### P2 R7'S ONE-BLOCK STATE AT DIAL 0
HYPOTHESES (all that F6..F15 allow)
 HA  PROBE. The file's frame 0 sits one device block into the asset on some takes (F7); the probe's grid assumes
     0 (F8). The take itself is consistent: markers and audio share one sample domain.
     FOR: the code order (F6); the shift is exactly one block with the lattice untouched (F11); two earlier takes
     show origin 0 and the normal state's numbers (F14); 4 of 14 = 0.29 is what a ~3 ms window in a 10.67 ms block
     gives (INFERRED). AGAINST: the shifted takes cluster (F12: 3 of 3 in one launch, never seen leaving the
     state) -- about 1 in 10 by chance over five launches at p = 0.29, so weak, but it is why this is not ruled
     by reading.
 HB  PRODUCT, analysis side: on some takes every hop is analysed one block later (everything audio-reactive is
     10.7 ms later in that state, with or without a take). AGAINST: F10 (nothing in the D = 0 loop holds a hop for
     a block; INFERRED by reading). A ring residue cannot give a uniform + 512 (INFERRED).
 HC  PRODUCT, stamping side: the message-thread tick reads the counter one block later for a whole take.
     AGAINST: the tick (120 Hz) and the block (93.75 Hz) drift against each other; that drift is F11's per-marker
     tail, present in both states; a whole-take version has no mechanism in the code read.
 HD  The transport's first block is silent because its read-ahead is not ready. AGAINST: that would LOSE click 0,
     not shift the grid; every take pairs 121 (ASSUMED JUCE behaviour, not in the repo).
THE EXPERIMENT (RD row, section 5; probe-only, NO app change, nothing to remove afterwards)
 E1  Before the probe deletes a take's asset it reads the asset WAV and the source click WAV, finds each click's
     onset in both (the first sample above half full scale; hits within 100 samples collapsed -- probe-step3's
     own detector) and prints per take: c0 = the median of (asset onset k - source onset k), its spread, c0 /
     block; the median error as today (e) and on the measured origin (e' = e - c0). It also prints, from the
     EXISTING witness (GET /api/debug/sync_witness: per-hop process time - drain stamp), the mean hop delay over
     the take.
     Arms: two launches of the lane app built at 68abc16; each: 8 takes at dial 0, 20 s each (new development knob
     PROBESYNC_R7_SECONDS; every line carries the SUBSET tag), then P3's two 60 s takes. About 5 minutes a launch.
     16 short takes: the chance of seeing no shifted take is 0.71^16 = 0.4 % if takes are independent.
 Readings: HA -> shifted takes have c0 = 512 (or 1024), normal takes c0 = 0, and e' has ONE median in all takes;
     hop delay the same in both. HB -> c0 = 0 everywhere, e' still two-valued, hop delay ~ +10.7 ms in the shifted
     takes. HC -> c0 = 0, e' two-valued, hop delay unchanged. HD -> a take with 120 markers and click 0 unpaired.
DECISION TABLE
 O1  c0 a multiple of the block in every take, e' one-valued (all take medians within 64 samples).
     INSTRUMENT ARTEFACT. R7 and R7b compute on the measured origin (strings in section 5); the bar and the
     validity rule stay and the validity rule GAINS a clause. No product change. One sentence in
     W:docs/claude/recording.md ("a take started with a file through REST holds up to one block before the file's
     first frame"). S6 may start.
 O2  c0 = 0 in every take and e' two-valued. PRODUCT DEFECT. S6 stays stopped. Hop delay + 10.7 ms -> the fix is
     in AnalysisThread::serviceOnce, pinned RED-first in test_analysis_sync_thread ("at D = 0 a hop is processed
     in the pass that drained it"). Hop delay unchanged -> the stamping side: one more instrument-and-run with a
     TEST-SERVER-only field per marker (the snapshot's sample count beside the tick's counter), then back to the
     architect with the numbers. R7 is not re-stated.
 O3  c0 varies AND e' is still two-valued: both; O1's re-statement and O2's route.
 O4  c0 not one value inside a take (spread > 4 samples) or not a multiple of the block: an asset anomaly. STOP,
     architect.
 O5  No shifted take in 16: not reproduced. One more pair of launches; if still none, the state is recorded as
     not reproducible on this tree, R7 is re-stated as O1 (the measured origin is right under every hypothesis)
     and its gate run prints c0 per arm so a recurrence is visible. S6 may start.
What Harmony's 21-arm run then needs: the re-stated row; nothing else. Its expected SE returns to RD: K4's 0.20 ms
(INFERRED: the one-block term was the only arm-to-arm step in the saved arms, F11).
NOT in this lane: making the REST record start sample-exact (starting the transport inside the arm). Under O1 it
changes nothing a take's reader can see.

### P3 THE "LATE" MARKER
VERIFIED (F13): raw index 70 is an EXTRA marker, the second of two onsets on click 69 (34.5 s), 53-64 ms after the
first. VERIFIED on earlier builds (F14, F15): the same click doubles in a take of 2026-10-02 and in a 20-minute take
of 2026-09-25, where 12 of 2424 clicks double at irregular times.
Hypotheses, each against that evidence:
 - a periodic or tempo take save stalling the stamping thread: REFUTED. A stall delays a stamp; it cannot add a
   marker; the save is at 60 s (F9); F15 shows no 60 s rhythm.
 - the click file is irregular there: the generator puts every onset on the grid (gen-click-wav.py) and the saved
   assets confirm it; what differs per click is its seeded noise.
 - a ring or resampler boundary: would follow app time, not the click's number; F14 / F15 / the 14 takes all hit
   click 69 from different app times. REFUTED (INFERRED).
 - the onset detector re-fires on some bursts just past its 50 ms minimum interval (F10): FITS everything.
   INFERRED until E3.
 E3 (rides in RD, no extra launch): take A = 60 s, dial 0, the usual file; take B = 60 s, dial 0, a file generated
   with --seed 1. The probe prints, per take, every click that carries two markers and the gap between them.
   Content-dependent (expected): A doubles click 69, B doubles other clicks or none. Time-dependent: B doubles
   near 34.5 s again -> architect, with a 120 s take as the next arm.
RULING: a measurement artefact of this click file meeting a property of the onset detector that is older than
this lane. R7's rule (drop beyond 21.333 ms, valid at <= 4 drops) already handles it and is not changed; the row
gains an INFO line naming doubled clicks. Does it matter to Boris: on this synthetic click about 1 hit in 200
raises two onsets 53-75 ms apart, so an onset-driven effect or a take marker doubles; on music the rate is unknown
(NOT measured). It is not one stamp late by a tenth of a second. Filed for an analysis lane (the detector's
minimum interval), not built here.

### P4 [timing] UNDER LOAD
What the case reads: F16. What can make the slope LOW rather than noisy: only an overhead term that differs
between the 37 and the 38 arms and repeats. F17 shows exactly that: in a low run every 37 arm carries ~1.55 ms and
every 38 arm ~1.28 ms; in a normal run both carry ~1.1 ms. It is a regime of the whole run, stable to about 0.1 ms
inside it, and it is NOT simply the video player's CPU share (the S3f run was normal at 27.2 %). Candidate
(INFERRED, not established): the analysis thread reaches a due hop through a chain of 1 ms sleeps
(AnalysisThread.cpp:85-96); when each sleep overshoots, the overshoot of the last one depends on how many came
before it, i.e. on D. Drift between arms is ruled out by F17 (it would alternate the pairs high / low).
THE RUN (TQ, section 5; no code change): 5 runs idle, then 5 runs with four plain shell busy loops, each run alone,
`ps` recorded before each; per run the ten arm lines and five pair lines D8 prints.
 - idle normal, loaded low with the 37 arms above the 38 arms every time: CPU contention triggers the regime; the
   case is honest only on a quiet machine. Gate as today.
 - idle normal, loaded normal: CPU share is not the trigger; record what else ran in M0's low runs (media
   playback, timers); gate as today; nothing more is built.
 - idle LOW at least once with nothing above 20 %: the regime is not load. STOP: the case as written cannot gate;
   architect, with the ten logs.
RULING: the case STAYS a gate and stays quiet-machine-only, exactly as RD: G1 states it; the bar 0.7 .. 1.3 and the
sd bar are untouched; no different measurement in this lane. The product meaning, if the candidate holds: under
load the applied delay is off by 0.2-0.45 ms depending on D -- below the dial's 1 ms step. Filed, not built.

### P5 THE UNTESTED HANDLER
VERIFIED (F5): no route reaches the handler. A TEST-SERVER-only route is needed.
 - W:src/api/ApiServer.{h,cpp}, inside the `#if AUDIODNA_TEST_SERVER` block at :368-373:
   POST /api/debug/binding_action, body {"action":"syncNudge","input":"key"|"note"|"cc","stepMs":<int>,
   "value":<number>,"ccMode":"relative"}; any other action -> 400. It builds a Binding VALUE (never added to the
   manager) and posts `onDebugBindingAction(binding, value)` to the message thread, as handleNudgeSync does (:2768).
 - W:src/MainComponent.cpp beside :2323: `onDebugBindingAction` = a call of the REAL handleBindingAction.
 The rig's ban is on synthetic OS key / MIDI events; an in-process test route is the house way (tab_click).
ROW R13 (section 5): press +1 -> +1; release -> no move; press -1 -> back; Relative CC 67 -> +3, 59 -> -5, 64 ->
no move; clamp at both ends. RED arm: a scratch mutant app (the handler's nudge called with the sign flipped) ->
the first step reads -1. A second mutant line (the nudge call removed) -> "still 0". Built in S4b; Harmony runs it.

### P6 THE DEAD BRANCH
RULED: REMOVE in this lane, in S4b. D5 made it dead, so it is this lane's orphan; no live gate of Harmony's has
run on BeatLead yet, so nothing is re-run because of it; S6 then builds on the clean file. Runner-up: keep as
debt -- loses because the next reader of BeatLead.cpp:194 must re-derive that both arms agree.
WHAT GOES (F18): the if / else at BeatLead.cpp:194-199 (the absorbed bars go into barX_ only); `originX_` (:202
reads barX_; :218 drops its term); `lastResyncs_` (:156, :220); `Flags::resyncs` (BeatLead.h:35) and the argument
at AnalysisThread.cpp:457; `Diag::originX` (BeatLead.h:63, BeatLead.cpp:229). Tests: test_beat_lead.cpp:138 goes;
:247, :266-267, :691 read d.barX; the checker's Resync test at :260 / :341 reads the tracker's appliedResyncs()
directly (that accessor stays: it is the checker's oracle).
PROOF behaviour is unchanged: test_beat_lead before and after prints the same lines -- "[T-L3] (iv) qualifying
(free-running) hops 50728 of 51161 non-re-base hops BeatLead ran (99.2 %)", the same T-L3 histogram line,
"[T-L6b] K 4; tracker step -0.296001 beat, published fold step -0.296001 beat; barsSinceResync 4 -> 3;
totalBarCount 4 -> 4" -- and passes. A removal has no RED of its own; its guard is a MUTANT: BeatLead.cpp:202
without the excess -> T-L3 (v) and T-L6 RED. Then G3 (b) (0 operator-new in the three windows) and G2.

## 4 STAGES + ORDER (P7)
| key | who | scope | needs | exit |
|---|---|---|---|---|
| D0 | builder | PROBE ONLY: probe-sync.py measures the click origin (E1), the doubled-click INFO (E3), knobs PROBESYNC_R7_SECONDS and PROBESYNC_R7_WAV_B, the hop-delay INFO; probe-sync.sh generates the seed-1 file; self-test cases for the origin and the doubled-click functions with their RED on mutated copies. No src, no test, no app build. | this ruling | SELFTEST |
| RD | Harmony | the diagnosis run (two launches, ~5 min each, live lock; the machine need not be quiet) on build-lane as built at 68abc16; reads the decision table of P2 and E3 of P3 | D0 | an outcome O1..O5, written into rulings-bf2.md |
| TQ | Harmony | [timing] 5 idle + 5 loaded (P4); needs the quiet window for the idle half | none (may precede D0) | an outcome of P4's table |
| S4b | builder | P1 (keys), P5 (route + row R13 + the mutant app), P6 (BeatLead), and -- on O1 / O5 -- R7 / R7b on the measured origin (the probe's default flips; self-test re-stated). Reviews: keys r3; a real-time read of the BeatLead diff. | RD = O1 or O5 (on O2 / O3 / O4: only P1, P5, P6 are built and S6 stays stopped) | G1, G1-RED, G2, G3 (b), SELFTEST, R13 |
| S6, S5a, S5b | as RD: section 4 | unchanged | S4b; RD's outcome known and not O2 / O3 / O4 | unchanged, + G7's learn state in S5b |
May S6 be built before P2's diagnosis is known? NO (H-12 stands): D9 edits the take's start wait; if RD reads O2
the fix lands in the same file and the two would be confounded. D0 + RD cost one short builder context and ten
minutes of live time, so nothing is gained by racing them. S4b touches no take code and may start the moment RD
has run (it must not rebuild build-lane while RD runs).
What Harmony runs herself, in order: at the first quiet window (no agent running, no video playing) TQ's idle
half and G1's quiet [timing] x3 (M6), then TQ's loaded half; RD after D0 (no quiet window needed); then after S4b: SELFTEST, G1, G2, R13 with its RED, and the S3f list still owed
(R5 x5, G6, R7's 21 arms + R7b on the re-stated row -- quiet machine, ~23 min); R7's mutant RED once more on the
re-stated row (build-mut-r7 is still on disk). After S6: a 5-arm R7 SUBSET as development evidence that D9 moved
nothing. The full R7 runs again in the final gate list on the merged tree, as RD: section 4 already says.

## 5 GATE ROWS that change or are added
R7 (changed again; S4b; applies on O1 / O5). In RD: section 5 row R7, REPLACE
  "Per arm: paired marker errors (probe-step3's T2 math);"
 WITH
  "Per arm: the click grid's origin c0 is MEASURED in the take's own audio: c0 = the median over the clicks of
  (onset frame of click k in the asset WAV - onset frame of click k in the source WAV), onset = the first sample
  above half full scale, hits within 100 samples collapsed. Paired marker errors = (marker stamp - the take's
  first sample - c0) - the nearest grid click (probe-step3's T2 math on that origin); c0 is printed per arm;"
 and REPLACE "VALID when every arm pairs >= 100 markers and drops <= 4."
 WITH "VALID when every arm pairs >= 100 markers, drops <= 4, every click's own origin lies within 4 samples of c0,
  and c0 is a non-negative multiple of the device block size."
 BAR, the extension, the SE limit and the finding rule: UNCHANGED. Add to INFO: "clicks carrying two markers, and
 the gap between the two".
 RED arms: the mutant app as before (FAIL near +100: the mutation moves markers, not audio); SELFTEST: an arm whose
 asset origin is 512 with errors + 512 reads the same value as an origin-0 arm; an arm with one click 9 samples
 off its origin -> INVALID; an arm with origin 300 -> INVALID; a mutated copy that ignores c0 -> DIFF.
R7b (changed; S4b). REPLACE "g_k = (gesture stamp - the take's first sample) - the click-grid position nearest
 marker k" WITH "g_k = (gesture stamp - the take's first sample - c0) - the click-grid position nearest marker k
 (on the same origin)"; "B = the median paired marker error over ALL 0 arms of the run" now reads R7's re-stated
 errors. BAR and RED unchanged.
RD (new; D0; a DIAGNOSIS run, never a gate line). PROBESYNC_ROWS=R0,R7,R6 with PROBESYNC_R7_ARMS=0,0,0,0,0,0,0,0
 PROBESYNC_R7_SECONDS=20, then PROBESYNC_R7_ARMS=0,0 at 60 s with PROBESYNC_R7_WAV_B=<the seed-1 file> for the
 second take; two launches. Lines: per take "INFO  RD take k: c0 <n> samples (<n / block> blocks, spread <s>);
 median error as assumed <e> / on the measured origin <e'>; hop delay mean <ms>; doubled clicks <list>". Verdict
 line: "RD outcome O<n>" by P2's table. Its own RED: SELFTEST cases feeding synthetic takes of each outcome.
R13 (new; S4b; PRODUCTION mode, test-server build). Start: POST /api/sync/set {"ms":0}, settled.
 {"input":"key","stepMs":1,"value":1} -> GET /api/sync targetMs 1 within 200 ms; the same with "value":0 -> still 1;
 {"input":"key","stepMs":-1,"value":1} -> 0; {"input":"note","stepMs":1,"value":1} -> 1;
 {"input":"cc","ccMode":"relative","stepMs":1,"value":67} -> 4; "value":59 -> -1; "value":64 -> still -1;
 set 499, key +1 three times -> 500, 500, 500; set -499, key -1 three times -> -500, -500, -500; set back to the
 start value before R6. PASS line "PASS  R13 the handler moves the dial: 13 of 13 steps".
 RED: build-mut-r13 (the handler's nudge with the sign flipped) -> "FAIL  R13 step 1: targetMs -1, want 1";
 `grep -rc MUTANT src tests` = 0 on the real tree afterwards.
TQ (new; a DIAGNOSIS run, never a gate line). `ps -Ao pcpu,comm -r | head -5` then
 ctest --test-dir <WT>/build-lane -R "\[timing\]" -V   five times idle; then four `while :; do :; done &` loops
 and five more; the loops are killed by their own pids. Recorded: the ten "[timing] arm" and five "[timing] pair"
 lines of each run. Outcome by P4's table.
G1 (changed): add to the suite list "S4b: test_binding_sync_nudge (bindingIsLive, learnMidiCC, fromVar),
 test_beat_lead (the same printed lines as before the removal)". The [timing] paragraph: UNCHANGED.
G7 (changed; S5b): to H-10's MIDI-learn state add "ML-2: 'Sync +1 ms' selected and waiting: the dump's title text
 is 'Send a MIDI note...'; after a CC is fed through the overlay's own MIDI entry (S5b's test route) the dump
 still reads waiting and shows no binding on the target". RED: the S4 tree binds it.
SELFTEST (changed): + the origin, validity and doubled-click cases above; exit 1 on each mutated copy.

## 6 WHAT ONLY BORIS CAN CHECK
 1. Shortcuts or MIDI learn: put a key (or a pad) on "Sync +1 ms" and one on "Sync -1 ms"; press each a few times.
    Expect: the SYNC number moves by one each press and stops at +500 / -500. Wrong: no move, a jump, or the
    wrong direction.
 2. MIDI learn: click "Sync +1 ms", then turn a knob. Expect: the screen keeps waiting and nothing is attached to
    the knob; a pad or key then attaches as usual. Wrong: the knob attaches, or a knob that was attached to
    something else loses it.
Nothing in P2..P6 is visible to him.

## 7 QUESTIONS FOR BORIS
 Q1 Sync is nudged with a key or a pad, one for earlier and one for later. Do you also want to turn a knob for
    it? DEFAULT: no. (A "yes" becomes its own piece of work: the app has to be told what kind of knob it is.)

## 8 RISKS (the strongest counterargument first)
 K1 P2: "re-stating R7 on a measured origin hides a product fault". It would if the fault were in how markers sit
    against the take's audio -- but that is exactly what the re-stated error still measures, and the mutant app
    still fails by +100. What it stops measuring is where REST started the file, which no take reader sees.
    Cheapest refuting test: RD itself -- O2 (origin 0 everywhere, errors still two-valued) kills this position
    and stops S6.
 K2 P2: the clustering in F12 fits a latched state better than a per-take race, and HA predicts no latch. If RD
    shows c0 following the state AND a latch, HA still holds for the gate (the origin is measured) but the window
    of F6 is launch-dependent; recorded, nothing built. If RD never reproduces the state: O5.
 K3 P2: the 16-bit asset may not reproduce the source bit for bit, so per-click origins could scatter and the new
    validity clause could fail for no product reason (NOT verified). RD prints the spread before any gate depends
    on it; a spread above 4 samples is O4 -> architect, not a quiet widening.
 K4 P3: the doubled click is shown on earlier builds and by arithmetic at the pin, not by a detector run at the
    pin. E3's seed-1 take is the test; if B also doubles near 34.5 s the content reading is wrong.
 K5 P1: plan I9 chose encoders and this ruling makes them unreachable from the screen. Counter: no screen ever
    made a Relative binding for ANY action (F1), so nothing Boris has is taken away; option (a) would put a
    63 ms jump under a fader. Cheapest refuting test: Q1, and the "Last:" readout shows what his knob sends.
 K6 P1: the prompt edit is on-screen text. It is a standing prompt of a waiting state, not a report of an event
    (H-2's line); the interaction critic judges ML-2 in G7. If it reads as a fault, the fallback is the prompt
    unchanged and the same silent waiting.
 K7 P4: the mechanism is INFERRED; TQ may show the regime on an idle machine, and then the case cannot gate as
    written. That outcome is in the table and stops for the architect; the bar is not touched either way.
 K8 P5: a test route that calls the handler skips BindingManager and the key path. Those are unit-tested with the
    real manager; the row's claim is only "the real case calls the real dial with the right sign and clamp".
 K9 P6: the removal edits analysis-thread code. Guards: identical printed T-L lines, the mutant, G3 (b), G2, and a
    real-time read of a diff that only deletes.
 K10 S4b carries four concerns in one builder context. If it runs long, split P6 off as S4c; the order does not
    change.

STATUS: DONE
