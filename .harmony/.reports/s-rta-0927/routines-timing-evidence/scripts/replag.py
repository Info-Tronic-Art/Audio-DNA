import json, os, sys, glob, statistics as st
def load(p):
    try: return json.load(open(p))
    except Exception: return None
def S_of(s, slot):
    run = [r for r in s if r.get('bank') and 'ts' in r and r['bank'][slot].get('state') == 'running' and isinstance(r['bank'][slot].get('position'), (int, float)) and r['bank'][slot].get('cycle') == 1]
    return min(r['ts'] - r['bank'][slot]['position'] * 0.5 for r in run) if run else None
def stall(s, W):
    rows = [r for r in s if isinstance(r.get('cb'), (int, float))]
    w0 = min(r['ts'] - r['cb'] * 0.5 for r in rows)
    v = [r['ts'] - (w0 + r['cb'] * 0.5) for r in rows if abs(r['t'] - W) <= 0.25]
    return max(v) if v else None
def m0(folder):
    b = []
    for ln in load(folder + '/take.json').get('lanes', []):
        k = ln.get('key', {}); L = k.get('layer'); L = L.get('i') if isinstance(L, dict) else L
        if k.get('scope') == 'layer' and k.get('scalar') == 'opacity' and L == 0:
            b += [p['beat'] for p in ln.get('points', [])] + [g['curve'][0]['x'] for g in ln.get('gestures', []) if g.get('curve')]
    return min(x for x in b if 0 <= x < 16)
for d in sorted(glob.glob(os.path.join(sys.argv[1], '*', 'run-*'))):
    lab = os.path.basename(os.path.dirname(d))
    ev = [json.loads(l) for l in open(os.path.join(d, 'timing.jsonl'))]
    an = next(e for e in ev if e.get('ev') == 'anchors'); M = m0(an['take'])
    jl, jm, js, sl, sm, ss = [], [], [], [], [], []
    for e in ev:
        if e.get('ev') == 'rjump':
            s = load(os.path.join(d, 'rjump%d.json' % e['k'])); W = S_of(s, 0)
            if W and e['TJ'] not in ('NA', ''):
                lag = float(e['TJ']) - W; jl.append(lag); jm.append(M * 0.5 - 0.5 - lag); js.append(stall(s, W))
        if e.get('ev') == 'rstack':
            s = load(os.path.join(d, 'rstack%d.json' % e['k'])); W = S_of(s, 0)
            if W and e['T6'] not in ('NA', ''):
                lag = float(e['T6']) - W; sl.append(lag); sm.append(M * 0.5 - 0.5 - lag); ss.append(stall(s, W))
    f = lambda v: 'n=%d med %.3f p90 %.3f max %.3f' % (len(v), st.median(v), sorted(v)[int(0.9 * (len(v) - 1))], max(v)) if v else '-'
    print(lab, 'm0 %.3f' % M)
    print('   jump  lag TJ-start:', f(jl), '| margin(end vs move):', f(jm), 'min %.3f' % min(jm), '| start stall:', f(js))
    print('   stack lag T6-start:', f(sl), '| margin:', f(sm), 'min %.3f' % min(sm), '| start stall:', f(ss))
