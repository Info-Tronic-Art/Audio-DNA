#!/bin/bash
# I2 configure rows (ruling amendment 7 + G0.5): stale-cache row and negative control, on the lane tree (HEAD) and,
# RED-first, on the pre-change tree (e89bb5f; extracted to scratch, configure only).
WT=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf10
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/bf10-S2
D=/Users/boriskarpman/projects/RealTimeAudio/build/_deps
STOCK=$HOME/.local/lib/cmake/projectM4
COMMON="-DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src"
df -h /System/Volumes/Data | tail -1
rm -rf $S/pre-src $S/cfg-*
mkdir -p $S/pre-src
git -C $WT archive e89bb5f CMakeLists.txt cmake src tests | tar -x -C $S/pre-src
run() { # name src extra...
  local name=$1 src=$2; shift 2
  echo "== $name ($(date '+%T')) src=$src args: $*"
  cmake -S $src -B $S/cfg-$name $COMMON "$@" > $S/cfg-$name.log 2>&1
  local rc=$?
  echo "exit=$rc"
  grep -E "libprojectM|build-projectm|render_frame_fbo|CMake Error" $S/cfg-$name.log | head -8
  echo "cached projectM4_DIR: $(grep '^projectM4_DIR' $S/cfg-$name/CMakeCache.txt 2>/dev/null)"
}
run pre-stale $S/pre-src -DprojectM4_DIR=$STOCK
run pre-neg   $S/pre-src -DAUDIODNA_PROJECTM_PREFIX=/nonexistent -DprojectM4_DIR=$STOCK
run lane-stale $WT -DprojectM4_DIR=$STOCK
run lane-neg   $WT -DAUDIODNA_PROJECTM_PREFIX=/nonexistent -DprojectM4_DIR=$STOCK
echo "done $(date '+%T')"
