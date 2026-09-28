#!/bin/bash
# probe-tempo-start.sh [OUTDIR] -- live witness for the s-rta-0928 take start race
# (.harmony/.reports/s-rta-0928/plan-tempo.md section 6 + its HARMONY ADOPTION rulings).
#
# THE RACE: a tempo command (set_bpm / Resync / Tap ...) is only a BPMTracker REQUEST that the
# analysis thread applies at its next hop (~10.7 ms). Record sent right after it used to start the
# take on a 120 Hz tick that still read the OLD snapshot: the take's "start" tempo anchor carried the
# old tempo (bpm 0 when unlocked -> no routine can be cut from beat 0) and meta.startBeatInBar was
# read once at arm (unknown when unlocked, the pre-Resync bar position after a Resync).
#
# ORACLE / PAIR: a python client sends, on fresh pre-connected sockets (Connection: close -- cpp-httplib
# drops a request that races its 5 s keep-alive close):
#   1. POST /api/debug/stall_message_thread {"ms": S}   (TEST-ONLY build path) and waits for its answer;
#   2. POST the command and waits for its answer (ApiServer queues the callAsync BEFORE it answers);
#   3. at once POST /api/perf/record {"name": N, "audio": false}.
# Both requests queue behind the S ms message-thread sleep and run back to back after it -- the
# diagnosis's lost-race order, made nearly certain. HARMONY ADOPTION A2: the stall-assisted pair is the
# ONLY RED/GATE mode; if the hook answers 404 the probe prints "BLOCKED: stall hook unavailable" and
# exits 2. TEMPOSTART_STALL_MS=0 is an extra natural-timing run (tight pair), reported, never the RED.
#
# ROWS (per-cycle: pair, sleep 0.3, POST /api/perf/stop, poll /api/perf/status until not recording,
# read the take's take.json):
#   W1  first take of a fresh launch (+2 s after health); precondition /api/bpm bpm == 0 (else SKIP).
#       Pair(set_bpm 120), stop after 0.5 s. PASS iff tempoMap[0] == {start, 120 +- 0.05}, no "lock"
#       anchor, meta.startBeatInBar in [0, 4), and /api/perf/load + /api/routine/save
#       {fromBeat 0, toBeat 4, slot 7} give lastSaved.slot 7 with lastError "" within 1 s.
#   W1b liveness: /api/bpm bpm == 120 and totalBeatCount advances >= 1 in 1 s (else every row is void).
#   W2  20 cycles of Pair(set_bpm 90 / 150 alternating). PASS iff tempoMap[0].bpm == the value sent, 20/20.
#   W3  set_bpm 120, sleep 1, 20 cycles of Pair(resync). PASS iff meta.startBeatInBar in [0, 0.25)
#       and tempoMap[0].bpm == 120, 20/20.
#   W4  grep -c "take start:" on the app's stderr == 0 (the handshake, not the fallback, started every take).
#   W6  (opt-in, TEMPOSTART_WITNESS_RUNS=N) the canonical witness per run: quit, fresh launch, health,
#       sleep 2, set_bpm 120, record audio:false, sleep 1.5, stop, sleep 1, read the final take.json.
#       PASS iff start-bpm-0 == 0/N and unknown grids (startBeatInBar absent) == 0/N.
#
# ENV: TEMPOSTART_APP (a full .app path) or TEMPOSTART_BUILD_DIR (default build-lane);
#      TEMPOSTART_CYCLES (20); TEMPOSTART_STALL_MS (40; 0 = tight pair only); TEMPOSTART_WITNESS_RUNS (0).
# SAFETY: production mode (NO --test-mode -- test mode never starts the analysis thread), port 7070.
# Never opens the Output window (no output endpoint is used), takes no screen capture, writes no
# composition file, records only audio:false takes and deletes exactly the takes it created
# (tstart-<runid>-* under ~/Documents/Audio-DNA/Takes). Graceful osascript quit; kill only as the last resort.
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists AND its first field
# equals AUDIODNA_LOCK_OWNER. Every pgrep/pkill is adna_pids/adna_running/adna_kill (ucomm-based,
# see probe-resync.sh's header for why).
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held (/tmp/audiodna-live.lock/owner)"; exit 64; }
[ -n "${AUDIODNA_LOCK_OWNER:-}" ] || { echo "REFUSE: AUDIODNA_LOCK_OWNER is not set"; exit 64; }
LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
[ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }

