# Reviewer Verdict — replay-restore r1
STATUS: DONE
VERDICT: APPROVE

## Scope
Independent review (did not build) of `lane/0925-replay-restore`
(`git diff main...lane/0925-replay-restore`, worktree
`.claude/worktrees/wf_63eb1225-562-1` @ b187781) against
`.harmony/.reports/s-rta-0925/plan-roadmap.md` §3 (D4 preamble: "replay
restore"). 17 files, +1127/-58. Commits: 2a89deb (RED tests) then b187781
(implementation).

## Checklist results

1. **Preamble filled in compile and fired in play, before audio transport
   start** — VERIFIED. `Program::compile` (`src/recording/Program.cpp:416-421`)
   calls `buildPreamble(take.checkpoint0, comp, *program)` unconditionally,
   before the lane-merge loop. `RecorderHost::play()`
   (`src/recording/RecorderHost.cpp:653-666`) calls `player_->start(0.0)` then
   `player_->firePreamble(*sink_)` **before** `playing_ = true`.
   `MainComponent::perfPlay` (`src/MainComponent.cpp:5148-5169`) calls
   `recorderHost_.play(...)` (which synchronously fires the preamble) and
   only afterward, for `withAudio`, calls `applyAudioTransport("play", ...)`
   — the audio transport start is strictly ordered after the restore. Traced
   the whole call chain; no reordering risk found.

2. **Held-control refusals and unresolved layers/decks counted, never
   silent** — VERIFIED. Discrete refusals bubble through
   `Player::firePreamble`'s return value into
   `RecorderHost::preambleRefused_`/`preambleFired_`
   (`RecorderHost.cpp:661-664`), published in `Status`
   (`RecorderHost.h:243`, `publishStatus()` `:779-786`) and surfaced via
   `/api/perf/status` (`MainComponent.cpp:5357-5361`) and a one-line notice
   in `perfPlay` (`:5233-5245`, F3 whole-word text). Unresolved
   deck/layer/clip/effect entries are pushed to
   `CompileReport::preambleUnresolved` at every failure site in
   `buildPreamble` (deck not found, layer not found, deck index out of
   range, clip index out of range, effect slot not found — read the full
   function, `Program.cpp:243-403`) and counted in `Status::preambleUnresolved`
   — none of these paths silently `continue` without an `Issue` push.
   `HostSink::fire` (`RecorderHost.cpp:65-77`) is careful not to
   double-count a Preamble-origin refusal into `skippedCount_`, matching the
   plan's "the probe pins `skipped == 1`" requirement — confirmed by reading
   the diff and by the new
   `"a fully-refused discrete Dispatch counts... never as skipped"` test
   case (`tests/test_recorder_host.cpp`, added section), which asserts
   `preambleRefused == 8`, `skipped == 0`.

3. **Threading — message thread only** — VERIFIED by static trace, no new
   evidence of any hop off it. `RecorderHost::play()` is guarded by
   `RECORDER_HOST_ASSERT_MESSAGE_THREAD()` (pre-existing macro, unchanged).
   `Player::firePreamble` runs synchronously inside that call, invoking only
   `Sink::fire/touch/set/release`, which forward to
   `dispatch.fire`/`dispatch.continuous.*` — the same lambdas a human
   click/REST callAsync already uses on the message thread
   (`MainComponent.cpp` constructor, unchanged wiring). No new mutex, no new
   thread, no GL/audio-thread code added anywhere in the diff — confirmed by
   reading every hunk in `Program.cpp`, `Player.cpp`, `RecorderHost.cpp`,
   `MainComponent.cpp`.

4. **RED evidence genuine** — VERIFIED. `git show 2a89deb^:src/recording/Program.h`
   / `Player.h` / `PerfState.h` contain zero occurrences of `PreambleSet`,
   `preambleContinuous`, `preambleUnresolved`, `preambleCount`,
   `firePreamble`, `fxParamKey/Slot/Index` — every symbol the RED commit's
   new tests reference. This is a disk fact (`git show`, not recall), so the
   RED-by-construction claim holds: those three test files could not have
   compiled at 2a89deb. Not physically rebuilt at that commit (matches the
   report's own disclosed time-budget inference) — this reviewer instead
   confirmed genuineness by symbol non-existence, which is sufficient to
   rule out a fabricated/copy-pasted-passing RED. Separately found
   corroborating (not authoritative — no committed builder report exists
   for this lane) evidence in `/tmp/build_full.log` (session-local, dated
   the same day) that a full build post-implementation DOES compile and
   link `test_program_preamble` successfully; no ctest execution log for
   this specific target was found on disk, so GREEN-after-fix is INFERRED
   from a successful link, not independently re-run by this reviewer.

5. **probe-step3.sh change: sound and fail-first pin meaningful** —
   VERIFIED. `bash -n .harmony/probe-step3.sh` passes (no syntax errors).
   Read the full diff (77 lines): `perturb_and_check_snapback` /
   `check_snapback_restored` helpers correctly read `CHK_COL`/`CHK_OP` from
   the take's own `checkpoint0` (never hardcoded), perturb via REST, assert
   the perturbation landed, `sleep 1` to clear the Decaying grip
   (`gripHoldMs`), then assert the post-Play snap-back within 1.5s and the
   three status counters. `seq_matches_expected`'s leading-drop logic
   correctly handles both poll-race orderings (R10) and never drops a
   genuine leader equal to `EXPECTED_SEQ[0]`. All helper functions it calls
   (`comp_active_col`, `take_field`, `perf_field`, `normnum`, `ok`/`no`)
   already exist in the file. This is a real fail-first pin: pre-fix,
   `RecorderHost::play()` never touched checkpoint0, so the perturbation
   would persist through Play and the snap-back checks would FAIL — matches
   the report's account; not independently re-run against the pre-fix
   commit (time budget), consistent with the packet's own disclosure.

6. **Scope fence respected (no end-of-replay change)** — VERIFIED. Grepped
   `RecorderHost.cpp` for `tick(`/`stopPlay`/`pos >= length` — zero diff
   hunks touch any of them; `RecorderHost::stopPlay()` and
   `stopPlayback()` are untouched. The plan's §3.11 "next micro-step"
   (auto-stop at end of replay) was correctly left alone.

## Other notes (non-blocking)

- `PerfStateCapture.cpp`'s key-encoding refactor to `PerfState::fxParamKey`
  is behavior-identical (`slot*100+param`, same stride) — confirmed by
  reading both sides of the diff.
- Plan §3.9 doc updates: only the `CLAUDE.md` one-sentence addition landed;
  `.harmony/APP-INVENTORY.md` and `binding-decisions.md` "TO VERIFY"->"BUILT"
  edits were skipped. The Builder's `.harmony/notebook.md` entry
  transparently discloses why (the worktree's `binding-decisions.md`, at
  its branch point, has no 2026-09-25 section to anchor the edit on — a
  verified, disk-checked fact, not an excuse) rather than silently dropping
  or fabricating the edit. Not part of this review's checklist; flagged for
  Harmony to decide whether to fold into a follow-up.
- New tests (`test_program_preamble.cpp` cases 1-5, `test_take.cpp`'s two
  `firePreamble` cases, `test_recorder_host.cpp`'s three-section case) all
  read as substantive, matching the plan's emission-order table and R4/R9
  edge cases exactly, including a genuine negative fixture (case 3: missing
  deck/layer both counted, not silently dropped) and a pre-existing test
  (`"RecorderHost clock -- t restarts..."`) correctly updated to isolate
  the recorded point's own timing from the new preamble fires by origin.

## Verdict
APPROVE. No blocking issues found across the six checklist items; the one
non-blocking gap (partial §3.9 docs) is honestly disclosed with a
disk-verified reason, not a silent scope drop.

FILES: src/recording/{Program.h,Program.cpp,Player.h,Player.cpp,RecorderHost.h,RecorderHost.cpp,PerfState.h,PerfStateCapture.cpp}, src/MainComponent.{h,cpp}, .harmony/probe-step3.sh, tests/{CMakeLists.txt,test_program_preamble.cpp,test_take.cpp,test_recorder_host.cpp}, CLAUDE.md, .harmony/notebook.md
ISSUES: none blocking. Suggestion (non-blocking): fold the deferred APP-INVENTORY.md / binding-decisions.md doc edits (plan §3.9 items 2-3) into a follow-up once the worktree's `main` snapshot catches up.
METADATA: reviewer=reviewer, builder_packet=s-rta-0925/replay-restore, date=2026-09-25T00:00:00Z, commits_reviewed=2a89deb,b187781
