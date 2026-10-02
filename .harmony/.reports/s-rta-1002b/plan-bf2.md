# PLAN bf2 -- one fine "sync" dial, saved per venue (s-rta-1002b)

Author: Architect (Fable), 2026-10-02, main 5e47d17. Read-only analysis; every claim about code cites file:line read
this session. Labels: VERIFIED = read / ran it; INFERRED = derived, not run.

---------------------------------------------------------------------------------------------------------------------
## (1) GOAL

One control, saved per venue and picked at soundcheck, that moves everything the music drives LATER (0..+500 ms: the
analysis hears the music later) or the beat-locked things EARLIER (-500..0 ms: beat clock / beat phase, BPM-synced
oscillators / envelopes, beat-snapped triggers, autopilot, beat uniforms -- loudness / onsets stay on time), in 1 ms
steps, with a typed value and -/+ fine-tune.

Boris, verbatim (.harmony/boris-feedback-backlog.md): "We need a nice clean way to adjust the delay or speeding ahead
for each room so we can set it per room. Like if the audio is slower than the video, this delays or advances signals
and bpm with one very sensitive dial."
Boris, verbatim (.harmony/binding-decisions.md:578-581): "default, -500 to +500 in 1 ms steps plus can enter in the
amount then click plus or minus to fine tune, we will rarely have an early delay so that is not a worry just a control
we have in case we want to have all early without loudness signals"
Defaults he accepted: venue profiles saved by name, picked at soundcheck; EARLY moves only beat-locked things; LATE
delays everything audio-driven.

---------------------------------------------------------------------------------------------------------------------
## (2) ESTABLISHED

