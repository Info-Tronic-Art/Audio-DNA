## BUILDER REPORT -- lane gop2 (s-rta-0930)

STATUS: DONE
RESULT: The plan-gop2 items 1-5 are built as amended by ruling-gop2's ARCHITECT RULING, on branch `lane/gop2` in the worktree `.claude/worktrees/gop2`, in 4 commits: c1 bb7eb84 (probe), c2 e4f1a70 (GC7 in-place window + A1 real keyframes), c3 9b40721 (R3 landing-key store gate + A2 pts clause) and c4 (docs + this report). Every pre-registered RED reproduces on 655d232 (or on its named mutant) and every GREEN bar holds on the committed code.
FACTS: `src/media/GopCache.h:373` servedDuringLead, `:382` keyAtOrBefore, `:392` kInPlaceMargin + `:393` struct Lead, `:430` the in-place loop, `:298` storeGateOpens; `src/media/VideoPlayer.cpp:297` keyRels_ built in open(), `:1317` the Lead passed, `:1448` the store gate, `:1615` seek reset, `:1644` landing branch; `tests/test_gop_cache.cpp:467` (T1a), `:570` (predicate); `tests/test_gop_cache_store.cpp:1383` (T1b), `:1434` (T1c), `:1507` (T1d), `:1554` (T3); `.harmony/probe-vupload-ab.py:26` (selftest), `.harmony/probe-vupload-ab.sh:29` (burners). ctest 1055/1055 serial (`ctest --test-dir build-lane -j1`); TSan 0 reports on 3 targets; `probe-vupload-ab.py --selftest` SELFTEST PASS.
METHOD: RED first. The new tests were compiled against 655d232's `src/media` with a relink rig: the committed test TU + a scratch copy of the sources, the main `build/`'s flags and link line used read-only, TEST_FIXTURES_DIR repointed at the worktree fixtures, output to scratch. GREEN was measured on the rig and then on the lane build (`build-lane`, Release, TEST_SERVER + SYPHON). Mutants m1-m6 were compiled only as scratch copies; the deliverable was never mutated. The full serial ctest ran on the c2 state (1053) and again on the c3 = final C++ state (1055). G2 TSan used `build-lane-tsan` (RelWithDebInfo, -DADNA_SANITIZE=thread). G3 = selftest + the ab256 replay. There was one live u8 smoke at 256 MB under the lock (INFO).
CONFIDENCE+VERIFY: High for the offline claims: deterministic tests, and the values reproduce the ruling's tables exactly. The live G4-G7 are Harmony's. To re-check: `ctest --test-dir build-lane -j1` -> 100 % of 1055; `build-lane/tests/test_gop_cache_store "gop2*"` prints the gop2 T1b/T1c/T1d/T3 lines tabled below; `python3 .harmony/probe-vupload-ab.py --selftest` -> SELFTEST PASS; the ab256 replay prints no "cap 0.0" rule line, and its five cap-256 verdicts equal summary.txt's.
UNKNOWNS/NOT-DONE: G4 (GC7 live 5 x 2), G5 / G5b / G6 / G6b / G7 are not run (Harmony's gates). The single live u8 smoke is INFO (1 launch, not GC13's median). The rendering.md placeholder "[G4's medians, filled in by Harmony]" awaits G4. APP-INVENTORY counts are Harmony's.
NUANCE: (1) The A/B driver counts burners only while an Audio-DNA process runs. Each probe launch encodes its fixtures with `ffmpeg` BEFORE the app starts, so a whole-launch sampler would taint every launch without a VIDEO_FIXTURES dir. This is a refinement of A9 (see ISSUES 1). (2) `landingPts_` is initialised with `INT64_MIN`, not `GopCache::kNoPts`: VideoPlayer.h only forward-declares GopCache, and the `static_assert(AV_NOPTS_VALUE == GopCache::kNoPts)` in the .cpp pins the value (ISSUES 2). (3) The A1 hunk sits inside open() (VideoPlayer.cpp:278-299), 18+ lines above the first std::cerr block; lane tsan's reserved lines are untouched.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
- GC7: a PREFETCH window now also counts the slots that the clock frees while the run decodes its lead-in. The lead-in is measured from the container index's real keyframe.
- R3: the reverse store gate also opens after a demuxer-key landing, but only for frames at or after the landing packet's pts.
- The u8 summarizer rules per B cap group and has a `--selftest`. The A/B driver taints launches that saw a burner.
- VFR and the index-less containers are documented as by-design, with numbers.

