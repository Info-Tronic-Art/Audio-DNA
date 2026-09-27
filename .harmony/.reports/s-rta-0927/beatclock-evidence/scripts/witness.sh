#!/bin/bash
# Tempo witness (take.json 1.5 s after Record: tempoMap start anchor bpm 120) + verify item (d) (clockBeat deltas on
# /api/routine/status are hop-quantised: multiples of 512*120/(60*48000) = 0.021333 beat at 120 BPM).
# $1 = app bundle, $2 = out dir. Caller holds the live lock. open -g only; graceful quit.
APP="$1"; OUT="$2"; mkdir -p "$OUT"
[ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "beatclock" ] || { echo "REFUSE: lock not held by beatclock"; exit 64; }
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
[ -n "$(adna_pids)" ] && { echo "REFUSE: Audio-DNA already running"; exit 64; }
A=http://127.0.0.1:7070
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 $A/api/health)" ] && break; sleep 1; done
NAME="beatclock-witness-$(date +%Y%m%d-%H%M%S)"
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python - "$NAME" <<'PY'
import json, sys, time, os, requests
A = 'http://127.0.0.1:7070'
S = requests.Session(); S.headers['Connection'] = 'close'
name = sys.argv[1]
S.post(A + '/api/set_bpm', json={'bpm': 120}, timeout=5)
time.sleep(1.5)
# (d) hop quantisation of clockBeat deltas
inc = 512.0 * 120.0 / (60.0 * 48000.0)
vals = []
end = time.time() + 3.0
while time.time() < end:
    vals.append(S.get(A + '/api/routine/status', timeout=3).json().get('clockBeat'))
    time.sleep(0.03)
d = [b - a for a, b in zip(vals, vals[1:]) if b != a]
off = [abs(x / inc - round(x / inc)) * inc for x in d]
worst = max(off) if off else None
print(('PASS' if off and worst < 2e-3 else 'FAIL') + '  (d) %d non-zero clockBeat deltas over 3 s, each a multiple of one hop (%.6f beat): worst residue %s beat, min delta %.4f, max delta %.4f'
      % (len(d), inc, None if worst is None else '%.2e' % worst, min(d) if d else 0, max(d) if d else 0))
# tempo witness
r = S.post(A + '/api/perf/record', json={'name': name, 'audio': False}, timeout=5).json()
time.sleep(1.5)
folder = os.path.expanduser('~/Documents/Audio-DNA/Takes/%s.adna-take' % name)
try:
    tm = json.load(open(folder + '/take.json')).get('tempoMap', [])
except Exception as e:
    tm = 'unreadable: %s' % e
ok = isinstance(tm, list) and tm and tm[0].get('why') == 'start' and abs((tm[0].get('bpm') or 0) - 120.0) < 0.01
print(('PASS' if ok else 'FAIL') + '  tempo witness: take.json 1.5 s after Record -> tempoMap %s' % json.dumps(tm)[:300])
S.post(A + '/api/perf/stop', json={}, timeout=5)
time.sleep(1.0)
try:
    tm2 = json.load(open(folder + '/take.json')).get('tempoMap', [])
    print('(after stop: %d anchor(s): %s)' % (len(tm2), [a.get('why') for a in tm2]))
except Exception as e:
    print('(after stop: take.json unreadable: %s)' % e)
print('(witness take: %s)' % folder)
PY
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do [ -z "$(adna_pids)" ] && break; sleep 1; done
[ -n "$(adna_pids)" ] && { echo "pkill last resort"; kill $(adna_pids); sleep 3; }
[ -z "$(adna_pids)" ] && echo "PASS  app terminated" || echo "FAIL  app still running"
W="$(/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', '')) and 'Output' in str(w.get('kCGWindowName', ''))]))")"
[ "$W" = "0" ] && echo "PASS  0 Audio-DNA Output windows (Quartz list)" || echo "FAIL  Output windows: $W"
