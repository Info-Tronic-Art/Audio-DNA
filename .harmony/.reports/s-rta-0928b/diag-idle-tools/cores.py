# cores.py RUNDIR... -- drawRect duration by union-rect class x core type (cpu0-1 = E-cores on this M1 Pro, VERIFIED by
# tools/cpuid), using the cpu number stamped on the drawRect start record (kind 9) and end record (kind 5).
import sys, json, collections, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
import anidle
from anidle import load, q
def core(c): return 'E' if c in (0, 1) else ('P' if c is not None and c >= 2 else '?')
tot = collections.defaultdict(list)
for run in sys.argv[1:]:
    anidle.CPU.clear()
    R, C, L, M = load(run)
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']
    starts = {round(t, 3): (int(x // 1e4), int(x % 1e4), int(d // 1e4), int(d % 1e4)) for k, thr, t, d, x, n in R if k == 9}
    for k, thr, t, d, x, n in R:
        if k == 5 and b <= t <= e:
            rc = starts.get(round(t, 3))
            c0 = anidle.CPU.get((9, round(t, 3))); c1 = anidle.CPU.get((5, round(t, 3)))
            cls = 'full' if rc and rc[1] == 4 and rc[3] > 900 else ('body' if rc and rc[3] > 900 else 'topbar')
            tot[(cls, core(c0) + core(c1))].append(d)
    # observer records: share on E-cores (main thread)
    obs = [anidle.CPU.get((k, round(t, 3))) for k, thr, t, d, x, n in R if k == 100 and b <= t <= e]
    print(run.rsplit('/', 2)[-2] + '/' + run.rsplit('/', 1)[-1], 'main-thread observer records on E-cores: %.1f%%' % (100 * sum(1 for c in obs if c in (0, 1)) / max(1, len(obs))))
for key in sorted(tot):
    v = tot[key]
    print('%-8s start/end core %s: n=%5d med %6.2f p90 %6.2f max %6.2f' % (key[0], key[1], len(v), st.median(v), q(v, .9), max(v)))
