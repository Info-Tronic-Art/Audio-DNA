#!/bin/bash
# probe-routines.sh -- s-rta-0926 L-R Routines slice 1, lane 1b live gate
# (.harmony/.reports/s-rta-0926/plan-routines-s1-final.md section 6.2).
#
# Proves, over the production REST API on a live app, the whole slice-1 loop: record a take, save
# a slice of it as a routine on a pad, perturb the look, fire the routine, see it wait for the next
# bar, restore the recorded look (pixel-decoded), replay its recorded moves on the beat grid, hold
# at the end (once) or loop (restore re-fired each cycle), restart on re-fire, skip the restore in
# "start from now", and -- stacking (D9) -- let the routine whose gesture BEGAN later win for the
# rest of the earlier routine's gesture.
#
# RED on a pre-routines binary: every /api/routine/* route is 404 and /api/composition clips carry
# no "effects" block; take.json meta has no startBeatInBar; a first-activation auto-play is not a
# `playing` lane point (routines-1a carried concern (a)).
#
# Mechanics copied from probe-step3.sh (helpers, `open -g` launch, graceful osascript quit,
# Quartz 0-Output-window witness, 'MacOS/Audio-DN[A]') and probe-mastersignal.sh (pixel oracle:
# decode with PIL+numpy, non-blank first, mean-absolute-difference thresholds, never md5).
# Production mode, port 7070, NO --test-mode. No audio device needed: POST /api/set_bpm puts the
# tracker in manual LOCKED mode with a predicted phase (a bar = 2.0 s at 120 BPM) and the take is
# recorded with "audio": false. Structural resets are suppressed in that regime, so barCount and
# totalBarCount advance together -- 2 Bar / 4 Bar parity is pinned by ctest case 11, not here.
#
# Timings (120 BPM: beat 0.5 s, bar 2 s). The take (row 2), relative to Record:
#   +0.6 L0 opacity 0.5 | +1.6 L1 opacity 0.3 | +2.6 trigger L0 C1 | +4.6 C1 Brightness 0.9 |
#   +6.6 L0 opacity 0.9 | +9.0..+12.0 L0 opacity ramp 0.2 -> 0.8 (31 writes, 0.1 s apart: ONE
#   gesture, each write inside gripHoldMs) | +13.0 stop.
# Routine "Probe Routine" (pad 1, slot 0) = take beats [0, 16): the first five moves, 8 s long.
# Routine "Probe Ramp" (pad 2, slot 1) = take beats [16, 32): the ramp, at routine beats ~2..8.
#
# Environment:
#   ROUTINES_APP        full path to Audio-DNA.app (default: $ROOT/$ROUTINES_BUILD_DIR/AudioDNA_artefacts/
#                       Release/Audio-DNA.app, ROUTINES_BUILD_DIR default "build")
#   ROUTINES_PY         python with PIL + numpy (default $ROOT/.venv/bin/python)
#   ROUTINES_MAD_REST   bound for mad(ref, rest) (default 2.0 -- plan R12: Harmony pins it from a real
#                       GREEN run, never widens it to pass)
# Artifacts: a FRESH directory per run under ${1:-/tmp/audiodna-routines}. Exit 1 on any FAIL.
#
# SCREEN-SAFETY LAW: launches the real app with `open -g` only, never opens the Output window (no
# endpoint used here reaches it), never a full-screen capture; graceful osascript quit first,
# pkill only as the last resort. The caller holds the live lock (/tmp/audiodna-live.lock).
set -u
OUT_BASE="${1:-/tmp/audiodna-routines}"
OUT="$OUT_BASE/run-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROUTINES_BUILD_DIR:-build}"
APPBUNDLE="${ROUTINES_APP:-$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app}"
PXPY="${ROUTINES_PY:-$ROOT/.venv/bin/python}"
MAD_REST="${ROUTINES_MAD_REST:-2.0}"
MAD_THRESHOLD="0.5"
A='http://127.0.0.1:7070'
TEMPLATE="$ROOT/.harmony/probe-routines.json"
FIXTURE="$OUT/probe-routines.json"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
TAKE_NAME="probe-routines-$(date +%Y%m%d-%H%M%S)"
TAKE_FOLDER="$TAKES_DIR/$TAKE_NAME.adna-take"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

# rows CMD...: runs a helper that prints "ok <text>" / "no <text>" / anything else (echoed as-is).
rows(){
    local line
    while IFS= read -r line; do
        case "$line" in
            "ok "*) ok "${line#ok }" ;;
            "no "*) no "${line#no }" ;;
            *) echo "$line" ;;
        esac
    done < <("$@" 2>&1)
}

# --- helpers (one python file: sampling + analysis; stdlib only) ------------
RT="$OUT/rt.py"
cat > "$RT" <<'PY'
import json, sys, time, urllib.request, urllib.error
A = 'http://127.0.0.1:7070'

def get(path, timeout=3):
    try:
        with urllib.request.urlopen(A + path, timeout=timeout) as r:
            return json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        return {'ok': False, 'http': e.code}
    except Exception as e:
        return {'ok': False, 'error': str(e)}

