#!/bin/bash
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin
T=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10/build-lane/tests/test_projectm_canvas_gl
echo "start $(date '+%F %T') binary sha=$(shasum -a 256 $T | cut -c1-16)"
for i in $(seq 11 30); do
  $T "T6 a MilkDrop soft cut draws into the canvas texture" > $D/rep-t6-$i.log 2>&1; rc=$?
  echo "T6 run $i rc=$rc | $(grep 'mid_transition' $D/rep-t6-$i.log | cut -c6-) | (near_A,near_B) t<1.0: $(grep '^DATA soft-cut t=' $D/rep-t6-$i.log | awk '{t=substr($3,3)+0; if (t<1.0) {split($10,b,"=");split($11,a,"="); printf "(%s,%s) ", a[2], b[2]}}')"
done
echo "end $(date '+%F %T')"
