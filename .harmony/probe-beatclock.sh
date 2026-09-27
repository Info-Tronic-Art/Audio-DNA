#!/bin/bash
# probe-beatclock.sh -- s-rta-0927 beat clock live gate (.harmony/.reports/s-rta-0927/plan-beatclock.md 5.4):
# the routine / take beat clock (RecorderClock, ticked by the 120 Hz message-thread timer) must not lose
# beats when the message thread stalls. It integrates FeatureSnapshot::totalBeatCount + beatPhase (the
# tracker's continuous beat time) instead of detecting beatPhase wraps at tick rate (Pitfall 42).
#
#   b0  guard: POST /api/debug/stall_message_thread {"ms":1} -- the TEST-ONLY stall hook (compiled only with
#       AUDIODNA_BUILD_TEST_SERVER=ON; no --test-mode needed). A binary without it prints SKIP for b2/b3.
#   b1  /api/bpm.totalBeatCount advances 6 +- 1 over 3.0 s at a manual 120 BPM.
#   b2  a 550 ms message-thread stall: /api/routine/status.clockBeat (published every tick with no routine
#       running -- the clock ticks from app start) advances by 2 x the wall seconds between two reads that
#       bracket the stall (|dBeat - 2 dWall| <= 0.10). The old wrap reader loses exactly 1 beat (a 1.1-beat gap
#       carries only its fractional part).
#   b3  a 350 ms stall started with beatPhase in [0.30, 0.55] (the gap contains a wrap): same check. The old
#       reader reads the wrap as a resync and FREEZES for the whole gap (0.7 beat short).
# RED: a binary built with ONLY the stall hook (lane commit 1, the pre-fix reader) fails b2 by ~-1.0 beat and b3 by
# ~-0.7 beat; the pre-lane main app has no hook (b2/b3 SKIP) and no totalBeatCount (b1 FAIL).
#
# Production mode, port 7070, NO --test-mode. No fixture, no take, no audio device needed: POST /api/set_bpm
# puts the tracker in manual LOCKED mode with a predicted phase. ~40 s.
#
# Environment:
#   BEATCLOCK_APP        full path to Audio-DNA.app (default: $ROOT/$BEATCLOCK_BUILD_DIR/AudioDNA_artefacts/
#                        Release/Audio-DNA.app, BEATCLOCK_BUILD_DIR default "build")
#   BEATCLOCK_PY         python with pyobjc Quartz for the Output-window witness (default $ROOT/.venv/bin/python)
# Artifacts: a FRESH directory per run under ${1:-/tmp/audiodna-beatclock}. Exit 1 on any FAIL.
#
# SCREEN-SAFETY LAW: `open -g` only, never the Output window (no endpoint used here reaches it), never a
# full-screen capture; graceful osascript quit first, pkill only as the last resort. The stall hook sleeps the
# app's message thread for at most 550 ms per call.
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if
# AUDIODNA_LOCK_OWNER is set, it must match the owner file's first field. adna_pids/adna_running/adna_kill filter
# on `ps -o ucomm=` (the kernel's real exec-time process name) being exactly "Audio-DNA" -- see probe-routines.sh.
set -u
# --- live-lock gate: refuse unless the caller holds /tmp/audiodna-live.lock (rig rule) ---
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
OUT_BASE="${1:-/tmp/audiodna-beatclock}"
OUT="$OUT_BASE/run-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BEATCLOCK_BUILD_DIR:-build}"
APPBUNDLE="${BEATCLOCK_APP:-$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app}"
PXPY="${BEATCLOCK_PY:-$ROOT/.venv/bin/python}"
A='http://127.0.0.1:7070'
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

# --- helper (stdlib only; a fresh connection per request: urllib + Connection: close, c1-state-fix) ---
BC="$OUT/bc.py"
cat > "$BC" <<'PY'
import json, sys, time, urllib.request, urllib.error
A = 'http://127.0.0.1:7070'
HDR = {'Connection': 'close'}

def get(path, timeout=3):
    req = urllib.request.Request(A + path, headers=HDR)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return json.loads(r.read().decode())
    except urllib.error.HTTPError as e:
        return {'ok': False, 'http': e.code}
    except Exception as e:
        return {'ok': False, 'error': str(e)}

def post(path, body, timeout=6):
    req = urllib.request.Request(A + path, data=json.dumps(body).encode(),
                                 headers=dict(HDR, **{'Content-Type': 'application/json'}), method='POST')
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

def clock():
    # (wall, clockBeat) -- wall taken when the answer arrived (the status is synchronous, <= 1 tick stale)
    s = get('/api/routine/status')
    return time.time(), s.get('clockBeat')