def post(path, body, timeout=6):
    req = urllib.request.Request(A + path, data=json.dumps(body).encode(),
                                 headers={'Content-Type': 'application/json'}, method='POST')
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            txt = r.read().decode()
    except urllib.error.HTTPError as e:
        return {'ok': False, 'http': e.code}
    except Exception as e:
        return {'ok': False, 'error': str(e)}
    try:
        return json.loads(txt)
    except Exception:
        return {'ok': False, 'raw': txt}

def snap():
    row = {'t': time.time()}
    c = get('/api/composition')
    try:
        L = c['decks'][0]['layers']
        row['op0'] = L[0]['opacity']
        row['op1'] = L[1]['opacity'] if len(L) > 1 else None
        row['col0'] = L[0]['activeClipColumn']
        row['amt1'] = None
        for clip in L[0].get('clips', []):
            if clip.get('column') == 1 and clip.get('effects'):
                row['amt1'] = clip['effects'][0]['params'][0]['value']
    except Exception:
        pass
    s = get('/api/routine/status')
    if isinstance(s.get('bank'), list):
        row['bank'] = [{k: b.get(k) for k in ('state', 'cycle', 'restarts', 'yielded', 'preambleFired', 'position')}
                       for b in s['bank'][:2]]
    return row

def near(v, target, tol):
    return isinstance(v, (int, float)) and abs(v - target) <= tol

def state(row, slot):
    try:
        return row['bank'][slot]['state']
    except Exception:
        return None

def first(samples, pred, after):
    for r in samples:
        if r['t'] >= after and pred(r):
            return r['t']
    return None

def load(path):
    with open(path) as f:
        return json.load(f)

cmd = sys.argv[1]

if cmd == 'post':                         # post PATH JSON -> prints the response JSON
    print(json.dumps(post(sys.argv[2], json.loads(sys.argv[3]))))

elif cmd == 'get':                        # get PATH EXPR -> prints eval(EXPR) against d, or NA
    d = get(sys.argv[2])
    try:
        print(eval(sys.argv[3]))
    except Exception:
        print('NA')

elif cmd == 'sample':                     # sample SECONDS OUTFILE
    end = time.time() + float(sys.argv[2])
    out = []
    while time.time() < end:
        out.append(snap())
        time.sleep(0.03)
    with open(sys.argv[3], 'w') as f:
        json.dump(out, f)
    print('(sampled %d rows over %s s)' % (len(out), sys.argv[2]))

elif cmd == 'wait':                       # wait SLOT STATE TIMEOUT -> epoch when bank[SLOT].state == STATE, else NA
    slot, want, end = int(sys.argv[2]), sys.argv[3], time.time() + float(sys.argv[4])
    while time.time() < end:
        s = get('/api/routine/status')
        try:
            if s['bank'][slot]['state'] == want:
                print('%.3f' % time.time()); sys.exit(0)
        except Exception:
            pass
        time.sleep(0.05)
    print('NA')

elif cmd == 'record':                     # record NAME: the whole take, on a precise schedule
    name = sys.argv[2]
    r = post('/api/perf/record', {'name': name, 'audio': False})
    T = time.time()
    print(('ok' if r.get('ok') else 'no') + ' record: /api/perf/record accepted (%s)' % json.dumps(r))
    sched = [(0.6, '/api/set_layer_opacity', {'layer': 0, 'opacity': 0.5}),
             (1.6, '/api/set_layer_opacity', {'layer': 1, 'opacity': 0.3}),
             (2.6, '/api/trigger_clip', {'layer': 0, 'column': 1}),
             (4.6, '/api/set_param', {'layer': 0, 'column': 1, 'effect': 'Brightness', 'param': 'amount', 'value': 0.9}),
             (6.6, '/api/set_layer_opacity', {'layer': 0, 'opacity': 0.9})]
    sched += [(9.0 + 0.1 * i, '/api/set_layer_opacity', {'layer': 0, 'opacity': round(0.2 + 0.02 * i, 4)}) for i in range(31)]
    sched += [(13.0, '/api/perf/stop', {})]
    late = 0.0
    for off, path, body in sched:
        dt = T + off - time.time()
        if dt > 0:
            time.sleep(dt)
        late = max(late, time.time() - (T + off))
        post(path, body)
    print(('ok' if late < 0.08 else 'no') + ' record: every scheduled move went out within 80 ms of its time (worst %.3f s late)' % late)

