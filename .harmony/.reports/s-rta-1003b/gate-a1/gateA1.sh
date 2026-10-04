#!/bin/bash
# Harmony GATE A1 -- lane bf2 at the head after S3f + S4 + fix round: unit rows only (G1, G3, G2, probe self-tests). rc on its own line.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad
M=/Users/boriskarpman/projects/RealTimeAudio
W=$M/.claude/worktrees/bf2
OUT=$SP/gate/a1; mkdir -p $OUT; LOG=$OUT/gate.log; : > $LOG
say(){ echo "$(date '+%F %T') $*" | tee -a $LOG; }
say "START head $(git -C $W rev-parse --short HEAD); dirty non-build lines: $(git -C $W status --short | grep -vc 'build-')"
say "top process: $(ps -Ao pcpu=,comm= | sort -rn | head -1 | cut -c1-90)"
# is the build dir current with the head? (a stale binary would gate the wrong tree)
cmake --build $W/build-lane -j6 > $OUT/build.log 2>&1
rc=$?
say "BUILD rc=$rc :: compiled objects: $(grep -c 'Building CXX' $OUT/build.log) :: $(tail -1 $OUT/build.log | cut -c1-120)"
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $W/build-lane > $OUT/g1.log 2>&1
rc=$?
rm -rf /tmp/audiodna-ctest.lock
say "G1 rc=$rc :: $(grep -E 'tests passed' $OUT/g1.log | tail -1)"
say "G1 failed: $(sed -n '/The following tests FAILED/,$p' $OUT/g1.log | grep -E '^\s*[0-9]+ - ' | tr '\n' ';' | cut -c1-700)"
say "G1 not-run / skipped lines: $(grep -cE 'Not Run|\*\*\*Skipped' $OUT/g1.log)"
$W/build-lane/tests/test_snapshot_time_lint "[lint]" > $OUT/g3a1.log 2>&1
rc=$?
say "G3a snapshot_time_lint rc=$rc :: $(tail -2 $OUT/g3a1.log | tr '\n' ' ' | cut -c1-160)"
$W/build-lane/tests/test_log_line_lint > $OUT/g3a2.log 2>&1
rc=$?
say "G3a log_line_lint rc=$rc :: $(tail -2 $OUT/g3a2.log | tr '\n' ' ' | cut -c1-160)"
$W/build-lane/tests/test_analysis_sync_alloc > $OUT/g3b.log 2>&1
rc=$?
say "G3b analysis_sync_alloc rc=$rc :: $(grep -cE '0 operator-new calls' $OUT/g3b.log) zero-heap lines; $(grep -E 'All tests passed|FAILED' $OUT/g3b.log | tr '\n' ' ' | cut -c1-120)"
bash $W/.harmony/probe-tsan-unit.sh > $OUT/g2.log 2>&1
rc=$?
say "G2 rc=$rc :: $(grep -E 'finds|tests passed|ctest rc|FAIL' $OUT/g2.log | tr '\n' ';' | cut -c1-400)"
say "G2 ThreadSanitizer report lines: $(grep -c 'WARNING: ThreadSanitizer' $OUT/g2.log)"
$M/.venv/bin/python $W/.harmony/probe-sync-selftest.py > $OUT/selftest.log 2>&1
rc=$?
say "PROBE-SYNC-SELFTEST rc=$rc :: $(tail -1 $OUT/selftest.log | cut -c1-160)"
bash $W/.harmony/probe-quit-ours-selftest.sh --fast > $OUT/quit.log 2>&1
rc=$?
say "QUIT-SELFTEST (fast) rc=$rc :: $(grep SELFTEST $OUT/quit.log | tail -1)"
for t in test_beat_lead test_binding_sync_nudge test_sync_witness; do
  $W/build-lane/tests/$t > $OUT/$t.log 2>&1
  rc=$?
  say "$t rc=$rc :: $(tail -2 $OUT/$t.log | tr '\n' ' ' | cut -c1-140)"
done
say "END"
