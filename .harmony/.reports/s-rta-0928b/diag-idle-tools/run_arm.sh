# run_arm.sh TAG FIXTURE N [VAR=val ...] -- N CLEAN launches of the instrumented build-lane app (production mode unless
# MODE=test; APP=... overrides the bundle). Each launch: open -g with --env, fixture via REST, settle, idle window, quit.
# Rig: never waits for quiet while holding the live lock (releases it and waits); a compiler seen during a launch's
# idle window (2 s sampling) marks the launch TAINTED and it is re-run (max N+6 attempts). Lock held <= ~12 min.
TAG=$1; FIX=$2; N=$3; shift 3
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-idle
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-idle
APP=${APP:-$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app}
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
IDLE_S=${IDLE_S:-30}; SETTLE_S=${SETTLE_S:-6}
LANE=diag-idle
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/lib/lock.sh
ENVARGS=()
for kv in "$@"; do ENVARGS+=(--env "$kv"); done
[ -n "$DIAGFILE_OFF" ] || ENVARGS+=(--env ADNA_DIAG_FILE=__OUT__/diag.tsv)
compilers(){ { pgrep -x clang; pgrep -x 'clang++'; } 2>/dev/null | wc -l | tr -d ' '; }
held=0; lockT=0; clean=0; attempt=0
while [ $clean -lt $N ] && [ $attempt -lt $((N + 6)) ]; do
  if [ $held -eq 1 ] && { [ "$(compilers)" != "0" ] || [ $(( $(date +%s) - lockT )) -gt 600 ]; }; then release_lock; held=0; fi
  if [ $held -eq 0 ]; then
    wait_quiet || { echo "$(date +%T) NOT QUIET after 30 min -- retry"; continue; }
    acquire_lock || exit 1; held=1; lockT=$(date +%s)
    [ "$(compilers)" != "0" ] && continue
  fi
  attempt=$((attempt + 1))
  OUT=$S/runs/$TAG/r$((clean + 1)); rm -rf $OUT; mkdir -p $OUT
  [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; break; }
  for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; release_lock; exit 1; }; done
  EA=("${ENVARGS[@]//__OUT__/$OUT}")
  echo "$(date +%T) [$TAG r$((clean + 1)) attempt $attempt] load start: $(sysctl -n vm.loadavg) env: $* mode: ${MODE:-prod} app: $APP" | tee $OUT/meta.txt
  if [ "$MODE" = "test" ]; then
    open -g "${EA[@]}" --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP" --args --test-mode
  else
    open -g "${EA[@]}" --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
  fi
  for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 -H 'Connection: close' http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
  sleep 2
  echo "$(date +%T) app up: $(adna | tr '\n' ' ')" | tee -a $OUT/meta.txt
  outwins | tee -a $OUT/meta.txt
  ( while [ ! -f $OUT/idle.json ]; do echo "$(date +%s) $(compilers) $(sysctl -n vm.loadavg | cut -d' ' -f2)"; sleep 2; done ) > $OUT/compilers.txt &
  SP=$!
  [ -n "$PRE_IDLE_HOOK" ] && bash -c "$PRE_IDLE_HOOK" >> $OUT/meta.txt 2>&1
  $PY $S/tools/idle.py $OUT $FIX $IDLE_S $SETTLE_S > $OUT/drive.out 2>&1
  rc=$?
  [ -n "$POST_IDLE_HOOK" ] && OUT=$OUT bash -c "$POST_IDLE_HOOK" >> $OUT/meta.txt 2>&1
  wait $SP 2>/dev/null
  echo "$(date +%T) idle done rc=$rc load end: $(sysctl -n vm.loadavg)" | tee -a $OUT/meta.txt
  outwins | tee -a $OUT/meta.txt
  quit_app | tee -a $OUT/meta.txt
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
