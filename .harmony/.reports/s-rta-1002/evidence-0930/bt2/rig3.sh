#!/bin/bash
set -e
SBX=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2
ROOT=/Users/boriskarpman/projects/RealTimeAudio
BASE=$(python3 - <<'PY'
import json
cc=json.load(open('/Users/boriskarpman/projects/RealTimeAudio/build/compile_commands.json'))
for e in cc:
    if e['file'].endswith('tests/test_device_policy.cpp'):
        print(e.get('command') or ' '.join(e['arguments']))
PY
)
SRC="$ROOT/tests/test_device_policy.cpp"
mkdir -p $SBX/p3 && rm -f $SBX/p3/*.o
cd "$ROOT/build/tests"
# the prototype sources shadow src/audio via an include dir placed FIRST (quoted includes "audio/..." resolve under $SBX/proto)
for f in "$ROOT/tests/test_device_policy.cpp" "$SBX/x_bt2_red.cpp" "$SBX/x_bt2_new.cpp" \
         "$SBX/proto/audio/DevicePolicy.cpp" "$SBX/proto/audio/DeviceGuard.cpp" "$SBX/proto/audio/CoreAudioDeviceInfo.cpp"; do
  o="$SBX/p3/$(basename "$f").o"
  CMD="${BASE//$SRC/$f}"
  CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $o#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##; s#-I$ROOT/src #-I$SBX/proto -I$ROOT/src #")
  eval "$CMD -I$SBX"
done
OBJ="$ROOT/build/tests/CMakeFiles/test_device_policy.dir"
for main in test_device_policy.cpp x_bt2_red.cpp x_bt2_new.cpp; do
/usr/bin/c++ -O3 -DNDEBUG -arch arm64 -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
  $SBX/p3/$main.o $SBX/p3/DevicePolicy.cpp.o $SBX/p3/CoreAudioDeviceInfo.cpp.o $SBX/p3/DeviceGuard.cpp.o \
  "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core.mm.o" \
  "$OBJ/__/_deps/juce-src/modules/juce_events/juce_events.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_basics/juce_audio_basics.mm.o" \
  "$OBJ/__/_deps/juce-src/modules/juce_audio_devices/juce_audio_devices.mm.o" -o $SBX/p3/run_${main%.cpp} \
  $ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a \
  -framework Cocoa -framework Foundation -framework IOKit -framework Security -framework Accelerate -framework CoreAudio -framework CoreMIDI -framework AudioToolbox
done
echo BUILT
