# Critic: finalize-truncation-plan.md — hostile review against source

Reviewer seat (Harmony council), 2026-09-25. Read finalize-truncation-plan.md in full and
re-derived every cited line against main @ HEAD (AudioTap.cpp/.h, RecorderHost.cpp/.h,
AudioStore.cpp, CombinedCallback.h, ApiServer.cpp, MainComponent.cpp, probe-step3.sh,
tests/CMakeLists.txt, test_recorder_host.cpp, test_audio_tap_sync.cpp, Take.h). Every
line-number citation in §1–§2 of the plan checked against actual source and found accurate
(AudioTap.cpp:150-221/223-269/285-311/343-381/440-500 verbatim match; AudioStore.cpp:211-218
verbatim match; RecorderHost.cpp:284-311/719 verbatim match; ApiServer.cpp:1240-1253 confirms
handlePerfStop answers `jsonOk()` before the `callAsync`'d `onPerfStop()` runs, and
MainComponent's `onPerfStop = [this]{ perfStop(); }` discards perfStop()'s returned string —
plan's "REST has NO channel for a finalize problem" is correct). probe-step3.sh:301-302 is
byte-for-byte the vacuous check the plan describes. The mechanism proof (§1, §2) is sound and
I found no error in it.

VERDICT: **REQUEST CHANGES** — not because the mechanism is wrong, but because **F1 as
literally specified reintroduces a narrower, silent variant of the exact defect it fixes**: a
push() that has already entered (busy==true) but has not yet reached its `running_` check can
now observe the new early `running_=false` store and drop a genuinely-delivered device block
with **zero trace** — no `framesWritten_` increment, no spill, no `droppedFrames_`, no
`unreliableFrom`, no truncation warning. This is reachable in principle, contradicts the plan's
own justification text verbatim, and the plan's own F3 test does not — cannot, as positioned —
catch it.

---

## BLOCKER — F1's "exclusive access after the wait" claim is false for a push already past `busy=true` but before its own `running_` check

**Evidence chain, all VERIFIED against source:**

1. `AudioTap::push()` AudioTap.cpp:159 `ScopedBusyFlag busy(audioThreadBusy_)` sets
   `audioThreadBusy_ = true` (seq_cst, line 19) **before** anything else in the function. The
   `running_` check the plan changes to seq_cst is at :190 — several instructions *later* in
   the same function (armed_.exchange at :180 sits between them). In production (debug hooks
   `#if`'d out) that gap is ~2 atomic RMWs, but it is real instruction distance, not zero.

2. F1 inserts `armed_.store(false, seq_cst); running_.store(false, seq_cst);` **before** the
   existing `activeWriter_.store(nullptr, seq_cst)` at the very *top* of `stopInternal()` —
   i.e. before the busy-wait loop even begins (plan lines 167-180). These three stores execute
   unconditionally, with no dependency on `audioThreadBusy_` at all.

3. Consequence: consider a push() that set `busy=true` (step 1) on the audio thread, and
   *before it reaches its own `running_` load at :190*, `stopInternal()` (message thread) is
   entered and runs its three top-of-function stores (step 2 — nothing gates them on `busy`).
   This is not the "already past that check, in flight" case the plan's own inserted comment
   describes (report lines 170-172: *"A push() already past that check is in flight
   (audioThreadBusy_==true) and is waited out below"*) — that comment is true only for pushes
   that already **committed** to proceeding by reading `running_==true`. It is silent about
   pushes that are `busy==true` but have **not yet reached** that read. Those exist and are the
   textbook definition of "in flight."

4. What happens to that push when it resumes and reaches :190 (now seq_cst): it observes
   `running_==false` (already set in step 2) and returns 0 at AudioTap.cpp:190-191 —
   **before ever calling `writeFrames()`** (:219). `framesWritten_` (:226) is never touched,
   `spillIntoPending()`/`droppedFrames_`/`hasUnreliableFrom_` are never touched. Meanwhile
   `CombinedCallback::audioDeviceIOCallbackWithContext` (CombinedCallback.h:129-134, VERIFIED
   read) advances `deliveredSamples_` by `numSamples` **unconditionally**, regardless of what
   `push()` returned. Net effect: one real, hardware-delivered 512-frame block is discarded
   with **no counter anywhere disagreeing** — `framesWritten_` and the WAV header stay
   perfectly consistent (both simply never saw it), so `AudioStore::finalize`'s truncation check
   (AudioStore.cpp:211, unchanged by this plan) **cannot and will not fire**. This is *strictly
   less observable* than the bug being fixed: today's defect at least announces itself via the
   "truncated" log line and (post-F4) `lastFinalizeError`; this one announces nothing.

5. **This is not a hypothetical I invented in isolation — it is demonstrable against the
   existing test the plan itself says must "stay green."** `tests/test_audio_tap_sync.cpp:622-686`
   ("s168 review: UAF race") already builds exactly this shape: a *second* push is parked at
   `debugArmPushBlockForTest()`'s hook, which sits at AudioTap.cpp:166, **immediately after**
   `ScopedBusyFlag busy(...)` (:159) and **before** the `armed_`/`running_` checks (:180/:190) —
   i.e. exactly the "busy==true, not yet at the running_ check" window. Pre-fix, when that push
   is released it still sees `running_==true` (stopInternal doesn't touch it until :499, long
   after this exact push has finished and the busy-wait has moved on to drain+reset), so its 512
   frames are spilled and drained correctly (window (a), the safe case the plan relies on).
   Under F1 as specified, by the time this parked push is released, `stopInternal()` (which was
   entered *after* the push was already parked, exactly the "in flight" premise) has *already*
   executed its new top-of-function `running_=false` store (nothing in F1 makes that store wait
   for this push — only the *later* busy-wait does, and by then the store already ran). The
   released push now observes `running_==false`, returns 0, and its 512 frames vanish untracked.
   The existing test does not currently assert on `framesWritten()` or WAV content (it only
   checks that `stop()` didn't race the destructor), so it stays green either way — **which is
   itself the "a test that would pass without the fix" failure mode the dispatch asked me to
   hunt for, just inverted: it's a regression the test suite is blind to, not a fix the test
   suite passes vacuously.**

6. The plan's own line 175-177 states the design intent as fact: *"Everything after the wait
   therefore runs with EXCLUSIVE access to the audio-thread-only fields."* That claim is false
   for the fields touched by a push caught in the window above — not because such a push
   corrupts shared state (it doesn't; it just returns 0), but because the *decision itself*
   (count-or-not) is settled by a race the wait was supposed to make irrelevant, and the losing
   outcome is unrecorded audio content.

**Amendment (concrete, not just "go think about it"):** the two new stores must not execute
*before* the (existing) busy-wait; they must execute *between* the existing busy-wait and the
drains — i.e. keep `activeWriter_.store(nullptr, seq_cst)` and the busy-wait exactly where they
are today (AudioTap.cpp:445, :466), and insert `armed_.store(false, seq_cst);
running_.store(false, seq_cst);` **immediately after the busy-wait loop exits, before the
`if (threadedWriter_)` drains at :469** — not at the top of the function. This preserves window
(a)'s existing guarantee in full (any push that was genuinely busy when the wait started has, by
construction, already made its `running_` decision on the *old* value and finished by the time
the wait exits, so its data is spilled and will be drained exactly as it is pre-fix) while still
shrinking window (b) — a *new* push starting strictly after the wait already confirmed
`busy==false` — from the writer-teardown's hundreds-of-microseconds-to-milliseconds span (the
observed ~9%/~0.9 ms) down to the width of two atomic stores (nanoseconds). This does not
mathematically reduce window (b) to zero (a push could still start in the store-store gap), so
if a provably-zero window is required, a second busy-wait immediately after these two stores and
before the drains (closer to the plan's own rejected "Alternative B," which this review's
analysis suggests is *more* correct than the plan credits, not less) closes it completely: the
plan's stated objection to B — "the first drain still races a concurrent push on the non-atomic
fields" — does not apply if the drains are placed after *both* waits rather than between them.
The plan dismissed B primarily on line-count grounds; given the finding above, B (or the
single-relocated-store variant here) should be the recommended path, not A as specified.

**Test gap this creates, independent of F3:** Test A (F3) cannot catch this — it calls
`tap.push()` directly from the test's own thread at a point *after* the pause hook (which sits
after both drains, deliberately late), so it never exercises a push racing the *top* of
`stopInternal()`. Catching the regression above requires extending the existing UAF-style test
(parking a push at AudioTap.h's existing `debugArmPushBlockForTest()` hook — the one at
AudioTap.cpp:166, before the armed/running checks — then calling `stop()` concurrently and
asserting, after release, that `framesWritten()` reflects that push's frames AND the WAV header
matches). This test does not exist today and is not in the plan's F3/F4 list.

---

## MAJOR — none beyond the above; the mechanism, F2 hook placement, F4 visibility plumbing, and F5/F6 probes are each individually correct against source

Specifically checked and found sound:
- Every `Status` field name added in F4 (`lastFinalizeError`, `finalizeErrors`) is genuinely
  absent from `RecorderHost.h`'s current `Status` struct (RecorderHost.h:188-229, confirmed by
  grep) — additive, no collision.
- `disarm()`'s current code (RecorderHost.cpp:305-311) really does drop `fin.error` into a
  local and never touches `lastError_` — F4 item 3's replacement is a correct, minimal fix for
  the visibility gap in §2, independent of the BLOCKER above.
- `tests/CMakeLists.txt`'s `test_recorder_host` target (lines 1051-1096, confirmed) really does
  not define `AUDIODNA_AUDIOTAP_TEST_HOOKS` today and really does compile its own
  `AudioTap.cpp` translation unit (line ~1065) — F4 item 7's "reaches only this test binary"
  claim is correct.
- `AudioStore::referencing()` (AudioStore.cpp:465-483, confirmed) computes
  `ref.unreliableFrom = *asset.unreliableFrom + firstSample`, matching Test B's asserted
  `*loaded->audio.unreliableFrom == tap.firstSample() + 5120` exactly.
- No RT violations in the diff as specified: the only audio-thread-reachable change is one
  `memory_order` token on one `running_.load()` (AudioTap.cpp:190) plus a wait-free
  seq_cst load already paid for by the existing busy flag machinery — no new allocation, lock,
  or syscall on the hot path either as specified or under my amendment above.
- The "arm-capture race" the plan flags (report lines 553-556, 563-565) as the reason the
  trailing `armed_`/`running_` stores at :498-499 must stay is real and correctly reasoned
  (verified: `push()`'s `armed_.exchange(false, acquire)` at :180 unconditionally sets
  `running_ = true` (relaxed) at :183 if it wins the exchange, which can race a `stopInternal()`
  that started concurrently) — but note this scenario is a *subset* of the same "busy==true
  before running_ is settled" family as the BLOCKER; whichever store-position amendment is
  chosen for F1, the trailing stores' role in resolving it should be re-verified against the new
  ordering, not assumed unchanged.

## MINOR — a disclosed-but-unresolved `lastError` gap the dispatch explicitly asked me to hunt for

RecorderHost.cpp:327-329 (confirmed): if `take.save(takeFolder_)` fails at stop, only
`dispatch.notify("could not save take.json at stop")` fires — `lastError_` is never set, so
`/api/perf/status` has no channel for this failure either, identical in shape to the bug F4
fixes for finalize errors. The plan already discloses this verbatim (report lines 575-577,
"Optional O1") and correctly scopes it out of the required fix. Recommend folding O1
(`lastError_ = "could not save take.json at stop";` at RecorderHost.cpp:328) into the same
commit since it's two lines, same pattern, and the dispatch explicitly asked for missed
`lastError` paths — but this is not a blocking omission since the plan names it.

## Live-check / F5 note (MINOR)

F6's `probe-finalize-loop.sh` and F5's new probe-step3.sh rows are statistically sound for
catching **window (b)** at its observed ~9% rate (0/40 bounds residual risk <7.2% at 95%, matches
the plan's own math) but are not the right instrument for the BLOCKER above — its window is
~10^5–10^6× narrower, so N=40 or even N=100 live cycles will almost certainly show 0 occurrences
whether or not the regression is present. The deterministic extended-UAF-test recommended above
is the only check that can actually falsify it; a clean live loop must not be read as clearing
this finding.

---

## Strongest counterargument to this critique, and why I still hold it

The window I found is astronomically narrower than the one being fixed (nanoseconds vs. the
observed ~0.9 ms), so in purely probabilistic terms it may never manifest in this application's
lifetime, and "the fix makes an already-rare bug even rarer while adding an unmeasurably-rarer
one" could reasonably be judged an acceptable trade by the project's own risk calculus (P(wrong)
× cost × difficulty-of-detection — the probability term here is vanishingly small). I hold the
BLOCKER anyway for three reasons specific to this codebase's own stated bar, not mine: (1) the
plan's own inserted code comment asserts unconditional exclusivity as a *proven* invariant, not
a probabilistic one — that assertion is what I'm disputing, not the practical risk; a wrong
"VERIFIED" comment in this exact file is precisely the class of defect the s168 review it's
patterned after was written to eliminate. (2) The amendment costs nothing extra in the diff (it
is a *placement* change, not new code) — there is no complexity/robustness trade being declined
here, so "it's rare enough" isn't buying anything. (3) The resulting failure mode is strictly
worse than the one being fixed (undetectable vs. logged-and-flagged), which matters more than
its rarity for a tool whose whole premise (Ruling 28, never-delete audio) is that recorded audio
is precious and its integrity should never silently degrade.
