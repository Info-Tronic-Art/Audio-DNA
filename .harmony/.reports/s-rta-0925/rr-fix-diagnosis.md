# rr-fix diagnosis -- snap-back FAIL on `.harmony/probe-step3.sh` (2 FAIL / 77 PASS on main@85afb70)

Author: Builder, 2026-09-25/26. Branch `lane/0925-rr-fix` from main@1631e99 (the tip main had actually
reached by the time this packet was dispatched -- one commit past the 85afb70 named in the packet; both
carry the replay-restore merge unchanged). Repo: `/Users/boriskarpman/projects/RealTimeAudio`, worktree
`.claude/worktrees/wf_d61ff7af-028-1`. Evidence style: VERIFIED = read on disk / reproduced this session;
INFERRED = derived from VERIFIED facts with the chain shown.

## VERDICT

**Root cause is a CODE bug, not a probe bug.** `Program.cpp`'s `buildPreamble()` skipped emitting a
continuous opacity restore whenever the CHECKPOINT value happened to equal the `LayerScalar::Opacity`
scalar default (1.0) -- an optimization not present in the original plan (`plan-roadmap.md` section
3.2 row 4), added unilaterally during implementation, and (unnoticed) endorsed by the reviewer-approved
test's own comment ("layers 1-2 have default opacity (skipped -- restoring a default is a no-op
write)", pre-fix `tests/test_program_preamble.cpp:123`). The reasoning is backwards: restoring a
checkpoint value that equals the default is a no-op **only if the live value has not since diverged
from default** -- which is exactly what the probe's own perturbation does on purpose (sets opacity to
0.9 before Play). The result: `activeClipColumn` correctly snaps back to -1 (a **discrete** Fired,
never gated on default), but opacity stays stuck at the perturbed 0.9 forever, because no
touch/set/release ever runs for it. The probe's 1.5 s poll window then never finds a match, burns its
full 15-iteration budget (confirmed at ~3-4 s of REAL wall time, not 1.5 s -- see section 3), and by
the time its last poll runs, the take's own recorded lane has already fired its first point (col 0 at
t=1.9487s), which is what the FAIL row actually reports (`col=0`, not the perturbed `3` and not the
restored `-1`).

Fixed by removing the skip-if-default check (always emit the continuous opacity `PreambleSet`, exactly
like the five boolean flags already do). New RED-then-GREEN test added; full ctest run: **484/484
(100%) pass** on this branch (see section 5). No probe change needed or made.

---

## 1. What checkpoint0 says for layer 0 (`step3gate1.adna-take`)

Read directly from `~/Documents/Audio-DNA/Takes/step3gate1.adna-take/take.json` (the SAME take both
snap-back sections load once via `/api/perf/load` and Play twice against, per
`.harmony/probe-step3.sh:662-663`):

```
checkpoint0.activeDeckIndex = 0
checkpoint0.decks[0].layers[0] = {
  "layer": "L1", "activeClipColumn": -1, "opacity": 1.0,
  "visible": true, "bypassed": false, "solo": false, "muted": false,
  "autopilotEnabled": false, "clips": []
}
```

This is the ONLY layer entry in `checkpoint0.decks[0].layers` (the live app's default deck/config used
by the gate has one layer named "L1" on deck "A" -- not `Composition::initDefault()`'s 3-layer "Deck
1" fixture the unit tests use; unrelated to the bug, just a different runtime default). `CHK_COL=-1`,
`CHK_OP=1.0` match the probe's own read (`.harmony/probe-step3.sh:609-610`) and the FAIL rows'
"expected -1/1.0" text exactly.

## 2. Does the preamble include layer opacity, and was it applied? -- NO (pre-fix)

`src/recording/Program.cpp` (pre-fix, `buildPreamble()`, pass A):

```cpp
{
    // LayerRuntime::opacity is always captured (not gated on non-default, unlike
    // ClipRuntime) -- skip a default-valued restore here rather than spam a no-op write.
    const auto& def = layerScalarDefs()[static_cast<size_t>(LayerScalar::Opacity)];
    const float norm = def.toNorm(rt.opacity);
    if (std::abs(norm - def.defaultNorm) > 1e-4f)
    {
        ControlPath k = controlKey("scalar"); k.scalar = "opacity";
        out.preambleContinuous.push_back(PreambleSet{ k, ctx.target, norm });
        out.report.preambleCount++;
    }
}
```

`layerScalarDefs()[Opacity].defaultNorm == 1.0f` (identity mapping, `src/connect/ScalarParams.h:96`).
Checkpoint opacity is 1.0 -> `norm == 1.0` -> `abs(1.0 - 1.0) > 1e-4` is **false** -> the continuous
entry is **never pushed**. This is confirmed live-gate-visible: with 1 layer and no captured clips,
`buildPreamble` emits exactly 8 discrete entries (activeDeck, quantize, 5 flags, activeClip) and,
pre-fix, **0** continuous entries -- matching the log's `preambleFired >= 8 (8)` PASS row exactly (a
value of 8, not 9). Contrast the flags immediately above this block (`emitFlag("visible", ...)` etc.,
`Program.cpp:275-279`): those are emitted **unconditionally**, regardless of whether the captured value
equals its live-model default. Only opacity had this asymmetric skip.

