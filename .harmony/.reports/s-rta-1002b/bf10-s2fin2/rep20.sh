#!/bin/bash
# 20 consecutive T6 runs (STOP 3: 20/20 required). Per run: rc, Catch2 summary, frames/reads, the clause DATA,
# dense-read count, how many reads met the 5 %/5 % branch, and the median branch (cross-fade).
D=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2fin2
T=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10/build-lane/tests/test_projectm_canvas_gl
mkdir -p $D/rep
echo "start $(date '+%F %T') binary sha=$(shasum -a 256 $T | cut -c1-16)"
echo "cpu top: $(ps -Ao pcpu=,etime=,comm= | sort -rn | head -3 | tr -s ' ' | tr '\n' ';')"
pass=0
for i in $(seq 1 20); do
  L=$D/rep/t6-$i.log
  $T "T6 a MilkDrop soft cut draws into the canvas texture" > $L 2>&1; rc=$?
  [ $rc -eq 0 ] && pass=$((pass+1))
  dense=$(grep '^DATA soft-cut t=' $L | awk '{t=substr($3,3)+0; if(t>=0.2&&t<=1.2)n++} END{print n+0}')
  both=$(grep '^DATA soft-cut t=' $L | awk '{split($9,b,"=");split($10,a,"="); if(b[2]+0>=5 && a[2]+0>=5)n++} END{print n+0}')
  xfade=$(grep '^DATA soft-cut t=' $L | awk '{m=$6; gsub(/median=\(|\)/,"",m); split(m,c,","); A[1]=204;A[2]=51;A[3]=26;B[1]=26;B[2]=178;B[3]=229; hit=0; for(k=1;k<=3;k++){da=c[k]-A[k]; if(da<0)da=-da; db=c[k]-B[k]; if(db<0)db=-db; if(da>=10&&db>=10)hit=1} n+=hit} END{print n+0}')
  echo "T6 run $i rc=$rc | $(grep -E '^(test cases|All tests)' $L) | $(grep -E 'frames=|mid_transition' $L | cut -c6- | tr '\n' ' ')| dense_reads=$dense wipe_reads(5%/5%)=$both xfade_reads(median>=10)=$xfade"
done
echo "T6 PASS $pass/20"
echo "end $(date '+%F %T')"
