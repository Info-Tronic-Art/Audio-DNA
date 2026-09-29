# seekstats.py -- the catch-up / re-seek loop (VideoPlayer::decodeFrameAtTime) per k0_* launch: contiguous runs of
# render-thread video calls that decode >= 5 frames, their duration, call count, per-call ms, and the phase they start in;
# plus the message thread's videoPlayerMutex_ waits (rd.getVideoPlayer.lockwait).
import glob, json, re, statistics as st
SP = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media'
rows, waits = [], []
import os
for run in sorted(glob.glob(SP + '/runs/k0_[0-9]*')):
    if os.path.exists(run + '/compiler-samples.txt') and os.path.getsize(run + '/compiler-samples.txt') > 0:
        continue   # a compiler ran during the launch: not a perf number
    try:
        phases = json.load(open(run + '/drive-log.json'))['phases']
    except Exception:
        continue
    lines = open(run + '/app-err.log', errors='replace').read().splitlines()
    marks = [(float(l.split()[1]), int(l.split()[4])) for l in lines if l.startswith('[EV] ') and ' hook mark ' in l]
    def phase_at(t):
        name = '?'
        for mt, i in marks:
            if mt <= t: name = phases[i]
        return name
    gv = [l.split() for l in lines if l.startswith('[GV] ')]
    cur = None
    for p in gv:
        t, size, ndec, adv, ph = float(p[1]), p[3], int(p[6]), float(p[5]), float(p[14])
        if ndec >= 5:
            if cur is None:
                cur = dict(run=run.split('/')[-1], start=t, phase=phase_at(t), size=size, calls=0, ms=[], ph0=ph)
            cur['calls'] += 1; cur['ms'].append(adv); cur['end'] = t; cur['ph1'] = ph
        elif cur is not None:
            rows.append(cur); cur = None
    if cur is not None:
        rows.append(cur)
    for l in lines:
        m = re.match(r'\[DG\] ([\d.]+) \d+ rd.getVideoPlayer.lockwait ([\d.]+)', l)
        if m:
            waits.append((run.split('/')[-1], phase_at(float(m[1])), float(m[2])))
agg = {}
for r in rows:
    key = (r['phase'].split('_')[0] + '_' + '_'.join(r['phase'].split('_')[1:]), r['size'])
    agg.setdefault(key, []).append(r)
out = {}
for (ph, size), rs in sorted(agg.items()):
    d = dict(n_events=len(rs), dur_ms=[round(r['end'] - r['start'], 0) for r in rs],
             calls=[r['calls'] for r in rs], call_ms_med=[round(st.median(r['ms']), 1) for r in rs],
             call_ms_max=[round(max(r['ms']), 1) for r in rs], playhead=[(round(r['ph0'], 3), round(r['ph1'], 3)) for r in rs])
    out[ph + ' ' + size] = d
    print(ph, size, json.dumps(d))
print('lockwaits', json.dumps(waits))
json.dump({'events': out, 'lockwaits': waits}, open(SP + '/runs/seekstats.json', 'w'), indent=1)
