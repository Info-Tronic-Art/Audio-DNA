#!/bin/bash
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin2
echo "waiting for ctest mutex $(date '+%F %T') (holder: $(cat /tmp/audiodna-ctest.lock/owner 2>/dev/null))"
n=0
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do n=$((n+1)); [ $((n % 8)) -eq 1 ] && echo "$(date +%T) ctest mutex busy"; sleep 15; done
echo "bf10-S2fin2 $$ $(date +%s)" > /tmp/audiodna-ctest.lock/owner
echo "ctest mutex acquired $(date '+%F %T') HEAD=$(git -C $WT rev-parse --short HEAD)"
ctest --test-dir $WT/build-lane -j1 --output-on-failure > $D/ctest-full.log 2>&1
echo "ctest exit=$? $(date '+%F %T')"
rm -rf /tmp/audiodna-ctest.lock
echo "ctest mutex released $(date '+%F %T')"
grep -E "tests passed|Total Test time" $D/ctest-full.log
grep -E "\(Failed\)|\(Timeout\)|\(SEGFAULT\)|Not Run|Skipped" $D/ctest-full.log | head -20
grep -E "test_projectm_canvas_gl:" $D/ctest-full.log | grep -E "Test +#" 
