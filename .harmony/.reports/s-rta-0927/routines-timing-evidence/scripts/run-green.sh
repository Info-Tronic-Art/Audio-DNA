# usage: bash run-green.sh LABEL (main|lane) N
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/timing
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w2
. $S/lock.sh
LABEL=$1; WHICH=$2; N=$3
[ -e $WT/.venv ] || ln -s /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
for i in $(seq 1 $N); do
  wait_quiet; Q=$?
  acquire_lock || exit 1
  LOG=$S/runs-green/$LABEL-$i.log
  echo "RUN $LABEL-$i start $(date '+%F %T') quiet_before=$([ $Q -eq 0 ] && echo yes || echo NO) load=$(sysctl -n vm.loadavg) probe=$WT/.harmony/probe-routines.sh ($(shasum -a 256 $WT/.harmony/probe-routines.sh | cut -c1-12))" > $LOG
  if [ "$WHICH" = main ]; then
    AUDIODNA_LOCK_OWNER=routines-timing ROUTINES_APP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app ROUTINES_RECORD_PAUSE=1.8 bash $WT/.harmony/probe-routines.sh $S/runs-green/$LABEL-$i >> $LOG 2>&1
  else
    AUDIODNA_LOCK_OWNER=routines-timing ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8 bash $WT/.harmony/probe-routines.sh $S/runs-green/$LABEL-$i >> $LOG 2>&1
  fi
  echo "RUN $LABEL-$i end $(date '+%F %T') exit=$? load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]" >> $LOG
  for _ in $(seq 1 30); do [ -z "$(ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}')" ] && break; sleep 1; done
  release_lock
  grep "PASS /" $LOG | tail -1
  sleep 25
done
