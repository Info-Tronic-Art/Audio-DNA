#!/usr/bin/env python3
"""timeline.py RUNDIR WINDOW_INDEX [N] [OFFSET] -- merged per-event timeline (ms from the window start) of display-link
ticks (DL, with the vsync host time), frame start/end (+ uploads, split), flushBuffer, render-thread waits."""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import analyze
run, wi = sys.argv[1], int(sys.argv[2]); N = int(sys.argv[3]) if len(sys.argv) > 3 else 60; OFF = float(sys.argv[4]) if len(sys.argv) > 4 else 1000.0
f, recs, th, disp, cal, hdr = analyze.load(run)
w = json.load(open(os.path.join(run, "windows.json")))[wi]
T0 = w["t0"] / f + OFF * 1e6 / f
ms = lambda t: (t - T0) * f / 1e6
ev = []
for r in recs[1]: ev.append((r[2], f"DL   cb {ms(r[2]):9.3f}  vsync(inNow) {ms(r[3]):9.3f}  out {ms(r[4]):9.3f}"))
for r in recs[4]: ev.append((r[2], f"DLl  matched {int(r[3])}"))
up = {r[2]: r for r in recs[11]}; ph = {r[2]: r for r in recs[12]}
for r in recs[10]:
    u = up.get(r[2]); p = ph.get(r[2])
    ev.append((r[2], f"FRAME start cpu{int(r[4])}  dur {ms(r[3]) - ms(r[2]):6.3f}  uploads {int(u[3]) if u else '?'} upms {u[4]*f/1e6 if u else 0:6.3f}"
                     + (f"  comp {(p[4]-p[3])*f/1e6:6.3f}" if p and p[4] > 0 and p[3] > 0 else "")))
for r in recs[2]: ev.append((r[2], f"FLUSH {ms(r[2]):9.3f} -> {ms(r[3]):9.3f} ({(r[3]-r[2])*f/1e6:6.3f} ms) juceFrameTime {int(r[4])}"))
for r in recs[3]: ev.append((r[2], f"WAIT  {ms(r[2]):9.3f} -> {ms(r[3]):9.3f} ({(r[3]-r[2])*f/1e6:6.3f}) req {int(r[4])}"))
if "--dec" in sys.argv:
    for r in recs[21]: ev.append((r[2], f"   dec[{int(r[5]) % 1000:3d}] cpu{r[1]} decodeWait {r[3]*f/1e6:6.3f} then post {r[4]*f/1e6:6.3f}"))
    for r in recs[22]: ev.append((r[2], f"   sws[{int(r[5]) % 1000:3d}] cpu{r[1]} {r[3]*f/1e6:6.3f} (ring wait before {r[4]*f/1e6:6.3f})"))
ev.sort()
n = 0
for t, s in ev:
    if t < T0: continue
    print(f"{ms(t):9.3f}  {s}"); n += 1
    if n >= N: break