elif cmd == 'take':                       # take FOLDER RAMPOUT: structural rows on take.json
    d = load(sys.argv[2] + '/take.json')
    lanes = d.get('lanes', [])
    # ControlPath JSON: layer / col are objects {"i": index, ...} (ControlPath::toVar).
    def idx(k, field):
        v = k.get(field)
        return v.get('i') if isinstance(v, dict) else v
    anchors = d.get('tempoMap', [])
    metered = bool(anchors) and all((a.get('bpm') or 0) > 0 for a in anchors)
    print(('ok' if metered else 'no') + ' take: tempoMap has %d anchor(s), all with a tempo (the take has a beat grid to cut)' % len(anchors))
    stamped = 0
    for ln in lanes:
        pts = ln.get('points', [])
        gs = ln.get('gestures', [])
        if (pts and all('beat' in p for p in pts)) or gs:
            stamped += 1
    print(('ok' if stamped >= 4 else 'no') + ' take: %d lanes carry beat stamps (>= 4)' % stamped)
    sbib = d.get('meta', {}).get('startBeatInBar', None)
    good = isinstance(sbib, (int, float)) and 0.0 <= sbib < 4.0
    print(('ok' if good else 'no') + ' take: meta.startBeatInBar present and in [0, 4) (%s)' % sbib)
    ramp = None
    for ln in lanes:
        k = ln.get('key', {})
        if k.get('scope') == 'layer' and k.get('control') == 'scalar' and k.get('scalar') == 'opacity' and idx(k, 'layer') == 0:
            for g in ln.get('gestures', []):
                c = g.get('curve', [])
                if len(c) >= 10 and (ramp is None or len(c) > len(ramp)):
                    ramp = c
    if ramp:
        print('ok take: the L0 opacity ramp is ONE gesture of %d breakpoints, beats %.2f..%.2f, values %.2f..%.2f'
              % (len(ramp), ramp[0]['x'], ramp[-1]['x'], ramp[0]['y'], ramp[-1]['y']))
        with open(sys.argv[3], 'w') as f:
            json.dump({'x0': ramp[0]['x'], 'x1': ramp[-1]['x'], 'y0': ramp[0]['y'], 'y1': ramp[-1]['y']}, f)
    else:
        print('no take: no L0 opacity gesture with >= 10 breakpoints (the 3 s ramp was not recorded as one gesture)')
    # Carried concern (a): switching L0 to the never-triggered, paused C1 auto-plays it -- that
    # auto-play must be a `playing` point (resume, v 1) next to the activeClip point.
    auto = [p for ln in lanes if ln.get('key', {}).get('control') == 'playing' and idx(ln.get('key', {}), 'layer') == 0
            and idx(ln.get('key', {}), 'col') == 1
            for p in ln.get('points', []) if p.get('action') == 'resume' and p.get('v') == 1]
    trig = [p for ln in lanes if ln.get('key', {}).get('control') == 'activeClip' and idx(ln.get('key', {}), 'layer') == 0
            for p in ln.get('points', []) if p.get('v') == 1]
    okA = len(auto) == 1 and len(trig) >= 1 and abs(auto[0].get('beat', -99) - trig[0].get('beat', 99)) < 0.05
    print(('ok' if okA else 'no') + ' take: the trigger of C1 recorded its auto-play as a playing/resume point at the same beat (%d point(s))' % len(auto))

elif cmd == 'grid':                       # grid T1 FILE: row 7
    T1, s = float(sys.argv[2]), load(sys.argv[3])
    marks = [('L0 opacity ~0.5', lambda r: near(r.get('op0'), 0.5, 0.05), 0.6),
             ('L0 on column 1', lambda r: r.get('col0') == 1, 2.6),
             ('C1 Brightness ~0.9', lambda r: near(r.get('amt1'), 0.9, 0.02), 4.6),
             ('L0 opacity ~0.9', lambda r: near(r.get('op0'), 0.9, 0.05), 6.6)]
    times = []
    for label, pred, at in marks:
        t = first(s, pred, T1)
        times.append(t)
        if t is None:
            print('no grid: %s never seen (expected at +%.1f s)' % (label, at))
        else:
            rel = t - T1
            print(('ok' if abs(rel - at) <= 0.35 else 'no') + ' grid: %s first at +%.2f s (expected +%.1f +/- 0.35)' % (label, rel, at))
    seen = [t for t in times if t is not None]
    print(('ok' if len(seen) == 4 and seen == sorted(seen) else 'no') + ' grid: the four moves arrive in the recorded order')
    tail = [r for r in s if r['t'] >= T1 + 8.3]
    if not tail:
        print('no grid: no samples after +8.3 s')
    else:
        r = tail[-1]
        print(('ok' if state(r, 0) == 'idle' else 'no') + ' grid: once -- state idle after the end (%s)' % state(r, 0))
        print(('ok' if near(r.get('op0'), 0.9, 0.05) and r.get('col0') == 1 else 'no')
              + ' grid: once -- the last look holds (L0 opacity %s, column %s)' % (r.get('op0'), r.get('col0')))

elif cmd == 'loop':                       # loop T2 FILE FIRED_AT_START: row 8
    T2, s, fired0 = float(sys.argv[2]), load(sys.argv[3]), int(sys.argv[4])
    pre = [r for r in s if T2 + 7.0 <= r['t'] <= T2 + 7.9]
    print(('ok' if pre and all(near(r.get('op0'), 0.9, 0.05) for r in pre) else 'no')
          + ' loop: end of cycle 1 holds L0 opacity 0.9 (%d samples)' % len(pre))
    t_back = first(s, lambda r: near(r.get('op0'), 1.0, 0.05), T2 + 7.95)
    print(('ok' if t_back is not None and t_back <= T2 + 8.6 else 'no')
          + ' loop: the restore re-fired at the loop (L0 opacity back to 1.0 at +%s s)' % (None if t_back is None else round(t_back - T2, 2)))
    t_05 = first(s, lambda r: near(r.get('op0'), 0.5, 0.05), (t_back or T2 + 8.0))
    print(('ok' if t_05 is not None and t_05 <= T2 + 9.0 else 'no')
          + ' loop: cycle 2 replays the first move (L0 opacity 0.5 at +%s s)' % (None if t_05 is None else round(t_05 - T2, 2)))
    cyc = max([r['bank'][0]['cycle'] or 0 for r in s if 'bank' in r] or [0])
    print(('ok' if cyc >= 2 else 'no') + ' loop: status cycle reached %d (>= 2)' % cyc)
    pf = max([r['bank'][0]['preambleFired'] or 0 for r in s if 'bank' in r] or [0])
    print(('ok' if pf > fired0 else 'no') + ' loop: preambleFired grew at the loop (%d -> %d)' % (fired0, pf))

