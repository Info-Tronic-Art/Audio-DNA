#!/bin/bash
# run.sh <label>: build the test target, ctest -R + direct run, logs into $D/<label>-*.log
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin2
B=$WT/build-lane
L=$1
echo "[$L] build start $(date '+%F %T') HEAD=$(git -C $WT rev-parse --short HEAD) ProjectMSource.cpp sha=$(shasum -a 256 $WT/src/sources/ProjectMSource.cpp | cut -c1-16) test sha=$(shasum -a 256 $WT/tests/test_projectm_canvas_gl.cpp | cut -c1-16)"
cmake --build $B --target test_projectm_canvas_gl -j3 > $D/$L-build.log 2>&1
echo "[$L] build exit=$? $(date '+%F %T') warnings=$(grep -c 'warning:' $D/$L-build.log) errors=$(grep -c 'error:' $D/$L-build.log)"
ctest --test-dir $B -R test_projectm_canvas_gl --output-on-failure > $D/$L-ctest.log 2>&1
echo "[$L] ctest exit=$? $(date '+%F %T')"
grep -E "tests passed|Test +#" $D/$L-ctest.log
$B/tests/test_projectm_canvas_gl "T6 a MilkDrop soft cut draws into the canvas texture" > $D/$L-t6.log 2>&1
echo "[$L] direct T6 exit=$? | $(grep -E '^(test cases|All tests)' $D/$L-t6.log) | $(grep -E 'frames=|mid_transition' $D/$L-t6.log | cut -c6- | tr '\n' ' ')"
echo "[$L] T6 reads by t: <0.2 $(grep '^DATA soft-cut t=' $D/$L-t6.log | awk '{t=substr($3,3)+0; if(t<0.2)n++} END{print n+0}') | 0.2-1.2 $(grep '^DATA soft-cut t=' $D/$L-t6.log | awk '{t=substr($3,3)+0; if(t>=0.2&&t<=1.2)n++} END{print n+0}') | >1.2 $(grep '^DATA soft-cut t=' $D/$L-t6.log | awk '{t=substr($3,3)+0; if(t>1.2)n++} END{print n+0}')"
