# Reviewer Verdict — SIGRACE
STATUS: DONE
VERDICT: PASS

## Synchronisation contract the new code actually provides (from source, verified)
`cachedValues_` changed from `std::vector<float>` (SignalRegistry.h:50) to
`std::vector<std::atomic<float>>`. Every write and read now goes through
`.store(v, memory_order_relaxed)` / `.load(memory_order_relaxed)`
(SignalRegistry.cpp:169, :178). Guaranteed by `static_assert(std::atomic<float>::
is_always_lock_free, ...)` (SignalRegistry.cpp:3-4) that this is a true lock-free
atomic, not a mutex-emulated one.

Writers: `Renderer::renderOpenGL` (GL thread, Renderer.cpp:231) and
`SignalBar::timerCallback` (message thread, juce::Timer @30Hz, confirmed
`class SignalBar : ... private juce::Timer` + `startTimerHz(30)` at
SignalBar.h:15/SignalBar.cpp:34) both call `evaluateAll()`, which does
per-index `store(..., relaxed)`. Readers (6 call sites, independently
re-grepped, matches packet exactly: SignalBar.cpp:122, ClipInspector.cpp:848,
EffectStackView.cpp:179, TestServer.cpp:768, RoutingEngine.cpp:89,
MacroBank.h:74) all go through `getCachedValue()`'s single `load(relaxed)`.

Guarantee provided: **per-index atomicity only** — each `cachedValues_[i]` is
individually torn-free and race-free. No ordering is established between
different indices, and none is needed: `Signal::getValue()` is verified pure
(computed only from the passed-in `FeatureSnapshot` and each signal's own
read-only config), so there is no producer/consumer payload beyond the float
itself — relaxed is the correct and cheapest sufficient ordering, not merely
the cheapest available. Bounded staleness (not a snapshot, not a seqlock):
a reader pulling several indices across one "frame" (RoutingEngine iterating
routes, UI panels iterating signals) may see a mix of a fresher GL-thread pass
and a slightly stale SignalBar-thread pass across different i/j — this is
inherited pre-existing behavior (each signal is an independently meaningful
scalar; nothing in the codebase requires cross-signal coherency), not a
regression, and is explicitly and correctly flagged as such in the packet.

## Is the race eliminated, or just narrowed?
**Eliminated**, not narrowed. This is standard-conforming: `std::atomic<float>`
load/store from multiple threads on the same object is defined behavior with
no data race, full stop — there's no window being narrowed, there's no UB left
to hit. Verified two ways beyond reading the diff: (1) reproduced the exact
`vector<atomic<float>>` rebuild pattern used in `addSignal`/`removeSignal`
(whole-vector move-assign vs. per-element load/store) in an isolated
`-std=c++20 -fsanitize=thread` build outside the repo — compiled clean, ran
clean, confirms `std::atomic<float>::is_always_lock_free` holds on this exact
toolchain (Apple clang 16, arm64, matches project's `CMAKE_CXX_STANDARD 20`).
(2) Both provided TSan logs were re-grepped independently and confirmed to
show exactly 2 race-pairs each, both anchored at the pre-fix
`SignalRegistry.cpp:154` (write, now line 169) / `:163` (read, now line 178) —
i.e. exactly the two statements this diff converted to atomic ops, nothing
adjacent.

## Memory ordering
Correct and cheapest correct choice — see contract above. No lock, no
seqlock, so no contention/livelock questions apply on the per-frame GL path
(this was explicitly and correctly the deciding factor against a mutex in the
packet's own tradeoff analysis, which I independently re-derived and agree
with: RoutingEngine/UI panels call `getCachedValue()` once per route/param,
so a mutex wouldn't even buy whole-vector coherency without a bigger
`snapshot()`-returning API change that's out of scope).

## Completeness sweep (done independently, not taken on the packet's word)
Re-ran every grep in the packet myself against live source (`cachedValues_`,
`evaluateAll`, `getCachedValue`, `addSignal`, `removeSignal`) — identical
results. Additionally swept `getSignal`/`getSignalAt`/`getSignalByName`/
`getSignalsByCategory`/`getNumSignals` myself (packet asserted `signals_` is
frozen post-init but I did not want to take that solely on trust): confirmed
zero GL-thread callers of any of these touch `signals_` directly — the GL
thread only ever touches `SignalRegistry` via `evaluateAll()` and, one line
later via `RoutingEngine::processFrame`, `getCachedValue()` (grepped
Renderer.cpp/RoutingEngine.cpp/MacroBank.h directly). All `getSignalAt`/
`getNumSignals`/`getSignal` callers are message-thread UI panels or
TestServer's httplib pool, reading a vector that only ever gets structurally
mutated by `initDefaults()` (single call, MainComponent ctor, before any
other thread exists) — `addSignal`/`removeSignal` confirmed zero callers by
my own independent grep, matching the packet. Also confirmed independently
that no code anywhere copy-constructs or copy-assigns a `SignalRegistry`
(everyone holds `SignalRegistry*`/`&`; `MainComponent::signalRegistry_` is
the sole instance) — relevant because `vector<atomic<float>>` made the class
implicitly non-copyable, and this confirms that's a no-op change, not a
latent compile trap missed by the packet.
No other shared member of the class, and no other reader/writer anywhere in
the tree, is left unprotected. `nextId_`/`signals_` have the same
init-once-then-frozen lifecycle as before and were never part of the active
race.

## Dead-but-public API (addSignal/removeSignal)
Traced both by hand against the new atomic-vector representation (not just
"does it compile"): `addSignal`'s copy loop (`i + 1 < newCache.size()`)
correctly migrates the old N-1 values into the first N-1 slots of the new
N-sized vector and zeros the appended slot; `removeSignal`'s skip-index loop
correctly compacts N values down to N-1 preserving order. Both are logically
equivalent to the old `push_back`/`erase` behavior for every size, including
the N=0→1 edge case. **Verdict: correct, not merely compiling.**

One real, non-blocking observation the packet doesn't call out explicitly:
if either function is ever wired up to a live caller, calling it concurrently
with `evaluateAll`/GL-thread reads would open a **new**, unrelated race — on
`signals_` itself (a plain `vector<unique_ptr<Signal>>`, `push_back`/`erase`
can reallocate/invalidate while another thread iterates `signals_.size()`/
`operator[]`). This isn't a regression (the old code had the identical
property — `signals_` was never protected either), and it's dead code today
so it's not blocking, but it's the same shape of trap the task asked about:
a future caller who reads "still compiles, logic is right" could reasonably
assume it's now safe to wire up post-SIGRACE, and it would not be. Suggest a
one-line comment on both declarations (SignalRegistry.h:24-25) noting they
are not safe to call concurrently with evaluateAll — cheap, prevents the trap.

## Behaviour preservation
No change. All 5 `cachedValues_.push_back(0.0f)` call sites collapsed into a
single post-loop bulk-init (SignalRegistry.cpp:82-84); nothing reads
`cachedValues_` during the intervening construction, so this is a pure
refactor. Signal count, order, ids, categories, and default values are
byte-for-byte identical to before. `cachedValues_.clear()` (line 9) is
untouched and still valid (clear() only destroys elements, no
copy/move required).

## Fence check
`git diff --name-only` on the live tree shows churn in
`src/model/Composition.h`, `src/model/Deck.h`, `src/render/Renderer.cpp`,
`src/render/Renderer.h`, `tests/test_composition.cpp`, plus an untracked
`src/core/CompositionLoad.h` — all confirmed NOT this lane's (verified the
Renderer.cpp hunks land at lines 985-1021, video/image-sequence opening,
nowhere near the evaluateAll call site at ~231). This lane's diff touches
only `src/signal/SignalRegistry.cpp` and `src/signal/SignalRegistry.h`, and
that diff is a clean superset match of the work packet's spec, line for
line. No attribution of other-lane churn made or implied.

