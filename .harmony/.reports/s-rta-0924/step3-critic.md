# s-rta-0924 — CRITIQUE of `.harmony/specs/s-rta-0923-step3-plan.md` (spec STEP 3: recorder in the running app)

Critic: Architect (Fable), 2026-09-24, read-only pass over `/Users/boriskarpman/projects/RealTimeAudio` @ HEAD
`f8f0888` (main). INTENDED HOME (dispatch-named, refused by the write fence — Harmony persists, A54):
`.harmony/.reports/s-rta-0924/step3-critic.md`. Working tree carries an UNRELATED concurrent builder's uncommitted
edits (`src/render/Renderer.cpp`, `src/connect/ScalarParams.h`, `tests/CMakeLists.txt`, new
`tests/test_comp_transform_units.cpp` — the comp-position fix); ignored per dispatch EXCEPT where they create a
serialization point (N11). Labels: VERIFIED = read on disk at the cited line this session; INFERRED = derived from
cited code; ASSUMED = stated so a builder can check it.
Rulings folded: **Lane 3's `src/connect/ManualWrite` is THE manual-write funnel (R8) — the plan's §3.5 `ManualWriter`
/ Lane S3-M is SUPERSEDED**; Ruling 28 `AudioStore` (take v3) landed at `33bdc67`; H1 landed `9257537`; L5 landed
`dd646de`; Lane 3 landed `9da23b7..7b1071f`.

---

## VERDICT: BUILDABLE-WITH-FIXES

The plan's spine holds: `RecorderHost` behind a `Dispatch` seam, the arm→tick→disarm lifecycle, provisional +
periodic `Take::save` through `AudioStore::referencing`, per-tick `popGap` drain, `isRunning()`-before-`stop()`,
the overdub asset-frame clock, REST `/api/perf/*` pulled forward, and the Origin-threaded handlers all match the
shipped `AudioStore`/`AudioTap`/`Player`/`PerformanceRecorder` APIs (§3.1 contract re-checked line by line below).
It is NOT buildable as written because (B1) a whole lane and the plan's central verdict describe a funnel that now
exists and must be HOOKED, not built; (B2) the hook shape it proposes (`onHumanSet(key, v)`) cannot open a gesture,
so every OSC/MIDI/REST continuous write would be silently dropped by `PerformanceRecorder::set`; (B3) the tick
insertion text is stale against the shipped engine tick; (B4) the live recipe triggers empty cells on the default
deck, which CLEARS the layer — it would fail on a correct build; (B5) the now-LIVE R13 (16 kHz device) is
unaddressed: `sample`/`t` clocks and the WAV are correct at any rate, the beat clock is not, and nothing in the plan
records, warns, or gates on it. Every fix is small and named exactly (§AMENDMENTS).

---

## BLOCKING

### B1 — §1, §3.5, §3.7 (Lane S3-M), §7 items 2/3/11: `ManualWriter` is SUPERSEDED; half B is NOT blocked
- Plan claim: `manualWrite` exists nowhere (T1); continuous capture waits for a half-day `src/connect/ManualWriter`
  slice (S3-M M1+M2) that instruments 11 writer sites.
- Disk (all VERIFIED): the funnel is shipped and every site is already through it.
  - Core: `src/connect/ManualWrite.h:49-71` — `resolveControl`, `gripActive`, `manualTouchCore`, `manualWriteCore`,
    `manualReleaseCore`; D8 rank `Hand{None,Lane,HumanDecaying,HumanHeld}` (`:24`). Bodies `src/connect/ManualWrite.cpp`.
  - MainComponent wrapper + **the hook seam the ruling names**: `src/MainComponent.h:103-117` —
    `using GripKind = ParamConnection::Grip::Kind;` `manualWrite(path, valueNorm, GripKind, Origin) -> bool`,
    `manualRelease(path, Origin)`, `manualTouch(path, GripKind, Origin) -> bool`, and
    `onManualWrite(path, valueNorm, GripKind, Origin, bool accepted)`, `onManualTouch(path, GripKind, Origin, bool
    accepted)`, `onManualRelease(path, Origin)`. Bodies `src/MainComponent.cpp:3189-3211`; `handFor(Origin, Kind)`
    `:159-163` maps Human→HumanHeld/HumanDecaying, **every other Origin (Replay/Routine/Engine/Preamble)→Hand::Lane**.
  - The 11 sites: OSC master `:1963`, OSC layer opacity `:1967`, OSC macro `:1998`; binding velocity→clip opacity
    `:5887` and `:5923`, MasterOpacity `:6050`, AdjustLayerOpacity `:6056`, AdjustMacro `:6108`; REST set_layer_opacity
    via `apiServer_->onSetLayerOpacity` `:1927-1930` and REST set_param clip branch via `onSetClipEffectParam`
    `:1935-1940` (`src/api/ApiServer.cpp:451-456, :541-544` no longer write the model — T7 stale); LayerStrip
    `src/ui/LayerStrip.cpp:422-436` writes `layer_->opacity` directly + `gripHeld()/release()` on drag — a WIDGET grip,
    deliberately NOT through the funnel (lane-3 plan §4.2, D14) → not recorded; disclose, do not "fix".
  - Path builders the hook can reuse: `compScalarPath/layerScalarPath/clipScalarPath/clipParamPath/macroPath`
    `src/MainComponent.cpp:71-157` (positional + names filled — exactly the D2-rebindable keys the plan wants).
