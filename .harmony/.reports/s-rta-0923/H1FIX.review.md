# Reviewer Verdict — H1FIX (independent SOURCE review)
STATUS: DONE
VERDICT: APPROVE

## Scope
Independently reviewed the H1FIX patch/report (`/private/tmp/rta-patches/H1FIX.patch`,
`H1FIX.report.md`) against the actual worktree
`/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_28bec082-043-1`
(HEAD bfc3907, matches main, no commits made, 3 files touched — exactly the owned fence).

## Independent verification performed (not recall — executed)
1. Read `src/recording/AudioTap.cpp` end to end (writeFrames/writeSilenceFrames/
   writeBlockRetrying/spillIntoPending/queueSilenceDebt/tryRealWrite/prepare) and
   `src/audio/CombinedCallback.h` end to end in the worktree — patch content matches disk
   exactly (verbatim `H1FIX` comment block present, diff matches file).
2. **Freshly built** (not reused from the report) a lane-scoped ASan config against the
   worktree's own `test_bt_device_shapes` target, reusing cached JUCE/Catch2/httplib source
   dirs. Build succeeded cleanly (same single benign pre-existing JUCE `-W#pragma-messages`
   warning the report cites, no other warnings/errors).
3. Ran `test_bt_device_shapes` post-fix under ASan (2 seeds): **all tests passed, 189
   assertions in 4 test cases**, matching the report's own numbers exactly.
