#!/bin/bash
# s-rta-1002 bt2: merge + Harmony gates (ruling-bt2-seats.md FINAL GATE LIST, adoption H1-H8).
# usage: bt2-gate.sh <PRE app copy> <expected ctest cases> [nomerge]
M=/Users/boriskarpman/projects/RealTimeAudio; H=$M/.harmony; W=$M/.claude/worktrees/bt2
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad
O=$S/gate/bt2; mkdir -p $O
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$1; EXP=$2; PY=$M/.venv/bin/python
LANE=harmony-gate
. $S/lib/lock.sh
sm() { grep -E '^(FAIL) |PY [0-9]+ PASS|PASS [0-9]+ / FAIL|[0-9]+ PASS / [0-9]+ FAIL|PROBE-[A-Z0-9-]+|[0-9]+ PASS, [0-9]+ FAIL' "$1" | tail -${2:-4} | cut -c1-230; }
unc() { $PY -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print(len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"; }
tcc() { /usr/bin/log show --start "$1" --predicate 'process == "tccd" AND eventMessage CONTAINS "AUTHREQ"' --style compact 2>/dev/null > $O/tcc-$2.log
  python3 - "$O/tcc-$2.log" <<'PY'
import re,sys,collections
svc={};subj={};res={}
for l in open(sys.argv[1],errors='ignore'):
  m=re.search(r'msgID=([\d.]+)',l)
  if not m: continue
  k=m.group(1)
  if 'AUTHREQ_CTX' in l: s=re.search(r'service=(\w+)',l); svc[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_SUBJECT' in l: s=re.search(r'subject=([^,]+)',l); subj[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_RESULT' in l: s=re.search(r'authValue=(\d+), authReason=(\d+)',l); res[k]=s.groups() if s else '?'
c=collections.Counter((svc.get(k),res.get(k)) for k in svc if 'audiodna' in subj.get(k,''))
print('tcc audiodna AUTHREQ', dict(c) if c else 'none')
PY
}
run() { n=$1; shift; local t0=$(date '+%Y-%m-%d %H:%M:%S'); echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg)"; env "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(sm $O/$n.log 3 | tr '\n' ' ')"; quit_app >/dev/null; }
BASE=92eacf9
echo "== GATE-6 + GATE-7 btguard x5 + tripwire $(date +%T)"
acquire_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
for i in 1 2 3 4 5; do t0=$(date '+%Y-%m-%d %H:%M:%S'); run green-btguard-$i BTGUARD_APP=$APP bash $H/probe-btguard.sh $O/green-$i; sleep 16; echo "  run $i UNC=$(unc) $(tcc "$t0" $i)"; done
release_lock; trap - EXIT
echo "== GATE-8 audio battery $(date +%T) burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | sed 's#.*/##' | tr '\n' ';')"
{ wait_quiet || echo "NOT QUIET: battery runs under load $(sysctl -n vm.loadavg) -- a FAIL gets 5 interleaved runs before attribution"; acquire_lock || exit 70; }
trap 'quit_app >/dev/null; release_lock' EXIT
run step3 STEP3_BUILD_DIR=build bash $H/probe-step3.sh
run manual-bpm MANUALBPM_BUILD_DIR=build bash $H/probe-manual-bpm.sh
run resync RESYNC_BUILD_DIR=build bash $H/probe-resync.sh
run downbeat DOWNBEAT_BUILD_DIR=build bash $H/probe-downbeat-level.sh
run onset-render ONSET_BUILD_DIR=build bash $H/probe-onset-render.sh
run tempo-start TEMPOSTART_BUILD_DIR=build bash $H/probe-tempo-start.sh
run finalize-loop FINLOOP_BUILD_DIR=build bash $H/probe-finalize-loop.sh
release_lock; trap - EXIT
{ wait_quiet || echo "NOT QUIET: battery runs under load $(sysctl -n vm.loadavg) -- a FAIL gets 5 interleaved runs before attribution"; acquire_lock || exit 70; }
trap 'quit_app >/dev/null; release_lock' EXIT
run routines ROUTINES_BUILD_DIR=build ROUTINES_RECORD_PAUSE=1.8 bash $H/probe-routines.sh
run async-load bash $H/probe-async-load.sh $O/async-load
quit_app >/dev/null; release_lock; trap - EXIT
echo "outwins: $(outwins)"; sleep 16; echo "UNC windows (OptionAll): $(unc)"
echo "new Audio-DNA ips today: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt "$(date '+%Y-%m-%d') 00:00" | wc -l)"
echo "### GATE DONE $(date '+%F %T')"
echo "== GATE-9 production absence $(date +%T)"; python3 $S/wf/g9.py $O/g9 2>&1 | tail -8
echo "### GATE-9 DONE $(date '+%F %T')"