OUT="${1:-/tmp/audiodna-tempo-start}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${TEMPOSTART_BUILD_DIR:-build-lane}"
APPBUNDLE="${TEMPOSTART_APP:-$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app}"
VENV_PY="${TEMPOSTART_VENV_PY:-$ROOT/.venv/bin/python}"
CYCLES="${TEMPOSTART_CYCLES:-20}"
STALL_MS="${TEMPOSTART_STALL_MS:-40}"
WITNESS_RUNS="${TEMPOSTART_WITNESS_RUNS:-0}"
A='http://127.0.0.1:7070'
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
RUNID="$(date +%Y%m%d%H%M%S)$$"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

launch() {  # $1 = log tag
    : > "$OUT/adna-out-$1.log"; : > "$OUT/adna-err-$1.log"
    open -g --stdout "$OUT/adna-out-$1.log" --stderr "$OUT/adna-err-$1.log" "$APPBUNDLE"
    for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && return 0; sleep 1; done
    return 1
}
quit_adna() {
    osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
    for _ in $(seq 1 30); do adna_running || break; sleep 1; done
    if adna_running; then
        echo "graceful osascript quit did not clear the process -- falling back to kill (last resort)"
        adna_kill
        for _ in $(seq 1 20); do adna_running || break; sleep 1; done
    fi
}
cleanup_takes() { rm -rf "$TAKES_DIR"/tstart-"$RUNID"-*.adna-take 2>/dev/null; }

adna_running && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no app at $APPBUNDLE (set TEMPOSTART_APP or TEMPOSTART_BUILD_DIR)"; exit 64; }
echo "app under test: $APPBUNDLE"
echo "run id $RUNID  cycles $CYCLES  stall ${STALL_MS} ms  witness runs $WITNESS_RUNS  $(date '+%F %T')"

# --- the python client (stdlib only) ------------------------------------------------------------------
PYC="$OUT/tempo_start_client.py"
cat > "$PYC" <<'PY'
import json, os, socket, sys, time, urllib.request, urllib.error

HOST, PORT = '127.0.0.1', 7070
TAKES = os.path.expanduser('~/Documents/Audio-DNA/Takes')

def connect():
    return socket.create_connection((HOST, PORT), timeout=5)

def send(sock, path, body):
    b = json.dumps(body).encode()
    head = ('POST %s HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: application/json\r\n'
            'Content-Length: %d\r\nConnection: close\r\n\r\n' % (path, HOST, PORT, len(b))).encode()
    sock.sendall(head + b)

def answer(sock):
    data = b''
    while True:
        chunk = sock.recv(65536)
        if not chunk:
            break
        data += chunk
    sock.close()
    head, _, body = data.partition(b'\r\n\r\n')
    status = int(head.split(b' ')[1]) if head else 0
    return status, body.decode(errors='replace')

def post(path, body):
    s = connect(); send(s, path, body); return answer(s)

def get(path):
    req = urllib.request.Request('http://%s:%d%s' % (HOST, PORT, path), headers={'Connection': 'close'})
    with urllib.request.urlopen(req, timeout=5) as r:
        return json.loads(r.read().decode())

def pair(cmd_path, cmd_body, name, stall_ms):
    """stall (optional) -> command -> record, on pre-connected sockets. Returns the command->record gap (ms)."""
    s_stall = connect() if stall_ms > 0 else None
    s_cmd, s_rec = connect(), connect()
    if s_stall is not None:
        send(s_stall, '/api/debug/stall_message_thread', {'ms': stall_ms})
        st, _ = answer(s_stall)
        if st != 200:
            raise RuntimeError('stall hook answered %d' % st)
    t0 = time.perf_counter()
    send(s_cmd, cmd_path, cmd_body)
    st, _ = answer(s_cmd)
    send(s_rec, '/api/perf/record', {'name': name, 'audio': False})
    t1 = time.perf_counter()
    rs, rb = answer(s_rec)
    if st != 200 or rs != 200:
        raise RuntimeError('command %d / record %d %s' % (st, rs, rb))
    return (t1 - t0) * 1000.0

