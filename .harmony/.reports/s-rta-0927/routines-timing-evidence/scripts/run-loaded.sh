# usage: bash run-timed.sh LABEL APP N
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/timing
. $S/lock.sh
LABEL=$1; APP=$2; N=$3
for i in $(seq 1 $N); do
  Q=9
  acquire_lock || exit 1
  LOG=$S/runs-loaded/$LABEL-$i.log
  { echo "RUN $LABEL-$i start $(date '+%F %T') LOADED-RUN(no quiet wait) load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]"; } > $LOG
  AUDIODNA_LOCK_OWNER=routines-timing ROUTINES_APP="$APP" ROUTINES_RECORD_PAUSE=1.8 bash $S/probe-routines-timed.sh $S/runs-loaded/$LABEL-$i >> $LOG 2>&1
  echo "RUN $LABEL-$i end $(date '+%F %T') exit=$? load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]" >> $LOG
  # ensure our app is gone
  for _ in $(seq 1 30); do [ -z "$(ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}')" ] && break; sleep 1; done
  release_lock
  grep "PASS /" $LOG | tail -1
  sleep 25   # let other lanes interleave
done
