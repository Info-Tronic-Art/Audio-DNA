#!/bin/bash
# probe-milkdrop.sh -- s-rta-1002b lane bf10 (plan-bf10.md I4, ruling-bf10.md amendments 2 / 3 / 5 / 9 / 14 / 17).
# Live witness that MilkDrop draws into its own composition-canvas-sized framebuffer: every render_frame PNG of a
# MilkDrop picture covers the whole canvas (alpha 255 everywhere, one colour for a solid fixture), the Preview panel's
# letter-box bars stay plain, a canvas change / composition load / context cycle / deck round trip keeps it whole, and
# a layer drawn over MilkDrop is not disturbed; plus the A/B perf rows (perf_md_*). Rows, bars and calibration: the
# docstring of .harmony/probe-milkdrop.py; fixtures and bars: .harmony/probe-milkdrop.json.
#
# Launch / quit, three ways (TEST MODE always: load_milkdrop_preset / set_composition_params / render_frame are 8080):
#   LOCK_LIB set (the rig lock helper; LANE set): the helper's start_app / quit_app (quit_app touches ONLY the pid
#                start_app recorded). This is the path .harmony/probe-vupload-ab.sh drives (it exports LOCK_LIB).
#   MILKDROP_ATTACH=1: no launch, no quit -- the caller started the app (e.g. with the helper) and quits it. It
#                attaches ONLY to the harness's own test-mode app (Harmony ruling R1, s-rta-1002b: Boris uses this Mac
#                and this app): the one running Audio-DNA pid must equal MILKDROP_ATTACH_PID, else the pid the lock
#                helper's start_app recorded (LOCK_LIB + LANE); that pid must own the 8080 listener and 8080
#                /api/health must answer. Otherwise: REFUSE, exit 2, no request sent (/api/health is asked only once
#                the pid and the listener match). Selftest: .harmony/probe-milkdrop-selftest.sh (no app, no request).
#   neither: standalone, cloned from probe-canvas.sh: open -g ... --args --test-mode, then a graceful quit of ONLY
#                the pid this script launched (refuses to quit any other Audio-DNA).
# Screen-safe: open -g, never an Output window (set_output_tap opens none), no synthetic input, window-only captures
# (m6, by Quartz window id), no full-screen capture. REFUSES if Audio-DNA is already running (except ATTACH).
#
# usage: probe-milkdrop.sh [out-base] [row,row,...]
#   MILKDROP_APP / VIDEO_APP   app bundle (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   MILKDROP_PY                python with PIL+numpy+requests+pyobjc (default: <root>/.venv, else the main checkout's)
#   MILKDROP_MODE              lane (default) | pre  (calibration on the pre-lane app: see probe-milkdrop.py)
#   MILKDROP_ARM               A | B label for the m3_rewarm DATA line
#   MILKDROP_ATTACH_PID        the pid MILKDROP_ATTACH=1 may attach to (see above)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/milkdrop.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE: refuses (exit 64) unless /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set it must
# match the owner file's first field. Process matching by the kernel's ucomm, never pgrep -f.
set -u
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${MILKDROP_APP:-${VIDEO_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}}"
PY="${MILKDROP_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests, Quartz' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests+pyobjc (set MILKDROP_PY)"; exit 64; }
ATTACH="${MILKDROP_ATTACH:-0}"
if [ "$ATTACH" = 1 ]; then
  adna_running || { echo "REFUSE: MILKDROP_ATTACH=1 but no Audio-DNA is running"; exit 64; }
  # R1: an Audio-DNA no harness started is Boris's -- m4 / m6 / m9 / m10 would replace his composition over 7070.
  RUNPID="$(adna_pids | tr -d ' \n')"
  WANTPID="${MILKDROP_ATTACH_PID:-}"
  if [ -z "$WANTPID" ] && [ -n "${LOCK_LIB:-}" ]; then   # the helper names start_app's pid record OURPID
    WANTPID="$(LANE="${LANE:-${LOCK_LANE:-}}"; . "$LOCK_LIB" >/dev/null 2>&1 && cat "$OURPID" 2>/dev/null | tr -d ' \n')"
  fi
  L8080="$(lsof -nP -t -iTCP:8080 -sTCP:LISTEN 2>/dev/null | sort -u | tr -d ' \n')"
  if [ -z "$WANTPID" ] || [ "$RUNPID" != "$WANTPID" ] || [ "$L8080" != "$RUNPID" ] \
     || [ -z "$(curl -s --max-time 2 -H 'Connection: close' http://localhost:8080/api/health)" ]; then
    echo "REFUSE: the running Audio-DNA is not a test-mode app this run started (it may be Boris's)"
    echo "        running pid ${RUNPID:-none}; expected ${WANTPID:-none} (MILKDROP_ATTACH_PID, else LOCK_LIB + LANE's start_app record); 8080 listener ${L8080:-none}"
    exit 2
  fi
