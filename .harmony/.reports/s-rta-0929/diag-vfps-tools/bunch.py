#!/usr/bin/env python3
"""bunch.py TAGDIR... -- per window: frames/s, the upload-bunch pattern (render frames with 1/2/3/4 video uploads),
the dominant bunch ("4" = the 4 players' new frames land on ONE render frame), the cost of the heaviest frames and the
vblank misses that follow a bunched frame. Prints the fps grouped by dominant bunch pattern."""
import glob, json, os, statistics, sys
from collections import Counter, defaultdict
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze


def pattern(c):
    # the sorted multiset of per-content-frame bunch sizes, e.g. "4", "3+1", "2+2", "2+1+1", "1+1+1+1"
    tot = sum(k * v for k, v in c.items() if k > 0)
    if tot == 0:
        return "none"
    shares = {k: k * v / tot for k, v in c.items() if k > 0}
    dom = sorted(((k, s) for k, s in shares.items() if s >= 0.15), key=lambda x: -x[0])
    return "+".join(f"{k}" for k, s in dom)


def per_window(run):
    f, recs, th, disp, cal, hdr = analyze.load(run)
    wins = json.load(open(os.path.join(run, "windows.json")))
    out = []
    up = {r[2]: r for r in recs[11]}
    fr_all = recs[10]
    sw = recs[2]
    for w in wins:
        a, b = w["t0"] / f, w["t1"] / f
        fr = [r for r in fr_all if a <= r[2] < b]
        n = [int(up[r[2]][3]) if r[2] in up else 0 for r in fr]
        c = Counter(n)
        # vblank misses: swap-to-swap > 1.5 vblank, attributed to the frame just before
        ends = [r[3] for r in sw if a <= r[2] < b]
        cost = [(r[3] - r[2]) * f / 1e6 for r in fr]
        heavy = [x for x, k in zip(cost, n) if k == max(n)] if n else []
        span = (b - a) * f / 1e9
        out.append({"run": os.path.basename(run), "tag": w["tag"], "fps": round(len(fr) / span, 1), "uploads_hist": dict(sorted(c.items())),
                    "pattern": pattern(c), "max_bunch": max(n) if n else 0,
                    "heavy_ms_p50": round(statistics.median(heavy), 2) if heavy else None, "load": round(w["load"][0], 2)})
    return out


if __name__ == "__main__":
    for tagdir in sys.argv[1:]:
        by = defaultdict(list)
        print("===", tagdir)
        for run in sorted(glob.glob(os.path.join(tagdir, "r*")), key=lambda p: int(os.path.basename(p)[1:])):
            if not os.path.exists(os.path.join(run, "windows.json")):
                continue
            for d in per_window(run):
                print(f"  {d['run']:>4} {d['tag']:<11} fps {d['fps']:>6}  uploads/frame {d['uploads_hist']}  pattern {d['pattern']:<8} "
                      f"heavy frame {d['heavy_ms_p50']} ms  load {d['load']}")
                by[d["pattern"]].append(d["fps"])
        for k in sorted(by, key=lambda x: -len(x)):
            v = sorted(by[k])
            print(f"  pattern {k:<8}: n={len(v):>2} fps median {statistics.median(v):6.1f} range [{v[0]}, {v[-1]}] {v}")
