# PLAN lane tsan (s-rta-0930) -- race-free cross-thread model publication, gated by ThreadSanitizer

Architect (Fable), 2026-09-30, main HEAD 655d232. Read-only on source; scratch experiments only under
/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/{a128,logline}
(no app launched). Labels: VERIFIED = read / ran it today (file:line); INFERRED = follows from verified facts; ASSUMED = not checked.

## (1) GOAL
Remove by design every race of the s-rta-0929b TSan sweep (29 unique / 5 families; fresh RED on 655d232 = 25 unique, all
5 families). Fix the F2 clip-trigger tuple for real (one consistent tuple per layer per frame, no lost update, no double
read), so a TSan app sweep a/b/c(/d) shows ZERO app-attributed reports. Threaded Catch2 tests fail under TSan on main
and keep that coverage in ctest.

## (2) ESTABLISHED FACTS

### 2.1 The trigger tuple (family B, F2 MEDIUM)
- VERIFIED Layer.h:181-190: 5 plain fields (activeClipColumn, previousClipColumn, crossfadeProgress, pendingTriggerColumn,
  pendingTriggerSnapOverride). Layer lives by value in Deck::layers (std::vector<Layer>, Deck.h:31-60).
- VERIFIED Layer.h:255-288 triggerClipImmediate: pending/snap cleared (260-261), retrigger branch (263-273), then
  previous / active / progress as 3 separate stores (276-278), then clip->playheadPosition / beatsPlayed / playing
  (282-286). triggerClip queues on beat snap (237-248); clearActiveClip 330-352; processPendingTrigger 292-328.
- VERIFIED Layer.h:193-211 getActiveClip reads activeClipColumn up to 3 times. A concurrent clearActiveClip (-1,
  Layer.h:338) plus a compiler reload can index clips[size_t(-1)] (UB; never observed).
- VERIFIED LayerClock.h:17-27: the RENDER thread read-modify-writes crossfadeProgress and stores previousClipColumn = -1 at
  fade end, so the tuple has two writers. Callers: CompositorEngine.cpp:1072-1075 (active deck), ~1330-1332
  (compositePersistentLayers), DeckClock.h:28-39 (decks not on screen, GL thread, Renderer.cpp:766-777).
- VERIFIED render-thread TRIGGER writers: Renderer.cpp:547 / :772 Autopilot::processFrame -> Autopilot.cpp:52-59
  processPendingTrigger, :34-36 / :101-105 advanceClip -> Layer::triggerClip (Autopilot.cpp:288, 310, 372);
  Autopilot.cpp:25 `beatsPlayed++`, :77 `beatsPlayed += beats`; Autopilot reads activeClipColumn separately from
  getActiveClip (14 vs 34; 67 vs 103).
- VERIFIED render-thread tuple READS per layer per frame: CompositorEngine.cpp:1049 (hasActiveLayers), 1072, 401
  (incomingImagePending), 991-992 (CrossfadeStartDetector::observe), 1646 / 1650 / 1667 / 1740 (applyTransition), 1271
  (hasPersistentContent), ~1330; Renderer.cpp:593-595; DeckClock.h:30-38.
- VERIFIED message-thread writers: MainComponent.cpp:4636 / 4638 / 4647 (handleClipTrigger), 4853 (handleColumnTrigger ->
  Deck::triggerColumn, Deck.h:111-119), 758 (X clear), 5522 (cancelPendingTriggers on deck switch), 5571, 6911, 7595 / 7633
  (momentary release: checks `activeClipColumn == resolvedColumn`, then calls clearActiveClip, a check-then-act race),
  and the undo commands (TriggerCommands.h:68-69 / 104-113; ClearActiveClipCmd DeckCommands.h:407-418; applyLayerRuntime
  DeckCommands.h:210-217; cancel / restore DeckCommands.h:241-278).
- VERIFIED undo capture is synchronous around the live mutation: MainComponent.cpp:4620-4652 and 4833-4879; pushCommands
  5136-5154 -> UndoManager::perform executes, then merges (UndoManager.cpp:12-45).
- VERIFIED TriggerClipCmd::execute re-applies `after` on its FIRST perform (TriggerCommands.h:20-26, 68). If the render
  advanced the fade between the live trigger and perform (microseconds), progress is reset to 0. The fade restarts, and
  CrossfadeStartDetector sees progress go backwards (CrossfadeHistory.h:38-50) and hands the history over a second time.
  Deterministic on main (test D1).
- VERIFIED the code documents this race as accepted status quo: TriggerCommands.h:34-36, DeckCommands.h:393, CrossfadeHistory.h:22-31.
- VERIFIED every non-render writer is the message thread: REST handlers callAsync (sweep stacks), OSC uses
  MessageLoopCallback + callAsync (MainComponent.cpp:2196-2210).
- VERIFIED the sweep fixtures use transitionSpeed 0.0 (scen.py layer()), so a/b/c exercise CUTS only. The fade-end lost
  update is not exercised by a/b/c; scenario d below adds fades.

### 2.2 Clip runtime fields (families B / C)
- VERIFIED Clip.h:228-231: `mutable bool playing`, `mutable double playheadPosition`, `int beatsPlayed`, `bool hasBeenTriggered`
  are plain.
- VERIFIED Renderer.cpp:1787 / 1789 read clip->playing twice, :1823 writes playheadPosition, :1826 writes back
  playing = player->isPlaying(), :1829 RE-READS playheadPosition for the out-point test, :1832-1839 OneShot / wrap
  writes. The sequence twin is at :1874-1925. syncMedia is the only render writer (tickMediaClock = syncMedia(decode=false),
  Renderer.h:688).
- VERIFIED the retrigger RESTART is done by seeking the player (MainComponent.cpp:4688-4712; handleClipTrigger only, only
  when the target deck is the active one, :4658). The model store playheadPosition = inPoint (Layer.h:268/282) is
  overwritten from the player next frame by design (comment :4690-4700). handleColumnTrigger (4898-4941) has no seek.
