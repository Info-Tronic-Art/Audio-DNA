## BUILDER REPORT — S166-L1 (single evaluator for SignalRegistry)

INBOX-RECHECK: 0 addenda folded

### STATUS
DONE

### RESULT
`SignalRegistry::evaluateAll()` now has exactly ONE call site: `MainComponent::tickFeaturePipeline`
(the 120Hz message-thread timer), placed after the snapshot read and before
`globalMacroBank_.updateValues(...)`. The GL-thread call in `Renderer::renderOpenGL()` and the
message-thread call inside `SignalBar::timerCallback()` are both deleted; `SignalBar` keeps its
30Hz repaint timer and reads `getCachedValue` only. The live data race the lane targets (GL thread
reading `Signal` objects the UI mutates on the message thread) is closed by confinement, not by
locking. The dead `routingEngine_.processFrame()` call in the same GL callback is also deleted
(decision + evidence below). Build exit 0, ctest 232/232 before -> 233/233 after on genuinely
clean serialized rebuilds (no concurrent builders, no running app instance).

### FACTS
- `src/MainComponent.cpp:3056-3072` (`tickFeaturePipeline`) — new call `signalRegistry_.evaluateAll(snap)`
  inserted after `const FeatureSnapshot snap = analysisThread_.getFeatureBus().read();` and before
  the existing `globalMacroBank_.updateValues(signalRegistry_);` (which the pre-existing "L9" comment
  in that same function already documents as needing fresh values this tick).
- `src/MainComponent.h:272` — `static constexpr int kMappingTickHz = 120;`; `MappingTickTimer`
  (`src/MainComponent.h:255-267`) runs `tickFeaturePipeline` unconditionally for the component's
  whole lifetime, independent of GL attach state (its own comment: "so mapped params keep updating
  even while this GL context is detached").
- `src/render/Renderer.cpp:226-236` (pre-edit) — deleted the `signalRegistry_->evaluateAll(snap)`
  block and the `routingEngine_.processFrame(*signalRegistry_, ...)` block; the surrounding `snap`
  read stays (it feeds autopilot/genre-detection/compositor code later in the same function,
  confirmed via `grep -n '\bsnap\b' src/render/Renderer.cpp`).
- `src/ui/SignalBar.cpp:111-127` (pre-edit) — deleted `displaySnap_ = featureBus_.read();` and
  `registry_.evaluateAll(displaySnap_);`; the strip-update loop (`getCachedValue` + `updateValue`)
  and `repaint()` are unchanged.
- `src/signal/SignalRegistry.cpp:166-176` — added
  `jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());` as the first line of
  `evaluateAll`, plus `#include <juce_events/juce_events.h>`. Exact form copied from
  `src/mapping/MappingEngine.cpp:11,20,46` (the A6 precedent), which is proven safe under headless
  ctest — `tests/test_mapping_engine.cpp` calls `MappingEngine::processFrame` (same jassert)
  directly with no `MessageManager` explicitly started, and the baseline suite passes.
- `src/signal/Signal.h:6-11` — class comment updated from "Signals are evaluated each frame on the
  render thread" to state message-thread confinement via `SignalRegistry::evaluateAll`.
- `tests/test_routing_engine.cpp:55-93` — new `TEST_CASE("SignalRegistry cache moves only on an
  explicit evaluateAll tick", "[signal]")`; no `tests/CMakeLists.txt` edit needed
  (`test_routing_engine` already exists and uses `catch_discover_tests`, confirmed
  `tests/CMakeLists.txt:254-287`).
- Two greps of different shape prove "nowhere else": `grep -rn "evaluateAll" src/` → declaration
  (`SignalRegistry.h:39`), definition (`SignalRegistry.cpp:166`), one call site
  (`MainComponent.cpp:3065`), and three explanatory comments (no other call). Second shape,
  `grep -rnE '(\.|->)evaluateAll\(' src/` → exactly one hit, `MainComponent.cpp:3065`.
- Commit `e49a6ad` on `main`, 6 files, +71/-24. HEAD before my work was `f674cc3` (docs-only on top
  of the packet's cited baseline `2e1afa3` — confirmed via `git show --stat f674cc3`, 2 `.harmony/`
  files, no source).

### METHOD
Read the work packet, `.harmony/gotchas.md`, and arch doc §4.2/§5 (L0-L1) in full before touching
anything. Traced every call site of `evaluateAll` and every reader of `signalRegistry_`/`snap` in
`Renderer.cpp` and `SignalBar.cpp` before deleting, to confirm nothing else depended on the deleted
reads. For the routingEngine deletion: two greps of different shape (`add_route` vs
`addRoute`/`route`) across `tests/` and `tests/visual/`, then read every hit's actual assertion
body (not just its existence) to determine whether any test's PASS/FAIL depends on
`routingEngine_.processFrame` executing. For the ctest before/after numbers: backed up my 5 edited
source files to the scratchpad with recorded md5s, reverted them to HEAD via `git show HEAD:path >
path` (never a whole-tree `git stash`/`checkout`, per the standing Iron Law — the tree had a
concurrent, unrelated `.harmony/idea-ledger.md` edit from another live agent at the time), rebuilt,
ran ctest (232/232), restored from the backups (md5-verified byte-identical), rebuilt, reran ctest
(232/232), then added the new test and got 233/233. For the new test's load-bearing proof:
temporarily made `SignalRegistry::getSignalAt` silently call `evaluateAll` (simulating the exact
"something else secretly evaluates" defect the test guards against), rebuilt, ran the test alone —
FAILED (3 passed / 1 failed, exit 42) — then restored `SignalRegistry.cpp` from the md5-recorded
backup, verified the hash, rebuilt, reran — PASSED (5/5, exit 0).

### CONFIDENCE+VERIFY
HIGH on everything build/ctest/grep-verified above — all of it is independently re-derivable from
the commands cited (re-run `cmake --build build --config Release`, `ctest`, or the two grep
patterns and get the same numbers). LOWER (structural, not behaviorally verified) on "signal
meters still move with the preview hidden" — I did not launch the app (packet forbids it; you run
the behavioral gate). What I can state from source: `MappingTickTimer`'s comment and its
constructor wiring show it already ran unconditionally pre-lane specifically to survive GL detach
(same mechanism this lane now also uses for `evaluateAll`), and `SignalBar`'s own 30Hz timer never
depended on GL state either — but the live meters-move check is yours to run.

