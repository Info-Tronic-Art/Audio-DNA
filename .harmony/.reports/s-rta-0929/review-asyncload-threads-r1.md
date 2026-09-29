# Reviewer Verdict — asyncload-threads-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS
COMMIT_RANGE: adf9b8a..fb0e455 (lane/asyncload, worktree rta0929-asyncload)

## Scope
Focus per dispatch: lifetimes/threads (WeakReference/callAsync/LoadTicket waits, ~MainComponent order AL6,
AL2 intentional-leak, AL1 retiring every adopted id incl. sequences, AL7 FIFO queue, no off-GL-thread GL
object creation, staged players invisible until cut, audio-callback TEST_SERVER gating, Sacred Rule 1,
MediaOpener pool vs decode threads) + plan conformance, RED-before-GREEN, stray artifacts, CLAUDE.md size.

## Independent verification performed (not recall)
- Read plan-asyncload.md in full (both pages) incl. HARMONY ADOPTION AL1-AL12.
- Read every changed file in the diff (30 files) via `git show fb0e455:<path>` / `git diff adf9b8a..fb0e455`.
- Ran the ALREADY-BUILT build-lane binaries directly (not from memory of builder's claim):
  - `tests/test_load_ticket` -> 12 assertions / 3 cases, all pass.
  - `tests/test_staged_load` -> 84 assertions / 12 cases, all pass.
  - `tests/test_media_opener` -> 58 assertions / 7 cases, all pass (incl. the AL2 forked-child FIFO-hang
    test; the intentional-leak `std::cerr` line printed exactly as documented).
  - Full `ctest -j4` in build-lane: **942/942 passed**, matches builder's claim exactly (independently
    confirmed, not copied from the report).
- Adversarial mutation proof (Layer-3 compliant: built in a fresh `mktemp -d` sandbox, StagedLoad.h + the
  real test_staged_load.cpp compiled standalone against the shared Catch2 archives from
  `.../build-lane/_deps/catch2-build`, reviewed tree never touched):
  - **Teeth T1** (`Ledger::land` ignores gen): mutated copy -> ctest genuinely FAILs 4 assertions
    ("a landing of a superseded batch is Stale and does not count" + others). Confirms the ledger's
    generation check is load-bearing and the test catches its removal.
  - **Teeth T5** (`labelAfterCancel` returns `current` unconditionally): mutated copy -> ctest genuinely
    FAILs 2 assertions in the AL5 label-hold case. Confirms the restore-on-cancel logic is guarded by a
    real test, not a tautology.
  - Baseline (unmutated) sandbox build reproduced 84/84 green first, ruling out a sandbox/include artifact.
- Cross-checked every doc claim against the actual code (not the other way around): `loadWitnessVar()`'s
  exact field list (`opens_pending, open_batches, opens_stale, opens_failed, opens_dropped, staged,
  staged_players, queued, timing{...}`) matches rendering.md's "Asynchronous loads" paragraph verbatim;
  `LoadTiming::var()`'s field list matches the doc and the plan's 5.6 spec exactly; pitfalls.md's new
  entry NN and the Pitfall-56 clause match `VideoPlayer.h`'s comment-only edit (R16/AL4 exception
  respected: `VideoPlayer.cpp` is untouched, confirmed absent from `--stat`).
