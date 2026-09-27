#!/bin/bash
# probe-fitmode.sh -- s-rta-0926b plan-fitmode. Live witness for the per-clip FIT MODE (Stretch / Bars / Crop):
# a clip whose picture is not the composition's shape. Rows f_* -- Stretch byte-identical to today (guards),
# Bars = own shape centred with TRANSPARENT bars (the lower layer shows), Crop = cover + cut, fit BEFORE the clip
# transform, a layer transform never fits twice, Sources ignore it, REST /api/set_clip_param + OSC
# /audiodna/clip/{l}/{c}/fit flip it live, perf report.
# Cases, fixtures: .harmony/probe-fitmode.json; assertions + calibration: the docstring of
# .harmony/probe-fitmode.py (REST calls on 7070, OSC on UDP 8000, every PNG decoded with PIL+numpy).
#
# Clone of .harmony/probe-render-state.sh (live-lock gate, adna_* ucomm filters, open -g, 7070 listener check,
# foreign-traffic check, graceful quit). Screen-safe: open -g (never plain open / foreground exec), no screen
# capture, no Output window, no synthetic input; graceful quit, pkill only if still running after 30 s.
# REFUSES if Audio-DNA is already running. The caller holds /tmp/audiodna-live.lock.
#
# usage: probe-fitmode.sh [out-base] [row,row,...]
#   FIT_APP    app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   FIT_PY     python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   FIT_SHOTS  optional dir: the .py also writes the composition frames (960x540) + PIL oracles there
# Every run captures into a FRESH dir: mktemp -d "<out-base>/fit.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists;
# if AUDIODNA_LOCK_OWNER is set, it must match the owner file's first field. See probe-render-state.sh for the
# ucomm rationale of adna_pids.
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
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${FIT_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${FIT_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set FIT_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set FIT_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
# s-rta-0926b render2: another process listening on 7070 (seen: a lane's stub_server.py) would either take the
# app's REST port or answer this probe itself -- refuse rather than measure the wrong process.
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener: $(lsof -nP -iTCP:7070 -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
# the OSC rows send to UDP 8000: another listener there would receive them instead of the app.
lsof -nP -iUDP:8000 >/dev/null 2>&1 && { echo "REFUSE: UDP 8000 already bound: $(lsof -nP -iUDP:8000 | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/fit.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-fitmode.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# s-rta-0926b render2: every render_frame this run asked for lands in $OUT. A capture anywhere else means another
# process drove this app over REST during the run (seen: a lane harness whose stub server could not bind 7070) --
# its effect-chain / composition writes invalidate every row, so the run is RED.
FOREIGN="$(grep -o 'Captured frame: [^ ]*' "$OUT/err.log" 2>/dev/null | grep -vc "Captured frame: $OUT/")"
if [ "${FOREIGN:-0}" -gt 0 ]; then
  echo "FAIL  foreign REST traffic: $FOREIGN render_frame capture(s) outside $OUT during this run -- results INVALID"
  grep -o 'Captured frame: [^ ]*' "$OUT/err.log" | grep -v "Captured frame: $OUT/" | head -3 | sed 's/^/      /'
  RC=1
else echo "PASS  no foreign render_frame traffic during the run"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
adna_running && { adna_kill; sleep 2; }
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-FITMODE GREEN" || echo "PROBE-FITMODE RED"
exit "$RC"
