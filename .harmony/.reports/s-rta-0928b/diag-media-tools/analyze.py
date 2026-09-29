# analyze.py RUNDIR [--json] -- per-phase attribution of one diag-media launch (app clock, ms).
# GL: [GF] per frame (start, callback ms, frame-window ms, compositeDeck ms, deckActive | video | exists | seq | pump)
#     [GV] per video syncMedia call (t, clip, WxH, decode?, adv ms, nDec, nSeek, decMs, seekMs, convMs, flipMs, nUpl, uplMs, playhead)
# MSG: [HB] heartbeat latency >= 2 ms (post t, latency), [DG] scopes (start, depth, name, dur), [CP] ClipCell paints,
#      [EV] hook marks (phase boundaries).
import json, re, sys, statistics as st
from collections import defaultdict

run = sys.argv[1]
lines = open(run + '/app-err.log', errors='replace').read().splitlines()
try:
    phases = json.load(open(run + '/drive-log.json'))['phases']
except Exception:
    phases = []
GF, GV, HB, DG, CP, MK, IM = [], [], [], [], [], [], []
lastT = 0.0
for ln in lines:
    if ln.startswith('[Image] decoded '):
        m = re.search(r'\((\d+)x(\d+)\) decode=([\d.]+) convert=([\d.]+) ms', ln)
        if m:
            IM.append((lastT, int(m[1]) * int(m[2]), float(m[3]), float(m[4]), 'seq_' in ln))
        continue
    if ln[:5] in ('[GF] ', '[GV] ', '[HB] ', '[CP] ') :
        try:
            lastT = float(ln.split()[1])
        except (IndexError, ValueError):
            pass
    if ln.startswith('[GF] '):
        p = ln.split()
        # [GF] t cb fw comp deck | v calls dec seek conv adv decMs seekMs convMs upl uplMs flipMs lockMs | e n ms | s calls upl ms uplMs | p pump
        try:
            GF.append(dict(t=float(p[1]), cb=float(p[2]), fw=float(p[3]), comp=float(p[4]), deck=int(p[5]),
                           vc=int(p[8]), vdec=int(p[9]), vseek=int(p[10]), vconv=int(p[11]), vadv=float(p[12]),
                           vdecms=float(p[13]), vseekms=float(p[14]), vconvms=float(p[15]), vupl=int(p[16]),
                           vuplms=float(p[17]), vflip=float(p[18]), vlock=float(p[19]), en=int(p[22]), ems=float(p[23]),
                           sc=int(p[26]), su=int(p[27]), sms=float(p[28]), suplms=float(p[29]), pump=float(p[32])))
        except (IndexError, ValueError):
            pass
    elif ln.startswith('[GV] '):
        p = ln.split()
        try:
            GV.append(dict(t=float(p[1]), clip=int(p[2]), size=p[3], decode=int(p[4]), adv=float(p[5]), ndec=int(p[6]),
                           nseek=int(p[7]), decms=float(p[8]), seekms=float(p[9]), convms=float(p[10]),
                           flipms=float(p[11]), nupl=int(p[12]), uplms=float(p[13]), ph=float(p[14])))
        except (IndexError, ValueError):
            pass
    elif ln.startswith('[HB] '):
        p = ln.split(); HB.append((float(p[1]), float(p[2])))
    elif ln.startswith('[DG] ') and not ln.startswith('[DG] heartbeat'):
        m = re.match(r'\[DG\] ([\d.\-]+) (\d+) (\S+) ([\d.]+)$', ln)
        if m: DG.append((float(m[1]), int(m[2]), m[3], float(m[4])))
    elif ln.startswith('[CP] '):
        p = ln.split(); CP.append((float(p[1]), float(p[2]), int(p[3]), float(p[4]), int(p[5])))
    elif ln.startswith('[EV] ') and ' hook mark ' in ln:
        p = ln.split(); MK.append((float(p[1]), int(p[4])))
END = max([g['t'] for g in GF] + [0]) + 1


def q(v, f):
    if not v: return None
    v = sorted(v); i = min(len(v) - 1, max(0, int(round(f * (len(v) - 1)))))
    return v[i]


def s3(v):
    return None if not v else dict(n=len(v), med=round(q(v, .5), 3), p90=round(q(v, .9), 3), max=round(max(v), 3),
                                    mean=round(sum(v) / len(v), 3))


def window(i):
    t0 = MK[i][0]
    t1 = MK[i + 1][0] if i + 1 < len(MK) else END
    return t0, t1


