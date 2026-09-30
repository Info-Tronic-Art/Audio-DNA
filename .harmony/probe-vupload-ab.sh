#!/bin/bash
# probe-vupload-ab.sh -- s-rta-0929 vupload (plan-vupload.md 4.7 / 4.9 + HARMONY ADOPTION VU7): INTERLEAVED A/B launches
# of a probe script on two app bundles. Round r launches A then B (A B A B ...), so slow drift (other lanes' load, GPU
# state, thermal) hits both arms alike -- sequential per-arm batches drifted ~5 fps over an hour in diag-vfps. Each
# launch = one run of SCRIPT (.harmony/probe-video.sh or .harmony/probe-vupload.sh) with VIDEO_APP=<arm app> and the
# given rows; its "DATA ..." lines are collected into OUTDIR/ab.tsv as "<round> <arm> <DATA line>", then summarised by
# .harmony/probe-vupload-ab.py (the w2c 4K bar rule of plan 4.9, the u7 ratio of VU7, per-row medians).
#
# usage: probe-vupload-ab.sh A_APP B_APP ROUNDS SCRIPT ROWS OUTDIR
#   LOCK_LIB   (required) the rig's lock helper (acquire_quiet_lock / release_lock / adna / outwins); LANE set before.
#   ENV_A / ENV_B  optional VAR=value for each arm's launch (passed as VIDEO_ENV).
#   VIDEO_FIXTURES  shared fixture dir (encode once).
# Lock: acquire_quiet_lock (waits for no compiler BEFORE locking), held <= 11 min, then released and re-taken; a launch
# that saw a compiler (sampled every 2 s) is TAINTED and re-run (same arm, same round). After every launch it counts
# on-screen UserNotificationCenter windows (a "quit unexpectedly" dialog) -- non-zero STOPS the run.
set -u
A_APP=$1; B_APP=$2; ROUNDS=$3; SCRIPT=$4; ROWS=$5; OUT=$6
: "${LOCK_LIB:?set LOCK_LIB to the rig lock helper}"
: "${LANE:?set LANE before running}"
. "$LOCK_LIB"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MAIN="$(cd "$(git -C "$ROOT" rev-parse --path-format=absolute --git-common-dir 2>/dev/null)/.." 2>/dev/null && pwd)"
PY="${VIDEO_PY:-}"; if [ -z "$PY" ]; then for c in "$ROOT/.venv/bin/python" "$MAIN/.venv/bin/python"; do [ -x "$c" ] && { PY="$c"; break; }; done; fi
mkdir -p "$OUT"; TSV="$OUT/ab.tsv"; : > "$TSV"
compilers(){ { pgrep -x clang; pgrep -x 'clang++'; } 2>/dev/null | wc -l | tr -d ' '; }
unc(){ "$PY" -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))"; }
held=0; lockT=0; tainted=0
echo "A = $A_APP ($(shasum -a 256 "$A_APP/Contents/MacOS/Audio-DNA" | cut -c1-16))" | tee "$OUT/meta.txt"
echo "B = $B_APP ($(shasum -a 256 "$B_APP/Contents/MacOS/Audio-DNA" | cut -c1-16))" | tee -a "$OUT/meta.txt"
for r in $(seq 1 "$ROUNDS"); do
  for arm in A B; do
    if [ "$arm" = A ]; then app="$A_APP"; env="${ENV_A:-}"; else app="$B_APP"; env="${ENV_B:-}"; fi
    done_ok=0
    while [ $done_ok -eq 0 ]; do
      if [ $held -eq 1 ] && { [ "$(compilers)" != "0" ] || [ $(( $(date +%s) - lockT )) -gt 660 ]; }; then release_lock; held=0; fi
      if [ $held -eq 0 ]; then acquire_quiet_lock || { echo "no quiet lock"; exit 1; }; held=1; lockT=$(date +%s); fi
      LOG="$OUT/r${r}_$arm.log"
      ( while [ ! -f "$LOG.done" ]; do echo "$(date +%s) $(compilers)"; sleep 2; done ) > "$LOG.compilers" &
      SP=$!
      echo "$(date +%T) [r$r $arm] start: load avg $(sysctl -n vm.loadavg)" | tee -a "$OUT/meta.txt"
      VIDEO_APP="$app" VIDEO_ENV="$env" bash "$SCRIPT" "$OUT/runs" "$ROWS" > "$LOG" 2>&1; rc=$?
      touch "$LOG.done"; wait $SP 2>/dev/null
      u=$(unc); maxc=$(awk '{if ($2 > m) m = $2} END {print m + 0}' "$LOG.compilers")
      echo "$(date +%T) [r$r $arm] rc $rc, compilers seen $maxc, UserNotificationCenter windows $u, $(outwins), $(tail -1 "$LOG")" | tee -a "$OUT/meta.txt"
      [ "$u" != "0" ] && { echo "STOP: a UserNotificationCenter window is on screen"; [ $held -eq 1 ] && release_lock; exit 2; }
      if [ "$maxc" != "0" ]; then
        tainted=$((tainted + 1)); mv "$LOG" "$OUT/tainted-r${r}_$arm-$tainted.log"; echo "TAINTED (compiler) -- re-run"
        [ $tainted -gt 20 ] && { echo "too many tainted"; [ $held -eq 1 ] && release_lock; exit 1; }
      else
        done_ok=1
        grep '^DATA ' "$LOG" | sed "s/^/$r $arm /" >> "$TSV"
      fi
    done
  done
done
[ $held -eq 1 ] && release_lock
echo "$(date +%T) rounds $ROUNDS x 2 arms done (tainted $tainted)"
"$PY" "$ROOT/.harmony/probe-vupload-ab.py" "$TSV" | tee "$OUT/summary.txt"
