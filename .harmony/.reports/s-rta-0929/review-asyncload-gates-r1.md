# Reviewer Verdict — asyncload-gates-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED_COMMIT: fb0e455 (base adf9b8a), worktree .claude/worktrees/rta0929-asyncload branch lane/asyncload

## Scope of this pass
Read-only, disk-verified: git show/diff only (never the working tree). Cross-checked plan-asyncload.md body +
HARMONY ADOPTION AL1-AL12 against the actual diff (src/core/{LoadTicket,LoadTiming,StagedLoad,MediaOpener}.{h,cpp},
MainComponent.{h,cpp}, ApiServer.{h,cpp}, TestServer.{h,cpp}, audio witness files, VideoPlayer.h comment, 3 new ctest
files, tests/CMakeLists.txt, CMakeLists.txt, probe-async-load.{sh,py,json}, CLAUDE.md + docs) and against the lane
report .harmony/.reports/s-rta-0929/asyncload.md.

## FOCUS items verified
1. **Gate rows / ctest RED**: RED transcripts on main (adf9b8a) and the commit-1 app in the report are internally
   consistent with the source (e.g. a1 fields absent pre-commit-4, a2/a2b stall+order fails, a3/a3b/a3c cancel fails,
   a4 window-too-short + leak-into-V32, a5/a5b duplicate fails, a6/a7 fails, end_quit_hung_open fails on main
   including the real `phase2_hung_open: killed` line — matches main lacking AL2's leak-not-kill path). ctest counts
   verified by direct grep: test_load_ticket.cpp 3 TEST_CASE, test_staged_load.cpp 12, test_media_opener.cpp 7 = 22,
   matching the claimed 920+22=942.
2. **ctest determinism (AL3)**: `tests/test_media_opener.cpp` (c1) uses a real start-latch (`Gate::arriveAndWait`,
   `waitArrived`) to force both jobs RUNNING before `cancel()` → deterministic Stale==2; (c2) uses blocker jobs
   (`addBlockerJobForTests`) to hold both pool threads so the real jobs stay QUEUED before `cancel()` →
   deterministic `dropped()==2`, no landing. No sleep is used as the actual synchronization primitive; the one fixed
   `sleep_for(300ms)` in (c2) is settle-margin for a "nothing ever lands" negative assertion (defensible, not a race).
3. **a4 precondition (AL9c) and (f) label**: `probe-async-load.py:822-827` gates (a)/(b) behind
   `w <= triggerDelayS → FAIL "window too short to test"`, matching main's/commit-1's actual RED lines verbatim.
   (f1)/(f2) verified against `StagedLoad::LabelHold`/`setFileLabel` (MainComponent.cpp:3028-3033) — AL5's label-hold
   is real (every label write funnels through `setFileLabel`, which diverts into `staged_->label` while held).
   NIT: check (c) is NOT gated by the same precondition (only a/b are); see NIT-2 below.
4. **end_quit_hung_open (FIFO) / T6 honesty**: `.harmony/probe-async-load.py:1022-1041` creates a REAL `os.mkfifo`
   with no writer (open(2) blocks forever) and posts it as a video cell; `.harmony/probe-async-load.sh` phase 2
   relaunches the app and runs the same `quit_check` (graceful osascript quit, 30s kill clock, .ips scan, 0
   UserNotificationCenter windows). `MediaOpener::~MediaOpener` (MediaOpener.cpp:74-86) implements the leak-not-kill
   path exactly as AL2 specifies (`removeAllJobs(true,5000)`, then release() the pool + one stderr line, never
   `~ThreadPool`'s stopThread/kill). T6 (destructor path restored) is honestly reported as a GUARD, not a live PASS:
   `tests/test_media_opener.cpp`'s AL2 case forks a child and shows the kill path doesn't crash there; the report
   states plainly it was NOT run live for screen-safety and says so rather than silently omitting it.
5. **a2 x5 table (AL1)**: T4 DATA table in the report (2x low vs 8x highest) matches AL1's stated logic (verdict by
   the OLD show's rows — late/hold frames — not by the raw arithmetic); the report explicitly states the 1-thread
   arm's trigger condition never fired (5/5 and 17/17 clean), so it was correctly skipped per the ruling's own
   conditional, and R3's arithmetic error (2 pool threads = up to 6 runnable threads via FFmpeg's 2 frame threads)
   is corrected in the report text as required.
6. **TEST_SERVER=OFF compile+strings (AL9a)**: every new audio-witness field (`CombinedCallback.h`,
   `AudioCallback.h/.cpp`, `AudioEngine.h`) and every new TEST-ONLY route (`ui_text`/`load_deck`/`duplicate_deck`/
   `cancel_load`) is behind `#if AUDIODNA_TEST_SERVER`, itself gated by the pre-existing
   `AUDIODNA_BUILD_TEST_SERVER` CMake option (CMakeLists.txt:457-458) — confirmed by direct grep, matching the
   report's strings-count-0 verification.
7. **Existing probes re-run, not re-thresholded**: `git diff --stat` confirms no existing `.harmony/probe-*` file
   was touched (only the 3 new probe-async-load files were added) — thresholds cannot have been touched because the
   scripts that hold them are byte-identical to main.
8. **23 REST load callers keep semantics**: `ApiServer::handleLoadComposition` keeps the request/response shape,
   the two synchronous `jsonFail` refusals, and now blocks the HTTP worker on a `LoadTicket` that finishes exactly
   at the staged swap (Done) — preserving the "answer only after the model changed" ordering every existing caller
   relied on via message-queue FIFO. `ApiServer::stop()` finishes every outstanding ticket Cancelled AND sets
   `ticketsClosed_` (checked under the same mutex before a new ticket is registered) before `server_.stop()`+join,
   closing the race a naive read of the plan might miss (a POST arriving exactly at quit either gets `cancelled`
   synchronously via `ticketsClosed_`, or its `callAsync` fails and it self-finishes Cancelled). `duplicateDeck` is
   confirmed still unreachable over production REST (only the TEST-ONLY debug route exists).
9. **CLAUDE.md ≤ 25,000 B + verbatim move (AL10)**: `wc -c CLAUDE.md` = 24,363. Diffed the removed "Routine pads and
   bands" paragraph in CLAUDE.md against the paragraph appended to docs/claude/recording.md "Surfaces" — byte-for-byte
   identical text, confirming the "no-loss verbatim move" claim.

## Additional checks (real-time rules, fences, plan conformance)
- Sacred Rule 1: every audio-callback addition (CombinedCallback.h, AudioCallback.cpp) is one `mach_absolute_time`
  read + relaxed atomics, compiled out entirely without AUDIODNA_TEST_SERVER; no allocation/lock/syscall added.
  `static_assert(is_always_lock_free)` present for every new counter (AL8 b), verified in source.
- R16 fence honored: `git diff --stat` confirms VideoPlayer.cpp, ImageSequence.*, SeqVram.h, VideoRing.h, Renderer.*,
  UndoService.*, FencedPtrSlot.h, CompositionLoad.h, DeckCommands.h are NOT in the changed-file list; VideoPlayer.h's
  only change is the AL4 comment.
- AL6 quit order verified verbatim in `~MainComponent` (ApiServer stop → cancelStagedOpen → routines/recorder →
  detach), matching the comment's own claim about the server sweep making the later finish a no-op compare-exchange.
- AL7 FIFO queue + modelEpoch: traced `admit()`/`LoadQueue`/`resolveDuplicateSource` end-to-end through
  `appendDeckFromFile`/`duplicateDeck`/`pumpLoadQueue`/`swapCompositionModel` — `modelEpoch_` bumps only on a full
  Composition swap (the one case that invalidates a queued Duplicate's deck-id reference); a plain load/append/
  duplicate cancel clears the queue outright via `cancelStagedOpen`. Consistent and correctly scoped.
- AL11 member order: `mediaOpener_` (MainComponent.h:454) declared after `previewPanel_` (line 332) — destructs
  first, matching the comment's claim.
- Member order / destructor safety in MediaOpener itself: `dropped_` is a `shared_ptr<atomic<uint64_t>>` so it
  outlives a leaked pool's still-running OpenJob destructors — a correct, non-obvious detail actually implemented.

## FINDINGS
1. [SHOULD] SLIM: `LoadTiming::active()` (src/core/LoadTiming.h:58) and `stagedload::LabelHold::loadingText()`
   (src/core/StagedLoad.h:100) are unreferenced (confirmed by grep — no caller in src/ or tests/). The builder's own
   SLIM CHECK already flags this as EXCESS_DEAD and gives a reason (keep the GREEN evidence pinned to the tested
   binary sha256 2c877262c6a80efd) — reasonable for THIS merge, but these two one-line accessors should be cut in
   the very next touch of either file (DEBT_FILED is the right disposition, not a re-open of this lane).
2. [NIT] `probe-async-load.py` a4's AL9(c) precondition gate (`w <= triggerDelayS` → "window too short to test")
   wraps checks (a) and (b) only (lines 824-827); check (c) — "V32 on screen with no active clip" — runs
   unconditionally below the if/else. The adoption text says the precondition sits "before (a)-(c)". In practice
   this is defensible: (c) is not window-dependent (it is a real invariant about the end state regardless of
   whether the trigger raced the load), and the RED evidence on main shows (c) correctly FAILing for its own reason
   (the trigger really did leak into V32 when the load was synchronous) rather than a false pass. Still worth a
   one-line confirmation from Harmony/Fable that widening AL9(c)'s "(a)-(c)" wording to "(a)-(b)" in the plan text is
   intentional, since the code and the ruling text disagree in scope (not a correctness bug either way).

## Verdict rationale
No MUST-level defect found: every FOCUS-listed gate traces to real code with the claimed behavior, the fail-closed
paths (ticket wait, ApiServer::stop() release, MediaOpener's leak-not-kill, AL3's deterministic teeth) are genuine
and not fail-open, the R16 fence is honored file-for-file, and the CLAUDE.md budget + verbatim move are verified
byte-for-byte. The two findings above are non-blocking (a pre-disclosed dead accessor pair, and a defensible scope
nuance in a probe precondition) — PASS_WITH_NITS.