E1 VERIFIED -- No sync / latency / offset control exists. grep of src/ for latency|syncOffset|audioOffset|avOffset|
   delayMs|offsetMs hits only unrelated comments (RoutinePad.cpp:177, VideoRecorder.cpp:259, SharedFrameSet.h:118,
   CompositorEngine.cpp:19, media/*).

E2 VERIFIED -- Every audio-driven consumer reads ONE bus, FeatureBus (seqlock, one writer, any readers, value copies;
   FeatureBus.h:8-37, read() FeatureBus.cpp:58-111):
   - render thread, once per frame: Renderer.cpp:401 `frameSnap_ = featureBus_.read()`, onset pulse :402-404, autopilot
     :553 and inactive decks :782, projectM playlist beat delta :594, compositor audio uniforms :734, legacy effect chain
     :856-863, procedural sources :1498-1500, clip beat-division speed :1817 / :1883;
   - message thread 120 Hz, MainComponent::tickFeaturePipeline (MainComponent.cpp:4030-4111): signals :4039, v1
     mappings :4051, macros :4061, take recorder :4080-4082, routines :4089-4091, ConnectionEngine :4108;
   - quantize / beat snap: MainComponent.cpp:4652, :4679-4683, :4861, :6105; slideshow / beat randomize beat deltas
     :4335-4336, :4497-4498;
   - TopBar beat wheel + bar readout (TopBar.cpp:316), SpectrumDisplay.cpp:17, AudioReadoutPanel.cpp:17, SignalBar,
     REST /api/bpm and /api/features (ApiServer.cpp:776-802, :804+).
   - projectM PCM and the waveform display come from AnalysisThread's own buffers, copied from analysisBuffer_ after
     each hop (AnalysisThread.cpp:383-407).
   => Delaying the AUDIO that enters the analysis moves every one of them, with zero consumer edits.

E3 VERIFIED -- No consumer extrapolates from the snapshot's time: timestamp / wallClockSeconds are written only by the
   writer (AnalysisThread.cpp:339-341; grep finds no reader), and wallClockSeconds is SAMPLE time
   (totalSamplesProcessed_ / 48000), not wall time.

E4 VERIFIED -- The analysis loop pulls one 512-sample 48 kHz hop through the resampler (AnalysisThread.cpp:90;
   AnalysisResampler.h:31-35: false until a whole hop exists), else sleep(1) (:91-93); appends it to the 2048 window
   (:100-118); runs 14 stages; publishes (:348). Hop = 512 / 48000 = 10.667 ms (AnalysisThread.h:46-50).

E5 VERIFIED -- The audio-callback -> analysis SPSC ring is 16384 floats (MainComponent.h:306): 341 ms at 48 kHz, 170 ms
   at 96 kHz. It cannot hold 500 ms, and the audio callback must stay memcpy-only (CLAUDE.md Sacred Rule 1).

E6 VERIFIED -- The beat clock advances per PROCESSED hop in sample time: phase += hop / period (BPMTracker.cpp:222-223),
   wraps count whole beats (:226-232), hard realigns use the half-beat count rule (:253-258). Tap / Resync / typed-tempo
   requests are applied at the analysis thread's NEXT processed hop (BPMTracker.h:132-149; Pitfall 48). So holding or
   bursting hop processing shifts the beat clock in real time without changing anything the tracker sees.

E7 VERIFIED -- In auto mode beatInBar advances on the DETECTED beat (scoreBeat, BPMTracker.cpp:365-402); in manual /
   silence it advances on the predicted phase wrap (advancePredictedBeat :260-275, called from updatePhase :243-246);
   bars count on the rising edge of the downbeat level (updatePhrase :515-523); Resync re-frames the bar without moving
   totalBarCount (applyResync :566-578). INFERRED from the two advance sites: in auto mode the beat count (phase wrap)
   and beatInBar (detected beat) can disagree for a hop or two. Matters only for EARLY (F2).

E8 VERIFIED -- Tap = setManualBPM (realign); typed BPM, REST / OSC set_bpm and Link go through the "link" / "manual"
   actions = followExternalTempo (rate only, never a realign) (MainComponent.cpp:5616-5648; REST :1922, OSC :2243, Link
   tick :4165). Link is tempo-only and not compiled into default builds (LinkSync.h:16-22; docs/claude/
   performance-controls.md "Tempo only, never the phase").

E9 VERIFIED -- The counters every slower reader diffs are monotone: onsetCount (FeatureSnapshot.h:103-116; OnsetPulse.h
   :22-28 treats a backwards jump as a reset), totalBeatCount (:128-137, Pitfall 42), totalBarCount with the downbeat
   LEVEL (:47-62, Pitfall 32), trackerRequestSeq (:139-144, Pitfall 48). Any design must keep them monotone across
   dial moves.

E10 VERIFIED -- FeatureSnapshot is 384 B with bytes 332..383 free (FeatureSnapshot.h:193-198); FeatureBus copies 96
   words (FeatureBus.h:127-133). One float at 332 costs nothing and needs no bus change.

E11 VERIFIED -- The take recorder stamps every point with the LIVE delivered-sample counter (RecorderHost.cpp:453-458
   sampleForClock = deliveredSamples; RecorderClock.cpp:26, :82-86); a with-audio replay positions the Player by the
   transport (RecorderHost.cpp:553-566). Pairing a delayed snapshot with that live counter is the time twin of the
   rate hazard Pitfall 29 describes.

E12 VERIFIED -- Test mode starts NO analysis thread; the TestServer is the bus writer (MainComponent.cpp:1859-1874).
   TestServer has no audio-injection route (TestServer.cpp:128-265). Production gates play a click WAV into the
   analysis through /api/perf/record {"audioFile": ...} (.harmony/probe-onset-render.sh:131-135; ApiServer.cpp:1599-
   1627) generated by .harmony/gen-click-wav.py. ApiServer (port 7070) runs in both modes (MainComponent.cpp:1902-1914).

E13 VERIFIED -- Machine settings live in settings.json through AppSettings (read-modify-write per key, message thread;
   AppSettings.h:3-38; keys today "milkDropPresetDir", "outputs" :18-19). A production launch uses the REAL
   ~/Library/Audio-DNA/settings.json; only a test-server build in --test-mode honours AUDIODNA_SETTINGS_FILE
   (MainComponent.cpp:70-91).

E14 VERIFIED -- The TimingWindow is a visible bottom-row panel beside the preview (default 22 %..50 % of the bottom
   width, MainComponent.h:498; laid out MainComponent.cpp:2743-2747; hidden only while the SignalBar is expanded
   :2597) whose BPM / Routing / Oscillators tabs are EMPTY placeholders (TimingWindow.cpp:51-65; APP-INVENTORY.md:63).
   The TopBar row is fixed widths end to end (TopBar.cpp:546-653); INFERRED by summing them: ~1,760 px, already more
   than a 1,512-pt 14-inch screen.

E15 VERIFIED -- Boris's standing UI rulings: "Inspector Grammar (everywhere): [triangle] [Label 90px right #888] [Value
   50px mono #e0e0e0] [-][+] [bar 14px]" (BORIS_DECISIONS.md:180-188); Rejected: "Circular knobs (any kind)" (:406),
   "Manual-only controls (no signal visualization)" (:413). CLAUDE.md UI Patterns: every slider is a ResettableSlider
   with setDefaultValue (UniversalParamControl.h:28-64); menus via showMenuAsync + withParentComponent.

E16 VERIFIED -- Bindings: the Action enum is APPEND-ONLY, saved as an int (Binding.h:24-46); a relative CC accumulates
   a private normalized value starting at 0.5 per CC (BindingManager.cpp:168-187) and never learns a value set
   elsewhere. Bindable targets are a synthetic global row (MainComponent.cpp:7436-7460).

E17 VERIFIED -- OSC callbacks run on the message thread (MainComponent.cpp:2205-2206); dispatch is by exact address
   (OscHandler.cpp:43-191).

E18 VERIFIED -- Text-scan lints pin AnalysisThread.cpp: the line `snap->trackerRequestSeq = bpmTracker_->
   appliedRequestSeq();` must precede `featureBusWriter_.publishWrite();` (tests/test_tempo_start.cpp:376-388); exactly
   4 `logLinef(` calls (tests/test_log_line_lint.cpp:116).

E19 VERIFIED -- HTTP-thread reads of message-thread state use a mutex-guarded status copy (RecorderHost::status(),
   RoutineEngine::status(); ApiServer.cpp:1737-1744, :2366-2368). CLAUDE.md is 24,002 B (wc -c).

---------------------------------------------------------------------------------------------------------------------
## (3) DESIGN FORKS

F1 -- Where the delay lives (the load-bearing choice)
 (a) Reader-side snapshot ring read at now - D (render + 120 Hz pipeline + every other reader). REJECTED: ~12 readers
     on 3 threads (GL, message, HTTP) must each convert or the bus read semantics change; 1 ms resolution needs interpolation between
     10.7 ms hops; Tap / Resync / typed tempo land at the tracker in real time and would then SHOW D late (after a
     Resync the visual downbeat is off by D) unless back-dated inside BPMTracker; projectM PCM and the waveform are
     separate buffers (E2) and stay early; every reader needs its own monotone guard or the counters (E9) run backwards
     on every dial increase.
 (b) Publish-side ring (analysis runs live, publishes the snapshot computed D ago). REJECTED: the same tracker-anchor
     and PCM problems as (a).
 (c) CHOSEN -- Delay the AUDIO into the analysis: a time-gated delay line of 512-sample 48 kHz hops between the
     resampler and the 2048 window. Each hop is stamped when it arrives and processed when now >= stamp + D. Every
     consumer in E2 plus PCM, waveform, readouts and REST moves by D with no edits; the analysis sees exactly the same
     continuous audio (no splice); its beat clock runs in sample time (E6) so a dial move only changes WHEN hops run;
     Tap / Resync / typed tempo apply at the next processed hop = at the press in show time (no back-dating);
     counters stay monotone by construction; resolution comes from the stamps (~0.1 ms), not the hop grid.
 (d) Delay in the audio callback / a bigger SPSC ring. REJECTED: Sacred Rule 1; 16384 floats (E5); it would also delay
     the take recorder's raw audio tap.
 Numbers: 500 ms / 10.667 ms = 46.9 -> at most 47 hops waiting (+1 in transit). Line = 64 hops (power of two, 25 %
 slack) x 512 floats = 32,768 floats = 128 KiB + 64 stamps, allocated ONCE in the AnalysisThread constructor (Sacred
 Rule 3). (The rejected snapshot ring would be 64 x 384 B = 24 KiB.)
 Strongest counter-argument to (c): "the input readouts (level, spectrum, waveform) also go D late". True and
 harmless (<= 0.5 s, and consistent with the picture). No consumer needs live data: none extrapolates (E3); the only
 reader that pairs snapshots with live audio counters is the take recorder, fixed by I12.

F2 -- EARLY (D < 0)
 (a) CHOSEN -- BeatLead: after the 14 stages, before the publish, rewrite only the snapshot's beat fields to the
     tracker's state run |D| x bpm / 60 beats ahead, using the tracker's own predicted-beat rule (advancePredictedBeat
     + the updatePhrase bar edge), with guards: totalBeatCount / totalBarCount / the bar-beat position never move
     backwards (HOLD for the hop or two the tracker's own bookkeeping lags, E7), and a RE-BASE that absorbs, never
     decreases, at the tracker's deliberate frame moves (Resync, Tap, downbeat lock / relock, phrase reset, lock lost).
     Onsets / loudness untouched (causality; Boris accepted). Exact rules: Appendix A.
 (b) Shift the tracker's internal phase by delta. REJECTED: in auto mode beatInBar advances on the DETECTED beat (E7),
     so every fold consumer (OscillatorSignal / EnvelopeSignal / ConnectionShaper::beatsNow) would jump back a whole
     beat for delta every beat.
 (c) Per-consumer extrapolation. REJECTED: ~10 consumers, inconsistent views, misses PCM / readouts.

F3 -- How a dial move takes effect
 (a) CHOSEN -- slew: the applied value moves toward the target at 0.25 ms per ms (500 ms in 2 s; a 1 ms click in 4 ms);
     the first value at startup snaps. During a slew the beat-locked visuals run 0.75x / 1.25x briefly (a DJ nudge):
     never a freeze, never a jump, counters monotone.
 (b) Instant: an increase freezes all audio motion for the change, a decrease jumps it forward. Safe but visible on a
     venue switch. (c) A slower slew (0.1): 5 s to settle -- sluggish while tuning by ear.

F4 -- Where venue profiles live
 (a) CHOSEN -- settings.json key "sync" via AppSettings, beside "outputs": the offset belongs to the room + rig (PA,
     projector, input path), not to the show. (b) The composition. REJECTED: the same show plays many rooms; a saved
     offset would impose the last room's timing on every load. (c) A separate venues.json. REJECTED: AppSettings
     already does safe read-modify-write of independent keys (E13).

F5 -- Where the control sits
 (a) CHOSEN -- the TimingWindow "BPM" tab (empty, always visible, right beside the preview Boris watches while tuning;
     E14), hosted through a generic per-tab content slot so bf45 can claim another tab without a clash.
 (b) A TopBar button + popup. REJECTED: the TopBar is over-full (E14) and bf7 edits it. (c) Preferences. REJECTED:
     modal, hides the preview while tuning.

F6 -- The control's form -- CONFLICT FLAGGED (Q1)
 Boris said "dial"; his standing rules reject circular knobs and prescribe one grammar (E15). Every function he named
 (-500..+500, 1 ms steps, type an amount, -/+ fine-tune) fits that grammar exactly. DEFAULT: the grammar row -- typed
 value, -/+ (hold to repeat), a long horizontal bar centred on 0. No routing triangle: a signal-driven offset would
 wobble the whole clock -- a reasoned exception to :413 "Manual-only controls", flagged for Harmony / Boris.

F7 -- Human tempo anchors (Tap / Resync / typed BPM)
 (a) CHOSEN -- nothing special. With F1(c) they land at the press for LATE (they mark what is heard; the dial then
     nudges from that anchor); EARLY leads them like every beat-locked thing. In AUTO mode the tracker re-aligns to
     detected beats within about a beat, restoring the dial-tuned alignment. Documented. (b) Back-date anchors by D in
     BPMTracker. REJECTED: needed only by F1(a)/(b).

F8 -- Keeping gates off Boris's real settings
 (a) CHOSEN -- TEST-ONLY POST /api/debug/sync_persist {"enabled": false} (test-server builds only): the session writes
     nothing; the gate checks the settings file's sha256 before / after. Venue persistence itself is gated in
     --test-mode with a scratch AUDIODNA_SETTINGS_FILE.
 (b) Honour AUDIODNA_SETTINGS_FILE in production mode of test-server builds. REJECTED: widens a safety rule others rely
     on (MainComponent.cpp:70-73). (c) Snapshot + restore the real file. REJECTED: a crash mid-gate leaves a probe
     offset in Boris's settings.

F9 -- Take-recorder stamps (stage S3)
 (a) CHOSEN -- show-time stamps: subtract the LATE part of the snapshot's applied offset from the delivered-sample
     stamps and from the with-audio replay position. A take records each move against the audio Boris was hearing,
     replays right in another room, and its onset markers stay sample-accurate at any setting (probe-step3's T2 math
     holds at D != 0). EARLY delays no audio: nothing to subtract.
 (b) Leave stamps live. REJECTED: a with-audio replay in another venue is off by the difference; markers and the tempo
     map sit D late in the take's audio.

F10 -- MIDI
 (a) CHOSEN -- keys / notes nudge +-1 ms per press; endless-encoder CCs (relative mode) nudge 1 ms per tick (the raw
     tick count reaches the action). (b) Absolute CC. REJECTED: 7-bit = 7.9 ms steps, and the first touch jumps the
     offset to the knob's physical position (E16).

F11 -- Ableton Link
 CHOSEN -- no own offset. Link sets only the tempo (E8); the dial moves the tracker's beat clock whatever its tempo
 source. If Link phase-following is ever built it must read Link's timeline at now + lead (Link's own latency-
 compensation convention) -- recorded as INFO in performance-controls.md.

F12 -- HTTP-thread reads of venue names (Sacred Rule 2 "no new mutex without explicit discussion": this is it)
 CHOSEN -- a mutex-guarded status copy in the controller, locked only by the message thread (publish) and the HTTP
 thread (GET), never by the audio / analysis / render threads -- the RecorderHost::status() precedent (E19). REJECTED:
 a blocking message-thread call from the HTTP handler (the house never does); a seqlocked fixed char table (overkill).

---------------------------------------------------------------------------------------------------------------------
## (4) ITEMS

Every RED is a test in tests/ registered in tests/CMakeLists.txt with apply_sanitizers + catch_discover_tests (house
pattern, tests/CMakeLists.txt:244-284). "Compile RED" = the test names an API that does not exist on 5e47d17, so the
target fails to build there; behavioural REDs are stated where an existing path changes.

I1 -- SyncOffset constants + SyncSlew (pure, header-only)
  Files: NEW src/analysis/SyncOffset.h: `namespace SyncOffset { constexpr int kMaxMs = 500; constexpr int kMinMs = 0;
  /* S2 sets -500 */ constexpr double kSlewMsPerMs = 0.25; constexpr int kDelayLineHops = 64; int clampMs(double) }`
  (round half away from zero, clamp). NEW src/analysis/SyncSlew.h: `class SyncSlew { double step(double targetMs,
  double nowMs) noexcept; double value() const noexcept; }` -- the first step snaps; afterwards moves at most
  kSlewMsPerMs x dt toward the target, dt clamped to [0, 1000] ms.
  RED: tests/test_sync_slew.cpp (compile RED). Cases: first step snaps; target 500 from 0 -> 250 at +1000 ms, 500 at
  +2000 ms; reversal mid-ramp moves from where it is; a 5 s stall moves at most 250 ms; clampMs(600.4)=500,
  clampMs(41.5)=42, clampMs(-30)=0 (S1) / -30 (S2), clampMs(-600)=-500 (S2).
  GREEN: all pass. Risk: none material.

I2 -- AnalysisDelayLine (pure, fixed)
  Files: NEW src/analysis/AnalysisDelayLine.h: fixed FIFO of kDelayLineHops hops x 512 floats + one double stamp per
  hop; storage handed in (a std::unique_ptr<float[]> the owner allocates once in its constructor). API: full(),
  empty(), size(), backSlot(), commitBack(stampMs), frontSlot(), frontStampMs(), popFront(), frontDue(nowMs, lateMs)
  (= full() || nowMs >= stamp + lateMs), msUntilFrontDue(nowMs, lateMs). Analysis thread only: no atomics, no
  allocation after construction.
  RED: tests/test_analysis_delay_line.cpp (compile RED). lateMs 0 -> due at its stamp; lateMs 40 -> not due at
  stamp + 39.999, due at stamp + 40.000; FIFO order and bit-identical samples over 1,000 random push / pop; a full line
  is due whatever the time; lateMs 300 -> 0 makes all 28 waiting hops due at once, oldest first.
  GREEN: all pass.

I3 -- AnalysisThread: the LATE gate + snapshot provenance  (THE core change)
  Files: src/analysis/AnalysisThread.h/.cpp; src/analysis/FeatureSnapshot.h; src/api/ApiServer.cpp handleGetBpm
  (:776-802) + handleGetFeatures (:804+), one property each.
  Behaviour:
  - New on AnalysisThread: `void setSyncTargetMs(int ms) noexcept` (any thread; SyncOffset::clampMs; std::atomic<int>,
    relaxed) and `int syncTargetMs() const`; diagnostics written only by the analysis thread as relaxed atomics:
    `float syncAppliedMs()` (the slewed value), `float syncHopDelayMs()` (EMA, alpha 0.05, of process time minus
    arrival stamp per processed hop), `int syncQueuedHops()`, `uint32_t syncLineOverruns()` (hops forced out by a full
    line; expected 0 forever).
  - run(), per iteration: (1) DRAIN: pull every hop the resampler can give now into the delay line, stamping each with
    juce::Time::getMillisecondCounterHiRes() (the Renderer's frame clock, Renderer.cpp:360) -- the SPSC ring keeps
    draining whatever D is; (2) applied = slew.step(target, now); (3) PROCESS, oldest first, every hop with
    frontDue(now, max(applied, 0)) through a new private `processHop(const float* hop, double appliedMs)` -- the
    existing body from "Shift analysis buffer left by hop" (AnalysisThread.cpp:100) through the PCM snapshot copy
    (:407), its text unchanged except the additions below; (4) if nothing ran: sleep until the front hop is due, capped
    at 1 ms (std::this_thread::sleep_for, microsecond resolution), else sleep(1) as today.
  - processHop writes `snap->syncOffsetMs = static_cast<float>(appliedMs);` beside the R13 provenance (:343-345). The
    lines `snap->trackerRequestSeq = bpmTracker_->appliedRequestSeq();` and `featureBusWriter_.publishWrite();` keep
    their exact text and order (test_tempo_start lint, E18). NO new logLinef / logLine on the analysis thread
    (test_log_line_lint pins 4); diagnostics go out through the atomics only.
  - FeatureSnapshot: `float syncOffsetMs = 0.0f;` at offset 332 + static_assert(offsetof == 332). Comment: provenance,
    not an audio feature (no readout): > 0 = the analysis heard this audio that many ms after it arrived; < 0 = the
    beat fields lead by that many ms (I5); 0 = off. clear() zeroes it (memset).
  - D = 0: every hop is due at its stamp, so it runs in the iteration it arrives -- today's behaviour plus one 2 KiB
    copy in and out (byte-identical analysis input, same order).
  RED: tests/test_analysis_sync_thread.cpp [timing][analysis] (compile RED: no setSyncTargetMs / syncOffsetMs).
  A real AnalysisThread(ring, nullptr) (resampler bypass) with its writer claimed; a feeder thread pushes a click train
  into the ring in real time (512-sample blocks every 10.667 ms, a click burst every 0.5 s, gen-click-wav.py's burst
  shape); the test thread stamps each click block's push time and polls the bus every 0.5 ms for onsetCount changes.
  Arm A target 0, arm B target 150 (set, wait 0.8 s, measure 3 s): median lag(B) - median lag(A) = 150 +- 12 ms;
  onsetCount delta == clicks fed in both arms (no loss, no duplicate); every published snapshot in B carries
  syncOffsetMs == 150. The test self-checks feeder jitter and SKIPs with a warning above 5 ms (loaded machine).
  Plus [tsan]: a second thread calls setSyncTargetMs 10,000 times while the analysis runs 2 s.
  Plus tests/test_feature_bus.cpp: offsetof(FeatureSnapshot, syncOffsetMs) == 332 and a publish / read round trip
  carries it.
  GREEN: [timing] bars on 3 consecutive runs; TSan clean (probe-tsan-unit.sh); test_integration_pipeline,
  test_feature_bus, test_tempo_start [lint], test_log_line_lint green and unchanged.
  Risk: the timing test can flake on a loaded machine -> generous bar + jitter self-check.

I4 -- Witnesses for the live gate (TEST-SERVER builds only, `#if AUDIODNA_TEST_SERVER`)
  Files: NEW src/analysis/SyncWitness.h: a single-writer ring of POD entries, one seqlock per entry (the FeatureBus /
  waveform construction), torn-read-free, no allocation. AnalysisThread::processHop records per hop {hop seq,
  stampMs, processMs, appliedMs, onsetCount, bpm, RAW beat fields (before I5's lead) and PUBLISHED beat fields:
  beatPhase, totalBeatCount, beatInBar, downbeatDetected, totalBarCount, resyncBarOrigin} (ring 1024 = ~11 s).
  Renderer.h/.cpp: inside the onset-pulse block only (Renderer.cpp:401-405), when the pulse fires, record
  {onsetCount, frameMs} (ring 256). ApiServer: GET /api/debug/sync_witness?since=N returns both rings' newer entries.
  RED: tests/test_sync_witness.cpp (compile RED): 1 writer x 2 readers, 200,000 entries, no torn entry; since=N
  returns exactly the newer entries; wrap at capacity.
  GREEN: pass, TSan clean.

I5 -- BeatLead: EARLY (stage S2)
  Files: NEW src/analysis/BeatLead.h (+ .cpp if it passes ~150 lines). src/analysis/BPMTracker.h/.cpp -- READ-ONLY
  additions, no behaviour change: `bool predictedBeatRegime() const` (the existing field, BPMTracker.h:237) and
  `uint32_t frameEpoch() const`, with `++frameEpoch_` at the tracker's deliberate frame moves only: applyResync
  (BPMTracker.cpp:566), applyTempoRequest with realign = Tap (:621), the initial downbeat lock (:476) and a relock
  (:447), the phrase reset (:541), and lock loss in updatePhrase's lockedBPM_ <= 0 branch (:506-513, only on the
  transition: when barCount_ or prevDownbeatDetected_ was still set). src/analysis/AnalysisThread.cpp processHop:
  after stage 14 and the provenance block, before the publish:
  `if (appliedMs < 0.0 || beatLead_.engaged()) beatLead_.apply(*snap, std::max(0.0, -appliedMs) / 1000.0,
   { bpmTracker_->predictedBeatRegime() || bpmTracker_->downbeatLocked(), bpmTracker_->frameEpoch(),
     bpmTracker_->phraseBars() });`
  (stages 13 / 14 read bpm / trackerState / onsetDetected / rms, which the lead never touches; the tracker itself is
  never touched). SyncOffset::kMinMs becomes -500.
  Rules: Appendix A (exact).
  RED: tests/test_beat_lead.cpp T-L1..T-L7 (compile RED; Appendix A). tests/test_bpm_stabilization.cpp new [epoch] case:
  frameEpoch rises on Resync, a Tap realign, downbeat lock and relock, a phrase reset and lock loss; NOT on a
  confident-detection realign, a phase wrap or followExternalTempo (compile RED: no frameEpoch()).
  GREEN: all pass; T-L3's 50,000-hop randomized run: totalBeatCount / totalBarCount never decrease, no hold longer than
  0.5 beat + 2 hops, view bar increments - tracker bar increments in [0, number of re-bases].
  Risk: the most intricate piece -- isolated: identity (byte-identical) whenever the lead is 0 and was never engaged.

