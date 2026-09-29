#!/usr/bin/env python3
"""q1table.py TAGDIR... -- one row per arm: launches, windows, bunch-pattern counts, frame fps median [min, max],
windows >= 117 / >= 110, lost display-link ticks per s (median), flushBuffer p99 (median over windows), GL wake-up
latency p99 (median), GL frames on E-cores (median fraction). Also writes TAGDIR/q1.json (per-window rows)."""
import glob, json, os, statistics, sys
from collections import Counter
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze, bunch


def lost_ticks(run):
    f, recs, *_ = analyze.load(run)
    out = {}
    for w in json.load(open(os.path.join(run, "windows.json"))):
        a, b = w["t0"] / f, w["t1"] / f
        fr = sorted(r[2] for r in recs[10] if a <= r[2] < b)
        dl = sorted(r[2] for r in recs[1] if a <= r[2] < b)
        j = 0; lost = 0
        for k in range(len(dl) - 1):
            while j < len(fr) and fr[j] < dl[k]:
                j += 1
            if not (j < len(fr) and fr[j] < dl[k + 1]):
                lost += 1
        out[w["tag"]] = lost / ((b - a) * f / 1e9)
    return out


def main():
    for tagdir in sys.argv[1:]:
        rows = []
        runs = sorted(glob.glob(os.path.join(tagdir, "r*")), key=lambda p: int(os.path.basename(p)[1:]))
        for run in runs:
            if not os.path.exists(os.path.join(run, "windows.json")):
                continue
            s = analyze.summarize(run); bw = {d["tag"]: d for d in bunch.per_window(run)}; lt = lost_ticks(run)
            for w in s["windows"]:
                if not w["tag"].startswith("w1"):
                    continue
                b = bw[w["tag"]]
                rows.append({"run": os.path.basename(run), "tag": w["tag"], "fps": w["fps_frames"], "poll": w["fps_poll_median"],
                             "pattern": b["pattern"], "lost_per_s": round(lt[w["tag"]], 1), "flush_p99": (w.get("flush_ms") or [None] * 3)[2],
                             "wake_p99": (w.get("wake_latency_ms") or [None] * 3)[2], "ecore": w.get("gl_on_ecore_frac"),
                             "render_p90": (w.get("render_ms") or [None] * 3)[1], "load": w["load"],
                             "gl": [r for r in w.get("threads_top", []) if r[1].startswith("OpenGL")][:1],
                             "dec": [r[2] for r in w.get("threads_top", []) if r[1].startswith("VideoDecode")][:1]})
        json.dump(rows, open(os.path.join(tagdir, "q1.json"), "w"), indent=1)
        if not rows:
            print(os.path.basename(tagdir), "no rows"); continue
        fps = sorted(r["fps"] for r in rows)
        med = lambda k: statistics.median([r[k] for r in rows if r[k] is not None]) if any(r[k] is not None for r in rows) else None
        print(f"{os.path.basename(tagdir):<14} launches {len(set(r['run'] for r in rows)):>2} windows {len(rows):>2} "
              f"patterns {dict(Counter(r['pattern'] for r in rows))} fps median {statistics.median(fps):6.1f} [{fps[0]}, {fps[-1]}] "
              f">=117 {sum(1 for x in fps if x >= 117)} >=110 {sum(1 for x in fps if x >= 110)} lost/s {med('lost_per_s')} "
              f"flush p99 {med('flush_p99')} wake p99 {med('wake_p99')} E {med('ecore')} render p90 {med('render_p90')} "
              f"load {min(r['load'] for r in rows)}-{max(r['load'] for r in rows)} GL {rows[0]['gl'][0][2] if rows[0]['gl'] else '?'} dec {rows[0]['dec']}")


if __name__ == "__main__":
    main()
