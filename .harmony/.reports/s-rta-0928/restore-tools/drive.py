# drive.py OUTDIR ROOT ARMS REPS -- a minimal routine start/restore driver over the production REST API (7070)
import json, os, sys, time, urllib.request
A = 'http://127.0.0.1:7070'
OUT, ROOT, ARMS, REPS = sys.argv[1], sys.argv[2], sys.argv[3].split(','), int(sys.argv[4])
log = []
def ev(what, **kw):
    kw.update(t=time.time(), what=what); log.append(kw); print(json.dumps(kw), flush=True)
def req(path, body=None, timeout=6):
    data = None if body is None else json.dumps(body).encode()
    r = urllib.request.Request(A + path, data=data, method='GET' if body is None else 'POST',
                               headers={'Content-Type': 'application/json', 'Connection': 'close'})
    try:
        with urllib.request.urlopen(r, timeout=timeout) as f:
            return json.loads(f.read().decode())
    except Exception as e:
        return {'ok': False, 'error': str(e)}
P = lambda p, b: req(p, b)
G = lambda p: req(p)
def wait_state(slot, want, to):
    end = time.time() + to
    while time.time() < end:
        s = G('/api/routine/status')
        try:
            if s['bank'][slot]['state'] == want: return time.time()
        except Exception: pass
        time.sleep(0.02)
    return None
def perturb():
    P('/api/trigger_clip', {'layer': 0, 'column': 1})
    P('/api/set_layer_opacity', {'layer': 0, 'opacity': 0.1})
    P('/api/set_layer_opacity', {'layer': 1, 'opacity': 0.6})
    P('/api/set_param', {'layer': 0, 'column': 1, 'effect': 'Brightness', 'param': 'amount', 'value': 0.2})
    time.sleep(1.0)
def edge_wait():
    # wait until early in a bar (beatInBar 0, phase < 0.5) so an Ease restore has its whole last beat
    end = time.time() + 4.0
    while time.time() < end:
        d = G('/api/bpm')
        try:
            if d['beatInBar'] == 0 and d['beatPhase'] < 0.5: return True
        except Exception: pass
        time.sleep(0.01)
    return False

# --- setup ---
fixture = os.path.join(OUT, 'fixture.json')
FIXVAR = os.environ.get('FIXVAR', 'card')
fx = json.loads(open(ROOT + '/.harmony/probe-routines.json').read().replace('@ROOT@', ROOT))
card = fx['decks'][0]['layers'][0]['clips'][0]
if FIXVAR == 'big':
    card['mediaFile'] = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/big4k.jpg'
elif FIXVAR == 'source':
    for k in ('mediaFile',): card.pop(k, None)
    card['mediaType'] = 4; card['sourceType'] = 'solid_color'
    card['sourceParams'] = [{'name': 'Red', 'uniform': 'u_src_red', 'value': 0.2, 'default': 1.0},
                            {'name': 'Green', 'uniform': 'u_src_green', 'value': 0.6, 'default': 1.0},
                            {'name': 'Blue', 'uniform': 'u_src_blue', 'value': 0.9, 'default': 1.0}]
if FIXVAR == 'many':
    import copy
    dk = fx['decks'][0]; dk['numColumns'] = 4
    L0, L1 = dk['layers'][0]['clips'], dk['layers'][1]['clips']
    nid = 10
    for lst in (L0, L1):
        while len(lst) < 4: lst.append(None)
    for lst, cols in ((L0, (2, 3)), (L1, (2, 3))):
        for c in cols:
            cc = copy.deepcopy(card); cc['id'] = nid; cc['name'] = 'card%d' % nid; nid += 1
            lst[c] = cc
json.dump(fx, open(fixture, 'w'), indent=1)
ev('setup.bpm', r=P('/api/set_bpm', {'bpm': 120}))
time.sleep(1)
ev('setup.load', r=P('/api/load_composition', {'path': fixture}))
time.sleep(1)
P('/api/trigger_clip', {'layer': 0, 'column': 0}); time.sleep(1)
name = 'restorediag-%s-%d' % (time.strftime('%H%M%S'), os.getpid())
time.sleep(1.8)
ev('record.start', r=P('/api/perf/record', {'name': name, 'audio': False}))
T = time.time()
sched = [(0.6, '/api/set_layer_opacity', {'layer': 0, 'opacity': 0.5}),
         (1.6, '/api/set_layer_opacity', {'layer': 1, 'opacity': 0.3}),
         (2.6, '/api/trigger_clip', {'layer': 0, 'column': 1}),
         (4.6, '/api/set_param', {'layer': 0, 'column': 1, 'effect': 'Brightness', 'param': 'amount', 'value': 0.9}),
         (6.6, '/api/set_layer_opacity', {'layer': 0, 'opacity': 0.9})]
sched += [(9.0 + 0.1 * i, '/api/set_layer_opacity', {'layer': 0, 'opacity': round(0.2 + 0.02 * i, 4)}) for i in range(31)]
sched += [(13.0, '/api/perf/stop', {})]
for off, path, body in sched:
    dt = T + off - time.time()
    if dt > 0: time.sleep(dt)
    P(path, body)