`activeClipColumn`'s discrete restore has NO such gate (`Program.cpp`, pass A, third block): `col=-1 <
numCols` is true whenever the layer has any columns at all, so the discrete `Fired{v=-1}` is always
built and pushed regardless of whether -1 is also some notion of "default". Dispatch wiring confirms
the clear path is correct: `MainComponent.cpp:1941-1942`, `if (f.p.v < 0) applyClearActiveClip(...)`.
Combined with `preambleRefused == 0` in every PASS row, this is why the FAIL's observed `col` is never
`3` (the perturbed value) -- the clear DID fire and succeed. It is also never `-1` by the time the
probe's LAST poll reads it, for the reason in section 3.

## 3. Do the take's own lanes legitimately move layer 0 within the probe's window, making its
   expectation wrong? -- NO; the window has margin, but the code bug forces a probe-loop overrun that
   crosses into it

`step3gate1.adna-take`'s `activeClip` lane (`lanes[0].points`, read directly):

```
seq=2  t=1.9486957...s  v=0   (origin: human)
seq=3  t=5.0154993...s  v=1
seq=4  t=8.0779965...s  v=2
seq=5  t=11.1457206..s  v=3
seq=7  t=16.2594360..s  v=2
```

The first point (v=0) lands at **t=1.9487s**, which is **outside** the probe's nominal 15 x 100ms =
1.5s poll window (450ms of margin). Under CORRECT behavior (opacity restore succeeds on poll 1, since
`Player::firePreamble` runs synchronously inside `RecorderHost::play()` before the REST handler even
returns), `check_snapback_restored` exits on its first or second iteration -- nowhere near 1.9487s of
real elapsed time. So the probe's design is sound; it does not need to change.

What actually happened, confirmed from the SAME log using a shared timestamp anchor: `probe-step3.sh`
resets `START_T` once, immediately after each `/api/perf/play` POST (`:675`, `:723`), BEFORE calling
`check_snapback_restored` (`:676`, `:724`). The very next block (the long replay-sequence poll loop,
`:680-705`) reuses that SAME `START_T` and prints the elapsed time of its first observed
`activeClipColumn` change. Because opacity never matched (section 2), `check_snapback_restored` ran
all 15 iterations (curl + python `jget` round-trips, not free) before giving up -- and the very FIRST
value the next loop observes is already `col=0` at:

```
replay(withAudio): t~3s activeClipColumn=0
replay(wallClock):  t~4s activeClipColumn=0
```

i.e. by the time `check_snapback_restored` exhausted its budget, **3-4 real seconds** had elapsed since
Play -- not the nominal 1.5s -- comfortably past the take's own first lane point at 1.9487s. This is
not an inference from timing theory alone; it is read directly off the log using the probe's own
`START_T` anchor shared between the two sections. The FAIL row's `col=0 opacity=0.9` is therefore the
take's own recorded first point overwriting the (correctly, but invisibly-to-the-probe) restored `-1`,
compounded by the opacity restore never having happened at all so the loop had no early exit.

**Conclusion for item 3**: the take's lane does legitimately move layer 0, but not within the probe's
intended 1.5s window -- only within the extended, bug-caused ~3-4s the loop actually ran. Fixing the
opacity bug removes the reason the loop ever runs past its first 1-2 iterations, which restores the
probe's original margin. No probe change is needed or made.

## 4. Fix

`src/recording/Program.cpp`, `buildPreamble()`: removed the `if (std::abs(norm - def.defaultNorm) >
1e-4f)` gate around the layer-opacity continuous emission -- it is now unconditional, exactly like the
five discrete flags immediately above it in emission order. `def.defaultNorm` is no longer read (kept
`def` for `toNorm()`). No other file changed.

### Tests (`tests/test_program_preamble.cpp`)

- **Updated** test 1 ("compile builds the preamble from checkpoint0 in restore order"): pre-fix this
  test's own comment said layers 1-2's default opacity was "skipped" -- that was the bug, hiding in a
  green test. Updated `preambleContinuous` from size 3 to size 5 (every layer's opacity is now
  present, including the two at-default ones), reordered indices for the clip fx/scalar entries
  accordingly, and removed the stale "skipped" comment.
- **Added** test 6, `[program][preamble][rr-fix]`: "a layer opacity captured AT the scalar default
  still emits a continuous restore" -- a fixture mirroring `step3gate1.adna-take` exactly
  (`activeClipColumn=-1`, `opacity=1.0`, i.e. the checkpoint's own values), asserting the continuous
  `PreambleSet` for layer 0 opacity exists with `v==1.0`, and that the discrete `activeClip` clear
  (`v==-1`) also exists (confirming the bug is isolated to the continuous opacity path, not a general
  "checkpoint == default is skipped" policy). This test does not need to simulate the live REST
  perturbation -- `Program::compile` has no visibility into runtime state at all, which is exactly the
  point: it must always hand the restore value to the caller (`Player::firePreamble` ->
  `Sink::touch/set/release`) and let the live dispatch decide, never pre-emptively decide "no one could
  possibly need this write."

### RED-then-GREEN (reproduced this session, scratch build, not inherited)

Built `test_program_preamble` twice against the SAME updated test file, toggling only the
fix/no-fix state of `Program.cpp` (Edit tool, not a git operation -- no shared-stash risk):

- **RED** (pre-fix `Program.cpp`, current test file): `FAILED: p->preambleContinuous.size() == 5 for: 3
  == 5` (test 1) and `FAILED: foundOpacityRestore for: false` (test 6). `6 test cases | 4 passed | 2
  failed`; `145 assertions | 143 passed | 2 failed`. Exactly the two tests touched by this bug fail;
  nothing else regresses.
- **GREEN** (fix reapplied, verified byte-identical to the intended fix via `diff` before rebuilding):
  `All tests passed (169 assertions in 6 test cases)`.

## 5. Full ctest count

Built all 45 test executables (43 from `grep -oP '(?<=^add_executable\()\S+' tests/CMakeLists.txt`,
plus `test_httplib_bodyless_post` and `syphon-check` which live inside `if()` blocks the anchored grep
missed) in a fresh scratch build dir inside this worktree
(`.claude/worktrees/wf_d61ff7af-028-1/build-rrfix`, never the main checkout's `build/`), configured
with `-DFETCHCONTENT_FULLY_DISCONNECTED=ON` and `-DFETCHCONTENT_SOURCE_DIR_{JUCE,HTTPLIB,CATCH2,SYPHON}`
pointing at the main checkout's already-populated `build/_deps/*-src`. No app binary built or launched;
no debugger used.

```
cd build-rrfix && ctest -j4
100% tests passed, 0 tests failed out of 484
Total Test time (real) = 51.70 sec
```

484 is HIGHER than the plan's "445 -> 445+7=452" estimate because this branch was created from main's
CURRENT tip (commit 1631e99), which already carries several other s-rta-0925 lanes merged after the
replay-restore lane (visualfix, rclick2, opacity, step4polish, downbeat, etc.) -- each added its own
test targets. This session's own diff adds exactly one new test case (`test_program_preamble.cpp`
test 6) plus the size-3->5 update to test 1's assertions on top of that baseline; the 484 count is the
real, re-run number for this branch, not carried forward from any prior report.

## 6. Residual risk / what would sharpen this further

- The probe's implicit timing assumption (1.5s window vs a 1.9487s first lane point, ~450ms margin)
  is now load-bearing again post-fix. It held throughout this project's prior gate runs before this
  bug was introduced and should hold again now that the opacity restore completes synchronously inside
  `RecorderHost::play()` (confirmed by source read: `Player::firePreamble` is called synchronously,
  no `callAsync` inside it, `RecorderHost.cpp:678-682`). I did not re-run the live gate myself (rig
  rules forbid launching the app / GUI / screenshots) -- Harmony's next live-gate run is the
  authoritative confirmation that the fix clears both FAIL rows.
- I did not verify against a physically rebuilt pre-fix COMMIT (e.g. `b187781^`) -- the RED build used
  the Edit tool to toggle the same source file back to its pre-fix text, then forward again, matching
  the prior reviewer's own disclosed methodology for the original RED commit (symbol-non-existence /
  toggle-and-rebuild rather than a full historical rebuild). `diff` confirmed byte-for-byte restoration
  of the fix before the final GREEN rebuild.
