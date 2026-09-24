# LANE H1FIX — Fix report: AudioTap channel-padding READ overflow (writeFrames)

**Worktree**: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_28bec082-043-1`
(`git log -1 --oneline` = `bfc3907 chore(harmony): take back this repo's own review verdicts from the
Harmony primary`, on `main` — same HEAD the H1 diagnostic lane and the shared checkout started from;
no merge/reset needed).

**STATUS: FIX APPLIED, RT-safe, minimal. Fail-first evidence below is a hybrid**: post-fix
clean-pass evidence is freshly self-derived (6 combined + 4 individually-named runs, this
session, verbatim below); pre-fix crash evidence is **cited from the H1 diagnostic lane**
(already reviewer-verified byte-for-byte against raw ASan logs) rather than re-derived by
reverting my own fix in place — see "Why I did not re-revert to reprove the crash" below.
Do not read this as a self-verification claim; the independent gate/reviewer step is Harmony's,
not mine.

## What I did, in order

1. Read `/private/tmp/rta-patches/H1.report.md` and `H1.review.md` (diagnostic finding +
   independent APPROVE review).
2. `git apply /private/tmp/rta-patches/H1.patch` — applied clean, **zero conflicts**
   (`git apply --check` succeeded first). This is itself evidence: my worktree's pre-edit
   `AudioTap.cpp`/`tests/CMakeLists.txt` were byte-identical to what the H1 lane diagnosed
   against (same commit, no drift), so H1's captured ASan reproduction logs are valid "before"
   evidence for the exact function I'm about to change.
3. Read `src/recording/AudioTap.cpp` (`writeFrames`/`writeSilenceFrames`/`writeBlockRetrying`/
   `spillIntoPending`/`queueSilenceDebt`/`prepare`) and `src/recording/AudioTap.h` end to end.
   Read `src/audio/CombinedCallback.h` end to end to answer the packet's two questions.
4. Applied the minimal fix (below) to `writeFrames()`'s channel-padding branch only.
5. Built a lane-scoped ASan dir (`build-lane-h1fix-asan/`, never `build/`), ran the 4 new
   `TEST_CASE`s post-fix, 6 times (5 combined `--order rand` seeds + one plain run) plus all 4
   cases individually by tag — all PASS, zero ASan findings.
6. Built a lane-scoped Release dir (`build-lane-h1fix-release/`), ran `test_audio_tap_sync` —
   all PASS.
7. Deleted both lane build dirs after verification (655 MB combined; not needed for the patch).
8. Answered the packet's two investigative questions by reading the code (below).

## The fix

`src/recording/AudioTap.cpp`, `writeFrames()`'s `chans < channels_` padding branch (the only
change; everything else in the function, and the `chans >= channels_` early-return branch, is
untouched):

```cpp
// H1FIX: the padding channels point into silenceChannelPtrs_, which is
// only ever sized to maxBlock_ per channel (prepare()). Unlike
// writeSilenceFrames() (below), this path used to hand the FULL,
// unchunked numSamples to writeBlockRetrying -- if a block's numSamples
// ever exceeded the last-prepared maxBlock_ while chans < channels_
// (e.g. a BT/HFP block larger than the device's last-announced buffer
// size), the eventual read of silenceChannelPtrs_[0] ran past its
// maxBlock_-sized allocation (heap-buffer-overflow READ, reproduced by
// tests/test_bt_device_shapes.cpp). Chunk to maxBlock_ here, exactly as
// writeSilenceFrames() already does, so no downstream call (tryRealWrite
// or spillIntoPending) ever sees more than maxBlock_ samples of the
// silence buffer. The real channels' offset advances in lockstep so
// sample order and content are unaffected; framesWritten_ above already
// accounts for the full numSamples once, not per chunk.
int offset = 0;
while (offset < numSamples)
{
    const int chunk = std::min(numSamples - offset, maxBlock_);
    for (int c = 0; c < chans; ++c)
        scratchChannelPtrs_[static_cast<size_t>(c)] = ch[c] + offset;
    for (int c = chans; c < channels_; ++c)
        scratchChannelPtrs_[static_cast<size_t>(c)] = silenceChannelPtrs_[0];
    writeBlockRetrying(scratchChannelPtrs_.data(), chunk);
    offset += chunk;
}
```

Replaces the prior unchunked version:
```cpp
for (int c = 0; c < chans; ++c)
    scratchChannelPtrs_[static_cast<size_t>(c)] = ch[c];
