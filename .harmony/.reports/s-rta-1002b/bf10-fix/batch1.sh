LANE=bf10-fix . /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/lib/lock.sh
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-fix
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
ST=$WT/.harmony/probe-milkdrop-selftest.sh
echo "$(date +%T) acquiring"
acquire_lock || exit 1
echo "$(date +%T) Audio-DNA running now: '$(adna)' (must be empty)"
echo "$(date +%T) RED on e3c73a2"; bash $ST $S/old/.harmony/probe-milkdrop.sh $S/old/.harmony/probe-milkdrop.py > $S/r12-red.log 2>&1; echo "rc $?"; tail -1 $S/r12-red.log
echo "$(date +%T) GREEN"; bash $ST > $S/r12-green.log 2>&1; echo "rc $?"; tail -1 $S/r12-green.log
bash $ST > $S/r12-green2.log 2>&1; echo "rc $?"; tail -1 $S/r12-green2.log
for m in M1 M2 M3 M4; do echo "$(date +%T) mutant $m"; bash $ST $S/$m/.harmony/probe-milkdrop.sh > $S/r12-$m.log 2>&1; echo "rc $?"; tail -1 $S/r12-$m.log; done
echo "$(date +%T) Audio-DNA running now: '$(adna)'"
release_lock
