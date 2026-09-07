# RTA Freeze Defect Class Enumeration — 2026-09-05
STATUS: DONE

Read-only investigation. No source changed. All line numbers are current as
of this read (2026-09-05) and, per convention, may drift.

## SCOPE OF THIS PASS

Enumerated every site that (a) computes a "connected"/modulated parameter
value inside a UI refresh/paint/timer path, and classified each against the
render-path consumer that actually reads the value. Traced beyond
UniversalParamControl-owning classes into MacroBank (the shared
signal-source-of-a-source-of-a-source layer) and independently verified the
critic's CompositionInspector lead by reading every `.cpp` that owns a
UniversalParamControl, not just CompositionInspector's.

## HEADLINE ANSWER

**L9's fix (as scoped in its FENCE list) closes 3 of the 6 instances below
completely, and improves a 4th (MacroBank) from "freezes almost always" to
"freezes only when the Signal tab is active" — it does not close it. A 5th
instance (CompositionInspector's own EffectStackView / Global scope) has
no currently-live render consumer, so L9's fix there is precautionary, not
curative. A 6th instance — 21 separate UniversalParamControl fields across
all three inspectors (position/scale/rotation/anchor/opacity/master/speed)
— has no compute-and-apply logic AT ALL; connecting them is decorative.
That is a different, larger defect (NEVER APPLIES, not FREEZES) that L9's
"extract into tickModulation()" fix shape cannot address by definition —
there is nothing there to extract.**

To honestly claim "modulation freeze: fixed" for a live VJ set, L9 must
either also cover instance 4 (MacroBank) or explicitly scope it out in the
commit message/plan doc as a known residual. Instance 5 needs no code
change (nothing to fix yet) but should not be reported as "fixed" either —
it was never broken because it never worked. Instance 6 should be logged
as a separate defect, out of L9's scope by construction, not silently
absorbed or silently ignored.

## ENUMERATION TABLE

| # | Site (compute+write) | Writes to | Render-path consumer (proof) | Gate | Class | Covered by L9's FENCE list? |
|---|---|---|---|---|---|---|
| 1 | `EffectStackView::refresh()`, `src/ui/EffectStackView.cpp:137-213`, owned by `ClipInspector::effectStackView_` | `Clip::EffectSlot::paramValues[p]` (via `clip->effects`) | `CompositorEngine::applyClipEffects`, `src/render/CompositorEngine.cpp:242-376` — reads `clip.effects[*].paramValues[p]` every GL frame via `compositeDeck()` → `applyClipEffects` (CompositorEngine.cpp:706-707: `clipTex = applyClipEffects(*clip, ...)`) | `ClipInspector::refresh()` `if (clip_)` (ClipInspector.cpp:819) + `InspectorPanel::refresh()` `switch(activeTab_)` Tab::Clip only (InspectorPanel.cpp:174) | **FREEZES** | YES — this is L9's D1 primary target |
| 2 | Same `EffectStackView::refresh()` class, owned by `LayerInspector::effectStackView_` | `Clip::EffectSlot::paramValues[p]` (via `layer->layerEffects`) | `CompositorEngine::compositeDeck`, `src/render/CompositorEngine.cpp:758-762`: `layerFxClip.effects = layer.layerEffects; clipTex = applyClipEffects(layerFxClip, ...)` — same read mechanism as #1, every GL frame | `LayerInspector::refresh()` `if (layer_)` (LayerInspector.cpp:775) + `InspectorPanel` tab==Layer only | **FREEZES** | YES — L9's FENCE list explicitly names `LayerInspector.h/.cpp` for the `tickModulation()` pass-through |
| 3 | Same `EffectStackView::refresh()` class, owned by `CompositionInspector::effectStackView_` | `Clip::EffectSlot::paramValues[p]` (via `composition_->globalEffects`) | **NONE FOUND.** `grep -rn "globalEffects" src/render/` → zero hits. `CompositorEngine`'s every entry point (`compositeDeck`, `applyClipEffects`, `applyFXOnlyLayer`, `applyMaskLayer`, etc.) takes `Deck&`/`Layer&`/`Clip&`, never `Composition&`. Confirmed independently, and matches an existing in-repo comment: `src/core/EffectCommands.h:80-87`: "Global scope (Composition::globalEffects) is NOT currently read anywhere on the GL side (verified...)" | `CompositionInspector::refresh()` (unconditional internally, but still gated by `InspectorPanel` tab==Composition) | **FINE today** (nothing to freeze — the whole Global effect chain is inert on render, connected or not; a pre-existing, separate, larger gap, not a freeze regression) | YES, but moot today — L9's own comment in EffectCommands.h calls this "future-proofing... if a later render change starts reading globalEffects," i.e. it already knows this branch does nothing observable yet |
| 4 | `ClipInspector::refresh()`'s inline loop, `src/ui/ClipInspector.cpp:817-878` (the critic's find) | `clip_->sourceParams[i].value` (Source-type clip params) | `CompositorEngine::compositeDeck`/`applyFXOnlyLayer`/`applyMaskLayer`/`getClipTexture`, `src/render/CompositorEngine.cpp:722,828,892,1072`: `sourceRenderFn_(clip->sourceType, time, width, height, params)` where `params = &clip->sourceParams` — passed to the procedural-source renderer every GL frame | `if (clip_)` (line 819) + `if (clip_->mediaType == Clip::MediaType::Source)` (line 826) + `InspectorPanel` tab==Clip + this exact clip selected | **FREEZES** | **NO — absent from L9's FENCE list.** `ClipInspector.h`/`.cpp` ARE in the fence list, but only for the `tickModulation()` pass-through to `effectStackView_`; the packet's own step-by-step (§3) never mentions this second, independent loop living in the same function. This is the packet's undercount the critic caught — confirmed still open in the packet text as read today. |
| 5 | `MacroBank::updateValues()`, `src/routing/MacroBank.h:64-77`, called only from `MacroPanel::refresh()`, `src/ui/MacroPanel.cpp:70-77` | `Macro::currentValue` (all 8 slots) of the single shared `MainComponent::globalMacroBank_` (`MainComponent.h:330` — one instance, `MacroBank::Scope::Global`; `setMacroBank()` fans the same pointer out to all three inspectors, `InspectorPanel.cpp:126-131`) | Indirectly, by instances #1/#2/#4 above: `EffectStackView.cpp:195: macroBank_->getMacroValue(macroIdx)`, `ClipInspector.cpp:861`, both inside `SourceMode::Macro` branches | `macroPanel_.refresh()` called from `ClipInspector::refresh()` (`if(clip_)`, ClipInspector.cpp:823), `LayerInspector::refresh()` (`if(layer_)`, LayerInspector.cpp:775), `CompositionInspector::refresh()` (unconditional, CompositionInspector.cpp:440) — **but NOT from `SignalInspector`** (`grep -rln "MacroPanel macroPanel_" src/` → only Clip/Layer/CompositionInspector.h) | **FREEZES, narrower window** — because the bank is shared, it refreshes whenever ANY of Clip/Layer/Composition is the active tab; it goes stale only while the Signal tab is active. Still a real freeze case (operator on Signal tab watching a macro-driven effect elsewhere) | **NO — absent from L9's FENCE list entirely** (no `MacroPanel.h/.cpp` or `MacroBank.h` in the file list). Even after L9 ships exactly as scoped, `EffectStackView::tickModulation()` and `ClipInspector`'s sourceParams loop will call `getMacroValue()` at 120 Hz, but the value they read is only as fresh as the last Clip/Layer/Composition tab refresh (~10 Hz, and only while one of those three tabs is active) |
| 6 | 21 individual `UniversalParamControl` fields with NO compute/apply path at all: `CompositionInspector` — `masterControl_, speedControl_, opacityControl_, posXControl_, posYControl_, scaleControl_, rotationControl_, anchorControl_` (8); `LayerInspector` — `masterControl_, opacityControl_, posXControl_, posYControl_, scaleControl_, rotationControl_, anchorControl_` (7); `ClipInspector` — `clipOpacityControl_, posXControl_, posYControl_, scaleControl_, rotationControl_, anchorControl_` (6) | Nothing — connecting these never writes anywhere | N/A — there is no write to observe | N/A | **NEVER APPLIES (not a freeze — never worked)**. Proof: (a) `isConnected()`/`setSourceValue(`/`getCachedValue(` repo-wide grep hits only in `EffectStackView.cpp` and `ClipInspector.cpp`'s sourceParams loop (instances 1/2/3/4) — zero hits for any of these 21 controls; (b) `onSourceChanged` (`UniversalParamControl.h:92`, fired at `UniversalParamControl.cpp:545` on every source-picker selection) has **zero assignments anywhere in `src/`** (`grep -rn "onSourceChanged\s*="` → empty) — the only callback that could react to a connection is wired to nothing; (c) each control's own `setupTransformParam` helper (`ClipInspector.cpp:391-396`, and the equivalent in `LayerInspector.cpp`/`CompositionInspector.cpp`) wires only `onExpandToggled`; each control's separate `onValueChanged` (lines cited above per class) only fires from direct manual slider drags. The triangle turns cyan, the source-name hint paints, the meter placeholder draws — `UniversalParamControl::paint()` (`UniversalParamControl.cpp:129,145,168`) — but nothing downstream ever reads `sourceMode_`/`sourceName_`. | **NO, and structurally cannot be** — L9's fix shape is "extract the existing compute+write loop out of refresh() into an unconditional tick." There is no existing compute+write loop for these 21 controls to extract. This needs new code (a per-control isConnected()-driven compute step deciding what each connected value writes into, e.g. `clip_->positionX`), not a relocation. It is a materially different, larger piece of work and should be tracked as its own item, not folded into "modulation freeze: fixed." |

