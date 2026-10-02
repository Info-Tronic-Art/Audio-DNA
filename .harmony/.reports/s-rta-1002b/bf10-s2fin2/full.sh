#!/bin/bash
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin2
echo "full build start $(date '+%F %T') HEAD=$(git -C $WT rev-parse --short HEAD) + working-tree test edit"
cmake --build $WT/build-lane -j3 > $D/full-build.log 2>&1
echo "full build exit=$? $(date '+%F %T') errors=$(grep -c 'error:' $D/full-build.log) warnings=$(grep -c 'warning:' $D/full-build.log)"
grep -E "Building|Linking" $D/full-build.log | head -20
