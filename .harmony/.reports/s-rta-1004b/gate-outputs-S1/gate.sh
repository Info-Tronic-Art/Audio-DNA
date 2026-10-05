#!/bin/bash
# Harmony's behavioural gate on lane outputs, stage S1 (pure + pool; no live row is ruled for S1). Serial ctest through the mutex.
R=/Users/boriskarpman/projects/RealTimeAudio; SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/31b4c846-ad01-4801-82d9-569a43efdd3b/scratchpad; WT=$R/.claude/worktrees/outputs-a; O=$SP/gate-outputs-S1
echo "$(date '+%F %T') GATE outputs S1 start"
echo "head: $(git -C $WT rev-parse --short HEAD)  branch: $(git -C $WT rev-parse --abbrev-ref HEAD)"
echo "status lines (expect 0): $(git -C $WT status --short | wc -l | tr -d ' ')"
echo "--- files changed 8b464a6..HEAD"
git -C $WT diff --name-only 8b464a6..HEAD
echo "--- build (expect nothing to compile: the code head was built by the builder)"
cmake --build $WT/build-lane -j 6 > $O/build.log 2>&1
rc=$?
echo "build rc=$rc; compiled objects: $(grep -c 'Building CXX\|Building OBJCXX' $O/build.log); last: $(tail -1 $O/build.log | cut -c1-120)"
echo "--- ctest -N"
ctest --test-dir $WT/build-lane -N > $O/ctestN.log 2>&1
tail -1 $O/ctestN.log
echo "--- full serial ctest through the mutex"
bash $SP/lib/ctest-mutex.sh $WT/build-lane > $O/ctest-full.log 2>&1
rc=$?
echo "ctest-mutex rc=$rc"; grep -E 'tests passed|tests failed|Total Test time|ctest rc=|\*\*\*Failed|MUTEX' $O/ctest-full.log | head -12
echo "--- stage cases"
ctest --test-dir $WT/build-lane -R "^(frame history|output look|surface pool)" > $O/ctest-stage.log 2>&1
rc=$?
echo "stage ctest rc=$rc"; grep -E 'tests passed|tests failed' $O/ctest-stage.log
echo "--- existing GL cases"
$WT/build-lane/tests/test_shared_frame_gl > $O/gl.log 2>&1
rc=$?
echo "gl rc=$rc"; tail -2 $O/gl.log | cut -c1-160
echo "--- mutants re-witnessed (the builder's runner; expected today: 22 ran, NOT RED = M-P2 only, EXIT 3)"
mkdir -p $O/mut
python3 $WT/.harmony/.reports/s-rta-1004b/outputs-S1-mutants.py $WT $WT/build-lane $O/mut > $O/mutants.log 2>&1
rc=$?
echo "mutant runner rc=$rc"; grep -E '^M-|NOT RED|EXIT|ran ' $O/mutants.log | cut -c1-110 | head -40
echo "worktree clean after mutants (expect 0): $(git -C $WT status --short | wc -l | tr -d ' ')"
echo "--- tsan (the stage's [tsan] cases)"
if [ -d $WT/build-tsan ]; then
  cmake --build $WT/build-tsan -j 6 --target test_frame_history > $O/tsan-build.log 2>&1
  rc=$?
  echo "tsan build rc=$rc"
  TSAN_OPTIONS="abort_on_error=0" ctest --test-dir $WT/build-tsan -L tsan -R "^frame history" --output-on-failure > $O/tsan.log 2>&1
  rc=$?
  echo "tsan ctest rc=$rc"; grep -E 'tests passed|tests failed' $O/tsan.log; echo "ThreadSanitizer warnings in log: $(grep -c 'WARNING: ThreadSanitizer' $O/tsan.log)"
else echo "no build-tsan dir in the worktree: TSan arm NOT RUN by me"; fi
echo "--- residue check: AppSettings cases side by side on MAIN's build, 5 runs with -j 4 (the builder saw 1 failure in one -j 4 full run)"
for i in 1 2 3 4 5; do
  bash $SP/lib/ctest-mutex.sh $R/build -R "AppSettings" -j 4 > $O/appsettings-j4-$i.log 2>&1
  rc=$?
  echo "run $i rc=$rc: $(grep -E 'tests passed|tests failed' $O/appsettings-j4-$i.log | tr '\n' ' ') $(grep -c '\*\*\*Failed' $O/appsettings-j4-$i.log) failed lines"
done
echo "$(date '+%F %T') GATE outputs S1 end"
