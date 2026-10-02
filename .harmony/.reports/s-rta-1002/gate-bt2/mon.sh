#!/bin/bash
# one-line lane monitor
M=/Users/boriskarpman/projects/RealTimeAudio
for l in tsan bt2; do W=$M/.claude/worktrees/$l; n=$(git -C $W rev-list --count 2d38b39..HEAD 2>/dev/null); last=$(git -C $W log -1 --format='%h %cr %s' 2>/dev/null | cut -c1-90); rpt=$(ls $W/.harmony/.reports/s-rta-1002/$l.md 2>/dev/null >/dev/null && wc -l < $W/.harmony/.reports/s-rta-1002/$l.md || echo 0); dirty=$(git -C $W status --short 2>/dev/null | grep -v '^??' | wc -l | tr -d ' '); echo "$l: commits=$n dirty=$dirty rpt=${rpt}L last=[$last]"; done
echo "lock=$(cat /tmp/audiodna-live.lock/owner 2>/dev/null | cut -d' ' -f1 || echo free) adna=$(pgrep -x Audio-DNA | wc -l | tr -d ' ') clang=$(pgrep -x 'clang\+\+' | wc -l | tr -d ' ')+$(pgrep -x clang | wc -l | tr -d ' ') disk=$(df -h /System/Volumes/Data | awk 'NR==2{print $4}') load=$(sysctl -n vm.loadavg | cut -d' ' -f2-4) $(date +%T)"
