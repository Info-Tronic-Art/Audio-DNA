W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-media
D=/Users/boriskarpman/projects/RealTimeAudio/build/_deps
S=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/diag-media
date +%T; uptime
if [ ! -f $W/build-lane/CMakeCache.txt ]; then
cmake -S $W -B $W/build-lane -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
 -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src \
 -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $S/runs/cfg.log 2>&1
echo "cfg exit $?"
fi
cmake --build $W/build-lane --target AudioDNA -j3 > $S/runs/build-last.log 2>&1
echo "build exit $?"; tail -3 $S/runs/build-last.log; date +%T
