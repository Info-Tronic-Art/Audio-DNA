#!/bin/bash
# run.sh <label>: build the test target, run ctest -R + direct, logs into $D/<label>-*.log
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin
B=$WT/build-lane
L=$1
echo "[$L] build start $(date '+%F %T') HEAD=$(git -C $WT rev-parse --short HEAD) ProjectMSource.cpp sha=$(shasum -a 256 $WT/src/sources/ProjectMSource.cpp | cut -c1-16) test sha=$(shasum -a 256 $WT/tests/test_projectm_canvas_gl.cpp | cut -c1-16)"
cmake --build $B --target test_projectm_canvas_gl -j3 > $D/$L-build.log 2>&1
echo "[$L] build exit=$? $(date '+%F %T') warnings=$(grep -c 'warning:' $D/$L-build.log)"
ctest --test-dir $B -R test_projectm_canvas_gl --output-on-failure > $D/$L-ctest.log 2>&1
echo "[$L] ctest exit=$? $(date '+%F %T')"
grep -E "tests passed|Test +#" $D/$L-ctest.log
$B/tests/test_projectm_canvas_gl > $D/$L-direct.log 2>&1
echo "[$L] direct exit=$?"
grep -E "^test cases|^assertions" $D/$L-direct.log
