# compclass.py RUNDIR... -- exclusive paint ms/s by component type, split by pass class (body / full / top bar),
# summed over launches and divided by total idle seconds; plus repaint() sources per second.
import sys, json, bisect, collections
sys.path.insert(0, __file__.rsplit('/', 1)[0])
import anidle
from anidle import load, demangle, short
tot_s = 0.0; comp = collections.defaultdict(lambda: collections.Counter()); src = collections.Counter()
for run in sys.argv[1:]:
    R, C, L, M = load(run)
    demangle(set(r[5] for r in R))
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']; tot_s += (e - b) / 1000
    rect = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
    dr = sorted((t, d) for k, thr, t, d, x, n in R if k in (5, 16) and thr == 0 and b <= t <= e)
    for k, thr, t, d, x, n in R:
        if k == 8 and b <= t <= e: src[short(n)] += 1
    dt = [z[0] for z in dr]
    for k, thr, t, d, x, n in R:
        if k != 4 or thr != 0 or not (b <= t <= e): continue
        i = bisect.bisect_right(dt, t) - 1
        if i < 0 or t > dr[i][0] + dr[i][1]: c = 'outside'
        else:
            rc = rect.get(round(dr[i][0], 3))
            c = 'metal' if rc is None else ('full' if rc[3] > 900 and rc[1] <= 10 else ('body' if rc[3] > 900 else 'top'))
        comp[c][short(n)] += d
for c, cc in comp.items():
    print('== %s: total %.1f ms/s' % (c, sum(cc.values()) / tot_s))
    print('   ' + ', '.join('%s %.1f' % (a, v / tot_s) for a, v in cc.most_common(12)))
print('repaint() calls/s: ' + ', '.join('%s %.1f' % (a, v / tot_s) for a, v in src.most_common(10)))