I6 -- SyncVenues model (pure)
  Files: NEW src/sync/SyncVenues.h/.cpp (juce_core only). src/model/AppSettings.h: `static constexpr const char* kSync =
  "sync";`.
  Behaviour: venues {name, ms}; always >= 1 (a missing / corrupt key reads as one venue "Default" at 0); names trimmed,
  1..40 chars, unique case-insensitively, <= 64 venues; ms clamped to [kMinMs, kMaxMs]; `current` always names an
  existing venue (else the first). Ops: select(name); create(name) -- copies the CURRENT offset and selects the new
  venue (creating never changes what is on screen); rename(from, to); remove(name) -- refused for the last venue,
  removing the current one selects the first; setMs(ms); nudge(delta). Every op returns ok / a plain-words refusal.
  JSON (settings.json): "sync": {"version": 1, "current": "Default", "venues": [{"name": "Default", "ms": 0}]}
  RED: tests/test_sync_venues.cpp (compile RED): tolerant load (missing, corrupt, duplicate names, ms out of range,
  unknown current); every op incl. each refusal; exact JSON round trip. tests/test_app_settings.cpp new case:
  update("sync", ...) keeps "outputs" and "milkDropPresetDir".

I7 -- SyncOffsetController (message-thread glue)
  Files: NEW src/sync/SyncOffsetController.h/.cpp. src/MainComponent.h: `std::unique_ptr<SyncOffsetController>
  syncOffset_;` declared AFTER analysisThread_ (MainComponent.h:312). src/MainComponent.cpp constructor: create it with
  appSettingsFile(testMode_) and a target sink `[this](int ms) { analysisThread_.setSyncTargetMs(ms); }` BEFORE the
  analysis start block (:1859), so the first hop already uses the saved offset.
  Behaviour: loads the venues at construction and pushes the current offset; EVERY change (panel, REST, OSC, keys,
  MIDI) goes through it: update the model, push the target, publish the mutex-guarded status copy (F12), notify
  listeners (the panel), schedule a save 500 ms after the last change (juce::Timer); the destructor flushes a pending
  save; a TEST-ONLY switch turns persistence off for the session (then nothing is ever written). The offset is a
  machine setting: not undoable, never recorded into takes or routines, not a ControlPath / grip.
  RED: tests/test_sync_offset_controller.cpp (compile RED): load pushes the saved offset to the sink; set / nudge /
  select push; a burst of 20 changes writes once (flushForTest) with exact JSON; persist off -> file bytes unchanged
  after 20 changes; destruction flushes a pending save; the status copy is readable from a second thread during
  changes ([tsan]).

