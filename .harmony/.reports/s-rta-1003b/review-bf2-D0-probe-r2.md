# Reviewer Verdict -- bf2 D0 probe r2 (lane/bf2 740b6d6, fix round a852465..740b6d6)
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS) -- no MUST
FILES: .harmony/probe-sync.py, .harmony/probe-sync-selftest.py (fix round: only these two code files + bf2-d0.md; probe-sync.sh and src/ tests/ untouched; VERIFIED diff --stat)
Method: git objects only (diff a852465..740b6d6, show 740b6d6:<path>); text reads and greps; nothing built or run. Builder's logs under SP1 read as text.

## Round-1 MUST/SHOULD, fixed as found
M1 FIXED (probe-sync.py:1508-1521). A device-event take is "shifted" when c0 is None, c0 != 0, or (c0 == 0 and e' >= 384 from the valid takes' median). It no longer uses the take's own post-take block. The old repros are covered: (a) c0 512 under block 1024 -> "c0 512 is not 0"; (d) unreadable asset -> "asset could not be read". VERIFIED by reading + tracing the three new selftest cases (:340-355) through rd_take_record/rd_classify: take 1 is a device event by opens +1 / block change, c0 and e1 come out as the case states, valid counts 25/26/26 match QUIET(24)+N_ (+ take 2). The builder did NOT take "e (not only e') >= 384 from median e"; reasoning checked and correct: e1 = e - c0 (:1385), so with c0 == 0 e and e' are the same number; the literal e-form would add a false STOP when most valid takes are shifted. Accepted.
S1 FIXED (:1552-1557). A valid take with c0 >= its block and C None -> INCOMPLETE (exit 3) after O4/O2 and the half-missing rule, before the 24-count. It can only add a stop. Two cases (:306-312) both read O1 before the fix (stated in report; logic confirmed by reading: shifted nonempty, enough valid / met).
S2 FIXED as INFO WARN (:1567-1578). O5 outcome unchanged (the table is the ruling's); out["erefFar"] returned; first O5 case now asserts erefFar False, new case asserts True and the WARN text. done() still sets outcome before the extra lines, so the exit mapping in rd_verdict_files is unaffected.
S3 FIXED (:1390-1394, :1430-1435). gaps/hopsApplied absent -> None in the record, NOT VALID ("no gap field"/"no applied field") in the verdict, "n/a" on the take line (rd_num(None) = "n/a", :1398). Two cases (:376-398); expected strings traced against the code and match.

## Tests can fail, drive real code (VERIFIED/INFERRED)
- All new cases call ps.rd_take_record / ps.rd_verdict / ps.rd_take_line on synthetic RAW takes (no stubbing of the functions under test).
- Builder's mutants.log: 25 lines "MUTANT ... exit 1  RED", "mutants not RED: 0 of 25"; each of the 9 new mutants maps to the DIFF of the new case it should break (read). Probe sha256 in that log (17903a2c...c58ed51b) equals the sha of `git show 740b6d6:.harmony/probe-sync.py` that I computed: VERIFIED, so the mutant run is of this head's probe. selftest-green.log: 65 "ok", "0 case(s) differ" (read, not re-run). RED-before logs for each fix exist (m1/s1/s2/s3-red.log). I did not re-run any of it (instructed).
- Default path: the fix-round hunks are all inside rd_take_record / rd_take_line / rd_classify / rd_verdict (lines 1387-1578); none in R7/R7b/R5/G6/main/launch code. probe-sync.sh not in the diff. No src/ tests/ file in 68abc16..740b6d6 (stat shows 4 files: bf2-d0.md, selftest, probe-sync.py, probe-sync.sh comment-only). Nothing stray; no .venv entry in the tree; no on-screen text.
- Script quit/launch code, settings.json sha lines, Output-window count: unchanged by this round; the report's live launch at 6e461eb prints "PASS R6 settings.json sha256 unchanged" and "PASS 0 Audio-DNA Output windows"; it quit only pid 95642 (its own), and notes another lane's pid left alone (report text; INFERRED, not run by me).

## New defects introduced by the fix: none found
Checked: the new `blind` comprehension reads t["c0"] >= t["block"] only for valid takes (valid guarantees c0 not None and block truthy); `how` chain cannot raise (every t.get guarded; e1 non-None for c0 not None and e not None); hopsApplied None path cannot raise in rd_classify/rd_take_line; O5 return still returns the out dict with "outcome" set; erefFar absent on non-O5 outcomes (callers use .get / outcome only).

## SHOULD
S1 C is only compared inside ONE launch (probe-sync.py:1534-1546 cspans; builder's R1-4). A shifted take that is the only valid take of its launch, or a launch whose valid takes are all shifted, gets C span 0 and its rulers "agree" by default. Ruling-literal ("C one-valued inside every launch") so not a MUST, and it can only turn an O1 that is in fact unverified into O1, never a product two-valued state (e' two-valued is O2/O4 first). Suggest the architect decide (it is stated R1-4 in the report); a cheap tightening is a cross-launch C span check when shifted takes exist, as an INFO line.

## NIT
N1 A device-event take with c0 == 0 and e1 None (no markers paired) is not flagged "shifted" (:1517 requires e1 not None), although a c0-None take is ("cannot be shown unshifted"). Listed as DEVICE-EVENT only; the outcome rests on valid takes so no wrong outcome. INFERRED from code.
N2 Selftest holes from r1-N1 remain (builder said so): rate-only device event, settled False, "no device fields", exactly 20 clicks, c0 < 0 alone. Unchanged behaviour, untested.
N3 The new S1 INCOMPLETE is not cleared by more launches (report R1-1, hand-over text says so). It extends the ruling's INCOMPLETE conditions; honest, documented, and cannot give a wrong outcome. Harmony/architect should know it escalates, not loops.
N4 r1 N3/N4/N5 (RD without R7 runs nothing; chain-wise click collapse; row_rd duplicates row_r7 loop) unchanged, accepted in the report.

SUMMARY: 2 code files in the fix round, 0 blocking, 1 SHOULD, 4 NIT. Confidence: VERIFIED for the code read, the case traces, the sha match, the diff scope; INFERRED for runtime results (selftest / mutants not re-run by me).
METADATA: reviewer=bf2-D0-probe-r2, date=2026-10-04
