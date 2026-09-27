# stop-witness.py -- routines-followup ITEM 1 live witness (scratch; drives the SCRATCH hook in perfRoutineSet)
import json, os, sys, time, urllib.request, urllib.error
A = 'http://127.0.0.1:7070'
PASS = FAIL = 0
def ok(c, t):
    global PASS, FAIL
    if c: PASS += 1; print('PASS  ' + t)
    else: FAIL += 1; print('FAIL  ' + t)
def get(p):
    try:
        with urllib.request.urlopen(A + p, timeout=3) as r: return json.loads(r.read().decode())
    except Exception as e: return {'ok': False, 'error': str(e)}
def post(p, b):
    req = urllib.request.Request(A + p, data=json.dumps(b).encode(), headers={'Content-Type': 'application/json'}, method='POST')
    try:
        with urllib.request.urlopen(req, timeout=6) as r: return json.loads(r.read().decode())
    except Exception as e: return {'ok': False, 'error': str(e)}
def wait_state(slot, want, t):
    end = time.time() + t
    while time.time() < end:
        try:
            if get('/api/routine/status')['bank'][slot]['state'] == want: return True
        except Exception: pass
        time.sleep(0.03)
    return False
def clips():
    c = get('/api/composition'); out = []
    for li, L in enumerate(c['decks'][0]['layers']):
        col = L['activeClipColumn']; pl = None
        for cl in L.get('clips', []):
            if cl.get('column') == col: pl = cl.get('playing')
        out.append((li, col, pl))
    return out
def points(): return get('/api/perf/status').get('points')
def state(): return get('/api/routine/status')['bank'][0]['state']

fixture, take, name = sys.argv[1], sys.argv[2], sys.argv[3]
post('/api/set_bpm', {'bpm': 120}); time.sleep(1)
ok(post('/api/load_composition', {'path': fixture}).get('ok') is True, 'load_composition accepted the fixture'); time.sleep(1)
post('/api/trigger_clip', {'layer': 0, 'column': 0}); post('/api/trigger_clip', {'layer': 1, 'column': 0}); time.sleep(1)
post('/api/perf/load', {'folder': take}); time.sleep(1)
post('/api/routine/save', {'name': 'Witness', 'fromBeat': 0, 'toBeat': 16, 'slot': 0, 'loop': True}); time.sleep(1)
ok(get('/api/routine/status')['bank'][0]['name'] == 'Witness', 'routine Witness saved on pad 1')
c0 = clips(); print('(clips before: %s)' % c0)
ok(all(pl is True for _, col, pl in c0 if col >= 0) and len([1 for _, col, _ in c0 if col >= 0]) == 2, 'both layers have an active clip that is playing')
r = post('/api/perf/record', {'name': name, 'audio': False}); ok(r.get('ok') is True, 'recording armed (%s)' % r)
time.sleep(0.6)

# --- the TopBar Stop button's handler (topBar_->onStop) ---
post('/api/routine/fire', {'slot': 0})
ok(wait_state(0, 'running', 2.6), 'button: the routine is running before the Stop')
time.sleep(0.15)
b0 = clips(); p0 = points()
post('/api/routine/set', {'slot': 0, 'name': '__scratch_stop_button'}); time.sleep(0.4)
b1 = clips(); p1 = points(); s1 = state()
print('(button: clips before %s after %s; recorded points %s -> %s; state %s)' % (b0, b1, p0, p1, s1))
ok(s1 == 'idle', 'button: Stop stops the routine (state %s)' % s1)
ok(all(a[2] is True and b[2] is True and a[1] == b[1] for a, b in zip(b0, b1)), 'button: no clip is stopped -- every active clip still playing on the same column (%s)' % b1)
ok(p1 == p0, 'button: nothing captured into the recording (points %s -> %s)' % (p0, p1))

# --- the GlobalStop key/MIDI binding ("Stop" in the bind overlay) ---
post('/api/routine/fire', {'slot': 0})
ok(wait_state(0, 'running', 2.6), 'binding: the routine is running before the Stop')
time.sleep(0.15)
g0 = clips(); q0 = points()
post('/api/routine/set', {'slot': 0, 'name': '__scratch_stop_binding'}); time.sleep(0.4)
g1 = clips(); q1 = points(); s2 = state()
print('(binding: clips before %s after %s; recorded points %s -> %s; state %s)' % (g0, g1, q0, q1, s2))
ok(s2 == 'idle', 'binding: Stop stops the routine (state %s)' % s2)
ok(all(a[1] == b[1] and a[2] == b[2] for a, b in zip(g0, g1)), 'binding: no clip changes (%s)' % g1)
ok(q1 == q0, 'binding: no audio-transport stop captured into the recording (points %s -> %s)' % (q0, q1))

post('/api/perf/stop', {}); time.sleep(1.5)
print('\n%d PASS / %d FAIL' % (PASS, FAIL))
