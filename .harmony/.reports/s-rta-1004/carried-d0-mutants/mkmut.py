import sys
src, dst = sys.argv[1:3]
s = open(src).read()
def rep(old, new):
    global s
    assert s.count(old) == 1, (s.count(old), old)
    s = s.replace(old, new)
# the gap-validity anchor moved with review r1 S3
rep(''' ("gap-validity-removed", '        if t.get("gaps"):\\n', '        if False:\\n'),''',
    ''' ("gap-validity-removed", '        elif t["gaps"]:\\n', '        elif False:\\n'),''')
rep(''' ("witness-offset-fixed-at-0",''',
''' # ---- review r1 (one per new branch)
 ("r1-dev-take-c0-against-its-own-block", '                elif t["c0"] != 0:\\n', '                elif t.get("block") and t["c0"] >= t["block"]:\\n'),
 ("r1-dev-take-unreadable-asset-passes", 'how = "its asset could not be read (c0 unknown), so it cannot be shown unshifted"', 'how = None'),
 ("r1-dev-take-e1-far-branch-removed", '                elif vm is not None and t.get("e1") is not None and abs(t["e1"] - vm) >= RD_TWO:', '                elif False:'),
 ("r1-o1-without-c-on-a-shifted-take", '    if blind:   #', '    if False:   #'),
 ("r1-eref-warn-removed", '    if out["erefFar"]:\\n', '    if False:\\n'),
 ("r1-no-gap-field-read-as-valid", '        if t.get("gaps") is None:\\n            why.append("no gap field")\\n        elif t["gaps"]:', '        if t.get("gaps"):'),
 ("r1-no-applied-field-read-as-valid", '        if t.get("hopsApplied") is None:', '        if False:'),
 ("r1-record-absent-gaps-as-0", '"gaps": len(take["audio"]["gaps"]) if isinstance(take.get("audio", {}).get("gaps"), list) else None,', '"gaps": len(take.get("audio", {}).get("gaps", []) or []),'),
 ("r1-record-absent-applied-as-0", '"hopsApplied": None if any("appliedMs" not in h for h in hops)\\n            else sum(1 for h in hops if h["appliedMs"] != 0)', '"hopsApplied": sum(1 for h in hops if h.get("appliedMs", 0) != 0)'),
 ("witness-offset-fixed-at-0",''')
open(dst, "w").write(s)
