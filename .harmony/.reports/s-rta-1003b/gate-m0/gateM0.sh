#!/bin/bash
# Harmony GATE M0 -- lane bf2 after the merge-in of main (head c45b579). Every rc is captured on its own line.
# Rows + PASS lines: the lane report's HAND-OVER TO HARMONY block (bf2-delta.md, stage M0) and ruling-bf2-delta.md section 5.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad
M=/Users/boriskarpman/projects/RealTimeAudio
W=$M/.claude/worktrees/bf2
OUT=$SP/gate/m0
mkdir -p $OUT
LOG=$OUT/gate.log
: > $LOG
say(){ echo "$(date '+%F %T') $*" | tee -a $LOG; }
top5(){ ps -Ao pcpu=,etime=,comm= | sort -rn | head -5 | cut -c1-110; }

say "START head $(git -C $W rev-parse --short HEAD) branch $(git -C $W rev-parse --abbrev-ref HEAD)"
git -C $W status --short | grep -v 'build-' > $OUT/status.txt
say "worktree dirty lines (non-build): $(wc -l < $OUT/status.txt | tr -d ' ')"

# ---- G1 full ctest, serial, under the cross-lane mutex
top5 > $OUT/ps-g1.txt
until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done
ctest --test-dir $W/build-lane > $OUT/g1.log 2>&1
rc=$?
rm -rf /tmp/audiodna-ctest.lock
say "G1 rc=$rc :: $(grep -E 'tests passed' $OUT/g1.log | tail -1)"
say "G1 failed: $(sed -n '/The following tests FAILED/,$p' $OUT/g1.log | grep -E '^\s*[0-9]+ - ' | tr '\n' ';' | cut -c1-700)"
say "G1 not-run / skipped lines: $(grep -cE 'Not Run|\*\*\*Skipped' $OUT/g1.log)"
say "G1 top process before: $(head -1 $OUT/ps-g1.txt)"

# ---- G3 (a) lints, (b) zero-heap
$W/build-lane/tests/test_snapshot_time_lint "[lint]" > $OUT/g3a1.log 2>&1
rc=$?
say "G3a snapshot_time_lint rc=$rc :: $(tail -3 $OUT/g3a1.log | tr '\n' ' ' | cut -c1-200)"
$W/build-lane/tests/test_log_line_lint > $OUT/g3a2.log 2>&1
rc=$?
say "G3a log_line_lint rc=$rc :: $(tail -3 $OUT/g3a2.log | tr '\n' ' ' | cut -c1-200)"
$W/build-lane/tests/test_analysis_sync_alloc > $OUT/g3b.log 2>&1
rc=$?
say "G3b analysis_sync_alloc rc=$rc :: $(grep -E 'operator-new|All tests passed|FAILED' $OUT/g3b.log | tr '\n' ';' | cut -c1-400)"

# ---- G2 TSan unit
bash $W/.harmony/probe-tsan-unit.sh > $OUT/g2.log 2>&1
rc=$?
say "G2 rc=$rc :: $(grep -E 'finds|tests passed|ctest rc|FAIL' $OUT/g2.log | tr '\n' ';' | cut -c1-500)"
say "G2 ThreadSanitizer report lines: $(grep -c 'WARNING: ThreadSanitizer' $OUT/g2.log)"

# ---- QUIT SWEEP + SMOKE under the live lock (own pid only; never an Output window)
LANE=harmony-m0 . $SP/lib/lock.sh
acquire_lock >> $LOG 2>&1
rc=$?
if [ $rc -eq 0 ]; then
  bash $W/.harmony/probe-quit-ours-selftest.sh --sweep > $OUT/sweep.log 2>&1
  rc=$?
  say "SWEEP rc=$rc :: $(grep SELFTEST $OUT/sweep.log | tail -1)"
  sha_before=$(shasum -a 256 ~/Library/Audio-DNA/settings.json 2>/dev/null | cut -d' ' -f1)
  start_app $W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app $OUT/smoke test >> $LOG 2>&1
  rc=$?
  if [ $rc -eq 0 ]; then
    hc=$(curl -s -m 5 -H 'Connection: close' -o $OUT/health.json -w '%{http_code}' http://127.0.0.1:7070/api/health)
    sc=$(curl -s -m 5 -H 'Connection: close' -o $OUT/sync.json -w '%{http_code}' http://127.0.0.1:7070/api/sync)
    say "SMOKE health $hc sync $sc :: $(head -c 260 $OUT/sync.json | tr '\n' ' ')"
    outwins >> $LOG 2>&1
    quit_app >> $LOG 2>&1
    rc=$?
    say "SMOKE quit rc=$rc"
  else
    say "SMOKE BLOCKED (start_app refused, rc=$rc) -- not a pass"
  fi
  sha_after=$(shasum -a 256 ~/Library/Audio-DNA/settings.json 2>/dev/null | cut -d' ' -f1)
  say "settings.json sha256 same before / after: $([ "$sha_before" = "$sha_after" ] && echo yes || echo NO)"
  release_lock >> $LOG 2>&1
  sleep 16
  $M/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
a=[w for w in wl if 'Audio-DNA' in str(w.get('kCGWindowOwnerName',''))]
u=[w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]
print('after live batch: audio-dna windows %d, Output-named %d, UNC windows %d' % (len(a), len([w for w in a if 'Output' in str(w.get('kCGWindowName',''))]), len(u)))" >> $LOG 2>&1
  say "Audio-DNA pids after batch: [$(adna | tr '\n' ' ')]"
else
  say "LIVE ROWS BLOCKED (lock, rc=$rc) -- not a pass"
fi

# ---- [timing] quiet run (ruling section 5 G1): no compiler, no app, no commit in the last 2 minutes; nothing above 20 % CPU
sleep 120
quiet=0; n=0
while [ $n -lt 90 ]; do
  t=$(ps -Ao pcpu= | sort -rn | head -1 | tr -d ' ')
  if [ "$(echo "$t <= 20.0" | bc)" = "1" ]; then quiet=$((quiet+1)); else quiet=0; fi
  [ $quiet -ge 3 ] && break
  n=$((n+1)); sleep 10
done
top5 > $OUT/ps-timing.txt
if [ $quiet -ge 3 ]; then
  say "TIMING pre-run ps (quiet: 3 samples <= 20 %): $(head -1 $OUT/ps-timing.txt)"
  ctest --test-dir $W/build-lane -R "\[timing\]" --repeat until-fail:3 -V > $OUT/timing.log 2>&1
  rc=$?
  say "TIMING rc=$rc :: $(grep -E 'tests passed' $OUT/timing.log | tail -1)"
else
  say "TIMING BLOCKED-NOT-QUIET after 15 min (top: $(head -1 $OUT/ps-timing.txt)) -- not a pass, not a fail; one INFO run follows"
  ctest --test-dir $W/build-lane -R "\[timing\]" -V > $OUT/timing-info.log 2>&1
  rc=$?
  say "TIMING-INFO (loaded machine, no verdict) rc=$rc :: $(grep -E 'tests passed' $OUT/timing-info.log | tail -1)"
fi
top5 > $OUT/ps-timing-after.txt
say "TIMING top process after: $(head -1 $OUT/ps-timing-after.txt)"
say "END"
