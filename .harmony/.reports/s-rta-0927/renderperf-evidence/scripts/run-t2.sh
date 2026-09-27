# usage: bash run-t2.sh LABEL APP RUNIDX LAYERID W H   (caller holds the lock)
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
. $SP/lock.sh
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
L=$1; APP=$2; I=$3; LID=$4; W=$5; H=$6
OUT=$SP/t2/$L/run$I; rm -rf $OUT; mkdir -p $OUT
[ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; exit 1; }
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: 7070 has a listener"; exit 1; }
wait_quiet; Q=$?
echo "T2 $L run $I start $(date '+%F %T') quiet=$([ $Q -eq 0 ] && echo yes || echo NO) load=$(sysctl -n vm.loadavg) canvas=${W}x${H} app=$APP"
open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
sleep 2
$PY $SP/t2.py $OUT $LID $W $H
echo "T2 $L run $I end $(date '+%F %T') load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]"
quit_app
outwins