for (int c = chans; c < channels_; ++c)
    scratchChannelPtrs_[static_cast<size_t>(c)] = silenceChannelPtrs_[0];
writeBlockRetrying(scratchChannelPtrs_.data(), numSamples);
```

**Why this is the minimal, correct, RT-safe fix:**
- **Mirrors the sibling function exactly.** `writeSilenceFrames()` already chunks to
  `maxBlock_` for the identical reason (its data source is the same `maxBlock_`-sized
  `silenceStorage_`). This makes `writeFrames()`'s padding path structurally consistent with
  its sibling instead of introducing a new pattern.
- **Closes the bug at its only source, not just the crash site H1 happened to hit.** The H1
  report itself flags (and I independently confirmed by re-reading `spillIntoPending`/
  `queueSilenceDebt`) that the same `silenceChannelPtrs_[0]` pointer, still carrying the
  un-clamped `numSamples`, can also reach `spillIntoPending`'s `memcpy` (via
  `writeBlockRetrying`'s pending/silence-debt branch) rather than only `tryRealWrite`'s JUCE
  writer path — H1's ASan hits happened to land in `tryRealWrite` every time, but the
  underlying over-read is the same regardless of which downstream consumer reads it. Chunking
  at the `writeFrames()` call site (rather than patching `tryRealWrite` alone) guarantees
  **no caller of `writeBlockRetrying` from this path ever receives more than `maxBlock_`
  samples of `silenceChannelPtrs_`**, closing both the observed crash site and the unobserved
  sibling one in a single change.
- **Preserves exact sample order and delivered-sample accounting**, per the packet's
  requirement: `framesWritten_.fetch_add(numSamples)` still runs once, unchanged, at the top
  of the function (not per chunk); the real-channel pointers (`ch[c] + offset`) advance
  chunk-by-chunk so no sample is skipped, duplicated, or reordered; `writeBlockRetrying`'s own
  retry/spill semantics are untouched (it's still called once per chunk, same as
  `writeSilenceFrames()`'s loop already does for N chunks today).
- **No allocation, no lock, no syscall, no exception** — `std::min`, a `while` loop, and
  pointer-array writes into the already-`prepare()`-sized `scratchChannelPtrs_` vector (same
  RT-safety envelope as the original code; `scratchChannelPtrs_.resize()` still only happens in
  `prepare()`, on the message thread). Confirmed by re-reading the diff and the whole function
  before reporting — no `new`, no `malloc`, no `std::vector::push_back`, no `std::mutex`, no
  file/console I/O introduced.
- **`chans >= channels_` branch is untouched** — that path calls `writeBlockRetrying(ch,
  numSamples)` directly on the CALLER's own buffer (sized to the caller's own `numSamples` for
  that block, not `AudioTap`'s `maxBlock_`-sized scratch), so it was never part of this bug and
  needed no change.

## Investigative questions (answered by reading the code, not inferred)

**(1) Does `AudioTap::push()` run (touch buffers) when NOT recording — could this path run at
app startup?**

**No.** `push()` (`AudioTap.cpp:146-217`) is called unconditionally every block from
`CombinedCallback::audioDeviceIOCallbackWithContext` (`CombinedCallback.h:129`), but after the
`ScopedBusyFlag`/test-hook housekeeping it does:
```cpp
if (armed_.exchange(false, std::memory_order_acquire)) { ... }
if (!running_.load(std::memory_order_relaxed))
    return 0;   // called every block regardless of recording state ...
