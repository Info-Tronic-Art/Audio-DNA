# RULING bf2 STOPS -- architect ruling on the blind council's attacks on plan-bf2-stops.md (lane bf2, s-rta-1003b)
Architect (opus, max effort), 2026-10-03. Harmony decides after this; it is her working document.
Read-only. Lane read ONLY at 68abc16 (git objects). Nothing built, run or launched; python arithmetic on saved logs.
Shorthands: W: = the lane tree at 68abc16. SP: = the session scratchpad (/private/tmp/claude-501/-Users-boriskarpman-
projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad). PL: = .harmony/.reports/s-rta-1003b/
plan-bf2-stops.md. RBD: = .harmony/.reports/s-rta-1003/ruling-bf2-delta.md (the plan calls this "RD:"; here RD is
ONLY the diagnosis run). J: = build/_deps/juce-src/modules (the JUCE the lane builds with, SP:bf2-S3f/build-mut-r7.sh).
Papers (verbatim, 4 seats, 29 attacks): .harmony/.reports/s-rta-1003b/attack-bf2-stops-papers.md.
Labels: VERIFIED (read at the pin, or computed from a saved file), INFERRED, ASSUMED.
Precedence: Boris's verbatim words > rulings-bf2.md > this ruling > the plan body. Section 3 OVERRIDES the plan.

