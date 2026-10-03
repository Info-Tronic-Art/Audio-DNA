#!/bin/bash
# probe-routines.sh -- s-rta-0926 L-R Routines slice 1, lane 1b live gate
# (.harmony/.reports/s-rta-0926/plan-routines-s1-final.md section 6.2).
#
# Proves, over the production REST API on a live app, the whole slice-1 loop: record a take, save
# a slice of it as a routine on a pad, perturb the look, fire the routine, see it wait for the next
# bar, restore the recorded look (pixel-decoded), replay its recorded moves on the beat grid, hold
# at the end (once) or loop (restore re-fired each cycle), restart on re-fire, skip the restore in
# "start from now", and -- stacking (D9) -- let the routine whose gesture BEGAN later win for the
# rest of the earlier routine's gesture. s-rta-0926b routines-followup (row 11j): a routine whose restore
# style is Jump cuts to its start look ON the bar -- at the start and at the loop return -- where the
# default Ease glides (rows 5g/8g/9g/11g).
# s-rta-0927 routines-timing: the windows of rows 8 ("holds 0.9"), 11 ("the later restore wins") and 11j (hard cut,
# restored on the bar, loop hold, loop landing) select samples by the routine's PUBLISHED position (routine beats),
# ending >= 0.1 s before the next recorded event -- not by wall time after a sampled anchor, which lags the start
# 0.06..0.12 s (the flake of s-rta-0926b). Why: the comment above LEN in the helpers.
# s-rta-0927 beat clock (row 7s): row 7's grid again with a 550 ms message-thread stall at +1.0 s (the TEST-ONLY
# POST /api/debug/stall_message_thread; SKIP in a binary without it) -- the routine clock loses no beat across it.
# s-rta-0928 restore (restore-diag.md; holdMs measures the two diagnosed causes only -- a GREEN is not "no restore hold
# exists"): rows 5h / 8h / 11h read /api/routine/status bank[0].holdMs (the Ease start) / holdMsMax (the Ease start + its
# loop return; the Jump start + its loop return) and hold them to 16 ms. RED on a binary before the ClipThumbnails fix:
# ~50-67 ms (every deck refresh decoded each image thumbnail on the message thread, once per discrete restore entry);
# on a binary before holdMs: "absent". Row 7m stalls the message thread 600 ms across the routine's first recorded move
# (TEST-ONLY stall hook; SKIP without it): the move must still land (L0 opacity 0.5 after the stall). RED before the
# Player fix: every sample stays 1.0 (the move was touched and released unwritten -- restore-diag cause 2). A stall
# that did not span the move is "inconclusive" (a FAIL, never a PASS), retried once.
#
# RED on a pre-routines binary: every /api/routine/* route is 404 and /api/composition clips carry
# no "effects" block; take.json meta has no startBeatInBar; a first-activation auto-play is not a
# `playing` lane point (routines-1a carried concern (a)).
#
# Mechanics copied from probe-step3.sh (helpers, `open -g` launch, graceful osascript quit,
# Quartz 0-Output-window witness, adna_pids/adna_running (ucomm-based)) and
# probe-mastersignal.sh (pixel oracle:
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
# pkill only as the last resort. The caller holds the live lock (/tmp/audiodna-live.lock, now enforced -- see PROBE RIG GATE below).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner
# file's first field. Every pgrep below is replaced by adna_pids/adna_running; every quit is quit_ours
# (defined right after the lock gate), which filter on `ps -o ucomm=` -- the KERNEL's real
# exec-time process name, set from the actual binary that was exec'd, NOT from argv[0] -- being
# exactly "Audio-DNA". Verified both directions: (1) a build's linker/compiler command line whose
# -o argument is ".../MacOS/Audio-DNA" has REAL ucomm clang/ld, never matched (the old `pgrep -f`
# substring match DID match it -- that was the bug); (2) a process that spoofs argv[0] via
# `exec -a .../MacOS/Audio-DNA <cmd>` still has the REAL exec'd command's ucomm (e.g. "sleep"), also
# never matched -- `pgrep -x` alone is NOT enough here, since macOS pgrep without -f still matches
# on the (spoofable) comm/argv[0] field, not on ucomm.
set -u
# --- live-lock gate: refuse unless the caller holds /tmp/audiodna-live.lock (rig rule) ---
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
# adna_pids: PIDs whose REAL kernel process name (ucomm, set at exec() time from the actual binary
# run -- immune to argv[0] spoofing) is exactly "Audio-DNA". adna_running builds on it.
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
OUT_BASE="${1:-/tmp/audiodna-routines}"
OUT="$OUT_BASE/run-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
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
import json, os, sys, time, urllib.request, urllib.error
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
        row['bank'] = [{k: b.get(k) for k in ('state', 'cycle', 'restarts', 'yielded', 'preambleFired', 'position', 'glides')}
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

# s-rta-0927 routines-timing (T1): windows on the ROUTINE's clock, not the wall clock.
# A probe anchor such as T6 / TJ / T2 is the first SAMPLE that saw a routine running. Measured over 16 full runs
# (pre- and post-canvas builds alike) it lags the routine's start by 0.06..0.12 s -- a start that restores holds
# the app's message thread 40..90 ms before it publishes "running", plus the sampler's ~40 ms cadence -- while
# the routine's recorded moves are scheduled on its own clock. So "T + 0.5 s" ended -0.02..+0.05 s from the
# first recorded move (beat ~1.2 = start + 0.6 s) and a sample could see the move inside the window. The beat
# clock can also lose time when the message thread stalls (seen: 0.15 s and 0.5 s), which moves every later
# event in wall time. The rows below therefore select samples by the routine's PUBLISHED position (routine beats
# since the cycle began, bank[].position): snap() reads the composition BEFORE the status, and both are written
# by the message thread, so a sample whose status says position p read a look from position <= p. A window
# that ends at p <= event - MARGIN_BEATS ends >= 0.1 s (at 120 BPM) before the event, whatever the lag.
LEN = 16.0            # Probe Routine's length in beats (saved as take beats [0, 16)): its loop point
MARGIN_BEATS = 0.2    # 0.1 s at the probe's 120 BPM

def rpos(r, slot, cycle=None):
    # (cycle, position) of bank[slot] if this sample saw it running (in `cycle`, when given), else None
    try:
        b = r['bank'][slot]
    except Exception:
        return None
    if b.get('state') != 'running' or not isinstance(b.get('position'), (int, float)):
        return None
    if cycle is not None and b.get('cycle') != cycle:
        return None
    return b.get('cycle'), b['position']

def pos_window(samples, slot, lo, hi, cycle=1):
    # samples that saw bank[slot] running in `cycle` at a published position in [lo, hi] (routine beats)
    return [r for r in samples if rpos(r, slot, cycle) and lo <= rpos(r, slot, cycle)[1] <= hi]

def first_move_beat(folder):
    # the routine's first recorded move: the earliest L0 opacity point of the take in [0, LEN) (take beat = routine beat)
    beats = []
    for ln in load(folder + '/take.json').get('lanes', []):
        k = ln.get('key', {})
        L = k.get('layer')
        L = L.get('i') if isinstance(L, dict) else L
        if k.get('scope') == 'layer' and k.get('scalar') == 'opacity' and L == 0:
            beats += [p['beat'] for p in ln.get('points', []) if isinstance(p.get('beat'), (int, float))]
            beats += [g['curve'][0]['x'] for g in ln.get('gestures', []) if g.get('curve')]
    beats = [b for b in beats if 0.0 <= b < LEN]
    return min(beats) if beats else None

def first_move_span(folder):
    # s-rta-0928 (row 7m): [x0, x1] of the routine's first recorded L0 opacity gesture (first_move_beat's lane)
    spans = []
    for ln in load(folder + '/take.json').get('lanes', []):
        k = ln.get('key', {})
        L = k.get('layer')
        L = L.get('i') if isinstance(L, dict) else L
        if k.get('scope') == 'layer' and k.get('scalar') == 'opacity' and L == 0:
            spans += [(g['curve'][0]['x'], g['curve'][-1]['x']) for g in ln.get('gestures', []) if g.get('curve')]
    spans = [sp for sp in spans if 0.0 <= sp[0] < LEN]
    return min(spans) if spans else None

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

elif cmd == 'edge':                       # edge -> epoch of the NEXT bar edge, polled early in a bar (plan3 C), else NA
    # Waits until beatInBar == 0 and beatPhase < 0.5 (so >= 3.5 beats remain: a routine fired now glides over
    # the bar's whole last beat), then predicts the edge from the tempo.
    end = time.time() + 4.0
    while time.time() < end:
        t = time.time()
        d = get('/api/bpm')
        try:
            if d['beatInBar'] == 0 and d['beatPhase'] < 0.5 and d['bpm'] > 0:
                print('%.3f' % (t + (4.0 - d['beatInBar'] - d['beatPhase']) * 60.0 / d['bpm'])); sys.exit(0)
        except Exception:
            pass
        time.sleep(0.01)
    print('NA')

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
    # ROUTINES_RECORD_PAUSE (seconds, default 0): push Record later into the bar. The default schedule tends to land
    # Record near the start of a bar; the s-rta-0926 stamp bug (RecorderClock, e5ceb98) only showed with
    # startBeatInBar > ~2 -- Harmony's gate runs 1.8 as well as the default.
    time.sleep(float(os.environ.get('ROUTINES_RECORD_PAUSE', '0') or 0))
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

elif cmd == 'grid':                       # grid T1 FILE [TAG]: row 7 (TAG 7s: the same marks through a stall)
    T1, s = float(sys.argv[2]), load(sys.argv[3])
    tag = sys.argv[4] if len(sys.argv) > 4 else 'grid'
    marks = [('L0 opacity ~0.5', lambda r: near(r.get('op0'), 0.5, 0.05), 0.6),
             ('L0 on column 1', lambda r: r.get('col0') == 1, 2.6),
             ('C1 Brightness ~0.9', lambda r: near(r.get('amt1'), 0.9, 0.02), 4.6),
             ('L0 opacity ~0.9', lambda r: near(r.get('op0'), 0.9, 0.05), 6.6)]
    times = []
    for label, pred, at in marks:
        t = first(s, pred, T1)
        times.append(t)
        if t is None:
            print('no %s: %s never seen (expected at +%.1f s)' % (tag, label, at))
        else:
            rel = t - T1
            print(('ok' if abs(rel - at) <= 0.35 else 'no') + ' %s: %s first at +%.2f s (expected +%.1f +/- 0.35)' % (tag, label, rel, at))
    seen = [t for t in times if t is not None]
    print(('ok' if len(seen) == 4 and seen == sorted(seen) else 'no') + ' %s: the four moves arrive in the recorded order' % tag)
    tail = [r for r in s if r['t'] >= T1 + 8.3]
    if not tail:
        print('no %s: no samples after +8.3 s' % tag)
    else:
        r = tail[-1]
        print(('ok' if state(r, 0) == 'idle' else 'no') + ' %s: once -- state idle after the end (%s)' % (tag, state(r, 0)))
        print(('ok' if near(r.get('op0'), 0.9, 0.05) and r.get('col0') == 1 else 'no')
              + ' %s: once -- the last look holds (L0 opacity %s, column %s)' % (tag, r.get('op0'), r.get('col0')))

elif cmd == 'loop':                       # loop T2 FILE FIRED_AT_START: row 8
    T2, s, fired0 = float(sys.argv[2]), load(sys.argv[3]), int(sys.argv[4])
    # plan3 C: the return glide moves L0 opacity over the cycle's last beat (from routine beat LEN - 1).
    # s-rta-0927 routines-timing: by routine position -- it ended at T2+7.45, 0.02..0.06 s INTO the glide
    # (T2 lags the start 0.07..0.11 s) and passed only on the 0.05 tolerance.
    pre = pos_window(s, 0, 14.0, LEN - 1.0 - MARGIN_BEATS)
    print(('ok' if len(pre) >= 3 and all(near(r.get('op0'), 0.9, 0.05) for r in pre) else 'no')
          + ' loop: end of cycle 1 holds L0 opacity 0.9 until the return glide begins (routine beats 14.0..%.1f, %d samples)'
          % (LEN - 1.0 - MARGIN_BEATS, len(pre)))
    # 8g (plan3 C): over the cycle's last beat L0 opacity EASES 0.9 -> 1.0 and lands on the loop point.
    ret = [r.get('op0') for r in s if T2 + 7.55 <= r['t'] <= T2 + 7.92]
    between = [v for v in ret if isinstance(v, (int, float)) and 0.92 < v < 0.98]
    nondec = all(isinstance(a, (int, float)) and isinstance(b, (int, float)) and b >= a - 0.005 for a, b in zip(ret, ret[1:]))
    print(('ok' if len(between) >= 2 and nondec else 'no')
          + ' loop: the return GLIDES 0.9 -> 1.0 over the last beat (+7.55..+7.92 s: %d samples, %d strictly between 0.92 and 0.98, non-decreasing=%s: %s)'
          % (len(ret), len(between), nondec, [round(v, 3) if isinstance(v, (int, float)) else v for v in ret]))
    t_land = first(s, lambda r: near(r.get('op0'), 1.0, 0.01), T2 + 7.55)
    print(('ok' if t_land is not None and T2 + 7.80 <= t_land <= T2 + 8.1 else 'no')
          + ' loop: the return lands ON the loop point (first sample within 0.01 of 1.0 at +%s s, expected +7.80..+8.1; s-rta-0926b: widened from 7.85 after a sampling-edge miss at +7.85 vs 3/3 landings at +7.88..7.89)'
          % (None if t_land is None else round(t_land - T2, 2)))
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

elif cmd == 'stack':                      # stack T5 T6 RAMPFILE FILE TAKEFOLDER: row 11
    T5, T6, ramp, s = float(sys.argv[2]), float(sys.argv[3]), load(sys.argv[4]), load(sys.argv[5])
    m0 = first_move_beat(sys.argv[6])
    rs = (ramp['x0'] - 16.0) / 2.0                    # ramp start / end, seconds after the Probe Ramp start
    re = (ramp['x1'] - 16.0) / 2.0
    lo, hi = min(ramp['y0'], ramp['y1']), max(ramp['y0'], ramp['y1'])
    print('(stack: Probe Ramp started at T5; its ramp runs +%.2f..+%.2f s, %.2f..%.2f; Probe Routine started at T5+%.2f s)'
          % (rs, re, lo, hi, T6 - T5))
    # plan3 C: the second routine's restore begins up to one beat BEFORE its bar, so the ramp's window ends at T6-0.55.
    before = [r for r in s if T5 + rs + 0.2 <= r['t'] <= T6 - 0.55]
    vals = [r.get('op0') for r in before]
    inside = bool(vals) and all(isinstance(v, (int, float)) and lo - 0.01 <= v <= hi + 0.01 for v in vals)
    rising = bool(vals) and all(b >= a - 0.005 for a, b in zip(vals, vals[1:])) and vals[-1] > vals[0]
    print(('ok' if inside and rising else 'no')
          + ' stack: before the second routine starts, the ramp drives L0 opacity (%d samples, %s..%s)'
          % (len(vals), vals[0] if vals else None, vals[-1] if vals else None))
    # 11g (plan3 C): over the last beat before its bar the second routine's restore GLIDES L0 opacity from the
    # ramp's value toward 1.0 -- far faster than the ramp (which alone stays below ~0.45 there).
    gl = [r.get('op0') for r in s if T6 - 0.45 <= r['t'] <= T6 - 0.05]
    gnum = [v for v in gl if isinstance(v, (int, float))]
    gnondec = len(gnum) == len(gl) and all(b >= a - 0.005 for a, b in zip(gnum, gnum[1:]))
    print(('ok' if len(gnum) >= 3 and gnondec and max(gnum) > 0.75 else 'no')
          + ' stack: the later restore GLIDES in over the last beat, from the ramp value toward 1.0 (T6-0.45..T6-0.05: %d samples, non-decreasing=%s, max %s > 0.75: %s)'
          % (len(gnum), gnondec, max(gnum) if gnum else None, [round(v, 3) for v in gnum]))
    # s-rta-0927 routines-timing: by Probe Routine's position, 0.4 beats in to MARGIN_BEATS before its first
    # recorded move (was T6+0.12..T6+0.5 s, which ended ON that move: T6 lags the start 0.06..0.11 s).
    win = [] if m0 is None else pos_window(s, 0, 0.4, m0 - MARGIN_BEATS)
    print(('ok' if len(win) >= 3 and all(near(r.get('op0'), 1.0, 0.03) for r in win) else 'no')
          + ' stack: the later restore wins -- L0 opacity holds 1.0 after Probe Routine starts, no ramp values '
          '(routine beats 0.4..%s, first move at beat %s: %d samples, %s)'
          % (None if m0 is None else round(m0 - MARGIN_BEATS, 3), None if m0 is None else round(m0, 3), len(win),
             sorted(set(round(r.get('op0') or -1, 3) for r in win))))
    mid = [r for r in s if T6 + 0.95 <= r['t'] <= T5 + re + 0.9]
    print(('ok' if mid and all(near(r.get('op0'), 0.5, 0.03) for r in mid) else 'no')
          + ' stack: the earlier ramp stays quiet for the rest of its gesture and after it (L0 opacity 0.5, %d samples)' % len(mid))
    y = max([r['bank'][1]['yielded'] or 0 for r in s if 'bank' in r] or [0])
    print(('ok' if y >= 1 else 'no') + ' stack: status bank[1].yielded == %d (>= 1: the displaced gesture is counted)' % y)

elif cmd == 'glide':                      # glide T1 FILE: row 5g (plan3 C) -- the start's restore glides onto the bar
    T1, s = float(sys.argv[2]), load(sys.argv[3])
    early = [r.get('op0') for r in s if r['t'] < T1 - 0.6]
    print(('ok' if len(early) >= 5 and all(near(v, 0.1, 0.02) for v in early) else 'no')
          + ' 5g: while the routine waits, L0 opacity holds the hand value 0.1 until its last beat (%d samples before T1-0.6)' % len(early))
    win = [r.get('op0') for r in s if T1 - 0.45 <= r['t'] <= T1 - 0.1]
    num = [v for v in win if isinstance(v, (int, float))]
    mid = [v for v in num if 0.15 < v < 0.95]
    nondec = len(num) == len(win) and all(b >= a - 0.005 for a, b in zip(num, num[1:]))
    print(('ok' if len(mid) >= 3 and nondec else 'no')
          + ' 5g: over the last beat before the bar L0 opacity GLIDES 0.1 -> 1.0 (T1-0.45..T1-0.1: %d samples, %d strictly between 0.15 and 0.95, non-decreasing=%s: %s)'
          % (len(num), len(mid), nondec, [round(v, 3) for v in num]))
    during = [r['bank'][0].get('glides') for r in s if T1 - 0.5 <= r['t'] < T1 and r.get('bank')]
    after = [r['bank'][0].get('glides') for r in s if r['t'] >= T1 + 0.1 and r.get('bank')]
    okc = any(isinstance(g, int) and g >= 1 for g in during) and bool(after) and all(g == 0 for g in after)
    print(('ok' if okc else 'no')
          + ' 5g: status bank[0].glides >= 1 during the last beat and 0 from T1+0.1 on (during: %s, after: %s)'
          % (sorted(set(str(g) for g in during)), sorted(set(str(g) for g in after))))

elif cmd == 'jump':                       # jump TJ FILE TAKEFOLDER: row 11j (s-rta-0926b routines-followup) -- a Jump start is a cut ON the bar
    TJ, s = float(sys.argv[2]), load(sys.argv[3])
    m0 = first_move_beat(sys.argv[4])
    before = [r.get('op0') for r in s if r['t'] < TJ - 0.12]
    bnum = [v for v in before if isinstance(v, (int, float))]
    print(('ok' if len(before) >= 10 and all(near(v, 0.1, 0.02) for v in before) else 'no')
          + ' 11j: while the Jump routine waits -- through its whole last beat -- L0 opacity holds the hand value 0.1 (%d samples before TJ-0.12, %s..%s)'
          % (len(before), min(bnum) if bnum else None, max(bnum) if bnum else None))
    # s-rta-0927 routines-timing: both windows end MARGIN_BEATS before the first recorded move BY ROUTINE POSITION
    # (they ended at TJ+0.5 s, ON that move: TJ lags the start 0.06..0.12 s). `upto` = every sample before the
    # first one that saw the routine past m0 - MARGIN_BEATS (the waiting samples included).
    hi = None if m0 is None else m0 - MARGIN_BEATS
    upto = []
    for r in s:
        p = rpos(r, 0, 1)
        if hi is None or (p and p[1] > hi):
            break
        upto.append(r.get('op0'))
    mid = [round(v, 3) for v in upto if isinstance(v, (int, float)) and 0.15 < v < 0.95]
    print(('ok' if upto and not mid else 'no')
          + ' 11j: a hard cut -- no sample strictly between 0.15 and 0.95 up to routine beat %s (%d samples, %d in between: %s)'
          % (None if hi is None else round(hi, 3), len(upto), len(mid), mid))
    after = [] if hi is None else [r.get('op0') for r in pos_window(s, 0, 0.4, hi)]
    print(('ok' if len(after) >= 3 and all(near(v, 1.0, 0.02) for v in after) else 'no')
          + ' 11j: on the bar L0 opacity is restored to 1.0 (routine beats 0.4..%s: %d samples, %s)'
          % (None if hi is None else round(hi, 3), len(after), sorted(set(round(v, 3) for v in after if isinstance(v, (int, float))))))
    gl = [r['bank'][0].get('glides') for r in s if r.get('bank')]
    print(('ok' if gl and all(g == 0 for g in gl) else 'no')
          + ' 11j: status bank[0].glides stays 0 across the start (%s)' % sorted(set(str(g) for g in gl)))

elif cmd == 'jumploop':                   # jumploop TJ FILE: row 11j (s-rta-0926b routines-followup) -- a Jump loop return is a cut ON the loop point
    TJ, s = float(sys.argv[2]), load(sys.argv[3])
    # s-rta-0927 routines-timing: by routine position. "Holds 0.9" ended at TJ+7.9 s, ON the loop point (TJ lags
    # the start 0.06..0.12 s) and held only because the loop tick's restore stalls the message thread ~80 ms; a
    # beat clock that lost 0.15 s moved the loop point past "+8.15" (reppre1 run). It now ends MARGIN_BEATS before
    # the loop point, and the landing is read off the cycle counter: the first 1.0 must be a sample of cycle 2 at
    # most 0.5 beats (0.25 s) in -- or cycle 1's last MARGIN_BEATS (the tick that loops writes before it publishes).
    pre = [r.get('op0') for r in pos_window(s, 0, 14.0, LEN - MARGIN_BEATS)]
    pnum = [v for v in pre if isinstance(v, (int, float))]
    print(('ok' if len(pre) >= 10 and all(near(v, 0.9, 0.02) for v in pre) else 'no')
          + ' 11j loop: the end of cycle 1 holds L0 opacity 0.9 through its last beat -- no return glide (routine beats 14.0..%.1f: %d samples, %s..%s)'
          % (LEN - MARGIN_BEATS, len(pre), min(pnum) if pnum else None, max(pnum) if pnum else None))
    land = next((r for r in s if (rpos(r, 0, 2) or (rpos(r, 0, 1) and rpos(r, 0, 1)[1] >= 15.0))
                 and near(r.get('op0'), 1.0, 0.01)), None)
    lp = None if land is None else (rpos(land, 0) if rpos(land, 0) else None)
    good = lp is not None and ((lp[0] == 2 and lp[1] <= 0.5) or (lp[0] == 1 and lp[1] >= LEN - MARGIN_BEATS))
    print(('ok' if good else 'no')
          + ' 11j loop: the return cuts to 1.0 ON the loop point (first sample within 0.01 of 1.0 at cycle/position %s, expected cycle 2 <= 0.5 beats in; +%s s after TJ)'
          % (None if lp is None else '%d/%.3f' % lp, None if land is None else round(land['t'] - TJ, 2)))
    between = [round(v, 3) for r in s if TJ + 7.0 <= r['t'] <= TJ + 8.3 for v in [r.get('op0')]
               if isinstance(v, (int, float)) and 0.92 < v < 0.98]
    print(('ok' if pre and not between else 'no')
          + ' 11j loop: no sample strictly between 0.92 and 0.98 around the loop point (+7.0..+8.3 s: %s)' % between)

elif cmd == 'stallat':                    # stallat LO MS SECS OUT: row 7m -- stall the message thread once bank[0] reaches LO
    lo, ms, secs, outp = float(sys.argv[2]), int(sys.argv[3]), float(sys.argv[4]), sys.argv[5]
    p_before, end = None, time.time() + 3.0
    while time.time() < end:
        s = get('/api/routine/status')
        try:
            b = s['bank'][0]
            if b['state'] == 'running' and b['position'] >= lo:
                p_before = b['position']; break
        except Exception:
            pass
        time.sleep(0.01)
    out = {'p_before': p_before, 't_stall': time.time(), 'ms': ms, 'samples': []}
    if p_before is not None:
        out['stall'] = post('/api/debug/stall_message_thread', {'ms': ms})
        end = time.time() + secs
        while time.time() < end:
            row = {'t': time.time()}
            st = get('/api/routine/status')
            c = get('/api/composition')
            try:
                b = st['bank'][0]
                row['pos'] = b['position'] if b['state'] == 'running' else None
            except Exception:
                row['pos'] = None
            try:
                row['op0'] = c['decks'][0]['layers'][0]['opacity']
            except Exception:
                row['op0'] = None
            out['samples'].append(row)
            time.sleep(0.04)
    with open(outp, 'w') as f:
        json.dump(out, f)
    print('(7m: stall of %d ms posted at routine position %s; %d samples)' % (ms, p_before, len(out['samples'])))

elif cmd == 'movestall':                  # movestall JSON TAKE_FOLDER: row 7m -- the stepped-over move still lands
    d, span = load(sys.argv[2]), first_move_span(sys.argv[3])
    pb = d.get('p_before')
    s = d.get('samples', [])
    pa = next((r['pos'] for r in s if isinstance(r.get('pos'), (int, float)) and pb is not None and r['pos'] > pb + 0.5), None)
    if span is None or pb is None or pa is None or not (pb < span[0] and pa > span[1]):
        print('no 7m: inconclusive -- the stall did not span the first move (p_before %s, p_after %s, move %s)'
              % (pb, None if pa is None else round(pa, 3), None if span is None else '[%.3f, %.3f]' % span))
    else:
        after = [r.get('op0') for r in s if isinstance(r.get('pos'), (int, float)) and pa <= r['pos'] <= 5.0]
        good = len(after) >= 3 and all(near(v, 0.5, 0.05) for v in after)
        vals = sorted(set(round(v, 3) if isinstance(v, (int, float)) else v for v in after), key=str)
        if good:
            print('ok 7m: a %d ms stall across the first move (positions %.3f -> %.3f over [%.3f, %.3f]) still lands it: L0 opacity 0.5 at %d samples in %.3f..5.0'
                  % (d.get('ms', 0), pb, pa, span[0], span[1], len(after), pa))
        else:
            print('no 7m: a %d ms stall across the first move (positions %.3f -> %.3f over [%.3f, %.3f]) did NOT land it: L0 opacity %s at %d samples in %.3f..5.0 (expected 0.5 +/- 0.05; every sample 1.0 = the move was touched and released unwritten -- restore-diag cause 2)'
                  % (d.get('ms', 0), pb, pa, span[0], span[1], vals, len(after), pa))

elif cmd == 'restartglide':               # restartglide FILE: row 9g (plan3 C) -- the restart's restore glides too
    s = load(sys.argv[2])
    TR = first(s, lambda r: bool(r.get('bank')) and r['bank'][0].get('restarts') == 1, 0)
    if TR is None:
        print('no re-fire: the sampler never saw restarts == 1')
    else:
        win = [r.get('op0') for r in s if TR - 0.5 <= r['t'] < TR]
        num = [v for v in win if isinstance(v, (int, float))]
        mid = [v for v in num if 0.55 < v < 0.95]
        nondec = len(num) == len(win) and all(b >= a - 0.005 for a, b in zip(num, num[1:]))
        print(('ok' if len(mid) >= 2 and nondec else 'no')
              + ' re-fire: over the last beat before the restart L0 opacity GLIDES from ~0.5 toward 1.0 (0.5 s before restarts == 1: %d samples, %d strictly between 0.55 and 0.95, non-decreasing=%s: %s)'
              % (len(num), len(mid), nondec, [round(v, 3) for v in num]))
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
refuse_foreign_start || exit 64
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
record_ourpid; echo "ours: pid ${OURPID:-none}"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (first launch after a rebuild may need the microphone permission allowed once -- do NOT full-screen capture to check)"; exit 1; }
PID="$(adna_pids | head -1)"
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
# 5g (s-rta-0926b plan3 C): fire EARLY in a bar so the restore's glide window is the bar's whole last beat;
# sample across it (glide.json) and render one frame 0.30 s before the bar (mid.png: between the two looks).
T_EDGE="$(python3 "$RT" edge)"
python3 "$RT" sample 3.0 "$OUT/glide.json" >/dev/null &
GLIDE_SAMPLER=$!
B0="$(G /api/bpm "d.get('totalBarCount','NA')")"; T0="$(now)"
P /api/routine/fire '{"slot":0}' >/dev/null
MID_SHOT=""
if [ "$T_EDGE" != "NA" ]; then
    ( sleep "$(perl -e "my \$d = ($T_EDGE - 0.30) - $(now); printf '%.3f', \$d > 0 ? \$d : 0")"
      T_MID="$(now)"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \
           -d "{\"output_path\":\"$OUT/mid.png\"}" > "$OUT/mid.json"
      echo "$T_MID" > "$OUT/mid.t" ) &
    MID_SHOT=$!
fi
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
# 5h (s-rta-0928 restore): the Ease start's restore held the message thread <= 16 ms (bank[0].holdMs; the diagnosed
# causes only -- a GREEN is not "no restore hold exists"). num_leq rejects "absent" and the -1.000 of "never started".
HOLD5="$(rstat "'%.3f' % d['bank'][0]['holdMs'] if isinstance(d['bank'][0].get('holdMs'), (int, float)) else 'absent'")"
num_leq "$HOLD5" 16 \
    && ok "5h: the Ease start's restore held the message thread $HOLD5 ms <= 16 ms (bank[0].holdMs -- the diagnosed causes only: a GREEN is not 'no restore hold exists')" \
    || no "5h: the Ease start held the message thread $HOLD5 ms (bank[0].holdMs; expected 0..16 -- RED: every deck refresh decoded each image thumbnail on the message thread, restore-diag.md)"
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

# --- 7s. the grid through a 550 ms message-thread stall (s-rta-0927 beat clock, plan-beatclock.md 5.5) -----
# Fire Probe Routine again (once, no loop) and stall the app's message thread for 550 ms at +1.0 s (the window
# +1.0..+1.55 s holds NO mark, so a mark inside the stall cannot fail a row for the wrong reason). The routine
# clock must keep every beat the tracker counted through the stall: the later marks stay on the recorded grid.
# RED on the pre-fix wrap reader (a 1.1-beat gap loses exactly 1 beat): marks 2-4 arrive +0.50 s late -> 3 FAILs.
# The stall hook is TEST-ONLY (AUDIODNA_BUILD_TEST_SERVER=ON); a binary without it SKIPs this row.
STALL_HOOK="$(P /api/debug/stall_message_thread '{"ms":1}')"
if echo "$STALL_HOOK" | grep -q '"ok": true'; then
    sleep 0.3
    P /api/routine/fire '{"slot":0}' >/dev/null
    T1s="$(python3 "$RT" wait 0 running 2.6)"
    if [ "$T1s" = "NA" ]; then no "7s: bank[0] never became running within 2.6 s"; T1s="$(now)"; fi
    python3 "$RT" sample 8.9 "$OUT/grid-stall.json" >/dev/null &
    STALL_SAMPLER=$!
    sleep "$(perl -e "my \$d = ($T1s + 1.0) - $(now); printf '%.3f', \$d > 0 ? \$d : 0")"
    echo "(7s: stall requested at +$(since "$T1s") s: $(P /api/debug/stall_message_thread '{"ms":550}'))"
    wait "$STALL_SAMPLER"
    rows python3 "$RT" grid "$T1s" "$OUT/grid-stall.json" 7s
    P /api/routine/stop '{"all":true}' >/dev/null
    sleep 0.3
    # --- 7m (s-rta-0928 restore, restore-diag cause 2): a 600 ms message-thread stall across the routine's FIRST
    # recorded move (L0 opacity 0.5, a ~0.5-beat gesture at routine beat ~1.2) -- the resume tick steps over the whole
    # gesture, and the move must still land. 600 ms = 1.2 beats: from position 0.80..0.95 the resume lands at ~2.0-2.2,
    # past the move's end and before the next L0 opacity event. A stall that did not span the move is inconclusive
    # (a FAIL), retried once.
    for ATT7M in 1 2; do
        P /api/routine/fire '{"slot":0}' >/dev/null
        T7m="$(python3 "$RT" wait 0 running 2.6)"
        if [ "$T7m" = "NA" ]; then no "7m: bank[0] never became running within 2.6 s"; break; fi
        python3 "$RT" stallat 0.80 600 3.5 "$OUT/movestall-$ATT7M.json"
        R7M="$(python3 "$RT" movestall "$OUT/movestall-$ATT7M.json" "$TAKE_FOLDER" 2>&1)"
        P /api/routine/stop '{"all":true}' >/dev/null
        sleep 0.3
        if [ "$ATT7M" = "1" ] && echo "$R7M" | grep -q "inconclusive"; then
            echo "(7m attempt 1: $R7M -- retrying once)"
            sleep 1.0
            continue
        fi
        rows echo "$R7M"
        break
    done
else
    echo "SKIP: 7s -- no TEST-ONLY stall hook in this binary ($STALL_HOOK)"
    echo "SKIP: 7m -- no TEST-ONLY stall hook in this binary"
fi

# --- 5g. the start's restore glides onto the bar (plan3 C) -----------------------------
wait "$GLIDE_SAMPLER"
if [ "$T_EDGE" = "NA" ]; then
    no "5g: never saw an early-in-bar poll (beatInBar 0, beatPhase < 0.5) within 4 s before the fire"
else
    echo "(5g: predicted bar edge at T1 $(perl -e "printf '%+.3f', $T_EDGE - $T1") s; mid.png requested at T_edge $(perl -e "printf '%+.3f', $(cat "$OUT/mid.t" 2>/dev/null || echo 0) - $T_EDGE") s)"
    [ -n "$MID_SHOT" ] && wait "$MID_SHOT"
    rows python3 "$RT" glide "$T1" "$OUT/glide.json"
    grep -q '"ok":[[:space:]]*true' "$OUT/mid.json" 2>/dev/null && [ -f "$OUT/mid.png" ] \
        && ok "5g: render_frame wrote mid.png 0.30 s before the bar" || no "5g: render_frame did not write mid.png ($(cat "$OUT/mid.json" 2>/dev/null))"
    assert_nonblank mid.png "5g mid-glide"
    MAD_PM="$(mean_abs_diff "$OUT/pert.png" "$OUT/mid.png" resize)"
    MAD_MR="$(mean_abs_diff "$OUT/mid.png" "$OUT/rest.png" resize)"
    num_gt "$MAD_PM" "$MAD_THRESHOLD" && num_gt "$MAD_MR" "$MAD_THRESHOLD" \
        && ok "5g: mid.png lies BETWEEN the two looks -- mad(pert, mid) = $MAD_PM and mad(mid, rest) = $MAD_MR, both > $MAD_THRESHOLD (not a cut)" \
        || no "5g: mid.png is not between the two looks -- mad(pert, mid) = $MAD_PM, mad(mid, rest) = $MAD_MR (both must be > $MAD_THRESHOLD)"
fi

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
# 8h (s-rta-0928 restore): the Ease start and every loop return held <= 16 ms (bank[0].holdMsMax; the diagnosed causes only)
HOLD8="$(rstat "'%.3f' % d['bank'][0]['holdMsMax'] if isinstance(d['bank'][0].get('holdMsMax'), (int, float)) else 'absent'")"
CYC8="$(rstat "d['bank'][0]['cycle']")"
[ "$CYC8" -ge 2 ] 2>/dev/null && num_leq "$HOLD8" 16 \
    && ok "8h: the Ease start and every loop return (cycle $CYC8) held <= 16 ms (bank[0].holdMsMax $HOLD8 -- the diagnosed causes only: a GREEN is not 'no restore hold exists')" \
    || no "8h: the Ease start / loop return held the message thread $HOLD8 ms at cycle $CYC8 (bank[0].holdMsMax; expected cycle >= 2 and 0..16 -- RED: every deck refresh decoded each image thumbnail on the message thread, restore-diag.md)"
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
python3 "$RT" sample 2.8 "$OUT/restart.json" >/dev/null &   # 9g (plan3 C): across the second fire
RESTART_SAMPLER=$!
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
wait "$RESTART_SAMPLER"
rows python3 "$RT" restartglide "$OUT/restart.json"
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
        rows python3 "$RT" stack "$T5" "$T6" "$OUT/ramp.json" "$OUT/stack.json" "$TAKE_FOLDER"
    fi
    P /api/routine/stop '{"all":true}' >/dev/null
else
    no "stack: skipped -- the take has no ramp gesture (row 2 failed)"
fi

# --- 11b. stop while pending (plan3 C): a stop inside the glide lets go where the glide left the knob --
P /api/set_layer_opacity '{"layer":0,"opacity":0.1}' >/dev/null
sleep 1   # > gripHoldMs: the REST write's Decaying grip expires
T_EDGE2="$(python3 "$RT" edge)"
if [ "$T_EDGE2" = "NA" ]; then
    no "stop while pending: never saw an early-in-bar poll within 4 s"
else
    P /api/routine/fire '{"slot":0}' >/dev/null
    sleep "$(perl -e "my \$d = ($T_EDGE2 - 0.40) - $(now); printf '%.3f', \$d > 0 ? \$d : 0")"
    ST_PEND="$(rstat "d['bank'][0]['state']")"
    sleep "$(perl -e "my \$d = ($T_EDGE2 - 0.25) - $(now); printf '%.3f', \$d > 0 ? \$d : 0")"
    P /api/routine/stop '{"slot":0}' >/dev/null
    T_SP="$(now)"
    sleep 0.1
    V_SP1="$(comp "d['decks'][0]['layers'][0]['opacity']")"
    sleep 0.5
    V_SP2="$(comp "d['decks'][0]['layers'][0]['opacity']")"
    ST_SP="$(rstat "d['bank'][0]['state']")"
    echo "(stop while pending: state before the stop '$ST_PEND', stopped $(perl -e "printf '%+.3f', $T_SP - $T_EDGE2") s from the predicted bar; L0 opacity +0.1 s: $V_SP1, +0.6 s: $V_SP2)"
    [ "$ST_PEND" = "pending" ] && [ "$ST_SP" = "idle" ] \
        && ok "stop while pending: the routine was pending at the stop and is idle after it" \
        || no "stop while pending: state before the stop '$ST_PEND' (expected pending), after '$ST_SP' (expected idle)"
    near "$V_SP2" "$V_SP1" 0.02 && awk -v v="$V_SP2" 'BEGIN{exit !(v > 0.15 && v < 0.95)}' \
        && ok "stop while pending: L0 opacity stays where the glide left it (+0.1 s $V_SP1, +0.6 s $V_SP2, strictly between 0.15 and 0.95)" \
        || no "stop while pending: L0 opacity +0.1 s $V_SP1, +0.6 s $V_SP2 (expected equal within 0.02 and strictly between 0.15 and 0.95 -- mid-glide)"
fi

# --- 11j. restore style Jump (s-rta-0926b routines-followup ITEM 2, Boris "controls for jump or ease in each"):
# a Jump routine restores in ONE cut on the bar -- nothing moves during the wait, no glide at the loop return.
# RED on a glide-only binary: it ignores restoreStyle, so the restore still eases in over the last beat.
P /api/routine/set '{"slot":0,"restoreStyle":"jump","loop":true}' >/dev/null
sleep 0.3
JSTYLE="$(rstat "d['bank'][0].get('restoreStyle')")"
[ "$JSTYLE" = "jump" ] && ok "11j: POST /api/routine/set restoreStyle jump -- status bank[0].restoreStyle == jump" \
    || no "11j: status bank[0].restoreStyle == $JSTYLE (expected jump)"
P /api/set_layer_opacity '{"layer":0,"opacity":0.1}' >/dev/null
sleep 1   # > gripHoldMs: the REST write's Decaying grip expires
frame jpre.png "11j waiting look"
assert_nonblank jpre.png "11j waiting look"
T_EDGEJ="$(python3 "$RT" edge)"
if [ "$T_EDGEJ" = "NA" ]; then
    no "11j: never saw an early-in-bar poll within 4 s"
else
    python3 "$RT" sample 3.0 "$OUT/jump.json" >/dev/null &
    JUMP_SAMPLER=$!
    P /api/routine/fire '{"slot":0}' >/dev/null
    ( sleep "$(perl -e "my \$d = ($T_EDGEJ - 0.30) - $(now); printf '%.3f', \$d > 0 ? \$d : 0")"
      curl -s --max-time 20 -X POST "$A/api/render_frame" -H 'Content-Type: application/json' \
           -d "{\"output_path\":\"$OUT/jmid.png\"}" > "$OUT/jmid.json" ) &
    JMID_SHOT=$!
    TJ="$(python3 "$RT" wait 0 running 2.6)"
    if [ "$TJ" = "NA" ]; then no "11j: bank[0] never became running within 2.6 s"; TJ="$(now)"; fi
    wait "$JUMP_SAMPLER"; wait "$JMID_SHOT"
    echo "(11j: predicted bar edge at TJ $(perl -e "printf '%+.3f', $T_EDGEJ - $TJ") s)"
    rows python3 "$RT" jump "$TJ" "$OUT/jump.json" "$TAKE_FOLDER"
    grep -q '"ok":[[:space:]]*true' "$OUT/jmid.json" 2>/dev/null && [ -f "$OUT/jmid.png" ] \
        && ok "11j: render_frame wrote jmid.png 0.30 s before the bar" || no "11j: render_frame did not write jmid.png ($(cat "$OUT/jmid.json" 2>/dev/null))"
    MAD_JJ="$(mean_abs_diff "$OUT/jpre.png" "$OUT/jmid.png")"
    num_leq "$MAD_JJ" "$MAD_REST" \
        && ok "11j: 0.30 s before the bar the frame is still the waiting look -- mad(jpre, jmid) = $MAD_JJ <= $MAD_REST (nothing eased in)" \
        || no "11j: 0.30 s before the bar the frame has moved -- mad(jpre, jmid) = $MAD_JJ (expected <= $MAD_REST: a Jump waits for the bar)"
    python3 "$RT" sample "$(perl -e "printf '%.2f', 8.5 - ($(now) - $TJ)")" "$OUT/jumploop.json" >/dev/null
    rows python3 "$RT" jumploop "$TJ" "$OUT/jumploop.json"
    # 11h (s-rta-0928 restore): the Jump start and its loop return held <= 16 ms (holdMsMax; the diagnosed causes only)
    HOLD11="$(rstat "'%.3f' % d['bank'][0]['holdMsMax'] if isinstance(d['bank'][0].get('holdMsMax'), (int, float)) else 'absent'")"
    CYC11="$(rstat "d['bank'][0]['cycle']")"
    [ "$CYC11" -ge 2 ] 2>/dev/null && num_leq "$HOLD11" 16 \
        && ok "11h: the Jump start and its loop return (cycle $CYC11) held <= 16 ms (holdMsMax $HOLD11 -- the diagnosed causes only: a GREEN is not 'no restore hold exists')" \
        || no "11h: the Jump start / loop return held the message thread $HOLD11 ms at cycle $CYC11 (holdMsMax; expected cycle >= 2 and 0..16 -- RED: every deck refresh decoded each image thumbnail on the message thread, restore-diag.md)"
fi
P /api/routine/stop '{"all":true}' >/dev/null

# --- 12. teardown (SCREEN-SAFETY LAW) ---------------------------------------------------
quit_ours
adna_running && no "APP STILL RUNNING after quit + pkill" || ok "app terminated, no process remains"
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
