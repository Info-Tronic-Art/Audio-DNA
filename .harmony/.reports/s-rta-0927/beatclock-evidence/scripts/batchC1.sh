#!/bin/bash
# Batch C1: GREEN on the final build (build-lane = commit 6) -- probe-routines x3, probe-beatclock x3, witness + (d).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
source $S/lock.sh
acquire_lock || exit 1
trap 'release_lock; rm -f $WT/.venv' EXIT
ln -sfn /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
envline() { echo "($(date '+%T') load: $(uptime | sed 's/.*averages: //'); compilers: $(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x "clang\\+\\+" | wc -l | tr -d ' ') clang++)"; }
for i in 1 2 3; do
  echo "=== C-routines-$i (build-lane, commit 6)"; envline
  ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8 bash $WT/.harmony/probe-routines.sh $S/runs/C-rt$i; echo "exit $?"
done
for i in 1 2 3; do
  echo "=== C-beatclock-$i (build-lane, commit 6)"; envline
  BEATCLOCK_BUILD_DIR=build-lane bash $WT/.harmony/probe-beatclock.sh $S/runs/C-bc$i; echo "exit $?"
done
echo "=== C-witness (build-lane)"; envline
bash $S/witness.sh $WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app $S/runs/C-wit; echo "exit $?"; envline
