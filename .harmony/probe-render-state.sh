#!/bin/bash
# probe-render-state.sh -- s-rta-0926b render lane. Live witness for three compositor state defects:
#   R2  a persistent layer from another deck shared its temporal buffer / frame ring with the active deck's
#       layer of the same id (layer ids are per deck)                       rows r2_temporal, r2_ring
#   R4  a persistent layer skipped clip transform + opacity, layer effects, layer transform, feedback and
#       its crossfade while its deck was inactive                           rows r4_*
#   R5  the temporal buffer was created inside the effect pass: on its first frame the pass drew into
#       framebuffer 0 and the layer showed only its target's glClear       rows r5_hold (deterministic),
#                                                                             r5_burst (the visible frame)
# Cases, fixtures: .harmony/probe-render-state.json; assertions + calibration: the docstring of
# .harmony/probe-render-state.py (REST calls on 7070, every PNG decoded with PIL+numpy).
#
# Screen-safe: open -g (never plain open / foreground exec), no screen capture, no Output window, no
# synthetic input; graceful quit, pkill only if still running after 30 s. REFUSES if Audio-DNA is already
# running. The caller holds /tmp/audiodna-live.lock (now enforced -- see PROBE RIG GATE below).
#
# usage: probe-render-state.sh [out-base] [row,row,...]
#   RSTATE_APP  app bundle to launch (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   RSTATE_PY   python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
#   RSTATE_ENV  optional VAR=value passed to the app via open --env (e.g. AUDIODNA_FBO_TRACE=1 on a build with
#               .harmony/.reports/s-rta-0926b/render-trace.diff applied; the trace lands in <out>/err.log)
# Every run captures into a FRESH dir: mktemp -d "<out-base>/rstate.XXXXXX" (out-base default /tmp).
# PROBE RIG GATE (probehygiene2, s-rta-0926b): refuses (exit 64) unless
# /tmp/audiodna-live.lock/owner exists; if AUDIODNA_LOCK_OWNER is set, it must match the owner
# file's first field. Every pgrep/pkill below is replaced by adna_pids/adna_running/adna_kill
# (defined right after the lock gate), which filter on `ps -o ucomm=` -- the KERNEL's real
# exec-time process name, set from the actual binary that was exec'd, NOT from argv[0] -- being
# exactly "Audio-DNA". Verified both directions: (1) a build's linker/compiler command line whose
# -o argument is ".../MacOS/Audio-DNA" has REAL ucomm clang/ld, never matched (the old `pgrep -f`
# substring match DID match it -- that was the bug); (2) a process that spoofs argv[0] via
# `exec -a .../MacOS/Audio-DNA <cmd>` still has the REAL exec'd command's ucomm (e.g. "sleep"), also
# never matched -- `pgrep -x` alone is NOT enough here, since macOS pgrep without -f still matches
# on the (spoofable) comm/argv[0] field, not on ucomm.
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
APP="${RSTATE_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
PY="${RSTATE_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$APP" ] || { echo "REFUSE: no app at $APP (set RSTATE_APP)"; exit 64; }
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set RSTATE_PY)"; exit 64; }
[ -f "$MEDIA/P16_01_baseline.png" ] || { echo "REFUSE: media/P16_01_baseline.png not found"; exit 64; }
adna_running && { echo "REFUSE: Audio-DNA already running"; exit 64; }
# s-rta-0926b render2: another process listening on 7070 (seen: a lane's stub_server.py) would either take the
# app's REST port or answer this probe itself -- refuse rather than measure the wrong process.
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port 7070 already has a listener: $(lsof -nP -iTCP:7070 -sTCP:LISTEN | tail -n +2 | awk '{print $1" "$2}' | head -2 | tr '\n' ' ')"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/rstate.XXXXXX")" || exit 64
echo "app: $APP"; echo "out: $OUT"
ENVARGS=(); [ -n "${RSTATE_ENV:-}" ] && ENVARGS=(--env "$RSTATE_ENV")
open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" ${ENVARGS[@]+"${ENVARGS[@]}"} "$APP"
UP=0; for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 "$A/api/health")" ] && { UP=1; break; }; sleep 1; done
sleep 2
RC=1
L7070="$(lsof -nP -iTCP:7070 -sTCP:LISTEN 2>/dev/null | awk 'NR>1{print $1}' | head -1)"
if [ "$UP" -eq 1 ] && [ "$L7070" != "Audio-DNA" ]; then echo "FAIL  port 7070 is answered by '$L7070', not Audio-DNA"; UP=0; fi
if [ "$UP" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-render-state.py" "$ROOT" "$OUT" "$MEDIA" "${2:-}"; RC=$?
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
echo; [ "$RC" -eq 0 ] && echo "PROBE-RENDER-STATE GREEN" || echo "PROBE-RENDER-STATE RED"
exit "$RC"
