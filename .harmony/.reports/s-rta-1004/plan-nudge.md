# PLAN nudge -- the beat nudge ("off beat by [+/- X] ms") -- lane "nudge", s-rta-1004

Author: architect (read-only). Pins checked: main HEAD = 185147b and `git status --short -- src tests docs CMakeLists.txt`
printed nothing (plain-file reads below are reads of 185147b); lane/bf2 = 740b6d6 clean; lane/bf2-keys = 9eab9bd clean.
Nothing was built, run or launched. Labels: VERIFIED = read at the file:line given; SHEET = a fact-sheet row its
VERIFICATION section did not overturn (FB = facts-beat-controls.md, FS = facts-syncdial-parts.md, FV = facts-saves.md);
INFERRED / ASSUMED are written where they are used. Paths are relative to /Users/boriskarpman/projects/RealTimeAudio.
Status of this document: DONE (sections 1-9 complete). What I could not establish by reading is listed in section 8 "NOT VERIFIED".

## 1 GOAL

Boris (binding-decisions.md, 2026-10-04 sections; boris-feedback-backlog.md BF55, BF59, BF60):
- "user can nudge main bpm forward or back and this needs to be displayed as "off beat by [+/- X] ms'"
- "You are exactly right. A nudge just moves the placement of the downbeat in time not the tempo."
- "If we are shifted forward or back, everything that is connected to BPM shifts forward or back. I mean everything. If the
  user twist the knob in real time, or triggers a clip, that is not affected unless it's set to be quantized"
- "If I tapped the tempo again, to set the tempo, the time does not change. If I press re-sync, then it does re-sync and that
  changes by how far off the beat we are."
- "46 default" (question 46 as asked: "The beat circle in the top bar, after a nudge: A (default) It moves with the nudge").
- "Key and pad is fine, knobs can skip the sync"

One signed number of milliseconds (the nudge, -500..+500, whole ms) moves the app's beat earlier or later. It is applied in
ONE place, on the analysis thread, just before the snapshot is published, so every reader of the beat sees the same shifted
beat. The tempo is untouched. The top bar shows "off beat by +12 ms" with an earlier and a later control. It is saved with
the show, can sit on a key or a pad, is not a recorded action, and is proven by unit cases on the field arithmetic plus
live rows Harmony runs.

Harmony constraint (lane nudge): ONE shifted beat for every reader; the published beat fields stay mutually consistent
after a shift in either direction (no beat fired twice, none skipped, no count running backwards); the step is 1 ms
whatever the 10.67 ms hop; at 0 the published values are bit-identical to today's and a gate proves it; what follows the
sound itself never shifts; a knob turned or a clip fired by hand is not shifted, and a quantised fire lands on the shifted beat.

## 2 ESTABLISHED FACTS (verified lines only)

F1  The beat fields are copied from the tracker into the snapshot at stage 5: bpm, beatPhase, trackerState, beatInBar,
    barPhase, downbeatDetected, barCount, totalBarCount, resyncBarOrigin, totalBeatCount, trackerRequestSeq, phrasePhase
    (src/analysis/AnalysisThread.cpp:203-214). The snapshot is published once per hop at
    `featureBusWriter_.publishWrite()` (AnalysisThread.cpp:348). VERIFIED.
F2  No other analysis stage reads the snapshot's beat fields between stage 5 and the publish: `grep snap->beat|bar|totalB|
    phrase|downbeat` over AnalysisThread.cpp hits only lines 204-214. VERIFIED (one grep; the stages' own classes were not
    opened: they take `mag` / chroma inputs, not the snapshot -- INFERRED for "no stage reads them through another path").
F3  The tracker's phase advances per hop by hop / period, counts every whole beat crossed, and hard-realigns to 0 on a
    confident detected beat; a realign from the second half of a beat adds one to totalBeatCount, one from the first half
    adds none (src/analysis/BPMTracker.cpp:207-258). In Manual a detected beat never moves the phase (BPMTracker.cpp:99-104).
    VERIFIED.
F4  beatInBar is NOT derived from totalBeatCount. With real onsets it advances in `scoreBeat` on the hop a beat is detected
    (BPMTracker.cpp:365-403); while no onset can arrive (Manual, held silence, locked with no raw BPM) it advances on the
    predicted phase wrap (`advancePredictedBeat`, BPMTracker.cpp:260-277). Before the downbeat is locked, beatInBar =
    beats scored mod 4 and downbeatDetected is false (BPMTracker.cpp:393-402). barCount / totalBarCount advance only on the
    rising edge of downbeatDetected (BPMTracker.cpp:505-515). barPhase = (beatInBar + phase) / 4 (BPMTracker.cpp:481-497);
    phrasePhase = (barCount mod phraseBars + barPhase) / phraseBars (BPMTracker.cpp:540-548). VERIFIED. Consequence
    (INFERRED from those lines, not run): in Auto, when an onset arrives a few hops after the phase wrap, the tracker's own
    published barPhase steps back a quarter bar for those hops; this exists today, un-nudged.
F5  Resync: realign + beatInBar 0, barPhase 0, downbeat level true, barCount 0, phrase 0, resyncBarOrigin = totalBarCount;
    totalBarCount is never rewound (BPMTracker.cpp:566-578). Tap = tempo + realign; typed BPM / REST / OSC / Link = tempo
    only (BPMTracker.cpp:580-622). VERIFIED.
F6  Every tempo / phase control funnels through `MainComponent::applyTempoCommand(action, bpm, origin)`
    (src/MainComponent.cpp:5764-5796); a take replays "tempo" actions through it with `Origin::Replay`
    (MainComponent.cpp:1978-1982); Link calls it about 30 times a second with "link" (MainComponent.cpp:4349-4356). VERIFIED.
F7  The only users of the tracker object outside the analysis thread are applyTempoCommand (requests only) and the take arm
    (reads `postedRequestSeq()` only): `grep getBpmTracker()` = MainComponent.cpp:5766 and :5896. No reader takes a beat
    value from the tracker directly; every beat reader reads FeatureSnapshot. VERIFIED.
F8  Beat readers (SHEET FB table T2, re-read section A "other lines spot-checked, all CONFIRMED"): quantised fire and
    autopilot (src/model/Autopilot.cpp:72-110; Layer.h:449-482), beat-snap seek (MainComponent.cpp:4866-4885), routines
    (src/recording/RoutineEngine.cpp:226-250, 535-548), take clock (src/recording/RecorderClock.cpp:11-40 -- VERIFIED by
    me at :15, `raw = totalBeatCount + beatPhase`), slideshow (MainComponent.cpp:4515-4535), beat-sync randomize
    (MainComponent.cpp:4685-4697), MilkDrop playlist (Renderer.cpp:552-568), shader uniforms (EffectChain.cpp:349-353,
    CompositorEngine.cpp:1910-1912, ProceduralSource.cpp:170-184), mapping sources (MappingEngine.cpp:84-87), oscillator /
    envelope signals (OscillatorSignal.h:25-66, EnvelopeSignal.h), connections (src/connect/ConnectionEngine.cpp:98-101),
    PresetSelector's barPhase wrap test (src/sources/PresetSelector.cpp:78-79), the top bar wheel and "Bar N"
    (src/ui/TopBar.cpp:297-305, 442-526), REST /api/features (ApiServer.cpp:229).
F9  `FeatureSnapshot::barsSinceResync()` is guarded: origin > count reads 0, never a wrapped value
    (src/analysis/FeatureSnapshot.h:153-156). sizeof(FeatureSnapshot) == 384 with trackerRequestSeq at offset 328
    (FeatureSnapshot.h:193-196): bytes 332..383 are free. VERIFIED.
F10 Test mode never starts the analysis thread; the TestServer writes the bus (MainComponent.cpp:5892-5894 comment;
    FeatureSnapshot.h:143 "0 ... in test mode (no analysis thread)"). `POST /api/inject_features` writes beatPhase /
    beatInBar / totalBeatCount straight onto the bus (SHEET FB VERIFICATION B2: ApiServer.cpp:957-1045; TestServer.cpp:495-525).
F11 Top bar: 15 Hz timer copies bpm / trackerState / beatInBar / barPhase / beatPhase / barCount from `featureBus_.read()`
    and repaints the wheel area (TopBar.cpp:289, 297-311). Layout left to right in `resized()` (TopBar.cpp:528-631):
    Audio 38 + 90, Gain 30 + 70, Play / Pause / Stop 3 x 24, wheel 26, Bar 44, tempo 50 + 64, Tap 32, Resync 50, Manual 80,
    Link 50, BPM field 60 (Manual only), five multiplier buttons 26 each, Quantize 55 + 100; from the right: DSP 55, FPS 45,
    Outputs 100, Master 90 + 42, Master Signal 90 + its label. The window opens at 1280 x 800 (MainComponent.cpp:2438); the
    bar is 34 px high (MainComponent.cpp:2688). VERIFIED. Sum of the fixed widths (my arithmetic, INFERRED, not captured):
    left block about 1035 px without the BPM field, right block about 516 px: more than 1280. The bar is already over its
    width at the default window size; the right-hand items take what is left.
F12 The five "/4 /2 x1 x2 x4" buttons write `composition_.bpmMultiplier` and nothing reads it (SHEET FB VERIFICATION A8;
    TopBar.cpp:403; Composition.h:142, 229, 741, 879). VERIFIED for the Composition.h lines.
F13 A composition is one JSON object; a top-level scalar is written at Composition.h:736-744 and read at :874-882; New /
    reset sets defaults at :222-230. A staged load swaps the model at `finishStagedLoad` -> `swapCompositionModel`
    (MainComponent.cpp:3318-3355). There is no dirty flag, no autosave (SHEET FV section 4 item 6). VERIFIED for the lines.
F14 Bindings: a bound key fires its action on every `keyPressed` (MainComponent.cpp:4100-4110 ->
    BindingManager.cpp:52-76, action value 1.0); key releases are polled only for Momentary bindings
    (MainComponent.cpp:4120-4144). VERIFIED. That the OS key auto-repeat reaches `keyPressed` is ASSUMED (JUCE behaviour,
    not read in the JUCE source).
F15 Bindings are written to disk only by Export Bindings (SHEET FV row A9). Question 50 (open) decides whether the app
    remembers them.
F16 The MIDI-learn title on main is a narrow string literal holding an em dash: `"MIDI Learn Mode — Click a target, then
    send MIDI"` (src/ui/MidiLearnOverlay.cpp:93-96). VERIFIED. That it paints wrongly is INFERRED (a `const char*` given to
    juce::String is not read as UTF-8); no capture exists.