### UNKNOWNS-NOT-DONE
Nothing in my fence is left undone. Two things outside my fence, both structural leftovers I did
not touch (not authorized to, and both harmless — see ISSUES): `Renderer::signalRegistry_`
pointer/setter/getter is now dead wiring inside `Renderer.h` (not in my fence); `SignalBar::featureBus_`
member is now unused (only `SignalBar.cpp` was in my fence, not `SignalBar.h`).

### NUANCE
The packet's suggested jassert form (`existsAndIsCurrentThread()`) would have been WRONG here —
see PACKET QUALITY. The routingEngine deletion is a judgment call built on "no test's assertion
depends on it" rather than "zero routes exist" — the route tests still exist and would still pass
(their assertions never checked the route's actual effect), they just now exercise a route that
gets added/listed but never applied. That is a pre-existing weakness in those tests (they are
explicitly labeled a not-yet-built "specification" in their own docstring), not something this
lane introduced.

### HANDOFF-NEEDS
None — DONE, not NEEDS_HANDOFF/NEEDS_SPECIALIST.

---

## SUMMARY
Closed the two-writer race on `SignalRegistry` per the packet's prescribed order: added the
message-thread call first, then deleted both old call sites, added the thread-confinement jassert,
fixed the stale `Signal.h` comment, and made the conditional routingEngine deletion (grepped first,
decided nothing depends on it, deleted). Added one Catch2 test proving the cache only moves on an
explicit tick, proven load-bearing by a real mutation-and-restore cycle.

## FILES CHANGED
- `src/MainComponent.cpp` — `tickFeaturePipeline`: added `signalRegistry_.evaluateAll(snap)` after
  the snapshot read, before `globalMacroBank_.updateValues(...)`.
- `src/render/Renderer.cpp` — deleted the GL-thread `signalRegistry_->evaluateAll(snap)` call and
  the `routingEngine_.processFrame(...)` call/lambda inside `renderOpenGL()`; replaced with an
  explanatory comment.
- `src/ui/SignalBar.cpp` — deleted `displaySnap_ = featureBus_.read();` and
  `registry_.evaluateAll(displaySnap_);` from `timerCallback()`; strip-update loop and `repaint()`
  unchanged.
- `src/signal/SignalRegistry.cpp` — added `#include <juce_events/juce_events.h>` and
  `jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());` as the first line of
  `evaluateAll`.
- `src/signal/Signal.h` — updated the class-doc comment (message-thread confinement, not render
  thread).
- `tests/test_routing_engine.cpp` — added
  `TEST_CASE("SignalRegistry cache moves only on an explicit evaluateAll tick", "[signal]")`.

## TESTS
- Build: `cmake --build build --config Release -j 8`, exit 0 (captured every time, never inferred).
- ctest BEFORE (clean rebuild at HEAD `f674cc3`, my 5 source files reverted via `git show
  HEAD:path`, no other lane's edits in the tree, no running app instance): **232/232, 3.0s**.
- ctest AFTER (my 5 files restored, byte-identical per md5, plus the new test added): **233/233,
  3.0s** (232 baseline + 1 new).
- New test alone, isolated run: `./build/tests/test_routing_engine "SignalRegistry cache moves only
  on an explicit evaluateAll tick"` — 5/5 assertions, exit 0.
- Load-bearing proof: mutated `SignalRegistry::getSignalAt` to call `evaluateAll` internally
  (simulating a "secretly evaluating" accessor) → same test → **FAILED, 3 passed / 1 failed, exit
  42** (assertion at what was line 87 of the diff, the mid-test check). Restored
  `src/signal/SignalRegistry.cpp` from the pre-mutation backup, verified md5
  `2ff1343b8ea576a2236f1b3217bf7491` matches exactly, rebuilt → **PASSED, 5/5, exit 0**.
- No app launch performed (per packet instruction — Harmony runs the behavioral gate).

## ISSUES
1. **routingEngine grep result and decision** (packet-required reporting item). Two greps of
   different shape: `grep -rn "add_route" tests/` (hits: `tests/visual/SIGNAL_TEST_SPEC.md`,
   `tests/visual/test_signals.py`, `tests/visual/vj_controller.py`); `grep -rn "addRoute" tests/`
   (hit: `tests/test_routing_engine.cpp:108,142,160`, which constructs its OWN local `RoutingEngine`
   + `SignalRegistry` and calls `.processFrame()` directly — does not go through `Renderer` at
   all, unaffected by this deletion). I then read every actual assertion in
   `tests/visual/test_signals.py`'s `TestSignalRouteEndToEnd` class (the only place a route is
   added through the SAME `RoutingEngine` instance `Renderer` owns — confirmed via
   `src/MainComponent.cpp:1779,1797` passing `previewPanel_.getRenderer().getRoutingEngine()` into
   `TestServer`). Every one of those tests either asserts a rendered file exists or that the HTTP
   response has `"ok": true` — none asserts that the route's parameter actually changed a rendered
   pixel (the one test whose docstring claims to check that, `test_create_route_volume_to_ripple`,
   explicitly says "exact PSNR check requires opencv" and then does NOT call the `psnr_between`
   helper already imported in the same file — it only checks file existence). The class's own
   header comment calls it "Require API endpoints (specification) — These define what to test once
   ... exist," i.e. it is pre-existing scaffolding for a not-yet-built feature, not a live
   assertion of route behavior. These tests are also not registered in `tests/CMakeLists.txt` (no
   `add_test` for `tests/visual/`), so they are outside the 232/233 ctest baseline entirely.
   **Decision: deleted the `routingEngine_.processFrame()` call.** Nothing depends on it for
   correctness of any test that currently runs or currently asserts anything meaningful about
   route effects.
2. **Arch doc's L1 scope is wider than my packet's WHAT TO DO list — flagged, not done.**
   `memory/.reports/s166/arch-universal-connection.md` §5, the "L1" lane entry (not §4.2, which the
   packet did quote in full) also says: "fix the one-bar fold in `OscillatorSignal.h:28-30` and
   `EnvelopeSignal.h:36-37` (add `4*barCount`)." My work packet's WHAT TO DO section (1-6) never
   mentions this, and those two files are not obviously covered by the fence's "SignalRegistry /
   Signal headers+sources for the jassert and the comment" wording (that phrase is scoped to "the
   jassert and the comment," not general bugfixes). I did not touch `OscillatorSignal.h` or
   `EnvelopeSignal.h`. Flagging per the packet's own "anything outside the fence you noticed"
   ask — this may be intentionally deferred to a later lane, or may have been dropped when the
   packet was authored from the arch doc; worth a decision either way before it's forgotten.
3. **Pre-existing, not caused by this lane:** `ApiServer.h:111` `signalRegistry_` private field is
   unused (`-Wunused-private-field`) — confirmed via `git diff --name-only` that `src/api/ApiServer.*`
   are not in my diff at all; this warning exists independent of my change.
4. **Unrelated concurrent activity observed, not touched:** `.harmony/idea-ledger.md` was modified
   (+26 lines) by another live process during my session (not by me — I never wrote to it); the
   `graphify-out/` tree is dirty from the repo's own documented post-commit background regen hook
   (fired after my commit, per the hook's own log line in the commit output). Neither is staged or
   committed by me.

## SKILL_PROPOSALS
None — this was a standard confined bug-fix lane; no new reusable procedure emerged beyond what
existing craft/gotcha guidance already covers.

## RISKS
- The routingEngine deletion (item 6 in the packet) removes the only place `RoutingEngine::processFrame`
  is invoked in the shipped app. `RoutingEngine`/`Route`/`addRoute` themselves are untouched (per
  fence — I did not delete the class, members, or TestServer endpoints), so `/api/add_route` still
  succeeds and `/api/routes` still lists routes; they simply no longer visibly affect rendered
  output. If a later lane (e.g. L3 in the arch doc) intends to replace `RoutingEngine` with the
  `ConnectionEngine` design, this is a clean, low-risk simplification in that direction. If instead
  someone expected the old routing feature to keep working meanwhile, this is a behavior change —
  flagged in ISSUES #1, not hidden.
- `jassert` in `evaluateAll` will fire (in debug builds, under a live `MessageManager`) if any
  future code calls `evaluateAll` off the message thread — this is the intended regression guard,
  not a risk, but worth noting it is now load-bearing for catching future confinement violations.

## METRICS
- Files changed: 6. Lines: +71/-24.
- Build invocations this session: 6 (initial full build, before-baseline full build, after-restore
  full build, isolated new-test build, mutant build, final full build) — all exit 0.
- ctest invocations: 3 full-suite runs (232/232, 232/232, 233/233) + 2 isolated single-test runs
  (FAIL 3/1, PASS 5/5).

## KNOWLEDGE CONTEXT
No `KNOWLEDGE_TOOLS` block in this packet. `graphify-out/` exists in this repo but the packet did
not name it as authoritative for this lane; I relied on grep + direct reading (small, well-defined
fence) per the "grep-only project" fallback. Impact authority: grep (not authoritative for "no
callers" claims) — applied conservatively for the routingEngine decision by reading every hit's
actual test body rather than trusting hit-count alone.

## PACKET QUALITY
- **Clarity: CLEAR.** The packet's ORDER, WHY, and FENCE sections were unusually precise and
  matched the actual code exactly (line-number drift aside, which the packet itself warned about
  and which held true — cited lines were off by ~1-4 lines from actual, consistent with the
  documented repo gotcha).
- **One packet-authored detail was wrong and had to be corrected against ground truth:** the
  packet's step 4 says `jassert(juce::MessageManager::existsAndIsCurrentThread())` but then says
  "match the precedent already used in MappingEngine.cpp — find it, copy its exact form." I did
  the latter; the actual precedent (`MappingEngine.cpp:11,20,46`) uses
  `juce::MessageManager::getInstance()->isThisTheMessageThread()`, not `existsAndIsCurrentThread()`.
  This distinction is load-bearing, not cosmetic: `existsAndIsCurrentThread()` returns `false` (and
  would trip the assert) when NO `MessageManager` instance exists yet — exactly the situation in
  the headless ctest binary that calls `evaluateAll` directly three times
  (`tests/test_routing_engine.cpp:42,91,130`). `getInstance()` auto-creates the singleton on first
  call instead, which is why the MappingEngine precedent is safe under ctest and
  `existsAndIsCurrentThread()` would not have been. Followed the packet's own "copy its exact form"
  instruction over its literal API name.
- **Missing context the packet didn't flag:** the arch doc's L1 entry (§5, not §4.2) includes the
  `OscillatorSignal.h`/`EnvelopeSignal.h` one-bar-fold fix that my packet's WHAT TO DO list omits —
  see ISSUES #2.
- **Unused context:** none — every section of the packet (order, fence, jassert instruction,
  routingEngine grep instruction, build rules) was exercised.
- **Self-brief files:** `.harmony/gotchas.md` (existed, useful — the shared-build-dir/one-instance/
  stale-binary/order-is-a-claim entries all directly shaped how I ran builds and made the ordering
  and routingEngine decisions) and the cited arch doc section (existed, useful, and revealed the
  scope gap in ISSUES #2).

### STATUS
DONE

## NEXT ACTION
None required from me. Suggest Harmony's behavioral gate specifically exercise "signal meters
still move with the preview hidden" (the packet's stated gate) and decide on ISSUES #2 (the
one-bar-fold fix scope gap) before it's lost between lanes.
