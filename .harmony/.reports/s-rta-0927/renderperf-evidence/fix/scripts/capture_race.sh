#!/bin/bash
# capture_race.sh <app> <out-base>  -- launches <app> (test mode, open -g, the TEMPORARY collect-delay hook armed via
# --env) and runs capture_race.py. Refuses without the live lock (owner must match AUDIODNA_LOCK_OWNER), if an
# Audio-DNA is already running, or if 7070/8080 already have a listener. Graceful quit; pkill only after 30 s.
set -u
D="$(cd "$(dirname "$0")" && pwd)"
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock"; exit 64; }
[ "$(cut -d' ' -f1 $LOCK_OWNER_FILE)" = "${AUDIODNA_LOCK_OWNER:-}" ] || { echo "REFUSE: lock owner mismatch"; exit 64; }
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
APP=$1; BASE=$2
[ -d "$APP" ] || { echo "REFUSE: no app $APP"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
for PORT in 7070 8080; do lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT busy"; exit 64; }; done
mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/race.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" --env AUDIODNA_TEST_CAPTURE_COLLECT_DELAY_MS=1500 "$APP" --args --test-mode
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 http://127.0.0.1:7070/api/health)" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070'"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" -B "$D/capture_race.py" "$OUT"; RC=$?; else echo "FAIL  app never answered /api/health"; fi
if grep -q "TESTHOOK collect delay 1500 ms" "$OUT/err.log"; then echo "PASS  h_hook: the collect-delay hook was armed ($(grep -c 'TESTHOOK collect delay' "$OUT/err.log") captures delayed)"
else echo "FAIL  h_hook: the collect-delay hook never fired -- this run proves nothing"; RC=1; fi
FOREIGN="$(grep -o 'Captured frame: [^ ]*' "$OUT/err.log" 2>/dev/null | grep -vc "Captured frame: $OUT/")"
if [ "${FOREIGN:-0}" -gt 0 ]; then echo "FAIL  foreign render_frame traffic: $FOREIGN"; RC=1; else echo "PASS  no foreign render_frame traffic"; fi
echo "--- [Eyes] lines:"; grep -E "\[Eyes\] (Captured frame|Invalid|Frame capture|Failed|TESTHOOK)" "$OUT/err.log" | sed "s#$OUT/##"
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
adna_running && { kill $(adna_pids) 2>/dev/null; sleep 2; }
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-CAPTURE-RACE GREEN" || echo "PROBE-CAPTURE-RACE RED"
exit "$RC"
