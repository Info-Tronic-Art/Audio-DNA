# Reviewer Verdict — renderleft-vj-r2
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED_RANGE: 5f1536f..ea47823917a934850639edab1b40a93594cf9138 (worktree .claude/worktrees/rta0928-w2, branch lane/renderleft-0928)
FILES: docs/claude/pitfalls.md, docs/claude/rendering.md, src/core/CompositionLoad.h, src/media/ImageSequence.{h,cpp},
  src/render/CompositorEngine.{h,cpp}, src/render/Renderer.cpp, .harmony/probe-image-load.{py,json,sh}

## MUST findings: none

MUST 1 (pitfalls restore) — VERIFIED via `git diff 15c5f8d:docs/claude/pitfalls.md ea47823:docs/claude/pitfalls.md`:
entries 48/49/50 present verbatim, byte-identical to main's 15c5f8d text; this round's diff to pitfalls.md touches
ONLY the insertion of 48-50 (confirmed via `git diff 5f1536f..ea47823 -- docs/claude/pitfalls.md`), no collateral
change to 37/46/51/52/53.

MUST 2 (C1 for image sequences) — VERIFIED by reading the code, not just trusting the report:
- `ImageSequence::firstFramePending()` (src/media/ImageSequence.cpp) is read-only (no decode/request/upload) and is
  logically equivalent to `getCurrentTexture`'s own pending determination for the CURRENT state: same idx-resident
  check, same lastShown-fallback check, same failed-sticky check; it only omits the side-effecting requestFrame/
  requestAhead calls, which is correct because `syncMedia` calls the real `getCurrentTexture` later in the SAME
  frame (compositeDeck calls `incomingImagePending` before the type switch, then calls `videoFrameFn_` — which
  reaches `ImageSequence::getCurrentTexture` — after it, in the same layer iteration), so the decode request is not
  starved by the read-only check.
- `CompositorEngine::incomingImagePending` now dispatches to `sequencePendingFn_` for `ImageSequence` clips, gated
  by the same fade/layer-type filter as the Image path (Opaque/Transparent/Mask only) — reused for both
  `compositeDeck` (line ~1061, covers Opaque/Transparent/Mask) and `compositePersistentLayers` (line ~1318).
- `Renderer::newOpenGLContextCreated` wires the provider under the EXISTING `imageSeqMutex_` — no new mutex added
  (Sacred Rule 2 respected), and the lock is never nested with `syncMedia`'s own acquisition of the same mutex
  (sequential, not nested, in the per-layer loop).
- `compload::imagePaths` (the composition-swap prefetch list) only collects `Clip::MediaType::Image`, confirming the
  test's premise ("a sequence is never prefetched") directly from source, not from the report's prose.
- New row i2ms_seq_fade_start has a real RED (p 0.578/0.553 on the pre-fix lane head, quoted with numbers in the
  report) -> GREEN (p 0.000) pair; not a vacuous/cannot-fail assertion.

MUST 3 (C2 snapshot row) — VERIFIED: `Renderer::takeSnapshot()` calls `captureFrame(outputFile)` (completeFrame
defaults false); both render_frame handlers (`TestServer.cpp:600`, `ApiServer.cpp:1177`) pass `completeFrame=true`
explicitly. This confirms C2 was already correct in the underlying code (round 1); this round's contribution is
the missing test row (i3n), which has a real RED (1422 ms / dbox 76.72 on main) -> GREEN (72-84 ms / dbox 0.00) pair.
`.harmony/probe-image-load.sh`'s own-traffic exclusion (`grep -cvxFf own-captures.txt`) verified by a standalone
repro in $TMPDIR — behaves as claimed (counts only lines NOT in the allowlist file).

SHOULD 4 (imagePaths dedup) — VERIFIED: `std::unordered_set` insert-based dedup, insertion order into `out`
preserved (same as before), existing order+dedup test (`test_composition.cpp` "compload::imagePaths...") unchanged
and still exercises the same expectation.

CLAUDE.md size — VERIFIED 24,482 bytes (`wc -c`), matches the report's claim, under the 25,000-byte cap.

Diff hygiene — VERIFIED: file list for this round is exactly the 11 source/doc/probe files the report names, no
stray files (no .venv, no ad hoc instrumentation, no env-var hooks). C6 fence respected: no src/ui/ClipCell.*,
LayerStrip.*, DeckView.* touched.

## SHOULD / nits

1. [SHOULD] Test coverage gap: `ImageSequence::firstFramePending()` is pure logic (no GL calls) but has no
   ctest-level unit test exercising its boundary branches directly (the `textures_.size() != n` / post-releaseGL
   path, the `failed_[idx] != 0` sticky path, the `lastShown_` fallback path) — coverage is only indirect, through
   the live GL probe row i2ms, which only exercises the "never-shown, not yet resident" branch. A pure test mirroring
   `tests/test_image_tex_cache.cpp`'s style (no GL context needed, since `firstFramePending` touches no GL state)
   would pin these branches directly and run in ctest rather than requiring a live app + lock. Not blocking: the
   function's equivalence to `getCurrentTexture`'s existing (already-covered) pending logic was hand-verified above,
   and the live row demonstrates the RED defect it was written to catch.
   Fix: add a small `tests/test_image_sequence_pending.cpp` (or a case group in an existing pure test file) covering:
   never-opened, opened-nothing-shown, resident current frame, resident lastShown fallback, failed-sticky, and the
   post-releaseGL size-mismatch state.

## Verdict rationale
No defect that ships, no missed plan item (C1/C2/C3/C4/C5/C6, R1-a hold semantics, real-time rules, mutex discipline
all checked directly against source), no test that cannot fail (RED/GREEN pairs quoted in the report were checked
against the actual code paths that would produce them). One SHOULD-level test-coverage suggestion, not blocking.