- VERIFIED LayerStrip.cpp:747-750 already reads playheadPosition via std::atomic_ref ("GL-side writers still write it
  plainly -- L5's open item"), so family C is the plain writers.
- VERIFIED GET /api/composition and /api/state read the live model on the httplib thread (ApiServer.cpp:380-440, 1425),
  making the httplib thread a third reader of these fields.

### 2.3 Manual scalars (family D)
- VERIFIED manualRef maps 7 Layer (Layer.cpp:4-19), 7 Clip (Clip.cpp:4-13) and 9 Composition (Composition.h:700-716) plain
  float fields. manualWriteCore writes `*r.manual = ...` (ManualWrite.cpp:179-188) through ControlRef::manual (float*,
  ManualWrite.h:31), which ALSO points at effect params / dryWet / source params / macros (ManualWrite.cpp:53-66, 92,
  128-129). eff() reads it on the GL thread (Layer.cpp:21-24; CompositorEngine.cpp:1157). Other ControlRef::manual
  users: MainComponent.cpp:2055-2058 and ManualWrite.cpp:183-187 (grep).
- VERIFIED LiveValue.h:17-38 is the house pattern for a copyable relaxed-atomic member (explicit copy / move ctor + assign).

### 2.4 activeDeckIndex (family E)
- VERIFIED Renderer.cpp:471 is the ONLY render-thread read of activeDeckIndex (grep of src/render + Autopilot.cpp).
  Renderer.cpp:543-546 already derives the active index from the acquire-loaded deck pointer. Writer order: index
  (MainComponent.cpp:5525) then setActiveDeck (release, :5529; FencedPtrSlot.h set()).

### 2.5 std::cerr (family A)
- VERIFIED RED 655d232 pairs: AnalysisThread.cpp:82-84 vs the ApiServer thread line (ApiServer.cpp:93); VideoPlayer.cpp
  307-316 (the "Opened" statement) on two MediaOpen threads. src has 113 std::cerr statements.
- VERIFIED scratch (scratchpad/logline/t.cpp, clang 17, -fsanitize=thread): a logLine helper (std::ostringstream + ONE
  std::fwrite to stderr) on two threads = 0 reports; one thread std::cerr + one thread logLine = 0 reports; two threads
  std::cerr = 4 reports (control).
- VERIFIED gop2 reserved VideoPlayer.cpp open() :83-316 for this lane's cerr edits; gop2's own open() hunk is :276-297
  (plan-gop2.md CROSS-LANE TOUCHES; ruling-gop2.md :227). bt2 touches MainComponent.cpp only at ~490, 2148-2150 and
  5697-5699, and ApiServer.{h,cpp} only in test-only hunks ~323 / ~2086 (plan-bt2.md section 10).

### 2.6 Structure vs render reads (the "prove or refute" question): REFUTED on every enumerated path
- VERIFIED every message-thread structural mutation of the LIVE model found by grep runs inside
  UndoService::withDeckDetached (UndoService.cpp:32-94: detachFenced + executeOnGLThread drain + restore):
  MainComponent.cpp:823(->831), 911(->918), 1012(->1017), 1142(->1148), 1223(->1226-1249), 1313(->1315), 1389(->1391),
  5429(->5436), 6641(->6649), 6724(->6726), 6836, 6860; swapCompositionModel 2981-3024 (composition swap / New); DeckCommands
  layer / deck / column / clips-row commands via DeckFenceHook (DeckCommands.h:24-39); staged loads mutate an unpublished
  Composition (MainComponent.cpp:3333 / 3509, ApiServer.cpp:1070; Pitfall 58). The render touches decks / layers / clips
  only inside `if (deckActive)` (Renderer.cpp:541-778), which the fence nulls with one acquire load (Renderer.cpp:387-429;
  FencedPtrSlot.h). So no vector<Layer> / vector<Deck> / clips reallocation can overlap a GL read on those paths.
- INFERRED residuals: (i) a grep cannot prove that no mutation is spelled some unusual way; (ii) the fenced frame with NO
  canvas yet (Renderer.cpp:428, startup only) still reads Composition SCALARS (:452-453, :471, :478, :712) during a swap.
  That is the scalar class, handled by T5 / T6 / risk R6. (iii) The httplib thread is NOT fenced: /api/composition
  iterates the live decks / layers / clips / strings (ApiServer.cpp:395-440), so a concurrent swap is a use-after-free
  hazard. It is pre-existing, not TSan-found and not in the 5 families: risk R7, follow-up.

### 2.7 Toolchain (VERIFIED)
- std::atomic<16-byte POD> is_always_lock_free = 1 on Apple clang 17.0.0 arm64; no libatomic call; TSan instruments it
  (___tsan_atomic128_compare_exchange_val / _load); 2-thread CAS stress = 0 reports (scratchpad/a128/t.cpp).
- Codegen -O2 (scratchpad/a128/ld.cpp): acquire load = `ldp` + `dmb ishld`; release store = `dmb ish` + `stp`; CAS =
  `caspal`. No exclusive-monitor loop; a reader never writes the cache line.
- cmake/Sanitizers.cmake:22-46: apply_sanitizers is called for every test target (tests/CMakeLists.txt), so a
  -DADNA_SANITIZE=thread build dir builds TSan ctest targets. TSan's default exitcode 66 fails a ctest process that raced.
- CompositionLoad.h:28 kMaxNumColumns = 10000. Clip::BeatSnapMode is uint8 with 5 values (Clip.h:130-; Layer.h:309-324).
- Baseline ctest: 1049 tests / 111 Catch2 targets (.harmony/APP-INVENTORY.md:31).
- TSan builds: prebuild.md:18-20 recipe (RelWithDebInfo, -DADNA_SANITIZE=thread, TEST_SERVER ON, SYPHON ON,
  FETCHCONTENT_FULLY_DISCONNECTED=ON, FETCHCONTENT_SOURCE_DIR_{JUCE,HTTPLIB,CATCH2,SYPHON} -> <main>/build/_deps/*-src).
  A TSan build of 655d232 exists: scratchpad/build-tsan-main (CMakeCache: ADNA_SANITIZE=thread, RelWithDebInfo).

## (3) DESIGN FORKS

### Fork 1: the trigger tuple (F2 + family B core)
(a) Relaxed-atomic wrapper per field (verifier F2 option a). This silences UB / TSan. But the render still reads a mixed
    tuple (3 separate stores, Layer.h:276-278), and the fade-end RMW (LayerClock.h:24-26) still overwrites a trigger, so
    the new clip hard-cuts. Fails "fixed properly". REJECTED.
(b) Single-writer mailbox drained at the top of renderOpenGL (verifiers F2 b, F4, F9, F19, F21, F22). This fixes both, but:
    (1) undo needs before/after synchronously (MainComponent.cpp:4620-4652, 4833-4879). The message thread would have to
    PREDICT `after`, and the prediction diverges whenever the render changes the tuple before the drain (fade end,
    autopilot, beat snap), so undo becomes inexact. (2) The UI reads the model right after the trigger (4658-4760: preview
    reload through getActiveClip, deckView refresh, recorder capture) and would read stale state. (3) The drain does not
    run while the GL context is detached (Renderer.cpp:532-537) or during a fence (413-429), so a second direct-apply path
    is needed. (4) About 440 headless test references assume synchronous mutation. REJECTED; this is the runner-up.
(c) Per-frame published snapshot (seqlock / triple buffer of the tuple). Readers get a consistent tuple, but there is no
    protocol for the render's own writes, and a seqlock reader that retries is a render thread that waits. REJECTED.
(d) CHOSEN: the WHOLE tuple in ONE lock-free 16-byte atomic word per Layer (LayerRuntimeCell). Every transition from
    EITHER thread is a CAS on that word. This gives:
    - a consistent tuple (one load per layer per frame);
    - no lost update (a fade-end CAS fails when a trigger landed, and the render adopts the trigger's tuple);
    - no double read;
    - ZERO added latency: the message thread's CAS is visible to the next render load exactly like today's store, and
      the render's load sits where today's first read sits;
    - exact undo: the trigger RETURNS the CAS pair {before, after};
    - no queue and no dependency on the render loop running; headless semantics are unchanged.
    The render never waits: its fade / clock CAS is ONE attempt (adopt-on-fail); its triggers (autopilot, beat snap)
    try at most 4 times, then give up for the frame.
    Strongest counter-argument: "single writer is the textbook fix; two CAS writers can livelock." It loses because the
    render's CAS never loops, and the message thread's loop only competes with at most a few render writes per layer per
    frame (expected retries ~0). Option (b) cannot keep undo exact without prediction, and needs two apply paths.
    Word (16 B, no padding bits, alignas(16)): int32 active, int32 previous, float progress,
    uint32 pendingPacked = (uint32(pending + 1) & 0x0FFFFFFF) | (uint32(snap) << 28); pending range -1..268,435,454.
    Portability: static_assert(is_always_lock_free) under __APPLE__ only (verified). x86_64 needs -mcx16, GCC does not
    report 16-byte atomics as always lock-free, and MSVC's are not lock-free. Documented, not built here (risk R2).

### Fork 2: per-clip runtime fields
- playing: Relaxed<bool>, and the render write-back becomes compare_exchange(expected = the value read at sync start).
  A trigger or pause landing inside syncMedia is then never overwritten (the F5/F13 lost update: an auto-played video
  stuck paused).
- playheadPosition: Relaxed<double>. The render reads the player value into a local for the out-point test, so there is
  no re-load.
- beatsPlayed: Relaxed<int>, render increments with fetchAdd, so a concurrent reset to 0 is never lost.
- hasBeenTriggered: Relaxed<bool>.
A mailbox for play requests: REJECTED (1b's reasons). Relaxed only (no CAS) for playing: REJECTED (the lost update stays).

### Fork 3: manual scalars (family D)
CHOSEN: the 23 manualRef fields become RelaxedFloat. They have a single writer (the message thread), so relaxed atomics
are exactly Sacred Rule 2. manualRef returns RelaxedFloat&, and ControlRef::manual becomes a ManualSlot
{RelaxedFloat* | float*} with load() / store().
atomic_ref at manualWriteCore / eff only: REJECTED. The other plain writers remain and still race; family C proves that
mixed plain / atomic_ref access still reports. TSan suppression: REJECTED (zero-app bar). Effect params / dryWet / source
params / macros stay plain floats: the sweep did not report them (residual R5).

### Fork 4: activeDeckIndex (family E)
CHOSEN: Renderer.cpp:469-495 derives the index from the acquire-loaded deck pointer (the arithmetic at :543-546), so no
GL read of the field remains. A null deck (fenced / none) keeps prevActiveDeckIndex_. A Relaxed<int> field would also be
acceptable but keeps a one-frame (new idx, old ptr) mismatch; not needed.

### Fork 5: std::cerr (family A)
CHOSEN: logLine(...) (new src/core/LogLine.h) formats into a local std::ostringstream and emits ONE std::fwrite to stderr.
The stdio FILE lock covers the write and no ios_base state is shared (VERIFIED 0 reports). Rule: code that can run off the
message thread never uses std::cerr; a textual lint pins the worker-thread file list.
- TSan suppression: REJECTED (it would hide a future setw race).
- std::osyncstream: availability in Apple libc++ ASSUMED-uncertain. REJECTED.
- A logger mutex: that is a new mutex. REJECTED.

### Fork 6: where the render loads the tuple (latency vs whole-column consistency)
CHOSEN: ONE load per layer, at the point where today's FIRST read of that layer happens (the layer's turn in the loop).
Latency stays identical to today for every layer.
Alternative: a top-of-frame snapshot pass over ALL layers. It would also remove today's one-frame "torn column" (when a
column trigger lands mid-loop, layers already drawn show the old clip and later layers the new one). But a late layer's
change would then show one frame later, with probability (its read offset / frame period). NOT taken: that is a timing
trade for Boris (Q2); the default is today's timing.

## (4) ITEMS
Commit order for the Builder: T0 first (RED evidence against 655d232's API), then T1..T8. Each commit after T0 builds and
passes the normal ctest, except the RED tests T0 names, which turn green in the item that fixes them. Never touch
EmbeddedShaders.h: its "u_crossfadeProgress" strings are GLSL, not the field.

### T0: RED-first tests (commit ONLY tests + CMake registration; compile against 655d232's API)
Files: tests/test_layer_runtime_race.cpp (NEW target; sources as test_autopilot + core/UndoManager.cpp, link set as
test_undo_commands, tests/CMakeLists.txt:523-551 / 693-703), tests/test_manual_scalar_race.cpp (NEW; link set of
test_manual_write, :1182-), D1 appended to tests/test_undo_commands.cpp, tests/test_log_line_lint.cpp (NEW; modelled on
test_hot_thread_io_lint.cpp, registration as :2773-2777).
Register the two race targets with `catch_discover_tests(<t> PROPERTIES LABELS tsan)` (ASSUMED syntax; verify that
`ctest -L tsan -N` lists them) and apply_sanitizers.
Use only APIs that exist on BOTH main and the lane:
- Layer::triggerClip / triggerClipImmediate / clearActiveClip / processPendingTrigger / getActiveClip / getClipAt;
- captureLayerRuntime / applyLayerRuntime / cancelPendingTriggers (DeckCommands.h);
- LayerClock::advanceCrossfade(Layer&, float), DeckClock::tick, Autopilot::processFrame, CrossfadeStartDetector::observe;
- TriggerClipCmd + UndoManager; resolveControl / manualWriteCore / eff();
- Clip fields in PLAIN assignment / read syntax (no compound operators such as +=).
Never read a tuple field directly in these tests.
- R1 [tsan] "message-thread triggers vs render clock / autopilot on one deck".
  Setup: a Deck of 3 layers x 4 columns, fades on (transitionSpeed 0.05); layer 2 autopilotEnabled, PlayNext at its
  shortest duration.
  Render thread loop: a FeatureSnapshot with totalBeatCount++ -> Autopilot::processFrame(deck). Per layer:
  LayerClock::advanceCrossfade(L, 0.004f), captureLayerRuntime, getActiveClip, CrossfadeStartDetector::observe(prev,
  active, progress). Then DeckClock::tick(deck, 0.004f, a clock that reads c->playing).
  Main thread, 20k iterations: before = capture; triggerClip(col, i%5==0 ? Beat : Off); after = capture;
  UndoManager::perform(TriggerClipCmd(before, after)); undo() every 7th; clearActiveClip() every 11th;
  cancelPendingTriggers(deck) every 13th.
  Post-join: deterministic range checks only.
  On 655d232 under TSan: data races on Layer.h fields -> exit 66 -> FAIL. In the normal build it passes on both.
- R2 [tsan] "clip runtime fields: trigger writes vs render transport write-back".
  Main thread: triggerClipImmediate(0/1), clearActiveClip, hasBeenTriggered = true.
  Render thread: a verbatim plain-syntax mirror of Renderer.cpp:1787-1839 on getActiveClip(): read playing, write
  playheadPosition, write playing back, and `c->beatsPlayed = c->beatsPlayed + 1` (the Autopilot.cpp:77 shape).
  On 655d232 under TSan: races on the Clip.h:228-231 fields -> FAIL. The render side is a MIRROR (Renderer.cpp is not
  linkable headless); the test says so. With Relaxed<> fields the same source becomes atomic operations.
- R3 [tsan] "manual scalar writes vs eff() reads".
  Main thread: resolveControl + manualWriteCore on Layer opacity / posX, Clip opacity, Comp opacity / speed (real code).
  Render thread: Layer::eff / Clip::eff / Composition::eff (real code).
  On 655d232 under TSan: FAILS (Layer.cpp:23 vs ManualWrite.cpp:187, as F18/F19).
- D1 (normal build) "a trigger's first perform does not restart a running fade".
  Layer with 2 clips, transitionSpeed 0.5. before = capture; triggerClip(1); after = capture;
  LayerClock::advanceCrossfade(L, 0.1f) (progress 0.2); UndoManager::perform(TriggerClipCmd(before, after));
  REQUIRE(capture.progress == Approx(0.2f)). Then undo gives before; redo gives after (progress 0).
  On 655d232 it FAILS deterministically: execute() re-stores progress 0 (TriggerCommands.h:68).
- D6 (normal build) test_log_line_lint: no `std::cerr` (line comments stripped) in T7's worker-thread file list.
  FAILS on 655d232 (AnalysisThread.cpp:82 etc.).
RED bar (T0 commit): in a TSan build, R1, R2 and R3 each exit non-zero with at least one
"WARNING: ThreadSanitizer: data race" whose stack has a src/ frame (Layer.h / Clip.h / Layer.cpp / ManualWrite.cpp). In the
normal build, D1 and D6 fail and everything else passes.
Risk: TSan must observe the pair. Tight loops make that reliable (the fresh RED and the scratch runs). A UB crash in R1 on
main also counts as RED.

### T1: foundations (Relaxed<T> and LogLine)
Files: NEW src/model/Relaxed.h, NEW src/core/LogLine.h, NEW tests/test_relaxed.cpp.
- Relaxed<T> mirrors LiveValue.h:17-38:
  - std::atomic<T> v; a constructor from T;
  - noexcept copy / move ctor + assign (relaxed load / store); operator=(T); operator T() const;
  - load(), store(), compareExchange(T& expected, T desired) (strong, relaxed), fetchAdd(T) for integral T;
  - static_assert(std::atomic<T>::is_always_lock_free);
  - NO compound operators (+=): a read-modify-write must be spelled fetchAdd, or load / store on purpose;
  - aliases RelaxedFloat / RelaxedDouble / RelaxedBool / RelaxedInt.
- logLine(const A&... a): std::ostringstream os; (os << ... << a); os << '\n'; one std::fwrite(s.data(), 1, s.size(), stderr).
- Tests (GREEN-only): copy / move preserve the value; implicit conversion + assignment; compareExchange success, and on
  failure it writes `expected`; fetchAdd; static_assert is_nothrow_move_constructible_v<Relaxed<double>>.
Risk: implicit-conversion surprises (R9). All are loud (compile errors, e.g. a juce::var built from Relaxed<bool> needs
.load()) except `auto x = field;`, which is an atomic copy and harmless.

### T2: the Layer tuple becomes LayerRuntimeCell (model + message-thread callers + undo)
Files:
- src/model/Layer.h (+ Layer.cpp if out-of-line);
- src/core/DeckCommands.h (185-217, 241-278, 393-418 comment); src/core/TriggerCommands.h (20-36 comments, 68-69, 104-113);
- src/model/Deck.h (111-119);
- src/MainComponent.cpp (203-207; 621 / 631 reads; 757-766; 4590-4795; 4814-4895; 5519-5525; 5564-5577; 6905-6915;
  7536-7542; 7590-7597; 7628-7634);
- src/core/CompositionLoad.h (163-164, 202); src/recording/PerfStateCapture.cpp (70-74); src/api/ApiServer.cpp (418-421);
  src/midi/MidiOutputHandler.cpp (84); src/ui/DeckView.cpp (191, 284);
- every test that touches the 5 fields: test_undo_commands 143 refs, test_deck_clock 36, test_autopilot 21,
  test_composition 12, test_program_preamble 5, test_crossfade_history 5, test_deck_thumbnails 4, test_recorder_host 3,
  test_compositor 3, and 2 layer-strip tests.
Behaviour change (the exact spec):
- LayerRuntimeSnapshot (same name, same 5 fields, same operator==) MOVES from DeckCommands.h:185-201 into Layer.h. It is
  an aggregate, so tests may write layer.setRuntime({.activeClipColumn = 1, .crossfadeProgress = 0.5f}).
  captureLayerRuntime(layer) becomes layer.runtime(), and applyLayerRuntime(layer, r) becomes layer.setRuntime(r). Both
  keep their signatures as the compat surface.
- Layer gets `LayerRuntimeCell runtime_` in place of the 5 fields, with this API:
  - runtime(): ONE acquire load;
  - setRuntime(r): release store;
  - casRuntime(expected&, desired): acq_rel, one attempt;
  - updateRuntime(fn, maxAttempts = 0 /*unbounded*/) -> LayerRuntimeTransition{before, after}; before == after when fn
    changed nothing.
- triggerClip / triggerClipImmediate / clearActiveClip / processPendingTrigger return LayerRuntimeTransition (statement
  callers still compile). Each one:
  1. reads the clip config once (target exists? beatSnapMode / beatSnap; transitionSpeed);
  2. calls updateRuntime with the PURE tuple function (queue / immediate new / immediate retrigger / clear; exactly the
     logic of Layer.h:223-352);
  3. THEN applies the clip effects to the final transition's target: playheadPosition = inPoint and beatsPlayed = 0 for
     new and retrigger; playing = true only for a new activation with !hasBeenTriggered. clearActiveClip sets the OLD
     active clip's (t.before) playing = false.
  clearActiveClip(std::optional<int> onlyIfActive) clears only if active == that column (one CAS).
- getActiveClip() / getActiveClip() const: ONE load, then getClipAt(col).
- processPendingTrigger(beatInBar, barCount, maxAttempts = 4) is CAS-guarded, so a pending trigger cancelled on the
  message thread (deck switch, clear) can never fire afterwards.
- cancelPendingTriggers / applyPendingTriggerCancellation call updateRuntime per layer; the returned list is the exact CAS pair.
- Deck::triggerColumn(col, forcedSnap, std::vector<std::optional<LayerRuntimeTransition>>* out = nullptr).
- MainComponent callers:
  - handleClipTrigger: rtBefore / rtAfter come from the returned transition; wasRetrigger = (t.before.activeClipColumn ==
    column); the hasBeenTriggered / preview block (4661) uses getClipAt(t.after.activeClipColumn);
  - handleColumnTrigger: per-layer before / after from Deck::triggerColumn's out vector (the capture loop keeps its
    playBefore / autoPlays reads);
  - X clear (757-766) and 6908-6915: before / after from clearActiveClip's transition;
  - momentary release 7593-7595 / 7632-7633: clearActiveClip(resolvedColumn), with no separate check first.