F17 The stopped branch's parts (SHEET FS, VERIFICATION V1 1-15 confirmed), and read by me in the bf2 worktree:
    `BeatLead` (src/analysis/BeatLead.h/.cpp, commits 5a46c07, 4ab00bd, d172156) rewrites nine beat fields for a lead
    >= 0 only -- `simulate` returns its input for delta <= 0 (BeatLead.cpp, "if (!(delta > 0.0)) return s;"); it needs
    `BPMTracker::frameEpoch()` / `appliedResyncs()` / `predictedBeatRegime()` (lane diff of BPMTracker.h, +14 lines);
    `SyncSlew` (bc1d89f) glides the applied ms at 0.25 ms per ms; `Binding::Action::SyncNudge`, `targetSyncStepMs`,
    `syncNudgeDeltaMs` (e15d1d0); `bindingIsLive`, `BindingManager::learnMidiCC`, `titleText`, `selectAt`, `bindingTag`,
    `stateForTests`, the routes `POST /api/debug/binding_action`, `POST` + `GET /api/debug/midi_learn` (df98f78, on
    lane/bf2-keys only). Main already has `downbeatLocked()` and `phraseBars()` (src/analysis/BPMTracker.h:109, 120) and
    does NOT have `predictedBeatRegime()` (grep: only the member at :237). VERIFIED.
