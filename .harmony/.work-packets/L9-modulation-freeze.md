# L9 modulation-freeze work packet

## VERDICT: BUILDABLE
REAL defect, confirmed end-to-end from render read-site back to the exact
gating statement. NOT fixed by c51aff7 (that commit fixed a different,
parallel modulation system — `MappingEngine`/`EffectChain` — not this one).
This packet also surfaces two adjacent, independently-verified findings
(a sibling un-relocated freeze in `RoutingEngine`, and a real data race on
`SignalRegistry`) discovered while tracing the dependency chain. Recommend
the orchestrator decide whether those ride with L9 or spin off — see OUT OF
SCOPE.

## SIZE: small (primary fix) + medium (if the RoutingEngine sibling is folded in)
Primary fix (D1 below): ~4 files, ~40-60 new/moved lines, 0 files deleted-from
substantially. RoutingEngine relocation (D2): ~2 files, ~15 lines, mechanical
(exact mirror of what c51aff7 already did for MappingEngine). Neither requires
a model/serialization change or touches any existing function's signature —
see CALL-SITE ENUMERATION for why tests/ is unaffected.

## CURRENT BEHAVIOUR (verified) — what happens today, traced end to end

There are **three independent, non-overlapping modulation systems** in this
codebase. Confusing them was the trap in the essentials-plan wording
("mapped/modulated parameter values" is used loosely) — I re-derived which
is which from source rather than trusting the plan's framing:

| System | Data it writes to | Driver | Status |
|---|---|---|---|
| A. `MappingEngine` (audio feature → param) | `EffectChain`/`Effect` (previewPanel's live GL effect rack — `PreviewPanel::getEffectChain()`) | `MainComponent::tickFeaturePipeline()`, unconditional 120 Hz message-thread timer | **Fixed by c51aff7.** VERIFIED. |
| B. `RoutingEngine` (Signal → param, "P16" routes, Global scope only — Clip/Layer scope is an explicit TODO, unimplemented) | same `EffectChain` as A | **`Renderer::renderOpenGL()`**, the GL-thread render callback — sole caller | **NOT fixed. Same freeze class as A, left behind.** VERIFIED. (D2, adjacent finding) |
| C. `EffectStackView`'s per-param "connected" mechanism (Signal / Oscillator / Envelope / Macro → param) | `Clip::EffectSlot::paramValues` / `Layer::layerEffects` / `Composition::globalEffects` (the actual Composition/Deck model that drives the live VJ output via `CompositorEngine`) | `EffectStackView::refresh()`, called only when its owning Inspector is the InspectorPanel's **active tab** | **This is the plan's "connected param" claim. REAL, not fixed.** VERIFIED. (D1, the assigned target) |

### D1 — the assigned claim, CONFIRMED REAL

**Step 1 of the render side (runs continuously, independent of any UI panel):**
`CompositorEngine` reads `Clip::EffectSlot::paramValues[p]` directly, every
GL frame, and uploads it as a shader uniform:

```
src/render/CompositorEngine.cpp:242:    for (const auto& slot : clip.effects)
...
src/render/CompositorEngine.cpp:372:        for (size_t p = 0; p < def->params.size() && p < slot.paramValues.size(); ++p)
src/render/CompositorEngine.cpp:375:            auto loc = program->getUniformIDFromName(def->params[p].uniformName.c_str());
src/render/CompositorEngine.cpp:376:                glUniform1f(loc, slot.paramValues[p]);
```
VERIFIED by reading `applyFXOnlyLayer`'s call sites (CompositorEngine.cpp:736,
817) into the deck-render path — this is not a preview-only readout, it is
the actual mix output.

**Step 2 — the only place that *computes* a live connected value and writes
it into that same `paramValues[p]`:**

```
src/ui/EffectStackView.cpp:137: void EffectStackView::refresh()
src/ui/EffectStackView.cpp:157:     for (size_t p = 0; p < row.paramControls.size() && p < fx.paramValues.size(); ++p)
src/ui/EffectStackView.cpp:162:         if (pc.isConnected())          // sourceMode_ != Manual
src/ui/EffectStackView.cpp:179:             signalValue = signalRegistry_->getCachedValue(sig->getId());   // Signal/Oscillator/Envelope
src/ui/EffectStackView.cpp:195:             signalValue = macroBank_->getMacroValue(macroIdx);            // Macro/Link
src/ui/EffectStackView.cpp:203:             fx.paramValues[p] = signalValue;      // <-- THE WRITE the renderer later reads
src/ui/EffectStackView.cpp:213:     pc.setParamValue(fx.paramValues[p]);   // display-only, unconditional
```
This function is **not** a pure repaint — it computes and applies (STEP 1 of
the task's instructions answered: refresh() COMPUTES/APPLIES, it does not
merely repaint from model state).

