# idle.py OUTDIR FIXTURE [IDLE_S] [SETTLE_S] -- set a fixture up over the production REST API (7070), settle, then
# mark an idle window (no request is sent during it). Window stamps use CLOCK_UPTIME_RAW ms = libc++ steady_clock on Apple.
import json, os, sys, time, urllib.request
A = 'http://127.0.0.1:7070'
OUT, FIX = sys.argv[1], sys.argv[2]
IDLE = float(sys.argv[3]) if len(sys.argv) > 3 else 30.0
SETTLE = float(sys.argv[4]) if len(sys.argv) > 4 else 6.0
ROOT = '/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-idle'
MEDIA = '/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle/media'
now = lambda: time.clock_gettime(time.CLOCK_UPTIME_RAW) * 1000.0
log = {'fixture': FIX, 'events': []}
def ev(what, **kw):
    kw.update(t=now(), wall=time.time(), what=what); log['events'].append(kw); print(json.dumps(kw), flush=True)
def req(path, body=None, timeout=10):
    data = None if body is None else json.dumps(body).encode()
    r = urllib.request.Request(A + path, data=data, method='GET' if body is None else 'POST',
                               headers={'Content-Type': 'application/json', 'Connection': 'close'})
    try:
        with urllib.request.urlopen(r, timeout=timeout) as f:
            return json.loads(f.read().decode())
    except Exception as e:
        return {'ok': False, 'error': str(e)}
def clip_img(cid, path):
    return {'name': 'img%d' % cid, 'id': cid, 'mediaType': 1, 'mediaFile': path}
if FIX in ('card', 'many16'):
    fx = json.loads(open(ROOT + '/.harmony/probe-routines.json').read().replace('@ROOT@', ROOT))
    if FIX == 'many16':
        imgs = sorted(os.path.join(MEDIA, f) for f in os.listdir(MEDIA) if f.startswith('img4k_'))
        dk = fx['decks'][0]; dk['numColumns'] = 4
        base = dk['layers'][0]
        layers = []
        cid = 100
        for li in range(4):
            L = {k: v for k, v in base.items() if k != 'clips'}
            L['name'] = 'L%d' % (li + 1); L['id'] = li
            L['clips'] = []
            for c in range(4):
                L['clips'].append(clip_img(cid, imgs[(li * 4 + c) % len(imgs)])); cid += 1
            layers.append(L)
        dk['layers'] = layers
    fixture = os.path.join(OUT, 'fixture.json')
    json.dump(fx, open(fixture, 'w'), indent=1)
    ev('setup.load', r=req('/api/load_composition', {'path': fixture}))
    time.sleep(1.5)
    nl = len(fx['decks'][0]['layers'])
    for li in range(nl):
        if fx['decks'][0]['layers'][li]['clips'][0] is not None:
            ev('setup.trigger', layer=li, r=req('/api/trigger_clip', {'layer': li, 'column': 0}))
            time.sleep(0.2)
elif FIX != 'default':
    raise SystemExit('unknown fixture ' + FIX)
ev('settle.begin'); time.sleep(SETTLE)
import subprocess
def ps_sample(tag):
    # PS_SAMPLE=1: per-thread cumulative CPU of the running app from `ps -M` (no instrumentation needed)
    if os.environ.get('PS_SAMPLE') != '1': return
    pid = subprocess.run(['pgrep', '-x', 'Audio-DNA'], capture_output=True, text=True).stdout.split()
    if pid:
        out = subprocess.run(['ps', '-M', '-p', pid[0]], capture_output=True, text=True).stdout
        open(os.path.join(OUT, 'ps-%s.txt' % tag), 'w').write('%.3f\n' % now() + out)
ev('idle.begin'); ps_sample('begin'); time.sleep(IDLE); ps_sample('end'); ev('idle.end')
json.dump(log, open(os.path.join(OUT, 'idle.json'), 'w'), indent=1)
