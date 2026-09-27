#!/bin/bash
# probe-canvas.sh -- s-rta-0926b plan4 item 1 (.harmony/.reports/s-rta-0926b/plan4-final.md sections 2.6 / 2.8).
# Live witness that the COMPOSITION CANVAS drives the picture's shape: every render_frame PNG is exactly
# Composition::outputWidth x outputHeight (1920x1080 when a JSON has no keys, 4K, 4:3), a runtime resolution
# change keeps the Freeze history, the legacy single image is fitted inside the canvas (never stretched),
# captures stay deterministic; plus the perf gate (c_perf_1080) and the capture-cost / 4K reports.
# Cases, fixtures: .harmony/probe-canvas.json; assertions + calibration: the docstring of .harmony/probe-canvas.py
# (REST calls on 7070 and the test-mode TestServer on [::1]:8080, every PNG decoded with PIL+numpy).
#
# Clone of .harmony/probe-render-state.sh (refuse / fresh out dir / foreign-traffic check / graceful quit),
# with ONE difference: the app is launched in TEST MODE (`open -g ... --args --test-mode`) -- the
# c_runtime_change_keeps_history row changes the resolution through 8080 /api/set_composition_params
# (test mode only). Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output
# window, no synthetic input; graceful quit, pkill only if still running after 30 s. REFUSES if Audio-DNA is
# already running. The caller holds /tmp/audiodna-live.lock (see PROBE RIG GATE below).
#
# usage: probe-canvas.sh [out-base] [row,row,...]
#   CANVAS_APP       app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   CANVAS_PY        python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   CANVAS_ENV       optional VAR=value passed to the app via open --env
#   CANVAS_BASE_FPS  c_perf_1080's base-binary fps (the gate falls back to fps >= base - 2 only when the base
#                    binary itself could not reach the 58 fps bar on this rig -- plan4 2.8)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/canvas.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE (probehygiene2): refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if
# AUDIODNA_LOCK_OWNER is set, it must match the owner file's first field. Process matching by the kernel's
# ucomm (adna_pids), never pgrep -f (see probe-render-state.sh's header for why).
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
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${CANVAS_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${CANVAS_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set CANVAS_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set CANVAS_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener: $(lsof -nP -iTCP:$PORT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/canvas.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${CANVAS_ENV:-}" ] && ENVARGS=(--env "$CANVAS_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP" --args --test-mode
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-canvas.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# Every render_frame this run asked for lands in $OUT. A capture anywhere else means another process drove
# this app over REST during the run -- its writes invalidate every row, so the run is RED.
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
echo; [ "$RC" -eq 0 ] && echo "PROBE-CANVAS GREEN" || echo "PROBE-CANVAS RED"
exit "$RC"