F18 Rulings: H-17 (the lane is superseded; nothing merges as it stands; parts are carried or dropped by the replacement
    plan), H-16 (three open items: the ML-1 label bar false on 15 old 32-px buttons; no test route for the keyboard bind
    overlay; the learn title's dash) -- .harmony/.reports/s-rta-1003b/rulings-bf2.md:118-146. VERIFIED.
F19 Live-gate precedents on main: .harmony/probe-manual-bpm.sh (real app, `open -g`, port 7070, the live lock) and
    .harmony/probe-beatclock.sh; .harmony/probe-quit-ours.sh (quit only the pid launched). Old probes still kill by name
    (SHEET FS V4 item 4): a new probe sources probe-quit-ours.sh and copies no kill line. VERIFIED that the files exist.
F20 No user manual and no Help menu exist (SHEET FB Q6, re-read A15).

## 3 ITEMS

### NB1 WHERE THE OFFSET LIVES

VERIFIED: F1-F5, F7, F17.

Forks.
 (a) Inside the tracker: add the offset to `phase_`. The tracker realigns to 0 in three places (a confident beat, Tap,
     Resync: F3, F5) and each would have to realign to the offset instead; the offset in beats changes with the tempo, so
     every tempo change would have to re-seat the phase; beatInBar is driven by onset scoring (F4), so the tracker's own
     downbeat statistics would be computed on a moved beat; and "at 0 bit-identical" would have to be proven through the
     whole tracker.
 (b) Rewrite the beat fields where the snapshot is published. One pure step between stage 14 and `publishWrite()`. The
     tracker is untouched and keeps following the sound; every reader (F8) reads the bus, so every reader gets the same
     fields; Auto's re-alignment cannot erase it, because the shift is re-derived from the tracker's position on every hop.
 (c) At each reader: more than fifteen readers on three threads (F8), four reading styles (counter delta, wrap test, raw
     phase, own continuous beat). One missed reader breaks the Harmony constraint silently.

CHOICE: (b). Runner-up (a) loses because it entangles the tracker's detection state with a user setting, needs three
realign sites and every tempo path changed, and cannot be shown bit-identical at 0 by a bypass. (c) is rejected outright by
the Harmony constraint.

THE RULE (new class `BeatShift`, src/analysis/BeatShift.h/.cpp; analysis thread only; plain value state; no allocation,
no lock, no system call).

 Sign: D = the nudge in ms. D > 0 = the app's beat lands D ms LATER than the tracker's; D < 0 = earlier (question 61).
 Inputs per hop: S = the tracker's nine beat fields as stage 5 wrote them; bpm = snap.bpm; the target D (whole ms) and a
 snap sequence, both in ONE `std::atomic<uint32_t>` word on AnalysisThread (low 16 bits the signed ms, high 16 bits the
 snap sequence) written by the message thread and loaded once per hop, relaxed -- the established UI -> hot path channel
 (Sacred Rule 2); no new mutex; flags: barsAdvance = `predictedBeatRegime() || downbeatLocked()`, phraseBars.

 1. APPLIED VALUE. Da (double, ms) glides toward D by at most 0.25 ms per ms = 2.667 ms per hop (the hop is 512 / 48000 s
    on the fixed internal rate, so the step is a constant per processed hop; no wall clock). It SNAPS (Da = D at once) on
    the first hop ever and whenever the snap sequence changed (a loaded show, New, a Resync). So a 1 ms press is fully
    applied at the next hop, a held key (about 30 ms per second) never lags, and a typed 500 glides for 2 s with the beat
    running at 0.75x (going later) or 1.25x (going earlier): never a freeze, never a jump.
 2. THE SHIFT IN BEATS. delta = -Da x bpm / 60000 (beats; 0 when bpm is not a positive finite number). At 200 BPM and 500
    ms that is 1.667 beats: the whole-beat carry w lies in -2..+2.
 3. TARGET. x = S.beatPhase + delta (double); w = floor(x); phase' = x - w (a float that rounds to 1.0 is the next beat:
    phase' = 0, w + 1). count' = S.totalBeatCount + w in signed 64-bit. beatInBar' = (S.beatInBar + w) mod 4 (always 0..3);
    bw = floor((S.beatInBar + w) / 4) (-1, 0 or +1). If barsAdvance: barCount' = S.barCount + bw, never below 0;
    totalBarCount' = S.totalBarCount + bw; if not barsAdvance (Auto before the downbeat lock: the tracker itself counts
    no bars, F4) both bar counts pass through. downbeatDetected' = S.downbeatDetected when w = 0, else
    (barsAdvance && beatInBar' == 0). barPhase' and phrasePhase' are recomputed from the primed values with the tracker's
    own two formulas (F4), in float, exactly as BPMTracker::updateBarPhase / updatePhrase do.
    resyncBarOrigin' = min(S.resyncBarOrigin, totalBarCount' as published) -- `barsSinceResync()` can then never see an
    origin above the count (F9 already guards the read; this keeps the fields honest).
 4. GUARDS against F, the nine fields published on the previous hop:
    - NO COUNT BACKWARDS, NO BEAT TWICE (hold). If count' < F.totalBeatCount, the shifted view has already fired a beat the
      target has not reached (the tracker stepped its own position back -- a first-half realign, a tempo change -- or the
      app has just started with a later nudge): F is REPUBLISHED UNCHANGED until the target's count reaches F's. The hold
      lasts at most as long as the step back: under half a beat for a realign, |D| ms after a snap to a later value.
    - Within one beat the phase MAY step back, exactly as the tracker's own phase restarts on a first-half realign (F3).
      The shifted beat behaves as the tracker's beat does today, displaced; it is not made stricter than today.
    - NONE SKIPPED (forward cap). If count' > F.totalBeatCount + 1, w is reduced so that count' = F.totalBeatCount + 1 and
      phase' = 0; the rest is paid one beat per hop. Reached only when a tempo change itself moves delta by more than a
      beat (a Tap from 60 to 200 with 500 ms set); the glide in rule 1 can never cause it.
    - totalBarCount' = max(F.totalBarCount, target) -- S168's "never rewound" holds for the shifted count too.
 5. IDENTITY. `apply` is a no-op that writes NO byte when: it has never been engaged and D == 0; or Da == 0, D == 0 and
    the target equals S with no hold or cap pending (then it disengages). While engaged it writes only the nine beat
    fields. A tenth field, `float beatNudgeAppliedMs` at offset 332 (F9: free space; sizeof stays 384, static_assert
    updated), is written by AnalysisThread every hop with Da -- 0.0f at rest, the same bits the cleared snapshot holds
    today (INFERRED: the padding of a `clear()`ed snapshot is zero; the unit gate T-N10 proves it on the real object).

 HOW 1 ms IS KEPT. The hop decides only WHEN a snapshot is published, not WHAT phase it carries: on every hop that is not
 in a hold, (count' + phase') - (count + phase) = delta exactly (float rounding aside), i.e. the published beat position
 is D ms from the tracker's at any D, any tempo, either sign. A reader that samples the phase (shaders, mappings, signals,
 the take clock) sees the 1 ms. A reader that waits for a count edge (quantised fire, autopilot, routines, slideshow) sees
 edges on the hop grid as it does today; its edge moves by D on average and by D +/- one hop on a single beat. That is
 today's edge granularity, not a new one, and it is stated to Boris in section 6.

 ACROSS A BAR LINE. beatInBar', both bar counts and the downbeat level carry with w (rule 3), so with D = +250 ms at 120
 BPM the published bar turns half a beat after the tracker's bar, together with the beat count: one edge, one bar.
 WHILE THE TEMPO CHANGES. delta is re-derived from the current bpm every hop: the shifted beat is always D ms from the
 tracker's beat at the tempo in force. A tempo step moves delta by D x (change in BPM) / 60000 beats at once: toward
 "later" that is a phase step back inside the beat or a short hold (guard 4), toward "earlier" a step forward (cap).
 A Link ramp moves it by a thousandth of a beat per hop: the beat just runs a hair slower or faster.
 MANUAL / AUTO / LINK. The rule does not know the mode. In Auto the tracker realigns to each confident onset (F3) and the
 shifted beat is re-derived D ms from it on the same hop: Auto cannot erase the nudge. In Manual and under Link the tracker
 free-runs and the shifted beat runs D ms beside it.

 CARRIED FROM `BeatLead` (5a46c07, 4ab00bd; read in the bf2 worktree): the `Beat` struct of the nine fields with `beatOf`
 / `writeBeat`; `kBeatFields` and its static_asserts ("apply writes no other byte"); `nonBeatCrc`; the float recompute of
 barPhase / phrasePhase; the hold idea; the identity / engaged rule; the one-line accessor
 `bool BPMTracker::predictedBeatRegime() const` (BPMTracker.h, no behaviour change). The test file's helpers for building
 tracker sequences are a model for T-N cases (tests/test_beat_lead.cpp, 1029 lines), re-typed, not merged.
 DROPPED: `frameEpoch_` and its seven increment sites in BPMTracker.cpp, `appliedResyncs()`, the excess counters beatX /
 barX / originX, the re-base, the lead slew in beats, `fold`, the `Diag` block, `SyncOffset.h`, `SyncSlew` as a class (its
 rule is rule 1 above with a constant per-hop step), `AnalysisDelayLine` and the whole LATE half (it delayed the SOUND:
 loudness and onsets would shift, which Boris's nudge must not do).
 WHAT "LATER" NEEDS THAT BeatLead LACKS: a backward walk (w < 0: beatInBar and the bar counts carried DOWN, floors at 0, the
 origin clamp), a hold at launch (count' below 0), and counters that may sit BELOW the tracker's -- BeatLead's excess
 counters only ever add. One signed rule replaces its forward-only walk; that is why it is re-written, not generalised.

Change by file / function:
- src/analysis/BeatShift.h/.cpp (new): `struct Beat`; `static Beat beatOf(const FeatureSnapshot&)`;
  `static void writeBeat(FeatureSnapshot&, const Beat&)`; `static Beat shifted(const Beat& S, double deltaBeats, bool
  barsAdvance, int phraseBars)` (rule 3, pure); `double stepApplied(int targetMs, bool snap)` (rule 1);
  `void apply(FeatureSnapshot&, int targetMs, uint16_t snapSeq, bool barsAdvance, int phraseBars)` (rules 1-5);
  `bool engaged() const`; `double appliedMs() const`; `static uint32_t nonBeatCrc(const FeatureSnapshot&)`.
- src/analysis/BPMTracker.h: `bool predictedBeatRegime() const` only.
- src/analysis/FeatureSnapshot.h: `float beatNudgeAppliedMs` at offset 332 + static_assert (FeatureSnapshot rule 8: the
  field is added to the snapshot before anything uses it).
- src/analysis/AnalysisThread.h/.cpp: `void setBeatNudge(int ms, bool snap)` (any thread: packs the word, bumps the snap
  sequence when asked); one `BeatShift` member; the call immediately before `publishWrite()` (after stage 14, so no
  analysis stage ever sees shifted fields); `snap->beatNudgeAppliedMs`.
- CMakeLists.txt, tests/CMakeLists.txt: the new source and test target.

RED-first tests and mutants: section 5, T-N1..T-N14, mutants M1..M10. Gate rows: G-N1 (unit), L1, L2, L4, L10.

### NB2 WHAT IT MUST NOT MOVE

Stays with the sound (never written by BeatShift; `nonBeatCrc` proves it per hop in T-N9): loudness (rms, peak, bands),
onsetDetected / onsetStrength / onsetCount and their readers (src/features/OnsetPulse.h, Renderer.cpp, CompositorEngine.cpp,
EffectChain.cpp, ProceduralSource.cpp, EmbeddedShaders.h, RecorderHost.cpp -- grep list, VERIFIED as files; lines not
opened), spectrum, MFCC, chroma, key, structural state, genre, the advanced analysis fields, `bpm`, `trackerState`,
`trackerRequestSeq`. The tempo number in the top bar reads `bpm` and so does not change (his "not the tempo").

Shifts (reads the nine fields; all on the bus, F7, F8): every reader in F8.

By hand, immediate, not shifted: a knob / fader / REST / OSC / MIDI value write is applied when it arrives -- none of those
paths reads the beat. A clip fired with Quantize Off and no per-clip snap is triggered at once
(`handleClipTrigger` immediate path; Layer.h:449-482 holds only QUEUED triggers for a beat edge). A fire that is set to be
quantised (global Quantize Next Beat / Next Downbeat; a clip's Beat / Bar / 2 Bar / 4 Bar snap) waits for the edge of the
PUBLISHED count (Autopilot.cpp:72-91), which is the shifted beat. Nothing to change in those files.

Ambiguous readers, ruled:
- The take recorder's beat stamps (RecorderClock.cpp:15): SHIFTED. It reads the bus like every reader; a take is stamped
  in the beat the pictures followed. Its "reset" arm (RecorderClock.cpp:62-70: raw < lastRaw absorbed) already absorbs a
  phase step back; a hold is a flat stretch. One addition: a change of `beatNudgeAppliedMs` writes an anchor
  ("nudge"), exactly as a bpm change does (RecorderClock.cpp:73-77), so the beat <-> sample map does not wait up to 32
  beats (`kPeriodicAnchorBeats`, RecorderClock.h:77) to learn that the beat moved.
- Routines replayed from a take: SHIFTED, by reading the bus (RoutineEngine.cpp:226-250, 535-548). They land on today's
  beat with today's nudge (NB7).
- Link: tempo only (F6; Link's beat phase has no caller). The nudge applies on top of whatever tempo Link gives. If Link's
  phase is ever followed, it feeds the tracker and the nudge still applies at publish: no change here.
- The inject / test path (F10): NOT shifted. An injected snapshot states the published values; in test mode there is no
  analysis thread, so BeatShift never runs, `beatNudgeAppliedMs` reads what was injected (0 unless named). A probe that
  wants a shifted clock injects one. `GET /api/beat_nudge` answers "analysisRunning": false there. Row L5 pins it; the new
  Pitfall 68 says it.
- The take arm's "start after tracker request" (MainComponent.cpp:5892-5897): untouched; the nudge is not a tracker request.

Tests: T-N9 (non-beat bytes equal), L3 (quantised vs by-hand fire), L5 (inject). Mutant M11: BeatShift also writes
`onsetCount` -> T-N9 RED.

### NB3 THE CONTROLS AND THE TEXT

VERIFIED: F11, F12; Pitfall 6 (ASCII glyphs on small buttons, docs/claude/pitfalls.md:21); UI rules in CLAUDE.md.

Sign convention (ruled; the direction is also question 61): "+" = later, "-" = earlier. The earlier control sits on the
LEFT of the text, the later control on the RIGHT, as a number line.
Exact text (ASCII only; a hyphen-minus, never a typographic minus): "off beat by 0 ms" at rest; "off beat by +12 ms";
"off beat by -7 ms"; at the ends "off beat by +500 ms" and "off beat by -500 ms". The text shows the TARGET the moment it
changes (never the gliding value). It is a state display, always shown; it never announces an event.
The two controls: text buttons "-" and "+" (Pitfall 6), 20 px each; one click = 1 ms; held = repeats
(`juce::Button::setRepeatSpeed`: first repeat after 400 ms, then every 50 ms = 20 ms per second); at an end the press
does nothing. Tooltips: "Move the beat 1 ms earlier. Hold to keep moving." / "Move the beat 1 ms later. Hold to keep
moving." / on the text: "How far the beat is moved from the detected or tapped beat. Click to type a number. Right-click
for 0."
Typed value: the text is an editable label (click to edit). The editor opens holding only the signed number, all
selected; Enter commits: the first whole number in what was typed, clamped to -500..+500; anything without a number leaves
the value unchanged; Esc cancels. Right-click on the text = 0 (the app's "right-click resets" habit; not a slider, so no
ResettableSlider is involved -- no slider is added by this lane). A typed value glides (rule 1).
No knob, no fader (his "knobs can skip the sync").

Width budget (F11: the bar is already over 1280 px -- INFERRED arithmetic; S0 captures it). The nudge group needs
20 + 2 + 118 + 2 + 20 = 162 px (118 = "off beat by -500 ms" at the bar's 11 pt font plus 8 px; the builder measures the
string with `GlyphArrangement::getStringWidthInt` as TopBar.cpp:622-628 does and uses max(118, measured + 8)). Placement:
directly after the tempo number and its state word, BEFORE Tap -- "beside the tempo". Under question 62's default A the
five dead buttons are hidden and their 140 px pay for it (net +22 px on the left block). Under 62 B they stay (and are
wired by another lane) and the bar grows by 162 px: then the group moves to the row's one flexible place by ONE constant
(`kNudgeAfterQuantize`), nothing else changes. Until Boris answers 62, default A is built: the five buttons are
`setVisible(false)` and take no width; their members, `bpmMultiplier` and its save / load stay (F12, F13), so wiring them
later or deleting them is a small change either way. This is the default of an asked question, shown to him in the
capture set, not a silent removal.
The BPM edit field (Manual on) keeps its place; with Manual on the left block is 60 + 4 px wider, as today.

The 15 Hz timer and "it moves with the nudge" (his answer 46). The wheel and "Bar N" read `featureBus_.read()`
(TopBar.cpp:297-305): they show the shifted beat with no change. The wheel repaints every 67 ms; a nudge of a few ms
cannot be SEEN on it and is not meant to be: the number is the instrument, the wheel shows which beat of the bar the app
is on. The timer rate is NOT raised in this lane (Pitfall 57: a faster timed repaint costs the whole window; an
own-layer wheel is a separate piece of work). The nudge text is set only when the value it would paint differs from what
it paints (Pitfall 59), from the same timer (Pitfall 41: a key, a pad, REST or a loaded show can change it).
Pure model (src/ui/TopBarModel.h, juce_core only, as `barReadoutText`): `juce::String offBeatText(int ms)`;
`bool parseOffBeat(const juce::String& typed, int& msOut)`; `int clampNudge(int ms)`.

States the visual gate must capture (window-only captures of the lane's own app, per the SCREEN-SAFETY LAW and RIG-RULES):
 V1 at rest "off beat by 0 ms", Auto, LOCKED;  V2 "+12";  V3 "-7";  V4 "+500";  V5 "-500";
 V6 the editor open with the number selected;  V7 Manual on (BPM field shown) with "-500" (the widest left block);
 V8 no tempo (SEARCHING, "---") with "+12";  V9 the default build's disabled Link beside it;
 V10 the window at 1280 wide and V11 at the width Boris runs (Harmony reads it from his last captures; ASSUMED 1728 or
 wider) -- both with Manual on; V12 each of the three tooltips; V13 the "-" button held (pressed look);
 V14 the keyboard bind overlay with the two new targets; V15 the MIDI-learn overlay with the two targets, V16 the same
 with a nudge target selected (title "Send a MIDI note..."), V17 the learn overlay's standing title (the dash, NB6);
 V18 today's top bar at 185147b at 1280 (the baseline, S0), so the critics see what was there before.
 Model facts per state go in the manifest (the nudge value, the label text, each widget's bounds, label width vs box).

Change by file: src/ui/TopBar.h/.cpp (two buttons, the label, the group's bounds in `resized()` lines 564-596 only, the
timer line, callbacks `std::function<void(int deltaMs)> onNudgeStep`, `std::function<void(int ms)> onNudgeSet`,
`std::function<int()> nudgeMs`); src/ui/TopBarModel.h; tests/test_topbar_model.cpp (+ cases).
Tests: T-U1..T-U4; mutants M12, M13. Gate rows: VG (five critic seats), L9.

### NB4 TAP, RESYNC, MANUAL, AUTO, LINK

VERIFIED: F5, F6.
- Tap (button or bound key): the number stays; the tracker gets its tempo + realign as today; the shifted beat is D ms
  from the tapped beat on the next hop (after the short hold or cap of NB1 rule 4 if the tempo moved). No code in the
  "tap" branch.
- Resync by hand (button, key, pad, REST /api/resync, OSC /audiodna/resync): question 49, default A = the number reads 0.
  ONE line in `applyTempoCommand`'s "resync" branch, guarded `origin != Origin::Replay`:
  `setBeatNudgeMs(kNudgeAfterResync, /*snap*/ true)` with `kNudgeAfterResync = 0`. The snap makes the "1" land where he
  pressed, at once (after at most the old |D| ms of hold when the old nudge was "earlier": the beat already shown is not
  taken back). If he answers 49 B, THAT line changes (it is deleted for "the number stays", or its argument becomes the
  value he names); nothing else in the lane moves.
- A REPLAYED Resync (a take or a routine, `Origin::Replay`): the tracker resyncs as recorded; the nudge is NOT touched
  (NB7). Ruled, not asked: a replay must not undo a room correction he made by hand.
- Manual on / off, Auto's own re-alignment, a typed BPM, REST / OSC set_bpm, Link: the number stays; the shifted beat stays
  D ms from the tracker's beat (NB1). No code.
Tests: T-M1..T-M3 (a small pure function `int nudgeAfterTempoCommand(const std::string& action, bool replay, int current)`
in src/model/BeatNudge.h carries the rule so it is unit-tested without MainComponent); mutant M14; live L9.

### NB5 WHERE IT IS REMEMBERED

VERIFIED: F13. Harmony's reading R28 told to him and not corrected: with the show.
- Field: `int beatNudgeMs = 0` on Composition (message-thread model field; the analysis thread never reads the model, it
  gets the packed word). Saved as the top-level key "beatNudgeMs" beside "bpmMultiplier" (Composition.h:741); read at
  :879 with a guard: absent or not a number -> 0; out of range -> clamped. Reset to 0 in the New / clear path (:229).
- ONE writer: `void MainComponent::setBeatNudgeMs(int ms, bool snap)` clamps, stores `composition_.beatNudgeMs`, calls
  `analysisThread_.setBeatNudge(ms, snap)`. Every source goes through it (top bar, key, pad, REST, Resync, load).
- Open / New / staged load: after the model swap (`swapCompositionModel` -> `refreshUiAfterModelSwap`,
  MainComponent.cpp:3349-3356 and the kCompNew path at :3136 that uses the same mechanism -- INFERRED that one hook covers
  both; the builder confirms and names the line) call `setBeatNudgeMs(composition_.beatNudgeMs, /*snap*/ true)`. During
  the staging window the old show's nudge stays live (Pitfall 58). Load Deck / Duplicate Deck (an append, not a swap) do
  NOT touch it. An old show without the key reads 0. No on-screen text on any of these.
- At launch (no show opened): 0.
- Not undoable (Cmd+Z never changes it: it is live performance state, as the tempo is); not a ControlPath.
- No dirty flag exists (F13): a nudge changed and not saved with the show is lost at quit, as every other show field is
  today. The quit window lane owns "unsaved progress"; nothing here.
Tests: T-C1..T-C3 in tests/test_composition.cpp; mutant M15; live L6.

### NB6 A KEY OR A PAD

VERIFIED: F14, F15, F16, F17, F18. His words: "Key and pad is fine, knobs can skip the sync".
Carried from lane/bf2-keys (e15d1d0, df98f78), renamed, re-typed onto main (no cherry-pick: the commits also carry the
dial's controller, routes and OSC):
- `Binding::Action::SyncNudge` -> `Binding::Action::BeatNudge` (appended last, so saved bindings keep their numbers);
  `targetSyncStepMs` -> `targetNudgeStepMs` (-1 earlier, +1 later; saved as "targetNudgeStepMs", absent -> +1);
  `syncNudgeDeltaMs` -> `beatNudgeDeltaMs(const Binding&, float value)`: a key or note press (value > 0) -> the step;
  a release -> 0; a MIDI CC -> 0.
- `bindingIsLive(const Binding&)`: SIMPLER than the lane's -- not live = BeatNudge on ANY MIDI CC. The lane let a Relative
  CC through for a hand-made file; his words say knobs skip it, so the Relative arm in `processMidiCC` is DROPPED.
- `BindingManager::learnMidiCC(const Binding&)` as it is (refuses a binding that is not live BEFORE removing anything).
- Overlay targets in `MainComponent::buildBindableTargets` (the global row, after Resync; main :7578-7582): "Beat earlier"
  and "Beat later" (was "Sync -1 ms" / "Sync +1 ms"); `BindingOverlay::BindableTarget::syncStepMs` -> `nudgeStepMs`; the
  match in both overlays on the step.
- `MidiLearnOverlay::titleText()`, `selectAt`, `bindingTag`, `stateForTests` as they are (df98f78), with the title for a
  nudge target "Send a MIDI note..." and the standing title in ASCII (below).
- `MainComponent::handleBindingAction` case: `if (int d = beatNudgeDeltaMs(binding, value); d != 0)
  setBeatNudgeMs(composition_.beatNudgeMs + d, false);`.
- TEST-SERVER routes (compiled out otherwise): `POST /api/debug/binding_action` (accepts "beatNudge" and "tapTempo";
  400 otherwise), `POST` + `GET /api/debug/midi_learn` -- as df98f78, handler names unchanged.
- tests/test_binding_sync_nudge.cpp -> tests/test_binding_beat_nudge.cpp: the 11 cases re-typed; the Relative-CC cases
  become "a CC, Absolute or Relative, does nothing and cannot be learned".
Dropped: `SyncOffsetController`, `SyncVenues`, the `/api/sync*` routes, OSC `/audiodna/sync*`, probe-sync rows R13 / R14
as files (their two shapes are re-used as L7 / L8), `AppSettings::kSync`.
A held key keeps moving through the OS key repeat (F14; ASSUMED that JUCE passes repeats to `keyPressed` -- row L7b reads
it off a real held key, which only Boris can press: section 6). A held PAD sends one note-on: one press = 1 ms; whether a
held pad should repeat is question 63 (default: no).
Where the bindings are kept between launches is question 50; this lane adds nothing to it: the two targets are ordinary
bindings and follow whatever 50 decides. Line that changes: none here.

H-16's three items, ruled (the overlay targets are carried):
 1. ML-1 "every label_w <= its w - 4" is false today on 15 old 32-px per-layer buttons. RE-STATED: "each of the two nudge
    targets has label_w <= w - 4; the count of targets failing that bar is 15 (H-16's count) and none of them is a
    nudge target" (a pinned baseline; a 16th FAILS; the 15 labels are printed as INFO by the first run -- they were not
    read for this plan). Fixing the 15 is not in this lane.
 2. The keyboard bind overlay gets a test route: `POST /api/debug/bind_overlay` {"op": "open" | "close" | "select",
    "label"} and `GET /api/debug/bind_overlay` (state: active, targets with label / x / y / w / h / label_w / bound key
    text), TEST-SERVER only, the same shape as midi_learn. `BindingOverlay::stateForTests()` and `selectAt()` are added
    the same way df98f78 added them to the learn overlay. No synthetic input is ever used.
 3. The learn title's dash: the literal becomes ASCII -- "MIDI Learn Mode - Click a target, then send MIDI". A visible
    text: it is captured (V17) and goes through the visual gate. Runner-up (keep the em dash through
    `juce::String::fromUTF8`) loses to Pitfall 6's rule of ASCII in small UI text and to one less encoding trap.
Tests: T-B1..T-B6; mutants M16, M17; live L7, L8.

### NB7 TAKES AND REPLAY

VERIFIED: F6 (tempo actions are captured and replayed with Origin::Replay), FB re-read C7.
Forks: (a) an action captured and replayed; (b) a setting the take stores once; (c) neither -- a setting of the show.
CHOICE: (c). The nudge is where he stands in a room on a night; a take is what he did to the picture, measured in beats.
Replaying an old room's correction in a new room would move the beat away from the music he hears now. Runner-up (b)
loses for the same reason and would add a second home beside the show's. (a) loses doubly: a held key would write a
point per ms into the take.
What Boris sees on replay: the number does not move; every replayed fire and fader lands on the beat as it is nudged NOW;
a replayed Resync resyncs the beat and leaves the number (NB4). While RECORDING, a nudge he makes is not in the take; the
take's beat stamps follow the nudged beat (NB2) and an anchor is written so the take's audio and beats stay paired.
Change: none beyond NB2's anchor and NB4's replay guard. `setBeatNudgeMs` never calls the capture path.
Tests: T-R1 (RecorderClock anchor), T-M2 (replay guard); mutant M18.

### NB8 PROOF

Unit (builder, RED first; section 5): the arithmetic T-N1..T-N14; the model T-U, T-M, T-C, T-B, T-R.
REST (production API, port 7070, beside /api/set_bpm): `GET /api/beat_nudge` -> {"ok", "ms" (target), "appliedMs",
"minMs": -500, "maxMs": 500, "analysisRunning"}; `POST /api/beat_nudge` body {"ms": N} sets, {"delta": N} steps; both
answer the GET body; a non-number is 400. The POST goes to the message thread as /api/set_bpm does and calls
`setBeatNudgeMs(.., false)`. `GET /api/features` gains "beatNudgeAppliedMs" from the same coherent snapshot. A probe that
needs "the snapshot reflects my set" waits for features.beatNudgeAppliedMs == ms (the Pitfall 48 habit for a new field).
Live rows (all Harmony's; probe written by a builder, self-tested on synthetic data first -- RIG-RULES A2 "instrument
first"): L1 zero identity; L2 the beat clock moves by D, both signs, 1 ms and 500 ms; L3 a quantised fire lands on the
shifted beat, a by-hand fire does not wait; L4 the bar counter turns with the shifted beat; L5 the inject path is not
shifted; L6 saved with the show; L7 key / pad action; L8 learn refuses a CC; L9 Resync zeroes, Tap and set_bpm leave;
L10 the mutant app (sign flipped) is RED on L2. No row opens an Output window, takes a full-screen capture or sends
synthetic input; the probe sources probe-quit-ours.sh and holds the live lock as probe-manual-bpm.sh does (F19).
Only Boris: the feel against a real room; the text at a glance; a really held key (section 6).

### NB9 STAGES AND ORDER -- section 4. The five dead buttons -- question 62 (default A: hidden). Docs and the manual -- stage S5.

## 4 STAGES + ORDER

One builder context per stage, one worktree + branch for the lane (lane/nudge from main 185147b or its successor); no two
builders in one worktree. A builder builds, runs unit tests and writes probes; it never runs a live row and never gives a
gate verdict. Harmony runs every live row, every mutant-app arm and the visual gate herself.

S0  BASELINE CAPTURE (capture builder; no source change). Owns: nothing in src. Produces V18 (today's top bar at 1280 and
    at Boris's width, Manual off and on) with a manifest of every top-bar widget's bounds. Proves or refutes F11's
    arithmetic ("the bar is over its width today"). If refuted, NB3's width numbers are re-read by Harmony before S3; the
    design does not change.
S1  THE ARITHMETIC. Owns: src/analysis/BeatShift.h/.cpp (new), src/analysis/BPMTracker.h (one accessor),
    tests/test_beat_shift.cpp (new), CMakeLists.txt + tests/CMakeLists.txt (the new source and target only).
    Proves: T-N1..T-N9, T-N11..T-N14 RED on a stub `apply` that writes nothing, then GREEN; mutants M1-M11 each RED.
    Nothing is wired: the app's behaviour is unchanged after S1.
S2  WIRING + SAVE + REST. Owns: src/analysis/AnalysisThread.h/.cpp, src/analysis/FeatureSnapshot.h (the one field),
    src/model/Composition.h ("beatNudgeMs"), src/model/BeatNudge.h (new: `clampNudge`, `nudgeAfterTempoCommand`,
    `kNudgeAfterResync`, the min / max), src/MainComponent.h/.cpp (`setBeatNudgeMs`, the resync line, the load hook, the
    REST callbacks), src/api/ApiServer.h/.cpp (`/api/beat_nudge`, the features key), src/recording/RecorderClock.h/.cpp
    (the "nudge" anchor), tests: test_analysis_nudge_thread.cpp (new: T-N10), test_composition.cpp (T-C),
    test_beat_nudge_model.cpp (new: T-M), test_recorder_clock (T-R1; the builder names the existing file),
    test_feature_bus / the snapshot layout test if one pins offsets; .harmony/probe-nudge.sh + probe-nudge.py +
    probe-nudge-selftest.py (rows L1-L6, L9, L10's harness).
    Proves: the unit cases; the probe self-test "0 case(s) differ". Harmony then runs G-N1, L1, L2, L3, L4, L5, L6, L9, L10.
S3  TOP BAR. Owns: src/ui/TopBar.h/.cpp (the nudge group; `resized()` lines for the tempo cluster only -- from the wheel to
    the multiplier buttons; the five buttons hidden under default 62 A), src/ui/TopBarModel.h, tests/test_topbar_model.cpp,
    MainComponent.cpp (three callback assignments beside :587-601). Proves T-U1..T-U4, M12, M13. Hands to the capture
    builder a TEST-SERVER read of the label: the existing `GET /api/debug/ui_text` gains the key "nudge" (the label's
    painted text) -- ApiServer.cpp handleDebugUiText.
S4  KEY AND PAD. Owns: src/binding/Binding.h, src/binding/BindingManager.h/.cpp, src/ui/BindingOverlay.h/.cpp,
    src/ui/MidiLearnOverlay.h/.cpp, MainComponent.cpp (`buildBindableTargets`, the `handleBindingAction` case, the debug
    callbacks), ApiServer.h/.cpp (the TEST-SERVER routes), tests/test_binding_beat_nudge.cpp (new),
    probe-nudge.py rows L7, L8. Proves T-B1..T-B6, M16, M17. S3 and S4 touch different functions of MainComponent.cpp and
    ApiServer.cpp; they run one after the other in the lane's worktree (S3 then S4), not in parallel.
VG  VISUAL GATE (capture builder, then five critic seats; Harmony's verdict). States V1-V17 plus V18 from S0, a manifest
    with model facts per state, Boris's words verbatim, and the list of pre-existing texts outside the lane (every top-bar
    text that is not the nudge group; the 15 old overlay labels). Runs after S4; Boris sees nothing before it is GREEN.
S5  DOCS + MANUAL. Owns: docs/claude/effects.md ("Manual BPM Mode": what Tap, Resync, Manual, Auto, Link do to the nudge;
    the facts FB section 3 item 2 found missing there), docs/claude/performance-controls.md (the two binding targets;
    Link line 76 gains "the nudge applies on top"), docs/claude/analysis.md (BeatShift: the step before publish, the
    rules of NB1), docs/claude/architecture.md (the snapshot field), docs/claude/integration.md (`/api/beat_nudge`),
    docs/claude/testing-eyes.md (the debug routes; and line 72's kill advice replaced by the own-pid rule -- the lane
    carry dc59573, one line), docs/claude/recording.md (not captured; the "nudge" anchor), docs/claude/pitfalls.md +
    CLAUDE.md index: Pitfall 68 "The published beat is the NUDGED beat: one step (BeatShift) before the publish; a beat
    reader reads FeatureSnapshot, never BPMTracker; an injected snapshot is published as given -- before adding a beat
    reader, a second publisher or a beat field"; CLAUDE.md "Key capabilities" and the top-bar line; .harmony/
    APP-INVENTORY.md (routes, the action, the test count RE-COUNTED on the lane with `ctest -N`, never copied);
    docs/manual/README.md (new: the manual's contents page) and docs/manual/01-line-up-the-beat.md (new, plain words, no
    code names): 1 pick the sound source and wait for LOCKED, or tap / type the tempo; 2 press Resync on the "1" you
    hear; 3 stand where you watch the show from; 4 if the picture's beat comes before the sound's, press "+" (later)
    until they meet; if after, press "-"; hold to move faster; type a number for a big jump; 5 the number is saved with
    the show; Resync sets it back to 0; Tap leaves it; 6 put "Beat earlier" / "Beat later" on a key or a pad; 7 each
    screen's own Delay (the output-settings lane) lines up a screen that is late by itself -- use Delay for a slow
    screen, the nudge for the beat. Step 7 is written only after the output-settings lane names its control; until then
    the entry ends at step 6 with no forward reference.
    The manual is text Boris reads: Harmony reads it against the built app (every control name on screen as written)
    before he sees it.
Order: S0 -> S1 -> S2 -> (Harmony: G-N1, L1-L6, L9, L10) -> S3 -> S4 -> (Harmony: L7, L8, G-N1 again) -> VG -> S5 -> merge.
Reviews: after S2 and after S4, the project's pinned review (realtime lens on S1 / S2: no allocation, lock or system call
in `BeatShift::apply` and the per-hop path; a text scan of BeatShift.cpp for new / malloc / vector / mutex / printf is
T-N14).

What the output-settings lane and this lane must not both touch. This lane does NOT touch: `outputsButton_` and the
right-hand section of `TopBar::resized()` (TopBar.cpp:604-631), src/output/**, OutputWindow, OutputManager, settings.json
(`AppSettings`), the Output menu, Syphon. The output-settings lane does NOT touch: BeatShift, the nine beat fields,
`beatNudgeAppliedMs`, the tempo cluster of `TopBar::resized()` (:553-602), `applyTempoCommand`, Binding.h's action list
(if it needs an action it appends AFTER BeatNudge -- whichever lane merges second appends last and re-runs its binding
tests). Shared files with disjoint hunks: TopBar.cpp, MainComponent.cpp, ApiServer.cpp, docs. The second lane to merge
takes main into its branch as its builder's step 0. The two controls do different jobs and never read each other: a
screen's Delay holds PICTURES back per screen; the nudge moves the BEAT for everything.

## 5 TESTS + GATE ROWS (pre-registered; each with its RED arm; a bar is met or reported, never loosened)

Unit, tests/test_beat_shift.cpp. Sequences are made by driving a REAL BPMTracker hop by hop (512 samples, 48 kHz) the way
tests/test_bpm_stabilization.cpp does, copying its nine fields into a snapshot, then `BeatShift::apply`. "pos" =
totalBeatCount + beatPhase as doubles. RED arm for all of S1 before the class exists: the stub that writes nothing.
T-N1  SIGN + SIZE. Manual 120 BPM, D = +250: on every hop after the first, pos' - pos = -0.5 within 1e-4. D = -250: +0.5.
      Mutant M1 (delta's sign flipped) RED.
T-N2  1 ms. For bpm in {60, 120, 174, 200} and D in {-500, -37, -1, +1, +37, +500}, settled: pos' - pos = -D x bpm / 60000
      within 2e-5 beats on every hop not in a hold; and pos'(D + 1) - pos'(D) = -bpm / 60000 within 2e-5.
      Mutant M6 (the target read as D + 1) RED.
T-N3  EDGE TIME. Manual 123 BPM, 1000 beats, D in {-500, -37, +37, +500}: the mean over beats of (hop index of count' edge
      - hop index of the tracker's edge of the same beat) x 10.6667 ms is within 1.0 ms of D; every single difference is
      within one hop of D. Mutant M1 RED.
T-N4  BAR LINE. Manual 120, D = +250 and D = -250, 64 beats: beatInBar' changes on exactly the hops count' changes;
      it runs 0,1,2,3,0; totalBarCount' and barCount' go up by one on exactly the hops beatInBar' goes 3 -> 0; the
      downbeat level is true exactly while beatInBar' == 0; barPhase' == (beatInBar' + beatPhase') / 4 and phrasePhase'
      == ((barCount' mod phraseBars) + barPhase') / phraseBars bit-for-bit with the float formula.
      Mutants M3 (beatInBar' = S.beatInBar) and M4 (bar counts not carried) RED.
T-N5  NO COUNT BACKWARDS, NONE TWICE. Auto-style sequence: every beat a confident onset arrives 2 hops AFTER the wrap
      (a first-half realign), D in {+250, -250, +3, -3}, 200 beats: totalBeatCount' never decreases; it rises by exactly
      200 +/- 1 over the run (the same as the tracker's own count); totalBarCount' never decreases.
      Mutant M2 (the hold removed) RED on "never decreases".
T-N6  NONE SKIPPED. Tap from 60 to 200 BPM with D = -500, and from 200 to 60 with D = +500: totalBeatCount' rises by at
      most 1 on any hop. Mutant M10 (the forward cap removed) RED.
T-N7  TEMPO CHANGE. 120 -> 126 by `followExternalTempo` with D = +500: from the hop after the change, pos' - pos =
      -500 x 126 / 60000 within 2e-5 once any hold (<= 0.06 beat) has ended; a Link-like ramp 120 -> 130 in 100 steps:
      beatPhase' never steps back and totalBeatCount' never decreases. Mutant M8 (delta computed from the first hop's
      bpm) RED.
T-N8  RESYNC + ORIGIN. D = +250, Resync applied mid-bar: resyncBarOrigin' <= totalBarCount' on every hop; within
      0.5 beat + 1 hop the published state is beatInBar' 0, barCount' 0, downbeat true, and beatPhase' crosses 0 there.
      D = -250: the same state within 1 hop + the hold. Mutant M9 (the origin clamp removed) RED.
T-N9  ONLY THE BEAT. For every hop of T-N4's and T-N5's runs: `nonBeatCrc` before == after. Mutant M11 (apply also
      writes onsetCount) RED.
T-N11 GLIDE. Target 0 -> +500 (no snap): the applied value moves by 2.6667 ms per hop (within 1e-9) and reaches 500.0
      exactly; beatPhase' never steps back and no hold occurs; a snap (sequence bumped) applies in one hop.
      Mutant M7 (no glide) RED on "never steps back / no hold".
T-N12 BACK TO ZERO. +10 then 0: once the applied value is 0.0 and no hold is pending, every later snapshot is memcmp-equal
      to the tracker's own (apply writes nothing; `engaged()` false). Mutant M5 (the glide stops 0.01 ms short) RED.
T-N13 LAUNCH WITH A LATER NUDGE. First hop with D = +500 (snap): totalBeatCount' stays 0 and beatPhase' 0 until the
      tracker's pos reaches the shift; no unsigned wrap. Mutant: count' computed in uint32 -> RED.
T-N14 REAL-TIME TEXT GATE. BeatShift.cpp / .h contain none of: "new ", "malloc", "std::vector", "std::string",
      "std::mutex", "lock_guard", "printf", "std::cout", "juce::". RED arm: a `std::vector` member added.
Unit, tests/test_analysis_nudge_thread.cpp (S2):
T-N10 ZERO = BIT-IDENTICAL. The real per-hop path (the function AnalysisThread calls before the publish) over a
      2000-hop click sequence in Auto and in Manual with the nudge word never written: every published snapshot is
      memcmp-equal (all 384 bytes) to the same run with the BeatShift call compiled out by the test's switch.
      RED arm: mutant M6 (target read as D + 1). And: with the word written 0 after +10 (T-N12's end state) -> equal again.
Model:
T-U1  `offBeatText`: 0 -> "off beat by 0 ms"; 12 -> "off beat by +12 ms"; -7 -> "off beat by -7 ms"; 500 -> "off beat
      by +500 ms"; -500 -> "off beat by -500 ms"; every character is ASCII. Mutant M12 (no "+" on positives) RED.
T-U2  `parseOffBeat`: "12" -> 12; "+12" -> 12; "-7" -> -7; "off beat by -7 ms" -> -7; " 30 ms" -> 30; "900" -> 500;
      "-900" -> -500; "abc" -> false; "" -> false; "1.9" -> 1. Mutant M13 (no clamp) RED.
T-U3  `clampNudge`: -501 -> -500; 501 -> 500; 0 -> 0.
T-U4  the label's change test: given painted "off beat by +12 ms" and model 12 -> no setText; model 13 -> setText once
      (a pure helper `bool nudgeTextChanged(const juce::String& painted, int ms)`; Pitfall 59).
T-M1  `nudgeAfterTempoCommand("resync", false, 40)` == 0 (kNudgeAfterResync).
T-M2  ("resync", true /*replay*/, 40) == 40. Mutant M14 (the replay guard removed) RED.
T-M3  "tap", "manual", "auto", "link" (replay or not, 40) == 40.
T-C1  save with beatNudgeMs 37 -> the JSON has "beatNudgeMs": 37 -> load -> 37. Mutant M15 (the key not written) RED.
T-C2  a JSON without the key -> 0; with 9999 -> 500; with "x" -> 0.
T-C3  the clear / New path: 37 -> 0.
T-R1  RecorderClock: two ticks with beatNudgeAppliedMs 0 then 5 at the same bpm -> an anchor "nudge" is appended on the
      second; equal values -> none. Mutant M18 (the anchor removed) RED.
T-B1  a key press on a BeatNudge binding with step +1 -> beatNudgeDeltaMs +1; step -1 -> -1; a release -> 0.
T-B2  a MIDI note-on -> the step; note-off -> 0.
T-B3  a MIDI CC, Absolute and Relative, any value -> 0; `bindingIsLive` false for both. Mutant M16 (bindingIsLive
      always true) RED.
T-B4  `learnMidiCC` with a BeatNudge candidate: returns false, the binding list is byte-equal before / after, an older
      binding on that CC is still there. Mutant M16 RED.
T-B5  toVar / fromVar: "targetNudgeStepMs" round-trips -1 and +1; absent -> +1; the action's saved number is the last
      one and every older action keeps its number (the enum order pinned). Mutant M17 (BeatNudge inserted before
      TriggerRoutine) RED.
T-B6  `learnMidiCC` with any non-nudge candidate behaves as the overlay's old inline loop did (the old binding on that CC
      removed, the new one added).

GATE ROWS (Harmony; strings exact).
G-N1 UNIT. "100% tests passed, 0 tests failed out of <N>" with N = the lane's `ctest -N` count, and N >= main's count +
     the new cases; each mutant M1-M18 built in build-mut-<n> prints at least one FAILED case of the test named for it.
     RED arm: the mutants.
L1   ZERO. Real app (not test mode), a 120 BPM click file as the source, Manual 120, Resync. `GET /api/beat_nudge` =
     "ms": 0, "appliedMs": 0.0; 500 polls of /api/features: beatNudgeAppliedMs == 0.0 in all; probe-beatclock.sh's
     rows print the same PASS lines as on main 185147b. RED arm: L10's mutant app (FAIL on "appliedMs").
L2   THE BEAT MOVES BY D. Same launch. Arms in the order 0, +250, 0, -250, 0, +500, 0, -500, 0, +1, 0, -1, 0; each arm 8 s
     after features.beatNudgeAppliedMs equals the arm's ms; /api/features polled every 5 ms; for each poll pos = totalBeatCount +
     beatPhase and t = the poll's send / receive midpoint. For each non-zero arm: a straight line is fitted to the two
     neighbouring 0 arms together; the arm's median residual r (beats) gives measured = -r x 60000 / bpm.
     BAR: |measured - D| <= 3.0 ms for the four large arms; for the +1 and -1 arms |measured - D| <= 1.5 ms AND
     measured(+1) - measured(-1) between 1.0 and 3.0 ms. VALID when each arm has >= 1000 polls and the two bracketing 0
     arms' own median residuals differ by <= 1.0 ms (else "INVALID: drift" -- blocks and goes to the architect, not
     re-run until green). Print: "PASS  L2 arm <D>: measured <m> ms" per arm.
     RED arms: SELFTEST on synthetic polls (a shift of 0 reported for arm +250 -> FAIL); L10.
L3   QUANTISED vs BY HAND. Nudge +250 at Manual 120. (a) Quantize "Next Beat": 30 fires by POST /api/trigger_clip at
     pseudo-random times (the probe's fixed seed); for each, the first /api/features poll (5 ms) in which the layer's
     playing clip is the new one. BAR: 30 of 30 have totalBeatCount == the count at the request + 1 and beatPhase <
     0.15. (b) Quantize "Off": 30 fires; BAR: 30 of 30 show the new clip within 150 ms of the request, and at least
     20 of 30 at beatPhase >= 0.15. The probe reads the playing clip from the route probe-boxes.sh already uses for it
     (the builder names it; if none answers on the message thread, /api/debug/ui_text). RED arm: SELFTEST with (a)'s data
     fed to (b)'s bar and the reverse -> FAIL both.
L4   THE BAR TURNS WITH THE BEAT. Nudge +500 at Manual 120 after a Resync (set AFTER the Resync, since Resync zeroes), 70
     s of polls: beatInBar changes only in a poll where totalBeatCount changed; totalBarCount rises by one exactly when
     beatInBar goes 3 -> 0; totalBeatCount and totalBarCount never decrease; ui_text's "Bar N" follows barCount mod 4
     + 1. BAR: 0 violations in >= 130 beats. RED arm: SELFTEST with one synthetic backward count -> FAIL.
L5   INJECT IS NOT SHIFTED. Test mode: POST /api/beat_nudge {"ms": 100} -> "analysisRunning": false; inject beatPhase
     0.25, beatInBar 2, totalBeatCount 7 -> /api/features reads exactly 0.25, 2, 7. RED arm: SELFTEST expecting 0.05.
L6   SAVED WITH THE SHOW. Set 37; POST /api/debug/save_composition to a temp path; set 0; load that file; wait for the
     load on ui_text; GET = 37 and appliedMs = 37.0 within 300 ms; New -> 0; load a fixture without the key -> 0; no
     new text in ui_text's notice keys across the three. RED arm: mutant M15's app or SELFTEST.
L7   KEY / PAD ACTION. POST /api/debug/binding_action {"action": "beatNudge", "step": +1} x 3, {"step": -1} x 5, a
     release (value 0), a CC-typed binding x 1: GET ms after each = 1, 2, 3, 2, 1, 0, -1, -2, -2, -2.
     "PASS  L7 the handler case moves the nudge: 10 of 10 steps". RED arm: a mutant app with the step's sign ignored.
L8   LEARN REFUSES A KNOB. midi_learn: open; select "Beat later"; title == "Send a MIDI note..."; send CC 20 -> still
     waiting, binding count unchanged; send note 60 -> attached, tag shows the note; a CC on a non-nudge target still
     attaches. "PASS  L8 MIDI learn refuses a CC on a nudge target and changes nothing else; a note attaches: 8 of 8
     states". GET /api/debug/bind_overlay: the two targets present with label_w <= w - 4; the count of targets failing
     that bar == 15 and none of them is a nudge target. RED arm: mutant M16's app.
L9   RESYNC ZEROES, TAP AND TEMPO LEAVE. Set +40. binding_action "tapTempo" twice 500 ms apart -> ms 40, bpm 120 +/- 2.
     POST /api/set_bpm 126 -> ms 40. POST /api/resync -> ms 0 and appliedMs 0.0 within 200 ms, and features show
     beatInBar 0 with beatPhase < 0.2 in the first poll after trackerRequestSeq reaches the posted value.
     RED arm: mutant M14's sibling (the resync line removed) -> "FAIL  L9 resync: ms 40, want 0".
L10  MUTANT APP. build-mut-sign (M1): L1 passes its "ms" check, L2 prints FAIL on all six arms with the measured value
     near -D. (RIG-RULES: after the mutant, the restore rebuild compiles >= 1 object and the binary sha differs.)
VG   Five critic seats on V1-V18: 0 MUST. Pre-registered MUSTs: any nudge text not exactly as T-U1; the label clipped at
     any state; any top-bar control overlapping another at 1280 with Manual on that did not overlap in V18; a non-ASCII
     glyph in the group or the learn title; a missing tooltip.
After every live batch: no Audio-DNA pid of the probe's left, 0 windows, the Output window never opened.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

B1  In the room, music playing, Manual or Auto locked, Resync on the "1". Stand where you watch from. Press "+" a few times,
    then "-" -> the flashes and cuts slide later, then earlier, against the music; the BPM number does not move; the
    text counts 1 ms per press. WRONG: the tempo number changes; the picture does not move; only some things move (a
    quantised clip still fires on the old beat while the effects pulse on the new one).
B2  Hold "+" -> the number runs up steadily and stops at +500. WRONG: it jumps, or the picture stutters while it runs.
B3  Type 120 in the text, Enter -> the number shows +120 at once and the beat slides there over about half a second with
    no jump. WRONG: a visible freeze or a double flash.
B4  Fire a clip with Quantize Off, turn a knob -> both act the instant you do it. Set Quantize to Next Beat and fire ->
    the clip starts on the nudged beat. WRONG: the by-hand fire waits; the quantised fire lands on the un-nudged beat.
B5  Loud hits (a snare, a drop) -> what follows loudness or hits still lands on the sound, not on the nudged beat.
    WRONG: hit-driven flashes drift with the nudge.
B6  Tap the tempo -> the number stays. Press Resync -> it reads 0 (question 49's default). WRONG: Tap changes it.
B7  Save the show at +30, open another show, open the first again -> +30 is back. WRONG: 0, or a message appears.
B8  Put "Beat earlier" / "Beat later" on two keys; hold one -> it keeps moving. Put them on two pads; one hit = 1 ms.
    Try to learn a knob on them -> it does not take. WRONG: a held key moves once; a knob attaches.
B9  At a glance from arm's length: is "off beat by +12 ms" readable where it sits, and are "-" / "+" where your hand
    expects them? Is "+" = later the way round you think of it (question 61)?
B10 The beat circle: with +250 at 120 BPM its lit quarter turns half a beat after the music's beat. A few ms cannot be
    seen on it; the number is what you read.
B11 Honest limit to judge by eye: things that START on a beat (a quantised clip, an autopilot step) land within about
    one hundredth of a second of the nudged beat, as they do today around the un-nudged one; smooth beat-driven
    movement follows the nudge to the millisecond.

## 7 QUESTIONS FOR BORIS (new numbers; each has a default A; nothing waits)

61. Which way is plus? "off beat by +12 ms" means:
    A (default) The app's beat comes 12 ms LATER. "+" moves it later, "-" earlier.
    B The other way round: "+" moves the beat earlier.
62. The five buttons "/4 /2 x1 x2 x4" next to the tempo do nothing today. The nudge needs their space.
    A (default) Take them out. (They can come back when they do something.)
    B Keep them and make them halve and double the tempo; the nudge goes further right, after Quantize.
63. A pad held down on "Beat earlier" / "Beat later":
    A (default) One hit = 1 ms. Holding does nothing more. (A held KEY keeps moving, as your keyboard repeats.)
    B A held pad keeps moving too.
Still open and touching this lane: 49 (after Resync the number reads 0 by default -- the one line named in NB4); 50 (where
key and pad settings are kept -- nothing here changes with it).

For Harmony (each with a default; not Boris's):
H-a  A replayed Resync leaves the nudge (NB4). Default as ruled.
H-b  Right-click on the text = 0 (NB3). Default: built; drop it if the visual gate's seats call it hidden behaviour.
H-c  OSC for the nudge is NOT built (nobody asked). Default: not built.
H-d  The wheel's 15 Hz stays (NB3). Default: stays; a faster own-layer wheel is filed as an idea, not built.

## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)

R1  STRONGEST: "A delayed copy would be more faithful for LATER." For D > 0 the plan computes the shifted beat from the
    tracker's state NOW (subtract D ms of beats at the current tempo). A true delay -- a short ring of the tracker's
    nine fields, published D ms late -- would replay every Resync, relock and phrase reset exactly D ms late and never
    need a hold. It loses because (1) it cannot do EARLIER at all, so two mechanisms with two test families would sit
    behind one number and meet at 0 with different behaviour on each side; (2) a Resync or Tap would take D ms to show,
    where the rule here puts the "1" D ms after the press with the grid already right; (3) the differences are confined
    to the D ms after a deliberate move or a tempo step, where the plan's hold and carry are bounded and tested (T-N5,
    T-N7, T-N8). Cheapest refuting test: T-N8 and T-N7 as written -- if a hold longer than the stated bound, a backward
    counter or a wrong bar after Resync shows up, the ring is the fallback for D > 0 and NB1 goes back to the architect.
R2  Auto with real onsets: beatInBar is onset-driven (F4). The shift carries whatever the tracker publishes, including
    its quarter-bar barPhase step when an onset is late. The nudge adds no new glitch but does not remove that one;
    PresetSelector's wrap test (F8) can read it as a bar crossing today. Cheapest test: T-N5 counts bar edges with and
    without a nudge on the same late-onset sequence; equal counts = not made worse. Filed as a side finding on main, not
    fixed here.
R3  Edge readers see the hop grid (NB1): a single quantised fire lands D +/- 10.7 ms. Boris may expect every fire to be
    1 ms true. Stated in B11. Cheapest test: L3's phase column. A sub-hop edge (readers interpolating to the frame) is
    another lane.
R4  The live +1 / -1 arm (L2) may be limited by the instrument (HTTP polling, clock drift) rather than the product. The
    row has an INVALID verdict for that and the unit rows T-N2 / T-N3 are the 1 ms proof; the live arm is not allowed to
    be loosened into a pass. Cheapest test: the probe's self-test with a synthetic 1 ms shift and 2 ms of poll jitter.
R5  The top bar is already too wide (F11, INFERRED). If S0 shows it is not, nothing changes; if it is, the nudge makes
    it 22 px worse under 62 A and 162 px worse under 62 B, and the right-hand items squeeze further. The lane does not
    re-lay the whole bar. Cheapest test: S0's capture.
R6  A Resync from an EARLIER nudge holds the shown beat for up to the old |D| ms before the new "1" runs (rule 4): at
    -500 ms that is half a second of frozen beat phase after pressing Resync. Typical nudges are tens of ms. Cheapest
    test: T-N8's -250 arm prints the hold length; B6 is his check. The alternative (BeatLead's excess counters) keeps a
    permanent count offset and can fire a beat edge twice around a correction; rejected by the Harmony constraint.
R7  `keyPressed` receiving OS key repeats is ASSUMED (F14). If JUCE filters them, a held key moves once: B8 catches it;
    the fix is a repeat on the 30 Hz UI tick while the key is down (the same code question 63 B would need).
R8  Hiding the five buttons before Boris answers 62. It is the default of an asked question and reversible by one
    constant; if he answers B the group moves and the buttons return unchanged.
R9  Float precision: beatPhase is a float; the shift is done in double and rounded once. At 1 ms and 60 BPM the step is
    0.001 beat, four orders above float resolution near 1.0. T-N2's 2e-5 bar is the test.
R10 Two lanes in TopBar.cpp / MainComponent.cpp / ApiServer.cpp (section 4): a merge conflict is a builder's step 0.

NOT VERIFIED (could not be established by reading; each is used above only where labelled):
- That the top bar overflows at 1280 (arithmetic only; S0).
- That JUCE delivers OS key auto-repeat to `keyPressed` (R7).
- That the learn title's em dash paints wrongly today (F16).
- That one hook after `swapCompositionModel` covers both Open and New (NB5); the kCompNew path was not read end to end.
- That no analysis stage reads beat fields through a path other than `snap->` (F2).
- The route a probe reads "the layer's playing clip" from (L3), and the existing RecorderClock test file's name.
- The 15 old overlay labels that fail the label bar: the count is H-16's; the labels themselves were not read.
- Whether Boris's build has Link compiled in (FB section 4 item 1); the plan does not depend on it.
- The width Boris runs the window at (V11).
- The padding bytes of a published snapshot being zero today (rule 5; T-N10 decides).

## 9 WHAT IS NOT IN THIS LANE

- The per-output Delay, the Outputs button, the output windows, Syphon, settings.json: the output-settings lane.
- Wiring or deleting the five multiplier buttons (question 62): hidden only.
- A faster or own-layer beat wheel; a beat flash on the output; sub-hop edge timing for quantised fires.
- OSC for the nudge; a knob / fader / CC for it; a room list; any per-machine memory of it.
- Following Link's beat phase. Fixing the tracker's late-onset barPhase step or PresetSelector's wrap test (R2).
- The two copies of the tap code, taps 9+ re-using the same average, Tap not moving beat-in-bar, the Manual toggle not
  following a REST set_bpm (FB section 3 items 2, 4, 5): unchanged, listed for the docs stage to describe truthfully.
- The quit window, a dirty flag, where bindings are kept (question 50).
- Fixing the 15 old overlay labels that are wider than their buttons.
- Stale "Triple-buffer" wording in architecture.md / CLAUDE.md and APP-INVENTORY's 1249 against 1252 beyond re-counting
  this lane's own number (H-17 d: open on main).

From the stopped branches -- CARRIED (re-typed onto main; nothing is merged or cherry-picked whole):
- 5a46c07 / 4ab00bd `BeatLead`: the nine-field `Beat`, `beatOf`, `writeBeat`, `kBeatFields` + static_asserts,
  `nonBeatCrc`, the barPhase / phrasePhase recompute, the hold idea, the identity rule; `BPMTracker::predictedBeatRegime()`.
- bc1d89f `SyncSlew`: the 0.25 ms per ms glide and "the first value snaps", as rule 1 (not the class).
- e15d1d0 the nudge action, its step field, the delta function, the two overlay targets, toVar / fromVar (renamed).
- df98f78 `bindingIsLive` (simplified), `learnMidiCC`, `titleText`, `selectAt`, `bindingTag`, `stateForTests`, the routes
  `POST /api/debug/binding_action`, `POST` + `GET /api/debug/midi_learn`; test_binding_sync_nudge's cases (renamed file).
- dc59573 testing-eyes.md line 72 (kill advice -> own pid), one line, in S5.
- The row shapes R13 / R14 of probe-sync (H-15) as L7 / L8; `http()` and the launch / lock / quit handling are taken from
  probe-manual-bpm.sh on main plus probe-quit-ours.sh, not from probe-sync.py.
DROPPED with the branches:
- ccae1dc, ea90149 and the rest of the LATE half: `AnalysisDelayLine`, the serviceOnce / processHop split, `SyncOffset.h`,
  `syncOffsetMs`, `SyncWitness`, `RenderPulseWitness`, the [timing] case 6bd1253.
- `BeatLead`'s `frameEpoch` / `appliedResyncs` views and the seven increment sites, the excess counters, re-base, `Diag`.
- e0010e1 `SyncVenues`, `SyncOffsetController`, `AppSettings::kSync`, `/api/sync*`, OSC `/audiodna/sync*`, the venue probe.
- 9bd1efe, ef0d8e6 RecorderHost's show-time stamps (they compensated a delayed SOUND; nothing is delayed here).
- The Relative-CC arm of the nudge action; "Sync -1 ms" / "Sync +1 ms" as labels.
- probe-sync.py / .sh / selftest, the D0 / RD rows 2d1c4ef..740b6d6 (H-14 stays a finding about main's take recorder),
  b56ed5f's ADNA_OPEN_ENV passthrough, the lane's Pitfall 68 text (the number is re-used for the nudge), f14eb31's dial
  docs, f4a7e34's merged test count, 28ef4a4's .gitignore line (the lane adds `/build-mut-*/` itself if it is not on main).
- Not carried and not dropped here (they belong to the replacement plans that own them): the music-beat wheel of S5a
  (his answer 46 now says the circle moves with the nudge, which this lane delivers by leaving the wheel on the bus) and
  the Gain 140 px of S5b (the Gain slider is not in this lane's part of the top bar).

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 14:07:07, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-nudge.md (status DONE; 31 attacks ruled: 21 ACCEPT, 9 PARTIAL, 1 REJECT;
24 amendments, each OVERRIDES this plan's body). Precedence for every stage, review and gate of lane "nudge": Boris's verbatim
words (binding-decisions.md, the 2026-10-04 sections) > this adoption > ruling-nudge.md > this plan's body. Workflow run
wf_503e3d12-834 (plan: architect opus high; seats beat-clock 7 attacks / 2 MUST, gates 8 / 4, stage-hands 8 / 0, scope 8 / 1 --
papers whole (39,421 characters) in attack-nudge-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's returned verdict, stage list, decisions, measurements, strongest
counter-argument, and its section 7 (questions) in full. NOT read by me: sections 1-6 and 8-10 in the file -- the builders'
and reviewers' spec; a gate string is copied only from section 5.
THE RULING'S OWN CAVEAT, kept in view: "nothing was built or run, so the amended rule's behaviour rests on a python paper
model of my reading, not on the app". Stage S1's unit cases on a REAL BPMTracker and my rows G-N1, L2-L4 are where it meets
the app; a disagreement there is a STOP to the architect, never a builder's re-design.
WHAT THE COUNCIL FOUND THAT THE PLAN DID NOT: in Auto the tracker moves its count and its beat-in-bar on different hops, so the
plan's carry rule made the shifted bar position dip and the downbeat fire twice (upheld; the bar fields are now seated at the
shifted beat's own edges); a hand Resync under the plan's hold froze 1-2 beats; and the two gates meant to prove "one shifted
beat" and "bit-identical at 0" could not fail (rebuilt: a golden recorded on 185147b; live rows against the tracker's own
line with a bypass mutant app).
HARMONY'S DECISIONS (the ruling's section 8):
H-1  default: the setter is a TEST-SERVER route; production gets no write route for now.
H-2  default: a held PAD is one hit = 1 ms, no repeat (a pad sends one note).
H-3  default: a replayed Resync resyncs the beat and leaves the nudge.
H-4  default: no OSC for the nudge.
H-5  default: the beat wheel stays at 15 Hz (the number is the instrument); told to Boris.
H-6  default: a held KEY rides the Mac's own key repeat; the app-side repeat is the pre-ruled fallback if his check B8 fails.
H-7  default: side finding SF-1 (at nudge 0 in Auto a bar-quantised fire can land a beat late) is FILED AGAINST MAIN, not
     fixed here; FM-2 measures it. It goes to the handoff's ledger.
H-8  default: tooltips are judged as text.
H-9  default: the Resync restart may set the tracker's beat and bar counters (adoptCounters); if the real-time review
     refuses a tracker write, the pre-ruled alternative is built.
H-10 default: yes -- the transport lane's bar reader reads the bus; snapshot offsets 332 / 336 and the next Binding action
     slot are this lane's; the second lane to merge takes main in as its step 0. RECONCILE with the transport adoption's
     H-T1: stage S4t there ("a Tap keeps the bar", one function in BPMTracker.cpp, case TL-U70) and this lane's two tracker
     views + adoptCounters touch the same file: they are built in ONE worktree order -- this lane's S1 first, S4t on top --
     or the architect rules otherwise in the transport answers delta. No two builders in BPMTracker.cpp at once.
H-11 default: the manual lives in docs/manual/.
H-12 CHANGED: both this ruling and ruling-outputs.md claim Pitfall 68. Lanes write "Pitfall NN"; I assign the number at each
     merge (the first of the two to merge gets 68).
ALSO CHANGED (the outputs adoption's HD-15): this lane RE-TYPES the key and pad work from lane/bf2-keys (stage S4), so the
bf2 and bf2keys worktrees stay, read-only, until this lane's S4 is gated; then they are removed.
ORDER: G-N0 (mine) -> S1 -> S2 (then my G-N1, L2-L6) -> S3a -> VG-0 -> S3b -> S4 (then my G-N1, L7, L8, L11) -> VG (five critic
seats) -> S5 -> merge. NOT STARTED in this session: nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWERS (2026-10-04 14:21:32)
Boris, verbatim (binding-decisions.md, "Boris's answers to questions 61-63"): "61 b but lets call it "nudge X ms""; "do a
similar to resolume: beatWheel play pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2 tap resync"; "good (this is just
nudge amount)". His words outrank the ruling. Consequences, binding for every packet of this lane:
(1) 61 B: PLUS MOVES THE BEAT EARLIER (the ruling's section 7: "61 -> the constant of A8"). The on-screen text is
    "nudge X ms", not "off beat by ...": every gate string, ui_text key and visual state that carries the old text changes.
(2) 62: neither A nor B as asked. The tempo row of the top bar is HIS list, in his order. /2 and *2 are kept and wired
    (halve / double the tempo); /4, x1, x4 go; play, pause, stop, BPM minus and BPM plus are NEW controls. This is larger
    than stage S3b as ruled (two buttons + an editable text) and re-opens VG-0 / VG.
(3) 63 A: the ruling's A7 stands (a show's own amount takes over when it is opened).
The engine stages (G-N0, S1, S2) and the key / pad stage S4 are NOT changed by (2); they may be built as ruled, with (1)'s
constant. OWED BEFORE the packets of S3a, VG-0, S3b and VG are written: an architect delta on answer 62 and the new text
(opus max), attacked by two blind seats (gates, stage-hands), with question 111's answer in hand (what play / pause / stop
run). No builder designs the row.

## POINTER (2026-10-04 15:35:36): the tempo row and the BPM timer are re-stated in plan-nudge-row.md / ruling-nudge-row.md (ADOPTED); they amend stages S3a..VG of this plan and add S1r, S2r, S3m, S3r, S4r.
