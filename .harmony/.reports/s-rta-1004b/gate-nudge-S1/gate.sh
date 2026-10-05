#!/bin/bash
# Harmony's gate on lane nudge, stage S1: G-N0(b) (the golden is MAIN's: her own pristine 185147b tree) and G-N1 (unit). No app is launched.
R=/Users/boriskarpman/projects/RealTimeAudio; SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad; L=$R/.claude/worktrees/nudge; G=$R/.claude/worktrees/gn0; D=$R/build/_deps; O=$SP/gate-nudge-S1
A1='[golden] auto publishes=1965 hash=0xd731eb3d8ada4fe7 raw=0xe1d4fdd8db3d2a63'
A2='[golden] commands publishes=1965 hash=0xee912dc7695ff904 raw=0xf1c78019a1f873c8'
echo "$(date '+%F %T') GATE nudge S1 start; lane head $(git -C $L rev-parse --short HEAD); lane status lines $(git -C $L status --short | wc -l | tr -d ' ')"
echo "=== G-N0(b): pristine 185147b + the golden commit 3ba5ca1's two files (taken from the commit object)"
echo "golden commit files: $(git -C $L show --stat --format= 3ba5ca1 | tr '\n' ' ' | cut -c1-200)"
df -h /System/Volumes/Data | tail -1 | awk '{print "free: " $4}'
git -C $R worktree add --detach $G 185147b > $O/wt-add.log 2>&1
rc=$?
echo "worktree add rc=$rc; head $(git -C $G rev-parse --short HEAD)"
git -C $L show 3ba5ca1:tests/test_analysis_nudge_golden.cpp > $G/tests/test_analysis_nudge_golden.cpp
git -C $L show 3ba5ca1:tests/CMakeLists.txt > $G/tests/CMakeLists.txt
echo "pristine tree now differs from 185147b only by:"; git -C $G status --short; git -C $G diff --stat | tail -1
cmake -S $G -B $G/build-g -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $O/gn0-configure.log 2>&1
rc=$?
echo "configure rc=$rc"
cmake --build $G/build-g --target test_analysis_nudge_golden -j 6 > $O/gn0-build.log 2>&1
rc=$?
echo "build rc=$rc; objects compiled $(grep -c 'Building CXX\|Building OBJCXX' $O/gn0-build.log); errors $(grep -c ' error' $O/gn0-build.log)"
ok=0
for i in 1 2 3; do
  $G/build-g/tests/test_analysis_nudge_golden > $O/gn0-run$i.log 2>&1
  rc=$?
  l1=$(grep '^\[golden\] auto' $O/gn0-run$i.log); l2=$(grep '^\[golden\] commands' $O/gn0-run$i.log)
  echo "run $i rc=$rc | $l1 | $l2 | $(tail -2 $O/gn0-run$i.log | tr '\n' ' ' | cut -c1-90)"
  [ "$l1" = "$A1" ] && [ "$l2" = "$A2" ] && [ $rc -eq 0 ] && ok=$((ok+1))
done
grep -h 'golden-info' $O/gn0-run1.log | cut -c1-200
ctest --test-dir $G/build-g -R "golden" > $O/gn0-ctest.log 2>&1
rc=$?
echo "ctest golden rc=$rc: $(grep -E 'tests passed|tests failed|No tests' $O/gn0-ctest.log | tr '\n' ' ')"
[ $ok -eq 3 ] && echo "PASS  G-N0(b): 3 of 3 runs print main's two golden lines equal to the recorded constants" || echo "FAIL  G-N0(b): only $ok of 3 runs matched"
git -C $R worktree remove --force $G > $O/wt-remove.log 2>&1
rc=$?
echo "pristine worktree removed rc=$rc; listed still: $(git -C $R worktree list | grep -c gn0)"
echo "=== G-N1 unit, on the lane"
echo "--- files changed 8b464a6..HEAD"; git -C $L diff --name-only 8b464a6..HEAD | tr '\n' ' '; echo
cmake --build $L/build-lane -j 6 > $O/lane-build.log 2>&1
rc=$?
echo "lane build rc=$rc; compiled objects: $(grep -c 'Building CXX\|Building OBJCXX' $O/lane-build.log)"
ctest --test-dir $L/build-lane -N > $O/ctestN.log 2>&1; tail -1 $O/ctestN.log
bash $SP/lib/ctest-mutex.sh $L/build-lane > $O/ctest-full.log 2>&1
rc=$?
echo "ctest-mutex rc=$rc"; grep -E 'tests passed|tests failed|Total Test time|ctest rc=|\*\*\*Failed|MUTEX' $O/ctest-full.log | head -10
ctest --test-dir $L/build-lane -R "^T-N|the nudge word|golden" > $O/ctest-own.log 2>&1
rc=$?
echo "own cases rc=$rc: $(grep -E 'tests passed|tests failed' $O/ctest-own.log)"
python3 $L/.harmony/nudge-mutants-s1.py --selftest > $O/harness-selftest.log 2>&1
rc=$?
echo "harness selftest rc=$rc: $(tail -1 $O/harness-selftest.log | cut -c1-200)"
python3 $L/.harmony/nudge-mutants-s1.py > $O/mutants.log 2>&1
rc=$?
echo "mutant harness rc=$rc"; grep -E '^(RED|SURVIVED|NOT APPLIED|PASS|FAIL)' $O/mutants.log | cut -c1-120 | head -34
echo "lane worktree clean after (expect 0): $(git -C $L status --short | wc -l | tr -d ' ')"
echo "$(date '+%F %T') GATE nudge S1 end"
