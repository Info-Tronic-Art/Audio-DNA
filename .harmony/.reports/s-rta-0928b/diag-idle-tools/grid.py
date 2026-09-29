# grid.py RUNDIR... -- per drawRect: the union rect class, the share of the 32x20 view grid inside the union rect, the
# share AppKit reports as needing display (needsToDrawRect, the TRUE dirty region), and the drawRect duration.
import sys, json, collections, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, q
rows = collections.defaultdict(list)
for run in sys.argv[1:]:
    R, C, L, M = load(run)
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']
    rect = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
    grid = {round(t, 3): (d, x) for k, thr, t, d, x, n in R if k == 19}
    for k, thr, t, d, x, n in R:
        if k == 5 and b <= t <= e:
            rc = rect.get(round(t, 3)); g = grid.get(round(t, 3))
            if not rc or not g: continue
            c = 'full' if rc[3] > 900 and rc[1] <= 10 else ('body' if rc[3] > 900 else 'other')
            rows[c].append((g[0] / 640, g[1] / 640, d))
for c, v in rows.items():
    inr = [a for a, _, _ in v]; dirty = [b for _, b, _ in v]; dur = [z for _, _, z in v]
    print('%-6s n=%5d  union-rect share of view med %.2f ; TRUE dirty share med %.2f p10 %.2f p90 %.2f ; drawRect ms med %.2f p90 %.2f'
          % (c, len(v), st.median(inr), st.median(dirty), q(dirty, .1), q(dirty, .9), st.median(dur), q(dur, .9)))
    # duration vs dirty share buckets
    bk = collections.defaultdict(list)
    for a, bb, z in v: bk[round(bb, 1)].append(z)
    print('       drawRect ms by TRUE dirty share: ' + ', '.join('%.1f:%.1f(n%d)' % (k, st.median(w), len(w)) for k, w in sorted(bk.items())))
