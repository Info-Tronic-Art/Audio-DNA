LANE=harmony-gate
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/lib/lock.sh
M=/Users/boriskarpman/projects/RealTimeAudio; W=$M/.claude/worktrees/rta0929-g4cpu; G=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/gate
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app; PRE=$G/pre-g4/Audio-DNA.app
rm -rf $G/pre-g4; mkdir -p $G/pre-g4; cp -R $APP $PRE
echo "== pre-merge app copy sha $(shasum -a 256 $PRE/Contents/MacOS/Audio-DNA | cut -c1-16) $(date +%T)"
echo "== RED (lane probe rows on the pre-merge app)"
acquire_quiet_lock || exit 1; uptime
IDLEPAINT_APP=$PRE bash $W/.harmony/probe-idle-paint.sh $G/red-g4 g4_routine,v5_routine_identity > $G/red-g4.log 2>&1; echo "probe rc $?"
quit_app; outwins; release_lock
grep -E '^(PASS|FAIL) |PROBE-IDLE' $G/red-g4.log | cut -c1-260
echo "== merge $(date +%T)"
git -C $M merge --no-ff -q -m "merge(s-rta-0929): lane/g4cpu - the ROUTINES pad repaints only when its painted picture changes (RoutinePad::paintKeyOf): pad repaint requests 29 -> 11.6/s, g4 main-thread CPU 176 -> 155 ms/s interleaved (INFO, J2), pixels identical (v5 0 px at two beat positions); TEST-ONLY per-source repaint / paint counters + per-pass log (GET /api/debug/ui_passes), probe-idle-paint a1 / v5 / G12 cadence rows

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" lane/g4cpu || { echo MERGE CONFLICT; git -C $M merge --abort; exit 2; }
git -C $M log --oneline -1 | cut -c1-80
echo "== build $(date +%T)"
cmake -S $M -B $M/build > $G/cfg-g4.log 2>&1 && cmake --build $M/build -j6 > $G/build-g4.log 2>&1; echo "build rc $?"
echo "== ctest $(date +%T)"; ctest --test-dir $M/build -j1 2>&1 | tail -3
echo "== GREEN $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
acquire_quiet_lock || exit 1; uptime
IDLEPAINT_APP_BEFORE=$PRE bash $M/.harmony/probe-idle-paint.sh $G/green-g4 > $G/green-g4.log 2>&1; echo "probe rc $?"
quit_app; outwins; release_lock
grep -E '^(PASS|FAIL) |PROBE-IDLE' $G/green-g4.log | cut -c1-200
acquire_quiet_lock || exit 1
bash $M/.harmony/probe-routine-display.sh $G/green-rd > $G/green-rd.log 2>&1; echo "routine-display rc $?"; quit_app; release_lock
grep -E 'PY [0-9]+ PASS|PROBE-' $G/green-rd.log | tail -2
outwins
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print('UNC windows', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "== done $(date +%T)"
