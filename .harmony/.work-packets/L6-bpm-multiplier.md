# L6 — BPM multiplier work packet

## VERDICT: BLOCKED — not a clean single factor. STOP AND REPORT (this is the successful outcome the recon was asked to reach).

`bpmMultiplier` cannot be applied as one multiplicative factor at the analysis
publish point without either (a) corrupting a real internal consumer
(GenreDetector's absolute-BPM genre buckets) that sits inside the SAME publish
sequence, or (b) leaving ~98% of the visible beat-reactive surface (56 of 57
shader hookups, all phase-driven) completely unaffected, or (c) introducing a
brand-new cross-thread dependency into the one component in this codebase that
is explicitly hardened to zero TSan suppressions. Full evidence below.

## SIZE: N/A (no buildable diff recommended). If forced to ship a
deliberately-scoped partial version, see "SMALLEST NON-INCOHERENT FALLBACK"
near the end — that would be `small` (2 files, ~10 lines), but it is a
materially incomplete feature and must be presented to Boris as such, not as
"BPM multiplier: done."

---

## CURRENT BEHAVIOUR (verified)

`bpmMultiplier` is a live, reachable, completely inert UI control:

- **Declaration**: `src/model/Composition.h:49`
  `int bpmMultiplier = 1; // -4 = ÷4, -2 = ÷2, 1 = ×1, 2 = ×2, 4 = ×4`
- **UI writer**: `src/ui/TopBar.cpp:290`, inside `TopBar::handleMultiplierButton(int multiplier)`:
  `composition_.bpmMultiplier = multiplier;`
  `composition_` is a real reference (`Composition& composition_;` — `src/ui/TopBar.h:49`), not a copy, so this write lands on the live model.
- **UI is wired and visible**: five real `juce::TextButton`s (`/4`, `/2`, `x1`, `x2`, `x4`) declared `src/ui/TopBar.h:78-82`, constructed and click-bound `src/ui/TopBar.cpp:137-141` (`btn.onClick = [this, mult] { handleMultiplierButton(mult); };` — `src/ui/TopBar.cpp:135`), laid out in `resized()` `src/ui/TopBar.cpp:487-495`. A user can click these today and nothing downstream reacts.
- **Dead broadcast**: `handleMultiplierButton` also fires `onBpmMultiplierChanged(multiplier)` (`src/ui/TopBar.cpp:312-313`) through a `std::function<void(int)> onBpmMultiplierChanged;` member (`src/ui/TopBar.h:27`). That callback is never assigned anywhere in the tree — see CALL-SITE ENUMERATION. It is a second dead path, not a working one.
- **Serialized**: yes. `src/model/Composition.h:169` (`toVar()`: `obj->setProperty("bpmMultiplier", bpmMultiplier);`) and `src/model/Composition.h:260` (`fromVar()`: `bpmMultiplier = static_cast<int>(obj->getProperty("bpmMultiplier"));`). Also reset to `1` in `initDefault()` (`src/model/Composition.h:120`).
- **Zero behavioural consumers**: confirmed by two independent grep patterns (see below) — the only non-declaration, non-serialization touches are the UI writer and a test that round-trips the field through JSON and asserts equality (`tests/test_composition.cpp:140,177`), never asserting any effect.

## ROOT CAUSE (verified)

Not a bug — an unfinished feature. The field was added to the model and wired to UI buttons, but the "apply it somewhere" step was never done, and per this recon, there is no single place downstream where it CAN be done cleanly (see below).

## FILES TOUCHED

None — no code change is recommended by this packet. Files read for this recon (exhaustive, read-only):

- `src/model/Composition.h` — field, serialization, default
- `src/ui/TopBar.h`, `src/ui/TopBar.cpp` — UI writer, dead callback, thread (message thread, `startTimerHz(15)` at `TopBar.cpp:211`)
- `src/analysis/FeatureSnapshot.h` — the 28-field-family "protected asset" struct (see PROTECTED-ASSET COUNT below)
- `src/analysis/AnalysisThread.h`, `src/analysis/AnalysisThread.cpp` — the analysis thread, its 14-stage pipeline, the proposed "publish point"
- `src/analysis/BPMTracker.h` (declarations only read) — source of `snap->bpm`/`beatPhase`/etc.
- `src/analysis/GenreDetector.h`, `src/analysis/GenreDetector.cpp` — absolute-BPM-range genre classifier fed by `snap->bpm` inside the SAME publish sequence
- `src/features/FeatureBus.h` — the seqlock bus; comment block documents "TSan-clean with ZERO suppressions," single-writer enforced by the type system
- `src/sync/LinkSync.h`, `src/sync/LinkSync.cpp` — Ableton Link tempo source
- `src/MainComponent.cpp` (relevant excerpts: `tickFeaturePipeline()` ~2549, `timerCallback()` Link-feed block ~2585-2595, destructor thread-shutdown comments ~1860-1888) — where Link's BPM gets pushed INTO the tracker, and documented "status-quo field-level race" precedent
- `src/render/Renderer.h`, `src/render/Renderer.cpp` — holds a raw `Composition*`, already reads plain Composition scalars on the GL thread (precedent); BPM-sync clip-speed math; playlist beat-phase-wrap advance; `autopilot_.processFrame` call site
- `src/render/CompositorEngine.cpp` — `u_bpm`/`u_beatPhase` uniform upload (GL thread)
- `src/render/EmbeddedShaders.h` — shader uniform declarations and usage counts (see evidence)
- `src/effects/EffectChain.cpp`, `src/effects/EffectChain.h` — `bpm`/`beatPhase` uniform upload + documented "scalar tearing, not container corruption" race precedent
- `src/sources/ProceduralSource.cpp` — `bpm`/`beatPhase` uniform upload
- `src/model/Autopilot.cpp` — beat-crossing detection via `beatPhase` wrap, on the GL thread
- `src/signal/EnvelopeSignal.h`, `src/signal/OscillatorSignal.h` — `beatPhase + beatInBar` used as modulation phase
- `src/mapping/MappingEngine.h`, `src/mapping/MappingEngine.cpp` — generic `MappingSource::BPM` / `MappingSource::BeatPhase`, confirmed to run on the MESSAGE thread (not GL) via the class-header comment
- `src/api/ApiServer.cpp` — exposes raw `bpm`/`beatPhase` over HTTP (external consumers)
- `src/test/TestServer.cpp` — test-only snapshot injection (not production)
- `src/core/TriggerCommands.h` — documented precedent comment for "status-quo field-level race" on plain Composition-adjacent scalar fields
- `tests/test_composition.cpp` — the only test touching the field (serialization round-trip only)

## CALL-SITE ENUMERATION — grep commands run + raw output

**1. Every `bpmMultiplier` touch (declaration, read, write) — two independent patterns:**

```
$ grep -rn "bpmMultiplier" --include="*.cpp" --include="*.h" --include="*.hpp" .
tests/test_composition.cpp:140:    comp.bpmMultiplier = 2;
tests/test_composition.cpp:177:    REQUIRE(loaded.bpmMultiplier == 2);
src/ui/TopBar.cpp:290:    composition_.bpmMultiplier = multiplier;
src/model/Composition.h:49:    int bpmMultiplier = 1; // -4 = ÷4, -2 = ÷2, 1 = ×1, 2 = ×2, 4 = ×4
src/model/Composition.h:120:        bpmMultiplier = 1;
src/model/Composition.h:169:        obj->setProperty("bpmMultiplier", bpmMultiplier);
src/model/Composition.h:260:            bpmMultiplier = static_cast<int>(obj->getProperty("bpmMultiplier"));

$ grep -rn "\.bpmMultiplier\|->bpmMultiplier" --include="*.cpp" --include="*.h" .
tests/test_composition.cpp:140:    comp.bpmMultiplier = 2;
tests/test_composition.cpp:177:    REQUIRE(loaded.bpmMultiplier == 2);
src/ui/TopBar.cpp:290:    composition_.bpmMultiplier = multiplier;
```
Both patterns agree: **zero read-consumers** anywhere in `src/` or `tests/`. Only writer (UI), only reader-back is the JSON round-trip test which never asserts a behavioural effect.

**2. The dead `onBpmMultiplierChanged` callback — proving it's never wired:**

```
$ grep -rn "onBpmMultiplierChanged" --include="*.cpp" --include="*.h" .
src/ui/TopBar.h:27:    std::function<void(int)> onBpmMultiplierChanged;
src/ui/TopBar.cpp:312:    if (onBpmMultiplierChanged)
src/ui/TopBar.cpp:313:        onBpmMultiplierChanged(multiplier);

$ grep -rn "onBpmMultiplierChanged\s*=" --include="*.cpp" --include="*.h" .
(no output)

$ grep -rn "\.onBpmMultiplierChanged\|->onBpmMultiplierChanged" --include="*.cpp" --include="*.h" .
(no output)
```
Two patterns agree: the callback is declared and invoked but never assigned. Confirmed dead.

**3. All downstream consumers of `.bpm` (the FeatureSnapshot field, NOT bpmMultiplier):**

```
$ grep -rn "\.bpm\b\|->bpm\b" --include="*.cpp" --include="*.h" src | grep -v "src/analysis/"
src/mapping/MappingEngine.cpp:84:        case MappingSource::BPM:              return snap.bpm;
src/ui/TopBar.cpp:108,217,245,246,360,403               (readout label, beat-flash gating)
src/ui/AudioReadoutPanel.cpp:35,125,126                 (readout label)
src/test/TestServer.cpp:425                             (test snapshot injection)
src/render/Renderer.cpp:1123,1187,1195                  (BPM-sync clip/image-sequence playback rate)
src/render/CompositorEngine.cpp:1445                    (u_bpm uniform upload)
src/sources/ProceduralSource.cpp:198                    (bpm uniform upload)
src/effects/EffectChain.cpp:355                         (bpm uniform upload)
src/api/ApiServer.cpp:248,542,587,623                   (HTTP GET/PUT bpm — external consumers)
```

**4. All downstream consumers of `.beatPhase`:**

```
$ grep -rn "\.beatPhase\b\|->beatPhase\b" --include="*.cpp" --include="*.h" src | grep -v "src/analysis/"
src/MainComponent.cpp:2997,3161,3295,3297               (phase-based logic, message-thread ticks)
src/ui/AudioReadoutPanel.cpp:36,317                      (visual readout)
src/mapping/MappingEngine.cpp:83                         (MappingSource::BeatPhase — generic mod source)
src/test/TestServer.cpp:426                              (test injection)
src/render/Renderer.cpp:308,309                          (playlist-advance beat-phase-wrap edge detector)
src/render/CompositorEngine.cpp:1433                     (u_beatPhase uniform upload)
src/ui/TopBar.cpp:221,379                                (beat-flash brightness)
src/signal/EnvelopeSignal.h:36 / OscillatorSignal.h:28   (totalBeatPhase = beatPhase + beatInBar, modulation source)
src/sources/ProceduralSource.cpp:170                     (beatPhase uniform upload)
src/model/Autopilot.cpp:44,45                            (autopilot beat-crossing edge detector)
src/api/ApiServer.cpp:249,543,588,624                    (HTTP GET/PUT beatPhase)
src/effects/EffectChain.cpp:331                          (beatPhase uniform upload)
```

**5. Shader uniform usage counts (why "just scale bpm" barely moves the visuals):**

```
$ grep -c "uniform float u_bpm;" src/render/EmbeddedShaders.h
1
$ grep -n "u_bpm" src/render/EmbeddedShaders.h | grep -v "uniform float u_bpm;"
10733:        p.x += u_time * (u_bpm > 0.0 ? u_bpm / 120.0 : 1.0) * 0.3;
$ grep -c "uniform float u_beatPhase;" src/render/EmbeddedShaders.h
56
```
`u_bpm` is declared once and used for real math in exactly ONE shader. `u_beatPhase` is declared and used for real-time pulse/rotation/sine animation in 56 shaders (e.g. `src/render/EmbeddedShaders.h:8741`: `pulse = 1.0 + pulseAmt * sin(u_beatPhase * 6.28318 + fi * 0.1);`).

**6. GenreDetector's absolute-BPM genre buckets (why scaling upstream of stage 13 breaks classification):**

```
$ grep -n "bpm" src/analysis/GenreDetector.cpp
36:    float bpm = features.bpm;
...
50:        if (bpmLocked && bpm >= 115.0f && bpm <= 135.0f)   // House
70:        if (bpmLocked && bpm >= 120.0f && bpm <= 150.0f)   // Techno
88:        if (bpmLocked && bpm >= 155.0f && bpm <= 185.0f)   // DnB
104:       if (bpmLocked && bpm >= 75.0f  && bpm <= 105.0f)   // Hip-Hop
139:       if (bpmLocked && bpm >= 95.0f  && bpm <= 150.0f)   // (further genre)
```
These are hard-coded absolute BPM windows (max ~185). AnalysisThread.cpp stage order: stage 5 (`AnalysisThread.cpp:173`) sets `snap->bpm = bpmTracker_->bpm()`; stage 13 (`AnalysisThread.cpp:260`) reads it back as `gf.bpm = snap->bpm` for genre scoring — **inside the same function, same thread, same snapshot, before publish**. Any multiplier applied to `snap->bpm` before stage 13 corrupts genre detection (e.g. a 125 BPM house track at "×2" reads as 250 BPM, outside every bucket).

**7. Ableton Link is the tempo SOURCE, not a downstream consumer — confirmed one-directional:**

```
$ grep -rn "getBPM()\|linkSync_\." --include="*.cpp" --include="*.h" src | grep -v "src/sync/LinkSync"
src/MainComponent.cpp:2587-2590   (only call site: linkSync_.isEnabled(), .update(), .getBPM())
$ grep -rn "setBPM(" --include="*.cpp" --include="*.h" src | grep -v "src/sync/LinkSync"
(no output — nothing in the app ever pushes a BPM value TO Link)
```
`src/MainComponent.cpp:2585-2595` (in `timerCallback()`, message thread, 30Hz): when Link is enabled, `linkBPM` is pushed INTO `bpmTracker_->setManualBPM()`, which becomes `snap->bpm` at the next publish. So Link already IS the analysis pipeline's tempo source when active; there is no separate "Link's own bpm" downstream value to protect from double-scaling — but scaling `snap->bpm` after that point still corrupts everything else that trusts it as ground truth (TopBar readout, ApiServer, GenreDetector — see #6).

**8. Threads each consumer group runs on — verified via source comments + class hierarchy, not inferred:**

- **Message thread**: `TopBar` (`startTimerHz(15)` — `TopBar.cpp:211`); `MappingEngine::processFrame` — explicitly documented in `src/mapping/MappingEngine.h:21-25`: *"processFrame() is called on the MESSAGE thread by MainComponent's mapping tick timer... NOT a render/GL thread."*; `MainComponent::timerCallback` (Link feed).
- **GL/render thread**: `Renderer` is a `juce::OpenGLRenderer` (`src/render/Renderer.h:41`, `renderOpenGL()` override at `:72`) — `CompositorEngine`'s uniform uploads, `EffectChain`'s uniform uploads, `ProceduralSource`'s uniform uploads, `Renderer.cpp`'s BPM-sync clip playback math, and `autopilot_.processFrame(*deck, snap)` (`Renderer.cpp:257`, called from inside the render pass) all execute here.
- **Analysis thread**: `AnalysisThread : public juce::Thread` (`AnalysisThread.h:41`) — `BPMTracker`, `GenreDetector`, and the `snap->bpm`/`beatPhase`/etc. assignment (the proposed injection point) all run here.
- **HTTP worker thread(s)**: `ApiServer.cpp` GET/PUT of `bpm`/`beatPhase` — a fourth thread family, external.

So a plain `int bpmMultiplier` on `Composition`, written only from the message thread today, would need to be read from at least three OTHER thread contexts (GL, analysis, HTTP) to reach "every consumer" — not one safe read site.

**9. Existing accepted-race precedent (so this isn't unprecedented, but it IS non-trivial) — confirmed:**

```
$ grep -n "composition_->" src/render/Renderer.cpp
460: composition_->decks
552: composition_->crossfaderBlendMode      (glUniform1i on GL thread)
568: composition_->activeDeckIndex
587: composition_->globalTransitionSpeed
1864-1869: composition_->compPositionX/Y/compScale/compRotation/compAnchorX/Y
```
`Renderer` already holds `Composition* composition_` (`Renderer.h:342`) and reads several plain scalar Composition fields directly on the GL thread, with no atomics — and this is DOCUMENTED as an accepted pattern elsewhere in the codebase: `src/core/TriggerCommands.h:33-35` — *"a trigger only writes per-layer runtime fields... in place — status-quo field-level race, exactly like today's direct writes. The fence is reserved for vector-structure ops."* So reading `composition_->bpmMultiplier` (a scalar int) on the GL thread, IF the change were confined to Renderer/CompositorEngine/EffectChain/ProceduralSource (all of which already touch `Composition*` or are one hop from it), would be consistent with the codebase's existing risk tolerance — it would NOT be introducing a new class of race, just one more instance of an already-accepted one. This is the one piece of good news in this recon.

What is NOT precedented or accepted: reaching `Composition` from the **analysis thread**, which today has zero reference to `Composition` (`AnalysisThread`'s constructor takes only `RingBuffer<float>&`; no `Composition*`/`&` member exists — confirmed by reading the full private member list in `AnalysisThread.h:78-152`) and whose `FeatureBus` is explicitly called out (`src/features/FeatureBus.h:15-19`) as *"TSan-clean with ZERO suppressions."* That is the "protected asset" the essentials plan flagged, and the plan's proposed injection point (analysis publish point) is exactly the point that would need this new, unprecedented cross-thread wiring.

## THE CHANGE

Not proposed — see VERDICT. If Boris wants to proceed anyway, two structurally different paths exist and a builder must be told explicitly which one, because they are not compatible:

**Path A — scale at the analysis publish point (the plan's original ask).** Requires: (1) a NEW thread-safe channel from Composition (message thread) into AnalysisThread (analysis thread) — e.g. `std::atomic<int>` mirror set by TopBar and polled/read at the top of `run()`'s per-hop loop; (2) splitting `FeatureSnapshot::bpm` into two fields (a genre-detector/internal "true" bpm and a published/display "scaled" bpm) to avoid corrupting GenreDetector (evidence #6) — this alone breaks "one multiplicative factor, smallest possible diff"; (3) a real per-consumer decision for every phase field (`beatPhase`, `barPhase`, `phrasePhase`, `beatInBar`, `downbeatDetected`) because a memoryless scalar multiply of a wrapped [0,1) ramp is only phase-correct for the integer-multiply directions (×2, ×4 — provably: `frac(k·x) = frac(k·frac(x))` for integer k) and is NOT expressible without extra parity state (from `beatInBar`/`barCount`) for the divide directions (÷2, ÷4). This is a redesign, not a diff.

**Path B — scale only isolated, already-scalar, non-phase consumers, confined to the GL thread where `Composition*` is already available.** This is the only path consistent with "smallest possible diff" and the existing race-tolerance precedent (evidence #9). It would touch exactly the two places that consume raw scalar `bpm` for real math and are NOT also feeding GenreDetector or the tempo readout:
  - `src/render/Renderer.cpp:1195` (`secondsPerCycle = clip->beatDivision * 60.0f / snap.bpm;`) — image-sequence BPM-sync playback rate.
  - `src/render/EmbeddedShaders.h:10733` (`u_bpm / 120.0`) — the one shader that actually uses `u_bpm` for motion.

  This would NOT touch `AnalysisThread.cpp`, `FeatureBus`, or `GenreDetector` at all — the "protected asset" stays untouched, which is good — but it would also visibly do almost nothing: it leaves all 56 `u_beatPhase`-driven shaders, `Autopilot`'s beat-crossing, `Renderer`'s playlist-advance, and `MappingEngine`'s `BeatPhase` source completely unaffected. A user clicking "×2" would see one background-texture-scroll shader speed up and nothing else visibly change. This should be surfaced to Boris explicitly as "technically correct, practically invisible" before anyone builds it — do not let a builder silently ship Path B and call the feature done.

Recommendation: report BLOCKED to Boris with these two paths named, and let him decide whether the actual product intent ("make it feel like the tempo doubled") is worth the Path-A redesign, or whether the feature should simply be removed/hidden (delete the dead buttons) since it currently misleads users into thinking a working control exists.

## FENCE

No files — this lane makes no source change. (If Boris orders Path B specifically: `src/render/Renderer.cpp`, `src/render/EmbeddedShaders.h` only — but that decision is explicitly deferred to Boris, not made by this packet.)

## TRAPS

- **The GenreDetector collision (evidence #6)** is the sharpest trap: it's INSIDE the analysis thread's own pipeline, so "apply the factor right before publish" still runs before/through GenreDetector's consumption unless the multiplier is applied to a COPY used only for the outward-facing snapshot fields, not the internal `gf.bpm` feed — a builder who doesn't re-derive this will silently break genre detection at any non-1x multiplier.
- **Divide-direction phase math is not a scalar multiply.** `frac(x/2)` needs the beat parity (odd/even full-beat count), which is NOT present in the instantaneous `beatPhase` float alone — a builder tempted to write `beatPhase * multiplierAsFloat` (treating -2 as 0.5×) will get a sawtooth that snaps/glitches every other beat, not a genuine half-time ramp. (Note in passing: ÷4 accidentally already exists as a *different* field — `barPhase`, defined as "[0,1) over 4 beats" — but that's a coincidence of this app's fixed 4-beat bar, not a general solution, and switching a consumer from `beatPhase` to `barPhase` changes which physical signal it's wired to, not "scaling.")
- **Ableton Link direction.** Link is upstream (feeds `bpmTracker_->setManualBPM`), not downstream — a builder who assumes "Link must not be scaled" needs to protect the FEED path (already fine, untouched) but must not conflate that with protecting `snap->bpm` in general, which Link itself also relies on being real once it's published (nothing currently reads Link's OWN bpm back out for comparison, but external API consumers reading `ApiServer`'s `bpm` field would see a scaled/fake number with no way to know it's not the real tempo).
- **Three-thread consumer spread.** Any fix that isn't confined to the GL thread (where `Composition*` precedent already exists — evidence #9) needs real synchronization, not a plain int. Message-thread-only consumers (`MappingEngine`) are fine as-is; GL-thread consumers match existing accepted-race precedent; analysis-thread and HTTP-thread consumers do not have precedent and would need an atomic or a snapshot-time copy.
- **`onBpmMultiplierChanged` is a second dead path** — if a builder "fixes" this feature by wiring that callback instead of reading `composition_.bpmMultiplier` directly, they still have to solve the exact same cross-thread/consumer problem; the callback doesn't sidestep any of the above, it just relocates where the read happens (and today nothing calls `topBar_->onBpmMultiplierChanged = ...` in `MainComponent.cpp`, so that's also a build step, not a wire-up).

## HOW TO PROVE IT WORKS

N/A — no change recommended. If Path B is explicitly ordered by Boris despite the "practically invisible" caveat, proof would be: play a track with a locked BPM, load an image-sequence clip in `BPMSync` transport mode, click `x2`, and visually confirm the image sequence's cycle rate doubles while the numeric tempo readout in the TopBar (`tempoLabel_`) stays showing the REAL detected BPM (not doubled) — a human needs to watch this, no automated test exercises the render/GL path.

## OUT OF SCOPE

- Actually implementing Path A or Path B — this packet is recon only, per the hard rules.
- Fixing the `onBpmMultiplierChanged` dead callback — out of scope for this lane regardless of path chosen, since it doesn't change the core finding.
- Any decision about whether to remove the dead UI buttons instead of wiring them — that's a product call for Boris, flagged as an option, not decided here.
- Auditing whether `MidiOutputHandler`/`MidiHandler` (`src/midi/`) emit an MIDI clock — checked (see below), found no tempo/clock output at all, so "MIDI clock" from the task's consumer list does not exist as a consumer in this codebase; not a gap in this recon, just an empty category.

## OPEN QUESTIONS

- **Product intent**: does Boris want "×2" to mean "the whole show, including the numeric BPM readout and genre detection, believes the track is 2x tempo" (Path A, full redesign, touches the protected asset for real) or "just make the visuals pulse faster while everything analytical stays honest" (Path B, small, GL-thread-only, but nearly invisible given the 56:1 beatPhase:bpm shader ratio)? This packet cannot resolve that from source — it's a product decision, not a code fact.
- **MIDI clock**: confirmed absent as a consumer (`grep -rn "bpm\|tempo\|clock" src/midi/*.cpp src/midi/*.h` → no output) — flagging this as a genuine "does not exist" finding per the hard rules' two-pattern requirement:
  ```
  $ grep -rln "bpm\|Bpm\|BPM\|tempo\|Tempo\|clock\|Clock" src/midi/*.cpp src/midi/*.h
  (no output)
  $ grep -rln "MidiClock\|kMidiClock\|0xF8\|sendClock" src/midi/*.cpp src/midi/*.h
  (no output)
  ```
  Two independent patterns agree: no MIDI clock consumer exists in this codebase today.
