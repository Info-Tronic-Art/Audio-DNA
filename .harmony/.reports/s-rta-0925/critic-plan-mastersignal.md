# Critic — plan-mastersignal.md (s-rta-0925)

Adversarial review against `main` @ `85afb70` (plan was written against `6d65fc9`/`6d5fc9`-era
`main` before `b187781`/`85afb70` merged — see §4 on line-drift). Confidence: VERIFIED = read on
disk this pass; INFERRED = derived; ASSUMED = plan's own label, not re-checked by me.

VERDICT: REQUEST_CHANGES (one BLOCKING finding — a missed signal path that reopens exactly the
bug this plan exists to fix). Everything else I attacked held up: the plan is unusually well
grounded, nearly every `path:line` citation I spot-checked matches disk exactly.

---

## BLOCKING — F1: `MacroPanel::refresh()`'s second `updateValues()` call site is not touched,
## so a signal-driven macro re-snaps to FULL depth ~10×/sec whenever any Inspector tab is open

**Claim in the plan** (§2.2, `MacroBank.h:74-87`): `updateValues(signals, signalDepth=1.0f)` is
edited so `macro.currentValue = applyDepth(manual, signal, signalDepth)`, and the plan's own audit
states: *"This is the UI-reachable macro drive (`MacroPanel.cpp:136,144` write `sourceSignalId`;
`Macro::conn` is JSON/REST-only today)."* — i.e. the plan believes it has enumerated every place a
macro's `currentValue` gets computed, and edits the ONE call site at
`MainComponent.cpp:3336` (`globalMacroBank_.updateValues(signalRegistry_)`, in the 120 Hz
`tickFeaturePipeline`) to pass `signalDepth`.

**What's actually on disk (VERIFIED):** there is a SECOND call site the plan's grep missed —
`src/ui/MacroPanel.cpp:76`, inside `MacroPanel::refresh()`:
```cpp
void MacroPanel::refresh()
{
    if (!macroBank_) return;
    if (signalRegistry_)
        macroBank_->updateValues(*signalRegistry_);   // <-- unedited by the plan
    ...
}
```
`macroBank_` here is `&globalMacroBank_` — the SAME bank the 120 Hz tick writes
(VERIFIED: `MainComponent.cpp:1494 inspectorPanel_->setMacroBank(&globalMacroBank_)` →
`InspectorPanel::setMacroBank` → `clipInspector_/layerInspector_/compInspector_.setMacroBank(bank)`
→ each does `macroPanel_.setMacroBank(bank)`). `MacroPanel::refresh()` is reached from
`ClipInspector::refresh()`, `LayerInspector::refresh()`, `CompositionInspector::refresh()` — all
three call `macroPanel_.refresh()` unconditionally (not display-sync-only like the sibling
`tickModulation()` paths the L9 fix carved out; this call still does the COMPUTE, not just paint).