### FILES CHANGED
- `.harmony/probe-vupload-ab.py` (c1): the u8 bars are ruled per cap group of B. A's lines at that cap are the baseline, else all A lines, labelled "A baseline, cap X". A cap that only A ran prints INFO. No B u8 line at all is one FAIL. A group header line `-- u8 group cap_mb X` is added; the rule text and logic are unchanged. `--selftest` covers three synthetic TSVs with A13's exact counts.
- `.harmony/probe-vupload-ab.sh` (c1): `burners()` (`yes` / `stress-ng` / `ffmpeg`).
  - An orphaned burner before round 1 is listed, and the driver exits 1.
  - The sampler's third column = burners while the app runs; non-zero TAINTS the launch ("TAINTED (burner)").
  - The result line prints the end-of-launch load average.
- `src/media/GopCache.h`:
  - c2: `servedDuringLead`, `keyAtOrBefore`, `kInPlaceMargin`, `struct Lead`; `planPrefetch(..., const Lead& lead = {})` with two iterations of the in-place window.
  - c3: `kNoPts`, `storeGateOpens`.
- `src/media/VideoPlayer.h`:
  - c2: `keyRels_`.
  - c3: `firstPacketSinceSeek_`, `landedOnKeyPacket_`, `landingPts_` (decode thread only).
- `src/media/VideoPlayer.cpp`:
  - c2: open() collects the keyframe entries' timestamps -> `keyRels_` (relative to the FIRST keyframe's DTS); `planPrefetchRun` passes `Lead{ decodeMsEma_, frameMsEff(), gopFramesEst_, &keyRels_ }`.
  - c3: `seekToTimestamp` resets the landing flags; `decodeNextFrame` records the landing packet's key flag and pts; `onRunFrame` uses `storeGateOpens` (+ static_assert).
- `tests/test_gop_cache.cpp`: T1a (c2), the predicate case (c3).
- `tests/test_gop_cache_store.cpp`:
  - c2: accessors `pinDecodeMs` / `inPrefetchLeadIn` / `gopEst` / `keyRels`, helpers `keyCounts` / `Emulated` / `openCapped`; T1b, T1c, T1d.
  - c3: T3.
- `tests/fixtures/video_h264_gop250_64x64.mp4` (48,497 B), `video_h264_scenecut_64x64.mp4` (51,651 B) (c2); `video_h264_intrarefresh_64x64.mp4` (25,353 B) (c3). Each was encoded with its pre-registered command, and each is byte-identical to the ruling's scratch prototype (`cmp`).
- Docs (c4): `docs/claude/rendering.md` :79 (a)-(e) as amended; `docs/claude/pitfalls.md` Pitfall 62 (the rule-(3) insertion, the VFR sentence, six guard case names); `docs/claude/testing-eyes.md` :15; `CLAUDE.md` :225 (23,999 -> 23,996 B).

### TESTS (the committed code; raw values)

