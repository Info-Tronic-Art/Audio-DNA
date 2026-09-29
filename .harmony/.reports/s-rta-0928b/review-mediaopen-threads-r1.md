# Reviewer Verdict — mediaopen-threads-r1
STATUS: DONE
VERDICT: APPROVE
REVIEWED_COMMIT: 328301de37dd56df2e83096089f9847fd0e51ff0..e32948c277b4bbf423dffc7e3c9bf51c324c0cf9 (worktree rta0928b-mediaopen, branch lane/mediaopen)

FILES: src/render/FencedPtrSlot.h (new), src/render/Renderer.h/.cpp, src/core/UndoService.h/.cpp,
src/MainComponent.h/.cpp, src/media/ImageSequence.h/.cpp, src/core/MediaPresence.h (new),
src/render/CompositorEngine.cpp, src/ui/ClipCell.h/.cpp, src/ui/LayerStrip.cpp, src/model/Clip.h/.cpp,
src/api/ApiServer.h/.cpp, src/api/MessageHeartbeat.h (new), src/test/TestServer.h/.cpp,
tests/test_fenced_ptr_slot.cpp, tests/test_media_presence.cpp, tests/test_hot_thread_io_lint.cpp,
tests/test_image_sequence_open.cpp, tests/test_deck_thumbnails.cpp, tests/test_composition.cpp,
tests/CMakeLists.txt, docs/claude/rendering.md, docs/claude/pitfalls.md, CLAUDE.md,
.harmony/probe-media-open.{sh,py,json}, .harmony/.reports/s-rta-0928b/mediaopen.md.

## FOCUS-area findings (VERIFIED unless noted)

1. Fence hold (P3, single atomic word) — VERIFIED. `src/render/FencedPtrSlot.h` packs the deck pointer + fence bit
   in one `std::atomic<uintptr_t>`; `view()` is one acquire load, `detachFenced()`/`set()` are one release store
   each — no two-load TOCTOU on either edge. `Renderer::renderOpenGL` (Renderer.cpp:362) reads `activeDeck_.view()`
   once per frame; `UndoService::withDeckDetached` begins the fence via `detachActiveDeckFenced()` (one store) and
   ends it via the restore's `setActiveDeck()` (one store). Grepped every `activeDeck_.` use in Renderer.cpp/.h —
   no stray raw two-load access remains (the one non-fence caller, `renderSource` at :1408, uses `.get()` and only
   runs once a deck is already known active, so it needs no fence view). Executed
   `tests/test_fenced_ptr_slot.cpp` fresh (built + ran): single-thread semantics case and the 300k-iteration
   two-thread stress case (writer replays the exact `detachFenced()` → `set(restored)` sequence) both PASS
   (16 assertions, 2 cases) — `fencedWithDeck == 0` and `unfencedWithoutDeck == 0` confirmed, not just claimed.
2. Fence-hold invariants (no capture, no recorder/Syphon, no UAF of the mutated model) — VERIFIED by reading.
   The hold branch (Renderer.cpp ~:388-402) only reads `canvasTex_`/`canvasW_`/`canvasH_`/`glContext_` (Renderer's
   own GL-thread-only members, never the Deck/Composition under mutation — no UAF), calls `publishToOutputs` +
   `presentCanvas`, and returns before `processPendingCapture` and before the video-recorder/Syphon calls (which
   live at Renderer.cpp:996-1006, well past this early return) — matching the two pre-existing early-return paths'
   contract (F2) exactly.
3. Drops prepared outside the fence, committed inside (4.2) — VERIFIED against MainComponent.cpp/.h. Every drop
   path (`handleFileDrop`, `onMultiVideoDropped`, `onMixedFilesDropped`, `handleMultiFileDrop`) now calls
   `prepareFileDrop`/`prepareMultiFileDrop` (video/sequence open + dims + thumbnail) before
   `undoService_.withDeckDetached`, and only `commitDrop` (before-snapshot + `deck->setClip`) runs inside the
   fence. Prepare-order matches the original apply-order (images then videos) so undo/edit semantics are
   unchanged. The one accepted-and-documented gap (a prepared drop whose commit never runs leaks a player) is
   unchanged from the plan's stated risk, not a new defect.
4. Sequence open with no I/O + seekTo as a GL-thread request (4.5) — VERIFIED. `ImageSequence::open` no longer
   calls `existsAsFile()` or `ImageFileFormat::loadFrom` (no frame-0 decode, `width_/height_`/`getThumbnail`
   removed with no remaining callers — grepped). `seekTo` now only stores `seekTarget_`/`seekRequested_`; the
   request is consumed at the top of `advanceFrame`, before the `playing_` check, exactly as VideoPlayer's shape.
   Executed `tests/test_image_sequence_open.cpp` fresh: 58 assertions / 2 cases PASS (open with missing paths does
   no I/O and keeps the slot; seek request/consume ordering; two seeks before one advance takes the last).
