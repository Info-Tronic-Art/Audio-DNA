# returns when a new non-attack agent RESULT lands in any journal of this session, or after $1 seconds (default 520)
D=/Users/boriskarpman/.claude/projects/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/subagents/workflows
sig() { /usr/bin/python3 /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad/wf/sig.py $D; }
s0=$(sig); end=$(( $(date +%s) + ${1:-520} ))
while [ $(date +%s) -lt $end ]; do sleep 20; [ "$(sig)" != "$s0" ] && break; done
date '+%H:%M:%S'
/usr/bin/python3 /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad/wf/status.py $D