def stop_and_read(name, timeout=3.0):
    post('/api/perf/stop', {})
    t_end = time.time() + timeout
    while time.time() < t_end:
        if not get('/api/perf/status').get('recording', True):
            break
        time.sleep(0.02)
    folder = os.path.join(TAKES, name + '.adna-take')
    with open(os.path.join(folder, 'take.json')) as f:
        return folder, json.load(f)

def facts(take):
    tm = take.get('tempoMap') or []
    first = tm[0] if tm else {}
    return {
        'start_why': first.get('why'), 'start_bpm': first.get('bpm'),
        'lock': any(a.get('why') == 'lock' for a in tm),
        'early_bpm_anchor': any(a.get('why') == 'bpm' and a.get('t', 1.0) < 0.05 for a in tm),
        'sbib': take.get('meta', {}).get('startBeatInBar'),
        'anchors': [(a.get('why'), round(a.get('bpm', 0.0), 2), round(a.get('t', 0.0), 4)) for a in tm],
    }

def cycle(cmd_path, cmd_body, name, stall_ms, hold=0.3):
    gap = pair(cmd_path, cmd_body, name, stall_ms)
    time.sleep(hold)
    folder, take = stop_and_read(name)
    return gap, folder, facts(take)

def near(x, v, tol=0.05):
    return x is not None and abs(x - v) <= tol

