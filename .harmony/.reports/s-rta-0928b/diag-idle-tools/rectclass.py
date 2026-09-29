# rectclass.py RUNDIR -- drawRect duration by union-rect class, and the time series of pass durations (first 40)
import sys, json, collections, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, q
run = sys.argv[1]
R, C, L, M = load(run)
ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
b, e = ev['idle.begin'], ev['idle.end']
rect = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
cls = collections.defaultdict(list); seq = []
for k, thr, t, d, x, n in R:
    if k == 5 and b <= t <= e:
        rc = rect.get(round(t, 3)); cls[rc].append(d); seq.append((round(t - b, 1), rc[1] if rc else None, rc[3] if rc else None, round(d, 1)))
for rc, v in sorted(cls.items(), key=lambda z: -len(z[1])):
    print(rc, 'n=%d/s=%.1f med %.2f p90 %.2f max %.2f sum %.1f ms/s' % (len(v), len(v) / ((e - b) / 1000), st.median(v), q(v, .9), max(v), sum(v) / ((e - b) / 1000)))
print(seq[:60])
