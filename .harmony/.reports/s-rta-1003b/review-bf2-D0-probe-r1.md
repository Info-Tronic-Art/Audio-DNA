# Reviewer Verdict -- bf2 D0 probe r1 (lane/bf2 a852465, base 68abc16)
STATUS: PARTIAL
VERDICT: FAIL (REQUEST_CHANGES)
FILES: .harmony/probe-sync.py, .harmony/probe-sync.sh, .harmony/probe-sync-selftest.py (bf2-d0.md read). Only these 4 paths changed; no src/, tests/ (VERIFIED, diff --stat).
Method: git objects only; pure-function repros of rd_verdict on synthetic records in a $TMPDIR copy (no app, no selftest, no probe run).

## MUST
M1 A device-event take that is shifted by anything but "c0 >= its own post-take block" escapes O4 and the outcome reads O5.
   probe-sync.py:1502-1505 (clause) + rd_classify:1418 (device-event take is dropped from `valid`). VERIFIED by repro:
   (a) device-event take, c0 = 512, post-take block 1024 (the file started one OLD block in, the buffer then grew) -> "RD outcome O5 (26 valid takes ... EREF 1472)", no O4 clause.
   (d) device-event take whose asset is unreadable (c0 None) and whose e is one block off -> O5. Only c0 >= block, or e1 >= 384 from the median, is tested.
   Ruling O4: "a shifted take that is also a device-event take"; lens: a device-event take must never end in O1/O5. B of a device-event take is exactly the unreliable quantity.
   Fix: flag any device-event take with c0 != 0, c0 None (asset unreadable), or e (not only e') >= 384 from the valid takes' median e, plus a selftest case for each (c0 512 under block 1024; c0 None).

## SHOULD
S1 O1 never requires C on the shifted takes (probe-sync.py:1534, :1542). Repro: 2 shifted takes with C None, 4 normal -> "O1 ... C span 0" (missing in 2 of 6 = not > half, and rd_span ignores None). Ruling K3: "O1 needs both rulers to agree". Fix: INCOMPLETE (or O4) when any c0 >= B take has C None.
S2 O5 blesses EREF with no nominal check (probe-sync.py:1517 is inside `if shifted`). Repro: 24 takes, c0 = 0, every e' = 1984 -> "O5 ... EREF 1984". A state present in every take (the case A6 says dbar cannot see) becomes R7r's reference. Ruling-compliant, but cheap to close: an INFO/WARN line (or O4) when EREF is > 128 from 1472 in O5.
S3 Missing fields read as 0: rd_classify:1426/:1428 `t.get("hopsApplied")`, `t.get("gaps")`, and rd_take_record:1391 `h.get("appliedMs", 0)`. Repro: a record with gaps and hopsApplied keys deleted -> valid True. Unreachable from probe-written files (row_rd always sets them; Take.cpp:65 always writes "gaps") but the verdict is offline over files. Fix: a missing key -> not valid ("no gap field").

## NIT
N1 Selftest holes: O4's device-event clause only through the "opens" branch (probe-sync-selftest.py:322); no case for the e1-far branch, a rate-only difference, `settled` False, "no device fields", or a take at exactly 20 clicks. No mutant for `c0 < 0` alone, the rate/block device-event branches.
N2 The committed probe was not launched (report caveat 1: the live-dev run used sha 657fa86d; committed 8ed83066 adds the witness "diffs" list). Harmony's first RD launch is its first live run.
N3 PROBESYNC_RD=1 without R7 in the rows runs nothing and prints nothing (main, :1706).
N4 click_onsets collapses chain-wise (`last = i` on every hit, :1319), the ruling says "to the first"; identical on the real asset (spread 0, 24 clicks).
N5 row_rd duplicates row_r7's take loop (accepted in the report; fold in R7r).

## CHECKS (item by item)
1 Rulers/fields/table: VERIFIED. Take and launch lines match the ruling's strings; c0, e, e', C (median stamp - rising-hop timestamp, offset by min SD), opens/block/rate/gaps/hopsApplied present. Table order O4, O2, INCOMPLETE tests, O1/O5; every s lands in exactly one row (<=128 one, 128<s<384 O4, >=384 O2). All 6 O4 clauses present (1496-1527). Early-stop reading S3(a) is consistent with A3/HD9.
2 Product-as-artefact: two-valued e' -> O2/O4 (never O1/O5); rulers disagree -> O4 (repro: C slip 24000 -> O4, a false STOP never a false pass); device-event -> see M1.
3 Selftest: 57 cases (27 + 30, counted), outcomes O1 x3, O2 x3, O5, INCOMPLETE x4, O4 x7 + order, validity x3, exits. 0 valid takes -> INCOMPLETE (no outcome) VERIFIED. Mutant log in SP has 16 `MUTANT ... exit 1 RED` lines (read); I did not re-run them. RED at 68abc16 is an AttributeError (absence only; INFERRED from report).
4 Default path: VERIFIED -- diff old/new shows exactly one removed line (`if "R7" in rows:` -> `if ... PROBESYNC_RD == "1": row_rd ... elif`), the rest additions; probe-sync.sh diff is comment lines only; the 27 old cases are untouched.
5 Scope: src/, tests/ unchanged; the script launch/quit code is not touched (sh unchanged but comments); cleanup rmtree is limited to RUNID-globbed takes and 32-hex asset ids; R6 sha lines present in the dev launch; 0 Output windows; no stray file, no .venv link in the diff. No on-screen text (probe only).