def main():
    mode, runid, cycles, stall = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
    if mode == 'w1':
        bpm0 = get('/api/bpm').get('bpm')
        if bpm0 is None or abs(bpm0) > 1e-6:
            print('SKIP  W1 precondition: /api/bpm bpm is %s, not 0 (the tracker was already metered)' % bpm0)
            return
        name = 'tstart-%s-w1' % runid
        gap, folder, f = cycle('/api/set_bpm', {'bpm': 120}, name, stall, hold=0.5)
        print('INFO  W1 gap %.2f ms anchors %s startBeatInBar %s' % (gap, f['anchors'], f['sbib']))
        good = (f['start_why'] == 'start' and near(f['start_bpm'], 120.0) and not f['lock']
                and f['sbib'] is not None and 0.0 <= f['sbib'] < 4.0)
        st, body = post('/api/perf/load', {'folder': folder})
        time.sleep(0.2)
        st2, body2 = post('/api/routine/save', {'name': 'TStart', 'fromBeat': 0, 'toBeat': 4, 'slot': 7})
        saved, err = None, None
        t_end = time.time() + 1.0
        while time.time() < t_end:
            rs = get('/api/routine/status')
            saved, err = (rs.get('lastSaved') or {}).get('slot'), rs.get('lastError')
            if saved == 7 or err:
                break
            time.sleep(0.05)
        print('INFO  W1 routine save (beats 0-4, slot 7): lastSaved.slot %s lastError "%s"' % (saved, err))
        good = good and saved == 7 and err == ''
        print('%s  W1 first take after launch: start anchor %s bpm %s, lock anchor %s, startBeatInBar %s, routine from beat 0 %s'
              % ('PASS' if good else 'FAIL', f['start_why'], f['start_bpm'], f['lock'], f['sbib'],
                 'saved' if saved == 7 and err == '' else 'refused (%s)' % err))
    elif mode == 'w1b':
        a = get('/api/bpm'); time.sleep(1.0); b = get('/api/bpm')
        good = near(b.get('bpm'), 120.0) and (b.get('totalBeatCount', 0) - a.get('totalBeatCount', 0)) >= 1
        print('%s  W1b liveness: bpm %s, totalBeatCount %s -> %s in 1 s%s'
              % ('PASS' if good else 'FAIL', b.get('bpm'), a.get('totalBeatCount'), b.get('totalBeatCount'),
                 '' if good else ' -- no analysis hops (no audio device?): every other row is meaningless'))
    elif mode == 'w2':
        passed = 0
        for i in range(cycles):
            v = 90.0 if i % 2 == 0 else 150.0
            gap, _, f = cycle('/api/set_bpm', {'bpm': v}, 'tstart-%s-w2-%02d' % (runid, i), stall)
            hit = f['start_why'] == 'start' and near(f['start_bpm'], v)
            passed += hit
            print('  W2 %02d sent %5.1f gap %6.2f ms start bpm %-6s early "bpm" anchor %-5s %s'
                  % (i, v, gap, f['start_bpm'], f['early_bpm_anchor'], 'ok' if hit else 'MISS'))
        print('%s  W2 start anchor carries the set_bpm sent just before Record: %d/%d'
              % ('PASS' if passed == cycles else 'FAIL', passed, cycles))
    elif mode == 'w3':
        post('/api/set_bpm', {'bpm': 120}); time.sleep(1.0)
        passed = 0
        for i in range(cycles):
            gap, _, f = cycle('/api/resync', {}, 'tstart-%s-w3-%02d' % (runid, i), stall)
            hit = (f['sbib'] is not None and 0.0 <= f['sbib'] < 0.25 and near(f['start_bpm'], 120.0))
            passed += hit
            print('  W3 %02d gap %6.2f ms start bpm %-6s startBeatInBar %-22s %s'
                  % (i, gap, f['start_bpm'], f['sbib'], 'ok' if hit else 'MISS'))
        print('%s  W3 a Resync sent just before Record is beat 0 of bar 1 (startBeatInBar in [0, 0.25)): %d/%d'
              % ('PASS' if passed == cycles else 'FAIL', passed, cycles))
    elif mode == 'hook':
        st, body = post('/api/debug/stall_message_thread', {'ms': 1})
        print(st)
    elif mode == 'witness':
        name = 'tstart-%s-w6-%s' % (runid, sys.argv[5])
        post('/api/set_bpm', {'bpm': 120})
        post('/api/perf/record', {'name': name, 'audio': False})
        time.sleep(1.5)
        post('/api/perf/stop', {})
        time.sleep(1.0)
        with open(os.path.join(TAKES, name + '.adna-take', 'take.json')) as fh:
            f = facts(json.load(fh))
        print('W6 %s start bpm %s startBeatInBar %s anchors %s' % (sys.argv[5], f['start_bpm'], f['sbib'], f['anchors']))

if __name__ == '__main__':
    main()
PY

# --- W1-W4 on one fresh launch -------------------------------------------------------------------------
launch main || { no "health never came up on $A"; quit_adna; exit 1; }
ok "app launched (production), /api/health answered, PID=$(adna_pids | head -1)"
sleep 2
HOOK="$(python3 "$PYC" hook "$RUNID" 0 0 2>&1)"
if [ "$STALL_MS" -gt 0 ]; then
    if [ "$HOOK" != "200" ]; then
        echo "BLOCKED: stall hook unavailable (/api/debug/stall_message_thread answered $HOOK) -- W1-W3 need the stall-assisted pair (HARMONY ADOPTION A2)"
        quit_adna; cleanup_takes; exit 2
    fi
    echo "mode: STALL-ASSISTED (${STALL_MS} ms message-thread sleep before each pair)"
else
    echo "mode: TIGHT PAIR (natural timing; an extra run, never the RED/GATE mode)"
fi
# The hook probe above slept the message thread for 1 ms only; the tracker is untouched.
RES="$(python3 "$PYC" w1 "$RUNID" 1 "$STALL_MS" 2>&1)"; echo "$RES"
echo "$RES" | grep -q '^PASS  W1' && ok "W1 (see above)"; echo "$RES" | grep -q '^FAIL  W1' && no "W1 (see above)"
echo "$RES" | grep -q '^SKIP  W1' && echo "SKIP  W1 (precondition)"
echo "$RES" | grep -q 'Traceback' && no "W1 client error"
if ! echo "$RES" | grep -q '^PASS  W1\|^FAIL  W1'; then   # W1 skipped / errored: the tracker still needs a tempo for W1b
    python3 -c "import urllib.request,json;urllib.request.urlopen(urllib.request.Request('$A/api/set_bpm',data=json.dumps({'bpm':120}).encode(),headers={'Content-Type':'application/json','Connection':'close'}),timeout=5).read()"
    sleep 1
