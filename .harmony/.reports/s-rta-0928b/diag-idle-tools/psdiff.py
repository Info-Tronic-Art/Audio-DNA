# psdiff.py RUNDIR... -- main-thread and process CPU ms/s over the idle window from ps -M samples (first thread row =
# the main thread), for arms run with PS_SAMPLE=1 (works on an uninstrumented app).
import sys, re, statistics as st
def t2ms(s):
    m = re.match(r'(?:(\d+):)?(\d+)\.(\d+)', s)
    return ((int(m[1] or 0) * 60 + int(m[2])) * 1000 + int(m[3].ljust(3, '0')[:3])) if m else 0
def parse(p):
    L = open(p).read().splitlines()
    t = float(L[0]); rows = []
    for ln in L[2:]:
        f = ln.split()
        # USER PID TT %CPU STAT PRI STIME UTIME [COMMAND] (thread rows have no PID column when the process row precedes)
        tm = [x for x in f if re.match(r'^\d+:\d+\.\d+$', x)]
        if len(tm) >= 2: rows.append(t2ms(tm[0]) + t2ms(tm[1]))
    return t, rows
res = []
for run in sys.argv[1:]:
    t0, a = parse(run + '/ps-begin.txt'); t1, b = parse(run + '/ps-end.txt')
    dt = (t1 - t0) / 1000.0
    main = (b[0] - a[0]) / dt
    tot = (sum(b) - sum(a)) / dt
    res.append((main, tot)); print(run.rsplit('/', 2)[-2:], 'main %.1f ms/s  process %.1f ms/s  (%d threads)' % (main, tot, len(b)))
if res:
    m = [x[0] for x in res]; t = [x[1] for x in res]
    print('median main %.1f [%.1f-%.1f]  process %.1f [%.1f-%.1f]' % (st.median(m), min(m), max(m), st.median(t), min(t), max(t)))