**RED on 655d232**, plus the new store cases on the rig. T1d's keyRels check and the keyRels accessor were stripped for this build only: `keyRels_` does not exist on 655d232.
```
gop2 T1b (a) CAP 21, K 3                      late 69 shown 246 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2220
gop2 T1b (b) CAP 16, K 4                      late 56 shown 250 mismatches 0 cacheMismatches 0/16 nonmono 0 decodes 2787
gop2 T1b (c) CAP 21, K 5                      late 0 shown 269 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2462
gop2 T1b (d) CAP 21, K 2                      late 403 shown 160 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 1744
gop2 T1b (e) CAP 21, K 3, EMA from a 2x seed  late 69 shown 246 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2220
gop2 T1b (f) CAP 21, K 3, pinned 2x           late 69 shown 246 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2220
gop2 T1c pause  fired@192 late pre 3 post 30 (total 33) shown pre 44 post 181 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c speed  fired@192 late pre 4 post 554 (total 558) shown pre 44 post 160 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c shrink fired@192 late pre 4 post 465 (total 469) shown pre 44 post 98 mismatches 0 cacheMismatches 0/10 nonmono 0
gop2 T1c slew   fired@192 late pre 0 post 38 (total 38) shown pre 47 post 208 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c flip   fired@192 late pre 3 post 0 (total 3) shown pre 45 post 222 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1d CAP 16, K 3  late 0 shown 268 decodes 1213 (main 1213) mismatches 0 cacheMismatches 0/16 nonmono 0
gop2 T1d CAP 13, K 4  late 0 shown 269 decodes 1461 (main 1461) mismatches 0 cacheMismatches 0/13 nonmono 0
test cases:   3 |  1 passed |  2 failed
gop2 T3 (a) whole file: resident after lap 1 118 of 120 (missing: 118 119 ) | laps 2-3: seeks 6 runs 2 misses 4 decodes 166 | mismatches 0 cacheMismatches 0/118 nonmono 0
gop2 T3 (b) 12-frame cache, lap 2: seeks 169 misses 77 decodes 3321 shown 120 | mismatches 0 cacheMismatches 0/12 nonmono 0
```
- T1a and the predicate case do not compile on 655d232: `error: use of undeclared identifier 'servedDuringLead'`.
- T3 on 655d232 FAILS 7 checks: missing / seeks / runs / misses / checked (a); misses / decodes (b).

**GREEN, lane build `build-lane` (c3 = final C++), 14:34:55:**
```
gop2 T1b (a) CAP 21, K 3                      late 0 shown 268 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2186
gop2 T1b (b) CAP 16, K 4                      late 0 shown 268 mismatches 0 cacheMismatches 0/16 nonmono 0 decodes 2776
gop2 T1b (c) CAP 21, K 5                      late 0 shown 269 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2349
gop2 T1b (d) CAP 21, K 2                      late 42 shown 253 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 1698
gop2 T1b (e) CAP 21, K 3, EMA from a 2x seed  late 0 shown 268 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2186
gop2 T1b (f) CAP 21, K 3, pinned 2x           late 0 shown 268 mismatches 0 cacheMismatches 0/21 nonmono 0 decodes 2427
gop2 T1c pause  fired@192 late pre 0 post 0 (total 0) shown pre 46 post 192 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c speed  fired@192 late pre 0 post 337 (total 337) shown pre 47 post 265 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c shrink fired@192 late pre 0 post 428 (total 428) shown pre 46 post 110 mismatches 0 cacheMismatches 0/10 nonmono 0
gop2 T1c slew   fired@192 late pre 0 post 0 (total 0) shown pre 47 post 222 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1c flip   fired@192 late pre 0 post 31 (total 31) shown pre 46 post 214 mismatches 0 cacheMismatches 0/21 nonmono 0
gop2 T1d CAP 16, K 3  late 0 shown 268 decodes 1140 (main 1213) mismatches 0 cacheMismatches 0/16 nonmono 0
gop2 T1d CAP 13, K 4  late 0 shown 269 decodes 1398 (main 1461) mismatches 0 cacheMismatches 0/13 nonmono 0
gop2 T3 (a) whole file: resident after lap 1 120 of 120 (missing: ) | laps 2-3: seeks 0 runs 0 misses 0 decodes 0 | mismatches 0 cacheMismatches 0/120 nonmono 0
gop2 T3 (b) 12-frame cache, lap 2: seeks 23 misses 0 decodes 425 shown 120 | mismatches 0 cacheMismatches 0/12 nonmono 0
All tests passed (139 assertions in 4 test cases)
test_gop_cache_store: All tests passed (662 assertions in 26 test cases)      [523/22 + the 4 new]
test_gop_cache:       All tests passed (148 assertions in 13 test cases)      [111/11 + T1a 30 + predicate 7]
test_video_decode_trace (golden forward traces), 3 runs: All tests passed (510 assertions in 5 test cases) x3
```

