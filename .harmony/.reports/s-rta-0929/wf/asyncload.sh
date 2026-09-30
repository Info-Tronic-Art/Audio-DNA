LANE=harmony-gate
. /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/lib/lock.sh
M=/Users/boriskarpman/projects/RealTimeAudio; W=$M/.claude/worktrees/rta0929-asyncload; G=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/gate
APP=$M/build/AudioDNA_artefacts/Release/Audio-DNA.app
echo "== RED on pre-merge main app $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
acquire_quiet_lock || exit 1; uptime
ASYNCLOAD_APP=$APP bash $W/.harmony/probe-async-load.sh $G/red-async > $G/red-async.log 2>&1; echo "probe rc $?"
quit_app; outwins; release_lock
grep -E 'PROBE-|PY [0-9]+ PASS|^(PASS|FAIL) ' $G/red-async.log | grep -cE '^FAIL'; grep -E 'PY [0-9]+ PASS|PROBE-ASYNC' $G/red-async.log | tail -4
echo "== merge $(date +%T)"
git -C $M merge --no-ff -q -m "merge(s-rta-0929): lane/asyncload - composition / Load Deck / Duplicate Deck loads are staged: videos open on a 2 x low MediaOpener pool, the UI stays live (16 x 4K load message-thread stall 244-257 -> 8-16 ms), the old show plays until a clean cut, POST /api/load_composition answers after the swap (LoadTicket), Append / Duplicate queue FIFO, cancel retires every adopted id, a hung open at quit leaks its pool instead of killing a thread; TEST-ONLY audio witnesses; probe-async-load

Co-Authored-By: Claude Opus 5.5 (1M context) <noreply@anthropic.com>" lane/asyncload || { echo MERGE CONFLICT; git -C $M merge --abort; exit 2; }
git -C $M log --oneline -1
echo "== build $(date +%T)"
cmake -S $M -B $M/build > $G/cfg.log 2>&1 && cmake --build $M/build -j6 > $G/build.log 2>&1; echo "build rc $?"; tail -2 $G/build.log
echo "== ctest $(date +%T)"; ctest --test-dir $M/build -j1 2>&1 | tail -3
echo "== GREEN $(date +%T) sha $(shasum -a 256 $APP/Contents/MacOS/Audio-DNA | cut -c1-16)"
acquire_quiet_lock || exit 1; uptime
bash $M/.harmony/probe-async-load.sh $G/green-async > $G/green-async.log 2>&1; echo "probe rc $?"
quit_app; outwins; release_lock
grep -E 'PY [0-9]+ PASS|PROBE-ASYNC' $G/green-async.log | tail -4
acquire_quiet_lock || exit 1
bash $M/.harmony/probe-media-open.sh $G/green-mo > $G/green-mo.log 2>&1; echo "media-open rc $?"; quit_app
release_lock
grep -E 'PY [0-9]+ PASS|PROBE-' $G/green-mo.log | tail -2
outwins
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -c "
import Quartz
wl = Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID)
print('UNC windows', len([w for w in wl if 'UserNotificationCenter' in str(w.get('kCGWindowOwnerName',''))]))"
echo "== done $(date +%T)"
