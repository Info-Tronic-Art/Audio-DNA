#!/bin/bash
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
date
cmake --build $WT/build-lane -j3 > /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock/build-full.log 2>&1
rc=$?
grep -E "error:|FAILED" /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock/build-full.log | head -40
tail -3 /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock/build-full.log
echo "BUILD EXIT $rc"
date
