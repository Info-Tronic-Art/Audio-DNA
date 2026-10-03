#!/bin/bash
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin
T=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10/build-lane/tests/test_projectm_canvas_gl
echo "start $(date '+%F %T') binary sha=$(shasum -a 256 $T | cut -c1-16)"
for i in 1 2 3 4 5 6 7 8 9 10; do
  $T "T6 a MilkDrop soft cut draws into the canvas texture" > $D/rep-t6-$i.log 2>&1; rc=$?
  kind=$(grep '^DATA soft-cut t=' $D/rep-t6-$i.log | head -8 | awk '{print $6}' | tr -d 'median=' | tr '\n' ' ')
  echo "T6 run $i rc=$rc | $(grep -E '^(test cases|All tests)' $D/rep-t6-$i.log) | $(grep 'mid_transition' $D/rep-t6-$i.log | cut -c6-) | medians: $kind"
done
for i in 1 2 3 4 5; do
  $T "T5 render() leaves the caller's GL state exactly as it found it" > $D/rep-t5-$i.log 2>&1; rc=$?
  echo "T5 run $i rc=$rc | $(grep -E '^(test cases|All tests)' $D/rep-t5-$i.log) | errors seen: $(grep -o 'T5 ([a-e])[^:]*errors=\[[^]]*\]' $D/rep-t5-$i.log | grep -v 'errors=\[ \]' | sed 's/draw=.*errors=/errors=/' | sort -u | tr '\n' ';')"
done
for i in 1 2 3 4 5; do
  $T "T4 MilkDrop draws round shapes round at the canvas's shape" > $D/rep-t4-$i.log 2>&1; rc=$?
  echo "T4 run $i rc=$rc | $(grep -E '^(test cases|All tests)' $D/rep-t4-$i.log) | $(grep 'alpha255' $D/rep-t4-$i.log | awk '{print $3,$4}' | tr '\n' ' ')"
done
echo "end $(date '+%F %T')"