## 0 VERDICT
The plan NEEDS REVISION; its core stands. 29 attacks ruled: 18 ACCEPT, 10 PARTIAL, 1 REJECT. 20 amendments.
 - Both MUSTs are upheld. (1) The "hop delay" reading that was to split the product hypotheses is 0.000 at dial 0
   by construction -- stamp and process time are the same variable (V1) -- so the plan's O2 branch could never be
   taken. It is struck; RD reads two independent rulers instead (the take's own audio, and the analysis witness),
   and a product outcome goes to a second, pre-specified instrument, not to a guessed fix. (2) G7's ML-2 would pass
   on a dead injection route, and no stage builds that route (V12). It becomes a live row of its own (R14) with a
   control arm in the same session and a mutant RED.
 - P2 stands as the plan argued it -- the saved numbers and the code order point at the probe's assumed grid origin,
   not at the take (V3, V7) -- but it is NOT ruled by reading. The diagnosis run is re-sized on launches (6 x 6
   short takes: the plan's "0.4 %" chance of seeing nothing was really about 22 %, V7), its table is rewritten so
   the artefact verdict needs a take that SHOWS the artefact, and re-stated R7 gains two clauses that make a
   product one-block state fail by name instead of raising the noise.
 - Keys: fork (b) stands (MIDI learn never attaches a CC to a Sync target), built smaller: no load-time refusal, the
   engine guard stays reachable and tested. The Boris question is reworded to name what he loses (buttons and knobs
   that send CC). A held key is ruled (it repeats) and asked.
 - Scope: the keys stage no longer waits for the diagnosis, S6 no longer waits for the keys stage, the BeatLead
   removal leaves this lane (filed debt; the real-time review approved it as debt, V15), the seed-1 arm and the
   loaded [timing] arm are cut.
 - Order: D0 -> RD -> R7r -> Harmony's owed R7 run -> S6 -> S5a -> S5b, with S4b (keys) beside them from the start.
Nothing in this ruling loosens a pre-registered bar. Every outcome of RD has a ruling (section 4).
H-12's six items -> where ruled: the keys MUST: A9-A15, R13, R14. R7's one-block state: A1-A6, A19, A20, section
4's table. The late marker at raw index 70: A7. [timing] under load: A8. The untested handler case: A14, R13.
The dead BeatLead branch: A18.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
The analysis loop and the witness
 V1  VERIFIED. W:src/analysis/AnalysisThread.cpp:134 stamps a drained hop with this pass's clock
     (`delayLine_.commitBack(nowMs, ...)`); :150 tests `frontDue(nowMs, lateMs)` in the SAME pass; :155 calls
     `processHop(..., stampMs, nowMs)`; :158 publishes the hop delay as `nowMs - stampMs`. At dial 0 lateMs is 0, the
     hop is due in the pass that drained it, and stampMs and processMs (W:src/analysis/SyncWitness.h:98-99) are one
     variable. "Process time minus drain stamp" is 0.000 at dial 0 whatever the product does. AU-1 / GA-2 are right.
 V2  VERIFIED. The witness ring holds 1024 hops (~10.9 s, SyncWitness.h:161), but the probe's collector polls every
     0.25 s in its own thread and keeps every entry (W:.harmony/probe-sync.py:172-222; started in main before any
     row). A 60 s take is covered as built. Each hop entry carries `timestamp` (48 kHz samples processed),
     `stampMs`, `processMs`, `appliedMs`, `onsetCount`; each loop entry `ringReadyAfterDrain`, `hopsDrained`
     (ApiServer.cpp handleDebugSyncWitness; SyncWitness.h:97-102, :144).
The take start and the stamps
 V3  VERIFIED (plan F6 holds). POST /api/perf/record with "audioFile", message thread, W:src/MainComponent.cpp:
     loadFile (:5910) -> RecorderHost::arm (:5946) -> applyAudioTransport("play") (:5955). Inside arm, AFTER the tap
     is armed (W:src/recording/AudioTap.cpp:146 `armed_.store(true)`): the clock reset, recorder_.start, the
     checkpoint capture, takeForSave, publishStatus and the provisional take.json save (RecorderHost.cpp:263-294).
     "play" is AudioEngine::play -> transportSource_.start() (AudioEngine.cpp:74-77; J:juce_audio_devices/sources/
     juce_AudioTransportSource.cpp:119-131: the next callback plays).
 V4  VERIFIED (plan F7 holds). The tap's first sample is the delivered counter of the first callback after the arm
     (AudioTap.cpp:180-183; W:src/audio/CombinedCallback.h:138, :162-167). In File mode the tap and the analysis both
     get the player's output (:126-133). So each callback between arm and play puts one block of silence at the head
     of the asset: the file's frame 0 sits at asset frame N x block. The MECHANISM is verified; that it accounts
     for the observed state is INFERRED until RD.
 V5  VERIFIED, and it corrects the plan's HD. The transport reads through a 32768-frame read-ahead buffer
     (AudioEngine.cpp:69-70). In this JUCE a block the buffer cannot serve at all is cleared and the play position
     does NOT advance (J:juce_audio_basics/sources/juce_BufferingAudioSource.cpp:117-122 returns before :169
     `nextPlayPos += info.numSamples`). A wholly missed first block therefore delays the file by one block and
     loses no click: the same signature as V4, not "click 0 lost". Both are "the file starts N blocks into the
     asset"; RD's measured origin covers both and cannot tell them apart (it does not need to).
 V6  VERIFIED (plan F8 holds). A marker's stamp is the delivered counter of the message-thread tick that saw the
     onset count rise (RecorderHost.cpp:456-462, :651-661, :737-752); the probe's error is (stamp - firstSample) -
     the nearest multiple of the click interval (probe-sync.py:888-913): it assumes file frame 0 = asset frame 0.
The saved runs (each probe-sync.json, rows.R7.arms; recomputed here)
 V7  VERIFIED. 14 takes in 4 launches; every firstSample is a multiple of 512.
     | launch (app) | arms (dial: median error, samples) | shifted |
     | SP:bf2-S3f/out-green (lane) | 0: 1472; 100: 1472; 0: 1984 | 1 of 3 |
     | SP:bf2-S3f/out-green2 (lane) | 0: 1472; 100: 1408; 0: 1472; 100: 1472; 0: 1472 | 0 of 5 |
     | SP:bf2-S3f/out-red-r7 (MUTANT app) | 0: 1984; 100: 6720; 0: 1984 | 3 of 3 |
     | SP:bf2-R1/out-live (lane) | 0: 1472; 100: 1536; 0: 1472 | 0 of 3 |
     4 of 14 takes; on the lane app proper 1 of 11; 2 of 4 launches. Every normal dial-0 take reads 1472 exactly
     (6 of 6), every shifted one 1984 (3 of 3): + 512, the lattice untouched. The 100 arms wobble 1408..1536
     because their markers split about evenly over two blocks (block histograms 67:53, 72:48, 69:51, 58:62).
     The plan's "0.71^16 = 0.4 %" uses p = 4/14 with the mutant's three takes in it. At p = 1/11:
     (10/11)^16 = 22 %. With the launch as the unit (2 of 4): 0.5^2 = 25 %. GA-3 and AU-2 are right.
 V8  VERIFIED (arithmetic). 24000 mod 512 = 448, so successive clicks walk the block phase p in steps of 64: the
     8-value sawtooth. Normal error = 1536 - p for p < 384, 2048 - p above: the onset is published 3 or 4 blocks
     after the click's own block. That is the detector's latency (about 31 ms), not a stamping error.
 V9  VERIFIED (plan F13 holds). Raw marker 70 is an EXTRA marker in all 14 takes: e.g. out-live arm 0, raw 66..72 =
     1664, 1216, 1792, 1344, 4416, 1408, 1472 -- index 71 carries the value index 70 should have had and the
     sequence runs one step behind from there; 121 markers for 120 clicks. It sits 3072 samples (6 blocks, 64 ms)
     after marker 69 there. Dial 0, dial 100 and the mutant alike. Detector settings: threshold 0.3, minimum
     inter-onset interval 50 ms, adaptive whitening on (W:src/analysis/OnsetDetector.cpp:19-22).
     NOT re-derived: plan F14 / F15 (two takes under ~/Documents/Audio-DNA, outside the files this ruling reads).
[timing]
 V10 VERIFIED. Overhead = arm mean - D. LOW: SP:bf2-M0/g1-LastTest.log (37 arms 1.50-1.55; 38 arms 1.20-1.33; 0
     arms 1.16-1.22; 150 arms 1.30-1.32), timing-run3.log (1.51-1.55; 1.24-1.30; 1.16; 1.37-1.39),
     ctest-timing-rerun.log (1.55-1.57; 1.24-1.29; 1.21-1.22; 1.37-1.46). NORMAL: timing-run5.log (1.07-1.12;
     1.05-1.09; 0.97-0.99; 1.09-1.10) and the S3f run in the lane report (1.12-1.15; 1.10-1.14; 0.96-1.01;
     1.06-1.12). timing-run4.log is BOTH: its 37 / 38 arms are low (1.54-1.63; 1.33-1.36) and its 0 / 150 arms,
     23 s later, are normal (0.99-1.01; 1.08-1.13). So the regime is a stretch of TIME, not a property of a run:
     the plan's "regime of a whole run" is too strong. Signature that separates the two in every saved run: the
     smallest 37-arm overhead minus the largest 38-arm overhead is +0.17 .. +0.26 ms when low and -0.02 when normal.
     run3 pair 4/5 = 39.240 - 38.548 = 0.692 < 0.7 (GA-5's figure holds). Machine record (lane report at the pin,
     bf2-delta.md:85-89, :317-318, :593): low with Boris's video player at 21.8-31.4 % CPU (4 runs), normal with
     no player (1 run) AND normal with the same player at 27.2 % (1 run, its pairs printed; one more
     pass inside G1 minutes earlier). The trigger is NOT established.
     The harness cannot produce a D-dependent term (the poller and the feeder do not know D,
     W:tests/test_analysis_sync_thread.cpp:525-530, :570-583): the term is the analysis loop's own wake timing.
     Over all arms of the six saved runs the overhead is 0.96 .. 1.63 ms: the applied delay moved by <= 0.7 ms.
Keys
 V11 VERIFIED. `ccMode` is set nowhere but W:src/binding/Binding.h:69 (default Absolute) and BindingManager.cpp
     fromVar (:285). The learn overlay's CC block (W:src/ui/MidiLearnOverlay.cpp:253-287) sets `lastMidiMessage_`
     first (:253), REMOVES every binding on that CC (:260-268), adds an Absolute one (:284), leaves the waiting
     state (:286-287). BindingManager.cpp:168-175: an Absolute CC on SyncNudge returns false; a Relative one hands
     `value - 64`. The generic Relative mode of every other action uses the same 64-offset coding (:177-190). A
     bindings file reaches the manager only through the Shortcuts menu's file chooser (MainComponent.cpp:7353
     loadFromFile -> fromVar); nothing saves or loads bindings by itself. The lane is not merged: no file of
     Boris's holds an S4-era entry (ASSUMED: he runs main).
 V12 VERIFIED. W:src/api/ApiServer.cpp registers no route that names midi, learn, bind or a binding action (route
     list :168-372). ruling-bf2-gates-restated.md contains neither "overlay" nor "learn" (grep): H-10's two overlay
     states have no route in any spec, and "S5b's test route" (PL:336) does not exist. GA-1 is right.
 V13 VERIFIED. The handler case is MainComponent.cpp:7939-7945 (`syncNudgeDeltaMs(binding, value)`, then nudge).
     For a MidiCC binding syncNudgeDeltaMs treats `value` as a signed TICK count x |step| (Binding.h:103-112);
     the value - 64 decode lives in BindingManager. A route posting raw 67 to the handler nudges +67. GA-4 is right.
     A note hands velocity / 127 (BindingManager.cpp:112-123); value > 0 is a press.
 V14 The key path: MainComponent::keyPressed (:4040) -> BindingManager::processKeyDown (:4150 -> W:src/binding/
     BindingManager.cpp:52-73) fires the action on EVERY call; no repeat guard anywhere (VERIFIED, grep). JUCE's
     mac peer forwards every key-down, repeats included (J:juce_gui_basics/native/juce_NSViewComponentPeer_mac.mm:
     918-925, no isARepeat test): INFERRED from source, not run. So a held Sync key nudges at the system key-repeat
     rate -- and so does every other key binding today (pre-existing).
BeatLead, pins, order
 V15 VERIFIED. `originX_` changes only at W:src/analysis/BeatLead.cpp:195 / :198 and `barX_` only at :191, in the
     same block; neither is reset anywhere (grep): originX_ == barX_ on every hop and both arms of :194-199 leave
     the same state. review-bf2-realtime-r2.md APPROVES the lane with the branch left as filed debt ("Round 1
     allowed DEBT_FILED"; its one remaining NIT: copy the debt to a ledger).
 V16 VERIFIED. RBD:516 gives S6 "needs M0, S3f" -- no keys stage. W:.harmony/APP-INVENTORY.md:31 pins "1330 unit
     tests (... `ctest -N` 1330 = 1324 + the 6 cases of test_binding_sync_nudge ...)" and its route counts;
     W:docs/claude/testing-eyes.md lists the TEST-ONLY routes. PL:142-143 names neither. GA-6 is right.
 V17 VERIFIED. The tap inserts silence and a gap marker when a callback arrives late (AudioTap.cpp:207-215); the
     counter advances by the same amount (CombinedCallback.h:167); the take saves them (W:src/recording/Take.cpp:
     60-67, audio "gaps"). After a gap every later click sits later in the asset than in the source file.
     GET /api/debug/audio_devices gives "opens" and the opened block size and rate; the probe already reads the
     latter two once per R7 run (probe-sync.py:1095-1098). All four saved launches log one device pair (built-in
     microphone + built-in speakers) and one "[Analysis] source rate 48000 Hz" line: no rate change in any of them.

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)
| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| AU-1 | MUST | ACCEPT | V1: stampMs and processMs are the same nowMs at dial 0. The seat's first quantity (stamp minus the onset hop's timestamp) is adopted as RD's second ruler; its second (tick wall time minus processMs) needs the take's t = 0 on the app clock, which nothing exposes -- that split moves to D1. Polling is already 0.25 s (V2). | A1, A2, A20 |
| AU-2 | SHOULD | ACCEPT | V7: 2 of 4 launches. PL:186-190 is met by a run that never shows the state. | A3, A4, A6 |
| AU-3 | SHOULD | ACCEPT | V17: "opens", block and rate are one GET away and no take records them. Cheap, and it is the only instrument for a device-layer cause. | A5 |
| AU-4 | SHOULD | PARTIAL | V10: the player correlates and its CPU share does not; "loaded normal -> CPU share is not the trigger" would rest on loops that are not the correlate. ACCEPTED: the loops arm is cut as designed, the product claim is re-based on the measured overheads. REJECTED: a player arm (the player is Boris's own app; a diagnosis arm should not need it started and stopped on his screen) and a D sweep (a change to a case H-8 froze, at idle where the regime is absent). The seat's simulation was not re-run (not arithmetic on a saved log). | A8 |
| AU-5 | NIT | PARTIAL | V9 confirms the reading. The seat is right that E3's "none" outcome proves nothing; nothing in this lane depends on E3, so it is cut rather than widened. The seat's same-file-offset control is filed with the detector item. | A7 |
| AU-6 | NIT | ACCEPT | PL:158-162: F14 / F15 are origin-0 takes; every hypothesis predicts them. They validate the click detector on a real asset, nothing more. The 3 ms window is a back-fit. | A4 |
| AU-7 | SHOULD | PARTIAL | V17. ACCEPTED: the block is the opened buffer_size read in that arm; the gap count is printed on every arm line. REJECTED: "a gap take stays valid with a flag" -- after a gap the click grid is broken for every later marker, on the old statement and the new; the arm is INVALID and the line says why. | A6 |
| MI-1 | SHOULD | PARTIAL | PL:113 and PL:349-351: (b) does drop CC buttons and Q1 never names them. ACCEPTED: Q1 is reworded so Boris can answer it and it states the loss. REJECTED: ruling a button-only edge rule now -- the app cannot tell a button from a knob from one message (PL:115), so a button rule also binds knobs; that is the design item a "yes" buys. | A9 |
| MI-2 | SHOULD | PARTIAL | V11: the Sync arm decodes 64-offset only, exactly like the app's generic Relative mode, and is reachable from a loaded bindings file like every Relative binding in the app. ACCEPTED: nothing may call it "the encoder case works"; docs and R13 say "64-offset coding only"; T4 goes. REJECTED: deleting a ruled, tested arm (I9), a per-message cap (unrequested), a test that pins a wrong result. | A11, A14 |
| MI-3 | SHOULD | ACCEPT | V11: fromVar clears and reloads; a refused entry is gone on the next save. Remedy differs from the seat's: the loader is not touched at all. | A10 |
| MI-4 | SHOULD | PARTIAL | V14: unruled, and decided by accident. ACCEPTED: ruled (a held key repeats, as every key binding does), pinned in a unit case, put in Boris's check and asked. REJECTED: a LIVE row for it -- the manager's key entry is the same code in the unit binary (T6 calls it three times); what a live row would add is the OS's own repeat, and that needs a real key, which the rig forbids; only Boris's hand shows it. | A13 |
| MI-5 | SHOULD | PARTIAL | V13. ACCEPTED: a fractional velocity is tested (unit, through the real manager; and one R13 step); the learned-CC-refused case gets a live row (R14). REJECTED: a manager-resident binding driven live -- the manager's entry points are unit-tested with the real class and R13's claim stays "the handler case". | A14, A15 |
| MI-6 | SHOULD | ACCEPT | V11 (:253, :260-268, :286-287): each of the three slips the seat names passes ML-2 as written. | A15 |
| MI-7 | NIT | REJECT | V11 (:94): the standing prompt says "Send a MIDI note or CC...". On a Sync target that sentence would invite the one input the target ignores. Removing two words states what the target takes before anything happens; it reports no event and no refusal (H-2's line: a state display is admissible, a text that reports the refusal is not). Harmony may overrule (HD4). | A12 |
| GA-1 | MUST | ACCEPT | V12: no such route in the tree or in any spec; "the S4 tree binds it" cannot be run (that tree has no route either). | A15 |
| GA-2 | MUST | ACCEPT | V1. | A1, A2, A20 |
| GA-3 | SHOULD | ACCEPT | V7: 1 of 11 on the lane app; (10/11)^16 = 22 %. | A3, A4, A19 |
| GA-4 | SHOULD | ACCEPT | V13: the handler takes ticks. | A14 |
| GA-5 | SHOULD | PARTIAL | V10. ACCEPTED: the loops arm does not reproduce the trigger; the table had no mixed rows; the action on a failing quiet run is now stated. REJECTED: "re-run N times" -- a new re-run path is a looser gate. | A8 |
| GA-6 | SHOULD | ACCEPT | V16. | A16 |
| GA-7 | SHOULD | ACCEPT | PL:305-306 accepts any multiple. The bound is the largest multiple RD records (at least 1), written down before the gate run; the block is the opened buffer_size, not CLAUDE.md's 128. | A6 |
| GA-8 | NIT | ACCEPT | PL:137-138 with PL:151: once addBinding refuses, the guard's test passes on an empty manager. Resolved at the root: addBinding is not changed, so the S4 case keeps failing when the guard goes. | A10 |
| SC-1 | SHOULD | ACCEPT | V16; PL:288 names a build-directory collision, not a dependency. | A17 |
| SC-2 | SHOULD | ACCEPT | V15: equal arms, a true comment, and a real-time review that approved it as debt. | A18 |
| SC-3 | SHOULD | PARTIAL | ACCEPTED: no addBinding / fromVar refusal. REJECTED: an overlay-only guard -- "an existing CC binding is not stripped" is an ordering fact that only non-UI code can pin RED-first, so the remove + add moves into one manager method; and the unchanged prompt (see MI-7). | A10, A12 |
| SC-4 | NIT | PARTIAL | ACCEPTED: one mutant line for R13; the Relative decode is not re-tested live. KEPT: one clamp step per side (two requests each) -- nothing read here shows another live row driving a nudge into the clamp. 9 steps, not 13. | A14 |
| SC-5 | SHOULD | ACCEPT | PL:224-229 rules P3 before E3 runs; V1 removes the hop-delay field. | A1, A7 |
| SC-6 | NIT | ACCEPT | PL:246-248 rules the gate before TQ runs; G1's quiet x3 is the idle half. | A8 |
| SC-7 | NIT | ACCEPT | It is the plan's own position (PL:286-289) and H-12's. Kept, and extended: Harmony's owed R7 run also precedes S6 (A19). | A19 |
Seats in conflict, reconciled: AU-5 (widen E3) against SC-5 (drop it) -> dropped, the control filed (A7). AU-4 / GA-5
(widen TQ) against SC-6 (shrink it) -> shrunk; what the three agree on (the loops arm proves nothing) decides (A8).
MI-2 (delete the Relative arm) against SC-3 (touch less) -> the arm stays, its claims are corrected (A10, A11).
MI-7 / SC-3 (prompt unchanged) against a prompt that would invite the one input its target ignores -> the two words
go (A12; Harmony may overrule, HD4).

## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
A1  (AU-1, GA-2, SC-5) The hop-delay reading is STRUCK: from E1 (PL:176-178), from the readings (PL:182-184), from
    O2 (PL:191-195), from D0's scope and from RD's line. It is 0.000 at dial 0 by construction (V1).
A2  (AU-1) RD reads TWO rulers per take, both from data that exists at the pin (no app change):
    - the AUDIO ruler: c0 and e' as PL:173-176 defines them (c0 = the median over the clicks of asset onset minus
      source onset; e = today's median marker error; e' = e - c0);
    - the WITNESS ruler: C = the median, over the take's markers, of (marker stamp - the `timestamp` of the witness
      hop on which onsetCount rose for that marker). Markers and count-rising hops are paired in order; the pairing
      offset is the one that minimises the spread of the differences. C carries an unknown constant per launch
      (the two counters start at different moments), so C is compared between takes of ONE launch only.
    What each ruler sees: a late file start (V4, V5) moves e and c0 together and moves neither e' nor C. A product
    lateness of a block -- the analysis behind the audio, or the tick behind the analysis -- moves e, e' AND C and
    leaves c0 at 0. The two references are independent: one is the asset WAV, the other the analysis thread's own
    sample count.
    Also printed per take from the witness: the number of hops whose appliedMs is not 0 (a dial that was not at 0).