else
  [ -d "$APP" ] || { echo "REFUSE: no app at $APP (set MILKDROP_APP)"; exit 64; }
  adna_running && { echo "REFUSE: Audio-DNA already running (pid $(adna_pids | tr '\n' ' '))-- never touch it"; exit 64; }
  for PORT in 7070 8080; do
    lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener"; exit 64; }
  done
fi
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/milkdrop.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"; echo "mode: ${MILKDROP_MODE:-lane} arm: ${MILKDROP_ARM:-?} attach: $ATTACH"
LOG=""; OURPID=""
if [ "$ATTACH" = 1 ]; then
  UP=1
elif [ -n "${LOCK_LIB:-}" ]; then
  : "${LANE:?LOCK_LIB needs LANE}"
  . "$LOCK_LIB"
  if start_app "$APP" "$OUT" test; then UP=1; LOG="$OUT/app-err.log"; else echo "FAIL  start_app refused"; exit 64; fi
else
  open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$APP" --args --test-mode
  UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' http://localhost:8080/api/health)" ] && { UP=1; break; }; sleep 1; done
  sleep 2; OURPID="$(adna_pids | tr -d ' \n')"; LOG="$OUT/err.log"
fi
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-milkdrop.py" "$ROOT" "$OUT" "${2:-}"; RC=$?
else echo "FAIL  app never answered /api/health"; fi
# A render_frame capture outside $OUT means another process drove this app over REST during the run: INVALID.
if [ -n "$LOG" ]; then
  FOREIGN="$(grep -o 'Captured frame: [^ ]*' "$LOG" 2>/dev/null | grep -vc "Captured frame: $OUT/")"
  if [ "${FOREIGN:-0}" -gt 0 ]; then echo "FAIL  foreign REST traffic: $FOREIGN render_frame capture(s) outside $OUT -- results INVALID"; RC=1
  else echo "PASS  no foreign render_frame traffic during the run"; fi
fi
if [ "$ATTACH" != 1 ]; then
  if [ -n "${LOCK_LIB:-}" ]; then quit_app || RC=1
  else
    RUN="$(adna_pids | tr -d ' \n')"
    if [ -n "$RUN" ] && [ "$RUN" = "$OURPID" ]; then
      osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
      for _ in $(seq 1 30); do adna_running || break; sleep 1; done
      [ "$(adna_pids | tr -d ' \n')" = "$OURPID" ] && { kill "$OURPID" 2>/dev/null; sleep 2; }
    elif [ -n "$RUN" ]; then echo "REFUSE quit: running Audio-DNA pid $RUN is not the pid this probe launched ($OURPID)"; RC=1; fi
  fi
  if adna_running; then echo "FAIL  app still running"; RC=1; else echo "PASS  app terminated"; fi
fi
echo; [ "$RC" -eq 0 ] && echo "PROBE-MILKDROP GREEN" || echo "PROBE-MILKDROP RED"
exit "$RC"
