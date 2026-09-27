#!/bin/bash
# Batch C2: regression probes on the final build (build-lane = commit 6).
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
source $S/lock.sh
acquire_lock || exit 1
trap 'release_lock; rm -f $WT/.venv' EXIT
ln -sfn /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
envline() { echo "($(date '+%T') load: $(uptime | sed 's/.*averages: //'); compilers: $(pgrep -x clang | wc -l | tr -d ' ') clang, $(pgrep -x "clang\\+\\+" | wc -l | tr -d ' ') clang++)"; }
echo "=== C2 probe-routine-display"; envline
ROUTINE_DISPLAY_BUILD_DIR=build-lane bash $WT/.harmony/probe-routine-display.sh $S/runs/C2-rd; echo "exit $?"
echo "=== C2 probe-resync"; envline
RESYNC_BUILD_DIR=build-lane bash $WT/.harmony/probe-resync.sh $S/runs/C2-resync; echo "exit $?"
echo "=== C2 probe-manual-bpm"; envline
MANUALBPM_BUILD_DIR=build-lane bash $WT/.harmony/probe-manual-bpm.sh $S/runs/C2-mbpm; echo "exit $?"
echo "=== C2 probe-downbeat-level"; envline
DOWNBEAT_BUILD_DIR=build-lane bash $WT/.harmony/probe-downbeat-level.sh $S/runs/C2-downbeat; echo "exit $?"
echo "=== C2 probe-step3"; envline
STEP3_BUILD_DIR=build-lane bash $WT/.harmony/probe-step3.sh $S/runs/C2-step3; echo "exit $?"; envline
