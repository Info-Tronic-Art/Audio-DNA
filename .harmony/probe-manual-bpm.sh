#!/bin/bash
# probe-manual-bpm.sh -- s-rta-0926 lane manual-bpm live gate: in Manual BPM mode the beat phase
# free-runs from the manual BPM; a beat the analysis detects in the audio NEVER moves it. The only
# manual realignments are Resync (POST /api/resync, OSC /audiodna/resync, TopBar Resync) and Tap --
# a typed / REST / OSC set_bpm only changes the tempo, never the beat (s-rta-0926b plan3 A; mode setbpm).
#
# BUG this pins (routine-grid lane, .harmony/notebook.md s-rta-0926 routine-grid): BPMTracker's
# manual branch still ran updatePhase()'s hard reset on an aubio beat at confidence >= 0.5, so at a
# manual 120 BPM with rhythmic sound at another tempo the phase reset every detected beat and bars
# stretched to 11-13 s.
#
# ORACLE: the app's own FILE mode plays a click track at 142 BPM (gen-click-wav.py, interval 20282
# frames @ 48 kHz) into the analysis thread -- deterministic, no mic, no room noise. POST
# /api/perf/record with audioFile is the only REST route that loads a file and starts the transport
# (probe-step3.sh does the same); the take is armed with "audio": false, stopped at once (the
# transport keeps playing -- disarm never touches it), and its folder is deleted at teardown.
#   A  AUTO sanity (fresh launch, never manual): the tracker LOCKS on the click (generous: /api/bpm
#      within 4 of 142 or 71 for 3 s straight). Run FIRST because no REST/OSC route leaves manual mode (set_bpm
#      and OSC bpm only ever enter it; "auto" is the TopBar toggle, which this probe must not click).
#      The "manual -> auto still resets on beats" half is ctest-pinned (test_bpm_stabilization.cpp,
#      "Tap still realigns in manual mode; leaving manual mode restores the AUTO beat reset").
#   M  POST /api/set_bpm 120, then poll /api/bpm ~50 Hz for MANUALBPM_POLL_S (20) s while the 142
#      BPM click keeps playing: bpm stays 120.0; the analysis really hears the click (onsetCount
#      advances); beatPhase advances at 2.00 beats/s with ZERO jumps (a jump = a poll whose circular
#      phase step differs from 2 beats/s * dt by > 0.15); every bar (totalBarCount edge) lasts
#      2.00 s +/- 3%.
#   R  POST /api/resync: the first poll >= 60 ms after it is the new downbeat (beatInBar 0, barCount
#      0, beatPhase < 0.25, resyncBarOrigin == totalBarCount); resyncBarOrigin changes exactly once;
#      from 300 ms after the POST ZERO phase jumps; the first bar after the Resync lands 2.0 s later
#      (1.90-2.12 s incl. the message-thread hop) and every later bar lasts 2.00 s +/- 3%.
# RED on a pre-fix build (main at 2bf1d56): M and R jump rows fail (one jump per detected beat) and
# the bar rows fail (bars 11-13 s or none at all).
#
# Environment:
#   MANUALBPM_APP        full path to Audio-DNA.app (default: $ROOT/$MANUALBPM_BUILD_DIR/AudioDNA_artefacts/
#                        Release/Audio-DNA.app, MANUALBPM_BUILD_DIR default "build-lane")
#   MANUALBPM_PY         python with pyobjc Quartz for the Output-window witness (default $ROOT/.venv/bin/python)
#   MANUALBPM_POLL_S     manual-mode observation window in seconds (default 20)
#   MANUALBPM_CLICK_BPM  click tempo (default 142 -- anything clearly != 120 and aubio-trackable)
# Artifacts: a FRESH directory per run under ${1:-/tmp/audiodna-manual-bpm}. Exit 1 on any FAIL.
#
# SCREEN-SAFETY LAW: `open -g` only, never the Output window (no endpoint used here reaches it),
# never a full-screen capture; graceful osascript quit first, pkill only as the last resort. The
# caller holds the live lock (/tmp/audiodna-live.lock, now enforced -- see PROBE RIG GATE below). The click is audible on the default output
# device for ~1 minute (same as probe-step3.sh).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner
# file's first field. Every pgrep/pkill below is replaced by adna_pids/adna_running/adna_kill
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
# run -- immune to argv[0] spoofing) is exactly "Audio-DNA". adna_running/adna_kill build on it.
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
OUT_BASE="${1:-/tmp/audiodna-manual-bpm}"
OUT="$OUT_BASE/run-$(date +%Y%m%d-%H%M%S)-$$"
mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${MANUALBPM_BUILD_DIR:-build-lane}"
APPBUNDLE="${MANUALBPM_APP:-$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app}"
VENV_PY="${MANUALBPM_PY:-$ROOT/.venv/bin/python}"
POLL_S="${MANUALBPM_POLL_S:-20}"
CLICK_BPM="${MANUALBPM_CLICK_BPM:-142}"
MANUAL_BPM=120
A='http://127.0.0.1:7070'
CLICK_WAV="$OUT/click-${CLICK_BPM}bpm.wav"
TAKES_DIR="$HOME/Documents/Audio-DNA/Takes"
TAKE_NAME="probe-manual-bpm-$(date +%Y%m%d-%H%M%S)-$$"
TAKE_FOLDER="$TAKES_DIR/$TAKE_NAME.adna-take"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
count_rows(){ # $1 = verdict file: add its PASS/FAIL lines to the totals
  cat "$1"
  PASS=$((PASS + $(grep -c '^PASS ' "$1"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$1")))
}

# --- 0. click WAV (stdlib python) + preconditions ------------------------------------------------
INTERVAL="$(python3 -c "print(round(48000*60/$CLICK_BPM))")"
python3 "$ROOT/.harmony/gen-click-wav.py" "$CLICK_WAV" --interval "$INTERVAL" --duration-s 150 >/dev/null \
  && ok "click WAV generated: $CLICK_BPM BPM (interval $INTERVAL frames @ 48 kHz), 150 s" \
  || { echo "REFUSE: click WAV generation failed"; exit 64; }
adna_running && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set MANUALBPM_APP or MANUALBPM_BUILD_DIR)"; exit 64; }
[ -e "$TAKE_FOLDER" ] && { echo "REFUSE: $TAKE_FOLDER already exists"; exit 64; }
echo "INFO  app: $APPBUNDLE"

# --- 1. production launch (probe-resync.sh section 1) ---------------------------------------------
: > "$OUT/adna-out.log"; : > "$OUT/adna-err.log"      # open --stdout/--stderr APPEND: clear first
open -g --stdout "$OUT/adna-out.log" --stderr "$OUT/adna-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
if [ -z "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ]; then
    echo "FAIL: health never came up on $A -- STOP, do not click anything; report (TCC prompt?)"
    osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
    exit 1
fi
PID="$(adna_pids | head -1)"
[ -n "$PID" ] && ok "app launched (production), /api/health answered, PID=$PID" || no "health answered but no Audio-DNA process found"
sleep 3

# --- 2. file-mode click into the analysis thread ---------------------------------------------------
curl -s --max-time 6 -X POST "$A/api/perf/record" -H 'Content-Type: application/json' \
  -d "{\"name\":\"$TAKE_NAME\",\"audio\":false,\"audioFile\":\"$CLICK_WAV\"}" | grep -q '"ok":[[:space:]]*true' \
  && ok "perf/record accepted the click file (file mode, transport playing)" || no "perf/record refused the click file"
sleep 1
curl -s --max-time 6 -X POST "$A/api/perf/stop" >/dev/null    # the take is not the point; the transport keeps playing
sleep 1

# --- 3. the rows (one python helper, stdlib only) --------------------------------------------------
cat > "$OUT/mb.py" <<'PY'
import json, math, sys, time, urllib.request, urllib.error
A = 'http://127.0.0.1:7070'
OUT = sys.argv[2]

def get(path, timeout=2):
    with urllib.request.urlopen(A + path, timeout=timeout) as r:
        return json.loads(r.read().decode())

def post(path, body=None, timeout=6):
    data = json.dumps(body).encode() if body is not None else None
    req = urllib.request.Request(A + path, data=data, method='POST',
                                 headers={'Content-Type': 'application/json'} if data else {})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode())

def v(cond, msg):
    print(('PASS  ' if cond else 'FAIL  ') + msg, flush=True)

def poll(seconds, period=0.02):
    rows = []
    t_end = time.monotonic() + seconds
    while time.monotonic() < t_end:
        t = time.monotonic()
        try:
            d = get('/api/bpm')
            rows.append({'t': t, 'bpm': d['bpm'], 'ph': d['beatPhase'], 'bib': d['beatInBar'],
                         'bc': d['barCount'], 'tbc': d['totalBarCount'], 'org': d['resyncBarOrigin'],
                         'db': d['downbeatDetected']})
        except Exception as e:
            rows.append({'t': t, 'err': str(e)})
        time.sleep(period)
    return [r for r in rows if 'err' not in r], sum(1 for r in rows if 'err' in r)

def jumps(rows, beats_per_s, tol=0.15):
    """Polls whose circular phase step differs from beats_per_s*dt by more than tol."""
    bad = []
    for a, b in zip(rows, rows[1:]):
        exp = beats_per_s * (b['t'] - a['t'])
        step = (b['ph'] - a['ph']) % 1.0
        err = abs(((step - exp) + 0.5) % 1.0 - 0.5)
        if err > tol:
            bad.append((round(b['t'] - rows[0]['t'], 3), round(a['ph'], 3), round(b['ph'], 3)))
    return bad

def phase_rate(rows):
    tot = sum((b['ph'] - a['ph']) % 1.0 for a, b in zip(rows, rows[1:]))
    dt = rows[-1]['t'] - rows[0]['t']
    return tot / dt if dt > 0 else float('nan')

def bar_edges(rows):
    """Edge time = midpoint between the last poll before and the first poll after the increment."""
    return [0.5 * (a['t'] + b['t']) for a, b in zip(rows, rows[1:]) if b['tbc'] != a['tbc']]

def durations(edges):
    return [round(y - x, 3) for x, y in zip(edges, edges[1:])]

mode = sys.argv[1]
click_bpm = float(sys.argv[3]); manual_bpm = float(sys.argv[4]); poll_s = float(sys.argv[5])
bps = manual_bpm / 60.0
bar_s = 4.0 / bps
lo, hi = bar_s * 0.97, bar_s * 1.03

if mode == 'auto':
    # A: AUTO sanity -- generous: /api/bpm (the tracker's LOCKED bpm; 0 until the first lock -- trackerState
    # is not on REST) holds within 4 BPM of the click tempo (or its half) for 3 s straight, within 30 s.
    t0 = time.monotonic(); t_end = t0 + 30.0; run = 0; last = None; locked_at = None
    cands = (click_bpm, click_bpm / 2.0)
    while time.monotonic() < t_end:
        b = get('/api/bpm')['bpm']; last = round(b, 2)
        run = run + 1 if any(abs(b - c) <= 4.0 for c in cands) else 0
        if run >= 12:
            locked_at = time.monotonic() - t0
            break
        time.sleep(0.25)
    v(locked_at is not None, "A  AUTO sanity: the tracker locks on the %.0f BPM click and holds 3 s (last bpm=%s%s)"
      % (click_bpm, last, (', held by %.1f s' % locked_at) if locked_at is not None else ', never within 30 s'))

elif mode == 'manual':
    post('/api/set_bpm', {'bpm': manual_bpm})
    time.sleep(1.5)
    o0 = get('/api/features').get('onsetCount')
    rows, errs = poll(poll_s)
    o1 = get('/api/features').get('onsetCount')
    json.dump(rows, open(OUT + '/manual-polls.json', 'w'))
    n = len(rows)
    v(n >= 25 * poll_s and errs == 0, "M  poller healthy: %d polls in %.0f s, %d errors" % (n, poll_s, errs))
    bpms = sorted(set(round(r['bpm'], 3) for r in rows))
    v(bpms == [round(manual_bpm, 3)], "M  bpm stays %.1f on every poll (seen %s)" % (manual_bpm, bpms[:6]))
    onsets = (o1 - o0) if isinstance(o0, int) and isinstance(o1, int) else None
    exp_onsets = click_bpm / 60.0 * poll_s
    v(onsets is not None and onsets >= 0.6 * exp_onsets,
      "M  non-vacuous: the analysis heard the click (onsetCount +%s over the window, ~%.0f clicks played)"
      % (onsets, exp_onsets))
    bad = jumps(rows, bps)
    v(len(bad) == 0, "M  beatPhase free-runs: %d jumps (first: %s)" % (len(bad), bad[:3]))
    rate = phase_rate(rows)
    v(abs(rate - bps) <= 0.03 * bps, "M  beatPhase advances at %.3f beats/s (expect %.2f +/- 3%%)" % (rate, bps))
    d = durations(bar_edges(rows))
    exp_bars = poll_s / bar_s
    v(len(d) >= int(exp_bars) - 2 and all(lo <= x <= hi for x in d),
      "M  every bar lasts %.2f s +/- 3%% (%d full bars in %.0f s, expect ~%d): %s"
      % (bar_s, len(d), poll_s, int(exp_bars) - 1, d))

elif mode == 'resync':
    before = get('/api/bpm')
    t_post = time.monotonic()
    post('/api/resync')
    rows, errs = poll(10.0)
    json.dump({'t_post': t_post, 'before': before, 'rows': rows}, open(OUT + '/resync-polls.json', 'w'))
    after = [r for r in rows if r['t'] - t_post >= 0.06]
    if not after:
        v(False, "R  no poll landed after the Resync")
        sys.exit(0)
    s0 = after[0]
    v(s0['t'] - t_post <= 0.25 and s0['bib'] == 0 and s0['bc'] == 0 and s0['ph'] < 0.25
      and s0['org'] == s0['tbc'] and s0['org'] != before['resyncBarOrigin'],
      "R  first poll %.0f ms after the Resync is the new downbeat (beatInBar=%s barCount=%s beatPhase=%.3f "
      "origin %s -> %s, totalBarCount=%s)" % (1000 * (s0['t'] - t_post), s0['bib'], s0['bc'], s0['ph'],
                                              before['resyncBarOrigin'], s0['org'], s0['tbc']))
    origins = sorted(set(r['org'] for r in after))
    v(len(origins) == 1, "R  a single realignment: resyncBarOrigin took %d value(s) after the POST %s" % (len(origins), origins))
    settled = [r for r in rows if r['t'] - t_post >= 0.3]
    bad = jumps(settled, bps)
    v(len(bad) == 0, "R  beatPhase free-runs after the Resync: %d jumps (first: %s)" % (len(bad), bad[:3]))
    edges = bar_edges(rows)
    first = (edges[0] - t_post) if edges else None
    v(first is not None and bar_s * 0.95 <= first <= bar_s * 1.06,
      "R  first bar after the Resync lands %.3f s after the POST (expect %.2f s + message-thread hop, [%.2f, %.2f])"
      % (first if first is not None else -1, bar_s, bar_s * 0.95, bar_s * 1.06))
    d = durations(edges)
    v(len(d) >= 3 and all(lo <= x <= hi for x in d),
      "R  bars after the Resync last %.2f s +/- 3%%: %s" % (bar_s, d))
    bpms = sorted(set(round(r['bpm'], 3) for r in rows))
    v(bpms == [round(manual_bpm, 3)], "R  bpm stays %.1f (seen %s)" % (manual_bpm, bpms[:6]))

elif mode == 'setbpm':
    # S (s-rta-0926b plan3 A): a tempo VALUE -- REST /api/set_bpm, OSC /audiodna/bpm, the typed BPM --
    # never moves the beat; only Tap and Resync realign it. Each POST lands mid-beat (beatPhase in
    # [0.40, 0.60]); the first poll >= 60 ms later must continue the beat: its bar position
    # (barCount*4 + beatInBar + beatPhase) moved by bps*dt within 0.06 beat -- phase, beat-in-bar and
    # bar count all unmoved by the request. RED before plan3 A: every set_bpm realigned the phase to 0.
    import socket, struct, threading

    def one():
        t0 = time.monotonic()
        d = get('/api/bpm')
        t1 = time.monotonic()
        return {'t': 0.5 * (t0 + t1), 'bpm': d['bpm'], 'ph': d['beatPhase'], 'bib': d['beatInBar'],
                'bc': d['barCount'], 'tbc': d['totalBarCount']}

    def mid_beat(timeout=3.0):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            r = one()
            if 0.40 <= r['ph'] <= 0.60:
                return r
            time.sleep(0.005)
        return None

    def barpos(r):
        return r['bc'] * 4 + r['bib'] + r['ph']

    def set_value(bpm):
        """POST set_bpm mid-beat; returns (pre, t_post, post, err) -- err = |bar-position step - bpm/60*dt|."""
        pre = mid_beat()
        if pre is None:
            return None, None, None, None
        t_post = time.monotonic()
        post('/api/set_bpm', {'bpm': bpm})
        post_row = None
        while time.monotonic() - t_post < 0.5:
            r = one()
            if r['t'] - t_post >= 0.06:
                post_row = r
                break
            time.sleep(0.005)
        if post_row is None:
            return pre, t_post, None, None
        err = abs((barpos(post_row) - barpos(pre)) - (bpm / 60.0) * (post_row['t'] - pre['t']))
        return pre, t_post, post_row, err

    def trial(label, bpm):
        pre, t_post, p, err = set_value(bpm)
        if p is None:
            return False, '%s: no mid-beat poll or no poll after the POST' % label
        rows, errs = poll(1.0)
        bad = jumps(rows, bpm / 60.0)
        bpms = sorted(set(round(r['bpm'], 3) for r in [p] + rows))
        good = (p['t'] - t_post <= 0.25 and err <= 0.06 and bpms == [round(bpm, 3)] and not bad and errs == 0)
        return good, ('%s: POST at phase %.3f (beat %d, bar %d) -> +%.0f ms phase %.3f (beat %d, bar %d), step error '
                      '%.3f beat, then 1 s: %d jumps, bpm %s' % (label, pre['ph'], pre['bib'], pre['bc'],
                      1000 * (p['t'] - t_post), p['ph'], p['bib'], p['bc'], err, len(bad), bpms))

    # S1: the same value, 3 times.
    res = [trial('#%d' % (i + 1), manual_bpm) for i in range(3)]
    json.dump([r[1] for r in res], open(OUT + '/setbpm-s1.json', 'w'))
    n_ok = sum(1 for r in res if r[0])
    v(n_ok == 3, "S1 set_bpm %.0f (the SAME value) never moves the beat: %d/3 continuous -- %s"
      % (manual_bpm, n_ok, ' | '.join(r[1] for r in res)))

    # S2: a changed value, and back.
    up = trial('%.0f -> 128' % manual_bpm, 128.0)
    v(up[0], "S2 set_bpm 128 (a CHANGED value) changes the tempo only, the beat runs on -- " + up[1])
    down = trial('128 -> %.0f' % manual_bpm, manual_bpm)
    v(down[0], "S2 set_bpm %.0f (changed back) changes the tempo only, the beat runs on -- %s" % (manual_bpm, down[1]))

    # S3: a value stream -- OSC /audiodna/bpm every 100 ms for 2 s (the probe-mastersignal.sh B3 UDP idiom).
    addr = b'/audiodna/bpm\0'
    addr += b'\0' * ((4 - len(addr) % 4) % 4)
    msg = addr + b',f\0\0' + struct.pack('>f', manual_bpm)
    sent = [0]
    def stream():
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        t_next = time.monotonic()
        for _ in range(20):
            s.sendto(msg, ('127.0.0.1', 8000))
            sent[0] += 1
            t_next += 0.1
            time.sleep(max(0.0, t_next - time.monotonic()))
    th = threading.Thread(target=stream)
    th.start()
    rows, errs = poll(2.4)
    th.join()
    json.dump(rows, open(OUT + '/setbpm-s3.json', 'w'))
    bad = jumps(rows, bps)
    bpms = sorted(set(round(r['bpm'], 3) for r in rows))
    adv = (rows[-1]['tbc'] - rows[0]['tbc']) if rows else 0
    v(sent[0] == 20 and errs == 0 and bpms == [round(manual_bpm, 3)] and not bad and adv >= 1,
      "S3 OSC /audiodna/bpm %.0f every 100 ms for 2 s (%d sent): the beat free-runs -- %d jumps (first: %s), "
      "bpm %s, totalBarCount +%d (>= 1)" % (manual_bpm, sent[0], len(bad), bad[:3], bpms, adv))
PY

"$VENV_PY" -c 'pass' 2>/dev/null && PY="$VENV_PY" || PY=python3
"$PY" "$OUT/mb.py" auto   "$OUT" "$CLICK_BPM" "$MANUAL_BPM" "$POLL_S" > "$OUT/rows-auto.txt" 2>&1;   count_rows "$OUT/rows-auto.txt"
"$PY" "$OUT/mb.py" manual "$OUT" "$CLICK_BPM" "$MANUAL_BPM" "$POLL_S" > "$OUT/rows-manual.txt" 2>&1; count_rows "$OUT/rows-manual.txt"
"$PY" "$OUT/mb.py" setbpm "$OUT" "$CLICK_BPM" "$MANUAL_BPM" "$POLL_S" > "$OUT/rows-setbpm.txt" 2>&1; count_rows "$OUT/rows-setbpm.txt"
"$PY" "$OUT/mb.py" resync "$OUT" "$CLICK_BPM" "$MANUAL_BPM" "$POLL_S" > "$OUT/rows-resync.txt" 2>&1; count_rows "$OUT/rows-resync.txt"
grep -q 'Traceback' "$OUT"/rows-*.txt && no "a python helper crashed (see $OUT/rows-*.txt)"
echo "INFO  'leave manual mode -> AUTO resets on beats again' is not drivable over REST/OSC (no route leaves manual mode); ctest-pinned instead."

# --- 4. teardown (probe-resync.sh section 8) -------------------------------------------------------
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
if adna_running; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    adna_kill
    for _ in $(seq 1 20); do adna_running || break; sleep 1; done
fi
adna_running && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"
case "$TAKE_FOLDER" in
    "$TAKES_DIR"/probe-manual-bpm-*.adna-take) [ -d "$TAKE_FOLDER" ] && rm -rf "$TAKE_FOLDER" && echo "INFO  removed this run's take folder $TAKE_FOLDER" ;;
esac
if [ -x "$VENV_PY" ]; then
    W="$("$VENV_PY" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))
           and 'Output' in str(w.get('kCGWindowName',''))]))" 2>/dev/null)"
    if [ "$W" = "0" ] || [ -z "$W" ]; then ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else no "$W Audio-DNA Output window(s) were open during the run -- screen-safety breach"; fi
else
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set MANUALBPM_PY to the main checkout's .venv/bin/python"
fi
echo "no full-screen capture: the Quartz window-list check above is the screen witness"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
