#!/bin/bash
# test-wait-for-replay-finish.sh -- headless unit test of wait_for_replay_finish(), added by
# commit ed6c669 (s-rta-0925 replayend-probe fix) to .harmony/probe-step3.sh.
#
# WHY THIS EXISTS: review-replayend-probe-r1.md's blocking finding -- no log postdating ed6c669
# exercises wait_for_replay_finish's own echo/polling logic (both cited logs predate the fix and
# were produced by the OLD probe script against an even earlier build). This proves the function's
# OWN mechanism -- diagnostic echo firing with the right numbers, early-exit-on-finished (not just
# timeout), the no-op path, and the fixed-fallback branch -- against a synthetic local HTTP
# responder standing in for /api/perf/status.
#
# This is a REAL child process (python3 http.server) and a REAL curl round-trip over 127.0.0.1,
# not a shell-function stub -- the class of teeth a timing/polling mechanism needs (Coding Gotcha
# #3 / test-authoring craft item #3, adapted from process-kill to HTTP-poll).
#
# WHAT THIS DOES NOT PROVE: it does not launch Audio-DNA.app and does not prove RecorderHost's real
# end-of-replay behavior against a live rebuilt binary -- this session's rig rules forbid launching
# the app, and per every probe-*.sh header in this directory ("the party that builds never
# verifies"), that live run is Harmony's gate to run, not the Builder's. See
# .harmony/.reports/s-rta-0925/replayend-probe-diagnosis.md for the disposition of that outstanding
# step.
#
# Extracts the function bodies under test from .harmony/probe-step3.sh BY LINE RANGE (never
# retyped), so this test tracks the live source rather than a hand-copied paraphrase. If the probe
# script is edited and these line numbers drift, the sanity check below fails loudly instead of
# silently testing stale code.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROBE="$ROOT/.harmony/probe-step3.sh"
PORT=7070
STATE_FILE="$(mktemp -t rwrf-state)"
SERVER_LOG="$(mktemp -t rwrf-server-log)"
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
no(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }

[ -f "$PROBE" ] || { echo "FAIL: $PROBE not found"; exit 1; }

A="http://127.0.0.1:$PORT"
eval "$(sed -n '162,169p' "$PROBE")"   # perf_field()
eval "$(sed -n '161p' "$PROBE")"       # perf_status()  (curls $A)
eval "$(sed -n '684p' "$PROBE")"       # is_true()
eval "$(sed -n '701,721p' "$PROBE")"   # wait_for_replay_finish()

declare -F wait_for_replay_finish >/dev/null || { echo "FAIL: could not extract wait_for_replay_finish from $PROBE (line numbers 701-721 stale? re-grep and update this script)"; exit 1; }
declare -F perf_field >/dev/null || { echo "FAIL: could not extract perf_field from $PROBE (line numbers 162-169 stale?)"; exit 1; }
declare -F is_true >/dev/null || { echo "FAIL: could not extract is_true from $PROBE (line 684 stale?)"; exit 1; }
ok "extracted wait_for_replay_finish/perf_field/perf_status/is_true verbatim from $PROBE by line range"

echo '{"finished": false, "lengthSeconds": 3}' > "$STATE_FILE"
python3 - "$PORT" "$STATE_FILE" >"$SERVER_LOG" 2>&1 <<'PYEOF' &
import http.server, sys
port = int(sys.argv[1]); state_file = sys.argv[2]
class H(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        with open(state_file) as f:
            body = f.read().encode()
        self.send_response(200); self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(body))); self.end_headers()
        self.wfile.write(body)
    def log_message(self, *a): pass
http.server.HTTPServer(('127.0.0.1', port), H).serve_forever()
PYEOF
SERVER_PID=$!
cleanup(){ kill "$SERVER_PID" 2>/dev/null; wait "$SERVER_PID" 2>/dev/null; rm -f "$STATE_FILE" "$SERVER_LOG"; }
trap cleanup EXIT

UP=0
for _ in $(seq 1 20); do
    curl -s --max-time 1 "$A/api/perf/status" >/dev/null 2>&1 && { UP=1; break; }
    sleep 0.1
done
[ "$UP" = 1 ] || { echo "FAIL: synthetic responder never came up on $A"; cat "$SERVER_LOG"; exit 1; }
ok "synthetic /api/perf/status responder is up (real child process pid=$SERVER_PID, real TCP on $A)"

# --- Test 1: early-exit-on-finished. Flips finished=true after ~1s; the function must return
# BEFORE its own lengthSeconds+margin bound (3+5=8s), proving the poll loop, not just the timeout.
START_T=$(date +%s)
( sleep 1; echo '{"finished": true, "lengthSeconds": 3}' > "$STATE_FILE" ) &
FLIP_PID=$!
T0=$(date +%s)
OUT="$(wait_for_replay_finish "unit-test" 2>&1)"
T1=$(date +%s)
wait "$FLIP_PID" 2>/dev/null
ELAPSED=$(( T1 - T0 ))
echo "--- test 1 output ---"
echo "$OUT"
echo "---------------------"
echo "$OUT" | grep -q "wait_for_replay_finish(unit-test): lengthSeconds=3" && ok "diagnostic echo fires with the correct lengthSeconds -- this is the exact line the reviewer found absent from both pre-fix logs" || no "diagnostic echo missing or wrong: $OUT"
[ "$ELAPSED" -le 6 ] && ok "returned in ${ELAPSED}s, well before the 8s (lengthSeconds+margin) bound -- proves the poll loop exits on finished=true, not on timeout" || no "took ${ELAPSED}s -- did not exit early on finished=true"

# --- Test 2: immediate no-op when finished is already true at entry.
echo '{"finished": true, "lengthSeconds": 3}' > "$STATE_FILE"
START_T=$(date +%s)
T0=$(date +%s)
NOOP_OUT="$(wait_for_replay_finish "unit-test-noop" 2>&1)"
T1=$(date +%s)
ELAPSED=$(( T1 - T0 ))
[ "$ELAPSED" -le 1 ] && [ -z "$NOOP_OUT" ] && ok "no-op immediate return when finished is already true at entry (no echo)" || no "expected an immediate silent return, got ${ELAPSED}s / output: $NOOP_OUT"

# --- Test 3: fixed-10s-fallback branch fires when lengthSeconds is unavailable.
echo '{"finished": false, "lengthSeconds": "NA"}' > "$STATE_FILE"
START_T=$(date +%s)
wait_for_replay_finish "unit-test-fallback" > /tmp/rwrf-fallback.out 2>&1 &
FUNC_PID=$!
sleep 1
kill "$FUNC_PID" 2>/dev/null
wait "$FUNC_PID" 2>/dev/null
FALLBACK_OUT="$(cat /tmp/rwrf-fallback.out)"; rm -f /tmp/rwrf-fallback.out
echo "$FALLBACK_OUT" | grep -q "lengthSeconds unavailable -- bounding at a fixed 10s fallback" && ok "fixed-10s-fallback branch fires when lengthSeconds is NA" || no "fallback branch did not fire: $FALLBACK_OUT"

echo
echo "== $PASS PASS, $FAIL FAIL =="
[ "$FAIL" -eq 0 ]