A3  (AU-2, GA-3) RD's size. The unit is the LAUNCH. Six launches of the lane app as built at 68abc16; each: six
    takes at dial 0, 12 s each (24 clicks). 36 takes, about 14 minutes. Chance of seeing no shifted take if the
    state exists: 3 % at 1 take in 11 independent; 1.6 % if it is latched per launch at one launch in two.
    Stop rule (pre-registered): Harmony may stop after any completed launch once at least 2 valid takes with c0 >=
    one block and at least 2 with c0 = 0 are on record; an O2 or O4 reading stops the run at the end of that launch.
    The plan's two 60 s takes and its seed-1 arm are not part of RD (A7).
A4  (AU-2, GA-3, AU-6) The decision table of PL:185-201 is REPLACED by section 4's. O1 requires a take that shows
    the artefact; "nothing seen" is O5 and nothing else; the plan's O3 is folded into O2 (a product reading stops
    the lane whatever c0 does). HA's "FOR" list loses F14, F15 and the 3 ms window (PL:159-161): they do not
    discriminate. What supports HA is V3 + V4 (the order in the code) and V7 (exactly one block, lattice intact).
A5  (AU-3) RD prints per take, from GET /api/debug/audio_devices read after the take: "opens" (and its change since
    the previous take), the opened buffer_size and sample_rate. A take whose "opens" rose, or whose block or rate
    differs from the launch's first take, is a DEVICE-EVENT take: left out of the outcome, listed by name, and
    reported to the architect if it is also a shifted take.
