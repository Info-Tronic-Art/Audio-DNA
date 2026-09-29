# lagclass.py RUNDIR... -- the attribution table: each display-cycle phase (BeforeTimers->BeforeSources, containing one
# JUCE drawRect) is classed by its union rect (full window / body below the top bar / top bar only / other); per class:
# events/s, phase ms per event (median/p90/max), busy ms/s, share of main busy and of heartbeat lag (lags >= 2 ms).
# Non-display phases are classed by run-loop phase.
import sys, json, bisect, collections, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, timeline, q
def cls(rc):
    if rc is None: return 'display:metal-or-unknown'
    x, y, w, h = rc
    if h > 900 and y <= 10: return 'display:FULL window (top bar + body)'
    if h > 900: return 'display:BODY (4,39,1720,978)'
    if y <= 10 and h <= 40: return 'display:TOP BAR only'
    return 'display:other rect %s' % (rc,)
ev_ms = collections.defaultdict(list); busy = collections.defaultdict(float); lag = collections.defaultdict(float)
tot_lag = 0.0; tot_busy = 0.0; tot_s = 0.0
for run in sys.argv[1:]:
    R, C, L, M = load(run)
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']; tot_s += (e - b) / 1000
    rect = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
    dr = sorted((t, d) for k, thr, t, d, x, n in R if k in (5, 16) and thr == 0 and b - 50 <= t <= e)
    dt = [z[0] for z in dr]
    iv = []
    for t0, t1, l in timeline(R, b, e):
        if l == 'timers-obs':
            i = bisect.bisect_left(dt, t0)
            inside = [z for z in dr[i:i + 4] if z[0] < t1]
            if inside:
                c = cls(rect.get(round(inside[0][0], 3)))
                ev_ms[c].append(t1 - t0)
            else:
                c = 'display:no drawRect'
            iv.append((t0, t1, c))
        elif l != 'sleep':
            iv.append((t0, t1, 'runloop:' + l))
    for t0, t1, c in iv: busy[c] += t1 - t0; tot_busy += t1 - t0
    it = [z[0] for z in iv]
    for k, thr, p, d, x, n in R:
        if k != 40 or not (b <= p <= e) or d < 2.0: continue
        s_ = p + d; tot_lag += d
        i = max(0, bisect.bisect_right(it, p) - 1)
        while i < len(iv) and iv[i][0] < s_:
            ov = min(iv[i][1], s_) - max(iv[i][0], p)
            if ov > 0: lag[iv[i][2]] += ov
            i += 1
print('%-44s %8s %24s %9s %8s %8s' % ('class', 'events/s', 'ms/event med/p90/max', 'busy ms/s', 'busy %', 'lag %'))
for c in sorted(busy, key=lambda c: -busy[c]):
    v = ev_ms.get(c, [])
    evs = '%.2f' % (len(v) / tot_s) if v else '-'
    msv = '%.2f/%.2f/%.2f' % (st.median(v), q(v, .9), max(v)) if v else '-'
    print('%-44s %8s %24s %9.1f %8.1f %8.1f' % (c[:44], evs, msv, busy[c] / tot_s, 100 * busy[c] / tot_busy, 100 * lag[c] / tot_lag if tot_lag else 0))
print('TOTAL busy %.1f ms/s ; heartbeat lag (>=2 ms) %.1f ms/s ; unaccounted lag %.1f%%' % (tot_busy / tot_s, tot_lag / tot_s, 100 * (tot_lag - sum(lag.values())) / tot_lag if tot_lag else 0))
