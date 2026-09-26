# Review: lane/0925-downbeat vs plan-downbeat.md (s-rta-0925, roadmap item 4)

VERDICT: PASS

## Method
Read every file in `git diff main...lane/0925-downbeat` (9 files, 351/-7). Cross-checked each
against `plan-downbeat.md` section 2 (spec) and `critic-downbeat.md` (independent pre-build
critique, VERDICT PASS). Independently reproduced the RED evidence (not trusted from the commit
message): copied the lane worktree into a `$TMPDIR` sandbox (never edited the reviewed tree),
configured a scratch CMake build reusing main's Catch2 FetchContent cache
(`-DFETCHCONTENT_SOURCE_DIR_CATCH2=.../build/_deps/catch2-src`), built `test_downbeat_detector`
(links only `BPMTracker.cpp` + Aubio — no full app build needed), ran it GREEN as committed, then
in the sandbox copy replaced `requireLevelContract(...)` with the task's premise as `CHECK`s
(the exact block from plan §2.2 step 1, wrapping the `&&` chain in parens — Catch2 3.x rejects an
un-parenthesized chained `&&` inside `CHECK`, a Catch2-version detail, not a plan defect worth
blocking on), recompiled that one translation unit, relinked against the unmodified
`BPMTracker.cpp.o`, and ran it.

## Independent verification results

**GREEN (as committed)**: `test_downbeat_detector "[cadence]"` → all 33 assertions pass.
Measured: `barsAdvanced=16 hopsTrue=752 hopEdges=16 | 60Hz true=480 edges=16 pulse=16 | 120Hz
true=960 edges=16 pulse=16 | 0.9s true=8 edges=8 pulse=16 delta=16`. Full target (`[cadence]` tag
plus the pre-existing 8 test cases): `All tests passed (182 assertions in 10 test cases)` — no
regression in the file, 2 new cases as the plan specified (ctest 445→447 claim is consistent).

**RED (premise re-enabled, sandbox-only)**: 4 `FAILED CHECK`s, matching the commit message and the
task dispatch's numbers exactly: `752 (0x2f0) == 16` FAILED, `481 (0x1e1) < 16` FAILED,
`16 < 16` FAILED, and the 120 Hz duplication check FAILED (readsTrue=960, not the ~1.26x/~20 the
premise predicted). This is genuine fail-first evidence, not a copied estimate: the numbers came
from the real `BPMTracker` driven through the file's existing helpers, and they refute the task
dispatch's stated premise ("downbeatDetected is a one-hop pulse") on every count. The GREEN
contract that replaced the premise block is the correct characterization: `downbeatDetected` is a
beat-long level (held ~28-94 hops), its rising edge fires exactly once per bar for any 60-120 Hz
reader, and only a poller slower than one beat (modeled here at 0.9 s) loses edges — where
`totalBarCount`'s delta (already-existing monotonic counter) does not.

## FILE-BY-FILE

FILE: src/analysis/FeatureSnapshot.h
  [OK] Correctness: comment-only change, replaces the wrong "true on the hop where beat 1 lands"
       with an accurate level description citing the exact assignment/edge-detect mechanism.
       Field type/layout/offset untouched — no snapshot-layout risk.

FILE: src/analysis/BPMTracker.h
  [OK] Correctness: comment-only change at the `downbeatDetected_` declaration, same content as
       FeatureSnapshot.h's fix, no logic touched.

FILE: src/api/ApiServer.cpp
  [OK] Correctness: adds `downbeatDetected` (bool) to `/api/bpm`'s existing `DynamicObject`, read
       from the same `featureBus_.read()` snapshot as `totalBarCount` two lines below — matches
       plan §2.1 exactly (line, comment, coherent-read rationale). HTTP thread, no lock/atomic
       added, identical discipline to its neighbours (§3 sacred-rules claim verified by inspection
       — `handleGetBpm` is a plain read-and-serialize).
  [OK] Sacred RT rules: no analysis/audio-callback/FeatureBus/render-thread code touched.

