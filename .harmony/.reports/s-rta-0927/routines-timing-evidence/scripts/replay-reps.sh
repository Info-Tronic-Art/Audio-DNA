S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/timing
P=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
for d in $S/runs-rep/*/run-*; do
  lab=$(basename $(dirname $d))
  TK=$(grep anchors $d/timing.jsonl | $P -c "import json,sys; print(json.load(sys.stdin)['take'])")
  grep -E '"(rjump|rstack)"' $d/timing.jsonl > $S/ev.tmp
  while read -r l; do
    set -- $(echo "$l" | $P -c "import json,sys; e=json.load(sys.stdin); print(e['ev'], e['k'], e.get('TJ') or e.get('T5'), e.get('T6', 'x'))")
    if [ "$1" = rjump ]; then
      $P $S/rt_old.py jump $3 $d/rjump$2.json < /dev/null | grep -E "hard cut|restored" | sed "s/^/$lab rep$2 OLD /"
      $P $S/rt_new.py jump $3 $d/rjump$2.json "$TK" < /dev/null | grep -E "hard cut|restored" | sed "s/^/$lab rep$2 NEW /"
    else
      [ "$4" = NA ] && continue
      $P $S/rt_old.py stack $3 $4 $d/ramp.json $d/rstack$2.json < /dev/null 2>/dev/null | grep "later restore" | sed "s/^/$lab rep$2 OLD /"
      $P $S/rt_new.py stack $3 $4 $d/ramp.json $d/rstack$2.json "$TK" < /dev/null 2>/dev/null | grep "later restore" | sed "s/^/$lab rep$2 NEW /"
    fi
  done < $S/ev.tmp
done
