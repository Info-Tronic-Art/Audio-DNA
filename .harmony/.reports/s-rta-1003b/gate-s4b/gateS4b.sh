#!/bin/bash
# Harmony GATE S4b -- the keys fix on lane/bf2-keys (worktree bf2keys): G1, selftest, R13 + R14 GREEN on the lane app and RED on the mutant app.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad
M=/Users/boriskarpman/projects/RealTimeAudio
W=$M/.claude/worktrees/bf2keys
PY=$M/.venv/bin/python
OUT=$SP/gate/s4b; mkdir -p $OUT; LOG=$OUT/gate.log; : > $LOG
say(){ echo "$(date '+%F %T') $*" | tee -a $LOG; }
say "START head $(git -C $W rev-parse --short HEAD); dirty non-build lines $(git -C $W status --short | grep -vc 'build-'); src/tests clean rc $(git -C $W diff --quiet -- src tests; echo $?); MUTANT files in src tests: $(grep -rl MUTANT $W/src $W/tests 2>/dev/null | wc -l | tr -d ' ')"
cmake --build $W/build-lane -j6 > $OUT/build.log 2>&1
rc=$?
say "BUILD rc=$rc :: objects compiled $(grep -c 'Building CXX' $OUT/build.log)"
say "top process: $(ps -Ao pcpu=,comm= | sort -rn | head -1 | cut -c1-80)"
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $W/build-lane > $OUT/g1.log 2>&1
rc=$?
rmdir /tmp/audiodna-ctest.lock
say "G1 rc=$rc :: $(grep -E 'tests passed' $OUT/g1.log | tail -1)"
say "G1 failed: $(sed -n '/The following tests FAILED/,$p' $OUT/g1.log | grep -E '^\s*[0-9]+ - ' | tr '\n' ';' | cut -c1-400)"
$W/build-lane/tests/test_binding_sync_nudge > $OUT/nudge.log 2>&1
rc=$?
say "test_binding_sync_nudge rc=$rc :: $(tail -2 $OUT/nudge.log | tr '\n' ' ' | cut -c1-120)"
$PY $W/.harmony/probe-sync-selftest.py > $OUT/selftest.log 2>&1
rc=$?
say "SELFTEST rc=$rc :: $(tail -1 $OUT/selftest.log | cut -c1-110)"
LANE=harmony-s4b . $SP/lib/lock.sh
acquire_lock >> $LOG 2>&1
rc=$?
if [ $rc -eq 0 ]; then
  PROBESYNC_PY=$PY PROBESYNC_ROWS=R0,R13,R14,R6 PROBESYNC_APP=$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app bash $W/.harmony/probe-sync.sh $OUT/green > $OUT/green.log 2>&1
  rc=$?
  say "GREEN arm (lane app) rc=$rc :: $(grep -E '^(PASS|FAIL)  R1[34]|FAIL group|R6' $OUT/green.log | tr '\n' ';' | cut -c1-700)"
  release_lock >> $LOG 2>&1
  acquire_lock >> $LOG 2>&1
  PROBESYNC_PY=$PY PROBESYNC_ROWS=R0,R13,R14,R6 PROBESYNC_APP=$W/build-mut-r13/AudioDNA_artefacts/Release/Audio-DNA.app bash $W/.harmony/probe-sync.sh $OUT/red > $OUT/red.log 2>&1
  rc=$?
  say "RED arm (mutant app build-mut-r13) rc=$rc :: $(grep -E '^(PASS|FAIL)  R1[34]|INFO  R1[34] [0-9]' $OUT/red.log | head -8 | tr '\n' ';' | cut -c1-700)"
  release_lock >> $LOG 2>&1
  sleep 16
  say "after batch: Audio-DNA pids [$(adna | tr '\n' ' ')] $(outwins 2>&1); UNC $($PY -c "import Quartz;print(len([w for w in Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID) if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"); settings sha $(shasum -a 256 ~/Library/Audio-DNA/settings.json | cut -c1-12)"
else
  say "LIVE ROWS BLOCKED (lock rc=$rc) -- not a pass"
fi
say "END"
