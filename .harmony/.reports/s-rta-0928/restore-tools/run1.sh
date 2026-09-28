# run1.sh TAG ARMS REPS -- one instrumented launch under the live lock
TAG=$1; ARMS=$2; REPS=$3
R=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w4
APP=$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
LANE=restorediag . /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/lib/lock.sh
OUT=$R/runs/$TAG
acquire_lock || exit 1
wait_quiet || { release_lock; exit 1; }
echo "$(date +%T) load: $(sysctl -n vm.loadavg)"
start_app $APP $OUT || { release_lock; exit 1; }
FIXVAR=${FIXVAR:-card} /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python $R/drive.py $OUT $W $ARMS $REPS > $OUT/drive.out 2>&1
echo "$(date +%T) drive exit $? load: $(sysctl -n vm.loadavg)"
quit_app
outwins
release_lock