Note also: `onParamChanged` (EffectStackView.h:65, invoked at
EffectStackView.cpp:207-208) is declared but **never assigned anywhere** —
confirmed with three independent grep patterns, see CALL-SITE ENUMERATION.
It is dead wiring; the actual effect on rendering comes entirely from the
direct model write at line 203, not from that callback.

**Step 3 — that compute+write only runs for whichever ONE chain is the
active InspectorPanel tab:**

```
src/ui/InspectorPanel.cpp:172: void InspectorPanel::refresh()
src/ui/InspectorPanel.cpp:173:     switch (activeTab_)
src/ui/InspectorPanel.cpp:174:         case Tab::Clip:        clipInspector_.refresh(); break;
src/ui/InspectorPanel.cpp:175:         case Tab::Layer:       layerInspector_.refresh(); break;
src/ui/InspectorPanel.cpp:176:         case Tab::Composition: compInspector_.refresh(); break;
src/ui/InspectorPanel.cpp:177:         case Tab::Signal:      signalInspector_.refresh(); break;
```
Exactly one branch fires. `clipInspector_`/`layerInspector_`/`compInspector_`
are plain members of `InspectorPanel` (InspectorPanel.h:89-91) — always
constructed, always alive, but their `refresh()` (and therefore their
embedded `EffectStackView::refresh()`) only runs while that tab is active:

```
src/ui/CompositionInspector.cpp:439:    effectStackView_.refresh();   // inside CompositionInspector::refresh()
src/ui/LayerInspector.cpp:775:     effectStackView_.refresh();       // inside LayerInspector::refresh() (guarded by `if (layer_)`)
src/ui/ClipInspector.cpp:822:      effectStackView_.refresh();       // inside ClipInspector::refresh() (guarded by `if (clip_)`)
```

**Step 4 — what drives `InspectorPanel::refresh()` at all:**

```
src/MainComponent.cpp:2583:    if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
src/MainComponent.cpp:2584:        inspectorPanel_->refresh();
```
This is inside `MainComponent::timerCallback()`, a 30 Hz UI timer, throttled
to ~10 Hz by the `% 3` guard — this is the ONLY periodic driver. The other
four call sites (MainComponent.cpp:3681, 4067, 4607, 4637) are one-shot,
event-triggered (undo/redo, load-preset, content-lock toggle) — none are a
substitute for continuous ticking.

**Net effect (VERIFIED, not inferred):** a "connected" (Signal / Oscillator /
Envelope / Macro) param on a Clip, Layer, or the global Composition effect
chain stops being recomputed the instant the InspectorPanel's active tab is
not the one that owns that chain — e.g. operator is on the Signal tab, or has
a different clip/layer selected than the one that's actually modulating on
screen. `CompositorEngine` keeps reading `paramValues[p]` every render frame
regardless, so the effect keeps rendering — frozen at its last computed
value, with no error, exactly matching "a 'connected' param that stops
modulating when unwatched."

### Was this already fixed by c51aff7? NO — checked directly.
`grep -rn "MappingEngine\|processFrame" src/ui/EffectStackView.cpp
src/ui/EffectStackView.h` → **zero hits** (see CALL-SITE ENUMERATION). System C
(EffectStackView/Clip::EffectSlot) is architecturally disjoint from System A
(MappingEngine/EffectChain) that c51aff7 touched — different data, different
driver, different files. The fix in c51aff7 cannot have touched this path.