## ITEM 3 FROM THE TASK — CompositionInspector's Scale/Rotation/Anchor, settled

**Confirmed, not refuted — and generalizes further than asked.** The critic's
lead is correct for `CompositionInspector`, and the identical shape (same
missing `onSourceChanged` wiring, same absent isConnected()-driven apply
step) is independently present in `LayerInspector`'s and `ClipInspector`'s
own transform/opacity/master controls too (row 6 above, VERIFIED for all
three classes by reading each `.cpp`'s constructor wiring and each
`refresh()` body in full — not inferred from one file and assumed for the
others). Connecting Scale/Rotation/Anchor/Position/Opacity/Master/Speed to
a signal on ANY of the three inspectors is currently a cosmetic no-op:
it never freezes because it never applied in the first place.

## ITEM 4 — DOES L9's FIX GENERALIZE?

- Instances 1, 2: yes, cleanly — same class, same shape, already in the
  FENCE list.
- Instance 3 (Global/Composition EffectStackView): yes mechanically, but
  moot until something reads `globalEffects` on the render side; applying
  the fix here is harmless future-proofing, not a bug fix today.
- Instance 4 (ClipInspector's sourceParams loop): **NO — this is a
  structurally identical bug (same tab-gating root cause, same "computed
  in refresh(), consumed every GL frame" shape) living in a different
  function than the one L9's packet traced.** The fix shape generalizes
  perfectly (give `ClipInspector` a second `tickModulation()` responsibility
  — or fold this loop into the same method — that also runs unconditionally
  from `InspectorPanel::tickModulation()`), but the packet's current FENCE
  list and step-by-step (§3) do not mention it and would ship without
  covering it if executed exactly as written.
- Instance 5 (MacroBank): the fix shape does NOT generalize by simply
  extending `tickModulation()` to the three inspectors — `MacroBank`'s
  `updateValues()` needs its own unconditional call (e.g.,
  `globalMacroBank_.updateValues(*signalRegistry_)` added directly to
  `MainComponent::tickFeaturePipeline()`, independent of any inspector).
  This is a small, mechanical addition (one call, one object, already
  owned by `MainComponent`), but it is a DIFFERENT call site than anything
  in L9's current fence list and must be added explicitly or the claim
  "modulation freeze: fixed" is false for every Macro-sourced connection.
- Instance 6: does NOT generalize — new code required, different bug class
  (NEVER APPLIES vs FREEZES), out of L9's scope by construction.

## ITEM 5 — COST OF UNCONDITIONAL 120 Hz TICK

Read JUCE's own `Component::repaint()`/`internalRepaint()`/
`internalRepaintUnchecked()` (`build/_deps/juce-src/modules/juce_gui_basics/
components/juce_Component.cpp:1574-1637`) directly rather than assuming:
each `repaint()` call walks up the parent chain via
`internalRepaint()` → `internalRepaintUnchecked()`, and **stops the instant
it reaches an ancestor whose own `flags.visibleFlag` is false** (line 1609:
`if (flags.visibleFlag) { ... }`, else falls through, no peer repaint, no
`cachedImage->invalidate`). `InspectorPanel::showActiveTab()`
(`InspectorPanel.cpp:216-221`) hides inactive tabs by calling
`setVisible(false)` on the whole tab **viewport** (`clipViewport_` /
`layerViewport_` / `compViewport_`), so a `pc.setSourceValue()` repaint
from a hidden tab's `UniversalParamControl` walks a handful of component
levels (control → row → `EffectStackView` → Inspector → hidden viewport)
and stops there — no actual OS paint dispatch, no GL/graphics cost.

Net: moving instances 1/2/4 (and, if added, 5) to an unconditional 120 Hz
tick is NOT "12x more real paint work" — it's ~12x more short pointer-chase
walks that dead-end at the first hidden ancestor, plus ~12x more of the
actual compute (a linear scan over `signalRegistry_->getNumSignals()` per
connected param, bounded by the number of signals defined, typically small
single digits). The compute set itself stays small regardless of tick
rate: only the ONE currently-selected clip, ONE currently-selected layer,
and the Composition's global chain exist as live `UniversalParamControl`
rows at any time (`rebuildRows()` / `buildSourceParamControls()` throw away
and rebuild rows on every `setClip()`/`setLayer()`/`setEffects()` call) — it
is not "every clip in the composition," so there is no combinatorial
blowup. **This looks acceptable** — same conclusion the packet itself
reached, now independently verified against JUCE's actual source rather
than assumed.

## COUNT-TO-HONESTY

L9 must cover **instances 1, 2, and 4** at minimum, and add the one-line
`MacroBank::updateValues()` relocation (**instance 5**) to honestly claim
"modulation freeze: fixed" for every currently-live, currently-reachable
"connected" parameter path in this codebase. Instance 3 needs no new work
(nothing to fix yet; leaving it as planned is fine). Instance 6 is real,
independently confirmed across all three inspectors, but is NOT a freeze —
it should be logged as its own defect ("connect-a-signal on Position/Scale/
Rotation/Anchor/Opacity/Master/Speed silently does nothing"), not merged
into or used to gate the freeze-fix lane.

## LABELS

All findings above are VERIFIED (source read end-to-end: compute site,
gate, and render/consumer site, or an exhaustive repo-wide grep proving
absence) except the acceptable-cost conclusion in ITEM 5, which is VERIFIED
against JUCE's actual `Component.cpp` source in this repo's vendored build
tree, not from memory of JUCE's general behavior.