elif cmd == 'hold':                       # hold VALUE FILE: after stop nothing moves L0 opacity again
    v, s = float(sys.argv[2]), load(sys.argv[3])
    t = first(s, lambda r: near(r.get('op0'), v, 0.005), 0)
    after = [r for r in s if t is not None and r['t'] >= t]
    print(('ok' if t is not None and all(near(r.get('op0'), v, 0.005) for r in after) else 'no')
          + ' stop: after Stop a hand write of %.2f sticks and nothing moves it (%d samples)' % (v, len(after)))

elif cmd == 'fromnow':                    # fromnow T4 FILE: row 10
    T4, s = float(sys.argv[2]), load(sys.argv[3])
    early = [r for r in s if T4 <= r['t'] <= T4 + 0.45]
    print(('ok' if early and all(near(r.get('op0'), 0.1, 0.01) for r in early) else 'no')
          + ' start from now: L0 opacity stays at the hand value 0.1 until the first move (%d samples)' % len(early))
    t = first(s, lambda r: near(r.get('op0'), 0.5, 0.05), T4)
    print(('ok' if t is not None and abs(t - T4 - 0.6) <= 0.35 else 'no')
          + ' start from now: the first move lands on the grid (0.5 at +%s s, expected +0.6)' % (None if t is None else round(t - T4, 2)))

elif cmd == 'stack':                      # stack T5 T6 RAMPFILE FILE: row 11
    T5, T6, ramp, s = float(sys.argv[2]), float(sys.argv[3]), load(sys.argv[4]), load(sys.argv[5])
    rs = (ramp['x0'] - 16.0) / 2.0                    # ramp start / end, seconds after the Probe Ramp start
    re = (ramp['x1'] - 16.0) / 2.0
    lo, hi = min(ramp['y0'], ramp['y1']), max(ramp['y0'], ramp['y1'])
    print('(stack: Probe Ramp started at T5; its ramp runs +%.2f..+%.2f s, %.2f..%.2f; Probe Routine started at T5+%.2f s)'
          % (rs, re, lo, hi, T6 - T5))
    before = [r for r in s if T5 + rs + 0.2 <= r['t'] <= T6 - 0.1]
    vals = [r.get('op0') for r in before]
    inside = bool(vals) and all(isinstance(v, (int, float)) and lo - 0.01 <= v <= hi + 0.01 for v in vals)
    rising = bool(vals) and all(b >= a - 0.005 for a, b in zip(vals, vals[1:])) and vals[-1] > vals[0]
    print(('ok' if inside and rising else 'no')
          + ' stack: before the second routine starts, the ramp drives L0 opacity (%d samples, %s..%s)'
          % (len(vals), vals[0] if vals else None, vals[-1] if vals else None))
    win = [r for r in s if T6 + 0.12 <= r['t'] <= T6 + 0.5]
    print(('ok' if win and all(near(r.get('op0'), 1.0, 0.03) for r in win) else 'no')
          + ' stack: the later restore wins -- L0 opacity holds 1.0 after Probe Routine starts, no ramp values (%s)'
          % sorted(set(round(r.get('op0') or -1, 3) for r in win)))
    mid = [r for r in s if T6 + 0.95 <= r['t'] <= T5 + re + 0.9]
    print(('ok' if mid and all(near(r.get('op0'), 0.5, 0.03) for r in mid) else 'no')
          + ' stack: the earlier ramp stays quiet for the rest of its gesture and after it (L0 opacity 0.5, %d samples)' % len(mid))
    y = max([r['bank'][1]['yielded'] or 0 for r in s if 'bank' in r] or [0])
    print(('ok' if y >= 1 else 'no') + ' stack: status bank[1].yielded == %d (>= 1: the displaced gesture is counted)' % y)
PY