I8 -- REST + OSC
  Files: src/api/ApiServer.h/.cpp (route lines appended after the registration block, ApiServer.cpp:338; debug routes
  inside the existing `#if AUDIODNA_TEST_SERVER` block :299-329; handlers appended at the end of the file; callback
  members); src/osc/OscHandler.h/.cpp (two exact-address branches before the /audiodna/routine/ branch :193-195, two
  callbacks); src/MainComponent.cpp (apiServer_ callbacks appended just before apiServer_->start() :2202; oscHandler_ callbacks
  appended at the end of the block :2205-2290).
  Contract:
   GET  /api/sync -> {"ok":true,"targetMs":42,"appliedMs":41.75,"venue":"Warehouse",
        "venues":[{"name":"Default","ms":0},{"name":"Warehouse","ms":42}],"persist":true,"analysisRunning":true,
        "hopDelayMs":42.1,"queuedHops":4,"lineOverruns":0,"leadEngaged":false,"lastError":""}
   POST /api/sync/set {"ms":N}            the current venue's offset (rounded, clamped)
   POST /api/sync/nudge {"ms":N}          +- N
   POST /api/sync/venue {"name":"X","create":false}   select; create:true creates (copying the current offset) + selects
   POST /api/sync/venue/rename {"from":"X","to":"Y"}
   POST /api/sync/venue/remove {"name":"X"}
   TEST-ONLY: POST /api/debug/sync_persist {"enabled":bool}; GET /api/debug/sync_witness?since=N (I4);
              POST /api/debug/sync_ui {"show":"venueMenu"|"newVenue"|"renameVenue"|"deleteVenue"|"none"} (I11 captures)
   POSTs parse on the HTTP thread, callAsync to the message thread, answer {"ok":true} at once (house pattern,
   ApiServer.cpp:1622-1626); a refused op (unknown venue, duplicate or empty name, last venue) lands in lastError. GET
   reads only the status copy + AnalysisThread atomics, never the model.
   OSC: /audiodna/sync <float ms> = set; /audiodna/sync/nudge <float ms> = nudge (rounded to whole ms).
  S1a minimal form: GET /api/sync (target / applied / diagnostics) and POST /api/sync/set|nudge wired straight to
  analysisThread_.setSyncTargetMs (session only); S1b reroutes them through the controller and adds the rest.
  RED: live -- GET /api/sync is 404 on 5e47d17 (probe-sync row R0). Unit: tests/test_osc_sync.cpp drives
  OscHandler::oscMessageReceived with juce::OSCMessage("/audiodna/sync", 25.0f) -> onSetSync(25) and
  ("/audiodna/sync/nudge", -3.0f) -> onNudgeSync(-3) (compile RED; links juce_osc).