def phase_stats(t0, t1, settle=0.0):
    a = t0 + settle
    gf = [g for g in GF if a <= g['t'] < t1]
    out = {}
    if gf:
        starts = [g['t'] for g in gf]
        iv = [b - a_ for a_, b in zip(starts, starts[1:])]
        dur_s = (starts[-1] - starts[0]) / 1000.0 if len(starts) > 1 else 0
        out['gl'] = dict(frames=len(gf), fps=round((len(gf) - 1) / dur_s, 1) if dur_s > 0 else None,
                         cb=s3([g['cb'] for g in gf]), interval=s3(iv),
                         iv_gt_20=sum(1 for x in iv if x > 20.0), iv_gt_34=sum(1 for x in iv if x > 34.0),
                         cb_gt_8=sum(1 for g in gf if g['cb'] > 8.0), cb_gt_16=sum(1 for g in gf if g['cb'] > 16.67),
                         nodeck=sum(1 for g in gf if g['deck'] == 0),
                         comp=s3([g['comp'] for g in gf if g['comp'] >= 0]),
                         vid_per_frame=s3([g['vadv'] for g in gf if g['vc'] > 0]),
                         vupl_per_frame=s3([g['vuplms'] for g in gf if g['vupl'] > 0]),
                         vlock=s3([g['vlock'] for g in gf if g['vc'] > 0]),
                         exists_per_frame_n=s3([g['en'] for g in gf]), exists_per_frame_ms=s3([g['ems'] for g in gf if g['en'] > 0]),
                         exists_us_each=s3([1000 * g['ems'] / g['en'] for g in gf if g['en'] > 0]),
                         exists_ms_per_s=round(sum(g['ems'] for g in gf) / dur_s, 3) if dur_s > 0 else None,
                         seq_ms=s3([g['sms'] for g in gf if g['sc'] > 0]), seq_upl_n=sum(g['su'] for g in gf),
                         seq_uplms_each=s3([g['suplms'] / g['su'] for g in gf if g['su'] > 0]),
                         pump=s3([g['pump'] for g in gf]))
    gv = [g for g in GV if a <= g['t'] < t1 and g['decode'] == 1]
    if gv:
        dec = [g for g in gv if g['ndec'] > 0]
        out['video'] = dict(calls=len(gv), decoding_calls=len(dec),
                            adv_all=s3([g['adv'] for g in gv]), adv_decoding=s3([g['adv'] for g in dec]),
                            dec_ms_each=s3([g['decms'] / g['ndec'] for g in dec]),
                            ndec_per_call=s3([g['ndec'] for g in dec]), seeks=sum(g['nseek'] for g in gv),
                            conv_ms=s3([g['convms'] for g in dec if g['convms'] > 0]),
                            flip_ms=s3([g['flipms'] for g in dec if g['flipms'] > 0]),
                            upl_ms=s3([g['uplms'] for g in gv if g['nupl'] > 0]), upl_calls=sum(g['nupl'] for g in gv),
                            calls_ndec_ge5=sum(1 for g in dec if g['ndec'] >= 5),
                            playhead_first_last=(round(gv[0]['ph'], 3), round(gv[-1]['ph'], 3)),
                            sizes=sorted(set(g['size'] for g in gv)))
    hb = [(p, l) for p, l in HB if a <= p < t1]
    dur = (t1 - a) / 1000.0
    out['msg'] = dict(hb_n=len(hb), hb_max=round(max([l for _, l in hb] or [0]), 3),
                      hb_gt_16=sum(1 for _, l in hb if l > 16.67), hb_gt_50=sum(1 for _, l in hb if l > 50),
                      hb_lat=s3([l for _, l in hb]), hb_sum_ms_per_s=round(sum(l for _, l in hb) / dur, 2) if dur > 0 else None)
    top = [d for d in DG if a <= d[0] < t1 and d[1] == 0]
    agg = defaultdict(list)
    for d in DG:
        if a <= d[0] < t1:
            agg[d[2]].append(d[3])
    out['scopes'] = {k: s3(v) for k, v in sorted(agg.items())}
    out['top_scopes'] = [(round(d[0], 1), d[2], round(d[3], 3)) for d in top][:40]
    im = [x for x in IM if a <= x[0] < t1]
    if im:
        out['imgdecode'] = dict(n=len(im), seq_n=sum(1 for x in im if x[4]), decode=s3([x[2] for x in im]),
                                convert=s3([x[3] for x in im]), mp=sorted(set(round(x[1] / 1e6, 2) for x in im)))
    cp = [c for c in CP if a <= c[0] < t1]
    if cp:
        out['paint'] = dict(n=len(cp), per_s=round(len(cp) / dur, 1) if dur > 0 else None, ms=s3([c[1] for c in cp]),
                            stats=sum(c[2] for c in cp), stats_per_s=round(sum(c[2] for c in cp) / dur, 1) if dur > 0 else None,
                            stat_us_each=s3([1000 * c[3] / c[2] for c in cp if c[2] > 0]),
                            stat_ms_per_s=round(sum(c[3] for c in cp) / dur, 3) if dur > 0 else None,
                            paint_ms_per_s=round(sum(c[1] for c in cp) / dur, 3) if dur > 0 else None,
                            media_cells_painted=sum(1 for c in cp if c[4] in (1, 2)))
    return out


res = {'run': run, 'phases': []}
for i, (t, idx) in enumerate(MK):
    t0, t1 = window(i)
    name = phases[idx] if idx < len(phases) else str(idx)
    res['phases'].append(dict(i=idx, name=name, t0=t0, t1=t1, all=phase_stats(t0, t1),
                              steady=phase_stats(t0, t1, settle=1000.0)))
# pre-mark window = startup
json.dump(res, open(run + '/analysis.json', 'w'), indent=1)
if '--quiet' not in sys.argv:
    for ph in res['phases']:
        a = ph['all']; s = ph['steady']
        gl = s.get('gl', {}); v = s.get('video', {}); m = a['msg']
        print('%-22s fps %-6s cb med/p90/max %s  iv>20 %s  cb>8 %s  nodeck %s | vid adv(dec) %s ndec %s seeks %s | hbmax(all) %s hb>16 %s | ex/frame %s' % (
            ph['name'], gl.get('fps'), (gl.get('cb') or {}).get('med'), gl.get('iv_gt_20'), gl.get('cb_gt_8'), a.get('gl', {}).get('nodeck'),
            (v.get('adv_decoding') or {}).get('med'), (v.get('ndec_per_call') or {}).get('max'), v.get('seeks'),
            m['hb_max'], m['hb_gt_16'], (gl.get('exists_per_frame_n') or {}).get('med')))
