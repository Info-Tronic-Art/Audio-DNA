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
echo "== START $(date '+%F %T') input transport: $(system_profiler SPAudioDataType | grep -A6 'Default Input Device: Yes' | grep -i transport | tr -s ' ')"
echo "burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | sed 's#.*/##' | tr '\n' ';')"
echo "== PRE sha $(shasum -a 256 $PRE/Contents/MacOS/Audio-DNA | cut -c1-16)"
BASE=$(git -C $M rev-parse HEAD)
if [ "$3" != nomerge ]; then
echo "== merge $(date +%T) base $BASE"
git -C $M merge --no-ff -q -F $S/wf/bt2-merge-msg.txt lane/bt2 || { echo "MERGE CONFLICT:"; git -C $M diff --name-only --diff-filter=U; git -C $M merge --abort; exit 2; }
fi
git -C $M log --oneline -1 | cut -c1-90
echo "code diff merge vs lane head (src tests docs CMakeLists CLAUDE.md probe): [$(git -C $M diff --stat lane/bt2 HEAD -- src tests docs CMakeLists.txt CLAUDE.md .harmony/probe-btguard.sh | tail -1)] (empty = identical, or = the other lane's merged change)"
cmake -S $M -B $M/build > $O/cfg.log 2>&1 && cmake --build $M/build -j6 > $O/build.log 2>&1; echo "GATE-1 build rc $? $(date +%T)"
TOUCHED=$(git -C $M diff --name-only $BASE HEAD -- src tests | sed 's#.*/##' | tr '\n' '|' | sed 's/|$//')
echo "GATE-1 warnings in touched files: $(grep -E 'warning:' $O/build.log | grep -E "($TOUCHED)" | sort -u | wc -l) (touched: $TOUCHED)"
grep -E 'warning:' $O/build.log | grep -E "($TOUCHED)" | sort -u | head -5
echo "== GATE-2 ctest $(date +%T)"; until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; ctest --test-dir $M/build -j1 > $O/ctest.log 2>&1; rm -rf /tmp/audiodna-ctest.lock; grep -E 'tests passed|Total Test' $O/ctest.log; echo "expected $EXP; ctest -N: $(ctest --test-dir $M/build -N | tail -1)"
echo "== GATE-10 sacred: callback diff lines $(git -C $M diff $BASE HEAD -- src/audio/CombinedCallback.h src/audio/AudioCallback.cpp | wc -l) (0 = PASS); new std::mutex lines: $(git -C $M diff $BASE HEAD -- src | grep -E '^\+' | grep -c 'std::mutex')"
echo "== GATE-11 docs: CLAUDE.md $(wc -c < $M/CLAUDE.md) B (base $(git -C $M show $BASE:CLAUDE.md | wc -c))"
P=$M/docs/claude/pitfalls.md; T=$M/docs/claude/testing-eyes.md
echo "  'The startup \`setSourceMode\` re-open stays' = $(grep -c 'The startup `setSourceMode` re-open stays' $P) (want 0)"
for s in 'the one exception is `DeviceReconciler`' 'the open device is STOPPED and nothing restarted it' 'tried once per distinct list of allowed inputs' 'lost - now listening on' 'M5r (JUCE'; do echo "  pitfalls '$s' = $(grep -cF "$s" $P) (want 1)"; done
for s in audio_stop device-stopped lost_input; do echo "  testing-eyes '$s' = $(grep -cF "$s" $T) (want >=1)"; done
echo "== GATE-6 + GATE-7 btguard x5 + tripwire $(date +%T)"
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
for i in 1 2 3 4 5; do t0=$(date '+%Y-%m-%d %H:%M:%S'); run green-btguard-$i BTGUARD_APP=$APP bash $H/probe-btguard.sh $O/green-$i; sleep 16; echo "  run $i UNC=$(unc) $(tcc "$t0" $i)"; done
release_lock; trap - EXIT
echo "== GATE-8 audio battery $(date +%T) burners: $(ps -Ao pcpu=,comm= | sort -rn | head -3 | sed 's#.*/##' | tr '\n' ';')"
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
run step3 STEP3_BUILD_DIR=build bash $H/probe-step3.sh
run manual-bpm MANUALBPM_BUILD_DIR=build bash $H/probe-manual-bpm.sh
run resync RESYNC_BUILD_DIR=build bash $H/probe-resync.sh
run downbeat DOWNBEAT_BUILD_DIR=build bash $H/probe-downbeat-level.sh
run onset-render ONSET_BUILD_DIR=build bash $H/probe-onset-render.sh
run tempo-start TEMPOSTART_BUILD_DIR=build bash $H/probe-tempo-start.sh
run finalize-loop FINLOOP_BUILD_DIR=build bash $H/probe-finalize-loop.sh
release_lock; trap - EXIT
acquire_quiet_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
run routines ROUTINES_BUILD_DIR=build ROUTINES_RECORD_PAUSE=1.8 bash $H/probe-routines.sh
run async-load bash $H/probe-async-load.sh $O/async-load
quit_app >/dev/null; release_lock; trap - EXIT
echo "outwins: $(outwins)"; sleep 16; echo "UNC windows (OptionAll): $(unc)"
echo "new Audio-DNA ips today: $(find ~/Library/Logs/DiagnosticReports -name 'Audio-DNA*' -newermt "$(date '+%Y-%m-%d') 00:00" | wc -l)"
echo "### GATE DONE $(date '+%F %T')"
echo "== GATE-9 production absence $(date +%T)"; python3 $S/wf/g9.py $O/g9 2>&1 | tail -8
echo "### GATE-9 DONE $(date '+%F %T')"