- TriggerClipCmd::execute and ClearActiveClipCmd::execute run updateRuntime(r -> sameIntent(r, after_) ? r : after_),
  where sameIntent compares active / previous / pending / snap and NOT progress. undo() calls setRuntime(before_). The
  first perform is therefore a no-op (the live trigger already produced `after`), and a redo after an undo applies
  `after`. Retire the "status-quo field race" comments (TriggerCommands.h:34-36, DeckCommands.h:393).
- Copy / move: LayerRuntimeCell has noexcept copy / move ctor + assign that load / store (the LiveValue pattern). Layer,
  Deck and Composition keep their implicit copy / move, and vector<Layer> reallocation (fenced) stays move-based. The
  tuple is NOT serialized (Layer.cpp toVar / fromVar never name it: VERIFIED by grep), so there is no format change;
  Deck::fromVar / CompositionLoad produce the default tuple as today.
RED: R1 (TSan), D1 (normal). GREEN: R1 exits 0 with 0 warnings under TSan; D1 passes; all migrated tests pass.
NEW GREEN-only tests (tests/test_layer_runtime.cpp, normal build; the teeth are the named mutants):
- pack / unpack round trip for active / previous in {-1, 0, 9999, 10000, INT32_MAX}, pending in {-1, 0, 9999, 268435454},
  and every BeatSnapMode;
