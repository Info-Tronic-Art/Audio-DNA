#!/bin/bash
# probe-boxes.sh -- lane bf9b (s-rta-1002b): decks are boxes of clips; the layers are ONE shared playing stack
# (plan-bf9b S4.1; ruling-bf9b amendment 14 + its FINAL CONSOLIDATED GATE LIST, K1-K10). Renamed from
# probe-deck-clock.sh (s-rta-0926b plan4 item 2: "inactive decks keep time"), whose rows ruling-bf9 K6 disposes of
# (the .py docstring). Boris: "when I switch between decks, do not change the clips playing in the layers or how they
# are playing. treat the decks as just a box of clips and I can switch between 20 decks looking for a clip and the
# playing will not be affected."
# Rows, fixtures, per-arm readers and bars: the docstring of .harmony/probe-boxes.py (REST on 7070, OSC on UDP 8000;
# every PNG decoded with PIL+numpy); numbers in .harmony/probe-boxes.json. Pictures are drawn with PIL and the video
# fixture made with ffmpeg in the fresh out dir at run start.
#
# ARMS: STAGE_P (the STAGE_P_HEAD app; RED expected) and BF9B (the lane head; GREEN expected). Run the STAGE_P arm's
# k7_old_take FIRST: it records the pre-bf9b take and prints its folder; pass it to the BF9B arm as BOXES_OLD_TAKE.
#
# Clone of probe-deck-clock.sh: TEST MODE launch (`open -g ... --args --test-mode`; inject_features and the
# /api/debug/* routes are test-mode only). REFUSES when ffmpeg is not on PATH. Screen-safe: open -g (never plain open
# / foreground exec), no screen capture (canvas captures via POST /api/render_frame), no Output window, no synthetic
# input. QUITS ONLY THE APP IT LAUNCHED (quit_ours, .harmony/probe-quit-ours.sh): graceful quit, kill of that one pid
# only if still running after 30 s; any other Audio-DNA is never touched. REFUSES if Audio-DNA is already running. The
# caller holds /tmp/audiodna-live.lock (PROBE RIG GATE below).
#
# Final line: "PROBE-BOXES GREEN" (exit 0) only when no row FAILED and none was BLOCKED; "PROBE-BOXES BLOCKED <n>
# [<row>, ...] (0 FAIL; <n> pre-registered bar(s) did not run -- not a pass)" (exit 3) when nothing failed but n
# pre-registered bars had no driver (their names in the brackets); "PROBE-BOXES RED" (exit 1 / 64) otherwise.
# ROW-COUNT PIN (ruling-bf9b-merge AM-9): a FULL run (no row list) must print exactly EXPECTED_ROWS "--- <row>"
# headers, and the .py's row list must hold that many; a mismatch (a row dropped from the list, a run that died
# early) is RED. A run with a row list prints its count without the pin.
# usage: probe-boxes.sh [out-base] [row,row,...]
#   BOXES_APP       app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   BOXES_PY        python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   BOXES_ENV       optional VAR=value passed to the app via open --env
#   BOXES_OLD_TAKE  the pre-bf9b take folder k7_old_take replays on the BF9B arm
# Every run captures into a FRESH dir: mktemp -d "<out-base>/boxes.XXXXXX" (out-base default /tmp); the app's
# stdout / stderr go to out.log / err.log there (k7_old_show counts its logLine in them).
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
# The rows of a full run: ruling-bf9b's K rows + k1b_duplicate (25: k1a_switch_static, k1b_switch_video,
# k1b_duplicate, k1c_switch_midfade, k1t_history_freeze, k1t_history_feedback, k1d_ia_speed_half, k1d_ib_pingpong,
# k1d_ii_opacity_blend, k1d_iii_connection, k2_nothing_unseen, k2v_decode, k3_autopilot, k4_ignore_column_across,
# k4b_empty_cell, k5_queue_link_off, k5_queue_link_on, k7_old_show, k7_old_take, k8_twenty_decks, k8b_browse_fire,
# k9a_remove_playing, k9b_remove_midfade, k9c_remove_undo, k10_fresh_and_resume). A new row re-pins this number.
EXPECTED_ROWS=25
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
adna_running() { [ -n "$(adna_pids)" ]; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours: quit ONLY the app this run launched
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
APP="${BOXES_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${BOXES_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set BOXES_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set BOXES_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
refuse_foreign_start || exit 64
command -v ffmpeg >/dev/null 2>&1 || { echo "REFUSE: ffmpeg not on PATH (the video fixtures are generated with it)"; exit 64; }
for PORT in 7070 8080; do
  lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener: $(lsof -nP -iTCP:$PORT -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
done
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/boxes.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${BOXES_ENV:-}" ] && ENVARGS=(--env "$BOXES_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP" --args --test-mode
record_ourpid; echo "ours: pid ${OURPID:-none}"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-boxes.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}" | tee "$OUT/py.log"; RC=${PIPESTATUS[0]}
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
# bf9b fix round (Harmony ruling R-N3, rulings-bf9b-merge.md): GREEN only when no row FAILED and none was BLOCKED (the
# .py exits 3 for FAIL 0 with BLOCKED rows): a BLOCKED bar never ran, so the verdict line says BLOCKED, never GREEN.
NBLOCKED="$(sed -n 's/^PY .* \([0-9][0-9]*\) BLOCKED (arm .*/\1/p' "$OUT/py.log" 2>/dev/null | tail -1)"
BROWS="$(sed -n 's/^PY-BLOCKED-ROWS \[\(.*\)\]$/\1/p' "$OUT/py.log" 2>/dev/null | tail -1)"
# ruling-bf9b-merge AM-9: the row-count pin. NRUN = the "--- <row>" headers this run printed; NREG = the .py's list.
NRUN="$(grep -c '^--- ' "$OUT/py.log" 2>/dev/null)"; NRUN="${NRUN:-0}"
NREG="$(sed -n 's/^PY-ROWS registered \([0-9][0-9]*\)$/\1/p' "$OUT/py.log" 2>/dev/null | tail -1)"
if [ -z "${2:-}" ]; then
  if [ "$NRUN" -eq "$EXPECTED_ROWS" ] && [ "${NREG:-0}" -eq "$EXPECTED_ROWS" ]; then echo "PASS  rows run $NRUN == EXPECTED_ROWS $EXPECTED_ROWS (registered ${NREG:-?})"
  else echo "FAIL  rows run $NRUN, registered ${NREG:-?} != EXPECTED_ROWS $EXPECTED_ROWS (full run)"; RC=1; fi
else echo "      rows run $NRUN of $EXPECTED_ROWS (row list given: no row-count pin)"; fi
echo
if [ "$RC" -eq 0 ]; then echo "PROBE-BOXES GREEN"
elif [ "$RC" -eq 3 ]; then echo "PROBE-BOXES BLOCKED ${NBLOCKED:-?} [${BROWS:-?}] (0 FAIL; ${NBLOCKED:-?} pre-registered bar(s) did not run -- not a pass)"
else echo "PROBE-BOXES RED"; fi
exit "$RC"
