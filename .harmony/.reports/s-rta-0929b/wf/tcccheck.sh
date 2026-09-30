LANE=harmony-gate
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad/lib/lock.sh
G=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/bd9a1c96-8ac1-42b0-a088-3d8fb87be738/scratchpad/gate
APP=/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app
system_profiler SPAudioDataType | grep -A6 'Default Input Device: Yes' | grep -i 'transport' ; system_profiler SPBluetoothDataType | grep -m1 State
acquire_lock || exit 1
T0=$(date '+%Y-%m-%d %H:%M:%S'); echo "start $T0"
start_app $APP $G/tcc
sleep 6; curl -s --max-time 3 -H 'Connection: close' http://127.0.0.1:7070/api/health | head -c 200; echo
quit_app; outwins; release_lock
sleep 16
/usr/bin/log show --start "$T0" --predicate 'process == "tccd" AND eventMessage CONTAINS "AUTHREQ"' --style compact 2>/dev/null > $G/tcc.log
python3 - "$G/tcc.log" <<'PY'
import re,sys,collections
svc={};subj={};res={}
for l in open(sys.argv[1],errors='ignore'):
  m=re.search(r'msgID=([\d.]+)',l)
  if not m: continue
  k=m.group(1)
  if 'AUTHREQ_CTX' in l: s=re.search(r'service=(\w+)',l); svc[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_SUBJECT' in l: s=re.search(r'subject=([^,]+)',l); subj[k]=s.group(1) if s else '?'
  elif 'AUTHREQ_RESULT' in l: s=re.search(r'authValue=(\d+), authReason=(\d+)',l); res[k]=s.groups() if s else '?'
print(collections.Counter((svc.get(k),res.get(k)) for k in svc if 'audiodna' in subj.get(k,'')))
PY
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionAll, Quartz.kCGNullWindowID)
print('UNC windows (OptionAll)', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
grep -i 'mic\|audio' $G/tcc/app-err.log | head -4
