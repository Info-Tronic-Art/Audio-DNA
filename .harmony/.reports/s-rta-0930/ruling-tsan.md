# RULING lane tsan (s-rta-0930) -- the blind council's attacks on plan-tsan.md

Architect (Fable), 2026-09-30, main HEAD 655d232.

- Inputs: plan-tsan.md, plus the three seat papers, written verbatim to attack-tsan-papers.md.
- Labels: VERIFIED = read or ran today (file:line or measurement); INFERRED = follows from verified facts; ASSUMED = not checked.
- Scratch measurements (no app launched):
  - /var/folders/xk/4b_nszfx60n0g13f6x0c3vy40000gn/T/a16 (Apple clang 17.0.0, clang-1700.6.3.2);
  - the fresh-RED archive `<scratchpad>/sweep-red1-archive/runs`;
  - the TSan runtime's `help=1` output (the scratch a128/t_tsan binary).

VERDICT: AMEND. The lane is ready to build with the amendments in "ARCHITECT RULING" below. Tally of the 30 attacks:
- 23 accepted (several with an amended fix);
- 4 partial: M-A10, R-A6, G-A2, G-A4;
- 3 rejected: M-A3, M-A8, M-A9.

Two findings change the gate most:
1. Scenario d as planned cannot pass its own zero bar, whatever the lane does. Both causes are R5/R7, outside the lane:
   - perf/play's checkpoint-0 preamble writes five plain layer flags per layer, plus quantizeMode. VERIFIED: Program.cpp:254-279 feeds MainComponent.cpp:6336-6340 and :2010, and the GL thread reads those flags every frame.
   - GET /api/composition reads thumbnails, clip sizes, mediaMissing and effect params with no fence (ApiServer.cpp:436-472).
2. T6 removes the GL read of activeDeckIndex but leaves the field plain. The httplib reads (ApiServer.cpp:360 / 384 / 1540) then become its only other-thread access. INFERRED: this likely unmasks a NEW lane-only report (see M-A5).

## 1. Rulings per attack

### Seat tsan-memmodel

M-A1 [MUST] ACCEPT, amended (see amendment 3).
- True: R1's post-join range checks pass under per-field relaxed atomics (Fork 1a), so R1 cannot tell the cell from option (a).
- Fix: a NEW R4 (value invariants under a paced trigger storm, run in both builds) plus per-snapshot invariants in R1.
- Two parts of the seat's fix are dropped:
  - The invariant "previous >= 0 implies progress < 1" is REJECTED. It is false for every cut and every clear: a cut stores progress 1.0 with previous = the old active (Layer.h:276-278), and clearActiveClip does the same (Layer.h:337-339).
  - The pairing oracle is replaced by R4's cyclic-order invariant I2, which has the same power with no bookkeeping.

M-A2 [MUST] ACCEPT, amended (see amendment 5).
- True: applying the tail after the CAS gives the render no happens-before with it.
- Worst case found: the render's autopilot loads a tuple naming the clip the user just fired, reads that clip's STALE beatsPlayed (e.g. 7 from its last run), and its fetchAdd reaches the target. It then advances off the clip the user just fired (Autopilot.cpp:67-105).
- The same window exists on main (store order Layer.h:276-286).
- Fix: the activation tail goes BEFORE the CAS attempt that installs it; the clear tail stays after.
- Two parts of the seat's fix are dropped:
  - "Re-load and compare before applying" is REJECTED: it is itself a check-then-act.
  - "CAS the clear's playing = false against the tuple" is unnecessary. VERIFIED: no render path can re-activate a cleared layer. Autopilot needs an active playing clip (Autopilot.cpp:14-16, 67-69). processPendingTrigger needs pending >= 0 (:54), and the clear cancels pending in the same word.

M-A3 [MUST] REJECT.
- Undo is a value restore, not a read-modify-write. updateRuntime(r -> before_) ignores r, so it is an exchange: the same single linearizable write as setRuntime's release store of the 16-byte word. A CAS loop changes nothing.
- The restore supersedes every render transition since the gesture (fade tick, autopilot, fired pending). Main does exactly the same with five blind stores (DeckCommands.h:210-217).
- Residue on non-target clips is already accepted by the command's contract (TriggerCommands.h:38-50, spec risk #5). An autopilot clip left at playing = true after an undo is in the normal state of any clip that was activated and then left (Layer.h:284-286).
- "Ban setRuntime on live layers outside the fence" is also REJECTED:
  - runtime undo/redo is deliberately unfenced (TriggerCommands.h:34-36);
  - it is now a single atomic store.
- Accepted residue: one doc sentence (amendment 6).

