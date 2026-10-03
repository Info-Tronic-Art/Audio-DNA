#!/bin/bash
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S3
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
A_APP=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/apps/pre-ui.app
B_APP=$WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
OUTD=${1:-$D/perf1}
echo "perf start $(date +%T)"; ps -Ao pcpu=,etime=,comm= | sort -rn | head -6
LOCK_LIB=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/lib/lock.sh LANE=bf10-S3 bash $WT/.harmony/probe-vupload-ab.sh $A_APP $B_APP 5 $WT/.harmony/probe-milkdrop.sh perf_md_1080,perf_md_4k,perf_md_1080_2l $OUTD
echo "driver rc $? at $(date +%T)"
/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python $WT/.harmony/probe-milkdrop-ab.py $OUTD/ab.tsv
sleep 16; echo "UNC 16 s after: $(/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python $D/unc.py)"
echo "perf end $(date +%T)"
