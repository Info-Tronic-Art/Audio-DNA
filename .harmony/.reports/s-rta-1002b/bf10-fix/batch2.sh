LOCK=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/lib/lock.sh
LANE=bf10-fix . $LOCK
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-fix
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
SPL=$(dirname $LOCK)
echo "$(date +%T) acquiring"
acquire_lock || exit 1
echo "$(date +%T) HEAD $(git -C $WT rev-parse --short HEAD); Audio-DNA running now: '$(adna)' (must be empty)"
bash $WT/.harmony/probe-milkdrop-selftest.sh > $S/r12-green-head.log 2>&1; echo "selftest at HEAD rc $?: $(tail -1 $S/r12-green-head.log)"
# the REAL lock helper's contract: sourced in a subshell with LANE set, it names start_app's record in OURPID
echo "real helper OURPID for LANE=bf10-fix: $( LANE=bf10-fix; . $LOCK >/dev/null 2>&1 && echo $OURPID )"
D=$(mktemp -d "${TMPDIR:-/tmp}/md-real.XXXXXX"); mkdir -p $D/shim $D/out
printf '#!/bin/bash\nfor p in ${SHIM_PIDS:-}; do echo "$p Audio-DNA"; done\n' > $D/shim/ps
printf '#!/bin/bash\nfor a in "$@"; do case "$a" in -iTCP:7070) printf "COMMAND PID\\nAudio-DNA 4242\\n"; exit 0;; -iTCP:8080) echo 4242; exit 0;; esac; done; exit 1\n' > $D/shim/lsof
printf '#!/bin/bash\necho "$*" >> %s/curl-calls; echo ok\n' $D > $D/shim/curl
printf '#!/bin/bash\n[ "$1" = -c ] && exit 0\necho BODY >> %s/py-invoked\n' $D > $D/stubpy
chmod +x $D/shim/* $D/stubpy
L=bf10-selftest-$$
for rec in 5555 4242; do
  echo $rec > $SPL/.ours-pid-$L; rm -f $D/py-invoked $D/curl-calls
  out=$(env -u MILKDROP_ATTACH_PID PATH=$D/shim:$PATH SHIM_PIDS=4242 MILKDROP_ATTACH=1 MILKDROP_PY=$D/stubpy LOCK_LIB=$LOCK LANE=$L bash $WT/.harmony/probe-milkdrop.sh $D/out 2>&1); rc=$?
  echo "real helper, record $rec, running 4242: rc $rc, body $([ -f $D/py-invoked ] && echo ran || echo 'not run'), curl calls $(cat $D/curl-calls 2>/dev/null | wc -l | tr -d ' '), $(grep -m1 REFUSE <<< "$out")"
done
rm -f $SPL/.ours-pid-$L; rm -rf $D
echo "$(date +%T) Audio-DNA running now: '$(adna)'"
release_lock
