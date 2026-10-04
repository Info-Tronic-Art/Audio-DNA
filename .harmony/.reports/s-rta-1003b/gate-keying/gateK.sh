#!/bin/bash
# Harmony's behavioural sample of the keying audit: re-run noise + two keying entries + three blend entries on main's app.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/00e87ddd-eec9-42a5-94d1-5dc67e66ea7d/scratchpad
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/keying
OUT=$SP/gate/k; mkdir -p $OUT; LOG=$OUT/gate.log; : > $LOG
say(){ echo "$(date '+%F %T') $*" | tee -a $LOG; }
LANE=harmony-k . $SP/lib/lock.sh
acquire_lock >> $LOG 2>&1
rc=$?
[ $rc -ne 0 ] && { say "BLOCKED lock rc=$rc"; exit 0; }
bash $W/.harmony/probe-keying-audit.sh $OUT/run1 inputs,noise,keying 0,5 > $OUT/run1.log 2>&1
rc=$?
say "KEYING sample rc=$rc :: $(grep -iE 'verdict|NO-OP|WORKS|ALIAS|BROKEN|noise|floor' $OUT/run1.log | tail -8 | tr '\n' ';' | cut -c1-900)"
bash $W/.harmony/probe-keying-audit.sh $OUT/run2 inputs,noise,blend 0,1,30 > $OUT/run2.log 2>&1
rc=$?
say "BLEND sample rc=$rc :: $(grep -iE 'verdict|NO-OP|WORKS|ALIAS|BROKEN|noise|floor' $OUT/run2.log | tail -8 | tr '\n' ';' | cut -c1-900)"
release_lock >> $LOG 2>&1
sleep 16
say "after batch: Audio-DNA pids [$(adna | tr '\n' ' ')] $(outwins 2>&1)"
say "END"