A6  (AU-7, GA-7, AU-2) Re-stated R7 (section 5) gains, beyond the plan's two clauses: the block is the opened
    buffer_size read in that arm; c0 = k x block with k in 0..NMAX; a take with a gap is INVALID and says so; the
    0-arm medians span <= 256 samples and the 100-arm medians span <= 320 samples; every 0-arm median lies within
    256 samples of EREF. NMAX and EREF are written into rulings-bf2.md by Harmony with RD's outcome (NMAX = the
    largest k RD recorded, at least 1; EREF = the median of RD's per-take e'; the six saved normal takes read 1472).
    Why the last two: dbar is a DIFFERENCE between dial 100 and dial 0, so a product one-block state that holds for
    a whole launch is invisible to it, and one that comes and goes only inflates the SE. The span clause names the
    second, the EREF clause the first. Thresholds from V7: the natural span is 0 for the 0 arms and 128 for the 100
    arms (their lattice wobble); a one-block shift makes it at least 384.
A7  (SC-5, AU-5) E3 is CUT: no seed-1 file, no PROBESYNC_R7_WAV_B, no extra 60 s takes. P3's ruling stands on V9
    with these labels: an extra onset on click 69 -- VERIFIED at the pin by arithmetic; older than this lane -- the
    plan's F14 / F15, not re-derived here; bound to the click's content rather than to take time -- NOT established
    and not needed (the analysis sees the same hop contents in every take, V4, so "every take, same click" follows
    either way). R7's rule (drop beyond 21.333 ms, valid at <= 4 drops) already handles it and is not changed; R7
    prints the doubled clicks as INFO (built in R7r). Filed for an analysis lane: the detector fires twice on
    about 1 click in 120 of this file, 53-64 ms apart; first test there = the same file replayed from +0.25 s.
A8  (AU-4, GA-5, SC-6) TQ is not a separate run. Its idle half IS G1's quiet [timing] x3 (RBD: G1, fact M6). Added
    to that record, no code change: for each run the ten arm lines, the five pair lines, the `ps` top five, and
    one sentence saying whether the run shows the LOW signature (V10: every 37-arm overhead at least 0.10 ms above
    every 38-arm overhead). Outcomes:
    - three normal runs: green; the regime stays recorded as "a time-local state of the machine, trigger unknown".
    - a run with the LOW signature that passes: green, recorded as a finding (RBD: D8 already records any pair
      outside [0.8, 1.2]; the signature is recorded either way).
    - a run that FAILS: RBD: G1's rule as written -- a process above 20 % in the pre-run `ps`: stop it or wait it
      out, re-run once, the re-run stands; nothing above 20 %: it BLOCKS, the record names the signature, to the
      architect. No other re-run.
    The loaded half (four shell loops) is cut; Harmony may run it once as INFO (HD6). P4's text is corrected: a
    stretch of time, not a whole run (V10, run 4); seen only while Boris's video player ran, not every time it
    ran; the bound on the product is the MEASURED overhead range (<= 0.7 ms across six runs, under the dial's 1 ms
    step), not the sleep-chain candidate, which stays unproven. The bars are untouched.
A9  (MI-1) P1's fork (b) stands. Q1 is replaced by section 7's Q1; check 6.2 states the loss in his words.
A10 (SC-3, MI-3, GA-8) P1's CHANGE (PL:131-143) is narrowed to:
    - W:src/binding/Binding.h: `inline bool bindingIsLive(const Binding&) noexcept` -- false exactly for action
      SyncNudge + inputType MidiCC + ccMode != Relative.
    - W:src/binding/BindingManager.{h,cpp}: `bool learnMidiCC(const Binding& candidate)` -- not live: change
      nothing, return false; else remove the bindings on that CC, add, return true. The SyncNudge guard in
      processMidiCC (:173) reads the same predicate (one definition of "live"; no behaviour change).
    - addBinding and fromVar are NOT changed. A loaded file's Absolute-CC Sync entry stays what it is today: loaded,
      inert (the guard), saved back unchanged. No screen can make one (HD5).
    - W:src/ui/MidiLearnOverlay.cpp CC block: `lastMidiMessage_` first as today; build the Binding; call
      learnMidiCC; leave the waiting state only when it returned true. The removal loop moves into the manager.
    Tests (RED first, on stubs: bindingIsLive returning true, learnMidiCC = remove + add): T1, T2, T3 of PL:145-148
    as written. T4 is dropped (fromVar is unchanged). The S4 case "an Absolute CC bound to SyncNudge fires no
    action" stays as written and stays the guard's test. Two cases are added: T5 "a note of any velocity nudges
    one step" (processMidiNoteOn velocity 1 and velocity 64 -> one step each) and T6 "three key-downs in a row
    nudge three steps" (A13). Mutants: bindingIsLive always true -> T1, T2 and the S4 Absolute case RED;
    learnMidiCC removing before the check -> T2 RED.
A11 (MI-2, GA-4) The Relative arm stays as built (I9). Docs (W:docs/claude/performance-controls.md:41,
    APP-INVENTORY): "MIDI learn does not attach a CC to a Sync target; keys and notes attach. A Relative CC entry
    in a bindings file loaded through the Shortcuts menu nudges by (value - 64) ticks -- the 64-offset coding
    only, as for every Relative binding in the app; other encoder codings are not decoded. An Absolute CC entry in
    such a file is ignored. A held key repeats at the system's key-repeat rate, like every key binding." No test,
    row or doc line says "the encoder case works".
A12 (MI-7, SC-3) The prompt edit of PL:139-141 STANDS: waiting on a Sync target the title reads "Send a MIDI
    note..."; on every other target "Send a MIDI note or CC..."; the "Last: ..." readout is untouched. One function
    gives the title to paint() and to the test dump (`juce::String titleText() const`), so the dump cannot drift.
    Harmony constraint: no on-screen text that announces an event or a failure -- this text does neither; it is
    there before any input and removes an invitation the target does not honour. HD4 if Harmony reads it otherwise.
A13 (MI-4) A held Sync key REPEATS (V14): one step per key-repeat, the same as every key binding today. No code.
    Pinned by T6, documented (A11), shown to Boris in check 6.1 and asked as Q2. A "one press, one step" answer is
    a small follow-up (a held-key latch for SyncNudge keys) with its own unit case; it is not built on a guess.
A14 (MI-5, GA-4, SC-4) P5's route and R13 are re-specified (section 5). The route's "value" is exactly the number
    the action callback receives: 1 / 0 for a key, velocity / 127 for a note, the signed TICK count for a CC. The
    "ccMode" field is dropped (the handler never reads it). 9 steps, one mutant line.
A15 (GA-1, MI-6) The learn refusal gets its own live row R14 (section 5), built in S4b with a TEST-SERVER-only
    route pair: POST /api/debug/midi_learn (open / close / select a target by its label / feed one MIDI message
    into the overlay's own `handleIncomingMidiMessage`) and GET /api/debug/midi_learn (the overlay's state: active,
    waiting, the selected label, the title, the "Last" text, each target's label, bounds and binding tag). The
    row holds a control arm (the same CC binds on a non-Sync target in the same session), the refusal, the two
    side conditions (the other target keeps its CC; a note then attaches) and a mutant RED. G7's ML-2 (PL:335-337)
    is REPLACED by two visual states that read this dump (section 5); the refusal is R14's, not G7's.
    Seams: `void MidiLearnOverlay::selectAt(juce::Point<int>)` = mouseDown's body under a name (every build;
    mouseDown calls it); a const state accessor for the dump (TEST-SERVER build only).
