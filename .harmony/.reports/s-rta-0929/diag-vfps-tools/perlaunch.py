#!/usr/bin/env python3
"""perlaunch.py TAGDIR... -- markdown: one row per LAUNCH, each load's frame fps with its bunch pattern and the
swap-to-swap histogram summary (share of 1-vblank intervals / 2+ vblank intervals, lost display-link ticks per s)."""
import glob, json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze, bunch, q1table  # noqa: F401  (q1table.lost_ticks)
from q1table import lost_ticks
for tagdir in sys.argv[1:]:
    print(f"\n{os.path.basename(tagdir)} -- launch: load (pattern) fps [1-vblank % / >=2-vblank % of swap intervals, lost ticks/s]")
    for run in sorted(glob.glob(os.path.join(tagdir, "r*")), key=lambda p: int(os.path.basename(p)[1:])):
        if not os.path.exists(os.path.join(run, "windows.json")):
            continue
        s = analyze.summarize(run); bw = {d["tag"]: d for d in bunch.per_window(run)}; lt = lost_ticks(run)
        cells = []
        for w in s["windows"]:
            h = {int(k): v for k, v in (w.get("swap_iv_hist_vblanks") or {}).items()}; n = sum(h.values()) or 1
            one = 100 * h.get(1, 0) / n; two = 100 * sum(v for k, v in h.items() if k >= 2) / n
            pat = bw.get(w["tag"], {}).get("pattern", "-")
            cells.append(f"{w['tag'].split('_')[-1]} ({pat}) {w['fps_frames']} [{one:.0f}/{two:.0f}, {lt.get(w['tag'], 0):.1f}]")
        print(f"| {os.path.basename(run)} | " + " | ".join(cells) + " |")
