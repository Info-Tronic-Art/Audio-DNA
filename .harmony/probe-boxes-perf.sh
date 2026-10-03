#!/bin/bash
# probe-boxes-perf.sh -- gate B6 of lane bf9b (ruling-bf9b B6; ruling-bf9b-merge AM-15 and section 5 "B6 PERF").
# Bars, definitions and the quiet rule: the docstring of .harmony/probe-boxes-perf.py.
#
# usage: probe-boxes-perf.sh [out-base]            the gate run: interleaved, PERF_RUNS runs per arm, PERF_SECS each
#        probe-boxes-perf.sh --selftest            no app, no lock: recorded run means must judge PASS, and the same
#                                                  numbers with the 20-deck arm shifted by 2 ms must judge FAIL
#        probe-boxes-perf.sh --dry [out-base]      one short pass (PERF_RUNS default 1, PERF_SECS default 10): the
#                                                  numbers, NO verdict
#   PERF_FH_APP    the lane's Release app (default: <root>/build/AudioDNA_artefacts/Release/Audio-DNA.app)
#   PERF_MAIN_APP  MAIN0, the pre-merge main app, for B6(i) and B6(iii); unset: "B6(i) NOT RUN"
#   PERF_PARTS     comma list of i, ii, iii (default: all three)
#   PERF_RUNS      runs per arm (default 5; the bar needs >= 5)      PERF_SECS  seconds sampled per run (default 60)
#   PERF_PY        python with PIL+numpy+requests (default: <root>/.venv, else the main checkout's .venv)
# Order: B6(ii) in ONE launch of FH, the two arms alternating (1 deck, 20 decks, 20, 1, ...), then FH's walk (iii);
# B6(i) = one launch per run, MAIN0 and FH alternating; MAIN0's walk (iii) after its last run.
# Last line: "PROBE-BOXES-PERF DONE (...)" (exit 0; read the B6 lines above it), "PROBE-BOXES-PERF RED (...)" (a
# fixture / VALID failure, exit 1) or "PROBE-BOXES-PERF BLOCKED (machine not quiet: <ps line>)" (exit 3; never a pass).
# Screen-safe: open -g with --test-mode, no Output window, no synthetic input, no capture. QUITS ONLY THE APP IT
# LAUNCHED (quit_ours, .harmony/probe-quit-ours.sh). The caller holds /tmp/audiodna-live.lock (taken with
# acquire_quiet_lock) and commits nothing to the repository while it runs (each one fires a background graph rebuild).
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; A='http://127.0.0.1:7070'
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
PY="${PERF_PY:-}"
if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
[ -n "$PY" ] && "$PY" -c 'import PIL, numpy, requests' 2>/dev/null || { echo "REFUSE: no python with PIL+numpy+requests (set PERF_PY)"; exit 64; }
DRY=0
case "${1:-}" in
  --selftest) "$PY" "$ROOT/.harmony/probe-boxes-perf.py" selftest; exit $? ;;
  --dry) DRY=1; shift ;;
esac
LOCK_OWNER_FILE=/tmp/audiodna-live.lock/owner
[ -f "$LOCK_OWNER_FILE" ] || { echo "REFUSE: no live lock held -- mkdir /tmp/audiodna-live.lock && echo \"<lane> \$\$ \$(date +%s)\" > $LOCK_OWNER_FILE first"; exit 64; }
if [ -n "${AUDIODNA_LOCK_OWNER:-}" ]; then
  LOCK_OWNER="$(cut -d' ' -f1 "$LOCK_OWNER_FILE" 2>/dev/null)"
  [ "$LOCK_OWNER" = "$AUDIODNA_LOCK_OWNER" ] || { echo "REFUSE: live lock owner '$LOCK_OWNER' != AUDIODNA_LOCK_OWNER '$AUDIODNA_LOCK_OWNER'"; exit 64; }