A16 (GA-6) Every stage that adds a test case or a route re-measures and updates the pins in the same commit:
    `ctest -N` against APP-INVENTORY:31 (S4b: 1330 + the cases it adds, listed by name in the lane report --
    T1, T2, T3, T5, T6 as specified = 1335; a different split is allowed, a mismatch between `ctest -N`, the
    inventory line and the list is a FAIL); the route registrations (S4b: + 3) in APP-INVENTORY and the TEST-ONLY
    route list in docs/claude/testing-eyes.md. G4's merge list gains R13 and R14.
A17 (SC-1, PL:379-380) Stages are split and re-ordered (section 4). S4b = keys + its two rows, and needs only this
    ruling: it runs beside D0 / RD in its own build directory and never rebuilds build-lane before RD is finished.
    The R7 re-statement is its own probe-only stage R7r after RD. S6 does not need S4b.
A18 (SC-2) P6 is REVERSED: the dead branch is NOT removed in this lane. It goes to the debt ledger with this text:
    "bf2 / D5: BeatLead.cpp:194-199 -- the manual-Resync arm equals the else arm (originX_ == barX_ on every hop);
    originX_, lastResyncs_, Flags::resyncs and its argument at AnalysisThread.cpp:457 are vestigial. Remove in the
    next lane that edits BeatLead: a deleting-only diff, proof = test_beat_lead's printed T-L3 / T-L6b lines
    unchanged, guard = a mutant that drops the bar excess at :202." Harmony may overrule (HD3: stage S4c).
A19 (GA-3, SC-7) S6 starts after RD's outcome is O1 or O5 AND after Harmony's owed R7 run (21 arms, on the re-stated
    row) has a verdict. Reason: a FAIL there is a finding in the stamping code (RecorderHost.cpp:456-462), the code
    D9 edits; and under O5 those 21 takes are the second and last look for the state before the take start changes.
    Harmony may run S6 beside it if she keeps the gate binary apart (HD2).
A20 (AU-1, GA-2) The instrument for a product outcome is specified now and built ONLY on O2 (stage D1). ONE field,
    TEST-SERVER-only: each witness hop entry gains `deliveredAtProcess` -- the audio callback's delivered-sample
    counter as the analysis thread reads it when it processes the hop (a relaxed atomic load through a cell handed
    to the thread, the way the source-rate cell is; no allocation, no lock). With the click's own position in the
    take's audio (the take's first sample + the click's onset frame in the asset) and the marker's stamp -- all
    three in the delivered domain -- every marker's error splits into two ABSOLUTE parts, with no clock compared
    across threads and no per-launch constant:
      analysis latency = deliveredAtProcess(the onset hop) - the click's position;
      tick latency     = the marker's stamp - deliveredAtProcess(the onset hop).
    Section 4 gives the ruling for each part moving by a block.

## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
| key | who | scope | needs | exit, and what it proves |
|---|---|---|---|---|
| D0 | builder | PROBE ONLY (.harmony/probe-sync.py, probe-sync.sh, probe-sync-selftest.py). RD mode: knobs PROBESYNC_RD=1 (R7's take loop WITHOUT R7's verdict and WITHOUT R7b; RD's lines instead) and PROBESYNC_R7_SECONDS; per take the audio ruler and the witness ruler (A2), the device fields (A5), the gap count, the hops with applied != 0; each take's record saved in probe-sync.json; an offline verdict over several saved files (`probe-sync.py --rd-verdict FILE ...`, no app). SELFTEST cases for every outcome and anomaly of the table below, with mutated copies. ONE development launch (2 takes of 12 s, live lock) to show the click detector reads a real asset. No src, no test, no app build. | this ruling | SELFTEST green, every mutated copy exits 1; the development launch prints two RD take lines with spread <= 4 (fact M2). Proves the instrument can read each outcome before it is trusted with one. |
| RD | Harmony | A3's run on build-lane as built at 68abc16 (live lock; the machine need not be quiet; nobody rebuilds build-lane meanwhile) | D0 | "RD outcome O<n>", NMAX and EREF written into rulings-bf2.md. Proves whether the one-block state is the probe's or the product's. |
| S4b | builder | KEYS: A10 (predicate, learnMidiCC, the overlay's CC block), A12 (title), A13 (T6), the two TEST-SERVER routes with rows R13 and R14 (A14, A15), the scratch app build-mut-r13 (two MUTANT lines), docs (A11), pins (A16). Its own build directory until RD has finished. | this ruling (NOT RD) | G1 with the pinned count, G1-RED (the two unit mutants); then Harmony: R13, R14, each with its RED; keys review r3. Proves no screen can make a dead Sync binding, a refused learn changes nothing, and the real handler case moves the real dial. |
| R7r | builder (short) | PROBE ONLY: R7 and R7b on the measured origin with A6's clauses (the only mode; no switch), the doubled-click INFO (A7), SELFTEST re-stated, one sentence in docs/claude/recording.md ("a take started with a file through REST holds whole device blocks of silence before the file's first frame; markers and audio share one sample domain") | RD = O1 or O5 | SELFTEST green and each mutated copy exits 1; a gates read of the diff. Proves the re-stated row still fails on what it must (section 5 RED list). |
| owed S3f rows | Harmony | quiet machine: R5 x5, G6, R7's 21 arms + R7b on the re-stated row, R7's mutant RED once more (build-mut-r7 is on disk) | R7r | the rows' own lines; c0 printed per arm. |
| S6 | builder | RBD: section 4, unchanged | RD = O1 or O5; R7's verdict (A19). NOT S4b. | unchanged; then a 5-arm R7 SUBSET as development evidence that D9 moved nothing. |
| S5a | builder | unchanged | S6 | unchanged |
| S5b | builder | unchanged, plus G7's states ML-1 / ML-2 read through S4b's dump, plus H-10's bind-overlay state (SF4) | S5a, S4b | unchanged + G7 |
| D1 | builder | ONLY on O2: A20's one witness field (TEST-SERVER-only), the probe's reading of it (the two latencies per marker), SELFTEST cases for each sub-outcome | RD = O2 | Harmony repeats RD's launches with it (RD2). Proves WHERE the block is lost before anyone plans a fix. |

RD's DECISION TABLE (replaces PL:185-201; complete: every reading lands in exactly one row)
A VALID take: dial 0 settled; no gap in the take; not a device-event take (A5); at least 20 clicks found in both files;
at least 20 paired markers. B = the opened block. "One-valued" = the take medians span <= 128 samples; "two-valued" =
they span >= 384. RD is INCOMPLETE (one more launch; not an outcome) while fewer than 24 valid takes are on record
after the launches run, or while C is missing in more than half the valid takes of a launch. Tested in this order:
 O4 ANOMALY. Any of: a valid take whose clicks scatter more than 4 samples about c0; c0 negative or not a whole
    multiple of B; e' spanning more than 128 and less than 384; e' one-valued but C spanning more than 128 inside one
    launch (the rulers disagree); e' one-valued with a c0 >= B take, but the median e' more than 128 from 1472; a
    shifted take that is also a device-event take.
    RULING: STOP. S6 stays stopped, R7 is not re-stated. To the architect with RD's lines.
 O2 PRODUCT. e' two-valued (whatever c0 does; the plan's O3 is this row).
    RULING: S6 stays stopped, R7 is not re-stated, no builder guesses a fix, and the Boris page says a timing
    defect is open in file takes. Stage D1 (A20), then Harmony repeats RD's launches with it (RD2). In the shifted
    takes:
    - tick latency + B -> the stamping TICK reads the bus a block late: the architect plans it in RecorderHost.
    - analysis latency + B, and C moves with e' inside a launch that holds both states -> the ANALYSIS is a block
      behind the audio in that state (everything audio-reactive is 10.7 ms late there, with or without a take):
      the architect plans the fix RED-first in test_analysis_sync_thread from RD2's loop entries
      (ringReadyAfterDrain, hopsDrained).
    - analysis latency + B, and C does NOT move with e' -> the analysis is where it should be against its own
      count; the take's AUDIO is a block off its first-sample origin: the tap (AudioTap.cpp:180-183 and its
      writer). To the architect.
    - analysis latency + B in a state that holds for whole launches (no launch with both states): C cannot split
      the last two; to the architect with RD2's lines and loop entries.
 O1 ARTEFACT, SHOWN. e' one-valued; at least one valid take has c0 >= B (so its e stands c0 above the others); C
    one-valued inside every launch; the median e' within 128 samples of 1472.
    RULING: INSTRUMENT ARTEFACT -- on some takes the file starts c0 samples into the take; markers and audio agree.
    No product change. R7r is built. NMAX = the largest c0 / B seen (at least 1); EREF = RD's median e'.
 O5 NOT REPRODUCED. All launches run; every valid take has c0 = 0 and e is one-valued.
    RULING (HD1, default): recorded as "not reproduced on this tree in 36 takes over 6 launches". R7r is built all
    the same -- the measured origin is the take's own audio, the right reference under every hypothesis, and A6's
    span and EREF clauses now fail a product one-block state BY NAME. NMAX = 1, EREF = RD's median e. Harmony's
    21-arm R7 run on the re-stated row is the second look (57 takes over 7 launches in all) and prints c0 per arm:
    an arm with c0 >= B on a VALID row is O1's evidence arriving late; an INVALID by the span or EREF clause is O2
    and S6 stops. S6 starts only after that verdict (A19).
