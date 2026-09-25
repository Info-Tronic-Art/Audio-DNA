#!/bin/bash
# probe-downbeat-level.sh -- live witness for s-rta-0925 roadmap item 4: downbeatDetected is a
# beat-long LEVEL (not a one-hop pulse), totalBarCount is its exact per-bar counter, a UI-cadence
# poller loses nothing, and a poller slower than a beat loses rising edges while the counter does
# not. Recipe: .harmony/.reports/s-rta-0925/plan-downbeat.md section 5.
# Written by the Builder, NOT run by the Builder. Validate: bash -n; shellcheck (if installed).
#
# ORACLE (no audio content needed): /api/set_bpm puts BPMTracker in manual mode; its predicted
# phase wraps cycle beatCounter_, so downbeatDetected_ = (beatCounter_ == 0) is held for one whole
# beat (500 ms at 120 BPM) and totalBarCount advances once per 4 wraps (2.0 s). A ~25 Hz poller of
# /api/bpm (level + counter from ONE coherent snapshot per poll) must see: duty ~0.25 with true-runs
# ~500 ms (LEVEL -- a one-hop pulse would read duty ~0.03 and runs <= 45 ms); rising edges ==
# totalBarCount delta (no loss at UI cadence); downbeatDetected == (beatInBar == 0) on every poll.
# The same stream subsampled at 0.9 s (slower than a beat) must LOSE rising edges while its
# totalBarCount delta still counts every bar. Pre-fix build: /api/bpm has no downbeatDetected -> FAIL.
#
# RIG FACTS (same as probe-onset-render.sh -- do not re-derive):
#   * ApiServer = http://127.0.0.1:7070 (IPv4 ONLY), PRODUCTION mode, NO --test-mode (test mode never
#     starts AnalysisThread; /api/bpm would read zeros forever -- probe-tempo-silence.sh's lesson).
#   * Analysis needs hops flowing: the default MicInput device must be open (TCC prompt on the first
#     launch after a rebuild). If beatPhase never moves, screencapture -x and LOOK.
#   * Launch via `open`; every pgrep uses the bracket trick 'MacOS/Audio-DN[A]'.
#   * SCREEN-SAFETY LAW: never open the Output window; graceful osascript quit first, pkill last.
set -u
OUT="${1:-/tmp/audiodna-downbeat-level}"; mkdir -p "$OUT"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${DOWNBEAT_BUILD_DIR:-build-lane}"
APPBUNDLE="$ROOT/$BUILD_DIR/AudioDNA_artefacts/Release/Audio-DNA.app"
VENV_PY="${DOWNBEAT_VENV_PY:-$ROOT/.venv/bin/python}"
A='http://127.0.0.1:7070'
BPM=120
POLL_S="${DOWNBEAT_POLL_S:-30}"
FAST_SLEEP=0.04     # ~25 Hz: the UI reader class (TopBar 15 Hz, AudioReadoutPanel 30 Hz)
SLOW_PERIOD=0.9     # slower than one beat (0.5 s at 120 BPM): where a LEVEL consumer does lose bars
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
info(){ echo "INFO  $1"; }
jget(){ # $1 = URL  $2 = python expr against `d`; "NA(...)" on any error
  curl -s --max-time 5 "$1" | python3 -c "
import json, sys
try:
    d = json.load(sys.stdin)
    print($2)
except Exception as e:
    print('NA(%s)' % e)
"
}
isint(){ echo "$1" | grep -Eq '^-?[0-9]+$'; }