- Required fix: delete §3.5 and the S3-M row; §1's decision becomes "half B proceeds NOW in Lane S3-B by hooking
  the three seams". Exact wiring (Lane S3-B, ctor, next to the `Dispatch` lambdas) — see AMENDMENT A1. Replay side:
  `touch → manualTouch(k, GripKind::Held, Origin::Replay)`, `set → manualWrite(k, v, GripKind::Held, Origin::Replay)`,
  `release → manualRelease(k, Origin::Replay)` (s167 spec `:346-348`: "on replay … begin → gripHeld()"; a Lane-rank
  Held grip still yields to any human hand — `src/connect/ParamConnection.h:178-185`, VERIFIED). Capture side: the
  three hooks forward `origin == Human && accepted` only. R5 (overdub feedback) is closed twice: `handFor` gives
  Replay writes `Hand::Lane` and the hook filters on Origin. The `continuousUnavailable` counter stays only as the
  headless FakeDispatch guard; in the app the seam is filled from day one. §1's "snap, not glide, until L3(b)" and
  §4's "(snap, no glide until L3(b))" are stale — L3(b) is live (`src/MainComponent.cpp:3246-3253`), the glide
  runs from `Composition::handBackGlideMs` (`src/model/Composition.h:83`).

### B2 — §3.1 `onHumanSet(key, v)`: cannot open a gesture → every Decaying continuous write is silently dropped
- Evidence: `PerformanceRecorder::set` ignores a `set()` with no open gesture — `src/recording/PerformanceRecorder.cpp:
  75-79` ("stray set() without a preceding touch() -- ignored"), VERIFIED. `MainComponent::manualWrite` does
  touch+write in ONE call and fires ONLY `onManualWrite` (`:3189-3196`); it never calls `manualTouch`, so
  `onManualTouch` never fires for any of the 11 sites (they are all `manualWrite(..., Decaying, Human)`). With the
  plan's hook (`onWrite → onHumanSet`), the recorder's `set()` finds no open gesture → nothing is recorded for OSC,
  MIDI CC, velocity, or REST opacity/param writes — the primary continuous capture path records zero gestures,
  silently. The plan's own tests (§3.6 #10/#14) would not catch it (they drive `onHumanTouch` first).