ORDER. Dependencies only; Harmony schedules. D0 and S4b may start at once (HD8). RD after D0. R7r after RD. The owed
S3f rows after R7r. S6 after RD's outcome (O1 / O5) and R7's verdict. S5a after S6. S5b after S5a and S4b.
May S6 be built before the diagnosis is known? NO (H-12 stands; SC-7).
What Harmony runs herself, in order (replaces PL:290-294):
 1. First quiet window (no builder building, no lane app running): G1's [timing] x3 with A8's record.
 2. After D0: RD, then `--rd-verdict`; the outcome, NMAX and EREF into rulings-bf2.md.
 3. After S4b (not quiet-dependent): G1, G1-RED, R13 and its RED, R14 and its RED, SELFTEST.
 4. After R7r, quiet machine (R7 alone is about 23 minutes): R5 x5, G6, R7's 21 arms + R7b, R7's mutant RED.
 5. After S6: its own rows; the 5-arm R7 SUBSET (development evidence).
 6. The final gate list on the merged tree as RBD: section 4 says, with R13 and R14 added to G4's list.

## 5 GATE ROWS changed or added (pre-registered; Harmony copies gate strings ONLY from here and from RBD: section 5)
R7  (changed again; stage R7r; REPLACES the two sentences named, in RBD: section 5 row R7; supersedes PL:297-311).
    REPLACE "Per arm: paired marker errors (probe-step3's T2 math);"
    WITH "Per arm: the click grid's origin c0 is MEASURED in the take's own audio before the probe deletes the
     asset. For every click k found in both files, o_k = (the onset frame of click k in the take's asset WAV) -
     (the onset frame of click k in the source click WAV); onset = the first sample whose magnitude is above half
     full scale, hits within 100 samples collapsed to the first. c0 = the median of the o_k. Paired marker errors =
     (marker stamp - the take's first sample - c0) - the nearest multiple of the click interval (probe-step3's T2
     math on that origin);"
    REPLACE "VALID when every arm pairs >= 100 markers and drops <= 4."
    WITH "VALID when, for every arm: it pairs >= 100 markers and drops <= 4; the take has no gap (take.json audio
     "gaps" is empty); at least 100 clicks are found in both files and every o_k lies within 4 samples of c0; c0 =
     k x B, B = the opened device block (GET /api/debug/audio_devices "opened" "buffer_size", read in that arm), k
     a whole number from 0 to NMAX; and, over the run: the 0-arm medians span <= 256 samples, the 100-arm medians
     span <= 320 samples, and every 0-arm median lies within 256 samples of EREF. NMAX and EREF are the two numbers
     in rulings-bf2.md beside RD's outcome. A broken clause is named on the verdict line (for example "INVALID: arm
     7 has 1 gap", "INVALID: arm 4 c0 = 2048 = 4 blocks > NMAX 1", "INVALID: 0-arm medians span 512 samples",
     "INVALID: arm 2 median 1984 is 512 from EREF 1472"); an INVALID row blocks and goes to the architect."
    BAR, the ONE extension, the 0.33 ms SE limit, the finding rule and the mutant-app RED (FAIL near +100: the
    mutation moves markers, not audio): UNCHANGED.
    INFO gains, per arm: "c0 <n> samples = <k> blocks (spread <s>, <m> clicks)", "gaps <g>", and "clicks carrying
    two markers, and the gap between the two".
    RED (SELFTEST, synthetic takes, the probe's real functions): (1) an arm whose asset origin is 512 with every
    error + 512 reads the same value as an origin-0 arm; (2) one click 9 samples off its origin -> INVALID naming
    that clause; (3) origin 300 -> INVALID (not a block multiple); (4) origin 4 blocks with NMAX 1 -> INVALID;
    (5) one gap -> INVALID; (6) c0 = 0 in every arm and every error + 512 in ONE 0 arm -> INVALID "0-arm medians
    span"; (7) c0 = 0 and every error + 512 in ALL arms -> INVALID "from EREF"; mutated copies -- c0 ignored, and
    each clause removed in turn -- each exit 1 with a DIFF on its case.
R7b (changed; R7r; in RBD: section 5 row R7b). REPLACE "g_k = (gesture stamp - the take's first sample) - the
    click-grid position nearest marker k" WITH "g_k = (gesture stamp - the take's first sample - c0) - the
    click-grid position nearest marker k, the grid on the same measured origin (c0 of that arm)". "B = the median
    paired marker error over ALL 0 arms of the run" now reads R7's re-stated errors. BAR and RED: UNCHANGED.
RD  (new; D0; a DIAGNOSIS run, never a gate line; supersedes PL:316-320). Six times, n = 1..6:
      PROBESYNC_ROWS=R0,R7,R6 PROBESYNC_RD=1 PROBESYNC_R7_ARMS=0,0,0,0,0,0 PROBESYNC_R7_SECONDS=12 \
      PROBESYNC_APP=<WT>/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app \
      bash <WT>/.harmony/probe-sync.sh <OUT>/rd-<n>
    then   <python> <WT>/.harmony/probe-sync.py --rd-verdict <OUT>/rd-1/.../probe-sync.json ... (every launch run)
    Per take: "INFO  RD take <i>: c0 <c0> samples = <q> blocks (spread <s>, <m> clicks); e <e> / e' <e1> samples
    over <n> markers; C <c> samples; block <b>, rate <r>, opens <o> (+<d>); gaps <g>; hops with applied != 0: <h>".
    Per launch: "INFO  RD launch: <a> valid takes; c0 = 0 in <x>, >= 1 block in <y>; e' span <s> samples; C span
    <t> samples". No "PASS  R7" or "FAIL  R7" line is printed in RD mode; R6 and R0 print as always.
    Verdict (offline): "RD outcome O<n> (<N> valid takes in <L> launches; c0 = 0 in <x>, >= 1 block in <y>; e'
    span <s>; C span <t>; NMAX <k>; EREF <e>)" or "RD INCOMPLETE (<reason>)", by section 4's table.
    Its RED: SELFTEST synthetic launches reading O1, O2, O5, INCOMPLETE and each clause of O4; mutated copies
    -- c0 not subtracted (the O1 case reads O2), O1 without "a take with c0 >= B" (the O5 case reads O1), the
    spread clause removed, C ignored (the rulers-disagree case reads O1) -- each exit 1.
R13 (new; S4b; production mode, TEST-SERVER build; supersedes PL:321-328).
    POST /api/debug/binding_action {"action":"syncNudge","input":"key"|"note"|"cc","stepMs":<int>,
    "value":<number>}; anything else -> 400. It builds a Binding VALUE (never added to the manager) and posts the
    REAL MainComponent::handleBindingAction(binding, value) to the message thread. "value" is the number the action
    callback hands the handler: 1 / 0 for a key press / release, velocity / 127 for a note, the signed TICK count
    for a CC (V13).
    Start: POST /api/sync/set {"ms":0}, settled. Each step: the POST, then GET /api/sync "targetMs" reads the wanted
    value within 200 ms ("still N" = it reads N 200 ms after the POST).
      1  key,  stepMs 1,  value 1      -> 1
      2  key,  stepMs 1,  value 0      -> still 1
      3  key,  stepMs -1, value 1      -> 0
      4  note, stepMs 1,  value 0.0079 -> 1          (velocity 1 of 127)
      5  cc,   stepMs 1,  value 3      -> 4
      6  cc,   stepMs 1,  value -5     -> -1
      7  cc,   stepMs 1,  value 0      -> still -1
      8  POST /api/sync/set {"ms":500}, settled; key, stepMs 1, value 1   -> still 500
      9  POST /api/sync/set {"ms":-500}, settled; key, stepMs -1, value 1 -> still -500
    then /api/sync/set back to 0 before R6.
    PASS line: "PASS  R13 the handler case moves the dial: 9 of 9 steps". A failing step prints
    "FAIL  R13 step <n>: targetMs <got>, want <want>".
    RED: the scratch app build-mut-r13, MUTANT line 1 (the handler's nudge called with -deltaMs) ->
    "FAIL  R13 step 1: targetMs -1, want 1". `grep -rc MUTANT src tests` on the real tree = 0 afterwards.
    It does NOT claim: the decode of a CC's value (unit: the S4 Relative case), the key and MIDI entry points (unit:
    T5, T6, the S4 cases), a real key press (Boris, 6.1).
R14 (new; S4b; production mode, TEST-SERVER build; REPLACES PL:335-337's refusal bar).
    POST /api/debug/midi_learn with {"op":"open"} | {"op":"close"} | {"op":"select","label":"<a target's label>"}
    | {"op":"midi","cc":<n>,"value":<v>,"channel":<c>} | {"op":"midi","note":<n>,"velocity":<v>,"channel":<c>}.
    open / close = the function the Shortcuts menu's MIDI Learn item calls (W:MainComponent.cpp:7587-7596), only
    when the state differs; select = selectAt(the centre of the target with that label), 400 on an unknown label;
    midi = the overlay's own handleIncomingMidiMessage(nullptr, message), called on the HTTP thread (the overlay
    posts to the message thread itself, as it does for a device). GET /api/debug/midi_learn, answered on the message
    thread: {"ok","active","waiting","selected" (a label or ""),"title","last","w","h" (the overlay's size),
    "targets":[{"label","x","y","w","h","label_w","binding"}]} -- "title" from titleText(); "last" = the overlay's
    last-message text; "binding" = the tag paint() draws on that target ("Note 36", "CC 21" or ""); "label_w" =
    the label's width at the painted font.
    States, each read by a GET after the POST:
      0  open                                   -> active true, waiting false
      a  select "Master Signal"                 -> waiting true, selected "Master Signal",
                                                   title "Send a MIDI note or CC..."
      b  midi cc 21 value 100 channel 1         -> waiting false, selected "", "Master Signal" binding "CC 21",
                                                   last "CC 21 val=100 ch=1"        (CONTROL: the route delivers,
                                                   and a CC attaches where it may)
      c  select "Sync +1 ms"                    -> waiting true, selected "Sync +1 ms", title "Send a MIDI note..."
      d  midi cc 21 value 101 channel 1         -> waiting true, selected "Sync +1 ms", "Sync +1 ms" binding "",
                                                   "Master Signal" binding "CC 21", last "CC 21 val=101 ch=1"
      e  midi cc 22 value 5 channel 1           -> as d, last "CC 22 val=5 ch=1"
      f  midi note 36 velocity 100 channel 1    -> waiting false, "Sync +1 ms" binding "Note 36", "Master Signal"
                                                   binding "CC 21"
      g  close                                  -> active false
    PASS line: "PASS  R14 MIDI learn refuses a CC on a Sync target and changes nothing else; a note attaches: 8 of
    8 states". A failing state prints "FAIL  R14 state <id>: <field> <got>, want <want>".
    RED: build-mut-r13, MUTANT line 2 (learnMidiCC with its refusal removed) -> "FAIL  R14 state d" (the Sync
    target shows "CC 21" and "Master Signal" shows ""). The control arm b is what makes d's "still waiting" mean a
    refusal: without b passing, d is not evaluated and the row FAILS.
G1  (changed; RBD: section 5 row G1). Add to the suite list: "S4b: test_binding_sync_nudge (bindingIsLive;
    learnMidiCC refused / accepted; a note of any velocity; three key-downs)". Add: "`ctest -N` prints the number
    APP-INVENTORY.md:31 states, which is 1330 + the S4b cases named in the lane report; any mismatch is a FAIL."
    The plan's "test_beat_lead (the same printed lines as before the removal)" is NOT added (A18).
    [timing] paragraph: every sentence UNCHANGED. Add: "The gate record carries, for each of the three runs, the
    ten arm lines, the five pair lines, the pre-run `ps` top five, and whether the run shows the LOW signature
    (every 37-arm overhead, mean - 37, at least 0.10 ms above every 38-arm overhead, mean - 38). A failing run
    with nothing above 20 % in `ps` blocks, as already ruled; its record names the signature."
G7  (changed; S5b; adds to H-10's MIDI-learn state; REPLACES PL:335-337).
    ML-1: POST /api/debug/midi_learn {"op":"open"}; capture and dump. Bars from the dump: every target's bounds lie
    inside the overlay's; no two targets' bounds intersect; every "label_w" <= its "w" - 4.
    ML-2: then {"op":"select","label":"Sync +1 ms"}; capture and dump. Bars: "waiting" true, "selected"
    "Sync +1 ms", "title" "Send a MIDI note...", that target's "binding" "".
    Both captures go to the five-seat panel. A refused CC is R14's row, not G7's.
G4  (changed; the merge list at RBD:670): R13 and R14 are added.
SELFTEST (changed): + RD's cases (stage D0); + R7's origin and validity cases (stage R7r); exit 1 on every mutated copy.
TQ  : no row. Its idle half is G1's [timing] record above (A8).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; replaces PL:340-347)
 6.1 Shortcuts: put one key on "Sync +1 ms" and another on "Sync -1 ms". Tap each a few times.
     Expect: the SYNC number moves by one per tap and stops at +500 / -500. Then HOLD one of them for two seconds.
     Expect: the number keeps moving while you hold. Wrong: no move, more than one per tap, the wrong direction,
     or nothing while holding. (Only his hand can show the hold: the rig sends no real key.)
 6.2 MIDI learn: click "Sync +1 ms". The top line reads "Send a MIDI note...". Hit a pad.
     Expect: the target shows "Note" and a number, and the pad now moves SYNC by one.
     Click "Sync +1 ms" again and turn a knob, or press a controller button that is not a pad.
     Expect: the screen keeps waiting and nothing is attached; the small line shows "Last: CC ..."; a knob that
     was already attached to something else keeps what it had. Wrong: the knob attaches, or another control loses
     its knob.
 Do 6.1 and 6.2 in one sitting: a shortcut is forgotten when the app quits unless it is exported (SF7).
 Nothing in P2, P3, P4 or P6 is visible to him.

## 7 BORIS QUESTIONS (each has a default; nothing waits)
 Q1 (replaces PL:349-351) Sync earlier and Sync later can be put on a keyboard key or on a controller pad. Knobs,
    faders and some controller buttons talk to the app in a different way, and those cannot be put on Sync in this
    version: when you use one in MIDI learn, the screen just keeps waiting. Is a key or a pad enough?
    DEFAULT: yes. (If the control you want will not attach, tell us which controller and which control. Either
    kind is its own piece of work: the app cannot tell a button from a knob by itself.)
 Q2 When you hold a Sync key down, the number keeps moving at your Mac's key-repeat speed, the way a held arrow
    key does. Do you want that, or exactly one step per press however long you hold?
    DEFAULT: it keeps moving.

## 8 HARMONY'S DECISIONS (each has a default)
 HD1 O5 (the state is not reproduced in six launches). DEFAULT: section 4's O5 ruling -- build R7r, run the 21-arm
     R7 on it, then S6. This lifts H-12's hold on the 21-arm run without the state having been seen again: her
     call. ALTERNATIVE: six more launches of RD first (14 more minutes; S6 waits).
 HD2 S6 beside the owed R7 run. DEFAULT: no -- R7's verdict first (A19). ALTERNATIVE: start S6's builder after RD's
     outcome, provided the binary R7 runs on is built at the R7r head and nothing S6 writes reaches it.
 HD3 The BeatLead dead branch. DEFAULT: not removed; the debt-ledger entry of A18. ALTERNATIVE: its own stage S4c,
     deleting-only, PL:267-276 as its spec, landed BEFORE her live R4 / R4b / R5 rows, never in the keys context and
     never while S6 builds (both touch AnalysisThread.cpp).
 HD4 The title "Send a MIDI note..." on a Sync target (A12). DEFAULT: built; the panel judges ML-2.
     ALTERNATIVE: the title unchanged -- then the waiting screen invites a CC it will ignore, and R14 c / G7 ML-2
     read "Send a MIDI note or CC...".
 HD5 An Absolute-CC Sync entry in a loaded bindings file. DEFAULT: left as today -- loaded, inert, saved back
     unchanged; no screen can make one. Harmony constraint: no bound control that silently does nothing -- this
     default reads the constraint as "none that the app can be made to create"; a file written by hand is outside
     it. ALTERNATIVE (the strict reading): refuse it at load; then a round-trip case and a mutant for the engine
     guard must be added (MI-3, GA-8).
 HD6 [timing]'s loaded arm (four shell loops, five runs). DEFAULT: not run (A8). ALTERNATIVE: once, as INFO.
 HD7 NMAX and EREF: written by Harmony with RD's outcome, by A6's rule, before R7's gate run.
 HD8 S4b beside D0 / RD. DEFAULT: yes, in its own build directory, never building during a quiet-window run.
     ALTERNATIVE (if two builders at once are not wanted -- adoption item 3): D0, RD, then S4b; nothing else moves.
 HD9 RD's early stop. DEFAULT: stop as A3's rule allows.
 HD10 Where A7's detector item and A8's regime record are filed (debt ledger or notebook). DEFAULT: the ledger.

## 9 SIDE FINDINGS
 SF1 (V5) The plan's HD rests on a wrong premise for this JUCE: a wholly missed first block shifts the file by a
     block and loses nothing. It has HA's signature; RD covers it; no action.
 SF2 (A6) R7's dbar cannot see a one-block state that lasts a whole launch -- true of the row since D3, not new.
     The EREF clause closes it.
 SF3 (V10) timing-run4.log leaves the low regime between its sixth and seventh arm; the plan's F17 lists it as
     wholly low. The M0 lane report's "load, not the merge" (bf2-delta.md:91) is likewise stronger than its data.
 SF4 (V12) H-10's BIND-overlay state has no route in any spec either. S5b's packet must name one (open + dump, the
     shape of R14's GET). Not this ruling's row.
 SF5 (V14; INFERRED as there) Every key binding re-fires on key-repeat today (Tap Tempo, toggles, clip triggers).
     Pre-existing, outside this lane; Q2's answer may be wanted app-wide. Filed for the keys lane.
 SF6 (V3) The REST record path arms the tap before it starts the file. Starting the transport inside the arm would
     make that start sample-exact; under O1 no reader of a take can see the difference. NOT this lane (PL:204-205).
 SF7 (V11) Bindings are never saved or loaded by themselves: a shortcut made in a session is gone at quit unless
     exported through the Shortcuts menu. Pre-existing, VERIFIED by grep at the pin (the only callers are the two
     file choosers). Boris may not expect it for a Sync key; it belongs on his page beside 6.1.
 SF8 (V7) R7's 100 arms split about evenly over two blocks, so their medians move by +-64 samples from arm to arm.
     Part of R7's SE is this lattice effect, not drift (RBD: K4).
 SF9 (V16) APP-INVENTORY carries route counts in two places (line 29, "41 registered REST routes", and line 31's
     registrations). S4b's builder updates the one its counting command produces and says which.

## 10 RISKS (the strongest counterargument first)
 K1 "Under O5 this ruling re-states a gate around a state nobody has understood, then lets S6 change the take
    start." It loses because the re-statement removes exactly one term -- where the file starts in the take --
    and everything a product fault could move stays in e'; the two new clauses turn a recurrence into a named
    INVALID instead of a wider SE; the pin stays buildable, so a later finding can still be diagnosed on the tree
    without D9; and 57 takes over 7 launches precede S6. What remains: a product state rarer than about 1 take in
    60 that is also absent from R7's gate run would reach the merge unseen -- and then show as an INVALID in the
    final gate list, which runs R7 again. Cheapest refuting test: RD itself; one shifted take settles it.
 K2 The strongest alternative to "the probe's origin": a product state latched per launch. The saved takes cluster
    (3 of 3 in one launch, never seen leaving the state), which fits a latch better than a per-take race. A late
    file start explains a latch only if the arm-to-play window differs from launch to launch (INFERRED: disk and
    allocation timing; nothing here measured it). That is why RD counts launches, why O1 needs both rulers to
    agree, and why R7 gains the EREF clause. It is not ruled out by reading.
 K3 The witness ruler is new and unrun: its marker-to-hop pairing could slip, or the two counters' offset could
    step inside a launch. It can only ADD a stop (O1 needs both rulers to agree), never a pass. SELFTEST covers
    the pairing; D0's development launch prints C on two real takes.
 K4 The click detector on a real 16-bit asset (plan K3) is unverified here. Fact M2 measures it before RD; a
    spread above 4 samples stops D0 for a different estimator rather than widening the clause.
 K5 Fork (b) may cut the control Boris wants. Q1 states the loss; a "no" costs a design item, not a patch.
 K6 "A held key repeats" is INFERRED from the JUCE source, not run (V14). Only 6.1 shows it. If his Mac does not
    repeat, 6.1's hold line fails harmlessly and Q2 is moot.
 K7 R14 adds test-only surface: a route pair and two seams in the overlay. The alternative was a gate that could
    not fail, and G7's H-10 states need the same dump.
 K8 [timing] can block the final gate on an evening when the player runs. The action is stated (A8); the trigger
    is not known; the bars are untouched.
 K9 The EREF clause ties R7 to this machine's detector latency (1472 samples at the pin). It is "as RD measures it
    on this tree"; a lane that changes the detector re-measures it. A false INVALID costs one round trip; a
    missed whole-launch state would cost a late show.
 K10 S4b is the largest builder context left (fix, two routes, two rows, one scratch app). If it runs long, the
    routes and rows split off as S4b-2; the order does not change.
 K11 The BeatLead debt stays in the code (A18): a reader of BeatLead.cpp:194 still meets two equal arms, with a
    comment that says so.

FACTS HARMONY MUST MEASURE (a run decides; the ruling for each outcome)
 M1 RD's outcome. O1 / O5 -> R7r, the owed rows, S6. O2 -> D1, RD2, architect. O4 -> architect. Section 4.
 M2 The click detector on a real asset (D0's development launch). Spread <= 4 samples on both takes -> RD may run.
    Above -> STOP before RD; to the architect (an origin by correlation instead).
 M3 R7's verdict on the re-stated row: dbar, SE, c0 per arm. RBD: fact M3's rule, plus the new INVALID clauses.
 M4 [timing] quiet x3: the pairs, the sd, the LOW signature yes / no. A8's three outcomes.
 M5 The strings R14 expects ("Master Signal", "Sync +1 ms", "Send a MIDI note or CC...", "CC 21 val=100 ch=1",
    "Note 36") are read from the source at the pin (MainComponent.cpp:7637-7646, MidiLearnOverlay.cpp:94, :135-142,
    :248-249). The first run confirms them. A mismatch -> STOP and report; a builder does not edit the row.
 M6 `ctest -N` after S4b = the inventory line = 1330 + the listed cases.
 M7 (Boris only) a held key repeats; a CC-sending button's silent refusal reads acceptably (6.1, 6.2, Q1, Q2).

SUMMARY: 29 attacks ruled (18 ACCEPT, 10 PARTIAL, 1 REJECT); 20 amendments (A1-A20); stages D0, RD, S4b, R7r, S6,
S5a, S5b, and D1 only on O2; 2 Boris questions with defaults; 10 Harmony decisions with defaults; 7 facts to measure.

STATUS: DONE
