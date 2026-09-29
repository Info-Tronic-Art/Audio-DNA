#!/usr/bin/env python3
"""probe-vupload-ab.py <ab.tsv> -- the summary of .harmony/probe-vupload-ab.sh (s-rta-0929 vupload).

Input lines: "<round> <arm> DATA <row> k=v k=v ...". A = the pre-lane app, B = the lane app (by convention).
- w2c_steady_4kx4_blit (plan-vupload.md 4.9): per launch the median of its loads' median fps -> mA (A launches), mB (B).
  fps4kBlitMin = floor(min(mB)) - 3 ONLY if that is >= max(mA) + 8; else null (INFO) -- never a knife-edge.
- u7_reverse_pingpong (adoption VU7): per scene, the median over launches of uploads_per_s and late per arm; PASS iff
  B's median uploads/s >= u7UploadsRatioMin x A's and B's median late <= A's.
- every other row: per arm, per numeric key, the median over all its DATA lines (and n).
"""
import collections, json, math, os, statistics, sys

rows = collections.defaultdict(list)   # row -> [(round, arm, {k: v})]
for line in open(sys.argv[1]):
    parts = line.split()
    if len(parts) < 4 or parts[2] != "DATA":
        continue
    kv = {}
    for p in parts[4:]:
        if "=" in p:
            k, v = p.split("=", 1)
            try:
                kv[k] = float(v)
            except ValueError:
                kv[k] = v
    rows[parts[3]].append((int(parts[0]), parts[1], kv))

VU = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "probe-vupload.json")))
rc = 0


def med(xs):
    xs = [x for x in xs if isinstance(x, float)]
    return statistics.median(xs) if xs else None


for row, recs in sorted(rows.items()):
    print(f"== {row}")
    if row == "w2c_steady_4kx4_blit":
        per = collections.defaultdict(list)
        for r, arm, kv in recs:
            per[(arm, r)].append(kv.get("fps"))
        mA = sorted(round(med(v), 2) for (a, r), v in per.items() if a == "A" and med(v) is not None)
        mB = sorted(round(med(v), 2) for (a, r), v in per.items() if a == "B" and med(v) is not None)
        print(f"   launch medians A (pre-lane) {mA}")
        print(f"   launch medians B (lane)     {mB}")
        if len(mA) >= 5 and len(mB) >= 5:
            bar = math.floor(min(mB)) - 3
            if bar >= max(mA) + 8:
                print(f"   fps4kBlitMin = floor(min(B)) - 3 = {bar} (>= max(A) {max(mA)} + 8: set it)")
            else:
                print(f"   fps4kBlitMin stays null: floor(min(B)) - 3 = {bar} < max(A) {max(mA)} + 8 (a knife-edge: INFO)")
        else:
            print(f"   fewer than 5 launches per arm ({len(mA)} / {len(mB)}): no bar")
    elif row == "u7_reverse_pingpong":
        per = collections.defaultdict(lambda: collections.defaultdict(list))
        for r, arm, kv in recs:
            per[kv.get("scene")][arm].append(kv)
        ratio = float(VU["u7UploadsRatioMin"])
        for sc, arms in sorted(per.items()):
            ua = med([kv.get("uploads_per_s") for kv in arms.get("A", [])]); ub = med([kv.get("uploads_per_s") for kv in arms.get("B", [])])
            la = med([kv.get("late") for kv in arms.get("A", [])]); lb = med([kv.get("late") for kv in arms.get("B", [])])
            n = (len(arms.get("A", [])), len(arms.get("B", [])))
            good = None not in (ua, ub, la, lb) and ub >= ratio * ua and lb <= la and min(n) >= 5
            rc |= 0 if good else 1
            print(f"   {'PASS' if good else 'FAIL'}  {sc}: uploads/s A {ua} B {ub} (>= {ratio} x A: {None if ua is None else round(ratio * ua, 2)}), "
                  f"late A {la} B {lb} (B <= A), launches {n}")
    else:
        for arm in ("A", "B"):
            ks = sorted({k for r, a, kv in recs if a == arm for k, v in kv.items() if isinstance(v, float)})
            if ks:
                print(f"   {arm}: " + ", ".join(f"{k} {med([kv.get(k) for r, a, kv in recs if a == arm])}" for k in ks)
                      + f" (n {sum(1 for r, a, kv in recs if a == arm)})")
sys.exit(rc)
