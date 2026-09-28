# analyze.py RUNDIR [ORDER] -- per routine event: the message-thread hold (heartbeat) and its call tree (steady_clock scopes)
import re, sys, json, statistics as st
run = sys.argv[1]
order = sys.argv[2].split(',') if len(sys.argv) > 2 else ['ease'] * 5 + ['jump'] * 5 + ['norestore'] * 5 + ['loop', 'loopjump']
lines = open(run + '/app-err.log', errors='replace').read().splitlines()
DG, HB, EV = [], [], []
for ln in lines:
    m = re.match(r'\[DG\] ([\d.]+) (\d+) (\S+) ([\d.]+)$', ln)
    if m: DG.append((float(m[1]), int(m[2]), m[3], float(m[4]))); continue
    m = re.match(r'\[HB\] ([\d.]+) ([\d.]+)$', ln)
    if m: HB.append((float(m[1]), float(m[2]))); continue
    m = re.match(r'\[EV\] ([\d.]+) (\S+) (.*)$', ln)
    if m: EV.append((float(m[1]), m[2], m[3]))
DG.sort(); HB.sort()
starts = [e for e in EV if e[1] == 'startNow']
ends = [e for e in EV if e[1] == 'loopEnd']
fires = [e for e in EV if e[1] == 'fire']
events = []
for i, e in enumerate(starts):
    events.append((e[0], 'start:' + (order[i] if i < len(order) else '?'), e[2]))
# loop ends: assign by which loop start precedes them
loopstarts = [(e[0], order[i]) for i, e in enumerate(starts) if i < len(order) and order[i].startswith('loop')]
for e in ends:
    arm = None
    for t, a in loopstarts:
        if t <= e[0]: arm = a
    events.append((e[0], 'loopReturn:' + str(arm), e[2]))
for i, e in enumerate(fires):
    events.append((e[0], 'fireCall', e[2]))
events.sort()

def enclosing_top(t):
    for s in DG:
        if s[1] == 0 and s[0] <= t + 0.001 and t <= s[0] + s[3] + 0.001:
            return s
    return None
def hb_cover(t):
    # the heartbeat ping whose wait covers t (posted before t, served after t)
    best = None
    for p, lat in HB:
        if p <= t + 0.01 and p + lat >= t:
            if best is None or lat > best[1]: best = (p, lat)
    return best
rows = []
for t, kind, args in events:
    top = enclosing_top(t)
    hb = None
    if top:
        c = [(p, l) for p, l in HB if top[0] - 4.0 <= p <= top[0] + top[3]]
        hb = max(c, key=lambda x: x[1]) if c else None
    t0 = top[0] if top else t
    t1 = top[0] + top[3] if top else t
    tree = [s for s in DG if s[0] >= t0 - 0.001 and s[0] + s[3] <= t1 + 0.001 and s[1] > 0]
    # the NEXT 150 ms after the tick: heartbeat lags and paint scopes (a follow-up repaint pass)
    after_hb = [(p, l) for p, l in HB if t1 < p <= t1 + 150]
    after_paint = [s for s in DG if t1 < s[0] <= t1 + 150 and s[2].startswith('paint.')]
    rows.append(dict(t=t, kind=kind, args=args, top=top[2] if top else None, topDur=top[3] if top else None,
                     hbLat=hb[1] if hb else None, hbPost=hb[0] if hb else None,
                     tree=[(round(s[0] - t0, 3), s[1], s[2], s[3]) for s in tree],
                     afterHbMax=max([l for p, l in after_hb] or [0]), afterPaintSum=round(sum(s[3] for s in after_paint), 3)))
out = open(run + '/analysis.txt', 'w')
def pr(*a):
    print(*a); print(*a, file=out)
pr('run', run, ' events', len(rows), ' HB lags>=2ms', len(HB))
for r in rows:
    pr('%10.3f %-22s args=%-10s top=%s topDur=%s hbLat=%s after150ms: hbMax=%.1f paintSum=%.2f'
       % (r['t'], r['kind'], r['args'], r['top'], None if r['topDur'] is None else round(r['topDur'], 2),
          None if r['hbLat'] is None else round(r['hbLat'], 2), r['afterHbMax'], r['afterPaintSum']))
