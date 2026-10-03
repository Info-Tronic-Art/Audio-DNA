# returns when a new agent RESULT lands in any journal, or after maxsec; prints per-workflow done/started counts
D=/Users/boriskarpman/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/subagents/workflows
sig() { python3 /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/wf/sig.py $D; }
s0=$(sig); end=$(( $(date +%s) + ${1:-530} ))
while [ $(date +%s) -lt $end ]; do sleep 20; [ "$(sig)" != "$s0" ] && break; done
date '+%H:%M:%S'
for j in $D/*/journal.jsonl; do
  python3 - "$j" <<'PY'
import json,sys
st={};res=set()
for l in open(sys.argv[1]):
    try:o=json.loads(l)
    except:continue
    if o.get('type')=='started': st[o.get('key')]=o.get('label')
    if o.get('type')=='result': res.add(o.get('key'))
run=[v for k,v in st.items() if k not in res]; done=[v for k,v in st.items() if k in res]
import os
print(os.path.basename(os.path.dirname(sys.argv[1])), 'done', len(done), '| running:', ', '.join(run) if run else '-')
PY
done