time.sleep(1.5)
folder = os.path.expanduser('~/Documents/Audio-DNA/Takes/%s.adna-take' % name)
ev('take', folder=folder, exists=os.path.exists(folder + '/take.json'))
P('/api/perf/load', {'folder': folder}); time.sleep(1)
ev('save0', r=P('/api/routine/save', {'name': 'Probe Routine', 'fromBeat': 0, 'toBeat': 16, 'slot': 0}))
time.sleep(0.5)
ev('save1', r=P('/api/routine/save', {'name': 'Probe Ramp', 'fromBeat': 16, 'toBeat': 32, 'slot': 1}))
time.sleep(0.5)
st = G('/api/routine/status')
ev('bank', b0={k: st['bank'][0].get(k) for k in ('name', 'lanes', 'preambleEntries', 'lengthBeats')})

# --- idle baseline ---
IDLE = float(os.environ.get('IDLE', '6'))
ev('idle.begin'); time.sleep(IDLE); ev('idle.end')

def once(arm, rep):
    style = {'ease': 'ease', 'jump': 'jump', 'norestore': 'ease'}[arm]
    P('/api/routine/set', {'slot': 0, 'loop': False, 'restoreState': arm != 'norestore', 'restoreStyle': style})
    time.sleep(0.3)
    perturb()
    edge_wait()
    ev('fire', arm=arm, rep=rep)
    P('/api/routine/fire', {'slot': 0})
    t = wait_state(0, 'running', 3.0)
    ev('running', arm=arm, rep=rep, seen=t)
    time.sleep(1.5)
    P('/api/routine/stop', {'all': True})
    ev('stopped', arm=arm, rep=rep)
    time.sleep(0.8)

def loop(arm, cycles):
    style = 'jump' if arm == 'loopjump' else 'ease'
    P('/api/routine/set', {'slot': 0, 'loop': True, 'restoreState': True, 'restoreStyle': style})
    time.sleep(0.3)
    perturb()
    edge_wait()
    ev('fire', arm=arm)
    P('/api/routine/fire', {'slot': 0})
    wait_state(0, 'running', 3.0)
    ev('running', arm=arm)
    time.sleep(8.0 * cycles + 0.6)
    P('/api/routine/stop', {'all': True})
    ev('stopped', arm=arm); time.sleep(0.8)

def snap():
    row = {'t': time.time()}
    c = G('/api/composition')
    try:
        row['op0'] = c['decks'][0]['layers'][0]['opacity']
    except Exception: pass
    s = G('/api/routine/status')
    try:
        row['bank'] = [{k: b.get(k) for k in ('state', 'position', 'cycle', 'yielded', 'glides')} for b in s['bank'][:2]]
        row['cb'] = s.get('clockBeat')
    except Exception: pass
    return row

def stack(arm, rep):
    stall = int(arm[len('stackstall'):]) if arm.startswith('stackstall') else 0
    for sl in (0, 1):
        P('/api/routine/set', {'slot': sl, 'loop': False, 'restoreState': True, 'restoreStyle': 'ease'})
    P('/api/set_layer_opacity', {'layer': 0, 'opacity': 0.1})
    time.sleep(1.0)
    ev('fire1', arm=arm, rep=rep)
    P('/api/routine/fire', {'slot': 1})
    T5 = wait_state(1, 'running', 3.0)
    ev('ramp.running', arm=arm, rep=rep, seen=T5)
    ev('fire0', arm=arm, rep=rep)
    P('/api/routine/fire', {'slot': 0})
    out, stalled = [], None
    end = (T5 or time.time()) + 5.2
    while time.time() < end:
        r = snap(); out.append(r)
        try:
            b0 = r['bank'][0]
            if stall and stalled is None and b0['state'] == 'running' and 0.9 <= b0['position'] < 1.1:
                stalled = {'t': time.time(), 'pos': b0['position'], 'r': P('/api/debug/stall_message_thread', {'ms': stall})}
                ev('stall', arm=arm, rep=rep, **stalled)
        except Exception: pass
        time.sleep(0.02)
    json.dump(out, open(os.path.join(OUT, 'stack-%s-%d.json' % (arm, rep)), 'w'))
    run = [(r['bank'][0]['position'], r.get('op0')) for r in out if r.get('bank') and r['bank'][0]['state'] == 'running' and r['bank'][0].get('cycle') == 1]
    win = [v for p, v in run if 1.8 <= p <= 5.0]
    halves = [v for v in win if isinstance(v, (int, float)) and abs(v - 0.5) <= 0.03]
    jumps = [(round(a[0], 3), round(b[0], 3)) for a, b in zip(run, run[1:]) if b[0] - a[0] > 0.3]
    ev('stack.verdict', arm=arm, rep=rep, n=len(win), n05=len(halves), anomaly=(len(win) > 0 and len(halves) == 0),
       vals=sorted(set(round(v, 3) for v in win if isinstance(v, (int, float)))), posJumps=jumps)
    P('/api/routine/stop', {'all': True})
    time.sleep(0.8)

for arm in ARMS:
    if arm.startswith('stack'):
        for rep in range(REPS):
            stack(arm, rep)
    elif arm.startswith('loop'):
        loop(arm, REPS)
    else:
        for rep in range(REPS):
            once(arm, rep)
json.dump(log, open(os.path.join(OUT, 'drive-log.json'), 'w'), indent=1)
ev('done', take=folder)
