# RD's SELFTEST RED: each mutant is a COPY of the committed probe with ONE replacement (applied exactly once);
# the self-test run against the copy must exit 1. The committed probe is never touched (sha256 before / after).
import hashlib, os, subprocess, sys
probe, selftest, outdir, py = sys.argv[1:5]
M = [
 ("c0-not-subtracted", '"e1": (e - c0) if e is not None', '"e1": (e) if e is not None'),
 ("o1-without-a-shifted-take", '    if shifted:\n        return done("O1"', '    if True:\n        return done("O1"'),
 ("spread-clause-removed", 'if t["valid"] and t["spread"] > RD_SPREAD:', 'if False:'),
 ("c-ignored", '            if c > RD_ONE:\n', '            if False:\n'),
 ("block-multiple-clause-removed", 'if t["valid"] and (t["c0"] < 0 or t["c0"] % t["block"] != 0):', 'if False:'),
 ("mid-span-clause-removed", '    if RD_ONE < s < RD_TWO:\n', '    if False:\n'),
 ("eref-clause-removed", '        if shifted and abs(eref - RD_E1_NOMINAL) > RD_ONE:', '        if False:'),
 ("device-event-ignored", '        t["deviceEvent"] = bool((t.get("opensDelta") or 0) > 0 or', '        t["deviceEvent"] = bool(False and'),
 ("gap-validity-removed", '        elif t["gaps"]:\n', '        elif False:\n'),
 ("dial-validity-removed", '        if not t.get("settled") or t.get("hopsApplied"):', '        if False:'),
 ("o2-threshold-gone", 'RD_TWO = 384 ', 'RD_TWO = 10**9 '),
 ("min-valid-gone", 'RD_MIN_VALID = 24 ', 'RD_MIN_VALID = 0 '),
 ("stop-rule-gone", '    if len(valid) < RD_MIN_VALID and not (shifted and met):', '    if len(valid) < RD_MIN_VALID and not shifted:'),
 ("c-missing-rule-gone", '        if v and missing * 2 > len(v):', '        if False:'),
 ("click-threshold-an-eighth", '    thr = full_scale / 2.0\n', '    thr = full_scale / 16.0\n'),
 # ---- review r1 (one per new branch)
 ("r1-dev-take-c0-against-its-own-block", '                elif t["c0"] != 0:\n', '                elif t.get("block") and t["c0"] >= t["block"]:\n'),
 ("r1-dev-take-unreadable-asset-passes", 'how = "its asset could not be read (c0 unknown), so it cannot be shown unshifted"', 'how = None'),
 ("r1-dev-take-e1-far-branch-removed", '                elif vm is not None and t.get("e1") is not None and abs(t["e1"] - vm) >= RD_TWO:', '                elif False:'),
 ("r1-o1-without-c-on-a-shifted-take", '    if blind:   #', '    if False:   #'),
 ("r1-eref-warn-removed", '    if out["erefFar"]:\n', '    if False:\n'),
 ("r1-no-gap-field-read-as-valid", '        if t.get("gaps") is None:\n            why.append("no gap field")\n        elif t["gaps"]:', '        if t.get("gaps"):'),
 ("r1-no-applied-field-read-as-valid", '        if t.get("hopsApplied") is None:', '        if False:'),
 ("r1-record-absent-gaps-as-0", '"gaps": len(take["audio"]["gaps"]) if isinstance(take.get("audio", {}).get("gaps"), list) else None,', '"gaps": len(take.get("audio", {}).get("gaps", []) or []),'),
 ("r1-record-absent-applied-as-0", '"hopsApplied": None if any("appliedMs" not in h for h in hops)\n            else sum(1 for h in hops if h["appliedMs"] != 0)', '"hopsApplied": sum(1 for h in hops if h.get("appliedMs", 0) != 0)'),
 ("witness-offset-fixed-at-0", '    for d in sorted(range(-reach, reach + 1), key=lambda x: (abs(x), x < 0)):', '    for d in [0]:'),
]
src = open(probe).read()
sha0 = hashlib.sha256(src.encode()).hexdigest()
bad = 0
for name, old, new in M:
    n = src.count(old)
    if n != 1:
        print("MUTANT %-32s NOT APPLIED (%d matches)" % (name, n)); bad += 1; continue
    p = os.path.join(outdir, "mut-%s.py" % name)
    open(p, "w").write(src.replace(old, new))
    r = subprocess.run([py, selftest], env=dict(os.environ, PROBESYNC_SELFTEST_TARGET=p), capture_output=True, text=True)
    open(os.path.join(outdir, "selftest-mut-%s.log" % name), "w").write(r.stdout + r.stderr)
    last = r.stdout.strip().splitlines()[-1] if r.stdout.strip() else r.stderr.strip().splitlines()[-1]
    diffs = [l[6:70] for l in r.stdout.splitlines() if l.startswith("DIFF")]
    print("MUTANT %-32s exit %d  %s" % (name, r.returncode, "RED" if r.returncode == 1 else "NOT RED"))
    for d in diffs: print("        DIFF " + d)
    bad += r.returncode != 1
print("probe sha256 before %s after %s" % (sha0, hashlib.sha256(open(probe).read().encode()).hexdigest()))
print("mutants not RED: %d of %d" % (bad, len(M)))
sys.exit(1 if bad else 0)