5. Presence flag writes vs GL reads (4.6) — VERIFIED. `Clip::mediaMissing` is written only from the message
   thread (`MediaPresenceSweeper::landed` via `callAsync`, and `presence::seed` in `openMediaForDeck`), and read
   from the GL thread (`CompositorEngine.cpp`'s new `mediaPresent()` helper at all 6 former `existsAsFile()`
   sites) and from `ClipCell::paint` on the message thread — the same message-thread-write/GL-thread-read shape
   the plan cites as precedent (`clip->playing`). Executed `tests/test_media_presence.cpp` fresh: 19 assertions /
   4 cases PASS (mediaPaths dedup, apply's flip+count, clear/replaceContent carry, seed). Executed
   `tests/test_hot_thread_io_lint.cpp` fresh: 297 assertions / 2 cases PASS — confirms by direct scan that
   `existsAsFile` is gone from CompositorEngine.cpp/ImageSequence.cpp/ClipCell.cpp and `ImageFileFormat::loadFrom`
   is confined to the three allowed decoders.
6. `Clip::fromVar` speed default (4.7) — VERIFIED. `fromVar` now guards `speed` with `hasProperty("speed")`,
   matching every other guarded field; `tests/test_composition.cpp` gained the RED-on-main assertion
   (`WithinAbs(1.0f,...)`) plus an explicit-0.0/-2.0 SECTION.
7. Scope fence — VERIFIED. No `MediaOpenBatch.h`/`MediaOpener.*`/`StagedLoad` code anywhere in the diff (`git diff
   --stat` file list has none of these) — commit 7 (async loads) is correctly deferred per HARMONY ADOPTION P1.
   No `src/media/VideoPlayer.*` file appears in the diff at all — P2's "do not edit VideoPlayer" is respected;
   the drop path still calls today's (unchanged) `getThumbnail`.

## Other dimensions
- Patterns/DRY: `ClipCell::classifyDrop`/`dispatchDrop` factor the Finder-drop routing into pure static functions
  shared by the real drop path and the new TEST-ONLY `/api/debug/drop_files`, avoiding a second, drifting copy of
  the dispatch logic — good factoring, matches the plan's intent exactly.
- Docs/spec fidelity: `docs/claude/rendering.md`'s new paragraph and Pitfall "NN" text were checked line-by-line
  against the code — no overclaim; it explicitly says the load path "still opens its videos on the message
  thread before the swap (the asynchronous load is a follow-up lane)", correctly NOT claiming commit-7 behavior
  that isn't built. CLAUDE.md is 24,659 B (< 25,000 cap), matching the builder's claim (`wc -c` re-run).
- Error handling: `MessageHeartbeat` correctly documents and tolerates a ping that lands after `stop()`/teardown
  (shared_ptr `State` kept alive by the queued lambda; `takePeakMs()` gates on `isOn()` so a post-stop write is
  inert) — read and reasoned through, no UAF.

## SLIM
No excess found: `FencedPtrSlot`, `MediaPresence.h`, `MessageHeartbeat.h`, and `ClipCell::DropRoute`/`classifyDrop`
are each load-bearing for the FOCUS items and are exercised by both a real caller and a new test — no
`EXCESS_DEAD`/`EXCESS_SPEC` candidates identified in this diff.

## Residual / not independently re-executed
- Did not launch the app / GUI to re-run `.harmony/probe-media-open.sh` or the full `ctest --test-dir build-lane
  -j1` (880/880) myself — INFERRED from the builder's report only for those live rows. I DID independently build
  and run all 4 new pure/lint ctest binaries fresh from the reviewed tree (test_fenced_ptr_slot,
  test_media_presence, test_hot_thread_io_lint, test_image_sequence_open) and all pass green, which covers every
  FOCUS item except the live GL fence behavior (covered instead by direct code reading + the executed two-thread
  stress test of the underlying primitive).
- Did not reproduce the lint's teeth (reverting the `existsAsFile()`/frame-0-decode removal) live — a sandboxed
  attempt hit CMake cache path entanglement between the worktree's build-lane and its cached absolute source
  path; confirmed (by binary mtime) the original tree's build artifacts were NOT touched. Given
  `test_hot_thread_io_lint` is a trivial substring scan of the named files (read directly), I'm confident by
  inspection that reintroducing `existsAsFile` in the three named files would fail it — this is INFERRED from
  code reading, not executed.

## NIT
- `MainComponent::prepareFileDrop` does not explicitly set `clip.mediaMissing = false` as plan section 4.2's text
  proposed ("add clip.mediaMissing = false; a Finder drop exists"). This is harmless (Clip's struct default is
  already `false`), but a future reader diffing the plan against the code could be confused by the omission.
  No behavior change, not blocking.

SUMMARY: 23 files reviewed (diff), 0 blocking issues, 1 NIT. VERIFIED: fence single-atomic hold design + its
invariants (capture/recorder/UAF), drop prepare/commit split, sequence no-I/O open + seekTo request/consume,
presence flag thread discipline, Clip::fromVar speed default, and scope conformance (no commit-7 code, no
VideoPlayer.* edit) — all by direct diff reading plus fresh execution of the 4 new pure/lint test binaries.
INFERRED (not re-executed by me): the live probe-media-open.sh rows and the full 880-test serial ctest count,
taken from the builder's report, which is internally consistent with the code I read.
METADATA: reviewer=reviewer-mediaopen-threads-r1, builder_packet=mediaopen, date=2026-09-29
