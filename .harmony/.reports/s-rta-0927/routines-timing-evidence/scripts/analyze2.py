# Per run: lag of each probe anchor (T1 T2 T4 T5 T6 TJ) behind the routine's TRUE start, estimated from the
# status the sampler read: Ws_hat = min over early running samples of (ts - position * 0.5 s/beat)
# (an upper bound on the start, error = the freshest sample's staleness). Also the message-thread stall
# around each start (max status staleness within +-0.25 s of Ws_hat), capture durations, fail rows.
import json, os, sys, glob, statistics as st
def load(p):
    try: return json.load(open(p))
    except Exception: return None
def w0(s):
    rows = [r for r in s if isinstance(r.get('cb'), (int, float)) and 'ts' in r]
    return min(r['ts'] - r['cb'] / 2.0 for r in rows) if rows else None
def ws_hat(s, slot, after=0.0):
    run = [r for r in s if r.get('bank') and r['t'] >= after and r['bank'][slot]['state'] == 'running'
           and isinstance(r['bank'][slot].get('position'), (int, float))]
    if not run: return None, None
    t0 = run[0]['t']
    early = [r for r in run if r['t'] <= t0 + 1.0 and r['bank'][slot]['position'] < 4.0]
    best = min(early, key=lambda r: r['ts'] - r['bank'][slot]['position'] * 0.5)
    return best['ts'] - best['bank'][slot]['position'] * 0.5, best['ts'] - best['bank'][slot]['position'] * 0.5 - (w0(s) + best['cb'] / 2.0 - best['bank'][slot]['position'] * 0.5)
def stall(s, Ws):
    W = w0(s)
    v = [r['ts'] - (W + r['cb'] / 2.0) for r in s if isinstance(r.get('cb'), (int, float)) and abs(r['t'] - Ws) <= 0.25]
    return max(v) if v else None
