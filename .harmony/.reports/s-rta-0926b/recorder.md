# Lane recorder -- s-rta-0926b -- report

STATUS: DONE
Branch: `lane/recorder-tempo-0926b`, worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-3`.
Base: `main @ bc69fd0` (code `6e8f120`). Head: `89829e0` (fix commit `598c559`, notebook commit `89829e0`).

## RESULT

**OBJECTIVE 1 (fix) -- DONE, live-verified.** The provisional `take.json` written at `arm()` has an
empty `tempoMap` because `RecorderClock` has not ticked yet; a crash/kill before the first periodic
save (`kCheckpointSeconds` = 60s) or before Stop recovered a take with `tempoMap: []`, which
`RoutineSlice::checkMetered` refuses ("no beat grid") -- even after ebbff22/e5ceb98 already fixed the
periodic/final save sites. Fixed with one extra, one-shot save inside `RecorderHost::tick()`: the
first tick whose `clock_.now().bpm > 0` (Manual BPM: tick 1 itself; detected BPM: whichever tick the
tracker locks) triggers the SAME `takeForSave()`/`Take::save()` path the periodic save already uses,
independent of the 60s cadence. Manual-BPM and detected-BPM both covered by the same mechanism (it
keys off the clock's own metered state, not the BPM source). Saves stay message-thread only (same
`RECORDER_HOST_ASSERT_MESSAGE_THREAD()` contract everything else in this class already has) -- no
audio/analysis-thread contact, no new mutex.

**OBJECTIVE 2 (diagnose only) -- DONE, disk read-only.** Full census of
`~/Documents/Audio-DNA/Takes` (157 takes, read-only, nothing modified/moved/deleted): every one of
them has `tempoMap: []`. 150 predate e5ceb98 (17:08:47), 141 predate ebbff22 (16:14:49) -- expected,
no migration exists. 7 postdate BOTH merges (e.g. `step3gate1`/`step3gate2`, recordedAt 18:16-18:20,
duration ~15s, i.e. cleanly disarmed, not a crash) and are STILL empty -- this is disk evidence for
exactly the gap this lane just fixed (a completed, sub-60s take that only ever got the always-empty
provisional save, or was recorded against a binary that predates the fix; the read-only main app at
`/Users/boriskarpman/projects/RealTimeAudio/build` was rebuilt at 18:12:35, after both merges, so it
is not itself the explanation -- see NUANCE). All 157 takes are dev/probe artifacts by name
(`finloop-*`, `probe-routines-*`, `step3gate*`, `s4gate`, `resid*`, `lat*`, `shot_*`) -- no user
performance take is at risk. Migration options in `open_forks` id `C-migration` below.

## FACTS (disk-cited)

- Fix: `src/recording/RecorderHost.h:318-324` (new `earlyTempoSaved_` field + comment),
  `src/recording/RecorderHost.h:150` (tick() doc), `src/recording/RecorderHost.cpp:259-260` (reset at
  arm), `src/recording/RecorderHost.cpp:495-511` (the early-save trigger, folded into the existing
  periodic-save block).
- Tests: `tests/test_recorder_host.cpp:2374-2493` (two new `TEST_CASE`s, tag `[host][tempo][earlysave]`);
  one comment fix at `tests/test_recorder_host.cpp:236` (a pre-existing test's "no save yet" comment,
  now inaccurate because that tick's bpm > 0 fires the new early save -- the assertions were already
  correct, only the comment needed a note).
- ctest, serial, `build-lane`: **582/582 passed** (`ctest --test-dir build-lane`, Total Test time 12.54
  sec). Pre-fix baseline was 580/580 (HANDOFF); +2 = the two new test cases.
- `test_recorder_host` alone: **566 assertions in 40 test cases, all passed** (post-fix); the two new
  cases were independently proven RED on the pre-fix code (see METHOD) before being proven GREEN here.
- LIVE witness, fix build (`build-lane`, this lane's binary): armed with `set_bpm(120)` then
  `POST /api/perf/record {"audio":false}`; `GET /api/perf/status` at `t=1.475s` (`bpm: 120.0`,
  `recording: true`); disk read of that take's `take.json` at the same moment:
  `"tempoMap": [{"t": 0.0, "beat": 0.0, "sample": 330752, "bpm": 120.0, "why": "start"}]`.
- LIVE witness, baseline app (`/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app`,
  read-only, built from `6e8f120`): identical procedure, `t=1.517s`, `bpm: 120.0`, `recording: true`;
  disk read of `take.json`: `"tempoMap": []`. RED confirmed live, not just in ctest.
- Take census: `python3` scan of `~/Documents/Audio-DNA/Takes/*/take.json` (157 dirs, 0 parse errors):
  `empty tempoMap: 157/157`; `predate ebbff22 (16:14:49): 141`; `predate e5ceb98 (17:08:47): 150`.

## METHOD

1. Read `docs/claude/recording.md`, `.harmony/.reports/s-rta-0926/review-tempomap-r1.md`,
   `.harmony/.reports/s-rta-0926/routine-grid.md`, and the three named notebook sections before
   touching anything.
2. Read `RecorderHost.{h,cpp}` (arm/tick/disarm/`takeForSave`), `RecorderClock.cpp` (anchor timing),
   `RoutineSlice.cpp`'s `checkMetered` (what "no beat grid" means on disk) to locate exactly where the
   gap is: `takeForSave` is already correct (ebbff22); the gap is the SAVE SCHEDULE, not the copy.
3. Implemented the one-shot early save (see FACTS), reusing the periodic save's code path rather than
   duplicating it -- `periodicDue || earlyTempoDue` guards one `if` block; each flag updates only its
   own latch (`lastCheckpointT_` / `earlyTempoSaved_`).
4. Wrote two `TEST_CASE`s driving the real `RecorderHost` (no mirror of production logic): Manual BPM
   (metered from tick 1) and detected BPM (a real unmetered-then-lock transition, same phase-integration
   idiom the existing tempomap test uses so the D1 loader lint stays honest, not fabricated).
5. **RED-on-base proof (mutation on a copy, not the committed deliverable):** `git diff` of only the two
   source files was saved to a scratch patch (`/private/tmp/.../scratchpad/tempomap-fix.patch`),
   `git apply -R`'d to revert JUST the source fix (tests untouched), rebuilt `test_recorder_host`, ran
   the two new cases -- both FAILED (`REQUIRE_FALSE(crashed->tempo.a.empty())` / `REQUIRE_FALSE(locked->tempo.a.empty())`).
   `git apply`'d the SAME patch back (verified `git diff --stat` matched the pre-revert diff exactly),
   rebuilt, reran -- both PASSED. This is the same information a `git stash` round-trip would give,
   without touching the shared stash stack.
6. Built the full app (`AudioDNA` target) and the full test suite in `build-lane`; ran the LIVE witness
   (lock acquired/released, `open -g`, graceful `osascript` quit, no lldb/debugserver, no full-screen
   capture) against both the fix build and the read-only baseline app, in that order, each under the
   single live-app lock.
7. Ran `ctest --test-dir build-lane` (serial) for the final count.
8. Census of `~/Documents/Audio-DNA/Takes` via a read-only Python scan (no take moved/modified/deleted).

## CONFIDENCE + VERIFY

Confidence: HIGH on OBJECTIVE 1 (ctest RED/GREEN + live REST RED/GREEN on two different real binaries,
both reproduced with the SAME procedure). MEDIUM on the "why 7 post-merge takes are still empty" claim
in OBJECTIVE 2 -- I did not forensically identify which binary recorded `step3gate1`/`step3gate2`
(out of fence: would need `otool`/binary inspection of build-lane apps under 3 concurrent lanes, and the
packet asks for diagnosis of the AGGREGATE pattern, not a per-file forensic trace). Verify: re-run
`ctest --test-dir build-lane -R "tempo"` (or the whole suite) on this branch; re-run the LIVE witness
procedure in FACTS against any build.

## UNKNOWNS / NOT DONE

- Did not determine which specific binary recorded the 7 post-merge-but-still-empty takes (see NUANCE).
- Did not implement any migration (out of scope by design -- Harmony decides, see `open_forks`).
- Did not touch the OPEN manual-BPM phase-reset-on-room-noise bug (notebook, `s-rta-0926 routine-grid`)
  -- outside this lane's fence and objective.

## NUANCE

The main app binary at `/Users/boriskarpman/projects/RealTimeAudio/build` was rebuilt at 18:12:35,
which is AFTER both ebbff22 (16:14:49) and e5ceb98 (17:08:47) -- so it is not, by itself, an obviously
stale binary that would explain the 7 anomalous post-merge-empty takes. The more likely explanation
(INFERRED, not verified) is that those specific probe runs used a DIFFERENT app instance -- one of the
sibling lanes' `build-lane` binaries (three concurrent worktrees were building/running apps this
session) built from a commit or branch state that does not yet include this session's fixes, or a run
that never reached a real save (crash/kill) despite `meta.duration` looking like a clean stop (duration
is written by the FINAL save itself, so an empty-tempo file with a real duration is only possible if
that file's own last write WAS the final save on a binary without ebbff22, or -- after THIS fix ships
-- a binary without this lane's patch too). Either way, this is disk evidence *for* the gap this lane
closes, not evidence against it; my own live witness independently reproduces the exact same shape
(`tempoMap: []`, non-zero `t`, mid-take) on the read-only baseline app.

## HANDOFF-NEEDS

None to build further code. Harmony: please rule on `open_forks` id `C-migration` (below) for the 157
existing empty-tempo takes; no code changes are blocked on that ruling.

## FILES CHANGED

- `src/recording/RecorderHost.h` -- new `earlyTempoSaved_` member + comments (fence: TARGET).
- `src/recording/RecorderHost.cpp` -- reset at `arm()`; the one-shot early-save trigger folded into
  the existing periodic-save `if` block in `tick()` (fence: TARGET).
- `tests/test_recorder_host.cpp` -- two new `TEST_CASE`s (`[host][tempo][earlysave]`); one comment
  correction on a pre-existing test (fence: TARGET, tests/).
- `.harmony/notebook.md` -- one new dated section (fence: explicitly allowed by the packet's protocol).
- This report.

No file outside the fence (`src/recording/**`, `tests/**`, `.harmony/probe-routines.*`/new probe,
`tests/CMakeLists.txt`) was touched. `src/render/**` and `src/analysis/**` untouched, as required.

## TESTS

- New: `RecorderHost tempo map -- Manual BPM: an early crash (no periodic save, no disarm) still
  recovers a metered beat grid that agrees with the final take` -- RED on base
  (`REQUIRE_FALSE(crashed->tempo.a.empty())` fails), GREEN after.
- New: `RecorderHost tempo map -- detected BPM: no beat grid while the tracker is unlocked; one
  appears the moment it locks, well before any periodic save` -- RED on base
  (`REQUIRE_FALSE(locked->tempo.a.empty())` fails), GREEN after.
- Existing `test_recorder_host` suite: 40/40 test cases, 566/566 assertions, unaffected (one comment
  updated for accuracy, no assertion changed).
- Full serial ctest: **582/582 passed** (Total Test time 12.54 sec).
- LIVE: REST witness on both the fix build and the read-only baseline app (see FACTS) -- RED/GREEN
  reproduced live, not just in ctest, per the packet's requirement ("No kill -9 crash simulation:
  reading the provisional file is the witness").

## ISSUES

None found requiring a fix outside this lane's scope. The 7 anomalous post-merge-empty takes
(OBJECTIVE 2) are disclosed above (NUANCE), not a blocking issue for this fix.

## SKILL_PROPOSALS

None -- this followed the project's existing "drive the real RecorderHost + an independent reference
clock" test idiom (already established by the tempomap/routine-grid lanes); no new reusable procedure.

## RISKS

- The early save writes to disk on the message thread on whichever tick first observes `bpm > 0`.
  This is a single extra `Take::save()` call over the take's whole lifetime (one-shot, latched by
  `earlyTempoSaved_`), same cost class as the existing periodic save -- no steady-state cost added.
- Does not change replay behavior: `Program.cpp` only reads the tempo map for stampless gestures (the
  `!haveStamps` fallback); every live-recorded gesture has stamps, so turning the map from
  "always eventually populated" to "populated earlier" changes nothing for playback, matching the
  ebbff22 review's own finding.

## METRICS

- Files changed (code): 2. Files changed (tests): 1. Files changed (docs): 1 (notebook) + this report.
- New tests: 2 (both proven RED on base, GREEN on fix, in addition to a live RED/GREEN pair on real
  binaries).
- Lines: +149/-5 (fix + tests commit), +26 (notebook commit).
- Build: cold configure + `AudioDNA` + full test suite in `build-lane`, ~30 min wall total across
  several background builds.
- ctest: 582/582 (serial), up from the 580/580 baseline (+2 new cases).

## KNOWLEDGE CONTEXT

No KNOWLEDGE_TOOLS block in this packet; no graphify graph consulted. Impact traced by direct source
read (`RecorderHost.cpp`/`.h`, `RecorderClock.cpp`, `RoutineSlice.cpp`, `Program.cpp`'s tempo-map
consumer) plus the existing pinned ctest (`Program::compile takes each breakpoint's x from its own
stamp`) as an independent guard that replay is unaffected. Risk level: NORMAL (single, well-contained
class; no god-node/cross-cutting signal from a grep-only pass).

## PACKET QUALITY

- Clarity: CLEAR. The packet named the exact bug, the exact file/method (`RecorderHost::takeForSave`,
  the three save sites), the exact spec constraint (D1 beat-grid consistency), and even suggested the
  mechanism ("write an extra save right after the clock's first tick that has a tempo") -- which is
  what got implemented, after tracing why a naive "save on every tick until metered" would be wasteful
  and confirming the one-shot latch matches "as soon as one exists" exactly.
- Missing context: none needed asking about; had to work out myself that the early save must NOT be
  expected to also carry gesture data captured strictly after the one-shot save fires (my first test
  draft wrongly asserted a post-save-captured point would appear in the crashed snapshot; caught and
  fixed before reporting, not shipped).
- Unused context: OBJECTIVE 2's "offline re-analysis of stored audio" migration option was written up
  as an option, not attempted (diagnose-only, per the packet).
- Self-brief files: `docs/claude/recording.md` (existed, useful -- documents `RecorderHost`'s
  arm/tick/disarm contract and the Routines slice-1 surface). `review-tempomap-r1.md` (existed, useful
  -- pinned exactly which lines/threading claims were already independently reviewed, so I did not
  re-derive them from scratch). `routine-grid.md` (existed, useful -- the phase-integration idiom I
  reused for realistic test driving came from here). `.harmony/notebook.md` sections (existed, useful,
  all three named sections present and current).

## STATUS

DONE.

## NEXT ACTION

Harmony: rule on `open_forks` id `C-migration`. No further code action needed from this lane.