# --- pixel oracle (probe-mastersignal.sh, 2026-09-26 md5-vs-pixel fix) ----
pixel_stats(){
    "$PXPY" -c "
try:
    from PIL import Image
    import numpy as np
    a = np.asarray(Image.open('$1').convert('RGBA'), dtype=np.float64)
    print(f'{float((a[:, :, 3] > 0).mean()):.6f} {float(a[:, :, :3].std()):.6f}')
except Exception:
    print('NA NA')
"
}
assert_nonblank(){ # FILE LABEL
    local stats af rs
    stats="$(pixel_stats "$OUT/$1")"; af="${stats% *}"; rs="${stats#* }"
    if echo "$af $rs" | grep -Eq '^[0-9.]+ [0-9.]+$' \
       && awk -v a="$af" -v s="$rs" 'BEGIN{exit !(a>=0.5 && s>2.0)}'; then
        ok "$2: $1 decodes NON-BLANK (alpha_frac=$af rgb_std=$rs)"
    else
        no "$2: $1 decodes BLANK-OR-UNDECODABLE (alpha_frac=$af rgb_std=$rs)"
    fi
}
# mean_abs_diff A B [resize]: mean absolute RGBA difference (0-255). render_frame's output size follows
# what is on screen (an image clip renders at the image's size, a source at the preview's), so two
# frames of DIFFERENT looks can differ in shape: only a "these differ" check passes "resize" (B is
# resampled to A's size first, and the sizes are logged); a "these are the same" check never does --
# a shape mismatch there is NA, a fail.
mean_abs_diff(){
    "$PXPY" -c "
try:
    from PIL import Image
    import numpy as np, sys
    ia, ib = Image.open('$1').convert('RGBA'), Image.open('$2').convert('RGBA')
    if ia.size != ib.size:
        if '${3:-}' != 'resize':
            raise ValueError('shape')
        sys.stderr.write('(frames differ in size: %s vs %s -- resampled for the difference)\n' % (ia.size, ib.size))
        ib = ib.resize(ia.size, Image.NEAREST)
    a, b = np.asarray(ia, dtype=np.float64), np.asarray(ib, dtype=np.float64)
    print(f'{float(np.abs(a - b).mean()):.6f}')
except Exception:
    print('NA')
"
}
num_gt(){ echo "$1" | grep -Eq '^[0-9]+(\.[0-9]+)?$' && awk -v v="$1" -v t="$2" 'BEGIN{exit !(v>t)}'; }
num_leq(){ echo "$1" | grep -Eq '^[0-9]+(\.[0-9]+)?$' && awk -v v="$1" -v t="$2" 'BEGIN{exit !(v<=t)}'; }
near(){ echo "$1" | grep -Eq '^-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?$' && awk -v v="$1" -v t="$2" -v e="$3" 'BEGIN{d=v-t; if(d<0)d=-d; exit !(d<=e)}'; }
# frame NAME LABEL: render_frame to $OUT/NAME; the JSON answer must say ok.
frame(){
    local resp
    resp="$(curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' -d "{\"output_path\":\"$OUT/$1\"}")"
    echo "$resp" | grep -q '"ok":[[:space:]]*true' && [ -f "$OUT/$1" ] \
        && ok "$2: render_frame wrote $1" || no "$2: render_frame did not write $1 ($resp)"
}
P(){ python3 "$RT" post "$1" "$2"; }
G(){ python3 "$RT" get "$1" "$2"; }
rstat(){ G /api/routine/status "$1"; }
comp(){ G /api/composition "$1"; }
now(){ perl -MTime::HiRes=time -e 'printf "%.3f", time'; }
since(){ perl -e "printf '%.3f', $(now) - $1"; }

# --- 0. preconditions --------------------------------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set ROUTINES_APP or ROUTINES_BUILD_DIR)"; exit 64; }
[ -x "$PXPY" ] || { echo "REFUSE: no python at $PXPY with PIL+numpy (set ROUTINES_PY)"; exit 64; }
"$PXPY" -c "import PIL, numpy" >/dev/null 2>&1 || { echo "REFUSE: $PXPY lacks PIL and/or numpy"; exit 64; }
[ -f "$ROOT/tests/fixtures/test_card.png" ] || { echo "REFUSE: tests/fixtures/test_card.png missing"; exit 64; }
sed "s#@ROOT@#$ROOT#g" "$TEMPLATE" > "$FIXTURE"
python3 -m json.tool "$FIXTURE" >/dev/null 2>&1 && ok "fixture is valid JSON ($FIXTURE)" || no "fixture is NOT valid JSON"
echo "(app: $APPBUNDLE)"
echo "(artifacts: $OUT)"

# --- 1. launch, beat clock, fixture, reference frame -------------------------
: > "$OUT/app-out.log"; : > "$OUT/app-err.log"
open -g --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (first launch after a rebuild may need the microphone permission allowed once -- do NOT full-screen capture to check)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] && ok "app launched via open -g, /api/health answered, PID=$PID" || { echo "FAIL: no Audio-DNA process"; exit 1; }

ST_OK="$(rstat "d.get('ok')")"
[ "$ST_OK" = "True" ] && ok "GET /api/routine/status answers ok" || no "GET /api/routine/status does not answer ok ($(curl -s -o /dev/null -w '%{http_code}' "$A/api/routine/status")) -- a pre-routines binary 404s here"

P /api/set_bpm '{"bpm":120}' >/dev/null
sleep 1
B_A="$(G /api/bpm "d.get('totalBarCount','NA')")"; sleep 2.3
B_B="$(G /api/bpm "d.get('totalBarCount','NA')")"
[ "$B_A" != "NA" ] && [ "$B_B" != "NA" ] && [ "$B_B" -gt "$B_A" ] 2>/dev/null \
    && ok "manual 120 BPM: totalBarCount advances ($B_A -> $B_B)" || no "totalBarCount does not advance ($B_A -> $B_B)"

P /api/load_composition "{\"path\":\"$FIXTURE\"}" | grep -q '"ok": true' \
    && ok "load_composition accepted the fixture" || no "load_composition rejected the fixture"
sleep 1
P /api/trigger_clip '{"layer":0,"column":0}' >/dev/null
sleep 1
[ "$(comp "d['decks'][0]['layers'][0]['activeClipColumn']")" = "0" ] && ok "L0 on column 0 (the image)" || no "L0 is not on column 0"
frame ref.png "reference"
assert_nonblank ref.png "reference"