**Each value against its pre-registered bar:**

| test | bar | main 655d232 | committed |
|---|---|---|---|
| T1a (i) values | 12 / 0 / 0 / 0 | compile fail | PASS |
| T1a (ii) pAt | == 17 | compile fail | 17 |
| T1a (ii) no Lead | None | compile fail | None |
| T1a (ii) Lead{1.8, 1000/30, 250} | Prefetch 232 / 218 / 218 | compile fail | Prefetch 232 / 218 / 218 |
| T1a (ii) keys {0, 250} | identical Run | compile fail | identical |
| T1a (ii) scene keys | None | compile fail | None |
| T1a (iii) | Lead 0.05 == no Lead (+ existing inputs, pAt 10 / 32) | compile fail | equal |
| T1a (iv) grid, 0 violations | 0 | compile fail | 0 in 12,898 states (= 2 x 6,449); 12,064 Prefetch runs, 10,924 grown |
| T1a (iv) avail0 == 0 | behindCap 10, pAt 38, None both | compile fail | as barred |
| T1a (v) keyAtOrBefore | 0 / 213 / 37 / 262 / 226 / 226 / 0 | compile fail | all equal |
| T1b (a) CAP 21 K 3 | late <= 4, shown >= 265 | 69 / 246 RED | 0 / 268 |
| T1b (b) CAP 16 K 4 | late <= 4, shown >= 265 | 56 / 250 RED | 0 / 268 |
| T1b (c) CAP 21 K 5 | late <= 4, shown >= 265 | 0 / 269 | 0 / 269 |
| T1b (d) CAP 21 K 2 | late <= 100, shown >= 240 | 403 / 160 RED | 42 / 253 |
| T1b (e) EMA 2x seed | late <= 4, shown >= 265 | 69 / 246 RED | 0 / 268 |
| T1b (f) pinned 2x | late <= 4, shown >= 265 | 69 / 246 RED | 0 / 268 |
| T1c pause | total <= 4 | 33 RED | 0 |
| T1c speed | total <= 558 | 558 | 337 |
| T1c shrink | total <= 469 | 469 | 428 |
| T1c slew 5 -> 3 | total <= 4 | 38 RED | 0 |
| T1c flip | post <= 90 AND pre <= 4 | 3 (post 0) | 31 (pre 0) |
| T1d CAP 16 K 3 | late <= 4, shown >= 265, decodes <= 1213 | 0 / 268 / 1213 | 0 / 268 / 1140 |
| T1d CAP 13 K 4 | late <= 4, shown >= 265, decodes <= 1461 | 0 / 269 / 1461 | 0 / 269 / 1398 |
| predicate (7 rows) | the table | compile fail | 7/7 |
| T3 sanity | key packets >= 3, key frames == 1 | 4 / 1 | 4 / 1 |
| T3 (a) whole file | 120 resident; laps 2-3 seeks / runs / misses <= 1 | 118; 6 / 2 / 4 (166 decodes) RED | 120; 0 / 0 / 0 |
| T3 (b) 12 frames, lap 2 | misses <= 2, decodes <= 700 | 169 seeks / 77 / 3321 RED | 23 / 0 / 425 |
| identity (every config) | mismatches 0, cacheMismatches 0, nonmono 0 | 0 / 0 / 0 | 0 / 0 / 0 |

- Every value equals the ruling's AMENDED column (N2-N8).
- T1c "fired@192": the event fired at frame 192 in both arms.
- The T1a grid count, `states 12,898`, is the two keys arms (none, {0, 250}) x 6,449.

**Mutants** (scratch copies, rig-compiled; the deliverable was never touched):
- m1, gain forced to 0:
  - FAILS T1b ((a) 69 / 246, (b) 56 / 250, (d) 403 / 160, (e) 69 / 246, (f) 69 / 246);
  - FAILS T1c (pause 33, slew 38);
  - FAILS T1a (ii) `REQUIRE( r.kind == RunKind::Prefetch )`.
- m2, gain bounded by the whole share (A2):
  - FAILS the committed "the budget: a cache capped at its floor" case: `perFrame := 4.548` > the bound (share 8);
  - FAILS T1d: 1352 / 1781 decodes > main.