fi
RES="$(python3 "$PYC" w1b "$RUNID" 1 "$STALL_MS" 2>&1)"; echo "$RES"
if echo "$RES" | grep -q '^PASS  W1b'; then
    ok "W1b liveness"
    for ROW in w2 w3; do
        RES="$(python3 "$PYC" $ROW "$RUNID" "$CYCLES" "$STALL_MS" 2>&1)"; echo "$RES"
        U="$(echo $ROW | tr a-z A-Z)"
        if echo "$RES" | grep -q "^PASS  $U"; then ok "$U (see above)"; else no "$U (see above)"; fi
    done
else
    no "W1b liveness -- W2/W3 not run (no analysis hops)"
fi
sleep 0.5
N4="$(grep -c 'take start:' "$OUT/adna-err-main.log" 2>/dev/null)"; N4="${N4:-0}"
[ "$N4" = "0" ] && ok "W4 0 \"take start:\" lines in the app's stderr (every take started by the handshake)" \
    || { no "W4 $N4 \"take start:\" line(s) in the app's stderr:"; grep 'take start:' "$OUT/adna-err-main.log" | head -5; }
quit_adna

# --- W6 (opt-in): the canonical witness, a fresh launch per run -----------------------------------------
if [ "$WITNESS_RUNS" -gt 0 ]; then
    : > "$OUT/w6.txt"
    for i in $(seq -w 1 "$WITNESS_RUNS"); do
        if ! launch "w6-$i"; then no "W6 run $i: health never came up"; quit_adna; continue; fi
        sleep 2
        python3 "$PYC" witness "$RUNID" 1 0 "$i" 2>&1 | tee -a "$OUT/w6.txt"
        quit_adna
    done
    W6N="$(grep -c '^W6 ' "$OUT/w6.txt")"
    W6B0="$(grep -c '^W6 .* start bpm 0.0 ' "$OUT/w6.txt")"
    W6UG="$(grep -c '^W6 .* startBeatInBar None ' "$OUT/w6.txt")"
    W6TS="$(cat "$OUT"/adna-err-w6-*.log 2>/dev/null | grep -c 'take start:')"
    [ "$W6N" = "$WITNESS_RUNS" ] && [ "$W6B0" = "0" ] && [ "$W6UG" = "0" ] \
        && ok "W6 witness x$WITNESS_RUNS: start-bpm-0 $W6B0/$W6N, unknown grid $W6UG/$W6N (take start: lines $W6TS)" \
        || no "W6 witness x$WITNESS_RUNS: completed $W6N, start-bpm-0 $W6B0/$W6N, unknown grid $W6UG/$W6N (take start: lines $W6TS)"
fi

# --- teardown -------------------------------------------------------------------------------------------
cleanup_takes
LEFT="$(ls -d "$TAKES_DIR"/tstart-"$RUNID"-* 2>/dev/null | wc -l | tr -d ' ')"
[ "$LEFT" = "0" ] && ok "the probe's takes were removed (tstart-$RUNID-*)" || no "$LEFT probe take(s) remain under $TAKES_DIR"
adna_running && no "APP STILL RUNNING AFTER graceful quit + kill fallback" || ok "app terminated, no process remains"
if [ -x "$VENV_PY" ]; then
    W="$("$VENV_PY" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))
           and 'Output' in str(w.get('kCGWindowName',''))]))" 2>/dev/null)"
    if [ "$W" = "0" ] || [ -z "$W" ]; then ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else no "$W Audio-DNA Output window(s) -- screen-safety breach"; fi
else
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set TEMPOSTART_VENV_PY"
fi
echo; echo "PASS $PASS / FAIL $FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
