# batch.sh "TAG:SCEN:ENV1,ENV2;TAG2:SCEN2:..." -- one lock hold, several launches (<= ~15 min), rig rules.
# Waits for no compiler BEFORE taking the lock; a compiler appearing mid-batch ends the batch (the rest -> skipped.txt).
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media
APP=${APP:-$SP/apps/instr.app}
LANE=diag-media
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/lib/lock.sh
busy(){ [ -n "$(pgrep -x clang; pgrep -x 'clang\+\+')" ]; }
while true; do
  wait_quiet || exit 1
  acquire_lock || exit 1
  busy || break
  echo "$(date +%T) compiler started after the lock -- releasing, retry"; release_lock; sleep 30
done
IFS=';' read -ra RUNS <<< "$1"
for R in "${RUNS[@]}"; do
  TAG=$(echo $R | cut -d: -f1); SCEN=$(echo $R | cut -d: -f2); ENVS=$(echo $R | cut -d: -f3)
  if busy; then echo "$(date +%T) compiler running -- skip $TAG"; echo "$R" >> $SP/runs/skipped.txt; continue; fi
  OUT=$SP/runs/$TAG; mkdir -p $OUT
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; break; }
  lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: 7070 has a listener"; break; }
  ENVARGS=(--env DIAG_MEDIA_HOOK=1)
  IFS=',' read -ra EV <<< "$ENVS"; for e in "${EV[@]}"; do [ -n "$e" ] && ENVARGS+=(--env "$e=1"); done
  echo "$(date +%T) $TAG scen=$SCEN env=${ENVARGS[*]} load: $(sysctl -n vm.loadavg)" | tee $OUT/meta.txt
  open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "${ENVARGS[@]}" "$APP"
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
  sleep 2
  echo "$(date +%T) app up: $(adna | tr '\n' ' ') $(outwins)" | tee -a $OUT/meta.txt
  /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python $SP/tools/drive.py $OUT $SCEN > $OUT/drive.out 2>&1
  echo "$(date +%T) drive exit $? load: $(sysctl -n vm.loadavg) compiler_during: $(busy && echo YES || echo no) $(outwins)" | tee -a $OUT/meta.txt
  quit_app | tee -a $OUT/meta.txt
  outwins | tee -a $OUT/meta.txt
  sleep 3
done
[ -n "$(adna)" ] && { echo "APP STILL RUNNING"; quit_app; }
release_lock