FILE: src/ui/TopBar.cpp / src/ui/AudioReadoutPanel.cpp
  [OK] Scope: deletes the two write-only `displaySnap_.downbeatDetected = snap.downbeatDetected;`
       copies (plan §2.4, "Harmony's call" — accepted). Independently grepped both files for any
       read of `displaySnap_.downbeatDetected`: zero, confirming dead-write status before deletion.
       `AudioReadoutPanel.cpp:70-74`'s LEVEL consumer (`if (snap.downbeatDetected) downbeatFlash_ =
       1.0f; else *= 0.85f;`) is untouched, correctly — that's the file's only live consumer and it
       is coherent with a level, not a pulse.

FILE: tests/test_downbeat_detector.cpp
  [OK] Test actually fails without the fix: independently reproduced (see above) — this is the
       load-bearing check for this review and it holds.
  [OK] Every consumer/regime covered: Test A drives the real-onset (`scoreBeat`) incrementer used
       live by mic/file audio; Test B drives the manual/predicted (`advancePredictedBeat`)
       incrementer, which is the exact regime the shipped live probe (`/api/set_bpm`) exercises —
       so the unit tests and the live probe test the same code path family, not two disconnected
       stories.
  [OK] No RT/hot-path impact: single-threaded, direct `BPMTracker` construction, same pattern as
       the file's pre-existing tests and `test_bpm_stabilization.cpp`.
  [COMMENT] The plan's own RED CHECK snippet (§2.2 step 1, the `&&`-chained `CHECK`) does not
       compile as literally written under Catch2 3.x ("chained comparisons are not supported
       inside assertions" — `BinaryExpr::operator&&` static_assert). Not blocking: it is scratch,
       throwaway, pre-GREEN-swap code that ships in neither the plan's file nor the commit (the
       committed file only carries the *results* as a comment, correctly using two separate
       `REQUIRE`s in the real GREEN contract). Filed as a doc nit for `plan-downbeat.md` if it is
       ever reused as a literal template.

FILE: .harmony/probe-downbeat-level.sh
  [OK] `bash -n` clean (verified in sandbox copy). Mirrors `probe-onset-render.sh`'s conventions
       exactly, including launching via bare `open --stdout/--stderr` (no `-g`) — this matches
       existing precedent in the repo (`probe-onset-render.sh:79` does the same), so it is not a
       new screen-safety gap; the script's own SCREEN-SAFETY LAW check (0 Output windows at
       teardown, graceful-quit-then-pkill, cleared logs before `open`) is present and correctly
       ordered. Not run live by this review (per task scope: "Harmony's live-app agent runs it,
       NOT the Architect, NOT the Builder" — and not the Reviewer either; this review's mandate is
       source correctness, which the sandboxed unit-test reproduction covers for the core claim).
  [OK] Oracle design: duty/run-length discriminates level (~0.25 duty, ~500 ms runs) from a
       one-hop pulse (~0.03 duty, <=45 ms runs) using only `/api/set_bpm`, no audio content needed
       — matches plan §2.6/§5 and the precedent probe's "no live audio" pattern.

FILE: CLAUDE.md / .harmony/APP-INVENTORY.md
  [OK] Pitfall 30's stale closing sentence corrected; new Pitfall 32 added with accurate content
       matching the verified mechanism (level, rising edge = totalBarCount, OnsetPulse recipe for
       a future pulse consumer, explicit "do not add downbeatCount" — correctly avoiding a
       duplicate field per plan §0.5/§1). FeatureSnapshot table row and APP-INVENTORY `/api/bpm`
       row both updated to match the real ApiServer.cpp change. Spec/claim fidelity: I opened
       ApiServer.cpp and confirmed the doc's claimed field name/route match the actual code
       (`obj->setProperty("downbeatDetected", ...)` on `/api/bpm`, not `/api/features` — doc says
       the same).

## SLIM CHECK
No new production abstraction, no new FeatureSnapshot field (the plan explicitly rejected adding
`downbeatCount` as `EXCESS_DUP` of `totalBarCount` — correct call, verified in §0.2/§0.5), no
speculative generality. Two dead lines removed (`JUSTIFIED_KEEP` n/a — they were `REMOVED`, 2
lines, write-only, zero behavior change, independently confirmed dead by grep above). Nothing else
to disposition.

## Scope / surgical fidelity
Every changed line traces to the plan. No adjacent refactors, no unrelated formatting. The plan's
own §2.5 "deliberately UNTOUCHED" list (AnalysisThread, BPMTracker.cpp, FeatureSnapshot layout,
Renderer/uploaders/shaders, TestServer, OnsetPulse.h, RecorderHost, Autopilot/Layer) matches the
actual diff — nothing outside that list was touched.

## Risk note (non-blocking, already filed by the plan itself)
Plan §6 names an adjacent finding (Autopilot beatInBar-vs-beatPhase-wrap skew) as "not this lane" —
correctly out of scope here; not re-litigated.

SUMMARY: 9 files reviewed, 0 blocking issues, 1 non-blocking doc nit (plan's throwaway RED CHECK
snippet needs parens to compile under Catch2 3.x — does not affect the shipped test file). Core
claim (test fails without the fix, on the task's own stated premise) independently reproduced
end-to-end in a sandboxed scratch build, not taken on the commit message's word. Every consumer
site accounted for and independently re-grepped. No sacred RT rule touched. No UI text change.
