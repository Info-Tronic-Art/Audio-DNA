#!/bin/bash
# usage: g4-run.sh <label> <probe> [<probe> ...]   -- G4 parity per probe into its own out dir; verdict lines to SUMMARY
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/267d1ba0-b658-4610-9f47-ef72f4b49872/scratchpad
M=/Users/boriskarpman/projects/RealTimeAudio; O=$S/gate/tsan; L=$1; shift
for p in "$@"; do bash $S/gatetools/g4-parity.real.sh $S/apps/pre-d4e81bd/Audio-DNA.app $M/build/AudioDNA_artefacts/Release/Audio-DNA.app $O/g4/$p $p > $O/g4-$p.log 2>&1
  echo "G4 [$L] $p rc=$? :: $(grep -E '^G4:' $O/g4-$p.log | tail -1) $(date +%T)" | tee -a $O/SUMMARY.txt
  grep -E 'PRE-EXISTING|MISSING' $O/g4-$p.log | head -4 | sed "s/^/   /" | tee -a $O/SUMMARY.txt; done
