#!/bin/bash
# full serial ctest on build-lane; $1 = log tag
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
date
ctest --test-dir $WT/build-lane -j1 --timeout 600 > $S/ctest-$1.log 2>&1
echo "CTEST EXIT $?"
grep -E "tests passed|tests failed|Total Test time" $S/ctest-$1.log
grep -E "^\s+[0-9]+ - .*\((Failed|Timeout|SEGFAULT|Subprocess|Exception|Not Run)" $S/ctest-$1.log | head -40
date
