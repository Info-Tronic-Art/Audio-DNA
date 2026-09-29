# anidle.py RUNDIR [RUNDIR ...] -- per launch: heartbeat lag stats, main run-loop phase split (sleep vs busy by phase),
# busy spans >= 2 ms with contributors, heartbeat-lag decomposition, per-thread CPU. Writes RUNDIR/anidle.json + .txt.
# Timestamps: libc++ steady_clock ms == CLOCK_UPTIME_RAW ms (the driver's idle window).
import sys, json, re, subprocess, statistics as st, bisect, collections, os

ACT = {1: 'Entry', 2: 'BeforeTimers', 4: 'BeforeSources', 32: 'BeforeWaiting', 64: 'AfterWaiting', 128: 'Exit'}
_dm = {}
CPU = {}   # (kind, t) -> cpu number at record time (0-1 = E-cores on this M1 Pro)
def demangle(names):
    todo = [n for n in names if n not in _dm and n != '-' and n[:1].isdigit() or n[:1] == 'N' and n not in _dm]
    if todo:
        out = subprocess.run(['c++filt', '-t'], input='\n'.join(todo), capture_output=True, text=True).stdout.splitlines()
        for a, b in zip(todo, out):
            m = re.match(r'^(\d+)(.*)$', b)
            _dm[a] = m.group(2)[:int(m.group(1))] if m else b
    return _dm

def short(n):
    s = _dm.get(n, n)
    s = re.sub(r'juce::MessageManager::callAsync\(std::__1::function<void \(\)>\)::AsyncCallInvoker', 'callAsync', s)
    return s[:150]

def q(v, x):
    v = sorted(v)
    return v[min(len(v) - 1, int(x * len(v)))] if v else float('nan')

def load(run):
    R, C, L, M = [], [], [], []
    for ln in open(run + '/diag.tsv', errors='replace'):
        p = ln.rstrip('\n').split('\t')
        if p[0] == 'R' and len(p) >= 7:
            R.append((int(p[1]), int(p[2]), float(p[3]), float(p[4]), float(p[5]), p[6]))
            if len(p) >= 8: CPU[(int(p[1]), round(float(p[3]), 3))] = int(p[7])
        elif p[0] == 'C' and len(p) >= 8:
            C.append((float(p[1]), int(p[2]), int(p[3]), int(p[4]), int(p[5]), int(p[6]), p[7]))
        elif p[0] == 'L':
            L.append((float(p[1]), float(p[2]), int(p[5])))
        elif p[0] == 'M':
            M.append(p[1:])
    return R, C, L, M

def timeline(R, b, e):
    """Main-thread phase intervals from the two observers, clipped to [b, e]."""
    obs = [(t, k, int(x)) for k, thr, t, d, x, n in R if k in (100, 200)]
    obs.sort(key=lambda z: (z[0], 0 if z[1] == 100 else 1))
    lab = {(100, 1): 'loop-entry', (100, 2): 'timers-obs', (100, 4): 'sources0', (100, 32): 'bw-observers', (200, 32): 'sleep',
           (100, 64): 'aw-observers', (200, 64): 'wake-handle', (100, 128): 'outside-runloop'}
    iv = []
    for (t0, k, a), (t1, _, _) in zip(obs, obs[1:]):
        if t1 < b or t0 > e: continue
        iv.append((max(t0, b), min(t1, e), lab.get((k, a), 'obs%d-%d' % (k, a))))
    return iv

