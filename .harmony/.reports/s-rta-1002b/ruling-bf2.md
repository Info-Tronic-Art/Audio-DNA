# RULING bf2 -- Architect ruling on the blind council (lane bf2, s-rta-1002b)

Author: Architect (Fable), 2026-10-02. Tree read: main fa9604d (the plan was written on 5e47d17; every line cited below was
re-read on fa9604d this session). Inputs: plan .harmony/.reports/s-rta-1002b/plan-bf2.md; seat papers VERBATIM in
.harmony/.reports/s-rta-1002b/attack-bf2-papers.md; Boris's words .harmony/boris-feedback-backlog.md:10-11 and
.harmony/binding-decisions.md:578-581. Labels: VERIFIED = read or ran it; INFERRED = derived, not run.
Scratch runs (session scratchpad, not in the repo; model of Appendix A in the predicted regime):
  beatlead_sim.py    -- Appendix A as written vs the AM-1 lead slew, tempo steps at a 500 ms lead.
  beatlead_rebase.py -- HOLD vs immediate re-base on a tracker correction (phase 0.30 -> 0 at a 0.8-beat lead).

Seat ids: SA = SIGNAL/AUDIO, LA = attack-bf2-live, GA = gates-tests-attack-bf2.
Totals: 32 attacks -- 29 ACCEPT (5 of them with the seat's fix replaced), 3 REJECT (SA3, SA6, LA2). 19 amendments.

=====================================================================================================================
## 1. RULING PER ATTACK

SA1 [MUST] ACCEPT -> AM-1.  VERIFIED by scratch run: Appendix A as written HOLDS 46 hops (491 ms) on 174 -> 87 BPM and
  109 hops (1,163 ms) on 200 -> 60 BPM at a 500 ms lead, and JUMPS forward up to 0.72 beat in one hop on 87 -> 174
  (published per-hop advance up to 33.8x a hop). Even 128 -> 126 dips the published rate to 0.26x for a hop. With
  AM-1 (lead in beats slewed at <= 0.25 x the per-hop advance): 0 held hops, published advance within [0.75, 1.25]x
  in all four steps. The seat's fix (re-base on a bpm change) is NOT adopted: a tempo change is not a deliberate frame
  move, and a re-base publishes the dip as a backward fold jump (beatInBar - 1) to the fold consumers
  (ConnectionShaper.cpp:16-22, OscillatorSignal.h:59, EnvelopeSignal.h:58) and mints phantom beats (SA6 evidence).

SA2 [MUST] ACCEPT, fix replaced -> AM-5(b), AM-11.  R1 is measured on the analysis thread's own clock and R2's +-3 ms
  is under-powered (a 16.7 ms frame quantum). The 1 ms claim moves to a unit test against an INDEPENDENT clock on
  every hop (~280 hops per arm, interleaved 37/38 pairs), where it is measurable; R2 gets a fixed +-8 ms bar (it
  proves the render path moves by D, not 1 ms) and one non-frame-multiple arm (253). ">= 200 clicks per arm" is not
  adopted (100 s per arm; unnecessary at +-8 ms).

SA3 [SHOULD] REJECT (the plain-words disclosure is adopted in AM-15).  Evidence: (1) the drain stamp IS the delivery
  time for hops delivered together, and the ring is drained on every loop iteration (sleep <= 1 ms,
  AnalysisThread.cpp:89-93 today; every iteration in the amended loop), so nothing waits in the ring before the stamp
  -- R1a (AM-9) witnesses it live; (2) the wake / drain jitter is the same at every D, so the MEAN moves exactly 1 ms
  per 1 ms step -- AM-5(b) proves it against an independent clock; (3) stall bursts happen identically at D = 0 today
  (the loop processes whatever it pulls), so the line does not make them worse; absorbing them needs a drift / gap
  re-anchoring estimator whose own failure (drift) fills or empties the line -- unrequested machinery; (4) a single
  flash lands on the display's frame grid whatever the stamp precision, so sample-clock stamps cannot move one flash
  by 1 ms either.

SA4 [SHOULD] ACCEPT (the seat's alternative: a Boris question with a test) -> AM-12.  The finding is right that F7 is
  not neutral. Derivation (INFERRED, from E6 + the gate): arrival A, dial D, display lag V, the crowd hears the beat
  at H = A + D + V. A tap applied at the next processed hop (plan F7(a)) shows at press + V: right when Boris hears
  what the crowd hears (press ~ H), D early when he hears the music first (press ~ A: headphones, a booth speaker fed
  from the mixer). The seat's delayed tap shows at press + D + V: right for press ~ A, D late for press ~ H. Which is
  right depends on where Boris listens -- only he knows. The plan is also inconsistent: LATE treats a tap in the show
  frame, EARLY leads it like a detected beat.

SA5 [SHOULD] ACCEPT -> AM-8(a)(c).  VERIFIED: rate check + setInputBandwidthHz run at pull time
  (AnalysisThread.cpp:77-87); provenance is written at process time (:344-345). Readers today: REST only
  (ApiServer.cpp:880-881) plus the band gating inside SpectralFeatures -- low impact, but the fix is two fields and
  removes a Pitfall 29-class mislabel.

SA6 [SHOULD] REJECT (the F3 wording contradiction is real and fixed in AM-2).  (1) Deferring S2: Boris asked for
  "-500 to +500" and "a control we have in case we want to have all early" (binding-decisions.md:578-581); S2 already
  ships alone after S1a (plan:448, :453), so LATE never waits on EARLY. (2) Replacing holds with a bounded-rate slew:
  after AM-1 the only holds left come from the TRACKER moving its own beat position backwards -- an auto-mode realign
  from the first half of a beat (BPMTracker.cpp:234-236, :253-258) or a detection-driven beatInBar that lags or misses
  the phase wrap (BPMTracker.cpp:343-346, :377-382). Pushing through them means either publishing the dip with the
  counters absorbed -- scratch run beatlead_rebase.py: +1 PHANTOM totalBeatCount event per correction, which
  Autopilot, beat-randomize, the slideshow and routines all act on (MainComponent.cpp:4334-4336, :4496-4498,
  Autopilot.cpp:65, RoutineEngine.cpp:542) -- or inventing a new clock-driven beat position in the riskiest code, for
  a mode Boris calls rare. HOLD gives 0 phantom events (same run: 14 held hops for a 0.3-beat correction).

SA7 [SHOULD] ACCEPT -> AM-5(a)(b), G1 rule (0 skipped).

SA8 [SHOULD] ACCEPT (the seat's alternative: document and test the intended semantics) -> AM-13.  Subtracting D from
  EVERY point is deliberate: a gesture made at wall t sat beside the visuals of audio t - D, so keeping gestures,
  onset markers and the tempo map in one stamp domain makes a with-audio replay reproduce the SCREEN (RecorderHost.cpp
  :453-458 stamps every point from one clock sample; :553-566 positions the replay). Live-stamped gestures would sit
  D away from the markers they were performed against on screen. R7b pins the domain.

SA9 [NIT] ACCEPT -> AM-8(d) + AM-5(a) startup case.

LA1 [MUST] ACCEPT, fix replaced -> AM-12.  Same finding as SA4. The seat's fix (realigning requests wait D; R9 bar
  "downbeat D after the press") becomes conditional stage S6, built only if Boris picks it in Q6; the default
  (lands where you press) is gated BLOCKING. Correction to the evidence: the dial does NOT stop moving the clock after
  a tap -- the beat clock runs in sample time per processed hop (BPMTracker.cpp:221-223), so a later dial move still
  shifts it; only where a NEW anchor lands differs.

LA2 [MUST] REJECT (under the default; folded into S6 if Boris picks "later").  Evidence: a Resync is applied by the
  tracker at the end of the NEXT processed hop (BPMTracker.cpp:357-362). At any constant D the analysis processes one
  hop per arrival (each due D after its own arrival), so the first snapshot carrying the Resync is published within
  ~11 ms of the press (~14 ms while the dial slews up) -- the same window as D = 0; there is no D-sized window. The
  message-thread reset (MainComponent.cpp:5637-5638) re-primes beatCrossings_ (OnsetPulse.h:31 -- the next consume()
  returns 0) on totalBeatCount, which applyResync never rewinds (BPMTracker.cpp:566-578; realignPhaseToZero only
  increments, :253-258). The slideshow keeps its own pulse (MainComponent.cpp:4334-4336, not reset); quantize (:4652)
  and beat snap (:4679-4683) read the snapshot at the trigger and hold no reset state. Under S6 the window DOES exist,
  and S6 carries the seat's deferral (AM-12(d)).

LA3 [MUST] ACCEPT -> AM-5, AM-9, AM-11.

LA4 [SHOULD] ACCEPT (the seat's second branch: longer holds kept, told to Boris, "never a freeze" dropped; plus the
  live frozen-hop gate) -> AM-2, R5.  The first branch ("re-base sooner, holds <= 2 hops") is rejected on SA6's
  evidence (phantom beats / backward fold jumps).

LA5 [SHOULD] ACCEPT, fix amended -> AM-14, Q2.  An always-visible indicator, placed inside the TopBar tempo cluster's
  EXISTING 64-px reserved slot (TopBar.cpp:578-586; TopBar 34 px tall, MainComponent.cpp:2561) so it costs no width on
  an already over-full bar (plan E14); display-only. Boris's own rule rejects "Hidden signal state"
  (BORIS_DECISIONS.md:412). Launch prompt rejected (a modal at launch). Click-to-zero rejected (one stray click
  mid-show would silently drop the venue's offset).

LA6 [SHOULD] ACCEPT the finding, flush REJECTED -> AM-8(b)(c).  The plan must say what happens; the answer is "play
  out, never flush": the queued audio is audio that really played; a flush makes NO hop due for D ms (the bus
  freezes on its last snapshot) and drops D of real audio from the visuals. Today's code does not flush the SPSC ring
  on a source switch either (AudioEngine.cpp:154-175 flips only useInputForAnalysis).

LA7 [SHOULD] ACCEPT, fix amended -> AM-15.  Q3 now offers exactly the seat's alternative (jump for typed values and
  venue switches, glide for -/+ and drag) and carries the frame note; the default stays glide. The +-10 % cap is
  rejected (500 ms would take 5 s = plan F3(c), rejected as sluggish). Correction: the slew is 0.75x / 1.25x (25 %);
  0.70-1.30 was only R3's tolerance band.

LA8 [SHOULD] ACCEPT -> AM-16.  VERIFIED: VideoRecorder.h:34-35 "Audio is NOT included in the recording".

LA9 [SHOULD] ACCEPT -> AM-6, R1a, R8 blocking.  Note: two publishes inside one render frame already happen at D = 0
  whenever the device delivers >= 2 hops per callback; onsetCount covers them (Pitfall 30).

LA10 [NIT] ACCEPT -> AM-18(d).

GA1 [MUST] ACCEPT -> AM-10.
GA2 [MUST] ACCEPT -> AM-11.
GA3 [MUST] ACCEPT, fix amended -> AM-9.  Absolute anchor = the unit test's independent push clock (AM-5(b)) plus a live
  ring-backlog / iteration witness (R1a). The transport-time comparison is not adopted: it needs new instrumentation
  inside the audio callback for no extra power, since R1a already proves nothing waits in the ring before the stamp.
GA4 [MUST] ACCEPT -> AM-3.
GA5 [MUST] ACCEPT -> AM-5(c), G1 rule.
GA6 [SHOULD] ACCEPT -> AM-4.  Extra finding (VERIFIED by reading the tracker): in AUTO mode beatInBar advances ONLY on
  detected beats (BPMTracker.cpp:343-346, :377-382; advancePredictedBeat runs only in the predicted regime, :243-246),
  so one missed detection leaves it a whole beat behind the phase-wrap count. The plan's synthetic "0-2 hop lag" model
  is too optimistic; AM-2 restates the hold guarantee so it holds for the real tracker.
GA7 [SHOULD] ACCEPT -> AM-6.  Also VERIFIED: test_integration_pipeline never links AnalysisThread.cpp
  (tests/CMakeLists.txt:244-284; it re-implements the pipeline), so it never guarded the loop the plan restructures.
GA8 [SHOULD] ACCEPT -> AM-18(a)(b)(c).  The narrowest width is now a number: 120 px (kMinPanelWidth,
  MainComponent.h:497); the minimum window is 1280 x 720 (Main.cpp:53).
GA9 [SHOULD] ACCEPT -> AM-17.
GA10 [SHOULD] ACCEPT -> AM-7.  (Sanitizers are off by default, cmake/Sanitizers.cmake:19-24, so a counting operator new
  is safe in the default build.)
GA11 [SHOULD] ACCEPT, fix amended -> AM-18(c), R10.  TEST-ONLY routes drive the REAL widgets in the live app (no OS
  mouse synthesis); hold-repeat stays headless (setRepeatSpeed config) plus Boris check 2.
GA12 [NIT] ACCEPT -> AM-19.  VERIFIED: wallClockSeconds / timestamp written only at AnalysisThread.cpp:339-341; no src/
  reader.
GA13 [NIT] ACCEPT -> AM-18(d).

=====================================================================================================================
## ARCHITECT RULING (s-rta-1002b)

The amendments below OVERRIDE plan-bf2.md wherever they differ. Everything not mentioned stands as written.

### AMENDMENTS

AM-1 (SA1) EARLY lead slews in beats [S2: I1, I5, Appendix A]
  - SyncOffset.h adds `constexpr double kLeadSlewPerBeat = 0.25;` (the lead, in beats, moves at most 0.25 beat per
    beat -- the same 0.75x / 1.25x band as the dial's own slew, F3).
  - BeatLead state adds `double leadBeats_ = 0.0` (the APPLIED lead, >= 0). Per hop: deltaT = (bpm > 0) ? l x bpm / 60
    : 0; aN = (512 / 48000) x bpm / 60 (the tracker's per-hop phase step, BPMTracker.cpp:222-223).
    Rule 1 (identity): not engaged and deltaT == 0 -> publish S unchanged; leadBeats_ = 0.
    Rule 2 (re-base hops, unchanged triggers): leadBeats_ = deltaT, then as written.
    Every other hop, BEFORE rule 3: leadBeats_ += clamp(deltaT - leadBeats_, -kLeadSlewPerBeat x aN,
    +kLeadSlewPerBeat x aN). simulate() and rules 3-4 use leadBeats_ wherever Appendix A says delta.
    Rule 4 (disengage) additionally requires leadBeats_ == 0.
  - BeatLead::leadBeats() feeds the witness (AM-9) and GET /api/sync ("leadBeats").
  - New T-L10 (test_beat_lead): predicted regime (manual), lead 500 ms constant, followExternalTempo steps 174 -> 87,
    87 -> 174, 200 -> 60, 60 -> 200 (no realign): 0 held hops; every hop's published fold advance in [0.75, 1.25] x aN
    (+-1e-6); leadBeats reaches 0.5 x bpm / 60 (+-1e-6) within ceil(|change| / (0.25 x aN)) + 1 hops of each step; the
    same bars with the dial slewing 0 -> 500 across a step. (Behavioural RED: Appendix A as written fails it -- 46 / 109
    held hops, scratch run.)

AM-2 (SA6, LA4) Holds stay ONLY for the tracker's own corrections; wording; guarantee; disclosure [S2]
  - Plan F3's "never a freeze, never a jump" applies to dial moves and (after AM-1) tempo changes. In EARLY, when the
    tracker moves its own beat position backwards (auto realign from the first half of a beat; beatInBar lagging or
    missing the phase wrap), rule 3 HOLDs the beat fields until the led position catches up. At D = 0 the same
    corrections show as backward jumps of the same size; a pushed-through position would mint phantom beat events
    (SA6 evidence).
  - T-L3's "no hold longer than 0.5 beat + 2 hops" is WITHDRAWN (false for the real tracker: a missed detection is a
    1-beat correction, GA6). T-L3 (50,000 randomized hops, tempo steps included) now asserts, per hop: (i) published
    totalBeatCount / totalBarCount never decrease; (ii) held => Q(target) < Q(F) or a counter would decrease (no
    spurious hold); (iii) not held => published fields == simulate(S'', leadBeats_) (no stale publish); (iv) on every
    hop where the raw fold did not move back, Q(target) advanced >= 0.75 x aN - 1e-9 (no hold outlasts its catch-up);
    (v) barsSinceResync() never decreases except at a Resync epoch; (vi) view bar increments - raw bar increments in
    [0, number of re-bases]. INFO in the log: held-run length histogram.
  - Disclosure (section 8 check 5 + artifact page + the Boris message): "With the dial EARLIER and BPM on Auto,
    beat-synced motion pauses briefly whenever the beat tracker corrects itself -- usually a few thousandths of a
    second, up to one beat if the tracker misses a beat."
  - Plan risk R2 text: "a long hold" now means "a hold as long as the tracker's own correction".

AM-3 (GA4) Non-beat fields untouched, proven [S2]
  - New T-L8 (test_beat_lead): 50,000 random hops at random leads 0..500 ms moved through SyncSlew, random non-beat
    contents: the published snapshot, after its 9 beat fields (beatPhase, totalBeatCount, beatInBar, downbeatDetected,
    barPhase, barCount, totalBarCount, resyncBarOrigin, phrasePhase) are copied back from the input, memcmp-equals the
    input over sizeof(FeatureSnapshot). BeatLead.h carries a static_assert block naming those 9 fields (offsetof); it
    writes no other field.
  - Live: the witness (AM-9) records CRC32 of the snapshot with those 9 fields zeroed, before and after BeatLead (R4,
    R5 bar: equal on every hop).

AM-4 (GA6) T-L9: the REAL tracker through BeatLead [S2]
  - test_beat_lead T-L9 drives a real BPMTracker via processRawBPM (BPMTracker.h:186), feedDownbeatFeatures,
    feedSilenceDetection, requestResync, setManualBPM, setManualMode, scripted: lock at 128 BPM; detected beats with
    +-2-hop jitter; one missed detection; one extra detection; a Resync; a Tap; manual on / off; 2 s of silence; a
    downbeat relock (bass pattern moved one beat); a phrase reset (structural state 2); a hysteresis tempo change
    128 -> 140 -- through BeatLead at leads 100 and 500 ms. Asserts AM-2's T-L3 invariants and T-L7's level contract.
  - It logs the histogram of hops between a phase wrap and the matching beatInBar advance (validates plan E7) and
    asserts <= 2 hops while jitter <= 2 hops and no detection is missed. If that fails, S2 does not merge until Harmony
    rules.

AM-5 (SA2, SA7, LA3, GA5) Timing tests that cannot pass vacuously [S1a; I3, I13]
  (a) AnalysisThread: the loop body becomes a private `int serviceOnce(double nowMs) noexcept` (rate check, DRAIN,
      slew, PROCESS every due hop; returns hops processed); run() = while (!threadShouldExit()) { if
      (serviceOnce(now) == 0) sleep min(msUntilFrontDue, 1 ms) } (D = 0 -> today's sleep(1)). Public
      `int serviceOnceForTest(double nowMs)` jasserts !isThreadRunning(). The drain loop stops when the line is full
      (the full line's front is then due -- lineOverruns + 1).
      test_analysis_sync_thread [gate] (fake clock, deterministic): D = 0 processes each hop in the call that drains it;
      D = 40: processed at the first call with now >= stamp + 40.000, not at +39.999; slew 0 -> 500 gives applied 250 at
      +1000 ms and 500 at +2000 ms; a full line forces its front out (lineOverruns + 1); startup with a saved 500: no
      publish before now >= first stamp + 500, queuedHops 46-48 at that moment (SA9).
  (b) [timing] (real thread, real clock, INDEPENDENT clock): a feeder thread pushes 512-sample blocks every 10.667 ms
      and records each block's push-complete time; the test thread polls the bus every 0.25 ms and maps each new
      snapshot to its block via snapshot.timestamp (samples processed); lag = observed - pushed, EVERY hop. Arms (3 s
      each, after the slew settles): [37, 38, 37, 38, 37, 38] and [0, 150, 0, 150]. Bars: each adjacent 37 / 38 pair
      mean(38) - mean(37) in [0.7, 1.3] ms; each 150 arm mean - the mean of its neighbouring 0 arms in [148, 152] ms;
      >= 99 % of published hops observed; no hop lost or duplicated (timestamp and onsetCount continuity). NO SKIP
      PATH: an arm whose lag SD exceeds 2 ms FAILS the test with the measured SD (G1's re-run rule then applies).
  (c) Every RED is behavioural: the builder first adds stub APIs (setSyncTargetMs that does nothing, the syncOffsetMs
      field, BeatLead::apply that does nothing, controller stubs ...) so each new test COMPILES on the base behaviour
      and FAILS on its assertions; the builder report lists the failing assertions per test (e.g. [timing]: 150-arm
      shift ~0 ms; T-L2: published phase == raw; T-L10: holds > 0 under Appendix A as written; I12: start anchor
      1,000,000 not 995,200). A compile-only RED is not accepted.
  (d) Plan F1(c)'s "resolution comes from the stamps (~0.1 ms)" (plan:137) is withdrawn. Replacement: "per hop, the
      delay jitters by the loop's wake (< 1 ms, witnessed); the MEAN moves 1 ms per 1 ms step (AM-5(b))".

AM-6 (GA7, LA9) Goldens; R8 blocking [S1a]
  (a) [golden] black-box: a real AnalysisThread at D = 0 fed in lockstep (4 hops to fill the window, then push one hop
      and wait for its publish) with a deterministic 21 s synthetic signal (fixed-seed noise + 120 BPM clicks + a 220 Hz
      tone under an amplitude envelope); FNV-1a-64 over a field-by-field serialization of every published snapshot
      (every field that exists on the base commit; syncOffsetMs excluded). The builder records the hash by running the
      same test on a worktree of the lane's base commit and commits the constant with that provenance; the branch must
      match bit for bit. A mismatch blocks until the builder reports the first differing hop + field and Harmony rules.
  (b) [golden-fake]: serviceOnceForTest with a fake clock at D = 150 -> the same hash; at D = -100 (after S2) -> the
      same hash over the non-beat fields.
  (c) R8 is BLOCKING (the plan's decision rule named only R0-R7).

AM-7 (GA10) Runtime zero-heap guard [S1a]
  - NEW target test_analysis_sync_alloc: NOT given apply_sanitizers and added only when ADNA_SANITIZE is empty. It
    replaces global operator new / new[] (all forms) with counting versions that count only when pthread_self() equals
    the analysis thread's native id (juce::Thread::getThreadId(), captured after start). A real AnalysisThread fed in
    real time: 1 s warm-up, then 2 s counting windows at D = 0, 250 and (after S2) -250, each window containing a
    profile dump (every 500 hops, AnalysisThread.cpp:360-381) and a dial slew. Bar: 0 allocations. The builder also
    runs it on the base commit (stub API) and records that count in the report (expected 0; a non-zero baseline is
    reported, never hidden).

AM-8 (SA5, LA6, SA9) Provenance per hop; play out, never flush; startup [S1a]
  (a) Each delay-line entry stores the source rate it was resampled under. processHop calls
      spectralFeatures_->setInputBandwidthHz(AnalysisResampler::bandwidthFor(rate)) when the hop's rate differs from the
      last processed hop's, and writes snap->sourceSampleRate = the hop's rate. The rate check, resampler_.setSourceRate
      and the logLinef stay at the drain (test_log_line_lint's count of 4 holds). AnalysisResampler gains
      `static float bandwidthFor(double hz)` (the formula inputBandwidthHz() uses, AnalysisResampler.h:28;
      inputBandwidthHz() calls it). At D = 0 this is byte-identical (AM-6 golden).
  (b) No flush, ever: on a source-mode switch, a device change, a transport stop or seek, the queued audio plays out;
      the visuals follow the switch D later, like all audio.
  (c) [gate] cases: 40 hops queued at 48 kHz then a rate change to 44.1 kHz (a test-owned std::atomic<double> rate
      cell) -> the 40 queued hops publish sourceSampleRate 48000 and the 48 kHz band mask, later hops 44100; hops pushed
      before and after a simulated source switch are all processed, in order, none dropped.
  (d) Docs (analysis.md): with a saved D the first publish comes D after the analysis starts (the line fills; the first
      value snaps, F3(a)); at D = 0 startup is unchanged.

AM-9 (GA3, LA9) Absolute anchor; witness additions [S1a; S2 adds the lead fields]
  - SyncWitness gains a second ring (1024 entries) written once per loop iteration: {startMs, ringReadyAfterDrain
    (RingBuffer::availableToRead, RingBuffer.h:58), hopsDrained, hopsProcessed, lineSize}. Per-hop entries gain
    trackerRequestSeq and sourceRate. GET /api/debug/sync_witness?since=N returns both rings plus the app's nowMs
    (probes align their clock through it, +-RTT/2).
  - S2 adds to the per-hop entry: barsAdvance, leadBeats, deltaTargetBeats, beatX, barX, held, deficitBeats
    (Q(F) - Q(target)), nonBeatCrcBefore, nonBeatCrcAfter (AM-3).
  - Live bars: R1a. Unit: AM-5(b) measures from the feeder's push time (arrival), not the analysis thread's stamp.

AM-10 (GA1) R3 metric made exact -- see gate R3 (windowed least-squares slope against witness processMs).

AM-11 (GA2, SA2) R2 and the decision rule -- see gate R2 (control pair, 253 arm, +-8 ms) and the DECISION RULE
  (the "within its own noise ... 2 of 3" rule, plan:531-532, is deleted).

AM-12 (SA4, LA1, LA2) Human tempo anchors
  (a) New Boris question Q6 (below). DEFAULT = plan F7(a): Tap / Resync land where you press, at any D.
  (b) R9 becomes BLOCKING and asserts the default (gate R9).
  (c) Plan F7 text adds: "Under EARLY, a tap is led like any beat; under LATE it lands at the press. This matches a
      performer who hears what the crowd hears. A performer who hears the music before the crowd (headphones, a booth
      speaker fed from the mixer) sees taps D early under LATE -- Q6."
  (d) CONDITIONAL stage S6 "Taps go through the dial" -- built ONLY if Boris answers Q6 "later". Outline (an architect
      addendum fixing the take-recorder rule is REQUIRED before a builder starts):
      - Realigning requests (Tap = setManualBPM; Resync) carry their press time P
        (juce::Time::getMillisecondCounterHiRes(), the delay line's clock) and are applied at the first processed hop
        whose arrival stamp >= P. Tempo VALUES (typed BPM, set_bpm, Link, manual / auto) stay immediate. Requests with
        Origin::Replay apply at the next hop (the Player already fires them in analysis time).
      - BPMTracker gains `void setHopArrivalMs(double)` (analysis thread; processHop calls it before process()) and a
        fixed SPSC queue (capacity 16, no allocation) of {kind, bpm, pressMs, seq} for stamped requests; runPipeline
        applies every queued request with pressMs <= hop arrival, in order; appliedRequestSeq_ advances only past
        applied requests (Pitfall 48 meaning unchanged). Queue full -> the oldest applies at once (counted; expected 0).
      - LA2's deferral: applyTempoCommand's resync branch (MainComponent.cpp:5637-5638) zeroes beatCounter_ /
        beatCrossings_ only at the first tickFeaturePipeline whose snapshot trackerRequestSeq >= the posted seq read
        right after requestResync().
      - Take recorder (addendum must settle): Tap / Resync points need input-domain stamps so a replay realigns on the
        same audio.
      - Tests: tracker unit (not applied before P, applied at the first stamp >= P, two taps inside D both apply in
        order, seq contract, queue-full path); serviceOnceForTest at D = 100 (a Resync posted at t publishes at
        >= t + 100); live R9 variant (= D +- 15 ms).

AM-13 (SA8) Take stamps: screen-fidelity domain, stated and tested [S3]
  - docs/claude/recording.md: "with a LATE offset every take point (gestures, onset markers, tempo map) is stamped in
    show time (delivered - D): a gesture is stored beside the audio whose picture was on screen when it was made, so a
    with-audio replay in any venue reproduces the screen."
  - Gate R7b.

AM-14 (LA5) NEW item I14 -- TopBar sync indicator [S5]
  - Files: src/ui/TopBar.h/.cpp; src/MainComponent.cpp (one listener line). A juce::Label `syncIndicator_`, 9 pt,
    bounds (tempoLabel_.getRight() + 2, area.getY(), 60, 12) -- above trackerStateLabel_, inside the existing 64-px
    tempo slot (TopBar.cpp:578-586), zero added width. `void setSyncIndicator(int targetMs, const juce::String& venue)`
    (message thread; repaint only on change, Pitfall 57); text "SYNC +42" / "SYNC -30"; hidden at 0; tooltip "Sync
    dial: visuals 42 ms later -- venue 'Warehouse'. Set it in the BPM panel." Colours from the existing palette
    (kTextSecondary, kTextPrimary); never orange #ff4500 (override-only), never red. Display-only. Driven by
    SyncOffsetController's change notification (no polling).
  - RED: NEW tests/test_topbar_sync_indicator.cpp: hidden at 0; "SYNC +42" at 42; "SYNC -30" at -30; tooltip names the
    venue; bounds inside the slot, overlapping neither tempoLabel_ nor trackerStateLabel_ (set the TopBar visible
    explicitly, Pitfall 34).
  - Merge note: bf7 edits TopBar.cpp (bar readout); this edit is additive (one member, one setter, one bounds line).

AM-15 (LA7, SA3) Q3 wording; what "1 ms" means [S5 + artifact page]
  - Q3 as below (offers jump-for-typed-and-venue; default glide).
  - Disclosure line (artifact page, the Boris message, docs/claude/analysis.md): "the screen draws a new picture every
    16.7 ms (60 Hz), so one 1 ms click moves the AVERAGE timing by 1 ms; any single flash still lands on the next
    picture."

AM-16 (LA8) Video recording and Syphon [docs]
  - docs/claude/integration.md (Sync dial section) + artifact page: video recordings and Syphon carry the picture
    exactly as shown, the venue's delay included; the video recorder has no audio track (VideoRecorder.h:34-35), so
    nothing else shifts.
  - Cross-lane note for bf1 (record-to-clip): a clip recorded at +D carries the audio-driven motion D later than the
    input; its start / stop (snapped to the published bar) and its later beat-snapped playback use the same published
    grid, so it stays self-consistent.

AM-17 (GA9) Gates can never write Boris's settings [S1b; I7, I13]
  - SyncOffsetController reads AUDIODNA_SYNC_TEST once at construction, compiled only under AUDIODNA_TEST_SERVER:
    unset = normal; "off" = persistence off for the session (starts from the saved venue); an integer N = persistence
    off and the session starts at N ms (clamped) on the current venue, in memory only.
  - Loading never writes: a missing or corrupt "sync" key reads as Default 0 in memory; the file is written only after
    a user change with persistence on. I6 / I7 tests add: construction with a missing key and with a corrupt key leaves
    the file bytes unchanged.
  - probe-sync.sh launches with `--env AUDIODNA_SYNC_TEST=off`. The R8 wrapper exports
    ADNA_OPEN_ENV=AUDIODNA_SYNC_TEST=0, and each of the four R8 probes' launch line gains
    `${ADNA_OPEN_ENV:+--env "$ADNA_OPEN_ENV"}` (written in S1b; no change when unset).
  - The real ~/Library/Audio-DNA/settings.json sha256 is recorded before every launch and after every quit in G4, G6 and
    around each R8 probe; any change = FAIL. The TEST-ONLY POST /api/debug/sync_persist stays (in-session toggle).

AM-18 (GA8, GA11, GA13, LA10) G7 made measurable; live widget row; Q1 timing [S5]
  (a) TEST-ONLY GET /api/debug/sync_ui_dump -> {panelWidth, widgets: [{name, x, y, w, h, text, textWidth, fits}],
      topBarIndicator: {visible, text, x, y, w, h}}; textWidth via juce::GlyphArrangement::getStringWidthInt with the
      widget's own font.
  (b) test_sync_panel adds: a refreshFromModel during a drag leaves the bar under the mouse, and drag end re-pulls the
      model value.
  (c) TEST-ONLY POST /api/debug/sync_ui extends {"show": ...} with {"type": "<text>"} (the real value label's text-edit
      path + Enter), {"click": "plus" | "minus"} (Button::triggerClick on the real buttons), {"rightClick": "value" |
      "bar"} (the real reset path). Gate R10.
  (d) Q1 goes in the FIRST Boris message; S5 builds the default (the grammar bar) if Q1 is unanswered when S5 starts.
      The artifact page's C1 caption: "a long bar, not a round knob -- your rule: no round knobs".

AM-19 (GA12) NEW tests/test_snapshot_time_lint.cpp [lint] [S1a]: fails if any file under src/ other than
  src/analysis/AnalysisThread.cpp and src/analysis/FeatureSnapshot.h contains `wallClockSeconds`, or contains
  `.timestamp` / `->timestamp` on a FeatureSnapshot (scan for `snap.timestamp`, `snap->timestamp`, `Snap.timestamp`,
  `snapshot.timestamp`, `frameSnap_.timestamp`; the allow-list names the writer lines). Pins plan E3.

### BUILD STAGES (final; one builder context each)

  S1a "Later -- engine": I1, I2, I3 (+ AM-5, AM-6, AM-7, AM-8), I4 (+ AM-9 S1a fields), I8-minimal, AM-19, I13 LATE rows
      (R0, R1, R1a, R2 LATE arms, R3, R6, R8, R9). Range 0..+500. Invisible; at 0 nothing changes. SHIPS ALONE.
  S1b "Venues & remote control": I6, I7 (+ AM-17), I8-full, the R8 launch passthrough (AM-17). Needs S1a. SHIPS ALONE.
  S2  "Earlier": I5 (+ AM-1, AM-2, AM-3, AM-4; T-L1..T-L10), AM-9 S2 witness fields, kMinMs -> -500, rows R2 (-100 arm),
      R4, R4b, R5. Needs S1a. SHIPS ALONE.
  S3  "Takes stay aligned": I12 (+ AM-13), rows R7, R7b. Needs S1a. SHIPS ALONE.
  S4  "Keys & MIDI": I9. Needs S1b. SHIPS ALONE.
  S5  "The control on screen": I10, I11 (+ AM-18), I14 (AM-14), G7 (+ C11), R10. Needs S1b; Q1 asked before it starts;
      shows the negative half once S2 is in.
  S6  CONDITIONAL "Taps go through the dial" (AM-12(d)): only if Boris answers Q6 "later"; needs S1a + S3 and an
      architect addendum first.
  Order: S1a -> {S1b, S2, S3} -> {S4, S5}; S6 only on Q6 = "later".
  Docs (plan section 6) stand, plus: AM-2 disclosure (analysis.md), AM-8(b)(d) (analysis.md), AM-13 (recording.md),
  AM-15 (analysis.md), AM-16 (integration.md), AM-12(c) (analysis.md "human anchors").

### FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings only from here)

Rig for every live row (plan:480-484, unchanged): a test-server build (AUDIODNA_BUILD_TEST_SERVER=ON) in its own build
dir; the live lock held (/tmp/audiodna-live.lock, exactly one Audio-DNA); `ps` checked for processes above 20 % CPU
before any timing or perf verdict; launch only `open -g [--env ...] <app> [--args --test-mode]`; never an Output window
(end-of-run Quartz full-window-list check); captures by Quartz window id only; HTTP clients send `Connection: close`;
graceful osascript quit; whole run <= 2 h.

G1 ctest, default build (ADNA_SANITIZE empty): every suite passes with 0 failed and 0 skipped (a skipped test is a
   failure). New / changed suites -- S1a: test_sync_slew, test_analysis_delay_line, test_analysis_sync_thread ([gate],
   [timing], [golden], [golden-fake]), test_analysis_sync_alloc, test_sync_witness, test_feature_bus (syncOffsetMs at
   offset 332 + publish / read round trip), test_snapshot_time_lint; S1b: test_sync_venues, test_app_settings ("sync"
   keeps "outputs" + "milkDropPresetDir"), test_sync_offset_controller, test_osc_sync; S2: test_beat_lead (T-L1..T-L10),
   test_bpm_stabilization [epoch]; S3: test_recorder_host [host][sync]; S4: test_binding_sync_nudge; S5:
   test_timing_window_content, test_sync_panel, test_topbar_sync_indicator. Unchanged and green: test_tempo_start
   [lint], test_log_line_lint (AnalysisThread.cpp still 4 `logLinef(`), test_integration_pipeline. A [timing] failure:
   if the pre-run `ps` check recorded a process above 20 % CPU, stop it or wait it out and re-run once (the re-run
   stands); otherwise it blocks.
G1-RED Builder evidence, checked by Harmony before each merge: for every new test, a first run against stub APIs on the
   base behaviour that compiles and FAILS on its assertions, listed per test in the builder report (AM-5(c)). A
   compile-only RED is not accepted.
G2 TSan (separate build dir, .harmony/probe-tsan-unit.sh): test_analysis_sync_thread [tsan] (a second thread calls
   setSyncTargetMs 10,000 times during 2 s of analysis), test_sync_witness (1 writer x 2 readers, 200,000 entries, no
   torn entry), test_sync_offset_controller [tsan]: 0 reports.
G3 Zero-heap: (a) [lint] source-text scan of SyncSlew.h, AnalysisDelayLine.h, BeatLead.h (and BeatLead.cpp if any),
   SyncWitness.h: no `new `, `malloc`, `std::vector`, `push_back`, `std::string`, `juce::String`; AnalysisThread.cpp
   has exactly 4 `logLinef(`. (b) test_analysis_sync_alloc: 0 operator-new calls on the analysis thread in each 2 s
   window at D = 0, 250 and (after S2) -250, after a 1 s warm-up, each window containing a profile dump and a dial slew;
   the base-commit count recorded in the builder report.
G4 .harmony/probe-sync.sh -- PRODUCTION mode (no --test-mode), launched with `--env AUDIODNA_SYNC_TEST=off`; sha256 of
   ~/Library/Audio-DNA/settings.json recorded before launch; GET /api/sync logged right after launch.
   R0 (S1a) GET /api/sync answers 200 with targetMs, appliedMs, venue, hopDelayMs, queuedHops, lineOverruns (base
      commit: 404).
   R1 (S1a, mechanism) any input (the mic is enough). Interleaved arms [0, 100, 0, 253, 0, 500, 0, 37, 38, 37, 38, 0].
      Per arm: set, wait until |appliedMs - targetMs| < 0.1 for 0.5 s, sample hopDelayMs 20 times over 2 s -> arm
      mean. Bars: mean(D) - the mean of its neighbouring 0 arms = D +- 0.8 ms for D in {100, 253, 500}; mean(38) -
      mean(37) in [0.6, 1.4] ms on both pairs; lineOverruns == 0 throughout; queuedHops at 500 in [46, 49].
   R1a (S1a, absolute anchor; iteration witness during R1's 0, 253, 500 arms and R3): (i) on every iteration with
      lineSize < 64, ringReadyAfterDrain < AnalysisResampler::inputNeededFor(512); (ii) iteration start-to-start
      <= 10.7 ms on >= 99.5 % of iterations; (iii) maxHopsPerIteration <= ceil(buffer_size x 48000 / (sample_rate x
      512)) + 1, buffer_size and sample_rate from GET /api/debug/audio_devices ("opened").
   R2 (S1a; -100 arm after S2) end to end: the click track (.harmony/gen-click-wav.py, 120 BPM = 24,000 frames at
      48 kHz) played by POST /api/perf/record {"name":"probesync","audio":false,"audioFile":<wav>}, 30 s per arm,
      preview visible (render_frame once to attach GL, as probe-onset-render.sh does). Arms [0, 0, 100, 0, 253, 0, 500,
      0] then, after S2, [-100, 0]. Per onset: lag = render pulse frameMs - its onset hop's stampMs (witness, paired by
      onsetCount); per arm: median lag. Bars: control pair (arms 1-2) |median difference| <= 6 ms; each D arm: median -
      the mean of its two neighbouring 0-arm medians = D +- 8 ms; the -100 arm: |that difference| <= 8 ms; every arm:
      render pulse delta == onsetCount delta and onsetCount delta >= 54 of 60.
   R3 (S1a) slew, manual 120 BPM (POST /api/set_bpm {"bpm":120}): 0 -> 500 then 500 -> 0. Metric: least-squares slope
      of the published beat position (totalBeatCount + beatPhase) against witness processMs, over every 200 ms window
      inside the slew with 100 ms trimmed at each end, divided by 2.0 beats/s. Bars: every window in [0.70, 0.80]
      (0 -> 500) and in [1.20, 1.30] (500 -> 0); appliedMs goes from its first value > 0 to 500 (and from below 500 to
      0) in 2.00 +- 0.05 s.
   R4 (S2) EARLY, manual 120 BPM: settled at -100, every hop (published - raw) beatPhase mod 1 = 0.200 +- 0.005; settled
      at -500, published totalBeatCount - raw = 1 with phases equal +- 0.005; slews 0 -> -500 and -500 -> 0: R3's
      metric in [1.20, 1.30] and [0.70, 0.80]; scripted run -100 -> Resync -> -500 -> Resync -> 0 -> +100: published
      totalBeatCount and totalBarCount never decrease, every published totalBarCount increment coincides with
      downbeatDetected rising and the level stays true for one published beat (a Resync may raise it without an
      increment), published - raw bar increments in [0, 2]; every hop: nonBeatCrcBefore == nonBeatCrcAfter.
   R4b (S2) EARLY tempo steps, manual, at -500: POST /api/set_bpm 174, then 87, 174, 200, 60, 200, 20 s each (no
      realign). Bars: witness held == false on every hop; per 200 ms window, the published slope / the raw slope over
      the same window in [0.70, 1.30]; leadBeats reaches 0.5 x bpm / 60 +- 0.001 within ceil(|change| / (0.25 x aN))
      + 2 hops of each step.
   R5 (S2) EARLY, AUTO, click track: arms [0, -100, 0, -500, 0], 60 s each. Bars: R4's monotonicity, level-contract
      and CRC bars on every hop; onsets: |median lag shift vs the neighbouring 0 arms| <= 8 ms; holds, every hop:
      held => deficitBeats > 0 or a counter would decrease; not held => published fold == target fold +- 1e-4 beat;
      where the raw fold did not move back, the target fold advanced >= 0.75 x aN - 1e-4. INFO: held-hop fraction,
      longest held run (ms), number of runs.
   R6 (S1a+) settings untouched: sha256 of ~/Library/Audio-DNA/settings.json identical before the first launch and
      after the last quit of G4 and around each R8 probe; GET /api/sync shows persist false throughout G4.
   R7 (S3) take alignment: click takes with onset markers (audio:true) at D = 0 and D = 100; probe-step3's T2
      marker-vs-click math: the median error at 100 equals the one at 0 within 1 ms (on S1a alone it differs by
      ~100 ms -- the live RED for S3).
   R7b (S3) gesture domain: in the D = 100 take the probe fires a REST gesture right after it sees onsetCount reach
      n0 + k (k = 10, 20, 30); in the saved take each gesture's stamp - the stamp of marker k is in [0, 2,880] samples
      (0-60 ms at 48 kHz).
   R8 (S1a+, BLOCKING) regressions at D = 0: probe-onset-render.sh, probe-tempo-start.sh, probe-beatclock.sh,
      probe-downbeat-level.sh PASS as on main; from S1b on each is launched with
      ADNA_OPEN_ENV=AUDIODNA_SYNC_TEST=0; settings.json sha256 unchanged around each.
   R9 (S1a, BLOCKING, Q6 default) at +100 and +253, manual 120 BPM, POST /api/resync three times per value, >= 3 s
      apart: the first witness hop whose trackerRequestSeq exceeds its pre-POST value publishes the Resync
      (beatInBar 0, beatPhase 0, resyncBarOrigin == totalBarCount) and its processMs is <= 50 ms after the POST
      returned (clocks aligned through the witness reply's nowMs). [Only if S6 is built: = D +- 15 ms instead.]
   R10 (S5, TEST mode, scratch settings) live widgets via POST /api/debug/sync_ui: {"type":"+42"} -> GET /api/sync
      targetMs 42 within 200 ms; {"click":"plus"} -> 43; {"click":"minus"} twice -> 41; {"type":"abc"} -> still 41 and
      the value box reads "+41"; {"rightClick":"value"} -> 0; GET /api/debug/sync_ui_dump: the TopBar indicator reads
      "SYNC +41" while at 41 and is hidden at 0.
   DECISION RULE: every row of the stage PASS = merge-ready; any FAIL blocks. One whole-probe re-run is allowed only
      when the pre-run `ps` check recorded a process above 20 % CPU (stopped or waited out first); the re-run's numbers
      stand. There is no "within its own noise" and no "2 of 3" rule.
G5 (S1b) Venues and remote control -- TEST mode, scratch AUDIODNA_SETTINGS_FILE seeded with "outputs" and
   "milkDropPresetDir". Launch with no "sync" key -> the file's sha256 unchanged 5 s after launch (loading never
   writes); same with a corrupt "sync" value. REST: create "Club A" (copies the current 0), set 42, create "Club B",
   set -30 (after S2; 0 before), rename "Club A" -> "Warehouse", remove "Club B", select "Default" -> the file's "sync"
   JSON equals {"version":1,"current":"Default","venues":[{"name":"Default","ms":0},{"name":"Warehouse","ms":42}]}
   exactly and the seeded keys are byte-identical. Refusals (duplicate name, empty name, removing the last venue,
   unknown venue) -> lastError set, file unchanged. Graceful quit + relaunch -> GET /api/sync shows the same current
   venue and offsets. OSC (python3 UDP socket to 127.0.0.1:8000): /audiodna/sync 25.0 -> targetMs 25;
   /audiodna/sync/nudge -3.0 -> 22; /audiodna/sync 900.0 -> 500. Keys / MIDI: unit tests only (G1).
G6 (S1a; -500 arm after S2) Perf A/B -- production, mic input, preview visible, `ps` checked first, settings.json sha256
   recorded around it. Interleaved A/B/A/B, >= 5 runs per arm, 20 s each: D = 0 vs D = 500 (and vs -500 after S2).
   Metrics: analysis cpuLoad (DSP %) and render frameMs (/api/status). Bar: |delta of means| <= max(0.3 percentage
   points, 2 x the within-arm run SD). A delta inside the run-to-run drift is INFO, not a verdict.
G7 (S5) VISUAL WORK GATE, before Boris sees anything. Captures in TEST mode with scratch settings, every state set by
   REST incl. TEST-ONLY POST /api/debug/sync_ui; the main window by Quartz window id, cropped to the TimingWindow
   (C1-C10) or to the TopBar tempo cluster (C11); at two widths: the TimingWindow's default width in a 1280 x 720 window
   (dividers 0.22 / 0.50, MainComponent.h:498) and 120 px (kMinPanelWidth, MainComponent.h:497). C1 first launch
   (Default, 0, "In step with the sound coming in"); C2 Warehouse +42; C3 -30 (after S2); C4 +500 / -500; C5 the venue
   menu open (3 venues, tick on the current, Delete in warning red); C6 New venue dialog; C7 Rename dialog; C8 a refusal
   caption (duplicate name); C9 the Routing / Oscillators tabs (placeholder still painted, no sync widgets); C10 the
   panel after a REST change while it is visible; C11 the TopBar at +42 ("SYNC +42") and at 0 (absent).
   Measurable bars from GET /api/debug/sync_ui_dump at each capture: default width -- label column 90 px, value box
   50 px, bar height 14 px, section header 24 px tall, every text fits its bounds; 120 px -- no two widgets overlap,
   the value box and -/+ lie fully inside the panel, only the label column and the caption may be cut; the TopBar
   indicator inside the tempo slot, overlapping neither the BPM number nor the tracker state.
   Critic panel in parallel (visual-design, UX, graphic-design, logic, interaction-logic), each given the decoded PNGs
   + plan I11 + BORIS_DECISIONS.md Inspector Grammar / Section Headers / Rejected list + this written checklist, and
   each returns PASS / FAIL per item: grammar widths; colours #888 / #e0e0e0; warning red only for Delete; no round
   control; no orange #ff4500; caption wording; header style; placeholder tabs unchanged; the panel follows the model
   (C10); the indicator legible and not alarming. Any FAIL blocks unless Harmony records why it is taste-only; blocking
   findings are fixed and re-shot. Then the artifact page Boris opens: the captures, one plain-words paragraph per
   state, C1's caption "a long bar, not a round knob -- your rule: no round knobs", questions Q1-Q6 with their
   defaults, and the disclosures (AM-2, AM-15, AM-16).
Stage -> gates: S1a: G1, G1-RED, G2, G3, G4 (R0, R1, R1a, R2 LATE arms, R3, R6, R8, R9), G6 (0 vs 500).
   S1b: G1, G1-RED, G2, G5, G4 (R0, R6, R8 with passthrough). S2: G1, G1-RED, G3(b) -250 window, G4 (R2 -100 arm, R4,
   R4b, R5), G6 (-500 arm). S3: G1, G1-RED, G4 (R7, R7b). S4: G1, G1-RED. S5: G1, G1-RED, G7, G4 R10.

### FINAL BORIS QUESTIONS (plain words; each has a default so nothing waits)

  Q1 (send first -- the on-screen stage S5 waits for it, else builds the default) You said "dial". Your earlier rule
     says no round knobs, and every control is a long bar with a number box and - / + buttons. DEFAULT: that bar --
     type the number, click - or + for 1 ms, or drag the bar. Want a round dial instead?
  Q2 When you open the app it comes back on the venue you used last, and a small "SYNC +42" sits next to the BPM
     whenever the dial isn't at 0, so a leftover setting from the last room is easy to spot. DEFAULT: yes. Prefer the
     app to start at 0 every time?
  Q3 Big changes (switching venue, typing a new number) glide over up to 2 seconds -- beat-synced motion runs a little
     slower or faster while it settles -- instead of jumping; - / + clicks and dragging always glide (one click settles
     in a few thousandths of a second). DEFAULT: glide. Prefer typed numbers and venue switches to jump instantly?
     (The screen draws a new picture every 16.7 ms, so one 1 ms click moves the AVERAGE timing by 1 ms; any single
     flash still lands on the next picture.)
  Q4 Where should it live? DEFAULT: in the empty "BPM" panel right next to the preview (bottom row). Other choice: a
     small button in the top bar that opens it.
  Q5 A new venue starts from the setting you have right now (tune first, then save it under a name). DEFAULT: yes. Or
     should a new venue always start at 0?
  Q6 (NEW) With the dial set LATER (say +100), when you press Tap or Resync, should the beat land exactly where you
     pressed -- right if you hear the same sound as the crowd -- or 100 ms after you pressed, together with the
     music-driven flashes -- right if you hear the music before the crowd does (headphones, or a booth speaker fed
     straight from the mixer)? DEFAULT: where you pressed.
  Disclosures (no answer needed): with the dial EARLIER and BPM on Auto, beat-synced motion pauses briefly whenever the
     beat tracker corrects itself (usually a few thousandths of a second, up to one beat if it misses a beat). Video
     recordings and Syphon carry the picture exactly as shown, the dial's delay included.

### RESIDUAL RISKS (after this ruling)

  RR1 Q6's default can be wrong for a booth-monitor setup (taps D early for the crowd under LATE). Mitigation: asked
      outright; S6 is specified; R9 pins whichever answer is built.
  RR2 EARLY + AUTO on real music may hold often (missed detections are 1-beat corrections). Only the click track is
      gated (R5); T-L9 and R5's INFO numbers are the evidence; if Boris dislikes the pauses after trying EARLY, a
      clock-driven led beat position is the follow-up design (not built now: Boris calls EARLY rare).
  RR3 The [golden] hash can differ across main and the branch for a benign codegen reason. Rule: the builder reports
      the first differing hop + field; Harmony rules; never re-record the constant on the branch.
  RR4 The [timing] test is real-time; on a loaded machine it FAILS loudly (no skip). The G1 re-run rule applies only
      with a recorded CPU burner.
  RR5 TopBar collision with bf7 (bar readout) -- additive edit, merge note in AM-14.
  Strongest counter-argument to this ruling: it adds test machinery (serviceOnce, two goldens, an allocation counter,
  a second witness ring, five live rows) to a feature Boris tunes by ear. Why it loses: the core change restructures
  the analysis loop -- the app's most timing-sensitive code (Sacred Rules 2-3, Pitfall 48) -- and three seats
  independently showed the plan's gates could pass without testing anything (a SKIP that counts as a pass, delays
  measured on the gate's own clock, no D = 0 golden, a lint that cannot see AnalysisThread.cpp). Each addition is the
  cheapest check that can fail on one named defect.

STATUS: COMPLETE -- ruling-bf2 written 2026-10-02; 32 attacks ruled (29 ACCEPT, 3 REJECT: SA3, SA6, LA2); 19
amendments override plan-bf2.md; ready_to_build: yes (S1a now; Q1 before S5; Q6 decides conditional S6).
