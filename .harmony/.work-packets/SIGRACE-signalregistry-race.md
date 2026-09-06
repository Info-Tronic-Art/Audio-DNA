# SIGRACE work packet — SignalRegistry::cachedValues_ data race

## VERDICT: BUILDABLE
Root cause is fully traced end to end; the fix is a self-contained representation
change inside `SignalRegistry.h`/`.cpp` with zero required edits anywhere else,
including tests/.

## SIZE: small
2 files touched (`src/signal/SignalRegistry.h`, `src/signal/SignalRegistry.cpp`).
~15 lines changed total: 1 member-type change, 1 `#include`, 2 statements at the
race site (write/read), and a restructure of `initDefaults()`'s cache-sizing
(5 `push_back` sites collapse into 1 post-loop sizing statement) plus small
rewrites of `addSignal()`/`removeSignal()` so they still compile against the
new member type (both are currently dead code but are public API and must not
be left uncompilable). Recommend also adding one `static_assert`. No test
file changes needed (verified below).

## CURRENT BEHAVIOUR (verified) — what happens today, traced end to end

`SignalRegistry` (src/signal/SignalRegistry.h) is a single instance,
`MainComponent::signalRegistry_` (src/MainComponent.h:317: `SignalRegistry
signalRegistry_;`), owned by `MainComponent` and handed out as `&`/`const &`
to everything else. Its private member

```
// src/signal/SignalRegistry.h:49
std::vector<float> cachedValues_;
```

is written by exactly two call sites and read by exactly one accessor
(`getCachedValue`), called from many places. All of this is a raw
`std::vector<float>` with **no synchronization of any kind** — no mutex, no
atomics, no seqlock.

**Writers of `cachedValues_` (both go through `evaluateAll`, SignalRegistry.cpp:150-156):**
1. `Renderer::renderOpenGL()` — src/render/Renderer.cpp:231:
   `signalRegistry_->evaluateAll(snap);` — runs on the **GL render thread**
   (JUCE's `OpenGLContext::CachedImage::RenderThread`), once per rendered
   frame, using the render loop's own `snap` (`featureBus_.read()` a few
   lines above, Renderer.cpp:224).
2. `SignalBar::timerCallback()` — src/ui/SignalBar.cpp:117:
   `registry_.evaluateAll(displaySnap_);` — runs on the **message thread**
   (`juce::Timer`, `startTimerHz(30)` at SignalBar.cpp:33), using SignalBar's
   *own*, independently-polled snapshot (`displaySnap_ = featureBus_.read();`
   at SignalBar.cpp:114-115 — the SAME `FeatureBus`, but read at a different
   cadence/instant than the render thread's read).

   **This is the correction to the task's own premise** (task section 4 frames
   it as "the reader is a UI bar... the writer is the render loop" — singular
   writer). That framing is wrong: SignalBar is not a pure reader, it is a
   second, independent WRITER, calling the exact same mutating function
   (`evaluateAll`) from a second thread. The two TSan logs bear this out
   directly — one of the two captured race pairs in
   `.harmony/ow-c1-signalregistry-race.log` (lines 3-25) is a **write-vs-write**
   race (`SignalBar::timerCallback` main-thread write vs.
   `Renderer::renderOpenGL` T39 write, both at SignalRegistry.cpp:154), not a
   read-vs-write race. See CALL-SITE ENUMERATION below for why this dual-write
   is almost certainly intentional (SignalBar is built to work independent of
   GL-context lifecycle), not an accidental redundant call.

**Reader of `cachedValues_` (all go through `getCachedValue`, SignalRegistry.cpp:158-166):**
- `SignalBar::timerCallback()` — SignalBar.cpp:122 (message thread, same
  timer as writer #2 above)
- `ClipInspector.cpp:848` (message thread — panel refresh, driven by a
  higher-level tick; see below)
- `EffectStackView.cpp:179`, inside `EffectStackView::refresh()`
  (src/ui/EffectStackView.h:53) — message thread; per the *existing* repo
  comment at src/render/Renderer.h:384-387, this class is driven by
  "EffectsRackPanel's 10Hz timer"
- `RoutingEngine::processFrame()` — src/routing/RoutingEngine.cpp:89, called
  from Renderer.cpp:236, **on the GL thread, immediately after the GL
  thread's own `evaluateAll()` call** (same thread, program order — not a
  race with the GL-thread writer, but see TRAPS for a same-buffer coherency
  caveat)
- `MacroBank::updateValues()` — src/routing/MacroBank.h:74, called from
  `MacroPanel::refresh()` (src/ui/MacroPanel.cpp:76), itself called from
  `CompositionInspector.cpp:440`, `ClipInspector.cpp:823`,
  `LayerInspector.cpp:775` — all plain `juce::Component`-derived panels
  (verified: none of `CompositionInspector`/`ClipInspector`/`LayerInspector`
  inherit `juce::Timer`; grep below), message-thread callback chains
  (selection-changed / DragAndDropTarget style panels)