def analyze(run):
    R, C, L, M = load(run)
    idl = json.load(open(run + '/idle.json'))
    evs = {x['what']: x['t'] for x in idl['events']}
    b, e = evs['idle.begin'], evs['idle.end']
    dur_s = (e - b) / 1000.0
    main = [r for r in R if r[1] == 0]
    demangle(set(r[5] for r in R))
    res = {'run': run, 'fixture': idl['fixture'], 'idle_s': round(dur_s, 2), 'meta': M}
    # --- heartbeat
    hb = [(t, d) for k, thr, t, d, x, n in R if k == 40 and b <= t <= e]
    lags = [d for t, d in hb]
    big = [d for d in lags if d >= 2.0]
    wmax = []
    t = b
    while t + 500 <= e:
        wmax.append(max([d for p, d in hb if t <= p < t + 500] or [0])); t += 500
    if not hb:   # ADNA_DIAG_NOHB arm: no heartbeat
        res['hb'] = dict(n=0, n_ge2_per_s=None, ge2_med=None, ge2_p90=None, win_max_med=None, win_max_p90=None, win_max_max=None, all_med=None, lag_ms_per_s=None)
    else:
        res['hb'] = dict(n=len(hb), n_ge2_per_s=round(len(big) / dur_s, 2), ge2_med=round(st.median(big), 2) if big else 0,
                     ge2_p90=round(q(big, .9), 2) if big else 0, win_max_med=round(st.median(wmax), 2), win_max_p90=round(q(wmax, .9), 2),
                     win_max_max=round(max(wmax), 2), all_med=round(st.median(lags), 3), lag_ms_per_s=round(sum(big) / dur_s, 1))
    # --- phases
    iv = timeline(R, b, e)
    ph = collections.Counter()
    for t0, t1, l in iv: ph[l] += t1 - t0
    covered = sum(ph.values())
    res['phase_ms_per_s'] = {k: round(v / dur_s, 2) for k, v in sorted(ph.items(), key=lambda z: -z[1])}
    res['covered_frac'] = round(covered / (e - b), 4)
    busy = covered - ph['sleep']
    res['busy_ms_per_s'] = round(busy / dur_s, 2)
    # --- JUCE records on main within window: top-level messages, timers, asyncupdaters, drawRect, paint
    def agg(kind, thr0=True):
        c = collections.defaultdict(lambda: [0, 0.0, 0.0])
        for k, thr, t, d, x, n in R:
            if k == kind and (thr == 0) == thr0 and b <= t <= e:
                c[n][0] += 1; c[n][1] += d; c[n][2] = max(c[n][2], d)
        return sorted(((short(n), v[0] / dur_s, v[1] / dur_s, v[1] / max(1, v[0]), v[2]) for n, v in c.items()), key=lambda z: -z[2])
    res['msgs'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(1)[:15]]
    res['timers'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(2)[:20]]
    res['async'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(3)[:10]]
    res['drawRect'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(5)[:5]]
    res['handlePaint'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(6)[:5]]
    res['paintSelf'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(4)[:25]]
    res['appScopes'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(30)[:40]]
    res['glMain'] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(11)[:5]]
    for kk, nm in ((12, 'glLockWait'), (13, 'glPaintUnderLock'), (14, 'glSwap'), (15, 'glRenderFrame')):
        res[nm] = [dict(name=a, per_s=round(b_, 2), ms_per_s=round(c_, 3), mean=round(d_, 3), max=round(m, 2)) for a, b_, c_, d_, m in agg(kk, False)[:3]]
    nd = [x for k, thr, t, d, x, n in R if k == 7 and b <= t <= e]
    res['setNeedsDisplay'] = dict(per_s=round(len(nd) / dur_s, 2), area_med=round(st.median(nd), 0) if nd else 0, area_max=max(nd) if nd else 0)
    # --- busy spans (between sleeps) >= 2 ms, contributors
    spans = []
    cur = None
    for t0, t1, l in iv:
        if l == 'sleep':
            if cur: spans.append(cur); cur = None
            continue
        if cur is None: cur = {'t0': t0, 't1': t1, 'ph': collections.Counter()}
        cur['t1'] = t1; cur['ph'][l] += t1 - t0
    if cur: spans.append(cur)
    tops = [(t, d, k, n) for k, thr, t, d, x, n in main if k in (1, 5) and b <= t <= e]
    tops.sort()
    tt = [z[0] for z in tops]
    subs = [(t, d, k, n) for k, thr, t, d, x, n in main if k in (2, 3, 30) and b <= t <= e]
    subs.sort(); st_ = [z[0] for z in subs]
    big_spans = []
    for s in spans:
        d = s['t1'] - s['t0']
        if d < 2.0: continue
        i0 = bisect.bisect_left(tt, s['t0'] - 0.01); i1 = bisect.bisect_right(tt, s['t1'] + 0.01)
        contrib = collections.Counter()
        for t, dd, k, n in tops[i0:i1]:
            contrib[('drawRect' if k == 5 else 'msg:') + ('' if k == 5 else short(n))] += dd
        j0 = bisect.bisect_left(st_, s['t0'] - 0.01); j1 = bisect.bisect_right(st_, s['t1'] + 0.01)
        sc = collections.Counter()
        for t, dd, k, n in subs[j0:j1]:
            sc[{2: 'timer:', 3: 'async:', 30: 'scope:'}[k] + short(n)] += dd
        big_spans.append(dict(t=round(s['t0'] - b, 1), dur=round(d, 2), phases={k: round(v, 2) for k, v in s['ph'].most_common()},
                              top=[(k, round(v, 2)) for k, v in contrib.most_common(4)], sub=[(k, round(v, 2)) for k, v in sc.most_common(4)]))
    res['spans_ge2'] = dict(per_s=round(len(big_spans) / dur_s, 2), dur_med=round(st.median([s['dur'] for s in big_spans]), 2) if big_spans else 0,
                            dur_p90=round(q([s['dur'] for s in big_spans], .9), 2) if big_spans else 0,
                            dur_max=max([s['dur'] for s in big_spans] or [0]),
                            ms_per_s=round(sum(s['dur'] for s in big_spans) / dur_s, 2))
    ge10 = [s for s in big_spans if s['dur'] >= 10]
    res['spans_ge10'] = dict(per_s=round(len(ge10) / dur_s, 2),
                             gap_med=round(st.median([bb['t'] - a['t'] for a, bb in zip(ge10, ge10[1:])]), 1) if len(ge10) > 2 else None)
    res['span_examples'] = sorted(big_spans, key=lambda s: -s['dur'])[:8]
    # phase share inside big spans
    pc = collections.Counter()
    for s in big_spans:
        for k, v in s['phases'].items(): pc[k] += v
    res['spans_ge2_phase_ms_per_s'] = {k: round(v / dur_s, 2) for k, v in pc.most_common()}
    # --- heartbeat lag decomposition (lags >= 2 ms): what the main thread was doing while the ping waited
    ivs = sorted(iv); it0 = [z[0] for z in ivs]
    dec = collections.Counter(); decm = collections.Counter(); total = 0.0
    hbmsg = [(t, d, n) for t, d, k, n in tops if k == 1]
    ht = [z[0] for z in hbmsg]
    for p, d in hb:
        if d < 2.0: continue
        s_ = p + d; total += d
        i = max(0, bisect.bisect_right(it0, p) - 1)
        while i < len(ivs) and ivs[i][0] < s_:
            t0, t1, l = ivs[i]
            ov = min(t1, s_) - max(t0, p)
            if ov > 0: dec[l] += ov
            i += 1
        j = bisect.bisect_left(ht, p - 60)
        while j < len(hbmsg) and hbmsg[j][0] < s_:
            t0, dd, n = hbmsg[j]
            ov = min(t0 + dd, s_) - max(t0, p)
            if ov > 0: decm[short(n)] += ov
            j += 1
        # drawRect overlap
    res['hb_lag_decomp_pct'] = {k: round(100 * v / total, 1) for k, v in dec.most_common()} if total else {}
    res['hb_lag_msg_pct'] = [(k, round(100 * v / total, 1)) for k, v in decm.most_common(8)] if total else []
    dr = [(t, d) for t, d, k, n in tops if k == 5]
    drv = 0.0
    for p, d in hb:
        if d < 2.0: continue
        for t0, dd in dr:
            ov = min(t0 + dd, p + d) - max(t0, p)
            if ov > 0: drv += ov
    res['hb_lag_drawRect_pct'] = round(100 * drv / total, 1) if total else 0
    # --- CPU per thread over the window
    cw = [c for c in C if b - 600 <= c[0] <= e + 600]
    if cw:
        ts = sorted(set(c[0] for c in cw))
        ta = min(ts, key=lambda x: abs(x - b)); tb = min(ts, key=lambda x: abs(x - e))
        A = {c[1]: c for c in cw if c[0] == ta}; B = {c[1]: c for c in cw if c[0] == tb}
        rows = []
        for tid, cb in B.items():
            ca = A.get(tid)
            if ca is None: continue
            ms = (cb[2] - ca[2]) / 1e6 / ((tb - ta) / 1000.0)
            rows.append((cb[6] + ('[MAIN]' if cb[3] else ''), round(ms, 2), cb[4], cb[5]))
        rows.sort(key=lambda r: -r[1])
        res['cpu_ms_per_s'] = rows[:14]
        res['cpu_main_ms_per_s'] = next((r[1] for r in rows if r[0].endswith('[MAIN]')), None)
        res['cpu_total_ms_per_s'] = round(sum(r[1] for r in rows), 1)
    mc = sorted((c[0], c[2]) for c in C if c[3] == 1 and b <= c[0] <= e)
    per = [(y[1] - x[1]) / 1e6 / ((y[0] - x[0]) / 1000.0) for x, y in zip(mc, mc[1:]) if y[0] > x[0]]
    res['cpu_main_per_window'] = dict(n=len(per), med=round(st.median(per), 1) if per else None, p90=round(q(per, .9), 1) if per else None,
                                      max=round(max(per), 1) if per else None)
    la = [l for l in L if b <= l[0] <= e]
    res['load'] = (la[0][1], la[-1][1]) if la else None
    res['lost'] = L[-1][2] if L else None
    st8 = [(x) for k, thr, t, d, x, n in R if k == 41 and b <= t <= e]
    res['appState'] = sorted(set(st8))
    return res

