#!/bin/bash
# probe-video.sh -- s-rta-0928b video (.harmony/.reports/s-rta-0928b/plan-video.md section 4.6 + HARMONY ADOPTION).
# Live witness that video decodes OFF the GL render thread: steady 4 x 1080p / 4 x 4K / ProRes playback at >= 110 fps
# with no avcodec call on the render thread and one upload per content frame; a mid-GOP retrigger / deck return /
# retrigger mid-fade holds the last shown frame (never black, never keyframe + 29) instead of freezing the whole output;
# decode threads park off screen; the message thread never waits on a decode; frame accuracy survives the ring.
# Rows, fixtures and calibration: the docstring of .harmony/probe-video.py (REST on 7070, every PNG decoded with
# PIL+numpy); thresholds: .harmony/probe-video.json. Fixtures (ffmpeg, nice'd, -threads 2) are encoded per run into
# <out>/media (deleted after the quit), or reused from $VIDEO_FIXTURES when that dir holds them. ADDITIONALLY REFUSES
# when ffmpeg / ffprobe are not on PATH (probe-deck-clock.sh precedent).
#
# Clone of .harmony/probe-render-state.sh (refuse / fresh out dir / foreign-traffic check / graceful quit).
# Production mode (no --test-mode). Screen-safe: open -g (never plain open / foreground exec), no screen capture,
# no Output window, no synthetic input; graceful quit, pkill only if still running after 30 s. REFUSES if Audio-DNA
# is already running. The caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# usage: probe-video.sh [out-base] [row,row,...]
#   VIDEO_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   VIDEO_PY   python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   VIDEO_ENV  optional VAR=value passed to the app via open --env
#   VIDEO_FIXTURES  optional dir: encode the fixtures once there and reuse them (keyframes re-asserted each run)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/video.XXXXXX" (out-base default /tmp).
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
APP="${VIDEO_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${VIDEO_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set VIDEO_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set VIDEO_PY)"; exit 64; }
command -v ffmpeg >/dev/null 2>&1 && command -v ffprobe >/dev/null 2>&1 || { echo "REFUSE: ffmpeg / ffprobe not on PATH (the fixtures are encoded per run)"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener: $(lsof -nP -iTCP:7070 -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/video.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
# The fixtures are encoded BEFORE the app starts (the 4K clip is the long pole, ~1 min): no encode competes with a row.
"$PY" "$ROOT/.harmony/probe-video.py" "$ROOT" "$OUT" --make-fixtures "${2:-}" || { echo "FAIL  fixtures"; exit 1; }
ENVARGS=(); [ -n "${VIDEO_ENV:-}" ] && ENVARGS=(--env "$VIDEO_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-video.py" "$ROOT" "$OUT" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# The run's own snapshots (i3n: the app's Snapshots dir, listed by the .py in own-captures.txt) are not foreign.
OWN="$OUT/own-captures.txt"; [ -f "$OWN" ] || : > "$OWN"
FOREIGN="$(grep -o 'Captured frame: [^ ]*' "$OUT/err.log" 2>/dev/null | grep -v "Captured frame: $OUT/" | sed 's/^Captured frame: //' | grep -cvxFf "$OWN")"
if [ "${FOREIGN:-0}" -gt 0 ]; then
  echo "FAIL  foreign REST traffic: $FOREIGN render_frame capture(s) outside $OUT during this run -- results INVALID"
  grep -o 'Captured frame: [^ ]*' "$OUT/err.log" | grep -v "Captured frame: $OUT/" | head -3 | sed 's/^/      /'
  RC=1
else echo "PASS  no foreign render_frame traffic during the run"; fi
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do adna_running || break; sleep 1; done
adna_running && { adna_kill; sleep 2; }
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
rm -rf "$OUT/media"
echo; [ "$RC" -eq 0 ] && echo "PROBE-VIDEO GREEN" || echo "PROBE-VIDEO RED"
exit "$RC"
