#!/bin/bash
# live.sh <tag>: one probe-btguard run of the lane build under the live lock, with AM8's TCC / dialog tripwire.
TAG=$1
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/bt2-K
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bt2
APP=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
OUT=$S/live-$TAG; mkdir -p $OUT
LANE=bt2-K
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad/lib/lock.sh
echo "=== live $TAG  build $(git -C $WT rev-parse --short HEAD)  binary $(stat -f '%Sm' $APP/Contents/MacOS/Audio-DNA)"
system_profiler SPAudioDataType 2>/dev/null | grep -B2 -A6 'Default Input Device: Yes' | grep -iE 'transport|^ +[A-Z].*:$' | head -4
acquire_lock || exit 1
T0=$(date '+%Y-%m-%d %H:%M:%S'); echo "run start $T0"
echo "--- ps (top CPU) at start:"; ps -Ao pcpu=,etime=,comm= | sort -rn | head -5
[ -e $WT/.venv ] || ln -s /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
BTGUARD_APP=$APP bash $WT/.harmony/probe-btguard.sh $OUT > $OUT/probe.out 2>&1
echo "probe exit $?"
[ -n "$(adna)" ] && { echo "app still running after the probe -> quit_app"; quit_app; }
echo "app running: $([ -n "$(adna)" ] && echo YES || echo no)"
outwins
release_lock
T1=$(date '+%Y-%m-%d %H:%M:%S'); echo "run end $T1"
rm -f $WT/.venv
sleep 16
/usr/bin/log show --start "$T0" --predicate 'process == "tccd" AND eventMessage CONTAINS "AUTHREQ"' --style compact 2>/dev/null > $OUT/tcc.log
echo "--- TCC tripwire $T0 .. +16 s after $T1:"
python3 - "$OUT/tcc.log" <<'PY'
import re,sys,collections
svc={};subj={};res={};prompt=0
for l in open(sys.argv[1],errors='ignore'):
  m=re.search(r'msgID=([\d.]+)',l)
  if 'AUTHREQ_PROMPTING' in l and 'audiodna' in l.lower(): prompt+=1
  if not m: continue
  k=m.group(1)
  if 'AUTHREQ_CTX' in l: s=re.search(r'service=(\w+)',l); svc[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_SUBJECT' in l: s=re.search(r'subject=([^,]+)',l); subj[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_RESULT' in l: s=re.search(r'authValue=(\d+), authReason=(\d+)',l); res[k]=s.groups() if s else '?'
print('audiodna requests (service, (authValue, authReason)):', dict(collections.Counter((svc.get(k),res.get(k)) for k in svc if 'audiodna' in subj.get(k,''))))
print('AUTHREQ_PROMPTING for audiodna:', prompt)
PY
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC windows (kCGWindowListOptionAll, >= 15 s after the last quit):', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "--- probe summary:"; grep -E "^(PASS|FAIL|SKIP|INFO)|PROBE-BTGUARD" $OUT/probe.out
