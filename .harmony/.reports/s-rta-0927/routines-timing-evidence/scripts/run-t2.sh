# usage: bash run-t2.sh RUNIDX LAYERID W H
echo "ARCHIVED RECORD (R-N1, s-rta-1003): this script quits Audio-DNA by name -- never run or source it; use .harmony/probe-quit-ours.sh" >&2; exit 64
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/timing
. $S/lock.sh
APP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
I=$1; LID=$2; W=$3; H=$4
OUT=$S/t2/run$I; mkdir -p $OUT
wait_quiet; Q=$?
acquire_lock || exit 1
adna(){ ps -eo pid=,ucomm= | awk '$2=="Audio-DNA"{print $1}'; }
[ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; release_lock; exit 1; }
echo "T2 run $I start $(date '+%F %T') quiet=$([ $Q -eq 0 ] && echo yes || echo NO) load=$(sysctl -n vm.loadavg) canvas=${W}x${H}"
open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP"
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:7070/api/health)" ] && break; sleep 1; done
sleep 2
$PY $S/t2.py $OUT $LID $W $H
echo "T2 run $I end $(date '+%F %T') load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x clang++ | wc -l | tr -d ' ')]"
osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1
for _ in $(seq 1 30); do [ -z "$(adna)" ] && break; sleep 1; done
[ -n "$(adna)" ] && { echo "pkill last resort"; kill $(adna); sleep 3; }
W2="$($PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName', '')) and 'Output' in str(w.get('kCGWindowName', ''))]))")"
echo "output windows: $W2; app running after quit: $([ -n "$(adna)" ] && echo YES || echo no)"
release_lock
