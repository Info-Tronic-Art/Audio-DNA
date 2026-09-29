#!/usr/bin/env python3
"""heavyphase.py TAGDIR -- for every window: fps, max uploads in one frame, and for the frames carrying the most
uploads: their start phase in the vblank grid (ms after a display-link outputTime grid point), their CPU cost, the
flushBuffer block that follows, and the ticks lost (display-link ticks with no frame start before the next tick)."""
import glob, json, os, statistics, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze
for run in sorted(glob.glob(os.path.join(sys.argv[1], "r*")), key=lambda p: int(os.path.basename(p)[1:])):
    f, recs, th, disp, cal, hdr = analyze.load(run)
    up = {r[2]: int(r[3]) for r in recs[11]}
    for w in json.load(open(os.path.join(run, "windows.json"))):
        a, b = w["t0"] / f, w["t1"] / f
        fr = [r for r in recs[10] if a <= r[2] < b]
        dl = [r for r in recs[1] if a <= r[2] < b]
        sw = [r for r in recs[2] if a <= r[2] < b]
        if not fr or not dl:
            continue
        P = 8.3333; g0 = dl[0][4]
        n = [up.get(r[2], 0) for r in fr]; mx = max(n)
        hv = [r for r, k in zip(fr, n) if k == mx and mx > 0]
        ph = [((r[2] - g0) * f / 1e6) % P for r in hv]
        cost = [(r[3] - r[2]) * f / 1e6 for r in hv]
        # lost ticks: DL callbacks with no frame start between it and the next callback
        starts = [r[2] for r in fr]; j = 0; lost = 0
        for k in range(len(dl) - 1):
            while j < len(starts) and starts[j] < dl[k][2]:
                j += 1
            if not (j < len(starts) and starts[j] < dl[k + 1][2]):
                lost += 1
        span = (b - a) * f / 1e9
        print(f"{os.path.basename(run):>4} {w['tag']:<12} fps {len(fr)/span:6.1f} maxUp {mx} heavy n {len(hv):3d} "
              f"phase p10/50/90 {statistics.quantiles(ph, n=10)[0]:5.2f}/{statistics.median(ph):5.2f}/{statistics.quantiles(ph, n=10)[-1]:5.2f} "
              f"cost p50 {statistics.median(cost):5.2f} lost ticks {lost} ({lost/span:5.1f}/s)" if len(hv) > 2 else
              f"{os.path.basename(run):>4} {w['tag']:<12} fps {len(fr)/span:6.1f} maxUp {mx} lost ticks {lost}")
