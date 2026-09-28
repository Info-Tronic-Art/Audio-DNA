W=/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w4
D=/Users/boriskarpman/projects/RealTimeAudio/build/_deps
date +%T; uptime
cmake -S $W -B $W/build-lane -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
 -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src \
 -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src > $W/../build-lane-w4-cfg.log 2>&1
echo "cfg exit $?"
cmake --build $W/build-lane --target AudioDNA -j3 2>&1 | tail -5
echo "build exit ${PIPESTATUS[0]}"; date +%T
