M=/Users/boriskarpman/projects/RealTimeAudio; G=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/gate
LANE=harmony-gate
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/lib/lock.sh
N0=$(ls ~/Library/Logs/DiagnosticReports/ | grep -c '^Audio-DNA')
acquire_quiet_lock || exit 70; uptime
IDLEPAINT_LAUNCHES=5 bash $M/.harmony/probe-idle-paint.sh $G/idle-rerun c0_preflight,i1_idle_card,i2_idle_many16,v0_capture_teeth > $G/idle-rerun.log 2>&1; echo "rc $?"
quit_app >/dev/null; outwins; release_lock
grep -E '^(PASS|FAIL) |PROBE-IDLE' $G/idle-rerun.log | cut -c1-230
echo "new crash reports: $(( $(ls ~/Library/Logs/DiagnosticReports/ | grep -c '^Audio-DNA') - N0 ))"
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print('UNC windows', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