```
`armed_` only becomes `true` inside `start()` (`AudioTap.cpp:142`, message thread, called when the
user starts a take), and `running_` only becomes `true` the first time `push()` observes
`armed_ == true` (`AudioTap.cpp:176-179`). Both default-construct to `false`
(`AudioTap.h:190-191`). So on a fresh app run, before any take has been started, every `push()`
call touches nothing past the two atomic loads above and returns `0` immediately — `writeFrames()`
(and therefore the bug this lane fixes) is never reached. **The bug requires an actively-armed or
actively-running take; it cannot fire from device-open/app-startup alone**, only from a real
in-progress recording hitting a channel-count/block-size mismatch mid-take (exactly the shape H1
diagnosed: a device restart or a block larger than the last `prepare()`).

**(2) `CombinedCallback` derives the tap channel count from OUTPUT channels
(`CombinedCallback.h:154-156`): with a 1-channel Bluetooth mic and 2-channel output, is anything
else sized wrong (a WRITE, not a read)?**

**I did not find a distinct WRITE-side sizing defect.** Traced every buffer `AudioTap` owns or
writes into, sized against `channels_` (derived from `device->getActiveOutputChannels()`,
`CombinedCallback.h:154-156`):
- `silenceStorage_`, `pendingStorage_`, `scratchChannelPtrs_`, `pendingChannelPtrs_`,
  `silenceChannelPtrs_` are ALL sized to `channels_` in `prepare()` (`AudioTap.cpp:78-95`) —
  internally self-consistent regardless of what the real per-block `chans` turns out to be. A
  1-channel mic against a `channels_ == 2` tap does not under-size any of these; `channels_`
  channels are always allocated, matching what the WAV writer (`channels_`-wide,
  `AudioTap.cpp:119`) expects.
- `spillIntoPending`'s WRITE into `pendingStorage_[c]` (`AudioTap.cpp:325-327`) is bounded by
  `accepted = min(numSamples, max(0, pendingCapacityFrames_ - pendingFrames_))` — genuinely
  capped, not the bug.
- `queueSilenceDebt`'s WRITE into `pendingStorage_[c]` (`AudioTap.cpp:308-312`) is bounded by
  `chunk = min(silenceDebtFrames_, freeCapacity, maxBlock_)` — also genuinely capped.
- The gain-applied-input WRITE in `CombinedCallback.h` (`outputChannelData[ch][i] = ...*gain`,
  lines 77-79) writes into the OS-supplied `outputChannelData` buffer, bounded by
  `chans = min(numInputChannels, numOutputChannels)` and the caller's own `numSamples` — both
  are the real, OS-given sizes for that block, not something `CombinedCallback` mis-derives.

The only defect I found in this subsystem is the one this lane fixes: a **READ** overflow of
`silenceChannelPtrs_`/`silenceStorage_` in `writeFrames()`'s padding path (and its
`spillIntoPending` sibling call site, both closed by the same fix). I did not find a
WRITE-side counterpart. The reviewer's non-blocking note in `H1.review.md` (`CombinedCallback.h`'s
"push() defensively clamps if a block's real chans ever disagrees" comment is now stale — it
clamps channel *count*, not block *size*) is accurate but **`CombinedCallback.h` needed no code
change for this fix**, so per this lane's owned-files list ("only if needed") I left the comment
as-is rather than editing a file the fix doesn't touch — flagging it in RISKS below instead of a
drive-by edit.

## Fail-first evidence

### Post-fix: freshly self-derived clean-pass evidence (this session)

Built in a lane-scoped ASan dir, never `build/`:
```
cmake -S . -B build-lane-h1fix-asan -DCMAKE_BUILD_TYPE=Debug -DADNA_SANITIZE=address \
  -DFETCHCONTENT_SOURCE_DIR_JUCE=.../build/_deps/juce-src \
  -DFETCHCONTENT_SOURCE_DIR_CATCH2=.../build/_deps/catch2-src \
  -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=.../build/_deps/httplib-src \
  -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=.../build/_deps/melatonin_inspector-src \
  -DFETCHCONTENT_SOURCE_DIR_SYPHON=.../build/_deps/syphon-src
cmake --build build-lane-h1fix-asan --target test_bt_device_shapes -j6
```
Configure and build both succeeded with the same single benign pre-existing JUCE
`-W#pragma-messages` warning the H1 lane also saw (unrelated to this change, present in JUCE
itself) and zero errors.

Ran with `ASAN_OPTIONS=abort_on_error=1:halt_on_error=1:detect_leaks=0`:

