#!/bin/bash
# Harmony's diagnosis run RD (ruling-bf2-stops section 4): launches of 6 takes x 12 s at dial 0 on build-lane AS BUILT at 68abc16.
# Verdict after every launch; stop on O1 (early-stop rule met) / O2 / O4; O5 only counts after 12 launches (decision HD1).
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad
M=/Users/boriskarpman/projects/RealTimeAudio
W=$M/.claude/worktrees/bf2
PY=$M/.venv/bin/python
OUT=$SP/gate/rd; mkdir -p $OUT; LOG=$OUT/gate.log; : > $LOG
say(){ echo "$(date '+%F %T') $*" | tee -a $LOG; }
say "START lane head $(git -C $W rev-parse --short HEAD); app binary $(stat -f '%Sm' $W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA) sha $(shasum -a 256 $W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA | cut -c1-12)"
$PY $W/.harmony/probe-sync-selftest.py > $OUT/selftest.log 2>&1
rc=$?
say "SELFTEST rc=$rc :: $(tail -1 $OUT/selftest.log | cut -c1-120)"
[ $rc -ne 0 ] && { say "STOP: selftest not green -- RD not run"; exit 0; }
LANE=harmony-rd . $SP/lib/lock.sh
n=0
while [ $n -lt 12 ]; do
  n=$((n+1))
  acquire_lock >> $LOG 2>&1
  rc=$?
  [ $rc -ne 0 ] && { say "BLOCKED lock rc=$rc at launch $n"; break; }
  say "launch $n: top process $(ps -Ao pcpu=,comm= | sort -rn | head -1 | cut -c1-70)"
  PROBESYNC_PY=$PY PROBESYNC_ROWS=R0,R7,R6 PROBESYNC_RD=1 PROBESYNC_R7_ARMS=0,0,0,0,0,0 PROBESYNC_R7_SECONDS=12 PROBESYNC_APP=$W/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app bash $W/.harmony/probe-sync.sh $OUT/rd-$n > $OUT/rd-$n.log 2>&1
  rc=$?
  release_lock >> $LOG 2>&1
  say "launch $n rc=$rc :: $(grep -E 'RD launch:' $OUT/rd-$n.log | tail -1 | cut -c1-260)"
  grep -E 'INFO  RD take' $OUT/rd-$n.log | cut -c1-210 >> $LOG
  $PY $W/.harmony/probe-sync.py --rd-verdict $OUT/rd-*/run-*/probe-sync.json > $OUT/verdict-$n.log 2>&1
  vrc=$?
  V=$(grep -E 'RD outcome|RD INCOMPLETE' $OUT/verdict-$n.log | tail -1 | cut -c1-300)
  say "verdict after $n launches (exit $vrc): $V"
  grep -E 'early-stop|WARN' $OUT/verdict-$n.log | cut -c1-240 >> $LOG
  case "$V" in
    *"RD outcome O1"*|*"RD outcome O2"*|*"RD outcome O4"*) say "STOP RULE: outcome reached after $n launches"; break;;
    *"RD outcome O5"*) [ $n -ge 12 ] && { say "O5 after 12 launches (HD1 satisfied)"; break; };;
  esac
done
sleep 16
say "after RD: Audio-DNA pids [$(adna | tr '\n' ' ')] $(outwins 2>&1); UNC windows $($PY -c "import Quartz;print(len([w for w in Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID) if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))")"
say "END after $n launches"
