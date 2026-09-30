#!/bin/bash
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; WG=$M/.claude/worktrees/rta0929b-gopcache
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad; G=$S/gate; O=$G/final; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$G/pre-gop/Audio-DNA.app; REF=$G/w10ref; PY=$M/.venv/bin/python
LANE=harmony-gate
. $S/lib/lock.sh
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+|AB-VERDICT|^(PASS|FAIL)  [a-z0-9_]+\[' "$1" | tail -${2:-4} | cut -c1-230; }
run() { n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 3 | tr '\n' ' ')"; quit_app >/dev/null; }
ab() { n=$1; shift; echo "### AB $n $(date +%T)"; LANE=$LANE LOCK_LIB=$S/lib/lock.sh ENV_A="${6:-}" ENV_B="${7:-}" bash $H/probe-vupload-ab.sh "$1" "$2" "$3" "$4" "$5" $O/ab-$n > $O/ab-$n.log 2>&1; echo "rc=$? :: $(grep -E 'PASS|FAIL|median|verdict' $O/ab-$n.log | tail -8 | cut -c1-200 | tr '\n' '|')"; }
ab ab7 $PRE $APP 5 $H/probe-vupload.sh u7_reverse_pingpong,u8_reverse_column_1080x4,u9_reverse_cache_drop,u10_reverse_4k,u11_reverse_column_return,u12_reverse_pixel_identity
ab ab256 $PRE $APP 5 $H/probe-vupload.sh u8_reverse_column_1080x4 "" ADNA_GOPCACHE_BUDGET_MB=256
ab abw $PRE $APP 5 $H/probe-video.sh w1c_column_trigger_1080x4,w2c_steady_4kx4_blit,w6b_retrigger_mid_fade
ab abw7 $PRE $APP 5 $H/probe-video.sh w7_message_thread_no_wait
echo "### async-load interleaved x5 per arm $(date +%T)"
for i in 1 2 3 4 5; do for arm in PRE APP; do
  [ $arm = PRE ] && A=$PRE || A=$APP
  acquire_quiet_lock || exit 70
  ASYNCLOAD_APP=$A bash $H/probe-async-load.sh $O/al-$arm-$i > $O/al-$arm-$i.log 2>&1
  quit_app >/dev/null; release_lock
  echo "al $arm $i load=$(sysctl -n vm.loadavg | cut -c1-12) :: $(grep -E 'end_hold_witness|PY [0-9]+ PASS' $O/al-$arm-$i.log | tr '\n' ' ' | cut -c1-200)"
done; done
echo "### step3 re-run $(date +%T)"; pmset -g assertions 2>/dev/null | grep -i 'audio' | head -3
acquire_quiet_lock || exit 70
STEP3_BUILD_DIR=build bash $H/probe-step3.sh > $O/step3-rerun.log 2>&1; echo "step3 rc $? :: $(grep -E 'PASS / .* FAIL|^FAIL' $O/step3-rerun.log | tail -4 | cut -c1-160 | tr '\n' '|')"
quit_app >/dev/null; release_lock
sleep 16; $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(\"UNC windows (OptionAll)\", len([w for w in wl if \"UserNotificationCenter\" in str(w.get(\"kCGWindowOwnerName\",\"\"))]))"
echo "### AB DONE $(date +%T)"