def stall_row(label, ms, settle, why_old):
    w0, b0 = clock()
    r = post('/api/debug/stall_message_thread', {'ms': ms})
    time.sleep(settle)
    w1, b1 = clock()
    if not isinstance(b0, (int, float)) or not isinstance(b1, (int, float)) or not r.get('ok'):
        print('no %s: could not read clockBeat around the stall (before %s, after %s, stall answer %s)' % (label, b0, b1, json.dumps(r)))
        return
    dW, dB = w1 - w0, b1 - b0
    err = dB - 2.0 * dW
    print(('ok' if abs(err) <= 0.10 else 'no')
          + ' %s: a %d ms message-thread stall -- clockBeat advanced %.3f beats over %.3f s of wall (expected %.3f at 120 BPM; error %+.3f, tolerance 0.10; %s)'
          % (label, ms, dB, dW, 2.0 * dW, err, why_old))

cmd = sys.argv[1]
if cmd == 'post':                        # post PATH JSON -> prints the response JSON
    print(json.dumps(post(sys.argv[2], json.loads(sys.argv[3]))))
elif cmd == 'b1':                        # totalBeatCount advances 6 +- 1 over 3.0 s
    d0 = get('/api/bpm'); t0 = time.time()
    time.sleep(3.0)
    d1 = get('/api/bpm'); t1 = time.time()
    if 'totalBeatCount' not in d0 or 'totalBeatCount' not in d1:
        print('no b1: /api/bpm has no totalBeatCount field (absent -- a pre-beat-clock binary)')
    else:
        n = d1['totalBeatCount'] - d0['totalBeatCount']
        print(('ok' if 5 <= n <= 7 else 'no')
              + ' b1: /api/bpm.totalBeatCount advanced %d over %.2f s at 120 BPM (%d -> %d; expected 6 +- 1)'
              % (n, t1 - t0, d0['totalBeatCount'], d1['totalBeatCount']))
elif cmd == 'b2':
    stall_row('b2', 550, 0.8, 'the old wrap reader loses 1 beat: a 1.1-beat gap carries only its fractional part')
elif cmd == 'b3':
    end = time.time() + 4.0
    seen = None
    while time.time() < end:
        d = get('/api/bpm')
        p = d.get('beatPhase')
        if isinstance(p, (int, float)) and 0.30 <= p <= 0.55:
            seen = p
            break
        time.sleep(0.01)
    if seen is None:
        print('no b3: never saw beatPhase in [0.30, 0.55] within 4 s')
    else:
        print('(b3: stall requested at beatPhase %.3f)' % seen)
        stall_row('b3', 350, 0.6, 'the old reader reads the wrap inside the gap as a resync and freezes for it')
PY

# --- 0. preconditions --------------------------------------------------------
adna_running && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set BEATCLOCK_APP or BEATCLOCK_BUILD_DIR)"; exit 64; }
[ -x "$PXPY" ] || { echo "REFUSE: no python at $PXPY with pyobjc Quartz (set BEATCLOCK_PY)"; exit 64; }
echo "(app: $APPBUNDLE)"
echo "(artifacts: $OUT)"

# --- 1. launch, manual 120 BPM -------------------------------------------------
: > "$OUT/app-out.log"; : > "$OUT/app-err.log"
open -g --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (do NOT full-screen capture to check)"; exit 1; }
PID="$(adna_pids | head -1)"
[ -n "$PID" ] && ok "app launched via open -g, /api/health answered, PID=$PID" || { echo "FAIL: no Audio-DNA process"; exit 1; }
python3 "$BC" post /api/set_bpm '{"bpm":120}' >/dev/null
sleep 1.5

# --- b0. the TEST-ONLY stall hook ---------------------------------------------------
HOOK="$(python3 "$BC" post /api/debug/stall_message_thread '{"ms":1}')"
if echo "$HOOK" | grep -q '"ok": true'; then
    HAVE_HOOK=1; echo "(b0: stall hook present: $HOOK)"
else
    HAVE_HOOK=""; echo "SKIP: no TEST-ONLY hook in this binary ($HOOK) -- b2/b3 not run"
fi
sleep 0.5

# --- b1. totalBeatCount on /api/bpm ------------------------------------------------
rows python3 "$BC" b1

# --- b2 / b3. message-thread stalls -----------------------------------------------
if [ -n "$HAVE_HOOK" ]; then
    rows python3 "$BC" b2
    sleep 1.0
    rows python3 "$BC" b3
fi

# --- teardown (SCREEN-SAFETY LAW) ---------------------------------------------------
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
if adna_running; then
    echo "graceful quit did not clear the process within 30 s -- pkill (last resort)"
    adna_kill
    for _ in $(seq 1 20); do adna_running || break; sleep 1; done
fi
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

echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
