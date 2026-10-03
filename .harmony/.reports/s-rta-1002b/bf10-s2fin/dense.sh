#!/bin/bash
# EXPERIMENT (scratch, reverted): T6 with a read on EVERY frame while 0.2 s <= t <= 1.2 s, ruled clause unchanged.
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin
F=$WT/tests/test_projectm_canvas_gl.cpp
T=$WT/build-lane/tests/test_projectm_canvas_gl
cp $F $D/test.cpp.lane
SHA0=$(shasum -a 256 $F | cut -d' ' -f1)
perl -0pi -e 's/if \(frame % 10 == 0 \|\| extra\)/if (frame % 10 == 0 || extra || (tr >= 0.2 && tr <= 1.2))/' $F
echo "mutant edit lines: $(diff $D/test.cpp.lane $F | grep -c '^>')"
cmake --build $WT/build-lane --target test_projectm_canvas_gl -j3 > $D/dense-build.log 2>&1; echo "dense build exit=$?"
pass=0
for i in $(seq 1 20); do
  $T "T6 a MilkDrop soft cut draws into the canvas texture" > $D/dense-t6-$i.log 2>&1; rc=$?
  [ $rc -eq 0 ] && pass=$((pass+1))
  echo "dense T6 run $i rc=$rc $(grep -E 'frames=|mid_transition' $D/dense-t6-$i.log | cut -c6- | tr '\n' ' ')"
done
echo "dense T6 PASS $pass/20"
cp $D/test.cpp.lane $F
SHA1=$(shasum -a 256 $F | cut -d' ' -f1)
echo "restored test: $([ "$SHA0" = "$SHA1" ] && echo IDENTICAL || echo MISMATCH) $SHA1"
cmake --build $WT/build-lane --target test_projectm_canvas_gl -j3 > $D/dense-rebuild.log 2>&1; echo "rebuild exit=$? binary sha=$(shasum -a 256 $T | cut -c1-16)"