# --- 1. preconditions + production launch ----------------------------------
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { echo "REFUSE: an Audio-DNA instance is already running. Quit it, then re-run."; exit 64; }
[ -d "$APPBUNDLE" ] || { echo "REFUSE: no built app at $APPBUNDLE (set DOWNBEAT_BUILD_DIR to override)"; exit 64; }
: > "$OUT/adna-out.log"; : > "$OUT/adna-err.log"      # open --stdout/--stderr APPEND: clear first
open --stdout "$OUT/adna-out.log" --stderr "$OUT/adna-err.log" "$APPBUNDLE"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] && break; sleep 1; done
[ -n "$(curl -s --max-time 2 "$A/api/health" 2>/dev/null)" ] || { echo "FAIL: health never came up on $A (TCC mic prompt? screencapture -x and LOOK)"; exit 1; }
PID="$(pgrep -f 'MacOS/Audio-DN[A]' | head -1)"
[ -n "$PID" ] || { echo "FAIL: health answered but no Audio-DNA process was found"; exit 1; }
ok "app launched (production), /api/health answered, PID=$PID"
sleep 10
B1="$(jget "$A/api/bpm" "d.get('totalBarCount','NA')")"
isint "$B1" && ok "analysis thread live: totalBarCount readable ($B1)" || no "totalBarCount not readable ($B1) -- analysis thread not running?"

# --- 2. the field under test (RED on a pre-fix build) -----------------------
HAS="$(jget "$A/api/bpm" "isinstance(d.get('downbeatDetected'), bool)")"
[ "$HAS" = "True" ] && ok "/api/bpm exposes downbeatDetected (bool)" \
  || no "/api/bpm has no bool downbeatDetected ($HAS) -- pre-fix build? the level cannot be observed"

# --- 3. manual tempo: predicted beats, no audio content needed ---------------
curl -s --max-time 6 -X POST "$A/api/set_bpm" -H 'Content-Type: application/json' -d "{\"bpm\":$BPM}" >/dev/null
sleep 1
B="$(jget "$A/api/bpm" "round(d['bpm'], 1)")"
[ "$B" = "$BPM.0" ] && ok "manual tempo took (bpm=$B)" || no "manual tempo did not take (bpm=$B)"

# --- 4. poll ~25 Hz for POLL_S seconds; verdicts computed in python ----------
if [ "$HAS" = "True" ]; then
POLL_S="$POLL_S" FAST_SLEEP="$FAST_SLEEP" SLOW_PERIOD="$SLOW_PERIOD" BPM="$BPM" OUT="$OUT" \
python3 - "$A" > "$OUT/verdicts.txt" <<'PY'
import json, os, statistics, sys, time, urllib.request
A = sys.argv[1] + '/api/bpm'
poll_s = float(os.environ['POLL_S']); fast_sleep = float(os.environ['FAST_SLEEP'])
slow_period = float(os.environ['SLOW_PERIOD']); bpm = int(os.environ['BPM']); out = os.environ['OUT']
def get():
    with urllib.request.urlopen(A, timeout=2) as r:
        return json.load(r)
polls = []
t_end = time.monotonic() + poll_s
while time.monotonic() < t_end:
    t = time.monotonic(); d = get()
    polls.append((t, bool(d['downbeatDetected']), int(d['beatInBar']), int(d['totalBarCount']), float(d['beatPhase'])))
    time.sleep(fast_sleep)
n = len(polls)
gaps = [polls[i][0] - polls[i-1][0] for i in range(1, n)]
mean_ms = 1000 * statistics.mean(gaps); max_ms = 1000 * max(gaps)
duty = sum(1 for p in polls if p[1]) / n
runs = []; start = None
for t, lvl, _, _, _ in polls:
    if lvl and start is None: start = t
    elif not lvl and start is not None: runs.append(t - start); start = None
run_median_ms = 1000 * statistics.median(runs) if runs else -1
edges_fast = sum(1 for i in range(1, n) if polls[i][1] and not polls[i-1][1])
delta_fast = polls[-1][3] - polls[0][3]
mismatch = sum(1 for p in polls if p[1] != (p[2] == 0))
phase_values = len(set(round(p[4], 2) for p in polls))
slow = []; nxt = polls[0][0]
for p in polls:
    if p[0] >= nxt:
        slow.append(p)
        while nxt <= p[0]: nxt += slow_period