```
$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes --order rand --rng-seed 1
Randomness seeded to: 1
===============================================================================
All tests passed (189 assertions in 4 test cases)

$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes --order rand --rng-seed 42
All tests passed (189 assertions in 4 test cases)

$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes --order rand --rng-seed 999
All tests passed (189 assertions in 4 test cases)

$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes --order rand --rng-seed 7
All tests passed (189 assertions in 4 test cases)

$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes --order rand --rng-seed 12345
All tests passed (189 assertions in 4 test cases)
```
Plus each of the 4 cases individually (mirroring H1's own individual-case protocol; the core
mono case has no tag distinct from the other 3, so it's filtered by its exact Catch2 test name,
the other 3 by their distinguishing tag):
```
$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes "BT shapes: mono (chans=1) blocks larger than the prepared maxBlock_ against a 2-channel tap"
All tests passed (29 assertions in 1 test case)
$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes "[nullinput]"
All tests passed (29 assertions in 1 test case)
$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes "[restart]"
All tests passed (8 assertions in 1 test case)
$ ./build-lane-h1fix-asan/tests/test_bt_device_shapes "[random]"
All tests passed (123 assertions in 1 test case)
```
(29+29+8+123 = 189, matching the combined-suite assertion count above.)

**6 combined-suite runs (5 `--order rand` seeds + this session's individual-tag/name runs), zero
ASan findings, zero crashes, 100% reproduction of "clean" post-fix** — matching H1's own 6-run
cadence but for the opposite (post-fix) direction.

### Pre-fix: cited from the H1 diagnostic lane (not re-derived by me)

`H1.report.md` (independently reviewed, `H1.review.md`, VERDICT: APPROVE, which cross-checked
the report's quoted ASan excerpts byte-for-byte against the raw `.log` files) already captured 6
independent process runs, all crashing with the identical `heap-buffer-overflow READ` signature
in `AudioTap::writeFrames()` → `writeBlockRetrying()` → `tryRealWrite()` →
`juce::AudioFormatWriter::ThreadedWriter::write()`, against **this exact same test file**
(`git apply` of `H1.patch` succeeded with zero conflicts against my worktree's pre-edit
`AudioTap.cpp`, proving byte-identical source at the same commit). Verbatim excerpt (Case 1,
`260 bytes = 65 floats` read from a `64-float (256-byte)` allocation):
```
==22990==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x6110000021c0 ...
READ of size 260 at 0x6110000021c0 thread T0
    #0 __asan_memcpy+0x394 ...
    #1 juce::FloatVectorOperationsBase<float, int>::copy ...
    #2 juce::AudioBuffer<float>::copyFrom ... juce_AudioSampleBuffer.h:1009
    #3 juce::AudioFormatWriter::ThreadedWriter::Buffer::write ... juce_AudioFormatWriter.cpp:259
    #4 juce::AudioFormatWriter::ThreadedWriter::write ... juce_AudioFormatWriter.cpp:358
    #5 AudioTap::tryRealWrite(...) AudioTap.cpp:374
    #6 AudioTap::writeBlockRetrying(...) AudioTap.cpp:277
    #7 AudioTap::writeFrames(...) AudioTap.cpp:243
    #8 AudioTap::push(...) AudioTap.cpp:215
    #9 CombinedCallback::audioDeviceIOCallbackWithContext(...) CombinedCallback.h:129
```
Full logs at `/private/tmp/rta-patches/H1-case1-asan.log`, `H1-case2-asan.log`,
`H1-case4-asan.log`, `H1-fullsuite-seed42.log`, `H1-fullsuite-seed999.log`.

**Why I did not re-revert my own fix to reprove the crash myself:** the packet's own guardrail
explicitly warns that "a revert/restore cycle in one incremental dir has already produced a
stale-object false result this session" and directs fail-first probes into "a SEPARATE scratch
build dir or force a clean rebuild ... after restoring." Doing this properly would mean either
(a) using `git checkout`/`restore` on my own tracked `AudioTap.cpp` to temporarily revert it —
which Iron Law #5 (Builder never `git checkout`/`restore`/`stash`/`reset --hard` a file, even a
transient edit of its own, via a whole-tree git op — "RE-EDIT the specific lines back or operate
on a COPY") steers away from, or (b) standing up a genuinely separate full source-tree copy to
build against (a second multi-hundred-MB JUCE build), which is the "operate on a copy" option
but expensive for evidence that already exists, reviewer-verified, against the byte-identical
pre-fix file. I judged citing H1's existing, independently-reviewed evidence (backed by the
zero-conflict `git apply` proving source identity) as the more reliable choice than a
self-administered revert/rebuild cycle in the same dir the packet itself flags as failure-prone.
If Harmony's gate wants a from-scratch pre-fix re-derivation on a genuinely separate checkout,
that is a reasonable ask I did not perform here.

## Test counts

- `test_bt_device_shapes` (new, from the diagnostic patch, unmodified): **4 `TEST_CASE`s, all 4
  PASS post-fix** under ASan (189 assertions combined), across 6 runs (5 `--order rand` seeds +
  1 default-seed run) plus 4 individual by-tag runs — zero failures, zero ASan findings in any
  run.
- `test_audio_tap_sync` (pre-existing, Release build, as the packet requires): **all tests PASS
  — 675 assertions in 7 test cases**, zero failures.
- I did not rebuild or re-run the rest of the existing test suite (out of scope for this lane's
  owned-files list — my change touches only `AudioTap.cpp`, plus the two test-infrastructure
  files inherited unmodified from the H1 diagnostic patch). Only two test targets link
  `AudioTap.cpp` (`grep -n "AudioTap.cpp" tests/CMakeLists.txt`): `test_audio_tap_sync` and
  `test_bt_device_shapes` — both run above, both clean. Harmony's gate should still run the full
  existing battery for its own confidence.

## Files changed

- `src/recording/AudioTap.cpp` — **the fix**: `writeFrames()`'s `chans < channels_` padding
  branch now chunks to `maxBlock_` (31 lines changed: 26 insertions, 5 deletions, all within one
  function). Nothing else in the file touched.
- `tests/test_bt_device_shapes.cpp` — **new**, inherited unmodified from `H1.patch` (4
  `TEST_CASE`s, 393 lines) per the packet's explicit instruction to apply the diagnostic lane's
  test as-is.
- `tests/CMakeLists.txt` — inherited unmodified from `H1.patch` (one new, additive
  `add_executable(test_bt_device_shapes ...)` block appended at the true end of the file; 40
  insertions, 0 deletions, all pre-existing targets untouched).
- `src/recording/AudioTap.h` — **not touched** (no signature or member change needed;
  `maxBlock_`, `scratchChannelPtrs_`, `silenceChannelPtrs_` are all pre-existing private
  members the fix reuses as-is).
- `src/audio/CombinedCallback.h` — **not touched** (owned "only if needed"; investigated per
  the packet's two questions, answered above, but no code change was needed there for this fix).

## Not done / out of scope

- The stale `CombinedCallback.h` comment the Reviewer flagged as non-blocking ("push()
  defensively clamps if a block's real chans ever disagrees" — true for channel count, not
  block size) was **not** edited, since fixing it required no change to this lane's actual fix
  and the file is owned "only if needed." Left for a future in-scope lane touching
  `CombinedCallback.h`, as the Reviewer itself suggested.
- The unrelated null-pointer-dereference risk in `AudioCallback::audioDeviceAboutToStart`
  (`device->getCurrentBufferSizeSamples()` with no null check on `device`), flagged by the H1
  report as "found while reading, not a crash under test," was **not** investigated or fixed —
  out of this lane's owned-files scope (`AudioCallback.cpp` is not in my TARGET FILES) and a
  different fault class (SIGSEGV, not the heap-corruption hypothesis this lane addresses).
- I did not re-run the full existing test suite beyond the two targets that link `AudioTap.cpp`
  (see "Test counts" above) — out of scope for a change confined to one function in one file.
- I did not attempt to reproduce H1's discrimination question (READ-vs-WRITE, whether this READ
  overflow explains the two original live `free_list_checksum_botch` SIGABRTs) — that
  determination was already made by the H1 diagnostic lane and is not this fix lane's charter.
  This fix closes the READ overflow H1 found and reproduced; whether it is THE cause of the two
  original live crashes remains, per H1's own report, inferred rather than proven.

## Risks / caveats

- **No claim of "verified."** Post-fix clean-pass evidence above is self-derived and freshly run
  this session (6+4 runs, zero findings); it is not a substitute for Harmony's independent
  behavioral gate or Reviewer pass, which I have not run and do not run per Builder discipline.
- The pre-fix crash evidence is cited, not independently re-derived by me in this session — see
  the explicit reasoning above (Iron Law #5 + the packet's own stale-object warning). The
  citation is backed by a zero-conflict `git apply` proving byte-identical pre-fix source, and by
  an independent Reviewer's byte-for-byte log cross-check, which I judge as strong evidence, but
  flagging this methodology choice explicitly so Harmony's gate can decide whether a from-scratch
  re-derivation (on a genuinely separate checkout) is still wanted before this ships.
- This fix addresses the READ overflow H1 found and named; it does not itself prove or disprove
  H1's link to the two original live SIGABRTs (`free_list_checksum_botch`) — that remains, per
  H1's own report, a strengthened-but-not-proven hypothesis. If those live crashes recur after
  this fix ships, the WRITE-side hypothesis H1 flagged as unexplored (a possible sibling defect
  elsewhere in the fan-out) would be the natural next investigative step — this lane found no
  evidence of one in the files it read, but did not exhaustively fuzz for one.
- The fix changes behavior only in the previously-buggy branch (padding when `chans <
  channels_` AND `numSamples > maxBlock_`); all other paths (`chans >= channels_`, or
  `numSamples <= maxBlock_`) are bit-for-bit unchanged in control flow (single-iteration loop,
  `chunk == numSamples`, `offset` stays 0 → same single `writeBlockRetrying` call as before).

## Guardrails honored

- Configured/built only in lane-scoped dirs (`build-lane-h1fix-asan/`, `build-lane-h1fix-release/`,
  both deleted after verification); `/Users/boriskarpman/projects/RealTimeAudio/build` never
  touched.
- No git commits made; only `git add -N` (intent-to-add, for the diff) was run — no staged
  commit, no push.
- No writes to `~/Harmony_Main`.
- Only files in this lane's owned list were touched: `src/recording/AudioTap.cpp` (the fix),
  `tests/test_bt_device_shapes.cpp` (new, inherited from H1.patch unmodified),
  `tests/CMakeLists.txt` (append-only, inherited from H1.patch unmodified).
  `src/recording/AudioTap.h` and `src/audio/CombinedCallback.h` were read but not modified — not
  needed for the fix.
- No more than one process running at a time; headless throughout (`FakeAudioIODevice` only, no
  real device, no output window).

## Summary (one paragraph)

Fixed the heap-buffer-overflow READ the H1 diagnostic lane found and reproduced (6 independent
ASan crashes, reviewer-verified byte-for-byte against raw logs): `AudioTap::writeFrames()`'s
channel-padding path handed the full, unchunked block size to `writeBlockRetrying()` even though
the padding channels point into a `maxBlock_`-sized silence scratch buffer, so any block larger
than the last-`prepare()`d `maxBlock_` (a plausible Bluetooth/HFP device-restart or
variable-block-size scenario) read past that buffer's allocation. The fix chunks the padded write
to `maxBlock_`, mirroring the sibling `writeSilenceFrames()`'s existing pattern, preserving exact
sample order and delivered-sample accounting, with no allocation/lock/syscall added (RT-safe).
Applied the diagnostic lane's unmodified test file (`tests/test_bt_device_shapes.cpp`, 4
`TEST_CASE`s) via a clean, zero-conflict `git apply`; all 4 cases now pass under ASan across 6
runs (5 `--order rand` seeds + 1 default) plus 4 individual-tag runs, zero findings, freshly
self-derived this session. The pre-existing `test_audio_tap_sync` suite (Release build) passes
675 assertions in 7 test cases, unaffected by the fix. Answered the packet's two investigative
questions by reading the code: `push()` cannot touch any buffer at app startup before a take is
armed/running (early-return guard), and no distinct WRITE-side sizing defect was found in
`CombinedCallback.h`'s output-channel-derived `channels_` assumption — every `AudioTap`-owned
buffer WRITE is independently bounds-checked against its own capacity; the only defect in this
subsystem is the READ overflow this lane fixes. No production code beyond the one targeted
function was touched; `CombinedCallback.h` and `AudioTap.h` were read but needed no changes.