M-A4 [SHOULD] ACCEPT (see amendment 6).
- True: sameIntent misfires in two windows between the live trigger and UndoManager::perform. Both re-apply after_ and restart or re-queue the trigger:
  - the fade ends (LayerClock.h:25-26);
  - the queued trigger fires (Layer.h:291-328).
- The window includes the preview reload (MainComponent.cpp:4661-4760).
- Fix: the first execute() is a no-op.
- VERIFIED safe: every production site live-applies before pushing (MainComponent.cpp:757-766, 4629-4652 / 4774, ~4876, 6910-6916). Every existing test does too (test_undo_commands.cpp:1128, 1377, 2196, 2277 / 2288, 2358, 2408, 2663 / 2672, 2795).

M-A5 [SHOULD] ACCEPT (see amendments 9 and 13).
- VERIFIED: the httplib thread reads the plain field in /api/status (ApiServer.cpp:360), /api/composition (:384) and /api/state (:1540); the writer is MainComponent.cpp:5525.
- INFERRED aggravation:
  - Every fresh-RED c launch ends with GET /api/state after four switch_deck writes, yet no ApiServer.cpp:1540 report exists (tsan-red-main-655d232.txt).
  - This is consistent with the GL thread's per-frame read (Renderer.cpp:471) evicting the write from TSan's shadow.
  - Once T6 removes that read, the write survives and the httplib read reports: a lane-only report.
- Fix: activeDeckIndex becomes Relaxed<int>, and scenario d drops /api/composition.

