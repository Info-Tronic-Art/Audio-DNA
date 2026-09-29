#!/bin/bash
# probe-vupload.sh -- s-rta-0929 vupload (.harmony/.reports/s-rta-0929/plan-vupload.md 4.7 + HARMONY ADOPTION VU5-VU11).
# Live witness of the video upload path's TEST-MODE rows: the GL render thread runs at QoS USER_INTERACTIVE (u2, re-checked
# after a context cycle), a GL context loss never leaves a playing clip without its picture (u4a: video_hold_no_texture
# stays 0 across TEST-ONLY /api/debug/gl_context_cycle detach + re-attach cycles), an idle player's free ring slots are
# purged and come back with a correct picture (u4b), a crossfade under a column trigger keeps both chains' pictures (u6),
# and reverse / ping-pong play (u7, compared across apps by .harmony/probe-vupload-ab.sh). Rows, fixtures and calibration:
# the docstring of .harmony/probe-vupload.py; thresholds: .harmony/probe-vupload.json.
#
# Clone of .harmony/probe-video.sh (refuse / lock gate / fresh out dir / foreign-traffic check / graceful quit), launched
# in TEST MODE (--args --test-mode: the TestServer on 8080 AND the ApiServer on 7070). Screen-safe: open -g, no screen
# capture, no Output window (no row opens one; gl_context_cycle only re-attaches the preview's context), no synthetic
# input; graceful quit, kill only if still running after 30 s. REFUSES if Audio-DNA is already running or 7070 / 8080 has
# a listener. The caller holds /tmp/audiodna-live.lock.
#
# usage: probe-vupload.sh [out-base] [row,row,...]
#   VIDEO_APP / VIDEO_PY / VIDEO_ENV / VIDEO_FIXTURES as .harmony/probe-video.sh
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
for PT in 7070 8080; do
  lsof -nP -iTCP:$PT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PT already has a listener: $(lsof -nP -iTCP:$PT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/vupload.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
# The fixtures are encoded BEFORE the app starts (the 4K clip is the long pole, ~1 min): no encode competes with a row.
"$PY" "$ROOT/.harmony/probe-vupload.py" "$ROOT" "$OUT" --make-fixtures "${2:-}" || { echo "FAIL  fixtures"; exit 1; }
ENVARGS=(); [ -n "${VIDEO_ENV:-}" ] && ENVARGS=(--env "$VIDEO_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP" --args --test-mode
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 http://localhost:8080/api/health)" ] && [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
L8080="$(lsof -nP -iTCP:8080 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && { [ "$L7070" != "Audio-DNA" ] || [ "$L8080" != "Audio-DNA" ]; }; then echo "FAIL  7070 / 8080 answered by '$L7070' / '$L8080', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-vupload.py" "$ROOT" "$OUT" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health on 7070 and 8080"; fi
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
echo; [ "$RC" -eq 0 ] && echo "PROBE-VUPLOAD GREEN" || echo "PROBE-VUPLOAD RED"
exit "$RC"
