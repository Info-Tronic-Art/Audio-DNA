#!/bin/bash
# probe-async-load.sh -- s-rta-0929 asyncload (.harmony/.reports/s-rta-0929/plan-asyncload.md 5.9 + HARMONY ADOPTION
# AL1-AL12). Live witness that a composition load, a deck append (Load Deck) and a deck duplicate open their videos OFF
# the message thread (TEST-ONLY heartbeat: peak_message_stall_ms), that the old show keeps playing until the cut, that
# the cut shows no black / no pending frame, that POST /api/load_composition still answers only once the new composition
# is on screen, that a newer load / a cancel retires the staged one (players, label, answer), that a trigger during the
# window acts on the live show, and that a quit with a load in flight -- even one whose open(2) hangs -- terminates
# cleanly. Rows, fixtures and calibration: the docstring of .harmony/probe-async-load.py; thresholds:
# .harmony/probe-async-load.json. Fixtures (ffmpeg videos + APFS clones, PIL PNGs, 600 sequence frames) are generated per
# run into <out>/media BEFORE the launch and deleted after the quit.
#
# probe-media-open.sh's scaffolding verbatim (ASYNCLOAD_* instead of MEDIAOPEN_*), plus: after phase 1 (every row, then
# end_quit_mid_load) the quit must terminate the app within 30 s WITHOUT a kill, with no new
# ~/Library/Logs/DiagnosticReports/Audio-DNA*.ips and 0 on-screen UserNotificationCenter windows; phase 2 (AL2) relaunches
# the app for end_quit_hung_open (a FIFO video cell: a pool thread hangs inside open(2)) and checks the same quit.
# Production mode (no --test-mode) on a TEST_SERVER build. Screen-safe: open -g, no screen capture, no Output window, no
# synthetic input; graceful quit, kill only if still running after 30 s (= FAIL). REFUSES if Audio-DNA is running. The
# caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# usage: probe-async-load.sh [out-base] [row,row,...]
#   ASYNCLOAD_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   ASYNCLOAD_PY   python with PIL+numpy+requests (+pyobjc Quartz) (default: <root>/.venv, else the main checkout's .venv)
#   ASYNCLOAD_ENV  optional VAR=value passed to the app via open --env
# Rows: empty = every phase-1 row + end_quit_hung_open (phase 2). A row list runs those rows in phase 1 (a row may repeat)
# and phase 2 only if it names end_quit_hung_open.
# PROBE RIG GATE (probehygiene2): refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if
# AUDIODNA_LOCK_OWNER is set, it must match the owner file's first field.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
adna_kill() { local p; p="$(adna_pids)"; [ -n "$p" ] && kill $p 2>/dev/null; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${ASYNCLOAD_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${ASYNCLOAD_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set ASYNCLOAD_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set ASYNCLOAD_PY)"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/asyncload.XXXXXX")" || exit 64
ROWS="${2:-}"
echo "app: $APP"; echo "out: $OUT"
touch "$OUT/start.stamp"
"$PY" "$ROOT/.harmony/probe-async-load.py" "$ROOT" "$OUT" --make-fixtures "$ROWS" || { echo "FAIL  fixtures"; exit 1; }
ENVARGS=(); [ -n "${ASYNCLOAD_ENV:-}" ] && ENVARGS=(--env "$ASYNCLOAD_ENV")
RC=0
unc_count() {
  "$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))" 2>/dev/null || echo "?"
}
launch() {  # launch <phase>
  open -g --stdout "$OUT/out-$1.log" --stderr "$OUT/err-$1.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP"
  local up=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { up=1; break; }; sleep 1; done
  sleep 2
  local l7070; l7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
  if [ "$up" -eq 1 ] && [ "$l7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$l7070', not Audio-DNA"; up=0; fi
  [ "$up" -eq 1 ]
}
quit_check() {  # quit_check <phase>: the graceful quit must end the app within 30 s, no .ips, no dialog
  local t0; t0=$(date +%s)
  # in the background: a hung app never answers the Apple event and osascript would block ~120 s before the 30 s clock
  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1 &
  local osa=$!
  for _ in $(seq 1 30); do adna_running || break; sleep 1; done
  kill "$osa" 2>/dev/null; wait "$osa" 2>/dev/null
  local dt=$(( $(date +%s) - t0 ))
  if adna_running; then echo "FAIL  $1: the app is still running $dt s after the quit (killed)"; adna_kill; sleep 3; RC=1
  else echo "PASS  $1: app terminated $dt s after the quit"; fi
  sleep 2
  local ips; ips="$(find "$HOME/Library/Logs/DiagnosticReports" -name 'Audio-DNA*' -newer "$OUT/start.stamp" 2>/dev/null | head -3)"
  if [ -n "$ips" ]; then echo "FAIL  $1: new crash report(s): $ips"; RC=1; else echo "PASS  $1: no new Audio-DNA crash report"; fi
  local u; u="$(unc_count)"
  if [ "$u" = "0" ]; then echo "PASS  $1: 0 UserNotificationCenter windows on screen"; else echo "FAIL  $1: UserNotificationCenter windows on screen: $u"; RC=1; fi
}
foreign_check() {  # foreign_check <phase>
  local own="$OUT/own-captures.txt"; [ -f "$own" ] || : > "$own"
  local n; n="$(grep -o 'Captured frame: [^ ]*' "$OUT/err-$1.log" 2>/dev/null | grep -v "Captured frame: $OUT/" | sed 's/^Captured frame: //' | grep -cvxFf "$own")"
  if [ "${n:-0}" -gt 0 ]; then echo "FAIL  $1: foreign REST traffic: $n render_frame capture(s) outside $OUT -- results INVALID"; RC=1
  else echo "PASS  $1: no foreign render_frame traffic during the run"; fi
}
PHASE1="$ROWS"; PHASE2=0
if [ -z "$ROWS" ]; then PHASE2=1
elif echo ",$ROWS," | grep -q ',end_quit_hung_open,'; then PHASE2=1; PHASE1="$(echo "$ROWS" | tr ',' '\n' | grep -vx end_quit_hung_open | paste -sd, -)"; fi
if [ -z "$ROWS" ] || [ -n "$PHASE1" ]; then
  echo "=== phase 1 ($(date +%T))"
  if launch 1; then "$PY" "$ROOT/.harmony/probe-async-load.py" "$ROOT" "$OUT" "$PHASE1" || RC=1
  else echo "FAIL  app never answered /api/health"; RC=1; fi
  foreign_check 1
  quit_check phase1
fi
if [ "$PHASE2" -eq 1 ]; then
  echo "=== phase 2: end_quit_hung_open ($(date +%T))"
  if launch 2; then "$PY" "$ROOT/.harmony/probe-async-load.py" "$ROOT" "$OUT" end_quit_hung_open || RC=1
  else echo "FAIL  app never answered /api/health (phase 2)"; RC=1; fi
  sleep 0.5
  quit_check phase2_hung_open
fi
rm -f "$OUT/media/hung.mp4"
rm -rf "$OUT/media"
echo; [ "$RC" -eq 0 ] && echo "PROBE-ASYNC-LOAD GREEN" || echo "PROBE-ASYNC-LOAD RED"
exit "$RC"
