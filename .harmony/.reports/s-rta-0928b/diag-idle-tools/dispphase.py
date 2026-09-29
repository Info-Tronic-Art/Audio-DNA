# dispphase.py RUNDIR... -- the AppKit display-cycle phase (BeforeTimers -> BeforeSources on the main run loop) split into
# pre-drawRect, drawRect (JUCE handlePaint) and post-drawRect (CA/AppKit after the paint), ms/s and per pass.
import sys, json, bisect, statistics as st
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from anidle import load, timeline, q
agg = {'pre': [], 'draw': [], 'post': [], 'nodraw': []}
per = {'pre': [], 'draw': [], 'post': []}
for run in sys.argv[1:]:
    R, C, L, M = load(run)
    ev = {x['what']: x['t'] for x in json.load(open(run + '/idle.json'))['events']}
    b, e = ev['idle.begin'], ev['idle.end']; dur = (e - b) / 1000
    iv = [z for z in timeline(R, b, e) if z[2] == 'timers-obs']
    dr = sorted((t, d) for k, thr, t, d, x, n in R if k in (5, 16) and thr == 0 and b <= t <= e)
    dt = [z[0] for z in dr]
    pre = draw = post = nod = 0.0
    for t0, t1, l in iv:
        i = bisect.bisect_left(dt, t0)
        inside = [z for z in dr[i:i + 4] if z[0] < t1]
        if not inside: nod += t1 - t0; continue
        s0 = inside[0][0]; s1 = inside[-1][0] + inside[-1][1]; dd = sum(z[1] for z in inside)
        pre += s0 - t0; draw += dd; post += max(0.0, t1 - s1)
        per['pre'].append(s0 - t0); per['draw'].append(dd); per['post'].append(max(0.0, t1 - s1))
    for k, v in (('pre', pre), ('draw', draw), ('post', post), ('nodraw', nod)): agg[k].append(v / dur)
print('display phase ms/s (median over launches): ' + ', '.join('%s %.1f [%.1f-%.1f]' % (k, st.median(v), min(v), max(v)) for k, v in agg.items()))
print('per pass: ' + ', '.join('%s med %.2f p90 %.2f' % (k, st.median(v), q(v, .9)) for k, v in per.items() if v))
