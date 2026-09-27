# usage: bash batch.sh NAME STEPFILE   -- STEPFILE: bash lines run in order while holding the lock (quiet-gated)
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
. $SP/fix/lock.sh
NAME=$1; STEPS=$2; O=$SP/fix/live/$NAME; mkdir -p $O
echo "### batch $NAME $(date '+%F %T')"
[ -n "${NEEDQUIET:-}" ] && { wait_quiet || echo "WARNING: not quiet"; }
acquire_lock || exit 1
[ -n "${NEEDQUIET:-}" ] && { wait_quiet || echo "WARNING: not quiet"; }
[ -e $WT/.venv ] || ln -s /Users/boriskarpman/projects/RealTimeAudio/.venv $WT/.venv
$PY $SP/sampler.py $O/window-sampler.txt & SAMP=$!
step() { local n=$1; shift; echo "### $n $(date +%T) load=$(sysctl -n vm.loadavg) compilers=[$(pgrep -x clang | wc -l | tr -d ' ')/$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')]"; "$@" > $O/$n.log 2>&1; echo "rc=$? :: $(grep -E 'PASS / .* FAIL|GREEN$|RED$|passed|failed' $O/$n.log | tail -2 | tr '\n' ' ')"; }
. $STEPS
kill $SAMP 2>/dev/null; cat $O/window-sampler.txt
[ -n "$(adna)" ] && { echo "app still running at batch end -- quitting"; quit_app; }
outwins; rm -f $WT/.venv; echo ".venv symlink removed: $([ -e $WT/.venv ] && echo NO || echo yes)"
release_lock
echo "### batch $NAME done $(date '+%F %T')"
