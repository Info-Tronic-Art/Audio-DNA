#!/bin/bash
# usage: configure.sh <builddir> <sanitize> [extra cmake args]
set -e
R=/Users/boriskarpman/projects/RealTimeAudio
B=$1; S=$2; shift 2
D=$R/build/_deps
cmake -S $R -B $B -DCMAKE_BUILD_TYPE=RelWithDebInfo -DADNA_SANITIZE="$S" \
 -DAUDIODNA_BUILD_TEST_SERVER=ON -DAUDIODNA_BUILD_SYPHON=ON -DFETCHCONTENT_FULLY_DISCONNECTED=ON \
 -DFETCHCONTENT_SOURCE_DIR_JUCE=$D/juce-src -DFETCHCONTENT_SOURCE_DIR_HTTPLIB=$D/httplib-src \
 -DFETCHCONTENT_SOURCE_DIR_CATCH2=$D/catch2-src -DFETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR=$D/melatonin_inspector-src \
 -DFETCHCONTENT_SOURCE_DIR_SYPHON=$D/syphon-src -DCMAKE_EXPORT_COMPILE_COMMANDS=ON "$@"
