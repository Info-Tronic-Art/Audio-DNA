# Reviewer Verdict -- bf10 gates r2
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS; 0 blocking, 0 SHOULD, 3 NIT)
PINNED: worktree .claude/worktrees/bf10, lane/bf10, fix-round base e3c73a2, head e04a609185fb8c076b93ff3dee2c6e8f87cdcc55
SCOPE: git diff e3c73a2..e04a609 -- code is only .harmony/probe-milkdrop.sh / .py / -selftest.sh (new), probe-vupload-ab.sh (+1 line) / -selftest.sh (new), docs/claude/rendering.md (1 line); src/ tests/ cmake untouched (empty diff stat).

VERIFIED (I executed these at the pinned blobs, in a $TMPDIR copy; the live-lock gate path was seded to a scratch owner file, nothing under /tmp/audiodna-live.lock touched, no app launched, no Audio-DNA running)
- R1 refusal is real: .harmony/probe-milkdrop.sh:56-70. ATTACH requires WANTPID (MILKDROP_ATTACH_PID, else the LOCK_LIB + LANE start_app record), RUNPID == WANTPID (two apps concatenate and never match), the 8080 listener pid == RUNPID, and a non-empty 8080 /api/health; else prints the exact ruled REFUSE line and exit 2 (:67-69). The checks short-circuit, so a1-a4, a6-a8 made 0 curl calls and the probe body (probe-milkdrop.py, where every 7070/8080 request lives) never ran; a5 (our pid + listener, health silent) makes exactly 1 /api/health ask and refuses. Fail-closed on missing lsof / empty listener. I ran probe-milkdrop-selftest.sh: SELFTEST 51 ok / 0 FAIL.
- R2: .sh:49-52 and .py:82-86 both refuse MILKDROP_P1 unless MILKDROP_MODE=pre (exit 2 before any request; lane default and explicit lane both refused: b1/b2/c1); .py:92 prints "P1 = <path>" as the FIRST line of every run, marked OVERRIDE in pre mode (c2/c3); empty MILKDROP_P1 = no override (c4).
- R3: probe-vupload-ab.sh:49 `rm -f "$LOG.done"` runs inside the retry loop before the sampler, every attempt. probe-vupload-ab-selftest.sh: 6 ok / 0 FAIL; attempt 2 (the re-run) .compilers = 3 lines, 2 compiler samples; both r1 A attempts report "compilers seen 1"; "tainted 2". probe-vupload-ab.py --selftest: SELFTEST PASS (226 ok lines; needs probe-vupload.json beside it, present in the tree).
- Tests can fail (my own mutants, run in the copy): R3 = delete the rm line -> SELFTEST 1 ok / 5 FAIL (attempt 2 .compilers empty, ab.tsv wrong). R1 = replace the pid + listener comparison with `false` -> a2/a3/a4/a6/... FAIL (exit 0, body ran, curl calls 1). The lane report also carries M1-M4 mutants (r12-M1..M4.log) and the e3c73a2 RED run (15 ok / 36 FAIL; r3-red 1 ok / 5 FAIL): consistent with what I reproduced.
- Round-1 items: SHOULD-1/2/3 fixed as ruled, not documented away (code + tests above). NIT-1 (MILKDROP_APP vs VIDEO_APP differ -> refuse 64) at .sh:38 fixed; NIT-4 (dead CB) removed at .py; gl NIT-1 rendering.md alpha wording fixed. Round-1 had no MUST. G3 stays INFO as ruled (lane report does not claim it closed).
- No new defect found: the R3 change cannot race (the previous sampler is `wait`ed before the next attempt); the R1 subshell source of LOCK_LIB cannot leak state; R2's early .sh refuse sits after the lock gate and before any launch/attach.

FINDINGS
NIT-1 .harmony/probe-milkdrop-selftest.sh:20 hard-requires the live lock and the selftests are mode 100644 (siblings are 100755): run via `bash`, fine; note for the README line only.
NIT-2 .harmony/probe-milkdrop.sh:62 a stale .ours-pid-<lane> record whose pid is later reused would still pass the pid check; the 8080-listener + /api/health requirements still gate it (test-mode only), so no action.
NIT-3 .harmony/APP-INVENTORY.md "1114 unit tests" vs ctest 1130 (r1 NIT-5) still open for Harmony at merge.

CONFIDENCE: VERIFIED (executed the selftests and two mutants at the pinned blobs). INFERRED: behaviour against the real lock helper beyond the lane's batch2.log (I did not have the helper and launched no app).
METADATA: reviewer=reviewer, slug=bf10-gates-r2, date=2026-10-02
