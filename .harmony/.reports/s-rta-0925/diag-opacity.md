# diag-opacity — Master opacity: Video twin removal, top-right fader link, right-click reset

Session: s-rta-0925 · Author: Architect (Fable) · Date: 2026-09-25
Ruling source: `.harmony/binding-decisions.md:523-533` (Boris: remove Video "Opacity" twin, keep "Master";
top-right fader is a SHORTCUT to the Master knob, must be linked both ways; right-click reset fails on ALL
Composition-tab sliders). Scope of this report: computed task items (1) and (2) plus the right-click bug
diagnosis (in Boris's verbatim request). The "Master Signal" slider is a separate design item — pointer only.

Confidence labels: VERIFIED = read in source this session at the cited line; INFERRED = follows from cited
code but not executed; ASSUMED = not checked, named as such.

---

QUESTION: (1) Remove the Composition inspector's Video-section "Opacity" twin, keep "Master". (2) Diagnose
why the top-right master fader and the Composition "Master" knob are not linked; map every writer/reader of
master opacity; spec the fix so REST/OSC/MIDI/UI all reflect in both. (3) Why right-click reset does nothing.

APPROACH (answer first):
- (2) ROOT CAUSE: they are two DIFFERENT variables applied as two multiplies in series, not one value with a
  broken sync. The top-right fader writes `Renderer::masterLevel_` (an atomic float outside the model,
  `src/render/Renderer.h:343-344,401`); the Master knob writes `Composition::masterOpacity`
  (`src/model/Composition.h:40`) through the connect/grip system. `Renderer::renderOpenGL` dims the frame
  by `masterLevel_` at `src/render/Renderer.cpp:677-704` and then AGAIN by `eff(CompScalar::Opacity)` at
  `:726-745`. Both at 0.5 = 25% output. No code path reads one into the other (VERIFIED, full table below).
  FIX: retire `masterLevel_`; make the TopBar fader a second widget-grip view of `CompScalar::Opacity`,
  exactly the pattern `LayerStrip::opacitySlider_` already uses for layer opacity (`src/ui/LayerStrip.cpp:
  414-436`), with a timer read-back mirroring `CompositionInspector::syncFromComposition`
  (`src/ui/CompositionInspector.cpp:538-544`). Every OSC/MIDI/replay writer already lands on
  `masterOpacity`, so they reflect in both for free once the fader is a view of the same field.
- (1) The Video section holds ONLY the twin; delete the whole section (7 sites, listed).
- (3) Three distinct right-click defects, all in `src/ui/UniversalParamControl.h/.cpp` + inspector setup:
  UPC knobs never armed with a default (dead click), IncDec/text-box sliders never receive the click at all,
  and the thumb-path reset never touches the connection grip.

---

## 1. Every writer and reader of the two "master" values (VERIFIED unless marked)

### 1a. `Composition::masterOpacity` — the model master (connect-system scalar `CompScalar::Opacity`)

Definition: `src/model/Composition.h:40`; `manualRef(CompScalar::Opacity)` → `&c.masterOpacity`
`:520`; `eff()` = `scalarLive[Opacity].effective(masterOpacity)` `:72-74` (NaN twin → manual field,
`src/connect/LiveValue.h:35-39`). ScalarDef: `src/connect/ScalarParams.h:110` (`"opacity"`, identity,
defaultNorm 1.0). Persisted: `Composition.h:214` (toVar) / `:331` (fromVar); `initDefault` `:156`.

WRITERS
| # | Path | Site | Grip semantics |
|---|------|------|----------------|
| W1 | Composition inspector "Master" knob | `src/ui/CompositionInspector.cpp:135-140` (`onValueChanged` → `composition_->masterOpacity = val`) | Widget grip: `UniversalParamControl.cpp:26-27` gripHeld on drag-start / release on drag-end; bound `:414` |
| W2 | Composition inspector Video "Opacity" twin (TO REMOVE) | `CompositionInspector.cpp:150-162`, bound `:414-415`, synced `:550` | same as W1 |
| W3 | OSC `/audiodna/master` | `src/osc/OscHandler.cpp:137-141` → `src/MainComponent.cpp:2070-2074` `manualWrite(compScalarPath("opacity"), level, Decaying, Human)` | Funnel: `MainComponent.cpp:3298-3304` → `src/connect/ManualWrite.cpp:180-189` writes `*r.manual` (= `&comp.masterOpacity`, proven by `tests/test_manual_write.cpp:94-100`) |
| W4 | Keyboard/MIDI binding `Binding::Action::MasterOpacity` | `MainComponent.cpp:6727-6730` (same funnel); bind-overlay target row `:6454-6455` | Decaying, Human |
| W5 | Recorder replay (continuous lane) | `MainComponent.cpp:2005-2006` `manualWrite(k, v, Held, Replay)` | Hand::Lane (`:166-170`) — any human hand wins |
| W6 | TestServer (build-gated, port 8080) `POST /api/set_composition_params` | `src/test/TestServer.cpp:214-215`, `:1356-1359` direct field write | NO grip, bypasses funnel. Its comment `:1348-1354` ("read back nowhere on the GL thread") is STALE — `Renderer.cpp:726` reads it on the GL thread |
| W7 | Composition file load / initDefault | `Composition.h:331` / `:156` | model load |

READERS
| Reader | Site |
|--------|------|
| Renderer (GL thread), unconditional dim of final frame | `src/render/Renderer.cpp:726-745` (`eff(CompScalar::Opacity)` → `glBlendColor`) — runs before recording/Syphon/capture, so it dims what leaves the app |
| Production REST `GET /api/composition` | `src/api/ApiServer.cpp:314` (+ live block `:318`) |
| TestServer `GET /api/composition_params` | `TestServer.cpp:1377,1385` |
| Composition inspector sync (~10 Hz) | `CompositionInspector.cpp:549-550`; cadence `MainComponent.cpp:3415-3417` (30 Hz timer `:417`, every 3rd tick) |
| Connect engine tick (publishes twin) | `src/connect/ConnectionEngine.cpp:199-218,301`; gripped → NaN `:84-91`; hand-back glide `:155-173` |

### 1b. `Renderer::masterLevel_` — the top-right fader's value (OUTSIDE the model)

Definition: `src/render/Renderer.h:401` `std::atomic<float> masterLevel_{1.0f}`; API `:343-344`.

WRITERS
| # | Path | Site |
|---|------|------|
| L1 | TopBar fader `masterLevelSlider_` (`src/ui/TopBar.h:95`; setup `TopBar.cpp:197-202`, `setDefaultValue(1.0)` `:200`; layout `:531`) | `MainComponent.cpp:629-632` `onValueChange` → `renderer.setMasterLevel(v)` |
| L2 | Hidden legacy v1 slider `MainComponent::masterLevelSlider_` (`MainComponent.h:272-273`; setup `.cpp:524-531`; hidden `:2427-2428`) | `.cpp:530` still wired; `loadDeck()` `:3702` calls `setValue(deck.masterVideoLevel, sendNotificationSync)` → changes `masterLevel_` WITHOUT the TopBar fader moving (ghost writer). v1 deck preset field `src/ui/PresetManager.h:87`, `.cpp:506,557`; `saveDeck()` `:3606` |
| L3 | TestServer reset | `TestServer.cpp:658` → 1.0 |

READERS: `Renderer.cpp:677` (dim, skipped when >= 0.99); REST `GET /api/status.masterLevel`
`ApiServer.cpp:287`; `GET /api/state.master_level` `:1136`; TestServer state `:595`.

NOT: persisted in composition files; bindable; OSC-addressable; REST-writable; connectable to a signal;
recorded/replayed. It resets to 1.0 on every boot. TopBar has no read-back — `TopBar::timerCallback`
(`TopBar.cpp:224-240`) only refreshes BPM/beat-wheel; `getMasterLevelSlider()` has exactly one caller
(`MainComponent.cpp:629`), a writer.

### 1c. Consequences (VERIFIED from 1a/1b)
- Two independent dims compound (`Renderer.cpp:677` then `:726`): fader 0.5 + knob 0.5 = 25%.
- Nothing syncs either direction. OSC/MIDI/binding writes move the knob only; the fader stays put.
- REST readback exposes TWO different numbers under master-ish names: `/api/status.masterLevel`
  (fader) vs `/api/composition.masterOpacity` (knob).
- Production REST (port 7070) has NO writer for master opacity at all: `src/api/ApiServer.h:59-114`
  callbacks are per-layer/per-clip (`onSetLayerOpacity :82`, `onSetClipEffectParam :83`); `/api/set_param`
  (`ApiServer.cpp:419-...`) targets effect params only. "REST must reflect in both" is today
  read-only; a writer must be ADDED if wanted (§3 step 5).
- The fullscreen output window applies NEITHER master: `OutputRenderer::renderOpenGL`
  (`src/ui/OutputWindow.cpp:67-172`) renders `texMgr_` image + `effectChain_` only — no deck composite,
  no masterLevel_, no masterOpacity (grep: zero hits). Pre-existing output-window-arc limitation; the fix
  below does not change what reaches a projector. Named in RISKS.

---

## 2. Item (1) — remove the Video "Opacity" twin, keep "Master" (spec)

The Video section contains nothing but the twin (VERIFIED): paint `CompositionInspector.cpp:257-258`,
layout `:359-362`, height `:481`. Remove the whole section.

Builder edits (`src/ui/CompositionInspector.h/.cpp` only, plus two doc lines):
1. `.h:101-102` — delete `// --- Video ---` + `UniversalParamControl opacityControl_;`.
2. `.h:122-125` comment (bindScalarControls) — drop the "masterControl_ and opacityControl_ share" sentence.
3. `.cpp:150-162` — delete the `// --- Video Opacity ---` block (setup + `onValueChanged` + `onExpandToggled`
   + `addAndMakeVisible`).
4. `.cpp:257-258` — delete the "Video" `paintSectionHeader` + its `y +=` line.
5. `.cpp:359-362` — delete the `// Video section` layout block.
6. `.cpp:414-415` — delete `bind(opacityControl_, CompScalar::Opacity);` (keep the `bind(masterControl_...)`).
7. `.cpp:481` — delete the `// Video` height line in `getPreferredHeight()`.
8. `.cpp:545-550` — delete the ruling-11 twin comment and `syncScalar(opacityControl_, ...)`; keep `:549`.
9. Docs: `.harmony/APP-INVENTORY.md:74` "Composition master/speed; Video opacity;" → drop "Video opacity".
   `CLAUDE.md` needs no edit (its "Video section (Opacity, Width, Height...)" line describes the LAYER
   inspector).
Done = builds clean; Composition tab shows Composition{Master, Speed} directly followed by Transform;
`ctest` unchanged (no test references `opacityControl_`; `tests/test_composition.cpp:21,142,179` are
model-only). Right-click on the remaining Master knob is covered by §4.

---

## 3. Item (2) — link the top-right fader to the Master knob (spec)

DECISION: one value. `Composition::masterOpacity` via `CompScalar::Opacity` is the master; the TopBar
fader becomes a second widget-grip view of it; `Renderer::masterLevel_` and the legacy v1 slider are
deleted. Rationale + rejected options in TRADEOFFS.

Step 1 — TopBar becomes a widget-grip writer (pattern: `src/ui/LayerStrip.cpp:414-436`).
- `src/ui/TopBar.cpp` after `:202`: add `onDragStart` → `composition_.scalarConns[size_t(CompScalar::Opacity)]
  .gripHeld(); masterDragging_ = true;`; `onDragEnd` → `...release(connNow()); masterDragging_ = false;`;
  `onValueChange` → `composition_.masterOpacity = (float) masterLevelSlider_.getValue();`. TopBar already
  holds `Composition& composition_` (`TopBar.h:47`); include `connect/ConnClock.h` for `connNow()` (as
  LayerStrip does) and `connect/ScalarParams.h` for `compScalarDefs()`.
- Add `bool masterDragging_ = false;` to `TopBar.h` private members (near `:95`).
- Delete `MainComponent.cpp:628-632` (the `getMasterLevelSlider().onValueChange` wiring — it would
  otherwise overwrite TopBar's lambda). `TopBar.h:39` `getMasterLevelSlider()` accessor then has no
  caller: delete it too.
- Same-thread safety: slider callbacks, grips and the model write all run on the message thread — the
  same shape as W1 (INFERRED from LayerStrip precedent + `manualWrite`'s message-thread assert
  `MainComponent.cpp:3299`).

Step 2 — read-back so the fader follows the knob, OSC, MIDI, replay and signals.
- In `TopBar::timerCallback` (`TopBar.cpp:224`; rate = TopBar's `startTimerHz` call in its ctor — any
  rate >= 10 Hz matches the inspector's 10 Hz sync), add, guarded by `!masterDragging_`:
  `const auto& def = compScalarDefs()[size_t(CompScalar::Opacity)];`
  `bool connected = composition_.scalarConns[...].isConnected();`
  `float shown = connected ? def.toNorm(composition_.eff(CompScalar::Opacity)) : composition_.masterOpacity;`
  `masterLevelSlider_.setValue(shown, juce::dontSendNotification);`
  This is `syncFromComposition`'s `syncScalar` (`CompositionInspector.cpp:538-544`) verbatim in
  semantics: a connected master shows the signal-driven eff(); during a Held grip the engine publishes NaN
  (`ConnectionEngine.cpp:84-91`) so eff() == manual and the thumb never fights the hand; after release
  the hand-back glide (`:155-173`) is visible on both controls identically.
- The knob → fader direction needs nothing else: the knob writes the same field (W1) and the fader
  polls it. The fader → knob direction is the inspector's existing 10 Hz sync (`:549`).

Step 3 — delete the second multiplier.
- `src/render/Renderer.cpp:676-704` — delete the `masterLevel_` dim block (the `:726-745` block already
  dims by `eff(Opacity)` unconditionally per S167-L4b; net effect one fewer quad draw).
- `src/render/Renderer.h:343-344` (`setMasterLevel/getMasterLevel`) and `:401` (`masterLevel_`) — delete.
- Readers, keep JSON keys for compatibility but source them from the model:
  `src/api/ApiServer.cpp:287` `masterLevel` → `composition_.eff(CompScalar::Opacity)`;
  `:1136` `master_level` → same; `src/test/TestServer.cpp:595` `master_level` → same;
  `TestServer.cpp:656-658` "Reset master level" → `composition_.masterOpacity = 1.0f;` (it already
  writes that field at `:1358`). Update `TestServer.cpp:1348-1354`'s stale comment while there.
  Include `connect/ScalarParams.h` where `CompScalar` is not yet visible (ApiServer.cpp already uses
  `compScalarDefs()` at `:318` — VERIFIED it is visible there).

Step 4 — delete the ghost legacy slider.
- `src/MainComponent.h:272-273` (`masterLevelLabel_`, `masterLevelSlider_`); `MainComponent.cpp:327`
  (label text), `:523-532` (setup), `:2427-2428` (hide), — delete.
- `saveDeck()` `:3606` → `deck.masterVideoLevel = composition_.masterOpacity;`
- `loadDeck()` `:3702` → `composition_.masterOpacity = deck.masterVideoLevel;` (a preset load is a model
  load, same as `Composition.h:331`; not a human grip). `PresetManager.h:87` field stays (file format).
  ASSUMED: the v1 deck Save/Load buttons (`:345-348`, laid out `:2461`) are still reachable in the v2
  layout — not verified; the edit is correct either way.

Step 5 — (OPTIONAL, decision for Harmony/Boris) production REST writer. Today none exists (§1c).
- `src/api/ApiServer.h` add `std::function<void(float opacity)> onSetMasterOpacity;` next to `:82`;
  register `POST /api/set_master_opacity` (`{"opacity": 0..1}`) mirroring `handleSetLayerOpacity`
  (`ApiServer.cpp:533-563`: parse, `callAsync`, fire callback, unconditional ok);
  `MainComponent.cpp` next to `:1927-1930`: `apiServer_->onSetMasterOpacity = [this](float v) {
  manualWrite(compScalarPath("opacity"), v, GripKind::Decaying, Origin::Human); };`.
  Endpoint count 24→25 (`APP-INVENTORY.md` endpoint table + `CLAUDE.md` "22 endpoints" text already lag
  the inventory's own 24; touch both). If skipped, say so in the handoff: REST remains read-only for
  master.

Step 6 — docs: `CLAUDE.md` Output & Integration ("/api/status ... masterLevel" now = composition master);
`APP-INVENTORY.md:56` TopBar row ("Master slider" = composition master opacity, linked to the
Composition tab knob); `:144/:164` key semantics.

DONE criteria (live gate, app running; port 7070):
1. Drag TopBar fader to 0.30 → Composition tab "Master" reads 0.30 within ~100 ms;
   `curl -s :7070/api/composition | jq .masterOpacity` == 0.30 and `/api/status | jq .masterLevel` == 0.30.
2. Drag "Master" knob to 0.80 → TopBar fader shows 0.80; both REST keys 0.80.
3. OSC `/audiodna/master 0.5` (UDP 8000) → both controls 0.5 (Decaying grip; expires after
   `gripHoldMs` 250 ms — `Composition.h:81`).
4. MIDI/keyboard binding on "Master Opacity" → both controls follow.
5. Connect Master to a signal (triangle → Audio/RMS): BOTH thumbs follow the signal; dragging either
   one holds it (Held grip) and it glides back after release; the other thumb mirrors throughout.
6. Compounding gone: a single control at 0.5 yields ~50% brightness (previously fader 0.5 + knob 0.5 =
   25%). Optional PSNR check via `/api/render_frame` before/after.
7. Right-click the TopBar fader → 1.0 on both (`TopBar.cpp:200` default already armed).
8. `cmake --build build` clean; `ctest` all pass (no test touches `masterLevel_`; `tests/test_composition*.cpp`
   and `tests/test_manual_write.cpp:94-100` unchanged).

---

## 4. Item (3) — right-click reset does nothing (diagnosis + plan)

Boris: "ALL Composition-tab sliders fail" — consistent with three distinct defects (VERIFIED in code, not
run live):

R1. UPC knobs are never armed with a default. `ResettableSlider::mouseDown`
(`src/ui/UniversalParamControl.h:30-39`) on a right-click does `if (hasDefault_) setValue(default); return;`
— with no default it swallows the click (stock `juce::Slider::mouseDown`, `build/_deps/juce-src/modules/
juce_gui_basics/widgets/juce_Slider.cpp:852-873`, would otherwise start a drag, so the override makes the
click dead). Only `UniversalParamControl::setDefaultValue` (`.h:60`) arms the inner `valueSlider_`; the
class default is `defaultValue_ = 0.5f` (`.h:122`) but the slider's `hasDefault_` stays false.
`CompositionInspector.cpp` calls `setDefaultValue` at exactly two sites (`:54` apClipLoops, `:96` the three
cycle sliders); NONE of its 8 UPCs (`CompositionInspector.h:98-109`: master, speed, opacity[twin], posX,
posY, scale, rotation, anchor) is armed. Right-click on the knob's NAME area (`UniversalParamControl.cpp:
291-300`) does reset — to 0.5, wrong for Master (1.0) and Speed (0.25 norm = 1.0x). Contrast: LayerInspector
arms its master/opacity knobs (`src/ui/LayerInspector.cpp:147,184`).

R2. The four numeric sliders that ARE armed can never receive the click. `apClipLoopsSlider_`
(`CompositionInspector.cpp:50-51`) and the cycle sliders (`:92-93`) are `IncDecButtons` + `TextBoxLeft`: their
visible surface is the internal text-box Label and the +/- TextButtons, so a right-click lands on a child
component and `Slider::mouseDown` is never called (INFERRED from JUCE Slider structure; matches Boris's
"ALL").

R3. Thumb-path reset never touches the connection. `ResettableSlider` resets via `setValue(...,
sendNotificationSync)` only; the UPC name-area path additionally calls `conn_->gripTouch(connNow())`
(`UniversalParamControl.cpp:300`). On a CONNECTED parameter the thumb-path reset writes the manual field
while the signal keeps driving `eff()` — visually nothing happens. Minor: the name-area path fires
`onValueChanged` twice (`:296` via the slider's own `onValueChange` at `:18-22`, then `:298` explicitly).

MacroPanel (Dashboard) knobs are armed (`src/ui/MacroPanel.cpp:11` default 0.5, `Knob.cpp:10`) — INFERRED
they work; include in the gate anyway.

Coverage audit (grep of slider-type members in `.h` vs `setDefaultValue` calls in `.cpp`; INFERRED, needs a
control-by-control pass in the fix lane): ClipInspector 12/8, CompositionInspector 12/2, LayerInspector
24/7, SignalInspector 7/1, MappingEditor 5/1, MilkDropBrowser 2/0, TopBar 3/3, LayerStrip 4/4.

PLAN (one "rclick" lane, ~5 surgical edits):
1. `UniversalParamControl` ctor (`.cpp` near `:10`): `valueSlider_.setDefaultValue(defaultValue_);` so no UPC
   slider is ever dead (falls back to 0.5 until the owner sets the real default).
2. Correct defaults from the existing descriptor tables — no new constants:
   `CompositionInspector::bindScalarControls()` (`:405-421`) inside its `bind` lambda:
   `c.setDefaultValue(compScalarDefs()[size_t(s)].defaultNorm);` (`ScalarParams.h:108-117`: opacity 1.0,
   speed 0.25, others 0.5). Same one-liner is available to Layer/Clip inspectors via `layerScalarDefs()` /
   `clipScalarDefs()` — apply in the same lane if Harmony widens scope.
3. `ResettableSlider`: in its ctor `addMouseListener(this, true)` so right-clicks on the text-box/+/-
   children reach `mouseDown`; in `mouseDown`, if `e.eventComponent != this` handle ONLY the right-click
   reset and return (never forward child events to `juce::Slider::mouseDown`). INFERRED: JUCE's
   `Component::addMouseListener(listener, wantsEventsForAllNestedChildComponents=true)` delivers nested
   child events; Builder verifies on the IncDec sliders live. Fallback if the Label eats it: attach the
   listener to `getChildComponent(i)` explicitly.
4. `ResettableSlider` gains `std::function<void()> onResetToDefault;` fired after the reset;
   `UniversalParamControl` wires it to `if (conn_) conn_->gripTouch(connNow());` (fixes R3) and drop the
   duplicate explicit `onValueChanged` at `.cpp:298` (the `sendNotificationSync` at `:296` already fires it).
5. GATE (Boris's scope: every Composition-tab control): for each of 4 IncDec sliders, 7 UPCs (after the twin
   removal), 8 macro knobs — right-click → value == its default AND, for a connected UPC, the reset is
   visible (grip touched). No headless seam exists for JUCE mouse events in this repo's tests (ASSUMED);
   gate is a live checklist, screenshot-backed.

---

TRADEOFFS CONSIDERED
- A (chosen): fader = second view of `CompScalar::Opacity`; delete `masterLevel_`. One value, one dim, all
  external writers already land there, persistence/bind/OSC/replay/connect come for free, matches the
  LayerStrip↔LayerInspector precedent already in the codebase.
- B (rejected): keep both variables and cross-copy values. Still two multiplies → a linked 0.5 renders 25%;
  or copying one into the other every frame squares the dim. Wrong by construction.
- C (rejected): make the knob write `masterLevel_` and drop `masterOpacity`. Loses connect/grip, OSC,
  MIDI binding, replay, persistence and undoes s-rta-0923 lane 3's funnel work.
- D (rejected): route the fader through `MainComponent::manualWrite` instead of a widget grip. The funnel
  is for release-less external writers (Decaying rank); a dragged fader has a real release and must be a
  Held grip like every other slider (`ParamConnection.h:182-191`), else a MIDI turn could displace the hand
  mid-drag. Widget grip = the inspector's own behaviour = "shortcut" semantics exactly.
Strongest counterargument to A: `/api/status.masterLevel` changes meaning for any external consumer.
Why it loses: the fader was `masterLevel_`'s only live writer, both default to 1.0, and the key now reports
the one true master — a consumer that watched the fader keeps seeing the fader. Keys are kept.

RISKS
- Output window (projector) is dimmed by NEITHER master today and still will not be after this change
  (`OutputWindow.cpp:67-172` renders the v1 single-image path). If Boris expects the master fader to dim
  the projector, that is a separate output-window-arc item; say so before he tests on a second display.
- Skipping step 5 leaves production REST without a master writer — the computed task's "REST writes must
  reflect in both" is then vacuously true. Decide explicitly.
- R2's `addMouseListener` approach is INFERRED; if the internal Label consumes right-clicks first (it is
  created with `isReadOnly=false`), use the per-child fallback named in §4 step 3.
- LayerInspector has the SAME twin pattern on layer opacity ("Master" `LayerInspector.cpp:145-152` and
  "Opacity" `:182-185`, both `LayerScalar::Opacity` `:919,922`). Not in this ruling — ask Boris, do not act.
- Legacy v1 deck presets with `masterVideoLevel < 1` will now load into `masterOpacity` (persisted with
  the composition) instead of a boot-transient render value. Benign, but different.
- Removing `Renderer::setMasterLevel` breaks any out-of-tree caller; in-tree callers are exactly the three
  listed in §1b (VERIFIED by grep).
- Master Signal slider (Boris's new idea, `binding-decisions.md:528-533`): out of this dispatch. Pointer
  for that design lane: input gain is pre-analysis (`src/audio/AudioEngine.h:51` `setInputGain`, TopBar
  `inputGainSlider_`); `SignalRegistry.h` has no global depth/gain today (grep: none) — that lane adds a
  post-analysis scale in the connect/routing stage, never in `AnalysisThread`.

STATUS: COMPLETE — diagnosis + build-ready specs for items (1), (2) and the right-click plan; step 5 (REST writer) and the layer-level twin await a decision.
