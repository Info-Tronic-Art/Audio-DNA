#!/bin/bash
# probe-deck-clock.sh -- s-rta-0926b plan4 item 2 (.harmony/.reports/s-rta-0926b/plan4-final.md section 3.6).
# Live witness that a deck which is NOT on screen keeps time (Boris 2026-09-26: "finish the fade. when we load a
# new deck that does not touch the clips playing in the layer"; Q1 answered "keep playing"):
#   B1  a clip-to-clip crossfade started on a deck finishes while another deck is shown; a layer's fade is advanced
#       exactly once per frame (d_single_advance; its file still says "persistent": true, which bf9 Stage P
#       ignores); a quantized trigger left waiting on the deck is still cancelled (L5)
#   B2  video / image-sequence clocks advance while hidden (no decode), autopilot keeps advancing clips to the beat,
#       and the return frame's catch-up decode stays bounded (d_return_hitch)
# Cases, fixtures: .harmony/probe-deck-clock.json; assertions + calibration: the docstring of
# .harmony/probe-deck-clock.py (REST calls on 7070; every PNG decoded with PIL+numpy). Video fixtures are generated
# with ffmpeg into the fresh out dir at run start.
#
# Clone of .harmony/probe-canvas.sh (itself a clone of probe-render-state.sh): TEST MODE launch
# (`open -g ... --args --test-mode`) because the autopilot / pending-trigger rows drive beat crossings through
# 7070 /api/inject_features (registered only in test mode). ADDITIONALLY REFUSES when ffmpeg is not on PATH.
# Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output window, no synthetic
# input. QUITS ONLY THE APP IT LAUNCHED (quit_ours, .harmony/probe-quit-ours.sh; bf9 Stage P): graceful quit, kill
# of that one pid only if still running after 30 s; any other Audio-DNA is never touched. REFUSES if Audio-DNA is
# already running. The caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# usage: probe-deck-clock.sh [out-base] [row,row,...]
#   DCLOCK_APP   app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   DCLOCK_PY    python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   DCLOCK_ENV   optional VAR=value passed to the app via open --env
# Every run captures into a FRESH dir: mktemp -d "<out-base>/dclock.XXXXXX" (out-base default /tmp).
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
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # record_ourpid / ours_running / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${DCLOCK_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${DCLOCK_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set DCLOCK_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set DCLOCK_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
command -v ffmpeg >/dev/null 2>&1 || { echo "REFUSE: ffmpeg not on PATH (the video fixtures are generated with it)"; exit 64; }
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener: $(lsof -nP -iTCP:$PORT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/dclock.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${DCLOCK_ENV:-}" ] && ENVARGS=(--env "$DCLOCK_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP" --args --test-mode
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
record_ourpid; echo "ours: pid ${OURPID:-none}"
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-deck-clock.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# Every render_frame this run asked for lands in $OUT. A capture anywhere else means another process drove
# this app over REST during the run -- its writes invalidate every row, so the run is RED.
FOREIGN="$(grep -o 'Captured frame: [^ ]*' "$OUT/err.log" 2>/dev/null | grep -vc "Captured frame: $OUT/")"
if [ "${FOREIGN:-0}" -gt 0 ]; then
  echo "FAIL  foreign REST traffic: $FOREIGN render_frame capture(s) outside $OUT during this run -- results INVALID"
  grep -o 'Captured frame: [^ ]*' "$OUT/err.log" | grep -v "Captured frame: $OUT/" | head -3 | sed 's/^/      /'
  RC=1
else echo "PASS  no foreign render_frame traffic during the run"; fi
quit_ours || RC=1
if ours_running; then echo "FAIL  app still running (pid $OURPID)"; RC=1; else echo "PASS  app terminated"; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-DECK-CLOCK GREEN" || echo "PROBE-DECK-CLOCK RED"
exit "$RC"
