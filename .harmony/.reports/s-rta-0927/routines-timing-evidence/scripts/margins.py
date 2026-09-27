# Old-window margins per run: how far each OLD window's end sits BEFORE the event it must not reach
# (negative = the window reached past the event). Event times on the routine clock (start_est on the samples).
import json, os, sys, glob
sys.argv_saved = sys.argv
SPB, M0, LEN = 0.5, None, 16.0
def load(p):
    try: return json.load(open(p))
    except Exception: return None
def S_of(s, slot, cyc):
    run = [r for r in s if r.get('bank') and 'ts' in r and r['bank'][slot].get('state') == 'running'
           and isinstance(r['bank'][slot].get('position'), (int, float)) and r['bank'][slot].get('cycle') == cyc]
    return min(r['ts'] - r['bank'][slot]['position'] * SPB for r in run) if run else None
def m0(folder):
    b = []
    for ln in load(folder + '/take.json').get('lanes', []):
        k = ln.get('key', {}); L = k.get('layer'); L = L.get('i') if isinstance(L, dict) else L
        if k.get('scope') == 'layer' and k.get('scalar') == 'opacity' and L == 0:
            b += [p['beat'] for p in ln.get('points', [])] + [g['curve'][0]['x'] for g in ln.get('gestures', []) if g.get('curve')]
    return min(x for x in b if 0 <= x < 16)
rows = []
for d in sorted(glob.glob(os.path.join(sys.argv[1], '*', 'run-*'))):
    lab = os.path.basename(os.path.dirname(d))
    tl = os.path.join(d, 'timing.jsonl')
    if not os.path.exists(tl): continue
    an = next((json.loads(l) for l in open(tl) if 'anchors' in l), None)
    if not an: continue
    f = lambda n: load(os.path.join(d, n + '.json')) or []
    m = m0(an['take'])
    T6, TJ, T2 = float(an['T6']), float(an['TJ']), float(an['T2'])
    S6, SJ, S2, SJl = S_of(f('stack'), 0, 1), S_of(f('jump'), 0, 1), S_of(f('loop'), 0, 1), S_of(f('jumploop'), 0, 1)
    r = {'run': lab, 'm0': round(m, 3),
         'lag T6/TJ/T2': [round(T6 - S6, 3), round(TJ - SJ, 3), round(T2 - S2, 3)],
         'stack win end vs 1st move': round((S6 + m * SPB) - (T6 + 0.5), 3),
         'jump win end vs 1st move': round((SJ + m * SPB) - (TJ + 0.5), 3),
         'loop8 hold end vs glide start': round((S2 + (LEN - 1) * SPB) - (T2 + 7.45), 3),
         'jumploop hold end vs loop point': round((SJl + LEN * SPB) - (TJ + 7.9), 3)}
    rows.append(r); print(json.dumps(r))