# --- 2. record the take --------------------------------------------------------
rows python3 "$RT" record "$TAKE_NAME"
sleep 1.5
[ "$(G /api/perf/status "d.get('recording')")" = "False" ] && ok "take stopped (recording false)" || no "take still recording"
[ -f "$TAKE_FOLDER/take.json" ] && ok "take.json written ($TAKE_FOLDER)" || no "no take.json at $TAKE_FOLDER"
rows python3 "$RT" take "$TAKE_FOLDER" "$OUT/ramp.json"

# --- 3. save the routine (pad 1) --------------------------------------------------
P /api/perf/load "{\"folder\":\"$TAKE_FOLDER\"}" >/dev/null
sleep 1
SAVE="$(P /api/routine/save '{"name":"Probe Routine","fromBeat":0,"toBeat":16,"slot":0}')"
echo "$SAVE" | grep -q '"ok": true' && ok "POST /api/routine/save accepted" || no "POST /api/routine/save refused ($SAVE)"
SAVED=""
for _ in $(seq 1 20); do [ "$(rstat "d['lastSaved']['slot']")" = "0" ] && { SAVED=1; break; }; sleep 0.05; done
[ -n "$SAVED" ] && ok "status lastSaved.slot == 0 within 1 s" || no "status lastSaved.slot never became 0 (lastError: $(rstat "d.get('lastError')"))"
[ "$(rstat "d['bank'][0]['name']")" = "Probe Routine" ] && ok "bank[0].name == Probe Routine" || no "bank[0].name == $(rstat "d['bank'][0]['name']")"
LANES="$(rstat "d['bank'][0]['lanes']")"; PRE="$(rstat "d['bank'][0]['preambleEntries']")"
[ "$LANES" -ge 4 ] 2>/dev/null && ok "bank[0].lanes == $LANES (>= 4)" || no "bank[0].lanes == $LANES (expected >= 4)"
[ "$PRE" -ge 5 ] 2>/dev/null && ok "bank[0].preambleEntries == $PRE (>= 5)" || no "bank[0].preambleEntries == $PRE (expected >= 5)"
near "$(rstat "d['bank'][0]['lengthBeats']")" 16 0.001 && ok "bank[0].lengthBeats == 16" || no "bank[0].lengthBeats == $(rstat "d['bank'][0]['lengthBeats']")"
[ "$(rstat "d['lastSaved']['dropped']")" = "[]" ] && ok "lastSaved.dropped == []" || no "lastSaved.dropped == $(rstat "d['lastSaved']['dropped']")"
[ "$(rstat "d['bank'][0]['state']")" = "idle" ] && ok "bank[0].state == idle" || no "bank[0].state == $(rstat "d['bank'][0]['state']")"

# --- 4. perturb the look ------------------------------------------------------------
P /api/trigger_clip '{"layer":0,"column":1}' >/dev/null
P /api/set_layer_opacity '{"layer":0,"opacity":0.1}' >/dev/null
P /api/set_layer_opacity '{"layer":1,"opacity":0.6}' >/dev/null
P /api/set_param '{"layer":0,"column":1,"effect":"Brightness","param":"amount","value":0.2}' >/dev/null
sleep 1   # > gripHoldMs (250 ms): the REST writes' Decaying grips expire, else the restore is (correctly) refused
PERT="$(comp "'%s|%s|%s|%s' % (d['decks'][0]['layers'][0]['activeClipColumn'], d['decks'][0]['layers'][0]['opacity'], d['decks'][0]['layers'][1]['opacity'], [c for c in d['decks'][0]['layers'][0]['clips'] if c['column']==1][0]['effects'][0]['params'][0]['value'])")"
IFS='|' read -r P_COL P_OP0 P_OP1 P_AMT <<< "$PERT"
[ "$P_COL" = "1" ] && near "$P_OP0" 0.1 0.01 && near "$P_OP1" 0.6 0.01 && near "$P_AMT" 0.2 0.01 \
    && ok "perturbed: L0 column 1, L0 0.1, L1 0.6, C1 Brightness 0.2" || no "perturbation did not land ($PERT)"
frame pert.png "perturbed"
assert_nonblank pert.png "perturbed"
MAD_RP="$(mean_abs_diff "$OUT/ref.png" "$OUT/pert.png" resize)"
num_gt "$MAD_RP" "$MAD_THRESHOLD" && ok "mad(ref, pert) = $MAD_RP > $MAD_THRESHOLD (the perturbation is visible)" || no "mad(ref, pert) = $MAD_RP (expected > $MAD_THRESHOLD)"

# --- 5. fire on the next bar ------------------------------------------------------------
B0="$(G /api/bpm "d.get('totalBarCount','NA')")"; T0="$(now)"
P /api/routine/fire '{"slot":0}' >/dev/null
T1="$(python3 "$RT" wait 0 running 2.6)"
if [ "$T1" = "NA" ]; then
    no "fire: bank[0] never became running within 2.6 s"
    T1="$(now)"
else
    ok "fire: bank[0] running $(perl -e "printf '%.2f', $T1 - $T0") s after the fire"