M-A6 [SHOULD] ACCEPT, scoped-goal option (see amendments 1, 12 and 15). The finding is stronger than the seat stated: the plan's own scenario d drives R5 fields.
- VERIFIED: perf/play's preamble emits visible / bypass / solo / mute / autopilot for EVERY layer, plus comp quantize (Program.cpp:254-279).
- These are applied by unconditional plain stores (MainComponent.cpp:6336-6340, 2010).
- Meanwhile the GL thread reads visible / bypassed / solo every frame (CompositorEngine.cpp:1048, 1069; DeckClock.h:20-25) and autopilotEnabled (Autopilot.cpp:11, 64).
- Widening T5 to all of R5 is REJECTED for this lane: dozens of fields and serializers, no RED evidence, more merge surface with bt2 / gop2.
- perf/* moves to INFO scenario e, which becomes the follow-up lane's baseline.

M-A7 [SHOULD] ACCEPT (see amendment 8).
- A lint that is RED on 655d232, where Renderer.cpp stores `playing` plainly at :1826, :1833, :1911 and :1918.
- Plus a hard requirement that syncMedia calls ClipTransportSync.h.

M-A8 [SHOULD] REJECT the epoch; ACCEPT one comment. The feared ABA cannot hurt:
- VERIFIED: the player's play state is written ONLY by syncMedia (the only setPlaying callers are Renderer.cpp:1788, 1790, 1834, 1877, 1879, 1919).
- VERIFIED: UI transport writes only the model (LayerStrip.cpp:373-391, ClipInspector.cpp:40-51, MainComponent.cpp:6392-6398).
- So inside the window the player holds `wanted`, or its own OneShot stop.
- After a pause + play (ABA), the CAS writes exactly what it would write with no UI activity. "The pause the player saw" cannot happen.

M-A9 [SHOULD] REJECT (measured). The compiler, not our assumption, picks the instruction:

| Target / flags | 16-byte atomic codegen |
|---|---|
| -O2 default, or -march=armv8-a | `ldp` / `stp` / `caspal` |
| -mcpu=generic (no LSE2) | `ldaxp` / `stlxp` exclusive-pair loops: still atomic and lock-free |
| x86_64-apple-macos11 | inline `cmpxchg16b`; is_always_lock_free = 1 |
| -O0 | no `___atomic` libcalls |

- The `__APPLE__` static_assert(is_always_lock_free) is the correct and sufficient guard.
- A non-LSE2 target changes only "a reader never writes the cache line": performance, not correctness.
- The (i, ~i) hammer is folded into R4, whose I2 catches a torn pair.

M-A10 [NIT] PARTIAL.
- ACCEPT: a live Layer copy joins R1, giving TSan coverage of the copy paths of the cell and of Relaxed<T>.
- REJECT acquire/release copies:
  - Layer declares `clips` (Layer.h:178) before the runtime block (:181-190), so an acquire on the cell cannot order the clip members already copied.
  - A live copy is per-field atomic, never a snapshot. Document it; use runtime() for a consistent tuple.

### Seat tsan-realtime

R-A1 [SHOULD] ACCEPT (see amendment 7).
- True for Bar / TwoBar / FourBar (Layer.h:315-323): after a give-up, the next call comes only at the next beat crossing (Autopilot.cpp:46-60).
- Fix: raise the bound to 16 and correct the claim.
- Latch REJECTED: new state for an event that needs 16 consecutive message-thread CASes on one layer inside about 1 us.

R-A2 [SHOULD] ACCEPT, same fix as M-A2 (amendment 5).
- "Apply only if the tuple still equals t.after" is REJECTED. Any fade tick changes progress, so under load the tail would be skipped, including the first auto-play.

R-A3 [SHOULD] ACCEPT as a fix (amendment 10; Boris Q3, default = fix).
- VERIFIED: MainComponent.cpp:7593 and :7632 gate the release on activeClipColumn == column. A still-queued momentary trigger therefore survives the release, and the clip latches on.
- The code's own L5 comment names this exact case as one that should cancel (Layer.h:341-349).
- The lane rewrites both sites anyway (plan T2).

R-A4 [SHOULD] ACCEPT (see amendments 12 and 13).
- Beat source without audio: POST /api/set_bpm enters Manual BPM, where the beat phase free-runs (probe-manual-bpm.sh:2-5, 22).
- Witness: new relaxed counters in /api/state (the house witness pattern).

R-A5 [SHOULD] ACCEPT (see amendments 3 and 8).
- The render-side violation counter lives in R4.
- The reviewer grep becomes a pinned-count lint.
- A production load counter is REJECTED: it would instrument the hot path.

R-A6 [SHOULD] PARTIAL.
- ACCEPT: G4 adds the already pre-registered retrigger rows:
  - probe-media-open m6_retrigger_seek (probe-media-open.py:47-49);
  - probe-video w3_retrigger_midgop_1080 and w6b_retrigger_mid_fade (probe-video.py:45, 65).
- REJECT the FakePlayer seek test. It would test the fake:
  - the restart itself is VideoPlayer / ImageSequence seekTo;
  - for a video or sequence, the model playhead is by design the render's mirror of the player clock (Renderer.cpp:1823, 1910; MainComponent.cpp:4690-4700), so its "lost update" is that designed overwrite.

R-A7 [SHOULD] ACCEPT, same as M-A6.

R-A8 [SHOULD] ACCEPT (see G6).
- The Debug concern is answered by measurement (the -O0 row above): no libatomic.

R-A9 [NIT] ACCEPT: a doc line plus the G2 wording.
- VERIFIED: CompositorEngine.cpp:1046-1056 (hasActiveLayers) vs :1072 (the per-layer load).
- The only window is a clear landing between the two loads. That frame shows the layer empty, which is what the clear shows one frame later anyway.

### Seat tsan-gates

G-A1 [MUST] ACCEPT (see amendments 2 and 11).
- A ctest that configures a TSan tree is REJECTED: it adds minutes to every full ctest, and the committed script is the same check on demand.

G-A2 [MUST] PARTIAL.
- REJECT the ~1/6 premise. Family-level per-launch detection rates in the fresh RED (tsan-red-main-655d232.txt F1-F25; tsan-red-main-launches.tsv):

| Family | Fresh-RED rate |
|---|---|
| A | 9/9 |
| B | c 3/3, b 1/3 |
| C | b + c 6/6 |
| D | c 2/3 |
| E | c 3/3 |

- 1/9 rates exist per UNIQUE pair, but each family is fixed by a type change, not pair by pair.
- ACCEPT:
  - a pre-registered power rule (G3.5);
  - "family missing on the main arm" is stated as "no app evidence", not silently passed;
  - drive D harder in scenario d.

G-A3 [MUST] ACCEPT, amended (see G3.2 / G3.3).
- VERIFIED from F1 (tsan-1):
  - the ApiServer thread's ACCESS stack is libc++ frames plus `std::__thread_proxy<...ApiServer::start()::$_0>` (thread.h:214);
  - src/ appears only in its creation stack (ApiServer.cpp:93).
- So the plan's access-stack-only rule would file a race between two such threads as JUCE/SYSTEM: a false GREEN.
- The RED had 0 "[failed to restore" in 63 warnings at history_size = 0 (the runtime default; VERIFIED help=1), so history_size = 4 is insurance.
- "Lane-only JUCE/SYSTEM = hard stop" becomes a pre-registered re-run rule. A hard stop on one rare JUCE race would make the gate flaky.

G-A4 [MUST] PARTIAL.
- ACCEPT d-specific RED validity: a LayerClock.h frame in the main arm's scenario d reports.
  - This runtime dedups by stack only (suppress_equal_stacks; it has no suppress_equal_addresses flag, per help=1).
  - A fade write at LayerClock.h:24 is a new write access, so it reports as its own pair when fades run (INFERRED).
- REJECT "adopt-on-fail > 0 on the lane" as a bar. It would fail by design:
  - a render adopt needs a message-thread CAS inside the about 1 us between that layer's load and its tick CAS;
  - at d's ~100 layer-triggers per launch, that is ~0.01 events per launch, about 3 % over 3 lane-d launches (INFERRED estimate).
- The adopt path is instead proven where it is frequent: the contention test (amendment 4).

G-A5 [SHOULD] ACCEPT, same as M-A7.

G-A6 [SHOULD] ACCEPT (see amendment 2 and G2).

G-A7 [SHOULD] ACCEPT (see amendment 2).

G-A8 [SHOULD] ACCEPT (see G4).
- VERIFIED: probe-deck-path.sh:56-58 and probe-mastersignal.sh print named PASS / FAIL rows, an inline registry, so rows are compared by name.

G-A9 [SHOULD] ACCEPT (see amendments 3 and 4).

G-A10 [NIT] ACCEPT (see amendment 16).

G-A11 [NIT] ACCEPT (see amendment 2 and G1).
- `catch_discover_tests(... PROPERTIES TIMEOUT 60)` is repo precedent (tests/CMakeLists.txt:1784), so PROPERTIES LABELS uses the same mechanism. It is still verified with `ctest -N`.

## ARCHITECT RULING (s-rta-0930)

These numbered amendments OVERRIDE the plan body wherever they conflict. Everything not named here stands.

1. GOAL (overrides plan (1)).
   - In scope: remove by design every race of the 5 observed families, and fix the F2 tuple for real. The families:
     - A: std::cerr on worker threads;
     - B: the Layer tuple, plus clip playing / hasBeenTriggered / beatsPlayed;
     - C: playheadPosition;
     - D: the 23 manualRef scalars;
     - E: activeDeckIndex.
   - NOT in scope, named follow-up lane "tsan-r5":
     - the R5 config-scalar class (plan :539-546, plus the perf/play preamble writes above);
     - R7, the unfenced httplib reader.
   - The G3 bars are defined per scenario, not as a blanket "zero app reports".

2. T0 test infrastructure (overrides plan :199-200 and :236-240).
   - Register every race target with:
     `catch_discover_tests(<t> PROPERTIES LABELS tsan TIMEOUT 300 ENVIRONMENT "TSAN_OPTIONS=exitcode=66:halt_on_error=0:abort_on_error=0:report_signal_unsafe=0:history_size=4" FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer")`
     The ENVIRONMENT property overrides the caller's shell, so a G3-style exitcode=0 can never make these tests pass. In a normal build the property and regex are inert.
   - In T0, verify that `ctest -L tsan -N` lists every case, and record the case count per target.
   - Size the iterations: measure each [tsan] case in both builds. Targets: <= 60 s under TSan, <= 5 s normal. Record the times in the work log.
   - RED bar at the T0 commit.
     - TSan build: every [tsan] case exits 66 (not a signal), and its output has >= 1 "WARNING: ThreadSanitizer: data race" that names a src/ file in ANY stack (access, location / allocation, or thread creation).
     - A crash is INCONCLUSIVE: re-run, up to 3 times. Store the logs.
     - Normal build: D1, D1b, D1c, D6 and lint case 1 (amendment 8) fail; everything else passes.

3. R4 and R1 invariants (T0, in tests/test_layer_runtime_race.cpp; overrides :208-217). All APIs used exist on both commits.
   - R4 [tsan] "tuple consistency and no lost fade under a paced trigger storm":
     - Setup: a Layer with 4 clips, and transitionSpeed chosen so that step = dt / speed = 0.3 (e.g. dt 0.003f, speed 0.01f; LayerClock.h:21-23). Call triggerClipImmediate(0) before the threads start.
     - Render thread, until told to stop: obs = captureLayerRuntime(L); check obs; renderObs++; LayerClock::advanceCrossfade(L, dt).
     - Main thread, N triggers (N sized per amendment 2). Before each: wait (yield) until renderObs >= last + k, with k cycling 1..5; a 60 s timeout is a FAIL ("render stalled"). Then L.triggerClipImmediate(i % 4): cyclic, so never a retrigger.
     - Checks on every observation, counted in an atomic violation counter; REQUIRE(violations == 0) after join:
       - I1: active >= 0 implies previous != active.
       - I2: previous is -1 or (active + 3) % 4.
       - I3: the first observation of a new active has progress <= step (+1e-6).
       - I4: progress is in [0, 1].
     - Why it has teeth in the NORMAL build:
       - per-field atomics (Fork 1a) tear I2;
       - a plain-store tick (the mutant) loses a trigger, which shows as I3: progress = old + step, or (c, -1, 1.0) at fade end.
     - On the lane, I1-I4 hold deterministically: there is one tick per observation, and a trigger landing between a tick's load and its CAS makes the tick adopt.
   - R1 additions:
     - Every render-side snapshot checks active / previous / pending in [-1, numColumns), progress in [0, 1], and I1, counted in a violation counter asserted 0 after join.
     - A THIRD thread (the httplib-like reader) loops captureLayerRuntime(L) plus plain-syntax reads of clip.playing, clip.playheadPosition and layer opacity.
     - Every 17th main iteration takes `Layer copy = L;`.
   - R2 gets the same third reader thread.

4. NEW GREEN-only tests (T2 / T3, tests/test_layer_runtime.cpp, normal build).
   - Bounded exhaustion (deterministic): updateRuntime(fn, 16), where fn calls setRuntime() with a different value on every call. It returns "not applied" after exactly 16 attempts and leaves the concurrent value intact.
   - Contention, two threads with a start barrier, run until adopts > 0 or 2 s:
     - the render thread ticks and calls processPendingTrigger(..., 16);
     - the message thread storms triggerClip;
     - every message-thread transition equals its intent (immediate: after.active == col; queued: after.pending == col);
     - REQUIRE tick adopts > 0 (the adopt path ran);
     - R4's I1 and I2 hold.

5. Clip tail ordering (overrides T2 step 3, :285-287).
   - Stated guarantee: the tuple word is the only consistent unit. Clip runtime fields are per-field atomics.
   - Activation tail (a new activation or a retrigger of col): playheadPosition = inPoint, beatsPlayed = 0, and playing = true only for a new activation with !hasBeenTriggered.
     - It is written BEFORE each CAS attempt that would install the transition, and recomputed per attempt (the writes are idempotent).
     - The acq_rel CAS then publishes it: any acquire load that names col happens-after its reset.
   - Clear tail (the old active clip's playing = false): stays AFTER the successful CAS, applied once to t.before.activeClipColumn.
     - It needs no guard: after the clear CAS, no render path can re-activate the layer (M-A2 evidence).
   - casRuntime's failure order is acquire. updateRuntime reports whether it applied.
   - GREEN-only contract test:
     - The message thread sets inactive clip c's beatsPlayed = 1000, then triggers c.
     - The render thread asserts beatsPlayed != 1000 on its first observation of active == c.
     - This is deterministic on the lane. A post-CAS mutant fails it only rarely, so the order is ALSO a reviewer check.

6. Undo (overrides :301-304).
   - TriggerClipCmd and ClearActiveClipCmd: the FIRST execute() call is a no-op (a one-shot flag), including the `playing` restore. The handler already applied the live mutation (mutate-then-push, TriggerCommands.h:20-26).
   - Later execute() calls (redo) do setRuntime(after_) and restore `playing` as today. undo() does setRuntime(before_) as today.
   - Delete sameIntent.
   - Add to tests/test_undo_commands.cpp in T0. Both are RED on 655d232 (execute re-applies after_, TriggerCommands.h:68) and GREEN on the lane:
     - D1b "the fade ends before perform": active 0, transitionSpeed 0.5; before; triggerClip(1); after; advanceCrossfade(L, 1.0f) gives (1, -1, 1.0); perform. REQUIRE the tuple is still (1, -1, 1.0).
     - D1c "the queued trigger fires before perform": clip 1 has beatSnapMode Beat; before; triggerClip(1) (queued); after; processPendingTrigger(0, 0) (fires); perform. REQUIRE active == 1 && pending == -1.
   - Add a doc sentence to TriggerCommands.h: undo and redo are value restores. They supersede any render transition since the gesture (fade tick, autopilot, fired pending), as main always did.

7. Render-side CAS bound (overrides :290, :345-346 and R3).
   - processPendingTrigger and Autopilot's triggerClip calls use maxAttempts = 16.
   - Corrected claim: exhausting the attempts needs 16 consecutive message-thread CASes on one layer inside one ~1 us render call, which is unreachable at human or REST rates.
   - If it ever happened:
     - an autopilot advance or a Beat-snapped trigger retries at the next beat crossing;
     - a Bar / TwoBar / FourBar trigger slips to its next qualifying edge.
   - No latch.
   - The fade tick stays at ONE attempt (adopt-on-fail).

8. syncMedia write-back and render lints (NEW target tests/test_render_thread_lint.cpp).
   - The video AND sequence branches of syncMedia both call ClipTransportSync.h: read the intent once and push it; CAS the write-back; test the out-point on the local ph.
   - Lint case 1 (committed in T0; RED on 655d232, where it fails on :1826 / :1833 / :1911 / :1918):
     - src/render/Renderer.cpp contains no plain store to `playing` (no `playing =`, no `playing.store(`);
     - it has >= 2 call sites of the ClipTransportSync entry points.
   - Lint case 2 (added at T3, pinned):
     - exact counts of `.runtime()` and `getActiveClip(` in CompositorEngine.cpp, Renderer.cpp, DeckClock.h and Autopilot.cpp;
     - a comment lists each site's function;
     - changing a count means re-justifying it in review.
   - A comment in ClipTransportSync.h records the verified ABA note (M-A8).

9. T6 widened (overrides Fork 4's "not needed").
   - Composition::activeDeckIndex becomes RelaxedInt.
   - getActiveDeck() (const and non-const, Composition.h:190-201) loads it ONCE.
   - juce::var sites use .load() (e.g. Composition.h:306, ApiServer.cpp:360 / 384 / 1540).
   - The GL still derives its index from the acquire-loaded deck pointer (plan T6 stands).
   - Fallout: 129 src and 91 test references; grep found no compound operators.

10. Momentary release (overrides T2 :300; Boris Q3, default = fix).
   - NEW Layer::releaseMomentary(int column) -> LayerRuntimeTransition: ONE CAS of a pure function:
     - active == column: today's clear, including the pending cancel;
     - else pending == column: pending = -1 and snap = Off, active untouched;
     - else: no-op.
   - MainComponent.cpp:7593-7595 and :7632-7633 call it with no pre-check.
   - GREEN-only tests:
     - a release before the beat cancels, and the clip does not latch;
     - press A, press B, release A leaves B's pending;
     - a release after the trigger fired clears, as today.
   - Mutant: drop the pending branch; the first test fails.
   - If Harmony declines: keep today's behaviour and file the bug.

11. Normal-build type pins and a committed TSan runner.
   - NEW tests/test_shared_field_types.cpp. The tuple pin lands at T2; T4, T5 and T6 extend it. Contents:
     - static_assert on decltype of Clip::playing / playheadPosition / beatsPlayed / hasBeenTriggered being the Relaxed types;
     - each of the 3 manualRef overloads returns RelaxedFloat&, which pins all 23 fields through their switch returns;
     - Composition::activeDeckIndex is RelaxedInt;
     - `template<class L> concept LooseTuple = requires(L& l) { l.activeClipColumn; };` with `static_assert(!LooseTuple<Layer>)`.
   - NEW committed .harmony/probe-tsan-unit.sh [build-dir]:
     - configures the prebuild TSan recipe if the dir is absent;
     - builds the tsan-labelled targets;
     - runs `ctest -L tsan --output-on-failure`, and exits with ctest's code;
     - launches no app.
   - The new pitfall names this script a REQUIRED gate for any change to a field another thread reads.

12. Scenario d (overrides :442-448).
   - Fixture: 2 decks x 3 layers x 4 columns (v1080 videos + images).
     - Every layer has transitionSpeed 0.5.
     - Deck 0, layer 1: autopilotEnabled, PlayNext at the shortest duration.
     - One clip has beatSnapMode Beat, one has Bar.
     - Keys follow Layer::toVar / Clip::toVar. Everything is set at load time, never by REST.
   - Start: POST /api/set_bpm {"bpm": 240} (Manual BPM gives beats without audio).
   - Every 0.25 s for about 12 s:
     - trigger_clip, including the active cell and an empty cell;
     - trigger_column;
     - set_layer_opacity, every step, rotating layers;
     - set_master_signal, every other step;
     - switch_deck, about every 2 s.
   - Reads during the storm: /api/health only. One GET /api/state at the end.
   - Forbidden: GET /api/composition, perf/*, and any load while stepping.
   - NEW INFO scenario e: the d fixture, then perf/record -> 3 triggers -> perf/stop -> perf/play (as probe-routines.sh drives them). It is the R5 baseline for "tsan-r5".

13. /api/state witnesses (NEW; exist on the lane only).
   - Three relaxed uint64 counters, owned by the Renderer:
     - render_pending_fired;
     - render_autopilot_advances;
     - render_tuple_adopts.
   - Sources:
     - Autopilot reports the transitions it applied (before != after) from processPendingTrigger / triggerClip, e.g. through an out-parameter; the Renderer accumulates them;
     - LayerClock::tick's false returns count as adopts.
   - Add them to TestServer's twin if it mirrors /api/state.

14. Docs, in addition to plan (6).
   - The new pitfall also records:
     - probe-tsan-unit.sh is required;
     - the first execute() of a mutate-then-push command is a no-op;
     - activation tails come before the CAS;
     - activeDeckIndex is Relaxed;
     - a live Layer copy is per-field atomic, not a snapshot.
   - rendering.md, beside R11: the hasActiveLayers one-frame note (R-A9).
   - performance-controls.md: a momentary release cancels its own queued trigger (if amendment 10 is adopted).
   - integration.md: the three /api/state fields.
   - Plan (8), Boris's live check, adds: "a momentary pad released before its beat does not latch on".

15. Scope record.
   - R5 stays unfixed (the plan's list, plus perf/play's preamble writing layer flags and quantizeMode plainly, verified above).
   - The "tsan-r5" follow-up lane starts from scenario e's reports.

16. Mutant evidence. The Builder runs each mutant once and pastes the failing output into the work log:
   - casRuntime as a plain store;
   - tick as a plain store (fails R4 I3 and the T3 stale-snapshot test);
   - the syncMedia write-back as a plain store (fails test_clip_transport_sync);
   - releaseMomentary without its pending branch;
   - the activation tail moved after the CAS (amendment 5 test; say so if it did not fire);
   - execute() without the first-call skip (fails D1, D1b and D1c).

17. Merge surface (adds to R13).
   - This lane's ApiServer.cpp hunks: /api/state (1370-1544) and lines 360 / 384.
   - bt2's ApiServer hunks (~323, ~2086) do not overlap.

18. RED-first bookkeeping.
   - The T0 commit gains R4, D1b, D1c, lint case 1, and R1's / R2's third reader.
   - All other new tests are GREEN-only, and their teeth are the mutants in amendment 16.

### FINAL GATES (pre-registered; override plan (5))

G1 Normal build + full ctest (Release).
- 100 % pass.
- Total cases = 1049 + the sum of the new per-target case counts the Builder recorded.
- These targets are present, each with exactly its recorded count: test_layer_runtime_race, test_manual_scalar_race, test_layer_runtime, test_clip_transport_sync, test_relaxed, test_log_line_lint, test_render_thread_lint, test_shared_field_types.

G2 TSan unit RED / GREEN via .harmony/probe-tsan-unit.sh. Put no TSAN_OPTIONS in the shell; the test ENVIRONMENT pins them.
- RED arm: a worktree at the T0 sha, with its own build dir.
  - Valid iff every [tsan] case (R1, R2, R3, R4) exits 66 with >= 1 data race naming src/ in any stack.
  - A crash is INCONCLUSIVE: re-run, up to 3 times.
- GREEN arm: merged main. Every [tsan] case passes with 0 "WARNING: ThreadSanitizer" (FAIL_REGULAR_EXPRESSION enforces it).
- INFO: the full TSan ctest on both arms; the lane adds no failing case.

G3 TSan app sweep.
- TSAN_OPTIONS, identical on both arms:
  `halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:history_size=4:log_path=<run>/tsan`
- Smoke first: one lane-d launch must pass launch validity. If history_size=4 breaks it, use 2 on both arms and record that.

G3.1 Launch order.
- 3 rounds of main-a, lane-a, main-b, lane-b, main-c, lane-c, main-d, lane-d.
- Then 2 rounds of main-e, lane-e.
- One live app at a time, lock held.
- Launch validity is as in the plan: a failed launch is re-run, never counted.

G3.2 Classes (analyzer .harmony/probe-tsan-analyze.py).
- Key: the harness's key, i.e. kind + the top-4 frames of both access stacks over src/ and _deps basenames (analyze.py:5-22).
- Classes:
  - APP: a src/ frame in either access stack, in the location's global / heap-allocation stack, or in either access thread's creation stack.
  - UNATTRIBUTED: not APP, and an access stack is empty or "[failed to restore the stack]".
  - JUCE/SYSTEM: everything else.
- FAMILY keys, judged by the racing source line of any src frame. The analyzer prints each top src frame's source line so every key is checkable.
  - A: the location is std::__1::cerr, or an ostream frame on a src thread, with at least one side in a file on T7's converted list. A cerr race with both sides in R8 residual files (VideoPlayer.cpp outside open(), AudioEngine.cpp, DeviceGuard.cpp) is PRE-EXISTING-R8: listed, not a lane failure.
  - B: a Layer tuple access, or clip playing / hasBeenTriggered / beatsPlayed.
  - C: playheadPosition.
  - D: one of the 23 manualRef fields, or eff().
  - E: activeDeckIndex.

G3.3 Bars.
- a. Any scenario, lane arm: 0 FAMILY-keyed reports.
- b. Scenarios a / b / c, lane arm: 0 APP and 0 UNATTRIBUTED. This is absolute: all 25 fresh-RED uniques were family reports.
  - An UNATTRIBUTED report is re-run with history_size=7 (3 launches per arm) and judged by the class it resolves to.
  - Still UNATTRIBUTED and lane-only = FAIL.
- c. Scenario d, lane arm: every APP / UNATTRIBUTED key must also appear in the main arm's d launches. Then it is PRE-EXISTING: listed and filed.
  - A lane-only one triggers 3 more d launches per arm. Recurs lane-only = FAIL; otherwise PRE-EXISTING.
- d. Scenario e: INFO, except bar a.
- e. JUCE/SYSTEM seen only on the lane (any scenario): 3 more launches of that scenario per arm. Recurs lane-only = FAIL; otherwise INFO.

G3.4 Validity.
- Main arm:
  - each of families A-E has >= 1 report. Pool with the fresh RED only if the main-arm commit is 655d232. If a family is missing, run up to 3 extra main-c launches.
  - >= 1 d report has a LayerClock.h frame. If absent, run up to 3 extra main-d launches.
- Each lane-d launch: render_pending_fired >= 1 AND render_autopilot_advances >= 1 at its final /api/state; otherwise the launch is invalid and is re-run.
  - 3 consecutive invalid lane-d launches = FAIL: the render writers do not run on the lane.
- If a requirement is still unmet after its extras, the matching claim is stated as unit-evidence-only. For example: "family D fixed by type change + R3; the app sweep had no main-arm D report".

G3.5 Power.
- p̂(f) = (main-arm launches with a family-f report) / (main-arm launches of f's driving scenarios). Driving scenarios: A all; B c, d (and b); C b, c, d; D c, d; E c, d.
- Required: (1 - p̂)^n <= 0.05, where n = the lane launches of those scenarios.
- If not met, add lane launches of f's highest-rate scenario, capped at +6.
- If the cap is hit, f is "app-sweep underpowered", and its claim rests on the type pins + R-tests, stated as such.

G3.6 render_tuple_adopts is reported, never a bar.

G4 Behaviour parity.
- Setup:
  - Release builds of 655d232 and of merged main, each from its own build dir;
  - same session, lock held;
  - per probe the arms alternate, and each probe runs twice per arm.
- Rows:
  - probes with a JSON registry: its rows;
  - probe-deck-path.sh and probe-mastersignal.sh: their printed PASS / FAIL row names.
- Probes: the plan's list, plus probe-media-open.sh m6_retrigger_seek and probe-video.sh w3_retrigger_midgop_1080 and w6b_retrigger_mid_fade. Use the whole probe where there is no row filter.
- FAIL iff a row passed on 655d232 (>= 1 of 2) and failed on merged main in 2 of 2.
- A row that is red on both arms is pre-existing and is listed.

G5 Latency.
- The plan's review bar.
- Plus test_render_thread_lint case 2 (pinned load counts) passes.

G6 Perf.
- Release builds, interleaved, >= 5 launches per arm, scenarios b and d. Check ps for orphaned CPU burners first.
- FLAG if a lane median of peak_frame_time_ms or peak_callback_ms exceeds main's by more than 15 % AND by more than 0.5 ms.
- A FLAG triggers 5 more launches per arm. If it holds on the pooled 10, it is a FAIL: a named render-thread regression. Otherwise INFO.

### QUESTION FOR BORIS (adds to plan (9); the build does not wait)
Q3. Take a MIDI or keyboard pad set to "momentary", with Quantize on. If you press it and let go before the beat arrives, today the clip still starts on the beat, then stays on. Should letting go before the beat cancel it?
DEFAULT: yes. The code's own notes say that is the intended behaviour.

### RISKS OF THIS RULING
- Strongest counter to amendment 6: a command whose first execute() does nothing depends on every handler live-applying first. A future handler that forgets would push a silent no-op.
  - It loses because every current site and test live-applies (VERIFIED), the class comment defines the pattern, and D1 / D1b / D1c pin it.
  - sameIntent, the alternative, has real misfire windows.
- Strongest counter to amendment 12: scenario d loses the httplib-reader and routine-replay coverage under load.
  - It loses because both paths touch unconverted plain state (R7, R5), which would fail the bar for reasons outside the lane.
  - The converted fields' third-reader coverage moves to R1 / R2's deterministic reader thread.
  - Replay reuses the REST trigger code (MainComponent.cpp:1964-1971 -> handleClipTrigger).
- INFERRED, not observed:
  - amendment 9's "unmasking" (TSan shadow eviction); the conversion is right regardless;
  - the ~1 us adopt window and the ~3 % figure in G-A4;
  - Manual BPM producing totalBeatCount crossings. The first lane-d launch's counters verify it.
- R4's mutant teeth are probabilistic, though tight loops make detection near-certain in practice. Amendment 16 records one run as evidence.
- Rig time: G3 grows to 28 counted launches, plus up to ~20 extras, about 40-60 min. G4 doubles every probe.

STATUS: DONE