I9 -- Keys & MIDI (stage S4)
  Files: src/binding/Binding.h: APPEND `SyncNudge` after TriggerRoutine (:44-46) and `int targetSyncStepMs = 1;`
  (signed: -1 earlier, +1 later). src/binding/BindingManager.cpp: processMidiCC (:168-187) -- for SyncNudge in Relative
  mode hand the action the raw signed tick count (value - 64), skip the accumulator; Absolute CC for SyncNudge is
  ignored; toVar / fromVar (:215-290) carry targetSyncStepMs (absent -> +1). src/ui/BindingOverlay.h/.cpp and
  src/ui/MidiLearnOverlay.cpp: BindableTarget gains `int syncStepMs`, capture cases beside TriggerRoutine (:292 / :339).
  src/MainComponent.cpp: buildBindableTargets global row (:7436-7460) appends "Sync -1 ms" and "Sync +1 ms";
  handleBindingAction gains a case after MasterSignal (:7757-7760): key / note press -> nudge(step); relative CC ->
  nudge(ticks x |step|).
  RED: tests/test_binding_sync_nudge.cpp (compile RED): relative CC 65 -> +1, 63 -> -1, 67 -> +3; absolute CC -> no
  action; serialization round trip; a saved file without targetSyncStepMs -> +1.
  Merge note: bf1 may also append a Binding action; the second to merge appends after the first (ints are persisted).

I10 -- TimingWindow per-tab content slot (stage S5)
  Files: src/ui/TimingWindow.h/.cpp: `void setTabContent(Tab, juce::Component*)` (non-owning, addChildComponent);
  resized() gives the active tab's content the area under the tab bar; setActiveTab shows only the active tab's
  content; paint() draws the placeholder name only for a tab without content.
  RED: tests/test_timing_window_content.cpp (compile RED): content visible with bounds == the content area on its tab;
  hidden on another tab; the placeholder is not painted under content (render to an Image and compare) -- Pitfall 34:
  set the window visible explicitly in the headless test.

