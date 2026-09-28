#!/bin/bash
# Harmony: lane tier1 harness test_fractals.py against APP (test mode). Usage: fractals.sh APP LABEL [extra pytest files...]
APP="$1"; L="$2"; shift 2
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad
T=${TESTDIR:-/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w3/tests/visual}
O=$S/gate/tier1/$L; mkdir -p $O
LANE=harmony . $S/lib/lock.sh
acquire_lock || exit 70
trap 'quit_app >/dev/null; release_lock' EXIT
start_app "$APP" $O test || exit 1
echo "START $L $(date '+%F %T') load=$(sysctl -n vm.loadavg) tests=$T"
for f in ${FILES:-test_fractals}; do
  AUDIODNA_NO_SPAWN=1 PYTHONDONTWRITEBYTECODE=1 /Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python -m pytest -p no:cacheprovider -rA -q --basetemp=$O/pt-$f $T/$f.py > $O/$f.log 2>&1
  echo "$f rc=$? :: $(tail -1 $O/$f.log)"
done
quit_app; outwins; echo "END $L $(date +%T)"