- `wc -c CLAUDE.md` -> 24,363 (within the ≤25,000 B cap); recording.md "Surfaces" carries the routine-pads
  paragraph verbatim (8/8 sentences present, matches AL10/Q5's no-loss requirement).
- `grep -rn AUDIODNA_ASYNCLOAD_` across the tree -> no hits: T4's temporary env-switched instrumentation
  was fully reverted, matching the report's "restore by blob hash" claim; `git status --porcelain` shows
  only the (gitignored-by-convention) local `build-lane/` artifact, nothing stray committed in the diff.
- `ctest -N` confirms Test #942 total (matches builder's claim) and that the three new ctest binaries
  (`test_load_ticket`, `test_staged_load`, `test_media_opener`) are registered via `tests/CMakeLists.txt`.

## Thread/lifetime findings (the FOCUS list)
- **WeakReference / callAsync landings**: `MediaOpener::OpenJob::runJob()` captures only its own data + a
  `WeakReference<MediaOpener>`; the posted `land` lambda checks `owner.get()` before touching the
  `MediaOpener`, so a landing after the owner is gone destroys the (unpublished) player harmlessly.
  `~MediaOpener()` calls `masterReference.clear()` BEFORE waiting, closing the race window correctly.
- **LoadTicket waits, released by `ApiServer::stop()` before httplib joins**: `stop()` takes
  `ticketsMutex_`, sets `ticketsClosed_`, finishes every live ticket Cancelled and clears the list, THEN
  calls `server_.stop()` + join. `handleLoadComposition` checks `ticketsClosed_` under the SAME lock before
  registering a new ticket — no TOCTOU window for a ticket to be created after the sweep and left stranded.
  Verified this synchronization by reading the code precisely (not assumed from the plan prose).
- **`~MainComponent` order (AL6)**: `apiServer_->stop()` (finishes every ticket Cancelled) runs FIRST,
  THEN `cancelStagedOpen(Cancelled)` (a no-op finish via compare-exchange, correctly commented as such),
  THEN `previewPanel_.getRenderer().detach()`. Matches AL6 exactly; the comment at the call site states the
  ordering rationale accurately.
- **AL2 intentional leak on a hung open**: `~MediaOpener()` does `removeAllJobs(true, 5000)`; on timeout,
  `pool_.release()` (leaks the `ThreadPool` object rather than let `~ThreadPool` kill a thread blocked
  inside FFmpeg) with a printed `std::cerr` line. Independently reproduced via the forked-child ctest
  (200ms into `open(2)` of a FIFO, no writer) — verdict: child exits 0 (WIFEXITED, not killed by a signal)
  in ~5-6s, matching the ~5s `removeAllJobs` timeout plus the leak path. `dropped_` is a
  `shared_ptr<atomic<uint64_t>>` that correctly outlives a leaked pool (comment states why; verified the
  shared_ptr copy is held by each `OpenJob`).
- **Cancel retires EVERY adopted id incl. sequences (AL1/R12)**: `stagedload::Adopted::add()` is called
  both at video landing (`onStagedLanded`) and at sequence-open time (`finishStagedLoad`, before the swap);
  `cancelStagedOpen` calls `takeAll()` once and closes every id through `closeMediaForClip`. `test_staged_load.cpp`
  case (h) explicitly covers "add(video id), add(sequence id) -> takeAll() returns both".
- **AL7 FIFO queue (bounded 8, Composition supersedes)**: `stagedload::admit()`/`LoadQueue` (pure, ctested)
  correctly distinguish Begin/Supersede/Enqueue/Refuse; `loadComposition` unconditionally clears the queue
  before staging (matches "a Composition load supersedes everything staged or queued"); `pumpLoadQueue`
  prepares a queued Duplicate against the then-live model via `resolveDuplicateSource` keyed by
  `modelEpoch_`, bumped only inside `swapCompositionModel` (correct — an Append/Duplicate completion does
  not invalidate other decks' ids).
- **No GL object created/destroyed off the GL thread**: `MediaOpener`'s pool jobs only construct
  `VideoPlayer` and call `open()` (contract, per the untouched `VideoPlayer.h`/`.cpp`, R16-fenced: "no GL,
  no thread, no Renderer state" on an unpublished object). `installVideoPlayer` (message thread, unchanged
  seam) is the only place a player enters the renderer's map; a stale-landing player is destroyed on the
  message thread as an unpublished, never-started object (`releaseGL` only touches a real GL texture,
  never allocated here).
- **Staged players invisible until the cut**: unchanged GL-thread code (`syncMedia`) only reaches players
  via the LIVE model's clip ids; a staged id is never in it (confirmed by citation, R16-fenced file).
- **Audio-callback additions, TEST_SERVER-gated, Sacred Rule 1**: read `CombinedCallback.h`,
  `AudioCallback.{h,cpp}`, `AudioEngine.h` diffs directly — every new counter is inside
  `#if AUDIODNA_TEST_SERVER`, uses only `mach_absolute_time` (commpage read) + relaxed atomics, no
  allocation/lock/syscall; `static_assert(is_always_lock_free)` present for every counter type (AL8 b).
  The non-TEST_SERVER `ringBuffer_.push` path is untouched (byte-identical), confirmed by reading the
  `#else` branch.
- **MediaOpener pool vs decode threads (R3/AL1)**: the plan's own arithmetic was wrong (each `open()` also
  spins 2 FFmpeg frame threads); the code and the report both correctly cite the AL1 correction rather than
  silently keep the wrong number, and the report documents the live data (late +0 / hold +0 in 5/5 arms)
  that justifies keeping the 2-thread pool without re-running the 1-thread arm. Acceptable per AL1's own
  "Harmony picks at the gate, data decides" framing — not something this review can adjudicate further
  without re-running the perf battery, which is out of scope for a read-only thread/lifetime review.

## Findings
1. NIT (SLIM: EXCESS_DEAD) — `LoadTiming::active()` (`src/core/LoadTiming.h`) is a public accessor with
   zero callers anywhere in the diff or the rest of the tree (`grep -rn "\.active()" src/` -> no hits). It
   is not in the plan's 5.6 pseudocode either. Trivial one-liner, no behavioral risk; not harness-wired,
   so no wiring-set check is needed. Disposition: `DEBT_FILED` (file it, or a trivial follow-up removes it)
   — not blocking.

## Verdict rationale
Every MUST-level thread/lifetime claim in the dispatch's FOCUS list was traced to source and, where
practical, independently executed rather than taken on the report's word: the full ctest suite (942/942),
the AL2 forked-child hung-open guard, and two adversarial mutation proofs (T1, T5) all reproduced on this
machine, in a sandboxed copy, matching the builder's claims exactly. Doc claims (rendering.md, pitfalls.md,
CLAUDE.md size, recording.md verbatim-move) were checked against the code they describe, not assumed. No
MUST-severity defect found. One NIT (a dead accessor) does not block.