`InspectorPanel::refresh()` is itself called unconditionally on a live timer
(VERIFIED, `MainComponent.cpp:3406-3409`):
```cpp
// Refresh inspector at ~10Hz to show signal-driven values
if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_)
    inspectorPanel_->refresh();
```
inside the 30 Hz `MainComponent::timerCallback` (`startTimerHz(30)` at `:416`) — a DIFFERENT JUCE
Timer from the 120 Hz `mappingTickTimer_` that drives `tickFeaturePipeline`. `signalRegistry_` is
unconditionally wired (`MainComponent.cpp:1493`), so this path is live in every build, every
session, any time an Inspector tab is visible (Clip/Layer/Composition — i.e. essentially always;
the app's whole UI is built around one of these tabs being open).

**Consequence at runtime:** with the plan's edit applied, `macro.currentValue` gets written by two
independent, unsynchronized timers on the same message thread:
- 120 Hz: `globalMacroBank_.updateValues(signalRegistry_, signalDepth)` → correctly scaled toward
  `manualValue` when the fader is below 100%.
- ~10 Hz (every ~100 ms): `macroPanel_`'s call → `updateValues(signalRegistry_)` with the
  DEFAULTED `signalDepth = 1.0f` → **full, unscaled signal value**, clobbering the depth-scaled
  value the 120 Hz tick just wrote.

Any render/read that happens to land in the ~8 ms window right after that 10 Hz call (before the
next 120 Hz tick overwrites it again) sees the macro at FULL signal strength regardless of where
the Master Signal fader sits — a visible "pop" roughly every 100 ms, worst (most visible) at
**exactly 0% depth**, which is the case Boris will test first (R1: "a macro knob driven by audio
sits at its hand value" at 0%). This is not a rare race window; it is a deterministic ~10×/sec
stomp, reachable in the default UI state (an Inspector tab open), and it directly reopens the
double-application-class bug (critic-mastersignal.md finding 2 / this plan's own D3) the plan
claims to have closed by construction. Same class of bug as the ORIGINAL ordering finding this
plan was written to fix — a second, un-hoisted writer of the same state.

**Fix (two lines, same pattern the plan already uses elsewhere):**
- Simplest: `MacroPanel::refresh()` should NOT recompute — display-sync only, matching the L9
  precedent already established for `EffectStackView`/`ClipInspector::tickModulation()`. Drop the
  `macroBank_->updateValues(*signalRegistry_)` call from `refresh()` entirely (the 120 Hz tick
  already keeps `currentValue` fresh; `refresh()` only needs to read it for display).
- If that call exists for a reason not visible from this read (e.g. correctness when no
  `tickFeaturePipeline` is running, such as before the first tick or in a test harness that drives
  `MacroPanel` standalone), then it must take the CURRENT `signalDepth` too — e.g. expose
  `MainComponent::currentSignalDepth()` or route the read through `composition_.eff(CompScalar::Signal)`
  at the call site — never the defaulted `1.0f`.
- Either way, add a grep gate: `grep -rn 'updateValues(' src | grep -v '\.h:'` must show every
  call site passing `signalDepth` (or none computing at all) — the plan's existing §2.8 gate list
  does not check this call count and should (`grep -c 'updateValues(' src/ui/MacroPanel.cpp src/MainComponent.cpp` should be reconciled against the depth-carrying edit).

---

## Non-blocking — verified correct / no other missed paths found

I attacked the other three hunt targets named in the task and could not break them:

- **The ordering bug (critic's original BLOCKING finding).** VERIFIED the hoist is real and in the
  right place: `signalRegistry_.evaluateAll(snap)` (`MainComponent.cpp:3324`) →
  `getMappingEngine().processFrame` (`:3326`) → `globalMacroBank_.updateValues` (`:3336`) →
  `ConnectionEngine::Context ctx{...}` (`:3368`) → `connectionEngine_.tick` (`:3370`). The plan's
  cited line numbers (3316/3318/3328/3360/3362/3364 against `6d65fc9`) are offset by exactly +8
  from current HEAD — fully explained by `b187781`'s first hunk (`-1944,13/+1944,21`, net +8 lines,
  landing textually BEFORE `tickFeaturePipeline`); no other replay-restore hunk falls before this
  function. The plan's own §4 already calls out this in-flight collision and says "merge
  replay-restore first if ready" — it is already merged in HEAD, so the drift is exactly what the
  plan anticipated, not new damage.
- **D3's double-depth-on-macro-chains fix.** VERIFIED by reading `ConnectionEngine::evaluate`
  (`:69-178`) and `tick` (`:262-...`): the proposed `if (c.source.kind != ConnSource::Kind::Macro)`
  guard is keyed on the SOURCE kind of the connection being evaluated, not on "am I inside the
  macro loop" — so a Signal/Lfo/Envelope-sourced macro (evaluated inside the `for` at `:287-299`)
  DOES take the depth (source.kind == Signal there), while a scalar/effect connection whose source
  IS `Kind::Macro` (evaluated later, e.g. `tickScalars` at `:301-302`) is correctly exempted
  because the macro's own conn already took it. Logic is sound and matches the plan's worked
  examples (S1-T5/T6) — modulo F1 above breaking it in practice via the untouched call site.
- **RT-safety / GL-thread isolation (Boris Q2).** VERIFIED `EffectChain::render`'s uniform upload
  (`EffectChain.cpp:387-393`, `param.value` plain-float read) and `CompositorEngine`/`Renderer`
  never gain any new field per the plan's D1; the only pre-existing message-thread→GL-thread
  crossing (`EffectParam::value`, the "A2 known-deferred crossing") is untouched by this plan — the
  race profile is unchanged, not newly introduced.
- **CompScalar::Signal append safety.** VERIFIED current `enum class CompScalar` has exactly 8
  entries ending at `AnchorY, Count` (`ScalarParams.h:34`); inserting `Signal` immediately before
  `Count` is a pure append, and `grep -rn 'CompScalar::Count' src tests` shows ZERO uses outside
  `ScalarParams.h` itself — the plan's claim "no other CompScalar::Count-sized structure exists" is
  correct, so this is not an insert-in-the-middle enum-renumbering hazard.
- **Persistence / Boris Q3 ("save it").** VERIFIED the plan's `fromVar` guard explicitly copies
  `masterSpeed`'s `hasProperty`-guarded pattern (`Composition.h:339-341`), not `masterOpacity`'s
  unguarded read (`:331`) — an old composition without the key correctly loads `1.0`, matching
  Boris's Q3 ruling and the binding-decisions.md entry verbatim.
- **Binding.h append.** VERIFIED `Action::ToggleRecording` is currently the LAST enumerator with no
  trailing comma — the plan's edit (append `MasterSignal` after it, add the comma) is a pure
  append, doesn't renumber existing bindings.
- **`effParam`/`onSourceParamsPublished`/source-param NaN-skip bugs (Step 0 basis).** All VERIFIED
  present exactly as cited: `Clip.h:90-91` unguarded `paramLive[i]` index; `onSourceParamsPublished`
  declared (`ConnectionEngine.h:75`) and never assigned anywhere in `src/`; the source-param loop
  (`ConnectionEngine.cpp:327-338`) skips the store on NaN (only `sp.live.v.store` inside the
  `if (!std::isnan(y))` branch) — confirms the Step-0 rationale is not invented.
- **`RoutingEngine` dead-code claim (D7).** VERIFIED `grep -rn 'processFrame(' src | grep -i rout`
  returns only the declaration/definition and Renderer.cpp's own "dead" comment (`:251`) — no live
  caller.

## Conflicts with binding-decisions.md

None found beyond F1's practical violation of Q2/R1's "at 0%, a hand-turned macro keeps working"
promise. Q1 (Gain stays visible) is untouched by this plan — correct, no conflict. Q2 (no GL/
uniform scaling, effects/sources keep pulsing) is honored by D1 and the grep gate in §2.8. Q3
(persist, absent→1.0) is honored per above.

## Minor / non-blocking observations (not gating)

- §2.10 ("What Boris sees") states universally that a connected control "sits at its own hand
  value" at 0% signal — but for v1 `MappingEngine`-targeted effect params (`EffectsRackPanel`),
  the anchor is `EffectParam::defaultValue` (the shader default), not a user-set hand value, since
  that legacy path has no manual field distinct from the live value (§2.2 already discloses this
  correctly in the design table — only the user-facing summary in §2.10 slightly overclaims for
  this one legacy surface). Cosmetic; not blocking, and the design section itself is honest about it.
- §2.4's width-budget arithmetic is explicitly INFERRED (launch width 1728pt) and self-flagged as
  "the builder measures" — appropriately hedged, not a defect.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0925/critic-plan-mastersignal.md
STATUS: DONE
