# build.sh [cfg] -- configure (when asked or missing) + build build-lane. JUCE_DIR defaults to the PRIVATE instrumented
# copy ($S/juce-src); JUCE_DIR=/Users/boriskarpman/projects/RealTimeAudio/build/_deps/juce-src for the clean rebuild.
W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vfps
D=/Users/boriskarpman/projects/RealTimeAudio/build/_deps
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/f33bd4ec-5bde-4244-b1b7-b2ce40654c9a/scratchpad/diag-vfps
JUCE_DIR=${JUCE_DIR:-$S/juce-src}
echo "$(date +%T) build start; load $(sysctl -n vm.loadavg); JUCE=$JUCE_DIR"
if [ "$1" = "cfg" ] || [ ! -f $W/build-lane/CMakeCache.txt ]; then
  cmake -S $W -B $W/build-lane -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
   -DFETCHCONTENT_SOURCE_DIR_JUCE=$JUCE_DIR -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src \
   -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $S/cfg.log 2>&1
  echo "cfg exit $?"
fi
cmake --build $W/build-lane --target AudioDNA -j3 > $S/build.log 2>&1
rc=$?
grep -E "error:|FAILED" $S/build.log | head -20
tail -2 $S/build.log
echo "build exit $rc $(date +%T)"