- transition pair exactness for queue, immediate new, retrigger, clear, and clear-if-mismatch (a no-op);
- a render-style casRuntime with a stale expected FAILS after a trigger and leaves the trigger's tuple intact (mutant:
  casRuntime as a plain store fails this);
- processPendingTrigger after cancelPendingTriggers does nothing;
- Layer copy / move keep the tuple;
- static_assert(std::is_nothrow_move_constructible_v<Layer>). Check first that it holds on 655d232; if it does not, keep
  the lane's value equal to main's and drop the assert.
Risk: the test migration can change a test's meaning (review each non-mechanical edit). Any caller that read
activeClipColumn and previousClipColumn separately now gets them from one snapshot, which is the intent.

### T3: render path (one load per layer, one CAS per fade tick)
Files: src/render/LayerClock.h, src/render/DeckClock.h, src/render/CompositorEngine.{h,cpp} (advanceCrossfade 969-975,
incomingImagePending 395-405, renderLayerStages 977-993, compositeDeck 1040-1135, compositePersistentLayers ~1300-1340,
applyTransition 1638-1745), src/render/Renderer.cpp:588-595, src/render/CrossfadeHistory.h (comment 22-31),
src/model/Autopilot.cpp (14-36, 52-59, 67-105; the advanceClip / smartAdvanceClip callers pass the snapshot's column).
Behaviour change:
- LayerClock gets:
  - `advanced(LayerRuntimeSnapshot, float transitionSpeed, float dt)`: pure, the LayerClock.h:19-27 math;
  - `bool tick(Layer&, LayerRuntimeSnapshot& rt, float dt)`: next = advanced(rt); if next == rt, return true. Otherwise
    one casRuntime: on success rt = next; on failure rt = the concurrent tuple and it returns false.
  advanceCrossfade(Layer&, float) stays (load + tick) for DeckClock / tests.
- compositeDeck / compositePersistentLayers, per layer:
  `auto rt = layer.runtime(); const Clip* clip = layer.getClipAt(rt.activeClipColumn);`
  `if (!incomingImagePending(layer, rt, clip) && !LayerClock::tick(layer, rt, dt)) clip = layer.getClipAt(rt.activeClipColumn);`
  Pass `const LayerRuntimeSnapshot& rt` into renderLayerStages (observe(rt.previous, rt.active, rt.progress)) and into
  applyTransition (rt.progress / rt.previous; getClipAt(rt.previous), getClipAt(rt.active); the u_crossfadeProgress
  uniform = rt.progress). The load sits where today's first read of that layer sits (Fork 6), so latency does not change.
  hasActiveLayers / hasPersistentContent keep getActiveClip() (one load).
- DeckClock::tick: rt = runtime(); tick (unless fadeOwnedElsewhere); then active / previous from rt. This keeps the
  "advance first" order of DeckClock.h:28-39.
- Renderer.cpp:593-595: one runtime() per layer.
- Autopilot: one runtime() per layer; `clip->beatsPlayed.fetchAdd(1) + 1 >= loopsTarget` (25-28) and fetchAdd(beats)
  (77); the pending check uses the snapshot. Its Layer::triggerClip calls pass maxAttempts = 4; if the attempts run out,
  the advance is retried at the next frame / beat (self-healing).
RED: R1 (TSan), the render half of the same pairs.
GREEN: R1 clean under TSan; test_deck_clock, test_compositor, test_crossfade_history and test_autopilot keep their
meaning. NEW GREEN-only test: tick() on a stale snapshot after a concurrent trigger returns false, adopts the trigger's
tuple, and never clears the new fade's previous. Mutant: a plain store in tick makes the new fade lose previous, and the
test fails.
Probe parity (after merge, Harmony): probe-crossfade.sh, probe-render-state.sh r1_*, probe-deck-clock.sh (B1 / B2 rows).
Risk: a missed read site that still takes its own load (a consistency drift, not a race). The reviewer greps the
compositor loop bodies for runtime() / getActiveClip() and must find exactly one load per layer.

### T4: Clip runtime fields + the CAS write-back
Files: src/model/Clip.h (228-231, 321-323, 363-365), NEW src/render/ClipTransportSync.h (pure, templated on the
player), src/render/Renderer.cpp (syncMedia: video 1784-1845, sequence 1874-1925), src/ui/LayerStrip.cpp (747-750),
src/model/Autopilot.cpp (via T3), the comment in src/connect/ConnectionEngine.h (the "s166 spec L5" note),
src/api/ApiServer.cpp (a juce::var built from a Relaxed field needs .load()).
Behaviour change:
- Fields: `mutable RelaxedBool playing`, `mutable RelaxedDouble playheadPosition`, `RelaxedInt beatsPlayed`,
  `RelaxedBool hasBeenTriggered`.
- syncMedia:
  1. `const bool wanted = clip->playing.load()` ONCE; push it to the player as today.
  2. After the advance: `const double ph = player->getPlayheadPosition(); clip->playheadPosition.store(ph);`
  3. Write-back: `bool e = wanted; clip->playing.compareExchange(e, player->isPlaying());` (skipped when the intent
     changed).
  4. The out-point test uses `ph`. The OneShot stop CASes the value it just wrote.
  The sequence twin gets the same treatment.
- LayerStrip.cpp:750 becomes `clip->playheadPosition.load()`. Everything else compiles through the implicit conversions
  (~30 playing sites, ~25 playhead sites, VERIFIED by grep).
Retrigger analysis (Harmony's question):
- The write-back at :1823 cannot lose a restart that was ISSUED. The restart is the player seek
  (MainComponent.cpp:4701-4711), and :1823 copies the player's clock, which the seek has already moved (Pitfall 56:
  seekTo posts a request from any thread). INFERRED that getPlayheadPosition reflects the seek from the next advance on;
  VideoPlayer was not read today (gop2's file).
- A retrigger that does NOT restart exists regardless of the race:
  - column retrigger (no seek, 4898-4941);
  - a Replay retrigger on a non-active deck (the :4658 gate);
  - an autopilot same-column retrigger (Layer.h:268 model store only).
  All unchanged here (parity); see Boris Q1.
RED: R2 (TSan; F5 / F13 / F16 / F22 and family C).
GREEN: R2 clean. NEW GREEN-only tests in tests/test_clip_transport_sync.cpp, using a FakePlayer:
- a trigger that sets playing = true between the sync read and the write-back survives (mutant: a plain store makes
  this fail);
- a OneShot stop with an unchanged intent writes false;
- a pause landing inside the window survives.
Risk: compile fallout from the implicit conversions (loud). `std::atomic_ref<double>` sites must be converted
(LayerStrip.cpp:750).

### T5: manual scalars (family D)
Files: src/model/Layer.h (62, 146-151), Layer.cpp (4-24), src/model/Clip.h / Clip.cpp (4-16 + the 7 fields),
src/model/Composition.h (the 9 fields, 700-716), src/connect/ManualWrite.{h,cpp} (ControlRef 26-35, resolveScalar,
179-188), src/MainComponent.cpp:2055-2058, src/connect/ConnectionEngine.cpp (its manualRef use).
Behaviour change:
- The 23 fields become RelaxedFloat, and manualRef returns RelaxedFloat&.
- ControlRef::manual becomes a ManualSlot {RelaxedFloat* atomicField; float* plainField; load(); store(v);
  explicit operator bool}. The plain pointer stays for effect params / dryWet / source params / macros.
- manualWriteCore calls r.manual.store(...).
- eff() keeps its shape; its argument becomes an atomic load.
RED: R3 (TSan).
GREEN: R3 clean; test_manual_write / test_connection / test_composition_tier_oracle keep their meaning.
Risk: std::tie / make_tuple in Clip.h:265 / 315 should compile through operator= / the copy ctor (INFERRED OK). A
serializer that passes a field to varargs fails to compile (loud).

### T6: activeDeckIndex (family E)
Files: src/render/Renderer.cpp:467-495, plus an optional pure helper `std::optional<int> deckIndexOf(const Deck*, const
std::vector<Deck>&)` in a render header with a 4-case unit test.
Behaviour change: under `if (composition_ && deck)`, index = deck - decks.data() when the pointer is inside the range (as
at :543-546); it is compared with prevActiveDeckIndex_ exactly as today. The GL thread no longer reads
composition_->activeDeckIndex.
Parity: identical at every unfenced frame, because the renderer's pointer is re-pointed after every index write
(MainComponent.cpp:5525-5529, UndoService restore). The removed one-frame (new idx, old ptr) window only started a
transition one frame early, from the same held picture (verify-sweep-F27).
RED: app sweep scenario c, Family E key (Renderer.cpp:471 vs MainComponent.cpp:5525; fresh RED 3/3 c launches).
GREEN: 0 Family E on the lane arm; probe-deck-tabs.sh / probe-deck-path.sh parity (deck switch transitions).

### T7: std::cerr on worker threads -> logLine (family A)
Convert EVERY std::cerr statement in these files; message-thread-only files stay as they are:
- analysis/AnalysisThread.cpp (4), api/ApiServer.cpp (3, incl. the :93 / :96 thread lambda), test/TestServer.cpp (4);
- render/Renderer.cpp (20), render/ImageDecode.h (2), render/TextureManager.cpp (1), render/LUTLoader.cpp (3);
- recording/VideoRecorder.cpp (16), core/MediaOpener.cpp (1), media/ImageSequence.cpp (1);
- output/SyphonOutput.mm (2), output/SharedFrameSet.cpp (2), output/OutputPresenter.cpp (2);
- media/VideoPlayer.cpp: ONLY the std::cerr statements inside open() (:83-316). gop2's hunk :276-297 must not be touched.
The Builder confirms, by reading each file, that its code can run off the message thread; if unsure, convert. The text
output stays byte-identical, except that each line is now written whole.
Lint (D6) covers the list above minus VideoPlayer.cpp (a whole-file lint is impossible until gop2 merges), plus a comment
naming the post-merge residual: VideoPlayer.cpp outside open() (gop2), audio/AudioEngine.cpp and audio/DeviceGuard.cpp
(bt2, including its new lines).
RED: D6 (normal build) + app sweep Family A (F1 on 9/9 RED launches; VideoPlayer "Opened" pairs in b/c).
GREEN: D6 passes; 0 Family A on the lane arm.

### T8: the TSan gate tooling (in-repo probe) + docs
Files: NEW .harmony/probe-tsan.sh + .harmony/probe-tsan.py (scenarios) + .harmony/probe-tsan-analyze.py, adapted from
.harmony/.reports/s-rta-0930/tsan-harness/ (run_batch.sh / scen.py / analyze.py / mkfix.sh):
- the PROBE RIG GATE section and the adna_pids helpers cloned from .harmony/probe-crossfade.sh;
- SPEC items `N:scen@arm`, with the app taken from TSAN_APP_<arm>; launches.tsv gains the arm;
- fixtures via mkfix.sh into the out dir (or SWEEP_MEDIA);
- TSAN_OPTIONS halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:log_path=<run>/tsan (unchanged);
- every request Connection: close; open -g only; no Output window (outwins check kept); no synthetic input.
Scenario d (NEW: fades + storms):
- 2 decks x 3 layers x 4 columns (videos v1080 + images); every layer transitionSpeed 0.5; one clip with beatSnapMode Beat
  (keys per Clip::toVar).
- Steps every 0.25 s for ~12 s: trigger_clip (including the active cell = retrigger, and an empty cell = clear),
  trigger_column, set_layer_opacity, switch_deck, GET /api/composition, and perf/record -> 3 triggers -> perf/stop ->
  perf/play (as probe-routines.sh drives them).
- NEVER load_composition while /api/composition is being polled (risk R7).
Analyzer: a unique report is APP if any frame of either ACCESS stack has a file under <tree>/src (union of both arms' src
basenames); otherwise it is JUCE/SYSTEM. Print both lists with stacks, per arm and per family (key table: A cerr, B Layer
tuple / playing, C playheadPosition, D manual scalars, E activeDeckIndex).
Docs: section 6.
RED / GREEN: section 5, G3.

## (5) GATES (Harmony runs these after the merge; bars pre-registered)
G1 Normal build + full ctest: `cmake --build build --config Release -j8 && ctest --test-dir build -j8`.
   Bar: 100 % pass; count = 1049 + the new cases; the new targets present (test_layer_runtime_race,
   test_manual_scalar_race, test_layer_runtime, test_clip_transport_sync, test_relaxed, test_log_line_lint).
G2 TSan ctest RED / GREEN.
   RED arm: `git worktree add <scr>/wt-red <T0 sha>`; configure with the prebuild recipe (-DADNA_SANITIZE=thread,
   RelWithDebInfo) into <scr>/build-tsan-red; build the two race targets;
   `TSAN_OPTIONS=halt_on_error=0:abort_on_error=0 ctest --test-dir <scr>/build-tsan-red -L tsan --output-on-failure`.
   GREEN arm: the same on merged main (<scr>/build-tsan-lane).
   Bars: RED is valid if and only if every [tsan] case fails with at least one data race carrying a src/ frame. GREEN
   holds if and only if every [tsan] case passes with 0 "WARNING: ThreadSanitizer".
   INFO: full TSan ctest on both arms; the lane must not ADD a failing case.
G3 TSan app sweep.
   Apps: arm main = the TSan build of the actual pre-merge commit, from its cmake build dir, never a copied bundle. If
   tsan merges first, that commit is 655d232, already built at
   <scratchpad>/build-tsan-main/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app. Arm lane = a fresh TSan build of merged
   main.
   Order (interleaved, one live app at a time, lock held per batch): 3 rounds of main-a, lane-a, main-b, lane-b, main-c,
   lane-c, main-d, lane-d = 24 launches.
   Pre-registered decision:
   - RED (validity): the main arm shows at least one report of each family B, C, D and E across its 12 launches (A is
     expected in every launch). If a family is missing, run 3 more main-c launches before judging; if it is still
     missing, record INFO (not a lane failure).
   - GREEN (the gate): the lane arm shows 0 APP reports across its 12 launches. Any APP report = FAIL (no benign
     exceptions). JUCE/SYSTEM-only reports are listed with stacks; one seen on the lane arm and not on the main arm is
     flagged for review.
   - Validity per launch (both arms): health up, alive at the end, graceful quit, Output-named windows 0, UNC dialogs 0,
     new .ips 0, built-in audio pre-check passed. A failed launch is re-run, never counted.
G4 Behaviour parity on a normal Release build of merged main, using each probe's own pre-registered rows. Any row green
   on 655d232 and red on merged main = FAIL. Probes: probe-crossfade.sh, probe-render-state.sh (r1_*), probe-deck-clock.sh
   (B1 / B2), probe-step3.sh, probe-routines.sh, probe-routine-display.sh, probe-async-load.sh (a4 trigger-in-window),
   probe-lane3.sh (manual scalars), probe-mastersignal.sh, probe-deck-tabs.sh, probe-deck-path.sh.
G5 Latency: a design invariant checked by review, not a run. The render's per-layer load sits where today's first read
   sits (Fork 6), and the message-thread write is one CAS, visible to the next load. Reviewer bar: no top-of-frame
   snapshot pass, no queue, no fence on a trigger path.
G6 Perf: INFO only; the expected effect (ldp + dmb per layer per frame) is far below run-to-run drift. Interleaved
   main / lane Release launches, at least 5 per arm, with the scenario b and d loads; medians of /api/state fps,
   peak_frame_time_ms and peak_callback_ms. Check ps for orphaned CPU burners first. Reported, never gating.

## (6) DOCS
- docs/claude/architecture.md:
  - Core Data Structures, Clip table (:96): playheadPosition -> Relaxed<double>; add playing as Relaxed<bool>.
  - Lock-Free Communication Chain (:100-110) gains "Model fields shared across threads":
    - single-writer scalars are Relaxed<T>;
    - the Layer trigger tuple is one 16-byte CAS word (LayerRuntimeCell) written by both threads;
    - the render loads it once per layer per frame and publishes a fade tick with one CAS (adopt-on-fail);
    - a render write-back of a message-thread intent is a CAS on the value it read;
    - structure stays behind withDeckDetached;
    - worker threads log with logLine, never std::cerr (test_log_line_lint).
  - Threading Deep-Dives: a pointer to the new pitfall.
- docs/claude/pitfalls.md: NEW "Pitfall NN" (Harmony assigns; next free is 63): "The Layer trigger tuple is ONE atomic
  word; model fields another thread reads are Relaxed<T>". Rules:
  - never add a plain field that the GL thread reads and the message thread writes;
  - never read two tuple fields separately on the GL thread (one runtime() per layer);
  - never write the tuple except through the Layer API / setRuntime;
  - a render write-back is a CAS against the value read;
  - mutate-then-push commands compare intent before re-applying.
  Guards: test_layer_runtime_race / test_manual_scalar_race (TSan label), test_layer_runtime, test_clip_transport_sync,
  test_log_line_lint; live .harmony/probe-tsan.sh.
- docs/claude/performance-controls.md, the beat-snap / Quantize paragraph: a queued trigger lives in the tuple word; a
  cancel is a CAS and can never be outrun by a beat.
- docs/claude/rendering.md :67 (Crossfades): observe() receives one consistent snapshot per layer per frame.
- CLAUDE.md (23,999 / 25,000 B):
  - line 74, Sacred Rule 2: append " (model fields: `Relaxed<T>`, Pitfall NN)" after "UI->hot path via `std::atomic<T>`";
  - pitfall index: add "NN. The Layer trigger tuple is one CAS word; shared model fields are `Relaxed<T>` -- before
    touching Layer runtime fields or a render write-back.";
  - line 118 (Transport state) stays: still true (playing is still mutable and still written by the render for OneShot
    stop).
  Pay for it by shrinking line 128 (Deck tab row, 285 B, already verbatim at performance-controls.md:51) to a one-line
  pointer. If that is not enough, trim the Outputs paragraph (its rules live in docs/claude/integration.md).
  Bar: `wc -c CLAUDE.md` <= 23,999 after the edit.
- Comments retired / rewritten: TriggerCommands.h:34-36, DeckCommands.h:393, CrossfadeHistory.h:22-31, LayerStrip.cpp:747-749,
  ConnectionEngine.h (the L5 note), Renderer.cpp:449-451 (the "house class" comment no longer covers activeDeckIndex).
- .harmony/APP-INVENTORY.md counts: Harmony's.

## (7) RISKS (named, each with the cheapest test that tells them apart)
R1 The render-path refactor drifts (a stray second load): a consistency problem, not a race. Test: the reviewer's grep
   of the compositor loop bodies, plus G4 probe-crossfade / probe-render-state / probe-deck-clock.
R2 16-byte atomics off macOS: x86_64 needs -mcx16, GCC does not report them as always lock-free, MSVC's are not
   lock-free. The static_assert is under __APPLE__ only; docs note. Test: none possible on this machine.
R3 CAS contention: the render tries once; autopilot / beat snap try at most 4 times, then wait for the next frame. The
   message loop is unbounded, but the render writes at most a few times per layer per frame. Test (INFO): an optional
   relaxed counter of render adopt-on-fail events, read in scenario d (expected ~0 per trigger).
R4 execute()'s intent comparison changes the redo / first-perform semantics. Test: D1 plus the full test_undo_commands suite.
R5 NOT FIXED (outside the 5 families, not driven by a/b/c/d): the config-scalar class that the GL thread reads while the
   UI writes it:
   - Layer: transitionSpeed / visible / bypassed / solo / type / blendMode / keying / feedback;
   - Clip: speed / reverse / loopMode / in / out / beatSnapMode, presetBeatsPlayed;
   - EffectSlot paramValues / dryWet, source params, macros;
   - Composition: outputWidth / outputHeight / globalTransitionSpeed / quantizeMode.
   TSan will report them when a slider moves during rendering. Follow-up lane: the same Relaxed<T> wrapper, mechanical.
   Cheapest witness: an INFO scenario with /api/set_param during render.
R6 The fenced startup frame with no canvas reads Composition scalars during a swap (2.6 ii). The comp manual scalars
   become atomic (T5) and activeDeckIndex is no longer read (T6); outputWidth / Height / globalTransitionSpeed stay plain (R5).
R7 NEW, pre-existing, not TSan-found: /api/composition and /api/state iterate the live decks / layers / clips / strings
   on the httplib thread with no fence (ApiServer.cpp:380-440). A composition swap or deck edit during a poll is a
   use-after-free hazard, and the probes poll this route heavily. Not in this lane: the fix is to marshal the read to the
   message thread with a bounded wait (ApiServer::stop() must not wait on the message thread, Pitfall 58). Gate
   scenario d never loads while polling.
R8 Residual std::cerr (VideoPlayer.cpp outside open(), AudioEngine.cpp, DeviceGuard.cpp, bt2's new lines): a post-merge
   mechanical sweep adds them to the lint list.
R9 Relaxed<T> implicit conversions: every surprise is a compile error, except `auto x = field` (an atomic copy on the
   stack, harmless). Test: the build.
R10 TSan sees only the interleavings that run, so GREEN 0 is evidence, not proof. The unit tests supply the deterministic
   failures (D1, the CAS mutants), and scenario d adds the fade path that a/b/c never drive.
R11 Consistency is per layer only: a column trigger landing mid-loop still shows one torn frame (pre-existing; Fork 6, Q2).
R12 Adopt-on-fail: in the rare frame where the render's tick CAS fails, it shows the concurrent tuple un-advanced, so a
   fade runs one frame longer (8 ms). Invisible.
R13 Merge conflicts:
   - MainComponent.cpp: bt2 ~490 / 2148-2150 / 5697-5699 vs this lane's regions listed in T2;
   - ApiServer.cpp: bt2 ~323 / ~2086 vs 85-100, 380-440, 1425;
   - VideoPlayer.cpp: gop2 276-297 vs the cerr statements in open();
   - tests/CMakeLists.txt: appends; pitfalls.md / CLAUDE.md index: adjacent appends.
R14 Strongest argument against the whole design: "a mailbox (single writer) is simpler to reason about." It would be, if
   undo did not need the result synchronously and the render always ran. Both are false here (2.1, Fork 1b).

## (8) WHAT ONLY BORIS CAN CHECK
- A live set (5 min, real music, Launchpad / MIDI + keyboard, not REST), with fades on:
  - firing columns and clips never produces an unexpected hard cut instead of a fade;
  - a never-played video auto-plays when first triggered;
  - retriggering the playing cell restarts it as before;
  - Undo right after a trigger does not make the fade jump or restart;
  - a deck switch mid-fade finishes the fade.
- Nothing visible should change; any difference is a bug in this lane.

## (9) QUESTIONS FOR BORIS (with defaults; the build does not wait)
- Q1 A column retrigger (firing the column that is already playing) does NOT restart its videos, while clicking a single
  playing cell does. Should firing the same column again restart every clip in it from its start? DEFAULT: keep today's
  behaviour; filed.
- Q2 When you fire a whole column, today a layer can switch one frame (1/120 s) before the others. Should all layers
  switch on exactly the same frame, even if that makes some layers one frame later? DEFAULT: keep today's timing.

STATUS: DONE

## HARMONY ADOPTION (s-rta-1002, 2026-10-02 08:07:54)
ADOPTED: the section "## ARCHITECT RULING (s-rta-0930)" of .harmony/.reports/s-rta-0930/ruling-tsan.md IN FULL (amendments 1-18,
"FINAL GATES (pre-registered; override plan (5))" G1-G6, the risks). Where the ruling differs from this plan body, the RULING wins.
The ruling ruled the three blind seats' papers (verbatim in attack-tsan-papers.md). These Harmony decisions override both where
they differ:
- H1 BASE = main 02b2913, not 655d232. VERIFIED (git diff --stat 655d232 HEAD -- src tests): only gop2's files moved
  (src/media/VideoPlayer.cpp / .h, src/media/GopCache.h, tests/test_gop_cache*.cpp, three fixtures). Every file:line the plan /
  ruling cites outside VideoPlayer.cpp is unchanged; re-locate VideoPlayer.cpp lines by content. Baseline ctest = 1055 cases
  (not 1049), 111 Catch2 targets registered. G1 total = 1055 + the sum of the new per-target counts the Builder records.
  Every "RED on 655d232" in the ruling means RED on the lane's base / T0 commit.
- H2 Boris questions (s-rta-0930 page items 3-5, unanswered) -> defaults: plan Q1 (column retrigger restarts its videos?) =
  keep today's behaviour, no code, filed; plan Q2 (column switch on one frame?) = keep today's per-layer load (Fork 6);
  ruling Q3 (momentary + Quantize released before the beat) = CANCEL: amendment 10 is ADOPTED (releaseMomentary, its three
  tests, its mutant, the performance-controls.md line, Boris live-check line "a momentary pad released before its beat does
  not latch on").
- H3 T7 WIDENED: gop2 has merged, so the reason to exclude VideoPlayer.cpp is gone. Convert EVERY std::cerr statement in
  src/media/VideoPlayer.cpp (14 on 02b2913, decode / MediaOpen / message threads alike) and put the whole file in the D6 lint
  list. R8 residual = src/audio/AudioEngine.cpp + src/audio/DeviceGuard.cpp ONLY (lane bt2 owns them now; never touch them
  here; a post-merge sweep converts them). G3.2's PRE-EXISTING-R8 class shrinks to those two files.
- H4 The new pitfall is "Pitfall 63" (write the number; do not write NN). CLAUDE.md is 23,996 B on the base; bar <= 24,999 B
  after the edit (the 25,000-B cap), paid for as plan (6) says.
- H5 FENCES vs lane bt2 (built concurrently on branch lane/bt2; merge order not fixed). This lane NEVER edits: src/audio/*,
  tests/test_device_policy.cpp, tests/test_audio_engine_devices.cpp, .harmony/probe-btguard.sh, Pitfall 61, the btguard rows
  of docs/claude/testing-eyes.md. src/MainComponent.cpp lines owned by bt2 on 02b2913: 484-494 (onDeviceStateChanged /
  notice wiring), 2144-2160 (the debug wiring inside #if AUDIODNA_TEST_SERVER), 3070-3095 (refreshAudioDeviceNotice),
  5690-5705 (perfRecord comment) -- no hunk of this lane may touch them (a compile-forced edit there is a named deviation in
  the report). src/api/ApiServer.{h,cpp}: bt2 owns the debug route list (~323), the handlers after ~2086, ApiServer.h ~204-208
  / ~272; this lane's hunks are :93-96 (cerr), 360 / 384 / 418-421, /api/state 1370-1544 (amendment 17).
  tests/CMakeLists.txt: bt2 appends after the test_device_policy block (~:518); this lane appends its targets at the END of
  the file only.
- H6 LANE SPLIT (bounds each builder's context): three sequential builders on ONE branch lane/tsan in worktree
  .claude/worktrees/tsan. B1 = T0 + T1 + T2. B2 = T3 + T4 + T5. B3 = T6 + T7 + T8 + docs (plan (6) + amendment 14) + the
  amendment-16 mutant evidence + the lane smoke (H8). Each builder continues from the previous head (no reset), reads the lane
  report so far, appends its own section, and commits per item.
- H7 TSan builds in the lane: build dir <worktree>/build-tsan, configured with the prebuild recipe (RelWithDebInfo,
  -DADNA_SANITIZE=thread, TEST_SERVER ON, SYPHON ON, FETCHCONTENT_FULLY_DISCONNECTED=ON, FETCHCONTENT_SOURCE_DIR_<DEP> ->
  main's build/_deps/<dep>-src incl. MELATONIN_INSPECTOR; template .harmony/.reports/s-rta-1002/evidence-0930/prebuild/
  configure.sh, but -S <worktree>). B1 runs amendment 2's T0 RED bar on the T0 commit (both builds) and pastes it verbatim;
  each later item runs its GREEN.
- H8 LANE SMOKE (B3, under the live lock): ONE lane-d launch of the lane's TSan app with the in-repo probe-tsan tooling ->
  launch validity + the final /api/state render_pending_fired >= 1 AND render_autopilot_advances >= 1 (G3.4), pasted; plus the
  analyzer over the fresh-RED archive .harmony/.reports/s-rta-1002/evidence-0930/sweep-red1-archive/runs: it must key the 25
  uniques into families A-E (table pasted). G1-G6 themselves are Harmony's, after the merge.
- H9 Evidence the plan / ruling cite under the old scratchpad (44b528dd: a128, logline, tsanflags, prebuild,
  sweep-red1-archive) is mirrored, text only, at .harmony/.reports/s-rta-1002/evidence-0930/ -- use the mirror if the old
  path is gone.
- H10 Filed, not built here: follow-up lane "tsan-r5" (R5 config scalars incl. the perf/play preamble writes, R7 the unfenced
  httplib reader), starting from scenario e's reports.
- H11 (2026-10-02 08:49:46, ruling on B1's flagged deviation) amendment 2's normal-build T0 bar is AMENDED: D1, D1b, D1c, D6 and lint case 1
  MUST fail; R1 and R4 MAY also fail in the normal build on the base (their value invariants -- I1 / range / I3 -- catch the
  torn tuple and the lost trigger without TSan: that is RED with more teeth, not a weakened test); everything else passes.
  G2's RED arm is unchanged (TSan exit 66 + a src/ data race per [tsan] case).
- H12 (2026-10-02 08:49:46) RIG: two lanes' concurrent full ctest runs collide on fixed temp names (test_preset_manager's
  preset_manager_test_<suffix>.json, test_app_settings' audiodna-test-app-settings): 464 / 471 / 472 / 821 / 823 failed in
  B1's T0 run and passed isolated. From now on every FULL ctest run takes the cross-lane mutex
  (until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; run; rm -rf /tmp/audiodna-ctest.lock), and a failure in
  those targets is re-run isolated before any verdict.
- H13 (2026-10-02 11:08:24) FIX-ROUND RULINGS over the round-1 reviews (review-tsan-{memmodel,tests,render}-r1.md): tests M1 (R1/R2/R3 false-fail
  under load: start handshake) MUST; memmodel S1 RULED MUST as the GUARD fix (a render-side trigger decided from a snapshot is a
  no-op when the tuple no longer names the column it decided from) -- not the comment softening; render SHOULD-1 RULED MUST
  (Sacred Rule 3: the analysis thread's steady-loop logging is zero-heap via a stack-buffer formatter); tests S1 / S2 / S3,
  memmodel S2 / S3, tests N1 fixed. Full text: .harmony/.reports/s-rta-1002/tsan-fix-rulings.txt.
