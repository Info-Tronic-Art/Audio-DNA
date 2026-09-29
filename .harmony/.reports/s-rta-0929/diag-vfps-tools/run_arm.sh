# run_arm.sh TAG SCEN N [LOADS] -- N CLEAN launches of an app (default: the instrumented build-lane app), production
# mode, open -g. Env for the app: ARM_ENV="VAR=val VAR2=val2" (each passed with open --env); ADNA_VFPS_FILE is added
# unless DIAG_OFF=1. Each launch: open -g, wait for 7070, drive.py SCEN, quit. Rig: the lock is taken with
# acquire_quiet_lock (never held while waiting for quiet), held <= ~12 min then released (45 s cooldown); a compiler seen
# during a launch (2 s sampling) marks it TAINTED and it is re-run (max N+6 attempts). After every launch: Output-named
# windows and UserNotificationCenter windows are counted (a non-zero UNC count STOPS the batch).
TAG=$1; SCEN=$2; N=$3; LOADS=${4:-}
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps
APP=${APP:-$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app}
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
LANE=diag-vfps
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/lib/lock.sh
compilers(){ { pgrep -x clang; pgrep -x 'clang++'; } 2>/dev/null | wc -l | tr -d ' '; }
unc(){ $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName', ''))]))"; }
held=0; lockT=0; clean=0; attempt=0; stop=0
while [ $clean -lt $N ] && [ $attempt -lt $((N + 6)) ] && [ $stop -eq 0 ]; do
  if [ $held -eq 1 ] && { [ "$(compilers)" != "0" ] || [ $(( $(date +%s) - lockT )) -gt 660 ]; }; then release_lock; held=0; fi
  if [ $held -eq 0 ]; then
    acquire_quiet_lock || { echo "$(date +%T) could not get a quiet lock"; exit 1; }
    held=1; lockT=$(date +%s)
  fi
  attempt=$((attempt + 1))
  OUT=$S/runs/$TAG/r$((clean + 1)); rm -rf $OUT; mkdir -p $OUT
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; break; }
  for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; release_lock; exit 1; }; done
  EA=()
  for kv in $ARM_ENV; do EA+=(--env "$kv"); done
  [ "${DIAG_OFF:-0}" = "1" ] || EA+=(--env "ADNA_VFPS_FILE=$OUT/diag.tsv")
  echo "$(date +%T) [$TAG r$((clean + 1)) attempt $attempt] load start: $(sysctl -n vm.loadavg) env: $ARM_ENV app: $APP" | tee $OUT/meta.txt
  open -g "${EA[@]}" --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 -H 'Connection: close' http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
  sleep 2
  echo "$(date +%T) app up: $(adna | tr '\n' ' ')" | tee -a $OUT/meta.txt
  outwins | tee -a $OUT/meta.txt
  ( while [ ! -f $OUT/done ]; do echo "$(date +%s) $(compilers) $(sysctl -n vm.loadavg | cut -d' ' -f2)"; sleep 2; done ) > $OUT/compilers.txt &
  SP=$!
  $PY $S/tools/drive.py $OUT $SCEN $LOADS > $OUT/drive.out 2>&1
  rc=$?
  touch $OUT/done; wait $SP 2>/dev/null
  echo "$(date +%T) drive done rc=$rc load end: $(sysctl -n vm.loadavg)" | tee -a $OUT/meta.txt
  grep WINDOW $OUT/drive.out | tee -a $OUT/meta.txt
  outwins | tee -a $OUT/meta.txt
  quit_app | tee -a $OUT/meta.txt
  u=$(unc); echo "UserNotificationCenter windows: $u" | tee -a $OUT/meta.txt
  [ "$u" != "0" ] && { echo "$(date +%T) STOP: a UserNotificationCenter window is on screen"; stop=1; }
  maxc=$(awk '{if ($2 > m) m = $2} END {print m + 0}' $OUT/compilers.txt)
  if [ "$maxc" != "0" ] || [ $rc -ne 0 ]; then
    echo "$(date +%T) TAINTED (compilers seen: $maxc, rc $rc) -- re-run" | tee -a $OUT/meta.txt
    mv $OUT $S/runs/$TAG/tainted-$attempt
  else
    clean=$((clean + 1))
  fi
done
[ $held -eq 1 ] && release_lock
echo "$(date +%T) [$TAG] clean $clean / $N in $attempt attempts"
outwins
