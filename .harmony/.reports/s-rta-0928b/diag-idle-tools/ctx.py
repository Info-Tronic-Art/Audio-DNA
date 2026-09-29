# ctx.py RUNDIR... -- per drawRect: pass class, CGContext type (CGContextGetType via dlsym), main-thread CPU ms vs wall ms
import sys, json, collections, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, q
rows = collections.defaultdict(list)
for run in sys.argv[1:]:
    R, C, L, M = load(run)
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']
    rect = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
    ctype = {round(t, 3): (d, x) for k, thr, t, d, x, n in R if k == 20}
    cpu = {round(t, 3): (d, x) for k, thr, t, d, x, n in R if k == 21}
    for t3, (wall, cms) in cpu.items():
        if not (b <= t3 <= e): continue
        rc = rect.get(t3); ct = ctype.get(t3)
        if not rc: continue
        c = 'full' if rc[3] > 900 and rc[1] <= 10 else ('body' if rc[3] > 900 else 'other')
        rows[(c, ct[1] if ct else None, ct[0] if ct else None)].append((wall, cms))
for k, v in sorted(rows.items(), key=lambda z: str(z[0])):
    w = [a for a, _ in v]; c = [b for _, b in v]
    print('%-6s ctxType=%s drawsAsync=%s n=%5d wall med %.2f p90 %.2f | thread CPU med %.2f p90 %.2f | CPU/wall %.2f'
          % (k[0], k[1], k[2], len(v), st.median(w), q(w, .9), st.median(c), q(c, .9), sum(c) / max(1e-9, sum(w))))