4. **Independently re-derived the fail-first (pre-fix) crash** — the one piece of evidence
   the report declined to re-derive itself (citing Iron Law #5 / the stale-object guardrail,
   and instead citing H1's already-reviewed logs). I built a throwaway scratch copy under
   `/tmp` (never the builder's tracked worktree — no `git checkout`/`reset` on any tracked
   file), reverted **only** the `writeFrames()` padding branch to its pre-fix form (the other
   four files untouched), rebuilt the same test target, and ran it under ASan:
   reproduced `AddressSanitizer: heap-buffer-overflow ... READ of size 260 at
   0x6110000021c0`, same stack (`tryRealWrite → writeBlockRetrying → writeFrames → push →
   CombinedCallback::audioDeviceIOCallbackWithContext`) — **byte-for-byte identical
   address and stack to the H1 report's cited Case 1 log**, confirming both that H1FIX's
   cited pre-fix evidence is real (T43 satisfied, now doubly so) and that the same test file
   deterministically catches the exact defect the fix addresses. This closes the one
   methodological gap in the builder's own report (self-flagged, not hidden) more cheaply
   than the report worried it would require.
5. Confirmed via `git status`/`git diff --stat` in the worktree: no commits, exactly the 3
   fenced files touched (`src/recording/AudioTap.cpp` modified, `tests/CMakeLists.txt`
   append-only diff, `tests/test_bt_device_shapes.cpp` new) — `src/recording/AudioTap.h` and
   `src/audio/CombinedCallback.h` correctly left untouched (fix needed neither).

## Correctness analysis
- Root cause and fix are correctly diagnosed: `writeFrames()`'s `chans < channels_` padding
  branch fed the whole unchunked `numSamples` to `writeBlockRetrying`, but the padding source
  (`silenceChannelPtrs_`) is only ever sized to `maxBlock_` (set in `prepare()`, e.g. from
  `device->getCurrentBufferSizeSamples()` in `CombinedCallback::audioDeviceAboutToStart`). Any
  block whose `numSamples > maxBlock_` while `chans < channels_` reads past that allocation.
- Fix chunks the padded write to `maxBlock_`, exactly mirroring the sibling
  `writeSilenceFrames()`'s pre-existing pattern — no new idiom introduced.
- Verified the `maxBlock_` invariant (`silenceStorage_[c].size() == maxBlock_` always) holds
  across `prepare()`'s realloc/no-realloc branches (pre-existing, unmodified code) — the fix's
  `chunk <= maxBlock_` bound is therefore a genuine, not incidental, safety guarantee.
- Sample order preserved: real-channel pointers advance `ch[c] + offset` per chunk in strict
  order; `framesWritten_.fetch_add(numSamples)` still happens once (outside the loop, not
  per-chunk) — delivered-sample accounting is unchanged.
- `chans >= channels_` branch (untouched) correctly left alone — that path uses the caller's
  own correctly-sized buffer, not the `maxBlock_`-bounded scratch/silence buffers.

## RT-safety
Confirmed by reading: `std::min`, a `while` loop, and writes into the already-`prepare()`-sized
`scratchChannelPtrs_` vector. No `new`/`malloc`/`push_back`, no `std::mutex`, no syscall, no
exception, no growth of any container on the audio thread. Same RT envelope as the unmodified
code around it.

## Investigative questions (report's answers checked against source)
1. `push()` cannot touch any buffer before a take is armed/running — confirmed by reading the
   `armed_`/`running_` guard at the top of `push()` (early-return `return 0` before
   `writeFrames` is ever reached). Correct as stated.
2. No distinct WRITE-side sizing defect in `CombinedCallback.h`'s output-channel-derived
   `channels_` assumption — confirmed by tracing every `AudioTap`-owned buffer
   (`pendingStorage_`, `scratchChannelPtrs_`, `silenceStorage_`) is sized to `channels_` in
   `prepare()` independent of the real per-block `chans`, and `spillIntoPending`/
   `queueSilenceDebt` are independently capacity-bounded (`min(numSamples, freeCapacity)` /
   `min(silenceDebtFrames_, freeCapacity, maxBlock_)`). Agrees with the report.

## Fence / hygiene
- Fence respected exactly: `src/recording/AudioTap.cpp` (fix), `tests/test_bt_device_shapes.cpp`
  (new, unmodified from the diagnostic patch), `tests/CMakeLists.txt` (append-only). No writes
  to `AudioTap.h`, `CombinedCallback.h`, or anywhere in `~/Harmony_Main`.
- No commits made (verified via `git log -1` == worktree base, `git status --short` shows only
  working-tree modifications).
- No UI changes (whole-word rule N/A).
- No silent failures introduced; no new fail-open path — this is a pure bug fix narrowing an
  overflow, not a new gate/check.
- Test file is new, not a modification of an existing suite, so T31/T42 `.new`-precondition
  hygiene doesn't apply here.

## Findings
1. [NON-BLOCKING] `CombinedCallback.h`'s stale comment ("push() defensively clamps if a
   block's real chans ever disagrees") — this refers to channel-count clamping, not
   block-size clamping, and is now doubly stale since this lane's fix is exactly the missing
   block-size clamp. The report correctly declined to edit this file (owned "only if needed";
   no code change required for the fix) and flagged it explicitly for a future in-scope lane.
   Agreed — not worth a drive-by edit here, but should not be forgotten (already reviewer-
   flagged once before per the report, still open).
2. [NON-BLOCKING] The report's own methodological gap (declining to re-derive the pre-fix
   crash rather than citing H1's evidence) is now closed by this review's independent
   re-derivation (finding 4 above) — no action needed from the builder; noting it here only
   so the record shows the gap was closed by the gate, not left open.

No blocking issues found.

SUMMARY: 3 files reviewed (AudioTap.cpp fix, tests/test_bt_device_shapes.cpp new,
tests/CMakeLists.txt append), 0 blocking issues, 2 non-blocking notes (both already
self-flagged by the builder). Fix is minimal, RT-safe, correctly scoped, and independently
reproduced both pre-fix crash (ASan heap-buffer-overflow READ, byte-identical stack/address to
H1's cited evidence) and post-fix clean pass (189/189 assertions) via fresh builds in this
review, not by recall. VERIFIED, not inferred.

METADATA: reviewer=claude-reviewer, builder_packet=H1FIX, date=2026-09-23
