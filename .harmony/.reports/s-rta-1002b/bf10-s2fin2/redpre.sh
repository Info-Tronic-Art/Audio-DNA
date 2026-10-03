#!/bin/bash
# RED: the STOP-3 test on the PRE-CHANGE e89bb5f ProjectMSource.cpp (swapped in, restored, sha-checked)
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin2
F=$WT/src/sources/ProjectMSource.cpp
cp $F $D/ProjectMSource.cpp.lane
SHA0=$(shasum -a 256 $F | cut -d' ' -f1)
git -C $WT show e89bb5f:src/sources/ProjectMSource.cpp > $F
echo "swapped in pre-change e89bb5f ProjectMSource.cpp: $(shasum -a 256 $F | cut -c1-16)"
bash $D/run.sh redPre
cp $D/ProjectMSource.cpp.lane $F
SHA1=$(shasum -a 256 $F | cut -d' ' -f1)
echo "restored: sha before=$SHA0 after=$SHA1 $([ "$SHA0" = "$SHA1" ] && echo IDENTICAL || echo MISMATCH)"
echo "git diff ProjectMSource.cpp lines: $(git -C $WT diff -- src/sources/ProjectMSource.cpp | wc -l | tr -d ' ')"
