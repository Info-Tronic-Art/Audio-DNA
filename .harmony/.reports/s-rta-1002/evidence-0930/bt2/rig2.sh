#!/bin/bash
set -e
SBX=/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2
ROOT=/Users/boriskarpman/projects/RealTimeAudio
BASE=$(python3 - <<'PY'
import json
cc=json.load(open('/Users/boriskarpman/projects/RealTimeAudio/build/compile_commands.json'))
for e in cc:
    if e['file'].endswith('tests/test_bt_device_shapes.cpp'):
        print(e.get('command') or ' '.join(e['arguments']))
PY
)
SRC="$ROOT/tests/test_bt_device_shapes.cpp"
cd "$ROOT/build/tests"
for f in "$SBX/x_c3_engine.cpp" "/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/44b528dd-1232-4d5c-a683-0145bc3a700e/scratchpad/bt2/c3mut/audio/AudioEngine.cpp" "$ROOT/src/audio/AudioCallback.cpp" "$ROOT/src/recording/AudioTap.cpp" \
         "$ROOT/src/audio/DeviceGuard.cpp" "$ROOT/src/audio/DevicePolicy.cpp" "$ROOT/src/audio/CoreAudioDeviceInfo.cpp"; do
  o="$SBX/c3_$(basename "$f").o"
  CMD="${BASE//$SRC/$f}"
  CMD=$(echo "$CMD" | sed -E "s# -o [^ ]+# -o $o#; s# -MF [^ ]+##; s# -MD##; s# -MT [^ ]+##")
  eval "$CMD -DAUDIODNA_TEST_SERVER=1 -DJUCE_MODAL_LOOPS_PERMITTED=1 -I$ROOT/src/audio"
done
OBJ="$ROOT/build/tests/CMakeFiles/test_bt_device_shapes.dir/__/_deps/juce-src/modules"
FW=$(cat "$ROOT/build/tests/CMakeFiles/test_bt_device_shapes.dir/link.txt" | grep -o -- '-\(weak_\)\?framework [A-Za-z]*' | tr '\n' ' ')
/usr/bin/c++ -O3 -DNDEBUG -arch arm64 -Wl,-search_paths_first -Wl,-headerpad_max_install_names $SBX/c3_*.o \
  "$OBJ/juce_core/juce_core_CompilationTime.cpp.o" "$OBJ/juce_core/juce_core.mm.o" "$OBJ/juce_events/juce_events.mm.o" \
  "$OBJ/juce_audio_basics/juce_audio_basics.mm.o" "$OBJ/juce_audio_devices/juce_audio_devices.mm.o" "$OBJ/juce_audio_formats/juce_audio_formats.mm.o" \
  "$OBJ/juce_audio_utils/juce_audio_utils.mm.o" "$OBJ/juce_audio_processors/juce_audio_processors_ara.cpp.o" "$OBJ/juce_audio_processors/juce_audio_processors_lv2_libs.cpp.o" \
  "$OBJ/juce_audio_processors/juce_audio_processors.mm.o" "$OBJ/juce_gui_extra/juce_gui_extra.mm.o" "$OBJ/juce_gui_basics/juce_gui_basics.mm.o" \
  "$OBJ/juce_graphics/juce_graphics_Harfbuzz.cpp.o" "$OBJ/juce_graphics/juce_graphics_Sheenbidi.c.o" "$OBJ/juce_graphics/juce_graphics.mm.o" \
  "$OBJ/juce_data_structures/juce_data_structures.mm.o" -o $SBX/x_c3_engine \
  $ROOT/build/_deps/catch2-build/src/libCatch2Main.a $ROOT/build/_deps/catch2-build/src/libCatch2.a $FW
echo BUILT