# per kind: hold stats + the named-call attribution (sum of each name per event)
kinds = sorted(set(r['kind'] for r in rows))
pr('\n== per kind: hold (heartbeat lat covering the event), enclosing top-level scope, attribution ==')
for k in kinds:
    rs = [r for r in rows if r['kind'] == k]
    hbs = [r['hbLat'] for r in rs if r['hbLat'] is not None]
    tops = [r['topDur'] for r in rs if r['topDur'] is not None]
    f = lambda v: 'n=%d med=%.2f min=%.2f max=%.2f' % (len(v), st.median(v), min(v), max(v)) if v else 'n=0'
    pr('%-22s hb: %s | top: %s' % (k, f(hbs), f(tops)))
    KEYS = ['eng.tick', 'eng.startNow', 'eng.loopEnd', 'mc.fire.preamble', 'mc.handleClipTrigger', 'dv.refresh',
            'ls.updateThumbnail.decodeImage', 'cc.updateThumbnail.decodeImage', 'hct.imagePreview', 'hct.sourcePreview',
            'hct.triggerClipImmediate', 'mc.manualWrite', 'mc.manualTouch', 'mc.manualRelease', 'mc.routineNotice',
            'eng.start.stepGlides', 'eng.loop.stepGlides', 'eng.start.firePreambleContinuous', 'eng.loop.firePreambleContinuous']
    for nm in KEYS:
        per = []
        for r in rs:
            v = [x for x in r['tree'] if x[2] == nm]
            per.append((sum(x[3] for x in v), len(v)))
        if any(c for _, c in per):
            sums = [x[0] for x in per]
            pr('    %-36s calls/event=%-8s sum/event med=%7.2f max=%7.2f' % (nm, sorted(set(c for _, c in per)), st.median(sums), max(sums)))
    if tops:
        dec = [sum(x[3] for x in r['tree'] if x[2].endswith('decodeImage')) for r in rs if r['topDur']]
        dvr = [sum(x[3] for x in r['tree'] if x[2] == 'dv.refresh') for r in rs if r['topDur']]
        tt = [r['topDur'] for r in rs if r['topDur']]
        pr('    ATTRIBUTION: image decodes / tick: med %.1f%% min %.1f%%;  DeckView::refresh (incl. decodes) / tick: med %.1f%% min %.1f%%'
           % (st.median([a / b * 100 for a, b in zip(dec, tt)]), min(a / b * 100 for a, b in zip(dec, tt)),
              st.median([a / b * 100 for a, b in zip(dvr, tt)]), min(a / b * 100 for a, b in zip(dvr, tt))))
json.dump(rows, open(run + '/analysis.json', 'w'), indent=1)

# ---- background: the idle window (no routine) vs +-250 ms around each event (the prior report's window) ----
drv = json.load(open(run + '/drive-log.json'))
dfires = [d['t'] for d in drv if d['what'] == 'fire']
afires = [e[0] for e in fires]
offs = sorted(a - w * 1000.0 for a, w in zip(afires, dfires))
off = offs[len(offs) // 2]
ib = [d['t'] for d in drv if d['what'] == 'idle.begin'][0] * 1000.0 + off + 500
ie = [d['t'] for d in drv if d['what'] == 'idle.end'][0] * 1000.0 + off
idle = [l for p, l in HB if ib <= p <= ie]
wmax = []
t = ib
while t + 500 <= ie:
    wmax.append(max([l for p, l in HB if t <= p < t + 500] or [0]))
    t += 500
def q(v, x):
    v = sorted(v); return v[min(len(v) - 1, int(x * len(v)))]
pr('\n== background (idle window %.1f s, no routine; wall->app offset from %d fire pairs, spread %.1f ms) ==' % ((ie - ib) / 1000, len(offs), offs[-1] - offs[0]))
pr('HB lags >= 2 ms: %d; median %.2f p90 %.2f max %.2f ms' % (len(idle), st.median(idle), q(idle, 0.9), max(idle)))
pr('per 500 ms window, the max HB lag: median %.2f p90 %.2f max %.2f ms (%d windows)' % (st.median(wmax), q(wmax, 0.9), max(wmax), len(wmax)))
pr('\n== +-250 ms around each event: the max HB lag (the prior report window) ==')
for k in kinds:
    rs = [r for r in rows if r['kind'] == k]
    v = [max([l for p, l in HB if r['t'] - 250 <= p <= r['t'] + 250] or [0]) for r in rs]
    pr('%-22s n=%d med=%.2f min=%.2f max=%.2f   %s' % (k, len(v), st.median(v), min(v), max(v), [round(x, 1) for x in v]))
