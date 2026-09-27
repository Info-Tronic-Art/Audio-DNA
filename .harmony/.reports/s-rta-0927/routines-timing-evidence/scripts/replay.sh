# offline replay of the OLD vs NEW row code over every recorded run; TEETH=1 adds mutant scenarios from real samples
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/timing
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
for d in $(ls -d ${1:-$S/runs}/*/run-* | sort); do
  lab=$(basename $(dirname $d))
  an=$(grep anchors $d/timing.jsonl) || continue
  get(){ echo "$an" | $PY -c "import json,sys; print(json.load(sys.stdin)['$1'])"; }
  T1=$(get T1); T2=$(get T2); T4=$(get T4); T5=$(get T5); T6=$(get T6); TJ=$(get TJ); TK=$(get take)
  echo "== $lab"
  echo "-- OLD"
  $PY $S/rt_old.py stack $T5 $T6 $d/ramp.json $d/stack.json | grep -E "later restore wins"
  $PY $S/rt_old.py jump $TJ $d/jump.json | grep -E "hard cut|restored to 1.0"
  $PY $S/rt_old.py loop $T2 $d/loop.json 0 | grep -E "holds L0 opacity 0.9"
  $PY $S/rt_old.py jumploop $TJ $d/jumploop.json | grep -E "holds L0|cuts to 1.0"
  echo "-- NEW"
  $PY $S/rt_new.py stack $T5 $T6 $d/ramp.json $d/stack.json "$TK" | grep -E "later restore wins"
  $PY $S/rt_new.py jump $TJ $d/jump.json "$TK" | grep -E "^\(11j|hard cut|restored to 1.0"
  $PY $S/rt_new.py loop $T2 $d/loop.json 0 | grep -E "holds L0 opacity 0.9"
  $PY $S/rt_new.py jumploop $TJ $d/jumploop.json | grep -E "holds L0|cuts to 1.0"
  if [ "${TEETH:-0}" = 1 ]; then
    echo "-- TEETH (expect no): jump rows on the Ease start (glide.json, T1) = restoreStyle ignored"
    $PY $S/rt_new.py jump $T1 $d/glide.json "$TK" | grep -E "hard cut"
    echo "-- TEETH (expect no): jump 'restored' + stack 'wins' on a start WITHOUT restore (fromnow.json, T4)"
    $PY $S/rt_new.py jump $T4 $d/fromnow.json "$TK" | grep -E "restored to 1.0"
    $PY $S/rt_new.py stack $T5 $T4 $d/ramp.json $d/fromnow.json "$TK" | grep -E "later restore wins"
    echo "-- TEETH (expect no): jumploop 'holds 0.9' on the Ease loop return (loop.json, T2) = the glide not disabled"
    $PY $S/rt_new.py jumploop $T2 $d/loop.json | grep -E "holds L0"
  fi
done
