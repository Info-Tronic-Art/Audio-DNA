# pool.py -- per-call distributions POOLED across the clean launches of an arm (median / p90 / max, n):
# message-thread scopes per phase, GL per-video-call fields per phase, GL per-frame callback / interval per phase.
import glob, json, os, re, sys
SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media'


def clean(d):
    c = d + '/compiler-samples.txt'
    return not (os.path.exists(c) and os.path.getsize(c) > 0)


def q(v, f):
    v = sorted(v); return v[min(len(v) - 1, max(0, int(round(f * (len(v) - 1)))))]


def s3(v):
    return None if not v else '%.3f / %.3f / %.3f (n=%d)' % (q(v, .5), q(v, .9), max(v), len(v))


def load_run(d):
    phases = json.load(open(d + '/drive-log.json'))['phases']
    lines = open(d + '/app-err.log', errors='replace').read().splitlines()
    marks = [(float(l.split()[1]), int(l.split()[4])) for l in lines if l.startswith('[EV] ') and ' hook mark ' in l]
    def ph(t):
        n = None
        for mt, i in marks:
            if mt <= t: n = phases[i]
        return n
    return phases, lines, marks, ph


def main(prefix):
    runs = [d for d in sorted(glob.glob(SP + '/runs/' + prefix + '[0-9]*')) if clean(d) and os.path.exists(d + '/drive-log.json')]
    scopes, gv, gf = {}, {}, {}
    for d in runs:
        phases, lines, marks, ph = load_run(d)
        mstart = {phases[i]: t for t, i in marks}
        for l in lines:
            if l.startswith('[DG] ') and not l.startswith('[DG] heartbeat'):
                m = re.match(r'\[DG\] ([\d.\-]+) (\d+) (\S+) ([\d.]+)$', l)
                if m:
                    scopes.setdefault((ph(float(m[1])), m[3]), []).append(float(m[4]))
            elif l.startswith('[GV] '):
                p = l.split(); t = float(p[1]); n = ph(t)
                if n is None or t < mstart.get(n, 0) + 1000: continue   # steady: skip the first second
                if p[4] != '1': continue
                g = gv.setdefault(n, {'adv_dec': [], 'adv_nodec': [], 'conv': [], 'flip': [], 'upl': [], 'ndec': []})
                ndec = int(p[6])
                (g['adv_dec'] if ndec > 0 else g['adv_nodec']).append(float(p[5]))
                if ndec > 0:
                    g['ndec'].append(ndec)
                    if float(p[10]) > 0: g['conv'].append(float(p[10])); g['flip'].append(float(p[11]))
                if p[12] != '0': g['upl'].append(float(p[13]))
            elif l.startswith('[GF] '):
                p = l.split(); t = float(p[1]); n = ph(t)
                if n is None or t < mstart.get(n, 0) + 1000: continue
                gf.setdefault(n, []).append((d, t, float(p[2]), int(p[22]), float(p[23])))
    print('==', prefix, [os.path.basename(r) for r in runs])
    for (phn, sc), v in sorted(scopes.items(), key=lambda x: (str(x[0][0]), x[0][1])):
        if sc in ('dv.refresh', 'hook.total', 'mc.handleClipTrigger') and max(v) < 1: continue
        print('  S %-24s %-28s %s' % (phn, sc, s3(v)))
    for phn, g in gv.items():
        print('  V %-24s ' % phn + ' '.join('%s=%s' % (k, s3(v)) for k, v in g.items() if v))
    for phn, rows in gf.items():
        rows.sort()
        cb = [r[2] for r in rows]
        iv, prev = [], None   # frame-start intervals within one run and one phase window
        for r in rows:
            if prev is not None and prev[0] == r[0] and 0 < r[1] - prev[1] < 2000: iv.append(r[1] - prev[1])
            prev = r
        ex = [1000 * r[4] / r[3] for r in rows if r[3] > 0]
        print('  F %-24s cb=%s iv=%s ex_us=%s' % (phn, s3(cb), s3(iv), s3(ex)))


for p in sys.argv[1:]:
    main(p)