def run(out, logp):
    ev = [json.loads(l) for l in open(os.path.join(out, 'timing.jsonl')) if l.strip()]
    an = next((e for e in ev if e.get('ev') == 'anchors'), {})
    f = lambda n: load(os.path.join(out, n + '.json')) or []
    R = {'caps': {e['name']: round(e['t1'] - e['t0'], 3) for e in ev if e.get('ev') == 'capture'}}
    def num(x):
        try: return float(x)
        except Exception: return None
    specs = [('T1', 'glide', 0, 0.0), ('T4', 'fromnow', 0, 0.0), ('T5', 'stack', 1, 0.0), ('T6', 'stack', 0, 0.0),
             ('TJ', 'jump', 0, 0.0)]
    for name, fn, slot, _ in specs:
        s = f(fn); T = num(an.get(name))
        if not s or T is None: continue
        Ws, _ = ws_hat(s, slot)
        if Ws is None: continue
        R[name] = {'lag': round(T - Ws, 3), 'stall': round(stall(s, Ws) or -1, 3)}
    # jump loop return: second cycle start in jumploop.json (cycle == 2)
    s = f('jumploop'); TJ = num(an.get('TJ'))
    if s and TJ:
        c2 = [r for r in s if r.get('bank') and r['bank'][0].get('cycle') == 2 and isinstance(r['bank'][0].get('position'), (int, float))]
        if c2:
            best = min(c2[:30], key=lambda r: r['ts'] - r['bank'][0]['position'] * 0.5)
            L = best['ts'] - best['bank'][0]['position'] * 0.5
            R['TJloop'] = {'loop_point_minus_TJ': round(L - TJ, 3)}
    s = f('loop'); T2 = num(an.get('T2'))
    if s and T2:
        c2 = [r for r in s if r.get('bank') and r['bank'][0].get('cycle') == 2 and isinstance(r['bank'][0].get('position'), (int, float))]
        c1 = [r for r in s if r.get('bank') and r['bank'][0].get('cycle') == 1 and isinstance(r['bank'][0].get('position'), (int, float))]
        if c1:
            best = min(c1, key=lambda r: r['ts'] - r['bank'][0]['position'] * 0.5)
            R['T2'] = {'lag': round(T2 - (best['ts'] - best['bank'][0]['position'] * 0.5), 3)}
        if c2:
            best = min(c2[:30], key=lambda r: r['ts'] - r['bank'][0]['position'] * 0.5)
            R['T2']['loop_point_minus_T2'] = round(best['ts'] - best['bank'][0]['position'] * 0.5 - T2, 3)
    allst = []
    for fn in ('glide', 'grid', 'loop', 'hold', 'restart', 'fromnow', 'stack', 'jump', 'jumploop'):
        s = f(fn); W = w0(s) if s else None
        if W is None: continue
        allst += [r['ts'] - (W + r['cb'] / 2.0) for r in s if isinstance(r.get('cb'), (int, float))]
        R.setdefault('get_ms', []).extend([(r['tc'] - r['t']) * 1000 for r in s if 'tc' in r])
    g = sorted(R.pop('get_ms', []))
    R['comp_get_ms_med_p90_max'] = [round(g[len(g)//2], 1), round(g[int(.9*len(g))], 1), round(g[-1], 1)] if g else None
    allst.sort()
    R['stale_ms_med_p90_max'] = [round(1000*allst[len(allst)//2], 1), round(1000*allst[int(.9*len(allst))], 1), round(1000*allst[-1], 1)] if allst else None
    lines = open(logp).read().splitlines()
    R['summary'] = next((l.split('   (')[0] for l in reversed(lines) if 'PASS /' in l), None)
    R['head'] = lines[0][lines[0].find('load='):] if lines else ''
    R['fails'] = [l for l in lines if l.startswith('FAIL')]
    return R
if __name__ == '__main__':
    base = sys.argv[1]
    for logp in sorted(glob.glob(os.path.join(base, '*.log'))):
        lab = os.path.basename(logp)[:-4]
        ds = glob.glob(os.path.join(base, lab, 'run-*'))
        if not ds or not os.path.exists(os.path.join(ds[0], 'timing.jsonl')): continue
        if 'end' not in open(logp).read().splitlines()[-1]: continue
        print(lab, json.dumps(run(ds[0], logp)))

def capture_stall(out):
    # For the two captures that overlap a sampler (mid.png in glide.json, jmid.png in jump.json): the max
    # status staleness and max composition-GET time of samples taken DURING the capture vs outside it.
    ev = [json.loads(l) for l in open(os.path.join(out, 'timing.jsonl')) if l.strip()]
    res = {}
    for cap, fn in (('mid.png', 'glide'), ('jmid.png', 'jump')):
        c = next((e for e in ev if e.get('ev') == 'capture' and e['name'] == cap), None)
        s = load(os.path.join(out, fn + '.json'))
        if not c or not s: continue
        W = w0(s)
        rows = [r for r in s if isinstance(r.get('cb'), (int, float))]
        inn = [r for r in rows if c['t0'] <= r['t'] <= c['t1']]
        outr = [r for r in rows if not (c['t0'] - 0.05 <= r['t'] <= c['t1'] + 0.05)]
        f = lambda rr: (round(1000 * max((r['ts'] - (W + r['cb'] / 2.0)) for r in rr), 1) if rr else None,
                        round(1000 * max((r['tc'] - r['t']) for r in rr), 1) if rr else None, len(rr))
        res[cap] = {'dur_ms': round(1000 * (c['t1'] - c['t0']), 1), 'during(stale_max_ms,get_max_ms,n)': f(inn), 'outside': f(outr)}
    return res

if __name__ == '__main__' and len(sys.argv) > 2 and sys.argv[2] == 'caps':
    for logp in sorted(glob.glob(os.path.join(sys.argv[1], '*.log'))):
        lab = os.path.basename(logp)[:-4]
        ds = glob.glob(os.path.join(sys.argv[1], lab, 'run-*'))
        if ds and os.path.exists(os.path.join(ds[0], 'timing.jsonl')) and 'end' in open(logp).read().splitlines()[-1]:
            print(lab, json.dumps(capture_stall(ds[0])))
