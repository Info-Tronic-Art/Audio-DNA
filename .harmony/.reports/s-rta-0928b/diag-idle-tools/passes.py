# passes.py RUNDIR -- per display pass (a drawRect of the main window) in the idle window: duration, the union rect,
# the exclusive paint time by component type, and the repaint() sources since the previous pass.
import sys, json, collections, statistics as st, bisect
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, demangle, short, q, timeline

def passes(run, show=12):
    R, C, L, M = load(run)
    idl = json.load(open(run + '/idle.json'))
    evs = {x['what']: x['t'] for x in idl['events']}
    b, e = evs['idle.begin'], evs['idle.end']
    dur_s = (e - b) / 1000
    demangle(set(r[5] for r in R))
    main = [r for r in R if r[1] == 0 and b - 200 <= r[2] <= e]
    dr = [(t, d, x) for k, thr, t, d, x, n in main if k == 5]
    rects = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in main if k == 9}
    paints = sorted((t, d, n, x) for k, thr, t, d, x, n in main if k == 4)
    pt = [p[0] for p in paints]
    reps = sorted((t, n, x) for k, thr, t, d, x, n in main if k == 8)
    rt = [r[0] for r in reps]
    iv = timeline(R, b, e)
    # the display-pass interval (phase containing the drawRect)
    ivt = [z[0] for z in iv]
    out = []
    prev = b
    rows = []
    comp_tot = collections.Counter()
    src_tot = collections.Counter(); src_area = collections.Counter()
    for t, d, area in dr:
        if t < b: prev = t; continue
        i0 = bisect.bisect_left(pt, t); i1 = bisect.bisect_right(pt, t + d)
        pc = collections.Counter()
        for pp in paints[i0:i1]: pc[short(pp[2])] += pp[1]
        j0 = bisect.bisect_left(rt, prev); j1 = bisect.bisect_left(rt, t)
        sc = collections.Counter(); sa = collections.Counter()
        for rr in reps[j0:j1]: sc[short(rr[1])] += 1; sa[short(rr[1])] += rr[2]
        k = max(0, bisect.bisect_right(ivt, t) - 1)
        phase = iv[k] if iv else None
        rect = rects.get(round(t, 3))
        rows.append(dict(t=round(t - b, 1), dur=round(d, 2), rect=rect, phaseLen=round(phase[1] - phase[0], 2) if phase else None,
                         phase=phase[2] if phase else None, paint=[(a, round(v, 2)) for a, v in pc.most_common(6)],
                         sources=[(a, c) for a, c in sc.most_common(8)]))
        for a, v in pc.items(): comp_tot[a] += v
        for a, c in sc.items(): src_tot[a] += c
        prev = t
    o = []
    o.append('== passes %s: %d drawRects in %.1f s (%.1f/s)' % (run, len(rows), dur_s, len(rows) / dur_s))
    ds = [r_['dur'] for r_ in rows]
    o.append('drawRect dur med %.2f p90 %.2f max %.2f; sum %.1f ms/s' % (st.median(ds), q(ds, .9), max(ds), sum(ds) / dur_s))
    # rect classes
    rc = collections.Counter(r_['rect'] for r_ in rows)
    o.append('union rects (x,y,w,h) most common: %s' % [(k_, c_) for k_, c_ in rc.most_common(8)])
    o.append('paint ms/s by component type: %s' % [(a, round(v / dur_s, 2)) for a, v in comp_tot.most_common(14)])
    o.append('repaint() calls/s by source type: %s' % [(a, round(c_ / dur_s, 1)) for a, c_ in src_tot.most_common(25)])
    big = sorted(rows, key=lambda r_: -r_['dur'])[:show]
    o.append('-- biggest passes')
    for r_ in big: o.append('  %s' % r_)
    o.append('-- first 15 passes in order')
    for r_ in rows[:15]: o.append('  %s' % r_)
    txt = '\n'.join(o)
    open(run + '/passes.txt', 'w').write(txt + '\n')
    return txt

if __name__ == '__main__':
    for run in sys.argv[1:]:
        print(passes(run))
