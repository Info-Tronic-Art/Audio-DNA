# Reviewer Verdict — renderleft-gl-r2
STATUS: DONE
VERDICT: APPROVE

REVIEWED_COMMIT: 5f1536f..ea47823917a934850639edab1b40a93594cf9138 (branch lane/renderleft-0928, worktree rta0928-w2)

FILE: docs/claude/pitfalls.md
  [OK] Spec/claim fidelity: entries 48/49/50 restored verbatim from main 15c5f8d. Byte-diffed against 15c5f8d (`awk '/^48\./,/^51\./'` on both sides) -- IDENTICAL. Fixes the sole MUST from review-renderleft-gl-r1.

FILE: docs/claude/rendering.md
  [OK] Doc accurately reflects the code: the C1 sentence now says a crossfade also pauses on "its incoming image sequence has nothing to show yet (`ImageSequence::firstFramePending`, no side effect)" -- matches the actual CompositorEngine/ImageSequence change below.

FILE: src/media/ImageSequence.h / .cpp
  [OK] `firstFramePending()` is a read-only mirror of `getCurrentTexture`'s pending derivation (same n/textures_/failed_/lastShown_ checks, no decode/request/upload call) -- verified line-by-line against getCurrentTexture (ImageSequence.cpp:255-312 vs 316-329). `const`, no GL calls, no mutation -- safe to call speculatively before the frame decides whether to fetch the texture.
  [OK] Readability: comment states the read-only invariant explicitly and cross-references the twin logic it mirrors.

FILE: src/render/CompositorEngine.h / .cpp
  [OK] `incomingImagePending` now branches on `ImageSequence` via an injected `SequencePendingFn`, defaulting to `false` when unset (no null-deref in unit tests that construct `CompositorEngine` without a `Renderer`) -- correctly closes the MUST 2 gap (vj-r1: "C1 pause scoped to Image clips only").
  [OK] Pattern: matches the existing `VideoFrameFn`/`SourceRenderFn` injection pattern already used for the video/source paths -- no new mechanism introduced (EXCESS_DUP check: none, this is the established DI style in this file).

FILE: src/render/Renderer.cpp
  [OK] Threading: `setSequencePendingProvider`'s lambda takes `imageSeqMutex_` and is invoked (via `incomingImagePending`) from `compositeDeck`/`compositePersistentLayers`, both GL-thread-only, BEFORE `videoFrameFn_`/`syncMedia` (which takes the same mutex) is called for that layer in the same iteration -- sequential, not nested; no double-lock/deadlock. Verified call ordering at CompositorEngine.cpp:1061-1090 and :1318-1358 (incomingImagePending precedes the clipTex fetch in both compositing passes).
  [OK] No GL call introduced off the GL thread: the new provider and `firstFramePending` do no GL work; wiring happens in `newOpenGLContextCreated` (GL thread), consistent with the sibling `setVideoFrameProvider` added in R1.

FILE: src/core/CompositionLoad.h
  [OK] `imagePaths` dedup changed from O(n^2) linear-scan (`std::find`) to O(n) via `std::unordered_set<std::string> seen`; insertion order into `out` is unchanged (the set only gates re-entry), so the pre-existing "active deck first, active clips first... deduplicated" test (tests/test_composition.cpp:1301) is unaffected and still exercises the exact duplicate case. Closes the SHOULD from vj-r1.

FILE: .harmony/probe-image-load.{py,json,sh}
  [OK] New row i2ms_seq_fade_start exercises the sequence-crossfade gap with no fillers (a sequence is never prefetched, so its first frame always takes the demand path) -- correct setup, matches the mechanism being fixed.
  [OK] New row i3n_snapshot_while_pending checks the three required invariants (fast answer, held-picture identity, and that the following render_frame still gates) in one row: correctly discriminates the C2 behavior (this round did not touch Renderer's snapshot/capture code -- confirmed by diffstat -- so this is purely the missing-test fix that vj-r1 flagged, not a masked behavioral change).
  [OK] probe-image-load.sh's own-traffic filter change is a genuine fix, not a weakening: it now excludes only the run's own recorded snapshot files (own-captures.txt, appended by the .py) from the foreign-REST-traffic FAIL check, rather than loosening the check generally -- confirmed the exclusion list is populated by the very snapshot() calls this row makes, so a real foreign capture from another process would still trip the gate.

FILE: .harmony/.reports/s-rta-0928/renderleft.md, renderleft-fix.md
  [OK] Append-only: `git diff` on renderleft.md shows pure addition (no lines removed) at the tail; renderleft-fix.md is a wholly new file. No prior report content disturbed.

SLIM: No excess found. Every changed line traces directly to one of the four round-1 findings (pitfalls restore, C1-for-sequences, C2 test row, O(n) dedup); no speculative generality, no new abstraction beyond the pre-existing FooFn injection pattern already used for video/source.

CROSS-CHECKS PERFORMED (this round, read-only, no build):
  - Diffed docs/claude/pitfalls.md entries 48-50 byte-for-byte against main 15c5f8d: IDENTICAL.
  - Verified lock-ordering between the new sequencePendingFn_ lambda and syncMedia's pre-existing imageSeqMutex_ use: no nested/recursive acquisition.
  - Verified firstFramePending() has no GL call and no side effect, matching its own doc comment and the pitfalls/rendering.md claims about it.
  - Verified CLAUDE.md is 24,482 bytes (unchanged this round, <= 25,000 constraint) and not part of this diff.
  - Verified no stray instrumentation/env-var hooks/debug prints introduced (grepped src/ diff for std::cerr/cout/printf/DBG).
  - Verified test_composition.cpp's existing imagePaths dedup test is untouched and still exercises the duplicate-path case against the new O(n) implementation.

Did NOT independently re-run: ctest / probe-image-load.sh / probe-crossfade (would require a full C++ rebuild -- proposed for the Tester/behavioral-gate step, not reproduced here). Confidence on the reported RED/GREEN numbers is INFERRED from code-level correctness of the fixes, not independently re-executed.

SUMMARY: 10 files reviewed (5 source, 3 probe harness, 2 report/docs), 0 blocking issues, 0 nits. All four round-1 findings (MUST: pitfalls restore, MUST: C1 image-sequence gap, MUST: missing C2 test row, SHOULD: imagePaths dedup) are genuinely fixed at the code level, each with a matching new/restored test or byte-diff proof. No new defect, no scope creep, no toothless/fail-open path found in the changed surface.
METADATA: reviewer=reviewer-renderleft-gl-r2, builder_packet=renderleft-fix, date=2026-09-28
