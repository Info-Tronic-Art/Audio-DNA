#!/usr/bin/env python3
"""lostpos.py TAGDIR [pattern] -- where the lost display-link ticks fall relative to the last frame that carried video
uploads (ms after that frame's start, 2 ms bins), over the windows of the given bunch pattern (default 4)."""
import glob, json, os, sys, bisect
from collections import Counter
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze, bunch
want = sys.argv[2] if len(sys.argv) > 2 else "4"
h = Counter(); nlost = 0; nwin = 0; burst = Counter()
for run in sorted(glob.glob(os.path.join(sys.argv[1], "r*"))):
    if not os.path.exists(os.path.join(run, "windows.json")):
        continue
    f, recs, *_ = analyze.load(run)
    up = {r[2]: int(r[3]) for r in recs[11]}
    pats = {d["tag"]: d["pattern"] for d in bunch.per_window(run)}
    for w in json.load(open(os.path.join(run, "windows.json"))):
        if pats.get(w["tag"]) != want:
            continue
        nwin += 1
        a, b = w["t0"] / f, w["t1"] / f
        fr = sorted(r[2] for r in recs[10] if a <= r[2] < b)
        hv = sorted(r[2] for r in recs[10] if a <= r[2] < b and up.get(r[2], 0) > 0)
        dl = sorted(r[2] for r in recs[1] if a <= r[2] < b)
        sws = sorted(r[2] for r in recs[22] if a <= r[2] < b)
        j = 0
        for k in range(len(dl) - 1):
            while j < len(fr) and fr[j] < dl[k]:
                j += 1
            if not (j < len(fr) and fr[j] < dl[k + 1]):
                i = bisect.bisect_right(hv, dl[k]) - 1
                if i >= 0:
                    h[int((dl[k] - hv[i]) * f / 1e6 // 2) * 2] += 1; nlost += 1
        for t in sws:
            i = bisect.bisect_right(hv, t) - 1
            if i >= 0:
                burst[int((t - hv[i]) * f / 1e6 // 2) * 2] += 1
print(f"{os.path.basename(sys.argv[1])} pattern {want}: windows {nwin}, lost ticks {nlost}")
print("  lost tick, ms after the upload frame's start:", dict(sorted(h.items())))
print("  decoder sws start, ms after the upload frame's start:", dict(sorted(burst.items())))
