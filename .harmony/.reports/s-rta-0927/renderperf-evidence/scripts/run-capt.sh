# usage: bash run-capt.sh LABEL APP   (caller holds the lock)
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
. $SP/lock.sh
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
L=$1; APP=$2
OUT=$SP/capt/$L; rm -rf $OUT; mkdir -p $OUT
[ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; exit 1; }
lsof -nP -iTCP:7070 -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: 7070 has a listener"; exit 1; }
if [ -n "${SKIPQUIET:-}" ]; then Q=1; else wait_quiet; Q=$?; fi
echo "CAPT $L start $(date '+%F %T') quiet=$([ $Q -eq 0 ] && echo yes || echo NO) load=$(sysctl -n vm.loadavg) app=$APP"
open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
sleep 2
$PY $SP/capt.py $OUT $L
echo "CAPT $L end $(date '+%F %T') load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]"
quit_app
outwins
echo "--- app err.log capture lines:"
grep -E "\[Eyes\] (Captured frame|Processing capture|Failed|Invalid|Frame capture)" $OUT/app-err.log
