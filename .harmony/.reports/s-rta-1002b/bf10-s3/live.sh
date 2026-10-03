#!/bin/bash
# usage: live.sh <arm-name> <app> <mode pre|lane> <arm A|B> [rows]
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S3
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
PY=/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python
NAME=$1; APPX=$2; MODEX=$3; ARMX=$4; ROWSX=${5:-}
LIB=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/lib/lock.sh
LANE=bf10-S3; . $LIB
echo "== $NAME start $(date +%T) app $APPX ($(shasum -a 256 "$APPX/Contents/MacOS/Audio-DNA" | cut -c1-16)) mode $MODEX arm $ARMX rows '${ROWSX}'"
echo "UNC before: $($PY $D/unc.py)"; echo "outwins before: $(outwins)"
acquire_lock || { echo "no lock"; exit 1; }
echo "top cpu at start:"; ps -Ao pcpu=,etime=,comm= | sort -rn | head -5
MILKDROP_MODE=$MODEX MILKDROP_ARM=$ARMX MILKDROP_APP="$APPX" LOCK_LIB=$LIB LANE=bf10-S3 bash $WT/.harmony/probe-milkdrop.sh $D/runs/$NAME "$ROWSX"
RCX=$?
echo "probe rc $RCX at $(date +%T)"
echo "outwins after: $(outwins)"
[ -n "$(adna)" ] && echo "WARNING: Audio-DNA pid $(adna) still running after the probe"
release_lock
sleep 16
echo "UNC 16 s after quit: $($PY $D/unc.py)"
echo "== $NAME end $(date +%T)"