fi
# Row 6's frame first: the routine's first move lands at +0.6 s.
frame rest.png "restored"; T_REST="$(since "$T1")"
num_leq "$T_REST" 0.55 && ok "restored frame captured +$T_REST s after the start (before the first move at +0.6)" || no "restored frame captured too late (+$T_REST s)"
REST="$(comp "'%s|%s|%s|%s' % (d['decks'][0]['layers'][0]['activeClipColumn'], d['decks'][0]['layers'][0]['opacity'], d['decks'][0]['layers'][1]['opacity'], [c for c in d['decks'][0]['layers'][0]['clips'] if c['column']==1][0]['effects'][0]['params'][0]['value'])")"; T_RC="$(since "$T1")"
python3 "$RT" sample 8.9 "$OUT/grid.json" &
SAMPLER=$!
STARTED="$(rstat "d['bank'][0]['startedTotalBar']")"
[ "$STARTED" -ge $((B0 + 1)) ] 2>/dev/null && ok "fire: started on a LATER bar (startedTotalBar $STARTED >= $B0 + 1)" || no "fire: startedTotalBar $STARTED (bar at fire: $B0)"
DT="$(perl -e "printf '%.3f', $T1 - $T0")"
num_leq "$DT" 2.2 && ok "fire: waited $DT s (<= 2.2 s, one bar)" || no "fire: waited $DT s (> 2.2 s)"
for f in preambleUnresolved preambleRefused unresolved; do
    V="$(rstat "d['bank'][0]['$f']")"
    [ "$V" = "0" ] && ok "fire: $f == 0" || no "fire: $f == $V"
done
PF1="$(rstat "d['bank'][0]['preambleFired']")"
[ "$PF1" -ge 5 ] 2>/dev/null && ok "fire: preambleFired == $PF1 (>= 5)" || no "fire: preambleFired == $PF1 (expected >= 5)"

# --- 6. restore -------------------------------------------------------------------------
IFS='|' read -r R_COL R_OP0 R_OP1 R_AMT <<< "$REST"
echo "(restore read at +$T_RC s: $REST)"
[ "$R_COL" = "0" ] && ok "restore: L0 back on column 0" || no "restore: L0 on column $R_COL"
near "$R_OP0" 1.0 0.05 && ok "restore: L0 opacity $R_OP0 ~ 1.0" || no "restore: L0 opacity $R_OP0 (expected ~1.0)"
near "$R_OP1" 1.0 0.05 && ok "restore: L1 opacity $R_OP1 ~ 1.0" || no "restore: L1 opacity $R_OP1 (expected ~1.0)"
near "$R_AMT" 0.5 0.02 && ok "restore: C1 Brightness $R_AMT ~ 0.5" || no "restore: C1 Brightness $R_AMT (expected ~0.5)"
assert_nonblank rest.png "restored"
MAD_PR="$(mean_abs_diff "$OUT/pert.png" "$OUT/rest.png" resize)"
MAD_RR="$(mean_abs_diff "$OUT/ref.png" "$OUT/rest.png")"
num_gt "$MAD_PR" "$MAD_THRESHOLD" && ok "mad(pert, rest) = $MAD_PR > $MAD_THRESHOLD (the restore changed the frame)" || no "mad(pert, rest) = $MAD_PR (expected > $MAD_THRESHOLD)"
num_leq "$MAD_RR" "$MAD_REST" && ok "mad(ref, rest) = $MAD_RR <= $MAD_REST (the restored look IS the recorded look)" || no "mad(ref, rest) = $MAD_RR (expected <= $MAD_REST)"

# --- 7. the recorded moves on the beat grid, then hold -----------------------------
wait "$SAMPLER"
rows python3 "$RT" grid "$T1" "$OUT/grid.json"

# --- 8. loop --------------------------------------------------------------------------------
P /api/routine/set '{"slot":0,"loop":true}' >/dev/null
sleep 0.3
[ "$(rstat "d['bank'][0]['loop']")" = "True" ] && ok "set: bank[0].loop == true" || no "set: bank[0].loop == $(rstat "d['bank'][0]['loop']")"
P /api/routine/fire '{"slot":0}' >/dev/null
T2="$(python3 "$RT" wait 0 running 2.6)"
if [ "$T2" = "NA" ]; then no "loop: never running"; T2="$(now)"; fi
PF_LOOP0="$(rstat "d['bank'][0]['preambleFired']")"
python3 "$RT" sample "$(perl -e "printf '%.2f', 9.3 - ($(now) - $T2)")" "$OUT/loop.json"
rows python3 "$RT" loop "$T2" "$OUT/loop.json" "$PF_LOOP0"
P /api/routine/stop '{"slot":0}' >/dev/null
T_STOP="$(now)"; IDLE=""
for _ in $(seq 1 8); do [ "$(rstat "d['bank'][0]['state']")" = "idle" ] && { IDLE=1; break; }; sleep 0.025; done
[ -n "$IDLE" ] && ok "stop: bank[0] idle $(since "$T_STOP") s after the stop" || no "stop: bank[0] not idle within 200 ms"
P /api/set_layer_opacity '{"layer":0,"opacity":0.42}' >/dev/null
python3 "$RT" sample 1.2 "$OUT/hold.json" >/dev/null
rows python3 "$RT" hold 0.42 "$OUT/hold.json"

