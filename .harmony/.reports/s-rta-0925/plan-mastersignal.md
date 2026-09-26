# Master Signal — buildable two-step plan (s-rta-0925)

Architect (Fable), 2026-09-25, against main `6d55fc9`. Inputs folded: the approved design
(`diag-mastersignal.md`), the critic's BLOCKING ordering finding + MEDIUM macro double-depth
(`critic-mastersignal.md`), Boris's rulings Q1-Q3 (`binding-decisions.md` 2026-09-25, end), and
main's one-master change (`16b133d`: the TopBar fader is a direct widget-grip view of
`CompScalar::Opacity`, NOT a funnel writer). Every code claim is `path:line` against `6d55fc9`;
confidence VERIFIED (read) / INFERRED (derived) / ASSUMED (builder verifies).

QUESTION: turn the design into two buildable, separately-mergeable steps — STEP 0 (effect/source
parameter twins render-live; nothing visible changes at 100%) and STEP 1 (Master Signal) — with
files, RED tests, gates, and what Boris sees.

APPROACH: Step 0 = the s166 L3 vertical slice, scoped to (a) renderer reads `effParam/effDryWet/
live.effective`, (b) engine publishes source-param twins always + fires the standalone-source
refresh, (c) effect/source rows bound to the model's `ParamConnection`s and the two `tickModulation`
loops become display-only, plus the R-A sizing fix (`EffectSlot::addParam`). Step 1 = one new
`CompScalar::Signal` / `Composition::masterSignal`, one `applyDepth` at the point where a SIGNAL
enters a chain (`ConnectionEngine::evaluate`, non-Macro sources; `MacroBank::updateValues`; v1
`MappingEngine`), read ONCE per tick above all three consumers, a TopBar fader built exactly like
main's Master fader, bindable via MIDI/OSC/REST, persisted (absent -> 1.0). NO snapshot / shader-
uniform scaling (Boris Q2), so Step 1 touches nothing on any GL thread.

---

## 0. Decisions that CHANGE the design (read before building)

| # | Change | Why |
|---|---|---|
| D1 | Design Step 3 (`scaleSignalLevels` on `frameSnap_`/`OutputRenderer`) is DELETED. No `Renderer.cpp`, `OutputWindow.*` or `ProceduralSource` change in Step 1. | Boris Q2: "keep pulsing, this control is only for signals" — effects/sources reading the beat clock or audio uniforms directly keep pulsing at 0%. `SignalDepth.h` ships `applyDepth` only. |
| D2 | The depth is read ONCE at the top of `MainComponent::tickFeaturePipeline`, ABOVE `MappingEngine::processFrame` (`src/MainComponent.cpp:3318`) and `globalMacroBank_.updateValues` (`:3328`), and passed to all three consumers (those two + `Context` at `:3360`). | Critic BLOCKING finding 1 (VERIFIED order: evaluateAll :3316 -> processFrame :3318 -> updateValues :3328 -> recorder :3342 -> ctx :3360 -> engine :3362 -> tickModulation :3364). |
| D3 | Depth is applied EXACTLY ONCE per chain, at the point where a SIGNAL enters: `evaluate()` applies it for `Kind::Signal/Lfo/Envelope/ClipPosition` and SKIPS `Kind::Macro`; a macro's own connection (`ConnectionEngine.cpp:287-299`, evaluated through the same `evaluate`) and the legacy `sourceSignalId` path (`MacroBank.h:74-87`) take the depth. | Critic MEDIUM finding 2 (double application on macro chains) — resolved by construction, not by exempting the macro loop. Consequences at 0%: a signal-driven macro knob sits at its hand value; every control linked to it follows the (now static) knob; a HAND-turned macro keeps fanning out (it is a hand, not a signal — Boris: "only for signals"); every control connected directly to audio/LFO/envelope/clip-position sits at its own hand value. Strongest counterargument + one-line switch: §5 R1. |
| D4 | The new fader copies main's Master fader verbatim (`src/ui/TopBar.cpp:211-224, 318-331`): direct model write + `scalarConns[]` grip + 15 Hz sync-back. Not the `manualWrite` funnel. | Task instruction "follow that same pattern". Consequence: a TopBar drag is NOT captured by the recorder (identical to Master today — the recorder's only capture path is the `onManualWrite` hook, `MainComponent.cpp:2009-2015`); OSC/MIDI/REST writes go through `manualWrite(compScalarPath("signal"))` and ARE recorded automatically (D6 below). |
| D5 | Step 0 keeps `UniversalParamControl::SourceMode` (it still drives the meter-row paint) and only retires the MODEL WRITES of the two tick loops. `SourceMode` deletion, `RoutingEngine` deletion and TestServer `add_connection` endpoints stay named follow-ups. | Surgical: the picker already writes `*conn_` when bound (`UniversalParamControl.cpp:634-649`, VERIFIED) — binding the rows is what makes the old loops inert; deleting the enum is cleanup with a wider fence. |
| D6 | Recorder: zero code. `resolveControl` finds `"signal"` by key (`ManualWrite.cpp:17-23, 95-97`), `Program::resolveKey` treats every Comp-scope key as `ExactMatch` (`Program.cpp:98-99`), `manualWrite` -> `onManualWrite` -> `RecorderHost::onHumanWrite` (`MainComponent.cpp:2009-2015`). | "if cheap": it is free for every funnel writer. Gap (shared with masterOpacity/masterSpeed, NOT this lane): `capturePerfState` records clip scalars only (`PerfStateCapture.cpp:53-110`) — owned by the in-flight replay-restore lane (§4). |
| D7 | `RoutingEngine`: NO change. | Dead: `processFrame` has zero callers (`grep -rn "processFrame(" src | grep -i rout` -> declaration/definition only; `Renderer.cpp:251-254` comment; `addRoute` only from `TestServer.cpp:884`). Gate in §2.9 keeps it so. |

---

## 1. STEP 0 — effect-parameter and source-parameter twins render-live

Own lane/worktree (`lane/0925-ms-step0`), own commit set, merged BEFORE Step 1 starts.

### 1.1 Why (VERIFIED at 6d55fc9)
- `Clip.h:90 effParam()` / `:91 effDryWet()` have ZERO callers. The compositor uploads raw
  `slot.paramValues[p]` (`CompositorEngine.cpp:376`, also `:278-279` frame_delay, `:1475-1478`
  Screen Split) and raw `slot.dryWet` (`:384, :409`); the source-param upload reads raw `cp.value`
  (`Renderer.cpp:1034` layer_router, `:1070` setParamValue).
- The rendered modulation for effect/source sliders is the UI tick writing the RAW registry/macro
  value into the manual field: `EffectStackView.cpp:197` (`fx.paramValues[p] = signalValue`) and
  `ClipInspector.cpp:893` (`clip_->sourceParams[i].value = val`). Consequences: no manual value
  survives to blend toward (Step 1 needs one); RANGE/INVERT/curve on effect params are ignored
  (the loop bypasses `ConnectionShaper`); BPM Sync / Clip Position picks on effect rows are dead
  (the loop handles only Signal/Oscillator/Envelope/Macro modes, `:161-190`); only the clip/
  layer/comp currently bound to an inspector is modulated at all (the loop walks `rows_`).