- Required fix: replace `onHumanSet(key, v)` with `onHumanWrite(const ControlPath& key, float v, const std::string&
  grip)` whose body is `if (!recorder_.hasOpenGesture(key)) recorder_.touch(key, grip); recorder_.set(key, v);`.
  `PerformanceRecorder` has no open-gesture query → Lane S3-A adds a THIRD one-liner to
  `src/recording/PerformanceRecorder.h`: `bool hasOpenGesture(const ControlPath& k) const { return
  openGestures_.count(k) != 0; }` (the plan already adds `setCheckpoint0`/`current()` there). Test #14 becomes:
  three `onHumanWrite(k, v, "decaying")` 20 ms apart with NO prior touch → exactly one gesture, begin exact, ≥1 mid
  point, exact synthesized end after the silence window. Add test #15 `[host][capture] onHumanWrite with no prior
  touch opens the gesture` (fail-first against the plan's shape).

### B3 — §3.3 B1 tick hunk: placement text and the "L3(b) unlanded" premise are stale (low severity, must amend)
- Plan: insert the recorder tick "immediately after `read()` and BEFORE `signalRegistry_.evaluateAll(snap)`",
  with "L3(b), when it lands — insert it AFTER this block"; `wallNow = juce::Time::getMillisecondCounterHiRes()/1000`.
- Disk: L3(b) HAS landed — `connectionEngine_.tick(composition_, ctx)` at `src/MainComponent.cpp:3253`, preceded by
  the placement contract at `:3237-3244` ("the recorder's Player::advanceTo (step 3) MUST be inserted BETWEEN
  updateValues and this call") and `src/connect/ConnectionEngine.cpp:266-273` (advanceTo BEFORE tick()). The ONE
  engine clock is `connNow()` (`src/connect/ConnClock.h:9`), already sampled once per tick at `:3246` and handed to
  the engine as `ctx.now` — the plan would sample a second, unequal timestamp for grips vs recorder stamps.
- Severity, honestly: the plan's placement does not violate the ORDERING FACT (it is still before `tick()`), and
  `MacroBank::updateValues` writes only `currentValue`, never `manualValue` (`src/routing/MacroBank.h:74-84`,
  VERIFIED) — no wrong behaviour results. But the instruction is false about the code's state, contradicts a
  documented on-disk contract, and a literal builder leaves a stale "L3(b) goes here" slot plus a duplicate clock.
- Required fix (AMENDMENT A3): the block goes between `:3232` (`globalMacroBank_.updateValues(signalRegistry_);`)
  and `:3246`; hoist `const double now = connNow();` above it and pass `now` as `wallNow`; delete the "L3(b) after
  this block" sentence; T3 → "ConnectionEngine::tick IS called at `:3253`".

### B4 — §4 live recipe: the default deck has NO clips; `trigger_clip` on an empty cell CLEARS the layer
- Evidence: `Deck::initDefault` creates layers with empty columns (`src/model/Deck.h:27-39`); `Layer::triggerClip`
  on an empty cell → `clearActiveClip()` and returns (`src/model/Layer.h:215-221`), VERIFIED. The recipe never loads
  content, then expects `activeClipColumn` to become 1, 2, 3 and ≥4 `activeClip` points — on a correct build the
  column stays −1 and the replay row FAILS (false negative; the capture itself would still record v=1,2,3 press
  points, so the take looks fine while the model check fails). Lane 3's gate loads a composition FIRST:
  `POST /api/load_composition {"path": ".../.harmony/probe-lane3-layer.json"}` (`.harmony/probe-lane3.sh:123-127`;
  handler key is `path`, `src/api/ApiServer.cpp:788`), whose only clip is layer 0 column 0 (`gravity_well`).
- Required fix (AMENDMENT A6): new fixture `.harmony/probe-step3.json` = the lane-3 layer fixture with source clips
  in layer 0 columns 0..3 (no connections, `quantizeMode` Off — a queued trigger on a `NextBeat/NextDownbeat`
  composition fires later on the GL thread and the press-time point would not match the model at poll time);
  load it BEFORE `perf/record` (so `checkpoint0` and the take's layer/clip names come from the loaded composition
  and `Program::compile` resolves on replay); expectations become columns 0,1,2,3.

### B5 — R13 is LIVE (HANDOFF `:2633`): the plan neither states nor gates what a non-48 kHz device does to a take
- Evidence: `AnalysisThread::kSampleRate = 48000` (`src/analysis/AnalysisThread.h:47`) feeds every analyzer
  (`AnalysisThread.cpp:25-40`); the only warning fires ONCE in the ctor (`src/MainComponent.cpp:547-560`, plus a
  production modal). Step 3's clocks: `t` = wall; `sample` = `deliveredSamples` in DEVICE frames
  (`src/audio/CombinedCallback.h:105,134`); the tap records at the device rate (`prepare(rate,…)` `:158-159`);
  `CaptureFacts.rate` = device rate at arm → WAV header, `segment.rate`, and `sample` stamps are self-consistent at
  ANY rate, and replay-with-audio is rate-agnostic (`assetFrame = transportFrames * asset.rate / deviceRate`;
  `AudioTransportSource::getNextReadPosition()` returns device-rate frames — `build-asan/_deps/juce-src/modules/
  juce_audio_devices/sources/juce_AudioTransportSource.cpp` `getNextReadPosition`, VERIFIED). BUT `beat`/`bpm` on
  every point, the `TempoMap`, `durationBeats`, and `DriveClock::Beat` (routines) come from the 48 kHz-assuming
  analysis → at 16 kHz they are ~3× wrong; the take does not record the device rate when audio is OFF (ruling 19)
  and records it only inside `audio.segments[0].rate` when ON. A mid-take rate change self-stops the tap (R-A5
  covers the audio) but `deliveredSamples` keeps counting in the NEW rate → `sample` stamps after the change are
  mixed-domain — the plan is silent.
- Required fix (AMENDMENT A5, ~15 lines, no format change): (a) `ArmOptions.analysisRate = AnalysisThread::
  kSampleRate`; at arm, if `deviceRate != analysisRate` → `notify("device rate N Hz ≠ analysis rate 48000: beat
  clock unreliable (R13)")`, `Status.rateMismatch = true`, and ONE marker `take.markers += {action:"rateMismatch",
  v = int(deviceRate), origin Engine}` (markers exist, `src/recording/Take.h:91`; `DiscretePoint.v` is int,
  `Lane.h:85`); (b) `tick()` publishes `deviceRate` into `Status` (it already receives it); (c) on a mid-take rate
  change (tap self-stop detected per R-A5) `notify` + `Status.lastError`, and disclose that `sample` after
  `unreliableFrom` is untrusted; (d) recipe precondition becomes machine-checked: `perf/status.deviceRate == 48000`
  before arming AND `grep -c "analysis pipeline assumes" /tmp/adna-err.log == 0`; (e) §5 only-Boris item 1 is
  corrected from "8.8 % off at 44.1 kHz" to "and 3× off on the 16 kHz HFP headset already seen in this rig".

---

## NON-BLOCKING (amend, but no builder is blocked by them)

- **N1 — §3.1 `enum class GripKind` in `RecorderHost.h` is a redundant third representation.** The recorder side
  already speaks strings: `Sink::touch(const ControlPath&, const std::string& grip)` (`src/recording/Player.h:20`),
  `PerformanceRecorder::touch(key, std::string grip)` (`.h:46`), `Gesture::grip` "held"|"decaying" (`Lane.h:133`);
  MainComponent's `GripKind` is `ParamConnection::Grip::Kind` (`MainComponent.h:103`, three values incl. None). A
  global `::GripKind` compiles (class scope hides it) but shadows confusingly. Drop the enum; `Dispatch::Continuous::
  touch(const ControlPath&, const std::string& grip)`; `onHumanTouch/onHumanWrite` take the string.
- **N2 — Engine-origin points.** (a) The genre auto-switch calls `handleDeckSwitch(deckIdx)` (`MainComponent.cpp:
  783`) — with the defaulted `Origin::Human` it is recorded as a HUMAN deck switch; pass `Origin::Engine` there (D6
  records Engine, tagged). (b) Autopilot advances happen on the GL thread and the message-thread hop
  `onAutopilotAdvanced_` carries NO payload (`Renderer.cpp:253-258`, `MainComponent.cpp:760-762`, `Renderer.h:96`) —
  the s167 threading table (G13) expects Engine `activeClip` points from that hop. Step 3 as planned captures none;
  replay relies on `checkpoint0.autopilotEnabled` per layer re-running autopilot live. Disclose as R-13 in §6 (either
  extend the hop to `(layer, fromCol, toCol)` in a later step, or accept). Not silent: name it.
- **N3 — `/api/perf/status` must read ONLY `recorderHost_.status()`.** `onPerfStatus` runs on the HTTP thread;
  `audioEngine_.getCurrentSampleRate()` walks `deviceManager_.getCurrentAudioDevice()` (`AudioEngine.cpp:89-93`)
  while the message thread may be restarting the device (`setAudioDeviceSetup`) — read it in `tick()` and publish
  through `Status` (B5 b). Same rule for any other field.
- **N4 — T11/§3.3 `applyAudioTransport` site count: SIX file-load auto-plays, not four** — `MainComponent.cpp:287,
  :294` (ctor selector), `:623, :630` (TopBar selector), `:3169` (file drop), `:3591` (deck load) — plus
  `GlobalPlayPause :6033-6041` (play/**stop** toggle) and `GlobalStop :6043-6046`. Reviewer grep (ii) catches it;
  the plan should count it right.
- **N5 — `activeClip` points are PRESS-time, not FIRE-time.** `handleClipTrigger` captures after
  `layer->triggerClip(column, forcedSnap)` (`:3979-3985`), which under `quantizeMode != Off` only QUEUES
  (`Layer.h:227-234`); the real change lands later on the GL thread. Replay fidelity therefore requires the same
  `quantizeMode`; `play()` should apply `checkpoint0.quantizeMode` through the `quantize` dispatcher the plan already
  accepts (§3.3 last bullet) before the first `advanceTo`. State this in the vocabulary comment + §6.
- **N6 — §4 continuous row expectation is wrong under TOUCH semantics.** A refused `set` marks the lane
  `displaced` for the REST OF THAT GESTURE (`src/recording/Player.cpp:120-131`), and a displaced gesture is not
  released by the Player (`:113-115`); the lane resumes at the next gesture, not "after gripHoldMs". Rewrite:
  "a `set_layer_opacity` sent DURING replay wins for the remainder of the current gesture; the next recorded gesture
  re-touches; the engine glides the connected value back over `handBackGlideMs`".
- **N7 — Synthesized Decaying end.** s167 `:348-350` ties it to `gripHoldMs`; pass `comp.gripHoldMs` in
  `ArmOptions` (or `tick()`) instead of a hard `kDecayingGestureEndMs = 250`, so the recorded end matches the
  engine's expiry (`ConnectionEngine::Context::gripHoldMs`, `ConnectionEngine.h:39`).
- **N8 — Device lifecycle (dispatch asked).** Step 3 touches it in exactly ONE place: B5's `setSourceMode(File)`
  at arm-with-`audioFile` and at play-with-audio → `deviceManager_.setAudioDeviceSetup(setup, true)`
  (`AudioEngine.cpp:110, :122`) → JUCE restarts the device ONLY when the setup changes (`juce_AudioDeviceManager.cpp:
  735-737` early return when equal, VERIFIED) — so mic→file at arm restarts once; file→file at play does not. The
  BT crash lives in `AudioIODeviceCombiner::restartAsync` on a device publish/die event (crash report `:75-87`) —
  a restart path; so: (a) recipe precondition "no BT device" stands and now has an oracle (B5 d); (b) step 3 adds
  NO `setSourceMode(MicInput)` call anywhere (confirmed: none planned); (c) `onPerfRecord{audioFile}` switches only
  `if (audioEngine_.getSourceMode() != File)` and REFUSES if already recording; (d) `deviceRate` must be read AFTER
  the mode switch (on a BT headset, enabling/disabling the HFP input can change the rate 48k↔16k) — the plan's B5
  order already does this; make it explicit; (e) `tap.prepare()` runs from `audioDeviceAboutToStart` — a mid-take
  restart with a changed rate/channels self-stops the tap (`AudioTap.h:55-59`); R-A5 + B5(c) surface it.
- **N9 — Destructor order.** Keep the shipped "HTTP servers stop FIRST" invariant (`MainComponent.cpp:2112-2130`);
  call `recorderHost_.shutdown(...)` right AFTER `apiServer_->stop()` (post-quit `callAsync` is refused anyway —
  `ApiServer.cpp:430-447` — so either order is safe; do not reorder an existing, reasoned invariant).
- **N10 — §3.3 `applyTempoCommand` dispatch wording contradicts itself** ("tap|manual|link → setManualMode(true)"
  then "tap keeps today's behaviour: setManualBPM only"). Give the builder the table: `tap` → `setManualBPM(bpm)`
  only (`:675-678`, `:6014`); `manual` → `setManualMode(true)` + `setManualBPM(bpm)` if `bpm > 0` (`:685-692`);
  `auto` → `setManualMode(false)`; `resync` → `beatCounter_=0; lastBeatPhase_=0; resetBeatPhase(); resetPhrase()`
  (`:694-703`, `:6020-6031`); `link`/REST/OSC set_bpm → `setManualMode(true)` + `setManualBPM` (`:1915-1922`,
  `:1985-1992`, `:3290-3300`).
- **N11 — Serialization points changed.** T29: `git worktree list` shows ONLY `main` now — each lane needs its own
  worktree or Harmony serializes; T30: `tests/CMakeLists.txt` is currently MODIFIED by the concurrent comp-position
  builder (uncommitted, `git status`) — Lane S3-A's append must land AFTER that commit; its EOF anchor is now
  `catch_discover_tests(test_conn_picker)` (`tests/CMakeLists.txt:983`). Root `CMakeLists.txt` insertion point is
  after `src/recording/AudioStore.cpp` at `:252` (plan says `:247`). Wave 0 (commit H1/L5) is DONE — delete it.
- **N12 — Refusal visibility (HANDOFF loose end 5).** `onManualWrite` also fires with `accepted == false`; count
  Human refusals in `Status.humanRefused` (one int) — the first diagnostic the funnel has ever had. Cheap.
- **N13 — `RecorderHost.h` include rule.** The header lists `recording/AudioTap.h` (pulls `juce_audio_formats`/
  `juce_audio_devices`) while claiming "juce_core only". Forward-declare `class AudioTap;` in the header, include in
  the .cpp. `RecorderHost.cpp` may include `model/Composition.h` (needed by `Program::compile` and
  `PerfStateCapture`); the header keeps its forward declaration.
- **N14 — §3.6 test 1 expectation VERIFIED to hold**: `AudioStore::resolve` reports `Incomplete` on wav-present/
  sidecar-absent without consulting `activeAssetId` (`AudioStore.cpp:374-390`; only `isIncomplete()` excludes the
  active id, `:307-312`) and `AudioTap::start` creates the WAV before the first push (`abandonAsset` comment
  `AudioStore.h:63-65`). No change — recorded so nobody "fixes" it.
- **N15 — §3.7 NOT-touched list gains** `src/connect/*` (incl. `ManualWrite.*`, done), `src/render/*` and
  `src/connect/ScalarParams.h` (the concurrent builder's files), `src/ui/*`.

---

## STALE ANCHORS

| Plan cite | Plan claim | Disk now (VERIFIED) | Status |
|---|---|---|---|
| T1 | `manualWrite` exists nowhere | `src/MainComponent.h:104-111`, `.cpp:3189-3211`; core `src/connect/ManualWrite.{h,cpp}` | STALE → B1 |
| T2 | MainComponent references no recorder type | `MainComponent.h:31` includes `recording/Lane.h` (for `Origin`); still no RecorderClock/Recorder/Store/Player | PARTLY STALE |
| T3 | `ConnectionEngine::tick` NOT called | called at `MainComponent.cpp:3253`; member `connectionEngine_` `.h:373` | STALE → B3 |
| T4 | analysis starts only in production, `:1762-1765` | same logic, now `:1869-1873` | line drift |
| T5 | `setSourceMode(MicInput)` at ctor `:153` | `:260` | line drift |
| T7 | REST set_param/set_layer_opacity write the model inline | both fire callbacks only (`ApiServer.cpp:451-456`, `:541-544`); MainComponent routes them through `manualWrite` (`:1927-1940`) | STALE |
| T8 | 10(+1) direct writer sites | all routed through `manualWrite` (B1 list); LayerStrip widget-gripped | STALE |
| T9 | LayerStrip has no `onDragStart/onDragEnd` | present, `LayerStrip.cpp:430-436` (`gripHeld()`/`release(connNow())`) | STALE |
| T14 | handler anchors + callers `:676, :693, …` | `handleClipTrigger :3958`, `handleColumnTrigger :4116`, `handleDeckSwitch :4690`; `pushCommands` in handlers `:4112`, `:4166`; callers `:783` (genre), `:799` (deckView), `:1489-1512` (tab), `:1907-1909` (REST), `:1949-1955` (OSC), `:5898, :5945, :5988` (bindings) | line drift; genre caller needs `Origin::Engine` (N2) |
| T11 | four file-load auto-plays | six (N4) | UNDER-COUNTED |
| T26/T27 | Decaying grips never expire; no glide | engine ticks (`:3253`); unconnected-param expiry in `gripActive` (`ManualWrite.cpp:157-167`); glide via `handBackGlideMs` | STALE → B1 wording |
| T28 | RecordPanel work is STAGED | committed `dd646de`; callbacks declared `RecordPanel.h:20-22`, never assigned (grep 0 in MainComponent) | now committed |
| T29 | two worktrees | one (`main`) | STALE → N11 |
| T30 | tests/CMakeLists ends with H1's staged block | H1 committed `9257537`; file is 983 lines, EOF = `test_conn_picker` block; currently modified by the concurrent builder | STALE → N11 |
| T33 | H1 fix is staged | committed `9257537` | now committed |
| §3.3 members | `audioEngine_ :222`, `composition_ :334`, declare after `syphonOutput_` | `audioEngine_ :250`, `composition_ :362`, `syphonOutput_ :486` (last member) — declare `recorderHost_` at `:487` | line drift |
| §3.3 B2 | `MainComponent.h:393-416` | `handleClipTrigger/handleColumnTrigger :429-430`, `handleDeckSwitch :452` | line drift |
| §3.3 B1 | before `evaluateAll`; L3(b) unlanded | between `:3232` and `:3246`; use `connNow()` | STALE → B3 |
| §3.4 | `ApiServer.cpp:132`, `:117-224`, `ApiServer.h:58-70`, version `:234` | routes `:146-253`, trigger_clip `:161`, callbacks `.h:58-78` (now incl. `onSetLayerOpacity`, `onSetClipEffectParam`), version `:263` | line drift |
| §3.2 CMake | root `+4 after :247` | `AudioStore.cpp` at `:252`; connect sources `:179-192` | line drift |
| §3.7 Wave 0 | commit staged H1 + L5 first | both committed; nothing recorder-related is staged | DONE → delete |
| §1, §4 | "snap not glide until L3(b)"; continuous rows "only after S3-M" | glide live; rows run in the first gate | STALE → B1/N6 |
| §5 item 1 | R13 "latent", 44.1 kHz | LIVE at 16 kHz (HANDOFF `:2633`) | STALE → B5 |
| §3.6 test 12 | TSan flag `-DADNA_SANITIZE=thread` | `cmake/Sanitizers.cmake` (ASSUMED unchanged — not re-read) | verify |

Anchors that HOLD (re-verified): T10 tempo writers (`:675, :685, :694, :1915, :1985, :3290-3300, :5993-6018,
:6020-6031`); T12 (`:711-738`); T13 (`:1970-1984`, `:5947-5991`, `:6061-6080`, `:6082-6102`); T15 (no
`onQuantizeChanged`/`quantizeMode =`/`onBpmMultiplierChanged` assignment in `MainComponent.cpp` — grep 0); T16
(`PerformanceRecorder.h:28-57`, `.cpp:12-22` resets `take_`); T17 (`PerfState.h:16-19`; `test_audio_store` links no
model, `tests/CMakeLists.txt:769-775`); T18 (`AudioEngine.h:19-53`); T19 (`AudioStore.h` whole); T20 (`Take.h:20-56,
91, 95-96`; `Take.cpp:284-290`); T21 (`Player.h:40-92`; seek-on-decrease `Player.cpp:85-89`); T22
(`Player.h:16-23`; `Program.h:29-46`); T24 (`Lane.h:79-90`); T25 (`Lane.h:50`; `ControlPath.h:41-45`); T31
(`Main.cpp:8`); T32 (VALIDATION row 4; `probe-lane3.sh:110-113` launches via `open`, production, IPv4 7070).

---

## REVISED LANE TABLE — DISJOINT file ownership (S3-M deleted)

| Lane | Owns (nothing else) | Depends on | Size |
|---|---|---|---|
| **S3-A** recording side | NEW `src/recording/RecorderHost.{h,cpp}`; NEW `src/recording/PerfStateCapture.{h,cpp}`; `src/recording/PerformanceRecorder.{h,cpp}` (+3 one-liners: `setCheckpoint0`, `current()`, `hasOpenGesture`); NEW `tests/test_recorder_host.cpp`; `tests/CMakeLists.txt` (append ONE block after `test_conn_picker`, `:983`, AFTER the comp-position builder's commit); root `CMakeLists.txt` (+4 lines after `:252`) | nothing code-side; CMake serialization only | M — commit **A1** (headers + CMake, tree configures) first hour, then **A2** |
| **S3-B** app wiring | `src/MainComponent.h`, `src/MainComponent.cpp` ONLY — member, dtor, B1 tick (amended placement), B2 Origin/deck, B3 choke points, B4 Dispatch lambdas **+ the three `onManual*` hooks and `dispatch.continuous`** (ex-S3-M2), B5 REST callbacks, `Origin::Engine` at the genre caller | A1 (headers), C (callback names) | M |
| **S3-C** REST | `src/api/ApiServer.h`, `src/api/ApiServer.cpp` ONLY (`/api/perf/*`; `onPerfStatus` synchronous, reads only what MainComponent hands it) | nothing | S |
| **S3-G** gate assets | NEW `.harmony/probe-step3.json` (4-clip fixture), NEW `.harmony/probe-step3.sh` (the §4 recipe scripted on the `probe-lane3.sh` skeleton: IPv4 7070, `open` launch, health wait, fixture load, R13 oracle, disk asserts, alignment numbers, graceful `osascript` quit, `screencapture -x` + read), the click-WAV generator | nothing | S |
| **S3-D** docs | `CLAUDE.md` (recording section + source tree), `.harmony/APP-INVENTORY.md`, `.harmony/VALIDATION.md` rows | everything | S |

NOT touched by any lane: `src/connect/*` (the funnel is done — ManualWrite/ConnectionEngine/ParamConnection/
ScalarParams), `src/render/*` and `src/connect/ScalarParams.h` (concurrent comp-position builder), `src/ui/*` (incl.
`RecordPanel.*` = step 4, `LayerStrip.*`, `TopBar.*`, inspectors), `Take.*`, `Program.*`, `Player.*`,
`RecorderClock.*`, `AudioTap.*`, `AudioStore.*`, `Lane.h`, `ControlPath.h`, `PerfState.{h,cpp}`, `AudioEngine.*`,
`CombinedCallback.h`.

Order: **Wave 1 (parallel)** A1, C, G. **Wave 2 (parallel)** A2, B. **Wave 3 (Harmony)** ONE clean forced rebuild
in ONE directory, `ctest` (RUN it), then `bash .harmony/probe-step3.sh`. No serial M2 wave exists any more.
Conventions per lane unchanged: own worktree + own `-B build-<lane>`; `git add -f` under `.harmony/`;
`git show --stat HEAD` per commit; create a file BEFORE the CMake line that references it.

---

## AMENDMENTS — apply verbatim to `.harmony/specs/s-rta-0923-step3-plan.md`

**A1 (B1) — §1 replace the "Decision" block; §3.5 delete; §3.7 delete the S3-M row; §7 items 2, 3, 11 rewrite.**
§1 Decision becomes: "Half A and half B BOTH proceed now. The funnel is `MainComponent::manualWrite/manualTouch/
manualRelease` (`src/MainComponent.h:104-111`, R8 — owned by the connection lane, shipped 75560f9); the recorder
HOOKS `onManualWrite/onManualTouch/onManualRelease` (`.h:115-117`) and fills `Dispatch::continuous` with the three
wrappers. No new files under `src/connect/`." §3.3 B4 gains this text (Lane S3-B, in the ctor after the `Dispatch`
lambdas):
```cpp
// s-rta-0924 step 3: the recorder HOOKS the connection lane's funnel (R8). Replay writes carry
// Origin::Replay -> handFor() gives them Hand::Lane (MainComponent.cpp:159-163), so a human hand
// always wins (D8) and the capture hooks below never record them (D6 origin rule).
recorderHost_.dispatch.continuous.touch   = [this](const ControlPath& k, const std::string& /*grip*/) {
    return manualTouch(k, GripKind::Held, Origin::Replay); };               // s167 spec :346-348: begin -> gripHeld()
