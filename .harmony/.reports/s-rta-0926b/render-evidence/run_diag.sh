#!/bin/bash
# Diagnosis runner (lane render, s-rta-0926b). Screen-safe: open -g only, graceful quit.
# usage: run_diag.sh <app> <outbase> <cases> [ENV=VAL]
set -u
LANE=render
APP="$1"; BASE="$2"; CASES="$3"; ENVV="${4:-}"
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-1
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
HERE="$(cd "$(dirname "$0")" && pwd)"
A=http://127.0.0.1:7070
# --- lock ---
waited=0
until mkdir /tmp/audiodna-live.lock 2>/dev/null; do
  [ "$waited" -ge 2700 ] && { echo "LOCK TIMEOUT"; exit 70; }
  echo "lock held by: $(cat /tmp/audiodna-live.lock/owner 2>/dev/null)"; sleep 20; waited=$((waited+20))
done
echo "$LANE $$ $(date +%s)" > /tmp/audiodna-live.lock/owner
release() { [ "$(cut -d' ' -f1 /tmp/audiodna-live.lock/owner 2>/dev/null)" = "$LANE" ] && rm -rf /tmp/audiodna-live.lock; }
if pgrep -f 'MacOS/Audio-DN[A]' >/dev/null; then echo "REFUSE: Audio-DNA already running"; release; exit 64; fi
mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/diag.XXXXXX")"
echo "app: $APP"; echo "out: $OUT"; echo "env: $ENVV"
if [ -n "$ENVV" ]; then open -g --env "$ENVV" --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
else open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"; fi
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
if [ "$UP" -eq 1 ]; then "$PY" "$HERE/diag.py" "$WT/media" "$OUT" "$CASES"; else echo "FAIL app never answered"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do pgrep -f 'MacOS/Audio-DN[A]' >/dev/null || break; sleep 1; done
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && { pkill -f 'MacOS/Audio-DN[A]'; sleep 2; }
pgrep -f 'MacOS/Audio-DN[A]' >/dev/null && echo "WARN app still running" || echo "app terminated"
release
echo "lock released: $([ -d /tmp/audiodna-live.lock ] && echo NO || echo yes)"
echo "OUT=$OUT"