- Engine gap: the source-param tick SKIPS the store on NaN (`ConnectionEngine.cpp:333-337`), so a
  gripped/disabled source-param twin freezes at its last signal value (critic-#1 class; the
  scalar/effect tickers already always-store, `:198-218, :232-244`). `onSourceParamsPublished`
  is never assigned (`ConnectionEngine.h:75`; grep: no assignment).
- R-A hazard (lane-3 plan §9): `tickEffectVector`'s lazy `resizeParams` (`ConnectionEngine.cpp:
  230`, message thread, unfenced; also `ManualWrite.cpp:63`) reallocates `paramLive` — harmless
  today because nothing on the GL thread reads it; a use-after-free the moment `effParam` is
  repointed UNLESS every creation site sizes the parallel arrays first. Creation sites (VERIFIED):
  `MainComponent.cpp:1030-1036, 1074-1080, 1147-1152`, `EffectStackView.cpp:522-527`,
  `TestServer.cpp:1087-1100`. The three `fromVar` sites already size (`Clip.cpp:285`,
  `Layer.cpp:247`, `Composition.h:461`).
- Bound-pointer hazard (NEW, found this pass): `UniversalParamControl::~UniversalParamControl`
  dereferences `conn_` (`UniversalParamControl.cpp:117-123`) and `bindConnection` dereferences the
  OLD `conn_` (`:127-128`). Effect rows will bind into `fx.paramConns[p]` (vector elements); the
  delete button erases the element BEFORE `rebuildRows()` destroys the rows (`EffectStackView.cpp:
  351-353`), a drop `push_back`s (may reallocate) before rebuild (`:527, :539`), and undo assigns
  the whole vector before `setEffects` (`MainComponent.cpp:4562` comment). Without a
  "forget-without-dereference" primitive, Step 0 would introduce a UAF in the destructor.

### 1.2 Files and exact edits
| File | Edit |
|---|---|
| `src/model/Clip.h` (EffectSlot, beside `:81-91`) | Add `void addParam(float v) { paramValues.push_back(v); paramConns.emplace_back(); paramLive.emplace_back(); }` (comment: the ONLY way to append a param; keeps `resizeParams` a production no-op so it never reallocates under the GL read). Make `effParam` self-guarding: `return i < paramLive.size() ? paramLive[i].effective(paramValues[i]) : paramValues[i];`. |
| `src/MainComponent.cpp:1035, :1079, :1150` | `slot.paramValues.push_back(p.defaultValue)` -> `slot.addParam(p.defaultValue)`. |
| `src/ui/EffectStackView.cpp:525` | same -> `slot.addParam(p.defaultValue)`. |
| `src/test/TestServer.cpp:1100` | same (test build only; keep consistent). |
| `src/render/CompositorEngine.cpp` | `:278-279` -> `slot.effParam(0)` / `slot.effParam(1)` (keep the size checks); `:376` -> `glUniform1f(loc, slot.effParam(p))`; `:384` -> `const float dryWet = slot.effDryWet(); if (dryWet < 0.999f)` and `:409` -> `glUniform1f(dwLoc, dryWet)`; `:1475-1478` -> `slot.effParam(0..3)`. |
| `src/render/Renderer.cpp:1034, :1070` | `cp.value` -> `cp.live.effective(cp.value)`. |
| `src/render/Renderer.h/.cpp` (beside `:97-101`) | Add `void updateActiveSourceParamsFor(const std::string& sourceType, const std::vector<Clip::SourceParam>& params)`: lock `activeSourceMutex_`; `if (!hasActiveSource_ \|\| activeSourceType_ != sourceType) return; activeSourceParams_ = params;`. Needed because the standalone-source path renders a COPY taken at select time (`:80-86, :209-213, :534-540`; callers `MainComponent.cpp:793, 4373, 4433` — the deck-view SELECT handler, so it is production-reachable whenever the deck composite yields no texture). |
| `src/connect/ConnectionEngine.cpp:327-341` | Always store: `if (!sp.conn.isConnected()) continue; if (!sp.conn.enabled) { sp.live.v.store(kNan); continue; } float y = evaluate(...); sp.live.v.store(std::isnan(y) ? kNan : y); anyDriven = true;` then `if (anyDriven && onSourceParamsPublished) onSourceParamsPublished(&clip);`. Same rule as `:198-218`. |
| `src/MainComponent.cpp` (constructor, beside `:1524-1527`) | `connectionEngine_.onSourceParamsPublished = [this](const Clip* c) { if (c) previewPanel_.getRenderer().updateActiveSourceParamsFor(c->sourceType, c->sourceParams); };` (120 Hz vector copy under the mutex while a source-param connection is live AND the type matches — same cost class as today's `onSourceParamsChanged` path, `:1524-1527`). |
| `src/ui/UniversalParamControl.h/.cpp` | Add `void forgetConnection() { conn_ = nullptr; live_ = nullptr; }` (drops the binding WITHOUT dereferencing — for owners about to destroy/reallocate the bound element) and the test seam `const ParamConnection* boundConnection() const { return conn_; }`. |
| `src/ui/EffectStackView.h/.cpp` | `rebuildRows()` (`:284-296`): before `removeChildComponent`/`rows_.clear()`, call `forgetConnection()` on every row's `dryWetControl` and `paramControls`. Row build (`:364-369`): `dwc->bindConnection(&fx.dryWetConn, &fx.dryWetLive)`; (`:411-413`): `if (p < fx.paramConns.size()) pc->bindConnection(&fx.paramConns[p], &fx.paramLive[p]);` — do NOT call `resizeParams` here (unfenced message-thread reallocation; sizes match by construction after `addParam`). `tickModulation()` (`:137-236`) becomes DISPLAY-ONLY: for each connected `pc`: `const float v = fx.effParam(p);` epsilon-gate vs `lastPushedValue`; `pc.setParamValue(v); pc.setSourceValue(v);` — DELETE the model write `:197` and the dead `onParamChanged` notify `:230-231` (zero consumers outside this file, VERIFIED grep). Also push `row.dryWetControl->setParamValue(fx.effDryWet())` when its conn is connected. `refresh()` (`:255-261`): display `fx.effDryWet()` / `fx.effParam(p)` instead of the raw fields. Test seams: `UniversalParamControl* paramControlForTest(int fx, int p)`, `UniversalParamControl* dryWetControlForTest(int fx)`. |
| `src/ui/ClipInspector.cpp` | `buildSourceParamControls()` (`:781-812`): `forgetConnection()` on every existing control before `sourceParamControls_.clear()`; after `pc->setSignalRegistry(...)`: `pc->bindConnection(&sp.conn, &sp.live);`. `setClip(nullptr)` branch (`:756-758`): forget before clear. `tickModulation()` (`:838-935`): DISPLAY-ONLY — `const float v = clip_->sourceParams[i].live.effective(clip_->sourceParams[i].value);` epsilon-gate; `pc.setParamValue(v); pc.setSourceValue(v);` — DELETE the model write `:893` and the `onSourceParamsChanged(clip_)` call inside the tick (`:925-926`; the renderer copy is now refreshed by the engine callback). Keep the HAND write path `:803-807` untouched. `refresh()` (`:942-947`): display `sp.live.effective(sp.value)`. |
| `tests/test_connection.cpp` | +4 cases (§1.3). |
| `tests/test_effect_stack_binding.cpp` (NEW) + `tests/CMakeLists.txt` (append at END) | Headless JUCE widgets under `ScopedJuceInitialiser_GUI`, same harness as `tests/test_right_click_reset.cpp:1-40`; source list = `test_right_click_reset`'s (`tests/CMakeLists.txt:1212-1226`) + `${SRC_DIR}/ui/EffectStackView.cpp` + `${SRC_DIR}/effects/EffectLibrary.cpp` (+ `core/UndoManager.cpp` if `EffectScope` pulls it — ASSUMED; add only "the ONE .cpp that defines a symbol the linker named", per the note at `:1257-1261`). |
| `.harmony/probe-mastersignal.sh` (NEW, part A) | §1.5. |
| `CLAUDE.md` | §3. |

Behaviour at 100% with a DEFAULT-shaped connection is bit-identical to today's raw write for raw
values in [0,1] (VERIFIED chain: `ConnectionShaper::shapeValue` = `clamp((raw-0)/1)` -> `linear`
(= `std::clamp`, returns `x` in range, `CurveTransforms.h:13-15`) -> `0 + c*1` = c; `applySmoothing`
returns `y` when `smoothingMs <= 0`, `ConnectionShaper.cpp:93-95`; no grip/glide). A raw value
outside [0,1] is now clamped where it used to be uploaded raw — a shader-side correction, not a
regression.

### 1.3 RED tests (fail-first on 6d55fc9; the RED commit adds the seams/stubs so they compile)
| id | file | case | RED proof on HEAD |
|---|---|---|---|
| S0-T1 | `tests/test_connection.cpp` | "tick: a gripped source-param connection publishes NaN (never a frozen last value)": `clip.sourceParams[0].conn` <- Lfo SawUp 1 beat; tick at bn=0.5 -> `live == 0.5`; `gripHeld()`; tick -> `std::isnan(live)`; `release`; tick -> value again. | Store skipped on NaN (`ConnectionEngine.cpp:333-337`) -> live stays 0.5 -> FAIL. |
| S0-T2 | same | "tick: a disabled source-param connection clears its twin to NaN" (`enabled=false` after one publishing tick). | same class -> FAIL. |
| S0-T3 | same | "EffectSlot::addParam keeps paramValues/paramConns/paramLive in lock-step" (3 pushes -> all sizes 3; `effParam(2)` reads the manual). | RED commit ships `addParam` as a stub that pushes `paramValues` only (the repo's RED convention, cf. `507ebc7`) -> sizes 3/0/0 -> FAIL. |
| S0-T4 | same | "effParam(i) with paramLive shorter than paramValues returns the manual value (never indexes past the end)": `paramValues={0.3}`, no resize; `REQUIRE(fx.effParam(0) == 0.3f)`. | HEAD indexes an empty vector -> ASan abort under `apply_sanitizers` -> FAIL. |
| S0-T5a | `tests/test_effect_stack_binding.cpp` | "setEffects binds every param row and the dry/wet row to the slot's connection": clip with one "Ripple" slot (params via `addParam`), `view.setEffects(&clip.effects)`, `REQUIRE(view.paramControlForTest(0,p)->boundConnection() == &clip.effects[0].paramConns[p])` for all p, and dry/wet likewise. | seams return `nullptr` on HEAD (rows never bound, `EffectStackView.cpp:398-425`) -> FAIL. |
| S0-T5b | same | "tickModulation never writes the manual field and the thumb shows effParam": connect `paramConns[0]` (Lfo) in the model, `paramLive[0].v = 0.9f`, `paramValues[0] = 0.2f`; `view.tickModulation()`; `REQUIRE(paramValues[0] == 0.2f)`; `REQUIRE(paramControlForTest(0,0)->getParamValue() == Approx(0.9f))`. | HEAD: control not connected -> no push -> display stays 0.2 -> FAIL (the write assertion passes on HEAD; the display one fails). |
| S0-T5c | same | PIN (passes on HEAD, must stay green under ASan): "rebuilding rows after the effects vector was cleared/reallocated touches no freed memory": bind, `onDragStart()` on param 0 (Held), then `clip.effects.clear(); view.setEffects(&clip.effects);` and `clip.effects` regrown + `setEffects` again. | Pin for the forget-before-rebuild rule (§1.1 last bullet). |

Grep gates (review-time, all must hold on the GREEN tree):
```
grep -c 'slot\.paramValues\[' src/render/CompositorEngine.cpp            # 0
grep -c 'cp\.value' src/render/Renderer.cpp                                # 0
grep -n 'paramValues\[p\] = \|sourceParams\[i\]\.value = ' src/ui/EffectStackView.cpp src/ui/ClipInspector.cpp   # empty (the hand writes at ESV:421 / CI:805 use capturedParam/idx and stay)
grep -rn 'paramValues\.push_back' src | grep -v 'model/Clip\.\|model/Layer\.cpp\|model/Composition\.h'   # empty
grep -n 'onSourceParamsPublished' src/MainComponent.cpp                    # exactly 1 assignment
```

### 1.4 What Boris sees after Step 0 (state in the commit message)
- Nothing changes for a slider connected with the default RANGE 0..1, no invert, linear
  (bit-identical, §1.2 last paragraph).
- Fixes he can notice: RANGE/INVERT/curve on an effect or source parameter now actually render
  (they were ignored); "BPM Sync" and "Clip Position" picks on effect rows now work (they were
  dead); connected effect params on clips NOT currently shown in the inspector now move (they
  were frozen); the hand value survives a connection (disconnect or right-click reset returns to
  it instead of to the last signal sample). Thumbs still follow the signal at 120 Hz.

### 1.5 Live gate — `.harmony/probe-mastersignal.sh` part A (Harmony runs; builder never)
Same launch/teardown/screen-safety skeleton as `.harmony/probe-lane3.sh` (production `open`,
7070 only, `set_bpm 120`, `load_composition`, `trigger_clip 0/0`, `render_frame` md5, Output-
window count 0, gated `pkill`). Fixtures are written by the script into `$OUT` with the absolute
image path: `$ROOT/media/P16_01_baseline.png` (VERIFIED present; `resources/default_image.png` named by CLAUDE.md does NOT exist — `resources/` holds only `projectm_presets`).
Image clip = `"mediaType": 1, "mediaFile": "<abs>"`; effect-param connection JSON = the exact
`Clip::toVar` shape (`Clip.cpp:98-128`): `"effects":[{"name":"Hue Shift","enabled":true,
"bypassed":false,"dryWet":1.0,"params":[0.5],"conns":[{"p":0,"src":{...},"shape":{...},
"enabled":true}]}]` with `src`/`shape` copied from `.harmony/probe-lane3-clip.json` (kind
"lfo", shape "sine", `cycleBeats` 1.0). `hue_shift` declares only `u_texture` + `u_hue_shift`
(`EmbeddedShaders.h:269-275`, VERIFIED) — no `u_time`, so a frozen param = a static frame.
- A1 `fx-live`: Hue Shift param0 <- 1-beat sine, RANGE 0..1. Two `render_frame`s 0.5 s apart
  DIFFER. Pre-change binary: a JSON-loaded effect-param connection is not rendered at all
  (nothing reads the twins; the UI loop needs `SourceMode`) -> identical -> this IS the fail-first.
- A2 `fx-range`: same with `min`=`max`=0.5 -> two frames IDENTICAL (RANGE honoured; a build that
  ignores the shape animates here).

### 1.6 Commit set (lane/0925-ms-step0)
c0 `test(s-rta-0925 ms-step0): RED -- effect/source-param twins are render-dead` (tests + seams +
stubs; S0-T1/2/3/4/5a/5b failing, 5c passing) -> c1 `fix(s-rta-0925 ms-step0): effect/source-param
connections render through their twins; rows bound; ticks display-only; addParam sizing` (all
green, gates hold) -> c2 `docs+probe`. Full `ctest` baseline: RE-RUN on the merged tree (466 at
`bbac78a` per the work index), never inherit.

---

## 2. STEP 1 — Master Signal

Own lane (`lane/0925-ms-step1`) branched from main AFTER Step 0 merged.

### 2.1 Model + persistence
| File | Edit |
|---|---|
| `src/connect/ScalarParams.h:35` | `enum class CompScalar : uint8_t { Opacity, Speed, PosX, PosY, Scale, Rotation, AnchorX, AnchorY, Signal, Count };` (appended before `Count`). `compScalarDefs()` (`:109-118`): append `{ "signal", ScalarMath::identity, ScalarMath::identity, 1.0f },  // -> masterSignal`. Fix the "N is at most 8" comment in `ManualWrite.cpp:14-15` (now 9). No other `CompScalar::Count`-sized structure exists outside `Composition.h` (VERIFIED grep of src+tests). |
| `src/model/Composition.h` | `:41` add `float masterSignal = 1.0f;  // Master Signal depth (s-rta-0925): 1 = signals move controls fully, 0 = every signal-connected control sits at its hand value`. `initDefault` (`:156`): `masterSignal = 1.0f;`. `toVar` (after `:223`): `obj->setProperty("masterSignal", static_cast<double>(masterSignal));`. `fromVar` (after `:341`): `if (obj->hasProperty("masterSignal")) masterSignal = static_cast<float>(static_cast<double>(obj->getProperty("masterSignal")));` — GUARDED (do not copy `masterOpacity`'s unguarded read at `:331`; an absent key must load 1.0). `manualRef` (`:516-532`): `case CompScalar::Signal: return c.masterSignal;`. The sparse `"conns"` map covers a connection on it automatically (`ConnSerialization.h:43-73`, `Composition.h:312-314`). |

### 2.2 Engine, macros, v1 mappings
| File | Edit |
|---|---|
| `src/features/SignalDepth.h` (NEW, header-only, no JUCE) | `inline float applyDepth(float manual, float driven, float depth) { if (depth >= 1.0f) return driven; if (depth <= 0.0f) return manual; return manual + depth * (driven - manual); }` — the `>= 1`/`<= 0` guards are load-bearing (100% must be bit-for-bit; out-of-range OSC/REST values degrade to 1/0). |
| `src/connect/ConnectionEngine.h:33-44` | Append to `Context` (LAST field, so every existing 7-value aggregate init in tests stays valid): `float signalDepth = 1.0f;  // Master Signal (s-rta-0925); 1 = today`. |
| `src/connect/ConnectionEngine.cpp` | `evaluate()` after the glide block, before `return y` (`:176-177`): `if (c.source.kind != ConnSource::Kind::Macro) y = applyDepth(manualNorm, y, ctx.signalDepth);` (comment: applied ONCE per chain where the SIGNAL enters; a Macro-sourced hop is exempt because the macro's own connection already took it, or it is a hand knob that must keep working at 0%; after smoothing and the glide so the fader never re-shapes the EMA). `tickScalars` (`:198-218`): add a trailing `size_t fullDepthIndex = SIZE_MAX` parameter; `Context full = ctx; full.signalDepth = 1.0f;` once before the loop; `evaluate(c, manualNorm, i == fullDepthIndex ? full : ctx, clock)`. Comp call (`:301-302`) passes `static_cast<size_t>(CompScalar::Signal)` — the ONE exemption (a Signal fader driven by a macro/LFO must not feed back on itself). Macro loop (`:287-299`): unchanged — it goes through `evaluate`, so a signal-driven macro takes the depth there. |
| `src/routing/MacroBank.h:74-87` | `void updateValues(const SignalRegistry& signals, float signalDepth = 1.0f)`; signal branch: `macro.currentValue = applyDepth(macro.manualValue, signals.getCachedValue(macro.sourceSignalId), signalDepth);` (`#include "features/SignalDepth.h"`). This is the UI-reachable macro drive (`MacroPanel.cpp:136,144` write `sourceSignalId`; `Macro::conn` is JSON/REST-only today). |
| `src/mapping/MappingEngine.h:49/.cpp:144-245` | `void processFrame(const FeatureSnapshot&, EffectChain&, float signalDepth = 1.0f);` final single store (`:242-243`): `auto& param = effect->getParam(...); param.value = applyDepth(param.defaultValue, std::clamp(sum, 0.0f, 1.0f), signalDepth);` (v1 mappings have no manual field; `EffectParam::defaultValue` at `Effect.h:13`). Still exactly one store. |
| `src/routing/RoutingEngine.*` | none (D7). |

### 2.3 The tick (critic BLOCKING) — `src/MainComponent.cpp:3307-3365`
```cpp
void MainComponent::tickFeaturePipeline()
{
    const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();
    // Master Signal (s-rta-0925): ONE read per tick, hoisted ABOVE all three consumers
    // (v1 MappingEngine, MacroBank, ConnectionEngine::Context) -- critic finding 1. When the
    // Signal scalar is itself connected this is the previous tick's twin (8 ms lag, accepted).
    const float signalDepth = composition_.eff(CompScalar::Signal);
    signalRegistry_.evaluateAll(snap);
    previewPanel_.getMappingEngine().processFrame(snap, previewPanel_.getEffectChain(), signalDepth);
    ...
    globalMacroBank_.updateValues(signalRegistry_, signalDepth);
    ... (recorder tick unchanged) ...
    ConnectionEngine::Context ctx{ signalRegistry_, globalMacroBank_, snap, dt, now,
                                   composition_.gripHoldMs, composition_.handBackGlideMs, signalDepth };
    connectionEngine_.tick(composition_, ctx);
    if (inspectorPanel_) inspectorPanel_->tickModulation();
}
```
Review-time order gate (must print HOIST-OK):
```
awk '/void MainComponent::tickFeaturePipeline/{f=1} f&&/eff\(CompScalar::Signal\)/{d=NR} f&&/getMappingEngine\(\)\.processFrame/{m=NR} f&&/updateValues\(signalRegistry_/{k=NR} f&&/connectionEngine_\.tick/{e=NR; exit} END{exit !(d && d<m && m<k && k<e)}' src/MainComponent.cpp && echo HOIST-OK
```

### 2.4 TopBar fader — copy of main's Master fader (`src/ui/TopBar.*`)
- `TopBar.h`: beside `:41-48` add `ResettableSlider& getMasterSignalSlider() { return masterSignalSlider_; }` and `void syncMasterSignalFromComposition();`; beside `:102-105` add `juce::Label masterSignalLabel_{"", "Signal:"}; ResettableSlider masterSignalSlider_; bool signalDragging_ = false;`; test seams `juce::Rectangle<int> masterSignalLabelBoundsForTest() const` and `juce::Rectangle<int> fadeSliderBoundsForTest() const` (layout test).
- `TopBar.cpp` constructor, directly after the master block (`:224`): label font/colour exactly as `masterLabel_` (`:194-197`); slider `setRange(0.0, 1.0, 0.01)`, `setValue(1.0, dontSendNotification)`, `setDefaultValue(1.0)`, `LinearHorizontal`, `NoTextBox`, `setTooltip("Master Signal: how strongly audio, oscillators and every other signal move the controls they are connected to. 100% = full; 0% = everything sits at its hand-set value.")`; wiring verbatim from `:211-224` with `masterOpacity` -> `masterSignal`, `CompScalar::Opacity` -> `CompScalar::Signal`, `masterDragging_` -> `signalDragging_`.
- `timerCallback` (`:262`): add `syncMasterSignalFromComposition();`. Body = `:322-331` with the Signal scalar (`connected ? def.toNorm(eff(Signal)) : composition_.masterSignal`, skipped while `signalDragging_`).
- `resized()` after `:570`: `rightSection.removeFromRight(4); masterSignalSlider_.setBounds(rightSection.removeFromRight(70)); masterSignalLabel_.setBounds(rightSection.removeFromRight(42));` => `... Fade | Signal: [==] | Master: [==] | Output: [...] | FPS DSP`.
- Width budget (VERIFIED arithmetic of `resized()` `:476-571`): the left-flowing block consumes 1175 px (1239 with Manual BPM on, `:529-533`); the right block needs 364 today, 480 after (+116). The window opens at the primary display's `userArea` (`src/Main.cpp:65-70`); the rig is 3456x2234 Retina, so ~1728 pt wide (INFERRED — builder confirms with `system_profiler SPDisplaysDataType | grep "UI Looks like"`). 1655/1719 <= 1728 fits, barely. If the measured width is smaller, shrink in this order: `displaySelector_` 100->90 (`:565`), `fadeSlider_` 100->80 (`:555`), `quantizeSelector_` 100->90 (`:550`), `manualModeBtn_` 80->70 (`:523`). (The design's "64 px slack at :516" is WRONG: that reserve hosts `trackerStateLabel_`, `:513-515`.) Note the bar already overflows at the 1280 px resize minimum (`Main.cpp:53`) today — pre-existing, not this lane's.

### 2.5 Bindability
| Surface | Edit (VERIFIED anchors) |
|---|---|
| Keyboard / MIDI note / CC incl. relative encoders | `src/binding/Binding.h:24-45`: append `MasterSignal` AFTER `ToggleRecording` (add the missing trailing comma) — the action is int-serialised (`BindingManager.cpp:228, 272`); inserting mid-enum re-maps every saved binding. Relative CC needs no getter: `BindingManager` accumulates per (channel,cc) (`BindingManager.cpp:168-185`). |
| Bind-mode / MIDI-learn target | `MainComponent.cpp:6436-6437`: after the "Master Opacity" push: `gx += gw + gap; targets.push_back({ { gx, topY, gw, gh }, "Master Signal", Binding::Action::MasterSignal, 0, 0, 0, 0, 0 });` (`BindingOverlay::BindableTarget`, `BindingOverlay.h:33-43`). No other table enumerates `Action` (VERIFIED grep `MasterOpacity`/`ToggleRecording`: only `Binding.h` + `MainComponent.cpp`). |
| Handler | after `MainComponent.cpp:6712`: `case Binding::Action::MasterSignal: manualWrite(compScalarPath("signal"), value, GripKind::Decaying, Origin::Human); break;` |
| OSC | `OscHandler.h:13-24` doc + `std::function<void(float depth)> onSetMasterSignal;`; `OscHandler.cpp` after the `/audiodna/master` block (`:137-143`): `if (address == "/audiodna/signal") { if (onSetMasterSignal) onSetMasterSignal(value); return; }`; `MainComponent.cpp` beside `:2057-2061`: `oscHandler_.onSetMasterSignal = [this](float depth) { manualWrite(compScalarPath("signal"), depth, GripKind::Decaying, Origin::Human); };` |
| REST write | `ApiServer.h` beside `:82`: `std::function<void(float depth)> onSetMasterSignal;` + `handleSetMasterSignal`; route after `:178`: `server_.Post("/api/set_master_signal", ...)`; handler = `handleSetLayerOpacity`'s shape (`:525-565`): parse `value` (error on missing), `callAsync` -> `onSetMasterSignal(v)`, `jsonOk()`. `MainComponent.cpp` beside the `apiServer_->onSetLayerOpacity` wiring (`~:1911`): `apiServer_->onSetMasterSignal = [this](float depth) { manualWrite(compScalarPath("signal"), depth, GripKind::Decaying, Origin::Human); };` |
| REST read | `ApiServer.cpp:316`: add `obj->setProperty("masterSignal", static_cast<double>(composition_.masterSignal));` (`live.signal` + `connected` come free from `addLiveBlock` `:318-320`). `:289`: add `obj->setProperty("masterSignal", static_cast<double>(composition_.eff(CompScalar::Signal)));` beside `masterLevel`. |
| Recorder | D6 — zero code. |
| Composition-inspector twin knob | NOT built (Boris asked for the top-right fader only; the Composition tab just went through the visualfix lane). Consequence: the picker cannot CONNECT the Signal scalar from the UI (only JSON); MIDI/OSC/REST cover the "APC fader drives it" case without a connection. Named follow-up: `bind(signalControl_, CompScalar::Signal)` in `CompositionInspector::bindScalarControls` (`:401-416`). |

Values are not clamped in the funnel (parity with `onSetMaster`, `:2057-2061`); `applyDepth`'s
guards make >1 behave as 1 and <0 as 0; the fader displays the clamped value.

### 2.6 RT-safety / threading
- Audio callback, analysis thread, FeatureBus: untouched. `CombinedCallback::inputGain` (Gain) untouched (Q1).
- GL threads: read NOTHING new (D1). `CompositorEngine`/`Renderer`/`ProceduralSource`/`OutputRenderer` never see the depth.
- Message thread: the one `eff(CompScalar::Signal)` read and every writer (TopBar, funnel) are on it (`manualWrite` asserts it, `MainComponent.cpp:3284`).
- HTTP thread: `composition_.masterSignal` / `eff(Signal)` read for JSON — the same plain-float/relaxed-atomic "known-deferred crossing" `masterOpacity` uses at `ApiServer.cpp:289, 316`.
- No allocation added to any tick: `applyDepth` is 2 flops per connection.

### 2.7 RED tests
| id | file / target | case | RED proof (against the c3 scaffold: enum+field+`Context::signalDepth`+stub `applyDepth` returning `driven`+defaulted params+unwired TopBar widgets) |
|---|---|---|---|
| S1-T1 | `tests/test_signal_depth.cpp` (NEW header-only target; copy `test_per_type_autopilot_layout`'s 5-line CMake block, `tests/CMakeLists.txt:1201-1205`) | `applyDepth`: d=1 returns `driven` by `==` for `m=0.1f, y=0.7f` (also assert `m + 1.0f*(y-m) != y` documents why the guard exists, or drop if it happens to be equal); d=0 returns `manual` by `==`; d=0.5 midpoint; d=1.5 -> driven; d=-0.5 -> manual. | stub returns `driven` -> d=0 case FAILS. |
| S1-T2 | `tests/test_connection.cpp` | "evaluate: depth blends toward manualNorm after shaping, for every non-Macro kind": SECTIONs Signal (registry "Volume" <- `snap.rms=0.8` after `sig.evaluateAll(snap)`), Lfo, Envelope(Beats), ClipPosition (stub `ClipClock` returning 0.7), each with shape `outMin=0.2,outMax=0.8,inverted=true`, `manualNorm=0.35`: d=0 -> `== 0.35f` exactly; d=1 -> `==` the value from an identical connection evaluated with a default ctx (bit-identical); d=0.5 -> `Approx(0.35 + 0.5*(full-0.35))`. Macro SECTION: `Kind::Macro` with `bank.getMacro(0).currentValue=0.9` -> d=0 returns `shape(0.9)`, NOT manualNorm (D3). | d=0 sections FAIL. |
| S1-T3 | same | "evaluate: depth is applied AFTER the hand-back glide": two identical connections mid-glide (`gripHeld`; tick; `release`; advance `now` by 60 ms with `handBackGlideMs=120`); A at d=1 -> `glided`; B at d=0.5 -> `== applyDepth(manualNorm, glided, 0.5)`. | FAIL. |
| S1-T4 | same | "evaluate: a gripped connection returns NaN at every depth (0, 0.5, 1)". | passes on scaffold — PIN. |
| S1-T5 | same | "tick: CompScalar::Signal's own connection is evaluated at full depth while Opacity (same Lfo) scales": both scalars <- the same 4-beat SawUp Lfo; `ctx.signalDepth=0`; `masterOpacity=1.0`, `masterSignal=1.0`: opacity twin `== 1.0f` (toModel(toNorm(1.0)) is exact for identity), signal twin `== Approx(0.125)`. | FAIL (no exemption). |
| S1-T6 | same | "tick: a macro chain applies depth once (D3)": macro0.conn <- Lfo (L=0.125, `manualValue=0.5`); `scalarConns[Opacity]` <- Macro 0 (identity shape), `masterOpacity=1.0`. d=0: `macro.currentValue == 0.5f`, opacity twin `== 0.5f` (follows the static knob). d=0.5: `macro.currentValue == Approx(0.3125)`, opacity twin `== Approx(0.3125)` (single application; the critic's compounding would give `1 + 0.5*(0.3125-1) = 0.65625`). Hand macro (no conn, `manualValue=0.7`) at d=0: opacity twin `== 0.7f`. | FAIL. |
| S1-T7 | same (links `MacroBank.h`) | `MacroBank::updateValues(signals, d)` with `sourceSignalId` = "Volume": d=0 -> `manualValue`; d=1 -> the cached signal by `==`. | FAIL. |
| S1-T8 | `tests/test_mapping_engine.cpp` | `processFrame(snap, chain, 0.0f)` -> param `== defaultValue`; `processFrame(snap, chain, 1.0f)` bit-identical to the 2-arg call on a fresh engine (pins the guard). | FAIL. |
| S1-T9 | `tests/test_composition.cpp` (`:137` roundtrip, `:474` backcompat) | round-trips `masterSignal=0.35`; an old object without the key loads `1.0`; `initDefault()` after `masterSignal=0.2` gives `1.0`. | FAIL (no toVar/fromVar). |
| S1-T10 | `tests/test_manual_write.cpp` (`:88`, `:206`) | `resolveControl(comp, bank, compScalar("signal"))->manual == &comp.masterSignal` and `->conn == &comp.scalarConns[Signal]`; `manualWriteCore` at 0.4 lands; a Lane write is refused under a HumanHeld grip. | passes on scaffold — PIN. |
| S1-T11 | `tests/test_master_signal_link.cpp` (NEW; CMake = `test_master_opacity_link`'s block `tests/CMakeLists.txt:1251-1298` with the new file) | the six cases of `tests/test_master_opacity_link.cpp:35-143` on `getMasterSignalSlider()` / `masterSignal` / `CompScalar::Signal`; + "the Signal and Master faders are independent" (write 0.3 on one, the other stays 1.0); + LAYOUT: `bar.setSize(W, H)` with W = the measured launch width (1728 INFERRED) and H = the app's top-bar height (`grep -n "topBar_->setBounds" src/MainComponent.cpp`): `sig.getWidth()==70`, `sig.getRight() <= master.getX()`, `!sig.intersects(master)`, `masterSignalLabelBoundsForTest().getX() >= fadeSliderBoundsForTest().getRight()`. | fader unwired/stub sync -> write/grip/sync/reset cases FAIL; layout FAILS until `resized()` places it. |

### 2.8 Grep gates (review-time)
```
grep -rn 'masterSignal\|CompScalar::Signal\|signalDepth\|applyDepth\|SignalDepth.h' src/render src/sources src/effects src/analysis src/audio src/ui/OutputWindow.cpp   # empty (Q2: no GL/uniform scaling)
grep -c 'applyDepth(' src/connect/ConnectionEngine.cpp     # 1
grep -c 'eff(CompScalar::Signal)' src/MainComponent.cpp    # 1  (+ the awk HOIST-OK gate in 2.3)
grep -rn 'processFrame(' src | grep -i 'rout'              # declaration/definition only (D7)
grep -n 'MasterSignal' src/binding/Binding.h               # 1 line, the LAST enumerator
```

### 2.9 Live gate — `.harmony/probe-mastersignal.sh` part B (Harmony runs)
Fixtures (script-written, absolute image path):
- B1 `ms-conn`: image clip with `"conns": {"scale": <2-beat sine, RANGE 0.3..0.7>}` (the exact
  object from `.harmony/probe-lane3-clip.json`) + Hue Shift param0 <- 1-beat sine RANGE 0..1.
  `POST /api/set_master_signal {"value":0}` -> (i) 10 samples of `d['decks'][0]['layers'][0]
  ['clips'][0]['live']['scale']` all `== 1.0` +/-1e-4 and `'scale' in ...['connected']`; (ii) two
  `render_frame`s 0.5 s apart md5-IDENTICAL (static image, hue frozen at its hand value). Then
  `{"value":1}` -> live.scale spans (min < 0.9, max > 1.1 over 20 samples @50 ms) and the two
  frames DIFFER. Then `{"value":0.5}` -> the live.scale swing is strictly smaller than at 1.
  `GET /api/composition` -> `masterSignal` and `live.signal` echo the last value.
- B2 `ms-pulse`: image clip + effect "Beat Ripple" (default params, no connections;
  `beat_ripple` declares `u_beatPhase`, `EmbeddedShaders.h:10361`; `set_bpm 120` runs the beat
  clock without audio). At `set_master_signal 0`: two frames 0.25 s apart DIFFER (keeps pulsing —
  Q2). If Beat Ripple's default params do not visibly move (ASSUMED they do), use a `plasma`
  source clip (`EmbeddedShaders.h:2670`, also declares `u_beatPhase`).
- B3 OSC (no dependency, python3 stdlib):
  `python3 -c "import socket,struct;a=b'/audiodna/signal\0';a+=b'\0'*((4-len(a)%4)%4);socket.socket(socket.AF_INET,socket.SOCK_DGRAM).sendto(a+b',f\0\0'+struct.pack('>f',0.3),('127.0.0.1',8000))"`
  -> within 1 s `GET /api/composition` `masterSignal == 0.3` (+/-0.01). The TopBar sync-back to
  0.3 is pinned headlessly (S1-T11) and, if the visual-gate workflow runs, by a window-only
  screenshot (never the output window).
- Pre-change binary: `set_master_signal` returns 404 and `live.signal` is absent -> B1/B3 FAIL =
  fail-first. Teardown identical to `probe-lane3.sh` (Output-window count 0, gated `pkill`).

### 2.10 What Boris sees after Step 1
A new "Signal:" fader immediately left of "Master:" at the top right, default 100%, right-click
resets to 100%, saved with the composition (old files open at 100%). At 100% nothing changes. As
it goes down, every slider connected to audio, an oscillator, an envelope, BPM Sync or clip
position moves less and at 0% sits still exactly where his hand left it (or where a MIDI/OSC
writer put it); a macro knob driven by audio does the same; a macro knob he turns by hand still
works. Meters keep dancing; clips keep playing; autopilot keeps sequencing; effects and sources
that read the beat clock or the audio directly keep pulsing (his Q2 answer). OSC `/audiodna/
signal`, REST `/api/set_master_signal`, a keyboard/MIDI binding ("Master Signal" in bind mode,
relative encoders included) all move it and the fader follows.

### 2.11 Commit set (lane/0925-ms-step1)
c3 `test(RED)`: scaffold (§2.7 header) + all tests; expected failing S1-T1(d=0)/T2/T3/T5/T6/T7/
T8/T9/T11, pins T4/T10 passing -> c4 `feat(GREEN) engine+model+persistence+tick hoist` (T1-T10
green) -> c5 `feat(GREEN) TopBar+Binding+OSC+REST` (T11 green, gates 2.8 + HOIST-OK) -> c6
`docs+probe`. Harmony runs `probe-mastersignal.sh` A+B on the merged tree and once on the pre-
Step-0 binary (A1, B1, B3 must FAIL there).

---

## 3. Docs to update (c2 / c6)
- `CLAUDE.md`: Mapping System — one sentence: "Master Signal (`Composition::masterSignal`,
  `CompScalar::Signal`) scales the reach of every signal->parameter connection where the signal
  enters (`ConnectionEngine::evaluate`, non-Macro sources; `MacroBank::updateValues`; v1
  `MappingEngine`); 1.0 = bit-identical to no fader, 0.0 = hand values." Audio Uniform System —
  "Master Signal does NOT scale these uniforms (Boris 2026-09-25 Q2)." P22 REST list (`:1151`):
  23 endpoints + `/api/set_master_signal`; OSC (`:1153`): 12 patterns + `/audiodna/signal`;
  `:238` Binding.h 20 actions; `:260/:262` counts. Common Pitfalls +32: "Effect/source-param rows
  are engine-driven: a UI tick never writes `paramValues`/`sourceParams[].value`; append params
  only via `EffectSlot::addParam` (parallel arrays sized at creation — the lazy `resizeParams`
  must stay a no-op or the GL `effParam` read is a use-after-free); forget bindings before a
  structural edit of the effects vector."
- `.harmony/APP-INVENTORY.md:30, :139, :149 (+row), :169, :212` counts (12 OSC, 23 REST, 20
  actions).
- `.harmony/binding-decisions.md`: nothing new (Q1-Q3 already recorded); D3's macro semantics go
  in the notebook as an implementation decision to restate to Boris (§5 R1).

## 4. Fence, collisions, order
- Step 0 files: `src/model/Clip.h`, `src/connect/ConnectionEngine.cpp`, `src/render/
  CompositorEngine.cpp`, `src/render/Renderer.{h,cpp}`, `src/ui/{EffectStackView,ClipInspector,
  UniversalParamControl}.{h,cpp}`, `src/MainComponent.cpp` (3 `addParam` sites + 1 callback),
  `src/test/TestServer.cpp`, tests. Step 1 files: `src/features/SignalDepth.h`, `src/connect/
  {ScalarParams.h,ConnectionEngine.h,ConnectionEngine.cpp,ManualWrite.cpp(comment)}`,
  `src/model/Composition.h`, `src/routing/MacroBank.h`, `src/mapping/MappingEngine.{h,cpp}`,
  `src/MainComponent.cpp`, `src/ui/TopBar.{h,cpp}`, `src/api/ApiServer.{h,cpp}`, `src/osc/
  OscHandler.{h,cpp}`, `src/binding/Binding.h`, tests, probe, docs.
- In-flight `lane/0925-replay-restore` (b187781) touches `MainComponent.cpp` hunks at 1944-1981,
  4079-4130, 4955-5000, 5187-5480 and `tests/CMakeLists.txt` at :734 (VERIFIED `git diff --stat`
  / hunk headers). This plan's `MainComponent.cpp` edits (≈1035-1150, 1524, 1911, 2057, 3307-
  3365, 6436, 6712) and CMake appends AT THE END do not overlap. Order anyway: merge replay-restore
  first if it is ready; Step 0 does not depend on it. Do NOT touch `PerfStateCapture.cpp`
  (theirs; D6 gap is a named follow-up for that lane: comp scalars opacity/speed/signal in the
  clip-scalar style, `PerfStateCapture.cpp:96-102`).
- Step 1 must branch from main AFTER Step 0 merged (it relies on `effParam` being read and the
  manual field surviving).
- No Xcode on the rig (binding-decisions 2026-09-25): Command Line Tools build only; own `-B` dir
  per lane; SCREEN-SAFETY law for the probe (never the output window).

## 5. RISKS — and the strongest counterargument
- R1 (D3, product semantics). Strongest counterargument: the task's literal invariant "0% => EVERY
  connected control equals its manual value" — under D3 a control linked to a HAND-turned macro
  sits at `shape(macroManual)`, not at its own hand value. Why D3 wins: Boris scoped the fader to
  "signals" (Q2 verbatim: "this control is only for signals"); a dashboard/macro knob is a hand,
  and killing hand fan-out at 0% would break the APC-style performance pattern the app is built
  for; every AUDIO/LFO/envelope-driven thing — including a signal-driven macro knob — does sit at
  its hand value, which is what he will test. Restate to Boris in plain words ("at 0%, knobs you
  turn by hand still work; only the music and the oscillators stop moving things"). If he wants
  the literal reading: two lines — drop the `!= Kind::Macro` guard in `evaluate` and give the
  macro loop a `full` context (the critic's option (a)); S1-T6's expectations flip.
- R2 Step 0 changes rendering for existing connected effect/source params with a non-default
  RANGE/INVERT, and starts modulating connected params on non-inspected clips (both fixes; §1.4).
  Verify with a saved composition that has effect-param `conns` before/after; state in the commit.
- R3 R-A: the lazy `resizeParams` (`ConnectionEngine.cpp:230`, `ManualWrite.cpp:63`) is still an
  unfenced message-thread reallocation if ANY creation site is missed; the `push_back` grep gate
  and `addParam` close the five known sites; `effParam`'s self-guard makes a missed site read the
  manual value instead of crashing, but a concurrent reallocation would still be a UAF — keep the
  gate green.
- R4 forget-before-rebuild trades the R-C mid-drag release for effect/source rows: a row destroyed
  mid-drag (clip re-triggered by MIDI while dragging an effect slider) leaves that param's Held grip
  until the next touch of that slider. Named; strictly better than the UAF it prevents. Pre-
  existing sibling hazard (not this lane): `bindConnection(nullptr,nullptr)` on a scalar control
  after its Clip was destroyed (`ClipInspector.cpp:766-770`).
- R5 TopBar width: 9-73 px of margin at 1728 pt (INFERRED width); the layout test at the measured
  width plus the shrink list in §2.4 is the control. The bar already overflows at the 1280 px
  resize minimum today.
- R6 Standalone-source refresh copies the params vector at 120 Hz while a source-param connection
  is live on a clip whose type matches the active standalone source (strings + atomics, message
  thread). Same class as today's `onSourceParamsChanged` path; acceptable. Two clips of the same
  `sourceType` both refresh it (last writer wins) — fallback path only.
- R7 100% bit-identity rests on two guards: `applyDepth`'s `>= 1` (S1-T1/T2/T8 pin it) and
  `shapeValue`'s identity for default shapes (Step 0, §1.2 last paragraph).
- R8 A show saved at 0% boots looking non-reactive — the fader shows it (Boris chose "save it").
- R9 Not done here (named): Composition-tab Signal knob (§2.5 last row); checkpoint0 comp scalars
  (D6); `SourceMode`/`RoutingEngine` deletion; TestServer `add_connection`; `POST /api/
  set_master_opacity` (the `16b133d` deferral — identical shape to `set_master_signal`, 10 lines,
  fold in if Harmony wants both).

## 6. Confidence
VERIFIED this pass (read/grep at 6d55fc9): tick order; one-master fader pattern; render-dead
`effParam`/`cp.value` sites; UI tick model writes; source-param NaN skip; picker writes `*conn_`
when bound; destructor/`bindConnection` dereference; creation sites; `linear()`/`shapeValue`/
`applySmoothing` identity; `Binding` int serialisation and the two tables; relative-CC
accumulation; OSC/REST anchors; `resolveKey` Comp scope; recorder hook site; replay-restore
hunks; window sizing; `hue_shift`/`beat_ripple`/`plasma` uniform declarations. INFERRED: launch
width 1728 pt; Beat Ripple visibly moves at defaults. ASSUMED (builder verifies): headless link
closure for `test_effect_stack_binding`; top-bar height
constant.

## 7. Summary for Harmony (10 lines)
1. Two lanes, strictly ordered: STEP 0 (`lane/0925-ms-step0`, effect/source twins render-live, nothing visible changes at 100%) then STEP 1 (`lane/0925-ms-step1`, Master Signal) branched from main after Step 0 merges; replay-restore does not collide (hunks verified) but merge it first if ready.
2. Boris Q2 deletes the design's snapshot/uniform scaling: Step 1 touches NO GL-thread code; a grep gate keeps `src/render`, `src/sources`, `src/effects`, `OutputWindow.cpp` depth-free.
3. Critic BLOCKING folded: ONE `composition_.eff(CompScalar::Signal)` read hoisted to the top of `tickFeaturePipeline` (above `processFrame :3318` and `updateValues :3328`), passed to MappingEngine, MacroBank and `Context.signalDepth`; an awk HOIST-OK gate pins the order.
4. Critic MEDIUM folded by construction: depth applied ONCE where a signal enters (`evaluate` for Signal/Lfo/Envelope/ClipPosition, `Kind::Macro` hops exempt; the macro's own conn and `MacroBank::updateValues(signals, d)` take it). Consequence to restate to Boris: hand-turned macro knobs keep working at 0% (R1 has the two-line flip).
5. Step 0 = repoint `CompositorEngine.cpp:278-279,376,384,409,1475-1478` + `Renderer.cpp:1034,1070`; always-store source twins + wire `onSourceParamsPublished` -> `Renderer::updateActiveSourceParamsFor`; bind effect/dry-wet/source rows via `bindConnection`; both `tickModulation`s display-only; `EffectSlot::addParam` at the 5 creation sites (R-A); `forgetConnection()` before every row rebuild (new UAF found in `~UniversalParamControl`).
6. Step 1 = `CompScalar::Signal` (before `Count`) / `masterSignal` (default 1.0, guarded `fromVar`, `initDefault`, `manualRef`); `SignalDepth.h` `applyDepth` with the `>=1`/`<=0` guards; Signal's own connection evaluated at full depth (self-feedback exemption); v1 `processFrame(snap, chain, d)` anchored to `defaultValue`; RoutingEngine untouched (dead).
7. Fader = main's Master pattern verbatim (`TopBar.cpp:211-224, 318-331`): direct model write + `scalarConns[Signal]` grip + 15 Hz sync; "Signal:" label + 70 px slider left of Master (+116 px; fits the ~1728 pt launch width — builder measures; shrink list given).
8. Bindability: `Binding::Action::MasterSignal` appended LAST (int-serialised), "Master Signal" bind target + handler, OSC `/audiodna/signal`, REST `POST /api/set_master_signal` + `masterSignal`/`live.signal` reads — all through `manualWrite(compScalarPath("signal"))`, so the recorder's continuous lane is automatic (zero code); TopBar drags are not recorded, same as Master.
9. RED tests: Step 0 — S0-T1..T5 (source-twin NaN store, `addParam` lock-step, `effParam` guard, rows bound, tick never writes the model) + ASan pin; Step 1 — S1-T1..T11 (`applyDepth` identities, per-kind depth incl. Macro exemption, depth after glide, self-exemption, single application on macro chains, MacroBank/MappingEngine depth, persistence round-trip + absent->1.0, funnel resolve, six fader cases + layout).
10. Live gate `.harmony/probe-mastersignal.sh` (Harmony runs, probe-lane3 skeleton): A1 effect-param LFO renders / A2 RANGE honoured; B1 `set_master_signal 0` -> `live.scale == 1.0` + identical frames, 1 -> varies + differ, 0.5 -> smaller swing; B2 Beat Ripple keeps pulsing at 0; B3 OSC `/audiodna/signal 0.3` echoes in `/api/composition`. A1/B1/B3 must FAIL on the pre-change binary.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0925/plan-mastersignal.md
STATUS: DONE