# --- 9. re-fire = restart on the next bar ------------------------------------------------
sleep 0.5
P /api/routine/fire '{"slot":0}' >/dev/null
T3="$(python3 "$RT" wait 0 running 2.6)"
if [ "$T3" = "NA" ]; then no "re-fire: never running"; T3="$(now)"; fi
sleep 0.9   # past the first move (+0.6 s: L0 opacity 0.5)
near "$(comp "d['decks'][0]['layers'][0]['opacity']")" 0.5 0.05 && ok "re-fire: first move played (L0 opacity 0.5)" || no "re-fire: L0 opacity $(comp "d['decks'][0]['layers'][0]['opacity']") (expected 0.5)"
P /api/routine/fire '{"slot":0}' >/dev/null
T_RF="$(now)"; RESTARTED=""
while [ "$(perl -e "print(($(now) - $T_RF) < 2.3 ? 1 : 0)")" = "1" ]; do
    [ "$(rstat "d['bank'][0]['restarts']")" = "1" ] && { RESTARTED=1; break; }
    sleep 0.03
done
if [ -n "$RESTARTED" ]; then
    ok "re-fire: restarts == 1 at the next bar ($(since "$T_RF") s after the second fire)"
    near "$(comp "d['decks'][0]['layers'][0]['opacity']")" 1.0 0.05 && ok "re-fire: the restart restored L0 opacity to 1.0" || no "re-fire: L0 opacity $(comp "d['decks'][0]['layers'][0]['opacity']") after the restart (expected 1.0)"
else
    no "re-fire: restarts never reached 1 within 2.3 s"
fi
P /api/routine/stop '{"all":true}' >/dev/null
sleep 0.3

# --- 10. start from now -------------------------------------------------------------------
P /api/routine/set '{"slot":0,"loop":false,"restoreState":false}' >/dev/null
P /api/set_layer_opacity '{"layer":0,"opacity":0.1}' >/dev/null
sleep 1
P /api/routine/fire '{"slot":0}' >/dev/null
T4="$(python3 "$RT" wait 0 running 2.6)"
if [ "$T4" = "NA" ]; then no "start from now: never running"; T4="$(now)"; fi
python3 "$RT" sample "$(perl -e "printf '%.2f', 1.4 - ($(now) - $T4)")" "$OUT/fromnow.json" >/dev/null
[ "$(rstat "d['bank'][0]['preambleFired']")" = "0" ] && ok "start from now: preambleFired == 0" || no "start from now: preambleFired == $(rstat "d['bank'][0]['preambleFired']")"
rows python3 "$RT" fromnow "$T4" "$OUT/fromnow.json"
P /api/routine/stop '{"all":true}' >/dev/null
P /api/routine/set '{"slot":0,"restoreState":true}' >/dev/null
sleep 0.3

# --- 11. stacking (D9): the gesture that began later wins -----------------------------
if [ -f "$OUT/ramp.json" ]; then
    SAVE2="$(P /api/routine/save '{"name":"Probe Ramp","fromBeat":16,"toBeat":32,"slot":1}')"
    echo "$SAVE2" | grep -q '"ok": true' && ok "stack: saved Probe Ramp to pad 2" || no "stack: save refused ($SAVE2)"
    S2=""
    for _ in $(seq 1 20); do [ "$(rstat "d['lastSaved']['slot']")" = "1" ] && { S2=1; break; }; sleep 0.05; done
    [ -n "$S2" ] && ok "stack: lastSaved.slot == 1" || no "stack: lastSaved.slot never became 1 ($(rstat "d.get('lastError')"))"
    P /api/routine/fire '{"slot":1}' >/dev/null
    T5="$(python3 "$RT" wait 1 running 2.6)"
    if [ "$T5" = "NA" ]; then no "stack: Probe Ramp never running"; T5="$(now)"; fi
    P /api/routine/fire '{"slot":0}' >/dev/null            # waits for the NEXT bar (2 s after T5)
    python3 "$RT" sample "$(perl -e "printf '%.2f', 5.2 - ($(now) - $T5)")" "$OUT/stack.json" >/dev/null
    T6="$("$PXPY" -c "
import json
s = json.load(open('$OUT/stack.json'))
t = [r['t'] for r in s if r.get('bank') and r['bank'][0]['state'] == 'running']
print('%.3f' % t[0] if t else 'NA')")"
    if [ "$T6" = "NA" ]; then
        no "stack: Probe Routine never ran while sampled"
    else
        # T6 = the first SAMPLE that saw Probe Routine running (the start fell between the previous
        # sample and this one's status read); the stack windows keep a margin on both sides.
        rows python3 "$RT" stack "$T5" "$T6" "$OUT/ramp.json" "$OUT/stack.json"
    fi
    P /api/routine/stop '{"all":true}' >/dev/null
else
    no "stack: skipped -- the take has no ramp gesture (row 2 failed)"
fi

# --- 12. teardown (SCREEN-SAFETY LAW) ---------------------------------------------------
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful quit did not clear the process within 30 s -- pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING after quit + pkill" || ok "app terminated, no process remains"
W="$("$PXPY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', '')) and 'Output' in str(w.get('kCGWindowName', ''))]))" 2>/dev/null)"
if [ "$W" = "0" ]; then
    ok "0 Audio-DNA Output windows in the full window list (screen-safety law held)"
elif [ -z "$W" ]; then
    no "Output-window check: $PXPY has no pyobjc/Quartz"
else
    no "$W Audio-DNA Output window(s) seen -- screen-safety breach"
fi
echo "no full-screen capture was taken; the Quartz window list is the screen witness"
echo "(take kept at $TAKE_FOLDER; look at $OUT/rest.png before accepting)"

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
