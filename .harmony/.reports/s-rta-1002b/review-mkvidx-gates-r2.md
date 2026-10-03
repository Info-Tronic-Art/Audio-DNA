# Reviewer Verdict -- mkvidx gates r2
STATUS: DONE
VERDICT: PASS (APPROVE; 0 MUST, 0 SHOULD, 1 NIT)
REVIEWED: lane/mkvidx @ 4a41ddc0d3cd8d862c781594dbe3c79df5c6ee4c (git show / diff 79dd452..4a41ddc; build-lane ctest -N read-only; mutants in a mktemp copy)
METADATA: reviewer=gates-lens-r2, date=2026-10-02

## Verified (executed here)
- `python3 .harmony/probe-vupload-ab.py --selftest` at head: 226 `ok`, 0 FAIL, `SELFTEST PASS` (rc 0).
- R3 teeth, independently: 35 mutants of the u13 block in a scratch copy (my own script, not the lane's abmut.py): each [CTRL-A] (i)/(ii)/(iii) comparison
  and each rule comparison ([FREEZE]/[MKV]/[HAP] uploads, late, bracket, mono, nonmono; [HAP] dpu; [CPU]; [PARITY]; [GUARD] late and per_player_min;
  [PP-PARITY]) neutralised to True AND to False, plus the 5-launch gate of every rule -> ALL 35 turn `SELFTEST FAIL`, each failing on the case named for
  that comparison (e.g. `bd <= cr*ad`->True fails only u13-[CPU]-over; `fl <= gl+pp`->False fails u13-all-pass). Matches the lane's 16-mutant table.
  Cases at selftest-ab.py:113-161: every rule has one just-over (that rule alone FAILs, asserted as an exact tag set) and one just-under (PASS); bars match
  probe-vupload.json (28.5 / 10 / 1.25 / 0.7 / 1.15 / 15 / 1.5 / 15 / 20 / 1.3) with 0.1-0.01 margins; the [CTRL-A] cases assert the exact INFO line flips.
- R4: probe-vupload-ab.py:389-394 binds bracket_ok / mono_ok / nonmono with `all(...)` per B launch (each list non-empty; a missing key -> None != 1 -> FAIL);
  uploads/s and late stay medians as ruled. The probe emits one DATA line per scene per launch (probe-vupload.py u13), so the per-launch list is per launch.
  Covered by the 9 one-bad-launch-of-five cases (launch 3 only, median still good); mutants all(..)->True are killed. Docstring, "_u13" (probe-vupload.json) and
  testing-eyes.md say "EVERY B launch, not medians". Frozen blob changed by this wording (bars untouched) -- the lane flagged it for Harmony to record before G6.
- ctest -N (build-lane) = 1123 (= 1122 + mkvidx T2f); 9 "mkvidx" entries. CLAUDE.md untouched in the fix round: 23,976 B (<= 25,000).
- R1: VideoPlayer.cpp readKeyIndex early-return `(!atOpen && intraOnly_)` + `++stats_->keyIndexRebuilds` (null-guarded) + T2f (test_gop_cache_store.cpp:2068, bar
  rebuilds <= 2, non-vacuity CHECK entriesNow > entriesAtOpen + 2); RED 22 -> GREEN 0 in the lane report (not re-built here). R2: no code change, GopCache.h
  comment + Pitfall 64 (3) + rendering.md name F8 / F6; F8 filed in the lane report. NIT keyIndexWitnessed_ reset: one line (VideoPlayer.cpp:1615). No new defect found.

## Findings
1. [NIT] selftest uses the 0.1-wide margin on each bar but never the exact-equality case for `>=` bars (uploads 28.5 exactly); `>=` -> `>` would survive. Harmless
   (a real run never lands on 28.5000 from a median of floats); record only.

## Residual (unchanged, not defects)
- Live A arm / G6 unmeasured; R4's blob hash must be re-recorded by Harmony after this round (before the first G6 launch).
