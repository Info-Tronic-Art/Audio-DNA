# run_mix.sh PREFIX SCEN LOADS ROUNDS "arm1=VAR=v,VAR2=v" "arm2=" ... -- INTERLEAVED arms: round r launches every arm
# once, in order (A B C A B C ...), so slow drift (other lanes' load, GPU, thermal) hits every arm alike. Each launch as
# run_arm.sh (open -g, production mode, ADNA_VFPS_FILE, drive.py SCEN LOADS, quit, Output / UNC window counts); output
# runs/PREFIX_ARM/rK. The lock is held <= 11 min, released and re-taken with acquire_quiet_lock; a launch that saw a
# compiler is TAINTED and re-run (same arm). APP / DIAG_OFF as run_arm.sh.
PREFIX=$1; SCEN=$2; LOADS=$3; ROUNDS=$4; shift 4
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
ARMS=("$@"); NA=${#ARMS[@]}
held=0; lockT=0; stop=0; tainted=0
for r in $(seq 1 $ROUNDS); do
  for spec in "${ARMS[@]}"; do
    arm=${spec%%=*}; envs=${spec#*=}
    done_ok=0
    while [ $done_ok -eq 0 ] && [ $stop -eq 0 ]; do
      if [ $held -eq 1 ] && { [ "$(compilers)" != "0" ] || [ $(( $(date +%s) - lockT )) -gt 660 ]; }; then release_lock; held=0; fi
      if [ $held -eq 0 ]; then acquire_quiet_lock || { echo "no quiet lock"; exit 1; }; held=1; lockT=$(date +%s); fi
      OUT=$S/runs/${PREFIX}_$arm/r$r; rm -rf $OUT; mkdir -p $OUT
      [ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; stop=1; break; }
      for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; stop=1; }; done
      [ $stop -eq 1 ] && break
      EA=()
      for kv in ${envs//,/ }; do EA+=(--env "$kv"); done
      [ "${DIAG_OFF:-0}" = "1" ] || EA+=(--env "ADNA_VFPS_FILE=$OUT/diag.tsv")
      echo "$(date +%T) [$PREFIX $arm r$r] load start: $(sysctl -n vm.loadavg) env: $envs" | tee $OUT/meta.txt
      open -g "${EA[@]}" --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
      for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 -H 'Connection: close' http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
      sleep 2
      outwins >> $OUT/meta.txt
      ( while [ ! -f $OUT/done ]; do echo "$(date +%s) $(compilers) $(sysctl -n vm.loadavg | cut -d' ' -f2)"; sleep 2; done ) > $OUT/compilers.txt &
      SP=$!
      $PY $S/tools/drive.py $OUT $SCEN $LOADS > $OUT/drive.out 2>&1; rc=$?
      touch $OUT/done; wait $SP 2>/dev/null
      echo "$(date +%T) drive rc=$rc load end: $(sysctl -n vm.loadavg)" >> $OUT/meta.txt
      grep WINDOW $OUT/drive.out | awk '{printf "%s %.1f  ", $2, $5}' | tee -a $OUT/meta.txt; echo
      outwins >> $OUT/meta.txt
      quit_app >> $OUT/meta.txt
      u=$(unc); echo "UserNotificationCenter windows: $u" >> $OUT/meta.txt
      [ "$u" != "0" ] && { echo "$(date +%T) STOP: a UserNotificationCenter window is on screen"; stop=1; }
      maxc=$(awk '{if ($2 > m) m = $2} END {print m + 0}' $OUT/compilers.txt)
      if [ "$maxc" != "0" ] || [ $rc -ne 0 ]; then
        tainted=$((tainted + 1)); echo "$(date +%T) TAINTED (compilers $maxc, rc $rc) -- re-run"; mv $OUT $S/runs/${PREFIX}_$arm/tainted-$r-$tainted
        [ $tainted -gt 30 ] && { echo "too many tainted"; stop=1; }
      else
        done_ok=1
      fi
    done
  done
done
[ $held -eq 1 ] && release_lock
echo "$(date +%T) [$PREFIX] rounds $ROUNDS x $NA arms done (tainted $tainted)"
outwins
