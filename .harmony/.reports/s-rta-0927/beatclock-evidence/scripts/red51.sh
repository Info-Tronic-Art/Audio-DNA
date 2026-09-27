#!/bin/bash
# RED for plan 5.1: compile the new test file against the PRE-change (HEAD = commit 1) analysis headers.
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w7
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/beatclock
rm -rf $S/oldsrc; mkdir -p $S/oldsrc/analysis
git -C $WT show HEAD:src/analysis/BPMTracker.h > $S/oldsrc/analysis/BPMTracker.h
for f in $(grep -o '#include "[^"]*"' $S/oldsrc/analysis/BPMTracker.h | cut -d'"' -f2); do mkdir -p $S/oldsrc/$(dirname $f); git -C $WT show HEAD:src/$f > $S/oldsrc/$f 2>/dev/null; done
clang++ -std=c++20 -fsyntax-only -I $S/oldsrc -I $WT/src -I $WT/build-lane/_deps/catch2-src/src -I $WT/build-lane/_deps/catch2-build/generated-includes -I /opt/homebrew/include $WT/tests/test_bpm_stabilization.cpp 2>&1 | grep -E "error:" | head -20
echo "RED51 compile exit ${PIPESTATUS[0]}"
