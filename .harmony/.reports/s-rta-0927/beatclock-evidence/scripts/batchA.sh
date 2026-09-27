#!/bin/bash
# Batch A: live RED -- probe-beatclock on the pre-change main app and on the commit-1 app; probe-routines on the commit-1 app.
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
source $S/lock.sh
acquire_lock || exit 1
trap 'release_lock; rm -f $WT/.venv' EXIT
ln -sfn /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
env() { echo "($(date '+%T') load: $(uptime | sed 's/.*averages: //'); compilers: $(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x clang++ | wc -l | tr -d ' ') clang++)"; }
echo "=== A1 probe-beatclock on the PRE-CHANGE main app"; env
BEATCLOCK_APP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app bash $WT/.harmony/probe-beatclock.sh $S/runs/A1
echo "exit $?"
echo "=== A2 probe-beatclock on the commit-1 app (hook, old reader)"; env
BEATCLOCK_APP=$S/app-c1/Audio-DNA.app bash $WT/.harmony/probe-beatclock.sh $S/runs/A2
echo "exit $?"
echo "=== A3 probe-routines on the commit-1 app (row 7s RED)"; env
ROUTINES_APP=$S/app-c1/Audio-DNA.app ROUTINES_RECORD_PAUSE=1.8 bash $WT/.harmony/probe-routines.sh $S/runs/A3
echo "exit $?"; env