### D2 — adjacent finding, same bug class as A, NOT relocated (RoutingEngine)
While tracing "where else does GL-callback dependence bite," found that
`Renderer::renderOpenGL()` — the exact GL-thread callback c51aff7's own
commit message says stops firing when the preview GL context detaches, and
that the same commit removed `mappingEngine_.processFrame()` from — still
contains:
```
src/render/Renderer.cpp:229:    const FeatureSnapshot snap = featureBus_.read();
src/render/Renderer.cpp:230:    if (signalRegistry_ != nullptr)
src/render/Renderer.cpp:231:        signalRegistry_->evaluateAll(snap);
src/render/Renderer.cpp:232:    if (signalRegistry_ != nullptr)
src/render/Renderer.cpp:236:        routingEngine_.processFrame(*signalRegistry_, [this](const Route& route, float value) {
src/render/Renderer.cpp:238:            if (route.targetScope == Route::TargetScope::Global)
src/render/Renderer.cpp:239:                if (auto* effect = effectChain_.getEffect(route.targetEffectIndex))
src/render/Renderer.cpp:240:                    effect->setParamValue(route.targetParamIndex, value);
```
`evaluateAll()` (line 231) turns out to be **accidentally already safe**: it
is *also* called, unconditionally, from `SignalBar::timerCallback()`
(src/ui/SignalBar.cpp:111-117, `startTimerHz(30)` at SignalBar.cpp:34), a
plain `juce::Timer` on the message thread that runs for the app's entire
life regardless of GL attach or SignalBar's own expand/minimize display
state — `signalBar_` is constructed once in `MainComponent`'s constructor
(MainComponent.cpp:575, inside `MainComponent::MainComponent(...)`,
MainComponent.cpp:13) and never torn down. So `SignalRegistry::cachedValues_`
gets refreshed at 30 Hz independent of the GL callback, in practice.

**But `routingEngine_.processFrame()` (line 236) has exactly one caller in
the whole repo** —
`grep -rn "routingEngine_\." src/ render/ test/` shows only
Renderer.cpp:236 for `.processFrame(`; every other hit is TestServer.cpp's
`addRoute`/`removeRoute`/`getRouteAt` (route *management*, not per-frame
apply). Nothing else ticks it. A Global-scope Route (Signal → previewPanel
EffectChain param) genuinely freezes when the preview GL context detaches —
the identical symptom c51aff7 fixed for MappingEngine, left un-relocated for
this sibling. `Renderer.h:118` already exposes `getRoutingEngine()` publicly,
same pattern as `getMappingEngine()` — the relocation is mechanical.

### Bonus finding (data race, NOT a freeze bug, flagging because it's adjacent and real)
`SignalRegistry::evaluateAll()` mutates `cachedValues_` (a plain
`std::vector<float>`, zero synchronization —
`grep -n "mutex\|jassert\|atomic" src/signal/SignalRegistry.h
src/signal/SignalRegistry.cpp` → zero hits). It is called from BOTH:
- the GL render thread (`Renderer::renderOpenGL()`, Renderer.cpp:231, live
  whenever the preview context is attached — i.e. the *common* case), and
- the message thread (`SignalBar::timerCallback()`, always running).

Concurrently, with no confinement guard of the kind c51aff7 added for
`MappingEngine` (A6: `jassert(...isThisTheMessageThread())` in
addMapping/removeMapping/clearAll/processFrame). This is a genuine
pre-existing data race, unrelated to the freeze claim, and NOT something
this lane should silently absorb — see OUT OF SCOPE.

## ROOT CAUSE (verified)
The compute-and-apply step for a "connected" effect param
(`EffectStackView.cpp:157-213`) is coupled to a UI-repaint code path
(`refresh()`) that only runs for the currently active `InspectorPanel` tab
(`InspectorPanel.cpp:172-179`), while the render path that consumes the
value it writes (`CompositorEngine.cpp:242-376`) runs continuously and
independently. The fix the essentials plan describes for `MappingEngine`
(decouple compute-and-apply from any UI/visibility gate; drive it from the
unconditional `tickFeaturePipeline()` seam) is the right shape for this too
— but note it is a **different code path in a different class**, not the
already-shipped MappingEngine relocation.

## FILES TOUCHED — exhaustive list, each with why

Primary fix (D1):
- `src/ui/EffectStackView.h` — declare a new method, e.g.
  `void tickModulation();` that does ONLY the compute+write (no display
  sync, no repaint) — the body currently at EffectStackView.cpp:156-210.
- `src/ui/EffectStackView.cpp` — extract that block out of `refresh()` into
  `tickModulation()`; have `refresh()` call `tickModulation()` first (so
  on-tab behavior is unchanged) then do the unconditional display-sync loop
  (line 213, `pc.setParamValue(...)`) and `repaint()`.
