#!/bin/bash
# G0 of lane "outputs" (Harmony's, on main, no code): M1 oa_publish_rate, M2 GET /api/syphon.
# Test mode, set_output_tap, NEVER an Output window. Quits only the pid it started (lock helper).
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad
MAIN=/Users/boriskarpman/projects/RealTimeAudio
APP=$MAIN/build/AudioDNA_artefacts/Release/Audio-DNA.app
PY=$MAIN/.venv/bin/python
OUT=$SP/g0/run
SET="$HOME/Library/Audio-DNA/settings.json"
LANE=h-g0
. $SP/lib/lock.sh
mkdir -p $OUT
: > $OUT/app-out.log; : > $OUT/app-err.log
echo "$(date '+%F %T') G0 start; app binary sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
echo "settings sha before: $(shasum -a 256 "$SET" | cut -d' ' -f1)"
acquire_lock
rc=$?
echo "acquire_lock rc=$rc"
[ $rc -ne 0 ] && exit 1
start_app $APP $OUT test
rc=$?
echo "start_app rc=$rc"
if [ $rc -ne 0 ]; then release_lock; exit 1; fi
$PY - <<'EOF'
import json, time, urllib.request, Quartz
def req(url, data=None):
    r = urllib.request.Request(url, data=(json.dumps(data).encode() if data is not None else None),
                               headers={'Connection': 'close', 'Content-Type': 'application/json'},
                               method=('POST' if data is not None else 'GET'))
    try:
        with urllib.request.urlopen(r, timeout=5) as f:
            return f.status, f.read().decode('utf-8', 'replace')
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode('utf-8', 'replace')
    except Exception as e:
        return -1, repr(e)
T = 'http://localhost:8080'
P = 'http://127.0.0.1:7070'
print('health 8080:', req(T + '/api/health'))
# displays (Quartz) and where the app's window is
err, ids, n = Quartz.CGGetActiveDisplayList(16, None, None)
disp = []
for d in ids[:n]:
    b = Quartz.CGDisplayBounds(d)
    m = Quartz.CGDisplayCopyDisplayMode(d)
    disp.append(dict(id=int(d), builtin=bool(Quartz.CGDisplayIsBuiltin(d)), main=bool(Quartz.CGDisplayIsMain(d)),
                     x=b.origin.x, y=b.origin.y, w=b.size.width, h=b.size.height,
                     hz=Quartz.CGDisplayModeGetRefreshRate(m)))
for d in disp: print('display', d)
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
for w in wl:
    if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', '')):
        b = w.get('kCGWindowBounds', {})
        if b.get('Width', 0) < 200: continue
        cx, cy = b['X'] + b['Width'] / 2, b['Y'] + b['Height'] / 2
        on = [d for d in disp if d['x'] <= cx < d['x'] + d['w'] and d['y'] <= cy < d['y'] + d['h']]
        print('app window', repr(str(w.get('kCGWindowName', ''))), dict(b), 'onscreen', w.get('kCGWindowIsOnscreen'),
              'centre on display', [(d['id'], 'builtin' if d['builtin'] else 'external', d['hz']) for d in on])
def outputs():
    st, body = req(T + '/api/state')
    j = json.loads(body)
    o = j.get('outputs', {})
    return o, j
o, j = outputs()
print('state.outputs BEFORE tap:', {k: o.get(k) for k in ('live', 'tap', 'frame_gen', 'frame_serial', 'canvas_w', 'canvas_h')}, 'fps', j.get('fps'))
print('displays per app:', [(d.get('index'), d.get('label'), d.get('w'), d.get('h'), d.get('main'), d.get('live')) for d in o.get('displays', [])])
print('set_output_tap on:', req(T + '/api/set_output_tap', {'enabled': True}))
time.sleep(3)
for i in range(3):
    o1, j1 = outputs(); t1 = time.monotonic()
    time.sleep(10)
    o2, j2 = outputs(); t2 = time.monotonic()
    ds = o2['frame_serial'] - o1['frame_serial']
    print('oa_publish_rate INFO window %d: serial %d -> %d in %.3f s  publish_hz=%.2f  (tap %s live %s fps %s canvas %sx%s)' % (
        i + 1, o1['frame_serial'], o2['frame_serial'], t2 - t1, ds / (t2 - t1), o2.get('tap'), o2.get('live'), j2.get('fps'), o2.get('canvas_w'), o2.get('canvas_h')))
print('M2 GET 8080 /api/syphon:', req(T + '/api/syphon'))
print('M2 GET 7070 /api/syphon:', req(P + '/api/syphon'))
print('set_output_tap off:', req(T + '/api/set_output_tap', {'enabled': False}))
time.sleep(1)
o, j = outputs()
print('state.outputs AFTER:', {k: o.get(k) for k in ('live', 'tap', 'frame_gen', 'frame_serial')})
EOF
rc=$?
echo "python rc=$rc"
outwins
quit_app
rc=$?
echo "quit_app rc=$rc"
sleep 16
$PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a=[w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', ''))]
u=[w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]
print('after quit +16 s: audio-dna windows %d, Output-named %d, UserNotificationCenter windows %d' % (len(a), len([w for w in a if 'Output' in str(w.get('kCGWindowName', ''))]), len(u)))"
echo "adna after: [$(adna | tr '\n' ' ')]"
release_lock
echo "settings sha after:  $(shasum -a 256 "$SET" | cut -d' ' -f1)"
echo "app-err.log bytes: $(wc -c < $OUT/app-err.log)"
echo "$(date '+%F %T') G0 end"