def fmt(r):
    o = []
    o.append('== %s fixture=%s idle=%.1fs load=%s lost=%s appState(1=active,2=visible)=%s' % (r['run'], r['fixture'], r['idle_s'], r['load'], r['lost'], r['appState']))
    o.append('HB: %s' % r['hb'])
    o.append('main busy ms/s %.1f (covered %.3f); phases ms/s %s' % (r['busy_ms_per_s'], r['covered_frac'], r['phase_ms_per_s']))
    o.append('CPU main ms/s %s (per 500 ms window %s) total %s ; top threads %s' % (r.get('cpu_main_ms_per_s'), r.get('cpu_main_per_window'), r.get('cpu_total_ms_per_s'), r.get('cpu_ms_per_s', [])[:8]))
    o.append('spans>=2ms %s ; >=10ms %s ; phase ms/s in spans %s' % (r['spans_ge2'], r['spans_ge10'], r['spans_ge2_phase_ms_per_s']))
    o.append('HB lag decomposition %% by phase: %s ; drawRect %s%% ; msgs %s' % (r['hb_lag_decomp_pct'], r['hb_lag_drawRect_pct'], r['hb_lag_msg_pct']))
    o.append('setNeedsDisplay %s' % r['setNeedsDisplay'])
    for key in ('drawRect', 'handlePaint', 'msgs', 'timers', 'async', 'paintSelf', 'appScopes', 'glMain', 'glLockWait', 'glPaintUnderLock', 'glSwap', 'glRenderFrame'):
        if r[key]:
            o.append('-- %s' % key)
            for x in r[key]: o.append('   %8.2f/s %8.3f ms/s mean %7.3f max %7.2f  %s' % (x['per_s'], x['ms_per_s'], x['mean'], x['max'], x['name']))
    o.append('-- biggest spans')
    for s in r['span_examples']: o.append('   %s' % s)
    return '\n'.join(o)

if __name__ == '__main__':
    for run in sys.argv[1:]:
        r = analyze(run)
        json.dump(r, open(run + '/anidle.json', 'w'), indent=1)
        txt = fmt(r)
        open(run + '/anidle.txt', 'w').write(txt + '\n')
        print(txt)
