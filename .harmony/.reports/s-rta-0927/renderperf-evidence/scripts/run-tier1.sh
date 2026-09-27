# usage: bash run-tier1.sh LABEL APP   (caller holds the lock). Exactly the five Tier-1 files; never tests/visual as a dir.
SP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/renderperf
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w8
. $SP/lock.sh
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
L=$1; APP=$2; OUT=$SP/tier1/$L; rm -rf $OUT; mkdir -p $OUT
[ -n "$(adna)" ] && { echo "REFUSE: Audio-DNA already running"; exit 1; }
for p in 7070 8080; do lsof -nP -iTCP:$p -sTCP:LISTEN >/dev/null 2>&1 && { echo "REFUSE: $p has a listener"; exit 1; }; done
echo "TIER1 $L start $(date '+%F %T') load=$(sysctl -n vm.loadavg) app=$APP"
open -g --stdout $OUT/app-out.log --stderr $OUT/app-err.log "$APP" --args --test-mode
for _ in $(seq 1 60); do [ -n "$(curl -s --max-time 2 http://127.0.0.1:8080/api/health)" ] && break; sleep 1; done
sleep 2
T=$WT/tests/visual
AUDIODNA_NO_SPAWN=1 PYTHONDONTWRITEBYTECODE=1 $PY -m pytest -p no:cacheprovider -rA -q \
  $T/test_sources.py $T/test_effects.py $T/test_audio_reactivity.py $T/test_time_sweep.py $T/test_performance.py \
  > $OUT/pytest.log 2>&1
echo "pytest rc=$?"
tail -1 $OUT/pytest.log
grep -E "^(PASSED|FAILED|ERROR) " $OUT/pytest.log | awk '{print $2, $1}' | sort > $OUT/outcomes.txt
echo "TIER1 $L end $(date '+%F %T')"
quit_app
outwins