- `src/ui/ClipInspector.h`/`.cpp`, `src/ui/LayerInspector.h`/`.cpp`,
  `src/ui/CompositionInspector.h`/`.cpp` — each add a one-line pass-through
  `void tickModulation() { effectStackView_.tickModulation(); }` (guard
  Layer/Clip the same way their own `refresh()` already guards — `if
  (layer_)` / `if (clip_)` — Composition has no such guard today at
  CompositionInspector.cpp:439, matching existing asymmetry, not introducing
  a new one).
- `src/ui/InspectorPanel.h`/`.cpp` — add
  `void tickModulation() { clipInspector_.tickModulation();
  layerInspector_.tickModulation(); compInspector_.tickModulation(); }`
  called unconditionally, NOT gated by `activeTab_` (this is the actual
  fix — every chain ticks regardless of which tab is showing).
- `src/MainComponent.h`/`.cpp` — `tickFeaturePipeline()` gains one line:
  `if (inspectorPanel_) inspectorPanel_->tickModulation();` alongside the
  existing `MappingEngine::processFrame` call.

If D2 (RoutingEngine) is folded in:
- `src/render/Renderer.cpp` — delete the `routingEngine_.processFrame(...)`
  block (lines 232-241); leave `evaluateAll` where it is (already safe via
  SignalBar, and moving it doesn't fix the data race — see OUT OF SCOPE).
- `src/MainComponent.cpp`/`.h` — `tickFeaturePipeline()` gains the
  relocated `previewPanel_.getRenderer().getRoutingEngine().processFrame(...)`
  call, using the same `snap`-derived `signalRegistry_` MainComponent
  already owns (MainComponent.h:317) and already passes to the renderer
  (MainComponent.cpp:424).

## CALL-SITE ENUMERATION — the grep commands run + raw output

```
$ grep -n "processFrame\|MappingEngine" src/ui/EffectStackView.cpp src/ui/EffectStackView.h
(no output — zero hits, confirms System C is untouched by c51aff7)

$ grep -rnF "onParamChanged =" --include="*.cpp" --include="*.h" src/
$ grep -rnF "\.onParamChanged" --include="*.cpp" --include="*.h" src/
$ grep -rnF "effectStackView_.onParamChanged" --include="*.cpp" --include="*.h" src/
(all three: no output — onParamChanged is declared and invoked but never assigned)

$ grep -rnF "effectStackView_.refresh()" --include="*.cpp" --include="*.h" src/
src/ui/CompositionInspector.cpp:439:    effectStackView_.refresh();
src/ui/LayerInspector.cpp:775:    if (layer_) { syncFromLayer(); effectStackView_.refresh(); macroPanel_.refresh(); }
src/ui/ClipInspector.cpp:822:        effectStackView_.refresh();
(exactly 3 — matches the 3 chain types Clip/Layer/Composition, no fourth)

$ grep -rnF "clipInspector_.refresh()" --include="*.cpp" --include="*.h" src/
src/ui/InspectorPanel.cpp:174:        case Tab::Clip:        clipInspector_.refresh(); break;
$ grep -rnF "layerInspector_.refresh()" --include="*.cpp" --include="*.h" src/
src/ui/InspectorPanel.cpp:175:        case Tab::Layer:       layerInspector_.refresh(); break;
$ grep -rnF "compInspector_.refresh()" --include="*.cpp" --include="*.h" src/
src/ui/InspectorPanel.cpp:176:        case Tab::Composition: compInspector_.refresh(); break;
(exactly 1 each, all inside the same switch — confirms tab-exclusive gating)

$ grep -rnF "inspectorPanel_->refresh()" --include="*.cpp" --include="*.h" src/
src/MainComponent.cpp:2584: (10Hz timer, the only periodic driver)
src/MainComponent.cpp:3681, 4067, 4607, 4637: (one-shot, event-triggered — undo/redo, load preset, content-lock toggle)
(5 total call sites, 1 periodic + 4 one-shot; re-checked each of the 4 by reading surrounding code — none loop or re-fire on a timer)

$ grep -rn "routingEngine_\." --include="*.cpp" --include="*.h" src/
src/test/TestServer.cpp:828,841,855,857  (route management: addRoute/removeRoute/getRouteAt — not per-frame apply)
src/render/Renderer.cpp:236              (.processFrame — the ONLY per-frame caller)

$ grep -n "startTimerHz\|timerCallback" src/ui/SignalBar.cpp src/ui/SignalBar.h
src/ui/SignalBar.cpp:34:    startTimerHz(30);
src/ui/SignalBar.cpp:111:void SignalBar::timerCallback()
(confirms SignalBar's own independent, always-on 30Hz message-thread tick)

$ grep -n "mutex\|jassert\|MessageManager\|atomic" src/signal/SignalRegistry.h src/signal/SignalRegistry.cpp
(no output — zero synchronization on cachedValues_, confirms the data race claim)

$ grep -rln "EffectStackView" tests/
$ grep -rln "isConnected\|SourceMode::Signal\|SourceMode::Macro\|SourceMode::Oscillator\|SourceMode::Envelope" tests/
(both: no output — zero test coverage of this code path in tests/, so the
proposed extraction changes no test-visible signature and cannot regress ctest)

$ grep -n "RoutingEngine engine\|\.processFrame(" tests/test_routing_engine.cpp
tests/test_routing_engine.cpp:93,113,132,145,157: constructs its own local
RoutingEngine and calls processFrame() directly — unaffected by relocating
the PRODUCTION caller from Renderer.cpp to MainComponent.cpp; signature
unchanged.
```

## THE CHANGE — step by step

1. In `src/ui/EffectStackView.h`, add `void tickModulation();` to the public
   interface (near `void refresh();`, EffectStackView.h:53).
2. In `src/ui/EffectStackView.cpp`, cut lines 156-210 (the `for (size_t p =
   ...)` loop body that computes `signalValue` and writes `fx.paramValues[p]`
   — everything currently inside `refresh()`'s per-param loop up to and
   including the `if (found) { ... }` block, i.e. NOT the trailing
   `pc.setParamValue(fx.paramValues[p]);` at line 213) into a new
   `EffectStackView::tickModulation()` that iterates `rows_` the same way
   `refresh()` does (lines 141-146: the `for (size_t i = 0; i < rows_.size();
   ++i)` / bounds-check / `fx` lookup preamble is identical — duplicate that
   preamble, or factor it into a tiny shared private helper if preferred;
   either is fine, this is mechanical).
3. In `EffectStackView::refresh()`, replace the cut block with a single call
   to `tickModulation();` at the top of the per-row loop, then keep the
   existing display-sync line (`pc.setParamValue(fx.paramValues[p]);`,
   line 213) and the trailing `repaint()` — refresh()'s VISIBLE behavior is
   byte-identical to today when it does run; only WHERE the compute+write
   happens changes.
4. Add matching one-line `tickModulation()` pass-throughs to
   `ClipInspector`, `LayerInspector`, `CompositionInspector` (mirror each
   class's existing `refresh()` guard pattern — `if (clip_)` /
   `if (layer_)` / unconditional for Composition).
5. Add `InspectorPanel::tickModulation()` that calls all three
   pass-throughs UNCONDITIONALLY (no `switch(activeTab_)` — this is the line
   that actually closes the bug).
6. In `MainComponent::tickFeaturePipeline()` (MainComponent.cpp:2549-2552),
   add `if (inspectorPanel_) inspectorPanel_->tickModulation();` alongside
   the existing `previewPanel_.getMappingEngine().processFrame(...)` call.
   `inspectorPanel_` is a `std::unique_ptr` per the existing null-guard
   pattern already used at every other call site (MainComponent.cpp:4067
   etc.) — reuse it, don't assume non-null.
7. (If D2 folded in) In `src/render/Renderer.cpp`, delete lines 232-241
   (the `routingEngine_.processFrame(...)` block) and its guarding `if
   (signalRegistry_ != nullptr)` at line 232 if nothing else needs it (line
   230's own `if` for `evaluateAll` stays, untouched). Add the equivalent
   call to `MainComponent::tickFeaturePipeline()`, targeting
   `previewPanel_.getRenderer().getEffectChain()` exactly as the deleted
   callback did.

## FENCE — every file this lane will WRITE
- `src/ui/EffectStackView.h`
- `src/ui/EffectStackView.cpp`
- `src/ui/ClipInspector.h`
- `src/ui/ClipInspector.cpp`
- `src/ui/LayerInspector.h`
- `src/ui/LayerInspector.cpp`
- `src/ui/CompositionInspector.h`
- `src/ui/CompositionInspector.cpp`
- `src/ui/InspectorPanel.h`
- `src/ui/InspectorPanel.cpp`
- `src/MainComponent.h`
- `src/MainComponent.cpp`
- (only if D2 folded in) `src/render/Renderer.cpp`

No test file is touched — confirmed zero existing test references any of
these symbols (see CALL-SITE ENUMERATION).

## TRAPS — what will bite the builder

- **Do not thread-guard `tickModulation()` the way `MappingEngine` was
  guarded.** `EffectStackView`, `UniversalParamControl`, `SignalRegistry`,
  and `MacroBank` are all message-thread-only objects already (they're
  `juce::Component`s / plain non-atomic state) — `tickFeaturePipeline()`
  itself runs on the message thread (it's driven by a `juce::Timer`), so
  this is a same-thread relocation, not a cross-thread one like the A6 work
  in c51aff7. No `jassert(isThisTheMessageThread())` is needed or wanted
  here; adding one would be a false-safety cargo-cult from the wrong commit.
- **`tickModulation()` on `ClipInspector`/`LayerInspector` must be called
  even when the tab is not active** — do not accidentally gate the new
  `InspectorPanel::tickModulation()` behind `showActiveTab()`'s visibility
  state or you've reproduced the exact bug in a new place.
- **The connection itself is ephemeral UI state, and is separately wiped on
  every reselect** — `UniversalParamControl::sourceMode_`/`sourceName_`
  (UniversalParamControl.h:116-117) live ONLY on the widget, are never
  persisted to `Clip::EffectSlot` (confirmed: that struct has no
  sourceMode/sourceName field, Clip.h:48-55), and `rebuildRows()`
  (EffectStackView.cpp:237) — called from every `setEffects()`
  (EffectStackView.cpp:130-135) — creates brand-new
  `UniversalParamControl`s every time. `ClipInspector::setClip()`
  (ClipInspector.cpp:746-761), `LayerInspector::setLayer()`
  (LayerInspector.cpp:737-749), unconditionally call `setEffects()` with NO
  guard against re-selecting the SAME clip/layer already showing. **This
  fix does not touch that.** After this fix, a connected param survives
  tab-switching and being "unwatched" — it does NOT survive clicking away
  from that clip and back, or clicking the same clip cell twice; the
  connection silently resets to Manual either way, same as today. Do not
  let a smoke-test declare victory just because tab-switching now works —
  test the reselect case too and expect it to still fail (that is a
  separate, larger, model-schema-touching bug — see OPEN QUESTIONS).
- **`onParamChanged` is dead — do not "fix" it by wiring it up as part of
  this change.** It looks like an obvious loose end but wiring it now is
  scope creep with no test coverage to catch a mistake; the actual
  modulation path is the direct `fx.paramValues[p]` write, already correct
  once relocated.
- **If D2 is folded in:** `routingEngine_` lives on `Renderer`
  (Renderer.h:337), not on `PreviewPanel` or `MainComponent` — go through
  `previewPanel_.getRenderer().getRoutingEngine()`
  (Renderer.h:118 already exposes it), don't add a new forwarding accessor
  on `PreviewPanel` unless you also update every other place that reaches
  into `Renderer` the same way (check first — `getMappingEngine()` and
  `getEffectChain()` ARE forwarded on `PreviewPanel`, `getRoutingEngine()`
  currently is NOT — that asymmetry is pre-existing, not something to “fix”
  silently as a drive-by).
- **Do not move `signalRegistry_->evaluateAll(snap)`** (Renderer.cpp:230-231)
  even though it's textually adjacent to the code you're deleting for D2 —
  it is accidentally already safe via `SignalBar`'s independent timer, and
  moving it doesn't fix the data race (the race is TWO writers; moving one
  of them from GL-thread to message-thread just makes it two message-thread
  writers with no ordering guarantee between MainComponent's tick and
  SignalBar's tick — still unsynchronized, still UB in principle, just a
  smaller window). Leave it exactly where it is; that's a separate fix
  (add a mutex or confine to one thread) this lane should not attempt.

## HOW TO PROVE IT WORKS

Static/logical (no runtime needed, can be checked by reading the diff):
- `InspectorPanel::tickModulation()`'s body has no `switch`/`if
  (activeTab_ == ...)` conditional — grep for `activeTab_` inside that new
  method and confirm zero hits.
- `EffectStackView::refresh()` still ends with the same observable output
  (repaint, `pc.setParamValue`) it does today — a UI screenshot diff of any
  expanded effect row before/after this change should be pixel-identical
  when the panel IS being watched (this is a pure relocation for the
  watched case).

Runtime, human-required (this genuinely cannot be checked without running
the app — flagging per the task's own framing that it "cannot be run" here):
1. Build in test mode (`--test-mode`, port 8080). Open a clip with an
   effect, connect one param to a Signal (e.g. an RMS-derived signal) via
   the right-click/connect UI on that param.
2. Confirm the param visibly modulates while the Clip tab is active (sanity
   baseline).
3. Switch InspectorPanel to the Signal tab (or select a different clip).
   BEFORE this fix: the connected param's rendered value should freeze —
   confirm visually in the output/preview window, or by reading
   `paramValues` through a temporary debug hook (there is currently NO
   `/api/state` field exposing Composition-model `Clip::EffectSlot` param
   values — confirmed: `grep -n "isConnected\|sourceMode\|connected"
   src/test/TestServer.cpp` returns nothing relevant; `/api/state`'s
   `effects` array is exclusively `effectChain_`, System A, not this
   Composition model — see CALL-SITE ENUMERATION). AFTER this fix: it
   should keep tracking the signal.
4. If a REST-probe is wanted for this instead of eyeballing it, the
   smallest addition is exposing the currently-inspected clip/layer/global
   effect chain's `paramValues` in `TestServer::/api/state` (a new field,
   not required by this lane, but would let a probe mirror
   `tests/visual/test_mapping_tick.py`'s existing pattern — sample twice
   across a tab switch instead of across a GL-attach state).
5. For D2 specifically (if folded in): `tests/visual/test_mapping_tick.py`
   already has the exact harness needed — states `signalbar` /
   `signalbar_output` detach the preview GL context. Add a Route (via the
   `/api/route` style endpoints TestServer.cpp:828/841/855/857 already
   support for route management) and assert it keeps applying in those two
   states, the same way that file already asserts for mappings. This is a
   natural sibling test, not a new pattern.
6. Regression check: reselecting the SAME clip, or clicking away and back,
   should STILL reset the connection to Manual after this fix (see TRAPS) —
   confirm this is unchanged behavior, not a new regression, so it isn't
   mistakenly reported as "the fix didn't work."

## OUT OF SCOPE — what this lane deliberately does not do, and why
- **Persisting "connected" source metadata into the model** (`Clip::
  EffectSlot` gaining per-param `sourceMode`/`sourceName` fields, threaded
  through `Clip.cpp`/`Layer.cpp`/`Composition.h` serialization). This would
  ALSO fix the reselect-wipes-connection bug (TRAPS, above) as a side
  effect, and would let `tickModulation()` run for chains that aren't even
  currently selected in any inspector (the deeper "other layers' clips
  never tick their connections at all" gap) — but it's a real schema
  migration with save/load compatibility questions, clearly a separate,
  larger lane. Flagging it here so it isn't lost.
- **Fixing the `SignalRegistry::cachedValues_` data race.** Real, verified,
  unrelated to the freeze claim. Needs its own decision (mutex vs. single-
  thread confinement vs. atomic-per-slot) and shouldn't ride in on a
  "modulation freeze" commit.
- **Implementing Clip/Layer-scope `RoutingEngine` targets** (the
  `// TODO: Clip/Layer scope routing needs compositor integration` at
  Renderer.cpp — currently a silent no-op for non-Global routes). Pre-
  existing incomplete feature, not a regression, not a freeze.
- **Wiring up `EffectStackView::onParamChanged`.** Dead code, but touching
  it is unrelated scope creep (see TRAPS).

## OPEN QUESTIONS — could not settle from source
- Is the reselect-wipes-connection behavior (TRAPS) already a KNOWN,
  accepted limitation (e.g. documented in gotchas/known-residuals
  elsewhere) or a second undiscovered defect? I did not find it mentioned
  in `OutputWindow.cpp`'s KNOWN-RESIDUAL block (which c51aff7 itself
  rewrote) or elsewhere I searched — worth a direct question to whoever
  owns the essentials plan before this ships as "the" fix for "lies during
  a live set," since an operator will still see connections vanish on
  clip-reselect immediately after this patches ships.
- Should D2 (RoutingEngine) ride with L9, or become its own lane? It's the
  same seam and mechanically trivial, but it's a genuinely different
  reported symptom (Routes, not "connected" params) that the essentials
  plan never mentioned — I don't have visibility into whether Routes are
  even a shipped/exposed feature in the current UI (TestServer.cpp exposes
  route CRUD, but I did not find a routing UI panel in the files read for
  this packet — did not chase further, out of this lane's read budget).
