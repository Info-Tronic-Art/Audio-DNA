#!/bin/bash
set -e
R=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/ruling
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
SRC="$ROOT/tests/test_device_policy.cpp"
CMD="${CMD//$SRC/$R/x_r13.cpp}"
CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $R/x_r13.o#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##")
cd "$ROOT/build/tests"
eval "$CMD -I$R"
OBJ="$ROOT/build/tests/CMakeFiles/test_device_policy.dir"
LIBS=("$OBJ/__/_deps/juce-src/modules/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/__/_deps/juce-src/modules/juce_core/juce_core.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_events/juce_events.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_basics/juce_audio_basics.mm.o" "$OBJ/__/_deps/juce-src/modules/juce_audio_devices/juce_audio_devices.mm.o")
FW="-framework Cocoa -framework Foundation -framework IOKit -framework Security -framework Accelerate -framework CoreAudio -framework CoreMIDI -framework AudioToolbox"
C2="$ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a"
/usr/bin/c++ -O3 -arch arm64 $R/x_r13.o $OBJ/__/src/audio/DevicePolicy.cpp.o $OBJ/__/src/audio/CoreAudioDeviceInfo.cpp.o $OBJ/__/src/audio/DeviceGuard.cpp.o "${LIBS[@]}" -o $R/x_r13_main $C2 $FW
/usr/bin/c++ -O3 -arch arm64 $R/x_r13.o $SBX/p3/DevicePolicy.cpp.o $SBX/p3/CoreAudioDeviceInfo.cpp.o $SBX/p3/DeviceGuard.cpp.o "${LIBS[@]}" -o $R/x_r13_proto $C2 $FW
echo BUILT
