#!/bin/bash
set -e
SBX=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2
ROOT=/Users/boriskarpman/projects/RealTimeAudio
CMD=$(python3 - <<'PY'
import json
cc=json.load(open('/Users/boriskarpman/projects/RealTimeAudio/build/compile_commands.json'))
for e in cc:
    if e['file'].endswith('tests/test_device_policy.cpp'):
        print(e.get('command') or ' '.join(e['arguments']))
PY
)
# swap source + output
SRC="$ROOT/tests/test_device_policy.cpp"
CMD="${CMD//$SRC/$SBX/x_bt2_red.cpp}"
CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $SBX/x_bt2_red.o#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##")
cd "$ROOT/build/tests"
eval "$CMD -I$SBX"
OBJ="$ROOT/build/tests/CMakeFiles/test_device_policy.dir"
/usr/bin/c++ -O3 -DNDEBUG -arch arm64 -Wl,-search_paths_first -Wl,-headerpad_max_install_names \
  $SBX/x_bt2_red.o $OBJ/__/src/audio/DevicePolicy.cpp.o $OBJ/__/src/audio/CoreAudioDeviceInfo.cpp.o $OBJ/__/src/audio/DeviceGuard.cpp.o \
  "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core.mm.o" \
  "$OBJ/__/_deps/juce-src/modules/juce_events/juce_events.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_basics/juce_audio_basics.mm.o" \
  "$OBJ/__/_deps/juce-src/modules/juce_audio_devices/juce_audio_devices.mm.o" -o $SBX/x_bt2_red \
  $ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a \
  -framework Cocoa -framework Foundation -framework IOKit -framework Security -framework Accelerate -framework CoreAudio -framework CoreMIDI -framework AudioToolbox
echo BUILT
