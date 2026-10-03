#!/bin/bash
# probe-outputs.sh -- s-rta-0927 outputs-c1 = plan5 slice C1 (.harmony/.reports/s-rta-0926b/plan5-final.md 10.2).
# Live, SCREEN-SAFE witness of the output frame path: the main renderer copies the composition canvas into shared
# IOSurface frames (tap forced on over 8080 /api/set_output_tap) and 8080 /api/output_probe presents the newest
# frame through a PRIVATE GL context of the app -- no window -- with the same code the Output window uses. Rows,
# assertions and calibration: the docstring of .harmony/probe-outputs.py; fixtures: .harmony/probe-outputs.json.
#
# NO ROW OPENS AN OUTPUT WINDOW (the screen-safety law). o_no_window_opened samples the Quartz window list for the
# whole run; after the quit this script checks the FULL window list again (0 Audio-DNA windows). The only capture
# taken is of the app's MAIN window, by its Quartz window id (screencapture -l) -- never the screen.
#
# Clone of .harmony/probe-canvas.sh (refuse / fresh out dir / foreign-traffic check / graceful quit), launched in
# TEST MODE (`open -g ... --args --test-mode`: the 8080 routes exist only there, and only in a build configured
# with -DAUDIODNA_BUILD_TEST_SERVER=ON). REFUSES if Audio-DNA is already running.
#
# usage: probe-outputs.sh [out-base] [row,row,...]     (o_tap_cost runs only when named)
#   OUTP_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   OUTP_PY   python with PIL+numpy+requests+pyobjc Quartz (default: <root>/.venv, else the main checkout's .venv)
#   OUTP_ENV  optional VAR=value passed to the app via open --env
#   OUTP_SETTINGS  optional settings.json to SEED the run's scratch settings file with (copied to $OUT/settings.json
#             before launch). EVERY run points the app at $OUT/settings.json (open --env AUDIODNA_SETTINGS_FILE=...;
#             honoured only by a test-server build in --test-mode, s-rta-0927 outputs-c3), so an app under this probe
#             never reads or writes the user's real settings file; absent unless seeded.
# Every run captures into a FRESH dir: mktemp -d "<out-base>/outputs.XXXXXX" (out-base default /tmp).
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
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${OUTP_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${OUTP_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set OUTP_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests, Quartz' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests+Quartz (set OUTP_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
refuse_foreign_start || exit 64
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener: $(lsof -nP -iTCP:$PORT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/outputs.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
SETTINGS_FILE="$OUT/settings.json"
if [ -n "${OUTP_SETTINGS:-}" ]; then cp "$OUTP_SETTINGS" "$SETTINGS_FILE" || { echo "REFUSE: cannot seed $SETTINGS_FILE"; exit 64; }; fi
echo "settings: $SETTINGS_FILE ($([ -f "$SETTINGS_FILE" ] && echo "seeded, sha256 $(shasum -a 256 "$SETTINGS_FILE" | cut -c1-16)" || echo absent))"
ENVARGS=(--env "AUDIODNA_SETTINGS_FILE=$SETTINGS_FILE"); [ -n "${OUTP_ENV:-}" ] && ENVARGS+=(--env "$OUTP_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP" --args --test-mode
record_ourpid; echo "ours: pid ${OURPID:-none}"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then OUTP_SETTINGS_FILE="$SETTINGS_FILE" "$PY" "$ROOT/.harmony/probe-outputs.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# Every render_frame / output_probe this run asked for lands in $OUT. One anywhere else means another process drove
# this app over REST during the run -- its writes invalidate every row, so the run is RED.
FOREIGN="$(grep -oE '(Captured frame|Output probe): [^ ]*' "$OUT/err.log" 2>/dev/null | grep -vcE ": $OUT/")"
if [ "${FOREIGN:-0}" -gt 0 ]; then
  echo "FAIL  foreign REST traffic: $FOREIGN capture(s)/probe(s) outside $OUT during this run -- results INVALID"
  RC=1
else echo "PASS  no foreign render_frame / output_probe traffic during the run"; fi
# The main window, by its Quartz window id (window-only; never the screen) -- Harmony looks at it.
"$PY" - "$OUT" <<'PYEOF'
import sys, subprocess, Quartz
out = sys.argv[1]
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
wins = [(float(w['kCGWindowBounds']['Width']) * float(w['kCGWindowBounds']['Height']), int(w['kCGWindowNumber']))
        for w in wl if str(w.get('kCGWindowOwnerName', '')) == 'Audio-DNA' and int(w.get('kCGWindowLayer', 0)) == 0]
wins.sort(reverse=True)
if wins:
    subprocess.run(["screencapture", "-x", "-o", "-l", str(wins[0][1]), out + "/main-window.png"], check=False)
    print("      main window (Quartz id %d) -> %s/main-window.png" % (wins[0][1], out))
else:
    print("      no on-screen Audio-DNA main window to capture")
PYEOF
quit_ours || RC=1
if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
W="$("$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if str(w.get('kCGWindowOwnerName', '')) == 'Audio-DNA']))" 2>/dev/null)"
if [ "$W" = "0" ]; then echo "PASS  after quit: 0 Audio-DNA windows in the FULL Quartz window list (no output window survives)"
else echo "FAIL  after quit: Audio-DNA windows in the full window list = '$W'"; RC=1; fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-OUTPUTS GREEN" || echo "PROBE-OUTPUTS RED"
exit "$RC"
