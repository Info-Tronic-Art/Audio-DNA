#!/bin/bash
echo "ARCHIVED RECORD (R-N1, s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" >&2; exit 64
# usage: live.sh <label> <app-bundle> <script.py> [script args...]
# Acquire /tmp/audiodna-live.lock as source-defects (poll 20 s, <= 120 min; >= 45 s after our own last release),
# launch <app> in test mode (open -g, background), run the script, quit (osascript; kill only after 30 s), release.
SD=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/srcdef
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
L=/tmp/audiodna-live.lock
LABEL=$1; APP=$2; SCRIPT=$3; shift 3
OUT=$SD/runs/$LABEL; mkdir -p "$OUT"
export AUDIODNA_LOCK_OWNER=source-defects
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
if [ -f $SD/.last_release ]; then gap=$(( $(date +%s) - $(cat $SD/.last_release) )); [ $gap -lt 45 ] && { echo "gap: sleeping $((45-gap)) s"; sleep $((45-gap)); }; fi
T0=$(date +%s)
while ! mkdir "$L" 2>/dev/null; do
  if [ $(( $(date +%s) - T0 )) -gt 7200 ]; then echo "LOCK: gave up after 120 min (owner: $(cat $L/owner 2>/dev/null))"; exit 75; fi
  echo "LOCK: held by '$(cat $L/owner 2>/dev/null)' -- waiting ($(date '+%H:%M:%S'))"; sleep 20
done
echo "source-defects $$ $(date +%s)" > "$L/owner"
release() { [ "$(cut -d' ' -f1 $L/owner 2>/dev/null)" = "source-defects" ] && rm -rf "$L" && date +%s > $SD/.last_release && echo "LOCK: released $(date '+%H:%M:%S')"; }
trap release EXIT
echo "LOCK: acquired $(date '+%H:%M:%S')"
if [ -n "$(adna_pids)" ]; then echo "REFUSE: an Audio-DNA is already running: $(adna_pids)"; exit 64; fi
for PORT in 7070 8080; do lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT busy"; exit 64; }; done
echo "launch: $APP ($(date '+%H:%M:%S'))"
if [ -n "$LIVE_ENV" ]; then echo "env: $LIVE_ENV"; open -g --env "$LIVE_ENV" --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" "$APP" --args --test-mode; else
  open -g --stdout "$OUT/app-out.log" --stderr "$OUT/app-err.log" "$APP" --args --test-mode; fi
up=0; for _ in $(seq 1 60); do curl -s --max-time 1 -H 'Connection: close' 'http://localhost:8080/api/health' >/dev/null && { up=1; break; }; sleep 1; done
if [ $up = 1 ]; then
  sleep 2; echo "app up: pids $(adna_pids) ($(date '+%H:%M:%S'))"
  PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$SD $PY "$SCRIPT" "$OUT" "$@" 2>&1; echo "--- script rc=$? ($(date '+%H:%M:%S'))"
else
  echo "FAIL: /api/health never answered in 60 s -- STOP (possible system dialog)"
fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do [ -z "$(adna_pids)" ] && break; sleep 1; done
[ -n "$(adna_pids)" ] && { echo "kill after 30 s: $(adna_pids)"; pkill -x Audio-DNA; sleep 2; }
echo "after quit: running=$([ -n "$(adna_pids)" ] && echo yes || echo no) ($(date '+%H:%M:%S'))"