## TSan falsifier (stated before result, per request)
**Expect:** a fresh TSan run under the same repro (signal bar visible, audio
playing, same or longer duration than the two prior captures, both of which
caught it within seconds) shows NEITHER of the two exact prior summaries —
`SUMMARY: ThreadSanitizer: data race SignalRegistry.cpp:154 in
SignalRegistry::evaluateAll(...)` and its paired read at line 163 — grep for
`SignalRegistry` in the TSan output should return zero `WARNING`/`SUMMARY`
hits.
**Would mean FAILED:** (a) either of those exact two summaries reappears at
those two statements — means the store/load didn't take effect (stale binary,
wrong build dir, or a build that silently fell back off `-DADNA_SANITIZE=thread`);
(b) a NEW race appears inside `SignalRegistry.cpp` at a different
function/line — means a shared member was missed (I found none in my own
sweep, so I'd want to see the exact new line before trusting it's really new
vs. a mislabeled repeat); (c) a race appears in `SignalInspector.cpp` /
`OscillatorSignal::getValue` / `EnvelopeSignal::getValue` — this is the
already-identified, out-of-scope, pre-existing adjacent hazard (Mod-knob
config fields) and must NOT be treated as this fix having failed; don't
close or reopen SIGRACE over it. A livelock/hang is not a coherent failure
mode here — there is no retry loop, just plain store/load.

## SLIM
No excess introduced. Every changed line is required by the representation
change (TRAPS section's compile-failure trace for the naive type swap was
independently plausible and I have no reason to doubt it — `push_back`/
`erase` on non-MoveInsertable `atomic<float>` is a genuine SFINAE failure).
No dead code added; `addSignal`/`removeSignal` were already dead pre-diff,
kept compiling per explicit public-API requirement, not expanded in scope.

FILE: src/signal/SignalRegistry.h
  [OK] Readability/Patterns: type change is minimal and self-documenting;
       matches the repo's own existing per-field-atomic precedent
       (ClipPositionSignal::currentPosition_, verified by reading the file).
  [OK] Error handling: N/A, no new failure modes.

FILE: src/signal/SignalRegistry.cpp
  [OK] Concurrency: race eliminated, not narrowed (see analysis above);
       correct and cheapest ordering (relaxed).
  [OK] Completeness: independently swept, no other shared member or
       caller left unprotected.
  [OK] Behaviour preservation: verified no observable change to defaults/
       sizing/ordering.
  [ISSUE-suggestion, non-blocking] Dead-but-public API: addSignal()/
       removeSignal() (SignalRegistry.h:24-25) are logically correct under
       the new representation but still race on `signals_` itself if ever
       wired up concurrently with evaluateAll — add a one-line "not safe to
       call concurrently with evaluateAll()" comment to close the trap for
       a future caller.

SUMMARY: 2 files reviewed (SignalRegistry.h, SignalRegistry.cpp), 0 blocking
issues, 1 suggestion (non-blocking doc comment on dead API). Confidence:
VERIFIED — re-derived the call-graph and both TSan logs from source myself
rather than trusting the packet's grep output, and reproduced the
atomic-vector rebuild mechanism in an isolated compile+run outside the repo.

FILES: src/signal/SignalRegistry.cpp, src/signal/SignalRegistry.h
ISSUES: 0 blocking / 1 suggestion (comment addSignal/removeSignal as not concurrency-safe)
METADATA: reviewer=rev-SIGRACE, builder_packet=SIGRACE-signalregistry-race.md, date=2026-09-05