recorderHost_.dispatch.continuous.set     = [this](const ControlPath& k, float v) {
    return manualWrite(k, v, GripKind::Held, Origin::Replay); };
recorderHost_.dispatch.continuous.release = [this](const ControlPath& k) { manualRelease(k, Origin::Replay); };

onManualWrite   = [this](const ControlPath& k, float v, GripKind g, Origin o, bool ok) {
    if (o != Origin::Human) return;
    if (ok) recorderHost_.onHumanWrite(k, v, g == GripKind::Held ? "held" : "decaying");
    else    recorderHost_.noteHumanRefused(k); };                            // N12: first refusal diagnostic
onManualTouch   = [this](const ControlPath& k, GripKind g, Origin o, bool ok) {
    if (o == Origin::Human && ok) recorderHost_.onHumanTouch(k, g == GripKind::Held ? "held" : "decaying"); };
onManualRelease = [this](const ControlPath& k, Origin o) {
    if (o == Origin::Human) recorderHost_.onHumanRelease(k); };
```
Reviewer grep for S3-B (add): `onManualWrite = \|onManualTouch = \|onManualRelease = ` → exactly one assignment each
in `MainComponent.cpp`; `manualTouch(\|manualWrite(\|manualRelease(` with `Origin::Replay` → only inside the three
`dispatch.continuous` lambdas.

**A2 (B2, N1) — §3.1 replace the capture block:**
```cpp
// Continuous capture: MainComponent's onManualWrite/onManualTouch/onManualRelease subscribers call these
// (origin Human, accepted only). grip is "held" | "decaying" (Lane.h Gesture::grip vocabulary).
// onHumanWrite OPENS the gesture if none is open on `key` (every OSC/MIDI/REST site is a single
// manualWrite call with no preceding manualTouch -- PerformanceRecorder::set ignores a set() with no
// open gesture, PerformanceRecorder.cpp:75-79).
void onHumanTouch(const ControlPath& key, const std::string& grip);
void onHumanWrite(const ControlPath& key, float v, const std::string& grip);
void onHumanRelease(const ControlPath& key);
void noteHumanRefused(const ControlPath& key);           // Status.humanRefused++ (diagnostic only)
```
Delete `enum class GripKind` from `RecorderHost.h`; `Dispatch::Continuous::touch` takes `const std::string& grip`.
§3.2: `PerformanceRecorder.h` +3 (not +2): add `bool hasOpenGesture(const ControlPath& k) const { return
openGestures_.count(k) != 0; }`. §3.6: test #14 rewritten per B2; add test #15 (`onHumanWrite` with no prior touch
opens the gesture); test #10 keeps the empty-seam refusal path (FakeDispatch only).

**A3 (B3) — §3.3 B1 replace the placement sentence and the hunk header with:** "inserted between
`globalMacroBank_.updateValues(signalRegistry_);` (`:3232`) and `const double now = connNow();` (`:3246`) — the
placement contract at `:3237-3244` / `ConnectionEngine.cpp:266-273`. Hoist `const double now = connNow();` ABOVE the
block and pass `now` as `wallNow` (one clock for grips and stamps, `ConnClock.h`)." Delete "(L3(b), when it lands —
insert it AFTER this block)". T3 → "ConnectionEngine::tick IS called (`:3253`)".

**A4 (N2, N4, N5, N9, N10) — §3.3:** B2: the genre caller (`:783`) passes `Origin::Engine`; B3 `applyAudioTransport`
sites: six file-load auto-plays (`:287, :294, :623, :630, :3169, :3591`) + `GlobalPlayPause` + `GlobalStop`;
`applyTempoCommand` dispatch table per N10; add "activeClip points are PRESS-time (queued under Quantize);
`play()` applies `checkpoint0.quantizeMode` through the `quantize` dispatcher before the first `advanceTo`";
`~MainComponent()`: `recorderHost_.shutdown(...)` immediately AFTER `apiServer_->stop()` (not first).

**A5 (B5, N3, N7, N8) — §3.1 `ArmOptions` + `Status` + `tick`:** add `double analysisRate` (MainComponent passes
`AnalysisThread::kSampleRate`) and `float gripHoldMs` (from `composition_.gripHoldMs`) to `ArmOptions`; add
`double deviceRate`, `bool rateMismatch`, `int humanRefused` to `Status` (published by `tick()`); arm-time rule and
marker per B5(a); mid-take self-stop → `notify` + `lastError` per B5(c); the synthesized Decaying end uses
`gripHoldMs` (N7). §3.3 B5: `onPerfRecord` refuses while recording; switches mode only if `getSourceMode() != File`;
reads `deviceRate`/channels AFTER the switch. §3.4: `onPerfStatus` reads nothing but `recorderHost_.status()`.

**A6 (B4, N6, B5 d) — §4 recipe:** after the health/bpm lines and BEFORE `perf/record`, insert
`curl -s -X POST http://127.0.0.1:7070/api/load_composition -d '{"path":"'$PWD'/.harmony/probe-step3.json"}'`
(fixture = `.harmony/probe-lane3-layer.json` with source clips in layer 0 columns 0..3, no connections, quantize
Off); precondition oracle: `curl -s …/api/perf/status | python3 -c 'import sys,json; d=json.load(sys.stdin);
assert d["deviceRate"]==48000 and not d["rateMismatch"]'` and `! grep -q "analysis pipeline assumes"
/tmp/adna-err.log`; trigger columns `0 1 2 3` and expect `activeClipColumn` 0→1→2→3 on replay; replace the
continuous row's expectation with N6's wording; the recipe is scripted as `.harmony/probe-step3.sh` (Lane S3-G) on
the `probe-lane3.sh` skeleton, teardown by `osascript -e 'quit app "Audio-DNA"'` (NOT `pkill` — `~MainComponent`
must run `shutdown`), `pgrep -f 'MacOS/Audio-DNA'` empty, `screencapture -x` read. Screen-safety re-verified: no
`/api/perf/*` or recipe endpoint reaches `openOutputOnDisplay` (`MainComponent.cpp:3337`; callers `:498, :672,
:3076, :4745` are menu/selector/keyboard paths only).

**A7 (N11, N13, N15) — §3.2/§3.7:** root `CMakeLists.txt` +4 after `:252`; `tests/CMakeLists.txt` append after
`:983`, AFTER the comp-position builder's commit lands; delete Wave 0; NOT-touched list per N15; `RecorderHost.h`
forward-declares `class AudioTap;`.

**A8 (§0 table) — mark T1, T3, T7, T8, T9, T26, T27, T29, T30 STALE with the disk facts in the table above; T28/T33
"committed"; T11 "six"; T14 line numbers per the table.**

---

## What I could not verify (say so)
- `cmake/Sanitizers.cmake` TSan flag name (§3.6 #12) — not re-read this pass (ASSUMED unchanged since the plan).
- Whether the comp-position builder's pending `tests/CMakeLists.txt` edit will conflict with S3-A's append beyond
  ordering (INFERRED: append-at-EOF is merge-safe once serialized).
- The BT-headset crash itself (only Boris can re-test); this critique only bounds step 3's contact with the device
  lifecycle (N8).

REPORT_FILE: /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/b1451bb3-62f7-434f-b25f-ce96c0653c67/scratchpad/step3-critic.md
STATUS: COMPLETE