I11 -- SyncPanel (stage S5 -- the only visible change)
  Files: NEW src/ui/SyncPanel.h/.cpp. src/ui/LookAndFeel.h/.cpp: drawLinearSlider fills from the 0 point when the
  slider carries the component property "bipolar" = true (nothing else changes). src/MainComponent.cpp: create it right
  after the TimingWindow (:1796-1797), `timingWindow_->setTabContent(TimingWindow::Tab::BPM, syncPanel_.get())`, wire it
  to the controller.
  Layout (house grammar, BORIS_DECISIONS.md:180-188; section header :192-199):
     SYNC                                     section header
     Venue   [ Warehouse  v ]                 one button; its menu: the venues (tick = current) | New venue... |
                                              Rename venue... | Delete venue... (warning red via addColouredItem, behind
                                              a confirm, greyed when only one venue); showMenuAsync +
                                              withParentComponent(getTopLevelComponent())
     Offset  [ +42 ms ] [-][+] [=======|=====]
             Visuals 42 ms later              caption, #888
  Behaviour: the bar is a ResettableSlider, LinearHorizontal, range kMinMs..kMaxMs, interval 1, setDefaultValue(0),
  NoTextBox, setSliderSnapsToMousePosition(false) (a click never jumps the offset), velocity drag tuned to ~1 ms per
  pixel. The value box is an editable label: "42", "+42", "-30", "42 ms" -> Enter applies (clamped), Esc reverts,
  anything else reverts. Right-click on the box or the bar -> 0. -/+ step 1 ms and repeat while held
  (setRepeatSpeed(400, 50): 20 steps a second after 0.4 s); right-click on -/+ does nothing. Caption: 0 -> "In step
  with the sound coming in"; +N -> "Visuals N ms later"; -N -> "Beats N ms earlier (loudness can't run early)".
  New / Rename use the AlertWindow + addTextEditor idiom (MainComponent.cpp:3683-3701); a refusal (duplicate / empty
  name, last venue) shows in the caption line in the warning colour until the next change. The panel holds no truth:
  every widget is re-pulled from the controller on each change notification (Pitfall 41), skipping the bar while it is
  dragged and re-pulling on drag end; it repaints only on change (Pitfall 57). Its range follows SyncOffset::kMinMs
  (0..500 until S2 is in).
  RED: tests/test_sync_panel.cpp (compile RED): the bar is a ResettableSlider, range -500..500 (S2), interval 1, default
  0; right-click -> onSetMs(0); typing "+42" -> onSetMs(42); "abc" -> no call, reverted; "-" / "+" -> onNudge(-1 / +1);
  refreshFromModel fires no callback; captions for 0 / +42 / -30; menu items for 1 vs 3 venues (Delete greyed with 1);
  a mouse-down mid-track does not change the value.
  Risk: taste -- the visual work gate (G7) is where it surfaces.

I12 -- Takes stay aligned (stage S3)
  Files: src/recording/RecorderHost.cpp tick() only: sampleForClock (:453-458) and the with-audio playback position
  (:553-566).
  Behaviour: late = max(snap.syncOffsetMs, 0); sampleForClock = deliveredSamples - llround(late / 1000 x deviceRate)
  (clamped at 0) [overdub: the asset-frame value - llround(late / 1000 x overdub asset rate)]; with-audio replay
  pos -= llround(late / 1000 x playAssetRate_). syncOffsetMs <= 0 changes nothing (bit-identical stamps). Wall-clock
  replay unchanged. The stamps then name the audio the analysis was describing -- what Boris saw and heard.
  RED: tests/test_recorder_host.cpp new [host][sync] cases (compile RED: no syncOffsetMs; behaviourally 5e47d17 stamps
  the raw counter): armed and ticked at syncOffsetMs 100, 48 kHz, delivered 1,000,000 -> the "start" anchor sample is
  995,200; at -100 -> 1,000,000; a with-audio replay at 100 ms fires a point stamped s when the transport reaches
  s + 4,800.
  GREEN: pass; live row R7.

I13 -- Gate tooling (written by the S1a builder; extended by S2 / S3 / S5 builders; RUN by Harmony only)
  Files: NEW .harmony/probe-sync.sh + .harmony/probe-sync.py (rows in section 5), .harmony/shoot-sync-panel.sh (G7).

## BUILD STAGES (one builder context each)

  S1a "Later -- engine": I1, I2, I3, I4, I8-minimal, I13 (LATE rows). Range 0..+500. Invisible; at 0 nothing changes.
      SHIPS ALONE.
  S1b "Venues & remote control": I6, I7, I8-full. Needs S1a. SHIPS ALONE (REST / OSC, persisted).
  S2  "Earlier": I5, kMinMs -> -500, probe EARLY rows. Needs S1a. SHIPS ALONE (reachable by REST / OSC).
  S3  "Takes stay aligned": I12, probe row R7. Needs S1a. SHIPS ALONE.
  S4  "Keys & MIDI": I9. Needs S1b. SHIPS ALONE.
  S5  "The control on screen": I10, I11, captures. Needs S1b; shows the negative half once S2 is in. Visual work gate
      (G7) before Boris sees it.
  Order: S1a -> {S1b, S2, S3} (disjoint files apart from tests/CMakeLists.txt; S2 touches only processHop's tail in
  AnalysisThread.cpp) -> {S4, S5}.

## FILES SHARED WITH OTHER LANES (exact blocks; bf2 plans nothing inside another lane's core)

  - src/MainComponent.cpp: constructor -- controller creation just before the analysis start block (:1859); apiServer_
    callbacks appended just before apiServer_->start() (:2202); oscHandler_ callbacks appended at the end of :2205-2290; one line after the TimingWindow
    creation (:1796-1797); buildBindableTargets global row (:7436-7460, two targets appended); handleBindingAction (one
    case after :7757-7760). NOT touched: handleDeckSwitch, tickFeaturePipeline, timerCallback, any render path.
  - src/MainComponent.h: one member after analysisThread_ (:312), one beside timingWindow_ (:491).
  - src/api/ApiServer.h/.cpp: appended routes / handlers / callbacks; one property in handleGetBpm and handleGetFeatures
    (bf1, ui, tsan-r5 likely touch ApiServer too -- all additive).
  - src/osc/OscHandler.h/.cpp: two branches + two callbacks (additive).
  - src/binding/Binding.h, BindingManager.cpp, src/ui/BindingOverlay.*, MidiLearnOverlay.cpp: enum APPEND (sequence
    with bf1 at merge).
  - src/render/Renderer.h/.cpp: the TEST-ONLY witness record inside the onset-pulse block (:401-405) only (bf9 edits
    :700-770).
  - src/ui/TimingWindow.h/.cpp: the per-tab content slot (bf45 can reuse setTabContent for another tab).
  - src/ui/LookAndFeel.h/.cpp: one property-keyed branch in drawLinearSlider.
  - src/analysis/BPMTracker.h/.cpp (S2): accessors + epoch increments only.
  - src/recording/RecorderHost.cpp (S3): tick() :453-458 and :553-566 only.
  - src/model/AppSettings.h: one key constant.
  - tests/CMakeLists.txt (appended targets); CLAUDE.md (byte budget); .harmony/APP-INVENTORY.md; docs/claude/*.md.

---------------------------------------------------------------------------------------------------------------------
## (5) GATES (Harmony runs them after each stage's merge; the builder writes the scripts, never runs the live ones)

Rig, every live row: a test-server build (AUDIODNA_BUILD_TEST_SERVER=ON) in its own build dir; the live lock held
(/tmp/audiodna-live.lock, exactly one Audio-DNA); `ps` checked for CPU burners (> 20 % CPU) before any timing / perf
verdict; launch only `open -g [--env AUDIODNA_SETTINGS_FILE=<abs scratch>] <app> [--args --test-mode]`; never an Output
window (end-of-run Quartz full-window-list check, as probe-onset-render.sh does); captures by Quartz window id only;
HTTP clients send `Connection: close`; graceful osascript quit; whole run <= 2 h.

G1 ctest -- every suite, incl. test_sync_slew, test_analysis_delay_line, test_analysis_sync_thread, test_sync_witness,
   test_beat_lead, test_bpm_stabilization [epoch], test_sync_venues, test_app_settings, test_sync_offset_controller,
   test_osc_sync, test_binding_sync_nudge, test_timing_window_content, test_sync_panel, test_recorder_host [sync],
   test_feature_bus; test_tempo_start [lint] and test_log_line_lint unchanged. Rule: any failure blocks.
G2 TSan -- .harmony/probe-tsan-unit.sh over test_analysis_sync_thread [tsan], test_sync_witness,
   test_sync_offset_controller [tsan]: 0 reports. Blocks.
G3 Zero-heap -- a [lint] case (test_hot_thread_io_lint style, source-text scan) over SyncSlew.h, AnalysisDelayLine.h,
   BeatLead.h, SyncWitness.h: no `new `, `malloc`, `std::vector`, `push_back`, `std::string`, `juce::String`;
   AnalysisThread.cpp's `logLinef(` count still 4. Blocks.

G4 .harmony/probe-sync.sh -- PRODUCTION mode (no --test-mode: the analysis thread runs). First: POST
   /api/debug/sync_persist {"enabled":false}; record sha256 of ~/Library/Audio-DNA/settings.json; read and log GET
   /api/sync (the venue offset the app launched with).
   R0 GET /api/sync answers 200 (5e47d17: 404 -- the RED).
   R1 hop delay (any input; the mic is enough). Interleaved arms [0, 100, 0, 250, 0, 500, 0, 37, 38, 37, 38, 1, 0]. Per
      arm: set, wait until |appliedMs - targetMs| < 0.1 for 0.5 s, sample hopDelayMs 20 times over 2 s -> arm mean.
      Bars: mean(D) - mean(neighbouring 0 arm) = D +- 0.8 ms for D in {100, 250, 500}; mean(38) - mean(37) in
      [0.6, 1.4] on both pairs; lineOverruns == 0 throughout; queuedHops at 500 in [46, 49].
   R2 end to end (the click track: gen-click-wav.py, 120 BPM grid = 24,000 frames at 48 kHz, played by
      POST /api/perf/record {"name":"probesync","audio":false,"audioFile":<wav>}, 30 s per arm; preview visible --
      render_frame once to attach GL, as probe-onset-render.sh does). Arms [0, 100, 0, 250, 0, 500, 0, -100 (after S2),
      0]. Per onset, lag = render pulse frameMs - its onset hop's stampMs (witness, paired by onsetCount). Bars: median
      lag shift vs the neighbouring 0 arm = D +- 3 ms (D > 0); at -100: |shift| <= 3 ms (onsets are never early);
      render pulse frames delta == onsetCount delta in every arm (no loss, no duplicate); onsetCount delta >= 54 of 60
      clicks per arm.
   R3 slew (manual 120 BPM via POST /api/set_bpm {"bpm":120}): 0 -> 500: appliedMs reaches 500 in 2.00 +- 0.15 s;
      while slewing every published hop advances the published beatPhase and the published beat rate stays within
      0.70-1.30x of 2 beats/s (witness); 500 -> 0 symmetric.
   R4 (S2) EARLY, manual 120 BPM: settled at -100, published - raw beatPhase (mod 1, witness, every hop) = 0.200 +- 0.005
      beat; at -500 published totalBeatCount - raw = 1 with equal phase +- 0.005; a scripted run -100 -> Resync -> -500
      -> Resync -> 0 -> +100: published totalBeatCount and totalBarCount never decrease; every published totalBarCount
      increment coincides with downbeatDetected rising, and the level stays true for one published beat (a Resync may
      raise the level without a bar increment, exactly as the tracker does: applyResync, BPMTracker.cpp:566-578); published bar
      increments - raw bar increments in [0, 2].
   R5 (S2) EARLY in AUTO mode with the click track at -100: R4's monotonicity / level bars hold; R2's onset bar holds.
   R6 settings untouched: sha256 identical at the end; GET /api/sync shows persist false throughout.
   R7 (S3) take alignment: a click take with onsetMarkers (audio:true) at D = 0 and at D = 100, probe-step3's T2 marker
      vs click math: median error at 100 equals the one at 0 within 1 ms. (On S1a alone it differs by ~100 ms: the live
      RED for S3.)
   R8 regression at D = 0: probe-onset-render.sh, probe-tempo-start.sh, probe-beatclock.sh, probe-downbeat-level.sh PASS
      as on main. They launch the app themselves, so Harmony confirms the launched offset (GET /api/sync, logged by R0 of
      the same session) is 0; if Boris has saved a non-zero venue by then, run them in test-server production mode
      after POST /api/sync/set 0 with persist off, or record the offset as a caveat.
   R9 Resync anchor under LATE (INFO, documents F7): at +100, manual 120 BPM, POST /api/resync: the published downbeat
      appears within 2 hops of the request (not 100 ms later).
   Decision rule: R0-R7 all PASS = merge-ready. A bar missed by no more than its own noise: re-run that row twice,
   interleaved; 2 of 3 PASS = PASS with the numbers recorded; otherwise block. R9 is INFO.

G5 Venues and remote control -- TEST mode, scratch AUDIODNA_SETTINGS_FILE seeded with "outputs" + "milkDropPresetDir":
   REST: create "Club A" (copies the current 0), set 42, create "Club B", set -30 (after S2; 0 before), rename
   "Club A" -> "Warehouse", remove "Club B", select "Default" -> the file's "sync" JSON equals the expected object
   exactly, the seeded keys untouched. Refusals (duplicate name, empty name, removing the last venue, unknown venue) ->
   lastError set, file unchanged. Graceful quit + relaunch -> GET /api/sync shows the same current venue and offsets.
   OSC (python3 UDP socket to 127.0.0.1:8000): /audiodna/sync 25.0 -> targetMs 25; /audiodna/sync/nudge -3.0 -> 22;
   /audiodna/sync 900.0 -> 500. MIDI / keys: unit tests only (G1) -- a live MIDI source would be synthetic input (INFO).

G6 Perf A/B -- production, mic input, preview visible, `ps` checked first. INTERLEAVED A/B/A/B..., >= 5 runs per arm, 20
   s each: D = 0 vs D = 500 (and vs -500 after S2). Metrics: analysis cpuLoad (DSP %), render frameMs (/api/status).
   Bar: |delta of means| <= max(0.3 percentage points, 2 x the within-arm run SD). A delta inside the run-to-run drift
   is INFO, not a verdict.

G7 VISUAL WORK GATE (S5) -- before Boris sees anything.
   Captures (test mode, scratch settings; every state set by REST incl. TEST-ONLY POST /api/debug/sync_ui; the main
   window by Quartz window id, cropped to the TimingWindow; at the default and the narrowest panel width):
   C1 first launch (Default, 0, "In step with the sound coming in"); C2 Warehouse +42; C3 -30 (early caption, after
   S2); C4 the limits +500 / -500; C5 the venue menu open (3 venues, tick on the current, Delete in warning red); C6 New
   venue dialog; C7 Rename dialog; C8 a refusal caption (duplicate name); C9 the Routing / Oscillators tabs (their
   placeholder still painted, no sync widgets); C10 the panel after a REST change while it is visible (follows the
   model).
   Critic panel, in parallel, each given the decoded PNGs + this plan's I11 + BORIS_DECISIONS.md Inspector Grammar /
   Section Headers / Rejected list: visual-design critic, UX critic, graphic-design critic, logic critic, and a
   dedicated INTERACTION-LOGIC critic (stateful + navigational: venue select / create / rename / delete, typed entry,
   -/+ hold-repeat, right-click reset, a REST / OSC change arriving while the panel is open or mid-drag). Blocking
   findings are fixed and re-shot; then an artifact page Boris opens: the captures, one plain-words paragraph per
   state, and the section (9) questions with their defaults.

---------------------------------------------------------------------------------------------------------------------
## (6) DOCS

  - docs/claude/analysis.md -- NEW "### Sync offset (per-venue dial)": the delay line (where, 64 hops, stamps, the gate,
    slew 0.25 ms/ms, D = 0 = today), FeatureSnapshot::syncOffsetMs (provenance), EARLY / BeatLead (what moves, hold,
    re-base, excess counters -- Appendix A in prose), human anchors (F7), test mode has no offset; plus the paragraph
    moved out of CLAUDE.md (below).
  - docs/claude/architecture.md -- Lock-Free Communication Chain: add "Resampler -- 64-hop delay line (sync dial;
    analysis-thread private) --> 2048 window" and "UI thread -- std::atomic<int> sync target --> Analysis thread" (and
    fix that block's stale "Triple-Buffer Atomic Swap" -> "seqlock FeatureBus" while there); Latency Budget: row "3c.
    Sync delay (per-venue dial) | the hop waits D ms (0 by default) | +D", total "~15-25 ms + the venue's LATE offset";
    Source Tree: the new files.
  - docs/claude/integration.md -- NEW "### Sync dial (per venue)": REST routes, OSC addresses, settings.json "sync" key
    and schema, the TEST-ONLY routes.
  - docs/claude/performance-controls.md -- Binding: SyncNudge (keys / notes +-1 ms per press; relative encoders 1 ms per
    tick; absolute CC ignored, and why); Ableton Link: "the sync dial moves the tracker's beat clock whatever its tempo
    source; Link has no offset of its own; if Link phase-following is ever built it reads Link's timeline at now +
    lead".
  - docs/claude/recording.md -- take stamps are in show time (S3): what is subtracted, EARLY untouched, replays across
    venues.
  - docs/claude/testing-eyes.md -- one line: test mode has no analysis thread, so the sync dial has no timing effect
    there; timing gates run in production mode (.harmony/probe-sync.sh).
  - docs/claude/pitfalls.md -- "Pitfall NN" (Harmony assigns; next free 64): **The analysis hears the music late by the
    venue's sync offset; EARLY moves only beat fields** -- with a LATE offset D > 0 every hop waits D ms in the analysis
    delay line before the 14 stages, so EVERY bus reader (render, 120 Hz pipeline, REST, projectM PCM, the waveform)
    describes audio delivered D ms ago, while the audio callback's delivered-sample counter, AudioTap and the file
    transport are live: never pair a snapshot with those without subtracting FeatureSnapshot::syncOffsetMs's LATE part
    (RecorderHost::tick does); never make the analysis loop wait on anything but the delay line (the ring must keep
    draining). EARLY (D < 0) delays nothing: BeatLead rewrites only the beat fields, never onsets or loudness, and never
    moves totalBeatCount / totalBarCount backwards. Test mode has no analysis thread, so no offset there. Guards:
    test_analysis_delay_line, test_beat_lead, test_analysis_sync_thread; live: .harmony/probe-sync.sh.
  - CLAUDE.md (24,002 of 25,000 B). ADD (312 B): Analysis Thread paragraph, after "...when the device already runs at 48
    kHz." -> " Each hop then waits D ms in a 64-hop delay line (the per-venue sync dial; 0 by default)." (89 B); Key
    capabilities -> "per-venue sync dial (-500..+500 ms), " (37 B); Pitfall index -> "NN. The analysis hears the music
    late by the venue's sync offset; EARLY moves only beat fields -- before pairing a snapshot with the delivered-sample
    counter, AudioTap or the transport." (~186 B). PAY (300 B): move the Analysis Thread parenthetical "(the resampler
    reconfigures a fixed-size interpolator in O(1), no aubio object is ever re-created)" (99 B) to analysis.md, and the
    per-category source breakdown "(3D 24, Geometric 11, ... Routing 1)" (201 B) to APP-INVENTORY.md (where CLAUDE.md:3
    says canonical counts live). Net ~ +12 B; Harmony arbitrates the shared budget across lanes.
  - .harmony/APP-INVENTORY.md -- TimingWindow row (BPM tab live: the Sync panel; Routing / Oscillators still empty); REST
    route list (+6 production, +3 test-only); OSC addresses (+2); Binding actions (+SyncNudge); settings.json keys
    (+"sync"); FeatureSnapshot field count (re-count by machine: the stale "43 fields" + syncOffsetMs); the source
    breakdown moved in from CLAUDE.md; new source files.
  - BORIS_DECISIONS.md -- new entry under "Playback Behaviour (Boris rulings)": "Sync dial per venue (2026-10-02,
    s-rta-1002b, verbatim): '...'" + what was built + Q1's answer when given (dial vs bar).

---------------------------------------------------------------------------------------------------------------------
## (7) RISKS (each with the cheapest discriminating test)

  R1 A saved venue offset applies at launch to everything; at a NEW room Boris may forget to switch venues and the show
     runs with the last room's timing. P medium, impact high. Mitigation: the venue name always visible beside the
     control; Q2. Test: G5 relaunch row.
  R2 BeatLead defects (a phantom bar, a long hold, a counter dip) -- the most intricate code. P medium, impact medium
     (EARLY is rarely used). Cheapest test: T-L3 (50,000 randomized hops, unit) before any live run; live R4 / R5.
  R3 The analysis-loop restructure changes D = 0 behaviour (timing, CPU, a lint). P low, impact high. Tests: G3 lints,
     R1's D = 0 arms (hopDelayMs ~ 0), R8 regressions, G6 A/B.
  R4 Tap / Resync / typed BPM land at the press under LATE (F7): in MANUAL mode the dial shifts the grid only from the
     last anchor. Semantic surprise, impact low. Test: R9 (INFO); explained on the artifact page.
  R5 Takes recorded in one venue and replayed with audio in another (until S3 ships). Test: R7.
  R6 The slew (0.75x / 1.25x for up to 2 s) is visible on a venue switch mid-show. Boris-feel item; Q3.
  R7 A gate pollutes Boris's settings.json. Mitigation: persist-off hook + sha256 (R6).
  R8 Merge collisions: TimingWindow (bf45), Binding enum append (bf1), CLAUDE.md bytes (all lanes), ApiServer route block
     (bf1 / ui / tsan-r5). Mitigation: exact blocks named above; additive edits only.
  R9 test_analysis_sync_thread flakes on a loaded machine. Mitigation: +-12 ms bar, feeder-jitter self-check that SKIPs
     with a warning; the live R1 / R2 rows are the authority.
  R10 A future reader pairs a snapshot with live audio counters and is silently D late. Mitigation: Pitfall NN + the
     syncOffsetMs field.
  R11 Strongest counter-argument to F1(c): "the analysis is the most timing-sensitive stage; changing its loop risks
     Pitfall 48 (commands apply at the next hop)". Answer: the steady-state hop cadence is unchanged at any constant D;
     during a slew-up the next hop comes <= 14 ms later instead of <= 10.7 ms. Test: probe-tempo-start.sh (R8) at D = 0;
     INFO at D > 0.
  R12 The app cannot measure the room or the projector: the dial is tuned by eye and ear. That is the product (Boris's
     check, section 8), not a defect.

---------------------------------------------------------------------------------------------------------------------
## (8) WHAT ONLY BORIS CAN CHECK (live rig, his taste)

  1. In a real room: pick or create the venue, play music, watch the kick flashes from where the audience stands, and
     nudge -/+ until they land together; the top bar's beat wheel then flashes with the beat he hears.
  2. Feel: is 1 ms per click right, is holding -/+ (20 steps a second after a short pause) fast enough, is dragging the
     bar precise enough?
  3. A venue switch mid-set: is the up-to-2-second glide fine, or jarring?
  4. The words: "Visuals 42 ms later" / "Beats 30 ms earlier (loudness can't run early)" / "In step with the sound
     coming in".
  5. EARLY (rare): beat-synced motion leads while the loudness flashes stay on the hit -- does that look right?
  6. After a Resync or Tap: does the grid land where he pressed?

---------------------------------------------------------------------------------------------------------------------
## (9) QUESTIONS FOR BORIS (plain words; each has a default so nothing waits)

  Q1 You said "dial". Your earlier rule says no round knobs, and every control is a long bar with a number box and - /
     + buttons. DEFAULT: build it that way -- type the number, click - or + for 1 ms, or drag the long bar. Want a round
     dial instead?
  Q2 When you open the app, should it come back on the venue you used last? DEFAULT: yes, with the venue's name always
     shown next to the control.
  Q3 Big changes (like switching venues) glide over up to 2 seconds -- the visuals run a little slower or faster while
     it settles -- instead of jumping. DEFAULT: glide. Prefer an instant jump?
  Q4 Where should it live? DEFAULT: in the empty "BPM" panel right next to the preview window (bottom row). The other
     choice: a small button in the top bar that opens it.
  Q5 A new venue starts from the setting you have right now (so you can tune first, then save it under a name).
     DEFAULT: yes. Or should a new venue always start at 0?

---------------------------------------------------------------------------------------------------------------------
## Appendix A -- BeatLead (EARLY) exact rules (I5)

  Per hop, input: the tracker's beat fields S = {beatPhase p, totalBeatCount c, beatInBar b, downbeatDetected L,
  barPhase, barCount N, totalBarCount T, resyncBarOrigin O, phrasePhase}; bpm; lead seconds l >= 0; flags
  {barsAdvance = predictedBeatRegime || downbeatLocked, frameEpoch e, phraseBars P}.
  delta = l x bpm / 60 when bpm > 0, else 0 (delta <= 0.5 s x 200 / 60 = 1.67 beats).

  simulate(S, delta) -- the tracker's own predicted rule run delta beats ahead, pure:
    x = p + delta; w = floor(x); p' = x - w; c' = c + w;
    walk w beats from b: b_j = (b_{j-1} + 1) mod 4; if barsAdvance: L_j = (b_j == 0), bars += (L_j && !L_{j-1});
    else L unchanged (pre-lock it is false) and no bars.
    b' = b_w; L' = (w > 0 ? L_w : L); N' = N + bars; T' = T + bars; barPhase' = (b' + p') / 4;
    phrasePhase' = ((N' mod P) + barPhase') / P (wrapped / clamped exactly like BPMTracker::updatePhrase :549-557);
    O' = O.
  Excess counters (start 0, never decrease): beatX, barX. c'' = c' + beatX; T'' = T' + barX; O'' = O + barX
  (so barsSinceResync() = T'' - O'' = T' - O: unchanged by the excess).
  Fold position: Q = barsAdvance ? 4 x T'' + b' + p' : c'' + p'.
  State: engaged, F (the beat fields published last), lastEpoch, lastBarsAdvance.

  1. Not engaged and delta == 0 -> publish S unchanged (IDENTITY, byte-identical). Done.
  2. RE-BASE hop (not yet engaged, bpm == 0, e != lastEpoch, or barsAdvance changed): if engaged and c'' < F.c then
     beatX += F.c - c'' and c'' = F.c; if engaged and T'' < F.T then barX += F.T - T'', T'' = F.T, O'' += the same.
     Publish the simulated fields (the phase may dip within a beat, as the tracker's own realign does).
  3. Otherwise, if Q < Q(F) or c'' < F.c or T'' < F.T -> HOLD: publish F again (every beat field frozen; all non-beat
     fields current). Else publish the simulated fields.
  4. Store F / lastEpoch / lastBarsAdvance; engaged = true. Disengage (engaged = false) when delta == 0, beatX == barX
     == 0 and the published fields equal S (caught up, not holding).
  Untouched always: onsetDetected / onsetStrength / onsetCount, every loudness / spectral / pitch / genre field, bpm,
  trackerState, trackerRequestSeq, syncOffsetMs.

  Guarantees, tested in tests/test_beat_lead.cpp:
  T-L1 identity: 10,000 random snapshots at lead 0, never engaged -> byte-identical.
  T-L2 predicted steady (manual 120 BPM, lead 100 ms = 0.2 beat): published phase = frac(raw + 0.2) every hop;
       published totalBarCount rises 0.2 beat (9.4 hops +- 1) before the raw one; downbeatDetected true for exactly the
       view's first beat of each bar.
  T-L3 50,000 randomized hops (auto regime with a 0-2 hop count / beatInBar lag, first- and second-half realigns,
       Resync, Tap, relock, phrase reset, silence on / off; lead in {0, 50, 100, 300, 500 ms} moved through SyncSlew;
       bpm 90 / 128 / 174 with jumps): totalBeatCount and totalBarCount never decrease; barsSinceResync() never
       decreases except at a Resync epoch; no hold longer than 0.5 beat + 2 hops; view bar increments - raw bar
       increments in [0, number of re-bases].
  T-L4 Resync re-base: the hop after a Resync publishes b' = w, barsSinceResync() in {0, 1}, no hold.
  T-L5 disengage: lead ramps to 0; once caught up with no excess the output is byte-identical again.
  T-L6 unmetered: bpm -> 0 re-bases to the tracker's fields (+ excess); nothing decreases.
  T-L7 level contract (Pitfall 32): every published totalBarCount increment coincides with downbeatDetected rising, and
       the level stays true until the view's next beat (a Resync may raise it without a bar increment, as the tracker's
       own applyResync does).

STATUS: COMPLETE -- plan-bf2 final (rev 2), 2026-10-02; stages S1a, S1b, S2, S3, S4, S5; 5 Boris questions with defaults.

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 15:55:19) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-bf2.md IN FULL (amendments, BUILD STAGES, FINAL CONSOLIDATED GATE LIST).
   THIS LANE BUILDS S1a -> S1b -> S2 -> S3 -> S4 (in that order, one builder each). S5 (the control on screen) is NOT in
   this run: it waits for Boris's Q1 (bar vs round dial) and becomes a follow-up stage on this branch. S6 only if Q6 =
   "later". Base = main eff2b1c (src identical to fa9604d).
2. BORIS QUESTIONS — defaults taken until he answers: Q2 come back on the last venue + "SYNC +N" badge; Q3 glide; Q4 the
   BPM panel (S5 only); Q5 a new venue starts from the current setting; Q6 the beat lands where you pressed.
3. FENCE (concurrent BUILD lanes): mkvidx (src/media/VideoPlayer* decode/seek, GopCache*, probe-video* / probe-vupload*,
   rendering.md, pitfalls.md 62 / 64, one CLAUDE.md index line) and ui (src/media/VideoInfo.h + VideoPlayer open-time
   getter, DeckTabRow / DeckView / MainComponent rename + Show in Finder wiring, ApiServer test routes, ClipInspector info
   line, probe-ui-files-rename.sh). Where you must edit MainComponent.cpp / ApiServer.cpp / ApiServer.h / CLAUDE.md /
   pitfalls.md / tests/CMakeLists.txt, keep hunks small and self-contained (new functions / routes appended, not
   interleaved); whoever merges second rebases. A new pitfall: write "Pitfall NN" (Harmony assigns at merge; 64 is
   mkvidx's).
4. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b
   incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release.
5. Sacred Rules: zero allocation on the analysis thread at any dial value or venue switch; the render thread never waits;
   no new mutex. MERGE by Harmony.
