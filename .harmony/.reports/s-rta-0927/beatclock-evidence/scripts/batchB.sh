#!/bin/bash
# Batch B: GREEN on the commit-4 app -- probe-beatclock x3, probe-routines x3, witness + verify (d).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
source $S/lock.sh
acquire_lock || exit 1
trap 'release_lock; rm -f $WT/.venv' EXIT
ln -sfn /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
envline() { echo "($(date '+%T') load: $(uptime | sed 's/.*averages: //'); compilers: $(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x "clang\\+\\+" | wc -l | tr -d ' ') clang++)"; }
for i in 1 2 3; do
  echo "=== B-beatclock-$i (commit-4 app)"; envline
  BEATCLOCK_APP=$S/app-c4/Audio-DNA.app bash $WT/.harmony/probe-beatclock.sh $S/runs/B-bc$i; echo "exit $?"
done
for i in 1 2 3; do
  echo "=== B-routines-$i (commit-4 app)"; envline
  ROUTINES_APP=$S/app-c4/Audio-DNA.app ROUTINES_RECORD_PAUSE=1.8 bash $WT/.harmony/probe-routines.sh $S/runs/B-rt$i; echo "exit $?"
done
echo "=== B-witness (commit-4 app)"; envline
bash $S/witness.sh $S/app-c4/Audio-DNA.app $S/runs/B-wit; echo "exit $?"; envline