- `TestServer::handleListSignals()` — src/test/TestServer.cpp:768 — **a third
  thread family**: httplib's own worker thread pool. Only compiled when
  `AUDIODNA_BUILD_TEST_SERVER` is ON (CMakeLists.txt:377-378, opt-in, off by
  default) via `AUDIODNA_TEST_SERVER=1` (src/test/TestServer.h:31-32: "Runs on
  a background thread"). Not exercised by either captured TSan log (neither
  log shows an httplib thread in a stack), but it IS a live reader with zero
  marshaling to the GL/message thread whenever that build flag is on — it
  must not be forgotten when reasoning about "is a mutex/atomic enough" vs.
  "do we also need thread confinement," because a mutex/atomic protects it
  automatically (it goes through the same `getCachedValue()` choke point)
  while a GL-thread-confinement design would NOT (see THE CHANGE for why
  this rules out confinement).

`ApiServer` holds a `SignalRegistry&` (src/api/ApiServer.h:111) but — verified
by grepping the whole file for "signal" (see CALL-SITE ENUMERATION) — never
calls `getCachedValue`/`evaluateAll`/anything else on it. It is currently a
dead reference. Not a contributor to this race; noted as an open question.

`Autopilot::processFrame(Deck&, const FeatureSnapshot&)` (src/model/Autopilot.h:22)
and `PresetSelector::processFrame(const FeatureSnapshot&)` (src/sources/PresetSelector.h:21)
and `MappingEngine::processFrame(const FeatureSnapshot&, EffectChain&)`
(src/mapping/MappingEngine.h:49) — verified by reading every signature above —
take no `SignalRegistry` parameter at all. **Autopilot does not touch
SignalRegistry.** This directly answers task item 2 ("used anywhere else...
autopilot") — no.

## ROOT CAUSE (verified)

`cachedValues_` is a plain `std::vector<float>` mutated in place, per-index,
by two threads with no synchronization, and read in place by several more.
The two TSan logs (`.harmony/ow-c1-signalregistry-race.log`, Aug 2, and
`.harmony/tsan-s-rta-0904-L7.log`, Sep 5 — 34 days apart, same binary line)
both land on the identical two lines:

```
// src/signal/SignalRegistry.cpp:150-156
void SignalRegistry::evaluateAll(const FeatureSnapshot& snapshot)
{
    for (size_t i = 0; i < signals_.size(); ++i)
    {
        cachedValues_[i] = signals_[i]->getValue(snapshot);   // line 154 — the write TSan names
    }
}

// src/signal/SignalRegistry.cpp:158-166
float SignalRegistry::getCachedValue(uint32_t signalId) const
{
    for (size_t i = 0; i < signals_.size(); ++i)
    {
        if (signals_[i]->getId() == signalId)
            return cachedValues_[i];                          // line 163 — the read TSan names
    }
    return 0.0f;
}
```

Both logs show TWO race pairs per run (grep `WARNING: ThreadSanitizer` — 2 per
log): (1) write-vs-write at line 154 (SignalBar's message-thread write vs.
Renderer's GL-thread write) and (2) write-vs-read (GL-thread write at line 154
vs. message-thread read at line 163). The `(mutexes: write M0, write M1)`
annotation on the GL-thread side is a **false sense of safety** — M0/M1 are
JUCE's own internal `OpenGLContext::CachedImage::RenderThread::renderAll()`
scoped-lock (juce_OpenGLContext.cpp:795, a `std::scoped_lock<mutex,mutex>`
over JUCE's own render/context bookkeeping) and the native CGL context mutex
(created in `CGLCreateContext`). Neither protects `SignalRegistry`'s memory;
they're just what JUCE happens to hold while calling out to
`Renderer::renderOpenGL()`. The main-thread side holds **no** mutex at all.
So: real, unguarded UB, not a benign TSan false positive.

`Signal::getValue()` itself (src/signal/Signal.h:32, `const`) is VERIFIED pure
for all four concrete Signal subclasses read: `AudioSignal::getValue`
(delegates to `MappingEngine::extractSource`, stateless), `OscillatorSignal::getValue`
and `EnvelopeSignal::getValue` (both read only their own read-only config
fields — `shape_`, `beatDuration_`, `amplitude_`, `phaseOffset_`,
`curveType_`, `points_`, `oneShot_`, `looping_` — no `mutable`, no internal
phase/time state, computed entirely from the passed `snapshot`), and
`ClipPositionSignal::getValue` (already `std::atomic<float>
currentPosition_`, relaxed load — see THE CHANGE, this is the repo's own
existing precedent for exactly this fix). So the race is confined to
`cachedValues_`; it is not hiding inside `getValue()`.

`signals_` (the `std::vector<std::unique_ptr<Signal>>` that `cachedValues_` is
index-parallel to) is NOT part of the active race: it is populated once by
`initDefaults()` (SignalRegistry.cpp:3-83), called exactly once, at
src/MainComponent.cpp:423, in `MainComponent`'s constructor — **before**
`Renderer`'s GL thread exists and before `SignalBar`'s timer starts (both are
wired up afterward: `previewPanel_.getRenderer().setSignalRegistry(...)` is
literally the very next line, MainComponent.cpp:424; `signalBar_ =
std::make_unique<SignalBar>(...)` is at MainComponent.cpp:575). `addSignal()`
and `removeSignal()` — the only other mutators of `signals_`/`cachedValues_`
size — have **zero callers anywhere in src/ or tests/** (verified with two
independent grep patterns, see CALL-SITE ENUMERATION). So the signal *set* is
fixed for the life of the app; only the *values* race.

## FILES TOUCHED
- `src/signal/SignalRegistry.h` — change `cachedValues_`'s declared type;
  add `#include <atomic>`.
- `src/signal/SignalRegistry.cpp` — change the write (line 154) and read
  (line 163) to atomic store/load; restructure `initDefaults()`'s cache
  sizing (currently 5 scattered `push_back(0.0f)` sites); rewrite the bodies
  of `addSignal()`/`removeSignal()` so they still compile (see TRAPS — this
  is not optional, the naive type swap does not compile as-is).

No other file needs to change. Every external caller (`SignalBar.cpp`,
`Renderer.cpp`, `ClipInspector.cpp`, `EffectStackView.cpp`, `MacroBank.h`,
`RoutingEngine.cpp`, `TestServer.cpp`, `test_routing_engine.cpp`) only ever
touches `cachedValues_` through the public `evaluateAll()`/`getCachedValue()`
functions, whose signatures do not change.

## CALL-SITE ENUMERATION — grep commands run + raw output

**All writes/reads of `cachedValues_` (exhaustive, from repo root):**
```
$ grep -rn "cachedValues_" --include="*.cpp" --include="*.h" src/
src/signal/SignalRegistry.h:49:    std::vector<float> cachedValues_;
src/signal/SignalRegistry.cpp:6:    cachedValues_.clear();
src/signal/SignalRegistry.cpp:13:        cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:34:        cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:66:        cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:72:        cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:81:        cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:89:    cachedValues_.push_back(0.0f);
src/signal/SignalRegistry.cpp:99:            cachedValues_.erase(cachedValues_.begin() + static_cast<ptrdiff_t>(i));
src/signal/SignalRegistry.cpp:154:        cachedValues_[i] = signals_[i]->getValue(snapshot);
src/signal/SignalRegistry.cpp:163:            return cachedValues_[i];

$ grep -rn "cachedValues_" --include="*.cpp" --include="*.h" tests/
(no output — private member, never touched directly by tests)
```

**Writers of the shared buffer (two independent patterns agree — both name
exactly the same two call sites, `evaluateAll`):**
```
$ grep -rn "\.evaluateAll(\|->evaluateAll(" --include="*.cpp" --include="*.h" src/
src/ui/SignalBar.cpp:117:    registry_.evaluateAll(displaySnap_);
src/render/Renderer.cpp:231:        signalRegistry_->evaluateAll(snap);

$ grep -rn "evaluateAll" --include="*.cpp" --include="*.h" src/ tests/
src/signal/SignalRegistry.h:38:    void evaluateAll(const FeatureSnapshot& snapshot);
src/signal/SignalRegistry.cpp:150:void SignalRegistry::evaluateAll(const FeatureSnapshot& snapshot)
src/render/Renderer.cpp:231:        signalRegistry_->evaluateAll(snap);
src/ui/SignalBar.cpp:117:    registry_.evaluateAll(displaySnap_);
tests/test_routing_engine.cpp:42:    registry.evaluateAll(snap);
tests/test_routing_engine.cpp:91:    registry.evaluateAll(snap);
tests/test_routing_engine.cpp:130:    registry.evaluateAll(snap);
(2 more test call sites in the same file, all single-threaded Catch2 unit
tests, no concurrency — irrelevant to the race, relevant to "does the fix
compile against tests," see below)
```
CONFIRMED: exactly 2 production writers (Renderer GL thread, SignalBar
message thread). Both grep patterns agree.

**Readers (two independent patterns agree):**
```
$ grep -rn "getCachedValue(" --include="*.cpp" --include="*.h" src/
src/ui/SignalBar.cpp:122:        float val = registry_.getCachedValue(strip->getSignal().getId());
src/ui/ClipInspector.cpp:848:                            val = signalRegistry_->getCachedValue(sig->getId());
src/ui/EffectStackView.cpp:179:                            signalValue = signalRegistry_->getCachedValue(sig->getId());
src/test/TestServer.cpp:768:        sigObj->setProperty("value", static_cast<double>(signalRegistry_.getCachedValue(sig->getId())));
src/signal/SignalRegistry.h:41:    float getCachedValue(uint32_t signalId) const;
src/signal/SignalRegistry.cpp:158:float SignalRegistry::getCachedValue(uint32_t signalId) const
src/routing/MacroBank.h:74:                macro.currentValue = signals.getCachedValue(macro.sourceSignalId);
src/routing/RoutingEngine.cpp:89:            raw = signals.getCachedValue(route.sourceId);
```
CONFIRMED: exactly 6 external reader call sites, all funneling through the
one `getCachedValue()` definition.

**Structural mutators — proving `signals_`/`cachedValues_` size is frozen
after startup (two different grep patterns, both zero hits outside the
method bodies themselves):**
```
$ grep -rn "\.addSignal(\|->addSignal(" --include="*.cpp" --include="*.h" src/
(no output)
$ grep -rn "addSignal\b" --include="*.cpp" --include="*.h" src/ tests/
src/signal/SignalRegistry.cpp:85:void SignalRegistry::addSignal(std::unique_ptr<Signal> signal)
src/signal/SignalRegistry.h:23:    void addSignal(std::unique_ptr<Signal> signal);
(only the declaration/definition — zero callers, confirmed by both patterns)

$ grep -rn "\.removeSignal(\|->removeSignal(" --include="*.cpp" --include="*.h" src/
(no output)
$ grep -rn "removeSignal\b" --include="*.cpp" --include="*.h" src/ tests/
src/signal/SignalRegistry.h:24:    bool removeSignal(uint32_t id);
src/signal/SignalRegistry.cpp:92:bool SignalRegistry::removeSignal(uint32_t id)
(only the declaration/definition — zero callers, confirmed by both patterns)

$ grep -rn "initDefaults(" --include="*.cpp" --include="*.h" src/
src/MainComponent.cpp:423:    signalRegistry_.initDefaults();
src/signal/SignalRegistry.h:20:    void initDefaults();
src/signal/SignalRegistry.cpp:3:void SignalRegistry::initDefaults()
(exactly one call site, in MainComponent's constructor)
```

**Test call sites (proving no test file needs to change — signature is
unaffected, only the private member's type changes):**
```
$ grep -rln "SignalRegistry" tests/
tests/CMakeLists.txt
tests/test_routing_engine.cpp
tests/visual/SIGNAL_TEST_SPEC.md
tests/visual/test_signals.py

$ grep -n "SignalRegistry" tests/test_routing_engine.cpp
tests/test_routing_engine.cpp:4:#include "signal/SignalRegistry.h"
tests/test_routing_engine.cpp:9:TEST_CASE("SignalRegistry initialization", "[signal]")
tests/test_routing_engine.cpp:11:    SignalRegistry registry;
tests/test_routing_engine.cpp:33:    SignalRegistry registry;
tests/test_routing_engine.cpp:86:    SignalRegistry registry;
tests/test_routing_engine.cpp:125:    SignalRegistry registry;
```
All 4 are `SignalRegistry registry;` (default-constructed local, single test
thread, Catch2), calling only `initDefaults()`, `evaluateAll()`,
`getCachedValue()`, `getSignalByName()`, `getNumSignals()` — none of which
change signature. `tests/visual/test_signals.py` drives `TestServer`'s HTTP
endpoints (Python, out of C++ compile scope). No test edits required.

**Other consumers of `SignalRegistry` (full enumeration, answers task item 2
— "used anywhere else"):**
```
$ grep -rn "SignalRegistry" --include="*.h" --include="*.cpp" src/ | grep -v "^src/signal/SignalRegistry\.\(h\|cpp\):"
```
(full output reviewed; summarized in CURRENT BEHAVIOUR above) — holders:
`MainComponent` (owner), `Renderer` (GL-thread evaluator + router),
`SignalBar` (message-thread evaluator + display), `ClipInspector`,
`EffectStackView`, `MacroPanel`/`MacroBank`, `CompositionInspector`,
`LayerInspector`, `InspectorPanel`, `UniversalParamControl`,
`RoutingEngine`, `SignalInspector`, `SignalStrip`, `ApiServer` (holds ref,
unused), `TestServer` (test-mode HTTP reader). `Autopilot` — confirmed NOT a
consumer (signature check above).

**Confirming the UI panels reading `getCachedValue` are message-thread-only
(no Timer base class of their own — driven by an ancestor's timer/callback
instead):**
```
$ grep -n "class CompositionInspector\|class ClipInspector\|class LayerInspector\|class EffectStackView" src/ui/CompositionInspector.h src/ui/ClipInspector.h src/ui/LayerInspector.h src/ui/EffectStackView.h
src/ui/CompositionInspector.h:23:class CompositionInspector : public juce::Component, public juce::DragAndDropTarget
src/ui/ClipInspector.h:25:class ClipInspector : public juce::Component, public juce::DragAndDropTarget
src/ui/LayerInspector.h:28:class LayerInspector : public juce::Component, public juce::DragAndDropTarget
src/ui/EffectStackView.h:26:class EffectStackView : public juce::Component, public juce::DragAndDropTarget
```
None inherit `juce::Timer`. `EffectStackView::refresh()` (called by these
panels' `getCachedValue` reads) is driven externally — per the *existing*
repo comment at `src/render/Renderer.h:384-387`, by "EffectsRackPanel's 10Hz
timer." All message-thread.

**A latent, adjacent, NOT-in-scope hazard found while enumerating — flagged
under OPEN QUESTIONS, not fixed here:**
```
$ for m in setAmplitude setPhaseOffset setShape setBeatDuration setCurveType setOneShot setLooping; do grep -rn "\->$m(\|\.$m(" --include="*.cpp" src/ | grep -v "src/signal/"; done
src/ui/SignalInspector.cpp:62:        osc->setAmplitude(...)
src/ui/SignalInspector.cpp:107:       env->setAmplitude(...)
src/ui/SignalInspector.cpp:70:        osc->setPhaseOffset(...)
src/ui/SignalInspector.cpp:115:       env->setPhaseOffset(...)
src/ui/SignalInspector.cpp:36:        osc->setShape(...)
src/ui/SignalInspector.cpp:54:        osc->setBeatDuration(...)
src/ui/SignalInspector.cpp:99:        env->setBeatDuration(...)
src/ui/SignalInspector.cpp:82:        env->setCurveType(...)
src/ui/SignalInspector.cpp:133:       env->setOneShot(...)
src/ui/SignalInspector.cpp:124:       env->setLooping(...)
```
`SignalInspector` (plain `juce::Component`, message-thread slider/toggle
callbacks) mutates the SAME `OscillatorSignal`/`EnvelopeSignal` objects'
config fields (`amplitude_`, `phaseOffset_`, `shape_`, `beatDuration_`,
`curveType_`, `oneShot_`, `looping_`) that the GL thread's `getValue()`
reads unsynchronized, every frame, inside `evaluateAll()`. VERIFIED as an
unsynchronized source-level hazard (I read both sides of it end to end); NOT
verified as a TSan hit — neither log shows it, presumably because no one
twiddled a Mod knob during either capture. This is a structurally identical
but textually distinct race from SIGRACE's named one. Not fixed by this
packet's change. See OUT OF SCOPE / OPEN QUESTIONS.

## THE CHANGE — step by step

**Design decision first (task items 3-4).** Two threads independently and
*legitimately* call `evaluateAll()` — this is not a bug to delete. VERIFIED
supporting evidence: `SignalBar` is constructed with the raw
`analysisThread_.getFeatureBus()` (MainComponent.cpp:575), not through
`Renderer`, and lives as a top-level `MainComponent`-owned widget independent
of `previewPanel_`'s GL context lifecycle. This is the exact same shape as
the *existing*, already-solved `MappingEngine::processFrame` situation: see
the repo's own comment at src/render/Renderer.cpp:246-251 — that call was
deliberately moved OFF the GL callback onto a message-thread
`MainComponent::MappingTickTimer` specifically "so mapped params keep
updating even while this GL context is detached (previewPanel_ hidden)."
SignalBar's independent `evaluateAll()` call is the same keep-the-signal-bar-
live-when-GL-is-idle design, not an accidental redundant call. **Do not
remove it.** The fix must make two legitimate concurrent writers +
several readers safe, not reduce it to one writer.

That rules out both idioms the task offered as the "obvious" two:
- **`juce::MessageManager::callAsync` (idiom a)** loses: it marshals one-shot
  GL-thread *events* to the message thread. There is no event here — both
  sides need to keep writing a continuously-live shared value on their own
  independent cadence (60+ Hz GL, 30 Hz message-thread timer); there's
  nothing to "call async" from one side to the other without making one of
  them stop being authoritative, which breaks the GL-idle keep-alive property
  above.
- **`Renderer::executeOnGLThread(..., blockUntilFinished=true)` confinement
  (idiom b)** loses, for exactly the reason the repo's own header comment
  gives for rejecting it on `effectChain_` (src/render/Renderer.h:378-390):
  confinement is the right call when writes are RARE and reads are hot
  *on the confining thread* (that's `activeSources_`'s shape — rare
  cross-thread mutation, hot GL-thread-only iteration). `cachedValues_` is
  the opposite shape: writes are frequent from **two** threads, and reads
  happen constantly **off** whichever thread you'd confine to (SignalBar's
  own 30 Hz timer, `ClipInspector`/`EffectStackView`/`MacroPanel` message-
  thread panels, optionally TestServer's httplib pool). Confining to the GL
  thread would force SignalBar's write through a *blocking* round-trip on
  every one of its 30 timer ticks/sec (UI-thread stall waiting on the render
  thread) and still wouldn't cover the httplib reader unless it too paid a
  blocking round-trip per HTTP request. This is the `effectChain_` shape,
  and the repo already concluded a lighter-weight-than-confinement mechanism
  wins there.
- **The FeatureBus seqlock (cb4d5fa, `src/features/FeatureBus.h`)** loses,
  for three concrete, source-verified reasons: (1) it is **single-writer by
  construction** — `Writer` is move-only and a second `createWriter()` call
  while one is live deterministically returns an invalid handle
  (FeatureBus.h:33-37, 93-97) — we have two legitimate writers, and bolting
  a second writer path onto that class contradicts its own documented
  invariant (R4). (2) it requires a **fixed-size, trivially-copyable POD**
  payload, pinned by `static_assert(sizeof(FeatureSnapshot) == 320, ...)`
  (FeatureBus.h:129-136) — `cachedValues_` is a `vector<float>` whose class-
  level doc comment (SignalRegistry.h:12-13) explicitly says "Users can
  add/remove modulation signals," i.e. dynamic sizing is stated design
  intent even though currently unwired (0 callers). (3) FeatureBus's own
  header says outright: **"`std::atomic_ref` is unavailable on this
  toolchain"** (FeatureBus.h:19-21) — that's the exact tool you'd otherwise
  reach for to seqlock-protect an *existing* `vector<float>` without
  changing its element type, and it's off the table here.
- **A `std::mutex` (matching `EffectChain::effectsMutex_`)** is the real
  runner-up, not a clear loser — it would work and needs no more files
  touched than the atomics approach (same single choke point:
  `evaluateAll()`+`getCachedValue()`). It loses on margin only: it adds
  lock/unlock pairs to the GL hot path contending with two other thread
  families, for a coherency guarantee ("one thread's whole 32-element pass
  finishes before the other's starts") that doesn't actually survive contact
  with the callers anyway — `RoutingEngine::processFrame` and the UI panels
  call `getCachedValue()` once **per route/per param**, i.e. many separate
  lock/unlock cycles per "frame," so a mutex does not give them a coherent
  whole-vector snapshot either without a bigger API change (a `snapshot()`
  method returning a full copy under one lock) that is out of scope here.
  Given it buys no more real coherency than atomics for meaningfully more
  machinery, atomics win.

**Recommended fix: per-element `std::atomic<float>`, relaxed ordering.**
This is not a new idiom for this repo — it is the exact mechanism
`ClipPositionSignal` (src/signal/ClipPositionSignal.h:44,
`std::atomic<float> currentPosition_{0.0f}`) already uses for "render thread
writes every frame, other code reads it for display, values are independent
scalars with no cross-field coherency requirement." `cachedValues_[i]` is
precisely that shape, index by index — each signal ("Volume", "Bass", "Mod
1", ...) is an independently meaningful scalar; nothing in this codebase
requires signal `i` and signal `j`'s cached values to come from the same
evaluation pass (they're rendered as separate bars/knobs, routed to separate
effect params one route at a time). Relaxed load/store is exactly what
removes the UB (no torn reads/writes, no data race) without pretending to
solve a whole-vector coherency problem the callers don't actually rely on
today.

1. `src/signal/SignalRegistry.h`:
   - add `#include <atomic>`
   - change:
     ```cpp
     std::vector<float> cachedValues_;
     ```
     to:
     ```cpp
     std::vector<std::atomic<float>> cachedValues_;
     ```
   - Public signatures (`evaluateAll`, `getCachedValue`, `addSignal`,
     `removeSignal`) are UNCHANGED.

2. `src/signal/SignalRegistry.cpp`:
   - `evaluateAll()` (line 154): change
     `cachedValues_[i] = signals_[i]->getValue(snapshot);` to
     `cachedValues_[i].store(signals_[i]->getValue(snapshot), std::memory_order_relaxed);`
   - `getCachedValue()` (line 163): change `return cachedValues_[i];` to
     `return cachedValues_[i].load(std::memory_order_relaxed);`
   - `initDefaults()`: **remove** the five `cachedValues_.push_back(0.0f);`
     statements at (current) lines 13, 34, 66, 72, 81 — `std::atomic<float>`
     is not MoveInsertable/CopyInsertable, so `push_back` will not compile
     against the new element type. In their place, add ONE statement at the
     very end of `initDefaults()` (after the `ClipPositionSignal` block,
     i.e. after all `signals_.push_back(...)` calls have run and
     `signals_.size()` is final):
     ```cpp
     cachedValues_ = std::vector<std::atomic<float>>(signals_.size());
     for (auto& v : cachedValues_)
         v.store(0.0f, std::memory_order_relaxed);
     ```
     (`vector`'s own move-assignment only moves the internal buffer
     pointer/size/capacity — it never move-constructs individual elements —
     so this compiles even though `std::atomic<float>` itself is neither
     copyable nor movable. Do the explicit `.store(0.0f, ...)` loop rather
     than relying on `vector(n)`'s value-initialization zeroing the atomics
     implicitly; it is guaranteed from C++20 on (this repo is C++20 per
     `CMakeLists.txt:4`), but the explicit loop removes any doubt for a
     reader and costs nothing.)
   - `cachedValues_.clear();` (line 6) — **leave as is**, `clear()` only
     destroys elements, no move/copy required, compiles unchanged against
     `vector<atomic<float>>`.
   - `addSignal()` (currently line 89, dead code, 0 callers, but public API —
     must still compile): replace
     ```cpp
     void SignalRegistry::addSignal(std::unique_ptr<Signal> signal)
     {
         signal->setId(nextId_++);
         signals_.push_back(std::move(signal));
         cachedValues_.push_back(0.0f);
     }
     ```
     with a rebuild-the-vector version (no `push_back` on the atomic vector):
     ```cpp
     void SignalRegistry::addSignal(std::unique_ptr<Signal> signal)
     {
         signal->setId(nextId_++);
         signals_.push_back(std::move(signal));

         std::vector<std::atomic<float>> newCache(signals_.size());
         for (size_t i = 0; i + 1 < newCache.size(); ++i)
             newCache[i].store(cachedValues_[i].load(std::memory_order_relaxed),
                                std::memory_order_relaxed);
         newCache.back().store(0.0f, std::memory_order_relaxed);
         cachedValues_ = std::move(newCache);
     }
     ```
   - `removeSignal()` (currently lines 92-104, dead code, 0 callers, but
     public API): replace the `cachedValues_.erase(...)` line with an
     equivalent rebuild:
     ```cpp
     bool SignalRegistry::removeSignal(uint32_t id)
     {
         for (size_t i = 0; i < signals_.size(); ++i)
         {
             if (signals_[i]->getId() == id)
             {
                 std::vector<std::atomic<float>> newCache(signals_.size() - 1);
                 for (size_t j = 0, k = 0; j < cachedValues_.size(); ++j)
                 {
                     if (j == i) continue;
                     newCache[k++].store(cachedValues_[j].load(std::memory_order_relaxed),
                                          std::memory_order_relaxed);
                 }
                 signals_.erase(signals_.begin() + static_cast<ptrdiff_t>(i));
                 cachedValues_ = std::move(newCache);
                 return true;
             }
         }
         return false;
     }
     ```
     Note: keep the doc-comment intent alive (SignalRegistry.h:12-13 says
     dynamic add/remove is a stated design goal) rather than deleting these
     methods — they're unreachable today so this is a compile-correctness
     fix, not a behavior change. (If Boris would rather just delete both
     methods since they are unreachable, that is a smaller diff and equally
     valid — flagging as a call for the builder/Boris, not re-deriving it
     myself since it's a product decision, not a correctness one.)
   - Recommend adding, near the member declaration or at the top of the
     .cpp, one more static assertion in the spirit of FeatureBus's own
     assumption-pinning style:
     ```cpp
     static_assert(std::atomic<float>::is_always_lock_free,
                   "SignalRegistry::cachedValues_ assumes lock-free float atomics");
     ```

That's the entire change. Zero edits to `SignalBar.cpp`, `Renderer.cpp`,
`ClipInspector.cpp`, `EffectStackView.cpp`, `MacroBank.h`,
`RoutingEngine.cpp`, `TestServer.cpp`, or any test file — they all call
`evaluateAll()`/`getCachedValue()`, whose signatures are untouched.

## FENCE — every file this lane will WRITE
- `src/signal/SignalRegistry.h`
- `src/signal/SignalRegistry.cpp`

No other file under `src/` or `tests/` is to be written by this lane.

## TRAPS
- **The naive type swap does not compile.** Changing only the declaration
  (`vector<float>` → `vector<atomic<float>>`) without also rewriting
  `initDefaults()`, `addSignal()`, and `removeSignal()` fails to build,
  because `push_back`/`erase` require `std::atomic<float>` to be
  MoveInsertable, which it is not. This will look like an unrelated
  compile error deep in template instantiation (`vector::push_back` /
  `vector::erase` SFINAE failure) if the builder doesn't expect it.
- **`vector<atomic<T>>` whole-object move/assign is fine; per-element
  move/copy is not.** Don't be tempted to "fix" the push_back sites by
  writing a loop that copy-constructs `atomic<float>` elements one at a
  time into a pre-sized vector via `emplace`/assignment-from-another-atomic
  — assigning `atomic<float>` FROM another `atomic<float>` (as opposed to
  from a plain `float`) is also deleted. Always go through `.load(...)` /
  `.store(...)` to move a value between atomics.
  Whole-vector move-assignment (`cachedValues_ = std::move(newCache);`) is
  the only vector-level operation used, and that's fine (verified: vector
  move-assignment transfers the internal buffer, it doesn't move elements).
- **Don't relax the fix to "just add a mutex around the whole class"
  half-heartedly and also leave the atomic-vector idea half-applied** — pick
  one mechanism. Mixing (e.g., atomics AND a mutex) is redundant and
  confusing; this packet recommends atomics only.
- **`RoutingEngine::processFrame` / UI panels reading multiple signals per
  "frame" will still, occasionally, read a mix of a fresher GL-thread pass
  and a slightly-stale SignalBar-thread pass across different indices** —
  this is now data-race-free (no UB) but not perfectly coherent
  frame-to-frame. This is accepted, pre-existing, cosmetic-only behavior
  (each signal is an independent named scalar; nothing currently requires
  cross-signal coherency) — not a regression introduced by this fix, and not
  fixable without a much bigger API change (see THE CHANGE, mutex
  discussion). Don't let a reviewer mistake this for an incomplete fix.
- **A DIFFERENT, adjacent, already-existing race can still fire during the
  TSan proof run and must not be confused with a SIGRACE regression**: if
  whoever runs the TSan gate happens to also drag a Mod 1/Mod 2 knob in
  `SignalInspector` (setAmplitude/setPhaseOffset/setShape/etc., src/ui/SignalInspector.cpp,
  message thread) while the render thread's `evaluateAll()` is reading that
  same `OscillatorSignal`/`EnvelopeSignal`'s config fields, TSan may report
  a *different* race, at a *different* line
  (`OscillatorSignal::getValue`/`EnvelopeSignal::getValue`, not
  `SignalRegistry::evaluateAll`). That is real, pre-existing, and NOT
  addressed by this fix (see OUT OF SCOPE). Whoever runs the proof should
  read the TSan summary line's function/file name carefully, not just
  "did TSan print a warning."
- **Ownership/destruction order**: unaffected by this change —
  `cachedValues_` is a plain member destroyed with the rest of
  `SignalRegistry`, same as today; no new lifetime issues introduced.
- **`AUDIODNA_BUILD_TEST_SERVER` builds**: if the TSan proof build is
  configured with this flag ON, `TestServer::handleListSignals` becomes a
  third live reader thread family (httplib worker pool). The fix protects it
  automatically (same `getCachedValue()` choke point) — no separate action
  needed, but worth knowing so its absence-from-the-original-logs isn't
  mistaken for "it doesn't need protecting."

## HOW TO PROVE IT WORKS
1. Rebuild the existing `build-tsan` tree (already configured per
   `cmake/Sanitizers.cmake`, `-DADNA_SANITIZE=thread`) — do not create a new
   sanitizer dir, don't run cmake/ctest yourself (recon is read-only; this is
   for the builder/human).
2. Run the app from that build with the signal bar visible and live audio
   playing (so `SignalBar`'s 30 Hz timer and `Renderer`'s GL thread are both
   actively calling `evaluateAll()`/`getCachedValue()` concurrently — the
   exact condition both captured logs were taken under).
3. **Gate**: grep the TSan output for
   `SignalRegistry::evaluateAll` / `SignalRegistry.cpp:154` /
   `SignalRegistry::getCachedValue` / `SignalRegistry.cpp:163` — none of
   these should appear in any `WARNING: ThreadSanitizer: data race` block or
   `SUMMARY:` line. Compare directly against
   `.harmony/tsan-s-rta-0904-L7.log`'s two `SUMMARY: ThreadSanitizer: data
   race SignalRegistry.cpp:154 in SignalRegistry::evaluateAll(...)` lines —
   those exact two summaries should be gone.
4. Leave the app running at least as long as the previous captures did
   (both prior logs caught the race within the first couple of seconds after
   the signal bar became visible — this is a hot, frequent race, not a rare
   one, so a clean short run is meaningful signal, not just luck).
5. **What only a human can see**: watch the signal bar strips (Volume, Bass,
   Mid, etc., and the two Mod knobs) with music playing, both with the
   preview panel visible (GL thread actively rendering) and with it hidden
   (GL thread idle/detached, if that's reachable in this build) — the bars
   should keep animating smoothly in both cases (proving SignalBar's
   independent-writer keep-alive behavior was preserved, not accidentally
   removed) and should show no visible "stuck"/frozen values or extreme
   flicker (a crude visual proxy — not a substitute for the TSan gate — for
   the "torn frame" cosmetic caveat noted in TRAPS staying within acceptable
   bounds).
6. Separately, if the human also happens to twiddle a Mod knob during the
   same run and TSan reports a NEW warning at `SignalInspector.cpp` /
   `OscillatorSignal::getValue` / `EnvelopeSignal::getValue`, that is the
   adjacent, out-of-scope hazard noted above firing for the first time it's
   been observed — it is not evidence the SIGRACE fix failed. Don't close
   this lane's ticket over it; open a new one.

## OUT OF SCOPE
- **`SignalInspector`'s unsynchronized writes to `OscillatorSignal`/
  `EnvelopeSignal` config fields** (`amplitude_`, `phaseOffset_`, `shape_`,
  `beatDuration_`, `curveType_`, `oneShot_`, `looping_`) racing against the
  GL thread's `getValue()` reads of those same fields. Verified as a real,
  unsynchronized source-level hazard; not verified as ever having fired in
  TSan (not in either provided log). Named here so it isn't lost; not fixed
  by this packet because it's a different set of member variables on a
  different set of classes (`OscillatorSignal`, `EnvelopeSignal`), not
  `SignalRegistry::cachedValues_`, and deserves its own design pass (likely
  the same atomic-per-field treatment, but each field has different natural
  types — `WaveShape` and `CurveType` are enums, `points_` is a
  `vector<ControlPoint>` which cannot trivially become atomic — this is a
  bigger lane, not a one-line addendum to SIGRACE).
- **`ApiServer`'s unused `SignalRegistry&`** (src/api/ApiServer.h:111) — it's
  held but never read from; not a race contributor, not touched here. If a
  future `/api/signals`-style endpoint gets added to `ApiServer` itself
  (as opposed to `TestServer`, which already has one), it will pick up this
  fix's protection for free through `getCachedValue()`.
- **Making `addSignal()`/`removeSignal()` actually reachable from any UI**
  (the class doc comment implies this was/is intended but nothing currently
  calls them). Out of scope — this packet only makes them continue to
  compile correctly against the new member type; it does not wire them up.
- **Rebuilding/running the app, cmake configure, or ctest** — explicitly
  disallowed for this recon lane; left for the builder.

## OPEN QUESTIONS
- Should `addSignal()`/`removeSignal()` be rewritten (as specced above) to
  keep compiling, or simply **deleted** since they are unreachable dead code?
  Both are valid; I've specced the rewrite (preserves the class doc comment's
  stated intent) but flagged the deletion as an equally reasonable, smaller
  alternative. This is a product-taste call, not a correctness one — left
  for the builder or Boris to pick.
- Is `AUDIODNA_BUILD_TEST_SERVER` ON in whatever build configuration will
  actually be used for the TSan proof run? I could not determine this from
  source alone (it's a CMake cache variable, not a repo default I could find
  pinned anywhere) — doesn't change the fix, but changes whether
  `TestServer::handleListSignals` is actually exercised during the proof.
- Confirmed C++20 (`CMakeLists.txt:4`) so `std::atomic<float>`'s default
  constructor value-initializes to `0.0f` per the standard, and
  `std::atomic<float>` is expected lock-free on arm64/x86_64 — I did not
  verify `std::atomic<float>::is_always_lock_free == true` by compiling on
  this exact toolchain (recon is read-only, no build attempted); the
  recommended `static_assert` in THE CHANGE will catch it immediately at
  configure/compile time if that assumption is ever wrong on some future
  target.