edges_slow = sum(1 for i in range(1, len(slow)) if slow[i][1] and not slow[i-1][1])
delta_slow = slow[-1][3] - slow[0][3]
pulse_slow = sum(1 for i in range(1, len(slow)) if slow[i][3] > slow[i-1][3])
with open(os.path.join(out, 'poll.json'), 'w') as f:
    json.dump({'n': n, 'mean_ms': mean_ms, 'max_ms': max_ms, 'duty': duty, 'runs_ms': [1000 * r for r in runs],
               'edges_fast': edges_fast, 'delta_fast': delta_fast, 'mismatch': mismatch, 'phase_values': phase_values,
               'slow_n': len(slow), 'edges_slow': edges_slow, 'delta_slow': delta_slow, 'pulse_slow': pulse_slow}, f)
def v(cond, msg): print(('PASS  ' if cond else 'FAIL  ') + msg)
v(n >= 400 and max_ms < 250, f"poller healthy: {n} polls, mean {mean_ms:.1f} ms, max gap {max_ms:.1f} ms (need >= 400 polls, max gap < 250 ms)")
v(phase_values > 5, f"analysis hops flowing: beatPhase took {phase_values} distinct values")
v(12 <= delta_fast <= 17, f"bars advance at the manual tempo: totalBarCount +{delta_fast} in {poll_s:.0f} s (expect ~15 at {bpm} BPM)")
v(0.18 <= duty <= 0.32 and 350 <= run_median_ms <= 700, f"LEVEL: downbeatDetected duty {duty:.3f} (expect ~0.25), true-run median {run_median_ms:.0f} ms over {len(runs)} runs (expect ~500 ms) -- a one-hop pulse would read duty ~0.03 and runs <= 45 ms")
v(edges_fast == delta_fast, f"no loss at ~25 Hz: rising edges {edges_fast} == totalBarCount delta {delta_fast}")
v(mismatch == 0, f"coherent: downbeatDetected == (beatInBar == 0) on every poll ({mismatch} mismatches)")
v(pulse_slow == delta_slow and 0 <= delta_fast - delta_slow <= 1, f"counter is cadence-independent: the 0.9 s subsample ({len(slow)} reads) saw {pulse_slow} advancing reads, delta {delta_slow} (fast delta {delta_fast})")
v(edges_slow < delta_slow, f"level edge-detection at 0.9 s LOSES bars: {edges_slow} edges vs {delta_slow} bars (expect roughly half) -- the counter above did not")
PY
  cat "$OUT/verdicts.txt"
  PASS=$((PASS + $(grep -c '^PASS ' "$OUT/verdicts.txt"))); FAIL=$((FAIL + $(grep -c '^FAIL ' "$OUT/verdicts.txt")))
else
  no "poll skipped: downbeatDetected not exposed"
fi

# --- 5. teardown (SCREEN-SAFETY LAW) ------------------------------------------
osascript -e 'quit app "Audio-DNA"' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then
    echo "graceful osascript quit did not clear the process -- falling back to pkill (last resort)"
    pkill -f 'MacOS/Audio-DN[A]' >/dev/null 2>&1
    for _ in $(seq 1 20); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
fi
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && no "APP STILL RUNNING AFTER graceful quit + pkill fallback" || ok "app terminated, no process remains"
if [ -x "$VENV_PY" ]; then
    W="$("$VENV_PY" -c "
import Quartz
wl=Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))
           and 'Output' in str(w.get('kCGWindowName',''))]))" 2>/dev/null)"
    if [ "$W" = "0" ] || [ -z "$W" ]; then ok "0 Audio-DNA Output windows in the FULL window list (SCREEN-SAFETY LAW held)"
    else no "$W Audio-DNA Output window(s) were open during the run -- screen-safety breach"; fi
else
    no "Output-window check: $VENV_PY (pyobjc/Quartz) unavailable -- set DOWNBEAT_VENV_PY to the main checkout's .venv/bin/python"
fi
screencapture -x "$OUT/downbeat-level-eos.png" 2>/dev/null \
  && echo "screenshot: $OUT/downbeat-level-eos.png (read it before concluding -- no dialog expected)" \
  || echo "screencapture failed (no display attached / headless run)"
echo; echo "$PASS PASS / $FAIL FAIL   (artifacts in $OUT)"
[ "$FAIL" -eq 0 ] || exit 1
