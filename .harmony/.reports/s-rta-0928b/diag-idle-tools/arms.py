# arms.py TAG [TAG ...] -- per arm (runs/<TAG>/r*): run anidle on every launch (cached) and print the per-arm table:
# medians across launches [min-max] of the heartbeat and main-thread numbers.
import sys, os, json, glob, statistics as st
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import anidle
S = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle'

def get(run, force=False):
    p = run + '/anidle.json'
    if force or not os.path.exists(p) or os.path.getmtime(p) < os.path.getmtime(run + '/diag.tsv'):
        r = anidle.analyze(run); json.dump(r, open(p, 'w'), indent=1); open(run + '/anidle.txt', 'w').write(anidle.fmt(r) + '\n')
    return json.load(open(p))

def md(v):
    v = [x for x in v if x is not None]
    if not v: return 'n/a'
    return '%.1f [%.1f-%.1f]' % (st.median(v), min(v), max(v))

def arm(tag, force=False):
    runs = sorted(glob.glob('%s/runs/%s/r*' % (S, tag)), key=lambda p: int(p.rsplit('r', 1)[1]))
    runs = [r for r in runs if os.path.exists(r + '/diag.tsv') and os.path.exists(r + '/idle.json')]
    rs = [get(r, force) for r in runs]
    if not rs: return None
    ph = lambda r, k: r['phase_ms_per_s'].get(k, 0.0)
    row = dict(tag=tag, n=len(rs), fixture=rs[0]['fixture'],
               lags_ge2_per_s=md([r['hb']['n_ge2_per_s'] for r in rs]),
               lag_ge2_med=md([r['hb']['ge2_med'] for r in rs]), lag_ge2_p90=md([r['hb']['ge2_p90'] for r in rs]),
               win_max_med=md([r['hb']['win_max_med'] for r in rs]), win_max_p90=md([r['hb']['win_max_p90'] for r in rs]),
               win_max_max=md([r['hb']['win_max_max'] for r in rs]), lag_ms_per_s=md([r['hb']['lag_ms_per_s'] for r in rs]),
               busy_ms_per_s=md([r['busy_ms_per_s'] for r in rs]),
               display_obs=md([ph(r, 'timers-obs') for r in rs]), sources0=md([ph(r, 'sources0') for r in rs]),
               wake_handle=md([ph(r, 'wake-handle') for r in rs]), outside_rl=md([ph(r, 'outside-runloop') for r in rs]),
               bw_obs=md([ph(r, 'bw-observers') for r in rs]),
               drawRect_ms_per_s=md([sum(x['ms_per_s'] for x in r['drawRect']) for r in rs]),
               handlePaint_ms_per_s=md([sum(x['ms_per_s'] for x in r['handlePaint']) for r in rs]),
               cpu_main=md([r.get('cpu_main_ms_per_s') for r in rs]), cpu_total=md([r.get('cpu_total_ms_per_s') for r in rs]),
               spans_ge10_per_s=md([r['spans_ge10']['per_s'] for r in rs]),
               load=md([r['load'][0] for r in rs if r['load']] + [r['load'][1] for r in rs if r['load']]),
               appState=sorted(set(x for r in rs for x in r['appState'])))
    return row

if __name__ == '__main__':
    force = '--force' in sys.argv
    tags = [a for a in sys.argv[1:] if not a.startswith('--')]
    rows = [arm(t, force) for t in tags]
    rows = [r for r in rows if r]
    keys = list(rows[0].keys()) if rows else []
    for k in keys:
        print('%-22s' % k + ''.join('%-26s' % str(r[k])[:25] for r in rows))
    json.dump(rows, open(S + '/arms-last.json', 'w'), indent=1)