- m3, landing ignored: FAILS T3 with main's exact numbers (118 of 120; 169 / 77 / 3321).
- m4, every landing treated as key: FAILS "GC3 keyframe gate: an MPEG-TS seek ...": `bad == 0` -> `29 == 0` (checked 60). This is the gopcache-fix m7 precedent's count.
- m5, keys ignored (the plan's grid):
  - FAILS T1d (1388 / 1690 decodes = the plan's grid column);
  - FAILS T1a (ii) `CHECK( plan(Lead{ 1.8, fms, 250, &sceneKeys }).kind == RunKind::None )`.
- m6, pts clause dropped: FAILS the predicate case `CHECK_FALSE( storeGateOpens(false, true, 999, 1000) )`. T3 is identical with and without it (the ruling's N8).

**Full ctest, serial (`ctest --test-dir build-lane -j1`):**
- c2 state: 14:32:53 start, "100% tests passed, 0 tests failed out of 1053".
- c3 = final C++: 14:36:14 -> 14:37:19, load avg { 2.58 3.77 4.59 } at the start, "100% tests passed, 0 tests failed out of 1055", "Total Test time (real) = 64.73 sec".
- `ctest -N`: 1055 = 1049 + 6 new cases (T1a, predicate, T1b, T1c, T1d, T3); 0 new targets.
- The c1 C++ is byte-identical to 655d232 (c1 touches only `.harmony/probe-vupload-ab.{py,sh}`), so no separate c1 ctest was run.

**G2 TSan** (`build-lane-tsan`, RelWithDebInfo, `-DADNA_SANITIZE=thread`; `TSAN_OPTIONS=halt_on_error=0:abort_on_error=0`), 14:37:33 -> 14:39:58:
- flags: `-fsanitize=thread` in flags.make (6 / 6 / 2) and link.txt (1 / 1 / 1); the tsan runtime is linked (otool) in each.
- test_gop_cache_store: rc 0, All tests passed (662 assertions in 26 test cases), TSan WARNINGs: 0.
- test_video_decode_trace: rc 0, All tests passed (510 assertions in 5 test cases), TSan WARNINGs: 0.
- test_gop_cache: rc 0, All tests passed (148 assertions in 13 test cases), TSan WARNINGs: 0.

**G3:**
- `python3 .harmony/probe-vupload-ab.py --selftest` prints all 12 checks ok, then "SELFTEST PASS", rc 0:
  - (i) capped-only: 0 PASS / FAIL lines naming cap 0.0; exactly 1 INFO "cap 0.0: pre-lane arm only -- no rule"; exactly 5 "u8 cap 256.0" rule lines, all PASS; exit 0.
  - (ii) mixed: exactly 6 cap-0.0 rule lines (pooled / GC6 / late / bytes / over_budget / hold) and 5 cap-256.0 lines, all PASS; exit 0.
  - (iii) A-only: exactly 1 FAIL "u8: no B launch"; exit 1.
- RED: 655d232's summarizer on TSV (i) prints six "cap 0.0" FAIL lines with None and exits 1, although every B value passes.
- Identity: when B covers every cap (a mixed TSV incl. FAILs), the new summarizer's 11 PASS / FAIL lines are byte-identical to 655d232's.
- The ab256 evidence (`evidence-0929b/ab-ab256/ab.tsv`, main tree), rc 1 (the late 123 FAIL: that data predates item 1). The rule lines are:
  ```
     INFO  u8 cap 0.0: pre-lane arm only -- no rule
     PASS  [GC7]  u8 cap 256.0 MB: B median slowest player 27.8 uploads/s >= 20
     FAIL  [ABS]  u8 cap 256.0: B median late 123.0 <= 40
     PASS  [ABS]  u8 cap 256.0: B median max video_gopcache_bytes 249.4 MB <= budget + floors 350.9 MB
     PASS  [ABS]  u8 cap 256.0: over_budget delta per B launch [0.0, 0.0, 0.0, 0.0, 0.0] (== 0 without a cap)
     PASS  [ABS]  u8 cap 256.0: hold_no_texture / pending 0 in every B launch
  ```
  `diff` of the cap-256 verdicts against summary.txt: IDENTICAL (5 lines); rule lines naming cap 0.0: 0.
- A9 driver, offline stub test: a stub lock lib + a stub probe; no app launched.
  - An orphaned `yes` before round 1 prints "orphaned burner -- kill it first: 74262 00:02 yes", exit 1.
  - A `yes` while the stub app is "up" prints "TAINTED (compiler + burner) -- re-run", then a clean re-run. The compiler count came from my own concurrent build.
  - The stub's own pre-app `yes`, which stands in for the fixture encode, never tainted ("burners seen 0").
  - Every result line carries "end load avg { ... }"; no `yes` was left over.

**Live smoke (INFO; one launch; my build; the lock):**
- 14:40:15 lock acquired; load avg { 2.68 3.23 4.15 }; burners 0; UNC 0; 0 Output-named windows before.
- `VIDEO_APP=build-lane/.../Audio-DNA.app (b61b67823830ede6) VIDEO_ENV=ADNA_GOPCACHE_BUDGET_MB=256 probe-vupload.sh u8_reverse_column_1080x4`; end load avg { 3.28 3.34 4.17 }:
  ```
  DATA u8_reverse_column_1080x4 cap_mb=256 uploads_per_s=120.8 per_player_min=30.2 per_player=30.2,30.2,30.2,30.2 late=0 hold_no_texture=0 pending=0 bytes_mb=249.4 frames=84 active=4 over_budget=0 cap_bytes_mb=64.0 decoded_per_upload=11.06 fps=119.98262786865234 footprint_delta_mb=-7.8 evictions=604
  PROBE-VUPLOAD GREEN
  ```
- After: UNC 0; "audio-dna windows 0, Output-named 0"; no Audio-DNA running; 14:40:29 lock released.
- INFO only: G4's 5 x 2 interleaved medians are the gate. The values agree with the ruling's predictions (late 0-15, dec/up 10.5-11.5, slowest ~30).

### SLIM CHECK
Nothing to cut. Every new symbol is named by the plan or the ruling. `gopEst` / `keyRels` / `inPrefetchLeadIn` / `pinDecodeMs` are each used by a test. The group header line `-- u8 group cap_mb X` is what the selftest parses to count rule lines per group. No speculative parameter.

### ISSUES
1. **A9 refinement (Harmony may overrule).** The burner sampler counts only while an Audio-DNA process exists (`adna` from the lock lib).
   - Why: `probe-vupload.sh` / `probe-video.sh` encode their fixtures with `ffmpeg` inside each launch, before `open`. Counting over the whole launch taints every launch that runs without a warm `VIDEO_FIXTURES` dir, and the run then stops after 20 taints.
   - The pre-round-1 orphan check counts all three names at any time, as ruled.
2. **`landingPts_` default.** The ruling writes `= GopCache::kNoPts` in VideoPlayer.h, but VideoPlayer.h only forward-declares `namespace GopCache` (VideoPlayer.h:18), and `kNoPts` did not compile there.
   - Used `INT64_MIN`, the same value; `static_assert(AV_NOPTS_VALUE == GopCache::kNoPts)` sits at VideoPlayer.cpp:1447. p8.py, the ruling's own prototype, did the same.
3. The plan's T1b (b) main value "251" is 250 here, matching the ruling's N2. The bar is unaffected.
4. My background lane build started before the c2 edits, so the app target kept stale objects. An incremental rebuild recompiled the app's VideoPlayer.cpp before any run; every ctest / smoke ran after it.
5. `rendering.md` (d) carries the placeholder "gop2: [G4's medians, filled in by Harmony]", as the plan directs.

### SKILL_PROPOSALS
None. The relink rig is a scratch tool, noted for the notebook below.

### RISKS
- K1 (the plan's): live contention beyond the offline rig. The one live smoke gave late 0, but the real gate is G4. Medium; mitigated by G4 itself.
- The rendering.md placeholder must be filled in or removed after G4. Low.

### NOTEBOOK NOTES (for Harmony to append to `.harmony/notebook.md`)
- **Relink a committed test target against scratch sources, and repoint TEST_FIXTURES_DIR.** A worktree lane can get RED on a base commit and compile mutants without building its own test targets: compile the committed test TU plus a scratch copy of `src/media` with the MAIN `build/tests/CMakeFiles/<t>.dir` flags.make and link.txt (read-only), output to scratch. The define `TEST_FIXTURES_DIR` points at the main checkout, so new fixtures are "not found" until it is rewritten to the worktree's `tests/fixtures`. | scratch gop2/rig/rig.py
- **A probe launch runs `ffmpeg` itself before the app starts** (the fixture encode). A burner detector that counts `ffmpeg` over the whole launch taints every launch; count only while Audio-DNA runs. | .harmony/probe-vupload-ab.sh
- **VideoPlayer.h keeps GopCache.h and FFmpeg out** (forward declarations only). A new member whose default is a GopCache / FFmpeg constant takes the plain value (`INT64_MIN`), with a `static_assert` in VideoPlayer.cpp. | src/media/VideoPlayer.h:299, VideoPlayer.cpp:1447
- **An emulated-decoder test counts decodes, never time.** T1b-T1d / T3 reproduced the ruling's scratch numbers exactly on both the rig (main build flags) and the lane build: counts are portable across builds of the same FFmpeg. | tests/test_gop_cache_store.cpp:1383

### METRICS
- Self-check:
  - builds: build-lane exit 0 (x3 incremental), build-lane-tsan exit 0; no warnings in touched files;
  - ctest: 1053/1053 (c2), 1055/1055 (c3);
  - TSan 0; selftest PASS; the golden traces 510/5 x3 (rig and lane).
- Tool calls: ~75 (Bash / Edit / Write).
- Files read: ~20 (plan, ruling, GopCache.h, VideoPlayer.h/.cpp sections, both test files, the probe .py/.sh, probe-vupload.sh, probe-video.py sections, lock.sh, rig2.py, scen.cpp, gc7step.cpp, irstep2.cpp, the p6 / p7 / p8 patches, the logs, docs).

### KNOWLEDGE CONTEXT
- Tools used: grep. No KNOWLEDGE_TOOLS block in the packet.
- Impact authority: grep (not authoritative); conservative posture taken.
  - Caller audit: `seekToTimestamp` / `av_read_frame` each have one definition. The seek callers are VideoPlayer.cpp :852 :881 :886 :897 :1351 :1537, all on the decode thread. `planPrefetch` has one production caller.
- God nodes in scope: n/a. Risk level: NORMAL. Dependencies discovered: none new.

### PACKET QUALITY
- Clarity: CLEAR. The plan plus the ruling specified every value, bar and mutant.
- Missing context:
  - The A9 sampler window: the fixture encode runs `ffmpeg` inside a launch.
  - VideoPlayer.h cannot see `GopCache::kNoPts`.
  - Both were resolved as noted in ISSUES 1-2.
- Unused context: the plan's CROSS-LANE bt2 line; the FILED F1-F4 (not this lane).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files:
  - plan-gop2.md, ruling-gop2.md, the scratch rigs (scen.cpp, gc7step.cpp, irstep2.cpp, p6 / p7 / p8, rig2.py) and the evidence-0929b ab256: all existed and were useful.
  - CLAUDE.md: read.
  - pulse.json: GREEN, no conflicting claim.

### STATUS
DONE. Items 1-5 are built as amended. Every RED and GREEN is recorded. The lane gates (full ctest serial, G2, G3, one INFO smoke) ran. Nothing was merged or pushed.
- Tree: clean except `build-lane/` and `build-lane-tsan/` (both kept).
- Lock released; no Audio-DNA running; 0 Output-named windows; no `.venv` symlink was created (the probe resolved python via VIDEO_PY / the main checkout).

### NEXT ACTION
Harmony: merge `lane/gop2`, then run G1-G7 per the ruling's FINAL GATE LIST. G4 is the GC7 decision; after it, fill in or remove the rendering.md placeholder.
