# Detect beat-clock discontinuities: in each sample file, the offset (ts - clockBeat*0.5) of the FRESHEST
# samples should be constant. Report the per-file spread of a rolling-min offset (0.5 s bins).
import json, os, sys, glob
for d in sorted(glob.glob(os.path.join(sys.argv[1], '*', 'run-*'))):
    lab = os.path.basename(os.path.dirname(d)); out = []
    for fn in ('glide','grid','loop','hold','restart','fromnow','stack','jump','jumploop'):
        try: s = json.load(open(os.path.join(d, fn + '.json')))
        except Exception: continue
        rows = [(r['ts'], r['ts'] - r['cb'] * 0.5) for r in s if isinstance(r.get('cb'), (int, float)) and 'ts' in r]
        if not rows: continue
        t0 = rows[0][0]; bins = {}
        for t, o in rows:
            b = int((t - t0) / 0.5); bins[b] = min(bins.get(b, 9e18), o)
        vals = [bins[k] for k in sorted(bins)]
        spread = max(vals) - min(vals)
        if spread > 0.03: out.append('%s spread %.3f (%s)' % (fn, spread, ' '.join('%+.3f' % (v - vals[0]) for v in vals)))
    print(lab, out if out else 'clock steady (all files spread <= 0.03 s)')