fi
adna_pids() { ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
. "$ROOT/.harmony/probe-quit-ours.sh"   # refuse_foreign_start / record_ourpid / quit_ours
FH="${PERF_FH_APP:-$ROOT/build/AudioDNA_artefacts/Release/Audio-DNA.app}"
M0="${PERF_MAIN_APP:-}"
PARTS=",${PERF_PARTS:-i,ii,iii},"
if [ "$DRY" -eq 1 ]; then RUNS="${PERF_RUNS:-1}"; export PERF_SECS="${PERF_SECS:-10}"; else RUNS="${PERF_RUNS:-5}"; export PERF_SECS="${PERF_SECS:-60}"; fi
MEDIA="$ROOT/media"; [ -f "$MEDIA/P16_01_baseline.png" ] || MEDIA="$MAIN/media"
[ -d "$FH" ] || { echo "REFUSE: no app at $FH (set PERF_FH_APP)"; exit 64; }
command -v ffmpeg >/dev/null 2>&1 || { echo "REFUSE: ffmpeg not on PATH (the video fixtures are generated with it)"; exit 64; }
BASE="${1:-/tmp}"; mkdir -p "$BASE"; OUT="$(mktemp -d "$BASE/boxes-perf.XXXXXX")" || exit 64
RES="$OUT/results.json"
DRYNOTE=""; [ "$DRY" -eq 1 ] && DRYNOTE=" DRY (no verdict)"
echo "FH: $FH"; echo "MAIN0: ${M0:-<none>}"; echo "out: $OUT"; echo "runs per arm $RUNS, $PERF_SECS s each, parts ${PARTS}${DRYNOTE}"
RC=0; NOTQUIET=""

# session <app> <label> <jobs>: one launch, the jobs, the quit of that pid. Sets RC / NOTQUIET.
session() {
  [ -n "$NOTQUIET" ] && return
  refuse_foreign_start || { RC=64; return; }
  for PORT in 7070 8080; do
    lsof -nP -iTCP:$PORT -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: port $PORT already has a listener"; RC=64; return; }
  done
  open -g --stdout "$OUT/out.log" --stderr "$OUT/err.log" "$1" --args --test-mode
  record_ourpid
  local up=0 prc=0
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 1 -H 'Connection: close' "$A/api/health")" ] && { up=1; break; }; sleep 1; done
  sleep 2
  if [ "$up" -eq 1 ]; then
    "$PY" "$ROOT/.harmony/probe-boxes-perf.py" session "$ROOT" "$OUT" "$MEDIA" "$RES" "$2" "$3" | tee -a "$OUT/py.log"
    prc=${PIPESTATUS[0]}
    if [ "$prc" -eq 4 ]; then NOTQUIET="$(sed -n 's/^NOTQUIET //p' "$OUT/py.log" | tail -1)"
    elif [ "$prc" -ne 0 ]; then RC=1; fi
  else echo "FAIL  $2 never answered /api/health"; RC=1; fi
  quit_ours || RC=1
  sleep 3
}

has() { [ "${PARTS#*,$1,}" != "$PARTS" ]; }
if has ii; then
  JOBS=""
  for r in $(seq 1 "$RUNS"); do
    if [ $((r % 2)) -eq 1 ]; then JOBS="$JOBS,one:$r,twenty:$r"; else JOBS="$JOBS,twenty:$r,one:$r"; fi
  done
  has iii && JOBS="$JOBS,walk"
  session "$FH" FH "${JOBS#,}"
elif has iii; then session "$FH" FH walk; fi
if has i && [ -n "$M0" ] && [ "$RC" -eq 0 ]; then
  [ -d "$M0" ] || { echo "REFUSE: no app at $M0 (PERF_MAIN_APP)"; exit 64; }
  for r in $(seq 1 "$RUNS"); do
    LASTJ="i:$r"; [ "$r" -eq "$RUNS" ] && has iii && LASTJ="i:$r,walk"
    if [ $((r % 2)) -eq 1 ]; then session "$M0" MAIN0 "$LASTJ"; session "$FH" FH "i:$r"
    else session "$FH" FH "i:$r"; session "$M0" MAIN0 "$LASTJ"; fi
  done
fi
echo
if [ -n "$NOTQUIET" ]; then echo "PROBE-BOXES-PERF BLOCKED (machine not quiet: $NOTQUIET)"; exit 3; fi
if [ "$RC" -ne 0 ]; then echo "PROBE-BOXES-PERF RED (a fixture, VALID clause or launch failed: see above; rc $RC)"; exit "$RC"; fi
if [ "$DRY" -eq 1 ]; then "$PY" "$ROOT/.harmony/probe-boxes-perf.py" verdict "$RES" dry; echo "PROBE-BOXES-PERF DONE (dry pass: no verdict)"
else "$PY" "$ROOT/.harmony/probe-boxes-perf.py" verdict "$RES"; echo "PROBE-BOXES-PERF DONE (read the B6 lines above)"; fi
exit 0
